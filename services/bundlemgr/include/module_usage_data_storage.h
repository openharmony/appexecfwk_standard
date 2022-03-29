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

#ifndef FOUNDATION_APPEXECFWK_SERVICES_BUNDLEMGR_INCLUDE_MODULE_USAGE_DATA_STORAGE_H
#define FOUNDATION_APPEXECFWK_SERVICES_BUNDLEMGR_INCLUDE_MODULE_USAGE_DATA_STORAGE_H

#include <map>
#include <mutex>

#include "bundle_constants.h"
#include "distributed_kv_data_manager.h"
#include "inner_bundle_info.h"

namespace OHOS {
namespace AppExecFwk {
class DataMgr;

class ModuleUsageRecordStorage : public std::enable_shared_from_this<ModuleUsageRecordStorage> {
public:
    ModuleUsageRecordStorage();
    virtual ~ModuleUsageRecordStorage();

    // add&update
    bool AddOrUpdateRecord(ModuleUsageRecord& data, const std::string& deviceId, int32_t userId);
    bool DeleteRecordByUserId(int32_t userId);
    bool DeleteUsageRecord(const InnerBundleInfo& data, int32_t userId);
    bool MarkUsageRecordRemoved(const InnerBundleInfo &data, int32_t userId);
    bool QueryRecordByNum(int32_t maxNum, std::vector<ModuleUsageRecord>& records, int32_t userId);
    void SetDataMgr(const std::weak_ptr<DataMgr>& dataMgr) const;
    void OnKvStoreDeath();
    void RegisterKvStoreDeathListener();
private:

    bool DeleteRecordByKeys(const std::string& bundleName, std::vector<DistributedKv::Key>& keys);
    bool QueryRecordByCondition(DistributedKv::DataQuery& query, std::vector<DistributedKv::Entry>& records);
    void AbilityRecordToKey(const std::string& userId, const std::string& deviceId,
        const std::string& bundleName, const std::string& moduleName, std::string& key) const;
    void InnerBundleInfoToKeys(const InnerBundleInfo& data, int32_t userId,
        std::vector<DistributedKv::Key>& keys) const;
    void FillDataStorageKeys(const std::string& userId, const std::string& bundleName,
        const std::string& moduleName, std::vector<DistributedKv::Key>& keys) const;
    bool ParseKey(const std::string& key, ModuleUsageRecord& record) const;
    void UpdateUsageRecord(const std::string& jsonString, ModuleUsageRecord& data);
    void SaveEntries(const std::vector<DistributedKv::Entry>& allEntries,
        std::vector<ModuleUsageRecord>& records) const;
    void TryTwice(const std::function<DistributedKv::Status()>& func) const;
    bool CheckKvStore();
    DistributedKv::Status GetKvStore();
    bool ResetKvStore();

private:
    const DistributedKv::AppId appId_ { Constants::APP_ID };
    const DistributedKv::StoreId storeId_ { Constants::ABILITY_USAGE_STORE_ID };
    DistributedKv::DistributedKvDataManager dataManager_;
    std::shared_ptr<DistributedKv::SingleKvStore> kvStorePtr_;
    mutable std::mutex kvStorePtrMutex_;
    enum {
        DATABASE_KEY_INDEX_USER_ID = 0,
        DATABASE_KEY_INDEX_DEVICE_ID,
        DATABASE_KEY_INDEX_BUNDLE_NAME,
        DATABASE_KEY_INDEX_MODULE_NAME,
        DATABASE_KEY_INDEX_MAX_LENGTH,
    };
};
} // namespace AppExecFwk
} // namespace OHOS
#endif // FOUNDATION_APPEXECFWK_SERVICES_BUNDLEMGR_INCLUDE_MODULE_USAGE_DATA_STORAGE_H