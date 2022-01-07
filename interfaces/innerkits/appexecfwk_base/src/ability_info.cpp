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

#include "ability_info.h"

#include <errno.h>
#include <fcntl.h>
#include <string.h>
#include <unistd.h>

#include "bundle_constants.h"
#include "json_util.h"
#include "nlohmann/json.hpp"
#include "parcel_macro.h"
#include "string_ex.h"

namespace OHOS {
namespace AppExecFwk {
namespace {
const std::string JSON_KEY_PACKAGE = "package";
const std::string JSON_KEY_NAME = "name";
const std::string JSON_KEY_BUNDLE_NAME = "bundleName";
const std::string JSON_KEY_APPLICATION_NAME = "applicationName";
const std::string JSON_KEY_LABEL = "label";
const std::string JSON_KEY_DESCRIPTION = "description";
const std::string JSON_KEY_ICON_PATH = "iconPath";
const std::string JSON_KEY_THEME = "theme";
const std::string JSON_KEY_VISIBLE = "visible";
const std::string JSON_KEY_KIND = "kind";
const std::string JSON_KEY_TYPE = "type";
const std::string JSON_KEY_ORIENTATION = "orientation";
const std::string JSON_KEY_LAUNCH_MODE = "launchMode";
const std::string JSON_KEY_CODE_PATH = "codePath";
const std::string JSON_KEY_RESOURCE_PATH = "resourcePath";
const std::string JSON_KEY_LIB_PATH = "libPath";
const std::string JSON_KEY_PERMISSIONS = "permissions";
const std::string JSON_KEY_PROCESS = "process";
const std::string JSON_KEY_DEVICE_TYPES = "deviceTypes";
const std::string JSON_KEY_DEVICE_CAPABILITIES = "deviceCapabilities";
const std::string JSON_KEY_URI = "uri";
const std::string JSON_KEY_MODULE_NAME = "moduleName";
const std::string JSON_KEY_DEVICE_ID = "deviceId";
const std::string JSON_KEY_IS_LAUNCHER_ABILITY = "isLauncherAbility";
const std::string JSON_KEY_IS_NATIVE_ABILITY = "isNativeAbility";
const std::string JSON_KEY_ENABLED = "enabled";
const std::string JSON_KEY_SUPPORT_PIP_MODE = "supportPipMode";
const std::string JSON_KEY_TARGET_ABILITY = "targetAbility";
const std::string JSON_KEY_READ_PERMISSION = "readPermission";
const std::string JSON_KEY_WRITE_PERMISSION = "writePermission";
const std::string JSON_KEY_CONFIG_CHANGES = "configChanges";
const std::string JSON_KEY_FORM = "form";
const std::string JSON_KEY_FORM_ENTITY = "formEntity";
const std::string JSON_KEY_MIN_FORM_HEIGHT = "minFormHeight";
const std::string JSON_KEY_DEFAULT_FORM_HEIGHT = "defaultFormHeight";
const std::string JSON_KEY_MIN_FORM_WIDTH = "minFormWidth";
const std::string JSON_KEY_DEFAULT_FORM_WIDTH = "defaultFormWidth";
const std::string JSON_KEY_BACKGROUND_MODES = "backgroundModes";
const std::string JSON_KEY_CUSTOMIZE_DATA = "customizeData";
const std::string JSON_KEY_META_DATA = "metaData";
const std::string JSON_KEY_META_VALUE = "value";
const std::string JSON_KEY_META_EXTRA = "extra";
const std::string JSON_KEY_LABEL_ID = "labelId";
const std::string JSON_KEY_DESCRIPTION_ID = "descriptionId";
const std::string JSON_KEY_ICON_ID = "iconId";
const std::string JSON_KEY_FORM_ENABLED = "formEnabled";
const std::string JSON_KEY_SRC_PATH = "srcPath";
const std::string JSON_KEY_SRC_LANGUAGE = "srcLanguage";
const std::string META_DATA = "metadata";
const std::string META_DATA_NAME = "name";
const std::string META_DATA_VALUE = "value";
const std::string META_DATA_RESOURCE = "resource";
const std::string SRC_ENTRANCE = "srcEntrance";
}  // namespace

bool AbilityInfo::ReadFromParcel(Parcel &parcel)
{
    name = Str16ToStr8(parcel.ReadString16());
    label = Str16ToStr8(parcel.ReadString16());
    description = Str16ToStr8(parcel.ReadString16());
    iconPath = Str16ToStr8(parcel.ReadString16());
    theme = Str16ToStr8(parcel.ReadString16());
    kind = Str16ToStr8(parcel.ReadString16());
    uri = Str16ToStr8(parcel.ReadString16());
    package = Str16ToStr8(parcel.ReadString16());
    bundleName = Str16ToStr8(parcel.ReadString16());
    moduleName = Str16ToStr8(parcel.ReadString16());
    applicationName = Str16ToStr8(parcel.ReadString16());
    process = Str16ToStr8(parcel.ReadString16());
    deviceId = Str16ToStr8(parcel.ReadString16());
    codePath = Str16ToStr8(parcel.ReadString16());
    resourcePath = Str16ToStr8(parcel.ReadString16());
    libPath = Str16ToStr8(parcel.ReadString16());
    targetAbility = Str16ToStr8(parcel.ReadString16());
    readPermission = Str16ToStr8(parcel.ReadString16());
    writePermission = Str16ToStr8(parcel.ReadString16());
    srcPath = Str16ToStr8(parcel.ReadString16());
    srcLanguage = Str16ToStr8(parcel.ReadString16());
    visible = parcel.ReadBool();
    isLauncherAbility = parcel.ReadBool();
    isNativeAbility = parcel.ReadBool();
    enabled = parcel.ReadBool();
    supportPipMode = parcel.ReadBool();
    formEnabled = parcel.ReadBool();
    int32_t configChangesSize;
    READ_PARCEL_AND_RETURN_FALSE_IF_FAIL(Int32, parcel, configChangesSize);
    for (int32_t i = 0; i < configChangesSize; i++) {
        configChanges.emplace_back(Str16ToStr8(parcel.ReadString16()));
    }
    formEntity = parcel.ReadInt32();
    minFormHeight = parcel.ReadInt32();
    defaultFormHeight = parcel.ReadInt32();
    minFormWidth = parcel.ReadInt32();
    defaultFormWidth = parcel.ReadInt32();
    backgroundModes = parcel.ReadInt32();
    labelId = parcel.ReadInt32();
    descriptionId = parcel.ReadInt32();
    iconId = parcel.ReadInt32();

    int32_t typeData;
    READ_PARCEL_AND_RETURN_FALSE_IF_FAIL(Int32, parcel, typeData);
    type = static_cast<AbilityType>(typeData);

    int32_t orientationData;
    READ_PARCEL_AND_RETURN_FALSE_IF_FAIL(Int32, parcel, orientationData);
    orientation = static_cast<DisplayOrientation>(orientationData);

    int32_t launchModeData;
    READ_PARCEL_AND_RETURN_FALSE_IF_FAIL(Int32, parcel, launchModeData);
    launchMode = static_cast<LaunchMode>(launchModeData);

    int32_t permissionsSize;
    READ_PARCEL_AND_RETURN_FALSE_IF_FAIL(Int32, parcel, permissionsSize);
    for (int32_t i = 0; i < permissionsSize; i++) {
        permissions.emplace_back(Str16ToStr8(parcel.ReadString16()));
    }

    int32_t deviceTypesSize;
    READ_PARCEL_AND_RETURN_FALSE_IF_FAIL(Int32, parcel, deviceTypesSize);
    for (int32_t i = 0; i < deviceTypesSize; i++) {
        deviceTypes.emplace_back(Str16ToStr8(parcel.ReadString16()));
    }

    int32_t deviceCapabilitiesSize;
    READ_PARCEL_AND_RETURN_FALSE_IF_FAIL(Int32, parcel, deviceCapabilitiesSize);
    for (int32_t i = 0; i < deviceCapabilitiesSize; i++) {
        deviceCapabilities.emplace_back(Str16ToStr8(parcel.ReadString16()));
    }

    int32_t customizeDataSize;
    READ_PARCEL_AND_RETURN_FALSE_IF_FAIL(Int32, parcel, customizeDataSize);
    for (auto i = 0; i < customizeDataSize; i++) {
        std::unique_ptr<CustomizeData> customizeData(parcel.ReadParcelable<CustomizeData>());
        if (!customizeData) {
            APP_LOGE("ReadParcelable<CustomizeData> failed");
            return false;
        }
        metaData.customizeData.emplace_back(*customizeData);
    }

    std::unique_ptr<ApplicationInfo> appInfo(parcel.ReadParcelable<ApplicationInfo>());
    if (!appInfo) {
        APP_LOGE("ReadParcelable<ApplicationInfo> failed");
        return false;
    }
    applicationInfo = *appInfo;
    return true;
}

AbilityInfo *AbilityInfo::Unmarshalling(Parcel &parcel)
{
    AbilityInfo *info = new (std::nothrow) AbilityInfo();
    if (info && !info->ReadFromParcel(parcel)) {
        APP_LOGW("read from parcel failed");
        delete info;
        info = nullptr;
    }
    return info;
}

bool AbilityInfo::Marshalling(Parcel &parcel) const
{
    WRITE_PARCEL_AND_RETURN_FALSE_IF_FAIL(String16, parcel, Str8ToStr16(name));
    WRITE_PARCEL_AND_RETURN_FALSE_IF_FAIL(String16, parcel, Str8ToStr16(label));
    WRITE_PARCEL_AND_RETURN_FALSE_IF_FAIL(String16, parcel, Str8ToStr16(description));
    WRITE_PARCEL_AND_RETURN_FALSE_IF_FAIL(String16, parcel, Str8ToStr16(iconPath));
    WRITE_PARCEL_AND_RETURN_FALSE_IF_FAIL(String16, parcel, Str8ToStr16(theme));
    WRITE_PARCEL_AND_RETURN_FALSE_IF_FAIL(String16, parcel, Str8ToStr16(kind));
    WRITE_PARCEL_AND_RETURN_FALSE_IF_FAIL(String16, parcel, Str8ToStr16(uri));
    WRITE_PARCEL_AND_RETURN_FALSE_IF_FAIL(String16, parcel, Str8ToStr16(package));
    WRITE_PARCEL_AND_RETURN_FALSE_IF_FAIL(String16, parcel, Str8ToStr16(bundleName));
    WRITE_PARCEL_AND_RETURN_FALSE_IF_FAIL(String16, parcel, Str8ToStr16(moduleName));
    WRITE_PARCEL_AND_RETURN_FALSE_IF_FAIL(String16, parcel, Str8ToStr16(applicationName));
    WRITE_PARCEL_AND_RETURN_FALSE_IF_FAIL(String16, parcel, Str8ToStr16(process));
    WRITE_PARCEL_AND_RETURN_FALSE_IF_FAIL(String16, parcel, Str8ToStr16(deviceId));
    WRITE_PARCEL_AND_RETURN_FALSE_IF_FAIL(String16, parcel, Str8ToStr16(codePath));
    WRITE_PARCEL_AND_RETURN_FALSE_IF_FAIL(String16, parcel, Str8ToStr16(resourcePath));
    WRITE_PARCEL_AND_RETURN_FALSE_IF_FAIL(String16, parcel, Str8ToStr16(libPath));
    WRITE_PARCEL_AND_RETURN_FALSE_IF_FAIL(String16, parcel, Str8ToStr16(targetAbility));
    WRITE_PARCEL_AND_RETURN_FALSE_IF_FAIL(String16, parcel, Str8ToStr16(readPermission));
    WRITE_PARCEL_AND_RETURN_FALSE_IF_FAIL(String16, parcel, Str8ToStr16(writePermission));
    WRITE_PARCEL_AND_RETURN_FALSE_IF_FAIL(String16, parcel, Str8ToStr16(srcPath));
    WRITE_PARCEL_AND_RETURN_FALSE_IF_FAIL(String16, parcel, Str8ToStr16(srcLanguage));
    WRITE_PARCEL_AND_RETURN_FALSE_IF_FAIL(Bool, parcel, visible);
    WRITE_PARCEL_AND_RETURN_FALSE_IF_FAIL(Bool, parcel, isLauncherAbility);
    WRITE_PARCEL_AND_RETURN_FALSE_IF_FAIL(Bool, parcel, isNativeAbility);
    WRITE_PARCEL_AND_RETURN_FALSE_IF_FAIL(Bool, parcel, enabled);
    WRITE_PARCEL_AND_RETURN_FALSE_IF_FAIL(Bool, parcel, supportPipMode);
    WRITE_PARCEL_AND_RETURN_FALSE_IF_FAIL(Bool, parcel, formEnabled);
    WRITE_PARCEL_AND_RETURN_FALSE_IF_FAIL(Int32, parcel, configChanges.size());
    for (auto &configChange : configChanges) {
        WRITE_PARCEL_AND_RETURN_FALSE_IF_FAIL(String16, parcel, Str8ToStr16(configChange));
    }
    WRITE_PARCEL_AND_RETURN_FALSE_IF_FAIL(Int32, parcel, formEntity);
    WRITE_PARCEL_AND_RETURN_FALSE_IF_FAIL(Int32, parcel, minFormHeight);
    WRITE_PARCEL_AND_RETURN_FALSE_IF_FAIL(Int32, parcel, defaultFormHeight);
    WRITE_PARCEL_AND_RETURN_FALSE_IF_FAIL(Int32, parcel, minFormWidth);
    WRITE_PARCEL_AND_RETURN_FALSE_IF_FAIL(Int32, parcel, defaultFormWidth);
    WRITE_PARCEL_AND_RETURN_FALSE_IF_FAIL(Int32, parcel, backgroundModes);
    WRITE_PARCEL_AND_RETURN_FALSE_IF_FAIL(Int32, parcel, labelId);
    WRITE_PARCEL_AND_RETURN_FALSE_IF_FAIL(Int32, parcel, descriptionId);
    WRITE_PARCEL_AND_RETURN_FALSE_IF_FAIL(Int32, parcel, iconId);
    WRITE_PARCEL_AND_RETURN_FALSE_IF_FAIL(Int32, parcel, static_cast<int32_t>(type));
    WRITE_PARCEL_AND_RETURN_FALSE_IF_FAIL(Int32, parcel, static_cast<int32_t>(orientation));
    WRITE_PARCEL_AND_RETURN_FALSE_IF_FAIL(Int32, parcel, static_cast<int32_t>(launchMode));

    WRITE_PARCEL_AND_RETURN_FALSE_IF_FAIL(Int32, parcel, permissions.size());
    for (auto &permission : permissions) {
        WRITE_PARCEL_AND_RETURN_FALSE_IF_FAIL(String16, parcel, Str8ToStr16(permission));
    }

    WRITE_PARCEL_AND_RETURN_FALSE_IF_FAIL(Int32, parcel, deviceTypes.size());
    for (auto &deviceType : deviceTypes) {
        WRITE_PARCEL_AND_RETURN_FALSE_IF_FAIL(String16, parcel, Str8ToStr16(deviceType));
    }

    WRITE_PARCEL_AND_RETURN_FALSE_IF_FAIL(Int32, parcel, deviceCapabilities.size());
    for (auto &deviceCapability : deviceCapabilities) {
        WRITE_PARCEL_AND_RETURN_FALSE_IF_FAIL(String16, parcel, Str8ToStr16(deviceCapability));
    }

    const auto customizeDataSize = static_cast<int32_t>(metaData.customizeData.size());
    WRITE_PARCEL_AND_RETURN_FALSE_IF_FAIL(Int32, parcel, customizeDataSize);
    for (auto i = 0; i < customizeDataSize; i++) {
        WRITE_PARCEL_AND_RETURN_FALSE_IF_FAIL(Parcelable, parcel, &metaData.customizeData[i]);
    }

    WRITE_PARCEL_AND_RETURN_FALSE_IF_FAIL(Parcelable, parcel, &applicationInfo);
    return true;
}

void AbilityInfo::Dump(std::string prefix, int fd)
{
    APP_LOGI("called dump Abilityinfo");
    if (fd < 0) {
        APP_LOGE("dump Abilityinfo fd error");
        return;
    }
    int flags = fcntl(fd, F_GETFL);
    if (flags < 0) {
        APP_LOGE("dump Abilityinfo fcntl error %{public}s", strerror(errno));
        return;
    }
    uint uflags = static_cast<uint>(flags);
    uflags &= O_ACCMODE;
    if ((uflags == O_WRONLY) || (uflags == O_RDWR)) {
        nlohmann::json jsonObject = *this;
        std::string result;
        result.append(prefix);
        result.append(jsonObject.dump(Constants::DUMP_INDENT));
        int ret = TEMP_FAILURE_RETRY(write(fd, result.c_str(), result.size()));
        if (ret < 0) {
            APP_LOGE("dump Abilityinfo write error %{public}s", strerror(errno));
        }
    }
    return;
}

void to_json(nlohmann::json &jsonObject, const CustomizeData &customizeData)
{
    jsonObject = nlohmann::json {
        {JSON_KEY_NAME, customizeData.name},
        {JSON_KEY_META_VALUE, customizeData.value},
        {JSON_KEY_META_EXTRA, customizeData.extra}
    };
}

void to_json(nlohmann::json &jsonObject, const MetaData &metaData)
{
    jsonObject = nlohmann::json {
        {JSON_KEY_CUSTOMIZE_DATA, metaData.customizeData}
    };
}

void to_json(nlohmann::json &jsonObject, const Metadata &metadata)
{
    jsonObject = nlohmann::json {
        {META_DATA_NAME, metadata.name},
        {META_DATA_VALUE, metadata.value},
        {META_DATA_RESOURCE, metadata.resource}
    };
}

void to_json(nlohmann::json &jsonObject, const Metadata &metadata)
{
    jsonObject = nlohmann::json {
        {META_DATA_NAME, metadata.name},
        {META_DATA_VALUE, metadata.value},
        {META_DATA_RESOURCE, metadata.resource}
    };
}

void to_json(nlohmann::json &jsonObject, const AbilityInfo &abilityInfo)
{
    jsonObject = nlohmann::json {
        {JSON_KEY_NAME, abilityInfo.name},
        {JSON_KEY_LABEL, abilityInfo.label},
        {JSON_KEY_DESCRIPTION, abilityInfo.description},
        {JSON_KEY_ICON_PATH, abilityInfo.iconPath},
        {JSON_KEY_THEME, abilityInfo.theme},
        {JSON_KEY_VISIBLE, abilityInfo.visible},
        {JSON_KEY_IS_LAUNCHER_ABILITY, abilityInfo.isLauncherAbility},
        {JSON_KEY_IS_NATIVE_ABILITY, abilityInfo.isNativeAbility},
        {JSON_KEY_ENABLED, abilityInfo.enabled},
        {JSON_KEY_SUPPORT_PIP_MODE, abilityInfo.supportPipMode},
        {JSON_KEY_READ_PERMISSION, abilityInfo.readPermission},
        {JSON_KEY_WRITE_PERMISSION, abilityInfo.writePermission},
        {JSON_KEY_SRC_PATH, abilityInfo.srcPath},
        {JSON_KEY_SRC_LANGUAGE, abilityInfo.srcLanguage},
        {JSON_KEY_CONFIG_CHANGES, abilityInfo.configChanges},
        {JSON_KEY_FORM_ENTITY, abilityInfo.formEntity},
        {JSON_KEY_MIN_FORM_HEIGHT, abilityInfo.minFormHeight},
        {JSON_KEY_DEFAULT_FORM_HEIGHT, abilityInfo.defaultFormHeight},
        {JSON_KEY_BACKGROUND_MODES, abilityInfo.backgroundModes},
        {JSON_KEY_MIN_FORM_WIDTH, abilityInfo.minFormWidth},
        {JSON_KEY_DEFAULT_FORM_WIDTH, abilityInfo.defaultFormWidth},
        {JSON_KEY_LABEL_ID, abilityInfo.labelId},
        {JSON_KEY_DESCRIPTION_ID, abilityInfo.descriptionId},
        {JSON_KEY_ICON_ID, abilityInfo.iconId},
        {JSON_KEY_KIND, abilityInfo.kind},
        {JSON_KEY_TYPE, abilityInfo.type},
        {JSON_KEY_ORIENTATION, abilityInfo.orientation},
        {JSON_KEY_LAUNCH_MODE, abilityInfo.launchMode},
        {JSON_KEY_PERMISSIONS, abilityInfo.permissions},
        {JSON_KEY_PROCESS, abilityInfo.process},
        {JSON_KEY_DEVICE_TYPES, abilityInfo.deviceTypes},
        {JSON_KEY_DEVICE_CAPABILITIES, abilityInfo.deviceCapabilities},
        {JSON_KEY_URI, abilityInfo.uri},
        {JSON_KEY_TARGET_ABILITY, abilityInfo.targetAbility},
        {JSON_KEY_PACKAGE, abilityInfo.package},
        {JSON_KEY_BUNDLE_NAME, abilityInfo.bundleName},
        {JSON_KEY_MODULE_NAME, abilityInfo.moduleName},
        {JSON_KEY_APPLICATION_NAME, abilityInfo.applicationName},
        {JSON_KEY_DEVICE_ID, abilityInfo.deviceId},
        {JSON_KEY_CODE_PATH, abilityInfo.codePath},
        {JSON_KEY_RESOURCE_PATH, abilityInfo.resourcePath},
        {JSON_KEY_LIB_PATH, abilityInfo.libPath},
        {JSON_KEY_META_DATA, abilityInfo.metaData},
        {JSON_KEY_FORM_ENABLED, abilityInfo.formEnabled},
        {SRC_ENTRANCE, abilityInfo.srcEntrance},
        {META_DATA, abilityInfo.metadata}
    };
}

void from_json(const nlohmann::json &jsonObject, CustomizeData &customizeData)
{
    const auto &jsonObjectEnd = jsonObject.end();
    int32_t parseResult = ERR_OK;
    GetValueIfFindKey<std::string>(jsonObject,
        jsonObjectEnd,
        JSON_KEY_NAME,
        customizeData.name,
        JsonType::STRING,
        false,
        parseResult,
        ArrayType::NOT_ARRAY);
    GetValueIfFindKey<std::string>(jsonObject,
        jsonObjectEnd,
        JSON_KEY_META_VALUE,
        customizeData.value,
        JsonType::STRING,
        false,
        parseResult,
        ArrayType::NOT_ARRAY);
    GetValueIfFindKey<std::string>(jsonObject,
        jsonObjectEnd,
        JSON_KEY_META_EXTRA,
        customizeData.extra,
        JsonType::STRING,
        false,
        parseResult,
        ArrayType::NOT_ARRAY);
}

void from_json(const nlohmann::json &jsonObject, MetaData &metaData)
{
    const auto &jsonObjectEnd = jsonObject.end();
    int32_t parseResult = ERR_OK;
    GetValueIfFindKey<std::vector<CustomizeData>>(jsonObject,
        jsonObjectEnd,
        JSON_KEY_CUSTOMIZE_DATA,
        metaData.customizeData,
        JsonType::ARRAY,
        false,
        parseResult,
        ArrayType::OBJECT);
}

void from_json(const nlohmann::json &jsonObject, Metadata &metadata)
{
    const auto &jsonObjectEnd = jsonObject.end();
    int32_t parseResult = ERR_OK;
    GetValueIfFindKey<std::string>(jsonObject,
        jsonObjectEnd,
        META_DATA_NAME,
        metadata.name,
        JsonType::STRING,
        false,
        parseResult,
        ArrayType::NOT_ARRAY);
    GetValueIfFindKey<std::string>(jsonObject,
        jsonObjectEnd,
        META_DATA_VALUE,
        metadata.value,
        JsonType::STRING,
        false,
        parseResult,
        ArrayType::NOT_ARRAY);
    GetValueIfFindKey<std::string>(jsonObject,
        jsonObjectEnd,
        META_DATA_RESOURCE,
        metadata.resource,
        JsonType::STRING,
        false,
        parseResult,
        ArrayType::NOT_ARRAY);
    if (parseResult != ERR_OK) {
        APP_LOGE("read Ability Metadata from database error, error code : %{public}d", parseResult);
    }
}

void from_json(const nlohmann::json &jsonObject, AbilityInfo &abilityInfo)
{
    const auto &jsonObjectEnd = jsonObject.end();
    int32_t parseResult = ERR_OK;
    GetValueIfFindKey<std::string>(jsonObject,
        jsonObjectEnd,
        JSON_KEY_NAME,
        abilityInfo.name,
        JsonType::STRING,
        false,
        parseResult,
        ArrayType::NOT_ARRAY);
    GetValueIfFindKey<std::string>(jsonObject,
        jsonObjectEnd,
        JSON_KEY_LABEL,
        abilityInfo.label,
        JsonType::STRING,
        false,
        parseResult,
        ArrayType::NOT_ARRAY);
    GetValueIfFindKey<uint32_t>(jsonObject,
        jsonObjectEnd,
        JSON_KEY_BACKGROUND_MODES,
        abilityInfo.backgroundModes,
        JsonType::NUMBER,
        false,
        parseResult,
        ArrayType::NOT_ARRAY);
    GetValueIfFindKey<std::string>(jsonObject,
        jsonObjectEnd,
        JSON_KEY_DESCRIPTION,
        abilityInfo.description,
        JsonType::STRING,
        false,
        parseResult,
        ArrayType::NOT_ARRAY);
    GetValueIfFindKey<std::string>(jsonObject,
        jsonObjectEnd,
        JSON_KEY_ICON_PATH,
        abilityInfo.iconPath,
        JsonType::STRING,
        false,
        parseResult,
        ArrayType::NOT_ARRAY);
    GetValueIfFindKey<std::string>(jsonObject,
        jsonObjectEnd,
        JSON_KEY_THEME,
        abilityInfo.theme,
        JsonType::STRING,
        false,
        parseResult,
        ArrayType::NOT_ARRAY);
    GetValueIfFindKey<bool>(jsonObject,
        jsonObjectEnd,
        JSON_KEY_VISIBLE,
        abilityInfo.visible,
        JsonType::BOOLEAN,
        false,
        parseResult,
        ArrayType::NOT_ARRAY);
    GetValueIfFindKey<bool>(jsonObject,
        jsonObjectEnd,
        JSON_KEY_IS_LAUNCHER_ABILITY,
        abilityInfo.isLauncherAbility,
        JsonType::BOOLEAN,
        false,
        parseResult,
        ArrayType::NOT_ARRAY);
    GetValueIfFindKey<bool>(jsonObject,
        jsonObjectEnd,
        JSON_KEY_IS_NATIVE_ABILITY,
        abilityInfo.isNativeAbility,
        JsonType::BOOLEAN,
        false,
        parseResult,
        ArrayType::NOT_ARRAY);
    GetValueIfFindKey<bool>(jsonObject,
        jsonObjectEnd,
        JSON_KEY_ENABLED,
        abilityInfo.enabled,
        JsonType::BOOLEAN,
        false,
        parseResult,
        ArrayType::NOT_ARRAY);
    GetValueIfFindKey<bool>(jsonObject,
        jsonObjectEnd,
        JSON_KEY_SUPPORT_PIP_MODE,
        abilityInfo.supportPipMode,
        JsonType::BOOLEAN,
        false,
        parseResult,
        ArrayType::NOT_ARRAY);
    GetValueIfFindKey<std::string>(jsonObject,
        jsonObjectEnd,
        JSON_KEY_READ_PERMISSION,
        abilityInfo.readPermission,
        JsonType::STRING,
        false,
        parseResult,
        ArrayType::NOT_ARRAY);
    GetValueIfFindKey<std::string>(jsonObject,
        jsonObjectEnd,
        JSON_KEY_WRITE_PERMISSION,
        abilityInfo.writePermission,
        JsonType::STRING,
        false,
        parseResult,
        ArrayType::NOT_ARRAY);
    GetValueIfFindKey<std::string>(jsonObject,
        jsonObjectEnd,
        JSON_KEY_SRC_PATH,
        abilityInfo.srcPath,
        JsonType::STRING,
        false,
        parseResult,
        ArrayType::NOT_ARRAY);
    GetValueIfFindKey<std::string>(jsonObject,
        jsonObjectEnd,
        JSON_KEY_SRC_LANGUAGE,
        abilityInfo.srcLanguage,
        JsonType::STRING,
        false,
        parseResult,
        ArrayType::NOT_ARRAY);
    GetValueIfFindKey<std::vector<std::string>>(jsonObject,
        jsonObjectEnd,
        JSON_KEY_CONFIG_CHANGES,
        abilityInfo.configChanges,
        JsonType::ARRAY,
        false,
        parseResult,
        ArrayType::STRING);
    GetValueIfFindKey<uint32_t>(jsonObject,
        jsonObjectEnd,
        JSON_KEY_FORM_ENTITY,
        abilityInfo.formEntity,
        JsonType::NUMBER,
        false,
        parseResult,
        ArrayType::NOT_ARRAY);
    GetValueIfFindKey<int32_t>(jsonObject,
        jsonObjectEnd,
        JSON_KEY_MIN_FORM_HEIGHT,
        abilityInfo.minFormHeight,
        JsonType::NUMBER,
        false,
        parseResult,
        ArrayType::NOT_ARRAY);
    GetValueIfFindKey<int32_t>(jsonObject,
        jsonObjectEnd,
        JSON_KEY_DEFAULT_FORM_HEIGHT,
        abilityInfo.defaultFormHeight,
        JsonType::NUMBER,
        false,
        parseResult,
        ArrayType::NOT_ARRAY);
    GetValueIfFindKey<int32_t>(jsonObject,
        jsonObjectEnd,
        JSON_KEY_MIN_FORM_WIDTH,
        abilityInfo.minFormWidth,
        JsonType::NUMBER,
        false,
        parseResult,
        ArrayType::NOT_ARRAY);
    GetValueIfFindKey<int32_t>(jsonObject,
        jsonObjectEnd,
        JSON_KEY_DEFAULT_FORM_WIDTH,
        abilityInfo.defaultFormWidth,
        JsonType::NUMBER,
        false,
        parseResult,
        ArrayType::NOT_ARRAY);
    GetValueIfFindKey<int32_t>(jsonObject,
        jsonObjectEnd,
        JSON_KEY_LABEL_ID,
        abilityInfo.labelId,
        JsonType::NUMBER,
        false,
        parseResult,
        ArrayType::NOT_ARRAY);
    GetValueIfFindKey<int32_t>(jsonObject,
        jsonObjectEnd,
        JSON_KEY_DESCRIPTION_ID,
        abilityInfo.descriptionId,
        JsonType::NUMBER,
        false,
        parseResult,
        ArrayType::NOT_ARRAY);
    GetValueIfFindKey<int32_t>(jsonObject,
        jsonObjectEnd,
        JSON_KEY_ICON_ID,
        abilityInfo.iconId,
        JsonType::NUMBER,
        false,
        parseResult,
        ArrayType::NOT_ARRAY);
    GetValueIfFindKey<std::string>(jsonObject,
        jsonObjectEnd,
        JSON_KEY_KIND,
        abilityInfo.kind,
        JsonType::STRING,
        false,
        parseResult,
        ArrayType::NOT_ARRAY);
    GetValueIfFindKey<AbilityType>(jsonObject,
        jsonObjectEnd,
        JSON_KEY_TYPE,
        abilityInfo.type,
        JsonType::NUMBER,
        false,
        parseResult,
        ArrayType::NOT_ARRAY);
    GetValueIfFindKey<DisplayOrientation>(jsonObject,
        jsonObjectEnd,
        JSON_KEY_ORIENTATION,
        abilityInfo.orientation,
        JsonType::NUMBER,
        false,
        parseResult,
        ArrayType::NOT_ARRAY);
    GetValueIfFindKey<LaunchMode>(jsonObject,
        jsonObjectEnd,
        JSON_KEY_LAUNCH_MODE,
        abilityInfo.launchMode,
        JsonType::NUMBER,
        false,
        parseResult,
        ArrayType::NOT_ARRAY);
    GetValueIfFindKey<std::vector<std::string>>(jsonObject,
        jsonObjectEnd,
        JSON_KEY_PERMISSIONS,
        abilityInfo.permissions,
        JsonType::ARRAY,
        false,
        parseResult,
        ArrayType::STRING);
    GetValueIfFindKey<std::string>(jsonObject,
        jsonObjectEnd,
        JSON_KEY_PROCESS,
        abilityInfo.process,
        JsonType::STRING,
        false,
        parseResult,
        ArrayType::NOT_ARRAY);
    GetValueIfFindKey<std::vector<std::string>>(jsonObject,
        jsonObjectEnd,
        JSON_KEY_DEVICE_TYPES,
        abilityInfo.deviceTypes,
        JsonType::ARRAY,
        false,
        parseResult,
        ArrayType::STRING);
    GetValueIfFindKey<std::vector<std::string>>(jsonObject,
        jsonObjectEnd,
        JSON_KEY_DEVICE_CAPABILITIES,
        abilityInfo.deviceCapabilities,
        JsonType::ARRAY,
        false,
        parseResult,
        ArrayType::STRING);
    GetValueIfFindKey<std::string>(jsonObject,
        jsonObjectEnd,
        JSON_KEY_URI,
        abilityInfo.uri,
        JsonType::STRING,
        false,
        parseResult,
        ArrayType::NOT_ARRAY);
    GetValueIfFindKey<std::string>(jsonObject,
        jsonObjectEnd,
        JSON_KEY_TARGET_ABILITY,
        abilityInfo.targetAbility,
        JsonType::STRING,
        false,
        parseResult,
        ArrayType::NOT_ARRAY);
    GetValueIfFindKey<std::string>(jsonObject,
        jsonObjectEnd,
        JSON_KEY_PACKAGE,
        abilityInfo.package,
        JsonType::STRING,
        false,
        parseResult,
        ArrayType::NOT_ARRAY);
    GetValueIfFindKey<std::string>(jsonObject,
        jsonObjectEnd,
        JSON_KEY_BUNDLE_NAME,
        abilityInfo.bundleName,
        JsonType::STRING,
        false,
        parseResult,
        ArrayType::NOT_ARRAY);
    GetValueIfFindKey<std::string>(jsonObject,
        jsonObjectEnd,
        JSON_KEY_MODULE_NAME,
        abilityInfo.moduleName,
        JsonType::STRING,
        false,
        parseResult,
        ArrayType::NOT_ARRAY);
    GetValueIfFindKey<std::string>(jsonObject,
        jsonObjectEnd,
        JSON_KEY_APPLICATION_NAME,
        abilityInfo.applicationName,
        JsonType::STRING,
        false,
        parseResult,
        ArrayType::NOT_ARRAY);
    GetValueIfFindKey<std::string>(jsonObject,
        jsonObjectEnd,
        JSON_KEY_DEVICE_ID,
        abilityInfo.deviceId,
        JsonType::STRING,
        false,
        parseResult,
        ArrayType::NOT_ARRAY);
    GetValueIfFindKey<std::string>(jsonObject,
        jsonObjectEnd,
        JSON_KEY_CODE_PATH,
        abilityInfo.codePath,
        JsonType::STRING,
        false,
        parseResult,
        ArrayType::NOT_ARRAY);
    GetValueIfFindKey<std::string>(jsonObject,
        jsonObjectEnd,
        JSON_KEY_RESOURCE_PATH,
        abilityInfo.resourcePath,
        JsonType::STRING,
        false,
        parseResult,
        ArrayType::NOT_ARRAY);
    GetValueIfFindKey<std::string>(jsonObject,
        jsonObjectEnd,
        JSON_KEY_LIB_PATH,
        abilityInfo.libPath,
        JsonType::STRING,
        false,
        parseResult,
        ArrayType::NOT_ARRAY);
    GetValueIfFindKey<MetaData>(jsonObject,
        jsonObjectEnd,
        JSON_KEY_META_DATA,
        abilityInfo.metaData,
        JsonType::OBJECT,
        false,
        parseResult,
        ArrayType::NOT_ARRAY);
    GetValueIfFindKey<bool>(jsonObject,
        jsonObjectEnd,
        JSON_KEY_FORM_ENABLED,
        abilityInfo.formEnabled,
        JsonType::BOOLEAN,
        false,
        parseResult,
        ArrayType::NOT_ARRAY);
    GetValueIfFindKey<std::string>(jsonObject,
        jsonObjectEnd,
        SRC_ENTRANCE,
        abilityInfo.srcEntrance,
        JsonType::STRING,
        false,
        parseResult,
        ArrayType::NOT_ARRAY);
    GetValueIfFindKey<std::vector<Metadata>>(jsonObject,
        jsonObjectEnd,
        META_DATA,
        abilityInfo.metadata,
        JsonType::ARRAY,
        false,
        parseResult,
        ArrayType::OBJECT);
    if (parseResult != ERR_OK) {
        APP_LOGE("read Ability from database error, error code : %{public}d", parseResult);
    }
}

void AbilityInfo::ConvertToCompatiableAbilityInfo(CompatibleAbilityInfo& compatibleAbilityInfo) const
{
    APP_LOGE("AbilityInfo::ConvertToCompatiableAbilityInfo called");
    compatibleAbilityInfo.package = package;
    compatibleAbilityInfo.name = name;
    compatibleAbilityInfo.label = label;
    compatibleAbilityInfo.description = description;
    compatibleAbilityInfo.iconPath = iconPath;
    compatibleAbilityInfo.uri = uri;
    compatibleAbilityInfo.moduleName = moduleName;
    compatibleAbilityInfo.process = process;
    compatibleAbilityInfo.targetAbility = targetAbility;
    compatibleAbilityInfo.appName = appName;
    compatibleAbilityInfo.privacyUrl = privacyUrl;
    compatibleAbilityInfo.privacyName = privacyName;
    compatibleAbilityInfo.downloadUrl = downloadUrl;
    compatibleAbilityInfo.versionName = versionName;
    compatibleAbilityInfo.backgroundModes = backgroundModes;
    compatibleAbilityInfo.packageSize = packageSize;
    compatibleAbilityInfo.visible = visible;
    compatibleAbilityInfo.formEnabled = formEnabled;
    compatibleAbilityInfo.multiUserShared = multiUserShared;
    compatibleAbilityInfo.type = type;
    compatibleAbilityInfo.subType = subType;
    compatibleAbilityInfo.orientation = orientation;
    compatibleAbilityInfo.launchMode = launchMode;
    compatibleAbilityInfo.permissions = permissions;
    compatibleAbilityInfo.deviceTypes = deviceTypes;
    compatibleAbilityInfo.deviceCapabilities = deviceCapabilities;
    compatibleAbilityInfo.supportPipMode = supportPipMode;
    compatibleAbilityInfo.grantPermission = grantPermission;
    compatibleAbilityInfo.readPermission = readPermission;
    compatibleAbilityInfo.writePermission = writePermission;
    compatibleAbilityInfo.uriPermissionMode = uriPermissionMode;
    compatibleAbilityInfo.uriPermissionPath = uriPermissionPath;
    compatibleAbilityInfo.directLaunch = directLaunch;
    compatibleAbilityInfo.bundleName = bundleName;
    compatibleAbilityInfo.className = className;
    compatibleAbilityInfo.originalClassName = originalClassName;
    compatibleAbilityInfo.deviceId = deviceId;
    CompatibleApplicationInfo convertedCompatibleApplicationInfo;
    applicationInfo.ConvertToCompatibleApplicationInfo(convertedCompatibleApplicationInfo);
    compatibleAbilityInfo.applicationInfo = convertedCompatibleApplicationInfo;
    compatibleAbilityInfo.formEntity = formEntity;
    compatibleAbilityInfo.minFormHeight = minFormHeight;
    compatibleAbilityInfo.defaultFormHeight = defaultFormHeight;
    compatibleAbilityInfo.minFormWidth = minFormWidth;
    compatibleAbilityInfo.defaultFormWidth = defaultFormWidth;
    compatibleAbilityInfo.iconId = iconId;
    compatibleAbilityInfo.labelId = labelId;
    compatibleAbilityInfo.descriptionId = descriptionId;
    compatibleAbilityInfo.enabled = enabled;
}
}  // namespace AppExecFwk
}  // namespace OHOS