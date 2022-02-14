/*
 * Copyright (c) 2021 Huawei Device Co., Ltd.
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

#include "bundle_util.h"

using namespace OHOS::DistributedKv;

namespace OHOS {
namespace AppExecFwk {
namespace {
const int32_t MAX_TIMES = 600;              // 1min
const int32_t SLEEP_INTERVAL = 100 * 1000;  // 100ms
}  // namespace

DistributedDataStorage::DistributedDataStorage()
{
    APP_LOGI("instance:%{private}p is created", this);
    TryTwice([this] { return GetKvStore(); });
}

DistributedDataStorage::~DistributedDataStorage()
{
    APP_LOGI("instance:%{private}p is destroyed", this);
    dataManager_.CloseKvStore(appId_, storeId_);
}

bool DistributedDataStorage::SaveStorageDistributeInfo(const DistributedBundleInfo &info)
{
    APP_LOGI("save DistributedBundleInfo data");
    {
        std::lock_guard<std::mutex> lock(kvStorePtrMutex_);
        if (!CheckKvStore()) {
            APP_LOGE("kvStore is nullptr");
            return false;
        }
    }
    std::string keyOfData;
    DeviceAndNameToKey(BundleUtil::GetCurrentDeviceId(), info.name, keyOfData);
    Key key(keyOfData);
    Value value(info.ToString());
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
    std::string keyOfData;
    DeviceAndNameToKey(BundleUtil::GetCurrentDeviceId(), bundleName, keyOfData);
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

bool DistributedDataStorage::GetDistributeInfoByUserId(
    int32_t userId, const DistributedKv::Key &key, const Value &value, DistributedBundleInfo &info)
{
    if (!info.FromJsonString(value.ToString())) {
        APP_LOGE("it's an error value");
        kvStorePtr_->Delete(key);
        return false;
    }
    if (userId == Constants::ALL_USERID) {
        return true;
    }
    if (userId >= Constants::DEFAULT_USERID) {
        for (auto it = info.bundleUserInfos.begin(); it != info.bundleUserInfos.end();) {
            if (it->userId != userId) {
                it = info.bundleUserInfos.erase(it);
            } else {
                ++it;
            }
        }
    }
    return true;
}

bool DistributedDataStorage::QueryStroageDistributeInfo(
    const std::string &bundleName, int32_t userId, const std::string &networkId,
    DistributedBundleInfo &info)
{
    APP_LOGI("query DistributedBundleInfo");
    std::string deviceId;
    bool ret = true;
    ret = GetDeviceIdByNetworkId(networkId, deviceId);
    if (!ret) {
        APP_LOGI("can not query networkId online");
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
    DeviceAndNameToKey(deviceId, bundleName, keyOfData);
    APP_LOGI("keyOfData: [%{public}s]", keyOfData.c_str());
    Key key(keyOfData);
    Value value;
    Status status;
    {
        std::lock_guard<std::mutex> lock(kvStorePtrMutex_);
        status = kvStorePtr_->Get(key, value);
        if (status == Status::SUCCESS) {
            if (!GetDistributeInfoByUserId(userId, key, value, info)) {
                return false;
            }
            return true;
        }
        APP_LOGI("get value status: %{public}d", status);
        else if (status == Status::IPC_ERROR) {
            status = kvStorePtr_->Get(key, value);
            APP_LOGW("distribute database ipc error and try to call again, result = %{public}d", status);
            if (status == Status::SUCCESS) {
                if (!GetDistributeInfoByUserId(userId, key, value, info)) {
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
    const std::string &deviceId, const std::string &bundleName, std::string &key) const
{
    key.append(deviceId);
    key.append(Constants::FILE_UNDERLINE);
    key.append(bundleName);
}

bool DistributedDataStorage::GetDeviceIdByNetworkId(const std::string &networkId, std::string &deviceId)
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
    auto it = std::find_if(std::begin(deviceInfoList), std::end(deviceInfoList),
        [&networkId](const auto &item) { return item.deviceId == networkId; });
    if (it == std::end(deviceInfoList)) {
        return false;
    }
    deviceId = it->deviceName;
    return true;
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
        .autoSync = true,
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

bool DistributedDataStorage::SetDeviceId()
{
    Status status;
    DeviceInfo deviceInfo;
    status = dataManager_.GetLocalDevice(deviceInfo);
    if (status == Status::SUCCESS) {
        APP_LOGD("set Device Id: %{public}s", deviceInfo.deviceName.c_str());
        BundleUtil::SetCurrentDeviceId(deviceInfo.deviceName);
        return true;
    }
    return false;
}
}  // namespace AppExecFwk
}  // namespace OHOS