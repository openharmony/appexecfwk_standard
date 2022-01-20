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

#include "amsabilityvisibletestpageb2.h"
#include "context/context.h"

namespace OHOS {
namespace AppExecFwk {
void AmsAbilityVisibleTestPageB2::Init(const std::shared_ptr<AbilityInfo> &abilityInfo,
    const std::shared_ptr<OHOSApplication> &application, std::shared_ptr<AbilityHandler> &handler,
    const sptr<IRemoteObject> &token)
{
    APP_LOGI("AmsAbilityVisibleTestPageB2::Init");
    Ability::Init(abilityInfo, application, handler, token);
    std::string eventData = GetAbilityName() + STEventName::g_abilityStateInit;
    pageAbilityEvent.PublishEvent(STEventName::g_eventName, STEventName::g_defeventCode, eventData);
    stub_ = (new (std::nothrow) AbilityConnectCallback());
    pageAbilityEvent.SubscribeEvent(STEventName::g_eventList, shared_from_this(), stub_);
}

void AmsAbilityVisibleTestPageB2::OnStart(const Want &want)
{
    GetWantInfo(want);
    APP_LOGI("AmsAbilityVisibleTestPageB2::onStart");
    Ability::OnStart(want);
    std::string eventData = GetAbilityName() + STEventName::g_abilityStateOnStart;
    pageAbilityEvent.PublishEvent(STEventName::g_eventName, pageAbilityEvent.GetOnStartCount(), eventData);
}

void AmsAbilityVisibleTestPageB2::OnNewWant(const Want &want)
{
    APP_LOGI("AmsAbilityVisibleTestPageB2::OnNewWant");
    Ability::OnNewWant(want);
    std::string eventData = GetAbilityName() + STEventName::g_abilityStateOnNewWant;
    pageAbilityEvent.PublishEvent(STEventName::g_eventName, pageAbilityEvent.GetOnNewWantCount(), eventData);
}

void AmsAbilityVisibleTestPageB2::OnForeground(const Want &want)
{
    APP_LOGI("AmsAbilityVisibleTestPageB2::OnForeground");
    Ability::OnForeground(want);
    std::string eventData = GetAbilityName() + STEventName::g_abilityStateOnForeground;
    pageAbilityEvent.PublishEvent(STEventName::g_eventName, pageAbilityEvent.GetOnForegroundCount(), eventData);
}

void AmsAbilityVisibleTestPageB2::OnStop()
{
    APP_LOGI("AmsAbilityVisibleTestPageB2::onStop");
    Ability::OnStop();
    pageAbilityEvent.UnsubscribeEvent();
    std::string eventData = GetAbilityName() + STEventName::g_abilityStateOnStop;
    pageAbilityEvent.PublishEvent(STEventName::g_eventName, pageAbilityEvent.GetOnStopCount(), eventData);
}

void AmsAbilityVisibleTestPageB2::OnActive()
{
    APP_LOGI("AmsAbilityVisibleTestPageB2::OnActive");
    Ability::OnActive();
    Ability::OnActive();
    std::string startBundleName = this->Split(targetBundle_, ",");
    std::string startAbilityName = this->Split(targetAbility_, ",");
    std::string startAbilityType = this->Split(targetType_, ",");
    if (!startBundleName.empty() && !startAbilityName.empty() && !startAbilityType.empty()) {
        Want want;
        want.SetElementName(startBundleName, startAbilityName);
        if (!targetBundle_.empty() && !targetAbility_.empty() && !targetType_.empty()) {
            want.SetParam("targetBundle", targetBundle_);
            want.SetParam("targetAbility", targetAbility_);
            want.SetParam("type", targetType_);
        }
        if (startAbilityType == "Page") {
            StartAbility(want);
        }
        if (startAbilityType == "Service") {
            if (stub_) {
                ConnectAbility(want, stub_);
            }
        }
        if (startAbilityType == "Data" || startAbilityType == "DataRelease") {
            Uri dataAbilityUri("dataability:///" + startBundleName + "." + startAbilityName);
            std::shared_ptr<DataAbilityHelper> helper = DataAbilityHelper::Creator(GetContext());
            NativeRdb::ValuesBucket bucket;
            helper->Insert(dataAbilityUri, bucket);
            if (startAbilityType == "DataRelease") {
                helper->Release();
            }
        }
        if (startAbilityType == "TriggerWantAgentPageAbility") {
            GetAndTriggerWantAgent(startAbilityType, want, WantAgentConstant::OperationType::START_ABILITY);
        }
        if (startAbilityType == "TriggerWantAgentServiceAbility") {
            GetAndTriggerWantAgent(startAbilityType, want, WantAgentConstant::OperationType::START_SERVICE);
        }
    }
    Clear();
    std::string eventData = GetAbilityName() + STEventName::g_abilityStateOnActive;
    pageAbilityEvent.PublishEvent(STEventName::g_eventName, pageAbilityEvent.GetOnActiveCount(), eventData);
}

void AmsAbilityVisibleTestPageB2::OnInactive()
{
    APP_LOGI("AmsAbilityVisibleTestPageB2::OnInactive");
    Ability::OnInactive();
    std::string eventData = GetAbilityName() + STEventName::g_abilityStateOnInactive;
    pageAbilityEvent.PublishEvent(STEventName::g_eventName, pageAbilityEvent.GetOnInactiveCount(), eventData);
}

void AmsAbilityVisibleTestPageB2::OnBackground()
{
    APP_LOGI("AmsAbilityVisibleTestPageB2::OnBackground");
    Ability::OnBackground();
    std::string eventData = GetAbilityName() + STEventName::g_abilityStateOnBackground;
    pageAbilityEvent.PublishEvent(STEventName::g_eventName, pageAbilityEvent.GetOnBackgroundCount(), eventData);
}

void AmsAbilityVisibleTestPageB2::Clear()
{
    targetType_ = "";
    targetBundle_ = "";
    targetAbility_ = "";
}

void AmsAbilityVisibleTestPageB2::GetWantInfo(const Want &want)
{
    Want mWant(want);
    targetType_ = mWant.GetStringParam("type");
    targetBundle_ = mWant.GetStringParam("targetBundle");
    targetAbility_ = mWant.GetStringParam("targetAbility");
}

std::string AmsAbilityVisibleTestPageB2::Split(std::string &str, std::string delim)
{
    std::string result;
    if (!str.empty()) {
        size_t index = str.find(delim);
        if (index != std::string::npos) {
            result = str.substr(0, index);
            str = str.substr(index + delim.size());
        } else {
            result = str;
            str = "";
        }
    }
    return result;
}

void AmsAbilityVisibleTestPageB2::GetAndTriggerWantAgent(
    std::string type, Want want, WantAgentConstant::OperationType operationType)
{
    std::vector<std::shared_ptr<AAFwk::Want>> vec;
    std::shared_ptr<Want> wantParameter = std::make_shared<Want>(want);
    vec.push_back(wantParameter);
    WantAgentInfo info(0, operationType, WantAgentConstant::Flags::UPDATE_PRESENT_FLAG, vec, nullptr);
    APP_LOGI("AmsAbilityVisibleTestPageB2::StartOtherAbility()");
    std::shared_ptr<AbilityRuntime::Context> context = OHOS::AbilityRuntime::Context::GetApplicationContext();
    std::shared_ptr<WantAgent> agent = WantAgentHelper::GetWantAgent(context, info);
    if (agent == nullptr) {
        pageAbilityEvent.PublishEvent(
            STEventName::g_eventName, STEventName::g_defeventCode, STEventName::g_getWantAgentState);
        APP_LOGI("AmsAbilityVisibleTestPageB2::StartOtherAbility():GetWantAgent is nullptr.");
        return;
    }
    TriggerInfo paramsInfo;
    std::shared_ptr<PendingWantCallback> callback = std::make_shared<PendingWantCallback>();
    WantAgentHelper::TriggerWantAgent(agent, callback, paramsInfo);
    APP_LOGI("AmsAbilityVisibleTestPageB2::StartOtherAbility():WantAgentHelper::TriggerWantAgent end.");
}

REGISTER_AA(AmsAbilityVisibleTestPageB2);
}  // namespace AppExecFwk
}  // namespace OHOS