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

#include "installd/installd_host_impl.h"

#include <cstdio>
#include <fstream>
#include <map>
#include <memory>
#include <sstream>
#include <string>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>

#include "app_log_wrapper.h"
#include "bundle_constants.h"
#include "directory_ex.h"
#include "installd/installd_operator.h"
#include "parameters.h"

namespace OHOS {
namespace AppExecFwk {

InstalldHostImpl::InstalldHostImpl()
{
    APP_LOGI("installd service instance is created");
}

InstalldHostImpl::~InstalldHostImpl()
{
    APP_LOGI("installd service instance is destroyed");
}

ErrCode InstalldHostImpl::CreateBundleDir(const std::string &bundleDir)
{
    if (bundleDir.empty()) {
        APP_LOGE("Calling the function CreateBundleDir with invalid param");
        return ERR_APPEXECFWK_INSTALLD_PARAM_ERROR;
    }
    if (InstalldOperator::IsExistDir(bundleDir)) {
        APP_LOGW("bundleDir %{public}s is exist", bundleDir.c_str());
        OHOS::ForceRemoveDirectory(bundleDir);
    }
    if (!InstalldOperator::MkRecursiveDir(bundleDir, true)) {
        APP_LOGE("create bundle dir %{public}s failed", bundleDir.c_str());
        return ERR_APPEXECFWK_INSTALLD_CREATE_DIR_FAILED;
    }

    auto bundleName = strrchr(bundleDir.c_str(), Constants::FILE_SEPARATOR_CHAR);
    if (!bundleName) {
        APP_LOGE("Calling the function CreateBundleDir with invalid bundleName");
        return ERR_APPEXECFWK_INSTALLD_PARAM_ERROR;
    }

    std::string newBundleDir = Constants::BUNDLE_CODE_DIR + bundleName;
    if (InstalldOperator::IsExistDir(newBundleDir)) {
        APP_LOGW("new bundle dir: %{public}s is exist", newBundleDir.c_str());
        OHOS::ForceRemoveDirectory(newBundleDir);
    }
    if (!InstalldOperator::MkRecursiveDir(newBundleDir, true)) {
        APP_LOGE("create new bundle dir %{public}s failed", newBundleDir.c_str());
        return ERR_APPEXECFWK_INSTALLD_CREATE_DIR_FAILED;
    }
    return ERR_OK;
}

ErrCode InstalldHostImpl::ExtractModuleFiles(const std::string &srcModulePath, const std::string &targetPath)
{
    APP_LOGD("ExtractModuleFiles extract original src %{public}s and target src %{public}s",
        srcModulePath.c_str(), targetPath.c_str());
    if (srcModulePath.empty() || targetPath.empty()) {
        APP_LOGE("Calling the function ExtractModuleFiles with invalid param");
        return ERR_APPEXECFWK_INSTALLD_PARAM_ERROR;
    }
    if (!InstalldOperator::MkRecursiveDir(targetPath, true)) {
        APP_LOGE("create target dir %{public}s failed", targetPath.c_str());
        return ERR_APPEXECFWK_INSTALLD_CREATE_DIR_FAILED;
    }
    if (!InstalldOperator::ExtractFiles(srcModulePath, targetPath)) {
        APP_LOGE("extract %{public}s to %{public}s failed", srcModulePath.c_str(), targetPath.c_str());
        InstalldOperator::DeleteDir(targetPath);
        return ERR_APPEXECFWK_INSTALL_DISK_MEM_INSUFFICIENT;
    }
    return ERR_OK;
}

ErrCode InstalldHostImpl::RenameModuleDir(const std::string &oldPath, const std::string &newPath)
{
    APP_LOGD("rename %{public}s to %{public}s", oldPath.c_str(), newPath.c_str());
    if (oldPath.empty() || newPath.empty()) {
        APP_LOGE("Calling the function RenameModuleDir with invalid param");
        return ERR_APPEXECFWK_INSTALLD_PARAM_ERROR;
    }
    if (!InstalldOperator::RenameDir(oldPath, newPath)) {
        APP_LOGE("rename module dir %{public}s to %{public}s failed", oldPath.c_str(), newPath.c_str());
        return ERR_APPEXECFWK_INSTALLD_RNAME_DIR_FAILED;
    }
    return ERR_OK;
}

ErrCode InstalldHostImpl::CreateBundleDataDir(const std::string &bundleDataDir,
    const int userid, const int uid, const int gid, bool onlyOneUser)
{
    if (bundleDataDir.empty() || uid < 0 || gid < 0) {
        APP_LOGE("Calling the function CreateBundleDataDir with invalid param");
        return ERR_APPEXECFWK_INSTALLD_PARAM_ERROR;
    }

    if (onlyOneUser) {
        std::string createDir;
        if (bundleDataDir.back() != Constants::PATH_SEPARATOR[0]) {
            createDir = bundleDataDir + Constants::PATH_SEPARATOR;
        } else {
            createDir = bundleDataDir;
        }

        if (!InstalldOperator::MkOwnerDir(createDir + Constants::DATA_DIR, true, uid, gid)) {
            APP_LOGE("CreateBundleDataDir MkOwnerDir DATA_DIR failed");
            return ERR_APPEXECFWK_INSTALLD_CREATE_DIR_FAILED;
        }

        if (!InstalldOperator::MkOwnerDir(createDir + Constants::DATA_BASE_DIR, true, uid, gid)) {
            APP_LOGE("CreateBundleDataDir MkOwnerDir DATA_BASE_DIR failed");
            return ERR_APPEXECFWK_INSTALLD_CREATE_DIR_FAILED;
        }

        if (!InstalldOperator::MkOwnerDir(createDir + Constants::CACHE_DIR, true, uid, gid)) {
            APP_LOGE("CreateBundleDataDir MkOwnerDir CACHE_DIR failed");
            return ERR_APPEXECFWK_INSTALLD_CREATE_DIR_FAILED;
        }

        if (!InstalldOperator::MkOwnerDir(createDir + Constants::SHARED_PREFERENCE_DIR, true, uid, gid)) {
            APP_LOGE("CreateBundleDataDir MkOwnerDir SHARED_PREFERENCE_DIR failed");
            return ERR_APPEXECFWK_INSTALLD_CREATE_DIR_FAILED;
        }
    }

    std::string bundleName = strrchr(bundleDataDir.c_str(), Constants::FILE_SEPARATOR_CHAR);
    if (CreateNewBundleDataDir(bundleName, userid, uid, gid) != ERR_OK) {
        APP_LOGE("CreateNewBundleDataDir MkOwnerDir failed");
    }
    return ERR_OK;
}

ErrCode InstalldHostImpl::CreateNewBundleDataDir(
    const std::string &bundleName, const int userid, const int uid, const int gid) const
{
    if (bundleName.empty() || userid < 0 || uid < 0 || gid < 0) {
        APP_LOGE("Calling the function CreateBundleDataDir with invalid param");
        return ERR_APPEXECFWK_INSTALLD_PARAM_ERROR;
    }
    for (const auto el : Constants::BUNDLE_EL) {
        std::string bundleDataDir = GetBundleDataDir(el, userid) + Constants::BASE + bundleName;
        if (!InstalldOperator::MkOwnerDir(bundleDataDir, S_IRWXU, uid, gid)) {
            APP_LOGE("CreateBundledatadir MkOwnerDir failed");
            return ERR_APPEXECFWK_INSTALLD_CREATE_DIR_FAILED;
        }
        if (el == Constants::BUNDLE_EL[1]) {
            for (const auto dir : Constants::BUNDLE_DATA_DIR) {
                if (!InstalldOperator::MkOwnerDir(bundleDataDir + dir, S_IRWXU, uid, gid)) {
                    APP_LOGE("CreateBundledatadir MkOwnerDir el2 failed");
                    return ERR_APPEXECFWK_INSTALLD_CREATE_DIR_FAILED;
                }
            }
        }
    }
    for (const auto el : Constants::DATABASE_EL) {
        std::string databaseDir = GetBundleDataDir(el, userid) + Constants::DATABASE + bundleName;
        if (!InstalldOperator::MkOwnerDir(databaseDir, S_IRWXU, uid, gid)) {
            APP_LOGE("CreateBundle databaseDir MkOwnerDir failed");
            return ERR_APPEXECFWK_INSTALLD_CREATE_DIR_FAILED;
        }
    }
    if (system::GetBoolParameter(Constants::DISTRIBUTED_FILE_PROPERTY, false)) {
        std::string distributedfile = Constants::DISTRIBUTED_FILE;
        distributedfile = distributedfile.replace(distributedfile.find("%"), 1, std::to_string(userid));
        if (!InstalldOperator::MkOwnerDir(distributedfile + bundleName, S_IRWXU, uid, gid)) {
            APP_LOGE("CreateBundle distributedfile MkOwnerDir failed");
            return ERR_APPEXECFWK_INSTALLD_CREATE_DIR_FAILED;
        }
    }
    return ERR_OK;
}

ErrCode InstalldHostImpl::RemoveBundleDataDir(const std::string &bundleName, const int userid)
{
    APP_LOGD("InstalldHostImpl::RemoveBundleDataDir bundleName:%{public}s", bundleName.c_str());
    if (bundleName.empty() || userid < 0) {
        APP_LOGE("Calling the function CreateBundleDataDir with invalid param");
        return ERR_APPEXECFWK_INSTALLD_PARAM_ERROR;
    }
    for (const auto el : Constants::BUNDLE_EL) {
        std::string bundleDataDir = GetBundleDataDir(el, userid) + Constants::BASE + bundleName;
        if (!InstalldOperator::DeleteDir(bundleDataDir)) {
            APP_LOGE("remove dir %{public}s failed", bundleDataDir.c_str());
            return ERR_APPEXECFWK_INSTALLD_REMOVE_DIR_FAILED;
        }
    }
    for (const auto el : Constants::DATABASE_EL) {
        std::string databaseDir = GetBundleDataDir(el, userid) + Constants::DATABASE + bundleName;
        if (!InstalldOperator::DeleteDir(databaseDir)) {
            APP_LOGE("remove dir %{public}s failed", databaseDir.c_str());
            return ERR_APPEXECFWK_INSTALLD_REMOVE_DIR_FAILED;
        }
    }
    return ERR_OK;
}

ErrCode InstalldHostImpl::CreateModuleDataDir(
    const std::string &ModuleDir, const std::vector<std::string> &abilityDirs, const int uid, const int gid)
{
    if (ModuleDir.empty() || uid < 0 || gid < 0) {
        APP_LOGE("Calling the function CreateModuleDataDir with invalid param");
        return ERR_APPEXECFWK_INSTALLD_PARAM_ERROR;
    }
    std::string createDir;
    if (ModuleDir.back() != Constants::PATH_SEPARATOR[0]) {
        createDir = ModuleDir + Constants::PATH_SEPARATOR;
    } else {
        createDir = ModuleDir;
    }

    if (!InstalldOperator::MkOwnerDir(createDir + Constants::SHARED_DIR, true, uid, gid)) {
        APP_LOGE("CreateModuleDataDir MkOwnerDir %{public}s failed", Constants::SHARED_DIR.c_str());
        return ERR_APPEXECFWK_INSTALLD_CREATE_DIR_FAILED;
    }

    for (auto &abilityDir : abilityDirs) {
        std::string dataDir = createDir + abilityDir + Constants::PATH_SEPARATOR + Constants::DATA_DIR;
        if (!InstalldOperator::MkOwnerDir(dataDir, true, uid, gid)) {
            APP_LOGE("CreateModuleDataDir MkOwnerDir %{public}s failed", dataDir.c_str());
            return ERR_APPEXECFWK_INSTALLD_CREATE_DIR_FAILED;
        }
        std::string cacheDir = createDir + abilityDir + Constants::PATH_SEPARATOR + Constants::CACHE_DIR;
        if (!InstalldOperator::MkOwnerDir(cacheDir, true, uid, gid)) {
            APP_LOGE("CreateModuleDataDir MkOwnerDir %{public}s failed", cacheDir.c_str());
            return ERR_APPEXECFWK_INSTALLD_CREATE_DIR_FAILED;
        }
        std::string dataBaseDir = createDir + abilityDir + Constants::PATH_SEPARATOR + Constants::DATA_BASE_DIR;
        if (!InstalldOperator::MkOwnerDir(dataBaseDir, true, uid, gid)) {
            APP_LOGE("CreateModuleDataDir MkOwnerDir %{public}s failed", dataBaseDir.c_str());
            return ERR_APPEXECFWK_INSTALLD_CREATE_DIR_FAILED;
        }
        std::string sharedDir = createDir + abilityDir + Constants::PATH_SEPARATOR + Constants::SHARED_PREFERENCE_DIR;
        if (!InstalldOperator::MkOwnerDir(sharedDir, true, uid, gid)) {
            APP_LOGE("CreateModuleDataDir MkOwnerDir %{public}s failed", sharedDir.c_str());
            return ERR_APPEXECFWK_INSTALLD_CREATE_DIR_FAILED;
        }
    }
    return ERR_OK;
}

ErrCode InstalldHostImpl::RemoveModuleDataDir(const std::string &ModuleDir, const int userid)
{
    APP_LOGD("InstalldHostImpl::RemoveModuleDataDir ModuleDir:%{public}s", ModuleDir.c_str());
    if (ModuleDir.empty() || userid < 0) {
        APP_LOGE("Calling the function CreateModuleDataDir with invalid param");
        return ERR_APPEXECFWK_INSTALLD_PARAM_ERROR;
    }

    for (const auto el : Constants::BUNDLE_EL) {
        std::string moduleDataDir = GetBundleDataDir(el, userid) + Constants::BASE + ModuleDir;
        if (!InstalldOperator::DeleteDir(moduleDataDir)) {
            APP_LOGE("remove dir %{public}s failed", moduleDataDir.c_str());
        }
    }
    return ERR_OK;
}

ErrCode InstalldHostImpl::RemoveDir(const std::string &dir)
{
    if (dir.empty()) {
        APP_LOGE("Calling the function RemoveDir with invalid param");
        return ERR_APPEXECFWK_INSTALLD_PARAM_ERROR;
    }
    if (!InstalldOperator::DeleteDir(dir)) {
        APP_LOGE("remove dir %{public}s failed", dir.c_str());
        return ERR_APPEXECFWK_INSTALLD_REMOVE_DIR_FAILED;
    }
    return ERR_OK;
}

ErrCode InstalldHostImpl::CleanBundleDataDir(const std::string &dataDir)
{
    if (dataDir.empty()) {
        APP_LOGE("Calling the function CleanBundleDataDir with invalid param");
        return ERR_APPEXECFWK_INSTALLD_PARAM_ERROR;
    }

    if (!InstalldOperator::DeleteFiles(dataDir)) {
        APP_LOGE("CleanBundleDataDir delete files failed");
        return ERR_APPEXECFWK_INSTALLD_CLEAN_DIR_FAILED;
    }
    return ERR_OK;
}

std::string InstalldHostImpl::GetBundleDataDir(const std::string &el, const int userid) const
{
    std::string dataDir = Constants::BUNDLE_APP_DATA_BASE_DIR +
                          el +
                          Constants::FILE_SEPARATOR_CHAR +
                          std::to_string(userid);
    return dataDir;
}

}  // namespace AppExecFwk
}  // namespace OHOS