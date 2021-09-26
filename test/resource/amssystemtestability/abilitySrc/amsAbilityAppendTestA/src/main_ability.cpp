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

#include "main_ability.h"
#include "app_log_wrapper.h"
#include "test_utils.h"

namespace OHOS {
namespace AppExecFwk {
using namespace OHOS::EventFwk;
namespace {
constexpr int numZero = 0;
constexpr int numOne = 0;
constexpr int numTwo = 0;
constexpr int numThree = 0;
}

void MainAbility::Init(const std::shared_ptr<AbilityInfo> &abilityInfo,
    const std::shared_ptr<OHOSApplication> &application, std::shared_ptr<AbilityHandler> &handler,
    const sptr<IRemoteObject> &token)
{
    APP_LOGI("MainAbility::Init");
    Ability::Init(abilityInfo, application, handler, token);
}

MainAbility::~MainAbility()
{
    CommonEventManager::UnSubscribeCommonEvent(subscriber_);
}

void MainAbility::OnStart(const Want &want)
{
    APP_LOGI("MainAbility::onStart");
    SubscribeEvent();
    Ability::OnStart(want);
    TestUtils::PublishEvent(g_EVENT_RESP_FIRST_LIFECYCLE, MAIN_ABILITY_A_CODE, "onStart");
}

void MainAbility::OnStop()
{
    APP_LOGI("MainAbility::OnStop");
    Ability::OnStop();
    CommonEventManager::UnSubscribeCommonEvent(subscriber_);
    TestUtils::PublishEvent(g_EVENT_RESP_FIRST_LIFECYCLE, MAIN_ABILITY_A_CODE, "OnStop");
}

void MainAbility::OnActive()
{
    APP_LOGI("MainAbility::OnActive");
    Ability::OnActive();
    TestUtils::PublishEvent(g_EVENT_RESP_FIRST_LIFECYCLE, MAIN_ABILITY_A_CODE, "OnActive");
}

void MainAbility::OnInactive()
{
    APP_LOGI("MainAbility::OnInactive");
    Ability::OnInactive();
    TestUtils::PublishEvent(g_EVENT_RESP_FIRST_LIFECYCLE, MAIN_ABILITY_A_CODE, "OnInactive");
}

void MainAbility::OnBackground()
{
    APP_LOGI("MainAbility::OnBackground");
    Ability::OnBackground();
    TestUtils::PublishEvent(g_EVENT_RESP_FIRST_LIFECYCLE, MAIN_ABILITY_A_CODE, "OnBackground");
}

void MainAbility::OnForeground(const Want &want)
{
    APP_LOGI("MainAbility::OnForeground");
    Ability::OnForeground(want);
    TestUtils::PublishEvent(g_EVENT_RESP_FIRST_LIFECYCLE, MAIN_ABILITY_A_CODE, "OnForeground");
}

Uri MainAbility::OnSetCaller()
{
    APP_LOGI("MainAbility::OnSetCaller");
    TestUtils::PublishEvent(g_EVENT_RESP_FIRST_LIFECYCLE, MAIN_ABILITY_A_CODE, "OnSetCaller");
    return Ability::OnSetCaller();
}

void MainAbility::SubscribeEvent()
{
    std::vector<std::string> eventList = {
        g_EVENT_REQU_FIRST,
    };
    MatchingSkills matchingSkills;
    for (const auto &e : eventList) {
        matchingSkills.AddEvent(e);
    }
    CommonEventSubscribeInfo subscribeInfo(matchingSkills);
    subscribeInfo.SetPriority(1);
    subscriber_ = std::make_shared<FirstEventSubscriber>(subscribeInfo);
    subscriber_->mainAbility = this;
    CommonEventManager::SubscribeCommonEvent(subscriber_);
}

void FirstEventSubscriber::OnReceiveEvent(const CommonEventData &data)
{
    APP_LOGI("FirstEventSubscriber::OnReceiveEvent:event=%{public}s", data.GetWant().GetAction().c_str());
    APP_LOGI("FirstEventSubscriber::OnReceiveEvent:data=%{public}s", data.GetData().c_str());
    APP_LOGI("FirstEventSubscriber::OnReceiveEvent:code=%{public}d", data.GetCode());
    auto eventName = data.GetWant().GetAction();
    if (std::strcmp(eventName.c_str(), g_EVENT_REQU_FIRST.c_str()) == 0) {
        auto target = data.GetData();
        auto caseInfo = TestUtils::split(target, "_");
        if (caseInfo.size() < numThree) {
            return;
        }
        if (mapTestFunc_.find(caseInfo[numZero]) != mapTestFunc_.end()) {
            mapTestFunc_[caseInfo[numZero]](std::stoi(caseInfo[numOne]), std::stoi(caseInfo[numTwo]), data.GetCode());
        } else {
            APP_LOGI("OnReceiveEvent: CommonEventData error(%{public}s)", target.c_str());
        }
    }
}

void MainAbility::TestAbility(int apiIndex, int caseIndex, int code)
{
    APP_LOGI("MainAbility::TestAbility");
    if (mapCase_.find(apiIndex) != mapCase_.end()) {
        if (caseIndex < (int)mapCase_[apiIndex].size()) {
            mapCase_[apiIndex][caseIndex](code);
        }
    }
}

void MainAbility::OnSetCallerCase1(int code)
{
    bool result = true;
    TestUtils::PublishEvent(g_EVENT_RESP_FIRST, code, std::to_string(result));
}

void MainAbility::TerminateAndRemoveMissonCase1(int code)
{
    bool result = true;
    TestUtils::PublishEvent(g_EVENT_RESP_FIRST, code, std::to_string(result));
    TerminateAndRemoveMission();
}

void MainAbility::TerminateAbilityResultCase1(int code)
{
    bool result = TerminateAbilityResult(1);
    TestUtils::PublishEvent(g_EVENT_RESP_FIRST, code, std::to_string(result));
}

void MainAbility::GetDispalyOrientationCase1(int code)
{
    bool result = true;
    int orientation = GetDisplayOrientation();
    result = orientation == static_cast<int>(DisplayOrientation::UNSPECIFIED);
    TestUtils::PublishEvent(g_EVENT_RESP_FIRST, code, std::to_string(result));
}

void MainAbility::GetPreferencesDirCase1(int code)
{
    bool result = true;
    string preferencesDir = GetPreferencesDir();
    result = !preferencesDir.empty();
    result = result && (bool)preferencesDir.find(this->GetBundleName());
    result = result && (bool)preferencesDir.find("com.ohos.amsst.AppAppendA/files/MainAbility/preferences");

    TestUtils::PublishEvent(g_EVENT_RESP_FIRST, code, std::to_string(result));
}

void MainAbility::StartAbilitiesCase1(int code)
{
    bool result = true;
    std::map<std::string, std::string> params;
    Want want1 = TestUtils::MakeWant("", "SecondAbility", "com.ohos.amsst.AppAppendA", params);
    Want want2 = TestUtils::MakeWant("", "MainAbility", "com.ohos.amsst.AppAppendB", params);
    std::vector<Want> wants = {want1, want2};
    AbilityContext::StartAbilities(wants);
    TestUtils::PublishEvent(g_EVENT_RESP_FIRST, code, std::to_string(result));
}

void MainAbility::GetColorModeCase1(int code)
{
    int colormode = GetColorMode();
    bool result = colormode == static_cast<int>(ModuleColorMode::AUTO);
    TestUtils::PublishEvent(g_EVENT_RESP_FIRST, code, std::to_string(result));
}

void MainAbility::SetColorModeCase1(int code)
{
    SetColorMode((int)ModuleColorMode::DARK);
    int colormode = GetColorMode();
    bool result = colormode == static_cast<int>(ModuleColorMode::DARK);
    TestUtils::PublishEvent(g_EVENT_RESP_FIRST, code, std::to_string(result));
}

void MainAbility::IsFirstInMissionCase1(int code)
{
    bool result = IsFirstInMission();
    TestUtils::PublishEvent(g_EVENT_RESP_FIRST, code, std::to_string(result));
}

REGISTER_AA(MainAbility)
}  // namespace AppExecFwk
}  // namespace OHOS
