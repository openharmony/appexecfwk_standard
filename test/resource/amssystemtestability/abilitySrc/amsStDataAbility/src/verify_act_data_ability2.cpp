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

#include "abs_shared_result_set.h"
#include "data_ability_predicates.h"
#include "values_bucket.h"

#include "app_log_wrapper.h"
#include "ability_loader.h"
#include "verify_act_data_ability2.h"

#include "rdb_helper.h"
#include "rdb_open_callback.h"

namespace OHOS {
namespace AppExecFwk {
namespace {
static std::shared_ptr<NativeRdb::RdbStore> testStore;
static const std::string RDB_TEST_PATH = "/data/test/";
static const std::string DATABASE_FILE_NAME = "insert_test.db";
static const std::string DATABASE_NAME = RDB_TEST_PATH + "insert_test.db";
static const int defaultReturn = 1;
}

class InsertTestOpenCallback2 : public NativeRdb::RdbOpenCallback {
public:
    int OnCreate(NativeRdb::RdbStore &rdbStore) override;
    int OnUpgrade(NativeRdb::RdbStore &rdbStore, int oldVersion, int newVersion) override;
    static const std::string CREATE_TABLE_TEST;
};

const std::string InsertTestOpenCallback2::CREATE_TABLE_TEST =
    std::string("CREATE TABLE IF NOT EXISTS test ") + std::string("(id INTEGER PRIMARY KEY AUTOINCREMENT, "
                                                                  "name TEXT NOT NULL, age INTEGER, salary "
                                                                  "REAL, blobType BLOB)");

int InsertTestOpenCallback2::OnCreate(NativeRdb::RdbStore &store)
{
    return store.ExecuteSql(CREATE_TABLE_TEST);
}

int InsertTestOpenCallback2::OnUpgrade(NativeRdb::RdbStore &store, int oldVersion, int newVersion)
{
    return 0;
}

void VerifyActDataAbility2::OnStart(const Want &want)
{
    APP_LOGI("VerifyActDataAbility2 OnStart");
    sharedList_.clear();
    Ability::OnStart(want);
}

int VerifyActDataAbility2::Insert(const Uri &uri, const NativeRdb::ValuesBucket &value)
{
    APP_LOGI("VerifyActDataAbility2 <<<<Insert>>>>");
    return defaultReturn;
}

std::shared_ptr<NativeRdb::AbsSharedResultSet> VerifyActDataAbility2::Query(
    const Uri &uri, const std::vector<std::string> &columns, const NativeRdb::DataAbilityPredicates &predicates)
{
    int errCode = 0;
    std::string dbDir = GetDatabaseDir();
    NativeRdb::RdbStoreConfig config(dbDir + "/" + DATABASE_FILE_NAME);
    InsertTestOpenCallback2 helper;
    testStore = NativeRdb::RdbHelper::GetRdbStore(config, 1, helper, errCode);
    APP_LOGI("VerifyActDataAbility2 <<<<Query>>>> Patch %{public}s", (dbDir + "/" + DATABASE_FILE_NAME).c_str());

    int64_t id;
    NativeRdb::ValuesBucket values;

    values.PutInt("id", 1);
    values.PutString("name", std::string("zhangsan"));
    values.PutInt("age", 18);
    values.PutDouble("salary", 100.5);
    values.PutBlob("blobType", std::vector<uint8_t>{1, 2, 3});

    if (testStore == nullptr) {
        APP_LOGI("VerifyActDataAbility2 <<<<Query>>>> testStore is nullptr, data will be empty");
        return nullptr;
    }

    if (testStore->Replace(id, "test", values) != 0) {
        APP_LOGE("VerifyActDataAbility2 <<<<Query>>>> store->Replace Error");
        return nullptr;
    }

    std::unique_ptr<NativeRdb::AbsSharedResultSet> rresultSet =
        testStore->QuerySql("SELECT * FROM test WHERE name = ?", std::vector<std::string>{"zhangsan"});
    if (rresultSet == nullptr) {
        APP_LOGE("VerifyActDataAbility2 <<<<Query>>>> rresultSet is nullptr");
        return nullptr;
    }

    APP_LOGI("VerifyActDataAbility2 <<<<Query>>>> Marshalling Object");
    std::shared_ptr<NativeRdb::AbsSharedResultSet> ret(rresultSet.release());
    sharedList_.push_back(ret);
    return ret;
}

int VerifyActDataAbility2::Update(
    const Uri &uri, const NativeRdb::ValuesBucket &value, const NativeRdb::DataAbilityPredicates &predicates)
{
    APP_LOGI("VerifyActDataAbility2 <<<<Update>>>>");
    return defaultReturn;
}
int VerifyActDataAbility2::Delete(const Uri &uri, const NativeRdb::DataAbilityPredicates &predicates)
{
    APP_LOGI("VerifyActDataAbility2 <<<<Delete>>>>");
    return defaultReturn;
}
std::string VerifyActDataAbility2::GetType(const Uri &uri)
{
    APP_LOGI("VerifyActDataAbility2 <<<<GetType>>>>");
    std::string retval(uri.ToString());
    return retval;
}
int VerifyActDataAbility2::OpenFile(const Uri &uri, const std::string &mode)
{
    APP_LOGI("VerifyActDataAbility2 <<<<OpenFile>>>>");
    return 0;
}
int VerifyActDataAbility2::BatchInsert(const Uri &uri, const std::vector<NativeRdb::ValuesBucket> &values)
{
    APP_LOGI("VerifyActDataAbility2 <<<<BatchInsert>>>>");
    return Ability::BatchInsert(uri, values);
}

REGISTER_AA(VerifyActDataAbility2);
}  // namespace AppExecFwk
}  // namespace OHOS
