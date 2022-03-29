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

#include "base_bundle_installer.h"

#include <unistd.h>
#include <set>
#include <vector>

#include "ability_manager_interface.h"
#include "account_helper.h"
#include "app_log_wrapper.h"
#include "bundle_constants.h"
#include "bundle_extractor.h"
#include "bundle_mgr_service.h"
#include "bundle_parser.h"
#include "bundle_permission_mgr.h"
#include "bundle_util.h"
#include "bytrace.h"
#include "datetime_ex.h"
#include "installd_client.h"
#include "nlohmann/json.hpp"
#include "perf_profile.h"
#include "string_ex.h"
#include "system_ability_definition.h"
#include "system_ability_helper.h"
#include "systemcapability.h"
#include "bundle_clone_mgr.h"
#include "scope_guard.h"
#include "bundle_verify_mgr.h"
namespace OHOS {
namespace AppExecFwk {
using namespace OHOS::Security;
namespace {
bool UninstallApplicationProcesses(const std::string &bundleName, const int uid)
{
    APP_LOGI("uninstall kill running processes, app name is %{public}s", bundleName.c_str());
    sptr<AAFwk::IAbilityManager> abilityMgrProxy =
        iface_cast<AAFwk::IAbilityManager>(SystemAbilityHelper::GetSystemAbility(ABILITY_MGR_SERVICE_ID));
    if (!abilityMgrProxy) {
        APP_LOGE("fail to find the app mgr service to kill application");
        return false;
    }
    if (abilityMgrProxy->UninstallApp(bundleName, uid) != 0) {
        APP_LOGE("kill application process failed");
        return false;
    }
    return true;
}
}  // namespace

BaseBundleInstaller::BaseBundleInstaller()
{
    APP_LOGI("base bundle installer instance is created");
}

BaseBundleInstaller::~BaseBundleInstaller()
{
    APP_LOGI("base bundle installer instance is destroyed");
}

ErrCode BaseBundleInstaller::InstallBundle(
    const std::string &bundlePath, const InstallParam &installParam, const Constants::AppType appType)
{
    std::vector<std::string> bundlePaths { bundlePath };
    return InstallBundle(bundlePaths, installParam, appType);
}

ErrCode BaseBundleInstaller::InstallBundle(
    const std::vector<std::string> &bundlePaths, const InstallParam &installParam, const Constants::AppType appType)
{
    BYTRACE_NAME(BYTRACE_TAG_APP, __PRETTY_FUNCTION__);
    APP_LOGD("begin to process bundle install");

    PerfProfile::GetInstance().SetBundleInstallStartTime(GetTickCount());

    int32_t uid = Constants::INVALID_UID;
    ErrCode result = ProcessBundleInstall(bundlePaths, installParam, appType, uid);
    if (installParam.needSendEvent && dataMgr_ && !bundleName_.empty()) {
        dataMgr_->NotifyBundleStatus(bundleName_,
            Constants::EMPTY_STRING,
            mainAbility_,
            result,
            isAppExist_ ? NotifyType::UPDATE : NotifyType::INSTALL,
            uid);
    }
    if (result == ERR_OK) {
        SaveStorageDistributeInfo();
    }

    PerfProfile::GetInstance().SetBundleInstallEndTime(GetTickCount());
    APP_LOGD("finish to process bundle install");
    return result;
}

ErrCode BaseBundleInstaller::Recover(
    const std::string &bundleName, const InstallParam &installParam)
{
    APP_LOGD("begin to process bundle install by bundleName, which is %{public}s.", bundleName.c_str());
    PerfProfile::GetInstance().SetBundleInstallStartTime(GetTickCount());

    int32_t uid = Constants::INVALID_UID;
    ErrCode result = ProcessRecover(bundleName, installParam, uid);
    if (installParam.needSendEvent && dataMgr_ && !bundleName_.empty() && !modulePackage_.empty()) {
        dataMgr_->NotifyBundleStatus(bundleName_,
            modulePackage_,
            mainAbility_,
            result,
            isAppExist_ ? NotifyType::UPDATE : NotifyType::INSTALL,
            uid);
    }
    if (result == ERR_OK) {
        SaveStorageDistributeInfo();
    }

    PerfProfile::GetInstance().SetBundleInstallEndTime(GetTickCount());
    APP_LOGD("finish to process %{public}s bundle install", bundleName.c_str());
    return result;
}

ErrCode BaseBundleInstaller::UninstallBundle(const std::string &bundleName, const InstallParam &installParam)
{
    APP_LOGD("begin to process %{public}s bundle uninstall", bundleName.c_str());
    PerfProfile::GetInstance().SetBundleUninstallStartTime(GetTickCount());

    int32_t uid = Constants::INVALID_UID;
    ErrCode result = ProcessBundleUninstall(bundleName, installParam, uid);
    if (installParam.needSendEvent && dataMgr_) {
        dataMgr_->NotifyBundleStatus(
            bundleName, Constants::EMPTY_STRING, Constants::EMPTY_STRING, result, NotifyType::UNINSTALL_BUNDLE, uid);
    }
    if (result == ERR_OK) {
        DeleteStorageDistributeInfo(bundleName);
    }

    PerfProfile::GetInstance().SetBundleUninstallEndTime(GetTickCount());
    APP_LOGD("finish to process %{public}s bundle uninstall", bundleName.c_str());
    return result;
}

ErrCode BaseBundleInstaller::UninstallBundle(
    const std::string &bundleName, const std::string &modulePackage, const InstallParam &installParam)
{
    APP_LOGD("begin to process %{public}s module in %{public}s uninstall", modulePackage.c_str(), bundleName.c_str());
    PerfProfile::GetInstance().SetBundleUninstallStartTime(GetTickCount());

    int32_t uid = Constants::INVALID_UID;
    ErrCode result = ProcessBundleUninstall(bundleName, modulePackage, installParam, uid);
    if (installParam.needSendEvent && dataMgr_) {
        dataMgr_->NotifyBundleStatus(
            bundleName, modulePackage, Constants::EMPTY_STRING, result, NotifyType::UNINSTALL_MODULE, uid);
    }

    PerfProfile::GetInstance().SetBundleUninstallEndTime(GetTickCount());
    APP_LOGD("finish to process %{public}s module in %{public}s uninstall", modulePackage.c_str(), bundleName.c_str());

    if (result == ERR_OK) {
        DeleteStorageDistributeInfo(bundleName);
    }
    return result;
}

void BaseBundleInstaller::UpdateInstallerState(const InstallerState state)
{
    APP_LOGD("UpdateInstallerState in BaseBundleInstaller state %{public}d", state);
    SetInstallerState(state);
}

ErrCode BaseBundleInstaller::InnerProcessBundleInstall(std::unordered_map<std::string, InnerBundleInfo> &newInfos,
    InnerBundleInfo &oldInfo, const InstallParam &installParam, int32_t &uid)
{
    BYTRACE_NAME(BYTRACE_TAG_APP, __PRETTY_FUNCTION__);
    bundleName_ = newInfos.begin()->second.GetBundleName();
    APP_LOGI("InnerProcessBundleInstall with bundleName %{public}s, userId is %{public}d", bundleName_.c_str(),
        userId_);
    if (installParam.needSavePreInstallInfo) {
        PreInstallBundleInfo preInstallBundleInfo;
        dataMgr_->GetPreInstallBundleInfo(bundleName_, preInstallBundleInfo);
        preInstallBundleInfo.SetAppType(newInfos.begin()->second.GetAppType());
        preInstallBundleInfo.SetVersionCode(newInfos.begin()->second.GetVersionCode());
        for (const auto &item : newInfos) {
            preInstallBundleInfo.AddBundlePath(item.first);
        }
        dataMgr_->SavePreInstallBundleInfo(bundleName_, preInstallBundleInfo);
    }

    // singleton app can only be installed in U0 and U0 can only install singleton app.
    bool isSingleton = newInfos.begin()->second.IsSingleton();
    if ((isSingleton && (userId_ != Constants::DEFAULT_USERID)) ||
        (!isSingleton && (userId_ == Constants::DEFAULT_USERID))) {
        APP_LOGW("singleton(%{public}d) app(%{public}s) and user(%{public}d) are not matched.",
            isSingleton, bundleName_.c_str(), userId_);
        return ERR_APPEXECFWK_INSTALL_ZERO_USER_WITH_NO_SINGLETON;
    }

    // try to get the bundle info to decide use install or update. Always keep other exceptions below this line.
    if (!GetInnerBundleInfo(oldInfo, isAppExist_)) {
        return ERR_APPEXECFWK_INSTALL_BUNDLE_MGR_SERVICE_ERROR;
    }

    if (isAppExist_) {
        for (const auto &item : newInfos) {
            if (oldInfo.GetIsNewVersion() != item.second.GetIsNewVersion()) {
                return ERR_APPEXECFWK_INSTALL_STATE_ERROR;
            }
        }
    }

    ErrCode result = ERR_OK;
    if (isAppExist_) {
        // to guarantee that the hap version can be compatible.
        result = CheckVersionCompatibility(oldInfo);
        if (result != ERR_OK) {
            APP_LOGE("The app has been installed and update lower version bundle.");
            return result;
        }

        hasInstalledInUser_ = oldInfo.HasInnerBundleUserInfo(userId_);
        if (!hasInstalledInUser_) {
            APP_LOGD("new userInfo with bundleName %{public}s and userId %{public}d",
                bundleName_.c_str(), userId_);
            InnerBundleUserInfo newInnerBundleUserInfo;
            newInnerBundleUserInfo.bundleUserInfo.userId = userId_;
            newInnerBundleUserInfo.bundleName = bundleName_;
            oldInfo.AddInnerBundleUserInfo(newInnerBundleUserInfo);
            ScopeGuard userGuard([&] { RemoveBundleUserData(oldInfo, false); });
            uint32_t tokenId = CreateAccessTokenId(oldInfo);
            oldInfo.SetAccessTokenId(tokenId, userId_);
            result = GrantRequestPermissions(oldInfo, tokenId);
            if (result != ERR_OK) {
                return result;
            }

            result = CreateBundleUserData(oldInfo, false);
            if (result != ERR_OK) {
                return result;
            }

            userGuard.Dismiss();
        }

        for (auto &info : newInfos) {
            std::string packageName = info.second.GetCurrentModulePackage();
            if (oldInfo.FindModule(packageName)) {
                installedModules_[packageName] = true;
            }
        }
    }

    auto it = newInfos.begin();
    if (!isAppExist_) {
        APP_LOGI("app is not exist");
        InnerBundleInfo &newInfo = it->second;
        modulePath_ = it->first;
        InnerBundleUserInfo newInnerBundleUserInfo;
        newInnerBundleUserInfo.bundleUserInfo.userId = userId_;
        newInnerBundleUserInfo.bundleName = bundleName_;
        newInfo.AddInnerBundleUserInfo(newInnerBundleUserInfo);
        result = ProcessBundleInstallStatus(newInfo, uid);
        if (result != ERR_OK) {
            return result;
        }

        it++;
        hasInstalledInUser_ = true;
    }

    InnerBundleInfo bundleInfo;
    bool isBundleExist = false;
    if (!GetInnerBundleInfo(bundleInfo, isBundleExist) || !isBundleExist) {
        return ERR_APPEXECFWK_INSTALL_BUNDLE_MGR_SERVICE_ERROR;
    }

    InnerBundleUserInfo innerBundleUserInfo;
    if (!bundleInfo.GetInnerBundleUserInfo(userId_, innerBundleUserInfo)) {
        APP_LOGE("oldInfo do not have user");
        return ERR_APPEXECFWK_USER_NOT_EXIST;
    }

    // update haps
    for (; it != newInfos.end(); ++it) {
        modulePath_ = it->first;
        InnerBundleInfo &newInfo = it->second;
        newInfo.AddInnerBundleUserInfo(innerBundleUserInfo);
        bool isReplace = (installParam.installFlag == InstallFlag::REPLACE_EXISTING);
        // app exist, but module may not
        if ((result = ProcessBundleUpdateStatus(
            bundleInfo, newInfo, isReplace, installParam.noSkipsKill)) != ERR_OK) {
            break;
        }
    }

    uid = bundleInfo.GetUid(userId_);
    mainAbility_ = bundleInfo.GetMainAbility();
    return result;
}

uint32_t BaseBundleInstaller::CreateAccessTokenId(const InnerBundleInfo &info)
{
    return BundlePermissionMgr::CreateAccessTokenId(info, bundleName_, userId_);
}

ErrCode BaseBundleInstaller::GrantRequestPermissions(const InnerBundleInfo &info, const uint32_t tokenId)
{
    if (!BundlePermissionMgr::GrantRequestPermissions(info, tokenId)) {
        APP_LOGE("GrantRequestPermissions failed");
        return ERR_APPEXECFWK_INSTALL_GRANT_REQUEST_PERMISSIONS_FAILED;
    }
    return ERR_OK;
}

ErrCode BaseBundleInstaller::ProcessBundleInstall(const std::vector<std::string> &inBundlePaths,
    const InstallParam &installParam, const Constants::AppType appType, int32_t &uid)
{
    APP_LOGD("ProcessBundleInstall bundlePath install");
    if (!dataMgr_) {
        dataMgr_ = DelayedSingleton<BundleMgrService>::GetInstance()->GetDataMgr();
        if (!dataMgr_) {
            APP_LOGE("Get dataMgr shared_ptr nullptr");
            return ERR_APPEXECFWK_UNINSTALL_BUNDLE_MGR_SERVICE_ERROR;
        }
    }

    userId_ = GetUserId(installParam);
    if (userId_ == Constants::INVALID_USERID) {
        return ERR_APPEXECFWK_INSTALL_PARAM_ERROR;
    }

    if (!dataMgr_->HasUserId(userId_)) {
        APP_LOGE("The user %{public}d does not exist when install.", userId_);
        return ERR_APPEXECFWK_USER_NOT_EXIST;
    }

    std::vector<std::string> bundlePaths;
    // check hap paths
    ErrCode result = BundleUtil::CheckFilePath(inBundlePaths, bundlePaths);
    CHECK_RESULT_WITHOUT_ROLLBACK(result, "hap file check failed %{public}d");
    UpdateInstallerState(InstallerState::INSTALL_BUNDLE_CHECKED);                  // ---- 5%

    // check syscap
    result = CheckSysCap(bundlePaths);
    CHECK_RESULT_WITHOUT_ROLLBACK(result, "hap syscap check failed %{public}d");
    UpdateInstallerState(InstallerState::INSTALL_SYSCAP_CHECKED);                  // ---- 10%

    // verify signature info for all haps
    std::vector<Security::Verify::HapVerifyResult> hapVerifyResults;
    result = CheckMultipleHapsSignInfo(bundlePaths, installParam, hapVerifyResults);
    CHECK_RESULT_WITHOUT_ROLLBACK(result, "hap files check signature info failed %{public}d");
    UpdateInstallerState(InstallerState::INSTALL_SIGNATURE_CHECKED);               // ---- 15%

    // parse the bundle infos for all haps
    // key is bundlePath , value is innerBundleInfo
    std::unordered_map<std::string, InnerBundleInfo> newInfos;
    result = ParseHapFiles(bundlePaths, installParam, appType, hapVerifyResults, newInfos);
    CHECK_RESULT_WITHOUT_ROLLBACK(result, "parse haps file failed %{public}d");
    UpdateInstallerState(InstallerState::INSTALL_PARSED);                          // ---- 20%

    // check versioncode and bundleName
    result = CheckAppLabelInfo(newInfos);
    CHECK_RESULT_WITHOUT_ROLLBACK(result, "verisoncode or bundleName is different in all haps %{public}d");
    UpdateInstallerState(InstallerState::INSTALL_VERSION_AND_BUNDLENAME_CHECKED);  // ---- 30%

    // this state should always be set when return
    ScopeGuard stateGuard([&] { dataMgr_->UpdateBundleInstallState(bundleName_, InstallState::INSTALL_SUCCESS); });

    // this state should always be set when return
    ScopeGuard enableGuard([&] { dataMgr_->EnableBundle(bundleName_); });
    InnerBundleInfo oldInfo;
    result = InnerProcessBundleInstall(newInfos, oldInfo, installParam, uid);
    CHECK_RESULT_WITH_ROLLBACK(result, "internal processing failed with result %{public}d", newInfos, oldInfo);
    UpdateInstallerState(InstallerState::INSTALL_INFO_SAVED);                      // ---- 80%

    // rename for all temp dirs
    for (const auto &info : newInfos) {
        if (info.second.IsOnlyCreateBundleUser()) {
            continue;
        }
        if ((result = RenameModuleDir(info.second)) != ERR_OK) {
            break;
        }
    }
    UpdateInstallerState(InstallerState::INSTALL_RENAMED);                         // ---- 90%

    CHECK_RESULT_WITH_ROLLBACK(result, "rename temp dirs failed with result %{public}d", newInfos, oldInfo);
    if (!uninstallModuleVec_.empty()) {
        UninstallLowerVersionFeature(uninstallModuleVec_);
    }

    UpdateInstallerState(InstallerState::INSTALL_SUCCESS);                         // ---- 100%
    APP_LOGD("finish ProcessBundleInstall bundlePath install");
    return result;
}

void BaseBundleInstaller::RollBack(const std::unordered_map<std::string, InnerBundleInfo> &newInfos,
    InnerBundleInfo &oldInfo)
{
    APP_LOGD("start rollback due to install failed");
    if (!isAppExist_) {
        RemoveBundleAndDataDir(newInfos.begin()->second, false);
        // delete accessTokenId
        if (BundlePermissionMgr::DeleteAccessTokenId(newInfos.begin()->second.GetAccessTokenId(userId_)) !=
            AccessToken::AccessTokenKitRet::RET_SUCCESS) {
            APP_LOGE("delete accessToken failed");
        }
        // remove innerBundleInfo
        RemoveInfo(bundleName_, "");
        return;
    }
    InnerBundleInfo preInfo;
    bool isExist = false;
    if (!GetInnerBundleInfo(preInfo, isExist) || !isExist) {
        APP_LOGI("finish rollback due to install failed");
        return;
    }
    for (const auto &info : newInfos) {
        RollBack(info.second, oldInfo);
    }
    // need delete definePermissions and requestPermissions
    ErrCode ret = UpdateDefineAndRequestPermissions(preInfo, oldInfo);
    if (ret != ERR_OK) {
        return;
    }
    APP_LOGD("finish rollback due to install failed");
}

ErrCode BaseBundleInstaller::UpdateDefineAndRequestPermissions(const InnerBundleInfo &oldInfo,
    const InnerBundleInfo &newInfo)
{
    APP_LOGD("UpdateDefineAndRequestPermissions %{public}s start", bundleName_.c_str());
    auto bundleUserInfos = newInfo.GetInnerBundleUserInfos();
    for (const auto &uerInfo : bundleUserInfos) {
        if (uerInfo.second.accessTokenId == 0) {
            continue;
        }
        std::vector<std::string> newRequestPermName;
        if (!BundlePermissionMgr::UpdateDefineAndRequestPermissions(uerInfo.second.accessTokenId, oldInfo,
            newInfo, newRequestPermName)) {
            APP_LOGE("UpdateDefineAndRequestPermissions %{public}s failed", bundleName_.c_str());
            return ERR_APPEXECFWK_INSTALL_UPDATE_HAP_TOKEN_FAILED;
        }
        if (!BundlePermissionMgr::GrantRequestPermissions(newInfo, newRequestPermName, uerInfo.second.accessTokenId)) {
            APP_LOGE("BundlePermissionMgr::GrantRequestPermissions failed %{public}s", bundleName_.c_str());
            return ERR_APPEXECFWK_INSTALL_GRANT_REQUEST_PERMISSIONS_FAILED;
        }
    }
    APP_LOGD("UpdateDefineAndRequestPermissions %{public}s end", bundleName_.c_str());
    return ERR_OK;
}

void BaseBundleInstaller::RollBack(const InnerBundleInfo &info, InnerBundleInfo &oldInfo)
{
    // rollback hap installed
    if (installedModules_[info.GetCurrentModulePackage()]) {
        std::string createModulePath = info.GetAppCodePath() + Constants::PATH_SEPARATOR +
            info.GetCurrentModulePackage() + Constants::TMP_SUFFIX;
        RemoveModuleDir(createModulePath);
        oldInfo.SetCurrentModulePackage(info.GetCurrentModulePackage());
        RollBackMoudleInfo(bundleName_, oldInfo);
    } else {
        auto modulePackage = info.GetCurrentModulePackage();
        RemoveModuleDir(info.GetModuleDir(modulePackage));
        // remove module info
        RemoveInfo(bundleName_, modulePackage);
    }
}

void BaseBundleInstaller::RemoveInfo(const std::string &bundleName, const std::string &packageName)
{
    APP_LOGD("remove innerBundleInfo due to rollback");
    if (packageName.empty()) {
        dataMgr_->UpdateBundleInstallState(bundleName, InstallState::UPDATING_FAIL);
    } else {
        InnerBundleInfo innerBundleInfo;
        bool isExist = false;
        if (!GetInnerBundleInfo(innerBundleInfo, isExist) || !isExist) {
            APP_LOGI("finish rollback due to install failed");
            return;
        }
        dataMgr_->UpdateBundleInstallState(bundleName, InstallState::ROLL_BACK);
        dataMgr_->RemoveModuleInfo(bundleName, packageName, innerBundleInfo);
    }
    APP_LOGD("finish to remove innerBundleInfo due to rollback");
}

void BaseBundleInstaller::RollBackMoudleInfo(const std::string &bundleName, InnerBundleInfo &oldInfo)
{
    APP_LOGD("rollBackMoudleInfo due to rollback");
    InnerBundleInfo innerBundleInfo;
    bool isExist = false;
    if (!GetInnerBundleInfo(innerBundleInfo, isExist) || !isExist) {
        return;
    }
    dataMgr_->UpdateBundleInstallState(bundleName, InstallState::ROLL_BACK);
    dataMgr_->UpdateInnerBundleInfo(bundleName, oldInfo, innerBundleInfo);
    APP_LOGD("finsih rollBackMoudleInfo due to rollback");
}

ErrCode BaseBundleInstaller::ProcessBundleUninstall(
    const std::string &bundleName, const InstallParam &installParam, int32_t &uid)
{
    APP_LOGD("start to process %{public}s bundle uninstall", bundleName.c_str());
    if (bundleName.empty()) {
        APP_LOGE("uninstall bundle name empty");
        return ERR_APPEXECFWK_UNINSTALL_INVALID_NAME;
    }

    dataMgr_ = DelayedSingleton<BundleMgrService>::GetInstance()->GetDataMgr();
    if (!dataMgr_) {
        APP_LOGE("Get dataMgr shared_ptr nullptr");
        return ERR_APPEXECFWK_UNINSTALL_BUNDLE_MGR_SERVICE_ERROR;
    }

    userId_ = GetUserId(installParam);
    if (userId_ == Constants::INVALID_USERID) {
        return ERR_APPEXECFWK_INSTALL_PARAM_ERROR;
    }

    if (!dataMgr_->HasUserId(userId_)) {
        APP_LOGE("The user %{public}d does not exist when uninstall.", userId_);
        return ERR_APPEXECFWK_USER_NOT_EXIST;
    }

    auto &mtx = dataMgr_->GetBundleMutex(bundleName);
    std::lock_guard lock {mtx};
    InnerBundleInfo oldInfo;
    if (!dataMgr_->GetInnerBundleInfo(bundleName, Constants::CURRENT_DEVICE_ID, oldInfo)) {
        APP_LOGE("uninstall bundle info missing");
        return ERR_APPEXECFWK_UNINSTALL_MISSING_INSTALLED_BUNDLE;
    }

    ScopeGuard enableGuard([&] { dataMgr_->EnableBundle(bundleName); });
    InnerBundleUserInfo curInnerBundleUserInfo;
    if (!oldInfo.GetInnerBundleUserInfo(userId_, curInnerBundleUserInfo)) {
        APP_LOGE("bundle(%{public}s) get user(%{public}d) failed when uninstall.",
            oldInfo.GetBundleName().c_str(), userId_);
        return ERR_APPEXECFWK_USER_NOT_INSTALL_HAP;
    }

    uid = curInnerBundleUserInfo.uid;
    oldInfo.SetIsKeepData(installParam.isKeepData);
    if (!installParam.forceExecuted && oldInfo.GetBaseApplicationInfo().isSystemApp &&
        !oldInfo.IsRemovable() && installParam.noSkipsKill) {
        APP_LOGE("uninstall system app");
        return ERR_APPEXECFWK_UNINSTALL_SYSTEM_APP_ERROR;
    }

    std::string cloneName;
    if (dataMgr_->GetClonedBundleName(bundleName, cloneName)) {
        APP_LOGI("GetClonedBundleName new name %{public}s ", cloneName.c_str());
        cloneMgr_->RemoveClonedBundle(bundleName, cloneName);
    }

    if (oldInfo.GetInnerBundleUserInfos().size() > 1) {
        APP_LOGD("only delete userinfo %{public}d", userId_);
        return RemoveBundleUserData(oldInfo);
    }

    if (!dataMgr_->UpdateBundleInstallState(bundleName, InstallState::UNINSTALL_START)) {
        APP_LOGE("uninstall already start");
        return ERR_APPEXECFWK_INSTALL_BUNDLE_MGR_SERVICE_ERROR;
    }

    // reboot scan case will not kill the bundle
    if (installParam.noSkipsKill) {
        // kill the bundle process during uninstall.
        if (!UninstallApplicationProcesses(oldInfo.GetApplicationName(), uid)) {
            APP_LOGE("can not kill process");
            dataMgr_->UpdateBundleInstallState(bundleName, InstallState::INSTALL_SUCCESS);
            return ERR_APPEXECFWK_UNINSTALL_KILLING_APP_ERROR;
        }
    }

    enableGuard.Dismiss();
    std::string packageName;
    oldInfo.SetInstallMark(bundleName, packageName, InstallExceptionStatus::UNINSTALL_BUNDLE_START);
    if (!dataMgr_->SaveInstallMark(oldInfo, true)) {
        APP_LOGE("save install mark failed");
        return ERR_APPEXECFWK_INSTALL_INTERNAL_ERROR;
    }

    ErrCode result = RemoveBundle(oldInfo);
    if (result != ERR_OK) {
        APP_LOGE("remove whole bundle failed");
        return result;
    }

    APP_LOGD("finish to process %{public}s bundle uninstall", bundleName.c_str());
    return ERR_OK;
}

ErrCode BaseBundleInstaller::ProcessBundleUninstall(
    const std::string &bundleName, const std::string &modulePackage, const InstallParam &installParam, int32_t &uid)
{
    APP_LOGD("start to process %{public}s in %{public}s uninstall", bundleName.c_str(), modulePackage.c_str());
    if (bundleName.empty() || modulePackage.empty()) {
        APP_LOGE("uninstall bundle name or module name empty");
        return ERR_APPEXECFWK_UNINSTALL_INVALID_NAME;
    }

    dataMgr_ = DelayedSingleton<BundleMgrService>::GetInstance()->GetDataMgr();
    cloneMgr_ = DelayedSingleton<BundleMgrService>::GetInstance()->GetCloneMgr();
    if (!dataMgr_) {
        APP_LOGE("Get dataMgr shared_ptr nullptr");
        return ERR_APPEXECFWK_UNINSTALL_BUNDLE_MGR_SERVICE_ERROR;
    }

    userId_ = GetUserId(installParam);
    if (userId_ == Constants::INVALID_USERID) {
        return ERR_APPEXECFWK_INSTALL_PARAM_ERROR;
    }

    if (!dataMgr_->HasUserId(userId_)) {
        APP_LOGE("The user %{public}d does not exist when uninstall.", userId_);
        return ERR_APPEXECFWK_USER_NOT_EXIST;
    }

    auto &mtx = dataMgr_->GetBundleMutex(bundleName);
    std::lock_guard lock {mtx};
    InnerBundleInfo oldInfo;
    if (!dataMgr_->GetInnerBundleInfo(bundleName, Constants::CURRENT_DEVICE_ID, oldInfo)) {
        APP_LOGE("uninstall bundle info missing");
        return ERR_APPEXECFWK_UNINSTALL_MISSING_INSTALLED_BUNDLE;
    }

    ScopeGuard enableGuard([&] { dataMgr_->EnableBundle(bundleName); });
    InnerBundleUserInfo curInnerBundleUserInfo;
    if (!oldInfo.GetInnerBundleUserInfo(userId_, curInnerBundleUserInfo)) {
        APP_LOGE("bundle(%{public}s) get user(%{public}d) failed when uninstall.",
            oldInfo.GetBundleName().c_str(), userId_);
        return ERR_APPEXECFWK_USER_NOT_INSTALL_HAP;
    }

    uid = curInnerBundleUserInfo.uid;
    oldInfo.SetIsKeepData(installParam.isKeepData);
    if (!installParam.forceExecuted && oldInfo.GetBaseApplicationInfo().isSystemApp
        && !oldInfo.IsRemovable() && installParam.noSkipsKill) {
        APP_LOGE("uninstall system app");
        return ERR_APPEXECFWK_UNINSTALL_SYSTEM_APP_ERROR;
    }

    bool isModuleExist = oldInfo.FindModule(modulePackage);
    if (!isModuleExist) {
        APP_LOGE("uninstall bundle info missing");
        return ERR_APPEXECFWK_UNINSTALL_MISSING_INSTALLED_MODULE;
    }

    if (!dataMgr_->UpdateBundleInstallState(bundleName, InstallState::UNINSTALL_START)) {
        APP_LOGE("uninstall already start");
        return ERR_APPEXECFWK_INSTALL_BUNDLE_MGR_SERVICE_ERROR;
    }

    ScopeGuard stateGuard([&] { dataMgr_->UpdateBundleInstallState(bundleName, InstallState::INSTALL_SUCCESS); });

    // reboot scan case will not kill the bundle
    if (installParam.noSkipsKill) {
        // kill the bundle process during uninstall.
        if (!UninstallApplicationProcesses(oldInfo.GetApplicationName(), uid)) {
            APP_LOGE("can not kill process");
            return ERR_APPEXECFWK_UNINSTALL_KILLING_APP_ERROR;
        }
    }

    oldInfo.SetInstallMark(bundleName, modulePackage, InstallExceptionStatus::UNINSTALL_PACKAGE_START);
    if (!dataMgr_->SaveInstallMark(oldInfo, true)) {
        APP_LOGE("save install mark failed");
        return ERR_APPEXECFWK_INSTALL_INTERNAL_ERROR;
    }

    bool onlyInstallInUser = oldInfo.GetInnerBundleUserInfos().size() == 1;
    // if it is the only module in the bundle
    if (oldInfo.IsOnlyModule(modulePackage)) {
        APP_LOGI("%{public}s is only module", modulePackage.c_str());
        std::string cloneName;
        if (dataMgr_->GetClonedBundleName(bundleName, cloneName)) {
            APP_LOGI("GetClonedBundleName new name %{public}s ", cloneName.c_str());
            cloneMgr_->RemoveClonedBundle(bundleName, cloneName);
        }

        enableGuard.Dismiss();
        stateGuard.Dismiss();
        if (onlyInstallInUser) {
            return RemoveBundle(oldInfo);
        }

        return RemoveBundleUserData(oldInfo);
    }

    ErrCode result = ERR_OK;
    if (onlyInstallInUser) {
        result = RemoveModuleAndDataDir(oldInfo, modulePackage, userId_);
    } else {
        result = RemoveHapModuleDataDir(oldInfo, modulePackage, userId_);
    }

    if (result != ERR_OK) {
        APP_LOGE("remove module dir failed");
        return result;
    }

    oldInfo.SetInstallMark(bundleName, modulePackage, InstallExceptionStatus::INSTALL_FINISH);
    if (!dataMgr_->RemoveModuleInfo(bundleName, modulePackage, oldInfo)) {
        APP_LOGE("RemoveModuleInfo failed");
        return ERR_APPEXECFWK_INSTALL_BUNDLE_MGR_SERVICE_ERROR;
    }

    APP_LOGD("finish to process %{public}s in %{public}s uninstall", bundleName.c_str(), modulePackage.c_str());
    return ERR_OK;
}

ErrCode BaseBundleInstaller::ProcessRecover(
    const std::string &bundleName, const InstallParam &installParam, int32_t &uid)
{
    APP_LOGD("Install Bundle by bundleName start, bundleName: %{public}s.", bundleName.c_str());
    dataMgr_ = DelayedSingleton<BundleMgrService>::GetInstance()->GetDataMgr();
    if (!dataMgr_) {
        APP_LOGE("Get dataMgr shared_ptr nullptr.");
        return ERR_APPEXECFWK_UNINSTALL_BUNDLE_MGR_SERVICE_ERROR;
    }

    userId_ = GetUserId(installParam);
    if (userId_ == Constants::INVALID_USERID) {
        return ERR_APPEXECFWK_INSTALL_PARAM_ERROR;
    }

    if (!dataMgr_->HasUserId(userId_)) {
        APP_LOGE("The user %{public}d does not exist when recover.", userId_);
        return ERR_APPEXECFWK_USER_NOT_EXIST;
    }

    {
        auto &mtx = dataMgr_->GetBundleMutex(bundleName);
        std::lock_guard lock {mtx};
        InnerBundleInfo oldInfo;
        bool isAppExist = dataMgr_->GetInnerBundleInfo(bundleName, Constants::CURRENT_DEVICE_ID, oldInfo);
        if (isAppExist) {
            dataMgr_->EnableBundle(bundleName);
            if (oldInfo.HasInnerBundleUserInfo(userId_)) {
                APP_LOGE("App is exist in user(%{public}d) when recover.", userId_);
                return ERR_APPEXECFWK_INSTALL_ALREADY_EXIST;
            }

            InnerBundleUserInfo curInnerBundleUserInfo;
            curInnerBundleUserInfo.bundleUserInfo.userId = userId_;
            curInnerBundleUserInfo.bundleName = bundleName;
            oldInfo.AddInnerBundleUserInfo(curInnerBundleUserInfo);
            uint32_t tokenId = CreateAccessTokenId(oldInfo);
            oldInfo.SetAccessTokenId(tokenId, userId_);
            ErrCode result = GrantRequestPermissions(oldInfo, tokenId);
            if (result != ERR_OK) {
                return result;
            }
            return CreateBundleUserData(oldInfo, true);
        }
    }

    PreInstallBundleInfo preInstallBundleInfo;
    preInstallBundleInfo.SetBundleName(bundleName);
    if (!dataMgr_->GetPreInstallBundleInfo(bundleName, preInstallBundleInfo)
        || preInstallBundleInfo.GetBundlePaths().empty()
        || preInstallBundleInfo.GetAppType() != Constants::AppType::SYSTEM_APP) {
        APP_LOGE("Get PreInstallBundleInfo faile, bundleName: %{public}s.", bundleName.c_str());
        return ERR_APPEXECFWK_RECOVER_GET_BUNDLEPATH_ERROR;
    }

    APP_LOGD("Get bundlePath success when recover.");
    std::vector<std::string> pathVec { preInstallBundleInfo.GetBundlePaths() };
    auto recoverInstallParam = installParam;
    recoverInstallParam.isPreInstallApp = true;
    return ProcessBundleInstall(
        pathVec, recoverInstallParam, Constants::AppType::SYSTEM_APP, uid);
}

ErrCode BaseBundleInstaller::RemoveBundle(InnerBundleInfo &info)
{
    ErrCode result = RemoveBundleAndDataDir(info, true);
    if (result != ERR_OK) {
        APP_LOGE("remove bundle dir failed");
        dataMgr_->UpdateBundleInstallState(info.GetBundleName(), InstallState::UNINSTALL_FAIL);
        return result;
    }

    if (!dataMgr_->UpdateBundleInstallState(info.GetBundleName(), InstallState::UNINSTALL_SUCCESS)) {
        APP_LOGE("delete inner info failed");
        return ERR_APPEXECFWK_INSTALL_BUNDLE_MGR_SERVICE_ERROR;
    }

    if (BundlePermissionMgr::DeleteAccessTokenId(info.GetAccessTokenId(userId_)) !=
        AccessToken::AccessTokenKitRet::RET_SUCCESS) {
        APP_LOGE("delete accessToken failed");
    }
    return ERR_OK;
}

ErrCode BaseBundleInstaller::ProcessBundleInstallStatus(InnerBundleInfo &info, int32_t &uid)
{
    if (!verifyUriPrefix(info, userId_)) {
        APP_LOGE("verifyUriPrefix failed");
        return ERR_APPEXECFWK_INSTALL_URI_DUPLICATE;
    }
    modulePackage_ = info.GetCurrentModulePackage();
    APP_LOGD("ProcessBundleInstallStatus with bundleName %{public}s and packageName %{public}s",
        bundleName_.c_str(), modulePackage_.c_str());
    if (!dataMgr_->UpdateBundleInstallState(bundleName_, InstallState::INSTALL_START)) {
        APP_LOGE("install already start");
        return ERR_APPEXECFWK_INSTALL_STATE_ERROR;
    }
    info.SetInstallMark(bundleName_, modulePackage_, InstallExceptionStatus::INSTALL_START);
    if (!dataMgr_->SaveInstallMark(info, isAppExist_)) {
        APP_LOGE("save install mark to storage failed");
        return ERR_APPEXECFWK_INSTALL_INTERNAL_ERROR;
    }
    ScopeGuard stateGuard([&] { dataMgr_->UpdateBundleInstallState(bundleName_, InstallState::INSTALL_FAIL); });
    ErrCode result = CreateBundleAndDataDir(info);
    if (result != ERR_OK) {
        APP_LOGE("create bundle and data dir failed");
        return result;
    }

    ScopeGuard bundleGuard([&] { RemoveBundleAndDataDir(info, false); });
    std::string modulePath = info.GetAppCodePath() + Constants::PATH_SEPARATOR + modulePackage_;
    result = ExtractModule(info, modulePath);
    if (result != ERR_OK) {
        APP_LOGE("extract module failed");
        return result;
    }

    info.SetInstallMark(bundleName_, modulePackage_, InstallExceptionStatus::INSTALL_FINISH);
    uid = info.GetUid(userId_);
    info.SetBundleInstallTime(BundleUtil::GetCurrentTime(), userId_);
    uint32_t tokenId = CreateAccessTokenId(info);
    info.SetAccessTokenId(tokenId, userId_);
    result = GrantRequestPermissions(info, tokenId);
    if (result != ERR_OK) {
        return result;
    }
    if (!dataMgr_->AddInnerBundleInfo(bundleName_, info)) {
        APP_LOGE("add bundle %{public}s info failed", bundleName_.c_str());
        dataMgr_->UpdateBundleInstallState(bundleName_, InstallState::UNINSTALL_START);
        dataMgr_->UpdateBundleInstallState(bundleName_, InstallState::UNINSTALL_SUCCESS);
        return ERR_APPEXECFWK_INSTALL_BUNDLE_MGR_SERVICE_ERROR;
    }

    stateGuard.Dismiss();
    bundleGuard.Dismiss();

    APP_LOGD("finish to call processBundleInstallStatus");
    return ERR_OK;
}

ErrCode BaseBundleInstaller::ProcessBundleUpdateStatus(
    InnerBundleInfo &oldInfo, InnerBundleInfo &newInfo, bool isReplace, bool noSkipsKill)
{
    modulePackage_ = newInfo.GetCurrentModulePackage();
    if (modulePackage_.empty()) {
        APP_LOGE("get current package failed");
        return ERR_APPEXECFWK_INSTALL_PARAM_ERROR;
    }
    if (isFeatureNeedUninstall_) {
        uninstallModuleVec_.emplace_back(modulePackage_);
    }
    APP_LOGD("ProcessBundleUpdateStatus with bundleName %{public}s and packageName %{public}s",
        newInfo.GetBundleName().c_str(), modulePackage_.c_str());

    if (!dataMgr_->UpdateBundleInstallState(bundleName_, InstallState::UPDATING_START)) {
        APP_LOGE("update already start");
        return ERR_APPEXECFWK_INSTALL_STATE_ERROR;
    }

    if (oldInfo.GetProvisionId() != newInfo.GetProvisionId()) {
        APP_LOGE("the signature of the new bundle is not the same as old one");
        return ERR_APPEXECFWK_INSTALL_FAILED_INCONSISTENT_SIGNATURE;
    }
    APP_LOGD("ProcessBundleUpdateStatus noSkipsKill = %{public}d", noSkipsKill);
    // now there are two cases for updating:
    // 1. bundle exist, hap exist, update hap
    // 2. bundle exist, install new hap
    bool isModuleExist = oldInfo.FindModule(modulePackage_);
    newInfo.RestoreFromOldInfo(oldInfo);
    auto result = isModuleExist ? ProcessModuleUpdate(newInfo, oldInfo,
        isReplace, noSkipsKill) : ProcessNewModuleInstall(newInfo, oldInfo);
    if (result != ERR_OK) {
        APP_LOGE("install module failed %{public}d", result);
        return result;
    }

    APP_LOGD("finish to call ProcessBundleUpdateStatus");
    return ERR_OK;
}

ErrCode BaseBundleInstaller::ProcessNewModuleInstall(InnerBundleInfo &newInfo, InnerBundleInfo &oldInfo)
{
    APP_LOGD("ProcessNewModuleInstall %{public}s, userId: %{public}d.",
        newInfo.GetBundleName().c_str(), userId_);
    ScopeGuard userGuard([&] {
        if (!hasInstalledInUser_) {
            RemoveBundleUserData(oldInfo);
        }
    });

    if (!verifyUriPrefix(newInfo, userId_)) {
        APP_LOGE("verifyUriPrefix failed");
        return ERR_APPEXECFWK_INSTALL_URI_DUPLICATE;
    }

    if (newInfo.HasEntry() && oldInfo.HasEntry()) {
        APP_LOGE("install more than one entry module");
        return ERR_APPEXECFWK_INSTALL_ENTRY_ALREADY_EXIST;
    }

    oldInfo.SetInstallMark(bundleName_, modulePackage_, InstallExceptionStatus::UPDATING_NEW_START);
    if (!dataMgr_->SaveInstallMark(oldInfo, true)) {
        APP_LOGE("save install mark failed");
        return ERR_APPEXECFWK_INSTALL_INTERNAL_ERROR;
    }
    std::string modulePath = newInfo.GetAppCodePath() + Constants::PATH_SEPARATOR + modulePackage_;
    ErrCode result = ExtractModule(newInfo, modulePath);
    if (result != ERR_OK) {
        APP_LOGE("extract module and rename failed");
        return result;
    }
    ScopeGuard moduleGuard([&] { RemoveModuleDir(modulePath); });
    if (!dataMgr_->UpdateBundleInstallState(bundleName_, InstallState::UPDATING_SUCCESS)) {
        APP_LOGE("new moduleupdate state failed");
        return ERR_APPEXECFWK_INSTALL_BUNDLE_MGR_SERVICE_ERROR;
    }

    oldInfo.SetInstallMark(bundleName_, modulePackage_, InstallExceptionStatus::INSTALL_FINISH);

    auto bundleUserInfos = oldInfo.GetInnerBundleUserInfos();
    for (const auto &info : bundleUserInfos) {
        if (info.second.accessTokenId == 0) {
            continue;
        }
        std::vector<std::string> newRequestPermName;
        if (!BundlePermissionMgr::AddDefineAndRequestPermissions(info.second.accessTokenId, newInfo,
            newRequestPermName)) {
            APP_LOGE("BundlePermissionMgr::AddDefineAndRequestPermissions failed %{public}s", bundleName_.c_str());
            return ERR_APPEXECFWK_INSTALL_UPDATE_HAP_TOKEN_FAILED;
        }
        if (!BundlePermissionMgr::GrantRequestPermissions(newInfo, newRequestPermName, info.second.accessTokenId)) {
            APP_LOGE("BundlePermissionMgr::GrantRequestPermissions failed %{public}s", bundleName_.c_str());
            return ERR_APPEXECFWK_INSTALL_GRANT_REQUEST_PERMISSIONS_FAILED;
        }
    }

    oldInfo.SetBundleUpdateTime(BundleUtil::GetCurrentTime(), userId_);
    if (!dataMgr_->AddNewModuleInfo(bundleName_, newInfo, oldInfo)) {
        APP_LOGE(
            "add module %{public}s to innerBundleInfo %{public}s failed", modulePackage_.c_str(), bundleName_.c_str());
        return ERR_APPEXECFWK_INSTALL_BUNDLE_MGR_SERVICE_ERROR;
    }

    moduleGuard.Dismiss();
    userGuard.Dismiss();
    return ERR_OK;
}

ErrCode BaseBundleInstaller::ProcessModuleUpdate(InnerBundleInfo &newInfo,
    InnerBundleInfo &oldInfo, bool isReplace, bool noSkipsKill)
{
    APP_LOGD("ProcessModuleUpdate, bundleName : %{public}s, moduleName : %{public}s, userId: %{public}d.",
        newInfo.GetBundleName().c_str(), newInfo.GetCurrentModulePackage().c_str(), userId_);
    ScopeGuard userGuard([&] {
        if (!hasInstalledInUser_) {
            RemoveBundleUserData(oldInfo);
        }
    });

    if (!verifyUriPrefix(newInfo, userId_, true)) {
        APP_LOGE("verifyUriPrefix failed");
        return ERR_APPEXECFWK_INSTALL_URI_DUPLICATE;
    }

    if (!isReplace && versionCode_ == oldInfo.GetVersionCode()) {
        if (hasInstalledInUser_) {
            APP_LOGE("fail to install already existing bundle using normal flag");
            return ERR_APPEXECFWK_INSTALL_ALREADY_EXIST;
        }

        // app versionCode equals to the old and do not need to update module
        // and only need to update userInfo
        newInfo.SetOnlyCreateBundleUser(true);
        userGuard.Dismiss();
        return ERR_OK;
    }
    APP_LOGE("ProcessModuleUpdate noSkipsKill = %{public}d", noSkipsKill);
    // reboot scan case will not kill the bundle
    if (noSkipsKill) {
        // kill the bundle process during updating
        if (!UninstallApplicationProcesses(oldInfo.GetApplicationName(), oldInfo.GetUid(userId_))) {
            APP_LOGE("fail to kill running application");
            return ERR_APPEXECFWK_INSTALL_INTERNAL_ERROR;
    }
    }
    oldInfo.SetInstallMark(bundleName_, modulePackage_, InstallExceptionStatus::UPDATING_EXISTED_START);
    if (!dataMgr_->SaveInstallMark(oldInfo, true)) {
        APP_LOGE("save install mark failed");
        return ERR_APPEXECFWK_INSTALL_INTERNAL_ERROR;
    }
    moduleTmpDir_ = newInfo.GetAppCodePath() + Constants::PATH_SEPARATOR + modulePackage_ + Constants::TMP_SUFFIX;
    ErrCode result = ExtractModule(newInfo, moduleTmpDir_);
    if (result != ERR_OK) {
        APP_LOGE("extract module and rename failed");
        return result;
    }
    if (!dataMgr_->UpdateBundleInstallState(bundleName_, InstallState::UPDATING_SUCCESS)) {
        APP_LOGE("old module update state failed");
        return ERR_APPEXECFWK_INSTALL_BUNDLE_MGR_SERVICE_ERROR;
    }

    newInfo.RestoreModuleInfo(oldInfo);
    oldInfo.SetInstallMark(bundleName_, modulePackage_, InstallExceptionStatus::UPDATING_FINISH);
    oldInfo.SetBundleUpdateTime(BundleUtil::GetCurrentTime(), userId_);
    auto noUpdateInfo = oldInfo;
    if (!dataMgr_->UpdateInnerBundleInfo(bundleName_, newInfo, oldInfo)) {
        APP_LOGE("update innerBundleInfo %{public}s failed", bundleName_.c_str());
        return ERR_APPEXECFWK_INSTALL_BUNDLE_MGR_SERVICE_ERROR;
    }
    auto bundleUserInfos = oldInfo.GetInnerBundleUserInfos();
    for (const auto &info : bundleUserInfos) {
        if (info.second.accessTokenId == 0) {
            continue;
        }
        std::vector<std::string> newRequestPermName;
        if (!BundlePermissionMgr::UpdateDefineAndRequestPermissions(info.second.accessTokenId, noUpdateInfo,
            oldInfo, newRequestPermName)) {
            APP_LOGE("UpdateDefineAndRequestPermissions %{public}s failed", bundleName_.c_str());
            return ERR_APPEXECFWK_INSTALL_UPDATE_HAP_TOKEN_FAILED;
        }
        if (!BundlePermissionMgr::GrantRequestPermissions(oldInfo, newRequestPermName, info.second.accessTokenId)) {
            APP_LOGE("BundlePermissionMgr::GrantRequestPermissions failed %{public}s", bundleName_.c_str());
            return ERR_APPEXECFWK_INSTALL_GRANT_REQUEST_PERMISSIONS_FAILED;
        }
    }
    ErrCode ret = SetDirApl(newInfo);
    if (ret != ERR_OK) {
        APP_LOGE("SetDirApl failed");
        return ret;
    }
    userGuard.Dismiss();
    return ERR_OK;
}

ErrCode BaseBundleInstaller::SetDirApl(const InnerBundleInfo &info)
{
    for (const auto &el : Constants::BUNDLE_EL) {
        std::string baseBundleDataDir = Constants::BUNDLE_APP_DATA_BASE_DIR +
                                        el +
                                        Constants::FILE_SEPARATOR_CHAR +
                                        std::to_string(userId_);
        std::string baseDataDir = baseBundleDataDir + Constants::BASE + info.GetBundleName();
        ErrCode result = InstalldClient::GetInstance()->SetDirApl(
            baseDataDir, info.GetBundleName(), info.GetAppPrivilegeLevel());
        if (result != ERR_OK) {
            APP_LOGE("fail to SetDirApl baseDir dir, error is %{public}d", result);
            return result;
        }
        std::string databaseDataDir = baseBundleDataDir + Constants::DATABASE + info.GetBundleName();
        result = InstalldClient::GetInstance()->SetDirApl(
            databaseDataDir, info.GetBundleName(), info.GetAppPrivilegeLevel());
        if (result != ERR_OK) {
            APP_LOGE("fail to SetDirApl databaseDir dir, error is %{public}d", result);
            return result;
        }
    }

    return ERR_OK;
}

ErrCode BaseBundleInstaller::CreateBundleAndDataDir(InnerBundleInfo &info) const
{
    ErrCode result = CreateBundleCodeDir(info);
    if (result != ERR_OK) {
        APP_LOGE("fail to create bundle code dir, error is %{public}d", result);
        return result;
    }
    ScopeGuard codePathGuard([&] { InstalldClient::GetInstance()->RemoveDir(info.GetAppCodePath()); });
    result = CreateBundleDataDir(info);
    if (result != ERR_OK) {
        APP_LOGE("fail to create bundle data dir, error is %{public}d", result);
        return result;
    }
    codePathGuard.Dismiss();
    return ERR_OK;
}

ErrCode BaseBundleInstaller::CreateBundleCodeDir(InnerBundleInfo &info) const
{
    auto appCodePath = Constants::BUNDLE_CODE_DIR + Constants::PATH_SEPARATOR + bundleName_;
    APP_LOGD("create bundle dir %{private}s", appCodePath.c_str());
    ErrCode result = InstalldClient::GetInstance()->CreateBundleDir(appCodePath);
    if (result != ERR_OK) {
        APP_LOGE("fail to create bundle dir, error is %{public}d", result);
        return result;
    }

    info.SetAppCodePath(appCodePath);
    return ERR_OK;
}

ErrCode BaseBundleInstaller::CreateBundleDataDir(InnerBundleInfo &info) const
{
    InnerBundleUserInfo newInnerBundleUserInfo;
    if (!info.GetInnerBundleUserInfo(userId_, newInnerBundleUserInfo)) {
        APP_LOGE("bundle(%{public}s) get user(%{public}d) failed.",
            info.GetBundleName().c_str(), userId_);
        return ERR_APPEXECFWK_USER_NOT_EXIST;
    }

    if (!dataMgr_->GenerateUidAndGid(newInnerBundleUserInfo)) {
        APP_LOGE("fail to gererate uid and gid");
        return ERR_APPEXECFWK_INSTALL_GENERATE_UID_ERROR;
    }

    auto result = InstalldClient::GetInstance()->CreateBundleDataDir(info.GetBundleName(), userId_,
        newInnerBundleUserInfo.uid, newInnerBundleUserInfo.uid, info.GetAppPrivilegeLevel());
    if (result != ERR_OK) {
        APP_LOGE("fail to create bundle data dir, error is %{public}d", result);
        return result;
    }
    std::string dataBaseDir = Constants::BUNDLE_APP_DATA_BASE_DIR + Constants::BUNDLE_EL[1] +
        Constants::DATABASE + info.GetBundleName();
    info.SetAppDataBaseDir(dataBaseDir);
    info.AddInnerBundleUserInfo(newInnerBundleUserInfo);
    return ERR_OK;
}

ErrCode BaseBundleInstaller::ExtractModule(InnerBundleInfo &info, const std::string &modulePath)
{
    std::string targetSoPath;
    std::string nativeLibraryPath = info.GetBaseApplicationInfo().nativeLibraryPath;
    if (!nativeLibraryPath.empty()) {
        targetSoPath.append(Constants::BUNDLE_CODE_DIR).append(Constants::PATH_SEPARATOR)
            .append(info.GetBundleName()).append(Constants::PATH_SEPARATOR)
            .append(nativeLibraryPath).append(Constants::PATH_SEPARATOR);
    }
    std::string cpuAbi = info.GetBaseApplicationInfo().cpuAbi;
    APP_LOGD("begin to extract module files, modulePath : %{private}s, targetSoPath : %{private}s, cpuAbi : %{public}s",
        modulePath.c_str(), targetSoPath.c_str(), cpuAbi.c_str());
    auto result = ExtractModuleFiles(info, modulePath, targetSoPath, cpuAbi);
    if (result != ERR_OK) {
        APP_LOGE("fail to extrace module dir, error is %{public}d", result);
        return result;
    }
    auto moduleDir = info.GetAppCodePath() + Constants::PATH_SEPARATOR + info.GetCurrentModulePackage();
    info.AddModuleSrcDir(moduleDir);
    info.AddModuleResPath(moduleDir);
    return ERR_OK;
}

ErrCode BaseBundleInstaller::RemoveBundleAndDataDir(const InnerBundleInfo &info, bool isUninstall) const
{
    // remove bundle dir
    auto result = RemoveBundleCodeDir(info);
    if (result != ERR_OK) {
        APP_LOGE("fail to remove bundle dir %{private}s, error is %{public}d", info.GetAppCodePath().c_str(), result);
        return result;
    }
    if (!info.GetIsKeepData() || !isUninstall) {
        result = RemoveBundleDataDir(info);
        if (result != ERR_OK) {
            APP_LOGE("fail to remove newbundleName: %{public}s, error is %{public}d",
                info.GetBundleName().c_str(), result);
        }
    }
    return ERR_OK;
}

ErrCode BaseBundleInstaller::RemoveBundleCodeDir(const InnerBundleInfo &info) const
{
    auto result = InstalldClient::GetInstance()->RemoveDir(info.GetAppCodePath());
    if (result != ERR_OK) {
        APP_LOGE("fail to remove bundle code dir %{public}s, error is %{public}d",
            info.GetAppCodePath().c_str(), result);
    }
    return result;
}

ErrCode BaseBundleInstaller::RemoveBundleDataDir(const InnerBundleInfo &info) const
{
    ErrCode result =
        InstalldClient::GetInstance()->RemoveBundleDataDir(info.GetBundleName(), userId_);
    if (result != ERR_OK) {
        APP_LOGE("fail to remove bundleName: %{public}s, error is %{public}d",
            info.GetBundleName().c_str(), result);
    }
    return result;
}

ErrCode BaseBundleInstaller::RemoveModuleAndDataDir(
    const InnerBundleInfo &info, const std::string &modulePackage, int32_t userId) const
{
    auto moduleDir = info.GetModuleDir(modulePackage);
    auto result = RemoveModuleDir(moduleDir);
    if (result != ERR_OK) {
        APP_LOGE("fail to remove module dir, error is %{public}d", result);
        return result;
    }

    if (!info.GetIsKeepData()) {
        if (userId != Constants::UNSPECIFIED_USERID) {
            RemoveHapModuleDataDir(info, modulePackage, userId);
            return ERR_OK;
        }

        for (auto infoItem : info.GetInnerBundleUserInfos()) {
            int32_t installedUserId = infoItem.second.bundleUserInfo.userId;
            if (installedUserId == userId_) {
                continue;
            }

            RemoveHapModuleDataDir(info, modulePackage, installedUserId);
        }
    }
    return ERR_OK;
}

ErrCode BaseBundleInstaller::RemoveModuleDir(const std::string &modulePath) const
{
    APP_LOGD("module dir %{private}s to be removed", modulePath.c_str());
    return InstalldClient::GetInstance()->RemoveDir(modulePath);
}

ErrCode BaseBundleInstaller::RemoveHapModuleDataDir(
    const InnerBundleInfo &info, const std::string &modulePackage, int32_t userId) const
{
    APP_LOGD("RemoveHapModuleDataDir bundleName: %{public}s  modulePackage: %{public}s",
             info.GetBundleName().c_str(),
             modulePackage.c_str());
    auto hapModuleInfo = info.FindHapModuleInfo(modulePackage);
    if (!hapModuleInfo) {
        APP_LOGE("fail to findHapModule info modulePackage: %{public}s", modulePackage.c_str());
        return ERR_NO_INIT;
    }
    std::string moduleDataDir = info.GetBundleName() + Constants::HAPS + (*hapModuleInfo).moduleName;
    APP_LOGD("RemoveHapModuleDataDir moduleDataDir: %{public}s", moduleDataDir.c_str());
    auto result = InstalldClient::GetInstance()->RemoveModuleDataDir(moduleDataDir, userId);
    if (result != ERR_OK) {
        APP_LOGE("fail to remove HapModuleData dir, error is %{public}d", result);
    }
    return result;
}

ErrCode BaseBundleInstaller::ParseBundleInfo(const std::string &bundleFilePath, InnerBundleInfo &info) const
{
    BundleParser bundleParser;
    ErrCode result = bundleParser.Parse(bundleFilePath, info);
    if (result != ERR_OK) {
        APP_LOGE("parse bundle info failed, error: %{public}d", result);
        return result;
    }
    return ERR_OK;
}

ErrCode BaseBundleInstaller::ExtractModuleFiles(const InnerBundleInfo &info, const std::string &modulePath,
    const std::string &targetSoPath, const std::string &cpuAbi)
{
    APP_LOGD("extract module to %{private}s", modulePath.c_str());
    auto result = InstalldClient::GetInstance()->ExtractModuleFiles(modulePath_, modulePath, targetSoPath, cpuAbi);
    if (result != ERR_OK) {
        APP_LOGE("extract module files failed, error is %{public}d", result);
        return result;
    }
    return ERR_OK;
}

ErrCode BaseBundleInstaller::RenameModuleDir(const InnerBundleInfo &info) const
{
    auto moduleDir = info.GetAppCodePath() + Constants::PATH_SEPARATOR + info.GetCurrentModulePackage();
    APP_LOGD("rename module to %{public}s", moduleDir.c_str());
    auto result = InstalldClient::GetInstance()->RenameModuleDir(moduleDir + Constants::TMP_SUFFIX, moduleDir);
    if (result != ERR_OK) {
        APP_LOGE("rename module dir failed, error is %{public}d", result);
        return result;
    }
    return ERR_OK;
}

ErrCode BaseBundleInstaller::CheckSysCap(const std::vector<std::string> &bundlePaths)
{
    BYTRACE_NAME(BYTRACE_TAG_APP, __PRETTY_FUNCTION__);
    APP_LOGD("check hap syscaps start.");
    if (bundlePaths.empty()) {
        APP_LOGE("check hap syscaps failed due to empty bundlePaths!");
        return ERR_APPEXECFWK_INSTALL_PARAM_ERROR;
    }

    ErrCode result = ERR_OK;
    BundleParser bundleParser;
    for (auto bundlePath : bundlePaths) {
        std::vector<std::string> bundleSysCaps;
        result = bundleParser.ParseSysCap(bundlePath, bundleSysCaps);
        if (result != ERR_OK) {
            APP_LOGE("parse bundle syscap failed, error: %{public}d", result);
            return result;
        }

        for (auto bundleSysCapItem : bundleSysCaps) {
            APP_LOGD("check syscap(%{public}s)", bundleSysCapItem.c_str());
            if (!HasSystemCapability(bundleSysCapItem.c_str())) {
                APP_LOGE("check syscap failed which %{public}s is not exsit",
                    bundleSysCapItem.c_str());
                return ERR_APPEXECFWK_INSTALL_CHECK_SYSCAP_FAILED;
            }
        }
    }

    APP_LOGD("finish check hap syscaps");
    return result;
}

ErrCode BaseBundleInstaller::CheckMultipleHapsSignInfo(const std::vector<std::string> &bundlePaths,
    const InstallParam &installParam, std::vector<Security::Verify::HapVerifyResult>& hapVerifyRes) const
{
    BYTRACE_NAME(BYTRACE_TAG_APP, __PRETTY_FUNCTION__);
    APP_LOGD("Check multiple haps signInfo");
    if (bundlePaths.empty()) {
        APP_LOGE("check hap sign info failed due to empty bundlePaths!");
        return ERR_APPEXECFWK_INSTALL_PARAM_ERROR;
    }
    if (installParam.noCheckSignature) {
        APP_LOGI("force-install and no signinfo needed!");
        return ERR_OK;
    }
    for (const std::string &bundlePath : bundlePaths) {
        Security::Verify::HapVerifyResult hapVerifyResult;
        auto verifyRes = BundleVerifyMgr::HapVerify(bundlePath, hapVerifyResult);
        if (verifyRes != ERR_OK) {
            APP_LOGE("hap file verify failed");
            return verifyRes;
        }
        hapVerifyRes.emplace_back(hapVerifyResult);
    }
    if (hapVerifyRes.empty()) {
        APP_LOGE("no sign info in the all haps!");
        return ERR_APPEXECFWK_INSTALL_FAILED_INCOMPATIBLE_SIGNATURE;
    }
    auto appId = hapVerifyRes[0].GetProvisionInfo().appId;
    APP_LOGD("bundle appid is %{private}s", appId.c_str());
    auto isValid = std::any_of(hapVerifyRes.begin(), hapVerifyRes.end(), [&](auto &hapVerifyResult) {
        APP_LOGD("module appid is %{private}s", hapVerifyResult.GetProvisionInfo().appId.c_str());
        return appId != hapVerifyResult.GetProvisionInfo().appId;
    });
    if (isValid) {
        APP_LOGE("hap files have different signuature info");
        return ERR_APPEXECFWK_INSTALL_FAILED_INCONSISTENT_SIGNATURE;
    }
    auto apl = hapVerifyRes[0].GetProvisionInfo().bundleInfo.apl;
    APP_LOGD("bundle apl is %{public}s", apl.c_str());
    isValid = std::any_of(hapVerifyRes.begin(), hapVerifyRes.end(), [&](auto &hapVerifyResult) {
        APP_LOGD("module appid is %{private}s", hapVerifyResult.GetProvisionInfo().bundleInfo.apl.c_str());
        return apl != hapVerifyResult.GetProvisionInfo().bundleInfo.apl;
    });
    if (isValid) {
        APP_LOGE("hap files have different apl info");
        return ERR_APPEXECFWK_INSTALL_FAILED_INCONSISTENT_SIGNATURE;
    }
    APP_LOGD("finish check multiple haps signInfo");
    return ERR_OK;
}

ErrCode BaseBundleInstaller::ParseHapFiles(const std::vector<std::string> &bundlePaths,
    const InstallParam &installParam, const Constants::AppType appType,
    std::vector<Security::Verify::HapVerifyResult> &hapVerifyRes,
    std::unordered_map<std::string, InnerBundleInfo> &infos)
{
    BYTRACE_NAME(BYTRACE_TAG_APP, __PRETTY_FUNCTION__);
    APP_LOGD("Parse hap file");
    ErrCode result = ERR_OK;
    for (uint32_t i = 0; i < bundlePaths.size(); ++i) {
        InnerBundleInfo newInfo;
        newInfo.SetAppType(appType);
        Security::Verify::ProvisionInfo provisionInfo;
        if (!installParam.noCheckSignature) {
            provisionInfo = hapVerifyRes[i].GetProvisionInfo();
            bool isSystemApp = (provisionInfo.bundleInfo.appFeature == Constants::HOS_SYSTEM_APP ||
                provisionInfo.bundleInfo.appFeature == Constants::OHOS_SYSTEM_APP);
            if (isSystemApp) {
                newInfo.SetAppType(Constants::AppType::SYSTEM_APP);
            }
        }
        newInfo.SetUserId(installParam.userId);
        newInfo.SetIsKeepData(installParam.isKeepData);
        newInfo.SetIsPreInstallApp(installParam.isPreInstallApp);
        result = ParseBundleInfo(bundlePaths[i], newInfo);
        if (result != ERR_OK) {
            APP_LOGE("bundle parse failed %{public}d", result);
            return result;
        }
        if (newInfo.HasEntry()) {
            if (isContainEntry_) {
                APP_LOGE("more than one entry hap in the direction!");
                return ERR_APPEXECFWK_INSTALL_INVALID_NUMBER_OF_ENTRY_HAP;
            } else {
                isContainEntry_ = true;
            }
        }

        if (!installParam.noCheckSignature) {
            newInfo.SetProvisionId(provisionInfo.appId);
            newInfo.SetAppFeature(provisionInfo.bundleInfo.appFeature);
            newInfo.SetAppPrivilegeLevel(provisionInfo.bundleInfo.apl);
            newInfo.SetAllowedAcls(provisionInfo.acls.allowedAcls);
        }

        if ((result = CheckSystemSize(bundlePaths[i], appType)) != ERR_OK) {
            APP_LOGE("install failed due to insufficient disk memory");
            return result;
        }

        infos.emplace(bundlePaths[i], newInfo);
    }
    APP_LOGD("finish parse hap file");
    return result;
}

ErrCode BaseBundleInstaller::CheckAppLabelInfo(const std::unordered_map<std::string, InnerBundleInfo> &infos)
{
    APP_LOGD("Check APP label");
    ErrCode ret = ERR_OK;
    std::string bundleName = (infos.begin()->second).GetBundleName();
    std::string vendor = (infos.begin()->second).GetVendor();
    versionCode_ = (infos.begin()->second).GetVersionCode();
    std::string versionName = (infos.begin()->second).GetVersionName();
    uint32_t target = (infos.begin()->second).GetTargetVersion();
    uint32_t compatible = (infos.begin()->second).GetCompatibleVersion();
    bool singleton = (infos.begin()->second).IsSingleton();
    Constants::AppType appType = (infos.begin()->second).GetAppType();

    for (const auto &info :infos) {
        // check bundleName
        if (bundleName != info.second.GetBundleName()) {
            return ERR_APPEXECFWK_INSTALL_BUNDLENAME_NOT_SAME;
        }
        // check version
        if (versionCode_ != info.second.GetVersionCode()) {
            return ERR_APPEXECFWK_INSTALL_VERSIONCODE_NOT_SAME;
        }
        if (versionName != info.second.GetVersionName()) {
            return ERR_APPEXECFWK_INSTALL_VERSIONNAME_NOT_SAME;
        }
        // check vendor
        if (vendor != info.second.GetVendor()) {
            return ERR_APPEXECFWK_INSTALL_VENDOR_NOT_SAME;
        }
        // check release type
        if (target != info.second.GetTargetVersion()) {
            return ERR_APPEXECFWK_INSTALL_RELEASETYPE_TARGET_NOT_SAME;
        }
        if (compatible != info.second.GetCompatibleVersion()) {
            return ERR_APPEXECFWK_INSTALL_RELEASETYPE_COMPATIBLE_NOT_SAME;
        }
        if (singleton != info.second.IsSingleton()) {
            return ERR_APPEXECFWK_INSTALL_SINGLETON_NOT_SAME;
        }
        if (appType != info.second.GetAppType()) {
            return ERR_APPEXECFWK_INSTALL_APPTYPE_NOT_SAME;
        }
    }
    APP_LOGD("finish check APP label");
    return ret;
}

bool BaseBundleInstaller::GetInnerBundleInfo(InnerBundleInfo &info, bool &isAppExist)
{
    if (!dataMgr_) {
        dataMgr_ = DelayedSingleton<BundleMgrService>::GetInstance()->GetDataMgr();
        if (!dataMgr_) {
            APP_LOGE("Get dataMgr shared_ptr nullptr");
            return false;
        }
    }
    auto &mtx = dataMgr_->GetBundleMutex(bundleName_);
    std::lock_guard lock { mtx };
    isAppExist = dataMgr_->GetInnerBundleInfo(bundleName_, Constants::CURRENT_DEVICE_ID, info);
    return true;
}

// In the process of hap updating, the version code of the entry hap which is about to be updated must not less the
// version code of the current entry haps in the device; if no-entry hap in the device, the updating haps should
// have same version code with the current version code; if the no-entry haps is to be updated, which should has the
// same version code with that of the entry hap in the device.
ErrCode BaseBundleInstaller::CheckVersionCompatibility(const InnerBundleInfo &oldInfo)
{
    APP_LOGD("start to check version compatibility");
    if (oldInfo.HasEntry()) {
        if (isContainEntry_ && versionCode_ < oldInfo.GetVersionCode()) {
            APP_LOGE("fail to update lower version bundle");
            return ERR_APPEXECFWK_INSTALL_VERSION_DOWNGRADE;
        }
        if (!isContainEntry_ && versionCode_ > oldInfo.GetVersionCode()) {
            APP_LOGE("version code is not compatible");
            return ERR_APPEXECFWK_INSTALL_VERSION_NOT_COMPATIBLE;
        }
        if (!isContainEntry_ && versionCode_ < oldInfo.GetVersionCode()) {
            APP_LOGE("version code is not compatible");
            return ERR_APPEXECFWK_INSTALL_VERSION_DOWNGRADE;
        }
    } else {
        if (versionCode_ < oldInfo.GetVersionCode()) {
            APP_LOGE("fail to update lower version bundle");
            return ERR_APPEXECFWK_INSTALL_VERSION_DOWNGRADE;
        }
    }

    if (versionCode_ > oldInfo.GetVersionCode()) {
        APP_LOGD("need to uninstall lower version feature hap");
        isFeatureNeedUninstall_ = true;
    }
    APP_LOGD("finish to check version compatibility");
    return ERR_OK;
}

ErrCode BaseBundleInstaller::UninstallLowerVersionFeature(const std::vector<std::string> &packageVec)
{
    APP_LOGD("start to uninstall lower version feature hap");
    InnerBundleInfo info;
    bool isExist = false;
    if (!GetInnerBundleInfo(info, isExist) || !isExist) {
        return ERR_APPEXECFWK_UNINSTALL_BUNDLE_MGR_SERVICE_ERROR;
    }

    if (!dataMgr_->UpdateBundleInstallState(bundleName_, InstallState::UNINSTALL_START)) {
        APP_LOGE("uninstall already start");
        return ERR_APPEXECFWK_INSTALL_BUNDLE_MGR_SERVICE_ERROR;
    }

    // kill the bundle process during uninstall.
    if (!UninstallApplicationProcesses(info.GetApplicationName(), info.GetUid(userId_))) {
        APP_LOGW("can not kill process");
    }
    std::vector<std::string> moduleVec = info.GetModuleNameVec();
    InnerBundleInfo oldInfo = info;
    for (const auto &package : moduleVec) {
        if (find(packageVec.begin(), packageVec.end(), package) == packageVec.end()) {
            APP_LOGD("uninstall package %{public}s", package.c_str());
            ErrCode result = RemoveModuleAndDataDir(info, package);
            if (result != ERR_OK) {
                APP_LOGE("remove module dir failed");
                return result;
            }
            if (!dataMgr_->RemoveModuleInfo(bundleName_, package, info)) {
                APP_LOGE("RemoveModuleInfo failed");
                return ERR_APPEXECFWK_INSTALL_BUNDLE_MGR_SERVICE_ERROR;
            }
        }
    }
    // need to delete lower version feature hap definePermissions and requestPermissions
    APP_LOGD("delete lower version feature hap definePermissions and requestPermissions");
    ErrCode ret = UpdateDefineAndRequestPermissions(oldInfo, info);
    if (ret != ERR_OK) {
        return ret;
    }
    APP_LOGD("finish to uninstall lower version feature hap");
    return ERR_OK;
}

ErrCode BaseBundleInstaller::CheckSystemSize(const std::string &bundlePath, const Constants::AppType appType) const
{
    if ((appType == Constants::AppType::SYSTEM_APP) &&
        (BundleUtil::CheckSystemSize(bundlePath, Constants::SYSTEM_APP_INSTALL_PATH))) {
        return ERR_OK;
    }
    if ((appType == Constants::AppType::THIRD_SYSTEM_APP) &&
        (BundleUtil::CheckSystemSize(bundlePath, Constants::THIRD_SYSTEM_APP_INSTALL_PATH))) {
        return ERR_OK;
    }
    if ((appType == Constants::AppType::THIRD_PARTY_APP) &&
        (BundleUtil::CheckSystemSize(bundlePath, Constants::THIRD_PARTY_APP_INSTALL_PATH))) {
        return ERR_OK;
    }
    APP_LOGE("install failed due to insufficient disk memory");
    return ERR_APPEXECFWK_INSTALL_DISK_MEM_INSUFFICIENT;
}

int32_t BaseBundleInstaller::GetUserId(const InstallParam& installParam) const
{
    int32_t userId = installParam.userId;
    if (userId < Constants::DEFAULT_USERID) {
        APP_LOGE("userId(%{public}d) is invalid.", userId);
        return Constants::INVALID_USERID;
    }

    APP_LOGD("BundleInstaller GetUserId, now userId is %{public}d", userId);
    return userId;
}

ErrCode BaseBundleInstaller::CreateBundleUserData(
    InnerBundleInfo &innerBundleInfo, bool needResetInstallState)
{
    APP_LOGD("CreateNewUserData %{public}s userId: %{public}d.",
        innerBundleInfo.GetBundleName().c_str(), userId_);
    if (!innerBundleInfo.HasInnerBundleUserInfo(userId_)) {
        return ERR_APPEXECFWK_USER_NOT_EXIST;
    }

    ErrCode result = CreateBundleDataDir(innerBundleInfo);
    if (result != ERR_OK) {
        return result;
    }

    innerBundleInfo.SetBundleInstallTime(BundleUtil::GetCurrentTime(), userId_);
    if (!dataMgr_->UpdateBundleInstallState(innerBundleInfo.GetBundleName(), InstallState::USER_CHANGE)) {
        APP_LOGE("update bundleinfo when user change failed.");
        return ERR_APPEXECFWK_INSTALL_BUNDLE_MGR_SERVICE_ERROR;
    }

    ScopeGuard stateGuard([&] {
        dataMgr_->UpdateBundleInstallState(innerBundleInfo.GetBundleName(), InstallState::INSTALL_SUCCESS);
    });
    InnerBundleUserInfo innerBundleUserInfo;
    if (!innerBundleInfo.GetInnerBundleUserInfo(userId_, innerBundleUserInfo)) {
        APP_LOGE("oldInfo do not have user");
        return ERR_APPEXECFWK_USER_NOT_EXIST;
    }

    if (!dataMgr_->AddInnerBundleUserInfo(innerBundleInfo.GetBundleName(), innerBundleUserInfo)) {
        APP_LOGE("update bundle user info to db failed %{public}s when createNewUser",
            innerBundleInfo.GetBundleName().c_str());
        return ERR_APPEXECFWK_INSTALL_BUNDLE_MGR_SERVICE_ERROR;
    }

    if (!needResetInstallState) {
        stateGuard.Dismiss();
    }

    return ERR_OK;
}

ErrCode BaseBundleInstaller::UpdateUserInfoToDb(
    InnerBundleInfo &innerBundleInfo, bool needResetInstallState)
{
    APP_LOGD("update user(%{public}d) in bundle(%{public}s) to Db start.",
        userId_, innerBundleInfo.GetBundleName().c_str());
    if (!dataMgr_->UpdateBundleInstallState(innerBundleInfo.GetBundleName(), InstallState::USER_CHANGE)) {
        APP_LOGE("update bundleinfo when user change failed.");
        return ERR_APPEXECFWK_INSTALL_BUNDLE_MGR_SERVICE_ERROR;
    }

    ScopeGuard stateGuard([&] {
        dataMgr_->UpdateBundleInstallState(innerBundleInfo.GetBundleName(), InstallState::INSTALL_SUCCESS);
    });
    auto newBundleInfo = innerBundleInfo;
    if (!dataMgr_->UpdateInnerBundleInfo(innerBundleInfo.GetBundleName(), newBundleInfo, innerBundleInfo)) {
        APP_LOGE("update bundle user info to db failed %{public}s when createNewUser",
            innerBundleInfo.GetBundleName().c_str());
        return ERR_APPEXECFWK_INSTALL_BUNDLE_MGR_SERVICE_ERROR;
    }

    if (!needResetInstallState) {
        stateGuard.Dismiss();
    }

    APP_LOGD("update user(%{public}d) in bundle(%{public}s) to Db end.",
        userId_, innerBundleInfo.GetBundleName().c_str());
    return ERR_OK;
}

ErrCode BaseBundleInstaller::RemoveBundleUserData(InnerBundleInfo &innerBundleInfo, bool needRemoveData)
{
    auto bundleName = innerBundleInfo.GetBundleName();
    APP_LOGD("remove user(%{public}d) in bundle(%{public}s).", userId_, bundleName.c_str());
    if (!innerBundleInfo.HasInnerBundleUserInfo(userId_)) {
        return ERR_APPEXECFWK_USER_NOT_EXIST;
    }

    if (needRemoveData) {
        ErrCode result = RemoveBundleDataDir(innerBundleInfo);
        if (result != ERR_OK) {
            APP_LOGE("remove user data directory failed.");
            return result;
        }
    }

    // delete accessTokenId
    if (BundlePermissionMgr::DeleteAccessTokenId(innerBundleInfo.GetAccessTokenId(userId_)) !=
        AccessToken::AccessTokenKitRet::RET_SUCCESS) {
        APP_LOGE("delete accessToken failed");
    }

    innerBundleInfo.RemoveInnerBundleUserInfo(userId_);
    if (!dataMgr_->UpdateBundleInstallState(bundleName, InstallState::USER_CHANGE)) {
        APP_LOGE("update bundleinfo when user change failed.");
        return ERR_APPEXECFWK_INSTALL_BUNDLE_MGR_SERVICE_ERROR;
    }

    if (!dataMgr_->RemoveInnerBundleUserInfo(bundleName, userId_)) {
        APP_LOGE("update bundle user info to db failed %{public}s when remove user",
            bundleName.c_str());
        return ERR_APPEXECFWK_INSTALL_BUNDLE_MGR_SERVICE_ERROR;
    }

    dataMgr_->UpdateBundleInstallState(bundleName, InstallState::INSTALL_SUCCESS);
    return ERR_OK;
}

bool BaseBundleInstaller::verifyUriPrefix(const InnerBundleInfo &info, int32_t userId, bool isUpdate) const
{
    // uriPrefix must be unique
    // verify current module uriPrefix
    std::vector<std::string> currentUriPrefixList;
    info.GetUriPrefixList(currentUriPrefixList);
    if (currentUriPrefixList.empty()) {
        APP_LOGD("current module not include uri, verify uriPrefix success");
        return true;
    }
    std::set<std::string> set;
    for (const std::string &currentUriPrefix : currentUriPrefixList) {
        if (!set.insert(currentUriPrefix).second) {
            APP_LOGE("current module contains duplicate uriPrefix, verify uriPrefix failed");
            APP_LOGE("bundleName : %{public}s, moduleName : %{public}s, uriPrefix : %{public}s",
                info.GetBundleName().c_str(), info.GetCurrentModulePackage().c_str(), currentUriPrefix.c_str());
            return false;
        }
    }
    set.clear();
    // verify exist bundle uriPrefix
    if (dataMgr_ == nullptr) {
        APP_LOGE("dataMgr_ is null, verify uriPrefix failed");
        return false;
    }
    std::vector<std::string> uriPrefixList;
    std::string excludeModule;
    if (isUpdate) {
        excludeModule.append(info.GetBundleName()).append(".").append(info.GetCurrentModulePackage()).append(".");
    }
    dataMgr_->GetAllUriPrefix(uriPrefixList, userId, excludeModule);
    if (uriPrefixList.empty()) {
        APP_LOGD("uriPrefixList empty, verify uriPrefix success");
        return true;
    }
    for (const std::string &currentUriPrefix : currentUriPrefixList) {
        auto iter = std::find(uriPrefixList.cbegin(), uriPrefixList.cend(), currentUriPrefix);
        if (iter != uriPrefixList.cend()) {
            APP_LOGE("uriPrefix alread exist in device, uriPrefix : %{public}s", currentUriPrefix.c_str());
            APP_LOGE("verify uriPrefix failed");
            return false;
        }
    }
    APP_LOGD("verify uriPrefix success");
    return true;
}

void BaseBundleInstaller::ResetInstallProperties()
{
    isContainEntry_ = false;
    isAppExist_ = false;
    hasInstalledInUser_ = false;
    isFeatureNeedUninstall_ = false;
    versionCode_ = 0;
    uninstallModuleVec_.clear();
    installedModules_.clear();
    state_ = InstallerState::INSTALL_START;
}

void BaseBundleInstaller::SaveStorageDistributeInfo()
{
    int32_t currentUserId = AccountHelper::GetCurrentActiveUserId();
    if (currentUserId == Constants::INVALID_USERID) {
        currentUserId = Constants::START_USERID;
    }
    if (userId_ != currentUserId) {
        APP_LOGW("install userid:%{public}d is not currentUserId:%{public}d", userId_, currentUserId);
        return;
    }
    BundleInfo bundleInfo;
    bool ret = dataMgr_->GetBundleInfo(
        bundleName_, BundleFlag::GET_BUNDLE_WITH_ABILITIES, bundleInfo, currentUserId);
    if (ret) {
        DistributedDataStorage::GetInstance()->SaveStorageDistributeInfo(bundleInfo);
    }
}

void BaseBundleInstaller::DeleteStorageDistributeInfo(const std::string &bundleName)
{
    int32_t currentUserId = AccountHelper::GetCurrentActiveUserId();
    if (userId_ != currentUserId) {
        APP_LOGW("uninstall userid:%{public}d is not currentUserId:%{public}d", userId_, currentUserId);
        return;
    }
    bool ret = DistributedDataStorage::GetInstance()->DeleteStorageDistributeInfo(bundleName);
    if (!ret) {
        APP_LOGW("bundleName:%{public}s, deleteStorageDistributeInfo failed", bundleName.c_str());
    }
}
}  // namespace AppExecFwk
}  // namespace OHOS
