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

#include "js_hap_module_info_utils.h"

#include "js_data_struct_converter.h"
#include "js_runtime_utils.h"

namespace OHOS {
namespace AbilityRuntime {
NativeValue* CreateJsHapModuleInfo(NativeEngine& engine, AppExecFwk::HapModuleInfo& hapModuleInfo)
{
    NativeValue* objValue = engine.CreateObject();
    NativeObject* object = ConvertNativeValueTo<NativeObject>(objValue);

    object->SetProperty("name", CreateJsValue(engine, hapModuleInfo.name));
    object->SetProperty("moduleName", CreateJsValue(engine, hapModuleInfo.moduleName));
    object->SetProperty("description", CreateJsValue(engine, hapModuleInfo.description));
    object->SetProperty("icon", CreateJsValue(engine, hapModuleInfo.iconPath));
    object->SetProperty("label", CreateJsValue(engine, hapModuleInfo.label));
    object->SetProperty("backgroundImg", CreateJsValue(engine, hapModuleInfo.backgroundImg));
    object->SetProperty("mainAbilityName", CreateJsValue(engine, hapModuleInfo.mainAbility));
    // object->SetProperty("srcPath", CreateJsValue(engine, hapModuleInfo.srcPath)); // d.ts no have
    object->SetProperty("supportedModes", CreateJsValue(engine, hapModuleInfo.supportedModes));

    // At present, the C++ data structure does not support the following structures, so pass in empty
    NativeValue* nullValue = nullptr;
    object->SetProperty("descriptionId", nullValue);
    object->SetProperty("labelId", nullValue);
    object->SetProperty("iconId", nullValue);
    object->SetProperty("installationFree", engine.CreateBoolean(false));

    NativeValue *capArrayValue = engine.CreateArray(hapModuleInfo.reqCapabilities.size());
    NativeArray *capArray = ConvertNativeValueTo<NativeArray>(capArrayValue);
    if (capArray != nullptr) {
        int index = 0;
        for (auto cap : hapModuleInfo.reqCapabilities) {
            capArray->SetElement(index++, CreateJsValue(engine, cap));
        }
    }
    object->SetProperty("reqCapabilities", capArrayValue);

    NativeValue *deviceArrayValue = engine.CreateArray(hapModuleInfo.deviceTypes.size());
    NativeArray *deviceArray = ConvertNativeValueTo<NativeArray>(deviceArrayValue);
    if (deviceArray != nullptr) {
        int index = 0;
        for (auto device : hapModuleInfo.deviceTypes) {
            deviceArray->SetElement(index++, CreateJsValue(engine, device));
        }
    }
    object->SetProperty("deviceTypes", deviceArrayValue);

    NativeValue *abilityArrayValue = engine.CreateArray(hapModuleInfo.abilityInfos.size());
    NativeArray *abilityArray = ConvertNativeValueTo<NativeArray>(abilityArrayValue);
    if (abilityArray != nullptr) {
        int index = 0;
        for (auto abilityInfo : hapModuleInfo.abilityInfos) {
            abilityArray->SetElement(index++, CreateJsAbilityInfo(engine, abilityInfo));
        }
    }
    object->SetProperty("abilityInfo", abilityArrayValue);

    return objValue;
}
}  // namespace AbilityRuntime
}  // namespace OHOS
