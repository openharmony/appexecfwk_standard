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

#include <thread>
#include <chrono>
#include <cstdio>
#include <gtest/gtest.h>
#include "hilog_wrapper.h"
#include "ability_manager_service.h"
#include "ability_manager_errors.h"
#include "app_mgr_service.h"
#include "module_test_dump_util.h"
#include "system_test_ability_util.h"
#include "module_test_dump_util.h"
#include "sa_mgr_client.h"
#include "system_ability_definition.h"
#include "common_event.h"
#include "common_event_manager.h"

using namespace testing::ext;
using namespace OHOS;
using namespace OHOS::AAFwk;
using namespace OHOS::AppExecFwk;
using namespace OHOS::MTUtil;
using namespace OHOS::STABUtil;
using namespace OHOS::EventFwk;

namespace {
using MAP_STR_STR = std::map<std::string, std::string>;
std::vector<std::string> bundleNameSuffix = {"A", "B", "N"};
std::string bundleNameBase = "com.ohos.amsst.app";
std::string hapNameBase = "amsSystemTest";
std::string abilityNameBase = "AmsStAbility";
std::string launcherBundle = "com.ix.launcher";
std::string launcherAbility = "LauncherAbility";
std::string systemUiBundle = "com.ohos.systemui";
std::string terminatePageAbility = "requ_page_ability_terminate";

static const std::string DUMP_STACK_LIST = "--stack-list";
static const std::string DUMP_STACK = "--stack";
static const std::string DUMP_MISSION = "--mission";
static const std::string DUMP_TOP = "--top";
static const std::string DUMP_ALL = "-a";
constexpr int WAIT_TIME = 3 * 1000;
constexpr int WAIT_LAUNCHER_OK = 5 * 1000;
static const std::string abilityStateInit = ":Init";
static const std::string abilityStateOnStart = ":OnStart";
static const std::string abilityStateOnStop = ":OnStop";
static const std::string abilityStateOnActive = ":OnActive";
static const std::string abilityStateOnInactive = ":OnInactive";
static const std::string abilityStateOnBackground = ":OnBackground";
static const std::string abilityStateOnForeground = ":OnForeground";
static const std::string abilityStateOnNewWant = ":OnNewWant";
static const int abilityStateCountOne = 1;
static const int abilityStateCountTwo = 2;
enum AbilityState_Test {
    INITIAL = 0,
    INACTIVE,
    ACTIVE,
    BACKGROUND,
    SUSPENDED,
    INACTIVATING,
    ACTIVATING,
    MOVING_BACKGROUND,
    TERMINATING,
    ALLSUM,
};
static const std::vector<std::string> abilityStateVec = {
    "INITIAL",
    "INACTIVE",
    "ACTIVE",
    "BACKGROUND",
    "SUSPENDED",
    "INACTIVATING",
    "ACTIVATING",
    "MOVING_BACKGROUND",
    "TERMINATING",
};
}  // namespace
class AppEventSubscriber;

class AmsPowerTest : public testing::Test {
public:
    static void SetUpTestCase(void);
    static void TearDownTestCase(void);
    void SetUp();
    void TearDown();

    static std::vector<std::string> GetBundleNames(
        const std::string &strBase, const std::vector<std::string> &strSuffixs);
    static bool SubscribeEvent();
    void ShowDump();
    static void CheckAbilityStateByName(const std::string &abilityName, const std::vector<std::string> &info,
        const std::string &state, const std::string &midState);
    void ExpectAbilityCurrentState(const std::string &abilityName, const AbilityState_Test &currentState,
        const AbilityState_Test &midState = AbilityState_Test::ALLSUM, const std::string &args = (DUMP_STACK + " 1"));
    void ExpectAbilityNumInStack(const std::string &abilityName, int abilityNum);
    class AppEventSubscriber : public CommonEventSubscriber {
    public:
        explicit AppEventSubscriber(const CommonEventSubscribeInfo &sp) : CommonEventSubscriber(sp)
        {}
        virtual ~AppEventSubscriber()
        {}
        virtual void OnReceiveEvent(const CommonEventData &data) override;
    };
    static sptr<IAppMgr> appMs_;
    static sptr<IAbilityManager> abilityMs_;
    static STtools::Event event_;
    static std::shared_ptr<AppEventSubscriber> subscriber_;
    static int *memTestValue_;
};

sptr<IAppMgr> AmsPowerTest::appMs_ = nullptr;
sptr<IAbilityManager> AmsPowerTest::abilityMs_ = nullptr;
STtools::Event AmsPowerTest::event_ = STtools::Event();
int *AmsPowerTest::memTestValue_ = nullptr;
std::shared_ptr<AmsPowerTest::AppEventSubscriber> AmsPowerTest::subscriber_ = nullptr;

void AmsPowerTest::SetUpTestCase(void)
{
    GTEST_LOG_(INFO) << "void AmsPowerTest::SetUpTestCase(void)";
    std::vector<std::string> hapNames = GetBundleNames(hapNameBase, bundleNameSuffix);
    STAbilityUtil::InstallHaps(hapNames);
    SubscribeEvent();
    appMs_ = STAbilityUtil::GetAppMgrService();
    abilityMs_ = STAbilityUtil::GetAbilityManagerService();
    if (appMs_) {
        appMs_->SetAppFreezingTime(60);
        int time = 0;
        appMs_->GetAppFreezingTime(time);
        std::cout << "appMs_->GetAppFreezingTime();" << time << std::endl;
    }
}

void AmsPowerTest::TearDownTestCase(void)
{
    GTEST_LOG_(INFO) << "void AmsPowerTest::TearDownTestCase(void)";
    CommonEventManager::UnSubscribeCommonEvent(subscriber_);
    std::vector<std::string> bundleNames = GetBundleNames(bundleNameBase, bundleNameSuffix);
    STAbilityUtil::UninstallBundle(bundleNames);
}

void AmsPowerTest::SetUp(void)
{
    GTEST_LOG_(INFO) << "void AmsPowerTest::SetUp(void)";
}

void AmsPowerTest::TearDown(void)
{
    GTEST_LOG_(INFO) << "void AmsPowerTest::TearDown(void)";
    if (memTestValue_) {
        delete[] memTestValue_;
    }
    STAbilityUtil::RemoveStack(1, abilityMs_, WAIT_TIME, WAIT_LAUNCHER_OK);
    std::vector<std::string> vecBundleName;
    for (const auto &suffix : bundleNameSuffix) {
        vecBundleName.push_back(bundleNameBase + suffix);
    }
    STAbilityUtil::KillBundleProcess(vecBundleName);

    STAbilityUtil::CleanMsg(event_);
}

std::vector<std::string> AmsPowerTest::GetBundleNames(
    const std::string &strBase, const std::vector<std::string> &strSuffixs)
{
    std::vector<std::string> bundleNames;
    for (auto strSuffix : strSuffixs) {
        bundleNames.push_back(strBase + strSuffix);
    }
    return bundleNames;
}

bool AmsPowerTest::SubscribeEvent()
{
    std::vector<std::string> eventList = {"resp_st_page_ability_callback"};
    MatchingSkills matchingSkills;
    for (const auto &e : eventList) {
        matchingSkills.AddEvent(e);
    }
    CommonEventSubscribeInfo subscribeInfo(matchingSkills);
    subscribeInfo.SetPriority(1);
    subscriber_ = std::make_shared<AppEventSubscriber>(subscribeInfo);
    return CommonEventManager::SubscribeCommonEvent(subscriber_);
}

void AmsPowerTest::ShowDump()
{
    if (abilityMs_) {
        std::vector<std::string> dumpInfo;
        abilityMs_->DumpState("-a", dumpInfo);
        for (const auto &info : dumpInfo) {
            std::cout << info << std::endl;
        }
    }
}

void AmsPowerTest::CheckAbilityStateByName(const std::string &abilityName, const std::vector<std::string> &info,
    const std::string &state, const std::string &midState)
{
    std::vector<std::string> result;
    MTDumpUtil::GetInstance()->GetAll("AbilityName", info, result);
    auto pos = MTDumpUtil::GetInstance()->GetSpecific(abilityName, result, result.begin());
    // ability exist
    EXPECT_NE(pos, result.end());
    MTDumpUtil::GetInstance()->GetAll("State", info, result);
    EXPECT_TRUE(pos < result.end());
    // ability state
    if (midState != "") {
        bool compareResult = ((*pos == state) || (*pos == midState));
        EXPECT_EQ(1, compareResult);
    } else {
        EXPECT_EQ(*pos, state);
    }
}

void AmsPowerTest::ExpectAbilityCurrentState(const std::string &abilityName, const AbilityState_Test &currentState,
    const AbilityState_Test &midState, const std::string &args)
{
    std::string strCurrentState = abilityStateVec.at(currentState);
    std::string strMidState = "";
    if (midState != AbilityState_Test::ALLSUM) {
        strMidState = abilityStateVec.at(midState);
    }
    std::vector<std::string> dumpInfo;
    if (abilityMs_ != nullptr) {
        abilityMs_->DumpState(args, dumpInfo);
        CheckAbilityStateByName(abilityName, dumpInfo, strCurrentState, strMidState);
    } else {
        HILOG_ERROR("ability manager service(abilityMs_) is nullptr");
    }
}

void AmsPowerTest::ExpectAbilityNumInStack(const std::string &abilityName, int abilityNum)
{
    std::vector<std::string> dumpInfo;
    if (abilityMs_ != nullptr) {
        abilityMs_->DumpState("-a", dumpInfo);
        std::vector<std::string> result;
        MTDumpUtil::GetInstance()->GetAll("AbilityName", dumpInfo, result);
        // only one record in stack
        EXPECT_EQ(abilityNum, std::count(result.begin(), result.end(), abilityName));
    } else {
        HILOG_ERROR("ability manager service(abilityMs_) is nullptr");
    }
}

void AmsPowerTest::AppEventSubscriber::OnReceiveEvent(const CommonEventData &data)
{
    GTEST_LOG_(INFO) << "OnReceiveEvent: event=" << data.GetWant().GetAction();
    GTEST_LOG_(INFO) << "OnReceiveEvent: data=" << data.GetData();
    GTEST_LOG_(INFO) << "OnReceiveEvent: code=" << data.GetCode();

    std::string eventName = data.GetWant().GetAction();
    if (eventName == "resp_st_page_ability_callback") {
        std::string target = data.GetData();
        STAbilityUtil::Completed(event_, target, data.GetCode());
    }
}

/*
 * @tc.number    : AMS_Power_0100
 * @tc.name      : Verify poweroff and poweron functions
 * @tc.desc      : 1.start ability
 *                 2.called PowerOff
 *                 3.Check the stack status and process status,
 * and verify the application callback function at the same time.
 *                 4.called PowerOn
 *                 5.Check the stack status and process status,
 * and verify the application callback function at the same time.
 */
HWTEST_F(AmsPowerTest, AMS_Power_0100, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "AmsPowerTest AMS_Power_0100 start";
    std::string bundleName = bundleNameBase + "A";
    std::string abilityName = abilityNameBase + "A1";

    MAP_STR_STR params;
    Want want = STAbilityUtil::MakeWant("device", abilityName, bundleName, params);
    STAbilityUtil::StartAbility(want, abilityMs_);

    EXPECT_EQ(STAbilityUtil::WaitCompleted(event_, abilityName + abilityStateOnActive, abilityStateCountOne), 0);

    if (abilityMs_) {
        abilityMs_->PowerOff();
    }
    EXPECT_EQ(STAbilityUtil::WaitCompleted(event_, abilityName + abilityStateOnInactive, abilityStateCountOne), 0);
    EXPECT_EQ(STAbilityUtil::WaitCompleted(event_, abilityName + abilityStateOnBackground, abilityStateCountOne), 0);
    ExpectAbilityCurrentState(abilityName, AbilityState_Test::BACKGROUND, AbilityState_Test::MOVING_BACKGROUND);
    RunningProcessInfo pInfo = STAbilityUtil::GetAppProcessInfoByName(bundleName, appMs_, WAIT_TIME);
    EXPECT_EQ(AppProcessState::APP_STATE_BACKGROUND, pInfo.state_);
    if (abilityMs_) {
        abilityMs_->PowerOn();
    }
    EXPECT_EQ(STAbilityUtil::WaitCompleted(event_, abilityName + abilityStateOnForeground, abilityStateCountOne), 0);
    EXPECT_EQ(STAbilityUtil::WaitCompleted(event_, abilityName + abilityStateOnActive, abilityStateCountTwo), 0);
    ExpectAbilityCurrentState(abilityName, AbilityState_Test::ACTIVATING, AbilityState_Test::ACTIVE);
    pInfo = STAbilityUtil::GetAppProcessInfoByName(bundleName, appMs_, WAIT_TIME);
    EXPECT_EQ(AppProcessState::APP_STATE_FOREGROUND, pInfo.state_);

    GTEST_LOG_(INFO) << "AmsPowerTest AMS_Power_0100 end";
}

/*
 * @tc.number    : AMS_Power_0200
 * @tc.name      : Verify poweroff and poweron functions(different bundle)[A1,B1:singleton]
 * @tc.desc      : 1.start ability
 *                 2.called PowerOff
 *                 3.Check the stack status and process status,
 * and verify the application callback function at the same time.
 *                 4.called PowerOn
 *                 5.Check the stack status and process status,
 * and verify the application callback function at the same time.
 */
HWTEST_F(AmsPowerTest, AMS_Power_0200, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "AmsPowerTest AMS_Power_0200 start";
    std::string bundleName = bundleNameBase + "A";
    std::string abilityName = abilityNameBase + "A1";
    std::string bundleName2 = bundleNameBase + "B";
    std::string abilityName2 = abilityNameBase + "B1";

    MAP_STR_STR params;
    params["targetBundle"] = bundleName2;
    params["targetAbility"] = abilityName2;
    Want want = STAbilityUtil::MakeWant("device", abilityName, bundleName, params);
    STAbilityUtil::StartAbility(want, abilityMs_);

    EXPECT_EQ(STAbilityUtil::WaitCompleted(event_, abilityName + abilityStateOnActive, abilityStateCountOne), 0);
    EXPECT_EQ(STAbilityUtil::WaitCompleted(event_, abilityName2 + abilityStateOnActive, abilityStateCountOne), 0);
    EXPECT_EQ(STAbilityUtil::WaitCompleted(event_, abilityName + abilityStateOnBackground, abilityStateCountOne), 0);
    std::this_thread::sleep_for(std::chrono::milliseconds(WAIT_TIME));
    ShowDump();
    if (abilityMs_) {
        abilityMs_->PowerOff();
    }
    std::this_thread::sleep_for(std::chrono::milliseconds(WAIT_TIME));
    ShowDump();
    EXPECT_EQ(STAbilityUtil::WaitCompleted(event_, abilityName2 + abilityStateOnInactive, abilityStateCountOne), 0);
    EXPECT_EQ(STAbilityUtil::WaitCompleted(event_, abilityName2 + abilityStateOnBackground, abilityStateCountOne), 0);
    ExpectAbilityCurrentState(abilityName2, AbilityState_Test::BACKGROUND, AbilityState_Test::MOVING_BACKGROUND);
    RunningProcessInfo pInfo = STAbilityUtil::GetAppProcessInfoByName(bundleName2, appMs_, WAIT_TIME);
    EXPECT_EQ(AppProcessState::APP_STATE_BACKGROUND, pInfo.state_);
    if (abilityMs_) {
        abilityMs_->PowerOn();
    }
    std::this_thread::sleep_for(std::chrono::milliseconds(WAIT_TIME));
    ShowDump();
    EXPECT_EQ(STAbilityUtil::WaitCompleted(event_, abilityName2 + abilityStateOnForeground, abilityStateCountOne), 0);
    EXPECT_EQ(STAbilityUtil::WaitCompleted(event_, abilityName2 + abilityStateOnActive, abilityStateCountTwo), 0);
    ExpectAbilityCurrentState(abilityName2, AbilityState_Test::ACTIVATING, AbilityState_Test::ACTIVE);
    pInfo = STAbilityUtil::GetAppProcessInfoByName(bundleName2, appMs_, WAIT_TIME);
    EXPECT_EQ(AppProcessState::APP_STATE_FOREGROUND, pInfo.state_);

    GTEST_LOG_(INFO) << "AmsPowerTest AMS_Power_0200 end";
}

/*
 * @tc.number    : AMS_Power_0300
 * @tc.name      : Verify poweroff and poweron functions(different bundle)[A1,B1:singleton]
 * @tc.desc      : 1.start ability(PowerOff now)
 *                 2.called PowerOff
 *                 3.Check the stack status and process status,
 * and verify the application callback function at the same time.
 *                 4.called PowerOn
 *                 5.Check the stack status and process status,
 * and verify the application callback function at the same time.
 */
HWTEST_F(AmsPowerTest, AMS_Power_0300, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "AmsPowerTest AMS_Power_0300 start";
    std::string bundleName = bundleNameBase + "A";
    std::string abilityName = abilityNameBase + "A1";
    std::string bundleName2 = bundleNameBase + "B";
    std::string abilityName2 = abilityNameBase + "B1";

    MAP_STR_STR params;
    params["targetBundle"] = bundleName2;
    params["targetAbility"] = abilityName2;
    Want want = STAbilityUtil::MakeWant("device", abilityName, bundleName, params);
    STAbilityUtil::StartAbility(want, abilityMs_);

    if (abilityMs_) {
        int errCode = abilityMs_->PowerOff();
        EXPECT_EQ(POWER_OFF_WAITING, errCode);
    }
    EXPECT_EQ(STAbilityUtil::WaitCompleted(event_, abilityName2 + abilityStateOnBackground, abilityStateCountOne), 0);
    ExpectAbilityCurrentState(abilityName2, AbilityState_Test::BACKGROUND, AbilityState_Test::MOVING_BACKGROUND);
    RunningProcessInfo pInfo = STAbilityUtil::GetAppProcessInfoByName(bundleName2, appMs_, WAIT_TIME);
    EXPECT_EQ(AppProcessState::APP_STATE_BACKGROUND, pInfo.state_);
    if (abilityMs_) {
        abilityMs_->PowerOn();
    }
    EXPECT_EQ(STAbilityUtil::WaitCompleted(event_, abilityName2 + abilityStateOnActive, abilityStateCountTwo), 0);
    ExpectAbilityCurrentState(abilityName2, AbilityState_Test::ACTIVATING, AbilityState_Test::ACTIVE);
    pInfo = STAbilityUtil::GetAppProcessInfoByName(bundleName2, appMs_, WAIT_TIME);
    EXPECT_EQ(AppProcessState::APP_STATE_FOREGROUND, pInfo.state_);

    GTEST_LOG_(INFO) << "AmsPowerTest AMS_Power_0300 end";
}

/*
 * @tc.number    : AMS_Power_0400
 * @tc.name      : Verify poweroff and poweron functions(different bundle)[A1:singleton,N3:singleton]
 * @tc.desc      : 1.start ability(PowerOff now)
 *                 2.called PowerOff
 *                 3.Check the stack status and process status,
 * and verify the application callback function at the same time.
 *                 4.called PowerOn
 *                 5.Check the stack status and process status,
 * and verify the application callback function at the same time.
 */
HWTEST_F(AmsPowerTest, AMS_Power_0400, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "AmsPowerTest AMS_Power_0400 start";
    std::string bundleName = bundleNameBase + "A";
    std::string abilityName = abilityNameBase + "A1";
    std::string bundleName2 = bundleNameBase + "N";
    std::string abilityName2 = abilityNameBase + "N3";

    MAP_STR_STR params;
    params["targetBundle"] = bundleName2;
    params["targetAbility"] = abilityName2;
    Want want = STAbilityUtil::MakeWant("device", abilityName, bundleName, params);
    STAbilityUtil::StartAbility(want, abilityMs_);
    EXPECT_EQ(STAbilityUtil::WaitCompleted(event_, abilityName + abilityStateOnActive, abilityStateCountOne), 0);
    EXPECT_EQ(STAbilityUtil::WaitCompleted(event_, abilityName2 + abilityStateOnActive, abilityStateCountOne), 0);
    EXPECT_EQ(STAbilityUtil::WaitCompleted(event_, abilityName + abilityStateOnBackground, abilityStateCountOne), 0);

    if (abilityMs_) {
        abilityMs_->PowerOff();
    }
    EXPECT_EQ(STAbilityUtil::WaitCompleted(event_, abilityName2 + abilityStateOnBackground, abilityStateCountOne), 0);
    ExpectAbilityCurrentState(abilityName2, AbilityState_Test::BACKGROUND, AbilityState_Test::MOVING_BACKGROUND);
    RunningProcessInfo pInfo = STAbilityUtil::GetAppProcessInfoByName(bundleName2, appMs_, WAIT_TIME);
    EXPECT_EQ(AppProcessState::APP_STATE_BACKGROUND, pInfo.state_);
    if (abilityMs_) {
        abilityMs_->PowerOn();
    }
    EXPECT_EQ(STAbilityUtil::WaitCompleted(event_, abilityName2 + abilityStateOnActive, abilityStateCountTwo), 0);
    ExpectAbilityCurrentState(abilityName2, AbilityState_Test::ACTIVATING, AbilityState_Test::ACTIVE);
    pInfo = STAbilityUtil::GetAppProcessInfoByName(bundleName2, appMs_, WAIT_TIME);
    EXPECT_EQ(AppProcessState::APP_STATE_FOREGROUND, pInfo.state_);

    GTEST_LOG_(INFO) << "AmsPowerTest AMS_Power_0400 end";
}

/*
 * @tc.number    : AMS_Power_0500
 * @tc.name      : Verify poweroff and poweron functions(different bundle)[A1:singleton,N3:singleton]
 * @tc.desc      : 1.start ability
 *                 2.called PowerOff(PowerOn now)
 *                 3.called PowerOn
 *                 4.Check the stack status and process status,
 * and verify the application callback function at the same time.
 */
HWTEST_F(AmsPowerTest, AMS_Power_0500, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "AmsPowerTest AMS_Power_0500 start";
    std::string bundleName = bundleNameBase + "A";
    std::string abilityName = abilityNameBase + "A1";
    std::string bundleName2 = bundleNameBase + "N";
    std::string abilityName2 = abilityNameBase + "N3";

    MAP_STR_STR params;
    params["targetBundle"] = bundleName2;
    params["targetAbility"] = abilityName2;
    Want want = STAbilityUtil::MakeWant("device", abilityName, bundleName, params);
    STAbilityUtil::StartAbility(want, abilityMs_);
    EXPECT_EQ(STAbilityUtil::WaitCompleted(event_, abilityName + abilityStateOnActive, abilityStateCountOne), 0);
    EXPECT_EQ(STAbilityUtil::WaitCompleted(event_, abilityName2 + abilityStateOnActive, abilityStateCountOne), 0);
    EXPECT_EQ(STAbilityUtil::WaitCompleted(event_, abilityName + abilityStateOnBackground, abilityStateCountOne), 0);

    if (abilityMs_) {
        abilityMs_->PowerOff();
    }
    if (abilityMs_) {
        abilityMs_->PowerOn();
    }
    EXPECT_EQ(STAbilityUtil::WaitCompleted(event_, abilityName2 + abilityStateOnActive, abilityStateCountTwo), 0);
    ExpectAbilityCurrentState(abilityName2, AbilityState_Test::ACTIVATING, AbilityState_Test::ACTIVE);
    RunningProcessInfo pInfo = STAbilityUtil::GetAppProcessInfoByName(bundleName2, appMs_, WAIT_TIME);
    EXPECT_EQ(AppProcessState::APP_STATE_FOREGROUND, pInfo.state_);

    GTEST_LOG_(INFO) << "AmsPowerTest AMS_Power_0500 end";
}

/*
 * @tc.number    : AMS_Power_0600
 * @tc.name      : Verify poweroff and poweron functions(launcher)[singleton]
 * @tc.desc      : 1.called PowerOff
 *                 2.Check the launcher process status
 *                 3.called PowerOn
 *                 4.Check the launcher process status
 */
HWTEST_F(AmsPowerTest, AMS_Power_0600, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "AmsPowerTest AMS_Power_0600 start";
    ExpectAbilityCurrentState(launcherAbility, AbilityState_Test::ACTIVE, AbilityState_Test::ACTIVATING, DUMP_ALL);
    ShowDump();
    if (abilityMs_) {
        abilityMs_->PowerOff();
    }
    std::this_thread::sleep_for(std::chrono::milliseconds(WAIT_TIME));
    ShowDump();
    ExpectAbilityCurrentState(
        launcherAbility, AbilityState_Test::BACKGROUND, AbilityState_Test::MOVING_BACKGROUND, DUMP_ALL);
    if (abilityMs_) {
        abilityMs_->PowerOn();
    }
    std::this_thread::sleep_for(std::chrono::milliseconds(WAIT_TIME));
    ExpectAbilityCurrentState(launcherAbility, AbilityState_Test::ACTIVE, AbilityState_Test::ACTIVATING, DUMP_ALL);
    ShowDump();

    GTEST_LOG_(INFO) << "AmsPowerTest AMS_Power_0600 end";
}