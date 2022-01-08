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

#include "context_impl.h"

#include "hilog_wrapper.h"
#include "ipc_singleton.h"
#include "locale_config.h"
#include "os_account_manager.h"
#include "sys_mgr_client.h"
#include "system_ability_definition.h"

namespace OHOS {
namespace AbilityRuntime {
const int64_t ContextImpl::CONTEXT_CREATE_BY_SYSTEM_APP(0x00000001);
const std::string ContextImpl::CONTEXT_BUNDLECODE_BASE("/data/app/base/");
const std::string ContextImpl::CONTEXT_BUNDLECODE("/data/storage/app/base");
const std::string ContextImpl::CONTEXT_BUNDLE("/bundle/");
const std::string ContextImpl::CONTEXT_DISTRIBUTEDFILES_BASE_BEFORE("/mnt/hmdfs/");
const std::string ContextImpl::CONTEXT_DISTRIBUTEDFILES_BASE_MIDDLE("/device_view/local/data/");
const std::string ContextImpl::CONTEXT_DISTRIBUTEDFILES("distributedfiles");
const std::string ContextImpl::CONTEXT_FILE_SEPARATOR("/");
const std::string ContextImpl::CONTEXT_DATA("/data/");
const std::string ContextImpl::CONTEXT_BASE("/data/storage/");
const std::string ContextImpl::CONTEXT_PRIVATE("/private/");
const std::string ContextImpl::CONTEXT_CACHES("caches");
const std::string ContextImpl::CONTEXT_STORAGE("storage");
const std::string ContextImpl::CONTEXT_DATABASE("database");
const std::string ContextImpl::CONTEXT_TEMP("/temp");
const std::string ContextImpl::CONTEXT_FILES("/files");
const std::string ContextImpl::CONTEXT_CE("ce");

std::string ContextImpl::GetBundleName() const
{
    if (parentContext_ != nullptr) {
        return parentContext_->GetBundleName();
    }
    return (applicationInfo_ != nullptr) ? applicationInfo_->bundleName : "";
}

std::string ContextImpl::GetBundleCodeDir()
{
    std::string dir;
    if (IsCreateBySystemApp()) {
        dir = CONTEXT_BUNDLECODE_BASE + GetBundleName();
    } else {
        dir = CONTEXT_BUNDLECODE;
    }
    HILOG_DEBUG("ContextImpl::GetBundleCodeDir:%{public}s", dir.c_str());
    return dir;
}

std::string ContextImpl::GetCacheDir()
{
    std::string dir = GetBaseDir() + CONTEXT_PRIVATE + CONTEXT_CACHES;
    HILOG_DEBUG("ContextImpl::GetCacheDir:%{public}s", dir.c_str());
    return dir;
}

std::string ContextImpl::GetDatabaseDir()
{
    std::string dir = GetBaseDir() + CONTEXT_PRIVATE + CONTEXT_DATABASE;
    HILOG_DEBUG("ContextImpl::GetDatabaseDir:%{public}s", dir.c_str());
    return dir;
}

std::string ContextImpl::GetStorageDir()
{
    std::string dir = GetBaseDir() + CONTEXT_PRIVATE + CONTEXT_STORAGE;
    HILOG_DEBUG("ContextImpl::GetStorageDir:%{public}s", dir.c_str());
    return dir;
}

std::string ContextImpl::GetTempDir()
{
    std::string dir = GetBaseDir() + CONTEXT_TEMP;
    HILOG_DEBUG("ContextImpl::GetTempDir:%{public}s", dir.c_str());
    return dir;
}

std::string ContextImpl::GetFilesDir()
{
    std::string dir = GetBaseDir() + CONTEXT_FILES;
    HILOG_DEBUG("ContextImpl::GetFilesDir:%{public}s", dir.c_str());
    return dir;
}

std::string ContextImpl::GetDistributedFilesDir()
{
    HILOG_DEBUG("ContextImpl::GetDistributedFilesDir");
    std::string dir;
    if (IsCreateBySystemApp()) {
        dir = CONTEXT_DISTRIBUTEDFILES_BASE_BEFORE + GetCurrentAccountId() +
            CONTEXT_DISTRIBUTEDFILES_BASE_MIDDLE + GetBundleName();
    } else {
        dir = CONTEXT_BASE + CONTEXT_DISTRIBUTEDFILES;
    }
    HILOG_DEBUG("ContextImpl::GetDistributedFilesDir:%{public}s", dir.c_str());
    return dir;
}

std::string ContextImpl::GetBaseDir() const
{
    std::string baseDir;
    if (IsCreateBySystemApp()) {
        baseDir = CONTEXT_DATA + currArea_ + CONTEXT_FILE_SEPARATOR + GetCurrentAccountId() +
            CONTEXT_BUNDLE + GetBundleName();
    } else {
        baseDir = CONTEXT_BASE + currArea_;
    }
    if (parentContext_ != nullptr) {
        baseDir =  baseDir + CONTEXT_FILE_SEPARATOR + GetHapModuleInfo()->moduleName;
    }

    HILOG_DEBUG("ContextImpl::GetBaseDir:%{public}s", baseDir.c_str());
    return baseDir;
}

std::string ContextImpl::GetCurrentAccountId() const
{
    int userId;
    AccountSA::OsAccountManager::GetOsAccountLocalIdFromProcess(userId);
    return std::to_string(userId);
}

std::shared_ptr<Context> ContextImpl::CreateBundleContext(const std::string &bundleName)
{
    if (parentContext_ != nullptr) {
        return parentContext_->CreateBundleContext(bundleName);
    }

    if (bundleName.empty()) {
        HILOG_ERROR("ContextImpl::CreateBundleContext bundleName is empty");
        return nullptr;
    }

    sptr<AppExecFwk::IBundleMgr> bundleMgr = GetBundleManager();
    if (bundleMgr == nullptr) {
        HILOG_ERROR("ContextImpl::CreateBundleContext GetBundleManager is nullptr");
        return nullptr;
    }

    AppExecFwk::BundleInfo bundleInfo;
    HILOG_DEBUG("ContextImpl::CreateBundleContext length: %{public}zu, bundleName: %{public}s",
        (size_t)bundleName.length(),
        bundleName.c_str());
    bundleMgr->GetBundleInfo(bundleName, AppExecFwk::BundleFlag::GET_BUNDLE_DEFAULT, bundleInfo);

    if (bundleInfo.name.empty() || bundleInfo.applicationInfo.name.empty()) {
        HILOG_ERROR("ContextImpl::CreateBundleContext GetBundleInfo is error");
        return nullptr;
    }

    std::shared_ptr<ContextImpl> appContext = std::make_shared<ContextImpl>();
    appContext->SetFlags(CONTEXT_CREATE_BY_SYSTEM_APP);

    // init resourceManager.
    InitResourceManager(bundleInfo, appContext);

    appContext->SetApplicationInfo(std::make_shared<AppExecFwk::ApplicationInfo>(bundleInfo.applicationInfo));
    return appContext;
}

void ContextImpl::InitResourceManager(
    const AppExecFwk::BundleInfo &bundleInfo, const std::shared_ptr<ContextImpl> &appContext) const
{
    std::shared_ptr<Global::Resource::ResourceManager> resourceManager(Global::Resource::CreateResourceManager());
    if (appContext == nullptr || resourceManager == nullptr) {
        HILOG_ERROR("ContextImpl::InitResourceManager create resourceManager failed");
        return;
    }

    HILOG_DEBUG(
        "ContextImpl::InitResourceManager moduleResPaths count: %{public}zu", bundleInfo.moduleResPaths.size());
    for (auto moduleResPath : bundleInfo.moduleResPaths) {
        if (!moduleResPath.empty()) {
            HILOG_ERROR("ContextImpl::InitResourceManager length: %{public}zu, moduleResPath: %{public}s",
                moduleResPath.length(),
                moduleResPath.c_str());
            if (!resourceManager->AddResource(moduleResPath.c_str())) {
                HILOG_ERROR("ContextImpl::InitResourceManager AddResource failed");
            }
        }
    }

    std::unique_ptr<Global::Resource::ResConfig> resConfig(Global::Resource::CreateResConfig());
    UErrorCode status = U_ZERO_ERROR;
    icu::Locale locale = icu::Locale::forLanguageTag(Global::I18n::LocaleConfig::GetSystemLanguage(), status);
    resConfig->SetLocaleInfo(locale);
    if (resConfig->GetLocaleInfo() != nullptr) {
        HILOG_DEBUG("ContextImpl::InitResourceManager language: %{public}s, script: %{public}s, region: %{public}s,",
            resConfig->GetLocaleInfo()->getLanguage(),
            resConfig->GetLocaleInfo()->getScript(),
            resConfig->GetLocaleInfo()->getCountry());
    } else {
        HILOG_ERROR("ContextImpl::InitResourceManager language: GetLocaleInfo is null.");
    }
    resourceManager->UpdateResConfig(*resConfig);
    appContext->SetResourceManager(resourceManager);
}

sptr<AppExecFwk::IBundleMgr> ContextImpl::GetBundleManager() const
{
    HILOG_DEBUG("ContextImpl::GetBundleManager begin");
    auto bundleObj =
        OHOS::DelayedSingleton<AppExecFwk::SysMrgClient>::GetInstance()->GetSystemAbility(
            BUNDLE_MGR_SERVICE_SYS_ABILITY_ID);
    if (bundleObj == nullptr) {
        HILOG_ERROR("failed to get bundle manager service");
        return nullptr;
    }
    HILOG_DEBUG("ContextImpl::GetBundleManager before iface_cast<bundleObj>");
    sptr<AppExecFwk::IBundleMgr> bms = iface_cast<AppExecFwk::IBundleMgr>(bundleObj);
    HILOG_DEBUG("ContextImpl::GetBundleManager after iface_cast<bundleObj>");
    HILOG_DEBUG("ContextImpl::GetBundleManager end");
    return bms;
}

void ContextImpl::SetApplicationInfo(const std::shared_ptr<AppExecFwk::ApplicationInfo> &info)
{
    if (info == nullptr) {
        HILOG_ERROR("ContextImpl::SetApplicationInfo failed, info is empty");
        return;
    }
    applicationInfo_ = info;
}

void ContextImpl::SetResourceManager(const std::shared_ptr<Global::Resource::ResourceManager> &resourceManager)
{
    HILOG_DEBUG("ContextImpl::initResourceManager. Start.");
    resourceManager_ = resourceManager;
    HILOG_DEBUG("ContextImpl::initResourceManager. End.");
}

std::shared_ptr<Global::Resource::ResourceManager> ContextImpl::GetResourceManager() const
{
    if (parentContext_ != nullptr) {
        return parentContext_->GetResourceManager();
    }

    return resourceManager_;
}

std::shared_ptr<AppExecFwk::ApplicationInfo> ContextImpl::GetApplicationInfo() const
{
    if (parentContext_ != nullptr) {
        return parentContext_->GetApplicationInfo();
    }

    return applicationInfo_;
}

void ContextImpl::SetParentContext(const std::shared_ptr<Context> &context)
{
    parentContext_ = context;
}

std::string ContextImpl::GetBundleCodePath() const
{
    if (parentContext_ != nullptr) {
        return parentContext_->GetBundleCodePath();
    }
    return (applicationInfo_ != nullptr) ? applicationInfo_->codePath : "";
}

void ContextImpl::InitHapModuleInfo(const std::shared_ptr<AppExecFwk::AbilityInfo> &abilityInfo)
{
    if (hapModuleInfo_ != nullptr || abilityInfo == nullptr) {
        return;
    }
    sptr<AppExecFwk::IBundleMgr> ptr = GetBundleManager();
    if (ptr == nullptr) {
        HILOG_ERROR("InitHapModuleInfo: failed to get bundle manager service");
        return;
    }

    hapModuleInfo_ = std::make_shared<AppExecFwk::HapModuleInfo>();
    if (!ptr->GetHapModuleInfo(*abilityInfo.get(), *hapModuleInfo_)) {
        HILOG_ERROR("InitHapModuleInfo: GetHapModuleInfo failed, will retval false value");
    }
}

std::shared_ptr<AppExecFwk::HapModuleInfo> ContextImpl::GetHapModuleInfo() const
{
    return hapModuleInfo_;
}

void ContextImpl::SetFlags(int64_t flags)
{
    flags_ = static_cast<uint64_t>(flags_) | CONTEXT_CREATE_BY_SYSTEM_APP;
}

bool ContextImpl::IsCreateBySystemApp() const
{
    return (static_cast<uint64_t>(flags_) & CONTEXT_CREATE_BY_SYSTEM_APP) == 1;
}

std::shared_ptr<Context> Context::appContext_ = nullptr;
std::mutex Context::contextMutex_;

std::shared_ptr<Context> Context::GetApplicationContext()
{
    std::lock_guard<std::mutex> lock(contextMutex_);
    return appContext_;
}

void ContextImpl::InitAppContext()
{
    std::lock_guard<std::mutex> lock(Context::contextMutex_);
    Context::appContext_ = shared_from_this();
}
}  // namespace AbilityRuntime
}  // namespace OHOS
