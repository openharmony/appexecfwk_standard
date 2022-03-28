/*
 * Copyright (c) 2022 Huawei Device Co., Ltd.
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include "distributed_data_storage.h"

#include <unistd.h>

#include "app_log_wrapper.h"
#ifdef DEVICE_MANAGER_ENABLE
#include "bms_device_manager.h"
#endif
#include "bundle_util.h"
#include "parameter.h"

using namespace OHOS::DistributedKv;

namespace OHOS {
namespace AppExecFwk {
namespace {
const int32_t MAX_TIMES = 600;              // 1min
const int32_t SLEEP_INTERVAL = 100 * 1000;  // 100ms
const uint32_t DEVICE_UDID_LENGTH = 65;
}  // namespace

std::shared_ptr<DistributedDataStorage> DistributedDataStorage::instance_ = nullptr;
std::recursive_mutex DistributedDataStorage::mutex_;

std::shared_ptr<DistributedDataStorage> DistributedDataStorage::GetInstance()
{
    if (instance_ == nullptr) {
        std::lock_guard<std::recursive_mutex> lock_l(mutex_);
        if (instance_ == nullptr) {
            instance_ = std::make_shared<DistributedDataStorage>();
        }
    }
    return instance_;
}

DistributedDataStorage::DistributedDataStorage()
{
    APP_LOGI("instance is created");
    TryTwice([this] { return GetKvStore(); });
}

DistributedDataStorage::~DistributedDataStorage()
{
    APP_LOGI("instance is destroyed");
    dataManager_.CloseKvStore(appId_, storeId_);
}

bool DistributedDataStorage::SaveStorageDistributeInfo(const BundleInfo &info)
{
    APP_LOGI("save DistributedBundleInfo data");
    {
        std::lock_guard<std::mutex> lock(kvStorePtrMutex_);
        if (!CheckKvStore()) {
            APP_LOGE("kvStore is nullptr");
            return false;
        }
    }
    std::string udid;
    bool ret = GetLocalUdid(udid);
    if (!ret) {
        APP_LOGE("GetLocalUdid error");
        return false;
    }
    std::string keyOfData;
    DeviceAndNameToKey(udid, info.name, keyOfData);
    Key key(keyOfData);
    Value value(ConvertToDistributedBundleInfo(info).ToString());
    Status status;
    {
        std::lock_guard<std::mutex> lock(kvStorePtrMutex_);
        status = kvStorePtr_->Put(key, value);
        if (status == Status::IPC_ERROR) {
            status = kvStorePtr_->Put(key, value);
            APP_LOGW("distribute database ipc error and try to call again, result = %{public}d", status);
        }
    }
    if (status != Status::SUCCESS) {
        APP_LOGE("put to kvStore error: %{public}d", status);
        return false;
    } else {
        APP_LOGI("put value to kvStore success");
    }
    return true;
}

bool DistributedDataStorage::DeleteStorageDistributeInfo(const std::string &bundleName)
{
    APP_LOGI("query DistributedBundleInfo");
    {
        std::lock_guard<std::mutex> lock(kvStorePtrMutex_);
        if (!CheckKvStore()) {
            APP_LOGE("kvStore is nullptr");
            return false;
        }
    }
    std::string udid;
    bool ret = GetLocalUdid(udid);
    if (!ret) {
        APP_LOGE("GetLocalUdid error");
        return false;
    }
    std::string keyOfData;
    DeviceAndNameToKey(udid, bundleName, keyOfData);
    Key key(keyOfData);
    Status status;
    {
        std::lock_guard<std::mutex> lock(kvStorePtrMutex_);
        status = kvStorePtr_->Delete(key);
        if (status == Status::IPC_ERROR) {
            status = kvStorePtr_->Delete(key);
            APP_LOGW("distribute database ipc error and try to call again, result = %{public}d", status);
        }
    }
    if (status != Status::SUCCESS) {
        APP_LOGE("delete key error: %{public}d", status);
        return false;
    } else {
        APP_LOGI("delete value to kvStore success");
    }
    return true;
}

bool DistributedDataStorage::QueryStroageDistributeInfo(
    const std::string &bundleName, int32_t userId, const std::string &networkId,
    DistributedBundleInfo &info)
{
    APP_LOGI("query DistributedBundleInfo");
    std::string udid;
    int32_t ret = GetUdidByNetworkId(networkId, udid);
    if (ret != 0) {
        APP_LOGI("can not get udid by networkId error:%{public}d", ret);
        return false;
    }

    {
        std::lock_guard<std::mutex> lock(kvStorePtrMutex_);
        if (!CheckKvStore()) {
            APP_LOGE("kvStore is nullptr");
            return false;
        }
    }

    std::string keyOfData;
    DeviceAndNameToKey(udid, bundleName, keyOfData);
    APP_LOGI("keyOfData: [%{public}s]", keyOfData.c_str());
    Key key(keyOfData);
    Value value;
    Status status;
    {
        std::lock_guard<std::mutex> lock(kvStorePtrMutex_);
        status = kvStorePtr_->Get(key, value);
        if (status == Status::SUCCESS) {
            if (!info.FromJsonString(value.ToString())) {
                APP_LOGE("it's an error value");
                kvStorePtr_->Delete(key);
                return false;
            }
            return true;
        }
        APP_LOGI("get value status: %{public}d", status);
        else if (status == Status::IPC_ERROR) {
            status = kvStorePtr_->Get(key, value);
            APP_LOGW("distribute database ipc error and try to call again, result = %{public}d", status);
            if (status == Status::SUCCESS) {
                if (!info.FromJsonString(value.ToString())) {
                    APP_LOGE("it's an error value");
                    kvStorePtr_->Delete(key);
                    return false;
                }
                return true;
            }
            APP_LOGE("get value error: %{public}d", status);
            return false;
        }
        return false;
    }
}

void DistributedDataStorage::DeviceAndNameToKey(
    const std::string &udid, const std::string &bundleName, std::string &key) const
{
    key.append(udid);
    key.append(Constants::FILE_UNDERLINE);
    key.append(bundleName);
}

bool DistributedDataStorage::CheckKvStore()
{
    if (kvStorePtr_ != nullptr) {
        return true;
    }
    int32_t tryTimes = MAX_TIMES;
    while (tryTimes > 0) {
        Status status = GetKvStore();
        if (status == Status::SUCCESS && kvStorePtr_ != nullptr) {
            return true;
        }
        APP_LOGI("CheckKvStore, Times: %{public}d", tryTimes);
        usleep(SLEEP_INTERVAL);
        tryTimes--;
    }
    return kvStorePtr_ != nullptr;
}

Status DistributedDataStorage::GetKvStore()
{
    Options options = {
        .createIfMissing = true,
        .encrypt = false,
        .autoSync = false,
        .kvStoreType = KvStoreType::SINGLE_VERSION
        };
    Status status = dataManager_.GetSingleKvStore(options, appId_, storeId_, kvStorePtr_);
    if (status != Status::SUCCESS) {
        APP_LOGE("return error: %{public}d", status);
    } else {
        APP_LOGI("get kvStore success");
    }
    return status;
}

void DistributedDataStorage::TryTwice(const std::function<Status()> &func) const
{
    Status status = func();
    if (status == Status::IPC_ERROR) {
        status = func();
        APP_LOGW("distribute database ipc error and try to call again, result = %{public}d", status);
    }
}

bool DistributedDataStorage::GetLocalUdid(std::string &udid)
{
    char innerUdid[DEVICE_UDID_LENGTH] = {0};
    int ret = GetDevUdid(innerUdid, DEVICE_UDID_LENGTH);
    if (ret != 0) {
        APP_LOGI("GetDevUdid failed ,ret:%{public}d", ret);
        return false;
    }
    udid = std::string(innerUdid);
    return true;
}

int32_t DistributedDataStorage::GetUdidByNetworkId(const std::string &networkId, std::string &udid)
{
#ifdef DEVICE_MANAGER_ENABLE
    return BmsDeviceManager::GetUdidByNetworkId(networkId, udid);
#else
    APP_LOGW("DEVICE_MANAGER_ENABLE is false");
    return -1;
#endif
}

DistributedBundleInfo DistributedDataStorage::ConvertToDistributedBundleInfo(const BundleInfo &bundleInfo)
{
    DistributedBundleInfo distributedBundleInfo;
    distributedBundleInfo.bundleName = bundleInfo.name;
    distributedBundleInfo.versionCode = bundleInfo.versionCode;
    distributedBundleInfo.compatibleVersionCode = bundleInfo.compatibleVersion;
    distributedBundleInfo.versionName = bundleInfo.versionName;
    distributedBundleInfo.minCompatibleVersion = bundleInfo.minCompatibleVersionCode;
    distributedBundleInfo.targetVersionCode = bundleInfo.targetVersion;
    distributedBundleInfo.appId = bundleInfo.appId;
    for (const auto &hapModuleInfo : bundleInfo.hapModuleInfos) {
        DistributedModuleInfo distributedModuleInfo;
        distributedModuleInfo.moduleName = hapModuleInfo.moduleName;
        for (const auto &abilityInfo : hapModuleInfo.abilityInfos) {
            DistributedAbilityInfo distributedAbilityInfo;
            distributedAbilityInfo.abilityName = abilityInfo.name;
            distributedAbilityInfo.permissions = abilityInfo.permissions;
            distributedAbilityInfo.type = abilityInfo.type;
            distributedAbilityInfo.enabled = abilityInfo.enabled;
            distributedModuleInfo.abilities.emplace_back(distributedAbilityInfo);
        }
        distributedBundleInfo.moduleInfos.emplace_back(distributedModuleInfo);
    }
    return distributedBundleInfo;
}

bool DistributedDataStorage::QueryAllDeviceIds(std::vector<std::string> &deviceIds)
{
    std::vector<DeviceInfo> deviceInfoList;
    Status status = dataManager_.GetDeviceList(deviceInfoList, DeviceFilterStrategy::FILTER);
    if (status != Status::SUCCESS) {
        APP_LOGE("get GetDeviceList error: %{public}d", status);
        return false;
    }

    DeviceInfo localDeviceInfo;
    status = dataManager_.GetLocalDevice(localDeviceInfo);
    if (status != Status::SUCCESS) {
        APP_LOGE("get GetLocalDevice error: %{public}d", status);
        return false;
    }

    deviceInfoList.emplace_back(localDeviceInfo);
    for (auto deviceInfo : deviceInfoList) {
        deviceIds.emplace_back(deviceInfo.deviceId);
    }

    return !deviceIds.empty();
}

void DistributedDataStorage::SyncDistributedData(const std::vector<std::string> &deviceList)
{
    APP_LOGD("syncDistributedData");
    if (kvStorePtr_ == nullptr) {
        APP_LOGE("kvStorePtr_ is null");
        return;
    }
    if (deviceList.size() == 0) {
        APP_LOGE("deviceList parameter is invalid");
        return;
    }
    Status status = kvStorePtr_->Sync(deviceList, SyncMode::PUSH);
    if (status != Status::SUCCESS) {
        APP_LOGE("kvStorePtr_ Sync error: %{public}d", status);
    }
}
}  // namespace AppExecFwk
}  // namespace OHOS