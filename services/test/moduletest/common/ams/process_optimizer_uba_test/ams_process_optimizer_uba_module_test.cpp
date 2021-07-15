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
#define private public
#define protected public
#include "app_mgr_service.h"
#undef private
#undef protected

#include <gtest/gtest.h>
#include "app_log_wrapper.h"

const std::string APP_RECORD_NAME = "App_Name_Z";
// foreground process oom_adj
constexpr int APP_OOM_ADJ_FOREGROUND = 0;
// background process oom_adj
constexpr int APP_OOM_ADJ_BACKGROUND_MIN = 400;
// suspend process oom_adj
constexpr int APP_OOM_ADJ_SUSPEND_MIN = 600;

static constexpr int APP_SUSPEND_TIMEOUT_DEFAULT = 100;
static constexpr int APP_USLEEP = 200 * 1000;

using namespace testing::ext;

namespace OHOS {
namespace AppExecFwk {

class ProcessOptimizerModuleTest : public testing::Test {
public:
    static void SetUpTestCase();
    static void TearDownTestCase();
    void SetUp();
    void TearDown();

    void SuspendApplication(const std::shared_ptr<AppRunningRecord> &appRecord);
    void ResumeApplication(const std::shared_ptr<AppRunningRecord> &appRecord);
    std::shared_ptr<ProcessOptimizerUBA> processOptimizerUBA =
        std::make_unique<ProcessOptimizerUBA>(nullptr, nullptr, APP_SUSPEND_TIMEOUT_DEFAULT);
};

void ProcessOptimizerModuleTest::SuspendApplication(const std::shared_ptr<AppRunningRecord> &appRecord)
{
    if (!appRecord) {
        APP_LOGE("app record is null");
        return;
    }
    APP_LOGD("%{public}s : app name is %{public}s , Uid is %{public}d",
        __func__,
        appRecord->GetName().c_str(),
        appRecord->GetUid());
    // Temporary unsubscribe via UID
    appRecord->SetState(ApplicationState::APP_STATE_SUSPENDED);
    processOptimizerUBA->OnAppStateChanged(appRecord, ApplicationState::APP_STATE_BACKGROUND);
}

void ProcessOptimizerModuleTest::ResumeApplication(const std::shared_ptr<AppRunningRecord> &appRecord)
{
    if (!appRecord) {
        APP_LOGE("app record is null");
        return;
    }
    APP_LOGD("%{public}s : app name is %{public}s , Uid is %{public}d",
        __func__,
        appRecord->GetName().c_str(),
        appRecord->GetUid());
}

void ProcessOptimizerModuleTest::SetUpTestCase()
{}

void ProcessOptimizerModuleTest::TearDownTestCase()
{}

void ProcessOptimizerModuleTest::SetUp()
{}

void ProcessOptimizerModuleTest::TearDown()
{}

/*
 * Feature: AMS
 * Function: ProcessOptimizer
 * SubFunction: OnAppAdded
 * FunctionPoints: add application
 * CaseDescription: Call the OnAppAdded function to add multiple application
 */
HWTEST_F(ProcessOptimizerModuleTest, OnAppAdded_001, TestSize.Level1)
{
    APP_LOGI("ProcessOptimizerModuleTest OnAppAdded_001 start");

    bool isSuccess = processOptimizerUBA->Init();
    EXPECT_TRUE(true == isSuccess);

    auto appInfo = std::make_shared<ApplicationInfo>();
    appInfo->name = APP_RECORD_NAME;
    appInfo->bundleName = APP_RECORD_NAME;  // specify process condition

    auto app1 = std::make_shared<AppRunningRecord>(appInfo, AppRecordId::Create(), appInfo->bundleName);
    app1->GetPriorityObject()->SetPid(1000);
    auto priorityObject1 = app1->GetPriorityObject();
    processOptimizerUBA->OnAppAdded(app1);

    auto oomAdj1 = priorityObject1->GetCurAdj();
    EXPECT_TRUE(oomAdj1 == APP_OOM_ADJ_FOREGROUND);

    auto app2 = std::make_shared<AppRunningRecord>(appInfo, AppRecordId::Create(), appInfo->bundleName);
    app2->GetPriorityObject()->SetPid(1100);
    auto priorityObject2 = app2->GetPriorityObject();
    processOptimizerUBA->OnAppAdded(app2);

    auto oomAdj2 = priorityObject2->GetCurAdj();
    EXPECT_TRUE(oomAdj2 == APP_OOM_ADJ_FOREGROUND);

    oomAdj1 = priorityObject1->GetCurAdj();

    APP_LOGI("ProcessOptimizerModuleTest OnAppAdded_001 end");
}

/*
 * Feature: AMS
 * Function: ProcessOptimizer
 * SubFunction: OnAppStateChanged
 * FunctionPoints: change application status
 * CaseDescription: Set a different state, check the value of adj
 */
HWTEST_F(ProcessOptimizerModuleTest, OnAppStateChanged_001, TestSize.Level1)
{
    APP_LOGI("ProcessOptimizerModuleTest OnAppStateChanged_001 start");

    bool isSuccess = processOptimizerUBA->Init();
    EXPECT_TRUE(true == isSuccess);

    auto appInfo = std::make_shared<ApplicationInfo>();
    appInfo->name = APP_RECORD_NAME;
    appInfo->bundleName = APP_RECORD_NAME;  // specify process condition

    auto app = std::make_shared<AppRunningRecord>(appInfo, AppRecordId::Create(), appInfo->bundleName);
    app->SetState(ApplicationState::APP_STATE_FOREGROUND);
    auto priorityObject = app->GetPriorityObject();
    priorityObject->SetPid(1200);

    processOptimizerUBA->OnAppStateChanged(app, ApplicationState::APP_STATE_BACKGROUND);
    auto oomAdj = priorityObject->GetCurAdj();
    EXPECT_TRUE(oomAdj == APP_OOM_ADJ_FOREGROUND);

    APP_LOGI("ProcessOptimizerModuleTest OnAppStateChanged_001 end");
}

/*
 * Feature: AMS
 * Function: ProcessOptimizer
 * SubFunction: OnAppStateChanged
 * FunctionPoints: change application status
 * CaseDescription: Set a different state, check the value of adj
 */
HWTEST_F(ProcessOptimizerModuleTest, OnAppStateChanged_002, TestSize.Level1)
{
    APP_LOGI("ProcessOptimizerModuleTest OnAppStateChanged_002 start");

    bool isSuccess = processOptimizerUBA->Init();
    EXPECT_TRUE(true == isSuccess);

    auto appInfo = std::make_shared<ApplicationInfo>();
    appInfo->name = APP_RECORD_NAME;
    appInfo->bundleName = APP_RECORD_NAME;  // specify process condition

    auto app = std::make_shared<AppRunningRecord>(appInfo, AppRecordId::Create(), appInfo->bundleName);
    processOptimizerUBA->OnAppAdded(app);
    app->SetState(ApplicationState::APP_STATE_BACKGROUND);
    auto priorityObject = app->GetPriorityObject();
    priorityObject->SetPid(1300);

    processOptimizerUBA->OnAppStateChanged(app, ApplicationState::APP_STATE_FOREGROUND);
    auto oomAdj = priorityObject->GetCurAdj();
    EXPECT_TRUE(oomAdj == APP_OOM_ADJ_BACKGROUND_MIN);

    APP_LOGI("ProcessOptimizerModuleTest OnAppStateChanged_002 end");
}

/*
 * Feature: AMS
 * Function: ProcessOptimizer
 * SubFunction: OnAppStateChanged
 * FunctionPoints: change application status
 * CaseDescription: 1. Call the OnAppStateChanged function, make the conditions meet, call the StartAppSuspendTimer
 function, make the app state suspended
                    2. Call the OnAppStateChanged function to remove the suspended state
 */
HWTEST_F(ProcessOptimizerModuleTest, OnAppStateChanged_003, TestSize.Level1)
{
    APP_LOGI("ProcessOptimizerModuleTest OnAppStateChanged_003 start");

    processOptimizerUBA->AppSuspended =
        std::bind(&ProcessOptimizerModuleTest::SuspendApplication, this, std::placeholders::_1);
    processOptimizerUBA->AppResumed =
        std::bind(&ProcessOptimizerModuleTest::ResumeApplication, this, std::placeholders::_1);
    bool isSuccess = processOptimizerUBA->Init();
    EXPECT_TRUE(true == isSuccess);

    auto appInfo = std::make_shared<ApplicationInfo>();
    appInfo->name = APP_RECORD_NAME;
    appInfo->bundleName = APP_RECORD_NAME;  // specify process condition

    auto app = std::make_shared<AppRunningRecord>(appInfo, AppRecordId::Create(), appInfo->bundleName);
    processOptimizerUBA->OnAppAdded(app);
    app->SetState(ApplicationState::APP_STATE_BACKGROUND);
    auto priorityObject = app->GetPriorityObject();
    priorityObject->SetPid(1);
    priorityObject->SetVisibleStatus(false);
    priorityObject->SetPerceptibleStatus(false);

    processOptimizerUBA->OnAppStateChanged(app, ApplicationState::APP_STATE_FOREGROUND);
    usleep(APP_USLEEP);
    auto oomAdj = priorityObject->GetCurAdj();
    EXPECT_TRUE(oomAdj == APP_OOM_ADJ_SUSPEND_MIN);

    GTEST_LOG_(INFO) << "ProcessOptimizerModuleTest OnAppStateChanged_003 oomAdj: " << oomAdj;

    auto appState = app->GetState();
    if (appState == ApplicationState::APP_STATE_SUSPENDED) {
        app->SetState(ApplicationState::APP_STATE_BACKGROUND);
        processOptimizerUBA->OnAppStateChanged(app, ApplicationState::APP_STATE_SUSPENDED);
        oomAdj = priorityObject->GetCurAdj();
        EXPECT_TRUE(oomAdj == APP_OOM_ADJ_BACKGROUND_MIN);
    }

    APP_LOGI("ProcessOptimizerModuleTest OnAppStateChanged_003 end");
}

/*
 * Feature: AMS
 * Function: ProcessOptimizer
 * SubFunction: OnAppRemoved
 * FunctionPoints: remove application
 * CaseDescription: 1. Call the OnAppStateChanged function to make the conditions meet and call the StartAppSuspendTimer
 function
                    2. Calling the OnAppRemoved function will call the StopAppSuspendTimer function
 */
HWTEST_F(ProcessOptimizerModuleTest, OnAppRemoved_001, TestSize.Level1)
{
    APP_LOGI("ProcessOptimizerModuleTest OnAppRemoved_001 start");

    bool isSuccess = processOptimizerUBA->Init();
    EXPECT_TRUE(true == isSuccess);

    auto appInfo = std::make_shared<ApplicationInfo>();
    appInfo->name = APP_RECORD_NAME;
    appInfo->bundleName = APP_RECORD_NAME;  // specify process condition

    auto app = std::make_shared<AppRunningRecord>(appInfo, AppRecordId::Create(), appInfo->bundleName);
    processOptimizerUBA->OnAppAdded(app);
    app->SetState(ApplicationState::APP_STATE_BACKGROUND);
    auto priorityObject = app->GetPriorityObject();
    priorityObject->SetPid(1500);
    priorityObject->SetVisibleStatus(false);
    priorityObject->SetPerceptibleStatus(false);

    processOptimizerUBA->OnAppStateChanged(app, ApplicationState::APP_STATE_FOREGROUND);
    processOptimizerUBA->OnAppRemoved(app);

    auto oomAdj = priorityObject->GetCurAdj();

    EXPECT_TRUE(oomAdj == APP_OOM_ADJ_BACKGROUND_MIN);

    APP_LOGI("ProcessOptimizerModuleTest OnAppRemoved_001 end");
}

}  // namespace AppExecFwk
}  // namespace OHOS
