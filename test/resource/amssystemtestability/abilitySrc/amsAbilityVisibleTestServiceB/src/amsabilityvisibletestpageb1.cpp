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

#include "amsabilityvisibletestpageb1.h"

namespace OHOS {
namespace AppExecFwk {
void AmsAbilityVisibleTestPageB1::Init(const std::shared_ptr<AbilityInfo> &abilityInfo,
    const std::shared_ptr<OHOSApplication> &application, std::shared_ptr<AbilityHandler> &handler,
    const sptr<IRemoteObject> &token)
{
    APP_LOGI("AmsAbilityVisibleTestPageB1::Init");
    Ability::Init(abilityInfo, application, handler, token);
    stub_ = (new (std::nothrow) AbilityConnectCallback());
    pageAbilityEvent.SubscribeEvent(STEventName::g_eventList, shared_from_this(), stub_);
    std::string eventData = GetAbilityName() + STEventName::g_abilityStateInit;
    pageAbilityEvent.PublishEvent(STEventName::g_eventName, STEventName::g_defeventCode, eventData);
}

void AmsAbilityVisibleTestPageB1::OnStart(const Want &want)
{
    GetWantInfo(want);
    APP_LOGI("AmsAbilityVisibleTestPageB1::onStart");
    Ability::OnStart(want);
    std::string eventData = GetAbilityName() + STEventName::g_abilityStateOnStart;
    pageAbilityEvent.PublishEvent(STEventName::g_eventName, pageAbilityEvent.GetOnStartCount(), eventData);
}

void AmsAbilityVisibleTestPageB1::OnNewWant(const Want &want)
{
    APP_LOGI("AmsAbilityVisibleTestPageB1::OnNewWant");
    Ability::OnNewWant(want);
    std::string eventData = GetAbilityName() + STEventName::g_abilityStateOnNewWant;
    pageAbilityEvent.PublishEvent(STEventName::g_eventName, pageAbilityEvent.GetOnNewWantCount(), eventData);
}

void AmsAbilityVisibleTestPageB1::OnForeground(const Want &want)
{
    APP_LOGI("AmsAbilityVisibleTestPageB1::OnForeground");
    Ability::OnForeground(want);
    std::string eventData = GetAbilityName() + STEventName::g_abilityStateOnForeground;
    pageAbilityEvent.PublishEvent(STEventName::g_eventName, pageAbilityEvent.GetOnForegroundCount(), eventData);
}

void AmsAbilityVisibleTestPageB1::OnStop()
{
    APP_LOGI("AmsAbilityVisibleTestPageB1::onStop");
    Ability::OnStop();
    pageAbilityEvent.UnsubscribeEvent();
    std::string eventData = GetAbilityName() + STEventName::g_abilityStateOnStop;
    pageAbilityEvent.PublishEvent(STEventName::g_eventName, pageAbilityEvent.GetOnStopCount(), eventData);
}

void AmsAbilityVisibleTestPageB1::OnActive()
{
    APP_LOGI("AmsAbilityVisibleTestPageB1::OnActive");
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
        if (startAbilityType == "Data") {
            Uri dataAbilityUri("dataability:///" + startBundleName + "." + startAbilityName);
            std::shared_ptr<DataAbilityHelper> helper = DataAbilityHelper::Creator(GetContext());
            NativeRdb::ValuesBucket bucket;
            helper->Insert(dataAbilityUri, bucket);
        }
    }
    Clear();
    std::string eventData = GetAbilityName() + STEventName::g_abilityStateOnActive;
    pageAbilityEvent.PublishEvent(STEventName::g_eventName, pageAbilityEvent.GetOnActiveCount(), eventData);
}

void AmsAbilityVisibleTestPageB1::OnInactive()
{
    APP_LOGI("AmsAbilityVisibleTestPageB1::OnInactive");
    Ability::OnInactive();
    std::string eventData = GetAbilityName() + STEventName::g_abilityStateOnInactive;
    pageAbilityEvent.PublishEvent(STEventName::g_eventName, pageAbilityEvent.GetOnInactiveCount(), eventData);
}

void AmsAbilityVisibleTestPageB1::OnBackground()
{
    APP_LOGI("AmsAbilityVisibleTestPageB1::OnBackground");
    Ability::OnBackground();
    std::string eventData = GetAbilityName() + STEventName::g_abilityStateOnBackground;
    pageAbilityEvent.PublishEvent(STEventName::g_eventName, pageAbilityEvent.GetOnBackgroundCount(), eventData);
}

void AmsAbilityVisibleTestPageB1::Clear()
{
    targetType_ = "";
    targetBundle_ = "";
    targetAbility_ = "";
}

void AmsAbilityVisibleTestPageB1::GetWantInfo(const Want &want)
{
    Want mWant(want);
    targetType_ = mWant.GetStringParam("type");
    targetBundle_ = mWant.GetStringParam("targetBundle");
    targetAbility_ = mWant.GetStringParam("targetAbility");
}

std::string AmsAbilityVisibleTestPageB1::Split(std::string &str, std::string delim)
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

REGISTER_AA(AmsAbilityVisibleTestPageB1);
}  // namespace AppExecFwk
}  // namespace OHOS