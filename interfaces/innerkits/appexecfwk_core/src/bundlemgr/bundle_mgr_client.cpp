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
#include "bundle_mgr_client.h"

#include "ability_info.h"
#include "app_log_wrapper.h"
#include "bundle_mgr_client_impl.h"
#include "extension_ability_info.h"
#include "hap_module_info.h"

namespace OHOS {
namespace AppExecFwk {
BundleMgrClient::BundleMgrClient()
{
    APP_LOGI("create BundleMgrClient");
    impl_ = std::make_shared<BundleMgrClientImpl>();
}

BundleMgrClient::~BundleMgrClient()
{
    APP_LOGI("destory BundleMgrClient");
}

bool BundleMgrClient::GetBundleNameForUid(const int uid, std::string &bundleName)
{
    return impl_->GetBundleNameForUid(uid, bundleName);
}

bool BundleMgrClient::GetBundleInfo(const std::string &bundleName, const BundleFlag flag, BundleInfo &bundleInfo)
{
    return impl_->GetBundleInfo(bundleName, flag, bundleInfo);
}

bool BundleMgrClient::GetHapModuleInfo(const std::string &bundleName, const std::string &hapName,
    HapModuleInfo &hapModuleInfo)
{
    return impl_->GetHapModuleInfo(bundleName, hapName, hapModuleInfo);
}

bool BundleMgrClient::GetResConfigFile(const HapModuleInfo &hapModuleInfo, const std::string &metadataName,
    std::vector<std::string> &profileInfos) const
{
    return impl_->GetResConfigFile(hapModuleInfo, metadataName, profileInfos);
}

bool BundleMgrClient::GetResConfigFile(const ExtensionAbilityInfo &extensionInfo, const std::string &metadataName,
    std::vector<std::string> &profileInfos) const
{
    return impl_->GetResConfigFile(extensionInfo, metadataName, profileInfos);
}

bool BundleMgrClient::GetResConfigFile(const AbilityInfo &abilityInfo, const std::string &metadataName,
    std::vector<std::string> &profileInfos) const
{
    return impl_->GetResConfigFile(abilityInfo, metadataName, profileInfos);
}
}  // namespace AppExecFwk
}  // namespace OHOS
