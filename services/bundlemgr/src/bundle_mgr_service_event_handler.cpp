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

#include "bundle_mgr_service_event_handler.h"

#include <future>

#include "app_log_wrapper.h"
#include "bundle_mgr_service.h"
#include "bundle_scanner.h"
#include "perf_profile.h"
#include "system_bundle_installer.h"

namespace OHOS {
namespace AppExecFwk {
BMSEventHandler::BMSEventHandler(const std::shared_ptr<EventRunner> &runner) : EventHandler(runner)
{
    APP_LOGD("instance is created");
}

BMSEventHandler::~BMSEventHandler()
{
    APP_LOGD("instance is destroyed");
}

void BMSEventHandler::ProcessEvent(const InnerEvent::Pointer &event)
{
    switch (event->GetInnerEventId()) {
        case BUNDLE_SCAN_START: {
            OnStartScanning(Constants::DEFAULT_USERID);
            SetAllInstallFlag();
            DelayedSingleton<BundleMgrService>::GetInstance()->RegisterService();
            break;
        }
        case BUNDLE_SCAN_FINISHED:
            break;
        case BMS_START_FINISHED:
            break;
        case BUNDLE_REBOOT_SCAN_START: {
            RebootStartScanning();
            SetAllInstallFlag();
            DelayedSingleton<BundleMgrService>::GetInstance()->RegisterService();
            break;
        }
        default:
            APP_LOGE("the eventId is not supported");
            break;
    }
}

void BMSEventHandler::OnStartScanning(int32_t userId)
{
    auto future = std::async(std::launch::async, [this, userId] {
        ProcessSystemBundleInstall(Constants::AppType::SYSTEM_APP, userId);
        ProcessSystemBundleInstall(Constants::AppType::THIRD_SYSTEM_APP, userId);
    });
    future.get();
}

void BMSEventHandler::ProcessSystemBundleInstall(Constants::AppType appType, int32_t userId) const
{
    APP_LOGD("scan thread start");
    auto scanner = std::make_unique<BundleScanner>();
    if (!scanner) {
        APP_LOGE("make scanner failed");
        return;
    }
    std::string scanDir = (appType == Constants::AppType::SYSTEM_APP) ? Constants::SYSTEM_APP_SCAN_PATH
                                                                      : Constants::THIRD_SYSTEM_APP_SCAN_PATH;
    APP_LOGD("scanDir: %{public}s and userId: %{public}d", scanDir.c_str(), userId);
    std::list<std::string> bundleList = scanner->Scan(scanDir);
    auto iter = std::find(bundleList.begin(), bundleList.end(), Constants::SYSTEM_RESOURCES_APP_PATH);
    if (iter != bundleList.end()) {
        bundleList.erase(iter);
        bundleList.insert(bundleList.begin(), Constants::SYSTEM_RESOURCES_APP_PATH);
    }
    for (const auto &item : bundleList) {
        SystemBundleInstaller installer(item);
        APP_LOGD("scan item %{public}s", item.c_str());
        if (!installer.InstallSystemBundle(appType, userId)) {
            APP_LOGW("Install System app:%{public}s error", item.c_str());
        }
    }
    PerfProfile::GetInstance().Dump();
}

void BMSEventHandler::SetAllInstallFlag() const
{
    auto dataMgr = DelayedSingleton<BundleMgrService>::GetInstance()->GetDataMgr();
    if (dataMgr == nullptr) {
        APP_LOGE("DataMgr is nullptr");
        return;
    }
    dataMgr->SetInitialUserFlag(true);
}

void BMSEventHandler::RebootStartScanning(int32_t userId)
{
    auto future = std::async(std::launch::async, [this, userId] {
        RebootProcessSystemBundle(userId);
    });
    future.get();
}

bool BMSEventHandler::GetScanBundleArchiveInfo(
    const std::string &hapFilePath, BundleInfo &bundleInfo)
{
    auto dataMgr = DelayedSingleton<BundleMgrService>::GetInstance()->GetDataMgr();
    std::string realPath;
    auto ret = BundleUtil::CheckFilePath(hapFilePath, realPath);
    if (ret != ERR_OK) {
        APP_LOGE("File path %{public}s invalid", hapFilePath.c_str());
        return false;
    }
    InnerBundleInfo info;
    BundleParser bundleParser;
    ret = bundleParser.Parse(realPath, info);
    if (ret != ERR_OK) {
        APP_LOGE("parse bundle info failed, error: %{public}d", ret);
        return false;
    }
    bundleInfo.name = info.GetBundleName();
    bundleInfo.versionCode = info.GetVersionCode();
    bundleInfo.hapModuleNames = info.GetModuleNameVec();
    return true;
}

void BMSEventHandler::RebootProcessSystemBundle(int32_t userId)
{
    APP_LOGD("reboot scan thread start");
    auto dataMgr = DelayedSingleton<BundleMgrService>::GetInstance()->GetDataMgr();
    if (dataMgr == nullptr) {
        APP_LOGE("DataMgr is nullptr");
        return;
    }
    auto scanner = std::make_unique<BundleScanner>();
    if (!scanner) {
        APP_LOGE("make scanner failed");
        return;
    }
    std::list<std::string> bundleList = scanner->Scan(Constants::SYSTEM_APP_SCAN_PATH);
    auto iter = std::find(bundleList.begin(), bundleList.end(), Constants::SYSTEM_RESOURCES_APP_PATH);
    if (iter != bundleList.end()) {
        bundleList.erase(iter);
        bundleList.insert(bundleList.begin(), Constants::SYSTEM_RESOURCES_APP_PATH);
    }

    std::list<std::string> vendorBundleList = scanner->Scan(Constants::THIRD_SYSTEM_APP_SCAN_PATH);

    BundleInfo bundleInfo;
    std::vector<PreInstallBundleInfo> preInstallBundleInfos;
    if (!dataMgr->LoadAllPreInstallBundleInfos(preInstallBundleInfos)) {
        APP_LOGE("LoadAllPreInstallBundleInfos failed.");
        return;
    }
    for (auto &iter : preInstallBundleInfos) {
        bundleInfo.name = iter.GetBundleName();
        bundleInfo.versionCode = iter.GetVersionCode();
        loadExistData_.emplace(iter.GetBundleName(), bundleInfo);
    }
    RebootBundleInstall(bundleList, Constants::AppType::SYSTEM_APP);
    RebootBundleInstall(vendorBundleList, Constants::AppType::THIRD_SYSTEM_APP);
    RebootBundleUninstall();
}

void BMSEventHandler::RebootBundleInstall(
    const std::list<std::string> &bundleList, Constants::AppType appType)
{
    auto dataMgr = DelayedSingleton<BundleMgrService>::GetInstance()->GetDataMgr();
    BundleInfo bundleInfo;
    for (auto &listIter : bundleList) {
        bool exist = false;
        APP_LOGD("reboot scan bundle path: %{public}s ", listIter.c_str());
        auto result = GetScanBundleArchiveInfo(listIter, bundleInfo);
        if (!result) {
            APP_LOGW("obtain bundleinfo failed : %{public}s ", listIter.c_str());
            continue;
        }
        bundleInfoMap_.emplace(bundleInfo.name, listIter);
        auto mapIter = loadExistData_.find(bundleInfo.name);

        APP_LOGD("reboot scan bundleName: %{public}s ", bundleInfo.name.c_str());
        if (mapIter != loadExistData_.end() && result) {
            APP_LOGD("reboot scan %{public}s is exist", bundleInfo.name.c_str());
            exist = true;

            BundleInfo Info;
            result = dataMgr->GetBundleInfo(bundleInfo.name, BundleFlag::GET_BUNDLE_DEFAULT, Info,
                Constants::ANY_USERID);
            if (!result) {
                APP_LOGW("obtain bundleinfo failed with moduleName: %{public}s ", bundleInfo.name.c_str());
                continue;
            }
            if (mapIter->second.versionCode < bundleInfo.versionCode) {
                SystemBundleInstaller installer(listIter);
                APP_LOGD("bundle update due to high version bundle %{public}s existed", listIter.c_str());
                if (!installer.OTAInstallSystemBundle(appType)) {
                    APP_LOGW("system app :%{public}s update failed", listIter.c_str());
                }
            }
            if (mapIter->second.versionCode == bundleInfo.versionCode) {
                APP_LOGD("reboot scan versioncode is same");
                if (find(Info.hapModuleNames.begin(), Info.hapModuleNames.end(), bundleInfo.hapModuleNames[0]) ==
                    Info.hapModuleNames.end()) {
                    SystemBundleInstaller installer(listIter);
                    APP_LOGD("bundle update due to another same version module %{public}s existed", listIter.c_str());
                    if (!installer.OTAInstallSystemBundle(appType)) {
                        APP_LOGW("system app :%{public}s update failed", listIter.c_str());
                    }
                }
            }
        }
        if (!exist) {
            SystemBundleInstaller installer(listIter);
            APP_LOGD("reboot scan %{public}s is not exist", listIter.c_str());
            if (!installer.OTAInstallSystemBundle(appType)) {
                APP_LOGW("reboot Install System app:%{public}s error", listIter.c_str());
            }
        }
    }
}

void BMSEventHandler::RebootBundleUninstall()
{
    APP_LOGD("RebootBundleUninstall start");
    for (auto &loadIter : loadExistData_) {
        std::string bundleName = loadIter.first;
        auto listIter = bundleInfoMap_.find(bundleName);
        if (listIter == bundleInfoMap_.end()) {
            APP_LOGD("reboot scan uninstall bundleInfo.name: %{public}s", loadIter.first.c_str());
            std::string list;
            SystemBundleInstaller installer(list);
            if (!installer.UninstallSystemBundle(bundleName)) {
                APP_LOGW("reboot Uninstall System app:%{public}s error", bundleName.c_str());
            }
            APP_LOGD("reboot scan uninstall success");
        }
    }
}
}  // namespace AppExecFwk
}  // namespace OHOS
