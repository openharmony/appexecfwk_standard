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

#include <chrono>
#include <cinttypes>

#include "app_log_wrapper.h"
#include "bundle_constants.h"
#include "bundle_data_storage_database.h"
#include "bundle_status_callback_death_recipient.h"
#include "common_event_manager.h"
#include "common_event_support.h"
#include "json_serializer.h"
#include "nlohmann/json.hpp"
#include "permission/permission_kit.h"
#include "permission_changed_death_recipient.h"

namespace OHOS {
namespace AppExecFwk {
namespace {
constexpr std::string_view ABILTY_NAME = "abilityName";
}

BundleDataMgr::BundleDataMgr()
{
    InitStateTransferMap();
    dataStorage_ = std::make_shared<BundleDataStorageDatabase>();
    usageRecordStorage_ = std::make_shared<ModuleUsageRecordStorage>();
    // register distributed data process death listener.
    usageRecordStorage_->RegisterKvStoreDeathListener();
    preInstallDataStorage_ = std::make_shared<PreInstallDataStorage>();
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
            APP_LOGD("update result:success, current:%{public}d, state:%{public}d", previousState->second, state);
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
    APP_LOGD("to save info:%{public}s", info.GetBundleName().c_str());
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
    if (statusItem->second == InstallState::INSTALL_START) {
        APP_LOGD("save bundle:%{public}s info", bundleName.c_str());
        int64_t time =
            std::chrono::duration_cast<std::chrono::seconds>(std::chrono::system_clock::now().time_since_epoch())
                .count();
        APP_LOGD("the bundle install time is %{public}" PRId64, time);
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

bool BundleDataMgr::SaveNewInfoToDB(const std::string &bundleName, InnerBundleInfo &info)
{
    APP_LOGE("SaveNewInfoToDB start");
    std::string Newbundlename = info.GetDBKeyBundleName();
    APP_LOGI("to save clone newinfo to DB info:%{public}s", Newbundlename.c_str());
    if (bundleName.empty()) {
        APP_LOGW("clone newinfo save info fail, empty bundle name");
        return false;
    }

    std::lock_guard<std::mutex> lock(bundleInfoMutex_);
    auto infoItem = bundleInfos_.find(Newbundlename);
    if (infoItem != bundleInfos_.end()) {
        APP_LOGE("clone newinfo bundle info already exist");
        return false;
    }
    int64_t time =
        std::chrono::duration_cast<std::chrono::seconds>(std::chrono::system_clock::now().time_since_epoch()).count();
    APP_LOGI("the clone newinfo bundle install time is %{public}" PRId64, time);
    info.SetBundleInstallTime(time);
    if (dataStorage_->SaveStorageBundleInfo(Constants::CURRENT_DEVICE_ID, info)) {
        APP_LOGI("clone newinfo write storage success bundle:%{public}s", Newbundlename.c_str());
        std::map<std::string, InnerBundleInfo> infoWithId;
        infoWithId.emplace(Constants::CURRENT_DEVICE_ID, info);
        bundleInfos_.emplace(Newbundlename, infoWithId);
        return true;
    }
    APP_LOGE("SaveNewInfoToDB finish");
    return false;
}

bool BundleDataMgr::AddNewModuleInfo(
    const std::string &bundleName, const InnerBundleInfo &newInfo, InnerBundleInfo &oldInfo)
{
    APP_LOGD("add new module info module name %{public}s ", newInfo.GetCurrentModulePackage().c_str());
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
        APP_LOGD("save bundle:%{public}s info", bundleName.c_str());
        int64_t time =
            std::chrono::duration_cast<std::chrono::seconds>(std::chrono::system_clock::now().time_since_epoch())
                .count();
        APP_LOGD("the bundle update time is %{public}" PRId64, time);
        oldInfo.SetBundleUpdateTime(time);
        oldInfo.UpdateVersionInfo(newInfo);
        oldInfo.updateCommonHapInfo(newInfo);
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
    APP_LOGD("remove module info:%{public}s/%{public}s", bundleName.c_str(), modulePackage.c_str());
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
    if (statusItem->second == InstallState::UNINSTALL_START || statusItem->second == InstallState::ROLL_BACK) {
        APP_LOGD("save bundle:%{public}s info", bundleName.c_str());
        oldInfo.RemoveModuleInfo(modulePackage);
        oldInfo.SetBundleStatus(InnerBundleInfo::BundleStatus::ENABLED);
        if (dataStorage_->DeleteStorageBundleInfo(Constants::CURRENT_DEVICE_ID, oldInfo)) {
            if (dataStorage_->SaveStorageBundleInfo(Constants::CURRENT_DEVICE_ID, oldInfo)) {
                APP_LOGI("update storage success bundle:%{public}s", bundleName.c_str());
                bundleInfos_.at(bundleName).at(Constants::CURRENT_DEVICE_ID) = oldInfo;
                return true;
            }
        }
        APP_LOGD("after delete modulePackage:%{public}s info", modulePackage.c_str());
    }
    return true;
}

bool BundleDataMgr::UpdateInnerBundleInfo(
    const std::string &bundleName, const InnerBundleInfo &newInfo, InnerBundleInfo &oldInfo)
{
    APP_LOGD("update module info:%{public}s", bundleName.c_str());
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
    if (statusItem->second == InstallState::UPDATING_SUCCESS || statusItem->second == InstallState::ROLL_BACK) {
        APP_LOGD("save bundle:%{public}s info", bundleName.c_str());
        int64_t time =
            std::chrono::duration_cast<std::chrono::seconds>(std::chrono::system_clock::now().time_since_epoch())
                .count();
        APP_LOGD("the bundle update time is %{public}" PRId64, time);
        oldInfo.SetBundleUpdateTime(time);
        oldInfo.UpdateVersionInfo(newInfo);
        oldInfo.updateCommonHapInfo(newInfo);
        oldInfo.UpdateModuleInfo(newInfo);
        oldInfo.SetRemovable(newInfo.IsRemovable());
        oldInfo.SetAppType(newInfo.GetAppType());
        oldInfo.SetAppFeature(newInfo.GetAppFeature());
        oldInfo.SetIsPreInstallApp(newInfo.IsPreInstallApp());
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

bool BundleDataMgr::QueryAbilityInfo(const Want &want, int32_t flags, int32_t userId, AbilityInfo &abilityInfo) const
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
                        ApplicationFlag::GET_BASIC_APPLICATION_INFO, 0, abilityInfo.applicationInfo);
                    return true;
                }
            }
        }
        return false;
    }

    ElementName element = want.GetElement();
    std::string bundleName = element.GetBundleName();
    std::string abilityName = element.GetAbilityName();
    APP_LOGD("bundle name:%{public}s, ability name:%{public}s", bundleName.c_str(), abilityName.c_str());
    // explicit query
    if (!bundleName.empty() && !abilityName.empty()) {
        bool ret = ExplicitQueryAbilityInfo(bundleName, abilityName, flags, userId, abilityInfo);
        if (ret == false) {
            APP_LOGE("explicit queryAbilityInfo error");
            return false;
        }
        return true;
    }
    std::vector<AbilityInfo> abilityInfos;
    bool ret = ImplicitQueryAbilityInfos(want, flags, userId, abilityInfos);
    if (ret == false) {
        APP_LOGE("implicit queryAbilityInfos error");
        return false;
    }
    if (abilityInfos.size() == 0) {
        APP_LOGE("no matching abilityInfo");
        return false;
    }
    abilityInfo = abilityInfos[0];
    return true;
}

bool BundleDataMgr::QueryAbilityInfos(
    const Want &want, int32_t flags, int32_t userId, std::vector<AbilityInfo> &abilityInfos) const
{
    ElementName element = want.GetElement();
    std::string bundleName = element.GetBundleName();
    std::string abilityName = element.GetAbilityName();
    APP_LOGD("bundle name:%{public}s, ability name:%{public}s", bundleName.c_str(), abilityName.c_str());
    // explicit query
    if (!bundleName.empty() && !abilityName.empty()) {
        AbilityInfo abilityInfo;
        bool ret = ExplicitQueryAbilityInfo(bundleName, abilityName, flags, userId, abilityInfo);
        if (ret == false) {
            APP_LOGE("explicit queryAbilityInfo error");
            return false;
        }
        abilityInfos.emplace_back(abilityInfo);
        return true;
    }
    // implicit query
    bool ret = ImplicitQueryAbilityInfos(want, flags, userId, abilityInfos);
    if (ret == false) {
        APP_LOGE("implicit queryAbilityInfos error");
        return false;
    }
    if (abilityInfos.size() == 0) {
        APP_LOGE("no matching abilityInfo");
        return false;
    }
    return true;
}

bool BundleDataMgr::ExplicitQueryAbilityInfo(const std::string &bundleName, const std::string &abilityName,
    int32_t flags, int32_t userId, AbilityInfo &abilityInfo) const
{
    APP_LOGD("bundleName:%{public}s, abilityName:%{public}s", bundleName.c_str(), abilityName.c_str());
    APP_LOGD("flags:%{public}d, userId:%{public}d", flags, userId);
    std::lock_guard<std::mutex> lock(bundleInfoMutex_);
    if (bundleInfos_.empty()) {
        APP_LOGE("bundleInfos_ is empty");
        return false;
    }
    auto item = bundleInfos_.find(bundleName);
    if (item == bundleInfos_.end()) {
        APP_LOGE("bundle:%{public}s not find", bundleName.c_str());
        return false;
    }
    auto infoWithIdItem = item->second.find(Constants::CURRENT_DEVICE_ID);
    if (infoWithIdItem == item->second.end()) {
        APP_LOGE("bundle:%{public}s device id not find", bundleName.c_str());
        return false;
    }
    if (infoWithIdItem->second.IsDisabled()) {
        APP_LOGE("app %{public}s is disabled", infoWithIdItem->second.GetBundleName().c_str());
        return false;
    }
    if (!infoWithIdItem->second.GetApplicationEnabled()) {
        APP_LOGE("bundle: %{public}s is disabled", infoWithIdItem->second.GetBundleName().c_str());
        return false;
    }
    auto ability = infoWithIdItem->second.FindAbilityInfo(bundleName, abilityName);
    if (!ability || !ability->enabled) {
        APP_LOGE("ability not found or disabled");
        return false;
    }
    if ((static_cast<uint32_t>(flags) & GET_ABILITY_INFO_WITH_PERMISSION) != GET_ABILITY_INFO_WITH_PERMISSION) {
        ability->permissions.clear();
    }
    if ((static_cast<uint32_t>(flags) & GET_ABILITY_INFO_WITH_METADATA) != GET_ABILITY_INFO_WITH_METADATA) {
        ability->metaData.customizeData.clear();
    }
    abilityInfo = (*ability);
    if ((static_cast<uint32_t>(flags) & GET_ABILITY_INFO_WITH_APPLICATION) == GET_ABILITY_INFO_WITH_APPLICATION) {
        infoWithIdItem->second.GetApplicationInfo(
            ApplicationFlag::GET_BASIC_APPLICATION_INFO, 0, abilityInfo.applicationInfo);
    }
    return true;
}

bool BundleDataMgr::QueryAbilityInfosForClone(const Want &want, std::vector<AbilityInfo> &abilityInfo)
{
    ElementName element = want.GetElement();
    std::string abilityName = element.GetAbilityName();
    std::string bundleName = element.GetBundleName();
    if (bundleName.empty()) {
        return false;
    }
    std::string keyName = bundleName + abilityName;
    std::lock_guard<std::mutex> lock(bundleInfoMutex_);
    if (bundleInfos_.empty()) {
        APP_LOGI("bundleInfos_ is empty");
        return false;
    }
    std::string cloneBundleName = bundleName;
    std::string name = bundleName + "#";
    for (auto it = bundleInfos_.begin(); it != bundleInfos_.end();) {
        if (it->first.find(name) != std::string::npos) {
            cloneBundleName = it->first;
            APP_LOGI("new name is %{public}s", cloneBundleName.c_str());
            break;
        } else {
            ++it;
        }
    }
    if (cloneBundleName != bundleName) {
        auto itemClone = bundleInfos_.find(cloneBundleName);
        if (itemClone == bundleInfos_.end()) {
            APP_LOGI("bundle:%{public}s not find", bundleName.c_str());
            return false;
        }
        auto infoWithIdItemClone = itemClone->second.find(Constants::CURRENT_DEVICE_ID);
        if (infoWithIdItemClone == itemClone->second.end()) {
            return false;
        }
        if (infoWithIdItemClone->second.IsDisabled()) {
            return false;
        }
        infoWithIdItemClone->second.FindAbilityInfosForClone(bundleName, abilityName, abilityInfo);
    }
    auto item = bundleInfos_.find(bundleName);
    if (item == bundleInfos_.end()) {
        APP_LOGI("bundle:%{public}s not find", bundleName.c_str());
        return false;
    }
    auto infoWithIdItem = item->second.find(Constants::CURRENT_DEVICE_ID);
    infoWithIdItem->second.FindAbilityInfosForClone(bundleName, abilityName, abilityInfo);
    if (abilityInfo.size() == 0) {
        return false;
    }
    return true;
}

bool BundleDataMgr::ImplicitQueryAbilityInfos(
    const Want &want, int32_t flags, int32_t userId, std::vector<AbilityInfo> &abilityInfos) const
{
    APP_LOGD("action:%{public}s, uri:%{public}s, type:%{public}s",
        want.GetAction().c_str(), want.GetUriString().c_str(), want.GetType().c_str());
    APP_LOGD("flags:%{public}d, userId:%{public}d", flags, userId);
    std::lock_guard<std::mutex> lock(bundleInfoMutex_);
    if (bundleInfos_.empty()) {
        APP_LOGE("bundleInfos_ is empty");
        return false;
    }
    std::string bundleName = want.GetElement().GetBundleName();
    // query in current bundleName
    if (!bundleName.empty()) {
        auto item = bundleInfos_.find(bundleName);
        if (item == bundleInfos_.end()) {
            APP_LOGE("no bundleName %{public}s found", bundleName.c_str());
            return false;
        }
        auto infoWithIdItem = item->second.find(Constants::CURRENT_DEVICE_ID);
        if (infoWithIdItem != item->second.end()) {
            GetMatchAbilityInfos(want, flags, infoWithIdItem->second, abilityInfos);
        }
        return true;
    }
    // query all
    for (const auto &item : bundleInfos_) {
        auto infoWithIdItem = item.second.find(Constants::CURRENT_DEVICE_ID);
        if (infoWithIdItem != item.second.end()) {
            GetMatchAbilityInfos(want, flags, infoWithIdItem->second, abilityInfos);
        }
    }
    return true;
}

void BundleDataMgr::GetMatchAbilityInfos(
    const Want &want, int32_t flags, const InnerBundleInfo &info, std::vector<AbilityInfo> &abilityInfos) const
{
    if (!info.GetApplicationEnabled() || info.IsDisabled()) {
        APP_LOGE("bundleName: %{public}s is disabled", info.GetBundleName().c_str());
        return;
    }
    std::map<std::string, std::vector<Skill>> skillInfos = info.GetInnerSkillInfos();
    for (const auto &abilityInfoPair : info.GetInnerAbilityInfos()) {
        auto skillsPair = skillInfos.find(abilityInfoPair.first);
        if (skillsPair == skillInfos.end()) {
            continue;
        }
        for (const Skill &skill : skillsPair->second) {
            if (skill.Match(want)) {
                AbilityInfo abilityinfo = abilityInfoPair.second;
                if (!abilityinfo.enabled) {
                    APP_LOGE("abilityinfo: %{public}s is disabled", abilityinfo.name.c_str());
                    continue;
                }
                if ((static_cast<uint32_t>(flags) & GET_ABILITY_INFO_WITH_APPLICATION) ==
                    GET_ABILITY_INFO_WITH_APPLICATION) {
                    info.GetApplicationInfo(
                        ApplicationFlag::GET_BASIC_APPLICATION_INFO, 0, abilityinfo.applicationInfo);
                }
                if ((static_cast<uint32_t>(flags) & GET_ABILITY_INFO_WITH_PERMISSION) !=
                    GET_ABILITY_INFO_WITH_PERMISSION) {
                    abilityinfo.permissions.clear();
                }
                if ((static_cast<uint32_t>(flags) & GET_ABILITY_INFO_WITH_METADATA) != GET_ABILITY_INFO_WITH_METADATA) {
                    abilityinfo.metaData.customizeData.clear();
                }
                abilityInfos.emplace_back(abilityinfo);
                break;
            }
        }
    }
}

bool BundleDataMgr::QueryAllAbilityInfos(uint32_t userId, std::vector<AbilityInfo> &abilityInfos) const
{
    std::lock_guard<std::mutex> lock(bundleInfoMutex_);
    for (const auto& bundle : bundleInfos_) {
        std::string bundleName = bundle.first;
        std::map<std::string, InnerBundleInfo> infoWithId = bundle.second;
        auto infoWithIdItem = infoWithId.find(Constants::CURRENT_DEVICE_ID);
        if (infoWithIdItem == infoWithId.end()) {
            APP_LOGW("bundle:%{public}s device id not find", bundleName.c_str());
            continue;
        }
        if (infoWithIdItem->second.IsDisabled()) {
            APP_LOGW("app %{public}s is disabled", infoWithIdItem->second.GetBundleName().c_str());
            continue;
        }

        // get ability in InnerBundleInfo
        auto itemAbilities = infoWithIdItem->second.FindAbilityInfos(bundleName);
        if (itemAbilities == std::nullopt) {
            continue;
        }

        for (auto& ability : (*itemAbilities)) {
            infoWithIdItem->second.GetApplicationInfo(
                ApplicationFlag::GET_APPLICATION_INFO_WITH_PERMS, 0,
                ability.applicationInfo);
            abilityInfos.emplace_back(ability);
        }
    }
    return true;
}

bool BundleDataMgr::QueryAbilityInfoByUri(const std::string &abilityUri, AbilityInfo &abilityInfo) const
{
    APP_LOGD("abilityUri is %{public}s", abilityUri.c_str());
    if (abilityUri.empty()) {
        return false;
    }
    if (abilityUri.find(Constants::DATA_ABILITY_URI_PREFIX) == std::string::npos) {
        return false;
    }
    std::lock_guard<std::mutex> lock(bundleInfoMutex_);
    if (bundleInfos_.empty()) {
        APP_LOGE("bundleInfos_ data is empty");
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
            APP_LOGE("bundle device id:%{public}s not find", deviceId.c_str());
            return false;
        }
        if (infoWithIdItem->second.IsDisabled()) {
            APP_LOGE("app %{public}s is disabled", infoWithIdItem->second.GetBundleName().c_str());
            continue;
        }
        auto ability = infoWithIdItem->second.FindAbilityInfoByUri(uri);
        if (!ability) {
            continue;
        }
        abilityInfo = (*ability);
        infoWithIdItem->second.GetApplicationInfo(
            ApplicationFlag::GET_BASIC_APPLICATION_INFO, 0, abilityInfo.applicationInfo);
        return true;
    }
    return false;
}

bool BundleDataMgr::QueryAbilityInfosByUri(const std::string &abilityUri, std::vector<AbilityInfo> &abilityInfos)
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
    for (auto &item : bundleInfos_) {
        auto infoWithIdItem = item.second.find(deviceId);
        if (infoWithIdItem == item.second.end()) {
            APP_LOGI("bundle device id:%{public}s not find", deviceId.c_str());
            return false;
        }
        if (infoWithIdItem->second.IsDisabled()) {
            APP_LOGI("app %{public}s is disabled", infoWithIdItem->second.GetBundleName().c_str());
            continue;
        }
        infoWithIdItem->second.FindAbilityInfosByUri(uri, abilityInfos);
    }
    if (abilityInfos.size() == 0) {
        return false;
    }

    return true;
}

bool BundleDataMgr::GetApplicationInfo(
    const std::string &appName, int32_t flags, const int userId, ApplicationInfo &appInfo) const
{
    APP_LOGD("GetApplicationInfo %{public}s", appName.c_str());
    if (appName.empty()) {
        return false;
    }
    std::lock_guard<std::mutex> lock(bundleInfoMutex_);
    if (bundleInfos_.empty()) {
        APP_LOGE("bundleInfos_ data is empty");
        return false;
    }
    APP_LOGD("GetApplicationInfo %{public}s", appName.c_str());
    auto infoItem = bundleInfos_.find(appName);
    if (infoItem == bundleInfos_.end()) {
        return false;
    }
    APP_LOGD("GetApplicationInfo %{public}s", infoItem->first.c_str());
    auto bundleInfo = infoItem->second.find(Constants::CURRENT_DEVICE_ID);
    if (bundleInfo == infoItem->second.end()) {
        return false;
    }
    if (bundleInfo->second.IsDisabled()) {
        APP_LOGE("app %{public}s is disabled", bundleInfo->second.GetBundleName().c_str());
        return false;
    }
    if (!bundleInfo->second.GetApplicationEnabled()) {
        APP_LOGE("bundleName: %{public}s is disabled", bundleInfo->second.GetBundleName().c_str());
        return false;
    }
    bundleInfo->second.GetApplicationInfo(flags, userId, appInfo);
    return true;
}

bool BundleDataMgr::GetApplicationInfos(
    int32_t flags, const int userId, std::vector<ApplicationInfo> &appInfos) const
{
    std::lock_guard<std::mutex> lock(bundleInfoMutex_);
    if (bundleInfos_.empty()) {
        APP_LOGE("bundleInfos_ data is empty");
        return false;
    }
    bool find = false;
    for (const auto &item : bundleInfos_) {
        for (const auto &info : item.second) {
            if (info.second.IsDisabled() || !info.second.GetApplicationEnabled()) {
                APP_LOGE("app %{public}s is disabled", info.second.GetBundleName().c_str());
                continue;
            }
            ApplicationInfo appInfo;
            info.second.GetApplicationInfo(flags, userId, appInfo);
            appInfos.emplace_back(appInfo);
            find = true;
        }
    }
    APP_LOGD("get installed bundles success");
    return find;
}

bool BundleDataMgr::GetBundleInfo(const std::string &bundleName, int32_t flags, BundleInfo &bundleInfo) const
{
    if (bundleName.empty()) {
        APP_LOGE("bundle name is empty");
        return false;
    }

    std::lock_guard<std::mutex> lock(bundleInfoMutex_);
    if (bundleInfos_.empty()) {
        APP_LOGE("bundleInfos_ data is empty");
        return false;
    }
    APP_LOGD("GetBundleInfo %{public}s", bundleName.c_str());
    auto infoItem = bundleInfos_.find(bundleName);
    if (infoItem == bundleInfos_.end()) {
        return false;
    }
    APP_LOGD("GetBundleInfo %{public}s", infoItem->first.c_str());
    auto innerBundleInfo = infoItem->second.find(Constants::CURRENT_DEVICE_ID);
    if (innerBundleInfo == infoItem->second.end()) {
        return false;
    }
    if (innerBundleInfo->second.IsDisabled()) {
        APP_LOGE("app %{public}s is disabled", innerBundleInfo->second.GetBundleName().c_str());
        return false;
    }
    if (!innerBundleInfo->second.GetApplicationEnabled()) {
        APP_LOGE("bundleName %{public}s is disabled", innerBundleInfo->second.GetBundleName().c_str());
        return false;
    }
    innerBundleInfo->second.GetBundleInfo(flags, bundleInfo);
    APP_LOGD("bundle:%{public}s device id find success", bundleName.c_str());
    return true;
}

bool BundleDataMgr::GetBundleInfosByMetaData(const std::string &metaData, std::vector<BundleInfo> &bundleInfos) const
{
    if (metaData.empty()) {
        APP_LOGE("bundle name is empty");
        return false;
    }

    std::lock_guard<std::mutex> lock(bundleInfoMutex_);
    if (bundleInfos_.empty()) {
        APP_LOGE("bundleInfos_ data is empty");
        return false;
    }
    bool find = false;
    for (const auto &item : bundleInfos_) {
        for (const auto &info : item.second) {
            if (info.second.IsDisabled()) {
                APP_LOGW("app %{public}s is disabled", info.second.GetBundleName().c_str());
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
        APP_LOGE("bundleInfos_ data is empty");
        return false;
    }
    bool find = false;
    for (const auto &item : bundleInfos_) {
        bundleNames.emplace_back(item.first);
        find = true;
    }
    APP_LOGD("get installed bundles success");
    return find;
}

bool BundleDataMgr::GetBundleInfos(int32_t flags, std::vector<BundleInfo> &bundleInfos) const
{
    std::lock_guard<std::mutex> lock(bundleInfoMutex_);
    if (bundleInfos_.empty()) {
        APP_LOGE("bundleInfos_ data is empty");
        return false;
    }
    bool find = false;
    for (const auto &item : bundleInfos_) {
        std::map<std::string, InnerBundleInfo> infoWithId = item.second;
        auto infoWithIdItem = infoWithId.find(Constants::CURRENT_DEVICE_ID);
        if (infoWithIdItem == infoWithId.end()) {
            APP_LOGW("current device id bundle not find");
            continue;
        }
        if (infoWithIdItem->second.IsDisabled()) {
            APP_LOGW("app %{public}s is disabled", infoWithIdItem->second.GetBundleName().c_str());
            continue;
        }
        if (!infoWithIdItem->second.GetApplicationEnabled()) {
            APP_LOGW("bundleName: %{public}s is disabled", infoWithIdItem->second.GetBundleName().c_str());
            continue;
        }
        BundleInfo bundleInfo;
        infoWithIdItem->second.GetBundleInfo(flags, bundleInfo);
        bundleInfos.emplace_back(bundleInfo);
        find = true;
    }
    APP_LOGD("get installed bundle infos success");
    return find;
}

bool BundleDataMgr::GetBundleNameForUid(const int uid, std::string &bundleName) const
{
    std::lock_guard<std::mutex> lock(bundleInfoMutex_);
    if (bundleInfos_.empty()) {
        APP_LOGE("bundleInfos_ data is empty");
        return false;
    }
    for (const auto &item : bundleInfos_) {
        for (const auto &info : item.second) {
            if (info.second.IsDisabled()) {
                APP_LOGW("app %{public}s is disabled", info.second.GetBundleName().c_str());
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
        APP_LOGE("bundleInfos_ data is empty");
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
        APP_LOGE("bundleInfos_ data is empty");
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

bool BundleDataMgr::GetBundleGidsByUid(const std::string &bundleName, const int &uid, std::vector<int> &gids) const
{
    return true;
}

bool BundleDataMgr::QueryKeepAliveBundleInfos(std::vector<BundleInfo> &bundleInfos) const
{
    std::lock_guard<std::mutex> lock(bundleInfoMutex_);
    if (bundleInfos_.empty()) {
        APP_LOGE("bundleInfos_ data is empty");
        return false;
    }
    for (const auto &item : bundleInfos_) {
        for (const auto &info : item.second) {
            if (info.second.IsDisabled()) {
                APP_LOGW("app %{public}s is disabled", info.second.GetBundleName().c_str());
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
        APP_LOGW("bundleInfos_ data is empty");
        return Constants::EMPTY_STRING;
    }
    APP_LOGD("GetAbilityLabel %{public}s", bundleName.c_str());
    auto infoItem = bundleInfos_.find(bundleName);
    if (infoItem == bundleInfos_.end()) {
        return Constants::EMPTY_STRING;
    }
    auto innerBundleInfo = infoItem->second.find(Constants::CURRENT_DEVICE_ID);
    if (innerBundleInfo == infoItem->second.end()) {
        return Constants::EMPTY_STRING;
    }
    if (innerBundleInfo->second.IsDisabled()) {
        APP_LOGW("app %{public}s is disabled", innerBundleInfo->second.GetBundleName().c_str());
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
        APP_LOGE("bundleInfos_ data is empty");
        return false;
    }
    APP_LOGD("GetHapModuleInfo %{public}s", abilityInfo.bundleName.c_str());
    auto infoItem = bundleInfos_.find(abilityInfo.bundleName);
    if (infoItem == bundleInfos_.end()) {
        return false;
    }
    auto innerBundleInfo = infoItem->second.find(Constants::CURRENT_DEVICE_ID);
    if (innerBundleInfo == infoItem->second.end()) {
        return false;
    }
    if (innerBundleInfo->second.IsDisabled()) {
        APP_LOGE("app %{public}s is disabled", innerBundleInfo->second.GetBundleName().c_str());
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
        APP_LOGE("bundleInfos_ data is empty");
        return false;
    }
    APP_LOGD("GetLaunchWantForBundle %{public}s", bundleName.c_str());
    auto infoItem = bundleInfos_.find(bundleName);
    if (infoItem == bundleInfos_.end()) {
        return false;
    }
    auto innerBundleInfo = infoItem->second.find(Constants::CURRENT_DEVICE_ID);
    if (innerBundleInfo == infoItem->second.end()) {
        return false;
    }
    if (innerBundleInfo->second.IsDisabled()) {
        APP_LOGE("app %{public}s is disabled", innerBundleInfo->second.GetBundleName().c_str());
        return false;
    }
    if (!innerBundleInfo->second.GetApplicationEnabled()) {
        APP_LOGE("bundleName: %{public}s is disabled", innerBundleInfo->second.GetBundleName().c_str());
        return false;
    }
    std::string mainAbility = innerBundleInfo->second.GetMainAbilityName();
    if (mainAbility.empty()) {
        APP_LOGE("no main ability in the bundle %{public}s", bundleName.c_str());
        return false;
    }
    want.SetElementName(Constants::CURRENT_DEVICE_ID, bundleName, mainAbility);
    want.SetAction(Constants::INTENT_ACTION_HOME);
    want.AddEntity(Constants::INTENT_ENTITY_HOME);
    return true;
}

bool BundleDataMgr::CheckIsSystemAppByUid(const int uid) const
{
    int maxSysUid {Constants::MAX_SYS_UID};
    int baseSysUid {Constants::ROOT_UID};
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
    transferStates_.emplace(InstallState::UNINSTALL_START, InstallState::INSTALL_START);
    transferStates_.emplace(InstallState::UNINSTALL_START, InstallState::UPDATING_SUCCESS);
    transferStates_.emplace(InstallState::UNINSTALL_FAIL, InstallState::UNINSTALL_START);
    transferStates_.emplace(InstallState::UNINSTALL_SUCCESS, InstallState::UNINSTALL_START);
    transferStates_.emplace(InstallState::UPDATING_START, InstallState::INSTALL_SUCCESS);
    transferStates_.emplace(InstallState::UPDATING_SUCCESS, InstallState::UPDATING_START);
    transferStates_.emplace(InstallState::UPDATING_FAIL, InstallState::UPDATING_START);
    transferStates_.emplace(InstallState::UPDATING_FAIL, InstallState::INSTALL_START);
    transferStates_.emplace(InstallState::UPDATING_START, InstallState::INSTALL_START);
    transferStates_.emplace(InstallState::INSTALL_SUCCESS, InstallState::UPDATING_START);
    transferStates_.emplace(InstallState::INSTALL_SUCCESS, InstallState::UPDATING_SUCCESS);
    transferStates_.emplace(InstallState::INSTALL_SUCCESS, InstallState::UNINSTALL_START);
    transferStates_.emplace(InstallState::UPDATING_START, InstallState::UPDATING_SUCCESS);
    transferStates_.emplace(InstallState::ROLL_BACK, InstallState::UPDATING_START);
    transferStates_.emplace(InstallState::ROLL_BACK, InstallState::UPDATING_SUCCESS);
    transferStates_.emplace(InstallState::INSTALL_SUCCESS, InstallState::ROLL_BACK);
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
        APP_LOGD("del bundle name:%{public}s", bundleName.c_str());
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
    APP_LOGD("GetInnerBundleInfo %{public}s", bundleName.c_str());
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
    APP_LOGD("DisableBundle %{public}s", bundleName.c_str());
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
    APP_LOGD("EnableBundle %{public}s", bundleName.c_str());
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
    APP_LOGD("IsApplicationEnabled %{public}s", bundleName.c_str());
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
    APP_LOGD("SetApplicationEnabled %{public}s", bundleName.c_str());
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
    InnerBundleInfo newInfo = infoWithIdItem->second;
    newInfo.SetApplicationEnabled(isEnable);
    if (dataStorage_->SaveStorageBundleInfo(Constants::CURRENT_DEVICE_ID, newInfo)) {
        infoWithIdItem->second.SetApplicationEnabled(isEnable);
        return true;
    } else {
        APP_LOGE("bundle:%{private}s SetApplicationEnabled failed", bundleName.c_str());
        return false;
    }
}

bool BundleDataMgr::IsAbilityEnabled(const AbilityInfo &abilityInfo) const
{
    APP_LOGD("IsAbilityEnabled %{public}s", abilityInfo.name.c_str());
    std::lock_guard<std::mutex> lock(bundleInfoMutex_);
    if (bundleInfos_.empty()) {
        APP_LOGE("bundleInfos_ data is empty");
        return false;
    }
    APP_LOGD("IsAbilityEnabled %{public}s", abilityInfo.bundleName.c_str());
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
    APP_LOGD("IsAbilityEnabled %{public}s", abilityInfo.name.c_str());
    std::lock_guard<std::mutex> lock(bundleInfoMutex_);
    if (bundleInfos_.empty()) {
        APP_LOGE("bundleInfos_ data is empty");
        return false;
    }
    APP_LOGD("IsAbilityEnabled %{public}s", abilityInfo.bundleName.c_str());
    auto infoItem = bundleInfos_.find(abilityInfo.bundleName);
    if (infoItem == bundleInfos_.end()) {
        return false;
    }
    auto innerBundleInfo = infoItem->second.find(Constants::CURRENT_DEVICE_ID);
    if (innerBundleInfo == infoItem->second.end()) {
        return false;
    }
    InnerBundleInfo newInfo = innerBundleInfo->second;
    newInfo.SetAbilityEnabled(abilityInfo.bundleName, abilityInfo.name, isEnabled);
    if (dataStorage_->SaveStorageBundleInfo(Constants::CURRENT_DEVICE_ID, newInfo)) {
        return innerBundleInfo->second.SetAbilityEnabled(abilityInfo.bundleName, abilityInfo.name, isEnabled);
    }
    APP_LOGD("dataStorage SetAbilityEnabled %{public}s failed", abilityInfo.bundleName.c_str());
    return false;
}

std::string BundleDataMgr::GetAbilityIcon(const std::string &bundleName, const std::string &className) const
{
    std::lock_guard<std::mutex> lock(bundleInfoMutex_);
    if (bundleInfos_.empty()) {
        APP_LOGW("bundleInfos_ data is empty");
        return Constants::EMPTY_STRING;
    }
    APP_LOGD("GetAbilityIcon %{public}s", bundleName.c_str());
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
    APP_LOGD("RegisterBundleStatusCallback %{public}s", bundleStatusCallback->GetBundleName().c_str());
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
    APP_LOGD("ClearBundleStatusCallback %{public}s", bundleStatusCallback->GetBundleName().c_str());
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
            APP_LOGI("the uid is %{public}d", uid);
            info.SetUid(uid);
            info.SetGid(uid);
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
    APP_LOGD("the uid is %{public}d", uid);
    info.SetUid(uid);
    info.SetGid(uid);
    return true;
}

bool BundleDataMgr::GenerateCloneUid(InnerBundleInfo &info)
{
    auto result = GenerateUidAndGid(info);
    return result;
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
    std::lock_guard<std::mutex> lock(uidMapMutex_);
    for (auto &kv : innerMap) {
        if (kv.second == info.GetBundleName()) {
            APP_LOGI("the recycle uid is %{public}d", kv.first);
            innerMap.erase(kv.first);
            return true;
        }
    }
    return true;
}

bool BundleDataMgr::GetUsageRecords(const int32_t maxNum, std::vector<ModuleUsageRecord> &records)
{
    APP_LOGD("GetUsageRecords, maxNum: %{public}d", maxNum);
    if ((maxNum <= 0) || (maxNum > ProfileReader::MAX_USAGE_RECORD_SIZE)) {
        APP_LOGE("maxNum illegal");
        return false;
    }
    records.clear();
    std::vector<ModuleUsageRecord> usageRecords;
    bool result = usageRecordStorage_->QueryRecordByNum(maxNum, usageRecords, 0);
    if (!result) {
        APP_LOGE("GetUsageRecords error");
        return false;
    }
    for (ModuleUsageRecord &item : usageRecords) {
        APP_LOGD("GetUsageRecords item:%{public}s,%{public}s,%{public}s",
            item.bundleName.c_str(),
            item.name.c_str(),
            item.abilityName.c_str());

        std::lock_guard<std::mutex> lock(bundleInfoMutex_);
        if (bundleInfos_.empty()) {
            APP_LOGW("bundleInfos_ data is empty");
            break;
        }
        auto infoItem = bundleInfos_.find(item.bundleName);
        if (infoItem == bundleInfos_.end()) {
            continue;
        }
        APP_LOGD("GetUsageRecords %{public}s", infoItem->first.c_str());
        auto bundleInfo = infoItem->second.find(Constants::CURRENT_DEVICE_ID);
        if (bundleInfo == infoItem->second.end()) {
            continue;
        }
        if (bundleInfo->second.IsDisabled()) {
            APP_LOGW("app %{public}s is disabled", bundleInfo->second.GetBundleName().c_str());
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
            APP_LOGW("ability:%{public}s not find", item.abilityName.c_str());
            continue;
        }
        if (ability->type != AbilityType::PAGE) {
            APP_LOGW("ability:%{public}s type is not PAGE", item.abilityName.c_str());
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
                std::lock_guard<std::mutex> lock(uidMapMutex_);
                sysUidMap_[uid - Constants::BASE_SYS_UID] = info.second.GetBundleName();
            } else if ((uid >= Constants::BASE_SYS_VEN_UID) && (uid <= Constants::MAX_SYS_VEN_UID)) {
                std::lock_guard<std::mutex> lock(uidMapMutex_);
                sysVendorUidMap_[uid - Constants::BASE_SYS_VEN_UID] = info.second.GetBundleName();
            } else if (uid > Constants::MAX_SYS_VEN_UID) {
                std::lock_guard<std::mutex> lock(uidMapMutex_);
                appUidMap_[uid - Constants::BASE_APP_UID] = info.second.GetBundleName();
            }
        }
    }
    return true;
}

bool BundleDataMgr::NotifyBundleStatus(const std::string& bundleName, const std::string& modulePackage,
    const std::string& abilityName, const ErrCode resultCode, const NotifyType type, const int32_t& uid)
{
    APP_LOGD("notify type %{public}d with %{public}d for %{public}s-%{public}s in %{public}s", type, resultCode,
        modulePackage.c_str(), abilityName.c_str(), bundleName.c_str());
    uint8_t installType = [&]() -> uint8_t {
        if ((type == NotifyType::UNINSTALL_BUNDLE) || (type == NotifyType::UNINSTALL_MODULE)) {
            return static_cast<uint8_t>(InstallType::UNINSTALL_CALLBACK);
        }
        return static_cast<uint8_t>(InstallType::INSTALL_CALLBACK);
    }();
    {
        std::lock_guard<std::mutex> lock(callbackMutex_);
        for (const auto& callback : callbackList_) {
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
            case NotifyType::ABILITY_ENABLE:
                return EventFwk::CommonEventSupport::COMMON_EVENT_PACKAGE_CHANGED;
            case NotifyType::APPLICATION_ENABLE:
                return EventFwk::CommonEventSupport::COMMON_EVENT_PACKAGE_CHANGED;
            default:
                APP_LOGE("event type error");
                return EventFwk::CommonEventSupport::COMMON_EVENT_PACKAGE_CHANGED;
        }
    }();
    APP_LOGD("will send event data %{public}s", eventData.c_str());
    Want want;
    want.SetAction(eventData);
    ElementName element;
    element.SetBundleName(bundleName);
    element.SetAbilityName(abilityName);
    want.SetElement(element);
    want.SetParam(Constants::UID, uid);
    want.SetParam(ABILTY_NAME.data(), abilityName);
    EventFwk::CommonEventData commonData { want };
    EventFwk::CommonEventManager::PublishCommonEvent(commonData);
    return true;
}

std::mutex &BundleDataMgr::GetBundleMutex(const std::string &bundleName)
{
    bundleMutex_.lock_shared();
    auto it = bundleMutexMap_.find(bundleName);
    if (it == bundleMutexMap_.end()) {
        bundleMutex_.unlock_shared();
        std::unique_lock lock {bundleMutex_};
        return bundleMutexMap_[bundleName];
    }
    bundleMutex_.unlock_shared();
    return it->second;
}

bool BundleDataMgr::GetProvisionId(const std::string &bundleName, std::string &provisionId) const
{
    APP_LOGD("GetProvisionId %{public}s", bundleName.c_str());
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
    APP_LOGD("GetAppFeature %{public}s", bundleName.c_str());
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
    APP_LOGD("SetAllInstallFlag %{public}d", flag);
    allInstallFlag_ = flag;
}

int BundleDataMgr::CheckPublicKeys(const std::string &firstBundleName, const std::string &secondBundleName) const
{
    APP_LOGD("CheckPublicKeys %{public}s and %{public}s", firstBundleName.c_str(), secondBundleName.c_str());
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
        APP_LOGE("bundleInfos_ data is empty");
        return false;
    }
    auto result = false;
    for (const auto &item : bundleInfos_) {
        for (const auto &info : item.second) {
            if (info.second.IsDisabled()) {
                APP_LOGW("app %{public}s is disabled", info.second.GetBundleName().c_str());
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
        APP_LOGE("bundleInfos_ data is empty");
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
        APP_LOGE("app %{public}s is disabled", innerBundleInfo->second.GetBundleName().c_str());
        return false;
    }
    innerBundleInfo->second.GetFormsInfoByModule(moduleName, formInfos);
    if (formInfos.empty()) {
        return false;
    }
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
        APP_LOGE("bundleInfos_ data is empty");
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
        APP_LOGE("app %{public}s is disabled", innerBundleInfo->second.GetBundleName().c_str());
        return false;
    }
    innerBundleInfo->second.GetFormsInfoByApp(formInfos);
    APP_LOGE("App forminfo find success");
    return true;
}

bool BundleDataMgr::NotifyActivityLifeStatus(
    const std::string &bundleName, const std::string &abilityName, const int64_t launchTime, const int uid) const
{
    if (bundleName.empty() || abilityName.empty()) {
        return false;
    }
    std::lock_guard<std::mutex> lock(bundleInfoMutex_);
    if (bundleInfos_.empty()) {
        return false;
    }

    int userId = Constants::DEFAULT_USERID;
    std::string cloneBundleName = bundleName;
    for (auto it = bundleInfos_.begin(); it != bundleInfos_.end();) {
        if (it->first.find(cloneBundleName) != std::string::npos) {
            auto bundleInfo = it->second.find(Constants::CURRENT_DEVICE_ID);
            if (bundleInfo != it->second.end() && bundleInfo->second.GetUid() == uid) {
                cloneBundleName = it->first;
                break;
            }
            ++it;
        } else {
            ++it;
        }
    }
    auto infoItem = bundleInfos_.find(cloneBundleName);
    if (infoItem == bundleInfos_.end()) {
        return false;
    }
    auto bundleInfo = infoItem->second.find(Constants::CURRENT_DEVICE_ID);
    if (bundleInfo == infoItem->second.end()) {
        return false;
    }
    if (bundleInfo->second.IsDisabled()) {
        return false;
    }
    auto ability = bundleInfo->second.FindAbilityInfo(bundleName, abilityName);
    if (!ability) {
        return false;
    }
    if (ability->applicationInfo.isCloned == true) {
        userId = Constants::C_UESRID;
    }
    if (ability->type != AbilityType::PAGE) {
        return false;
    }
    ModuleUsageRecord moduleUsageRecord;
    moduleUsageRecord.bundleName = bundleName;
    moduleUsageRecord.name = ability->moduleName;
    moduleUsageRecord.abilityName = abilityName;
    moduleUsageRecord.lastLaunchTime = launchTime;
    moduleUsageRecord.launchedCount = 1;
    return usageRecordStorage_->AddOrUpdateRecord(moduleUsageRecord, Constants::CURRENT_DEVICE_ID, userId);
}

bool BundleDataMgr::UpdateUsageRecordOnBundleRemoved(
    bool keepUsage, const int userId, const std::string &bundleName) const
{
    std::lock_guard<std::mutex> lock(bundleInfoMutex_);
    if (bundleInfos_.empty()) {
        APP_LOGE("bundleInfos_ data is empty");
        return false;
    }
    auto infoItem = bundleInfos_.find(bundleName);
    if (infoItem == bundleInfos_.end()) {
        return false;
    }
    APP_LOGD("UpdateUsageRecordOnBundleRemoved %{public}s", infoItem->first.c_str());
    auto bundleInfo = infoItem->second.find(Constants::CURRENT_DEVICE_ID);
    if (bundleInfo == infoItem->second.end()) {
        return false;
    }
    if (bundleInfo->second.IsDisabled()) {
        APP_LOGE("app %{public}s is disabled", bundleInfo->second.GetBundleName().c_str());
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
        APP_LOGE("bundleInfos_ data is empty");
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
        APP_LOGE("app %{public}s is disabled", innerBundleInfo->second.GetBundleName().c_str());
        return false;
    }
    if (!innerBundleInfo->second.GetApplicationEnabled()) {
        APP_LOGE("bundle %{public}s is disabled", innerBundleInfo->second.GetBundleName().c_str());
        return false;
    }
    innerBundleInfo->second.GetShortcutInfos(shortcutInfos);
    if (shortcutInfos.size() == 0) {
        return false;
    }
    return true;
}

bool BundleDataMgr::GetAllCommonEventInfo(const std::string &eventKey,
    std::vector<CommonEventInfo> &commonEventInfos) const
{
    if (eventKey.empty()) {
        APP_LOGW("event key is empty");
        return false;
    }
    std::lock_guard<std::mutex> lock(bundleInfoMutex_);
    if (bundleInfos_.empty()) {
        APP_LOGI("bundleInfos_ data is empty");
        return false;
    }
    for (auto infoItem : bundleInfos_) {
        auto innerBundleInfo = infoItem.second.find(Constants::CURRENT_DEVICE_ID);
        if (innerBundleInfo == infoItem.second.end()) {
            continue;
        }
        if (innerBundleInfo->second.IsDisabled()) {
            APP_LOGI("app %{public}s is disabled", innerBundleInfo->second.GetBundleName().c_str());
            continue;
        }
        innerBundleInfo->second.GetCommonEvents(eventKey, commonEventInfos);
    }
    if (commonEventInfos.size() == 0) {
        APP_LOGE("commonEventInfos is empty");
        return false;
    }
    APP_LOGE("commonEventInfos find success");
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

        for (auto allPermissionsItem = allPermissionsCallbacks_.begin();
             allPermissionsItem != allPermissionsCallbacks_.end();) {
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
    APP_LOGD("notify permission changed, uid = %{public}d", uid);
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

bool BundleDataMgr::RemoveClonedBundleInfo(const std::string &bundleName)
{
    std::lock_guard<std::mutex> lock(bundleInfoMutex_);
    auto infoItem = bundleInfos_.find(bundleName);
    if (infoItem != bundleInfos_.end()) {
        APP_LOGI("del bundle name:%{public}s", bundleName.c_str());
        const InnerBundleInfo &innerBundleInfo = infoItem->second[Constants::CURRENT_DEVICE_ID];
        RecycleUidAndGid(innerBundleInfo);
        bool ret = dataStorage_->DeleteStorageBundleInfo(Constants::CURRENT_DEVICE_ID, innerBundleInfo);
        if (!ret) {
            APP_LOGW("delete storage error name:%{public}s", bundleName.c_str());
            return false;
        }
        // only delete self-device bundle
        infoItem->second.erase(Constants::CURRENT_DEVICE_ID);
        if (infoItem->second.empty()) {
            APP_LOGI("now only store current device cloned info, delete all");
            bundleInfos_.erase(bundleName);
        }
    }
    return true;
}

bool BundleDataMgr::GetClonedBundleName(const std::string &bundleName, std::string &newName)
{
    APP_LOGI("GetCloneBundleName start");
    std::string name = bundleName + "#";
    for (auto it = bundleInfos_.begin(); it != bundleInfos_.end();) {
        if (it->first.find(name) != std::string::npos) {
            newName = it->first;
            APP_LOGI("new name is %{public}s", newName.c_str());
            return true;
        } else {
            ++it;
        }
    }
    return false;
    APP_LOGI("GetCloneBundleName finish");
}

bool BundleDataMgr::SavePreInstallBundleInfo(
    const std::string &bundleName, const PreInstallBundleInfo &preInstallBundleInfo)
{
    if (!preInstallDataStorage_) {
        return false;
    }

    if (preInstallDataStorage_->SavePreInstallStorageBundleInfo(
        Constants::PRE_INSTALL_DEVICE_ID, preInstallBundleInfo)) {
        APP_LOGD("write storage success bundle:%{public}s", bundleName.c_str());
        return true;
    }

    return false;
}

bool BundleDataMgr::GetPreInstallBundleInfo(
    const std::string &bundleName, PreInstallBundleInfo &preInstallBundleInfo)
{
    if (bundleName.empty()) {
        APP_LOGE("bundleName or deviceId empty");
        return false;
    }

    if (!preInstallDataStorage_) {
        return false;
    }

    if (preInstallDataStorage_->GetPreInstallStorageBundleInfo(
        Constants::PRE_INSTALL_DEVICE_ID, preInstallBundleInfo)) {
        APP_LOGD("get storage success bundle:%{public}s", bundleName.c_str());
        return true;
    }

    return false;
}

bool BundleDataMgr::SaveInstallMark(const InnerBundleInfo &info, bool isAppExisted) const
{
    APP_LOGD("write install mark to storage with bundle:%{public}s", info.GetBundleName().c_str());
    if (!isAppExisted) {
        if (dataStorage_->SaveStorageBundleInfo(Constants::CURRENT_DEVICE_ID, info)) {
            APP_LOGD("save install mark successfully");
            return true;
        }
        APP_LOGE("save install mark failed!");
        return false;
    }
    if (dataStorage_->DeleteStorageBundleInfo(Constants::CURRENT_DEVICE_ID, info) &&
        dataStorage_->SaveStorageBundleInfo(Constants::CURRENT_DEVICE_ID, info)) {
        APP_LOGD("save install mark successfully");
        return true;
    }
    APP_LOGE("save install mark failed!");
    return false;
}
}  // namespace AppExecFwk
}  // namespace OHOS
