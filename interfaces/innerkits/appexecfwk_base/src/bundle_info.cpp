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

#include "bundle_info.h"

#include "json_util.h"
#include "parcel_macro.h"
#include "message_parcel.h"
#include "string_ex.h"

namespace OHOS {
namespace AppExecFwk {
namespace {
const std::string BUNDLE_INFO_NAME = "name";
const std::string BUNDLE_INFO_LABEL = "label";
const std::string BUNDLE_INFO_DESCRIPTION = "description";
const std::string BUNDLE_INFO_VENDOR = "vendor";
const std::string BUNDLE_INFO_IS_KEEP_ALIVE = "isKeepAlive";
const std::string BUNDLE_INFO_IS_NATIVE_APP = "isNativeApp";
const std::string BUNDLE_INFO_IS_DIFFERENT_NAME = "isDifferentName";
const std::string BUNDLE_INFO_APPLICATION_INFO = "applicationInfo";
const std::string BUNDLE_INFO_ABILITY_INFOS = "abilityInfos";
const std::string BUNDLE_INFO_EXTENSION_ABILITY_INFOS = "extensionAbilityInfo";
const std::string BUNDLE_INFO_JOINT_USERID = "jointUserId";
const std::string BUNDLE_INFO_VERSION_CODE = "versionCode";
const std::string BUNDLE_INFO_MIN_COMPATIBLE_VERSION_CODE = "minCompatibleVersionCode";
const std::string BUNDLE_INFO_VERSION_NAME = "versionName";
const std::string BUNDLE_INFO_MIN_SDK_VERSION = "minSdkVersion";
const std::string BUNDLE_INFO_MAX_SDK_VERSION = "maxSdkVersion";
const std::string BUNDLE_INFO_MAIN_ENTRY = "mainEntry";
const std::string BUNDLE_INFO_CPU_ABI = "cpuAbi";
const std::string BUNDLE_INFO_APPID = "appId";
const std::string BUNDLE_INFO_COMPATIBLE_VERSION = "compatibleVersion";
const std::string BUNDLE_INFO_TARGET_VERSION = "targetVersion";
const std::string BUNDLE_INFO_RELEASE_TYPE = "releaseType";
const std::string BUNDLE_INFO_UID = "uid";
const std::string BUNDLE_INFO_GID = "gid";
const std::string BUNDLE_INFO_SEINFO = "seInfo";
const std::string BUNDLE_INFO_INSTALL_TIME = "installTime";
const std::string BUNDLE_INFO_UPDATE_TIME = "updateTime";
const std::string BUNDLE_INFO_ENTRY_MODULE_NAME = "entryModuleName";
const std::string BUNDLE_INFO_ENTRY_INSTALLATION_FREE = "entryInstallationFree";
const std::string BUNDLE_INFO_REQ_PERMISSIONS = "reqPermissions";
const std::string BUNDLE_INFO_REQ_PERMISSION_STATES = "reqPermissionStates";
const std::string BUNDLE_INFO_DEF_PERMISSIONS = "defPermissions";
const std::string BUNDLE_INFO_HAP_MODULE_NAMES = "hapModuleNames";
const std::string BUNDLE_INFO_MODULE_NAMES = "moduleNames";
const std::string BUNDLE_INFO_MODULE_PUBLIC_DIRS = "modulePublicDirs";
const std::string BUNDLE_INFO_MODULE_DIRS = "moduleDirs";
const std::string BUNDLE_INFO_MODULE_RES_PATHS = "moduleResPaths";
}

bool BundleInfo::ReadFromParcel(Parcel &parcel)
{
    MessageParcel *messageParcel = reinterpret_cast<MessageParcel *>(&parcel);
    if (!messageParcel) {
        APP_LOGE("Type conversion failed");
        return false;
    }
    uint32_t length = messageParcel->ReadUint32();
    if (length == 0) {
        APP_LOGE("Invalid data length");
        return false;
    }
    const char *data = reinterpret_cast<const char *>(messageParcel->ReadRawData(length));
    if (!data) {
        APP_LOGE("Fail to read raw data, length = %{public}d", length);
        return false;
    }
    nlohmann::json jsonObject = nlohmann::json::parse(data, nullptr, false);
    if (jsonObject.is_discarded()) {
        APP_LOGE("failed to parse BundleInfo");
        return false;
    }
    *this = jsonObject.get<BundleInfo>();
    return true;
}

bool BundleInfo::Marshalling(Parcel &parcel) const
{
    MessageParcel *messageParcel = reinterpret_cast<MessageParcel *>(&parcel);
    if (!messageParcel) {
        APP_LOGE("Type conversion failed");
        return false;
    }
    nlohmann::json json = *this;
    std::string str = json.dump();
    if (!messageParcel->WriteUint32(str.size() + 1)) {
        APP_LOGE("Failed to write data size");
        return false;
    }
    if (!messageParcel->WriteRawData(str.c_str(), str.size() + 1)) {
        APP_LOGE("Failed to write data");
        return false;
    }
    return true;
}

BundleInfo *BundleInfo::Unmarshalling(Parcel &parcel)
{
    BundleInfo *info = new (std::nothrow) BundleInfo();
    if (info && !info->ReadFromParcel(parcel)) {
        APP_LOGW("read from parcel failed");
        delete info;
        info = nullptr;
    }
    return info;
}

void to_json(nlohmann::json &jsonObject, const BundleInfo &bundleInfo)
{
    jsonObject = nlohmann::json {
        {BUNDLE_INFO_NAME, bundleInfo.name},
        {BUNDLE_INFO_LABEL, bundleInfo.label},
        {BUNDLE_INFO_DESCRIPTION, bundleInfo.description},
        {BUNDLE_INFO_VENDOR, bundleInfo.vendor},
        {BUNDLE_INFO_IS_KEEP_ALIVE, bundleInfo.isKeepAlive},
        {BUNDLE_INFO_IS_NATIVE_APP, bundleInfo.isNativeApp},
        {BUNDLE_INFO_IS_DIFFERENT_NAME, bundleInfo.isDifferentName},
        {BUNDLE_INFO_APPLICATION_INFO, bundleInfo.applicationInfo},
        {BUNDLE_INFO_ABILITY_INFOS, bundleInfo.abilityInfos},
        {BUNDLE_INFO_EXTENSION_ABILITY_INFOS, bundleInfo.extensionInfos},
        {BUNDLE_INFO_JOINT_USERID, bundleInfo.jointUserId},
        {BUNDLE_INFO_VERSION_CODE, bundleInfo.versionCode},
        {BUNDLE_INFO_MIN_COMPATIBLE_VERSION_CODE, bundleInfo.minCompatibleVersionCode},
        {BUNDLE_INFO_VERSION_NAME, bundleInfo.versionName},
        {BUNDLE_INFO_MIN_SDK_VERSION, bundleInfo.minSdkVersion},
        {BUNDLE_INFO_MAX_SDK_VERSION, bundleInfo.maxSdkVersion},
        {BUNDLE_INFO_MAIN_ENTRY, bundleInfo.mainEntry},
        {BUNDLE_INFO_CPU_ABI, bundleInfo.cpuAbi},
        {BUNDLE_INFO_APPID, bundleInfo.appId},
        {BUNDLE_INFO_COMPATIBLE_VERSION, bundleInfo.compatibleVersion},
        {BUNDLE_INFO_TARGET_VERSION, bundleInfo.targetVersion},
        {BUNDLE_INFO_RELEASE_TYPE, bundleInfo.releaseType},
        {BUNDLE_INFO_UID, bundleInfo.uid},
        {BUNDLE_INFO_GID, bundleInfo.gid},
        {BUNDLE_INFO_SEINFO, bundleInfo.seInfo},
        {BUNDLE_INFO_INSTALL_TIME, bundleInfo.installTime},
        {BUNDLE_INFO_UPDATE_TIME, bundleInfo.updateTime},
        {BUNDLE_INFO_ENTRY_MODULE_NAME, bundleInfo.entryModuleName},
        {BUNDLE_INFO_ENTRY_INSTALLATION_FREE, bundleInfo.entryInstallationFree},
        {BUNDLE_INFO_REQ_PERMISSIONS, bundleInfo.reqPermissions},
        {BUNDLE_INFO_REQ_PERMISSION_STATES, bundleInfo.reqPermissionStates},
        {BUNDLE_INFO_DEF_PERMISSIONS, bundleInfo.defPermissions},
        {BUNDLE_INFO_HAP_MODULE_NAMES, bundleInfo.hapModuleNames},
        {BUNDLE_INFO_MODULE_NAMES, bundleInfo.moduleNames},
        {BUNDLE_INFO_MODULE_PUBLIC_DIRS, bundleInfo.modulePublicDirs},
        {BUNDLE_INFO_MODULE_DIRS, bundleInfo.moduleDirs},
        {BUNDLE_INFO_MODULE_RES_PATHS, bundleInfo.moduleResPaths},
        {"singleUser", bundleInfo.singleUser}
    };
}

void from_json(const nlohmann::json &jsonObject, BundleInfo &bundleInfo)
{
    const auto &jsonObjectEnd = jsonObject.end();
    int32_t parseResult = ERR_OK;
    GetValueIfFindKey<std::string>(jsonObject,
        jsonObjectEnd,
        BUNDLE_INFO_NAME,
        bundleInfo.name,
        JsonType::STRING,
        false,
        parseResult,
        ArrayType::NOT_ARRAY);
    GetValueIfFindKey<std::string>(jsonObject,
        jsonObjectEnd,
        BUNDLE_INFO_LABEL,
        bundleInfo.label,
        JsonType::STRING,
        false,
        parseResult,
        ArrayType::NOT_ARRAY);
    GetValueIfFindKey<std::string>(jsonObject,
        jsonObjectEnd,
        BUNDLE_INFO_DESCRIPTION,
        bundleInfo.description,
        JsonType::STRING,
        false,
        parseResult,
        ArrayType::NOT_ARRAY);
    GetValueIfFindKey<std::string>(jsonObject,
        jsonObjectEnd,
        BUNDLE_INFO_VENDOR,
        bundleInfo.vendor,
        JsonType::STRING,
        false,
        parseResult,
        ArrayType::NOT_ARRAY);
    GetValueIfFindKey<bool>(jsonObject,
        jsonObjectEnd,
        BUNDLE_INFO_IS_KEEP_ALIVE,
        bundleInfo.isKeepAlive,
        JsonType::BOOLEAN,
        false,
        parseResult,
        ArrayType::NOT_ARRAY);
    GetValueIfFindKey<bool>(jsonObject,
        jsonObjectEnd,
        BUNDLE_INFO_IS_NATIVE_APP,
        bundleInfo.isNativeApp,
        JsonType::BOOLEAN,
        false,
        parseResult,
        ArrayType::NOT_ARRAY);
    GetValueIfFindKey<bool>(jsonObject,
        jsonObjectEnd,
        BUNDLE_INFO_IS_DIFFERENT_NAME,
        bundleInfo.isDifferentName,
        JsonType::BOOLEAN,
        false,
        parseResult,
        ArrayType::NOT_ARRAY);
    GetValueIfFindKey<ApplicationInfo>(jsonObject,
        jsonObjectEnd,
        BUNDLE_INFO_APPLICATION_INFO,
        bundleInfo.applicationInfo,
        JsonType::OBJECT,
        false,
        parseResult,
        ArrayType::NOT_ARRAY);
    GetValueIfFindKey<std::vector<AbilityInfo>>(jsonObject,
        jsonObjectEnd,
        BUNDLE_INFO_ABILITY_INFOS,
        bundleInfo.abilityInfos,
        JsonType::ARRAY,
        false,
        parseResult,
        ArrayType::OBJECT);
    GetValueIfFindKey<uint32_t>(jsonObject,
        jsonObjectEnd,
        BUNDLE_INFO_VERSION_CODE,
        bundleInfo.versionCode,
        JsonType::NUMBER,
        false,
        parseResult,
        ArrayType::NOT_ARRAY);
    GetValueIfFindKey<uint32_t>(jsonObject,
        jsonObjectEnd,
        BUNDLE_INFO_MIN_COMPATIBLE_VERSION_CODE,
        bundleInfo.minCompatibleVersionCode,
        JsonType::NUMBER,
        false,
        parseResult,
        ArrayType::NOT_ARRAY);
    GetValueIfFindKey<std::string>(jsonObject,
        jsonObjectEnd,
        BUNDLE_INFO_VERSION_NAME,
        bundleInfo.versionName,
        JsonType::STRING,
        false,
        parseResult,
        ArrayType::NOT_ARRAY);
    GetValueIfFindKey<std::string>(jsonObject,
        jsonObjectEnd,
        BUNDLE_INFO_JOINT_USERID,
        bundleInfo.jointUserId,
        JsonType::STRING,
        false,
        parseResult,
        ArrayType::NOT_ARRAY);
    GetValueIfFindKey<int32_t>(jsonObject,
        jsonObjectEnd,
        BUNDLE_INFO_MIN_SDK_VERSION,
        bundleInfo.minSdkVersion,
        JsonType::NUMBER,
        false,
        parseResult,
        ArrayType::NOT_ARRAY);
    GetValueIfFindKey<int32_t>(jsonObject,
        jsonObjectEnd,
        BUNDLE_INFO_MAX_SDK_VERSION,
        bundleInfo.maxSdkVersion,
        JsonType::NUMBER,
        false,
        parseResult,
        ArrayType::NOT_ARRAY);
    GetValueIfFindKey<std::string>(jsonObject,
        jsonObjectEnd,
        BUNDLE_INFO_MAIN_ENTRY,
        bundleInfo.mainEntry,
        JsonType::STRING,
        false,
        parseResult,
        ArrayType::NOT_ARRAY);
    GetValueIfFindKey<std::string>(jsonObject,
        jsonObjectEnd,
        BUNDLE_INFO_CPU_ABI,
        bundleInfo.cpuAbi,
        JsonType::STRING,
        false,
        parseResult,
        ArrayType::NOT_ARRAY);
    GetValueIfFindKey<std::string>(jsonObject,
        jsonObjectEnd,
        BUNDLE_INFO_APPID,
        bundleInfo.appId,
        JsonType::STRING,
        false,
        parseResult,
        ArrayType::NOT_ARRAY);
    GetValueIfFindKey<int>(jsonObject,
        jsonObjectEnd,
        BUNDLE_INFO_COMPATIBLE_VERSION,
        bundleInfo.compatibleVersion,
        JsonType::NUMBER,
        false,
        parseResult,
        ArrayType::NOT_ARRAY);
    GetValueIfFindKey<int>(jsonObject,
        jsonObjectEnd,
        BUNDLE_INFO_TARGET_VERSION,
        bundleInfo.targetVersion,
        JsonType::NUMBER,
        false,
        parseResult,
        ArrayType::NOT_ARRAY);
    GetValueIfFindKey<std::string>(jsonObject,
        jsonObjectEnd,
        BUNDLE_INFO_RELEASE_TYPE,
        bundleInfo.releaseType,
        JsonType::STRING,
        false,
        parseResult,
        ArrayType::NOT_ARRAY);
    GetValueIfFindKey<int>(jsonObject,
        jsonObjectEnd,
        BUNDLE_INFO_UID,
        bundleInfo.uid,
        JsonType::NUMBER,
        false,
        parseResult,
        ArrayType::NOT_ARRAY);
    GetValueIfFindKey<int>(jsonObject,
        jsonObjectEnd,
        BUNDLE_INFO_GID,
        bundleInfo.gid,
        JsonType::NUMBER,
        false,
        parseResult,
        ArrayType::NOT_ARRAY);
    GetValueIfFindKey<std::string>(jsonObject,
        jsonObjectEnd,
        BUNDLE_INFO_SEINFO,
        bundleInfo.seInfo,
        JsonType::STRING,
        false,
        parseResult,
        ArrayType::NOT_ARRAY);
    GetValueIfFindKey<int64_t>(jsonObject,
        jsonObjectEnd,
        BUNDLE_INFO_INSTALL_TIME,
        bundleInfo.installTime,
        JsonType::NUMBER,
        false,
        parseResult,
        ArrayType::NOT_ARRAY);
    GetValueIfFindKey<int64_t>(jsonObject,
        jsonObjectEnd,
        BUNDLE_INFO_UPDATE_TIME,
        bundleInfo.updateTime,
        JsonType::NUMBER,
        false,
        parseResult,
        ArrayType::NOT_ARRAY);
    GetValueIfFindKey<std::string>(jsonObject,
        jsonObjectEnd,
        BUNDLE_INFO_ENTRY_MODULE_NAME,
        bundleInfo.entryModuleName,
        JsonType::STRING,
        false,
        parseResult,
        ArrayType::NOT_ARRAY);
    GetValueIfFindKey<bool>(jsonObject,
        jsonObjectEnd,
        BUNDLE_INFO_ENTRY_INSTALLATION_FREE,
        bundleInfo.entryInstallationFree,
        JsonType::BOOLEAN,
        false,
        parseResult,
        ArrayType::NOT_ARRAY);
    GetValueIfFindKey<std::vector<std::string>>(jsonObject,
        jsonObjectEnd,
        BUNDLE_INFO_REQ_PERMISSIONS,
        bundleInfo.reqPermissions,
        JsonType::ARRAY,
        false,
        parseResult,
        ArrayType::STRING);
    GetValueIfFindKey<std::vector<int32_t>>(jsonObject,
        jsonObjectEnd,
        BUNDLE_INFO_REQ_PERMISSION_STATES,
        bundleInfo.reqPermissionStates,
        JsonType::ARRAY,
        false,
        parseResult,
        ArrayType::NUMBER);
    GetValueIfFindKey<std::vector<std::string>>(jsonObject,
        jsonObjectEnd,
        BUNDLE_INFO_DEF_PERMISSIONS,
        bundleInfo.defPermissions,
        JsonType::ARRAY,
        false,
        parseResult,
        ArrayType::STRING);
    GetValueIfFindKey<std::vector<std::string>>(jsonObject,
        jsonObjectEnd,
        BUNDLE_INFO_HAP_MODULE_NAMES,
        bundleInfo.hapModuleNames,
        JsonType::ARRAY,
        false,
        parseResult,
        ArrayType::STRING);
    GetValueIfFindKey<std::vector<std::string>>(jsonObject,
        jsonObjectEnd,
        BUNDLE_INFO_MODULE_NAMES,
        bundleInfo.moduleNames,
        JsonType::ARRAY,
        false,
        parseResult,
        ArrayType::STRING);
    GetValueIfFindKey<std::vector<std::string>>(jsonObject,
        jsonObjectEnd,
        BUNDLE_INFO_MODULE_PUBLIC_DIRS,
        bundleInfo.modulePublicDirs,
        JsonType::ARRAY,
        false,
        parseResult,
        ArrayType::STRING);
    GetValueIfFindKey<std::vector<std::string>>(jsonObject,
        jsonObjectEnd,
        BUNDLE_INFO_MODULE_DIRS,
        bundleInfo.moduleDirs,
        JsonType::ARRAY,
        false,
        parseResult,
        ArrayType::STRING);
    GetValueIfFindKey<std::vector<std::string>>(jsonObject,
        jsonObjectEnd,
        BUNDLE_INFO_MODULE_RES_PATHS,
        bundleInfo.moduleResPaths,
        JsonType::ARRAY,
        false,
        parseResult,
        ArrayType::STRING);
    GetValueIfFindKey<bool>(jsonObject,
        jsonObjectEnd,
        "singleUser",
        bundleInfo.singleUser,
        JsonType::BOOLEAN,
        false,
        parseResult,
        ArrayType::NOT_ARRAY);
    GetValueIfFindKey<std::vector<ExtensionAbilityInfo>>(jsonObject,
        jsonObjectEnd,
        BUNDLE_INFO_EXTENSION_ABILITY_INFOS,
        bundleInfo.extensionInfos,
        JsonType::ARRAY,
        false,
        parseResult,
        ArrayType::OBJECT);
}

}  // namespace AppExecFwk
}  // namespace OHOS
