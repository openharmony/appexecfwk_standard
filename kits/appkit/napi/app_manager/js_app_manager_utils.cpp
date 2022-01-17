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

#include "js_app_manager_utils.h"

#include <cstdint>

#include "hilog_wrapper.h"
#include "iapplication_state_observer.h"
#include "js_runtime.h"
#include "js_runtime_utils.h"

namespace OHOS {
namespace AbilityRuntime {
NativeValue* CreateJsAppStateData(NativeEngine &engine, const AppStateData &appStateData)
{
    HILOG_INFO("%{public}s called.", __func__); 
    NativeValue* objValue = engine.CreateObject();
    NativeObject* object = ConvertNativeValueTo<NativeObject>(objValue);
    object->SetProperty("bundleName", CreateJsValue(engine, appStateData.bundleName));
    object->SetProperty("uid", CreateJsValue(engine, appStateData.uid));
    object->SetProperty("state", CreateJsValue(engine, appStateData.state));
    return objValue;
}

NativeValue* CreateJsAbilityStateData(NativeEngine &engine, const AbilityStateData &abilityStateData)
{
    HILOG_INFO("%{public}s called.", __func__); 
    NativeValue* objValue = engine.CreateObject();
    NativeObject* object = ConvertNativeValueTo<NativeObject>(objValue);
    object->SetProperty("bundleName", CreateJsValue(engine, abilityStateData.bundleName));
    object->SetProperty("abilityName", CreateJsValue(engine, abilityStateData.abilityName));
    object->SetProperty("pid", CreateJsValue(engine, abilityStateData.pid));
    object->SetProperty("uid", CreateJsValue(engine, abilityStateData.uid));
    object->SetProperty("state", CreateJsValue(engine, abilityStateData.abilityState));
    return objValue;
}

NativeValue* CreateJsProcessData(NativeEngine &engine, const ProcessData &processData)
{
    HILOG_INFO("%{public}s called.", __func__); 
    NativeValue* objValue = engine.CreateObject();
    NativeObject* object = ConvertNativeValueTo<NativeObject>(objValue);
    object->SetProperty("bundleName", CreateJsValue(engine, processData.bundleName));
    object->SetProperty("pid", CreateJsValue(engine, processData.pid));
    object->SetProperty("uid", CreateJsValue(engine, processData.uid));
    return objValue;
}

NativeValue* CreateJsAppStateDataArray(NativeEngine &engine, std::vector<AppStateData> &appStateDatas)
{
    NativeValue* arrayValue = engine.CreateArray(appStateDatas.size());
    NativeArray* array = ConvertNativeValueTo<NativeArray>(arrayValue);
    uint32_t index = 0;
    for (const auto &appStateData : appStateDatas) {
        array->SetElement(index++, CreateJsAppStateData(engine, appStateData));
    }
    return arrayValue;
}

NativeValue* CreateJsProcessRunningInfoArray(NativeEngine &engine, const std::vector<RunningProcessInfo> &infos)
{
    NativeValue* arrayValue = engine.CreateArray(infos.size());
    NativeArray* array = ConvertNativeValueTo<NativeArray>(arrayValue);
    uint32_t index = 0;
    for (const auto &runningInfo : infos) {
        array->SetElement(index++, CreateJsProcessRunningInfo(engine, runningInfo));
    }
    return arrayValue;
}

NativeValue* CreateJsProcessRunningInfo(NativeEngine &engine, const RunningProcessInfo &info)
{
    NativeValue* objValue = engine.CreateObject();
    NativeObject* object = ConvertNativeValueTo<NativeObject>(objValue);

    object->SetProperty("processName", CreateJsValue(engine, info.processName_));
    object->SetProperty("pid", CreateJsValue(engine, info.pid_));
    object->SetProperty("uid", CreateJsValue(engine, info.uid_));
    object->SetProperty("bundleNames", CreateNativeArray(engine, info.bundleNames));
    return objValue;
}
}  // namespace AbilityRuntime
}  // namespace OHOS
