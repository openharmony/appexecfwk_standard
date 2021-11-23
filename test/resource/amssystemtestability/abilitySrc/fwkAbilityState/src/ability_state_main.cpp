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

#include "ability_state_main.h"
#include <string>
#include "test_utils.h"

namespace OHOS {
namespace AppExecFwk {
using namespace OHOS::EventFwk;
namespace {
std::string FwkAbilityState_Event_Resp_A = "resp_com_ohos_amsst_FwkAbilityStateA";
std::string FwkAbilityState_Event_Requ_A = "requ_com_ohos_amsst_FwkAbilityStateA";
int OnStopCode = 3;
int OnActiveCode = 1;
int OnBackgroundCode = 2;
}

FwkAbilityStateMain::~FwkAbilityStateMain()
{
    CommonEventManager::UnSubscribeCommonEvent(subscriber_);
}

void FwkAbilityStateMain::OnStart(const Want &want)
{
    APP_LOGI("FwkAbilityStateMain::onStart");
    SubscribeEvent();
    Ability::OnStart(want);
    callback_seq += "OnStart";
}

void FwkAbilityStateMain::OnNewWant(const Want &want)
{
    APP_LOGI("FwkAbilityStateMain::OnNewWant");
    Ability::OnNewWant(want);
    callback_seq += "OnNewWant";
}

void FwkAbilityStateMain::OnForeground(const Want &want)
{
    APP_LOGI("FwkAbilityStateMain::OnForeground");
    Ability::OnForeground(want);
    callback_seq += "OnForeground";
}

void FwkAbilityStateMain::OnStop()
{
    APP_LOGI("FwkAbilityStateMain::onStop");
    Ability::OnStop();
    CommonEventManager::UnSubscribeCommonEvent(subscriber_);
    callback_seq += "OnStop";
    TestUtils::PublishEvent(FwkAbilityState_Event_Resp_A, OnStopCode, callback_seq);
    callback_seq = "";
}

void FwkAbilityStateMain::OnActive()
{
    APP_LOGI("FwkAbilityStateMain::OnActive");
    Ability::OnActive();
    callback_seq += "OnActive";
    TestUtils::PublishEvent(FwkAbilityState_Event_Resp_A, OnActiveCode, callback_seq);
    callback_seq = "";
}

void FwkAbilityStateMain::OnInactive()
{
    APP_LOGI("FwkAbilityStateMain::OnInactive");
    Ability::OnInactive();
    callback_seq += "OnInactive";
}

void FwkAbilityStateMain::OnBackground()
{
    APP_LOGI("FwkAbilityStateMain::OnBackground");
    Ability::OnBackground();
    callback_seq += "OnBackground";
    TestUtils::PublishEvent(FwkAbilityState_Event_Resp_A, OnBackgroundCode, callback_seq);
    callback_seq = "";
}

void FwkAbilityStateMain::OnSaveAbilityState(PacMap &outState)
{
    APP_LOGI("FwkAbilityStateMain::OnSaveAbilityState");
    Ability::OnSaveAbilityState(outState);
    callback_seq += "OnSaveAbilityState";
}

void FwkAbilityStateMain::OnRestoreAbilityState(const PacMap &inState)
{
    APP_LOGI("FwkAbilityStateMain::OnRestoreAbilityState");
    Ability::OnRestoreAbilityState(inState);
    callback_seq += "OnRestoreAbilityState";
}

void FwkAbilityStateMain::SubscribeEvent()
{
    std::vector<std::string> eventList = {
        FwkAbilityState_Event_Requ_A,
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
    if (strcmp(eventName.c_str(), FwkAbilityState_Event_Requ_A.c_str()) == 0) {
        auto target = data.GetData();
        if (mapAction_.find(target) != mapAction_.end()) {
            mapAction_[target](target, data.GetCode());
        } else {
            APP_LOGI("OnReceiveEvent: CommonEventData error(%{public}s)", target.c_str());
        }
    }
}

void FwkAbilityStateMain::Action(std::string action, int code)
{
    APP_LOGI("FwkAbilityStateMain::Action Called");
    if (mapAction_.find(action) != mapAction_.end()) {
        mapAction_[action](code);
    }
}

void FwkAbilityStateMain::DoCrash(std::string action, int code)
{
    APP_LOGI("FwkAbilityStateMain::DoCrash Called");
    CrashMaker *pcCrashMaker = nullptr;
    int a = pcCrashMaker->CrashTry();
    APP_LOGI("FwkAbilityStateMain::DoCrash Process %{public}d", a);
}

void FwkAbilityStateMain::StartNextAbility(int code)
{
    std::string targetBundle = "com.ohos.amsst.fwkAbilityState";
    std::string targetAbility = "FwkAbilityStateSecond";
    Want want;
    want.SetElementName(targetBundle, targetAbility);
    StartAbility(want);
}

REGISTER_AA(FwkAbilityStateMain);
}  // namespace AppExecFwk
}  // namespace OHOS