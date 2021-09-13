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

#include "bundle_data_mgr.h"

#include <cinttypes>
#include <chrono>

#include "nlohmann/json.hpp"
#include "app_log_wrapper.h"
#include "permission/permission_kit.h"
#include "bundle_constants.h"
#include "bundle_data_storage_database.h"
#include "json_serializer.h"
#include "common_event_manager.h"
#include "common_event_support.h"
#include "bundle_status_callback_death_recipient.h"
#include "permission_changed_death_recipient.h"

namespace OHOS {
namespace AppExecFwk {

BundleDataMgr::BundleDataMgr()
{
    InitStateTransferMap();
    dataStorage_ = std::make_shared<BundleDataStorageDatabase>();
    usageRecordStorage_ = std::make_shared<ModuleUsageRecordStorage>();
    // register distributed data process death listener.
    usageRecordStorage_->RegisterKvStoreDeathListener();
    APP_LOGI("BundleDataMgr instance is created");
}

BundleDataMgr::~BundleDataMgr()
{
    APP_LOGI("BundleDataMgr instance is destroyed");
    installStates_.clear();
    transferStates_.clear();
    bundleInfos_.clear();
}

bool BundleDataMgr::LoadDataFromPersistentStorage()
{
    std::lock_guard<std::mutex> lock(bundleInfoMutex_);
    bool ret = dataStorage_->LoadAllData(bundleInfos_);
    if (ret) {
        if (bundleInfos_.empty()) {
            APP_LOGW("persistent data is empty");
            return false;
        }
        for (const auto &item : bundleInfos_) {
            std::lock_guard<std::mutex> lock(stateMutex_);
            installStates_.emplace(item.first, InstallState::INSTALL_SUCCESS);
        }
        RestoreUidAndGid();
        allInstallFlag_ = true;
    }
    return ret;
}

bool BundleDataMgr::UpdateBundleInstallState(const std::string &bundleName, const InstallState state)
{
    if (bundleName.empty()) {
        APP_LOGW("update result:fail, reason:bundle name is empty");
        return false;
    }

    std::lock_guard<std::mutex> lock(stateMutex_);
    auto item = installStates_.find(bundleName);
    if (item == installStates_.end()) {
        if (state == InstallState::INSTALL_START) {
            installStates_.emplace(bundleName, state);
            APP_LOGD("update result:success, state:INSTALL_START");
            return true;
        }
        APP_LOGW("update result:fail, reason:incorrect state");
        return false;
    }

    auto stateRange = transferStates_.equal_range(state);
    for (auto previousState = stateRange.first; previousState != stateRange.second; ++previousState) {
        if (item->second == previousState->second) {
            APP_LOGI("update result:success, current:%{public}d, state:%{public}d", previousState->second, state);
            if (IsDeleteDataState(state)) {
                installStates_.erase(item);
                DeleteBundleInfo(bundleName, state);
                return true;
            }
            item->second = state;
            return true;
        }
    }
    APP_LOGW("update result:fail, reason:incorrect current:%{public}d, state:%{public}d", item->second, state);
    return false;
}

bool BundleDataMgr::AddInnerBundleInfo(const std::string &bundleName, InnerBundleInfo &info)
{
    APP_LOGI("to save info:%{public}s", info.GetBundleName().c_str());
    if (bundleName.empty()) {
        APP_LOGW("save info fail, empty bundle name");
        return false;
    }

    std::lock_guard<std::mutex> lock(bundleInfoMutex_);
    auto infoItem = bundleInfos_.find(bundleName);
    if (infoItem != bundleInfos_.end()) {
        APP_LOGE("bundle info already exist");
        return false;
    }
    std::lock_guard<std::mutex> stateLock(stateMutex_);
    auto statusItem = installStates_.find(bundleName);
    if (statusItem == installStates_.end()) {
        APP_LOGE("save info fail, app:%{public}s is not installed", bundleName.c_str());
        return false;
    }
    if (statusItem->second == InstallState::INSTALL_SUCCESS) {
        APP_LOGI("save bundle:%{public}s info", bundleName.c_str());
        int64_t time =
            std::chrono::duration_cast<std::chrono::seconds>(std::chrono::system_clock::now().time_since_epoch())
                .count();
        APP_LOGI("the bundle install time is %{public}" PRId64, time);
        info.SetBundleInstallTime(time);
        if (dataStorage_->SaveStorageBundleInfo(Constants::CURRENT_DEVICE_ID, info)) {
            APP_LOGI("write storage success bundle:%{public}s", bundleName.c_str());
            std::map<std::string, InnerBundleInfo> infoWithId;
            infoWithId.emplace(Constants::CURRENT_DEVICE_ID, info);
            bundleInfos_.emplace(bundleName, infoWithId);
            return true;
        }
    }
    return false;
}

bool BundleDataMgr::AddNewModuleInfo(
    const std::string &bundleName, const InnerBundleInfo &newInfo, InnerBundleInfo &oldInfo)
{
    APP_LOGI("add new module info:%{public}s", bundleName.c_str());
    std::lock_guard<std::mutex> lock(bundleInfoMutex_);
    auto infoItem = bundleInfos_.find(bundleName);
    if (infoItem == bundleInfos_.end()) {
        APP_LOGE("bundle info not exist");
        return false;
    }
    std::lock_guard<std::mutex> stateLock(stateMutex_);
    auto statusItem = installStates_.find(bundleName);
    if (statusItem == installStates_.end()) {
        APP_LOGE("save info fail, app:%{public}s is not updated", bundleName.c_str());
        return false;
    }
    if (statusItem->second == InstallState::UPDATING_SUCCESS) {
        APP_LOGI("save bundle:%{public}s info", bundleName.c_str());
        int64_t time =
            std::chrono::duration_cast<std::chrono::seconds>(std::chrono::system_clock::now().time_since_epoch())
                .count();
        APP_LOGI("the bundle update time is %{public}" PRId64, time);
        oldInfo.SetBundleUpdateTime(time);
        oldInfo.UpdateVersionInfo(newInfo);
        oldInfo.AddModuleInfo(newInfo);
        oldInfo.SetBundleStatus(InnerBundleInfo::BundleStatus::ENABLED);
        if (dataStorage_->DeleteStorageBundleInfo(Constants::CURRENT_DEVICE_ID, oldInfo)) {
            if (dataStorage_->SaveStorageBundleInfo(Constants::CURRENT_DEVICE_ID, oldInfo)) {
                APP_LOGI("update storage success bundle:%{public}s", bundleName.c_str());
                bundleInfos_.at(bundleName).at(Constants::CURRENT_DEVICE_ID) = oldInfo;
                return true;
            }
        }
    }
    return false;
}

bool BundleDataMgr::RemoveModuleInfo(
    const std::string &bundleName, const std::string &modulePackage, InnerBundleInfo &oldInfo)
{
    APP_LOGI("remove module info:%{public}s/%{public}s", bundleName.c_str(), modulePackage.c_str());
    std::lock_guard<std::mutex> lock(bundleInfoMutex_);
    auto infoItem = bundleInfos_.find(bundleName);
    if (infoItem == bundleInfos_.end()) {
        APP_LOGE("bundle info not exist");
        return false;
    }
    std::lock_guard<std::mutex> stateLock(stateMutex_);
    auto statusItem = installStates_.find(bundleName);
    if (statusItem == installStates_.end()) {
        APP_LOGE("save info fail, app:%{public}s is not updated", bundleName.c_str());
        return false;
    }
    if (statusItem->second == InstallState::UNINSTALL_START) {
        APP_LOGI("save bundle:%{public}s info", bundleName.c_str());
        oldInfo.RemoveModuleInfo(modulePackage);
        oldInfo.SetBundleStatus(InnerBundleInfo::BundleStatus::ENABLED);
        if (dataStorage_->DeleteStorageBundleInfo(Constants::CURRENT_DEVICE_ID, oldInfo)) {
            if (dataStorage_->SaveStorageBundleInfo(Constants::CURRENT_DEVICE_ID, oldInfo)) {
                APP_LOGI("update storage success bundle:%{public}s", bundleName.c_str());
                bundleInfos_.at(bundleName).at(Constants::CURRENT_DEVICE_ID) = oldInfo;
                return true;
            }
        }
        APP_LOGI("after delete modulePackage:%{public}s info", modulePackage.c_str());
    }
    return true;
}

bool BundleDataMgr::UpdateInnerBundleInfo(
    const std::string &bundleName, const InnerBundleInfo &newInfo, InnerBundleInfo &oldInfo)
{
    APP_LOGI("update module info:%{public}s", bundleName.c_str());
    std::lock_guard<std::mutex> lock(bundleInfoMutex_);
    auto infoItem = bundleInfos_.find(bundleName);
    if (infoItem == bundleInfos_.end()) {
        APP_LOGE("bundle info not exist");
        return false;
    }
    std::lock_guard<std::mutex> stateLock(stateMutex_);
    auto statusItem = installStates_.find(bundleName);
    if (statusItem == installStates_.end()) {
        APP_LOGE("save info fail, app:%{public}s is not updated", bundleName.c_str());
        return false;
    }
    if (statusItem->second == InstallState::UPDATING_SUCCESS) {
        APP_LOGI("save bundle:%{public}s info", bundleName.c_str());
        int64_t time =
            std::chrono::duration_cast<std::chrono::seconds>(std::chrono::system_clock::now().time_since_epoch())
                .count();
        APP_LOGI("the bundle update time is %{public}" PRId64, time);
        oldInfo.SetBundleUpdateTime(time);
        oldInfo.UpdateVersionInfo(newInfo);
        oldInfo.UpdateModuleInfo(newInfo);
        oldInfo.SetBundleStatus(InnerBundleInfo::BundleStatus::ENABLED);
        if (dataStorage_->DeleteStorageBundleInfo(Constants::CURRENT_DEVICE_ID, oldInfo)) {
            if (dataStorage_->SaveStorageBundleInfo(Constants::CURRENT_DEVICE_ID, oldInfo)) {
                APP_LOGI("update storage success bundle:%{public}s", bundleName.c_str());
                bundleInfos_.at(bundleName).at(Constants::CURRENT_DEVICE_ID) = oldInfo;
                return true;
            }
        }
    }
    return false;
}

bool BundleDataMgr::QueryAbilityInfo(const Want &want, AbilityInfo &abilityInfo) const
{
    // for launcher
    if (want.HasEntity(Want::FLAG_HOME_INTENT_FROM_SYSTEM)) {
        std::lock_guard<std::mutex> lock(bundleInfoMutex_);
        for (const auto &item : bundleInfos_) {
            for (const auto &info : item.second) {
                if (allInstallFlag_ && info.second.GetIsLauncherApp()) {
                    APP_LOGI("find launcher app %{public}s", info.second.GetBundleName().c_str());
                    info.second.GetMainAbilityInfo(abilityInfo);
                    info.second.GetApplicationInfo(
                        ApplicationFlag::GET_APPLICATION_INFO_WITH_PERMS, 0, abilityInfo.applicationInfo);
                    return true;
                }
            }
        }
        return false;
    }

    ElementName element = want.GetElement();
    std::string abilityName = element.GetAbilityName();
    std::string bundleName = element.GetBundleName();
    APP_LOGI("bundle name:%{public}s, ability name:%{public}s", bundleName.c_str(), abilityName.c_str());
    if (bundleName.empty() || abilityName.empty()) {
        APP_LOGW("Want error, bundleName or abilityName empty");
        return false;
    }

    std::string keyName = bundleName + abilityName;
    APP_LOGI("ability, name:%{public}s", keyName.c_str());
    std::lock_guard<std::mutex> lock(bundleInfoMutex_);
    if (bundleInfos_.empty()) {
        APP_LOGI("bundleInfos_ is empty");
        return false;
    }
    auto item = bundleInfos_.find(bundleName);
    if (item == bundleInfos_.end()) {
        APP_LOGI("bundle:%{public}s not find", bundleName.c_str());
        return false;
    }

    auto infoWithIdItem = item->second.find(Constants::CURRENT_DEVICE_ID);
    if (infoWithIdItem == item->second.end()) {
        APP_LOGI("bundle:%{public}s device id not find", bundleName.c_str());
        return false;
    }
    if (infoWithIdItem->second.IsDisabled()) {
        APP_LOGI("app %{public}s is disabled", infoWithIdItem->second.GetBundleName().c_str());
        return false;
    }
    auto ability = infoWithIdItem->second.FindAbilityInfo(bundleName, abilityName);
    if (!ability) {
        APP_LOGE("ability:%{public}s not find", keyName.c_str());
        return false;
    }
    abilityInfo = (*ability);
    infoWithIdItem->second.GetApplicationInfo(
        ApplicationFlag::GET_APPLICATION_INFO_WITH_PERMS, 0, abilityInfo.applicationInfo);
    return true;
}

bool BundleDataMgr::QueryAbilityInfos(const Want &want, std::vector<AbilityInfo> &abilityInfo) const
{
    ElementName element = want.GetElement();
    std::string abilityName = element.GetAbilityName();
    std::string bundleName = element.GetBundleName();
    APP_LOGI("bundle name:%{public}s, ability name:%{public}s", bundleName.c_str(), abilityName.c_str());

    std::string keyName = bundleName + abilityName;
    APP_LOGI("ability, name:%{public}s", keyName.c_str());
    std::lock_guard<std::mutex> lock(bundleInfoMutex_);
    if (bundleInfos_.empty()) {
        APP_LOGI("bundleInfos_ is empty");
        return false;
    }
    auto item = bundleInfos_.find(bundleName);
    if (item == bundleInfos_.end()) {
        APP_LOGI("bundle:%{public}s not find", bundleName.c_str());
        return false;
    }

    auto infoWithIdItem = item->second.find(Constants::CURRENT_DEVICE_ID);
    if (infoWithIdItem == item->second.end()) {
        APP_LOGI("bundle:%{public}s device id not find", bundleName.c_str());
        return false;
    }
    if (infoWithIdItem->second.IsDisabled()) {
        APP_LOGI("app %{public}s is disabled", infoWithIdItem->second.GetBundleName().c_str());
        return false;
    }
    auto ability = infoWithIdItem->second.FindAbilityInfos(bundleName);
    if (!ability) {
        APP_LOGE("ability:%{public}s not find", keyName.c_str());
        return false;
    }
    abilityInfo = (*ability);
    for (auto &ability : abilityInfo) {
        infoWithIdItem->second.GetApplicationInfo(
            ApplicationFlag::GET_APPLICATION_INFO_WITH_PERMS, 0, ability.applicationInfo);
    }

    return true;
}

bool BundleDataMgr::QueryAbilityInfoByUri(const std::string &abilityUri, AbilityInfo &abilityInfo) const
{
    APP_LOGI("abilityUri is %{public}s", abilityUri.c_str());
    if (abilityUri.empty()) {
        return false;
    }
    if (abilityUri.find(Constants::DATA_ABILITY_URI_PREFIX) == std::string::npos) {
        return false;
    }
    std::lock_guard<std::mutex> lock(bundleInfoMutex_);
    if (bundleInfos_.empty()) {
        APP_LOGI("bundleInfos_ data is empty");
        return false;
    }
    std::string noPpefixUri = abilityUri.substr(Constants::DATA_ABILITY_URI_PREFIX.size());
    auto posFirstSeparator = noPpefixUri.find(Constants::DATA_ABILITY_URI_SEPARATOR);
    if (posFirstSeparator == std::string::npos) {
        return false;
    }
    auto posSecondSeparator = noPpefixUri.find(Constants::DATA_ABILITY_URI_SEPARATOR, posFirstSeparator + 1);
    std::string uri;
    if (posSecondSeparator == std::string::npos) {
        uri = noPpefixUri.substr(posFirstSeparator + 1, noPpefixUri.size() - posFirstSeparator - 1);
    } else {
        uri = noPpefixUri.substr(posFirstSeparator + 1, posSecondSeparator - posFirstSeparator - 1);
    }

    std::string deviceId = noPpefixUri.substr(0, posFirstSeparator);
    if (deviceId.empty()) {
        deviceId = Constants::CURRENT_DEVICE_ID;
    }
    for (const auto &item : bundleInfos_) {
        auto infoWithIdItem = item.second.find(deviceId);
        if (infoWithIdItem == item.second.end()) {
            APP_LOGI("bundle device id:%{public}s not find", deviceId.c_str());
            return false;
        }
        if (infoWithIdItem->second.IsDisabled()) {
            APP_LOGI("app %{public}s is disabled", infoWithIdItem->second.GetBundleName().c_str());
            continue;
        }
        auto ability = infoWithIdItem->second.FindAbilityInfoByUri(uri);
        if (!ability) {
            continue;
        }
        abilityInfo = (*ability);
        infoWithIdItem->second.GetApplicationInfo(
            ApplicationFlag::GET_APPLICATION_INFO_WITH_PERMS, 0, abilityInfo.applicationInfo);
        return true;
    }
    return false;
}

bool BundleDataMgr::GetApplicationInfo(
    const std::string &appName, const ApplicationFlag flag, const int userId, ApplicationInfo &appInfo) const
{
    APP_LOGI("GetApplicationInfo %{public}s", appName.c_str());
    if (appName.empty()) {
        return false;
    }
    std::lock_guard<std::mutex> lock(bundleInfoMutex_);
    if (bundleInfos_.empty()) {
        APP_LOGI("bundleInfos_ data is empty");
        return false;
    }
    APP_LOGI("GetApplicationInfo %{public}s", appName.c_str());
    auto infoItem = bundleInfos_.find(appName);
    if (infoItem == bundleInfos_.end()) {
        return false;
    }
    APP_LOGI("GetApplicationInfo %{public}s", infoItem->first.c_str());
    auto bundleInfo = infoItem->second.find(Constants::CURRENT_DEVICE_ID);
    if (bundleInfo == infoItem->second.end()) {
        return false;
    }
    if (bundleInfo->second.IsDisabled()) {
        APP_LOGI("app %{public}s is disabled", bundleInfo->second.GetBundleName().c_str());
        return false;
    }
    bundleInfo->second.GetApplicationInfo(flag, userId, appInfo);
    return true;
}

bool BundleDataMgr::GetApplicationInfos(
    const ApplicationFlag flag, const int userId, std::vector<ApplicationInfo> &appInfos) const
{
    std::lock_guard<std::mutex> lock(bundleInfoMutex_);
    if (bundleInfos_.empty()) {
        APP_LOGI("bundleInfos_ data is empty");
        return false;
    }
    bool find = false;
    for (const auto &item : bundleInfos_) {
        for (const auto &info : item.second) {
            if (info.second.IsDisabled()) {
                APP_LOGI("app %{public}s is disabled", info.second.GetBundleName().c_str());
                continue;
            }
            ApplicationInfo appInfo;
            info.second.GetApplicationInfo(flag, userId, appInfo);
            appInfos.emplace_back(appInfo);
            find = true;
        }
    }
    APP_LOGI("get installed bundles success");
    return find;
}

bool BundleDataMgr::GetBundleInfo(const std::string &bundleName, const BundleFlag flag, BundleInfo &bundleInfo) const
{
    if (bundleName.empty()) {
        APP_LOGW("bundle name is empty");
        return false;
    }

    std::lock_guard<std::mutex> lock(bundleInfoMutex_);
    if (bundleInfos_.empty()) {
        APP_LOGI("bundleInfos_ data is empty");
        return false;
    }
    APP_LOGI("GetBundleInfo %{public}s", bundleName.c_str());
    auto infoItem = bundleInfos_.find(bundleName);
    if (infoItem == bundleInfos_.end()) {
        return false;
    }
    APP_LOGI("GetBundleInfo %{public}s", infoItem->first.c_str());
    auto innerBundleInfo = infoItem->second.find(Constants::CURRENT_DEVICE_ID);
    if (innerBundleInfo == infoItem->second.end()) {
        return false;
    }
    if (innerBundleInfo->second.IsDisabled()) {
        APP_LOGI("app %{public}s is disabled", innerBundleInfo->second.GetBundleName().c_str());
        return false;
    }
    innerBundleInfo->second.GetBundleInfo(flag, bundleInfo);
    APP_LOGI("bundle:%{public}s device id find success", bundleName.c_str());
    return true;
}

bool BundleDataMgr::GetBundleInfosByMetaData(const std::string &metaData, std::vector<BundleInfo> &bundleInfos) const
{
    if (metaData.empty()) {
        APP_LOGW("bundle name is empty");
        return false;
    }

    std::lock_guard<std::mutex> lock(bundleInfoMutex_);
    if (bundleInfos_.empty()) {
        APP_LOGI("bundleInfos_ data is empty");
        return false;
    }
    bool find = false;
    for (const auto &item : bundleInfos_) {
        for (const auto &info : item.second) {
            if (info.second.IsDisabled()) {
                APP_LOGI("app %{public}s is disabled", info.second.GetBundleName().c_str());
                continue;
            }
            if (info.second.CheckSpecialMetaData(metaData)) {
                BundleInfo bundleInfo;
                info.second.GetBundleInfo(BundleFlag::GET_BUNDLE_WITH_ABILITIES, bundleInfo);
                bundleInfos.emplace_back(bundleInfo);
                find = true;
            }
        }
    }
    return find;
}

bool BundleDataMgr::GetBundleList(std::vector<std::string> &bundleNames) const
{
    std::lock_guard<std::mutex> lock(bundleInfoMutex_);
    if (bundleInfos_.empty()) {
        APP_LOGI("bundleInfos_ data is empty");
        return false;
    }
    bool find = false;
    for (const auto &item : bundleInfos_) {
        bundleNames.emplace_back(item.first);
        find = true;
    }
    APP_LOGI("get installed bundles success");
    return find;
}

bool BundleDataMgr::GetBundleInfos(const BundleFlag flag, std::vector<BundleInfo> &bundleInfos) const
{
    std::lock_guard<std::mutex> lock(bundleInfoMutex_);
    if (bundleInfos_.empty()) {
        APP_LOGI("bundleInfos_ data is empty");
        return false;
    }
    bool find = false;
    for (const auto &item : bundleInfos_) {
        std::map<std::string, InnerBundleInfo> infoWithId = item.second;
        auto infoWithIdItem = infoWithId.find(Constants::CURRENT_DEVICE_ID);
        if (infoWithIdItem == infoWithId.end()) {
            APP_LOGI("current device id bundle not find");
            continue;
        }
        if (infoWithIdItem->second.IsDisabled()) {
            APP_LOGI("app %{public}s is disabled", infoWithIdItem->second.GetBundleName().c_str());
            continue;
        }
        BundleInfo bundleInfo;
        infoWithIdItem->second.GetBundleInfo(flag, bundleInfo);
        bundleInfos.emplace_back(bundleInfo);
        find = true;
    }
    APP_LOGI("get installed bundle infos success");
    return find;
}

bool BundleDataMgr::GetBundleNameForUid(const int uid, std::string &bundleName) const
{
    std::lock_guard<std::mutex> lock(bundleInfoMutex_);
    if (bundleInfos_.empty()) {
        APP_LOGI("bundleInfos_ data is empty");
        return false;
    }
    for (const auto &item : bundleInfos_) {
        for (const auto &info : item.second) {
            if (info.second.IsDisabled()) {
                APP_LOGI("app %{public}s is disabled", info.second.GetBundleName().c_str());
                continue;
            }
            if (info.second.GetUid() == uid) {
                bundleName = info.second.GetBundleName();
                return true;
            }
        }
    }
    return false;
}

bool BundleDataMgr::GetBundlesForUid(const int uid, std::vector<std::string> &bundleNames) const
{
    std::lock_guard<std::mutex> lock(bundleInfoMutex_);
    if (bundleInfos_.empty()) {
        APP_LOGI("bundleInfos_ data is empty");
        return false;
    }
    for (const auto &item : bundleInfos_) {
        for (const auto &info : item.second) {
            if (info.second.GetUid() == uid) {
                bundleNames.emplace_back(info.second.GetBundleName());
                return true;
            }
        }
    }
    return false;
}

bool BundleDataMgr::GetNameForUid(const int uid, std::string &name) const
{
    std::lock_guard<std::mutex> lock(bundleInfoMutex_);
    if (bundleInfos_.empty()) {
        APP_LOGI("bundleInfos_ data is empty");
        return false;
    }
    for (const auto &item : bundleInfos_) {
        for (const auto &info : item.second) {
            if (info.second.GetUid() == uid) {
                name = info.second.GetBundleName();
                return true;
            }
        }
    }
    return false;
}

bool BundleDataMgr::GetBundleGids(const std::string &bundleName, std::vector<int> &gids) const
{
    return true;
}

bool BundleDataMgr::QueryKeepAliveBundleInfos(std::vector<BundleInfo> &bundleInfos) const
{
    std::lock_guard<std::mutex> lock(bundleInfoMutex_);
    if (bundleInfos_.empty()) {
        APP_LOGI("bundleInfos_ data is empty");
        return false;
    }
    for (const auto &item : bundleInfos_) {
        for (const auto &info : item.second) {
            if (info.second.IsDisabled()) {
                APP_LOGI("app %{public}s is disabled", info.second.GetBundleName().c_str());
                continue;
            }
            if (info.second.GetIsKeepAlive()) {
                BundleInfo bundleInfo;
                info.second.GetBundleInfo(BundleFlag::GET_BUNDLE_WITH_ABILITIES, bundleInfo);
                bundleInfos.emplace_back(bundleInfo);
            }
        }
    }
    return !(bundleInfos.empty());
}

std::string BundleDataMgr::GetAbilityLabel(const std::string &bundleName, const std::string &className) const
{
    std::lock_guard<std::mutex> lock(bundleInfoMutex_);
    if (bundleInfos_.empty()) {
        APP_LOGI("bundleInfos_ data is empty");
        return Constants::EMPTY_STRING;
    }
    APP_LOGI("GetAbilityLabel %{public}s", bundleName.c_str());
    auto infoItem = bundleInfos_.find(bundleName);
    if (infoItem == bundleInfos_.end()) {
        return Constants::EMPTY_STRING;
    }
    auto innerBundleInfo = infoItem->second.find(Constants::CURRENT_DEVICE_ID);
    if (innerBundleInfo == infoItem->second.end()) {
        return Constants::EMPTY_STRING;
    }
    if (innerBundleInfo->second.IsDisabled()) {
        APP_LOGI("app %{public}s is disabled", innerBundleInfo->second.GetBundleName().c_str());
        return Constants::EMPTY_STRING;
    }
    auto ability = innerBundleInfo->second.FindAbilityInfo(bundleName, className);
    if (!ability) {
        return Constants::EMPTY_STRING;
    }
    return (*ability).label;
}

bool BundleDataMgr::GetHapModuleInfo(const AbilityInfo &abilityInfo, HapModuleInfo &hapModuleInfo) const
{
    std::lock_guard<std::mutex> lock(bundleInfoMutex_);
    if (bundleInfos_.empty()) {
        APP_LOGI("bundleInfos_ data is empty");
        return false;
    }
    APP_LOGI("GetHapModuleInfo %{public}s", abilityInfo.bundleName.c_str());
    auto infoItem = bundleInfos_.find(abilityInfo.bundleName);
    if (infoItem == bundleInfos_.end()) {
        return false;
    }
    auto innerBundleInfo = infoItem->second.find(Constants::CURRENT_DEVICE_ID);
    if (innerBundleInfo == infoItem->second.end()) {
        return false;
    }
    if (innerBundleInfo->second.IsDisabled()) {
        APP_LOGI("app %{public}s is disabled", innerBundleInfo->second.GetBundleName().c_str());
        return false;
    }
    auto module = innerBundleInfo->second.FindHapModuleInfo(abilityInfo.package);
    if (!module) {
        APP_LOGE("can not find module %{public}s", abilityInfo.package.c_str());
        return false;
    }
    hapModuleInfo = *module;
    return true;
}

bool BundleDataMgr::GetLaunchWantForBundle(const std::string &bundleName, Want &want) const
{
    std::lock_guard<std::mutex> lock(bundleInfoMutex_);
    if (bundleInfos_.empty()) {
        APP_LOGI("bundleInfos_ data is empty");
        return false;
    }
    APP_LOGI("GetLaunchWantForBundle %{public}s", bundleName.c_str());
    auto infoItem = bundleInfos_.find(bundleName);
    if (infoItem == bundleInfos_.end()) {
        return false;
    }
    auto innerBundleInfo = infoItem->second.find(Constants::CURRENT_DEVICE_ID);
    if (innerBundleInfo == infoItem->second.end()) {
        return false;
    }
    if (innerBundleInfo->second.IsDisabled()) {
        APP_LOGI("app %{public}s is disabled", innerBundleInfo->second.GetBundleName().c_str());
        return false;
    }
    std::string mainAbility = innerBundleInfo->second.GetMainAbility();
    if (mainAbility.empty()) {
        APP_LOGE("no main ability in the bundle %{public}s", bundleName.c_str());
        return false;
    }
    auto skills = innerBundleInfo->second.FindSkills(mainAbility);
    if (!skills || (*skills).empty()) {
        APP_LOGE("no skills of %{public}s", mainAbility.c_str());
        return false;
    }

    want.SetElementName(Constants::CURRENT_DEVICE_ID, bundleName, mainAbility);
    want.SetAction((*skills)[0].actions[0]);
    for (auto &skill : (*skills)) {
        for (auto &entity : skill.entities) {
            want.AddEntity(entity);
        }
    }
    return true;
}

bool BundleDataMgr::CheckIsSystemAppByUid(const int uid) const
{
    int maxSysUid{Constants::MAX_SYS_UID};
    int baseSysUid{Constants::ROOT_UID};
    if (uid >= baseSysUid && uid <= maxSysUid) {
        return true;
    }
    return false;
}

void BundleDataMgr::InitStateTransferMap()
{
    transferStates_.emplace(InstallState::INSTALL_SUCCESS, InstallState::INSTALL_START);
    transferStates_.emplace(InstallState::INSTALL_FAIL, InstallState::INSTALL_START);
    transferStates_.emplace(InstallState::UNINSTALL_START, InstallState::INSTALL_SUCCESS);
    transferStates_.emplace(InstallState::UNINSTALL_FAIL, InstallState::UNINSTALL_START);
    transferStates_.emplace(InstallState::UNINSTALL_SUCCESS, InstallState::UNINSTALL_START);
    transferStates_.emplace(InstallState::UPDATING_START, InstallState::INSTALL_SUCCESS);
    transferStates_.emplace(InstallState::UPDATING_SUCCESS, InstallState::UPDATING_START);
    transferStates_.emplace(InstallState::UPDATING_FAIL, InstallState::UPDATING_START);
    transferStates_.emplace(InstallState::INSTALL_SUCCESS, InstallState::UPDATING_START);
    transferStates_.emplace(InstallState::INSTALL_SUCCESS, InstallState::UPDATING_SUCCESS);
    transferStates_.emplace(InstallState::INSTALL_SUCCESS, InstallState::UNINSTALL_START);
}

bool BundleDataMgr::IsDeleteDataState(const InstallState state) const
{
    return (state == InstallState::INSTALL_FAIL || state == InstallState::UNINSTALL_FAIL ||
            state == InstallState::UNINSTALL_SUCCESS || state == InstallState::UPDATING_FAIL);
}

bool BundleDataMgr::IsDisableState(const InstallState state) const
{
    if (state == InstallState::UPDATING_START || state == InstallState::UNINSTALL_START) {
        return true;
    }
    return false;
}

void BundleDataMgr::DeleteBundleInfo(const std::string &bundleName, const InstallState state)
{
    if (InstallState::INSTALL_FAIL == state) {
        APP_LOGW("del fail, bundle:%{public}s has no installed info", bundleName.c_str());
        return;
    }

    std::lock_guard<std::mutex> lock(bundleInfoMutex_);
    auto infoItem = bundleInfos_.find(bundleName);
    if (infoItem != bundleInfos_.end()) {
        APP_LOGI("del bundle name:%{public}s", bundleName.c_str());
        const InnerBundleInfo &innerBundleInfo = infoItem->second[Constants::CURRENT_DEVICE_ID];
        RecycleUidAndGid(innerBundleInfo);
        bool ret = dataStorage_->DeleteStorageBundleInfo(Constants::CURRENT_DEVICE_ID, innerBundleInfo);
        if (!ret) {
            APP_LOGW("delete storage error name:%{public}s", bundleName.c_str());
        }
        // only delete self-device bundle
        infoItem->second.erase(Constants::CURRENT_DEVICE_ID);
        if (infoItem->second.empty()) {
            APP_LOGD("now only store current device installed info, delete all");
            bundleInfos_.erase(bundleName);
        }
    }
}

bool BundleDataMgr::IsAppOrAbilityInstalled(const std::string &bundleName) const
{
    if (bundleName.empty()) {
        APP_LOGW("name:%{public}s empty", bundleName.c_str());
        return false;
    }

    std::lock_guard<std::mutex> lock(stateMutex_);
    auto statusItem = installStates_.find(bundleName);
    if (statusItem == installStates_.end()) {
        APP_LOGW("name:%{public}s not find", bundleName.c_str());
        return false;
    }

    if (statusItem->second == InstallState::INSTALL_SUCCESS) {
        return true;
    }

    APP_LOGW("name:%{public}s not install success", bundleName.c_str());
    return false;
}

bool BundleDataMgr::GetInnerBundleInfo(
    const std::string &bundleName, const std::string &deviceId, InnerBundleInfo &info)
{
    APP_LOGI("GetInnerBundleInfo %{public}s", bundleName.c_str());
    if (bundleName.empty() || deviceId.empty()) {
        APP_LOGE("bundleName or deviceId empty");
        return false;
    }

    std::lock_guard<std::mutex> lock(bundleInfoMutex_);
    auto infoItem = bundleInfos_.find(bundleName);
    if (infoItem == bundleInfos_.end()) {
        APP_LOGE("can not find bundle %{public}s", bundleName.c_str());
        return false;
    }
    auto infoWithIdItem = infoItem->second.find(deviceId);
    if (infoWithIdItem == infoItem->second.end()) {
        APP_LOGE("bundle:%{public}s device id not find", bundleName.c_str());
        return false;
    }
    infoWithIdItem->second.SetBundleStatus(InnerBundleInfo::BundleStatus::DISABLED);
    info = infoWithIdItem->second;
    return true;
}

bool BundleDataMgr::DisableBundle(const std::string &bundleName)
{
    APP_LOGI("DisableBundle %{public}s", bundleName.c_str());
    if (bundleName.empty()) {
        APP_LOGE("bundleName empty");
        return false;
    }

    std::lock_guard<std::mutex> lock(bundleInfoMutex_);
    auto infoItem = bundleInfos_.find(bundleName);
    if (infoItem == bundleInfos_.end()) {
        APP_LOGE("can not find bundle %{public}s", bundleName.c_str());
        return false;
    }
    auto infoWithIdItem = infoItem->second.find(Constants::CURRENT_DEVICE_ID);
    if (infoWithIdItem == infoItem->second.end()) {
        APP_LOGE("bundle:%{public}s device id not find", bundleName.c_str());
        return false;
    }
    infoWithIdItem->second.SetBundleStatus(InnerBundleInfo::BundleStatus::DISABLED);
    return true;
}

bool BundleDataMgr::EnableBundle(const std::string &bundleName)
{
    APP_LOGI("EnableBundle %{public}s", bundleName.c_str());
    if (bundleName.empty()) {
        APP_LOGE("bundleName empty");
        return false;
    }

    std::lock_guard<std::mutex> lock(bundleInfoMutex_);
    auto infoItem = bundleInfos_.find(bundleName);
    if (infoItem == bundleInfos_.end()) {
        APP_LOGE("can not find bundle %{public}s", bundleName.c_str());
        return false;
    }
    auto infoWithIdItem = infoItem->second.find(Constants::CURRENT_DEVICE_ID);
    if (infoWithIdItem == infoItem->second.end()) {
        APP_LOGE("bundle:%{public}s device id not find", bundleName.c_str());
        return false;
    }
    infoWithIdItem->second.SetBundleStatus(InnerBundleInfo::BundleStatus::ENABLED);
    return true;
}

bool BundleDataMgr::IsApplicationEnabled(const std::string &bundleName) const
{
    APP_LOGI("IsApplicationEnabled %{public}s", bundleName.c_str());
    if (bundleName.empty()) {
        APP_LOGE("bundleName empty");
        return false;
    }

    std::lock_guard<std::mutex> lock(bundleInfoMutex_);
    auto infoItem = bundleInfos_.find(bundleName);
    if (infoItem == bundleInfos_.end()) {
        APP_LOGE("can not find bundle %{public}s", bundleName.c_str());
        return false;
    }
    auto infoWithIdItem = infoItem->second.find(Constants::CURRENT_DEVICE_ID);
    if (infoWithIdItem == infoItem->second.end()) {
        APP_LOGE("bundle:%{public}s device id not find", bundleName.c_str());
        return false;
    }
    return infoWithIdItem->second.GetApplicationEnabled();
}

bool BundleDataMgr::SetApplicationEnabled(const std::string &bundleName, bool isEnable)
{
    APP_LOGI("SetApplicationEnabled %{public}s", bundleName.c_str());
    if (bundleName.empty()) {
        APP_LOGE("bundleName empty");
        return false;
    }

    std::lock_guard<std::mutex> lock(bundleInfoMutex_);
    auto infoItem = bundleInfos_.find(bundleName);
    if (infoItem == bundleInfos_.end()) {
        APP_LOGE("can not find bundle %{public}s", bundleName.c_str());
        return false;
    }
    auto infoWithIdItem = infoItem->second.find(Constants::CURRENT_DEVICE_ID);
    if (infoWithIdItem == infoItem->second.end()) {
        APP_LOGE("bundle:%{public}s device id not find", bundleName.c_str());
        return false;
    }
    infoWithIdItem->second.SetApplicationEnabled(isEnable);
    return true;
}

bool BundleDataMgr::IsAbilityEnabled(const AbilityInfo &abilityInfo) const
{
    APP_LOGI("IsAbilityEnabled %{public}s", abilityInfo.name.c_str());
    std::lock_guard<std::mutex> lock(bundleInfoMutex_);
    if (bundleInfos_.empty()) {
        APP_LOGI("bundleInfos_ data is empty");
        return false;
    }
    APP_LOGI("IsAbilityEnabled %{public}s", abilityInfo.bundleName.c_str());
    auto infoItem = bundleInfos_.find(abilityInfo.bundleName);
    if (infoItem == bundleInfos_.end()) {
        return false;
    }
    auto innerBundleInfo = infoItem->second.find(Constants::CURRENT_DEVICE_ID);
    if (innerBundleInfo == infoItem->second.end()) {
        return false;
    }
    auto ability = innerBundleInfo->second.FindAbilityInfo(abilityInfo.bundleName, abilityInfo.name);
    if (!ability) {
        return false;
    }
    return (*ability).enabled;
}

bool BundleDataMgr::SetAbilityEnabled(const AbilityInfo &abilityInfo, bool isEnabled)
{
    APP_LOGI("IsAbilityEnabled %{public}s", abilityInfo.name.c_str());
    std::lock_guard<std::mutex> lock(bundleInfoMutex_);
    if (bundleInfos_.empty()) {
        APP_LOGI("bundleInfos_ data is empty");
        return false;
    }
    APP_LOGI("IsAbilityEnabled %{public}s", abilityInfo.bundleName.c_str());
    auto infoItem = bundleInfos_.find(abilityInfo.bundleName);
    if (infoItem == bundleInfos_.end()) {
        return false;
    }
    auto innerBundleInfo = infoItem->second.find(Constants::CURRENT_DEVICE_ID);
    if (innerBundleInfo == infoItem->second.end()) {
        return false;
    }
    return innerBundleInfo->second.SetAbilityEnabled(abilityInfo.bundleName, abilityInfo.name, isEnabled);
}

std::string BundleDataMgr::GetAbilityIcon(const std::string &bundleName, const std::string &className) const
{
    std::lock_guard<std::mutex> lock(bundleInfoMutex_);
    if (bundleInfos_.empty()) {
        APP_LOGI("bundleInfos_ data is empty");
        return Constants::EMPTY_STRING;
    }
    APP_LOGI("GetAbilityIcon %{public}s", bundleName.c_str());
    auto infoItem = bundleInfos_.find(bundleName);
    if (infoItem == bundleInfos_.end()) {
        return Constants::EMPTY_STRING;
    }
    auto innerBundleInfo = infoItem->second.find(Constants::CURRENT_DEVICE_ID);
    if (innerBundleInfo == infoItem->second.end()) {
        return Constants::EMPTY_STRING;
    }
    auto ability = innerBundleInfo->second.FindAbilityInfo(bundleName, className);
    if (!ability) {
        return Constants::EMPTY_STRING;
    }
    return (*ability).iconPath;
}

bool BundleDataMgr::RegisterBundleStatusCallback(const sptr<IBundleStatusCallback> &bundleStatusCallback)
{
    APP_LOGI("RegisterBundleStatusCallback %{public}s", bundleStatusCallback->GetBundleName().c_str());
    std::lock_guard<std::mutex> lock(callbackMutex_);
    callbackList_.emplace_back(bundleStatusCallback);
    if (bundleStatusCallback->AsObject() != nullptr) {
        sptr<BundleStatusCallbackDeathRecipient> deathRecipient = new BundleStatusCallbackDeathRecipient();
        bundleStatusCallback->AsObject()->AddDeathRecipient(deathRecipient);
    }
    return true;
}

bool BundleDataMgr::ClearBundleStatusCallback(const sptr<IBundleStatusCallback> &bundleStatusCallback)
{
    APP_LOGI("ClearBundleStatusCallback %{public}s", bundleStatusCallback->GetBundleName().c_str());
    std::lock_guard<std::mutex> lock(callbackMutex_);
    callbackList_.erase(std::remove_if(callbackList_.begin(),
                            callbackList_.end(),
                            [&](const sptr<IBundleStatusCallback> &callback) {
                                return callback->AsObject() == bundleStatusCallback->AsObject();
                            }),
        callbackList_.end());
    return true;
}

bool BundleDataMgr::UnregisterBundleStatusCallback()
{
    std::lock_guard<std::mutex> lock(callbackMutex_);
    callbackList_.clear();
    return true;
}

bool BundleDataMgr::GenerateUidAndGid(InnerBundleInfo &info)
{
    int baseUid;
    std::map<int, std::string> &innerMap = [&]() -> decltype(auto) {
        switch (info.GetAppType()) {
            case Constants::AppType::SYSTEM_APP:
                baseUid = Constants::BASE_SYS_UID;
                return (sysUidMap_);
            case Constants::AppType::THIRD_SYSTEM_APP:
                baseUid = Constants::BASE_SYS_VEN_UID;
                return (sysVendorUidMap_);
            case Constants::AppType::THIRD_PARTY_APP:
                baseUid = Constants::BASE_APP_UID;
                return (appUidMap_);
            default:
                APP_LOGE("app type error");
                baseUid = Constants::BASE_APP_UID;
                return (appUidMap_);
        }
    }();
    std::lock_guard<std::mutex> lock(uidMapMutex_);
    if (innerMap.empty()) {
        APP_LOGI("first app install");
        innerMap.emplace(0, info.GetBundleName());
        info.SetUid(baseUid);
        info.SetGid(baseUid);
        return true;
    }
    int uid = 0;
    for (int i = 0; i < innerMap.rbegin()->first; ++i) {
        if (innerMap.find(i) == innerMap.end()) {
            APP_LOGI("the %{public}d app install", i);
            innerMap.emplace(i, info.GetBundleName());
            uid = i + baseUid;
            info.SetUid(baseUid);
            info.SetGid(baseUid);
            return true;
        }
    }
    if ((info.GetAppType() == Constants::AppType::SYSTEM_APP) && (innerMap.rbegin()->first == Constants::MAX_SYS_UID)) {
        return false;
    }
    if ((info.GetAppType() == Constants::AppType::THIRD_SYSTEM_APP) &&
        (innerMap.rbegin()->first == Constants::MAX_SYS_VEN_UID)) {
        return false;
    }

    innerMap.emplace((innerMap.rbegin()->first + 1), info.GetBundleName());
    uid = innerMap.rbegin()->first + baseUid;
    APP_LOGI("the uid is %{public}d", uid);
    info.SetUid(uid);
    info.SetGid(uid);
    return true;
}

bool BundleDataMgr::RecycleUidAndGid(const InnerBundleInfo &info)
{
    std::map<int, std::string> &innerMap = [&]() -> decltype(auto) {
        switch (info.GetAppType()) {
            case Constants::AppType::SYSTEM_APP:
                return (sysUidMap_);
            case Constants::AppType::THIRD_SYSTEM_APP:
                return (sysVendorUidMap_);
            case Constants::AppType::THIRD_PARTY_APP:
                return (appUidMap_);
            default:
                APP_LOGE("app type error");
                return (appUidMap_);
        }
    }();
    for (auto &kv : innerMap) {
        if (kv.second == info.GetBundleName()) {
            innerMap.erase(kv.first);
            return true;
        }
    }
    return true;
}

bool BundleDataMgr::GetUsageRecords(const int32_t maxNum, std::vector<ModuleUsageRecord> &records)
{
    APP_LOGI("GetUsageRecords, maxNum: %{public}d", maxNum);
    records.clear();
    std::vector<ModuleUsageRecord> usageRecords;
    bool result = usageRecordStorage_->QueryRecordByNum(maxNum, usageRecords, 0);
    if (!result) {
        APP_LOGI("GetUsageRecords error");
        return false;
    }
    for (ModuleUsageRecord &item : usageRecords) {
        APP_LOGD("GetUsageRecords item:%{public}s,%{public}s,%{public}s",
            item.bundleName.c_str(),
            item.name.c_str(),
            item.abilityName.c_str());

        std::lock_guard<std::mutex> lock(bundleInfoMutex_);
        if (bundleInfos_.empty()) {
            APP_LOGI("bundleInfos_ data is empty");
            break;
        }
        auto infoItem = bundleInfos_.find(item.bundleName);
        if (infoItem == bundleInfos_.end()) {
            continue;
        }
        APP_LOGI("GetUsageRecords %{public}s", infoItem->first.c_str());
        auto bundleInfo = infoItem->second.find(Constants::CURRENT_DEVICE_ID);
        if (bundleInfo == infoItem->second.end()) {
            continue;
        }
        if (bundleInfo->second.IsDisabled()) {
            APP_LOGI("app %{public}s is disabled", bundleInfo->second.GetBundleName().c_str());
            continue;
        }
        auto innerModuleInfo = bundleInfo->second.GetInnerModuleInfoByModuleName(item.name);
        if (!innerModuleInfo) {
            continue;
        }
        item.labelId = innerModuleInfo->labelId;
        item.descriptionId = innerModuleInfo->descriptionId;
        item.installationFreeSupported = innerModuleInfo->installationFree;
        auto appInfo = bundleInfo->second.GetBaseApplicationInfo();
        item.appLabelId = appInfo.labelId;
        auto ability = bundleInfo->second.FindAbilityInfo(item.bundleName, item.abilityName);
        if (!ability) {
            APP_LOGE("ability:%{public}s not find", item.abilityName.c_str());
            continue;
        }
        if (ability->type != AbilityType::PAGE) {
            APP_LOGE("ability:%{public}s type is not PAGE", item.abilityName.c_str());
            continue;
        }
        item.abilityName = ability->name;
        item.abilityLabelId = ability->labelId;
        item.abilityDescriptionId = ability->descriptionId;
        item.abilityIconId = ability->iconId;
        records.emplace_back(item);
    }
    return true;
}

bool BundleDataMgr::RestoreUidAndGid()
{
    // this function should be called with bundleInfoMutex_ locked
    for (const auto &item : bundleInfos_) {
        for (const auto &info : item.second) {
            uint32_t uid = info.second.GetUid();
            if ((uid < Constants::BASE_SYS_VEN_UID) && (uid >= Constants::BASE_SYS_UID)) {
                sysUidMap_[uid - Constants::BASE_SYS_UID] = info.second.GetBundleName();
            } else if ((uid >= Constants::BASE_SYS_VEN_UID) && (uid <= Constants::MAX_SYS_VEN_UID)) {
                sysVendorUidMap_[uid - Constants::BASE_SYS_VEN_UID] = info.second.GetBundleName();
            } else if (uid > Constants::MAX_SYS_VEN_UID) {
                appUidMap_[uid - Constants::BASE_APP_UID] = info.second.GetBundleName();
            }
        }
    }
    return true;
}

bool BundleDataMgr::NotifyBundleStatus(const std::string &bundleName, const std::string &modulePackage,
    const std::string &mainAbility, const ErrCode resultCode, const NotifyType type, const int32_t &uid)
{
    APP_LOGI("notify type %{public}d with %{public}d for %{public}s-%{public}s in %{public}s",
        type,
        resultCode,
        modulePackage.c_str(),
        mainAbility.c_str(),
        bundleName.c_str());
    uint8_t installType = [&]() -> uint8_t {
        if ((type == NotifyType::UNINSTALL_BUNDLE) || (type == NotifyType::UNINSTALL_MODULE)) {
            return static_cast<uint8_t>(InstallType::UNINSTALL_CALLBACK);
        }
        return static_cast<uint8_t>(InstallType::INSTALL_CALLBACK);
    }();
    {
        std::lock_guard<std::mutex> lock(callbackMutex_);
        for (const auto &callback : callbackList_) {
            if (callback->GetBundleName() == bundleName) {
                // if the msg needed, it could convert in the proxy node
                callback->OnBundleStateChanged(installType, resultCode, Constants::EMPTY_STRING, bundleName);
            }
        }
    }

    if (resultCode != ERR_OK) {
        return true;
    }
    std::string eventData = [type]() -> std::string {
        switch (type) {
            case NotifyType::INSTALL:
                return EventFwk::CommonEventSupport::COMMON_EVENT_PACKAGE_ADDED;
            case NotifyType::UNINSTALL_BUNDLE:
                return EventFwk::CommonEventSupport::COMMON_EVENT_PACKAGE_REMOVED;
            case NotifyType::UNINSTALL_MODULE:
                return EventFwk::CommonEventSupport::COMMON_EVENT_PACKAGE_REMOVED;
            case NotifyType::UPDATE:
                return EventFwk::CommonEventSupport::COMMON_EVENT_PACKAGE_CHANGED;
            default:
                APP_LOGE("event type error");
                return EventFwk::CommonEventSupport::COMMON_EVENT_PACKAGE_CHANGED;
        }
    }();
    APP_LOGI("will send event data %{public}s", eventData.c_str());
    Want want;
    want.SetAction(eventData);
    ElementName element;
    element.SetBundleName(bundleName);
    element.SetAbilityName(mainAbility);
    want.SetElement(element);
    want.SetParam(Constants::UID, uid);
    APP_LOGI("want.SetParam uid %{public}d", uid);
    EventFwk::CommonEventData commonData{want};
    EventFwk::CommonEventManager::PublishCommonEvent(commonData);
    return true;
}

std::mutex &BundleDataMgr::GetBundleMutex(const std::string &bundleName)
{
    bundleMutex_.lock_shared();
    auto it = bundleMutexMap_.find(bundleName);
    if (it == bundleMutexMap_.end()) {
        bundleMutex_.unlock_shared();
        std::unique_lock lock{bundleMutex_};
        return bundleMutexMap_[bundleName];
    }
    bundleMutex_.unlock_shared();
    return it->second;
}

bool BundleDataMgr::GetProvisionId(const std::string &bundleName, std::string &provisionId) const
{
    APP_LOGI("GetProvisionId %{public}s", bundleName.c_str());
    if (bundleName.empty()) {
        APP_LOGE("bundleName empty");
        return false;
    }
    std::lock_guard<std::mutex> lock(bundleInfoMutex_);
    auto infoItem = bundleInfos_.find(bundleName);
    if (infoItem == bundleInfos_.end()) {
        APP_LOGE("can not find bundle %{public}s", bundleName.c_str());
        return false;
    }
    auto infoWithIdItem = infoItem->second.find(Constants::CURRENT_DEVICE_ID);
    if (infoWithIdItem == infoItem->second.end()) {
        APP_LOGE("bundle:%{public}s device id not find", bundleName.c_str());
        return false;
    }
    provisionId = infoWithIdItem->second.GetProvisionId();
    return true;
}

bool BundleDataMgr::GetAppFeature(const std::string &bundleName, std::string &appFeature) const
{
    APP_LOGI("GetAppFeature %{public}s", bundleName.c_str());
    if (bundleName.empty()) {
        APP_LOGE("bundleName empty");
        return false;
    }
    std::lock_guard<std::mutex> lock(bundleInfoMutex_);
    auto infoItem = bundleInfos_.find(bundleName);
    if (infoItem == bundleInfos_.end()) {
        APP_LOGE("can not find bundle %{public}s", bundleName.c_str());
        return false;
    }
    auto infoWithIdItem = infoItem->second.find(Constants::CURRENT_DEVICE_ID);
    if (infoWithIdItem == infoItem->second.end()) {
        APP_LOGE("bundle:%{public}s device id not find", bundleName.c_str());
        return false;
    }
    appFeature = infoWithIdItem->second.GetAppFeature();
    return true;
}

void BundleDataMgr::SetAllInstallFlag(bool flag)
{
    APP_LOGI("SetAllInstallFlag %{public}d", flag);
    allInstallFlag_ = flag;
}

int BundleDataMgr::CheckPublicKeys(const std::string &firstBundleName, const std::string &secondBundleName) const
{
    APP_LOGI("CheckPublicKeys %{public}s and %{public}s", firstBundleName.c_str(), secondBundleName.c_str());
    if (firstBundleName.empty() || secondBundleName.empty()) {
        APP_LOGE("bundleName empty");
        return Constants::SIGNATURE_UNKNOWN_BUNDLE;
    }
    std::lock_guard<std::mutex> lock(bundleInfoMutex_);
    auto firstInfoItem = bundleInfos_.find(firstBundleName);
    if (firstInfoItem == bundleInfos_.end()) {
        APP_LOGE("can not find bundle %{public}s", firstBundleName.c_str());
        return Constants::SIGNATURE_UNKNOWN_BUNDLE;
    }
    auto secondInfoItem = bundleInfos_.find(secondBundleName);
    if (secondInfoItem == bundleInfos_.end()) {
        APP_LOGE("can not find bundle %{public}s", secondBundleName.c_str());
        return Constants::SIGNATURE_UNKNOWN_BUNDLE;
    }
    auto firstInfoWithIdItem = firstInfoItem->second.find(Constants::CURRENT_DEVICE_ID);
    if (firstInfoWithIdItem == firstInfoItem->second.end()) {
        APP_LOGE("bundle:%{public}s device id not find", firstBundleName.c_str());
        return Constants::SIGNATURE_UNKNOWN_BUNDLE;
    }
    auto secondInfoWithIdItem = secondInfoItem->second.find(Constants::CURRENT_DEVICE_ID);
    if (secondInfoWithIdItem == secondInfoItem->second.end()) {
        APP_LOGE("bundle:%{public}s device id not find", secondBundleName.c_str());
        return Constants::SIGNATURE_UNKNOWN_BUNDLE;
    }
    auto firstProvisionId = secondInfoWithIdItem->second.GetProvisionId();
    auto secondProvisionId = secondInfoWithIdItem->second.GetProvisionId();
    return (firstProvisionId == secondProvisionId) ? Constants::SIGNATURE_MATCHED : Constants::SIGNATURE_NOT_MATCHED;
}

std::shared_ptr<IBundleDataStorage> BundleDataMgr::GetDataStorage() const
{
    return dataStorage_;
}

bool BundleDataMgr::GetAllFormsInfo(std::vector<FormInfo> &formInfos) const
{
    std::lock_guard<std::mutex> lock(bundleInfoMutex_);
    if (bundleInfos_.empty()) {
        APP_LOGI("bundleInfos_ data is empty");
        return false;
    }
    auto result = false;
    for (const auto &item : bundleInfos_) {
        for (const auto &info : item.second) {
            if (info.second.IsDisabled()) {
                APP_LOGI("app %{public}s is disabled", info.second.GetBundleName().c_str());
                continue;
            }
            info.second.GetFormsInfoByApp(formInfos);
            result = true;
        }
    }
    APP_LOGE("all the form infos find success");
    return result;
}

bool BundleDataMgr::GetFormsInfoByModule(
    const std::string &bundleName, const std::string &moduleName, std::vector<FormInfo> &formInfos) const
{
    if (bundleName.empty()) {
        APP_LOGW("bundle name is empty");
        return false;
    }
    std::lock_guard<std::mutex> lock(bundleInfoMutex_);
    if (bundleInfos_.empty()) {
        APP_LOGI("bundleInfos_ data is empty");
        return false;
    }
    auto infoItem = bundleInfos_.find(bundleName);
    if (infoItem == bundleInfos_.end()) {
        return false;
    }
    auto innerBundleInfo = infoItem->second.find(Constants::CURRENT_DEVICE_ID);
    if (innerBundleInfo == infoItem->second.end()) {
        return false;
    }
    if (innerBundleInfo->second.IsDisabled()) {
        APP_LOGI("app %{public}s is disabled", innerBundleInfo->second.GetBundleName().c_str());
        return false;
    }
    innerBundleInfo->second.GetFormsInfoByModule(moduleName, formInfos);
    APP_LOGE("module forminfo find success");
    return true;
}

bool BundleDataMgr::GetFormsInfoByApp(const std::string &bundleName, std::vector<FormInfo> &formInfos) const
{
    if (bundleName.empty()) {
        APP_LOGW("bundle name is empty");
        return false;
    }
    std::lock_guard<std::mutex> lock(bundleInfoMutex_);
    if (bundleInfos_.empty()) {
        APP_LOGI("bundleInfos_ data is empty");
        return false;
    }
    auto infoItem = bundleInfos_.find(bundleName);
    if (infoItem == bundleInfos_.end()) {
        return false;
    }
    auto innerBundleInfo = infoItem->second.find(Constants::CURRENT_DEVICE_ID);
    if (innerBundleInfo == infoItem->second.end()) {
        return false;
    }
    if (innerBundleInfo->second.IsDisabled()) {
        APP_LOGI("app %{public}s is disabled", innerBundleInfo->second.GetBundleName().c_str());
        return false;
    }
    innerBundleInfo->second.GetFormsInfoByApp(formInfos);
    APP_LOGE("App forminfo find success");
    return true;
}

bool BundleDataMgr::NotifyActivityLifeStatus(
    const std::string &bundleName, const std::string &abilityName, const int64_t launchTime) const
{
    APP_LOGI("NotifyActivityLifeStatus %{public}s, %{public}s", bundleName.c_str(), abilityName.c_str());
    if (bundleName.empty() || abilityName.empty()) {
        return false;
    }
    std::lock_guard<std::mutex> lock(bundleInfoMutex_);
    if (bundleInfos_.empty()) {
        APP_LOGI("bundleInfos_ data is empty");
        return false;
    }
    auto infoItem = bundleInfos_.find(bundleName);
    if (infoItem == bundleInfos_.end()) {
        return false;
    }
    APP_LOGI("NotifyActivityLifeStatus %{public}s", infoItem->first.c_str());
    auto bundleInfo = infoItem->second.find(Constants::CURRENT_DEVICE_ID);
    if (bundleInfo == infoItem->second.end()) {
        return false;
    }
    if (bundleInfo->second.IsDisabled()) {
        APP_LOGI("app %{public}s is disabled", bundleInfo->second.GetBundleName().c_str());
        return false;
    }
    auto ability = bundleInfo->second.FindAbilityInfo(bundleName, abilityName);
    if (!ability) {
        APP_LOGE("ability:%{public}s not find", abilityName.c_str());
        return false;
    }
    if (ability->type != AbilityType::PAGE) {
        APP_LOGE("ability:%{public}s type is not PAGE", abilityName.c_str());
        return false;
    }
    ModuleUsageRecord moduleUsageRecord;
    moduleUsageRecord.bundleName = bundleName;
    moduleUsageRecord.name = ability->moduleName;
    moduleUsageRecord.abilityName = abilityName;
    moduleUsageRecord.lastLaunchTime = launchTime;
    moduleUsageRecord.launchedCount = 1;
    return usageRecordStorage_->AddOrUpdateRecord(moduleUsageRecord, Constants::CURRENT_DEVICE_ID, 0);
}

bool BundleDataMgr::UpdateUsageRecordOnBundleRemoved(
    bool keepUsage, const int userId, const std::string &bundleName) const
{
    std::lock_guard<std::mutex> lock(bundleInfoMutex_);
    if (bundleInfos_.empty()) {
        APP_LOGI("bundleInfos_ data is empty");
        return false;
    }
    auto infoItem = bundleInfos_.find(bundleName);
    if (infoItem == bundleInfos_.end()) {
        return false;
    }
    APP_LOGI("UpdateUsageRecordOnBundleRemoved %{public}s", infoItem->first.c_str());
    auto bundleInfo = infoItem->second.find(Constants::CURRENT_DEVICE_ID);
    if (bundleInfo == infoItem->second.end()) {
        return false;
    }
    if (bundleInfo->second.IsDisabled()) {
        APP_LOGI("app %{public}s is disabled", bundleInfo->second.GetBundleName().c_str());
        return false;
    }
    std::vector<std::string> moduleNames;
    return keepUsage ? usageRecordStorage_->MarkUsageRecordRemoved(bundleInfo->second, userId)
                     : usageRecordStorage_->DeleteUsageRecord(bundleInfo->second, userId);
}

bool BundleDataMgr::GetShortcutInfos(const std::string &bundleName, std::vector<ShortcutInfo> &shortcutInfos) const
{
    if (bundleName.empty()) {
        APP_LOGW("bundle name is empty");
        return false;
    }
    std::lock_guard<std::mutex> lock(bundleInfoMutex_);
    if (bundleInfos_.empty()) {
        APP_LOGI("bundleInfos_ data is empty");
        return false;
    }
    auto infoItem = bundleInfos_.find(bundleName);
    if (infoItem == bundleInfos_.end()) {
        return false;
    }
    auto innerBundleInfo = infoItem->second.find(Constants::CURRENT_DEVICE_ID);
    if (innerBundleInfo == infoItem->second.end()) {
        return false;
    }
    if (innerBundleInfo->second.IsDisabled()) {
        APP_LOGI("app %{public}s is disabled", innerBundleInfo->second.GetBundleName().c_str());
        return false;
    }
    innerBundleInfo->second.GetShortcutInfos(shortcutInfos);
    APP_LOGE("shortcutInfo find success");
    return true;
}

bool BundleDataMgr::RegisterAllPermissionsChanged(const sptr<OnPermissionChangedCallback> &callback)
{
    if (!callback) {
        APP_LOGE("callback is nullptr");
        return false;
    }
    std::lock_guard<std::mutex> lock(allPermissionsChangedLock_);
    std::set<sptr<OnPermissionChangedCallback>>::iterator it = allPermissionsCallbacks_.begin();
    while (it != allPermissionsCallbacks_.end()) {
        if ((*it)->AsObject() == callback->AsObject()) {
            break;
        }
        it++;
    }
    if (it == allPermissionsCallbacks_.end()) {
        allPermissionsCallbacks_.emplace(callback);
    }
    APP_LOGD("all permissions callbacks size = %{public}zu", allPermissionsCallbacks_.size());
    return AddDeathRecipient(callback);
}

bool BundleDataMgr::RegisterPermissionsChanged(
    const std::vector<int> &uids, const sptr<OnPermissionChangedCallback> &callback)
{
    if (!callback) {
        APP_LOGE("callback is nullptr");
        return false;
    }
    std::lock_guard<std::mutex> lock(permissionsChangedLock_);
    for (int32_t uid : uids) {
        std::set<sptr<OnPermissionChangedCallback>>::iterator it = permissionsCallbacks_[uid].begin();
        while (it != permissionsCallbacks_[uid].end()) {
            if ((*it)->AsObject() == callback->AsObject()) {
                break;
            }
            it++;
        }
        if (it == permissionsCallbacks_[uid].end()) {
            permissionsCallbacks_[uid].emplace(callback);
        }
    }
    APP_LOGD("specified permissions callbacks size = %{public}zu", permissionsCallbacks_.size());

    for (const auto &item1 : permissionsCallbacks_) {
        APP_LOGD("item1->first = %{public}d", item1.first);
        APP_LOGD("item1->second.size() = %{public}zu", item1.second.size());
    }
    return AddDeathRecipient(callback);
}

bool BundleDataMgr::AddDeathRecipient(const sptr<OnPermissionChangedCallback> &callback)
{
    if (!callback) {
        APP_LOGE("callback is nullptr");
        return false;
    }
    auto object = callback->AsObject();
    if (!object) {
        APP_LOGW("callback object is nullptr");
        return false;
    }
    // add callback death recipient.
    sptr<PermissionChangedDeathRecipient> deathRecipient = new PermissionChangedDeathRecipient();
    object->AddDeathRecipient(deathRecipient);
    return true;
}

bool BundleDataMgr::UnregisterPermissionsChanged(const sptr<OnPermissionChangedCallback> &callback)
{
    bool ret = false;
    if (!callback) {
        APP_LOGE("callback is nullptr");
        return ret;
    }
    {
        std::lock_guard<std::mutex> lock(allPermissionsChangedLock_);

        for (auto allPermissionsItem = allPermissionsCallbacks_.begin(); allPermissionsItem != allPermissionsCallbacks_.end();) {
            if ((*allPermissionsItem)->AsObject() == callback->AsObject()) {
                allPermissionsItem = allPermissionsCallbacks_.erase(allPermissionsItem);
                APP_LOGI("unregister from all permissions callbacks success!");
                ret = true;
                break;
            } else {
                allPermissionsItem++;
            }
        }
    }
    {
        std::lock_guard<std::mutex> lock(permissionsChangedLock_);
        for (auto mapIter = permissionsCallbacks_.begin(); mapIter != permissionsCallbacks_.end();) {
            for (auto it = mapIter->second.begin(); it != mapIter->second.end();) {
                if ((*it)->AsObject() == callback->AsObject()) {
                    it = mapIter->second.erase(it);
                    APP_LOGI("unregister from specific permissions callbacks success!");
                    APP_LOGD("mapIter->first = %{public}d", (*mapIter).first);
                    APP_LOGD("*mapIter.second.size() = %{public}zu", (*mapIter).second.size());
                    ret = true;
                } else {
                    it++;
                }
            }
            if (mapIter->second.empty()) {
                mapIter = permissionsCallbacks_.erase(mapIter);
            } else {
                mapIter++;
            }
        }
    }
    for (const auto &item1 : permissionsCallbacks_) {
        APP_LOGD("item1->first = %{public}d", item1.first);
        APP_LOGD("item1->second.size() = %{public}zu", item1.second.size());
    }
    return ret;
}
bool BundleDataMgr::NotifyPermissionsChanged(int32_t uid)
{
    if (uid < 0) {
        APP_LOGE("uid(%{private}d) is invalid", uid);
        return false;
    }
    APP_LOGI("notify permission changed, uid = %{public}d", uid);
    // for all permissions callback.
    {
        std::lock_guard<std::mutex> lock(allPermissionsChangedLock_);
        for (const auto &allPermissionItem : allPermissionsCallbacks_) {
            if (!allPermissionItem) {
                APP_LOGE("callback is nullptr");
                return false;
            }

            allPermissionItem->OnChanged(uid);
            APP_LOGD("all permissions changed callback");
        }
    }
    // for uid permissions callback.
    {
        std::lock_guard<std::mutex> lock(permissionsChangedLock_);
        APP_LOGD("specified permissions callbacks size = %{public}zu", permissionsCallbacks_.size());
        for (const auto &item1 : permissionsCallbacks_) {
            APP_LOGD("item1->first = %{public}d", item1.first);
            APP_LOGD("item1->second.size() = %{public}zu", item1.second.size());
        }
        auto callbackItem = permissionsCallbacks_.find(uid);
        if (callbackItem != permissionsCallbacks_.end()) {
            auto callbacks = callbackItem->second;
            for (const auto &item : callbacks) {
                if (!item) {
                    APP_LOGE("callback is nullptr");
                    return false;
                }
                item->OnChanged(uid);
                APP_LOGD("specified permissions changed callback");
            }
        }
    }
    return true;
}

}  // namespace AppExecFwk
}  // namespace OHOS