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

#include <gtest/gtest.h>
#include <thread>
#include <string>

#include "app_log_wrapper.h"
#include "appexecfwk_errors.h"
#define private public
#include "form_bms_helper.h"
#include "form_constants.h"
#include "form_timer_mgr.h"
#include "form_mgr_service.h"
#include "form_mgr_adapter.h"
#include "form_data_mgr.h"
#undef private
#include "mock_bundle_manager.h"

using namespace testing::ext;
using namespace OHOS;
using namespace OHOS::AppExecFwk;

namespace {
const std::string FORM_HOST_BUNDLE_NAME = "com.form.provider.service";
const std::string FORM_PROVIDER_ABILITY_NAME = "com.form.provider.app.test.abiliy";
const std::string PERMISSION_NAME_REQUIRE_FORM = "ohos.permission.REQUIRE_FORM";
const std::string DEF_LABEL1 = "PermissionFormRequireGrant";

class FmsFormSetNextRefreshTest : public testing::Test {
public:
    FmsFormSetNextRefreshTest() : formSetNextRefresh_(nullptr)
    {}
    ~FmsFormSetNextRefreshTest()
    {}
    std::shared_ptr<FormMgrService> formSetNextRefresh_ = DelayedSingleton<FormMgrService>::GetInstance();
    static void SetUpTestCase(void);
    static void TearDownTestCase(void);
    void SetUp();
    void TearDown();
protected:
    sptr<BundleMgrService> mockBundleMgr_;
};
void FmsFormSetNextRefreshTest::SetUpTestCase(void)
{}

void FmsFormSetNextRefreshTest::TearDownTestCase(void)
{}

void FmsFormSetNextRefreshTest::SetUp(void)
{
    formSetNextRefresh_ = std::make_shared<FormMgrService>();

    formSetNextRefresh_->OnStart();

    // mock BundleMgr
    mockBundleMgr_ = new (std::nothrow) BundleMgrService();
    ASSERT_TRUE(mockBundleMgr_ != nullptr);
    FormBmsHelper::GetInstance().SetBundleManager(mockBundleMgr_);
}

void FmsFormSetNextRefreshTest::TearDown(void)
{}


/**
 * @tc.number: FmsFormSetNextRefreshTest_SetNextRefreshTime_001
 * @tc.name: SetNextRefreshTime
 * @tc.desc: Verify that the return value is true.(formId is invalid)
 */
HWTEST_F(FmsFormSetNextRefreshTest, FmsFormSetNextRefreshTest_SetNextRefreshTime_001, TestSize.Level0)
{
    GTEST_LOG_(INFO) << "FmsFormSetNextRefreshTest_SetNextRefreshTime_001 start";
    int64_t formId = 0; // invalid formId
    int64_t nextTime = Constants::MIN_NEXT_TIME;
    
    EXPECT_EQ(ERR_FORM_INVALID_PARAM, formSetNextRefresh_->SetNextRefreshTime(formId, nextTime));
    GTEST_LOG_(INFO) << "FmsFormSetNextRefreshTest_SetNextRefreshTime_001 end";
}

/**
 * @tc.number: FmsFormSetNextRefreshTest_SetNextRefreshTime_002
 * @tc.name: SetNextRefreshTime
 * @tc.desc: Verify that the return value is true.(not found in form record)
 */
HWTEST_F(FmsFormSetNextRefreshTest, FmsFormSetNextRefreshTest_SetNextRefreshTime_002, TestSize.Level0)
{
    GTEST_LOG_(INFO) << "FmsFormSetNextRefreshTest_SetNextRefreshTime_002 start";
    int64_t formId = 2;
    int64_t nextTime = Constants::MIN_NEXT_TIME;
    
    EXPECT_EQ(ERR_APPEXECFWK_FORM_INFO_NOT_EXIST, formSetNextRefresh_->SetNextRefreshTime(formId, nextTime));
    GTEST_LOG_(INFO) << "FmsFormSetNextRefreshTest_SetNextRefreshTime_002 end";
}

/**
 * @tc.number: FmsFormSetNextRefreshTest_SetNextRefreshTime_003
 * @tc.name: SetNextRefreshTime
 * @tc.desc: Verify that the return value is true.(BundleName is found in form record, but no dynamicRefreshTask)
 */
HWTEST_F(FmsFormSetNextRefreshTest, FmsFormSetNextRefreshTest_SetNextRefreshTime_003, TestSize.Level0)
{
    GTEST_LOG_(INFO) << "FmsFormSetNextRefreshTest_SetNextRefreshTime_003 start";
    int64_t formId = 3;  
    int64_t nextTime = Constants::MIN_NEXT_TIME;

    // check dynamicRefreshTasks_
    EXPECT_EQ(true, FormTimerMgr::GetInstance().dynamicRefreshTasks_.empty());
    
    // creat formRecords_
    FormItemInfo iteminfo;
    iteminfo.formId_ = formId;
    iteminfo.providerBundleName_ = FORM_HOST_BUNDLE_NAME;
    iteminfo.abilityName_ = FORM_PROVIDER_ABILITY_NAME;
    FormDataMgr::GetInstance().AllotFormRecord(iteminfo, 0);	

    EXPECT_EQ(ERR_OK, formSetNextRefresh_->SetNextRefreshTime(formId, nextTime));
    
    // check dynamicRefreshTasks_
    EXPECT_EQ(false, FormTimerMgr::GetInstance().dynamicRefreshTasks_.empty());

    GTEST_LOG_(INFO) << "FmsFormSetNextRefreshTest_SetNextRefreshTime_003 end";
}

/**
 * @tc.number: FmsFormSetNextRefreshTest_SetNextRefreshTime_004
 * @tc.name: SetNextRefreshTime
 * @tc.desc: Verify that the return value is true.(BundleName is not found in form record)
 */
HWTEST_F(FmsFormSetNextRefreshTest, FmsFormSetNextRefreshTest_SetNextRefreshTime_004, TestSize.Level0)
{
    GTEST_LOG_(INFO) << "FmsFormSetNextRefreshTest_SetNextRefreshTime_004 start";
    int64_t formId = 4;  
    int64_t nextTime = Constants::MIN_NEXT_TIME;

    // creat formRecords_
    FormItemInfo iteminfo;
    iteminfo.formId_ = formId;
    iteminfo.providerBundleName_ = "other_bundleName";
    iteminfo.abilityName_ = FORM_PROVIDER_ABILITY_NAME;
    FormDataMgr::GetInstance().AllotFormRecord(iteminfo, 0);	

    EXPECT_EQ(ERR_OPERATION_FORM_NOT_SELF, formSetNextRefresh_->SetNextRefreshTime(formId, nextTime));
    GTEST_LOG_(INFO) << "FmsFormSetNextRefreshTest_SetNextRefreshTime_004 end";
}

/**
 * @tc.number: FmsFormSetNextRefreshTest_SetNextRefreshTime_005
 * @tc.name: SetNextRefreshTime
 * @tc.desc: Verify that the return value is true.(have dynamicRefreshTask , have IntervalTimerTasks)
 */
HWTEST_F(FmsFormSetNextRefreshTest, FmsFormSetNextRefreshTest_SetNextRefreshTime_005, TestSize.Level0)
{
    GTEST_LOG_(INFO) << "FmsFormSetNextRefreshTest_SetNextRefreshTime_005 start";
    int64_t formId = 5;  
    int64_t nextTime = Constants::MIN_NEXT_TIME;

    // creat formRecords_
    FormItemInfo iteminfo;
    iteminfo.formId_ = formId;
    iteminfo.providerBundleName_ = FORM_HOST_BUNDLE_NAME;
    iteminfo.abilityName_ = FORM_PROVIDER_ABILITY_NAME;
    FormDataMgr::GetInstance().AllotFormRecord(iteminfo, 0);	

    // Creat dynamicRefreshTasks_
    DynamicRefreshItem theItem;
    theItem.formId = formId;
    theItem.settedTime = 1;
    FormTimerMgr::GetInstance().dynamicRefreshTasks_.clear();
    FormTimerMgr::GetInstance().dynamicRefreshTasks_.emplace_back(theItem);
    // check dynamicRefreshTasks_
    EXPECT_EQ(1, FormTimerMgr::GetInstance().dynamicRefreshTasks_.at(0).settedTime);

    // Create IntervalTimerTasks_
    FormTimer task;
    task.formId = formId;
    task.isEnable = true;
    FormTimerMgr::GetInstance().AddIntervalTimer(task);

    EXPECT_EQ(ERR_OK, formSetNextRefresh_->SetNextRefreshTime(formId, nextTime));
    // check dynamicRefreshTasks_
    EXPECT_EQ(true, FormTimerMgr::GetInstance().dynamicRefreshTasks_.at(0).settedTime != 1);

    GTEST_LOG_(INFO) << "FmsFormSetNextRefreshTest_SetNextRefreshTime_005 end";
}

/**
 * @tc.number: FmsFormSetNextRefreshTest_SetNextRefreshTime_006
 * @tc.name: SetNextRefreshTime
 * @tc.desc: Verify that the return value is true.(timerRefreshedCount >= 50)
 */
HWTEST_F(FmsFormSetNextRefreshTest, FmsFormSetNextRefreshTest_SetNextRefreshTime_006, TestSize.Level0)
{
    GTEST_LOG_(INFO) << "FmsFormSetNextRefreshTest_SetNextRefreshTime_006 start";

    int64_t formId = 6;  
    int64_t nextTime = Constants::MIN_NEXT_TIME;

    // creat formRecords_
    FormItemInfo iteminfo;
    iteminfo.formId_ = formId;
    iteminfo.providerBundleName_ = FORM_HOST_BUNDLE_NAME;
    iteminfo.abilityName_ = FORM_PROVIDER_ABILITY_NAME;
    FormDataMgr::GetInstance().AllotFormRecord(iteminfo, 0);

    // set timerRefreshedCount
    FormTimerMgr::GetInstance().refreshLimiter_.AddItem(formId);
    auto iter = FormTimerMgr::GetInstance().refreshLimiter_.limiterMap_.find(formId);
    if (iter == FormTimerMgr::GetInstance().refreshLimiter_.limiterMap_.end()) {
        GTEST_LOG_(INFO) << "not found in limiterMap_!!!";
    } else {
        iter->second.refreshCount = Constants::LIMIT_COUNT;
    }
    EXPECT_EQ(ERR_MAX_REFRESH, formSetNextRefresh_->SetNextRefreshTime(formId, nextTime));
    
    GTEST_LOG_(INFO) << "FmsFormSetNextRefreshTest_SetNextRefreshTime_006 end";
}
}
