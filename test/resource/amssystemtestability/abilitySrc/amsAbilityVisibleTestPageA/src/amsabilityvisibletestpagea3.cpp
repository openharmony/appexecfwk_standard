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

#include "amsabilityvisibletestpagea3.h"

namespace OHOS {
namespace AppExecFwk {
void AmsAbilityVisibleTestPageA3::OnStart(const Want &want)
{
    GetWantInfo(want);
    APP_LOGI("AmsAbilityVisibleTestPageA3::onStart");
    stub_ = (new (std::nothrow) AbilityConnectCallback());
    pageAbilityEvent.SubscribeEvent(STEventName::g_eventList, shared_from_this(), stub_);
    Ability::OnStart(want);
    std::string eventData = GetAbilityName() + STEventName::g_abilityStateOnStart;
    pageAbilityEvent.PublishEvent(STEventName::g_eventName, pageAbilityEvent.GetOnStartCount(), eventData);
}

void AmsAbilityVisibleTestPageA3::OnNewWant(const Want &want)
{
    APP_LOGI("AmsAbilityVisibleTestPageA3::OnNewWant");
    Ability::OnNewWant(want);
    std::string eventData = GetAbilityName() + STEventName::g_abilityStateOnNewWant;
    pageAbilityEvent.PublishEvent(STEventName::g_eventName, pageAbilityEvent.GetOnNewWantCount(), eventData);
}

void AmsAbilityVisibleTestPageA3::OnForeground(const Want &want)
{
    APP_LOGI("AmsAbilityVisibleTestPageA3::OnForeground");
    Ability::OnForeground(want);
    std::string eventData = GetAbilityName() + STEventName::g_abilityStateOnForeground;
    pageAbilityEvent.PublishEvent(STEventName::g_eventName, pageAbilityEvent.GetOnForegroundCount(), eventData);
}

void AmsAbilityVisibleTestPageA3::OnStop()
{
    APP_LOGI("AmsAbilityVisibleTestPageA3::onStop");
    Ability::OnStop();
    pageAbilityEvent.UnsubscribeEvent();
    std::string eventData = GetAbilityName() + STEventName::g_abilityStateOnStop;
    pageAbilityEvent.PublishEvent(STEventName::g_eventName, pageAbilityEvent.GetOnStopCount(), eventData);
}

void AmsAbilityVisibleTestPageA3::OnActive()
{
    APP_LOGI("AmsAbilityVisibleTestPageA3::OnActive");
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
            AppExecFwk::ValuesBucket bucket;
            helper->Insert(dataAbilityUri, bucket);
        }
    }
    Clear();
    std::string eventData = GetAbilityName() + STEventName::g_abilityStateOnActive;
    pageAbilityEvent.PublishEvent(STEventName::g_eventName, pageAbilityEvent.GetOnActiveCount(), eventData);
}

void AmsAbilityVisibleTestPageA3::OnInactive()
{
    APP_LOGI("AmsAbilityVisibleTestPageA3::OnInactive");
    Ability::OnInactive();
    std::string eventData = GetAbilityName() + STEventName::g_abilityStateOnInactive;
    pageAbilityEvent.PublishEvent(STEventName::g_eventName, pageAbilityEvent.GetOnInactiveCount(), eventData);
}

void AmsAbilityVisibleTestPageA3::OnBackground()
{
    APP_LOGI("AmsAbilityVisibleTestPageA3::OnBackground");
    Ability::OnBackground();
    std::string eventData = GetAbilityName() + STEventName::g_abilityStateOnBackground;
    pageAbilityEvent.PublishEvent(STEventName::g_eventName, pageAbilityEvent.GetOnBackgroundCount(), eventData);
}

void AmsAbilityVisibleTestPageA3::Clear()
{
    targetType_ = "";
    targetBundle_ = "";
    targetAbility_ = "";
}

void AmsAbilityVisibleTestPageA3::GetWantInfo(const Want &want)
{
    Want mWant(want);
    targetType_ = mWant.GetStringParam("type");
    targetBundle_ = mWant.GetStringParam("targetBundle");
    targetAbility_ = mWant.GetStringParam("targetAbility");
}

std::string AmsAbilityVisibleTestPageA3::Split(std::string &str, std::string delim)
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

REGISTER_AA(AmsAbilityVisibleTestPageA3);
}  // namespace AppExecFwk
}  // namespace OHOS