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

#include "fifth_ability.h"
#include "app_log_wrapper.h"
#include "test_utils.h"

namespace OHOS {
namespace AppExecFwk {
using namespace OHOS::EventFwk;
using namespace OHOS::AAFwk;

void FifthAbility::Init(const std::shared_ptr<AbilityInfo> &abilityInfo,
    const std::shared_ptr<OHOSApplication> &application, std::shared_ptr<AbilityHandler> &handler,
    const sptr<IRemoteObject> &token)
{
    APP_LOGI("FifthAbility::Init");
    Ability::Init(abilityInfo, application, handler, token);
}

FifthAbility::~FifthAbility()
{
    CommonEventManager::UnSubscribeCommonEvent(subscriber_);
}

void FifthAbility::OnStart(const Want &want)
{
    APP_LOGI("FifthAbility::OnStart");
    SubscribeEvent();
    Ability::OnStart(want);
    callbackSeq += "OnStart";
    TestUtils::PublishEvent(g_EVENT_RESP_FIFTH_LIFECYCLE, FIFTH_ABILITY_CODE, "OnStart");
}

void FifthAbility::OnStop()
{
    APP_LOGI("FifthAbility::OnStop");
    Ability::OnStop();
    CommonEventManager::UnSubscribeCommonEvent(subscriber_);
    callbackSeq += "OnStop";  // OnInactiveOnBackgroundOnStop
    TestUtils::PublishEvent(g_EVENT_RESP_FIFTH_LIFECYCLE, FIFTH_ABILITY_CODE, callbackSeq);
    callbackSeq = "";
}

void FifthAbility::OnActive()
{
    APP_LOGI("FifthAbility::OnActive====<");
    Ability::OnActive();
    callbackSeq += "OnActive";  // OnStartOnActive
    TestUtils::PublishEvent(g_EVENT_RESP_FIFTH_LIFECYCLE, FIFTH_ABILITY_CODE, callbackSeq);
    callbackSeq = "";
}

void FifthAbility::OnConfigurationUpdated(const Configuration &configuration)
{
    APP_LOGI("FifthAbility::OnConfigurationUpdated====<");
    Ability::OnConfigurationUpdated(configuration);
    callbackUpdated += "Updated";  // UpdatedUpdated
    TestUtils::PublishEvent(g_EVENT_RESP_FIFTH_LIFECYCLE, FIFTH_ABILITY_CODE, callbackUpdated);
}

void FifthAbility::OnInactive()
{
    APP_LOGI("FifthAbility::OnInactive");
    Ability::OnInactive();
    callbackSeq += "OnInactive";
    TestUtils::PublishEvent(g_EVENT_RESP_FIFTH_LIFECYCLE, FIFTH_ABILITY_CODE, "OnInactive");
}

void FifthAbility::OnBackground()
{
    APP_LOGI("FifthAbility::OnBackground");
    Ability::OnBackground();
    callbackSeq += "OnBackground";
    TestUtils::PublishEvent(g_EVENT_RESP_FIFTH_LIFECYCLE, FIFTH_ABILITY_CODE, "OnBackground");
}

void FifthAbility::OnForeground(const Want &want)
{
    APP_LOGI("FifthAbility::OnForeground");
    Ability::OnForeground(want);
    callbackSeq += "OnForeground";
    TestUtils::PublishEvent(g_EVENT_RESP_FIFTH_LIFECYCLE, FIFTH_ABILITY_CODE, "OnForeground");
}

void FifthAbility::SubscribeEvent()
{
    std::vector<std::string> eventList = {
        // g_EVENT_REQU_FIFTH,
    };
    MatchingSkills matchingSkills;
    for (const auto &e : eventList) {
        matchingSkills.AddEvent(e);
    }
    CommonEventSubscribeInfo subscribeInfo(matchingSkills);
    subscribeInfo.SetPriority(1);
    subscriber_ = std::make_shared<FifthAbilityEventSubscriber>(subscribeInfo);
    subscriber_->fifthAbility = this;
    CommonEventManager::SubscribeCommonEvent(subscriber_);
}

void FifthAbilityEventSubscriber::OnReceiveEvent(const CommonEventData &data)
{
    APP_LOGI("FifthAbilityEventSubscriber::OnReceiveEvent:event=%{public}s", data.GetWant().GetAction().c_str());
    APP_LOGI("FifthAbilityEventSubscriber::OnReceiveEvent:data=%{public}s", data.GetData().c_str());
    APP_LOGI("FifthAbilityEventSubscriber::OnReceiveEvent:code=%{public}d", data.GetCode());
    auto eventName = data.GetWant().GetAction();
}

void FifthAbility::TestAbility(int apiIndex, int caseIndex, int code)
{
    APP_LOGI("FifthAbility::TestAbility");
    if (mapCase_.find(apiIndex) != mapCase_.end()) {
        if (caseIndex < (int)mapCase_[apiIndex].size()) {
            mapCase_[apiIndex][caseIndex](code);
        }
    }
}

REGISTER_AA(FifthAbility)
}  // namespace AppExecFwk
}  // namespace OHOS
