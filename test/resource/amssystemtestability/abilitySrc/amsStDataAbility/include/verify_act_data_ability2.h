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
#ifndef VERIFY_ACT_DATA_ABILITY2_H
#define VERIFY_ACT_DATA_ABILITY2_H
#include "ability.h"
#include "rdb_helper.h"
#include "rdb_open_callback.h"

namespace OHOS {
namespace NativeRdb {
class AbsSharedResultSet;
class DataAbilityPredicates;
class ValuesBucket;
}  // namespace NativeRdb
namespace AppExecFwk {
class VerifyActDataAbility2 : public Ability {
public:
    virtual void OnStart(const Want &want) override;
    virtual int Insert(const Uri &uri, const NativeRdb::ValuesBucket &value) override;
    virtual std::shared_ptr<NativeRdb::AbsSharedResultSet> Query(const Uri &uri,
        const std::vector<std::string> &columns, const NativeRdb::DataAbilityPredicates &predicates) override;
    virtual int Update(const Uri &uri, const NativeRdb::ValuesBucket &value,
        const NativeRdb::DataAbilityPredicates &predicates) override;
    virtual int Delete(const Uri &uri, const NativeRdb::DataAbilityPredicates &predicates) override;

    virtual std::string GetType(const Uri &uri) override;
    virtual int OpenFile(const Uri &uri, const std::string &mode) override;

    virtual int BatchInsert(const Uri &uri, const std::vector<NativeRdb::ValuesBucket> &values) override;

    std::vector<std::shared_ptr<NativeRdb::AbsSharedResultSet>> sharedList_;
};

class InsertTestOpenCallback2 : public NativeRdb::RdbOpenCallback {
public:
    int OnCreate(NativeRdb::RdbStore &rdbStore) override;
    int OnUpgrade(NativeRdb::RdbStore &rdbStore, int oldVersion, int newVersion) override;
    static const std::string CREATE_TABLE_TEST;
};
}  // namespace AppExecFwk
}  // namespace OHOS
#endif  // VERIFY_ACT_DATA_ABILITY2_H