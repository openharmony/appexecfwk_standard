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
#include "form_ability_b.h"
#include "app_log_wrapper.h"
#include "form_st_common_info.h"
#include "form_test_utils.h"
#include "system_test_form_util.h"

namespace {
using namespace OHOS::AAFwk;
using namespace OHOS::EventFwk;
}

namespace OHOS {
namespace AppExecFwk {
std::vector<std::string> eventList = {
    FORM_EVENT_REQ_DELETE_FORM_COMMON, FORM_EVENT_REQ_ACQUIRE_FORM_TEMP, FORM_EVENT_REQ_ACQUIRE_FORM_1400,
    FORM_EVENT_REQ_ACQUIRE_FORM_1500, FORM_EVENT_REQ_ACQUIRE_FORM_1800_2, FORM_EVENT_REQ_ACQUIRE_FORM_1800_3,
    FORM_EVENT_REQ_ACQUIRE_FORM_2400_1, FORM_EVENT_REQ_ACQUIRE_FORM_3100, FORM_EVENT_REQ_ACQUIRE_FORM_BATCH,
    FORM_EVENT_REQ_ACQUIRE_TEMP_FORM_BATCH,
};
void FormAbilityB::AcquireFormCallback::OnAcquired(const int32_t result, const FormJsInfo &formJsInfo) const
{
    APP_LOGI("%{public}s called", __func__);
    FormTestUtils::PublishEvent(this->caseName_, this->code_, std::to_string(formJsInfo.formId));
    if (this->caseName_ == FORM_EVENT_RECV_ACQUIRE_FORM_1500) {
        std::string strFormId = std::to_string(formJsInfo.formId);
        APP_LOGI("%{public}s, delete form, formId: %{public}s", __func__, strFormId.c_str());
        ability_->FMS_deleteFormCommon(strFormId);
    }
}
void FormAbilityB::AcquireFormCallback::OnUpdate(const int32_t result, const FormJsInfo &formJsInfo) const
{
    APP_LOGI("%{public}s called", __func__);
    FormTestUtils::PublishEvent(this->caseName_, this->code_ + 1, formJsInfo.formData);
    if (this->caseName_ == FORM_EVENT_RECV_ACQUIRE_FORM_1800_2 ||
        this->caseName_ == FORM_EVENT_RECV_ACQUIRE_FORM_1800_3) {
        std::string strFormId = std::to_string(formJsInfo.formId);
        APP_LOGI("%{public}s, delete form, formId: %{public}s", __func__, strFormId.c_str());
        ability_->FMS_deleteFormCommon(strFormId);
    }
}

void FormAbilityB::AcquireFormCallback::OnFormUninstall(const int64_t formId) const
{
    APP_LOGI("%{public}s called", __func__);
}
void FormAbilityB::FMS_deleteFormCommon(std::string strFormId)
{
    APP_LOGI("%{public}s called, formId: %{public}s", __func__, strFormId.c_str());
    if (strFormId.empty()) {
        APP_LOGE("DeleteForm error, formId is 0");
        FormTestUtils::PublishEvent(FORM_EVENT_RECV_DELETE_FORM_COMMON, EVENT_CODE_999, "false");
        return;
    }
    int64_t formId = std::stoll(strFormId);
    sleep(1);
    bool bResult = DeleteForm(formId);
    sleep(1);
    if (bResult) {
        APP_LOGI("DeleteForm end");
        FormTestUtils::PublishEvent(FORM_EVENT_RECV_DELETE_FORM_COMMON, EVENT_CODE_999, "true");
    } else {
        APP_LOGE("DeleteForm error");
        FormTestUtils::PublishEvent(FORM_EVENT_RECV_DELETE_FORM_COMMON, EVENT_CODE_999, "false");
    }
}

void FormAbilityB::FMS_acquireForm_1400(std::string data)
{
    APP_LOGI("%{public}s called", __func__);
    std::shared_ptr<AcquireFormCallback> callback =
        std::make_shared<AcquireFormCallback>(FORM_EVENT_RECV_ACQUIRE_FORM_1400, EVENT_CODE_1400);
    // Set Want info begin
    std::string formName = "formName999";
    Want want;
    want.SetParam(Constants::PARAM_FORM_DIMENSION_KEY, FORM_DIMENSION_1);
    want.SetParam(Constants::PARAM_FORM_NAME_KEY, formName);
    want.SetParam(Constants::PARAM_MODULE_NAME_KEY, PARAM_PROVIDER_MODULE_NAME1);
    want.SetParam(Constants::PARAM_FORM_TEMPORARY_KEY, FORM_TEMP_FORM_FLAG_FALSE);
    want.SetElementName(FORM_TEST_DEVICEID, FORM_PROVIDER_BUNDLE_NAME1, FORM_PROVIDER_ABILITY_NAME1);
    // Set Want info end
    int64_t formId = std::stoll(data);
    bool bResult = AcquireForm(formId, want, callback);
    if (bResult) {
        APP_LOGI("AcquireForm end");
    } else {
        APP_LOGE("AcquireForm error");
        FormTestUtils::PublishEvent(FORM_EVENT_RECV_ACQUIRE_FORM_1400, EVENT_CODE_1400, "false");
    }
}
void FormAbilityB::FMS_acquireForm_1500(std::string data)
{
    APP_LOGI("%{public}s called", __func__);
    std::shared_ptr<AcquireFormCallback> callback =
        std::make_shared<AcquireFormCallback>(FORM_EVENT_RECV_ACQUIRE_FORM_1500, EVENT_CODE_1500);
    callback->ability_ = this;
    // Set Want info begin
    Want want;
    want.SetParam(Constants::PARAM_FORM_DIMENSION_KEY, FORM_DIMENSION_1);
    want.SetParam(Constants::PARAM_FORM_NAME_KEY, PARAM_FORM_NAME1);
    want.SetParam(Constants::PARAM_MODULE_NAME_KEY, PARAM_PROVIDER_MODULE_NAME1);
    want.SetParam(Constants::PARAM_FORM_TEMPORARY_KEY, FORM_TEMP_FORM_FLAG_FALSE);
    want.SetElementName(FORM_TEST_DEVICEID, FORM_PROVIDER_BUNDLE_NAME1, FORM_PROVIDER_ABILITY_NAME1);
    // Set Want info end
    int64_t formId = std::stoll(data);
    bool bResult = AcquireForm(formId, want, callback);
    if (bResult) {
        APP_LOGI("AcquireForm end");
    } else {
        APP_LOGE("AcquireForm error");
        FormTestUtils::PublishEvent(FORM_EVENT_RECV_ACQUIRE_FORM_1500, EVENT_CODE_1500, "");
    }
}

void FormAbilityB::FMS_acquireForm_1800_2(std::string data)
{
    APP_LOGI("%{public}s called", __func__);
    std::shared_ptr<AcquireFormCallback> callback =
        std::make_shared<AcquireFormCallback>(FORM_EVENT_RECV_ACQUIRE_FORM_1800_2, EVENT_CODE_1820);
    callback->ability_ = this;
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
        FormTestUtils::PublishEvent(FORM_EVENT_RECV_ACQUIRE_FORM_1800_2, EVENT_CODE_1820, "");
    }
}
void FormAbilityB::FMS_acquireForm_1800_3(std::string data)
{
    APP_LOGI("%{public}s called", __func__);
    std::shared_ptr<AcquireFormCallback> callback =
        std::make_shared<AcquireFormCallback>(FORM_EVENT_RECV_ACQUIRE_FORM_1800_3, EVENT_CODE_1830);
    callback->ability_ = this;
    // Set Want info begin
    Want want;
    want.SetParam(Constants::PARAM_FORM_DIMENSION_KEY, FORM_DIMENSION_1);
    want.SetParam(Constants::PARAM_FORM_NAME_KEY, PARAM_FORM_NAME1);
    want.SetParam(Constants::PARAM_MODULE_NAME_KEY, PARAM_PROVIDER_MODULE_NAME1);
    want.SetParam(Constants::PARAM_FORM_TEMPORARY_KEY, FORM_TEMP_FORM_FLAG_TRUE);
    want.SetElementName(FORM_TEST_DEVICEID, FORM_PROVIDER_BUNDLE_NAME1, FORM_PROVIDER_ABILITY_NAME1);
    // Set Want info end
    bool bResult = AcquireForm(0, want, callback);
    if (bResult) {
        APP_LOGI("AcquireForm end");
    } else {
        APP_LOGE("AcquireForm error");
        FormTestUtils::PublishEvent(FORM_EVENT_RECV_ACQUIRE_FORM_1800_3, EVENT_CODE_1830, "");
    }
}

void FormAbilityB::FMS_acquireForm_2400_1(std::string data)
{
    APP_LOGI("%{public}s called", __func__);
    std::shared_ptr<AcquireFormCallback> callback =
        std::make_shared<AcquireFormCallback>(FORM_EVENT_RECV_ACQUIRE_FORM_2400_1, EVENT_CODE_2410);
    callback->ability_ = this;
    // Set Want info end
    int64_t formId = std::stoll(data);
    bool bResult = CastTempForm(formId);
    if (bResult) {
        APP_LOGI("CastTempForm end");
    } else {
        APP_LOGE("CastTempForm error");
        FormTestUtils::PublishEvent(FORM_EVENT_RECV_ACQUIRE_FORM_2400_1, EVENT_CODE_2410, "false");
    }
}
void FormAbilityB::FMS_acquireForm_3100(std::string data)
{
    APP_LOGI("%{public}s called", __func__);
    std::shared_ptr<AcquireFormCallback> callback =
        std::make_shared<AcquireFormCallback>(FORM_EVENT_RECV_ACQUIRE_FORM_3100, EVENT_CODE_3100);
    callback->ability_ = this;
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
        FormTestUtils::PublishEvent(FORM_EVENT_RECV_ACQUIRE_FORM_3100, EVENT_CODE_3100, "");
    }
}
void FormAbilityB::FMS_acquireForm_tempForm(std::string data)
{
    APP_LOGI("%{public}s called", __func__);
    std::shared_ptr<AcquireFormCallback> callback =
        std::make_shared<AcquireFormCallback>(FORM_EVENT_RECV_ACQUIRE_FORM_TEMP, EVENT_CODE_TEMP);

    Want want;
    want.SetParam(Constants::PARAM_FORM_DIMENSION_KEY, FORM_DIMENSION_1);
    want.SetParam(Constants::PARAM_FORM_NAME_KEY, PARAM_FORM_NAME1);
    want.SetParam(Constants::PARAM_MODULE_NAME_KEY, PARAM_PROVIDER_MODULE_NAME1);
    want.SetParam(Constants::PARAM_FORM_TEMPORARY_KEY, FORM_TEMP_FORM_FLAG_TRUE);
    want.SetElementName(FORM_TEST_DEVICEID, FORM_PROVIDER_BUNDLE_NAME1, FORM_PROVIDER_ABILITY_NAME1);

    // Set Want info end
    bool bResult = AcquireForm(0, want, callback);
    if (bResult) {
        APP_LOGI("AcquireForm end");
    } else {
        APP_LOGE("AcquireForm error");
        FormTestUtils::PublishEvent(FORM_EVENT_RECV_ACQUIRE_FORM_TEMP, EVENT_CODE_TEMP, "");
    }
}

void FormAbilityB::FMS_acquireFormBatch(std::string data)
{
    APP_LOGI("%{public}s called", __func__);
    // Set Want info begin
    Want want;
    want.SetParam(Constants::PARAM_FORM_DIMENSION_KEY, FORM_DIMENSION_1);
    want.SetParam(Constants::PARAM_FORM_NAME_KEY, PARAM_FORM_NAME1);
    want.SetParam(Constants::PARAM_MODULE_NAME_KEY, PARAM_PROVIDER_MODULE_NAME1);
    want.SetParam(Constants::PARAM_FORM_TEMPORARY_KEY, FORM_TEMP_FORM_FLAG_FALSE);
    want.SetElementName(FORM_TEST_DEVICEID, FORM_PROVIDER_BUNDLE_NAME1, FORM_PROVIDER_ABILITY_NAME1);

    int formCount = std::stoi(data);
    APP_LOGI("%{public}s, formCount: %{public}d", __func__, formCount);
    want.SetParam(Constants::PARAM_FORM_ADD_COUNT, formCount);
    // Set Want info end
    int result = STtools::SystemTestFormUtil::BatchAddFormRecords(want);
    if (result == ERR_OK) {
        APP_LOGI("Batch add form end");
        FormTestUtils::PublishEvent(FORM_EVENT_RECV_ACQUIRE_FORM_BATCH, EVENT_CODE_BATCH, "true");
    } else {
        APP_LOGE("Batch add form error");
        FormTestUtils::PublishEvent(FORM_EVENT_RECV_ACQUIRE_FORM_BATCH, EVENT_CODE_BATCH, "false");
    }
}

void FormAbilityB::FMS_acquireFormTempBatch(std::string data)
{
    APP_LOGI("%{public}s called", __func__);
    // Set Want info begin
    Want want;
    want.SetParam(Constants::PARAM_FORM_DIMENSION_KEY, FORM_DIMENSION_1);
    want.SetParam(Constants::PARAM_FORM_NAME_KEY, PARAM_FORM_NAME1);
    want.SetParam(Constants::PARAM_MODULE_NAME_KEY, PARAM_PROVIDER_MODULE_NAME1);
    want.SetParam(Constants::PARAM_FORM_TEMPORARY_KEY, FORM_TEMP_FORM_FLAG_FALSE);
    want.SetParam(Constants::PARAM_FORM_TEMPORARY_KEY, FORM_TEMP_FORM_FLAG_TRUE);
    want.SetElementName(FORM_TEST_DEVICEID, FORM_PROVIDER_BUNDLE_NAME1, FORM_PROVIDER_ABILITY_NAME1);

    int formCount = std::stoi(data);
    APP_LOGI("%{public}s, formCount: %{public}d", __func__, formCount);
    want.SetParam(Constants::PARAM_FORM_ADD_COUNT, formCount);
    // Set Want info end
    int result = STtools::SystemTestFormUtil::BatchAddFormRecords(want);
    if (result == ERR_OK) {
        APP_LOGI("Batch add temp form end");
        FormTestUtils::PublishEvent(FORM_EVENT_RECV_ACQUIRE_TEMP_FORM_BATCH, EVENT_CODE_TEMP_BATCH, "true");
    } else {
        APP_LOGE("Batch add temp form error");
        FormTestUtils::PublishEvent(FORM_EVENT_RECV_ACQUIRE_TEMP_FORM_BATCH, EVENT_CODE_TEMP_BATCH, "false");
    }
}
FormAbilityB::~FormAbilityB()
{
    CommonEventManager::UnSubscribeCommonEvent(subscriber_);
}
void FormAbilityB::OnStart(const Want &want)
{
    APP_LOGI("FormAbilityB::onStart");
    Ability::OnStart(want);
}
void FormAbilityB::OnActive()
{
    APP_LOGI("FormAbilityB::OnActive");
    Ability::OnActive();
    std::string eventData = GetAbilityName() + FORM_ABILITY_STATE_ONACTIVE;
    FormTestUtils::PublishEvent(FORM_EVENT_ABILITY_ONACTIVED, 0, eventData);
}

void FormAbilityB::OnStop()
{
    APP_LOGI("FormAbilityB::OnStop");

    Ability::OnStop();
}
void FormAbilityB::OnInactive()
{
    APP_LOGI("FormAbilityB::OnInactive");

    Ability::OnInactive();
}
void FormAbilityB::OnBackground()
{
    APP_LOGI("FormAbilityB::OnBackground");

    Ability::OnBackground();
}
void FormAbilityB::SubscribeEvent()
{
    APP_LOGI("FormAbilityB::SubscribeEvent");
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
void FormAbilityB::Init(const std::shared_ptr<AbilityInfo> &abilityInfo,
    const std::shared_ptr<OHOSApplication> &application, std::shared_ptr<AbilityHandler> &handler,
    const sptr<IRemoteObject> &token)
{
    APP_LOGI("FormAbilityB::Init");
    Ability::Init(abilityInfo, application, handler, token);
    memberFuncMap_[FORM_EVENT_REQ_DELETE_FORM_COMMON] = &FormAbilityB::FMS_deleteFormCommon;
    memberFuncMap_[FORM_EVENT_REQ_ACQUIRE_FORM_TEMP] = &FormAbilityB::FMS_acquireForm_tempForm;
    memberFuncMap_[FORM_EVENT_REQ_ACQUIRE_FORM_BATCH] = &FormAbilityB::FMS_acquireFormBatch;
    memberFuncMap_[FORM_EVENT_REQ_ACQUIRE_TEMP_FORM_BATCH] = &FormAbilityB::FMS_acquireFormTempBatch;
    memberFuncMap_[FORM_EVENT_REQ_ACQUIRE_FORM_1400] = &FormAbilityB::FMS_acquireForm_1400;
    memberFuncMap_[FORM_EVENT_REQ_ACQUIRE_FORM_1500] = &FormAbilityB::FMS_acquireForm_1500;
    memberFuncMap_[FORM_EVENT_REQ_ACQUIRE_FORM_1800_2] = &FormAbilityB::FMS_acquireForm_1800_2;
    memberFuncMap_[FORM_EVENT_REQ_ACQUIRE_FORM_1800_3] = &FormAbilityB::FMS_acquireForm_1800_3;
    memberFuncMap_[FORM_EVENT_REQ_ACQUIRE_FORM_2400_1] = &FormAbilityB::FMS_acquireForm_2400_1;
    memberFuncMap_[FORM_EVENT_REQ_ACQUIRE_FORM_3100] = &FormAbilityB::FMS_acquireForm_3100;
    SubscribeEvent();
}

void FormAbilityB::handleEvent(std::string action, std::string data)
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

void FormAbilityB::Clear()
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

REGISTER_AA(FormAbilityB)
}  // namespace AppExecFwk
}  // namespace OHOS