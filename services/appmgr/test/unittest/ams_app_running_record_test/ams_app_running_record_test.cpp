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
#include "app_mgr_service_inner.h"
#undef private

#include <limits>
#include <gtest/gtest.h>
#include "iremote_object.h"
#include "refbase.h"
#include "application_info.h"
#include "app_log_wrapper.h"
#include "app_record_id.h"
#include "app_scheduler_host.h"
#include "ability_info.h"
#include "ability_running_record.h"
#include "mock_app_scheduler.h"
#include "mock_ability_token.h"
#include "mock_app_spawn_client.h"
#include "mock_app_mgr_service_inner.h"
#include "mock_iapp_state_callback.h"
#include "mock_bundle_manager.h"

using namespace testing::ext;
using testing::_;
using testing::Return;
using testing::SetArgReferee;
namespace OHOS {
namespace AppExecFwk {
class AmsAppRunningRecordTest : public testing::Test {
public:
    static void SetUpTestCase();
    static void TearDownTestCase();
    void SetUp();
    void TearDown();

protected:
    static const std::string GetTestProcessName()
    {
        return "com.ohos.test.helloworld";
    }
    static const std::string GetTestAppName()
    {
        return "com.ohos.test.helloworld";
    }
    static const std::string GetTestAbilityName()
    {
        return "test_ability_name";
    }
    static int GetTestUid()
    {
        // a valid inner uid value which is not border value.
        const static int VALID_UID_VALUE = 1010;
        return VALID_UID_VALUE;
    }

    std::shared_ptr<AppRunningRecord> GetTestAppRunningRecord();
    sptr<IAppScheduler> GetMockedAppSchedulerClient() const;
    std::shared_ptr<AppRunningRecord> StartLoadAbility(const sptr<IRemoteObject> &token,
        const std::shared_ptr<AbilityInfo> &abilityInfo, const std::shared_ptr<ApplicationInfo> &appInfo,
        const pid_t newPid) const;
    sptr<MockAbilityToken> GetMockToken() const
    {
        return mock_token_;
    }

protected:
    std::shared_ptr<AbilityRunningRecord> testAbilityRecord_;
    sptr<IAppScheduler> client_;
    sptr<MockAppScheduler> mockAppSchedulerClient_;
    std::shared_ptr<AppRunningRecord> testAppRecord_;
    std::unique_ptr<AppMgrServiceInner> service_;
    sptr<MockAbilityToken> mock_token_;
    sptr<BundleMgrService> mockBundleMgr;
};

void AmsAppRunningRecordTest::SetUpTestCase()
{}

void AmsAppRunningRecordTest::TearDownTestCase()
{}

void AmsAppRunningRecordTest::SetUp()
{
    mockAppSchedulerClient_ = new (std::nothrow) MockAppScheduler();
    service_.reset(new (std::nothrow) AppMgrServiceInner());
    mock_token_ = new (std::nothrow) MockAbilityToken();
    client_ = iface_cast<IAppScheduler>(mockAppSchedulerClient_.GetRefPtr());
    mockBundleMgr = new (std::nothrow) BundleMgrService();
    service_->SetBundleManager(mockBundleMgr);
}

void AmsAppRunningRecordTest::TearDown()
{
    testAbilityRecord_.reset();
    testAppRecord_.reset();
}

sptr<IAppScheduler> AmsAppRunningRecordTest::GetMockedAppSchedulerClient() const
{
    if (client_) {
        return client_;
    }
    return nullptr;
}

std::shared_ptr<AppRunningRecord> AmsAppRunningRecordTest::GetTestAppRunningRecord()
{
    if (!testAppRecord_) {
        auto appInfo = std::make_shared<ApplicationInfo>();
        appInfo->name = GetTestAppName();
        testAppRecord_ = std::make_shared<AppRunningRecord>(appInfo, AppRecordId::Create(), GetTestProcessName());
        testAppRecord_->SetApplicationClient(GetMockedAppSchedulerClient());
    }
    return testAppRecord_;
}

std::shared_ptr<AppRunningRecord> AmsAppRunningRecordTest::StartLoadAbility(const sptr<IRemoteObject> &token,
    const std::shared_ptr<AbilityInfo> &abilityInfo, const std::shared_ptr<ApplicationInfo> &appInfo,
    const pid_t newPid) const
{
    std::shared_ptr<MockAppSpawnClient> mockClientPtr = std::make_shared<MockAppSpawnClient>();
    service_->SetAppSpawnClient(mockClientPtr);
    EXPECT_CALL(*mockClientPtr, StartProcess(_, _)).Times(1).WillOnce(DoAll(SetArgReferee<1>(newPid), Return(ERR_OK)));

    service_->LoadAbility(token, nullptr, abilityInfo, appInfo);

    BundleInfo bundleInfo;
    bundleInfo.appId = "com.ohos.test.helloworld_code123";

    auto record = service_->appRunningManager_->CheckAppRunningRecordIsExist(
        appInfo->name, GetTestProcessName(), appInfo->uid, bundleInfo);

    EXPECT_TRUE(record);
    auto clent = GetMockedAppSchedulerClient();
    record->SetApplicationClient(clent);
    EXPECT_EQ(record->GetPriorityObject()->GetPid(), newPid);
    EXPECT_NE(record->GetApplicationClient(), nullptr);
    return record;
}

/*
 * Feature: AMS
 * Function: AppRunningRecord
 * SubFunction: NA
 * FunctionPoints: Create using correct args with app/ability not exists.
 * EnvConditions: NA
 * CaseDescription: Call CreateAppRunningRecord to get result.
 */
HWTEST_F(AmsAppRunningRecordTest, CreateAppRunningRecord_001, TestSize.Level1)
{
    auto abilityInfo = std::make_shared<AbilityInfo>();
    abilityInfo->name = GetTestAbilityName();
    auto appInfo = std::make_shared<ApplicationInfo>();
    appInfo->name = GetTestAppName();
    BundleInfo bundleInfo;
    bundleInfo.appId = "com.ohos.test.helloworld_code123";
    bundleInfo.jointUserId = "joint456";
    HapModuleInfo hapModuleInfo;
    hapModuleInfo.moduleName = "module789";
    EXPECT_TRUE(service_ != nullptr);
    auto record = service_->CreateAppRunningRecord(
        GetMockToken(), nullptr, appInfo, abilityInfo, GetTestProcessName(), bundleInfo, hapModuleInfo);

    EXPECT_TRUE(record != nullptr);
    EXPECT_EQ(record->GetName(), GetTestAppName());

    EXPECT_EQ(record->GetProcessName(), GetTestProcessName());

    auto abilityRecord = record->GetAbilityRunningRecordByToken(GetMockToken());
    EXPECT_TRUE(abilityRecord != nullptr);
}

/*
 * Feature: AMS
 * Function: AppRunningRecord
 * SubFunction: NA
 * FunctionPoints Create using correct args with app/ability exists.
 * EnvConditions: NA
 * CaseDescription: Call CreateAppRunningRecord twice to create/get a AppRunningRecord.
 */
HWTEST_F(AmsAppRunningRecordTest, CreateAppRunningRecord_002, TestSize.Level1)
{
    auto abilityInfo = std::make_shared<AbilityInfo>();
    abilityInfo->name = GetTestAbilityName();
    auto appInfo = std::make_shared<ApplicationInfo>();
    appInfo->name = GetTestAppName();
    abilityInfo->applicationInfo.uid = 1010;
    appInfo->uid = 1010;
    BundleInfo bundleInfo;
    bundleInfo.appId = "com.ohos.test.helloworld_code123";
    bundleInfo.jointUserId = "joint456";
    HapModuleInfo hapModuleInfo;
    hapModuleInfo.moduleName = "module789";
    EXPECT_TRUE(service_ != nullptr);
    // Create
    sptr<IRemoteObject> token = GetMockToken();
    auto record = service_->CreateAppRunningRecord(
        GetMockToken(), nullptr, appInfo, abilityInfo, GetTestProcessName(), bundleInfo, hapModuleInfo);
    record->SetUid(1010);
    // Get
    auto record1 = service_->CreateAppRunningRecord(
        GetMockToken(), nullptr, appInfo, abilityInfo, GetTestProcessName(), bundleInfo, hapModuleInfo);
    EXPECT_TRUE(record1 != nullptr);
    EXPECT_EQ(record1->GetName(), GetTestAppName());
    EXPECT_EQ(record1->GetProcessName(), GetTestProcessName());
}

/*
 * Feature: AMS
 * Function: AppRunningRecord
 * SubFunction: NA
 * FunctionPoints: Create using correct args with app exists but ability not.
 * EnvConditions: NA
 * CaseDescription: Call CreateAppRunningRecord twice which second call uses a different ability info.
 */
HWTEST_F(AmsAppRunningRecordTest, CreateAppRunningRecord_003, TestSize.Level1)
{
    auto abilityInfo = std::make_shared<AbilityInfo>();
    abilityInfo->name = GetTestAbilityName();
    auto appInfo = std::make_shared<ApplicationInfo>();
    appInfo->name = GetTestAppName();
    appInfo->bundleName = GetTestAppName();
    abilityInfo->applicationInfo.uid = 1010;
    appInfo->uid = 1010;
    BundleInfo bundleInfo;
    bundleInfo.appId = "com.ohos.test.helloworld_code123";
    bundleInfo.jointUserId = "joint456";
    HapModuleInfo hapModuleInfo;
    hapModuleInfo.moduleName = "module789";
    EXPECT_TRUE(service_ != nullptr);
    auto record = service_->CreateAppRunningRecord(
        GetMockToken(), nullptr, appInfo, abilityInfo, GetTestProcessName(), bundleInfo, hapModuleInfo);
    record->SetUid(1010);

    auto anotherAbilityInfo = std::make_shared<AbilityInfo>();
    anotherAbilityInfo->name = "Another_ability";
    anotherAbilityInfo->applicationInfo.uid = 1010;
    sptr<IRemoteObject> anotherToken = new (std::nothrow) MockAbilityToken();
    auto record1 = service_->CreateAppRunningRecord(
        GetMockToken(), anotherToken, appInfo, anotherAbilityInfo, GetTestProcessName(), bundleInfo, hapModuleInfo);
    EXPECT_EQ(record1->GetName(), GetTestAppName());
    EXPECT_EQ(record1->GetProcessName(), GetTestProcessName());

    auto abilityRecord = record1->GetAbilityRunningRecordByToken(GetMockToken());
    EXPECT_TRUE(abilityRecord != nullptr);
}

/*
 * Feature: AMS
 * Function: AppRunningRecord
 * SubFunction: NA
 * FunctionPoints: Create using empty appInfo.
 * EnvConditions: NA
 * CaseDescription: Call CreateAppRunningRecord using empty appInfo.
 */
HWTEST_F(AmsAppRunningRecordTest, CreateAppRunningRecord_004, TestSize.Level1)
{
    auto abilityInfo = std::make_shared<AbilityInfo>();
    abilityInfo->name = GetTestAbilityName();
    auto appInfo = std::make_shared<ApplicationInfo>();
    appInfo->name = GetTestAppName();
    BundleInfo bundleInfo;
    bundleInfo.appId = "com.ohos.test.helloworld_code123";
    bundleInfo.jointUserId = "joint456";
    HapModuleInfo hapModuleInfo;
    hapModuleInfo.moduleName = "module789";
    EXPECT_TRUE(service_ != nullptr);
    // Create
    auto record = service_->CreateAppRunningRecord(
        GetMockToken(), nullptr, nullptr, abilityInfo, GetTestProcessName(), bundleInfo, hapModuleInfo);
    EXPECT_TRUE(record == nullptr);
}

/*
 * Feature: AMS
 * Function: AppRunningRecord
 * SubFunction: NA
 * FunctionPoints: Create using empty abilityInfo.
 * EnvConditions: NA
 * CaseDescription: Call CreateAppRunningRecord using empty abilityInfo.
 */
HWTEST_F(AmsAppRunningRecordTest, CreateAppRunningRecord_005, TestSize.Level1)
{
    auto appInfo = std::make_shared<ApplicationInfo>();
    appInfo->name = GetTestAppName();
    BundleInfo bundleInfo;
    bundleInfo.appId = "com.ohos.test.helloworld_code123";
    bundleInfo.jointUserId = "joint456";
    HapModuleInfo hapModuleInfo;
    hapModuleInfo.moduleName = "module789";
    EXPECT_TRUE(service_ != nullptr);
    // Create
    auto record = service_->CreateAppRunningRecord(
        GetMockToken(), nullptr, appInfo, nullptr, GetTestProcessName(), bundleInfo, hapModuleInfo);
    EXPECT_TRUE(record != nullptr);
}

/*
 * Feature: AMS
 * Function: AppRunningRecord
 * SubFunction: NA
 * FunctionPoints: Test launch application.
 * EnvConditions: NA
 * CaseDescription: Create an AppRunningRecord and call LaunchApplication.
 */
HWTEST_F(AmsAppRunningRecordTest, LaunchApplication_001, TestSize.Level1)
{
    auto record = GetTestAppRunningRecord();
    EXPECT_CALL(*mockAppSchedulerClient_, ScheduleLaunchApplication(_)).Times(1);
    record->LaunchApplication();
}

/*
 * Feature: AMS
 * Function: AppRunningRecord
 * SubFunction: NA
 * FunctionPoints: Test launch ability via AppRunningRecord using valid name.
 * EnvConditions: NA
 * CaseDescription: Create an AppRunningRecord and call LaunchAbility which is exists.
 */
HWTEST_F(AmsAppRunningRecordTest, LaunchAbility_001, TestSize.Level1)
{
    auto appInfo = std::make_shared<ApplicationInfo>();
    appInfo->name = GetTestAppName();
    HapModuleInfo hapModuleInfo;
    hapModuleInfo.moduleName = "module789";
    auto record = GetTestAppRunningRecord();
    EXPECT_TRUE(record);
    record->AddModule(appInfo, nullptr, GetMockToken(), hapModuleInfo);
    auto moduleRecord = record->GetModuleRecordByModuleName(appInfo->bundleName, hapModuleInfo.moduleName);
    EXPECT_TRUE(moduleRecord);
    auto abilityRecord = moduleRecord->GetAbilityRunningRecordByToken(GetMockToken());
    EXPECT_EQ(nullptr, abilityRecord);
    EXPECT_CALL(*mockAppSchedulerClient_, ScheduleLaunchAbility(_, _)).Times(0);
    record->LaunchAbility(abilityRecord);
}

/*
 * Feature: AMS
 * Function: AppRunningRecord
 * SubFunction: NA
 * FunctionPoints: Test launch ability via AppRunningRecord using empty name.
 * EnvConditions: NA
 * CaseDescription: Create an AppRunningRecord and call LaunchAbility which is not exists.
 */
HWTEST_F(AmsAppRunningRecordTest, LaunchAbility_002, TestSize.Level1)
{
    auto appInfo = std::make_shared<ApplicationInfo>();
    appInfo->name = GetTestAppName();
    auto abilityInfo = std::make_shared<AbilityInfo>();
    abilityInfo->name = GetTestAbilityName();
    HapModuleInfo hapModuleInfo;
    hapModuleInfo.moduleName = "module789";
    auto record = GetTestAppRunningRecord();
    record->AddModule(appInfo, abilityInfo, GetMockToken(), hapModuleInfo);
    auto moduleRecord = record->GetModuleRecordByModuleName(appInfo->bundleName, hapModuleInfo.moduleName);
    EXPECT_TRUE(moduleRecord);
    auto abilityRecord = moduleRecord->GetAbilityRunningRecordByToken(GetMockToken());

    EXPECT_TRUE(abilityRecord);
    EXPECT_CALL(*mockAppSchedulerClient_, ScheduleLaunchAbility(_, _)).Times(1);

    record->LaunchAbility(abilityRecord);

    EXPECT_EQ(AbilityState::ABILITY_STATE_READY, abilityRecord->GetState());
}

/*
 * Feature: AMS
 * Function: AppRunningRecord
 * SubFunction: NA
 * FunctionPoints: Schedule application terminate by AppRunningRecord.
 * EnvConditions: NA
 * CaseDescription: Create an AppRunningRecord and call ScheduleTerminate.
 */
HWTEST_F(AmsAppRunningRecordTest, ScheduleTerminate_001, TestSize.Level1)
{
    auto record = GetTestAppRunningRecord();
    EXPECT_CALL(*mockAppSchedulerClient_, ScheduleTerminateApplication()).Times(1);
    record->ScheduleTerminate();
}

/*
 * Feature: AMS
 * Function: AppRunningRecord
 * SubFunction: NA
 * FunctionPoints: Schedule application foreground by AppRunningRecord.
 * EnvConditions: NA
 * CaseDescription: Create an AppRunningRecord and call ScheduleForegroundRunning.
 */
HWTEST_F(AmsAppRunningRecordTest, ScheduleForegroundRunning_001, TestSize.Level1)
{
    auto record = GetTestAppRunningRecord();
    EXPECT_CALL(*mockAppSchedulerClient_, ScheduleForegroundApplication()).Times(1);
    record->ScheduleForegroundRunning();
}

/*
 * Feature: AMS
 * Function: AppRunningRecord
 * SubFunction: NA
 * FunctionPoints: Schedule application background by AppRunningRecord.
 * EnvConditions: NA
 * CaseDescription: Create an AppRunningRecord and call ScheduleBackgroundRunning.
 */
HWTEST_F(AmsAppRunningRecordTest, ScheduleBackgroundRunning_001, TestSize.Level1)
{
    auto record = GetTestAppRunningRecord();
    EXPECT_CALL(*mockAppSchedulerClient_, ScheduleBackgroundApplication()).Times(1);
    record->ScheduleBackgroundRunning();
}

/*
 * Feature: AMS
 * Function: AppRunningRecord
 * SubFunction: NA
 * FunctionPoints: Schedule application trim memory by AppRunningRecord.
 * EnvConditions: NA
 * CaseDescription: Create an AppRunningRecord and call ScheduleTrimMemory.
 */
HWTEST_F(AmsAppRunningRecordTest, ScheduleTrimMemory_001, TestSize.Level1)
{
    auto record = GetTestAppRunningRecord();
    EXPECT_CALL(*mockAppSchedulerClient_, ScheduleShrinkMemory(_)).Times(1);
    EXPECT_NE(nullptr, record->GetPriorityObject());
    record->ScheduleTrimMemory();
}

/*
 * Feature: AMS
 * Function: AppRunningRecord
 * SubFunction: NA
 * FunctionPoints: Test low memory warning notification handling.
 * EnvConditions: NA
 * CaseDescription: Create an AppRunningRecord and call LowMemoryWarning.
 */
HWTEST_F(AmsAppRunningRecordTest, LowMemoryWarning_001, TestSize.Level1)
{
    auto record = GetTestAppRunningRecord();
    EXPECT_CALL(*mockAppSchedulerClient_, ScheduleLowMemory()).Times(1);
    record->LowMemoryWarning();
}

/*
 * Feature: AMS
 * Function: AppRunningRecord
 * SubFunction: NA
 * FunctionPoints: Update application state using correct args.
 * EnvConditions: NA
 * CaseDescription: Create an AppRunningRecord and call SetState in a for-each cycle.
 */
HWTEST_F(AmsAppRunningRecordTest, UpdateAppRunningRecord_001, TestSize.Level1)
{
    auto record = GetTestAppRunningRecord();
    for (ApplicationState state = ApplicationState::APP_STATE_BEGIN; state < ApplicationState::APP_STATE_END;
         state = (ApplicationState)(static_cast<std::underlying_type<ApplicationState>::type>(state) + 1)) {
        record->SetState(state);
        EXPECT_EQ(record->GetState(), state);
    }
}

/*
 * Feature: AMS
 * Function: AppRunningRecord
 * SubFunction: NA
 * FunctionPoints: Update application state using wrong args.
 * EnvConditions: NA
 * CaseDescription: Create an AppRunningRecord and call SetState using arg |APP_STATE_END|.
 */
HWTEST_F(AmsAppRunningRecordTest, UpdateAppRunningRecord_002, TestSize.Level1)
{
    auto record = GetTestAppRunningRecord();
    record->SetState(ApplicationState::APP_STATE_END);
    EXPECT_NE(record->GetState(), ApplicationState::APP_STATE_END);
}

/*
 * Feature: AMS
 * Function: AppRunningRecord
 * SubFunction: NA
 * FunctionPoints: Delete application record info when application terminated.
 * EnvConditions: NA
 * CaseDescription: Create an AppRunningRecord and call AppMgrService::ApplicationTerminated passing exists
 |RecordId|.
 */
HWTEST_F(AmsAppRunningRecordTest, DeleteAppRunningRecord_001, TestSize.Level1)
{
    auto abilityInfo = std::make_shared<AbilityInfo>();
    abilityInfo->name = GetTestAbilityName();
    auto appInfo = std::make_shared<ApplicationInfo>();
    appInfo->name = GetTestAppName();
    BundleInfo bundleInfo;
    bundleInfo.appId = "com.ohos.test.helloworld_code123";
    bundleInfo.jointUserId = "joint456";
    HapModuleInfo hapModuleInfo;
    hapModuleInfo.moduleName = "module789";
    EXPECT_TRUE(service_ != nullptr);
    auto record = service_->CreateAppRunningRecord(
        GetMockToken(), nullptr, appInfo, abilityInfo, GetTestProcessName(), bundleInfo, hapModuleInfo);
    EXPECT_TRUE(record != nullptr);
    record->SetState(ApplicationState::APP_STATE_BACKGROUND);
    record->SetApplicationClient(GetMockedAppSchedulerClient());
    service_->ApplicationTerminated(record->GetRecordId());
    record = service_->GetAppRunningRecordByAppRecordId(record->GetRecordId());
    EXPECT_TRUE(record == nullptr);
}

/*
 * Feature: AMS
 * Function: AppRunningRecord
 * SubFunction: NA
 * FunctionPoints: When server received attachApplication request.
 * EnvConditions: NA
 * CaseDescription: Test server received normal pid attachApplication request.
 */
HWTEST_F(AmsAppRunningRecordTest, AttachApplication_001, TestSize.Level1)
{
    APP_LOGI("AmsAppRunningRecordTest AttachApplication_001 start");
    auto abilityInfo = std::make_shared<AbilityInfo>();
    abilityInfo->name = GetTestAbilityName();
    abilityInfo->applicationName = GetTestAppName();
    abilityInfo->process = GetTestAppName();
    abilityInfo->applicationInfo.bundleName = GetTestAppName();
    auto appInfo = std::make_shared<ApplicationInfo>();
    appInfo->name = GetTestAppName();
    appInfo->bundleName = GetTestAppName();
    sptr<IRemoteObject> token = GetMockToken();

    const pid_t newPid = 1234;
    EXPECT_TRUE(service_);
    auto record = StartLoadAbility(token, abilityInfo, appInfo, newPid);

    EXPECT_CALL(*mockAppSchedulerClient_, ScheduleLaunchApplication(_)).Times(1);
    EXPECT_CALL(*mockAppSchedulerClient_, ScheduleLaunchAbility(_, _)).Times(1);
    service_->AttachApplication(newPid, mockAppSchedulerClient_);
    EXPECT_EQ(record->GetState(), ApplicationState::APP_STATE_READY);
    APP_LOGI("AmsAppRunningRecordTest AttachApplication_001 end");
}

/*
 * Feature: AMS
 * Function: AppRunningRecord
 * SubFunction: NA
 * FunctionPoints: When server received attachApplication request.
 * EnvConditions: NA
 * CaseDescription: Test server received invalid pid attachApplication request.
 */
HWTEST_F(AmsAppRunningRecordTest, AttachApplication_002, TestSize.Level1)
{
    APP_LOGI("AmsAppRunningRecordTest AttachApplication_002 start");
    auto abilityInfo = std::make_shared<AbilityInfo>();
    abilityInfo->name = GetTestAbilityName();
    abilityInfo->applicationName = GetTestAppName();
    abilityInfo->process = GetTestAppName();
    abilityInfo->applicationInfo.bundleName = GetTestAppName();
    auto appInfo = std::make_shared<ApplicationInfo>();
    appInfo->name = GetTestAppName();
    appInfo->bundleName = GetTestAppName();
    sptr<IRemoteObject> token = GetMockToken();
    EXPECT_TRUE(service_ != nullptr);
    const pid_t newPid = 1234;
    const pid_t invalidPid = -1;
    auto record = StartLoadAbility(token, abilityInfo, appInfo, newPid);

    EXPECT_CALL(*mockAppSchedulerClient_, ScheduleLaunchApplication(_)).Times(0);
    service_->AttachApplication(invalidPid, GetMockedAppSchedulerClient());
    EXPECT_EQ(record->GetState(), ApplicationState::APP_STATE_CREATE);
    APP_LOGI("AmsAppRunningRecordTest AttachApplication_002 end");
}

/*
 * Feature: AMS
 * Function: AppRunningRecord
 * SubFunction: NA
 * FunctionPoints: When server received attachApplication request.
 * EnvConditions: NA
 * CaseDescription: Test server received non-exist pid attachApplication request.
 */
HWTEST_F(AmsAppRunningRecordTest, AttachApplication_003, TestSize.Level1)
{
    APP_LOGI("AmsAppRunningRecordTest AttachApplication_003 start");
    auto abilityInfo = std::make_shared<AbilityInfo>();
    abilityInfo->name = GetTestAbilityName();
    abilityInfo->applicationName = GetTestAppName();
    abilityInfo->process = GetTestAppName();
    abilityInfo->applicationInfo.bundleName = GetTestAppName();
    auto appInfo = std::make_shared<ApplicationInfo>();
    appInfo->name = GetTestAppName();
    appInfo->bundleName = GetTestAppName();
    sptr<IRemoteObject> token = GetMockToken();
    EXPECT_TRUE(service_ != nullptr);
    const pid_t newPid = 1234;
    const pid_t anotherPid = 1000;
    auto record = StartLoadAbility(token, abilityInfo, appInfo, newPid);

    EXPECT_CALL(*mockAppSchedulerClient_, ScheduleLaunchApplication(_)).Times(0);
    service_->AttachApplication(anotherPid, GetMockedAppSchedulerClient());
    EXPECT_EQ(record->GetState(), ApplicationState::APP_STATE_CREATE);
    APP_LOGI("AmsAppRunningRecordTest AttachApplication_003 end");
}

/*
 * Feature: AMS
 * Function: AppRunningRecord
 * SubFunction: NA
 * FunctionPoints: When server received attachApplication request.
 * EnvConditions: NA
 * CaseDescription: Test server received null appClient attachApplication request.
 */
HWTEST_F(AmsAppRunningRecordTest, AttachApplication_004, TestSize.Level1)
{
    APP_LOGI("AmsAppRunningRecordTest AttachApplication_004 start");
    auto abilityInfo = std::make_shared<AbilityInfo>();
    abilityInfo->name = GetTestAbilityName();
    abilityInfo->applicationName = GetTestAppName();
    abilityInfo->process = GetTestAppName();
    abilityInfo->applicationInfo.bundleName = GetTestAppName();

    auto appInfo = std::make_shared<ApplicationInfo>();
    appInfo->name = GetTestAppName();
    appInfo->bundleName = GetTestAppName();
    sptr<IRemoteObject> token = GetMockToken();
    EXPECT_TRUE(service_ != nullptr);
    const pid_t newPid = 1234;
    auto record = StartLoadAbility(token, abilityInfo, appInfo, newPid);
    service_->AttachApplication(newPid, nullptr);
    EXPECT_EQ(record->GetState(), ApplicationState::APP_STATE_CREATE);
    APP_LOGI("AmsAppRunningRecordTest AttachApplication_004 end");
}

/*
 * Feature: AMS
 * Function: AppRunningRecord
 * SubFunction: NA
 * FunctionPoints: When server received attachApplication request.
 * EnvConditions: NA
 * CaseDescription: Test server received multiple same attachApplication request.
 */
HWTEST_F(AmsAppRunningRecordTest, AttachApplication_005, TestSize.Level1)
{
    APP_LOGI("AmsAppRunningRecordTest AttachApplication_005 start");
    auto abilityInfo = std::make_shared<AbilityInfo>();
    abilityInfo->name = GetTestAbilityName();
    abilityInfo->applicationName = GetTestAppName();
    abilityInfo->process = GetTestAppName();
    abilityInfo->applicationInfo.bundleName = GetTestAppName();

    auto appInfo = std::make_shared<ApplicationInfo>();
    appInfo->name = GetTestAppName();
    appInfo->bundleName = GetTestAppName();

    sptr<IRemoteObject> token = GetMockToken();
    const pid_t newPid = 1234;
    EXPECT_TRUE(service_ != nullptr);
    auto record = StartLoadAbility(token, abilityInfo, appInfo, newPid);

    EXPECT_CALL(*mockAppSchedulerClient_, ScheduleLaunchApplication(_)).Times(1);
    EXPECT_CALL(*mockAppSchedulerClient_, ScheduleLaunchAbility(_, _)).Times(1);
    service_->AttachApplication(newPid, GetMockedAppSchedulerClient());
    EXPECT_NE(record->GetApplicationClient(), nullptr);
    EXPECT_EQ(record->GetState(), ApplicationState::APP_STATE_READY);

    EXPECT_CALL(*mockAppSchedulerClient_, ScheduleLaunchApplication(_)).Times(0);
    service_->AttachApplication(newPid, GetMockedAppSchedulerClient());
    EXPECT_EQ(record->GetState(), ApplicationState::APP_STATE_READY);
    APP_LOGI("AmsAppRunningRecordTest AttachApplication_005 end");
}

/*
 * Feature: AMS
 * Function: AppRunningRecord
 * SubFunction: NA
 * FunctionPoints: When server received attachApplication request.
 * EnvConditions: NA
 * CaseDescription: Test server received attachApplication request after multiple loadAbility.
 */
HWTEST_F(AmsAppRunningRecordTest, AttachApplication_006, TestSize.Level1)
{
    APP_LOGI("AmsAppRunningRecordTest AttachApplication_006 start");
    auto abilityInfo = std::make_shared<AbilityInfo>();
    abilityInfo->name = GetTestAbilityName();
    abilityInfo->applicationName = GetTestAppName();
    abilityInfo->process = GetTestAppName();
    abilityInfo->applicationInfo.bundleName = GetTestAppName();

    auto abilityInfo2 = std::make_shared<AbilityInfo>();
    abilityInfo2->name = GetTestAbilityName() + "_1";
    abilityInfo2->applicationName = GetTestAppName();
    abilityInfo2->process = GetTestAppName();

    auto abilityInfo3 = std::make_shared<AbilityInfo>();
    abilityInfo3->name = GetTestAbilityName() + "_2";
    abilityInfo3->applicationName = GetTestAppName();
    abilityInfo3->process = GetTestAppName();

    auto appInfo = std::make_shared<ApplicationInfo>();
    appInfo->name = GetTestAppName();
    appInfo->bundleName = GetTestAppName();

    sptr<IRemoteObject> token = GetMockToken();
    const uint32_t EXPECT_RECORD_SIZE = 3;
    const int EXPECT_ABILITY_LAUNCH_TIME = 3;
    const pid_t PID = 1234;
    EXPECT_TRUE(service_ != nullptr);
    auto record = StartLoadAbility(token, abilityInfo, appInfo, PID);

    sptr<IRemoteObject> token2 = new (std::nothrow) MockAbilityToken();
    service_->LoadAbility(token2, nullptr, abilityInfo2, appInfo);
    sptr<IRemoteObject> token3 = new (std::nothrow) MockAbilityToken();
    service_->LoadAbility(token3, nullptr, abilityInfo3, appInfo);
    EXPECT_EQ(record->GetAbilities().size(), EXPECT_RECORD_SIZE);

    EXPECT_CALL(*mockAppSchedulerClient_, ScheduleLaunchApplication(_)).Times(1);
    EXPECT_CALL(*mockAppSchedulerClient_, ScheduleLaunchAbility(_, _)).Times(EXPECT_ABILITY_LAUNCH_TIME);
    service_->AttachApplication(PID, mockAppSchedulerClient_);
    EXPECT_NE(record->GetApplicationClient(), nullptr);
    EXPECT_EQ(record->GetState(), ApplicationState::APP_STATE_READY);
    APP_LOGI("AmsAppRunningRecordTest AttachApplication_006 end");
}

/*
 * Feature: AMS
 * Function: AppRunningRecord
 * SubFunction: NA
 * FunctionPoints: When server LaunchApplication and LaunchAbility.
 * EnvConditions: NA
 * CaseDescription: Test normal case of LaunchAbility after LaunchApplication.
 */
HWTEST_F(AmsAppRunningRecordTest, LaunchAbilityForApp_001, TestSize.Level1)
{
    APP_LOGI("AmsAppRunningRecordTest LaunchAbilityForApp_001 start");
    auto abilityInfo = std::make_shared<AbilityInfo>();
    abilityInfo->name = GetTestAbilityName();
    abilityInfo->applicationName = GetTestAppName();
    abilityInfo->process = GetTestAppName();
    abilityInfo->applicationInfo.bundleName = GetTestAppName();

    auto appInfo = std::make_shared<ApplicationInfo>();
    BundleInfo bundleInfo;
    bundleInfo.appId = "com.ohos.test.helloworld_code123";
    bundleInfo.jointUserId = "joint456";
    HapModuleInfo hapModuleInfo;
    hapModuleInfo.moduleName = "module789";

    EXPECT_TRUE(service_ != nullptr);
    std::shared_ptr<AppRunningRecord> record = service_->CreateAppRunningRecord(
        GetMockToken(), nullptr, appInfo, abilityInfo, GetTestProcessName(), bundleInfo, hapModuleInfo);
    auto abilityRecord = record->GetAbilityRunningRecord(GetTestAbilityName());
    EXPECT_TRUE(abilityRecord != nullptr);

    EXPECT_CALL(*mockAppSchedulerClient_, ScheduleLaunchApplication(_)).Times(1);
    EXPECT_CALL(*mockAppSchedulerClient_, ScheduleLaunchAbility(_, _)).Times(1);
    record->SetApplicationClient(GetMockedAppSchedulerClient());
    service_->LaunchApplication(record);
    EXPECT_EQ(record->GetState(), ApplicationState::APP_STATE_READY);
    APP_LOGI("AmsAppRunningRecordTest LaunchAbilityForApp_001 end");
}

/*
 * Feature: AMS
 * Function: AppRunningRecord
 * SubFunction: NA
 * FunctionPoints: When server LaunchApplication and LaunchAbility.
 * EnvConditions: NA
 * CaseDescription: Test normal case of multiple LaunchAbility after LaunchApplication.
 */
HWTEST_F(AmsAppRunningRecordTest, LaunchAbilityForApp_002, TestSize.Level1)
{
    APP_LOGI("AmsAppRunningRecordTest LaunchAbilityForApp_002 start");
    auto abilityInfo = std::make_shared<AbilityInfo>();
    abilityInfo->name = GetTestAbilityName();
    abilityInfo->applicationName = GetTestAppName();
    auto abilityInfo2 = std::make_shared<AbilityInfo>();
    abilityInfo2->name = GetTestAbilityName() + "_1";
    abilityInfo2->applicationName = GetTestAppName();
    auto abilityInfo3 = std::make_shared<AbilityInfo>();
    abilityInfo3->name = GetTestAbilityName() + "_2";
    abilityInfo3->applicationName = GetTestAppName();
    auto appInfo = std::make_shared<ApplicationInfo>();
    appInfo->name = GetTestAppName();
    BundleInfo bundleInfo;
    bundleInfo.appId = "com.ohos.test.helloworld_code123";
    bundleInfo.jointUserId = "joint456";
    HapModuleInfo hapModuleInfo;
    hapModuleInfo.moduleName = "module789";
    const int EXPECT_ABILITY_LAUNCH_TIME = 3;
    EXPECT_TRUE(service_ != nullptr);

    std::shared_ptr<AppRunningRecord> record = service_->CreateAppRunningRecord(
        GetMockToken(), nullptr, appInfo, abilityInfo, GetTestProcessName(), bundleInfo, hapModuleInfo);
    auto abilityRecord = record->GetAbilityRunningRecord(GetTestAbilityName());
    EXPECT_TRUE(abilityRecord != nullptr);

    sptr<IRemoteObject> token2 = new (std::nothrow) MockAbilityToken();
    record->AddModule(appInfo, abilityInfo2, token2, hapModuleInfo);
    auto moduleRecord = record->GetModuleRecordByModuleName(appInfo->bundleName, hapModuleInfo.moduleName);
    EXPECT_TRUE(moduleRecord);
    auto abilityRecord2 = moduleRecord->GetAbilityRunningRecordByToken(token2);
    EXPECT_TRUE(abilityRecord2 != nullptr);

    sptr<IRemoteObject> token3 = new (std::nothrow) MockAbilityToken();
    record->AddModule(appInfo, abilityInfo3, token3, hapModuleInfo);
    auto moduleRecord3 = record->GetModuleRecordByModuleName(appInfo->bundleName, hapModuleInfo.moduleName);
    EXPECT_TRUE(moduleRecord3);
    auto abilityRecord3 = moduleRecord3->GetAbilityRunningRecordByToken(token3);
    EXPECT_TRUE(abilityRecord3 != nullptr);

    EXPECT_CALL(*mockAppSchedulerClient_, ScheduleLaunchApplication(_)).Times(1);
    EXPECT_CALL(*mockAppSchedulerClient_, ScheduleLaunchAbility(_, _)).Times(EXPECT_ABILITY_LAUNCH_TIME);
    record->SetApplicationClient(GetMockedAppSchedulerClient());
    service_->LaunchApplication(record);
    EXPECT_EQ(record->GetState(), ApplicationState::APP_STATE_READY);
    APP_LOGI("AmsAppRunningRecordTest LaunchAbilityForApp_002 end");
}

/*
 * Feature: AMS
 * Function: AppRunningRecord
 * SubFunction: NA
 * FunctionPoints: When server LaunchApplication and LaunchAbility.
 * EnvConditions: NA
 * CaseDescription: Test abnormal case of LaunchApplication with wrong state.
 */
HWTEST_F(AmsAppRunningRecordTest, LaunchAbilityForApp_003, TestSize.Level1)
{
    APP_LOGI("AmsAppRunningRecordTest LaunchAbilityForApp_003 start");
    auto abilityInfo = std::make_shared<AbilityInfo>();
    abilityInfo->name = GetTestAbilityName();
    abilityInfo->applicationName = GetTestAppName();
    auto appInfo = std::make_shared<ApplicationInfo>();
    appInfo->name = GetTestAppName();
    BundleInfo bundleInfo;
    bundleInfo.appId = "com.ohos.test.helloworld_code123";
    bundleInfo.jointUserId = "joint456";
    HapModuleInfo hapModuleInfo;
    hapModuleInfo.moduleName = "module789";
    EXPECT_TRUE(service_ != nullptr);

    std::shared_ptr<AppRunningRecord> record = service_->CreateAppRunningRecord(
        GetMockToken(), nullptr, appInfo, abilityInfo, GetTestProcessName(), bundleInfo, hapModuleInfo);
    record->SetState(ApplicationState::APP_STATE_READY);
    record->SetApplicationClient(GetMockedAppSchedulerClient());
    auto abilityRecord = record->GetAbilityRunningRecord(GetTestAbilityName());
    EXPECT_TRUE(abilityRecord != nullptr);

    EXPECT_CALL(*mockAppSchedulerClient_, ScheduleLaunchApplication(_)).Times(0);
    EXPECT_CALL(*mockAppSchedulerClient_, ScheduleLaunchAbility(_, _)).Times(0);
    service_->LaunchApplication(record);
    EXPECT_EQ(record->GetState(), ApplicationState::APP_STATE_READY);
    APP_LOGI("AmsAppRunningRecordTest LaunchAbilityForApp_003 end");
}

/*
 * Feature: AMS
 * Function: AppRunningRecord
 * SubFunction: NA
 * FunctionPoints: When server LaunchApplication and LaunchAbility.
 * EnvConditions: NA
 * CaseDescription: Test normal case of LoadAbility after LaunchAbility and LaunchApplication.
 */
HWTEST_F(AmsAppRunningRecordTest, LaunchAbilityForApp_004, TestSize.Level1)
{
    APP_LOGI("AmsAppRunningRecordTest LaunchAbilityForApp_004 start");
    auto abilityInfo = std::make_shared<AbilityInfo>();
    abilityInfo->name = GetTestAbilityName();
    abilityInfo->applicationName = GetTestAppName();
    abilityInfo->process = GetTestAppName();

    auto abilityInfo2 = std::make_shared<AbilityInfo>();
    abilityInfo2->name = GetTestAbilityName() + "_1";
    abilityInfo2->applicationName = GetTestAppName();
    abilityInfo2->process = GetTestAppName();

    auto appInfo = std::make_shared<ApplicationInfo>();
    appInfo->name = GetTestAppName();
    appInfo->bundleName = GetTestAppName();

    BundleInfo bundleInfo;
    bundleInfo.appId = "com.ohos.test.helloworld_code123";
    bundleInfo.jointUserId = "joint456";
    HapModuleInfo hapModuleInfo;
    hapModuleInfo.moduleName = "module789";
    EXPECT_TRUE(service_ != nullptr);

    std::shared_ptr<AppRunningRecord> record = service_->CreateAppRunningRecord(
        GetMockToken(), nullptr, appInfo, abilityInfo, GetTestProcessName(), bundleInfo, hapModuleInfo);
    auto abilityRecord = record->GetAbilityRunningRecord(GetTestAbilityName());
    EXPECT_TRUE(abilityRecord != nullptr);

    EXPECT_CALL(*mockAppSchedulerClient_, ScheduleLaunchApplication(_)).Times(1);
    EXPECT_CALL(*mockAppSchedulerClient_, ScheduleLaunchAbility(_, _)).Times(1);
    record->SetApplicationClient(GetMockedAppSchedulerClient());
    service_->LaunchApplication(record);
    EXPECT_EQ(record->GetState(), ApplicationState::APP_STATE_READY);

    EXPECT_CALL(*mockAppSchedulerClient_, ScheduleLaunchApplication(_)).Times(0);
    EXPECT_CALL(*mockAppSchedulerClient_, ScheduleLaunchAbility(_, _)).Times(1);
    sptr<IRemoteObject> token2 = new (std::nothrow) MockAbilityToken();
    service_->LoadAbility(token2, nullptr, abilityInfo2, appInfo);
    APP_LOGI("AmsAppRunningRecordTest LaunchAbilityForApp_004 end");
}

/*
 * Feature: AMS
 * Function: AppRunningRecord
 * SubFunction: NA
 * FunctionPoints: When server LaunchApplication and LaunchAbility.
 * EnvConditions: NA
 * CaseDescription: Test normal case of multiple LaunchAbility with wrong state after LaunchApplication.
 */
HWTEST_F(AmsAppRunningRecordTest, LaunchAbilityForApp_005, TestSize.Level1)
{
    APP_LOGI("AmsAppRunningRecordTest LaunchAbilityForApp_005 start");
    auto abilityInfo = std::make_shared<AbilityInfo>();
    abilityInfo->name = GetTestAbilityName();
    abilityInfo->applicationName = GetTestAppName();
    auto abilityInfo2 = std::make_shared<AbilityInfo>();
    abilityInfo2->name = GetTestAbilityName() + "_1";
    abilityInfo2->applicationName = GetTestAppName();
    auto abilityInfo3 = std::make_shared<AbilityInfo>();
    abilityInfo3->name = GetTestAbilityName() + "_2";
    abilityInfo3->applicationName = GetTestAppName();
    auto appInfo = std::make_shared<ApplicationInfo>();
    appInfo->name = GetTestAppName();
    BundleInfo bundleInfo;
    bundleInfo.appId = "com.ohos.test.helloworld_code123";
    bundleInfo.jointUserId = "joint456";
    HapModuleInfo hapModuleInfo;
    hapModuleInfo.moduleName = "module789";
    const int EXPECT_ABILITY_LAUNCH_TIME = 2;
    EXPECT_TRUE(service_ != nullptr);

    std::shared_ptr<AppRunningRecord> record = service_->CreateAppRunningRecord(
        GetMockToken(), nullptr, appInfo, abilityInfo, GetTestProcessName(), bundleInfo, hapModuleInfo);
    auto abilityRecord = record->GetAbilityRunningRecord(GetTestAbilityName());
    EXPECT_TRUE(abilityRecord != nullptr);

    sptr<IRemoteObject> token2 = new (std::nothrow) MockAbilityToken();
    record->AddModule(appInfo, abilityInfo2, token2, hapModuleInfo);
    auto moduleRecord2 = record->GetModuleRecordByModuleName(appInfo->bundleName, hapModuleInfo.moduleName);
    EXPECT_TRUE(moduleRecord2);
    auto abilityRecord2 = moduleRecord2->GetAbilityRunningRecordByToken(token2);
    abilityRecord2->SetState(AbilityState::ABILITY_STATE_READY);

    sptr<IRemoteObject> token3 = new (std::nothrow) MockAbilityToken();
    record->AddModule(appInfo, abilityInfo3, token3, hapModuleInfo);
    auto moduleRecord3 = record->GetModuleRecordByModuleName(appInfo->bundleName, hapModuleInfo.moduleName);
    EXPECT_TRUE(moduleRecord3);
    auto abilityRecord3 = moduleRecord3->GetAbilityRunningRecordByToken(token3);

    EXPECT_CALL(*mockAppSchedulerClient_, ScheduleLaunchApplication(_)).Times(1);
    EXPECT_CALL(*mockAppSchedulerClient_, ScheduleLaunchAbility(_, _)).Times(EXPECT_ABILITY_LAUNCH_TIME);
    record->SetApplicationClient(GetMockedAppSchedulerClient());
    service_->LaunchApplication(record);
    EXPECT_EQ(record->GetState(), ApplicationState::APP_STATE_READY);
    APP_LOGI("AmsAppRunningRecordTest LaunchAbilityForApp_005 end");
}

/*
 * Feature: AMS
 * Function: AppRunningRecord
 * SubFunction: TerminateAbility
 * FunctionPoints: check params
 * EnvConditions: Mobile that can run ohos test framework
 * CaseDescription: Verify the function TerminateAbility can check the token which not added.
 */
HWTEST_F(AmsAppRunningRecordTest, TerminateAbility_001, TestSize.Level1)
{
    APP_LOGI("AmsAppRunningRecordTest TerminateAbility_001 start");

    auto record = GetTestAppRunningRecord();
    EXPECT_CALL(*mockAppSchedulerClient_, ScheduleCleanAbility(_)).Times(0);
    record->TerminateAbility(GetMockToken(), false);

    APP_LOGI("AmsAppRunningRecordTest TerminateAbility_001 end");
}

/*
 * Feature: AMS
 * Function: AppRunningRecord
 * SubFunction: TerminateAbility
 * FunctionPoints: check params
 * EnvConditions: Mobile that can run ohos test framework
 * CaseDescription: Verify the function TerminateAbility can check the state not in background.
 */
HWTEST_F(AmsAppRunningRecordTest, TerminateAbility_002, TestSize.Level1)
{
    APP_LOGI("AmsAppRunningRecordTest TerminateAbility_002 start");

    auto abilityInfo = std::make_shared<AbilityInfo>();
    abilityInfo->name = GetTestAbilityName();

    HapModuleInfo hapModuleInfo;
    hapModuleInfo.moduleName = "module789";
    BundleInfo bundleInfo;
    bundleInfo.appId = "com.ohos.test.helloworld_code123";

    auto appInfo = std::make_shared<ApplicationInfo>();
    appInfo->name = GetTestAppName();
    appInfo->bundleName = GetTestAppName();

    std::shared_ptr<AppRunningRecord> record = service_->CreateAppRunningRecord(
        GetMockToken(), nullptr, appInfo, abilityInfo, GetTestProcessName(), bundleInfo, hapModuleInfo);
    auto abilityRecord = record->GetAbilityRunningRecord(GetTestAbilityName());
    EXPECT_TRUE(abilityRecord != nullptr);

    EXPECT_CALL(*mockAppSchedulerClient_, ScheduleCleanAbility(_)).Times(0);
    record->TerminateAbility(GetMockToken(), false);

    APP_LOGI("AmsAppRunningRecordTest TerminateAbility_002 end");
}

/*
 * Feature: AMS
 * Function: AppRunningRecord
 * SubFunction: AbilityTerminated
 * FunctionPoints: check params
 * EnvConditions: Mobile that can run ohos test framework
 * CaseDescription: Verify the function AbilityTerminated can check the token is nullptr.
 */
HWTEST_F(AmsAppRunningRecordTest, AbilityTerminated_001, TestSize.Level1)
{
    APP_LOGI("AmsAppRunningRecordTest AbilityTerminated_001 start");

    auto record = GetTestAppRunningRecord();
    EXPECT_CALL(*mockAppSchedulerClient_, ScheduleTerminateApplication()).Times(0);
    record->AbilityTerminated(nullptr);

    APP_LOGI("AmsAppRunningRecordTest AbilityTerminated_001 end");
}

/*
 * Feature: AMS
 * Function: AppRunningRecord
 * SubFunction: GetAbilityRunningRecord
 * FunctionPoints: check params
 * EnvConditions: Mobile that can run ohos test framework
 * CaseDescription: Verify the function GetAbilityRunningRecord return nullptr when the ability doesn't added.
 */
HWTEST_F(AmsAppRunningRecordTest, GetAbilityRunningRecord_001, TestSize.Level1)
{
    APP_LOGI("AmsAppRunningRecordTest GetAbilityRunningRecord_001 start");

    auto abilityInfo = std::make_shared<AbilityInfo>();
    abilityInfo->name = GetTestAbilityName();

    HapModuleInfo hapModuleInfo;
    hapModuleInfo.moduleName = "module789";
    BundleInfo bundleInfo;
    bundleInfo.appId = "com.ohos.test.helloworld_code123";
    auto appInfo = std::make_shared<ApplicationInfo>();
    appInfo->name = GetTestAppName();
    appInfo->bundleName = GetTestAppName();

    std::shared_ptr<AppRunningRecord> record = service_->CreateAppRunningRecord(
        GetMockToken(), nullptr, appInfo, abilityInfo, GetTestProcessName(), bundleInfo, hapModuleInfo);
    auto abilityRecord = record->GetAbilityRunningRecord(GetTestAbilityName());
    EXPECT_TRUE(abilityRecord != nullptr);

    std::string abilityName = "not_exist_ability_name";
    EXPECT_EQ(nullptr, record->GetAbilityRunningRecord(abilityName));

    APP_LOGI("AmsAppRunningRecordTest GetAbilityRunningRecord_001 end");
}

/*
 * Feature: AMS
 * Function: AppRunningRecord
 * SubFunction: GetAbilityRunningRecordByToken
 * FunctionPoints: check params
 * EnvConditions: Mobile that can run ohos test framework
 * CaseDescription: Verify the function GetAbilityRunningRecordByToken can check token is nullptr.
 */
HWTEST_F(AmsAppRunningRecordTest, GetAbilityRunningRecordByToken_001, TestSize.Level1)
{
    APP_LOGI("AmsAppRunningRecordTest GetAbilityRunningRecordByToken_001 start");

    auto record = GetTestAppRunningRecord();
    EXPECT_EQ(nullptr, record->GetAbilityRunningRecordByToken(nullptr));

    APP_LOGI("AmsAppRunningRecordTest GetAbilityRunningRecordByToken_001 end");
}

/*
 * Feature: AMS
 * Function: AppRunningRecord::SetUid, AppRunningRecord::GetUid()
 * SubFunction: GetAbilityRunningRecordByToken
 * FunctionPoints: check params
 * EnvConditions: Mobile that can run ohos test framework
 * CaseDescription: Verify the function GetAbilityRunningRecordByToken can check token is nullptr.
 */

HWTEST_F(AmsAppRunningRecordTest, SetUid_GetUid_001, TestSize.Level1)
{
    auto abilityInfo = std::make_shared<AbilityInfo>();
    abilityInfo->name = GetTestAbilityName();
    auto appInfo = std::make_shared<ApplicationInfo>();
    appInfo->name = GetTestAppName();

    BundleInfo bundleInfo;
    bundleInfo.appId = "com.ohos.test.helloworld_code123";
    bundleInfo.jointUserId = "joint456";
    HapModuleInfo hapModuleInfo;
    hapModuleInfo.moduleName = "module789";
    EXPECT_TRUE(service_ != nullptr);
    auto record = service_->CreateAppRunningRecord(
        GetMockToken(), nullptr, appInfo, abilityInfo, GetTestProcessName(), bundleInfo, hapModuleInfo);

    EXPECT_TRUE(record != nullptr);
    record->SetUid(102);

    auto otherRecord = service_->GetAppRunningRecordByAppRecordId(record->GetRecordId());
    EXPECT_TRUE(record != nullptr);

    EXPECT_EQ(otherRecord->GetUid(), 102);
}

/*
 * Feature: AMS
 * Function: OnAbilityStateChanged
 * SubFunction: App state switch
 * FunctionPoints: check params
 * EnvConditions: Mobile that can run ohos test framework
 * CaseDescription: Notify ability when the status of the app changes
 */

HWTEST_F(AmsAppRunningRecordTest, OnAbilityStateChanged_001, TestSize.Level1)
{
    auto abilityInfo = std::make_shared<AbilityInfo>();
    abilityInfo->name = GetTestAbilityName();

    auto appInfo = std::make_shared<ApplicationInfo>();
    appInfo->name = GetTestAppName();
    appInfo->bundleName = GetTestAppName();

    BundleInfo bundleInfo;
    bundleInfo.appId = "com.ohos.test.helloworld_code123";
    HapModuleInfo hapModuleInfo;
    hapModuleInfo.moduleName = "module789";

    std::shared_ptr<AppRunningRecord> record = service_->CreateAppRunningRecord(
        GetMockToken(), nullptr, appInfo, abilityInfo, GetTestProcessName(), bundleInfo, hapModuleInfo);
    auto moduleRecord = record->GetModuleRecordByModuleName(appInfo->bundleName, hapModuleInfo.moduleName);
    EXPECT_TRUE(moduleRecord != nullptr);
    auto abilityRecord = record->GetAbilityRunningRecord(GetTestAbilityName());
    EXPECT_TRUE(abilityRecord != nullptr);

    sptr<MockAppStateCallback> callback = new (std::nothrow) MockAppStateCallback();
    EXPECT_CALL(*callback, OnAbilityRequestDone(_, _)).Times(0);

    moduleRecord->OnAbilityStateChanged(nullptr, AbilityState::ABILITY_STATE_FOREGROUND);

    EXPECT_NE(AbilityState::ABILITY_STATE_FOREGROUND, abilityRecord->GetState());

    std::shared_ptr<AppMgrServiceInner> serviceInner;
    serviceInner.reset(new (std::nothrow) AppMgrServiceInner());
    EXPECT_TRUE(serviceInner);

    EXPECT_CALL(*callback, OnAbilityRequestDone(_, _)).Times(2);
    serviceInner->RegisterAppStateCallback(callback);
    record->SetAppMgrServiceInner(serviceInner);

    moduleRecord->OnAbilityStateChanged(abilityRecord, AbilityState::ABILITY_STATE_FOREGROUND);
    EXPECT_EQ(AbilityState::ABILITY_STATE_FOREGROUND, abilityRecord->GetState());

    moduleRecord->OnAbilityStateChanged(abilityRecord, AbilityState::ABILITY_STATE_BACKGROUND);
    EXPECT_EQ(AbilityState::ABILITY_STATE_BACKGROUND, abilityRecord->GetState());
}

/*
 * Feature: AMS
 * Function: AddModule
 * SubFunction: AddModule
 * FunctionPoints: check params
 * EnvConditions: Mobile that can run ohos test framework
 * CaseDescription: add module
 */

HWTEST_F(AmsAppRunningRecordTest, AddModule_001, TestSize.Level1)
{
    auto abilityInfo = std::make_shared<AbilityInfo>();
    abilityInfo->name = GetTestAbilityName();
    auto appInfo = std::make_shared<ApplicationInfo>();
    appInfo->name = GetTestAppName();

    BundleInfo bundleInfo;
    bundleInfo.appId = "com.ohos.test.helloworld_code123";
    HapModuleInfo hapModuleInfo;
    hapModuleInfo.moduleName = "module789";
    EXPECT_TRUE(service_ != nullptr);
    auto record = service_->CreateAppRunningRecord(
        GetMockToken(), nullptr, appInfo, abilityInfo, GetTestProcessName(), bundleInfo, hapModuleInfo);

    EXPECT_TRUE(record != nullptr);

    auto moduleRecordList = record->GetAllModuleRecord();
    EXPECT_TRUE(moduleRecordList.size() == 1);

    auto abilityInfo2 = std::make_shared<AbilityInfo>();
    abilityInfo2->name = GetTestAbilityName() + "_1";
    abilityInfo2->applicationName = GetTestAppName();
    HapModuleInfo hapModuleInfo1;
    hapModuleInfo1.moduleName = "module123";
    sptr<IRemoteObject> token2 = new (std::nothrow) MockAbilityToken();
    record->AddModule(appInfo, abilityInfo2, token2, hapModuleInfo1);

    moduleRecordList = record->GetAllModuleRecord();
    EXPECT_TRUE(moduleRecordList.size() == 2);
}

/*
 * Feature: AMS
 * Function: AddModule
 * SubFunction: AddModule
 * FunctionPoints: check params
 * EnvConditions: Mobile that can run ohos test framework
 * CaseDescription: add module
 */

HWTEST_F(AmsAppRunningRecordTest, AddModule_002, TestSize.Level1)
{
    auto abilityInfo = std::make_shared<AbilityInfo>();
    abilityInfo->name = GetTestAbilityName();
    auto appInfo = std::make_shared<ApplicationInfo>();
    appInfo->name = GetTestAppName();

    BundleInfo bundleInfo;
    bundleInfo.appId = "com.ohos.test.helloworld_code123";
    HapModuleInfo hapModuleInfo;
    hapModuleInfo.moduleName = "module789";
    EXPECT_TRUE(service_ != nullptr);
    auto record = service_->CreateAppRunningRecord(
        GetMockToken(), nullptr, appInfo, abilityInfo, GetTestProcessName(), bundleInfo, hapModuleInfo);

    EXPECT_TRUE(record != nullptr);

    auto moduleRecordList = record->GetAllModuleRecord();
    EXPECT_TRUE(moduleRecordList.size() == 1);

    auto abilityInfo2 = std::make_shared<AbilityInfo>();
    abilityInfo2->name = GetTestAbilityName() + "_1";
    abilityInfo2->applicationName = GetTestAppName();
    sptr<IRemoteObject> token2 = new (std::nothrow) MockAbilityToken();
    record->AddModule(appInfo, abilityInfo2, token2, hapModuleInfo);

    moduleRecordList = record->GetAllModuleRecord();
    EXPECT_TRUE(moduleRecordList.size() == 1);
}

/*
 * Feature: AMS
 * Function: GetModuleRecordByModuleName
 * SubFunction: GetModuleRecordByModuleName
 * FunctionPoints: check params
 * EnvConditions: Mobile that can run ohos test framework
 * CaseDescription: Get ModuleRecord By ModuleName
 */

HWTEST_F(AmsAppRunningRecordTest, GetModuleRecordByModuleName_001, TestSize.Level1)
{
    auto abilityInfo = std::make_shared<AbilityInfo>();
    abilityInfo->name = GetTestAbilityName();
    auto appInfo = std::make_shared<ApplicationInfo>();
    appInfo->name = GetTestAppName();
    appInfo->bundleName = GetTestAppName();

    BundleInfo bundleInfo;
    bundleInfo.appId = "com.ohos.test.helloworld_code123";
    HapModuleInfo hapModuleInfo;
    hapModuleInfo.moduleName = "module789";
    EXPECT_TRUE(service_ != nullptr);
    auto record = service_->CreateAppRunningRecord(
        GetMockToken(), nullptr, appInfo, abilityInfo, GetTestProcessName(), bundleInfo, hapModuleInfo);

    EXPECT_TRUE(record != nullptr);

    auto abilityInfo2 = std::make_shared<AbilityInfo>();
    abilityInfo2->name = GetTestAbilityName() + "_1";
    abilityInfo2->applicationName = GetTestAppName();
    HapModuleInfo hapModuleInfo1;
    hapModuleInfo1.moduleName = "module123";
    sptr<IRemoteObject> token2 = new (std::nothrow) MockAbilityToken();
    record->AddModule(appInfo, abilityInfo2, token2, hapModuleInfo1);

    auto moduleRecord = record->GetModuleRecordByModuleName(appInfo->bundleName, hapModuleInfo.moduleName);
    EXPECT_TRUE(moduleRecord);
}

/*
 * Feature: AMS
 * Function: GetAbilities
 * SubFunction: GetAbilities
 * FunctionPoints: check params
 * EnvConditions: Mobile that can run ohos test framework
 * CaseDescription: Get All Abilities
 */

HWTEST_F(AmsAppRunningRecordTest, GetAbilities_001, TestSize.Level1)
{
    auto abilityInfo = std::make_shared<AbilityInfo>();
    abilityInfo->name = GetTestAbilityName();
    auto appInfo = std::make_shared<ApplicationInfo>();
    appInfo->name = GetTestAppName();

    BundleInfo bundleInfo;
    bundleInfo.appId = "com.ohos.test.helloworld_code123";
    HapModuleInfo hapModuleInfo;
    hapModuleInfo.moduleName = "module789";
    EXPECT_TRUE(service_ != nullptr);
    auto record = service_->CreateAppRunningRecord(
        GetMockToken(), nullptr, appInfo, abilityInfo, GetTestProcessName(), bundleInfo, hapModuleInfo);

    EXPECT_TRUE(record != nullptr);

    auto abilityInfo2 = std::make_shared<AbilityInfo>();
    abilityInfo2->name = GetTestAbilityName() + "_1";
    abilityInfo2->applicationName = GetTestAppName();
    sptr<IRemoteObject> token2 = new (std::nothrow) MockAbilityToken();
    record->AddModule(appInfo, abilityInfo2, token2, hapModuleInfo);

    auto abilities = record->GetAbilities();
    EXPECT_TRUE(abilities.size() == 2);
}

/*
 * Feature: AMS
 * Function: GetAbilities
 * SubFunction: GetAbilities
 * FunctionPoints: check params
 * EnvConditions: Mobile that can run ohos test framework
 * CaseDescription: Get All Abilities
 */

HWTEST_F(AmsAppRunningRecordTest, GetAbilities_002, TestSize.Level1)
{
    auto abilityInfo = std::make_shared<AbilityInfo>();
    abilityInfo->name = GetTestAbilityName();
    auto appInfo = std::make_shared<ApplicationInfo>();
    appInfo->name = GetTestAppName();

    BundleInfo bundleInfo;
    bundleInfo.appId = "com.ohos.test.helloworld_code123";
    HapModuleInfo hapModuleInfo;
    hapModuleInfo.moduleName = "module789";
    EXPECT_TRUE(service_ != nullptr);
    auto record = service_->CreateAppRunningRecord(
        GetMockToken(), nullptr, appInfo, abilityInfo, GetTestProcessName(), bundleInfo, hapModuleInfo);

    EXPECT_TRUE(record != nullptr);

    auto abilityInfo2 = std::make_shared<AbilityInfo>();
    abilityInfo2->name = GetTestAbilityName() + "_1";
    abilityInfo2->applicationName = GetTestAppName();
    HapModuleInfo hapModuleInfo1;
    hapModuleInfo1.moduleName = "module123";
    sptr<IRemoteObject> token2 = new (std::nothrow) MockAbilityToken();
    record->AddModule(appInfo, abilityInfo2, token2, hapModuleInfo1);

    auto abilities = record->GetAbilities();
    EXPECT_TRUE(abilities.size() == 2);
}

/*
 * Feature: AMS
 * Function: RemoveModuleRecord
 * SubFunction: RemoveModuleRecord
 * FunctionPoints: check params
 * EnvConditions: Mobile that can run ohos test framework
 * CaseDescription: Remove ModuleRecord
 */

HWTEST_F(AmsAppRunningRecordTest, RemoveModuleRecord_001, TestSize.Level1)
{
    auto abilityInfo = std::make_shared<AbilityInfo>();
    abilityInfo->name = GetTestAbilityName();
    auto appInfo = std::make_shared<ApplicationInfo>();
    appInfo->name = GetTestAppName();
    appInfo->bundleName = GetTestAppName();

    BundleInfo bundleInfo;
    bundleInfo.appId = "com.ohos.test.helloworld_code123";
    HapModuleInfo hapModuleInfo;
    hapModuleInfo.moduleName = "module789";
    EXPECT_TRUE(service_ != nullptr);
    auto record = service_->CreateAppRunningRecord(
        GetMockToken(), nullptr, appInfo, abilityInfo, GetTestProcessName(), bundleInfo, hapModuleInfo);

    EXPECT_TRUE(record != nullptr);

    auto abilityInfo2 = std::make_shared<AbilityInfo>();
    abilityInfo2->name = GetTestAbilityName() + "_1";
    abilityInfo2->applicationName = GetTestAppName();
    HapModuleInfo hapModuleInfo1;
    hapModuleInfo1.moduleName = "module123";
    sptr<IRemoteObject> token2 = new (std::nothrow) MockAbilityToken();
    record->AddModule(appInfo, abilityInfo2, token2, hapModuleInfo1);

    auto moduleRecord = record->GetModuleRecordByModuleName(appInfo->bundleName, hapModuleInfo.moduleName);
    EXPECT_TRUE(moduleRecord);

    record->RemoveModuleRecord(moduleRecord);
    auto moduleRecordList = record->GetAllModuleRecord();
    EXPECT_TRUE(moduleRecordList.size() == 1);
}
}  // namespace AppExecFwk
}  // namespace OHOS
