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

// #include <chrono>
#include <gtest/gtest.h>
#include <string>
#include <thread>
#include "app_log_wrapper.h"
#define private public
#include "form_db_cache.h"
#include "form_mgr_adapter.h"
#include "form_storage_mgr.h"
#undef private
#include "form_record.h"

using namespace testing::ext;
using namespace OHOS;
using namespace OHOS::AppExecFwk;

namespace {
class FmsFormDbRecordTest : public testing::Test {
public:
    void InitFormRecord();
    FormRecord formRecord_;
    FormMgrAdapter formMgrAdapter_;
};

void FmsFormDbRecordTest::InitFormRecord()
{
    formRecord_.isInited = false;
    formRecord_.needFreeInstall = false;
    formRecord_.versionUpgrade = false;
    formRecord_.needRefresh = false;
    formRecord_.packageName = "TestPackageName";
    formRecord_.bundleName = "TestBundleName";
    formRecord_.moduleName = "TestModuleName";
    formRecord_.abilityName = "TestAbilityName";
    formRecord_.formName = "TestFormName";
    formRecord_.specification = 0;
    formRecord_.isEnableUpdate = false;
    formRecord_.updateDuration = 0;
    formRecord_.updateAtHour = 0;
    formRecord_.updateAtMin = 0;
    formRecord_.hapSourceDirs.emplace_back("hapSourceDirs1");
    formRecord_.formName = "formNameTest";
    formRecord_.formTempFlg = false;
    formRecord_.formUserUids.emplace_back(1);
    formRecord_.formVisibleNotify = false;
    formRecord_.formVisibleNotifyState = 0;
}

HWTEST_F(FmsFormDbRecordTest, FmsFormDbRecordTest_001, TestSize.Level0) // create
{
    GTEST_LOG_(INFO) << "FmsFormDbRecordTest_001 start";
    FormDbCache::GetInstance().Start();
    GTEST_LOG_(INFO) << "FmsFormDbRecordTest_001 end";
}

HWTEST_F(FmsFormDbRecordTest, FmsFormDbRecordTest_002, TestSize.Level0) // save formId 0, callIds[1]
{
    GTEST_LOG_(INFO) << "FmsFormDbRecordTest_002 start";
    InitFormRecord();
    EXPECT_EQ(ERR_OK, FormDbCache::GetInstance().UpdateDBRecord(0, formRecord_));
    GTEST_LOG_(INFO) << "FmsFormDbRecordTest_002 end";
}

HWTEST_F(FmsFormDbRecordTest, FmsFormDbRecordTest_003, TestSize.Level0) // save formId 1, callIds[1, 0]
{
    GTEST_LOG_(INFO) << "FmsFormDbRecordTest_003 start";
    InitFormRecord();
    formRecord_.formUserUids.emplace_back(0);
    EXPECT_EQ(ERR_OK, FormDbCache::GetInstance().UpdateDBRecord(1, formRecord_));
    GTEST_LOG_(INFO) << "FmsFormDbRecordTest_003 end";
}

HWTEST_F(FmsFormDbRecordTest, FmsFormDbRecordTest_004, TestSize.Level0) // modify formId 1, callIds[1,2]
{
    GTEST_LOG_(INFO) << "FmsFormDbRecordTest_004 start";
    InitFormRecord();
    formRecord_.formUserUids.emplace_back(2);
    EXPECT_EQ(ERR_OK, FormDbCache::GetInstance().UpdateDBRecord(1, formRecord_));
    GTEST_LOG_(INFO) << "FmsFormDbRecordTest_004 end";
}

HWTEST_F(FmsFormDbRecordTest, FmsFormDbRecordTest_005, TestSize.Level0) // modify formId 0, callIds[1,2]
{
    GTEST_LOG_(INFO) << "FmsFormDbRecordTest_005 start";
    InitFormRecord();
    formRecord_.formUserUids.emplace_back(2);
    EXPECT_EQ(ERR_OK, FormDbCache::GetInstance().UpdateDBRecord(0, formRecord_));
    GTEST_LOG_(INFO) << "FmsFormDbRecordTest_005 end";
}

HWTEST_F(FmsFormDbRecordTest, FmsFormDbRecordTest_006, TestSize.Level0) // GetAllDBRecord
{
    GTEST_LOG_(INFO) << "FmsFormDbRecordTest_006 start";
    FormRecord record;
    EXPECT_EQ(ERR_OK, FormDbCache::GetInstance().GetDBRecord(0, record));

    GTEST_LOG_(INFO) << "FmsFormDbRecordTest_006 formName: " << record.formName;
    GTEST_LOG_(INFO) << "FmsFormDbRecordTest_006 bundleName:" << record.bundleName;
    GTEST_LOG_(INFO) << "FmsFormDbRecordTest_006 moduleName:" << record.moduleName;
    GTEST_LOG_(INFO) << "FmsFormDbRecordTest_006 abilityName:" << record.abilityName;
    for (unsigned int j = 0; j < record.formUserUids.size(); j++){
        GTEST_LOG_(INFO) << "FmsFormDbRecordTest_006 formUserUids:" << record.formUserUids[j];
    }

    GTEST_LOG_(INFO) << "FmsFormDbRecordTest_006 -------------------";

    EXPECT_EQ(ERR_OK, FormDbCache::GetInstance().GetDBRecord(1, record));
    GTEST_LOG_(INFO) << "FmsFormDbRecordTest_006 formName: " << record.formName;
    GTEST_LOG_(INFO) << "FmsFormDbRecordTest_006 bundleName:" << record.bundleName;
    GTEST_LOG_(INFO) << "FmsFormDbRecordTest_006 moduleName:" << record.moduleName;
    GTEST_LOG_(INFO) << "FmsFormDbRecordTest_006 abilityName:" << record.abilityName;
    for (unsigned int j = 0; j < record.formUserUids.size(); j++){
        GTEST_LOG_(INFO) << "FmsFormDbRecordTest_006 formUserUids:" << record.formUserUids[j];
    }

    GTEST_LOG_(INFO) << "FmsFormDbRecordTest_006 end";
}

HWTEST_F(FmsFormDbRecordTest, FmsFormDbRecordTest_007, TestSize.Level0) // save for next load: formId 0, callIds[1]
{
    GTEST_LOG_(INFO) << "FmsFormDbRecordTest_007 start";

    InitFormRecord();
    EXPECT_EQ(ERR_OK, FormDbCache::GetInstance().UpdateDBRecord(0, formRecord_));
    std::this_thread::sleep_for(std::chrono::milliseconds(100));

    GTEST_LOG_(INFO) << "FmsFormDbRecordTest_007 end";
}

HWTEST_F(FmsFormDbRecordTest, FmsFormDbRecordTest_008, TestSize.Level0) // save for next load: formId 1, callIds[1,0]
{
    GTEST_LOG_(INFO) << "FmsFormDbRecordTest_008 start";

    InitFormRecord();
    formRecord_.formUserUids.emplace_back(0);
    EXPECT_EQ(ERR_OK, FormDbCache::GetInstance().UpdateDBRecord(1, formRecord_));
    GTEST_LOG_(INFO) << "FmsFormDbRecordTest_008 end";
}

HWTEST_F(FmsFormDbRecordTest, FmsFormDbRecordTest_009, TestSize.Level0) // GetAllDBRecord
{
    GTEST_LOG_(INFO) << "FmsFormDbRecordTest_009 start";

    FormRecord record;
    EXPECT_EQ(ERR_OK, FormDbCache::GetInstance().GetDBRecord(0, record));
    GTEST_LOG_(INFO) << "FmsFormDbRecordTest_009 formName: " << record.formName;
    GTEST_LOG_(INFO) << "FmsFormDbRecordTest_009 bundleName:" << record.bundleName;
    GTEST_LOG_(INFO) << "FmsFormDbRecordTest_009 moduleName:" << record.moduleName;
    GTEST_LOG_(INFO) << "FmsFormDbRecordTest_009 abilityName:" << record.abilityName;
    for (unsigned int j = 0; j < record.formUserUids.size(); j++){
        GTEST_LOG_(INFO) << "FmsFormDbRecordTest_009 formUserUids:" << record.formUserUids[j];
    }

    GTEST_LOG_(INFO) << "FmsFormDbRecordTest_009 -------------------";

    EXPECT_EQ(ERR_OK, FormDbCache::GetInstance().GetDBRecord(1, record));
    GTEST_LOG_(INFO) << "FmsFormDbRecordTest_009 formName: " << record.formName;
    GTEST_LOG_(INFO) << "FmsFormDbRecordTest_009 bundleName:" << record.bundleName;
    GTEST_LOG_(INFO) << "FmsFormDbRecordTest_009 moduleName:" << record.moduleName;
    GTEST_LOG_(INFO) << "FmsFormDbRecordTest_009 abilityName:" << record.abilityName;
    for (unsigned int j = 0; j < record.formUserUids.size(); j++){
        GTEST_LOG_(INFO) << "FmsFormDbRecordTest_009 formUserUids:" << record.formUserUids[j];
    }
    GTEST_LOG_(INFO) << "FmsFormDbRecordTest_009 end";
}

HWTEST_F(FmsFormDbRecordTest, FmsFormDbRecordTest_010, TestSize.Level0) // DeleteDbRecord(1)
{
    GTEST_LOG_(INFO) << "FmsFormDbRecordTest_010 start";
    EXPECT_EQ(ERR_OK, FormDbCache::GetInstance().DeleteFormInfo(1));
    GTEST_LOG_(INFO) << "FmsFormDbRecordTest_010 end";
}

HWTEST_F(FmsFormDbRecordTest, FmsFormDbRecordTest_011, TestSize.Level0) // DeleteDbRecord not exist
{
    GTEST_LOG_(INFO) << "FmsFormDbRecordTest_011 start";
    EXPECT_EQ(ERR_APPEXECFWK_FORM_COMMON_CODE, FormDbCache::GetInstance().DeleteFormInfo(2));
    GTEST_LOG_(INFO) << "FmsFormDbRecordTest_011 end";
}
}
