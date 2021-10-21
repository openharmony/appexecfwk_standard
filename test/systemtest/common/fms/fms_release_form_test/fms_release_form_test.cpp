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
#include "iservice_registry.h"
#include "nlohmann/json.hpp"
#include "system_ability_definition.h"
#include "system_test_form_util.h"

using OHOS::AAFwk::Want;
using namespace testing::ext;
using namespace std::chrono_literals;
using namespace OHOS::STtools;

namespace OHOS {
namespace AppExecFwk {
class FmsReleaseFormTest : public testing::Test {
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
};

std::vector<std::string> FmsReleaseFormTest::eventList = {
    FORM_EVENT_ABILITY_ONACTIVED,
    FORM_EVENT_RECV_RELEASE_FORM_0100,
    FORM_EVENT_RECV_RELEASE_FORM_0200,
    FORM_EVENT_RECV_RELEASE_FORM_0300,
    FORM_EVENT_RECV_RELEASE_FORM_0400,
    FORM_EVENT_RECV_RELEASE_FORM_0500,
    FORM_EVENT_RECV_RELEASE_FORM_0600,
    FORM_EVENT_RECV_RELEASE_FORM_0700,
    FORM_EVENT_RECV_RELEASE_FORM_0800,
    FORM_EVENT_RECV_RELEASE_FORM_0900,
    FORM_EVENT_RECV_RELEASE_FORM_1000,
    FORM_EVENT_RECV_RELEASE_FORM_1100,
    FORM_EVENT_RECV_RELEASE_FORM_1200,
    FORM_EVENT_RECV_RELEASE_FORM_1300,
    FORM_EVENT_RECV_RELEASE_FORM_1400,
    FORM_EVENT_RECV_RELEASE_FORM_COMMON_DEL,
    FORM_EVENT_RECV_ONE_NORMAL_FORM,
    FORM_EVENT_RECV_ONE_NORMAL_FORM_DEL
};

FormEvent FmsReleaseFormTest::event = FormEvent();
sptr<AAFwk::IAbilityManager> FmsReleaseFormTest::abilityManager = nullptr;
std::shared_ptr<FmsReleaseFormTest::FormEventSubscriber> FmsReleaseFormTest::subscriber = nullptr;
void FmsReleaseFormTest::FormEventSubscriber::OnReceiveEvent(const CommonEventData &data)
{
    GTEST_LOG_(INFO) << "OnReceiveEvent: event=" << data.GetWant().GetAction();
    GTEST_LOG_(INFO) << "OnReceiveEvent: data=" << data.GetData();
    GTEST_LOG_(INFO) << "OnReceiveEvent: code=" << data.GetCode();
    SystemTestFormUtil::Completed(event, data.GetWant().GetAction(), data.GetCode(), data.GetData());
}

void FmsReleaseFormTest::SetUpTestCase()
{
    std::cout << "START Install============" << std::endl;
    if (!SubscribeEvent()) {
        GTEST_LOG_(INFO) << "SubscribeEvent error";
    }
}

void FmsReleaseFormTest::TearDownTestCase()
{
    std::cout << "START Uninstall============" << std::endl;
    CommonEventManager::UnSubscribeCommonEvent(subscriber);
}

void FmsReleaseFormTest::SetUp()
{
}

void FmsReleaseFormTest::TearDown()
{
    SystemTestFormUtil::CleanMsg(event);
}
bool FmsReleaseFormTest::SubscribeEvent()
{
    std::vector<std::string> eventList = {
        FORM_EVENT_ABILITY_ONACTIVED,
        FORM_EVENT_RECV_RELEASE_FORM_0100,
        FORM_EVENT_RECV_RELEASE_FORM_0200,
        FORM_EVENT_RECV_RELEASE_FORM_0300,
        FORM_EVENT_RECV_RELEASE_FORM_0400,
        FORM_EVENT_RECV_RELEASE_FORM_0500,
        FORM_EVENT_RECV_RELEASE_FORM_0600,
        FORM_EVENT_RECV_RELEASE_FORM_0700,
        FORM_EVENT_RECV_RELEASE_FORM_0800,
        FORM_EVENT_RECV_RELEASE_FORM_0900,
        FORM_EVENT_RECV_RELEASE_FORM_1000,
        FORM_EVENT_RECV_RELEASE_FORM_1100,
        FORM_EVENT_RECV_RELEASE_FORM_1200,
        FORM_EVENT_RECV_RELEASE_FORM_1300,
        FORM_EVENT_RECV_RELEASE_FORM_1400,
        FORM_EVENT_RECV_RELEASE_FORM_COMMON_DEL,
        FORM_EVENT_RECV_ONE_NORMAL_FORM,
        FORM_EVENT_RECV_ONE_NORMAL_FORM_DEL
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
 * @tc.number: FMS_releaseForm_0100
 * @tc.name: host is not a system app
 * @tc.desc: 1.release the form that formID is 1
 *           2.Verify the result is false
 */
HWTEST_F(FmsReleaseFormTest, FMS_releaseForm_0100, Function | MediumTest | Level0)
{
    std::cout << "============START FMS_releaseForm_0100" << std::endl;

    std::string bundleName = "com.ohos.form.manager.notsystemapp";
    std::string abilityName = "FormAbilityNotSys";
    MAP_STR_STR params;
    Want want = SystemTestFormUtil::MakeWant("device", abilityName, bundleName, params);
    SystemTestFormUtil::StartAbility(want, abilityManager);
    EXPECT_EQ(SystemTestFormUtil::WaitCompleted(event, FORM_EVENT_ABILITY_ONACTIVED, 0), 0);

    std::string eventData = FORM_EVENT_REQ_RELEASE_FORM_0100;
    SystemTestFormUtil::PublishEvent(FORM_EVENT_REQ_RELEASE_FORM_0100, EVENT_CODE_100, eventData);
    EXPECT_EQ(0, SystemTestFormUtil::WaitCompleted(event, FORM_EVENT_RECV_RELEASE_FORM_0100, EVENT_CODE_100));
    std::string data = SystemTestFormUtil::GetData(event, FORM_EVENT_RECV_RELEASE_FORM_0100, EVENT_CODE_100);
    bool result = data == "false";
    EXPECT_TRUE(result);
    if (!result) {
        GTEST_LOG_(INFO) << "FMS_releaseForm_0100,  result:" << result;
    }

    std::cout << "============END FMS_releaseForm_0100" << std::endl;
}

/**
 * @tc.number: FMS_releaseForm_0200
 * @tc.name: host does not have permission
 * @tc.desc: 1.release the form that formID is 1
 *           2.Verify the result is false
 */
HWTEST_F(FmsReleaseFormTest, FMS_releaseForm_0200, Function | MediumTest | Level0)
{
    std::cout << "============START FMS_releaseForm_0200" << std::endl;

    std::string bundleName = "com.ohos.form.manager.nopermission";
    std::string abilityName = "FormAbilityNoPerm";
    MAP_STR_STR params;
    Want want = SystemTestFormUtil::MakeWant("device", abilityName, bundleName, params);
    SystemTestFormUtil::StartAbility(want, abilityManager);
    EXPECT_EQ(SystemTestFormUtil::WaitCompleted(event, FORM_EVENT_ABILITY_ONACTIVED, 0), 0);

    std::string eventData = FORM_EVENT_REQ_RELEASE_FORM_0200;
    SystemTestFormUtil::PublishEvent(FORM_EVENT_REQ_RELEASE_FORM_0200, EVENT_CODE_200, eventData);
    EXPECT_EQ(0, SystemTestFormUtil::WaitCompleted(event, FORM_EVENT_RECV_RELEASE_FORM_0200, EVENT_CODE_200));
    std::string data = SystemTestFormUtil::GetData(event, FORM_EVENT_RECV_RELEASE_FORM_0200, EVENT_CODE_200);
    bool result = data == "false";
    EXPECT_TRUE(result);
    if (!result) {
        GTEST_LOG_(INFO) << "FMS_releaseForm_0200,  result:" << result;
    }

    std::cout << "============END FMS_releaseForm_0200" << std::endl;
}

/**
 * @tc.number: FMS_releaseForm_0300
 * @tc.name: formID id is error(formID < 0)
 * @tc.desc: 1.release the form
 *           2.Verify the result is false
 */
HWTEST_F(FmsReleaseFormTest, FMS_releaseForm_0300, Function | MediumTest | Level0)
{
    std::cout << "============START FMS_releaseForm_0300" << std::endl;

    std::string bundleName = "com.ohos.form.manager.normal";
    std::string abilityName = "FormAbilityReleaseForm";
    MAP_STR_STR params;
    Want want = SystemTestFormUtil::MakeWant("device", abilityName, bundleName, params);
    SystemTestFormUtil::StartAbility(want, abilityManager);
    EXPECT_EQ(SystemTestFormUtil::WaitCompleted(event, FORM_EVENT_ABILITY_ONACTIVED, 0), 0);

    std::string eventData = FORM_EVENT_REQ_RELEASE_FORM_0300;
    SystemTestFormUtil::PublishEvent(FORM_EVENT_REQ_RELEASE_FORM_0300, EVENT_CODE_300, eventData);
    EXPECT_EQ(0, SystemTestFormUtil::WaitCompleted(event, FORM_EVENT_RECV_RELEASE_FORM_0300, EVENT_CODE_300));
    std::string data = SystemTestFormUtil::GetData(event, FORM_EVENT_RECV_RELEASE_FORM_0300, EVENT_CODE_300);
    bool result = data == "false";
    EXPECT_TRUE(result);
    if (!result) {
        GTEST_LOG_(INFO) << "FMS_releaseForm_0300,  result:" << result;
    }
    SystemTestFormUtil::CleanMsg(event);
    std::cout << "============END FMS_releaseForm_0300" << std::endl;
}

/**
 * @tc.number: FMS_releaseForm_0400
 * @tc.name: formID id is error(formID = 0)
 * @tc.desc: 1.release the form
 *           2.Verify the result is false
 */
HWTEST_F(FmsReleaseFormTest, FMS_releaseForm_0400, Function | MediumTest | Level0)
{
    std::cout << "============START FMS_releaseForm_0400" << std::endl;

    std::string bundleName = "com.ohos.form.manager.normal";
    std::string abilityName = "FormAbilityReleaseForm";
    MAP_STR_STR params;
    Want want = SystemTestFormUtil::MakeWant("device", abilityName, bundleName, params);
    SystemTestFormUtil::StartAbility(want, abilityManager);
    EXPECT_EQ(SystemTestFormUtil::WaitCompleted(event, FORM_EVENT_ABILITY_ONACTIVED, 0), 0);

    std::string eventData = FORM_EVENT_REQ_RELEASE_FORM_0400;
    SystemTestFormUtil::PublishEvent(FORM_EVENT_REQ_RELEASE_FORM_0400, EVENT_CODE_400, eventData);
    EXPECT_EQ(0, SystemTestFormUtil::WaitCompleted(event, FORM_EVENT_RECV_RELEASE_FORM_0400, EVENT_CODE_400));
    std::string data = SystemTestFormUtil::GetData(event, FORM_EVENT_RECV_RELEASE_FORM_0400, EVENT_CODE_400);
    bool result = data == "false";
    EXPECT_TRUE(result);
    if (!result) {
        GTEST_LOG_(INFO) << "FMS_releaseForm_0400,  result:" << result;
    }
    SystemTestFormUtil::CleanMsg(event);
    std::cout << "============END FMS_releaseForm_0400" << std::endl;
}

/**
 * @tc.number: FMS_releaseForm_0500
 * @tc.name: formID id is error because formId is not self
 * @tc.desc: 1.host A create a formA
 *           2.host B release the formA
 *           2.Verify the result is false
 */
HWTEST_F(FmsReleaseFormTest, FMS_releaseForm_0500, Function | MediumTest | Level0)
{
    std::cout << "============START FMS_releaseForm_0500" << std::endl;
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
    
    GTEST_LOG_(INFO) << "FMS_releaseForm_0500,  data:[" << data << "]" << std::endl;
    if (data != "") {
        std::string bundleName2 = "com.ohos.form.manager.normal";
        std::string abilityName2 = "FormAbilityReleaseForm";
        MAP_STR_STR params2;
        Want want2 = SystemTestFormUtil::MakeWant("device", abilityName2, bundleName2, params2);
        SystemTestFormUtil::StartAbility(want2, abilityManager);
        EXPECT_EQ(SystemTestFormUtil::WaitCompleted(event, FORM_EVENT_ABILITY_ONACTIVED, 0), 0);

        std::string eventData2 = data;
        SystemTestFormUtil::PublishEvent(FORM_EVENT_REQ_RELEASE_FORM_0500, EVENT_CODE_500, eventData2);
        EXPECT_EQ(0, SystemTestFormUtil::WaitCompleted(event, FORM_EVENT_RECV_RELEASE_FORM_0500, EVENT_CODE_500));
        std::string data2 = SystemTestFormUtil::GetData(event, FORM_EVENT_RECV_RELEASE_FORM_0500, EVENT_CODE_500);
        bool result = data2 == "false";
        EXPECT_TRUE(result);
        if (!result) {
            GTEST_LOG_(INFO) << "FMS_releaseForm_0500,  result:" << result;
        }
        SystemTestFormUtil::PublishEvent(FORM_EVENT_REQ_ONE_NORMAL_FORM_DEL, EVENT_CODE_101, data);
        EXPECT_EQ(0, SystemTestFormUtil::WaitCompleted(event, FORM_EVENT_RECV_ONE_NORMAL_FORM_DEL, EVENT_CODE_101));
    }
    SystemTestFormUtil::CleanMsg(event);
    std::cout << "============END FMS_releaseForm_0500" << std::endl;
}

/**
 * @tc.number: FMS_releaseForm_0600
 * @tc.name: Form ID error (FormID does not exist)
 * @tc.desc: 1.host create one normal card
 *           2.host delete the form
 *           3.host release the form and verify the result is false
 */
HWTEST_F(FmsReleaseFormTest, FMS_releaseForm_0600, Function | MediumTest | Level0)
{
    std::cout << "============START FMS_releaseForm_0600" << std::endl;

    std::string bundleName = "com.ohos.form.manager.normal";
    std::string abilityName = "FormAbilityReleaseForm";
    MAP_STR_STR params;
    Want want = SystemTestFormUtil::MakeWant("device", abilityName, bundleName, params);
    SystemTestFormUtil::StartAbility(want, abilityManager);
    EXPECT_EQ(SystemTestFormUtil::WaitCompleted(event, FORM_EVENT_ABILITY_ONACTIVED, 0), 0);

    std::string eventData = FORM_EVENT_REQ_RELEASE_FORM_0600;
    SystemTestFormUtil::PublishEvent(FORM_EVENT_REQ_RELEASE_FORM_0600, EVENT_CODE_600, eventData);
    EXPECT_EQ(0, SystemTestFormUtil::WaitCompleted(event, FORM_EVENT_RECV_RELEASE_FORM_0600, EVENT_CODE_600));
    std::string data = SystemTestFormUtil::GetData(event, FORM_EVENT_RECV_RELEASE_FORM_0600, EVENT_CODE_600);
    bool result = data == "false";
    EXPECT_TRUE(result);
    if (!result) {
        GTEST_LOG_(INFO) << "FMS_releaseForm_0600,  result:" << result;
    }
    SystemTestFormUtil::CleanMsg(event);

    std::cout << "============END FMS_releaseForm_0600" << std::endl;
}

/**
 * @tc.number: FMS_releaseForm_0700
 * @tc.name: form reference is not 0 after release form(isReleaseCache：false)
 * @tc.desc: 1.host A create a normal formA
 *           2.host B create a form with formA's ID
 *           3.release formA(isReleaseCache is false), and verify the result is true
 */
HWTEST_F(FmsReleaseFormTest, FMS_releaseForm_0700, Function | MediumTest | Level1)
{
    std::cout << "============START FMS_releaseForm_0700" << std::endl;

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
    GTEST_LOG_(INFO) << "FMS_releaseForm_0700,  formOne:[" << formOne << "]" << std::endl;
    if (formOne != "") {
        std::string bundleName2 = "com.ohos.form.manager.normal";
        std::string abilityName2 = "FormAbilityReleaseForm";
        MAP_STR_STR params2;
        Want want2 = SystemTestFormUtil::MakeWant("device", abilityName2, bundleName2, params2);
        SystemTestFormUtil::StartAbility(want2, abilityManager);
        EXPECT_EQ(SystemTestFormUtil::WaitCompleted(event, FORM_EVENT_ABILITY_ONACTIVED, 0), 0);

        std::string eventData2 = formOne;
        SystemTestFormUtil::PublishEvent(FORM_EVENT_REQ_RELEASE_FORM_0700, EVENT_CODE_700, eventData2);
        EXPECT_EQ(0, SystemTestFormUtil::WaitCompleted(event, FORM_EVENT_RECV_RELEASE_FORM_0700, EVENT_CODE_700));
        std::string formTwo = SystemTestFormUtil::GetData(event, FORM_EVENT_RECV_RELEASE_FORM_0700, EVENT_CODE_700);
        bool result = formTwo == "true";
        EXPECT_TRUE(result);
        if (!result) {
            GTEST_LOG_(INFO) << "FMS_releaseForm_0700,  result:" << result;
        }
        bool cacheRes = FmsGetCacheInfoByFormId(atoll(formOne.c_str()));
        bool hostRes = FmsGetHostInfoByFormId(atoll(formOne.c_str()), "com.ohos.form.manager.normal");
        bool storageRes = FmsGetStorageFormInfos(atoll(formOne.c_str()));
        EXPECT_TRUE(cacheRes);
        EXPECT_FALSE(hostRes);
        EXPECT_TRUE(storageRes);
        SystemTestFormUtil::PublishEvent(FORM_EVENT_REQ_ONE_NORMAL_FORM_DEL, EVENT_CODE_101, formOne);
        EXPECT_EQ(0, SystemTestFormUtil::WaitCompleted(event, FORM_EVENT_RECV_ONE_NORMAL_FORM_DEL, EVENT_CODE_101));
        SystemTestFormUtil::PublishEvent(FORM_EVENT_REQ_RELEASE_FORM_COMMON_DEL, EVENT_CODE_101, formOne);
        EXPECT_EQ(0, SystemTestFormUtil::WaitCompleted(event, FORM_EVENT_RECV_RELEASE_FORM_COMMON_DEL, EVENT_CODE_101));
    }
    SystemTestFormUtil::CleanMsg(event);
    std::cout << "============END FMS_releaseForm_0700" << std::endl;
}

/**
 * @tc.number: FMS_releaseForm_0800
 * @tc.name: After releasing a normal form, the form reference is 0.(isReleaseCache：false)
 *     there are still created normal forms in FMS.
 * @tc.desc: 1.host A an host B create one normal card respectively
 *           2.host A release the form and verify the result is true
 */
HWTEST_F(FmsReleaseFormTest, FMS_releaseForm_0800, Function | MediumTest | Level1)
{
    std::cout << "============START FMS_releaseForm_0800" << std::endl;

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
    GTEST_LOG_(INFO) << "FMS_releaseForm_0800,  formOne:[" << formOne << "]" << std::endl;
    if (formOne != "") {
        std::string bundleName2 = "com.ohos.form.manager.normal";
        std::string abilityName2 = "FormAbilityReleaseForm";
        MAP_STR_STR params2;
        Want want2 = SystemTestFormUtil::MakeWant("device", abilityName2, bundleName2, params2);
        SystemTestFormUtil::StartAbility(want2, abilityManager);
        EXPECT_EQ(SystemTestFormUtil::WaitCompleted(event, FORM_EVENT_ABILITY_ONACTIVED, 0), 0);

        std::string eventData2 = formOne;
        SystemTestFormUtil::PublishEvent(FORM_EVENT_REQ_RELEASE_FORM_0800, EVENT_CODE_800, eventData2);
        EXPECT_EQ(0, SystemTestFormUtil::WaitCompleted(event, FORM_EVENT_RECV_RELEASE_FORM_0800, EVENT_CODE_800));
        std::string formTwo = SystemTestFormUtil::GetData(event, FORM_EVENT_RECV_RELEASE_FORM_0800, EVENT_CODE_800);
        GTEST_LOG_(INFO) << "FMS_releaseForm_0800,  formTwo:[" << formTwo << "]" << std::endl;
        bool result = (formTwo != "false" && formTwo != "");
        EXPECT_TRUE(result);
        if (!result) {
            GTEST_LOG_(INFO) << "FMS_releaseForm_0800,  result:" << result;
        }
        bool cacheRes = FmsGetCacheInfoByFormId(atoll(formTwo.c_str()));
        bool storageRes = FmsGetStorageFormInfos(atoll(formTwo.c_str()));
        bool timerRes = FmsGetFormTimerTask(atoll(formTwo.c_str()));
        EXPECT_TRUE(cacheRes);
        EXPECT_TRUE(storageRes);
        EXPECT_FALSE(timerRes);
        SystemTestFormUtil::PublishEvent(FORM_EVENT_REQ_ONE_NORMAL_FORM_DEL, EVENT_CODE_101, formOne);
        EXPECT_EQ(0, SystemTestFormUtil::WaitCompleted(event, FORM_EVENT_RECV_ONE_NORMAL_FORM_DEL, EVENT_CODE_101));
        SystemTestFormUtil::PublishEvent(FORM_EVENT_REQ_RELEASE_FORM_COMMON_DEL, EVENT_CODE_101, formTwo);
        EXPECT_EQ(0, SystemTestFormUtil::WaitCompleted(event, FORM_EVENT_RECV_RELEASE_FORM_COMMON_DEL, EVENT_CODE_101));
    }
    SystemTestFormUtil::CleanMsg(event);
    std::cout << "============END FMS_releaseForm_0800" << std::endl;
}

/**
 * @tc.number: FMS_releaseForm_0900
 * @tc.name: After releasing a normal form, there is no created normal form in FMS.(isReleaseCache：false)
 * @tc.desc: 1.host create one normal card
 *           2.host release the form and verify the result is true
 */
HWTEST_F(FmsReleaseFormTest, FMS_releaseForm_0900, Function | MediumTest | Level1)
{
    std::cout << "============START FMS_releaseForm_0900" << std::endl;

    std::string bundleName = "com.ohos.form.manager.normal";
    std::string abilityName = "FormAbilityReleaseForm";
    MAP_STR_STR params;
    Want want = SystemTestFormUtil::MakeWant("device", abilityName, bundleName, params);
    SystemTestFormUtil::StartAbility(want, abilityManager);
    EXPECT_EQ(SystemTestFormUtil::WaitCompleted(event, FORM_EVENT_ABILITY_ONACTIVED, 0), 0);

    std::string eventData = FORM_EVENT_REQ_RELEASE_FORM_0900;
    SystemTestFormUtil::PublishEvent(FORM_EVENT_REQ_RELEASE_FORM_0900, EVENT_CODE_900, eventData);
    EXPECT_EQ(0, SystemTestFormUtil::WaitCompleted(event, FORM_EVENT_RECV_RELEASE_FORM_0900, EVENT_CODE_900));
    std::string data = SystemTestFormUtil::GetData(event, FORM_EVENT_RECV_RELEASE_FORM_0900, EVENT_CODE_900);
    GTEST_LOG_(INFO) << "FMS_releaseForm_0900,  data:[" << data << "]" << std::endl;
    bool result = (data != "false" && data != "");
    EXPECT_TRUE(result);
    if (!result) {
        GTEST_LOG_(INFO) << "FMS_releaseForm_0900,  result:" << result;
    }
    bool cacheRes = FmsGetCacheInfoByFormId(atoll(data.c_str()));
    bool hostRes = FmsGetHostInfoByFormId(atoll(data.c_str()), "com.ohos.form.manager.normal");
    bool storageRes = FmsGetStorageFormInfos(atoll(data.c_str()));
    EXPECT_TRUE(cacheRes);
    EXPECT_FALSE(hostRes);
    EXPECT_TRUE(storageRes);
    SystemTestFormUtil::PublishEvent(FORM_EVENT_REQ_RELEASE_FORM_COMMON_DEL, EVENT_CODE_101, data);
    EXPECT_EQ(0, SystemTestFormUtil::WaitCompleted(event, FORM_EVENT_RECV_RELEASE_FORM_COMMON_DEL, EVENT_CODE_101));
    SystemTestFormUtil::CleanMsg(event);

    std::cout << "============END FMS_releaseForm_0900" << std::endl;
}

/**
 * @tc.number: FMS_releaseForm_1100
 * @tc.name: After releasing a normal form, the form reference is 0.(isReleaseCache：true)
 *     there are still created normal forms in FMS.
 * @tc.desc: 1.host A an host B create one normal form respectively
 *           2.host A release the form and verify the result is true
 */
HWTEST_F(FmsReleaseFormTest, FMS_releaseForm_1100, Function | MediumTest | Level1)
{
    std::cout << "============START FMS_releaseForm_1100" << std::endl;

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
    GTEST_LOG_(INFO) << "FMS_releaseForm_1100,  formOne:[" << formOne << "]" << std::endl;
    if (formOne != "") {
        std::string bundleName2 = "com.ohos.form.manager.normal";
        std::string abilityName2 = "FormAbilityReleaseForm";
        MAP_STR_STR params2;
        Want want2 = SystemTestFormUtil::MakeWant("device", abilityName2, bundleName2, params2);
        SystemTestFormUtil::StartAbility(want2, abilityManager);
        EXPECT_EQ(SystemTestFormUtil::WaitCompleted(event, FORM_EVENT_ABILITY_ONACTIVED, 0), 0);

        std::string eventData2 = formOne;
        SystemTestFormUtil::PublishEvent(FORM_EVENT_REQ_RELEASE_FORM_1100, EVENT_CODE_1100, eventData2);
        EXPECT_EQ(0, SystemTestFormUtil::WaitCompleted(event, FORM_EVENT_RECV_RELEASE_FORM_1100, EVENT_CODE_1100));
        std::string formTwo = SystemTestFormUtil::GetData(event, FORM_EVENT_RECV_RELEASE_FORM_1100, EVENT_CODE_1100);
        GTEST_LOG_(INFO) << "FMS_releaseForm_1100,  formTwo:[" << formTwo << "]" << std::endl;
        bool result = (formTwo != "false" && formTwo != "");
        EXPECT_TRUE(result);
        if (!result) {
            GTEST_LOG_(INFO) << "FMS_releaseForm_1100,  result:" << result;
        }
        bool cacheRes = FmsGetCacheInfoByFormId(atoll(formTwo.c_str()));
        bool storageRes = FmsGetStorageFormInfos(atoll(formTwo.c_str()));
        bool timerRes = FmsGetFormTimerTask(atoll(formTwo.c_str()));
        EXPECT_FALSE(cacheRes);
        EXPECT_TRUE(storageRes);
        EXPECT_FALSE(timerRes);
        SystemTestFormUtil::PublishEvent(FORM_EVENT_REQ_ONE_NORMAL_FORM_DEL, EVENT_CODE_101, formOne);
        EXPECT_EQ(0, SystemTestFormUtil::WaitCompleted(event, FORM_EVENT_RECV_ONE_NORMAL_FORM_DEL, EVENT_CODE_101));
        SystemTestFormUtil::PublishEvent(FORM_EVENT_REQ_RELEASE_FORM_COMMON_DEL, EVENT_CODE_101, formTwo);
        EXPECT_EQ(0, SystemTestFormUtil::WaitCompleted(event, FORM_EVENT_RECV_RELEASE_FORM_COMMON_DEL, EVENT_CODE_101));
    }
    SystemTestFormUtil::CleanMsg(event);

    std::cout << "============END FMS_releaseForm_1100" << std::endl;
}

/**
 * @tc.number: FMS_releaseForm_1200
 * @tc.name: After releasing a normal form, there is no created normal form in FMS.(isReleaseCache：true)
 * @tc.desc: 1.host create one normal card
 *           2.host release the form and verify the result is true
 */
HWTEST_F(FmsReleaseFormTest, FMS_releaseForm_1200, Function | MediumTest | Level1)
{
    std::cout << "============START FMS_releaseForm_1200" << std::endl;
    
    std::string bundleName = "com.ohos.form.manager.normal";
    std::string abilityName = "FormAbilityReleaseForm";
    MAP_STR_STR params;
    Want want = SystemTestFormUtil::MakeWant("device", abilityName, bundleName, params);
    SystemTestFormUtil::StartAbility(want, abilityManager);
    EXPECT_EQ(SystemTestFormUtil::WaitCompleted(event, FORM_EVENT_ABILITY_ONACTIVED, 0), 0);

    std::string eventData = FORM_EVENT_REQ_RELEASE_FORM_1200;
    SystemTestFormUtil::PublishEvent(FORM_EVENT_REQ_RELEASE_FORM_1200, EVENT_CODE_1200, eventData);
    EXPECT_EQ(0, SystemTestFormUtil::WaitCompleted(event, FORM_EVENT_RECV_RELEASE_FORM_1200, EVENT_CODE_1200));
    std::string data = SystemTestFormUtil::GetData(event, FORM_EVENT_RECV_RELEASE_FORM_1200, EVENT_CODE_1200);
    GTEST_LOG_(INFO) << "FMS_releaseForm_1200,  data:[" << data << "]" << std::endl;
    bool result = (data != "false" && data != "");
    EXPECT_TRUE(result);
    if (!result) {
        GTEST_LOG_(INFO) << "FMS_releaseForm_1200,  result:" << result;
    }
    bool cacheRes = FmsGetCacheInfoByFormId(atoll(data.c_str()));
    bool hostRes = FmsGetHostInfoByFormId(atoll(data.c_str()), "com.ohos.form.manager.normal");
    bool storageRes = FmsGetStorageFormInfos(atoll(data.c_str()));
    EXPECT_FALSE(cacheRes);
    EXPECT_FALSE(hostRes);
    EXPECT_TRUE(storageRes);
    SystemTestFormUtil::PublishEvent(FORM_EVENT_REQ_RELEASE_FORM_COMMON_DEL, EVENT_CODE_101, data);
    EXPECT_EQ(0, SystemTestFormUtil::WaitCompleted(event, FORM_EVENT_RECV_RELEASE_FORM_COMMON_DEL, EVENT_CODE_101));
    SystemTestFormUtil::CleanMsg(event);
    std::cout << "============END FMS_releaseForm_1200" << std::endl;
}

/**
 * @tc.number: FMS_releaseForm_1300
 * @tc.name: single host continuously release forms
 * @tc.desc: 1.There are 2 forms created by the single host
 *           2.single host release 2 forms successfully
 */
HWTEST_F(FmsReleaseFormTest, FMS_releaseForm_1300, Function | MediumTest | Level2)
{
    std::cout << "============START FMS_releaseForm_1300" << std::endl;
    
    std::string bundleName = "com.ohos.form.manager.normal";
    std::string abilityName = "FormAbilityReleaseForm";
    MAP_STR_STR params;
    Want want = SystemTestFormUtil::MakeWant("device", abilityName, bundleName, params);
    SystemTestFormUtil::StartAbility(want, abilityManager);
    EXPECT_EQ(SystemTestFormUtil::WaitCompleted(event, FORM_EVENT_ABILITY_ONACTIVED, 0), 0);

    std::string eventData = "";
    SystemTestFormUtil::PublishEvent(FORM_EVENT_REQ_RELEASE_FORM_1300, EVENT_CODE_1300, eventData);
    EXPECT_EQ(0, SystemTestFormUtil::WaitCompleted(event, FORM_EVENT_RECV_RELEASE_FORM_1300, EVENT_CODE_1300));
    std::string data = SystemTestFormUtil::GetData(event, FORM_EVENT_RECV_RELEASE_FORM_1300, EVENT_CODE_1300);
    bool result = data != "";
    EXPECT_TRUE(result);
    if (!result) {
        GTEST_LOG_(INFO) << "FMS_releaseForm_1300,  result:" << result;
    }
    GTEST_LOG_(INFO) << "FMS_releaseForm_1300,  data:[" << data << "]" << std::endl;
    if (data != "") {
        std::string eventData2 = data;
        SystemTestFormUtil::PublishEvent(FORM_EVENT_REQ_RELEASE_FORM_1300, EVENT_CODE_1300, eventData2);
        EXPECT_EQ(0, SystemTestFormUtil::WaitCompleted(event, FORM_EVENT_RECV_RELEASE_FORM_1300, EVENT_CODE_1300));
        std::string data2 = SystemTestFormUtil::GetData(event, FORM_EVENT_RECV_RELEASE_FORM_1300, EVENT_CODE_1300);
        bool result2 = data2 == "true";
        EXPECT_TRUE(result2);
        if (!result2) {
            GTEST_LOG_(INFO) << "FMS_releaseForm_1300,  result2:" << result2;
        }
    }
    SystemTestFormUtil::CleanMsg(event);

    std::cout << "============END FMS_releaseForm_1300" << std::endl;
}

/**
 * @tc.number: FMS_releaseForm_1400
 * @tc.name: release temp form
 * @tc.desc: 1.host create one temp form
 *           2.release the form and judge the result is true
 */
HWTEST_F(FmsReleaseFormTest, FMS_releaseForm_1400, Function | MediumTest | Level1)
{
    std::cout << "============START FMS_releaseForm_1400" << std::endl;

    std::string bundleName = "com.ohos.form.manager.normal";
    std::string abilityName = "FormAbilityReleaseForm";
    MAP_STR_STR params;
    Want want = SystemTestFormUtil::MakeWant("device", abilityName, bundleName, params);
    SystemTestFormUtil::StartAbility(want, abilityManager);
    EXPECT_EQ(SystemTestFormUtil::WaitCompleted(event, FORM_EVENT_ABILITY_ONACTIVED, 0), 0);

    std::string eventData = FORM_EVENT_REQ_RELEASE_FORM_1400;
    SystemTestFormUtil::PublishEvent(FORM_EVENT_REQ_RELEASE_FORM_1400, EVENT_CODE_1400, eventData);
    EXPECT_EQ(0, SystemTestFormUtil::WaitCompleted(event, FORM_EVENT_RECV_RELEASE_FORM_1400, EVENT_CODE_1400));
    std::string data = SystemTestFormUtil::GetData(event, FORM_EVENT_RECV_RELEASE_FORM_1400, EVENT_CODE_1400);
    bool result = data == "true";
    EXPECT_TRUE(result);
    if (!result) {
        GTEST_LOG_(INFO) << "FMS_releaseForm_1400,  result:" << result;
    }
    SystemTestFormUtil::CleanMsg(event);

    std::cout << "============END FMS_releaseForm_1400" << std::endl;
}
/**
 * @tc.number: FMS_releaseForm_1000
 * @tc.name: form reference is not 0 after release form(isReleaseCache：true)
 * @tc.desc: 1.host A create a normal formA
 *           2.host B create a form with formA's ID
 *           3.release formA(isReleaseCache：true), and verify the result is true
 */
HWTEST_F(FmsReleaseFormTest, FMS_releaseForm_1000, Function | MediumTest | Level1)
{
    std::cout << "============START FMS_releaseForm_1000" << std::endl;

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
    
    GTEST_LOG_(INFO) << "FMS_releaseForm_1000,  formOne:[" << formOne << "]" << std::endl;
    if (formOne != "") {
        std::string bundleName2 = "com.ohos.form.manager.normal";
        std::string abilityName2 = "FormAbilityReleaseForm";
        MAP_STR_STR params2;
        Want want2 = SystemTestFormUtil::MakeWant("device", abilityName2, bundleName2, params2);
        SystemTestFormUtil::StartAbility(want2, abilityManager);
        EXPECT_EQ(SystemTestFormUtil::WaitCompleted(event, FORM_EVENT_ABILITY_ONACTIVED, 0), 0);

        std::string eventData2 = formOne;
        SystemTestFormUtil::PublishEvent(FORM_EVENT_REQ_RELEASE_FORM_1000, EVENT_CODE_1000, eventData2);
        EXPECT_EQ(0, SystemTestFormUtil::WaitCompleted(event, FORM_EVENT_RECV_RELEASE_FORM_1000, EVENT_CODE_1000));
        std::string formTwo = SystemTestFormUtil::GetData(event, FORM_EVENT_RECV_RELEASE_FORM_1000, EVENT_CODE_1000);
        bool result = formTwo == "true";
        EXPECT_TRUE(result);
        if (!result) {
            GTEST_LOG_(INFO) << "FMS_releaseForm_1000,  result:" << result;
        }
        bool cacheRes = FmsGetCacheInfoByFormId(atoll(formOne.c_str()));
        bool hostRes = FmsGetHostInfoByFormId(atoll(formOne.c_str()), "com.ohos.form.manager.normal");
        bool storageRes = FmsGetStorageFormInfos(atoll(formOne.c_str()));
        EXPECT_TRUE(cacheRes);
        EXPECT_FALSE(hostRes);
        EXPECT_TRUE(storageRes);
        SystemTestFormUtil::PublishEvent(FORM_EVENT_REQ_ONE_NORMAL_FORM_DEL, EVENT_CODE_101, formOne);
        EXPECT_EQ(0, SystemTestFormUtil::WaitCompleted(event, FORM_EVENT_RECV_ONE_NORMAL_FORM_DEL, EVENT_CODE_101));
        SystemTestFormUtil::PublishEvent(FORM_EVENT_REQ_RELEASE_FORM_COMMON_DEL, EVENT_CODE_101, formOne);
        EXPECT_EQ(0, SystemTestFormUtil::WaitCompleted(event, FORM_EVENT_RECV_RELEASE_FORM_COMMON_DEL, EVENT_CODE_101));
    }
    SystemTestFormUtil::CleanMsg(event);

    std::cout << "============END FMS_releaseForm_1000" << std::endl;
}
bool FmsReleaseFormTest::FmsGetCacheInfoByFormId(std::int64_t formId)
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
bool FmsReleaseFormTest::FmsGetHostInfoByFormId(std::int64_t formId, std::string hostBundleName)
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
bool FmsReleaseFormTest::FmsGetStorageFormInfos(std::int64_t formId)
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
bool FmsReleaseFormTest::FmsGetFormTimerTask(std::int64_t formId)
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
}  // namespace AppExecFwk
}  // namespace OHOS