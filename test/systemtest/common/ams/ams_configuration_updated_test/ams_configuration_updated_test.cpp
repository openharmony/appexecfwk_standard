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
#include <algorithm>
#include <vector>
#include "ability_append_test_info.h"
#include "common_event.h"
#include "common_event_manager.h"
#include "configuration.h"
#include "hilog_wrapper.h"
#include "stoperator.h"
#include "st_ability_util.h"
namespace {
using namespace OHOS;
using namespace OHOS::AAFwk;
using namespace OHOS::AppExecFwk;
using namespace OHOS::EventFwk;
using namespace OHOS::STtools;
using namespace OHOS::STABUtil;
using namespace testing::ext;

using MAP_STR_STR = std::map<std::string, std::string>;
namespace {
static const string KIT_BUNDLE_NAME = "com.ohos.amsst.ConfigurationUpdated";
static const string KIT_SECOND_BUNDLE_NAME = "com.ohos.amsst.ConfigurationUpdatedSingleton";
static const string KIT_HAP_NAME = "amsConfigurationUpdatedTest";
static const string KIT_SECOND_HAP_NAME = "amsConfigurationUpdatedSingletonTest";
static const string MAIN_ABILITY = "MainAbility";
static const string SECOND_ABILITY = "SecondAbility";
static constexpr int WAIT_TIME = 1;
static constexpr int WAIT_LAUNCHER_TIME = 5;
static constexpr int WAIT_SETUP_TIME = 1;
static constexpr int WAIT_TEARDOWN_TIME = 1;
static constexpr int WAIT_ONACTIVE_TIME = 1;
static constexpr int WAIT_BLOCKUPDATE_TIME = 7;
static constexpr int TEST_TIMES = 10;
static string g_eventMessage = "";
static string g_tempDataStr = "";
static int g_mainAbilityUpdateTimes = 0;
static int g_secondAbilityUpdateTimes = 0;
static int g_thirdAbilityUpdateTimes = 0;
std::string ConfigurationUpdate_Event_Resp_A = "resp_com_ohos_amsst_ConfigurationUpdateB";
}  // namespace

std::vector<std::string> eventList = {
    g_EVENT_RESP_MAIN_LIFECYCLE,
};

class AmsConfigurationUpdatedTest : public testing::Test {
public:
    static void SetUpTestCase(void);
    static void TearDownTestCase(void);
    void SetUp();
    void TearDown();

    static bool SubscribeEvent();
    static int TestWaitCompleted(Event &event, const std::string &eventName, const int code, const int timeout = 10);
    static void TestCompleted(Event &event, const std::string &eventName, const int code);
    void SetDefaultConfig();

    class AppEventSubscriber : public CommonEventSubscriber {
    public:
        explicit AppEventSubscriber(const CommonEventSubscribeInfo &sp) : CommonEventSubscriber(sp){};
        virtual void OnReceiveEvent(const CommonEventData &data) override;
        ~AppEventSubscriber(){};
    };

    static sptr<IAbilityManager> abilityMgrService;
    static Event event;
    static std::shared_ptr<AppEventSubscriber> subscriber_;
};

Event AmsConfigurationUpdatedTest::event = Event();
sptr<IAbilityManager> AmsConfigurationUpdatedTest::abilityMgrService = nullptr;
std::shared_ptr<AmsConfigurationUpdatedTest::AppEventSubscriber> AmsConfigurationUpdatedTest::subscriber_ = nullptr;

void AmsConfigurationUpdatedTest::AppEventSubscriber::OnReceiveEvent(const CommonEventData &data)
{
    GTEST_LOG_(INFO) << "\nOnReceiveEvent: event====>" << data.GetWant().GetAction();
    GTEST_LOG_(INFO) << "\nOnReceiveEvent: data=====>" << data.GetData();
    GTEST_LOG_(INFO) << "\nOnReceiveEvent: code=====>" << data.GetCode();
    if (data.GetData() == "Updated") {
        switch (data.GetCode()) {
            case MAIN_ABILITY_CODE:
                g_mainAbilityUpdateTimes++;
                break;
            case SECOND_ABILITY_CODE:
                g_secondAbilityUpdateTimes++;
                break;
            case THIRD_ABILITY_CODE:
                g_thirdAbilityUpdateTimes++;
                break;
            default:
                break;
        }
    }
    if (find(eventList.begin(), eventList.end(), data.GetWant().GetAction()) != eventList.end()) {
        TestCompleted(event, data.GetData(), data.GetCode());
    }
}

int AmsConfigurationUpdatedTest::TestWaitCompleted(
    Event &event, const std::string &eventName, const int code, const int timeout)
{
    GTEST_LOG_(INFO) << "---------->\n\nTestWaitCompleted ====>: " << eventName << " " << code;
    return STAbilityUtil::WaitCompleted(event, eventName, code, timeout);
}

void AmsConfigurationUpdatedTest::TestCompleted(Event &event, const std::string &eventName, const int code)
{
    GTEST_LOG_(INFO) << "----------<\nTestCompleted ====>: " << eventName << " " << code << "\n";
    return STAbilityUtil::Completed(event, eventName, code);
}

void AmsConfigurationUpdatedTest::SetUpTestCase(void)
{
    if (!SubscribeEvent()) {
        GTEST_LOG_(INFO) << "\nSubscribeEvent error====<";
    }
}

void AmsConfigurationUpdatedTest::TearDownTestCase(void)
{
    CommonEventManager::UnSubscribeCommonEvent(subscriber_);
}

void AmsConfigurationUpdatedTest::SetUp(void)
{
    STAbilityUtil::Install(KIT_HAP_NAME);
    sleep(WAIT_SETUP_TIME);
    STAbilityUtil::Install(KIT_SECOND_HAP_NAME);
    sleep(WAIT_SETUP_TIME);
    STAbilityUtil::CleanMsg(event);
}

void AmsConfigurationUpdatedTest::TearDown(void)
{
    STAbilityUtil::Uninstall(KIT_BUNDLE_NAME);
    sleep(WAIT_TEARDOWN_TIME);
    STAbilityUtil::Uninstall(KIT_SECOND_BUNDLE_NAME);
    sleep(WAIT_TEARDOWN_TIME);
    STAbilityUtil::CleanMsg(event);
}

bool AmsConfigurationUpdatedTest::SubscribeEvent()
{
    MatchingSkills matchingSkills;
    for (const auto &e : eventList) {
        matchingSkills.AddEvent(e);
    }
    CommonEventSubscribeInfo subscribeInfo(matchingSkills);
    subscribeInfo.SetPriority(1);
    subscriber_ = std::make_shared<AppEventSubscriber>(subscribeInfo);
    return CommonEventManager::SubscribeCommonEvent(subscriber_);
}

void AmsConfigurationUpdatedTest::SetDefaultConfig()
{
    AppExecFwk::Configuration configuration;
    configuration.AddItem(GlobalConfigurationKey::SYSTEM_LANGUAGE, "ZH-HANS");
    abilityMgrService->UpdateConfiguration(configuration);
    (void)TestWaitCompleted(event, "Updated", MAIN_ABILITY_CODE, WAIT_LAUNCHER_TIME);

    AppExecFwk::Configuration configuration2;
    configuration2.AddItem(GlobalConfigurationKey::SYSTEM_ORIENTATION, "vertical");
    abilityMgrService->UpdateConfiguration(configuration2);
    (void)TestWaitCompleted(event, "Updated", MAIN_ABILITY_CODE, WAIT_LAUNCHER_TIME);
    STAbilityUtil::CleanMsg(event);
}

/**
 * @tc.number    : 0100
 * @tc.name      : AMS_UpdateConfiguration_0100
 * @tc.desc      : Verify whether the results of the orientation function of the system configuration concerned by
 * capability are correct.
 */
HWTEST_F(AmsConfigurationUpdatedTest, AMS_UpdateConfiguration_0100, Function | MediumTest | Level1)
{
    GTEST_LOG_(INFO) << "==========>\nAmsConfigurationUpdatedTest AMS_UpdateConfiguration_0100 start";
    MAP_STR_STR params;

    Want want = STAbilityUtil::MakeWant("device", MAIN_ABILITY, KIT_BUNDLE_NAME, params);
    ErrCode eCode = STAbilityUtil::StartAbility(want, abilityMgrService, WAIT_TIME);
    GTEST_LOG_(INFO) << "\nStartAbility ====>> " << eCode;
    g_tempDataStr = "OnStartOnActive";
    EXPECT_EQ(TestWaitCompleted(event, "OnStartOnActive", MAIN_ABILITY_CODE, WAIT_LAUNCHER_TIME), 0);
    sleep(WAIT_ONACTIVE_TIME);
    SetDefaultConfig();

    AppExecFwk::Configuration configuration;
    configuration.AddItem(GlobalConfigurationKey::SYSTEM_LANGUAGE, "fr_FR");
    abilityMgrService->UpdateConfiguration(configuration);
    g_tempDataStr = "Updated";
    EXPECT_EQ(TestWaitCompleted(event, "fr_FR", MAIN_ABILITY_CODE, WAIT_LAUNCHER_TIME), 0);

    GTEST_LOG_(INFO) << "\nAmsConfigurationUpdatedTest AMS_UpdateConfiguration_0100 end=========<";
}

/**
 * @tc.number    : 0200
 * @tc.name      : AMS_UpdateConfiguration_0200
 * @tc.desc      : Verify whether the results of the orientation, locale function of the system configuration concerned
 * by capability are correct.
 */

HWTEST_F(AmsConfigurationUpdatedTest, AMS_UpdateConfiguration_0200, Function | MediumTest | Level1)
{
    GTEST_LOG_(INFO) << "==========>\nAmsConfigurationUpdatedTest AMS_UpdateConfiguration_0200 start";
    MAP_STR_STR params;

    Want want = STAbilityUtil::MakeWant("device", MAIN_ABILITY, KIT_BUNDLE_NAME, params);
    ErrCode eCode = STAbilityUtil::StartAbility(want, abilityMgrService, WAIT_TIME);
    GTEST_LOG_(INFO) << "\nStartAbility ====>> " << eCode;

    g_tempDataStr = "OnStartOnActive";
    EXPECT_EQ(TestWaitCompleted(event, "OnStartOnActive", MAIN_ABILITY_CODE, WAIT_LAUNCHER_TIME), 0);
    sleep(WAIT_ONACTIVE_TIME);
    SetDefaultConfig();

    AppExecFwk::Configuration configuration;
    configuration.AddItem(GlobalConfigurationKey::SYSTEM_LANGUAGE, "ZH-HANS");
    abilityMgrService->UpdateConfiguration(configuration);
    EXPECT_NE(TestWaitCompleted(event, "Updated", MAIN_ABILITY_CODE, WAIT_LAUNCHER_TIME), 0);

    GTEST_LOG_(INFO) << "\nAmsConfigurationUpdatedTest AMS_UpdateConfiguration_0200 end=========<";
}

/**
 * @tc.number    : 0300
 * @tc.name      : AMS_UpdateConfiguration_0300
 * @tc.desc      : Verify whether the results of the orientation, locale, layout function of the system configuration
 * concerned by capability are correct.
 */

HWTEST_F(AmsConfigurationUpdatedTest, AMS_UpdateConfiguration_0300, Function | MediumTest | Level1)
{
    GTEST_LOG_(INFO) << "==========>\nAmsConfigurationUpdatedTest AMS_UpdateConfiguration_0300 start";
    MAP_STR_STR params;

    Want want = STAbilityUtil::MakeWant("device", MAIN_ABILITY, KIT_BUNDLE_NAME, params);
    ErrCode eCode = STAbilityUtil::StartAbility(want, abilityMgrService, WAIT_TIME);
    GTEST_LOG_(INFO) << "\nStartAbility ====>> " << eCode;

    g_tempDataStr = "OnStartOnActive";
    EXPECT_EQ(TestWaitCompleted(event, "OnStartOnActive", MAIN_ABILITY_CODE, WAIT_LAUNCHER_TIME), 0);
    sleep(WAIT_ONACTIVE_TIME);
    SetDefaultConfig();

    AppExecFwk::Configuration configuration;
    configuration.AddItem(GlobalConfigurationKey::SYSTEM_ORIENTATION, "horizontal");
    abilityMgrService->UpdateConfiguration(configuration);
    EXPECT_EQ(TestWaitCompleted(event, "horizontal", MAIN_ABILITY_CODE, WAIT_LAUNCHER_TIME), 0);

    GTEST_LOG_(INFO) << "\nAmsConfigurationUpdatedTest AMS_UpdateConfiguration_0300 end=========<";
}

/**
 * @tc.number    : 0400
 * @tc.name      : AMS_UpdateConfiguration_0400
 * @tc.desc      : Verify whether the results of the orientation, locale, layout,fontSize function of the system
 * configuration concerned by capability are correct.
 */

HWTEST_F(AmsConfigurationUpdatedTest, AMS_UpdateConfiguration_0400, Function | MediumTest | Level1)
{
    GTEST_LOG_(INFO) << "==========>\nAmsConfigurationUpdatedTest AMS_UpdateConfiguration_0400 start";
    MAP_STR_STR params;

    Want want = STAbilityUtil::MakeWant("device", MAIN_ABILITY, KIT_BUNDLE_NAME, params);
    ErrCode eCode = STAbilityUtil::StartAbility(want, abilityMgrService, WAIT_TIME);
    GTEST_LOG_(INFO) << "\nStartAbility ====>> " << eCode;

    g_tempDataStr = "OnStartOnActive";
    EXPECT_EQ(TestWaitCompleted(event, "OnStartOnActive", MAIN_ABILITY_CODE, WAIT_LAUNCHER_TIME), 0);
    sleep(WAIT_ONACTIVE_TIME);
    SetDefaultConfig();

    AppExecFwk::Configuration configuration;
    configuration.AddItem(GlobalConfigurationKey::SYSTEM_ORIENTATION, "vertical");
    abilityMgrService->UpdateConfiguration(configuration);

    g_tempDataStr = "Updated";
    EXPECT_NE(TestWaitCompleted(event, "Updated", MAIN_ABILITY_CODE, WAIT_LAUNCHER_TIME), 0);

    GTEST_LOG_(INFO) << "\nAmsConfigurationUpdatedTest AMS_UpdateConfiguration_0400 end=========<";
}

/**
 * @tc.number    : 0500
 * @tc.name      : AMS_UpdateConfiguration_0500
 * @tc.desc      : Verify whether the results of the orientation, locale, layout,fontSize function of the system
 * configuration concerned by capability are correct.
 */

HWTEST_F(AmsConfigurationUpdatedTest, AMS_UpdateConfiguration_0500, Function | MediumTest | Level1)
{
    GTEST_LOG_(INFO) << "==========>\nAmsConfigurationUpdatedTest AMS_UpdateConfiguration_0500 start";
    MAP_STR_STR params;

    Want want = STAbilityUtil::MakeWant("device", MAIN_ABILITY, KIT_BUNDLE_NAME, params);
    ErrCode eCode = STAbilityUtil::StartAbility(want, abilityMgrService, WAIT_TIME);
    GTEST_LOG_(INFO) << "\nStartAbility ====>> " << eCode;
    EXPECT_EQ(TestWaitCompleted(event, "OnStartOnActive", MAIN_ABILITY_CODE, WAIT_LAUNCHER_TIME), 0);
    SetDefaultConfig();
    STAbilityUtil::PublishEvent(g_EVENT_REQU_MAIN, MAIN_ABILITY_CODE, "StartNextAbility");
    EXPECT_EQ(TestWaitCompleted(event, "OnStartOnActive", SECOND_ABILITY_CODE, WAIT_LAUNCHER_TIME), 0);

    Want want2 = STAbilityUtil::MakeWant("device", MAIN_ABILITY, KIT_SECOND_BUNDLE_NAME, params);
    eCode = STAbilityUtil::StartAbility(want2, abilityMgrService, WAIT_TIME);
    GTEST_LOG_(INFO) << "\nStartSecondApp,ThirdAbility ====>> " << eCode;
    EXPECT_EQ(TestWaitCompleted(event, "OnStartOnActive", THIRD_ABILITY_CODE, WAIT_LAUNCHER_TIME), 0);
    sleep(WAIT_ONACTIVE_TIME);

    AppExecFwk::Configuration configuration;
    configuration.AddItem(GlobalConfigurationKey::SYSTEM_LANGUAGE, "fr_FR");
    abilityMgrService->UpdateConfiguration(configuration);
    EXPECT_EQ(TestWaitCompleted(event, "fr_FR", MAIN_ABILITY_CODE, WAIT_LAUNCHER_TIME), 0);
    EXPECT_EQ(TestWaitCompleted(event, "fr_FR", SECOND_ABILITY_CODE, WAIT_LAUNCHER_TIME), 0);
    EXPECT_EQ(TestWaitCompleted(event, "fr_FR", THIRD_ABILITY_CODE, WAIT_LAUNCHER_TIME), 0);

    GTEST_LOG_(INFO) << "\nAmsConfigurationUpdatedTest AMS_UpdateConfiguration_0500 end=========<";
}

/**
 * @tc.number    : 0600
 * @tc.name      : AMS_UpdateConfiguration_0600
 * @tc.desc      : Verify whether the results of the orientation, locale, layout,fontSize function of the system
 * configuration concerned by capability are correct.
 */

HWTEST_F(AmsConfigurationUpdatedTest, AMS_UpdateConfiguration_0600, Function | MediumTest | Level1)
{
    GTEST_LOG_(INFO) << "==========>\nAmsConfigurationUpdatedTest AMS_UpdateConfiguration_0600 start";
    MAP_STR_STR params;

    Want want = STAbilityUtil::MakeWant("device", MAIN_ABILITY, KIT_BUNDLE_NAME, params);
    ErrCode eCode = STAbilityUtil::StartAbility(want, abilityMgrService, WAIT_TIME);
    GTEST_LOG_(INFO) << "\nStartAbility ====>> " << eCode;
    EXPECT_EQ(TestWaitCompleted(event, "OnStartOnActive", MAIN_ABILITY_CODE, WAIT_LAUNCHER_TIME), 0);
    SetDefaultConfig();

    STAbilityUtil::PublishEvent(g_EVENT_REQU_MAIN, MAIN_ABILITY_CODE, "StartNextAbility");
    EXPECT_EQ(TestWaitCompleted(event, "OnStartOnActive", SECOND_ABILITY_CODE, WAIT_LAUNCHER_TIME), 0);

    Want want2 = STAbilityUtil::MakeWant("device", MAIN_ABILITY, KIT_SECOND_BUNDLE_NAME, params);
    eCode = STAbilityUtil::StartAbility(want2, abilityMgrService, WAIT_TIME);
    GTEST_LOG_(INFO) << "\nStartSecondAbility ====>> " << eCode;
    EXPECT_EQ(TestWaitCompleted(event, "OnStartOnActive", THIRD_ABILITY_CODE, WAIT_LAUNCHER_TIME), 0);
    sleep(WAIT_ONACTIVE_TIME);

    AppExecFwk::Configuration configuration;
    configuration.AddItem(GlobalConfigurationKey::SYSTEM_ORIENTATION, "horizontal");
    abilityMgrService->UpdateConfiguration(configuration);
    EXPECT_EQ(TestWaitCompleted(event, "horizontal", MAIN_ABILITY_CODE, WAIT_LAUNCHER_TIME), 0);
    EXPECT_EQ(TestWaitCompleted(event, "horizontal", SECOND_ABILITY_CODE, WAIT_LAUNCHER_TIME), 0);
    EXPECT_EQ(TestWaitCompleted(event, "horizontal", THIRD_ABILITY_CODE, WAIT_LAUNCHER_TIME), 0);

    GTEST_LOG_(INFO) << "\nAmsConfigurationUpdatedTest AMS_UpdateConfiguration_0600 end=========<";
}

/**
 * @tc.number    : 0700
 * @tc.name      : AMS_UpdateConfiguration_0700
 * @tc.desc      : Verify whether the results of the orientation, locale, layout,fontSize function of the system
 * configuration concerned by capability are correct.
 */

HWTEST_F(AmsConfigurationUpdatedTest, AMS_UpdateConfiguration_0700, Function | MediumTest | Level1)
{
    GTEST_LOG_(INFO) << "==========>\nAmsConfigurationUpdatedTest AMS_UpdateConfiguration_0700 start";
    MAP_STR_STR params;

    Want want = STAbilityUtil::MakeWant("device", MAIN_ABILITY, KIT_BUNDLE_NAME, params);
    ErrCode eCode = STAbilityUtil::StartAbility(want, abilityMgrService, WAIT_TIME);
    GTEST_LOG_(INFO) << "\nStartAbility ====>> " << eCode;
    EXPECT_EQ(TestWaitCompleted(event, "OnStartOnActive", MAIN_ABILITY_CODE, WAIT_LAUNCHER_TIME), 0);
    SetDefaultConfig();

    STAbilityUtil::PublishEvent(g_EVENT_REQU_MAIN, MAIN_ABILITY_CODE, "StartNextAbilityWithBlockFlag");
    EXPECT_EQ(TestWaitCompleted(event, "OnStartOnActive", SECOND_ABILITY_CODE, WAIT_LAUNCHER_TIME), 0);

    Want want2 = STAbilityUtil::MakeWant("device", MAIN_ABILITY, KIT_SECOND_BUNDLE_NAME, params);
    eCode = STAbilityUtil::StartAbility(want2, abilityMgrService, WAIT_TIME);
    GTEST_LOG_(INFO) << "\nStartSecondApp,ThirdAbility ====>> " << eCode;
    EXPECT_EQ(TestWaitCompleted(event, "OnStartOnActive", THIRD_ABILITY_CODE, WAIT_LAUNCHER_TIME), 0);
    sleep(WAIT_ONACTIVE_TIME);

    AppExecFwk::Configuration configuration;
    configuration.AddItem(GlobalConfigurationKey::SYSTEM_LANGUAGE, "fr_FR");
    GTEST_LOG_(INFO) << "\nUpdateConfiguration First Time ====>> ";
    abilityMgrService->UpdateConfiguration(configuration);
    AppExecFwk::Configuration configuration2;
    configuration2.AddItem(GlobalConfigurationKey::SYSTEM_LANGUAGE, "en_US");
    GTEST_LOG_(INFO) << "\nUpdateConfiguration Second Time ====>> ";
    abilityMgrService->UpdateConfiguration(configuration2);

    EXPECT_EQ(TestWaitCompleted(event, "fr_FR", THIRD_ABILITY_CODE, WAIT_BLOCKUPDATE_TIME), 0);
    EXPECT_EQ(TestWaitCompleted(event, "fr_FR", SECOND_ABILITY_CODE, WAIT_BLOCKUPDATE_TIME), 0);
    EXPECT_EQ(TestWaitCompleted(event, "fr_FR", MAIN_ABILITY_CODE, WAIT_BLOCKUPDATE_TIME), 0);

    EXPECT_EQ(TestWaitCompleted(event, "en_US", THIRD_ABILITY_CODE, WAIT_BLOCKUPDATE_TIME), 0);
    EXPECT_EQ(TestWaitCompleted(event, "en_US", SECOND_ABILITY_CODE, WAIT_BLOCKUPDATE_TIME), 0);
    EXPECT_EQ(TestWaitCompleted(event, "en_US", MAIN_ABILITY_CODE, WAIT_BLOCKUPDATE_TIME), 0);

    GTEST_LOG_(INFO) << "\nAmsConfigurationUpdatedTest AMS_UpdateConfiguration_0700 end=========<";
}

/**
 * @tc.number    : 0800
 * @tc.name      : AMS_UpdateConfiguration_0800
 * @tc.desc      : Verify whether the results of the orientation, locale, layout,fontSize function of the system
 * configuration concerned by capability are correct.
 */

HWTEST_F(AmsConfigurationUpdatedTest, AMS_UpdateConfiguration_0800, Function | MediumTest | Level1)
{
    GTEST_LOG_(INFO) << "==========>\nAmsConfigurationUpdatedTest AMS_UpdateConfiguration_0800 start";
    MAP_STR_STR params;

    Want want = STAbilityUtil::MakeWant("device", MAIN_ABILITY, KIT_BUNDLE_NAME, params);
    ErrCode eCode = STAbilityUtil::StartAbility(want, abilityMgrService, WAIT_TIME);
    GTEST_LOG_(INFO) << "\nStartAbility ====>> " << eCode;
    EXPECT_EQ(TestWaitCompleted(event, "OnStartOnActive", MAIN_ABILITY_CODE, WAIT_LAUNCHER_TIME), 0);
    SetDefaultConfig();

    STAbilityUtil::PublishEvent(g_EVENT_REQU_MAIN, MAIN_ABILITY_CODE, "StartNextAbilityWithBlockFlag");
    EXPECT_EQ(TestWaitCompleted(event, "OnStartOnActive", SECOND_ABILITY_CODE, WAIT_LAUNCHER_TIME), 0);

    Want want2 = STAbilityUtil::MakeWant("device", MAIN_ABILITY, KIT_SECOND_BUNDLE_NAME, params);
    eCode = STAbilityUtil::StartAbility(want2, abilityMgrService, WAIT_TIME);
    GTEST_LOG_(INFO) << "\nStartSecondApp,ThirdAbility ====>> " << eCode;
    EXPECT_EQ(TestWaitCompleted(event, "OnStartOnActive", THIRD_ABILITY_CODE, WAIT_LAUNCHER_TIME), 0);
    sleep(WAIT_ONACTIVE_TIME);

    AppExecFwk::Configuration configuration;
    configuration.AddItem(GlobalConfigurationKey::SYSTEM_ORIENTATION, "horizontal");
    GTEST_LOG_(INFO) << "\nUpdateConfiguration First Time ====>> ";
    abilityMgrService->UpdateConfiguration(configuration);
    AppExecFwk::Configuration configuration2;
    configuration2.AddItem(GlobalConfigurationKey::SYSTEM_ORIENTATION, "vertical");
    GTEST_LOG_(INFO) << "\nUpdateConfiguration Second Time ====>> ";
    abilityMgrService->UpdateConfiguration(configuration2);

    EXPECT_EQ(TestWaitCompleted(event, "horizontal", THIRD_ABILITY_CODE, WAIT_BLOCKUPDATE_TIME), 0);
    EXPECT_EQ(TestWaitCompleted(event, "horizontal", SECOND_ABILITY_CODE, WAIT_BLOCKUPDATE_TIME), 0);
    EXPECT_EQ(TestWaitCompleted(event, "horizontal", MAIN_ABILITY_CODE, WAIT_BLOCKUPDATE_TIME), 0);

    EXPECT_EQ(TestWaitCompleted(event, "vertical", THIRD_ABILITY_CODE, WAIT_BLOCKUPDATE_TIME), 0);
    EXPECT_EQ(TestWaitCompleted(event, "vertical", SECOND_ABILITY_CODE, WAIT_BLOCKUPDATE_TIME), 0);
    EXPECT_EQ(TestWaitCompleted(event, "vertical", MAIN_ABILITY_CODE, WAIT_BLOCKUPDATE_TIME), 0);

    GTEST_LOG_(INFO) << "\nAmsConfigurationUpdatedTest AMS_UpdateConfiguration_0800 end=========<";
}

/**
 * @tc.number    : 0900
 * @tc.name      : AMS_UpdateConfiguration_0900
 * @tc.desc      : Verify whether the results of the orientation, locale, layout,fontSize function of the system
 * configuration concerned by capability are correct.
 */

HWTEST_F(AmsConfigurationUpdatedTest, AMS_UpdateConfiguration_0900, Function | MediumTest | Level1)
{
    GTEST_LOG_(INFO) << "==========>\nAmsConfigurationUpdatedTest AMS_UpdateConfiguration_0900 start";
    MAP_STR_STR params;

    Want want = STAbilityUtil::MakeWant("device", MAIN_ABILITY, KIT_BUNDLE_NAME, params);
    ErrCode eCode = STAbilityUtil::StartAbility(want, abilityMgrService, WAIT_TIME);
    GTEST_LOG_(INFO) << "\nStartAbility ====>> " << eCode;
    EXPECT_EQ(TestWaitCompleted(event, "OnStartOnActive", MAIN_ABILITY_CODE, WAIT_LAUNCHER_TIME), 0);
    SetDefaultConfig();

    STAbilityUtil::PublishEvent(g_EVENT_REQU_MAIN, MAIN_ABILITY_CODE, "StartNextAbility");
    EXPECT_EQ(TestWaitCompleted(event, "OnStartOnActive", SECOND_ABILITY_CODE, WAIT_LAUNCHER_TIME), 0);

    Want want2 = STAbilityUtil::MakeWant("device", MAIN_ABILITY, KIT_SECOND_BUNDLE_NAME, params);
    eCode = STAbilityUtil::StartAbility(want2, abilityMgrService, WAIT_TIME);
    GTEST_LOG_(INFO) << "\nStartSecondApp,ThirdAbility ====>> " << eCode;
    EXPECT_EQ(TestWaitCompleted(event, "OnStartOnActive", THIRD_ABILITY_CODE, WAIT_LAUNCHER_TIME), 0);
    sleep(WAIT_ONACTIVE_TIME);

    g_mainAbilityUpdateTimes = 0;
    g_secondAbilityUpdateTimes = 0;
    g_thirdAbilityUpdateTimes = 0;
    AppExecFwk::Configuration configuration;
    configuration.AddItem(GlobalConfigurationKey::SYSTEM_LANGUAGE, "fr_FR");
    AppExecFwk::Configuration configuration2;
    configuration2.AddItem(GlobalConfigurationKey::SYSTEM_LANGUAGE, "en_US");
    for (int iLoop = 0; iLoop < TEST_TIMES / 2; iLoop++) {
        abilityMgrService->UpdateConfiguration(configuration);
        abilityMgrService->UpdateConfiguration(configuration2);
    }
    sleep(WAIT_LAUNCHER_TIME);

    EXPECT_EQ(g_mainAbilityUpdateTimes, TEST_TIMES);
    EXPECT_EQ(g_secondAbilityUpdateTimes, TEST_TIMES);
    EXPECT_EQ(g_thirdAbilityUpdateTimes, TEST_TIMES);

    GTEST_LOG_(INFO) << "\nAmsConfigurationUpdatedTest AMS_UpdateConfiguration_0900 end=========<";
}

/**
 * @tc.number    : 1000
 * @tc.name      : AMS_UpdateConfiguration_1000
 * @tc.desc      : Verify whether the results of the orientation, locale, layout,fontSize function of the system
 * configuration concerned by capability are correct.
 */

HWTEST_F(AmsConfigurationUpdatedTest, AMS_UpdateConfiguration_1000, Function | MediumTest | Level1)
{
    GTEST_LOG_(INFO) << "==========>\nAmsConfigurationUpdatedTest AMS_UpdateConfiguration_1000 start";
    MAP_STR_STR params;

    Want want = STAbilityUtil::MakeWant("device", MAIN_ABILITY, KIT_BUNDLE_NAME, params);
    ErrCode eCode = STAbilityUtil::StartAbility(want, abilityMgrService, WAIT_TIME);
    GTEST_LOG_(INFO) << "\nStartAbility ====>> " << eCode;
    EXPECT_EQ(TestWaitCompleted(event, "OnStartOnActive", MAIN_ABILITY_CODE, WAIT_LAUNCHER_TIME), 0);
    SetDefaultConfig();

    STAbilityUtil::PublishEvent(g_EVENT_REQU_MAIN, MAIN_ABILITY_CODE, "StartNextAbility");
    EXPECT_EQ(TestWaitCompleted(event, "OnStartOnActive", SECOND_ABILITY_CODE, WAIT_LAUNCHER_TIME), 0);

    Want want2 = STAbilityUtil::MakeWant("device", MAIN_ABILITY, KIT_SECOND_BUNDLE_NAME, params);
    eCode = STAbilityUtil::StartAbility(want2, abilityMgrService, WAIT_TIME);
    GTEST_LOG_(INFO) << "\nStartSecondApp,ThirdAbility ====>> " << eCode;
    EXPECT_EQ(TestWaitCompleted(event, "OnStartOnActive", THIRD_ABILITY_CODE, WAIT_LAUNCHER_TIME), 0);
    sleep(WAIT_ONACTIVE_TIME);

    g_mainAbilityUpdateTimes = 0;
    g_secondAbilityUpdateTimes = 0;
    g_thirdAbilityUpdateTimes = 0;
    AppExecFwk::Configuration configuration;
    configuration.AddItem(GlobalConfigurationKey::SYSTEM_ORIENTATION, "horizontal");
    AppExecFwk::Configuration configuration2;
    configuration2.AddItem(GlobalConfigurationKey::SYSTEM_ORIENTATION, "vertical");
    for (int iLoop = 0; iLoop < TEST_TIMES / 2; iLoop++) {
        abilityMgrService->UpdateConfiguration(configuration);
        abilityMgrService->UpdateConfiguration(configuration2);
    }
    sleep(WAIT_LAUNCHER_TIME);

    EXPECT_EQ(g_mainAbilityUpdateTimes, TEST_TIMES);
    EXPECT_EQ(g_secondAbilityUpdateTimes, TEST_TIMES);
    EXPECT_EQ(g_thirdAbilityUpdateTimes, TEST_TIMES);

    GTEST_LOG_(INFO) << "\nAmsConfigurationUpdatedTest AMS_UpdateConfiguration_1000 end=========<";
}
}  // namespace