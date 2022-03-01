/*
 * Copyright (c) 2021-2022 Huawei Device Co., Ltd.
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

#include "bundle_data_storage_database.h"

#include <unistd.h>

#include "app_log_wrapper.h"
#include "bundle_exception_handler.h"
#include "kvstore_death_recipient_callback.h"

using namespace OHOS::DistributedKv;

namespace OHOS {
namespace AppExecFwk {
namespace {
const int32_t MAX_TIMES = 600;              // 1min
const int32_t SLEEP_INTERVAL = 100 * 1000;  // 100ms
}  // namespace

BundleDataStorageDatabase::BundleDataStorageDatabase()
{
    APP_LOGI("instance:%{private}p is created", this);
    TryTwice([this] { return GetKvStore(); });
    RegisterKvStoreDeathListener();
}

BundleDataStorageDatabase::~BundleDataStorageDatabase()
{
    APP_LOGI("instance:%{private}p is destroyed", this);
    dataManager_.CloseKvStore(appId_, kvStorePtr_);
}

bool BundleDataStorageDatabase::KeyToDeviceAndName(
    const std::string &key, std::string &deviceId, std::string &bundleName) const
{
    const size_t underlinePos = key.find_first_of(Constants::FILE_UNDERLINE);
    if (underlinePos == std::string::npos) {
        APP_LOGW("invalid key : %{private}s", key.c_str());
        return false;
    }
    deviceId = key.substr(0, underlinePos);
    bundleName = key.substr(underlinePos + 1);
    return true;
}

void BundleDataStorageDatabase::DeviceAndNameToKey(
    const std::string &deviceId, const std::string &bundleName, std::string &key) const
{
    key.append(deviceId);
    key.append(Constants::FILE_UNDERLINE);
    key.append(bundleName);
    APP_LOGD("bundleName = %{public}s", bundleName.c_str());
}

void BundleDataStorageDatabase::SaveEntries(
    const std::vector<Entry> &allEntries, std::map<std::string, std::map<std::string, InnerBundleInfo>> &infos)
{
    for (const auto &item : allEntries) {
        std::string bundleName;
        std::string deviceId;
        InnerBundleInfo innerBundleInfo;
        if (!KeyToDeviceAndName(item.key.ToString(), deviceId, bundleName)) {
            APP_LOGE("error key: %{private}s", item.key.ToString().c_str());
            // it's an error key, delete it
            {
                std::lock_guard<std::mutex> lock(kvStorePtrMutex_);
                kvStorePtr_->Delete(item.key);
            }
            continue;
        }

        nlohmann::json jsonObject = nlohmann::json::parse(item.value.ToString(), nullptr, false);
        if (jsonObject.is_discarded()) {
            APP_LOGE("error key: %{private}s", item.key.ToString().c_str());
            // it's an bad json, delete it
            {
                std::lock_guard<std::mutex> lock(kvStorePtrMutex_);
                kvStorePtr_->Delete(item.key);
            }
            continue;
        }
        if (innerBundleInfo.FromJson(jsonObject) != ERR_OK) {
            APP_LOGE("error key: %{private}s", item.key.ToString().c_str());
            // it's an error value, delete it
            {
                std::lock_guard<std::mutex> lock(kvStorePtrMutex_);
                kvStorePtr_->Delete(item.key);
            }
            continue;
        }
        bool isBundleValid = true;
        auto handler = std::make_shared<BundleExceptionHandler>(shared_from_this());
        handler->HandleInvalidBundle(innerBundleInfo, isBundleValid);
        if (!isBundleValid) {
            continue;
        }
        auto allDevicesInfosIter = infos.find(bundleName);
        if (allDevicesInfosIter != infos.end()) {
            APP_LOGD("already have bundle: %{public}s", bundleName.c_str());
            allDevicesInfosIter->second.emplace(deviceId, innerBundleInfo);
        } else {
            APP_LOGD("emplace bundle: %{public}s", bundleName.c_str());
            std::map<std::string, InnerBundleInfo> allDevicesInfos;
            allDevicesInfos.emplace(deviceId, innerBundleInfo);
            infos.emplace(bundleName, allDevicesInfos);
        }
    }
    APP_LOGD("SaveEntries end");
}

bool BundleDataStorageDatabase::LoadAllData(std::map<std::string, std::map<std::string, InnerBundleInfo>> &infos)
{
    APP_LOGI("load all installed bundle data to map");
    {
        std::lock_guard<std::mutex> lock(kvStorePtrMutex_);
        if (!CheckKvStore()) {
            APP_LOGE("kvStore is nullptr");
            return false;
        }
    }
    Status status;
    std::vector<Entry> allEntries;
    TryTwice([this, &status, &allEntries] {
        status = GetEntries(allEntries);
        return status;
    });

    bool ret = true;
    if (status != Status::SUCCESS) {
        APP_LOGE("get entries error: %{public}d", status);
        // KEY_NOT_FOUND means no data in database, no need to report.
        if (status != Status::KEY_NOT_FOUND) {
            const std::string interfaceName = "KvStoreSnapshot::GetEntries()";
        }
        ret = false;
    } else {
        SaveEntries(allEntries, infos);
    }
    return ret;
}

bool BundleDataStorageDatabase::SaveStorageBundleInfo(
    const std::string &deviceId, const InnerBundleInfo &innerBundleInfo)
{
    APP_LOGI("save bundle data");
    {
        std::lock_guard<std::mutex> lock(kvStorePtrMutex_);
        if (!CheckKvStore()) {
            APP_LOGE("kvStore is nullptr");
            return false;
        }
    }

    std::string keyOfData;
    DeviceAndNameToKey(deviceId, innerBundleInfo.GetBundleName(), keyOfData);
    Key key(keyOfData);
    Value value(innerBundleInfo.ToString());
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
        const std::string interfaceName = "kvStorePtr::Put()";
        APP_LOGE("put valLocalAbilityManager::InitializeSaProfilesue to kvStore error: %{public}d", status);
        return false;
    }

    APP_LOGI("put value to kvStore success");
    return true;
}

bool BundleDataStorageDatabase::DeleteStorageBundleInfo(
    const std::string &deviceId, const InnerBundleInfo &innerBundleInfo)
{
    APP_LOGI("delete bundle data");
    {
        std::lock_guard<std::mutex> lock(kvStorePtrMutex_);
        if (!CheckKvStore()) {
            APP_LOGE("kvStore is nullptr");
            return false;
        }
    }
    std::string keyOfData;
    DeviceAndNameToKey(deviceId, innerBundleInfo.GetBundleName(), keyOfData);
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
        const std::string interfaceName = "kvStorePtr::Delete()";
        APP_LOGE("delete key error: %{public}d", status);
        return false;
    } else {
        APP_LOGI("delete value to kvStore success");
    }
    return true;
}

void BundleDataStorageDatabase::RegisterKvStoreDeathListener()
{
    APP_LOGI("register kvStore death listener");
    std::shared_ptr<DistributedKv::KvStoreDeathRecipient> callback = std::make_shared<KvStoreDeathRecipientCallback>();
    dataManager_.RegisterKvStoreServiceDeathRecipient(callback);
}

bool BundleDataStorageDatabase::CheckKvStore()
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
        APP_LOGD("CheckKvStore, Times: %{public}d", tryTimes);
        usleep(SLEEP_INTERVAL);
        tryTimes--;
    }
    return kvStorePtr_ != nullptr;
}

Status BundleDataStorageDatabase::GetKvStore()
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

Status BundleDataStorageDatabase::GetEntries(std::vector<Entry> &allEntries) const
{
    Status status = Status::ERROR;
    Key token;
    // if prefix is empty, get all entries.
    Key allEntryKeyPrefix("");
    if (kvStorePtr_) {
        // sync call GetEntries, the callback will be trigger at once
        status = kvStorePtr_->GetEntries(allEntryKeyPrefix, allEntries);
    }
    APP_LOGI("get all entries status: %{public}d", status);
    return status;
}

void BundleDataStorageDatabase::TryTwice(const std::function<Status()> &func) const
{
    Status status = func();
    if (status == Status::IPC_ERROR) {
        status = func();
        APP_LOGW("distribute database ipc error and try to call again, result = %{public}d", status);
    }
}

bool BundleDataStorageDatabase::ResetKvStore()
{
    std::lock_guard<std::mutex> lock(kvStorePtrMutex_);
    kvStorePtr_ = nullptr;
    Status status = GetKvStore();
    if (status == Status::SUCCESS && kvStorePtr_ != nullptr) {
        return true;
    }
    APP_LOGW("failed");
    return false;
}
}  // namespace AppExecFwk
}  // namespace OHOS