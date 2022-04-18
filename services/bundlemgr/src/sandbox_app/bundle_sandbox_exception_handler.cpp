/*
 * Copyright (c) 2022 Huawei Device Co., Ltd.
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

#include "bundle_sandbox_exception_handler.h"

#include <dirent.h>
#include <sys/stat.h>
#include <thread>
#include <unistd.h>

#include "bundle_constants.h"
#include "installd_client.h"

namespace OHOS {
namespace AppExecFwk {
namespace {
const std::string FIRST_DATA_DIR = "/data/app/el1/";
const std::string EL1 = "el1";
const std::string EL2 = "el2";
} // namespace

BundleSandboxExceptionHandler::BundleSandboxExceptionHandler(
    const std::shared_ptr<BundleDataStorageDatabase> &dataStorage) : dataStorage_(dataStorage)
{
    APP_LOGD("create bundle excepetion handler instance");
}

BundleSandboxExceptionHandler::~BundleSandboxExceptionHandler()
{
    APP_LOGD("destroy bundle excepetion handler instance");
}

void BundleSandboxExceptionHandler::RemoveSandboxAppDataDir(const std::string &bundleName)
{
    APP_LOGD("start to remove sandbox dir of %{public}s", bundleName.c_str());
    if (bundleName.empty()) {
        return;
    }

    std::thread t(ScanDataDir, std::ref(bundleName));
    t.detach();
}

void BundleSandboxExceptionHandler::ScanDataDir(const std::string &bundleName)
{
    DIR* dir = opendir(FIRST_DATA_DIR.c_str());
    if (dir == nullptr) {
        APP_LOGD("open dir failed :%{public}s", FIRST_DATA_DIR.c_str());
        return;
    }

    struct dirent *entry = nullptr;
    while ((entry = readdir(dir)) != nullptr) {
        if (strcmp(entry->d_name, ".") == 0 || strcmp(entry->d_name, "..") == 0) {
            continue;
        }
        std::string dataDirFirst = FIRST_DATA_DIR + entry->d_name + "/base/";
        std::string dataDirSecond = FIRST_DATA_DIR + entry->d_name + "/database/";
        RemoveDataDir(bundleName, dataDirFirst);
        RemoveDataDir(bundleName, dataDirSecond);
    }
}

void BundleSandboxExceptionHandler::RemoveDataDir(const std::string &bundleName, const std::string &pathDir)
{
    if (pathDir.empty()) {
        return;
    }
    APP_LOGD("start to remove data dir %{public}s", pathDir.c_str());
    DIR* dir = opendir(pathDir.c_str());
    if (dir == nullptr) {
        APP_LOGD("open dir failed :%{private}s", pathDir.c_str());
        return;
    }

    struct dirent *entry = nullptr;
    while ((entry = readdir(dir)) != nullptr) {
        if (strcmp(entry->d_name, ".") == 0 || strcmp(entry->d_name, "..") == 0) {
            continue;
        }
        std::string dataDir = pathDir + entry->d_name;
        APP_LOGD("current data dir is %{public}s", dataDir.c_str());
        if (dataDir.find(bundleName + Constants::FILE_UNDERLINE) != std::string::npos) {
            ErrCode result = InstalldClient::GetInstance()->RemoveDir(dataDir);
            if (result != ERR_OK) {
                APP_LOGW("fail to remove data dir %{public}s, error is %{public}d", dataDir.c_str(), result);
            }
            auto tmpStr = dataDir.replace(dataDir.find(EL1), EL1.length(), EL2);
            result = InstalldClient::GetInstance()->RemoveDir(tmpStr);
            if (result != ERR_OK) {
                APP_LOGW("fail to remove data dir %{public}s, error is %{public}d", dataDir.c_str(), result);
            }
        }
    }
}
} // AppExecFwk
} // OHOS