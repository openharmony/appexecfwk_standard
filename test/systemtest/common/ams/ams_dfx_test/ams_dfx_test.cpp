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
#include "ability_connect_callback_proxy.h"
#include "ability_connect_callback_stub.h"
#include "ability_manager_service.h"
#include "ability_manager_errors.h"
#include "app_mgr_service.h"
#include "module_test_dump_util.h"
#include "st_ability_util.h"
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
std::vector<std::string> bundleNameSuffix = {"Q", "R", "DFX"};
std::string bundleNameBase = "com.ohos.amsst.app";
std::string hapNameBase = "amsSystemTest";
std::string abilityNameBase = "AmsStAbility";
std::string launcherBundle = "com.ohos.launcher";
std::string launcherAbility = "com.ohos.launcher.MainAbility";
std::string systemUiBundle = "com.ohos.systemui";
std::string terminatePageAbility = "requ_page_ability_terminate";

static const std::string DUMP_STACK_LIST = "--stack-list";
const std::string DUMP_SERVICE = "--serv";
static const std::string DUMP_STACK = "--stack";
static const std::string DUMP_MISSION = "--mission";
static const std::string DUMP_TOP = "--top";
static const std::string DUMP_ALL = "-a";
constexpr int WAIT_TIME = 7 * 1000;
constexpr int BACKGROUND_WAIT_TIME = 15 * 1000;
static const std::string abilityStateInit = ":Init";
static const std::string abilityStateOnStart = ":OnStart";
static const std::string abilityStateOnStop = ":OnStop";
static const std::string abilityStateOnActive = ":OnActive";
static const std::string abilityStateOnInactive = ":OnInactive";
static const std::string abilityStateOnBackground = ":OnBackground";
static const std::string abilityStateOnForeground = ":OnForeground";
static const std::string abilityStateOnNewWant = ":OnNewWant";
static const std::string abilityStateOnCommand = ":OnCommand";
static const std::string abilityStateOnConnect = ":OnConnect";
static const std::string abilityStateOnDisconnect = ":OnDisconnect";
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

class AmsDFXTest : public testing::Test {
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
    void ExpectServiceAbilityNumInStack(const std::string &abilityName, int abilityNum);
    class AppEventSubscriber : public CommonEventSubscriber {
    public:
        explicit AppEventSubscriber(const CommonEventSubscribeInfo &sp) : CommonEventSubscriber(sp)
        {}
        virtual ~AppEventSubscriber()
        {}
        virtual void OnReceiveEvent(const CommonEventData &data) override;
    };

    class AbilityConnectCallback : public AbilityConnectionStub {
    public:
        /**
         * OnAbilityConnectDone, AbilityMs notify caller ability the result of connect.
         *
         * @param element,.service ability's ElementName.
         * @param remoteObject,.the session proxy of service ability.
         * @param resultCode, ERR_OK on success, others on failure.
         */
        void OnAbilityConnectDone(
            const AppExecFwk::ElementName &element, const sptr<IRemoteObject> &remoteObject, int resultCode) override
        {
            GTEST_LOG_(INFO) << "AbilityConnectCallback::OnAbilityConnectDone:resultCode = " << resultCode;
            if (resultCode == 0) {
                onAbilityConnectDoneCount++;
            }
            resultConnectCode = resultCode;
        }

        /**
         * OnAbilityDisconnectDone, AbilityMs notify caller ability the result of disconnect.
         *
         * @param element,.service ability's ElementName.
         * @param resultCode, ERR_OK on success, others on failure.
         */
        void OnAbilityDisconnectDone(const AppExecFwk::ElementName &element, int resultCode) override
        {
            GTEST_LOG_(INFO) << "AbilityConnectCallback::OnAbilityDisconnectDone:resultCode = " << resultCode;
            if (resultCode == 0) {
                onAbilityConnectDoneCount--;
            }
            resultConnectCode = resultCode;
        }

        static size_t onAbilityConnectDoneCount;
        static int resultConnectCode;
    };
    static sptr<IAppMgr> appMs_;
    static sptr<IAbilityManager> abilityMs_;
    static STtools::Event event_;
    static std::shared_ptr<AppEventSubscriber> subscriber_;
};

sptr<IAppMgr> AmsDFXTest::appMs_ = nullptr;
sptr<IAbilityManager> AmsDFXTest::abilityMs_ = nullptr;
STtools::Event AmsDFXTest::event_ = STtools::Event();
std::shared_ptr<AmsDFXTest::AppEventSubscriber> AmsDFXTest::subscriber_ = nullptr;
size_t AmsDFXTest::AbilityConnectCallback::onAbilityConnectDoneCount = 0;
int AmsDFXTest::AbilityConnectCallback::resultConnectCode = 0;

void AmsDFXTest::SetUpTestCase(void)
{
    GTEST_LOG_(INFO) << "void AmsDFXTest::SetUpTestCase(void)";

    SubscribeEvent();
    appMs_ = STAbilityUtil::GetAppMgrService();
    abilityMs_ = STAbilityUtil::GetAbilityManagerService();
    if (appMs_) {
        int freeTime = 60;
        appMs_->SetAppFreezingTime(freeTime);
        int time = 0;
        appMs_->GetAppFreezingTime(time);
        std::cout << "appMs_->GetAppFreezingTime();" << time << std::endl;
    }
}

void AmsDFXTest::TearDownTestCase(void)
{
    GTEST_LOG_(INFO) << "void AmsDFXTest::TearDownTestCase(void)";
    CommonEventManager::UnSubscribeCommonEvent(subscriber_);
}

void AmsDFXTest::SetUp(void)
{
    GTEST_LOG_(INFO) << "void AmsDFXTest::SetUp(void)";
    std::vector<std::string> hapNames = GetBundleNames(hapNameBase, bundleNameSuffix);
    STAbilityUtil::InstallHaps(hapNames);
}

void AmsDFXTest::TearDown(void)
{
    GTEST_LOG_(INFO) << "void AmsDFXTest::TearDown(void)";

    std::vector<std::string> bundleNames = GetBundleNames(bundleNameBase, bundleNameSuffix);
    STAbilityUtil::UninstallBundle(bundleNames);

    STAbilityUtil::CleanMsg(event_);
}

std::vector<std::string> AmsDFXTest::GetBundleNames(
    const std::string &strBase, const std::vector<std::string> &strSuffixs)
{
    std::vector<std::string> bundleNames;
    for (auto strSuffix : strSuffixs) {
        bundleNames.push_back(strBase + strSuffix);
    }
    return bundleNames;
}

bool AmsDFXTest::SubscribeEvent()
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

void AmsDFXTest::ShowDump()
{
    if (abilityMs_) {
        std::vector<std::string> dumpInfo;
        abilityMs_->DumpState("-a", dumpInfo);
        for (const auto &info : dumpInfo) {
            std::cout << info << std::endl;
        }
    }
}

void AmsDFXTest::CheckAbilityStateByName(const std::string &abilityName, const std::vector<std::string> &info,
    const std::string &state, const std::string &midState)
{
    std::vector<std::string> result;
    MTDumpUtil::GetInstance()->GetAll("AbilityName", info, result);
    auto pos = MTDumpUtil::GetInstance()->GetSpecific(abilityName, result, result.begin());
    // ability exist
    EXPECT_NE(pos, result.end());
    MTDumpUtil::GetInstance()->GetAll("State", info, result);
    EXPECT_TRUE(pos < result.end());
    if (pos == result.end()) {
        HILOG_ERROR("pos == result.end()");
        return;
    }
    // ability state
    if (midState != "") {
        bool compareResult = ((*pos == state) || (*pos == midState));
        EXPECT_EQ(1, compareResult);
    } else {
        EXPECT_EQ(*pos, state);
    }
}

void AmsDFXTest::ExpectAbilityCurrentState(const std::string &abilityName, const AbilityState_Test &currentState,
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

void AmsDFXTest::ExpectAbilityNumInStack(const std::string &abilityName, int abilityNum)
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

void AmsDFXTest::ExpectServiceAbilityNumInStack(const std::string &abilityName, int abilityNum)
{
    std::vector<std::string> dumpInfo;
    if (abilityMs_ != nullptr) {
        abilityMs_->DumpState(DUMP_SERVICE, dumpInfo);
        std::vector<std::string> result;
        MTDumpUtil::GetInstance()->GetAll("AbilityName", dumpInfo, result);
        // only one record in stack
        EXPECT_EQ(abilityNum, std::count(result.begin(), result.end(), abilityName));
    } else {
        HILOG_ERROR("ability manager service(abilityMs_) is nullptr");
    }
}

void AmsDFXTest::AppEventSubscriber::OnReceiveEvent(const CommonEventData &data)
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
 * @tc.number    : AMS_DFX_0100
 * @tc.name      : Lifecycle switch timeout.
 * @tc.desc      : 1.LoadAbility timeout pageAbility
 *                 2.Restart the previous ability(different bundle)
 */
HWTEST_F(AmsDFXTest, AMS_DFX_0100, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "AmsDFXTest AMS_DFX_0100 start";
    std::string bundleName = bundleNameBase + "Q";
    std::string abilityName = abilityNameBase + "Q1";
    std::string bundleName2 = bundleNameBase + "R";
    std::string abilityName2 = abilityNameBase + "R1";

    MAP_STR_STR params;
    params["targetBundle"] = bundleName2;
    params["targetAbility"] = abilityName2;
    Want want = STAbilityUtil::MakeWant("device", abilityName, bundleName, params);
    STAbilityUtil::StartAbility(want, abilityMs_);

    EXPECT_EQ(STAbilityUtil::WaitCompleted(event_, abilityName + abilityStateOnActive, abilityStateCountOne), 0);
    EXPECT_EQ(STAbilityUtil::WaitCompleted(event_, abilityName + abilityStateOnActive, abilityStateCountTwo), 0);
    ExpectAbilityNumInStack(abilityName2, 0);
    GTEST_LOG_(INFO) << "AmsDFXTest AMS_DFX_0100 end";
}

/*
 * @tc.number    : AMS_DFX_0200
 * @tc.name      : Lifecycle switch timeout.
 * @tc.desc      : 1.LoadAbility timeout pageAbility
 *                 2.Restart the previous ability(same bundle)
 */
HWTEST_F(AmsDFXTest, AMS_DFX_0200, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "AmsDFXTest AMS_DFX_0200 start";
    std::string bundleName = bundleNameBase + "Q";
    std::string abilityName = abilityNameBase + "Q1";
    std::string abilityName2 = abilityNameBase + "Q2";

    MAP_STR_STR params;
    params["targetBundle"] = bundleName;
    params["targetAbility"] = abilityName2;
    Want want = STAbilityUtil::MakeWant("device", abilityName, bundleName, params);
    STAbilityUtil::StartAbility(want, abilityMs_);

    EXPECT_EQ(STAbilityUtil::WaitCompleted(event_, abilityName + abilityStateOnActive, abilityStateCountOne), 0);
    EXPECT_EQ(STAbilityUtil::WaitCompleted(event_, abilityName + abilityStateOnActive, abilityStateCountTwo), 0);
    ExpectAbilityNumInStack(abilityName2, 0);
    GTEST_LOG_(INFO) << "AmsDFXTest AMS_DFX_0200 end";
}

/*
 * @tc.number    : AMS_DFX_0300
 * @tc.name      : Lifecycle switch timeout.
 * @tc.desc      : 1.LoadAbility timeout pageAbility
 *                 2.Restart the previous ability(back launch)
 */
HWTEST_F(AmsDFXTest, AMS_DFX_0300, TestSize.Level1)
{
    std::this_thread::sleep_for(std::chrono::milliseconds(5000));
    GTEST_LOG_(INFO) << "AmsDFXTest AMS_DFX_0300 start";
    std::string bundleName = bundleNameBase + "R";
    std::string abilityName = abilityNameBase + "R1";

    MAP_STR_STR params;
    Want want = STAbilityUtil::MakeWant("device", abilityName, bundleName, params);
    STAbilityUtil::StartAbility(want, abilityMs_);
    std::this_thread::sleep_for(std::chrono::milliseconds(WAIT_TIME));
    ExpectAbilityCurrentState(launcherAbility, AbilityState_Test::ACTIVE, AbilityState_Test::ACTIVATING, DUMP_ALL);
    ExpectAbilityNumInStack(abilityName, 0);
    GTEST_LOG_(INFO) << "AmsDFXTest AMS_DFX_0300 end";
}

/*
 * @tc.number    : AMS_DFX_0400
 * @tc.name      : Lifecycle switch timeout.
 * @tc.desc      : 1.LoadAbility timeout pageAbility
 *                 2.Restart the previous ability
 */
HWTEST_F(AmsDFXTest, AMS_DFX_0400, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "AmsDFXTest AMS_DFX_0400 start";
    std::string bundleName = bundleNameBase + "Q";
    std::string abilityName = abilityNameBase + "Q1";
    std::string bundleName2 = bundleNameBase + "R";
    std::string abilityName2 = abilityNameBase + "R1";
    std::string abilityName3 = abilityNameBase + "Q3";

    MAP_STR_STR params;
    params["targetBundle"] = bundleName + "," + bundleName2;
    params["targetAbility"] = abilityName3 + "," + abilityName2;
    Want want = STAbilityUtil::MakeWant("device", abilityName, bundleName, params);
    STAbilityUtil::StartAbility(want, abilityMs_);

    EXPECT_EQ(STAbilityUtil::WaitCompleted(event_, abilityName + abilityStateOnActive, abilityStateCountOne), 0);
    EXPECT_EQ(STAbilityUtil::WaitCompleted(event_, abilityName3 + abilityStateOnActive, abilityStateCountOne), 0);
    EXPECT_EQ(STAbilityUtil::WaitCompleted(event_, abilityName3 + abilityStateOnActive, abilityStateCountTwo), 0);
    ExpectAbilityNumInStack(abilityName2, 0);
    GTEST_LOG_(INFO) << "AmsDFXTest AMS_DFX_0400 end";
}

/*
 * @tc.number    : AMS_DFX_0500
 * @tc.name      : Lifecycle switch timeout.
 * @tc.desc      : 1.background timeout pageAbility
 *                 2.Force call CompleteBackground function(same bundle)
 */
HWTEST_F(AmsDFXTest, AMS_DFX_0500, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "AmsDFXTest AMS_DFX_0500 start";
    std::string bundleName = bundleNameBase + "Q";
    std::string abilityName = abilityNameBase + "Q1";
    std::string abilityName2 = abilityNameBase + "Q4";

    MAP_STR_STR params;
    params["targetBundle"] = bundleName;
    params["targetAbility"] = abilityName;
    Want want = STAbilityUtil::MakeWant("device", abilityName2, bundleName, params);
    STAbilityUtil::StartAbility(want, abilityMs_);

    EXPECT_EQ(STAbilityUtil::WaitCompleted(event_, abilityName2 + abilityStateOnActive, abilityStateCountOne), 0);
    EXPECT_EQ(STAbilityUtil::WaitCompleted(event_, abilityName + abilityStateOnActive, abilityStateCountOne), 0);
    std::this_thread::sleep_for(std::chrono::milliseconds(BACKGROUND_WAIT_TIME));
    ExpectAbilityCurrentState(abilityName2, AbilityState_Test::BACKGROUND);
    RunningProcessInfo pInfo = STAbilityUtil::GetAppProcessInfoByName(bundleName, appMs_, WAIT_TIME);
    EXPECT_EQ(AppProcessState::APP_STATE_FOREGROUND, pInfo.state_);
    ExpectAbilityNumInStack(abilityName2, 1);
    GTEST_LOG_(INFO) << "AmsDFXTest AMS_DFX_0500 end";
}

/*
 * @tc.number    : AMS_DFX_0600
 * @tc.name      : Lifecycle switch timeout.
 * @tc.desc      : 1.background timeout pageAbility
 *                 2.Force call CompleteBackground function(different bundle)
 */
HWTEST_F(AmsDFXTest, AMS_DFX_0600, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "AmsDFXTest AMS_DFX_0600 start";
    std::string bundleName = bundleNameBase + "Q";
    std::string abilityName = abilityNameBase + "Q1";
    std::string bundleName2 = bundleNameBase + "R";
    std::string abilityName2 = abilityNameBase + "R2";

    MAP_STR_STR params;
    params["targetBundle"] = bundleName;
    params["targetAbility"] = abilityName;
    Want want = STAbilityUtil::MakeWant("device", abilityName2, bundleName2, params);
    STAbilityUtil::StartAbility(want, abilityMs_);

    EXPECT_EQ(STAbilityUtil::WaitCompleted(event_, abilityName2 + abilityStateOnActive, abilityStateCountOne), 0);
    EXPECT_EQ(STAbilityUtil::WaitCompleted(event_, abilityName + abilityStateOnActive, abilityStateCountOne), 0);
    std::this_thread::sleep_for(std::chrono::milliseconds(BACKGROUND_WAIT_TIME));
    ExpectAbilityCurrentState(abilityName2, AbilityState_Test::BACKGROUND);
    RunningProcessInfo pInfo = STAbilityUtil::GetAppProcessInfoByName(bundleName2, appMs_, WAIT_TIME);
    EXPECT_EQ(AppProcessState::APP_STATE_FOREGROUND, pInfo.state_);
    std::cout << "pInfo.state_:" << static_cast<int>(pInfo.state_) << std::endl;
    std::cout << "pInfo.pid_:" << static_cast<int>(pInfo.pid_) << std::endl;
    ExpectAbilityNumInStack(abilityName2, 1);
    GTEST_LOG_(INFO) << "AmsDFXTest AMS_DFX_0600 end";
}

/*
 * @tc.number    : AMS_DFX_0700
 * @tc.name      : Lifecycle switch timeout.
 * @tc.desc      : 1.background timeout pageAbility
 *                 2.Force call CompleteBackground function(different bundle)
 */
HWTEST_F(AmsDFXTest, AMS_DFX_0700, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "AmsDFXTest AMS_DFX_0700 start";
    std::string bundleName = bundleNameBase + "Q";
    std::string abilityName = abilityNameBase + "Q1";
    std::string bundleName2 = bundleNameBase + "R";
    std::string abilityName2 = abilityNameBase + "R3";

    MAP_STR_STR params;
    params["targetBundle"] = bundleName;
    params["targetAbility"] = abilityName;
    Want want = STAbilityUtil::MakeWant("device", abilityName2, bundleName2, params);
    STAbilityUtil::StartAbility(want, abilityMs_);

    EXPECT_EQ(STAbilityUtil::WaitCompleted(event_, abilityName2 + abilityStateOnActive, abilityStateCountOne), 0);
    EXPECT_EQ(STAbilityUtil::WaitCompleted(event_, abilityName + abilityStateOnActive, abilityStateCountOne), 0);
    std::this_thread::sleep_for(std::chrono::milliseconds(BACKGROUND_WAIT_TIME));
    ExpectAbilityCurrentState(abilityName2, AbilityState_Test::BACKGROUND);
    EXPECT_EQ(STAbilityUtil::WaitCompleted(event_, abilityName2 + abilityStateOnBackground, abilityStateCountOne), 0);
    RunningProcessInfo pInfo = STAbilityUtil::GetAppProcessInfoByName(bundleName2, appMs_, WAIT_TIME);
    EXPECT_EQ(AppProcessState::APP_STATE_BACKGROUND, pInfo.state_);
    std::cout << "pInfo.state_:" << static_cast<int>(pInfo.state_) << std::endl;
    std::cout << "pInfo.pid_:" << static_cast<int>(pInfo.pid_) << std::endl;
    ExpectAbilityNumInStack(abilityName2, 1);
    GTEST_LOG_(INFO) << "AmsDFXTest AMS_DFX_0700 end";
}

/*
 * @tc.number    : AMS_DFX_0800
 * @tc.name      : Lifecycle switch timeout.
 * @tc.desc      : 1.Terminate timeout pageAbility
 *                 2.Force call CompleteTerminate function
 */
HWTEST_F(AmsDFXTest, AMS_DFX_0800, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "AmsDFXTest AMS_DFX_0800 start";
    std::string bundleName = bundleNameBase + "Q";
    std::string abilityName = abilityNameBase + "Q1";
    std::string bundleName2 = bundleNameBase + "R";
    std::string abilityName2 = abilityNameBase + "R2";

    MAP_STR_STR params;
    params["targetBundle"] = bundleName;
    params["targetAbility"] = abilityName;
    Want want = STAbilityUtil::MakeWant("device", abilityName2, bundleName2, params);
    STAbilityUtil::StartAbility(want, abilityMs_);

    EXPECT_EQ(STAbilityUtil::WaitCompleted(event_, abilityName2 + abilityStateOnActive, abilityStateCountOne), 0);
    EXPECT_EQ(STAbilityUtil::WaitCompleted(event_, abilityName + abilityStateOnActive, abilityStateCountOne), 0);
    int eventCode = 0;
    STAbilityUtil::PublishEvent(terminatePageAbility, eventCode, abilityName2);
    std::this_thread::sleep_for(std::chrono::milliseconds(BACKGROUND_WAIT_TIME));
    ExpectAbilityNumInStack(abilityName2, 0);
    GTEST_LOG_(INFO) << "AmsDFXTest AMS_DFX_0800 end";
}

/*
 * @tc.number    : AMS_DFX_0900
 * @tc.name      : Lifecycle switch timeout.
 * @tc.desc      : 1.Terminate timeout pageAbility
 *                 2.Force call CompleteTerminate function
 */
HWTEST_F(AmsDFXTest, AMS_DFX_0900, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "AmsDFXTest AMS_DFX_0900 start";
    std::string bundleName = bundleNameBase + "Q";
    std::string abilityName = abilityNameBase + "Q1";
    std::string bundleName2 = bundleNameBase + "R";
    std::string abilityName2 = abilityNameBase + "R2";

    MAP_STR_STR params;
    params["targetBundle"] = bundleName;
    params["targetAbility"] = abilityName;
    Want want = STAbilityUtil::MakeWant("device", abilityName2, bundleName2, params);
    STAbilityUtil::StartAbility(want, abilityMs_);

    EXPECT_EQ(STAbilityUtil::WaitCompleted(event_, abilityName2 + abilityStateOnActive, abilityStateCountOne), 0);
    EXPECT_EQ(STAbilityUtil::WaitCompleted(event_, abilityName + abilityStateOnActive, abilityStateCountOne), 0);
    int eventCode = 0;
    STAbilityUtil::PublishEvent(terminatePageAbility, eventCode, abilityName2);
    std::this_thread::sleep_for(std::chrono::milliseconds(BACKGROUND_WAIT_TIME));
    ExpectAbilityNumInStack(abilityName2, 0);
    std::this_thread::sleep_for(std::chrono::milliseconds(WAIT_TIME));
    params.clear();
    want = STAbilityUtil::MakeWant("device", abilityName2, bundleName2, params);
    STAbilityUtil::StartAbility(want, abilityMs_);
    EXPECT_EQ(STAbilityUtil::WaitCompleted(event_, abilityName2 + abilityStateOnActive, abilityStateCountOne), 0);
    GTEST_LOG_(INFO) << "AmsDFXTest AMS_DFX_0900 end";
}

/*
 * @tc.number    : AMS_DFX_1000
 * @tc.name      : service ability connection,disconnection exception.
 * @tc.desc      : 1.connection service ability timeout
 *                 2.verification callBack resultCode
 */
HWTEST_F(AmsDFXTest, AMS_DFX_1000, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "AmsDFXTest AMS_DFX_1000 start";
    std::string bundleName = bundleNameBase + "DFX";
    std::string abilityName = "AmsDfxStServiceAbilityA1";

    MAP_STR_STR params;
    Want want = STAbilityUtil::MakeWant("device", abilityName, bundleName, params);
    // connect service ability
    sptr<AbilityConnectCallback> stub(new (std::nothrow) AbilityConnectCallback());
    STAbilityUtil::ConnectAbility(want, stub, stub->AsObject());

    EXPECT_EQ(STAbilityUtil::WaitCompleted(event_, abilityName + abilityStateOnStart, abilityStateCountOne), 0);
    std::this_thread::sleep_for(std::chrono::milliseconds(WAIT_TIME));
    EXPECT_EQ(AbilityConnectCallback::resultConnectCode, CONNECTION_TIMEOUT);
    ExpectServiceAbilityNumInStack(abilityName, 1);

    RunningProcessInfo pInfo = STAbilityUtil::GetAppProcessInfoByName(bundleName, appMs_, WAIT_TIME);
    EXPECT_TRUE(pInfo.pid_ > 0);

    int eventCode = 0;
    STAbilityUtil::PublishEvent(terminatePageAbility, eventCode, abilityName);
    EXPECT_EQ(STAbilityUtil::WaitCompleted(event_, abilityName + abilityStateOnStop, abilityStateCountOne), 0);
    pInfo = STAbilityUtil::GetAppProcessInfoByName(bundleName, appMs_, WAIT_TIME);
    EXPECT_TRUE(pInfo.pid_ == 0);
    GTEST_LOG_(INFO) << "AmsDFXTest AMS_DFX_1000 end";
}

/*
 * @tc.number    : AMS_DFX_1100
 * @tc.name      : service ability connection,disconnection exception.
 * @tc.desc      : 1.disconnection service ability timeout
 *                 2.verification disconnection
 */
HWTEST_F(AmsDFXTest, AMS_DFX_1100, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "AmsDFXTest AMS_DFX_1100 start";
    std::string bundleName = bundleNameBase + "DFX";
    std::string abilityName = "AmsDfxStServiceAbilityA2";

    MAP_STR_STR params;
    Want want = STAbilityUtil::MakeWant("device", abilityName, bundleName, params);
    // connect service ability
    sptr<AbilityConnectCallback> stub(new (std::nothrow) AbilityConnectCallback());
    STAbilityUtil::ConnectAbility(want, stub, stub->AsObject());

    EXPECT_EQ(STAbilityUtil::WaitCompleted(event_, abilityName + abilityStateOnStart, abilityStateCountOne), 0);
    std::this_thread::sleep_for(std::chrono::milliseconds(WAIT_TIME));
    EXPECT_EQ(AbilityConnectCallback::resultConnectCode, 0);
    ExpectServiceAbilityNumInStack(abilityName, 1);

    RunningProcessInfo pInfo = STAbilityUtil::GetAppProcessInfoByName(bundleName, appMs_, WAIT_TIME);
    EXPECT_TRUE(pInfo.pid_ > 0);

    STAbilityUtil::DisconnectAbility(stub);
    std::this_thread::sleep_for(std::chrono::milliseconds(WAIT_TIME));
    ExpectServiceAbilityNumInStack(abilityName, 0);
    GTEST_LOG_(INFO) << "AmsDFXTest AMS_DFX_1100 end";
}