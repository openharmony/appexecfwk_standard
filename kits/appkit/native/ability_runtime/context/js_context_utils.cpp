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

#include "js_context_utils.h"

#include "hilog_wrapper.h"
#include "js_data_struct_converter.h"
#include "js_hap_module_info_utils.h"
#include "js_resource_manager_utils.h"
#include "js_runtime_utils.h"

namespace OHOS {
namespace AbilityRuntime {
namespace {
constexpr char BASE_CONTEXT_NAME[] = "__base_context_ptr__";

class JsBaseContext {
public:
    explicit JsBaseContext(std::weak_ptr<Context>&& context) : context_(std::move(context)) {}
    virtual ~JsBaseContext() = default;

    static void Finalizer(NativeEngine* engine, void* data, void* hint);
    static NativeValue* CreateBundleContext(NativeEngine* engine, NativeCallbackInfo* info);
    static NativeValue* GetApplicationContext(NativeEngine* engine, NativeCallbackInfo* info);

    void KeepContext(std::shared_ptr<Context> context)
    {
        keepContext_ = context;
    }

private:
    NativeValue* OnCreateBundleContext(NativeEngine& engine, NativeCallbackInfo& info);
    NativeValue* OnGetApplicationContext(NativeEngine& engine, NativeCallbackInfo& info);

    std::shared_ptr<Context> keepContext_;

protected:
    std::weak_ptr<Context> context_;
};

void JsBaseContext::Finalizer(NativeEngine* engine, void* data, void* hint)
{
    HILOG_INFO("JsBaseContext::Finalizer is called");
    std::unique_ptr<JsBaseContext>(static_cast<JsBaseContext*>(data));
}

NativeValue* JsBaseContext::CreateBundleContext(NativeEngine* engine, NativeCallbackInfo* info)
{
    JsBaseContext* me = CheckParamsAndGetThis<JsBaseContext>(engine, info, BASE_CONTEXT_NAME);
    return me != nullptr ? me->OnCreateBundleContext(*engine, *info) : nullptr;
}

NativeValue* JsBaseContext::GetApplicationContext(NativeEngine* engine, NativeCallbackInfo* info)
{
    JsBaseContext* me = CheckParamsAndGetThis<JsBaseContext>(engine, info, BASE_CONTEXT_NAME);
    return me != nullptr ? me->OnGetApplicationContext(*engine, *info) : nullptr;
}

NativeValue* JsBaseContext::OnCreateBundleContext(NativeEngine& engine, NativeCallbackInfo& info)
{
    if (info.argc == 0) {
        HILOG_ERROR("Not enough params");
        return engine.CreateUndefined();
    }

    auto context = context_.lock();
    if (!context) {
        HILOG_WARN("context is already released");
        return engine.CreateUndefined();
    }

    std::string bundleName;
    if (!ConvertFormJsValue(engine, info.argv[0], bundleName)) {
        HILOG_ERROR("Parse bundleName failed");
        return engine.CreateUndefined();
    }

    auto bundleContext = context->CreateBundleContext(bundleName);
    if (!bundleContext) {
        HILOG_ERROR("bundleContext is nullptr");
        return engine.CreateUndefined();
    }

    JsRuntime& jsRuntime = *static_cast<JsRuntime*>(engine.GetJsEngine());
    NativeValue* value = CreateJsBaseContext(engine, bundleContext, true);
    return jsRuntime.LoadSystemModule("application.AbilityStageContext", &value, 1)->Get();
}

NativeValue* JsBaseContext::OnGetApplicationContext(NativeEngine& engine, NativeCallbackInfo& info)
{
    auto context = context_.lock();
    if (!context) {
        HILOG_WARN("context is already released");
        return engine.CreateUndefined();
    }

    auto appContext = context->GetApplicationContext();
    if (!appContext) {
        HILOG_ERROR("appContext is nullptr");
        return engine.CreateUndefined();
    }

    JsRuntime& jsRuntime = *static_cast<JsRuntime*>(engine.GetJsEngine());
    NativeValue* value = CreateJsBaseContext(engine, appContext, true);
    return jsRuntime.LoadSystemModule("application.Context", &value, 1)->Get();
}
} // namespace

NativeValue* CreateJsBaseContext(NativeEngine& engine, std::shared_ptr<Context> context, bool keepContext)
{
    NativeValue* objValue = engine.CreateObject();
    NativeObject* object = ConvertNativeValueTo<NativeObject>(objValue);

    auto jsContext = std::make_unique<JsBaseContext>(context);
    if (keepContext) {
        jsContext->KeepContext(context);
    }
    SetNamedNativePointer(engine, *object, BASE_CONTEXT_NAME, jsContext.release(), JsBaseContext::Finalizer);

    auto appInfo = context->GetApplicationInfo();
    if (appInfo != nullptr) {
        object->SetProperty("applicationInfo", CreateJsApplicationInfo(engine, *appInfo));
    }
    auto hapModuleInfo = context->GetHapModuleInfo();
    if (hapModuleInfo != nullptr) {
        object->SetProperty("currentHapModuleInfo", CreateJsHapModuleInfo(engine, *hapModuleInfo));
    }
    auto resourceManager = context->GetResourceManager();
    if (resourceManager != nullptr) {
        object->SetProperty("resourceManager", CreateJsResourceManager(engine, resourceManager));
    }
    object->SetProperty("cacheDir", CreateJsValue(engine, context->GetCacheDir()));
    object->SetProperty("tempDir", CreateJsValue(engine, context->GetTempDir()));
    object->SetProperty("filesDir", CreateJsValue(engine, context->GetFilesDir()));
    object->SetProperty("distributedFilesDir", CreateJsValue(engine, context->GetDistributedFilesDir()));
    object->SetProperty("databaseDir", CreateJsValue(engine, context->GetDatabaseDir()));
    object->SetProperty("storageDir", CreateJsValue(engine, context->GetStorageDir()));
    object->SetProperty("bundleCodeDir", CreateJsValue(engine, context->GetBundleCodeDir()));

    BindNativeFunction(engine, *object, "createBundleContext", JsBaseContext::CreateBundleContext);
    BindNativeFunction(engine, *object, "getApplicationContext", JsBaseContext::GetApplicationContext);

    return objValue;
}
}  // namespace AbilityRuntime
}  // namespace OHOS
