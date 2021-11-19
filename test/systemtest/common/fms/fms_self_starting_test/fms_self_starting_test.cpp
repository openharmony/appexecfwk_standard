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
#include <fstream>

#include "ability_handler.h"
#include "ability_info.h"
#include "ability_local_record.h"
#include "ability_start_setting.h"
#include "app_log_wrapper.h"
#include "common_event.h"
#include "common_event_manager.h"
#include "context_deal.h"
#include "distributed_kv_data_manager.h"
#include "form_db_info.h"
#include "form_data_mgr.h"
#include "form_item_info.h"
#include "form_storage_mgr.h"
#include "form_event.h"
#include "form_st_common_info.h"
#include "iservice_registry.h"
#include "nlohmann/json.hpp"
#include "self_starting_test_config_parser.h"
#include "system_ability_definition.h"
#include "system_test_form_util.h"

using OHOS::AAFwk::Want;
using namespace testing::ext;
using namespace std::chrono_literals;
using namespace OHOS::STtools;

namespace OHOS {
namespace AppExecFwk {
static SelfStartingTestInfo selfStarting;
static SelfStartingTestConfigParser selfStartingParser;
const int ADD_FORM_A_NUMBER = 10;
const int ADD_FORM_LENGTH = 20;
class FmsSelfStartingTest : public testing::Test {
public:
    static void SetUpTestCase();
    static void TearDownTestCase();
    static bool SubscribeEvent();

    bool CompareA();
    bool CompareB();

    void SetUp();
    void TearDown();

    void StartAbilityKitTest(const std::string &abilityName, const std::string &bundleName);
    void TerminateAbility(const std::string &eventName, const std::string &abilityName);
    void ClearStorage();
    bool CheckKvStore();
    void TryTwice(const std::function<DistributedKv::Status()> &func);
  
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
    SelfStartingTestConfigParser selfStartingTcp;
    selfStartingTcp.ParseForSelfStartingTest(FMS_TEST_CONFIG_FILE_PATH, selfStarting);
    std::cout << "self starting test status : "
        << "addFormStatus : " << selfStarting.addFormStatus <<
            ", deleteFormStatus:" << selfStarting.deleteFormStatus <<
            ", compareStatus:" << selfStarting.compareStatus << std::endl;
    
    if (selfStarting.addFormStatus) {
        selfStartingParser.ClearStorage();
        for (int iCount = 0; iCount < ADD_FORM_A_NUMBER; iCount++) {
            Want want;
            want.SetParam(Constants::PARAM_FORM_DIMENSION_KEY, FORM_DIMENSION_1);
            want.SetParam(Constants::PARAM_FORM_NAME_KEY, PARAM_FORM_NAME1);
            want.SetParam(Constants::PARAM_MODULE_NAME_KEY, PARAM_PROVIDER_MODULE_NAME1);
            want.SetParam(Constants::PARAM_FORM_TEMPORARY_KEY, FORM_TEMP_FORM_FLAG_FALSE);
            want.SetParam(Constants::PARAM_FORM_TEMPORARY_KEY, FORM_TEMP_FORM_FLAG_FALSE);
            want.SetElementName(FORM_TEST_DEVICEID, FORM_PROVIDER_BUNDLE_NAME1, FORM_PROVIDER_ABILITY_NAME1);
            APP_LOGI("%{public}s, formCount: %{public}d", __func__, iCount + 1);
            want.SetParam(Constants::PARAM_FORM_ADD_COUNT, iCount + 1);
            // Set Want info end
            int errorCode = STtools::SystemTestFormUtil::DistributedDataAddForm(want);
            if (errorCode != 0) {
                GTEST_LOG_(INFO) << "add form failed, iCount:" << iCount << ", errorCode:" << errorCode;
            }
            sleep(1);
        }
    }

    if (selfStarting.addFormStatus) {
        for (int iCount = ADD_FORM_A_NUMBER; iCount < ADD_FORM_LENGTH; iCount++) {
            Want want;
            want.SetParam(Constants::PARAM_FORM_DIMENSION_KEY, FORM_DIMENSION_1);
            want.SetParam(Constants::PARAM_FORM_NAME_KEY, PARAM_FORM_NAME2);
            want.SetParam(Constants::PARAM_MODULE_NAME_KEY, PARAM_PROVIDER_MODULE_NAME2);
            want.SetParam(Constants::PARAM_FORM_TEMPORARY_KEY, FORM_TEMP_FORM_FLAG_TRUE);
            want.SetParam(Constants::PARAM_FORM_TEMPORARY_KEY, FORM_TEMP_FORM_FLAG_TRUE);
            want.SetElementName(FORM_TEST_DEVICEID, FORM_PROVIDER_BUNDLE_NAME2, FORM_PROVIDER_ABILITY_NAME2);
            APP_LOGI("%{public}s, formCount: %{public}d", __func__, iCount + 1);
            want.SetParam(Constants::PARAM_FORM_ADD_COUNT, iCount + 1);
            // Set Want info end
            int errorCode = STtools::SystemTestFormUtil::DistributedDataAddForm(want);
            if (errorCode != 0) {
                GTEST_LOG_(INFO) << "add form failed, iCount:" << iCount << ", errorCode:" << errorCode;
            }
            sleep(1);
        }
    }
}

bool FmsSelfStartingTest::CompareA()
{
    bool compare = true;
    for (int iCount = 0; iCount < ADD_FORM_A_NUMBER; iCount++) {
        int64_t formId = iCount + 1;
        InnerFormInfo innerFormInfo;
        selfStartingParser.GetStorageFormInfoById(std::to_string(formId), innerFormInfo);
        if (innerFormInfo.GetFormId() != formId) {
            compare = false;
            break;
        }
        if (innerFormInfo.GetModuleName() != PARAM_PROVIDER_MODULE_NAME1) {
            compare = false;
            break;
        }
        if (innerFormInfo.GetBundleName() != FORM_PROVIDER_BUNDLE_NAME1) {
            compare = false;
            break;
        }
        if (innerFormInfo.GetAbilityName() != FORM_PROVIDER_ABILITY_NAME1) {
            compare = false;
            break;
        }
        if (innerFormInfo.GetFormName() != PARAM_FORM_NAME1) {
            compare = false;
            break;
        }
    }
    return compare;
}

bool FmsSelfStartingTest::CompareB()
{
    bool compare = true;
    for (int iCount = ADD_FORM_A_NUMBER; iCount < ADD_FORM_LENGTH; iCount++) {
        int64_t formId = iCount + 1;
        InnerFormInfo innerFormInfo;
        selfStartingParser.GetStorageFormInfoById(std::to_string(formId), innerFormInfo);
        if (innerFormInfo.GetFormId() != formId) {
            compare = false;
            break;
        }
        if (innerFormInfo.GetModuleName() != PARAM_PROVIDER_MODULE_NAME2) {
            compare = false;
            break;
        }
        if (innerFormInfo.GetBundleName() != FORM_PROVIDER_BUNDLE_NAME2) {
            compare = false;
            break;
        }
        if (innerFormInfo.GetAbilityName() != FORM_PROVIDER_ABILITY_NAME2) {
            compare = false;
            break;
        }
        if (innerFormInfo.GetFormName() != PARAM_FORM_NAME2) {
            compare = false;
            break;
        }
    }
    return compare;
}

void FmsSelfStartingTest::TearDownTestCase()
{
    GTEST_LOG_(INFO) << "UnSubscribeCommonEvent calld";
    CommonEventManager::UnSubscribeCommonEvent(subscriber_);
    if (selfStarting.deleteFormStatus) {
        for (int iCount = 0; iCount < ADD_FORM_A_NUMBER; iCount++) {
            int64_t formId = iCount + 1;
            int errorCode = STtools::SystemTestFormUtil::DistributedDataDeleteForm(std::to_string(formId));
            if (errorCode != 0) {
                GTEST_LOG_(INFO) << "delete form failed, iCount:" << iCount << ", errorCode:" << errorCode;
            }
        }
    }

    if (selfStarting.deleteFormStatus) {
        for (int iCount = ADD_FORM_A_NUMBER; iCount < ADD_FORM_LENGTH; iCount++) {
            int64_t formId = iCount + 1;
            int errorCode = STtools::SystemTestFormUtil::DistributedDataDeleteForm(std::to_string(formId));
            if (errorCode != 0) {
                GTEST_LOG_(INFO) << "delete form failed, iCount:" << ", errorCode:" << errorCode;
            }
        }
        selfStartingParser.ClearStorage();
    }
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
 * @tc.number: FMS_Start_0300_03
 * @tc.name: Form number 512
 * @tc.desc:
 */
HWTEST_F(FmsSelfStartingTest, FMS_Start_0300_03, Function | MediumTest | Level1)
{
    std::cout << "START FMS_Start_0300_03" << std::endl;

    if (selfStarting.compareStatus) {
        std::ifstream opbefore("/data/formmgr/beforeKill.txt");
        std::ifstream opafter("/data/formmgr/afterKill.txt");
        std::string beforeKill;
        std::string afterKill;
        while (!opbefore.eof()) {
            beforeKill += opbefore.get();
        }
        while (!opafter.eof()) {
            afterKill += opafter.get();
        }
        opbefore.close();
        opafter.close();
        EXPECT_TRUE(beforeKill != afterKill);
 
        EXPECT_TRUE(CompareA());

        EXPECT_TRUE(CompareB());
    }
    std::cout << "END FMS_Start_0300_03" << std::endl;
}
}  // namespace AppExecFwk
}  // namespace OHOS