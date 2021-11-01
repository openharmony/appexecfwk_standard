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
#include <iostream>
#include <numeric>
#include <sstream>

#include "form_ability_performance.h"
#include "app_log_wrapper.h"
#include "form_st_common_info.h"
#include "form_test_utils.h"

namespace {
    using namespace OHOS::AAFwk;
    using namespace OHOS::EventFwk;
}

namespace OHOS {
namespace AppExecFwk {
const int ONE = 1;
const int TWO = 2;
std::vector<std::string> eventList = {
    FORM_EVENT_REQ_PERFORMANCE_TEST_0100,
    FORM_EVENT_REQ_PERFORMANCE_TEST_0200,
    FORM_EVENT_REQ_PERFORMANCE_TEST_0300,
    FORM_EVENT_REQ_PERFORMANCE_TEST_0400,
    FORM_EVENT_REQ_PERFORMANCE_TEST_0500,
    FORM_EVENT_REQ_PERFORMANCE_TEST_0600,
    FORM_EVENT_REQ_PERFORMANCE_TEST_0700,
    FORM_EVENT_REQ_PERFORMANCE_TEST_1300,
    FORM_EVENT_REQ_PERFORMANCE_TEST_1400,
    FORM_EVENT_REQ_PERFORMANCE_TEST_1500,
    FORM_EVENT_REQ_PERFORMANCE_TEST_1600,
};

static std::string g_formId = 0;
static std::string g_bundleName = "com.form.formsystemtestservicea";
static std::string g_moduleName = "formmodule001";
void FormAbilityPerformance::PerformanceFormCallback::OnAcquired(
    const int32_t result, const FormJsInfo &formJsInfo) const
{
    APP_LOGI("%{public}s called", __func__);
    APP_LOGI("%{public}s formId: %{public}s", __func__, std::to_string(formJsInfo.formId).c_str());
    APP_LOGI("%{public}s bundleName: %{public}s", __func__, formJsInfo.bundleName.c_str());
    APP_LOGI("%{public}s abilityName: %{public}s", __func__, formJsInfo.abilityName.c_str());
    APP_LOGI("%{public}s formName: %{public}s", __func__, formJsInfo.formName.c_str());
    APP_LOGI("%{public}s formTempFlg: %{public}d", __func__, formJsInfo.formTempFlg);
    APP_LOGI("%{public}s jsFormCodePath: %{public}s", __func__, formJsInfo.jsFormCodePath.c_str());
    APP_LOGI("%{public}s formData: %{public}s", __func__, formJsInfo.formData.c_str());
    APP_LOGI("%{public}s formProviderData GetDataString: %{public}s",
        __func__, formJsInfo.formProviderData.GetDataString().c_str());

    g_formId = std::to_string(formJsInfo.formId);

    if (this->caseName_ == FORM_EVENT_RECV_PERFORMANCE_TEST_0100) {
        FormTestUtils::PublishEvent(this->caseName_, EVENT_CODE_101, std::to_string(formJsInfo.formId));
    } else if (this->caseName_ == FORM_EVENT_RECV_PERFORMANCE_TEST_0200) {
        FormTestUtils::PublishEvent(this->caseName_, EVENT_CODE_201, std::to_string(formJsInfo.formId));
    } else if (this->caseName_ == FORM_EVENT_RECV_PERFORMANCE_TEST_0300) {
        FormTestUtils::PublishEvent(this->caseName_, EVENT_CODE_301, std::to_string(formJsInfo.formId));
    } else if (this->caseName_ == FORM_EVENT_RECV_PERFORMANCE_TEST_0400) {
        FormTestUtils::PublishEvent(this->caseName_, EVENT_CODE_401, std::to_string(formJsInfo.formId));
    } else if (this->caseName_ == FORM_EVENT_RECV_PERFORMANCE_TEST_0500) {
        FormTestUtils::PublishEvent(this->caseName_, EVENT_CODE_501, std::to_string(formJsInfo.formId));
    } else {
        FormTestUtils::PublishEvent(this->caseName_, this->code_ + ONE, std::to_string(formJsInfo.formId));
    }
}

void FormAbilityPerformance::PerformanceFormCallback::OnUpdate(const int32_t result, const FormJsInfo &formJsInfo) const
{
    APP_LOGI("%{public}s called", __func__);
    APP_LOGI("%{public}s formId: %{public}s", __func__, std::to_string(formJsInfo.formId).c_str());
    APP_LOGI("%{public}s bundleName: %{public}s", __func__, formJsInfo.bundleName.c_str());
    APP_LOGI("%{public}s abilityName: %{public}s", __func__, formJsInfo.abilityName.c_str());
    APP_LOGI("%{public}s formName: %{public}s", __func__, formJsInfo.formName.c_str());
    APP_LOGI("%{public}s formTempFlg: %{public}d", __func__, formJsInfo.formTempFlg);
    APP_LOGI("%{public}s jsFormCodePath: %{public}s", __func__, formJsInfo.jsFormCodePath.c_str());
    APP_LOGI("%{public}s formData: %{public}s", __func__, formJsInfo.formData.c_str());
    APP_LOGI("%{public}s formProviderData GetDataString: %{public}s",
        __func__, formJsInfo.formProviderData.GetDataString().c_str());

    if (this->caseName_ == FORM_EVENT_RECV_PERFORMANCE_TEST_0100) {
        FormTestUtils::PublishEvent(this->caseName_, EVENT_CODE_102, "true");
    } else if (this->caseName_ == FORM_EVENT_RECV_PERFORMANCE_TEST_0200) {
        FormTestUtils::PublishEvent(this->caseName_, EVENT_CODE_202, "true");
    } else if (this->caseName_ == FORM_EVENT_RECV_PERFORMANCE_TEST_0300) {
        FormTestUtils::PublishEvent(this->caseName_, EVENT_CODE_302, "true");
    } else if (this->caseName_ == FORM_EVENT_RECV_PERFORMANCE_TEST_0400) {
        FormTestUtils::PublishEvent(this->caseName_, EVENT_CODE_402, "true");
    } else if (this->caseName_ == FORM_EVENT_RECV_PERFORMANCE_TEST_0500) {
        FormTestUtils::PublishEvent(this->caseName_, EVENT_CODE_502, "true");
    } else {
        FormTestUtils::PublishEvent(this->caseName_, this->code_ + TWO, "true");
    }
}

void FormAbilityPerformance::PerformanceFormCallback::OnFormUninstall(const int64_t formId) const
{
    APP_LOGI("%{public}s called", __func__);
}

void FormAbilityPerformance::FMS_performanceTest_0100(std::string data)
{
    std::shared_ptr<PerformanceFormCallback> callback =
        std::make_shared<PerformanceFormCallback>(FORM_EVENT_RECV_PERFORMANCE_TEST_0100, EVENT_CODE_100);
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
        APP_LOGI("[FMS_performanceTest_0100] AcquireForm end");
    } else {
        APP_LOGE("[FMS_performanceTest_0100] AcquireForm error");
        FormTestUtils::PublishEvent(FORM_EVENT_RECV_PERFORMANCE_TEST_0100, EVENT_CODE_100, "false");
    }
}

void FormAbilityPerformance::FMS_performanceTest_0200(std::string data)
{
    bool bResult = DeleteForm(atoll(g_formId.c_str()));
    if (bResult) {
        APP_LOGI("[FMS_performanceTest_0200] DeleteForm end");
        FormTestUtils::PublishEvent(FORM_EVENT_RECV_PERFORMANCE_TEST_0200, EVENT_CODE_200, "true");
    } else {
        APP_LOGE("[FMS_performanceTest_0200] DeleteForm error");
        FormTestUtils::PublishEvent(FORM_EVENT_RECV_PERFORMANCE_TEST_0200, EVENT_CODE_200, "false");
    }
}

void FormAbilityPerformance::FMS_performanceTest_0300(std::string data)
{
    bool isReleaseCache = false;
    bool bResult = ReleaseForm(atoll(g_formId.c_str()), isReleaseCache);
    if (bResult) {
        APP_LOGI("[FMS_performanceTest_0300] ReleaseForm end");
        FormTestUtils::PublishEvent(FORM_EVENT_RECV_PERFORMANCE_TEST_0300, EVENT_CODE_300, "true");
    } else {
        APP_LOGE("[FMS_performanceTest_0300] ReleaseForm error");
        FormTestUtils::PublishEvent(FORM_EVENT_RECV_PERFORMANCE_TEST_0300, EVENT_CODE_300, "false");
    }
}

void FormAbilityPerformance::FMS_performanceTest_0400(std::string data)
{
    bool bResult = CastTempForm(atoll(g_formId.c_str()));
    if (bResult) {
        APP_LOGI("[FMS_performanceTest_0400] CastTempForm end");
        FormTestUtils::PublishEvent(FORM_EVENT_RECV_PERFORMANCE_TEST_0400, EVENT_CODE_400, "true");
    } else {
        APP_LOGE("[FMS_performanceTest_0400] CastTempForm error");
        FormTestUtils::PublishEvent(FORM_EVENT_RECV_PERFORMANCE_TEST_0400, EVENT_CODE_400, "false");
    }
}

void FormAbilityPerformance::FMS_performanceTest_0500(std::string data)
{
    std::vector<int64_t> formIds;
    formIds.push_back(atoll(g_formId.c_str()));
    bool bResult = NotifyVisibleForms(formIds);
    if (bResult) {
        APP_LOGI("[FMS_performanceTest_0500] NotifyVisibleForms end");
        FormTestUtils::PublishEvent(FORM_EVENT_RECV_PERFORMANCE_TEST_0500, EVENT_CODE_500, "true");
    } else {
        APP_LOGE("[FMS_performanceTest_0500] NotifyVisibleForms error");
        FormTestUtils::PublishEvent(FORM_EVENT_RECV_PERFORMANCE_TEST_0500, EVENT_CODE_500, "false");
    }
}

void FormAbilityPerformance::FMS_performanceTest_0600(std::string data)
{
    std::vector<int64_t> formIds;
    formIds.push_back(atoll(g_formId.c_str()));
    bool bResult = NotifyInvisibleForms(formIds);
    if (bResult) {
        APP_LOGI("[FMS_performanceTest_0600] NotifyInvisibleForms end");
        FormTestUtils::PublishEvent(FORM_EVENT_RECV_PERFORMANCE_TEST_0600, EVENT_CODE_600, "true");
    } else {
        APP_LOGE("[FMS_performanceTest_0600] NotifyInvisibleForms error");
        FormTestUtils::PublishEvent(FORM_EVENT_RECV_PERFORMANCE_TEST_0600, EVENT_CODE_600, "false");
    }
}

void FormAbilityPerformance::FMS_performanceTest_0700(std::string data)
{
    bool bResult = RequestForm(atoll(g_formId.c_str()));
    if (bResult) {
        APP_LOGI("[FMS_performanceTest_0700] RequestForm end");
        FormTestUtils::PublishEvent(FORM_EVENT_RECV_PERFORMANCE_TEST_0700, EVENT_CODE_700, "true");
    } else {
        APP_LOGE("[FMS_performanceTest_0700] RequestForm error");
        FormTestUtils::PublishEvent(FORM_EVENT_RECV_PERFORMANCE_TEST_1300, EVENT_CODE_700, "false");
    }
}

void FormAbilityPerformance::FMS_performanceTest_1300(std::string data)
{
    std::vector<FormInfo> formInfos;
    bool bResult = GetAllFormsInfo(formInfos);
    if (bResult) {
        APP_LOGI("[FMS_performanceTest_1300] GetAllFormsInfo end");
        FormTestUtils::PublishEvent(FORM_EVENT_RECV_PERFORMANCE_TEST_1300, EVENT_CODE_1300, "true");
    } else {
        APP_LOGE("[FMS_performanceTest_1300] GetAllFormsInfo error");
        FormTestUtils::PublishEvent(FORM_EVENT_RECV_PERFORMANCE_TEST_1300, EVENT_CODE_1300, "false");
    }
}

void FormAbilityPerformance::FMS_performanceTest_1400(std::string data)
{
    std::vector<FormInfo> formInfos;
    bool bResult = GetFormsInfoByApp(g_bundleName, formInfos);
    if (bResult) {
        APP_LOGI("[FMS_performanceTest_1400] GetFormsInfoByApp end");
        FormTestUtils::PublishEvent(FORM_EVENT_RECV_PERFORMANCE_TEST_1400, EVENT_CODE_1400, "true");
    } else {
        APP_LOGE("[FMS_performanceTest_1400] GetFormsInfoByApp error");
        FormTestUtils::PublishEvent(FORM_EVENT_RECV_PERFORMANCE_TEST_1400, EVENT_CODE_1400, "false");
    }
}

void FormAbilityPerformance::FMS_performanceTest_1500(std::string data)
{
    std::vector<FormInfo> formInfos;
    bool bResult = GetFormsInfoByModule(g_bundleName, g_moduleName, formInfos);
    if (bResult) {
        APP_LOGI("[FMS_performanceTest_1500] GetFormsInfoByModule end");
        FormTestUtils::PublishEvent(FORM_EVENT_RECV_PERFORMANCE_TEST_1500, EVENT_CODE_1500, "true");
    } else {
        APP_LOGE("[FMS_performanceTest_1500] GetFormsInfoByModule error");
        FormTestUtils::PublishEvent(FORM_EVENT_RECV_PERFORMANCE_TEST_1500, EVENT_CODE_1500, "false");
    }
}

void FormAbilityPerformance::FMS_performanceTest_1600(std::string data)
{
    bool bResult = CheckFMSReady();
    if (bResult) {
        APP_LOGI("[FMS_performanceTest_1600] CheckFMSReady end");
        FormTestUtils::PublishEvent(FORM_EVENT_RECV_PERFORMANCE_TEST_1600, EVENT_CODE_1600, "true");
    } else {
        APP_LOGE("[FMS_performanceTest_1600] CheckFMSReady error");
        FormTestUtils::PublishEvent(FORM_EVENT_RECV_PERFORMANCE_TEST_1600, EVENT_CODE_1600, "false");
    }
}

void FormAbilityPerformance::OnStart(const Want &want)
{
    APP_LOGI("FormAbilityPerformance::onStart");
    Ability::OnStart(want);
}

void FormAbilityPerformance::OnActive()
{
    APP_LOGI("FormAbilityPerformance::OnActive");
    Ability::OnActive();
    std::string eventData = GetAbilityName() + FORM_ABILITY_STATE_ONACTIVE;
    FormTestUtils::PublishEvent(FORM_EVENT_ABILITY_ONACTIVED, 0, eventData);
}

void FormAbilityPerformance::OnStop()
{
    APP_LOGI("FormAbilityPerformance::OnStop");

    Ability::OnStop();
    CommonEventManager::UnSubscribeCommonEvent(subscriber_);
}

void FormAbilityPerformance::OnInactive()
{
    APP_LOGI("FormAbilityPerformance::OnInactive");

    Ability::OnInactive();
}

void FormAbilityPerformance::OnBackground()
{
    APP_LOGI("FormAbilityPerformance::OnBackground");

    Ability::OnBackground();
}

void FormAbilityPerformance::SubscribeEvent()
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
void FormAbilityPerformance::Init(const std::shared_ptr<AbilityInfo> &abilityInfo,
    const std::shared_ptr<OHOSApplication> &application, std::shared_ptr<AbilityHandler> &handler,
    const sptr<IRemoteObject> &token)
{
    APP_LOGI("FormAbilityPerformance::Init");
    Ability::Init(abilityInfo, application, handler, token);

    memberFuncMap_[FORM_EVENT_REQ_PERFORMANCE_TEST_0100] = &FormAbilityPerformance::FMS_performanceTest_0100;
    memberFuncMap_[FORM_EVENT_REQ_PERFORMANCE_TEST_0200] = &FormAbilityPerformance::FMS_performanceTest_0200;
    memberFuncMap_[FORM_EVENT_REQ_PERFORMANCE_TEST_0300] = &FormAbilityPerformance::FMS_performanceTest_0300;
    memberFuncMap_[FORM_EVENT_REQ_PERFORMANCE_TEST_0400] = &FormAbilityPerformance::FMS_performanceTest_0400;
    memberFuncMap_[FORM_EVENT_REQ_PERFORMANCE_TEST_0500] = &FormAbilityPerformance::FMS_performanceTest_0500;
    memberFuncMap_[FORM_EVENT_REQ_PERFORMANCE_TEST_0600] = &FormAbilityPerformance::FMS_performanceTest_0600;
    memberFuncMap_[FORM_EVENT_REQ_PERFORMANCE_TEST_0700] = &FormAbilityPerformance::FMS_performanceTest_0700;
    memberFuncMap_[FORM_EVENT_REQ_PERFORMANCE_TEST_1300] = &FormAbilityPerformance::FMS_performanceTest_1300;
    memberFuncMap_[FORM_EVENT_REQ_PERFORMANCE_TEST_1400] = &FormAbilityPerformance::FMS_performanceTest_1400;
    memberFuncMap_[FORM_EVENT_REQ_PERFORMANCE_TEST_1500] = &FormAbilityPerformance::FMS_performanceTest_1500;
    memberFuncMap_[FORM_EVENT_REQ_PERFORMANCE_TEST_1600] = &FormAbilityPerformance::FMS_performanceTest_1600;

    SubscribeEvent();
}

void FormAbilityPerformance::handleEvent(std::string action, std::string data)
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

void FormAbilityPerformance::Clear()
{
}

void FormEventSubscriber::OnReceiveEvent(const CommonEventData &data)
{
    APP_LOGI("FormEventSubscriber::OnReceiveEvent:event=%{public}s, code=%{public}d, data=%{public}s",
        data.GetWant().GetAction().c_str(), data.GetCode(), data.GetData().c_str());
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

REGISTER_AA(FormAbilityPerformance)
}  // namespace AppExecFwk
}  // namespace OHOS