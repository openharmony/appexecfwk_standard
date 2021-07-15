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

#include "context_deal.h"
#include "file_ex.h"
#include "directory_ex.h"
#include "iservice_registry.h"
#include "app_log_wrapper.h"
#include "ability_manager_interface.h"
#include "ability_manager_client.h"
#include "system_ability_definition.h"
#include "sys_mgr_client.h"
#include "spec_task_dispatcher.h"
#include "application_context.h"
#include "task_dispatcher_context.h"

#define MODE 0771
namespace OHOS {
namespace AppExecFwk {

const std::string ContextDeal::CONTEXT_DEAL_FILE_SEPARATOR("/");
const std::string ContextDeal::CONTEXT_DEAL_CODE_CACHE("code_cache");
const std::string ContextDeal::CONTEXT_DEAL_Files("files");
const std::string ContextDeal::CONTEXT_DEAL_NO_BACKUP_Files("no_backup");
const std::string ContextDeal::CONTEXT_DEAL_DIRNAME("preferences");

/**
 * Called when getting the ProcessInfo
 *
 * @return ProcessInfo
 */
std::shared_ptr<ProcessInfo> ContextDeal::GetProcessInfo() const
{
    return processInfo_;
}

/**
 * Called when setting the ProcessInfo
 *
 * @param info ProcessInfo instance
 */
void ContextDeal::SetProcessInfo(const std::shared_ptr<ProcessInfo> &info)
{
    if (info == nullptr) {
        APP_LOGE("ContextDeal::SetProcessInfo failed, info is empty");
        return;
    }
    processInfo_ = info;
}

/**
 * @brief Obtains information about the current application. The returned application information includes basic
 * information such as the application name and application permissions.
 *
 * @return Returns the ApplicationInfo for the current application.
 */
std::shared_ptr<ApplicationInfo> ContextDeal::GetApplicationInfo() const
{
    return applicationInfo_;
}

/**
 * @brief Set ApplicationInfo
 *
 * @param info ApplicationInfo instance.
 */
void ContextDeal::SetApplicationInfo(const std::shared_ptr<ApplicationInfo> &info)
{
    if (info == nullptr) {
        APP_LOGE("ContextDeal::SetApplicationInfo failed, info is empty");
        return;
    }
    applicationInfo_ = info;
}

/**
 * @brief Obtains the Context object of the application.
 *
 * @return Returns the Context object of the application.
 */
std::shared_ptr<Context> ContextDeal::GetApplicationContext() const
{
    return appContext_;
}

/**
 * @brief Set ApplicationContext
 *
 * @param context ApplicationContext instance.
 */
void ContextDeal::SetApplicationContext(const std::shared_ptr<Context> &context)
{
    if (context == nullptr) {
        APP_LOGE("ContextDeal::SetApplicationContext failed, context is empty");
        return;
    }
    appContext_ = context;
}

/**
 * @brief Obtains the path of the package containing the current ability. The returned path contains the resources,
 *  source code, and configuration files of a module.
 *
 * @return Returns the path of the package file.
 */
std::string ContextDeal::GetBundleCodePath()
{
    return (applicationInfo_ != nullptr) ? applicationInfo_->codePath : "";
}

/**
 * @brief SetBundleCodePath
 *
 * @param Returns string path
 */
void ContextDeal::SetBundleCodePath(std::string &path)
{
    path_ = path;
}

/**
 * @brief Obtains information about the current ability.
 * The returned information includes the class name, bundle name, and other information about the current ability.
 *
 * @return Returns the AbilityInfo object for the current ability.
 */
const std::shared_ptr<AbilityInfo> ContextDeal::GetAbilityInfo()
{
    return abilityInfo_;
}

/**
 * @brief Set AbilityInfo
 *
 * @param info AbilityInfo instance.
 */
void ContextDeal::SetAbilityInfo(const std::shared_ptr<AbilityInfo> &info)
{
    if (info == nullptr) {
        APP_LOGE("ContextDeal::SetAbilityInfo failed, info is empty");
        return;
    }
    abilityInfo_ = info;
}

/**
 * @brief Obtains the Context object of the ability.
 *
 * @return Returns the Context object of the ability.
 */
std::shared_ptr<Context> ContextDeal::GetContext()
{
    return abilityContext_;
}

/**
 * @brief Set Ability context
 *
 * @param context Ability object
 */
void ContextDeal::SetContext(const std::shared_ptr<Context> &context)
{
    if (context == nullptr) {
        APP_LOGE("ContextDeal::SetContext failed, context is empty");
        return;
    }
    abilityContext_ = context;
}

/**
 * @brief Obtains an IBundleMgr instance.
 * You can use this instance to obtain information about the application bundle.
 *
 * @return Returns an IBundleMgr instance.
 */
sptr<IBundleMgr> ContextDeal::GetBundleManager() const
{
    auto bundleObj =
        OHOS::DelayedSingleton<SysMrgClient>::GetInstance()->GetSystemAbility(BUNDLE_MGR_SERVICE_SYS_ABILITY_ID);
    if (bundleObj == nullptr) {
        APP_LOGE("failed to get bundle manager service");
        return nullptr;
    }

    APP_LOGI("get bundle manager proxy success.");
    return iface_cast<IBundleMgr>(bundleObj);
}

/**
 * @brief Obtains a resource manager.
 *
 * @return Returns a ResourceManager object.
 */
std::shared_ptr<Global::Resource::ResourceManager> ContextDeal::GetResourceManager() const
{
    return resourceManager_;
}

/**
 * @brief Set Profile instance.
 *
 * @param Profile instance.
 */
void ContextDeal::SetProfile(const std::shared_ptr<Profile> &profile)
{
    if (profile == nullptr) {
        APP_LOGE("ContextDeal::SetProfile failed, profile is nullptr");
        return;
    }
    profile_ = profile;
}

/**
 * @brief Obtains an Profile instance.
 *
 * @return Returns an Profile instance.
 */
std::shared_ptr<Profile> ContextDeal::GetProfile() const
{
    return profile_;
}

/**
 * @brief Deletes the specified private file associated with the application.
 *
 * @param fileName Indicates the name of the file to delete. The file name cannot contain path separators.
 *
 * @return Returns true if the file is deleted successfully; returns false otherwise.
 */
bool ContextDeal::DeleteFile(const std::string &fileName)
{
    std::string path = GetDataDir() + CONTEXT_DEAL_FILE_SEPARATOR + fileName;
    return OHOS::RemoveFile(path);
}

/**
 * @brief Destroys another ability that uses the AbilityInfo.AbilityType.SERVICE template.
 * The current ability using either the AbilityInfo.AbilityType.SERVICE or AbilityInfo.AbilityType.PAGE
 * template can call this method to destroy another ability that uses the AbilityInfo.AbilityType.SERVICE
 * template. The current ability itself can be destroyed by calling the terminateAbility() method.
 *
 * @param want Indicates the Want containing information about the ability to destroy.
 *
 * @return Returns true if the ability is destroyed successfully; returns false otherwise.
 */
bool ContextDeal::StopAbility(const AAFwk::Want &want)
{
    return false;
}

/**
 * @brief Obtains the application-specific cache directory on the device's internal storage. The system
 * automatically deletes files from the cache directory if disk space is required elsewhere on the device.
 * Older files are always deleted first.
 *
 * @return Returns the application-specific cache directory.
 */
std::string ContextDeal::GetCacheDir()
{
    return (applicationInfo_ != nullptr) ? applicationInfo_->cacheDir : "";
}

/**
 * @brief Obtains the application-specific code-cache directory on the device's internal storage.
 * The system will delete any files stored in this location both when your specific application is upgraded,
 * and when the entire platform is upgraded.
 *
 * @return Returns the application-specific code-cache directory.
 */
std::string ContextDeal::GetCodeCacheDir()
{
    return (applicationInfo_ != nullptr)
               ? (applicationInfo_->dataDir + CONTEXT_DEAL_FILE_SEPARATOR + CONTEXT_DEAL_CODE_CACHE)
               : "";
}

/**
 * @brief Obtains the local database path.
 * If the local database path does not exist, the system creates one and returns the created path.
 *
 * @return Returns the local database file.
 */
std::string ContextDeal::GetDatabaseDir()
{
    return (applicationInfo_ != nullptr) ? applicationInfo_->dataBaseDir : "";
}

/**
 * @brief Obtains the absolute path where all private data files of this application are stored.
 *
 * @return Returns the absolute path storing all private data files of this application.
 */
std::string ContextDeal::GetDataDir()
{
    return (applicationInfo_ != nullptr) ? applicationInfo_->dataDir : "";
}

/**
 * @brief Obtains the directory for storing custom data files of the application.
 * You can use the returned File object to create and access files in this directory. The files
 * can be accessible only by the current application.
 *
 * @param name Indicates the name of the directory to retrieve. This directory is created as part
 * of your application data.
 * @param mode Indicates the file operating mode. The value can be 0 or a combination of MODE_PRIVATE.
 *
 * @return Returns a File object for the requested directory.
 */
std::string ContextDeal::GetDir(const std::string &name, int mode)
{
    if (applicationInfo_ == nullptr) {
        APP_LOGE("ContextDeal::GetDir failed, applicationInfo_ == nullptr");
        return "";
    }
    std::string dir = applicationInfo_->dataDir + CONTEXT_DEAL_FILE_SEPARATOR + name;
    if (!OHOS::FileExists(dir)) {
        APP_LOGI("ContextDeal::GetDir File is not exits");
        OHOS::ForceCreateDirectory(dir);
        OHOS::ChangeModeDirectory(dir, mode);
    }
    return dir;
}

/**
 * @brief Obtains the absolute path to the application-specific cache directory
 * on the primary external or shared storage device.
 *
 * @return Returns the absolute path to the application-specific cache directory on the external or
 * shared storage device; returns null if the external or shared storage device is temporarily unavailable.
 */
std::string ContextDeal::GetExternalCacheDir()
{
    return "";
}

/**
 * @brief Obtains the absolute path to the directory for storing files for the application on the
 * primary external or shared storage device.
 *
 * @param type Indicates the type of the file directory to return
 *
 * @return Returns the absolute path to the application file directory on the external or shared storage
 * device; returns null if the external or shared storage device is temporarily unavailable.
 */
std::string ContextDeal::GetExternalFilesDir(std::string &type)
{
    return "";
}

/**
 * @brief Obtains the directory for storing files for the application on the device's internal storage.
 *
 * @return Returns the application file directory.
 */
std::string ContextDeal::GetFilesDir()
{
    return (applicationInfo_ != nullptr)
               ? (applicationInfo_->dataDir + CONTEXT_DEAL_FILE_SEPARATOR + CONTEXT_DEAL_Files)
               : "";
}

/**
 * @brief Obtains the absolute path which app created and will be excluded from automatic backup to remote storage.
 * The returned path maybe changed if the application is moved to an adopted storage device.
 *
 * @return The path of the directory holding application files that will not be automatically backed up to remote
 * storage.
 */
std::string ContextDeal::GetNoBackupFilesDir()
{
    std::string dir = applicationInfo_->dataDir + CONTEXT_DEAL_NO_BACKUP_Files;
    if (!OHOS::FileExists(dir)) {
        APP_LOGI("ContextDeal::GetDir GetNoBackupFilesDir is not exits");
        OHOS::ForceCreateDirectory(dir);
        OHOS::ChangeModeDirectory(dir, MODE);
    }
    return dir;
}

/**
 * @brief Checks whether the calling process for inter-process communication has the given permission.
 * The calling process is not the current process.
 *
 * @param permission Indicates the permission to check. This parameter cannot be null.
 *
 * @return Returns 0 (IBundleManager.PERMISSION_GRANTED) if the calling process has the permission;
 * returns -1 (IBundleManager.PERMISSION_DENIED) otherwise.
 */
int ContextDeal::VerifyCallingPermission(const std::string &permission)
{
    return 0;
}

/**
 * @brief Checks whether the current process has the given permission.
 * You need to call requestPermissionsFromUser(java.lang.std::string[],int) to request a permission only
 * if the current process does not have the specific permission.
 *
 * @param permission Indicates the permission to check. This parameter cannot be null.
 *
 * @return Returns 0 (IBundleManager.PERMISSION_GRANTED) if the current process has the permission;
 * returns -1 (IBundleManager.PERMISSION_DENIED) otherwise.
 */
int ContextDeal::VerifySelfPermission(const std::string &permission)
{
    return 0;
}

/**
 * @brief Obtains the bundle name of the current ability.
 *
 * @return Returns the bundle name of the current ability.
 */
std::string ContextDeal::GetBundleName()
{
    return (applicationInfo_ != nullptr) ? applicationInfo_->bundleName : "";
}

/**
 * @brief Obtains the path of the OHOS Ability Package (HAP} containing this ability.
 *
 * @return Returns the path of the HAP containing this ability.
 */
std::string ContextDeal::GetBundleResourcePath()
{
    return (abilityInfo_ != nullptr) ? abilityInfo_->resourcePath : "";
}

/**
 * @brief Starts a new ability.
 * An ability using the AbilityInfo.AbilityType.SERVICE or AbilityInfo.AbilityType.PAGE template uses this method
 * to start a specific ability. The system locates the target ability from installed abilities based on the value
 * of the want parameter and then starts it. You can specify the ability to start using the want parameter.
 *
 * @param want Indicates the Want containing information about the target ability to start.
 *
 * @param requestCode Indicates the request code returned after the ability using the AbilityInfo.AbilityType.PAGE
 * template is started. You can define the request code to identify the results returned by abilities. The value
 * ranges from 0 to 65535. This parameter takes effect only on abilities using the AbilityInfo.AbilityType.PAGE
 * template.
 *
 */
void ContextDeal::StartAbility(const AAFwk::Want &want, int requestCode)
{
    APP_LOGI("ContextDeal::StartAbility is called");
}

/**
 * @brief Remove permissions for all users who have access to specific permissions
 *
 * @param permission Indicates the permission to unauth. This parameter cannot be null.
 * @param uri Indicates the URI to unauth. This parameter cannot be null.
 * @param uid Indicates the UID of the unauth to check.
 *
 */
void ContextDeal::UnauthUriPermission(const std::string &permission, const Uri &uri, int uid)
{}

/**
 * @brief Obtains an ability manager.
 * The ability manager provides information about running processes and memory usage of an application.
 *
 * @return Returns an IAbilityManager instance.
 */
sptr<AAFwk::IAbilityManager> ContextDeal::GetAbilityManager()
{
    auto remoteObject = OHOS::DelayedSingleton<SysMrgClient>::GetInstance()->GetSystemAbility(ABILITY_MGR_SERVICE_ID);
    if (remoteObject == nullptr) {
        APP_LOGE("failed to get ability manager service");
        return nullptr;
    }

    APP_LOGI("get bundle ability proxy success.");
    return iface_cast<AAFwk::IAbilityManager>(remoteObject);
}

/**
 * @brief Obtains the type of this application.
 *
 * @return Returns system if this application is a system application;
 * returns normal if it is released in OHOS AppGallery;
 * returns other if it is released by a third-party vendor;
 * returns an empty string if the query fails.
 */
std::string ContextDeal::GetAppType()
{
    sptr<IBundleMgr> ptr = GetBundleManager();
    if (ptr == nullptr) {
        APP_LOGE("GetAppType failed to get bundle manager service");
        return "";
    }

    return ptr->GetAppType(applicationInfo_->bundleName);
}

/**
 * @brief Destroys another ability you had previously started by calling Ability.startAbilityForResult
 * (ohos.aafwk.content.Want, int, ohos.aafwk.ability.startsetting.AbilityStartSetting) with the same requestCode passed.
 *
 * @param requestCode Indicates the request code passed for starting the ability.
 *
 */
void ContextDeal::TerminateAbility(int requestCode)
{}

/**
 * @brief Confirms with the permission management module to check whether a request prompt is required for granting a
 * certain permission. You need to call the current method to check whether a prompt is required before calling
 * requestPermissionsFromUser(java.lang.String[],int) to request a permission. If a prompt is not required, permission
 * request will not be initiated.
 *
 * @param requestCode Indicates the permission to be queried. This parameter cannot be null.
 *
 * @return Returns true if the current application does not have the permission and the user does not turn off further
 * requests; returns false if the current application already has the permission, the permission is rejected by the
 * system, or the permission is denied by the user and the user has turned off further requests.
 */
bool ContextDeal::CanRequestPermission(const std::string &permission)
{
    return false;
}

/**
 * @brief When there is a remote call to check whether the remote has permission, otherwise check whether it has
 * permission
 *
 * @param permissions Indicates the list of permissions to be requested. This parameter cannot be null.
 * @return Returns 0 (IBundleManager.PERMISSION_GRANTED) if the current process has the permission;
 * returns -1 (IBundleManager.PERMISSION_DENIED) otherwise.
 */
int ContextDeal::VerifyCallingOrSelfPermission(const std::string &permission)
{
    return 0;
}

/**
 * @brief Query whether the application of the specified PID and UID has been granted a certain permission
 *
 * @param permissions Indicates the list of permissions to be requested. This parameter cannot be null.
 * @param pid Process id
 * @param uid
 * @return Returns 0 (IBundleManager.PERMISSION_GRANTED) if the current process has the permission;
 * returns -1 (IBundleManager.PERMISSION_DENIED) otherwise.
 */
int ContextDeal::VerifyPermission(const std::string &permission, int pid, int uid)
{
    return 0;
}

/**
 * @brief Obtains the distributed file path.
 * If the distributed file path does not exist, the system creates one and returns the created path. This method is
 * applicable only to the context of an ability rather than that of an application.
 *
 * @return Returns the distributed file.
 */
std::string ContextDeal::GetDistributedDir()
{
    return "";
}
/**
 * @brief Sets the pattern of this Context based on the specified pattern ID.
 *
 * @param patternId Indicates the resource ID of the pattern to set.
 */
void ContextDeal::SetPattern(int patternId)
{
    if (resourceManager_ != nullptr) {
        if (!pattern_.empty()) {
            pattern_.clear();
        }

        OHOS::Global::Resource::RState errval = resourceManager_->GetPatternById(patternId, pattern_);
        if (errval != OHOS::Global::Resource::RState::SUCCESS) {
            APP_LOGE("ContextDeal::SetPattern GetPatternById(patternId:%d) retval is %u", patternId, errval);
        }
    } else {
        APP_LOGE("ContextDeal::SetPattern resourceManager_ is nullptr");
    }
}

/**
 * @brief Obtains the Context object of this ability.
 *
 * @return Returns the Context object of this ability.
 */
std::shared_ptr<Context> ContextDeal::GetAbilityPackageContext()
{
    return nullptr;
}

/**
 * @brief Obtains the HapModuleInfo object of the application.
 *
 * @return Returns the HapModuleInfo object of the application.
 */
std::shared_ptr<HapModuleInfo> ContextDeal::GetHapModuleInfo()
{
    // fix set HapModuleInfoLocal data failed, request only once
    if (hapModuleInfoLocal_ == nullptr) {
        HapModuleInfoRequestInit();
    }

    return hapModuleInfoLocal_;
}

/**
 * @brief Obtains the name of the current process.
 *
 * @return Returns the current process name.
 */
std::string ContextDeal::GetProcessName()
{
    return (processInfo_ != nullptr) ? processInfo_->GetProcessName() : "";
}

/**
 * @brief Obtains the bundle name of the ability that called the current ability.
 * You can use the obtained bundle name to check whether the calling ability is allowed to receive the data you will
 * send. If you did not use Ability.startAbilityForResult(ohos.aafwk.content.Want, int,
 * ohos.aafwk.ability.startsetting.AbilityStartSetting) to start the calling ability, null is returned.
 *
 * @return Returns the bundle name of the calling ability; returns null if no calling ability is available.
 */
std::string ContextDeal::GetCallingBundle()
{
    return "";
}

/**
 * @brief Requests certain permissions from the system.
 * This method is called for permission request. This is an asynchronous method. When it is executed,
 * the Ability.onRequestPermissionsFromUserResult(int, String[], int[]) method will be called back.
 *
 * @param permissions Indicates the list of permissions to be requested. This parameter cannot be null.
 * @param requestCode Indicates the request code to be passed to the Ability.onRequestPermissionsFromUserResult(int,
 * String[], int[]) callback method. This code cannot be a negative number.
 *
 */
void ContextDeal::RequestPermissionsFromUser(std::vector<std::string> &permissions, int requestCode)
{}

/**
 * @brief Starts a new ability with special ability start setting.
 *
 * @param want Indicates the Want containing information about the target ability to start.
 * @param requestCode Indicates the request code returned after the ability is started. You can define the request code
 * to identify the results returned by abilities. The value ranges from 0 to 65535.
 * @param abilityStartSetting Indicates the special start setting used in starting ability.
 *
 */
void ContextDeal::StartAbility(const Want &want, int requestCode, const AbilityStartSetting &abilityStartSetting)
{}

/**
 * @brief Destroys the current ability.
 *
 */
void ContextDeal::TerminateAbility()
{}

/**
 * @brief Connects the current ability to an ability
 *
 * @param want Indicates the want containing information about the ability to connect
 *
 * @param conn Indicates the callback object when the target ability is connected.
 *
 * @return True means success and false means failure
 */
bool ContextDeal::ConnectAbility(const Want &want, const sptr<AAFwk::IAbilityConnection> &conn)
{
    return false;
}

/**
 * @brief Disconnects the current ability from an ability
 *
 * @param conn Indicates the IAbilityConnection callback object passed by connectAbility after the connection
 *              is set up. The IAbilityConnection object uniquely identifies a connection between two abilities.
 */
void ContextDeal::DisconnectAbility(const sptr<AAFwk::IAbilityConnection> &conn)
{}

sptr<IRemoteObject> ContextDeal::GetToken()
{
    return nullptr;
}

/**
 * @brief init the ResourceManager for ContextDeal.
 *
 * @param the ResourceManager has been inited.
 *
 */
void ContextDeal::initResourceManager(const std::shared_ptr<Global::Resource::ResourceManager> &resourceManager)
{
    resourceManager_ = resourceManager;
}

/**
 * @brief Obtains information about the caller of this ability.
 *
 * @return Returns the caller information.
 */
Uri ContextDeal::GetCaller()
{
    Uri uri(uriString_);
    return uri;
}

/**
 * @brief SerUriString
 */
void ContextDeal::SerUriString(const std::string &uri)
{
    uriString_ = uri;
}

/**
 * @brief Get the string of this Context based on the specified resource ID.
 *
 * @param resId Indicates the resource ID of the string to get.
 *
 * @return Returns the string of this Context.
 */
std::string ContextDeal::GetString(int resId)
{
    if (resourceManager_ == nullptr) {
        APP_LOGE("ContextDeal::GetString resourceManager_ is nullptr");
        return "";
    }

    std::string ret;
    OHOS::Global::Resource::RState errval = resourceManager_->GetStringById(resId, ret);
    if (errval == OHOS::Global::Resource::RState::SUCCESS) {
        return ret;
    } else {
        APP_LOGE("ContextDeal::GetString GetStringById(resId:%d) retval is %u", resId, errval);
        return "";
    }
}

/**
 * @brief Get the string array of this Context based on the specified resource ID.
 *
 * @param resId Indicates the resource ID of the string array to get.
 *
 * @return Returns the string array of this Context.
 */
std::vector<std::string> ContextDeal::GetStringArray(int resId)
{
    if (resourceManager_ == nullptr) {
        APP_LOGE("ContextDeal::GetStringArray resourceManager_ is nullptr");
        return std::vector<std::string>();
    }

    std::vector<std::string> retv;
    OHOS::Global::Resource::RState errval = resourceManager_->GetStringArrayById(resId, retv);
    if (errval == OHOS::Global::Resource::RState::SUCCESS) {
        return retv;
    } else {
        APP_LOGE("ContextDeal::GetStringArray GetStringArrayById(resId:%d) retval is %u", resId, errval);
        return std::vector<std::string>();
    }
}

/**
 * @brief Get the integer array of this Context based on the specified resource ID.
 *
 * @param resId Indicates the resource ID of the integer array to get.
 *
 * @return Returns the integer array of this Context.
 */
std::vector<int> ContextDeal::GetIntArray(int resId)
{
    if (resourceManager_ == nullptr) {
        APP_LOGE("ContextDeal::GetIntArray resourceManager_ is nullptr");
        return std::vector<int>();
    }

    std::vector<int> retv;
    OHOS::Global::Resource::RState errval = resourceManager_->GetIntArrayById(resId, retv);
    if (errval == OHOS::Global::Resource::RState::SUCCESS) {
        return retv;
    } else {
        APP_LOGE("ContextDeal::GetIntArray GetIntArrayById(resId:%d) retval is %u", resId, errval);
        return std::vector<int>();
    }
}

/**
 * @brief Obtains the theme of this Context.
 *
 * @return theme Returns the theme of this Context.
 */
std::map<std::string, std::string> ContextDeal::GetTheme()
{
    if (theme_.empty()) {
        SetTheme(GetThemeId());
    }

    return theme_;
}

/**
 * @brief Sets the theme of this Context based on the specified theme ID.
 *
 * @param themeId Indicates the resource ID of the theme to set.
 */
void ContextDeal::SetTheme(int themeId)
{
    if (resourceManager_ == nullptr) {
        APP_LOGE("ContextDeal::SetTheme resourceManager_ is nullptr");
        return;
    }

    auto hapModInfo = GetHapModuleInfo();
    if (hapModInfo == nullptr) {
        APP_LOGE("ContextDeal::SetTheme hapModInfo is nullptr");
        return;
    }

    if (!theme_.empty()) {
        theme_.clear();
    }

    OHOS::Global::Resource::RState errval = resourceManager_->GetThemeById(themeId, theme_);
    if (errval != OHOS::Global::Resource::RState::SUCCESS) {
        APP_LOGE("ContextDeal::SetTheme GetThemeById(themeId:%d) retval is %u", themeId, errval);
        return;
    }

    // hapModInfo->themeId = themeId;

    return;
}

/**
 * @brief Obtains the pattern of this Context.
 *
 * @return getPattern in interface Context
 */
std::map<std::string, std::string> ContextDeal::GetPattern()
{
    if (!pattern_.empty()) {
        return pattern_;
    } else {
        APP_LOGE("ContextDeal::GetPattern pattern_ is empty");
        return std::map<std::string, std::string>();
    }
}

/**
 * @brief Get the color of this Context based on the specified resource ID.
 *
 * @param resId Indicates the resource ID of the color to get.
 *
 * @return Returns the color value of this Context.
 */
int ContextDeal::GetColor(int resId)
{
    if (resourceManager_ == nullptr) {
        APP_LOGE("ContextDeal::GetColor resourceManager_ is nullptr");
        return INVALID_RESOURCE_VALUE;
    }

    uint32_t ret = INVALID_RESOURCE_VALUE;
    OHOS::Global::Resource::RState errval = resourceManager_->GetColorById(resId, ret);
    if (errval == OHOS::Global::Resource::RState::SUCCESS) {
        return ret;
    } else {
        APP_LOGE("ContextDeal::GetColor GetColorById(resId:%d) retval is %u", resId, errval);
        return INVALID_RESOURCE_VALUE;
    }
}

/**
 * @brief Obtains the theme id of this Context.
 *
 * @return int Returns the theme id of this Context.
 */
int ContextDeal::GetThemeId()
{
    auto hapModInfo = GetHapModuleInfo();
    if (hapModInfo != nullptr) {
        // return hapModInfo->themeId;
        return -1;
    } else {
        APP_LOGE("ContextDeal::GetThemeId hapModInfo is nullptr");
        return -1;
    }
}

/**
 * @brief
 * Destroys this Service ability if the number of times it has been started equals the number represented by the
 * given {@code startId}. This method is the same as calling {@link #terminateAbility} to destroy this Service
 * ability, except that this method helps you avoid destroying it if a client has requested a Service
 * ability startup in {@link ohos.aafwk.ability.Ability#onCommand} but you are unaware of it.
 *
 * @param startId Indicates the number of startup times of this Service ability passed to
 *                {@link ohos.aafwk.ability.Ability#onCommand}. The {@code startId} is
 *                incremented by 1 every time this ability is started. For example,
 *                if this ability has been started for six times, the value of {@code startId} is {@code 6}.
 *
 * @return Returns {@code true} if the {@code startId} matches the number of startup times
 *         and this Service ability will be destroyed; returns {@code false} otherwise.
 */
bool ContextDeal::TerminateAbilityResult(int startId)
{
    return false;
}

/**
 * @brief Obtains the current display orientation of this ability.
 *
 * @return Returns the current display orientation.
 */
int ContextDeal::GetDisplayOrientation()
{
    if (abilityInfo_ != nullptr) {
        return static_cast<int>(abilityInfo_->orientation);
    } else {
        APP_LOGE("ContextDeal::GetDisplayOrientation abilityInfo_ is nullptr");
        return static_cast<int>(DisplayOrientation::UNSPECIFIED);
    }
}

/**
 * @brief Obtains the path storing the preference file of the application.
 *        If the preference file path does not exist, the system creates one and returns the created path.
 *
 * @return Returns the preference file path .
 */
std::string ContextDeal::GetPreferencesDir()
{
    if (!preferenceDir_.empty()) {
        return preferenceDir_;
    }

    if (abilityInfo_ == nullptr || applicationInfo_ == nullptr) {
        APP_LOGE("ContextDeal::GetPreferencesDir %s is nullptr",
            (abilityInfo_ == nullptr) ? "abilityInfo_" : "applicationInfo_");
        return "";
    }

    std::string abilityDirectoryName = "";
    if (abilityInfo_->isNativeAbility) {  // Native Interface corresponding IX-1038
        abilityDirectoryName = abilityInfo_->name;
    } else {
        std::string abilityname = abilityInfo_->name;
        size_t findpos = abilityname.find_last_of(".");
        if (findpos == std::string::npos) {
            APP_LOGE("ContextDeal::GetPreferencesDir Find last of string failed.");
            return "";
        }

        abilityDirectoryName = abilityname.substr(findpos + 1, abilityname.length());
        if (abilityDirectoryName == "") {
            APP_LOGE("ContextDeal::GetPreferencesDir abilityDirectoryName is nullptr");
            return "";
        }
    }

    std::string dataDir = applicationInfo_->dataDir + CONTEXT_DEAL_FILE_SEPARATOR + abilityDirectoryName +
                          CONTEXT_DEAL_FILE_SEPARATOR + CONTEXT_DEAL_DIRNAME;

    if (!OHOS::FileExists(dataDir)) {
        APP_LOGI("ContextDeal::GetPreferencesDir File is not exits. Bengin create");
        if (!OHOS::ForceCreateDirectory(dataDir)) {
            APP_LOGE("ContextDeal::GetPreferencesDir ForceCreateDirectory return false. Create failed");
            return "";
        }
        if (!OHOS::ChangeModeDirectory(dataDir, MODE)) {
            APP_LOGW("ContextDeal::GetPreferencesDir ChangeModeDirectory(%s, 0771) return false. Change failed",
                dataDir.c_str());
        }
        APP_LOGI("ContextDeal::GetPreferencesDir File create complete");
    }

    preferenceDir_ = dataDir;

    return preferenceDir_;
}

/**
 * @brief Set color mode
 *
 * @param the value of color mode.
 */
void ContextDeal::SetColorMode(int mode)
{
    auto hapModInfo = GetHapModuleInfo();
    if (hapModInfo == nullptr) {
        APP_LOGE("ContextDeal::SetColorMode hapModInfo is nullptr");
        return;
    }

    if (mode == static_cast<int>(ModuleColorMode::DARK)) {
        hapModInfo->colorMode = ModuleColorMode::DARK;
    } else if (mode == static_cast<int>(ModuleColorMode::LIGHT)) {
        hapModInfo->colorMode = ModuleColorMode::LIGHT;
    } else {  // default use AUTO
        hapModInfo->colorMode = ModuleColorMode::AUTO;
    }
}

/**
 * @brief Obtains color mode.
 *
 * @return Returns the color mode value.
 */
int ContextDeal::GetColorMode()
{
    auto hapModInfo = GetHapModuleInfo();
    if (hapModInfo == nullptr) {
        APP_LOGE("ContextDeal::GetColorMode hapModInfo is nullptr");
        return -1;
    }

    return static_cast<int>(hapModInfo->colorMode);
}

/**
 * @brief Set the LifeCycleStateInfo to the deal.
 *
 * @param info the info to set.
 */
void ContextDeal::SetLifeCycleStateInfo(const AAFwk::LifeCycleStateInfo &info)
{
    lifeCycleStateInfo_ = info;
}

/**
 * @brief Obtains the unique ID of the mission containing this ability.
 *
 * @return Returns the unique mission ID.
 */
int ContextDeal::GetMissionId()
{
    return lifeCycleStateInfo_.missionId;
}

/**
 * @brief Call this when your ability should be closed and the mission should be completely removed as a part of
 * finishing the root ability of the mission.
 */
void ContextDeal::TerminateAndRemoveMission()
{
    auto abilityManagerClient = AAFwk::AbilityManagerClient::GetInstance();
    if (abilityManagerClient == nullptr) {
        APP_LOGE("ContextDeal::TerminateAndRemoveMission abilityManagerClient is nullptr");
        return;
    }

    std::vector<int32_t> removeIdList = {GetMissionId()};
    ErrCode errval = abilityManagerClient->RemoveMissions(removeIdList);
    if (errval != ERR_OK) {
        APP_LOGW("ContextDeal::TerminateAndRemoveMission RemoveMissions retval is ERROR(%d)", errval);
    }
}

/**
 * @brief Starts multiple abilities.
 *
 * @param wants Indicates the Want containing information array about the target ability to start.
 */
void ContextDeal::StartAbilities(const std::vector<AAFwk::Want> &wants)
{}

/**
 * @brief Checks whether this ability is the first ability in a mission.
 *
 * @return Returns true is first in Mission.
 */
bool ContextDeal::IsFirstInMission()
{
    return false;
}

/**
 * @brief Obtains a task dispatcher that is bound to the UI thread.
 *
 * @return Returns the task dispatcher that is bound to the UI thread.
 */
std::shared_ptr<TaskDispatcher> ContextDeal::GetUITaskDispatcher()
{
    return ContextDeal::GetMainTaskDispatcher();
}

/**
 * @brief Obtains a task dispatcher that is bound to the application main thread.
 *
 * @return Returns the task dispatcher that is bound to the application main thread.
 */
std::shared_ptr<TaskDispatcher> ContextDeal::GetMainTaskDispatcher()
{
    if (mainTaskDispatcher_ != nullptr) {
        return mainTaskDispatcher_;
    }

    if (mainEventRunner_ == nullptr) {
        APP_LOGE("ContextDeal::GetMainTaskDispatcher mainTaskDispatcher_ is nullptr");
        return nullptr;
    }

    std::shared_ptr<SpecDispatcherConfig> config =
        std::make_shared<SpecDispatcherConfig>(SpecDispatcherConfig::MAIN, TaskPriority::HIGH);
    if (config == nullptr) {
        APP_LOGE("ContextDeal::GetMainTaskDispatcher config is nullptr");
        return nullptr;
    }

    mainTaskDispatcher_ = std::make_shared<SpecTaskDispatcher>(config, mainEventRunner_);

    return mainTaskDispatcher_;
}

/**
 * @brief Creates a parallel task dispatcher with a specified priority.
 *
 * @param name Indicates the task dispatcher name. This parameter is used to locate problems.
 * @param priority Indicates the priority of all tasks dispatched by the parallel task dispatcher.
 *
 * @return Returns a parallel task dispatcher.
 */
std::shared_ptr<TaskDispatcher> ContextDeal::CreateParallelTaskDispatcher(
    const std::string &name, const TaskPriority &priority)
{
    if (appContext_ == nullptr) {
        APP_LOGE("ContextDeal::CreateParallelTaskDispatcher appContext_ is nullptr");
        return nullptr;
    }

    return appContext_->CreateParallelTaskDispatcher(name, priority);
}

/**
 * @brief Creates a serial task dispatcher with a specified priority.
 *
 * @param name Indicates the task dispatcher name. This parameter is used to locate problems.
 * @param priority Indicates the priority of all tasks dispatched by the created task dispatcher.
 *
 * @return Returns a serial task dispatcher.
 */
std::shared_ptr<TaskDispatcher> ContextDeal::CreateSerialTaskDispatcher(
    const std::string &name, const TaskPriority &priority)
{
    if (appContext_ == nullptr) {
        APP_LOGE("ContextDeal::CreateSerialTaskDispatcher appContext_ is nullptr");
        return nullptr;
    }

    return appContext_->CreateSerialTaskDispatcher(name, priority);
}

/**
 * @brief Obtains a global task dispatcher with a specified priority.
 *
 * @param priority Indicates the priority of all tasks dispatched by the global task dispatcher.
 *
 * @return Returns a global task dispatcher.
 */
std::shared_ptr<TaskDispatcher> ContextDeal::GetGlobalTaskDispatcher(const TaskPriority &priority)
{
    if (appContext_ == nullptr) {
        APP_LOGE("ContextDeal::GetGlobalTaskDispatcher appContext_ is nullptr");
        return nullptr;
    }

    return appContext_->GetGlobalTaskDispatcher(priority);
}

/**
 * @brief Set EventRunner for main thread.
 *
 * @param runner The EventRunner.
 */
void ContextDeal::SetRunner(const std::shared_ptr<EventRunner> &runner)
{
    mainEventRunner_ = runner;
}

bool ContextDeal::HapModuleInfoRequestInit()
{
    sptr<IBundleMgr> ptr = GetBundleManager();
    if (ptr == nullptr) {
        APP_LOGE("GetHapModuleInfo failed to get bundle manager service");
        return false;
    }

    if (abilityInfo_ == nullptr) {
        APP_LOGE("GetHapModuleInfo failed for abilityInfo_ is nullptr");
        return false;
    }

    hapModuleInfoLocal_ = std::make_shared<HapModuleInfo>();
    if (!ptr->GetHapModuleInfo(*abilityInfo_.get(), *hapModuleInfoLocal_)) {
        APP_LOGE("IBundleMgr::GetHapModuleInfo failed, will retval false value");
        return false;
    }

    return true;
}

}  // namespace AppExecFwk
}  // namespace OHOS