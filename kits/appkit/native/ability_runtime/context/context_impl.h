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

#ifndef ABILITY_RUNTIME_CONTEXT_IMPL_H
#define ABILITY_RUNTIME_CONTEXT_IMPL_H

#include "context.h"

#include "bundle_mgr_interface.h"

namespace OHOS {
namespace AbilityRuntime {
class ContextImpl : public Context, public std::enable_shared_from_this<ContextImpl> {
public:
    ContextImpl() = default;
    virtual ~ContextImpl() = default;

    /**
     * @brief Obtains the bundle name of the current ability.
     *
     * @return Returns the bundle name of the current ability.
     */
    std::string GetBundleName() const override;

    /**
     * @brief Obtains the path of the package containing the current ability. The returned path contains the resources,
     *  source code, and configuration files of a module.
     *
     * @return Returns the path of the package file.
     */
    std::string GetBundleCodeDir() override;

    /**
     * @brief Obtains the application-specific cache directory on the device's internal storage. The system
     * automatically deletes files from the cache directory if disk space is required elsewhere on the device.
     * Older files are always deleted first.
     *
     * @return Returns the application-specific cache directory.
     */
    std::string GetCacheDir() override;

    /**
     * @brief Obtains the temporary directory.
     *
     * @return Returns the application temporary directory.
     */
    std::string GetTempDir() override;

    /**
     * @brief Obtains the directory for storing files for the application on the device's internal storage.
     *
     * @return Returns the application file directory.
     */
    std::string GetFilesDir() override;

    /**
     * @brief Obtains the local database path.
     * If the local database path does not exist, the system creates one and returns the created path.
     *
     * @return Returns the local database file.
     */
    std::string GetDatabaseDir() override;

    /**
     * @brief Obtains the path storing the storage file of the application.
     *
     * @return Returns the local storage file.
     */
    std::string GetStorageDir() override;

    /**
     * @brief Obtains the path distributed file of the application
     *
     * @return Returns the distributed file.
     */
    std::string GetDistributedFilesDir() override;

    /**
     * @brief set the ResourceManager.
     *
     * @param the ResourceManager has been inited.
     *
     */
    void SetResourceManager(const std::shared_ptr<Global::Resource::ResourceManager> &resourceManager);

    /**
    * @brief Obtains a resource manager.
    *
    * @return Returns a ResourceManager object.
    */
    std::shared_ptr<Global::Resource::ResourceManager> GetResourceManager() const override;

    /**
     * @brief Creates a Context object for an application with the given bundle name.
     *
     * @param bundleName Indicates the bundle name of the application.
     *
     * @return Returns a Context object created for the specified application.
     */
    std::shared_ptr<Context> CreateBundleContext(const std::string &bundleName) override;

    /**
    * @brief Obtains an IBundleMgr instance.
    * You can use this instance to obtain information about the application bundle.
    *
    * @return Returns an IBundleMgr instance.
    */
    sptr<AppExecFwk::IBundleMgr> GetBundleManager() const;

    /**
     * @brief Set ApplicationInfo
     *
     * @param info ApplicationInfo instance.
     */
    void SetApplicationInfo(const std::shared_ptr<AppExecFwk::ApplicationInfo> &info);

    /**
     * @brief Obtains information about the current application. The returned application information includes basic
     * information such as the application name and application permissions.
     *
     * @return Returns the ApplicationInfo for the current application.
     */
    std::shared_ptr<AppExecFwk::ApplicationInfo> GetApplicationInfo() const override;

    /**
     * @brief Set ApplicationInfo
     *
     * @param info ApplicationInfo instance.
     */
    void SetParentContext(const std::shared_ptr<Context> &context);

    /**
     * @brief Obtains the path of the package containing the current ability. The returned path contains the resources,
     *  source code, and configuration files of a module.
     *
     * @return Returns the path of the package file.
     */
    std::string GetBundleCodePath() const override;

    /**
     * @brief Obtains the HapModuleInfo object of the application.
     *
     * @return Returns the HapModuleInfo object of the application.
     */
    std::shared_ptr<AppExecFwk::HapModuleInfo> GetHapModuleInfo() const override;

    /**
     * @brief Set HapModuleInfo
     *
     * @param hapModuleInfo HapModuleInfo instance.
     */
    void InitHapModuleInfo(const std::shared_ptr<AppExecFwk::AbilityInfo> &abilityInfo);

    /**
     * @brief Set application context
     */
    void InitAppContext();
private:
    static const std::string CONTEXT_BUNDLECODE_BASE;
    static const std::string CONTEXT_BUNDLECODE;
    static const std::string CONTEXT_BUNDLE;
    static const std::string CONTEXT_DISTRIBUTEDFILES_BASE_BEFORE;
    static const std::string CONTEXT_DISTRIBUTEDFILES_BASE_MIDDLE;
    static const std::string CONTEXT_DISTRIBUTEDFILES;
    static const std::string CONTEXT_FILE_SEPARATOR;
    static const std::string CONTEXT_DATA;
    static const std::string CONTEXT_BASE;
    static const std::string CONTEXT_PRIVATE;
    static const std::string CONTEXT_CACHES;
    static const std::string CONTEXT_STORAGE;
    static const std::string CONTEXT_DATABASE;
    static const std::string CONTEXT_TEMP;
    static const std::string CONTEXT_FILES;
    static const std::string CONTEXT_CE;
    static const int64_t CONTEXT_CREATE_BY_SYSTEM_APP;
    int flags_ = 0x00000000;

    void InitResourceManager(
        const AppExecFwk::BundleInfo &bundleInfo, const std::shared_ptr<ContextImpl> &appContext) const;
    bool IsCreateBySystemApp() const;
    std::string GetBaseDir() const;
    std::string GetCurrentAccountId() const;
    void SetFlags(int64_t flags);

    std::shared_ptr<AppExecFwk::ApplicationInfo> applicationInfo_ = nullptr;
    std::shared_ptr<Context> parentContext_ = nullptr;
    std::shared_ptr<Global::Resource::ResourceManager> resourceManager_ = nullptr;
    std::shared_ptr<AppExecFwk::HapModuleInfo> hapModuleInfo_ = nullptr;
    std::string currArea_ = CONTEXT_CE;
};
}  // namespace AbilityRuntime
}  // namespace OHOS
#endif  // ABILITY_RUNTIME_CONTEXT_IMPL_H
