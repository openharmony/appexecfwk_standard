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

#ifndef BUNDLE_MGR_H_
#define BUNDLE_MGR_H_
#include <vector>

#include "napi/native_common.h"
#include "napi/native_node_api.h"

#include "hilog_wrapper.h"
#include "ohos/aafwk/content/want.h"
#include "bundle_mgr_interface.h"
#include "application_info.h"

namespace OHOS {
namespace AppExecFwk {
struct QueryParameter {
    int flags;
    std::string userId;
};

struct AsyncAbilityInfoCallbackInfo {
    napi_env env;
    napi_async_work asyncWork;
    napi_deferred deferred;
    napi_ref callback = 0;
    OHOS::AAFwk::Want want;
    std::vector<OHOS::AppExecFwk::AbilityInfo> abilityInfos;
    bool ret = false;
};

struct AsyncBundleInfoCallbackInfo {
    napi_env env;
    napi_async_work asyncWork;
    napi_deferred deferred;
    napi_ref callback = 0;
    std::string param;
    OHOS::AppExecFwk::BundleFlag bundleFlag;
    OHOS::AppExecFwk::BundleInfo bundleInfo;
    bool ret = false;
};

struct AsyncApplicationInfoCallbackInfo {
    napi_env env;
    napi_async_work asyncWork;
    napi_deferred deferred;
    napi_ref callback = 0;
    std::string bundleName;
    OHOS::AppExecFwk::ApplicationFlag applicationFlag;
    int userId;
    OHOS::AppExecFwk::ApplicationInfo appInfo;
    bool ret = false;
};

struct AsyncPermissionDefCallbackInfo {
    napi_env env;
    napi_async_work asyncWork;
    napi_deferred deferred;
    napi_ref callback = 0;
    std::string permissionName;
    OHOS::AppExecFwk::PermissionDef permissionDef;
    bool ret = false;
};

struct AsyncPermissionCallbackInfo {
    napi_env env;
    napi_async_work asyncWork;
    napi_deferred deferred;
    napi_ref callback = 0;
    std::string bundleName;
    std::string permission;
    int ret = -1;
};

struct AsyncBundleInfosCallbackInfo {
    napi_env env;
    napi_async_work asyncWork;
    napi_deferred deferred;
    napi_ref callback = 0;
    OHOS::AppExecFwk::BundleFlag bundleFlag;
    std::vector<OHOS::AppExecFwk::BundleInfo> bundleInfos;
    bool ret = false;
};

struct AsyncApplicationInfosCallbackInfo {
    napi_env env;
    napi_async_work asyncWork;
    napi_deferred deferred;
    napi_ref callback = 0;
    OHOS::AppExecFwk::ApplicationFlag applicationFlag;
    int userId;
    std::vector<OHOS::AppExecFwk::ApplicationInfo> appInfos;
    bool ret = false;
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
    std::string param;
    OHOS::AppExecFwk::InstallParam installParam;
    InstallResult installResult;
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

struct AsyncShortcutInfosCallbackInfo {
    napi_env env;
    napi_async_work asyncWork;
    napi_deferred deferred;
    napi_ref callback = 0;
    std::string bundleName;
    std::vector<OHOS::AppExecFwk::ShortcutInfo> shortcutInfos;
    bool ret = false;
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

extern napi_ref g_classBundleInstaller;

napi_value GetApplicationInfos(napi_env env, napi_callback_info info);
napi_value GetApplicationInfo(napi_env env, napi_callback_info info);
napi_value QueryAbilityInfos(napi_env env, napi_callback_info info);
napi_value GetBundleInfos(napi_env env, napi_callback_info info);
napi_value GetBundleInfo(napi_env env, napi_callback_info info);
napi_value GetBundleArchiveInfo(napi_env env, napi_callback_info info);
napi_value GetPermissionDef(napi_env env, napi_callback_info info);
napi_value GetBundleInstaller(napi_env env, napi_callback_info info);
napi_value Install(napi_env env, napi_callback_info info);
napi_value Uninstall(napi_env env, napi_callback_info info);
napi_value BundleInstallerConstructor(napi_env env, napi_callback_info info);
napi_value GetAllFormsInfo(napi_env env, napi_callback_info info);
napi_value GetFormsInfoByApp(napi_env env, napi_callback_info info);
napi_value GetFormsInfoByModule(napi_env env, napi_callback_info info);
napi_value GetShortcutInfos(napi_env env, napi_callback_info info);
napi_value GetModuleUsageRecords(napi_env env, napi_callback_info info);
napi_value RegisterAllPermissionsChanged(napi_env env, napi_callback_info info);
napi_value UnregisterPermissionsChanged(napi_env env, napi_callback_info info);
napi_value CheckPermission(napi_env env, napi_callback_info info);
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
void CreateInstallErrorCodeObject(napi_env env, napi_value value);
}  // namespace AppExecFwk
}  // namespace OHOS
#endif /* BUNDLE_MGR_H_ */
