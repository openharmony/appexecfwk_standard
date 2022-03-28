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

#include "bundle_data_mgr.h"

#include <chrono>
#include <cinttypes>

#include "app_log_wrapper.h"
#include "bundle_constants.h"
#include "bundle_data_storage_database.h"
#include "bundle_mgr_service.h"
#include "bundle_status_callback_death_recipient.h"
#include "common_event_manager.h"
#include "common_event_support.h"
#include "ipc_skeleton.h"
#include "image_source.h"
#include "json_serializer.h"
#include "nlohmann/json.hpp"
#include "permission_changed_death_recipient.h"
#include "singleton.h"

namespace OHOS {
namespace AppExecFwk {
BundleDataMgr::BundleDataMgr()
{
    InitStateTransferMap();
    dataStorage_ = std::make_shared<BundleDataStorageDatabase>();
    usageRecordStorage_ = std::make_shared<ModuleUsageRecordStorage>();
    // register distributed data process death listener.
    usageRecordStorage_->RegisterKvStoreDeathListener();
    preInstallDataStorage_ = std::make_shared<PreInstallDataStorage>();
    distributedDataStorage_ = DistributedDataStorage::GetInstance();
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

        LoadAllPreInstallBundleInfos(preInstallBundleInfos_);
        RestoreUidAndGid();
        SetInitialUserFlag(true);
    }
    return ret;
}

bool BundleDataMgr::UpdateBundleInstallState(const std::string &bundleName, const InstallState state)
{
    if (bundleName.empty()) {
        APP_LOGW("update result:fail, reason:bundle name is empty");
        return false;
    }

    // always keep lock bundleInfoMutex_ before locking stateMutex_ to avoid deadlock
    std::lock_guard<std::mutex> lck(bundleInfoMutex_);
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
    APP_LOGD("SaveNewInfoToDB start");
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
    APP_LOGD("SaveNewInfoToDB finish");
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
        oldInfo.UpdateBaseBundleInfo(newInfo.GetBaseBundleInfo(), newInfo.HasEntry());
        oldInfo.updateCommonHapInfo(newInfo);
        oldInfo.AddModuleInfo(newInfo);
        oldInfo.SetAppPrivilegeLevel(newInfo.GetAppPrivilegeLevel());
        oldInfo.SetAllowedAcls(newInfo.GetAllowedAcls());
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

bool BundleDataMgr::AddInnerBundleUserInfo(
    const std::string &bundleName, const InnerBundleUserInfo& newUserInfo)
{
    APP_LOGD("AddInnerBundleUserInfo:%{public}s", bundleName.c_str());
    std::lock_guard<std::mutex> lock(bundleInfoMutex_);
    auto infoItem = bundleInfos_.find(bundleName);
    if (infoItem == bundleInfos_.end()) {
        APP_LOGE("bundle info not exist");
        return false;
    }

    std::lock_guard<std::mutex> stateLock(stateMutex_);
    auto& info = bundleInfos_.at(bundleName).at(Constants::CURRENT_DEVICE_ID);
    info.AddInnerBundleUserInfo(newUserInfo);
    info.SetBundleStatus(InnerBundleInfo::BundleStatus::ENABLED);
    if (!dataStorage_->SaveStorageBundleInfo(Constants::CURRENT_DEVICE_ID, info)) {
        APP_LOGE("update storage failed bundle:%{public}s", bundleName.c_str());
        return false;
    }
    return true;
}

bool BundleDataMgr::RemoveInnerBundleUserInfo(
    const std::string &bundleName, int32_t userId)
{
    APP_LOGD("RemoveInnerBundleUserInfo:%{public}s", bundleName.c_str());
    std::lock_guard<std::mutex> lock(bundleInfoMutex_);
    auto infoItem = bundleInfos_.find(bundleName);
    if (infoItem == bundleInfos_.end()) {
        APP_LOGE("bundle info not exist");
        return false;
    }

    std::lock_guard<std::mutex> stateLock(stateMutex_);
    auto& info = bundleInfos_.at(bundleName).at(Constants::CURRENT_DEVICE_ID);
    info.RemoveInnerBundleUserInfo(userId);
    info.SetBundleStatus(InnerBundleInfo::BundleStatus::ENABLED);
    if (!dataStorage_->SaveStorageBundleInfo(Constants::CURRENT_DEVICE_ID, info)) {
        APP_LOGE("update storage failed bundle:%{public}s", bundleName.c_str());
        return false;
    }
    return true;
}

bool BundleDataMgr::UpdateInnerBundleInfo(
    const std::string &bundleName, const InnerBundleInfo &newInfo, InnerBundleInfo &oldInfo)
{
    APP_LOGD("UpdateInnerBundleInfo:%{public}s", bundleName.c_str());
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
    // ROLL_BACK and USER_CHANGE should not be here
    if (statusItem->second == InstallState::UPDATING_SUCCESS
        || statusItem->second == InstallState::ROLL_BACK
        || statusItem->second == InstallState::USER_CHANGE) {
        APP_LOGD("begin to update, bundleName : %{public}s, moduleName : %{public}s",
            bundleName.c_str(), newInfo.GetCurrentModulePackage().c_str());
        // 1.exist entry, update entry.
        // 2.only exist feature, update feature.
        if (newInfo.HasEntry() || !oldInfo.HasEntry()) {
            oldInfo.UpdateBaseBundleInfo(newInfo.GetBaseBundleInfo(), newInfo.HasEntry());
            oldInfo.UpdateBaseApplicationInfo(newInfo.GetBaseApplicationInfo());
            oldInfo.SetMainAbility(newInfo.GetMainAbility());
            oldInfo.SetMainAbilityName(newInfo.GetMainAbilityName());
            oldInfo.SetAppType(newInfo.GetAppType());
            oldInfo.SetAppFeature(newInfo.GetAppFeature());
            oldInfo.SetAppPrivilegeLevel(newInfo.GetAppPrivilegeLevel());
            oldInfo.SetAllowedAcls(newInfo.GetAllowedAcls());
        }
        if (newInfo.HasEntry()) {
            oldInfo.SetHasEntry(true);
        }
        oldInfo.updateCommonHapInfo(newInfo);
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

bool BundleDataMgr::QueryAbilityInfo(const Want &want, int32_t flags, int32_t userId, AbilityInfo &abilityInfo) const
{
    int32_t requestUserId = GetUserId(userId);
    if (requestUserId == Constants::INVALID_USERID) {
        return false;
    }

    // for launcher
    if (want.HasEntity(Want::FLAG_HOME_INTENT_FROM_SYSTEM)) {
        std::lock_guard<std::mutex> lock(bundleInfoMutex_);
        for (const auto &item : bundleInfos_) {
            for (const auto &info : item.second) {
                if (HasInitialUserCreated() && info.second.GetIsLauncherApp()) {
                    APP_LOGI("find launcher app %{public}s", info.second.GetBundleName().c_str());
                    info.second.GetMainAbilityInfo(abilityInfo);
                    int32_t responseUserId = info.second.GetResponseUserId(requestUserId);
                    info.second.GetApplicationInfo(
                        ApplicationFlag::GET_BASIC_APPLICATION_INFO, responseUserId, abilityInfo.applicationInfo);
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
        bool ret = ExplicitQueryAbilityInfo(bundleName, abilityName, flags, requestUserId, abilityInfo);
        if (!ret) {
            APP_LOGE("explicit queryAbilityInfo error");
            return false;
        }
        return true;
    }
    std::vector<AbilityInfo> abilityInfos;
    bool ret = ImplicitQueryAbilityInfos(want, flags, requestUserId, abilityInfos);
    if (!ret) {
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
    int32_t requestUserId = GetUserId(userId);
    if (requestUserId == Constants::INVALID_USERID) {
        return false;
    }

    ElementName element = want.GetElement();
    std::string bundleName = element.GetBundleName();
    std::string abilityName = element.GetAbilityName();
    APP_LOGD("bundle name:%{public}s, ability name:%{public}s", bundleName.c_str(), abilityName.c_str());
    // explicit query
    if (!bundleName.empty() && !abilityName.empty()) {
        AbilityInfo abilityInfo;
        bool ret = ExplicitQueryAbilityInfo(
            bundleName, abilityName, flags, requestUserId, abilityInfo);
        if (!ret) {
            APP_LOGE("explicit queryAbilityInfo error");
            return false;
        }
        abilityInfos.emplace_back(abilityInfo);
        return true;
    }
    // implicit query
    bool ret = ImplicitQueryAbilityInfos(want, flags, requestUserId, abilityInfos);
    if (!ret) {
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
    int32_t requestUserId = GetUserId(userId);
    if (requestUserId == Constants::INVALID_USERID) {
        return false;
    }

    std::lock_guard<std::mutex> lock(bundleInfoMutex_);
    InnerBundleInfo innerBundleInfo;
    if (!GetInnerBundleInfoWithFlags(
        bundleName, flags, Constants::CURRENT_DEVICE_ID, innerBundleInfo, requestUserId)) {
        APP_LOGE("ExplicitQueryAbilityInfo failed");
        return false;
    }

    int32_t responseUserId = innerBundleInfo.GetResponseUserId(requestUserId);
    auto ability = innerBundleInfo.FindAbilityInfo(bundleName, abilityName, responseUserId);
    if (!ability) {
        APP_LOGE("ability not found");
        return false;
    }
    if ((static_cast<uint32_t>(flags) & GET_ABILITY_INFO_SYSTEMAPP_ONLY) == GET_ABILITY_INFO_SYSTEMAPP_ONLY &&
        !innerBundleInfo.IsSystemApp()) {
        return false;
    }
    if (!(static_cast<uint32_t>(flags) & GET_ABILITY_INFO_WITH_DISABLE)) {
        if (!innerBundleInfo.IsAbilityEnabled((*ability), responseUserId)) {
            APP_LOGE("ability:%{public}s is disabled", ability->name.c_str());
            return false;
        }
    }
    if ((static_cast<uint32_t>(flags) & GET_ABILITY_INFO_WITH_PERMISSION) != GET_ABILITY_INFO_WITH_PERMISSION) {
        ability->permissions.clear();
    }
    if ((static_cast<uint32_t>(flags) & GET_ABILITY_INFO_WITH_METADATA) != GET_ABILITY_INFO_WITH_METADATA) {
        ability->metaData.customizeData.clear();
        ability->metadata.clear();
    }
    abilityInfo = (*ability);
    if ((static_cast<uint32_t>(flags) & GET_ABILITY_INFO_WITH_APPLICATION) == GET_ABILITY_INFO_WITH_APPLICATION) {
        innerBundleInfo.GetApplicationInfo(
            ApplicationFlag::GET_BASIC_APPLICATION_INFO, responseUserId, abilityInfo.applicationInfo);
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

        int32_t responseUserId = infoWithIdItemClone->second.GetResponseUserId(GetUserId());
        infoWithIdItemClone->second.FindAbilityInfosForClone(bundleName, abilityName, responseUserId, abilityInfo);
    }

    auto item = bundleInfos_.find(bundleName);
    if (item == bundleInfos_.end()) {
        APP_LOGI("bundle:%{public}s not find", bundleName.c_str());
        return false;
    }
    auto infoWithIdItem = item->second.find(Constants::CURRENT_DEVICE_ID);
    int32_t responseUserId = infoWithIdItem->second.GetResponseUserId(GetUserId());
    infoWithIdItem->second.FindAbilityInfosForClone(bundleName, abilityName, responseUserId, abilityInfo);
    if (abilityInfo.size() == 0) {
        return false;
    }
    return true;
}

bool BundleDataMgr::ImplicitQueryAbilityInfos(
    const Want &want, int32_t flags, int32_t userId, std::vector<AbilityInfo> &abilityInfos) const
{
    int32_t requestUserId = GetUserId(userId);
    if (requestUserId == Constants::INVALID_USERID) {
        return false;
    }

    if (want.GetAction().empty() && want.GetEntities().empty()
        && want.GetUriString().empty() && want.GetType().empty()) {
        APP_LOGE("param invalid");
        return false;
    }
    APP_LOGD("action:%{public}s, uri:%{private}s, type:%{public}s",
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
        InnerBundleInfo innerBundleInfo;
        if (!GetInnerBundleInfoWithFlags(
            bundleName, flags, Constants::CURRENT_DEVICE_ID, innerBundleInfo, requestUserId)) {
            APP_LOGE("ImplicitQueryAbilityInfos failed");
            return false;
        }
        int32_t responseUserId = innerBundleInfo.GetResponseUserId(requestUserId);
        GetMatchAbilityInfos(want, flags, innerBundleInfo, responseUserId, abilityInfos);
        return true;
    }
    // query all
    for (const auto &item : bundleInfos_) {
        InnerBundleInfo innerBundleInfo;
        if (!GetInnerBundleInfoWithFlags(
            item.first, flags, Constants::CURRENT_DEVICE_ID, innerBundleInfo, requestUserId)) {
            APP_LOGE("ImplicitQueryAbilityInfos failed");
            continue;
        }

        int32_t responseUserId = innerBundleInfo.GetResponseUserId(requestUserId);
        GetMatchAbilityInfos(want, flags, innerBundleInfo, responseUserId, abilityInfos);
    }
    return true;
}

void BundleDataMgr::GetMatchAbilityInfos(const Want &want, int32_t flags,
    const InnerBundleInfo &info, int32_t userId, std::vector<AbilityInfo> &abilityInfos) const
{
    if ((static_cast<uint32_t>(flags) & GET_ABILITY_INFO_SYSTEMAPP_ONLY) == GET_ABILITY_INFO_SYSTEMAPP_ONLY &&
        !info.IsSystemApp()) {
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
                if (!(static_cast<uint32_t>(flags) & GET_ABILITY_INFO_WITH_DISABLE)) {
                    if (!info.IsAbilityEnabled(abilityinfo, GetUserId(userId))) {
                        APP_LOGW("GetMatchAbilityInfos %{public}s is disabled", abilityinfo.name.c_str());
                        continue;
                    }
                }
                if ((static_cast<uint32_t>(flags) & GET_ABILITY_INFO_WITH_APPLICATION) ==
                    GET_ABILITY_INFO_WITH_APPLICATION) {
                    info.GetApplicationInfo(
                        ApplicationFlag::GET_BASIC_APPLICATION_INFO, userId, abilityinfo.applicationInfo);
                }
                if ((static_cast<uint32_t>(flags) & GET_ABILITY_INFO_WITH_PERMISSION) !=
                    GET_ABILITY_INFO_WITH_PERMISSION) {
                    abilityinfo.permissions.clear();
                }
                if ((static_cast<uint32_t>(flags) & GET_ABILITY_INFO_WITH_METADATA) != GET_ABILITY_INFO_WITH_METADATA) {
                    abilityinfo.metaData.customizeData.clear();
                    abilityinfo.metadata.clear();
                }
                abilityInfos.emplace_back(abilityinfo);
                break;
            }
        }
    }
}

void BundleDataMgr::GetMatchLauncherAbilityInfos(const Want& want,
    const InnerBundleInfo& info, std::vector<AbilityInfo>& abilityInfos, int32_t userId) const
{
    int32_t requestUserId = GetUserId(userId);
    if (requestUserId == Constants::INVALID_USERID) {
        return;
    }

    int32_t responseUserId = info.GetResponseUserId(requestUserId);
    std::map<std::string, std::vector<Skill>> skillInfos = info.GetInnerSkillInfos();
    for (const auto& abilityInfoPair : info.GetInnerAbilityInfos()) {
        auto skillsPair = skillInfos.find(abilityInfoPair.first);
        if (skillsPair == skillInfos.end()) {
            continue;
        }
        for (const Skill& skill : skillsPair->second) {
            if (skill.MatchLauncher(want)) {
                AbilityInfo abilityinfo = abilityInfoPair.second;
                info.GetApplicationInfo(ApplicationFlag::GET_BASIC_APPLICATION_INFO,
                    responseUserId, abilityinfo.applicationInfo);
                abilityInfos.emplace_back(abilityinfo);
                break;
            }
        }
    }
}

bool BundleDataMgr::QueryLauncherAbilityInfos(
    const Want& want, uint32_t userId, std::vector<AbilityInfo>& abilityInfos) const
{
    int32_t requestUserId = GetUserId(userId);
    if (requestUserId == Constants::INVALID_USERID) {
        return false;
    }

    std::lock_guard<std::mutex> lock(bundleInfoMutex_);
    if (bundleInfos_.empty()) {
        APP_LOGE("bundleInfos_ is empty");
        return false;
    }

    ElementName element = want.GetElement();
    std::string bundleName = element.GetBundleName();
    if (bundleName.empty()) {
        // query all launcher ability
        for (const auto &item : bundleInfos_) {
            auto infoWithIdItem = item.second.find(Constants::CURRENT_DEVICE_ID);
            if (infoWithIdItem != item.second.end() && infoWithIdItem->second.IsDisabled()) {
                APP_LOGI("app %{public}s is disabled", infoWithIdItem->second.GetBundleName().c_str());
                continue;
            }
            if (infoWithIdItem != item.second.end()) {
                GetMatchLauncherAbilityInfos(want, infoWithIdItem->second, abilityInfos, requestUserId);
            }
        }
        return true;
    } else {
        // query definite abilitys by bundle name
        auto item = bundleInfos_.find(bundleName);
        if (item == bundleInfos_.end()) {
            APP_LOGE("no bundleName %{public}s found", bundleName.c_str());
            return false;
        }
        auto infoWithIdItem = item->second.find(Constants::CURRENT_DEVICE_ID);
        if (infoWithIdItem != item->second.end()) {
            GetMatchLauncherAbilityInfos(want, infoWithIdItem->second, abilityInfos, requestUserId);
        }
        return true;
    }
}

bool BundleDataMgr::QueryAbilityInfoByUri(
    const std::string &abilityUri, int32_t userId, AbilityInfo &abilityInfo) const
{
    APP_LOGD("abilityUri is %{private}s", abilityUri.c_str());
    int32_t requestUserId = GetUserId(userId);
    if (requestUserId == Constants::INVALID_USERID) {
        return false;
    }

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
            continue;
        }

        if (infoWithIdItem->second.IsDisabled()) {
            APP_LOGE("app %{public}s is disabled", infoWithIdItem->second.GetBundleName().c_str());
            continue;
        }

        int32_t responseUserId = infoWithIdItem->second.GetResponseUserId(requestUserId);
        if (!infoWithIdItem->second.GetApplicationEnabled(responseUserId)) {
            continue;
        }

        auto ability = infoWithIdItem->second.FindAbilityInfoByUri(uri);
        if (!ability) {
            continue;
        }

        abilityInfo = (*ability);
        infoWithIdItem->second.GetApplicationInfo(
            ApplicationFlag::GET_BASIC_APPLICATION_INFO, responseUserId, abilityInfo.applicationInfo);
        return true;
    }

    APP_LOGE("query abilityUri(%{private}s) failed.", abilityUri.c_str());
    return false;
}

bool BundleDataMgr::QueryAbilityInfosByUri(const std::string &abilityUri, std::vector<AbilityInfo> &abilityInfos)
{
    APP_LOGI("abilityUri is %{private}s", abilityUri.c_str());
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
            APP_LOGI("bundle device id:%{private}s not find", deviceId.c_str());
            return false;
        }
        if (infoWithIdItem->second.IsDisabled()) {
            APP_LOGI("app %{public}s is disabled", infoWithIdItem->second.GetBundleName().c_str());
            continue;
        }
        infoWithIdItem->second.FindAbilityInfosByUri(uri, abilityInfos, GetUserId());
    }
    if (abilityInfos.size() == 0) {
        return false;
    }

    return true;
}

bool BundleDataMgr::GetApplicationInfo(
    const std::string &appName, int32_t flags, const int userId, ApplicationInfo &appInfo) const
{
    int32_t requestUserId = GetUserId(userId);
    if (requestUserId == Constants::INVALID_USERID) {
        return false;
    }

    std::lock_guard<std::mutex> lock(bundleInfoMutex_);
    InnerBundleInfo innerBundleInfo;
    if (!GetInnerBundleInfoWithFlags(
        appName, flags, Constants::CURRENT_DEVICE_ID, innerBundleInfo, requestUserId)) {
        APP_LOGE("GetApplicationInfo failed");
        return false;
    }

    int32_t responseUserId = innerBundleInfo.GetResponseUserId(requestUserId);
    innerBundleInfo.GetApplicationInfo(flags, responseUserId, appInfo);
    return true;
}

bool BundleDataMgr::GetApplicationInfos(
    int32_t flags, const int userId, std::vector<ApplicationInfo> &appInfos) const
{
    int32_t requestUserId = GetUserId(userId);
    if (requestUserId == Constants::INVALID_USERID) {
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
                APP_LOGE("app %{public}s is disabled", info.second.GetBundleName().c_str());
                continue;
            }

            int32_t responseUserId = info.second.GetResponseUserId(requestUserId);
            if (!(static_cast<uint32_t>(flags) & GET_APPLICATION_INFO_WITH_DISABLE)
                && !info.second.GetApplicationEnabled(responseUserId)) {
                APP_LOGD("bundleName: %{public}s is disabled", info.second.GetBundleName().c_str());
                continue;
            }

            ApplicationInfo appInfo;
            info.second.GetApplicationInfo(flags, responseUserId, appInfo);
            appInfos.emplace_back(appInfo);
            find = true;
        }
    }
    APP_LOGD("get installed bundles success");
    return find;
}

bool BundleDataMgr::GetBundleInfo(
    const std::string &bundleName, int32_t flags, BundleInfo &bundleInfo, int32_t userId) const
{
    std::vector<InnerBundleUserInfo> innerBundleUserInfos;
    if (userId == Constants::ANY_USERID) {
        if (!GetInnerBundleUserInfos(bundleName, innerBundleUserInfos)) {
            APP_LOGE("no userInfos for this bundle(%{public}s)", bundleName.c_str());
            return false;
        }
        userId = innerBundleUserInfos.begin()->bundleUserInfo.userId;
    }

    int32_t requestUserId = GetUserId(userId);
    if (requestUserId == Constants::INVALID_USERID) {
        return false;
    }
    std::lock_guard<std::mutex> lock(bundleInfoMutex_);
    InnerBundleInfo innerBundleInfo;
    if (!GetInnerBundleInfoWithFlags(
        bundleName, flags, Constants::CURRENT_DEVICE_ID, innerBundleInfo, requestUserId)) {
        APP_LOGE("GetBundleInfo failed");
        return false;
    }

    int32_t responseUserId = innerBundleInfo.GetResponseUserId(requestUserId);
    innerBundleInfo.GetBundleInfo(flags, bundleInfo, responseUserId);
    APP_LOGD("get bundleInfo(%{public}s) successfully in user(%{public}d)", bundleName.c_str(), userId);
    return true;
}

bool BundleDataMgr::GetBundleInfosByMetaData(
    const std::string &metaData, std::vector<BundleInfo> &bundleInfos) const
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
    int32_t requestUserId = GetUserId();
    for (const auto &item : bundleInfos_) {
        for (const auto &info : item.second) {
            if (info.second.IsDisabled()) {
                APP_LOGW("app %{public}s is disabled", info.second.GetBundleName().c_str());
                continue;
            }
            if (info.second.CheckSpecialMetaData(metaData)) {
                BundleInfo bundleInfo;
                int32_t responseUserId = info.second.GetResponseUserId(requestUserId);
                info.second.GetBundleInfo(
                    BundleFlag::GET_BUNDLE_WITH_ABILITIES, bundleInfo, responseUserId);
                bundleInfos.emplace_back(bundleInfo);
                find = true;
            }
        }
    }
    return find;
}

bool BundleDataMgr::GetBundleList(
    std::vector<std::string> &bundleNames, int32_t userId) const
{
    int32_t requestUserId = GetUserId(userId);
    if (requestUserId == Constants::INVALID_USERID) {
        return false;
    }

    std::lock_guard<std::mutex> lock(bundleInfoMutex_);
    if (bundleInfos_.empty()) {
        APP_LOGE("bundleInfos_ data is empty");
        return false;
    }

    bool find = false;
    for (const auto &infoItem : bundleInfos_) {
        InnerBundleInfo innerBundleInfo;
        if (!GetInnerBundleInfoWithFlags(infoItem.first, BundleFlag::GET_BUNDLE_DEFAULT,
            Constants::CURRENT_DEVICE_ID, innerBundleInfo, requestUserId)) {
            continue;
        }

        bundleNames.emplace_back(infoItem.first);
        find = true;
    }
    APP_LOGD("user(%{public}d) get installed bundles list result(%{public}d)", userId, find);
    return find;
}

bool BundleDataMgr::GetBundleInfos(
    int32_t flags, std::vector<BundleInfo> &bundleInfos, int32_t userId) const
{
    if (userId == Constants::ALL_USERID) {
        return GetAllBundleInfos(flags, bundleInfos);
    }

    int32_t requestUserId = GetUserId(userId);
    if (requestUserId == Constants::INVALID_USERID) {
        return false;
    }

    std::lock_guard<std::mutex> lock(bundleInfoMutex_);
    if (bundleInfos_.empty()) {
        APP_LOGE("bundleInfos_ data is empty");
        return false;
    }

    bool find = false;
    for (const auto &item : bundleInfos_) {
        InnerBundleInfo innerBundleInfo;
        if (!GetInnerBundleInfoWithFlags(
            item.first, flags, Constants::CURRENT_DEVICE_ID, innerBundleInfo, requestUserId)) {
            continue;
        }

        BundleInfo bundleInfo;
        int32_t responseUserId = innerBundleInfo.GetResponseUserId(requestUserId);
        innerBundleInfo.GetBundleInfo(flags, bundleInfo, responseUserId);
        bundleInfos.emplace_back(bundleInfo);
        find = true;
    }
    APP_LOGD("get bundleInfos result(%{public}d) in user(%{public}d).", find, userId);
    return find;
}

bool BundleDataMgr::GetAllBundleInfos(
    int32_t flags, std::vector<BundleInfo> &bundleInfos) const
{
    std::lock_guard<std::mutex> lock(bundleInfoMutex_);
    if (bundleInfos_.empty()) {
        APP_LOGE("bundleInfos_ data is empty");
        return false;
    }

    bool find = false;
    for (const auto &item : bundleInfos_) {
        for (const auto &info : item.second) {
            if (info.second.IsDisabled()) {
                APP_LOGD("app %{public}s is disabled", info.second.GetBundleName().c_str());
                continue;
            }

            BundleInfo bundleInfo;
            info.second.GetBundleInfo(flags, bundleInfo, Constants::ALL_USERID);
            bundleInfos.emplace_back(bundleInfo);
            find = true;
        }
    }

    APP_LOGD("get all bundleInfos result(%{public}d).", find);
    return find;
}

bool BundleDataMgr::GetBundleNameForUid(const int uid, std::string &bundleName) const
{
    InnerBundleInfo innerBundleInfo;
    if (!GetInnerBundleInfoByUid(uid, innerBundleInfo)) {
        APP_LOGE("get innerBundleInfo by uid failed.");
        return false;
    }

    bundleName = innerBundleInfo.GetBundleName();
    return true;
}

bool BundleDataMgr::GetInnerBundleInfoByUid(const int uid, InnerBundleInfo &innerBundleInfo) const
{
    int32_t userId = GetUserIdByUid(uid);
    if (userId == Constants::UNSPECIFIED_USERID || userId == Constants::INVALID_USERID) {
        APP_LOGE("the uid %{public}d is illegal when get bundleName by uid.", uid);
        return false;
    }

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

            if (info.second.GetUid(userId) == uid) {
                innerBundleInfo = info.second;
                return true;
            }
        }
    }

    APP_LOGD("the uid(%{public}d) is not exists.", uid);
    return false;
}

bool BundleDataMgr::GetBundlesForUid(const int uid, std::vector<std::string> &bundleNames) const
{
    InnerBundleInfo innerBundleInfo;
    if (!GetInnerBundleInfoByUid(uid, innerBundleInfo)) {
        APP_LOGE("get innerBundleInfo by uid failed.");
        return false;
    }

    bundleNames.emplace_back(innerBundleInfo.GetBundleName());
    return true;
}

bool BundleDataMgr::GetNameForUid(const int uid, std::string &name) const
{
    InnerBundleInfo innerBundleInfo;
    if (!GetInnerBundleInfoByUid(uid, innerBundleInfo)) {
        APP_LOGE("get innerBundleInfo by uid failed.");
        return false;
    }

    name = innerBundleInfo.GetBundleName();
    return true;
}

bool BundleDataMgr::GetBundleGids(const std::string &bundleName, std::vector<int> &gids) const
{
    int32_t requestUserId = GetUserId();
    InnerBundleUserInfo innerBundleUserInfo;
    if (!GetInnerBundleUserInfoByUserId(bundleName, requestUserId, innerBundleUserInfo)) {
        APP_LOGE("the user(%{public}d) is not exists in bundleName(%{public}s) .",
            requestUserId, bundleName.c_str());
        return false;
    }

    gids = innerBundleUserInfo.gids;
    return true;
}

bool BundleDataMgr::GetBundleGidsByUid(
    const std::string &bundleName, const int &uid, std::vector<int> &gids) const
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

    int32_t requestUserId = GetUserId();
    for (const auto &item : bundleInfos_) {
        for (const auto &info : item.second) {
            if (info.second.IsDisabled()) {
                APP_LOGW("app %{public}s is disabled", info.second.GetBundleName().c_str());
                continue;
            }
            if (info.second.GetIsKeepAlive()) {
                BundleInfo bundleInfo;
                int32_t responseUserId = info.second.GetResponseUserId(requestUserId);
                info.second.GetBundleInfo(BundleFlag::GET_BUNDLE_WITH_ABILITIES, bundleInfo, responseUserId);
                if (bundleInfo.name == "") {
                    continue;
                }
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
        APP_LOGW("app %{public}s is disabled", innerBundleInfo->second.GetBundleName().c_str());
        return Constants::EMPTY_STRING;
    }
    auto ability = innerBundleInfo->second.FindAbilityInfo(bundleName, className, GetUserId());
    if (!ability) {
        return Constants::EMPTY_STRING;
    }
    if ((*ability).labelId == 0) {
        return (*ability).label;
    }
    std::string abilityLabel;
    BundleInfo bundleInfo;
    innerBundleInfo->second.GetBundleInfo(0, bundleInfo, GetUserIdByCallingUid());
    std::shared_ptr<OHOS::Global::Resource::ResourceManager> resourceManager = GetResourceManager(bundleInfo);
    if (resourceManager == nullptr) {
        APP_LOGE("InitResourceManager failed");
        return Constants::EMPTY_STRING;
    }
    OHOS::Global::Resource::RState errval =
        resourceManager->GetStringById(static_cast<uint32_t>((*ability).labelId), abilityLabel);
    if (errval != OHOS::Global::Resource::RState::SUCCESS) {
        return Constants::EMPTY_STRING;
    } else {
        return abilityLabel;
    }
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

    int32_t responseUserId = innerBundleInfo->second.GetResponseUserId(GetUserId());
    auto module = innerBundleInfo->second.FindHapModuleInfo(abilityInfo.package, responseUserId);
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
    InnerBundleInfo innerBundleInfo;

    if (!GetInnerBundleInfoWithFlags(bundleName, BundleFlag::GET_BUNDLE_DEFAULT, Constants::CURRENT_DEVICE_ID,
        innerBundleInfo, GetUserIdByCallingUid())) {
        APP_LOGE("GetLaunchWantForBundle failed");
        return false;
    }
    std::string mainAbility = innerBundleInfo.GetMainAbilityName();
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
    // If the value of uid is 0 (ROOT_UID) or 1000 (BMS_UID),
    // the uid should be the system uid.
    if (uid == Constants::ROOT_UID || uid == Constants::BMS_UID) {
        return true;
    }

    InnerBundleInfo innerBundleInfo;
    if (!GetInnerBundleInfoByUid(uid, innerBundleInfo)) {
        return false;
    }

    return innerBundleInfo.IsSystemApp();
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
    transferStates_.emplace(InstallState::UNINSTALL_START, InstallState::USER_CHANGE);
    transferStates_.emplace(InstallState::UPDATING_START, InstallState::USER_CHANGE);
    transferStates_.emplace(InstallState::INSTALL_SUCCESS, InstallState::USER_CHANGE);
    transferStates_.emplace(InstallState::UPDATING_SUCCESS, InstallState::USER_CHANGE);
    transferStates_.emplace(InstallState::USER_CHANGE, InstallState::INSTALL_SUCCESS);
    transferStates_.emplace(InstallState::USER_CHANGE, InstallState::UPDATING_SUCCESS);
    transferStates_.emplace(InstallState::USER_CHANGE, InstallState::UPDATING_START);
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

bool BundleDataMgr::GetInnerBundleInfoWithFlags(const std::string &bundleName,
    const int32_t flags, const std::string &deviceId, InnerBundleInfo &info, int32_t userId) const
{
    int32_t requestUserId = GetUserId(userId);
    if (requestUserId == Constants::INVALID_USERID) {
        return false;
    }

    if (bundleInfos_.empty()) {
        APP_LOGE("bundleInfos_ data is empty");
        return false;
    }
    APP_LOGD("GetInnerBundleInfoWithFlags: %{public}s", bundleName.c_str());
    auto infoItem = bundleInfos_.find(bundleName);
    if (infoItem == bundleInfos_.end()) {
        APP_LOGE("GetInnerBundleInfoWithFlags: bundleName not find");
        return false;
    }
    auto innerBundleInfo = infoItem->second.find(deviceId);
    if (innerBundleInfo == infoItem->second.end()) {
        APP_LOGE("GetInnerBundleInfoWithFlags: deviceid not find");
        return false;
    }
    if (innerBundleInfo->second.IsDisabled()) {
        APP_LOGE("bundleName: %{public}s status is disabled", innerBundleInfo->second.GetBundleName().c_str());
        return false;
    }

    int32_t responseUserId = innerBundleInfo->second.GetResponseUserId(requestUserId);
    if (!(static_cast<uint32_t>(flags) & GET_APPLICATION_INFO_WITH_DISABLE)
        && !innerBundleInfo->second.GetApplicationEnabled(responseUserId)) {
        APP_LOGE("bundleName: %{public}s is disabled", innerBundleInfo->second.GetBundleName().c_str());
        return false;
    }
    info = innerBundleInfo->second;
    return true;
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

    int32_t responseUserId = infoWithIdItem->second.GetResponseUserId(GetUserId());
    return infoWithIdItem->second.GetApplicationEnabled(responseUserId);
}

bool BundleDataMgr::SetApplicationEnabled(const std::string &bundleName, bool isEnable, int32_t userId)
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
    newInfo.SetApplicationEnabled(isEnable, GetUserId(userId));
    if (dataStorage_->SaveStorageBundleInfo(Constants::CURRENT_DEVICE_ID, newInfo)) {
        infoWithIdItem->second.SetApplicationEnabled(isEnable, GetUserId(userId));
        return true;
    } else {
        APP_LOGE("bundle:%{private}s SetApplicationEnabled failed", bundleName.c_str());
        return false;
    }
}

bool BundleDataMgr::IsAbilityEnabled(const AbilityInfo &abilityInfo) const
{
    int32_t flags = GET_ABILITY_INFO_DEFAULT;
    AbilityInfo abilityInfoIn;
    return ExplicitQueryAbilityInfo(abilityInfo.bundleName, abilityInfo.name, flags, GetUserId(), abilityInfoIn);
}

bool BundleDataMgr::SetAbilityEnabled(const AbilityInfo &abilityInfo, bool isEnabled, int32_t userId)
{
    APP_LOGD("SetAbilityEnabled %{public}s", abilityInfo.name.c_str());
    std::lock_guard<std::mutex> lock(bundleInfoMutex_);
    if (bundleInfos_.empty()) {
        APP_LOGE("bundleInfos_ data is empty");
        return false;
    }
    APP_LOGD("SetAbilityEnabled %{public}s", abilityInfo.bundleName.c_str());
    auto infoItem = bundleInfos_.find(abilityInfo.bundleName);
    if (infoItem == bundleInfos_.end()) {
        return false;
    }
    auto innerBundleInfo = infoItem->second.find(Constants::CURRENT_DEVICE_ID);
    if (innerBundleInfo == infoItem->second.end()) {
        return false;
    }
    InnerBundleInfo newInfo = innerBundleInfo->second;
    newInfo.SetAbilityEnabled(abilityInfo.bundleName, abilityInfo.name, isEnabled, GetUserId(userId));
    if (dataStorage_->SaveStorageBundleInfo(Constants::CURRENT_DEVICE_ID, newInfo)) {
        return innerBundleInfo->second.SetAbilityEnabled(
            abilityInfo.bundleName, abilityInfo.name, isEnabled, GetUserId(userId));
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
    auto ability = innerBundleInfo->second.FindAbilityInfo(bundleName, className, GetUserId());
    if (!ability) {
        return Constants::EMPTY_STRING;
    }
    return (*ability).iconPath;
}

#ifdef SUPPORT_GRAPHICS
std::shared_ptr<Media::PixelMap> BundleDataMgr::GetAbilityPixelMapIcon(const std::string &bundleName,
    const std::string &abilityName) const
{
    std::lock_guard<std::mutex> lock(bundleInfoMutex_);
    if (bundleInfos_.empty()) {
        APP_LOGW("bundleInfos_ data is empty");
        return nullptr;
    }
    APP_LOGD("GetAbilityIcon %{public}s", bundleName.c_str());
    auto infoItem = bundleInfos_.find(bundleName);
    if (infoItem == bundleInfos_.end()) {
        APP_LOGE("can not find bundle %{public}s", bundleName.c_str());
        return nullptr;
    }
    auto innerBundleInfo = infoItem->second.find(Constants::CURRENT_DEVICE_ID);
    if (innerBundleInfo == infoItem->second.end()) {
        APP_LOGE("bundle:%{public}s device id not find", bundleName.c_str());
        return nullptr;
    }
    auto ability = innerBundleInfo->second.FindAbilityInfo(bundleName, abilityName, GetUserId());
    if (!ability) {
        APP_LOGE("abilityName:%{public}s not find", abilityName.c_str());
        return nullptr;
    }
    BundleInfo bundleInfo;
    int32_t flags = 0;
    innerBundleInfo->second.GetBundleInfo(flags, bundleInfo, GetUserId());
    std::shared_ptr<Global::Resource::ResourceManager> resourceManager = GetResourceManager(bundleInfo);
    if (resourceManager == nullptr) {
        APP_LOGE("InitResourceManager failed");
        return nullptr;
    }
    std::string iconPath;
    OHOS::Global::Resource::RState iconPathErrval =
        resourceManager->GetMediaById(static_cast<uint32_t>((*ability).iconId), iconPath);
    if (iconPathErrval != OHOS::Global::Resource::RState::SUCCESS) {
        APP_LOGE("GetMediaById iconPath failed");
        return nullptr;
    }
    APP_LOGD("GetMediaById iconPath: %{private}s", iconPath.c_str());
    auto pixelMapPtr = LoadImageFile(iconPath);
    if (!pixelMapPtr) {
        APP_LOGE("LoadImageFile failed");
        return nullptr;
    }
    return pixelMapPtr;
}
#endif

bool BundleDataMgr::RegisterBundleStatusCallback(const sptr<IBundleStatusCallback> &bundleStatusCallback)
{
    APP_LOGD("RegisterBundleStatusCallback %{public}s", bundleStatusCallback->GetBundleName().c_str());
    std::lock_guard<std::mutex> lock(callbackMutex_);
    callbackList_.emplace_back(bundleStatusCallback);
    if (bundleStatusCallback->AsObject() != nullptr) {
        sptr<BundleStatusCallbackDeathRecipient> deathRecipient =
            new (std::nothrow) BundleStatusCallbackDeathRecipient();
        if (deathRecipient == nullptr) {
            APP_LOGE("deathRecipient is null");
            return false;
        }
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

bool BundleDataMgr::GenerateUidAndGid(InnerBundleUserInfo &innerBundleUserInfo)
{
    if (innerBundleUserInfo.bundleName.empty()) {
        APP_LOGE("bundleName is null.");
        return false;
    }

    int32_t bundleId = Constants::INVALID_BUNDLEID;
    if (!GenerateBundleId(innerBundleUserInfo.bundleName, bundleId)) {
        APP_LOGE("Generate bundleId failed.");
        return false;
    }

    innerBundleUserInfo.uid = innerBundleUserInfo.bundleUserInfo.userId * Constants::BASE_USER_RANGE
        + bundleId % Constants::BASE_USER_RANGE;
    innerBundleUserInfo.gids.emplace_back(innerBundleUserInfo.uid);
    return true;
}

bool BundleDataMgr::GenerateBundleId(const std::string &bundleName, int32_t &bundleId)
{
    std::lock_guard<std::mutex> lock(bundleIdMapMutex_);
    if (bundleIdMap_.empty()) {
        APP_LOGI("first app install");
        bundleId = Constants::BASE_APP_UID;
        bundleIdMap_.emplace(bundleId, bundleName);
        return true;
    }

    for (const auto &innerBundleId : bundleIdMap_) {
        if (innerBundleId.second == bundleName) {
            bundleId = innerBundleId.first;
            return true;
        }
    }

    for (int32_t i = Constants::BASE_APP_UID; i < bundleIdMap_.rbegin()->first; ++i) {
        if (bundleIdMap_.find(i) == bundleIdMap_.end()) {
            APP_LOGI("the %{public}d app install", i);
            bundleId = i;
            bundleIdMap_.emplace(bundleId, bundleName);
            BundleUtil::MakeHmdfsConfig(bundleName, bundleId);
            return true;
        }
    }

    if (bundleIdMap_.rbegin()->first == Constants::MAX_APP_UID) {
        APP_LOGE("the bundleId exceeding the maximum value.");
        return false;
    }

    bundleId = bundleIdMap_.rbegin()->first + 1;
    bundleIdMap_.emplace(bundleId, bundleName);
    BundleUtil::MakeHmdfsConfig(bundleName, bundleId);
    return true;
}

void BundleDataMgr::RecycleUidAndGid(const InnerBundleInfo &info)
{
    auto userInfos = info.GetInnerBundleUserInfos();
    if (userInfos.empty()) {
        return;
    }

    auto innerBundleUserInfo = userInfos.begin()->second;
    int32_t bundleId = innerBundleUserInfo.uid -
        innerBundleUserInfo.bundleUserInfo.userId * Constants::BASE_USER_RANGE;
    std::lock_guard<std::mutex> lock(bundleIdMapMutex_);
    auto infoItem = bundleIdMap_.find(bundleId);
    if (infoItem == bundleIdMap_.end()) {
        return;
    }

    bundleIdMap_.erase(bundleId);
    BundleUtil::RemoveHmdfsConfig(innerBundleUserInfo.bundleName);
}

bool BundleDataMgr::GenerateCloneUid(InnerBundleInfo &info)
{
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
    bool result = usageRecordStorage_->QueryRecordByNum(maxNum, usageRecords, GetUserId());
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
        item.appLabelId = static_cast<uint32_t>(appInfo.labelId);
        auto ability = bundleInfo->second.FindAbilityInfo(item.bundleName, item.abilityName, GetUserId());
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
    for (const auto &item : bundleInfos_) {
        for (const auto &info : item.second) {
            bool onlyInsertOne = false;
            for (auto infoItem : info.second.GetInnerBundleUserInfos()) {
                auto innerBundleUserInfo = infoItem.second;
                AddUserId(innerBundleUserInfo.bundleUserInfo.userId);
                if (!onlyInsertOne) {
                    onlyInsertOne = true;
                    int32_t bundleId = innerBundleUserInfo.uid -
                        innerBundleUserInfo.bundleUserInfo.userId * Constants::BASE_USER_RANGE;
                    std::lock_guard<std::mutex> lock(bundleIdMapMutex_);
                    auto infoItem = bundleIdMap_.find(bundleId);
                    if (infoItem == bundleIdMap_.end()) {
                        bundleIdMap_.emplace(bundleId, innerBundleUserInfo.bundleName);
                    } else {
                        bundleIdMap_[bundleId] = innerBundleUserInfo.bundleName;
                    }
                    BundleUtil::MakeHmdfsConfig(innerBundleUserInfo.bundleName, bundleId);
                }
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
    want.SetParam(Constants::USER_ID, GetUserIdByUid(uid));
    want.SetParam(Constants::ABILTY_NAME.data(), abilityName);
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

void BundleDataMgr::SetInitialUserFlag(bool flag)
{
    APP_LOGD("SetInitialUserFlag %{public}d", flag);
    if (!initialUserFlag_ && flag && bundlePromise_ != nullptr) {
        bundlePromise_->NotifyAllTasksExecuteFinished();
    }

    initialUserFlag_ = flag;
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

bool BundleDataMgr::NotifyAbilityLifeStatus(
    const std::string &bundleName, const std::string &abilityName, const int64_t launchTime, const int uid) const
{
    if (bundleName.empty() || abilityName.empty()) {
        return false;
    }
    std::lock_guard<std::mutex> lock(bundleInfoMutex_);
    if (bundleInfos_.empty()) {
        return false;
    }

    int userId = GetUserIdByUid(uid);
    std::string cloneBundleName = bundleName;
    for (auto it = bundleInfos_.begin(); it != bundleInfos_.end();) {
        if (it->first.find(cloneBundleName) != std::string::npos) {
            auto bundleInfo = it->second.find(Constants::CURRENT_DEVICE_ID);
            if (bundleInfo != it->second.end() && bundleInfo->second.GetUid(userId) == uid) {
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
    auto ability = bundleInfo->second.FindAbilityInfo(bundleName, abilityName, GetUserId(userId));
    if (!ability) {
        return false;
    }
    if (ability->applicationInfo.isCloned) {
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
    return keepUsage ? usageRecordStorage_->MarkUsageRecordRemoved(bundleInfo->second, GetUserId(userId))
                     : usageRecordStorage_->DeleteUsageRecord(bundleInfo->second, GetUserId(userId));
}

bool BundleDataMgr::GetShortcutInfos(
    const std::string &bundleName, int32_t userId, std::vector<ShortcutInfo> &shortcutInfos) const
{
    int32_t requestUserId = GetUserId(userId);
    if (requestUserId == Constants::INVALID_USERID) {
        return false;
    }

    std::lock_guard<std::mutex> lock(bundleInfoMutex_);
    InnerBundleInfo innerBundleInfo;
    if (!GetInnerBundleInfoWithFlags(
        bundleName, BundleFlag::GET_BUNDLE_DEFAULT, Constants::CURRENT_DEVICE_ID, innerBundleInfo, requestUserId)) {
        APP_LOGE("GetLaunchWantForBundle failed");
        return false;
    }
    innerBundleInfo.GetShortcutInfos(shortcutInfos);
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
    sptr<PermissionChangedDeathRecipient> deathRecipient = new (std::nothrow) PermissionChangedDeathRecipient();
    if (deathRecipient == nullptr) {
        APP_LOGE("create PermissionChangedDeathRecipient failed");
        return false;
    }
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
}

bool BundleDataMgr::SavePreInstallBundleInfo(
    const std::string &bundleName, const PreInstallBundleInfo &preInstallBundleInfo)
{
    std::lock_guard<std::mutex> lock(preInstallInfoMutex_);
    if (!preInstallDataStorage_) {
        return false;
    }

    if (preInstallDataStorage_->SavePreInstallStorageBundleInfo(
        Constants::PRE_INSTALL_DEVICE_ID, preInstallBundleInfo)) {
        auto info = std::find_if(
            preInstallBundleInfos_.begin(), preInstallBundleInfos_.end(), preInstallBundleInfo);
        if (info != preInstallBundleInfos_.end()) {
            *info = preInstallBundleInfo;
        } else {
            preInstallBundleInfos_.emplace_back(preInstallBundleInfo);
        }
        APP_LOGD("write storage success bundle:%{public}s", bundleName.c_str());
        return true;
    }

    return false;
}

bool BundleDataMgr::DeletePreInstallBundleInfo(
    const std::string &bundleName, const PreInstallBundleInfo &preInstallBundleInfo)
{
    std::lock_guard<std::mutex> lock(preInstallInfoMutex_);
    if (!preInstallDataStorage_) {
        return false;
    }

    if (preInstallDataStorage_->DeletePreInstallStorageBundleInfo(
        Constants::PRE_INSTALL_DEVICE_ID, preInstallBundleInfo)) {
        auto info = std::find_if(
            preInstallBundleInfos_.begin(), preInstallBundleInfos_.end(), preInstallBundleInfo);
        if (info != preInstallBundleInfos_.end()) {
            preInstallBundleInfos_.erase(info);
        }
        APP_LOGI("Delete PreInstall Storage success bundle:%{public}s", bundleName.c_str());
        return true;
    }

    return false;
}

bool BundleDataMgr::GetPreInstallBundleInfo(
    const std::string &bundleName, PreInstallBundleInfo &preInstallBundleInfo)
{
    std::lock_guard<std::mutex> lock(preInstallInfoMutex_);
    if (bundleName.empty()) {
        APP_LOGE("bundleName or deviceId empty");
        return false;
    }

    preInstallBundleInfo.SetBundleName(bundleName);
    auto info = std::find_if(
        preInstallBundleInfos_.begin(), preInstallBundleInfos_.end(), preInstallBundleInfo);
    if (info != preInstallBundleInfos_.end()) {
        preInstallBundleInfo = *info;
        return true;
    }

    APP_LOGE("get preInstall bundleInfo failed by bundle(%{public}s).", bundleName.c_str());
    return false;
}

bool BundleDataMgr::LoadAllPreInstallBundleInfos(
    std::vector<PreInstallBundleInfo> &preInstallBundleInfos)
{
    if (!preInstallDataStorage_) {
        return false;
    }

    if (preInstallDataStorage_->LoadAllPreInstallBundleInfos(preInstallBundleInfos)) {
        APP_LOGD("load all storage success");
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

bool BundleDataMgr::GetInnerBundleUserInfoByUserId(const std::string &bundleName,
    int32_t userId, InnerBundleUserInfo &innerBundleUserInfo) const
{
    APP_LOGD("get user info start: bundleName: (%{public}s)  userId: (%{public}d) ",
        bundleName.c_str(), userId);
    int32_t requestUserId = GetUserId(userId);
    if (requestUserId == Constants::INVALID_USERID) {
        return false;
    }

    if (bundleName.empty()) {
        APP_LOGW("bundle name is empty");
        return false;
    }

    std::lock_guard<std::mutex> lock(bundleInfoMutex_);
    if (bundleInfos_.empty()) {
        APP_LOGE("bundleInfos data is empty");
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

    return innerBundleInfo->second.GetInnerBundleUserInfo(requestUserId, innerBundleUserInfo);
}

int32_t BundleDataMgr::GetUserId(int32_t userId) const
{
    if (userId == Constants::ANY_USERID || userId == Constants::ALL_USERID) {
        return userId;
    }

    if (userId == Constants::UNSPECIFIED_USERID) {
        userId = GetUserIdByCallingUid();
    }

    if (!HasUserId(userId)) {
        APP_LOGE("user is not existed.");
        userId = Constants::INVALID_USERID;
    }

    return userId;
}

int32_t BundleDataMgr::GetUserIdByUid(int32_t uid) const
{
    return BundleUtil::GetUserIdByUid(uid);
}

void BundleDataMgr::AddUserId(int32_t userId)
{
    std::lock_guard<std::mutex> lock(multiUserIdSetMutex_);
    auto item = multiUserIdsSet_.find(userId);
    if (item != multiUserIdsSet_.end()) {
        return;
    }

    multiUserIdsSet_.insert(userId);
}

void BundleDataMgr::RemoveUserId(int32_t userId)
{
    std::lock_guard<std::mutex> lock(multiUserIdSetMutex_);
    auto item = multiUserIdsSet_.find(userId);
    if (item == multiUserIdsSet_.end()) {
        return;
    }

    multiUserIdsSet_.erase(item);
}

bool BundleDataMgr::HasUserId(int32_t userId) const
{
    std::lock_guard<std::mutex> lock(multiUserIdSetMutex_);
    return multiUserIdsSet_.find(userId) != multiUserIdsSet_.end();
}

int32_t BundleDataMgr::GetUserIdByCallingUid() const
{
    return BundleUtil::GetUserIdByCallingUid();
}

std::set<int32_t> BundleDataMgr::GetAllUser() const
{
    std::lock_guard<std::mutex> lock(multiUserIdSetMutex_);
    return multiUserIdsSet_;
}

bool BundleDataMgr::GetDistributedBundleInfo(
    const std::string &networkId, int32_t userId, const std::string &bundleName,
    DistributedBundleInfo &distributedBundleInfo)
{
    if (userId == Constants::INVALID_USERID) {
        return false;
    }
    return distributedDataStorage_->QueryStroageDistributeInfo(
        bundleName, userId, networkId, distributedBundleInfo);
}

bool BundleDataMgr::GetInnerBundleUserInfos(
    const std::string &bundleName, std::vector<InnerBundleUserInfo> &innerBundleUserInfos) const
{
    APP_LOGD("get all user info in bundle(%{public}s)", bundleName.c_str());
    if (bundleName.empty()) {
        APP_LOGW("bundle name is empty");
        return false;
    }

    std::lock_guard<std::mutex> lock(bundleInfoMutex_);
    if (bundleInfos_.empty()) {
        APP_LOGE("bundleInfos data is empty");
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

    for (auto userInfo : innerBundleInfo->second.GetInnerBundleUserInfos()) {
        innerBundleUserInfos.emplace_back(userInfo.second);
    }

    return !innerBundleUserInfos.empty();
}

std::string BundleDataMgr::GetAppPrivilegeLevel(const std::string &bundleName, int32_t userId)
{
    APP_LOGD("GetAppPrivilegeLevel:%{public}s, userId:%{public}d", bundleName.c_str(), userId);
    std::lock_guard<std::mutex> lock(bundleInfoMutex_);
    InnerBundleInfo info;
    if (!GetInnerBundleInfoWithFlags(bundleName, 0, Constants::CURRENT_DEVICE_ID, info, userId)) {
        return Constants::EMPTY_STRING;
    }

    return info.GetAppPrivilegeLevel();
}

bool BundleDataMgr::QueryExtensionAbilityInfos(const Want &want, int32_t flags, int32_t userId,
    std::vector<ExtensionAbilityInfo> &extensionInfos) const
{
    int32_t requestUserId = GetUserId(userId);
    if (requestUserId == Constants::INVALID_USERID) {
        return false;
    }

    ElementName element = want.GetElement();
    std::string bundleName = element.GetBundleName();
    std::string extensionName = element.GetAbilityName();
    APP_LOGD("bundle name:%{public}s, extension name:%{public}s", bundleName.c_str(), extensionName.c_str());
    // explicit query
    if (!bundleName.empty() && !extensionName.empty()) {
        ExtensionAbilityInfo info;
        bool ret = ExplicitQueryExtensionInfo(bundleName, extensionName, flags, requestUserId, info);
        if (!ret) {
            APP_LOGE("explicit queryExtensionInfo error");
            return false;
        }
        extensionInfos.emplace_back(info);
        return true;
    }

    bool ret = ImplicitQueryExtensionInfos(want, flags, requestUserId, extensionInfos);
    if (!ret) {
        APP_LOGE("implicit queryExtensionAbilityInfos error");
        return false;
    }
    if (extensionInfos.size() == 0) {
        APP_LOGE("no matching abilityInfo");
        return false;
    }
    APP_LOGD("query extensionAbilityInfo successfully");
    return true;
}

bool BundleDataMgr::ExplicitQueryExtensionInfo(const std::string &bundleName, const std::string &extensionName,
    int32_t flags, int32_t userId, ExtensionAbilityInfo &extensionInfo) const
{
    APP_LOGD("bundleName:%{public}s, abilityName:%{public}s", bundleName.c_str(), extensionName.c_str());
    APP_LOGD("flags:%{public}d, userId:%{public}d", flags, userId);
    int32_t requestUserId = GetUserId(userId);
    if (requestUserId == Constants::INVALID_USERID) {
        return false;
    }
    std::lock_guard<std::mutex> lock(bundleInfoMutex_);
    InnerBundleInfo innerBundleInfo;
    if (!GetInnerBundleInfoWithFlags(
        bundleName, flags, Constants::CURRENT_DEVICE_ID, innerBundleInfo, requestUserId)) {
        APP_LOGE("ExplicitQueryExtensionInfo failed");
        return false;
    }
    auto extension = innerBundleInfo.FindExtensionInfo(bundleName, extensionName);
    if (!extension) {
        APP_LOGE("extensionAbility not found or disabled");
        return false;
    }
    if ((static_cast<uint32_t>(flags) & GET_ABILITY_INFO_WITH_PERMISSION) != GET_ABILITY_INFO_WITH_PERMISSION) {
        extension->permissions.clear();
    }
    if ((static_cast<uint32_t>(flags) & GET_ABILITY_INFO_WITH_METADATA) != GET_ABILITY_INFO_WITH_METADATA) {
        extension->metadata.clear();
    }
    extensionInfo = (*extension);
    if ((static_cast<uint32_t>(flags) & GET_ABILITY_INFO_WITH_APPLICATION) == GET_ABILITY_INFO_WITH_APPLICATION) {
        int32_t responseUserId = innerBundleInfo.GetResponseUserId(requestUserId);
        innerBundleInfo.GetApplicationInfo(
            ApplicationFlag::GET_BASIC_APPLICATION_INFO, responseUserId, extensionInfo.applicationInfo);
    }
    return true;
}

bool BundleDataMgr::ImplicitQueryExtensionInfos(
    const Want &want, int32_t flags, int32_t userId, std::vector<ExtensionAbilityInfo> &extensionInfos) const
{
    if (want.GetAction().empty() && want.GetEntities().empty()
        && want.GetUriString().empty() && want.GetType().empty()) {
        APP_LOGE("param invalid");
        return false;
    }
    APP_LOGD("action:%{public}s, uri:%{private}s, type:%{public}s",
        want.GetAction().c_str(), want.GetUriString().c_str(), want.GetType().c_str());
    APP_LOGD("flags:%{public}d, userId:%{public}d", flags, userId);

    int32_t requestUserId = GetUserId(userId);
    if (requestUserId == Constants::INVALID_USERID) {
        return false;
    }
    std::string bundleName = want.GetElement().GetBundleName();
    // query at current bundle
    if (!bundleName.empty()) {
        std::lock_guard<std::mutex> lock(bundleInfoMutex_);
        InnerBundleInfo innerBundleInfo;
        if (!GetInnerBundleInfoWithFlags(
            bundleName, flags, Constants::CURRENT_DEVICE_ID, innerBundleInfo, requestUserId)) {
            APP_LOGE("ExplicitQueryExtensionAbilityInfo failed");
            return false;
        }
        int32_t responseUserId = innerBundleInfo.GetResponseUserId(requestUserId);
        GetMatchExtensionInfos(want, flags, responseUserId, innerBundleInfo, extensionInfos);
        return true;
    }

    // query all
    std::lock_guard<std::mutex> lock(bundleInfoMutex_);
    for (const auto &item : bundleInfos_) {
        InnerBundleInfo innerBundleInfo;
        if (!GetInnerBundleInfoWithFlags(
            item.first, flags, Constants::CURRENT_DEVICE_ID, innerBundleInfo, requestUserId)) {
            APP_LOGE("ImplicitQueryExtensionAbilityInfos failed");
            continue;
        }
        int32_t responseUserId = innerBundleInfo.GetResponseUserId(requestUserId);
        GetMatchExtensionInfos(want, flags, responseUserId, innerBundleInfo, extensionInfos);
    }
    return true;
}

void BundleDataMgr::GetMatchExtensionInfos(const Want &want, int32_t flags, const int32_t &userId,
    const InnerBundleInfo &info, std::vector<ExtensionAbilityInfo> &infos) const
{
    auto extensionSkillInfos = info.GetExtensionSkillInfos();
    auto extensionInfos = info.GetInnerExtensionInfos();
    for (const auto &skillInfos : extensionSkillInfos) {
        for (const auto &skill : skillInfos.second) {
            if (!skill.Match(want)) {
                continue;
            }
            if (extensionInfos.find(skillInfos.first) == extensionInfos.end()) {
                APP_LOGW("cannot find the extension info with %{public}s", skillInfos.first.c_str());
                break;
            }
            ExtensionAbilityInfo extensionInfo = extensionInfos[skillInfos.first];
            if ((static_cast<uint32_t>(flags) & GET_ABILITY_INFO_WITH_APPLICATION) ==
                GET_ABILITY_INFO_WITH_APPLICATION) {
                info.GetApplicationInfo(
                    ApplicationFlag::GET_BASIC_APPLICATION_INFO, userId, extensionInfo.applicationInfo);
            }
            if ((static_cast<uint32_t>(flags) & GET_ABILITY_INFO_WITH_PERMISSION) !=
                GET_ABILITY_INFO_WITH_PERMISSION) {
                extensionInfo.permissions.clear();
            }
            if ((static_cast<uint32_t>(flags) & GET_ABILITY_INFO_WITH_METADATA) != GET_ABILITY_INFO_WITH_METADATA) {
                extensionInfo.metadata.clear();
            }
            infos.emplace_back(extensionInfo);
            break;
        }
    }
}

bool BundleDataMgr::QueryExtensionAbilityInfos(const ExtensionAbilityType &extensionType, const int32_t &userId,
    std::vector<ExtensionAbilityInfo> &extensionInfos) const
{
    int32_t requestUserId = GetUserId(userId);
    if (requestUserId == Constants::INVALID_USERID) {
        return false;
    }
    std::lock_guard<std::mutex> lock(bundleInfoMutex_);
    for (const auto &item : bundleInfos_) {
        InnerBundleInfo innerBundleInfo;
        if (!GetInnerBundleInfoWithFlags(
            item.first, 0, Constants::CURRENT_DEVICE_ID, innerBundleInfo, requestUserId)) {
            APP_LOGE("QueryExtensionAbilityInfos failed");
            continue;
        }
        auto innerExtensionInfos = innerBundleInfo.GetInnerExtensionInfos();
        int32_t responseUserId = innerBundleInfo.GetResponseUserId(requestUserId);
        for (const auto &info : innerExtensionInfos) {
            if (info.second.type == extensionType) {
                ExtensionAbilityInfo extensionAbilityInfo = info.second;
                innerBundleInfo.GetApplicationInfo(
                    ApplicationFlag::GET_BASIC_APPLICATION_INFO, responseUserId, extensionAbilityInfo.applicationInfo);
                extensionInfos.emplace_back(extensionAbilityInfo);
            }
        }
    }
    return true;
}

std::vector<std::string> BundleDataMgr::GetAccessibleAppCodePaths(int32_t userId) const
{
    std::lock_guard<std::mutex> lock(bundleInfoMutex_);
    std::vector<std::string> vec;
    if (bundleInfos_.empty()) {
        APP_LOGE("bundleInfos_ is empty");
        return vec;
    }

    for (const auto &info : bundleInfos_) {
        auto item = info.second.find(Constants::CURRENT_DEVICE_ID);
        if (item == info.second.end()) {
            continue;
        }

        InnerBundleInfo innerBundleInfo = item->second;
        auto userInfoMap = innerBundleInfo.GetInnerBundleUserInfos();
        for (const auto &userInfo : userInfoMap) {
            auto innerUserId = userInfo.second.bundleUserInfo.userId;
            if (((innerUserId == 0) || (innerUserId == userId)) && innerBundleInfo.IsAccessible()) {
                vec.emplace_back(innerBundleInfo.GetAppCodePath());
            }
        }
    }
    return vec;
}

bool BundleDataMgr::QueryExtensionAbilityInfoByUri(const std::string &uri, int32_t userId,
    ExtensionAbilityInfo &extensionAbilityInfo) const
{
    int32_t requestUserId = GetUserId(userId);
    if (requestUserId == Constants::INVALID_USERID) {
        APP_LOGE("invalid userId -1");
        return false;
    }
    if (uri.empty()) {
        APP_LOGE("uri empty");
        return false;
    }
    // example of valid param uri : fileShare:///com.example.FileShare/person/10
    // example of convertUri : fileShare://com.example.FileShare
    size_t schemePos = uri.find(Constants::PARAM_URI_SEPARATOR);
    if (schemePos == uri.npos) {
        APP_LOGE("uri not include :///, invalid");
        return false;
    }
    size_t cutPos = uri.find(Constants::SEPARATOR, schemePos + Constants::PARAM_URI_SEPARATOR_LEN);
    if (cutPos == uri.npos) {
        APP_LOGE("uri not include /, invalid");
        return false;
    }
    // 1. cut string
    std::string convertUri = uri.substr(0, cutPos);
    // 2. replace :/// with ://
    convertUri.replace(schemePos, Constants::PARAM_URI_SEPARATOR_LEN,
        Constants::URI_SEPARATOR);
    APP_LOGD("convertUri : %{private}s", convertUri.c_str());

    std::lock_guard<std::mutex> lock(bundleInfoMutex_);
    if (bundleInfos_.empty()) {
        APP_LOGE("bundleInfos_ data is empty");
        return false;
    }
    std::string deviceId = Constants::CURRENT_DEVICE_ID;
    for (const auto &item : bundleInfos_) {
        auto infoWithIdItem = item.second.find(deviceId);
        if (infoWithIdItem == item.second.end()) {
            continue;
        }

        if (infoWithIdItem->second.IsDisabled()) {
            APP_LOGE("app %{public}s is disabled", infoWithIdItem->second.GetBundleName().c_str());
            continue;
        }

        int32_t responseUserId = infoWithIdItem->second.GetResponseUserId(requestUserId);
        if (!infoWithIdItem->second.GetApplicationEnabled(responseUserId)) {
            continue;
        }

        bool ret = infoWithIdItem->second.FindExtensionAbilityInfoByUri(convertUri, extensionAbilityInfo);
        if (!ret) {
            continue;
        }
        infoWithIdItem->second.GetApplicationInfo(
            ApplicationFlag::GET_BASIC_APPLICATION_INFO, responseUserId, extensionAbilityInfo.applicationInfo);
        return true;
    }
    APP_LOGE("QueryExtensionAbilityInfoByUri (%{private}s) failed.", uri.c_str());
    return false;
}

void BundleDataMgr::GetAllUriPrefix(std::vector<std::string> &uriPrefixList, int32_t userId,
    const std::string &excludeModule) const
{
    std::lock_guard<std::mutex> lock(bundleInfoMutex_);
    APP_LOGD("begin to GetAllUriPrefix, userId : %{public}d, excludeModule : %{public}s",
        userId, excludeModule.c_str());
    if (bundleInfos_.empty()) {
        APP_LOGE("bundleInfos_ is empty");
        return;
    }
    for (const auto &item : bundleInfos_) {
        auto infoWithIdItem = item.second.find(Constants::CURRENT_DEVICE_ID);
        if (infoWithIdItem == item.second.end()) {
            continue;
        }
        infoWithIdItem->second.GetUriPrefixList(uriPrefixList, userId, excludeModule);
        infoWithIdItem->second.GetUriPrefixList(uriPrefixList, Constants::DEFAULT_USERID, excludeModule);
    }
}

std::shared_ptr<Global::Resource::ResourceManager> BundleDataMgr::GetResourceManager(
    const AppExecFwk::BundleInfo &bundleInfo) const
{
    std::shared_ptr<Global::Resource::ResourceManager> resourceManager(Global::Resource::CreateResourceManager());
    for (auto moduleResPath : bundleInfo.moduleResPaths) {
        if (!moduleResPath.empty()) {
            APP_LOGE("DistributedBms::InitResourceManager, moduleResPath: %{private}s", moduleResPath.c_str());
            if (!resourceManager->AddResource(moduleResPath.c_str())) {
                APP_LOGE("DistributedBms::InitResourceManager AddResource failed");
            }
        }
    }

    std::unique_ptr<Global::Resource::ResConfig> resConfig(Global::Resource::CreateResConfig());
    resConfig->SetLocaleInfo("zh", "Hans", "CN");
    resourceManager->UpdateResConfig(*resConfig);
    return resourceManager;
}

bool BundleDataMgr::QueryAllDeviceIds(std::vector<std::string> &deviceIds)
{
    return distributedDataStorage_->QueryAllDeviceIds(deviceIds);
}

const std::vector<PreInstallBundleInfo>& BundleDataMgr::GetAllPreInstallBundleInfos()
{
    std::lock_guard<std::mutex> lock(preInstallInfoMutex_);
    return preInstallBundleInfos_;
}

#ifdef SUPPORT_GRAPHICS
std::shared_ptr<Media::PixelMap> BundleDataMgr::LoadImageFile(const std::string &path) const
{
    APP_LOGD("BundleDataMgr::LoadImageFile IN");
    uint32_t errorCode = 0;
    Media::SourceOptions opts;
    std::unique_ptr<Media::ImageSource> imageSource = Media::ImageSource::CreateImageSource(path,
                                                                                            opts,
                                                                                            errorCode);
    if (errorCode != 0) {
        APP_LOGE("Failed to create image source path %{private}s err %{public}d",
            path.c_str(), errorCode);
        return nullptr;
    }

    Media::DecodeOptions decodeOpts;
    auto pixelMapPtr = imageSource->CreatePixelMap(decodeOpts, errorCode);
    if (errorCode != 0) {
        APP_LOGE("Failed to create pixelmap path %{private}s err %{public}d",
            path.c_str(), errorCode);
        return nullptr;
    }
    APP_LOGD("BundleDataMgr::LoadImageFile OUT");
    return std::shared_ptr<Media::PixelMap>(std::move(pixelMapPtr));
}
#endif
}  // namespace AppExecFwk
}  // namespace OHOS
