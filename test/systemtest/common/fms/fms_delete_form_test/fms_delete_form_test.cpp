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
};
std::vector<std::string> hapNameList = {
    "fmsSystemTestHostNotSys-signed",
};
}  // namespace

namespace OHOS {
namespace AppExecFwk {
class FmsDeleteFormTest : public testing::Test {
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

    static sptr<AAFwk::IAbilityManager> abilityManager;
    static FormEvent event;
    static std::vector<std::string> eventList;
    static std::shared_ptr<FormEventSubscriber> subscriber;
    bool FmsGetCacheInfoByFormId(std::int64_t formId);
    bool FmsGetHostInfoByFormId(std::int64_t formId, std::string hostBundleName);
    bool FmsGetStorageFormInfos(std::int64_t formId);
    bool FmsGetFormTimerTask(std::int64_t formId);
    void FMS_deleteForm_1400_A();
    void FMS_deleteForm_1400_B();
    void FMS_deleteForm_1500_acquireForm(const std::string &bundleName, const std::string &abilityName,
        std::string &outformId);
    void FMS_deleteForm_1500_clearForm(const std::string &bundleName, const std::string &abilityName,
        std::string &outformId);
    void FMS_deleteForm_1500_Create_Delete(std::string delFormId, std::string &createFormId);
    void FMS_deleteForm_1600_acquireForm(const std::string &bundleName, const std::string &abilityName,
        std::string &outformId);
    void FMS_deleteForm_1600_clearForm(const std::string &bundleName, const std::string &abilityName,
        std::string &outformId);
    void FMS_deleteForm_1600_Delete_Create(std::string delFormId, std::string &createFormId);
    void FMS_deleteForm_1700_acquireForm(const std::string &bundleName, const std::string &abilityName,
        std::string &outformId);
    void FMS_deleteForm_1700_clearForm(const std::string &bundleName, const std::string &abilityName,
        std::string &outformId);
    void FMS_deleteForm_1700_Delete_Create(std::string delFormId, std::string &createFormId);
    void FMS_acquireTempFormBatch(const std::string &bundleName, const std::string &abilityName,
        const int count);
    void FMS_acquireNormalFormBatch(const std::string &bundleName, const std::string &abilityName,
        const int count);
    void FMS_deleteFormBatch(const std::string &bundleName, const std::string &abilityName);
};

std::vector<std::string> FmsDeleteFormTest::eventList = {
    FORM_EVENT_ABILITY_ONACTIVED,
    FORM_EVENT_RECV_DELETE_FORM_0100,
    FORM_EVENT_RECV_DELETE_FORM_0200,
    FORM_EVENT_RECV_DELETE_FORM_0300,
    FORM_EVENT_RECV_DELETE_FORM_0400,
    FORM_EVENT_RECV_DELETE_FORM_0500,
    FORM_EVENT_RECV_DELETE_FORM_0600,
    FORM_EVENT_RECV_DELETE_FORM_0700,
    FORM_EVENT_RECV_DELETE_FORM_0800,
    FORM_EVENT_RECV_DELETE_FORM_0900,
    FORM_EVENT_RECV_DELETE_FORM_1000,
    FORM_EVENT_RECV_DELETE_FORM_1100,
    FORM_EVENT_RECV_DELETE_FORM_1200,
    FORM_EVENT_RECV_DELETE_FORM_1201,
    FORM_EVENT_RECV_DELETE_FORM_1400,
    FORM_EVENT_RECV_DELETE_FORM_1401,
    FORM_EVENT_RECV_DELETE_FORM_1500,
    FORM_EVENT_RECV_DELETE_FORM_1501,
    FORM_EVENT_RECV_DELETE_FORM_1502,
    FORM_EVENT_RECV_DELETE_FORM_1600,
    FORM_EVENT_RECV_DELETE_FORM_1601,
    FORM_EVENT_RECV_DELETE_FORM_1602,
    FORM_EVENT_RECV_DELETE_FORM_1700,
    FORM_EVENT_RECV_DELETE_FORM_1701,
    FORM_EVENT_RECV_DELETE_FORM_1702,
    FORM_EVENT_RECV_ONE_NORMAL_FORM,
    FORM_EVENT_RECV_ONE_NORMAL_FORM_DEL,
    FORM_EVENT_RECV_ONE_NORMAL_FORM_B,
    FORM_EVENT_RECV_ONE_NORMAL_FORM_B_DEL,
    COMMON_EVENT_ON_DELETE,
    FORM_EVENT_RECV_ACQUIRE_FORM_BATCH,
    FORM_EVENT_RECV_ACQUIRE_TEMP_FORM_BATCH,
    FORM_EVENT_RECV_CLEAR_FORM_BATCH
};

FormEvent FmsDeleteFormTest::event = FormEvent();
sptr<AAFwk::IAbilityManager> FmsDeleteFormTest::abilityManager = nullptr;
std::shared_ptr<FmsDeleteFormTest::FormEventSubscriber> FmsDeleteFormTest::subscriber = nullptr;
void FmsDeleteFormTest::FormEventSubscriber::OnReceiveEvent(const CommonEventData &data)
{
    GTEST_LOG_(INFO) << "OnReceiveEvent: event=" << data.GetWant().GetAction();
    GTEST_LOG_(INFO) << "OnReceiveEvent: data=" << data.GetData();
    GTEST_LOG_(INFO) << "OnReceiveEvent: code=" << data.GetCode();
    SystemTestFormUtil::Completed(event, data.GetWant().GetAction(), data.GetCode(), data.GetData());
}

void FmsDeleteFormTest::SetUpTestCase()
{
    std::cout << "START Install============" << std::endl;
    SystemTestFormUtil::InstallHaps(hapNameList);
    if (!SubscribeEvent()) {
        GTEST_LOG_(INFO) << "SubscribeEvent error";
    }
}

void FmsDeleteFormTest::TearDownTestCase()
{
    std::cout << "START Uninstall============" << std::endl;
    SystemTestFormUtil::UninstallBundle(bundleNameList);
    CommonEventManager::UnSubscribeCommonEvent(subscriber);
}

void FmsDeleteFormTest::SetUp()
{
}

void FmsDeleteFormTest::TearDown()
{
    SystemTestFormUtil::CleanMsg(event);
}
bool FmsDeleteFormTest::SubscribeEvent()
{
    std::vector<std::string> eventList = {
        FORM_EVENT_ABILITY_ONACTIVED,
        FORM_EVENT_RECV_DELETE_FORM_0100,
        FORM_EVENT_RECV_DELETE_FORM_0200,
        FORM_EVENT_RECV_DELETE_FORM_0300,
        FORM_EVENT_RECV_DELETE_FORM_0400,
        FORM_EVENT_RECV_DELETE_FORM_0500,
        FORM_EVENT_RECV_DELETE_FORM_0600,
        FORM_EVENT_RECV_DELETE_FORM_0700,
        FORM_EVENT_RECV_DELETE_FORM_0800,
        FORM_EVENT_RECV_DELETE_FORM_0900,
        FORM_EVENT_RECV_DELETE_FORM_1000,
        FORM_EVENT_RECV_DELETE_FORM_1100,
        FORM_EVENT_RECV_DELETE_FORM_1200,
        FORM_EVENT_RECV_DELETE_FORM_1201,
        FORM_EVENT_RECV_DELETE_FORM_1400,
        FORM_EVENT_RECV_DELETE_FORM_1401,
        FORM_EVENT_RECV_DELETE_FORM_1500,
        FORM_EVENT_RECV_DELETE_FORM_1501,
        FORM_EVENT_RECV_DELETE_FORM_1502,
        FORM_EVENT_RECV_DELETE_FORM_1600,
        FORM_EVENT_RECV_DELETE_FORM_1601,
        FORM_EVENT_RECV_DELETE_FORM_1602,
        FORM_EVENT_RECV_DELETE_FORM_1700,
        FORM_EVENT_RECV_DELETE_FORM_1701,
        FORM_EVENT_RECV_DELETE_FORM_1702,
        FORM_EVENT_RECV_ONE_NORMAL_FORM,
        FORM_EVENT_RECV_ONE_NORMAL_FORM_DEL,
        FORM_EVENT_RECV_ONE_NORMAL_FORM_B,
        FORM_EVENT_RECV_ONE_NORMAL_FORM_B_DEL,
        COMMON_EVENT_ON_DELETE,
        FORM_EVENT_RECV_ACQUIRE_FORM_BATCH,
        FORM_EVENT_RECV_ACQUIRE_TEMP_FORM_BATCH,
        FORM_EVENT_RECV_CLEAR_FORM_BATCH
    };
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
 * @tc.number: FMS_deleteForm_0100
 * @tc.name: host is not a system app
 * @tc.desc: 1.delete the form that formID is 1
 *           2.Verify the result is false
 */
HWTEST_F(FmsDeleteFormTest, FMS_deleteForm_0100, Function | MediumTest | Level0)
{
    std::cout << "============START FMS_deleteForm_0100" << std::endl;

    std::string bundleName = "com.ohos.form.manager.notsystemapp";
    std::string abilityName = "FormAbilityNotSys";
    MAP_STR_STR params;
    Want want = SystemTestFormUtil::MakeWant("device", abilityName, bundleName, params);
    SystemTestFormUtil::StartAbility(want, abilityManager);
    EXPECT_EQ(SystemTestFormUtil::WaitCompleted(event, FORM_EVENT_ABILITY_ONACTIVED, 0), 0);

    std::string eventData = FORM_EVENT_REQ_DELETE_FORM_0100;
    SystemTestFormUtil::PublishEvent(FORM_EVENT_REQ_DELETE_FORM_0100, EVENT_CODE_100, eventData);
    EXPECT_EQ(0, SystemTestFormUtil::WaitCompleted(event, FORM_EVENT_RECV_DELETE_FORM_0100, EVENT_CODE_100));
    std::string data = SystemTestFormUtil::GetData(event, FORM_EVENT_RECV_DELETE_FORM_0100, EVENT_CODE_100);
    bool result = data == "false";
    EXPECT_TRUE(result);
    if (!result) {
        GTEST_LOG_(INFO) << "FMS_deleteForm_0100,  result:" << result;
    }

    std::cout << "============END FMS_deleteForm_0100" << std::endl;
}

/**
 * @tc.number: FMS_deleteForm_0200
 * @tc.name: host does not have permission
 * @tc.desc: 1.delete the form that formID is 1
 *           2.Verify the result is false
 */
HWTEST_F(FmsDeleteFormTest, FMS_deleteForm_0200, Function | MediumTest | Level0)
{
    std::cout << "============START FMS_deleteForm_0200" << std::endl;

    std::string bundleName = "com.ohos.form.manager.nopermission";
    std::string abilityName = "FormAbilityNoPerm";
    MAP_STR_STR params;
    Want want = SystemTestFormUtil::MakeWant("device", abilityName, bundleName, params);
    SystemTestFormUtil::StartAbility(want, abilityManager);
    EXPECT_EQ(SystemTestFormUtil::WaitCompleted(event, FORM_EVENT_ABILITY_ONACTIVED, 0), 0);

    std::string eventData = FORM_EVENT_REQ_DELETE_FORM_0200;
    SystemTestFormUtil::PublishEvent(FORM_EVENT_REQ_DELETE_FORM_0200, EVENT_CODE_200, eventData);
    EXPECT_EQ(0, SystemTestFormUtil::WaitCompleted(event, FORM_EVENT_RECV_DELETE_FORM_0200, EVENT_CODE_200));
    std::string data = SystemTestFormUtil::GetData(event, FORM_EVENT_RECV_DELETE_FORM_0200, EVENT_CODE_200);
    bool result = data == "false";
    EXPECT_TRUE(result);
    if (!result) {
        GTEST_LOG_(INFO) << "FMS_deleteForm_0200,  result:" << result;
    }

    std::cout << "============END FMS_deleteForm_0200" << std::endl;
}

/**
 * @tc.number: FMS_deleteForm_0300
 * @tc.name: formID id is error(formID < 0)
 * @tc.desc: 1.delete the form
 *           2.Verify the result is false
 */
HWTEST_F(FmsDeleteFormTest, FMS_deleteForm_0300, Function | MediumTest | Level0)
{
    std::cout << "============START FMS_deleteForm_0300" << std::endl;

    std::string bundleName = "com.ohos.form.manager.normal";
    std::string abilityName = "FormAbilityDeleteForm";
    MAP_STR_STR params;
    Want want = SystemTestFormUtil::MakeWant("device", abilityName, bundleName, params);
    SystemTestFormUtil::StartAbility(want, abilityManager);
    EXPECT_EQ(SystemTestFormUtil::WaitCompleted(event, FORM_EVENT_ABILITY_ONACTIVED, 0), 0);

    std::string eventData = FORM_EVENT_REQ_DELETE_FORM_0300;
    SystemTestFormUtil::PublishEvent(FORM_EVENT_REQ_DELETE_FORM_0300, EVENT_CODE_300, eventData);
    EXPECT_EQ(0, SystemTestFormUtil::WaitCompleted(event, FORM_EVENT_RECV_DELETE_FORM_0300, EVENT_CODE_300));
    std::string data = SystemTestFormUtil::GetData(event, FORM_EVENT_RECV_DELETE_FORM_0300, EVENT_CODE_300);
    bool result = data == "false";
    EXPECT_TRUE(result);
    if (!result) {
        GTEST_LOG_(INFO) << "FMS_deleteForm_0300,  result:" << result;
    }
    // can receive onDelete
    EXPECT_EQ(0, SystemTestFormUtil::WaitCompleted(event, COMMON_EVENT_ON_DELETE,
        FORM_EVENT_TRIGGER_RESULT::FORM_EVENT_TRIGGER_RESULT_OK));
    SystemTestFormUtil::CleanMsg(event);
    std::cout << "============END FMS_deleteForm_0300" << std::endl;
}

/**
 * @tc.number: FMS_deleteForm_0400
 * @tc.name: formID id is error(formID = 0)
 * @tc.desc: 1.delete the form
 *           2.Verify the result is false
 */
HWTEST_F(FmsDeleteFormTest, FMS_deleteForm_0400, Function | MediumTest | Level0)
{
    std::cout << "============START FMS_deleteForm_0400" << std::endl;

    std::string bundleName = "com.ohos.form.manager.normal";
    std::string abilityName = "FormAbilityDeleteForm";
    MAP_STR_STR params;
    Want want = SystemTestFormUtil::MakeWant("device", abilityName, bundleName, params);
    SystemTestFormUtil::StartAbility(want, abilityManager);
    EXPECT_EQ(SystemTestFormUtil::WaitCompleted(event, FORM_EVENT_ABILITY_ONACTIVED, 0), 0);

    std::string eventData = FORM_EVENT_REQ_DELETE_FORM_0400;
    SystemTestFormUtil::PublishEvent(FORM_EVENT_REQ_DELETE_FORM_0400, EVENT_CODE_400, eventData);
    EXPECT_EQ(0, SystemTestFormUtil::WaitCompleted(event, FORM_EVENT_RECV_DELETE_FORM_0400, EVENT_CODE_400));
    std::string data = SystemTestFormUtil::GetData(event, FORM_EVENT_RECV_DELETE_FORM_0400, EVENT_CODE_400);
    bool result = data == "false";
    EXPECT_TRUE(result);
    if (!result) {
        GTEST_LOG_(INFO) << "FMS_deleteForm_0400,  result:" << result;
    }
    // can receive onDelete
    EXPECT_EQ(0, SystemTestFormUtil::WaitCompleted(event, COMMON_EVENT_ON_DELETE,
        FORM_EVENT_TRIGGER_RESULT::FORM_EVENT_TRIGGER_RESULT_OK));
    SystemTestFormUtil::CleanMsg(event);
    std::cout << "============END FMS_deleteForm_0400" << std::endl;
}

/**
 * @tc.number: FMS_deleteForm_0500
 * @tc.name: formID id is error because formId is not self
 * @tc.desc: 1.host A create a formA
 *           2.host B delete the formA
 *           2.Verify the result is false
 */
HWTEST_F(FmsDeleteFormTest, FMS_deleteForm_0500, Function | MediumTest | Level1)
{
    std::cout << "============START FMS_deleteForm_0500" << std::endl;

    std::string bundleName = "com.ohos.form.manager.commona";
    std::string abilityName = "FormAbilityCommonA";
    MAP_STR_STR params;
    Want want = SystemTestFormUtil::MakeWant("device", abilityName, bundleName, params);
    SystemTestFormUtil::StartAbility(want, abilityManager);
    EXPECT_EQ(SystemTestFormUtil::WaitCompleted(event, FORM_EVENT_ABILITY_ONACTIVED, 0), 0);

    std::string eventData = "false"; // normal form
    SystemTestFormUtil::PublishEvent(FORM_EVENT_REQ_ONE_NORMAL_FORM, EVENT_CODE_100, eventData);
    EXPECT_EQ(0, SystemTestFormUtil::WaitCompleted(event, FORM_EVENT_RECV_ONE_NORMAL_FORM, EVENT_CODE_100));
    std::string data = SystemTestFormUtil::GetData(event, FORM_EVENT_RECV_ONE_NORMAL_FORM, EVENT_CODE_100);
    
    GTEST_LOG_(INFO) << "FMS_deleteForm_0500,  data:[" << data << "]" << std::endl;
    bool result1 = data != "";
    EXPECT_TRUE(result1);

    if (data != "") {
        std::string bundleName2 = "com.ohos.form.manager.normal";
        std::string abilityName2 = "FormAbilityDeleteForm";
        MAP_STR_STR params2;
        Want want2 = SystemTestFormUtil::MakeWant("device", abilityName2, bundleName2, params2);
        SystemTestFormUtil::StartAbility(want2, abilityManager);
        EXPECT_EQ(SystemTestFormUtil::WaitCompleted(event, FORM_EVENT_ABILITY_ONACTIVED, 0), 0);

        std::string eventData2 = data;
        SystemTestFormUtil::PublishEvent(FORM_EVENT_REQ_DELETE_FORM_0500, EVENT_CODE_500, eventData2);
        EXPECT_EQ(0, SystemTestFormUtil::WaitCompleted(event, FORM_EVENT_RECV_DELETE_FORM_0500, EVENT_CODE_500));
        std::string data2 = SystemTestFormUtil::GetData(event, FORM_EVENT_RECV_DELETE_FORM_0500, EVENT_CODE_500);
        bool result2 = data2 == "false";
        EXPECT_TRUE(result2);
        if (!result2) {
            GTEST_LOG_(INFO) << "FMS_deleteForm_0500,  result2:" << result2;
        }
        SystemTestFormUtil::PublishEvent(FORM_EVENT_REQ_ONE_NORMAL_FORM_DEL, EVENT_CODE_101, data);
        EXPECT_EQ(0, SystemTestFormUtil::WaitCompleted(event, FORM_EVENT_RECV_ONE_NORMAL_FORM_DEL, EVENT_CODE_101));
        // can receive onDelete
        EXPECT_EQ(0, SystemTestFormUtil::WaitCompleted(event, COMMON_EVENT_ON_DELETE,
            FORM_EVENT_TRIGGER_RESULT::FORM_EVENT_TRIGGER_RESULT_OK));
    }
    SystemTestFormUtil::CleanMsg(event);

    std::cout << "============END FMS_deleteForm_0500" << std::endl;
}

/**
 * @tc.number: FMS_deleteForm_0600
 * @tc.name: Delete a normal form and the form reference is not 0 after deletion
 * @tc.desc: 1.host A create a normal formA
 *           2.host B create a form with formA's ID
 *           3.delete formA, and verify the result is true
 */
HWTEST_F(FmsDeleteFormTest, FMS_deleteForm_0600, Function | MediumTest | Level1)
{
    std::cout << "============START FMS_deleteForm_0600" << std::endl;
    std::string bundleName = "com.ohos.form.manager.commona";
    std::string abilityName = "FormAbilityCommonA";
    MAP_STR_STR params;
    Want want = SystemTestFormUtil::MakeWant("device", abilityName, bundleName, params);
    SystemTestFormUtil::StartAbility(want, abilityManager);
    EXPECT_EQ(SystemTestFormUtil::WaitCompleted(event, FORM_EVENT_ABILITY_ONACTIVED, 0), 0);

    std::string eventData = "false"; // normal form
    SystemTestFormUtil::PublishEvent(FORM_EVENT_REQ_ONE_NORMAL_FORM, EVENT_CODE_100, eventData);
    EXPECT_EQ(0, SystemTestFormUtil::WaitCompleted(event, FORM_EVENT_RECV_ONE_NORMAL_FORM, EVENT_CODE_100));
    std::string formOne = SystemTestFormUtil::GetData(event, FORM_EVENT_RECV_ONE_NORMAL_FORM, EVENT_CODE_100);
    GTEST_LOG_(INFO) << "FMS_deleteForm_0600,  formOne:[" << formOne << "]" << std::endl;
    if (formOne != "") {
        std::string bundleName2 = "com.ohos.form.manager.normal";
        std::string abilityName2 = "FormAbilityDeleteForm";
        MAP_STR_STR params2;
        Want want2 = SystemTestFormUtil::MakeWant("device", abilityName2, bundleName2, params2);
        SystemTestFormUtil::StartAbility(want2, abilityManager);
        EXPECT_EQ(SystemTestFormUtil::WaitCompleted(event, FORM_EVENT_ABILITY_ONACTIVED, 0), 0);

        std::string eventData2 = formOne;
        SystemTestFormUtil::PublishEvent(FORM_EVENT_REQ_DELETE_FORM_0600, EVENT_CODE_600, eventData2);
        EXPECT_EQ(0, SystemTestFormUtil::WaitCompleted(event, FORM_EVENT_RECV_DELETE_FORM_0600, EVENT_CODE_600));
        std::string formTwo = SystemTestFormUtil::GetData(event, FORM_EVENT_RECV_DELETE_FORM_0600, EVENT_CODE_600);
        bool result = formTwo == "true";
        EXPECT_TRUE(result);
        if (!result) {
            GTEST_LOG_(INFO) << "FMS_deleteForm_0600,  result:" << result;
        }
        // can't receive onDelete
        EXPECT_EQ(-1, SystemTestFormUtil::WaitCompleted(event, COMMON_EVENT_ON_DELETE,
            FORM_EVENT_TRIGGER_RESULT::FORM_EVENT_TRIGGER_RESULT_OK));

        bool cacheRes = FmsGetCacheInfoByFormId(atoll(formOne.c_str()));
        bool hostRes = FmsGetHostInfoByFormId(atoll(formOne.c_str()), "com.ohos.form.manager.normal");
        bool storageRes = FmsGetStorageFormInfos(atoll(formOne.c_str()));
        EXPECT_TRUE(cacheRes);
        EXPECT_FALSE(hostRes);
        EXPECT_TRUE(storageRes);
        SystemTestFormUtil::PublishEvent(FORM_EVENT_REQ_ONE_NORMAL_FORM_DEL, EVENT_CODE_101, formOne);
        EXPECT_EQ(0, SystemTestFormUtil::WaitCompleted(event, FORM_EVENT_RECV_ONE_NORMAL_FORM_DEL, EVENT_CODE_101));
        EXPECT_EQ(0, SystemTestFormUtil::WaitCompleted(event, COMMON_EVENT_ON_DELETE,
            FORM_EVENT_TRIGGER_RESULT::FORM_EVENT_TRIGGER_RESULT_OK));
    }
    SystemTestFormUtil::CleanMsg(event);
    std::cout << "============END FMS_deleteForm_0600" << std::endl;
}

/**
 * @tc.number: FMS_deleteForm_0700
 * @tc.name: After deleting a normal form, the form reference is 0.
 *     After deleting a form, there are still created normal forms in FMS.
 * @tc.desc: 1.host A an host B create one normal card respectively
 *           2.host A delete the form and verify the result is true
 */
HWTEST_F(FmsDeleteFormTest, FMS_deleteForm_0700, Function | MediumTest | Level1)
{
    std::cout << "============START FMS_deleteForm_0700" << std::endl;
    std::string bundleName = "com.ohos.form.manager.commona";
    std::string abilityName = "FormAbilityCommonA";
    MAP_STR_STR params;
    Want want = SystemTestFormUtil::MakeWant("device", abilityName, bundleName, params);
    SystemTestFormUtil::StartAbility(want, abilityManager);
    EXPECT_EQ(SystemTestFormUtil::WaitCompleted(event, FORM_EVENT_ABILITY_ONACTIVED, 0), 0);

    std::string eventData = "false"; // normal form
    SystemTestFormUtil::PublishEvent(FORM_EVENT_REQ_ONE_NORMAL_FORM, EVENT_CODE_100, eventData);
    EXPECT_EQ(0, SystemTestFormUtil::WaitCompleted(event, FORM_EVENT_RECV_ONE_NORMAL_FORM, EVENT_CODE_100));
    std::string formOne = SystemTestFormUtil::GetData(event, FORM_EVENT_RECV_ONE_NORMAL_FORM, EVENT_CODE_100);
    GTEST_LOG_(INFO) << "FMS_deleteForm_0700,  formOne:[" << formOne << "]" << std::endl;
    EXPECT_TRUE(formOne != "");
    if (formOne != "") {
        std::string bundleName2 = "com.ohos.form.manager.normal";
        std::string abilityName2 = "FormAbilityDeleteForm";
        MAP_STR_STR params2;
        Want want2 = SystemTestFormUtil::MakeWant("device", abilityName2, bundleName2, params2);
        SystemTestFormUtil::StartAbility(want2, abilityManager);
        EXPECT_EQ(SystemTestFormUtil::WaitCompleted(event, FORM_EVENT_ABILITY_ONACTIVED, 0), 0);

        std::string eventData2 = formOne;
        SystemTestFormUtil::PublishEvent(FORM_EVENT_REQ_DELETE_FORM_0700, EVENT_CODE_700, eventData2);
        EXPECT_EQ(0, SystemTestFormUtil::WaitCompleted(event, FORM_EVENT_RECV_DELETE_FORM_0700, EVENT_CODE_700));
        std::string formTwo = SystemTestFormUtil::GetData(event, FORM_EVENT_RECV_DELETE_FORM_0700, EVENT_CODE_700);
        GTEST_LOG_(INFO) << "FMS_deleteForm_0700,  formTwo:[" << formTwo << "]" << std::endl;
        bool result = (formTwo != "false" && formTwo != "");
        EXPECT_TRUE(result);
        if (!result) {
            GTEST_LOG_(INFO) << "FMS_deleteForm_0700,  result:" << result;
        }
        // can receive onDelete
        EXPECT_EQ(0, SystemTestFormUtil::WaitCompleted(event, COMMON_EVENT_ON_DELETE,
            FORM_EVENT_TRIGGER_RESULT::FORM_EVENT_TRIGGER_RESULT_OK));
        std::string onDeleteData = SystemTestFormUtil::GetData(event, COMMON_EVENT_ON_DELETE,
            FORM_EVENT_TRIGGER_RESULT::FORM_EVENT_TRIGGER_RESULT_OK);
        EXPECT_TRUE(onDeleteData == formTwo);
        bool cacheRes = FmsGetCacheInfoByFormId(atoll(formTwo.c_str()));
        bool storageRes = FmsGetStorageFormInfos(atoll(formTwo.c_str()));
        bool timerRes = FmsGetFormTimerTask(atoll(formTwo.c_str()));
        EXPECT_FALSE(cacheRes);
        EXPECT_FALSE(storageRes);
        EXPECT_FALSE(timerRes);
        SystemTestFormUtil::PublishEvent(FORM_EVENT_REQ_ONE_NORMAL_FORM_DEL, EVENT_CODE_101, formOne);
        EXPECT_EQ(0, SystemTestFormUtil::WaitCompleted(event, FORM_EVENT_RECV_ONE_NORMAL_FORM_DEL, EVENT_CODE_101));
        EXPECT_EQ(0, SystemTestFormUtil::WaitCompleted(event, COMMON_EVENT_ON_DELETE,
            FORM_EVENT_TRIGGER_RESULT::FORM_EVENT_TRIGGER_RESULT_OK));
    }
    SystemTestFormUtil::CleanMsg(event);
    std::cout << "============END FMS_deleteForm_0700" << std::endl;
}

/**
 * @tc.number: FMS_deleteForm_0800
 * @tc.name: After deleting a normal form, there is no created normal form in FMS.
 * @tc.desc: 1.host create one normal card
 *           2.host delete the form and verify the result is true
 */
HWTEST_F(FmsDeleteFormTest, FMS_deleteForm_0800, Function | MediumTest | Level1)
{
    std::cout << "============START FMS_deleteForm_0800" << std::endl;

    std::string bundleName = "com.ohos.form.manager.normal";
    std::string abilityName = "FormAbilityDeleteForm";
    MAP_STR_STR params;
    Want want = SystemTestFormUtil::MakeWant("device", abilityName, bundleName, params);
    SystemTestFormUtil::StartAbility(want, abilityManager);
    EXPECT_EQ(SystemTestFormUtil::WaitCompleted(event, FORM_EVENT_ABILITY_ONACTIVED, 0), 0);

    std::string eventData = FORM_EVENT_REQ_DELETE_FORM_0800;
    SystemTestFormUtil::PublishEvent(FORM_EVENT_REQ_DELETE_FORM_0800, EVENT_CODE_800, eventData);
    EXPECT_EQ(0, SystemTestFormUtil::WaitCompleted(event, FORM_EVENT_RECV_DELETE_FORM_0800, EVENT_CODE_800));
    std::string data = SystemTestFormUtil::GetData(event, FORM_EVENT_RECV_DELETE_FORM_0800, EVENT_CODE_800);
    bool result = (data != "false" && data != "");
    EXPECT_TRUE(result);
    if (!result) {
        GTEST_LOG_(INFO) << "FMS_deleteForm_0800,  result:" << result;
    }
    // can receive onDelete
    EXPECT_EQ(0, SystemTestFormUtil::WaitCompleted(event, COMMON_EVENT_ON_DELETE,
        FORM_EVENT_TRIGGER_RESULT::FORM_EVENT_TRIGGER_RESULT_OK));
    std::string onDeleteData = SystemTestFormUtil::GetData(event, COMMON_EVENT_ON_DELETE,
        FORM_EVENT_TRIGGER_RESULT::FORM_EVENT_TRIGGER_RESULT_OK);
    EXPECT_TRUE(onDeleteData == data);
    bool cacheRes = FmsGetCacheInfoByFormId(atoll(data.c_str()));
    bool storageRes = FmsGetStorageFormInfos(atoll(data.c_str()));
    bool timerRes = FmsGetFormTimerTask(atoll(data.c_str()));
    EXPECT_FALSE(cacheRes);
    EXPECT_FALSE(storageRes);
    EXPECT_FALSE(timerRes);
    SystemTestFormUtil::CleanMsg(event);
    std::cout << "============END FMS_deleteForm_0800" << std::endl;
}

/**
 * @tc.number: FMS_deleteForm_0900
 * @tc.name: Form ID error (FormID does not exist)
 * @tc.desc: 1.host create one normal card
 *           2.host delete the form
 *           3.host delete the form again and verify the result is false
 */
HWTEST_F(FmsDeleteFormTest, FMS_deleteForm_0900, Function | MediumTest | Level1)
{
    std::cout << "============START FMS_deleteForm_0900" << std::endl;

    std::string bundleName = "com.ohos.form.manager.normal";
    std::string abilityName = "FormAbilityDeleteForm";
    MAP_STR_STR params;
    Want want = SystemTestFormUtil::MakeWant("device", abilityName, bundleName, params);
    SystemTestFormUtil::StartAbility(want, abilityManager);
    EXPECT_EQ(SystemTestFormUtil::WaitCompleted(event, FORM_EVENT_ABILITY_ONACTIVED, 0), 0);

    std::string eventData = FORM_EVENT_REQ_DELETE_FORM_0900;
    SystemTestFormUtil::PublishEvent(FORM_EVENT_REQ_DELETE_FORM_0900, EVENT_CODE_900, eventData);
    EXPECT_EQ(0, SystemTestFormUtil::WaitCompleted(event, FORM_EVENT_RECV_DELETE_FORM_0900, EVENT_CODE_900));
    std::string data = SystemTestFormUtil::GetData(event, FORM_EVENT_RECV_DELETE_FORM_0900, EVENT_CODE_900);
    bool result = data == "false";
    EXPECT_TRUE(result);
    if (!result) {
        GTEST_LOG_(INFO) << "FMS_deleteForm_0900,  result:" << result;
    }
    // can receive onDelete
    EXPECT_EQ(0, SystemTestFormUtil::WaitCompleted(event, COMMON_EVENT_ON_DELETE,
        FORM_EVENT_TRIGGER_RESULT::FORM_EVENT_TRIGGER_RESULT_OK));
    SystemTestFormUtil::CleanMsg(event);

    std::cout << "============END FMS_deleteForm_0900" << std::endl;
}

/**
 * @tc.number: FMS_deleteForm_1000
 * @tc.name: You can delete a card after releasing it
 * @tc.desc: 1.host create one normal card
 *           2.host release the form
 *           3.host delete the form and verify the result is true
 */
HWTEST_F(FmsDeleteFormTest, FMS_deleteForm_1000, Function | MediumTest | Level1)
{
    std::cout << "============START FMS_deleteForm_1000" << std::endl;

    std::string bundleName = "com.ohos.form.manager.normal";
    std::string abilityName = "FormAbilityDeleteForm";
    MAP_STR_STR params;
    Want want = SystemTestFormUtil::MakeWant("device", abilityName, bundleName, params);
    SystemTestFormUtil::StartAbility(want, abilityManager);
    EXPECT_EQ(SystemTestFormUtil::WaitCompleted(event, FORM_EVENT_ABILITY_ONACTIVED, 0), 0);

    std::string eventData = FORM_EVENT_REQ_DELETE_FORM_1000;
    SystemTestFormUtil::PublishEvent(FORM_EVENT_REQ_DELETE_FORM_1000, EVENT_CODE_1000, eventData);
    EXPECT_EQ(0, SystemTestFormUtil::WaitCompleted(event, FORM_EVENT_RECV_DELETE_FORM_1000, EVENT_CODE_1000));
    std::string data = SystemTestFormUtil::GetData(event, FORM_EVENT_RECV_DELETE_FORM_1000, EVENT_CODE_1000);
    bool result = data == "true";
    EXPECT_TRUE(result);
    if (!result) {
        GTEST_LOG_(INFO) << "FMS_deleteForm_1000,  result:" << result;
    }
    // can receive onDelete
    EXPECT_EQ(0, SystemTestFormUtil::WaitCompleted(event, COMMON_EVENT_ON_DELETE,
        FORM_EVENT_TRIGGER_RESULT::FORM_EVENT_TRIGGER_RESULT_OK));
    SystemTestFormUtil::CleanMsg(event);

    std::cout << "============END FMS_deleteForm_1000" << std::endl;
}

/**
 * @tc.number: FMS_deleteForm_1100
 * @tc.name: When deleting a temporary form, cache data is deleted.
 * @tc.desc: 1.host a and b create one temporary card respectively
 *           2.host a delete the form and verify the result is true
 */
HWTEST_F(FmsDeleteFormTest, FMS_deleteForm_1100, Function | MediumTest | Level1)
{
    std::cout << "============START FMS_deleteForm_1100" << std::endl;

    std::string bundleName = "com.ohos.form.manager.commona";
    std::string abilityName = "FormAbilityCommonA";
    MAP_STR_STR params;
    Want want = SystemTestFormUtil::MakeWant("device", abilityName, bundleName, params);
    SystemTestFormUtil::StartAbility(want, abilityManager);
    EXPECT_EQ(SystemTestFormUtil::WaitCompleted(event, FORM_EVENT_ABILITY_ONACTIVED, 0), 0);

    std::string eventData = "true"; // temp form
    SystemTestFormUtil::PublishEvent(FORM_EVENT_REQ_ONE_NORMAL_FORM, EVENT_CODE_100, eventData);
    EXPECT_EQ(0, SystemTestFormUtil::WaitCompleted(event, FORM_EVENT_RECV_ONE_NORMAL_FORM, EVENT_CODE_100));
    std::string formOne = SystemTestFormUtil::GetData(event, FORM_EVENT_RECV_ONE_NORMAL_FORM, EVENT_CODE_100);
    
    GTEST_LOG_(INFO) << "FMS_deleteForm_1100,  formOne:[" << formOne << "]" << std::endl;
    if (formOne != "") {
        std::string bundleName2 = "com.ohos.form.manager.normal";
        std::string abilityName2 = "FormAbilityDeleteForm";
        MAP_STR_STR params2;
        Want want2 = SystemTestFormUtil::MakeWant("device", abilityName2, bundleName2, params2);
        SystemTestFormUtil::StartAbility(want2, abilityManager);
        EXPECT_EQ(SystemTestFormUtil::WaitCompleted(event, FORM_EVENT_ABILITY_ONACTIVED, 0), 0);

        std::string eventData2 = formOne;
        SystemTestFormUtil::PublishEvent(FORM_EVENT_REQ_DELETE_FORM_1100, EVENT_CODE_1100, eventData2);
        EXPECT_EQ(0, SystemTestFormUtil::WaitCompleted(event, FORM_EVENT_RECV_DELETE_FORM_1100, EVENT_CODE_1100));
        std::string formTwo = SystemTestFormUtil::GetData(event, FORM_EVENT_RECV_DELETE_FORM_1100, EVENT_CODE_1100);
        GTEST_LOG_(INFO) << "FMS_deleteForm_1100,  formTwo:[" << formTwo << "]" << std::endl;
        bool result = (formTwo != "false" && formTwo != "");
        EXPECT_TRUE(result);
        if (!result) {
            GTEST_LOG_(INFO) << "FMS_deleteForm_1100,  result:" << result;
        }
        // can receive onDelete
        EXPECT_EQ(0, SystemTestFormUtil::WaitCompleted(event, COMMON_EVENT_ON_DELETE,
            FORM_EVENT_TRIGGER_RESULT::FORM_EVENT_TRIGGER_RESULT_OK));
        std::string onDeleteData = SystemTestFormUtil::GetData(event, COMMON_EVENT_ON_DELETE,
            FORM_EVENT_TRIGGER_RESULT::FORM_EVENT_TRIGGER_RESULT_OK);
        GTEST_LOG_(INFO) << "00000,  onDeleteData:[" << onDeleteData << "]" << std::endl;
        EXPECT_TRUE(onDeleteData == formTwo);

        bool cacheRes = FmsGetCacheInfoByFormId(atoll(formTwo.c_str()));
        EXPECT_FALSE(cacheRes);
        bool cacheRes2 = FmsGetCacheInfoByFormId(atoll(formOne.c_str()));
        EXPECT_TRUE(cacheRes2);
        SystemTestFormUtil::PublishEvent(FORM_EVENT_REQ_ONE_NORMAL_FORM_DEL, EVENT_CODE_101, formOne);
        EXPECT_EQ(0, SystemTestFormUtil::WaitCompleted(event, FORM_EVENT_RECV_ONE_NORMAL_FORM_DEL, EVENT_CODE_101));
        EXPECT_EQ(0, SystemTestFormUtil::WaitCompleted(event, COMMON_EVENT_ON_DELETE,
            FORM_EVENT_TRIGGER_RESULT::FORM_EVENT_TRIGGER_RESULT_OK));
    }
    SystemTestFormUtil::CleanMsg(event);

    std::cout << "============END FMS_deleteForm_1100" << std::endl;
}

/**
 * @tc.number: FMS_deleteForm_1200
 * @tc.name: single host (using a single thread) continuously deletes multiple normal and temporary forms
 * @tc.desc: 1.There are 5 normal forms and 5 temporary forms created by the single host
 *           2.single host (using a single thread) deletes 10 deletes successfully
 */
HWTEST_F(FmsDeleteFormTest, FMS_deleteForm_1200, Function | MediumTest | Level2)
{
    int createFormCount = 10;
    std::string formIds[10] = {""};
    std::cout << "============START FMS_deleteForm_1200" << std::endl;
    std::string bundleName = "com.ohos.form.manager.normal";
    std::string abilityName = "FormAbilityDeleteForm";
    MAP_STR_STR params;

    for (int i=0; i<createFormCount; i++) {
        Want want = SystemTestFormUtil::MakeWant("device", abilityName, bundleName, params);
        SystemTestFormUtil::StartAbility(want, abilityManager);
        EXPECT_EQ(SystemTestFormUtil::WaitCompleted(event, FORM_EVENT_ABILITY_ONACTIVED, 0), 0);
        std::string eventData = (i/5 == 1 ? "true" : "false");
        SystemTestFormUtil::PublishEvent(FORM_EVENT_REQ_DELETE_FORM_1200, EVENT_CODE_1200, eventData);
        EXPECT_EQ(0, SystemTestFormUtil::WaitCompleted(event, FORM_EVENT_RECV_DELETE_FORM_1200, EVENT_CODE_1200));
        formIds[i] = SystemTestFormUtil::GetData(event, FORM_EVENT_RECV_DELETE_FORM_1200, EVENT_CODE_1200);
        GTEST_LOG_(INFO) << "========FMS_deleteForm_1200 [#Create#]formId:[" << formIds[i] << "]======>>index:" << i;
        bool result = (formIds[i] != "false" && formIds[i] != "");
        EXPECT_TRUE(result);
        if (!result) {
            GTEST_LOG_(INFO) << "FMS_deleteForm_1200,  result:" << result;
        }
    }
    for (int i=0; i<createFormCount; i++) {
        Want want = SystemTestFormUtil::MakeWant("device", abilityName, bundleName, params);
        SystemTestFormUtil::StartAbility(want, abilityManager);
        EXPECT_EQ(SystemTestFormUtil::WaitCompleted(event, FORM_EVENT_ABILITY_ONACTIVED, 0), 0);
        std::string eventData = formIds[i];
        SystemTestFormUtil::PublishEvent(FORM_EVENT_REQ_DELETE_FORM_1201, EVENT_CODE_1201, eventData);
        EXPECT_EQ(0, SystemTestFormUtil::WaitCompleted(event, FORM_EVENT_RECV_DELETE_FORM_1201, EVENT_CODE_1201));
        std::string data = SystemTestFormUtil::GetData(event, FORM_EVENT_RECV_DELETE_FORM_1201, EVENT_CODE_1201);
        GTEST_LOG_(INFO) << "========FMS_deleteForm_1200 [#Delete#]formId:[" << formIds[i] << "]======>>index:" << i;
        bool result = data == "true";
        EXPECT_TRUE(result);
        if (!result) {
            GTEST_LOG_(INFO) << "FMS_deleteForm_1201,  result:" << result;
        }
    }
    SystemTestFormUtil::CleanMsg(event);
    std::cout << "============END FMS_deleteForm_1200" << std::endl;
}

/**
 * @tc.number: FMS_deleteForm_1400
 * @tc.name: Multiple hosts delete normal and temporary forms respectively
 * @tc.desc: 1.host A and B have 5 normal forms and 5 temporary forms respectively.
 *           2.host A and B delete their forms successively.
 */
HWTEST_F(FmsDeleteFormTest, FMS_deleteForm_1400, Function | MediumTest | Level2)
{
    std::cout << "============START FMS_deleteForm_1400" << std::endl;
    FMS_deleteForm_1400_A();
    FMS_deleteForm_1400_B();
    SystemTestFormUtil::CleanMsg(event);
    std::cout << "============END FMS_deleteForm_1400" << std::endl;
}

/**
 * @tc.number: FMS_deleteForm_1500
 * @tc.name: create 256 temporary forms and delete one
 * @tc.desc: 1.create 256 temporary forms and delete one
 *           2.create one temporary form successfully .
 */
HWTEST_F(FmsDeleteFormTest, FMS_deleteForm_1500, Function | MediumTest | Level2)
{
    std::cout << "============START FMS_deleteForm_1500" << std::endl;
    std::string bundleName = "com.ohos.form.manager.normal";
    std::string abilityName = "FormAbilityDeleteForm";
    int count = Constants::MAX_RECORD_PER_APP - 1;
    FMS_acquireTempFormBatch(bundleName, abilityName, count);
    std::string formId = "";
    FMS_deleteForm_1500_acquireForm(bundleName, abilityName, formId);
    FMS_deleteForm_1500_clearForm(bundleName, abilityName, formId);
    SystemTestFormUtil::CleanMsg(event);
    std::cout << "============END FMS_deleteForm_1500" << std::endl;
}

/**
 * @tc.number: FMS_deleteForm_1600
 * @tc.name: create 256 normal forms and delete one
 * @tc.desc: 1.create 256 normal forms and delete one
 *           2.create one normal form successfully .
 */
HWTEST_F(FmsDeleteFormTest, FMS_deleteForm_1600, Function | MediumTest | Level2)
{
    std::cout << "============START FMS_deleteForm_1600" << std::endl;
    std::string bundleName = "com.ohos.form.manager.normal";
    std::string abilityName = "FormAbilityDeleteForm";
    int count = Constants::MAX_RECORD_PER_APP - 1;
    FMS_acquireNormalFormBatch(bundleName, abilityName, count);
    std::string formId = "";
    FMS_deleteForm_1600_acquireForm(bundleName, abilityName, formId);
    FMS_deleteForm_1600_clearForm(bundleName, abilityName, formId);
    SystemTestFormUtil::CleanMsg(event);
    std::cout << "============END FMS_deleteForm_1600" << std::endl;
}

/**
 * @tc.number: FMS_deleteForm_1700
 * @tc.name: When the number of normal forms of multiple hosts reaches 512,
 *     you can delete forms and create forms
 * @tc.desc: 1.host A create 200 normal forms
 *           2.host B create 200 normal forms
 *           3.host C create 112 normal forms
 *           4.host C delete one normal form and create one normal form successfully .
 */
HWTEST_F(FmsDeleteFormTest, FMS_deleteForm_1700, Function | MediumTest | Level2)
{
    std::cout << "============START FMS_deleteForm_1700" << std::endl;
    std::string bundleNameA = "com.ohos.form.manager.commona";
    std::string abilityNameA = "FormAbilityCommonA";
    int countA = 200;
    FMS_acquireNormalFormBatch(bundleNameA, abilityNameA, countA);

    std::string bundleNameB = "com.ohos.form.manager.commonb";
    std::string abilityNameB = "FormAbilityCommonB";
    int countB = 200;
    FMS_acquireNormalFormBatch(bundleNameB, abilityNameB, countB);

    std::string bundleNameC = "com.ohos.form.manager.normal";
    std::string abilityNameC = "FormAbilityDeleteForm";
    int countC = 111;
    FMS_acquireNormalFormBatch(bundleNameC, abilityNameC, countC);

    std::string formId = "";
    FMS_deleteForm_1700_acquireForm(bundleNameC, abilityNameC, formId);
    FMS_deleteForm_1700_clearForm(bundleNameC, abilityNameC, formId);
    FMS_deleteFormBatch(bundleNameA, abilityNameA);
    FMS_deleteFormBatch(bundleNameB, abilityNameB);
    FMS_deleteFormBatch(bundleNameC, abilityNameC);
    SystemTestFormUtil::CleanMsg(event);
    std::cout << "============END FMS_deleteForm_1700" << std::endl;
}

bool FmsDeleteFormTest::FmsGetCacheInfoByFormId(std::int64_t formId)
{
    std::string cachedData = SystemTestFormUtil::QueryFormInfoByFormId(formId);
    bool cacheRes = false;
    std::string formIdStr = std::to_string(formId);
    std::string::size_type pos = cachedData.find(formIdStr);
    if (pos != cachedData.npos) {
        std::cout << "######### formId is found in cache" << std::endl;
        cacheRes = true;
    } else {
        std::cout << "######### Can't find formId in cache!" << std::endl;
        cacheRes = false;
    }
    return cacheRes;
}
bool FmsDeleteFormTest::FmsGetHostInfoByFormId(std::int64_t formId, std::string hostBundleName)
{
    std::string hostData = SystemTestFormUtil::QueryFormInfoByFormId(formId);
    bool hostRes = false;
    std::string::size_type pos = hostData.find(hostBundleName);
    if (pos != hostData.npos) {
        std::cout << "######### hostBundleName is found in cache" << std::endl;
        hostRes = true;
    } else {
        std::cout << "######### Can't find hostBundleName in cache!" << std::endl;
        hostRes = false;
    }
    return hostRes;
}
bool FmsDeleteFormTest::FmsGetStorageFormInfos(std::int64_t formId)
{
    std::string storageData = SystemTestFormUtil::QueryStorageFormInfos();
    bool storageRes = false;
    std::string formIdStr = std::to_string(formId);
    std::string::size_type pos = storageData.find(formIdStr);
    if (pos != storageData.npos) {
        std::cout << "######### formId is found in storage" << std::endl;
        storageRes = true;
    } else {
        std::cout << "######### Can't find formId in storage!" << std::endl;
        storageRes = false;
    }
    return storageRes;
}
bool FmsDeleteFormTest::FmsGetFormTimerTask(std::int64_t formId)
{
    std::string timerRes = SystemTestFormUtil::QueryFormTimerByFormId(formId);
    if (timerRes == "true") {
        std::cout << "######### formId is found in TimerTask" << std::endl;
        return true;
    } else {
        std::cout << "######### Can't find formId in TimerTask!" << std::endl;
        return false;
    }
}
void FmsDeleteFormTest::FMS_deleteForm_1400_A()
{
    int createFormCount = 10;
    std::string formIds[10] = {""};
    std::string bundleName = "com.ohos.form.manager.commona";
    std::string abilityName = "FormAbilityCommonA";
    MAP_STR_STR params;

    for (int i = 0; i < createFormCount; i++) {
        Want want = SystemTestFormUtil::MakeWant("device", abilityName, bundleName, params);
        SystemTestFormUtil::StartAbility(want, abilityManager);
        EXPECT_EQ(SystemTestFormUtil::WaitCompleted(event, FORM_EVENT_ABILITY_ONACTIVED, 0), 0);
        std::string eventData = (i/5 == 1 ? "true" : "false");
        SystemTestFormUtil::PublishEvent(FORM_EVENT_REQ_ONE_NORMAL_FORM, EVENT_CODE_1400, eventData);
        EXPECT_EQ(0, SystemTestFormUtil::WaitCompleted(event, FORM_EVENT_RECV_ONE_NORMAL_FORM, EVENT_CODE_100));
        formIds[i] = SystemTestFormUtil::GetData(event, FORM_EVENT_RECV_ONE_NORMAL_FORM, EVENT_CODE_100);
        GTEST_LOG_(INFO) << "========FMS_deleteForm_1400[A][#Create#]formId:[" << formIds[i] << "]======>>index:" << i;
        bool result = (formIds[i] != "false" && formIds[i] != "");
        EXPECT_TRUE(result);
        if (!result) {
            GTEST_LOG_(INFO) << "FMS_deleteForm_1400[A],  result:" << result;
        }
    }
    for (int i=0; i<createFormCount; i++) {
        Want want = SystemTestFormUtil::MakeWant("device", abilityName, bundleName, params);
        SystemTestFormUtil::StartAbility(want, abilityManager);
        EXPECT_EQ(SystemTestFormUtil::WaitCompleted(event, FORM_EVENT_ABILITY_ONACTIVED, 0), 0);
        std::string eventData = formIds[i];
        SystemTestFormUtil::PublishEvent(FORM_EVENT_REQ_ONE_NORMAL_FORM_DEL, EVENT_CODE_1400, eventData);
        EXPECT_EQ(0, SystemTestFormUtil::WaitCompleted(event, FORM_EVENT_RECV_ONE_NORMAL_FORM_DEL, EVENT_CODE_101));
        std::string data = SystemTestFormUtil::GetData(event, FORM_EVENT_RECV_ONE_NORMAL_FORM_DEL, EVENT_CODE_101);
        GTEST_LOG_(INFO) << "========FMS_deleteForm_1400[A][#Delete#]formId:[" << formIds[i] << "]======>>index:" << i;
        bool result = data == "true";
        EXPECT_TRUE(result);
        if (!result) {
            GTEST_LOG_(INFO) << "FMS_deleteForm_1401[A],  result:" << result;
        }
        EXPECT_EQ(0, SystemTestFormUtil::WaitCompleted(event, COMMON_EVENT_ON_DELETE,
            FORM_EVENT_TRIGGER_RESULT::FORM_EVENT_TRIGGER_RESULT_OK));
    }
}
void FmsDeleteFormTest::FMS_deleteForm_1400_B()
{
    int createFormCount = 10;
    std::string formIds[10] = {""};
    std::string bundleName = "com.ohos.form.manager.normal";
    std::string abilityName = "FormAbilityDeleteForm";
    MAP_STR_STR params;

    for (int i = 0; i < createFormCount; i++) {
        Want want = SystemTestFormUtil::MakeWant("device", abilityName, bundleName, params);
        SystemTestFormUtil::StartAbility(want, abilityManager);
        EXPECT_EQ(SystemTestFormUtil::WaitCompleted(event, FORM_EVENT_ABILITY_ONACTIVED, 0), 0);
        std::string eventData = (i/5 == 1 ? "true" : "false");
        SystemTestFormUtil::PublishEvent(FORM_EVENT_REQ_DELETE_FORM_1400, EVENT_CODE_1400, eventData);
        EXPECT_EQ(0, SystemTestFormUtil::WaitCompleted(event, FORM_EVENT_RECV_DELETE_FORM_1400, EVENT_CODE_1400));
        formIds[i] = SystemTestFormUtil::GetData(event, FORM_EVENT_RECV_DELETE_FORM_1400, EVENT_CODE_1400);
        GTEST_LOG_(INFO) << "========FMS_deleteForm_1400[B][#Create#]formId:[" << formIds[i] << "]======>>index:" << i;
        bool result = (formIds[i] != "false" && formIds[i] != "");
        EXPECT_TRUE(result);
        if (!result) {
            GTEST_LOG_(INFO) << "FMS_deleteForm_1400[B],  result:" << result;
        }
    }
    for (int i = 0; i < createFormCount; i++) {
        Want want = SystemTestFormUtil::MakeWant("device", abilityName, bundleName, params);
        SystemTestFormUtil::StartAbility(want, abilityManager);
        EXPECT_EQ(SystemTestFormUtil::WaitCompleted(event, FORM_EVENT_ABILITY_ONACTIVED, 0), 0);
        std::string eventData = formIds[i];
        SystemTestFormUtil::PublishEvent(FORM_EVENT_REQ_DELETE_FORM_1401, EVENT_CODE_1401, eventData);
        EXPECT_EQ(0, SystemTestFormUtil::WaitCompleted(event, FORM_EVENT_RECV_DELETE_FORM_1401, EVENT_CODE_1401));
        std::string data = SystemTestFormUtil::GetData(event, FORM_EVENT_RECV_DELETE_FORM_1401, EVENT_CODE_1401);
        GTEST_LOG_(INFO) << "========FMS_deleteForm_1400[B][#Delete#]formId:[" << formIds[i] << "]======>>index:" << i;
        bool result = data == "true";
        EXPECT_TRUE(result);
        if (!result) {
            GTEST_LOG_(INFO) << "FMS_deleteForm_1401[B],  result:" << result;
        }
        EXPECT_EQ(0, SystemTestFormUtil::WaitCompleted(event, COMMON_EVENT_ON_DELETE,
            FORM_EVENT_TRIGGER_RESULT::FORM_EVENT_TRIGGER_RESULT_OK));
    }
}
void FmsDeleteFormTest::FMS_deleteForm_1500_Create_Delete(std::string delFormId, std::string &createFormId)
{
    std::string bundleName = "com.ohos.form.manager.normal";
    std::string abilityName = "FormAbilityDeleteForm";
    MAP_STR_STR params;
    Want want = SystemTestFormUtil::MakeWant("device", abilityName, bundleName, params);
    SystemTestFormUtil::StartAbility(want, abilityManager);
    EXPECT_EQ(SystemTestFormUtil::WaitCompleted(event, FORM_EVENT_ABILITY_ONACTIVED, 0), 0);

    // delete one and create one
    SystemTestFormUtil::PublishEvent(FORM_EVENT_REQ_DELETE_FORM_1501, EVENT_CODE_1510, delFormId);
    EXPECT_EQ(0, SystemTestFormUtil::WaitCompleted(event, FORM_EVENT_RECV_DELETE_FORM_1501, EVENT_CODE_1510));
    createFormId = SystemTestFormUtil::GetData(event, FORM_EVENT_RECV_DELETE_FORM_1501, EVENT_CODE_1510);
    bool result = (createFormId != "deleteError" && createFormId != "acquireError" && createFormId != "");
    EXPECT_TRUE(result);
    if (!result) {
        GTEST_LOG_(INFO) << "FMS_deleteForm_1501,  result:" << result;
    }
}
void FmsDeleteFormTest::FMS_deleteForm_1600_Delete_Create(std::string delFormId, std::string &createFormId)
{
    std::string bundleName = "com.ohos.form.manager.normal";
    std::string abilityName = "FormAbilityDeleteForm";
    MAP_STR_STR params;
    Want want = SystemTestFormUtil::MakeWant("device", abilityName, bundleName, params);
    SystemTestFormUtil::StartAbility(want, abilityManager);
    EXPECT_EQ(SystemTestFormUtil::WaitCompleted(event, FORM_EVENT_ABILITY_ONACTIVED, 0), 0);

    // delete one and create one
    SystemTestFormUtil::PublishEvent(FORM_EVENT_REQ_DELETE_FORM_1601, EVENT_CODE_1610, delFormId);
    EXPECT_EQ(0, SystemTestFormUtil::WaitCompleted(event, FORM_EVENT_RECV_DELETE_FORM_1601, EVENT_CODE_1610));
    createFormId = SystemTestFormUtil::GetData(event, FORM_EVENT_RECV_DELETE_FORM_1601, EVENT_CODE_1610);
    bool result = (createFormId != "deleteError" && createFormId != "acquireError" && createFormId != "");
    EXPECT_TRUE(result);
    if (!result) {
        GTEST_LOG_(INFO) << "FMS_deleteForm_1601,  result:" << result;
    }
}
void FmsDeleteFormTest::FMS_deleteForm_1700_Delete_Create(std::string delFormId, std::string &createFormId)
{
    std::string bundleName = "com.ohos.form.manager.normal";
    std::string abilityName = "FormAbilityDeleteForm";
    MAP_STR_STR params;
    Want want = SystemTestFormUtil::MakeWant("device", abilityName, bundleName, params);
    SystemTestFormUtil::StartAbility(want, abilityManager);
    EXPECT_EQ(SystemTestFormUtil::WaitCompleted(event, FORM_EVENT_ABILITY_ONACTIVED, 0), 0);

    // delete one and create one
    SystemTestFormUtil::PublishEvent(FORM_EVENT_REQ_DELETE_FORM_1701, EVENT_CODE_1701, delFormId);
    EXPECT_EQ(0, SystemTestFormUtil::WaitCompleted(event, FORM_EVENT_RECV_DELETE_FORM_1701, EVENT_CODE_1701));
    createFormId = SystemTestFormUtil::GetData(event, FORM_EVENT_RECV_DELETE_FORM_1701, EVENT_CODE_1701);
    bool result = (createFormId != "deleteError" && createFormId != "acquireError" && createFormId != "");
    EXPECT_TRUE(result);
    if (!result) {
        GTEST_LOG_(INFO) << "FMS_deleteForm_1701,  result:" << result;
    }
}
void FmsDeleteFormTest::FMS_deleteForm_1500_acquireForm(const std::string &bundleName, const std::string &abilityName,
    std::string &outformId)
{
    MAP_STR_STR params;
    Want want = SystemTestFormUtil::MakeWant("device", abilityName, bundleName, params);
    SystemTestFormUtil::StartAbility(want, abilityManager);
    EXPECT_EQ(SystemTestFormUtil::WaitCompleted(event, FORM_EVENT_ABILITY_ONACTIVED, 0), 0);
    std::string eventData = FORM_EVENT_REQ_DELETE_FORM_1500;
    SystemTestFormUtil::PublishEvent(FORM_EVENT_REQ_DELETE_FORM_1500, EVENT_CODE_1500, eventData);
    EXPECT_EQ(0, SystemTestFormUtil::WaitCompleted(event, FORM_EVENT_RECV_DELETE_FORM_1500, EVENT_CODE_1500));
    std::string formId = SystemTestFormUtil::GetData(event, FORM_EVENT_RECV_DELETE_FORM_1500, EVENT_CODE_1500);
    GTEST_LOG_(INFO) << "========FMS_deleteForm_1500 [#Create#]formId:[" << formId << "]";
    bool result = (formId != "false" && formId != "");
    EXPECT_TRUE(result);
    if (!result) {
        GTEST_LOG_(INFO) << "FMS_deleteForm_1500,  result:" << result;
    }
    FMS_deleteForm_1500_Create_Delete(formId, outformId);
}
void FmsDeleteFormTest::FMS_deleteForm_1500_clearForm(const std::string &bundleName, const std::string &abilityName,
    std::string &outformId)
{
    MAP_STR_STR params;
    Want want = SystemTestFormUtil::MakeWant("device", abilityName, bundleName, params);
    SystemTestFormUtil::StartAbility(want, abilityManager);
    EXPECT_EQ(SystemTestFormUtil::WaitCompleted(event, FORM_EVENT_ABILITY_ONACTIVED, 0), 0);
    std::string eventData = outformId;
    SystemTestFormUtil::PublishEvent(FORM_EVENT_REQ_DELETE_FORM_1502, EVENT_CODE_1511, eventData);
    EXPECT_EQ(0, SystemTestFormUtil::WaitCompleted(event, FORM_EVENT_RECV_DELETE_FORM_1502, EVENT_CODE_1511));
    std::string data = SystemTestFormUtil::GetData(event, FORM_EVENT_RECV_DELETE_FORM_1502, EVENT_CODE_1511);
    GTEST_LOG_(INFO) << "========FMS_deleteForm_1500_clearForm [#Delete#]formId:[" << outformId << "]";
    bool result = data == "true";
    EXPECT_TRUE(result);
    if (!result) {
        GTEST_LOG_(INFO) << "FMS_deleteForm_1500_clearForm,  result:" << result;
    }
    FMS_deleteFormBatch(bundleName, abilityName);
}
void FmsDeleteFormTest::FMS_deleteForm_1600_acquireForm(const std::string &bundleName, const std::string &abilityName,
    std::string &outformId)
{
    MAP_STR_STR params;
    Want want = SystemTestFormUtil::MakeWant("device", abilityName, bundleName, params);
    SystemTestFormUtil::StartAbility(want, abilityManager);
    EXPECT_EQ(SystemTestFormUtil::WaitCompleted(event, FORM_EVENT_ABILITY_ONACTIVED, 0), 0);
    std::string eventData = FORM_EVENT_REQ_DELETE_FORM_1600;
    SystemTestFormUtil::PublishEvent(FORM_EVENT_REQ_DELETE_FORM_1600, EVENT_CODE_1600, eventData);
    EXPECT_EQ(0, SystemTestFormUtil::WaitCompleted(event, FORM_EVENT_RECV_DELETE_FORM_1600, EVENT_CODE_1600));
    std::string formId = SystemTestFormUtil::GetData(event, FORM_EVENT_RECV_DELETE_FORM_1600, EVENT_CODE_1600);
    GTEST_LOG_(INFO) << "========FMS_deleteForm_1600 [#Create#]formId:[" << formId << "]";
    bool result = (formId != "false" && formId != "");
    EXPECT_TRUE(result);
    if (!result) {
        GTEST_LOG_(INFO) << "FMS_deleteForm_1600,  result:" << result;
    }
    FMS_deleteForm_1600_Delete_Create(formId, outformId);
}
void FmsDeleteFormTest::FMS_deleteForm_1600_clearForm(const std::string &bundleName, const std::string &abilityName,
    std::string &outformId)
{
    MAP_STR_STR params;
    Want want = SystemTestFormUtil::MakeWant("device", abilityName, bundleName, params);
    SystemTestFormUtil::StartAbility(want, abilityManager);
    EXPECT_EQ(SystemTestFormUtil::WaitCompleted(event, FORM_EVENT_ABILITY_ONACTIVED, 0), 0);
    std::string eventData = outformId;
    SystemTestFormUtil::PublishEvent(FORM_EVENT_REQ_DELETE_FORM_1602, EVENT_CODE_1611, eventData);
    EXPECT_EQ(0, SystemTestFormUtil::WaitCompleted(event, FORM_EVENT_RECV_DELETE_FORM_1602, EVENT_CODE_1611));
    std::string data = SystemTestFormUtil::GetData(event, FORM_EVENT_RECV_DELETE_FORM_1602, EVENT_CODE_1611);
    GTEST_LOG_(INFO) << "========FMS_deleteForm_1600 [#Delete#]formId:[" << outformId << "]";
    bool result = data == "true";
    EXPECT_TRUE(result);
    if (!result) {
        GTEST_LOG_(INFO) << "FMS_deleteForm_1600,  result:" << result;
    }
    FMS_deleteFormBatch(bundleName, abilityName);
}
void FmsDeleteFormTest::FMS_deleteForm_1700_acquireForm(const std::string &bundleName, const std::string &abilityName,
    std::string &outformId)
{
    MAP_STR_STR params;
    Want want = SystemTestFormUtil::MakeWant("device", abilityName, bundleName, params);
    SystemTestFormUtil::StartAbility(want, abilityManager);
    EXPECT_EQ(SystemTestFormUtil::WaitCompleted(event, FORM_EVENT_ABILITY_ONACTIVED, 0), 0);
    std::string eventData = FORM_EVENT_REQ_DELETE_FORM_1700;
    SystemTestFormUtil::PublishEvent(FORM_EVENT_REQ_DELETE_FORM_1700, EVENT_CODE_1700, eventData);
    EXPECT_EQ(0, SystemTestFormUtil::WaitCompleted(event, FORM_EVENT_RECV_DELETE_FORM_1700, EVENT_CODE_1700));
    std::string formId = SystemTestFormUtil::GetData(event, FORM_EVENT_RECV_DELETE_FORM_1700, EVENT_CODE_1700);
    GTEST_LOG_(INFO) << "========FMS_deleteForm_1700 [#Create#]formId:[" << formId << "]";
    bool result = (formId != "false" && formId != "");
    EXPECT_TRUE(result);
    if (!result) {
        GTEST_LOG_(INFO) << "FMS_deleteForm_1700,  result:" << result;
    }
    FMS_deleteForm_1700_Delete_Create(formId, outformId);
}
void FmsDeleteFormTest::FMS_deleteForm_1700_clearForm(const std::string &bundleName, const std::string &abilityName,
    std::string &outformId)
{
    MAP_STR_STR params;
    Want want = SystemTestFormUtil::MakeWant("device", abilityName, bundleName, params);
    SystemTestFormUtil::StartAbility(want, abilityManager);
    EXPECT_EQ(SystemTestFormUtil::WaitCompleted(event, FORM_EVENT_ABILITY_ONACTIVED, 0), 0);

    std::string eventData = outformId;
    SystemTestFormUtil::PublishEvent(FORM_EVENT_REQ_DELETE_FORM_1702, EVENT_CODE_1702, eventData);
    EXPECT_EQ(0, SystemTestFormUtil::WaitCompleted(event, FORM_EVENT_RECV_DELETE_FORM_1702, EVENT_CODE_1702));
    std::string data = SystemTestFormUtil::GetData(event, FORM_EVENT_RECV_DELETE_FORM_1702, EVENT_CODE_1702);
    GTEST_LOG_(INFO) << "========FMS_deleteForm_1700 [#Delete#]formId:[" << outformId << "]";
    bool result = data == "true";
    EXPECT_TRUE(result);
    if (!result) {
        GTEST_LOG_(INFO) << "FMS_deleteForm_1700,  result:" << result;
    }
}
void FmsDeleteFormTest::FMS_acquireTempFormBatch(const std::string &bundleName, const std::string &abilityName,
    const int count)
{
    std::cout << "START FMS_acquireTempFormBatch" << std::endl;
    MAP_STR_STR params;
    Want want = SystemTestFormUtil::MakeWant("device", abilityName, bundleName, params);
    SystemTestFormUtil::StartAbility(want, abilityManager);
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
        GTEST_LOG_(INFO) << "FMS_acquireTempFormBatch, result:" << result;
    }
    std::cout << "END FMS_acquireTempFormBatch" << std::endl;
}
void FmsDeleteFormTest::FMS_acquireNormalFormBatch(const std::string &bundleName, const std::string &abilityName,
    const int count)
{
    std::cout << "START FMS_acquireNormalFormBatch" << std::endl;
    MAP_STR_STR params;
    Want want = SystemTestFormUtil::MakeWant("device", abilityName, bundleName, params);
    SystemTestFormUtil::StartAbility(want, abilityManager);
    EXPECT_EQ(SystemTestFormUtil::WaitCompleted(event, FORM_EVENT_ABILITY_ONACTIVED, 0), 0);

    std::string eventData = std::to_string(count);
    SystemTestFormUtil::PublishEvent(FORM_EVENT_REQ_ACQUIRE_FORM_BATCH, EVENT_CODE_BATCH, eventData);

    EXPECT_EQ(0, SystemTestFormUtil::WaitCompleted(event, FORM_EVENT_RECV_ACQUIRE_FORM_BATCH, EVENT_CODE_BATCH));
    std::string data = SystemTestFormUtil::GetData(event, FORM_EVENT_RECV_ACQUIRE_FORM_BATCH, EVENT_CODE_BATCH);
    bool result = data == "true";
    EXPECT_TRUE(result);
    if (result) {
        GTEST_LOG_(INFO) << "FMS_acquireNormalFormBatch, count: " << eventData;
    } else {
        GTEST_LOG_(INFO) << "FMS_acquireNormalFormBatch, result:" << result;
    }
    std::cout << "END FMS_acquireNormalFormBatch" << std::endl;
}
void FmsDeleteFormTest::FMS_deleteFormBatch(const std::string &bundleName, const std::string &abilityName)
{
    std::cout << "START FMS_deleteFormBatch, delete forms" << std::endl;
    MAP_STR_STR params;
    Want want = SystemTestFormUtil::MakeWant("device", abilityName, bundleName, params);
    SystemTestFormUtil::StartAbility(want, abilityManager);
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