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

#include "nlohmann/json.hpp"

#include "string_ex.h"

#include "app_log_wrapper.h"
#include "bundle_constants.h"
#include "parcel_macro.h"

namespace OHOS {
namespace AppExecFwk {
using namespace Constants;

bool CompatibleAbilityInfo::ReadFromParcel(Parcel& parcel)
{
    APP_LOGD("CompatibleAbilityInfo::ReadFromParcel called");
    package = Str16ToStr8(parcel.ReadString16());
    name = Str16ToStr8(parcel.ReadString16());
    originalClassName = Str16ToStr8(parcel.ReadString16());
    label = Str16ToStr8(parcel.ReadString16());
    description = Str16ToStr8(parcel.ReadString16());
    iconPath = Str16ToStr8(parcel.ReadString16());
    uri = Str16ToStr8(parcel.ReadString16());
    moduleName = Str16ToStr8(parcel.ReadString16());
    process = Str16ToStr8(parcel.ReadString16());
    targetAbility = Str16ToStr8(parcel.ReadString16());
    appName = Str16ToStr8(parcel.ReadString16());
    privacyUrl = Str16ToStr8(parcel.ReadString16());
    privacyName = Str16ToStr8(parcel.ReadString16());
    downloadUrl = Str16ToStr8(parcel.ReadString16());
    versionName = Str16ToStr8(parcel.ReadString16());
    backgroundModes = parcel.ReadInt32();
    packageSize = parcel.ReadInt32();
    visible = parcel.ReadBool();
    formEnabled = parcel.ReadBool();
    multiUserShared = parcel.ReadBool();

    int32_t typeData;
    READ_PARCEL_AND_RETURN_FALSE_IF_FAIL(Int32, parcel, typeData);
    type = static_cast<AbilityType>(typeData);

    int32_t orientationData;
    READ_PARCEL_AND_RETURN_FALSE_IF_FAIL(Int32, parcel, orientationData);
    orientation = static_cast<DisplayOrientation>(orientationData);

    int32_t launchModeData;
    READ_PARCEL_AND_RETURN_FALSE_IF_FAIL(Int32, parcel, launchModeData);
    launchMode = static_cast<LaunchMode>(launchModeData);

    supportPipMode = parcel.ReadBool();
    grantPermission = parcel.ReadBool();
    readPermission = Str16ToStr8(parcel.ReadString16());
    writePermission = Str16ToStr8(parcel.ReadString16());
    uriPermissionMode = Str16ToStr8(parcel.ReadString16());
    uriPermissionPath = Str16ToStr8(parcel.ReadString16());
    directLaunch = parcel.ReadBool();
    bundleName = Str16ToStr8(parcel.ReadString16());
    className = Str16ToStr8(parcel.ReadString16());
    deviceId = Str16ToStr8(parcel.ReadString16());
    formEntity = parcel.ReadInt32();
    minFormHeight = parcel.ReadInt32();
    defaultFormHeight = parcel.ReadInt32();
    minFormWidth = parcel.ReadInt32();
    defaultFormWidth = parcel.ReadInt32();
    iconId = parcel.ReadInt32();
    descriptionId = parcel.ReadInt32();
    labelId = parcel.ReadInt32();
    enabled = parcel.ReadBool();
    return true;
}

CompatibleAbilityInfo* CompatibleAbilityInfo::Unmarshalling(Parcel& parcel)
{
    APP_LOGD("CompatibleAbilityInfo::Unmarshalling called");
    CompatibleAbilityInfo* info = new CompatibleAbilityInfo();
    if (!info->ReadFromParcel(parcel)) {
        APP_LOGW("read from parcel failed");
        delete info;
        info = nullptr;
    }
    return info;
}

bool CompatibleAbilityInfo::Marshalling(Parcel& parcel) const
{
    APP_LOGD("CompatibleAbilityInfo::Marshalling called");
    WRITE_PARCEL_AND_RETURN_FALSE_IF_FAIL(String16, parcel, Str8ToStr16(package));
    WRITE_PARCEL_AND_RETURN_FALSE_IF_FAIL(String16, parcel, Str8ToStr16(name));
    WRITE_PARCEL_AND_RETURN_FALSE_IF_FAIL(String16, parcel, Str8ToStr16(originalClassName));
    WRITE_PARCEL_AND_RETURN_FALSE_IF_FAIL(String16, parcel, Str8ToStr16(label));
    WRITE_PARCEL_AND_RETURN_FALSE_IF_FAIL(String16, parcel, Str8ToStr16(description));
    WRITE_PARCEL_AND_RETURN_FALSE_IF_FAIL(String16, parcel, Str8ToStr16(iconPath));
    WRITE_PARCEL_AND_RETURN_FALSE_IF_FAIL(String16, parcel, Str8ToStr16(uri));
    WRITE_PARCEL_AND_RETURN_FALSE_IF_FAIL(String16, parcel, Str8ToStr16(moduleName));
    WRITE_PARCEL_AND_RETURN_FALSE_IF_FAIL(String16, parcel, Str8ToStr16(process));
    WRITE_PARCEL_AND_RETURN_FALSE_IF_FAIL(String16, parcel, Str8ToStr16(targetAbility));
    WRITE_PARCEL_AND_RETURN_FALSE_IF_FAIL(String16, parcel, Str8ToStr16(appName));
    WRITE_PARCEL_AND_RETURN_FALSE_IF_FAIL(String16, parcel, Str8ToStr16(privacyUrl));
    WRITE_PARCEL_AND_RETURN_FALSE_IF_FAIL(String16, parcel, Str8ToStr16(privacyName));
    WRITE_PARCEL_AND_RETURN_FALSE_IF_FAIL(String16, parcel, Str8ToStr16(downloadUrl));
    WRITE_PARCEL_AND_RETURN_FALSE_IF_FAIL(String16, parcel, Str8ToStr16(versionName));
    WRITE_PARCEL_AND_RETURN_FALSE_IF_FAIL(Int32, parcel, backgroundModes);
    WRITE_PARCEL_AND_RETURN_FALSE_IF_FAIL(Int32, parcel, packageSize);
    WRITE_PARCEL_AND_RETURN_FALSE_IF_FAIL(Bool, parcel, visible);
    WRITE_PARCEL_AND_RETURN_FALSE_IF_FAIL(Bool, parcel, formEnabled);
    WRITE_PARCEL_AND_RETURN_FALSE_IF_FAIL(Bool, parcel, multiUserShared);
    WRITE_PARCEL_AND_RETURN_FALSE_IF_FAIL(Int32, parcel, static_cast<int32_t>(type));
    WRITE_PARCEL_AND_RETURN_FALSE_IF_FAIL(Int32, parcel, static_cast<int32_t>(orientation));
    WRITE_PARCEL_AND_RETURN_FALSE_IF_FAIL(Int32, parcel, static_cast<int32_t>(launchMode));

    WRITE_PARCEL_AND_RETURN_FALSE_IF_FAIL(Bool, parcel, supportPipMode);
    WRITE_PARCEL_AND_RETURN_FALSE_IF_FAIL(Bool, parcel, grantPermission);
    WRITE_PARCEL_AND_RETURN_FALSE_IF_FAIL(String16, parcel, Str8ToStr16(readPermission));
    WRITE_PARCEL_AND_RETURN_FALSE_IF_FAIL(String16, parcel, Str8ToStr16(writePermission));
    WRITE_PARCEL_AND_RETURN_FALSE_IF_FAIL(String16, parcel, Str8ToStr16(uriPermissionMode));
    WRITE_PARCEL_AND_RETURN_FALSE_IF_FAIL(String16, parcel, Str8ToStr16(uriPermissionPath));
    WRITE_PARCEL_AND_RETURN_FALSE_IF_FAIL(Bool, parcel, directLaunch);
    WRITE_PARCEL_AND_RETURN_FALSE_IF_FAIL(String16, parcel, Str8ToStr16(bundleName));
    WRITE_PARCEL_AND_RETURN_FALSE_IF_FAIL(String16, parcel, Str8ToStr16(className));
    WRITE_PARCEL_AND_RETURN_FALSE_IF_FAIL(String16, parcel, Str8ToStr16(deviceId));
    APP_LOGE("CompatibleAbilityInfo::Marshalling start to write application info.");
    WRITE_PARCEL_AND_RETURN_FALSE_IF_FAIL(Int32, parcel, formEntity);
    WRITE_PARCEL_AND_RETURN_FALSE_IF_FAIL(Int32, parcel, minFormHeight);
    WRITE_PARCEL_AND_RETURN_FALSE_IF_FAIL(Int32, parcel, defaultFormHeight);
    WRITE_PARCEL_AND_RETURN_FALSE_IF_FAIL(Int32, parcel, minFormWidth);
    WRITE_PARCEL_AND_RETURN_FALSE_IF_FAIL(Int32, parcel, defaultFormWidth);
    WRITE_PARCEL_AND_RETURN_FALSE_IF_FAIL(Int32, parcel, iconId);
    WRITE_PARCEL_AND_RETURN_FALSE_IF_FAIL(Int32, parcel, descriptionId);
    WRITE_PARCEL_AND_RETURN_FALSE_IF_FAIL(Int32, parcel, labelId);
    WRITE_PARCEL_AND_RETURN_FALSE_IF_FAIL(Bool, parcel, enabled);
    return true;
}

void CompatibleAbilityInfo::ConvertToAbilityInfo(AbilityInfo& abilityInfo) const
{
    APP_LOGD("CompatibleAbilityInfo::ConvertToAbilityInfo called");
    abilityInfo.package = package;
    abilityInfo.name = name;
    abilityInfo.label = label;
    abilityInfo.description = description;
    abilityInfo.iconPath = iconPath;
    abilityInfo.uri = uri;
    abilityInfo.moduleName = moduleName;
    abilityInfo.process = process;
    abilityInfo.targetAbility = targetAbility;
    abilityInfo.visible = visible;
    abilityInfo.type = type;
    abilityInfo.orientation = orientation;
    abilityInfo.launchMode = launchMode;
    abilityInfo.permissions = permissions;
    abilityInfo.deviceTypes = deviceTypes;
    abilityInfo.deviceCapabilities = deviceCapabilities;
    abilityInfo.readPermission = readPermission;
    abilityInfo.writePermission = writePermission;
    abilityInfo.bundleName = bundleName;
    abilityInfo.deviceId = deviceId;
    ApplicationInfo convertedApplicationInfo;
    applicationInfo.ConvertToApplicationInfo(convertedApplicationInfo);
    abilityInfo.applicationInfo = convertedApplicationInfo;
    abilityInfo.formEntity = formEntity;
    abilityInfo.minFormHeight = minFormHeight;
    abilityInfo.defaultFormHeight = defaultFormHeight;
    abilityInfo.minFormWidth = minFormWidth;
    abilityInfo.defaultFormWidth = defaultFormWidth;
    abilityInfo.enabled = enabled;
}
} // namespace AppExecFwk
} // namespace OHOS
