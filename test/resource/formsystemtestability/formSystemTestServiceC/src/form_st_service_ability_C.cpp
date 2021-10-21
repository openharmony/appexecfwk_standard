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

#include "form_st_service_ability_C.h"
#include "app_log_wrapper.h"
#include "common_event.h"
#include "common_event_manager.h"
#include "form_provider_client.h"

using namespace OHOS::EventFwk;

namespace OHOS {
namespace AppExecFwk {
constexpr int64_t SEC_TO_MILLISEC = 1000;
constexpr int64_t MILLISEC_TO_NANOSEC = 1000000;
using AbilityConnectionProxy = OHOS::AAFwk::AbilityConnectionProxy;

int FormStServiceAbilityC::AbilityConnectCallback::onAbilityConnectDoneCount = 0;
std::map<std::string, FormStServiceAbilityC::func> FormStServiceAbilityC::funcMap_ = {
    {"StartOtherAbility", &FormStServiceAbilityC::StartOtherAbility},
    {"ConnectOtherAbility", &FormStServiceAbilityC::ConnectOtherAbility},
    {"DisConnectOtherAbility", &FormStServiceAbilityC::DisConnectOtherAbility},
    {"StopSelfAbility", &FormStServiceAbilityC::StopSelfAbility},
};

FormStServiceAbilityC::~FormStServiceAbilityC()
{
    CommonEventManager::UnSubscribeCommonEvent(subscriber_);
}

std::vector<std::string> FormStServiceAbilityC::Split(std::string str, const std::string &token)
{
    APP_LOGI("FormStServiceAbilityC::Split");

    std::vector<std::string> splitString;
    while (str.size()) {
        size_t index = str.find(token);
        if (index != std::string::npos) {
            splitString.push_back(str.substr(0, index));
            str = str.substr(index + token.size());
            if (str.size() == 0) {
                splitString.push_back(str);
            }
        } else {
            splitString.push_back(str);
            str = "";
        }
    }
    return splitString;
}
void FormStServiceAbilityC::StartOtherAbility()
{
    APP_LOGI("FormStServiceAbilityC::StartOtherAbility begin targetBundle=%{public}s, targetAbility=%{public}s",
        targetBundle_.c_str(),
        targetAbility_.c_str());
    APP_LOGI("FormStServiceAbilityC::StartOtherAbility begin nextTargetBundleConn=%{public}s, "
             "nextTargetAbilityConn=%{public}s",
        nextTargetBundleConn_.c_str(),
        nextTargetAbilityConn_.c_str());

    if (!targetBundle_.empty() && !targetAbility_.empty()) {
        std::vector<std::string> strtargetBundles = Split(targetBundle_, ",");
        std::vector<std::string> strTargetAbilitys = Split(targetAbility_, ",");
        for (size_t i = 0; i < strtargetBundles.size() && i < strTargetAbilitys.size(); i++) {
            Want want;
            want.SetElementName(strtargetBundles[i], strTargetAbilitys[i]);
            want.SetParam("shouldReturn", shouldReturn_);
            want.SetParam("targetBundle", nextTargetBundle_);
            want.SetParam("targetAbility", nextTargetAbility_);
            want.SetParam("targetBundleConn", nextTargetBundleConn_);
            want.SetParam("targetAbilityConn", nextTargetAbilityConn_);
            StartAbility(want);
            sleep(1);
        }
    }
}
void FormStServiceAbilityC::ConnectOtherAbility()
{
    APP_LOGI(
        "FormStServiceAbilityC::ConnectOtherAbility begin targetBundleConn=%{public}s, targetAbilityConn=%{public}s",
        targetBundleConn_.c_str(),
        targetAbilityConn_.c_str());
    APP_LOGI("FormStServiceAbilityC::ConnectOtherAbility begin nextTargetBundleConn=%{public}s, "
             "nextTargetAbilityConn=%{public}s",
        nextTargetBundleConn_.c_str(),
        nextTargetAbilityConn_.c_str());

    // connect service ability
    if (!targetBundleConn_.empty() && !targetAbilityConn_.empty()) {
        std::vector<std::string> strtargetBundles = Split(targetBundleConn_, ",");
        std::vector<std::string> strTargetAbilitys = Split(targetAbilityConn_, ",");
        for (size_t i = 0; i < strtargetBundles.size() && i < strTargetAbilitys.size(); i++) {
            Want want;
            want.SetElementName(strtargetBundles[i], strTargetAbilitys[i]);
            want.SetParam("shouldReturn", shouldReturn_);
            want.SetParam("targetBundle", nextTargetBundle_);
            want.SetParam("targetAbility", nextTargetAbility_);
            want.SetParam("targetBundleConn", nextTargetBundleConn_);
            want.SetParam("targetAbilityConn", nextTargetAbilityConn_);
            stub_ = new (std::nothrow) AbilityConnectCallback();
            connCallback_ = new (std::nothrow) AbilityConnectionProxy(stub_);
            APP_LOGI("FormStServiceAbilityC::ConnectOtherAbility->ConnectAbility");
            bool ret = ConnectAbility(want, connCallback_);
            sleep(1);
            if (!ret) {
                APP_LOGE("FormStServiceAbilityC::ConnectAbility failed!");
            }
        }
    }
}
void FormStServiceAbilityC::DisConnectOtherAbility()
{
    APP_LOGI("FormStServiceAbilityC::DisConnectOtherAbility begin");
    if (connCallback_ != nullptr) {
        DisconnectAbility(connCallback_);
        sleep(1);
    }
    APP_LOGI("FormStServiceAbilityC::DisConnectOtherAbility end");
}

void FormStServiceAbilityC::StopSelfAbility()
{
    APP_LOGI("FormStServiceAbilityC::StopSelfAbility");

    TerminateAbility();
}

void FormStServiceAbilityC::OnStart(const Want &want)
{
    APP_LOGI("FormStServiceAbilityC::OnStart");

    GetWantInfo(want);
    Ability::OnStart(want);
    PublishEvent(APP_A_RESP_EVENT_NAME, AbilityLifecycleExecutor::LifecycleState::INACTIVE, "OnStart");
    SubscribeEvent();

    // make exception for test
    if (!zombie_.empty()) {
        std::unique_ptr<Want> pWant = nullptr;
        pWant->GetScheme();
    }
}
void FormStServiceAbilityC::OnCommand(const AAFwk::Want &want, bool restart, int startId)
{
    APP_LOGI("FormStServiceAbilityC::OnCommand");

    GetWantInfo(want);
    Ability::OnCommand(want, restart, startId);
    PublishEvent(APP_A_RESP_EVENT_NAME, AbilityLifecycleExecutor::LifecycleState::ACTIVE, "OnCommand");
}
void FormStServiceAbilityC::OnNewWant(const Want &want)
{
    APP_LOGI("FormStServiceAbilityC::OnNewWant");

    GetWantInfo(want);
    Ability::OnNewWant(want);
}
void FormStServiceAbilityC::OnStop()
{
    APP_LOGI("FormStServiceAbilityC::OnStop");

    Ability::OnStop();
    PublishEvent(APP_A_RESP_EVENT_NAME, AbilityLifecycleExecutor::LifecycleState::INITIAL, "OnStop");
}
void FormStServiceAbilityC::OnActive()
{
    APP_LOGI("FormStServiceAbilityC::OnActive");

    Ability::OnActive();
    PublishEvent(APP_A_RESP_EVENT_NAME, AbilityLifecycleExecutor::LifecycleState::ACTIVE, "OnActive");
}
void FormStServiceAbilityC::OnInactive()
{
    APP_LOGI("FormStServiceAbilityC::OnInactive");

    Ability::OnInactive();
    PublishEvent(APP_A_RESP_EVENT_NAME, AbilityLifecycleExecutor::LifecycleState::INACTIVE, "OnInactive");
}
void FormStServiceAbilityC::OnBackground()
{
    APP_LOGI("FormStServiceAbilityC::OnBackground");

    Ability::OnBackground();
    PublishEvent(APP_A_RESP_EVENT_NAME, AbilityLifecycleExecutor::LifecycleState::BACKGROUND, "OnBackground");
}

void FormStServiceAbilityC::Clear()
{
    shouldReturn_ = "";
    targetBundle_ = "";
    targetAbility_ = "";
    targetBundleConn_ = "";
    targetAbilityConn_ = "";
    nextTargetBundle_ = "";
    nextTargetAbility_ = "";
    nextTargetBundleConn_ = "";
    nextTargetAbilityConn_ = "";
}
void FormStServiceAbilityC::GetWantInfo(const Want &want)
{
    Want mWant(want);
    shouldReturn_ = mWant.GetStringParam("shouldReturn");
    targetBundle_ = mWant.GetStringParam("targetBundle");
    targetAbility_ = mWant.GetStringParam("targetAbility");
    targetBundleConn_ = mWant.GetStringParam("targetBundleConn");
    targetAbilityConn_ = mWant.GetStringParam("targetAbilityConn");
    nextTargetBundle_ = mWant.GetStringParam("nextTargetBundle");
    nextTargetAbility_ = mWant.GetStringParam("nextTargetAbility");
    nextTargetBundleConn_ = mWant.GetStringParam("nextTargetBundleConn");
    nextTargetAbilityConn_ = mWant.GetStringParam("nextTargetAbilityConn");
    zombie_ = mWant.GetStringParam("zombie");
    FormStServiceAbilityC::AbilityConnectCallback::onAbilityConnectDoneCount = 0;
}
bool FormStServiceAbilityC::PublishEvent(const std::string &eventName, const int &code, const std::string &data)
{
    APP_LOGI("FormStServiceAbilityC::PublishEvent eventName = %{public}s, code = %{public}d, data = %{public}s",
        eventName.c_str(),
        code,
        data.c_str());

    Want want;
    want.SetAction(eventName);
    CommonEventData commonData;
    commonData.SetWant(want);
    commonData.SetCode(code);
    commonData.SetData(data);
    return CommonEventManager::PublishCommonEvent(commonData);
}
sptr<IRemoteObject> FormStServiceAbilityC::OnConnect(const Want &want)
{
    APP_LOGI("FormStServiceAbilityC::OnConnect");
    
    sptr<FormProviderClient> formProviderClient = new (std::nothrow) FormProviderClient();
    std::shared_ptr<Ability> thisAbility = this->shared_from_this();
    formProviderClient->SetOwner(thisAbility);

    return formProviderClient;
}
void FormStServiceAbilityC::OnDisconnect(const Want &want)
{
    APP_LOGI("FormStServiceAbilityC::OnDisconnect");

    Ability::OnDisconnect(want);
    PublishEvent(APP_A_RESP_EVENT_NAME, AbilityLifecycleExecutor::LifecycleState::BACKGROUND, "OnDisconnect");
}
bool FormStServiceAbilityC::SubscribeEvent()
{
    MatchingSkills matchingSkills;
    matchingSkills.AddEvent(APP_A_REQ_EVENT_NAME);
    CommonEventSubscribeInfo subscribeInfo(matchingSkills);
    subscribeInfo.SetPriority(1);
    subscriber_ = std::make_shared<AppEventSubscriber>(subscribeInfo);
    subscriber_->mainAbility_ = this;
    return CommonEventManager::SubscribeCommonEvent(subscriber_);
}
void FormStServiceAbilityC::AppEventSubscriber::OnReceiveEvent(const CommonEventData &data)
{
    auto eventName = data.GetWant().GetAction();
    auto dataContent = data.GetData();
    APP_LOGI("FormStServiceAbilityC::OnReceiveEvent eventName = %{public}s, code = %{public}d, data = %{public}s",
        eventName.c_str(),
        data.GetCode(),
        dataContent.c_str());
    if (APP_A_REQ_EVENT_NAME.compare(eventName) == 0) {
        if (funcMap_.find(dataContent) == funcMap_.end()) {
            APP_LOGI(
                "FormStServiceAbilityC::OnReceiveEvent eventName = %{public}s, code = %{public}d, data = %{public}s",
                eventName.c_str(),
                data.GetCode(),
                dataContent.c_str());
        } else {
            if (mainAbility_ != nullptr) {
                (mainAbility_->*funcMap_[dataContent])();
            }
        }
    }
}

FormProviderInfo FormStServiceAbilityC::OnCreate(const Want &want)
{
    APP_LOGI("%{public}s start", __func__);
    FormProviderInfo formProviderInfo;
    if (!want.HasParameter(Constants::PARAM_FORM_IDENTITY_KEY)) {
        APP_LOGE("%{public}s, formId not exist", __func__);
        return formProviderInfo;
    }
    std::string formId = want.GetStringParam(Constants::PARAM_FORM_IDENTITY_KEY);
    std::string jsonData = std::string("{\"city\":\"beijingC\"}");
    FormProviderData formProviderData = FormProviderData(jsonData);
    formProviderInfo.SetFormData(formProviderData);
    PublishEvent(COMMON_EVENT_TEST_ACTION1, FORM_EVENT_TRIGGER_RESULT::FORM_EVENT_TRIGGER_RESULT_OK, "OnCreate");
    APP_LOGI("%{public}s end, formId: %{public}s", __func__, formId.c_str());
    return formProviderInfo;
}

void FormStServiceAbilityC::OnUpdate(const int64_t formId)
{
    APP_LOGI("%{public}s start", __func__);
    struct timespec ts;
    clock_gettime(CLOCK_REALTIME, &ts);
    long currentTime = ts.tv_sec * SEC_TO_MILLISEC + ts.tv_nsec / MILLISEC_TO_NANOSEC;

    PublishEvent(COMMON_EVENT_TEST_ACTION1, FORM_EVENT_TRIGGER_RESULT::FORM_EVENT_TRIGGER_RESULT_OK, "OnUpdate");
    APP_LOGI("%{public}s end, formId: %{public}s, current time: %{public}ld", __func__,
        std::to_string(formId).c_str(), currentTime);
}

void FormStServiceAbilityC::OnTriggerEvent(const int64_t formId, const std::string &message)
{
    APP_LOGI("%{public}s start", __func__);
    
    struct timespec ts;
    clock_gettime(CLOCK_REALTIME, &ts);
    long currentTime = ts.tv_sec * SEC_TO_MILLISEC + ts.tv_nsec / MILLISEC_TO_NANOSEC;

    PublishEvent(COMMON_EVENT_TEST_ACTION1, FORM_EVENT_TRIGGER_RESULT::FORM_EVENT_TRIGGER_RESULT_OK, "OnTriggerEvent");
    APP_LOGI("%{public}s end, formId: %{public}s, message: %{public}s, current time: %{public}ld", __func__,
        std::to_string(formId).c_str(), message.c_str(), currentTime);
}

void FormStServiceAbilityC::OnDelete(const int64_t formId)
{
    APP_LOGI("%{public}s start", __func__);

    struct timespec ts;
    clock_gettime(CLOCK_REALTIME, &ts);
    long currentTime = ts.tv_sec * SEC_TO_MILLISEC + ts.tv_nsec / MILLISEC_TO_NANOSEC;

    PublishEvent(COMMON_EVENT_TEST_ACTION1, FORM_EVENT_TRIGGER_RESULT::FORM_EVENT_TRIGGER_RESULT_OK, "OnDelete");
    APP_LOGI("%{public}s end, formId: %{public}s, current time: %{public}ld", __func__,
        std::to_string(formId).c_str(), currentTime);
}

void FormStServiceAbilityC::OnCastTemptoNormal(const int64_t formId)
{
    APP_LOGI("%{public}s start", __func__);

    struct timespec ts;
    clock_gettime(CLOCK_REALTIME, &ts);
    long currentTime = ts.tv_sec * SEC_TO_MILLISEC + ts.tv_nsec / MILLISEC_TO_NANOSEC;

    PublishEvent(COMMON_EVENT_TEST_ACTION1, FORM_EVENT_TRIGGER_RESULT::FORM_EVENT_TRIGGER_RESULT_OK,
        "OnCastTemptoNormal");
    APP_LOGI("%{public}s end, formId: %{public}s, current time: %{public}ld", __func__,
        std::to_string(formId).c_str(), currentTime);
}

void FormStServiceAbilityC::OnVisibilityChanged(const std::map<int64_t, int32_t> &formEventsMap)
{
    APP_LOGI("%{public}s start", __func__);

    struct timespec ts;
    clock_gettime(CLOCK_REALTIME, &ts);
    long currentTime = ts.tv_sec * SEC_TO_MILLISEC + ts.tv_nsec / MILLISEC_TO_NANOSEC;

    PublishEvent(COMMON_EVENT_TEST_ACTION1, FORM_EVENT_TRIGGER_RESULT::FORM_EVENT_TRIGGER_RESULT_OK,
        "OnVisibilityChanged");
    APP_LOGI("%{public}s end, current time: %{public}ld", __func__, currentTime);
}

REGISTER_AA(FormStServiceAbilityC);
}  // namespace AppExecFwk
}  // namespace OHOS
