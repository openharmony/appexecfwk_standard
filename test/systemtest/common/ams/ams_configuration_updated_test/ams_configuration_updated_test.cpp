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
static const string KIT_HAP_NAME = "amsConfigurationUpdatedTest";
static const string MAIN_ABILITY = "MainAbility";
static constexpr int WAIT_TIME = 1;
static constexpr int WAIT_LAUNCHER_TIME = 5;
static constexpr int WAIT_SETUP_TIME = 1;
static constexpr int WAIT_TEARDOWN_TIME = 1;
static constexpr int WAIT_ONACTIVE_TIME = 5;
static string g_eventMessage = "";
static string g_tempDataStr = "";
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
    STAbilityUtil::CleanMsg(event);
}

void AmsConfigurationUpdatedTest::TearDown(void)
{
    STAbilityUtil::Uninstall(KIT_BUNDLE_NAME);
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

    AppExecFwk::Configuration configuration;
    configuration.AddItem(GlobalConfigurationKey::SYSTEM_LANGUAGE, "ZH-HANS");
    abilityMgrService->UpdateConfiguration(configuration);
    (void)TestWaitCompleted(event, "Updated", MAIN_ABILITY_CODE, WAIT_LAUNCHER_TIME);
    STAbilityUtil::CleanMsg(event);

    AppExecFwk::Configuration configuration2;
    configuration2.AddItem(GlobalConfigurationKey::SYSTEM_LANGUAGE, "fr_FR");
    abilityMgrService->UpdateConfiguration(configuration2);
    g_tempDataStr = "Updated";
    EXPECT_EQ(TestWaitCompleted(event, "Updated", MAIN_ABILITY_CODE, WAIT_LAUNCHER_TIME), 0);

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

    AppExecFwk::Configuration configuration;
    configuration.AddItem(GlobalConfigurationKey::SYSTEM_LANGUAGE, "ZH-HANS");
    abilityMgrService->UpdateConfiguration(configuration);
    (void)TestWaitCompleted(event, "Updated", MAIN_ABILITY_CODE, WAIT_LAUNCHER_TIME);
    STAbilityUtil::CleanMsg(event);

    AppExecFwk::Configuration configuration2;
    configuration2.AddItem(GlobalConfigurationKey::SYSTEM_LANGUAGE, "ZH-HANS");
    abilityMgrService->UpdateConfiguration(configuration2);

    g_tempDataStr = "Updated";
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

    AppExecFwk::Configuration configuration;
    configuration.AddItem(GlobalConfigurationKey::SYSTEM_ORIENTATION, "vertical");
    abilityMgrService->UpdateConfiguration(configuration);
    (void)TestWaitCompleted(event, "Updated", MAIN_ABILITY_CODE, WAIT_LAUNCHER_TIME);
    STAbilityUtil::CleanMsg(event);

    AppExecFwk::Configuration configuration2;
    configuration2.AddItem(GlobalConfigurationKey::SYSTEM_ORIENTATION, "horizontal");
    abilityMgrService->UpdateConfiguration(configuration2);

    g_tempDataStr = "Updated";
    EXPECT_EQ(TestWaitCompleted(event, "Updated", MAIN_ABILITY_CODE, WAIT_LAUNCHER_TIME), 0);

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

    AppExecFwk::Configuration configuration;
    configuration.AddItem(GlobalConfigurationKey::SYSTEM_ORIENTATION, "vertical");
    abilityMgrService->UpdateConfiguration(configuration);
    (void)TestWaitCompleted(event, "Updated", MAIN_ABILITY_CODE, WAIT_LAUNCHER_TIME);
    STAbilityUtil::CleanMsg(event);

    AppExecFwk::Configuration configuration2;
    configuration2.AddItem(GlobalConfigurationKey::SYSTEM_ORIENTATION, "vertical");
    abilityMgrService->UpdateConfiguration(configuration2);

    g_tempDataStr = "Updated";
    EXPECT_NE(TestWaitCompleted(event, "Updated", MAIN_ABILITY_CODE, WAIT_LAUNCHER_TIME), 0);

    GTEST_LOG_(INFO) << "\nAmsConfigurationUpdatedTest AMS_UpdateConfiguration_0400 end=========<";
}
}  // namespace