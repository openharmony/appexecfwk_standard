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

#ifndef FOUNDATION_APPEXECFWK_SERVICES_BUNDLEMGR_INCLUDE_INSTALLD_HOST_IMPL_H
#define FOUNDATION_APPEXECFWK_SERVICES_BUNDLEMGR_INCLUDE_INSTALLD_HOST_IMPL_H

#include "ipc/installd_host.h"
#include "installd/installd_operator.h"

namespace OHOS {
namespace AppExecFwk {
class InstalldHostImpl : public InstalldHost {
public:
    InstalldHostImpl();
    virtual ~InstalldHostImpl();
    /**
     * @brief Create a bundle code directory.
     * @param bundleDir Indicates the bundle code directory path that to be created.
     * @return Returns ERR_OK if the bundle directory created successfully; returns error code otherwise.
     */
    virtual ErrCode CreateBundleDir(const std::string &bundleDir) override;
    /**
     * @brief Extract the files of a HAP module to the code directory.
     * @param srcModulePath Indicates the HAP file path.
     * @param targetPath Indicates the code directory path that the HAP to be extracted to.
     * @return Returns ERR_OK if the HAP file extracted successfully; returns error code otherwise.
     */
    virtual ErrCode ExtractModuleFiles(const std::string &srcModulePath, const std::string &targetPath) override;
    /**
     * @brief Rename the module directory from temporaily path to the real path.
     * @param oldPath Indicates the old path name.
     * @param newPath Indicates the new path name.
     * @return Returns ERR_OK if the module directory renamed successfully; returns error code otherwise.
     */
    virtual ErrCode RenameModuleDir(const std::string &oldPath, const std::string &newPath) override;
    /**
     * @brief Create a bundle data directory.
     * @param bundleDataDir Indicates the bundle data directory path that to be created.
     * @param userid Indicates userid to be set to the directory.
     * @param uid Indicates uid to be set to the directory.
     * @param gid Indicates gid to be set to the directory.
     * @param onlyOneUser Indicates is only one user or not.
     * @return Returns ERR_OK if the bundle data directory created successfully; returns error code otherwise.
     */
    virtual ErrCode CreateBundleDataDir(const std::string &bundleDataDir, const int userid,
        const int uid, const int gid, bool onlyOneUser = true) override;
    /**
     * @brief Remove a bundle data directory.
     * @param bundleName Indicates the bundleName data directory path that to be created.
     * @param userid Indicates userid to be set to the directory.
     * @return Returns ERR_OK if the bundle data directory created successfully; returns error code otherwise.
     */
    virtual ErrCode RemoveBundleDataDir(
        const std::string &bundleName, const int userid) override;
    /**
     * @brief Create a module and it's abilities data directory.
     * @param bundleDir Indicates the module data directory path that to be created.
     * @param abilityDirs Indicates the abilities data directory name that to be created.
     * @param uid Indicates uid to be set to the directory.
     * @param gid Indicates gid to be set to the directory.
     * @return Returns ERR_OK if the data directories created successfully; returns error code otherwise.
     */
    virtual ErrCode CreateModuleDataDir(const std::string &ModuleDir, const std::vector<std::string> &abilityDirs,
        const int uid, const int gid) override;
    /**
     * @brief Remove a module data directory.
     * @param ModuleDir Indicates the module data directory path that to be created.
     * @param userid Indicates userid to be set to the directory.
     * @return Returns ERR_OK if the data directories created successfully; returns error code otherwise.
     */
    virtual ErrCode RemoveModuleDataDir(const std::string &ModuleDir, const int userid) override;
    /**
     * @brief Remove a directory.
     * @param dir Indicates the directory path that to be removed.
     * @return Returns ERR_OK if the  directory removed successfully; returns error code otherwise.
     */
    virtual ErrCode RemoveDir(const std::string &dir) override;
    /**
     * @brief Clean all files in a bundle data directory.
     * @param bundleDir Indicates the data directory path that to be cleaned.
     * @return Returns ERR_OK if the data directory cleaned successfully; returns error code otherwise.
     */
    virtual ErrCode CleanBundleDataDir(const std::string &bundleDir) override;
    /**
     * @brief Get bundle Stats.
     * @param bundleName Indicates the bundle name.
     * @param userId Indicates the user Id.
     * @param bundleStats Indicates the bundle Stats.
     * @return Returns ERR_OK if get stats successfully; returns error code otherwise.
     */
    virtual ErrCode GetBundleStats(
        const std::string &bundleName, const int32_t userId, std::vector<int64_t> &bundleStats) override;
private:
    std::string GetBundleDataDir(const std::string &el, const int userid) const;
    ErrCode CreateNewBundleDataDir(const std::string &bundleName, const int userid, const int uid, const int gid) const;
};
}  // namespace AppExecFwk
}  // namespace OHOS
#endif  // FOUNDATION_APPEXECFWK_SERVICES_BUNDLEMGR_INCLUDE_INSTALLD_HOST_IMPL_H