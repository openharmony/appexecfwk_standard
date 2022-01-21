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

#ifndef FOUNDATION_APPEXECFWK_SERVICES_BUNDLEMGR_INCLUDE_BUNDLE_PERMISSION_MGR_H
#define FOUNDATION_APPEXECFWK_SERVICES_BUNDLEMGR_INCLUDE_BUNDLE_PERMISSION_MGR_H

#include "accesstoken_kit.h"
#include "bundle_constants.h"
#include "inner_bundle_info.h"
#include "permission_def.h"
#include "permission/permission_kit.h"

namespace OHOS {
namespace AppExecFwk {
class BundlePermissionMgr {
public:
    /**
     * @brief Initialize the system defined permissions on first start up.
     * @return Returns true if the permissions initialized successfully; returns false otherwise.
     */
    static bool InitPermissions();
    /**
     * @brief Handle the permissions in installation progress.
     * @param innerBundleInfo Indicates the current installing inner bundle information.
     * @param userId Indicates the userId of the bundle.
     * @param onlyOneUser Indicates is the only one user.
     * @return Returns true if the permissions install successfully; returns false otherwise.
     */
    static bool InstallPermissions(
        const InnerBundleInfo &innerBundleInfo, int32_t userId = Constants::DEFAULT_USERID, bool onlyOneUser = true);
    /**
     * @brief Handle the permissions in updating progress.
     * @param innerBundleInfo Indicates the current installing inner bundle information.
     * @param userId Indicates the userId of the bundle.
     * @param onlyOneUser Indicates is the only one user.
     * @return Returns true if the permissions updating successfully; returns false otherwise.
     */
    static bool UpdatePermissions(
        const InnerBundleInfo &innerBundleInfo, int32_t userId = Constants::DEFAULT_USERID, bool onlyOneUser = true);
    /**
     * @brief Handle the permissions in uninstall progress.
     * @param innerBundleInfo Indicates the current installing inner bundle information.
     * @param userId Indicates the userId of the bundle.
     * @param onlyOneUser Indicates is the only one user.
     * @return Returns true if the permissions uninstall successfully; returns false otherwise.
     */
    static bool UninstallPermissions(
        const InnerBundleInfo &innerBundleInfo, int32_t userId = Constants::DEFAULT_USERID, bool onlyOneUser = true);
    /**
     * @brief Check the permission whether granted for calling process.
     * @param permissionName Indicates the permission name.
     * @param userId Indicates the userId of the bundle.
     * @return Returns true if the permissions has been granted; returns false otherwise.
     */
    static bool CheckCallingPermission(
        const std::string &permissionName, int32_t userId = Constants::DEFAULT_USERID);
    /**
     * @brief Verify whether a specified bundle has been granted a specific permission.
     * @param bundleName Indicates the name of the bundle to check.
     * @param permission Indicates the permission to check.
     * @param userId Indicates the userId of the bundle.
     * @return Returns 0 if the bundle has the permission; returns -1 otherwise.
     */
    static int32_t VerifyPermission(const std::string &bundleName, const std::string &permissionName,
        const int32_t userId);
    /**
     * @brief Obtains detailed information about a specified permission.
     * @param permissionName Indicates the name of the permission.
     * @param permissionDef Indicates the object containing detailed information about the given permission.
     * @return Returns true if the PermissionDef object is successfully obtained; returns false otherwise.
     */
    static bool GetPermissionDef(const std::string &permissionName, PermissionDef &permissionDef);
    /**
     * @brief Confirms with the permission management module to check whether a request prompt is required for granting
     * a certain permission.
     * @param bundleName Indicates the name of the bundle.
     * @param permission Indicates the permission to quest.
     * @param userId Indicates the userId of the bundle.
     * @return Returns true if the current application does not have the permission and the user does not turn off
     * further requests; returns false if the current application already has the permission, the permission is rejected
     * by the system, or the permission is denied by the user and the user has turned off further requests.
     */
    static bool CanRequestPermission(
        const std::string &bundleName, const std::string &permissionName, const int32_t userId);
    /**
     * @brief Requests a certain permission from user.
     * @param bundleName Indicates the name of the bundle.
     * @param permission Indicates the permission to request.
     * @param userId Indicates the userId of the bundle.
     * @return Returns true if the permission request successfully; returns false otherwise.
     */
    static bool RequestPermissionFromUser(
        const std::string &bundleName, const std::string &permissionName, const int32_t userId);

    static Security::AccessToken::AccessTokenID CreateAccessTokenId(
        const InnerBundleInfo &innerBundleInfo, const std::string bundleName, const int32_t userId);

    static bool UpdateDefineAndRequestPermissions(const Security::AccessToken::AccessTokenID tokenId,
        const InnerBundleInfo &oldInfo, const InnerBundleInfo &newInfo, std::vector<std::string> &newRequestPermName);

    static bool AddDefineAndRequestPermissions(const Security::AccessToken::AccessTokenID tokenId,
        const InnerBundleInfo &innerBundleInfo, std::vector<std::string> &newRequestPermName);

    static int32_t DeleteAccessTokenId(const Security::AccessToken::AccessTokenID tokenId);

    static bool GrantRequestPermissions(const InnerBundleInfo &innerBundleInfo,
        const Security::AccessToken::AccessTokenID tokenId);

    static bool GrantRequestPermissions(const InnerBundleInfo &innerBundleInfo,
        const std::vector<std::string> &requestPermName,
        const Security::AccessToken::AccessTokenID tokenId);

    static bool GetRequestPermissionStates(BundleInfo &bundleInfo);

    static int32_t ClearUserGrantedPermissionState(const Security::AccessToken::AccessTokenID tokenId);

    static bool GetDefinePermission(
        const std::string& permissionName, PermissionDefine& permissionDefResult);

    static int32_t VerifyPermission(Security::AccessToken::AccessTokenID tokenId, const std::string &permissionName);

private:
    /**
     * @brief Add the defPermissions to permission kit.
     * @param innerBundleInfo Indicates the current installing inner bundle information.
     * @return Returns 0 if the defPermissions add successfully; returns -1 otherwise.
     */
    static int32_t AddDefPermissions(const InnerBundleInfo &innerBundleInfo);
    /**
     * @brief Add and grant the reqPermissions to permission kit.
     * @param innerBundleInfo Indicates the current installing inner bundle information.
     * @param userId Indicates the userId of the bundle.
     * @param onlyOneUser Indicates is the only one user.
     * @return Returns 0 if the reqPermissions add and grant successfully; returns -1 otherwise.
     */
    static int32_t AddAndGrantedReqPermissions(const InnerBundleInfo &innerBundleInfo,
        int32_t userId = Constants::DEFAULT_USERID, bool onlyOneUser = true);
    /**
     * @brief Grant a reqPermission from permission kit.
     * @param bundleName Indicates the name of the bundle.
     * @param permissionName Indicates the permission.
     * @return Returns 0 if the reqPermission grant successfully; returns -1 otherwise.
     */
    static int32_t GrantReqPermissions(const std::string &bundleName, const std::string &permissionName);
    /**
     * @brief Add user granted reqPermissions to permission kit.
     * @param bundleName Indicates the name of the bundle to add.
     * @param permList Indicates the list of reqPermission to add.
     * @param userId Indicates the userId of the bundle.
     * @return Returns 0 if the reqPermissions add successfully; returns -1 otherwise.
     */
    static int32_t AddUserGrantedReqPermissions(
        const std::string &bundleName, const std::vector<std::string> &permList, const int32_t userId);
    /**
     * @brief Add system granted reqPermissions to permission kit.
     * @param bundleName Indicates the name of the bundle to add.
     * @param permList Indicates the list of reqPermission to add.
     * @return Returns 0 if the reqPermissions add successfully; returns -1 otherwise.
     */
    static int32_t AddSystemGrantedReqPermissions(const std::string &bundleName,
        const std::vector<std::string> &permList);
    /**
     * @brief Check whether a permission need to be granted.
     * @param permissionDef Indicates the definition of a permission.
     * @param innerBundleInfo Indicates the current installing inner bundle information.
     * @return Returns true if the permission need to be granted; returns false otherwise.
     */
    static bool CheckPermissionAuthorization(
        const Security::Permission::PermissionDef &permissionDef, const InnerBundleInfo &innerBundleInfo);
    /**
     * @brief Remove the defPermissions from permission kit.
     * @param innerBundleInfo Indicates the current uninstalling inner bundle information.
     * @return Returns 0 if the defPermissions removed successfully; returns -1 otherwise.
     */
    static int32_t RemoveDefPermissions(const std::string &bundleName);
    /**
     * @brief Remove user granted reqPermissions from permission kit.
     * @param bundleName Indicates the name of the bundle to remove.
     * @param userId Indicates the userId of the bundle.
     * @return Returns 0 if the reqPermissions removed successfully; returns -1 otherwise.
     */
    static int32_t RemoveUserGrantedReqPermissions(const std::string &bundleName, const int32_t userId);
    /**
     * @brief Remove system granted reqPermissions from permission kit.
     * @param bundleName Indicates the name of the bundle to remove.
     * @return Returns 0 if the reqPermissions removed successfully; returns -1 otherwise.
     */
    static int32_t RemoveSystemGrantedReqPermissions(const std::string &bundleName);

    static std::vector<Security::AccessToken::PermissionDef> GetPermissionDefList(
        const InnerBundleInfo &innerBundleInfo);

    static std::vector<Security::AccessToken::PermissionStateFull> GetPermissionStateFullList(
        const InnerBundleInfo &innerBundleInfo);

    static bool CheckGrantPermission(const Security::AccessToken::PermissionDef &permDef,
        const std::string &apl,
        const std::vector<std::string> &acls);

    static bool GetNewPermissionDefList(Security::AccessToken::AccessTokenID tokenId,
        const std::vector<Security::AccessToken::PermissionDef> &permissionDef,
        std::vector<Security::AccessToken::PermissionDef> &newPermission);

    static bool GetNewPermissionStateFull(Security::AccessToken::AccessTokenID tokenId,
        const std::vector<Security::AccessToken::PermissionStateFull> &permissionState,
        std::vector<Security::AccessToken::PermissionStateFull> &newPermissionState,
        std::vector<std::string> &newRequestPermName);

    static bool GetAllReqPermissionStateFull(Security::AccessToken::AccessTokenID tokenId,
        std::vector<Security::AccessToken::PermissionStateFull> &newPermissionState);

    static bool InnerGrantRequestPermissions(const std::vector<RequestPermission> &reqPermissions,
        const std::string &apl, const std::vector<std::string> &acls,
        const Security::AccessToken::AccessTokenID tokenId);

    static Security::AccessToken::ATokenAplEnum GetTokenApl(const std::string &apl);

    static Security::AccessToken::HapPolicyParams CreateHapPolicyParam(const InnerBundleInfo &innerBundleInfo);

    static void ConvertPermissionDef(const Security::Permission::PermissionDef &permDef, PermissionDef &permissionDef);
    static void ConvertPermissionDef(
        Security::Permission::PermissionDef &permDef, const DefPermission &defPermission,
        const std::string &bundleName);

    static void ConvertPermissionDef(const Security::AccessToken::PermissionDef &permDef,
        PermissionDefine &permissionDef);
    static void ConvertPermissionDef(
        Security::AccessToken::PermissionDef &permDef, const DefinePermission &defPermission,
        const std::string &bundleName);

    static std::vector<std::string> GetNeedDeleteDefinePermissionName(const InnerBundleInfo &oldInfo,
        const InnerBundleInfo &newInfo);
    
    static std::vector<std::string> GetNeedDeleteRequestPermissionName(const InnerBundleInfo &oldInfo,
        const InnerBundleInfo &newInfo);
};
}  // namespace AppExecFwk
}  // namespace OHOS
#endif  // FOUNDATION_APPEXECFWK_SERVICES_BUNDLEMGR_INCLUDE_BUNDLE_PERMISSION_MGR_H