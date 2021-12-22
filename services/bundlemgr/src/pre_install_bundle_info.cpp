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

#include "pre_install_bundle_info.h"

#include "common_profile.h"

namespace OHOS {
namespace AppExecFwk {
namespace {
const std::string BUNDLE_NAME = "bundleName";
const std::string BUNDLE_PATH = "bundlePath";
}  // namespace

void PreInstallBundleInfo::ToJson(nlohmann::json &jsonObject) const
{
    jsonObject[BUNDLE_NAME] = bundleName_;
    jsonObject[BUNDLE_PATH] = bundlePath_;
}

int32_t PreInstallBundleInfo::FromJson(const nlohmann::json &jsonObject)
{
    const auto &jsonObjectEnd = jsonObject.end();
    GetValueIfFindKey<std::string>(jsonObject,
        jsonObjectEnd,
        BUNDLE_NAME,
        bundleName_,
        JsonType::STRING,
        true,
        ProfileReader::parseResult,
        ArrayType::NOT_ARRAY);
    GetValueIfFindKey<std::string>(jsonObject,
        jsonObjectEnd,
        BUNDLE_PATH,
        bundlePath_,
        JsonType::STRING,
        true,
        ProfileReader::parseResult,
        ArrayType::NOT_ARRAY);
    int32_t ret = ProfileReader::parseResult;
    // need recover parse result to ERR_OK
    ProfileReader::parseResult = ERR_OK;
    return ret;
}

std::string PreInstallBundleInfo::ToString() const
{
    nlohmann::json j;
    j[BUNDLE_NAME] = bundleName_;
    j[BUNDLE_PATH] = bundlePath_;
    return j.dump();
}
}  // namespace AppExecFwk
}  // namespace OHOS
