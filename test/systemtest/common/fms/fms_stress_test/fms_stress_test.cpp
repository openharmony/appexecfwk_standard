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

namespace {
std::vector<std::string> bundleNameList = {
    "com.form.formsystemtestservicea",
    "com.form.formsystemtestserviceb"
};
std::vector<std::string> hapNameList = {
    "ActsFormSystemTestServiceA",
    "ActsFormSystemTestServiceB"
};
}  // namespace

namespace OHOS {
namespace AppExecFwk {
static StressTestLevel stLevel_;
static int g_iSuccessfulTimes = 0;
class FormStressTest : public testing::Test {
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
        explicit FormEventSubscriber(const CommonEventSubscribeInfo &sp) : CommonEventSubscriber(sp){};
        virtual void OnReceiveEvent(const CommonEventData &data) override;
        ~FormEventSubscriber() = default;
    };

    static sptr<AAFwk::IAbilityManager> abilityMs;
    static FormEvent event;
    static std::vector<std::string> eventList;
    static std::shared_ptr<FormEventSubscriber> subscriber_;
};

std::vector<std::string> FormStressTest::eventList = {
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

FormEvent FormStressTest::event = FormEvent();
sptr<AAFwk::IAbilityManager> FormStressTest::abilityMs = nullptr;
std::shared_ptr<FormStressTest::FormEventSubscriber> FormStressTest::subscriber_ = nullptr;
void FormStressTest::FormEventSubscriber::OnReceiveEvent(const CommonEventData &data)
{
    GTEST_LOG_(INFO) << "OnReceiveEvent: event=" << data.GetWant().GetAction();
    GTEST_LOG_(INFO) << "OnReceiveEvent: data=" << data.GetData();
    GTEST_LOG_(INFO) << "OnReceiveEvent: code=" << data.GetCode();
    SystemTestFormUtil::Completed(event, data.GetWant().GetAction(), data.GetCode(), data.GetData());
}

void FormStressTest::SetUpTestCase()
{
    SystemTestFormUtil::InstallHaps(hapNameList);
    if (!SubscribeEvent()) {
        GTEST_LOG_(INFO) << "SubscribeEvent error";
    }

    StressTestConfigParser stcp;
    stcp.ParseForStressTest(STRESS_TEST_CONFIG_FILE_PATH, stLevel_);
    std::cout << "stress test level : "
        << "executionTimes : " << stLevel_.executionTimesLevel << ", time:" << stLevel_.sleepTime << std::endl;
}

void FormStressTest::TearDownTestCase()
{
    GTEST_LOG_(INFO) << "UnSubscribeCommonEvent calld";
    SystemTestFormUtil::UninstallBundle(bundleNameList);
    CommonEventManager::UnSubscribeCommonEvent(subscriber_);
    std::cout << "========stress test level : "
        << "case execution Times : " << stLevel_.executionTimesLevel << ", time:" << stLevel_.sleepTime << std::endl;
}

void FormStressTest::SetUp()
{
    g_iSuccessfulTimes = 0;
}

void FormStressTest::TearDown()
{
    std::cout << "========Stress test: "
              << "Current case Successful Times : " << g_iSuccessfulTimes++ << "=========" << std::endl;
    GTEST_LOG_(INFO) << "CleanMsg calld";
    SystemTestFormUtil::CleanMsg(event);
}
bool FormStressTest::SubscribeEvent()
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

void FormStressTest::StressTest_0100()
{
    std::string bundleName = "com.ohos.form.manager.stress";
    std::string abilityName = "FormAbilityStress";
    MAP_STR_STR params;
    Want want = SystemTestFormUtil::MakeWant("device", abilityName, bundleName, params);
    SystemTestFormUtil::StartAbility(want, abilityMs);
    EXPECT_EQ(SystemTestFormUtil::WaitCompleted(event, FORM_EVENT_ABILITY_ONACTIVED, 0), 0);

    // AcquireForm
    std::string eventData = FORM_EVENT_REQ_STRESS_TEST_0100;
    SystemTestFormUtil::PublishEvent(FORM_EVENT_REQ_STRESS_TEST_0100, 100, eventData);

    // OnAcquired
    EXPECT_EQ(0, SystemTestFormUtil::WaitCompleted(event, FORM_EVENT_RECV_STRESS_TEST_0100, 101));
    std::string data = SystemTestFormUtil::GetData(event, FORM_EVENT_RECV_STRESS_TEST_0100, 101);
    std::string formId = data;

    // OnUpdate
    EXPECT_EQ(0, SystemTestFormUtil::WaitCompleted(event, FORM_EVENT_RECV_STRESS_TEST_0100, 102));
    data = SystemTestFormUtil::GetData(event, FORM_EVENT_RECV_STRESS_TEST_0100, 102);
    bool result = data == "true";
    EXPECT_TRUE(result);
    if (result) {
        GTEST_LOG_(INFO) << "FMS_stressTest_0100 AcquireForm,  result:" << result;
    }

    // DeleteForm Result
    EXPECT_EQ(0, SystemTestFormUtil::WaitCompleted(event, FORM_EVENT_RECV_STRESS_TEST_0100_01, 103));
    data = SystemTestFormUtil::GetData(event, FORM_EVENT_RECV_STRESS_TEST_0100_01, 103);
    result = data == "true";
    EXPECT_TRUE(result);
    if (result) {
        g_iSuccessfulTimes++;
        GTEST_LOG_(INFO) << "FMS_stressTest_0100 DeleteForm,  result:" << result;
    }

    SystemTestFormUtil::CleanMsg(event);
}

void FormStressTest::StressTest_0200()
{
    std::string bundleName = "com.ohos.form.manager.stress";
    std::string abilityName = "FormAbilityStress";
    MAP_STR_STR params;
    Want want = SystemTestFormUtil::MakeWant("device", abilityName, bundleName, params);
    SystemTestFormUtil::StartAbility(want, abilityMs);
    EXPECT_EQ(SystemTestFormUtil::WaitCompleted(event, FORM_EVENT_ABILITY_ONACTIVED, 0), 0);

    // AcquireForm
    std::string eventData = FORM_EVENT_REQ_STRESS_TEST_0200;
    SystemTestFormUtil::PublishEvent(FORM_EVENT_REQ_STRESS_TEST_0200, 200, eventData);

    // OnAcquired
    EXPECT_EQ(0, SystemTestFormUtil::WaitCompleted(event, FORM_EVENT_RECV_STRESS_TEST_0200, 201));
    std::string data = SystemTestFormUtil::GetData(event, FORM_EVENT_RECV_STRESS_TEST_0200, 201);
    std::string formId = data;
    GTEST_LOG_(INFO) << "FMS_stressTest_0200 AcquireForm,  formId:" << formId;

    // OnUpdate
    EXPECT_EQ(0, SystemTestFormUtil::WaitCompleted(event, FORM_EVENT_RECV_STRESS_TEST_0200, 202));
    data = SystemTestFormUtil::GetData(event, FORM_EVENT_RECV_STRESS_TEST_0200, 202);
    bool result = data == "true";
    EXPECT_TRUE(result);
    if (result) {
        GTEST_LOG_(INFO) << "FMS_stressTest_0200 AcquireForm 001,  result:" << result;
    }

    // ReleaseForm Result
    EXPECT_EQ(0, SystemTestFormUtil::WaitCompleted(event, FORM_EVENT_RECV_STRESS_TEST_0200_01, 203));
    data = SystemTestFormUtil::GetData(event, FORM_EVENT_RECV_STRESS_TEST_0200_01, 203);
    result = data == "true";
    EXPECT_TRUE(result);
    if (result) {
        GTEST_LOG_(INFO) << "FMS_stressTest_0200 ReleaseForm,  result:" << result;
    }

    // OnAcquired
    EXPECT_EQ(0, SystemTestFormUtil::WaitCompleted(event, FORM_EVENT_RECV_STRESS_TEST_0200_02, 204));
    data = SystemTestFormUtil::GetData(event, FORM_EVENT_RECV_STRESS_TEST_0200_02, 204);
    result = data == "true";
    EXPECT_TRUE(result);
    if (result) {
        GTEST_LOG_(INFO) << "FMS_stressTest_0200 AcquireForm 002,  result:" << result;
    }

    // DeleteForm Result
    EXPECT_EQ(0, SystemTestFormUtil::WaitCompleted(event, FORM_EVENT_RECV_STRESS_TEST_0200_03, 205));
    data = SystemTestFormUtil::GetData(event, FORM_EVENT_RECV_STRESS_TEST_0200_03, 205);
    result = data == "true";
    EXPECT_TRUE(result);
    if (result) {
        g_iSuccessfulTimes++;
        GTEST_LOG_(INFO) << "FMS_stressTest_0200 DeleteForm,  result:" << result;
    }

    SystemTestFormUtil::CleanMsg(event);
}

void FormStressTest::StressTest_0300()
{
    std::string bundleName = "com.ohos.form.manager.stress";
    std::string abilityName = "FormAbilityStress";
    MAP_STR_STR params;
    Want want = SystemTestFormUtil::MakeWant("device", abilityName, bundleName, params);
    SystemTestFormUtil::StartAbility(want, abilityMs);
    EXPECT_EQ(SystemTestFormUtil::WaitCompleted(event, FORM_EVENT_ABILITY_ONACTIVED, 0), 0);

    // AcquireForm
    std::string eventData = FORM_EVENT_REQ_STRESS_TEST_0300;
    SystemTestFormUtil::PublishEvent(FORM_EVENT_REQ_STRESS_TEST_0300, 300, eventData);

    // OnAcquired
    EXPECT_EQ(0, SystemTestFormUtil::WaitCompleted(event, FORM_EVENT_RECV_STRESS_TEST_0300, 301));
    std::string data = SystemTestFormUtil::GetData(event, FORM_EVENT_RECV_STRESS_TEST_0300, 301);
    std::string formId = data;

    // OnUpdate
    EXPECT_EQ(0, SystemTestFormUtil::WaitCompleted(event, FORM_EVENT_RECV_STRESS_TEST_0300, 302));
    data = SystemTestFormUtil::GetData(event, FORM_EVENT_RECV_STRESS_TEST_0300, 302);
    bool result = data == "true";
    EXPECT_TRUE(result);
    if (result) {
        GTEST_LOG_(INFO) << "FMS_stressTest_0300 AcquireForm,  result:" << result;
    }

    // CastTempForm Result
    EXPECT_EQ(0, SystemTestFormUtil::WaitCompleted(event, FORM_EVENT_RECV_STRESS_TEST_0300_01, 303));
    data = SystemTestFormUtil::GetData(event, FORM_EVENT_RECV_STRESS_TEST_0300_01, 303);
    result = data == "true";
    EXPECT_TRUE(result);
    if (result) {
        GTEST_LOG_(INFO) << "FMS_stressTest_0300 CastTempForm,  result:" << result;
    }

    // DeleteForm Result
    EXPECT_EQ(0, SystemTestFormUtil::WaitCompleted(event, FORM_EVENT_RECV_STRESS_TEST_0300_02, 304));
    data = SystemTestFormUtil::GetData(event, FORM_EVENT_RECV_STRESS_TEST_0300_02, 304);
    result = data == "true";
    EXPECT_TRUE(result);
    if (result) {
        g_iSuccessfulTimes++;
        GTEST_LOG_(INFO) << "FMS_stressTest_0300 DeleteForm,  result:" << result;
    }

    SystemTestFormUtil::CleanMsg(event);
}

void FormStressTest::StressTest_1100()
{
    std::string bundleName = "com.ohos.form.manager.stress";
    std::string abilityName = "FormAbilityStress";
    MAP_STR_STR params;
    Want want = SystemTestFormUtil::MakeWant("device", abilityName, bundleName, params);
    SystemTestFormUtil::StartAbility(want, abilityMs);
    EXPECT_EQ(SystemTestFormUtil::WaitCompleted(event, FORM_EVENT_ABILITY_ONACTIVED, 0), 0);

    // AcquireForm
    std::string eventData = FORM_EVENT_REQ_STRESS_TEST_1100;
    SystemTestFormUtil::PublishEvent(FORM_EVENT_REQ_STRESS_TEST_1100, 1100, eventData);

    // OnAcquired
    EXPECT_EQ(0, SystemTestFormUtil::WaitCompleted(event, FORM_EVENT_RECV_STRESS_TEST_1100, 1101));
    std::string data = SystemTestFormUtil::GetData(event, FORM_EVENT_RECV_STRESS_TEST_1100, 1101);
    std::string formId = data;

    // OnUpdate
    EXPECT_EQ(0, SystemTestFormUtil::WaitCompleted(event, FORM_EVENT_RECV_STRESS_TEST_1100, 1102));
    data = SystemTestFormUtil::GetData(event, FORM_EVENT_RECV_STRESS_TEST_1100, 1102);
    bool result = data == "true";
    EXPECT_TRUE(result);
    if (result) {
        GTEST_LOG_(INFO) << "FMS_stressTest_1100 AcquireForm,  result:" << result;
    }

    // NotifyInvisibleForms Result
    EXPECT_EQ(0, SystemTestFormUtil::WaitCompleted(event, FORM_EVENT_RECV_STRESS_TEST_1100_01, 1103));
    data = SystemTestFormUtil::GetData(event, FORM_EVENT_RECV_STRESS_TEST_1100_01, 1103);
    result = data == "true";
    EXPECT_TRUE(result);
    if (result) {
        GTEST_LOG_(INFO) << "FMS_stressTest_1100 NotifyInvisibleForms,  result:" << result;
    }

    // NotifyVisibleForms Result
    EXPECT_EQ(0, SystemTestFormUtil::WaitCompleted(event, FORM_EVENT_RECV_STRESS_TEST_1100_02, 1104));
    data = SystemTestFormUtil::GetData(event, FORM_EVENT_RECV_STRESS_TEST_1100_02, 1104);
    result = data == "true";
    EXPECT_TRUE(result);
    if (result) {
        GTEST_LOG_(INFO) << "FMS_stressTest_1100 notifyVisibleForms,  result:" << result;
    }

    // DeleteForm
    eventData = formId;
    SystemTestFormUtil::PublishEvent(FORM_EVENT_REQ_STRESS_TEST_1100_03, 1105, eventData);

    // DeleteForm Result
    EXPECT_EQ(0, SystemTestFormUtil::WaitCompleted(event, FORM_EVENT_RECV_STRESS_TEST_1100_03, 1105));
    data = SystemTestFormUtil::GetData(event, FORM_EVENT_RECV_STRESS_TEST_1100_03, 1105);
    result = data == "true";
    EXPECT_TRUE(result);
    if (result) {
        g_iSuccessfulTimes++;
        GTEST_LOG_(INFO) << "FMS_stressTest_1100 DeleteForm,  result:" << result;
    }

    SystemTestFormUtil::CleanMsg(event);
}

void FormStressTest::StressTest_1300()
{
    std::string bundleName = "com.ohos.form.manager.stress";
    std::string abilityName = "FormAbilityStress";
    MAP_STR_STR params;
    Want want = SystemTestFormUtil::MakeWant("device", abilityName, bundleName, params);
    SystemTestFormUtil::StartAbility(want, abilityMs);
    EXPECT_EQ(SystemTestFormUtil::WaitCompleted(event, FORM_EVENT_ABILITY_ONACTIVED, 0), 0);

    // AcquireForm
    std::string eventData = FORM_EVENT_REQ_STRESS_TEST_1300;
    SystemTestFormUtil::PublishEvent(FORM_EVENT_REQ_STRESS_TEST_1300, 1300, eventData);

    // OnAcquired
    EXPECT_EQ(0, SystemTestFormUtil::WaitCompleted(event, FORM_EVENT_RECV_STRESS_TEST_1300, 1301));
    std::string data = SystemTestFormUtil::GetData(event, FORM_EVENT_RECV_STRESS_TEST_1300, 1301);
    std::string formId = data;

    // OnUpdate
    EXPECT_EQ(0, SystemTestFormUtil::WaitCompleted(event, FORM_EVENT_RECV_STRESS_TEST_1300, 1302));
    data = SystemTestFormUtil::GetData(event, FORM_EVENT_RECV_STRESS_TEST_1300, 1302);
    bool result = data == "true";
    EXPECT_TRUE(result);
    if (result) {
        GTEST_LOG_(INFO) << "FMS_stressTest_1300 AcquireForm,  result:" << result;
    }

    // EnableUpdateForm Result
    EXPECT_EQ(0, SystemTestFormUtil::WaitCompleted(event, FORM_EVENT_RECV_STRESS_TEST_1300_01, 1303));
    data = SystemTestFormUtil::GetData(event, FORM_EVENT_RECV_STRESS_TEST_1300_01, 1303);
    result = data == "true";
    EXPECT_TRUE(result);
    if (result) {
        GTEST_LOG_(INFO) << "FMS_stressTest_1300 EnableUpdateForm,  result:" << result;
    }

    // DisableUpdateForm Result
    EXPECT_EQ(0, SystemTestFormUtil::WaitCompleted(event, FORM_EVENT_RECV_STRESS_TEST_1300_02, 1304));
    data = SystemTestFormUtil::GetData(event, FORM_EVENT_RECV_STRESS_TEST_1300_02, 1304);
    result = data == "true";
    EXPECT_TRUE(result);
    if (result) {
        GTEST_LOG_(INFO) << "FMS_stressTest_1300 DisableUpdateForm,  result:" << result;
    }

    // DeleteForm
    eventData = formId;
    SystemTestFormUtil::PublishEvent(FORM_EVENT_REQ_STRESS_TEST_1300_03, 1305, eventData);

    // DeleteForm Result
    EXPECT_EQ(0, SystemTestFormUtil::WaitCompleted(event, FORM_EVENT_RECV_STRESS_TEST_1300_03, 1305));
    data = SystemTestFormUtil::GetData(event, FORM_EVENT_RECV_STRESS_TEST_1300_03, 1305);
    result = data == "true";
    EXPECT_TRUE(result);
    if (result) {
        g_iSuccessfulTimes++;
        GTEST_LOG_(INFO) << "FMS_stressTest_1300 DeleteForm,  result:" << result;
    }

    SystemTestFormUtil::CleanMsg(event);
}

void FormStressTest::StressTest_1700()
{
    std::string bundleName = "com.ohos.form.manager.stress";
    std::string abilityName = "FormAbilityStress";
    MAP_STR_STR params;
    Want want = SystemTestFormUtil::MakeWant("device", abilityName, bundleName, params);
    SystemTestFormUtil::StartAbility(want, abilityMs);
    EXPECT_EQ(SystemTestFormUtil::WaitCompleted(event, FORM_EVENT_ABILITY_ONACTIVED, 0), 0);

    // GetAllFormsInfo
    std::string eventData = FORM_EVENT_REQ_STRESS_TEST_1700;
    SystemTestFormUtil::PublishEvent(FORM_EVENT_REQ_STRESS_TEST_1700, 1700, eventData);

    // GetAllFormsInfo Result
    EXPECT_EQ(0, SystemTestFormUtil::WaitCompleted(event, FORM_EVENT_RECV_STRESS_TEST_1700, 1700));
    std::string data = SystemTestFormUtil::GetData(event, FORM_EVENT_RECV_STRESS_TEST_1700, 1700);
    bool result = data == "true";
    EXPECT_TRUE(result);
    if (result) {
        GTEST_LOG_(INFO) << "FMS_stressTest_1700 GetAllFormsInfo,  result:" << result;
    }

    // GetFormsInfoByApp Result
    EXPECT_EQ(0, SystemTestFormUtil::WaitCompleted(event, FORM_EVENT_RECV_STRESS_TEST_1700_01, 1701));
    data = SystemTestFormUtil::GetData(event, FORM_EVENT_RECV_STRESS_TEST_1700_01, 1701);
    result = data == "true";
    EXPECT_TRUE(result);
    if (result) {
        GTEST_LOG_(INFO) << "FMS_stressTest_1700 GetFormsInfoByApp,  result:" << result;
    }

    // GetFormsInfoByModule Result
    EXPECT_EQ(0, SystemTestFormUtil::WaitCompleted(event, FORM_EVENT_RECV_STRESS_TEST_1700_02, 1702));
    data = SystemTestFormUtil::GetData(event, FORM_EVENT_RECV_STRESS_TEST_1700_02, 1702);
    result = data == "true";
    EXPECT_TRUE(result);
    if (result) {
        g_iSuccessfulTimes++;
        GTEST_LOG_(INFO) << "FMS_stressTest_1700 GetFormsInfoByModule,  result:" << result;
    }

    SystemTestFormUtil::CleanMsg(event);
}

void FormStressTest::StressTest_1800()
{
    std::string bundleName = "com.ohos.form.manager.stress";
    std::string abilityName = "FormAbilityStress";
    MAP_STR_STR params;
    Want want = SystemTestFormUtil::MakeWant("device", abilityName, bundleName, params);
    SystemTestFormUtil::StartAbility(want, abilityMs);
    EXPECT_EQ(SystemTestFormUtil::WaitCompleted(event, FORM_EVENT_ABILITY_ONACTIVED, 0), 0);

    // CheckFMSReady
    std::string eventData = FORM_EVENT_REQ_STRESS_TEST_1800;
    SystemTestFormUtil::PublishEvent(FORM_EVENT_REQ_STRESS_TEST_1800, 1800, eventData);

    // CheckFMSReady Result
    EXPECT_EQ(0, SystemTestFormUtil::WaitCompleted(event, FORM_EVENT_RECV_STRESS_TEST_1800, 1800));
    std::string data = SystemTestFormUtil::GetData(event, FORM_EVENT_RECV_STRESS_TEST_1800, 1800);
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
HWTEST_F(FormStressTest, FMS_stressTest_0100, Function | MediumTest | Level2)
{
    std::cout << "START FMS_stressTest_0100" << std::endl;

    for (int iExecutionTimes = 0; iExecutionTimes < stLevel_.executionTimesLevel; iExecutionTimes++) {
        sleep(stLevel_.sleepTime);
        FormStressTest::StressTest_0100();
        std::cout << "FMS_stressTest_0100 ExecutionTimes:" << iExecutionTimes + 1 << std::endl;
    }

    std::cout << "END FMS_stressTest_0100" << std::endl;
}

/**
 * @tc.number    : FMS_stressTest_0200
 * @tc.name      : AcquireForm、ReleaseForm/DeleteForm stress test
 * @tc.desc      : AcquireForm successfully/ReleaseForm successfully/DeleteForm successfully
 */
HWTEST_F(FormStressTest, FMS_stressTest_0200, Function | MediumTest | Level2)
{
    std::cout << "START FMS_stressTest_0200" << std::endl;

    for (int iExecutionTimes = 0; iExecutionTimes < stLevel_.executionTimesLevel; iExecutionTimes++) {
        sleep(stLevel_.sleepTime);
        FormStressTest::StressTest_0200();
        std::cout << "FMS_stressTest_0200 ExecutionTimes:" << iExecutionTimes + 1 << std::endl;
    }

    std::cout << "END FMS_stressTest_0200" << std::endl;
}

/**
 * @tc.number    : FMS_stressTest_0300
 * @tc.name      : AcquireForm/CastTempForm/DeleteForm stress test
 * @tc.desc      : AcquireForm successfully/CastTempForm successfully/DeleteForm successfully
 */
HWTEST_F(FormStressTest, FMS_stressTest_0300, Function | MediumTest | Level2)
{
    std::cout << "START FMS_stressTest_0300" << std::endl;

    for (int iExecutionTimes = 0; iExecutionTimes < stLevel_.executionTimesLevel; iExecutionTimes++) {
        sleep(stLevel_.sleepTime);
        FormStressTest::StressTest_0300();
        std::cout << "FMS_stressTest_0300 ExecutionTimes:" << iExecutionTimes + 1 << std::endl;
    }

    std::cout << "END FMS_stressTest_0300" << std::endl;
}

/**
 * @tc.number    : FMS_stressTest_1100
 * @tc.name      : NotifyInvisibleForms/NotifyVisibleForms stress test
 * @tc.desc      : NotifyInvisibleForms successfully/NotifyVisibleForms successfully
 */
HWTEST_F(FormStressTest, FMS_stressTest_1100, Function | MediumTest | Level2)
{
    std::cout << "START FMS_stressTest_1100" << std::endl;

    for (int iExecutionTimes = 0; iExecutionTimes < stLevel_.executionTimesLevel; iExecutionTimes++) {
        sleep(stLevel_.sleepTime);
        FormStressTest::StressTest_1100();
        std::cout << "FMS_stressTest_1100 ExecutionTimes:" << iExecutionTimes + 1 << std::endl;
    }

    std::cout << "END FMS_stressTest_1100" << std::endl;
}

/**
 * @tc.number    : FMS_stressTest_1300
 * @tc.name      : EnableUpdateForm/DisableUpdateForm stress test
 * @tc.desc      : EnableUpdateForm successfully/DisableUpdateForm successfully
 */
HWTEST_F(FormStressTest, FMS_stressTest_1300, Function | MediumTest | Level2)
{
    std::cout << "START FMS_stressTest_1300" << std::endl;

    for (int iExecutionTimes = 0; iExecutionTimes < stLevel_.executionTimesLevel; iExecutionTimes++) {
        sleep(stLevel_.sleepTime);
        FormStressTest::StressTest_1300();
        std::cout << "FMS_stressTest_1300 ExecutionTimes:" << iExecutionTimes + 1 << std::endl;
    }

    std::cout << "END FMS_stressTest_1300" << std::endl;
}

/**
 * @tc.number    : FMS_stressTest_1700
 * @tc.name      : GetAllFormsInfo/GetFormsInfoByApp/GetFormsInfoByModule stress test
 * @tc.desc      : GetAllFormsInfo successfully/GetFormsInfoByApp successfully/GetFormsInfoByModule successfully
 */
HWTEST_F(FormStressTest, FMS_stressTest_1700, Function | MediumTest | Level2)
{
    std::cout << "START FMS_stressTest_1700" << std::endl;

    for (int iExecutionTimes = 0; iExecutionTimes < stLevel_.executionTimesLevel; iExecutionTimes++) {
        FormStressTest::StressTest_1700();
        std::cout << "FMS_stressTest_1700 ExecutionTimes:" << iExecutionTimes + 1 << std::endl;
    }

    std::cout << "END FMS_stressTest_1700" << std::endl;
}

/**
 * @tc.number    : FMS_stressTest_1800
 * @tc.name      : CheckFMSReady stress test
 * @tc.desc      : CheckFMSReady successfully
 */
HWTEST_F(FormStressTest, FMS_stressTest_1800, Function | MediumTest | Level2)
{
    std::cout << "START FMS_stressTest_1800" << std::endl;

    for (int iExecutionTimes = 0; iExecutionTimes < stLevel_.executionTimesLevel; iExecutionTimes++) {
        FormStressTest::StressTest_1800();
        std::cout << "FMS_stressTest_1800 ExecutionTimes:" << iExecutionTimes + 1 << std::endl;
    }

    std::cout << "END FMS_stressTest_1800" << std::endl;
}
}  // namespace AppExecFwk
}  // namespace OHOS