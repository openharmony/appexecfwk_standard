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
using namespace OHOS::AAFwk;
constexpr int index_f = 0;
constexpr int index_s = 1;
constexpr int index_t = 2;
constexpr int paramCnt = 3;

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
    APP_LOGI("MainAbility::OnStart");
    SubscribeEvent();
    Ability::OnStart(want);
    callbackSeq += "OnStart";
    TestUtils::PublishEvent(g_EVENT_RESP_MAIN_LIFECYCLE_SUBSIDIARY, MAIN_ABILITY_CODE_SUBSIDIARY, "OnStart");
}

void MainAbility::OnStop()
{
    APP_LOGI("MainAbility::OnStop");
    Ability::OnStop();
    CommonEventManager::UnSubscribeCommonEvent(subscriber_);
    callbackSeq = "OnStop";
    TestUtils::PublishEvent(g_EVENT_RESP_MAIN_LIFECYCLE_SUBSIDIARY, MAIN_ABILITY_CODE_SUBSIDIARY, callbackSeq);
    callbackSeq = "";
}

void MainAbility::OnActive()
{
    APP_LOGI("MainAbility::OnActive====<");
    Ability::OnActive();
    callbackSeq = "OnActive";
    TestUtils::PublishEvent(g_EVENT_RESP_MAIN_LIFECYCLE_SUBSIDIARY, MAIN_ABILITY_CODE_SUBSIDIARY, callbackSeq);
    callbackSeq = "";
}

void MainAbility::OnConfigurationUpdated(const Configuration &configuration)
{
    APP_LOGI("MainAbility::OnConfigurationUpdated====<");
    Ability::OnConfigurationUpdated(configuration);
    callbackUpdated += "Updated";  // UpdatedUpdated
    TestUtils::PublishEvent(g_EVENT_RESP_MAIN_LIFECYCLE_SUBSIDIARY, MAIN_ABILITY_CODE_SUBSIDIARY, callbackUpdated);
}

void MainAbility::OnInactive()
{
    APP_LOGI("MainAbility::OnInactive");
    Ability::OnInactive();
    callbackSeq += "OnInactive";
    TestUtils::PublishEvent(g_EVENT_RESP_MAIN_LIFECYCLE_SUBSIDIARY, MAIN_ABILITY_CODE_SUBSIDIARY, "OnInactive");
}

void MainAbility::OnBackground()
{
    APP_LOGI("MainAbility::OnBackground");
    Ability::OnBackground();
    callbackSeq += "OnBackground";
    TestUtils::PublishEvent(g_EVENT_RESP_MAIN_LIFECYCLE_SUBSIDIARY, MAIN_ABILITY_CODE_SUBSIDIARY, "OnBackground");
}

void MainAbility::OnForeground(const Want &want)
{
    APP_LOGI("MainAbility::OnForeground");
    Ability::OnForeground(want);
    callbackSeq += "OnForeground";
    TestUtils::PublishEvent(g_EVENT_RESP_MAIN_LIFECYCLE_SUBSIDIARY, MAIN_ABILITY_CODE_SUBSIDIARY, "OnForeground");
}

void MainAbility::SubscribeEvent()
{
    std::vector<std::string> eventList = {
        g_EVENT_REQU_MAIN_SUBSIDIARY,
    };
    MatchingSkills matchingSkills;
    for (const auto &e : eventList) {
        matchingSkills.AddEvent(e);
    }
    CommonEventSubscribeInfo subscribeInfo(matchingSkills);
    subscribeInfo.SetPriority(1);
    subscriber_ = std::make_shared<MainAbilityEventSubscriber>(subscribeInfo);
    subscriber_->mainAbility = this;
    CommonEventManager::SubscribeCommonEvent(subscriber_);
}

void MainAbilityEventSubscriber::OnReceiveEvent(const CommonEventData &data)
{
    APP_LOGI("MainAbilityEventSubscriber::OnReceiveEvent:event=%{public}s", data.GetWant().GetAction().c_str());
    APP_LOGI("MainAbilityEventSubscriber::OnReceiveEvent:data=%{public}s", data.GetData().c_str());
    APP_LOGI("MainAbilityEventSubscriber::OnReceiveEvent:code=%{public}d", data.GetCode());
    auto eventName = data.GetWant().GetAction();
    if (std::strcmp(eventName.c_str(), g_EVENT_REQU_MAIN_SUBSIDIARY.c_str()) == 0) {
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

void MainAbility::TestAbility(int apiIndex, int caseIndex, int code)
{
    APP_LOGI("MainAbility::TestAbility");
    if (mapCase_.find(apiIndex) != mapCase_.end()) {
        if (caseIndex < (int)mapCase_[apiIndex].size()) {
            mapCase_[apiIndex][caseIndex](code);
        }
    }
}

void MainAbility::MissionStackCase1(int code)
{
    APP_LOGI("MainAbility::MissionStackCase1");
    bool result = true;
    std::map<std::string, std::string> params;
    Want want = TestUtils::MakeWant("", "SecondAbility", "com.ohos.amsst.MissionStackSubsidiary", params);
    AbilityContext::StartAbility(want, 1);

    TestUtils::PublishEvent(g_EVENT_RESP_MAIN_SUBSIDIARY, code, std::to_string(result));
}

void MainAbility::MissionStackCase2(int code)
{
    APP_LOGI("MainAbility::MissionStackCase2");
    bool result = true;
    result = MoveMissionToEnd(true);
    if (result) {
        APP_LOGI("MainAbility::MissionStackCase2 MoveMissionToEnd(true)true ====> %{public}d", result);
    } else {
        APP_LOGI("MainAbility::MissionStackCase2 MoveMissionToEnd(true)false ====> %{public}d", result);
    }
    TestUtils::PublishEvent(g_EVENT_RESP_MAIN_SUBSIDIARY, code, std::to_string(result));
}
void MainAbility::MissionStackCase3(int code)
{
    APP_LOGI("MainAbility::MissionStackCase3");
    bool result = true;
    result = MoveMissionToEnd(false);
    if (result) {
        APP_LOGI("MainAbility::MissionStackCase3 MoveMissionToEnd(false)true ====> %{public}d", result);
    } else {
        APP_LOGI("MainAbility::MissionStackCase3 MoveMissionToEnd(false)false ====> %{public}d", result);
    }
    TestUtils::PublishEvent(g_EVENT_RESP_MAIN_SUBSIDIARY, code, std::to_string(result));
}

REGISTER_AA(MainAbility)
}  // namespace AppExecFwk
}  // namespace OHOS
