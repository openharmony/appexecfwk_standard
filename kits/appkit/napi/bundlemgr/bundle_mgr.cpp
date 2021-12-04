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
#include "bundle_mgr.h"

#include "napi/native_api.h"
#include "napi/native_node_api.h"
#include "securec.h"
#include "system_ability_definition.h"
#include "if_system_ability_manager.h"
#include "iservice_registry.h"
#include "installer_callback.h"
#include "permission_callback.h"
#include "ipc_skeleton.h"
#include "bundle_constants.h"

namespace OHOS {
namespace AppExecFwk {

using namespace OHOS;
using namespace OHOS::AAFwk;
using namespace OHOS::AppExecFwk;
namespace {

constexpr size_t CALLBACK_SIZE = 1;
constexpr size_t ARGS_SIZE_ONE = 1;
constexpr size_t ARGS_SIZE_TWO = 2;
constexpr size_t ARGS_SIZE_THREE = 3;
constexpr size_t ARGS_SIZE_FOUR = 4;
constexpr int32_t DEFAULT_INT32 = 0;
constexpr int32_t PARAM0 = 0;
constexpr int32_t PARAM1 = 1;
constexpr int32_t PARAM2 = 2;
constexpr int32_t NAPI_RETURN_FAILED = -1;
constexpr int32_t NAPI_RETURN_ZERO = 0;
constexpr int32_t NAPI_RETURN_ONE = 1;
constexpr int32_t NAPI_RETURN_TWO = 2;
constexpr int32_t NAPI_RETURN_THREE = 3;
constexpr int32_t CODE_SUCCESS = 0;
constexpr int32_t CODE_FAILED = -1;
enum class InstallErrorCode {
    SUCCESS = 0,
    STATUS_INSTALL_FAILURE = 1,
    STATUS_INSTALL_FAILURE_ABORTED = 2,
    STATUS_INSTALL_FAILURE_INVALID = 3,
    STATUS_INSTALL_FAILURE_CONFLICT = 4,
    STATUS_INSTALL_FAILURE_STORAGE = 5,
    STATUS_INSTALL_FAILURE_INCOMPATIBLE = 6,
    STATUS_UNINSTALL_FAILURE = 7,
    STATUS_UNINSTALL_FAILURE_BLOCKED = 8,
    STATUS_UNINSTALL_FAILURE_ABORTED = 9,
    STATUS_UNINSTALL_FAILURE_CONFLICT = 10,
    STATUS_INSTALL_FAILURE_DOWNLOAD_TIMEOUT = 0x0B,
    STATUS_INSTALL_FAILURE_DOWNLOAD_FAILED = 0x0C,
    STATUS_ABILITY_NOT_FOUND = 0x40,
    STATUS_BMS_SERVICE_ERROR = 0x41
};

const std::string PERMISSION_CHANGE = "permissionChange";
const std::string ANY_PERMISSION_CHANGE = "anyPermissionChange";

std::mutex permissionsCallbackMutex;
std::mutex anyPermissionsCallbackMutex;

struct PermissionsKey {
    napi_ref callback = 0;
    std::vector<int32_t> uids;
    bool operator<(const PermissionsKey &other) const
    {
        return this->callback < other.callback;
    }
};

std::map<PermissionsKey, OHOS::sptr<PermissionCallback>> permissionsCallback;
std::map<napi_ref, OHOS::sptr<PermissionCallback>> anyPermissionsCallback;

}  // namespace

napi_ref g_classBundleInstaller;

static OHOS::sptr<OHOS::AppExecFwk::IBundleMgr> GetBundleMgr()
{
    OHOS::sptr<OHOS::ISystemAbilityManager> systemAbilityManager =
        OHOS::SystemAbilityManagerClient::GetInstance().GetSystemAbilityManager();
    OHOS::sptr<OHOS::IRemoteObject> remoteObject =
        systemAbilityManager->GetSystemAbility(OHOS::BUNDLE_MGR_SERVICE_SYS_ABILITY_ID);
    return OHOS::iface_cast<IBundleMgr>(remoteObject);
}

static bool CheckIsSystemApp()
{
    int32_t uid = IPCSkeleton::GetCallingUid();
    if (uid >= OHOS::AppExecFwk::Constants::ROOT_UID && uid <= OHOS::AppExecFwk::Constants::MAX_SYS_UID) {
        return true;
    }
    return false;
}

static void ConvertApplicationInfo(napi_env env, napi_value objAppInfo, const ApplicationInfo &appInfo)
{
    napi_value nName;
    NAPI_CALL_RETURN_VOID(env, napi_create_string_utf8(env, appInfo.name.c_str(), NAPI_AUTO_LENGTH, &nName));
    NAPI_CALL_RETURN_VOID(env, napi_set_named_property(env, objAppInfo, "name", nName));
    HILOG_INFO("ConvertApplicationInfo name=%{public}s.", appInfo.name.c_str());

    napi_value nDescription;
    NAPI_CALL_RETURN_VOID(
        env, napi_create_string_utf8(env, appInfo.description.c_str(), NAPI_AUTO_LENGTH, &nDescription));
    NAPI_CALL_RETURN_VOID(env, napi_set_named_property(env, objAppInfo, "description", nDescription));

    napi_value nDescriptionId;
    NAPI_CALL_RETURN_VOID(env, napi_create_int32(env, appInfo.descriptionId, &nDescriptionId));
    NAPI_CALL_RETURN_VOID(env, napi_set_named_property(env, objAppInfo, "descriptionId", nDescriptionId));

    napi_value nIconPath;
    NAPI_CALL_RETURN_VOID(env, napi_create_string_utf8(env, appInfo.iconPath.c_str(), NAPI_AUTO_LENGTH, &nIconPath));
    NAPI_CALL_RETURN_VOID(env, napi_set_named_property(env, objAppInfo, "icon", nIconPath));

    napi_value nIconId;
    NAPI_CALL_RETURN_VOID(env, napi_create_int32(env, appInfo.iconId, &nIconId));
    NAPI_CALL_RETURN_VOID(env, napi_set_named_property(env, objAppInfo, "iconId", nIconId));

    napi_value nLabel;
    NAPI_CALL_RETURN_VOID(env, napi_create_string_utf8(env, appInfo.label.c_str(), NAPI_AUTO_LENGTH, &nLabel));
    NAPI_CALL_RETURN_VOID(env, napi_set_named_property(env, objAppInfo, "label", nLabel));

    napi_value nLabelId;
    NAPI_CALL_RETURN_VOID(env, napi_create_int32(env, appInfo.labelId, &nLabelId));
    NAPI_CALL_RETURN_VOID(env, napi_set_named_property(env, objAppInfo, "labelId", nLabelId));

    napi_value nIsSystemApp;
    NAPI_CALL_RETURN_VOID(env, napi_get_boolean(env, appInfo.isSystemApp, &nIsSystemApp));
    NAPI_CALL_RETURN_VOID(env, napi_set_named_property(env, objAppInfo, "systemApp", nIsSystemApp));

    napi_value nSupportedModes;
    NAPI_CALL_RETURN_VOID(env, napi_create_int32(env, appInfo.supportedModes, &nSupportedModes));
    NAPI_CALL_RETURN_VOID(env, napi_set_named_property(env, objAppInfo, "supportedModes", nSupportedModes));

    napi_value nProcess;
    NAPI_CALL_RETURN_VOID(env, napi_create_string_utf8(env, appInfo.process.c_str(), NAPI_AUTO_LENGTH, &nProcess));
    NAPI_CALL_RETURN_VOID(env, napi_set_named_property(env, objAppInfo, "process", nProcess));

    napi_value nSingleUser;
    NAPI_CALL_RETURN_VOID(env, napi_get_boolean(env, appInfo.singleUser, &nSingleUser));
    NAPI_CALL_RETURN_VOID(env, napi_set_named_property(env, objAppInfo, "singleUser", nSingleUser));

    if (CheckIsSystemApp()) {
        napi_value nEntryDir;
        NAPI_CALL_RETURN_VOID(
            env, napi_create_string_utf8(env, appInfo.entryDir.c_str(), NAPI_AUTO_LENGTH, &nEntryDir));
        NAPI_CALL_RETURN_VOID(env, napi_set_named_property(env, objAppInfo, "entryDir", nEntryDir));
    }

    napi_value nPermissions;
    NAPI_CALL_RETURN_VOID(env, napi_create_array(env, &nPermissions));
    for (size_t idx = 0; idx < appInfo.permissions.size(); idx++) {
        napi_value nPermission;
        NAPI_CALL_RETURN_VOID(
            env, napi_create_string_utf8(env, appInfo.permissions[idx].c_str(), NAPI_AUTO_LENGTH, &nPermission));
        NAPI_CALL_RETURN_VOID(env, napi_set_element(env, nPermissions, idx, nPermission));
    }
    NAPI_CALL_RETURN_VOID(env, napi_set_named_property(env, objAppInfo, "permissions", nPermissions));

    napi_value nModuleSourceDirs;
    NAPI_CALL_RETURN_VOID(env, napi_create_array(env, &nModuleSourceDirs));
    for (size_t idx = 0; idx < appInfo.moduleSourceDirs.size(); idx++) {
        napi_value nModuleSourceDir;
        NAPI_CALL_RETURN_VOID(env,
            napi_create_string_utf8(env, appInfo.moduleSourceDirs[idx].c_str(), NAPI_AUTO_LENGTH, &nModuleSourceDir));
        NAPI_CALL_RETURN_VOID(env, napi_set_element(env, nModuleSourceDirs, idx, nModuleSourceDir));
    }
    NAPI_CALL_RETURN_VOID(env, napi_set_named_property(env, objAppInfo, "moduleSourceDirs", nModuleSourceDirs));

    napi_value nModuleInfos;
    NAPI_CALL_RETURN_VOID(env, napi_create_array(env, &nModuleInfos));
    for (size_t idx = 0; idx < appInfo.moduleInfos.size(); idx++) {
        napi_value objModuleInfos;
        NAPI_CALL_RETURN_VOID(env, napi_create_object(env, &objModuleInfos));

        napi_value nModuleName;
        NAPI_CALL_RETURN_VOID(env,
            napi_create_string_utf8(env, appInfo.moduleInfos[idx].moduleName.c_str(), NAPI_AUTO_LENGTH, &nModuleName));
        NAPI_CALL_RETURN_VOID(env, napi_set_named_property(env, objModuleInfos, "moduleName", nModuleName));

        napi_value nModuleSourceDir;
        NAPI_CALL_RETURN_VOID(env,
            napi_create_string_utf8(
                env, appInfo.moduleInfos[idx].moduleSourceDir.c_str(), NAPI_AUTO_LENGTH, &nModuleSourceDir));
        NAPI_CALL_RETURN_VOID(env, napi_set_named_property(env, objModuleInfos, "moduleSourceDir", nModuleSourceDir));

        NAPI_CALL_RETURN_VOID(env, napi_set_element(env, nModuleInfos, idx, objModuleInfos));
    }
    NAPI_CALL_RETURN_VOID(env, napi_set_named_property(env, objAppInfo, "moduleInfos", nModuleInfos));

    napi_value nEnabled;
    NAPI_CALL_RETURN_VOID(env, napi_get_boolean(env, appInfo.enabled, &nEnabled));
    NAPI_CALL_RETURN_VOID(env, napi_set_named_property(env, objAppInfo, "enabled", nEnabled));

    napi_value nFlags;
    NAPI_CALL_RETURN_VOID(env, napi_create_int32(env, DEFAULT_INT32, &nFlags));
    NAPI_CALL_RETURN_VOID(env, napi_set_named_property(env, objAppInfo, "flags", nFlags));

    HILOG_INFO("ConvertApplicationInfo entryDir=%{public}s.", appInfo.entryDir.c_str());
}

static void ConvertCustomizeData(napi_env env, napi_value objCustomizeData, const CustomizeData &customizeData)
{
    napi_value nName;
    NAPI_CALL_RETURN_VOID(env, napi_create_string_utf8(env, customizeData.name.c_str(), NAPI_AUTO_LENGTH, &nName));
    NAPI_CALL_RETURN_VOID(env, napi_set_named_property(env, objCustomizeData, "name", nName));
    HILOG_INFO("ConvertCustomizeData name=%{public}s.", customizeData.name.c_str());
    napi_value nValue;
    NAPI_CALL_RETURN_VOID(env, napi_create_string_utf8(env, customizeData.value.c_str(), NAPI_AUTO_LENGTH, &nValue));
    NAPI_CALL_RETURN_VOID(env, napi_set_named_property(env, objCustomizeData, "value", nValue));
    HILOG_INFO("ConvertCustomizeData value=%{public}s.", customizeData.value.c_str());
    napi_value nExtra;
    NAPI_CALL_RETURN_VOID(env, napi_create_string_utf8(env, customizeData.extra.c_str(), NAPI_AUTO_LENGTH, &nExtra));
    NAPI_CALL_RETURN_VOID(env, napi_set_named_property(env, objCustomizeData, "extra", nExtra));
    HILOG_INFO("ConvertCustomizeData extra=%{public}s.", customizeData.extra.c_str());
}

static void ConvertParameters(napi_env env, napi_value objParameters, const Parameters &parameters)
{
    napi_value nDescription;
    NAPI_CALL_RETURN_VOID(
        env, napi_create_string_utf8(env, parameters.description.c_str(), NAPI_AUTO_LENGTH, &nDescription));
    NAPI_CALL_RETURN_VOID(env, napi_set_named_property(env, objParameters, "description", nDescription));
    HILOG_INFO("ConvertParameters parameters.description=%{public}s.", parameters.description.c_str());
    napi_value nName;
    NAPI_CALL_RETURN_VOID(env, napi_create_string_utf8(env, parameters.name.c_str(), NAPI_AUTO_LENGTH, &nName));
    NAPI_CALL_RETURN_VOID(env, napi_set_named_property(env, objParameters, "name", nName));
    HILOG_INFO("ConvertParameters parameters.name=%{public}s.", parameters.name.c_str());
    napi_value nType;
    NAPI_CALL_RETURN_VOID(env, napi_create_string_utf8(env, parameters.type.c_str(), NAPI_AUTO_LENGTH, &nType));
    NAPI_CALL_RETURN_VOID(env, napi_set_named_property(env, objParameters, "type", nType));
    HILOG_INFO("ConvertParameters parameters.type=%{public}s.", parameters.type.c_str());
}

static void ConvertResults(napi_env env, napi_value objResults, const Results &results)
{
    napi_value nDescription;
    NAPI_CALL_RETURN_VOID(
        env, napi_create_string_utf8(env, results.description.c_str(), NAPI_AUTO_LENGTH, &nDescription));
    NAPI_CALL_RETURN_VOID(env, napi_set_named_property(env, objResults, "description", nDescription));
    HILOG_INFO("ConvertResults results.description=%{public}s.", results.description.c_str());
    napi_value nName;
    NAPI_CALL_RETURN_VOID(env, napi_create_string_utf8(env, results.name.c_str(), NAPI_AUTO_LENGTH, &nName));
    NAPI_CALL_RETURN_VOID(env, napi_set_named_property(env, objResults, "name", nName));
    HILOG_INFO("ConvertResults results.name=%{public}s.", results.name.c_str());
    napi_value nType;
    NAPI_CALL_RETURN_VOID(env, napi_create_string_utf8(env, results.type.c_str(), NAPI_AUTO_LENGTH, &nType));
    NAPI_CALL_RETURN_VOID(env, napi_set_named_property(env, objResults, "type", nType));
    HILOG_INFO("ConvertResults results.type=%{public}s.", results.type.c_str());
}

static void ConvertMetaData(napi_env env, napi_value objMetaData, const MetaData &metaData)
{
    napi_value nCustomizeDatas;
    NAPI_CALL_RETURN_VOID(env, napi_create_array(env, &nCustomizeDatas));
    for (size_t idx = 0; idx < metaData.customizeData.size(); idx++) {
        napi_value nCustomizeData;
        NAPI_CALL_RETURN_VOID(env, napi_create_object(env, &nCustomizeData));
        ConvertCustomizeData(env, nCustomizeData, metaData.customizeData[idx]);
        NAPI_CALL_RETURN_VOID(env, napi_set_element(env, nCustomizeDatas, idx, nCustomizeData));
    }
    NAPI_CALL_RETURN_VOID(env, napi_set_named_property(env, objMetaData, "customizeDatas", nCustomizeDatas));

    napi_value nParameters;
    NAPI_CALL_RETURN_VOID(env, napi_create_array(env, &nParameters));
    for (size_t idx = 0; idx < metaData.parameters.size(); idx++) {
        napi_value nParameter;
        NAPI_CALL_RETURN_VOID(env, napi_create_object(env, &nParameter));
        ConvertParameters(env, nParameter, metaData.parameters[idx]);
        NAPI_CALL_RETURN_VOID(env, napi_set_element(env, nParameters, idx, nParameter));
    }
    NAPI_CALL_RETURN_VOID(env, napi_set_named_property(env, objMetaData, "parameters", nParameters));

    napi_value nResults;
    NAPI_CALL_RETURN_VOID(env, napi_create_array(env, &nResults));
    for (size_t idx = 0; idx < metaData.results.size(); idx++) {
        napi_value nResult;
        NAPI_CALL_RETURN_VOID(env, napi_create_object(env, &nResult));
        ConvertResults(env, nResult, metaData.results[idx]);
        NAPI_CALL_RETURN_VOID(env, napi_set_element(env, nResults, idx, nResult));
    }
    NAPI_CALL_RETURN_VOID(env, napi_set_named_property(env, objMetaData, "results", nResults));
}

static void ConvertAbilityInfo(napi_env env, napi_value objAbilityInfo, const AbilityInfo &abilityInfo)
{
    napi_value nName;
    NAPI_CALL_RETURN_VOID(env, napi_create_string_utf8(env, abilityInfo.name.c_str(), NAPI_AUTO_LENGTH, &nName));
    NAPI_CALL_RETURN_VOID(env, napi_set_named_property(env, objAbilityInfo, "name", nName));
    HILOG_INFO("ConvertAbilityInfo name=%{public}s.", abilityInfo.name.c_str());

    napi_value nLabel;
    NAPI_CALL_RETURN_VOID(env, napi_create_string_utf8(env, abilityInfo.label.c_str(), NAPI_AUTO_LENGTH, &nLabel));
    NAPI_CALL_RETURN_VOID(env, napi_set_named_property(env, objAbilityInfo, "label", nLabel));
    HILOG_INFO("ConvertAbilityInfo label=%{public}s.", abilityInfo.label.c_str());

    napi_value nDescription;
    NAPI_CALL_RETURN_VOID(
        env, napi_create_string_utf8(env, abilityInfo.description.c_str(), NAPI_AUTO_LENGTH, &nDescription));
    NAPI_CALL_RETURN_VOID(env, napi_set_named_property(env, objAbilityInfo, "description", nDescription));

    napi_value nIconPath;
    NAPI_CALL_RETURN_VOID(
        env, napi_create_string_utf8(env, abilityInfo.iconPath.c_str(), NAPI_AUTO_LENGTH, &nIconPath));
    NAPI_CALL_RETURN_VOID(env, napi_set_named_property(env, objAbilityInfo, "icon", nIconPath));

    napi_value nsrcPath;
    NAPI_CALL_RETURN_VOID(env, napi_create_string_utf8(env, abilityInfo.srcPath.c_str(), NAPI_AUTO_LENGTH, &nsrcPath));
    NAPI_CALL_RETURN_VOID(env, napi_set_named_property(env, objAbilityInfo, "srcPath", nsrcPath));

    napi_value nLaunguage;
    NAPI_CALL_RETURN_VOID(
        env, napi_create_string_utf8(env, abilityInfo.srcLanguage.c_str(), NAPI_AUTO_LENGTH, &nLaunguage));
    NAPI_CALL_RETURN_VOID(env, napi_set_named_property(env, objAbilityInfo, "srcLanguage", nLaunguage));

    napi_value nVisible;
    NAPI_CALL_RETURN_VOID(env, napi_get_boolean(env, abilityInfo.visible, &nVisible));
    NAPI_CALL_RETURN_VOID(env, napi_set_named_property(env, objAbilityInfo, "isVisible", nVisible));

    napi_value nPermissions;
    NAPI_CALL_RETURN_VOID(env, napi_create_array(env, &nPermissions));
    for (size_t idx = 0; idx < abilityInfo.permissions.size(); idx++) {
        napi_value nPermission;
        NAPI_CALL_RETURN_VOID(
            env, napi_create_string_utf8(env, abilityInfo.permissions[idx].c_str(), NAPI_AUTO_LENGTH, &nPermission));
        NAPI_CALL_RETURN_VOID(env, napi_set_element(env, nPermissions, idx, nPermission));
    }
    NAPI_CALL_RETURN_VOID(env, napi_set_named_property(env, objAbilityInfo, "permissions", nPermissions));

    napi_value nDeviceCapabilities;
    NAPI_CALL_RETURN_VOID(env, napi_create_array(env, &nDeviceCapabilities));
    for (size_t idx = 0; idx < abilityInfo.deviceCapabilities.size(); idx++) {
        napi_value nDeviceCapability;
        NAPI_CALL_RETURN_VOID(env,
            napi_create_string_utf8(
                env, abilityInfo.deviceCapabilities[idx].c_str(), NAPI_AUTO_LENGTH, &nDeviceCapability));
        NAPI_CALL_RETURN_VOID(env, napi_set_element(env, nDeviceCapabilities, idx, nDeviceCapability));
    }
    NAPI_CALL_RETURN_VOID(env, napi_set_named_property(env, objAbilityInfo, "deviceCapabilities", nDeviceCapabilities));

    napi_value nDeviceTypes;
    NAPI_CALL_RETURN_VOID(env, napi_create_array(env, &nDeviceTypes));
    for (size_t idx = 0; idx < abilityInfo.deviceTypes.size(); idx++) {
        napi_value nDeviceType;
        NAPI_CALL_RETURN_VOID(
            env, napi_create_string_utf8(env, abilityInfo.deviceTypes[idx].c_str(), NAPI_AUTO_LENGTH, &nDeviceType));
        NAPI_CALL_RETURN_VOID(env, napi_set_element(env, nDeviceTypes, idx, nDeviceType));
    }
    NAPI_CALL_RETURN_VOID(env, napi_set_named_property(env, objAbilityInfo, "deviceTypes", nDeviceTypes));

    napi_value nProcess;
    NAPI_CALL_RETURN_VOID(env, napi_create_string_utf8(env, abilityInfo.process.c_str(), NAPI_AUTO_LENGTH, &nProcess));
    NAPI_CALL_RETURN_VOID(env, napi_set_named_property(env, objAbilityInfo, "process", nProcess));

    napi_value nUri;
    NAPI_CALL_RETURN_VOID(env, napi_create_string_utf8(env, abilityInfo.uri.c_str(), NAPI_AUTO_LENGTH, &nUri));
    NAPI_CALL_RETURN_VOID(env, napi_set_named_property(env, objAbilityInfo, "uri", nUri));
    HILOG_INFO("ConvertAbilityInfo uri=%{public}s.", abilityInfo.uri.c_str());

    napi_value nBundleName;
    NAPI_CALL_RETURN_VOID(
        env, napi_create_string_utf8(env, abilityInfo.bundleName.c_str(), NAPI_AUTO_LENGTH, &nBundleName));
    NAPI_CALL_RETURN_VOID(env, napi_set_named_property(env, objAbilityInfo, "bundleName", nBundleName));
    HILOG_INFO("ConvertAbilityInfo bundleName=%{public}s.", abilityInfo.bundleName.c_str());

    napi_value nModuleName;
    NAPI_CALL_RETURN_VOID(
        env, napi_create_string_utf8(env, abilityInfo.moduleName.c_str(), NAPI_AUTO_LENGTH, &nModuleName));
    NAPI_CALL_RETURN_VOID(env, napi_set_named_property(env, objAbilityInfo, "moduleName", nModuleName));

    napi_value nAppInfo;
    NAPI_CALL_RETURN_VOID(env, napi_create_object(env, &nAppInfo));
    ConvertApplicationInfo(env, nAppInfo, abilityInfo.applicationInfo);
    NAPI_CALL_RETURN_VOID(env, napi_set_named_property(env, objAbilityInfo, "applicationInfo", nAppInfo));

    napi_value nType;
    NAPI_CALL_RETURN_VOID(env, napi_create_int32(env, static_cast<int32_t>(abilityInfo.type), &nType));
    NAPI_CALL_RETURN_VOID(env, napi_set_named_property(env, objAbilityInfo, "type", nType));

    napi_value nOrientation;
    NAPI_CALL_RETURN_VOID(env, napi_create_int32(env, static_cast<int32_t>(abilityInfo.orientation), &nOrientation));
    NAPI_CALL_RETURN_VOID(env, napi_set_named_property(env, objAbilityInfo, "orientation", nOrientation));

    napi_value nLaunchMode;
    NAPI_CALL_RETURN_VOID(env, napi_create_int32(env, static_cast<int32_t>(abilityInfo.launchMode), &nLaunchMode));
    NAPI_CALL_RETURN_VOID(env, napi_set_named_property(env, objAbilityInfo, "launchMode", nLaunchMode));

    napi_value nBackgroundModes;
    NAPI_CALL_RETURN_VOID(env, napi_create_int32(env, abilityInfo.backgroundModes, &nBackgroundModes));
    NAPI_CALL_RETURN_VOID(env, napi_set_named_property(env, objAbilityInfo, "backgroundModes", nBackgroundModes));

    napi_value nDescriptionId;
    NAPI_CALL_RETURN_VOID(env, napi_create_int32(env, DEFAULT_INT32, &nDescriptionId));
    NAPI_CALL_RETURN_VOID(env, napi_set_named_property(env, objAbilityInfo, "descriptionId", nDescriptionId));

    napi_value nFormEnabled;
    NAPI_CALL_RETURN_VOID(env, napi_get_boolean(env, false, &nFormEnabled));
    NAPI_CALL_RETURN_VOID(env, napi_set_named_property(env, objAbilityInfo, "formEnabled", nFormEnabled));

    napi_value nIconId;
    NAPI_CALL_RETURN_VOID(env, napi_create_int32(env, DEFAULT_INT32, &nIconId));
    NAPI_CALL_RETURN_VOID(env, napi_set_named_property(env, objAbilityInfo, "iconId", nIconId));

    napi_value nLabelId;
    NAPI_CALL_RETURN_VOID(env, napi_create_int32(env, DEFAULT_INT32, &nLabelId));
    NAPI_CALL_RETURN_VOID(env, napi_set_named_property(env, objAbilityInfo, "labelId", nLabelId));

    napi_value nFormEntity;
    NAPI_CALL_RETURN_VOID(env, napi_create_int32(env, abilityInfo.formEntity, &nFormEntity));
    NAPI_CALL_RETURN_VOID(env, napi_set_named_property(env, objAbilityInfo, "formEntity", nFormEntity));

    napi_value nMinFormHeight;
    NAPI_CALL_RETURN_VOID(env, napi_create_int32(env, abilityInfo.minFormHeight, &nMinFormHeight));
    NAPI_CALL_RETURN_VOID(env, napi_set_named_property(env, objAbilityInfo, "minFormHeight", nMinFormHeight));

    napi_value nMinFormWidth;
    NAPI_CALL_RETURN_VOID(env, napi_create_int32(env, abilityInfo.minFormWidth, &nMinFormWidth));
    NAPI_CALL_RETURN_VOID(env, napi_set_named_property(env, objAbilityInfo, "minFormWidth", nMinFormWidth));

    napi_value nDefaultFormHeight;
    NAPI_CALL_RETURN_VOID(env, napi_create_int32(env, abilityInfo.defaultFormHeight, &nDefaultFormHeight));
    NAPI_CALL_RETURN_VOID(env, napi_set_named_property(env, objAbilityInfo, "defaultFormHeight", nDefaultFormHeight));

    napi_value nDefaultFormWidth;
    NAPI_CALL_RETURN_VOID(env, napi_create_int32(env, abilityInfo.defaultFormWidth, &nDefaultFormWidth));
    NAPI_CALL_RETURN_VOID(env, napi_set_named_property(env, objAbilityInfo, "defaultFormWidth", nDefaultFormWidth));

    napi_value nSubType;
    NAPI_CALL_RETURN_VOID(env, napi_create_int32(env, DEFAULT_INT32, &nSubType));
    NAPI_CALL_RETURN_VOID(env, napi_set_named_property(env, objAbilityInfo, "subType", nSubType));

    napi_value nReadPermission;
    NAPI_CALL_RETURN_VOID(env, napi_create_string_utf8(env, std::string().c_str(), NAPI_AUTO_LENGTH, &nReadPermission));
    NAPI_CALL_RETURN_VOID(env, napi_set_named_property(env, objAbilityInfo, "readPermission", nReadPermission));

    napi_value nWritePermission;
    NAPI_CALL_RETURN_VOID(
        env, napi_create_string_utf8(env, std::string().c_str(), NAPI_AUTO_LENGTH, &nWritePermission));
    NAPI_CALL_RETURN_VOID(env, napi_set_named_property(env, objAbilityInfo, "writePermission", nWritePermission));

    napi_value nTargetAbility;
    NAPI_CALL_RETURN_VOID(env, napi_create_string_utf8(env, std::string().c_str(), NAPI_AUTO_LENGTH, &nTargetAbility));
    NAPI_CALL_RETURN_VOID(env, napi_set_named_property(env, objAbilityInfo, "targetAbility", nTargetAbility));

    napi_value nTheme;
    NAPI_CALL_RETURN_VOID(env, napi_create_string_utf8(env, std::string().c_str(), NAPI_AUTO_LENGTH, &nTheme));
    NAPI_CALL_RETURN_VOID(env, napi_set_named_property(env, objAbilityInfo, "theme", nTheme));
    HILOG_INFO("nTheme=%{public}s.", abilityInfo.theme.c_str());

    napi_value nMetaData;
    NAPI_CALL_RETURN_VOID(env, napi_create_object(env, &nMetaData));
    ConvertMetaData(env, nMetaData, abilityInfo.metaData);
    NAPI_CALL_RETURN_VOID(env, napi_set_named_property(env, objAbilityInfo, "metaData", nMetaData));
}

static void ProcessAbilityInfos(
    napi_env env, napi_value result, const std::vector<OHOS::AppExecFwk::AbilityInfo> abilityInfos)
{
    if (abilityInfos.size() > 0) {
        HILOG_INFO("-----abilityInfos is not null-----");
        size_t index = 0;
        for (const auto &item : abilityInfos) {
            HILOG_INFO("name{%s} ", item.name.c_str());
            napi_value objAbilityInfo = nullptr;
            napi_create_object(env, &objAbilityInfo);
            ConvertAbilityInfo(env, objAbilityInfo, item);
            napi_set_element(env, result, index, objAbilityInfo);
            index++;
        }
    } else {
        HILOG_INFO("-----abilityInfos is null-----");
    }
}

static void ConvertHapModuleInfo(napi_env env, napi_value objHapModuleInfo, const HapModuleInfo &hapModuleInfo)
{
    napi_value nName;
    NAPI_CALL_RETURN_VOID(env, napi_create_string_utf8(env, hapModuleInfo.name.c_str(), NAPI_AUTO_LENGTH, &nName));
    NAPI_CALL_RETURN_VOID(env, napi_set_named_property(env, objHapModuleInfo, "name", nName));
    HILOG_INFO("ConvertHapModuleInfo name=%{public}s.", hapModuleInfo.name.c_str());

    napi_value nModuleName;
    NAPI_CALL_RETURN_VOID(
        env, napi_create_string_utf8(env, hapModuleInfo.moduleName.c_str(), NAPI_AUTO_LENGTH, &nModuleName));
    NAPI_CALL_RETURN_VOID(env, napi_set_named_property(env, objHapModuleInfo, "moduleName", nModuleName));
    HILOG_INFO("ConvertHapModuleInfo moduleName=%{public}s.", hapModuleInfo.moduleName.c_str());

    napi_value nDescription;
    NAPI_CALL_RETURN_VOID(
        env, napi_create_string_utf8(env, hapModuleInfo.description.c_str(), NAPI_AUTO_LENGTH, &nDescription));
    NAPI_CALL_RETURN_VOID(env, napi_set_named_property(env, objHapModuleInfo, "description", nDescription));
    HILOG_INFO("ConvertHapModuleInfo description=%{public}s.", hapModuleInfo.description.c_str());

    napi_value ndescriptionId;
    NAPI_CALL_RETURN_VOID(env, napi_create_int32(env, 0, &ndescriptionId));
    NAPI_CALL_RETURN_VOID(env, napi_set_named_property(env, objHapModuleInfo, "descriptionId", ndescriptionId));

    napi_value nIconPath;
    NAPI_CALL_RETURN_VOID(
        env, napi_create_string_utf8(env, hapModuleInfo.iconPath.c_str(), NAPI_AUTO_LENGTH, &nIconPath));
    NAPI_CALL_RETURN_VOID(env, napi_set_named_property(env, objHapModuleInfo, "iconPath", nIconPath));
    HILOG_INFO("ConvertHapModuleInfo iconPath=%{public}s.", hapModuleInfo.iconPath.c_str());

    napi_value nIcon;
    std::string theIcon = "";
    NAPI_CALL_RETURN_VOID(env, napi_create_string_utf8(env, theIcon.c_str(), NAPI_AUTO_LENGTH, &nIcon));
    NAPI_CALL_RETURN_VOID(env, napi_set_named_property(env, objHapModuleInfo, "icon", nIcon));

    napi_value nLabel;
    NAPI_CALL_RETURN_VOID(env, napi_create_string_utf8(env, hapModuleInfo.label.c_str(), NAPI_AUTO_LENGTH, &nLabel));
    NAPI_CALL_RETURN_VOID(env, napi_set_named_property(env, objHapModuleInfo, "label", nLabel));
    HILOG_INFO("ConvertHapModuleInfo label=%{public}s.", hapModuleInfo.label.c_str());

    napi_value nLabelId;
    NAPI_CALL_RETURN_VOID(env, napi_create_int32(env, 0, &nLabelId));
    NAPI_CALL_RETURN_VOID(env, napi_set_named_property(env, objHapModuleInfo, "labelId", nLabelId));

    napi_value nIconId;
    NAPI_CALL_RETURN_VOID(env, napi_create_int32(env, 0, &nIconId));
    NAPI_CALL_RETURN_VOID(env, napi_set_named_property(env, objHapModuleInfo, "iconId", nIconId));

    napi_value nBackgroundImg;
    NAPI_CALL_RETURN_VOID(
        env, napi_create_string_utf8(env, hapModuleInfo.backgroundImg.c_str(), NAPI_AUTO_LENGTH, &nBackgroundImg));
    NAPI_CALL_RETURN_VOID(env, napi_set_named_property(env, objHapModuleInfo, "backgroundImg", nBackgroundImg));
    HILOG_INFO("ConvertHapModuleInfo backgroundImg=%{public}s.", hapModuleInfo.backgroundImg.c_str());

    napi_value nSupportedModes;
    NAPI_CALL_RETURN_VOID(env, napi_create_int32(env, hapModuleInfo.supportedModes, &nSupportedModes));
    NAPI_CALL_RETURN_VOID(env, napi_set_named_property(env, objHapModuleInfo, "supportedModes", nSupportedModes));

    napi_value nReqCapabilities;
    NAPI_CALL_RETURN_VOID(env, napi_create_array(env, &nReqCapabilities));
    for (size_t idx = 0; idx < hapModuleInfo.reqCapabilities.size(); idx++) {
        napi_value nReqCapabilitie;
        NAPI_CALL_RETURN_VOID(env,
            napi_create_string_utf8(
                env, hapModuleInfo.reqCapabilities[idx].c_str(), NAPI_AUTO_LENGTH, &nReqCapabilitie));
        NAPI_CALL_RETURN_VOID(env, napi_set_element(env, nReqCapabilities, idx, nReqCapabilitie));
    }
    NAPI_CALL_RETURN_VOID(env, napi_set_named_property(env, objHapModuleInfo, "reqCapabilities", nReqCapabilities));

    napi_value nDeviceTypes;
    NAPI_CALL_RETURN_VOID(env, napi_create_array(env, &nDeviceTypes));
    for (size_t idx = 0; idx < hapModuleInfo.deviceTypes.size(); idx++) {
        napi_value nDeviceType;
        NAPI_CALL_RETURN_VOID(
            env, napi_create_string_utf8(env, hapModuleInfo.deviceTypes[idx].c_str(), NAPI_AUTO_LENGTH, &nDeviceType));
        NAPI_CALL_RETURN_VOID(env, napi_set_element(env, nDeviceTypes, idx, nDeviceType));
    }
    NAPI_CALL_RETURN_VOID(env, napi_set_named_property(env, objHapModuleInfo, "deviceTypes", nDeviceTypes));

    napi_value nAbilityInfos;
    NAPI_CALL_RETURN_VOID(env, napi_create_array(env, &nAbilityInfos));
    for (size_t idx = 0; idx < hapModuleInfo.abilityInfos.size(); idx++) {
        napi_value objAbilityInfo;
        NAPI_CALL_RETURN_VOID(env, napi_create_object(env, &objAbilityInfo));
        ConvertAbilityInfo(env, objAbilityInfo, hapModuleInfo.abilityInfos[idx]);
        NAPI_CALL_RETURN_VOID(env, napi_set_element(env, nAbilityInfos, idx, objAbilityInfo));
    }
    NAPI_CALL_RETURN_VOID(env, napi_set_named_property(env, objHapModuleInfo, "abilityInfo", nAbilityInfos));

    napi_value nColorMode;
    NAPI_CALL_RETURN_VOID(env, napi_create_int32(env, static_cast<int32_t>(hapModuleInfo.colorMode), &nColorMode));
    NAPI_CALL_RETURN_VOID(env, napi_set_named_property(env, objHapModuleInfo, "colorMode", nColorMode));

    napi_value nMainAbilityName;
    NAPI_CALL_RETURN_VOID(
        env, napi_create_string_utf8(env, hapModuleInfo.mainAbility.c_str(), NAPI_AUTO_LENGTH, &nMainAbilityName));
    NAPI_CALL_RETURN_VOID(env, napi_set_named_property(env, objHapModuleInfo, "mainAbilityName", nMainAbilityName));
    HILOG_INFO("ConvertHapModuleInfo mainAbilityName=%{public}s.", hapModuleInfo.mainAbility.c_str());

    napi_value nInstallationFree;
    NAPI_CALL_RETURN_VOID(env, napi_get_boolean(env, false, &nInstallationFree));
    NAPI_CALL_RETURN_VOID(env, napi_set_named_property(env, objHapModuleInfo, "installationFree", nInstallationFree));
}

static void ConvertBundleInfo(napi_env env, napi_value objBundleInfo, const BundleInfo &bundleInfo)
{
    HILOG_INFO("ConvertBundleInfo ");
    napi_value nName;
    NAPI_CALL_RETURN_VOID(env, napi_create_string_utf8(env, bundleInfo.name.c_str(), NAPI_AUTO_LENGTH, &nName));
    NAPI_CALL_RETURN_VOID(env, napi_set_named_property(env, objBundleInfo, "name", nName));

    napi_value nSingleUser;
    NAPI_CALL_RETURN_VOID(env, napi_get_boolean(env, bundleInfo.singleUser, &nSingleUser));
    NAPI_CALL_RETURN_VOID(env, napi_set_named_property(env, objBundleInfo, "singleUser", nSingleUser));
    HILOG_INFO("ConvertApplicationInfo singleUser=%{public}d.", bundleInfo.singleUser);

    napi_value nIsKeepAlive;
    NAPI_CALL_RETURN_VOID(env, napi_get_boolean(env, bundleInfo.isKeepAlive, &nIsKeepAlive));
    NAPI_CALL_RETURN_VOID(env, napi_set_named_property(env, objBundleInfo, "isKeepAlive", nIsKeepAlive));
    HILOG_INFO("ConvertApplicationInfo isKeepAlive=%{public}d.", bundleInfo.isKeepAlive);

    napi_value nVendor;
    NAPI_CALL_RETURN_VOID(env, napi_create_string_utf8(env, bundleInfo.vendor.c_str(), NAPI_AUTO_LENGTH, &nVendor));
    NAPI_CALL_RETURN_VOID(env, napi_set_named_property(env, objBundleInfo, "vendor", nVendor));

    napi_value nVersionCode;
    NAPI_CALL_RETURN_VOID(env, napi_create_uint32(env, bundleInfo.versionCode, &nVersionCode));
    NAPI_CALL_RETURN_VOID(env, napi_set_named_property(env, objBundleInfo, "versionCode", nVersionCode));

    napi_value nVersionName;
    NAPI_CALL_RETURN_VOID(
        env, napi_create_string_utf8(env, bundleInfo.versionName.c_str(), NAPI_AUTO_LENGTH, &nVersionName));
    NAPI_CALL_RETURN_VOID(env, napi_set_named_property(env, objBundleInfo, "versionName", nVersionName));

    napi_value nCpuAbi;
    NAPI_CALL_RETURN_VOID(env, napi_create_string_utf8(env, bundleInfo.cpuAbi.c_str(), NAPI_AUTO_LENGTH, &nCpuAbi));
    NAPI_CALL_RETURN_VOID(env, napi_set_named_property(env, objBundleInfo, "cpuAbi", nCpuAbi));

    napi_value nAppId;
    NAPI_CALL_RETURN_VOID(env, napi_create_string_utf8(env, bundleInfo.appId.c_str(), NAPI_AUTO_LENGTH, &nAppId));
    NAPI_CALL_RETURN_VOID(env, napi_set_named_property(env, objBundleInfo, "appId", nAppId));

    napi_value nEntryModuleName;
    NAPI_CALL_RETURN_VOID(
        env, napi_create_string_utf8(env, bundleInfo.entryModuleName.c_str(), NAPI_AUTO_LENGTH, &nEntryModuleName));
    NAPI_CALL_RETURN_VOID(env, napi_set_named_property(env, objBundleInfo, "entryModuleName", nEntryModuleName));

    napi_value nCompatibleVersion;
    NAPI_CALL_RETURN_VOID(env, napi_create_int32(env, bundleInfo.compatibleVersion, &nCompatibleVersion));
    NAPI_CALL_RETURN_VOID(env, napi_set_named_property(env, objBundleInfo, "compatibleVersion", nCompatibleVersion));

    napi_value nTargetVersion;
    NAPI_CALL_RETURN_VOID(env, napi_create_int32(env, bundleInfo.targetVersion, &nTargetVersion));
    NAPI_CALL_RETURN_VOID(env, napi_set_named_property(env, objBundleInfo, "targetVersion", nTargetVersion));

    napi_value nUid;
    NAPI_CALL_RETURN_VOID(env, napi_create_int32(env, bundleInfo.uid, &nUid));
    NAPI_CALL_RETURN_VOID(env, napi_set_named_property(env, objBundleInfo, "uid", nUid));

    napi_value nInstallTime;
    NAPI_CALL_RETURN_VOID(env, napi_create_int64(env, bundleInfo.installTime, &nInstallTime));
    NAPI_CALL_RETURN_VOID(env, napi_set_named_property(env, objBundleInfo, "installTime", nInstallTime));

    napi_value nUpdateTime;
    NAPI_CALL_RETURN_VOID(env, napi_create_int64(env, bundleInfo.updateTime, &nUpdateTime));
    NAPI_CALL_RETURN_VOID(env, napi_set_named_property(env, objBundleInfo, "updateTime", nUpdateTime));

    napi_value nAppInfo;
    NAPI_CALL_RETURN_VOID(env, napi_create_object(env, &nAppInfo));
    ConvertApplicationInfo(env, nAppInfo, bundleInfo.applicationInfo);
    NAPI_CALL_RETURN_VOID(env, napi_set_named_property(env, objBundleInfo, "appInfo", nAppInfo));

    napi_value nAbilityInfos;
    NAPI_CALL_RETURN_VOID(env, napi_create_array(env, &nAbilityInfos));
    for (size_t idx = 0; idx < bundleInfo.abilityInfos.size(); idx++) {
        napi_value objAbilityInfo;
        NAPI_CALL_RETURN_VOID(env, napi_create_object(env, &objAbilityInfo));
        ConvertAbilityInfo(env, objAbilityInfo, bundleInfo.abilityInfos[idx]);
        NAPI_CALL_RETURN_VOID(env, napi_set_element(env, nAbilityInfos, idx, objAbilityInfo));
    }
    NAPI_CALL_RETURN_VOID(env, napi_set_named_property(env, objBundleInfo, "abilityInfo", nAbilityInfos));

    napi_value nAbilityInfoss;
    NAPI_CALL_RETURN_VOID(env, napi_create_array(env, &nAbilityInfoss));
    for (size_t idx = 0; idx < bundleInfo.abilityInfos.size(); idx++) {
        napi_value objAbilityInfo;
        NAPI_CALL_RETURN_VOID(env, napi_create_object(env, &objAbilityInfo));
        ConvertAbilityInfo(env, objAbilityInfo, bundleInfo.abilityInfos[idx]);
        NAPI_CALL_RETURN_VOID(env, napi_set_element(env, nAbilityInfoss, idx, objAbilityInfo));
    }
    NAPI_CALL_RETURN_VOID(env, napi_set_named_property(env, objBundleInfo, "abilityInfos", nAbilityInfoss));

    napi_value nHapModuleInfos;
    NAPI_CALL_RETURN_VOID(env, napi_create_array(env, &nHapModuleInfos));
    for (size_t idx = 0; idx < bundleInfo.hapModuleInfos.size(); idx++) {
        napi_value objHapModuleInfo;
        NAPI_CALL_RETURN_VOID(env, napi_create_object(env, &objHapModuleInfo));
        ConvertHapModuleInfo(env, objHapModuleInfo, bundleInfo.hapModuleInfos[idx]);
        NAPI_CALL_RETURN_VOID(env, napi_set_element(env, nHapModuleInfos, idx, objHapModuleInfo));
    }
    NAPI_CALL_RETURN_VOID(env, napi_set_named_property(env, objBundleInfo, "hapModuleInfo", nHapModuleInfos));

    napi_value nReqPermissions;
    NAPI_CALL_RETURN_VOID(env, napi_create_array(env, &nReqPermissions));
    for (size_t idx = 0; idx < bundleInfo.reqPermissions.size(); idx++) {
        napi_value nReqPermission;
        NAPI_CALL_RETURN_VOID(env,
            napi_create_string_utf8(env, bundleInfo.reqPermissions[idx].c_str(), NAPI_AUTO_LENGTH, &nReqPermission));
        NAPI_CALL_RETURN_VOID(env, napi_set_element(env, nReqPermissions, idx, nReqPermission));
    }
    NAPI_CALL_RETURN_VOID(env, napi_set_named_property(env, objBundleInfo, "reqPermissions", nReqPermissions));

    napi_value nIsCompressNativeLibs;
    NAPI_CALL_RETURN_VOID(env, napi_get_boolean(env, false, &nIsCompressNativeLibs));
    NAPI_CALL_RETURN_VOID(
        env, napi_set_named_property(env, objBundleInfo, "isCompressNativeLibs", nIsCompressNativeLibs));

    napi_value nIsSilentInstallation;
    NAPI_CALL_RETURN_VOID(
        env, napi_create_string_utf8(env, std::string().c_str(), NAPI_AUTO_LENGTH, &nIsSilentInstallation));
    NAPI_CALL_RETURN_VOID(
        env, napi_set_named_property(env, objBundleInfo, "isSilentInstallation", nIsSilentInstallation));

    napi_value nType;
    NAPI_CALL_RETURN_VOID(env, napi_create_string_utf8(env, std::string().c_str(), NAPI_AUTO_LENGTH, &nType));
    NAPI_CALL_RETURN_VOID(env, napi_set_named_property(env, objBundleInfo, "type", nType));

    napi_value nReqPermissionDetails;
    NAPI_CALL_RETURN_VOID(env, napi_create_array(env, &nReqPermissionDetails));
    NAPI_CALL_RETURN_VOID(
        env, napi_set_named_property(env, objBundleInfo, "reqPermissionDetails", nReqPermissionDetails));

    napi_value nMinCompatibleVersionCode;
    NAPI_CALL_RETURN_VOID(env, napi_create_int32(env, DEFAULT_INT32, &nMinCompatibleVersionCode));
    NAPI_CALL_RETURN_VOID(
        env, napi_set_named_property(env, objBundleInfo, "minCompatibleVersionCode", nMinCompatibleVersionCode));

    napi_value nEntryInstallationFree;
    NAPI_CALL_RETURN_VOID(env, napi_get_boolean(env, false, &nEntryInstallationFree));
    NAPI_CALL_RETURN_VOID(
        env, napi_set_named_property(env, objBundleInfo, "entryInstallationFree", nEntryInstallationFree));
}

static void ConvertFormCustomizeData(napi_env env, napi_value objformInfo, const FormCustomizeData &customizeData)
{
    napi_value nName;
    NAPI_CALL_RETURN_VOID(env, napi_create_string_utf8(env, customizeData.name.c_str(), NAPI_AUTO_LENGTH, &nName));
    NAPI_CALL_RETURN_VOID(env, napi_set_named_property(env, objformInfo, "name", nName));
    napi_value nValue;
    NAPI_CALL_RETURN_VOID(env, napi_create_string_utf8(env, customizeData.value.c_str(), NAPI_AUTO_LENGTH, &nValue));
    NAPI_CALL_RETURN_VOID(env, napi_set_named_property(env, objformInfo, "value", nValue));
}

static void ConvertFormWindow(napi_env env, napi_value objWindowInfo, const FormWindow &formWindow)
{
    napi_value nDesignWidth;
    NAPI_CALL_RETURN_VOID(env, napi_create_uint32(env, formWindow.designWidth, &nDesignWidth));
    NAPI_CALL_RETURN_VOID(env, napi_set_named_property(env, objWindowInfo, "designWidth", nDesignWidth));
    napi_value nAutoDesignWidth;
    NAPI_CALL_RETURN_VOID(env, napi_get_boolean(env, formWindow.autoDesignWidth, &nAutoDesignWidth));
    NAPI_CALL_RETURN_VOID(env, napi_set_named_property(env, objWindowInfo, "autoDesignWidth", nAutoDesignWidth));
}

static void ConvertFormInfo(napi_env env, napi_value objformInfo, const FormInfo &formInfo)
{
    napi_value nName;
    NAPI_CALL_RETURN_VOID(env, napi_create_string_utf8(env, formInfo.name.c_str(), NAPI_AUTO_LENGTH, &nName));
    NAPI_CALL_RETURN_VOID(env, napi_set_named_property(env, objformInfo, "name", nName));
    HILOG_INFO("ConvertFormInfo name=%{public}s.", formInfo.name.c_str());

    napi_value nDescription;
    NAPI_CALL_RETURN_VOID(
        env, napi_create_string_utf8(env, formInfo.description.c_str(), NAPI_AUTO_LENGTH, &nDescription));
    NAPI_CALL_RETURN_VOID(env, napi_set_named_property(env, objformInfo, "description", nDescription));

    napi_value nDescriptionId;
    NAPI_CALL_RETURN_VOID(env, napi_create_int32(env, formInfo.descriptionId, &nDescriptionId));
    NAPI_CALL_RETURN_VOID(env, napi_set_named_property(env, objformInfo, "descriptionId", nDescriptionId));

    napi_value nBundleName;
    NAPI_CALL_RETURN_VOID(
        env, napi_create_string_utf8(env, formInfo.bundleName.c_str(), NAPI_AUTO_LENGTH, &nBundleName));
    NAPI_CALL_RETURN_VOID(env, napi_set_named_property(env, objformInfo, "bundleName", nBundleName));

    napi_value nModuleName;
    NAPI_CALL_RETURN_VOID(
        env, napi_create_string_utf8(env, formInfo.moduleName.c_str(), NAPI_AUTO_LENGTH, &nModuleName));
    NAPI_CALL_RETURN_VOID(env, napi_set_named_property(env, objformInfo, "moduleName", nModuleName));

    napi_value nAbilityName;
    NAPI_CALL_RETURN_VOID(
        env, napi_create_string_utf8(env, formInfo.abilityName.c_str(), NAPI_AUTO_LENGTH, &nAbilityName));
    NAPI_CALL_RETURN_VOID(env, napi_set_named_property(env, objformInfo, "abilityName", nAbilityName));

    napi_value nRelatedBundleName;
    NAPI_CALL_RETURN_VOID(
        env, napi_create_string_utf8(env, formInfo.relatedBundleName.c_str(), NAPI_AUTO_LENGTH, &nRelatedBundleName));
    NAPI_CALL_RETURN_VOID(env, napi_set_named_property(env, objformInfo, "relatedBundleName", nRelatedBundleName));

    napi_value nDefaultFlag;
    NAPI_CALL_RETURN_VOID(env, napi_get_boolean(env, formInfo.defaultFlag, &nDefaultFlag));
    NAPI_CALL_RETURN_VOID(env, napi_set_named_property(env, objformInfo, "defaultFlag", nDefaultFlag));

    napi_value nFormVisibleNotify;
    NAPI_CALL_RETURN_VOID(env, napi_get_boolean(env, formInfo.formVisibleNotify, &nFormVisibleNotify));
    NAPI_CALL_RETURN_VOID(env, napi_set_named_property(env, objformInfo, "formVisibleNotify", nFormVisibleNotify));

    napi_value nFormConfigAbility;
    NAPI_CALL_RETURN_VOID(
        env, napi_create_string_utf8(env, formInfo.formConfigAbility.c_str(), NAPI_AUTO_LENGTH, &nFormConfigAbility));
    NAPI_CALL_RETURN_VOID(env, napi_set_named_property(env, objformInfo, "formConfigAbility", nFormConfigAbility));

    napi_value nType;
    NAPI_CALL_RETURN_VOID(env, napi_create_int32(env, static_cast<int32_t>(formInfo.type), &nType));
    NAPI_CALL_RETURN_VOID(env, napi_set_named_property(env, objformInfo, "type", nType));

    napi_value nColorMode;
    NAPI_CALL_RETURN_VOID(env, napi_create_int32(env, static_cast<int32_t>(formInfo.colorMode), &nColorMode));
    NAPI_CALL_RETURN_VOID(env, napi_set_named_property(env, objformInfo, "colorMode", nColorMode));

    napi_value nSupportDimensions;
    NAPI_CALL_RETURN_VOID(env, napi_create_array(env, &nSupportDimensions));
    for (size_t idx = 0; idx < formInfo.supportDimensions.size(); idx++) {
        napi_value nSupportDimension;
        NAPI_CALL_RETURN_VOID(env, napi_create_int32(env, formInfo.supportDimensions[idx], &nSupportDimension));
        NAPI_CALL_RETURN_VOID(env, napi_set_element(env, nSupportDimensions, idx, nSupportDimension));
    }
    NAPI_CALL_RETURN_VOID(env, napi_set_named_property(env, objformInfo, "supportDimensions", nSupportDimensions));

    napi_value nDefaultDimension;
    NAPI_CALL_RETURN_VOID(env, napi_create_int32(env, formInfo.defaultDimension, &nDefaultDimension));
    NAPI_CALL_RETURN_VOID(env, napi_set_named_property(env, objformInfo, "defaultDimension", nDefaultDimension));

    napi_value nJsComponentName;
    NAPI_CALL_RETURN_VOID(
        env, napi_create_string_utf8(env, formInfo.jsComponentName.c_str(), NAPI_AUTO_LENGTH, &nJsComponentName));
    NAPI_CALL_RETURN_VOID(env, napi_set_named_property(env, objformInfo, "jsComponentName", nJsComponentName));

    napi_value nUpdateDuration;
    NAPI_CALL_RETURN_VOID(env, napi_create_int32(env, formInfo.updateDuration, &nUpdateDuration));
    NAPI_CALL_RETURN_VOID(env, napi_set_named_property(env, objformInfo, "updateDuration", nUpdateDuration));

    napi_value nCustomizeDatas;
    NAPI_CALL_RETURN_VOID(env, napi_create_array(env, &nCustomizeDatas));
    for (size_t idx = 0; idx < formInfo.customizeDatas.size(); idx++) {
        napi_value nCustomizeData;
        NAPI_CALL_RETURN_VOID(env, napi_create_object(env, &nCustomizeData));
        ConvertFormCustomizeData(env, nCustomizeData, formInfo.customizeDatas[idx]);
        NAPI_CALL_RETURN_VOID(env, napi_set_element(env, nCustomizeDatas, idx, nCustomizeData));
    }
    NAPI_CALL_RETURN_VOID(env, napi_set_named_property(env, objformInfo, "customizeDatas", nCustomizeDatas));

    napi_value nSrc;
    NAPI_CALL_RETURN_VOID(env, napi_create_string_utf8(env, formInfo.src.c_str(), NAPI_AUTO_LENGTH, &nSrc));
    NAPI_CALL_RETURN_VOID(env, napi_set_named_property(env, objformInfo, "src", nSrc));
    HILOG_INFO("ConvertFormInfo src=%{public}s.", formInfo.src.c_str());

    napi_value nWindow;
    NAPI_CALL_RETURN_VOID(env, napi_create_object(env, &nWindow));
    ConvertFormWindow(env, nWindow, formInfo.window);
    HILOG_INFO("ConvertFormInfo window.designWidth=%{public}d.", formInfo.window.designWidth);
    HILOG_INFO("ConvertFormInfo window.autoDesignWidth=%{public}d.", formInfo.window.autoDesignWidth);
    NAPI_CALL_RETURN_VOID(env, napi_set_named_property(env, objformInfo, "window", nWindow));
}

static void ConvertShortcutIntent(napi_env env, napi_value objShortcutInfo, const ShortcutIntent &shortcutIntent)
{
    napi_value nTargetBundle;
    NAPI_CALL_RETURN_VOID(
        env, napi_create_string_utf8(env, shortcutIntent.targetBundle.c_str(), NAPI_AUTO_LENGTH, &nTargetBundle));
    NAPI_CALL_RETURN_VOID(env, napi_set_named_property(env, objShortcutInfo, "targetBundle", nTargetBundle));
    napi_value nTargetClass;
    NAPI_CALL_RETURN_VOID(
        env, napi_create_string_utf8(env, shortcutIntent.targetClass.c_str(), NAPI_AUTO_LENGTH, &nTargetClass));
    NAPI_CALL_RETURN_VOID(env, napi_set_named_property(env, objShortcutInfo, "targetClass", nTargetClass));
}

static void ConvertShortcutInfos(napi_env env, napi_value objShortcutInfo, const ShortcutInfo &shortcutInfo)
{
    napi_value nId;
    NAPI_CALL_RETURN_VOID(env, napi_create_string_utf8(env, shortcutInfo.id.c_str(), NAPI_AUTO_LENGTH, &nId));
    NAPI_CALL_RETURN_VOID(env, napi_set_named_property(env, objShortcutInfo, "id", nId));
    HILOG_INFO("ConvertShortcutInfos Id=%{public}s.", shortcutInfo.id.c_str());

    napi_value nBundleName;
    NAPI_CALL_RETURN_VOID(
        env, napi_create_string_utf8(env, shortcutInfo.bundleName.c_str(), NAPI_AUTO_LENGTH, &nBundleName));
    NAPI_CALL_RETURN_VOID(env, napi_set_named_property(env, objShortcutInfo, "bundleName", nBundleName));

    napi_value nHostAbility;
    NAPI_CALL_RETURN_VOID(
        env, napi_create_string_utf8(env, shortcutInfo.hostAbility.c_str(), NAPI_AUTO_LENGTH, &nHostAbility));
    NAPI_CALL_RETURN_VOID(env, napi_set_named_property(env, objShortcutInfo, "hostAbility", nHostAbility));

    napi_value nIcon;
    NAPI_CALL_RETURN_VOID(env, napi_create_string_utf8(env, shortcutInfo.icon.c_str(), NAPI_AUTO_LENGTH, &nIcon));
    NAPI_CALL_RETURN_VOID(env, napi_set_named_property(env, objShortcutInfo, "icon", nIcon));

    napi_value nLabel;
    NAPI_CALL_RETURN_VOID(env, napi_create_string_utf8(env, shortcutInfo.label.c_str(), NAPI_AUTO_LENGTH, &nLabel));
    NAPI_CALL_RETURN_VOID(env, napi_set_named_property(env, objShortcutInfo, "label", nLabel));

    napi_value nDisableMessage;
    NAPI_CALL_RETURN_VOID(
        env, napi_create_string_utf8(env, shortcutInfo.disableMessage.c_str(), NAPI_AUTO_LENGTH, &nDisableMessage));
    NAPI_CALL_RETURN_VOID(env, napi_set_named_property(env, objShortcutInfo, "disableMessage", nDisableMessage));

    napi_value nIsStatic;
    NAPI_CALL_RETURN_VOID(env, napi_get_boolean(env, shortcutInfo.isStatic, &nIsStatic));
    NAPI_CALL_RETURN_VOID(env, napi_set_named_property(env, objShortcutInfo, "isStatic", nIsStatic));

    napi_value nIsHomeShortcut;
    NAPI_CALL_RETURN_VOID(env, napi_get_boolean(env, shortcutInfo.isHomeShortcut, &nIsHomeShortcut));
    NAPI_CALL_RETURN_VOID(env, napi_set_named_property(env, objShortcutInfo, "isHomeShortcut", nIsHomeShortcut));

    napi_value nIsEnables;
    NAPI_CALL_RETURN_VOID(env, napi_get_boolean(env, shortcutInfo.isEnables, &nIsEnables));
    NAPI_CALL_RETURN_VOID(env, napi_set_named_property(env, objShortcutInfo, "isEnabled", nIsEnables));

    napi_value nIntents;
    NAPI_CALL_RETURN_VOID(env, napi_create_array(env, &nIntents));
    for (size_t idx = 0; idx < shortcutInfo.intents.size(); idx++) {
        napi_value nIntent;
        NAPI_CALL_RETURN_VOID(env, napi_create_object(env, &nIntent));
        ConvertShortcutIntent(env, nIntent, shortcutInfo.intents[idx]);
        NAPI_CALL_RETURN_VOID(env, napi_set_element(env, nIntents, idx, nIntent));
    }
    NAPI_CALL_RETURN_VOID(env, napi_set_named_property(env, objShortcutInfo, "wants", nIntents));
}

static void ConvertModuleUsageRecords(
    napi_env env, napi_value objModuleUsageRecord, const ModuleUsageRecord &moduleUsageRecord)
{
    napi_value nbundleName;
    NAPI_CALL_RETURN_VOID(
        env, napi_create_string_utf8(env, moduleUsageRecord.bundleName.c_str(), NAPI_AUTO_LENGTH, &nbundleName));
    NAPI_CALL_RETURN_VOID(env, napi_set_named_property(env, objModuleUsageRecord, "bundleName", nbundleName));
    HILOG_INFO("ConvertModuleUsageRecords bundleName=%{public}s.", moduleUsageRecord.bundleName.c_str());

    napi_value nappLabelId;
    NAPI_CALL_RETURN_VOID(env, napi_create_uint32(env, moduleUsageRecord.appLabelId, &nappLabelId));
    NAPI_CALL_RETURN_VOID(env, napi_set_named_property(env, objModuleUsageRecord, "appLabelId", nappLabelId));
    HILOG_INFO("ConvertModuleUsageRecords appLabelId=%{public}ud.", moduleUsageRecord.appLabelId);

    napi_value nname;
    NAPI_CALL_RETURN_VOID(env, napi_create_string_utf8(env, moduleUsageRecord.name.c_str(), NAPI_AUTO_LENGTH, &nname));
    NAPI_CALL_RETURN_VOID(env, napi_set_named_property(env, objModuleUsageRecord, "name", nname));
    HILOG_INFO("ConvertModuleUsageRecords name=%{public}s.", moduleUsageRecord.name.c_str());

    napi_value nlabelId;
    NAPI_CALL_RETURN_VOID(env, napi_create_uint32(env, moduleUsageRecord.labelId, &nlabelId));
    NAPI_CALL_RETURN_VOID(env, napi_set_named_property(env, objModuleUsageRecord, "labelId", nlabelId));
    HILOG_INFO("ConvertModuleUsageRecords labelId=%{public}ud.", moduleUsageRecord.labelId);

    napi_value ndescriptionId;
    NAPI_CALL_RETURN_VOID(env, napi_create_uint32(env, moduleUsageRecord.descriptionId, &ndescriptionId));
    NAPI_CALL_RETURN_VOID(env, napi_set_named_property(env, objModuleUsageRecord, "descriptionId", ndescriptionId));
    HILOG_INFO("ConvertModuleUsageRecords descriptionId=%{public}ud.", moduleUsageRecord.descriptionId);

    napi_value nabilityName;
    NAPI_CALL_RETURN_VOID(
        env, napi_create_string_utf8(env, moduleUsageRecord.abilityName.c_str(), NAPI_AUTO_LENGTH, &nabilityName));
    NAPI_CALL_RETURN_VOID(env, napi_set_named_property(env, objModuleUsageRecord, "abilityName", nabilityName));
    HILOG_INFO("ConvertModuleUsageRecords abilityName=%{public}s.", moduleUsageRecord.abilityName.c_str());

    napi_value nabilityLabelId;
    NAPI_CALL_RETURN_VOID(env, napi_create_uint32(env, moduleUsageRecord.abilityLabelId, &nabilityLabelId));
    NAPI_CALL_RETURN_VOID(env, napi_set_named_property(env, objModuleUsageRecord, "abilityLabelId", nabilityLabelId));
    HILOG_INFO("ConvertModuleUsageRecords abilityLabelId=%{public}ud.", moduleUsageRecord.abilityLabelId);

    napi_value nabilityDescriptionId;
    NAPI_CALL_RETURN_VOID(env, napi_create_uint32(env, moduleUsageRecord.abilityDescriptionId, &nabilityDescriptionId));
    NAPI_CALL_RETURN_VOID(
        env, napi_set_named_property(env, objModuleUsageRecord, "abilityDescriptionId", nabilityDescriptionId));
    HILOG_INFO("ConvertModuleUsageRecords abilityDescriptionId=%{public}ud.", moduleUsageRecord.abilityDescriptionId);

    napi_value nabilityIconId;
    NAPI_CALL_RETURN_VOID(env, napi_create_uint32(env, moduleUsageRecord.abilityIconId, &nabilityIconId));
    NAPI_CALL_RETURN_VOID(env, napi_set_named_property(env, objModuleUsageRecord, "abilityIconId", nabilityIconId));
    HILOG_INFO("ConvertModuleUsageRecords abilityIconId=%{public}ud.", moduleUsageRecord.abilityIconId);

    napi_value nlaunchedCount;
    NAPI_CALL_RETURN_VOID(env, napi_create_uint32(env, moduleUsageRecord.launchedCount, &nlaunchedCount));
    NAPI_CALL_RETURN_VOID(env, napi_set_named_property(env, objModuleUsageRecord, "launchedCount", nlaunchedCount));
    HILOG_INFO("ConvertModuleUsageRecords launchedCount=%{public}ud.", moduleUsageRecord.launchedCount);

    napi_value nlastLaunchTime;
    NAPI_CALL_RETURN_VOID(env, napi_create_int64(env, moduleUsageRecord.lastLaunchTime, &nlastLaunchTime));
    NAPI_CALL_RETURN_VOID(env, napi_set_named_property(env, objModuleUsageRecord, "lastLaunchTime", nlastLaunchTime));

    napi_value nremoved;
    NAPI_CALL_RETURN_VOID(env, napi_get_boolean(env, moduleUsageRecord.removed, &nremoved));
    NAPI_CALL_RETURN_VOID(env, napi_set_named_property(env, objModuleUsageRecord, "isRemoved", nremoved));

    napi_value ninstallationFreeSupported;
    NAPI_CALL_RETURN_VOID(
        env, napi_get_boolean(env, moduleUsageRecord.installationFreeSupported, &ninstallationFreeSupported));
    NAPI_CALL_RETURN_VOID(env,
        napi_set_named_property(env, objModuleUsageRecord, "installationFreeSupported", ninstallationFreeSupported));
}

static std::string GetStringFromNAPI(napi_env env, napi_value value)
{
    std::string result;
    size_t size = 0;

    if (napi_get_value_string_utf8(env, value, nullptr, NAPI_RETURN_ZERO, &size) != napi_ok) {
        HILOG_ERROR("can not get string size");
        return "";
    }
    result.reserve(size + NAPI_RETURN_ONE);
    result.resize(size);
    if (napi_get_value_string_utf8(env, value, result.data(), (size + NAPI_RETURN_ONE), &size) != napi_ok) {
        HILOG_ERROR("can not get string value");
        return "";
    }
    return result;
}

static napi_value ParseInt(napi_env env, int &param, napi_value args)
{
    napi_valuetype valuetype = napi_undefined;
    NAPI_CALL(env, napi_typeof(env, args, &valuetype));
    HILOG_INFO("param=%{public}d.", valuetype);
    NAPI_ASSERT(env, valuetype == napi_number, "Wrong argument type. int32 expected.");
    int32_t value = 0;
    napi_get_value_int32(env, args, &value);
    HILOG_INFO("param=%{public}d.", value);
    param = value;
    // create result code
    napi_value result = nullptr;
    napi_status status = napi_create_int32(env, NAPI_RETURN_ONE, &result);
    NAPI_ASSERT(env, status == napi_ok, "napi_create_int32 error!");
    return result;
}

static napi_value GetCallbackErrorValue(napi_env env, int errCode)
{
    napi_value result = nullptr;
    napi_value eCode = nullptr;
    NAPI_CALL(env, napi_create_int32(env, errCode, &eCode));
    NAPI_CALL(env, napi_create_object(env, &result));
    NAPI_CALL(env, napi_set_named_property(env, result, "code", eCode));
    return result;
}

static bool InnerGetApplicationInfos(napi_env env, const ApplicationFlag applicationFlag, const int userId,
    std::vector<OHOS::AppExecFwk::ApplicationInfo> &appInfos)
{
    auto iBundleMgr = GetBundleMgr();
    if (!iBundleMgr) {
        HILOG_ERROR("can not get iBundleMgr");
        return false;
    }
    return iBundleMgr->GetApplicationInfos(applicationFlag, userId, appInfos);
}

static void ProcessApplicationInfos(
    napi_env env, napi_value result, const std::vector<OHOS::AppExecFwk::ApplicationInfo> &appInfos)
{
    if (appInfos.size() > 0) {
        HILOG_INFO("-----appInfos is not null-----");
        size_t index = 0;
        for (const auto &item : appInfos) {
            HILOG_INFO("name{%s} ", item.name.c_str());
            HILOG_INFO("bundleName{%s} ", item.bundleName.c_str());
            for (const auto &moduleInfo : item.moduleInfos) {
                HILOG_INFO("moduleName{%s} ", moduleInfo.moduleName.c_str());
                HILOG_INFO("bundleName{%s} ", moduleInfo.moduleSourceDir.c_str());
            }
            napi_value objAppInfo;
            NAPI_CALL_RETURN_VOID(env, napi_create_object(env, &objAppInfo));
            ConvertApplicationInfo(env, objAppInfo, item);
            NAPI_CALL_RETURN_VOID(env, napi_set_element(env, result, index, objAppInfo));
            index++;
        }
    } else {
        HILOG_INFO("-----appInfos is null-----");
    }
}
/**
 * Promise and async callback
 */
napi_value GetApplicationInfos(napi_env env, napi_callback_info info)
{
    size_t argc = ARGS_SIZE_THREE;
    napi_value argv[ARGS_SIZE_THREE] = {nullptr};
    napi_value thisArg;
    void *data = nullptr;
    NAPI_CALL(env, napi_get_cb_info(env, info, &argc, argv, &thisArg, &data));
    HILOG_INFO("ARGCSIZE is =%{public}zu.", argc);
    int flag;
    int userId;
    ParseInt(env, flag, argv[PARAM0]);
    ParseInt(env, userId, argv[PARAM1]);
    ApplicationFlag applicationFlag = ApplicationFlag::GET_APPLICATION_INFO_WITH_PERMS;
    if (flag == static_cast<int>(ApplicationFlag::GET_BASIC_APPLICATION_INFO)) {
        applicationFlag = ApplicationFlag::GET_BASIC_APPLICATION_INFO;
    }

    AsyncApplicationInfosCallbackInfo *asyncCallbackInfo = new (std::nothrow) AsyncApplicationInfosCallbackInfo {
        .env = env, .asyncWork = nullptr, .deferred = nullptr, .applicationFlag = applicationFlag, .userId = userId};
    if (asyncCallbackInfo == nullptr) {
        return nullptr;
    }
    if (argc > (ARGS_SIZE_THREE - CALLBACK_SIZE)) {
        HILOG_INFO("GetApplicationInfos asyncCallback.");
        napi_value resourceName;
        NAPI_CALL(env, napi_create_string_latin1(env, "GetApplicationInfos", NAPI_AUTO_LENGTH, &resourceName));
        napi_valuetype valuetype = napi_undefined;
        napi_typeof(env, argv[ARGS_SIZE_TWO], &valuetype);
        NAPI_ASSERT(env, valuetype == napi_function, "Wrong argument type. Function expected.");
        NAPI_CALL(env, napi_create_reference(env, argv[ARGS_SIZE_TWO], NAPI_RETURN_ONE, &asyncCallbackInfo->callback));

        napi_create_async_work(env,
            nullptr,
            resourceName,
            [](napi_env env, void *data) {
                AsyncApplicationInfosCallbackInfo *asyncCallbackInfo = (AsyncApplicationInfosCallbackInfo *)data;
                asyncCallbackInfo->ret = InnerGetApplicationInfos(
                    env, asyncCallbackInfo->applicationFlag, asyncCallbackInfo->userId, asyncCallbackInfo->appInfos);
            },
            [](napi_env env, napi_status status, void *data) {
                AsyncApplicationInfosCallbackInfo *asyncCallbackInfo = (AsyncApplicationInfosCallbackInfo *)data;
                napi_value result[ARGS_SIZE_TWO] = {0};
                napi_value callback = 0;
                napi_value undefined = 0;
                napi_value callResult = 0;
                napi_get_undefined(env, &undefined);
                napi_create_array(env, &result[PARAM1]);
                ProcessApplicationInfos(env, result[PARAM1], asyncCallbackInfo->appInfos);
                result[PARAM0] = GetCallbackErrorValue(env, asyncCallbackInfo->ret ? CODE_SUCCESS : CODE_FAILED);
                napi_get_reference_value(env, asyncCallbackInfo->callback, &callback);
                napi_call_function(env, undefined, callback, ARGS_SIZE_TWO, &result[PARAM0], &callResult);

                if (asyncCallbackInfo->callback != nullptr) {
                    napi_delete_reference(env, asyncCallbackInfo->callback);
                }
                napi_delete_async_work(env, asyncCallbackInfo->asyncWork);
                delete asyncCallbackInfo;
                asyncCallbackInfo = nullptr;
            },
            (void *)asyncCallbackInfo,
            &asyncCallbackInfo->asyncWork);
        NAPI_CALL(env, napi_queue_async_work(env, asyncCallbackInfo->asyncWork));
        napi_value ret = nullptr;
        NAPI_CALL(env, napi_get_null(env, &ret));
        if (ret == nullptr) {
            if (asyncCallbackInfo != nullptr) {
                delete asyncCallbackInfo;
                asyncCallbackInfo = nullptr;
            }
        }
        napi_value result;
        NAPI_CALL(env, napi_create_int32(env, NAPI_RETURN_ONE, &result));
        return result;
    } else {
        HILOG_INFO("GetApplicationInfos promise.");
        napi_deferred deferred;
        napi_value promise;
        NAPI_CALL(env, napi_create_promise(env, &deferred, &promise));
        asyncCallbackInfo->deferred = deferred;

        napi_value resourceName;
        napi_create_string_latin1(env, "GetApplicationInfos", NAPI_AUTO_LENGTH, &resourceName);
        napi_create_async_work(env,
            nullptr,
            resourceName,
            [](napi_env env, void *data) {
                AsyncApplicationInfosCallbackInfo *asyncCallbackInfo = (AsyncApplicationInfosCallbackInfo *)data;
                InnerGetApplicationInfos(
                    env, asyncCallbackInfo->applicationFlag, asyncCallbackInfo->userId, asyncCallbackInfo->appInfos);
            },
            [](napi_env env, napi_status status, void *data) {
                HILOG_INFO("=================load=================");
                AsyncApplicationInfosCallbackInfo *asyncCallbackInfo = (AsyncApplicationInfosCallbackInfo *)data;
                napi_value result;
                napi_create_array(env, &result);
                ProcessApplicationInfos(env, result, asyncCallbackInfo->appInfos);
                napi_resolve_deferred(asyncCallbackInfo->env, asyncCallbackInfo->deferred, result);
                napi_delete_async_work(env, asyncCallbackInfo->asyncWork);
                delete asyncCallbackInfo;
                asyncCallbackInfo = nullptr;
            },
            (void *)asyncCallbackInfo,
            &asyncCallbackInfo->asyncWork);
        napi_queue_async_work(env, asyncCallbackInfo->asyncWork);

        napi_value ret = nullptr;
        NAPI_CALL(env, napi_get_null(env, &ret));
        if (ret == nullptr) {
            if (asyncCallbackInfo != nullptr) {
                delete asyncCallbackInfo;
                asyncCallbackInfo = nullptr;
            }
        }
        return promise;
    }
}

// QueryAbilityInfos(want)
static bool InnerQueryAbilityInfos(napi_env env, const Want &want, std::vector<AbilityInfo> &abilityInfos)
{
    auto iBundleMgr = GetBundleMgr();
    if (!iBundleMgr) {
        HILOG_ERROR("can not get iBundleMgr");
        return false;
    }
    return iBundleMgr->QueryAbilityInfos(want, abilityInfos);
}

static napi_value ParseWant(napi_env env, Want &want, napi_value args)
{
    napi_status status;
    napi_valuetype valueType;
    NAPI_CALL(env, napi_typeof(env, args, &valueType));
    NAPI_ASSERT(env, valueType == napi_object, "param type mismatch!");
    HILOG_INFO("-----ParseWant type1-----");
    napi_value wantProp = nullptr;
    status = napi_get_named_property(env, args, "want", &wantProp);
    NAPI_ASSERT(env, status == napi_ok, "property name incorrect!");
    napi_typeof(env, wantProp, &valueType);
    NAPI_ASSERT(env, valueType == napi_object, "property type mismatch!");
    HILOG_INFO("-----ParseWant want-----");
    std::string deviceId;
    std::string bundleName;
    std::string abilityName;
    napi_value property = nullptr;
    status = napi_get_named_property(env, wantProp, "elementName", &property);
    NAPI_ASSERT(env, status == napi_ok, "property name incorrect!");
    napi_typeof(env, property, &valueType);
    NAPI_ASSERT(env, valueType == napi_object, "property type mismatch!");
    // get elementName:deviceId_ property
    napi_value prop = nullptr;
    status = napi_get_named_property(env, property, "deviceId", &prop);
    NAPI_ASSERT(env, status == napi_ok, "property name incorrect!");
    napi_typeof(env, prop, &valueType);
    NAPI_ASSERT(env, valueType == napi_string, "property type mismatch!");
    deviceId = GetStringFromNAPI(env, prop);
    // get elementName:bundleName_ property
    prop = nullptr;
    status = napi_get_named_property(env, property, "bundleName", &prop);
    NAPI_ASSERT(env, status == napi_ok, "property name incorrect!");
    napi_typeof(env, prop, &valueType);
    NAPI_ASSERT(env, valueType == napi_string, "property type mismatch!");
    bundleName = GetStringFromNAPI(env, prop);
    HILOG_INFO("ParseWant bundleName=%{public}s.", bundleName.c_str());
    // get elementName:abilityName_ property
    prop = nullptr;
    status = napi_get_named_property(env, property, "abilityName", &prop);
    NAPI_ASSERT(env, status == napi_ok, "property name incorrect!");
    napi_typeof(env, prop, &valueType);
    NAPI_ASSERT(env, valueType == napi_string, "property type mismatch!");
    abilityName = GetStringFromNAPI(env, prop);
    HILOG_INFO("ParseWant abilityName=%{public}s.", abilityName.c_str());
    ElementName elementName;
    elementName.SetBundleName(bundleName);
    elementName.SetDeviceID(deviceId);
    want.SetElement(elementName);

    // create result code
    napi_value result;
    status = napi_create_int32(env, NAPI_RETURN_ONE, &result);
    NAPI_ASSERT(env, status == napi_ok, "napi_create_int32 error!");
    return result;
}

/**
 * Promise and async callback
 */
napi_value QueryAbilityInfos(napi_env env, napi_callback_info info)
{

    HILOG_INFO("QueryAbilityInfos called");
    size_t argc = ARGS_SIZE_FOUR;
    napi_value argv[ARGS_SIZE_FOUR] = {nullptr};
    NAPI_CALL(env, napi_get_cb_info(env, info, &argc, argv, NULL, NULL));
    HILOG_INFO("argc = [%{public}zu]", argc);
    Want want;
    ParseWant(env, want, argv[PARAM0]);
    HILOG_INFO("After ParseWant action=%{public}s.", want.GetAction().c_str());
    HILOG_INFO("After ParseWant bundleName=%{public}s.", want.GetElement().GetBundleName().c_str());
    HILOG_INFO("After ParseWant abilityName=%{public}s.", want.GetElement().GetAbilityName().c_str());
    int bundleFlags;
    int userId;
    ParseInt(env, bundleFlags, argv[PARAM1]);
    ParseInt(env, userId, argv[PARAM2]);

    AsyncAbilityInfoCallbackInfo *asyncCallbackInfo =
        new AsyncAbilityInfoCallbackInfo {.env = env, .asyncWork = nullptr, .deferred = nullptr, .want = want};
    if (asyncCallbackInfo == nullptr) {
        return nullptr;
    }
    if (argc > (ARGS_SIZE_FOUR - CALLBACK_SIZE)) {
        HILOG_INFO("QueryAbilityInfos asyncCallback.");
        napi_valuetype valuetype = napi_undefined;
        napi_typeof(env, argv[ARGS_SIZE_THREE], &valuetype);
        NAPI_ASSERT(env, valuetype == napi_function, "Wrong argument type. Function expected.");
        NAPI_CALL(
            env, napi_create_reference(env, argv[ARGS_SIZE_THREE], NAPI_RETURN_ONE, &asyncCallbackInfo->callback));

        napi_value resourceName;
        napi_create_string_latin1(env, "QueryAbilityInfos", NAPI_AUTO_LENGTH, &resourceName);
        napi_create_async_work(env,
            nullptr,
            resourceName,
            [](napi_env env, void *data) {
                AsyncAbilityInfoCallbackInfo *asyncCallbackInfo = (AsyncAbilityInfoCallbackInfo *)data;
                asyncCallbackInfo->ret =
                    InnerQueryAbilityInfos(env, asyncCallbackInfo->want, asyncCallbackInfo->abilityInfos);
            },
            [](napi_env env, napi_status status, void *data) {
                AsyncAbilityInfoCallbackInfo *asyncCallbackInfo = (AsyncAbilityInfoCallbackInfo *)data;
                napi_value result[ARGS_SIZE_TWO] = {0};
                napi_value callback = 0;
                napi_value undefined = 0;
                napi_value callResult = 0;
                napi_get_undefined(env, &undefined);
                napi_create_array(env, &result[PARAM1]);
                ProcessAbilityInfos(env, result[PARAM1], asyncCallbackInfo->abilityInfos);
                result[PARAM0] = GetCallbackErrorValue(env, asyncCallbackInfo->ret ? CODE_SUCCESS : CODE_FAILED);
                napi_get_reference_value(env, asyncCallbackInfo->callback, &callback);
                napi_call_function(env, undefined, callback, ARGS_SIZE_TWO, &result[PARAM0], &callResult);

                if (asyncCallbackInfo->callback != nullptr) {
                    napi_delete_reference(env, asyncCallbackInfo->callback);
                }
                napi_delete_async_work(env, asyncCallbackInfo->asyncWork);
                delete asyncCallbackInfo;
                asyncCallbackInfo = nullptr;
            },
            (void *)asyncCallbackInfo,
            &asyncCallbackInfo->asyncWork);
        NAPI_CALL(env, napi_queue_async_work(env, asyncCallbackInfo->asyncWork));

        napi_value ret = nullptr;
        NAPI_CALL(env, napi_get_null(env, &ret));
        if (ret == nullptr) {
            if (asyncCallbackInfo != nullptr) {
                delete asyncCallbackInfo;
                asyncCallbackInfo = nullptr;
            }
        }
        napi_value result;
        NAPI_CALL(env, napi_create_int32(env, NAPI_RETURN_ONE, &result));
        return result;
    } else {
        HILOG_INFO("QueryAbilityInfos promise.");
        napi_deferred deferred;
        napi_value promise;
        NAPI_CALL(env, napi_create_promise(env, &deferred, &promise));
        asyncCallbackInfo->deferred = deferred;

        napi_value resourceName;
        napi_create_string_latin1(env, "QueryAbilityInfos", NAPI_AUTO_LENGTH, &resourceName);
        napi_create_async_work(env,
            nullptr,
            resourceName,
            [](napi_env env, void *data) {
                AsyncAbilityInfoCallbackInfo *asyncCallbackInfo = (AsyncAbilityInfoCallbackInfo *)data;
                InnerQueryAbilityInfos(env, asyncCallbackInfo->want, asyncCallbackInfo->abilityInfos);
            },
            [](napi_env env, napi_status status, void *data) {
                HILOG_INFO("=================load=================");
                AsyncAbilityInfoCallbackInfo *asyncCallbackInfo = (AsyncAbilityInfoCallbackInfo *)data;
                napi_value result;
                napi_create_array(env, &result);
                ProcessAbilityInfos(env, result, asyncCallbackInfo->abilityInfos);
                napi_resolve_deferred(asyncCallbackInfo->env, asyncCallbackInfo->deferred, result);
                napi_delete_async_work(env, asyncCallbackInfo->asyncWork);
                delete asyncCallbackInfo;
                asyncCallbackInfo = nullptr;
            },
            (void *)asyncCallbackInfo,
            &asyncCallbackInfo->asyncWork);
        napi_queue_async_work(env, asyncCallbackInfo->asyncWork);

        napi_value ret = nullptr;
        NAPI_CALL(env, napi_get_null(env, &ret));
        if (ret == nullptr) {
            if (asyncCallbackInfo != nullptr) {
                delete asyncCallbackInfo;
                asyncCallbackInfo = nullptr;
            }
        }

        return promise;
    }
}

static bool InnerGetApplicationInfo(napi_env env, const std::string &bundleName, const ApplicationFlag applicationFlag,
    const int userId, ApplicationInfo &appInfo)
{
    auto iBundleMgr = GetBundleMgr();
    if (!iBundleMgr) {
        HILOG_ERROR("can not get iBundleMgr");
        return false;
    }
    return iBundleMgr->GetApplicationInfo(bundleName, applicationFlag, userId, appInfo);
}

static napi_value ParseString(napi_env env, std::string &param, napi_value args)
{
    napi_status status;
    napi_valuetype valuetype;
    NAPI_CALL(env, napi_typeof(env, args, &valuetype));
    NAPI_ASSERT(env, valuetype == napi_string, "Wrong argument type. String expected.");
    param = GetStringFromNAPI(env, args);
    HILOG_INFO("param=%{public}s.", param.c_str());
    // create result code
    napi_value result;
    status = napi_create_int32(env, NAPI_RETURN_ONE, &result);
    NAPI_ASSERT(env, status == napi_ok, "napi_create_int32 error!");
    return result;
}
/**
 * Promise and async callback
 */
napi_value GetApplicationInfo(napi_env env, napi_callback_info info)
{
    HILOG_INFO("NAPI_GetApplicationInfo called");
    size_t argc = ARGS_SIZE_FOUR;
    napi_value argv[ARGS_SIZE_FOUR] = {nullptr};
    NAPI_CALL(env, napi_get_cb_info(env, info, &argc, argv, NULL, NULL));
    HILOG_INFO("argc = [%{public}zu]", argc);
    int flag;
    int userId;
    std::string bundleName;
    ParseString(env, bundleName, argv[PARAM0]);
    ParseInt(env, flag, argv[PARAM1]);
    ParseInt(env, userId, argv[PARAM2]);
    ApplicationFlag applicationFlag = ApplicationFlag::GET_APPLICATION_INFO_WITH_PERMS;
    if (flag == static_cast<int>(ApplicationFlag::GET_BASIC_APPLICATION_INFO)) {
        applicationFlag = ApplicationFlag::GET_BASIC_APPLICATION_INFO;
    }
    AsyncApplicationInfoCallbackInfo *asyncCallbackInfo =
        new (std::nothrow) AsyncApplicationInfoCallbackInfo {.env = env,
            .asyncWork = nullptr,
            .deferred = nullptr,
            .bundleName = bundleName,
            .applicationFlag = applicationFlag,
            .userId = userId};
    if (asyncCallbackInfo == nullptr) {
        return nullptr;
    }
    if (argc > (ARGS_SIZE_FOUR - CALLBACK_SIZE)) {
        HILOG_INFO("GetApplicationInfo asyncCallback.");
        napi_valuetype valuetype = napi_undefined;
        napi_typeof(env, argv[ARGS_SIZE_THREE], &valuetype);
        NAPI_ASSERT(env, valuetype == napi_function, "Wrong argument type. Function expected.");
        NAPI_CALL(
            env, napi_create_reference(env, argv[ARGS_SIZE_THREE], NAPI_RETURN_ONE, &asyncCallbackInfo->callback));

        napi_value resourceName;
        napi_create_string_latin1(env, "NAPI_GetApplicationInfoCallBack", NAPI_AUTO_LENGTH, &resourceName);
        napi_create_async_work(env,
            nullptr,
            resourceName,
            [](napi_env env, void *data) {
                AsyncApplicationInfoCallbackInfo *asyncCallbackInfo = (AsyncApplicationInfoCallbackInfo *)data;
                asyncCallbackInfo->ret = InnerGetApplicationInfo(env,
                    asyncCallbackInfo->bundleName,
                    asyncCallbackInfo->applicationFlag,
                    asyncCallbackInfo->userId,
                    asyncCallbackInfo->appInfo);
            },
            [](napi_env env, napi_status status, void *data) {
                AsyncApplicationInfoCallbackInfo *asyncCallbackInfo = (AsyncApplicationInfoCallbackInfo *)data;
                napi_value result[ARGS_SIZE_TWO] = {0};
                napi_value callback = 0;
                napi_value undefined = 0;
                napi_value callResult = 0;
                napi_get_undefined(env, &undefined);
                napi_create_object(env, &result[PARAM1]);
                HILOG_INFO("appInfo.name=%{public}s.", asyncCallbackInfo->appInfo.name.c_str());
                HILOG_INFO("appInfo.bundleName=%{public}s.", asyncCallbackInfo->appInfo.bundleName.c_str());
                HILOG_INFO("appInfo.description=%{public}s.", asyncCallbackInfo->appInfo.description.c_str());
                HILOG_INFO("appInfo.descriptionId=%{public}d.", asyncCallbackInfo->appInfo.descriptionId);
                ConvertApplicationInfo(env, result[PARAM1], asyncCallbackInfo->appInfo);
                result[PARAM0] = GetCallbackErrorValue(env, asyncCallbackInfo->ret ? CODE_SUCCESS : CODE_FAILED);
                napi_get_reference_value(env, asyncCallbackInfo->callback, &callback);
                napi_call_function(env, undefined, callback, ARGS_SIZE_TWO, &result[PARAM0], &callResult);

                if (asyncCallbackInfo->callback != nullptr) {
                    napi_delete_reference(env, asyncCallbackInfo->callback);
                }
                napi_delete_async_work(env, asyncCallbackInfo->asyncWork);
                delete asyncCallbackInfo;
                asyncCallbackInfo = nullptr;
            },
            (void *)asyncCallbackInfo,
            &asyncCallbackInfo->asyncWork);
        NAPI_CALL(env, napi_queue_async_work(env, asyncCallbackInfo->asyncWork));

        napi_value ret = nullptr;
        NAPI_CALL(env, napi_get_null(env, &ret));
        if (ret == nullptr) {
            if (asyncCallbackInfo != nullptr) {
                delete asyncCallbackInfo;
                asyncCallbackInfo = nullptr;
            }
        }
        napi_value result;
        NAPI_CALL(env, napi_create_int32(env, NAPI_RETURN_ONE, &result));
        return result;
    } else {
        HILOG_INFO("GetApplicationInfo promise.");
        napi_deferred deferred;
        napi_value promise;
        NAPI_CALL(env, napi_create_promise(env, &deferred, &promise));
        asyncCallbackInfo->deferred = deferred;

        napi_value resourceName;
        napi_create_string_latin1(env, "GetApplicationInfo", NAPI_AUTO_LENGTH, &resourceName);
        napi_create_async_work(env,
            nullptr,
            resourceName,
            [](napi_env env, void *data) {
                AsyncApplicationInfoCallbackInfo *asyncCallbackInfo = (AsyncApplicationInfoCallbackInfo *)data;
                InnerGetApplicationInfo(env,
                    asyncCallbackInfo->bundleName,
                    asyncCallbackInfo->applicationFlag,
                    asyncCallbackInfo->userId,
                    asyncCallbackInfo->appInfo);
            },
            [](napi_env env, napi_status status, void *data) {
                HILOG_INFO("=================load=================");
                AsyncApplicationInfoCallbackInfo *asyncCallbackInfo = (AsyncApplicationInfoCallbackInfo *)data;
                napi_value result;
                napi_create_object(env, &result);
                HILOG_INFO("appInfo.name=%{public}s.", asyncCallbackInfo->appInfo.name.c_str());
                HILOG_INFO("appInfo.bundleName=%{public}s.", asyncCallbackInfo->appInfo.bundleName.c_str());
                HILOG_INFO("appInfo.description=%{public}s.", asyncCallbackInfo->appInfo.description.c_str());
                HILOG_INFO("appInfo.descriptionId=%{public}d.", asyncCallbackInfo->appInfo.descriptionId);
                ConvertApplicationInfo(env, result, asyncCallbackInfo->appInfo);
                napi_resolve_deferred(asyncCallbackInfo->env, asyncCallbackInfo->deferred, result);
                napi_delete_async_work(env, asyncCallbackInfo->asyncWork);
                delete asyncCallbackInfo;
                asyncCallbackInfo = nullptr;
            },
            (void *)asyncCallbackInfo,
            &asyncCallbackInfo->asyncWork);
        napi_queue_async_work(env, asyncCallbackInfo->asyncWork);

        napi_value ret = nullptr;
        NAPI_CALL(env, napi_get_null(env, &ret));
        if (ret == nullptr) {
            if (asyncCallbackInfo != nullptr) {
                delete asyncCallbackInfo;
                asyncCallbackInfo = nullptr;
            }
        }
        return promise;
    }
}

static bool InnerGetBundleInfos(
    napi_env env, BundleFlag bundleFlag, std::vector<OHOS::AppExecFwk::BundleInfo> &bundleInfos)
{
    auto iBundleMgr = GetBundleMgr();
    if (!iBundleMgr) {
        HILOG_ERROR("can not get iBundleMgr");
        return false;
    }
    return iBundleMgr->GetBundleInfos(bundleFlag, bundleInfos);
}

static void ProcessBundleInfos(
    napi_env env, napi_value result, const std::vector<OHOS::AppExecFwk::BundleInfo> &bundleInfos)
{
    if (bundleInfos.size() > 0) {
        HILOG_INFO("-----bundleInfos is not null-----");
        size_t index = 0;
        for (const auto &item : bundleInfos) {
            HILOG_INFO("name{%s} ", item.name.c_str());
            HILOG_INFO("bundleName{%s} ", item.applicationInfo.bundleName.c_str());
            for (const auto &moduleInfo : item.applicationInfo.moduleInfos) {
                HILOG_INFO("moduleName{%s} ", moduleInfo.moduleName.c_str());
                HILOG_INFO("moduleSourceDir{%s} ", moduleInfo.moduleSourceDir.c_str());
            }
            napi_value objBundleInfo = nullptr;
            napi_create_object(env, &objBundleInfo);
            ConvertBundleInfo(env, objBundleInfo, item);
            napi_set_element(env, result, index, objBundleInfo);
            index++;
        }
    } else {
        HILOG_INFO("-----bundleInfos is null-----");
    }
}
/**
 * Promise and async callback
 */
napi_value GetBundleInfos(napi_env env, napi_callback_info info)
{
    size_t argc = ARGS_SIZE_TWO;
    napi_value argv[ARGS_SIZE_TWO] = {0};
    napi_value thisArg = nullptr;
    void *data = nullptr;
    NAPI_CALL(env, napi_get_cb_info(env, info, &argc, argv, &thisArg, &data));
    int flag;
    ParseInt(env, flag, argv[PARAM0]);
    BundleFlag bundleFlag = BundleFlag::GET_BUNDLE_WITH_ABILITIES;
    if (flag == static_cast<int>(BundleFlag::GET_BUNDLE_DEFAULT)) {
        bundleFlag = BundleFlag::GET_BUNDLE_DEFAULT;
    }
    AsyncBundleInfosCallbackInfo *asyncCallbackInfo = new (std::nothrow)
        AsyncBundleInfosCallbackInfo {.env = env, .asyncWork = nullptr, .deferred = nullptr, .bundleFlag = bundleFlag};
    if (asyncCallbackInfo == nullptr) {
        return nullptr;
    }
    if (argc > (ARGS_SIZE_TWO - CALLBACK_SIZE)) {
        HILOG_INFO("GetBundleInfo asyncCallback.");
        napi_value resourceName;
        napi_create_string_latin1(env, "GetBundleInfos", NAPI_AUTO_LENGTH, &resourceName);
        napi_valuetype valuetype = napi_undefined;
        napi_typeof(env, argv[ARGS_SIZE_ONE], &valuetype);
        NAPI_ASSERT(env, valuetype == napi_function, "Wrong argument type. Function expected.");
        NAPI_CALL(env, napi_create_reference(env, argv[ARGS_SIZE_ONE], NAPI_RETURN_ONE, &asyncCallbackInfo->callback));

        napi_create_async_work(env,
            nullptr,
            resourceName,
            [](napi_env env, void *data) {
                AsyncBundleInfosCallbackInfo *asyncCallbackInfo = (AsyncBundleInfosCallbackInfo *)data;
                asyncCallbackInfo->ret =
                    InnerGetBundleInfos(env, asyncCallbackInfo->bundleFlag, asyncCallbackInfo->bundleInfos);
            },
            [](napi_env env, napi_status status, void *data) {
                AsyncBundleInfosCallbackInfo *asyncCallbackInfo = (AsyncBundleInfosCallbackInfo *)data;
                napi_value result[ARGS_SIZE_TWO] = {0};
                napi_value callback = 0;
                napi_value undefined = 0;
                napi_value callResult = 0;
                napi_get_undefined(env, &undefined);
                napi_create_array(env, &result[PARAM1]);
                ProcessBundleInfos(env, result[PARAM1], asyncCallbackInfo->bundleInfos);
                result[PARAM0] = GetCallbackErrorValue(env, asyncCallbackInfo->ret ? CODE_SUCCESS : CODE_FAILED);
                napi_get_reference_value(env, asyncCallbackInfo->callback, &callback);
                napi_call_function(env, undefined, callback, ARGS_SIZE_TWO, &result[PARAM0], &callResult);

                if (asyncCallbackInfo->callback != nullptr) {
                    napi_delete_reference(env, asyncCallbackInfo->callback);
                }
                napi_delete_async_work(env, asyncCallbackInfo->asyncWork);
                delete asyncCallbackInfo;
                asyncCallbackInfo = nullptr;
            },
            (void *)asyncCallbackInfo,
            &asyncCallbackInfo->asyncWork);
        NAPI_CALL(env, napi_queue_async_work(env, asyncCallbackInfo->asyncWork));

        napi_value ret = nullptr;
        NAPI_CALL(env, napi_get_null(env, &ret));
        if (ret == nullptr) {
            if (asyncCallbackInfo != nullptr) {
                delete asyncCallbackInfo;
                asyncCallbackInfo = nullptr;
            }
        }
        napi_value result;
        NAPI_CALL(env, napi_create_int32(env, NAPI_RETURN_ONE, &result));
        return result;
    } else {
        HILOG_INFO("BundleMgr::GetBundleInfos promise.");
        napi_deferred deferred;
        napi_value promise;
        NAPI_CALL(env, napi_create_promise(env, &deferred, &promise));
        asyncCallbackInfo->deferred = deferred;

        napi_value resourceName;
        napi_create_string_latin1(env, "GetBundleInfos", NAPI_AUTO_LENGTH, &resourceName);
        napi_create_async_work(env,
            nullptr,
            resourceName,
            [](napi_env env, void *data) {
                AsyncBundleInfosCallbackInfo *asyncCallbackInfo = (AsyncBundleInfosCallbackInfo *)data;
                InnerGetBundleInfos(env, asyncCallbackInfo->bundleFlag, asyncCallbackInfo->bundleInfos);
            },
            [](napi_env env, napi_status status, void *data) {
                HILOG_INFO("=================load=================");
                AsyncBundleInfosCallbackInfo *asyncCallbackInfo = (AsyncBundleInfosCallbackInfo *)data;
                napi_value result;
                napi_create_array(env, &result);
                ProcessBundleInfos(env, result, asyncCallbackInfo->bundleInfos);
                napi_resolve_deferred(asyncCallbackInfo->env, asyncCallbackInfo->deferred, result);
                napi_delete_async_work(env, asyncCallbackInfo->asyncWork);
                delete asyncCallbackInfo;
                asyncCallbackInfo = nullptr;
            },
            (void *)asyncCallbackInfo,
            &asyncCallbackInfo->asyncWork);
        napi_queue_async_work(env, asyncCallbackInfo->asyncWork);

        napi_value ret = nullptr;
        NAPI_CALL(env, napi_get_null(env, &ret));
        if (ret == nullptr) {
            if (asyncCallbackInfo != nullptr) {
                delete asyncCallbackInfo;
                asyncCallbackInfo = nullptr;
            }
        }
        return promise;
    }
}

static bool InnerGetBundleInfo(
    napi_env env, const std::string &bundleName, const BundleFlag bundleFlag, BundleInfo &bundleInfo)
{
    auto iBundleMgr = GetBundleMgr();
    if (!iBundleMgr) {
        HILOG_ERROR("can not get iBundleMgr");
        return false;
    }
    bool ret = iBundleMgr->GetBundleInfo(bundleName, bundleFlag, bundleInfo);
    if (!ret) {
        HILOG_INFO("-----bundleInfo is not find-----");
    }
    return ret;
}
/**
 * Promise and async callback
 */
napi_value GetBundleInfo(napi_env env, napi_callback_info info)
{
    HILOG_INFO("NAPI_InnerGetBundleInfo called");
    size_t argc = ARGS_SIZE_THREE;
    napi_value argv[ARGS_SIZE_THREE] = {nullptr};
    NAPI_CALL(env, napi_get_cb_info(env, info, &argc, argv, NULL, NULL));
    HILOG_INFO("argc = [%{public}zu]", argc);
    std::string bundleName;
    int flag;
    ParseString(env, bundleName, argv[PARAM0]);
    ParseInt(env, flag, argv[PARAM1]);
    BundleFlag bundleFlag = BundleFlag::GET_BUNDLE_WITH_ABILITIES;
    if (flag == static_cast<int>(BundleFlag::GET_BUNDLE_DEFAULT)) {
        bundleFlag = BundleFlag::GET_BUNDLE_DEFAULT;
    }
    AsyncBundleInfoCallbackInfo *asyncCallbackInfo = new (std::nothrow) AsyncBundleInfoCallbackInfo {
        .env = env, .asyncWork = nullptr, .deferred = nullptr, .param = bundleName, .bundleFlag = bundleFlag};
    if (asyncCallbackInfo == nullptr) {
        return nullptr;
    }
    if (argc > (ARGS_SIZE_THREE - CALLBACK_SIZE)) {
        HILOG_INFO("InnerGetBundleInfo asyncCallback.");
        napi_valuetype valuetype = napi_undefined;
        napi_typeof(env, argv[ARGS_SIZE_TWO], &valuetype);
        NAPI_ASSERT(env, valuetype == napi_function, "Wrong argument type. Function expected.");
        NAPI_CALL(env, napi_create_reference(env, argv[ARGS_SIZE_TWO], NAPI_RETURN_ONE, &asyncCallbackInfo->callback));

        napi_value resourceName;
        napi_create_string_latin1(env, "NAPI_InnerGetBundleInfo", NAPI_AUTO_LENGTH, &resourceName);
        napi_create_async_work(env,
            nullptr,
            resourceName,
            [](napi_env env, void *data) {
                AsyncBundleInfoCallbackInfo *asyncCallbackInfo = (AsyncBundleInfoCallbackInfo *)data;
                asyncCallbackInfo->ret = InnerGetBundleInfo(
                    env, asyncCallbackInfo->param, asyncCallbackInfo->bundleFlag, asyncCallbackInfo->bundleInfo);
            },
            [](napi_env env, napi_status status, void *data) {
                AsyncBundleInfoCallbackInfo *asyncCallbackInfo = (AsyncBundleInfoCallbackInfo *)data;
                napi_value result[ARGS_SIZE_TWO] = {0};
                napi_value callback = 0;
                napi_value undefined = 0;
                napi_value callResult = 0;
                napi_get_undefined(env, &undefined);
                napi_create_object(env, &result[PARAM1]);
                ConvertBundleInfo(env, result[PARAM1], asyncCallbackInfo->bundleInfo);
                result[PARAM0] = GetCallbackErrorValue(env, asyncCallbackInfo->ret ? CODE_SUCCESS : CODE_FAILED);
                napi_get_reference_value(env, asyncCallbackInfo->callback, &callback);
                napi_call_function(env, undefined, callback, ARGS_SIZE_TWO, &result[PARAM0], &callResult);

                if (asyncCallbackInfo->callback != nullptr) {
                    napi_delete_reference(env, asyncCallbackInfo->callback);
                }
                napi_delete_async_work(env, asyncCallbackInfo->asyncWork);
                delete asyncCallbackInfo;
                asyncCallbackInfo = nullptr;
            },
            (void *)asyncCallbackInfo,
            &asyncCallbackInfo->asyncWork);

        NAPI_CALL(env, napi_queue_async_work(env, asyncCallbackInfo->asyncWork));
        napi_value ret = nullptr;
        NAPI_CALL(env, napi_get_null(env, &ret));
        if (ret == nullptr) {
            if (asyncCallbackInfo != nullptr) {
                delete asyncCallbackInfo;
                asyncCallbackInfo = nullptr;
            }
        }
        napi_value result;
        NAPI_CALL(env, napi_create_int32(env, NAPI_RETURN_ONE, &result));
        return result;
    } else {
        HILOG_INFO("GetBundleinfo promise.");
        napi_deferred deferred;
        napi_value promise;
        NAPI_CALL(env, napi_create_promise(env, &deferred, &promise));
        asyncCallbackInfo->deferred = deferred;

        napi_value resourceName;
        napi_create_string_latin1(env, "GetBundleInfo", NAPI_AUTO_LENGTH, &resourceName);
        napi_create_async_work(env,
            nullptr,
            resourceName,
            [](napi_env env, void *data) {
                AsyncBundleInfoCallbackInfo *asyncCallbackInfo = (AsyncBundleInfoCallbackInfo *)data;
                InnerGetBundleInfo(
                    env, asyncCallbackInfo->param, asyncCallbackInfo->bundleFlag, asyncCallbackInfo->bundleInfo);
            },
            [](napi_env env, napi_status status, void *data) {
                HILOG_INFO("=================load=================");
                AsyncBundleInfoCallbackInfo *asyncCallbackInfo = (AsyncBundleInfoCallbackInfo *)data;
                napi_value result;
                napi_create_object(env, &result);
                ConvertBundleInfo(env, result, asyncCallbackInfo->bundleInfo);
                napi_resolve_deferred(asyncCallbackInfo->env, asyncCallbackInfo->deferred, result);
                napi_delete_async_work(env, asyncCallbackInfo->asyncWork);
                delete asyncCallbackInfo;
                asyncCallbackInfo = nullptr;
            },
            (void *)asyncCallbackInfo,
            &asyncCallbackInfo->asyncWork);
        napi_queue_async_work(env, asyncCallbackInfo->asyncWork);

        napi_value ret = nullptr;
        NAPI_CALL(env, napi_get_null(env, &ret));
        if (ret == nullptr) {
            if (asyncCallbackInfo != nullptr) {
                delete asyncCallbackInfo;
                asyncCallbackInfo = nullptr;
            }
        }
        return promise;
    }
}

static bool InnerGetArchiveInfo(
    napi_env env, const std::string &hapFilePath, const BundleFlag bundleFlag, BundleInfo &bundleInfo)
{
    auto iBundleMgr = GetBundleMgr();
    if (!iBundleMgr) {
        HILOG_ERROR("can not get iBundleMgr");
        return false;
    };
    bool ret = iBundleMgr->GetBundleArchiveInfo(hapFilePath, bundleFlag, bundleInfo);
    if (!ret) {
        HILOG_INFO("-----bundleInfo is not find-----");
    }
    return ret;
}
/**
 * Promise and async callback
 */
napi_value GetBundleArchiveInfo(napi_env env, napi_callback_info info)
{
    HILOG_INFO("NAPI_GetBundleArchiveInfo called");
    size_t argc = ARGS_SIZE_THREE;
    napi_value argv[ARGS_SIZE_THREE] = {nullptr};
    NAPI_CALL(env, napi_get_cb_info(env, info, &argc, argv, NULL, NULL));
    HILOG_INFO("argc = [%{public}zu]", argc);
    std::string hapFilePath;
    int flag;
    ParseString(env, hapFilePath, argv[PARAM0]);
    ParseInt(env, flag, argv[PARAM1]);
    BundleFlag bundleFlag = BundleFlag::GET_BUNDLE_WITH_ABILITIES;
    if (flag == static_cast<int>(BundleFlag::GET_BUNDLE_DEFAULT)) {
        bundleFlag = BundleFlag::GET_BUNDLE_DEFAULT;
    }
    AsyncBundleInfoCallbackInfo *asyncCallbackInfo = new (std::nothrow) AsyncBundleInfoCallbackInfo {
        .env = env, .asyncWork = nullptr, .deferred = nullptr, .param = hapFilePath, .bundleFlag = bundleFlag};
    if (asyncCallbackInfo == nullptr) {
        return nullptr;
    }
    if (argc > (ARGS_SIZE_THREE - CALLBACK_SIZE)) {
        HILOG_INFO("GetBundleArchiveInfo asyncCallback.");
        napi_valuetype valuetype = napi_undefined;
        napi_typeof(env, argv[ARGS_SIZE_TWO], &valuetype);
        NAPI_ASSERT(env, valuetype == napi_function, "Wrong argument type. Function expected.");
        NAPI_CALL(env, napi_create_reference(env, argv[ARGS_SIZE_TWO], NAPI_RETURN_ONE, &asyncCallbackInfo->callback));

        napi_value resourceName;
        napi_create_string_latin1(env, "NAPI_GetBundleArchiveInfo", NAPI_AUTO_LENGTH, &resourceName);
        napi_create_async_work(env,
            nullptr,
            resourceName,
            [](napi_env env, void *data) {
                AsyncBundleInfoCallbackInfo *asyncCallbackInfo = (AsyncBundleInfoCallbackInfo *)data;
                asyncCallbackInfo->ret = InnerGetArchiveInfo(
                    env, asyncCallbackInfo->param, asyncCallbackInfo->bundleFlag, asyncCallbackInfo->bundleInfo);
            },
            [](napi_env env, napi_status status, void *data) {
                AsyncBundleInfoCallbackInfo *asyncCallbackInfo = (AsyncBundleInfoCallbackInfo *)data;
                napi_value result[ARGS_SIZE_TWO] = {0};
                napi_value callback = 0;
                napi_value undefined = 0;
                napi_value callResult = 0;
                napi_get_undefined(env, &undefined);
                napi_create_object(env, &result[PARAM1]);
                ConvertBundleInfo(env, result[PARAM1], asyncCallbackInfo->bundleInfo);
                result[PARAM0] = GetCallbackErrorValue(env, asyncCallbackInfo->ret ? CODE_SUCCESS : CODE_FAILED);
                napi_get_reference_value(env, asyncCallbackInfo->callback, &callback);
                napi_call_function(env, undefined, callback, ARGS_SIZE_TWO, &result[PARAM0], &callResult);

                if (asyncCallbackInfo->callback != nullptr) {
                    napi_delete_reference(env, asyncCallbackInfo->callback);
                }
                napi_delete_async_work(env, asyncCallbackInfo->asyncWork);
                delete asyncCallbackInfo;
                asyncCallbackInfo = nullptr;
            },
            (void *)asyncCallbackInfo,
            &asyncCallbackInfo->asyncWork);
        NAPI_CALL(env, napi_queue_async_work(env, asyncCallbackInfo->asyncWork));
        napi_value ret = nullptr;
        NAPI_CALL(env, napi_get_null(env, &ret));
        if (ret == nullptr) {
            if (asyncCallbackInfo != nullptr) {
                delete asyncCallbackInfo;
                asyncCallbackInfo = nullptr;
            }
        }
        napi_value result;
        NAPI_CALL(env, napi_create_int32(env, NAPI_RETURN_ONE, &result));
        return result;
    } else {
        HILOG_INFO("GetBundleArchiveInfo promise.");
        napi_deferred deferred;
        napi_value promise;
        NAPI_CALL(env, napi_create_promise(env, &deferred, &promise));
        asyncCallbackInfo->deferred = deferred;

        napi_value resourceName;
        napi_create_string_latin1(env, "GetBundleArchiveInfo", NAPI_AUTO_LENGTH, &resourceName);
        napi_create_async_work(env,
            nullptr,
            resourceName,
            [](napi_env env, void *data) {
                AsyncBundleInfoCallbackInfo *asyncCallbackInfo = (AsyncBundleInfoCallbackInfo *)data;
                InnerGetArchiveInfo(
                    env, asyncCallbackInfo->param, asyncCallbackInfo->bundleFlag, asyncCallbackInfo->bundleInfo);
            },
            [](napi_env env, napi_status status, void *data) {
                HILOG_INFO("=================load=================");
                AsyncBundleInfoCallbackInfo *asyncCallbackInfo = (AsyncBundleInfoCallbackInfo *)data;
                napi_value result;
                napi_create_object(env, &result);
                ConvertBundleInfo(env, result, asyncCallbackInfo->bundleInfo);
                napi_resolve_deferred(asyncCallbackInfo->env, asyncCallbackInfo->deferred, result);
                napi_delete_async_work(env, asyncCallbackInfo->asyncWork);
                delete asyncCallbackInfo;
                asyncCallbackInfo = nullptr;
            },
            (void *)asyncCallbackInfo,
            &asyncCallbackInfo->asyncWork);
        napi_queue_async_work(env, asyncCallbackInfo->asyncWork);

        napi_value ret = nullptr;
        NAPI_CALL(env, napi_get_null(env, &ret));
        if (ret == nullptr) {
            if (asyncCallbackInfo != nullptr) {
                delete asyncCallbackInfo;
                asyncCallbackInfo = nullptr;
            }
        }
        return promise;
    }
}

static void ConvertPermissionDef(napi_env env, napi_value result, const PermissionDef &permissionDef)
{
    napi_value nPermissionName;
    NAPI_CALL_RETURN_VOID(
        env, napi_create_string_utf8(env, permissionDef.permissionName.c_str(), NAPI_AUTO_LENGTH, &nPermissionName));
    NAPI_CALL_RETURN_VOID(env, napi_set_named_property(env, result, "name", nPermissionName));
    HILOG_INFO("InnerGetPermissionDef name=%{public}s.", permissionDef.permissionName.c_str());

    napi_value nBundleName;
    NAPI_CALL_RETURN_VOID(
        env, napi_create_string_utf8(env, permissionDef.bundleName.c_str(), NAPI_AUTO_LENGTH, &nBundleName));
    NAPI_CALL_RETURN_VOID(env, napi_set_named_property(env, result, "bundleName", nBundleName));
    HILOG_INFO("InnerGetPermissionDef bundleName=%{public}s.", permissionDef.bundleName.c_str());

    napi_value nGrantMode;
    NAPI_CALL_RETURN_VOID(env, napi_create_int32(env, permissionDef.grantMode, &nGrantMode));
    NAPI_CALL_RETURN_VOID(env, napi_set_named_property(env, result, "grantMode", nGrantMode));

    napi_value nAvailableScope;
    NAPI_CALL_RETURN_VOID(env, napi_create_int32(env, permissionDef.availableScope, &nAvailableScope));
    NAPI_CALL_RETURN_VOID(env, napi_set_named_property(env, result, "availableScope", nAvailableScope));

    napi_value nLabel;
    NAPI_CALL_RETURN_VOID(env, napi_create_string_utf8(env, permissionDef.label.c_str(), NAPI_AUTO_LENGTH, &nLabel));
    NAPI_CALL_RETURN_VOID(env, napi_set_named_property(env, result, "label", nLabel));

    napi_value nLabelId;
    NAPI_CALL_RETURN_VOID(env, napi_create_int32(env, permissionDef.labelId, &nLabelId));
    NAPI_CALL_RETURN_VOID(env, napi_set_named_property(env, result, "labelRes", nLabelId));

    napi_value nDescription;
    NAPI_CALL_RETURN_VOID(
        env, napi_create_string_utf8(env, permissionDef.description.c_str(), NAPI_AUTO_LENGTH, &nDescription));
    NAPI_CALL_RETURN_VOID(env, napi_set_named_property(env, result, "description", nDescription));

    napi_value nDescriptionId;
    NAPI_CALL_RETURN_VOID(env, napi_create_int32(env, permissionDef.descriptionId, &nDescriptionId));
    NAPI_CALL_RETURN_VOID(env, napi_set_named_property(env, result, "descriptionRes", nDescriptionId));

    napi_value nUsageInfo;
    NAPI_CALL_RETURN_VOID(env, napi_create_int32(env, DEFAULT_INT32, &nUsageInfo));
    NAPI_CALL_RETURN_VOID(env, napi_set_named_property(env, result, "usageInfo", nUsageInfo));

    napi_value nGroup;
    NAPI_CALL_RETURN_VOID(env, napi_create_string_utf8(env, std::string().c_str(), NAPI_AUTO_LENGTH, &nGroup));
    NAPI_CALL_RETURN_VOID(env, napi_set_named_property(env, result, "group", nGroup));

    napi_value nReminderIcon;
    NAPI_CALL_RETURN_VOID(env, napi_create_int32(env, DEFAULT_INT32, &nReminderIcon));
    NAPI_CALL_RETURN_VOID(env, napi_set_named_property(env, result, "reminderIcon", nReminderIcon));

    napi_value nReminderDesc;
    NAPI_CALL_RETURN_VOID(env, napi_create_string_utf8(env, std::string().c_str(), NAPI_AUTO_LENGTH, &nReminderDesc));
    NAPI_CALL_RETURN_VOID(env, napi_set_named_property(env, result, "reminderDesc", nReminderDesc));

    napi_value nPermissionFlags;
    NAPI_CALL_RETURN_VOID(env, napi_create_int32(env, DEFAULT_INT32, &nPermissionFlags));
    NAPI_CALL_RETURN_VOID(env, napi_set_named_property(env, result, "permissionFlags", nPermissionFlags));
}

static bool InnerGetPermissionDef(napi_env env, const std::string &permissionName, PermissionDef &permissionDef)
{
    auto iBundleMgr = GetBundleMgr();
    if (!iBundleMgr) {
        HILOG_ERROR("can not get iBundleMgr");
        return false;
    };
    bool ret = iBundleMgr->GetPermissionDef(permissionName, permissionDef);
    if (ret) {
        HILOG_INFO("-----permissionName is not find-----");
    }
    return ret;
}
/**
 * Promise and async callback
 */
napi_value GetPermissionDef(napi_env env, napi_callback_info info)
{
    HILOG_INFO("GetPermissionDef called");
    size_t argc = ARGS_SIZE_TWO;
    napi_value argv[ARGS_SIZE_TWO] = {nullptr};
    NAPI_CALL(env, napi_get_cb_info(env, info, &argc, argv, NULL, NULL));
    HILOG_INFO("argc = [%{public}zu]", argc);
    std::string permissionName;
    ParseString(env, permissionName, argv[PARAM0]);
    AsyncPermissionDefCallbackInfo *asyncCallbackInfo = new (std::nothrow) AsyncPermissionDefCallbackInfo {
        .env = env, .asyncWork = nullptr, .deferred = nullptr, .permissionName = permissionName};
    if (asyncCallbackInfo == nullptr) {
        return nullptr;
    }
    if (argc > (ARGS_SIZE_TWO - CALLBACK_SIZE)) {
        HILOG_INFO("GetPermissionDef asyncCallback.");
        napi_valuetype valuetype = napi_undefined;
        napi_typeof(env, argv[ARGS_SIZE_ONE], &valuetype);
        NAPI_ASSERT(env, valuetype == napi_function, "Wrong argument type. Function expected.");
        NAPI_CALL(env, napi_create_reference(env, argv[ARGS_SIZE_ONE], NAPI_RETURN_ONE, &asyncCallbackInfo->callback));

        napi_value resourceName;
        napi_create_string_latin1(env, "GetPermissionDef", NAPI_AUTO_LENGTH, &resourceName);
        napi_create_async_work(env,
            nullptr,
            resourceName,
            [](napi_env env, void *data) {
                AsyncPermissionDefCallbackInfo *asyncCallbackInfo = (AsyncPermissionDefCallbackInfo *)data;
                HILOG_INFO("asyncCallbackInfo->permissionName=%{public}s.", asyncCallbackInfo->permissionName.c_str());
                InnerGetPermissionDef(env, asyncCallbackInfo->permissionName, asyncCallbackInfo->permissionDef);
            },
            [](napi_env env, napi_status status, void *data) {
                AsyncPermissionDefCallbackInfo *asyncCallbackInfo = (AsyncPermissionDefCallbackInfo *)data;
                napi_value result[ARGS_SIZE_TWO] = {0};
                napi_value callback = 0;
                napi_value undefined = 0;
                napi_value callResult = 0;
                napi_get_undefined(env, &undefined);
                napi_create_object(env, &result[PARAM1]);
                ConvertPermissionDef(env, result[PARAM1], asyncCallbackInfo->permissionDef);
                result[PARAM0] = GetCallbackErrorValue(env, asyncCallbackInfo->ret ? CODE_SUCCESS : CODE_FAILED);
                napi_get_reference_value(env, asyncCallbackInfo->callback, &callback);
                napi_call_function(env, undefined, callback, ARGS_SIZE_TWO, &result[PARAM0], &callResult);

                if (asyncCallbackInfo->callback != nullptr) {
                    napi_delete_reference(env, asyncCallbackInfo->callback);
                }
                napi_delete_async_work(env, asyncCallbackInfo->asyncWork);
                delete asyncCallbackInfo;
                asyncCallbackInfo = nullptr;
            },
            (void *)asyncCallbackInfo,
            &asyncCallbackInfo->asyncWork);
        NAPI_CALL(env, napi_queue_async_work(env, asyncCallbackInfo->asyncWork));
        napi_value ret = nullptr;
        NAPI_CALL(env, napi_get_null(env, &ret));
        if (ret == nullptr) {
            if (asyncCallbackInfo != nullptr) {
                delete asyncCallbackInfo;
                asyncCallbackInfo = nullptr;
            }
        }
        napi_value result;
        NAPI_CALL(env, napi_create_int32(env, NAPI_RETURN_ONE, &result));
        return result;
    } else {
        napi_deferred deferred;
        napi_value promise;
        NAPI_CALL(env, napi_create_promise(env, &deferred, &promise));
        asyncCallbackInfo->deferred = deferred;

        napi_value resourceName;
        napi_create_string_latin1(env, "GetBundleInfo", NAPI_AUTO_LENGTH, &resourceName);
        napi_create_async_work(env,
            nullptr,
            resourceName,
            [](napi_env env, void *data) {
                AsyncPermissionDefCallbackInfo *asyncCallbackInfo = (AsyncPermissionDefCallbackInfo *)data;
                InnerGetPermissionDef(env, asyncCallbackInfo->permissionName, asyncCallbackInfo->permissionDef);
            },
            [](napi_env env, napi_status status, void *data) {
                HILOG_INFO("=================load=================");
                AsyncPermissionDefCallbackInfo *asyncCallbackInfo = (AsyncPermissionDefCallbackInfo *)data;
                napi_value result;
                napi_create_object(env, &result);
                ConvertPermissionDef(env, result, asyncCallbackInfo->permissionDef);
                napi_resolve_deferred(asyncCallbackInfo->env, asyncCallbackInfo->deferred, result);
                napi_delete_async_work(env, asyncCallbackInfo->asyncWork);
                delete asyncCallbackInfo;
                asyncCallbackInfo = nullptr;
            },
            (void *)asyncCallbackInfo,
            &asyncCallbackInfo->asyncWork);
        napi_queue_async_work(env, asyncCallbackInfo->asyncWork);

        napi_value ret = nullptr;
        NAPI_CALL(env, napi_get_null(env, &ret));
        if (ret == nullptr) {
            if (asyncCallbackInfo != nullptr) {
                delete asyncCallbackInfo;
                asyncCallbackInfo = nullptr;
            }
        }
        return promise;
    }
}

static void InnerInstall(napi_env env, const std::vector<std::string> &bundleFilePath, InstallParam &installParam,
    InstallResult &installResult)
{
    if (bundleFilePath.empty()) {
        installResult.resultCode = static_cast<int32_t>(IStatusReceiver::ERR_INSTALL_INVALID_HAP_NAME);
        return;
    }
    auto iBundleMgr = GetBundleMgr();
    if (!iBundleMgr) {
        HILOG_ERROR("can not get iBundleMgr");
        return;
    }
    auto iBundleInstaller = iBundleMgr->GetBundleInstaller();
    if (!iBundleInstaller) {
        HILOG_ERROR("can not get iBundleInstaller");
        return;
    }
    installParam.installFlag = InstallFlag::REPLACE_EXISTING;
    for (const auto &file : bundleFilePath) {
        OHOS::sptr<InstallerCallback> callback = new InstallerCallback();
        if (!callback) {
            HILOG_ERROR("callback nullptr");
            return;
        }
        iBundleInstaller->Install(file, installParam, callback);
        installResult.resultMsg = callback->GetResultMsg();
        HILOG_INFO("-----InnerInstall resultMsg %{public}s-----", installResult.resultMsg.c_str());
        installResult.resultCode = callback->GetResultCode();
        HILOG_INFO("-----InnerInstall resultCode %{public}d-----", installResult.resultCode);
    }
}
/**
 * Promise and async callback
 */
napi_value GetBundleInstaller(napi_env env, napi_callback_info info)
{
    HILOG_INFO("GetBundleInstaller called");
    size_t argc = ARGS_SIZE_ONE;
    napi_value argv[ARGS_SIZE_ONE] = {nullptr};
    NAPI_CALL(env, napi_get_cb_info(env, info, &argc, argv, NULL, NULL));
    HILOG_INFO("argc = [%{public}zu]", argc);

    AsyncGetBundleInstallerCallbackInfo *asyncCallbackInfo =
        new AsyncGetBundleInstallerCallbackInfo {.env = env, .asyncWork = nullptr, .deferred = nullptr};
    if (asyncCallbackInfo == nullptr) {
        return nullptr;
    }
    if (argc > (ARGS_SIZE_ONE - CALLBACK_SIZE)) {
        HILOG_INFO("GetBundleInstaller asyncCallback.");
        napi_valuetype valuetype = napi_undefined;
        NAPI_CALL(env, napi_typeof(env, argv[PARAM0], &valuetype));
        NAPI_ASSERT(env, valuetype == napi_function, "Wrong argument type. Function expected.");
        napi_create_reference(env, argv[PARAM0], NAPI_RETURN_ONE, &asyncCallbackInfo->callback);

        napi_value resourceName;
        napi_create_string_latin1(env, "GetBundleInstaller", NAPI_AUTO_LENGTH, &resourceName);
        napi_create_async_work(env,
            nullptr,
            resourceName,
            [](napi_env env, void *data) {},
            [](napi_env env, napi_status status, void *data) {
                AsyncGetBundleInstallerCallbackInfo *asyncCallbackInfo = (AsyncGetBundleInstallerCallbackInfo *)data;
                napi_value result[ARGS_SIZE_TWO] = {0};
                napi_value callback = 0;
                napi_value undefined = 0;
                napi_value callResult = 0;
                napi_value m_classBundleInstaller = nullptr;
                napi_get_reference_value(env, g_classBundleInstaller, &m_classBundleInstaller);
                napi_get_undefined(env, &undefined);
                napi_new_instance(env, m_classBundleInstaller, 0, nullptr, &result[PARAM1]);
                result[PARAM0] = GetCallbackErrorValue(env, CODE_SUCCESS);
                napi_get_reference_value(env, asyncCallbackInfo->callback, &callback);
                napi_call_function(env, undefined, callback, ARGS_SIZE_TWO, &result[PARAM0], &callResult);
                if (asyncCallbackInfo->callback != nullptr) {
                    napi_delete_reference(env, asyncCallbackInfo->callback);
                }
                napi_delete_async_work(env, asyncCallbackInfo->asyncWork);
                delete asyncCallbackInfo;
                asyncCallbackInfo = nullptr;
            },
            (void *)asyncCallbackInfo,
            &asyncCallbackInfo->asyncWork);
        NAPI_CALL(env, napi_queue_async_work(env, asyncCallbackInfo->asyncWork));
        napi_value ret = nullptr;
        NAPI_CALL(env, napi_get_null(env, &ret));
        if (ret == nullptr) {
            if (asyncCallbackInfo != nullptr) {
                delete asyncCallbackInfo;
                asyncCallbackInfo = nullptr;
            }
        }
        napi_value result;
        NAPI_CALL(env, napi_create_int32(env, NAPI_RETURN_ONE, &result));
        return result;
    } else {
        napi_deferred deferred;
        napi_value promise;
        NAPI_CALL(env, napi_create_promise(env, &deferred, &promise));
        asyncCallbackInfo->deferred = deferred;

        napi_value resourceName;
        napi_create_string_latin1(env, "GetBundleInstaller", NAPI_AUTO_LENGTH, &resourceName);
        napi_create_async_work(env,
            nullptr,
            resourceName,
            [](napi_env env, void *data) {},
            [](napi_env env, napi_status status, void *data) {
                HILOG_INFO("=================load=================");
                AsyncGetBundleInstallerCallbackInfo *asyncCallbackInfo = (AsyncGetBundleInstallerCallbackInfo *)data;
                napi_value result;
                napi_value m_classBundleInstaller = nullptr;
                napi_get_reference_value(env, g_classBundleInstaller, &m_classBundleInstaller);
                napi_new_instance(env, m_classBundleInstaller, 0, nullptr, &result);
                napi_resolve_deferred(asyncCallbackInfo->env, asyncCallbackInfo->deferred, result);
                napi_delete_async_work(env, asyncCallbackInfo->asyncWork);
                delete asyncCallbackInfo;
                asyncCallbackInfo = nullptr;
            },
            (void *)asyncCallbackInfo,
            &asyncCallbackInfo->asyncWork);
        napi_queue_async_work(env, asyncCallbackInfo->asyncWork);

        napi_value ret = nullptr;
        NAPI_CALL(env, napi_get_null(env, &ret));
        if (ret == nullptr) {
            if (asyncCallbackInfo != nullptr) {
                delete asyncCallbackInfo;
                asyncCallbackInfo = nullptr;
            }
        }
        return promise;
    }
}

static napi_value ParseInstallParam(napi_env env, InstallParam &installParam, napi_value args)
{
    napi_status status;
    napi_valuetype valueType;
    NAPI_CALL(env, napi_typeof(env, args, &valueType));
    NAPI_ASSERT(env, valueType == napi_object, "param type mismatch!");
    HILOG_INFO("-----ParseInstallParam type-----");
    napi_value installProp = nullptr;
    status = napi_get_named_property(env, args, "param", &installProp);
    NAPI_ASSERT(env, status == napi_ok, "property name incorrect!");
    napi_typeof(env, installProp, &valueType);
    NAPI_ASSERT(env, valueType == napi_object, "property type mismatch!");
    HILOG_INFO("-----ParseInstallParam InstallParam-----");

    napi_value property = nullptr;
    status = napi_get_named_property(env, installProp, "userId", &property);
    NAPI_ASSERT(env, status == napi_ok, "property userId incorrect!");
    napi_typeof(env, property, &valueType);
    NAPI_ASSERT(env, valueType == napi_number, "property type mismatch!");
    int userId = 0;
    NAPI_CALL(env, napi_get_value_int32(env, property, &userId));
    installParam.userId = userId;
    HILOG_INFO("ParseInstallParam userId=%{public}d.", installParam.userId);

    property = nullptr;
    status = napi_get_named_property(env, installProp, "installFlag", &property);
    NAPI_ASSERT(env, status == napi_ok, "property installFlag incorrect!");
    napi_typeof(env, property, &valueType);
    NAPI_ASSERT(env, valueType == napi_number, "property type mismatch!");
    int installFlag = 0;
    NAPI_CALL(env, napi_get_value_int32(env, property, &installFlag));
    installParam.installFlag = static_cast<OHOS::AppExecFwk::InstallFlag>(installFlag);
    HILOG_INFO("ParseInstallParam installFlag=%{public}d.", installParam.installFlag);

    property = nullptr;
    status = napi_get_named_property(env, installProp, "isKeepData", &property);
    NAPI_ASSERT(env, status == napi_ok, "property isKeepData incorrect!");
    napi_typeof(env, property, &valueType);
    NAPI_ASSERT(env, valueType == napi_boolean, "property type mismatch!");
    bool isKeepData = false;
    NAPI_CALL(env, napi_get_value_bool(env, property, &isKeepData));
    installParam.isKeepData = isKeepData;
    HILOG_INFO("ParseInstallParam isKeepData=%{public}d.", installParam.isKeepData);
    // create result code
    napi_value result;
    status = napi_create_int32(env, NAPI_RETURN_ONE, &result);
    NAPI_ASSERT(env, status == napi_ok, "napi_create_int32 error!");
    return result;
}

static napi_value ParseStringArray(napi_env env, std::vector<std::string> &hapFiles, napi_value args)
{
    HILOG_INFO("ParseStringArray called");
    bool isArray = false;
    uint32_t arrayLength = 0;
    napi_value valueAry = 0;
    napi_valuetype valueAryType = napi_undefined;
    NAPI_CALL(env, napi_is_array(env, args, &isArray));
    NAPI_CALL(env, napi_get_array_length(env, args, &arrayLength));
    HILOG_INFO("ParseStringArray args is array, length=%{public}ud", arrayLength);

    for (uint32_t j = 0; j < arrayLength; j++) {
        NAPI_CALL(env, napi_get_element(env, args, j, &valueAry));
        NAPI_CALL(env, napi_typeof(env, valueAry, &valueAryType));

        napi_valuetype valuetype;
        NAPI_CALL(env, napi_typeof(env, valueAry, &valuetype));
        NAPI_ASSERT(env, valuetype == napi_string, "Wrong argument type. String expected.");
        hapFiles.push_back(GetStringFromNAPI(env, valueAry));
    }
    // create result code
    napi_value result;
    napi_status status;
    status = napi_create_int32(env, NAPI_RETURN_ONE, &result);
    NAPI_ASSERT(env, status == napi_ok, "napi_create_int32 error!");
    return result;
}

static void ConvertInstallResult(InstallResult &installResult)
{
    HILOG_INFO("ConvertInstallResult = %{public}s.", installResult.resultMsg.c_str());
    switch (installResult.resultCode) {
        case static_cast<int32_t>(IStatusReceiver::SUCCESS):
            installResult.resultCode = static_cast<int32_t>(InstallErrorCode::SUCCESS);
            installResult.resultMsg = "SUCCESS";
            break;
        case static_cast<int32_t>(IStatusReceiver::ERR_INSTALL_INTERNAL_ERROR):
        case static_cast<int32_t>(IStatusReceiver::ERR_INSTALL_HOST_INSTALLER_FAILED):
            installResult.resultCode = static_cast<int32_t>(InstallErrorCode::STATUS_INSTALL_FAILURE);
            installResult.resultMsg = "STATUS_INSTALL_FAILURE";
            break;
        case static_cast<int32_t>(IStatusReceiver::ERR_INSTALL_PARSE_FAILED):
        case static_cast<int32_t>(IStatusReceiver::ERR_INSTALL_VERIFICATION_FAILED):
        case static_cast<int32_t>(IStatusReceiver::ERR_INSTALL_NO_SIGNATURE_INFO):
        case static_cast<int32_t>(IStatusReceiver::ERR_INSTALL_PARAM_ERROR):
        case static_cast<int32_t>(IStatusReceiver::ERR_INSTALL_FILE_PATH_INVALID):
        case static_cast<int32_t>(IStatusReceiver::ERR_INSTALL_INVALID_HAP_NAME):
        case static_cast<int32_t>(IStatusReceiver::ERR_INSTALL_INVALID_BUNDLE_FILE):
        case static_cast<int32_t>(IStatusReceiver::ERR_INSTALL_PARSE_UNEXPECTED):
        case static_cast<int32_t>(IStatusReceiver::ERR_INSTALL_PARSE_MISSING_BUNDLE):
        case static_cast<int32_t>(IStatusReceiver::ERR_INSTALL_PARSE_NO_PROFILE):
        case static_cast<int32_t>(IStatusReceiver::ERR_INSTALL_PARSE_BAD_PROFILE):
        case static_cast<int32_t>(IStatusReceiver::ERR_INSTALL_PARSE_PROFILE_PROP_TYPE_ERROR):
        case static_cast<int32_t>(IStatusReceiver::ERR_INSTALL_PARSE_PROFILE_MISSING_PROP):
        case static_cast<int32_t>(IStatusReceiver::ERR_INSTALL_PARSE_PERMISSION_ERROR):
            installResult.resultCode = static_cast<int32_t>(InstallErrorCode::STATUS_INSTALL_FAILURE_INVALID);
            installResult.resultMsg = "STATUS_INSTALL_FAILURE_INVALID";
            break;
        case static_cast<int32_t>(IStatusReceiver::ERR_INSTALL_PARSE_MISSING_ABILITY):
            installResult.resultCode = static_cast<int32_t>(InstallErrorCode::STATUS_ABILITY_NOT_FOUND);
            installResult.resultMsg = "STATUS_ABILITY_NOT_FOUND";
            break;
        case static_cast<int32_t>(IStatusReceiver::ERR_INSTALL_VERSION_DOWNGRADE):
        case static_cast<int32_t>(IStatusReceiver::ERR_INSTALL_UPDATE_INCOMPATIBLE):
            installResult.resultCode = static_cast<int32_t>(InstallErrorCode::STATUS_INSTALL_FAILURE_INCOMPATIBLE);
            installResult.resultMsg = "STATUS_INSTALL_FAILURE_INCOMPATIBLE";
            break;
        case static_cast<int32_t>(IStatusReceiver::ERR_INSTALL_PERMISSION_DENIED):
        case static_cast<int32_t>(IStatusReceiver::ERR_INSTALL_ENTRY_ALREADY_EXIST):
        case static_cast<int32_t>(IStatusReceiver::ERR_INSTALL_ALREADY_EXIST):
            installResult.resultCode = static_cast<int32_t>(InstallErrorCode::STATUS_INSTALL_FAILURE_CONFLICT);
            installResult.resultMsg = "STATUS_INSTALL_FAILURE_CONFLICT";
            break;
        case static_cast<int32_t>(IStatusReceiver::ERR_INSTALLD_PARAM_ERROR):
        case static_cast<int32_t>(IStatusReceiver::ERR_INSTALLD_GET_PROXY_ERROR):
        case static_cast<int32_t>(IStatusReceiver::ERR_INSTALLD_CREATE_DIR_FAILED):
        case static_cast<int32_t>(IStatusReceiver::ERR_INSTALLD_CREATE_DIR_EXIST):
        case static_cast<int32_t>(IStatusReceiver::ERR_INSTALLD_CHOWN_FAILED):
        case static_cast<int32_t>(IStatusReceiver::ERR_INSTALLD_REMOVE_DIR_FAILED):
        case static_cast<int32_t>(IStatusReceiver::ERR_INSTALLD_EXTRACT_FILES_FAILED):
        case static_cast<int32_t>(IStatusReceiver::ERR_INSTALLD_RNAME_DIR_FAILED):
        case static_cast<int32_t>(IStatusReceiver::ERR_INSTALLD_CLEAN_DIR_FAILED):
        case static_cast<int32_t>(IStatusReceiver::ERR_INSTALL_STATE_ERROR):
        case static_cast<int32_t>(IStatusReceiver::ERR_INSTALL_GENERATE_UID_ERROR):
        case static_cast<int32_t>(IStatusReceiver::ERR_INSTALL_INSTALLD_SERVICE_ERROR):
            installResult.resultCode = static_cast<int32_t>(InstallErrorCode::STATUS_INSTALL_FAILURE_STORAGE);
            installResult.resultMsg = "STATUS_INSTALL_FAILURE_STORAGE";
            break;
        case static_cast<int32_t>(IStatusReceiver::ERR_UNINSTALL_INVALID_NAME):
        case static_cast<int32_t>(IStatusReceiver::ERR_UNINSTALL_PARAM_ERROR):
        case static_cast<int32_t>(IStatusReceiver::ERR_UNINSTALL_PERMISSION_DENIED):
            if (CheckIsSystemApp()) {
                installResult.resultCode = static_cast<int32_t>(InstallErrorCode::STATUS_UNINSTALL_FAILURE_ABORTED);
                installResult.resultMsg = "STATUS_UNINSTALL_FAILURE_ABORTED";
                break;
            }
        case static_cast<int32_t>(IStatusReceiver::ERR_UNINSTALL_SYSTEM_APP_ERROR):
        case static_cast<int32_t>(IStatusReceiver::ERR_UNINSTALL_KILLING_APP_ERROR):
            if (CheckIsSystemApp()) {
                installResult.resultCode = static_cast<int32_t>(InstallErrorCode::STATUS_UNINSTALL_FAILURE_CONFLICT);
                installResult.resultMsg = "STATUS_UNINSTALL_FAILURE_CONFLICT";
                break;
            }
        case static_cast<int32_t>(IStatusReceiver::ERR_UNINSTALL_BUNDLE_MGR_SERVICE_ERROR):
        case static_cast<int32_t>(IStatusReceiver::ERR_UNINSTALL_MISSING_INSTALLED_BUNDLE):
        case static_cast<int32_t>(IStatusReceiver::ERR_UNINSTALL_MISSING_INSTALLED_MODULE):
            installResult.resultCode = static_cast<int32_t>(InstallErrorCode::STATUS_UNINSTALL_FAILURE);
            installResult.resultMsg = "STATUS_UNINSTALL_FAILURE";
            break;
        default:
            installResult.resultCode = static_cast<int32_t>(InstallErrorCode::STATUS_BMS_SERVICE_ERROR);
            installResult.resultMsg = "STATUS_BMS_SERVICE_ERROR";
            break;
    }
}

/**
 * Promise and async callback
 */
napi_value Install(napi_env env, napi_callback_info info)
{
    HILOG_INFO("Install called");
    size_t argc = ARGS_SIZE_THREE;
    napi_value argv[ARGS_SIZE_THREE] = {nullptr};
    NAPI_CALL(env, napi_get_cb_info(env, info, &argc, argv, NULL, NULL));
    HILOG_INFO("argc = [%{public}zu]", argc);
    std::vector<std::string> bundleFilePaths;
    ParseStringArray(env, bundleFilePaths, argv[PARAM0]);
    InstallParam installParam;
    ParseInstallParam(env, installParam, argv[PARAM1]);
    AsyncInstallCallbackInfo *asyncCallbackInfo = new (std::nothrow) AsyncInstallCallbackInfo {
        .env = env,
        .asyncWork = nullptr,
        .deferred = nullptr,
        .hapFiles = bundleFilePaths,
        .installParam = installParam,
    };
    if (asyncCallbackInfo == nullptr) {
        return nullptr;
    }
    if (argc > (ARGS_SIZE_THREE - CALLBACK_SIZE)) {
        HILOG_INFO("Install asyncCallback.");
        napi_valuetype valuetype = napi_undefined;
        NAPI_CALL(env, napi_typeof(env, argv[ARGS_SIZE_TWO], &valuetype));
        NAPI_ASSERT(env, valuetype == napi_function, "Wrong argument type. Function expected.");
        napi_create_reference(env, argv[ARGS_SIZE_TWO], NAPI_RETURN_ONE, &asyncCallbackInfo->callback);

        napi_value resourceName;
        napi_create_string_latin1(env, "Install", NAPI_AUTO_LENGTH, &resourceName);
        napi_create_async_work(env,
            nullptr,
            resourceName,
            [](napi_env env, void *data) {
                AsyncInstallCallbackInfo *asyncCallbackInfo = (AsyncInstallCallbackInfo *)data;
                InnerInstall(env,
                    asyncCallbackInfo->hapFiles,
                    asyncCallbackInfo->installParam,
                    asyncCallbackInfo->installResult);
            },
            [](napi_env env, napi_status status, void *data) {
                AsyncInstallCallbackInfo *asyncCallbackInfo = (AsyncInstallCallbackInfo *)data;
                napi_value result[ARGS_SIZE_TWO] = {0};
                napi_value callback = 0;
                napi_value undefined = 0;
                napi_value callResult = 0;
                napi_create_object(env, &result[PARAM1]);
                ConvertInstallResult(asyncCallbackInfo->installResult);
                napi_value nResultMsg;
                napi_create_string_utf8(
                    env, asyncCallbackInfo->installResult.resultMsg.c_str(), NAPI_AUTO_LENGTH, &nResultMsg);
                napi_set_named_property(env, result[PARAM1], "statusMessage", nResultMsg);
                napi_value nResultCode;
                napi_create_int32(env, asyncCallbackInfo->installResult.resultCode, &nResultCode);
                napi_set_named_property(env, result[PARAM1], "status", nResultCode);
                result[PARAM0] = GetCallbackErrorValue(
                    env, (asyncCallbackInfo->installResult.resultCode == 0) ? CODE_SUCCESS : CODE_FAILED);
                napi_get_undefined(env, &undefined);
                napi_get_reference_value(env, asyncCallbackInfo->callback, &callback);
                napi_call_function(env, undefined, callback, ARGS_SIZE_TWO, &result[PARAM0], &callResult);

                if (asyncCallbackInfo->callback != nullptr) {
                    napi_delete_reference(env, asyncCallbackInfo->callback);
                }
                napi_delete_async_work(env, asyncCallbackInfo->asyncWork);
                delete asyncCallbackInfo;
                asyncCallbackInfo = nullptr;
            },
            (void *)asyncCallbackInfo,
            &asyncCallbackInfo->asyncWork);
        NAPI_CALL(env, napi_queue_async_work(env, asyncCallbackInfo->asyncWork));
        napi_value ret = nullptr;
        NAPI_CALL(env, napi_get_null(env, &ret));
        if (ret == nullptr) {
            if (asyncCallbackInfo != nullptr) {
                delete asyncCallbackInfo;
                asyncCallbackInfo = nullptr;
            }
        }
        napi_value result;
        NAPI_CALL(env, napi_create_int32(env, NAPI_RETURN_ONE, &result));
        return result;
    } else {
        napi_deferred deferred;
        napi_value promise;
        NAPI_CALL(env, napi_create_promise(env, &deferred, &promise));
        asyncCallbackInfo->deferred = deferred;

        napi_value resourceName;
        napi_create_string_latin1(env, "Install", NAPI_AUTO_LENGTH, &resourceName);
        napi_create_async_work(env,
            nullptr,
            resourceName,
            [](napi_env env, void *data) {
                AsyncInstallCallbackInfo *asyncCallbackInfo = (AsyncInstallCallbackInfo *)data;
                InnerInstall(env,
                    asyncCallbackInfo->hapFiles,
                    asyncCallbackInfo->installParam,
                    asyncCallbackInfo->installResult);
            },
            [](napi_env env, napi_status status, void *data) {
                HILOG_INFO("=================load=================");
                AsyncInstallCallbackInfo *asyncCallbackInfo = (AsyncInstallCallbackInfo *)data;
                ConvertInstallResult(asyncCallbackInfo->installResult);
                napi_value result;
                napi_create_object(env, &result);
                napi_value nResultMsg;
                napi_create_string_utf8(
                    env, asyncCallbackInfo->installResult.resultMsg.c_str(), NAPI_AUTO_LENGTH, &nResultMsg);
                napi_set_named_property(env, result, "statusMessage", nResultMsg);
                napi_value nResultCode;
                napi_create_int32(env, asyncCallbackInfo->installResult.resultCode, &nResultCode);
                napi_set_named_property(env, result, "status", nResultCode);
                napi_resolve_deferred(asyncCallbackInfo->env, asyncCallbackInfo->deferred, result);
                napi_delete_async_work(env, asyncCallbackInfo->asyncWork);
                delete asyncCallbackInfo;
                asyncCallbackInfo = nullptr;
            },
            (void *)asyncCallbackInfo,
            &asyncCallbackInfo->asyncWork);
        napi_queue_async_work(env, asyncCallbackInfo->asyncWork);

        napi_value ret = nullptr;
        NAPI_CALL(env, napi_get_null(env, &ret));
        if (ret == nullptr) {
            if (asyncCallbackInfo != nullptr) {
                delete asyncCallbackInfo;
                asyncCallbackInfo = nullptr;
            }
        }
        return promise;
    }
}

static void InnerUninstall(
    napi_env env, const std::string &bundleName, InstallParam &installParam, InstallResult &installResult)
{
    auto iBundleMgr = GetBundleMgr();
    if (!iBundleMgr) {
        HILOG_ERROR("can not get iBundleMgr");
        return;
    }
    auto iBundleInstaller = iBundleMgr->GetBundleInstaller();
    if (!iBundleInstaller) {
        HILOG_ERROR("can not get iBundleInstaller");
        return;
    }
    installParam.installFlag = InstallFlag::NORMAL;
    OHOS::sptr<InstallerCallback> callback = new InstallerCallback();
    if (!callback) {
        HILOG_ERROR("callback nullptr");
        return;
    }
    iBundleInstaller->Uninstall(bundleName, installParam, callback);
    installResult.resultMsg = callback->GetResultMsg();
    HILOG_INFO("-----InnerUninstall resultMsg %{public}s-----", installResult.resultMsg.c_str());
    installResult.resultCode = callback->GetResultCode();
    HILOG_INFO("-----InnerUninstall resultCode %{public}d-----", installResult.resultCode);
}
/**
 * Promise and async callback
 */
napi_value Uninstall(napi_env env, napi_callback_info info)
{
    HILOG_INFO("Uninstall called");
    size_t argc = ARGS_SIZE_THREE;
    napi_value argv[ARGS_SIZE_THREE] = {nullptr};
    NAPI_CALL(env, napi_get_cb_info(env, info, &argc, argv, NULL, NULL));
    HILOG_INFO("argc = [%{public}zu]", argc);
    std::string bundleName;
    ParseString(env, bundleName, argv[PARAM0]);
    InstallParam installParam;
    ParseInstallParam(env, installParam, argv[PARAM1]);
    AsyncInstallCallbackInfo *asyncCallbackInfo = new (std::nothrow) AsyncInstallCallbackInfo {
        .env = env,
        .asyncWork = nullptr,
        .deferred = nullptr,
        .param = bundleName,
        .installParam = installParam,
    };
    if (asyncCallbackInfo == nullptr) {
        return nullptr;
    }
    if (argc > (ARGS_SIZE_THREE - CALLBACK_SIZE)) {
        HILOG_INFO("Uninstall asyncCallback.");
        napi_valuetype valuetype = napi_undefined;
        NAPI_CALL(env, napi_typeof(env, argv[ARGS_SIZE_TWO], &valuetype));
        NAPI_ASSERT(env, valuetype == napi_function, "Wrong argument type. Function expected.");
        napi_create_reference(env, argv[ARGS_SIZE_TWO], NAPI_RETURN_ONE, &asyncCallbackInfo->callback);

        napi_value resourceName;
        napi_create_string_latin1(env, "Uninstall", NAPI_AUTO_LENGTH, &resourceName);
        napi_create_async_work(env,
            nullptr,
            resourceName,
            [](napi_env env, void *data) {
                AsyncInstallCallbackInfo *asyncCallbackInfo = (AsyncInstallCallbackInfo *)data;
                InnerUninstall(
                    env, asyncCallbackInfo->param, asyncCallbackInfo->installParam, asyncCallbackInfo->installResult);
            },
            [](napi_env env, napi_status status, void *data) {
                AsyncInstallCallbackInfo *asyncCallbackInfo = (AsyncInstallCallbackInfo *)data;
                napi_value result[ARGS_SIZE_TWO] = {0};
                napi_value callback = 0;
                napi_value undefined = 0;
                napi_value callResult = 0;
                napi_create_object(env, &result[PARAM1]);
                ConvertInstallResult(asyncCallbackInfo->installResult);
                napi_value nResultMsg;
                napi_create_string_utf8(
                    env, asyncCallbackInfo->installResult.resultMsg.c_str(), NAPI_AUTO_LENGTH, &nResultMsg);
                napi_set_named_property(env, result[PARAM1], "statusMessage", nResultMsg);
                napi_value nResultCode;
                napi_create_int32(env, asyncCallbackInfo->installResult.resultCode, &nResultCode);
                napi_set_named_property(env, result[PARAM1], "status", nResultCode);
                result[PARAM0] = GetCallbackErrorValue(
                    env, (asyncCallbackInfo->installResult.resultCode == 0) ? CODE_SUCCESS : CODE_FAILED);
                napi_get_undefined(env, &undefined);
                napi_get_reference_value(env, asyncCallbackInfo->callback, &callback);
                napi_call_function(env, undefined, callback, ARGS_SIZE_TWO, &result[PARAM0], &callResult);

                if (asyncCallbackInfo->callback != nullptr) {
                    napi_delete_reference(env, asyncCallbackInfo->callback);
                }
                napi_delete_async_work(env, asyncCallbackInfo->asyncWork);
                delete asyncCallbackInfo;
                asyncCallbackInfo = nullptr;
            },
            (void *)asyncCallbackInfo,
            &asyncCallbackInfo->asyncWork);
        NAPI_CALL(env, napi_queue_async_work(env, asyncCallbackInfo->asyncWork));

        napi_value ret = nullptr;
        NAPI_CALL(env, napi_get_null(env, &ret));
        if (ret == nullptr) {
            if (asyncCallbackInfo != nullptr) {
                delete asyncCallbackInfo;
                asyncCallbackInfo = nullptr;
            }
        }
        napi_value result;
        NAPI_CALL(env, napi_create_int32(env, NAPI_RETURN_ONE, &result));
        return result;
    } else {
        napi_deferred deferred;
        napi_value promise;
        NAPI_CALL(env, napi_create_promise(env, &deferred, &promise));
        asyncCallbackInfo->deferred = deferred;

        napi_value resourceName;
        napi_create_string_latin1(env, "Install", NAPI_AUTO_LENGTH, &resourceName);
        napi_create_async_work(env,
            nullptr,
            resourceName,
            [](napi_env env, void *data) {
                AsyncInstallCallbackInfo *asyncCallbackInfo = (AsyncInstallCallbackInfo *)data;
                InnerUninstall(
                    env, asyncCallbackInfo->param, asyncCallbackInfo->installParam, asyncCallbackInfo->installResult);
            },
            [](napi_env env, napi_status status, void *data) {
                HILOG_INFO("=================load=================");
                AsyncInstallCallbackInfo *asyncCallbackInfo = (AsyncInstallCallbackInfo *)data;
                ConvertInstallResult(asyncCallbackInfo->installResult);
                napi_value result;
                napi_create_object(env, &result);
                napi_value nResultMsg;
                napi_create_string_utf8(
                    env, asyncCallbackInfo->installResult.resultMsg.c_str(), NAPI_AUTO_LENGTH, &nResultMsg);
                napi_set_named_property(env, result, "statusMessage", nResultMsg);
                napi_value nResultCode;
                napi_create_int32(env, asyncCallbackInfo->installResult.resultCode, &nResultCode);
                napi_set_named_property(env, result, "status", nResultCode);
                napi_resolve_deferred(asyncCallbackInfo->env, asyncCallbackInfo->deferred, result);
                napi_delete_async_work(env, asyncCallbackInfo->asyncWork);
                delete asyncCallbackInfo;
                asyncCallbackInfo = nullptr;
            },
            (void *)asyncCallbackInfo,
            &asyncCallbackInfo->asyncWork);
        napi_queue_async_work(env, asyncCallbackInfo->asyncWork);

        napi_value ret = nullptr;
        NAPI_CALL(env, napi_get_null(env, &ret));
        if (ret == nullptr) {
            if (asyncCallbackInfo != nullptr) {
                delete asyncCallbackInfo;
                asyncCallbackInfo = nullptr;
            }
        }
        return promise;
    }
}

napi_value BundleInstallerConstructor(napi_env env, napi_callback_info info)
{
    napi_value jsthis = nullptr;
    NAPI_CALL(env, napi_get_cb_info(env, info, nullptr, nullptr, &jsthis, nullptr));
    return jsthis;
}

static bool InnerGetAllFormsInfo(napi_env env, std::vector<OHOS::AppExecFwk::FormInfo> &formInfos)
{
    auto iBundleMgr = GetBundleMgr();
    if (!iBundleMgr) {
        HILOG_ERROR("can not get iBundleMgr");
        return false;
    }
    return iBundleMgr->GetAllFormsInfo(formInfos);
}

static void ProcessFormsInfo(napi_env env, napi_value result, const std::vector<OHOS::AppExecFwk::FormInfo> &formInfos)
{
    if (formInfos.size() > 0) {
        HILOG_INFO("-----formInfos is not null-----");
        size_t index = 0;
        for (const auto &item : formInfos) {
            HILOG_INFO("name{%s} ", item.name.c_str());
            HILOG_INFO("bundleName{%s} ", item.bundleName.c_str());
            napi_value objFormInfo;
            NAPI_CALL_RETURN_VOID(env, napi_create_object(env, &objFormInfo));
            ConvertFormInfo(env, objFormInfo, item);
            NAPI_CALL_RETURN_VOID(env, napi_set_element(env, result, index, objFormInfo));
            index++;
        }
    } else {
        HILOG_INFO("-----formInfos is null-----");
    }
}
/**
 * Promise and async callback
 */
napi_value GetAllFormsInfo(napi_env env, napi_callback_info info)
{
    size_t argc = ARGS_SIZE_ONE;
    napi_value argv[ARGS_SIZE_ONE] = {nullptr};
    napi_value thisArg;
    void *data = nullptr;
    NAPI_CALL(env, napi_get_cb_info(env, info, &argc, argv, &thisArg, &data));
    HILOG_INFO("ARGCSIZE is =%{public}zu.", argc);

    AsyncFormInfosCallbackInfo *asyncCallbackInfo =
        new AsyncFormInfosCallbackInfo {.env = env, .asyncWork = nullptr, .deferred = nullptr};
    if (asyncCallbackInfo == nullptr) {
        return nullptr;
    }
    if (argc > (ARGS_SIZE_ONE - CALLBACK_SIZE)) {
        HILOG_INFO("GetAllFormsInfo asyncCallback.");
        napi_value resourceName;
        NAPI_CALL(env, napi_create_string_latin1(env, "GetAllFormsInfo", NAPI_AUTO_LENGTH, &resourceName));
        napi_valuetype valuetype = napi_undefined;
        napi_typeof(env, argv[PARAM0], &valuetype);
        NAPI_ASSERT(env, valuetype == napi_function, "Wrong argument type. Function expected.");
        NAPI_CALL(env, napi_create_reference(env, argv[PARAM0], NAPI_RETURN_ONE, &asyncCallbackInfo->callback));

        napi_create_async_work(env,
            nullptr,
            resourceName,
            [](napi_env env, void *data) {
                AsyncFormInfosCallbackInfo *asyncCallbackInfo = (AsyncFormInfosCallbackInfo *)data;
                asyncCallbackInfo->ret = InnerGetAllFormsInfo(env, asyncCallbackInfo->formInfos);
            },
            [](napi_env env, napi_status status, void *data) {
                AsyncFormInfosCallbackInfo *asyncCallbackInfo = (AsyncFormInfosCallbackInfo *)data;
                napi_value result[ARGS_SIZE_TWO] = {0};
                napi_value callback = 0;
                napi_value undefined = 0;
                napi_value callResult = 0;
                napi_get_undefined(env, &undefined);
                napi_create_array(env, &result[PARAM1]);
                ProcessFormsInfo(env, result[PARAM1], asyncCallbackInfo->formInfos);
                result[PARAM0] = GetCallbackErrorValue(env, asyncCallbackInfo->ret ? CODE_SUCCESS : CODE_FAILED);
                napi_get_reference_value(env, asyncCallbackInfo->callback, &callback);
                napi_call_function(env, undefined, callback, ARGS_SIZE_TWO, &result[PARAM0], &callResult);

                if (asyncCallbackInfo->callback != nullptr) {
                    napi_delete_reference(env, asyncCallbackInfo->callback);
                }
                napi_delete_async_work(env, asyncCallbackInfo->asyncWork);
                delete asyncCallbackInfo;
                asyncCallbackInfo = nullptr;
            },
            (void *)asyncCallbackInfo,
            &asyncCallbackInfo->asyncWork);
        NAPI_CALL(env, napi_queue_async_work(env, asyncCallbackInfo->asyncWork));

        napi_value ret = nullptr;
        NAPI_CALL(env, napi_get_null(env, &ret));
        if (ret == nullptr) {
            if (asyncCallbackInfo != nullptr) {
                delete asyncCallbackInfo;
                asyncCallbackInfo = nullptr;
            }
        }
        napi_value result;
        NAPI_CALL(env, napi_create_int32(env, NAPI_RETURN_ONE, &result));
        return result;
    } else {
        HILOG_INFO("GetFormInfos promise.");
        napi_deferred deferred;
        napi_value promise;
        NAPI_CALL(env, napi_create_promise(env, &deferred, &promise));
        asyncCallbackInfo->deferred = deferred;

        napi_value resourceName;
        napi_create_string_latin1(env, "GetFormInfos", NAPI_AUTO_LENGTH, &resourceName);
        napi_create_async_work(env,
            nullptr,
            resourceName,
            [](napi_env env, void *data) {
                AsyncFormInfosCallbackInfo *asyncCallbackInfo = (AsyncFormInfosCallbackInfo *)data;
                InnerGetAllFormsInfo(env, asyncCallbackInfo->formInfos);
            },
            [](napi_env env, napi_status status, void *data) {
                HILOG_INFO("=================load=================");
                AsyncFormInfosCallbackInfo *asyncCallbackInfo = (AsyncFormInfosCallbackInfo *)data;
                napi_value result;
                napi_create_array(env, &result);
                ProcessFormsInfo(env, result, asyncCallbackInfo->formInfos);
                napi_resolve_deferred(asyncCallbackInfo->env, asyncCallbackInfo->deferred, result);
                napi_delete_async_work(env, asyncCallbackInfo->asyncWork);
                delete asyncCallbackInfo;
                asyncCallbackInfo = nullptr;
            },
            (void *)asyncCallbackInfo,
            &asyncCallbackInfo->asyncWork);
        napi_queue_async_work(env, asyncCallbackInfo->asyncWork);

        napi_value ret = nullptr;
        NAPI_CALL(env, napi_get_null(env, &ret));
        if (ret == nullptr) {
            if (asyncCallbackInfo != nullptr) {
                delete asyncCallbackInfo;
                asyncCallbackInfo = nullptr;
            }
        }
        return promise;
    }
}

static bool InnerGetFormInfosByModule(napi_env env, const std::string &bundleName, const std::string &moduleName,
    std::vector<OHOS::AppExecFwk::FormInfo> &formInfos)
{
    auto iBundleMgr = GetBundleMgr();
    if (!iBundleMgr) {
        HILOG_ERROR("can not get iBundleMgr");
        return false;
    }
    return iBundleMgr->GetFormsInfoByModule(bundleName, moduleName, formInfos);
}
/**
 * Promise and async callback
 */
napi_value GetFormsInfoByModule(napi_env env, napi_callback_info info)
{
    size_t argc = ARGS_SIZE_THREE;
    napi_value argv[ARGS_SIZE_THREE] = {nullptr};
    napi_value thisArg;
    void *data = nullptr;
    NAPI_CALL(env, napi_get_cb_info(env, info, &argc, argv, &thisArg, &data));
    HILOG_INFO("ARGCSIZE is =%{public}zu.", argc);
    std::string bundleName;
    std::string moduleName;
    ParseString(env, bundleName, argv[PARAM0]);
    ParseString(env, moduleName, argv[PARAM1]);

    AsyncFormInfosByModuleCallbackInfo *asyncCallbackInfo = new (std::nothrow) AsyncFormInfosByModuleCallbackInfo {
        .env = env, .asyncWork = nullptr, .deferred = nullptr, .bundleName = bundleName, .moduleName = moduleName};
    if (asyncCallbackInfo == nullptr) {
        return nullptr;
    }
    if (argc > (ARGS_SIZE_THREE - CALLBACK_SIZE)) {
        HILOG_INFO("GetFormsInfoByModule asyncCallback.");
        napi_value resourceName;
        NAPI_CALL(env, napi_create_string_latin1(env, "GetFormsInfoByModule", NAPI_AUTO_LENGTH, &resourceName));
        napi_valuetype valuetype = napi_undefined;
        napi_typeof(env, argv[ARGS_SIZE_TWO], &valuetype);
        NAPI_ASSERT(env, valuetype == napi_function, "Wrong argument type. Function expected.");
        NAPI_CALL(env, napi_create_reference(env, argv[ARGS_SIZE_TWO], NAPI_RETURN_ONE, &asyncCallbackInfo->callback));

        napi_create_async_work(env,
            nullptr,
            resourceName,
            [](napi_env env, void *data) {
                AsyncFormInfosByModuleCallbackInfo *asyncCallbackInfo = (AsyncFormInfosByModuleCallbackInfo *)data;
                asyncCallbackInfo->ret = InnerGetFormInfosByModule(
                    env, asyncCallbackInfo->bundleName, asyncCallbackInfo->moduleName, asyncCallbackInfo->formInfos);
            },
            [](napi_env env, napi_status status, void *data) {
                AsyncFormInfosByModuleCallbackInfo *asyncCallbackInfo = (AsyncFormInfosByModuleCallbackInfo *)data;
                napi_value result[ARGS_SIZE_TWO] = {0};
                napi_value callback = 0;
                napi_value undefined = 0;
                napi_value callResult = 0;
                napi_get_undefined(env, &undefined);
                napi_create_array(env, &result[PARAM1]);
                ProcessFormsInfo(env, result[PARAM1], asyncCallbackInfo->formInfos);
                result[PARAM0] = GetCallbackErrorValue(env, asyncCallbackInfo->ret ? CODE_SUCCESS : CODE_FAILED);
                napi_get_reference_value(env, asyncCallbackInfo->callback, &callback);
                napi_call_function(env, undefined, callback, ARGS_SIZE_TWO, &result[PARAM0], &callResult);

                if (asyncCallbackInfo->callback != nullptr) {
                    napi_delete_reference(env, asyncCallbackInfo->callback);
                }
                napi_delete_async_work(env, asyncCallbackInfo->asyncWork);
                delete asyncCallbackInfo;
                asyncCallbackInfo = nullptr;
            },
            (void *)asyncCallbackInfo,
            &asyncCallbackInfo->asyncWork);
        NAPI_CALL(env, napi_queue_async_work(env, asyncCallbackInfo->asyncWork));
        napi_value ret = nullptr;
        NAPI_CALL(env, napi_get_null(env, &ret));
        if (ret == nullptr) {
            if (asyncCallbackInfo != nullptr) {
                delete asyncCallbackInfo;
                asyncCallbackInfo = nullptr;
            }
        }
        napi_value result;
        NAPI_CALL(env, napi_create_int32(env, NAPI_RETURN_ONE, &result));
        return result;
    } else {
        HILOG_INFO("GetFormsInfoByModule promise.");
        napi_deferred deferred;
        napi_value promise;
        NAPI_CALL(env, napi_create_promise(env, &deferred, &promise));
        asyncCallbackInfo->deferred = deferred;

        napi_value resourceName;
        napi_create_string_latin1(env, "GetFormsInfoByModule", NAPI_AUTO_LENGTH, &resourceName);
        napi_create_async_work(env,
            nullptr,
            resourceName,
            [](napi_env env, void *data) {
                AsyncFormInfosByModuleCallbackInfo *asyncCallbackInfo = (AsyncFormInfosByModuleCallbackInfo *)data;
                InnerGetFormInfosByModule(
                    env, asyncCallbackInfo->bundleName, asyncCallbackInfo->moduleName, asyncCallbackInfo->formInfos);
            },
            [](napi_env env, napi_status status, void *data) {
                HILOG_INFO("=================load=================");
                AsyncFormInfosByModuleCallbackInfo *asyncCallbackInfo = (AsyncFormInfosByModuleCallbackInfo *)data;
                napi_value result;
                napi_create_array(env, &result);
                ProcessFormsInfo(env, result, asyncCallbackInfo->formInfos);
                napi_resolve_deferred(asyncCallbackInfo->env, asyncCallbackInfo->deferred, result);
                napi_delete_async_work(env, asyncCallbackInfo->asyncWork);
                delete asyncCallbackInfo;
                asyncCallbackInfo = nullptr;
            },
            (void *)asyncCallbackInfo,
            &asyncCallbackInfo->asyncWork);
        napi_queue_async_work(env, asyncCallbackInfo->asyncWork);

        napi_value ret = nullptr;
        NAPI_CALL(env, napi_get_null(env, &ret));
        if (ret == nullptr) {
            if (asyncCallbackInfo != nullptr) {
                delete asyncCallbackInfo;
                asyncCallbackInfo = nullptr;
            }
        }
        return promise;
    }
}

static bool InnerGetFormInfosByApp(
    napi_env env, const std::string &bundleName, std::vector<OHOS::AppExecFwk::FormInfo> &formInfos)
{
    auto iBundleMgr = GetBundleMgr();
    if (!iBundleMgr) {
        HILOG_ERROR("can not get iBundleMgr");
        return false;
    }
    return iBundleMgr->GetFormsInfoByApp(bundleName, formInfos);
}
/**
 * Promise and async callback
 */
napi_value GetFormsInfoByApp(napi_env env, napi_callback_info info)
{
    size_t argc = ARGS_SIZE_TWO;
    napi_value argv[ARGS_SIZE_TWO] = {nullptr};
    napi_value thisArg;
    void *data = nullptr;
    NAPI_CALL(env, napi_get_cb_info(env, info, &argc, argv, &thisArg, &data));
    HILOG_INFO("ARGCSIZE is =%{public}zu.", argc);
    std::string bundleName;
    ParseString(env, bundleName, argv[PARAM0]);

    AsyncFormInfosByAppCallbackInfo *asyncCallbackInfo = new (std::nothrow) AsyncFormInfosByAppCallbackInfo {
        .env = env, .asyncWork = nullptr, .deferred = nullptr, .bundleName = bundleName};
    if (asyncCallbackInfo == nullptr) {
        return nullptr;
    }
    if (argc > (ARGS_SIZE_TWO - CALLBACK_SIZE)) {
        HILOG_INFO("GetFormsInfoByApp asyncCallback.");
        napi_value resourceName;
        NAPI_CALL(env, napi_create_string_latin1(env, "GetFormsInfoByApp", NAPI_AUTO_LENGTH, &resourceName));
        napi_valuetype valuetype = napi_undefined;
        napi_typeof(env, argv[ARGS_SIZE_ONE], &valuetype);
        NAPI_ASSERT(env, valuetype == napi_function, "Wrong argument type. Function expected.");
        NAPI_CALL(env, napi_create_reference(env, argv[ARGS_SIZE_ONE], NAPI_RETURN_ONE, &asyncCallbackInfo->callback));

        napi_create_async_work(env,
            nullptr,
            resourceName,
            [](napi_env env, void *data) {
                AsyncFormInfosByAppCallbackInfo *asyncCallbackInfo = (AsyncFormInfosByAppCallbackInfo *)data;
                asyncCallbackInfo->ret =
                    InnerGetFormInfosByApp(env, asyncCallbackInfo->bundleName, asyncCallbackInfo->formInfos);
            },
            [](napi_env env, napi_status status, void *data) {
                AsyncFormInfosByAppCallbackInfo *asyncCallbackInfo = (AsyncFormInfosByAppCallbackInfo *)data;
                napi_value result[ARGS_SIZE_TWO] = {0};
                napi_value callback = 0;
                napi_value undefined = 0;
                napi_value callResult = 0;
                napi_get_undefined(env, &undefined);
                napi_create_array(env, &result[PARAM1]);
                ProcessFormsInfo(env, result[PARAM1], asyncCallbackInfo->formInfos);
                result[PARAM0] = GetCallbackErrorValue(env, asyncCallbackInfo->ret ? CODE_SUCCESS : CODE_FAILED);
                napi_get_reference_value(env, asyncCallbackInfo->callback, &callback);
                napi_call_function(env, undefined, callback, ARGS_SIZE_TWO, &result[PARAM0], &callResult);

                if (asyncCallbackInfo->callback != nullptr) {
                    napi_delete_reference(env, asyncCallbackInfo->callback);
                }
                napi_delete_async_work(env, asyncCallbackInfo->asyncWork);
                delete asyncCallbackInfo;
                asyncCallbackInfo = nullptr;
            },
            (void *)asyncCallbackInfo,
            &asyncCallbackInfo->asyncWork);
        NAPI_CALL(env, napi_queue_async_work(env, asyncCallbackInfo->asyncWork));
        napi_value ret = nullptr;
        NAPI_CALL(env, napi_get_null(env, &ret));
        if (ret == nullptr) {
            if (asyncCallbackInfo != nullptr) {
                delete asyncCallbackInfo;
                asyncCallbackInfo = nullptr;
            }
        }
        napi_value result;
        NAPI_CALL(env, napi_create_int32(env, NAPI_RETURN_ONE, &result));
        return result;
    } else {
        HILOG_INFO("GetFormsInfoByApp promise.");
        napi_deferred deferred;
        napi_value promise;
        NAPI_CALL(env, napi_create_promise(env, &deferred, &promise));
        asyncCallbackInfo->deferred = deferred;

        napi_value resourceName;
        napi_create_string_latin1(env, "GetFormsInfoByApp", NAPI_AUTO_LENGTH, &resourceName);
        napi_create_async_work(env,
            nullptr,
            resourceName,
            [](napi_env env, void *data) {
                AsyncFormInfosByAppCallbackInfo *asyncCallbackInfo = (AsyncFormInfosByAppCallbackInfo *)data;
                InnerGetFormInfosByApp(env, asyncCallbackInfo->bundleName, asyncCallbackInfo->formInfos);
            },
            [](napi_env env, napi_status status, void *data) {
                HILOG_INFO("=================load=================");
                AsyncFormInfosByAppCallbackInfo *asyncCallbackInfo = (AsyncFormInfosByAppCallbackInfo *)data;
                napi_value result;
                napi_create_array(env, &result);
                ProcessFormsInfo(env, result, asyncCallbackInfo->formInfos);
                napi_resolve_deferred(asyncCallbackInfo->env, asyncCallbackInfo->deferred, result);
                napi_delete_async_work(env, asyncCallbackInfo->asyncWork);
                delete asyncCallbackInfo;
                asyncCallbackInfo = nullptr;
            },
            (void *)asyncCallbackInfo,
            &asyncCallbackInfo->asyncWork);
        napi_queue_async_work(env, asyncCallbackInfo->asyncWork);

        napi_value ret = nullptr;
        NAPI_CALL(env, napi_get_null(env, &ret));
        if (ret == nullptr) {
            if (asyncCallbackInfo != nullptr) {
                delete asyncCallbackInfo;
                asyncCallbackInfo = nullptr;
            }
        }
        return promise;
    }
}

static void ProcessShortcutInfos(
    napi_env env, napi_value result, const std::vector<OHOS::AppExecFwk::ShortcutInfo> &shortcutInfos)
{
    if (shortcutInfos.size() > 0) {
        HILOG_INFO("-----ShortcutInfos is not null-----");
        size_t index = 0;
        for (const auto &item : shortcutInfos) {
            HILOG_INFO("shortcutId{%s} ", item.id.c_str());
            HILOG_INFO("bundleName{%s} ", item.bundleName.c_str());
            napi_value objShortcutInfo;
            NAPI_CALL_RETURN_VOID(env, napi_create_object(env, &objShortcutInfo));
            ConvertShortcutInfos(env, objShortcutInfo, item);
            NAPI_CALL_RETURN_VOID(env, napi_set_element(env, result, index, objShortcutInfo));
            index++;
        }
    } else {
        HILOG_INFO("-----ShortcutInfos is null-----");
    }
}

static bool InnerGetShortcutInfos(
    napi_env env, const std::string &bundleName, std::vector<OHOS::AppExecFwk::ShortcutInfo> &shortcutInfos)
{
    auto iBundleMgr = GetBundleMgr();
    if (!iBundleMgr) {
        HILOG_ERROR("can not get iBundleMgr");
        return false;
    }
    return iBundleMgr->GetShortcutInfos(bundleName, shortcutInfos);
}
/**
 * Promise and async callback
 */
napi_value GetShortcutInfos(napi_env env, napi_callback_info info)
{
    size_t argc = ARGS_SIZE_THREE;
    napi_value argv[ARGS_SIZE_THREE] = {nullptr};
    napi_value thisArg;
    void *data = nullptr;
    NAPI_CALL(env, napi_get_cb_info(env, info, &argc, argv, &thisArg, &data));
    HILOG_INFO("ARGCSIZE is =%{public}zu.", argc);
    std::string bundleName;
    ParseString(env, bundleName, argv[PARAM0]);
    AsyncShortcutInfosCallbackInfo *asyncCallbackInfo = new (std::nothrow)
        AsyncShortcutInfosCallbackInfo {.env = env, .asyncWork = nullptr, .deferred = nullptr,
            .bundleName = bundleName};
    if (asyncCallbackInfo == nullptr) {
        return nullptr;
    }
    if (argc > (ARGS_SIZE_TWO - CALLBACK_SIZE)) {
        HILOG_INFO("GetShortcutInfos asyncCallback.");
        napi_value resourceName;
        NAPI_CALL(env, napi_create_string_latin1(env, "GetShortcutInfos", NAPI_AUTO_LENGTH, &resourceName));
        napi_valuetype valuetype = napi_undefined;
        napi_typeof(env, argv[ARGS_SIZE_ONE], &valuetype);
        NAPI_ASSERT(env, valuetype == napi_function, "Wrong argument type. Function expected.");
        NAPI_CALL(env, napi_create_reference(env, argv[ARGS_SIZE_ONE], NAPI_RETURN_ONE, &asyncCallbackInfo->callback));

        napi_create_async_work(env,
            nullptr,
            resourceName,
            [](napi_env env, void *data) {
                AsyncShortcutInfosCallbackInfo *asyncCallbackInfo = (AsyncShortcutInfosCallbackInfo *)data;
                asyncCallbackInfo->ret =
                    InnerGetShortcutInfos(env, asyncCallbackInfo->bundleName, asyncCallbackInfo->shortcutInfos);
            },
            [](napi_env env, napi_status status, void *data) {
                AsyncShortcutInfosCallbackInfo *asyncCallbackInfo = (AsyncShortcutInfosCallbackInfo *)data;
                napi_value result[ARGS_SIZE_TWO] = {0};
                napi_value callback = 0;
                napi_value undefined = 0;
                napi_value callResult = 0;
                napi_get_undefined(env, &undefined);
                napi_create_array(env, &result[PARAM1]);
                ProcessShortcutInfos(env, result[PARAM1], asyncCallbackInfo->shortcutInfos);
                result[PARAM0] = GetCallbackErrorValue(env, asyncCallbackInfo->ret ? CODE_SUCCESS : CODE_FAILED);
                napi_get_reference_value(env, asyncCallbackInfo->callback, &callback);
                napi_call_function(env, undefined, callback, ARGS_SIZE_TWO, &result[PARAM0], &callResult);

                if (asyncCallbackInfo->callback != nullptr) {
                    napi_delete_reference(env, asyncCallbackInfo->callback);
                }
                napi_delete_async_work(env, asyncCallbackInfo->asyncWork);
                delete asyncCallbackInfo;
                asyncCallbackInfo = nullptr;
            },
            (void *)asyncCallbackInfo,
            &asyncCallbackInfo->asyncWork);
        NAPI_CALL(env, napi_queue_async_work(env, asyncCallbackInfo->asyncWork));
        napi_value ret = nullptr;
        NAPI_CALL(env, napi_get_null(env, &ret));
        if (ret == nullptr) {
            if (asyncCallbackInfo != nullptr) {
                delete asyncCallbackInfo;
                asyncCallbackInfo = nullptr;
            }
        }
        napi_value result;
        NAPI_CALL(env, napi_create_int32(env, NAPI_RETURN_ONE, &result));
        return result;
    } else {
        HILOG_INFO("GetShortcutInfos promise.");
        napi_deferred deferred;
        napi_value promise;
        NAPI_CALL(env, napi_create_promise(env, &deferred, &promise));
        asyncCallbackInfo->deferred = deferred;

        napi_value resourceName;
        napi_create_string_latin1(env, "GetShortcutInfos", NAPI_AUTO_LENGTH, &resourceName);
        napi_create_async_work(env,
            nullptr,
            resourceName,
            [](napi_env env, void *data) {
                AsyncShortcutInfosCallbackInfo *asyncCallbackInfo = (AsyncShortcutInfosCallbackInfo *)data;
                InnerGetShortcutInfos(env, asyncCallbackInfo->bundleName, asyncCallbackInfo->shortcutInfos);
            },
            [](napi_env env, napi_status status, void *data) {
                HILOG_INFO("=================load=================");
                AsyncShortcutInfosCallbackInfo *asyncCallbackInfo = (AsyncShortcutInfosCallbackInfo *)data;
                napi_value result;
                napi_create_array(env, &result);
                ProcessShortcutInfos(env, result, asyncCallbackInfo->shortcutInfos);
                napi_resolve_deferred(asyncCallbackInfo->env, asyncCallbackInfo->deferred, result);
                napi_delete_async_work(env, asyncCallbackInfo->asyncWork);
                delete asyncCallbackInfo;
                asyncCallbackInfo = nullptr;
            },
            (void *)asyncCallbackInfo,
            &asyncCallbackInfo->asyncWork);
        napi_queue_async_work(env, asyncCallbackInfo->asyncWork);

        napi_value ret = nullptr;
        NAPI_CALL(env, napi_get_null(env, &ret));
        if (ret == nullptr) {
            if (asyncCallbackInfo != nullptr) {
                delete asyncCallbackInfo;
                asyncCallbackInfo = nullptr;
            }
        }
        return promise;
    }
}

static void ProcessModuleUsageRecords(
    napi_env env, napi_value result, const std::vector<OHOS::AppExecFwk::ModuleUsageRecord> &moduleUsageRecords)
{
    if (moduleUsageRecords.size() > 0) {
        HILOG_INFO("-----moduleUsageRecords is not null-----");
        size_t index = 0;
        for (const auto &item : moduleUsageRecords) {
            HILOG_INFO("bundleName{%s} ", item.bundleName.c_str());
            HILOG_INFO("abilityName{%s} ", item.abilityName.c_str());
            napi_value objModuleUsageRecord;
            NAPI_CALL_RETURN_VOID(env, napi_create_object(env, &objModuleUsageRecord));
            ConvertModuleUsageRecords(env, objModuleUsageRecord, item);
            NAPI_CALL_RETURN_VOID(env, napi_set_element(env, result, index, objModuleUsageRecord));
            index++;
        }
    } else {
        HILOG_INFO("-----moduleUsageRecords is null-----");
    }
}

static bool InnerGetModuleUsageRecords(
    napi_env env, const int32_t number, std::vector<OHOS::AppExecFwk::ModuleUsageRecord> &moduleUsageRecords)
{
    auto iBundleMgr = GetBundleMgr();
    if (!iBundleMgr) {
        HILOG_ERROR("can not get iBundleMgr");
        return false;
    }
    return iBundleMgr->GetModuleUsageRecords(number, moduleUsageRecords);
}
/**
 * Promise and async callback
 */
napi_value GetModuleUsageRecords(napi_env env, napi_callback_info info)
{
    size_t argc = ARGS_SIZE_THREE;
    napi_value argv[ARGS_SIZE_THREE] = {nullptr};
    napi_value thisArg;
    void *data = nullptr;
    NAPI_CALL(env, napi_get_cb_info(env, info, &argc, argv, &thisArg, &data));
    HILOG_INFO("ARGCSIZE is =%{public}zu.", argc);
    int number;
    ParseInt(env, number, argv[PARAM0]);
    AsyncModuleUsageRecordsCallbackInfo *asyncCallbackInfo = new (std::nothrow)
        AsyncModuleUsageRecordsCallbackInfo {.env = env, .asyncWork = nullptr, .deferred = nullptr, .number = number};
    if (asyncCallbackInfo == nullptr) {
        return nullptr;
    }
    if (argc > (ARGS_SIZE_TWO - CALLBACK_SIZE)) {
        HILOG_INFO("GetModuleUsageRecords asyncCallback.");
        napi_value resourceName;
        NAPI_CALL(env, napi_create_string_latin1(env, "GetModuleUsageRecords", NAPI_AUTO_LENGTH, &resourceName));
        napi_valuetype valuetype = napi_undefined;
        napi_typeof(env, argv[ARGS_SIZE_ONE], &valuetype);
        NAPI_ASSERT(env, valuetype == napi_function, "Wrong argument type. Function expected.");
        NAPI_CALL(env, napi_create_reference(env, argv[ARGS_SIZE_ONE], NAPI_RETURN_ONE, &asyncCallbackInfo->callback));

        napi_create_async_work(env,
            nullptr,
            resourceName,
            [](napi_env env, void *data) {
                AsyncModuleUsageRecordsCallbackInfo *asyncCallbackInfo = (AsyncModuleUsageRecordsCallbackInfo *)data;
                asyncCallbackInfo->ret =
                    InnerGetModuleUsageRecords(env, asyncCallbackInfo->number, asyncCallbackInfo->moduleUsageRecords);
            },
            [](napi_env env, napi_status status, void *data) {
                AsyncModuleUsageRecordsCallbackInfo *asyncCallbackInfo = (AsyncModuleUsageRecordsCallbackInfo *)data;
                napi_value result[ARGS_SIZE_TWO] = {0};
                napi_value callback = 0;
                napi_value undefined = 0;
                napi_value callResult = 0;
                napi_get_undefined(env, &undefined);
                napi_create_array(env, &result[PARAM1]);
                ProcessModuleUsageRecords(env, result[PARAM1], asyncCallbackInfo->moduleUsageRecords);
                result[PARAM0] = GetCallbackErrorValue(env, asyncCallbackInfo->ret ? CODE_SUCCESS : CODE_FAILED);
                napi_get_reference_value(env, asyncCallbackInfo->callback, &callback);
                napi_call_function(env, undefined, callback, ARGS_SIZE_TWO, &result[PARAM0], &callResult);

                if (asyncCallbackInfo->callback != nullptr) {
                    napi_delete_reference(env, asyncCallbackInfo->callback);
                }
                napi_delete_async_work(env, asyncCallbackInfo->asyncWork);
                delete asyncCallbackInfo;
                asyncCallbackInfo = nullptr;
            },
            (void *)asyncCallbackInfo,
            &asyncCallbackInfo->asyncWork);
        NAPI_CALL(env, napi_queue_async_work(env, asyncCallbackInfo->asyncWork));
        napi_value ret = nullptr;
        NAPI_CALL(env, napi_get_null(env, &ret));
        if (ret == nullptr) {
            if (asyncCallbackInfo != nullptr) {
                delete asyncCallbackInfo;
                asyncCallbackInfo = nullptr;
            }
        }
        napi_value result;
        NAPI_CALL(env, napi_create_int32(env, NAPI_RETURN_ONE, &result));
        return result;
    } else {
        HILOG_INFO("GetModuleUsageRecords promise.");
        napi_deferred deferred;
        napi_value promise;
        NAPI_CALL(env, napi_create_promise(env, &deferred, &promise));
        asyncCallbackInfo->deferred = deferred;

        napi_value resourceName;
        napi_create_string_latin1(env, "GetModuleUsageRecords", NAPI_AUTO_LENGTH, &resourceName);
        napi_create_async_work(env,
            nullptr,
            resourceName,
            [](napi_env env, void *data) {
                AsyncModuleUsageRecordsCallbackInfo *asyncCallbackInfo = (AsyncModuleUsageRecordsCallbackInfo *)data;
                InnerGetModuleUsageRecords(env, asyncCallbackInfo->number, asyncCallbackInfo->moduleUsageRecords);
            },
            [](napi_env env, napi_status status, void *data) {
                HILOG_INFO("=================load=================");
                AsyncModuleUsageRecordsCallbackInfo *asyncCallbackInfo = (AsyncModuleUsageRecordsCallbackInfo *)data;
                napi_value result;
                napi_create_array(env, &result);
                ProcessModuleUsageRecords(env, result, asyncCallbackInfo->moduleUsageRecords);
                napi_resolve_deferred(asyncCallbackInfo->env, asyncCallbackInfo->deferred, result);
                napi_delete_async_work(env, asyncCallbackInfo->asyncWork);
                delete asyncCallbackInfo;
                asyncCallbackInfo = nullptr;
            },
            (void *)asyncCallbackInfo,
            &asyncCallbackInfo->asyncWork);
        napi_queue_async_work(env, asyncCallbackInfo->asyncWork);

        napi_value ret = nullptr;
        NAPI_CALL(env, napi_get_null(env, &ret));
        if (ret == nullptr) {
            if (asyncCallbackInfo != nullptr) {
                delete asyncCallbackInfo;
                asyncCallbackInfo = nullptr;
            }
        }
        return promise;
    }
}

static bool InnerRegisterAllPermissionsChanged(napi_env env, napi_ref callbackRef)
{
    HILOG_INFO("InnerRegisterAllPermissionsChanged begin");
    auto iBundleMgr = GetBundleMgr();
    if (!iBundleMgr) {
        HILOG_ERROR("can not get iBundleMgr");
        return false;
    }
    OHOS::sptr<PermissionCallback> callback = new PermissionCallback(env, callbackRef);
    if (!callback) {
        HILOG_ERROR("callback nullptr");
        return false;
    }
    auto result = iBundleMgr->RegisterAllPermissionsChanged(callback);
    if (!result) {
        HILOG_ERROR("RegisterAllPermissionsChanged call error");
        return false;
    }
    std::lock_guard<std::mutex> lock(anyPermissionsCallbackMutex);
    auto ret = anyPermissionsCallback.emplace(callbackRef, callback);
    if (!ret.second) {
        HILOG_ERROR("RegisterAllPermissionsChanged emplace failed");
        return false;
    }
    HILOG_INFO("InnerRegisterAllPermissionsChanged end");
    return true;
}

static napi_value ParseInt32Array(napi_env env, std::vector<int32_t> &uids, napi_value args)
{
    HILOG_INFO("Parseint32Array called");
    bool isArray = false;
    uint32_t arrayLength = 0;
    napi_value valueAry = 0;
    napi_valuetype valueAryType = napi_undefined;
    NAPI_CALL(env, napi_is_array(env, args, &isArray));
    NAPI_CALL(env, napi_get_array_length(env, args, &arrayLength));
    HILOG_INFO("Parseint32Array args is array, length=%{public}ud", arrayLength);

    for (uint32_t j = 0; j < arrayLength; j++) {
        NAPI_CALL(env, napi_get_element(env, args, j, &valueAry));
        NAPI_CALL(env, napi_typeof(env, valueAry, &valueAryType));
        int uid;
        ParseInt(env, uid, valueAry);
        uids.emplace_back(uid);
    }
    // create result code
    napi_value result = nullptr;
    napi_status status = napi_ok;
    status = napi_create_int32(env, NAPI_RETURN_ONE, &result);
    NAPI_ASSERT(env, status == napi_ok, "napi_create_int32 error!");
    return result;
}

static bool InnerRegisterPermissionsChanged(napi_env env, const std::vector<int32_t> &uids, napi_ref callbackRef)
{
    HILOG_INFO("InnerRegisterPermissionsChanged begin");
    auto iBundleMgr = GetBundleMgr();
    if (!iBundleMgr) {
        HILOG_ERROR("can not get iBundleMgr");
        return false;
    }
    OHOS::sptr<PermissionCallback> callback = new PermissionCallback(env, callbackRef);
    if (!callback) {
        HILOG_ERROR("callback nullptr");
        return false;
    }
    auto result = iBundleMgr->RegisterPermissionsChanged(uids, callback);
    if (!result) {
        HILOG_ERROR("RegisterAllPermissionsChanged call error");
        return false;
    }

    PermissionsKey permissonsKey {.callback = callbackRef, .uids = uids};

    std::lock_guard<std::mutex> lock(permissionsCallbackMutex);
    auto ret = permissionsCallback.emplace(permissonsKey, callback);
    if (!ret.second) {
        HILOG_ERROR("InnerRegisterPermissionsChanged emplace failed");
        return false;
    }
    HILOG_INFO("InnerRegisterPermissionsChanged end");
    return true;
}

napi_value RegisterAllPermissionsChanged(napi_env env, napi_callback_info info)
{
    size_t argc = ARGS_SIZE_THREE;
    napi_value argv[ARGS_SIZE_THREE] = {nullptr};
    napi_value thisArg;
    void *data = nullptr;
    NAPI_CALL(env, napi_get_cb_info(env, info, &argc, argv, &thisArg, &data));
    HILOG_INFO("ARGCSIZE is = %{public}zu.", argc);
    std::string permissionEvent;
    ParseString(env, permissionEvent, argv[PARAM0]);
    if (permissionEvent == PERMISSION_CHANGE && argc == ARGS_SIZE_THREE) {
        std::vector<int32_t> uids;
        ParseInt32Array(env, uids, argv[ARGS_SIZE_ONE]);
        AsyncRegisterPermissions *asyncCallbackInfo =
            new (std::nothrow) AsyncRegisterPermissions {.env = env, .asyncWork = nullptr, .uids = uids};
        if (asyncCallbackInfo == nullptr) {
            return nullptr;
        }
        HILOG_INFO("RegisterAllPermissionsChanged permissionChange asyncCallback.");
        napi_valuetype valuetype = napi_undefined;
        napi_typeof(env, argv[ARGS_SIZE_TWO], &valuetype);
        NAPI_ASSERT(env, valuetype == napi_function, "Wrong argument type. Function expected.");
        NAPI_CALL(env, napi_create_reference(env, argv[ARGS_SIZE_TWO], NAPI_RETURN_ONE, &asyncCallbackInfo->callback));

        napi_value resourceName;
        napi_create_string_latin1(env, "NAPI_RegisterPermissionsChanged", NAPI_AUTO_LENGTH, &resourceName);
        napi_create_async_work(env,
            nullptr,
            resourceName,
            [](napi_env env, void *data) {
                AsyncRegisterPermissions *asyncCallbackInfo = (AsyncRegisterPermissions *)data;
                InnerRegisterPermissionsChanged(env, asyncCallbackInfo->uids, asyncCallbackInfo->callback);
            },
            [](napi_env env, napi_status status, void *data) {
                AsyncRegisterPermissions *asyncCallbackInfo = (AsyncRegisterPermissions *)data;
                napi_delete_async_work(env, asyncCallbackInfo->asyncWork);
                delete asyncCallbackInfo;
                asyncCallbackInfo = nullptr;
            },
            (void *)asyncCallbackInfo,
            &asyncCallbackInfo->asyncWork);
        NAPI_CALL(env, napi_queue_async_work(env, asyncCallbackInfo->asyncWork));

        napi_value ret = nullptr;
        NAPI_CALL(env, napi_get_null(env, &ret));
        if (ret == nullptr) {
            if (asyncCallbackInfo != nullptr) {
                delete asyncCallbackInfo;
                asyncCallbackInfo = nullptr;
            }
        }

        napi_value result;
        NAPI_CALL(env, napi_create_int32(env, NAPI_RETURN_ONE, &result));
        return result;
    } else if (permissionEvent == ANY_PERMISSION_CHANGE && argc == ARGS_SIZE_TWO) {
        AsyncRegisterAllPermissions *asyncCallbackInfo =
            new (std::nothrow) AsyncRegisterAllPermissions {.env = env, .asyncWork = nullptr};
        if (asyncCallbackInfo == nullptr) {
            return nullptr;
        }
        HILOG_INFO("RegisterAllPermissionsChanged anyPermissionChange asyncCallback.");
        napi_valuetype valuetype = napi_undefined;
        napi_typeof(env, argv[ARGS_SIZE_ONE], &valuetype);
        NAPI_ASSERT(env, valuetype == napi_function, "Wrong argument type. Function expected.");
        NAPI_CALL(env, napi_create_reference(env, argv[ARGS_SIZE_ONE], NAPI_RETURN_ONE, &asyncCallbackInfo->callback));
        napi_value resourceName;
        napi_create_string_latin1(env, "NAPI_RegisterAllPermissionsChanged", NAPI_AUTO_LENGTH, &resourceName);
        napi_create_async_work(env,
            nullptr,
            resourceName,
            [](napi_env env, void *data) {
                AsyncRegisterAllPermissions *asyncCallbackInfo = (AsyncRegisterAllPermissions *)data;
                InnerRegisterAllPermissionsChanged(env, asyncCallbackInfo->callback);
            },
            [](napi_env env, napi_status status, void *data) {
                AsyncRegisterAllPermissions *asyncCallbackInfo = (AsyncRegisterAllPermissions *)data;
                napi_delete_async_work(env, asyncCallbackInfo->asyncWork);
                delete asyncCallbackInfo;
                asyncCallbackInfo = nullptr;
            },
            (void *)asyncCallbackInfo,
            &asyncCallbackInfo->asyncWork);
        NAPI_CALL(env, napi_queue_async_work(env, asyncCallbackInfo->asyncWork));

        napi_value ret = nullptr;
        NAPI_CALL(env, napi_get_null(env, &ret));
        if (ret == nullptr) {
            if (asyncCallbackInfo != nullptr) {
                delete asyncCallbackInfo;
                asyncCallbackInfo = nullptr;
            }
        }

        napi_value result;
        NAPI_CALL(env, napi_create_int32(env, NAPI_RETURN_ONE, &result));
        return result;
    }

    napi_value result;
    NAPI_CALL(env, napi_create_int32(env, NAPI_RETURN_ZERO, &result));
    return result;
}

static bool InnerUnregisterAnyPermissionsChanged(napi_env env, napi_ref callbackRef)
{
    HILOG_INFO("InnerUnregisterAnyPermissionsChanged");
    std::lock_guard<std::mutex> lock(anyPermissionsCallbackMutex);
    for (const auto &item : anyPermissionsCallback) {
        napi_value callback = 0;
        napi_value ref = 0;
        napi_get_reference_value(env, item.first, &callback);
        napi_get_reference_value(env, callbackRef, &ref);
        bool result = false;
        auto napiRet = napi_strict_equals(env, callback, ref, &result);
        HILOG_INFO("status is = %{public}d.", napiRet);
        if (result) {
            HILOG_INFO("find value in anyPermissionsCallback");
            auto iBundleMgr = GetBundleMgr();
            if (!iBundleMgr) {
                HILOG_ERROR("can not get iBundleMgr");
                return false;
            }
            auto ret = iBundleMgr->UnregisterPermissionsChanged(item.second);
            if (!ret) {
                HILOG_ERROR("UnregisterPermissionsChanged call error");
                return false;
            }
            anyPermissionsCallback.erase(item.first);
            return true;
        }
    }
    HILOG_INFO("InnerUnregisterAnyPermissionsChanged end");
    return false;
}

static bool InnerUnregisterPermissionsChanged(napi_env env, const std::vector<int32_t> &uids, napi_ref callbackRef)
{
    HILOG_INFO("InnerUnregisterPermissionsChanged");
    std::lock_guard<std::mutex> lock(permissionsCallbackMutex);
    for (const auto &item : permissionsCallback) {
        napi_value callback = 0;
        napi_value ref = 0;
        napi_get_reference_value(env, item.first.callback, &callback);
        napi_get_reference_value(env, callbackRef, &ref);
        bool result = false;
        auto napiRet = napi_strict_equals(env, callback, ref, &result);
        HILOG_INFO("status is = %{public}d.", napiRet);
        if (result && uids == item.first.uids) {
            HILOG_INFO("find value in permissionsCallback");
            auto iBundleMgr = GetBundleMgr();
            if (!iBundleMgr) {
                HILOG_ERROR("can not get iBundleMgr");
                return false;
            }
            auto ret = iBundleMgr->UnregisterPermissionsChanged(item.second);
            if (!ret) {
                HILOG_ERROR("InnerUnregisterPermissionsChanged call error");
                return false;
            }
            HILOG_INFO("call UnregisterPermissionsChanged success = %{public}zu.", permissionsCallback.size());
            permissionsCallback.erase(item.first);
            return true;
        }
        HILOG_INFO("can not find value in permissionsCallback");
    }
    HILOG_INFO("InnerUnregisterPermissionsChanged end");
    return false;
}

napi_value UnregisterPermissionsChanged(napi_env env, napi_callback_info info)
{
    size_t argc = ARGS_SIZE_THREE;
    napi_value argv[ARGS_SIZE_THREE] = {nullptr};
    napi_value thisArg;
    void *data = nullptr;
    NAPI_CALL(env, napi_get_cb_info(env, info, &argc, argv, &thisArg, &data));
    HILOG_INFO("ARGCSIZE is =%{public}zu.", argc);
    std::string permissionEvent;
    ParseString(env, permissionEvent, argv[PARAM0]);

    if (permissionEvent == ANY_PERMISSION_CHANGE && argc == ARGS_SIZE_TWO) {
        AsyncUnregisterPermissions *asyncCallbackInfo = new (std::nothrow) AsyncUnregisterPermissions {
            .env = env,
            .asyncWork = nullptr,
        };
        if (asyncCallbackInfo == nullptr) {
            return nullptr;
        }
        HILOG_INFO("UnregisterAnyPermissionsChanged asyncCallback.");
        napi_valuetype valuetype = napi_undefined;
        napi_typeof(env, argv[ARGS_SIZE_ONE], &valuetype);
        NAPI_ASSERT(env, valuetype == napi_function, "Wrong argument type. Function expected.");
        NAPI_CALL(env, napi_create_reference(env, argv[ARGS_SIZE_ONE], NAPI_RETURN_ONE, &asyncCallbackInfo->callback));
        napi_value resourceName;
        napi_create_string_latin1(env, "NAPI_UnreegisterAnyPermissionsChanged", NAPI_AUTO_LENGTH, &resourceName);
        napi_create_async_work(env,
            nullptr,
            resourceName,
            [](napi_env env, void *data) {
                AsyncUnregisterPermissions *asyncCallbackInfo = (AsyncUnregisterPermissions *)data;
                asyncCallbackInfo->ret = InnerUnregisterAnyPermissionsChanged(env, asyncCallbackInfo->callback);
            },
            [](napi_env env, napi_status status, void *data) {
                AsyncUnregisterPermissions *asyncCallbackInfo = (AsyncUnregisterPermissions *)data;
                napi_value result[ARGS_SIZE_ONE] = {0};
                napi_value callback = 0;
                napi_value undefined = 0;
                napi_value callResult = 0;
                result[PARAM0] = GetCallbackErrorValue(env, asyncCallbackInfo->ret ? CODE_SUCCESS : CODE_FAILED);
                napi_get_reference_value(env, asyncCallbackInfo->callback, &callback);
                napi_call_function(env, undefined, callback, ARGS_SIZE_ONE, &result[PARAM0], &callResult);
                if (asyncCallbackInfo->callback != nullptr) {
                    napi_delete_reference(env, asyncCallbackInfo->callback);
                }
                napi_delete_async_work(env, asyncCallbackInfo->asyncWork);
                delete asyncCallbackInfo;
                asyncCallbackInfo = nullptr;
            },
            (void *)asyncCallbackInfo,
            &asyncCallbackInfo->asyncWork);
        NAPI_CALL(env, napi_queue_async_work(env, asyncCallbackInfo->asyncWork));

        napi_value ret = nullptr;
        NAPI_CALL(env, napi_get_null(env, &ret));
        if (ret == nullptr) {
            if (asyncCallbackInfo != nullptr) {
                delete asyncCallbackInfo;
                asyncCallbackInfo = nullptr;
            }
        }
        napi_value result;
        NAPI_CALL(env, napi_create_int32(env, NAPI_RETURN_ONE, &result));
        return result;
    } else if (permissionEvent == PERMISSION_CHANGE && argc == ARGS_SIZE_THREE) {
        std::vector<int32_t> uids;
        ParseInt32Array(env, uids, argv[ARGS_SIZE_ONE]);
        AsyncUnregisterPermissions *asyncCallbackInfo =
            new AsyncUnregisterPermissions {.env = env, .asyncWork = nullptr, .uids = uids};
        if (asyncCallbackInfo == nullptr) {
            return nullptr;
        }
        HILOG_INFO("UnregisterPermissionsChanged asyncCallback.");
        napi_valuetype valuetype = napi_undefined;
        napi_typeof(env, argv[ARGS_SIZE_TWO], &valuetype);
        NAPI_ASSERT(env, valuetype == napi_function, "Wrong argument type. Function expected.");
        NAPI_CALL(env, napi_create_reference(env, argv[ARGS_SIZE_TWO], NAPI_RETURN_ONE, &asyncCallbackInfo->callback));
        napi_value resourceName;
        napi_create_string_latin1(env, "NAPI_UnreegisterPermissionsChanged", NAPI_AUTO_LENGTH, &resourceName);
        napi_create_async_work(env,
            nullptr,
            resourceName,
            [](napi_env env, void *data) {
                AsyncUnregisterPermissions *asyncCallbackInfo = (AsyncUnregisterPermissions *)data;
                asyncCallbackInfo->ret =
                    InnerUnregisterPermissionsChanged(env, asyncCallbackInfo->uids, asyncCallbackInfo->callback);
            },
            [](napi_env env, napi_status status, void *data) {
                AsyncUnregisterPermissions *asyncCallbackInfo = (AsyncUnregisterPermissions *)data;
                napi_value result[ARGS_SIZE_ONE] = {0};
                napi_value callback = 0;
                napi_value undefined = 0;
                napi_value callResult = 0;
                result[PARAM0] = GetCallbackErrorValue(env, asyncCallbackInfo->ret ? CODE_SUCCESS : CODE_FAILED);
                napi_get_reference_value(env, asyncCallbackInfo->callback, &callback);
                napi_call_function(env, undefined, callback, ARGS_SIZE_ONE, &result[PARAM0], &callResult);
                if (asyncCallbackInfo->callback != nullptr) {
                    napi_delete_reference(env, asyncCallbackInfo->callback);
                }
                napi_delete_async_work(env, asyncCallbackInfo->asyncWork);
                delete asyncCallbackInfo;
                asyncCallbackInfo = nullptr;
            },
            (void *)asyncCallbackInfo,
            &asyncCallbackInfo->asyncWork);
        NAPI_CALL(env, napi_queue_async_work(env, asyncCallbackInfo->asyncWork));

        napi_value ret = nullptr;
        NAPI_CALL(env, napi_get_null(env, &ret));
        if (ret == nullptr) {
            if (asyncCallbackInfo != nullptr) {
                delete asyncCallbackInfo;
                asyncCallbackInfo = nullptr;
            }
        }

        napi_value result;
        NAPI_CALL(env, napi_create_int32(env, NAPI_RETURN_ONE, &result));
        return result;
    }
    napi_value result;
    NAPI_CALL(env, napi_create_int32(env, NAPI_RETURN_ZERO, &result));
    return result;
}

static int InnerCheckPermission(napi_env env, const std::string &bundleName, const std::string &permission)
{
    auto iBundleMgr = GetBundleMgr();
    if (!iBundleMgr) {
        HILOG_ERROR("can not get iBundleMgr");
        return false;
    };
    int ret = iBundleMgr->CheckPermission(bundleName, permission);

    return ret;
}

/**
 * Promise and async callback
 */
napi_value CheckPermission(napi_env env, napi_callback_info info)
{
    size_t argc = ARGS_SIZE_THREE;
    napi_value argv[ARGS_SIZE_THREE] = {0};
    napi_value thisArg = nullptr;
    void *data = nullptr;
    NAPI_CALL(env, napi_get_cb_info(env, info, &argc, argv, &thisArg, &data));
    std::string bundleName;
    ParseString(env, bundleName, argv[PARAM0]);
    std::string permission;
    ParseString(env, permission, argv[PARAM1]);

    AsyncPermissionCallbackInfo *asyncCallbackInfo = new (std::nothrow) AsyncPermissionCallbackInfo {
        .env = env, .asyncWork = nullptr, .deferred = nullptr, .bundleName = bundleName, .permission = permission};
    if (asyncCallbackInfo == nullptr) {
        return nullptr;
    }
    if (argc > (ARGS_SIZE_THREE - CALLBACK_SIZE)) {
        HILOG_INFO("CheckPermission asyncCallback.");
        napi_value resourceName;
        napi_create_string_latin1(env, "CheckPermission", NAPI_AUTO_LENGTH, &resourceName);

        napi_valuetype valuetype = napi_undefined;
        napi_typeof(env, argv[ARGS_SIZE_TWO], &valuetype);
        NAPI_ASSERT(env, valuetype == napi_function, "Wrong argument type. Function expected.");
        NAPI_CALL(env, napi_create_reference(env, argv[ARGS_SIZE_TWO], NAPI_RETURN_ONE, &asyncCallbackInfo->callback));

        napi_create_async_work(env,
            nullptr,
            resourceName,
            [](napi_env env, void *data) {
                AsyncPermissionCallbackInfo *asyncCallbackInfo = (AsyncPermissionCallbackInfo *)data;
                asyncCallbackInfo->ret =
                    InnerCheckPermission(env, asyncCallbackInfo->bundleName, asyncCallbackInfo->permission);
            },
            [](napi_env env, napi_status status, void *data) {
                AsyncPermissionCallbackInfo *asyncCallbackInfo = (AsyncPermissionCallbackInfo *)data;
                napi_value result[ARGS_SIZE_TWO] = {0};
                napi_value callback = 0;
                napi_value undefined = 0;
                napi_value callResult = 0;
                napi_get_undefined(env, &undefined);
                napi_create_int32(env, asyncCallbackInfo->ret, &result[PARAM1]);
                result[PARAM0] = GetCallbackErrorValue(env, asyncCallbackInfo->ret == 0 ? CODE_SUCCESS : CODE_FAILED);
                napi_get_reference_value(env, asyncCallbackInfo->callback, &callback);
                napi_call_function(env, undefined, callback, ARGS_SIZE_TWO, &result[PARAM0], &callResult);

                if (asyncCallbackInfo->callback != nullptr) {
                    napi_delete_reference(env, asyncCallbackInfo->callback);
                }
                napi_delete_async_work(env, asyncCallbackInfo->asyncWork);
                delete asyncCallbackInfo;
                asyncCallbackInfo = nullptr;
            },
            (void *)asyncCallbackInfo,
            &asyncCallbackInfo->asyncWork);

        NAPI_CALL(env, napi_queue_async_work(env, asyncCallbackInfo->asyncWork));
        napi_value ret = nullptr;
        NAPI_CALL(env, napi_get_null(env, &ret));
        if (ret == nullptr) {
            if (asyncCallbackInfo != nullptr) {
                delete asyncCallbackInfo;
                asyncCallbackInfo = nullptr;
            }
        }
        napi_value result;
        NAPI_CALL(env, napi_create_int32(env, NAPI_RETURN_ONE, &result));
        return result;
    } else {
        HILOG_INFO("BundleMgr::CheckPermission promise.");
        napi_deferred deferred;
        napi_value promise;
        NAPI_CALL(env, napi_create_promise(env, &deferred, &promise));
        asyncCallbackInfo->deferred = deferred;

        napi_value resourceName;
        napi_create_string_latin1(env, "CheckPermission", NAPI_AUTO_LENGTH, &resourceName);
        napi_create_async_work(env,
            nullptr,
            resourceName,
            [](napi_env env, void *data) {
                AsyncPermissionCallbackInfo *asyncCallbackInfo = (AsyncPermissionCallbackInfo *)data;
                asyncCallbackInfo->ret =
                    InnerCheckPermission(env, asyncCallbackInfo->bundleName, asyncCallbackInfo->permission);
            },
            [](napi_env env, napi_status status, void *data) {
                HILOG_INFO("=================load=================");
                AsyncPermissionCallbackInfo *asyncCallbackInfo = (AsyncPermissionCallbackInfo *)data;
                napi_value result;
                napi_create_int32(asyncCallbackInfo->env, asyncCallbackInfo->ret, &result);
                napi_resolve_deferred(asyncCallbackInfo->env, asyncCallbackInfo->deferred, result);
                napi_delete_async_work(env, asyncCallbackInfo->asyncWork);
                delete asyncCallbackInfo;
                asyncCallbackInfo = nullptr;
            },
            (void *)asyncCallbackInfo,
            &asyncCallbackInfo->asyncWork);
        napi_queue_async_work(env, asyncCallbackInfo->asyncWork);

        napi_value ret = nullptr;
        NAPI_CALL(env, napi_get_null(env, &ret));
        if (ret == nullptr) {
            if (asyncCallbackInfo != nullptr) {
                delete asyncCallbackInfo;
                asyncCallbackInfo = nullptr;
            }
        }
        return promise;
    }
}

void CreateAbilityTypeObject(napi_env env, napi_value value)
{
    napi_value nUnknow;
    NAPI_CALL_RETURN_VOID(env, napi_create_int32(env, static_cast<int32_t>(AbilityType::UNKNOWN), &nUnknow));
    NAPI_CALL_RETURN_VOID(env, napi_set_named_property(env, value, "UNKNOWN", nUnknow));
    napi_value nPage;
    NAPI_CALL_RETURN_VOID(env, napi_create_int32(env, static_cast<int32_t>(AbilityType::PAGE), &nPage));
    NAPI_CALL_RETURN_VOID(env, napi_set_named_property(env, value, "PAGE", nPage));
    napi_value nService;
    NAPI_CALL_RETURN_VOID(env, napi_create_int32(env, static_cast<int32_t>(AbilityType::SERVICE), &nService));
    NAPI_CALL_RETURN_VOID(env, napi_set_named_property(env, value, "SERVICE", nService));
    napi_value nData;
    NAPI_CALL_RETURN_VOID(env, napi_create_int32(env, static_cast<int32_t>(AbilityType::DATA), &nData));
    NAPI_CALL_RETURN_VOID(env, napi_set_named_property(env, value, "DATA", nData));
}

void CreateAbilitySubTypeObject(napi_env env, napi_value value)
{
    napi_value nUnspecified;
    NAPI_CALL_RETURN_VOID(env, napi_create_int32(env, NAPI_RETURN_ZERO, &nUnspecified));
    NAPI_CALL_RETURN_VOID(env, napi_set_named_property(env, value, "UNSPECIFIED", nUnspecified));
    napi_value nCa;
    NAPI_CALL_RETURN_VOID(env, napi_create_int32(env, NAPI_RETURN_ONE, &nCa));
    NAPI_CALL_RETURN_VOID(env, napi_set_named_property(env, value, "CA", nCa));
}

void CreateDisplayOrientationObject(napi_env env, napi_value value)
{
    napi_value nUnspecified;
    NAPI_CALL_RETURN_VOID(
        env, napi_create_int32(env, static_cast<int32_t>(DisplayOrientation::UNSPECIFIED), &nUnspecified));
    NAPI_CALL_RETURN_VOID(env, napi_set_named_property(env, value, "UNSPECIFIED", nUnspecified));
    napi_value nLandscape;
    NAPI_CALL_RETURN_VOID(
        env, napi_create_int32(env, static_cast<int32_t>(DisplayOrientation::LANDSCAPE), &nLandscape));
    NAPI_CALL_RETURN_VOID(env, napi_set_named_property(env, value, "LANDSCAPE", nLandscape));
    napi_value nPortrait;
    NAPI_CALL_RETURN_VOID(env, napi_create_int32(env, static_cast<int32_t>(DisplayOrientation::PORTRAIT), &nPortrait));
    NAPI_CALL_RETURN_VOID(env, napi_set_named_property(env, value, "PORTRAIT", nPortrait));
    napi_value nFollowrecent;
    NAPI_CALL_RETURN_VOID(
        env, napi_create_int32(env, static_cast<int32_t>(DisplayOrientation::FOLLOWRECENT), &nFollowrecent));
    NAPI_CALL_RETURN_VOID(env, napi_set_named_property(env, value, "FOLLOWRECENT", nFollowrecent));
}

void CreateLaunchModeObject(napi_env env, napi_value value)
{
    napi_value nSingleton;
    NAPI_CALL_RETURN_VOID(env, napi_create_int32(env, static_cast<int32_t>(LaunchMode::SINGLETON), &nSingleton));
    NAPI_CALL_RETURN_VOID(env, napi_set_named_property(env, value, "SINGLETON", nSingleton));
    napi_value nSingletop;
    NAPI_CALL_RETURN_VOID(env, napi_create_int32(env, static_cast<int32_t>(LaunchMode::SINGLETOP), &nSingletop));
    NAPI_CALL_RETURN_VOID(env, napi_set_named_property(env, value, "SINGLETOP", nSingletop));
    napi_value nStandard;
    NAPI_CALL_RETURN_VOID(env, napi_create_int32(env, static_cast<int32_t>(LaunchMode::STANDARD), &nStandard));
    NAPI_CALL_RETURN_VOID(env, napi_set_named_property(env, value, "STANDARD", nStandard));
}

void CreateModuleUpdateFlagObject(napi_env env, napi_value value)
{
    napi_value nFlagModuleUpgradeCheck;
    NAPI_CALL_RETURN_VOID(env, napi_create_int32(env, NAPI_RETURN_ZERO, &nFlagModuleUpgradeCheck));
    NAPI_CALL_RETURN_VOID(
        env, napi_set_named_property(env, value, "FLAG_MODULE_UPGRADE_CHECK", nFlagModuleUpgradeCheck));
    napi_value nFlagModuleUpgradeInstall;
    NAPI_CALL_RETURN_VOID(env, napi_create_int32(env, NAPI_RETURN_ONE, &nFlagModuleUpgradeInstall));
    NAPI_CALL_RETURN_VOID(
        env, napi_set_named_property(env, value, "FLAG_MODULE_UPGRADE_INSTALL", nFlagModuleUpgradeInstall));
    napi_value nFlagModuleUpgradeinstallWithConfigWindows;
    NAPI_CALL_RETURN_VOID(env, napi_create_int32(env, NAPI_RETURN_TWO, &nFlagModuleUpgradeinstallWithConfigWindows));
    NAPI_CALL_RETURN_VOID(env,
        napi_set_named_property(
            env, value, "FLAG_MODULE_UPGRADE_INSTALL_WITH_CONFIG_WINDOWS", nFlagModuleUpgradeinstallWithConfigWindows));
}

void CreateFormTypeObject(napi_env env, napi_value value)
{
    napi_value nJava;
    NAPI_CALL_RETURN_VOID(env, napi_create_int32(env, NAPI_RETURN_ZERO, &nJava));
    NAPI_CALL_RETURN_VOID(env, napi_set_named_property(env, value, "JAVA", nJava));
    napi_value nJs;
    NAPI_CALL_RETURN_VOID(env, napi_create_int32(env, NAPI_RETURN_ONE, &nJs));
    NAPI_CALL_RETURN_VOID(env, napi_set_named_property(env, value, "JS", nJs));
}

void CreateColorModeObject(napi_env env, napi_value value)
{
    napi_value nAutoMode;
    NAPI_CALL_RETURN_VOID(env, napi_create_int32(env, NAPI_RETURN_FAILED, &nAutoMode));
    NAPI_CALL_RETURN_VOID(env, napi_set_named_property(env, value, "AUTO_MODE", nAutoMode));
    napi_value nDarkMode;
    NAPI_CALL_RETURN_VOID(env, napi_create_int32(env, NAPI_RETURN_ZERO, &nDarkMode));
    NAPI_CALL_RETURN_VOID(env, napi_set_named_property(env, value, "DARK_MODE", nDarkMode));
    napi_value nLightMode;
    NAPI_CALL_RETURN_VOID(env, napi_create_int32(env, NAPI_RETURN_ONE, &nLightMode));
    NAPI_CALL_RETURN_VOID(env, napi_set_named_property(env, value, "LIGHT_MODE", nLightMode));
}

void CreateGrantStatusObject(napi_env env, napi_value value)
{
    napi_value nPermissionDenied;
    NAPI_CALL_RETURN_VOID(env, napi_create_int32(env, NAPI_RETURN_FAILED, &nPermissionDenied));
    NAPI_CALL_RETURN_VOID(env, napi_set_named_property(env, value, "PERMISSION_DENIED", nPermissionDenied));
    napi_value nPermissionGranted;
    NAPI_CALL_RETURN_VOID(env, napi_create_int32(env, NAPI_RETURN_ZERO, &nPermissionGranted));
    NAPI_CALL_RETURN_VOID(env, napi_set_named_property(env, value, "PERMISSION_GRANTED", nPermissionGranted));
}

void CreateModuleRemoveFlagObject(napi_env env, napi_value value)
{
    napi_value nFlagModuleNotUsedByForm;
    NAPI_CALL_RETURN_VOID(env, napi_create_int32(env, NAPI_RETURN_ZERO, &nFlagModuleNotUsedByForm));
    NAPI_CALL_RETURN_VOID(
        env, napi_set_named_property(env, value, "FLAG_MODULE_NOT_USED_BY_FORM", nFlagModuleNotUsedByForm));
    napi_value nFlagModuleUsedByForm;
    NAPI_CALL_RETURN_VOID(env, napi_create_int32(env, NAPI_RETURN_ONE, &nFlagModuleUsedByForm));
    NAPI_CALL_RETURN_VOID(env, napi_set_named_property(env, value, "FLAG_MODULE_USED_BY_FORM", nFlagModuleUsedByForm));
    napi_value nFlagModuleNotUsedByShortcut;
    NAPI_CALL_RETURN_VOID(env, napi_create_int32(env, NAPI_RETURN_TWO, &nFlagModuleNotUsedByShortcut));
    NAPI_CALL_RETURN_VOID(
        env, napi_set_named_property(env, value, "FLAG_MODULE_NOT_USED_BY_SHORTCUT", nFlagModuleNotUsedByShortcut));
    napi_value nFlagModuleUsedByShortcut;
    NAPI_CALL_RETURN_VOID(env, napi_create_int32(env, NAPI_RETURN_THREE, &nFlagModuleUsedByShortcut));
    NAPI_CALL_RETURN_VOID(
        env, napi_set_named_property(env, value, "FLAG_MODULE_USED_BY_SHORTCUT", nFlagModuleUsedByShortcut));
}

void CreateSignatureCompareResultObject(napi_env env, napi_value value)
{
    napi_value nSignatureMatched;
    NAPI_CALL_RETURN_VOID(env, napi_create_int32(env, NAPI_RETURN_ZERO, &nSignatureMatched));
    NAPI_CALL_RETURN_VOID(env, napi_set_named_property(env, value, "SIGNATURE_MATCHED", nSignatureMatched));
    napi_value nSignatureNotMatched;
    NAPI_CALL_RETURN_VOID(env, napi_create_int32(env, NAPI_RETURN_ONE, &nSignatureNotMatched));
    NAPI_CALL_RETURN_VOID(env, napi_set_named_property(env, value, "SIGNATURE_NOT_MATCHED", nSignatureNotMatched));
    napi_value nSignatureUnknownBundle;
    NAPI_CALL_RETURN_VOID(env, napi_create_int32(env, NAPI_RETURN_TWO, &nSignatureUnknownBundle));
    NAPI_CALL_RETURN_VOID(
        env, napi_set_named_property(env, value, "SIGNATURE_UNKNOWN_BUNDLE", nSignatureUnknownBundle));
}

void CreateShortcutExistenceObject(napi_env env, napi_value value)
{
    napi_value nShortcutExistenceExists;
    NAPI_CALL_RETURN_VOID(env, napi_create_int32(env, NAPI_RETURN_ZERO, &nShortcutExistenceExists));
    NAPI_CALL_RETURN_VOID(
        env, napi_set_named_property(env, value, "SHORTCUT_EXISTENCE_EXISTS", nShortcutExistenceExists));
    napi_value nShortcutExistenceNotExists;
    NAPI_CALL_RETURN_VOID(env, napi_create_int32(env, NAPI_RETURN_ONE, &nShortcutExistenceNotExists));
    NAPI_CALL_RETURN_VOID(
        env, napi_set_named_property(env, value, "SHORTCUT_EXISTENCE_NOT_EXISTS", nShortcutExistenceNotExists));
    napi_value nShortcutExistenceUnknow;
    NAPI_CALL_RETURN_VOID(env, napi_create_int32(env, NAPI_RETURN_TWO, &nShortcutExistenceUnknow));
    NAPI_CALL_RETURN_VOID(
        env, napi_set_named_property(env, value, "SHORTCUT_EXISTENCE_UNKNOW", nShortcutExistenceUnknow));
}

void CreateQueryShortCutFlagObject(napi_env env, napi_value value)
{
    napi_value nQueryShortCutHome;
    NAPI_CALL_RETURN_VOID(env, napi_create_int32(env, NAPI_RETURN_ZERO, &nQueryShortCutHome));
    NAPI_CALL_RETURN_VOID(env, napi_set_named_property(env, value, "QUERY_SHORTCUT_HOME", nQueryShortCutHome));
}

void CreateBundleFlagObject(napi_env env, napi_value value)
{
    napi_value nDefault;
    NAPI_CALL_RETURN_VOID(
        env, napi_create_int32(env, static_cast<int32_t>(BundleFlag::GET_BUNDLE_DEFAULT), &nDefault));
    NAPI_CALL_RETURN_VOID(env, napi_set_named_property(env, value, "GET_BUNDLE_DEFAULT", nDefault));
    napi_value nWithAbilities;
    NAPI_CALL_RETURN_VOID(
        env, napi_create_int32(env, static_cast<int32_t>(BundleFlag::GET_BUNDLE_WITH_ABILITIES), &nWithAbilities));
    NAPI_CALL_RETURN_VOID(env, napi_set_named_property(env, value, "GET_BUNDLE_WITH_ABILITIES", nWithAbilities));
    napi_value nWithPermission;
    NAPI_CALL_RETURN_VOID(
        env, napi_create_int32(
            env, static_cast<int32_t>(BundleFlag::GET_APPLICATION_INFO_WITH_PERMISSION), &nWithPermission));
    NAPI_CALL_RETURN_VOID(
        env, napi_set_named_property(env, value, "GET_APPLICATION_INFO_WITH_PERMISSION", nWithPermission));
}

void CreateInstallErrorCodeObject(napi_env env, napi_value value)
{
    napi_value nSuccess;
    NAPI_CALL_RETURN_VOID(env, napi_create_int32(env, static_cast<int32_t>(InstallErrorCode::SUCCESS), &nSuccess));
    NAPI_CALL_RETURN_VOID(env, napi_set_named_property(env, value, "SUCCESS", nSuccess));
    napi_value nStatusInstallFailure;
    NAPI_CALL_RETURN_VOID(env,
        napi_create_int32(env, static_cast<int32_t>(InstallErrorCode::STATUS_INSTALL_FAILURE), &nStatusInstallFailure));
    NAPI_CALL_RETURN_VOID(env, napi_set_named_property(env, value, "STATUS_INSTALL_FAILURE", nStatusInstallFailure));
    napi_value nStatusInstallFailureAborted;
    NAPI_CALL_RETURN_VOID(env,
        napi_create_int32(env,
            static_cast<int32_t>(InstallErrorCode::STATUS_INSTALL_FAILURE_ABORTED),
            &nStatusInstallFailureAborted));
    NAPI_CALL_RETURN_VOID(
        env, napi_set_named_property(env, value, "STATUS_INSTALL_FAILURE_ABORTED", nStatusInstallFailureAborted));
    napi_value nStatusInstallFailureInvalid;
    NAPI_CALL_RETURN_VOID(env,
        napi_create_int32(env,
            static_cast<int32_t>(InstallErrorCode::STATUS_INSTALL_FAILURE_INVALID),
            &nStatusInstallFailureInvalid));
    NAPI_CALL_RETURN_VOID(
        env, napi_set_named_property(env, value, "STATUS_INSTALL_FAILURE_INVALID", nStatusInstallFailureInvalid));
    napi_value nStatusInstallFailureConflict;
    NAPI_CALL_RETURN_VOID(env,
        napi_create_int32(env,
            static_cast<int32_t>(InstallErrorCode::STATUS_INSTALL_FAILURE_CONFLICT),
            &nStatusInstallFailureConflict));
    NAPI_CALL_RETURN_VOID(
        env, napi_set_named_property(env, value, "STATUS_INSTALL_FAILURE_CONFLICT", nStatusInstallFailureConflict));
    napi_value nStatusInstallFailureStorage;
    NAPI_CALL_RETURN_VOID(env,
        napi_create_int32(env,
            static_cast<int32_t>(InstallErrorCode::STATUS_INSTALL_FAILURE_STORAGE),
            &nStatusInstallFailureStorage));
    NAPI_CALL_RETURN_VOID(
        env, napi_set_named_property(env, value, "STATUS_INSTALL_FAILURE_STORAGE", nStatusInstallFailureStorage));
    napi_value nStatusInstallFailureIncompatible;
    NAPI_CALL_RETURN_VOID(env,
        napi_create_int32(env,
            static_cast<int32_t>(InstallErrorCode::STATUS_INSTALL_FAILURE_INCOMPATIBLE),
            &nStatusInstallFailureIncompatible));
    NAPI_CALL_RETURN_VOID(env,
        napi_set_named_property(env, value, "STATUS_INSTALL_FAILURE_INCOMPATIBLE", nStatusInstallFailureIncompatible));
    napi_value nStatusUninstallFailure;
    NAPI_CALL_RETURN_VOID(env,
        napi_create_int32(
            env, static_cast<int32_t>(InstallErrorCode::STATUS_UNINSTALL_FAILURE), &nStatusUninstallFailure));
    NAPI_CALL_RETURN_VOID(
        env, napi_set_named_property(env, value, "STATUS_UNINSTALL_FAILURE", nStatusUninstallFailure));
    napi_value nStatusUninstallFailureBlocked;
    NAPI_CALL_RETURN_VOID(env,
        napi_create_int32(env,
            static_cast<int32_t>(InstallErrorCode::STATUS_UNINSTALL_FAILURE_BLOCKED),
            &nStatusUninstallFailureBlocked));
    NAPI_CALL_RETURN_VOID(
        env, napi_set_named_property(env, value, "STATUS_UNINSTALL_FAILURE_BLOCKED", nStatusUninstallFailureBlocked));
    napi_value nStatusUninstallFailureAborted;
    NAPI_CALL_RETURN_VOID(env,
        napi_create_int32(env,
            static_cast<int32_t>(InstallErrorCode::STATUS_UNINSTALL_FAILURE_ABORTED),
            &nStatusUninstallFailureAborted));
    NAPI_CALL_RETURN_VOID(
        env, napi_set_named_property(env, value, "STATUS_UNINSTALL_FAILURE_ABORTED", nStatusUninstallFailureAborted));
    napi_value nStatusUninstallFailureConflict;
    NAPI_CALL_RETURN_VOID(env,
        napi_create_int32(env,
            static_cast<int32_t>(InstallErrorCode::STATUS_UNINSTALL_FAILURE_CONFLICT),
            &nStatusUninstallFailureConflict));
    NAPI_CALL_RETURN_VOID(
        env, napi_set_named_property(env, value, "STATUS_UNINSTALL_FAILURE_CONFLICT", nStatusUninstallFailureConflict));
    napi_value nStatusInstallFailureDownloadTimeout;
    NAPI_CALL_RETURN_VOID(env,
        napi_create_int32(env,
            static_cast<int32_t>(InstallErrorCode::STATUS_INSTALL_FAILURE_DOWNLOAD_TIMEOUT),
            &nStatusInstallFailureDownloadTimeout));
    NAPI_CALL_RETURN_VOID(env,
        napi_set_named_property(
            env, value, "STATUS_INSTALL_FAILURE_DOWNLOAD_TIMEOUT", nStatusInstallFailureDownloadTimeout));
    napi_value nStatusInstallFailureDownloadFailed;
    NAPI_CALL_RETURN_VOID(env,
        napi_create_int32(env,
            static_cast<int32_t>(InstallErrorCode::STATUS_INSTALL_FAILURE_DOWNLOAD_FAILED),
            &nStatusInstallFailureDownloadFailed));
    NAPI_CALL_RETURN_VOID(env,
        napi_set_named_property(
            env, value, "STATUS_INSTALL_FAILURE_DOWNLOAD_FAILED", nStatusInstallFailureDownloadFailed));
    napi_value nStatusAbilityNotFound;
    NAPI_CALL_RETURN_VOID(env,
        napi_create_int32(
            env, static_cast<int32_t>(InstallErrorCode::STATUS_ABILITY_NOT_FOUND), &nStatusAbilityNotFound));
    NAPI_CALL_RETURN_VOID(env, napi_set_named_property(env, value, "STATUS_ABILITY_NOT_FOUND", nStatusAbilityNotFound));
    napi_value nBmsServiceError;
    NAPI_CALL_RETURN_VOID(env,
        napi_create_int32(env, static_cast<int32_t>(InstallErrorCode::STATUS_BMS_SERVICE_ERROR), &nBmsServiceError));
    NAPI_CALL_RETURN_VOID(env, napi_set_named_property(env, value, "STATUS_BMS_SERVICE_ERROR", nBmsServiceError));
}
}  // namespace AppExecFwk
}  // namespace OHOS