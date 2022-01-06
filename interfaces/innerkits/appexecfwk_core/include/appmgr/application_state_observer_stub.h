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

#ifndef FOUNDATION_APPEXECFWK_INTERFACES_INNERKITS_APPEXECFWK_CORE_INCLUDE_APPMGR_APPLICATION_STATE_OBSERVER_STUB_H
#define FOUNDATION_APPEXECFWK_INTERFACES_INNERKITS_APPEXECFWK_CORE_INCLUDE_APPMGR_APPLICATION_STATE_OBSERVER_STUB_H

#include <map>

#include "iremote_stub.h"
#include "nocopyable.h"
#include "string_ex.h"
#include "app_mgr_constants.h"
#include "iapplication_state_observer.h"

namespace OHOS {
namespace AppExecFwk {
class ApplicationStateObserverStub : public IRemoteStub<IApplicationStateObserver> {
public:
    ApplicationStateObserverStub();
    virtual ~ApplicationStateObserverStub();

    virtual int OnRemoteRequest(
        uint32_t code, MessageParcel &data, MessageParcel &reply, MessageOption &option) override;

    /**
     * Application foreground state changed callback.
     *
     * @param appStateData Application Process data.
     */
    virtual void OnForegroundApplicationChanged(const AppStateData &appStateData) override;

    /**
     * Will be called when the ability state changes.
     *
     * @param abilityStateData Ability state data.
     */
    virtual void OnAbilityStateChanged(const AbilityStateData &abilityStateData) override;

    /**
     * Will be called when the extension state changes.
     *
     * @param abilityStateData Extension state data.
     */
    virtual void OnExtensionStateChanged(const AbilityStateData &abilityStateData) override;

    /**
     * Will be called when the process start.
     *
     * @param processData Process data.
     */
    virtual void OnProcessCreated(const ProcessData &processData) override;

    /**
     * Will be called when the process die.
     *
     * @param processData Process data.
     */
    virtual void OnProcessDied(const ProcessData &processData) override;

private:
    int32_t HandleOnForegroundApplicationChanged(MessageParcel &data, MessageParcel &reply);

    int32_t HandleOnAbilityStateChanged(MessageParcel &data, MessageParcel &reply);

    int32_t HandleOnExtensionStateChanged(MessageParcel &data, MessageParcel &reply);

    int32_t HandleOnProcessCreated(MessageParcel &data, MessageParcel &reply);

    int32_t HandleOnProcessDied(MessageParcel &data, MessageParcel &reply);

    using ApplicationStateObserverFunc = int32_t (ApplicationStateObserverStub::*)(MessageParcel &data,
        MessageParcel &reply);
    std::map<uint32_t, ApplicationStateObserverFunc> memberFuncMap_;

    DISALLOW_COPY_AND_MOVE(ApplicationStateObserverStub);
};

/**
 * @class ApplicationStateObserverRecipient
 * ApplicationStateObserverRecipient notices IRemoteBroker died.
 */
class ApplicationStateObserverRecipient : public IRemoteObject::DeathRecipient {
public:
    using RemoteDiedHandler = std::function<void(const wptr<IRemoteObject> &)>;
    ApplicationStateObserverRecipient(RemoteDiedHandler handler);
    virtual ~ApplicationStateObserverRecipient();
    virtual void OnRemoteDied(const wptr<IRemoteObject> &remote);

private:
    RemoteDiedHandler handler_;
};
}  // namespace AppExecFwk
}  // namespace OHOS
#endif  // FOUNDATION_APPEXECFWK_INTERFACES_INNERKITS_APPEXECFWK_CORE_INCLUDE_APPMGR_APPLICATION_STATE_OBSERVER_STUB_H
