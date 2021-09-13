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
constexpr int index_f = 0;
constexpr int index_s = 1;
constexpr int index_t = 2;
constexpr int paramCnt = 3;

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
    TestUtils::PublishEvent(g_EVENT_RESP_SECOND_LIFECYCLE_SUBSIDIARY, SECOND_ABILITY_CODE_SUBSIDIARY, "OnStart");
}

void SecondAbility::OnStop()
{
    APP_LOGI("SecondAbility::OnStop");
    Ability::OnStop();
    CommonEventManager::UnSubscribeCommonEvent(subscriber_);
    callbackSeq = "OnStop";
    TestUtils::PublishEvent(g_EVENT_RESP_SECOND_LIFECYCLE_SUBSIDIARY, SECOND_ABILITY_CODE_SUBSIDIARY, callbackSeq);
    callbackSeq = "";
}

void SecondAbility::OnActive()
{
    APP_LOGI("SecondAbility::OnActive====<");
    Ability::OnActive();
    callbackSeq = "OnActive";
    TestUtils::PublishEvent(g_EVENT_RESP_SECOND_LIFECYCLE_SUBSIDIARY, SECOND_ABILITY_CODE_SUBSIDIARY, callbackSeq);
    callbackSeq = "";
}

void SecondAbility::OnConfigurationUpdated(const Configuration &configuration)
{
    APP_LOGI("SecondAbility::OnConfigurationUpdated====<");
    Ability::OnConfigurationUpdated(configuration);
    callbackUpdated += "Updated";  // UpdatedUpdated
    TestUtils::PublishEvent(g_EVENT_RESP_SECOND_LIFECYCLE_SUBSIDIARY, SECOND_ABILITY_CODE_SUBSIDIARY, callbackUpdated);
}

void SecondAbility::OnInactive()
{
    APP_LOGI("SecondAbility::OnInactive");
    Ability::OnInactive();
    callbackSeq += "OnInactive";
    TestUtils::PublishEvent(g_EVENT_RESP_SECOND_LIFECYCLE_SUBSIDIARY, SECOND_ABILITY_CODE_SUBSIDIARY, "OnInactive");
}

void SecondAbility::OnBackground()
{
    APP_LOGI("SecondAbility::OnBackground");
    Ability::OnBackground();
    callbackSeq += "OnBackground";
    TestUtils::PublishEvent(g_EVENT_RESP_SECOND_LIFECYCLE_SUBSIDIARY, SECOND_ABILITY_CODE_SUBSIDIARY, "OnBackground");
}

void SecondAbility::OnForeground(const Want &want)
{
    APP_LOGI("SecondAbility::OnForeground");
    Ability::OnForeground(want);
    callbackSeq += "OnForeground";
    TestUtils::PublishEvent(g_EVENT_RESP_SECOND_LIFECYCLE_SUBSIDIARY, SECOND_ABILITY_CODE_SUBSIDIARY, "OnForeground");
}

void SecondAbility::OnRestoreAbilityState(const PacMap &inState)
{
    APP_LOGI("SecondAbility::OnRestoreAbilityState");
    Ability::OnRestoreAbilityState(inState);
    TestUtils::PublishEvent(
        g_EVENT_RESP_SECOND_LIFECYCLE_SUBSIDIARY, SECOND_ABILITY_CODE_SUBSIDIARY, "OnRestoreAbilityState");
}
void SecondAbility::OnSaveAbilityState(PacMap &outState)
{
    APP_LOGI("SecondAbility::OnSaveAbilityState");
    Ability::OnSaveAbilityState(outState);
    TestUtils::PublishEvent(
        g_EVENT_RESP_SECOND_LIFECYCLE_SUBSIDIARY, SECOND_ABILITY_CODE_SUBSIDIARY, "OnSaveAbilityState");
}

void SecondAbility::SubscribeEvent()
{
    std::vector<std::string> eventList = {
        g_EVENT_REQU_SECOND_SUBSIDIARY,
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
    if (std::strcmp(eventName.c_str(), g_EVENT_REQU_SECOND.c_str()) == 0) {
        auto target = data.GetData();
        auto caseInfo = TestUtils::split(target, "_");
        if (caseInfo.size() < paramCnt) {
            return;
        }
        if (mapTestFunc_.find(caseInfo[index_f]) != mapTestFunc_.end()) {
            mapTestFunc_[caseInfo[index_f]](std::stoi(caseInfo[index_s]), std::stoi(caseInfo[index_t]), data.GetCode());
        } else {
            APP_LOGI("OnReceiveEvent: CommonEventData error(%{public}s)", target.c_str());
        }
    }
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

void SecondAbility::MissionStackCase1(int code)
{
    APP_LOGI("SecondAbility::MissionStackCase1====<");
    bool result = true;
    result = true;

    TestUtils::PublishEvent(g_EVENT_RESP_SECOND, code, std::to_string(result));
}

void SecondAbility::MissionStackCase2(int code)
{
    bool result = true;
    result = true;
    TestUtils::PublishEvent(g_EVENT_RESP_SECOND, code, std::to_string(result));
}

REGISTER_AA(SecondAbility)
}  // namespace AppExecFwk
}  // namespace OHOS
