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

#include "hap_module_info.h"

#include "nlohmann/json.hpp"
#include "string_ex.h"

#include "json_util.h"
#include "parcel_macro.h"

namespace OHOS {
namespace AppExecFwk {
namespace {
const std::string HAP_MODULE_INFO_NAME = "name";
const std::string HAP_MODULE_INFO_MODULE_NAME = "moduleName";
const std::string HAP_MODULE_INFO_DESCRIPTION = "description";
const std::string HAP_MODULE_INFO_ICON_PATH = "iconPath";
const std::string HAP_MODULE_INFO_LABEL = "label";
const std::string HAP_MODULE_INFO_BACKGROUND_IMG = "backgroundImg";
const std::string HAP_MODULE_INFO_MAIN_ABILITY = "mainAbility";
const std::string HAP_MODULE_INFO_SRC_PATH = "srcPath";
const std::string HAP_MODULE_INFO_SUPPORTED_MODES = "supportedModes";
const std::string HAP_MODULE_INFO_BUNDLE_NAME = "bundleName";
const std::string HAP_MODULE_INFO_SRC_ENTRANCE = "srcEntrance";
const std::string HAP_MODULE_INFO_MAIN_ELEMENTNAME = "mainElementName";
const std::string HAP_MODULE_INFO_REQ_CAPABILITIES = "reqCapabilities";
const std::string HAP_MODULE_INFO_DEVICE_TYPES = "deviceTypes";
const std::string HAP_MODULE_INFO_COLOR_MODE = "colorMode";
const std::string HAP_MODULE_INFO_IS_MODULE_JSON = "isModuleJsonHap";
const std::string HAP_MODULE_INFO_IS_STAGE_BASED_MODEL = "isStageBasedModelHap";
}

bool HapModuleInfo::ReadFromParcel(Parcel &parcel)
{
    name = Str16ToStr8(parcel.ReadString16());
    moduleName = Str16ToStr8(parcel.ReadString16());
    description = Str16ToStr8(parcel.ReadString16());
    iconPath = Str16ToStr8(parcel.ReadString16());
    label = Str16ToStr8(parcel.ReadString16());
    backgroundImg = Str16ToStr8(parcel.ReadString16());
    mainAbility = Str16ToStr8(parcel.ReadString16());
    srcPath = Str16ToStr8(parcel.ReadString16());
    supportedModes = parcel.ReadInt32();
    bundleName = Str16ToStr8(parcel.ReadString16());
    srcEntrance = Str16ToStr8(parcel.ReadString16());
    mainElementName = Str16ToStr8(parcel.ReadString16());
    int32_t extensionAbilityInfoSize = parcel.ReadInt32();
    for (int32_t i = 0; i < extensionAbilityInfoSize; ++i) {
        std::unique_ptr<ExtensionAbilityInfo> extensionAbilityInfo(parcel.ReadParcelable<ExtensionAbilityInfo>());
        if (!extensionAbilityInfo) {
            APP_LOGE("ReadParcelable<ExtensionAbilityInfo> failed");
            return false;
        }
        extensionInfos.emplace_back(*extensionAbilityInfo);
    }
    int32_t metadataSize = parcel.ReadInt32();
    APP_LOGI("Read metadataSize is %{public}d", metadataSize);
    for (int32_t i = 0; i < metadataSize; ++i) {
        std::unique_ptr<Metadata> meta(parcel.ReadParcelable<Metadata>());
        if (!meta) {
            APP_LOGE("ReadParcelable<Metadata> failed");
            return false;
        }
        metadata.emplace_back(*meta);
    }
    resourcePath = Str16ToStr8(parcel.ReadString16());
    isModuleJson = parcel.ReadBool();
    isStageBasedModel = parcel.ReadBool();
    int32_t reqCapabilitiesSize;
    READ_PARCEL_AND_RETURN_FALSE_IF_FAIL(Int32, parcel, reqCapabilitiesSize);
    for (int32_t i = 0; i < reqCapabilitiesSize; i++) {
        reqCapabilities.emplace_back(Str16ToStr8(parcel.ReadString16()));
    }

    int32_t deviceTypesSize;
    READ_PARCEL_AND_RETURN_FALSE_IF_FAIL(Int32, parcel, deviceTypesSize);
    for (int32_t i = 0; i < deviceTypesSize; i++) {
        deviceTypes.emplace_back(Str16ToStr8(parcel.ReadString16()));
    }

    int32_t abilityInfosSize = parcel.ReadInt32();
    for (int32_t i = 0; i < abilityInfosSize; i++) {
        std::unique_ptr<AbilityInfo> abilityInfo(parcel.ReadParcelable<AbilityInfo>());
        if (!abilityInfo) {
            APP_LOGE("ReadParcelable<AbilityInfo> failed");
            return false;
        }
        abilityInfos.emplace_back(*abilityInfo);
    }

    int32_t colorModeData;
    READ_PARCEL_AND_RETURN_FALSE_IF_FAIL(Int32, parcel, colorModeData);
    colorMode = static_cast<ModuleColorMode>(colorModeData);
    return true;
}

HapModuleInfo *HapModuleInfo::Unmarshalling(Parcel &parcel)
{
    HapModuleInfo *info = new (std::nothrow) HapModuleInfo();
    if (info && !info->ReadFromParcel(parcel)) {
        APP_LOGW("read from parcel failed");
        delete info;
        info = nullptr;
    }
    return info;
}

bool HapModuleInfo::Marshalling(Parcel &parcel) const
{
    WRITE_PARCEL_AND_RETURN_FALSE_IF_FAIL(String16, parcel, Str8ToStr16(name));
    WRITE_PARCEL_AND_RETURN_FALSE_IF_FAIL(String16, parcel, Str8ToStr16(moduleName));
    WRITE_PARCEL_AND_RETURN_FALSE_IF_FAIL(String16, parcel, Str8ToStr16(description));
    WRITE_PARCEL_AND_RETURN_FALSE_IF_FAIL(String16, parcel, Str8ToStr16(iconPath));
    WRITE_PARCEL_AND_RETURN_FALSE_IF_FAIL(String16, parcel, Str8ToStr16(label));
    WRITE_PARCEL_AND_RETURN_FALSE_IF_FAIL(String16, parcel, Str8ToStr16(backgroundImg));
    WRITE_PARCEL_AND_RETURN_FALSE_IF_FAIL(String16, parcel, Str8ToStr16(mainAbility));
    WRITE_PARCEL_AND_RETURN_FALSE_IF_FAIL(String16, parcel, Str8ToStr16(srcPath));
    WRITE_PARCEL_AND_RETURN_FALSE_IF_FAIL(Int32, parcel, supportedModes);
    WRITE_PARCEL_AND_RETURN_FALSE_IF_FAIL(String16, parcel, Str8ToStr16(bundleName));
    WRITE_PARCEL_AND_RETURN_FALSE_IF_FAIL(String16, parcel, Str8ToStr16(srcEntrance));
    WRITE_PARCEL_AND_RETURN_FALSE_IF_FAIL(String16, parcel, Str8ToStr16(mainElementName));
    WRITE_PARCEL_AND_RETURN_FALSE_IF_FAIL(Int32, parcel, extensionInfos.size());
    for (auto &extensionInfo : extensionInfos) {
        WRITE_PARCEL_AND_RETURN_FALSE_IF_FAIL(Parcelable, parcel, &extensionInfo);
    }
    WRITE_PARCEL_AND_RETURN_FALSE_IF_FAIL(Int32, parcel, metadata.size());
    for (const auto &meta : metadata) {
        WRITE_PARCEL_AND_RETURN_FALSE_IF_FAIL(Parcelable, parcel, &meta);
    }
    WRITE_PARCEL_AND_RETURN_FALSE_IF_FAIL(String16, parcel, Str8ToStr16(resourcePath));
    WRITE_PARCEL_AND_RETURN_FALSE_IF_FAIL(Bool, parcel, isModuleJson);
    WRITE_PARCEL_AND_RETURN_FALSE_IF_FAIL(Bool, parcel, isStageBasedModel);

    WRITE_PARCEL_AND_RETURN_FALSE_IF_FAIL(Int32, parcel, reqCapabilities.size());
    for (auto &reqCapability : reqCapabilities) {
        WRITE_PARCEL_AND_RETURN_FALSE_IF_FAIL(String16, parcel, Str8ToStr16(reqCapability));
    }
    WRITE_PARCEL_AND_RETURN_FALSE_IF_FAIL(Int32, parcel, deviceTypes.size());
    for (auto &deviceType : deviceTypes) {
        WRITE_PARCEL_AND_RETURN_FALSE_IF_FAIL(String16, parcel, Str8ToStr16(deviceType));
    }
    WRITE_PARCEL_AND_RETURN_FALSE_IF_FAIL(Int32, parcel, abilityInfos.size());
    for (auto &abilityInfo : abilityInfos) {
        WRITE_PARCEL_AND_RETURN_FALSE_IF_FAIL(Parcelable, parcel, &abilityInfo);
    }
    WRITE_PARCEL_AND_RETURN_FALSE_IF_FAIL(Int32, parcel, static_cast<int32_t>(colorMode));
    return true;
}

void to_json(nlohmann::json &jsonObject, const HapModuleInfo &hapModuleInfo)
{
    jsonObject = nlohmann::json {
        {HAP_MODULE_INFO_NAME, hapModuleInfo.name},
        {HAP_MODULE_INFO_MODULE_NAME, hapModuleInfo.moduleName},
        {HAP_MODULE_INFO_DESCRIPTION, hapModuleInfo.description},
        {HAP_MODULE_INFO_ICON_PATH, hapModuleInfo.iconPath},
        {HAP_MODULE_INFO_LABEL, hapModuleInfo.label},
        {HAP_MODULE_INFO_BACKGROUND_IMG, hapModuleInfo.backgroundImg},
        {HAP_MODULE_INFO_MAIN_ABILITY, hapModuleInfo.mainAbility},
        {HAP_MODULE_INFO_SRC_PATH, hapModuleInfo.srcPath},
        {HAP_MODULE_INFO_SUPPORTED_MODES, hapModuleInfo.supportedModes},
        {HAP_MODULE_INFO_BUNDLE_NAME, hapModuleInfo.bundleName},
        {HAP_MODULE_INFO_SRC_ENTRANCE, hapModuleInfo.srcEntrance},
        {HAP_MODULE_INFO_MAIN_ELEMENTNAME, hapModuleInfo.mainElementName},
        {HAP_MODULE_INFO_REQ_CAPABILITIES, hapModuleInfo.reqCapabilities},
        {HAP_MODULE_INFO_DEVICE_TYPES, hapModuleInfo.deviceTypes},
        {HAP_MODULE_INFO_COLOR_MODE, hapModuleInfo.colorMode},
        {HAP_MODULE_INFO_IS_MODULE_JSON, hapModuleInfo.isModuleJson},
        {HAP_MODULE_INFO_IS_STAGE_BASED_MODEL, hapModuleInfo.isStageBasedModel},
    };
}

void from_json(const nlohmann::json &jsonObject, HapModuleInfo &hapModuleInfo)
{
    const auto &jsonObjectEnd = jsonObject.end();
    int32_t parseResult = ERR_OK;
    GetValueIfFindKey<std::string>(jsonObject,
        jsonObjectEnd,
        HAP_MODULE_INFO_NAME,
        hapModuleInfo.name,
        JsonType::STRING,
        false,
        parseResult,
        ArrayType::NOT_ARRAY);
    GetValueIfFindKey<std::string>(jsonObject,
        jsonObjectEnd,
        HAP_MODULE_INFO_MODULE_NAME,
        hapModuleInfo.moduleName,
        JsonType::STRING,
        false,
        parseResult,
        ArrayType::NOT_ARRAY);
    GetValueIfFindKey<std::string>(jsonObject,
        jsonObjectEnd,
        HAP_MODULE_INFO_DESCRIPTION,
        hapModuleInfo.description,
        JsonType::STRING,
        false,
        parseResult,
        ArrayType::NOT_ARRAY);
    GetValueIfFindKey<std::string>(jsonObject,
        jsonObjectEnd,
        HAP_MODULE_INFO_ICON_PATH,
        hapModuleInfo.iconPath,
        JsonType::STRING,
        false,
        parseResult,
        ArrayType::NOT_ARRAY);
    GetValueIfFindKey<std::string>(jsonObject,
        jsonObjectEnd,
        HAP_MODULE_INFO_LABEL,
        hapModuleInfo.label,
        JsonType::STRING,
        false,
        parseResult,
        ArrayType::NOT_ARRAY);
    GetValueIfFindKey<std::string>(jsonObject,
        jsonObjectEnd,
        HAP_MODULE_INFO_BACKGROUND_IMG,
        hapModuleInfo.backgroundImg,
        JsonType::STRING,
        false,
        parseResult,
        ArrayType::NOT_ARRAY);
    GetValueIfFindKey<std::string>(jsonObject,
        jsonObjectEnd,
        HAP_MODULE_INFO_MAIN_ABILITY,
        hapModuleInfo.mainAbility,
        JsonType::STRING,
        false,
        parseResult,
        ArrayType::NOT_ARRAY);
    GetValueIfFindKey<std::string>(jsonObject,
        jsonObjectEnd,
        HAP_MODULE_INFO_SRC_PATH,
        hapModuleInfo.srcPath,
        JsonType::STRING,
        false,
        parseResult,
        ArrayType::NOT_ARRAY);
    GetValueIfFindKey<int>(jsonObject,
        jsonObjectEnd,
        HAP_MODULE_INFO_SUPPORTED_MODES,
        hapModuleInfo.supportedModes,
        JsonType::NUMBER,
        false,
        parseResult,
        ArrayType::NOT_ARRAY);
    GetValueIfFindKey<std::string>(jsonObject,
        jsonObjectEnd,
        HAP_MODULE_INFO_BUNDLE_NAME,
        hapModuleInfo.bundleName,
        JsonType::STRING,
        false,
        parseResult,
        ArrayType::NOT_ARRAY);
    GetValueIfFindKey<std::string>(jsonObject,
        jsonObjectEnd,
        HAP_MODULE_INFO_SRC_ENTRANCE,
        hapModuleInfo.srcEntrance,
        JsonType::STRING,
        false,
        parseResult,
        ArrayType::NOT_ARRAY);
    GetValueIfFindKey<std::string>(jsonObject,
        jsonObjectEnd,
        HAP_MODULE_INFO_MAIN_ELEMENTNAME,
        hapModuleInfo.mainElementName,
        JsonType::STRING,
        false,
        parseResult,
        ArrayType::NOT_ARRAY);
    GetValueIfFindKey<std::vector<std::string>>(jsonObject,
        jsonObjectEnd,
        HAP_MODULE_INFO_REQ_CAPABILITIES,
        hapModuleInfo.reqCapabilities,
        JsonType::ARRAY,
        false,
        parseResult,
        ArrayType::STRING);
    GetValueIfFindKey<std::vector<std::string>>(jsonObject,
        jsonObjectEnd,
        HAP_MODULE_INFO_DEVICE_TYPES,
        hapModuleInfo.deviceTypes,
        JsonType::ARRAY,
        false,
        parseResult,
        ArrayType::STRING);
    GetValueIfFindKey<ModuleColorMode>(jsonObject,
        jsonObjectEnd,
        HAP_MODULE_INFO_COLOR_MODE,
        hapModuleInfo.colorMode,
        JsonType::NUMBER,
        false,
        parseResult,
        ArrayType::NOT_ARRAY);
    GetValueIfFindKey<bool>(jsonObject,
        jsonObjectEnd,
        HAP_MODULE_INFO_IS_MODULE_JSON,
        hapModuleInfo.isModuleJson,
        JsonType::BOOLEAN,
        false,
        parseResult,
        ArrayType::NOT_ARRAY);
    GetValueIfFindKey<bool>(jsonObject,
        jsonObjectEnd,
        HAP_MODULE_INFO_IS_STAGE_BASED_MODEL,
        hapModuleInfo.isStageBasedModel,
        JsonType::BOOLEAN,
        false,
        parseResult,
        ArrayType::NOT_ARRAY);
    if (parseResult != ERR_OK) {
        APP_LOGE("read hapModuleInfo from database error, error code : %{public}d", parseResult);
    }
}
}  // namespace AppExecFwk
}  // namespace OHOS
