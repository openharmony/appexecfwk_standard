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

#include "application_state_observer_stub.h"
#include "native_engine/native_engine.h"
#include "event_handler.h"

#ifndef OHOS_APPEXECFWK_RUNTIME_APP_MANAGER_H
#define OHOS_APPEXECFWK_RUNTIME_APP_MANAGER_H

namespace OHOS {
namespace AbilityRuntime {
using OHOS::AppExecFwk::ApplicationStateObserverStub;
using OHOS::AppExecFwk::AppStateData;
using OHOS::AppExecFwk::AbilityStateData;
using OHOS::AppExecFwk::ProcessData;
NativeValue* JsAppManagerInit(NativeEngine* engine, NativeValue* exportObj);

class JSApplicationStateObserver : public ApplicationStateObserverStub {
public:
    void OnForegroundApplicationChanged(const AppStateData &appStateData) override;
    void OnAbilityStateChanged(const AbilityStateData &abilityStateData) override;
    void OnExtensionStateChanged(const AbilityStateData &abilityStateData) override;
    void OnProcessCreated(const ProcessData &processData) override;
    void OnProcessDied(const ProcessData &processData) override;
    void HandleOnForegroundApplicationChanged(const AppStateData &appStateData);
    void HandleOnAbilityStateChanged(const AbilityStateData &abilityStateData);
    void HandleOnExtensionStateChanged(const AbilityStateData &abilityStateData);
    void HandleOnProcessCreated(const ProcessData &processData);
    void HandleOnProcessDied(const ProcessData &processData);
    void SetNativeEngine(NativeEngine* engine);
    void SetJsObserverObject(NativeValue* jsObserverObject);
    void CallJsFunction(const char* methodName, NativeValue* const* argv, size_t argc);

private:
    NativeEngine* engine_ = nullptr;
    std::unique_ptr<NativeReference> jsObserverObject_ = nullptr;
};

static std::map<int64_t, sptr<JSApplicationStateObserver>> observerIds_;
static int64_t serialNumber_ = 0;
static std::shared_ptr<AppExecFwk::EventHandler> handler_ = nullptr;
}  // namespace AbilityRuntime
}  // namespace OHOS
#endif // OHOS_APPEXECFWK_RUNTIME_APP_MANAGER_H