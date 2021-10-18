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
    "com.ohos.form.manager.notsystemapp",
    "com.form.formsystemtestservicea",
    "com.form.formsystemtestserviceb",
};
std::vector<std::string> hapNameList = {
    "fmsSystemTestHostNotSys-signed",
    "formSystemTestServiceA-signed",
    "formSystemTestServiceB-signed",
};

std::string catchFormId; // FormId when creating a cached form
}  // namespace

namespace OHOS {
namespace AppExecFwk {
class FmsAcquireFormTest : public testing::Test {
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
    static std::shared_ptr<FormEventSubscriber> subscriber;

    void FmsAcquireFormCatched1500(std::string strFormId);
    void FmsAcquireForm1600();
    void FmsAcquireForm1600A();
    void FmsAcquireForm1800();
    void FmsAcquireForm1800A();
    void FmsAcquireForm1800B();
    void FmsAcquireForm1800C();
    std::string FmsAcquireForm2400();
    void FmsAcquireForm2400_1(std::string strFormId);
    void FmsAcquireFormDeleteA(std::string strFormId);
};

std::vector<std::string> FmsAcquireFormTest::eventList = {
    FORM_EVENT_RECV_DELETE_FORM_COMMON,
    FORM_EVENT_ABILITY_ONACTIVED, FORM_EVENT_RECV_ACQUIRE_FORM_0100, FORM_EVENT_RECV_ACQUIRE_FORM_0200,
    FORM_EVENT_RECV_ACQUIRE_FORM_0300, FORM_EVENT_RECV_ACQUIRE_FORM_0400, FORM_EVENT_RECV_ACQUIRE_FORM_0500,
    FORM_EVENT_RECV_ACQUIRE_FORM_0600, FORM_EVENT_RECV_ACQUIRE_FORM_0700, FORM_EVENT_RECV_ACQUIRE_FORM_1000,
    FORM_EVENT_RECV_ACQUIRE_FORM_1100, FORM_EVENT_RECV_ACQUIRE_FORM_1200, FORM_EVENT_RECV_ACQUIRE_FORM_1400,
    FORM_EVENT_RECV_ACQUIRE_FORM_1500, FORM_EVENT_RECV_ACQUIRE_FORM_1600, FORM_EVENT_RECV_ACQUIRE_FORM_1600_1,
    FORM_EVENT_RECV_ACQUIRE_FORM_1800, FORM_EVENT_RECV_ACQUIRE_FORM_1800_1, FORM_EVENT_RECV_ACQUIRE_FORM_1800_2,
    FORM_EVENT_RECV_ACQUIRE_FORM_1800_3, FORM_EVENT_RECV_ACQUIRE_FORM_1900, FORM_EVENT_RECV_ACQUIRE_FORM_2100,
    FORM_EVENT_RECV_ACQUIRE_FORM_2200, FORM_EVENT_RECV_ACQUIRE_FORM_2300, FORM_EVENT_RECV_ACQUIRE_FORM_2500,
    FORM_EVENT_RECV_ACQUIRE_FORM_1500_1, FORM_EVENT_RECV_ACQUIRE_FORM_2600, FORM_EVENT_RECV_ACQUIRE_FORM_2600_1,
    FORM_EVENT_RECV_ACQUIRE_FORM_2400, FORM_EVENT_RECV_ACQUIRE_FORM_2400_1,
};


FormEvent FmsAcquireFormTest::event = FormEvent();
sptr<AAFwk::IAbilityManager> FmsAcquireFormTest::abilityMs = nullptr;
std::shared_ptr<FmsAcquireFormTest::FormEventSubscriber> FmsAcquireFormTest::subscriber = nullptr;
void FmsAcquireFormTest::FormEventSubscriber::OnReceiveEvent(const CommonEventData &data)
{
    GTEST_LOG_(INFO) << "OnReceiveEvent: event=" << data.GetWant().GetAction();
    GTEST_LOG_(INFO) << "OnReceiveEvent: data=" << data.GetData();
    GTEST_LOG_(INFO) << "OnReceiveEvent: code=" << data.GetCode();
    SystemTestFormUtil::Completed(event, data.GetWant().GetAction(), data.GetCode(), data.GetData());
}

void FmsAcquireFormTest::SetUpTestCase()
{
    // SystemTestFormUtil::InstallHaps(hapNameList);
    if (!SubscribeEvent()) {
        GTEST_LOG_(INFO) << "SubscribeEvent error";
    }
}

void FmsAcquireFormTest::TearDownTestCase()
{
    // SystemTestFormUtil::UninstallBundle(bundleNameList);
    GTEST_LOG_(INFO) << "UnSubscribeCommonEvent calld";
    CommonEventManager::UnSubscribeCommonEvent(subscriber);
}

void FmsAcquireFormTest::SetUp()
{
}

void FmsAcquireFormTest::TearDown()
{
    GTEST_LOG_(INFO) << "CleanMsg calld";
    SystemTestFormUtil::CleanMsg(event);
}
bool FmsAcquireFormTest::SubscribeEvent()
{
    GTEST_LOG_(INFO) << "SubscribeEvent calld";
    MatchingSkills matchingSkills;
    for (const auto &e : eventList) {
        matchingSkills.AddEvent(e);
    }
    CommonEventSubscribeInfo subscribeInfo(matchingSkills);
    subscribeInfo.SetPriority(1);
    subscriber = std::make_shared<FormEventSubscriber>(subscribeInfo);
    return CommonEventManager::SubscribeCommonEvent(subscriber);
}

/**
 * @tc.number: FMS_acquireForm_1100
 * @tc.name: When the formId is set to 0, a non cached form is added.
 * @tc.desc: Verify that the non cache form can be successfully created.
 */
HWTEST_F(FmsAcquireFormTest, FMS_acquireForm_1100, Function | MediumTest | Level1)
{
    sleep(1);
    std::cout << "START FMS_acquireForm_1100" << std::endl;
    std::string bundleName = "com.ohos.form.manager.normal";
    std::string abilityName = "FormAbilityA";
    MAP_STR_STR params;
    Want want = SystemTestFormUtil::MakeWant("device", abilityName, bundleName, params);
    SystemTestFormUtil::StartAbility(want, abilityMs);
    EXPECT_EQ(SystemTestFormUtil::WaitCompleted(event, FORM_EVENT_ABILITY_ONACTIVED, 0), 0);

    std::string eventData = FORM_EVENT_REQ_ACQUIRE_FORM_1100;
    SystemTestFormUtil::PublishEvent(FORM_EVENT_REQ_ACQUIRE_FORM_1100, EVENT_CODE_1100, eventData);
    EXPECT_EQ(0, SystemTestFormUtil::WaitCompleted(event, FORM_EVENT_RECV_ACQUIRE_FORM_1100, EVENT_CODE_1100));
    std::string strFormId = SystemTestFormUtil::GetData(event, FORM_EVENT_RECV_ACQUIRE_FORM_1100, EVENT_CODE_1100);
    bool result = !strFormId.empty();
    EXPECT_TRUE(result);
    if (!result) {
        GTEST_LOG_(INFO) << "FMS_acquireForm_1100,  result:" << result;
    } else {
        GTEST_LOG_(INFO) << "FMS_acquireForm_1100,  formId:" << strFormId;
    }

    catchFormId = strFormId;

    EXPECT_EQ(0, SystemTestFormUtil::WaitCompleted(event, FORM_EVENT_RECV_ACQUIRE_FORM_1100, EVENT_CODE_1101));
    std::string data2 = SystemTestFormUtil::GetData(event, FORM_EVENT_RECV_ACQUIRE_FORM_1100, EVENT_CODE_1101);
    bool result2 = !data2.empty();
    EXPECT_TRUE(result2);
    if (!result2) {
        GTEST_LOG_(INFO) << "FMS_acquireForm_1100,  result:" << result2;
    } else {
        GTEST_LOG_(INFO) << "FMS_acquireForm_1100,  formData:" << data2;
    }
    int64_t formId = std::stoll(strFormId);
    std::string cachedData = SystemTestFormUtil::QueryFormInfoByFormId(formId);
    GTEST_LOG_(INFO) << "FMS_acquireForm_1100,  cachedData:" << cachedData;

    std::cout << "END FMS_acquireForm_1100" << std::endl;
}
/**
 * @tc.number: FMS_acquireForm_1200
 * @tc.name: When the formId is set to 0, a non cached form is added.
 * @tc.desc: Verify that the non cache form can be successfully created.
 */
HWTEST_F(FmsAcquireFormTest, FMS_acquireForm_1200, Function | MediumTest | Level1)
{
    sleep(1);
    std::cout << "START FMS_acquireForm_1200" << std::endl;
    std::string bundleName = "com.ohos.form.manager.normal";
    std::string abilityName = "FormAbilityA";
    MAP_STR_STR params;
    Want want = SystemTestFormUtil::MakeWant("device", abilityName, bundleName, params);
    SystemTestFormUtil::StartAbility(want, abilityMs);
    EXPECT_EQ(SystemTestFormUtil::WaitCompleted(event, FORM_EVENT_ABILITY_ONACTIVED, 0), 0);

    std::string eventData = FORM_EVENT_REQ_ACQUIRE_FORM_1200;
    SystemTestFormUtil::PublishEvent(FORM_EVENT_REQ_ACQUIRE_FORM_1200, EVENT_CODE_1200, eventData);
    EXPECT_EQ(0, SystemTestFormUtil::WaitCompleted(event, FORM_EVENT_RECV_ACQUIRE_FORM_1200, EVENT_CODE_1200));
    std::string strFormId = SystemTestFormUtil::GetData(event, FORM_EVENT_RECV_ACQUIRE_FORM_1200, EVENT_CODE_1200);
    bool result = !strFormId.empty();
    EXPECT_TRUE(result);
    if (!result) {
        GTEST_LOG_(INFO) << "FMS_acquireForm_1200,  result:" << result;
    } else {
        GTEST_LOG_(INFO) << "FMS_acquireForm_1200,  formId:" << strFormId;
    }

    EXPECT_EQ(0, SystemTestFormUtil::WaitCompleted(event, FORM_EVENT_RECV_ACQUIRE_FORM_1200, EVENT_CODE_1201));
    std::string data2 = SystemTestFormUtil::GetData(event, FORM_EVENT_RECV_ACQUIRE_FORM_1200, EVENT_CODE_1201);
    bool result2 = !data2.empty();
    EXPECT_TRUE(result2);
    if (!result2) {
        GTEST_LOG_(INFO) << "FMS_acquireForm_1200,  result:" << result2;
    } else {
        GTEST_LOG_(INFO) << "FMS_acquireForm_1200,  formData:" << data2;
    }
    
    int64_t formId = std::stoll(strFormId);
    std::string cachedData = SystemTestFormUtil::QueryFormInfoByFormId(formId);
    GTEST_LOG_(INFO) << "FMS_acquireForm_1200,  cachedData:" << cachedData;

    // wait delete form
    EXPECT_EQ(0, SystemTestFormUtil::WaitCompleted(event, FORM_EVENT_RECV_DELETE_FORM_COMMON, EVENT_CODE_999));
    std::string data3 = SystemTestFormUtil::GetData(event, FORM_EVENT_RECV_DELETE_FORM_COMMON, EVENT_CODE_999);
    bool result3 = data3 == "true";
    EXPECT_TRUE(result3);
    GTEST_LOG_(INFO) << "FMS_acquireForm_1200,  delete form, result:" << result3;
    
    std::cout << "END FMS_acquireForm_1200" << std::endl;
}

/**
 * @tc.number: FMS_acquireForm_1500
 * @tc.name: Add a cached form.
 * @tc.desc: Verify that the cached form can be successfully created.
 */
HWTEST_F(FmsAcquireFormTest, FMS_acquireForm_1500, Function | MediumTest | Level1)
{
    sleep(1);
    std::cout << "START FMS_acquireForm_1500" << std::endl;
    std::string bundleName = "com.ohos.form.manager.normal";
    std::string abilityName = "FormAbilityA";
    MAP_STR_STR params;
    Want want = SystemTestFormUtil::MakeWant("device", abilityName, bundleName, params);
    SystemTestFormUtil::StartAbility(want, abilityMs);
    EXPECT_EQ(SystemTestFormUtil::WaitCompleted(event, FORM_EVENT_ABILITY_ONACTIVED, 0), 0);
    GTEST_LOG_(INFO) << "FMS_acquireForm_1500,  add no catch form start";
    std::string eventData = FORM_EVENT_REQ_ACQUIRE_FORM_1500_1;
    SystemTestFormUtil::PublishEvent(FORM_EVENT_REQ_ACQUIRE_FORM_1500_1, EVENT_CODE_1510, eventData);
    EXPECT_EQ(0, SystemTestFormUtil::WaitCompleted(event, FORM_EVENT_RECV_ACQUIRE_FORM_1500_1, EVENT_CODE_1510));
    std::string strFormId = SystemTestFormUtil::GetData(event, FORM_EVENT_RECV_ACQUIRE_FORM_1500_1, EVENT_CODE_1510);
    bool result = !strFormId.empty();
    EXPECT_TRUE(result);
    if (!result) {
        GTEST_LOG_(INFO) << "FMS_acquireForm_1500,  result:" << result;
    } else {
        GTEST_LOG_(INFO) << "FMS_acquireForm_1500,  formId:" << strFormId;
    }
    EXPECT_EQ(0, SystemTestFormUtil::WaitCompleted(event, FORM_EVENT_RECV_ACQUIRE_FORM_1500_1, EVENT_CODE_1511));
    std::string data2 = SystemTestFormUtil::GetData(event, FORM_EVENT_RECV_ACQUIRE_FORM_1500_1, EVENT_CODE_1511);
    bool result2 = !data2.empty();
    EXPECT_TRUE(result2);
    if (!result2) {
        GTEST_LOG_(INFO) << "FMS_acquireForm_1500,  result:" << result2;
    } else {
        GTEST_LOG_(INFO) << "FMS_acquireForm_1500,  formData:" << data2;
    }
    GTEST_LOG_(INFO) << "FMS_acquireForm_1500,  add no catch form end";

    sleep(1);
    FmsAcquireFormCatched1500(strFormId);
    
    sleep(1);
    FmsAcquireFormDeleteA(strFormId);
    std::cout << "END FMS_acquireForm_1500" << std::endl;
}

/**
 * @tc.number: FMS_acquireForm_1600
 * @tc.name: A single form host (using a single thread) continuously creates 2 normal and 2 temporary forms.
 * @tc.desc: Verify that a single host (using a single thread) can successfully create multiple normal
 *           and temporary forms continuously.
 */
HWTEST_F(FmsAcquireFormTest, FMS_acquireForm_1600, Function | MediumTest | Level1)
{
    sleep(1);
    int normalCount = 0;
    int tempCount = 0;
    for (int iCount = 0; iCount < 2; iCount++) {
        sleep(1);
        FmsAcquireForm1600();
        normalCount++;
        std::cout << "END FMS_acquireForm_1600, normal form end, count:"  << normalCount << std::endl;
        sleep(1);
        FmsAcquireForm1600A();
        tempCount++;
        std::cout << "END FMS_acquireForm_1600_1, temp form end, count:"  << tempCount << std::endl;
    }
}

/**
 * @tc.number: FMS_acquireForm_1800
 * @tc.name: Multiple form hosts create one normal form and one temporary form respectively.
 * @tc.desc: Verify that multiple hosts can successfully create multiple normal and temporary forms respectively.
 */
HWTEST_F(FmsAcquireFormTest, FMS_acquireForm_1800, Function | MediumTest | Level1)
{
    sleep(1);
    FmsAcquireForm1800();
    sleep(1);
    FmsAcquireForm1800A();
    sleep(1);
    FmsAcquireForm1800B();
    sleep(1);
    FmsAcquireForm1800C();
}

/**
 * @tc.number: FMS_acquireForm_1900
 * @tc.name: Add a temporary form when the formId is set to 0.
 * @tc.desc: When the verification formId is set to 0, the temporary form is created successfully.
 */
HWTEST_F(FmsAcquireFormTest, FMS_acquireForm_1900, Function | MediumTest | Level1)
{
    sleep(1);
    std::cout << "START FMS_acquireForm_1900" << std::endl;
    std::string bundleName = "com.ohos.form.manager.normal";
    std::string abilityName = "FormAbilityA";
    MAP_STR_STR params;
    Want want = SystemTestFormUtil::MakeWant("device", abilityName, bundleName, params);
    SystemTestFormUtil::StartAbility(want, abilityMs);
    EXPECT_EQ(SystemTestFormUtil::WaitCompleted(event, FORM_EVENT_ABILITY_ONACTIVED, 0), 0);

    std::string eventData = FORM_EVENT_REQ_ACQUIRE_FORM_1900;
    SystemTestFormUtil::PublishEvent(FORM_EVENT_REQ_ACQUIRE_FORM_1900, EVENT_CODE_1900, eventData);
    EXPECT_EQ(0, SystemTestFormUtil::WaitCompleted(event, FORM_EVENT_RECV_ACQUIRE_FORM_1900, EVENT_CODE_1900));
    std::string strFormId = SystemTestFormUtil::GetData(event, FORM_EVENT_RECV_ACQUIRE_FORM_1900, EVENT_CODE_1900);
    bool result = !strFormId.empty();
    EXPECT_TRUE(result);
    if (!result) {
        GTEST_LOG_(INFO) << "FMS_acquireForm_1900,  result:" << result;
    } else {
        GTEST_LOG_(INFO) << "FMS_acquireForm_1900, normal, formId:" << strFormId;
    }

    EXPECT_EQ(0, SystemTestFormUtil::WaitCompleted(event, FORM_EVENT_RECV_ACQUIRE_FORM_1900, EVENT_CODE_1901));
    std::string data2 = SystemTestFormUtil::GetData(event, FORM_EVENT_RECV_ACQUIRE_FORM_1900, EVENT_CODE_1901);
    bool result2 = !data2.empty();
    EXPECT_TRUE(result2);
    if (!result2) {
        GTEST_LOG_(INFO) << "FMS_acquireForm_1900,  result:" << result2;
    } else {
        GTEST_LOG_(INFO) << "FMS_acquireForm_1900,  normal, formData:" << data2;
    }

    // wait delete form
    EXPECT_EQ(0, SystemTestFormUtil::WaitCompleted(event, FORM_EVENT_RECV_DELETE_FORM_COMMON, EVENT_CODE_999));
    std::string data3 = SystemTestFormUtil::GetData(event, FORM_EVENT_RECV_DELETE_FORM_COMMON, EVENT_CODE_999);
    bool result3 = data3 == "true";
    EXPECT_TRUE(result3);
    if (!result3) {
        GTEST_LOG_(INFO) << "FMS_acquireForm_1900,  delete form, result:" << result3;
    }

    std::cout << "END FMS_acquireForm_1900" << std::endl;
}

/**
 * @tc.number: FMS_acquireForm_2400
 * @tc.name: FormId does not match other parameters when adding a cached form.
 * @tc.desc: Failed to create a cached form when verifying that FormId does not match other parameters.
 */
HWTEST_F(FmsAcquireFormTest, FMS_acquireForm_2400, Function | MediumTest | Level1)
{
    sleep(1);
    std::string strFormId = FmsAcquireForm2400();
    sleep(1);
    FmsAcquireForm2400_1(strFormId);
    sleep(1);
    FmsAcquireFormDeleteA(strFormId);
}

/**
 * @tc.number: FMS_acquireForm_2600
 * @tc.name: When the normal form does not reach the maximum value created by the form
 *           (the single host is 256 and the FMS is 512), the temporary form changes to the normal form.
 * @tc.desc: Verify that when the normal form does not reach the maximum value created by the form.
 *           (the single host is 256 and the FMS is 512), the temporary form can be successfully
 *           casted to the normal form.
 */
HWTEST_F(FmsAcquireFormTest, FMS_acquireForm_2600, Function | MediumTest | Level1)
{
    sleep(1);
    std::cout << "START FMS_acquireForm_2600" << std::endl;
    std::string bundleName = "com.ohos.form.manager.normal";
    std::string abilityName = "FormAbilityA";
    MAP_STR_STR params;
    Want want = SystemTestFormUtil::MakeWant("device", abilityName, bundleName, params);
    SystemTestFormUtil::StartAbility(want, abilityMs);
    EXPECT_EQ(SystemTestFormUtil::WaitCompleted(event, FORM_EVENT_ABILITY_ONACTIVED, 0), 0);
    std::string eventData = FORM_EVENT_REQ_ACQUIRE_FORM_2600_1;
    SystemTestFormUtil::PublishEvent(FORM_EVENT_REQ_ACQUIRE_FORM_2600_1, EVENT_CODE_2610, eventData);
    EXPECT_EQ(0, SystemTestFormUtil::WaitCompleted(event, FORM_EVENT_RECV_ACQUIRE_FORM_2600_1, EVENT_CODE_2610));
    std::string strFormId = SystemTestFormUtil::GetData(event, FORM_EVENT_RECV_ACQUIRE_FORM_2600_1, EVENT_CODE_2610);
    bool result = !strFormId.empty();
    EXPECT_TRUE(result);
    if (!result) {
        GTEST_LOG_(INFO) << "FMS_acquireForm_2600,  result:" << result;
    } else {
        GTEST_LOG_(INFO) << "FMS_acquireForm_2600,  formId:" << strFormId;
    }
    EXPECT_EQ(0, SystemTestFormUtil::WaitCompleted(event, FORM_EVENT_RECV_ACQUIRE_FORM_2600_1, EVENT_CODE_2611));
    std::string data2 = SystemTestFormUtil::GetData(event, FORM_EVENT_RECV_ACQUIRE_FORM_2600_1, EVENT_CODE_2611);
    bool result2 = !data2.empty();
    EXPECT_TRUE(result2);
    if (!result2) {
        GTEST_LOG_(INFO) << "FMS_acquireForm_2600,  result:" << result2;
    } else {
        GTEST_LOG_(INFO) << "FMS_acquireForm_2600,  formData:" << data2;
    }
    SystemTestFormUtil::StartAbility(want, abilityMs);
    EXPECT_EQ(SystemTestFormUtil::WaitCompleted(event, FORM_EVENT_ABILITY_ONACTIVED, 0), 0);
    std::string eventData1 = strFormId;
    SystemTestFormUtil::PublishEvent(FORM_EVENT_REQ_ACQUIRE_FORM_2600, EVENT_CODE_2600, eventData1);
    EXPECT_EQ(0, SystemTestFormUtil::WaitCompleted(event, FORM_EVENT_RECV_ACQUIRE_FORM_2600, EVENT_CODE_2600));
    std::string data3 = SystemTestFormUtil::GetData(event, FORM_EVENT_RECV_ACQUIRE_FORM_2600, EVENT_CODE_2600);
    bool result3 = data3 == "true";
    EXPECT_TRUE(result3);
    GTEST_LOG_(INFO) << "FMS_acquireForm_2600,  result:" << result3;
    // wait delete form
    EXPECT_EQ(0, SystemTestFormUtil::WaitCompleted(event, FORM_EVENT_RECV_DELETE_FORM_COMMON, EVENT_CODE_999));
    std::string data4 = SystemTestFormUtil::GetData(event, FORM_EVENT_RECV_DELETE_FORM_COMMON, EVENT_CODE_999);
    bool result4 = data4 == "true";
    EXPECT_TRUE(result4);
    GTEST_LOG_(INFO) << "FMS_acquireForm_2600,  delete form, result:" << result4;
    std::cout << "END FMS_acquireForm_2600" << std::endl;
}

/**
 * @tc.number: FMS_acquireForm_0100
 * @tc.name: Form host is not a system application
 * @tc.desc: Failed to create form when verifying that the form host is not a system application.
 */
HWTEST_F(FmsAcquireFormTest, FMS_acquireForm_0100, Function | MediumTest | Level1)
{
    sleep(1);
    std::cout << "START FMS_acquireForm_0100" << std::endl;

    std::string bundleName = "com.ohos.form.manager.notsystemapp";
    std::string abilityName = "FormAbilityNotSys";
    MAP_STR_STR params;
    Want want = SystemTestFormUtil::MakeWant("device", abilityName, bundleName, params);
    SystemTestFormUtil::StartAbility(want, abilityMs);
    EXPECT_EQ(SystemTestFormUtil::WaitCompleted(event, FORM_EVENT_ABILITY_ONACTIVED, 0), 0);

    std::string eventData = FORM_EVENT_REQ_ACQUIRE_FORM_0100;
    SystemTestFormUtil::PublishEvent(FORM_EVENT_REQ_ACQUIRE_FORM_0100, EVENT_CODE_100, eventData);
    EXPECT_EQ(0, SystemTestFormUtil::WaitCompleted(event, FORM_EVENT_RECV_ACQUIRE_FORM_0100, EVENT_CODE_100));
    std::string data = SystemTestFormUtil::GetData(event, FORM_EVENT_RECV_ACQUIRE_FORM_0100, EVENT_CODE_100);
    bool result = data == "false";
    EXPECT_TRUE(result);
    GTEST_LOG_(INFO) << "FMS_acquireForm_0100,  result:" << result;

    std::cout << "END FMS_acquireForm_0100" << std::endl;
}

/**
 * @tc.number: FMS_acquireForm_0200
 * @tc.name: The form host does not have permission to use the form
 * @tc.desc: Failed to create a form when verifying that the form host does not have form permission.
 */
HWTEST_F(FmsAcquireFormTest, FMS_acquireForm_0200, Function | MediumTest | Level1)
{
    sleep(1);
    std::cout << "START FMS_acquireForm_0200" << std::endl;

    std::string bundleName = "com.ohos.form.manager.nopermission";
    std::string abilityName = "FormAbilityNoPerm";
    MAP_STR_STR params;
    Want want = SystemTestFormUtil::MakeWant("device", abilityName, bundleName, params);
    SystemTestFormUtil::StartAbility(want, abilityMs);
    EXPECT_EQ(SystemTestFormUtil::WaitCompleted(event, FORM_EVENT_ABILITY_ONACTIVED, 0), 0);

    std::string eventData = FORM_EVENT_REQ_ACQUIRE_FORM_0200;
    SystemTestFormUtil::PublishEvent(FORM_EVENT_REQ_ACQUIRE_FORM_0200, EVENT_CODE_200, eventData);
    EXPECT_EQ(0, SystemTestFormUtil::WaitCompleted(event, FORM_EVENT_RECV_ACQUIRE_FORM_0200, EVENT_CODE_200));
    std::string data = SystemTestFormUtil::GetData(event, FORM_EVENT_RECV_ACQUIRE_FORM_0200, EVENT_CODE_200);
    bool result = data == "false";
    EXPECT_TRUE(result);
    GTEST_LOG_(INFO) << "FMS_acquireForm_0200,  result:" << result;

    std::cout << "END FMS_acquireForm_0200" << std::endl;
}

/**
 * @tc.number: FMS_acquireForm_0300
 * @tc.name: Failed to add form when parameter formId is wrong.
 * @tc.desc: Failed to create form when verifying that the parameter (formId) of creating form is wrong.
 */
HWTEST_F(FmsAcquireFormTest, FMS_acquireForm_0300, Function | MediumTest | Level1)
{
    sleep(1);
    std::cout << "START FMS_acquireForm_0300" << std::endl;

    std::string bundleName = "com.ohos.form.manager.normal";
    std::string abilityName = "FormAbilityA";
    MAP_STR_STR params;
    Want want = SystemTestFormUtil::MakeWant("device", abilityName, bundleName, params);
    SystemTestFormUtil::StartAbility(want, abilityMs);
    EXPECT_EQ(SystemTestFormUtil::WaitCompleted(event, FORM_EVENT_ABILITY_ONACTIVED, 0), 0);

    std::string eventData = FORM_EVENT_REQ_ACQUIRE_FORM_0300;
    SystemTestFormUtil::PublishEvent(FORM_EVENT_REQ_ACQUIRE_FORM_0300, EVENT_CODE_300, eventData);
    EXPECT_EQ(0, SystemTestFormUtil::WaitCompleted(event, FORM_EVENT_RECV_ACQUIRE_FORM_0300, EVENT_CODE_300));
    std::string data = SystemTestFormUtil::GetData(event, FORM_EVENT_RECV_ACQUIRE_FORM_0300, EVENT_CODE_300);
    bool result = data == "false";
    EXPECT_TRUE(result);
    GTEST_LOG_(INFO) << "FMS_acquireForm_0300,  result:" << result;
    std::cout << "END FMS_acquireForm_0300" << std::endl;
}

/**
 * @tc.number: FMS_acquireForm_0400
 * @tc.name: Failed to add form when parameter bundlename is wrong.
 * @tc.desc: Form creation failed when verifying the wrong parameter (bundlename) of form creation.
 */
HWTEST_F(FmsAcquireFormTest, FMS_acquireForm_0400, Function | MediumTest | Level1)
{
    sleep(1);
    std::cout << "START FMS_acquireForm_0400" << std::endl;

    std::string bundleName = "com.ohos.form.manager.normal";
    std::string abilityName = "FormAbilityA";
    MAP_STR_STR params;
    Want want = SystemTestFormUtil::MakeWant("device", abilityName, bundleName, params);
    SystemTestFormUtil::StartAbility(want, abilityMs);
    EXPECT_EQ(SystemTestFormUtil::WaitCompleted(event, FORM_EVENT_ABILITY_ONACTIVED, 0), 0);

    std::string eventData = FORM_EVENT_REQ_ACQUIRE_FORM_0400;
    SystemTestFormUtil::PublishEvent(FORM_EVENT_REQ_ACQUIRE_FORM_0400, EVENT_CODE_400, eventData);
    EXPECT_EQ(0, SystemTestFormUtil::WaitCompleted(event, FORM_EVENT_RECV_ACQUIRE_FORM_0400, EVENT_CODE_400));
    std::string data = SystemTestFormUtil::GetData(event, FORM_EVENT_RECV_ACQUIRE_FORM_0400, EVENT_CODE_400);
    bool result = data == "false";
    EXPECT_TRUE(result);
    GTEST_LOG_(INFO) << "FMS_acquireForm_0400,  result:" << result;
    std::cout << "END FMS_acquireForm_0400" << std::endl;
}

/**
 * @tc.number: FMS_acquireForm_0500
 * @tc.name: Failed to add form when parameter moduleName is wrong.
 * @tc.desc: Form creation failed when verifying the wrong parameter (moduleName) of form creation.
 */
HWTEST_F(FmsAcquireFormTest, FMS_acquireForm_0500, Function | MediumTest | Level1)
{
    sleep(1);
    std::cout << "START FMS_acquireForm_0500" << std::endl;

    std::string bundleName = "com.ohos.form.manager.normal";
    std::string abilityName = "FormAbilityA";
    MAP_STR_STR params;
    Want want = SystemTestFormUtil::MakeWant("device", abilityName, bundleName, params);
    SystemTestFormUtil::StartAbility(want, abilityMs);
    EXPECT_EQ(SystemTestFormUtil::WaitCompleted(event, FORM_EVENT_ABILITY_ONACTIVED, 0), 0);

    std::string eventData = FORM_EVENT_REQ_ACQUIRE_FORM_0500;
    SystemTestFormUtil::PublishEvent(FORM_EVENT_REQ_ACQUIRE_FORM_0500, EVENT_CODE_500, eventData);
    EXPECT_EQ(0, SystemTestFormUtil::WaitCompleted(event, FORM_EVENT_RECV_ACQUIRE_FORM_0500, EVENT_CODE_500));
    std::string data = SystemTestFormUtil::GetData(event, FORM_EVENT_RECV_ACQUIRE_FORM_0500, EVENT_CODE_500);
    bool result = data == "false";
    EXPECT_TRUE(result);
    GTEST_LOG_(INFO) << "FMS_acquireForm_0500,  result:" << result;
    std::cout << "END FMS_acquireForm_0500" << std::endl;
}

/**
 * @tc.number: FMS_acquireForm_0600
 * @tc.name: Failed to add form when parameter formName is wrong.
 * @tc.desc: Form creation failed when verifying the wrong parameter (formName) of form creation.
 */
HWTEST_F(FmsAcquireFormTest, FMS_acquireForm_0600, Function | MediumTest | Level1)
{
    sleep(1);
    std::cout << "START FMS_acquireForm_0600" << std::endl;

    std::string bundleName = "com.ohos.form.manager.normal";
    std::string abilityName = "FormAbilityA";
    MAP_STR_STR params;
    Want want = SystemTestFormUtil::MakeWant("device", abilityName, bundleName, params);
    SystemTestFormUtil::StartAbility(want, abilityMs);
    EXPECT_EQ(SystemTestFormUtil::WaitCompleted(event, FORM_EVENT_ABILITY_ONACTIVED, 0), 0);

    std::string eventData = FORM_EVENT_REQ_ACQUIRE_FORM_0600;
    SystemTestFormUtil::PublishEvent(FORM_EVENT_REQ_ACQUIRE_FORM_0600, EVENT_CODE_600, eventData);
    EXPECT_EQ(0, SystemTestFormUtil::WaitCompleted(event, FORM_EVENT_RECV_ACQUIRE_FORM_0600, EVENT_CODE_600));
    std::string data = SystemTestFormUtil::GetData(event, FORM_EVENT_RECV_ACQUIRE_FORM_0600, EVENT_CODE_600);
    bool result = data == "false";
    EXPECT_TRUE(result);
    GTEST_LOG_(INFO) << "FMS_acquireForm_0600,  result:" << result;

    std::cout << "END FMS_acquireForm_0600" << std::endl;
}

/**
 * @tc.number: FMS_acquireForm_0700
 * @tc.name: Failed to add form when parameter abilityName is wrong.
 * @tc.desc: Form creation failed when verifying the wrong parameter (abilityName) of form creation.
 */
HWTEST_F(FmsAcquireFormTest, FMS_acquireForm_0700, Function | MediumTest | Level1)
{
    sleep(1);
    std::cout << "START FMS_acquireForm_0700" << std::endl;

    std::string bundleName = "com.ohos.form.manager.normal";
    std::string abilityName = "FormAbilityA";
    MAP_STR_STR params;
    Want want = SystemTestFormUtil::MakeWant("device", abilityName, bundleName, params);
    SystemTestFormUtil::StartAbility(want, abilityMs);
    EXPECT_EQ(SystemTestFormUtil::WaitCompleted(event, FORM_EVENT_ABILITY_ONACTIVED, 0), 0);

    std::string eventData = FORM_EVENT_REQ_ACQUIRE_FORM_0700;
    SystemTestFormUtil::PublishEvent(FORM_EVENT_REQ_ACQUIRE_FORM_0700, EVENT_CODE_700, eventData);
    EXPECT_EQ(0, SystemTestFormUtil::WaitCompleted(event, FORM_EVENT_RECV_ACQUIRE_FORM_0700, EVENT_CODE_700));
    std::string data = SystemTestFormUtil::GetData(event, FORM_EVENT_RECV_ACQUIRE_FORM_0700, EVENT_CODE_700);
    bool result = data == "false";
    EXPECT_TRUE(result);
    if (!result) {
        GTEST_LOG_(INFO) << "FMS_acquireForm_0700,  result:" << result;
    }
    std::cout << "END FMS_acquireForm_0700" << std::endl;
}
/**
 * @tc.number: FMS_acquireForm_1000
 * @tc.name: Failed to add form when parameter form dimension is wrong.
 * @tc.desc: Form creation failed when verifying the wrong parameter (form dimension) of form creation.
 */
HWTEST_F(FmsAcquireFormTest, FMS_acquireForm_1000, Function | MediumTest | Level1)
{
    sleep(1);
    std::cout << "START FMS_acquireForm_1000" << std::endl;

    std::string bundleName = "com.ohos.form.manager.normal";
    std::string abilityName = "FormAbilityA";
    MAP_STR_STR params;
    Want want = SystemTestFormUtil::MakeWant("device", abilityName, bundleName, params);
    SystemTestFormUtil::StartAbility(want, abilityMs);
    EXPECT_EQ(SystemTestFormUtil::WaitCompleted(event, FORM_EVENT_ABILITY_ONACTIVED, 0), 0);

    std::string eventData = FORM_EVENT_REQ_ACQUIRE_FORM_1000;
    SystemTestFormUtil::PublishEvent(FORM_EVENT_REQ_ACQUIRE_FORM_1000, EVENT_CODE_1000, eventData);
    EXPECT_EQ(0, SystemTestFormUtil::WaitCompleted(event, FORM_EVENT_RECV_ACQUIRE_FORM_1000, EVENT_CODE_1000));
    std::string data = SystemTestFormUtil::GetData(event, FORM_EVENT_RECV_ACQUIRE_FORM_1000, EVENT_CODE_1000);
    bool result = data == "false";
    EXPECT_TRUE(result);
    GTEST_LOG_(INFO) << "FMS_acquireForm_1000,  result:" << result;

    std::cout << "END FMS_acquireForm_1000" << std::endl;
}
/**
 * @tc.number: FMS_acquireForm_1400
 * @tc.name: FormId does not match other parameters when adding a cached form.
 * @tc.desc: Failed to create a cached form when verifying that FormId does not match other parameters.
 */
HWTEST_F(FmsAcquireFormTest, FMS_acquireForm_1400, Function | MediumTest | Level1)
{
    sleep(1);
    std::cout << "START FMS_acquireForm_1400" << std::endl;
    std::string bundleName = "com.ohos.form.manager.normalb";
    std::string abilityName = "FormAbilityB";
    MAP_STR_STR params;
    Want want = SystemTestFormUtil::MakeWant("device", abilityName, bundleName, params);
    SystemTestFormUtil::StartAbility(want, abilityMs);
    EXPECT_EQ(SystemTestFormUtil::WaitCompleted(event, FORM_EVENT_ABILITY_ONACTIVED, 0), 0);

    std::string eventData = catchFormId;
    GTEST_LOG_(INFO) << "FMS_acquireForm_1400,  formId:" << eventData;
    SystemTestFormUtil::PublishEvent(FORM_EVENT_REQ_ACQUIRE_FORM_1400, EVENT_CODE_1400, eventData);
    EXPECT_EQ(0, SystemTestFormUtil::WaitCompleted(event, FORM_EVENT_RECV_ACQUIRE_FORM_1400, EVENT_CODE_1400));
    std::string data = SystemTestFormUtil::GetData(event, FORM_EVENT_RECV_ACQUIRE_FORM_1400, EVENT_CODE_1400);
    bool result = data == "false";
    EXPECT_TRUE(result);
    GTEST_LOG_(INFO) << "FMS_acquireForm_1400,  result:" << result;

    std::cout << "END FMS_acquireForm_1400" << std::endl;
}

/**
 * @tc.number: FMS_acquireForm_2100
 * @tc.name: When creating a temporary form, the formId is specified as the created normal formId.
 * @tc.desc: Failed to create a temporary form when verifying that the formId is set to create a normal formId.
 */
HWTEST_F(FmsAcquireFormTest, FMS_acquireForm_2100, Function | MediumTest | Level1)
{
    sleep(1);
    std::cout << "START FMS_acquireForm_2100" << std::endl;
    std::string bundleName = "com.ohos.form.manager.normal";
    std::string abilityName = "FormAbilityA";
    MAP_STR_STR params;
    Want want = SystemTestFormUtil::MakeWant("device", abilityName, bundleName, params);
    SystemTestFormUtil::StartAbility(want, abilityMs);
    EXPECT_EQ(SystemTestFormUtil::WaitCompleted(event, FORM_EVENT_ABILITY_ONACTIVED, 0), 0);

    std::string eventData = catchFormId;
    GTEST_LOG_(INFO) << "FMS_acquireForm_2100,  formId:" << eventData;
    SystemTestFormUtil::PublishEvent(FORM_EVENT_REQ_ACQUIRE_FORM_2100, EVENT_CODE_2100, eventData);
    EXPECT_EQ(0, SystemTestFormUtil::WaitCompleted(event, FORM_EVENT_RECV_ACQUIRE_FORM_2100, EVENT_CODE_2100));
    std::string data = SystemTestFormUtil::GetData(event, FORM_EVENT_RECV_ACQUIRE_FORM_2100, EVENT_CODE_2100);
    bool result = data == "false";
    EXPECT_TRUE(result);
    GTEST_LOG_(INFO) << "FMS_acquireForm_2100,  result:" << result;
    sleep(1);
    FmsAcquireFormDeleteA(catchFormId);
    std::cout << "END FMS_acquireForm_2100" << std::endl;
}

/**
 * @tc.number: FMS_acquireForm_2200
 * @tc.name: Cast to normal form when the temporary formId is set to negative.
 * @tc.desc: Failed to Cast to normal form when the temporary formId is set to negative
 */
HWTEST_F(FmsAcquireFormTest, FMS_acquireForm_2200, Function | MediumTest | Level1)
{
    sleep(1);
    std::cout << "START FMS_acquireForm_2200" << std::endl;
    std::string bundleName = "com.ohos.form.manager.normal";
    std::string abilityName = "FormAbilityA";
    MAP_STR_STR params;
    Want want = SystemTestFormUtil::MakeWant("device", abilityName, bundleName, params);
    SystemTestFormUtil::StartAbility(want, abilityMs);
    EXPECT_EQ(SystemTestFormUtil::WaitCompleted(event, FORM_EVENT_ABILITY_ONACTIVED, 0), 0);

    std::string eventData = FORM_EVENT_REQ_ACQUIRE_FORM_2200;
    SystemTestFormUtil::PublishEvent(FORM_EVENT_REQ_ACQUIRE_FORM_2200, EVENT_CODE_2200, eventData);
    EXPECT_EQ(0, SystemTestFormUtil::WaitCompleted(event, FORM_EVENT_RECV_ACQUIRE_FORM_2200, EVENT_CODE_2200));
    std::string data = SystemTestFormUtil::GetData(event, FORM_EVENT_RECV_ACQUIRE_FORM_2200, EVENT_CODE_2200);
    bool result = data == "false";
    EXPECT_TRUE(result);
    GTEST_LOG_(INFO) << "FMS_acquireForm_2200,  result:" << result;

    std::cout << "END FMS_acquireForm_2200" << std::endl;
}

/**
 * @tc.number: FMS_acquireForm_2300
 * @tc.name: Cast to normal form when the temporary formIdis set to 0.
 * @tc.desc: Failed to convert to normal form when verifying that the temporary formIdis set to 0
 */
HWTEST_F(FmsAcquireFormTest, FMS_acquireForm_2300, Function | MediumTest | Level1)
{
    sleep(1);
    std::cout << "START FMS_acquireForm_2300" << std::endl;
    std::string bundleName = "com.ohos.form.manager.normal";
    std::string abilityName = "FormAbilityA";
    MAP_STR_STR params;
    Want want = SystemTestFormUtil::MakeWant("device", abilityName, bundleName, params);
    SystemTestFormUtil::StartAbility(want, abilityMs);
    EXPECT_EQ(SystemTestFormUtil::WaitCompleted(event, FORM_EVENT_ABILITY_ONACTIVED, 0), 0);

    std::string eventData = FORM_EVENT_REQ_ACQUIRE_FORM_2300;
    SystemTestFormUtil::PublishEvent(FORM_EVENT_REQ_ACQUIRE_FORM_2300, EVENT_CODE_2300, eventData);
    EXPECT_EQ(0, SystemTestFormUtil::WaitCompleted(event, FORM_EVENT_RECV_ACQUIRE_FORM_2300, EVENT_CODE_2300));
    std::string data = SystemTestFormUtil::GetData(event, FORM_EVENT_RECV_ACQUIRE_FORM_2300, EVENT_CODE_2300);
    bool result = data == "false";
    EXPECT_TRUE(result);
    GTEST_LOG_(INFO) << "FMS_acquireForm_2300,  result:" << result;

    std::cout << "END FMS_acquireForm_2300" << std::endl;
}

/**
 * @tc.number: FMS_acquireForm_2500
 * @tc.name: cast non-existent temporary formId to normal form.
 * @tc.desc: Failed to verify that the temporary formId that does not exist is casted to normal form.
 */
HWTEST_F(FmsAcquireFormTest, FMS_acquireForm_2500, Function | MediumTest | Level1)
{
    sleep(1);
    std::cout << "START FMS_acquireForm_2500" << std::endl;
    std::string bundleName = "com.ohos.form.manager.normal";
    std::string abilityName = "FormAbilityA";
    MAP_STR_STR params;
    Want want = SystemTestFormUtil::MakeWant("device", abilityName, bundleName, params);
    SystemTestFormUtil::StartAbility(want, abilityMs);
    EXPECT_EQ(SystemTestFormUtil::WaitCompleted(event, FORM_EVENT_ABILITY_ONACTIVED, 0), 0);

    std::string eventData = FORM_EVENT_REQ_ACQUIRE_FORM_2500;
    SystemTestFormUtil::PublishEvent(FORM_EVENT_REQ_ACQUIRE_FORM_2500, EVENT_CODE_2500, eventData);
    EXPECT_EQ(0, SystemTestFormUtil::WaitCompleted(event, FORM_EVENT_RECV_ACQUIRE_FORM_2500, EVENT_CODE_2500));
    std::string data = SystemTestFormUtil::GetData(event, FORM_EVENT_RECV_ACQUIRE_FORM_2500, EVENT_CODE_2500);
    bool result = data == "false";
    EXPECT_TRUE(result);
    GTEST_LOG_(INFO) << "FMS_acquireForm_2500,  result:" << result;

    std::cout << "END FMS_acquireForm_2500" << std::endl;
}

void FmsAcquireFormTest::FmsAcquireFormCatched1500(std::string strFormId)
{
    std::string bundleName1 = "com.ohos.form.manager.normalb";
    std::string abilityName1 = "FormAbilityB";
    MAP_STR_STR params1;
    Want want1 = SystemTestFormUtil::MakeWant("device", abilityName1, bundleName1, params1);
    SystemTestFormUtil::StartAbility(want1, abilityMs);
    EXPECT_EQ(SystemTestFormUtil::WaitCompleted(event, FORM_EVENT_ABILITY_ONACTIVED, 0), 0);

    GTEST_LOG_(INFO) << "FMS_acquireForm_1500,  add catched form start";
    std::string eventData2 = strFormId;
    GTEST_LOG_(INFO) << "FMS_acquireForm_1500,  formId:" << eventData2;
    SystemTestFormUtil::PublishEvent(FORM_EVENT_REQ_ACQUIRE_FORM_1500, EVENT_CODE_1500, eventData2);
    EXPECT_EQ(0, SystemTestFormUtil::WaitCompleted(event, FORM_EVENT_RECV_ACQUIRE_FORM_1500, EVENT_CODE_1500));
    std::string strFormId2 = SystemTestFormUtil::GetData(event, FORM_EVENT_RECV_ACQUIRE_FORM_1500, EVENT_CODE_1500);
    bool result3 = !strFormId2.empty();
    EXPECT_TRUE(result3);
    if (!result3) {
        GTEST_LOG_(INFO) << "FMS_acquireForm_1500,  result:" << result3;
    } else {
        GTEST_LOG_(INFO) << "FMS_acquireForm_1500,  formId:" << strFormId2;
    }
  
    GTEST_LOG_(INFO) << "FMS_acquireForm_1500,  add catched form end";
    // wait delete form
    EXPECT_EQ(0, SystemTestFormUtil::WaitCompleted(event, FORM_EVENT_RECV_DELETE_FORM_COMMON, EVENT_CODE_999));
    std::string data5 = SystemTestFormUtil::GetData(event, FORM_EVENT_RECV_DELETE_FORM_COMMON, EVENT_CODE_999);
    bool result5 = data5 == "true";
    EXPECT_TRUE(result5);
    GTEST_LOG_(INFO) << "FMS_acquireForm_1500,  delete form, result:" << result5;
}

void FmsAcquireFormTest::FmsAcquireForm1600()
{
    std::cout << "START FMS_acquireForm_1600, normal form start." << std::endl;
    std::string bundleName = "com.ohos.form.manager.normal";
    std::string abilityName = "FormAbilityA";
    MAP_STR_STR params;
    Want want = SystemTestFormUtil::MakeWant("device", abilityName, bundleName, params);
    SystemTestFormUtil::StartAbility(want, abilityMs);
    EXPECT_EQ(SystemTestFormUtil::WaitCompleted(event, FORM_EVENT_ABILITY_ONACTIVED, 0), 0);

    std::string eventData = FORM_EVENT_REQ_ACQUIRE_FORM_1600;
    SystemTestFormUtil::PublishEvent(FORM_EVENT_REQ_ACQUIRE_FORM_1600, EVENT_CODE_1600, eventData);
    EXPECT_EQ(0, SystemTestFormUtil::WaitCompleted(event, FORM_EVENT_RECV_ACQUIRE_FORM_1600, EVENT_CODE_1600));
    std::string strFormId = SystemTestFormUtil::GetData(event, FORM_EVENT_RECV_ACQUIRE_FORM_1600, EVENT_CODE_1600);
    bool result = !strFormId.empty();
    EXPECT_TRUE(result);
    if (!result) {
        GTEST_LOG_(INFO) << "FMS_acquireForm_1600,  result:" << result;
    } else {
        GTEST_LOG_(INFO) << "FMS_acquireForm_1600, formId:" << strFormId;
    }

    EXPECT_EQ(0, SystemTestFormUtil::WaitCompleted(event, FORM_EVENT_RECV_ACQUIRE_FORM_1600, EVENT_CODE_1601));
    std::string data2 = SystemTestFormUtil::GetData(event, FORM_EVENT_RECV_ACQUIRE_FORM_1600, EVENT_CODE_1601);
    bool result2 = !data2.empty();
    EXPECT_TRUE(result2);
    if (!result2) {
        GTEST_LOG_(INFO) << "FMS_acquireForm_1600,  result:" << result2;
    } else {
        GTEST_LOG_(INFO) << "FMS_acquireForm_1600, formData:" << data2;
    }

    // wait delete form
    EXPECT_EQ(0, SystemTestFormUtil::WaitCompleted(event, FORM_EVENT_RECV_DELETE_FORM_COMMON, EVENT_CODE_999));
    std::string data3 = SystemTestFormUtil::GetData(event, FORM_EVENT_RECV_DELETE_FORM_COMMON, EVENT_CODE_999);
    bool result3 = data3 == "true";
    EXPECT_TRUE(result3);
    GTEST_LOG_(INFO) << "FMS_acquireForm_1600,  delete form, result:" << result3;
}

void FmsAcquireFormTest::FmsAcquireForm1600A()
{
    std::cout << "START FMS_acquireForm_1600_1, temp form start." << std::endl;

    std::string bundleName = "com.ohos.form.manager.normal";
    std::string abilityName = "FormAbilityA";
    MAP_STR_STR params;
    Want want = SystemTestFormUtil::MakeWant("device", abilityName, bundleName, params);
    SystemTestFormUtil::StartAbility(want, abilityMs);
    EXPECT_EQ(SystemTestFormUtil::WaitCompleted(event, FORM_EVENT_ABILITY_ONACTIVED, 0), 0);

    std::string eventData1 = FORM_EVENT_REQ_ACQUIRE_FORM_1600_1;
    SystemTestFormUtil::PublishEvent(FORM_EVENT_REQ_ACQUIRE_FORM_1600_1, EVENT_CODE_1610, eventData1);
    EXPECT_EQ(0, SystemTestFormUtil::WaitCompleted(event, FORM_EVENT_RECV_ACQUIRE_FORM_1600_1, EVENT_CODE_1610));
    std::string strFormId1 = SystemTestFormUtil::GetData(event, FORM_EVENT_RECV_ACQUIRE_FORM_1600_1, EVENT_CODE_1610);
    bool result4 = !strFormId1.empty();
    EXPECT_TRUE(result4);
    if (!result4) {
        GTEST_LOG_(INFO) << "FMS_acquireForm_1600_1,  result:" << result4;
    } else {
        GTEST_LOG_(INFO) << "FMS_acquireForm_1600_1 formId:" << strFormId1;
    }

    EXPECT_EQ(0, SystemTestFormUtil::WaitCompleted(event, FORM_EVENT_RECV_ACQUIRE_FORM_1600_1, EVENT_CODE_1611));
    std::string data4 = SystemTestFormUtil::GetData(event, FORM_EVENT_RECV_ACQUIRE_FORM_1600_1, EVENT_CODE_1611);
    bool result5 = !data4.empty();
    EXPECT_TRUE(result5);
    if (!result5) {
        GTEST_LOG_(INFO) << "FMS_acquireForm_1600_1,  result:" << result5;
    } else {
        GTEST_LOG_(INFO) << "FMS_acquireForm_1600_1,  formData:" << data4;
    }

    // wait delete form
    EXPECT_EQ(0, SystemTestFormUtil::WaitCompleted(event, FORM_EVENT_RECV_DELETE_FORM_COMMON, EVENT_CODE_999));
    std::string data5 = SystemTestFormUtil::GetData(event, FORM_EVENT_RECV_DELETE_FORM_COMMON, EVENT_CODE_999);
    bool result6 = data5 == "true";
    EXPECT_TRUE(result6);
    GTEST_LOG_(INFO) << "FMS_acquireForm_1600_1,  delete form, result:" << result6;
}
void FmsAcquireFormTest::FmsAcquireForm1800()
{
    int normalCount = 0;
    std::cout << "START FMS_acquireForm_1800, normal form start." << std::endl;
    std::string bundleName = "com.ohos.form.manager.normal";
    std::string abilityName = "FormAbilityA";
    std::cout << "START FMS_acquireForm_1800, bundleName: "<< bundleName << std::endl;
    std::cout << "START FMS_acquireForm_1800, abilityName: "<< abilityName << std::endl;
    MAP_STR_STR params;
    Want want = SystemTestFormUtil::MakeWant("device", abilityName, bundleName, params);
    SystemTestFormUtil::StartAbility(want, abilityMs);
    EXPECT_EQ(SystemTestFormUtil::WaitCompleted(event, FORM_EVENT_ABILITY_ONACTIVED, 0), 0);

    std::string eventData = FORM_EVENT_REQ_ACQUIRE_FORM_1800;
    SystemTestFormUtil::PublishEvent(FORM_EVENT_REQ_ACQUIRE_FORM_1800, EVENT_CODE_1800, eventData);
    EXPECT_EQ(0, SystemTestFormUtil::WaitCompleted(event, FORM_EVENT_RECV_ACQUIRE_FORM_1800, EVENT_CODE_1800));
    std::string strFormId = SystemTestFormUtil::GetData(event, FORM_EVENT_RECV_ACQUIRE_FORM_1800, EVENT_CODE_1800);
    bool result = !strFormId.empty();
    EXPECT_TRUE(result);
    if (!result) {
        GTEST_LOG_(INFO) << "FMS_acquireForm_1800,  result:" << result;
    } else {
        GTEST_LOG_(INFO) << "FMS_acquireForm_1800, formId:" << strFormId;
    }

    EXPECT_EQ(0, SystemTestFormUtil::WaitCompleted(event, FORM_EVENT_RECV_ACQUIRE_FORM_1800, EVENT_CODE_1801));
    std::string data2 = SystemTestFormUtil::GetData(event, FORM_EVENT_RECV_ACQUIRE_FORM_1800, EVENT_CODE_1801);
    bool result2 = !data2.empty();
    EXPECT_TRUE(result2);
    if (!result2) {
        GTEST_LOG_(INFO) << "FMS_acquireForm_1800,  result:" << result2;
    } else {
        GTEST_LOG_(INFO) << "FMS_acquireForm_1800, formData:" << data2;
    }

    // wait delete form
    EXPECT_EQ(0, SystemTestFormUtil::WaitCompleted(event, FORM_EVENT_RECV_DELETE_FORM_COMMON, EVENT_CODE_999));
    std::string data3 = SystemTestFormUtil::GetData(event, FORM_EVENT_RECV_DELETE_FORM_COMMON, EVENT_CODE_999);
    bool result3 = data3 == "true";
    EXPECT_TRUE(result3);
    GTEST_LOG_(INFO) << "FMS_acquireForm_1800,  delete form, result:" << result3;

    normalCount++;
    std::cout << "END FMS_acquireForm_1800, normal form end, count:"  << normalCount << std::endl;
}
void FmsAcquireFormTest::FmsAcquireForm1800A()
{
    int tempCount = 0;
    std::cout << "START FMS_acquireForm_1800_1, temp form start." << std::endl;

    std::string bundleName = "com.ohos.form.manager.normal";
    std::string abilityName = "FormAbilityA";
    std::cout << "START FMS_acquireForm_1800_1, bundleName: "<< bundleName << std::endl;
    std::cout << "START FMS_acquireForm_1800_1, abilityName: "<< abilityName << std::endl;
    MAP_STR_STR params;
    Want want = SystemTestFormUtil::MakeWant("device", abilityName, bundleName, params);
    SystemTestFormUtil::StartAbility(want, abilityMs);
    EXPECT_EQ(SystemTestFormUtil::WaitCompleted(event, FORM_EVENT_ABILITY_ONACTIVED, 0), 0);

    std::string eventData1 = FORM_EVENT_REQ_ACQUIRE_FORM_1800_1;
    SystemTestFormUtil::PublishEvent(FORM_EVENT_REQ_ACQUIRE_FORM_1800_1, EVENT_CODE_1810, eventData1);
    EXPECT_EQ(0, SystemTestFormUtil::WaitCompleted(event, FORM_EVENT_RECV_ACQUIRE_FORM_1800_1, EVENT_CODE_1810));
    std::string strFormId1 = SystemTestFormUtil::GetData(event, FORM_EVENT_RECV_ACQUIRE_FORM_1800_1, EVENT_CODE_1810);
    bool result4 = !strFormId1.empty();
    EXPECT_TRUE(result4);
    if (!result4) {
        GTEST_LOG_(INFO) << "FMS_acquireForm_1800_1,  result:" << result4;
    } else {
        GTEST_LOG_(INFO) << "FMS_acquireForm_1800_1 formId:" << strFormId1;
    }

    EXPECT_EQ(0, SystemTestFormUtil::WaitCompleted(event, FORM_EVENT_RECV_ACQUIRE_FORM_1800_1, EVENT_CODE_1811));
    std::string data4 = SystemTestFormUtil::GetData(event, FORM_EVENT_RECV_ACQUIRE_FORM_1800_1, EVENT_CODE_1811);
    bool result5 = !data4.empty();
    EXPECT_TRUE(result5);
    if (!result5) {
        GTEST_LOG_(INFO) << "FMS_acquireForm_1800_1,  result:" << result5;
    } else {
        GTEST_LOG_(INFO) << "FMS_acquireForm_1800_1,  formData:" << data4;
    }

    // wait delete form
    EXPECT_EQ(0, SystemTestFormUtil::WaitCompleted(event, FORM_EVENT_RECV_DELETE_FORM_COMMON, EVENT_CODE_999));
    std::string data5 = SystemTestFormUtil::GetData(event, FORM_EVENT_RECV_DELETE_FORM_COMMON, EVENT_CODE_999);
    bool result6 = data5 == "true";
    EXPECT_TRUE(result6);
    GTEST_LOG_(INFO) << "FMS_acquireForm_1800_1,  delete form, result:" << result6;

    tempCount++;
    std::cout << "END FMS_acquireForm_1800_1, temp form end, count:"  << tempCount << std::endl;
}
void FmsAcquireFormTest::FmsAcquireForm1800B()
{
    int normalCount = 0;
    std::cout << "START FMS_acquireForm_1800_2, normal form start." << std::endl;
    std::string bundleName = "com.ohos.form.manager.normalb";
    std::string abilityName = "FormAbilityB";
    std::cout << "START FMS_acquireForm_1800_2, bundleName: "<< bundleName << std::endl;
    std::cout << "START FMS_acquireForm_1800_2, abilityName: "<< abilityName << std::endl;
    MAP_STR_STR params;
    Want want = SystemTestFormUtil::MakeWant("device", abilityName, bundleName, params);
    SystemTestFormUtil::StartAbility(want, abilityMs);
    EXPECT_EQ(SystemTestFormUtil::WaitCompleted(event, FORM_EVENT_ABILITY_ONACTIVED, 0), 0);

    std::string eventData = FORM_EVENT_REQ_ACQUIRE_FORM_1800_2;
    SystemTestFormUtil::PublishEvent(FORM_EVENT_REQ_ACQUIRE_FORM_1800_2, EVENT_CODE_1820, eventData);
    EXPECT_EQ(0, SystemTestFormUtil::WaitCompleted(event, FORM_EVENT_RECV_ACQUIRE_FORM_1800_2, EVENT_CODE_1820));
    std::string strFormId = SystemTestFormUtil::GetData(event, FORM_EVENT_RECV_ACQUIRE_FORM_1800_2, EVENT_CODE_1820);
    bool result = !strFormId.empty();
    EXPECT_TRUE(result);
    if (!result) {
        GTEST_LOG_(INFO) << "FMS_acquireForm_1800_2,  result:" << result;
    } else {
        GTEST_LOG_(INFO) << "FMS_acquireForm_1800_2, formId:" << strFormId;
    }

    EXPECT_EQ(0, SystemTestFormUtil::WaitCompleted(event, FORM_EVENT_RECV_ACQUIRE_FORM_1800_2, EVENT_CODE_1821));
    std::string data2 = SystemTestFormUtil::GetData(event, FORM_EVENT_RECV_ACQUIRE_FORM_1800_2, EVENT_CODE_1821);
    bool result2 = !data2.empty();
    EXPECT_TRUE(result2);
    if (!result2) {
        GTEST_LOG_(INFO) << "FMS_acquireForm_1800_2,  result:" << result2;
    } else {
        GTEST_LOG_(INFO) << "FMS_acquireForm_1800_2, formData:" << data2;
    }

    // wait delete form
    EXPECT_EQ(0, SystemTestFormUtil::WaitCompleted(event, FORM_EVENT_RECV_DELETE_FORM_COMMON, EVENT_CODE_999));
    std::string data3 = SystemTestFormUtil::GetData(event, FORM_EVENT_RECV_DELETE_FORM_COMMON, EVENT_CODE_999);
    bool result3 = data3 == "true";
    EXPECT_TRUE(result3);
    GTEST_LOG_(INFO) << "FMS_acquireForm_1800_2,  delete form, result:" << result3;

    normalCount++;
    std::cout << "END FMS_acquireForm_1800_2, normal form end, count:"  << normalCount << std::endl;
}
void FmsAcquireFormTest::FmsAcquireForm1800C()
{
    int tempCount = 0;
    std::cout << "START FMS_acquireForm_1800_3, temp form start." << std::endl;

    std::string bundleName = "com.ohos.form.manager.normalb";
    std::string abilityName = "FormAbilityB";
    std::cout << "START FMS_acquireForm_1800_3, bundleName: "<< bundleName << std::endl;
    std::cout << "START FMS_acquireForm_1800_3, abilityName: "<< abilityName << std::endl;
    MAP_STR_STR params;
    Want want = SystemTestFormUtil::MakeWant("device", abilityName, bundleName, params);
    SystemTestFormUtil::StartAbility(want, abilityMs);
    EXPECT_EQ(SystemTestFormUtil::WaitCompleted(event, FORM_EVENT_ABILITY_ONACTIVED, 0), 0);

    std::string eventData1 = FORM_EVENT_REQ_ACQUIRE_FORM_1800_3;
    SystemTestFormUtil::PublishEvent(FORM_EVENT_REQ_ACQUIRE_FORM_1800_3, EVENT_CODE_1830, eventData1);
    EXPECT_EQ(0, SystemTestFormUtil::WaitCompleted(event, FORM_EVENT_RECV_ACQUIRE_FORM_1800_3, EVENT_CODE_1830));
    std::string strFormId1 = SystemTestFormUtil::GetData(event, FORM_EVENT_RECV_ACQUIRE_FORM_1800_3, EVENT_CODE_1830);
    bool result4 = !strFormId1.empty();
    EXPECT_TRUE(result4);
    if (!result4) {
        GTEST_LOG_(INFO) << "FMS_acquireForm_1800_3,  result:" << result4;
    } else {
        GTEST_LOG_(INFO) << "FMS_acquireForm_1800_3 formId:" << strFormId1;
    }

    EXPECT_EQ(0, SystemTestFormUtil::WaitCompleted(event, FORM_EVENT_RECV_ACQUIRE_FORM_1800_3, EVENT_CODE_1831));
    std::string data4 = SystemTestFormUtil::GetData(event, FORM_EVENT_RECV_ACQUIRE_FORM_1800_3, EVENT_CODE_1831);
    bool result5 = !data4.empty();
    EXPECT_TRUE(result5);
    if (!result5) {
        GTEST_LOG_(INFO) << "FMS_acquireForm_1800_3,  result:" << result5;
    } else {
        GTEST_LOG_(INFO) << "FMS_acquireForm_1800_3,  formData:" << data4;
    }

    // wait delete form
    EXPECT_EQ(0, SystemTestFormUtil::WaitCompleted(event, FORM_EVENT_RECV_DELETE_FORM_COMMON, EVENT_CODE_999));
    std::string data5 = SystemTestFormUtil::GetData(event, FORM_EVENT_RECV_DELETE_FORM_COMMON, EVENT_CODE_999);
    bool result6 = data5 == "true";
    EXPECT_TRUE(result6);
    GTEST_LOG_(INFO) << "FMS_acquireForm_1800_3,  delete form, result:" << result6;

    tempCount++;
    std::cout << "END FMS_acquireForm_1800_3, temp form end, count:"  << tempCount << std::endl;
}

std::string FmsAcquireFormTest::FmsAcquireForm2400()
{
    int normalCount = 0;
    std::cout << "START FMS_acquireForm_2400, temp form start." << std::endl;
    std::string bundleName = "com.ohos.form.manager.normal";
    std::string abilityName = "FormAbilityA";
    std::cout << "START FMS_acquireForm_2400, bundleName: "<< bundleName << std::endl;
    std::cout << "START FMS_acquireForm_2400, abilityName: "<< abilityName << std::endl;
    MAP_STR_STR params;
    Want want = SystemTestFormUtil::MakeWant("device", abilityName, bundleName, params);
    SystemTestFormUtil::StartAbility(want, abilityMs);
    EXPECT_EQ(SystemTestFormUtil::WaitCompleted(event, FORM_EVENT_ABILITY_ONACTIVED, 0), 0);

    std::string eventData = FORM_EVENT_REQ_ACQUIRE_FORM_2400;
    SystemTestFormUtil::PublishEvent(FORM_EVENT_REQ_ACQUIRE_FORM_2400, EVENT_CODE_2400, eventData);
    EXPECT_EQ(0, SystemTestFormUtil::WaitCompleted(event, FORM_EVENT_RECV_ACQUIRE_FORM_2400, EVENT_CODE_2400));
    std::string strFormId = SystemTestFormUtil::GetData(event, FORM_EVENT_RECV_ACQUIRE_FORM_2400, EVENT_CODE_2400);
    bool result = !strFormId.empty();
    EXPECT_TRUE(result);
    if (!result) {
        GTEST_LOG_(INFO) << "FMS_acquireForm_2400,  result:" << result;
    } else {
        GTEST_LOG_(INFO) << "FMS_acquireForm_2400, formId:" << strFormId;
    }

    EXPECT_EQ(0, SystemTestFormUtil::WaitCompleted(event, FORM_EVENT_RECV_ACQUIRE_FORM_2400, EVENT_CODE_2401));
    std::string data2 = SystemTestFormUtil::GetData(event, FORM_EVENT_RECV_ACQUIRE_FORM_2400, EVENT_CODE_2401);
    bool result2 = !data2.empty();
    EXPECT_TRUE(result2);
    if (!result2) {
        GTEST_LOG_(INFO) << "FMS_acquireForm_2400,  result:" << result2;
    } else {
        GTEST_LOG_(INFO) << "FMS_acquireForm_2400, formData:" << data2;
    }

    normalCount++;
    std::cout << "END FMS_acquireForm_2400, temp form end, count:"  << normalCount << std::endl;

    return strFormId;
}

void FmsAcquireFormTest::FmsAcquireForm2400_1(std::string strFormId)
{
    std::cout << "START FMS_acquireForm_2400_1, cast temp start." << std::endl;
    std::string bundleName = "com.ohos.form.manager.normalb";
    std::string abilityName = "FormAbilityB";
    std::cout << "START FmsAcquireForm2400_1, bundleName: "<< bundleName << std::endl;
    std::cout << "START FmsAcquireForm2400_1, abilityName: "<< abilityName << std::endl;
    MAP_STR_STR params;
    Want want = SystemTestFormUtil::MakeWant("device", abilityName, bundleName, params);
    SystemTestFormUtil::StartAbility(want, abilityMs);
    EXPECT_EQ(SystemTestFormUtil::WaitCompleted(event, FORM_EVENT_ABILITY_ONACTIVED, 0), 0);
    std::string eventData1 = strFormId;
    SystemTestFormUtil::PublishEvent(FORM_EVENT_REQ_ACQUIRE_FORM_2400_1, EVENT_CODE_2410, eventData1);
    EXPECT_EQ(0, SystemTestFormUtil::WaitCompleted(event, FORM_EVENT_RECV_ACQUIRE_FORM_2400_1, EVENT_CODE_2410));
    std::string data3 = SystemTestFormUtil::GetData(event, FORM_EVENT_RECV_ACQUIRE_FORM_2400_1, EVENT_CODE_2410);
    bool result3 = data3 == "false";
    EXPECT_TRUE(result3);
    GTEST_LOG_(INFO) << "FmsAcquireForm2400_1,  result:" << result3;

    std::cout << "END FmsAcquireForm2400_1 cast temp end" << std::endl;
}

void FmsAcquireFormTest::FmsAcquireFormDeleteA(std::string strFormId)
{
    std::cout << "START FmsAcquireFormDeleteA, start." << std::endl;
    std::string bundleName = "com.ohos.form.manager.normal";
    std::string abilityName = "FormAbilityA";
    std::cout << "START FmsAcquireFormDeleteA, bundleName: " << bundleName << std::endl;
    std::cout << "START FmsAcquireFormDeleteA, abilityName: " << abilityName << std::endl;
    MAP_STR_STR params;
    Want want = SystemTestFormUtil::MakeWant("device", abilityName, bundleName, params);
    SystemTestFormUtil::StartAbility(want, abilityMs);
    EXPECT_EQ(SystemTestFormUtil::WaitCompleted(event, FORM_EVENT_ABILITY_ONACTIVED, 0), 0);
    std::string eventData = strFormId;
    SystemTestFormUtil::PublishEvent(FORM_EVENT_REQ_DELETE_FORM_COMMON, EVENT_CODE_999, eventData);
    // wait delete form
    EXPECT_EQ(0, SystemTestFormUtil::WaitCompleted(event, FORM_EVENT_RECV_DELETE_FORM_COMMON, EVENT_CODE_999));
    std::string data = SystemTestFormUtil::GetData(event, FORM_EVENT_RECV_DELETE_FORM_COMMON, EVENT_CODE_999);
    bool result = data == "true";
    EXPECT_TRUE(result);
    GTEST_LOG_(INFO) << "FmsAcquireFormDeleteA,  delete form, result:" << result;

    std::cout << "END FmsAcquireFormDeleteA end" << std::endl;
}

}  // namespace AppExecFwk
}  // namespace OHOS