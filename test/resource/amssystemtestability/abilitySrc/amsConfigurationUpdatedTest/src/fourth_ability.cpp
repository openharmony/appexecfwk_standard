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

#include "fourth_ability.h"
#include "app_log_wrapper.h"
#include "test_utils.h"

namespace OHOS {
namespace AppExecFwk {
using namespace OHOS::EventFwk;
using namespace OHOS::AAFwk;

void FourthAbility::Init(const std::shared_ptr<AbilityInfo> &abilityInfo,
    const std::shared_ptr<OHOSApplication> &application, std::shared_ptr<AbilityHandler> &handler,
    const sptr<IRemoteObject> &token)
{
    APP_LOGI("FourthAbility::Init");
    Ability::Init(abilityInfo, application, handler, token);
}

FourthAbility::~FourthAbility()
{
    CommonEventManager::UnSubscribeCommonEvent(subscriber_);
}

void FourthAbility::OnStart(const Want &want)
{
    APP_LOGI("FourthAbility::OnStart");
    SubscribeEvent();
    Ability::OnStart(want);
    callbackSeq += "OnStart";
    TestUtils::PublishEvent(g_EVENT_RESP_FOURTH_LIFECYCLE, FOURTH_ABILITY_CODE, "OnStart");
}

void FourthAbility::OnStop()
{
    APP_LOGI("FourthAbility::OnStop");
    Ability::OnStop();
    CommonEventManager::UnSubscribeCommonEvent(subscriber_);
    callbackSeq += "OnStop";  // OnInactiveOnBackgroundOnStop
    TestUtils::PublishEvent(g_EVENT_RESP_FOURTH_LIFECYCLE, FOURTH_ABILITY_CODE, callbackSeq);
    callbackSeq = "";
}

void FourthAbility::OnActive()
{
    APP_LOGI("FourthAbility::OnActive====<");
    Ability::OnActive();
    callbackSeq += "OnActive";  // OnStartOnActive
    TestUtils::PublishEvent(g_EVENT_RESP_FOURTH_LIFECYCLE, FOURTH_ABILITY_CODE, callbackSeq);
    callbackSeq = "";
}

void FourthAbility::OnConfigurationUpdated(const Configuration &configuration)
{
    APP_LOGI("FourthAbility::OnConfigurationUpdated====<");
    Ability::OnConfigurationUpdated(configuration);
    callbackUpdated += "Updated";  // UpdatedUpdated
    TestUtils::PublishEvent(g_EVENT_RESP_FOURTH_LIFECYCLE, FOURTH_ABILITY_CODE, callbackUpdated);
}

void FourthAbility::OnInactive()
{
    APP_LOGI("FourthAbility::OnInactive");
    Ability::OnInactive();
    callbackSeq += "OnInactive";
    TestUtils::PublishEvent(g_EVENT_RESP_FOURTH_LIFECYCLE, FOURTH_ABILITY_CODE, "OnInactive");
}

void FourthAbility::OnBackground()
{
    APP_LOGI("FourthAbility::OnBackground");
    Ability::OnBackground();
    callbackSeq += "OnBackground";
    TestUtils::PublishEvent(g_EVENT_RESP_FOURTH_LIFECYCLE, FOURTH_ABILITY_CODE, "OnBackground");
}

void FourthAbility::OnForeground(const Want &want)
{
    APP_LOGI("FourthAbility::OnForeground");
    Ability::OnForeground(want);
    callbackSeq += "OnForeground";
    TestUtils::PublishEvent(g_EVENT_RESP_FOURTH_LIFECYCLE, FOURTH_ABILITY_CODE, "OnForeground");
}

void FourthAbility::SubscribeEvent()
{
    std::vector<std::string> eventList = {
        // g_EVENT_REQU_FOURTH,
    };
    MatchingSkills matchingSkills;
    for (const auto &e : eventList) {
        matchingSkills.AddEvent(e);
    }
    CommonEventSubscribeInfo subscribeInfo(matchingSkills);
    subscribeInfo.SetPriority(1);
    subscriber_ = std::make_shared<FourthAbilityEventSubscriber>(subscribeInfo);
    subscriber_->fourthAbility = this;
    CommonEventManager::SubscribeCommonEvent(subscriber_);
}

void FourthAbilityEventSubscriber::OnReceiveEvent(const CommonEventData &data)
{
    APP_LOGI("FourthAbilityEventSubscriber::OnReceiveEvent:event=%{public}s", data.GetWant().GetAction().c_str());
    APP_LOGI("FourthAbilityEventSubscriber::OnReceiveEvent:data=%{public}s", data.GetData().c_str());
    APP_LOGI("FourthAbilityEventSubscriber::OnReceiveEvent:code=%{public}d", data.GetCode());
    auto eventName = data.GetWant().GetAction();
}

void FourthAbility::TestAbility(int apiIndex, int caseIndex, int code)
{
    APP_LOGI("FourthAbility::TestAbility");
    if (mapCase_.find(apiIndex) != mapCase_.end()) {
        if (caseIndex < (int)mapCase_[apiIndex].size()) {
            mapCase_[apiIndex][caseIndex](code);
        }
    }
}

REGISTER_AA(FourthAbility)
}  // namespace AppExecFwk
}  // namespace OHOS
