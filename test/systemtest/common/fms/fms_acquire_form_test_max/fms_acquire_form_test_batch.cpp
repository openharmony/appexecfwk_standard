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
const int FORM_COUNT_200 = 200;
const int FORM_COUNT_111 = 111;
const int FORM_COUNT_112 = 112;
const int TEMP_FORM_COUNT_256 = 256;
const int TEMP_FORM_COUNT_128 = 128;

std::vector<std::string> bundleNameList = {
    "com.form.formsystemtestservicea",
    "com.form.formsystemtestserviceb",
};
std::vector<std::string> hapNameList = {
    "formSystemTestServiceA-signed",
    "formSystemTestServiceB-signed",
};

std::vector<std::string> normalFormsMaxA;
std::vector<std::string> normalFormsMaxB;
std::vector<std::string> normalFormsMaxC;

std::vector<std::string> tempFormsMaxA;
std::vector<std::string> tempFormsMaxB;
}  // namespace

namespace OHOS {
namespace AppExecFwk {
class FmsAcquireFormTestBatch : public testing::Test {
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

    void FmsAcquireForm2700(std::string strFormId);
    std::string FmsAcquireForm2900A();
    std::string FmsAcquireForm2900B();
    void FmsAcquireForm3000();
    std::string FmsAcquireForm3100(const std::string &bundleName, const std::string &abilityName);
    void FmsAcquireForm2800(std::string strFormId);
    void FmsAcquireForm3200();

    void FmsAcquireFormDeleteA(const std::string &strFormId);
    void FmsAcquireFormDeleteB(const std::string &strFormId);
    void FmsAcquireFormDeleteC(const std::string &strFormId);
    std::string FmsAcquireFormTemp(const std::string &bundleName, const std::string &abilityName);
    bool FmsAcquireFormTempForFailed(const std::string &bundleName, const std::string &abilityName);

    void FMS_acquireFormBatchA(const std::string &bundleName, const std::string &abilityName, const int count);
    void FMS_acquireFormBatchB(const std::string &bundleName, const std::string &abilityName, const int count);

    void FMS_acquireTempFormBatch(const std::string &bundleName, const std::string &abilityName, const int count);
    void FMS_deleteFormBatch();
};
std::vector<std::string> FmsAcquireFormTestBatch::eventList = {
    FORM_EVENT_RECV_DELETE_FORM_COMMON, FORM_EVENT_ABILITY_ONACTIVED, FORM_EVENT_RECV_ACQUIRE_FORM_TEMP,
    FORM_EVENT_RECV_ACQUIRE_FORM_2700, FORM_EVENT_RECV_ACQUIRE_FORM_2800, FORM_EVENT_RECV_ACQUIRE_FORM_2900,
    FORM_EVENT_RECV_ACQUIRE_FORM_2900_1, FORM_EVENT_RECV_ACQUIRE_FORM_3000, FORM_EVENT_RECV_ACQUIRE_FORM_3100,
    FORM_EVENT_RECV_ACQUIRE_FORM_3200, FORM_EVENT_RECV_ACQUIRE_FORM_BATCH, FORM_EVENT_RECV_ACQUIRE_FORM_BATCH_B,
    FORM_EVENT_RECV_CLEAR_FORM_BATCH, FORM_EVENT_RECV_ACQUIRE_TEMP_FORM_BATCH,
};


FormEvent FmsAcquireFormTestBatch::event = FormEvent();
sptr<AAFwk::IAbilityManager> FmsAcquireFormTestBatch::abilityMs = nullptr;
std::shared_ptr<FmsAcquireFormTestBatch::FormEventSubscriber> FmsAcquireFormTestBatch::subscriber = nullptr;
void FmsAcquireFormTestBatch::FormEventSubscriber::OnReceiveEvent(const CommonEventData &data)
{
    GTEST_LOG_(INFO) << "OnReceiveEvent: event=" << data.GetWant().GetAction();
    GTEST_LOG_(INFO) << "OnReceiveEvent: data=" << data.GetData();
    GTEST_LOG_(INFO) << "OnReceiveEvent: code=" << data.GetCode();
    SystemTestFormUtil::Completed(event, data.GetWant().GetAction(), data.GetCode(), data.GetData());
}

void FmsAcquireFormTestBatch::SetUpTestCase()
{
    // SystemTestFormUtil::InstallHaps(hapNameList);
    if (!SubscribeEvent()) {
        GTEST_LOG_(INFO) << "SubscribeEvent error";
    }
}

void FmsAcquireFormTestBatch::TearDownTestCase()
{
    // SystemTestFormUtil::UninstallBundle(bundleNameList);
    GTEST_LOG_(INFO) << "UnSubscribeCommonEvent calld";
    CommonEventManager::UnSubscribeCommonEvent(subscriber);
}

void FmsAcquireFormTestBatch::SetUp()
{
}

void FmsAcquireFormTestBatch::TearDown()
{
    GTEST_LOG_(INFO) << "CleanMsg calld";
    SystemTestFormUtil::CleanMsg(event);
}
bool FmsAcquireFormTestBatch::SubscribeEvent()
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
 * @tc.number: FMS_acquireForm_2900
 * @tc.name: A single host creates 256 different provider forms.
 * @tc.desc: The single host can successfully create 256 different provider forms.
 */
HWTEST_F(FmsAcquireFormTestBatch, FMS_acquireForm_2900, Function | MediumTest | Level1)
{
    std::cout << "START FMS_acquireForm_2900" << std::endl;
    std::string bundleName = "com.ohos.form.manager.normal";
    std::string abilityName = "FormAbilityA";
    int countA = Constants::MAX_RECORD_PER_APP/2;
    int countB = Constants::MAX_RECORD_PER_APP/2 - 1;
    sleep(1);
    FMS_acquireFormBatchA(bundleName, abilityName, countA);
    sleep(1);
    FMS_acquireFormBatchB(bundleName, abilityName, countB);
    int count = Constants::MAX_RECORD_PER_APP - 1;
    std::cout << "FMS_acquireForm_2900, form size of the host A:" << count << std::endl;
    
    sleep(1);
    std::string strFormId = FmsAcquireForm2900A();
    normalFormsMaxA.emplace_back(strFormId);
    std::cout << "END FMS_acquireForm_2900" << std::endl;
}
/**
 * @tc.number: FMS_acquireForm_3000
 * @tc.name: Create limit value verification using single party form.
 * @tc.desc: Failed to create the 257th host form.
 */
HWTEST_F(FmsAcquireFormTestBatch, FMS_acquireForm_3000, Function | MediumTest | Level1)
{
    sleep(1);
    std::cout << "START FMS_acquireForm_3000" << std::endl;
    std::cout << "FMS_acquireForm_3000, form size of the host A:" << normalFormsMaxA.size() << std::endl;
    FmsAcquireForm3000();
    std::cout << "END FMS_acquireForm_3000" << std::endl;
}

/**
 * @tc.number: FMS_acquireForm_2700
 * @tc.name: When the normal form reaches the maximum value (256) created by the host,
 *           the temporary form is transferred to the normal form.
 * @tc.desc: Verify that when the normal form reaches the maximum value (256) created by the single host,
 *           the conversion of the temporary form to the normal form fails.
 */
HWTEST_F(FmsAcquireFormTestBatch, FMS_acquireForm_2700, Function | MediumTest | Level1)
{
    sleep(1);
    std::cout << "START FMS_acquireForm_2700" << std::endl;
    std::cout << "FMS_acquireForm_2700, form size of the host A:" << normalFormsMaxA.size() << std::endl;
    std::string bundleNameA = "com.ohos.form.manager.normal";
    std::string abilityNameA = "FormAbilityA";
    std::string strFormId = FmsAcquireFormTemp(bundleNameA, abilityNameA);

    sleep(1);
    FmsAcquireForm2700(strFormId);
    std::cout << "END FMS_acquireForm_2700" << std::endl;

    std::cout << "the host A, delete form start" << std::endl;
    if (normalFormsMaxA.size() > 0) {
        for (int count = 0; count < normalFormsMaxA.size(); count++) {
            FmsAcquireFormDeleteA(normalFormsMaxA[count]);
        }
    }
    sleep(1);
    FMS_deleteFormBatch();
    std::cout << "the host A, delete form end" << std::endl;
}

/**
 * @tc.number: FMS_acquireForm_3100
 * @tc.name: Multiple hosts create 512 forms respectively.
 * @tc.desc: Verify that multiple hosts can create 512 forms.
 */
HWTEST_F(FmsAcquireFormTestBatch, FMS_acquireForm_3100, Function | MediumTest | Level1)
{
    sleep(1);
    std::cout << "START FMS_acquireForm_3100" << std::endl;

    std::cout << "START add form to the host A" << std::endl;
    std::string bundleNameA = "com.ohos.form.manager.normal";
    std::string abilityNameA = "FormAbilityA";
    std::cout << "bundleName: " << bundleNameA << std::endl;
    std::cout << "abilityName: " << abilityNameA << std::endl;
    FMS_acquireFormBatchA(bundleNameA, abilityNameA, FORM_COUNT_200);
    std::cout << "count:" << FORM_COUNT_200 << std::endl;
    std::cout << "END add form to the host A" << std::endl;

    sleep(1);
    std::cout << "START add form to the host B" << std::endl;
    std::string bundleNameB = "com.ohos.form.manager.normalb";
    std::string abilityNameB = "FormAbilityB";
    std::cout << "bundleName: " << bundleNameB << std::endl;
    std::cout << "abilityName: " << abilityNameB << std::endl;
    FMS_acquireFormBatchA(bundleNameB, abilityNameB, FORM_COUNT_200);
    std::cout << "count:" << FORM_COUNT_200 << std::endl;
    std::cout << "END add form to the host B" << std::endl;

    sleep(1);
    std::cout << "START add form to the host C" << std::endl;
    std::string bundleNameC = "com.ohos.form.manager.normalc";
    std::string abilityNameC = "FormAbilityC";
    std::cout << "bundleName: " << bundleNameC << std::endl;
    std::cout << "abilityName: " << abilityNameC << std::endl;
    FMS_acquireFormBatchA(bundleNameC, abilityNameC, FORM_COUNT_111);
    std::cout << "count:" << FORM_COUNT_111 << std::endl;
    std::cout << "END add form to the host C" << std::endl;

    sleep(1);
    std::string formId = FmsAcquireForm3100(bundleNameC, abilityNameC);
    normalFormsMaxC.emplace_back(formId);
    std::cout << "END FMS_acquireForm_3100" << std::endl;
}

/**
 * @tc.number: FMS_acquireForm_2800
 * @tc.name: When the normal form reaches the maximum value (512) of the form created by FMS,
 *           the temporary form will be transferred to the normal form.
 * @tc.desc: When the normal form reaches the maximum value (512) created by FMS,
 *           the conversion of temporary form to normal form fails.
 */
HWTEST_F(FmsAcquireFormTestBatch, FMS_acquireForm_2800, Function | MediumTest | Level1)
{
    sleep(1);
    std::cout << "START FMS_acquireForm_2800" << std::endl;
    std::cout << "FMS_acquireForm_2800, form size of the host A:" << FORM_COUNT_200 << std::endl;
    std::cout << "FMS_acquireForm_2800, form size of the host B:" << FORM_COUNT_200 << std::endl;
    std::cout << "FMS_acquireForm_2800, form size of the host C:" << FORM_COUNT_112 << std::endl;
    std::string bundleNameA = "com.ohos.form.manager.normal";
    std::string abilityNameA = "FormAbilityA";

    std::string strFormId = FmsAcquireFormTemp(bundleNameA, abilityNameA);

    sleep(1);
    FmsAcquireForm2800(strFormId);

    std::cout << "END FMS_acquireForm_2800" << std::endl;
}

/**
 * @tc.number: FMS_acquireForm_3200
 * @tc.name: FMS create form limit value (512) verification.
 * @tc.desc: Verify that 512 forms can be successfully created by creating forms with different users
 *           (the number of single user forms is less than 256), and the 513rd form fails to be created.
 */
HWTEST_F(FmsAcquireFormTestBatch, FMS_acquireForm_3200, Function | MediumTest | Level1)
{
    sleep(1);
    std::cout << "START FMS_acquireForm_3200" << std::endl;
    std::cout << "FMS_acquireForm_3200, form size of the host A:" << FORM_COUNT_200 << std::endl;
    std::cout << "FMS_acquireForm_3200, form size of the host B:" << FORM_COUNT_200 << std::endl;
    std::cout << "FMS_acquireForm_3200, form size of the host C:" << FORM_COUNT_112 << std::endl;
    FmsAcquireForm3200();
    std::cout << "END FMS_acquireForm_3200" << std::endl;

    std::cout << "dlete form start" << std::endl;
    if (normalFormsMaxC.size() > 0) {
        for (int count = 0; count < normalFormsMaxC.size(); count++) {
            sleep(1);
            FmsAcquireFormDeleteC(normalFormsMaxC[count]);
        }
    }
    sleep(1);
    FMS_deleteFormBatch();
    std::cout << "dlete form end" << std::endl;
}
/**
 * @tc.number: FMS_acquireForm_3300
 * @tc.name: A single host can create 256 temporary forms.
 * @tc.desc: The host of the verification form can successfully create 256 temporary forms.
 */
HWTEST_F(FmsAcquireFormTestBatch, FMS_acquireForm_3300, Function | MediumTest | Level1)
{
    sleep(1);
    std::cout << "START FMS_acquireForm_3300" << std::endl;

    std::cout << "START add temp form to the host A" << std::endl;
    std::string bundleNameA = "com.ohos.form.manager.normal";
    std::string abilityNameA = "FormAbilityA";
    std::cout << "bundleName: " << bundleNameA << std::endl;
    std::cout << "abilityName: " << abilityNameA << std::endl;
    FMS_acquireTempFormBatch(bundleNameA, abilityNameA, TEMP_FORM_COUNT_256 - 1);
    std::cout << "FMS_acquireForm_3300, temp form size of the host A:" << (TEMP_FORM_COUNT_256 - 1)<< std::endl;

    sleep(1);
    std::string strFormId = FmsAcquireFormTemp(bundleNameA, abilityNameA);
    std::cout << "FMS_acquireForm_3300, temp form size of the host A:" << TEMP_FORM_COUNT_256 << std::endl;
    std::cout << "END add temp form to the host A" << std::endl;
    std::cout << "END FMS_acquireForm_3300" << std::endl;

    sleep(1);
    std::cout << "the host A, dlete temp form start" << std::endl;
    FmsAcquireFormDeleteA(strFormId);
    sleep(1);
    FMS_deleteFormBatch();
    std::cout << "the host A, dlete temp form end" << std::endl;
}
/**
 * @tc.number: FMS_acquireForm_3400
 * @tc.name: 256 temporary forms can be created by multiple hosts.
 * @tc.desc: Verify that multiple hosts can successfully create 256 temporary forms.
 */
HWTEST_F(FmsAcquireFormTestBatch, FMS_acquireForm_3400, Function | MediumTest | Level1)
{
    sleep(1);
    std::cout << "START FMS_acquireForm_3400" << std::endl;

    std::cout << "START add temp form to the host A" << std::endl;
    std::string bundleNameA = "com.ohos.form.manager.normal";
    std::string abilityNameA = "FormAbilityA";
    std::cout << "bundleName: " << bundleNameA << std::endl;
    std::cout << "abilityName: " << abilityNameA << std::endl;
    FMS_acquireTempFormBatch(bundleNameA, abilityNameA, TEMP_FORM_COUNT_128);
    std::cout << "FMS_acquireForm_3400, temp form size:" << TEMP_FORM_COUNT_128 << std::endl;
    std::cout << "END add temp form to the host A" << std::endl;

    sleep(1);
    std::cout << "START add temp form to the host B" << std::endl;
    std::string bundleNameB = "com.ohos.form.manager.normalb";
    std::string abilityNameB = "FormAbilityB";
    std::cout << "bundleName: " << bundleNameB << std::endl;
    std::cout << "abilityName: " << abilityNameB << std::endl;
    FMS_acquireTempFormBatch(bundleNameB, abilityNameB, TEMP_FORM_COUNT_128 - 1);
    std::cout << "FMS_acquireForm_3400, temp form size:" << (TEMP_FORM_COUNT_128 - 1) << std::endl;
    sleep(1);
    std::string strFormId = FmsAcquireFormTemp(bundleNameB, abilityNameB);
    std::cout << "FMS_acquireForm_3400, temp form size:" << TEMP_FORM_COUNT_128 << std::endl;
    std::cout << "END add temp form to the host B" << std::endl;

    sleep(1);
    std::cout << "FMS_acquireForm_3400, dlete temp form start" << std::endl;
    FmsAcquireFormDeleteB(strFormId);
    sleep(1);
    FMS_deleteFormBatch();
    std::cout << "FMS_acquireForm_3400, dlete temp form end" << std::endl;

    std::cout << "END FMS_acquireForm_3400" << std::endl;
}

/**
 * @tc.number: FMS_acquireForm_3500
 * @tc.name: Create temporary form limit value (256) verification.
 * @tc.desc: Failed to create the 257th temporary form for multiple users.
 */
HWTEST_F(FmsAcquireFormTestBatch, FMS_acquireForm_3500, Function | MediumTest | Level1)
{
    sleep(1);
    std::cout << "START FMS_acquireForm_3500" << std::endl;
    std::cout << "START add temp form to the host A" << std::endl;
    std::string bundleNameA = "com.ohos.form.manager.normal";
    std::string abilityNameA = "FormAbilityA";
    std::cout << "bundleName: " << bundleNameA << std::endl;
    std::cout << "abilityName: " << abilityNameA << std::endl;
    FMS_acquireTempFormBatch(bundleNameA, abilityNameA, TEMP_FORM_COUNT_128);
    std::cout << "FMS_acquireForm_3500, temp form size:" << TEMP_FORM_COUNT_128 << std::endl;
    std::cout << "END add temp form to the host A" << std::endl;
    sleep(1);
    std::cout << "START add temp form to the host B" << std::endl;
    std::string bundleNameB = "com.ohos.form.manager.normalb";
    std::string abilityNameB = "FormAbilityB";
    std::cout << "bundleName: " << bundleNameB << std::endl;
    std::cout << "abilityName: " << abilityNameB << std::endl;
    FMS_acquireTempFormBatch(bundleNameB, abilityNameB, TEMP_FORM_COUNT_128);
    std::cout << "FMS_acquireForm_3500, temp form size:" << TEMP_FORM_COUNT_128 << std::endl;
    std::cout << "END add temp form to the host B" << std::endl;
    sleep(1);
    bool result = FmsAcquireFormTempForFailed(bundleNameB, abilityNameB);
    EXPECT_TRUE(result);
    if (result) {
        std::cout << "END add temp form to the host B, Failed to create the 257th temporary form." << std::endl;
    }
    sleep(1);
    std::cout << "FMS_acquireForm_3500, dlete temp form start" << std::endl;
    FMS_deleteFormBatch();
    std::cout << "FMS_acquireForm_3500, dlete temp form end" << std::endl;

    std::cout << "END FMS_acquireForm_3500" << std::endl;
}

std::string FmsAcquireFormTestBatch::FmsAcquireForm3100(const std::string &bundleName, const std::string &abilityName)
{
    MAP_STR_STR params;
    Want want = SystemTestFormUtil::MakeWant("device", abilityName, bundleName, params);
    SystemTestFormUtil::StartAbility(want, abilityMs);
    EXPECT_EQ(SystemTestFormUtil::WaitCompleted(event, FORM_EVENT_ABILITY_ONACTIVED, 0), 0);

    std::string eventData = FORM_EVENT_REQ_ACQUIRE_FORM_3100;
    SystemTestFormUtil::PublishEvent(FORM_EVENT_REQ_ACQUIRE_FORM_3100, EVENT_CODE_3100, eventData);

    EXPECT_EQ(0, SystemTestFormUtil::WaitCompleted(event, FORM_EVENT_RECV_ACQUIRE_FORM_3100, EVENT_CODE_3100));
    std::string strFormId = SystemTestFormUtil::GetData(event, FORM_EVENT_RECV_ACQUIRE_FORM_3100, EVENT_CODE_3100);
    bool result = !strFormId.empty();
    EXPECT_TRUE(result);
    if (!result) {
        GTEST_LOG_(INFO) << "FmsAcquireForm3100,  result:" << result;
    } else {
        GTEST_LOG_(INFO) << "FmsAcquireForm3100,  formId:" << strFormId;
    }

    EXPECT_EQ(0, SystemTestFormUtil::WaitCompleted(event, FORM_EVENT_RECV_ACQUIRE_FORM_3100, EVENT_CODE_3101));
    std::string data2 = SystemTestFormUtil::GetData(event, FORM_EVENT_RECV_ACQUIRE_FORM_3100, EVENT_CODE_3101);
    bool result2 = !data2.empty();
    EXPECT_TRUE(result2);
    GTEST_LOG_(INFO) << "FmsAcquireForm3100,  result:" << result2;

    return strFormId;
}

void FmsAcquireFormTestBatch::FmsAcquireForm2700(std::string strFormId)
{
    std::cout << "START FmsAcquireForm2700, cast temp form" << std::endl;
    std::string bundleName = "com.ohos.form.manager.normal";
    std::string abilityName = "FormAbilityA";
    MAP_STR_STR params;
    Want want = SystemTestFormUtil::MakeWant("device", abilityName, bundleName, params);
    SystemTestFormUtil::StartAbility(want, abilityMs);
    EXPECT_EQ(SystemTestFormUtil::WaitCompleted(event, FORM_EVENT_ABILITY_ONACTIVED, 0), 0);
    std::string eventData1 = strFormId;
    SystemTestFormUtil::PublishEvent(FORM_EVENT_REQ_ACQUIRE_FORM_2700, EVENT_CODE_2700, eventData1);
    EXPECT_EQ(0, SystemTestFormUtil::WaitCompleted(event, FORM_EVENT_RECV_ACQUIRE_FORM_2700, EVENT_CODE_2700));
    std::string data3 = SystemTestFormUtil::GetData(event, FORM_EVENT_RECV_ACQUIRE_FORM_2700, EVENT_CODE_2700);
    bool result3 = data3 == "false";
    EXPECT_TRUE(result3);
    GTEST_LOG_(INFO) << "FmsAcquireForm2700,  result:" << result3;
    // wait delete form
    EXPECT_EQ(0, SystemTestFormUtil::WaitCompleted(event, FORM_EVENT_RECV_DELETE_FORM_COMMON, EVENT_CODE_999));
    std::string data4 = SystemTestFormUtil::GetData(event, FORM_EVENT_RECV_DELETE_FORM_COMMON, EVENT_CODE_999);
    bool result4 = data4 == "true";
    EXPECT_TRUE(result4);
    GTEST_LOG_(INFO) << "FmsAcquireForm2700,  delete form, result:" << result4;
    std::cout << "END FmsAcquireForm2700, cast temp form" << std::endl;
}
void FmsAcquireFormTestBatch::FmsAcquireForm3200()
{
    std::cout << "START FmsAcquireForm3200" << std::endl;

    std::string bundleName = "com.ohos.form.manager.normalc";
    std::string abilityName = "FormAbilityC";
    MAP_STR_STR params;
    Want want = SystemTestFormUtil::MakeWant("device", abilityName, bundleName, params);
    SystemTestFormUtil::StartAbility(want, abilityMs);
    EXPECT_EQ(SystemTestFormUtil::WaitCompleted(event, FORM_EVENT_ABILITY_ONACTIVED, 0), 0);

    std::string eventData = FORM_EVENT_REQ_ACQUIRE_FORM_3200;
    SystemTestFormUtil::PublishEvent(FORM_EVENT_REQ_ACQUIRE_FORM_3200, EVENT_CODE_3200, eventData);
    EXPECT_EQ(0, SystemTestFormUtil::WaitCompleted(event, FORM_EVENT_RECV_ACQUIRE_FORM_3200, EVENT_CODE_3200));
    std::string data = SystemTestFormUtil::GetData(event, FORM_EVENT_RECV_ACQUIRE_FORM_3200, EVENT_CODE_3200);
    bool result = data == "false";
    EXPECT_TRUE(result);
    GTEST_LOG_(INFO) << "FmsAcquireForm3200,  result:" << result;

    std::cout << "END FmsAcquireForm3200" << std::endl;
}
void FmsAcquireFormTestBatch::FmsAcquireForm2800(std::string strFormId)
{
    std::cout << "START FmsAcquireForm2800, cast temp form" << std::endl;
    std::string bundleName = "com.ohos.form.manager.normal";
    std::string abilityName = "FormAbilityA";
    MAP_STR_STR params;
    Want want = SystemTestFormUtil::MakeWant("device", abilityName, bundleName, params);
    SystemTestFormUtil::StartAbility(want, abilityMs);
    EXPECT_EQ(SystemTestFormUtil::WaitCompleted(event, FORM_EVENT_ABILITY_ONACTIVED, 0), 0);
    std::string eventData1 = strFormId;
    SystemTestFormUtil::PublishEvent(FORM_EVENT_REQ_ACQUIRE_FORM_2800, EVENT_CODE_2800, eventData1);
    EXPECT_EQ(0, SystemTestFormUtil::WaitCompleted(event, FORM_EVENT_RECV_ACQUIRE_FORM_2800, EVENT_CODE_2800));
    std::string data3 = SystemTestFormUtil::GetData(event, FORM_EVENT_RECV_ACQUIRE_FORM_2800, EVENT_CODE_2800);
    bool result3 = data3 == "false";
    EXPECT_TRUE(result3);
    GTEST_LOG_(INFO) << "FmsAcquireForm2800,  result:" << result3;
    // wait delete form
    EXPECT_EQ(0, SystemTestFormUtil::WaitCompleted(event, FORM_EVENT_RECV_DELETE_FORM_COMMON, EVENT_CODE_999));
    std::string data4 = SystemTestFormUtil::GetData(event, FORM_EVENT_RECV_DELETE_FORM_COMMON, EVENT_CODE_999);
    bool result4 = data4 == "true";
    EXPECT_TRUE(result4);
    GTEST_LOG_(INFO) << "FmsAcquireForm2800,  delete form, result:" << result4;
    std::cout << "END FmsAcquireForm2800, cast temp form" << std::endl;
}

std::string FmsAcquireFormTestBatch::FmsAcquireForm2900A()
{
    std::cout << "START FmsAcquireForm2900A, Provider A" << std::endl;
    std::string bundleName = "com.ohos.form.manager.normal";
    std::string abilityName = "FormAbilityA";
    MAP_STR_STR params;
    Want want = SystemTestFormUtil::MakeWant("device", abilityName, bundleName, params);
    SystemTestFormUtil::StartAbility(want, abilityMs);
    EXPECT_EQ(SystemTestFormUtil::WaitCompleted(event, FORM_EVENT_ABILITY_ONACTIVED, 0), 0);

    std::string eventData = FORM_EVENT_REQ_ACQUIRE_FORM_2900;
    SystemTestFormUtil::PublishEvent(FORM_EVENT_REQ_ACQUIRE_FORM_2900, EVENT_CODE_2900, eventData);

    EXPECT_EQ(0, SystemTestFormUtil::WaitCompleted(event, FORM_EVENT_RECV_ACQUIRE_FORM_2900, EVENT_CODE_2900));
    std::string strFormId = SystemTestFormUtil::GetData(event, FORM_EVENT_RECV_ACQUIRE_FORM_2900, EVENT_CODE_2900);
    bool result = !strFormId.empty();
    EXPECT_TRUE(result);
    if (!result) {
        GTEST_LOG_(INFO) << "FmsAcquireForm2900A,  result:" << result;
    } else {
        GTEST_LOG_(INFO) << "FmsAcquireForm2900A,  formId:" << strFormId;
    }

    EXPECT_EQ(0, SystemTestFormUtil::WaitCompleted(event, FORM_EVENT_RECV_ACQUIRE_FORM_2900, EVENT_CODE_2901));
    std::string data2 = SystemTestFormUtil::GetData(event, FORM_EVENT_RECV_ACQUIRE_FORM_2900, EVENT_CODE_2901);
    bool result2 = !data2.empty();
    EXPECT_TRUE(result2);
    GTEST_LOG_(INFO) << "FmsAcquireForm2900A,  result:" << result2;

    std::cout << "END FmsAcquireForm2900A, Provider A" << std::endl;
    return strFormId;
}
void FmsAcquireFormTestBatch::FmsAcquireForm3000()
{
    std::cout << "START FmsAcquireForm3000" << std::endl;

    std::string bundleName = "com.ohos.form.manager.normal";
    std::string abilityName = "FormAbilityA";
    MAP_STR_STR params;
    Want want = SystemTestFormUtil::MakeWant("device", abilityName, bundleName, params);
    SystemTestFormUtil::StartAbility(want, abilityMs);
    EXPECT_EQ(SystemTestFormUtil::WaitCompleted(event, FORM_EVENT_ABILITY_ONACTIVED, 0), 0);

    std::string eventData = FORM_EVENT_REQ_ACQUIRE_FORM_3000;
    SystemTestFormUtil::PublishEvent(FORM_EVENT_REQ_ACQUIRE_FORM_3000, EVENT_CODE_3000, eventData);
    EXPECT_EQ(0, SystemTestFormUtil::WaitCompleted(event, FORM_EVENT_RECV_ACQUIRE_FORM_3000, EVENT_CODE_3000));
    std::string data = SystemTestFormUtil::GetData(event, FORM_EVENT_RECV_ACQUIRE_FORM_3000, EVENT_CODE_3000);
    bool result = data == "false";
    EXPECT_TRUE(result);
    GTEST_LOG_(INFO) << "FmsAcquireForm3000,  result:" << result;

    std::cout << "END FmsAcquireForm3000" << std::endl;
}

void FmsAcquireFormTestBatch::FMS_acquireFormBatchA(const std::string &bundleName, const std::string &abilityName,
    const int count)
{
    std::cout << "START FMS_acquireFormBatchA" << std::endl;
    MAP_STR_STR params;
    Want want = SystemTestFormUtil::MakeWant("device", abilityName, bundleName, params);
    SystemTestFormUtil::StartAbility(want, abilityMs);
    EXPECT_EQ(SystemTestFormUtil::WaitCompleted(event, FORM_EVENT_ABILITY_ONACTIVED, 0), 0);

    std::string eventData = std::to_string(count);
    SystemTestFormUtil::PublishEvent(FORM_EVENT_REQ_ACQUIRE_FORM_BATCH, EVENT_CODE_BATCH, eventData);

    EXPECT_EQ(0, SystemTestFormUtil::WaitCompleted(event, FORM_EVENT_RECV_ACQUIRE_FORM_BATCH, EVENT_CODE_BATCH));
    std::string data = SystemTestFormUtil::GetData(event, FORM_EVENT_RECV_ACQUIRE_FORM_BATCH, EVENT_CODE_BATCH);
    bool result = data == "true";
    EXPECT_TRUE(result);
    if (result) {
        GTEST_LOG_(INFO) << "FMS_acquireFormBatchA, for provider A, count: " << eventData;
    } else {
        GTEST_LOG_(INFO) << "FMS_acquireFormBatchA,  result:" << result;
    }
    std::cout << "END FMS_acquireFormBatchA" << std::endl;
}

void FmsAcquireFormTestBatch::FMS_acquireFormBatchB(const std::string &bundleName, const std::string &abilityName,
    const int count)
{
    std::cout << "START FMS_acquireFormBatchB" << std::endl;
    MAP_STR_STR params;
    Want want = SystemTestFormUtil::MakeWant("device", abilityName, bundleName, params);
    SystemTestFormUtil::StartAbility(want, abilityMs);
    EXPECT_EQ(SystemTestFormUtil::WaitCompleted(event, FORM_EVENT_ABILITY_ONACTIVED, 0), 0);

    std::string eventData = std::to_string(count);
    SystemTestFormUtil::PublishEvent(FORM_EVENT_REQ_ACQUIRE_FORM_BATCH_B, EVENT_CODE_BATCH_B, eventData);

    EXPECT_EQ(0, SystemTestFormUtil::WaitCompleted(event, FORM_EVENT_RECV_ACQUIRE_FORM_BATCH_B, EVENT_CODE_BATCH_B));
    std::string data = SystemTestFormUtil::GetData(event, FORM_EVENT_RECV_ACQUIRE_FORM_BATCH_B, EVENT_CODE_BATCH_B);
    bool result = data == "true";
    EXPECT_TRUE(result);
    if (result) {
        GTEST_LOG_(INFO) << "FMS_acquireFormBatchB, for provider B, count: " << eventData;
    } else {
        GTEST_LOG_(INFO) << "FMS_acquireFormBatchB,  result:" << result;
    }
 
    std::cout << "END FMS_acquireFormBatchB" << std::endl;
}
void FmsAcquireFormTestBatch::FMS_acquireTempFormBatch(const std::string &bundleName, const std::string &abilityName,
    const int count)
{
    std::cout << "START FMS_acquireTempFormBatch" << std::endl;
    MAP_STR_STR params;
    Want want = SystemTestFormUtil::MakeWant("device", abilityName, bundleName, params);
    SystemTestFormUtil::StartAbility(want, abilityMs);
    EXPECT_EQ(SystemTestFormUtil::WaitCompleted(event, FORM_EVENT_ABILITY_ONACTIVED, 0), 0);

    std::string eventData = std::to_string(count);
    SystemTestFormUtil::PublishEvent(FORM_EVENT_REQ_ACQUIRE_TEMP_FORM_BATCH, EVENT_CODE_TEMP_BATCH, eventData);

    EXPECT_EQ(0, SystemTestFormUtil::WaitCompleted(event, FORM_EVENT_RECV_ACQUIRE_TEMP_FORM_BATCH,
        EVENT_CODE_TEMP_BATCH));
    std::string data = SystemTestFormUtil::GetData(event, FORM_EVENT_RECV_ACQUIRE_TEMP_FORM_BATCH,
        EVENT_CODE_TEMP_BATCH);
    bool result = data == "true";
    EXPECT_TRUE(result);
    if (result) {
        GTEST_LOG_(INFO) << "FMS_acquireTempFormBatch, count: " << eventData;
    } else {
        GTEST_LOG_(INFO) << "FMS_acquireTempFormBatch,  result:" << result;
    }
   
    std::cout << "END FMS_acquireTempFormBatch" << std::endl;
}

std::string FmsAcquireFormTestBatch::FmsAcquireFormTemp(const std::string &bundleName, const std::string &abilityName)
{
    std::cout << "START FmsAcquireFormTemp, add temp form" << std::endl;
    MAP_STR_STR params;
    Want want = SystemTestFormUtil::MakeWant("device", abilityName, bundleName, params);
    SystemTestFormUtil::StartAbility(want, abilityMs);
    EXPECT_EQ(SystemTestFormUtil::WaitCompleted(event, FORM_EVENT_ABILITY_ONACTIVED, 0), 0);
    std::string eventData = FORM_EVENT_REQ_ACQUIRE_FORM_TEMP;
    SystemTestFormUtil::PublishEvent(FORM_EVENT_REQ_ACQUIRE_FORM_TEMP, EVENT_CODE_TEMP, eventData);
    EXPECT_EQ(0, SystemTestFormUtil::WaitCompleted(event, FORM_EVENT_RECV_ACQUIRE_FORM_TEMP, EVENT_CODE_TEMP));
    std::string strFormId = SystemTestFormUtil::GetData(event, FORM_EVENT_RECV_ACQUIRE_FORM_TEMP, EVENT_CODE_TEMP);
    bool result = !strFormId.empty();
    EXPECT_TRUE(result);
    if (!result) {
        GTEST_LOG_(INFO) << "FmsAcquireFormTemp,  result:" << result;
    } else {
        GTEST_LOG_(INFO) << "FmsAcquireFormTemp,  formId:" << strFormId;
    }
    EXPECT_EQ(0, SystemTestFormUtil::WaitCompleted(event, FORM_EVENT_RECV_ACQUIRE_FORM_TEMP, EVENT_CODE_TEMP_1));
    std::string data2 = SystemTestFormUtil::GetData(event, FORM_EVENT_RECV_ACQUIRE_FORM_TEMP, EVENT_CODE_TEMP_1);
    bool result2 = !data2.empty();
    EXPECT_TRUE(result2);
    if (!result2) {
        GTEST_LOG_(INFO) << "FmsAcquireFormTemp,  result:" << result2;
    } else {
        GTEST_LOG_(INFO) << "FmsAcquireFormTemp,  formData:" << data2;
    }
    std::cout << "END FmsAcquireFormTemp, add temp form" << std::endl;

    return strFormId;
}
bool FmsAcquireFormTestBatch::FmsAcquireFormTempForFailed(const std::string &bundleName, const std::string &abilityName)
{
    std::cout << "START FmsAcquireFormTempForFailed, add temp form" << std::endl;
    MAP_STR_STR params;
    Want want = SystemTestFormUtil::MakeWant("device", abilityName, bundleName, params);
    SystemTestFormUtil::StartAbility(want, abilityMs);
    EXPECT_EQ(SystemTestFormUtil::WaitCompleted(event, FORM_EVENT_ABILITY_ONACTIVED, 0), 0);
    std::string eventData = FORM_EVENT_REQ_ACQUIRE_FORM_TEMP;
    SystemTestFormUtil::PublishEvent(FORM_EVENT_REQ_ACQUIRE_FORM_TEMP, EVENT_CODE_TEMP, eventData);
    EXPECT_EQ(0, SystemTestFormUtil::WaitCompleted(event, FORM_EVENT_RECV_ACQUIRE_FORM_TEMP, EVENT_CODE_TEMP));
    std::string strFormId = SystemTestFormUtil::GetData(event, FORM_EVENT_RECV_ACQUIRE_FORM_TEMP, EVENT_CODE_TEMP);
    bool result = strFormId.empty();
    EXPECT_TRUE(result);
    GTEST_LOG_(INFO) << "FmsAcquireFormTempForFailed,  result:" << result;

    std::cout << "END FmsAcquireFormTempForFailed, add temp form" << std::endl;

    return result;
}

void FmsAcquireFormTestBatch::FmsAcquireFormDeleteA(const std::string &strFormId)
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

void FmsAcquireFormTestBatch::FmsAcquireFormDeleteB(const std::string &strFormId)
{
    std::cout << "START FmsAcquireFormDeleteB, start." << std::endl;
    std::string bundleName = "com.ohos.form.manager.normalb";
    std::string abilityName = "FormAbilityB";
    std::cout << "START FmsAcquireFormDeleteB, bundleName: " << bundleName << std::endl;
    std::cout << "START FmsAcquireFormDeleteB, abilityName: " << abilityName << std::endl;
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
    GTEST_LOG_(INFO) << "FmsAcquireFormDeleteB,  delete form, result:" << result;

    std::cout << "END FmsAcquireFormDeleteB end" << std::endl;
}
void FmsAcquireFormTestBatch::FmsAcquireFormDeleteC(const std::string &strFormId)
{
    std::cout << "START FmsAcquireFormDeleteC, start." << std::endl;
    std::string bundleName = "com.ohos.form.manager.normalc";
    std::string abilityName = "FormAbilityC";
    std::cout << "START FmsAcquireFormDeleteC, bundleName: " << bundleName << std::endl;
    std::cout << "START FmsAcquireFormDeleteC, abilityName: " << abilityName << std::endl;
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
    GTEST_LOG_(INFO) << "FmsAcquireFormDeleteC,  delete form, result:" << result;

    std::cout << "END FmsAcquireFormDeleteC end" << std::endl;
}

void FmsAcquireFormTestBatch::FMS_deleteFormBatch()
{
    std::cout << "START FMS_deleteFormBatch, delete forms" << std::endl;
    std::string bundleName = "com.ohos.form.manager.normal";
    std::string abilityName = "FormAbilityA";
    MAP_STR_STR params;
    Want want = SystemTestFormUtil::MakeWant("device", abilityName, bundleName, params);
    SystemTestFormUtil::StartAbility(want, abilityMs);
    EXPECT_EQ(SystemTestFormUtil::WaitCompleted(event, FORM_EVENT_ABILITY_ONACTIVED, 0), 0);
    std::string eventData = FORM_EVENT_REQ_CLEAR_FORM_BATCH;
    SystemTestFormUtil::PublishEvent(FORM_EVENT_REQ_CLEAR_FORM_BATCH, EVENT_CODE_TEMP, eventData);
    EXPECT_EQ(0, SystemTestFormUtil::WaitCompleted(event, FORM_EVENT_RECV_CLEAR_FORM_BATCH, EVENT_CODE_CLEAR_BATCH));
    std::string data = SystemTestFormUtil::GetData(event, FORM_EVENT_RECV_CLEAR_FORM_BATCH, EVENT_CODE_CLEAR_BATCH);
    bool result = data == "true";
    EXPECT_TRUE(result);
    GTEST_LOG_(INFO) << "FMS_deleteFormBatch,  result:" << result;

    std::cout << "END FMS_deleteFormBatch, delete forms" << std::endl;
}
}  // namespace AppExecFwk
}  // namespace OHOS