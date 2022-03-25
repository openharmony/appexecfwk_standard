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

#include "bundle_mgr_proxy_ndk.h"

#include "ipc_types.h"
#include "parcel.h"
#include "string_ex.h"

#include "app_log_wrapper.h"
#include "appexecfwk_errors.h"
#include "bundle_constants.h"
#include "securec.h"

namespace OHOS {
namespace AppExecFwk {

BundleMgrProxyNdk::BundleMgrProxyNdk(const sptr<IRemoteObject> &impl) : IRemoteProxy<IBundleMgr>(impl)
{
    APP_LOGI("create bundle mgr proxy ndk instance");
}

BundleMgrProxyNdk::~BundleMgrProxyNdk()
{
    APP_LOGI("destroy create bundle mgr proxy ndk instance");
}

bool BundleMgrProxyNdk::GetApplicationInfo(
    const std::string &appName, const ApplicationFlag flag, const int userId, ApplicationInfo &appInfo)
{
    return false;
}

bool BundleMgrProxyNdk::GetApplicationInfo(
    const std::string &appName, int32_t flags, int32_t userId, ApplicationInfo &appInfo)
{
    return false;
}

bool BundleMgrProxyNdk::GetApplicationInfos(
    const ApplicationFlag flag, int userId, std::vector<ApplicationInfo> &appInfos)
{
    return false;
}

bool BundleMgrProxyNdk::GetApplicationInfos(
    int32_t flags, int32_t userId, std::vector<ApplicationInfo> &appInfos)
{
    return false;
}

bool BundleMgrProxyNdk::GetBundleInfo(
    const std::string &bundleName, const BundleFlag flag, BundleInfo &bundleInfo, int32_t userId)
{
    return false;
}

bool BundleMgrProxyNdk::GetBundleInfo(
    const std::string &bundleName, int32_t flags, BundleInfo &bundleInfo, int32_t userId)
{
    return false;
}

bool BundleMgrProxyNdk::GetBundleInfos(
    const BundleFlag flag, std::vector<BundleInfo> &bundleInfos, int32_t userId)
{
    return false;
}

bool BundleMgrProxyNdk::GetBundleInfos(
    int32_t flags, std::vector<BundleInfo> &bundleInfos, int32_t userId)
{
    return false;
}

int BundleMgrProxyNdk::GetUidByBundleName(const std::string &bundleName, const int userId)
{
    return Constants::INVALID_UID;
}

std::string BundleMgrProxyNdk::GetAppIdByBundleName(const std::string &bundleName, const int userId)
{
    if (bundleName.empty()) {
        APP_LOGE("failed to GetAppIdByBundleName due to bundleName empty");
        return Constants::EMPTY_STRING;
    }
    APP_LOGD("begin to get appId of %{public}s", bundleName.c_str());

    MessageParcel data;
    if (!data.WriteInterfaceToken(GetDescriptor())) {
        APP_LOGE("failed to GetAppIdByBundleName due to write InterfaceToken fail");
        return Constants::EMPTY_STRING;
    }
    if (!data.WriteString(bundleName)) {
        APP_LOGE("failed to GetAppIdByBundleName due to write bundleName fail");
        return Constants::EMPTY_STRING;
    }
    if (!data.WriteInt32(userId)) {
        APP_LOGE("failed to GetAppIdByBundleName due to write uid fail");
        return Constants::EMPTY_STRING;
    }

    MessageParcel reply;
    if (!SendTransactCmd(IBundleMgr::Message::GET_APPID_BY_BUNDLE_NAME, data, reply)) {
        APP_LOGE("failed to GetAppIdByBundleName from server");
        return Constants::EMPTY_STRING;
    }
    std::string appId = reply.ReadString();
    APP_LOGD("appId is %{private}s", appId.c_str());
    return appId;
}

bool BundleMgrProxyNdk::GetBundleNameForUid(const int uid, std::string &bundleName)
{
    return false;
}

bool BundleMgrProxyNdk::GetBundlesForUid(const int uid, std::vector<std::string> &bundleNames)
{
    return false;
}

bool BundleMgrProxyNdk::GetNameForUid(const int uid, std::string &name)
{
    return false;
}

bool BundleMgrProxyNdk::GetBundleGids(const std::string &bundleName, std::vector<int> &gids)
{
    return false;
}

bool BundleMgrProxyNdk::GetBundleGidsByUid(const std::string &bundleName, const int &uid, std::vector<int> &gids)
{
    return false;
}

std::string BundleMgrProxyNdk::GetAppType(const std::string &bundleName)
{
    return Constants::EMPTY_STRING;
}

bool BundleMgrProxyNdk::CheckIsSystemAppByUid(const int uid)
{
    return false;
}

bool BundleMgrProxyNdk::GetBundleInfosByMetaData(const std::string &metaData, std::vector<BundleInfo> &bundleInfos)
{
    return false;
}

bool BundleMgrProxyNdk::QueryAbilityInfo(const Want &want, AbilityInfo &abilityInfo)
{
    return false;
}

bool BundleMgrProxyNdk::QueryAbilityInfo(const Want &want, int32_t flags, int32_t userId, AbilityInfo &abilityInfo)
{
    return false;
}

bool BundleMgrProxyNdk::QueryAbilityInfos(const Want &want, std::vector<AbilityInfo> &abilityInfos)
{
    return false;
}

bool BundleMgrProxyNdk::QueryAbilityInfos(
    const Want &want, int32_t flags, int32_t userId, std::vector<AbilityInfo> &abilityInfos)
{
    return false;
}

bool BundleMgrProxyNdk::QueryAllAbilityInfos(const Want &want, int32_t userId, std::vector<AbilityInfo> &abilityInfos)
{
    return false;
}

bool BundleMgrProxyNdk::QueryAbilityInfosForClone(const Want &want, std::vector<AbilityInfo> &abilityInfos)
{
    return false;
}

bool BundleMgrProxyNdk::QueryAbilityInfoByUri(const std::string &abilityUri, AbilityInfo &abilityInfo)
{
    return false;
}

bool BundleMgrProxyNdk::QueryAbilityInfosByUri(const std::string &abilityUri, std::vector<AbilityInfo> &abilityInfos)
{
    return false;
}

bool BundleMgrProxyNdk::QueryAbilityInfoByUri(
    const std::string &abilityUri, int32_t userId, AbilityInfo &abilityInfo)
{
    return false;
}

bool BundleMgrProxyNdk::QueryKeepAliveBundleInfos(std::vector<BundleInfo> &bundleInfos)
{
    return false;
}

std::string BundleMgrProxyNdk::GetAbilityLabel(const std::string &bundleName, const std::string &className)
{
    return Constants::EMPTY_STRING;
}

bool BundleMgrProxyNdk::GetBundleArchiveInfo(const std::string &hapFilePath, const BundleFlag flag, BundleInfo &bundleInfo)
{
    return false;
}

bool BundleMgrProxyNdk::GetBundleArchiveInfo(const std::string &hapFilePath, int32_t flags, BundleInfo &bundleInfo)
{
    return false;
}

bool BundleMgrProxyNdk::GetHapModuleInfo(const AbilityInfo &abilityInfo, HapModuleInfo &hapModuleInfo)
{
    return false;
}

bool BundleMgrProxyNdk::GetLaunchWantForBundle(const std::string &bundleName, Want &want)
{
    return false;
}

int BundleMgrProxyNdk::CheckPublicKeys(const std::string &firstBundleName, const std::string &secondBundleName)
{
    return Constants::SIGNATURE_UNKNOWN_BUNDLE;
}

int BundleMgrProxyNdk::CheckPermission(const std::string &bundleName, const std::string &permission)
{
    return Constants::PERMISSION_NOT_GRANTED;
}

int BundleMgrProxyNdk::CheckPermissionByUid(const std::string &bundleName, const std::string &permission, const int userId)
{
    return Constants::PERMISSION_NOT_GRANTED;
}

bool BundleMgrProxyNdk::GetPermissionDef(const std::string &permissionName, PermissionDef &permissionDef)
{
    return false;
}

bool BundleMgrProxyNdk::GetAllPermissionGroupDefs(std::vector<PermissionDef> &permissionDefs)
{
    return false;
}

bool BundleMgrProxyNdk::GetAppsGrantedPermissions(
    const std::vector<std::string> &permissions, std::vector<std::string> &appNames)
{
    return false;
}

bool BundleMgrProxyNdk::HasSystemCapability(const std::string &capName)
{
    return false;
}

bool BundleMgrProxyNdk::GetSystemAvailableCapabilities(std::vector<std::string> &systemCaps)
{
    return false;
}

bool BundleMgrProxyNdk::IsSafeMode()
{
    return false;
}

bool BundleMgrProxyNdk::CleanBundleCacheFiles(
    const std::string &bundleName, const sptr<ICleanCacheCallback> &cleanCacheCallback, int32_t userId)
{
    return false;
}

bool BundleMgrProxyNdk::CleanBundleDataFiles(const std::string &bundleName, const int userId)
{
    return false;
}

bool BundleMgrProxyNdk::RegisterBundleStatusCallback(const sptr<IBundleStatusCallback> &bundleStatusCallback)
{
    return false;
}

bool BundleMgrProxyNdk::ClearBundleStatusCallback(const sptr<IBundleStatusCallback> &bundleStatusCallback)
{
    return false;
}

bool BundleMgrProxyNdk::UnregisterBundleStatusCallback()
{
    return false;
}

bool BundleMgrProxyNdk::DumpInfos(
    const DumpFlag flag, const std::string &bundleName, int32_t userId, std::string &result)
{
    return false;
}

bool BundleMgrProxyNdk::IsApplicationEnabled(const std::string &bundleName)
{
    return false;
}

bool BundleMgrProxyNdk::SetApplicationEnabled(const std::string &bundleName, bool isEnable, int32_t userId)
{
    return false;
}

bool BundleMgrProxyNdk::IsAbilityEnabled(const AbilityInfo &abilityInfo)
{
    return false;
}

bool BundleMgrProxyNdk::SetAbilityEnabled(const AbilityInfo &abilityInfo, bool isEnabled, int32_t userId)
{
    return false;
}

bool BundleMgrProxyNdk::GetAbilityInfo(
    const std::string &bundleName, const std::string &abilityName, AbilityInfo &abilityInfo)
{
    return false;
}

std::string BundleMgrProxyNdk::GetAbilityIcon(const std::string &bundleName, const std::string &className)
{
    return Constants::EMPTY_STRING;
}

#ifdef SUPPORT_GRAPHICS
std::shared_ptr<Media::PixelMap> BundleMgrProxyNdk::GetAbilityPixelMapIcon(const std::string &bundleName,
    const std::string &abilityName)
{
    return nullptr;
}
#endif

sptr<IBundleInstaller> BundleMgrProxyNdk::GetBundleInstaller()
{
    return nullptr;
}

sptr<IBundleUserMgr> BundleMgrProxyNdk::GetBundleUserMgr()
{
    return nullptr;
}

bool BundleMgrProxyNdk::CanRequestPermission(
    const std::string &bundleName, const std::string &permissionName, const int userId)
{
    return false;
}

bool BundleMgrProxyNdk::RequestPermissionFromUser(
    const std::string &bundleName, const std::string &permission, const int userId)
{
    return false;
}

bool BundleMgrProxyNdk::RegisterAllPermissionsChanged(const sptr<OnPermissionChangedCallback> &callback)
{
    return false;
}

bool BundleMgrProxyNdk::RegisterPermissionsChanged(
    const std::vector<int> &uids, const sptr<OnPermissionChangedCallback> &callback)
{
    return false;
}

bool BundleMgrProxyNdk::UnregisterPermissionsChanged(const sptr<OnPermissionChangedCallback> &callback)
{
    return false;
}

bool BundleMgrProxyNdk::GetAllFormsInfo(std::vector<FormInfo> &formInfos)
{
    return false;
}

bool BundleMgrProxyNdk::GetFormsInfoByApp(const std::string &bundleName, std::vector<FormInfo> &formInfos)
{
    return false;
}

bool BundleMgrProxyNdk::GetFormsInfoByModule(
    const std::string &bundleName, const std::string &moduleName, std::vector<FormInfo> &formInfos)
{
    return false;
}

bool BundleMgrProxyNdk::GetShortcutInfos(const std::string &bundleName, std::vector<ShortcutInfo> &shortcutInfos)
{
    return false;
}

bool BundleMgrProxyNdk::GetAllCommonEventInfo(const std::string &eventKey, std::vector<CommonEventInfo> &commonEventInfos)
{
    return false;
}

bool BundleMgrProxyNdk::GetModuleUsageRecords(const int32_t number, std::vector<ModuleUsageRecord> &moduleUsageRecords)
{
    return false;
}

bool BundleMgrProxyNdk::NotifyAbilityLifeStatus(
    const std::string &bundleName, const std::string &abilityName, const int64_t launchTime, const int uid)
{
    return false;
}

bool BundleMgrProxyNdk::CheckBundleNameInAllowList(const std::string &bundleName)
{
    return false;
}

bool BundleMgrProxyNdk::BundleClone(const std::string &bundleName)
{
    return false;
}

bool BundleMgrProxyNdk::RemoveClonedBundle(const std::string &bundleName, const int32_t uid)
{
    return false;
}

bool BundleMgrProxyNdk::GetDistributedBundleInfo(
    const std::string &networkId, int32_t userId, const std::string &bundleName,
    DistributedBundleInfo &distributedBundleInfo)
{
    return false;
}

std::string BundleMgrProxyNdk::GetAppPrivilegeLevel(const std::string &bundleName, int32_t userId)
{
    return Constants::EMPTY_STRING;
}

bool BundleMgrProxyNdk::QueryExtensionAbilityInfos(const Want &want, const int32_t &flag, const int32_t &userId,
    std::vector<ExtensionAbilityInfo> &extensionInfos)
{
    return false;
}

bool BundleMgrProxyNdk::QueryExtensionAbilityInfos(const Want &want, const ExtensionAbilityType &extensionType,
    const int32_t &flag, const int32_t &userId, std::vector<ExtensionAbilityInfo> &extensionInfos)
{
    return false;
}

bool BundleMgrProxyNdk::QueryExtensionAbilityInfos(const ExtensionAbilityType &extensionType, const int32_t &userId,
    std::vector<ExtensionAbilityInfo> &extensionInfos)
{
    return false;
}

bool BundleMgrProxyNdk::VerifyCallingPermission(const std::string &permission)
{
    return false;
}

std::vector<std::string> BundleMgrProxyNdk::GetAccessibleAppCodePaths(int32_t userId)
{
    std::vector<std::string> vec;
    return vec;
}

bool BundleMgrProxyNdk::QueryExtensionAbilityInfoByUri(const std::string &uri, int32_t userId,
    ExtensionAbilityInfo &extensionAbilityInfo)
{
    return false;
}

bool BundleMgrProxyNdk::SendTransactCmd(IBundleMgr::Message code, MessageParcel &data, MessageParcel &reply)
{
    MessageOption option(MessageOption::TF_SYNC);

    sptr<IRemoteObject> remote = Remote();
    if (!remote) {
        APP_LOGE("fail to send transact cmd %{public}d due to remote object", code);
        return false;
    }
    int32_t result = remote->SendRequest(static_cast<uint32_t>(code), data, reply, option);
    if (result != NO_ERROR) {
        APP_LOGE("receive error transact code %{public}d in transact cmd %{public}d", result, code);
        return false;
    }
    return true;
}
}  // namespace AppExecFwk
}  // namespace OHOS