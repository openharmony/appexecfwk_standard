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

#ifndef BUNDLE_MGR_H
#define BUNDLE_MGR_H
#include <vector>

#include "napi/native_api.h"
#include "napi/native_common.h"
#include "napi/native_node_api.h"

#include "application_info.h"
#include "bundle_mgr_interface.h"
#include "cleancache_callback.h"
#ifdef SUPPORT_GRAPHICS
#include "pixel_map.h"
#endif
#include "ohos/aafwk/content/want.h"

namespace OHOS {
namespace AppExecFwk {

struct CommonNapiInfo {
    napi_env env;
    napi_async_work asyncWork;
    napi_deferred deferred;
    napi_ref callback = 0;
};

struct QueryParameter {
    int flags;
    std::string userId;
};

struct BundleOptions {
    int32_t userId = Constants::UNSPECIFIED_USERID;
};

struct AsyncAbilityInfoCallbackInfo {
    napi_env env;
    napi_async_work asyncWork;
    napi_deferred deferred;
    napi_ref callback = 0;
    OHOS::AAFwk::Want want;
    int32_t flags = 0;
    int32_t userId = Constants::UNSPECIFIED_USERID;
    std::vector<OHOS::AppExecFwk::AbilityInfo> abilityInfos;
    bool ret = false;
    int32_t err = 0;
};

struct AsyncAbilityInfosCallbackInfo {
    napi_env env;
    napi_async_work asyncWork;
    napi_deferred deferred;
    napi_ref callback = 0;
    int32_t flags = 0;
    std::string bundleName;
    std::string abilityName;
    OHOS::AppExecFwk::AbilityInfo abilityInfo;
    bool ret = false;
    int32_t err = 0;
    std::string message;
};

struct AsyncBundleInfoCallbackInfo {
    napi_env env;
    napi_async_work asyncWork;
    napi_deferred deferred;
    napi_ref callback = 0;
    std::string param;
    int32_t flags = 0;
    OHOS::AppExecFwk::BundleInfo bundleInfo;
    bool ret = false;
    int32_t err = 0;
    std::string message;
    BundleOptions bundleOptions;
};

struct AsyncApplicationInfoCallbackInfo {
    napi_env env;
    napi_async_work asyncWork;
    napi_deferred deferred;
    napi_ref callback = 0;
    std::string bundleName;
    int32_t flags = 0;
    int32_t userId;
    OHOS::AppExecFwk::ApplicationInfo appInfo;
    bool ret = false;
    int32_t err = 0;
    std::string message;
};

struct AsyncPermissionDefCallbackInfo {
    napi_env env;
    napi_async_work asyncWork;
    napi_deferred deferred;
    napi_ref callback = 0;
    std::string permissionName;
    OHOS::AppExecFwk::PermissionDef permissionDef;
    bool ret = false;
    int32_t err = 0;
    std::string message;
};

struct AsyncBundleInfosCallbackInfo {
    napi_env env;
    napi_async_work asyncWork;
    napi_deferred deferred;
    napi_ref callback = 0;
    int32_t flags = 0;
    std::vector<OHOS::AppExecFwk::BundleInfo> bundleInfos;
    bool ret = false;
    int32_t err = 0;
    std::string message;
    int32_t userId = Constants::UNSPECIFIED_USERID;
};

struct AsyncApplicationInfosCallbackInfo {
    napi_env env;
    napi_async_work asyncWork;
    napi_deferred deferred;
    napi_ref callback = 0;
    int32_t flags = 0;
    int32_t userId = 0;
    std::vector<OHOS::AppExecFwk::ApplicationInfo> appInfos;
    bool ret = false;
    int32_t err = 0;
    std::string message;
};

struct AsyncAbilityLabelCallbackInfo {
    napi_env env;
    napi_async_work asyncWork;
    napi_deferred deferred;
    napi_ref callback = 0;
    std::string bundleName;
    std::string className;
    std::string abilityLabel;
    int32_t err = 0;
    std::string message;
};

struct InstallResult {
    std::string resultMsg;
    int32_t resultCode;
};

struct AsyncInstallCallbackInfo {
    napi_env env;
    napi_async_work asyncWork;
    napi_deferred deferred;
    napi_ref callback = 0;
    std::vector<std::string> hapFiles;
    std::string bundleName;
    std::string param;
    OHOS::AppExecFwk::InstallParam installParam;
    InstallResult installResult;
    int32_t errCode = 0;
};

struct AsyncGetBundleInstallerCallbackInfo {
    napi_env env;
    napi_async_work asyncWork;
    napi_deferred deferred;
    napi_ref callback = 0;
};

struct AsyncFormInfosCallbackInfo {
    napi_env env;
    napi_async_work asyncWork;
    napi_deferred deferred;
    napi_ref callback = 0;
    std::vector<OHOS::AppExecFwk::FormInfo> formInfos;
    bool ret = false;
};

struct AsyncFormInfosByModuleCallbackInfo {
    napi_env env;
    napi_async_work asyncWork;
    napi_deferred deferred;
    napi_ref callback = 0;
    std::string bundleName;
    std::string moduleName;
    std::vector<OHOS::AppExecFwk::FormInfo> formInfos;
    bool ret = false;
};

struct AsyncFormInfosByAppCallbackInfo {
    napi_env env;
    napi_async_work asyncWork;
    napi_deferred deferred;
    napi_ref callback = 0;
    std::string bundleName;
    std::vector<OHOS::AppExecFwk::FormInfo> formInfos;
    bool ret = false;
};

struct AsyncLaunchWantForBundleCallbackInfo {
    napi_env env;
    napi_async_work asyncWork;
    napi_deferred deferred;
    napi_ref callback = 0;
    std::string bundleName;
    OHOS::AAFwk::Want want;
    bool ret = false;
    int32_t err = 0;
};

struct AsyncGetBundleGidsCallbackInfo {
    napi_env env;
    napi_async_work asyncWork;
    napi_deferred deferred;
    napi_ref callback = 0;
    std::string bundleName;
    std::vector<int32_t> gids;
    int32_t err = 0;
    bool ret = false;
    std::string message;
};

struct AsyncModuleUsageRecordsCallbackInfo {
    napi_env env;
    napi_async_work asyncWork;
    napi_deferred deferred;
    napi_ref callback = 0;
    int32_t number;
    std::vector<OHOS::AppExecFwk::ModuleUsageRecord> moduleUsageRecords;
    bool ret = false;
};

struct AsyncExtensionInfoCallbackInfo {
    napi_env env;
    napi_async_work asyncWork;
    napi_deferred deferred;
    napi_ref callback = 0;
    OHOS::AAFwk::Want want;
    std::string extensionAbilityName;
    int32_t extensionAbilityType = -1;
    int32_t flags = 0;
    int32_t userId = Constants::UNSPECIFIED_USERID;
    std::vector<OHOS::AppExecFwk::ExtensionAbilityInfo> extensionInfos;
    bool ret = false;
    int32_t err = 0;
};

struct AsyncGetNameByUidInfo {
    CommonNapiInfo commonNapiInfo;
    int32_t uid;
    std::string bundleName;
    int32_t err = 0;
    bool ret = false;
};

struct AsyncRegisterAllPermissions {
    napi_env env;
    napi_async_work asyncWork;
    napi_ref callback = 0;
};

struct AsyncRegisterPermissions {
    napi_env env;
    napi_async_work asyncWork;
    napi_ref callback = 0;
    std::vector<int32_t> uids;
};

struct AsyncUnregisterPermissions {
    napi_env env;
    napi_async_work asyncWork;
    napi_ref callback = 0;
    std::vector<int32_t> uids;
    bool ret = false;
};

struct AsyncHandleBundleContext {
    napi_env env = nullptr;
    napi_async_work asyncWork = nullptr;
    napi_deferred deferred = nullptr;
    napi_ref callbackRef = 0;
    napi_ref handleCallback = nullptr;
    OHOS::sptr<CleanCacheCallback> cleanCacheCallback;
    std::string bundleName;
    std::string className;
    int32_t labelId;
    int32_t iconId;
    bool ret = false;
    int32_t err = 0;
};

struct EnabledInfo {
    napi_env env = nullptr;
    napi_async_work asyncWork = nullptr;
    napi_deferred deferred = nullptr;
    napi_ref callbackRef = 0;
    std::string bundleName;
    OHOS::AppExecFwk::AbilityInfo abilityInfo;
    bool isEnable = false;
    bool result = false;
    int32_t errCode = 0;
    std::string errMssage;
};

struct AppPrivilegeLevel {
    napi_env env = nullptr;
    napi_async_work asyncWork = nullptr;
    napi_deferred deferred = nullptr;
    napi_ref callbackRef = 0;
    std::string bundleName;
    std::string appPrivilegeLevel;
    int32_t errCode = 0;
    bool result = false;
    std::string errMssage;
};

struct AsyncAbilityInfo {
    napi_env env = nullptr;
    napi_async_work asyncWork = nullptr;
    napi_deferred deferred = nullptr;
    napi_ref callbackRef = 0;
    std::string bundleName;
    std::string abilityName;
#ifdef SUPPORT_GRAPHICS
    std::shared_ptr<Media::PixelMap> pixelMap;
#endif
    int32_t errCode = 0;
    bool result = false;
    std::string errMssage;
};

struct CheckPackageHasInstalledResponse {
    bool result = false;
};
struct CheckPackageHasInstalledOptions {
    napi_env env = nullptr;
    napi_async_work asyncWork = nullptr;
    napi_ref successRef = nullptr;
    napi_ref failRef = nullptr;
    napi_ref completeRef = nullptr;
    std::string bundleName;
    bool isString = false;
    CheckPackageHasInstalledResponse response;
    int32_t errCode = 0;
};

extern thread_local napi_ref g_classBundleInstaller;

napi_value WrapVoidToJS(napi_env env);
napi_value GetApplicationInfos(napi_env env, napi_callback_info info);
napi_value GetApplicationInfo(napi_env env, napi_callback_info info);
napi_value GetAbilityInfo(napi_env env, napi_callback_info info);
napi_value QueryAbilityInfos(napi_env env, napi_callback_info info);
napi_value GetBundleInfos(napi_env env, napi_callback_info info);
napi_value GetBundleInfo(napi_env env, napi_callback_info info);
napi_value GetBundleArchiveInfo(napi_env env, napi_callback_info info);
napi_value GetLaunchWantForBundle(napi_env env, napi_callback_info info);
napi_value GetPermissionDef(napi_env env, napi_callback_info info);
napi_value GetBundleInstaller(napi_env env, napi_callback_info info);
napi_value Install(napi_env env, napi_callback_info info);
napi_value Recover(napi_env env, napi_callback_info info);
napi_value Uninstall(napi_env env, napi_callback_info info);
napi_value BundleInstallerConstructor(napi_env env, napi_callback_info info);
napi_value GetAllFormsInfo(napi_env env, napi_callback_info info);
napi_value GetFormsInfoByApp(napi_env env, napi_callback_info info);
napi_value GetFormsInfoByModule(napi_env env, napi_callback_info info);
napi_value GetShortcutInfos(napi_env env, napi_callback_info info);
napi_value GetModuleUsageRecords(napi_env env, napi_callback_info info);
napi_value RegisterAllPermissionsChanged(napi_env env, napi_callback_info info);
napi_value UnregisterPermissionsChanged(napi_env env, napi_callback_info info);
napi_value ClearBundleCache(napi_env env, napi_callback_info info);
napi_value SetApplicationEnabled(napi_env env, napi_callback_info info);
napi_value SetAbilityEnabled(napi_env env, napi_callback_info info);
napi_value GetAppPrivilegeLevel(napi_env env, napi_callback_info info);
napi_value QueryExtensionInfoByWant(napi_env env, napi_callback_info info);
napi_value GetNameForUid(napi_env env, napi_callback_info info);
napi_value GetAbilityLabel(napi_env env, napi_callback_info info);
napi_value GetAbilityIcon(napi_env env, napi_callback_info info);
napi_value GetBundleGids(napi_env env, napi_callback_info info);
napi_value IsAbilityEnabled(napi_env env, napi_callback_info info);
napi_value IsApplicationEnabled(napi_env env, napi_callback_info info);
napi_value HasInstalled(napi_env env, napi_callback_info info);
bool UnwrapAbilityInfo(napi_env env, napi_value param, OHOS::AppExecFwk::AbilityInfo& abilityInfo);
void CreateAbilityTypeObject(napi_env env, napi_value value);
void CreateAbilitySubTypeObject(napi_env env, napi_value value);
void CreateDisplayOrientationObject(napi_env env, napi_value value);
void CreateLaunchModeObject(napi_env env, napi_value value);
void CreateModuleUpdateFlagObject(napi_env env, napi_value value);
void CreateFormTypeObject(napi_env env, napi_value value);
void CreateColorModeObject(napi_env env, napi_value value);
void CreateGrantStatusObject(napi_env env, napi_value value);
void CreateModuleRemoveFlagObject(napi_env env, napi_value value);
void CreateSignatureCompareResultObject(napi_env env, napi_value value);
void CreateShortcutExistenceObject(napi_env env, napi_value value);
void CreateQueryShortCutFlagObject(napi_env env, napi_value value);
void CreateBundleFlagObject(napi_env env, napi_value value);
void CreateInstallErrorCodeObject(napi_env env, napi_value value);
void CreateExtensionAbilityTypeObject(napi_env env, napi_value value);
void CreateExtensionFlagObject(napi_env env, napi_value value);
}  // namespace AppExecFwk
}  // namespace OHOS
#endif /* BUNDLE_MGR_H */