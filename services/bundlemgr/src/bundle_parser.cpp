/*
 * Copyright (c) 2021-2022 Huawei Device Co., Ltd.
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

#include "bundle_parser.h"

#include <sstream>

#include "app_log_wrapper.h"
#include "bundle_constants.h"
#include "bundle_extractor.h"
#include "bundle_profile.h"
#include "module_profile.h"
#include "rpcid_decode/syscap_tool.h"
#include "securec.h"

namespace OHOS {
namespace AppExecFwk {
namespace {
bool ParseStr(const char *buf, const int itemLen, int totalLen, std::vector<std::string> &sysCaps)
{
    APP_LOGD("Parse rpcid output start, itemLen:%{public}d  totalLen:%{public}d.", itemLen, totalLen);
    if (buf == nullptr || itemLen <= 0 || totalLen <= 0) {
        APP_LOGE("param invalid.");
        return false;
    }

    int index = 0;
    while (index + itemLen <= totalLen) {
        char item[itemLen];
        if (strncpy_s(item, sizeof(item), buf + index, itemLen) != 0) {
            APP_LOGE("Parse rpcid failed due to strncpy_s error.");
            return false;
        }

        sysCaps.emplace_back((std::string)item, 0, itemLen);
        index += itemLen;
    }

    return true;
}
}
ErrCode BundleParser::Parse(const std::string &pathName, InnerBundleInfo &innerBundleInfo) const
{
    APP_LOGI("parse from %{public}s", pathName.c_str());
    BundleExtractor bundleExtractor(pathName);
    if (!bundleExtractor.Init()) {
        APP_LOGE("bundle extractor init failed");
        return ERR_APPEXECFWK_PARSE_UNEXPECTED;
    }

    std::ostringstream outStream;
    if (!bundleExtractor.ExtractProfile(outStream)) {
        APP_LOGE("extract profile file failed");
        return ERR_APPEXECFWK_PARSE_NO_PROFILE;
    }

    if (bundleExtractor.IsNewVersion()) {
        APP_LOGD("module.json transform to InnerBundleInfo");
        innerBundleInfo.SetIsNewVersion(true);
        ModuleProfile moduleProfile;
        return moduleProfile.TransformTo(outStream, bundleExtractor, innerBundleInfo);
    }
    APP_LOGD("config.json transform to InnerBundleInfo");
    innerBundleInfo.SetIsNewVersion(false);
    BundleProfile bundleProfile;
    ErrCode ret = bundleProfile.TransformTo(outStream, bundleExtractor, innerBundleInfo);
    auto& abilityInfos = innerBundleInfo.FetchAbilityInfos();
    for (auto& info : abilityInfos) {
        info.second.isStageBasedModel = bundleExtractor.IsStageBasedModel(info.second.name);
        auto iter = innerBundleInfo.FetchInnerModuleInfos().find(info.second.package);
        if (iter != innerBundleInfo.FetchInnerModuleInfos().end()) {
            iter->second.isStageBasedModel = info.second.isStageBasedModel;
        }
    }
    return ret;
}

ErrCode BundleParser::ParseSysCap(const std::string &pathName, std::vector<std::string> &sysCaps) const
{
    APP_LOGD("Parse sysCaps from %{public}s", pathName.c_str());
    BundleExtractor bundleExtractor(pathName);
    if (!bundleExtractor.Init()) {
        APP_LOGE("Bundle extractor init failed");
        return ERR_APPEXECFWK_PARSE_UNEXPECTED;
    }

    if (!bundleExtractor.HasEntry(Constants::SYSCAP_NAME)) {
        APP_LOGD("Rpcid.sc is not exist, and do not need verification sysCaps.");
        return ERR_OK;
    }

    std::stringstream rpcidStream;
    if (!bundleExtractor.ExtractByName(Constants::SYSCAP_NAME, rpcidStream)) {
        APP_LOGE("Extract rpcid file failed");
        return ERR_APPEXECFWK_PARSE_RPCID_FAILED;
    }

    int32_t rpcidLen = rpcidStream.tellp();
    if (rpcidLen < 0) {
        return ERR_APPEXECFWK_PARSE_UNEXPECTED;
    }
    char rpcidBuf[rpcidLen];
    rpcidStream.read(rpcidBuf, rpcidLen);
    uint32_t outLen;
    char *outBuffer;
    int result = RPCIDStreamDecodeToBuffer(rpcidBuf, rpcidLen, &outBuffer, &outLen);
    if (result != 0) {
        APP_LOGE("Decode syscaps failed");
        return ERR_APPEXECFWK_PARSE_RPCID_FAILED;
    }

    if (!ParseStr(outBuffer, SINGLE_SYSCAP_LENGTH, outLen, sysCaps)) {
        APP_LOGE("Parse syscaps str failed");
        return ERR_APPEXECFWK_PARSE_RPCID_FAILED;
    }

    APP_LOGE("Parse sysCaps str success");
    return ERR_OK;
}
}  // namespace AppExecFwk
}  // namespace OHOS
