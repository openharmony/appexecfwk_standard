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

#include "string_ex.h"

#include "app_log_wrapper.h"
#include "json_serializer.h"
#include "parcel_macro.h"
#include "message_parcel.h"

namespace OHOS {
namespace AppExecFwk {

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
    jsonObject = nlohmann::json{
        {"name", bundleInfo.name},
        {"label", bundleInfo.label},
        {"description", bundleInfo.description},
        {"vendor", bundleInfo.vendor},
        {"isKeepAlive", bundleInfo.isKeepAlive},
        {"isNativeApp", bundleInfo.isNativeApp},
        {"isDifferentName", bundleInfo.isDifferentName},
        {"applicationInfo", bundleInfo.applicationInfo},
        {"abilityInfos", bundleInfo.abilityInfos},
        {"jointUserId", bundleInfo.jointUserId},
        {"versionCode", bundleInfo.versionCode},
        {"versionName", bundleInfo.versionName},
        {"minSdkVersion", bundleInfo.minSdkVersion},
        {"maxSdkVersion", bundleInfo.maxSdkVersion},
        {"mainEntry", bundleInfo.mainEntry},
        {"cpuAbi", bundleInfo.cpuAbi},
        {"appId", bundleInfo.appId},
        {"compatibleVersion", bundleInfo.compatibleVersion},
        {"targetVersion", bundleInfo.targetVersion},
        {"releaseType", bundleInfo.releaseType},
        {"uid", bundleInfo.uid},
        {"gid", bundleInfo.gid},
        {"seInfo", bundleInfo.seInfo},
        {"installTime", bundleInfo.installTime},
        {"updateTime", bundleInfo.updateTime},
        {"entryModuleName", bundleInfo.entryModuleName},
        {"reqPermissions", bundleInfo.reqPermissions},
        {"defPermissions", bundleInfo.defPermissions},
        {"hapModuleNames", bundleInfo.hapModuleNames},
        {"moduleNames", bundleInfo.moduleNames},
        {"modulePublicDirs", bundleInfo.modulePublicDirs},
        {"moduleDirs", bundleInfo.moduleDirs},
        {"moduleResPaths", bundleInfo.moduleResPaths}
    };
}

void from_json(const nlohmann::json &jsonObject, BundleInfo &bundleInfo)
{
    bundleInfo.name = jsonObject.at("name").get<std::string>();
    bundleInfo.label = jsonObject.at("label").get<std::string>();
    bundleInfo.description = jsonObject.at("description").get<std::string>();
    bundleInfo.vendor = jsonObject.at("vendor").get<std::string>();
    bundleInfo.isKeepAlive = jsonObject.at("isKeepAlive").get<bool>();
    bundleInfo.isNativeApp = jsonObject.at("isNativeApp").get<bool>();
    bundleInfo.isDifferentName = jsonObject.at("isDifferentName").get<bool>();
    bundleInfo.applicationInfo = jsonObject.at("applicationInfo").get<ApplicationInfo>();
    bundleInfo.abilityInfos = jsonObject.at("abilityInfos").get<std::vector<AbilityInfo>>();
    bundleInfo.versionCode = jsonObject.at("versionCode").get<uint32_t>();
    bundleInfo.versionName = jsonObject.at("versionName").get<std::string>();
    bundleInfo.jointUserId = jsonObject.at("jointUserId").get<std::string>();
    bundleInfo.minSdkVersion = jsonObject.at("minSdkVersion").get<int32_t>();
    bundleInfo.maxSdkVersion = jsonObject.at("maxSdkVersion").get<int32_t>();
    bundleInfo.mainEntry = jsonObject.at("mainEntry").get<std::string>();
    bundleInfo.cpuAbi = jsonObject.at("cpuAbi").get<std::string>();
    bundleInfo.appId = jsonObject.at("appId").get<std::string>();
    bundleInfo.compatibleVersion = jsonObject.at("compatibleVersion").get<int>();
    bundleInfo.targetVersion = jsonObject.at("targetVersion").get<int>();
    bundleInfo.releaseType = jsonObject.at("releaseType").get<std::string>();
    bundleInfo.uid = jsonObject.at("uid").get<int>();
    bundleInfo.gid = jsonObject.at("gid").get<int>();
    bundleInfo.seInfo = jsonObject.at("seInfo").get<std::string>();
    bundleInfo.installTime = jsonObject.at("installTime").get<int64_t>();
    bundleInfo.updateTime = jsonObject.at("updateTime").get<int64_t>();
    bundleInfo.entryModuleName = jsonObject.at("entryModuleName").get<std::string>();
    bundleInfo.reqPermissions = jsonObject.at("reqPermissions").get<std::vector<std::string>>();
    bundleInfo.defPermissions = jsonObject.at("defPermissions").get<std::vector<std::string>>();
    bundleInfo.hapModuleNames = jsonObject.at("hapModuleNames").get<std::vector<std::string>>();
    bundleInfo.moduleNames = jsonObject.at("moduleNames").get<std::vector<std::string>>();
    bundleInfo.modulePublicDirs = jsonObject.at("modulePublicDirs").get<std::vector<std::string>>();
    bundleInfo.moduleDirs = jsonObject.at("moduleDirs").get<std::vector<std::string>>();
    bundleInfo.moduleResPaths = jsonObject.at("moduleResPaths").get<std::vector<std::string>>();
}

}  // namespace AppExecFwk
}  // namespace OHOS