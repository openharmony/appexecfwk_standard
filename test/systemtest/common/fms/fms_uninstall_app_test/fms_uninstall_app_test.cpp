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
#include "uninstall_app_test_config_parser.h"
#include "system_ability_definition.h"

using OHOS::AAFwk::Want;
using namespace testing::ext;
using namespace std::chrono_literals;
using namespace OHOS::STtools;

namespace {
std::vector<std::string> bundleNameList = {
    "com.form.formsystemtestservicea",
    //"com.form.formsystemtestserviceb"
};
std::vector<std::string> hapNameList = {
    "ActsFormSystemTestServiceA",
    //"ActsFormSystemTestServiceB"
};
}  // namespace

namespace OHOS {
namespace AppExecFwk {

//static UninstallAPPTestLevel unLevel_;
static std::string uninstallBundleName = "";
const std::string COMMON_EVENT_PACKAGE_REMOVED = "usual.event.PACKAGE_REMOVED";

class FormUninstallAppTest : public testing::Test {
public:
    static void SetUpTestCase();
    static void TearDownTestCase();
    static bool SubscribeEvent();

    void SetUp();
    void TearDown();
    void StartAbilityKitTest(const std::string &abilityName, const std::string &bundleName);
    void TerminateAbility(const std::string &eventName, const std::string &abilityName);

    // Test case
    static void UninstallTest_0100();
    static void UninstallTest_0200();
    static void UninstallTest_0300();
    static void UninstallTest_1100();
    static void UninstallTest_1300();
    static void UninstallTest_1700();
    static void UninstallTest_1800();

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

std::vector<std::string> FormUninstallAppTest::eventList = {
    FORM_EVENT_ABILITY_ONACTIVED, FORM_EVENT_RECV_UNINSTALL_TEST_0100, FORM_EVENT_RECV_UNINSTALL_TEST_0200,
    FORM_EVENT_RECV_UNINSTALL_TEST_0300, FORM_EVENT_RECV_UNINSTALL_TEST_0400, FORM_EVENT_RECV_UNINSTALL_TEST_0500,
    FORM_EVENT_RECV_UNINSTALL_TEST_0600, FORM_EVENT_RECV_UNINSTALL_TEST_0700, FORM_EVENT_RECV_UNINSTALL_TEST_0800,
    FORM_EVENT_RECV_UNINSTALL_TEST_0900, FORM_EVENT_RECV_UNINSTALL_TEST_1000, FORM_EVENT_RECV_UNINSTALL_TEST_1100,
    FORM_EVENT_RECV_UNINSTALL_TEST_1200, FORM_EVENT_RECV_UNINSTALL_TEST_1300, FORM_EVENT_RECV_UNINSTALL_TEST_1400,
    FORM_EVENT_RECV_UNINSTALL_TEST_1500, FORM_EVENT_RECV_UNINSTALL_TEST_1600, FORM_EVENT_RECV_UNINSTALL_TEST_1700,
    FORM_EVENT_RECV_UNINSTALL_TEST_1800, COMMON_EVENT_PACKAGE_REMOVED
};

FormEvent FormUninstallAppTest::event = FormEvent();
sptr<AAFwk::IAbilityManager> FormUninstallAppTest::abilityMs = nullptr;
std::shared_ptr<FormUninstallAppTest::FormEventSubscriber> FormUninstallAppTest::subscriber_ = nullptr;
void FormUninstallAppTest::FormEventSubscriber::OnReceiveEvent(const CommonEventData &data)
{
    GTEST_LOG_(INFO) << "OnReceiveEvent: event=" << data.GetWant().GetAction();
    GTEST_LOG_(INFO) << "OnReceiveEvent: data=" << data.GetData();
    GTEST_LOG_(INFO) << "OnReceiveEvent: code=" << data.GetCode();
    SystemTestFormUtil::Completed(event, data.GetWant().GetAction(), data.GetCode(), data.GetData());

    if (data.GetWant().GetAction() == COMMON_EVENT_PACKAGE_REMOVED) {
        GTEST_LOG_(INFO) << "OnReceiveEvent: bundleName =" << data.GetWant().GetElement().GetBundleName();
        uninstallBundleName = data.GetWant().GetElement().GetBundleName();
    }
}

void FormUninstallAppTest::SetUpTestCase()
{
    SystemTestFormUtil::InstallHaps(hapNameList);
    if (!SubscribeEvent()) {
        GTEST_LOG_(INFO) << "SubscribeEvent error";
    }
}

void FormUninstallAppTest::TearDownTestCase()
{
    GTEST_LOG_(INFO) << "UnSubscribeCommonEvent calld";
    //SystemTestFormUtil::UninstallBundle(bundleNameList);
    CommonEventManager::UnSubscribeCommonEvent(subscriber_);
}

void FormUninstallAppTest::SetUp()
{
}

void FormUninstallAppTest::TearDown()
{
    GTEST_LOG_(INFO) << "CleanMsg calld";
    SystemTestFormUtil::CleanMsg(event);
}
bool FormUninstallAppTest::SubscribeEvent()
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

void FormUninstallAppTest::UninstallTest_0100()
{
    std::string bundleName = "com.ohos.form.manager.Uninstall";
    std::string abilityName = "FormAbilityUninstallApp";
    MAP_STR_STR params;
    Want want = SystemTestFormUtil::MakeWant("device", abilityName, bundleName, params);
    SystemTestFormUtil::StartAbility(want, abilityMs);
    EXPECT_EQ(SystemTestFormUtil::WaitCompleted(event, FORM_EVENT_ABILITY_ONACTIVED, 0), 0);

    // AcquireForm
    std::string eventData = FORM_EVENT_REQ_UNINSTALL_TEST_0100;
    SystemTestFormUtil::PublishEvent(FORM_EVENT_REQ_UNINSTALL_TEST_0100, 100, eventData);

    // OnAcquired
    EXPECT_EQ(0, SystemTestFormUtil::WaitCompleted(event, FORM_EVENT_RECV_UNINSTALL_TEST_0100, 101));
    std::string data = SystemTestFormUtil::GetData(event, FORM_EVENT_RECV_UNINSTALL_TEST_0100, 101);
    std::string formId = data;

    // OnUpdate
    EXPECT_EQ(0, SystemTestFormUtil::WaitCompleted(event, FORM_EVENT_RECV_UNINSTALL_TEST_0100, 102));
    data = SystemTestFormUtil::GetData(event, FORM_EVENT_RECV_UNINSTALL_TEST_0100, 102);
    bool result = data == "true";
    EXPECT_TRUE(result);
    if (result) {
        GTEST_LOG_(INFO) << "FMS_uninstallAPP_0100 AcquireForm,  result:" << result;
    }

    // Uninstall app
    SystemTestFormUtil::UninstallBundle(bundleNameList);

    // Uninstall app event
    EXPECT_EQ(0, SystemTestFormUtil::WaitCompleted(event, COMMON_EVENT_PACKAGE_REMOVED, 0));
    result = uninstallBundleName == "com.form.formsystemtestservicea";
    EXPECT_TRUE(result);
    if (result) {
        uninstallBundleName = "";
        GTEST_LOG_(INFO) << "FMS_uninstallAPP_0100 Uninstall app,  result:" << result;
    }

    SystemTestFormUtil::CleanMsg(event);
}

void FormUninstallAppTest::UninstallTest_0200()
{
    std::string bundleName = "com.ohos.form.manager.uninstall";
    std::string abilityName = "FormAbilityUninstallApp";
    MAP_STR_STR params;
    Want want = SystemTestFormUtil::MakeWant("device", abilityName, bundleName, params);
    SystemTestFormUtil::StartAbility(want, abilityMs);
    EXPECT_EQ(SystemTestFormUtil::WaitCompleted(event, FORM_EVENT_ABILITY_ONACTIVED, 0), 0);

    SystemTestFormUtil::CleanMsg(event);
}

void FormUninstallAppTest::UninstallTest_0300()
{
    std::string bundleName = "com.ohos.form.manager.uninstall";
    std::string abilityName = "FormAbilityUninstallApp";
    MAP_STR_STR params;
    Want want = SystemTestFormUtil::MakeWant("device", abilityName, bundleName, params);
    SystemTestFormUtil::StartAbility(want, abilityMs);
    EXPECT_EQ(SystemTestFormUtil::WaitCompleted(event, FORM_EVENT_ABILITY_ONACTIVED, 0), 0);

    SystemTestFormUtil::CleanMsg(event);
}

void FormUninstallAppTest::UninstallTest_1100()
{
    std::string bundleName = "com.ohos.form.manager.uninstall";
    std::string abilityName = "FormAbilityUninstallApp";
    MAP_STR_STR params;
    Want want = SystemTestFormUtil::MakeWant("device", abilityName, bundleName, params);
    SystemTestFormUtil::StartAbility(want, abilityMs);
    EXPECT_EQ(SystemTestFormUtil::WaitCompleted(event, FORM_EVENT_ABILITY_ONACTIVED, 0), 0);

    SystemTestFormUtil::CleanMsg(event);
}

void FormUninstallAppTest::UninstallTest_1300()
{
}

void FormUninstallAppTest::UninstallTest_1700()
{
}

void FormUninstallAppTest::UninstallTest_1800()
{
}

/**
 * @tc.number    : FMS_uninstallAPP_0100
 * @tc.name      : AcquireForm. then uninstall the service app
 * @tc.desc      : uninstall the service app successfully
 */
HWTEST_F(FormUninstallAppTest, FMS_uninstallAPP_0100, Function | MediumTest | Level2)
{
    std::cout << "START FMS_uninstallAPP_0100" << std::endl;

    FormUninstallAppTest::UninstallTest_0100();

    std::cout << "END FMS_uninstallAPP_0100" << std::endl;
}

/**
 * @tc.number    : FMS_uninstallAPP_0200
 * @tc.name      : AcquireForm、ReleaseForm/DeleteForm uninstall test
 * @tc.desc      : AcquireForm successfully/ReleaseForm successfully/DeleteForm successfully
 */
HWTEST_F(FormUninstallAppTest, FMS_uninstallAPP_0200, Function | MediumTest | Level2)
{
    std::cout << "START FMS_uninstallAPP_0200" << std::endl;

    std::cout << "END FMS_uninstallAPP_0200" << std::endl;
}

/**
 * @tc.number    : FMS_uninstallAPP_0300
 * @tc.name      : AcquireForm/CastTempForm/DeleteForm uninstall test
 * @tc.desc      : AcquireForm successfully/CastTempForm successfully/DeleteForm successfully
 */
HWTEST_F(FormUninstallAppTest, FMS_uninstallAPP_0300, Function | MediumTest | Level2)
{
    std::cout << "START FMS_uninstallAPP_0300" << std::endl;
    std::cout << "END FMS_uninstallAPP_0300" << std::endl;
}

/**
 * @tc.number    : FMS_uninstallAPP_1100
 * @tc.name      : NotifyInvisibleForms/NotifyVisibleForms uninstall test
 * @tc.desc      : NotifyInvisibleForms successfully/NotifyVisibleForms successfully
 */
HWTEST_F(FormUninstallAppTest, FMS_uninstallAPP_1100, Function | MediumTest | Level2)
{
    std::cout << "START FMS_uninstallAPP_1100" << std::endl;

    std::cout << "END FMS_uninstallAPP_1100" << std::endl;
}

/**
 * @tc.number    : FMS_uninstallAPP_1300
 * @tc.name      : EnableUpdateForm/DisableUpdateForm uninstall test
 * @tc.desc      : EnableUpdateForm successfully/DisableUpdateForm successfully
 */
HWTEST_F(FormUninstallAppTest, FMS_uninstallAPP_1300, Function | MediumTest | Level2)
{
    std::cout << "START FMS_uninstallAPP_1300" << std::endl;

    std::cout << "END FMS_uninstallAPP_1300" << std::endl;
}

/**
 * @tc.number    : FMS_uninstallAPP_1700
 * @tc.name      : GetAllFormsInfo/GetFormsInfoByApp/GetFormsInfoByModule uninstall test
 * @tc.desc      : GetAllFormsInfo successfully/GetFormsInfoByApp successfully/GetFormsInfoByModule successfully
 */
HWTEST_F(FormUninstallAppTest, FMS_uninstallAPP_1700, Function | MediumTest | Level2)
{
    std::cout << "START FMS_uninstallAPP_1700" << std::endl;

    std::cout << "END FMS_uninstallAPP_1700" << std::endl;
}

/**
 * @tc.number    : FMS_uninstallAPP_1800
 * @tc.name      : CheckFMSReady uninstall test
 * @tc.desc      : CheckFMSReady successfully
 */
HWTEST_F(FormUninstallAppTest, FMS_uninstallAPP_1800, Function | MediumTest | Level2)
{
    std::cout << "START FMS_uninstallAPP_1800" << std::endl;

    std::cout << "END FMS_uninstallAPP_1800" << std::endl;
}
}  // namespace AppExecFwk
}  // namespace OHOS