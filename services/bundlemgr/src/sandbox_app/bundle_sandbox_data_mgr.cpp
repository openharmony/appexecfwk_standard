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

#include "bundle_sandbox_data_mgr.h"

#include "app_log_wrapper.h"
#include "bundle_constants.h"
#include "bundle_mgr_service.h"

namespace OHOS {
namespace AppExecFwk {
BundleSandboxDataMgr::BundleSandboxDataMgr()
{
    APP_LOGI("BundleSandboxDataMgr instance is created");
}

BundleSandboxDataMgr::~BundleSandboxDataMgr()
{
    APP_LOGI("BundleSandboxDataMgr instance is destroyed");
}

void BundleSandboxDataMgr::SaveSandboxAppInfo(const InnerBundleInfo &info, const int32_t &appIndex)
{
    APP_LOGD("SaveSandboxAppInfo begin");
    std::string bundleName = info.GetBundleName();
    if (bundleName.empty()) {
        APP_LOGE("SaveSandboxAppInfo bundleName is empty");
        return;
    }
    std::string key = bundleName + Constants::FILE_UNDERLINE + std::to_string(appIndex);
    std::unique_lock<std::shared_mutex> lock(sandboxAppMutex_);
    sandboxAppInfos_[key] = info;
    APP_LOGD("save sandbox app %{public}s info successfully", key.c_str());
}

void BundleSandboxDataMgr::DeleteSandboxAppInfo(const std::string &bundleName, const int32_t &appIndex)
{
    APP_LOGD("DeleteSandboxAppInfo begin");
    if (bundleName.empty()) {
        APP_LOGE("DeleteSandboxAppInfo bundleName is empty");
        return;
    }
    auto key = bundleName + Constants::FILE_UNDERLINE + std::to_string(appIndex);
    std::unique_lock<std::shared_mutex> lock(sandboxAppMutex_);
    auto ret = sandboxAppInfos_.erase(key);
    if (ret == 0) {
        APP_LOGE("delete sandbox app info failed due to no sandbox app in the dataMgr");
        return;
    }
    APP_LOGD("delete sandbox app %{public}s info successfully", key.c_str());
}

ErrCode BundleSandboxDataMgr::GetSandboxAppInfo(
    const std::string &bundleName, const int32_t &appIndex, int32_t &userId, InnerBundleInfo &info) const
{
    APP_LOGD("GetSandboxAppInfo begin");
    if (bundleName.empty()) {
        APP_LOGE("GetSandboxAppInfo bundleName is empty");
        return ERR_APPEXECFWK_SANDBOX_INSTALL_PARAM_ERROR;
    }
    if (appIndex <= Constants::INITIAL_APP_INDEX) {
        APP_LOGE("GetSandboxAppInfo appIndex is invalid");
        return ERR_APPEXECFWK_SANDBOX_INSTALL_PARAM_ERROR;
    }
    if (userId < Constants::DEFAULT_USERID) {
        APP_LOGE("userId(%{public}d) is invalid.", userId);
        return ERR_APPEXECFWK_SANDBOX_INSTALL_PARAM_ERROR;
    }

    auto key = bundleName + Constants::FILE_UNDERLINE + std::to_string(appIndex);

    {
        std::shared_lock<std::shared_mutex> lock(sandboxAppMutex_);
        auto it = sandboxAppInfos_.find(key);
        if (it == sandboxAppInfos_.end()) {
            APP_LOGE("GetSandboxAppInfo no sandbox app info can be found");
            return ERR_APPEXECFWK_SANDBOX_INSTALL_NO_SANDBOX_APP_INFO;
        }

        InnerBundleUserInfo userInfo;
        if (!(it->second).GetInnerBundleUserInfo(userId, userInfo)) {
            APP_LOGE("the sandbox app is not installed at this user %{public}d", userId);
            return ERR_APPEXECFWK_SANDBOX_INSTALL_NOT_INSTALLED_AT_SPECIFIED_USERID;
        }
        info = it->second;
    }

    APP_LOGD("GetSandboxAppInfo successfully");
    return ERR_OK;
}

ErrCode BundleSandboxDataMgr::GetSandboxAppBundleInfo(
    const std::string &bundleName, const int32_t &appIndex, const int32_t &userId, BundleInfo &info) const
{
    APP_LOGD("GetSandboxAppBundleInfo begin");
    InnerBundleInfo innerBundleInfo;
    int32_t requestUserId = userId;
    ErrCode result = ERR_OK;
    if ((result = GetSandboxAppInfo(bundleName, appIndex, requestUserId, innerBundleInfo)) != ERR_OK) {
        APP_LOGE("GetSandboxAppBundleInfo get sandbox app info failed");
        return result;
    }
    InnerBundleUserInfo userInfo;
    if (!innerBundleInfo.GetInnerBundleUserInfo(requestUserId, userInfo)) {
        APP_LOGE("the origin application is not installed at current user");
        return ERR_APPEXECFWK_SANDBOX_INSTALL_NOT_INSTALLED_AT_SPECIFIED_USERID;
    }

    innerBundleInfo.GetBundleInfo(
        BundleFlag::GET_BUNDLE_DEFAULT |
        BundleFlag::GET_BUNDLE_WITH_ABILITIES |
        BundleFlag::GET_BUNDLE_WITH_REQUESTED_PERMISSION |
        BundleFlag::GET_BUNDLE_WITH_EXTENSION_INFO |
        BundleFlag::GET_BUNDLE_WITH_HASH_VALUE, info, requestUserId);
    APP_LOGD("GetSandboxAppBundleInfo successfully");
    return ERR_OK;
}

int32_t BundleSandboxDataMgr::GenerateSandboxAppIndex(const std::string &bundleName)
{
    APP_LOGD("GenerateSandboxAppIndex begin");
    if (bundleName.empty()) {
        APP_LOGE("GenerateSandboxAppIndex bundleName is empty");
        return Constants::INITIAL_APP_INDEX;
    }
    std::unique_lock<std::mutex> lock(sandboxAppIndexMapMutex_);
    auto firstIterator = sandboxAppIndexMap_.find(bundleName);
    if (firstIterator == sandboxAppIndexMap_.end()) {
        std::set<int32_t> innerSet { Constants::INITIAL_APP_INDEX + 1 };
        sandboxAppIndexMap_.emplace(bundleName, innerSet);
        APP_LOGD("GenerateSandboxAppIndex successfully");
        return Constants::INITIAL_APP_INDEX + 1;
    }

    if (firstIterator->second.empty()) {
        firstIterator->second.insert(Constants::INITIAL_APP_INDEX + 1);
        APP_LOGD("GenerateSandboxAppIndex successfully");
        return Constants::INITIAL_APP_INDEX + 1;
    }

    int32_t pre = Constants::INITIAL_APP_INDEX;
    for (auto secondIterator = firstIterator->second.begin(); secondIterator != firstIterator->second.end();
        ++secondIterator) {
        if (*secondIterator == pre + 1) {
            pre++;
            continue;
        }
        break;
    }

    int32_t newAppIndex = pre + 1;
    if (newAppIndex > Constants::MAX_APP_INDEX) {
        APP_LOGE("GenerateSandboxAppIndex failed due to exceed limitation of maximum appIndex");
        return Constants::INITIAL_APP_INDEX;
    }
    firstIterator->second.insert(newAppIndex);
    APP_LOGD("GenerateSandboxAppIndex successfully with appIndex %{public}d", newAppIndex);
    return newAppIndex;
}

bool BundleSandboxDataMgr::DeleteSandboxAppIndex(const std::string &bundleName, int32_t appIndex)
{
    APP_LOGD("DeleteSandboxAppIndex begin");
    if (bundleName.empty()) {
        APP_LOGE("DeleteSandboxAppIndex bundleName is empty");
        return false;
    }
    std::unique_lock<std::mutex> lock(sandboxAppIndexMapMutex_);
    auto it = sandboxAppIndexMap_.find(bundleName);
    if (it == sandboxAppIndexMap_.end() || it->second.empty()) {
        APP_LOGE("no sandbox app can be found %{public}s", bundleName.c_str());
        return false;
    }

    auto ret = it->second.erase(appIndex);
    if (ret == 0) {
        APP_LOGE("no sandbox app index can be found %{public}d", appIndex);
        return false;
    }

    if (it->second.empty()) {
        sandboxAppIndexMap_.erase(bundleName);
    }
    APP_LOGD("DeleteSandboxAppIndex successfully");
    return true;
}

std::unordered_map<std::string, InnerBundleInfo> BundleSandboxDataMgr::GetSandboxAppInfoMap() const
{
    std::shared_lock<std::shared_mutex> lock(sandboxAppMutex_);
    return sandboxAppInfos_;
}
} // AppExecFwk
} // OHOS