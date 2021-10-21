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

#include "form_st_service_ability_A.h"
#include "app_log_wrapper.h"
#include "common_event.h"
#include "common_event_manager.h"
#include "form_provider_client.h"

using namespace OHOS::EventFwk;

constexpr int64_t SEC_TO_MILLISEC = 1000;
constexpr int64_t MILLISEC_TO_NANOSEC = 1000000;

namespace OHOS {
namespace AppExecFwk {
using AbilityConnectionProxy = OHOS::AAFwk::AbilityConnectionProxy;

int FormStServiceAbilityA::AbilityConnectCallback::onAbilityConnectDoneCount = 0;
std::map<std::string, FormStServiceAbilityA::func> FormStServiceAbilityA::funcMap_ = {
    {"StartOtherAbility", &FormStServiceAbilityA::StartOtherAbility},
    {"ConnectOtherAbility", &FormStServiceAbilityA::ConnectOtherAbility},
    {"DisConnectOtherAbility", &FormStServiceAbilityA::DisConnectOtherAbility},
    {"StopSelfAbility", &FormStServiceAbilityA::StopSelfAbility},
};

FormStServiceAbilityA::~FormStServiceAbilityA()
{
    CommonEventManager::UnSubscribeCommonEvent(subscriber_);
}

std::vector<std::string> FormStServiceAbilityA::Split(std::string str, const std::string &token)
{
    APP_LOGI("FormStServiceAbilityA::Split");

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
void FormStServiceAbilityA::StartOtherAbility()
{
    APP_LOGI("FormStServiceAbilityA::StartOtherAbility begin targetBundle=%{public}s, targetAbility=%{public}s",
        targetBundle_.c_str(),
        targetAbility_.c_str());
    APP_LOGI("FormStServiceAbilityA::StartOtherAbility begin nextTargetBundleConn=%{public}s, "
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
void FormStServiceAbilityA::ConnectOtherAbility()
{
    APP_LOGI(
        "FormStServiceAbilityA::ConnectOtherAbility begin targetBundleConn=%{public}s, targetAbilityConn=%{public}s",
        targetBundleConn_.c_str(),
        targetAbilityConn_.c_str());
    APP_LOGI("FormStServiceAbilityA::ConnectOtherAbility begin nextTargetBundleConn=%{public}s, "
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
            APP_LOGI("FormStServiceAbilityA::ConnectOtherAbility->ConnectAbility");
            bool ret = ConnectAbility(want, connCallback_);
            sleep(1);
            if (!ret) {
                APP_LOGE("FormStServiceAbilityA::ConnectAbility failed!");
            }
        }
    }
}
void FormStServiceAbilityA::DisConnectOtherAbility()
{
    APP_LOGI("FormStServiceAbilityA::DisConnectOtherAbility begin");
    if (connCallback_ != nullptr) {
        DisconnectAbility(connCallback_);
        sleep(1);
    }
    APP_LOGI("FormStServiceAbilityA::DisConnectOtherAbility end");
}

void FormStServiceAbilityA::StopSelfAbility()
{
    APP_LOGI("FormStServiceAbilityA::StopSelfAbility");

    TerminateAbility();
}

void FormStServiceAbilityA::OnStart(const Want &want)
{
    APP_LOGI("FormStServiceAbilityA::OnStart");

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
void FormStServiceAbilityA::OnCommand(const AAFwk::Want &want, bool restart, int startId)
{
    APP_LOGI("FormStServiceAbilityA::OnCommand");

    GetWantInfo(want);
    Ability::OnCommand(want, restart, startId);
    PublishEvent(APP_A_RESP_EVENT_NAME, AbilityLifecycleExecutor::LifecycleState::ACTIVE, "OnCommand");
}
void FormStServiceAbilityA::OnNewWant(const Want &want)
{
    APP_LOGI("FormStServiceAbilityA::OnNewWant");

    GetWantInfo(want);
    Ability::OnNewWant(want);
}
void FormStServiceAbilityA::OnStop()
{
    APP_LOGI("FormStServiceAbilityA::OnStop");

    Ability::OnStop();
    PublishEvent(APP_A_RESP_EVENT_NAME, AbilityLifecycleExecutor::LifecycleState::INITIAL, "OnStop");
}
void FormStServiceAbilityA::OnActive()
{
    APP_LOGI("FormStServiceAbilityA::OnActive");

    Ability::OnActive();
    PublishEvent(APP_A_RESP_EVENT_NAME, AbilityLifecycleExecutor::LifecycleState::ACTIVE, "OnActive");
}
void FormStServiceAbilityA::OnInactive()
{
    APP_LOGI("FormStServiceAbilityA::OnInactive");

    Ability::OnInactive();
    PublishEvent(APP_A_RESP_EVENT_NAME, AbilityLifecycleExecutor::LifecycleState::INACTIVE, "OnInactive");
}
void FormStServiceAbilityA::OnBackground()
{
    APP_LOGI("FormStServiceAbilityA::OnBackground");

    Ability::OnBackground();
    PublishEvent(APP_A_RESP_EVENT_NAME, AbilityLifecycleExecutor::LifecycleState::BACKGROUND, "OnBackground");
}

void FormStServiceAbilityA::Clear()
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
void FormStServiceAbilityA::GetWantInfo(const Want &want)
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
    FormStServiceAbilityA::AbilityConnectCallback::onAbilityConnectDoneCount = 0;
}
bool FormStServiceAbilityA::PublishEvent(const std::string &eventName, const int &code, const std::string &data)
{
    APP_LOGI("FormStServiceAbilityA::PublishEvent eventName = %{public}s, code = %{public}d, data = %{public}s",
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
sptr<IRemoteObject> FormStServiceAbilityA::OnConnect(const Want &want)
{
    APP_LOGI("FormStServiceAbilityA::OnConnect");
    
    sptr<FormProviderClient> formProviderClient = new (std::nothrow) FormProviderClient();
    std::shared_ptr<Ability> thisAbility = this->shared_from_this();
    formProviderClient->SetOwner(thisAbility);

    return formProviderClient;
}
void FormStServiceAbilityA::OnDisconnect(const Want &want)
{
    APP_LOGI("FormStServiceAbilityA::OnDisconnect");

    Ability::OnDisconnect(want);
    PublishEvent(APP_A_RESP_EVENT_NAME, AbilityLifecycleExecutor::LifecycleState::BACKGROUND, "OnDisconnect");
}
bool FormStServiceAbilityA::SubscribeEvent()
{
    MatchingSkills matchingSkills;
    matchingSkills.AddEvent(APP_A_REQ_EVENT_NAME);
    CommonEventSubscribeInfo subscribeInfo(matchingSkills);
    subscribeInfo.SetPriority(1);
    subscriber_ = std::make_shared<AppEventSubscriber>(subscribeInfo);
    subscriber_->mainAbility_ = this;
    return CommonEventManager::SubscribeCommonEvent(subscriber_);
}
void FormStServiceAbilityA::AppEventSubscriber::OnReceiveEvent(const CommonEventData &data)
{
    auto eventName = data.GetWant().GetAction();
    auto dataContent = data.GetData();
    APP_LOGI("FormStServiceAbilityA::OnReceiveEvent eventName = %{public}s, code = %{public}d, data = %{public}s",
        eventName.c_str(),
        data.GetCode(),
        dataContent.c_str());
    if (APP_A_REQ_EVENT_NAME.compare(eventName) == 0) {
        if (funcMap_.find(dataContent) == funcMap_.end()) {
            APP_LOGI(
                "FormStServiceAbilityA::OnReceiveEvent eventName = %{public}s, code = %{public}d, data = %{public}s",
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

FormProviderInfo FormStServiceAbilityA::OnCreate(const Want &want)
{
    APP_LOGI("%{public}s start", __func__);
    FormProviderInfo formProviderInfo;
    if(!want.HasParameter(Constants::PARAM_FORM_IDENTITY_KEY)) {
         APP_LOGE("%{public}s, formId not exist", __func__);
        return formProviderInfo;
    }
    std::string formId = want.GetStringParam(Constants::PARAM_FORM_IDENTITY_KEY);
    std::string jsonData = std::string("{\"city\":\"beijingA\"}");
    FormProviderData formProviderData = FormProviderData(jsonData);
    formProviderInfo.SetFormData(formProviderData);
    PublishEvent(COMMON_EVENT_TEST_ACTION1, FORM_EVENT_TRIGGER_RESULT::FORM_EVENT_TRIGGER_RESULT_OK, "OnCreate");
    APP_LOGI("%{public}s end, formId: %{public}s", __func__, formId.c_str());
    return formProviderInfo;
}

void FormStServiceAbilityA::OnUpdate(const int64_t formId)
{
    APP_LOGI("%{public}s start", __func__);
    struct timespec ts;
    clock_gettime(CLOCK_REALTIME, &ts);
    long currentTime = ts.tv_sec * SEC_TO_MILLISEC + ts.tv_nsec / MILLISEC_TO_NANOSEC;

    PublishEvent(COMMON_EVENT_TEST_ACTION1, FORM_EVENT_TRIGGER_RESULT::FORM_EVENT_TRIGGER_RESULT_OK, "OnUpdate");
    APP_LOGI("%{public}s end, formId: %{public}s, current time: %{public}ld", __func__,
        std::to_string(formId).c_str(), currentTime);
}

void FormStServiceAbilityA::OnTriggerEvent(const int64_t formId, const std::string &message)
{
    APP_LOGI("%{public}s start", __func__);
    
    struct timespec ts;
    clock_gettime(CLOCK_REALTIME, &ts);
    long currentTime = ts.tv_sec * SEC_TO_MILLISEC + ts.tv_nsec / MILLISEC_TO_NANOSEC;

    PublishEvent(COMMON_EVENT_TEST_ACTION1, FORM_EVENT_TRIGGER_RESULT::FORM_EVENT_TRIGGER_RESULT_OK, "OnTriggerEvent");
    APP_LOGI("%{public}s end, formId: %{public}s, message: %{public}s,  current time: %{public}ld", __func__,
        std::to_string(formId).c_str(), message.c_str(), currentTime);
}

void FormStServiceAbilityA::OnDelete(const int64_t formId)
{
    APP_LOGI("%{public}s start", __func__);

    struct timespec ts;
    clock_gettime(CLOCK_REALTIME, &ts);
    long currentTime = ts.tv_sec * SEC_TO_MILLISEC + ts.tv_nsec / MILLISEC_TO_NANOSEC;

    PublishEvent(COMMON_EVENT_ON_DELETE, FORM_EVENT_TRIGGER_RESULT::FORM_EVENT_TRIGGER_RESULT_OK,
        std::to_string(formId));
    APP_LOGI("%{public}s end, formId: %{public}s, current time: %{public}ld", __func__,
        std::to_string(formId).c_str(), currentTime);
}

void FormStServiceAbilityA::OnCastTemptoNormal(const int64_t formId)
{
    APP_LOGI("%{public}s start", __func__);

    struct timespec ts;
    clock_gettime(CLOCK_REALTIME, &ts);
    long currentTime = ts.tv_sec * SEC_TO_MILLISEC + ts.tv_nsec / MILLISEC_TO_NANOSEC;

    PublishEvent(COMMON_EVENT_TEST_ACTION1, FORM_EVENT_TRIGGER_RESULT::FORM_EVENT_TRIGGER_RESULT_OK, "OnCastTemptoNormal");
    APP_LOGI("%{public}s end, formId: %{public}s, current time: %{public}ld", __func__,
        std::to_string(formId).c_str(), currentTime);
}

void FormStServiceAbilityA::OnVisibilityChanged(const std::map<int64_t, int32_t> &formEventsMap)
{
    APP_LOGI("%{public}s start", __func__);

    struct timespec ts;
    clock_gettime(CLOCK_REALTIME, &ts);
    long currentTime = ts.tv_sec * SEC_TO_MILLISEC + ts.tv_nsec / MILLISEC_TO_NANOSEC;

    PublishEvent(COMMON_EVENT_TEST_ACTION1, FORM_EVENT_TRIGGER_RESULT::FORM_EVENT_TRIGGER_RESULT_OK, "OnVisibilityChanged");
    APP_LOGI("%{public}s end, current time: %{public}ld", __func__, currentTime);
}

REGISTER_AA(FormStServiceAbilityA);
}  // namespace AppExecFwk
}  // namespace OHOS
