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

#ifndef FOUNDATION_APPEXECFWK_SERVICES_BUNDLEMGR_INCLUDE_BUNDLE_MGR_SERVICE_EVENT_HANDLER_H
#define FOUNDATION_APPEXECFWK_SERVICES_BUNDLEMGR_INCLUDE_BUNDLE_MGR_SERVICE_EVENT_HANDLER_H

#include "bundle_constants.h"
#include "bundle_data_mgr.h"
#include "event_handler.h"

namespace OHOS {
namespace AppExecFwk {
class BundleMgrService;

class BMSEventHandler : public EventHandler {
public:
    explicit BMSEventHandler(const std::shared_ptr<EventRunner> &runner);
    virtual ~BMSEventHandler() override;
    /**
     * @brief Process the event of install system bundles.
     * @param event Indicates the event to be processed.
     * @return
     */
    virtual void ProcessEvent(const InnerEvent::Pointer &event) override;

    enum {
        BUNDLE_SCAN_START = 1,
        BUNDLE_SCAN_FINISHED,
        BMS_START_FINISHED,
        BUNDLE_REBOOT_SCAN_START,
    };

private:
    /**
     * @brief Install system and system vendor bundles.
     * @param scanDir Indicates the scanDir.
     * @param appType Indicates the bundle type.
     * @param userId Indicates userId.
     * @return
     */
    void ProcessSystemBundleInstall(const std::string &scanDir,
        Constants::AppType appType, int32_t userId = Constants::UNSPECIFIED_USERID);
    /**
     * @brief Set the flag indicates that all system and vendor applications installed.
     * @return
     */
    void SetAllInstallFlag() const;
    /**
     * @brief start scan.
     * @param userId Indicates the userId.
     * @return
     */
    void OnStartScanning(int32_t userId = Constants::UNSPECIFIED_USERID);
    /**
     * @brief start scan.
     * @param userId Indicates the userId.
     * @return
     */
    void RebootStartScanning(int32_t userId = Constants::UNSPECIFIED_USERID);
    /**
     * @brief Install system and system vendor bundles.
     * @param userId Indicates userId.
     * @return
     */
    void RebootProcessSystemBundle(int32_t userId = Constants::UNSPECIFIED_USERID);
    /**
     * @brief Get bundleinfo of HAP by path.
     * @param hapFilePath Indicates the absolute file path of the HAP.
     * @param infos Indicates the obtained BundleInfo object.
     * @return Returns true if the BundleInfo is successfully obtained; returns false otherwise.
     */
    bool ParseHapFiles(const std::string &hapFilePath,
        std::unordered_map<std::string, InnerBundleInfo> &infos);
    /**
     * @brief Reboot install system and system vendor bundles.
     * @param bundleList Indicates store bundle list.
     * @param appType Indicates the bundle type.
     * @return
     */
    void RebootBundleInstall(
        const std::list<std::string> &bundleList, Constants::AppType appType);
    /**
     * @brief Reboot uninstall system and system vendor bundles.
     * @return
     */
    void RebootBundleUninstall();
    /**
     * @brief To check the version code and bundleName in all haps.
     * @param infos .Indicates all innerBundleInfo for all haps need to be installed.
     * @return Returns ERR_OK if haps checking successfully; returns error code otherwise.
     */
    ErrCode CheckAppLabelInfo(const std::unordered_map<std::string, InnerBundleInfo> &infos);
    /**
     * @brief OTA Install system app and system vendor bundles.
     * @param filePath Indicates the filePath.
     * @param appType Indicates the bundle type.
     * @return Returns true if this function called successfully; returns false otherwise.
     */
    bool OTAInstallSystemBundle(const std::string &filePath, Constants::AppType appType);
    /**
     * @brief Used to determine whether the module has been installed. If the installation has
     *        been uninstalled, OTA install and upgrade will not be allowed.
     * @param bundleName Indicates the bundleName.
     * @param bundlePath Indicates the bundlePath.
     * @return Returns true if this function called successfully; returns false otherwise.
     */
    bool HasModuleSavedInPreInstalledDb(
        const std::string &bundleName, const std::string &bundlePath);
    /**
     * @brief Delete preInstallInfo to Db.
     * @param bundleName Indicates the bundleName.
     * @param bundlePath Indicates the bundlePath.
     */
    void DeletePreInfoInDb(
        const std::string &bundleName, const std::string &bundlePath, bool bundleLevel);
    /**
     * @brief Add parseInfos to map.
     * @param bundleName Indicates the bundleName.
     * @param infos Indicates the infos.
     */
    void AddParseInfosToMap(const std::string &bundleName,
        const std::unordered_map<std::string, InnerBundleInfo> &infos);
    bool LoadAllPreInstallBundleInfos();
    void RebootProcessBundleInstall(
        const std::string &scanDir, Constants::AppType appType);

    // Used to save the information parsed by Hap in the scanned directory.
    std::map<std::string, std::unordered_map<std::string, InnerBundleInfo>> hapParseInfoMap_;
    // used to save application information that already exists in the Db.
    std::map<std::string, PreInstallBundleInfo> loadExistData_;

    std::vector<std::string> scanPaths_;
};
}  // namespace AppExecFwk
}  // namespace OHOS
#endif  // FOUNDATION_APPEXECFWK_SERVICES_BUNDLEMGR_INCLUDE_BUNDLE_MGR_SERVICE_EVENT_HANDLER_H
