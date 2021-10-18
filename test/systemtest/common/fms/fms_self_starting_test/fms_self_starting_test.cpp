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

#include <fcntl.h>
#include <fstream>
#include <future>
#include <gtest/gtest.h>

#include "ability_handler.h"
#include "ability_info.h"
#include "ability_local_record.h"
#include "ability_start_setting.h"
#include "app_log_wrapper.h"
#include "common_event.h"
#include "common_event_manager.h"
#include "context_deal.h"
#include "form_event.h"
#include "form_st_common_info.h"
#include "system_test_form_util.h"
#include "iservice_registry.h"
#include "nlohmann/json.hpp"
#include "system_ability_definition.h"

using OHOS::AAFwk::Want;
using namespace testing::ext;
using namespace std::chrono_literals;
using namespace OHOS::STtools;

namespace {
std::vector<std::string> bundleNameList = {
    "com.form.formsystemtestservicea"
};
std::vector<std::string> hapNameList = {
    "ActsFormSystemTestServiceA"
};
}  // namespace

namespace OHOS {
namespace AppExecFwk {
class FmsSelfStartingTest : public testing::Test {
public:
    static void SetUpTestCase();
    static void TearDownTestCase();

    static bool SubscribeEvent();

    void SetUp();
    void TearDown();

    void StartAbilityKitTest(const std::string &abilityName, const std::string &bundleName);
    void TerminateAbility(const std::string &eventName, const std::string &abilityName);
  
    class FormEventSubscriber : public CommonEventSubscriber {
    public:
        explicit FormEventSubscriber(const CommonEventSubscribeInfo &sp) : CommonEventSubscriber(sp) {};
        virtual void OnReceiveEvent(const CommonEventData &data) override;
        ~FormEventSubscriber() = default;
    };

    static sptr<AAFwk::IAbilityManager> abilityMs;
    static FormEvent event;
    static std::vector<std::string> eventList;
    static std::shared_ptr<FormEventSubscriber> subscriber_;
};

std::vector<std::string> FmsSelfStartingTest::eventList = {
    FORM_EVENT_ABILITY_ONACTIVED, FORM_EVENT_RECV_SELF_STARTING_TEST_0100, FORM_EVENT_RECV_SELF_STARTING_TEST_0200,
    FORM_EVENT_RECV_SELF_STARTING_TEST_0300
};

FormEvent FmsSelfStartingTest::event = FormEvent();
sptr<AAFwk::IAbilityManager> FmsSelfStartingTest::abilityMs = nullptr;
std::shared_ptr<FmsSelfStartingTest::FormEventSubscriber> FmsSelfStartingTest::subscriber_ = nullptr;
void FmsSelfStartingTest::FormEventSubscriber::OnReceiveEvent(const CommonEventData &data)
{
    GTEST_LOG_(INFO) << "OnReceiveEvent: event=" << data.GetWant().GetAction();
    GTEST_LOG_(INFO) << "OnReceiveEvent: data=" << data.GetData();
    GTEST_LOG_(INFO) << "OnReceiveEvent: code=" << data.GetCode();
    SystemTestFormUtil::Completed(event, data.GetWant().GetAction(), data.GetCode(), data.GetData());
}

void FmsSelfStartingTest::SetUpTestCase()
{
    if (!SubscribeEvent()) {
        GTEST_LOG_(INFO) << "SubscribeEvent error";
    }
}

void FmsSelfStartingTest::TearDownTestCase()
{
    GTEST_LOG_(INFO) << "UnSubscribeCommonEvent calld";
    CommonEventManager::UnSubscribeCommonEvent(subscriber_);
}

void FmsSelfStartingTest::SetUp()
{
}

void FmsSelfStartingTest::TearDown()
{
    GTEST_LOG_(INFO) << "CleanMsg calld";
    SystemTestFormUtil::CleanMsg(event);
}
bool FmsSelfStartingTest::SubscribeEvent()
{
    GTEST_LOG_(INFO) << "SubscribeEvent calld";
    MatchingSkills matchingSkills;
    for (const auto &e : eventList) {
        matchingSkills.AddEvent(e);
    }
    CommonEventSubscribeInfo subscribeInfo(matchingSkills);
    subscribeInfo.SetPriority(1);
    subscriber_ = std::make_shared<FormEventSubscriber>(subscribeInfo);
    return CommonEventManager::SubscribeCommonEvent(subscriber_);
}

/**
 * @tc.number: FMS_Start_0300_01
 * @tc.name: Host A add one form
 * @tc.desc:
 */
HWTEST_F(FmsSelfStartingTest, FMS_Start_0300_01, Function | MediumTest | Level1)
{
    std::cout << "START FMS_Start_0300_01" << std::endl;

    std::string bundleName = "com.ohos.form.manager.selfStartingA";
    std::string abilityName = "FormAbilitySelfStartingA";
    MAP_STR_STR params;
    Want want = SystemTestFormUtil::MakeWant("device", abilityName, bundleName, params);
    SystemTestFormUtil::StartAbility(want, abilityMs);
    EXPECT_EQ(SystemTestFormUtil::WaitCompleted(event, FORM_EVENT_ABILITY_ONACTIVED, 0), 0);

    std::string eventData = FORM_EVENT_REQ_SELF_STARTING_TEST_0100;
    SystemTestFormUtil::PublishEvent(FORM_EVENT_REQ_SELF_STARTING_TEST_0100, EVENT_CODE_100, eventData);
    EXPECT_EQ(0, SystemTestFormUtil::WaitCompleted(event, FORM_EVENT_RECV_SELF_STARTING_TEST_0100, EVENT_CODE_100));
    std::string data = SystemTestFormUtil::GetData(event, FORM_EVENT_RECV_SELF_STARTING_TEST_0100, EVENT_CODE_100);
    bool result = data == "true";
    EXPECT_TRUE(result);
    if (!result) {
        GTEST_LOG_(INFO) << "FMS_Start_0300_01,  result:" << result;
    }
    std::cout << "END FMS_Start_0300_01" << std::endl;
}

/**
 * @tc.number: FMS_Start_0300_02
 * @tc.name: Host B add form one
 * @tc.desc:
 */
HWTEST_F(FmsSelfStartingTest, FMS_Start_0300_02, Function | MediumTest | Level1)
{
    std::cout << "START FMS_Start_0300_02" << std::endl;
    std::string bundleName = "com.ohos.form.manager.selfStartingB";
    std::string abilityName = "FormAbilitySelfStartingB";
    MAP_STR_STR params;
    Want want = SystemTestFormUtil::MakeWant("device", abilityName, bundleName, params);
    SystemTestFormUtil::StartAbility(want, abilityMs);
    EXPECT_EQ(SystemTestFormUtil::WaitCompleted(event, FORM_EVENT_ABILITY_ONACTIVED, 0), 0);

    std::string eventData = FORM_EVENT_REQ_SELF_STARTING_TEST_0200;
    GTEST_LOG_(INFO) << "FMS_Start_0300_02,  eventData:" << eventData;
    SystemTestFormUtil::PublishEvent(FORM_EVENT_REQ_SELF_STARTING_TEST_0200, EVENT_CODE_200, eventData);
    EXPECT_EQ(0, SystemTestFormUtil::WaitCompleted(event, FORM_EVENT_RECV_SELF_STARTING_TEST_0200, EVENT_CODE_200));
    std::string data = SystemTestFormUtil::GetData(event, FORM_EVENT_RECV_SELF_STARTING_TEST_0200, EVENT_CODE_200);
    bool result = data == "true";
    EXPECT_TRUE(result);
    if (!result) {
        GTEST_LOG_(INFO) << "FMS_Start_0300_02,  result:" << result;
    }

    std::cout << "END FMS_Start_0300_02" << std::endl;
}

/**
 * @tc.number: FMS_Start_0300_03
 * @tc.name: Host B add form two
 * @tc.desc:
 */
HWTEST_F(FmsSelfStartingTest, FMS_Start_0300_03, Function | MediumTest | Level1)
{
    std::cout << "START FMS_Start_0300_03" << std::endl;
    std::string bundleName = "com.ohos.form.manager.selfStartingB";
    std::string abilityName = "FormAbilitySelfStartingB";
    MAP_STR_STR params;
    Want want = SystemTestFormUtil::MakeWant("device", abilityName, bundleName, params);
    SystemTestFormUtil::StartAbility(want, abilityMs);
    EXPECT_EQ(SystemTestFormUtil::WaitCompleted(event, FORM_EVENT_ABILITY_ONACTIVED, 0), 0);

    std::string eventData = FORM_EVENT_REQ_SELF_STARTING_TEST_0300;
    GTEST_LOG_(INFO) << "FMS_Start_0300_03,  eventData:" << eventData;
    SystemTestFormUtil::PublishEvent(FORM_EVENT_REQ_SELF_STARTING_TEST_0300, EVENT_CODE_300, eventData);
    EXPECT_EQ(0, SystemTestFormUtil::WaitCompleted(event, FORM_EVENT_RECV_SELF_STARTING_TEST_0300, EVENT_CODE_300));
    std::string data = SystemTestFormUtil::GetData(event, FORM_EVENT_RECV_SELF_STARTING_TEST_0300, EVENT_CODE_300);
    bool result = data == "true";
    EXPECT_TRUE(result);
    if (!result) {
        GTEST_LOG_(INFO) << "FMS_Start_0300_03,  result:" << result;
    }
}
}  // namespace AppExecFwk
}  // namespace OHOS