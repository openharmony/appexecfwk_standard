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

#ifndef AMS_ABILITY_APPEND_TEST_A_MAIN_ABILITY_H
#define AMS_ABILITY_APPEND_TEST_A_MAIN_ABILITY_H
#include "ability.h"
#include "ability_loader.h"
#include "common_event.h"
#include "common_event_manager.h"
#include "ability_append_test_info.h"

namespace OHOS {
namespace AppExecFwk {
using namespace OHOS::EventFwk;
class FirstEventSubscriber;
class MainAbility : public Ability {
public:
    void SubscribeEvent();
    void TestAbility(int apiIndex, int caseIndex, int code);

    void OnSetCallerCase1(int code);
    void TerminateAndRemoveMissonCase1(int code);
    void TerminateAbilityResultCase1(int code);
    void GetDispalyOrientationCase1(int code);
    void GetPreferencesDirCase1(int code);
    void StartAbilitiesCase1(int code);
    void GetColorModeCase1(int code);
    void SetColorModeCase1(int code);
    void IsFirstInMissionCase1(int code);

    MainAbility()
    {
        mapCase_ = {
            {(int)AppendApi::OnSetCaller,
                {
                    [this](int code) { OnSetCallerCase1(code); },
                }},
                {(int)AppendApi::TerminateAndRemoveMisson,
                {
                    [this](int code) { TerminateAndRemoveMissonCase1(code); },
                }},
                {(int)AppendApi::TerminateAbilityResult,
                {
                    [this](int code) { TerminateAbilityResultCase1(code); },
                }},
                {(int)AppendApi::GetDispalyOrientation,
                {
                    [this](int code) { GetDispalyOrientationCase1(code); },
                }},
                {(int)AppendApi::GetPreferencesDir,
                {
                    [this](int code) { GetPreferencesDirCase1(code); },
                }},
                {(int)AppendApi::StartAbilities,
                {
                    [this](int code) { StartAbilitiesCase1(code); },
                }},
                {(int)AppendApi::GetColorMode,
                {
                    [this](int code) { GetColorModeCase1(code); },
                }},
                {(int)AppendApi::SetColorMode,
                {
                    [this](int code) { SetColorModeCase1(code); },
                }},
                {(int)AppendApi::IsFirstInMission,
                {
                    [this](int code) { IsFirstInMissionCase1(code); },
                }},
        };
    }

    std::unordered_map<int, std::vector<std::function<void(int)>>> mapCase_;
    ~MainAbility();

protected:
    void Init(const std::shared_ptr<AbilityInfo> &abilityInfo, const std::shared_ptr<OHOSApplication> &application,
        std::shared_ptr<AbilityHandler> &handler, const sptr<IRemoteObject> &token) override;
    virtual void OnStart(const Want &want) override;
    virtual void OnStop() override;
    virtual void OnActive() override;
    virtual void OnInactive() override;
    virtual void OnBackground() override;
    virtual void OnForeground(const Want &want) override;
    virtual Uri OnSetCaller() override;
    std::shared_ptr<FirstEventSubscriber> subscriber_;
};
class FirstEventSubscriber : public EventFwk::CommonEventSubscriber {
public:
    explicit FirstEventSubscriber(const EventFwk::CommonEventSubscribeInfo &sp) : CommonEventSubscriber(sp)
    {
        mapTestFunc_ = {
            {"Ability", [this](int apiIndex, int caseIndex, int code) { TestAbility(apiIndex, caseIndex, code); }},
        };
        mainAbility = nullptr;
    }

    void TestAbility(int apiIndex, int caseIndex, int code)
    {
        mainAbility->TestAbility(apiIndex, caseIndex, code);
    }

    virtual void OnReceiveEvent(const EventFwk::CommonEventData &data);

    MainAbility *mainAbility;
    std::unordered_map<std::string, std::function<void(int, int, int)>> mapTestFunc_;
    ~FirstEventSubscriber() = default;
};
}  // namespace AppExecFwk
}  // namespace OHOS
#endif  // AMS_ABILITY_APPEND_TEST_A_MAIN_ABILITY_H