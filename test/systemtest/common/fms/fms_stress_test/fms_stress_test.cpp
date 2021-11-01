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

#include "ability_info.h"
#include "ability_handler.h"
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
#include "stress_test_config_parser.h"
#include "system_ability_definition.h"

using OHOS::AAFwk::Want;
using namespace testing::ext;
using namespace std::chrono_literals;
using namespace OHOS::STtools;

namespace OHOS {
namespace AppExecFwk {
static StressTestLevel stLevel_;
static int g_iSuccessfulTimes = 0;
class FmsStressTest : public testing::Test {
public:
    static void SetUpTestCase();
    static void TearDownTestCase();
    static bool SubscribeEvent();

    void SetUp();
    void TearDown();
    void StartAbilityKitTest(const std::string &abilityName, const std::string &bundleName);
    void TerminateAbility(const std::string &eventName, const std::string &abilityName);

    // Test case
    static void StressTest_0100();
    static void StressTest_0200();
    static void StressTest_0300();
    static void StressTest_1100();
    static void StressTest_1300();
    static void StressTest_1700();
    static void StressTest_1800();

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

std::vector<std::string> FmsStressTest::eventList = {
    FORM_EVENT_ABILITY_ONACTIVED, FORM_EVENT_RECV_STRESS_TEST_0100,
    FORM_EVENT_RECV_STRESS_TEST_0100_01, FORM_EVENT_RECV_STRESS_TEST_0200,
    FORM_EVENT_RECV_STRESS_TEST_0200_01, FORM_EVENT_RECV_STRESS_TEST_0200_02, FORM_EVENT_RECV_STRESS_TEST_0200_03,
    FORM_EVENT_RECV_STRESS_TEST_0300, FORM_EVENT_RECV_STRESS_TEST_0300_01, FORM_EVENT_RECV_STRESS_TEST_0300_02,
    FORM_EVENT_RECV_STRESS_TEST_0400, FORM_EVENT_RECV_STRESS_TEST_0500,
    FORM_EVENT_RECV_STRESS_TEST_1100, FORM_EVENT_RECV_STRESS_TEST_1100_01, FORM_EVENT_RECV_STRESS_TEST_1100_02,
    FORM_EVENT_RECV_STRESS_TEST_1100_03, FORM_EVENT_RECV_STRESS_TEST_1300, FORM_EVENT_RECV_STRESS_TEST_1300_01,
    FORM_EVENT_RECV_STRESS_TEST_1300_02, FORM_EVENT_RECV_STRESS_TEST_1300_03,
    FORM_EVENT_RECV_STRESS_TEST_1700, FORM_EVENT_RECV_STRESS_TEST_1700_01, FORM_EVENT_RECV_STRESS_TEST_1700_02,
    FORM_EVENT_RECV_STRESS_TEST_1800,
};

FormEvent FmsStressTest::event = FormEvent();
sptr<AAFwk::IAbilityManager> FmsStressTest::abilityMs = nullptr;
std::shared_ptr<FmsStressTest::FormEventSubscriber> FmsStressTest::subscriber_ = nullptr;
void FmsStressTest::FormEventSubscriber::OnReceiveEvent(const CommonEventData &data)
{
    GTEST_LOG_(INFO) << "OnReceiveEvent: event=" << data.GetWant().GetAction();
    GTEST_LOG_(INFO) << "OnReceiveEvent: data=" << data.GetData();
    GTEST_LOG_(INFO) << "OnReceiveEvent: code=" << data.GetCode();
    SystemTestFormUtil::Completed(event, data.GetWant().GetAction(), data.GetCode(), data.GetData());
}

void FmsStressTest::SetUpTestCase()
{
    if (!SubscribeEvent()) {
        GTEST_LOG_(INFO) << "SubscribeEvent error";
    }

    StressTestConfigParser stcp;
    stcp.ParseForStressTest(STRESS_TEST_CONFIG_FILE_PATH, stLevel_);
    std::cout << "stress test level : "
        << "executionTimes : " << stLevel_.executionTimesLevel << ", time:" << stLevel_.sleepTime << std::endl;
}

void FmsStressTest::TearDownTestCase()
{
    GTEST_LOG_(INFO) << "UnSubscribeCommonEvent calld";
    CommonEventManager::UnSubscribeCommonEvent(subscriber_);
    std::cout << "========stress test level : "
        << "case execution Times : " << stLevel_.executionTimesLevel << ", time:" << stLevel_.sleepTime << std::endl;
}

void FmsStressTest::SetUp()
{
    g_iSuccessfulTimes = 0;
}

void FmsStressTest::TearDown()
{
    std::cout << "========Stress test: "
              << "Current case Successful Times : " << g_iSuccessfulTimes++ << "=========" << std::endl;
    GTEST_LOG_(INFO) << "CleanMsg calld";
    SystemTestFormUtil::CleanMsg(event);
}
bool FmsStressTest::SubscribeEvent()
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

void FmsStressTest::StressTest_0100()
{
    std::string bundleName = "com.ohos.form.manager.stress";
    std::string abilityName = "FormAbilityStress";
    MAP_STR_STR params;
    Want want = SystemTestFormUtil::MakeWant("device", abilityName, bundleName, params);
    SystemTestFormUtil::StartAbility(want, abilityMs);
    EXPECT_EQ(SystemTestFormUtil::WaitCompleted(event, FORM_EVENT_ABILITY_ONACTIVED, 0), 0);

    // AcquireForm
    std::string eventData = FORM_EVENT_REQ_STRESS_TEST_0100;
    SystemTestFormUtil::PublishEvent(FORM_EVENT_REQ_STRESS_TEST_0100, EVENT_CODE_100, eventData);

    // OnAcquired
    EXPECT_EQ(0, SystemTestFormUtil::WaitCompleted(event, FORM_EVENT_RECV_STRESS_TEST_0100, EVENT_CODE_101));
    std::string data = SystemTestFormUtil::GetData(event, FORM_EVENT_RECV_STRESS_TEST_0100, EVENT_CODE_101);
    std::string formId = data;

    // OnUpdate
    EXPECT_EQ(0, SystemTestFormUtil::WaitCompleted(event, FORM_EVENT_RECV_STRESS_TEST_0100, EVENT_CODE_102));
    data = SystemTestFormUtil::GetData(event, FORM_EVENT_RECV_STRESS_TEST_0100, EVENT_CODE_102);
    bool result = data == "true";
    EXPECT_TRUE(result);
    if (result) {
        GTEST_LOG_(INFO) << "FMS_stressTest_0100 AcquireForm,  result:" << result;
    }

    // DeleteForm Result
    EXPECT_EQ(0, SystemTestFormUtil::WaitCompleted(event, FORM_EVENT_RECV_STRESS_TEST_0100_01, EVENT_CODE_103));
    data = SystemTestFormUtil::GetData(event, FORM_EVENT_RECV_STRESS_TEST_0100_01, EVENT_CODE_103);
    result = data == "true";
    EXPECT_TRUE(result);
    if (result) {
        g_iSuccessfulTimes++;
        GTEST_LOG_(INFO) << "FMS_stressTest_0100 DeleteForm,  result:" << result;
    }

    SystemTestFormUtil::CleanMsg(event);
}

void FmsStressTest::StressTest_0200()
{
    std::string bundleName = "com.ohos.form.manager.stress";
    std::string abilityName = "FormAbilityStress";
    MAP_STR_STR params;
    Want want = SystemTestFormUtil::MakeWant("device", abilityName, bundleName, params);
    SystemTestFormUtil::StartAbility(want, abilityMs);
    EXPECT_EQ(SystemTestFormUtil::WaitCompleted(event, FORM_EVENT_ABILITY_ONACTIVED, 0), 0);

    // AcquireForm
    std::string eventData = FORM_EVENT_REQ_STRESS_TEST_0200;
    SystemTestFormUtil::PublishEvent(FORM_EVENT_REQ_STRESS_TEST_0200, EVENT_CODE_200, eventData);

    // OnAcquired
    EXPECT_EQ(0, SystemTestFormUtil::WaitCompleted(event, FORM_EVENT_RECV_STRESS_TEST_0200, EVENT_CODE_201));
    std::string data = SystemTestFormUtil::GetData(event, FORM_EVENT_RECV_STRESS_TEST_0200, EVENT_CODE_201);
    std::string formId = data;
    GTEST_LOG_(INFO) << "FMS_stressTest_0200 AcquireForm,  formId:" << formId;

    // OnUpdate
    EXPECT_EQ(0, SystemTestFormUtil::WaitCompleted(event, FORM_EVENT_RECV_STRESS_TEST_0200, EVENT_CODE_202));
    data = SystemTestFormUtil::GetData(event, FORM_EVENT_RECV_STRESS_TEST_0200, EVENT_CODE_202);
    bool result = data == "true";
    EXPECT_TRUE(result);
    if (result) {
        GTEST_LOG_(INFO) << "FMS_stressTest_0200 AcquireForm 001,  result:" << result;
    }

    // ReleaseForm Result
    EXPECT_EQ(0, SystemTestFormUtil::WaitCompleted(event, FORM_EVENT_RECV_STRESS_TEST_0200_01, EVENT_CODE_203));
    data = SystemTestFormUtil::GetData(event, FORM_EVENT_RECV_STRESS_TEST_0200_01, EVENT_CODE_203);
    result = data == "true";
    EXPECT_TRUE(result);
    if (result) {
        GTEST_LOG_(INFO) << "FMS_stressTest_0200 ReleaseForm,  result:" << result;
    }

    // OnAcquired
    EXPECT_EQ(0, SystemTestFormUtil::WaitCompleted(event, FORM_EVENT_RECV_STRESS_TEST_0200_02, EVENT_CODE_204));
    data = SystemTestFormUtil::GetData(event, FORM_EVENT_RECV_STRESS_TEST_0200_02, EVENT_CODE_204);
    result = data == "true";
    EXPECT_TRUE(result);
    if (result) {
        GTEST_LOG_(INFO) << "FMS_stressTest_0200 AcquireForm 002,  result:" << result;
    }

    // DeleteForm Result
    EXPECT_EQ(0, SystemTestFormUtil::WaitCompleted(event, FORM_EVENT_RECV_STRESS_TEST_0200_03, EVENT_CODE_205));
    data = SystemTestFormUtil::GetData(event, FORM_EVENT_RECV_STRESS_TEST_0200_03, EVENT_CODE_205);
    result = data == "true";
    EXPECT_TRUE(result);
    if (result) {
        g_iSuccessfulTimes++;
        GTEST_LOG_(INFO) << "FMS_stressTest_0200 DeleteForm,  result:" << result;
    }

    SystemTestFormUtil::CleanMsg(event);
}

void FmsStressTest::StressTest_0300()
{
    std::string bundleName = "com.ohos.form.manager.stress";
    std::string abilityName = "FormAbilityStress";
    MAP_STR_STR params;
    Want want = SystemTestFormUtil::MakeWant("device", abilityName, bundleName, params);
    SystemTestFormUtil::StartAbility(want, abilityMs);
    EXPECT_EQ(SystemTestFormUtil::WaitCompleted(event, FORM_EVENT_ABILITY_ONACTIVED, 0), 0);

    // AcquireForm
    std::string eventData = FORM_EVENT_REQ_STRESS_TEST_0300;
    SystemTestFormUtil::PublishEvent(FORM_EVENT_REQ_STRESS_TEST_0300, EVENT_CODE_300, eventData);

    // OnAcquired
    EXPECT_EQ(0, SystemTestFormUtil::WaitCompleted(event, FORM_EVENT_RECV_STRESS_TEST_0300, EVENT_CODE_301));
    std::string data = SystemTestFormUtil::GetData(event, FORM_EVENT_RECV_STRESS_TEST_0300, EVENT_CODE_301);
    std::string formId = data;

    // OnUpdate
    EXPECT_EQ(0, SystemTestFormUtil::WaitCompleted(event, FORM_EVENT_RECV_STRESS_TEST_0300, EVENT_CODE_302));
    data = SystemTestFormUtil::GetData(event, FORM_EVENT_RECV_STRESS_TEST_0300, EVENT_CODE_302);
    bool result = data == "true";
    EXPECT_TRUE(result);
    if (result) {
        GTEST_LOG_(INFO) << "FMS_stressTest_0300 AcquireForm,  result:" << result;
    }

    // CastTempForm Result
    EXPECT_EQ(0, SystemTestFormUtil::WaitCompleted(event, FORM_EVENT_RECV_STRESS_TEST_0300_01, EVENT_CODE_303));
    data = SystemTestFormUtil::GetData(event, FORM_EVENT_RECV_STRESS_TEST_0300_01, EVENT_CODE_303);
    result = data == "true";
    EXPECT_TRUE(result);
    if (result) {
        GTEST_LOG_(INFO) << "FMS_stressTest_0300 CastTempForm,  result:" << result;
    }

    // DeleteForm Result
    EXPECT_EQ(0, SystemTestFormUtil::WaitCompleted(event, FORM_EVENT_RECV_STRESS_TEST_0300_02, EVENT_CODE_304));
    data = SystemTestFormUtil::GetData(event, FORM_EVENT_RECV_STRESS_TEST_0300_02, EVENT_CODE_304);
    result = data == "true";
    EXPECT_TRUE(result);
    if (result) {
        g_iSuccessfulTimes++;
        GTEST_LOG_(INFO) << "FMS_stressTest_0300 DeleteForm,  result:" << result;
    }

    SystemTestFormUtil::CleanMsg(event);
}

void FmsStressTest::StressTest_1100()
{
    std::string bundleName = "com.ohos.form.manager.stress";
    std::string abilityName = "FormAbilityStress";
    MAP_STR_STR params;
    Want want = SystemTestFormUtil::MakeWant("device", abilityName, bundleName, params);
    SystemTestFormUtil::StartAbility(want, abilityMs);
    EXPECT_EQ(SystemTestFormUtil::WaitCompleted(event, FORM_EVENT_ABILITY_ONACTIVED, 0), 0);

    // AcquireForm
    std::string eventData = FORM_EVENT_REQ_STRESS_TEST_1100;
    SystemTestFormUtil::PublishEvent(FORM_EVENT_REQ_STRESS_TEST_1100, EVENT_CODE_1100, eventData);

    // OnAcquired
    EXPECT_EQ(0, SystemTestFormUtil::WaitCompleted(event, FORM_EVENT_RECV_STRESS_TEST_1100, EVENT_CODE_1101));
    std::string data = SystemTestFormUtil::GetData(event, FORM_EVENT_RECV_STRESS_TEST_1100, EVENT_CODE_1101);
    std::string formId = data;

    // OnUpdate
    EXPECT_EQ(0, SystemTestFormUtil::WaitCompleted(event, FORM_EVENT_RECV_STRESS_TEST_1100, EVENT_CODE_1102));
    data = SystemTestFormUtil::GetData(event, FORM_EVENT_RECV_STRESS_TEST_1100, EVENT_CODE_1102);
    bool result = data == "true";
    EXPECT_TRUE(result);
    if (result) {
        GTEST_LOG_(INFO) << "FMS_stressTest_1100 AcquireForm,  result:" << result;
    }

    // NotifyInvisibleForms Result
    EXPECT_EQ(0, SystemTestFormUtil::WaitCompleted(event, FORM_EVENT_RECV_STRESS_TEST_1100_01, EVENT_CODE_1103));
    data = SystemTestFormUtil::GetData(event, FORM_EVENT_RECV_STRESS_TEST_1100_01, EVENT_CODE_1103);
    result = data == "true";
    EXPECT_TRUE(result);
    if (result) {
        GTEST_LOG_(INFO) << "FMS_stressTest_1100 NotifyInvisibleForms,  result:" << result;
    }

    // NotifyVisibleForms Result
    EXPECT_EQ(0, SystemTestFormUtil::WaitCompleted(event, FORM_EVENT_RECV_STRESS_TEST_1100_02, EVENT_CODE_1104));
    data = SystemTestFormUtil::GetData(event, FORM_EVENT_RECV_STRESS_TEST_1100_02, EVENT_CODE_1104);
    result = data == "true";
    EXPECT_TRUE(result);
    if (result) {
        GTEST_LOG_(INFO) << "FMS_stressTest_1100 notifyVisibleForms,  result:" << result;
    }

    // DeleteForm
    eventData = formId;
    SystemTestFormUtil::PublishEvent(FORM_EVENT_REQ_STRESS_TEST_1100_03, EVENT_CODE_1105, eventData);

    // DeleteForm Result
    EXPECT_EQ(0, SystemTestFormUtil::WaitCompleted(event, FORM_EVENT_RECV_STRESS_TEST_1100_03, EVENT_CODE_1105));
    data = SystemTestFormUtil::GetData(event, FORM_EVENT_RECV_STRESS_TEST_1100_03, EVENT_CODE_1105);
    result = data == "true";
    EXPECT_TRUE(result);
    if (result) {
        g_iSuccessfulTimes++;
        GTEST_LOG_(INFO) << "FMS_stressTest_1100 DeleteForm,  result:" << result;
    }

    SystemTestFormUtil::CleanMsg(event);
}

void FmsStressTest::StressTest_1300()
{
    std::string bundleName = "com.ohos.form.manager.stress";
    std::string abilityName = "FormAbilityStress";
    MAP_STR_STR params;
    Want want = SystemTestFormUtil::MakeWant("device", abilityName, bundleName, params);
    SystemTestFormUtil::StartAbility(want, abilityMs);
    EXPECT_EQ(SystemTestFormUtil::WaitCompleted(event, FORM_EVENT_ABILITY_ONACTIVED, 0), 0);

    // AcquireForm
    std::string eventData = FORM_EVENT_REQ_STRESS_TEST_1300;
    SystemTestFormUtil::PublishEvent(FORM_EVENT_REQ_STRESS_TEST_1300, EVENT_CODE_1300, eventData);

    // OnAcquired
    EXPECT_EQ(0, SystemTestFormUtil::WaitCompleted(event, FORM_EVENT_RECV_STRESS_TEST_1300, EVENT_CODE_1301));
    std::string data = SystemTestFormUtil::GetData(event, FORM_EVENT_RECV_STRESS_TEST_1300, EVENT_CODE_1301);
    std::string formId = data;

    // OnUpdate
    EXPECT_EQ(0, SystemTestFormUtil::WaitCompleted(event, FORM_EVENT_RECV_STRESS_TEST_1300, EVENT_CODE_1302));
    data = SystemTestFormUtil::GetData(event, FORM_EVENT_RECV_STRESS_TEST_1300, EVENT_CODE_1302);
    bool result = data == "true";
    EXPECT_TRUE(result);
    if (result) {
        GTEST_LOG_(INFO) << "FMS_stressTest_1300 AcquireForm,  result:" << result;
    }

    // EnableUpdateForm Result
    EXPECT_EQ(0, SystemTestFormUtil::WaitCompleted(event, FORM_EVENT_RECV_STRESS_TEST_1300_01, EVENT_CODE_1303));
    data = SystemTestFormUtil::GetData(event, FORM_EVENT_RECV_STRESS_TEST_1300_01, EVENT_CODE_1303);
    result = data == "true";
    EXPECT_TRUE(result);
    if (result) {
        GTEST_LOG_(INFO) << "FMS_stressTest_1300 EnableUpdateForm,  result:" << result;
    }

    // DisableUpdateForm Result
    EXPECT_EQ(0, SystemTestFormUtil::WaitCompleted(event, FORM_EVENT_RECV_STRESS_TEST_1300_02, EVENT_CODE_1304));
    data = SystemTestFormUtil::GetData(event, FORM_EVENT_RECV_STRESS_TEST_1300_02, EVENT_CODE_1304);
    result = data == "true";
    EXPECT_TRUE(result);
    if (result) {
        GTEST_LOG_(INFO) << "FMS_stressTest_1300 DisableUpdateForm,  result:" << result;
    }

    // DeleteForm
    eventData = formId;
    SystemTestFormUtil::PublishEvent(FORM_EVENT_REQ_STRESS_TEST_1300_03, EVENT_CODE_1305, eventData);

    // DeleteForm Result
    EXPECT_EQ(0, SystemTestFormUtil::WaitCompleted(event, FORM_EVENT_RECV_STRESS_TEST_1300_03, EVENT_CODE_1305));
    data = SystemTestFormUtil::GetData(event, FORM_EVENT_RECV_STRESS_TEST_1300_03, EVENT_CODE_1305);
    result = data == "true";
    EXPECT_TRUE(result);
    if (result) {
        g_iSuccessfulTimes++;
        GTEST_LOG_(INFO) << "FMS_stressTest_1300 DeleteForm,  result:" << result;
    }

    SystemTestFormUtil::CleanMsg(event);
}

void FmsStressTest::StressTest_1700()
{
    std::string bundleName = "com.ohos.form.manager.stress";
    std::string abilityName = "FormAbilityStress";
    MAP_STR_STR params;
    Want want = SystemTestFormUtil::MakeWant("device", abilityName, bundleName, params);
    SystemTestFormUtil::StartAbility(want, abilityMs);
    EXPECT_EQ(SystemTestFormUtil::WaitCompleted(event, FORM_EVENT_ABILITY_ONACTIVED, 0), 0);

    // GetAllFormsInfo
    std::string eventData = FORM_EVENT_REQ_STRESS_TEST_1700;
    SystemTestFormUtil::PublishEvent(FORM_EVENT_REQ_STRESS_TEST_1700, EVENT_CODE_1700, eventData);

    // GetAllFormsInfo Result
    EXPECT_EQ(0, SystemTestFormUtil::WaitCompleted(event, FORM_EVENT_RECV_STRESS_TEST_1700, EVENT_CODE_1700));
    std::string data = SystemTestFormUtil::GetData(event, FORM_EVENT_RECV_STRESS_TEST_1700, EVENT_CODE_1700);
    bool result = data == "true";
    EXPECT_TRUE(result);
    if (result) {
        GTEST_LOG_(INFO) << "FMS_stressTest_1700 GetAllFormsInfo,  result:" << result;
    }

    // GetFormsInfoByApp Result
    EXPECT_EQ(0, SystemTestFormUtil::WaitCompleted(event, FORM_EVENT_RECV_STRESS_TEST_1700_01, EVENT_CODE_1701));
    data = SystemTestFormUtil::GetData(event, FORM_EVENT_RECV_STRESS_TEST_1700_01, EVENT_CODE_1701);
    result = data == "true";
    EXPECT_TRUE(result);
    if (result) {
        GTEST_LOG_(INFO) << "FMS_stressTest_1700 GetFormsInfoByApp,  result:" << result;
    }

    // GetFormsInfoByModule Result
    EXPECT_EQ(0, SystemTestFormUtil::WaitCompleted(event, FORM_EVENT_RECV_STRESS_TEST_1700_02, EVENT_CODE_1702));
    data = SystemTestFormUtil::GetData(event, FORM_EVENT_RECV_STRESS_TEST_1700_02, EVENT_CODE_1702);
    result = data == "true";
    EXPECT_TRUE(result);
    if (result) {
        g_iSuccessfulTimes++;
        GTEST_LOG_(INFO) << "FMS_stressTest_1700 GetFormsInfoByModule,  result:" << result;
    }

    SystemTestFormUtil::CleanMsg(event);
}

void FmsStressTest::StressTest_1800()
{
    std::string bundleName = "com.ohos.form.manager.stress";
    std::string abilityName = "FormAbilityStress";
    MAP_STR_STR params;
    Want want = SystemTestFormUtil::MakeWant("device", abilityName, bundleName, params);
    SystemTestFormUtil::StartAbility(want, abilityMs);
    EXPECT_EQ(SystemTestFormUtil::WaitCompleted(event, FORM_EVENT_ABILITY_ONACTIVED, 0), 0);

    // CheckFMSReady
    std::string eventData = FORM_EVENT_REQ_STRESS_TEST_1800;
    SystemTestFormUtil::PublishEvent(FORM_EVENT_REQ_STRESS_TEST_1800, EVENT_CODE_1800, eventData);

    // CheckFMSReady Result
    EXPECT_EQ(0, SystemTestFormUtil::WaitCompleted(event, FORM_EVENT_RECV_STRESS_TEST_1800, EVENT_CODE_1800));
    std::string data = SystemTestFormUtil::GetData(event, FORM_EVENT_RECV_STRESS_TEST_1800, EVENT_CODE_1800);
    bool result = data == "true";
    EXPECT_TRUE(result);
    if (result) {
        g_iSuccessfulTimes++;
        GTEST_LOG_(INFO) << "FMS_stressTest_1800 CheckFMSReady,  result:" << result;
    }

    SystemTestFormUtil::CleanMsg(event);
}

/**
 * @tc.number    : FMS_stressTest_0100
 * @tc.name      : AcquireForm/DeleteForm stress test
 * @tc.desc      : AcquireForm successfully/DeleteForm successfully
 */
HWTEST_F(FmsStressTest, FMS_stressTest_0100, Function | MediumTest | Level2)
{
    std::cout << "START FMS_stressTest_0100" << std::endl;

    for (int iExecutionTimes = 0; iExecutionTimes < stLevel_.executionTimesLevel; iExecutionTimes++) {
        sleep(stLevel_.sleepTime);
        FmsStressTest::StressTest_0100();
        std::cout << "FMS_stressTest_0100 ExecutionTimes:" << iExecutionTimes + 1 << std::endl;
    }

    std::cout << "END FMS_stressTest_0100" << std::endl;
}

/**
 * @tc.number    : FMS_stressTest_0200
 * @tc.name      : AcquireForm、ReleaseForm/DeleteForm stress test
 * @tc.desc      : AcquireForm successfully/ReleaseForm successfully/DeleteForm successfully
 */
HWTEST_F(FmsStressTest, FMS_stressTest_0200, Function | MediumTest | Level2)
{
    std::cout << "START FMS_stressTest_0200" << std::endl;

    for (int iExecutionTimes = 0; iExecutionTimes < stLevel_.executionTimesLevel; iExecutionTimes++) {
        sleep(stLevel_.sleepTime);
        FmsStressTest::StressTest_0200();
        std::cout << "FMS_stressTest_0200 ExecutionTimes:" << iExecutionTimes + 1 << std::endl;
    }

    std::cout << "END FMS_stressTest_0200" << std::endl;
}

/**
 * @tc.number    : FMS_stressTest_0300
 * @tc.name      : AcquireForm/CastTempForm/DeleteForm stress test
 * @tc.desc      : AcquireForm successfully/CastTempForm successfully/DeleteForm successfully
 */
HWTEST_F(FmsStressTest, FMS_stressTest_0300, Function | MediumTest | Level2)
{
    std::cout << "START FMS_stressTest_0300" << std::endl;

    for (int iExecutionTimes = 0; iExecutionTimes < stLevel_.executionTimesLevel; iExecutionTimes++) {
        sleep(stLevel_.sleepTime);
        FmsStressTest::StressTest_0300();
        std::cout << "FMS_stressTest_0300 ExecutionTimes:" << iExecutionTimes + 1 << std::endl;
    }

    std::cout << "END FMS_stressTest_0300" << std::endl;
}

/**
 * @tc.number    : FMS_stressTest_1100
 * @tc.name      : NotifyInvisibleForms/NotifyVisibleForms stress test
 * @tc.desc      : NotifyInvisibleForms successfully/NotifyVisibleForms successfully
 */
HWTEST_F(FmsStressTest, FMS_stressTest_1100, Function | MediumTest | Level2)
{
    std::cout << "START FMS_stressTest_1100" << std::endl;

    for (int iExecutionTimes = 0; iExecutionTimes < stLevel_.executionTimesLevel; iExecutionTimes++) {
        sleep(stLevel_.sleepTime);
        FmsStressTest::StressTest_1100();
        std::cout << "FMS_stressTest_1100 ExecutionTimes:" << iExecutionTimes + 1 << std::endl;
    }

    std::cout << "END FMS_stressTest_1100" << std::endl;
}

/**
 * @tc.number    : FMS_stressTest_1300
 * @tc.name      : EnableUpdateForm/DisableUpdateForm stress test
 * @tc.desc      : EnableUpdateForm successfully/DisableUpdateForm successfully
 */
HWTEST_F(FmsStressTest, FMS_stressTest_1300, Function | MediumTest | Level2)
{
    std::cout << "START FMS_stressTest_1300" << std::endl;

    for (int iExecutionTimes = 0; iExecutionTimes < stLevel_.executionTimesLevel; iExecutionTimes++) {
        sleep(stLevel_.sleepTime);
        FmsStressTest::StressTest_1300();
        std::cout << "FMS_stressTest_1300 ExecutionTimes:" << iExecutionTimes + 1 << std::endl;
    }

    std::cout << "END FMS_stressTest_1300" << std::endl;
}

/**
 * @tc.number    : FMS_stressTest_1700
 * @tc.name      : GetAllFormsInfo/GetFormsInfoByApp/GetFormsInfoByModule stress test
 * @tc.desc      : GetAllFormsInfo successfully/GetFormsInfoByApp successfully/GetFormsInfoByModule successfully
 */
HWTEST_F(FmsStressTest, FMS_stressTest_1700, Function | MediumTest | Level2)
{
    std::cout << "START FMS_stressTest_1700" << std::endl;

    for (int iExecutionTimes = 0; iExecutionTimes < stLevel_.executionTimesLevel; iExecutionTimes++) {
        FmsStressTest::StressTest_1700();
        std::cout << "FMS_stressTest_1700 ExecutionTimes:" << iExecutionTimes + 1 << std::endl;
    }

    std::cout << "END FMS_stressTest_1700" << std::endl;
}

/**
 * @tc.number    : FMS_stressTest_1800
 * @tc.name      : CheckFMSReady stress test
 * @tc.desc      : CheckFMSReady successfully
 */
HWTEST_F(FmsStressTest, FMS_stressTest_1800, Function | MediumTest | Level2)
{
    std::cout << "START FMS_stressTest_1800" << std::endl;

    for (int iExecutionTimes = 0; iExecutionTimes < stLevel_.executionTimesLevel; iExecutionTimes++) {
        FmsStressTest::StressTest_1800();
        std::cout << "FMS_stressTest_1800 ExecutionTimes:" << iExecutionTimes + 1 << std::endl;
    }

    std::cout << "END FMS_stressTest_1800" << std::endl;
}
}  // namespace AppExecFwk
}  // namespace OHOS