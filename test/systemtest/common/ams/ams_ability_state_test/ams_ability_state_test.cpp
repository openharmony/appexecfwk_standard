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
#include "ability_lifecycle_executor.h"
#include "common_event_manager.h"
#include "st_ability_util.h"
#include "testConfigParser.h"

namespace {
using namespace OHOS;
using namespace OHOS::AppExecFwk;
using namespace OHOS::AAFwk;
using namespace OHOS::EventFwk;
using namespace OHOS::STtools;
using namespace testing::ext;
using namespace STABUtil;
namespace {
const std::string bundleName = "com.ohos.amsst.fwkAbilityState";
const std::string firstAbilityName = "FwkAbilityStateMain";
const std::string secondAbilityname = "FwkAbilityStateSecond";
std::string FwkAbilityState_Event_Resp_A = "resp_com_ohos_amsst_FwkAbilityStateA";
std::string FwkAbilityState_Event_Requ_A = "requ_com_ohos_amsst_FwkAbilityStateA";
std::string FwkAbilityState_Event_Requ_B = "requ_com_ohos_amsst_FwkAbilityStateB";
constexpr int onActive = 1;
constexpr int onBackground = 2;
constexpr int onStop = 3;
constexpr int WAIT_TIME = 100;
constexpr int DELAY = 10;
}
class AmsAbilityStateTest : public testing::Test {
public:
    static void SetUpTestCase(void);
    static void TearDownTestCase(void);
    void SetUp();
    void TearDown();

    static bool SubscribeEvent();
    class AppEventSubscriber : public OHOS::EventFwk::CommonEventSubscriber {
    public:
        explicit AppEventSubscriber(const OHOS::EventFwk::CommonEventSubscribeInfo &sp) : CommonEventSubscriber(sp) {};
        virtual void OnReceiveEvent(const OHOS::EventFwk::CommonEventData &data) override;
        ~AppEventSubscriber() = default;
    };

    void StartAbility(const std::string &abilityName, const std::string &bundleName);
    static OHOS::sptr<OHOS::AAFwk::IAbilityManager> abilityMs;
    static OHOS::STtools::Event event;
    static OHOS::StressTestLevel stLevel_;
    static std::shared_ptr<AppEventSubscriber> subscriber_;
};

Event AmsAbilityStateTest::event = OHOS::STtools::Event();
sptr<IAbilityManager> AmsAbilityStateTest::abilityMs = nullptr;
StressTestLevel AmsAbilityStateTest::stLevel_ {};
std::shared_ptr<AmsAbilityStateTest::AppEventSubscriber> AmsAbilityStateTest::subscriber_ = nullptr;

void AmsAbilityStateTest::SetUpTestCase(void)
{
    if (!SubscribeEvent()) {
        GTEST_LOG_(INFO) << "SubscribeEvent error";
    }
    TestConfigParser tcp;
    tcp.ParseFromFile4StressTest(STRESS_TEST_CONFIG_FILE_PATH, stLevel_);
    std::cout << "stress test level : "
              << "AMS : " << stLevel_.AMSLevel << " "
              << "BMS : " << stLevel_.BMSLevel << " "
              << "CES : " << stLevel_.CESLevel << std::endl;
    stLevel_.AMSLevel = 1;
}
void AmsAbilityStateTest::TearDownTestCase(void)
{
    CommonEventManager::UnSubscribeCommonEvent(subscriber_);
}

void AmsAbilityStateTest::SetUp()
{
    std::vector<std::string> hapNameList = {"fwkAbilityState"};
    STAbilityUtil::InstallHaps(hapNameList);
}

void AmsAbilityStateTest::TearDown()
{
    std::vector<std::string> bundleNameList = {
        bundleName,
    };
    STAbilityUtil::UninstallBundle(bundleNameList);
    STAbilityUtil::CleanMsg(event);
}

void AmsAbilityStateTest::AppEventSubscriber::OnReceiveEvent(const CommonEventData &data)
{
    GTEST_LOG_(INFO) << "OnReceiveEvent: event=" << data.GetWant().GetAction();
    GTEST_LOG_(INFO) << "OnReceiveEvent: data=" << data.GetData();
    GTEST_LOG_(INFO) << "OnReceiveEvent: code=" << data.GetCode();
    STAbilityUtil::Completed(event, data.GetWant().GetAction(), data.GetCode(), data.GetData());
}

bool AmsAbilityStateTest::SubscribeEvent()
{
    std::vector<std::string> eventList = {
        FwkAbilityState_Event_Resp_A,
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

void AmsAbilityStateTest::StartAbility(const std::string &abilityName, const std::string &bundleName)
{
    std::map<std::string, std::string> params;
    Want want = STAbilityUtil::MakeWant("device", abilityName, bundleName, params);
    ErrCode result = STAbilityUtil::StartAbility(want, abilityMs, WAIT_TIME);
    GTEST_LOG_(INFO) << "AmsAbilityStateTest::StartAbility : " << result;
    EXPECT_EQ(0, STAbilityUtil::WaitCompleted(event, bundleName, 0, DELAY));
}

/**
 * @tc.number    : FWK_SaveAbilityState_0100
 * @tc.name      : onSaveAbilityState called when home event
 * @tc.desc      : onSaveAbilityState called when home event
 */
HWTEST_F(AmsAbilityStateTest, FWK_SaveAbilityState_0100, Function | MediumTest | Level1)
{
    std::string data;
    Want wantEntity;
    wantEntity.AddEntity(Want::FLAG_HOME_INTENT_FROM_SYSTEM);
    bool result = false;
    for (int i = 0; i < stLevel_.AMSLevel; i++) {
        // start ability
        StartAbility(firstAbilityName, bundleName);
        // home
        STAbilityUtil::StartAbility(wantEntity, abilityMs);
        // ability state BACKGROUND
        EXPECT_EQ(0, STAbilityUtil::WaitCompleted(event, FwkAbilityState_Event_Resp_A, onBackground, DELAY));
        data = STAbilityUtil::GetData(event, FwkAbilityState_Event_Resp_A, onBackground);
        result = data.compare("OnInactiveOnSaveAbilityStateOnBackground") == 0;
        EXPECT_TRUE(result);
        if (!result && i > 0) {
            GTEST_LOG_(INFO) << "FWK_SaveAbilityState_0100 : " << i;
            break;
        }
    }
}

/**
 * @tc.number    : FWK_SaveAbilityState_0200
 * @tc.name      : onSaveAbilityState called when system property changes
 * @tc.desc      : onSaveAbilityState called when system property changes
 */
HWTEST_F(AmsAbilityStateTest, FWK_SaveAbilityState_0200, Function | MediumTest | Level1)
{
    StartAbility(firstAbilityName, bundleName);
    std::string data;
    DummyConfiguration configuration {"orientation#"};
    bool result = false;
    for (int i = 0; i < stLevel_.AMSLevel; i++) {
        abilityMs->UpdateConfiguration(configuration);
        EXPECT_EQ(0, STAbilityUtil::WaitCompleted(event, FwkAbilityState_Event_Resp_A, onStop, DELAY));
        data = STAbilityUtil::GetData(event, FwkAbilityState_Event_Resp_A, onStop);
        result = data.compare("OnInactiveOnSaveAbilityStateOnBackgroundOnStop") == 0;
        EXPECT_TRUE(result);
        if (!result && i > 0) {
            GTEST_LOG_(INFO) << "FWK_SaveAbilityState_0200 : " << i;
            break;
        }
    }
}

/**
 * @tc.number    : FWK_SaveAbilityState_0300
 * @tc.name      : onSaveAbilityState called when start another ability
 * @tc.desc      : onSaveAbilityState called when start another ability
 */
HWTEST_F(AmsAbilityStateTest, FWK_SaveAbilityState_0300, Function | MediumTest | Level1)
{
    std::string eventData = "StartNextAbility";
    std::string resultData;
    bool result = false;
    for (int i = 0; i < stLevel_.AMSLevel; i++) {
        StartAbility(firstAbilityName, bundleName);
        STAbilityUtil::PublishEvent(FwkAbilityState_Event_Requ_A, onActive, eventData);
        EXPECT_EQ(0, STAbilityUtil::WaitCompleted(event, FwkAbilityState_Event_Resp_A, onBackground, DELAY));
        resultData = STAbilityUtil::GetData(event, FwkAbilityState_Event_Resp_A, onBackground);
        result = resultData.compare("OnInactiveOnSaveAbilityStateOnBackground") == 0;
        EXPECT_TRUE(result);
        if (!result && i > 0) {
            GTEST_LOG_(INFO) << "FWK_SaveAbilityState_0300 : " << i;
            break;
        }
    }
}

/**
 * @tc.number    : FWK_RestoreAbilityState_0100
 * @tc.name      : onRestoreAbilityState called when restart after home event
 * @tc.desc      : onRestoreAbilityState called when restart after home event
 */
HWTEST_F(AmsAbilityStateTest, FWK_RestoreAbilityState_0100, Function | MediumTest | Level1)
{
    StartAbility(firstAbilityName, bundleName);
    std::string data;
    Want wantEntity;
    wantEntity.AddEntity(Want::FLAG_HOME_INTENT_FROM_SYSTEM);
    bool result = false;
    for (int i = 0; i < stLevel_.AMSLevel; i++) {
        // home
        STAbilityUtil::StartAbility(wantEntity, abilityMs);
        // ability state BACKGROUND
        EXPECT_EQ(0, STAbilityUtil::WaitCompleted(event, FwkAbilityState_Event_Resp_A, onBackground, DELAY));
        // restart ability
        StartAbility(firstAbilityName, bundleName);
        EXPECT_EQ(0, STAbilityUtil::WaitCompleted(event, FwkAbilityState_Event_Resp_A, onActive, DELAY));
        data = STAbilityUtil::GetData(event, FwkAbilityState_Event_Resp_A, onActive);
        result = data.compare("OnForegroundOnRestoreAbilityStateOnActive") == 0;
        EXPECT_TRUE(result);
        if (!result && i > 0) {
            GTEST_LOG_(INFO) << "FWK_RestoreAbilityState_0100 : " << i;
            break;
        }
    }
}

/**
 * @tc.number    : FWK_RestoreAbilityState_0200
 * @tc.name      : onRestoreAbilityState called when system property changes
 * @tc.desc      : onRestoreAbilityState called when system property changes
 */
HWTEST_F(AmsAbilityStateTest, FWK_RestoreAbilityState_0200, Function | MediumTest | Level1)
{
    StartAbility(firstAbilityName, bundleName);
    std::string data;
    DummyConfiguration configuration {"orientation#"};
    bool result = false;
    for (int i = 0; i < stLevel_.AMSLevel; i++) {
        abilityMs->UpdateConfiguration(configuration);
        EXPECT_EQ(0, STAbilityUtil::WaitCompleted(event, FwkAbilityState_Event_Resp_A, onActive, DELAY));
        data = STAbilityUtil::GetData(event, FwkAbilityState_Event_Resp_A, onActive);
        result = data.compare("OnStartOnRestoreAbilityStateOnActive") == 0;
        EXPECT_TRUE(result);
        if (!result && i > 0) {
            GTEST_LOG_(INFO) << "FWK_RestoreAbilityState_0200 : " << i;
            break;
        }
    }
}

/**
 * @tc.number    : FWK_RestoreAbilityState_0300
 * @tc.name      : onRestoreAbilityState called when back event after start another ability
 * @tc.desc      : onRestoreAbilityState called when back event after start another ability
 */
HWTEST_F(AmsAbilityStateTest, FWK_RestoreAbilityState_0300, Function | MediumTest | Level1)
{
    StartAbility(firstAbilityName, bundleName);
    std::string eventData;
    std::string resultData;
    bool result = false;
    for (int i = 0; i < stLevel_.AMSLevel; i++) {
        eventData = "StartNextAbility";
        STAbilityUtil::PublishEvent(FwkAbilityState_Event_Requ_A, onActive, eventData);
        EXPECT_EQ(0, STAbilityUtil::WaitCompleted(event, FwkAbilityState_Event_Resp_A, onBackground, DELAY));
        eventData = "TerminateSecond";
        STAbilityUtil::PublishEvent(FwkAbilityState_Event_Requ_B, onStop, eventData);
        resultData = STAbilityUtil::GetData(event, FwkAbilityState_Event_Resp_A, onActive);
        result = resultData.compare("OnForegroundOnRestoreAbilityStateOnActive") == 0;
        EXPECT_TRUE(result);
        if (!result && i > 0) {
            GTEST_LOG_(INFO) << "FWK_RestoreAbilityState_0300 : " << i;
            break;
        }
    }
}
}  // namespace