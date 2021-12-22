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

#include "bundle_util.h"

#include <cinttypes>
#include <dirent.h>
#include <sys/stat.h>
#include <sys/statfs.h>
#include <unistd.h>

#include "app_log_wrapper.h"
#include "bundle_constants.h"
#include "directory_ex.h"
#include "string_ex.h"

namespace OHOS {
namespace AppExecFwk {

ErrCode BundleUtil::CheckFilePath(const std::string &bundlePath, std::string &realPath)
{
    if (!CheckFileName(bundlePath)) {
        APP_LOGE("bundle file path invalid");
        return ERR_APPEXECFWK_INSTALL_FILE_PATH_INVALID;
    }
    if (!CheckFileType(bundlePath, Constants::INSTALL_FILE_SUFFIX)) {
        APP_LOGE("file is not hap");
        return ERR_APPEXECFWK_INSTALL_INVALID_HAP_NAME;
    }
    if (!PathToRealPath(bundlePath, realPath)) {
        APP_LOGE("file is not real path");
        return ERR_APPEXECFWK_INSTALL_FILE_PATH_INVALID;
    }
    if (access(realPath.c_str(), F_OK) != 0) {
        APP_LOGE("can not access the bundle file path: %{private}s", realPath.c_str());
        return ERR_APPEXECFWK_INSTALL_INVALID_BUNDLE_FILE;
    }
    if (!CheckFileSize(realPath, Constants::MAX_HAP_SIZE)) {
        APP_LOGE("file size is larger than max hap size Max size is: %{public}" PRId64, Constants::MAX_HAP_SIZE);
        return ERR_APPEXECFWK_INSTALL_INVALID_HAP_SIZE;
    }
    return ERR_OK;
}

ErrCode BundleUtil::CheckFilePath(const std::vector<std::string> &bundlePaths, std::vector<std::string> &realPaths)
{
    // there are three cases for bundlePaths:
    // 1. one bundle direction in the bundlePaths, some hap files under this bundle direction.
    // 2. one hap direction in the bundlePaths.
    // 3. some hap file directions in the bundlePaths.
    APP_LOGD("check file path");
    if (bundlePaths.empty()) {
        APP_LOGE("bundle file paths invalid");
        return ERR_APPEXECFWK_INSTALL_FILE_PATH_INVALID;
    }
    ErrCode ret = ERR_OK;

    if (bundlePaths.size() == 1) {
        struct stat s;
        std::string bundlePath = bundlePaths.front();
        std::string realPath = "";
        if (stat(bundlePath.c_str(), &s) == 0) {
            // it is a direction
            if ((s.st_mode & S_IFDIR) && !GetHapFilesFromBundlePath(bundlePath, realPaths)) {
                APP_LOGE("GetHapFilesFromBundlePath failed with bundlePath:%{private}s", bundlePaths.front().c_str());
                return ERR_APPEXECFWK_INSTALL_FILE_PATH_INVALID;
            }
            // it is a file
            if ((s.st_mode & S_IFREG) && (ret = CheckFilePath(bundlePaths.front(), realPath)) == ERR_OK) {
                realPaths.emplace_back(realPath);
            }
            return ret;
        } else {
            APP_LOGE("bundlePath is not existed with :%{private}s", bundlePaths.front().c_str());
            return ERR_APPEXECFWK_INSTALL_FILE_PATH_INVALID;
        }
    } else {
        for (const std::string& bundlePath : bundlePaths) {
            std::string realPath = "";
            ret = CheckFilePath(bundlePath, realPath);
            if (ret != ERR_OK) {
                return ret;
            }
            realPaths.emplace_back(realPath);
        }
    }
    APP_LOGD("finish check file path");
    return ret;
}

bool BundleUtil::CheckFileType(const std::string &fileName, const std::string &extensionName)
{
    APP_LOGD("path is %{public}s, support suffix is %{public}s", fileName.c_str(), extensionName.c_str());
    if (!CheckFileName(fileName)) {
        return false;
    }

    auto position = fileName.rfind('.');
    if (position == std::string::npos) {
        APP_LOGE("filename no extension name");
        return false;
    }

    std::string suffixStr = fileName.substr(position);
    return LowerStr(suffixStr) == extensionName;
}

bool BundleUtil::CheckFileName(const std::string &fileName)
{
    if (fileName.empty()) {
        APP_LOGE("the file name is empty");
        return false;
    }
    if (fileName.size() > Constants::PATH_MAX_SIZE) {
        APP_LOGE("bundle file path length %{public}zu too long", fileName.size());
        return false;
    }
    return true;
}

bool BundleUtil::CheckFileSize(const std::string &bundlePath, const int64_t fileSize)
{
    struct stat fileInfo = { 0 };
    if (stat(bundlePath.c_str(), &fileInfo) != 0) {
        APP_LOGE("call stat error");
        return false;
    }
    if (fileInfo.st_size > fileSize) {
        return false;
    }
    return true;
}

bool BundleUtil::CheckSystemSize(const std::string &bundlePath, const std::string &diskPath)
{
    struct statfs diskInfo = { 0 };
    if (statfs(diskPath.c_str(), &diskInfo) != 0) {
        APP_LOGE("call statfs error");
        return false;
    }
    int64_t freeSize = diskInfo.f_bfree * diskInfo.f_bsize;
    APP_LOGD("left free size in the disk path is %{public}" PRId64, freeSize / Constants::ONE_GB);

    return CheckFileSize(bundlePath, freeSize);
}

bool BundleUtil::GetHapFilesFromBundlePath(const std::string& currentBundlePath, std::vector<std::string>& hapFileList)
{
    APP_LOGD("GetHapFilesFromBundlePath with path is %{public}s", currentBundlePath.c_str());
    if (currentBundlePath.empty()) {
        return false;
    }
    DIR* dir = opendir(currentBundlePath.c_str());
    if (dir == nullptr) {
        APP_LOGE("GetHapFilesFromBundlePath open bundle dir:%{private}s is failure", currentBundlePath.c_str());
        return false;
    }
    std::string bundlePath = currentBundlePath;
    if (bundlePath.back() != Constants::FILE_SEPARATOR_CHAR) {
        bundlePath.append(Constants::PATH_SEPARATOR);
    }
    struct dirent *entry = NULL;
    while ((entry = readdir(dir)) != nullptr) {
        if (strcmp(entry->d_name, ".") == 0 || strcmp(entry->d_name, "..") == 0) {
            continue;
        }
        const std::string hapFilePath = bundlePath + entry->d_name;
        std::string realPath = "";
        if (CheckFilePath(hapFilePath, realPath) != ERR_OK) {
            APP_LOGE("find invalid hap path %{public}s", hapFilePath.c_str());
            closedir(dir);
            return false;
        }
        hapFileList.emplace_back(realPath);
        APP_LOGD("find hap path %{public}s", realPath.c_str());

        if (!hapFileList.empty() && (hapFileList.size() > Constants::MAX_HAP_NUMBER)) {
            APP_LOGE("reach the max hap number 128, stop to add more.");
            closedir(dir);
            return false;
        }
    }
    closedir(dir);
    return true;
}
}  // namespace AppExecFwk
}  // namespace OHOS
