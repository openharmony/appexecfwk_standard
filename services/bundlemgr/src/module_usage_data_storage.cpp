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

#include "module_usage_data_storage.h"

#include <cinttypes>
#include <unistd.h>
#include "datetime_ex.h"
#include "permission/permission_kit.h"
#include "string_ex.h"

#include "app_log_wrapper.h"
#include "bundle_util.h"
#include "kvstore_death_recipient_callback.h"
#include "nlohmann/json.hpp"

using namespace OHOS::DistributedKv;

namespace OHOS {
namespace AppExecFwk {
namespace {
const int32_t MAX_TIMES = 6000;             // tem min
const int32_t SLEEP_INTERVAL = 100 * 1000;  // 100ms
const std::string POUND_KEY_SEPARATOR = "#";
const std::string SCHEMA_DEFINE = "{\"SCHEMA_VERSION\":\"1.0\","
                                  "\"SCHEMA_MODE\":\"COMPATIBLE\","
                                  "\"SCHEMA_SKIPSIZE\":0,"
                                  "\"SCHEMA_DEFINE\":{"
                                  "\"launchedCount\":\"INTEGER, NOT NULL\","
                                  "\"lastLaunchTime\":\"LONG, NOT NULL\","
                                  "\"isRemoved\":\"BOOL, NOT NULL\""
                                  "},"
                                  "\"SCHEMA_INDEXES\":[\"$.lastLaunchTime\"]}";
}  // namespace

ModuleUsageRecordStorage::ModuleUsageRecordStorage()
{
    APP_LOGI("usage instance is created");
    TryTwice([this] { return GetKvStore(); });
}

ModuleUsageRecordStorage::~ModuleUsageRecordStorage()
{
    APP_LOGI("usage instance is destroyed");
    dataManager_.CloseKvStore(appId_, kvStorePtr_);
}

void ModuleUsageRecordStorage::RegisterKvStoreDeathListener()
{}

bool ModuleUsageRecordStorage::ParseKey(const std::string &key, ModuleUsageRecord &record) const
{
    std::vector<std::string> splitKeys;
    SplitStr(key, POUND_KEY_SEPARATOR, splitKeys);
    if (splitKeys.size() != DATABASE_KEY_INDEX_MAX_LENGTH) {
        APP_LOGE("error key, parsed failed!");
        return false;
    }

    record.bundleName = (splitKeys[DATABASE_KEY_INDEX_BUNDLE_NAME]);
    record.name = (splitKeys[DATABASE_KEY_INDEX_MODULE_NAME]);
    APP_LOGD(
        "parseKey::bundleName = %{public}s, moduleName = %{public}s", record.bundleName.c_str(), record.name.c_str());
    return true;
}

void ModuleUsageRecordStorage::AbilityRecordToKey(const std::string &userId, const std::string &deviceId,
    const std::string &bundleName, const std::string &moduleName, std::string &key) const
{
    // deviceId_bundleName_moduleName
    key.append(userId);
    key.append(POUND_KEY_SEPARATOR);
    key.append(deviceId);
    key.append(POUND_KEY_SEPARATOR);
    key.append(bundleName);
    key.append(POUND_KEY_SEPARATOR);
    key.append(moduleName);
    APP_LOGD("userId = %{public}s, bundleName = %{public}s, moduleName = %{public}s",
        userId.c_str(),
        bundleName.c_str(),
        moduleName.c_str());
}

void ModuleUsageRecordStorage::UpdateUsageRecord(const std::string &jsonString, ModuleUsageRecord &data)
{
    nlohmann::json jsonObject = nlohmann::json::parse(jsonString);
    if (jsonObject.is_discarded()) {
        APP_LOGE("failed to parse existing usage record: %{private}s.", jsonString.c_str());
        return;
    }
    uint32_t launchedCount = jsonObject.at(UsageRecordKey::LAUNCHED_COUNT).get<int32_t>();
    data.launchedCount = launchedCount + 1;
    APP_LOGD("launchedCount = %{public}d", data.launchedCount);
    return;
}

bool ModuleUsageRecordStorage::AddOrUpdateRecord(ModuleUsageRecord &data, const std::string &deviceId, int32_t userId)
{
    APP_LOGI("add usage record data %{public}s", data.bundleName.c_str());
    {
        std::lock_guard<std::mutex> lock(kvStorePtrMutex_);
        if (!CheckKvStore()) {
            APP_LOGE("kvStore is nullptr");
            return false;
        }
    }
    Status status;
    bool isExist = false;
    {
        std::string keyOfData;
        AbilityRecordToKey(std::to_string(userId), deviceId, data.bundleName, data.name, keyOfData);
        Key key(keyOfData);
        Value oldValue;
        std::lock_guard<std::mutex> lock(kvStorePtrMutex_);
        status = kvStorePtr_->Get(key, oldValue);
        if (status == Status::SUCCESS) {
            APP_LOGD("get old value %{public}s", oldValue.ToString().c_str());
            // already isExist, update
            UpdateUsageRecord(oldValue.ToString(), data);
            isExist = true;
        }
        if (status == Status::KEY_NOT_FOUND || isExist) {
            Value value(data.ToString());
            APP_LOGD("add to DB::value %{public}s", value.ToString().c_str());
            status = kvStorePtr_->Put(key, value);
            APP_LOGW("add result = %{public}d", status);
            if (status == Status::IPC_ERROR) {
                status = kvStorePtr_->Put(key, value);
                APP_LOGW("distribute database ipc error and try to call again, result = %{public}d", status);
            }
        }
    }

    if (status != Status::SUCCESS) {
        APP_LOGE("put value to kvStore error: %{public}d", status);
        return false;
    }

    APP_LOGD("update success");
    return true;
}

bool ModuleUsageRecordStorage::DeleteRecordByKeys(const std::string &bundleName, std::vector<DistributedKv::Key> &keys)
{
    if (keys.size() == 0) {
        APP_LOGE("delete error: empty key");
        return false;
    }
    Status status;
    {
        std::lock_guard<std::mutex> lock(kvStorePtrMutex_);
        status = kvStorePtr_->DeleteBatch(keys);
        if (status == Status::IPC_ERROR) {
            status = kvStorePtr_->DeleteBatch(keys);
            APP_LOGW("distribute database ipc error and try to call again, result = %{public}d", status);
        }
    }

    if (status != Status::SUCCESS) {
        APP_LOGE("delete keys error: %{public}d", status);
        return false;
    }
    APP_LOGD("delete success");
    return true;
}

bool ModuleUsageRecordStorage::DeleteUsageRecord(const InnerBundleInfo &data, int32_t userId)
{
    APP_LOGD("delete usage data");
    {
        std::lock_guard<std::mutex> lock(kvStorePtrMutex_);
        if (!CheckKvStore()) {
            APP_LOGE("kvStore is nullptr");
            return false;
        }
    }

    std::vector<Key> keys;
    InnerBundleInfoToKeys(data, userId, keys);
    APP_LOGD("delete key %{public}zu", keys.size());
    return DeleteRecordByKeys(data.GetBundleName(), keys);
}

bool ModuleUsageRecordStorage::MarkUsageRecordRemoved(const InnerBundleInfo &data, int32_t userId)
{
    {
        std::lock_guard<std::mutex> lock(kvStorePtrMutex_);
        if (!CheckKvStore()) {
            APP_LOGE("kvStore is nullptr");
            return false;
        }
    }
    ModuleUsageRecord record;
    Value value;
    std::string jsonString;
    std::vector<Key> keys;
    InnerBundleInfoToKeys(data, userId, keys);
    {
        std::lock_guard<std::mutex> lock(kvStorePtrMutex_);
        for (const Key &key : keys) {
            Status status = kvStorePtr_->Get(key, value);
            if (status != Status::SUCCESS) {
                APP_LOGD("database query by key error, result = %{public}d", status);
                return false;
            }
            if (!record.FromJsonString(value.ToString())) {
                APP_LOGW("database parse entry failed");
                return false;
            }
            if (!record.removed) {
                record.removed = true;
                jsonString = record.ToString();
                APP_LOGD("new value %{public}s", jsonString.c_str());
                value = jsonString;
                status = kvStorePtr_->Put(key, value);
                APP_LOGI("value update result: %{public}d", status);
            }
        }
    }
    return true;
}

void ModuleUsageRecordStorage::InnerBundleInfoToKeys(
    const InnerBundleInfo &data, int32_t userId, std::vector<DistributedKv::Key> &keys) const
{
    std::vector<std::string> mouduleNames;
    data.GetModuleNames(mouduleNames);
    const std::string &bundleName = data.GetBundleName();
    for (const auto &moduleName : mouduleNames) {
        FillDataStorageKeys(std::to_string(userId), bundleName, moduleName, keys);
    }
}

void ModuleUsageRecordStorage::FillDataStorageKeys(const std::string &userId, const std::string &bundleName,
    const std::string &moduleName, std::vector<DistributedKv::Key> &keys) const
{
    std::string keyOfData;
    AbilityRecordToKey(userId, Constants::CURRENT_DEVICE_ID, bundleName, moduleName, keyOfData);
    Key key(keyOfData);
    keys.push_back(key);
}

void ModuleUsageRecordStorage::SaveEntries(
    const std::vector<Entry> &allEntries, std::vector<ModuleUsageRecord> &records) const
{
    APP_LOGD("SaveEntries %{public}zu", allEntries.size());
    for (const auto &item : allEntries) {
        APP_LOGD("SaveEntries %{public}s", item.value.ToString().c_str());
        ModuleUsageRecord record;
        if (!record.FromJsonString(item.value.ToString())) {
            APP_LOGE("error entry: %{private}s", item.value.ToString().c_str());
            continue;
        }

        if (!ParseKey(item.key.ToString(), record)) {
            APP_LOGE("error key");
            continue;
        }
        records.emplace_back(record);
    }
}

bool ModuleUsageRecordStorage::QueryRecordByNum(int32_t maxNum, std::vector<ModuleUsageRecord> &records, int32_t userId)
{
    APP_LOGI("query record by num %{public}d userId %{public}d", maxNum, userId);
    DataQuery query;
    query.KeyPrefix(std::to_string(userId) + POUND_KEY_SEPARATOR + Constants::CURRENT_DEVICE_ID + POUND_KEY_SEPARATOR);
    query.OrderByDesc(UsageRecordKey::SCHEMA_LAST_LAUNCH_TIME);
    query.Limit(maxNum, 0);
    std::vector<Entry> allEntries;
    bool queryResult = QueryRecordByCondition(query, allEntries);
    if (!queryResult || static_cast<int>(allEntries.size()) > maxNum) {
        APP_LOGE("query record error");
        return queryResult;
    }
    APP_LOGD("query record success");
    SaveEntries(allEntries, records);
    return true;
}

bool ModuleUsageRecordStorage::QueryRecordByCondition(DataQuery &query, std::vector<Entry> &records)
{
    Status status;
    {
        std::lock_guard<std::mutex> lock(kvStorePtrMutex_);
        if (!CheckKvStore()) {
            APP_LOGE("kvStore is nullptr");
            return false;
        }
    }

    status = kvStorePtr_->GetEntriesWithQuery(query, records);
    APP_LOGI("query record by condition %{public}d", status);
    if (status == Status::IPC_ERROR) {
        status = kvStorePtr_->GetEntriesWithQuery(query, records);
        APP_LOGW("distribute database ipc error and try to call again, result = %{public}d", status);
    }

    if (status != Status::SUCCESS) {
        APP_LOGE("query key error: %{public}d", status);
        return false;
    }
    return true;
}

Status ModuleUsageRecordStorage::GetKvStore()
{
    Options options = {
        .createIfMissing = true, 
        .encrypt = false, 
        .autoSync = true, 
        .kvStoreType = KvStoreType::SINGLE_VERSION
    };

    options.schema = SCHEMA_DEFINE;
    Status status = dataManager_.GetSingleKvStore(options, appId_, storeId_, kvStorePtr_);
    if (status != Status::SUCCESS) {
        APP_LOGE("usage get kvStore error: %{public}d", status);
    } else {
        APP_LOGI("usage get kvStore success");
    }
    return status;
}

void ModuleUsageRecordStorage::TryTwice(const std::function<Status()> &func) const
{
    Status status = func();
    if (status == Status::IPC_ERROR) {
        status = func();
        APP_LOGW("distribute database ipc error and try to call again, result = %{public}d", status);
    }
}

bool ModuleUsageRecordStorage::CheckKvStore()
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

bool ModuleUsageRecordStorage::ResetKvStore()
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

}  // namespace AppExecFwk
}  // namespace OHOS
