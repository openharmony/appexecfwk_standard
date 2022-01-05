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
#include <chrono>
#include <condition_variable>
#include <cstdio>
#include <gtest/gtest.h>
#include <mutex>
#include <queue>
#include <set>
#include <thread>

#include "ability_append_test_info.h"
#include "ability_lifecycle.h"
#include "ability_lifecycle_executor.h"
#include "ability_manager_errors.h"
#include "ability_manager_service.h"
#include "app_mgr_service.h"
#include "common_event.h"
#include "common_event_manager.h"
#include "event.h"
#include "hilog_wrapper.h"
#include "module_test_dump_util.h"
#include "sa_mgr_client.h"
#include "semaphore_ex.h"
#include "skills.h"
#include "stoperator.h"
#include "system_ability_definition.h"
#include "st_ability_util.h"
#include "uri.h"
namespace {
using namespace OHOS;
using namespace OHOS::AAFwk;
using namespace OHOS::AppExecFwk;
using namespace OHOS::EventFwk;
using namespace OHOS::MTUtil;
using namespace OHOS::STtools;
using namespace OHOS::STABUtil;
using namespace testing::ext;

using MAP_STR_STR = std::map<std::string, std::string>;
namespace {
static const string KIT_BUNDLE_NAME = "com.ohos.amsst.AppAppend";
static const string KIT_HAP_NAME = "amsAbilityAppendTest";
static const string FIRST_ABILITY_NAME = "MainAbility";
static const string SECOND_ABILITY_NAME = "SecondAbility";
static const string THIRD_ABILITY_NAME = "MainAbility";
static constexpr int WAIT_TIME = 1;
constexpr int START_DELAY = 1000;
static string g_eventMessage = "";
}
class AmsAbilityAppendTest : public testing::Test {
public:
    static void SetUpTestCase(void);
    static void TearDownTestCase(void);
    void SetUp();
    void TearDown();

    static void Reinstall(const std::string &hapName, const std::string &bundleName);
    void ResetSystem() const;
    static bool SubscribeEvent();
    static int TestWaitCompleted(Event &event, const std::string &eventName, const int code, const int timeout = 10);
    static void TestCompleted(Event &event, const std::string &eventName, const int code);

    class AppEventSubscriber : public CommonEventSubscriber {
    public:
        explicit AppEventSubscriber(const CommonEventSubscribeInfo &sp) : CommonEventSubscriber(sp){};
        virtual void OnReceiveEvent(const CommonEventData &data) override;
        ~AppEventSubscriber(){};
    };

    static sptr<IAbilityManager> abilityMgrService;
    static Event event;
};

Event AmsAbilityAppendTest::event = Event();
sptr<IAbilityManager> AmsAbilityAppendTest::abilityMgrService = nullptr;

void AmsAbilityAppendTest::AppEventSubscriber::OnReceiveEvent(const CommonEventData &data)
{
    GTEST_LOG_(INFO) << "OnReceiveEvent: event=" << data.GetWant().GetAction();
    GTEST_LOG_(INFO) << "OnReceiveEvent: data=" << data.GetData();
    GTEST_LOG_(INFO) << "OnReceiveEvent: code=" << data.GetCode();
    if (data.GetWant().GetAction() == g_EVENT_RESP_FIRST_LIFECYCLE ||
        data.GetWant().GetAction() == g_EVENT_RESP_FIRSTB_LIFECYCLE ||
        data.GetWant().GetAction() == g_EVENT_RESP_SECOND_LIFECYCLE) {
        TestCompleted(event, data.GetData(), data.GetCode());
    } else if (data.GetWant().GetAction() == g_EVENT_RESP_FIRST || data.GetWant().GetAction() == g_EVENT_RESP_FIRSTB ||
               data.GetWant().GetAction() == g_EVENT_RESP_SECOND) {
        g_eventMessage = data.GetData();
        TestCompleted(event, data.GetWant().GetAction(), data.GetCode());
        GTEST_LOG_(INFO) << "OnReceiveEvent: g_eventMessage=" << data.GetData();
    }
}

int AmsAbilityAppendTest::TestWaitCompleted(
    Event &event, const std::string &eventName, const int code, const int timeout)
{
    GTEST_LOG_(INFO) << "TestWaitCompleted : " << eventName << " " << code;
    return STAbilityUtil::WaitCompleted(event, eventName, code, timeout);
}

void AmsAbilityAppendTest::TestCompleted(Event &event, const std::string &eventName, const int code)
{
    GTEST_LOG_(INFO) << "TestCompleted : " << eventName << " " << code;
    return STAbilityUtil::Completed(event, eventName, code);
}

void AmsAbilityAppendTest::SetUpTestCase(void)
{
    if (!SubscribeEvent()) {
        GTEST_LOG_(INFO) << "SubscribeEvent error";
    }
}

void AmsAbilityAppendTest::TearDownTestCase(void)
{}

static int CODE_ = 0;
void AmsAbilityAppendTest::SetUp(void)
{
    ResetSystem();
    STAbilityUtil::Install(KIT_HAP_NAME + "A");
    STAbilityUtil::Install(KIT_HAP_NAME + "B");
    STAbilityUtil::CleanMsg(event);

    CODE_++;
}

void AmsAbilityAppendTest::TearDown(void)
{
    STAbilityUtil::Uninstall(KIT_BUNDLE_NAME + "A");
    STAbilityUtil::Uninstall(KIT_BUNDLE_NAME + "B");
    STAbilityUtil::CleanMsg(event);
}

bool AmsAbilityAppendTest::SubscribeEvent()
{
    std::vector<std::string> eventList = {
        g_EVENT_RESP_FIRST_LIFECYCLE,
        g_EVENT_RESP_SECOND_LIFECYCLE,
        g_EVENT_RESP_FIRSTB_LIFECYCLE,
        g_EVENT_RESP_FIRST,
        g_EVENT_RESP_SECOND,
        g_EVENT_RESP_FIRSTB
    };
    MatchingSkills matchingSkills;
    for (const auto &e : eventList) {
        matchingSkills.AddEvent(e);
    }
    CommonEventSubscribeInfo subscribeInfo(matchingSkills);
    subscribeInfo.SetPriority(1);
    auto subscriber = std::make_shared<AppEventSubscriber>(subscribeInfo);
    return CommonEventManager::SubscribeCommonEvent(subscriber);
}

void AmsAbilityAppendTest::Reinstall(const std::string &hapName, const std::string &bundleName)
{
    STAbilityUtil::Uninstall(bundleName);
    STAbilityUtil::Install(hapName);
}

void AmsAbilityAppendTest::ResetSystem() const
{
    GTEST_LOG_(INFO) << "ResetSystem";
}

/**
 * @tc.number    : AMS_Ability_Append_00100
 * @tc.name      : test OnSetCaller in ability.h
 * @tc.desc      : Verify that the result of OnSetCaller function is correct.
 */
HWTEST_F(AmsAbilityAppendTest, AMS_Ability_Append_00100, Function | MediumTest | Level1)
{
    GTEST_LOG_(INFO) << "AmsAbilityAppendTest AMS_Ability_Append_00100 start";
    MAP_STR_STR params;
    Want want = STAbilityUtil::MakeWant("device", FIRST_ABILITY_NAME, KIT_BUNDLE_NAME + "A", params);
    // start first ability
    ErrCode eCode = STAbilityUtil::StartAbility(want, abilityMgrService, WAIT_TIME);
    GTEST_LOG_(INFO) << eCode;

    EXPECT_EQ(TestWaitCompleted(event, "onStart", MAIN_ABILITY_A_CODE, START_DELAY), 0);
    EXPECT_EQ(TestWaitCompleted(event, "OnActive", MAIN_ABILITY_A_CODE, START_DELAY), 0);

    STAbilityUtil::CleanMsg(event);
    STAbilityUtil::PublishEvent(
        g_EVENT_REQU_FIRST, CODE_, "Ability_" + std::to_string((int)AppendApi::OnSetCaller) + "_0");
    EXPECT_EQ(TestWaitCompleted(event, g_EVENT_RESP_FIRST, CODE_, START_DELAY), 0);
    string appInfo = g_eventMessage;

    GTEST_LOG_(INFO) << appInfo;
    EXPECT_EQ(appInfo, "1");
    GTEST_LOG_(INFO) << "AmsAbilityAppendTest AMS_Ability_Append_00100 end";
}

/**
 * @tc.number    : AMS_Ability_Append_00200
 * @tc.name      : test TerminateAndRemoveMisson in ability.h
 * @tc.desc      : Verify that the result of TerminateAndRemoveMisson function is correct.
 */
HWTEST_F(AmsAbilityAppendTest, AMS_Ability_Append_00200, Function | MediumTest | Level1)
{
    GTEST_LOG_(INFO) << "AmsAbilityAppendTest AMS_Ability_Append_00200 start";
    MAP_STR_STR params;
    Want wantFirst = STAbilityUtil::MakeWant("device", FIRST_ABILITY_NAME, KIT_BUNDLE_NAME + "A", params);
    // start first ability
    ErrCode eCode = STAbilityUtil::StartAbility(wantFirst, abilityMgrService, WAIT_TIME);
    GTEST_LOG_(INFO) << eCode;
    EXPECT_EQ(TestWaitCompleted(event, "onStart", MAIN_ABILITY_A_CODE, START_DELAY), 0);
    EXPECT_EQ(TestWaitCompleted(event, "OnActive", MAIN_ABILITY_A_CODE, START_DELAY), 0);

    auto abilityMs = STAbilityUtil::GetAbilityManagerService();
    Want wantEntity;
    wantEntity.AddEntity(Want::FLAG_HOME_INTENT_FROM_SYSTEM);
    STAbilityUtil::StartAbility(wantEntity, abilityMs);

    sleep(5);
    Want wantSecond = STAbilityUtil::MakeWant("device", THIRD_ABILITY_NAME, KIT_BUNDLE_NAME + "B", params);
    // start first ability
    eCode = STAbilityUtil::StartAbility(wantSecond, abilityMgrService, WAIT_TIME);
    GTEST_LOG_(INFO) << eCode;
    EXPECT_EQ(TestWaitCompleted(event, "onStart", MAIN_ABILITY_B_CODE, START_DELAY), 0);
    EXPECT_EQ(TestWaitCompleted(event, "OnActive", MAIN_ABILITY_B_CODE, START_DELAY), 0);

    EXPECT_EQ(TestWaitCompleted(event, "OnBackground", MAIN_ABILITY_A_CODE, START_DELAY), 0);
    STAbilityUtil::PublishEvent(
        g_EVENT_REQU_FIRST, CODE_, "Ability_" + std::to_string((int)AppendApi::TerminateAndRemoveMisson) + "_0");
    EXPECT_EQ(TestWaitCompleted(event, g_EVENT_RESP_FIRST, CODE_, START_DELAY), 0);
    string appInfo = g_eventMessage;
    GTEST_LOG_(INFO) << appInfo;
    EXPECT_EQ(appInfo, "1");
    STAbilityUtil::CleanMsg(event);
    GTEST_LOG_(INFO) << "AmsAbilityAppendTest AMS_Ability_Append_00200 end";
}

/**
 * @tc.number    : AMS_Ability_Append_00300
 * @tc.name      : test TerminateAbilityResult in ability.h
 * @tc.desc      : Verify that the result of TerminateAbilityResult function is correct.
 */
HWTEST_F(AmsAbilityAppendTest, AMS_Ability_Append_00300, Function | MediumTest | Level1)
{
    GTEST_LOG_(INFO) << "AmsAbilityAppendTest AMS_Ability_Append_00300 start";
    MAP_STR_STR params;
    Want want = STAbilityUtil::MakeWant("device", SECOND_ABILITY_NAME, KIT_BUNDLE_NAME + "A", params);
    // start first ability
    ErrCode eCode = STAbilityUtil::StartAbility(want, abilityMgrService, WAIT_TIME);
    GTEST_LOG_(INFO) << eCode;

    EXPECT_EQ(TestWaitCompleted(event, "onStart", SECOND_ABILITY_A_CODE, START_DELAY), 0);
    EXPECT_EQ(TestWaitCompleted(event, "OnActive", SECOND_ABILITY_A_CODE, START_DELAY), 0);

    STAbilityUtil::CleanMsg(event);
    STAbilityUtil::PublishEvent(
        g_EVENT_REQU_SECOND, CODE_, "Ability_" + std::to_string((int)AppendApi::TerminateAbilityResult) + "_0");
    EXPECT_EQ(TestWaitCompleted(event, g_EVENT_RESP_SECOND, CODE_, START_DELAY), 0);
    string appInfo = g_eventMessage;

    GTEST_LOG_(INFO) << appInfo;
    EXPECT_EQ(appInfo, "1");
    EXPECT_EQ(TestWaitCompleted(event, "OnStop", SECOND_ABILITY_A_CODE, START_DELAY), 0);
    GTEST_LOG_(INFO) << "AmsAbilityAppendTest AMS_Ability_Append_00300 end";
}

/**
 * @tc.number    : AMS_Ability_Append_00400
 * @tc.name      : test GetDispalyOrientation in ability.h
 * @tc.desc      : Verify that the result of GetDispalyOrientation function is correct.
 */
HWTEST_F(AmsAbilityAppendTest, AMS_Ability_Append_00400, Function | MediumTest | Level1)
{
    GTEST_LOG_(INFO) << "AmsAbilityAppendTest AMS_Ability_Append_00400 start";
    MAP_STR_STR params;
    Want want = STAbilityUtil::MakeWant("device", FIRST_ABILITY_NAME, KIT_BUNDLE_NAME + "A", params);
    // start first ability
    ErrCode eCode = STAbilityUtil::StartAbility(want, abilityMgrService, WAIT_TIME);
    GTEST_LOG_(INFO) << eCode;

    EXPECT_EQ(TestWaitCompleted(event, "onStart", MAIN_ABILITY_A_CODE), 0);
    EXPECT_EQ(TestWaitCompleted(event, "OnActive", MAIN_ABILITY_A_CODE), 0);

    STAbilityUtil::CleanMsg(event);
    STAbilityUtil::PublishEvent(
        g_EVENT_REQU_FIRST, CODE_, "Ability_" + std::to_string((int)AppendApi::GetDispalyOrientation) + "_0");
    EXPECT_EQ(TestWaitCompleted(event, g_EVENT_RESP_FIRST, CODE_), 0);
    string appInfo = g_eventMessage;

    GTEST_LOG_(INFO) << appInfo;
    EXPECT_EQ(appInfo, "1");
    GTEST_LOG_(INFO) << "AmsAbilityAppendTest AMS_Ability_Append_00400 end";
}

/**
 * @tc.number    : AMS_Ability_Append_00500
 * @tc.name      : test GetPreferencesDir in ability.h
 * @tc.desc      : Verify that the result of GetPreferencesDir function is correct.
 */
HWTEST_F(AmsAbilityAppendTest, AMS_Ability_Append_00500, Function | MediumTest | Level1)
{
    GTEST_LOG_(INFO) << "AmsAbilityAppendTest AMS_Ability_Append_00500 start";
    MAP_STR_STR params;
    Want want = STAbilityUtil::MakeWant("device", FIRST_ABILITY_NAME, KIT_BUNDLE_NAME + "A", params);
    // start first ability
    ErrCode eCode = STAbilityUtil::StartAbility(want, abilityMgrService, WAIT_TIME);
    GTEST_LOG_(INFO) << eCode;

    EXPECT_EQ(TestWaitCompleted(event, "onStart", MAIN_ABILITY_A_CODE, START_DELAY), 0);
    EXPECT_EQ(TestWaitCompleted(event, "OnActive", MAIN_ABILITY_A_CODE, START_DELAY), 0);

    STAbilityUtil::CleanMsg(event);
    STAbilityUtil::PublishEvent(
        g_EVENT_REQU_FIRST, CODE_, "Ability_" + std::to_string((int)AppendApi::GetPreferencesDir) + "_0");
    EXPECT_EQ(TestWaitCompleted(event, g_EVENT_RESP_FIRST, CODE_, START_DELAY), 0);
    string appInfo = g_eventMessage;

    GTEST_LOG_(INFO) << appInfo;
    EXPECT_EQ(appInfo, "0");
    GTEST_LOG_(INFO) << "AmsAbilityAppendTest AMS_Ability_Append_00500 end";
}

/**
 * @tc.number    : AMS_Ability_Append_00600
 * @tc.name      : test StartAbilities in ability.h
 * @tc.desc      : Verify that the result of StartAbilities function is correct.
 */
HWTEST_F(AmsAbilityAppendTest, AMS_Ability_Append_00600, Function | MediumTest | Level1)
{
    GTEST_LOG_(INFO) << "AmsAbilityAppendTest AMS_Ability_Append_00600 start";
    MAP_STR_STR params;
    Want want = STAbilityUtil::MakeWant("device", FIRST_ABILITY_NAME, KIT_BUNDLE_NAME + "A", params);
    // start first ability
    ErrCode eCode = STAbilityUtil::StartAbility(want, abilityMgrService, WAIT_TIME);
    GTEST_LOG_(INFO) << eCode;

    EXPECT_EQ(TestWaitCompleted(event, "onStart", MAIN_ABILITY_A_CODE), 0);
    EXPECT_EQ(TestWaitCompleted(event, "OnActive", MAIN_ABILITY_A_CODE), 0);

    STAbilityUtil::CleanMsg(event);
    STAbilityUtil::PublishEvent(
        g_EVENT_REQU_FIRST, CODE_, "Ability_" + std::to_string((int)AppendApi::StartAbilities) + "_0");
    EXPECT_EQ(TestWaitCompleted(event, g_EVENT_RESP_FIRST, CODE_), 0);
    string appInfo = g_eventMessage;
    GTEST_LOG_(INFO) << appInfo;
    EXPECT_EQ(appInfo, "1");
    EXPECT_EQ(TestWaitCompleted(event, "onStart", SECOND_ABILITY_A_CODE), 0);
    EXPECT_EQ(TestWaitCompleted(event, "OnActive", SECOND_ABILITY_A_CODE), 0);
    EXPECT_EQ(TestWaitCompleted(event, "onStart", MAIN_ABILITY_B_CODE), 0);
    EXPECT_EQ(TestWaitCompleted(event, "OnActive", MAIN_ABILITY_B_CODE), 0);
    GTEST_LOG_(INFO) << "AmsAbilityAppendTest AMS_Ability_Append_00600 end";
}

/**
 * @tc.number    : AMS_Ability_Append_00700
 * @tc.name      : test GetColorMode in ability.h
 * @tc.desc      : Verify that the result of GetColorMode function is correct.
 */
HWTEST_F(AmsAbilityAppendTest, AMS_Ability_Append_00700, Function | MediumTest | Level1)
{
    GTEST_LOG_(INFO) << "AmsAbilityAppendTest AMS_Ability_Append_00700 start";
    MAP_STR_STR params;
    Want want = STAbilityUtil::MakeWant("device", FIRST_ABILITY_NAME, KIT_BUNDLE_NAME + "A", params);
    // start first ability
    ErrCode eCode = STAbilityUtil::StartAbility(want, abilityMgrService, WAIT_TIME);
    GTEST_LOG_(INFO) << eCode;

    EXPECT_EQ(TestWaitCompleted(event, "onStart", MAIN_ABILITY_A_CODE), 0);
    EXPECT_EQ(TestWaitCompleted(event, "OnActive", MAIN_ABILITY_A_CODE), 0);

    STAbilityUtil::CleanMsg(event);
    STAbilityUtil::PublishEvent(
        g_EVENT_REQU_FIRST, CODE_, "Ability_" + std::to_string((int)AppendApi::GetColorMode) + "_0");
    EXPECT_EQ(TestWaitCompleted(event, g_EVENT_RESP_FIRST, CODE_), 0);
    string appInfo = g_eventMessage;

    GTEST_LOG_(INFO) << appInfo;
    EXPECT_EQ(appInfo, "1");
    GTEST_LOG_(INFO) << "AmsAbilityAppendTest AMS_Ability_Append_00700 end";
}

/**
 * @tc.number    : AMS_Ability_Append_00800
 * @tc.name      : test SetColorMode in ability.h
 * @tc.desc      : Verify that the result of SetColorMode function is correct.
 */
HWTEST_F(AmsAbilityAppendTest, AMS_Ability_Append_00800, Function | MediumTest | Level1)
{
    GTEST_LOG_(INFO) << "AmsAbilityAppendTest AMS_Ability_Append_00800 start";
    MAP_STR_STR params;
    Want want = STAbilityUtil::MakeWant("device", FIRST_ABILITY_NAME, KIT_BUNDLE_NAME + "A", params);
    // start first ability
    ErrCode eCode = STAbilityUtil::StartAbility(want, abilityMgrService, WAIT_TIME);
    GTEST_LOG_(INFO) << eCode;

    EXPECT_EQ(TestWaitCompleted(event, "onStart", MAIN_ABILITY_A_CODE), 0);
    EXPECT_EQ(TestWaitCompleted(event, "OnActive", MAIN_ABILITY_A_CODE), 0);

    STAbilityUtil::CleanMsg(event);
    STAbilityUtil::PublishEvent(
        g_EVENT_REQU_FIRST, CODE_, "Ability_" + std::to_string((int)AppendApi::SetColorMode) + "_0");
    EXPECT_EQ(TestWaitCompleted(event, g_EVENT_RESP_FIRST, CODE_), 0);
    string appInfo = g_eventMessage;

    GTEST_LOG_(INFO) << appInfo;
    EXPECT_EQ(appInfo, "1");
    GTEST_LOG_(INFO) << "AmsAbilityAppendTest AMS_Ability_Append_00800 end";
}

/**
 * @tc.number    : AMS_Ability_Append_00900
 * @tc.name      : test IsFirstInMission in ability.h
 * @tc.desc      : Verify that the result of IsFirstInMission function is correct.
 */
HWTEST_F(AmsAbilityAppendTest, AMS_Ability_Append_00900, Function | MediumTest | Level1)
{
    GTEST_LOG_(INFO) << "AmsAbilityAppendTest AMS_Ability_Append_00900 start";
    MAP_STR_STR params;
    Want want = STAbilityUtil::MakeWant("device", FIRST_ABILITY_NAME, KIT_BUNDLE_NAME + "A", params);
    // start first ability
    ErrCode eCode = STAbilityUtil::StartAbility(want, abilityMgrService, WAIT_TIME);
    GTEST_LOG_(INFO) << eCode;

    EXPECT_EQ(TestWaitCompleted(event, "onStart", MAIN_ABILITY_A_CODE), 0);
    EXPECT_EQ(TestWaitCompleted(event, "OnActive", MAIN_ABILITY_A_CODE), 0);

    STAbilityUtil::CleanMsg(event);
    STAbilityUtil::PublishEvent(
        g_EVENT_REQU_FIRST, CODE_, "Ability_" + std::to_string((int)AppendApi::IsFirstInMission) + "_0");
    EXPECT_EQ(TestWaitCompleted(event, g_EVENT_RESP_FIRST, CODE_), 0);
    string appInfo = g_eventMessage;

    GTEST_LOG_(INFO) << appInfo;
    EXPECT_EQ(appInfo, "1");
    GTEST_LOG_(INFO) << "AmsAbilityAppendTest AMS_Ability_Append_00900 end";
}
}  // namespace