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
// #include <cstdio>
// #include <condition_variable>
#include <gtest/gtest.h>
//#include <mutex>
//#include <queue>
//#include <set>
//#include <thread>

#include "ability_append_test_info.h"
// #include "ability_lifecycle.h"
// #include "ability_lifecycle_executor.h"
// #include "ability_manager_errors.h"
// #include "ability_manager_service.h"
// #include "app_mgr_service.h"
#include "common_event.h"
#include "common_event_manager.h"
//#include "event.h"
#include "hilog_wrapper.h"
// #include "module_test_dump_util.h"
// #include "sa_mgr_client.h"
// #include "semaphore_ex.h"
// #include "skills.h"
#include "stoperator.h"
//#include "system_ability_definition.h"
#include "system_test_ability_util.h"
//#include "uri.h"
namespace {
using namespace OHOS;
using namespace OHOS::AAFwk;
using namespace OHOS::AppExecFwk;
using namespace OHOS::EventFwk;
//using namespace OHOS::MTUtil;
using namespace OHOS::STtools;
using namespace OHOS::STABUtil;
using namespace testing::ext;

using MAP_STR_STR = std::map<std::string, std::string>;
static const string KIT_BUNDLE_NAME = "com.ohos.amsst.MissionStack";
static const string KIT_HAP_NAME = "amsMissionStackTest";
static const string KIT_BUNDLE_NAME_SUBSIDIARY = "com.ohos.amsst.MissionStackSubsidiary";
static const string KIT_HAP_NAME_SUBSIDIARY = "amsMissionStackTestSubsidiary";
static const string MAIN_ABILITY_NAME = "MainAbility";
static const string SECOND_ABILITY_NAME = "SecondAbility";
static const string THIRD_ABILITY_NAME = "ThirdAbility";
static constexpr int WAIT_TIME = 1;
static constexpr int WAIT_LAUNCHER_TIME = 5;


static string g_eventMessage = "";
class AmsMissionStackTest : public testing::Test {
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

    static sptr<IAbilityManager> g_abilityMs;
    static Event event;
};

Event AmsMissionStackTest::event = Event();
sptr<IAbilityManager> AmsMissionStackTest::g_abilityMs = nullptr;

void AmsMissionStackTest::AppEventSubscriber::OnReceiveEvent(const CommonEventData &data)
{
    GTEST_LOG_(INFO) << "OnReceiveEvent: event=" << data.GetWant().GetAction();
    GTEST_LOG_(INFO) << "OnReceiveEvent: data=" << data.GetData();
    GTEST_LOG_(INFO) << "OnReceiveEvent: code=" << data.GetCode();
    if (data.GetWant().GetAction() == g_EVENT_RESP_MAIN_LIFECYCLE   ||
        data.GetWant().GetAction() == g_EVENT_RESP_SECOND_LIFECYCLE ||
        data.GetWant().GetAction() == g_EVENT_RESP_THIRD_LIFECYCLE  ||
        data.GetWant().GetAction() == g_EVENT_RESP_MAIN_LIFECYCLE_SUBSIDIARY    ||
        data.GetWant().GetAction() == g_EVENT_RESP_SECOND_LIFECYCLE_SUBSIDIARY
        ) {
        TestCompleted(event, data.GetData(), data.GetCode());
    } else if (data.GetWant().GetAction() == g_EVENT_RESP_MAIN   ||
               data.GetWant().GetAction() == g_EVENT_RESP_SECOND ||
               data.GetWant().GetAction() == g_EVENT_RESP_THIRD  ||
               data.GetWant().GetAction() == g_EVENT_RESP_MAIN_SUBSIDIARY   ||
               data.GetWant().GetAction() == g_EVENT_RESP_SECOND_SUBSIDIARY
               ) {
        g_eventMessage = data.GetData();
        TestCompleted(event, data.GetWant().GetAction(), data.GetCode());
        GTEST_LOG_(INFO) << "OnReceiveEvent: g_eventMessage=" << data.GetData();
    }
}

int AmsMissionStackTest::TestWaitCompleted(
    Event &event, const std::string &eventName, const int code, const int timeout)
{
    GTEST_LOG_(INFO) << "TestWaitCompleted : " << eventName << " " << code;
    return STAbilityUtil::WaitCompleted(event, eventName, code, timeout);
}

void AmsMissionStackTest::TestCompleted(Event &event, const std::string &eventName, const int code)
{
    GTEST_LOG_(INFO) << "TestCompleted : " << eventName << " " << code;
    return STAbilityUtil::Completed(event, eventName, code);
}

void AmsMissionStackTest::SetUpTestCase(void)
{
    if (!SubscribeEvent()) {
        GTEST_LOG_(INFO) << "SubscribeEvent error";
    }
}

void AmsMissionStackTest::TearDownTestCase(void)
{}

static int CODE_ = 0;
void AmsMissionStackTest::SetUp(void)
{
    STAbilityUtil::Install(KIT_HAP_NAME);
    STAbilityUtil::CleanMsg(event);
    CODE_++;
    sleep(WAIT_LAUNCHER_TIME);
}

void AmsMissionStackTest::TearDown(void)
{
    STAbilityUtil::Uninstall(KIT_BUNDLE_NAME);
    STAbilityUtil::CleanMsg(event);
}

bool AmsMissionStackTest::SubscribeEvent()
{
    std::vector<std::string> eventList = {
        g_EVENT_RESP_MAIN_LIFECYCLE,
        g_EVENT_RESP_SECOND_LIFECYCLE,
        g_EVENT_RESP_THIRD_LIFECYCLE,
        g_EVENT_RESP_MAIN,
        g_EVENT_RESP_SECOND,
        g_EVENT_RESP_THIRD,

        g_EVENT_RESP_MAIN_SUBSIDIARY,
        g_EVENT_RESP_MAIN_LIFECYCLE_SUBSIDIARY,

        g_EVENT_RESP_SECOND_SUBSIDIARY,
        g_EVENT_RESP_SECOND_LIFECYCLE_SUBSIDIARY,
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

void AmsMissionStackTest::Reinstall(const std::string &hapName, const std::string &bundleName)
{
    STAbilityUtil::Uninstall(bundleName);
    STAbilityUtil::Install(hapName);
}


/**
 * @tc.number    : FWK_MissionStack_0100
 * @tc.name      : test LockMission in ability_context.h
 * @tc.desc      : Verify that the result of LockMission function is correct.
 */
HWTEST_F(AmsMissionStackTest, FWK_MissionStack_0100, Function | MediumTest | Level1)
{
    GTEST_LOG_(INFO) << "AmsMissionStackTest FWK_MissionStack_0100 start";
    MAP_STR_STR params;
    Want want = STAbilityUtil::MakeWant("device", MAIN_ABILITY_NAME, KIT_BUNDLE_NAME, params);
    // start first ability
    ErrCode eCode = STAbilityUtil::StartAbility(want, g_abilityMs, WAIT_TIME);
    GTEST_LOG_(INFO) << eCode;

    EXPECT_EQ(TestWaitCompleted(event, "OnStart", MAIN_ABILITY_CODE), 0);
    EXPECT_EQ(TestWaitCompleted(event, "OnActive", MAIN_ABILITY_CODE), 0);
    STAbilityUtil::CleanMsg(event);
    STAbilityUtil::PublishEvent(
        g_EVENT_REQU_MAIN, CODE_, "MissionStack_" + std::to_string((int)MissionStackApi::LockMission) + "_0");
    EXPECT_EQ(TestWaitCompleted(event, g_EVENT_RESP_MAIN, CODE_), 0);
    EXPECT_EQ(TestWaitCompleted(event, "OnBackground", MAIN_ABILITY_CODE), 0);

    EXPECT_EQ(TestWaitCompleted(event, "OnStart", SECOND_ABILITY_CODE), 0);
    EXPECT_EQ(TestWaitCompleted(event, "OnActive", SECOND_ABILITY_CODE), 0);
    STAbilityUtil::CleanMsg(event);
    STAbilityUtil::PublishEvent(
        g_EVENT_REQU_SECOND, CODE_, "MissionStack_" + std::to_string((int)MissionStackApi::LockMission) + "_0");
    EXPECT_EQ(TestWaitCompleted(event, g_EVENT_RESP_SECOND, CODE_), 0);
    EXPECT_EQ(TestWaitCompleted(event, "OnStop", SECOND_ABILITY_CODE), 0);

    EXPECT_EQ(TestWaitCompleted(event, "OnActive", MAIN_ABILITY_CODE), 0);

    GTEST_LOG_(INFO) << "AmsMissionStackTest FWK_MissionStack_0100 end";
}

/**
 * @tc.number    : FWK_MissionStack_0200
 * @tc.name      : test LockMission in ability_context.h
 * @tc.desc      : Verify that the result of LockMission function is correct.
 */
HWTEST_F(AmsMissionStackTest, FWK_MissionStack_0200, Function | MediumTest | Level1)
{
    GTEST_LOG_(INFO) << "AmsMissionStackTest FWK_MissionStack_0200 start";
    MAP_STR_STR params;
    Want want = STAbilityUtil::MakeWant("device", MAIN_ABILITY_NAME, KIT_BUNDLE_NAME, params);
    // start first ability
    ErrCode eCode = STAbilityUtil::StartAbility(want, g_abilityMs, WAIT_TIME);
    GTEST_LOG_(INFO) << eCode;

    EXPECT_EQ(TestWaitCompleted(event, "OnStart", MAIN_ABILITY_CODE), 0);
    EXPECT_EQ(TestWaitCompleted(event, "OnActive", MAIN_ABILITY_CODE), 0);
    STAbilityUtil::CleanMsg(event);
    STAbilityUtil::PublishEvent(
        g_EVENT_REQU_MAIN, CODE_, "MissionStack_" + std::to_string((int)MissionStackApi::LockMission) + "_1");
    EXPECT_EQ(TestWaitCompleted(event, g_EVENT_RESP_MAIN, CODE_), 0);

    EXPECT_NE(TestWaitCompleted(event, "OnBackground", MAIN_ABILITY_CODE,WAIT_LAUNCHER_TIME), 0);

    GTEST_LOG_(INFO) << "AmsMissionStackTest FWK_MissionStack_0200 end";
}

/**
 * @tc.number    : FWK_MissionStack_0300
 * @tc.name      : test LockMission in ability_context.h
 * @tc.desc      : Verify that the result of LockMission function is correct.
 */
HWTEST_F(AmsMissionStackTest, FWK_MissionStack_0300, Function | MediumTest | Level1)
{
    GTEST_LOG_(INFO) << "AmsMissionStackTest FWK_MissionStack_0300 start";
    MAP_STR_STR params;
    Want want = STAbilityUtil::MakeWant("device", MAIN_ABILITY_NAME, KIT_BUNDLE_NAME, params);
    // start first ability
    ErrCode eCode = STAbilityUtil::StartAbility(want, g_abilityMs, WAIT_TIME);
    GTEST_LOG_(INFO) << eCode;

    EXPECT_EQ(TestWaitCompleted(event, "OnStart", MAIN_ABILITY_CODE), 0);
    EXPECT_EQ(TestWaitCompleted(event, "OnActive", MAIN_ABILITY_CODE), 0);
    STAbilityUtil::CleanMsg(event);
    STAbilityUtil::PublishEvent(
        g_EVENT_REQU_MAIN, CODE_, "MissionStack_" + std::to_string((int)MissionStackApi::LockMission) + "_2");
    EXPECT_EQ(TestWaitCompleted(event, g_EVENT_RESP_MAIN, CODE_), 0);
    
    Want wantEntity;
    wantEntity.AddEntity(Want::FLAG_HOME_INTENT_FROM_SYSTEM);
    STAbilityUtil::StartAbility(wantEntity, g_abilityMs);

    EXPECT_NE(TestWaitCompleted(event, "OnBackground", MAIN_ABILITY_CODE), 0);

    GTEST_LOG_(INFO) << "AmsMissionStackTest FWK_MissionStack_0300 end";
}

/**
 * @tc.number    : FWK_MissionStack_0400
 * @tc.name      : 
 * @tc.desc      : 
 */
HWTEST_F(AmsMissionStackTest, FWK_MissionStack_0400, Function | MediumTest | Level1)
{
    GTEST_LOG_(INFO) << "AmsMissionStackTest FWK_MissionStack_0400 start";
    //GTEST_LOG_(INFO) << "需要使用JS环境测试";
    GTEST_LOG_(INFO) << "AmsMissionStackTest FWK_MissionStack_0400 end";
}

/**
 * @tc.number    : FWK_MissionStack_0500
 * @tc.name      : test LockMission in ability_context.h
 * @tc.desc      : Verify that the result of LockMission function is correct.
 */
HWTEST_F(AmsMissionStackTest, FWK_MissionStack_0500, Function | MediumTest | Level1)
{
    GTEST_LOG_(INFO) << "AmsMissionStackTest FWK_MissionStack_0500 start";
    MAP_STR_STR params;
    Want want = STAbilityUtil::MakeWant("device", THIRD_ABILITY_NAME, KIT_BUNDLE_NAME, params);
    // start third ability
    ErrCode eCode = STAbilityUtil::StartAbility(want, g_abilityMs, WAIT_TIME);
    GTEST_LOG_(INFO) << eCode;

    EXPECT_EQ(TestWaitCompleted(event, "OnStart", THIRD_ABILITY_CODE), 0);
    EXPECT_EQ(TestWaitCompleted(event, "OnActive", THIRD_ABILITY_CODE), 0);
    STAbilityUtil::CleanMsg(event);
    STAbilityUtil::PublishEvent(
        g_EVENT_REQU_THIRD, CODE_, "MissionStack_" + std::to_string((int)MissionStackApi::LockMission) + "_4");
    EXPECT_EQ(TestWaitCompleted(event, g_EVENT_RESP_THIRD, CODE_), 0);
    
    EXPECT_NE(TestWaitCompleted(event, "OnStart", SECOND_ABILITY_CODE), 0);

    GTEST_LOG_(INFO) << "AmsMissionStackTest FWK_MissionStack_0500 end";
}

/**
 * @tc.number    : FWK_MissionStack_0600
 * @tc.name      : test LockMission in ability_context.h
 * @tc.desc      : Verify that the result of LockMission function is correct.
 */
HWTEST_F(AmsMissionStackTest, FWK_MissionStack_0600, Function | MediumTest | Level1)
{
    GTEST_LOG_(INFO) << "AmsMissionStackTest FWK_MissionStack_0600 start";
    MAP_STR_STR params;
    Want want = STAbilityUtil::MakeWant("device", MAIN_ABILITY_NAME, KIT_BUNDLE_NAME, params);
    // start third ability
    ErrCode eCode = STAbilityUtil::StartAbility(want, g_abilityMs, WAIT_TIME);
    GTEST_LOG_(INFO) << eCode;

    EXPECT_EQ(TestWaitCompleted(event, "OnStart", MAIN_ABILITY_CODE), 0);
    EXPECT_EQ(TestWaitCompleted(event, "OnActive", MAIN_ABILITY_CODE), 0);
    STAbilityUtil::CleanMsg(event);
    STAbilityUtil::PublishEvent(
        g_EVENT_REQU_MAIN, CODE_, "MissionStack_" + std::to_string((int)MissionStackApi::LockMission) + "_5");
    EXPECT_EQ(TestWaitCompleted(event, g_EVENT_RESP_MAIN, CODE_), 0);
    
    EXPECT_NE(TestWaitCompleted(event, "OnStart", THIRD_ABILITY_CODE), 0);

    GTEST_LOG_(INFO) << "AmsMissionStackTest FWK_MissionStack_0600 end";
}

/**
 * @tc.number    : FWK_MissionStack_0700
 * @tc.name      : test LockMission in ability_context.h
 * @tc.desc      : Verify that the result of LockMission function is correct.
 */
HWTEST_F(AmsMissionStackTest, FWK_MissionStack_0700, Function | MediumTest | Level1)
{
    GTEST_LOG_(INFO) << "AmsMissionStackTest FWK_MissionStack_0700 start";
    MAP_STR_STR params;
    Want want = STAbilityUtil::MakeWant("device", MAIN_ABILITY_NAME, KIT_BUNDLE_NAME, params);
    // start first ability
    ErrCode eCode = STAbilityUtil::StartAbility(want, g_abilityMs, WAIT_TIME);
    GTEST_LOG_(INFO) << eCode;

    EXPECT_EQ(TestWaitCompleted(event, "OnStart", MAIN_ABILITY_CODE), 0);
    EXPECT_EQ(TestWaitCompleted(event, "OnActive", MAIN_ABILITY_CODE), 0);
    STAbilityUtil::CleanMsg(event);
    STAbilityUtil::PublishEvent(
        g_EVENT_REQU_MAIN, CODE_, "MissionStack_" + std::to_string((int)MissionStackApi::LockMission) + "_6");
    EXPECT_EQ(TestWaitCompleted(event, g_EVENT_RESP_MAIN, CODE_), 0);
    
    Want wantEntity;
    wantEntity.AddEntity(Want::FLAG_HOME_INTENT_FROM_SYSTEM);
    STAbilityUtil::StartAbility(wantEntity, g_abilityMs);

    EXPECT_EQ(TestWaitCompleted(event, "OnBackground", MAIN_ABILITY_CODE,WAIT_LAUNCHER_TIME), 0);

    GTEST_LOG_(INFO) << "AmsMissionStackTest FWK_MissionStack_0700 end";
}

/**
 * @tc.number    : FWK_MissionStack_0800
 * @tc.name      : 
 * @tc.desc      : 
 */
HWTEST_F(AmsMissionStackTest, FWK_MissionStack_0800, Function | MediumTest | Level1)
{
    GTEST_LOG_(INFO) << "AmsMissionStackTest FWK_MissionStack_0800 start";
    //GTEST_LOG_(INFO) << "需要使用JS环境测试";
    GTEST_LOG_(INFO) << "AmsMissionStackTest FWK_MissionStack_0800 end";
}

/**
 * @tc.number    : FWK_MissionStack_0900
 * @tc.name      : test LockMission in ability_context.h
 * @tc.desc      : Verify that the result of LockMission function is correct.
 */
HWTEST_F(AmsMissionStackTest, FWK_MissionStack_0900, Function | MediumTest | Level1)
{
    GTEST_LOG_(INFO) << "AmsMissionStackTest FWK_MissionStack_0900 start";
    MAP_STR_STR params;
    Want want = STAbilityUtil::MakeWant("device", MAIN_ABILITY_NAME, KIT_BUNDLE_NAME, params);
    // start first ability
    ErrCode eCode = STAbilityUtil::StartAbility(want, g_abilityMs, WAIT_TIME);
    GTEST_LOG_(INFO) << eCode;

    EXPECT_EQ(TestWaitCompleted(event, "OnStart", MAIN_ABILITY_CODE), 0);
    EXPECT_EQ(TestWaitCompleted(event, "OnActive", MAIN_ABILITY_CODE), 0);
    STAbilityUtil::CleanMsg(event);
    STAbilityUtil::PublishEvent(
        g_EVENT_REQU_MAIN, CODE_, "MissionStack_" + std::to_string((int)MissionStackApi::LockMission) + "_8");
    EXPECT_EQ(TestWaitCompleted(event, g_EVENT_RESP_MAIN, CODE_), 0);
    EXPECT_EQ(TestWaitCompleted(event, "OnBackground", MAIN_ABILITY_CODE), 0);

    EXPECT_EQ(TestWaitCompleted(event, "OnStart", SECOND_ABILITY_CODE), 0);
    EXPECT_EQ(TestWaitCompleted(event, "OnActive", SECOND_ABILITY_CODE), 0);
    STAbilityUtil::CleanMsg(event);
    STAbilityUtil::PublishEvent(
        g_EVENT_REQU_SECOND, CODE_, "MissionStack_" + std::to_string((int)MissionStackApi::LockMission) + "_8");
    EXPECT_EQ(TestWaitCompleted(event, g_EVENT_RESP_SECOND, CODE_), 0);
    EXPECT_EQ(TestWaitCompleted(event, "OnStop", SECOND_ABILITY_CODE), 0);

    EXPECT_EQ(TestWaitCompleted(event, "OnActive", MAIN_ABILITY_CODE), 0);

    GTEST_LOG_(INFO) << "AmsMissionStackTest FWK_MissionStack_0900 end";
}

/**
 * @tc.number    : FWK_MissionStack_1000
 * @tc.name      : test LockMission in ability_context.h
 * @tc.desc      : Verify that the result of LockMission function is correct.
 */
HWTEST_F(AmsMissionStackTest, FWK_MissionStack_1000, Function | MediumTest | Level1)
{
    GTEST_LOG_(INFO) << "AmsMissionStackTest FWK_MissionStack_1000 start";
    MAP_STR_STR params;
    Want want = STAbilityUtil::MakeWant("device", MAIN_ABILITY_NAME, KIT_BUNDLE_NAME, params);
    // start first ability
    ErrCode eCode = STAbilityUtil::StartAbility(want, g_abilityMs, WAIT_TIME);
    GTEST_LOG_(INFO) << eCode;

    EXPECT_EQ(TestWaitCompleted(event, "OnStart", MAIN_ABILITY_CODE), 0);
    EXPECT_EQ(TestWaitCompleted(event, "OnActive", MAIN_ABILITY_CODE), 0);
    STAbilityUtil::CleanMsg(event);
    STAbilityUtil::PublishEvent(
        g_EVENT_REQU_MAIN, CODE_, "MissionStack_" + std::to_string((int)MissionStackApi::LockMission) + "_9");
    EXPECT_EQ(TestWaitCompleted(event, g_EVENT_RESP_MAIN, CODE_), 0);
    string appInfo = g_eventMessage;

    GTEST_LOG_(INFO) << "id=" << appInfo;
    //EXPECT_EQ(appInfo, "1");

    GTEST_LOG_(INFO) << "AmsMissionStackTest FWK_MissionStack_1000 end";
}

/**
 * @tc.number    : FWK_MissionStack_1100
 * @tc.name      : 
 * @tc.desc      : 
 */
HWTEST_F(AmsMissionStackTest, FWK_MissionStack_1100, Function | MediumTest | Level1)
{
    GTEST_LOG_(INFO) << "AmsMissionStackTest FWK_MissionStack_1100 start";
    //GTEST_LOG_(INFO) << "需要使用JS环境测试";
    GTEST_LOG_(INFO) << "AmsMissionStackTest FWK_MissionStack_1100 end";
}

/**
 * @tc.number    : FWK_MissionStack_1200
 * @tc.name      : test MoveMissionToEnd in ability_context.h
 * @tc.desc      : Verify that the result of MoveMissionToEnd function is correct.
 */
HWTEST_F(AmsMissionStackTest, FWK_MissionStack_1200, Function | MediumTest | Level1)
{
    GTEST_LOG_(INFO) << "AmsMissionStackTest FWK_MissionStack_1200 start";
    MAP_STR_STR params;
    Want want = STAbilityUtil::MakeWant("device", MAIN_ABILITY_NAME, KIT_BUNDLE_NAME, params);
    // start first ability
    ErrCode eCode = STAbilityUtil::StartAbility(want, g_abilityMs, WAIT_TIME);
    GTEST_LOG_(INFO) << eCode;

    EXPECT_EQ(TestWaitCompleted(event, "OnStart", MAIN_ABILITY_CODE), 0);
    EXPECT_EQ(TestWaitCompleted(event, "OnActive", MAIN_ABILITY_CODE), 0);
    STAbilityUtil::CleanMsg(event);
    STAbilityUtil::PublishEvent(
        g_EVENT_REQU_MAIN, CODE_, "MissionStack_" + std::to_string((int)MissionStackApi::LockMission) + "_11");
    EXPECT_EQ(TestWaitCompleted(event, g_EVENT_RESP_MAIN, CODE_), 0);
    
    EXPECT_EQ(TestWaitCompleted(event, "OnBackground", MAIN_ABILITY_CODE,WAIT_LAUNCHER_TIME), 0);

    GTEST_LOG_(INFO) << "AmsMissionStackTest FWK_MissionStack_1200 end";
}

/**
 * @tc.number    : FWK_MissionStack_1300
 * @tc.name      : test MoveMissionToEnd in ability_context.h
 * @tc.desc      : Verify that the result of MoveMissionToEnd function is correct.
 */
HWTEST_F(AmsMissionStackTest, FWK_MissionStack_1300, Function | MediumTest | Level1)
{
    GTEST_LOG_(INFO) << "AmsMissionStackTest FWK_MissionStack_1300 start";
    MAP_STR_STR params;
    Want want = STAbilityUtil::MakeWant("device", MAIN_ABILITY_NAME, KIT_BUNDLE_NAME, params);
    // start first ability
    ErrCode eCode = STAbilityUtil::StartAbility(want, g_abilityMs, WAIT_TIME);
    GTEST_LOG_(INFO) << eCode;

    EXPECT_EQ(TestWaitCompleted(event, "OnStart", MAIN_ABILITY_CODE), 0);
    EXPECT_EQ(TestWaitCompleted(event, "OnActive", MAIN_ABILITY_CODE), 0);
    STAbilityUtil::CleanMsg(event);
    STAbilityUtil::PublishEvent(
        g_EVENT_REQU_MAIN, CODE_, "MissionStack_" + std::to_string((int)MissionStackApi::LockMission) + "_12");
    EXPECT_EQ(TestWaitCompleted(event, g_EVENT_RESP_MAIN, CODE_), 0);
    
    EXPECT_EQ(TestWaitCompleted(event, "OnBackground", MAIN_ABILITY_CODE,WAIT_LAUNCHER_TIME), 0);

    GTEST_LOG_(INFO) << "AmsMissionStackTest FWK_MissionStack_1300 end";
}

/**
 * @tc.number    : FWK_MissionStack_1400
 * @tc.name      : test MoveMissionToEnd in ability_context.h
 * @tc.desc      : Verify that the result of MoveMissionToEnd function is correct.
 */
HWTEST_F(AmsMissionStackTest, FWK_MissionStack_1400, Function | MediumTest | Level1)
{
    GTEST_LOG_(INFO) << "AmsMissionStackTest FWK_MissionStack_1400 start";
    MAP_STR_STR params;
    Want want = STAbilityUtil::MakeWant("device", MAIN_ABILITY_NAME, KIT_BUNDLE_NAME, params);
    // start first ability
    ErrCode eCode = STAbilityUtil::StartAbility(want, g_abilityMs, WAIT_TIME);
    GTEST_LOG_(INFO) << eCode;

    EXPECT_EQ(TestWaitCompleted(event, "OnStart", MAIN_ABILITY_CODE), 0);
    EXPECT_EQ(TestWaitCompleted(event, "OnActive", MAIN_ABILITY_CODE), 0);
    STAbilityUtil::CleanMsg(event);
    STAbilityUtil::PublishEvent(
        g_EVENT_REQU_MAIN, CODE_, "MissionStack_" + std::to_string((int)MissionStackApi::LockMission) + "_13");
    EXPECT_EQ(TestWaitCompleted(event, g_EVENT_RESP_MAIN, CODE_), 0);
    EXPECT_EQ(TestWaitCompleted(event, "OnBackground", MAIN_ABILITY_CODE), 0);

    EXPECT_EQ(TestWaitCompleted(event, "OnStart", SECOND_ABILITY_CODE), 0);
    EXPECT_EQ(TestWaitCompleted(event, "OnActive", SECOND_ABILITY_CODE), 0);
    STAbilityUtil::CleanMsg(event);
    STAbilityUtil::PublishEvent(
        g_EVENT_REQU_SECOND, CODE_, "MissionStack_" + std::to_string((int)MissionStackApi::LockMission) + "_13");
    EXPECT_EQ(TestWaitCompleted(event, g_EVENT_RESP_SECOND, CODE_), 0);
    EXPECT_EQ(TestWaitCompleted(event, "OnBackground", SECOND_ABILITY_CODE,WAIT_LAUNCHER_TIME), 0);

    GTEST_LOG_(INFO) << "AmsMissionStackTest FWK_MissionStack_1400 end";
}

/**
 * @tc.number    : FWK_MissionStack_1500
 * @tc.name      : test MoveMissionToEnd in ability_context.h
 * @tc.desc      : Verify that the result of MoveMissionToEnd function is correct.
 */
HWTEST_F(AmsMissionStackTest, FWK_MissionStack_1500, Function | MediumTest | Level1)
{
    GTEST_LOG_(INFO) << "AmsMissionStackTest FWK_MissionStack_1500 start";
    MAP_STR_STR params;
    Want want = STAbilityUtil::MakeWant("device", MAIN_ABILITY_NAME, KIT_BUNDLE_NAME, params);
    // start first ability
    ErrCode eCode = STAbilityUtil::StartAbility(want, g_abilityMs, WAIT_TIME);
    GTEST_LOG_(INFO) << eCode;

    EXPECT_EQ(TestWaitCompleted(event, "OnStart", MAIN_ABILITY_CODE), 0);
    EXPECT_EQ(TestWaitCompleted(event, "OnActive", MAIN_ABILITY_CODE), 0);
    STAbilityUtil::CleanMsg(event);
    STAbilityUtil::PublishEvent(
        g_EVENT_REQU_MAIN, CODE_, "MissionStack_" + std::to_string((int)MissionStackApi::LockMission) + "_14");
    EXPECT_EQ(TestWaitCompleted(event, g_EVENT_RESP_MAIN, CODE_), 0);
    EXPECT_EQ(TestWaitCompleted(event, "OnBackground", MAIN_ABILITY_CODE), 0);

    EXPECT_EQ(TestWaitCompleted(event, "OnStart", SECOND_ABILITY_CODE), 0);
    EXPECT_EQ(TestWaitCompleted(event, "OnActive", SECOND_ABILITY_CODE), 0);
    STAbilityUtil::CleanMsg(event);
    STAbilityUtil::PublishEvent(
        g_EVENT_REQU_SECOND, CODE_, "MissionStack_" + std::to_string((int)MissionStackApi::LockMission) + "_14");
    EXPECT_EQ(TestWaitCompleted(event, g_EVENT_RESP_SECOND, CODE_), 0);
    EXPECT_NE(TestWaitCompleted(event, "OnBackground", SECOND_ABILITY_CODE,WAIT_LAUNCHER_TIME), 0);

    GTEST_LOG_(INFO) << "AmsMissionStackTest FWK_MissionStack_1500 end";
}

/**
 * @tc.number    : FWK_MissionStack_1600
 * @tc.name      : test MoveMissionToEnd in ability_context.h
 * @tc.desc      : Verify that the result of MoveMissionToEnd function is correct.
 */
HWTEST_F(AmsMissionStackTest, FWK_MissionStack_1600, Function | MediumTest | Level1)
{
    GTEST_LOG_(INFO) << "AmsMissionStackTest FWK_MissionStack_1600 start";
    MAP_STR_STR params;
    Want want = STAbilityUtil::MakeWant("device", MAIN_ABILITY_NAME, KIT_BUNDLE_NAME, params);
    // start first ability
    ErrCode eCode = STAbilityUtil::StartAbility(want, g_abilityMs, WAIT_TIME);
    GTEST_LOG_(INFO) << eCode;

    EXPECT_EQ(TestWaitCompleted(event, "OnStart", MAIN_ABILITY_CODE), 0);
    EXPECT_EQ(TestWaitCompleted(event, "OnActive", MAIN_ABILITY_CODE), 0);
    Want wantEntity;
    wantEntity.AddEntity(Want::FLAG_HOME_INTENT_FROM_SYSTEM);
    STAbilityUtil::StartAbility(wantEntity, g_abilityMs);
    sleep(WAIT_LAUNCHER_TIME);
    STAbilityUtil::CleanMsg(event);

    STAbilityUtil::Install(KIT_HAP_NAME_SUBSIDIARY);
    STAbilityUtil::CleanMsg(event);
    sleep(WAIT_LAUNCHER_TIME);
    want = STAbilityUtil::MakeWant("device", MAIN_ABILITY_NAME, KIT_BUNDLE_NAME_SUBSIDIARY, params);
    // start first ability
    eCode = STAbilityUtil::StartAbility(want, g_abilityMs, WAIT_TIME);
    GTEST_LOG_(INFO) <<"SUBSIDIARY:StartAbility:eCode=" << eCode;

    EXPECT_EQ(TestWaitCompleted(event, "OnStart", MAIN_ABILITY_CODE_SUBSIDIARY), 0);
    EXPECT_EQ(TestWaitCompleted(event, "OnActive", MAIN_ABILITY_CODE_SUBSIDIARY), 0);

    STAbilityUtil::PublishEvent(
        g_EVENT_REQU_MAIN_SUBSIDIARY, CODE_, "MissionStack_" + std::to_string((int)MissionStackApi::LockMission) + "_0");
    EXPECT_EQ(TestWaitCompleted(event, g_EVENT_RESP_MAIN_SUBSIDIARY, CODE_), 0);

    EXPECT_EQ(TestWaitCompleted(event, "OnBackground", MAIN_ABILITY_CODE,WAIT_LAUNCHER_TIME), 0);
    EXPECT_EQ(TestWaitCompleted(event, "OnBackground", MAIN_ABILITY_CODE_SUBSIDIARY,WAIT_LAUNCHER_TIME), 0);
    EXPECT_EQ(TestWaitCompleted(event, "OnStart", SECOND_ABILITY_CODE_SUBSIDIARY), 0);
    EXPECT_EQ(TestWaitCompleted(event, "OnActive", SECOND_ABILITY_CODE_SUBSIDIARY), 0);

    STAbilityUtil::PublishEvent(
        g_EVENT_REQU_MAIN_SUBSIDIARY, CODE_, "MissionStack_" + std::to_string((int)MissionStackApi::LockMission) + "_1");
    EXPECT_EQ(TestWaitCompleted(event, g_EVENT_RESP_MAIN_SUBSIDIARY, CODE_), 0);
    EXPECT_EQ(TestWaitCompleted(event, "OnBackground", SECOND_ABILITY_CODE_SUBSIDIARY,WAIT_LAUNCHER_TIME), 0);

    STAbilityUtil::Uninstall(KIT_BUNDLE_NAME_SUBSIDIARY);
    STAbilityUtil::CleanMsg(event);
    GTEST_LOG_(INFO) << "AmsMissionStackTest FWK_MissionStack_1600 end";
}

// /**
//  * @tc.number    : FWK_MissionStack_1700
//  * @tc.name      : test MoveMissionToEnd in ability_context.h
//  * @tc.desc      : Verify that the result of MoveMissionToEnd function is correct.
//  */
HWTEST_F(AmsMissionStackTest, FWK_MissionStack_1700, Function | MediumTest | Level1)
{
    GTEST_LOG_(INFO) << "AmsMissionStackTest FWK_MissionStack_1700 start";
    MAP_STR_STR params;
    Want want = STAbilityUtil::MakeWant("device", MAIN_ABILITY_NAME, KIT_BUNDLE_NAME, params);
    // start first ability
    ErrCode eCode = STAbilityUtil::StartAbility(want, g_abilityMs, WAIT_TIME);
    GTEST_LOG_(INFO) << eCode;

    EXPECT_EQ(TestWaitCompleted(event, "OnStart", MAIN_ABILITY_CODE), 0);
    EXPECT_EQ(TestWaitCompleted(event, "OnActive", MAIN_ABILITY_CODE), 0);
    Want wantEntity;
    wantEntity.AddEntity(Want::FLAG_HOME_INTENT_FROM_SYSTEM);
    STAbilityUtil::StartAbility(wantEntity, g_abilityMs);
    sleep(WAIT_LAUNCHER_TIME);
    STAbilityUtil::CleanMsg(event);

    STAbilityUtil::Install(KIT_HAP_NAME_SUBSIDIARY);
    STAbilityUtil::CleanMsg(event);
    sleep(WAIT_LAUNCHER_TIME);
    want = STAbilityUtil::MakeWant("device", MAIN_ABILITY_NAME, KIT_BUNDLE_NAME_SUBSIDIARY, params);
    // start first ability
    eCode = STAbilityUtil::StartAbility(want, g_abilityMs, WAIT_TIME);
    GTEST_LOG_(INFO) <<"SUBSIDIARY:StartAbility:eCode=" << eCode;

    EXPECT_EQ(TestWaitCompleted(event, "OnStart", MAIN_ABILITY_CODE_SUBSIDIARY), 0);
    EXPECT_EQ(TestWaitCompleted(event, "OnActive", MAIN_ABILITY_CODE_SUBSIDIARY), 0);

    STAbilityUtil::PublishEvent(
        g_EVENT_REQU_MAIN_SUBSIDIARY, CODE_, "MissionStack_" + std::to_string((int)MissionStackApi::LockMission) + "_0");
    EXPECT_EQ(TestWaitCompleted(event, g_EVENT_RESP_MAIN_SUBSIDIARY, CODE_), 0);

    EXPECT_EQ(TestWaitCompleted(event, "OnBackground", MAIN_ABILITY_CODE,WAIT_LAUNCHER_TIME), 0);
    EXPECT_EQ(TestWaitCompleted(event, "OnBackground", MAIN_ABILITY_CODE_SUBSIDIARY,WAIT_LAUNCHER_TIME), 0);
    EXPECT_EQ(TestWaitCompleted(event, "OnStart", SECOND_ABILITY_CODE_SUBSIDIARY), 0);
    EXPECT_EQ(TestWaitCompleted(event, "OnActive", SECOND_ABILITY_CODE_SUBSIDIARY), 0);

    STAbilityUtil::PublishEvent(
        g_EVENT_REQU_MAIN_SUBSIDIARY, CODE_, "MissionStack_" + std::to_string((int)MissionStackApi::LockMission) + "_2");
    EXPECT_EQ(TestWaitCompleted(event, g_EVENT_RESP_MAIN_SUBSIDIARY, CODE_), 0);
    EXPECT_NE(TestWaitCompleted(event, "OnBackground", SECOND_ABILITY_CODE_SUBSIDIARY,WAIT_LAUNCHER_TIME), 0);

    STAbilityUtil::Uninstall(KIT_BUNDLE_NAME_SUBSIDIARY);
    STAbilityUtil::CleanMsg(event);
    GTEST_LOG_(INFO) << "AmsMissionStackTest FWK_MissionStack_1700 end";
}

/**
 * @tc.number    : FWK_MissionStack_1800
 * @tc.name      : test MoveMissionToEnd in ability_context.h
 * @tc.desc      : Verify that the result of MoveMissionToEnd function is correct.
 */
HWTEST_F(AmsMissionStackTest, FWK_MissionStack_1800, Function | MediumTest | Level1)
{
    GTEST_LOG_(INFO) << "AmsMissionStackTest FWK_MissionStack_1800 start";
    MAP_STR_STR params;
    Want want = STAbilityUtil::MakeWant("device", MAIN_ABILITY_NAME, KIT_BUNDLE_NAME, params);
    // start first ability
    ErrCode eCode = STAbilityUtil::StartAbility(want, g_abilityMs, WAIT_TIME);
    GTEST_LOG_(INFO) << eCode;

    EXPECT_EQ(TestWaitCompleted(event, "OnStart", MAIN_ABILITY_CODE), 0);
    EXPECT_EQ(TestWaitCompleted(event, "OnActive", MAIN_ABILITY_CODE), 0);
    STAbilityUtil::CleanMsg(event);
    STAbilityUtil::PublishEvent(
        g_EVENT_REQU_MAIN, CODE_, "MissionStack_" + std::to_string((int)MissionStackApi::LockMission) + "_17");
    EXPECT_EQ(TestWaitCompleted(event, g_EVENT_RESP_MAIN, CODE_), 0);
    
    EXPECT_NE(TestWaitCompleted(event, "OnBackground", MAIN_ABILITY_CODE,WAIT_LAUNCHER_TIME), 0);

    GTEST_LOG_(INFO) << "AmsMissionStackTest FWK_MissionStack_1800 end";
}

// /**
//  * @tc.number    : FWK_SaveAbilityState_0100
//  * @tc.name      : 
//  * @tc.desc      : 
//  */
// HWTEST_F(AmsMissionStackTest, FWK_SaveAbilityState_0100, Function | MediumTest | Level1)
// {
//     GTEST_LOG_(INFO) << "AmsMissionStackTest FWK_SaveAbilityState_0100 start";
//     MAP_STR_STR params;
//     Want want = STAbilityUtil::MakeWant("device", MAIN_ABILITY_NAME, KIT_BUNDLE_NAME, params);
//     // start first ability
//     ErrCode eCode = STAbilityUtil::StartAbility(want, g_abilityMs, WAIT_TIME);
//     GTEST_LOG_(INFO) << eCode;

//     EXPECT_EQ(TestWaitCompleted(event, "OnStart", MAIN_ABILITY_CODE), 0);
//     EXPECT_EQ(TestWaitCompleted(event, "OnActive", MAIN_ABILITY_CODE), 0);
    
//     Want wantEntity;
//     wantEntity.AddEntity(Want::FLAG_HOME_INTENT_FROM_SYSTEM);
//     STAbilityUtil::StartAbility(wantEntity, g_abilityMs);

//     EXPECT_NE(TestWaitCompleted(event, "OnSaveAbilityState", MAIN_ABILITY_CODE), 0);
//     GTEST_LOG_(INFO) << "AmsMissionStackTest FWK_SaveAbilityState_0100 end";
// }

// /**
//  * @tc.number    : FWK_SaveAbilityState_0200
//  * @tc.name      : 
//  * @tc.desc      : 
//  */
// HWTEST_F(AmsMissionStackTest, FWK_SaveAbilityState_0200, Function | MediumTest | Level1)
// {
//     GTEST_LOG_(INFO) << "AmsMissionStackTest FWK_SaveAbilityState_0200 start";

//     // 该case需要屏幕翻转事件
//     GTEST_LOG_(INFO) << "AmsMissionStackTest FWK_SaveAbilityState_0200 end";
// }

// /**
//  * @tc.number    : FWK_SaveAbilityState_0300
//  * @tc.name      : 
//  * @tc.desc      : 
//  */
// HWTEST_F(AmsMissionStackTest, FWK_SaveAbilityState_0300, Function | MediumTest | Level1)
// {
//     GTEST_LOG_(INFO) << "AmsMissionStackTest FWK_SaveAbilityState_0300 start";
//     MAP_STR_STR params;
//     Want want = STAbilityUtil::MakeWant("device", MAIN_ABILITY_NAME, KIT_BUNDLE_NAME, params);
//     // start first ability
//     ErrCode eCode = STAbilityUtil::StartAbility(want, g_abilityMs, WAIT_TIME);
//     GTEST_LOG_(INFO) << eCode;

//     EXPECT_EQ(TestWaitCompleted(event, "OnStart", MAIN_ABILITY_CODE), 0);
//     EXPECT_EQ(TestWaitCompleted(event, "OnActive", MAIN_ABILITY_CODE), 0);
//     STAbilityUtil::CleanMsg(event);
//     STAbilityUtil::PublishEvent(
//         g_EVENT_REQU_MAIN, CODE_, "AbilityState_" + std::to_string((int)TestAbilityState::OnSaveAbilityState) + "_2");
//     EXPECT_EQ(TestWaitCompleted(event, g_EVENT_RESP_MAIN, CODE_), 0);
    
//     EXPECT_EQ(TestWaitCompleted(event, "OnStart", SECOND_ABILITY_CODE), 0);
//     EXPECT_EQ(TestWaitCompleted(event, "OnActive", SECOND_ABILITY_CODE), 0);

//     EXPECT_NE(TestWaitCompleted(event, "OnSaveAbilityState", MAIN_ABILITY_CODE), 0);
//     GTEST_LOG_(INFO) << "AmsMissionStackTest FWK_SaveAbilityState_0300 end";
// }

// /**
//  * @tc.number    : FWK_RestoreAbilityState_0100
//  * @tc.name      : 
//  * @tc.desc      : 
//  */
// HWTEST_F(AmsMissionStackTest, FWK_RestoreAbilityState_0100, Function | MediumTest | Level1)
// {
//     GTEST_LOG_(INFO) << "AmsMissionStackTest FWK_RestoreAbilityState_0100 start";

//     MAP_STR_STR params;
//     Want want = STAbilityUtil::MakeWant("device", THIRD_ABILITY_NAME, KIT_BUNDLE_NAME, params);
//     // start first ability
//     ErrCode eCode = STAbilityUtil::StartAbility(want, g_abilityMs, WAIT_TIME);
//     GTEST_LOG_(INFO) << eCode;

//     EXPECT_EQ(TestWaitCompleted(event, "OnStart", THIRD_ABILITY_CODE), 0);
//     EXPECT_EQ(TestWaitCompleted(event, "OnActive", THIRD_ABILITY_CODE), 0);

//     Want wantEntity;
//     wantEntity.AddEntity(Want::FLAG_HOME_INTENT_FROM_SYSTEM);
//     STAbilityUtil::StartAbility(wantEntity, g_abilityMs);

//     sleep(WAIT_LAUNCHER_TIME);
//     eCode = STAbilityUtil::StartAbility(want, g_abilityMs, WAIT_TIME);
//     GTEST_LOG_(INFO) << eCode;
//     EXPECT_EQ(TestWaitCompleted(event, "OnActive", THIRD_ABILITY_CODE), 0);

//     EXPECT_NE(TestWaitCompleted(event, "OnRestoreAbilityState", THIRD_ABILITY_CODE), 0);

//     GTEST_LOG_(INFO) << "AmsMissionStackTest FWK_RestoreAbilityState_0100 end";
// }

// /**
//  * @tc.number    : FWK_RestoreAbilityState_0200
//  * @tc.name      : 
//  * @tc.desc      : 
//  */
// HWTEST_F(AmsMissionStackTest, FWK_RestoreAbilityState_0200, Function | MediumTest | Level1)
// {
//     GTEST_LOG_(INFO) << "AmsMissionStackTest FWK_RestoreAbilityState_0200 start";
//     // 该case需要屏幕翻转事件
//     GTEST_LOG_(INFO) << "AmsMissionStackTest FWK_RestoreAbilityState_0200 end";
// }

// /**
//  * @tc.number    : FWK_RestoreAbilityState_0300
//  * @tc.name      : 
//  * @tc.desc      : 
//  */
// HWTEST_F(AmsMissionStackTest, FWK_RestoreAbilityState_0300, Function | MediumTest | Level1)
// {
//     GTEST_LOG_(INFO) << "AmsMissionStackTest FWK_RestoreAbilityState_0300 start";
//     MAP_STR_STR params;
//     Want want = STAbilityUtil::MakeWant("device", MAIN_ABILITY_NAME, KIT_BUNDLE_NAME, params);
//     // start first ability
//     ErrCode eCode = STAbilityUtil::StartAbility(want, g_abilityMs, WAIT_TIME);
//     GTEST_LOG_(INFO) << eCode;

//     EXPECT_EQ(TestWaitCompleted(event, "OnStart", MAIN_ABILITY_CODE), 0);
//     EXPECT_EQ(TestWaitCompleted(event, "OnActive", MAIN_ABILITY_CODE), 0);
//     STAbilityUtil::CleanMsg(event);
//     STAbilityUtil::PublishEvent(
//         g_EVENT_REQU_MAIN, CODE_, "AbilityState_" + std::to_string((int)TestAbilityState::OnRestoreAbilityState) + "_2");
//     EXPECT_EQ(TestWaitCompleted(event, g_EVENT_RESP_MAIN, CODE_), 0);
    
//     EXPECT_EQ(TestWaitCompleted(event, "OnStart", SECOND_ABILITY_CODE), 0);
//     EXPECT_EQ(TestWaitCompleted(event, "OnActive", SECOND_ABILITY_CODE), 0);
//     STAbilityUtil::PublishEvent(
//         g_EVENT_REQU_SECOND, CODE_, "AbilityState_" + std::to_string((int)TestAbilityState::OnRestoreAbilityState) + "_2");
//     EXPECT_EQ(TestWaitCompleted(event, g_EVENT_RESP_SECOND, CODE_), 0);
//     EXPECT_EQ(TestWaitCompleted(event, "OnStop", SECOND_ABILITY_CODE), 0);

//     EXPECT_NE(TestWaitCompleted(event, "OnRestoreAbilityState", MAIN_ABILITY_CODE), 0);

//     GTEST_LOG_(INFO) << "AmsMissionStackTest FWK_RestoreAbilityState_0300 end";
// }

}  // namespace
