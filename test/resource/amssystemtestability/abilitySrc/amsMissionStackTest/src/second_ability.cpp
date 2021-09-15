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
    TestUtils::PublishEvent(g_EVENT_RESP_SECOND_LIFECYCLE, SECOND_ABILITY_CODE, "OnStart");
}

void SecondAbility::OnStop()
{
    APP_LOGI("SecondAbility::OnStop");
    Ability::OnStop();
    CommonEventManager::UnSubscribeCommonEvent(subscriber_);
    TestUtils::PublishEvent(g_EVENT_RESP_SECOND_LIFECYCLE, SECOND_ABILITY_CODE, "OnStop");
}

void SecondAbility::OnActive()
{
    APP_LOGI("SecondAbility::OnActive");
    Ability::OnActive();
    TestUtils::PublishEvent(g_EVENT_RESP_SECOND_LIFECYCLE, SECOND_ABILITY_CODE, "OnActive");
}

void SecondAbility::OnInactive()
{
    APP_LOGI("SecondAbility::OnInactive");
    Ability::OnInactive();
    TestUtils::PublishEvent(g_EVENT_RESP_SECOND_LIFECYCLE, SECOND_ABILITY_CODE, "OnInactive");
}

void SecondAbility::OnBackground()
{
    APP_LOGI("SecondAbility::OnBackground");
    Ability::OnBackground();
    TestUtils::PublishEvent(g_EVENT_RESP_SECOND_LIFECYCLE, SECOND_ABILITY_CODE, "OnBackground");
}

void SecondAbility::OnForeground(const Want &want)
{
    APP_LOGI("SecondAbility::OnForeground");
    Ability::OnForeground(want);
    TestUtils::PublishEvent(g_EVENT_RESP_SECOND_LIFECYCLE, SECOND_ABILITY_CODE, "OnForeground");
}

void SecondAbility::OnRestoreAbilityState(const PacMap &inState)
{
    APP_LOGI("SecondAbility::OnRestoreAbilityState");
    Ability::OnRestoreAbilityState(inState);
    TestUtils::PublishEvent(g_EVENT_RESP_SECOND_LIFECYCLE, SECOND_ABILITY_CODE, "OnRestoreAbilityState");
}
void SecondAbility::OnSaveAbilityState(PacMap &outState)
{
    APP_LOGI("SecondAbility::OnSaveAbilityState");
    Ability::OnSaveAbilityState(outState);
    TestUtils::PublishEvent(g_EVENT_RESP_SECOND_LIFECYCLE, SECOND_ABILITY_CODE, "OnSaveAbilityState");
}

void SecondAbility::SubscribeEvent()
{
    std::vector<std::string> eventList = {
        g_EVENT_REQU_SECOND,
    };
    MatchingSkills matchingSkills;
    for (const auto &e : eventList) {
        matchingSkills.AddEvent(e);
    }
    CommonEventSubscribeInfo subscribeInfo(matchingSkills);
    subscribeInfo.SetPriority(1);
    subscriber_ = std::make_shared<SecondEventSubscriber>(subscribeInfo);
    subscriber_->secondAbility = this;
    CommonEventManager::SubscribeCommonEvent(subscriber_);
}

void SecondEventSubscriber::OnReceiveEvent(const CommonEventData &data)
{
    APP_LOGI("SecondEventSubscriber::OnReceiveEvent:event=%{public}s", data.GetWant().GetAction().c_str());
    APP_LOGI("SecondEventSubscriber::OnReceiveEvent:data=%{public}s", data.GetData().c_str());
    APP_LOGI("SecondEventSubscriber::OnReceiveEvent:code=%{public}d", data.GetCode());
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
    bool result = true;
    LockMission();
    TestUtils::PublishEvent(g_EVENT_RESP_SECOND, code, std::to_string(result));
    TerminateAbility();
}
void SecondAbility::MissionStackCase2(int code)
{
    bool result = true;
    TestUtils::PublishEvent(g_EVENT_RESP_SECOND, code, std::to_string(result));
}
void SecondAbility::MissionStackCase3(int code)
{
    bool result = true;
    TestUtils::PublishEvent(g_EVENT_RESP_SECOND, code, std::to_string(result));
}
void SecondAbility::MissionStackCase4(int code)
{
    bool result = true;
    TestUtils::PublishEvent(g_EVENT_RESP_SECOND, code, std::to_string(result));
}
void SecondAbility::MissionStackCase5(int code)
{
    bool result = true;
    TestUtils::PublishEvent(g_EVENT_RESP_SECOND, code, std::to_string(result));
}
void SecondAbility::MissionStackCase6(int code)
{
    bool result = true;
    TestUtils::PublishEvent(g_EVENT_RESP_SECOND, code, std::to_string(result));
}
void SecondAbility::MissionStackCase7(int code)
{
    bool result = true;
    TestUtils::PublishEvent(g_EVENT_RESP_SECOND, code, std::to_string(result));
}
void SecondAbility::MissionStackCase8(int code)
{
    bool result = true;
    TestUtils::PublishEvent(g_EVENT_RESP_SECOND, code, std::to_string(result));
}
void SecondAbility::MissionStackCase9(int code)
{
    bool result = true;
    LockMission();
    UnlockMission();
    TestUtils::PublishEvent(g_EVENT_RESP_SECOND, code, std::to_string(result));
    TerminateAbility();
}
void SecondAbility::MissionStackCase10(int code)
{
    bool result = true;
    TestUtils::PublishEvent(g_EVENT_RESP_SECOND, code, std::to_string(result));
}
void SecondAbility::MissionStackCase11(int code)
{
    bool result = true;
    result = MoveMissionToEnd(false);
    TestUtils::PublishEvent(g_EVENT_RESP_SECOND, code, std::to_string(result));
}
void SecondAbility::MissionStackCase12(int code)
{
    bool result = true;
    TestUtils::PublishEvent(g_EVENT_RESP_SECOND, code, std::to_string(result));
}
void SecondAbility::MissionStackCase13(int code)
{
    bool result = true;
    TestUtils::PublishEvent(g_EVENT_RESP_SECOND, code, std::to_string(result));
}
void SecondAbility::MissionStackCase14(int code)
{
    APP_LOGI("SecondAbility::MissionStackCase14");
    bool result = true;
    result = MoveMissionToEnd(true);
    if (result) {
        APP_LOGI("SecondAbility::MissionStackCase14 MoveMissionToEnd(true)true ====> %{public}d", result);
    } else {
        APP_LOGI("SecondAbility::MissionStackCase14 MoveMissionToEnd(true)false ====> %{public}d", result);
    }
    TestUtils::PublishEvent(g_EVENT_RESP_SECOND, code, std::to_string(result));
}
void SecondAbility::MissionStackCase15(int code)
{
    APP_LOGI("SecondAbility::MissionStackCase15");
    bool result = true;
    result = MoveMissionToEnd(false);
    if (result) {
        APP_LOGI("SecondAbility::MissionStackCase15 MoveMissionToEnd(false)true ====> %{public}d", result);
    } else {
        APP_LOGI("SecondAbility::MissionStackCase15 MoveMissionToEnd(false)false ====> %{public}d", result);
    }
    TestUtils::PublishEvent(g_EVENT_RESP_SECOND, code, std::to_string(result));
}
void SecondAbility::MissionStackCase16(int code)
{
    APP_LOGI("SecondAbility::MissionStackCase16");
    bool result = true;
    TestUtils::PublishEvent(g_EVENT_RESP_SECOND, code, std::to_string(result));
}
void SecondAbility::MissionStackCase17(int code)
{
    APP_LOGI("SecondAbility::MissionStackCase17");
    bool result = true;
    TestUtils::PublishEvent(g_EVENT_RESP_SECOND, code, std::to_string(result));
}
void SecondAbility::MissionStackCase18(int code)
{
    APP_LOGI("SecondAbility::MissionStackCase18");
    bool result = true;
    TestUtils::PublishEvent(g_EVENT_RESP_SECOND, code, std::to_string(result));
}

void SecondAbility::SaveAbilityStateCase1(int code)
{
    bool result = true;
    TestUtils::PublishEvent(g_EVENT_RESP_SECOND, code, std::to_string(result));
}
void SecondAbility::SaveAbilityStateCase2(int code)
{
    bool result = true;
    TestUtils::PublishEvent(g_EVENT_RESP_SECOND, code, std::to_string(result));
}
void SecondAbility::SaveAbilityStateCase3(int code)
{
    bool result = true;
    TestUtils::PublishEvent(g_EVENT_RESP_SECOND, code, std::to_string(result));
}

void SecondAbility::RestoreAbilityStateCase1(int code)
{
    bool result = true;
    TestUtils::PublishEvent(g_EVENT_RESP_SECOND, code, std::to_string(result));
}
void SecondAbility::RestoreAbilityStateCase2(int code)
{
    bool result = true;
    TestUtils::PublishEvent(g_EVENT_RESP_SECOND, code, std::to_string(result));
}
void SecondAbility::RestoreAbilityStateCase3(int code)
{
    bool result = true;
    TestUtils::PublishEvent(g_EVENT_RESP_SECOND, code, std::to_string(result));
    TerminateAbility();
}

REGISTER_AA(SecondAbility)
}  // namespace AppExecFwk
}  // namespace OHOS
