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

#include "third_ability.h"
#include "app_log_wrapper.h"
#include "test_utils.h"

namespace OHOS {
namespace AppExecFwk {
using namespace OHOS::EventFwk;
constexpr int index_f = 0;
constexpr int index_s = 1;
constexpr int index_t = 2;
constexpr int paramCnt = 3;

void ThirdAbility::Init(const std::shared_ptr<AbilityInfo> &abilityInfo,
    const std::shared_ptr<OHOSApplication> &application, std::shared_ptr<AbilityHandler> &handler,
    const sptr<IRemoteObject> &token)
{
    APP_LOGI("ThirdAbility::Init");
    Ability::Init(abilityInfo, application, handler, token);
}

ThirdAbility::~ThirdAbility()
{
    CommonEventManager::UnSubscribeCommonEvent(subscriber_);
}

void ThirdAbility::OnStart(const Want &want)
{
    APP_LOGI("ThirdAbility::OnStart");
    SubscribeEvent();
    Ability::OnStart(want);
    TestUtils::PublishEvent(g_EVENT_RESP_THIRD_LIFECYCLE, THIRD_ABILITY_CODE, "OnStart");
}

void ThirdAbility::OnStop()
{
    APP_LOGI("ThirdAbility::OnStop");
    Ability::OnStop();
    CommonEventManager::UnSubscribeCommonEvent(subscriber_);
    TestUtils::PublishEvent(g_EVENT_RESP_THIRD_LIFECYCLE, THIRD_ABILITY_CODE, "OnStop");
}

void ThirdAbility::OnActive()
{
    APP_LOGI("ThirdAbility::OnActive");
    Ability::OnActive();
    TestUtils::PublishEvent(g_EVENT_RESP_THIRD_LIFECYCLE, THIRD_ABILITY_CODE, "OnActive");
}

void ThirdAbility::OnInactive()
{
    APP_LOGI("ThirdAbility::OnInactive");
    Ability::OnInactive();
    TestUtils::PublishEvent(g_EVENT_RESP_THIRD_LIFECYCLE, THIRD_ABILITY_CODE, "OnInactive");
}

void ThirdAbility::OnBackground()
{
    APP_LOGI("ThirdAbility::OnBackground");
    Ability::OnBackground();
    TestUtils::PublishEvent(g_EVENT_RESP_THIRD_LIFECYCLE, THIRD_ABILITY_CODE, "OnBackground");
}

void ThirdAbility::OnForeground(const Want &want)
{
    APP_LOGI("ThirdAbility::OnForeground");
    Ability::OnForeground(want);
    TestUtils::PublishEvent(g_EVENT_RESP_THIRD_LIFECYCLE, THIRD_ABILITY_CODE, "OnForeground");
}

void ThirdAbility::OnRestoreAbilityState(const PacMap &inState)
{
    APP_LOGI("ThirdAbility::OnRestoreAbilityState");
    Ability::OnRestoreAbilityState(inState);
    TestUtils::PublishEvent(g_EVENT_RESP_THIRD_LIFECYCLE, THIRD_ABILITY_CODE, "OnRestoreAbilityState");
}
void ThirdAbility::OnSaveAbilityState(PacMap &outState)
{
    APP_LOGI("ThirdAbility::OnSaveAbilityState");
    Ability::OnSaveAbilityState(outState);
    TestUtils::PublishEvent(g_EVENT_RESP_THIRD_LIFECYCLE, THIRD_ABILITY_CODE, "OnSaveAbilityState");
}

void ThirdAbility::SubscribeEvent()
{
    std::vector<std::string> eventList = {
        g_EVENT_REQU_THIRD,
    };
    MatchingSkills matchingSkills;
    for (const auto &e : eventList) {
        matchingSkills.AddEvent(e);
    }
    CommonEventSubscribeInfo subscribeInfo(matchingSkills);
    subscribeInfo.SetPriority(1);
    subscriber_ = std::make_shared<ThirdEventSubscriber>(subscribeInfo);
    subscriber_->thirdAbility = this;
    CommonEventManager::SubscribeCommonEvent(subscriber_);
}

void ThirdEventSubscriber::OnReceiveEvent(const CommonEventData &data)
{
    APP_LOGI("ThirdEventSubscriber::OnReceiveEvent:event=%{public}s", data.GetWant().GetAction().c_str());
    APP_LOGI("ThirdEventSubscriber::OnReceiveEvent:data=%{public}s", data.GetData().c_str());
    APP_LOGI("ThirdEventSubscriber::OnReceiveEvent:code=%{public}d", data.GetCode());
    auto eventName = data.GetWant().GetAction();
    if (std::strcmp(eventName.c_str(), g_EVENT_REQU_THIRD.c_str()) == 0) {
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

void ThirdAbility::TestAbility(int apiIndex, int caseIndex, int code)
{
    APP_LOGI("ThirdAbility::TestAbility");
    if (mapCase_.find(apiIndex) != mapCase_.end()) {
        if (caseIndex < (int)mapCase_[apiIndex].size()) {
            mapCase_[apiIndex][caseIndex](code);
        }
    }
}

void ThirdAbility::MissionStackCase1(int code)
{
    bool result = true;
    TestUtils::PublishEvent(g_EVENT_RESP_THIRD, code, std::to_string(result));
}
void ThirdAbility::MissionStackCase2(int code)
{
    bool result = true;
    TestUtils::PublishEvent(g_EVENT_RESP_THIRD, code, std::to_string(result));
}
void ThirdAbility::MissionStackCase3(int code)
{
    bool result = true;
    TestUtils::PublishEvent(g_EVENT_RESP_THIRD, code, std::to_string(result));
}
void ThirdAbility::MissionStackCase4(int code)
{
    bool result = true;
    TestUtils::PublishEvent(g_EVENT_RESP_THIRD, code, std::to_string(result));
}
void ThirdAbility::MissionStackCase5(int code)
{
    bool result = true;
    LockMission();
    std::map<std::string, std::string> params;
    Want want = TestUtils::MakeWant("", "SecondAbility", "com.ohos.amsst.MissionStack", params);
    AbilityContext::StartAbility(want, 1);
    TestUtils::PublishEvent(g_EVENT_RESP_THIRD, code, std::to_string(result));
}
void ThirdAbility::MissionStackCase6(int code)
{
    bool result = true;
    TestUtils::PublishEvent(g_EVENT_RESP_THIRD, code, std::to_string(result));
}
void ThirdAbility::MissionStackCase7(int code)
{
    bool result = true;
    TestUtils::PublishEvent(g_EVENT_RESP_THIRD, code, std::to_string(result));
}
void ThirdAbility::MissionStackCase8(int code)
{
    bool result = true;
    TestUtils::PublishEvent(g_EVENT_RESP_THIRD, code, std::to_string(result));
}
void ThirdAbility::MissionStackCase9(int code)
{
    bool result = true;
    TestUtils::PublishEvent(g_EVENT_RESP_THIRD, code, std::to_string(result));
}
void ThirdAbility::MissionStackCase10(int code)
{
    bool result = true;
    TestUtils::PublishEvent(g_EVENT_RESP_THIRD, code, std::to_string(result));
}
void ThirdAbility::MissionStackCase11(int code)
{
    bool result = true;
    TestUtils::PublishEvent(g_EVENT_RESP_THIRD, code, std::to_string(result));
}
void ThirdAbility::MissionStackCase12(int code)
{
    bool result = true;
    TestUtils::PublishEvent(g_EVENT_RESP_THIRD, code, std::to_string(result));
}
void ThirdAbility::MissionStackCase13(int code)
{
    bool result = true;
    TestUtils::PublishEvent(g_EVENT_RESP_THIRD, code, std::to_string(result));
}
void ThirdAbility::MissionStackCase14(int code)
{
    bool result = true;
    TestUtils::PublishEvent(g_EVENT_RESP_THIRD, code, std::to_string(result));
}
void ThirdAbility::MissionStackCase15(int code)
{
    bool result = true;
    TestUtils::PublishEvent(g_EVENT_RESP_THIRD, code, std::to_string(result));
}
void ThirdAbility::MissionStackCase16(int code)
{
    bool result = true;
    TestUtils::PublishEvent(g_EVENT_RESP_THIRD, code, std::to_string(result));
}
void ThirdAbility::MissionStackCase17(int code)
{
    bool result = true;
    TestUtils::PublishEvent(g_EVENT_RESP_THIRD, code, std::to_string(result));
}
void ThirdAbility::MissionStackCase18(int code)
{
    bool result = true;
    TestUtils::PublishEvent(g_EVENT_RESP_THIRD, code, std::to_string(result));
}
REGISTER_AA(ThirdAbility)
}  // namespace AppExecFwk
}  // namespace OHOS
