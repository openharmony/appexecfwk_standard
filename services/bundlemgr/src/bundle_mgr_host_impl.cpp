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
#include "ipc_skeleton.h"
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
    if (!VerifyQueryPermission(true, appName)) {
        APP_LOGE("verify permission failed");
        return false;
    }
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
    if (!VerifyQueryPermission()) {
        APP_LOGE("verify permission failed");
        return false;
    }
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
    if (!VerifyQueryPermission(true, bundleName)) {
        APP_LOGE("verify permission failed");
        return false;
    }
    auto dataMgr = GetDataMgrFromService();
    if (dataMgr == nullptr) {
        APP_LOGE("DataMgr is nullptr");
        return false;
    }
    return dataMgr->GetBundleInfo(bundleName, flags, bundleInfo, userId);
}

bool BundleMgrHostImpl::GetBundleUserInfo(
    const std::string &bundleName, int32_t userId, InnerBundleUserInfo &innerBundleUserInfo)
{
    auto dataMgr = GetDataMgrFromService();
    if (dataMgr == nullptr) {
        APP_LOGE("DataMgr is nullptr");
        return false;
    }
    return dataMgr->GetInnerBundleUserInfoByUserId(bundleName, userId, innerBundleUserInfo);
}

bool BundleMgrHostImpl::GetBundleUserInfos(
    const std::string &bundleName, std::vector<InnerBundleUserInfo> &innerBundleUserInfos)
{
    auto dataMgr = GetDataMgrFromService();
    if (dataMgr == nullptr) {
        APP_LOGE("DataMgr is nullptr");
        return false;
    }
    return dataMgr->GetInnerBundleUserInfos(bundleName, innerBundleUserInfos);
}

bool BundleMgrHostImpl::GetBundleInfos(const BundleFlag flag, std::vector<BundleInfo> &bundleInfos, int32_t userId)
{
    return GetBundleInfos(static_cast<int32_t>(flag), bundleInfos, userId);
}

bool BundleMgrHostImpl::GetBundleInfos(int32_t flags, std::vector<BundleInfo> &bundleInfos, int32_t userId)
{
    if (!VerifyQueryPermission()) {
        APP_LOGE("verify permission failed");
        return false;
    }
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
    if (!VerifyQueryPermission(true, want.GetElement().GetBundleName())) {
        APP_LOGE("verify permission failed");
        return false;
    }
    auto dataMgr = GetDataMgrFromService();
    if (dataMgr == nullptr) {
        APP_LOGE("DataMgr is nullptr");
        return false;
    }
    return dataMgr->QueryAbilityInfo(want, flags, userId, abilityInfo);
}

bool BundleMgrHostImpl::QueryAbilityInfos(const Want &want, std::vector<AbilityInfo> &abilityInfos)
{
    return QueryAbilityInfos(
        want, GET_ABILITY_INFO_WITH_APPLICATION, Constants::UNSPECIFIED_USERID, abilityInfos);
}

bool BundleMgrHostImpl::QueryAbilityInfos(
    const Want &want, int32_t flags, int32_t userId, std::vector<AbilityInfo> &abilityInfos)
{
    if (!VerifyQueryPermission(true, want.GetElement().GetBundleName())) {
        APP_LOGE("verify permission failed");
        return false;
    }
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
    if (!VerifyQueryPermission()) {
        APP_LOGE("verify permission failed");
        return false;
    }
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
    return dataMgr->QueryAbilityInfoByUri(abilityUri, Constants::UNSPECIFIED_USERID, abilityInfo);
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

bool BundleMgrHostImpl::QueryAbilityInfoByUri(
    const std::string &abilityUri, int32_t userId, AbilityInfo &abilityInfo)
{
    auto dataMgr = GetDataMgrFromService();
    if (dataMgr == nullptr) {
        APP_LOGE("DataMgr is nullptr");
        return false;
    }
    return dataMgr->QueryAbilityInfoByUri(abilityUri, userId, abilityInfo);
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
    if (!VerifyQueryPermission(true, info.GetBundleName())) {
        APP_LOGE("verify permission failed");
        return false;
    }
    info.GetBundleInfo(flags, bundleInfo, Constants::NOT_EXIST_USERID);
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
    if (!VerifyQueryPermission()) {
        APP_LOGE("verify permission failed");
        return false;
    }
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
    int32_t userId = BundleUtil::GetUserIdByCallingUid();
    return BundlePermissionMgr::VerifyPermission(bundleName, permission, userId);
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
    const std::string &bundleName, const sptr<ICleanCacheCallback> &cleanCacheCallback,
    int32_t userId)
{
    if (bundleName.empty() || !cleanCacheCallback) {
        APP_LOGE("the cleanCacheCallback is nullptr or bundleName empty");
        return false;
    }
    if (!BundlePermissionMgr::VerifyCallingPermission(Constants::PERNISSION_REMOVECACHEFILE)) {
        APP_LOGE("ohos.permission.REMOVE_CACHE_FILES permission denied");
        return false;
    }
    ApplicationInfo applicationInfo;
    if (!GetApplicationInfo(bundleName, ApplicationFlag::GET_BASIC_APPLICATION_INFO,
        userId, applicationInfo)) {
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
        APP_LOGD("CleanBundleCacheFiles with error %{public}d", error);
        cleanCacheCallback->OnCleanCacheFinished(error);
    };
    handler_->PostTask(cleanCache);
    return true;
}

bool BundleMgrHostImpl::CleanBundleDataFiles(const std::string &bundleName, const int userId)
{
    if (bundleName.empty() || userId < 0) {
        APP_LOGE("the  bundleName empty or invalid userid");
        return false;
    }
    ApplicationInfo applicationInfo;
    if (!GetApplicationInfo(bundleName, ApplicationFlag::GET_BASIC_APPLICATION_INFO, userId, applicationInfo)) {
        APP_LOGE("can not get application info of %{public}s", bundleName.c_str());
        return false;
    }
    InnerBundleUserInfo innerBundleUserInfo;
    if (!GetBundleUserInfo(bundleName, userId, innerBundleUserInfo)) {
        APP_LOGE("%{public}s, userId:%{public}d, GetBundleUserInfo failed", bundleName.c_str(), userId);
        return false;
    }
    if (BundlePermissionMgr::ClearUserGrantedPermissionState(applicationInfo.accessTokenId)) {
        APP_LOGE("%{public}s, ClearUserGrantedPermissionState failed", bundleName.c_str());
        return false;
    }
    bool isStartUserId = false;
    if (userId == Constants::START_USERID) {
        isStartUserId = true;
        if (InstalldClient::GetInstance()->RemoveDir(applicationInfo.dataDir) != ERR_OK) {
            APP_LOGE("%{public}s, RemoveDir:%{public}s failed", bundleName.c_str(), applicationInfo.dataDir.c_str());
            return false;
        }
    }
    if (InstalldClient::GetInstance()->RemoveBundleDataDir(bundleName, userId) != ERR_OK) {
        APP_LOGE("%{public}s, RemoveBundleDataDir failed", bundleName.c_str());
        return false;
    }
    if (InstalldClient::GetInstance()->CreateBundleDataDir(applicationInfo.dataDir, userId, innerBundleUserInfo.uid,
        innerBundleUserInfo.uid, GetAppPrivilegeLevel(bundleName), isStartUserId) != ERR_OK) {
        APP_LOGE("%{public}s, CreateBundleDataDir failed", bundleName.c_str());
        return false;
    }
    return true;
}

bool BundleMgrHostImpl::RegisterBundleStatusCallback(const sptr<IBundleStatusCallback> &bundleStatusCallback)
{
    if ((!bundleStatusCallback) || (bundleStatusCallback->GetBundleName().empty())) {
        APP_LOGE("the bundleStatusCallback is nullptr or bundleName empty");
        return false;
    }
    // check permission
    if (!BundlePermissionMgr::VerifyCallingPermission(Constants::LISTEN_BUNDLE_CHANGE)) {
        APP_LOGE("register bundle status callback failed due to lack of permission");
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

    // check permission
    if (!BundlePermissionMgr::VerifyCallingPermission(Constants::LISTEN_BUNDLE_CHANGE)) {
        APP_LOGE("register bundle status callback failed due to lack of permission");
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
    // check permission
    if (!BundlePermissionMgr::VerifyCallingPermission(Constants::LISTEN_BUNDLE_CHANGE)) {
        APP_LOGE("register bundle status callback failed due to lack of permission");
        return false;
    }

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
    APP_LOGD("dump infos begin");
    bool ret = false;
    switch (flag) {
        case DumpFlag::DUMP_BUNDLE_LIST: {
            ret = DumpAllBundleInfoNames(userId, result);
            break;
        }
        case DumpFlag::DUMP_ALL_BUNDLE_INFO: {
            ret = DumpAllBundleInfos(userId, result);
            break;
        }
        case DumpFlag::DUMP_BUNDLE_INFO: {
            ret = DumpBundleInfo(bundleName, userId, result);
            break;
        }
        case DumpFlag::DUMP_SHORTCUT_INFO: {
            ret = DumpShortcutInfo(bundleName, userId, result);
            break;
        }
        default:
            APP_LOGE("dump flag error");
            return false;
    }
    return ret;
}

bool BundleMgrHostImpl::DumpAllBundleInfoNames(int32_t userId, std::string &result)
{
    if (userId != Constants::ALL_USERID) {
        return DumpAllBundleInfoNamesByUserId(userId, result);
    }

    auto userIds = GetExistsCommonUserIs();
    for (auto userId : userIds) {
        DumpAllBundleInfoNamesByUserId(userId, result);
    }

    APP_LOGI("get installed bundles success");
    return true;
}

bool BundleMgrHostImpl::DumpAllBundleInfoNamesByUserId(int32_t userId, std::string &result)
{
    auto dataMgr = GetDataMgrFromService();
    if (dataMgr == nullptr) {
        APP_LOGE("DataMgr is nullptr");
        return false;
    }

    std::vector<std::string> bundleNames;
    if (!dataMgr->GetBundleList(bundleNames, userId)) {
        APP_LOGE("get bundle list failed by userId(%{public}d)", userId);
        return false;
    }

    result.append("ID: ");
    result.append(std::to_string(userId));
    result.append(":\n");
    for (const auto &name : bundleNames) {
        result.append("\t");
        result.append(name);
        result.append("\n");
    }
    return true;
}

bool BundleMgrHostImpl::DumpAllBundleInfos(int32_t userId, std::string &result)
{
    std::vector<BundleInfo> bundleInfos;
    if (!GetBundleInfos(BundleFlag::GET_BUNDLE_WITH_ABILITIES | BundleFlag::GET_BUNDLE_WITH_EXTENSION_INFO,
        bundleInfos, userId)) {
        APP_LOGE("get bundleInfos failed.");
        return false;
    }

    for (const auto &info : bundleInfos) {
        std::vector<InnerBundleUserInfo> innerBundleUserInfos;
        if (userId == Constants::ALL_USERID) {
            if (!GetBundleUserInfos(info.name, innerBundleUserInfos)) {
                APP_LOGE("get all userInfos in bundle(%{public}s) failed", info.name.c_str());
                return false;
            }
        } else {
            InnerBundleUserInfo innerBundleUserInfo;
            if (!GetBundleUserInfo(info.name, userId, innerBundleUserInfo)) {
                APP_LOGI("get all userInfo in bundle(%{public}s) failed", info.name.c_str());
            }
            innerBundleUserInfos.emplace_back(innerBundleUserInfo);
        }

        result.append(info.name);
        result.append(":\n");
        nlohmann::json jsonObject = info;
        jsonObject["hapModuleInfos"] = info.hapModuleInfos;
        jsonObject["userInfo"] = innerBundleUserInfos;
        result.append(jsonObject.dump(Constants::DUMP_INDENT));
        result.append("\n");
    }
    APP_LOGI("get all bundle info success");
    return true;
}

bool BundleMgrHostImpl::DumpBundleInfo(
    const std::string &bundleName, int32_t userId, std::string &result)
{
    APP_LOGD("dump bundle info begin");
    std::vector<InnerBundleUserInfo> innerBundleUserInfos;
    if (userId == Constants::ALL_USERID) {
        if (!GetBundleUserInfos(bundleName, innerBundleUserInfos)) {
            APP_LOGE("get all userInfos in bundle(%{public}s) failed", bundleName.c_str());
            return false;
        }
        userId = innerBundleUserInfos.begin()->bundleUserInfo.userId;
    } else {
        InnerBundleUserInfo innerBundleUserInfo;
        if (!GetBundleUserInfo(bundleName, userId, innerBundleUserInfo)) {
            APP_LOGI("get userInfo in bundle(%{public}s) failed", bundleName.c_str());
        }
        innerBundleUserInfos.emplace_back(innerBundleUserInfo);
    }

    BundleInfo bundleInfo;
    if (!GetBundleInfo(bundleName,
        BundleFlag::GET_BUNDLE_WITH_ABILITIES | BundleFlag::GET_BUNDLE_WITH_REQUESTED_PERMISSION |
        BundleFlag::GET_BUNDLE_WITH_EXTENSION_INFO, bundleInfo, userId)) {
        APP_LOGE("get bundleInfo(%{public}s) failed", bundleName.c_str());
        return false;
    }

    result.append(bundleName);
    result.append(":\n");
    nlohmann::json jsonObject = bundleInfo;
    jsonObject["hapModuleInfos"] = bundleInfo.hapModuleInfos;
    jsonObject["userInfo"] = innerBundleUserInfos;
    result.append(jsonObject.dump(Constants::DUMP_INDENT));
    result.append("\n");
    APP_LOGI("get %{public}s bundle info success", bundleName.c_str());
    return true;
}

bool BundleMgrHostImpl::DumpShortcutInfo(
    const std::string &bundleName, int32_t userId, std::string &result)
{
    std::vector<ShortcutInfo> shortcutInfos;
    if (userId == Constants::ALL_USERID) {
        std::vector<InnerBundleUserInfo> innerBundleUserInfos;
        if (!GetBundleUserInfos(bundleName, innerBundleUserInfos)) {
            APP_LOGE("get all userInfos in bundle(%{public}s) failed", bundleName.c_str());
            return false;
        }
        userId = innerBundleUserInfos.begin()->bundleUserInfo.userId;
    }

    if (!GetShortcutInfos(bundleName, userId, shortcutInfos)) {
        APP_LOGE("get all shortcut info by bundle(%{public}s) failed", bundleName.c_str());
        return false;
    }

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
    return true;
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

bool BundleMgrHostImpl::SetApplicationEnabled(const std::string &bundleName, bool isEnable, int32_t userId)
{
    if (!BundlePermissionMgr::VerifyCallingPermission(Constants::PERMISSION_CHANGE_ABILITY_ENABLED_STATE)) {
        APP_LOGE("verify permission failed");
        return false;
    }
    auto dataMgr = GetDataMgrFromService();
    if (dataMgr == nullptr) {
        APP_LOGE("DataMgr is nullptr");
        return false;
    }

    if (!dataMgr->SetApplicationEnabled(bundleName, isEnable, userId)) {
        APP_LOGE("Set application(%{public}s) enabled value faile.", bundleName.c_str());
        return false;
    }

    InnerBundleUserInfo innerBundleUserInfo;
    if (!GetBundleUserInfo(bundleName, userId, innerBundleUserInfo)) {
        APP_LOGE("Get calling userInfo in bundle(%{public}s) failed", bundleName.c_str());
        return false;
    }

    dataMgr->NotifyBundleStatus(bundleName,
                                Constants::EMPTY_STRING,
                                Constants::EMPTY_STRING,
                                ERR_OK,
                                NotifyType::APPLICATION_ENABLE,
                                innerBundleUserInfo.uid);
    return true;
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

bool BundleMgrHostImpl::SetAbilityEnabled(const AbilityInfo &abilityInfo, bool isEnabled, int32_t userId)
{
    if (!BundlePermissionMgr::VerifyCallingPermission(Constants::PERMISSION_CHANGE_ABILITY_ENABLED_STATE)) {
        APP_LOGE("verify permission failed");
        return false;
    }
    auto dataMgr = GetDataMgrFromService();
    if (dataMgr == nullptr) {
        APP_LOGE("DataMgr is nullptr");
        return false;
    }

    if (!dataMgr->SetAbilityEnabled(abilityInfo, isEnabled, userId)) {
        APP_LOGE("Set ability(%{public}s) enabled value faile.", abilityInfo.bundleName.c_str());
        return false;
    }

    InnerBundleUserInfo innerBundleUserInfo;
    if (!GetBundleUserInfo(abilityInfo.bundleName, userId, innerBundleUserInfo)) {
        APP_LOGE("Get calling userInfo in bundle(%{public}s) failed", abilityInfo.bundleName.c_str());
        return false;
    }

    dataMgr->NotifyBundleStatus(abilityInfo.bundleName,
                                Constants::EMPTY_STRING,
                                abilityInfo.name,
                                ERR_OK,
                                NotifyType::APPLICATION_ENABLE,
                                innerBundleUserInfo.uid);
    return true;
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

bool BundleMgrHostImpl::GetShortcutInfos(
    const std::string &bundleName, std::vector<ShortcutInfo> &shortcutInfos)
{
    return GetShortcutInfos(bundleName, Constants::UNSPECIFIED_USERID, shortcutInfos);
}

bool BundleMgrHostImpl::GetShortcutInfos(
    const std::string &bundleName, int32_t userId, std::vector<ShortcutInfo> &shortcutInfos)
{
    if (!VerifyQueryPermission()) {
        APP_LOGE("verify permission failed");
        return false;
    }
    auto dataMgr = GetDataMgrFromService();
    if (dataMgr == nullptr) {
        APP_LOGE("DataMgr is nullptr");
        return false;
    }
    return dataMgr->GetShortcutInfos(bundleName, userId, shortcutInfos);
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
    auto task = [this, bundleName, abilityName, launchTime, uid] {
        auto dataMgr = GetDataMgrFromService();
        dataMgr->NotifyAbilityLifeStatus(bundleName, abilityName, launchTime, uid);
    };
    handler_->PostTask(task);
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

bool BundleMgrHostImpl::GetDistributedBundleInfo(
    const std::string &networkId, int32_t userId, const std::string &bundleName,
    DistributedBundleInfo &distributedBundleInfo)
{
    auto dataMgr = GetDataMgrFromService();
    if (dataMgr == nullptr) {
        APP_LOGE("DataMgr is nullptr");
        return false;
    }
    return dataMgr->GetDistributedBundleInfo(networkId, userId, bundleName, distributedBundleInfo);
}

bool BundleMgrHostImpl::QueryExtensionAbilityInfos(const Want &want, const int32_t &flag, const int32_t &userId,
    std::vector<ExtensionAbilityInfo> &extensionInfos)
{
    APP_LOGD("QueryExtensionAbilityInfos without type begin");
    APP_LOGD("want uri is %{public}s", want.GetUriString().c_str());
    auto dataMgr = GetDataMgrFromService();
    if (dataMgr == nullptr) {
        APP_LOGE("DataMgr is nullptr");
        return false;
    }
    bool ret = dataMgr->QueryExtensionAbilityInfos(want, flag, userId, extensionInfos);
    if (!ret) {
        APP_LOGE("QueryExtensionAbilityInfos is failed");
        return false;
    }
    if (extensionInfos.empty()) {
        APP_LOGE("no valid extension info can be inquired");
        return false;
    }
    return true;
}

bool BundleMgrHostImpl::QueryExtensionAbilityInfos(const Want &want, const ExtensionAbilityType &extensionType,
    const int32_t &flag, const int32_t &userId, std::vector<ExtensionAbilityInfo> &extensionInfos)
{
    APP_LOGD("QueryExtensionAbilityInfos begin");
    auto dataMgr = GetDataMgrFromService();
    if (dataMgr == nullptr) {
        APP_LOGE("DataMgr is nullptr");
        return false;
    }
    std::vector<ExtensionAbilityInfo> infos;
    bool ret = dataMgr->QueryExtensionAbilityInfos(want, flag, userId, infos);
    if (!ret) {
        APP_LOGE("QueryExtensionAbilityInfos is failed");
        return false;
    }
    for_each(infos.begin(), infos.end(), [&extensionType, &extensionInfos](const auto &info)->decltype(auto) {
        APP_LOGD("QueryExtensionAbilityInfos extensionType is %{public}d, info.type is %{public}d",
            static_cast<int32_t>(extensionType), static_cast<int32_t>(info.type));
        if (extensionType == info.type) {
            extensionInfos.emplace_back(info);
        }
    });
    if (extensionInfos.empty()) {
        APP_LOGE("no valid extension info can be inquired");
        return false;
    }
    return true;
}

bool BundleMgrHostImpl::QueryExtensionAbilityInfos(const ExtensionAbilityType &extensionType, const int32_t &userId,
    std::vector<ExtensionAbilityInfo> &extensionInfos)
{
    APP_LOGD("QueryExtensionAbilityInfos with type begin");
    auto dataMgr = GetDataMgrFromService();
    if (dataMgr == nullptr) {
        APP_LOGE("DataMgr is nullptr");
        return false;
    }
    bool ret = dataMgr->QueryExtensionAbilityInfos(extensionType, userId, extensionInfos);
    if (!ret) {
        APP_LOGE("QueryExtensionAbilityInfos is failed");
        return false;
    }

    if (extensionInfos.empty()) {
        APP_LOGE("no valid extension info can be inquired");
        return false;
    }
    return true;
}

const std::shared_ptr<BundleCloneMgr> BundleMgrHostImpl::GetCloneMgrFromService()
{
    return DelayedSingleton<BundleMgrService>::GetInstance()->GetCloneMgr();
}

const std::shared_ptr<BundleDataMgr> BundleMgrHostImpl::GetDataMgrFromService()
{
    return DelayedSingleton<BundleMgrService>::GetInstance()->GetDataMgr();
}

std::set<int32_t> BundleMgrHostImpl::GetExistsCommonUserIs()
{
    std::set<int32_t> userIds;
    auto dataMgr = GetDataMgrFromService();
    if (dataMgr == nullptr) {
        APP_LOGE("Get dataMgr shared_ptr nullptr");
        return userIds;
    }

    for (auto userId : dataMgr->GetAllUser()) {
        if (userId >= Constants::START_USERID) {
            userIds.insert(userId);
        }
    }
    return userIds;
}

bool BundleMgrHostImpl::VerifyQueryPermission(bool allowNormalApl, const std::string &queryBundleName)
{
    if (BundlePermissionMgr::VerifyCallingPermission(Constants::PERMISSION_GET_BUNDLE_INFO_PRIVILEGED)) {
        APP_LOGD("verify GET_BUNDLE_INFO_PRIVILEGED success");
        return true;
    }
    if (!allowNormalApl) {
        APP_LOGD("not allow normal apl, verify permission failed");
        return false;
    }
    std::string callingBundleName;
    bool ret = GetBundleNameForUid(IPCSkeleton::GetCallingUid(), callingBundleName);
    if (!ret) {
        APP_LOGE("GetBundleNameForUid failed");
        return false;
    }
    bool isBundleNameEqual = queryBundleName == callingBundleName;
    APP_LOGD("isBundleNameEqual : %{public}d", isBundleNameEqual);
    if (isBundleNameEqual) {
        APP_LOGD("query own info, verify success");
        return true;
    }
    // adapt HO
    ApplicationInfo applicationInfo;
    auto dataMgr = GetDataMgrFromService();
    if (dataMgr == nullptr) {
        APP_LOGE("get DataMgr failed");
        return false;
    }
    bool retVal = dataMgr->GetApplicationInfo(
        callingBundleName, GET_BASIC_APPLICATION_INFO, Constants::UNSPECIFIED_USERID, applicationInfo);
    if (!retVal) {
        APP_LOGE("GetApplicationInfo failed");
        return false;
    }
    if (applicationInfo.apiCompatibleVersion <= Constants::PERMISSION_COMPATIBLE_API_VERSION) {
        APP_LOGD("begin to verify GET_BUNDLE_INFO");
        if (BundlePermissionMgr::VerifyCallingPermission(Constants::PERMISSION_GET_BUNDLE_INFO)) {
            APP_LOGD("verify GET_BUNDLE_INFO success");
            return true;
        }
    }
    APP_LOGE("verify query permission failed");
    return false;
}

std::string BundleMgrHostImpl::GetAppPrivilegeLevel(const std::string &bundleName, int32_t userId)
{
    auto dataMgr = GetDataMgrFromService();
    if (dataMgr == nullptr) {
        APP_LOGE("DataMgr is nullptr");
        return Constants::EMPTY_STRING;
    }
    return dataMgr->GetAppPrivilegeLevel(bundleName, userId);
}

bool BundleMgrHostImpl::VerifyCallingPermission(const std::string &permission)
{
    APP_LOGD("VerifyCallingPermission begin");
    return BundlePermissionMgr::VerifyCallingPermission(permission);
}

std::vector<std::string> BundleMgrHostImpl::GetAccessibleAppCodePaths(int32_t userId)
{
    APP_LOGD("GetAccessibleAppCodePaths begin");
    auto dataMgr = GetDataMgrFromService();
    if (dataMgr == nullptr) {
        APP_LOGE("DataMgr is nullptr");
        std::vector<std::string> vec;
        return vec;
    }

    return dataMgr->GetAccessibleAppCodePaths(userId);
}

bool BundleMgrHostImpl::QueryExtensionAbilityInfoByUri(const std::string &uri, int32_t userId,
    ExtensionAbilityInfo &extensionAbilityInfo)
{
    APP_LOGD("uri : %{public}s, userId : %{public}d", uri.c_str(), userId);
    auto dataMgr = GetDataMgrFromService();
    if (dataMgr == nullptr) {
        APP_LOGE("DataMgr is nullptr");
        return false;
    }
    return dataMgr->QueryExtensionAbilityInfoByUri(uri, userId, extensionAbilityInfo);
}
}  // namespace AppExecFwk
}  // namespace OHOS
