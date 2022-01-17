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

#include <cerrno>
#include <fstream>
#include <unistd.h>

#include "ability_info.h"
#include "app_log_wrapper.h"
#include "bundle_constants.h"
#include "bundle_mgr_interface.h"
#include "iservice_registry.h"
#include "locale_config.h"
#include "nlohmann/json.hpp"
#include "system_ability_definition.h"

using namespace OHOS::Global::Resource;
using namespace OHOS::Global::I18n;

namespace OHOS {
namespace AppExecFwk {
BundleMgrClient::BundleMgrClient()
{
    APP_LOGI("enter");
}

BundleMgrClient::~BundleMgrClient()
{
    APP_LOGI("enter");
}

bool BundleMgrClient::GetBundleNameForUid(const int uid, std::string &bundleName)
{
    APP_LOGI("enter");

    ErrCode result = Connect();
    if (result != ERR_OK) {
        APP_LOGE("failed to connect");
        return false;
    }

    return bundleMgr_->GetBundleNameForUid(uid, bundleName);
}

bool BundleMgrClient::GetBundleInfo(const std::string &bundleName, const BundleFlag flag, BundleInfo &bundleInfo)
{
    APP_LOGI("enter");

    ErrCode result = Connect();
    if (result != ERR_OK) {
        APP_LOGE("failed to connect");
        return false;
    }

    return bundleMgr_->GetBundleInfo(bundleName, flag, bundleInfo);
}

bool BundleMgrClient::GetHapModuleInfo(const std::string &bundleName, const std::string &hapName,
    HapModuleInfo &hapModuleInfo)
{
    ErrCode result = Connect();
    if (result != ERR_OK) {
        APP_LOGE("failed to connect");
        return false;
    }

    AbilityInfo info;
    info.bundleName = bundleName;
    info.package = hapName;
    return bundleMgr_->GetHapModuleInfo(info, hapModuleInfo);
}

bool BundleMgrClient::GetResConfigFile(const HapModuleInfo &hapModuleInfo, const std::string &metadataName,
    std::vector<std::string> &profileInfos) const
{
    std::vector<Metadata> data = hapModuleInfo.metadata;
    std::string resourcePath = hapModuleInfo.resourcePath;
    if (!GetResProfileByMetadata(data, metadataName, resourcePath, profileInfos)) {
        APP_LOGE("GetResProfileByMetadata failed");
        return false;
    }
    if (profileInfos.empty()) {
        APP_LOGE("no valid file can be obtained");
        return false;
    }
    return true;
}

bool BundleMgrClient::GetResConfigFile(const ExtensionAbilityInfo &extensionInfo, const std::string &metadataName,
    std::vector<std::string> &profileInfos) const
{
    std::vector<Metadata> data = extensionInfo.metadata;
    std::string resourcePath = extensionInfo.resourcePath;
    if (!GetResProfileByMetadata(data, metadataName, resourcePath, profileInfos)) {
        APP_LOGE("GetResProfileByMetadata failed");
        return false;
    }
    if (profileInfos.empty()) {
        APP_LOGE("no valid file can be obtained");
        return false;
    }
    return true;
}

bool BundleMgrClient::GetResConfigFile(const AbilityInfo &abilityInfo, const std::string &metadataName,
    std::vector<std::string> &profileInfos) const
{
    std::vector<Metadata> data = abilityInfo.metadata;
    std::string resourcePath = abilityInfo.resourcePath;
    if (!GetResProfileByMetadata(data, metadataName, resourcePath, profileInfos)) {
        APP_LOGE("GetResProfileByMetadata failed");
        return false;
    }
    if (profileInfos.empty()) {
        APP_LOGE("no valid file can be obtained");
        return false;
    }
    return true;
}

bool BundleMgrClient::GetResProfileByMetadata(const std::vector<Metadata> &metadata, const std::string &metadataName,
    const std ::string &resourcePath, std::vector<std::string> &profileInfos) const
{
    if (metadata.empty() || resourcePath.empty()) {
        APP_LOGE("GetResProfileByMetadata failed due to invalid params");
        return false;
    }
    std::shared_ptr<ResourceManager> resMgr = InitResMgr(resourcePath);
    if (resMgr == nullptr) {
        APP_LOGE("GetResProfileByMetadata init resMgr failed");
        return false;
    }

    if (metadataName.empty()) {
        for_each(metadata.begin(), metadata.end(), [this, &resMgr, &profileInfos](const Metadata& data)->void {
            if (!GetResFromResMgr(data.resource, resMgr, profileInfos)) {
                APP_LOGW("GetResFromResMgr failed");
            }
        });
    } else {
        for_each(metadata.begin(), metadata.end(),
            [this, &resMgr, &metadataName, &profileInfos](const Metadata& data)->void {
            if ((metadataName.compare(data.name) == 0) && (!GetResFromResMgr(data.resource, resMgr, profileInfos))) {
                APP_LOGW("GetResFromResMgr failed");
            }
        });
    }

    return true;
}

std::shared_ptr<ResourceManager> BundleMgrClient::InitResMgr(const std::string &resourcePath) const
{
    APP_LOGD("InitResMgr begin");
    if (resourcePath.empty()) {
        APP_LOGE("InitResMgr failed due to invalid param");
        return nullptr;
    }
    std::shared_ptr<ResourceManager> resMgr(CreateResourceManager());
    if (!resMgr) {
        APP_LOGE("InitResMgr resMgr is nullptr");
        return nullptr;
    }

    std::unique_ptr<ResConfig> resConfig(CreateResConfig());
    if (!resConfig) {
        APP_LOGE("InitResMgr resConfig is nullptr");
        return nullptr;
    }
    resConfig->SetLocaleInfo(LocaleConfig::GetSystemLanguage().c_str(), LocaleConfig::GetSystemLocale().c_str(),
        LocaleConfig::GetSystemRegion().c_str());
    resMgr->UpdateResConfig(*resConfig);

    APP_LOGD("resourcePath is %{public}s", resourcePath.c_str());
    if (!resourcePath.empty() && !resMgr->AddResource(resourcePath.c_str())) {
        APP_LOGE("InitResMgr AddResource failed");
        return nullptr;
    }
    return resMgr;
}

bool BundleMgrClient::GetResFromResMgr(const std::string &resName, const std::shared_ptr<ResourceManager> &resMgr,
    std::vector<std::string> &profileInfos) const
{
    APP_LOGD("GetResFromResMgr begin");
    if (resName.empty()) {
        APP_LOGE("GetResFromResMgr res name is empty");
        return false;
    }

    size_t pos = resName.rfind(Constants::PROFILE_FILE_COLON);
    if ((pos == std::string::npos) || (pos == resName.length() - 1)) {
        APP_LOGE("GetResFromResMgr res name is invalid");
        return false;
    }
    std::string profileName = resName.substr(pos + 1);
    std::string resPath;
    if (resMgr->GetProfileByName(profileName.c_str(), resPath) != SUCCESS) {
        APP_LOGE("GetResFromResMgr profileName cannot be found");
        return false;
    }
    APP_LOGD("GetResFromResMgr resPath is %{public}s", resPath.c_str());
    std::string profile;
    if (!TransformFileToJsonString(resPath, profile)) {
        return false;
    }
    profileInfos.emplace_back(profile);
    return true;
}

bool BundleMgrClient::IsFileExisted(const std::string &filePath, const std::string &suffix) const
{
    if (filePath.empty()) {
        APP_LOGE("the file is not existed due to empty file path");
        return false;
    }

    auto position = filePath.rfind('.');
    if (position == std::string::npos) {
        APP_LOGE("filePath no suffix");
        return false;
    }

    std::string suffixStr = filePath.substr(position);
    if (LowerStr(suffixStr) != suffix) {
        APP_LOGE("file is not json");
        return false;
    }

    if (access(filePath.c_str(), F_OK) != 0) {
        APP_LOGE("can not access the file: %{private}s", filePath.c_str());
        return false;
    }
    return true;
}

bool BundleMgrClient::TransformFileToJsonString(const std::string &resPath, std::string &profile) const
{
    if (!IsFileExisted(resPath, Constants::PROFILE_FILE_SUFFIX)) {
        APP_LOGE("the file is not existed");
        return false;
    }
    std::fstream in;
    in.open(resPath, std::ios_base::in | std::ios_base::binary);
    if (!in.is_open()) {
        APP_LOGE("the file cannot be open due to %{public}s", strerror(errno));
        return false;
    }
    in.seekg(0, std::ios::end);
    size_t size = in.tellg();
    if (size == 0) {
        APP_LOGE("the file is an empty file");
        in.close();
        return false;
    }
    in.seekg(0, std::ios::beg);
    nlohmann::json profileJson = nlohmann::json::parse(in, nullptr, false);
    if (profileJson.is_discarded()) {
        APP_LOGE("bad profile file");
        in.close();
        return false;
    }
    profile = profileJson.dump(Constants::DUMP_INDENT);
    in.close();
    return true;
}

ErrCode BundleMgrClient::Connect()
{
    APP_LOGI("enter");

    std::lock_guard<std::mutex> lock(mutex_);
    if (bundleMgr_ == nullptr) {
        sptr<ISystemAbilityManager> systemAbilityManager =
            SystemAbilityManagerClient::GetInstance().GetSystemAbilityManager();
        if (systemAbilityManager == nullptr) {
            APP_LOGE("failed to get system ability manager");
            return ERR_APPEXECFWK_SERVICE_NOT_CONNECTED;
        }

        sptr<IRemoteObject> remoteObject_ = systemAbilityManager->GetSystemAbility(BUNDLE_MGR_SERVICE_SYS_ABILITY_ID);
        if (remoteObject_ == nullptr || (bundleMgr_ = iface_cast<IBundleMgr>(remoteObject_)) == nullptr) {
            APP_LOGE("failed to get bundle mgr service remote object");
            return ERR_APPEXECFWK_SERVICE_NOT_CONNECTED;
        }
    }

    APP_LOGI("end");

    return ERR_OK;
}
}  // namespace AppExecFwk
}  // namespace OHOS
