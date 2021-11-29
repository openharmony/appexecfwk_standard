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

#include "ability_state_second.h"
#include <string>
#include "test_utils.h"

namespace OHOS {
namespace AppExecFwk {
using namespace OHOS::EventFwk;
std::string FwkAbilityState_Event_Requ_B = "requ_com_ohos_amsst_FwkAbilityStateB";
std::string FwkAbilityState_Event_Resp_B = "resp_com_ohos_amsst_FwkAbilityStateB";
int OnActiveCode = 1;

FwkAbilityStateSecond::~FwkAbilityStateSecond()
{
    CommonEventManager::UnSubscribeCommonEvent(subscriber_);
}

void FwkAbilityStateSecond::OnStart(const Want &want)
{
    SubscribeEvent();
    APP_LOGI("FwkAbilityStateSecond::onStart");
    Ability::OnStart(want);
    callback_seq += "OnStart";
}

void FwkAbilityStateSecond::OnForeground(const Want &want)
{
    APP_LOGI("FwkAbilityStateSecond::OnForeground");
    Ability::OnForeground(want);
}

void FwkAbilityStateSecond::OnNewWant(const Want &want)
{
    APP_LOGI("FwkAbilityStateSecond::OnNewWant");
    Ability::OnNewWant(want);
}

void FwkAbilityStateSecond::OnStop()
{
    APP_LOGI("FwkAbilityStateSecond::onStop");
    Ability::OnStop();
}

void FwkAbilityStateSecond::OnActive()
{
    APP_LOGI("FwkAbilityStateSecond::OnActive");
    Ability::OnActive();
    callback_seq += "OnActive";
    TestUtils::PublishEvent(FwkAbilityState_Event_Resp_B, OnActiveCode, callback_seq);
    callback_seq = "";
}

void FwkAbilityStateSecond::OnInactive()
{
    APP_LOGI("FwkAbilityStateSecond::OnInactive");
    Ability::OnInactive();
}

void FwkAbilityStateSecond::OnBackground()
{
    APP_LOGI("FwkAbilityStateSecond::OnBackground");
    Ability::OnBackground();
}

void FwkAbilityStateSecond::OnSaveAbilityState(PacMap &outState)
{
    APP_LOGI("FwkAbilityStateSecond::OnSaveAbilityState");
    Ability::OnSaveAbilityState(outState);
}

void FwkAbilityStateSecond::OnRestoreAbilityState(const PacMap &inState)
{
    APP_LOGI("FwkAbilityStateSecond::OnRestoreAbilityState");
    Ability::OnRestoreAbilityState(inState);
}


void FwkAbilityStateSecond::SubscribeEvent()
{
    std::vector<std::string> eventList = {
        FwkAbilityState_Event_Requ_B,
    };
    MatchingSkills matchingSkills;
    for (const auto &e : eventList) {
        matchingSkills.AddEvent(e);
    }
    CommonEventSubscribeInfo subscribeInfo(matchingSkills);
    subscribeInfo.SetPriority(1);
    subscriber_ = std::make_shared<FwkAbilityStateSecondSubscriber>(subscribeInfo);
    subscriber_->mainAbility = this;
    CommonEventManager::SubscribeCommonEvent(subscriber_);
}

void FwkAbilityStateSecondSubscriber::OnReceiveEvent(const CommonEventData &data)
{
    auto eventName = data.GetWant().GetAction();
    if (strcmp(eventName.c_str(), FwkAbilityState_Event_Requ_B.c_str()) == 0) {
        auto target = data.GetData();
        if (mapAction_.find(target) != mapAction_.end()) {
            mapAction_[target](target, data.GetCode());
        } else {
            APP_LOGI("OnReceiveEvent: CommonEventData error(%{public}s)", target.c_str());
        }
    }
}

void FwkAbilityStateSecond::Action(std::string action, int code)
{
    if (mapAction_.find(action) != mapAction_.end()) {
        mapAction_[action](code);
    }
}

void FwkAbilityStateSecond::TerminateThis(int code)
{
    std::string targetBundle = "com.ohos.amsst.fwkAbilityState";
    std::string targetAbility = "FwkAbilityStateSecond";
    Want want;
    want.SetElementName(targetBundle, targetAbility);
    StartAbility(want);
}

REGISTER_AA(FwkAbilityStateSecond);
}  // namespace AppExecFwk
}  // namespace OHOS