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
#include "appexecfwk_errors.h"
#define private public
#include "form_data_mgr.h"
#undef private
#include "form_constants.h"
#include "form_record.h"
#include "mock_form_host_client.h"

using namespace testing::ext;
using namespace OHOS;
using namespace OHOS::AppExecFwk;
const std::string FORM_HOST_BUNDLE_NAME = "com.form.provider.service";
const std::string FORM_PROVIDER_ABILITY_NAME = "com.form.provider.app.test.abiliy";
const std::string PARAM_PROVIDER_MODULE_NAME = "com.form.provider.app.test.abiliy";
const std::string FORM_NAME = "formName";

namespace {
class FmsFormDataMgrTest : public testing::Test {
public:
    FmsFormDataMgrTest() 
    {}
    ~FmsFormDataMgrTest()
    {}

    void SetUp();
    void TearDown();
    void InitFormItemInfo(int64_t formId, FormItemInfo &form_item_info);

protected:
    FormDataMgr formDataMgr_;
    sptr<OHOS::AppExecFwk::MockFormHostClient> token_;
};

void FmsFormDataMgrTest::SetUp(void)
{
    // token
    token_ = new (std::nothrow) OHOS::AppExecFwk::MockFormHostClient();
}

void FmsFormDataMgrTest::TearDown(void)
{
    while (!formDataMgr_.formRecords_.empty()) {
        formDataMgr_.formRecords_.erase(formDataMgr_.formRecords_.begin());
    }
    if (!formDataMgr_.clientRecords_.empty()) {
        formDataMgr_.clientRecords_.erase(formDataMgr_.clientRecords_.begin(), formDataMgr_.clientRecords_.end());
    }
    if (!formDataMgr_.tempForms_.empty()) {
        formDataMgr_.tempForms_.erase(formDataMgr_.tempForms_.begin(), formDataMgr_.tempForms_.end());
    }
    formDataMgr_.udidHash_ = 0;
}

void FmsFormDataMgrTest::InitFormItemInfo(int64_t formId, FormItemInfo &form_item_info)
{
    // create hapSourceDirs
    std::vector<std::string> hapSourceDirs;
    std::string hapSourceDir = "1/2/3";
    hapSourceDirs.emplace_back(hapSourceDir);

    // create form_item_info
    form_item_info.SetFormId(formId);
    form_item_info.SetTemporaryFlag(true);
    form_item_info.SetEnableUpdateFlag(true);
    form_item_info.SetUpdateDuration(Constants::MIN_CONFIG_DURATION);
    form_item_info.SetScheduledUpdateTime("10:30");
    form_item_info.SetHapSourceDirs(hapSourceDirs);
}

/**
 * @tc.number: FmsFormDataMgrTest_AllotFormRecord_001
 * @tc.name: AllotFormRecord
 * @tc.desc: Verify that the return value is correct.
 * @tc.details: 
 *      temporaryFlag is true, and tempForms is empty, then create a tempForm.
 *      formRecords_ is empty, then create formRecords.
 */
HWTEST_F(FmsFormDataMgrTest, FmsFormDataMgrTest_AllotFormRecord_001, TestSize.Level0)
{
    GTEST_LOG_(INFO) << "FmsFormDataMgrTest_AllotFormRecord_001 start";

    int64_t formId = 1;
    int callingUid = 0;

    // create FormItemInfo
    FormItemInfo form_item_info;
    InitFormItemInfo(formId, form_item_info);

    FormRecord recordResult = formDataMgr_.AllotFormRecord(form_item_info, callingUid);
    EXPECT_EQ(formId, recordResult.formId);

    GTEST_LOG_(INFO) << "FmsFormDataMgrTest_AllotFormRecord_001 end";
}

/**
 * @tc.number: FmsFormDataMgrTest_AllotFormRecord_002
 * @tc.name: AllotFormRecord
 * @tc.desc: Verify that the return value is correct.
 * @tc.details: 
 *      formRecords_ is include this formId.
 */
HWTEST_F(FmsFormDataMgrTest, FmsFormDataMgrTest_AllotFormRecord_002, TestSize.Level0)
{
    GTEST_LOG_(INFO) << "FmsFormDataMgrTest_AllotFormRecord_002 start";

    int64_t formId = 2;
    int callingUid = 0;

    // create FormItemInfo
    FormItemInfo form_item_info;
    InitFormItemInfo(formId, form_item_info);

    // create formRecords
    FormRecord record = formDataMgr_.CreateFormRecord(form_item_info, callingUid);
    formDataMgr_.formRecords_.emplace(formId, record);

    FormRecord recordResult = formDataMgr_.AllotFormRecord(form_item_info, callingUid);
    EXPECT_EQ(formId, recordResult.formId);

    GTEST_LOG_(INFO) << "FmsFormDataMgrTest_AllotFormRecord_002 end";
}

/**
 * @tc.number: FmsFormDataMgrTest_AllotFormRecord_003
 * @tc.name: AllotFormRecord
 * @tc.desc: Verify that the return value is correct.
 * @tc.details: 
 *      formRecords_ is not include this formId, then create formRecords.
 */
HWTEST_F(FmsFormDataMgrTest, FmsFormDataMgrTest_AllotFormRecord_003, TestSize.Level0)
{
    GTEST_LOG_(INFO) << "FmsFormDataMgrTest_AllotFormRecord_003 start";

    int64_t formId = 3;
    int64_t otherformId = 100;
    int callingUid = 0;

    // create FormItemInfo
    FormItemInfo form_item_info;
    InitFormItemInfo(formId, form_item_info);
    
    // create other FormItemInfo
    FormItemInfo otherFormItemInfo;
    InitFormItemInfo(otherformId, otherFormItemInfo);

    // create formRecords
    FormRecord record = formDataMgr_.CreateFormRecord(otherFormItemInfo, callingUid);
    formDataMgr_.formRecords_.emplace(otherformId, record);

    FormRecord recordResult = formDataMgr_.AllotFormRecord(form_item_info, callingUid);
    EXPECT_EQ(formId, recordResult.formId);

    GTEST_LOG_(INFO) << "FmsFormDataMgrTest_AllotFormRecord_003 end";
}

/**
 * @tc.number: FmsFormDataMgrTest_AllotFormRecord_004
 * @tc.name: AllotFormRecord
 * @tc.desc: Verify that the return value is correct.
 * @tc.details: 
 *      EnableUpdateFlag is true,
 *      SetUpdateDuration is not MAX_CONFIG_DURATION.(call ParseIntervalConfig)
 */
HWTEST_F(FmsFormDataMgrTest, FmsFormDataMgrTest_AllotFormRecord_004, TestSize.Level0)
{
    GTEST_LOG_(INFO) << "FmsFormDataMgrTest_AllotFormRecord_004 start";

    int64_t formId = 4;
    int callingUid = 0;

    // create FormItemInfo
    FormItemInfo form_item_info;
    InitFormItemInfo(formId, form_item_info);
    form_item_info.SetUpdateDuration(Constants::MAX_CONFIG_DURATION);

    FormRecord recordResult = formDataMgr_.AllotFormRecord(form_item_info, callingUid);
    EXPECT_EQ(formId, recordResult.formId);

    GTEST_LOG_(INFO) << "FmsFormDataMgrTest_AllotFormRecord_004 end";
}

/**
 * @tc.number: FmsFormDataMgrTest_AllotFormRecord_005
 * @tc.name: AllotFormRecord
 * @tc.desc: Verify that the return value is correct.
 * @tc.details: 
 *      EnableUpdateFlag is true,
 *      SetUpdateDuration is between MIN_CONFIG_DURATION and MAX_CONFIG_DURATION.(call ParseIntervalConfig)
 */
HWTEST_F(FmsFormDataMgrTest, FmsFormDataMgrTest_AllotFormRecord_005, TestSize.Level0)
{
    GTEST_LOG_(INFO) << "FmsFormDataMgrTest_AllotFormRecord_005 start";

    int64_t formId = 5;
    int callingUid = 0;

    // create FormItemInfo
    FormItemInfo form_item_info;
    InitFormItemInfo(formId, form_item_info);
    form_item_info.SetUpdateDuration(Constants::MAX_CONFIG_DURATION-2);

    FormRecord recordResult = formDataMgr_.AllotFormRecord(form_item_info, callingUid);
    EXPECT_EQ(formId, recordResult.formId);

    GTEST_LOG_(INFO) << "FmsFormDataMgrTest_AllotFormRecord_005 end";
}

/**
 * @tc.number: FmsFormDataMgrTest_AllotFormRecord_006
 * @tc.name: AllotFormRecord
 * @tc.desc: Verify that the return value is correct.
 * @tc.details:  
 *      SetUpdateDuration is 0.(call ParseAtTimerConfig)
 *      获取配置项scheduledUpdateTime_为empty
 */
HWTEST_F(FmsFormDataMgrTest, FmsFormDataMgrTest_AllotFormRecord_006, TestSize.Level0)
{
    GTEST_LOG_(INFO) << "FmsFormDataMgrTest_AllotFormRecord_006 start";

    int64_t formId = 6;
    int callingUid = 0;

    // create FormItemInfo
    FormItemInfo form_item_info;
    InitFormItemInfo(formId, form_item_info);
    form_item_info.SetUpdateDuration(0);
    form_item_info.scheduledUpdateTime_.clear();

    FormRecord recordResult = formDataMgr_.AllotFormRecord(form_item_info, callingUid);
    EXPECT_EQ(formId, recordResult.formId);

    GTEST_LOG_(INFO) << "FmsFormDataMgrTest_AllotFormRecord_006 end";
}

/**
 * @tc.number: FmsFormDataMgrTest_AllotFormRecord_007
 * @tc.name: AllotFormRecord
 * @tc.desc: Verify that the return value is correct.
 * @tc.details: 
 *      SetUpdateDuration is 0.(call ParseAtTimerConfig)
 *      获取配置项scheduledUpdateTime_为无效值
 */
HWTEST_F(FmsFormDataMgrTest, FmsFormDataMgrTest_AllotFormRecord_007, TestSize.Level0)
{
    GTEST_LOG_(INFO) << "FmsFormDataMgrTest_AllotFormRecord_007 start";

    int64_t formId = 7;
    int callingUid = 0;

    // create FormItemInfo
    FormItemInfo form_item_info;
    InitFormItemInfo(formId, form_item_info);
    form_item_info.SetUpdateDuration(0);
    form_item_info.SetScheduledUpdateTime("10:30:10");

    FormRecord recordResult = formDataMgr_.AllotFormRecord(form_item_info, callingUid);
    EXPECT_EQ(formId, recordResult.formId);

    GTEST_LOG_(INFO) << "FmsFormDataMgrTest_AllotFormRecord_007 end";
}

/**
 * @tc.number: FmsFormDataMgrTest_AllotFormRecord_008
 * @tc.name: AllotFormRecord
 * @tc.desc: Verify that the return value is correct.
 * @tc.details: 
 *      SetUpdateDuration is 0.(call ParseAtTimerConfig)
 *      获取配置项scheduledUpdateTime_为无效值
 *      不存在hapSourceDirs_
 */
HWTEST_F(FmsFormDataMgrTest, FmsFormDataMgrTest_AllotFormRecord_008, TestSize.Level0)
{
    GTEST_LOG_(INFO) << "FmsFormDataMgrTest_AllotFormRecord_008 start";

    int64_t formId = 8;
    int callingUid = 0;

    // create FormItemInfo
    FormItemInfo form_item_info;
    InitFormItemInfo(formId, form_item_info);
    form_item_info.SetUpdateDuration(0);
    form_item_info.SetScheduledUpdateTime("10:70");
    form_item_info.hapSourceDirs_.clear();

    FormRecord recordResult = formDataMgr_.AllotFormRecord(form_item_info, callingUid);
    EXPECT_EQ(formId, recordResult.formId);

    GTEST_LOG_(INFO) << "FmsFormDataMgrTest_AllotFormRecord_008 end";
}

/**
 * @tc.number: FmsFormDataMgrTest_AllotFormRecord_009
 * @tc.name: AllotFormRecord
 * @tc.desc: Verify that the return value is correct.
 * @tc.details: 
 *      SetUpdateDuration is 0.(call ParseAtTimerConfig)
 *      获取配置项scheduledUpdateTime_为有效值
 */
HWTEST_F(FmsFormDataMgrTest, FmsFormDataMgrTest_AllotFormRecord_009, TestSize.Level0)
{
    GTEST_LOG_(INFO) << "FmsFormDataMgrTest_AllotFormRecord_009 start";

    int64_t formId = 9;
    int callingUid = 0;

    // create FormItemInfo
    FormItemInfo form_item_info;
    InitFormItemInfo(formId, form_item_info);
    form_item_info.SetUpdateDuration(0);

    FormRecord recordResult = formDataMgr_.AllotFormRecord(form_item_info, callingUid);
    EXPECT_EQ(formId, recordResult.formId);

    GTEST_LOG_(INFO) << "FmsFormDataMgrTest_AllotFormRecord_009 end";
}


/**
 * @tc.number: FmsFormDataMgrTest_AllotFormHostRecord_001
 * @tc.name: AllotFormHostRecord
 * @tc.desc: Verify that the return value is correct.
 * @tc.details: 
 *      clientRecords_ is include token_.
 */
HWTEST_F(FmsFormDataMgrTest, FmsFormDataMgrTest_AllotFormHostRecord_001, TestSize.Level0)
{
    GTEST_LOG_(INFO) << "FmsFormDataMgrTest_AllotFormHostRecord_001 start";

    int64_t formId = 1;
    int callingUid = 0;

    // create FormItemInfo
    FormItemInfo formItemInfo;
    InitFormItemInfo(formId, formItemInfo);

    // create clientRecords_
    FormHostRecord formHostRecord;
    formHostRecord.SetClientStub(token_);
    formDataMgr_.clientRecords_.push_back(formHostRecord);

    EXPECT_EQ(true, formDataMgr_.AllotFormHostRecord(formItemInfo, token_, formId, callingUid));
    EXPECT_EQ(true, formDataMgr_.clientRecords_.begin()->forms_[formId]);

    GTEST_LOG_(INFO) << "FmsFormDataMgrTest_AllotFormHostRecord_001 end";
}

/**
 * @tc.number: FmsFormDataMgrTest_AllotFormHostRecord_002
 * @tc.name: AllotFormHostRecord
 * @tc.desc: Verify that the return value is correct.
 * @tc.details: 
 *      clientRecords_ is not include token_.
 *      CreateHostRecord is OK.
 */
HWTEST_F(FmsFormDataMgrTest, FmsFormDataMgrTest_AllotFormHostRecord_002, TestSize.Level0)
{
    GTEST_LOG_(INFO) << "FmsFormDataMgrTest_AllotFormHostRecord_002 start";

    int64_t formId = 2;
    int callingUid = 0;

    // create FormItemInfo
    FormItemInfo formItemInfo;
    InitFormItemInfo(formId, formItemInfo);

    EXPECT_EQ(true, formDataMgr_.AllotFormHostRecord(formItemInfo, token_, formId, callingUid));

    GTEST_LOG_(INFO) << "FmsFormDataMgrTest_AllotFormHostRecord_002 end";
}

/**
 * @tc.number: FmsFormDataMgrTest_AllotFormHostRecord_003
 * @tc.name: AllotFormHostRecord
 * @tc.desc: Verify that the return value is correct.
 * @tc.details: 
 *      clientRecords_ is not include token_.
 *      CreateHostRecord is NG.
 */
HWTEST_F(FmsFormDataMgrTest, FmsFormDataMgrTest_AllotFormHostRecord_003, TestSize.Level0)
{
    GTEST_LOG_(INFO) << "FmsFormDataMgrTest_AllotFormHostRecord_003 start";

    int64_t formId = 3;
    int callingUid = 0;

    // create FormItemInfo
    FormItemInfo formItemInfo;
    InitFormItemInfo(formId, formItemInfo);

    // set callerToken nullptr
    token_ = nullptr;

    EXPECT_EQ(false, formDataMgr_.AllotFormHostRecord(formItemInfo, token_, formId, callingUid));

    GTEST_LOG_(INFO) << "FmsFormDataMgrTest_AllotFormHostRecord_003 end";
}

/**
 * @tc.number: FmsFormDataMgrTest_CreateFormInfo_001
 * @tc.name: CreateFormInfo
 * @tc.desc: Verify that the return value is correct.     
 */
HWTEST_F(FmsFormDataMgrTest, FmsFormDataMgrTest_CreateFormInfo_001, TestSize.Level0)
{
    GTEST_LOG_(INFO) << "FmsFormDataMgrTest_CreateFormInfo_001 start";

    int64_t formId = 1;

    // create record
    FormRecord record;
    record.bundleName = FORM_HOST_BUNDLE_NAME;
    record.abilityName = FORM_PROVIDER_ABILITY_NAME;
    record.formName = FORM_NAME;
    record.formTempFlg = true;

    FormJsInfo formInfo;

    formDataMgr_.CreateFormInfo(formId, record, formInfo);

    GTEST_LOG_(INFO) << "FmsFormDataMgrTest_CreateFormInfo_001 output=>bundleName:"<<formInfo.bundleName
    <<"abilityName:"<<formInfo.abilityName<<"formName:"<<formInfo.formName<<"formTempFlg:"<<formInfo.formTempFlg;
    GTEST_LOG_(INFO) << "FmsFormDataMgrTest_CreateFormInfo_001 end";
}

/**
 * @tc.number: FmsFormDataMgrTest_CheckTempEnoughForm_001
 * @tc.name: CheckTempEnoughForm
 * @tc.desc: Verify that the return value is correct.
 * @tc.details: 
 *      tempForms_ is empty.    
 */
HWTEST_F(FmsFormDataMgrTest, FmsFormDataMgrTest_CheckTempEnoughForm_001, TestSize.Level0)
{
    GTEST_LOG_(INFO) << "FmsFormDataMgrTest_CheckTempEnoughForm_001 start";

    EXPECT_EQ(ERR_OK, formDataMgr_.CheckTempEnoughForm());

    GTEST_LOG_(INFO) << "FmsFormDataMgrTest_CheckTempEnoughForm_001 end";
}

/**
 * @tc.number: FmsFormDataMgrTest_CheckTempEnoughForm_002
 * @tc.name: CheckTempEnoughForm
 * @tc.desc: Verify that the return value is correct.    
* @tc.details: 
 *      tempForms_'s size is over 256.    
 */
HWTEST_F(FmsFormDataMgrTest, FmsFormDataMgrTest_CheckTempEnoughForm_002, TestSize.Level0)
{
    GTEST_LOG_(INFO) << "FmsFormDataMgrTest_CheckTempEnoughForm_002 start";

    for (int index = 0; index < Constants::MAX_TEMP_FORMS; index++) {
        formDataMgr_.tempForms_.emplace_back(index);
    }
    EXPECT_EQ(ERR_APPEXECFWK_FORM_MAX_SYSTEM_TEMP_FORMS, formDataMgr_.CheckTempEnoughForm());

    GTEST_LOG_(INFO) << "FmsFormDataMgrTest_CheckTempEnoughForm_002 end";
}

/**
 * @tc.number: FmsFormDataMgrTest_CheckEnoughForm_001
 * @tc.name: CheckEnoughForm
 * @tc.desc: Verify that the return value is correct. 
 * @tc.details: 
 *      formRecords_'s size is over 512.    
 */
HWTEST_F(FmsFormDataMgrTest, FmsFormDataMgrTest_CheckEnoughForm_001, TestSize.Level0)
{
    GTEST_LOG_(INFO) << "FmsFormDataMgrTest_CheckEnoughForm_001 start";

    int callingUid = 0;

    // create formRecords
    for (int formId_index = 0; formId_index < Constants::MAX_FORMS; formId_index++) {
        FormItemInfo formItemInfo;
        InitFormItemInfo(formId_index, formItemInfo);

        FormRecord record = formDataMgr_.CreateFormRecord(formItemInfo, callingUid);
        formDataMgr_.formRecords_.emplace(formId_index, record);
    }

    EXPECT_EQ(ERR_APPEXECFWK_FORM_MAX_SYSTEM_FORMS, formDataMgr_.CheckEnoughForm(callingUid));

    GTEST_LOG_(INFO) << "FmsFormDataMgrTest_CheckEnoughForm_001 end";
}

/**
 * @tc.number: FmsFormDataMgrTest_CheckEnoughForm_002
 * @tc.name: CheckEnoughForm
 * @tc.desc: Verify that the return value is correct. 
 * @tc.details: 
 *      there is no formRecords.    
 */
HWTEST_F(FmsFormDataMgrTest, FmsFormDataMgrTest_CheckEnoughForm_002, TestSize.Level0)
{
    GTEST_LOG_(INFO) << "FmsFormDataMgrTest_CheckEnoughForm_002 start";

    int callingUid = 0;

    EXPECT_EQ(ERR_OK, formDataMgr_.CheckEnoughForm(callingUid));

    GTEST_LOG_(INFO) << "FmsFormDataMgrTest_CheckEnoughForm_002 end";
}

/**
 * @tc.number: FmsFormDataMgrTest_CheckEnoughForm_003
 * @tc.name: CheckEnoughForm
 * @tc.desc: Verify that the return value is correct. 
 * @tc.details: 
 *      there is 256 formRecords and their callingUid is 0.    
 */
HWTEST_F(FmsFormDataMgrTest, FmsFormDataMgrTest_CheckEnoughForm_003, TestSize.Level0)
{
    GTEST_LOG_(INFO) << "FmsFormDataMgrTest_CheckEnoughForm_003 start";

    int callingUid = 0;

    // create formRecords
    for (int formId_index = 0; formId_index < Constants::MAX_RECORD_PER_APP; formId_index++) {
        FormItemInfo formItemInfo;
        InitFormItemInfo(formId_index, formItemInfo);
        formItemInfo.SetTemporaryFlag(false);
        FormRecord record = formDataMgr_.CreateFormRecord(formItemInfo, callingUid);
        formDataMgr_.formRecords_.emplace(formId_index, record);
    }

    EXPECT_EQ(ERR_APPEXECFWK_FORM_MAX_FORMS_PER_CLIENT, formDataMgr_.CheckEnoughForm(callingUid));

    GTEST_LOG_(INFO) << "FmsFormDataMgrTest_CheckEnoughForm_003 end";
}

/**
 * @tc.number: FmsFormDataMgrTest_DeleteTempForm_001
 * @tc.name: DeleteTempForm
 * @tc.desc: Verify that the return value is correct. 
 * @tc.details: 
 *       temp form is not found.
 */
HWTEST_F(FmsFormDataMgrTest, FmsFormDataMgrTest_DeleteTempForm_001, TestSize.Level0)
{
    GTEST_LOG_(INFO) << "FmsFormDataMgrTest_DeleteTempForm_001 start";

    int64_t formId = 1;
    EXPECT_EQ(false, formDataMgr_.DeleteTempForm(formId));

    GTEST_LOG_(INFO) << "FmsFormDataMgrTest_DeleteTempForm_001 end";
}

/**
 * @tc.number: FmsFormDataMgrTest_DeleteTempForm_002
 * @tc.name: DeleteTempForm
 * @tc.desc: Verify that the return value is correct. 
 * @tc.details: 
 *       temp form is found.
 */
HWTEST_F(FmsFormDataMgrTest, FmsFormDataMgrTest_DeleteTempForm_002, TestSize.Level0)
{
    GTEST_LOG_(INFO) << "FmsFormDataMgrTest_DeleteTempForm_002 start";

    int64_t formId = 2;
    formDataMgr_.tempForms_.emplace_back(formId);

    EXPECT_EQ(true, formDataMgr_.DeleteTempForm(formId));
    EXPECT_EQ(true, formDataMgr_.tempForms_.empty());

    GTEST_LOG_(INFO) << "FmsFormDataMgrTest_DeleteTempForm_002 end";
}

/**
 * @tc.number: FmsFormDataMgrTest_ModifyFormTempFlg_001
 * @tc.name: ModifyFormTempFlg
 * @tc.desc: Verify that the return value is correct. 
 * @tc.details: 
 *       formRecords is not found.
 */
HWTEST_F(FmsFormDataMgrTest, FmsFormDataMgrTest_ModifyFormTempFlg_001, TestSize.Level0)
{
    GTEST_LOG_(INFO) << "FmsFormDataMgrTest_ModifyFormTempFlg_001 start";

    int64_t formId = 1;
    bool formTempFlg = true;

    EXPECT_EQ(false, formDataMgr_.ModifyFormTempFlg(formId, formTempFlg));

    GTEST_LOG_(INFO) << "FmsFormDataMgrTest_ModifyFormTempFlg_001 end";
}

/**
 * @tc.number: FmsFormDataMgrTest_ModifyFormTempFlg_002
 * @tc.name: ModifyFormTempFlg
 * @tc.desc: Verify that the return value is correct. 
 * @tc.details: 
 *       formRecords is found.
 */
HWTEST_F(FmsFormDataMgrTest, FmsFormDataMgrTest_ModifyFormTempFlg_002, TestSize.Level0)
{
    GTEST_LOG_(INFO) << "FmsFormDataMgrTest_ModifyFormTempFlg_002 start";

    int64_t formId = 2;
    bool formTempFlg = false;

    // create formRecords
    int callingUid = 0;
    FormItemInfo formItemInfo;
    InitFormItemInfo(formId, formItemInfo);
    FormRecord record = formDataMgr_.CreateFormRecord(formItemInfo, callingUid);
    formDataMgr_.formRecords_.emplace(formId, record);

    EXPECT_EQ(true, formDataMgr_.ModifyFormTempFlg(formId, formTempFlg));
    EXPECT_EQ(false, formDataMgr_.formRecords_[formId].formTempFlg);

    GTEST_LOG_(INFO) << "FmsFormDataMgrTest_ModifyFormTempFlg_002 end";
}

/**
 * @tc.number: FmsFormDataMgrTest_AddFormUserUid_001
 * @tc.name: AddFormUserUid
 * @tc.desc: Verify that the return value is correct. 
 * @tc.details: 
 *       formRecords is not found.
 */
HWTEST_F(FmsFormDataMgrTest, FmsFormDataMgrTest_AddFormUserUid_001, TestSize.Level0)
{
    GTEST_LOG_(INFO) << "FmsFormDataMgrTest_AddFormUserUid_001 start";

    int64_t formId = 1;
    int formUserUid = 100;

    EXPECT_EQ(false, formDataMgr_.AddFormUserUid(formId, formUserUid));

    GTEST_LOG_(INFO) << "FmsFormDataMgrTest_AddFormUserUid_001 end";
}

/**
 * @tc.number: FmsFormDataMgrTest_AddFormUserUid_002
 * @tc.name: AddFormUserUid
 * @tc.desc: Verify that the return value is correct. 
 * @tc.details: 
 *       formRecords is found, and add new formUserUid.
 */
HWTEST_F(FmsFormDataMgrTest, FmsFormDataMgrTest_AddFormUserUid_002, TestSize.Level0)
{
    GTEST_LOG_(INFO) << "FmsFormDataMgrTest_AddFormUserUid_002 start";

    int64_t formId = 2;
    int formUserUid = 100;

    // create formRecords
    int callingUid = 0;
    FormItemInfo formItemInfo;
    InitFormItemInfo(formId, formItemInfo);
    FormRecord record = formDataMgr_.CreateFormRecord(formItemInfo, callingUid);
    formDataMgr_.formRecords_.emplace(formId, record);

    EXPECT_EQ(true, formDataMgr_.AddFormUserUid(formId, formUserUid));

    // check formUserUids
    bool find = false;
    for (int uid : formDataMgr_.formRecords_[formId].formUserUids) {
        if (uid == formUserUid) {
            find = true;
        }
    }
    EXPECT_EQ(true, find);

    GTEST_LOG_(INFO) << "FmsFormDataMgrTest_AddFormUserUid_002 end";
}

/**
 * @tc.number: FmsFormDataMgrTest_DeleteFormUserUid_001
 * @tc.name: DeleteFormUserUid
 * @tc.desc: Verify that the return value is correct. 
 * @tc.details: 
 *       formRecords is not found.
 */
HWTEST_F(FmsFormDataMgrTest, FmsFormDataMgrTest_DeleteFormUserUid_001, TestSize.Level0)
{
    GTEST_LOG_(INFO) << "FmsFormDataMgrTest_DeleteFormUserUid_001 start";

    int64_t formId = 1;
    int32_t formUserUid = 100;

    EXPECT_EQ(false, formDataMgr_.DeleteFormUserUid(formId, formUserUid));

    GTEST_LOG_(INFO) << "FmsFormDataMgrTest_DeleteFormUserUid_001 end";
}

/**
 * @tc.number: FmsFormDataMgrTest_DeleteFormUserUid_002
 * @tc.name: DeleteFormUserUid
 * @tc.desc: Verify that the return value is correct. 
 * @tc.details: 
 *       formRecords is found, and delete.
 */
HWTEST_F(FmsFormDataMgrTest, FmsFormDataMgrTest_DeleteFormUserUid_002, TestSize.Level0)
{
    GTEST_LOG_(INFO) << "FmsFormDataMgrTest_DeleteFormUserUid_002 start";

    int64_t formId = 2;
    int32_t formUserUid = 100;

    // create formRecords
    FormItemInfo formItemInfo;
    InitFormItemInfo(formId, formItemInfo);
    FormRecord record = formDataMgr_.CreateFormRecord(formItemInfo, formUserUid);
    formDataMgr_.formRecords_.emplace(formId, record);

    EXPECT_EQ(true, formDataMgr_.DeleteFormUserUid(formId, formUserUid));

    // check formUserUids
    bool find = false;
    for (int uid : formDataMgr_.formRecords_[formId].formUserUids) {
        if (uid == formUserUid) {
            find = true;
        }
    }
    EXPECT_EQ(false, find);

    GTEST_LOG_(INFO) << "FmsFormDataMgrTest_DeleteFormUserUid_002 end";
}

/**
 * @tc.number: FmsFormDataMgrTest_UpdateFormRecord_001
 * @tc.name: UpdateFormRecord
 * @tc.desc: Verify that the return value is correct. 
 * @tc.details: 
 *       formRecords is not found.
 */
HWTEST_F(FmsFormDataMgrTest, FmsFormDataMgrTest_UpdateFormRecord_001, TestSize.Level0)
{
    GTEST_LOG_(INFO) << "FmsFormDataMgrTest_UpdateFormRecord_001 start";

    int64_t formId = 1;

    // create record
    int callingUid = 0;
    FormItemInfo formItemInfo;
    InitFormItemInfo(formId, formItemInfo);
    FormRecord record = formDataMgr_.CreateFormRecord(formItemInfo, callingUid);

    EXPECT_EQ(false, formDataMgr_.UpdateFormRecord(formId, record));

    GTEST_LOG_(INFO) << "FmsFormDataMgrTest_UpdateFormRecord_001 end";
}

/**
 * @tc.number: FmsFormDataMgrTest_UpdateFormRecord_002
 * @tc.name: UpdateFormRecord
 * @tc.desc: Verify that the return value is correct. 
 * @tc.details: 
 *       formRecords is found.
 */
HWTEST_F(FmsFormDataMgrTest, FmsFormDataMgrTest_UpdateFormRecord_002, TestSize.Level0)
{
    GTEST_LOG_(INFO) << "FmsFormDataMgrTest_UpdateFormRecord_002 start";

    int64_t formId = 2;

    // create formRecords
    int callingUid = 0;
    FormItemInfo formItemInfo;
    InitFormItemInfo(formId, formItemInfo);
    FormRecord record = formDataMgr_.CreateFormRecord(formItemInfo, callingUid);
    formDataMgr_.formRecords_.emplace(formId, record);

    int callingUidModify = 1;
    FormRecord recordModify = formDataMgr_.CreateFormRecord(formItemInfo, callingUidModify);

    EXPECT_EQ(true, formDataMgr_.UpdateFormRecord(formId, recordModify));

    // check update form
    auto iter = std::find(formDataMgr_.formRecords_[formId].formUserUids.begin(), 
    formDataMgr_.formRecords_[formId].formUserUids.end(), callingUidModify);
    if (iter != formDataMgr_.formRecords_[formId].formUserUids.end())
    {
        GTEST_LOG_(INFO) << "FmsFormDataMgrTest_UpdateFormRecord_002 find callingUidModify after update!";
    }

    GTEST_LOG_(INFO) << "FmsFormDataMgrTest_UpdateFormRecord_002 end";
}

/**
 * @tc.number: FmsFormDataMgrTest_GetFormRecord_001
 * @tc.name: GetFormRecord
 * @tc.desc: Verify that the return value is correct. 
 * @tc.details: 
 *       formRecords is not found.
 */
HWTEST_F(FmsFormDataMgrTest, FmsFormDataMgrTest_GetFormRecord_001, TestSize.Level0)
{
    GTEST_LOG_(INFO) << "FmsFormDataMgrTest_GetFormRecord_001 start";

    int64_t formId = 1;
    FormRecord record;

    EXPECT_EQ(false, formDataMgr_.GetFormRecord(formId, record));

    GTEST_LOG_(INFO) << "FmsFormDataMgrTest_GetFormRecord_001 end";
}

/**
 * @tc.number: FmsFormDataMgrTest_GetFormRecord_002
 * @tc.name: GetFormRecord
 * @tc.desc: Verify that the return value is correct. 
 * @tc.details: 
 *       formRecords is found.
 */
HWTEST_F(FmsFormDataMgrTest, FmsFormDataMgrTest_GetFormRecord_002, TestSize.Level0)
{
    GTEST_LOG_(INFO) << "FmsFormDataMgrTest_GetFormRecord_002 start";

    int64_t formId = 2;

    // create formRecords_
    int callingUid = 0;
    FormItemInfo formItemInfo;
    InitFormItemInfo(formId, formItemInfo);
    FormRecord record = formDataMgr_.CreateFormRecord(formItemInfo, callingUid);
    formDataMgr_.formRecords_.emplace(formId, record);

    FormRecord recordOutput;
    EXPECT_EQ(true, formDataMgr_.GetFormRecord(formId, recordOutput));
    EXPECT_EQ(formId, recordOutput.formId);

    GTEST_LOG_(INFO) << "FmsFormDataMgrTest_GetFormRecord_002 end";
}


/**
 * @tc.number: FmsFormDataMgrTest_GetFormRecord_2_001
 * @tc.name: GetFormRecord_2
 * @tc.desc: Verify that the return value is correct. 
 * @tc.details: 
 *       formRecords is not found.
 */
HWTEST_F(FmsFormDataMgrTest, FmsFormDataMgrTest_GetFormRecord_2_001, TestSize.Level0)
{
    GTEST_LOG_(INFO) << "FmsFormDataMgrTest_GetFormRecord_2_001 start";

    std::string bundleName = "bundleName";
    std::vector<FormRecord> formInfos;

    EXPECT_EQ(false, formDataMgr_.GetFormRecord(bundleName, formInfos));

    GTEST_LOG_(INFO) << "FmsFormDataMgrTest_GetFormRecord_2_001 end";
}

/**
 * @tc.number: FmsFormDataMgrTest_GetFormRecord_2_002
 * @tc.name: GetFormRecord_2
 * @tc.desc: Verify that the return value is correct. 
 * @tc.details: 
 *       formRecords is found.
 */
HWTEST_F(FmsFormDataMgrTest, FmsFormDataMgrTest_GetFormRecord_2_002, TestSize.Level0)
{
    GTEST_LOG_(INFO) << "FmsFormDataMgrTest_GetFormRecord_2_002 start";

    std::string bundleName = "bundleName";
    std::vector<FormRecord> formInfos;

    // create formRecords_
    int64_t formId = 2;
    int callingUid = 0;
    FormItemInfo formItemInfo;
    InitFormItemInfo(formId, formItemInfo);
    formItemInfo.SetProviderBundleName(bundleName);
    FormRecord record = formDataMgr_.CreateFormRecord(formItemInfo, callingUid);
    formDataMgr_.formRecords_.emplace(formId, record);

    EXPECT_EQ(true, formDataMgr_.GetFormRecord(bundleName, formInfos));

    GTEST_LOG_(INFO) << "FmsFormDataMgrTest_GetFormRecord_2_002 end";
}

/**
 * @tc.number: FmsFormDataMgrTest_HasFormUserUids_001
 * @tc.name: HasFormUserUids
 * @tc.desc: Verify that the return value is correct. 
 * @tc.details: 
 *       formRecords is not found.
 */
HWTEST_F(FmsFormDataMgrTest, FmsFormDataMgrTest_HasFormUserUids_001, TestSize.Level0)
{
    GTEST_LOG_(INFO) << "FmsFormDataMgrTest_HasFormUserUids_001 start";

    int64_t formId = 1;

    EXPECT_EQ(false, formDataMgr_.HasFormUserUids(formId));

    GTEST_LOG_(INFO) << "FmsFormDataMgrTest_HasFormUserUids_001 end";
}

/**
 * @tc.number: FmsFormDataMgrTest_HasFormUserUids_002
 * @tc.name: HasFormUserUids
 * @tc.desc: Verify that the return value is correct. 
 * @tc.details: 
 *       formRecords is found.
 */
HWTEST_F(FmsFormDataMgrTest, FmsFormDataMgrTest_HasFormUserUids_002, TestSize.Level0)
{
    GTEST_LOG_(INFO) << "FmsFormDataMgrTest_HasFormUserUids_002 start";

    int64_t formId = 2;

    // create formRecords_
    int callingUid = 0;
    FormItemInfo formItemInfo;
    InitFormItemInfo(formId, formItemInfo);
    FormRecord record = formDataMgr_.CreateFormRecord(formItemInfo, callingUid);
    formDataMgr_.formRecords_.emplace(formId, record);

    EXPECT_EQ(true, formDataMgr_.HasFormUserUids(formId));

    GTEST_LOG_(INFO) << "FmsFormDataMgrTest_HasFormUserUids_002 end";
}

/**
 * @tc.number: FmsFormDataMgrTest_GetFormHostRecord_001
 * @tc.name: GetFormHostRecord
 * @tc.desc: Verify that the return value is correct. 
 * @tc.details: 
 *       clientRecords_ is not found.
 */
HWTEST_F(FmsFormDataMgrTest, FmsFormDataMgrTest_GetFormHostRecord_001, TestSize.Level0)
{
    GTEST_LOG_(INFO) << "FmsFormDataMgrTest_GetFormHostRecord_001 start";

    int64_t formId = 1;
    FormHostRecord formHostRecord;

    EXPECT_EQ(false, formDataMgr_.GetFormHostRecord(formId, formHostRecord));

    GTEST_LOG_(INFO) << "FmsFormDataMgrTest_GetFormHostRecord_001 end";
}

/**
 * @tc.number: FmsFormDataMgrTest_GetFormHostRecord_002
 * @tc.name: GetFormHostRecord
 * @tc.desc: Verify that the return value is correct. 
 * @tc.details: 
 *       clientRecords_ is found.
 */
HWTEST_F(FmsFormDataMgrTest, FmsFormDataMgrTest_GetFormHostRecord_002, TestSize.Level0)
{
    GTEST_LOG_(INFO) << "FmsFormDataMgrTest_GetFormHostRecord_002 start";

    int64_t formId = 2;
    FormHostRecord formHostRecord;
    
    // create clientRecords_
    FormHostRecord form_host_record;
    form_host_record.SetClientStub(token_);
    form_host_record.AddForm(formId);
    formDataMgr_.clientRecords_.push_back(form_host_record);

    EXPECT_EQ(true, formDataMgr_.GetFormHostRecord(formId, formHostRecord));
    EXPECT_EQ(true, formHostRecord.forms_[formId]);

    GTEST_LOG_(INFO) << "FmsFormDataMgrTest_GetFormHostRecord_002 end";
}

/**
 * @tc.number: FmsFormDataMgrTest_DeleteHostRecord_001
 * @tc.name: DeleteHostRecord
 * @tc.desc: Verify that the return value is correct. 
 * @tc.details: 
 *       clientRecords_ is found.
 */
HWTEST_F(FmsFormDataMgrTest, FmsFormDataMgrTest_DeleteHostRecord_001, TestSize.Level0)
{
    GTEST_LOG_(INFO) << "FmsFormDataMgrTest_DeleteHostRecord_001 start";

    int64_t formId = 1;

    // create clientRecords_
    FormHostRecord formHostRecord;
    formHostRecord.SetClientStub(token_);
    formHostRecord.AddForm(formId);
    formDataMgr_.clientRecords_.push_back(formHostRecord);

    EXPECT_EQ(true, formDataMgr_.DeleteHostRecord(token_, formId));
    EXPECT_EQ(true, formDataMgr_.clientRecords_.empty());

    GTEST_LOG_(INFO) << "FmsFormDataMgrTest_DeleteHostRecord_001 end";
}

/**
 * @tc.number: FmsFormDataMgrTest_CleanHostRemovedForms_001
 * @tc.name: CleanHostRemovedForms
 * @tc.desc: Verify that the return value is correct. 
 * @tc.details: 
 *       clientRecords_ is found.
 */
HWTEST_F(FmsFormDataMgrTest, FmsFormDataMgrTest_CleanHostRemovedForms_001, TestSize.Level0)
{
    GTEST_LOG_(INFO) << "FmsFormDataMgrTest_CleanHostRemovedForms_001 start";

    std::vector<int64_t> removedFormIds;
    int64_t formId = 1;
    removedFormIds.emplace_back(formId);

    // create clientRecords_
    FormHostRecord formHostRecord;
    formHostRecord.SetClientStub(token_);
    formHostRecord.AddForm(formId);
    formDataMgr_.clientRecords_.push_back(formHostRecord);

    formDataMgr_.CleanHostRemovedForms(removedFormIds);

    GTEST_LOG_(INFO) << "FmsFormDataMgrTest_CleanHostRemovedForms_001 end";
}

/**
 * @tc.number: FmsFormDataMgrTest_HandleHostDied_001
 * @tc.name: HandleHostDied
 * @tc.desc: Verify that the return value is correct. 
 * @tc.details: 
 *       clientRecords_ & tempForms_ & formRecords is found.
 */
HWTEST_F(FmsFormDataMgrTest, FmsFormDataMgrTest_HandleHostDied_001, TestSize.Level0)
{
    GTEST_LOG_(INFO) << "FmsFormDataMgrTest_HandleHostDied_001 start";

    int64_t formId = 1;

    // create tempForms_
    formDataMgr_.tempForms_.emplace_back(formId);

    // create clientRecords_
    FormHostRecord formHostRecord;
    formHostRecord.SetClientStub(token_);
    formHostRecord.AddForm(formId);
    formDataMgr_.clientRecords_.push_back(formHostRecord);

    // create formRecords
    int callingUid = 0;
    FormItemInfo formItemInfo;
    InitFormItemInfo(formId, formItemInfo);
    FormRecord record = formDataMgr_.CreateFormRecord(formItemInfo, callingUid);
    formDataMgr_.formRecords_.emplace(formId, record);

    formDataMgr_.HandleHostDied(token_);

    GTEST_LOG_(INFO) << "FmsFormDataMgrTest_HandleHostDied_001 end";
}

/**
 * @tc.number: FmsFormDataMgrTest_HandleHostDied_002
 * @tc.name: HandleHostDied
 * @tc.desc: Verify that the return value is correct. 
 * @tc.details: 
 *       tempForms_ is not match.
 */
HWTEST_F(FmsFormDataMgrTest, FmsFormDataMgrTest_HandleHostDied_002, TestSize.Level0)
{
    GTEST_LOG_(INFO) << "FmsFormDataMgrTest_HandleHostDied_002 start";

    int64_t formId = 2;
    int64_t otherFormId = 3;

    // create tempForms_
    formDataMgr_.tempForms_.emplace_back(otherFormId);

    // create clientRecords_
    FormHostRecord formHostRecord;
    formHostRecord.SetClientStub(token_);
    formHostRecord.AddForm(formId);
    formDataMgr_.clientRecords_.push_back(formHostRecord);

    // create formRecords
    int callingUid = 0;
    FormItemInfo formItemInfo;
    InitFormItemInfo(formId, formItemInfo);
    FormRecord record = formDataMgr_.CreateFormRecord(formItemInfo, callingUid);
    formDataMgr_.formRecords_.emplace(formId, record);

    formDataMgr_.HandleHostDied(token_);

    GTEST_LOG_(INFO) << "FmsFormDataMgrTest_HandleHostDied_002 end";
}

/**
 * @tc.number: FmsFormDataMgrTest_HandleHostDied_003
 * @tc.name: HandleHostDied
 * @tc.desc: Verify that the return value is correct. 
 * @tc.details: 
 *       remoteHost is not match, formRecords is not found.
 */
HWTEST_F(FmsFormDataMgrTest, FmsFormDataMgrTest_HandleHostDied_003, TestSize.Level0)
{
    GTEST_LOG_(INFO) << "FmsFormDataMgrTest_HandleHostDied_003 start";

    int64_t formId = 3;

    // create clientRecords_
    sptr<OHOS::AppExecFwk::MockFormHostClient> token_2;
    FormHostRecord formHostRecord;
    formHostRecord.SetClientStub(token_2);
    formHostRecord.AddForm(formId);
    formDataMgr_.clientRecords_.push_back(formHostRecord);
   
    // create formRecords
    int callingUid = 0;
    FormItemInfo formItemInfo;
    InitFormItemInfo(formId, formItemInfo);
    FormRecord record = formDataMgr_.CreateFormRecord(formItemInfo, callingUid);
    formDataMgr_.formRecords_.emplace(formId, record);

    formDataMgr_.HandleHostDied(token_);

    GTEST_LOG_(INFO) << "FmsFormDataMgrTest_HandleHostDied_003 end";
}

/**
 * @tc.number: FmsFormDataMgrTest_IsEnableRefresh_001
 * @tc.name: IsEnableRefresh
 * @tc.desc: Verify that the return value is correct. 
 * @tc.details: 
 *       clientRecords_ is not found.
 */
HWTEST_F(FmsFormDataMgrTest, FmsFormDataMgrTest_IsEnableRefresh_001, TestSize.Level0)
{
    GTEST_LOG_(INFO) << "FmsFormDataMgrTest_IsEnableRefresh_001 start";
    
    int64_t formId = 1;
    EXPECT_EQ(false, formDataMgr_.IsEnableRefresh(formId));

    GTEST_LOG_(INFO) << "FmsFormDataMgrTest_IsEnableRefresh_001 end";
}

/**
 * @tc.number: FmsFormDataMgrTest_IsEnableRefresh_002
 * @tc.name: IsEnableRefresh
 * @tc.desc: Verify that the return value is correct. 
 * @tc.details: 
 *       clientRecords_ is found.
 */
HWTEST_F(FmsFormDataMgrTest, FmsFormDataMgrTest_IsEnableRefresh_002, TestSize.Level0)
{
    GTEST_LOG_(INFO) << "FmsFormDataMgrTest_IsEnableRefresh_002 start";
    
    int64_t formId = 2;

    // create clientRecords_
    sptr<OHOS::AppExecFwk::MockFormHostClient> token_;
    FormHostRecord formHostRecord;
    formHostRecord.SetClientStub(token_);
    formHostRecord.AddForm(formId);
    formDataMgr_.clientRecords_.push_back(formHostRecord);

    EXPECT_EQ(true, formDataMgr_.IsEnableRefresh(formId));

    GTEST_LOG_(INFO) << "FmsFormDataMgrTest_IsEnableRefresh_002 end";
}

/**
 * @tc.number: FmsFormDataMgrTest_GenerateUdidHash_001
 * @tc.name: GenerateUdidHash
 * @tc.desc: Verify that the return value is correct. 
 * @tc.details: 
 *       udidHash_ is not 0.
 */
HWTEST_F(FmsFormDataMgrTest, FmsFormDataMgrTest_GenerateUdidHash_001, TestSize.Level0)
{
    GTEST_LOG_(INFO) << "FmsFormDataMgrTest_GenerateUdidHash_001 start";
    
    formDataMgr_.udidHash_ = 1;
    EXPECT_EQ(true, formDataMgr_.GenerateUdidHash());

    GTEST_LOG_(INFO) << "FmsFormDataMgrTest_GenerateUdidHash_001 end";
}

/**
 * @tc.number: FmsFormDataMgrTest_GenerateUdidHash_002
 * @tc.name: GenerateUdidHash
 * @tc.desc: Verify that the return value is correct. 
 * @tc.details: 
 *       udidHash_ is 0.
 */
HWTEST_F(FmsFormDataMgrTest, FmsFormDataMgrTest_GenerateUdidHash_002, TestSize.Level0)
{
    GTEST_LOG_(INFO) << "FmsFormDataMgrTest_GenerateUdidHash_002 start";
    
    formDataMgr_.udidHash_ = Constants::INVALID_UDID_HASH;
    EXPECT_EQ(true, formDataMgr_.GenerateUdidHash());

    GTEST_LOG_(INFO) << "FmsFormDataMgrTest_GenerateUdidHash_002 end";
}

/**
 * @tc.number: FmsFormDataMgrTest_GetMatchedHostClient_001
 * @tc.name: GetMatchedHostClient
 * @tc.desc: Verify that the return value is correct. 
 * @tc.details: 
 *       clientRecords_ is not found.
 */
HWTEST_F(FmsFormDataMgrTest, FmsFormDataMgrTest_GetMatchedHostClient_001, TestSize.Level0)
{
    GTEST_LOG_(INFO) << "FmsFormDataMgrTest_GetMatchedHostClient_001 start";
    
    // create clientRecords_
    FormHostRecord formHostRecord;

    EXPECT_EQ(false, formDataMgr_.GetMatchedHostClient(token_, formHostRecord));

    GTEST_LOG_(INFO) << "FmsFormDataMgrTest_GetMatchedHostClient_001 end";
}

/**
 * @tc.number: FmsFormDataMgrTest_GetMatchedHostClient_002
 * @tc.name: GetMatchedHostClient
 * @tc.desc: Verify that the return value is correct. 
 * @tc.details: 
 *       clientRecords_ is found.
 */
HWTEST_F(FmsFormDataMgrTest, FmsFormDataMgrTest_GetMatchedHostClient_002, TestSize.Level0)
{
    GTEST_LOG_(INFO) << "FmsFormDataMgrTest_GetMatchedHostClient_002 start";
    
    int64_t formId = 2;

    // create clientRecords_
    FormHostRecord formHostRecord;
    formHostRecord.SetClientStub(token_);
    formHostRecord.AddForm(formId);
    formDataMgr_.clientRecords_.push_back(formHostRecord);

    FormHostRecord formHostRecordOutput;

    EXPECT_EQ(true, formDataMgr_.GetMatchedHostClient(token_, formHostRecordOutput));
    EXPECT_EQ(true, formHostRecordOutput.forms_[formId]);

    GTEST_LOG_(INFO) << "FmsFormDataMgrTest_GetMatchedHostClient_002 end";
}

/**
 * @tc.number: FmsFormDataMgrTest_SetNeedRefresh_001
 * @tc.name: SetNeedRefresh
 * @tc.desc: Verify that the return value is correct. 
 * @tc.details: 
 *       formRecords_ is not found.
 */
HWTEST_F(FmsFormDataMgrTest, FmsFormDataMgrTest_SetNeedRefresh_001, TestSize.Level0)
{
    GTEST_LOG_(INFO) << "FmsFormDataMgrTest_SetNeedRefresh_001 start";
        
    int64_t formId = 1;
    bool needRefresh = true;

    formDataMgr_.SetNeedRefresh(formId, needRefresh);

    GTEST_LOG_(INFO) << "FmsFormDataMgrTest_SetNeedRefresh_001 end";
}

/**
 * @tc.number: FmsFormDataMgrTest_SetNeedRefresh_002
 * @tc.name: SetNeedRefresh
 * @tc.desc: Verify that the return value is correct. 
 * @tc.details: 
 *       formRecords_ is not found.
 */
HWTEST_F(FmsFormDataMgrTest, FmsFormDataMgrTest_SetNeedRefresh_002, TestSize.Level0)
{
    GTEST_LOG_(INFO) << "FmsFormDataMgrTest_SetNeedRefresh_002 start";
        
    int64_t formId = 2;
    bool needRefresh = true;

    // create formRecords
    int callingUid = 0;
    FormItemInfo formItemInfo;
    InitFormItemInfo(formId, formItemInfo);
    FormRecord record = formDataMgr_.CreateFormRecord(formItemInfo, callingUid);
    formDataMgr_.formRecords_.emplace(formId, record);

    formDataMgr_.SetNeedRefresh(formId, needRefresh);
    EXPECT_EQ(true, formDataMgr_.formRecords_.find(formId)->second.needRefresh);

    GTEST_LOG_(INFO) << "FmsFormDataMgrTest_SetNeedRefresh_002 end";
}

/**
 * @tc.number: FmsFormDataMgrTest_SetCountTimerRefresh_001
 * @tc.name: SetCountTimerRefresh
 * @tc.desc: Verify that the return value is correct. 
 * @tc.details: 
 *       formRecords_ is not found.
 */
HWTEST_F(FmsFormDataMgrTest, FmsFormDataMgrTest_SetCountTimerRefresh_001, TestSize.Level0)
{
    GTEST_LOG_(INFO) << "FmsFormDataMgrTest_SetCountTimerRefresh_001 start";
        
    int64_t formId = 1;
    bool countTimerRefresh = true;

    formDataMgr_.SetCountTimerRefresh(formId, countTimerRefresh);

    GTEST_LOG_(INFO) << "FmsFormDataMgrTest_SetCountTimerRefresh_001 end";
}

/**
 * @tc.number: FmsFormDataMgrTest_SetCountTimerRefresh_002
 * @tc.name: SetCountTimerRefresh
 * @tc.desc: Verify that the return value is correct. 
 * @tc.details: 
 *       formRecords_ is not found.
 */
HWTEST_F(FmsFormDataMgrTest, FmsFormDataMgrTest_SetCountTimerRefresh_002, TestSize.Level0)
{
    GTEST_LOG_(INFO) << "FmsFormDataMgrTest_SetCountTimerRefresh_002 start";
        
    int64_t formId = 2;
    bool countTimerRefresh = true;

    // create formRecords
    int callingUid = 0;
    FormItemInfo formItemInfo;
    InitFormItemInfo(formId, formItemInfo);
    FormRecord record = formDataMgr_.CreateFormRecord(formItemInfo, callingUid);
    formDataMgr_.formRecords_.emplace(formId, record);

    formDataMgr_.SetCountTimerRefresh(formId, countTimerRefresh);
    EXPECT_EQ(true, formDataMgr_.formRecords_.find(formId)->second.isCountTimerRefresh);

    GTEST_LOG_(INFO) << "FmsFormDataMgrTest_SetCountTimerRefresh_002 end";
}

/**
 * @tc.number: FmsFormDataMgrTest_GetUpdatedForm_001
 * @tc.name: GetUpdatedForm
 * @tc.desc: Verify that the return value is correct. 
 * @tc.details: 
 *       targetForms is empty.
 */
HWTEST_F(FmsFormDataMgrTest, FmsFormDataMgrTest_GetUpdatedForm_001, TestSize.Level0)
{
    GTEST_LOG_(INFO) << "FmsFormDataMgrTest_GetUpdatedForm_001 start";
        
    FormRecord record;
    record.bundleName = "";
    record.moduleName = "";
    record.abilityName = "";
    record.formName = "";
    record.specification = 0;
    
    std::vector<FormInfo> targetForms;
    FormInfo updatedForm;

    EXPECT_EQ(false, formDataMgr_.GetUpdatedForm(record, targetForms, updatedForm));

    GTEST_LOG_(INFO) << "FmsFormDataMgrTest_GetUpdatedForm_001 end";
}

/**
 * @tc.number: FmsFormDataMgrTest_GetUpdatedForm_002
 * @tc.name: GetUpdatedForm
 * @tc.desc: Verify that the return value is correct. 
 * @tc.details: 
 *       targetForms is not empty. record is same as formInfo.
 */
HWTEST_F(FmsFormDataMgrTest, FmsFormDataMgrTest_GetUpdatedForm_002, TestSize.Level0)
{
    GTEST_LOG_(INFO) << "FmsFormDataMgrTest_GetUpdatedForm_002 start";
        
    int32_t specification = 2;

    FormRecord record;
    record.bundleName = FORM_HOST_BUNDLE_NAME;
    record.moduleName = PARAM_PROVIDER_MODULE_NAME;
    record.abilityName = FORM_PROVIDER_ABILITY_NAME;
    record.formName = FORM_NAME;
    record.specification = specification;

    std::vector<FormInfo> targetForms;
    FormInfo formInfo;
    formInfo.bundleName = FORM_HOST_BUNDLE_NAME;
    formInfo.moduleName = PARAM_PROVIDER_MODULE_NAME;
    formInfo.abilityName = FORM_PROVIDER_ABILITY_NAME;
    formInfo.name = FORM_NAME;
    formInfo.supportDimensions.emplace_back(specification);
    targetForms.emplace_back(formInfo);

    FormInfo updatedForm;

    EXPECT_EQ(true, formDataMgr_.GetUpdatedForm(record, targetForms, updatedForm));
    EXPECT_EQ(FORM_HOST_BUNDLE_NAME, updatedForm.bundleName);

    GTEST_LOG_(INFO) << "FmsFormDataMgrTest_GetUpdatedForm_002 end";
}

/**
 * @tc.number: FmsFormDataMgrTest_GetUpdatedForm_003
 * @tc.name: GetUpdatedForm
 * @tc.desc: Verify that the return value is correct. 
 * @tc.details: 
 *       targetForms is not empty. record is not same as formInfo.
 */
HWTEST_F(FmsFormDataMgrTest, FmsFormDataMgrTest_GetUpdatedForm_003, TestSize.Level0)
{
    GTEST_LOG_(INFO) << "FmsFormDataMgrTest_GetUpdatedForm_003 start";
        
    int32_t specification = 3;

    FormRecord record;
    record.bundleName = "bundleName";
    record.moduleName = PARAM_PROVIDER_MODULE_NAME;
    record.abilityName = FORM_PROVIDER_ABILITY_NAME;
    record.formName = FORM_NAME;
    record.specification = specification;

    std::vector<FormInfo> targetForms;
    FormInfo formInfo;
    formInfo.bundleName = FORM_HOST_BUNDLE_NAME;
    formInfo.moduleName = PARAM_PROVIDER_MODULE_NAME;
    formInfo.abilityName = FORM_PROVIDER_ABILITY_NAME;
    formInfo.name = FORM_NAME;
    formInfo.supportDimensions.emplace_back(specification);
    targetForms.emplace_back(formInfo);

    FormInfo updatedForm;

    EXPECT_EQ(false, formDataMgr_.GetUpdatedForm(record, targetForms, updatedForm));

    GTEST_LOG_(INFO) << "FmsFormDataMgrTest_GetUpdatedForm_003 end";
}

/**
 * @tc.number: FmsFormDataMgrTest_SetEnableUpdate_001
 * @tc.name: SetEnableUpdate
 * @tc.desc: Verify that the return value is correct. 
 * @tc.details: 
 *       formRecords_ is not found.
 */
HWTEST_F(FmsFormDataMgrTest, FmsFormDataMgrTest_SetEnableUpdate_001, TestSize.Level0)
{
    GTEST_LOG_(INFO) << "FmsFormDataMgrTest_SetEnableUpdate_001 start";
        
    int64_t formId = 1;
    bool enableUpdate = true;

    formDataMgr_.SetEnableUpdate(formId, enableUpdate);

    GTEST_LOG_(INFO) << "FmsFormDataMgrTest_SetEnableUpdate_001 end";
}

/**
 * @tc.number: FmsFormDataMgrTest_SetEnableUpdate_002
 * @tc.name: SetEnableUpdate
 * @tc.desc: Verify that the return value is correct. 
 * @tc.details: 
 *       formRecords_ is not found.
 */
HWTEST_F(FmsFormDataMgrTest, FmsFormDataMgrTest_SetEnableUpdate_002, TestSize.Level0)
{
    GTEST_LOG_(INFO) << "FmsFormDataMgrTest_SetEnableUpdate_002 start";
        
    int64_t formId = 2;
    bool enableUpdate = true;

    // create formRecords
    int callingUid = 0;
    FormItemInfo formItemInfo;
    InitFormItemInfo(formId, formItemInfo);
    FormRecord record = formDataMgr_.CreateFormRecord(formItemInfo, callingUid);
    formDataMgr_.formRecords_.emplace(formId, record);

    formDataMgr_.SetEnableUpdate(formId, enableUpdate);
    EXPECT_EQ(true, formDataMgr_.formRecords_.find(formId)->second.isEnableUpdate);

    GTEST_LOG_(INFO) << "FmsFormDataMgrTest_SetEnableUpdate_002 end";
}

/**
 * @tc.number: FmsFormDataMgrTest_SetUpdateInfo_001
 * @tc.name: SetUpdateInfo
 * @tc.desc: Verify that the return value is correct. 
 * @tc.details: 
 *       formRecords_ is not found.
 */
HWTEST_F(FmsFormDataMgrTest, FmsFormDataMgrTest_SetUpdateInfo_001, TestSize.Level0)
{
    GTEST_LOG_(INFO) << "FmsFormDataMgrTest_SetUpdateInfo_001 start";

    int64_t formId = 1;
    bool enableUpdate = true;
    long updateDuration = 100;
    int updateAtHour = 24;
    int updateAtMin = 59;

    formDataMgr_.SetUpdateInfo(formId, enableUpdate, updateDuration, updateAtHour, updateAtMin);

    GTEST_LOG_(INFO) << "FmsFormDataMgrTest_SetUpdateInfo_001 end";
}

/**
 * @tc.number: FmsFormDataMgrTest_SetUpdateInfo_002
 * @tc.name: SetUpdateInfo
 * @tc.desc: Verify that the return value is correct. 
 * @tc.details: 
 *       formRecords_ is not found.
 */
HWTEST_F(FmsFormDataMgrTest, FmsFormDataMgrTest_SetUpdateInfo_002, TestSize.Level0)
{
    GTEST_LOG_(INFO) << "FmsFormDataMgrTest_SetUpdateInfo_002 start";
        
    int64_t formId = 1;
    bool enableUpdate = true;
    long updateDuration = 100;
    int updateAtHour = 24;
    int updateAtMin = 59;

    // create formRecords
    int callingUid = 0;
    FormItemInfo formItemInfo;
    InitFormItemInfo(formId, formItemInfo);
    FormRecord record = formDataMgr_.CreateFormRecord(formItemInfo, callingUid);
    formDataMgr_.formRecords_.emplace(formId, record);

    formDataMgr_.SetUpdateInfo(formId, enableUpdate, updateDuration, updateAtHour, updateAtMin);
    EXPECT_EQ(true, formDataMgr_.formRecords_.find(formId)->second.isEnableUpdate);
    EXPECT_EQ(100, formDataMgr_.formRecords_.find(formId)->second.updateDuration);
    EXPECT_EQ(24, formDataMgr_.formRecords_.find(formId)->second.updateAtHour);
    EXPECT_EQ(59, formDataMgr_.formRecords_.find(formId)->second.updateAtMin);

    GTEST_LOG_(INFO) << "FmsFormDataMgrTest_SetUpdateInfo_002 end";
}

/**
 * @tc.number: FmsFormDataMgrTest_CleanRemovedFormRecords_001
 * @tc.name: CleanRemovedFormRecords

 * @tc.desc: Verify that the return value is correct. 
 * @tc.details: 
 *       removedForm is matched with formRecords_.
 */
HWTEST_F(FmsFormDataMgrTest, FmsFormDataMgrTest_CleanRemovedFormRecords_001, TestSize.Level0)
{
    GTEST_LOG_(INFO) << "FmsFormDataMgrTest_CleanRemovedFormRecords_001 start";

    // create formRecords
    int callingUid = 0;
    int64_t formId = 1;
    FormItemInfo formItemInfo;
    InitFormItemInfo(formId, formItemInfo);
    FormRecord record = formDataMgr_.CreateFormRecord(formItemInfo, callingUid);
    formDataMgr_.formRecords_.emplace(formId, record);

    std::string bundleName = FORM_HOST_BUNDLE_NAME;

    std::set<int64_t> removedForms;
    int64_t removedForm = formId;
    removedForms.emplace(removedForm);
    
    formDataMgr_.CleanRemovedFormRecords(bundleName, removedForms);
    EXPECT_EQ(true, formDataMgr_.formRecords_.empty());

    GTEST_LOG_(INFO) << "FmsFormDataMgrTest_CleanRemovedFormRecords_001 end";
}

/**
 * @tc.number: FmsFormDataMgrTest_CleanRemovedFormRecords_002
 * @tc.name: CleanRemovedFormRecords

 * @tc.desc: Verify that the return value is correct. 
 * @tc.details: 
 *       removedForm is not matched with formRecords_.
 */
HWTEST_F(FmsFormDataMgrTest, FmsFormDataMgrTest_CleanRemovedFormRecords_002, TestSize.Level0)
{
    GTEST_LOG_(INFO) << "FmsFormDataMgrTest_CleanRemovedFormRecords_002 start";

    // create formRecords
    int callingUid = 0;
    int64_t formId = 2;
    FormItemInfo formItemInfo;
    InitFormItemInfo(formId, formItemInfo);
    FormRecord record = formDataMgr_.CreateFormRecord(formItemInfo, callingUid);
    formDataMgr_.formRecords_.emplace(formId, record);

    std::string bundleName = FORM_HOST_BUNDLE_NAME;

    std::set<int64_t> removedForms;
    int64_t removedForm = 100;
    removedForms.emplace(removedForm);
    
    formDataMgr_.CleanRemovedFormRecords(bundleName, removedForms);

    GTEST_LOG_(INFO) << "FmsFormDataMgrTest_CleanRemovedFormRecords_002 end";
}

/**
 * @tc.number: FmsFormDataMgrTest_CleanRemovedTempFormRecords_001
 * @tc.name: CleanRemovedTempFormRecords

 * @tc.desc: Verify that the return value is correct. 
 * @tc.details: 
 *      bundleName is matched with formRecords_, and it is temp.
 *      erase formRecords_ and tempForms_.
 */
HWTEST_F(FmsFormDataMgrTest, FmsFormDataMgrTest_CleanRemovedTempFormRecords_001, TestSize.Level0)
{
    GTEST_LOG_(INFO) << "FmsFormDataMgrTest_CleanRemovedTempFormRecords_001 start";
    
    int64_t formId = 1;
    std::string bundleName = FORM_HOST_BUNDLE_NAME;
   
    std::set<int64_t> removedForms;
    int64_t removedForm = formId;
    removedForms.emplace(removedForm);

    // create formRecords
    int callingUid = 0;
    FormItemInfo formItemInfo;
    InitFormItemInfo(formId, formItemInfo);
    formItemInfo.SetProviderBundleName(bundleName);
    FormRecord record = formDataMgr_.CreateFormRecord(formItemInfo, callingUid);
    formDataMgr_.formRecords_.emplace(formId, record);

    // create tempForms_
    formDataMgr_.tempForms_.emplace_back(formId);

    formDataMgr_.CleanRemovedTempFormRecords(bundleName, removedForms);
    EXPECT_EQ(true, formDataMgr_.formRecords_.empty());
    EXPECT_EQ(true, formDataMgr_.tempForms_.empty());

    GTEST_LOG_(INFO) << "FmsFormDataMgrTest_CleanRemovedTempFormRecords_001 end";
}

/**
 * @tc.number: FmsFormDataMgrTest_CleanRemovedTempFormRecords_002
 * @tc.name: CleanRemovedTempFormRecords

 * @tc.desc: Verify that the return value is correct. 
 * @tc.details: 
 *      bundleName is not matched with formRecords_.
 *      erase none.
 */
HWTEST_F(FmsFormDataMgrTest, FmsFormDataMgrTest_CleanRemovedTempFormRecords_002, TestSize.Level0)
{
    GTEST_LOG_(INFO) << "FmsFormDataMgrTest_CleanRemovedTempFormRecords_002 start";
    
    int64_t formId = 2;
    std::string bundleName = FORM_HOST_BUNDLE_NAME;
    std::string otherBundleName = "bundleName";

    std::set<int64_t> removedForms;
    int64_t removedForm = formId;
    removedForms.emplace(removedForm);

    // create formRecords
    int callingUid = 0;
    FormItemInfo formItemInfo;
    InitFormItemInfo(formId, formItemInfo);
    formItemInfo.SetProviderBundleName(otherBundleName);
    FormRecord record = formDataMgr_.CreateFormRecord(formItemInfo, callingUid);
    formDataMgr_.formRecords_.emplace(formId, record);

    // create tempForms_
    formDataMgr_.tempForms_.emplace_back(formId);

    formDataMgr_.CleanRemovedTempFormRecords(bundleName, removedForms);
    EXPECT_EQ(false, formDataMgr_.formRecords_.empty());
    EXPECT_EQ(false, formDataMgr_.tempForms_.empty());

    GTEST_LOG_(INFO) << "FmsFormDataMgrTest_CleanRemovedTempFormRecords_002 end";
}

/**
 * @tc.number: FmsFormDataMgrTest_GetReCreateFormRecordsByBundleName_001
 * @tc.name: GetReCreateFormRecordsByBundleName

 * @tc.desc: Verify that the return value is correct. 
 */
HWTEST_F(FmsFormDataMgrTest, FmsFormDataMgrTest_GetReCreateFormRecordsByBundleName_001, TestSize.Level0)
{
    GTEST_LOG_(INFO) << "FmsFormDataMgrTest_GetReCreateFormRecordsByBundleName_001 start";
    
    int64_t formId = 1;
    std::string bundleName = FORM_HOST_BUNDLE_NAME;
   
    std::set<int64_t> reCreateForms;
    int64_t reCreateForm = formId;
    reCreateForms.emplace(reCreateForm);

    // create formRecords
    int callingUid = 0;
    FormItemInfo formItemInfo;
    InitFormItemInfo(formId, formItemInfo);
    formItemInfo.SetProviderBundleName(bundleName);
    FormRecord record = formDataMgr_.CreateFormRecord(formItemInfo, callingUid);
    formDataMgr_.formRecords_.emplace(formId, record);

    formDataMgr_.GetReCreateFormRecordsByBundleName(bundleName, reCreateForms);
    EXPECT_EQ(true, reCreateForms.count(formId));

    GTEST_LOG_(INFO) << "FmsFormDataMgrTest_GetReCreateFormRecordsByBundleName_001 end";
}

/**
 * @tc.number: FmsFormDataMgrTest_SetFormCacheInited_001
 * @tc.name: SetFormCacheInited
 * @tc.desc: Verify that the return value is correct. 
 * @tc.details: 
 *       formRecords_ is not found.
 */
HWTEST_F(FmsFormDataMgrTest, FmsFormDataMgrTest_SetFormCacheInited_001, TestSize.Level0)
{
    GTEST_LOG_(INFO) << "FmsFormDataMgrTest_SetFormCacheInited_001 start";

    int64_t formId = 1;

    formDataMgr_.SetFormCacheInited(formId, true);

    GTEST_LOG_(INFO) << "FmsFormDataMgrTest_SetFormCacheInited_001 end";
}

/**
 * @tc.number: FmsFormDataMgrTest_SetFormCacheInited_002
 * @tc.name: SetFormCacheInited
 * @tc.desc: Verify that the return value is correct. 
 * @tc.details: 
 *       formRecords_ is found.
 */
HWTEST_F(FmsFormDataMgrTest, FmsFormDataMgrTest_SetFormCacheInited_002, TestSize.Level0)
{
    GTEST_LOG_(INFO) << "FmsFormDataMgrTest_SetFormCacheInited_002 start";

    int64_t formId = 2;

    // create formRecords
    int callingUid = 0;
    FormItemInfo formItemInfo;
    InitFormItemInfo(formId, formItemInfo);
    FormRecord record = formDataMgr_.CreateFormRecord(formItemInfo, callingUid);
    formDataMgr_.formRecords_.emplace(formId, record);

    formDataMgr_.SetFormCacheInited(formId, true);
    EXPECT_EQ(true, formDataMgr_.formRecords_.find(formId)->second.isInited);
    EXPECT_EQ(false, formDataMgr_.formRecords_.find(formId)->second.needRefresh);

    GTEST_LOG_(INFO) << "FmsFormDataMgrTest_SetFormCacheInited_002 end";
}

/**
 * @tc.number: FmsFormDataMgrTest_SetVersionUpgrade_001
 * @tc.name: SetVersionUpgrade
 * @tc.desc: Verify that the return value is correct. 
 * @tc.details: 
 *       formRecords_ is not found.
 */
HWTEST_F(FmsFormDataMgrTest, FmsFormDataMgrTest_SetVersionUpgrade_001, TestSize.Level0)
{
    GTEST_LOG_(INFO) << "FmsFormDataMgrTest_SetVersionUpgrade_001 start";

    int64_t formId = 1;
    bool version = true;

    formDataMgr_.SetVersionUpgrade(formId, version);

    GTEST_LOG_(INFO) << "FmsFormDataMgrTest_SetVersionUpgrade_001 end";
}

/**
 * @tc.number: FmsFormDataMgrTest_SetVersionUpgrade_002
 * @tc.name: SetFormCacheInited
 * @tc.desc: Verify that the return value is correct. 
 * @tc.details: 
 *       formRecords_ is found.
 */
HWTEST_F(FmsFormDataMgrTest, FmsFormDataMgrTest_SetVersionUpgrade_002, TestSize.Level0)
{
    GTEST_LOG_(INFO) << "FmsFormDataMgrTest_SetVersionUpgrade_002 start";

    int64_t formId = 2;
    bool versionUpgrade = true;

    // create formRecords
    int callingUid = 0;
    FormItemInfo formItemInfo;
    InitFormItemInfo(formId, formItemInfo);
    FormRecord record = formDataMgr_.CreateFormRecord(formItemInfo, callingUid);
    formDataMgr_.formRecords_.emplace(formId, record);

    formDataMgr_.SetVersionUpgrade(formId, versionUpgrade);
    EXPECT_EQ(true, formDataMgr_.formRecords_.find(formId)->second.versionUpgrade);

    GTEST_LOG_(INFO) << "FmsFormDataMgrTest_SetVersionUpgrade_002 end";
}

/**
 * @tc.number: FmsFormDataMgrTest_UpdateHostNeedRefresh_001
 * @tc.name: UpdateHostNeedRefresh
 * @tc.desc: Verify that the return value is correct. 
 */
HWTEST_F(FmsFormDataMgrTest, FmsFormDataMgrTest_UpdateHostNeedRefresh_001, TestSize.Level0)
{
    GTEST_LOG_(INFO) << "FmsFormDataMgrTest_UpdateHostNeedRefresh_001 start";

    int64_t formId = 1;
    bool needRefresh = true;

    // create clientRecords_
    FormHostRecord formHostRecord;
    formHostRecord.AddForm(formId);
    formDataMgr_.clientRecords_.push_back(formHostRecord);

    formDataMgr_.UpdateHostNeedRefresh(formId, needRefresh);

    GTEST_LOG_(INFO) << "FmsFormDataMgrTest_UpdateHostNeedRefresh_001 end";
}

/**
 * @tc.number: FmsFormDataMgrTest_UpdateHostForm_001
 * @tc.name: UpdateHostForm
 * @tc.desc: Verify that the return value is correct. 
 */
HWTEST_F(FmsFormDataMgrTest, FmsFormDataMgrTest_UpdateHostForm_001, TestSize.Level0)
{
    GTEST_LOG_(INFO) << "FmsFormDataMgrTest_UpdateHostForm_001 start";

    int64_t formId = 1;
    FormRecord formRecord;

    // create clientRecords_
    FormHostRecord formHostRecord;
    formHostRecord.AddForm(formId);
    formDataMgr_.clientRecords_.push_back(formHostRecord);

    EXPECT_EQ(true, formDataMgr_.UpdateHostForm(formId, formRecord));

    GTEST_LOG_(INFO) << "FmsFormDataMgrTest_UpdateHostForm_001 end";
}

/**
 * @tc.number: FmsFormDataMgrTest_UpdateHostFormFlag_001
 * @tc.name: UpdateHostFormFlag
 * @tc.desc: Verify that the return value is correct. 
 * @tc.details: 
 *       clientRecords_ is not exit.  
 */
HWTEST_F(FmsFormDataMgrTest, FmsFormDataMgrTest_UpdateHostFormFlag_001, TestSize.Level0)
{
    GTEST_LOG_(INFO) << "FmsFormDataMgrTest_UpdateHostFormFlag_001 start";

    std::vector<int64_t> formIds;
    int64_t formId = 1;
    formIds.emplace_back(formId);

    bool flag = true;

    std::vector<int64_t> refreshForms;

    EXPECT_EQ(ERR_APPEXECFWK_FORM_INVALID_PARAM, formDataMgr_.UpdateHostFormFlag(formIds, token_, flag, refreshForms));

    GTEST_LOG_(INFO) << "FmsFormDataMgrTest_UpdateHostFormFlag_001 end";
}

/**
 * @tc.number: FmsFormDataMgrTest_UpdateHostFormFlag_002
 * @tc.name: UpdateHostFormFlag
 * @tc.desc: Verify that the return value is correct.
 * @tc.details: 
 *       formRecords's VersionUpgrade is false.  
 */
HWTEST_F(FmsFormDataMgrTest, FmsFormDataMgrTest_UpdateHostFormFlag_002, TestSize.Level0)
{
    GTEST_LOG_(INFO) << "FmsFormDataMgrTest_UpdateHostFormFlag_002 start";

    std::vector<int64_t> formIds;
    int64_t formId = 2;
    formIds.emplace_back(formId);

    bool flag = true;

    std::vector<int64_t> refreshForms;

    // create clientRecords_
    FormHostRecord formHostRecord;
    formHostRecord.SetClientStub(token_);
    formHostRecord.AddForm(formId);
    // SetNeedRefresh:true
    formHostRecord.SetNeedRefresh(formId, true);
    formDataMgr_.clientRecords_.push_back(formHostRecord);

    // create formRecords
    int callingUid = 0;
    FormItemInfo formItemInfo;
    InitFormItemInfo(formId, formItemInfo);
    FormRecord record = formDataMgr_.CreateFormRecord(formItemInfo, callingUid);
    formDataMgr_.formRecords_.emplace(formId, record);
    // versionUpgrade : false
    formDataMgr_.SetVersionUpgrade(formId, false);

    EXPECT_EQ(ERR_OK, formDataMgr_.UpdateHostFormFlag(formIds, token_, flag, refreshForms));

    GTEST_LOG_(INFO) << "FmsFormDataMgrTest_UpdateHostFormFlag_002 end";
}

/**
 * @tc.number: FmsFormDataMgrTest_UpdateHostFormFlag_003
 * @tc.name: UpdateHostFormFlag
 * @tc.desc: Verify that the return value is correct. 
 * @tc.details: 
 *       formRecords's VersionUpgrade is true.  
 */
HWTEST_F(FmsFormDataMgrTest, FmsFormDataMgrTest_UpdateHostFormFlag_003, TestSize.Level0)
{
    GTEST_LOG_(INFO) << "FmsFormDataMgrTest_UpdateHostFormFlag_003 start";

    std::vector<int64_t> formIds;
    int64_t formId = 3;
    formIds.emplace_back(formId);

    bool flag = true;

    std::vector<int64_t> refreshForms;

    // create clientRecords_
    FormHostRecord formHostRecord;
    formHostRecord.SetClientStub(token_);
    formHostRecord.AddForm(formId);
    // SetNeedRefresh:true
    formHostRecord.SetNeedRefresh(formId, true);
    formDataMgr_.clientRecords_.push_back(formHostRecord);

    // create formRecords
    int callingUid = 0;
    FormItemInfo formItemInfo;
    InitFormItemInfo(formId, formItemInfo);
    FormRecord record = formDataMgr_.CreateFormRecord(formItemInfo, callingUid);
    formDataMgr_.formRecords_.emplace(formId, record);
    // versionUpgrade : true
    formDataMgr_.SetVersionUpgrade(formId, true);

    EXPECT_EQ(ERR_OK, formDataMgr_.UpdateHostFormFlag(formIds, token_, flag, refreshForms));

    GTEST_LOG_(INFO) << "FmsFormDataMgrTest_UpdateHostFormFlag_003 end";
}

/**
 * @tc.number: FmsFormDataMgrTest_UpdateHostFormFlag_004
 * @tc.name: UpdateHostFormFlag
 * @tc.desc: Verify that the return value is correct. 
 * @tc.details: 
 *       clientRecords_'s NeedRefresh is false.  
 */
HWTEST_F(FmsFormDataMgrTest, FmsFormDataMgrTest_UpdateHostFormFlag_004, TestSize.Level0)
{
    GTEST_LOG_(INFO) << "FmsFormDataMgrTest_UpdateHostFormFlag_004 start";

    std::vector<int64_t> formIds;
    int64_t formId = 4;
    formIds.emplace_back(formId);

    bool flag = true;

    std::vector<int64_t> refreshForms;

    // create clientRecords_
    FormHostRecord formHostRecord;
    formHostRecord.SetClientStub(token_);
    formHostRecord.AddForm(formId);
    // SetNeedRefresh:false
    formHostRecord.SetNeedRefresh(formId, false);
    formDataMgr_.clientRecords_.push_back(formHostRecord);

    // create formRecords
    int callingUid = 0;
    FormItemInfo formItemInfo;
    InitFormItemInfo(formId, formItemInfo);
    FormRecord record = formDataMgr_.CreateFormRecord(formItemInfo, callingUid);
    formDataMgr_.formRecords_.emplace(formId, record);

    EXPECT_EQ(ERR_OK, formDataMgr_.UpdateHostFormFlag(formIds, token_, flag, refreshForms));

    GTEST_LOG_(INFO) << "FmsFormDataMgrTest_UpdateHostFormFlag_004 end";
}

/**
 * @tc.number: FmsFormDataMgrTest_UpdateHostFormFlag_005
 * @tc.name: UpdateHostFormFlag
 * @tc.desc: Verify that the return value is correct. 
 * @tc.details: 
 *       clientRecords_ is not include formId.  
 */
HWTEST_F(FmsFormDataMgrTest, FmsFormDataMgrTest_UpdateHostFormFlag_005, TestSize.Level0)
{
    GTEST_LOG_(INFO) << "FmsFormDataMgrTest_UpdateHostFormFlag_005 start";

    std::vector<int64_t> formIds;
    int64_t formId = 5;
    formIds.emplace_back(formId);

    bool flag = true;

    std::vector<int64_t> refreshForms;

    // create clientRecords_
    int64_t otherformId = 500;
    FormHostRecord formHostRecord;
    formHostRecord.SetClientStub(token_);
    formHostRecord.AddForm(otherformId);
    formDataMgr_.clientRecords_.push_back(formHostRecord);

    // create formRecords
    int callingUid = 0;
    FormItemInfo formItemInfo;
    InitFormItemInfo(formId, formItemInfo);
    FormRecord record = formDataMgr_.CreateFormRecord(formItemInfo, callingUid);
    formDataMgr_.formRecords_.emplace(formId, record);

    EXPECT_EQ(ERR_OK, formDataMgr_.UpdateHostFormFlag(formIds, token_, flag, refreshForms));

    GTEST_LOG_(INFO) << "FmsFormDataMgrTest_UpdateHostFormFlag_005 end";
}

/**
 * @tc.number: FmsFormDataMgrTest_UpdateHostFormFlag_006
 * @tc.name: UpdateHostFormFlag
 * @tc.desc: Verify that the return value is correct. 
 * @tc.details: 
 *       flag is false.  
 */
HWTEST_F(FmsFormDataMgrTest, FmsFormDataMgrTest_UpdateHostFormFlag_006, TestSize.Level0)
{
    GTEST_LOG_(INFO) << "FmsFormDataMgrTest_UpdateHostFormFlag_006 start";

    std::vector<int64_t> formIds;
    int64_t formId = 6;
    formIds.emplace_back(formId);

    bool flag = false;

    std::vector<int64_t> refreshForms;

    // create clientRecords_
    FormHostRecord formHostRecord;
    formHostRecord.SetClientStub(token_);
    formHostRecord.AddForm(formId);
    formDataMgr_.clientRecords_.push_back(formHostRecord);

    // create formRecords
    int callingUid = 0;
    FormItemInfo formItemInfo;
    InitFormItemInfo(formId, formItemInfo);
    FormRecord record = formDataMgr_.CreateFormRecord(formItemInfo, callingUid);
    formDataMgr_.formRecords_.emplace(formId, record);

    EXPECT_EQ(ERR_OK, formDataMgr_.UpdateHostFormFlag(formIds, token_, flag, refreshForms));

    GTEST_LOG_(INFO) << "FmsFormDataMgrTest_UpdateHostFormFlag_006 end";
}

/**
 * @tc.number: FmsFormDataMgrTest_UpdateHostFormFlag_007
 * @tc.name: UpdateHostFormFlag
 * @tc.desc: Verify that the return value is correct. 
 * @tc.details: 
 *       formRecords's needRefresh is true.  
 */
HWTEST_F(FmsFormDataMgrTest, FmsFormDataMgrTest_UpdateHostFormFlag_007, TestSize.Level0)
{
    GTEST_LOG_(INFO) << "FmsFormDataMgrTest_UpdateHostFormFlag_007 start";

    std::vector<int64_t> formIds;
    int64_t formId = 7;
    formIds.emplace_back(formId);

    bool flag = true;

    std::vector<int64_t> refreshForms;

    // create clientRecords_
    FormHostRecord formHostRecord;
    formHostRecord.SetClientStub(token_);
    formHostRecord.AddForm(formId);
    formDataMgr_.clientRecords_.push_back(formHostRecord);

    // create formRecords
    int callingUid = 0;
    FormItemInfo formItemInfo;
    InitFormItemInfo(formId, formItemInfo);
    FormRecord record = formDataMgr_.CreateFormRecord(formItemInfo, callingUid);
    // needRefresh:true
    record.needRefresh = true; 
    formDataMgr_.formRecords_.emplace(formId, record);

    EXPECT_EQ(ERR_OK, formDataMgr_.UpdateHostFormFlag(formIds, token_, flag, refreshForms));

    GTEST_LOG_(INFO) << "FmsFormDataMgrTest_UpdateHostFormFlag_007 end";
}

/**
 * @tc.number: FmsFormDataMgrTest_UpdateHostFormFlag_008
 * @tc.name: UpdateHostFormFlag
 * @tc.desc: Verify that the return value is correct. 
 * @tc.details: 
 *       formRecords is not include formId.  
 */
HWTEST_F(FmsFormDataMgrTest, FmsFormDataMgrTest_UpdateHostFormFlag_008, TestSize.Level0)
{
    GTEST_LOG_(INFO) << "FmsFormDataMgrTest_UpdateHostFormFlag_008 start";

    std::vector<int64_t> formIds;
    int64_t formId = 8;
    formIds.emplace_back(formId);

    bool flag = true;

    std::vector<int64_t> refreshForms;

    // create clientRecords_
    FormHostRecord formHostRecord;
    formHostRecord.SetClientStub(token_);
    formHostRecord.AddForm(formId);
    formDataMgr_.clientRecords_.push_back(formHostRecord);

    // create formRecords
    int64_t otherFormId = 800;
    int callingUid = 0;
    FormItemInfo formItemInfo;
    InitFormItemInfo(otherFormId, formItemInfo);
    FormRecord record = formDataMgr_.CreateFormRecord(formItemInfo, callingUid);
    formDataMgr_.formRecords_.emplace(otherFormId, record);

    EXPECT_EQ(ERR_OK, formDataMgr_.UpdateHostFormFlag(formIds, token_, flag, refreshForms));

    GTEST_LOG_(INFO) << "FmsFormDataMgrTest_UpdateHostFormFlag_008 end";
}
}
