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

#include "app_log_wrapper.h"
#include "form_ability_uninstall_app.h"
#include "form_st_common_info.h"
#include "form_test_utils.h"

namespace {
    using namespace OHOS::AAFwk;
    using namespace OHOS::EventFwk;
}

namespace OHOS {
namespace AppExecFwk {
std::vector<std::string> eventList = {
    FORM_EVENT_REQ_UNINSTALL_TEST_0100, FORM_EVENT_REQ_UNINSTALL_TEST_0200, FORM_EVENT_REQ_UNINSTALL_TEST_0300,
    FORM_EVENT_REQ_UNINSTALL_TEST_0400, FORM_EVENT_REQ_UNINSTALL_TEST_0500, FORM_EVENT_REQ_UNINSTALL_TEST_0600, 
    FORM_EVENT_REQ_UNINSTALL_TEST_0700, FORM_EVENT_REQ_UNINSTALL_TEST_0800, FORM_EVENT_REQ_UNINSTALL_TEST_0900,
    FORM_EVENT_REQ_UNINSTALL_TEST_1000, FORM_EVENT_REQ_UNINSTALL_TEST_1100, FORM_EVENT_REQ_UNINSTALL_TEST_1200,
    FORM_EVENT_REQ_UNINSTALL_TEST_1300, FORM_EVENT_REQ_UNINSTALL_TEST_1400, FORM_EVENT_REQ_UNINSTALL_TEST_1500,
    FORM_EVENT_REQ_UNINSTALL_TEST_1600, FORM_EVENT_REQ_UNINSTALL_TEST_1700, FORM_EVENT_REQ_UNINSTALL_TEST_1800,
};

static std::string g_bundleName = "com.form.formsystemtestservicea";
static std::string g_moduleName = "formmodule001";
void FormAbilityUninstallApp::UninstallAppFormCallback::OnAcquired(const int32_t result, const FormJsInfo &formJsInfo) const
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

    APP_LOGI("%{public}s caseName_: %{public}s, code_: %{public}d", __func__, this->caseName_.c_str(), this->code_);

    if (this->caseName_ == FORM_EVENT_RECV_UNINSTALL_TEST_0100) {
        FormTestUtils::PublishEvent(this->caseName_, 101, std::to_string(formJsInfo.formId));
    } else if (this->caseName_ == FORM_EVENT_RECV_UNINSTALL_TEST_0200) {
        FormTestUtils::PublishEvent(this->caseName_, 201, std::to_string(formJsInfo.formId));
    } else {
        FormTestUtils::PublishEvent(this->caseName_, this->code_ + 1, std::to_string(formJsInfo.formId));       
    }
}

void FormAbilityUninstallApp::UninstallAppFormCallback::OnUpdate(const int32_t result, const FormJsInfo &formJsInfo) const
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

    APP_LOGI("%{public}s caseName_: %{public}s, code_: %{public}d", __func__, this->caseName_.c_str(), this->code_);

    if (this->caseName_ == FORM_EVENT_RECV_UNINSTALL_TEST_0100) {
        FormTestUtils::PublishEvent(this->caseName_, 102, "true");
    } else if (this->caseName_ == FORM_EVENT_RECV_UNINSTALL_TEST_0200) {
        FormTestUtils::PublishEvent(this->caseName_, 202, "true");
    } else {
        FormTestUtils::PublishEvent(this->caseName_, this->code_ + 2, "true");        
    }
}

void FormAbilityUninstallApp::UninstallAppFormCallback::OnFormUninstall(const int64_t formId) const
{
    APP_LOGI("%{public}s called", __func__);
}

void FormAbilityUninstallApp::FMS_uninstallTest_0100(std::string data)
{
    std::shared_ptr<UninstallAppFormCallback> callback = 
    std::make_shared<UninstallAppFormCallback>(FORM_EVENT_RECV_UNINSTALL_TEST_0100, 100);
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
        APP_LOGI("[FMS_uninstallTest_0100] AcquireForm end");
    } else {
        APP_LOGE("[FMS_uninstallTest_0100] AcquireForm error");
        FormTestUtils::PublishEvent(FORM_EVENT_RECV_UNINSTALL_TEST_0100, 100, "false");
    }
}

void FormAbilityUninstallApp::FMS_uninstallTest_0200(std::string data)
{
    std::shared_ptr<UninstallAppFormCallback> callback = 
    std::make_shared<UninstallAppFormCallback>(FORM_EVENT_RECV_UNINSTALL_TEST_0200, 200);
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
        APP_LOGI("[FMS_uninstallTest_0200] AcquireForm end");
    } else {
        APP_LOGE("[FMS_uninstallTest_0200] AcquireForm error");
        FormTestUtils::PublishEvent(FORM_EVENT_RECV_UNINSTALL_TEST_0200, 200, "false");
    }
}

void FormAbilityUninstallApp::FMS_uninstallTest_0300(std::string data)
{
    std::shared_ptr<UninstallAppFormCallback> callback = 
    std::make_shared<UninstallAppFormCallback>(FORM_EVENT_RECV_UNINSTALL_TEST_0300, 300);
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
        APP_LOGI("[FMS_uninstallTest_0300] AcquireForm end");
    } else {
        APP_LOGE("[FMS_uninstallTest_0300] AcquireForm error");
        FormTestUtils::PublishEvent(FORM_EVENT_RECV_UNINSTALL_TEST_0300, 300, "false");
    }
}

void FormAbilityUninstallApp::FMS_uninstallTest_0400(std::string data)
{
}

void FormAbilityUninstallApp::FMS_uninstallTest_0500(std::string data)
{
}

void FormAbilityUninstallApp::FMS_uninstallTest_0600(std::string data)
{
}

void FormAbilityUninstallApp::FMS_uninstallTest_0700(std::string data)
{
}

void FormAbilityUninstallApp::FMS_uninstallTest_0800(std::string data)
{
}

void FormAbilityUninstallApp::FMS_uninstallTest_0900(std::string data)
{
}

void FormAbilityUninstallApp::FMS_uninstallTest_1000(std::string data)
{
}

void FormAbilityUninstallApp::FMS_uninstallTest_1100(std::string data)
{
    std::shared_ptr<UninstallAppFormCallback> callback = 
    std::make_shared<UninstallAppFormCallback>(FORM_EVENT_RECV_UNINSTALL_TEST_1100, 1100);
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
        APP_LOGI("[FMS_uninstallTest_1100] AcquireForm end");
    } else {
        APP_LOGE("[FMS_uninstallTest_1100] AcquireForm error");
        FormTestUtils::PublishEvent(FORM_EVENT_RECV_UNINSTALL_TEST_1100, 1100, "false");
    }
}

void FormAbilityUninstallApp::FMS_uninstallTest_1200(std::string data)
{
}

void FormAbilityUninstallApp::FMS_uninstallTest_1300(std::string data)
{
    std::shared_ptr<UninstallAppFormCallback> callback = 
    std::make_shared<UninstallAppFormCallback>(FORM_EVENT_RECV_UNINSTALL_TEST_1300, 1300);
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
        APP_LOGI("[FMS_uninstallTest_1300] AcquireForm end");
    } else {
        APP_LOGE("[FMS_uninstallTest_1300] AcquireForm error");
        FormTestUtils::PublishEvent(FORM_EVENT_RECV_UNINSTALL_TEST_1300, 1300, "false");
    }
}

void FormAbilityUninstallApp::FMS_uninstallTest_1400(std::string data)
{
}

void FormAbilityUninstallApp::FMS_uninstallTest_1500(std::string data)
{
}

void FormAbilityUninstallApp::FMS_uninstallTest_1600(std::string data)
{
}

void FormAbilityUninstallApp::FMS_uninstallTest_1700(std::string data)
{
    std::vector<FormInfo> formInfos;
    bool bResult = GetAllFormsInfo(formInfos); 
    if (bResult) {
        APP_LOGI("[FMS_uninstallTest_1700] GetAllFormsInfo end");
        FormTestUtils::PublishEvent(FORM_EVENT_RECV_UNINSTALL_TEST_1700, 1700, "true");
    } else {
        APP_LOGE("[FMS_uninstallTest_1700] GetAllFormsInfo error");
        FormTestUtils::PublishEvent(FORM_EVENT_RECV_UNINSTALL_TEST_1700, 1700, "false");
    }
}

void FormAbilityUninstallApp::FMS_uninstallTest_1800(std::string data)
{
    bool bResult = CheckFMSReady(); 
    if (bResult) {
        APP_LOGI("[FMS_uninstallTest_1800] CheckFMSReady end");
        FormTestUtils::PublishEvent(FORM_EVENT_RECV_UNINSTALL_TEST_1800, 1800, "true");
    } else {
        APP_LOGE("[FMS_uninstallTest_1800] CheckFMSReady error");
        FormTestUtils::PublishEvent(FORM_EVENT_RECV_UNINSTALL_TEST_1800, 1800, "false");
    }
}

void FormAbilityUninstallApp::OnStart(const Want &want)
{
    APP_LOGI("FormAbilityUninstallApp::onStart");
    Ability::OnStart(want);
}

void FormAbilityUninstallApp::OnActive()
{
    APP_LOGI("FormAbilityUninstallApp::OnActive");
    Ability::OnActive();
    std::string eventData = GetAbilityName() + FORM_ABILITY_STATE_ONACTIVE;
    FormTestUtils::PublishEvent(FORM_EVENT_ABILITY_ONACTIVED, 0, eventData);
}

void FormAbilityUninstallApp::OnStop()
{
    APP_LOGI("FormAbilityUninstallApp::OnStop");

    Ability::OnStop();
    CommonEventManager::UnSubscribeCommonEvent(subscriber_);
}

void FormAbilityUninstallApp::OnInactive()
{
    APP_LOGI("FormAbilityUninstallApp::OnInactive");

    Ability::OnInactive();
}

void FormAbilityUninstallApp::OnBackground()
{
    APP_LOGI("FormAbilityUninstallApp::OnBackground");

    Ability::OnBackground();
}

void FormAbilityUninstallApp::SubscribeEvent()
{
    APP_LOGI("FormAbilityUninstallApp::SubscribeEvent");
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
void FormAbilityUninstallApp::Init(const std::shared_ptr<AbilityInfo> &abilityInfo,
    const std::shared_ptr<OHOSApplication> &application, std::shared_ptr<AbilityHandler> &handler,
    const sptr<IRemoteObject> &token)
{
    APP_LOGI("FormAbilityUninstallApp::Init");
    Ability::Init(abilityInfo, application, handler, token);

    memberFuncMap_[FORM_EVENT_REQ_UNINSTALL_TEST_0100] = &FormAbilityUninstallApp::FMS_uninstallTest_0100;
    memberFuncMap_[FORM_EVENT_REQ_UNINSTALL_TEST_0200] = &FormAbilityUninstallApp::FMS_uninstallTest_0200;
    memberFuncMap_[FORM_EVENT_REQ_UNINSTALL_TEST_0300] = &FormAbilityUninstallApp::FMS_uninstallTest_0300;
    memberFuncMap_[FORM_EVENT_REQ_UNINSTALL_TEST_0400] = &FormAbilityUninstallApp::FMS_uninstallTest_0400;
    memberFuncMap_[FORM_EVENT_REQ_UNINSTALL_TEST_0500] = &FormAbilityUninstallApp::FMS_uninstallTest_0500;
    memberFuncMap_[FORM_EVENT_REQ_UNINSTALL_TEST_0600] = &FormAbilityUninstallApp::FMS_uninstallTest_0600;
    memberFuncMap_[FORM_EVENT_REQ_UNINSTALL_TEST_0700] = &FormAbilityUninstallApp::FMS_uninstallTest_0700;
    memberFuncMap_[FORM_EVENT_REQ_UNINSTALL_TEST_0800] = &FormAbilityUninstallApp::FMS_uninstallTest_0800;
    memberFuncMap_[FORM_EVENT_REQ_UNINSTALL_TEST_0900] = &FormAbilityUninstallApp::FMS_uninstallTest_0900;
    memberFuncMap_[FORM_EVENT_REQ_UNINSTALL_TEST_1000] = &FormAbilityUninstallApp::FMS_uninstallTest_1000;
    memberFuncMap_[FORM_EVENT_REQ_UNINSTALL_TEST_1100] = &FormAbilityUninstallApp::FMS_uninstallTest_1100;
    memberFuncMap_[FORM_EVENT_REQ_UNINSTALL_TEST_1300] = &FormAbilityUninstallApp::FMS_uninstallTest_1300;
    memberFuncMap_[FORM_EVENT_REQ_UNINSTALL_TEST_1700] = &FormAbilityUninstallApp::FMS_uninstallTest_1700;
    memberFuncMap_[FORM_EVENT_REQ_UNINSTALL_TEST_1800] = &FormAbilityUninstallApp::FMS_uninstallTest_1800;

    SubscribeEvent();
} 

void FormAbilityUninstallApp::handleEvent(std::string action, std::string data)
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

void FormAbilityUninstallApp::Clear()
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

REGISTER_AA(FormAbilityUninstallApp)
}  // namespace AppExecFwk
}  // namespace OHOS