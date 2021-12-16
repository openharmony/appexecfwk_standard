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

#include "second_ability.h"
#include "app_log_wrapper.h"
#include "test_utils.h"

namespace OHOS {
namespace AppExecFwk {
using namespace OHOS::EventFwk;
using namespace OHOS::AAFwk;

void SecondAbility::Init(const std::shared_ptr<AbilityInfo> &abilityInfo,
    const std::shared_ptr<OHOSApplication> &application, std::shared_ptr<AbilityHandler> &handler,
    const sptr<IRemoteObject> &token)
{
    APP_LOGI("SecondAbility::Init");
    Ability::Init(abilityInfo, application, handler, token);
}

SecondAbility::~SecondAbility()
{
    CommonEventManager::UnSubscribeCommonEvent(subscriber_);
}

void SecondAbility::OnStart(const Want &want)
{
    APP_LOGI("SecondAbility::OnStart");
    SubscribeEvent();
    Ability::OnStart(want);
    callbackSeq += "OnStart";
    TestUtils::PublishEvent(g_EVENT_RESP_MAIN_LIFECYCLE, SECOND_ABILITY_CODE, "OnStart");
}

void SecondAbility::OnStop()
{
    APP_LOGI("SecondAbility::OnStop");
    Ability::OnStop();
    CommonEventManager::UnSubscribeCommonEvent(subscriber_);
    callbackSeq += "OnStop";  // OnInactiveOnBackgroundOnStop
    TestUtils::PublishEvent(g_EVENT_RESP_MAIN_LIFECYCLE, SECOND_ABILITY_CODE, callbackSeq);
    callbackSeq = "";
}

void SecondAbility::OnActive()
{
    APP_LOGI("SecondAbility::OnActive====<");
    Ability::OnActive();
    callbackSeq += "OnActive";  // OnStartOnActive
    TestUtils::PublishEvent(g_EVENT_RESP_MAIN_LIFECYCLE, SECOND_ABILITY_CODE, callbackSeq);
    callbackSeq = "";
}

void SecondAbility::OnConfigurationUpdated(const Configuration &configuration)
{
    std::string languageValue;
    std::string orientationValue;

    APP_LOGI("SecondAbility::OnConfigurationUpdated====<");
    Ability::OnConfigurationUpdated(configuration);

    languageValue = configuration.GetItem(GlobalConfigurationKey::SYSTEM_LANGUAGE);
    orientationValue = configuration.GetItem(GlobalConfigurationKey::SYSTEM_ORIENTATION);
    TestUtils::PublishEvent(g_EVENT_RESP_MAIN_LIFECYCLE, SECOND_ABILITY_CODE, languageValue);
    TestUtils::PublishEvent(g_EVENT_RESP_MAIN_LIFECYCLE, SECOND_ABILITY_CODE, orientationValue);
    callbackUpdated += "Updated";
    TestUtils::PublishEvent(g_EVENT_RESP_MAIN_LIFECYCLE, SECOND_ABILITY_CODE, callbackUpdated);
    callbackUpdated = "";
}

void SecondAbility::OnInactive()
{
    APP_LOGI("SecondAbility::OnInactive");
    Ability::OnInactive();
    callbackSeq += "OnInactive";
    TestUtils::PublishEvent(g_EVENT_RESP_MAIN_LIFECYCLE, SECOND_ABILITY_CODE, "OnInactive");
}

void SecondAbility::OnBackground()
{
    APP_LOGI("SecondAbility::OnBackground");
    Ability::OnBackground();
    callbackSeq += "OnBackground";
    TestUtils::PublishEvent(g_EVENT_RESP_MAIN_LIFECYCLE, SECOND_ABILITY_CODE, "OnBackground");
}

void SecondAbility::OnForeground(const Want &want)
{
    APP_LOGI("SecondAbility::OnForeground");
    Ability::OnForeground(want);
    callbackSeq += "OnForeground";
    TestUtils::PublishEvent(g_EVENT_RESP_MAIN_LIFECYCLE, SECOND_ABILITY_CODE, "OnForeground");
}

void SecondAbility::SubscribeEvent()
{
    std::vector<std::string> eventList = {
        g_EVENT_REQU_MAIN,
    };
    MatchingSkills matchingSkills;
    for (const auto &e : eventList) {
        matchingSkills.AddEvent(e);
    }
    CommonEventSubscribeInfo subscribeInfo(matchingSkills);
    subscribeInfo.SetPriority(1);
    subscriber_ = std::make_shared<SecondAbilityEventSubscriber>(subscribeInfo);
    subscriber_->secondAbility = this;
    CommonEventManager::SubscribeCommonEvent(subscriber_);
}

void SecondAbilityEventSubscriber::OnReceiveEvent(const CommonEventData &data)
{
    APP_LOGI("SecondAbilityEventSubscriber::OnReceiveEvent:event=%{public}s", data.GetWant().GetAction().c_str());
    APP_LOGI("SecondAbilityEventSubscriber::OnReceiveEvent:data=%{public}s", data.GetData().c_str());
    APP_LOGI("SecondAbilityEventSubscriber::OnReceiveEvent:code=%{public}d", data.GetCode());
    auto eventName = data.GetWant().GetAction();
}

void SecondAbility::TestAbility(int apiIndex, int caseIndex, int code)
{
    APP_LOGI("SecondAbility::TestAbility");
    if (mapCase_.find(apiIndex) != mapCase_.end()) {
        if (caseIndex < (int)mapCase_[apiIndex].size()) {
            mapCase_[apiIndex][caseIndex](code);
        }
    }
}

REGISTER_AA(SecondAbility)
}  // namespace AppExecFwk
}  // namespace OHOS
