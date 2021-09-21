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
std::vector<std::string> bundleNameSuffix = {"PageA", "ServiceB"};
static const std::string bundleNameBase = "com.ohos.amsst.appAbilityVisible";
static const std::string hapNameBase = "amsAbilityVisibleTest";
static const std::string abilityNameBase = "AmsAbilityVisibleTest";
static const std::string launcherBundle = "com.ix.launcher";
static const std::string launcherAbility = "LauncherAbility";
static const std::string systemUiBundle = "com.ohos.systemui";
static const std::string terminateAbility = "requ_page_ability_terminate";
static const std::string terminateAbilityCaller = "requ_page_ability_terminate_caller";
static const std::string disconnectService = "requ_disconnect_service";
static const std::string terminateAbilityResult = "requ_page_ability_terminate_result";

static const std::string DUMP_STACK_LIST = "--stack-list";
static const std::string DUMP_SERVICE = "--serv";
static const std::string DUMP_DATA = "--data";
static const std::string DUMP_STACK = "--stack";
static const std::string DUMP_MISSION = "--mission";
static const std::string DUMP_TOP = "--top";
static const std::string DUMP_ALL = "-a";
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
static const std::string abilityStateInsert = ":Insert";
static const std::string abilityStateDelete = ":Delete";
static const std::string abilityStateUpdate = ":Update";
static const std::string abilityStateQuery = ":Query";
static const std::string abilityStateGetFileTypes = ":GetFileTypes";
static const std::string abilityStateOpenFile = ":OpenFile";
static const std::string getWantAgentState = ":GetWantAgentFail";
static const std::string triggerWantAgentState = ":TriggerWantAgentSuccess";
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

class AmsAbilityVisibleTest : public testing::Test {
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
    void ExpectAbilityNumInStack(const std::string &abilityName, int abilityNum, const std::string &type = "-a");
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
    static int *memTestValue_;
};

sptr<IAppMgr> AmsAbilityVisibleTest::appMs_ = nullptr;
sptr<IAbilityManager> AmsAbilityVisibleTest::abilityMs_ = nullptr;
STtools::Event AmsAbilityVisibleTest::event_ = STtools::Event();
int *AmsAbilityVisibleTest::memTestValue_ = nullptr;
std::shared_ptr<AmsAbilityVisibleTest::AppEventSubscriber> AmsAbilityVisibleTest::subscriber_ = nullptr;
size_t AmsAbilityVisibleTest::AbilityConnectCallback::onAbilityConnectDoneCount = 0;
int AmsAbilityVisibleTest::AbilityConnectCallback::resultConnectCode = 0;

void AmsAbilityVisibleTest::SetUpTestCase(void)
{
    GTEST_LOG_(INFO) << "void AmsAbilityVisibleTest::SetUpTestCase(void)";

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

void AmsAbilityVisibleTest::TearDownTestCase(void)
{
    GTEST_LOG_(INFO) << "void AmsAbilityVisibleTest::TearDownTestCase(void)";
    CommonEventManager::UnSubscribeCommonEvent(subscriber_);
}

void AmsAbilityVisibleTest::SetUp(void)
{
    GTEST_LOG_(INFO) << "void AmsAbilityVisibleTest::SetUp(void)";
    std::vector<std::string> hapNames = GetBundleNames(hapNameBase, bundleNameSuffix);
    STAbilityUtil::InstallHaps(hapNames);
}

void AmsAbilityVisibleTest::TearDown(void)
{
    GTEST_LOG_(INFO) << "void AmsAbilityVisibleTest::TearDown(void)";
    if (memTestValue_) {
        delete[] memTestValue_;
    }

    std::vector<std::string> bundleNames = GetBundleNames(bundleNameBase, bundleNameSuffix);
    STAbilityUtil::UninstallBundle(bundleNames);

    STAbilityUtil::CleanMsg(event_);
}

std::vector<std::string> AmsAbilityVisibleTest::GetBundleNames(
    const std::string &strBase, const std::vector<std::string> &strSuffixs)
{
    std::vector<std::string> bundleNames;
    for (auto strSuffix : strSuffixs) {
        bundleNames.push_back(strBase + strSuffix);
    }
    return bundleNames;
}

bool AmsAbilityVisibleTest::SubscribeEvent()
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

void AmsAbilityVisibleTest::ShowDump()
{
    if (abilityMs_) {
        std::vector<std::string> dumpInfo;
        abilityMs_->DumpState("-a", dumpInfo);
        for (const auto &info : dumpInfo) {
            std::cout << info << std::endl;
        }
        dumpInfo.clear();
        abilityMs_->DumpState(DUMP_SERVICE, dumpInfo);
        for (const auto &info : dumpInfo) {
            std::cout << info << std::endl;
        }
        dumpInfo.clear();
        abilityMs_->DumpState(DUMP_DATA, dumpInfo);
        for (const auto &info : dumpInfo) {
            std::cout << info << std::endl;
        }
    }
}

void AmsAbilityVisibleTest::CheckAbilityStateByName(const std::string &abilityName,
    const std::vector<std::string> &info, const std::string &state, const std::string &midState)
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

void AmsAbilityVisibleTest::ExpectAbilityCurrentState(const std::string &abilityName,
    const AbilityState_Test &currentState, const AbilityState_Test &midState, const std::string &args)
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

void AmsAbilityVisibleTest::ExpectAbilityNumInStack(
    const std::string &abilityName, int abilityNum, const std::string &type)
{
    std::vector<std::string> dumpInfo;
    if (abilityMs_ != nullptr) {
        abilityMs_->DumpState(type, dumpInfo);
        std::vector<std::string> result;
        MTDumpUtil::GetInstance()->GetAll("AbilityName", dumpInfo, result);
        // only one record in stack
        EXPECT_EQ(abilityNum, std::count(result.begin(), result.end(), abilityName));
    } else {
        HILOG_ERROR("ability manager service(abilityMs_) is nullptr");
    }
}

void AmsAbilityVisibleTest::ExpectServiceAbilityNumInStack(const std::string &abilityName, int abilityNum)
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

void AmsAbilityVisibleTest::AppEventSubscriber::OnReceiveEvent(const CommonEventData &data)
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
 * @tc.number    : AMS_ABILITY_VISIBLE_0100
 * @tc.name      : Visible attribute impact on startAbility
 * @tc.desc      : The test process starts an ability whose visible attribute is false
 */
HWTEST_F(AmsAbilityVisibleTest, AMS_ABILITY_VISIBLE_0100, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "AmsAbilityVisibleTest AMS_ABILITY_VISIBLE_0100 start";
    std::string bundleName = bundleNameBase + "PageA";
    std::string abilityName = abilityNameBase + "PageA1";

    MAP_STR_STR params;
    Want want = STAbilityUtil::MakeWant("device", abilityName, bundleName, params);
    ErrCode result = STAbilityUtil::StartAbility(want, abilityMs_);
    EXPECT_EQ(result, ABILITY_VISIBLE_FALSE_DENY_REQUEST);

    ExpectAbilityNumInStack(abilityName, 0);
    GTEST_LOG_(INFO) << "AmsAbilityVisibleTest AMS_ABILITY_VISIBLE_0100 end";
}

/*
 * @tc.number    : AMS_ABILITY_VISIBLE_0200
 * @tc.name      : Visible attribute impact on startAbility
 * @tc.desc      : The test process starts an ability,
 *                 and the visible attribute of the ability is the default value.
 */
HWTEST_F(AmsAbilityVisibleTest, AMS_ABILITY_VISIBLE_0200, TestSize.Level2)
{
    GTEST_LOG_(INFO) << "AmsAbilityVisibleTest AMS_ABILITY_VISIBLE_0200 start";
    std::string bundleName = bundleNameBase + "PageA";
    std::string abilityName = abilityNameBase + "PageA4";

    MAP_STR_STR params;
    Want want = STAbilityUtil::MakeWant("device", abilityName, bundleName, params);
    ErrCode result = STAbilityUtil::StartAbility(want, abilityMs_);
    EXPECT_EQ(result, ABILITY_VISIBLE_FALSE_DENY_REQUEST);

    ExpectAbilityNumInStack(abilityName, 0);
    GTEST_LOG_(INFO) << "AmsAbilityVisibleTest AMS_ABILITY_VISIBLE_0200 end";
}

/*
 * @tc.number    : AMS_ABILITY_VISIBLE_0300
 * @tc.name      : Visible attribute impact on startAbility
 * @tc.desc      : The test process starts an ability whose visible attribute is true
 */
HWTEST_F(AmsAbilityVisibleTest, AMS_ABILITY_VISIBLE_0300, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "AmsAbilityVisibleTest AMS_ABILITY_VISIBLE_0300 start";
    std::string bundleName = bundleNameBase + "PageA";
    std::string abilityName = abilityNameBase + "PageA2";

    MAP_STR_STR params;
    Want want = STAbilityUtil::MakeWant("device", abilityName, bundleName, params);
    STAbilityUtil::StartAbility(want, abilityMs_);
    EXPECT_EQ(STAbilityUtil::WaitCompleted(event_, abilityName + abilityStateOnStart, abilityStateCountOne), 0);
    EXPECT_EQ(STAbilityUtil::WaitCompleted(event_, abilityName + abilityStateOnActive, abilityStateCountOne), 0);

    ExpectAbilityNumInStack(abilityName, 1);
    ExpectAbilityCurrentState(abilityName, AbilityState_Test::ACTIVE, AbilityState_Test::ACTIVATING);
    GTEST_LOG_(INFO) << "AmsAbilityVisibleTest AMS_ABILITY_VISIBLE_0300 end";
}

/*
 * @tc.number    : AMS_ABILITY_VISIBLE_0400
 * @tc.name      : Visible attribute impact on startAbility
 * @tc.desc      : 1.The test process starts an ability(PageA2) whose visible attribute is true
 *                 2.The ability(PageA2) starts an ability(PageA1) whose visible attribute is false
 *                 3.PageA2 has the same bundleName as PageA1
 */
HWTEST_F(AmsAbilityVisibleTest, AMS_ABILITY_VISIBLE_0400, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "AmsAbilityVisibleTest AMS_ABILITY_VISIBLE_0400 start";
    std::string bundleName = bundleNameBase + "PageA";
    std::string abilityName = abilityNameBase + "PageA2";
    std::string abilityName2 = abilityNameBase + "PageA1";

    MAP_STR_STR params;
    params["targetBundle"] = bundleName;
    params["targetAbility"] = abilityName2;
    params["type"] = "Page";

    Want want = STAbilityUtil::MakeWant("device", abilityName, bundleName, params);
    STAbilityUtil::StartAbility(want, abilityMs_);
    EXPECT_EQ(STAbilityUtil::WaitCompleted(event_, abilityName + abilityStateOnStart, abilityStateCountOne), 0);
    EXPECT_EQ(STAbilityUtil::WaitCompleted(event_, abilityName + abilityStateOnActive, abilityStateCountOne), 0);
    EXPECT_EQ(STAbilityUtil::WaitCompleted(event_, abilityName2 + abilityStateOnStart, abilityStateCountOne), 0);
    EXPECT_EQ(STAbilityUtil::WaitCompleted(event_, abilityName2 + abilityStateOnActive, abilityStateCountOne), 0);
    EXPECT_EQ(STAbilityUtil::WaitCompleted(event_, abilityName + abilityStateOnInactive, abilityStateCountOne), 0);
    EXPECT_EQ(STAbilityUtil::WaitCompleted(event_, abilityName + abilityStateOnBackground, abilityStateCountOne), 0);

    ExpectAbilityNumInStack(abilityName, 1);
    ExpectAbilityCurrentState(abilityName, AbilityState_Test::BACKGROUND, AbilityState_Test::MOVING_BACKGROUND);
    ExpectAbilityCurrentState(abilityName2, AbilityState_Test::ACTIVE, AbilityState_Test::ACTIVATING);
    GTEST_LOG_(INFO) << "AmsAbilityVisibleTest AMS_ABILITY_VISIBLE_0400 end";
}

/*
 * @tc.number    : AMS_ABILITY_VISIBLE_0500
 * @tc.name      : Visible attribute impact on startAbility
 * @tc.desc      : 1.The test process starts an ability(PageA2) whose visible attribute is true
 *                 2.The ability(PageA2) starts an ability(PageA3,type:singleton) whose visible attribute is false
 *                 3.PageA2 has the same bundleName as PageA3
 */
HWTEST_F(AmsAbilityVisibleTest, AMS_ABILITY_VISIBLE_0500, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "AmsAbilityVisibleTest AMS_ABILITY_VISIBLE_0500 start";
    std::string bundleName = bundleNameBase + "PageA";
    std::string abilityName = abilityNameBase + "PageA2";
    std::string abilityName2 = abilityNameBase + "PageA3";

    MAP_STR_STR params;
    params["targetBundle"] = bundleName;
    params["targetAbility"] = abilityName2;
    params["type"] = "Page";

    Want want = STAbilityUtil::MakeWant("device", abilityName, bundleName, params);
    STAbilityUtil::StartAbility(want, abilityMs_);
    EXPECT_EQ(STAbilityUtil::WaitCompleted(event_, abilityName + abilityStateOnStart, abilityStateCountOne), 0);
    EXPECT_EQ(STAbilityUtil::WaitCompleted(event_, abilityName + abilityStateOnActive, abilityStateCountOne), 0);
    EXPECT_EQ(STAbilityUtil::WaitCompleted(event_, abilityName2 + abilityStateOnStart, abilityStateCountOne), 0);
    EXPECT_EQ(STAbilityUtil::WaitCompleted(event_, abilityName2 + abilityStateOnActive, abilityStateCountOne), 0);
    EXPECT_EQ(STAbilityUtil::WaitCompleted(event_, abilityName + abilityStateOnInactive, abilityStateCountOne), 0);
    EXPECT_EQ(STAbilityUtil::WaitCompleted(event_, abilityName + abilityStateOnBackground, abilityStateCountOne), 0);

    ExpectAbilityNumInStack(abilityName, 1);
    ExpectAbilityNumInStack(abilityName2, 1);
    ExpectAbilityCurrentState(abilityName, AbilityState_Test::BACKGROUND, AbilityState_Test::MOVING_BACKGROUND);
    ExpectAbilityCurrentState(abilityName2, AbilityState_Test::ACTIVE, AbilityState_Test::ACTIVATING);
    GTEST_LOG_(INFO) << "AmsAbilityVisibleTest AMS_ABILITY_VISIBLE_0500 end";
}

/*
 * @tc.number    : AMS_ABILITY_VISIBLE_0600
 * @tc.name      : Visible attribute impact on startAbility
 * @tc.desc      : 1.The test process starts an ability(PageA2) whose visible attribute is true
 *                 2.The ability(PageA2) starts an ability(PageB1) whose visible attribute is false
 *                 3.PageA2 has the different bundleName as PageB1
 */
HWTEST_F(AmsAbilityVisibleTest, AMS_ABILITY_VISIBLE_0600, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "AmsAbilityVisibleTest AMS_ABILITY_VISIBLE_0600 start";
    std::string bundleName = bundleNameBase + "PageA";
    std::string abilityName = abilityNameBase + "PageA2";
    std::string bundleName2 = bundleNameBase + "ServiceB";
    std::string abilityName2 = abilityNameBase + "PageB1";

    MAP_STR_STR params;
    params["targetBundle"] = bundleName2;
    params["targetAbility"] = abilityName2;
    params["type"] = "Page";

    Want want = STAbilityUtil::MakeWant("device", abilityName, bundleName, params);
    STAbilityUtil::StartAbility(want, abilityMs_);
    EXPECT_EQ(STAbilityUtil::WaitCompleted(event_, abilityName + abilityStateOnStart, abilityStateCountOne), 0);
    EXPECT_EQ(STAbilityUtil::WaitCompleted(event_, abilityName + abilityStateOnActive, abilityStateCountOne), 0);

    ExpectAbilityNumInStack(abilityName, 1);
    ExpectAbilityNumInStack(abilityName2, 0);
    ExpectAbilityCurrentState(abilityName, AbilityState_Test::ACTIVE, AbilityState_Test::ACTIVATING);
    GTEST_LOG_(INFO) << "AmsAbilityVisibleTest AMS_ABILITY_VISIBLE_0600 end";
}

/*
 * @tc.number    : AMS_ABILITY_VISIBLE_0700
 * @tc.name      : Visible attribute impact on startAbility
 * @tc.desc      : 1.The test process starts an ability(PageA2) whose visible attribute is true
 *                 2.The ability(PageA2) starts an ability(PageB2) whose visible attribute is true
 *                 3.PageA2 has the different bundleName as PageB2
 */
HWTEST_F(AmsAbilityVisibleTest, AMS_ABILITY_VISIBLE_0700, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "AmsAbilityVisibleTest AMS_ABILITY_VISIBLE_0700 start";
    std::string bundleName = bundleNameBase + "PageA";
    std::string abilityName = abilityNameBase + "PageA2";
    std::string bundleName2 = bundleNameBase + "ServiceB";
    std::string abilityName2 = abilityNameBase + "PageB2";

    MAP_STR_STR params;
    params["targetBundle"] = bundleName2;
    params["targetAbility"] = abilityName2;
    params["type"] = "Page";

    Want want = STAbilityUtil::MakeWant("device", abilityName, bundleName, params);
    STAbilityUtil::StartAbility(want, abilityMs_);
    EXPECT_EQ(STAbilityUtil::WaitCompleted(event_, abilityName + abilityStateOnStart, abilityStateCountOne), 0);
    EXPECT_EQ(STAbilityUtil::WaitCompleted(event_, abilityName + abilityStateOnActive, abilityStateCountOne), 0);
    EXPECT_EQ(STAbilityUtil::WaitCompleted(event_, abilityName2 + abilityStateOnStart, abilityStateCountOne), 0);
    EXPECT_EQ(STAbilityUtil::WaitCompleted(event_, abilityName2 + abilityStateOnActive, abilityStateCountOne), 0);
    EXPECT_EQ(STAbilityUtil::WaitCompleted(event_, abilityName + abilityStateOnInactive, abilityStateCountOne), 0);
    EXPECT_EQ(STAbilityUtil::WaitCompleted(event_, abilityName + abilityStateOnBackground, abilityStateCountOne), 0);

    ExpectAbilityNumInStack(abilityName, 1);
    ExpectAbilityNumInStack(abilityName2, 1);
    ExpectAbilityCurrentState(abilityName, AbilityState_Test::BACKGROUND, AbilityState_Test::MOVING_BACKGROUND);
    ExpectAbilityCurrentState(abilityName2, AbilityState_Test::ACTIVE, AbilityState_Test::ACTIVATING);
    GTEST_LOG_(INFO) << "AmsAbilityVisibleTest AMS_ABILITY_VISIBLE_0700 end";
}

/*
 * @tc.number    : AMS_ABILITY_VISIBLE_0800
 * @tc.name      : Visible attribute impact on startAbility
 * @tc.desc      : 1.The test process starts an ability(PageA2) whose visible attribute is true
 *                 2.The ability(PageA2) starts a Service ability(Service) whose visible attribute is false
 *                 3.PageA2 has the different bundleName as Service
 */
HWTEST_F(AmsAbilityVisibleTest, AMS_ABILITY_VISIBLE_0800, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "AmsAbilityVisibleTest AMS_ABILITY_VISIBLE_0800 start";
    std::string bundleName = bundleNameBase + "PageA";
    std::string abilityName = abilityNameBase + "PageA2";
    std::string bundleName2 = bundleNameBase + "ServiceB";
    std::string abilityName2 = abilityNameBase + "Service";

    MAP_STR_STR params;
    params["targetBundle"] = bundleName2;
    params["targetAbility"] = abilityName2;
    params["type"] = "Page";

    Want want = STAbilityUtil::MakeWant("device", abilityName, bundleName, params);
    STAbilityUtil::StartAbility(want, abilityMs_);
    EXPECT_EQ(STAbilityUtil::WaitCompleted(event_, abilityName + abilityStateOnStart, abilityStateCountOne), 0);
    EXPECT_EQ(STAbilityUtil::WaitCompleted(event_, abilityName + abilityStateOnActive, abilityStateCountOne), 0);

    ExpectAbilityNumInStack(abilityName, 1);
    ExpectAbilityNumInStack(abilityName2, 0, DUMP_SERVICE);
    ExpectAbilityCurrentState(abilityName, AbilityState_Test::ACTIVE, AbilityState_Test::ACTIVATING);
    GTEST_LOG_(INFO) << "AmsAbilityVisibleTest AMS_ABILITY_VISIBLE_0800 end";
}

/*
 * @tc.number    : AMS_ABILITY_VISIBLE_0900
 * @tc.name      : Visible attribute impact on startAbility
 * @tc.desc      : 1.The test process starts an ability(PageB2) whose visible attribute is true
 *                 2.The ability(PageB2) starts a Service ability(Service) whose visible attribute is false
 *                 3.PageB2 has the same bundleName as Service
 */
HWTEST_F(AmsAbilityVisibleTest, AMS_ABILITY_VISIBLE_0900, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "AmsAbilityVisibleTest AMS_ABILITY_VISIBLE_0900 start";
    std::string bundleName = bundleNameBase + "ServiceB";
    std::string abilityName = abilityNameBase + "PageB2";
    std::string bundleName2 = bundleNameBase + "ServiceB";
    std::string abilityName2 = abilityNameBase + "Service";

    MAP_STR_STR params;
    params["targetBundle"] = bundleName2;
    params["targetAbility"] = abilityName2;
    params["type"] = "Page";

    Want want = STAbilityUtil::MakeWant("device", abilityName, bundleName, params);
    STAbilityUtil::StartAbility(want, abilityMs_);
    EXPECT_EQ(STAbilityUtil::WaitCompleted(event_, abilityName + abilityStateOnStart, abilityStateCountOne), 0);
    EXPECT_EQ(STAbilityUtil::WaitCompleted(event_, abilityName + abilityStateOnActive, abilityStateCountOne), 0);
    EXPECT_EQ(STAbilityUtil::WaitCompleted(event_, abilityName2 + abilityStateOnStart, abilityStateCountOne), 0);
    EXPECT_EQ(STAbilityUtil::WaitCompleted(event_, abilityName2 + abilityStateOnCommand, abilityStateCountOne), 0);

    ExpectAbilityNumInStack(abilityName, 1);
    ExpectAbilityNumInStack(abilityName2, 1, DUMP_SERVICE);
    ExpectAbilityCurrentState(abilityName, AbilityState_Test::ACTIVE, AbilityState_Test::ACTIVATING);
    GTEST_LOG_(INFO) << "AmsAbilityVisibleTest AMS_ABILITY_VISIBLE_0900 end";
}

/*
 * @tc.number    : AMS_ABILITY_VISIBLE_1000
 * @tc.name      : Visible attribute impact on ConnectAbility
 * @tc.desc      : 1.The test process starts an ability(PageA2) whose visible attribute is true
 *                 2.The ability(PageA2) Connect a Service ability(Service) whose visible attribute is false
 *                 3.PageA2 has the different bundleName as Service
 */
HWTEST_F(AmsAbilityVisibleTest, AMS_ABILITY_VISIBLE_1000, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "AmsAbilityVisibleTest AMS_ABILITY_VISIBLE_1000 start";
    std::string bundleName = bundleNameBase + "PageA";
    std::string abilityName = abilityNameBase + "PageA2";
    std::string bundleName2 = bundleNameBase + "ServiceB";
    std::string abilityName2 = abilityNameBase + "Service";

    MAP_STR_STR params;
    params["targetBundle"] = bundleName2;
    params["targetAbility"] = abilityName2;
    params["type"] = "Service";

    Want want = STAbilityUtil::MakeWant("device", abilityName, bundleName, params);
    STAbilityUtil::StartAbility(want, abilityMs_);
    EXPECT_EQ(STAbilityUtil::WaitCompleted(event_, abilityName + abilityStateOnStart, abilityStateCountOne), 0);
    EXPECT_EQ(STAbilityUtil::WaitCompleted(event_, abilityName + abilityStateOnActive, abilityStateCountOne), 0);

    ExpectAbilityNumInStack(abilityName, 1);
    ExpectAbilityNumInStack(abilityName2, 0, DUMP_SERVICE);
    ExpectAbilityCurrentState(abilityName, AbilityState_Test::ACTIVE, AbilityState_Test::ACTIVATING);
    GTEST_LOG_(INFO) << "AmsAbilityVisibleTest AMS_ABILITY_VISIBLE_1000 end";
}

/*
 * @tc.number    : AMS_ABILITY_VISIBLE_1100
 * @tc.name      : Visible attribute impact on ConnectAbility
 * @tc.desc      : 1.The test process starts an ability(PageB2) whose visible attribute is true
 *                 2.The ability(PageB2) Connect a Service ability(Service) whose visible attribute is false
 *                 3.PageB2 has the same bundleName as Service
 */
HWTEST_F(AmsAbilityVisibleTest, AMS_ABILITY_VISIBLE_1100, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "AmsAbilityVisibleTest AMS_ABILITY_VISIBLE_1100 start";
    std::string bundleName = bundleNameBase + "ServiceB";
    std::string abilityName = abilityNameBase + "PageB2";
    std::string bundleName2 = bundleNameBase + "ServiceB";
    std::string abilityName2 = abilityNameBase + "Service";

    MAP_STR_STR params;
    params["targetBundle"] = bundleName2;
    params["targetAbility"] = abilityName2;
    params["type"] = "Service";

    Want want = STAbilityUtil::MakeWant("device", abilityName, bundleName, params);
    STAbilityUtil::StartAbility(want, abilityMs_);
    EXPECT_EQ(STAbilityUtil::WaitCompleted(event_, abilityName + abilityStateOnStart, abilityStateCountOne), 0);
    EXPECT_EQ(STAbilityUtil::WaitCompleted(event_, abilityName + abilityStateOnActive, abilityStateCountOne), 0);
    EXPECT_EQ(STAbilityUtil::WaitCompleted(event_, abilityName2 + abilityStateOnStart, abilityStateCountOne), 0);
    EXPECT_EQ(STAbilityUtil::WaitCompleted(event_, abilityName2 + abilityStateOnConnect, abilityStateCountOne), 0);

    ExpectAbilityNumInStack(abilityName, 1);
    ExpectAbilityNumInStack(abilityName2, 1, DUMP_SERVICE);
    ExpectAbilityCurrentState(abilityName, AbilityState_Test::ACTIVE, AbilityState_Test::ACTIVATING);
    GTEST_LOG_(INFO) << "AmsAbilityVisibleTest AMS_ABILITY_VISIBLE_1100 end";
}

/*
 * @tc.number    : AMS_ABILITY_VISIBLE_1200
 * @tc.name      : Visible attribute impact on start dataAbility
 * @tc.desc      : 1.The test process starts an ability(PageA2) whose visible attribute is true
 *                 2.The ability(PageA2) start a Data ability(Data) whose visible attribute is false
 *                 3.PageA2 has the different bundleName as Data
 */
HWTEST_F(AmsAbilityVisibleTest, AMS_ABILITY_VISIBLE_1200, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "AmsAbilityVisibleTest AMS_ABILITY_VISIBLE_1200 start";
    std::string bundleName = bundleNameBase + "PageA";
    std::string abilityName = abilityNameBase + "PageA2";
    std::string bundleName2 = bundleNameBase + "ServiceB";
    std::string abilityName2 = abilityNameBase + "Data";

    MAP_STR_STR params;
    params["targetBundle"] = bundleName2;
    params["targetAbility"] = abilityName2;
    params["type"] = "Data";

    Want want = STAbilityUtil::MakeWant("device", abilityName, bundleName, params);
    STAbilityUtil::StartAbility(want, abilityMs_);
    EXPECT_EQ(STAbilityUtil::WaitCompleted(event_, abilityName + abilityStateOnStart, abilityStateCountOne), 0);
    EXPECT_EQ(STAbilityUtil::WaitCompleted(event_, abilityName + abilityStateOnActive, abilityStateCountOne), 0);

    ExpectAbilityNumInStack(abilityName, 1);
    ExpectAbilityNumInStack(abilityName2, 0, DUMP_DATA);
    ExpectAbilityCurrentState(abilityName, AbilityState_Test::ACTIVE, AbilityState_Test::ACTIVATING);
    GTEST_LOG_(INFO) << "AmsAbilityVisibleTest AMS_ABILITY_VISIBLE_1200 end";
}

/*
 * @tc.number    : AMS_ABILITY_VISIBLE_1300
 * @tc.name      : Visible attribute impact on start dataAbility
 * @tc.desc      : 1.The test process starts an ability(PageB2) whose visible attribute is true
 *                 2.The ability(PageB2) start a Data ability(Data) whose visible attribute is false
 *                 3.PageB2 has the same bundleName as Data
 */
HWTEST_F(AmsAbilityVisibleTest, AMS_ABILITY_VISIBLE_1300, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "AmsAbilityVisibleTest AMS_ABILITY_VISIBLE_1300 start";
    std::string bundleName = bundleNameBase + "ServiceB";
    std::string abilityName = abilityNameBase + "PageB2";
    std::string bundleName2 = bundleNameBase + "ServiceB";
    std::string abilityName2 = abilityNameBase + "Data";

    MAP_STR_STR params;
    params["targetBundle"] = bundleName2;
    params["targetAbility"] = abilityName2;
    params["type"] = "Data";

    Want want = STAbilityUtil::MakeWant("device", abilityName, bundleName, params);
    STAbilityUtil::StartAbility(want, abilityMs_);
    EXPECT_EQ(STAbilityUtil::WaitCompleted(event_, abilityName + abilityStateOnStart, abilityStateCountOne), 0);
    EXPECT_EQ(STAbilityUtil::WaitCompleted(event_, abilityName + abilityStateOnActive, abilityStateCountOne), 0);
    EXPECT_EQ(STAbilityUtil::WaitCompleted(event_, abilityName2 + abilityStateOnStart, abilityStateCountOne), 0);
    EXPECT_EQ(STAbilityUtil::WaitCompleted(event_, abilityName2 + abilityStateInsert, abilityStateCountOne), 0);

    ExpectAbilityNumInStack(abilityName, 1);
    ExpectAbilityNumInStack(abilityName2, 1, DUMP_DATA);
    ExpectAbilityCurrentState(abilityName, AbilityState_Test::ACTIVE, AbilityState_Test::ACTIVATING);
    GTEST_LOG_(INFO) << "AmsAbilityVisibleTest AMS_ABILITY_VISIBLE_1300 end";
}

/*
 * @tc.number    : AMS_ABILITY_VISIBLE_1400
 * @tc.name      : Visible attribute impact on terminateAbility
 * @tc.desc      : 1.The test process starts an ability(PageA2) whose visible attribute is true
 *                 2.The ability(PageA2) starts an ability(PageA3,type:singleton) whose visible attribute is false
 *                 3.PageA2 has the same bundleName as PageA3
 *                 4.terminate PageA3
 */
HWTEST_F(AmsAbilityVisibleTest, AMS_ABILITY_VISIBLE_1400, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "AmsAbilityVisibleTest AMS_ABILITY_VISIBLE_1400 start";
    std::string bundleName = bundleNameBase + "PageA";
    std::string abilityName = abilityNameBase + "PageA2";
    std::string abilityName2 = abilityNameBase + "PageA3";

    MAP_STR_STR params;
    params["targetBundle"] = bundleName;
    params["targetAbility"] = abilityName2;
    params["type"] = "Page";

    Want want = STAbilityUtil::MakeWant("device", abilityName, bundleName, params);
    STAbilityUtil::StartAbility(want, abilityMs_);
    EXPECT_EQ(STAbilityUtil::WaitCompleted(event_, abilityName + abilityStateOnStart, abilityStateCountOne), 0);
    EXPECT_EQ(STAbilityUtil::WaitCompleted(event_, abilityName + abilityStateOnActive, abilityStateCountOne), 0);
    EXPECT_EQ(STAbilityUtil::WaitCompleted(event_, abilityName2 + abilityStateOnStart, abilityStateCountOne), 0);
    EXPECT_EQ(STAbilityUtil::WaitCompleted(event_, abilityName2 + abilityStateOnActive, abilityStateCountOne), 0);
    EXPECT_EQ(STAbilityUtil::WaitCompleted(event_, abilityName + abilityStateOnInactive, abilityStateCountOne), 0);
    EXPECT_EQ(STAbilityUtil::WaitCompleted(event_, abilityName + abilityStateOnBackground, abilityStateCountOne), 0);

    ExpectAbilityNumInStack(abilityName, 1);
    ExpectAbilityNumInStack(abilityName2, 1);
    ExpectAbilityCurrentState(abilityName, AbilityState_Test::BACKGROUND, AbilityState_Test::MOVING_BACKGROUND);
    ExpectAbilityCurrentState(abilityName2, AbilityState_Test::ACTIVE, AbilityState_Test::ACTIVATING);
    int eventCode = -1;
    STAbilityUtil::PublishEvent(terminateAbility, eventCode, abilityName2);
    EXPECT_EQ(STAbilityUtil::WaitCompleted(event_, abilityName2 + abilityStateOnInactive, abilityStateCountOne), 0);
    EXPECT_EQ(STAbilityUtil::WaitCompleted(event_, abilityName2 + abilityStateOnBackground, abilityStateCountOne), 0);
    EXPECT_EQ(STAbilityUtil::WaitCompleted(event_, abilityName2 + abilityStateOnStop, abilityStateCountOne), 0);
    EXPECT_EQ(STAbilityUtil::WaitCompleted(event_, abilityName + abilityStateOnActive, abilityStateCountTwo), 0);

    GTEST_LOG_(INFO) << "AmsAbilityVisibleTest AMS_ABILITY_VISIBLE_1400 end";
}

/*
 * @tc.number    : AMS_ABILITY_VISIBLE_1500
 * @tc.name      : Visible attribute impact on terminateAbilityCaller
 * @tc.desc      : 1.The test process starts an ability(PageA2) whose visible attribute is true
 *                 2.The ability(PageA2) starts an ability(PageA3,type:singleton) whose visible attribute is false
 *                 3.PageA2 has the same bundleName as PageA3
 *                 4.terminateCaller PageA2
 */
HWTEST_F(AmsAbilityVisibleTest, AMS_ABILITY_VISIBLE_1500, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "AmsAbilityVisibleTest AMS_ABILITY_VISIBLE_1500 start";
    std::string bundleName = bundleNameBase + "PageA";
    std::string abilityName = abilityNameBase + "PageA2";
    std::string abilityName2 = abilityNameBase + "PageA3";

    MAP_STR_STR params;
    params["targetBundle"] = bundleName;
    params["targetAbility"] = abilityName2;
    params["type"] = "Page";

    Want want = STAbilityUtil::MakeWant("device", abilityName, bundleName, params);
    STAbilityUtil::StartAbility(want, abilityMs_);
    EXPECT_EQ(STAbilityUtil::WaitCompleted(event_, abilityName + abilityStateOnStart, abilityStateCountOne), 0);
    EXPECT_EQ(STAbilityUtil::WaitCompleted(event_, abilityName + abilityStateOnActive, abilityStateCountOne), 0);
    EXPECT_EQ(STAbilityUtil::WaitCompleted(event_, abilityName2 + abilityStateOnStart, abilityStateCountOne), 0);
    EXPECT_EQ(STAbilityUtil::WaitCompleted(event_, abilityName2 + abilityStateOnActive, abilityStateCountOne), 0);
    EXPECT_EQ(STAbilityUtil::WaitCompleted(event_, abilityName + abilityStateOnInactive, abilityStateCountOne), 0);
    EXPECT_EQ(STAbilityUtil::WaitCompleted(event_, abilityName + abilityStateOnBackground, abilityStateCountOne), 0);

    ExpectAbilityNumInStack(abilityName, 1);
    ExpectAbilityNumInStack(abilityName2, 1);
    ExpectAbilityCurrentState(abilityName, AbilityState_Test::BACKGROUND, AbilityState_Test::MOVING_BACKGROUND);
    ExpectAbilityCurrentState(abilityName2, AbilityState_Test::ACTIVE, AbilityState_Test::ACTIVATING);

    int eventCode = -1;
    STAbilityUtil::PublishEvent(terminateAbilityCaller, eventCode, abilityName);
    EXPECT_EQ(STAbilityUtil::WaitCompleted(event_, abilityName2 + abilityStateOnInactive, abilityStateCountOne), 0);
    EXPECT_EQ(STAbilityUtil::WaitCompleted(event_, abilityName2 + abilityStateOnBackground, abilityStateCountOne), 0);
    EXPECT_EQ(STAbilityUtil::WaitCompleted(event_, abilityName2 + abilityStateOnStop, abilityStateCountOne), 0);
    EXPECT_EQ(STAbilityUtil::WaitCompleted(event_, abilityName + abilityStateOnActive, abilityStateCountTwo), 0);
    ExpectAbilityNumInStack(abilityName, 1);
    ExpectAbilityNumInStack(abilityName2, 0);
    ExpectAbilityCurrentState(abilityName, AbilityState_Test::ACTIVE, AbilityState_Test::ACTIVATING);

    GTEST_LOG_(INFO) << "AmsAbilityVisibleTest AMS_ABILITY_VISIBLE_1500 end";
}

/*
 * @tc.number    : AMS_ABILITY_VISIBLE_1600
 * @tc.name      : Visible attribute impact on terminateAbility
 * @tc.desc      : 1.The test process starts an ability(PageB2) whose visible attribute is true
 *                 2.The ability(PageB2) start a Service ability(Service) whose visible attribute is false
 *                 3.PageB2 has the same bundleName as Service
 *                 4.terminate Service and PageB2
 */
HWTEST_F(AmsAbilityVisibleTest, AMS_ABILITY_VISIBLE_1600, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "AmsAbilityVisibleTest AMS_ABILITY_VISIBLE_1600 start";
    std::string bundleName = bundleNameBase + "ServiceB";
    std::string abilityName = abilityNameBase + "PageB2";
    std::string bundleName2 = bundleNameBase + "ServiceB";
    std::string abilityName2 = abilityNameBase + "Service";

    MAP_STR_STR params;
    params["targetBundle"] = bundleName2;
    params["targetAbility"] = abilityName2;
    params["type"] = "Page";

    Want want = STAbilityUtil::MakeWant("device", abilityName, bundleName, params);
    STAbilityUtil::StartAbility(want, abilityMs_);
    EXPECT_EQ(STAbilityUtil::WaitCompleted(event_, abilityName + abilityStateOnStart, abilityStateCountOne), 0);
    EXPECT_EQ(STAbilityUtil::WaitCompleted(event_, abilityName + abilityStateOnActive, abilityStateCountOne), 0);
    EXPECT_EQ(STAbilityUtil::WaitCompleted(event_, abilityName2 + abilityStateOnStart, abilityStateCountOne), 0);
    EXPECT_EQ(STAbilityUtil::WaitCompleted(event_, abilityName2 + abilityStateOnCommand, abilityStateCountOne), 0);

    ExpectAbilityNumInStack(abilityName, 1);
    ExpectAbilityNumInStack(abilityName2, 1, DUMP_SERVICE);
    ExpectAbilityCurrentState(abilityName, AbilityState_Test::ACTIVE, AbilityState_Test::ACTIVATING);

    int eventCode = -1;
    STAbilityUtil::PublishEvent(terminateAbility, eventCode, abilityName2);
    EXPECT_EQ(STAbilityUtil::WaitCompleted(event_, abilityName2 + abilityStateOnBackground, abilityStateCountOne), 0);
    EXPECT_EQ(STAbilityUtil::WaitCompleted(event_, abilityName2 + abilityStateOnStop, abilityStateCountOne), 0);

    STAbilityUtil::PublishEvent(terminateAbility, eventCode, abilityName);
    EXPECT_EQ(STAbilityUtil::WaitCompleted(event_, abilityName + abilityStateOnInactive, abilityStateCountOne), 0);
    EXPECT_EQ(STAbilityUtil::WaitCompleted(event_, abilityName + abilityStateOnBackground, abilityStateCountOne), 0);
    EXPECT_EQ(STAbilityUtil::WaitCompleted(event_, abilityName + abilityStateOnStop, abilityStateCountOne), 0);
    ExpectAbilityNumInStack(abilityName, 0);

    GTEST_LOG_(INFO) << "AmsAbilityVisibleTest AMS_ABILITY_VISIBLE_1600 end";
}

/*
 * @tc.number    : AMS_ABILITY_VISIBLE_1700
 * @tc.name      : Visible attribute impact on terminateAbilityCaller
 * @tc.desc      : 1.The test process starts an ability(PageB2) whose visible attribute is true
 *                 2.The ability(PageB2) Connect a Service ability(Service) whose visible attribute is false
 *                 3.The ability(Service) start a Service ability(ServiceA1) whose visible attribute is false
 *                 3.PageB2 has the same bundleName as Service and ServiceA1
 *                 4.terminateCaller Service
 */
HWTEST_F(AmsAbilityVisibleTest, AMS_ABILITY_VISIBLE_1700, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "AmsAbilityVisibleTest AMS_ABILITY_VISIBLE_1700 start";
    std::string bundleName = bundleNameBase + "ServiceB";
    std::string abilityName = abilityNameBase + "PageB2";
    std::string bundleName2 = bundleNameBase + "ServiceB";
    std::string abilityName2 = abilityNameBase + "Service";
    std::string abilityName3 = abilityNameBase + "ServiceA1";

    MAP_STR_STR params;
    params["targetBundle"] = bundleName2 + "," + bundleName2;
    params["targetAbility"] = abilityName2 + "," + abilityName3;
    params["type"] = "Service,Page";

    Want want = STAbilityUtil::MakeWant("device", abilityName, bundleName, params);
    STAbilityUtil::StartAbility(want, abilityMs_);
    EXPECT_EQ(STAbilityUtil::WaitCompleted(event_, abilityName + abilityStateOnStart, abilityStateCountOne), 0);
    EXPECT_EQ(STAbilityUtil::WaitCompleted(event_, abilityName + abilityStateOnActive, abilityStateCountOne), 0);
    EXPECT_EQ(STAbilityUtil::WaitCompleted(event_, abilityName2 + abilityStateOnStart, abilityStateCountOne), 0);
    EXPECT_EQ(STAbilityUtil::WaitCompleted(event_, abilityName2 + abilityStateOnConnect, abilityStateCountOne), 0);
    EXPECT_EQ(STAbilityUtil::WaitCompleted(event_, abilityName3 + abilityStateOnStart, abilityStateCountOne), 0);
    EXPECT_EQ(STAbilityUtil::WaitCompleted(event_, abilityName3 + abilityStateOnCommand, abilityStateCountOne), 0);

    ExpectAbilityNumInStack(abilityName, 1);
    ExpectAbilityNumInStack(abilityName2, 1, DUMP_SERVICE);
    ExpectAbilityNumInStack(abilityName3, 1, DUMP_SERVICE);
    ExpectAbilityCurrentState(abilityName, AbilityState_Test::ACTIVE, AbilityState_Test::ACTIVATING);

    int eventCode = -1;
    STAbilityUtil::PublishEvent(terminateAbilityCaller, eventCode, abilityName2);
    EXPECT_EQ(STAbilityUtil::WaitCompleted(event_, abilityName3 + abilityStateOnBackground, abilityStateCountOne), 0);
    EXPECT_EQ(STAbilityUtil::WaitCompleted(event_, abilityName3 + abilityStateOnStop, abilityStateCountOne), 0);

    GTEST_LOG_(INFO) << "AmsAbilityVisibleTest AMS_ABILITY_VISIBLE_1700 end";
}

/*
 * @tc.number    : AMS_ABILITY_VISIBLE_1800
 * @tc.name      : Visible attribute impact on terminateAbilityResult
 * @tc.desc      : 1.The test process starts an ability(PageB2) whose visible attribute is true
 *                 2.The ability(PageB2) start a Service ability(Service) whose visible attribute is false
 *                 3.PageB2 has the same bundleName as Service
 *                 4.terminateAbilityResult Service and terminateAbility PageB2
 */
HWTEST_F(AmsAbilityVisibleTest, AMS_ABILITY_VISIBLE_1800, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "AmsAbilityVisibleTest AMS_ABILITY_VISIBLE_1800 start";
    std::string bundleName = bundleNameBase + "ServiceB";
    std::string abilityName = abilityNameBase + "PageB2";
    std::string bundleName2 = bundleNameBase + "ServiceB";
    std::string abilityName2 = abilityNameBase + "Service";

    MAP_STR_STR params;
    params["targetBundle"] = bundleName2;
    params["targetAbility"] = abilityName2;
    params["type"] = "Page";

    Want want = STAbilityUtil::MakeWant("device", abilityName, bundleName, params);
    STAbilityUtil::StartAbility(want, abilityMs_);
    EXPECT_EQ(STAbilityUtil::WaitCompleted(event_, abilityName + abilityStateOnStart, abilityStateCountOne), 0);
    EXPECT_EQ(STAbilityUtil::WaitCompleted(event_, abilityName + abilityStateOnActive, abilityStateCountOne), 0);
    EXPECT_EQ(STAbilityUtil::WaitCompleted(event_, abilityName2 + abilityStateOnStart, abilityStateCountOne), 0);
    EXPECT_EQ(STAbilityUtil::WaitCompleted(event_, abilityName2 + abilityStateOnCommand, abilityStateCountOne), 0);

    ExpectAbilityNumInStack(abilityName, 1);
    ExpectAbilityNumInStack(abilityName2, 1, DUMP_SERVICE);
    ExpectAbilityCurrentState(abilityName, AbilityState_Test::ACTIVE, AbilityState_Test::ACTIVATING);

    int eventCode = 1;
    STAbilityUtil::PublishEvent(terminateAbilityResult, eventCode, abilityName2);
    EXPECT_EQ(STAbilityUtil::WaitCompleted(event_, abilityName2 + abilityStateOnBackground, abilityStateCountOne), 0);
    EXPECT_EQ(STAbilityUtil::WaitCompleted(event_, abilityName2 + abilityStateOnStop, abilityStateCountOne), 0);

    STAbilityUtil::PublishEvent(terminateAbility, eventCode, abilityName);
    EXPECT_EQ(STAbilityUtil::WaitCompleted(event_, abilityName + abilityStateOnInactive, abilityStateCountOne), 0);
    EXPECT_EQ(STAbilityUtil::WaitCompleted(event_, abilityName + abilityStateOnBackground, abilityStateCountOne), 0);
    EXPECT_EQ(STAbilityUtil::WaitCompleted(event_, abilityName + abilityStateOnStop, abilityStateCountOne), 0);
    ExpectAbilityNumInStack(abilityName, 0);

    GTEST_LOG_(INFO) << "AmsAbilityVisibleTest AMS_ABILITY_VISIBLE_1800 end";
}

/*
 * @tc.number    : AMS_ABILITY_VISIBLE_1900
 * @tc.name      : Visible attribute impact on disconnectService
 * @tc.desc      : 1.The test process starts an ability(PageB2) whose visible attribute is true
 *                 2.The ability(PageB2) Connect a Service ability(Service) whose visible attribute is false
 *                 3.PageB2 has the same bundleName as Service
 *                 4.disconnectService Service
 */
HWTEST_F(AmsAbilityVisibleTest, AMS_ABILITY_VISIBLE_1900, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "AmsAbilityVisibleTest AMS_ABILITY_VISIBLE_1900 start";
    std::string bundleName = bundleNameBase + "ServiceB";
    std::string abilityName = abilityNameBase + "PageB2";
    std::string bundleName2 = bundleNameBase + "ServiceB";
    std::string abilityName2 = abilityNameBase + "Service";

    MAP_STR_STR params;
    params["targetBundle"] = bundleName2;
    params["targetAbility"] = abilityName2;
    params["type"] = "Service";

    Want want = STAbilityUtil::MakeWant("device", abilityName, bundleName, params);
    STAbilityUtil::StartAbility(want, abilityMs_);
    EXPECT_EQ(STAbilityUtil::WaitCompleted(event_, abilityName + abilityStateOnStart, abilityStateCountOne), 0);
    EXPECT_EQ(STAbilityUtil::WaitCompleted(event_, abilityName + abilityStateOnActive, abilityStateCountOne), 0);
    EXPECT_EQ(STAbilityUtil::WaitCompleted(event_, abilityName2 + abilityStateOnStart, abilityStateCountOne), 0);
    EXPECT_EQ(STAbilityUtil::WaitCompleted(event_, abilityName2 + abilityStateOnConnect, abilityStateCountOne), 0);

    ExpectAbilityNumInStack(abilityName, 1);
    ExpectAbilityNumInStack(abilityName2, 1, DUMP_SERVICE);
    ExpectAbilityCurrentState(abilityName, AbilityState_Test::ACTIVE, AbilityState_Test::ACTIVATING);

    int eventCode = 0;
    STAbilityUtil::PublishEvent(disconnectService, eventCode, abilityName);
    EXPECT_EQ(STAbilityUtil::WaitCompleted(event_, abilityName2 + abilityStateOnDisconnect, abilityStateCountOne), 0);

    GTEST_LOG_(INFO) << "AmsAbilityVisibleTest AMS_ABILITY_VISIBLE_1900 end";
}

/*
 * @tc.number    : AMS_ABILITY_VISIBLE_2000
 * @tc.name      : Visible attribute impact on data Release
 * @tc.desc      : 1.The test process starts an ability(PageB2) whose visible attribute is true
 *                 2.The ability(PageB2) start a data ability(Data) whose visible attribute is false
 *                 3.PageB2 has the same bundleName as Data
 *                 4.Release Data
 */
HWTEST_F(AmsAbilityVisibleTest, AMS_ABILITY_VISIBLE_2000, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "AmsAbilityVisibleTest AMS_ABILITY_VISIBLE_2000 start";
    std::string bundleName = bundleNameBase + "ServiceB";
    std::string abilityName = abilityNameBase + "PageB2";
    std::string bundleName2 = bundleNameBase + "ServiceB";
    std::string abilityName2 = abilityNameBase + "Data";

    MAP_STR_STR params;
    params["targetBundle"] = bundleName2;
    params["targetAbility"] = abilityName2;
    params["type"] = "DataRelease";

    Want want = STAbilityUtil::MakeWant("device", abilityName, bundleName, params);
    STAbilityUtil::StartAbility(want, abilityMs_);
    EXPECT_EQ(STAbilityUtil::WaitCompleted(event_, abilityName + abilityStateOnStart, abilityStateCountOne), 0);
    EXPECT_EQ(STAbilityUtil::WaitCompleted(event_, abilityName + abilityStateOnActive, abilityStateCountOne), 0);
    EXPECT_EQ(STAbilityUtil::WaitCompleted(event_, abilityName2 + abilityStateOnStart, abilityStateCountOne), 0);
    EXPECT_EQ(STAbilityUtil::WaitCompleted(event_, abilityName2 + abilityStateInsert, abilityStateCountOne), 0);

    ExpectAbilityNumInStack(abilityName, 1);
    ExpectAbilityNumInStack(abilityName2, 1, DUMP_DATA);
    ExpectAbilityCurrentState(abilityName, AbilityState_Test::ACTIVE, AbilityState_Test::ACTIVATING);

    GTEST_LOG_(INFO) << "AmsAbilityVisibleTest AMS_ABILITY_VISIBLE_2000 end";
}

/*
 * @tc.number    : AMS_ABILITY_VISIBLE_2100
 * @tc.name      : Visible attribute impact on terminateAbilityCaller
 * @tc.desc      : 1.The test process starts an ability(PageB2) whose visible attribute is true
 *                 2.The ability(PageB2) Connect a Service ability(Service) whose visible attribute is false
 *                 3.PageB2 has the same bundleName as Service
 *                 4.terminateCaller Service
 */
HWTEST_F(AmsAbilityVisibleTest, AMS_ABILITY_VISIBLE_2100, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "AmsAbilityVisibleTest AMS_ABILITY_VISIBLE_2100 start";
    std::string bundleName = bundleNameBase + "ServiceB";
    std::string abilityName = abilityNameBase + "PageB2";
    std::string bundleName2 = bundleNameBase + "ServiceB";
    std::string abilityName2 = abilityNameBase + "Service";

    MAP_STR_STR params;
    params["targetBundle"] = bundleName2;
    params["targetAbility"] = abilityName2;
    params["type"] = "Page";

    Want want = STAbilityUtil::MakeWant("device", abilityName, bundleName, params);
    STAbilityUtil::StartAbility(want, abilityMs_);
    EXPECT_EQ(STAbilityUtil::WaitCompleted(event_, abilityName + abilityStateOnStart, abilityStateCountOne), 0);
    EXPECT_EQ(STAbilityUtil::WaitCompleted(event_, abilityName + abilityStateOnActive, abilityStateCountOne), 0);
    EXPECT_EQ(STAbilityUtil::WaitCompleted(event_, abilityName2 + abilityStateOnStart, abilityStateCountOne), 0);
    EXPECT_EQ(STAbilityUtil::WaitCompleted(event_, abilityName2 + abilityStateOnCommand, abilityStateCountOne), 0);

    ExpectAbilityNumInStack(abilityName, 1);
    ExpectAbilityNumInStack(abilityName2, 1, DUMP_SERVICE);
    ExpectAbilityCurrentState(abilityName, AbilityState_Test::ACTIVE, AbilityState_Test::ACTIVATING);

    int eventCode = -1;
    STAbilityUtil::PublishEvent(terminateAbilityCaller, eventCode, abilityName);
    EXPECT_EQ(STAbilityUtil::WaitCompleted(event_, abilityName2 + abilityStateOnBackground, abilityStateCountOne), 0);
    EXPECT_EQ(STAbilityUtil::WaitCompleted(event_, abilityName2 + abilityStateOnStop, abilityStateCountOne), 0);

    GTEST_LOG_(INFO) << "AmsAbilityVisibleTest AMS_ABILITY_VISIBLE_2100 end";
}

/*
 * @tc.number    : AMS_ABILITY_VISIBLE_2200
 * @tc.name      : Visible attribute impact on wantagent
 * @tc.desc      : 1.The test process starts an ability(PageA2) whose visible attribute is true
 *                 2.The ability(PageA2) get the wantagent to starts an ability(PageA1) whose visible attribute is false
 *                 3.PageA2 has the same bundleName as PageA1
 *                 4.trigger wantagent
 */
HWTEST_F(AmsAbilityVisibleTest, AMS_ABILITY_VISIBLE_2200, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "AmsAbilityVisibleTest AMS_ABILITY_VISIBLE_2200 start";
    std::string bundleName = bundleNameBase + "PageA";
    std::string abilityName = abilityNameBase + "PageA2";
    std::string abilityName2 = abilityNameBase + "PageA1";

    MAP_STR_STR params;
    params["targetBundle"] = bundleName;
    params["targetAbility"] = abilityName2;
    params["type"] = "TriggerWantAgentPageAbility";

    Want want = STAbilityUtil::MakeWant("device", abilityName, bundleName, params);
    STAbilityUtil::StartAbility(want, abilityMs_);
    EXPECT_EQ(STAbilityUtil::WaitCompleted(event_, abilityName + abilityStateOnStart, abilityStateCountOne), 0);
    EXPECT_EQ(STAbilityUtil::WaitCompleted(event_, abilityName + abilityStateOnActive, abilityStateCountOne), 0);
    EXPECT_EQ(STAbilityUtil::WaitCompleted(event_, abilityName2 + abilityStateOnStart, abilityStateCountOne), 0);
    int eventCode = 0;
    EXPECT_EQ(STAbilityUtil::WaitCompleted(event_, triggerWantAgentState, eventCode), 0);
    EXPECT_EQ(STAbilityUtil::WaitCompleted(event_, abilityName2 + abilityStateOnActive, abilityStateCountOne), 0);
    EXPECT_EQ(STAbilityUtil::WaitCompleted(event_, abilityName + abilityStateOnInactive, abilityStateCountOne), 0);
    EXPECT_EQ(STAbilityUtil::WaitCompleted(event_, abilityName + abilityStateOnBackground, abilityStateCountOne), 0);

    ExpectAbilityNumInStack(abilityName, 1);
    ExpectAbilityCurrentState(abilityName, AbilityState_Test::BACKGROUND, AbilityState_Test::MOVING_BACKGROUND);
    ExpectAbilityCurrentState(abilityName2, AbilityState_Test::ACTIVE, AbilityState_Test::ACTIVATING);

    GTEST_LOG_(INFO) << "AmsAbilityVisibleTest AMS_ABILITY_VISIBLE_2200 end";
}

/*
 * @tc.number    : AMS_ABILITY_VISIBLE_2300
 * @tc.name      : Visible attribute impact on wantagent
 * @tc.desc      : 1.The test process starts an ability(PageA2) whose visible attribute is true
 *                 2.The ability(PageA2) get the wantagent to starts an ability(PageB1) whose visible attribute is false
 *                 3.PageA2 has the different bundleName as PageB1
 *                 4.trigger wantagent
 */
HWTEST_F(AmsAbilityVisibleTest, AMS_ABILITY_VISIBLE_2300, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "AmsAbilityVisibleTest AMS_ABILITY_VISIBLE_2300 start";
    std::string bundleName = bundleNameBase + "PageA";
    std::string abilityName = abilityNameBase + "PageA2";
    std::string bundleName2 = bundleNameBase + "ServiceB";
    std::string abilityName2 = abilityNameBase + "PageB1";

    MAP_STR_STR params;
    params["targetBundle"] = bundleName2;
    params["targetAbility"] = abilityName2;
    params["type"] = "TriggerWantAgentPageAbility";

    Want want = STAbilityUtil::MakeWant("device", abilityName, bundleName, params);
    STAbilityUtil::StartAbility(want, abilityMs_);
    EXPECT_EQ(STAbilityUtil::WaitCompleted(event_, abilityName + abilityStateOnStart, abilityStateCountOne), 0);
    int eventCode = 0;
    EXPECT_EQ(STAbilityUtil::WaitCompleted(event_, triggerWantAgentState, eventCode), 0);
    EXPECT_EQ(STAbilityUtil::WaitCompleted(event_, abilityName + abilityStateOnActive, abilityStateCountOne), 0);

    ExpectAbilityNumInStack(abilityName, 1);
    ExpectAbilityNumInStack(abilityName2, 0);
    ExpectAbilityCurrentState(abilityName, AbilityState_Test::ACTIVE, AbilityState_Test::ACTIVATING);

    GTEST_LOG_(INFO) << "AmsAbilityVisibleTest AMS_ABILITY_VISIBLE_2300 end";
}

/*
 * @tc.number    : AMS_ABILITY_VISIBLE_2400
 * @tc.name      : Visible attribute impact on wantagent
 * @tc.desc      : 1.The test process starts an ability(PageA2) whose visible attribute is true
 *                 2.The ability(PageA2) get the wantagent to starts an ability(PageB2) whose visible attribute is true
 *                 3.PageA2 has the different bundleName as PageB2
 *                 4.trigger wantagent
 */
HWTEST_F(AmsAbilityVisibleTest, AMS_ABILITY_VISIBLE_2400, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "AmsAbilityVisibleTest AMS_ABILITY_VISIBLE_2400 start";
    std::string bundleName = bundleNameBase + "PageA";
    std::string abilityName = abilityNameBase + "PageA2";
    std::string bundleName2 = bundleNameBase + "ServiceB";
    std::string abilityName2 = abilityNameBase + "PageB2";

    MAP_STR_STR params;
    params["targetBundle"] = bundleName2;
    params["targetAbility"] = abilityName2;
    params["type"] = "TriggerWantAgentPageAbility";

    Want want = STAbilityUtil::MakeWant("device", abilityName, bundleName, params);
    STAbilityUtil::StartAbility(want, abilityMs_);
    EXPECT_EQ(STAbilityUtil::WaitCompleted(event_, abilityName + abilityStateOnStart, abilityStateCountOne), 0);
    int eventCode = 0;
    EXPECT_EQ(STAbilityUtil::WaitCompleted(event_, triggerWantAgentState, eventCode), 0);
    EXPECT_EQ(STAbilityUtil::WaitCompleted(event_, abilityName + abilityStateOnActive, abilityStateCountOne), 0);
    EXPECT_EQ(STAbilityUtil::WaitCompleted(event_, abilityName2 + abilityStateOnStart, abilityStateCountOne), 0);
    EXPECT_EQ(STAbilityUtil::WaitCompleted(event_, abilityName2 + abilityStateOnActive, abilityStateCountOne), 0);
    EXPECT_EQ(STAbilityUtil::WaitCompleted(event_, abilityName + abilityStateOnBackground, abilityStateCountOne), 0);

    ShowDump();
    ExpectAbilityNumInStack(abilityName, 1);
    ExpectAbilityNumInStack(abilityName2, 1);
    ExpectAbilityCurrentState(abilityName2, AbilityState_Test::ACTIVE, AbilityState_Test::ACTIVATING);
    GTEST_LOG_(INFO) << "AmsAbilityVisibleTest AMS_ABILITY_VISIBLE_2400 end";
}

/*
 * @tc.number    : AMS_ABILITY_VISIBLE_2500
 * @tc.name      : Visible attribute impact on wantagent
 * @tc.desc      : 1.The test process starts an ability(PageA2) whose visible attribute is true
 *                 2.The ability(PageA2) get the wantagent to starts an ability whose visible attribute is false
 *                 3.PageA2 has the different bundleName as Service
 *                 4.trigger wantagent
 */
HWTEST_F(AmsAbilityVisibleTest, AMS_ABILITY_VISIBLE_2500, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "AmsAbilityVisibleTest AMS_ABILITY_VISIBLE_2500 start";
    std::string bundleName = bundleNameBase + "PageA";
    std::string abilityName = abilityNameBase + "PageA2";
    std::string bundleName2 = bundleNameBase + "ServiceB";
    std::string abilityName2 = abilityNameBase + "Service";

    MAP_STR_STR params;
    params["targetBundle"] = bundleName2;
    params["targetAbility"] = abilityName2;
    params["type"] = "TriggerWantAgentServiceAbility";

    Want want = STAbilityUtil::MakeWant("device", abilityName, bundleName, params);
    STAbilityUtil::StartAbility(want, abilityMs_);
    EXPECT_EQ(STAbilityUtil::WaitCompleted(event_, abilityName + abilityStateOnStart, abilityStateCountOne), 0);
    int eventCode = 0;
    EXPECT_EQ(STAbilityUtil::WaitCompleted(event_, triggerWantAgentState, eventCode), 0);
    EXPECT_EQ(STAbilityUtil::WaitCompleted(event_, abilityName + abilityStateOnActive, abilityStateCountOne), 0);

    ExpectAbilityNumInStack(abilityName, 1);
    ExpectAbilityNumInStack(abilityName2, 0, DUMP_SERVICE);
    ExpectAbilityCurrentState(abilityName, AbilityState_Test::ACTIVE, AbilityState_Test::ACTIVATING);
    GTEST_LOG_(INFO) << "AmsAbilityVisibleTest AMS_ABILITY_VISIBLE_2500 end";
}

/*
 * @tc.number    : AMS_ABILITY_VISIBLE_2600
 * @tc.name      : Visible attribute impact on wantagent
 * @tc.desc      : 1.The test process starts an ability(PageB2) whose visible attribute is true
 *                 2.The ability(PageB2) get the wantagent to starts an ability whose visible attribute is false
 *                 3.PageB2 has the same bundleName as Service
 *                 4.trigger wantagent
 */
HWTEST_F(AmsAbilityVisibleTest, AMS_ABILITY_VISIBLE_2600, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "AmsAbilityVisibleTest AMS_ABILITY_VISIBLE_2600 start";
    std::string bundleName = bundleNameBase + "ServiceB";
    std::string abilityName = abilityNameBase + "PageB2";
    std::string bundleName2 = bundleNameBase + "ServiceB";
    std::string abilityName2 = abilityNameBase + "Service";

    MAP_STR_STR params;
    params["targetBundle"] = bundleName2;
    params["targetAbility"] = abilityName2;
    params["type"] = "TriggerWantAgentServiceAbility";

    Want want = STAbilityUtil::MakeWant("device", abilityName, bundleName, params);
    STAbilityUtil::StartAbility(want, abilityMs_);
    EXPECT_EQ(STAbilityUtil::WaitCompleted(event_, abilityName + abilityStateOnStart, abilityStateCountOne), 0);
    int eventCode = 0;
    EXPECT_EQ(STAbilityUtil::WaitCompleted(event_, triggerWantAgentState, eventCode), 0);
    EXPECT_EQ(STAbilityUtil::WaitCompleted(event_, abilityName + abilityStateOnActive, abilityStateCountOne), 0);
    EXPECT_EQ(STAbilityUtil::WaitCompleted(event_, abilityName2 + abilityStateOnStart, abilityStateCountOne), 0);
    EXPECT_EQ(STAbilityUtil::WaitCompleted(event_, abilityName2 + abilityStateOnCommand, abilityStateCountOne), 0);

    ExpectAbilityNumInStack(abilityName, 1);
    ExpectAbilityNumInStack(abilityName2, 1, DUMP_SERVICE);
    ExpectAbilityCurrentState(abilityName, AbilityState_Test::ACTIVE, AbilityState_Test::ACTIVATING);
    GTEST_LOG_(INFO) << "AmsAbilityVisibleTest AMS_ABILITY_VISIBLE_2600 end";
}