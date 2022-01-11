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

#include "distributed_bundle_info.h"

#include "nlohmann/json.hpp"
#include "parcel_macro.h"
#include "string_ex.h"

#include "app_log_wrapper.h"
#include "json_util.h"

namespace OHOS {
namespace AppExecFwk {
namespace {
const std::string JSON_KEY_VERSION = "version";
const std::string JSON_KEY_NAME = "name";
const std::string JSON_KEY_VERSION_CODE = "versionCode";
const std::string JSON_KEY_COMPATIBLE_VERSION_CODE = "compatibleVersionCode";
const std::string JSON_KEY_VERSION_NAME = "versionName";
const std::string JSON_KEY_MIN_COMPATIBLE_VERSION = "minCompatibleVersion";
const std::string JSON_KEY_TARGET_VERSION_CODE = "targetVersionCode";
const std::string JSON_KEY_APP_ID = "appId";
const std::string JSON_KEY_MAIN_ABILITY = "mainAbility";
const std::string JSON_KEY_USER_INFOS = "bundleUserInfos";
const std::string JSON_KEY_USER_ID = "userId";
const std::string JSON_KEY_ENABLE = "enable";
const std::string JSON_KEY_ENABLE_ABILITIES = "enableAbilities";
}
bool DistributedBundleInfo::ReadFromParcel(Parcel &parcel)
{
    version = parcel.ReadUint32();
    versionCode = parcel.ReadUint32();
    compatibleVersionCode = parcel.ReadUint32();
    minCompatibleVersion = parcel.ReadUint32();
    targetVersionCode = parcel.ReadUint32();
    name = Str16ToStr8(parcel.ReadString16());
    versionName = Str16ToStr8(parcel.ReadString16());
    appId = Str16ToStr8(parcel.ReadString16());
    mainAbility = Str16ToStr8(parcel.ReadString16());

    int32_t bundleUserInfosSize;
    READ_PARCEL_AND_RETURN_FALSE_IF_FAIL(Int32, parcel, bundleUserInfosSize);
    for (int32_t i = 0; i < bundleUserInfosSize; i++) {
        std::unique_ptr<BundleUserInfo> userInfo(parcel.ReadParcelable<BundleUserInfo>());
        if (!userInfo) {
            APP_LOGE("ReadParcelable<UserInfo> failed");
            return false;
        }
        bundleUserInfos.emplace_back(*userInfo);
    }
    return true;
}

bool DistributedBundleInfo::Marshalling(Parcel &parcel) const
{
    WRITE_PARCEL_AND_RETURN_FALSE_IF_FAIL(Uint32, parcel, version);
    WRITE_PARCEL_AND_RETURN_FALSE_IF_FAIL(Uint32, parcel, versionCode);
    WRITE_PARCEL_AND_RETURN_FALSE_IF_FAIL(Uint32, parcel, compatibleVersionCode);
    WRITE_PARCEL_AND_RETURN_FALSE_IF_FAIL(Uint32, parcel, minCompatibleVersion);
    WRITE_PARCEL_AND_RETURN_FALSE_IF_FAIL(Uint32, parcel, targetVersionCode);
    WRITE_PARCEL_AND_RETURN_FALSE_IF_FAIL(String16, parcel, Str8ToStr16(name));
    WRITE_PARCEL_AND_RETURN_FALSE_IF_FAIL(String16, parcel, Str8ToStr16(versionName));
    WRITE_PARCEL_AND_RETURN_FALSE_IF_FAIL(String16, parcel, Str8ToStr16(appId));
    WRITE_PARCEL_AND_RETURN_FALSE_IF_FAIL(String16, parcel, Str8ToStr16(mainAbility));

    WRITE_PARCEL_AND_RETURN_FALSE_IF_FAIL(Int32, parcel, bundleUserInfos.size());
    for (auto &userInfo : bundleUserInfos) {
        WRITE_PARCEL_AND_RETURN_FALSE_IF_FAIL(Parcelable, parcel, &userInfo);
    }
    return true;
}

DistributedBundleInfo *DistributedBundleInfo::Unmarshalling(Parcel &parcel)
{
    DistributedBundleInfo *info = new (std::nothrow) DistributedBundleInfo();
    if (info && !info->ReadFromParcel(parcel)) {
        APP_LOGW("read from parcel failed");
        delete info;
        info = nullptr;
    }
    return info;
}

std::string DistributedBundleInfo::ToString() const
{
    nlohmann::json jsonObject;
    jsonObject[JSON_KEY_VERSION] = version;
    jsonObject[JSON_KEY_NAME] = name;
    jsonObject[JSON_KEY_VERSION_CODE] = versionCode;
    jsonObject[JSON_KEY_VERSION_NAME] = versionName;
    jsonObject[JSON_KEY_COMPATIBLE_VERSION_CODE] = compatibleVersionCode;
    jsonObject[JSON_KEY_MIN_COMPATIBLE_VERSION] = minCompatibleVersion;
    jsonObject[JSON_KEY_TARGET_VERSION_CODE] = targetVersionCode;
    jsonObject[JSON_KEY_APP_ID] = appId;
    jsonObject[JSON_KEY_MAIN_ABILITY] = mainAbility;
    jsonObject[JSON_KEY_USER_INFOS] = bundleUserInfos;
    return jsonObject.dump();
}

bool DistributedBundleInfo::FromJsonString(const std::string &jsonString)
{
    nlohmann::json jsonObject = nlohmann::json::parse(jsonString, nullptr, false);
    if (jsonObject.is_discarded()) {
        APP_LOGE("failed to parse DistributedBundleInfo: %{public}s.", jsonString.c_str());
        return false;
    }

    const auto &jsonObjectEnd = jsonObject.end();
    int32_t parseResult = ERR_OK;
    GetValueIfFindKey<uint32_t>(jsonObject,
        jsonObjectEnd,
        JSON_KEY_VERSION,
        version,
        JsonType::NUMBER,
        false,
        parseResult,
        ArrayType::NOT_ARRAY);
    GetValueIfFindKey<std::string>(jsonObject,
        jsonObjectEnd,
        JSON_KEY_NAME,
        name,
        JsonType::STRING,
        false,
        parseResult,
        ArrayType::NOT_ARRAY);
    GetValueIfFindKey<uint32_t>(jsonObject,
        jsonObjectEnd,
        JSON_KEY_VERSION_CODE,
        versionCode,
        JsonType::NUMBER,
        false,
        parseResult,
        ArrayType::NOT_ARRAY);
    GetValueIfFindKey<uint32_t>(jsonObject,
        jsonObjectEnd,
        JSON_KEY_COMPATIBLE_VERSION_CODE,
        compatibleVersionCode,
        JsonType::NUMBER,
        false,
        parseResult,
        ArrayType::NOT_ARRAY);
    GetValueIfFindKey<std::string>(jsonObject,
        jsonObjectEnd,
        JSON_KEY_VERSION_NAME,
        versionName,
        JsonType::STRING,
        false,
        parseResult,
        ArrayType::NOT_ARRAY);
    GetValueIfFindKey<uint32_t>(jsonObject,
        jsonObjectEnd,
        JSON_KEY_MIN_COMPATIBLE_VERSION,
        minCompatibleVersion,
        JsonType::NUMBER,
        false,
        parseResult,
        ArrayType::NOT_ARRAY);
    GetValueIfFindKey<uint32_t>(jsonObject,
        jsonObjectEnd,
        JSON_KEY_TARGET_VERSION_CODE,
        targetVersionCode,
        JsonType::NUMBER,
        false,
        parseResult,
        ArrayType::NOT_ARRAY);
    GetValueIfFindKey<std::string>(jsonObject,
        jsonObjectEnd,
        JSON_KEY_APP_ID,
        appId,
        JsonType::STRING,
        false,
        parseResult,
        ArrayType::NOT_ARRAY);
    GetValueIfFindKey<std::string>(jsonObject,
        jsonObjectEnd,
        JSON_KEY_MAIN_ABILITY,
        mainAbility,
        JsonType::STRING,
        false,
        parseResult,
        ArrayType::NOT_ARRAY);
    GetValueIfFindKey<std::vector<BundleUserInfo>>(jsonObject,
        jsonObjectEnd,
        JSON_KEY_USER_INFOS,
        bundleUserInfos,
        JsonType::ARRAY,
        false,
        parseResult,
        ArrayType::OBJECT);
    return parseResult == ERR_OK;
}
}  // namespace AppExecFwk
}  // namespace OHOS
