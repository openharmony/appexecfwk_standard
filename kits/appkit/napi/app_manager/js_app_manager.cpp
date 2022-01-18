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

#include "js_app_manager.h"

#include <cstdint>

#include "ability_manager_interface.h"
#include "app_mgr_interface.h"
#include "hilog_wrapper.h"
#include "js_runtime.h"
#include "js_runtime_utils.h"
#include "napi/native_api.h"
#include "if_system_ability_manager.h"
#include "iservice_registry.h"
#include "system_ability_definition.h"
#include "js_app_manager_utils.h"
#include "event_runner.h"

namespace OHOS {
namespace AbilityRuntime {
namespace {
constexpr int32_t INDEX_ZERO = 0;
constexpr int32_t INDEX_ONE = 1;
constexpr int32_t ERROR_CODE_ONE = 1;
constexpr size_t ARGC_ZERO = 0;
constexpr size_t ARGC_ONE = 1;
constexpr size_t ARGC_TWO = 2;

class JsAppManager final {
public:
    JsAppManager(sptr<OHOS::AppExecFwk::IAppMgr> appManager,
        sptr<OHOS::AAFwk::IAbilityManager> abilityManager) : appManager_(appManager),
        abilityManager_(abilityManager) {}
    ~JsAppManager() = default;

    static void Finalizer(NativeEngine* engine, void* data, void* hint)
    {
        HILOG_INFO("JsAbilityContext::Finalizer is called");
        std::unique_ptr<JsAppManager>(static_cast<JsAppManager*>(data));
    }

    static NativeValue* RegisterApplicationStateObserver(NativeEngine* engine, NativeCallbackInfo* info)
    {
        JsAppManager* me = CheckParamsAndGetThis<JsAppManager>(engine, info);
        return (me != nullptr) ? me->OnRegisterApplicationStateObserver(*engine, *info) : nullptr;
    }

    static NativeValue* UnregisterApplicationStateObserver(NativeEngine* engine, NativeCallbackInfo* info)
    {
        JsAppManager* me = CheckParamsAndGetThis<JsAppManager>(engine, info);
        return (me != nullptr) ? me->OnUnregisterApplicationStateObserver(*engine, *info) : nullptr;
    }

    static NativeValue* GetForegroundApplications(NativeEngine* engine, NativeCallbackInfo* info)
    {
        JsAppManager* me = CheckParamsAndGetThis<JsAppManager>(engine, info);
        return (me != nullptr) ? me->OnGetForegroundApplications(*engine, *info) : nullptr;
    }

    static NativeValue* GetProcessRunningInfos(NativeEngine* engine, NativeCallbackInfo* info)
    {
        JsAppManager* me = CheckParamsAndGetThis<JsAppManager>(engine, info);
        return (me != nullptr) ? me->OnGetProcessRunningInfos(*engine, *info) : nullptr;
    }

    static NativeValue* IsUserAStabilityTest(NativeEngine* engine, NativeCallbackInfo* info)
    {
        JsAppManager* me = CheckParamsAndGetThis<JsAppManager>(engine, info);
        return (me != nullptr) ? me->OnIsUserAStabilityTest(*engine, *info) : nullptr;
    }

private:
    sptr<OHOS::AppExecFwk::IAppMgr> appManager_ = nullptr;
    sptr<OHOS::AAFwk::IAbilityManager> abilityManager_ = nullptr;

    NativeValue* OnRegisterApplicationStateObserver(NativeEngine& engine, NativeCallbackInfo& info)
    {
        HILOG_INFO("OnRegisterApplicationStateObserver is called");
        // only support one or two params
        if (info.argc != ARGC_ONE) {
            HILOG_ERROR("Not enough params");
            return engine.CreateUndefined();
        }

        // unwarp observer
        sptr<JSApplicationStateObserver> observer = new JSApplicationStateObserver();
        observer->SetNativeEngine(&engine);
        observer->SetJsObserverObject(info.argv[0]);
        int64_t observerId = serialNumber_;
        observerIds_.emplace(observerId, observer);
        if (serialNumber_ < INT64_MAX) {
            serialNumber_++;
        } else {
            serialNumber_ = 0;
        }
        HILOG_INFO("%{public}s create observer:%{public}p", __func__, observer.GetRefPtr());
        AsyncTask::CompleteCallback complete =
            [appManager = appManager_, observer](NativeEngine& engine, AsyncTask& task, int32_t status) {
                HILOG_INFO("RegisterApplicationStateObserver callback begin");
                if (appManager == nullptr) {
                    HILOG_ERROR("appManager nullptr");
                    task.Reject(engine, CreateJsError(engine, ERROR_CODE_ONE, "appManager nullptr"));
                    return;
                }
                int32_t err = appManager->RegisterApplicationStateObserver(observer);
                if (err == 0) {
                    HILOG_INFO("RegisterApplicationStateObserver success err 0");
                } else {
                    HILOG_ERROR("RegisterApplicationStateObserver failed error:%{public}d", err);
                }
            };

        NativeValue* result = nullptr;
        AsyncTask::Schedule(
            engine, CreateAsyncTaskWithLastParam(engine, nullptr, nullptr, std::move(complete), &result));
        return engine.CreateNumber(observerId);
    }

    NativeValue* OnUnregisterApplicationStateObserver(NativeEngine& engine, NativeCallbackInfo& info)
    {
        HILOG_INFO("OnUnregisterApplicationStateObserver is called");
        // only support one or two params
        if (info.argc != ARGC_ONE && info.argc != ARGC_TWO) {
            HILOG_ERROR("Not enough params");
            return engine.CreateUndefined();
        }

        // unwrap connectId
        int64_t observerId = -1;
        sptr<JSApplicationStateObserver> observer = nullptr;
        napi_get_value_int64(reinterpret_cast<napi_env>(&engine),
            reinterpret_cast<napi_value>(info.argv[INDEX_ZERO]), &observerId);
        HILOG_INFO("OnUnregisterApplicationStateObserver observer:%{public}d", (int32_t)observerId);
        auto item = observerIds_.find(observerId);
        if (item != observerIds_.end()) {
            // match id
            observer = item->second;
            HILOG_INFO("%{public}s find observer:%{public}p exist", __func__, item->second.GetRefPtr());
        } else {
            HILOG_INFO("%{public}s not find observer exist.", __func__);
        }

        AsyncTask::CompleteCallback complete =
            [appManager = appManager_, observer, observerId](
                NativeEngine& engine, AsyncTask& task, int32_t status) {
                HILOG_INFO("OnUnregisterApplicationStateObserver begin");
                if (observer == nullptr || appManager == nullptr) {
                    HILOG_ERROR("observer or appManager nullptr");
                    task.Reject(engine, CreateJsError(engine, ERROR_CODE_ONE, "observer or appManager nullptr"));
                    return;
                }
                HILOG_INFO("observer->UnregisterApplicationStateObserver");
                int32_t err = appManager->UnregisterApplicationStateObserver(observer);
                if (err == 0) {
                    HILOG_INFO("UnregisterApplicationStateObserver success err 0");
                    task.Resolve(engine, engine.CreateUndefined());
                    observerIds_.erase(observerId);
                    HILOG_INFO("UnregisterApplicationStateObserver erase size:%{public}zu", observerIds_.size());
                } else {
                    HILOG_ERROR("UnregisterApplicationStateObserver failed error:%{public}d", err);
                    task.Reject(engine, CreateJsError(engine, err, "UnregisterApplicationStateObserver failed"));
                }
            };

        NativeValue* lastParam = (info.argc == ARGC_ONE) ? nullptr : info.argv[INDEX_ONE];
        NativeValue* result = nullptr;
        AsyncTask::Schedule(
            engine, CreateAsyncTaskWithLastParam(engine, lastParam, nullptr, std::move(complete), &result));
        return result;
    }

    NativeValue* OnGetForegroundApplications(NativeEngine& engine, NativeCallbackInfo& info)
    {
        // only support 0 or 1 params
        if (info.argc != ARGC_ONE && info.argc != ARGC_ZERO) {
            HILOG_ERROR("Not enough params");
            return engine.CreateUndefined();
        }
        AsyncTask::CompleteCallback complete =
            [appManager = appManager_](
                NativeEngine& engine, AsyncTask& task, int32_t status) {
                HILOG_INFO("OnGetForegroundApplications begin");
                if (appManager == nullptr) {
                    HILOG_ERROR("appManager nullptr");
                    task.Reject(engine, CreateJsError(engine, ERROR_CODE_ONE, "appManager nullptr"));
                    return;
                }
                std::vector<AppExecFwk::AppStateData> list;
                int32_t err = appManager->GetForegroundApplications(list);
                if (err == 0) {
                    HILOG_INFO("OnGetForegroundApplications success err 0");
                    task.Resolve(engine, CreateJsAppStateDataArray(engine, list));
                } else {
                    HILOG_ERROR("OnGetForegroundApplications failed error:%{public}d", err);
                    task.Reject(engine, CreateJsError(engine, err, "OnGetForegroundApplications failed"));
                }
            };

        NativeValue* lastParam = (info.argc == ARGC_ZERO) ? nullptr : info.argv[INDEX_ZERO];
        NativeValue* result = nullptr;
        AsyncTask::Schedule(
            engine, CreateAsyncTaskWithLastParam(engine, lastParam, nullptr, std::move(complete), &result));
        return result;
    }

    NativeValue* OnGetProcessRunningInfos(NativeEngine &engine, NativeCallbackInfo &info)
    {
        AsyncTask::CompleteCallback complete =
            [appManager = appManager_](
                NativeEngine &engine, AsyncTask &task, int32_t status) {
                std::vector<AppExecFwk::RunningProcessInfo> infos;
                auto errcode = appManager->GetAllRunningProcesses(infos);
                if (errcode == 0) {
                    task.Resolve(engine, CreateJsProcessRunningInfoArray(engine, infos));
                } else {
                    task.Reject(engine, CreateJsError(engine, errcode, "Get mission infos failed."));
                }
            };

        NativeValue* lastParam = (info.argc == 0) ? nullptr : info.argv[0];
        NativeValue* result = nullptr;
        AsyncTask::Schedule(
            engine, CreateAsyncTaskWithLastParam(engine, lastParam, nullptr, std::move(complete), &result));
        return result;
    }

    NativeValue* OnIsUserAStabilityTest(NativeEngine& engine, NativeCallbackInfo& info)
    {
        // only support 0 or 1 params
        if (info.argc != ARGC_ONE && info.argc != ARGC_ZERO) {
            HILOG_ERROR("Not enough params");
            return engine.CreateUndefined();
        }
        AsyncTask::CompleteCallback complete =
            [abilityManager = abilityManager_](NativeEngine& engine, AsyncTask& task, int32_t status) {
                if (abilityManager == nullptr) {
                    HILOG_WARN("abilityManager nullptr");
                    task.Reject(engine, CreateJsError(engine, ERROR_CODE_ONE, "abilityManager nullptr"));
                    return;
                }
                HILOG_INFO("IsUserAStabilityTest begin");
                bool ret = abilityManager->IsUserAStabilityTest();
                HILOG_INFO("IsUserAStabilityTest result:%{public}d", ret);
                task.Resolve(engine, CreateJsValue(engine, ret));
            };

        NativeValue* lastParam = (info.argc == ARGC_ZERO) ? nullptr : info.argv[INDEX_ZERO];
        NativeValue* result = nullptr;
        AsyncTask::Schedule(
            engine, CreateAsyncTaskWithLastParam(engine, lastParam, nullptr, std::move(complete), &result));
        return result;
    }
};
} // namespace

OHOS::sptr<OHOS::AppExecFwk::IAppMgr> GetAppManagerInstance()
{
    OHOS::sptr<OHOS::ISystemAbilityManager> systemAbilityManager =
        OHOS::SystemAbilityManagerClient::GetInstance().GetSystemAbilityManager();
    OHOS::sptr<OHOS::IRemoteObject> appObject = systemAbilityManager->GetSystemAbility(OHOS::APP_MGR_SERVICE_ID);
    return OHOS::iface_cast<OHOS::AppExecFwk::IAppMgr>(appObject);
}

OHOS::sptr<OHOS::AAFwk::IAbilityManager> GetAbilityManagerInstance()
{
    OHOS::sptr<OHOS::ISystemAbilityManager> systemAbilityManager =
        OHOS::SystemAbilityManagerClient::GetInstance().GetSystemAbilityManager();
    OHOS::sptr<OHOS::IRemoteObject> abilityObject =
        systemAbilityManager->GetSystemAbility(OHOS::ABILITY_MGR_SERVICE_ID);
    return OHOS::iface_cast<OHOS::AAFwk::IAbilityManager>(abilityObject);
}

NativeValue* JsAppManagerInit(NativeEngine* engine, NativeValue* exportObj)
{
    HILOG_INFO("JsAppManagerInit is called");

    if (engine == nullptr || exportObj == nullptr) {
        HILOG_INFO("engine or exportObj null");
        return nullptr;
    }

    NativeObject* object = ConvertNativeValueTo<NativeObject>(exportObj);
    if (object == nullptr) {
        HILOG_INFO("object null");
        return nullptr;
    }

    std::unique_ptr<JsAppManager> jsAppManager =
        std::make_unique<JsAppManager>(GetAppManagerInstance(), GetAbilityManagerInstance());
    object->SetNativePointer(jsAppManager.release(), JsAppManager::Finalizer, nullptr);

    //make handler
    handler_ = std::make_shared<AppExecFwk::EventHandler>(AppExecFwk::EventRunner::GetMainEventRunner());

    HILOG_INFO("JsAppManagerInit BindNativeFunction called");
    BindNativeFunction(*engine, *object, "registerApplicationStateObserver",
        JsAppManager::RegisterApplicationStateObserver);
    BindNativeFunction(*engine, *object, "unregisterApplicationStateObserver",
        JsAppManager::UnregisterApplicationStateObserver);
    BindNativeFunction(*engine, *object, "getForegroundApplications",
        JsAppManager::GetForegroundApplications);
    BindNativeFunction(*engine, *object, "isUserAStabilityTest",
        JsAppManager::IsUserAStabilityTest);
    BindNativeFunction(*engine, *object, "getProcessRunningInfos",
        JsAppManager::GetProcessRunningInfos);
    HILOG_INFO("JsAppManagerInit end");
    return engine->CreateUndefined();
}

void JSApplicationStateObserver::OnForegroundApplicationChanged(const AppStateData &appStateData)
{
    HILOG_DEBUG("onForegroundApplicationChanged bundleName:%{public}s, uid:%{public}d, state:%{public}d",
        appStateData.bundleName.c_str(), appStateData.uid, appStateData.state);
    if (handler_ == nullptr) {
        HILOG_INFO("handler_ nullptr");
        return;
    }
    wptr<JSApplicationStateObserver> jsObserver = this;
    auto task = [jsObserver, appStateData]() {
        sptr<JSApplicationStateObserver> jsObserverSptr = jsObserver.promote();
        if (!jsObserverSptr) {
            HILOG_INFO("jsObserverSptr nullptr");
            return;
        }
        jsObserverSptr->HandleOnForegroundApplicationChanged(appStateData);
    };
    handler_->PostTask(task, "OnForegroundApplicationChanged");
}

void JSApplicationStateObserver::HandleOnForegroundApplicationChanged(const AppStateData &appStateData)
{
    HILOG_DEBUG("HandleOnForegroundApplicationChanged bundleName:%{public}s, uid:%{public}d, state:%{public}d",
        appStateData.bundleName.c_str(), appStateData.uid, appStateData.state);
    if (engine_ == nullptr) {
        HILOG_ERROR("engine_ nullptr");
        return;
    }
    NativeValue* argv[] = {CreateJsAppStateData(*engine_, appStateData)};
    CallJsFunction("onForegroundApplicationChanged", argv, ARGC_ONE);
}

void JSApplicationStateObserver::OnAbilityStateChanged(const AbilityStateData &abilityStateData)
{
    HILOG_INFO("OnAbilityStateChanged begin");
    if (handler_ == nullptr) {
        HILOG_INFO("handler_ nullptr");
        return;
    }
    wptr<JSApplicationStateObserver> jsObserver = this;
    auto task = [jsObserver, abilityStateData]() {
        sptr<JSApplicationStateObserver> jsObserverSptr = jsObserver.promote();
        if (!jsObserverSptr) {
            HILOG_INFO("jsObserverSptr nullptr");
            return;
        }
        jsObserverSptr->HandleOnAbilityStateChanged(abilityStateData);
    };
    handler_->PostTask(task, "OnAbilityStateChanged");
}

void JSApplicationStateObserver::HandleOnAbilityStateChanged(const AbilityStateData &abilityStateData)
{
    HILOG_INFO("HandleOnAbilityStateChanged begin");
    if (engine_ == nullptr) {
        HILOG_ERROR("engine_ nullptr");
        return;
    }
    NativeValue* argv[] = {CreateJsAbilityStateData(*engine_, abilityStateData)};
    CallJsFunction("onAbilityStateChanged", argv, ARGC_ONE);
}

void JSApplicationStateObserver::OnExtensionStateChanged(const AbilityStateData &abilityStateData)
{
    HILOG_INFO("OnExtensionStateChanged begin");
    if (handler_ == nullptr) {
        HILOG_INFO("handler_ nullptr");
        return;
    }
    wptr<JSApplicationStateObserver> jsObserver = this;
    auto task = [jsObserver, abilityStateData]() {
        sptr<JSApplicationStateObserver> jsObserverSptr = jsObserver.promote();
        if (!jsObserverSptr) {
            HILOG_INFO("jsObserverSptr nullptr");
            return;
        }
        jsObserverSptr->HandleOnExtensionStateChanged(abilityStateData);
    };
    handler_->PostTask(task, "OnExtensionStateChanged");
}

void JSApplicationStateObserver::HandleOnExtensionStateChanged(const AbilityStateData &abilityStateData)
{
    HILOG_INFO("HandleOnExtensionStateChanged begin");
    if (engine_ == nullptr) {
        HILOG_ERROR("engine_ nullptr");
        return;
    }
    NativeValue* argv[] = {CreateJsAbilityStateData(*engine_, abilityStateData)};
    CallJsFunction("onExtensionStateChanged", argv, ARGC_ONE);
}

void JSApplicationStateObserver::OnProcessCreated(const ProcessData &processData)
{
    HILOG_INFO("OnProcessCreated begin");
    if (handler_ == nullptr) {
        HILOG_INFO("handler_ nullptr");
        return;
    }
    wptr<JSApplicationStateObserver> jsObserver = this;
    auto task = [jsObserver, processData]() {
        sptr<JSApplicationStateObserver> jsObserverSptr = jsObserver.promote();
        if (!jsObserverSptr) {
            HILOG_INFO("jsObserverSptr nullptr");
            return;
        }
        jsObserverSptr->HandleOnProcessCreated(processData);
    };
    handler_->PostTask(task, "OnProcessCreated");
}

void JSApplicationStateObserver::HandleOnProcessCreated(const ProcessData &processData)
{
    HILOG_INFO("HandleOnProcessCreated begin");
    if (engine_ == nullptr) {
        HILOG_ERROR("engine_ nullptr");
        return;
    }
    NativeValue* argv[] = {CreateJsProcessData(*engine_, processData)};
    CallJsFunction("onProcessCreated", argv, ARGC_ONE);
}

void JSApplicationStateObserver::OnProcessDied(const ProcessData &processData)
{
    HILOG_INFO("OnProcessDied begin");
    if (handler_ == nullptr) {
        HILOG_INFO("handler_ nullptr");
        return;
    }
    wptr<JSApplicationStateObserver> jsObserver = this;
    auto task = [jsObserver, processData]() {
        sptr<JSApplicationStateObserver> jsObserverSptr = jsObserver.promote();
        if (!jsObserverSptr) {
            HILOG_INFO("jsObserverSptr nullptr");
            return;
        }
        jsObserverSptr->HandleOnProcessDied(processData);
    };
    handler_->PostTask(task, "OnProcessDied");
}

void JSApplicationStateObserver::HandleOnProcessDied(const ProcessData &processData)
{
    HILOG_INFO("HandleOnProcessDied begin");
    if (engine_ == nullptr) {
        HILOG_ERROR("engine_ nullptr");
        return;
    }
    NativeValue* argv[] = {CreateJsProcessData(*engine_, processData)};
    CallJsFunction("onProcessDied", argv, ARGC_ONE);
}

void JSApplicationStateObserver::CallJsFunction(const char* methodName, NativeValue* const* argv, size_t argc)
{
    HILOG_INFO("CallJsFunction begin, method:%{public}s", methodName);
    if (jsObserverObject_ == nullptr) {
        HILOG_ERROR("jsObserverObject_ nullptr");
        return;
    }
    NativeValue* value = jsObserverObject_->Get();
    NativeObject* obj = ConvertNativeValueTo<NativeObject>(value);
    if (obj == nullptr) {
        HILOG_ERROR("Failed to get object");
        return;
    }

    NativeValue* method = obj->GetProperty(methodName);
    if (method == nullptr) {
        HILOG_ERROR("Failed to get from object");
        return;
    }
    if (engine_ == nullptr) {
        HILOG_ERROR("engine_ nullptr");
        return;
    }
    HILOG_INFO("CallJsFunction CallFunction success");
    engine_->CallFunction(value, method, argv, argc);
    HILOG_INFO("CallJsFunction end");
}

void JSApplicationStateObserver::SetNativeEngine(NativeEngine* engine)
{
    engine_ = engine;
}

void JSApplicationStateObserver::SetJsObserverObject(NativeValue* jsObserverObject)
{
    jsObserverObject_ = std::unique_ptr<NativeReference>(engine_->CreateReference(jsObserverObject, 1));
}
}  // namespace AbilityRuntime
}  // namespace OHOS
