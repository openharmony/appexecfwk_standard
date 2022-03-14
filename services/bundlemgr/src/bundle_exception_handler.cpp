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

#include "bundle_exception_handler.h"

#include <dirent.h>
#include <sys/stat.h>
#include <unistd.h>

#include "bundle_constants.h"
#include "bundle_permission_mgr.h"
#include "installd_client.h"

namespace OHOS {
namespace AppExecFwk {
BundleExceptionHandler::BundleExceptionHandler(const std::shared_ptr<BundleDataStorageDatabase> &dataStorage)
    : dataStorage_(dataStorage)
{
    APP_LOGD("create bundle excepetion handler instance");
}

BundleExceptionHandler::~BundleExceptionHandler()
{
    APP_LOGD("destroy bundle excepetion handler instance");
}


void BundleExceptionHandler::HandleInvalidBundle(InnerBundleInfo &info, bool &isBundleValid)
{
    auto mark = info.GetInstallMark();
    if (mark.status == InstallExceptionStatus::INSTALL_FINISH) {
        APP_LOGD("bundle %{public}s is under correct installation status", info.GetBundleName().c_str());
        return;
    }

    std::string appCodePath = GetBundleAndDataDir(info, Constants::APP_CODE_DIR);
    std::string appDataPath = GetBundleAndDataDir(info, Constants::APP_DATA_DIR);
    auto moduleDir = appCodePath.empty() ? "" : (appCodePath + Constants::PATH_SEPARATOR + mark.packageName);
    auto moduleDataDir = appDataPath.empty() ? "" : (appDataPath + Constants::PATH_SEPARATOR + mark.packageName);
    // install and update failed before service restart
    if (mark.status == InstallExceptionStatus::INSTALL_START && RemoveBundleAndDataDir(appCodePath, appDataPath)) {
        DeleteBundleInfoFromStorage(info);
        isBundleValid = false;
    } else if (mark.status == InstallExceptionStatus::UPDATING_EXISTED_START) {
        if (InstalldClient::GetInstance()->RemoveDir(moduleDir + Constants::TMP_SUFFIX) == ERR_OK) {
            info.SetInstallMark(mark.bundleName, mark.packageName, InstallExceptionStatus::INSTALL_FINISH);
            info.SetBundleStatus(InnerBundleInfo::BundleStatus::ENABLED);
        }
    } else if (mark.status == InstallExceptionStatus::UPDATING_NEW_START &&
        RemoveBundleAndDataDir(moduleDir, moduleDataDir)) {
        info.SetInstallMark(mark.bundleName, mark.packageName, InstallExceptionStatus::INSTALL_FINISH);
        info.SetBundleStatus(InnerBundleInfo::BundleStatus::ENABLED);
    } else if (mark.status == InstallExceptionStatus::UNINSTALL_BUNDLE_START &&
        RemoveBundleAndDataDir(appCodePath, appDataPath)) {   // continue to uninstall
        DeleteBundleInfoFromStorage(info);
        isBundleValid = false;
    } else if (mark.status == InstallExceptionStatus::UNINSTALL_PACKAGE_START) {
        if (info.IsOnlyModule(mark.packageName) && RemoveBundleAndDataDir(appCodePath, appDataPath)) {
            DeleteBundleInfoFromStorage(info);
            isBundleValid = false;
            return;
        }
        if (RemoveBundleAndDataDir(moduleDir, moduleDataDir)) {
            info.RemoveModuleInfo(mark.packageName);
            info.SetInstallMark(mark.bundleName, mark.packageName, InstallExceptionStatus::INSTALL_FINISH);
            info.SetBundleStatus(InnerBundleInfo::BundleStatus::ENABLED);
        }
    } else if (mark.status == InstallExceptionStatus::UPDATING_FINISH) {
        if (InstalldClient::GetInstance()->RenameModuleDir(moduleDir + Constants::TMP_SUFFIX, moduleDir) == ERR_OK) {
            info.SetInstallMark(mark.bundleName, mark.packageName, InstallExceptionStatus::INSTALL_FINISH);
        }
    } else {
        APP_LOGD("bundle %{public}s is under unknown installation status", info.GetBundleName().c_str());
    }
}

bool BundleExceptionHandler::RemoveBundleAndDataDir(const std::string &bundleDir, const std::string &dataDir) const
{
    ErrCode result = InstalldClient::GetInstance()->RemoveDir(bundleDir);
    if (result != ERR_OK) {
        APP_LOGE("fail to remove bundle dir %{public}s, error is %{public}d", bundleDir.c_str(), result);
        return false;
    }
    result = InstalldClient::GetInstance()->RemoveDir(dataDir);
    if (result != ERR_OK) {
        APP_LOGE("fail to remove data dir %{public}s, error is %{public}d", dataDir.c_str(), result);
        return false;
    }
    return true;
}

std::string BundleExceptionHandler::GetBundleAndDataDir(const InnerBundleInfo &info, const std::string &path)
{
    std::string innerBasePath = Constants::PATH_SEPARATOR + Constants::USER_ACCOUNT_DIR + Constants::FILE_UNDERLINE +
                        std::to_string(info.GetUserId()) + Constants::PATH_SEPARATOR;
    std::string basePath;
    APP_LOGD("App type is %{public}d", static_cast<int32_t>(info.GetAppType()));
    switch (info.GetAppType()) {
        case Constants::AppType::SYSTEM_APP:
            basePath = Constants::SYSTEM_APP_INSTALL_PATH + innerBasePath + path;
            break;
        case Constants::AppType::THIRD_SYSTEM_APP:
            basePath = Constants::THIRD_SYSTEM_APP_INSTALL_PATH + innerBasePath + path;
            break;
        case Constants::AppType::THIRD_PARTY_APP:
            basePath = Constants::THIRD_PARTY_APP_INSTALL_PATH + innerBasePath + path;
            break;
        default:
            APP_LOGE("App type error");
            return std::string();
    }
    APP_LOGD("base path is %{private}s", basePath.c_str());
    DIR* dir = opendir(basePath.c_str());
    if (dir == nullptr) {
        APP_LOGE("GetBundleAndDataDir open bundle dir:%{private}s is failure", basePath.c_str());
        return std::string();
    }

    if (basePath.back() != Constants::FILE_SEPARATOR_CHAR) {
        basePath.append(Constants::PATH_SEPARATOR);
    }

    struct dirent *entry = nullptr;
    while ((entry = readdir(dir)) != nullptr) {
        const std::string innerPath = basePath + entry->d_name;
        if (innerPath.find(info.GetBundleName()) != std::string::npos) {
            APP_LOGD("find bundle path or bundle data path %{private}s", innerPath.c_str());
            closedir(dir);
            return innerPath;
        }
    }
    closedir(dir);
    return std::string();
}

void BundleExceptionHandler::DeleteBundleInfoFromStorage(const InnerBundleInfo &info)
{
    auto storage = dataStorage_.lock();
    if (storage) {
        APP_LOGD("remove bundle info of %{public}s from the storage", info.GetBundleName().c_str());
        storage->DeleteStorageBundleInfo(Constants::CURRENT_DEVICE_ID, info);
    } else {
        APP_LOGE(" fail to remove bundle info of %{public}s from the storage", info.GetBundleName().c_str());
    }
}
}  // namespace AppExecFwk
}  // namespace OHOS