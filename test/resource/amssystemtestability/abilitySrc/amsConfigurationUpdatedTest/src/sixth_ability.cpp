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

#include "sixth_ability.h"
#include "app_log_wrapper.h"
#include "test_utils.h"

namespace OHOS {
namespace AppExecFwk {
using namespace OHOS::EventFwk;
using namespace OHOS::AAFwk;

void SixthAbility::Init(const std::shared_ptr<AbilityInfo> &abilityInfo,
    const std::shared_ptr<OHOSApplication> &application, std::shared_ptr<AbilityHandler> &handler,
    const sptr<IRemoteObject> &token)
{
    APP_LOGI("SixthAbility::Init");
    Ability::Init(abilityInfo, application, handler, token);
}

SixthAbility::~SixthAbility()
{
    CommonEventManager::UnSubscribeCommonEvent(subscriber_);
}

void SixthAbility::OnStart(const Want &want)
{
    APP_LOGI("SixthAbility::OnStart");
    SubscribeEvent();
    Ability::OnStart(want);
    callbackSeq += "OnStart";
    TestUtils::PublishEvent(g_EVENT_RESP_SIXTH_LIFECYCLE, SIXTH_ABILITY_CODE, "OnStart");
}

void SixthAbility::OnStop()
{
    APP_LOGI("SixthAbility::OnStop");
    Ability::OnStop();
    CommonEventManager::UnSubscribeCommonEvent(subscriber_);
    callbackSeq += "OnStop";  // OnInactiveOnBackgroundOnStop
    TestUtils::PublishEvent(g_EVENT_RESP_SIXTH_LIFECYCLE, SIXTH_ABILITY_CODE, callbackSeq);
    callbackSeq = "";
}

void SixthAbility::OnActive()
{
    APP_LOGI("SixthAbility::OnActive====<");
    Ability::OnActive();
    callbackSeq += "OnActive";  // OnStartOnActive
    TestUtils::PublishEvent(g_EVENT_RESP_SIXTH_LIFECYCLE, SIXTH_ABILITY_CODE, callbackSeq);
    callbackSeq = "";
}

void SixthAbility::OnConfigurationUpdated(const Configuration &configuration)
{
    APP_LOGI("SixthAbility::OnConfigurationUpdated====<");
    Ability::OnConfigurationUpdated(configuration);
    TestUtils::PublishEvent(g_EVENT_RESP_SIXTH_LIFECYCLE, SIXTH_ABILITY_CODE, "OnConfigurationUpdated");
}

void SixthAbility::OnInactive()
{
    APP_LOGI("SixthAbility::OnInactive");
    Ability::OnInactive();
    callbackSeq += "OnInactive";
    TestUtils::PublishEvent(g_EVENT_RESP_SIXTH_LIFECYCLE, SIXTH_ABILITY_CODE, "OnInactive");
}

void SixthAbility::OnBackground()
{
    APP_LOGI("SixthAbility::OnBackground");
    Ability::OnBackground();
    callbackSeq += "OnBackground";
    TestUtils::PublishEvent(g_EVENT_RESP_SIXTH_LIFECYCLE, SIXTH_ABILITY_CODE, "OnBackground");
}

void SixthAbility::OnForeground(const Want &want)
{
    APP_LOGI("SixthAbility::OnForeground");
    Ability::OnForeground(want);
    callbackSeq += "OnForeground";
    TestUtils::PublishEvent(g_EVENT_RESP_SIXTH_LIFECYCLE, SIXTH_ABILITY_CODE, "OnForeground");
}

void SixthAbility::SubscribeEvent()
{
    std::vector<std::string> eventList = {
        // g_EVENT_REQU_SIXTH,
    };
    MatchingSkills matchingSkills;
    for (const auto &e : eventList) {
        matchingSkills.AddEvent(e);
    }
    CommonEventSubscribeInfo subscribeInfo(matchingSkills);
    subscribeInfo.SetPriority(1);
    subscriber_ = std::make_shared<SixthAbilityEventSubscriber>(subscribeInfo);
    subscriber_->sixthAbility = this;
    CommonEventManager::SubscribeCommonEvent(subscriber_);
}

void SixthAbilityEventSubscriber::OnReceiveEvent(const CommonEventData &data)
{
    APP_LOGI("SixthAbilityEventSubscriber::OnReceiveEvent:event=%{public}s", data.GetWant().GetAction().c_str());
    APP_LOGI("SixthAbilityEventSubscriber::OnReceiveEvent:data=%{public}s", data.GetData().c_str());
    APP_LOGI("SixthAbilityEventSubscriber::OnReceiveEvent:code=%{public}d", data.GetCode());
    auto eventName = data.GetWant().GetAction();
}

void SixthAbility::TestAbility(int apiIndex, int caseIndex, int code)
{
    APP_LOGI("SixthAbility::TestAbility");
    if (mapCase_.find(apiIndex) != mapCase_.end()) {
        if (caseIndex < (int)mapCase_[apiIndex].size()) {
            mapCase_[apiIndex][caseIndex](code);
        }
    }
}

REGISTER_AA(SixthAbility)
}  // namespace AppExecFwk
}  // namespace OHOS
