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

#define private public
#define protected public
#include "process_optimizer.h"
#undef private
#undef protected
#include "app_log_wrapper.h"
#include <fcntl.h>
#include <sys/eventfd.h>
#include <unistd.h>
#include "lmks_client.h"

namespace{
constexpr int APP_OOM_ADJ_BACKGROUND_MIN = 400;
constexpr int APP_OOM_ADJ_SUSPEND_MIN = 600;
const std::string APP_RECORD_NAME = "App_Name_Z";
static constexpr int APP_SUSPEND_TIMEOUT_DEFAULT = 100;
static constexpr int APP_USLEEP = 200 * 1000;
using namespace testing::ext;
};

namespace OHOS {
namespace AppExecFwk {
class AmsProcessOptimizerTest : public testing::Test {
public:
    static void SetUpTestCase();
    static void TearDownTestCase();
    void SetUp();
    void TearDown();
};

void AmsProcessOptimizerTest::SetUpTestCase()
{}
void AmsProcessOptimizerTest::TearDownTestCase()
{}
void AmsProcessOptimizerTest::SetUp()
{}
void AmsProcessOptimizerTest::TearDown()
{}

/*
 * Feature: AMS
 * Function: ProcessOptimizer
 * SubFunction: Init
 * FunctionPoints: initialization
 * EnvConditions: Mobile that can run ohos test framework.
 * CaseDescription: Verify the function Init can check the invalid suspendTimeout
 */
HWTEST_F(AmsProcessOptimizerTest, AmsProcessOptimizerTest_Init_001, TestSize.Level1)
{
    APP_LOGD("AmsProcessOptimizerTest_Init_001 start.");

    auto lmksClient = std::make_shared<LmksClient>();
    int suspendTimeout = -1;
    std::unique_ptr<ProcessOptimizer> amsProcessOptimizer =
        std::make_unique<ProcessOptimizer>(lmksClient, suspendTimeout);
    EXPECT_EQ(false, amsProcessOptimizer->Init());

    APP_LOGD("AmsProcessOptimizerTest_Init_001 end.");
}

/*
 * Feature: AMS
 * Function: ProcessOptimizer
 * SubFunction: Init
 * FunctionPoints: initialization
 * EnvConditions: Mobile that can run ohos test framework.
 * CaseDescription: Verify the function Init can check the invalid eventHandler_
 */
HWTEST_F(AmsProcessOptimizerTest, AmsProcessOptimizerTest_Init_002, TestSize.Level1)
{
    APP_LOGD("AmsProcessOptimizerTest_Init_002 start.");

    auto lmksClient = std::make_shared<LmksClient>();
    int suspendTimeout = 0;
    std::unique_ptr<ProcessOptimizer> amsProcessOptimizer =
        std::make_unique<ProcessOptimizer>(lmksClient, suspendTimeout);
    amsProcessOptimizer->eventHandler_ = std::make_shared<EventHandler>(EventRunner::Create());
    EXPECT_EQ(false, amsProcessOptimizer->Init());
    amsProcessOptimizer->eventHandler_ = nullptr;

    APP_LOGD("AmsProcessOptimizerTest_Init_002 end.");
}

/*
 * Feature: AMS
 * Function: ProcessOptimizer
 * SubFunction: Init
 * FunctionPoints: initialization
 * EnvConditions: Mobile that can run ohos test framework.
 * CaseDescription: Verify that the function Init can be executed normally and initialize
 */
HWTEST_F(AmsProcessOptimizerTest, AmsProcessOptimizerTest_Init_003, TestSize.Level1)
{
    APP_LOGD("AmsProcessOptimizerTest_Init_003 start.");

    auto lmksClient = std::make_shared<LmksClient>();
    std::unique_ptr<ProcessOptimizer> amsProcessOptimizer = std::make_unique<ProcessOptimizer>(lmksClient);

    EXPECT_EQ(true, amsProcessOptimizer->Init());

    APP_LOGD("AmsProcessOptimizerTest_Init_003 end.");
}

/*
 * Feature: AMS
 * Function: ProcessOptimizer
 * SubFunction: OnAppAdded
 * FunctionPoints: add application in appLru_
 * EnvConditions: Mobile that can run ohos test framework.
 * CaseDescription: Verify the function OnAppAdded can check the invalid app
 */
HWTEST_F(AmsProcessOptimizerTest, AmsProcessOptimizerTest_OnAppAdded_001, TestSize.Level1)
{
    APP_LOGD("AmsProcessOptimizerTest_OnAppAdded_001 start.");

    auto lmksClient = std::make_shared<LmksClient>();
    std::unique_ptr<ProcessOptimizer> amsProcessOptimizer = std::make_unique<ProcessOptimizer>(lmksClient);
    amsProcessOptimizer->appLru_.clear();
    amsProcessOptimizer->OnAppAdded(nullptr);
    EXPECT_TRUE(amsProcessOptimizer->appLru_.size() == 0);

    APP_LOGD("AmsProcessOptimizerTest_OnAppAdded_001 end.");
}

/*
 * Feature: AMS
 * Function: ProcessOptimizer
 * SubFunction: OnAppAdded
 * FunctionPoints: add application in appLru_
 * EnvConditions: Mobile that can run ohos test framework.
 * CaseDescription: Verify that the function OnAppAdded adds app to appLru_
 */
HWTEST_F(AmsProcessOptimizerTest, AmsProcessOptimizerTest_OnAppAdded_002, TestSize.Level1)
{
    APP_LOGD("AmsProcessOptimizerTest_OnAppAdded_002 start.");

    auto appInfo = std::make_shared<ApplicationInfo>();
    appInfo->name = APP_RECORD_NAME;
    appInfo->bundleName = APP_RECORD_NAME;  // specify process condition

    auto lmksClient = std::make_shared<LmksClient>();
    std::unique_ptr<ProcessOptimizer> amsProcessOptimizer = std::make_unique<ProcessOptimizer>(lmksClient);
    auto app = std::make_shared<AppRunningRecord>(appInfo, AppRecordId::Create(), appInfo->bundleName);
    amsProcessOptimizer->appLru_.clear();
    amsProcessOptimizer->OnAppAdded(app);
    EXPECT_TRUE(amsProcessOptimizer->appLru_.size() > 0);

    APP_LOGD("AmsProcessOptimizerTest_OnAppAdded_002 end.");
}

/*
 * Feature: AMS
 * Function: ProcessOptimizer
 * SubFunction: OnAppRemoved
 * FunctionPoints: remove application from appLru_
 * EnvConditions: Mobile that can run ohos test framework.
 * CaseDescription: Verify the function OnAppRemoved can check the invalid app
 */
HWTEST_F(AmsProcessOptimizerTest, AmsProcessOptimizerTest_OnAppRemoved_001, TestSize.Level1)
{
    APP_LOGD("AmsProcessOptimizerTest_OnAppRemoved_001 start.");

    auto appInfo = std::make_shared<ApplicationInfo>();
    appInfo->name = APP_RECORD_NAME;
    appInfo->bundleName = APP_RECORD_NAME;  // specify process condition

    auto lmksClient = std::make_shared<LmksClient>();
    std::unique_ptr<ProcessOptimizer> amsProcessOptimizer = std::make_unique<ProcessOptimizer>(lmksClient);
    auto app = std::make_shared<AppRunningRecord>(appInfo, AppRecordId::Create(), appInfo->bundleName);
    amsProcessOptimizer->appLru_.clear();
    amsProcessOptimizer->appLru_.push_front(app);
    amsProcessOptimizer->OnAppRemoved(nullptr);
    EXPECT_TRUE(amsProcessOptimizer->appLru_.size() > 0);

    APP_LOGD("AmsProcessOptimizerTest_OnAppRemoved_001 end.");
}

/*
 * Feature: AMS
 * Function: ProcessOptimizer
 * SubFunction: OnAppRemoved
 * FunctionPoints: remove application from appLru_
 * EnvConditions: Mobile that can run ohos test framework.
 * CaseDescription: Verify whether the function OnAppRemoved removes the app in appLru_
 */
HWTEST_F(AmsProcessOptimizerTest, AmsProcessOptimizerTest_OnAppRemoved_002, TestSize.Level1)
{
    APP_LOGD("AmsProcessOptimizerTest_OnAppRemoved_002 start.");

    auto appInfo = std::make_shared<ApplicationInfo>();
    appInfo->name = APP_RECORD_NAME;
    appInfo->bundleName = APP_RECORD_NAME;  // specify process condition

    auto lmksClient = std::make_shared<LmksClient>();
    std::unique_ptr<ProcessOptimizer> amsProcessOptimizer = std::make_unique<ProcessOptimizer>(lmksClient);
    auto app = std::make_shared<AppRunningRecord>(appInfo, AppRecordId::Create(), appInfo->bundleName);
    amsProcessOptimizer->appLru_.clear();
    amsProcessOptimizer->appLru_.push_front(app);
    amsProcessOptimizer->OnAppRemoved(app);
    EXPECT_TRUE(amsProcessOptimizer->appLru_.size() == 0);

    APP_LOGD("AmsProcessOptimizerTest_OnAppRemoved_002 end.");
}

/*
 * Feature: AMS
 * Function: ProcessOptimizer
 * SubFunction: OnAppStateChanged
 * FunctionPoints: change application state
 * EnvConditions: Mobile that can run ohos test framework.
 * CaseDescription: Verify whether the function OnAppStateChanged moves app2 in appLru_ to the head
 */
HWTEST_F(AmsProcessOptimizerTest, AmsProcessOptimizerTest_OnAppStateChanged_001, TestSize.Level1)
{
    APP_LOGD("AmsProcessOptimizerTest_OnAppStateChanged_001 start.");

    auto lmksClient = std::make_shared<LmksClient>();
    std::unique_ptr<ProcessOptimizer> amsProcessOptimizer = std::make_unique<ProcessOptimizer>(lmksClient);

    auto appInfo = std::make_shared<ApplicationInfo>();
    appInfo->name = APP_RECORD_NAME;
    appInfo->bundleName = APP_RECORD_NAME;  // specify process condition

    auto app1 = std::make_shared<AppRunningRecord>(appInfo, AppRecordId::Create(), appInfo->bundleName);
    app1->GetPriorityObject()->SetPid(11);
    auto app2 = std::make_shared<AppRunningRecord>(appInfo, AppRecordId::Create(), appInfo->bundleName);
    app2->GetPriorityObject()->SetPid(22);
    auto app3 = std::make_shared<AppRunningRecord>(appInfo, AppRecordId::Create(), appInfo->bundleName);
    app3->GetPriorityObject()->SetPid(33);
    amsProcessOptimizer->appLru_.clear();
    amsProcessOptimizer->appLru_.push_back(app1);
    amsProcessOptimizer->appLru_.push_back(app2);
    amsProcessOptimizer->appLru_.push_back(app3);
    app2->SetState(ApplicationState::APP_STATE_FOREGROUND);
    amsProcessOptimizer->OnAppStateChanged(app2, ApplicationState::APP_STATE_BEGIN);
    EXPECT_TRUE(app2 == amsProcessOptimizer->appLru_.front());

    APP_LOGD("AmsProcessOptimizerTest_OnAppStateChanged_001 end.");
}

/*
 * Feature: AMS
 * Function: ProcessOptimizer
 * SubFunction: SetAppOomAdj
 * FunctionPoints: set application OOM adjustment
 * EnvConditions: Mobile that can run ohos test framework.
 * CaseDescription: Verify the function SetAppOomAdj can check the invalid app
 */
HWTEST_F(AmsProcessOptimizerTest, AmsProcessOptimizerTest_SetAppOomAdj_001, TestSize.Level1)
{
    APP_LOGD("AmsProcessOptimizerTest_SetAppOomAdj_001 start.");

    auto lmksClient = std::make_shared<LmksClient>();
    std::unique_ptr<ProcessOptimizer> amsProcessOptimizer = std::make_unique<ProcessOptimizer>(lmksClient);
    int oomAdj = 1;
    EXPECT_EQ(false, amsProcessOptimizer->SetAppOomAdj(nullptr, oomAdj));

    APP_LOGD("AmsProcessOptimizerTest_SetAppOomAdj_001 end.");
}

/*
 * Feature: AMS
 * Function: ProcessOptimizer
 * SubFunction: SetAppOomAdj
 * FunctionPoints: set application OOM adjustment
 * EnvConditions: Mobile that can run ohos test framework.
 * CaseDescription: Verify the function SetAppOomAdj can check the invalid oomAdj
 */
HWTEST_F(AmsProcessOptimizerTest, AmsProcessOptimizerTest_SetAppOomAdj_002, TestSize.Level1)
{
    APP_LOGD("AmsProcessOptimizerTest_SetAppOomAdj_002 start.");

    auto appInfo = std::make_shared<ApplicationInfo>();
    appInfo->name = APP_RECORD_NAME;
    appInfo->bundleName = APP_RECORD_NAME;  // specify process condition

    auto lmksClient = std::make_shared<LmksClient>();
    std::unique_ptr<ProcessOptimizer> amsProcessOptimizer = std::make_unique<ProcessOptimizer>(lmksClient);
    int oomAdj = 1100;
    auto app = std::make_shared<AppRunningRecord>(appInfo, AppRecordId::Create(), appInfo->bundleName);
    EXPECT_EQ(false, amsProcessOptimizer->SetAppOomAdj(app, oomAdj));

    APP_LOGD("AmsProcessOptimizerTest_SetAppOomAdj_002 end.");
}

/*
 * Feature: AMS
 * Function: ProcessOptimizer
 * SubFunction: SetAppOomAdj
 * FunctionPoints: set application OOM adjustment
 * EnvConditions: Mobile that can run ohos test framework.
 * CaseDescription: Verify the function SetAppOomAdj can check the invalid lmksClient_
 */
HWTEST_F(AmsProcessOptimizerTest, AmsProcessOptimizerTest_SetAppOomAdj_003, TestSize.Level1)
{
    APP_LOGD("AmsProcessOptimizerTest_SetAppOomAdj_003 start.");

    auto appInfo = std::make_shared<ApplicationInfo>();
    appInfo->name = APP_RECORD_NAME;
    appInfo->bundleName = APP_RECORD_NAME;  // specify process condition

    std::unique_ptr<ProcessOptimizer> amsProcessOptimizer = std::make_unique<ProcessOptimizer>(nullptr);
    int oomAdj = 1;
    auto app = std::make_shared<AppRunningRecord>(appInfo, AppRecordId::Create(), appInfo->bundleName);
    EXPECT_EQ(false, amsProcessOptimizer->SetAppOomAdj(app, oomAdj));

    APP_LOGD("AmsProcessOptimizerTest_SetAppOomAdj_003 end.");
}

/*
 * Feature: AMS
 * Function: ProcessOptimizer
 * SubFunction: SetAppOomAdj
 * FunctionPoints: set application OOM adjustment
 * EnvConditions: Mobile that can run ohos test framework.
 * CaseDescription: Oom adj has no change in the verification function SetAppOomAdj
 */
HWTEST_F(AmsProcessOptimizerTest, AmsProcessOptimizerTest_SetAppOomAdj_004, TestSize.Level1)
{
    APP_LOGD("AmsProcessOptimizerTest_SetAppOomAdj_004 start.");

    auto appInfo = std::make_shared<ApplicationInfo>();
    appInfo->name = APP_RECORD_NAME;
    appInfo->bundleName = APP_RECORD_NAME;  // specify process condition

    auto lmksClient = std::make_shared<LmksClient>();
    std::unique_ptr<ProcessOptimizer> amsProcessOptimizer = std::make_unique<ProcessOptimizer>(lmksClient);
    int oomAdj = 1;
    auto app = std::make_shared<AppRunningRecord>(appInfo, AppRecordId::Create(), appInfo->bundleName);
    auto priorityObject = app->GetPriorityObject();
    EXPECT_TRUE(priorityObject != nullptr);
    priorityObject->SetCurAdj(oomAdj);
    EXPECT_EQ(true, amsProcessOptimizer->SetAppOomAdj(app, oomAdj));

    APP_LOGD("AmsProcessOptimizerTest_SetAppOomAdj_004 end.");
}

/*
 * Feature: AMS
 * Function: ProcessOptimizer
 * SubFunction: SetAppOomAdj
 * FunctionPoints: set application OOM adjustment
 * EnvConditions: Mobile that can run ohos test framework.
 * CaseDescription: Verify that the function SetAppOomAdj is executed normally
 */
HWTEST_F(AmsProcessOptimizerTest, AmsProcessOptimizerTest_SetAppOomAdj_005, TestSize.Level1)
{
    APP_LOGD("AmsProcessOptimizerTest_SetAppOomAdj_005 start.");

    auto appInfo = std::make_shared<ApplicationInfo>();
    appInfo->name = APP_RECORD_NAME;
    appInfo->bundleName = APP_RECORD_NAME;  // specify process condition

    auto lmksClient = std::make_shared<LmksClient>();
    std::unique_ptr<ProcessOptimizer> amsProcessOptimizer = std::make_unique<ProcessOptimizer>(lmksClient);
    int oomAdj = 1;
    auto app = std::make_shared<AppRunningRecord>(appInfo, AppRecordId::Create(), appInfo->bundleName);
    EXPECT_EQ(true, amsProcessOptimizer->SetAppOomAdj(app, oomAdj));

    APP_LOGD("AmsProcessOptimizerTest_SetAppOomAdj_005 end.");
}

/*
 * Feature: AMS
 * Function: ProcessOptimizer
 * SubFunction: SetAppSchedPolicy
 * FunctionPoints: set the application SchedPolicy
 * EnvConditions: Mobile that can run ohos test framework.
 * CaseDescription: Verify the function SetAppSchedPolicy can check the invalid app
 */
HWTEST_F(AmsProcessOptimizerTest, AmsProcessOptimizerTest_SetAppSchedPolicy_001, TestSize.Level1)
{
    APP_LOGD("AmsProcessOptimizerTest_SetAppSchedPolicy_001 start.");

    auto appInfo = std::make_shared<ApplicationInfo>();
    appInfo->name = APP_RECORD_NAME;
    appInfo->bundleName = APP_RECORD_NAME;  // specify process condition

    auto lmksClient = std::make_shared<LmksClient>();
    std::unique_ptr<ProcessOptimizer> amsProcessOptimizer = std::make_unique<ProcessOptimizer>(lmksClient);
    auto app = std::make_shared<AppRunningRecord>(appInfo, AppRecordId::Create(), appInfo->bundleName);
    EXPECT_EQ(
        false, amsProcessOptimizer->SetAppSchedPolicy(nullptr, CgroupManager::SchedPolicy::SCHED_POLICY_BACKGROUND));

    APP_LOGD("AmsProcessOptimizerTest_SetAppSchedPolicy_001 end.");
}

/*
 * Feature: AMS
 * Function: ProcessOptimizer
 * SubFunction: SetAppSchedPolicy
 * FunctionPoints: set the application SchedPolicy
 * EnvConditions: Mobile that can run ohos test framework.
 * CaseDescription: Check whether the incoming SchedPolicy is the same as before
 */
HWTEST_F(AmsProcessOptimizerTest, AmsProcessOptimizerTest_SetAppSchedPolicy_002, TestSize.Level1)
{
    APP_LOGD("AmsProcessOptimizerTest_SetAppSchedPolicy_002 start.");

    auto appInfo = std::make_shared<ApplicationInfo>();
    appInfo->name = APP_RECORD_NAME;
    appInfo->bundleName = APP_RECORD_NAME;  // specify process condition

    auto lmksClient = std::make_shared<LmksClient>();
    std::unique_ptr<ProcessOptimizer> amsProcessOptimizer = std::make_unique<ProcessOptimizer>(lmksClient);
    auto app = std::make_shared<AppRunningRecord>(appInfo, AppRecordId::Create(), appInfo->bundleName);
    auto priorityObject = app->GetPriorityObject();
    priorityObject->SetCurCgroup(CgroupManager::SchedPolicy::SCHED_POLICY_BACKGROUND);
    EXPECT_EQ(true, amsProcessOptimizer->SetAppSchedPolicy(app, CgroupManager::SchedPolicy::SCHED_POLICY_BACKGROUND));

    APP_LOGD("AmsProcessOptimizerTest_SetAppSchedPolicy_002 end.");
}

/*
 * Feature: AMS
 * Function: ProcessOptimizer
 * SubFunction: SetAppSchedPolicy
 * FunctionPoints: set the application SchedPolicy
 * EnvConditions: Mobile that can run ohos test framework.
 * CaseDescription: Check whether the incoming SchedPolicy is equal to CgroupManager::SCHED_POLICY_FREEZED
 */
HWTEST_F(AmsProcessOptimizerTest, AmsProcessOptimizerTest_SetAppSchedPolicy_003, TestSize.Level1)
{
    APP_LOGD("AmsProcessOptimizerTest_SetAppSchedPolicy_003 start.");

    auto appInfo = std::make_shared<ApplicationInfo>();
    appInfo->name = APP_RECORD_NAME;
    appInfo->bundleName = APP_RECORD_NAME;  // specify process condition

    auto lmksClient = std::make_shared<LmksClient>();
    std::unique_ptr<ProcessOptimizer> amsProcessOptimizer = std::make_unique<ProcessOptimizer>(lmksClient);
    auto app = std::make_shared<AppRunningRecord>(appInfo, AppRecordId::Create(), appInfo->bundleName);
    EXPECT_EQ(true, amsProcessOptimizer->SetAppSchedPolicy(app, CgroupManager::SchedPolicy::SCHED_POLICY_FREEZED));

    APP_LOGD("AmsProcessOptimizerTest_SetAppSchedPolicy_003 end.");
}

/*
 * Feature: AMS
 * Function: ProcessOptimizer
 * SubFunction: SetAppSchedPolicy
 * FunctionPoints: set the application SchedPolicy
 * EnvConditions: Mobile that can run ohos test framework.
 * CaseDescription: Check whether the old SchedPolicy is equal to CgroupManager::SCHED_POLICY_FREEZED
 */
HWTEST_F(AmsProcessOptimizerTest, AmsProcessOptimizerTest_SetAppSchedPolicy_004, TestSize.Level1)
{
    APP_LOGD("AmsProcessOptimizerTest_SetAppSchedPolicy_004 start.");

    auto appInfo = std::make_shared<ApplicationInfo>();
    appInfo->name = APP_RECORD_NAME;
    appInfo->bundleName = APP_RECORD_NAME;  // specify process condition

    auto lmksClient = std::make_shared<LmksClient>();
    std::unique_ptr<ProcessOptimizer> amsProcessOptimizer = std::make_unique<ProcessOptimizer>(lmksClient);
    auto app = std::make_shared<AppRunningRecord>(appInfo, AppRecordId::Create(), appInfo->bundleName);
    auto priorityObject = app->GetPriorityObject();
    priorityObject->SetCurCgroup(CgroupManager::SchedPolicy::SCHED_POLICY_FREEZED);
    EXPECT_EQ(true, amsProcessOptimizer->SetAppSchedPolicy(app, CgroupManager::SchedPolicy::SCHED_POLICY_BACKGROUND));

    APP_LOGD("AmsProcessOptimizerTest_SetAppSchedPolicy_004 end.");
}

/*
 * Feature: AMS
 * Function: ProcessOptimizer
 * SubFunction: OnLowMemoryAlert
 * FunctionPoints: processing when low memory
 * EnvConditions: Mobile that can run ohos test framework.
 * CaseDescription: Check whether the OnLowMemoryAlert function will remove the app in appLru_ when the pid is less than
 * zero
 */
HWTEST_F(AmsProcessOptimizerTest, AmsProcessOptimizerTest_OnLowMemoryAlert_001, TestSize.Level1)
{
    APP_LOGD("AmsProcessOptimizerTest_OnLowMemoryAlert_001 start.");

    auto appInfo = std::make_shared<ApplicationInfo>();
    appInfo->name = APP_RECORD_NAME;
    appInfo->bundleName = APP_RECORD_NAME;  // specify process condition

    auto lmksClient = std::make_shared<LmksClient>();
    std::unique_ptr<ProcessOptimizer> amsProcessOptimizer = std::make_unique<ProcessOptimizer>(lmksClient);
    auto app = std::make_shared<AppRunningRecord>(appInfo, AppRecordId::Create(), appInfo->bundleName);
    auto priorityObject = app->GetPriorityObject();
    amsProcessOptimizer->appLru_.clear();
    amsProcessOptimizer->appLru_.push_back(app);
    priorityObject->SetPid(-1);
    amsProcessOptimizer->OnLowMemoryAlert(CgroupManager::LowMemoryLevel::LOW_MEMORY_LEVEL_CRITICAL);
    EXPECT_TRUE(amsProcessOptimizer->appLru_.size() == 0);

    APP_LOGD("AmsProcessOptimizerTest_OnLowMemoryAlert_001 end.");
}

/*
 * Feature: AMS
 * Function: ProcessOptimizer
 * SubFunction: UpdateAppOomAdj
 * FunctionPoints: update application OOM adjustment
 * EnvConditions: Mobile that can run ohos test framework.
 * CaseDescription: Verify the function UpdateAppOomAdj can check the invalid app
 */
HWTEST_F(AmsProcessOptimizerTest, AmsProcessOptimizerTest_UpdateAppOomAdj_001, TestSize.Level1)
{
    APP_LOGD("AmsProcessOptimizerTest_UpdateAppOomAdj_001 start.");

    auto lmksClient = std::make_shared<LmksClient>();
    std::unique_ptr<ProcessOptimizer> amsProcessOptimizer = std::make_unique<ProcessOptimizer>(lmksClient);
    EXPECT_EQ(false, amsProcessOptimizer->UpdateAppOomAdj(nullptr));

    APP_LOGD("AmsProcessOptimizerTest_UpdateAppOomAdj_001 end.");
}

/*
 * Feature: AMS
 * Function: ProcessOptimizer
 * SubFunction: UpdateAppOomAdj
 * FunctionPoints: update application OOM adjustment
 * EnvConditions: Mobile that can run ohos test framework.
 * CaseDescription: Verify the function UpdateAppOomAdj can check the invalid state
 */
HWTEST_F(AmsProcessOptimizerTest, AmsProcessOptimizerTest_UpdateAppOomAdj_002, TestSize.Level1)
{
    APP_LOGD("AmsProcessOptimizerTest_UpdateAppOomAdj_002 start.");

    auto appInfo = std::make_shared<ApplicationInfo>();
    appInfo->name = APP_RECORD_NAME;
    appInfo->bundleName = APP_RECORD_NAME;  // specify process condition

    auto lmksClient = std::make_shared<LmksClient>();
    std::unique_ptr<ProcessOptimizer> amsProcessOptimizer = std::make_unique<ProcessOptimizer>(lmksClient);
    auto app = std::make_shared<AppRunningRecord>(appInfo, AppRecordId::Create(), appInfo->bundleName);
    app->SetState(ApplicationState::APP_STATE_FOREGROUND);
    auto priorityObject = app->GetPriorityObject();
    priorityObject->SetCurAdj(0);
    EXPECT_EQ(true, amsProcessOptimizer->UpdateAppOomAdj(app));

    APP_LOGD("AmsProcessOptimizerTest_UpdateAppOomAdj_002 end.");
}

/*
 * Feature: AMS
 * Function: ProcessOptimizer
 * SubFunction: UpdateAppOomAdj
 * FunctionPoints: update application OOM adjustment
 * EnvConditions: Mobile that can run ohos test framework.
 * CaseDescription: Verify the function UpdateAppOomAdj can check the invalid state
 */
HWTEST_F(AmsProcessOptimizerTest, AmsProcessOptimizerTest_UpdateAppOomAdj_003, TestSize.Level1)
{
    APP_LOGD("AmsProcessOptimizerTest_UpdateAppOomAdj_003 start.");

    auto appInfo = std::make_shared<ApplicationInfo>();
    appInfo->name = APP_RECORD_NAME;
    appInfo->bundleName = APP_RECORD_NAME;  // specify process condition

    auto lmksClient = std::make_shared<LmksClient>();
    std::unique_ptr<ProcessOptimizer> amsProcessOptimizer = std::make_unique<ProcessOptimizer>(lmksClient);
    auto app = std::make_shared<AppRunningRecord>(appInfo, AppRecordId::Create(), appInfo->bundleName);
    app->SetState(ApplicationState::APP_STATE_CREATE);
    auto priorityObject = app->GetPriorityObject();
    priorityObject->SetCurAdj(0);
    EXPECT_EQ(true, amsProcessOptimizer->UpdateAppOomAdj(app));

    APP_LOGD("AmsProcessOptimizerTest_UpdateAppOomAdj_003 end.");
}

/*
 * Feature: AMS
 * Function: ProcessOptimizer
 * SubFunction: UpdateAppOomAdj
 * FunctionPoints: update application OOM adjustment
 * EnvConditions: Mobile that can run ohos test framework.
 * CaseDescription: Verify the function UpdateAppOomAdj can check the invalid state
 */
HWTEST_F(AmsProcessOptimizerTest, AmsProcessOptimizerTest_UpdateAppOomAdj_004, TestSize.Level1)
{
    APP_LOGD("AmsProcessOptimizerTest_UpdateAppOomAdj_004 start.");

    auto appInfo = std::make_shared<ApplicationInfo>();
    appInfo->name = APP_RECORD_NAME;
    appInfo->bundleName = APP_RECORD_NAME;  // specify process condition

    auto lmksClient = std::make_shared<LmksClient>();
    std::unique_ptr<ProcessOptimizer> amsProcessOptimizer = std::make_unique<ProcessOptimizer>(lmksClient);
    auto app = std::make_shared<AppRunningRecord>(appInfo, AppRecordId::Create(), appInfo->bundleName);
    app->SetState(ApplicationState::APP_STATE_READY);
    auto priorityObject = app->GetPriorityObject();
    priorityObject->SetCurAdj(0);
    EXPECT_EQ(true, amsProcessOptimizer->UpdateAppOomAdj(app));

    APP_LOGD("AmsProcessOptimizerTest_UpdateAppOomAdj_004 end.");
}

/*
 * Feature: AMS
 * Function: ProcessOptimizer
 * SubFunction: UpdateAppOomAdj
 * FunctionPoints: update application OOM adjustment
 * EnvConditions: Mobile that can run ohos test framework.
 * CaseDescription: Check the execution of the UpdateAppOomAdj function when the state is normal
 */
HWTEST_F(AmsProcessOptimizerTest, AmsProcessOptimizerTest_UpdateAppOomAdj_005, TestSize.Level1)
{
    APP_LOGD("AmsProcessOptimizerTest_UpdateAppOomAdj_005 start.");

    auto appInfo = std::make_shared<ApplicationInfo>();
    appInfo->name = APP_RECORD_NAME;
    appInfo->bundleName = APP_RECORD_NAME;  // specify process condition

    auto lmksClient = std::make_shared<LmksClient>();
    std::unique_ptr<ProcessOptimizer> amsProcessOptimizer = std::make_unique<ProcessOptimizer>(lmksClient);
    auto app = std::make_shared<AppRunningRecord>(appInfo, AppRecordId::Create(), appInfo->bundleName);
    app->SetState(ApplicationState::APP_STATE_BACKGROUND);
    amsProcessOptimizer->appLru_.push_back(app);
    EXPECT_EQ(true, amsProcessOptimizer->UpdateAppOomAdj(app));
    auto priorityObject = app->GetPriorityObject();
    auto curAdj = priorityObject->GetCurAdj();
    EXPECT_TRUE(curAdj == APP_OOM_ADJ_BACKGROUND_MIN);

    APP_LOGD("AmsProcessOptimizerTest_UpdateAppOomAdj_005 end.");
}

/*
 * Feature: AMS
 * Function: ProcessOptimizer
 * SubFunction: UpdateAppOomAdj
 * FunctionPoints: update application OOM adjustment
 * EnvConditions: Mobile that can run ohos test framework.
 * CaseDescription: Check the execution of the UpdateAppOomAdj function when the state is normal
 */
HWTEST_F(AmsProcessOptimizerTest, AmsProcessOptimizerTest_UpdateAppOomAdj_006, TestSize.Level1)
{
    APP_LOGD("AmsProcessOptimizerTest_UpdateAppOomAdj_006 start.");

    auto appInfo = std::make_shared<ApplicationInfo>();
    appInfo->name = APP_RECORD_NAME;
    appInfo->bundleName = APP_RECORD_NAME;  // specify process condition

    auto lmksClient = std::make_shared<LmksClient>();
    std::unique_ptr<ProcessOptimizer> amsProcessOptimizer = std::make_unique<ProcessOptimizer>(lmksClient);
    auto app = std::make_shared<AppRunningRecord>(appInfo, AppRecordId::Create(), appInfo->bundleName);
    app->SetState(ApplicationState::APP_STATE_SUSPENDED);
    amsProcessOptimizer->appLru_.push_back(app);
    EXPECT_EQ(true, amsProcessOptimizer->UpdateAppOomAdj(app));
    auto priorityObject = app->GetPriorityObject();
    auto curAdj = priorityObject->GetCurAdj();
    EXPECT_TRUE(curAdj == APP_OOM_ADJ_SUSPEND_MIN);

    APP_LOGD("AmsProcessOptimizerTest_UpdateAppOomAdj_006 end.");
}

/*
 * Feature: AMS
 * Function: ProcessOptimizer
 * SubFunction: UpdateAppSchedPolicy
 * FunctionPoints: update application SchedPolicy
 * EnvConditions: Mobile that can run ohos test framework.
 * CaseDescription: Verify the function UpdateAppSchedPolicy can check the invalid app
 */
HWTEST_F(AmsProcessOptimizerTest, AmsProcessOptimizerTest_UpdateAppSchedPolicy_001, TestSize.Level1)
{
    APP_LOGD("AmsProcessOptimizerTest_UpdateAppSchedPolicy_001 start.");

    auto lmksClient = std::make_shared<LmksClient>();
    std::unique_ptr<ProcessOptimizer> amsProcessOptimizer = std::make_unique<ProcessOptimizer>(lmksClient);
    EXPECT_EQ(false, amsProcessOptimizer->UpdateAppSchedPolicy(nullptr));

    APP_LOGD("AmsProcessOptimizerTest_UpdateAppSchedPolicy_001 end.");
}

/*
 * Feature: AMS
 * Function: ProcessOptimizer
 * SubFunction: UpdateAppSchedPolicy
 * FunctionPoints: update application SchedPolicy
 * EnvConditions: Mobile that can run ohos test framework.
 * CaseDescription: Check whether the case ApplicationState::APP_STATE_CREATE in the switch case can be executed
 * correctly
 */
HWTEST_F(AmsProcessOptimizerTest, AmsProcessOptimizerTest_UpdateAppSchedPolicy_002, TestSize.Level1)
{
    APP_LOGD("AmsProcessOptimizerTest_UpdateAppSchedPolicy_002 start.");

    auto appInfo = std::make_shared<ApplicationInfo>();
    appInfo->name = APP_RECORD_NAME;
    appInfo->bundleName = APP_RECORD_NAME;  // specify process condition

    auto lmksClient = std::make_shared<LmksClient>();
    std::unique_ptr<ProcessOptimizer> amsProcessOptimizer = std::make_unique<ProcessOptimizer>(lmksClient);
    auto app = std::make_shared<AppRunningRecord>(appInfo, AppRecordId::Create(), appInfo->bundleName);
    app->SetState(ApplicationState::APP_STATE_CREATE);
    EXPECT_EQ(true, amsProcessOptimizer->UpdateAppSchedPolicy(app));

    APP_LOGD("AmsProcessOptimizerTest_UpdateAppSchedPolicy_002 end.");
}

/*
 * Feature: AMS
 * Function: ProcessOptimizer
 * SubFunction: UpdateAppSchedPolicy
 * FunctionPoints: update application SchedPolicy
 * EnvConditions: Mobile that can run ohos test framework.
 * CaseDescription: Check whether the case ApplicationState::APP_STATE_READY in the switch case can be executed
 * correctly
 */
HWTEST_F(AmsProcessOptimizerTest, AmsProcessOptimizerTest_UpdateAppSchedPolicy_003, TestSize.Level1)
{
    APP_LOGD("AmsProcessOptimizerTest_UpdateAppSchedPolicy_003 start.");

    auto appInfo = std::make_shared<ApplicationInfo>();
    appInfo->name = APP_RECORD_NAME;
    appInfo->bundleName = APP_RECORD_NAME;  // specify process condition

    auto lmksClient = std::make_shared<LmksClient>();
    std::unique_ptr<ProcessOptimizer> amsProcessOptimizer = std::make_unique<ProcessOptimizer>(lmksClient);
    auto app = std::make_shared<AppRunningRecord>(appInfo, AppRecordId::Create(), appInfo->bundleName);
    app->SetState(ApplicationState::APP_STATE_READY);
    EXPECT_EQ(true, amsProcessOptimizer->UpdateAppSchedPolicy(app));

    APP_LOGD("AmsProcessOptimizerTest_UpdateAppSchedPolicy_003 end.");
}

/*
 * Feature: AMS
 * Function: ProcessOptimizer
 * SubFunction: UpdateAppSchedPolicy
 * FunctionPoints: update application SchedPolicy
 * EnvConditions: Mobile that can run ohos test framework.
 * CaseDescription: Check whether the case ApplicationState::APP_STATE_FOREGROUND in the switch case can be executed
 * correctly
 */
HWTEST_F(AmsProcessOptimizerTest, AmsProcessOptimizerTest_UpdateAppSchedPolicy_004, TestSize.Level1)
{
    APP_LOGD("AmsProcessOptimizerTest_UpdateAppSchedPolicy_004 start.");

    auto appInfo = std::make_shared<ApplicationInfo>();
    appInfo->name = APP_RECORD_NAME;
    appInfo->bundleName = APP_RECORD_NAME;  // specify process condition

    auto lmksClient = std::make_shared<LmksClient>();
    std::unique_ptr<ProcessOptimizer> amsProcessOptimizer = std::make_unique<ProcessOptimizer>(lmksClient);
    auto app = std::make_shared<AppRunningRecord>(appInfo, AppRecordId::Create(), appInfo->bundleName);
    app->SetState(ApplicationState::APP_STATE_FOREGROUND);
    EXPECT_EQ(true, amsProcessOptimizer->UpdateAppSchedPolicy(app));

    APP_LOGD("AmsProcessOptimizerTest_UpdateAppSchedPolicy_004 end.");
}

/*
 * Feature: AMS
 * Function: ProcessOptimizer
 * SubFunction: UpdateAppSchedPolicy
 * FunctionPoints: update application SchedPolicy
 * EnvConditions: Mobile that can run ohos test framework.
 * CaseDescription: Check whether the case ApplicationState::APP_STATE_BACKGROUND in the switch case can be executed
 * correctly
 */
HWTEST_F(AmsProcessOptimizerTest, AmsProcessOptimizerTest_UpdateAppSchedPolicy_005, TestSize.Level1)
{
    APP_LOGD("AmsProcessOptimizerTest_UpdateAppSchedPolicy_005 start.");

    auto appInfo = std::make_shared<ApplicationInfo>();
    appInfo->name = APP_RECORD_NAME;
    appInfo->bundleName = APP_RECORD_NAME;  // specify process condition

    auto lmksClient = std::make_shared<LmksClient>();
    std::unique_ptr<ProcessOptimizer> amsProcessOptimizer = std::make_unique<ProcessOptimizer>(lmksClient);
    auto app = std::make_shared<AppRunningRecord>(appInfo, AppRecordId::Create(), appInfo->bundleName);
    app->SetState(ApplicationState::APP_STATE_BACKGROUND);
    EXPECT_EQ(true, amsProcessOptimizer->UpdateAppSchedPolicy(app));

    APP_LOGD("AmsProcessOptimizerTest_UpdateAppSchedPolicy_005 end.");
}

/*
 * Feature: AMS
 * Function: ProcessOptimizer
 * SubFunction: UpdateAppSchedPolicy
 * FunctionPoints: update application SchedPolicy
 * EnvConditions: Mobile that can run ohos test framework.
 * CaseDescription: Check whether the case ApplicationState::APP_STATE_SUSPENDED in the switch case can be executed
 * correctly
 */
HWTEST_F(AmsProcessOptimizerTest, AmsProcessOptimizerTest_UpdateAppSchedPolicy_006, TestSize.Level1)
{
    APP_LOGD("AmsProcessOptimizerTest_UpdateAppSchedPolicy_006 start.");

    auto appInfo = std::make_shared<ApplicationInfo>();
    appInfo->name = APP_RECORD_NAME;
    appInfo->bundleName = APP_RECORD_NAME;  // specify process condition

    auto lmksClient = std::make_shared<LmksClient>();
    std::unique_ptr<ProcessOptimizer> amsProcessOptimizer = std::make_unique<ProcessOptimizer>(lmksClient);
    auto app = std::make_shared<AppRunningRecord>(appInfo, AppRecordId::Create(), appInfo->bundleName);
    app->SetState(ApplicationState::APP_STATE_SUSPENDED);
    EXPECT_EQ(true, amsProcessOptimizer->UpdateAppSchedPolicy(app));

    APP_LOGD("AmsProcessOptimizerTest_UpdateAppSchedPolicy_006 end.");
}

/*
 * Feature: AMS
 * Function: ProcessOptimizer
 * SubFunction: UpdateAppSchedPolicy
 * FunctionPoints: update application SchedPolicy
 * EnvConditions: Mobile that can run ohos test framework.
 * CaseDescription: Check whether the default in switch case can be executed correctly
 */
HWTEST_F(AmsProcessOptimizerTest, AmsProcessOptimizerTest_UpdateAppSchedPolicy_007, TestSize.Level1)
{
    APP_LOGD("AmsProcessOptimizerTest_UpdateAppSchedPolicy_007 start.");

    auto appInfo = std::make_shared<ApplicationInfo>();
    appInfo->name = APP_RECORD_NAME;
    appInfo->bundleName = APP_RECORD_NAME;  // specify process condition

    auto lmksClient = std::make_shared<LmksClient>();
    std::unique_ptr<ProcessOptimizer> amsProcessOptimizer = std::make_unique<ProcessOptimizer>(lmksClient);
    auto app = std::make_shared<AppRunningRecord>(appInfo, AppRecordId::Create(), appInfo->bundleName);
    app->SetState(ApplicationState::APP_STATE_END);
    EXPECT_EQ(true, amsProcessOptimizer->UpdateAppSchedPolicy(app));

    APP_LOGD("AmsProcessOptimizerTest_UpdateAppSchedPolicy_007 end.");
}

/*
 * Feature: AMS
 * Function: ProcessOptimizer
 * SubFunction: StartAppSuspendTimer
 * FunctionPoints: start application SuspendTimer
 * EnvConditions: Mobile that can run ohos test framework.
 * CaseDescription: Verify that the function StartAppSuspendTimer can work normally.
 */
HWTEST_F(AmsProcessOptimizerTest, AmsProcessOptimizerTest_StartAppSuspendTimer_001, TestSize.Level1)
{
    APP_LOGD("AmsProcessOptimizerTest_StartAppSuspendTimer_001 start.");

    auto appInfo = std::make_shared<ApplicationInfo>();
    appInfo->name = APP_RECORD_NAME;
    appInfo->bundleName = APP_RECORD_NAME;  // specify process condition

    auto lmksClient = std::make_shared<LmksClient>();
    std::unique_ptr<ProcessOptimizer> amsProcessOptimizer =
        std::make_unique<ProcessOptimizer>(lmksClient, APP_SUSPEND_TIMEOUT_DEFAULT);
    auto app = std::make_shared<AppRunningRecord>(appInfo, AppRecordId::Create(), appInfo->bundleName);
    amsProcessOptimizer->eventHandler_ = std::make_shared<EventHandler>(EventRunner::Create());
    EXPECT_TRUE(amsProcessOptimizer->eventHandler_ != nullptr);
    amsProcessOptimizer->suspendTimers_.clear();
    amsProcessOptimizer->StartAppSuspendTimer(app);
    EXPECT_TRUE(amsProcessOptimizer->suspendTimers_.size() > 0);
    usleep(APP_USLEEP);
    EXPECT_TRUE(amsProcessOptimizer->suspendTimers_.size() == 0);
    amsProcessOptimizer->eventHandler_ = nullptr;

    APP_LOGD("AmsProcessOptimizerTest_StartAppSuspendTimer_001 end.");
}

/*
 * Feature: AMS
 * Function: ProcessOptimizer
 * SubFunction: StopAppSuspendTimer
 * FunctionPoints: stop application SuspendTimer
 * EnvConditions: Mobile that can run ohos test framework.
 * CaseDescription: Verify that the function StopAppSuspendTimer can work normally.
 */
HWTEST_F(AmsProcessOptimizerTest, AmsProcessOptimizerTest_StopAppSuspendTimer_001, TestSize.Level1)
{
    APP_LOGD("AmsProcessOptimizerTest_StopAppSuspendTimer_001 start.");

    auto appInfo = std::make_shared<ApplicationInfo>();
    appInfo->name = APP_RECORD_NAME;
    appInfo->bundleName = APP_RECORD_NAME;  // specify process condition

    auto lmksClient = std::make_shared<LmksClient>();
    std::unique_ptr<ProcessOptimizer> amsProcessOptimizer = std::make_unique<ProcessOptimizer>(lmksClient);
    auto app = std::make_shared<AppRunningRecord>(appInfo, AppRecordId::Create(), appInfo->bundleName);
    amsProcessOptimizer->eventHandler_ = std::make_shared<EventHandler>(EventRunner::Create());
    EXPECT_TRUE(amsProcessOptimizer->eventHandler_ != nullptr);
    amsProcessOptimizer->suspendTimers_.clear();
    auto timerName = amsProcessOptimizer->GetAppSuspendTimerName(app);
    amsProcessOptimizer->suspendTimers_.emplace(timerName);
    amsProcessOptimizer->StopAppSuspendTimer(app);
    auto it = amsProcessOptimizer->suspendTimers_.find(timerName);
    EXPECT_TRUE(it == amsProcessOptimizer->suspendTimers_.end());

    APP_LOGD("AmsProcessOptimizerTest_StopAppSuspendTimer_001 end.");
}
}  // namespace AppExecFwk
}  // namespace OHOS