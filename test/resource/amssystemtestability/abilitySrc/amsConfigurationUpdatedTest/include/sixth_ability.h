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

#ifndef AMS_CONFIGURATION_UPDATED_TEST_SIXTH_ABILITY_H
#define AMS_CONFIGURATION_UPDATED_TEST_SIXTH_ABILITY_H
#include "ability.h"
#include "ability_loader.h"
#include "common_event.h"
#include "common_event_manager.h"
#include "ability_append_test_info.h"

namespace OHOS {
namespace AppExecFwk {
using namespace OHOS::EventFwk;
class SixthAbilityEventSubscriber;
class SixthAbility : public Ability {
public:
    void SubscribeEvent();
    void TestAbility(int apiIndex, int caseIndex, int code);

    SixthAbility()
    {
        mapCase_ = {};
    }

    std::unordered_map<int, std::vector<std::function<void(int)>>> mapCase_;
    ~SixthAbility();

protected:
    void Init(const std::shared_ptr<AbilityInfo> &abilityInfo, const std::shared_ptr<OHOSApplication> &application,
        std::shared_ptr<AbilityHandler> &handler, const sptr<IRemoteObject> &token) override;
    virtual void OnStart(const Want &want) override;
    virtual void OnStop() override;
    virtual void OnActive() override;
    virtual void OnInactive() override;
    virtual void OnBackground() override;
    virtual void OnForeground(const Want &want) override;
    virtual void OnConfigurationUpdated(const Configuration &configuration) override;

    std::shared_ptr<SixthAbilityEventSubscriber> subscriber_;

    std::string callbackSeq;
    std::string callbackUpdated;
};
class SixthAbilityEventSubscriber : public EventFwk::CommonEventSubscriber {
public:
    explicit SixthAbilityEventSubscriber(const EventFwk::CommonEventSubscribeInfo &sp) : CommonEventSubscriber(sp)
    {
        mapTestFunc_ = {};
        sixthAbility = nullptr;
    }

    void TestAbility(int apiIndex, int caseIndex, int code)
    {
        sixthAbility->TestAbility(apiIndex, caseIndex, code);
    }

    virtual void OnReceiveEvent(const EventFwk::CommonEventData &data);

    SixthAbility *sixthAbility;
    std::unordered_map<std::string, std::function<void(int, int, int)>> mapTestFunc_;
    ~SixthAbilityEventSubscriber() = default;
};
}  // namespace AppExecFwk
}  // namespace OHOS
#endif  // AMS_CONFIGURATION_UPDATED_TEST_SIXTH_ABILITY_H