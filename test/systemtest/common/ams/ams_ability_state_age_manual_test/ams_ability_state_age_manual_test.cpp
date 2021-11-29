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
#include "module_test_dump_util.h"
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
using namespace OHOS::MTUtil;
namespace {
const std::string bundleName = "com.ohos.amsst.fwkAbilityState";
const std::string firstAbilityName = "FwkAbilityStateMain";
const std::string secondAbilityname = "FwkAbilityStateSecond";
static const std::string DUMP_ALL = "-a";
std::string FwkAbilityState_Event_Resp_A = "resp_com_ohos_amsst_FwkAbilityStateA";
std::string FwkAbilityState_Event_Requ_A = "requ_com_ohos_amsst_FwkAbilityStateA";
std::string FwkAbilityState_Event_Resp_B = "resp_com_ohos_amsst_FwkAbilityStateB";
std::string FwkAbilityState_Event_Requ_B = "requ_com_ohos_amsst_FwkAbilityStateB";
constexpr int onActive = 1;
constexpr int onBackground = 2;
constexpr int WAIT_TIME = 500;
constexpr int DELAY = 10;
}  // namespace
class AmsAbilityStateAgeManualTest : public testing::Test {
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

Event AmsAbilityStateAgeManualTest::event = OHOS::STtools::Event();
sptr<IAbilityManager> AmsAbilityStateAgeManualTest::abilityMs = nullptr;
StressTestLevel AmsAbilityStateAgeManualTest::stLevel_ {};
std::shared_ptr<AmsAbilityStateAgeManualTest::AppEventSubscriber> AmsAbilityStateAgeManualTest::subscriber_ = nullptr;

void AmsAbilityStateAgeManualTest::SetUpTestCase(void)
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
void AmsAbilityStateAgeManualTest::TearDownTestCase(void)
{
    CommonEventManager::UnSubscribeCommonEvent(subscriber_);
}

void AmsAbilityStateAgeManualTest::SetUp()
{
    std::vector<std::string> hapNameList = {"fwkAbilityState"};
    STAbilityUtil::InstallHaps(hapNameList);
}

void AmsAbilityStateAgeManualTest::TearDown()
{
    std::vector<std::string> bundleNameList = {
        bundleName,
    };

    sleep(1);
    STAbilityUtil::UninstallBundle(bundleNameList);
    STAbilityUtil::CleanMsg(event);
}

void AmsAbilityStateAgeManualTest::AppEventSubscriber::OnReceiveEvent(const CommonEventData &data)
{
    GTEST_LOG_(INFO) << "OnReceiveEvent: event=" << data.GetWant().GetAction();
    GTEST_LOG_(INFO) << "OnReceiveEvent: data=" << data.GetData();
    GTEST_LOG_(INFO) << "OnReceiveEvent: code=" << data.GetCode();
    STAbilityUtil::Completed(event, data.GetWant().GetAction(), data.GetCode(), data.GetData());
}

bool AmsAbilityStateAgeManualTest::SubscribeEvent()
{
    std::vector<std::string> eventList = {
        FwkAbilityState_Event_Resp_A,
        FwkAbilityState_Event_Resp_B,
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

void AmsAbilityStateAgeManualTest::StartAbility(const std::string &abilityName, const std::string &bundleName)
{
    std::map<std::string, std::string> params;
    Want want = STAbilityUtil::MakeWant("device", abilityName, bundleName, params);
    ErrCode result = STAbilityUtil::StartAbility(want, abilityMs, WAIT_TIME);
    GTEST_LOG_(INFO) << "AmsAbilityStateAgeManualTest::StartAbility : " << result;
    EXPECT_EQ(0, STAbilityUtil::WaitCompleted(event, FwkAbilityState_Event_Resp_A, onActive, DELAY));
}

/**
 * @tc.number    : FWK_RestoreAbilityStateAged_0100
 * @tc.name      : RestoreAbilityState aged
 * @tc.desc      : RestoreAbilityState not called when aging time expired
 */
HWTEST_F(AmsAbilityStateAgeManualTest, FWK_RestoreAbilityStateAged_0100, Function | MediumTest | Level1)
{
    std::string data;
    Want wantEntity;
    wantEntity.AddEntity(Want::FLAG_HOME_INTENT_FROM_SYSTEM);
    std::string crashEvent = "DoCrash";
    bool result = false;

    for (int i = 0; i < stLevel_.AMSLevel; i++) {
        StartAbility(firstAbilityName, bundleName);
        STAbilityUtil::StartAbility(wantEntity, abilityMs);
        EXPECT_EQ(0, STAbilityUtil::WaitCompleted(event, FwkAbilityState_Event_Resp_A, onBackground, DELAY));
        data = STAbilityUtil::GetData(event, FwkAbilityState_Event_Resp_A, onBackground);
        result = data.compare("OnInactiveOnSaveAbilityStateOnBackground") == 0;
        EXPECT_TRUE(result);
        sleep(1);
        EXPECT_EQ(true, STAbilityUtil::PublishEvent(FwkAbilityState_Event_Requ_A, onActive, crashEvent));
        GTEST_LOG_(INFO) << "FWK_RestoreAbilityStateAged_0100 DOCRASH NOTIFY";
        sleep(10);
        StartAbility(firstAbilityName, bundleName);
        data = STAbilityUtil::GetData(event, FwkAbilityState_Event_Resp_A, onActive);
        GTEST_LOG_(INFO) << "FWK_RestoreAbilityStateAged_0100 data= : " << data;
        result = data.compare("OnStartOnNewWantOnActive") == 0;
        EXPECT_TRUE(result);

        if (!result && i > 0) {
            GTEST_LOG_(INFO) << "FWK_RestoreAbilityStateAged_0100 : " << i;
            break;
        }
    }
}
}  // namespace