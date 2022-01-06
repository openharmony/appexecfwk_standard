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

#include "bundle_mgr_host_impl.h"

#include <dirent.h>
#include <future>

#include "app_log_wrapper.h"
#include "bundle_mgr_service.h"
#include "bundle_parser.h"
#include "bundle_permission_mgr.h"
#include "bundle_util.h"
#include "directory_ex.h"
#include "installd_client.h"
#include "json_serializer.h"

namespace OHOS {
namespace AppExecFwk {

bool BundleMgrHostImpl::GetApplicationInfo(
    const std::string &appName, const ApplicationFlag flag, const int userId, ApplicationInfo &appInfo)
{
    return GetApplicationInfo(appName, static_cast<int32_t>(flag), userId, appInfo);
}

bool BundleMgrHostImpl::GetApplicationInfo(
    const std::string &appName, int32_t flags, int32_t userId, ApplicationInfo &appInfo)
{
    auto dataMgr = GetDataMgrFromService();
    if (dataMgr == nullptr) {
        APP_LOGE("DataMgr is nullptr");
        return false;
    }
    return dataMgr->GetApplicationInfo(appName, flags, userId, appInfo);
}

bool BundleMgrHostImpl::GetApplicationInfos(
    const ApplicationFlag flag, const int userId, std::vector<ApplicationInfo> &appInfos)
{
    return GetApplicationInfos(static_cast<int32_t>(flag), userId, appInfos);
}

bool BundleMgrHostImpl::GetApplicationInfos(
    int32_t flags, int32_t userId, std::vector<ApplicationInfo> &appInfos)
{
    auto dataMgr = GetDataMgrFromService();
    if (dataMgr == nullptr) {
        APP_LOGE("DataMgr is nullptr");
        return false;
    }
    return dataMgr->GetApplicationInfos(flags, userId, appInfos);
}

bool BundleMgrHostImpl::GetBundleInfo(
    const std::string &bundleName, const BundleFlag flag, BundleInfo &bundleInfo, int32_t userId)
{
    return GetBundleInfo(bundleName, static_cast<int32_t>(flag), bundleInfo, userId);
}

bool BundleMgrHostImpl::GetBundleInfo(
    const std::string &bundleName, int32_t flags, BundleInfo &bundleInfo, int32_t userId)
{
    auto dataMgr = GetDataMgrFromService();
    if (dataMgr == nullptr) {
        APP_LOGE("DataMgr is nullptr");
        return false;
    }
    return dataMgr->GetBundleInfo(bundleName, flags, bundleInfo, userId);
}

bool BundleMgrHostImpl::GetBundleUserInfos(
    const std::string &bundleName, int32_t userId, std::vector<InnerBundleUserInfo> &innerBundleUserInfo)
{
    auto dataMgr = GetDataMgrFromService();
    if (dataMgr == nullptr) {
        APP_LOGE("DataMgr is nullptr");
        return false;
    }
    return dataMgr->GetInnerBundleUserInfosByUserId(bundleName, userId, innerBundleUserInfo);
}

bool BundleMgrHostImpl::GetBundleInfos(const BundleFlag flag, std::vector<BundleInfo> &bundleInfos, int32_t userId)
{
    return GetBundleInfos(static_cast<int32_t>(flag), bundleInfos, userId);
}

bool BundleMgrHostImpl::GetBundleInfos(int32_t flags, std::vector<BundleInfo> &bundleInfos, int32_t userId)
{
    auto dataMgr = GetDataMgrFromService();
    if (dataMgr == nullptr) {
        APP_LOGE("DataMgr is nullptr");
        return false;
    }
    return dataMgr->GetBundleInfos(flags, bundleInfos, userId);
}

bool BundleMgrHostImpl::GetBundleNameForUid(const int uid, std::string &bundleName)
{
    auto dataMgr = GetDataMgrFromService();
    if (dataMgr == nullptr) {
        APP_LOGE("DataMgr is nullptr");
        return false;
    }
    return dataMgr->GetBundleNameForUid(uid, bundleName);
}

bool BundleMgrHostImpl::GetBundlesForUid(const int uid, std::vector<std::string> &bundleNames)
{
    auto dataMgr = GetDataMgrFromService();
    if (dataMgr == nullptr) {
        APP_LOGE("DataMgr is nullptr");
        return false;
    }
    return dataMgr->GetBundlesForUid(uid, bundleNames);
}

bool BundleMgrHostImpl::GetNameForUid(const int uid, std::string &name)
{
    auto dataMgr = GetDataMgrFromService();
    if (dataMgr == nullptr) {
        APP_LOGE("DataMgr is nullptr");
        return false;
    }
    return dataMgr->GetNameForUid(uid, name);
}

bool BundleMgrHostImpl::GetBundleGids(const std::string &bundleName, std::vector<int> &gids)
{
    auto dataMgr = GetDataMgrFromService();
    if (dataMgr == nullptr) {
        APP_LOGE("DataMgr is nullptr");
        return false;
    }
    return dataMgr->GetBundleGids(bundleName, gids);
}

bool BundleMgrHostImpl::GetBundleGidsByUid(const std::string &bundleName, const int &uid, std::vector<int> &gids)
{
    auto dataMgr = GetDataMgrFromService();
    if (dataMgr == nullptr) {
        APP_LOGE("DataMgr is nullptr");
        return false;
    }
    return dataMgr->GetBundleGidsByUid(bundleName, uid, gids);
}

bool BundleMgrHostImpl::CheckIsSystemAppByUid(const int uid)
{
    auto dataMgr = GetDataMgrFromService();
    if (dataMgr == nullptr) {
        APP_LOGE("DataMgr is nullptr");
        return false;
    }
    return dataMgr->CheckIsSystemAppByUid(uid);
}

bool BundleMgrHostImpl::GetBundleInfosByMetaData(const std::string &metaData, std::vector<BundleInfo> &bundleInfos)
{
    auto dataMgr = GetDataMgrFromService();
    if (dataMgr == nullptr) {
        APP_LOGE("DataMgr is nullptr");
        return false;
    }
    return dataMgr->GetBundleInfosByMetaData(metaData, bundleInfos);
}

bool BundleMgrHostImpl::QueryAbilityInfo(const Want &want, AbilityInfo &abilityInfo)
{
    return QueryAbilityInfo(want, GET_ABILITY_INFO_WITH_APPLICATION, Constants::UNSPECIFIED_USERID, abilityInfo);
}

bool BundleMgrHostImpl::QueryAbilityInfo(const Want &want, int32_t flags, int32_t userId, AbilityInfo &abilityInfo)
{
    auto dataMgr = GetDataMgrFromService();
    if (dataMgr == nullptr) {
        APP_LOGE("DataMgr is nullptr");
        return false;
    }
    return dataMgr->QueryAbilityInfo(want, flags, userId, abilityInfo);
}

bool BundleMgrHostImpl::QueryAbilityInfos(const Want &want, std::vector<AbilityInfo> &abilityInfos)
{
    return QueryAbilityInfos(want, GET_ABILITY_INFO_WITH_APPLICATION, 0, abilityInfos);
}

bool BundleMgrHostImpl::QueryAbilityInfos(
    const Want &want, int32_t flags, int32_t userId, std::vector<AbilityInfo> &abilityInfos)
{
    auto dataMgr = GetDataMgrFromService();
    if (dataMgr == nullptr) {
        APP_LOGE("DataMgr is nullptr");
        return false;
    }
    return dataMgr->QueryAbilityInfos(want, flags, userId, abilityInfos);
}

bool BundleMgrHostImpl::QueryAbilityInfosForClone(const Want &want, std::vector<AbilityInfo> &abilityInfos)
{
    auto dataMgr = GetDataMgrFromService();
    if (dataMgr == nullptr) {
        APP_LOGE("DataMgr is nullptr");
        return false;
    }
    return dataMgr->QueryAbilityInfosForClone(want, abilityInfos);
}

bool BundleMgrHostImpl::QueryAllAbilityInfos(const Want &want, int32_t userId, std::vector<AbilityInfo> &abilityInfos)
{
    auto dataMgr = GetDataMgrFromService();
    if (dataMgr == nullptr) {
        APP_LOGE("DataMgr is nullptr");
        return false;
    }
    return dataMgr->QueryLauncherAbilityInfos(want, userId, abilityInfos);
}

bool BundleMgrHostImpl::QueryAbilityInfoByUri(const std::string &abilityUri, AbilityInfo &abilityInfo)
{
    auto dataMgr = GetDataMgrFromService();
    if (dataMgr == nullptr) {
        APP_LOGE("DataMgr is nullptr");
        return false;
    }
    return dataMgr->QueryAbilityInfoByUri(abilityUri, abilityInfo);
}

bool BundleMgrHostImpl::QueryAbilityInfosByUri(const std::string &abilityUri, std::vector<AbilityInfo> &abilityInfos)
{
    auto dataMgr = GetDataMgrFromService();
    if (dataMgr == nullptr) {
        APP_LOGE("DataMgr is nullptr");
        return false;
    }
    return dataMgr->QueryAbilityInfosByUri(abilityUri, abilityInfos);
}

bool BundleMgrHostImpl::QueryKeepAliveBundleInfos(std::vector<BundleInfo> &bundleInfos)
{
    auto dataMgr = GetDataMgrFromService();
    if (dataMgr == nullptr) {
        APP_LOGE("DataMgr is nullptr");
        return false;
    }
    return dataMgr->QueryKeepAliveBundleInfos(bundleInfos);
}

std::string BundleMgrHostImpl::GetAbilityLabel(const std::string &bundleName, const std::string &className)
{
    auto dataMgr = GetDataMgrFromService();
    if (dataMgr == nullptr) {
        APP_LOGE("DataMgr is nullptr");
        return Constants::EMPTY_STRING;
    }
    return dataMgr->GetAbilityLabel(bundleName, className);
}

bool BundleMgrHostImpl::GetBundleArchiveInfo(
    const std::string &hapFilePath, const BundleFlag flag, BundleInfo &bundleInfo)
{
    return GetBundleArchiveInfo(hapFilePath, static_cast<int32_t>(flag), bundleInfo);
}

bool BundleMgrHostImpl::GetBundleArchiveInfo(
    const std::string &hapFilePath, int32_t flags, BundleInfo &bundleInfo)
{
    std::string realPath;
    auto ret = BundleUtil::CheckFilePath(hapFilePath, realPath);
    if (ret != ERR_OK) {
        APP_LOGE("GetBundleArchiveInfo file path %{public}s invalid", hapFilePath.c_str());
        return false;
    }
    InnerBundleInfo info;
    BundleParser bundleParser;
    ret = bundleParser.Parse(realPath, info);
    if (ret != ERR_OK) {
        APP_LOGE("parse bundle info failed, error: %{public}d", ret);
        return false;
    }
    info.GetBundleInfo(flags, bundleInfo);
    return true;
}

bool BundleMgrHostImpl::GetHapModuleInfo(const AbilityInfo &abilityInfo, HapModuleInfo &hapModuleInfo)
{
    if (abilityInfo.bundleName.empty() || abilityInfo.package.empty()) {
        APP_LOGE("fail to GetHapModuleInfo due to params empty");
        return false;
    }
    auto dataMgr = GetDataMgrFromService();
    if (dataMgr == nullptr) {
        APP_LOGE("DataMgr is nullptr");
        return false;
    }
    return dataMgr->GetHapModuleInfo(abilityInfo, hapModuleInfo);
}

bool BundleMgrHostImpl::GetLaunchWantForBundle(const std::string &bundleName, Want &want)
{
    auto dataMgr = GetDataMgrFromService();
    if (dataMgr == nullptr) {
        APP_LOGE("DataMgr is nullptr");
        return false;
    }
    return dataMgr->GetLaunchWantForBundle(bundleName, want);
}

int BundleMgrHostImpl::CheckPublicKeys(const std::string &firstBundleName, const std::string &secondBundleName)
{
    auto dataMgr = GetDataMgrFromService();
    if (dataMgr == nullptr) {
        APP_LOGE("DataMgr is nullptr");
        return false;
    }
    return dataMgr->CheckPublicKeys(firstBundleName, secondBundleName);
}

int BundleMgrHostImpl::CheckPermission(const std::string &bundleName, const std::string &permission)
{
    if (bundleName.empty() || permission.empty()) {
        APP_LOGE("fail to CheckPermission due to params empty");
        return Constants::PERMISSION_NOT_GRANTED;
    }
    return BundlePermissionMgr::VerifyPermission(bundleName, permission, Constants::DEFAULT_USERID);
}

int BundleMgrHostImpl::CheckPermissionByUid(
    const std::string &bundleName, const std::string &permission, const int userId)
{
    if (bundleName.empty() || permission.empty()) {
        APP_LOGE("fail to CheckPermission due to params empty");
        return Constants::PERMISSION_NOT_GRANTED;
    }
    return BundlePermissionMgr::VerifyPermission(bundleName, permission, userId);
}

bool BundleMgrHostImpl::GetPermissionDef(const std::string &permissionName, PermissionDef &permissionDef)
{
    if (permissionName.empty()) {
        APP_LOGE("fail to GetPermissionDef due to params empty");
        return false;
    }
    return BundlePermissionMgr::GetPermissionDef(permissionName, permissionDef);
}

bool BundleMgrHostImpl::GetAllPermissionGroupDefs(std::vector<PermissionDef> &permissionDefs)
{
    return true;
}

bool BundleMgrHostImpl::GetAppsGrantedPermissions(
    const std::vector<std::string> &permissions, std::vector<std::string> &appNames)
{
    return true;
}

bool BundleMgrHostImpl::HasSystemCapability(const std::string &capName)
{
    return true;
}

bool BundleMgrHostImpl::GetSystemAvailableCapabilities(std::vector<std::string> &systemCaps)
{
    return true;
}

bool BundleMgrHostImpl::IsSafeMode()
{
    return true;
}

bool BundleMgrHostImpl::TraverseCacheDirectory(const std::string& rootDir, std::vector<std::string>& cacheDirs)
{
    DIR* dir = opendir(rootDir.c_str());
    struct dirent *ptr = nullptr;
    if (dir == nullptr) {
        return false;
    }
    while ((ptr = readdir(dir)) != NULL) {
        if (strcmp(ptr->d_name, ".") == 0 || strcmp(ptr->d_name, "..") == 0) {
            continue;
        }
        if (ptr->d_type == DT_DIR && strcmp(ptr->d_name, "cache") == 0) {
            std::string currentDir = OHOS::IncludeTrailingPathDelimiter(rootDir) + std::string(ptr->d_name);
            cacheDirs.emplace_back(currentDir);
            continue;
        }
        if (ptr->d_type == DT_DIR) {
            std::string currentDir = OHOS::IncludeTrailingPathDelimiter(rootDir) + std::string(ptr->d_name);
            TraverseCacheDirectory(currentDir, cacheDirs);
        }
    }
    closedir(dir);

    return true;
}

bool BundleMgrHostImpl::CleanBundleCacheFiles(
    const std::string &bundleName, const sptr<ICleanCacheCallback> &cleanCacheCallback)
{
    if (bundleName.empty() || !cleanCacheCallback) {
        APP_LOGE("the cleanCacheCallback is nullptr or bundleName empty");
        return false;
    }
    ApplicationInfo applicationInfo;
    if (!GetApplicationInfo(bundleName, ApplicationFlag::GET_BASIC_APPLICATION_INFO, 0, applicationInfo)) {
        APP_LOGE("can not get application info of %{public}s", bundleName.c_str());
        return false;
    }
    std::string rootDir = applicationInfo.dataDir;
    auto cleanCache = [this, rootDir, cleanCacheCallback]() {
        std::vector<std::string> caches;
        bool result = this->TraverseCacheDirectory(rootDir, caches);
        bool error = false;
        if (result) {
            for (const auto& cache : caches) {
                error = InstalldClient::GetInstance()->CleanBundleDataDir(cache);
                if (error) {
                    break;
                }
            }
        }
        cleanCacheCallback->OnCleanCacheFinished(error);
    };
    handler_->PostTask(cleanCache);
    return true;
}

bool BundleMgrHostImpl::CleanBundleDataFiles(const std::string &bundleName, const int userId)
{
    if (bundleName.empty()) {
        APP_LOGE("the  bundleName empty");
        return false;
    }
    std::vector<ApplicationInfo> appInfos;
    if (!GetApplicationInfos(ApplicationFlag::GET_BASIC_APPLICATION_INFO, 0, appInfos)) {
        APP_LOGE("can not get application info of %{public}s", bundleName.c_str());
        return false;
    }
    bool result = false;
    for (auto applicationInfo : appInfos) {
        if (userId == Constants::C_UESRID) {
            if (applicationInfo.bundleName == bundleName && applicationInfo.isCloned == true) {
                result = true;
                if (InstalldClient::GetInstance()->CleanBundleDataDir(applicationInfo.dataDir) != ERR_OK) {
                    return false;
                }
                break;
            }
        } else {
            if (applicationInfo.bundleName == bundleName && applicationInfo.isCloned == false) {
                result = true;
                if (InstalldClient::GetInstance()->CleanBundleDataDir(applicationInfo.dataDir) != ERR_OK) {
                    return false;
                }
                break;
            }
        }
    }
    return result;
}

bool BundleMgrHostImpl::RegisterBundleStatusCallback(const sptr<IBundleStatusCallback> &bundleStatusCallback)
{
    if ((!bundleStatusCallback) || (bundleStatusCallback->GetBundleName().empty())) {
        APP_LOGE("the bundleStatusCallback is nullptr or bundleName empty");
        return false;
    }
    auto dataMgr = GetDataMgrFromService();
    if (dataMgr == nullptr) {
        APP_LOGE("DataMgr is nullptr");
        return false;
    }
    return dataMgr->RegisterBundleStatusCallback(bundleStatusCallback);
}

bool BundleMgrHostImpl::ClearBundleStatusCallback(const sptr<IBundleStatusCallback> &bundleStatusCallback)
{
    if (!bundleStatusCallback) {
        APP_LOGE("the bundleStatusCallback is nullptr");
        return false;
    }
    auto dataMgr = GetDataMgrFromService();
    if (dataMgr == nullptr) {
        APP_LOGE("DataMgr is nullptr");
        return false;
    }
    return dataMgr->ClearBundleStatusCallback(bundleStatusCallback);
}

bool BundleMgrHostImpl::UnregisterBundleStatusCallback()
{
    auto dataMgr = GetDataMgrFromService();
    if (dataMgr == nullptr) {
        APP_LOGE("DataMgr is nullptr");
        return false;
    }
    return dataMgr->UnregisterBundleStatusCallback();
}

bool BundleMgrHostImpl::DumpInfos(
    const DumpFlag flag, const std::string &bundleName, int32_t userId, std::string &result)
{
    auto dataMgr = GetDataMgrFromService();
    if (dataMgr == nullptr) {
        APP_LOGE("DataMgr is nullptr");
        return false;
    }
    bool ret = false;
    switch (flag) {
        case DumpFlag::DUMP_BUNDLE_LIST: {
            std::vector<std::string> bundleNames;
            ret = dataMgr->GetBundleList(bundleNames, userId);
            if (ret) {
                for (const auto &name : bundleNames) {
                    result.append(name);
                    result.append("\n");
                }
                APP_LOGI("get installed bundles success");
            }
            break;
        }
        case DumpFlag::DUMP_ALL_BUNDLE_INFO: {
            std::vector<BundleInfo> bundleInfos;
            ret = GetBundleInfos(
                BundleFlag::GET_BUNDLE_WITH_ABILITIES, bundleInfos, userId);
            if (ret) {
                for (const auto &info : bundleInfos) {
                    std::vector<InnerBundleUserInfo> innerBundleUserInfos;
                    if (!GetBundleUserInfos(info.name, userId, innerBundleUserInfos)) {
                        APP_LOGI("get all userInfo in bundle(%{public}s) failed", info.name.c_str());
                    }

                    result.append(info.name);
                    result.append(":\n");
                    nlohmann::json jsonObject = info;
                    jsonObject["hapModuleInfos"] = info.hapModuleInfos;
                    jsonObject["innerBundleUserInfos"] = innerBundleUserInfos;
                    result.append(jsonObject.dump(Constants::DUMP_INDENT));
                    result.append("\n");
                }
                APP_LOGI("get all bundle info success");
            }
            break;
        }
        case DumpFlag::DUMP_BUNDLE_INFO: {
            BundleInfo bundleInfo;
            ret = GetBundleInfo(
                bundleName, BundleFlag::GET_BUNDLE_WITH_ABILITIES | BundleFlag::GET_APPLICATION_INFO_WITH_PERMISSION,
                bundleInfo, userId);
            std::vector<InnerBundleUserInfo> innerBundleUserInfos;
            if (!GetBundleUserInfos(bundleName, userId, innerBundleUserInfos)) {
                APP_LOGI("get all userInfo in bundle(%{public}s) failed", bundleName.c_str());
            }

            if (ret) {
                result.append(bundleName);
                result.append(":\n");
                nlohmann::json jsonObject = bundleInfo;
                jsonObject["hapModuleInfos"] = bundleInfo.hapModuleInfos;
                jsonObject["innerBundleUserInfos"] = innerBundleUserInfos;
                result.append(jsonObject.dump(Constants::DUMP_INDENT));
                result.append("\n");
                APP_LOGI("get %{public}s bundle info success", bundleName.c_str());
            }
            break;
        }
        case DumpFlag::DUMP_SHORTCUT_INFO: {
            std::vector<ShortcutInfo> shortcutInfos;
            ret = GetShortcutInfos(bundleName, shortcutInfos);
            if (ret) {
                result.append("shortcuts");
                result.append(":\n");
                for (const auto &info : shortcutInfos) {
                    result.append("\"shortcut\"");
                    result.append(":\n");
                    nlohmann::json jsonObject = info;
                    result.append(jsonObject.dump(Constants::DUMP_INDENT));
                    result.append("\n");
                }
                APP_LOGI("get %{public}s shortcut info success", bundleName.c_str());
            }
            break;
        }
        default:
            APP_LOGE("dump flag error");
            return false;
    }
    return ret;
}

bool BundleMgrHostImpl::IsApplicationEnabled(const std::string &bundleName)
{
    auto dataMgr = GetDataMgrFromService();
    if (dataMgr == nullptr) {
        APP_LOGE("DataMgr is nullptr");
        return false;
    }
    return dataMgr->IsApplicationEnabled(bundleName);
}

bool BundleMgrHostImpl::SetApplicationEnabled(const std::string &bundleName, bool isEnable)
{
    auto dataMgr = GetDataMgrFromService();
    if (dataMgr == nullptr) {
        APP_LOGE("DataMgr is nullptr");
        return false;
    }
    bool result = dataMgr->SetApplicationEnabled(bundleName, isEnable);
    if (result) {
        int32_t uid = Constants::INVALID_UID;
        dataMgr->NotifyBundleStatus(bundleName,
                                    Constants::EMPTY_STRING,
                                    Constants::EMPTY_STRING,
                                    ERR_OK,
                                    NotifyType::APPLICATION_ENABLE,
                                    uid);
    }
    return result;
}

bool BundleMgrHostImpl::IsAbilityEnabled(const AbilityInfo &abilityInfo)
{
    auto dataMgr = GetDataMgrFromService();
    if (dataMgr == nullptr) {
        APP_LOGE("DataMgr is nullptr");
        return false;
    }
    return dataMgr->IsAbilityEnabled(abilityInfo);
}

bool BundleMgrHostImpl::SetAbilityEnabled(const AbilityInfo &abilityInfo, bool isEnabled)
{
    auto dataMgr = GetDataMgrFromService();
    if (dataMgr == nullptr) {
        APP_LOGE("DataMgr is nullptr");
        return false;
    }
    bool result = dataMgr->SetAbilityEnabled(abilityInfo, isEnabled);
    if (result) {
        int32_t uid = Constants::INVALID_UID;
        dataMgr->NotifyBundleStatus(abilityInfo.bundleName,
                                    Constants::EMPTY_STRING,
                                    abilityInfo.name,
                                    ERR_OK,
                                    NotifyType::APPLICATION_ENABLE,
                                    uid);
    }
    return result;
}

std::string BundleMgrHostImpl::GetAbilityIcon(const std::string &bundleName, const std::string &className)
{
    auto dataMgr = GetDataMgrFromService();
    if (dataMgr == nullptr) {
        APP_LOGE("DataMgr is nullptr");
        return Constants::EMPTY_STRING;
    }
    return dataMgr->GetAbilityIcon(bundleName, className);
}

sptr<IBundleInstaller> BundleMgrHostImpl::GetBundleInstaller()
{
    return DelayedSingleton<BundleMgrService>::GetInstance()->GetBundleInstaller();
}

sptr<IBundleUserMgr> BundleMgrHostImpl::GetBundleUserMgr()
{
    return DelayedSingleton<BundleMgrService>::GetInstance()->GetBundleUserMgr();
}

bool BundleMgrHostImpl::CanRequestPermission(
    const std::string &bundleName, const std::string &permissionName, const int userId)
{
    if (bundleName.empty() || permissionName.empty()) {
        APP_LOGE("fail to CanRequestPermission due to params empty");
        return false;
    }
    return BundlePermissionMgr::CanRequestPermission(bundleName, permissionName, userId);
}

bool BundleMgrHostImpl::RequestPermissionFromUser(
    const std::string &bundleName, const std::string &permissionName, const int userId)
{
    if (bundleName.empty() || permissionName.empty()) {
        APP_LOGE("fail to CanRequestPermission due to params empty");
        return false;
    }
    bool ret = BundlePermissionMgr::RequestPermissionFromUser(bundleName, permissionName, userId);
    // send Permissions Changed event
    APP_LOGI("send Permissions Changed event");
    BundleInfo info;
    bool ret_getInfo = GetBundleInfo(bundleName, BundleFlag::GET_BUNDLE_DEFAULT, info);
    APP_LOGI("ret_getInfo = %{public}d", ret_getInfo);
    if (ret && ret_getInfo) {
        Want want;
        want.SetAction("PERMISSIONS_CHANGED_EVENT");
        EventFwk::CommonEventData commonData;
        commonData.SetWant(want);
        commonData.SetCode(info.uid);
        EventFwk::CommonEventManager::PublishCommonEvent(commonData);
    }
    return ret;
}

bool BundleMgrHostImpl::RegisterAllPermissionsChanged(const sptr<OnPermissionChangedCallback> &callback)
{
    auto dataMgr = GetDataMgrFromService();
    if (dataMgr == nullptr) {
        APP_LOGE("DataMgr is nullptr");
        return false;
    }
    return dataMgr->RegisterAllPermissionsChanged(callback);
}

bool BundleMgrHostImpl::RegisterPermissionsChanged(
    const std::vector<int> &uids, const sptr<OnPermissionChangedCallback> &callback)
{
    auto dataMgr = GetDataMgrFromService();
    if (dataMgr == nullptr) {
        APP_LOGE("DataMgr is nullptr");
        return false;
    }
    return dataMgr->RegisterPermissionsChanged(uids, callback);
}

bool BundleMgrHostImpl::UnregisterPermissionsChanged(const sptr<OnPermissionChangedCallback> &callback)
{
    auto dataMgr = GetDataMgrFromService();
    if (dataMgr == nullptr) {
        APP_LOGE("DataMgr is nullptr");
        return false;
    }
    return dataMgr->UnregisterPermissionsChanged(callback);
}

bool BundleMgrHostImpl::GetAllFormsInfo(std::vector<FormInfo> &formInfos)
{
    auto dataMgr = GetDataMgrFromService();
    if (dataMgr == nullptr) {
        APP_LOGE("DataMgr is nullptr");
        return false;
    }
    return dataMgr->GetAllFormsInfo(formInfos);
}

bool BundleMgrHostImpl::GetFormsInfoByApp(const std::string &bundleName, std::vector<FormInfo> &formInfos)
{
    auto dataMgr = GetDataMgrFromService();
    if (dataMgr == nullptr) {
        APP_LOGE("DataMgr is nullptr");
        return false;
    }
    return dataMgr->GetFormsInfoByApp(bundleName, formInfos);
}

bool BundleMgrHostImpl::GetFormsInfoByModule(
    const std::string &bundleName, const std::string &moduleName, std::vector<FormInfo> &formInfos)
{
    auto dataMgr = GetDataMgrFromService();
    if (dataMgr == nullptr) {
        APP_LOGE("DataMgr is nullptr");
        return false;
    }
    return dataMgr->GetFormsInfoByModule(bundleName, moduleName, formInfos);
}

bool BundleMgrHostImpl::GetShortcutInfos(const std::string &bundleName, std::vector<ShortcutInfo> &shortcutInfos)
{
    auto dataMgr = GetDataMgrFromService();
    if (dataMgr == nullptr) {
        APP_LOGE("DataMgr is nullptr");
        return false;
    }
    return dataMgr->GetShortcutInfos(bundleName, shortcutInfos);
}

bool BundleMgrHostImpl::GetAllCommonEventInfo(const std::string &eventKey,
    std::vector<CommonEventInfo> &commonEventInfos)
{
    auto dataMgr = GetDataMgrFromService();
    if (dataMgr == nullptr) {
        APP_LOGE("DataMgr is nullptr");
        return false;
    }
    return dataMgr->GetAllCommonEventInfo(eventKey, commonEventInfos);
}

bool BundleMgrHostImpl::GetModuleUsageRecords(const int32_t number, std::vector<ModuleUsageRecord> &moduleUsageRecords)
{
    auto dataMgr = GetDataMgrFromService();
    if (dataMgr == nullptr) {
        APP_LOGE("DataMgr is nullptr");
        return false;
    }
    return dataMgr->GetUsageRecords(number, moduleUsageRecords);
}

bool BundleMgrHostImpl::NotifyAbilityLifeStatus(
    const std::string &bundleName, const std::string &abilityName, const int64_t launchTime, const int uid)
{
    APP_LOGI("NotifyAbilityLifeStatus begin");
    std::thread([this, bundleName, abilityName, launchTime, uid]() {
        auto dataMgr = GetDataMgrFromService();
        dataMgr->NotifyAbilityLifeStatus(bundleName, abilityName, launchTime, uid);
    }).detach();
    APP_LOGI("NotifyAbilityLifeStatus end");
    return true;
}

bool BundleMgrHostImpl::RemoveClonedBundle(const std::string &bundleName, const int32_t uid)
{
    APP_LOGI("RemoveClonedBundle begin");
    if (bundleName.empty()) {
        APP_LOGI("remove cloned bundle failed");
        return false;
    }
    auto cloneMgr = GetCloneMgrFromService();
    if (cloneMgr == nullptr) {
        APP_LOGE("cloneMgr is nullptr");
        return false;
    }
    std::string struid = std::to_string(uid);
    std::string newName = bundleName + "#" + struid;
    return cloneMgr->RemoveClonedBundle(bundleName, newName);
}

bool BundleMgrHostImpl::BundleClone(const std::string &bundleName)
{
    APP_LOGI("bundle clone begin");
    if (bundleName.empty()) {
        APP_LOGI("bundle clone failed");
        return false;
    }
    auto cloneMgr = GetCloneMgrFromService();
    if (cloneMgr == nullptr) {
        APP_LOGE("cloneMgr is nullptr");
        return false;
    }
    auto result = cloneMgr->BundleClone(bundleName);
    return result;
}

bool BundleMgrHostImpl::CheckBundleNameInAllowList(const std::string &bundleName)
{
    APP_LOGI("Check BundleName In AllowList begin");
    if (bundleName.empty()) {
        APP_LOGI("Check BundleName In AllowList failed");
        return false;
    }
    auto cloneMgr = GetCloneMgrFromService();
    if (cloneMgr == nullptr) {
        APP_LOGE("cloneMgr is nullptr");
        return false;
    }
    auto result = cloneMgr->CheckBundleNameInAllowList(bundleName);
    return result;
}

const std::shared_ptr<BundleCloneMgr> BundleMgrHostImpl::GetCloneMgrFromService()
{
    return DelayedSingleton<BundleMgrService>::GetInstance()->GetCloneMgr();
}

const std::shared_ptr<BundleDataMgr> BundleMgrHostImpl::GetDataMgrFromService()
{
    return DelayedSingleton<BundleMgrService>::GetInstance()->GetDataMgr();
}

}  // namespace AppExecFwk
}  // namespace OHOS
