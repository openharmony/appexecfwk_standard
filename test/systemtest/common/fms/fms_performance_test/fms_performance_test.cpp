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
#include <iostream>
#include <sys/time.h>

#include "app_log_wrapper.h"
#include "ability_handler.h"
#include "ability_info.h"
#include "ability_local_record.h"
#include "ability_start_setting.h"
#include "common_event.h"
#include "common_event_manager.h"
#include "context_deal.h"
#include "form_event.h"
#include "form_st_common_info.h"
#include "system_test_form_util.h"
#include "iservice_registry.h"
#include "nlohmann/json.hpp"
#include "performance_config_parser.h"
#include "system_ability_definition.h"

using OHOS::AAFwk::Want;
using namespace testing::ext;
using namespace std::chrono_literals;
using namespace OHOS::STtools;

namespace OHOS {
namespace AppExecFwk {
const int FOUR = 4;
static PerformanceLevel pLevel_;
static int64_t totalTime = 0;
const std::string TEST_RESULT_PATH = "./performance_test_result.txt";

class FormPerformanceTest : public testing::Test {
public:
    static void SetUpTestCase();
    static void TearDownTestCase();
    static bool SubscribeEvent();
    static bool SavePerformanceTestResult(const std::string &testcase, const int64_t &timeValue);

    void SetUp();
    void TearDown();
    void StartAbilityKitTest(const std::string &abilityName, const std::string &bundleName);
    void TerminateAbility(const std::string &eventName, const std::string &abilityName);

    // Test case
    static void PerformanceTest_0100();
    static void PerformanceTest_0200();
    static void PerformanceTest_0300();
    static void PerformanceTest_0400();
    static void PerformanceTest_0500();
    static void PerformanceTest_0600();
    static void PerformanceTest_0700();
    static void PerformanceTest_1300();
    static void PerformanceTest_1400();
    static void PerformanceTest_1500();
    static void PerformanceTest_1600();

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

std::vector<std::string> FormPerformanceTest::eventList = {
    FORM_EVENT_ABILITY_ONACTIVED, FORM_EVENT_RECV_PERFORMANCE_TEST_0100, FORM_EVENT_RECV_PERFORMANCE_TEST_0200,
    FORM_EVENT_RECV_PERFORMANCE_TEST_0300, FORM_EVENT_RECV_PERFORMANCE_TEST_0400, FORM_EVENT_RECV_PERFORMANCE_TEST_0500,
    FORM_EVENT_RECV_PERFORMANCE_TEST_0600, FORM_EVENT_RECV_PERFORMANCE_TEST_0700, FORM_EVENT_RECV_PERFORMANCE_TEST_1300,
    FORM_EVENT_RECV_PERFORMANCE_TEST_1400, FORM_EVENT_RECV_PERFORMANCE_TEST_1500, FORM_EVENT_RECV_PERFORMANCE_TEST_1600,
};

FormEvent FormPerformanceTest::event = FormEvent();
sptr<AAFwk::IAbilityManager> FormPerformanceTest::abilityMs = nullptr;
std::shared_ptr<FormPerformanceTest::FormEventSubscriber> FormPerformanceTest::subscriber_ = nullptr;
void FormPerformanceTest::FormEventSubscriber::OnReceiveEvent(const CommonEventData &data)
{
    GTEST_LOG_(INFO) << "OnReceiveEvent: event=" << data.GetWant().GetAction();
    GTEST_LOG_(INFO) << "OnReceiveEvent: data=" << data.GetData();
    GTEST_LOG_(INFO) << "OnReceiveEvent: code=" << data.GetCode();
    SystemTestFormUtil::Completed(event, data.GetWant().GetAction(), data.GetCode(), data.GetData());
}

bool FormPerformanceTest::SavePerformanceTestResult(const std::string &testcase, const int64_t &timeValue)
{
    std::ofstream o(TEST_RESULT_PATH, std::ios::app);
    o.close();

    std::fstream f(TEST_RESULT_PATH, std::ios::app);
    if (!f.good()) {
        return false;
    }

    f << std::setw(FOUR) << testcase << " consuming:" << timeValue << " ms" << std::endl;

    f.close();
    return true;
}

void FormPerformanceTest::SetUpTestCase()
{
    if (!SubscribeEvent()) {
        GTEST_LOG_(INFO) << "SubscribeEvent error";
    }

    PerformanceConfigParser pcp;
    pcp.ParseForPerformance(PERFORMANCE_CONFIG_FILE_PATH, pLevel_);
    std::cout << "performance test level : "
              << "executionTimes : " << pLevel_.executionTimesLevel << std::endl;
}

void FormPerformanceTest::TearDownTestCase()
{
    GTEST_LOG_(INFO) << "UnSubscribeCommonEvent calld";
    CommonEventManager::UnSubscribeCommonEvent(subscriber_);
    std::cout << "========performance test level : "
    << "case execution Times : " << pLevel_.executionTimesLevel << std::endl;
}

void FormPerformanceTest::SetUp()
{
    std::ofstream o(TEST_RESULT_PATH);
    o.close();
    std::fstream f(TEST_RESULT_PATH);
    if (!f.good()) {
        return;
    }
    f << std::setw(FOUR) << "total execution times:" << pLevel_.executionTimesLevel << std::endl;

    f.close();
}

void FormPerformanceTest::TearDown()
{
    SystemTestFormUtil::CleanMsg(event);

    std::ofstream o(TEST_RESULT_PATH, std::ios::app);
    o.close();
    std::fstream f(TEST_RESULT_PATH, std::ios::app);
    if (!f.good()) {
        return;
    }
    f << std::setw(FOUR) << "total time:" << totalTime << " ms" << std::endl;

    f.close();
}
bool FormPerformanceTest::SubscribeEvent()
{
    MatchingSkills matchingSkills;
    for (const auto &e : eventList) {
        matchingSkills.AddEvent(e);
    }
    CommonEventSubscribeInfo subscribeInfo(matchingSkills);
    subscribeInfo.SetPriority(1);
    subscriber_ = std::make_shared<FormEventSubscriber>(subscribeInfo);
    return CommonEventManager::SubscribeCommonEvent(subscriber_);
}

void FormPerformanceTest::PerformanceTest_0100()
{
    struct timespec time1 = {0};
    struct timespec time2 = {0};
    int64_t usecTimesA = 1000000000;
    int64_t usecTimesB = 1000000;
    int64_t consuming = 0;

    clock_gettime(CLOCK_REALTIME, &time1);

    std::string bundleName = "com.ohos.form.manager.performance";
    std::string abilityName = "FormAbilityPerformance";
    MAP_STR_STR params;
    Want want = SystemTestFormUtil::MakeWant("device", abilityName, bundleName, params);
    SystemTestFormUtil::StartAbility(want, abilityMs);
    EXPECT_EQ(SystemTestFormUtil::WaitCompleted(event, FORM_EVENT_ABILITY_ONACTIVED, 0), 0);

    // AcquireForm
    std::string eventData = FORM_EVENT_REQ_PERFORMANCE_TEST_0100;
    SystemTestFormUtil::PublishEvent(FORM_EVENT_REQ_PERFORMANCE_TEST_0100, EVENT_CODE_100, eventData);

    // OnAcquired
    EXPECT_EQ(0, SystemTestFormUtil::WaitCompleted(event, FORM_EVENT_RECV_PERFORMANCE_TEST_0100, EVENT_CODE_101));
    std::string data = SystemTestFormUtil::GetData(event, FORM_EVENT_RECV_PERFORMANCE_TEST_0100, EVENT_CODE_101);
    std::string formId = data;

    // OnUpdate
    EXPECT_EQ(0, SystemTestFormUtil::WaitCompleted(event, FORM_EVENT_RECV_PERFORMANCE_TEST_0100, EVENT_CODE_102));
    data = SystemTestFormUtil::GetData(event, FORM_EVENT_RECV_PERFORMANCE_TEST_0100, EVENT_CODE_102);
    bool result = data == "true";
    EXPECT_TRUE(result);
    if (result) {
        clock_gettime(CLOCK_REALTIME, &time2);
        consuming = (time2.tv_sec * usecTimesA + time2.tv_nsec - (time1.tv_sec * usecTimesA + time1.tv_nsec))
            / usecTimesB;
        totalTime += consuming;
        bool ret = FormPerformanceTest::SavePerformanceTestResult("FMS_performanceTest_0100", consuming);
        std::cout << "SavePerformanceTestResult: "<< ret << "consuming:" << consuming << std::endl;
        GTEST_LOG_(INFO) << "FMS_performanceTest_0100 AcquireForm,  result:" << result;
    }

    SystemTestFormUtil::CleanMsg(event);
}

void FormPerformanceTest::PerformanceTest_0200()
{
    struct timespec time1 = {0};
    struct timespec time2 = {0};
    int64_t usecTimesA = 1000000000;
    int64_t usecTimesB = 1000000;
    int64_t consuming = 0;

    clock_gettime(CLOCK_REALTIME, &time1);

    std::string bundleName = "com.ohos.form.manager.performance";
    std::string abilityName = "FormAbilityPerformance";
    MAP_STR_STR params;
    Want want = SystemTestFormUtil::MakeWant("device", abilityName, bundleName, params);
    SystemTestFormUtil::StartAbility(want, abilityMs);
    EXPECT_EQ(SystemTestFormUtil::WaitCompleted(event, FORM_EVENT_ABILITY_ONACTIVED, 0), 0);

    // DeleteForm
    std::string eventData = FORM_EVENT_REQ_PERFORMANCE_TEST_0100;
    SystemTestFormUtil::PublishEvent(FORM_EVENT_RECV_PERFORMANCE_TEST_0200, EVENT_CODE_200, eventData);

    // DeleteForm Result
    EXPECT_EQ(0, SystemTestFormUtil::WaitCompleted(event, FORM_EVENT_RECV_PERFORMANCE_TEST_0200, EVENT_CODE_200));
    std::string data = SystemTestFormUtil::GetData(event, FORM_EVENT_RECV_PERFORMANCE_TEST_0200, EVENT_CODE_200);
    bool result = data == "true";
    EXPECT_TRUE(result);
    if (result) {
        clock_gettime(CLOCK_REALTIME, &time2);
        consuming = (time2.tv_sec * usecTimesA + time2.tv_nsec - (time1.tv_sec * usecTimesA + time1.tv_nsec))
            / usecTimesB;
        totalTime += consuming;
        bool ret = FormPerformanceTest::SavePerformanceTestResult("FMS_performanceTest_0200", consuming);
        std::cout << "SavePerformanceTestResult: "<< ret << "consuming:" << consuming << std::endl;
        GTEST_LOG_(INFO) << "FMS_performanceTest_0200 DeleteForm,  result:" << result;
    }

    SystemTestFormUtil::CleanMsg(event);
}

void FormPerformanceTest::PerformanceTest_0300()
{
    struct timespec time1 = {0};
    struct timespec time2 = {0};
    int64_t usecTimesA = 1000000000;
    int64_t usecTimesB = 1000000;
    int64_t consuming = 0;

    clock_gettime(CLOCK_REALTIME, &time1);

    std::string bundleName = "com.ohos.form.manager.performance";
    std::string abilityName = "FormAbilityPerformance";
    MAP_STR_STR params;
    Want want = SystemTestFormUtil::MakeWant("device", abilityName, bundleName, params);
    SystemTestFormUtil::StartAbility(want, abilityMs);
    EXPECT_EQ(SystemTestFormUtil::WaitCompleted(event, FORM_EVENT_ABILITY_ONACTIVED, 0), 0);

    // ReleaseForm
    std::string eventData = FORM_EVENT_RECV_PERFORMANCE_TEST_0300;
    SystemTestFormUtil::PublishEvent(FORM_EVENT_RECV_PERFORMANCE_TEST_0300, EVENT_CODE_300, eventData);

    // ReleaseForm Result
    EXPECT_EQ(0, SystemTestFormUtil::WaitCompleted(event, FORM_EVENT_RECV_PERFORMANCE_TEST_0300, EVENT_CODE_300));
    std::string data = SystemTestFormUtil::GetData(event, FORM_EVENT_RECV_PERFORMANCE_TEST_0300, EVENT_CODE_300);
    bool result = data == "true";
    EXPECT_TRUE(result);
    if (result) {
        clock_gettime(CLOCK_REALTIME, &time2);
        consuming = (time2.tv_sec * usecTimesA + time2.tv_nsec - (time1.tv_sec * usecTimesA + time1.tv_nsec))
            / usecTimesB;
        totalTime += consuming;
        bool ret = FormPerformanceTest::SavePerformanceTestResult("FMS_performanceTest_0300", consuming);
        std::cout << "SavePerformanceTestResult: "<< ret << "consuming:" << consuming << std::endl;
        GTEST_LOG_(INFO) << "FMS_performanceTest_0300 ReleaseForm,  result:" << result;
    }

    SystemTestFormUtil::CleanMsg(event);
}

void FormPerformanceTest::PerformanceTest_0400()
{
    struct timespec time1 = {0};
    struct timespec time2 = {0};
    int64_t usecTimesA = 1000000000;
    int64_t usecTimesB = 1000000;
    int64_t consuming = 0;

    clock_gettime(CLOCK_REALTIME, &time1);

    std::string bundleName = "com.ohos.form.manager.performance";
    std::string abilityName = "FormAbilityPerformance";
    MAP_STR_STR params;
    Want want = SystemTestFormUtil::MakeWant("device", abilityName, bundleName, params);
    SystemTestFormUtil::StartAbility(want, abilityMs);
    EXPECT_EQ(SystemTestFormUtil::WaitCompleted(event, FORM_EVENT_ABILITY_ONACTIVED, 0), 0);

    // CastTempForm
    std::string eventData = FORM_EVENT_RECV_PERFORMANCE_TEST_0400;
    SystemTestFormUtil::PublishEvent(FORM_EVENT_RECV_PERFORMANCE_TEST_0400, EVENT_CODE_400, eventData);

    // CastTempForm Result
    EXPECT_EQ(0, SystemTestFormUtil::WaitCompleted(event, FORM_EVENT_RECV_PERFORMANCE_TEST_0400, EVENT_CODE_400));
    std::string data = SystemTestFormUtil::GetData(event, FORM_EVENT_RECV_PERFORMANCE_TEST_0400, EVENT_CODE_400);
    bool result = data == "true";
    EXPECT_TRUE(result);
    if (result) {
        clock_gettime(CLOCK_REALTIME, &time2);
        consuming = (time2.tv_sec * usecTimesA + time2.tv_nsec - (time1.tv_sec * usecTimesA + time1.tv_nsec))
            / usecTimesB;
        totalTime += consuming;
        bool ret = FormPerformanceTest::SavePerformanceTestResult("FMS_performanceTest_0400", consuming);
        std::cout << "SavePerformanceTestResult: "<< ret << "consuming:" << consuming << std::endl;
        GTEST_LOG_(INFO) << "FMS_performanceTest_0400 CastTempForm,  result:" << result;
    }

    SystemTestFormUtil::CleanMsg(event);
}

void FormPerformanceTest::PerformanceTest_0500()
{
    struct timespec time1 = {0};
    struct timespec time2 = {0};
    int64_t usecTimesA = 1000000000;
    int64_t usecTimesB = 1000000;
    int64_t consuming = 0;

    clock_gettime(CLOCK_REALTIME, &time1);

    std::string bundleName = "com.ohos.form.manager.performance";
    std::string abilityName = "FormAbilityPerformance";
    MAP_STR_STR params;
    Want want = SystemTestFormUtil::MakeWant("device", abilityName, bundleName, params);
    SystemTestFormUtil::StartAbility(want, abilityMs);
    EXPECT_EQ(SystemTestFormUtil::WaitCompleted(event, FORM_EVENT_ABILITY_ONACTIVED, 0), 0);

    // NotifyVisibleForms
    std::string eventData = FORM_EVENT_RECV_PERFORMANCE_TEST_0500;
    SystemTestFormUtil::PublishEvent(FORM_EVENT_RECV_PERFORMANCE_TEST_0500, EVENT_CODE_500, eventData);

    // NotifyVisibleForms Result
    EXPECT_EQ(0, SystemTestFormUtil::WaitCompleted(event, FORM_EVENT_RECV_PERFORMANCE_TEST_0500, EVENT_CODE_500));
    std::string data = SystemTestFormUtil::GetData(event, FORM_EVENT_RECV_PERFORMANCE_TEST_0500, EVENT_CODE_500);
    bool result = data == "true";
    EXPECT_TRUE(result);
    if (result) {
        clock_gettime(CLOCK_REALTIME, &time2);
        consuming = (time2.tv_sec * usecTimesA + time2.tv_nsec - (time1.tv_sec * usecTimesA + time1.tv_nsec))
            / usecTimesB;
        totalTime += consuming;
        bool ret = FormPerformanceTest::SavePerformanceTestResult("FMS_performanceTest_0500", consuming);
        std::cout << "SavePerformanceTestResult: "<< ret << "consuming:" << consuming << std::endl;
        GTEST_LOG_(INFO) << "FMS_performanceTest_0500 NotifyVisibleForms,  result:" << result;
    }

    SystemTestFormUtil::CleanMsg(event);
}

void FormPerformanceTest::PerformanceTest_0600()
{
    struct timespec time1 = {0};
    struct timespec time2 = {0};
    int64_t usecTimesA = 1000000000;
    int64_t usecTimesB = 1000000;
    int64_t consuming = 0;

    clock_gettime(CLOCK_REALTIME, &time1);

    std::string bundleName = "com.ohos.form.manager.performance";
    std::string abilityName = "FormAbilityPerformance";
    MAP_STR_STR params;
    Want want = SystemTestFormUtil::MakeWant("device", abilityName, bundleName, params);
    SystemTestFormUtil::StartAbility(want, abilityMs);
    EXPECT_EQ(SystemTestFormUtil::WaitCompleted(event, FORM_EVENT_ABILITY_ONACTIVED, 0), 0);

    // NotifyInvisibleForms
    std::string eventData = FORM_EVENT_RECV_PERFORMANCE_TEST_0600;
    SystemTestFormUtil::PublishEvent(FORM_EVENT_RECV_PERFORMANCE_TEST_0600, EVENT_CODE_600, eventData);

    // NotifyInvisibleForms Result
    EXPECT_EQ(0, SystemTestFormUtil::WaitCompleted(event, FORM_EVENT_RECV_PERFORMANCE_TEST_0600, EVENT_CODE_600));
    std::string data = SystemTestFormUtil::GetData(event, FORM_EVENT_RECV_PERFORMANCE_TEST_0600, EVENT_CODE_600);
    bool result = data == "true";
    EXPECT_TRUE(result);
    if (result) {
        clock_gettime(CLOCK_REALTIME, &time2);
        consuming = (time2.tv_sec * usecTimesA + time2.tv_nsec - (time1.tv_sec * usecTimesA + time1.tv_nsec))
            / usecTimesB;
        totalTime += consuming;
        bool ret = FormPerformanceTest::SavePerformanceTestResult("FMS_performanceTest_0600", consuming);
        std::cout << "SavePerformanceTestResult: "<< ret << "consuming:" << consuming << std::endl;
        GTEST_LOG_(INFO) << "FMS_performanceTest_0600 NotifyInvisibleForms,  result:" << result;
    }

    SystemTestFormUtil::CleanMsg(event);
}

void FormPerformanceTest::PerformanceTest_0700()
{
    struct timespec time1 = {0};
    struct timespec time2 = {0};
    int64_t usecTimesA = 1000000000;
    int64_t usecTimesB = 1000000;
    int64_t consuming = 0;

    clock_gettime(CLOCK_REALTIME, &time1);

    std::string bundleName = "com.ohos.form.manager.performance";
    std::string abilityName = "FormAbilityPerformance";
    MAP_STR_STR params;
    Want want = SystemTestFormUtil::MakeWant("device", abilityName, bundleName, params);
    SystemTestFormUtil::StartAbility(want, abilityMs);
    EXPECT_EQ(SystemTestFormUtil::WaitCompleted(event, FORM_EVENT_ABILITY_ONACTIVED, 0), 0);

    // RequestForm
    std::string eventData = FORM_EVENT_RECV_PERFORMANCE_TEST_0700;
    SystemTestFormUtil::PublishEvent(FORM_EVENT_RECV_PERFORMANCE_TEST_0700, EVENT_CODE_700, eventData);

    // RequestForm Result
    EXPECT_EQ(0, SystemTestFormUtil::WaitCompleted(event, FORM_EVENT_RECV_PERFORMANCE_TEST_0700, EVENT_CODE_700));
    std::string data = SystemTestFormUtil::GetData(event, FORM_EVENT_RECV_PERFORMANCE_TEST_0700, EVENT_CODE_700);
    bool result = data == "true";
    EXPECT_TRUE(result);
    if (result) {
        clock_gettime(CLOCK_REALTIME, &time2);
        consuming = (time2.tv_sec * usecTimesA + time2.tv_nsec - (time1.tv_sec * usecTimesA + time1.tv_nsec))
            / usecTimesB;
        totalTime += consuming;
        bool ret = FormPerformanceTest::SavePerformanceTestResult("FMS_performanceTest_0700", consuming);
        std::cout << "SavePerformanceTestResult: "<< ret << "consuming:" << consuming << std::endl;
        GTEST_LOG_(INFO) << "FMS_performanceTest_0700 RequestForm,  result:" << result;
    }

    SystemTestFormUtil::CleanMsg(event);
}

void FormPerformanceTest::PerformanceTest_1300()
{
    struct timespec time1 = {0};
    struct timespec time2 = {0};
    int64_t usecTimesA = 1000000000;
    int64_t usecTimesB = 1000000;
    int64_t consuming = 0;

    clock_gettime(CLOCK_REALTIME, &time1);

    std::string bundleName = "com.ohos.form.manager.performance";
    std::string abilityName = "FormAbilityPerformance";
    MAP_STR_STR params;
    Want want = SystemTestFormUtil::MakeWant("device", abilityName, bundleName, params);
    SystemTestFormUtil::StartAbility(want, abilityMs);
    EXPECT_EQ(SystemTestFormUtil::WaitCompleted(event, FORM_EVENT_ABILITY_ONACTIVED, 0), 0);

    // GetAllFormsInfo
    std::string eventData = FORM_EVENT_RECV_PERFORMANCE_TEST_1300;
    SystemTestFormUtil::PublishEvent(FORM_EVENT_RECV_PERFORMANCE_TEST_1300, EVENT_CODE_1300, eventData);

    // GetAllFormsInfo Result
    EXPECT_EQ(0, SystemTestFormUtil::WaitCompleted(event, FORM_EVENT_RECV_PERFORMANCE_TEST_1300, EVENT_CODE_1300));
    std::string data = SystemTestFormUtil::GetData(event, FORM_EVENT_RECV_PERFORMANCE_TEST_1300, EVENT_CODE_1300);
    bool result = data == "true";
    EXPECT_TRUE(result);
    if (result) {
        clock_gettime(CLOCK_REALTIME, &time2);
        consuming = (time2.tv_sec * usecTimesA + time2.tv_nsec - (time1.tv_sec * usecTimesA + time1.tv_nsec))
            / usecTimesB;
        totalTime += consuming;
        bool ret = FormPerformanceTest::SavePerformanceTestResult("FMS_performanceTest_1300", consuming);
        std::cout << "SavePerformanceTestResult: "<< ret << "consuming:" << consuming << std::endl;
        GTEST_LOG_(INFO) << "FMS_performanceTest_1300 RequestForm,  result:" << result;
    }

    SystemTestFormUtil::CleanMsg(event);
}

void FormPerformanceTest::PerformanceTest_1400()
{
    struct timespec time1 = {0};
    struct timespec time2 = {0};
    int64_t usecTimesA = 1000000000;
    int64_t usecTimesB = 1000000;
    int64_t consuming = 0;

    clock_gettime(CLOCK_REALTIME, &time1);

    std::string bundleName = "com.ohos.form.manager.performance";
    std::string abilityName = "FormAbilityPerformance";
    MAP_STR_STR params;
    Want want = SystemTestFormUtil::MakeWant("device", abilityName, bundleName, params);
    SystemTestFormUtil::StartAbility(want, abilityMs);
    EXPECT_EQ(SystemTestFormUtil::WaitCompleted(event, FORM_EVENT_ABILITY_ONACTIVED, 0), 0);

    // GetAllFormsInfo
    std::string eventData = FORM_EVENT_RECV_PERFORMANCE_TEST_1300;
    SystemTestFormUtil::PublishEvent(FORM_EVENT_RECV_PERFORMANCE_TEST_1400, EVENT_CODE_1400, eventData);

    // GetAllFormsInfo Result
    EXPECT_EQ(0, SystemTestFormUtil::WaitCompleted(event, FORM_EVENT_RECV_PERFORMANCE_TEST_1400, EVENT_CODE_1400));
    std::string data = SystemTestFormUtil::GetData(event, FORM_EVENT_RECV_PERFORMANCE_TEST_1400, EVENT_CODE_1400);
    bool result = data == "true";
    EXPECT_TRUE(result);
    if (result) {
        clock_gettime(CLOCK_REALTIME, &time2);
        consuming = (time2.tv_sec * usecTimesA + time2.tv_nsec - (time1.tv_sec * usecTimesA + time1.tv_nsec))
            / usecTimesB;
        totalTime += consuming;
        bool ret = FormPerformanceTest::SavePerformanceTestResult("FMS_performanceTest_1400", consuming);
        std::cout << "SavePerformanceTestResult: "<< ret << "consuming:" << consuming << std::endl;
        GTEST_LOG_(INFO) << "FMS_performanceTest_1400 RequestForm,  result:" << result;
    }

    SystemTestFormUtil::CleanMsg(event);
}

void FormPerformanceTest::PerformanceTest_1500()
{
    struct timespec time1 = {0};
    struct timespec time2 = {0};
    int64_t usecTimesA = 1000000000;
    int64_t usecTimesB = 1000000;
    int64_t consuming = 0;

    clock_gettime(CLOCK_REALTIME, &time1);

    std::string bundleName = "com.ohos.form.manager.performance";
    std::string abilityName = "FormAbilityPerformance";
    MAP_STR_STR params;
    Want want = SystemTestFormUtil::MakeWant("device", abilityName, bundleName, params);
    SystemTestFormUtil::StartAbility(want, abilityMs);
    EXPECT_EQ(SystemTestFormUtil::WaitCompleted(event, FORM_EVENT_ABILITY_ONACTIVED, 0), 0);

    // GetAllFormsInfo
    std::string eventData = FORM_EVENT_RECV_PERFORMANCE_TEST_1500;
    SystemTestFormUtil::PublishEvent(FORM_EVENT_RECV_PERFORMANCE_TEST_1500, EVENT_CODE_1500, eventData);

    // GetAllFormsInfo Result
    EXPECT_EQ(0, SystemTestFormUtil::WaitCompleted(event, FORM_EVENT_RECV_PERFORMANCE_TEST_1500, EVENT_CODE_1500));
    std::string data = SystemTestFormUtil::GetData(event, FORM_EVENT_RECV_PERFORMANCE_TEST_1500, EVENT_CODE_1500);
    bool result = data == "true";
    EXPECT_TRUE(result);
    if (result) {
        clock_gettime(CLOCK_REALTIME, &time2);
        consuming = (time2.tv_sec * usecTimesA + time2.tv_nsec - (time1.tv_sec * usecTimesA + time1.tv_nsec))
            / usecTimesB;
        totalTime += consuming;
        bool ret = FormPerformanceTest::SavePerformanceTestResult("FMS_performanceTest_1500", consuming);
        std::cout << "SavePerformanceTestResult: "<< ret << "consuming:" << consuming << std::endl;
        GTEST_LOG_(INFO) << "FMS_performanceTest_1500 RequestForm,  result:" << result;
    }

    SystemTestFormUtil::CleanMsg(event);
}

void FormPerformanceTest::PerformanceTest_1600()
{
    struct timespec time1 = {0};
    struct timespec time2 = {0};
    int64_t usecTimesA = 1000000000;
    int64_t usecTimesB = 1000000;
    int64_t consuming = 0;

    clock_gettime(CLOCK_REALTIME, &time1);

    std::string bundleName = "com.ohos.form.manager.performance";
    std::string abilityName = "FormAbilityPerformance";
    MAP_STR_STR params;
    Want want = SystemTestFormUtil::MakeWant("device", abilityName, bundleName, params);
    SystemTestFormUtil::StartAbility(want, abilityMs);
    EXPECT_EQ(SystemTestFormUtil::WaitCompleted(event, FORM_EVENT_ABILITY_ONACTIVED, 0), 0);

    // GetAllFormsInfo
    std::string eventData = FORM_EVENT_RECV_PERFORMANCE_TEST_1600;
    SystemTestFormUtil::PublishEvent(FORM_EVENT_RECV_PERFORMANCE_TEST_1600, EVENT_CODE_1600, eventData);

    // GetAllFormsInfo Result
    EXPECT_EQ(0, SystemTestFormUtil::WaitCompleted(event, FORM_EVENT_RECV_PERFORMANCE_TEST_1600, EVENT_CODE_1600));
    std::string data = SystemTestFormUtil::GetData(event, FORM_EVENT_RECV_PERFORMANCE_TEST_1600, EVENT_CODE_1600);
    bool result = data == "true";
    EXPECT_TRUE(result);
    if (result) {
        clock_gettime(CLOCK_REALTIME, &time2);
        consuming = (time2.tv_sec * usecTimesA + time2.tv_nsec - (time1.tv_sec * usecTimesA + time1.tv_nsec))
            / usecTimesB;
        totalTime += consuming;
        bool ret = FormPerformanceTest::SavePerformanceTestResult("FMS_performanceTest_1600", consuming);
        std::cout << "SavePerformanceTestResult: "<< ret << "consuming:" << consuming << std::endl;
        GTEST_LOG_(INFO) << "FMS_performanceTest_1600 RequestForm,  result:" << result;
    }

    SystemTestFormUtil::CleanMsg(event);
}

/**
 * @tc.number    : FMS_performanceTest_0100
 * @tc.name      : AcquireForm performance test
 * @tc.desc      : Test the time required to AcquireForm
 */
HWTEST_F(FormPerformanceTest, FMS_performanceTest_0100, Function | MediumTest | Level3)
{
    std::cout << "START FMS_performanceTest_0100" << std::endl;

    for (int iExecutionTimes = 0; iExecutionTimes < pLevel_.executionTimesLevel; iExecutionTimes++) {
        FormPerformanceTest::PerformanceTest_0100();
        std::cout << "FMS_performanceTest_0100 ExecutionTimes:" << iExecutionTimes + 1 << std::endl;
    }

    std::cout << "END FMS_performanceTest_0100" << std::endl;
}

/**
 * @tc.number    : FMS_performanceTest_0400
 * @tc.name      : CastTempForm performance test
 * @tc.desc      : Test the time required to CastTempForm
 */
HWTEST_F(FormPerformanceTest, FMS_performanceTest_0400, Function | MediumTest | Level3)
{
    std::cout << "START FMS_performanceTest_0400" << std::endl;

    for (int iExecutionTimes = 0; iExecutionTimes < pLevel_.executionTimesLevel; iExecutionTimes++) {
        FormPerformanceTest::PerformanceTest_0400();
        std::cout << "FMS_performanceTest_0400 ExecutionTimes:" << iExecutionTimes + 1 << std::endl;
    }

    std::cout << "END FMS_performanceTest_0400" << std::endl;
}

/**
 * @tc.number    : FMS_performanceTest_0300
 * @tc.name      : ReleaseForm performance test
 * @tc.desc      : Test the time required to ReleaseForm
 */
HWTEST_F(FormPerformanceTest, FMS_performanceTest_0300, Function | MediumTest | Level3)
{
    std::cout << "START FMS_performanceTest_0300" << std::endl;

    for (int iExecutionTimes = 0; iExecutionTimes < pLevel_.executionTimesLevel; iExecutionTimes++) {
        FormPerformanceTest::PerformanceTest_0300();
        std::cout << "FMS_performanceTest_0300 ExecutionTimes:" << iExecutionTimes + 1 << std::endl;
    }

    std::cout << "END FMS_performanceTest_0300" << std::endl;
}

/**
 * @tc.number    : FMS_performanceTest_0200
 * @tc.name      : DeleteForm performance test
 * @tc.desc      : Test the time required to DeleteForm
 */
HWTEST_F(FormPerformanceTest, FMS_performanceTest_0200, Function | MediumTest | Level3)
{
    std::cout << "START FMS_performanceTest_0200" << std::endl;

    for (int iExecutionTimes = 0; iExecutionTimes < pLevel_.executionTimesLevel; iExecutionTimes++) {
        FormPerformanceTest::PerformanceTest_0200();
        std::cout << "FMS_performanceTest_0200 ExecutionTimes:" << iExecutionTimes + 1 << std::endl;
    }

    std::cout << "END FMS_performanceTest_0200" << std::endl;
}

/**
 * @tc.number    : FMS_performanceTest_0500
 * @tc.name      : NotifyVisibleForms performance test
 * @tc.desc      : Test the time required to NotifyVisibleForms
 */
HWTEST_F(FormPerformanceTest, FMS_performanceTest_0500, Function | MediumTest | Level3)
{
    std::cout << "START FMS_performanceTest_0500" << std::endl;

    for (int iExecutionTimes = 0; iExecutionTimes < pLevel_.executionTimesLevel; iExecutionTimes++) {
        FormPerformanceTest::PerformanceTest_0500();
        std::cout << "FMS_performanceTest_0500 ExecutionTimes:" << iExecutionTimes + 1 << std::endl;
    }

    std::cout << "END FMS_performanceTest_0500" << std::endl;
}

/**
 * @tc.number    : FMS_performanceTest_0600
 * @tc.name      : NotifyInvisibleForms performance test
 * @tc.desc      : Test the time required to NotifyInvisibleForms
 */
HWTEST_F(FormPerformanceTest, FMS_performanceTest_0600, Function | MediumTest | Level3)
{
    std::cout << "START FMS_performanceTest_0600" << std::endl;

    for (int iExecutionTimes = 0; iExecutionTimes < pLevel_.executionTimesLevel; iExecutionTimes++) {
        FormPerformanceTest::PerformanceTest_0600();
        std::cout << "FMS_performanceTest_0600 ExecutionTimes:" << iExecutionTimes + 1 << std::endl;
    }

    std::cout << "END FMS_performanceTest_0600" << std::endl;
}

/**
 * @tc.number    : FMS_performanceTest_0700
 * @tc.name      : RequestForm performance test
 * @tc.desc      : Test the time required to RequestForm
 */
HWTEST_F(FormPerformanceTest, FMS_performanceTest_0700, Function | MediumTest | Level3)
{
    std::cout << "START FMS_performanceTest_0700" << std::endl;

    for (int iExecutionTimes = 0; iExecutionTimes < pLevel_.executionTimesLevel; iExecutionTimes++) {
        FormPerformanceTest::PerformanceTest_0700();
        std::cout << "FMS_performanceTest_0700 ExecutionTimes:" << iExecutionTimes + 1 << std::endl;
    }

    std::cout << "END FMS_performanceTest_0700" << std::endl;
}

/**
 * @tc.number    : FMS_performanceTest_1300
 * @tc.name      : GetAllFormsInfo performance test
 * @tc.desc      : Test the time required to GetAllFormsInfo
 */
HWTEST_F(FormPerformanceTest, FMS_performanceTest_1300, Function | MediumTest | Level3)
{
    std::cout << "START FMS_performanceTest_1300" << std::endl;

    for (int iExecutionTimes = 0; iExecutionTimes < pLevel_.executionTimesLevel; iExecutionTimes++) {
        FormPerformanceTest::PerformanceTest_1300();
        std::cout << "FMS_performanceTest_1300 ExecutionTimes:" << iExecutionTimes + 1 << std::endl;
    }

    std::cout << "END FMS_performanceTest_1300" << std::endl;
}

/**
 * @tc.number    : FMS_performanceTest_1400
 * @tc.name      : GetFormsInfoByApp performance test
 * @tc.desc      : Test the time required to GetFormsInfoByApp
 */
HWTEST_F(FormPerformanceTest, FMS_performanceTest_1400, Function | MediumTest | Level3)
{
    std::cout << "START FMS_performanceTest_1400" << std::endl;

    for (int iExecutionTimes = 0; iExecutionTimes < pLevel_.executionTimesLevel; iExecutionTimes++) {
        FormPerformanceTest::PerformanceTest_1400();
        std::cout << "FMS_performanceTest_1400 ExecutionTimes:" << iExecutionTimes + 1 << std::endl;
    }

    std::cout << "END FMS_performanceTest_1400" << std::endl;
}

/**
 * @tc.number    : FMS_performanceTest_1500
 * @tc.name      : GetFormsInfoByModule performance test
 * @tc.desc      : Test the time required to GetFormsInfoByModule
 */
HWTEST_F(FormPerformanceTest, FMS_performanceTest_1500, Function | MediumTest | Level3)
{
    std::cout << "START FMS_performanceTest_1500" << std::endl;

    for (int iExecutionTimes = 0; iExecutionTimes < pLevel_.executionTimesLevel; iExecutionTimes++) {
        FormPerformanceTest::PerformanceTest_1500();
        std::cout << "FMS_performanceTest_1500 ExecutionTimes:" << iExecutionTimes + 1 << std::endl;
    }

    std::cout << "END FMS_performanceTest_1500" << std::endl;
}

/**
 * @tc.number    : FMS_performanceTest_1600
 * @tc.name      : CheckFMSReady performance test
 * @tc.desc      : Test the time required to CheckFMSReady
 */
HWTEST_F(FormPerformanceTest, FMS_performanceTest_1600, Function | MediumTest | Level3)
{
    std::cout << "START FMS_performanceTest_1600" << std::endl;

    for (int iExecutionTimes = 0; iExecutionTimes < pLevel_.executionTimesLevel; iExecutionTimes++) {
        FormPerformanceTest::PerformanceTest_1600();
        std::cout << "FMS_performanceTest_1600 ExecutionTimes:" << iExecutionTimes + 1 << std::endl;
    }

    std::cout << "END FMS_performanceTest_1600" << std::endl;
}
}  // namespace AppExecFwk
}  // namespace OHOS