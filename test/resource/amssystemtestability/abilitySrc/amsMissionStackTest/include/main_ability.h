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

    void MissionStackCase1(int code);
    void MissionStackCase2(int code);
    void MissionStackCase3(int code);
    void MissionStackCase4(int code);
    void MissionStackCase5(int code);
    void MissionStackCase6(int code);
    void MissionStackCase7(int code);
    void MissionStackCase8(int code);
    void MissionStackCase9(int code);
    void MissionStackCase10(int code);
    void MissionStackCase11(int code);
    void MissionStackCase12(int code);
    void MissionStackCase13(int code);
    void MissionStackCase14(int code);
    void MissionStackCase15(int code);
    void MissionStackCase16(int code);
    void MissionStackCase17(int code);
    void MissionStackCase18(int code);

    void SaveAbilityStateCase1(int code);
    void SaveAbilityStateCase2(int code);
    void SaveAbilityStateCase3(int code);

    void RestoreAbilityStateCase1(int code);
    void RestoreAbilityStateCase2(int code);
    void RestoreAbilityStateCase3(int code);

    MainAbility()
    {
        mapCase_ = {
            {(int)MissionStackApi::LockMission,
                {
                    [this](int code) { MissionStackCase1(code); },
                    [this](int code) { MissionStackCase2(code); },
                    [this](int code) { MissionStackCase3(code); },
                    [this](int code) { MissionStackCase4(code); },
                    [this](int code) { MissionStackCase5(code); },
                    [this](int code) { MissionStackCase6(code); },
                    [this](int code) { MissionStackCase7(code); },
                    [this](int code) { MissionStackCase8(code); },
                    [this](int code) { MissionStackCase9(code); },
                    [this](int code) { MissionStackCase10(code); },
                    [this](int code) { MissionStackCase11(code); },
                    [this](int code) { MissionStackCase12(code); },
                    [this](int code) { MissionStackCase13(code); },
                    [this](int code) { MissionStackCase14(code); },
                    [this](int code) { MissionStackCase15(code); },
                    [this](int code) { MissionStackCase16(code); },
                    [this](int code) { MissionStackCase17(code); },
                    [this](int code) { MissionStackCase18(code); },

                }},
            {(int)TestAbilityState::OnSaveAbilityState,
                {
                    [this](int code) { SaveAbilityStateCase1(code); },
                    [this](int code) { SaveAbilityStateCase2(code); },
                    [this](int code) { SaveAbilityStateCase3(code); },
                }},
            {(int)TestAbilityState::OnRestoreAbilityState,
                {
                    [this](int code) { RestoreAbilityStateCase1(code); },
                    [this](int code) { RestoreAbilityStateCase2(code); },
                    [this](int code) { RestoreAbilityStateCase3(code); },
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
    virtual void OnRestoreAbilityState(const PacMap &inState) override;
    virtual void OnSaveAbilityState(PacMap &outState) override;
    std::shared_ptr<FirstEventSubscriber> subscriber_;
};
class FirstEventSubscriber : public EventFwk::CommonEventSubscriber {
public:
    explicit FirstEventSubscriber(const EventFwk::CommonEventSubscribeInfo &sp) : CommonEventSubscriber(sp)
    {
        mapTestFunc_ = {
            {"MissionStack", [this](int apiIndex, int caseIndex, int code) { TestAbility(apiIndex, caseIndex, code); }},
            {"TestAbilityState", [this](int apiIndex, int caseIndex, int code) { TestAbility(apiIndex, caseIndex, code); }},
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