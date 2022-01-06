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

#ifndef FOUNDATION_APPEXECFWK_INTERFACES_INNERKITS_APPEXECFWK_CORE_INCLUDE_APPMGR_IAPPLICATION_STATE_OBSERVER_H
#define FOUNDATION_APPEXECFWK_INTERFACES_INNERKITS_APPEXECFWK_CORE_INCLUDE_APPMGR_IAPPLICATION_STATE_OBSERVER_H

#include "ability_state_data.h"
#include "app_state_data.h"
#include "process_data.h"
#include "iremote_broker.h"
#include "iremote_object.h"

namespace OHOS {
namespace AppExecFwk {
class IApplicationStateObserver : public IRemoteBroker {
public:
    DECLARE_INTERFACE_DESCRIPTOR(u"ohos.appexecfwk.IApplicationStateObserver");

    /**
     * Application foreground state changed callback.
     *
     * @param appStateData Application state data.
     */
    virtual void OnForegroundApplicationChanged(const AppStateData &appStateData) = 0;

    /**
     * Will be called when the ability state changes.
     *
     * @param abilityStateData Ability state data.
     */
    virtual void OnAbilityStateChanged(const AbilityStateData &abilityStateData) = 0;

    /**
     * Will be called when the extension state changes.
     *
     * @param abilityStateData Extension state data.
     */
    virtual void OnExtensionStateChanged(const AbilityStateData &abilityStateData) = 0;

    /**
     * Will be called when the process start.
     *
     * @param processData Process data.
     */
    virtual void OnProcessCreated(const ProcessData &processData) = 0;

    /**
     * Will be called when the process die.
     *
     * @param processData Process data.
     */
    virtual void OnProcessDied(const ProcessData &processData) = 0;

    enum class Message {
        TRANSACT_ON_FOREGROUND_APPLICATION_CHANGED = 0,
        TRANSACT_ON_ABILITY_STATE_CHANGED,
        TRANSACT_ON_EXTENSION_STATE_CHANGED,
        TRANSACT_ON_PROCESS_CREATED,
        TRANSACT_ON_PROCESS_DIED,
    };
};
}  // namespace AppExecFwk
}  // namespace OHOS
#endif  // FOUNDATION_APPEXECFWK_INTERFACES_INNERKITS_APPEXECFWK_CORE_INCLUDE_APPMGR_IAPPLICATION_STATE_OBSERVER_H
