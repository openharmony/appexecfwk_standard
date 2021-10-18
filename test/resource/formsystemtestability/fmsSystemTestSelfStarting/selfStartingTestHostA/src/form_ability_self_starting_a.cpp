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

#include "form_ability_self_starting_a.h"
#include "app_log_wrapper.h"
#include "form_st_common_info.h"
#include "form_test_utils.h"

namespace {
using namespace OHOS::AAFwk;
using namespace OHOS::EventFwk;
}

namespace OHOS {
namespace AppExecFwk {
std::vector<std::string> eventList = {
    FORM_EVENT_REQ_SELF_STARTING_TEST_0100,
};
void FormAbilitySelfStartingA::SelfStartingCallback::OnAcquired(const int32_t result,
    const FormJsInfo &formJsInfo) const
{
    APP_LOGI("%{public}s called", __func__);
}
void FormAbilitySelfStartingA::SelfStartingCallback::OnUpdate(const int32_t result, const FormJsInfo &formJsInfo) const
{
    APP_LOGI("%{public}s called", __func__);
    APP_LOGI("%{public}s called, caseName_: %{public}s, code_: %{public}d", __func__, this->caseName_.c_str(),
        this->code_);
    FormTestUtils::PublishEvent(this->caseName_, this->code_, "true");
}

void FormAbilitySelfStartingA::SelfStartingCallback::OnFormUninstall(const int64_t formId) const
{
    APP_LOGI("%{public}s called", __func__);
}

void FormAbilitySelfStartingA::FMS_Start_0300_01(std::string data)
{
    APP_LOGI("%{public}s called", __func__);
    std::shared_ptr<SelfStartingCallback> callback =
        std::make_shared<SelfStartingCallback>(FORM_EVENT_RECV_SELF_STARTING_TEST_0100, EVENT_CODE_100);
    // Set Want info begin
    Want want;
    want.SetParam(Constants::PARAM_FORM_DIMENSION_KEY, FORM_DIMENSION_1);
    want.SetParam(Constants::PARAM_FORM_NAME_KEY, PARAM_FORM_NAME1);
    want.SetParam(Constants::PARAM_MODULE_NAME_KEY, PARAM_PROVIDER_MODULE_NAME1);
    want.SetParam(Constants::PARAM_FORM_TEMPORARY_KEY, FORM_TEMP_FORM_FLAG_FALSE);
    want.SetElementName(FORM_TEST_DEVICEID, FORM_PROVIDER_BUNDLE_NAME1, FORM_PROVIDER_ABILITY_NAME1);
    // Set Want info end
    bool bResult = AcquireForm(0, want, callback);
    if (bResult) {
        APP_LOGI("AcquireForm end");
    } else {
        APP_LOGE("AcquireForm error");
        FormTestUtils::PublishEvent(FORM_EVENT_RECV_SELF_STARTING_TEST_0100, EVENT_CODE_100, "false");
    }
}

FormAbilitySelfStartingA::~FormAbilitySelfStartingA()
{
    CommonEventManager::UnSubscribeCommonEvent(subscriber_);
}

void FormAbilitySelfStartingA::OnStart(const Want &want)
{
    APP_LOGI("FormAbilitySelfStartingA::onStart");
    Ability::OnStart(want);
}
void FormAbilitySelfStartingA::OnActive()
{
    APP_LOGI("FormAbilitySelfStartingA::OnActive");
    Ability::OnActive();
    std::string eventData = GetAbilityName() + FORM_ABILITY_STATE_ONACTIVE;
    FormTestUtils::PublishEvent(FORM_EVENT_ABILITY_ONACTIVED, 0, eventData);
}

void FormAbilitySelfStartingA::OnStop()
{
    APP_LOGI("FormAbilitySelfStartingA::OnStop");

    Ability::OnStop();
}
void FormAbilitySelfStartingA::OnInactive()
{
    APP_LOGI("FormAbilitySelfStartingA::OnInactive");

    Ability::OnInactive();
}
void FormAbilitySelfStartingA::OnBackground()
{
    APP_LOGI("FormAbilitySelfStartingA::OnBackground");

    Ability::OnBackground();
}
void FormAbilitySelfStartingA::SubscribeEvent()
{
    APP_LOGI("FormAbilitySelfStartingA::SubscribeEvent");
    MatchingSkills matchingSkills;
    for (const auto &e : eventList) {
        matchingSkills.AddEvent(e);
    }
    CommonEventSubscribeInfo subscribeInfo(matchingSkills);
    subscribeInfo.SetPriority(1);
    subscriber_ = std::make_shared<FormEventSubscriber>(subscribeInfo);
    subscriber_->ability_ = this;
    CommonEventManager::SubscribeCommonEvent(subscriber_);
}

// KitTest End
void FormAbilitySelfStartingA::Init(const std::shared_ptr<AbilityInfo> &abilityInfo,
    const std::shared_ptr<OHOSApplication> &application, std::shared_ptr<AbilityHandler> &handler,
    const sptr<IRemoteObject> &token)
{
    APP_LOGI("FormAbilitySelfStartingA::Init");
    Ability::Init(abilityInfo, application, handler, token);
    memberFuncMap_[FORM_EVENT_REQ_SELF_STARTING_TEST_0100] = &FormAbilitySelfStartingA::FMS_Start_0300_01;

    SubscribeEvent();
}

void FormAbilitySelfStartingA::handleEvent(std::string action, std::string data)
{
    APP_LOGI("%{public}s called", __func__);
    if (calledFuncMap_.find(action) != calledFuncMap_.end()) {
        return;
    }
    calledFuncMap_.emplace(action, 0);
    auto itFunc = memberFuncMap_.find(action);
    if (itFunc != memberFuncMap_.end()) {
        auto memberFunc = itFunc->second;
        if (memberFunc != nullptr) {
            return (this->*memberFunc)(data);
        }
    }
}

void FormAbilitySelfStartingA::Clear()
{
}

void FormEventSubscriber::OnReceiveEvent(const CommonEventData &data)
{
    APP_LOGI("FormEventSubscriber::OnReceiveEvent:event=%{public}s", data.GetWant().GetAction().c_str());
    APP_LOGI("KitTestEventSubscriber::OnReceiveEvent:data=%{public}s", data.GetData().c_str());
    APP_LOGI("FormEventSubscriber::OnReceiveEvent:code=%{public}d", data.GetCode());
    auto eventName = data.GetWant().GetAction();
    ability_->handleEvent(eventName, data.GetData());
    
    CommonEventManager::UnSubscribeCommonEvent(ability_->subscriber_);
}

void FormEventSubscriber::KitTerminateAbility()
{
    if (ability_ != nullptr) {
        ability_->TerminateAbility();
    }
}

REGISTER_AA(FormAbilitySelfStartingA)
}  // namespace AppExecFwk
}  // namespace OHOS