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

#ifndef FWK_ABILITY_STATE_ABILITY_B_H
#define FWK_ABILITY_STATE_ABILITY_B_H
#include <string>
#include "ability_loader.h"
#include "app_log_wrapper.h"
#include "common_event.h"
#include "common_event_manager.h"
#include "pac_map.h"
#include "stpageabilityevent.h"

namespace OHOS {
namespace AppExecFwk {
class FwkAbilityStateMainSubscriber;
class FwkAbilityStateSecond : public Ability {
public:
    FwkAbilityStateSecond()
    {
        mapAction_["Terminate"] = [this](int code) { TerminateThis(code); };
    }
    ~FwkAbilityStateSecond();
    virtual void OnStart(const Want &want) override;
    virtual void OnStop() override;
    virtual void OnActive() override;
    virtual void OnInactive() override;
    virtual void OnBackground() override;
    virtual void OnForeground(const Want &want) override;
    virtual void OnNewWant(const Want &want) override;
    virtual void OnSaveAbilityState(PacMap &outState) override;
    virtual void OnRestoreAbilityState(const PacMap &inState) override;

    void Action(std::string action, int code);
    void TerminateThis(int code);

private:
    std::shared_ptr<FwkAbilityStateMainSubscriber> subscriber_;
    std::unordered_map<std::string, std::function<void(int)>> mapAction_;
    void SubscribeEvent();
};

class FwkAbilityStateMainSubscriber : public EventFwk::CommonEventSubscriber {
public:
    explicit FwkAbilityStateMainSubscriber(const EventFwk::CommonEventSubscribeInfo &sp)
        : CommonEventSubscriber(sp)
    {
        mapAction_ = {
            {"Terminate", [this](std::string action, int code) { Action(action, code); }},
        };
        mainAbility = nullptr;
    }

    void Action(std::string action, int code)
    {
        mainAbility->Action(action, code);
    }

    virtual void OnReceiveEvent(const EventFwk::CommonEventData &data);

    FwkAbilityStateSecond *mainAbility;
    std::unordered_map<std::string, std::function<void(std::string, int)>> mapAction_;
    ~FwkAbilityStateMainSubscriber() = default;
};
}  // namespace AppExecFwk
}  // namespace OHOS
#endif  // _FWK_ABILITY_STATE_ABILITY_B_H