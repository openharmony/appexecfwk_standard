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

#ifndef VERIFY_ACT_FIRST_ABILITY
#define VERIFY_ACT_FIRST_ABILITY
#include "ability_loader.h"
#include "common_event.h"
#include "app_log_wrapper.h"
#include "common_event_manager.h"
#include "ability_connect_callback_stub.h"

namespace OHOS {
namespace AppExecFwk {
namespace {
const int CONNECT_SERVICE_ABILITY = 0;
const int DISCONNECT_SERVICE_ABILITY = 1;
const int START_SERVICE_ABILITY = 2;
const int STOP_SERVICE_ABILITY = 3;
}  // namespace
class VerifyActFirstAbility : public Ability {
public:
    virtual void OnStart(const Want &want) override;
    virtual void OnStop() override;
    virtual void OnActive() override;
    virtual void OnInactive() override;
    virtual void OnBackground() override;
};
class VerifyIOAbilityLifecycleCallbacks : public AbilityLifecycleCallbacks {
public:
    VerifyIOAbilityLifecycleCallbacks() = default;
    virtual ~VerifyIOAbilityLifecycleCallbacks() = default;

    virtual void OnAbilityStart(const std::shared_ptr<Ability> &ability);
    virtual void OnAbilityInactive(const std::shared_ptr<Ability> &ability);
    virtual void OnAbilityBackground(const std::shared_ptr<Ability> &ability);
    virtual void OnAbilityForeground(const std::shared_ptr<Ability> &ability);
    virtual void OnAbilityActive(const std::shared_ptr<Ability> &ability);
    virtual void OnAbilityStop(const std::shared_ptr<Ability> &ability);
    virtual void OnAbilitySaveState(const PacMap &outState);
};

class AbilityContextStartAbilityTest : public EventFwk::CommonEventSubscriber {
public:
    AbilityContextStartAbilityTest(const EventFwk::CommonEventSubscribeInfo &sp)
        : EventFwk::CommonEventSubscriber(sp){};
    virtual void OnReceiveEvent(const EventFwk::CommonEventData &data);
};
class ConnectServiceAbilityTest : public EventFwk::CommonEventSubscriber {
public:
    ConnectServiceAbilityTest(const EventFwk::CommonEventSubscribeInfo &sp, std::shared_ptr<Context> abilityContext,
        sptr<AAFwk::IAbilityConnection> conne)
        : EventFwk::CommonEventSubscriber(sp)
    {
        abilityContext_ = abilityContext;
        conne_ = conne;
    };
    virtual void OnReceiveEvent(const EventFwk::CommonEventData &data) override;

    std::shared_ptr<Context> abilityContext_ = nullptr;
    sptr<AAFwk::IAbilityConnection> conne_ = nullptr;
};
class AbilityConnectionActFirst : public AAFwk::AbilityConnectionStub {
public:
    AbilityConnectionActFirst(){};
    virtual ~AbilityConnectionActFirst(){};
    virtual void OnAbilityConnectDone(
        const AppExecFwk::ElementName &element, const sptr<IRemoteObject> &remoteObject, int resultCode) override;
    virtual void OnAbilityDisconnectDone(const AppExecFwk::ElementName &element, int resultCode) override;
    static int onAbilityConnectDoneCount;
    static int onAbilityDisconnectDoneCount;
    static int test;
};
}  // namespace AppExecFwk
}  // namespace OHOS
#endif  // VERIFY_ACT_FIRST_ABILITY