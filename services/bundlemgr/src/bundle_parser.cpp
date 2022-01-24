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

#include "bundle_parser.h"

#include <sstream>

#include "app_log_wrapper.h"
#include "bundle_constants.h"
#include "bundle_extractor.h"
#include "bundle_profile.h"
#include "module_profile.h"

namespace OHOS {
namespace AppExecFwk {

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
        return moduleProfile.TransformTo(outStream, innerBundleInfo);
    }
    APP_LOGD("config.json transform to InnerBundleInfo");
    innerBundleInfo.SetIsNewVersion(false);
    BundleProfile bundleProfile;
    ErrCode ret = bundleProfile.TransformTo(outStream, innerBundleInfo);
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

}  // namespace AppExecFwk
}  // namespace OHOS
