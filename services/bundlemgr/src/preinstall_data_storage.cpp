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

#include "preinstall_data_storage.h"

#include <unistd.h>
#include "app_log_wrapper.h"

using namespace OHOS::DistributedKv;

namespace OHOS {
namespace AppExecFwk {
namespace {
const int32_t MAX_TIMES = 600;             // 1 min
const int32_t SLEEP_INTERVAL = 100 * 1000;  // 100ms

void DeviceAndNameToKey(
    const std::string &deviceId, const std::string &bundleName, std::string &key)
{
    key.append(deviceId);
    key.append(Constants::FILE_UNDERLINE);
    key.append(bundleName);
    APP_LOGD("bundleName = %{public}s", bundleName.c_str());
}
}  // namespace

PreInstallDataStorage::PreInstallDataStorage()
{
    APP_LOGI("PreInstall instance is created");
    TryTwice([this] { return GetKvStore(); });
}

PreInstallDataStorage::~PreInstallDataStorage()
{
    APP_LOGI("PreInstall instance is destroyed");
    dataManager_.CloseKvStore(appId_, kvStorePtr_);
}

Status PreInstallDataStorage::GetKvStore()
{
    Options options = {
        .createIfMissing = true,
        .encrypt = false,
        .autoSync = false,
        .kvStoreType = KvStoreType::SINGLE_VERSION
    };

    Status status = dataManager_.GetSingleKvStore(options, appId_, storeId_, kvStorePtr_);
    if (status != Status::SUCCESS) {
        APP_LOGE("usage get kvStore error: %{public}d", status);
    } else {
        APP_LOGI("usage get kvStore success");
    }
    return status;
}

void PreInstallDataStorage::TryTwice(const std::function<Status()> &func) const
{
    Status status = func();
    if (status == Status::IPC_ERROR) {
        status = func();
        APP_LOGW("distribute database ipc error and try to call again, result = %{public}d", status);
    }
}

bool PreInstallDataStorage::CheckKvStore()
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
        APP_LOGD("usage CheckKvStore, Times: %{public}d", tryTimes);
        usleep(SLEEP_INTERVAL);
        tryTimes--;
    }
    return kvStorePtr_ != nullptr;
}

bool PreInstallDataStorage::ResetKvStore()
{
    std::lock_guard<std::mutex> lock(kvStorePtrMutex_);
    kvStorePtr_ = nullptr;
    Status status = GetKvStore();
    if (status == Status::SUCCESS && kvStorePtr_ != nullptr) {
        return true;
    }
    APP_LOGW("usage reset failed");
    return false;
}

bool PreInstallDataStorage::SavePreInstallStorageBundleInfo(
    const std::string &deviceId, const PreInstallBundleInfo &preInstallBundleInfo)
{
    APP_LOGI("save PreInstall bundle data.");
    {
        std::lock_guard<std::mutex> lock(kvStorePtrMutex_);
        if (!CheckKvStore()) {
            APP_LOGE("kvStore is nullptr");
            return false;
        }
    }

    std::string keyOfData;
    DeviceAndNameToKey(deviceId, preInstallBundleInfo.GetBundleName(), keyOfData);
    Key key(keyOfData);
    Value value(preInstallBundleInfo.ToString());
    APP_LOGI("save PreInstallStorageBundleInfo, key: %{public}s value: %{public}s.",
        keyOfData.c_str(), preInstallBundleInfo.ToString().c_str());
    Status status;
    {
        std::lock_guard<std::mutex> lock(kvStorePtrMutex_);
        status = kvStorePtr_->Put(key, value);
        if (status == Status::IPC_ERROR) {
            status = kvStorePtr_->Put(key, value);
            APP_LOGI("distribute database ipc error and try to call again, result = %{public}d", status);
        }
    }

    if (status != Status::SUCCESS) {
        const std::string interfaceName = "kvStorePtr::Put()";
        APP_LOGI("put valLocalAbilityManager::InitializeSaProfilesue to kvStore error: %{public}d", status);
        return false;
    }
    APP_LOGI("put value to kvStore success when save PreInstall bundle data.");
    return true;
}

bool PreInstallDataStorage::GetPreInstallStorageBundleInfo(
    const std::string &deviceId, PreInstallBundleInfo &preInstallBundleInfo)
{
    APP_LOGI("Get PreInstall bundle data start.");
    {
        std::lock_guard<std::mutex> lock(kvStorePtrMutex_);
        if (!CheckKvStore()) {
            APP_LOGE("kvStore is nullptr");
            return false;
        }
    }
    std::string keyOfData;
    DeviceAndNameToKey(deviceId, preInstallBundleInfo.GetBundleName(), keyOfData);
    APP_LOGI("Get PreInstall bundle data when key is: %{public}s ", keyOfData.c_str());
    Key key(keyOfData);
    Value value;
    Status status;

    {
        std::lock_guard<std::mutex> lock(kvStorePtrMutex_);
        status = kvStorePtr_->Get(key, value);
        if (status == Status::IPC_ERROR) {
            status = kvStorePtr_->Get(key, value);
            APP_LOGI("distribute database ipc error and try to call again, result = %{public}d", status);
        }
    }

    if (status != Status::SUCCESS) {
        APP_LOGE("Get key error: %{public}d", status);
        return false;
    }

    nlohmann::json jsonObject = nlohmann::json::parse(value.ToString(), nullptr, false);
    if (jsonObject.is_discarded()) {
        APP_LOGE("error key: %{private}s", key.ToString().c_str());
        // it's an bad json, delete it
        {
            std::lock_guard<std::mutex> lock(kvStorePtrMutex_);
            kvStorePtr_->Delete(key);
        }
        return false;
    }

    if (preInstallBundleInfo.FromJson(jsonObject) != ERR_OK) {
        APP_LOGE("error key: %{private}s", key.ToString().c_str());
        // it's an error value, delete it
        {
            std::lock_guard<std::mutex> lock(kvStorePtrMutex_);
            kvStorePtr_->Delete(key);
        }
        return false;
    }

    APP_LOGI("Get value success when Get PreInstall bundle data.");
    return true;
}
}  // namespace AppExecFwk
}  // namespace OHOS
