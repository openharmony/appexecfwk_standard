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

#include "ams_dfx_st_service_ability_a2.h"
#include "app_log_wrapper.h"
#include "common_event.h"
#include "common_event_manager.h"
using namespace OHOS::EventFwk;

namespace OHOS {
namespace AppExecFwk {

AmsStAbilityDFXA2::~AmsStAbilityDFXA2()
{}

void AmsStAbilityDFXA2::OnStart(const Want &want)
{
    APP_LOGI("AmsStAbilityDFXA2::OnStart");
    pageAbilityEvent.SubscribeEvent(STEventName::g_eventList, shared_from_this());
    Ability::OnStart(want);
    std::string eventData = GetAbilityName() + STEventName::g_abilityStateOnStart;
    pageAbilityEvent.PublishEvent(STEventName::g_eventName, pageAbilityEvent.GetOnStartCount(), eventData);
}

void AmsStAbilityDFXA2::OnCommand(const AAFwk::Want &want, bool restart, int startId)
{
    APP_LOGI("AmsStAbilityDFXA2::OnCommand");

    Ability::OnCommand(want, restart, startId);
    std::string eventData = GetAbilityName() + STEventName::g_abilityStateOnCommand;
    pageAbilityEvent.PublishEvent(STEventName::g_eventName, pageAbilityEvent.GetOnCommandCount(), eventData);
}

void AmsStAbilityDFXA2::OnNewWant(const Want &want)
{
    APP_LOGI("AmsStAbilityDFXA2::OnNewWant");

    Ability::OnNewWant(want);
    std::string eventData = GetAbilityName() + STEventName::g_abilityStateOnNewWant;
    pageAbilityEvent.PublishEvent(STEventName::g_eventName, pageAbilityEvent.GetOnNewWantCount(), eventData);
}

void AmsStAbilityDFXA2::OnStop()
{
    APP_LOGI("AmsStAbilityDFXA2::OnStop");

    Ability::OnStop();
    pageAbilityEvent.UnsubscribeEvent();
    std::string eventData = GetAbilityName() + STEventName::g_abilityStateOnStop;
    pageAbilityEvent.PublishEvent(STEventName::g_eventName, pageAbilityEvent.GetOnStopCount(), eventData);
}

void AmsStAbilityDFXA2::OnActive()
{
    APP_LOGI("AmsStAbilityDFXA2::OnActive");

    Ability::OnActive();
    std::string eventData = GetAbilityName() + STEventName::g_abilityStateOnActive;
    pageAbilityEvent.PublishEvent(STEventName::g_eventName, pageAbilityEvent.GetOnActiveCount(), eventData);
}

void AmsStAbilityDFXA2::OnInactive()
{
    APP_LOGI("AmsStAbilityDFXA2::OnInactive");

    Ability::OnInactive();
    std::string eventData = GetAbilityName() + STEventName::g_abilityStateOnInactive;
    pageAbilityEvent.PublishEvent(STEventName::g_eventName, pageAbilityEvent.GetOnInactiveCount(), eventData);
}

void AmsStAbilityDFXA2::OnBackground()
{
    APP_LOGI("AmsStAbilityDFXA2::OnBackground");

    Ability::OnBackground();
    std::string eventData = GetAbilityName() + STEventName::g_abilityStateOnBackground;
    pageAbilityEvent.PublishEvent(STEventName::g_eventName, pageAbilityEvent.GetOnBackgroundCount(), eventData);
}

sptr<IRemoteObject> AmsStAbilityDFXA2::OnConnect(const Want &want)
{
    APP_LOGI("AmsStAbilityDFXA2::OnConnect");

    sptr<IRemoteObject> ret = Ability::OnConnect(want);
    std::string eventData = GetAbilityName() + STEventName::g_abilityStateOnConnect;
    pageAbilityEvent.PublishEvent(STEventName::g_eventName, pageAbilityEvent.GetOnConnectCount(), eventData);
    return ret;
}

void AmsStAbilityDFXA2::OnDisconnect(const Want &want)
{
    APP_LOGI("AmsStAbilityDFXA2::OnDisconnect");

    Ability::OnDisconnect(want);
    std::string eventData = GetAbilityName() + STEventName::g_abilityStateOnDisconnect;
    pageAbilityEvent.PublishEvent(STEventName::g_eventName, pageAbilityEvent.GetOnDisconnectCount(), eventData);
    std::this_thread::sleep_for(std::chrono::milliseconds(STEventName::DISCONNECT_TIMEOUT));
}

REGISTER_AA(AmsStAbilityDFXA2);
}  // namespace AppExecFwk
}  // namespace OHOS