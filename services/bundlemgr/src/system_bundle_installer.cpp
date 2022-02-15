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

#include "system_bundle_installer.h"

#include "app_log_wrapper.h"
#include "bundle_mgr_service.h"

namespace OHOS {
namespace AppExecFwk {
SystemBundleInstaller::SystemBundleInstaller(const std::string &filePath) : filePath_(filePath)
{
    APP_LOGI("system bundle installer instance is created");
}

SystemBundleInstaller::~SystemBundleInstaller()
{
    APP_LOGI("system bundle installer instance is destroyed");
}

bool SystemBundleInstaller::InstallSystemBundle(Constants::AppType appType, int32_t userId)
{
    InstallParam installParam;
    installParam.userId = userId;
    installParam.isPreInstallApp = true;
    installParam.noSkipsKill = false;
    if (appType == Constants::AppType::SYSTEM_APP
        || appType == Constants::AppType::THIRD_SYSTEM_APP) {
        installParam.needSavePreInstallInfo = true;
    }
    ErrCode result = InstallBundle(filePath_, installParam, appType);
    if (result != ERR_OK) {
        APP_LOGE("install system bundle fail, error: %{public}d", result);
        return false;
    }
    return true;
}

bool SystemBundleInstaller::OTAInstallSystemBundle(Constants::AppType appType)
{
    auto dataMgr = DelayedSingleton<BundleMgrService>::GetInstance()->GetDataMgr();
    if (!dataMgr) {
        APP_LOGE("Get dataMgr shared_ptr nullptr");
        return false;
    }
    InstallParam installParam;
    installParam.isPreInstallApp = true;
    installParam.noSkipsKill = false;
    if (appType == Constants::AppType::SYSTEM_APP
    || appType == Constants::AppType::THIRD_SYSTEM_APP) {
        installParam.needSavePreInstallInfo = true;
    }
    for (auto allUserId : dataMgr->GetAllUser()) {
        installParam.userId = allUserId;
        ErrCode result = InstallBundle(filePath_, installParam, appType);
        if (result != ERR_OK) {
            APP_LOGE("install system bundle fail, error: %{public}d", result);
        }
        ResetInstallProperties();
    }
    return true;
}

bool SystemBundleInstaller::UninstallSystemBundle(const std::string &bundleName)
{
    auto dataMgr = DelayedSingleton<BundleMgrService>::GetInstance()->GetDataMgr();
    if (!dataMgr) {
        APP_LOGE("Get dataMgr shared_ptr nullptr");
        return false;
    }
    InstallParam installParam;
    for (auto userId : dataMgr->GetAllUser()) {
        installParam.userId = userId;
        installParam.needSavePreInstallInfo = true;
        installParam.noSkipsKill = false;
        ErrCode result = UninstallBundle(bundleName, installParam);
        if (result != ERR_OK) {
            APP_LOGE("uninstall system bundle fail, error: %{public}d", result);
            return false;
        }
        ResetInstallProperties();
    }
    return true;
}
}  // namespace AppExecFwk
}  // namespace OHOS
