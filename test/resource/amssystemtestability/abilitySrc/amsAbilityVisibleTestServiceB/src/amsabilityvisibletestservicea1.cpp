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

#include "amsabilityvisibletestservicea1.h"
#include "app_log_wrapper.h"
#include "common_event.h"
#include "common_event_manager.h"
using namespace OHOS::EventFwk;

namespace OHOS {
namespace AppExecFwk {

AmsAbilityVisibleTestServiceA1::~AmsAbilityVisibleTestServiceA1()
{}

void AmsAbilityVisibleTestServiceA1::OnStart(const Want &want)
{
    APP_LOGI("AmsAbilityVisibleTestServiceA1::OnStart");
    pageAbilityEvent.SubscribeEvent(STEventName::g_eventList, shared_from_this());
    Ability::OnStart(want);
    std::string eventData = GetAbilityName() + STEventName::g_abilityStateOnStart;
    pageAbilityEvent.PublishEvent(STEventName::g_eventName, pageAbilityEvent.GetOnStartCount(), eventData);
}

void AmsAbilityVisibleTestServiceA1::OnCommand(const AAFwk::Want &want, bool restart, int startId)
{
    APP_LOGI("AmsAbilityVisibleTestServiceA1::OnCommand");

    Ability::OnCommand(want, restart, startId);
    std::string eventData = GetAbilityName() + STEventName::g_abilityStateOnCommand;
    pageAbilityEvent.PublishEvent(STEventName::g_eventName, pageAbilityEvent.GetOnCommandCount(), eventData);
}

void AmsAbilityVisibleTestServiceA1::OnNewWant(const Want &want)
{
    APP_LOGI("AmsAbilityVisibleTestServiceA1::OnNewWant");

    Ability::OnNewWant(want);
    std::string eventData = GetAbilityName() + STEventName::g_abilityStateOnNewWant;
    pageAbilityEvent.PublishEvent(STEventName::g_eventName, pageAbilityEvent.GetOnNewWantCount(), eventData);
}

void AmsAbilityVisibleTestServiceA1::OnStop()
{
    APP_LOGI("AmsAbilityVisibleTestServiceA1::OnStop");

    Ability::OnStop();
    pageAbilityEvent.UnsubscribeEvent();
    std::string eventData = GetAbilityName() + STEventName::g_abilityStateOnStop;
    pageAbilityEvent.PublishEvent(STEventName::g_eventName, pageAbilityEvent.GetOnStopCount(), eventData);
}

void AmsAbilityVisibleTestServiceA1::OnActive()
{
    APP_LOGI("AmsAbilityVisibleTestServiceA1::OnActive");

    Ability::OnActive();
    std::string eventData = GetAbilityName() + STEventName::g_abilityStateOnActive;
    pageAbilityEvent.PublishEvent(STEventName::g_eventName, pageAbilityEvent.GetOnActiveCount(), eventData);
}

void AmsAbilityVisibleTestServiceA1::OnInactive()
{
    APP_LOGI("AmsAbilityVisibleTestServiceA1::OnInactive");

    Ability::OnInactive();
    std::string eventData = GetAbilityName() + STEventName::g_abilityStateOnInactive;
    pageAbilityEvent.PublishEvent(STEventName::g_eventName, pageAbilityEvent.GetOnInactiveCount(), eventData);
}

void AmsAbilityVisibleTestServiceA1::OnBackground()
{
    APP_LOGI("AmsAbilityVisibleTestServiceA1::OnBackground");

    Ability::OnBackground();
    std::string eventData = GetAbilityName() + STEventName::g_abilityStateOnBackground;
    pageAbilityEvent.PublishEvent(STEventName::g_eventName, pageAbilityEvent.GetOnBackgroundCount(), eventData);
}

sptr<IRemoteObject> AmsAbilityVisibleTestServiceA1::OnConnect(const Want &want)
{
    APP_LOGI("AmsAbilityVisibleTestServiceA1::OnConnect");

    sptr<IRemoteObject> ret = Ability::OnConnect(want);
    GetWantInfo(want);
    std::string startBundleName = this->Split(targetBundle_, ",");
    std::string startAbilityName = this->Split(targetAbility_, ",");
    std::string startAbilityType = this->Split(targetType_, ",");
    if (!startBundleName.empty() && !startAbilityName.empty() && !startAbilityType.empty()) {
        Want wantOther;
        wantOther.SetElementName(startBundleName, startAbilityName);
        if (!targetBundle_.empty() && !targetAbility_.empty() && !targetType_.empty()) {
            wantOther.SetParam("targetBundle", targetBundle_);
            wantOther.SetParam("targetAbility", targetAbility_);
            wantOther.SetParam("type", targetType_);
        }
        if (startAbilityType == "Page") {
            StartAbility(wantOther);
        }
        if (startAbilityType == "Service") {
            sptr<AbilityConnectCallback> stub(new (std::nothrow) AbilityConnectCallback());
            ConnectAbility(wantOther, stub);
        }
        if (startAbilityType == "Data") {
            Uri dataAbilityUri("dataability:///" + startBundleName + "." + startAbilityName);
            std::shared_ptr<DataAbilityHelper> helper = DataAbilityHelper::Creator(GetContext());
            NativeRdb::ValuesBucket bucket;
            helper->Insert(dataAbilityUri, bucket);
        }
    }
    Clear();
    std::string eventData = GetAbilityName() + STEventName::g_abilityStateOnConnect;
    pageAbilityEvent.PublishEvent(STEventName::g_eventName, pageAbilityEvent.GetOnConnectCount(), eventData);
    return ret;
}

void AmsAbilityVisibleTestServiceA1::OnDisconnect(const Want &want)
{
    APP_LOGI("AmsAbilityVisibleTestServiceA1::OnDisconnect");

    Ability::OnDisconnect(want);
    std::string eventData = GetAbilityName() + STEventName::g_abilityStateOnDisconnect;
    pageAbilityEvent.PublishEvent(STEventName::g_eventName, pageAbilityEvent.GetOnDisconnectCount(), eventData);
}

void AmsAbilityVisibleTestServiceA1::Clear()
{
    targetType_ = "";
    targetBundle_ = "";
    targetAbility_ = "";
}

void AmsAbilityVisibleTestServiceA1::GetWantInfo(const Want &want)
{
    Want mWant(want);
    targetType_ = mWant.GetStringParam("type");
    targetBundle_ = mWant.GetStringParam("targetBundle");
    targetAbility_ = mWant.GetStringParam("targetAbility");
}

std::string AmsAbilityVisibleTestServiceA1::Split(std::string &str, std::string delim)
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

REGISTER_AA(AmsAbilityVisibleTestServiceA1);
}  // namespace AppExecFwk
}  // namespace OHOS