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
#ifdef CONFIG_POLOCY_ENABLE
#include "config_policy_utils.h"
#endif
#include "perf_profile.h"
#include "system_bundle_installer.h"

namespace OHOS {
namespace AppExecFwk {
namespace {
const std::string APP_SUFFIX = "/app";
}
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
    scanPaths_.clear();
    ProcessSystemBundleInstall(
        Constants::SYSTEM_APP_SCAN_PATH, Constants::AppType::SYSTEM_APP, userId);
    ProcessSystemBundleInstall(
        Constants::THIRD_SYSTEM_APP_SCAN_PATH, Constants::AppType::THIRD_SYSTEM_APP, userId);
#ifdef CONFIG_POLOCY_ENABLE
    auto cfgDirList = GetCfgDirList();
    if (cfgDirList == nullptr) {
        APP_LOGD("cfgDirList is empty");
        return;
    }

    for (auto cfgDir : cfgDirList->paths) {
        if (!cfgDir) {
            continue;
        }

        APP_LOGD("cfgDir: %{public}s ", cfgDir);
        ProcessSystemBundleInstall(
            cfgDir + APP_SUFFIX, Constants::AppType::SYSTEM_APP, userId);
    }
#endif
}

void BMSEventHandler::ProcessSystemBundleInstall(
    const std::string &scanDir, Constants::AppType appType, int32_t userId)
{
    APP_LOGD("scan thread start");
    if (std::find(scanPaths_.begin(), scanPaths_.end(), scanDir) != scanPaths_.end()) {
        APP_LOGD("scanDir(%{public}s) has scan", scanDir.c_str());
        return;
    }

    APP_LOGD("scanDir: %{public}s and userId: %{public}d", scanDir.c_str(), userId);
    scanPaths_.emplace_back(scanDir);
    auto scanner = std::make_unique<BundleScanner>();
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
    scanPaths_.clear();
    auto future = std::async(std::launch::async, [this, userId] {
        RebootProcessSystemBundle(userId);
    });
    future.get();
}

void BMSEventHandler::RebootProcessSystemBundle(int32_t userId)
{
    APP_LOGD("reboot scan thread start");
    if (!LoadAllPreInstallBundleInfos()) {
        APP_LOGE("Load all preInstall bundleInfos failed.");
        return;
    }

    RebootProcessBundleInstall(
        Constants::SYSTEM_APP_SCAN_PATH, Constants::AppType::SYSTEM_APP);
    RebootProcessBundleInstall(
        Constants::THIRD_SYSTEM_APP_SCAN_PATH, Constants::AppType::THIRD_SYSTEM_APP);
#ifdef CONFIG_POLOCY_ENABLE
    auto cfgDirList = GetCfgDirList();
    if (cfgDirList == nullptr) {
        APP_LOGD("cfgDirList is empty");
        return;
    }

    for (auto cfgDir : cfgDirList->paths) {
        if (!cfgDir) {
            continue;
        }

        APP_LOGD("cfgDir: %{public}s ", cfgDir);
        RebootProcessBundleInstall(cfgDir + APP_SUFFIX, Constants::AppType::SYSTEM_APP);
    }
#endif

    RebootBundleUninstall();
}

bool BMSEventHandler::LoadAllPreInstallBundleInfos()
{
    auto dataMgr = DelayedSingleton<BundleMgrService>::GetInstance()->GetDataMgr();
    if (dataMgr == nullptr) {
        APP_LOGE("DataMgr is nullptr");
        return false;
    }

    std::vector<PreInstallBundleInfo> preInstallBundleInfos = dataMgr->GetAllPreInstallBundleInfos();
    for (auto &iter : preInstallBundleInfos) {
        APP_LOGD("preInstallBundleInfos: %{public}s ", iter.GetBundleName().c_str());
        loadExistData_.emplace(iter.GetBundleName(), iter);
    }

    return true;
}

void BMSEventHandler::RebootProcessBundleInstall(
    const std::string &scanDir, Constants::AppType appType)
{
    if (std::find(scanPaths_.begin(), scanPaths_.end(), scanDir) != scanPaths_.end()) {
        APP_LOGD("Reboot scanDir(%{public}s) has scan", scanDir.c_str());
        return;
    }

    APP_LOGD("Reboot scanDir: %{public}s", scanDir.c_str());
    scanPaths_.emplace_back(scanDir);
    auto scanner = std::make_unique<BundleScanner>();
    std::list<std::string> bundleList = scanner->Scan(scanDir);
    auto iter = std::find(
        bundleList.begin(), bundleList.end(), Constants::SYSTEM_RESOURCES_APP_PATH);
    if (iter != bundleList.end()) {
        bundleList.erase(iter);
        bundleList.insert(bundleList.begin(), Constants::SYSTEM_RESOURCES_APP_PATH);
    }

    RebootBundleInstall(bundleList, appType);
}

void BMSEventHandler::RebootBundleInstall(
    const std::list<std::string> &scanPathList, Constants::AppType appType)
{
    auto dataMgr = DelayedSingleton<BundleMgrService>::GetInstance()->GetDataMgr();
    if (dataMgr == nullptr) {
        APP_LOGE("DataMgr is nullptr");
        return;
    }

    for (auto &scanPathIter : scanPathList) {
        APP_LOGD("reboot scan bundle path: %{private}s ", scanPathIter.c_str());
        std::unordered_map<std::string, InnerBundleInfo> infos;
        if (!ParseHapFiles(scanPathIter, infos) || infos.empty()) {
            APP_LOGE("obtain bundleinfo failed : %{private}s ", scanPathIter.c_str());
            continue;
        }

        auto bundleName = infos.begin()->second.GetBundleName();
        auto hapVersionCode = infos.begin()->second.GetVersionCode();
        AddParseInfosToMap(bundleName, infos);
        auto mapIter = loadExistData_.find(bundleName);
        if (mapIter == loadExistData_.end()) {
            APP_LOGD("OTA Install new bundle(%{public}s) by path(%{private}s).",
                bundleName.c_str(), scanPathIter.c_str());
            if (!OTAInstallSystemBundle(scanPathIter, appType)) {
                APP_LOGE("OTA Install new bundle(%{public}s) error.", bundleName.c_str());
            }

            continue;
        }

        APP_LOGD("OTA process bundle(%{public}s) by path(%{private}s).",
            bundleName.c_str(), scanPathIter.c_str());
        BundleInfo hasInstalledInfo;
        auto hasBundleInstalled = dataMgr->GetBundleInfo(
            bundleName, BundleFlag::GET_BUNDLE_DEFAULT, hasInstalledInfo, Constants::ANY_USERID);
        if (!hasBundleInstalled) {
            APP_LOGW("app(%{public}s) has been uninstalled and do not OTA install.",
                bundleName.c_str());
            continue;
        }

        for (auto item : infos) {
            auto parserModuleNames = item.second.GetModuleNameVec();
            if (parserModuleNames.empty()) {
                APP_LOGE("module is empty when parser path(%{public}s).", item.first.c_str());
                continue;
            }

            // Used to judge whether the module has been installed.
            bool hasModuleInstalled = std::find(
                hasInstalledInfo.hapModuleNames.begin(), hasInstalledInfo.hapModuleNames.end(),
                parserModuleNames[0]) != hasInstalledInfo.hapModuleNames.end();
            if (HasModuleSavedInPreInstalledDb(bundleName, item.first) && !hasModuleInstalled) {
                APP_LOGW("module(%{public}s) has been uninstalled and do not OTA install",
                    parserModuleNames[0].c_str());
                continue;
            }

            // Generally, when the versionCode of Hap is greater than the installed versionCode,
            // Except for the uninstalled app, they can be installed or upgraded directly by OTA.
            if (hasInstalledInfo.versionCode < hapVersionCode) {
                APP_LOGD("OTA update module(%{public}s) by path(%{private}s)",
                    parserModuleNames[0].c_str(), item.first.c_str());
                if (!OTAInstallSystemBundle(item.first, appType)) {
                    APP_LOGE("OTA update module(%{public}s) failed", parserModuleNames[0].c_str());
                }
            }

            // The versionCode of Hap is equal to the installed versionCode.
            // You can only install new modules by OTA
            if (hasInstalledInfo.versionCode == hapVersionCode) {
                if (hasModuleInstalled) {
                    APP_LOGD("module(%{public}s) has been installed and versionCode is same.",
                        parserModuleNames[0].c_str());
                    continue;
                }

                APP_LOGD("OTA install module(%{public}s) by path(%{private}s)",
                    parserModuleNames[0].c_str(), item.first.c_str());
                if (!OTAInstallSystemBundle(item.first, appType)) {
                    APP_LOGE("OTA install module(%{public}s) failed", parserModuleNames[0].c_str());
                }
            }
        }
    }
}

void BMSEventHandler::AddParseInfosToMap(
    const std::string &bundleName, const std::unordered_map<std::string, InnerBundleInfo> &infos)
{
    auto hapParseInfoMapIter = hapParseInfoMap_.find(bundleName);
    if (hapParseInfoMapIter == hapParseInfoMap_.end()) {
        hapParseInfoMap_.emplace(bundleName, infos);
        return;
    }

    auto iterMap = hapParseInfoMapIter->second;
    for (auto infoIter : infos) {
        iterMap.emplace(infoIter.first, infoIter.second);
    }

    hapParseInfoMap_.at(bundleName) = iterMap;
}

void BMSEventHandler::RebootBundleUninstall()
{
    APP_LOGD("Reboot scan and OTA uninstall start");
    auto dataMgr = DelayedSingleton<BundleMgrService>::GetInstance()->GetDataMgr();
    if (dataMgr == nullptr) {
        APP_LOGE("DataMgr is nullptr");
        return;
    }

    for (auto &loadIter : loadExistData_) {
        std::string bundleName = loadIter.first;
        auto listIter = hapParseInfoMap_.find(bundleName);
        if (listIter == hapParseInfoMap_.end()) {
            APP_LOGD("OTA uninstall app(%{public}s).", bundleName.c_str());
            std::string list;
            SystemBundleInstaller installer(list);
            if (!installer.UninstallSystemBundle(bundleName)) {
                APP_LOGE("OTA uninstall app(%{public}s) error", bundleName.c_str());
            } else {
                std::string moduleName;
                DeletePreInfoInDb(bundleName, moduleName, true);
            }

            continue;
        }

        BundleInfo hasInstalledInfo;
        auto hasBundleInstalled = dataMgr->GetBundleInfo(
            bundleName, BundleFlag::GET_BUNDLE_DEFAULT, hasInstalledInfo, Constants::ANY_USERID);
        if (!hasBundleInstalled) {
            APP_LOGW("app(%{public}s) maybe has been uninstall.", bundleName.c_str());
            continue;
        }

        // Check the installed module.
        // If the corresponding Hap does not exist, it should be uninstalled.
        for (auto moduleName : hasInstalledInfo.hapModuleNames) {
            bool hasModuleHapExist = false;
            for (auto parserInfoIter : listIter->second) {
                auto parserModuleNames = parserInfoIter.second.GetModuleNameVec();
                if (!parserModuleNames.empty() && moduleName == parserModuleNames[0]) {
                    hasModuleHapExist = true;
                    break;
                }
            }

            if (!hasModuleHapExist) {
                APP_LOGD("OTA app(%{public}s) uninstall module(%{public}s).",
                    bundleName.c_str(), moduleName.c_str());
                std::string list;
                SystemBundleInstaller installer(list);
                if (!installer.UninstallSystemBundle(bundleName, moduleName)) {
                    APP_LOGE("OTA app(%{public}s) uninstall module(%{public}s) error.",
                        bundleName.c_str(), moduleName.c_str());
                }
            }
        }

        // Check the preInstall path in Db.
        // If the corresponding Hap does not exist, it should be deleted.
        auto parserInfoMap = listIter->second;
        for (auto preBundlePath : loadIter.second.GetBundlePaths()) {
            auto parserInfoIter = parserInfoMap.find(preBundlePath);
            if (parserInfoIter != parserInfoMap.end()) {
                APP_LOGD("OTA uninstall app(%{public}s) module path(%{private}s) exits.",
                    bundleName.c_str(), preBundlePath.c_str());
                continue;
            }

            APP_LOGD("OTA app(%{public}s) delete path(%{private}s).",
                bundleName.c_str(), preBundlePath.c_str());
            DeletePreInfoInDb(bundleName, preBundlePath, false);
        }
    }

    APP_LOGD("Reboot scan and OTA uninstall success");
}

void BMSEventHandler::DeletePreInfoInDb(
    const std::string &bundleName, const std::string &bundlePath, bool bundleLevel)
{
    APP_LOGD("DeletePreInfoInDb bundle(%{public}s)", bundleName.c_str());
    auto dataMgr = DelayedSingleton<BundleMgrService>::GetInstance()->GetDataMgr();
    if (dataMgr == nullptr) {
        APP_LOGE("DataMgr is nullptr");
        return;
    }

    PreInstallBundleInfo preInstallBundleInfo;
    preInstallBundleInfo.SetBundleName(bundleName);
    if (bundleLevel) {
        APP_LOGD("DeletePreInfoInDb bundle bundleLevel");
        dataMgr->DeletePreInstallBundleInfo(bundleName, preInstallBundleInfo);
        return;
    }

    APP_LOGD("DeletePreInfoInDb bundle not bundleLevel with path(%{private}s)",
        bundlePath.c_str());
    dataMgr->GetPreInstallBundleInfo(bundleName, preInstallBundleInfo);
    preInstallBundleInfo.DeleteBundlePath(bundlePath);
    if (preInstallBundleInfo.GetBundlePaths().empty()) {
        dataMgr->DeletePreInstallBundleInfo(bundleName, preInstallBundleInfo);
    } else {
        dataMgr->SavePreInstallBundleInfo(bundleName, preInstallBundleInfo);
    }
}

bool BMSEventHandler::HasModuleSavedInPreInstalledDb(
    const std::string &bundleName, const std::string &bundlePath)
{
    auto preInstallIter = loadExistData_.find(bundleName);
    if (preInstallIter == loadExistData_.end()) {
        APP_LOGE("app(%{public}s) does not save in PreInstalledDb.", bundleName.c_str());
        return false;
    }

    return preInstallIter->second.HasBundlePath(bundlePath);
}

bool BMSEventHandler::OTAInstallSystemBundle(
    const std::string &filePath, Constants::AppType appType)
{
    APP_LOGD("OTA install start in bunlde: %{private}s", filePath.c_str());
    if (filePath.empty()) {
        return false;
    }

    SystemBundleInstaller installer(filePath);
    return installer.OTAInstallSystemBundle(appType);
}

bool BMSEventHandler::ParseHapFiles(
    const std::string &hapFilePath, std::unordered_map<std::string, InnerBundleInfo> &infos)
{
    auto dataMgr = DelayedSingleton<BundleMgrService>::GetInstance()->GetDataMgr();
    std::vector<std::string> hapFilePathVec { hapFilePath };
    std::vector<std::string> realPaths;
    auto ret = BundleUtil::CheckFilePath(hapFilePathVec, realPaths);
    if (ret != ERR_OK) {
        APP_LOGE("File path %{private}s invalid", hapFilePath.c_str());
        return false;
    }

    BundleParser bundleParser;
    for (auto realPath : realPaths) {
        InnerBundleInfo innerBundleInfo;
        ret = bundleParser.Parse(realPath, innerBundleInfo);
        if (ret != ERR_OK) {
            APP_LOGE("parse bundle info failed, error: %{public}d", ret);
            continue;
        }

        infos.emplace(realPath, innerBundleInfo);
    }

    ret = CheckAppLabelInfo(infos);
    if (ret != ERR_OK) {
        APP_LOGE("Check APP label failed %{public}d", ret);
        return false;
    }

    return true;
}

ErrCode BMSEventHandler::CheckAppLabelInfo(
    const std::unordered_map<std::string, InnerBundleInfo> &infos)
{
    APP_LOGD("Check APP label");
    ErrCode ret = ERR_OK;
    if (infos.empty()) {
        return ret;
    }

    std::string bundleName = (infos.begin()->second).GetBundleName();
    std::string vendor = (infos.begin()->second).GetVendor();
    auto versionCode = (infos.begin()->second).GetVersionCode();
    std::string versionName = (infos.begin()->second).GetVersionName();
    uint32_t target = (infos.begin()->second).GetTargetVersion();
    uint32_t compatible = (infos.begin()->second).GetCompatibleVersion();
    bool singleton = (infos.begin()->second).IsSingleton();
    Constants::AppType appType = (infos.begin()->second).GetAppType();

    for (const auto &info :infos) {
        // check bundleName
        if (bundleName != info.second.GetBundleName()) {
            return ERR_APPEXECFWK_INSTALL_BUNDLENAME_NOT_SAME;
        }
        // check version
        if (versionCode != info.second.GetVersionCode()) {
            return ERR_APPEXECFWK_INSTALL_VERSIONCODE_NOT_SAME;
        }
        if (versionName != info.second.GetVersionName()) {
            return ERR_APPEXECFWK_INSTALL_VERSIONNAME_NOT_SAME;
        }
        // check vendor
        if (vendor != info.second.GetVendor()) {
            return ERR_APPEXECFWK_INSTALL_VENDOR_NOT_SAME;
        }
        // check release type
        if (target != info.second.GetTargetVersion()) {
            return ERR_APPEXECFWK_INSTALL_RELEASETYPE_TARGET_NOT_SAME;
        }
        if (compatible != info.second.GetCompatibleVersion()) {
            return ERR_APPEXECFWK_INSTALL_RELEASETYPE_COMPATIBLE_NOT_SAME;
        }
        if (singleton != info.second.IsSingleton()) {
            return ERR_APPEXECFWK_INSTALL_SINGLETON_NOT_SAME;
        }
        if (appType != info.second.GetAppType()) {
            return ERR_APPEXECFWK_INSTALL_APPTYPE_NOT_SAME;
        }
    }
    APP_LOGD("finish check APP label");
    return ret;
}
}  // namespace AppExecFwk
}  // namespace OHOS
