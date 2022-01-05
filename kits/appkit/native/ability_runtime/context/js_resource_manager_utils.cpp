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

#include "js_resource_manager_utils.h"

#include "hilog_wrapper.h"
#include "js_runtime_utils.h"
#include "rstate.h"

namespace OHOS {
namespace AbilityRuntime {
namespace {
const int32_t GETSTRING_PARAM_SIZE = 1;
class JsResourceManager final {
public:
    JsResourceManager(const std::shared_ptr<Global::Resource::ResourceManager>& resourceManager)
        : resourceManager_(resourceManager) {}
    ~JsResourceManager() = default;

    static void Finalizer(NativeEngine* engine, void* data, void* hint)
    {
        HILOG_INFO("JsResourceManager::Finalizer is called");
        std::unique_ptr<JsResourceManager>(static_cast<JsResourceManager*>(data));
    }

    static NativeValue* GetString(NativeEngine* engine, NativeCallbackInfo* info)
    {
        JsResourceManager* me = CheckParamsAndGetThis<JsResourceManager>(engine, info);
        return me != nullptr ? me->GetString(*engine, *info) : nullptr;
    }

private:
    NativeValue* GetString(NativeEngine& engine, NativeCallbackInfo& info)
    {
        HILOG_INFO("JsResourceManager GetString is called");

        if (info.argc == 0) {
            HILOG_ERROR("Not enough params");
            return engine.CreateUndefined();
        }

        NativeNumber* number = ConvertNativeValueTo<NativeNumber>(info.argv[0]);
        uint32_t param = number ? *(number) : 0;

        AsyncTask::CompleteCallback complete =
            [weak = resourceManager_, resId = param](NativeEngine& engine, AsyncTask& task, int32_t status) {
                auto resourceManager = weak.lock();
                if (!resourceManager) {
                    HILOG_INFO("JsResourceManager is released");
                    task.Reject(engine, CreateJsError(engine, 1, "resourceManager is released"));
                    return;
                }
                std::string retValue;
                OHOS::Global::Resource::RState errval = resourceManager->GetStringById(resId, retValue);
                if (errval == OHOS::Global::Resource::RState::SUCCESS) {
                    HILOG_DEBUG("resourceManager GetStringById:%{public}s", retValue.c_str());
                    task.Resolve(engine, engine.CreateString(retValue.c_str(), retValue.length()));
                } else {
                    HILOG_DEBUG("resourceManager GetStringById failed");
                    task.Reject(engine, CreateJsError(engine, errval, "getString failed"));
                }
            };

        NativeValue* lastParam = info.argc > GETSTRING_PARAM_SIZE ? info.argv[GETSTRING_PARAM_SIZE] : nullptr;
        NativeValue* result = nullptr;
        AsyncTask::Schedule(
            engine, CreateAsyncTaskWithLastParam(engine, lastParam, nullptr, std::move(complete), &result));
        return result;
    }

    std::weak_ptr<Global::Resource::ResourceManager> resourceManager_;
};
} // namespace

NativeValue* CreateJsResourceManager(
    NativeEngine& engine, std::shared_ptr<Global::Resource::ResourceManager> resourceManager)
{
    NativeValue* objValue = engine.CreateObject();
    NativeObject* object = ConvertNativeValueTo<NativeObject>(objValue);

    std::unique_ptr<JsResourceManager> jsResourceManager = std::make_unique<JsResourceManager>(resourceManager);
    object->SetNativePointer(jsResourceManager.release(), JsResourceManager::Finalizer, nullptr);

    BindNativeFunction(engine, *object, "getString", JsResourceManager::GetString);
    return objValue;
}
}  // namespace AbilityRuntime
}  // namespace OHOS