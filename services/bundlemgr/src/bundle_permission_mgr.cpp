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

#include "bundle_permission_mgr.h"

#include "app_log_wrapper.h"
#include "bundle_mgr_service.h"
#include "ipc_skeleton.h"

namespace OHOS {
namespace AppExecFwk {
using namespace OHOS::Security;
// convert the Permission::PermissionDef struct to
// AppExecFwk::PermissionDef struct that can be used in IPC process
void BundlePermissionMgr::ConvertPermissionDef(const Permission::PermissionDef &permDef, PermissionDef &permissionDef)
{
    permissionDef.permissionName = permDef.permissionName;
    permissionDef.bundleName = permDef.bundleName;
    permissionDef.grantMode = permDef.grantMode;
    permissionDef.availableScope = permDef.availableScope;
    permissionDef.label = permDef.label;
    permissionDef.labelId = permDef.labelId;
    permissionDef.description = permDef.description;
    permissionDef.descriptionId = permDef.descriptionId;
}

// Convert from the struct DefPermission that parsed from config.json
void BundlePermissionMgr::ConvertPermissionDef(
    Permission::PermissionDef &permDef, const DefPermission &defPermission, const std::string &bundleName)
{
    permDef.permissionName = defPermission.name;
    permDef.bundleName = bundleName;
    permDef.grantMode = [&defPermission]() -> int {
        if (defPermission.grantMode ==
            ProfileReader::BUNDLE_MODULE_PROFILE_KEY_DEF_PERMISSIONS_GRANTMODE_SYSTEM_GRANT) {
            return Permission::GrantMode::SYSTEM_GRANT;
        }
        return Permission::GrantMode::USER_GRANT;
    }();
    permDef.availableScope = [&defPermission]() -> int {
        unsigned flag = 0;
        if (std::find(defPermission.availableScope.begin(),
                      defPermission.availableScope.end(),
                      ProfileReader::BUNDLE_MODULE_PROFILE_KEY_DEF_PERMISSIONS_AVAILABLESCOPE_SIGNATURE) !=
                      defPermission.availableScope.end()) {
            flag |= Permission::AvailableScope::AVAILABLE_SCOPE_SIGNATURE;
        }
        if (std::find(defPermission.availableScope.begin(),
                      defPermission.availableScope.end(),
                      ProfileReader::BUNDLE_MODULE_PROFILE_KEY_DEF_PERMISSIONS_AVAILABLESCOPE_RESTRICTED) !=
                      defPermission.availableScope.end()) {
            flag |= Permission::AvailableScope::AVAILABLE_SCOPE_RESTRICTED;
        }
        if (flag == 0) {
            return Permission::AvailableScope::AVAILABLE_SCOPE_ALL;
        }
        return flag;
    }();
    permDef.label = defPermission.label;
    permDef.labelId = defPermission.labelId;
    permDef.description = defPermission.description;
    permDef.descriptionId = defPermission.descriptionId;
}

void BundlePermissionMgr::ConvertPermissionDef(
    const AccessToken::PermissionDef &permDef, PermissionDefine &permissionDef)
{
    permissionDef.permissionName = permDef.permissionName;
    permissionDef.bundleName = permDef.bundleName;
    permissionDef.grantMode = permDef.grantMode;
    permissionDef.availableLevel = permDef.availableLevel;
    permissionDef.provisionEnable = permDef.provisionEnable;
    permissionDef.distributedSceneEnable = permDef.distributedSceneEnable;
    permissionDef.label = permDef.label;
    permissionDef.labelId = permDef.labelId;
    permissionDef.description = permDef.description;
    permissionDef.descriptionId = permDef.descriptionId;
}

// Convert from the struct DefinePermission that parsed from config.json
void BundlePermissionMgr::ConvertPermissionDef(
    AccessToken::PermissionDef &permDef, const DefinePermission &definePermission, const std::string &bundleName)
{
    permDef.permissionName = definePermission.name;
    permDef.bundleName = bundleName;
    permDef.grantMode = [&definePermission]() -> int {
        if (definePermission.grantMode ==
            ProfileReader::BUNDLE_MODULE_PROFILE_KEY_DEF_PERMISSIONS_GRANTMODE_SYSTEM_GRANT) {
            return AccessToken::GrantMode::SYSTEM_GRANT;
        }
        return AccessToken::GrantMode::USER_GRANT;
    }();

    permDef.availableLevel = GetTokenApl(definePermission.availableLevel);
    permDef.provisionEnable = definePermission.provisionEnable;
    permDef.distributedSceneEnable = definePermission.distributedSceneEnable;
    permDef.label = definePermission.label;
    permDef.labelId = definePermission.labelId;
    permDef.description = definePermission.description;
    permDef.descriptionId = definePermission.descriptionId;
}

AccessToken::ATokenAplEnum BundlePermissionMgr::GetTokenApl(const std::string &apl)
{
    if (apl == Profile::AVAILABLELEVEL_SYSTEM_CORE) {
        return AccessToken::ATokenAplEnum::APL_SYSTEM_CORE;
    }
    if (apl == Profile::AVAILABLELEVEL_SYSTEM_BASIC) {
        return AccessToken::ATokenAplEnum::APL_SYSTEM_BASIC;
    }
    return AccessToken::ATokenAplEnum::APL_NORMAL;
}

Security::AccessToken::HapPolicyParams BundlePermissionMgr::CreateHapPolicyParam(
    const InnerBundleInfo &innerBundleInfo)
{
    AccessToken::HapPolicyParams hapPolicy;
    std::string apl = innerBundleInfo.GetAppPrivilegeLevel();
    APP_LOGD("BundlePermissionMgr::CreateHapPolicyParam apl : %{public}s", apl.c_str());
    std::vector<AccessToken::PermissionDef> permDef = GetPermissionDefList(innerBundleInfo);
    std::vector<AccessToken::PermissionStateFull> permStateFull = GetPermissionStateFullList(innerBundleInfo);
    hapPolicy.apl = GetTokenApl(apl);
    hapPolicy.domain = "domain";
    hapPolicy.permList = permDef;
    hapPolicy.permStateList = permStateFull;
    return hapPolicy;
}

AccessToken::AccessTokenID BundlePermissionMgr::CreateAccessTokenId(
    const InnerBundleInfo &innerBundleInfo, const std::string bundleName, const int32_t userId)
{
    APP_LOGD("BundlePermissionMgr::CreateAccessTokenId bundleName = %{public}s, userId = %{public}d",
        bundleName.c_str(), userId);
    AccessToken::HapInfoParams hapInfo;
    hapInfo.userID = userId;
    hapInfo.bundleName = bundleName;
    hapInfo.instIndex = 0;
    hapInfo.appIDDesc = innerBundleInfo.GetProvisionId();
    AccessToken::HapPolicyParams hapPolicy = CreateHapPolicyParam(innerBundleInfo);
    AccessToken::AccessTokenIDEx accessToken = AccessToken::AccessTokenKit::AllocHapToken(hapInfo, hapPolicy);
    APP_LOGD("BundlePermissionMgr::CreateAccessTokenId accessTokenId = %{public}u",
             accessToken.tokenIdExStruct.tokenID);
    return accessToken.tokenIdExStruct.tokenID;
}

int32_t BundlePermissionMgr::UpdateHapToken(const Security::AccessToken::AccessTokenID tokenId,
    const InnerBundleInfo &innerBundleInfo)
{
    APP_LOGD("BundlePermissionMgr::UpdateHapToken bundleName = %{public}s", innerBundleInfo.GetBundleName().c_str());
    std::string appId = innerBundleInfo.GetProvisionId();
    AccessToken::HapPolicyParams hapPolicy = CreateHapPolicyParam(innerBundleInfo);
    return AccessToken::AccessTokenKit::UpdateHapToken(tokenId, appId, hapPolicy);
}

bool BundlePermissionMgr::GetNewPermissionDefList(Security::AccessToken::AccessTokenID tokenId,
    const std::vector<Security::AccessToken::PermissionDef> &permissionDef,
    std::vector<Security::AccessToken::PermissionDef> &newPermissionDef)
{
    int32_t ret = AccessToken::AccessTokenKit::GetDefPermissions(tokenId, newPermissionDef);
    if (ret != AccessToken::AccessTokenKitRet::RET_SUCCESS) {
        APP_LOGE("BundlePermissionMgr::GetNewPermissionDefList GetDefPermissions failed errcode: %{public}d", ret);
        return false;
    }
    for (const auto &perm : permissionDef) {
        if (std::find_if(newPermissionDef.begin(), newPermissionDef.end(), [&perm](const auto &newPerm) {
            return newPerm.permissionName == perm.permissionName;
            }) == newPermissionDef.end()) {
            newPermissionDef.emplace_back(perm);
        }
    }
    return true;
}

bool BundlePermissionMgr::GetNewPermissionStateFull(Security::AccessToken::AccessTokenID tokenId,
    const std::vector<Security::AccessToken::PermissionStateFull> &permissionState,
    std::vector<Security::AccessToken::PermissionStateFull> &newPermissionState,
    std::vector<std::string> &newRequestPermName)
{
    if (!GetAllReqPermissionStateFull(tokenId, newPermissionState)) {
        APP_LOGE("BundlePermissionMgr::GetNewPermissionStateFull failed");
        return false;
    }
    for (const auto &perm : permissionState) {
        if (std::find_if(newPermissionState.begin(), newPermissionState.end(), [&perm](const auto &newPerm) {
            return newPerm.permissionName == perm.permissionName;
            }) == newPermissionState.end()) {
            APP_LOGD("BundlePermissionMgr::GetNewPermissionStateFull add request permission %{public}s",
                     perm.permissionName.c_str());
            newPermissionState.emplace_back(perm);
            newRequestPermName.emplace_back(perm.permissionName);
        }
    }
    return true;
}

bool BundlePermissionMgr::AddDefinePermissions(const Security::AccessToken::AccessTokenID tokenId,
    const InnerBundleInfo &innerBundleInfo, std::vector<std::string> &newRequestPermName)
{
    APP_LOGD("BundlePermissionMgr::AddDefinePermissions start");
    std::vector<AccessToken::PermissionDef> defPermList = GetPermissionDefList(innerBundleInfo);
    std::vector<AccessToken::PermissionDef> newDefPermList;
    if (!GetNewPermissionDefList(tokenId, defPermList, newDefPermList)) {
        return false;
    }

    std::vector<AccessToken::PermissionStateFull> reqPermissionStateList = GetPermissionStateFullList(innerBundleInfo);
    std::vector<AccessToken::PermissionStateFull> newPermissionStateList;
    if (!GetNewPermissionStateFull(tokenId, reqPermissionStateList, newPermissionStateList, newRequestPermName)) {
        return false;
    }

    AccessToken::HapPolicyParams hapPolicy;
    std::string apl = innerBundleInfo.GetAppPrivilegeLevel();
    APP_LOGD("BundlePermissionMgr::AddDefinePermissions apl : %{public}s, newDefPermList size : %{public}zu, \
             newPermissionStateList size : %{public}zu", apl.c_str(), newDefPermList.size(),
             newPermissionStateList.size());
    hapPolicy.apl = GetTokenApl(apl);
    hapPolicy.domain = "domain"; // default
    hapPolicy.permList = newDefPermList;
    hapPolicy.permStateList = newPermissionStateList;
    std::string appId = innerBundleInfo.GetProvisionId();
    int32_t ret = AccessToken::AccessTokenKit::UpdateHapToken(tokenId, appId, hapPolicy);
    if (ret != AccessToken::AccessTokenKitRet::RET_SUCCESS) {
        APP_LOGE("BundlePermissionMgr::AddDefinePermissions UpdateHapToken failed errcode: %{public}d", ret);
        return false;
    }
    APP_LOGD("BundlePermissionMgr::AddDefinePermissions end");
    return true;
}

int32_t BundlePermissionMgr::DeleteAccessTokenId(const AccessToken::AccessTokenID tokenId)
{
    APP_LOGD("BundlePermissionMgr::DeleteAccessTokenId tokenId : %{public}u", tokenId);
    return AccessToken::AccessTokenKit::DeleteToken(tokenId);
}

int32_t BundlePermissionMgr::ClearUserGrantedPermissionState(const AccessToken::AccessTokenID tokenId)
{
    return AccessToken::AccessTokenKit::ClearUserGrantedPermissionState(tokenId);
}

std::vector<AccessToken::PermissionDef> BundlePermissionMgr::GetPermissionDefList(
    const InnerBundleInfo &innerBundleInfo)
{
    const auto bundleName = innerBundleInfo.GetBundleName();
    const auto defPermissions = innerBundleInfo.GetDefinePermissions();
    std::vector<AccessToken::PermissionDef> permList;
    if (!defPermissions.empty()) {
        for (const auto &defPermission : defPermissions) {
            AccessToken::PermissionDef perm;
            APP_LOGI("defPermission %{public}s", defPermission.name.c_str());
            ConvertPermissionDef(perm, defPermission, bundleName);
            permList.emplace_back(perm);
        }
    }
    return permList;
}

std::vector<AccessToken::PermissionStateFull> BundlePermissionMgr::GetPermissionStateFullList(
    const InnerBundleInfo &innerBundleInfo)
{
    auto reqPermissions = innerBundleInfo.GetRequestPermissions();
    std::vector<std::string> grantPermList;
    std::vector<AccessToken::PermissionStateFull> permStateFullList;
    if (!reqPermissions.empty()) {
        for (const auto &reqPermission : reqPermissions) {
            AccessToken::PermissionStateFull perState;
            perState.permissionName = reqPermission.name;
            perState.isGeneral = true;
            perState.resDeviceID.emplace_back(innerBundleInfo.GetBaseApplicationInfo().deviceId);
            perState.grantStatus.emplace_back(AccessToken::PermissionState::PERMISSION_DENIED);
            perState.grantFlags.emplace_back(AccessToken::PermissionFlag::PERMISSION_USER_SET);
            permStateFullList.emplace_back(perState);
        }
    }
    return permStateFullList;
}

bool BundlePermissionMgr::InnerGrantRequestPermissions(const std::vector<RequestPermission> &reqPermissions,
    const std::string &apl, const std::vector<std::string> &acls,
    const Security::AccessToken::AccessTokenID tokenId)
{
    std::vector<std::string> grantPermList;
    for (const auto &reqPermission : reqPermissions) {
        APP_LOGI("add permission %{public}s", reqPermission.name.c_str());
        AccessToken::PermissionDef permDef;
        int32_t ret = AccessToken::AccessTokenKit::GetDefPermission(reqPermission.name, permDef);
        if (ret != AccessToken::AccessTokenKitRet::RET_SUCCESS) {
            APP_LOGE("get permission def failed, request permission name: %{public}s", reqPermission.name.c_str());
            continue;
        }
        // need to check apl
        if (permDef.grantMode == AccessToken::GrantMode::SYSTEM_GRANT) {
            APP_LOGD("InnerGrantRequestPermissions system grant permission %{public}s", reqPermission.name.c_str());
            grantPermList.emplace_back(reqPermission.name);
        }
    }

    APP_LOGD("InnerGrantRequestPermissions add system grant permission %{public}zu", grantPermList.size());
    for (const auto &perm : grantPermList) {
        auto ret = AccessToken::AccessTokenKit::GrantPermission(tokenId, perm, 0);
        if (ret != AccessToken::AccessTokenKitRet::RET_SUCCESS) {
            APP_LOGE("GrantReqPermission failed, request permission name:%{public}s", perm.c_str());
            return false;
        }
    }
    return true;
}

bool BundlePermissionMgr::GrantRequestPermissions(const InnerBundleInfo &innerBundleInfo,
    const AccessToken::AccessTokenID tokenId)
{
    std::vector<RequestPermission> reqPermissions = innerBundleInfo.GetRequestPermissions();
    std::string apl = innerBundleInfo.GetAppPrivilegeLevel();
    std::vector<std::string> acls = innerBundleInfo.GetAllowedAcls();
    return InnerGrantRequestPermissions(reqPermissions, apl, acls, tokenId);
}

bool BundlePermissionMgr::GrantRequestPermissions(const InnerBundleInfo &innerBundleInfo,
    const std::vector<std::string> &requestPermName,
    const AccessToken::AccessTokenID tokenId)
{
    std::vector<RequestPermission> reqPermissions = innerBundleInfo.GetRequestPermissions();
    std::string apl = innerBundleInfo.GetAppPrivilegeLevel();
    std::vector<std::string> acls = innerBundleInfo.GetAllowedAcls();
    std::vector<RequestPermission> newRequestPermissions;
    for (const auto &name : requestPermName) {
        auto iter = find_if(reqPermissions.begin(), reqPermissions.end(), [&name](const auto &req) {
            return name == req.name;
        });
        if (iter != reqPermissions.end()) {
            newRequestPermissions.emplace_back(*iter);
        }
    }
    return InnerGrantRequestPermissions(newRequestPermissions, apl, acls, tokenId);
}

bool BundlePermissionMgr::GetAllReqPermissionStateFull(AccessToken::AccessTokenID tokenId,
    std::vector<AccessToken::PermissionStateFull> &newPermissionState)
{
    std::vector<AccessToken::PermissionStateFull> userGrantReqPermList;
    int32_t ret = AccessToken::AccessTokenKit::GetReqPermissions(tokenId, userGrantReqPermList, false);
    if (ret != AccessToken::AccessTokenKitRet::RET_SUCCESS) {
        APP_LOGE("GetAllReqPermissionStateFull get user grant failed errcode: %{public}d", ret);
        return false;
    }
    std::vector<AccessToken::PermissionStateFull> systemGrantReqPermList;
    ret = AccessToken::AccessTokenKit::GetReqPermissions(tokenId, systemGrantReqPermList, true);
    if (ret != AccessToken::AccessTokenKitRet::RET_SUCCESS) {
        APP_LOGE("GetAllReqPermissionStateFull get system grant failed errcode: %{public}d", ret);
        return false;
    }
    newPermissionState = userGrantReqPermList;
    for (auto &perm : systemGrantReqPermList) {
        newPermissionState.emplace_back(perm);
    }
    return true;
}

bool BundlePermissionMgr::GetRequestPermissionStates(BundleInfo &bundleInfo)
{
    std::vector<std::string> requestPermission = bundleInfo.reqPermissions;
    if (requestPermission.empty()) {
        APP_LOGD("GetRequestPermissionStates requestPermission empty");
        return true;
    }
    uint32_t tokenId = bundleInfo.applicationInfo.accessTokenId;
    std::vector<Security::AccessToken::PermissionStateFull> allPermissionState;
    if (!GetAllReqPermissionStateFull(tokenId, allPermissionState)) {
        APP_LOGE("BundlePermissionMgr::GetRequestPermissionStates failed");
        return false;
    }
    std::string deviceId = bundleInfo.applicationInfo.deviceId;
    for (auto &req : requestPermission) {
        auto iter = std::find_if(allPermissionState.begin(), allPermissionState.end(),
            [&req](const auto &perm) {
                return perm.permissionName == req;
            });
        if (iter != allPermissionState.end()) {
            APP_LOGD("GetRequestPermissionStates request permission name : %{public}s, deviceId : %{public}s",
                     req.c_str(), deviceId.c_str());
            for (std::vector<std::string>::size_type i = 0; i < iter->resDeviceID.size(); i++) {
                if (iter->resDeviceID[i] == deviceId) {
                    bundleInfo.reqPermissionStates.emplace_back(iter->grantStatus[i]);
                    break;
                }
            }
        } else {
            APP_LOGE("request permission name : %{public}s is not exit", req.c_str());
            bundleInfo.reqPermissionStates.emplace_back(Constants::PERMISSION_INVALID_GRANTED);
        }
    }
    return true;
}

bool BundlePermissionMgr::CheckGrantPermission(
    const AccessToken::PermissionDef &permDef,
    const std::string &apl,
    const std::vector<std::string> &acls)
{
    AccessToken::ATokenAplEnum availableLevel = permDef.availableLevel;
    APP_LOGD("BundlePermissionMgr::CheckGrantPermission availableLevel %{public}d, apl %{public}s",
             availableLevel, apl.c_str());
    switch (availableLevel) {
        case AccessToken::ATokenAplEnum::APL_NORMAL: {
            return true;
        }
        case AccessToken::ATokenAplEnum::APL_SYSTEM_BASIC: {
            if ((apl == Profile::AVAILABLELEVEL_SYSTEM_BASIC) ||
                (apl == Profile::AVAILABLELEVEL_SYSTEM_CORE)) {
                return true;
            }
            break;
        }
        case AccessToken::ATokenAplEnum::APL_SYSTEM_CORE: {
            if (apl == Profile::AVAILABLELEVEL_SYSTEM_CORE) {
                return true;
            }
            break;
        }
        default:
            APP_LOGE("availableLevel %{public}d error", availableLevel);
            break;
    }
    if (permDef.provisionEnable) {
        for (auto &perm : acls) {
            if (permDef.permissionName == perm) {
                return true;
            }
        }
    }
    APP_LOGE("BundlePermissionMgr::CheckGrantPermission failed permission name : %{public}s",
             permDef.permissionName.c_str());
    return false;
}

int32_t BundlePermissionMgr::VerifyPermission(AccessToken::AccessTokenID tokenId, const std::string &permissionName)
{
    APP_LOGD("VerifyPermission permission %{public}s", permissionName.c_str());
    return AccessToken::AccessTokenKit::VerifyAccessToken(tokenId, permissionName);
}

bool BundlePermissionMgr::GetDefinePermission(
    const std::string& permissionName, PermissionDefine& permissionDefResult)
{
    APP_LOGD("GetPermissionDef permission %{public}s", permissionName.c_str());
    AccessToken::PermissionDef permDef;
    int32_t ret = AccessToken::AccessTokenKit::GetDefPermission(permissionName, permDef);
    if (ret != AccessToken::AccessTokenKitRet::RET_SUCCESS) {
        APP_LOGE("get permission def failed");
        return false;
    }
    ConvertPermissionDef(permDef, permissionDefResult);
    return true;
}

bool BundlePermissionMgr::InitPermissions()
{
    // need load all system defined permissions here on first start up.
    return true;
}

int32_t BundlePermissionMgr::AddDefPermissions(const InnerBundleInfo &innerBundleInfo)
{
    const auto bundleName = innerBundleInfo.GetBundleName();
    const auto defPermissions = innerBundleInfo.GetDefPermissions();
    int32_t ret = Permission::RET_FAILED;
    if (!defPermissions.empty()) {
        std::vector<Permission::PermissionDef> permList;
        for (const auto &defPermission : defPermissions) {
            Permission::PermissionDef perm;
            APP_LOGI("add defPermission %{public}s", defPermission.name.c_str());
            ConvertPermissionDef(perm, defPermission, bundleName);
            permList.emplace_back(perm);
        }
        if (!permList.empty()) {
            ret = Permission::PermissionKit::AddDefPermissions(permList);
        }
    }
    return ret;
}

bool BundlePermissionMgr::CheckPermissionAuthorization(
    const Permission::PermissionDef &permissionDef, const InnerBundleInfo &innerBundleInfo)
{
    APP_LOGI("availableScope type is %{public}d", permissionDef.availableScope);
    if (permissionDef.availableScope == Permission::TypeAvailableScope::AVAILABLE_SCOPE_ALL) {
        return true;
    }
    if (permissionDef.availableScope != Permission::TypeAvailableScope::AVAILABLE_SCOPE_SIGNATURE) {
        return false;
    }
    auto appFeature = innerBundleInfo.GetAppFeature();
    APP_LOGI("appFeature is %{public}s", appFeature.c_str());
    APP_LOGI("permission name is %{public}s", permissionDef.permissionName.c_str());
    auto dataMgr = DelayedSingleton<BundleMgrService>::GetInstance()->GetDataMgr();
    if (!dataMgr) {
        APP_LOGE("Get dataMgr shared_ptr nullptr");
        return false;
    }
    if (appFeature == Constants::HOS_NORMAL_APP) {
        std::string provisionId;
        bool result = dataMgr->GetProvisionId(permissionDef.bundleName, provisionId);
        if (result && (provisionId == innerBundleInfo.GetProvisionId())) {
            APP_LOGI("provisionId is same");
            return true;
        }
    } else if (appFeature == Constants::HOS_SYSTEM_APP || appFeature == Constants::OHOS_SYSTEM_APP) {
        std::string oldAppFeature;
        bool result = dataMgr->GetAppFeature(permissionDef.bundleName, oldAppFeature);
        APP_LOGI("oldAppFeature %{public}s", oldAppFeature.c_str());
        if (result && (oldAppFeature == Constants::HOS_SYSTEM_APP || oldAppFeature == Constants::OHOS_SYSTEM_APP)) {
            APP_LOGI("appFeature is same");
            return true;
        }
    }
    return false;
}

int32_t BundlePermissionMgr::AddAndGrantedReqPermissions(
    const InnerBundleInfo &innerBundleInfo, int32_t userId, bool onlyOneUser)
{
    auto reqPermissions = innerBundleInfo.GetReqPermissions();
    std::vector<std::string> userPermList;
    std::vector<std::string> systemPermList;
    std::vector<std::string> grantPermList;
    for (const auto &reqPermission : reqPermissions) {
        APP_LOGI("add permission %{public}s", reqPermission.name.c_str());
        Permission::PermissionDef permDef;
        int32_t ret = Permission::PermissionKit::GetDefPermission(reqPermission.name, permDef);
        if (ret != Permission::RET_SUCCESS) {
            APP_LOGE("get permission def failed");
            continue;
        }
        if (permDef.grantMode == Permission::GrantMode::USER_GRANT) {
            userPermList.emplace_back(reqPermission.name);
            continue;
        }
        if (permDef.grantMode == Permission::GrantMode::SYSTEM_GRANT) {
            systemPermList.emplace_back(reqPermission.name);
            if (CheckPermissionAuthorization(permDef, innerBundleInfo)) {
                grantPermList.emplace_back(reqPermission.name);
            }
            continue;
        }
    }

    std::string bundleName = innerBundleInfo.GetBundleName();
    APP_LOGI("add permission %{public}s %{public}zu  %{public}zu  %{public}zu",
        bundleName.c_str(),
        userPermList.size(),
        systemPermList.size(),
        grantPermList.size());
    if (!userPermList.empty()) {
        auto ret = AddUserGrantedReqPermissions(bundleName, userPermList, userId);
        if (ret != Permission::RET_SUCCESS) {
            APP_LOGE("AddUserGrantedReqPermissions failed");
        }
    }

    if (onlyOneUser) {
        if (!systemPermList.empty()) {
            auto ret = AddSystemGrantedReqPermissions(bundleName, systemPermList);
            if (ret != Permission::RET_SUCCESS) {
                APP_LOGE("AddSystemGrantedReqPermissions failed");
            }
        }

        for (const auto &perm : grantPermList) {
            auto ret = GrantReqPermissions(bundleName, perm);
            if (ret != Permission::RET_SUCCESS) {
                APP_LOGE("GrantReqPermissions failed");
            }
        }
    }

    return Permission::RET_SUCCESS;
}

bool BundlePermissionMgr::InstallPermissions(
    const InnerBundleInfo &innerBundleInfo, int32_t userId, bool onlyOneUser)
{
    int32_t ret = Permission::RET_SUCCESS;
    if (onlyOneUser) {
        ret = AddDefPermissions(innerBundleInfo);
        if (ret != Permission::RET_SUCCESS) {
            APP_LOGE("AddDefPermissions ret %{public}d", ret);
        }
    }

    ret = AddAndGrantedReqPermissions(innerBundleInfo, userId, onlyOneUser);
    if (ret != Permission::RET_SUCCESS) {
        APP_LOGE("AddAndGrantedReqPermissions ret %{public}d", ret);
    }
    return true;
}

bool BundlePermissionMgr::UpdatePermissions(
    const InnerBundleInfo &innerBundleInfo, int32_t userId, bool onlyOneUser)
{
    // at current time the update permissions process is same as installation process.
    return InstallPermissions(innerBundleInfo, userId, onlyOneUser);
}

bool BundlePermissionMgr::UninstallPermissions(
    const InnerBundleInfo &innerBundleInfo, int32_t userId, bool onlyOneUser)
{
    int32_t ret = Permission::RET_SUCCESS;
    auto bundleName = innerBundleInfo.GetBundleName();
    if (onlyOneUser) {
        ret = RemoveDefPermissions(bundleName);
        if (ret != Permission::RET_SUCCESS) {
            APP_LOGE("RemoveDefPermissions ret %{public}d", ret);
        }

        ret = RemoveSystemGrantedReqPermissions(bundleName);
        if (ret != Permission::RET_SUCCESS) {
            APP_LOGE("RemoveSystemGrantedReqPermissions ret %{public}d", ret);
        }
    }

    ret = RemoveUserGrantedReqPermissions(bundleName, userId);
    if (ret != Permission::RET_SUCCESS) {
        APP_LOGE("RemoveUserGrantedReqPermissions ret %{public}d", ret);
    }

    return true;
}

int32_t BundlePermissionMgr::VerifyPermission(
    const std::string &bundleName, const std::string &permissionName, const int32_t userId)
{
    APP_LOGI(
        "VerifyPermission bundleName %{public}s, permission %{public}s", bundleName.c_str(), permissionName.c_str());
    return Permission::PermissionKit::VerifyPermission(bundleName, permissionName, userId);
}

bool BundlePermissionMgr::CanRequestPermission(
    const std::string &bundleName, const std::string &permissionName, const int32_t userId)
{
    APP_LOGI("CanRequestPermission bundleName %{public}s, permission %{public}s",
        bundleName.c_str(),
        permissionName.c_str());
    return Permission::PermissionKit::CanRequestPermission(bundleName, permissionName, userId);
}

bool BundlePermissionMgr::RequestPermissionFromUser(
    const std::string &bundleName, const std::string &permissionName, const int32_t userId)
{
    APP_LOGI("RequestPermissionFromUser bundleName %{public}s, permission %{public}s",
        bundleName.c_str(),
        permissionName.c_str());
    return (Permission::PermissionKit::GrantUserGrantedPermission(bundleName, permissionName, userId) ==
            Permission::RET_SUCCESS);
}

bool BundlePermissionMgr::GetPermissionDef(const std::string &permissionName, PermissionDef &permissionDef)
{
    APP_LOGI("GetPermissionDef permission %{public}s", permissionName.c_str());
    Permission::PermissionDef permDef;
    int32_t ret = Permission::PermissionKit::GetDefPermission(permissionName, permDef);
    if (ret != Permission::RET_SUCCESS) {
        APP_LOGE("get permission def failed");
        return false;
    }
    ConvertPermissionDef(permDef, permissionDef);
    return true;
}

bool BundlePermissionMgr::CheckCallingPermission(const std::string &permissionName, int32_t userId)
{
    int32_t uid = IPCSkeleton::GetCallingUid();
    if (uid >= Constants::ROOT_UID && uid < Constants::BASE_SYS_UID) {
        APP_LOGE("check permission true for system uid %{public}d", uid);
        return true;
    }
    auto dataMgr = DelayedSingleton<BundleMgrService>::GetInstance()->GetDataMgr();
    if (!dataMgr) {
        APP_LOGE("Get dataMgr shared_ptr nullptr");
        return false;
    }
    std::string bundleName;
    bool result = dataMgr->GetBundleNameForUid(uid, bundleName);
    if (!result) {
        APP_LOGE("cannot get bundle name by uid %{public}d", uid);
        return false;
    }
    // now if the application is system app, pass it through for installing permission
    {
        APP_LOGD(
            "get app bundleName %{public}s and permissionName %{public}s", bundleName.c_str(), permissionName.c_str());
        ApplicationInfo appInfo;
        bool ret = dataMgr->GetApplicationInfo(
            bundleName, ApplicationFlag::GET_BASIC_APPLICATION_INFO, userId, appInfo);
        if (ret && appInfo.isSystemApp && (permissionName == Constants::PERMISSION_INSTALL_BUNDLE)) {
            APP_LOGD("system app %{public}s pass through", bundleName.c_str());
            return true;
        }
    }
    int32_t ret = VerifyPermission(bundleName, permissionName, userId);
    APP_LOGI("permission = %{public}s, uid = %{public}d, ret = %{public}d", permissionName.c_str(), uid, ret);
    return ret == Permission::PermissionState::PERMISSION_GRANTED;
}

int32_t BundlePermissionMgr::GrantReqPermissions(const std::string &bundleName, const std::string &permissionName)
{
    APP_LOGI(
        "GrantReqPermissions bundleName %{public}s, permission %{public}s", bundleName.c_str(), permissionName.c_str());
    return Permission::PermissionKit::GrantSystemGrantedPermission(bundleName, permissionName);
}

int32_t BundlePermissionMgr::AddUserGrantedReqPermissions(
    const std::string &bundleName, const std::vector<std::string> &permList, const int32_t userId)
{
    APP_LOGI("AddUserGrantedReqPermissions bundleName %{public}s %{public}zu", bundleName.c_str(), permList.size());
    return Permission::PermissionKit::AddUserGrantedReqPermissions(bundleName, permList, userId);
}

int32_t BundlePermissionMgr::AddSystemGrantedReqPermissions(
    const std::string &bundleName, const std::vector<std::string> &permList)
{
    APP_LOGI("AddSystemGrantedReqPermissions bundleName %{public}s %{public}zu", bundleName.c_str(), permList.size());
    return Permission::PermissionKit::AddSystemGrantedReqPermissions(bundleName, permList);
}

int32_t BundlePermissionMgr::RemoveDefPermissions(const std::string &bundleName)
{
    APP_LOGI("RemoveDefPermissions bundleName %{public}s", bundleName.c_str());
    return Permission::PermissionKit::RemoveDefPermissions(bundleName);
}

int32_t BundlePermissionMgr::RemoveUserGrantedReqPermissions(const std::string &bundleName, const int32_t userId)
{
    APP_LOGI("RemoveUserGrantedReqPermissions bundleName %{public}s", bundleName.c_str());
    return Permission::PermissionKit::RemoveUserGrantedReqPermissions(bundleName, userId);
}

int32_t BundlePermissionMgr::RemoveSystemGrantedReqPermissions(const std::string &bundleName)
{
    APP_LOGI("RemoveSystemGrantedReqPermissions bundleName %{public}s", bundleName.c_str());
    return Permission::PermissionKit::RemoveSystemGrantedReqPermissions(bundleName);
}
}  // namespace AppExecFwk
}  // namespace OHOS