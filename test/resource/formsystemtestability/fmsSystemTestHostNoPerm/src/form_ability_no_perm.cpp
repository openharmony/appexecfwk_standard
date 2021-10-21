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
#include "form_ability_no_perm.h"
#include "app_log_wrapper.h"
#include "form_st_common_info.h"
#include "form_test_utils.h"

namespace {
using namespace OHOS::AAFwk;
using namespace OHOS::EventFwk;
}

namespace OHOS {
namespace AppExecFwk {
void FormAbilityNoPerm::AcquireFormCallback::OnAcquired(const int32_t result, const FormJsInfo &formJsInfo) const
{
    APP_LOGI("%{public}s called", __func__);
}
void FormAbilityNoPerm::AcquireFormCallback::OnUpdate(const int32_t result, const FormJsInfo &formJsInfo) const
{
    APP_LOGI("%{public}s called", __func__);
}
void FormAbilityNoPerm::AcquireFormCallback::OnFormUninstall(const int64_t formId) const
{
    APP_LOGI("%{public}s called", __func__);
}

void FormAbilityNoPerm::FMS_acquireForm_0200()
{
    std::shared_ptr<AcquireFormCallback> callback = std::make_shared<AcquireFormCallback>("FMS_acquireForm_0200");
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
        FormTestUtils::PublishEvent(FORM_EVENT_RECV_ACQUIRE_FORM_0200, EVENT_CODE_200, "false");
    }
}

void FormAbilityNoPerm::FMS_deleteForm_0200()
{
    int64_t formId = 1;
    bool bResult = DeleteForm(formId);
    if (bResult) {
        APP_LOGI("[FMS_deleteForm_0200] end");
    } else {
        APP_LOGE("[FMS_deleteForm_0200] error");
        FormTestUtils::PublishEvent(FORM_EVENT_RECV_DELETE_FORM_0200, EVENT_CODE_200, "false");
    }
}

void FormAbilityNoPerm::FMS_releaseForm_0200()
{
    int64_t formId = 1;
    bool isReleaseCache = true;
    bool bResult = ReleaseForm(formId, isReleaseCache);
    if (bResult) {
        APP_LOGI("[FMS_releaseForm_0200] end");
    } else {
        APP_LOGE("[FMS_releaseForm_0200] error");
        FormTestUtils::PublishEvent(FORM_EVENT_RECV_RELEASE_FORM_0200, EVENT_CODE_200, "false");
    }
}


void FormAbilityNoPerm::OnStart(const Want &want)
{
    APP_LOGI("FormAbilityNoPerm::onStart");
    Ability::OnStart(want);

    std::vector<std::string> eventList = {
        FORM_EVENT_REQ_ACQUIRE_FORM_0200,
        FORM_EVENT_REQ_DELETE_FORM_0200,
        FORM_EVENT_REQ_RELEASE_FORM_0200,
    };
    SubscribeEvent(eventList);
}
void FormAbilityNoPerm::OnActive()
{
    APP_LOGI("FormAbilityNoPerm::OnActive");
    Ability::OnActive();
    std::string eventData = GetAbilityName() + FORM_ABILITY_STATE_ONACTIVE;
    FormTestUtils::PublishEvent(FORM_EVENT_ABILITY_ONACTIVED, 0, eventData);
}

void FormAbilityNoPerm::OnStop()
{
    APP_LOGI("FormAbilityNoPerm::OnStop");

    Ability::OnStop();
}
void FormAbilityNoPerm::OnInactive()
{
    APP_LOGI("FormAbilityNoPerm::OnInactive");

    Ability::OnInactive();
}
void FormAbilityNoPerm::OnBackground()
{
    APP_LOGI("FormAbilityNoPerm::OnBackground");

    Ability::OnBackground();
}
void FormAbilityNoPerm::SubscribeEvent(const std::vector<std::string> &eventList)
{
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
void FormAbilityNoPerm::Init(const std::shared_ptr<AbilityInfo> &abilityInfo,
    const std::shared_ptr<OHOSApplication> &application, std::shared_ptr<AbilityHandler> &handler,
    const sptr<IRemoteObject> &token)
{
    APP_LOGI("FormAbilityNoPerm::Init");
    Ability::Init(abilityInfo, application, handler, token);

    memberFuncMap_[FORM_EVENT_REQ_ACQUIRE_FORM_0200] = &FormAbilityNoPerm::FMS_acquireForm_0200;
    memberFuncMap_[FORM_EVENT_REQ_DELETE_FORM_0200] = &FormAbilityNoPerm::FMS_deleteForm_0200;
    memberFuncMap_[FORM_EVENT_REQ_RELEASE_FORM_0200] = &FormAbilityNoPerm::FMS_releaseForm_0200;
}

void FormAbilityNoPerm::handleEvent(std::string action)
{
    auto itFunc = memberFuncMap_.find(action);
    if (itFunc != memberFuncMap_.end()) {
        auto memberFunc = itFunc->second;
        if (memberFunc != nullptr) {
            return (this->*memberFunc)();
        }
    }
}

void FormAbilityNoPerm::Clear()
{
}

void FormEventSubscriber::OnReceiveEvent(const CommonEventData &data)
{
    APP_LOGI("FormEventSubscriber::OnReceiveEvent:event=%{public}s", data.GetWant().GetAction().c_str());
    APP_LOGI("FormEventSubscriber::OnReceiveEvent:code=%{public}d", data.GetCode());
    auto eventName = data.GetWant().GetAction();
    ability_->handleEvent(eventName);
}

void FormEventSubscriber::KitTerminateAbility()
{
    if (ability_ != nullptr) {
        ability_->TerminateAbility();
    }
}

REGISTER_AA(FormAbilityNoPerm)
}  // namespace AppExecFwk
}  // namespace OHOS