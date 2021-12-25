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
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>

#include "app_log_wrapper.h"
#include "bundle_constants.h"
#include "directory_ex.h"
#include "installd/installd_operator.h"

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

ErrCode InstalldHostImpl::CreateBundleDataDir(const std::string &bundleDataDir, const int uid, const int gid)
{
    if (bundleDataDir.empty() || uid < 0 || gid < 0) {
        APP_LOGE("Calling the function CreateBundleDataDir with invalid param");
        return ERR_APPEXECFWK_INSTALLD_PARAM_ERROR;
    }
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

}  // namespace AppExecFwk
}  // namespace OHOS