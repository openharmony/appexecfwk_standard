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

#include "app_running_manager.h"

#include "iremote_object.h"
#include "datetime_ex.h"

#include "app_log_wrapper.h"
#include "perf_profile.h"
#include "appexecfwk_errors.h"

namespace OHOS {
namespace AppExecFwk {
namespace {
bool CheckUid(const int32_t uid)
{
    return uid >= 0 && uid < std::numeric_limits<int32_t>::max();
}

const int RESTART_RESIDENT_PROCESS_MAX_TIMES = 15;
}  // namespace

AppRunningManager::AppRunningManager()
{}
AppRunningManager::~AppRunningManager()
{}

std::shared_ptr<AppRunningRecord> AppRunningManager::GetOrCreateAppRunningRecord(const sptr<IRemoteObject> &token,
    const std::shared_ptr<ApplicationInfo> &appInfo, const std::shared_ptr<AbilityInfo> &abilityInfo,
    const std::string &processName, const int32_t uid, RecordQueryResult &result)
{
    APP_LOGI("GetOrCreateAppRunningRecord processName : %{public}s | uid : %{public}d",
        processName.c_str(), uid);

    std::lock_guard<std::recursive_mutex> guard(lock_);
    result.Reset();
    if (!token || !appInfo || !abilityInfo) {
        APP_LOGE("param error");
        result.error = ERR_INVALID_VALUE;
        return nullptr;
    }
    if (!CheckUid(uid)) {
        APP_LOGE("uid invalid");
        result.error = ERR_APPEXECFWK_INVALID_UID;
        return nullptr;
    }
    if (processName.empty()) {
        APP_LOGE("processName error");
        result.error = ERR_INVALID_VALUE;
        return nullptr;
    }

    auto record = GetAppRunningRecordByProcessName(appInfo->name, processName, appInfo->uid);
    if (!record) {
        APP_LOGI("no app record, create");
        auto recordId = AppRecordId::Create();
        record = std::make_shared<AppRunningRecord>(appInfo, recordId, processName);
        appRunningRecordMap_.emplace(recordId, record);
    } else {
        result.appExists = true;
    }

    result.appRecordId = record->GetRecordId();
    auto abilityRecord = record->GetAbilityRunningRecordByToken(token);
    result.abilityExists = !!abilityRecord;
    if (!abilityRecord) {
        APP_LOGI("no ability record, create");
        abilityRecord = record->AddAbility(token, abilityInfo);
    }
    return record;
}

std::shared_ptr<AppRunningRecord> AppRunningManager::GetOrCreateAppRunningRecord(
    const ApplicationInfo &appInfo, bool &appExist)
{
    if (appInfo.bundleName.empty() || !CheckUid(appInfo.uid)) {
        APP_LOGE("processName error");
        return nullptr;
    }

    auto processName = appInfo.process.empty() ? appInfo.bundleName : appInfo.process;
    APP_LOGI("processName = [%{public}s]", processName.c_str());
    auto record = GetAppRunningRecordByProcessName(appInfo.name, processName, appInfo.uid);
    if (!record) {
        APP_LOGI("no app record, create");
        auto recordId = AppRecordId::Create();
        auto app = std::make_shared<ApplicationInfo>(appInfo);
        APP_LOGI("app is create : [%{public}d]", app != nullptr);
        record = std::make_shared<AppRunningRecord>(app, recordId, processName);
        record->SetUid(appInfo.uid);
        appRunningRecordMap_.emplace(recordId, record);
        appExist = false;
    } else {
        appExist = true;
    }
    return record;
}

std::shared_ptr<AppRunningRecord> AppRunningManager::GetAppRunningRecordByAppName(const std::string &appName)
{
    std::lock_guard<std::recursive_mutex> guard(lock_);
    auto iter = std::find_if(appRunningRecordMap_.begin(), appRunningRecordMap_.end(), [&appName](const auto &pair) {
        return pair.second->GetName() == appName;
    });
    return ((iter == appRunningRecordMap_.end()) ? nullptr : iter->second);
}

std::shared_ptr<AppRunningRecord> AppRunningManager::GetAppRunningRecordByProcessName(
    const std::string &appName, const std::string &processName, const int uid)
{
    APP_LOGI("GetAppRunningRecordByProcessName appName : %{public}s | processName : %{public}s | uid : %{public}d",
        appName.c_str(), processName.c_str(), uid);
    std::lock_guard<std::recursive_mutex> guard(lock_);
    auto iter = std::find_if(
        appRunningRecordMap_.begin(), appRunningRecordMap_.end(), [&appName, &processName, &uid](const auto &pair) {
            return ((pair.second->GetName() == appName) && (pair.second->GetProcessName() == processName) &&
                    (pair.second->GetUid() == uid) && !(pair.second->IsTerminating()));
        });
    APP_LOGI("find result : %{public}d", iter != appRunningRecordMap_.end());
    return ((iter == appRunningRecordMap_.end()) ? nullptr : iter->second);
}

std::shared_ptr<AppRunningRecord> AppRunningManager::GetAppRunningRecordByPid(const pid_t pid)
{
    std::lock_guard<std::recursive_mutex> guard(lock_);
    auto iter = std::find_if(appRunningRecordMap_.begin(), appRunningRecordMap_.end(), [&pid](const auto &pair) {
        return pair.second->GetPriorityObject()->GetPid() == pid;
    });
    return ((iter == appRunningRecordMap_.end()) ? nullptr : iter->second);
}

std::shared_ptr<AppRunningRecord> AppRunningManager::GetAppRunningRecordByAbilityToken(
    const sptr<IRemoteObject> &abilityToken)
{
    std::lock_guard<std::recursive_mutex> guard(lock_);
    for (const auto &item : appRunningRecordMap_) {
        const auto &appRecord = item.second;
        if (appRecord && appRecord->GetAbilityRunningRecordByToken(abilityToken)) {
            return appRecord;
        }
    }
    return nullptr;
}

bool AppRunningManager::GetPidsByBundleName(const std::string &bundleName, std::list<pid_t> &pids)
{
    std::lock_guard<std::recursive_mutex> guard(lock_);
    for (const auto &item : appRunningRecordMap_) {
        const auto &appRecord = item.second;
        if (appRecord && appRecord->GetBundleName() == bundleName && appRecord->GetCloneInfo() == false) {
            pid_t pid = appRecord->GetPriorityObject()->GetPid();
            if (pid > 0) {
                pids.push_back(pid);
                appRecord->ScheduleProcessSecurityExit();
            }
        }
    }

    return (pids.empty() ? false : true);
}

bool AppRunningManager::GetPidsByBundleNameByUid(const std::string &bundleName, const int uid, std::list<pid_t> &pids)
{
    std::lock_guard<std::recursive_mutex> guard(lock_);
    for (const auto &item : appRunningRecordMap_) {
        const auto &appRecord = item.second;
        if (appRecord && appRecord->GetBundleName() == bundleName && appRecord->GetUid() == uid) {
            pid_t pid = appRecord->GetPriorityObject()->GetPid();
            if (pid > 0) {
                pids.push_back(pid);
                appRecord->ScheduleProcessSecurityExit();
            }
        }
    }

    return (pids.empty() ? false : true);
}

std::shared_ptr<AppRunningRecord> AppRunningManager::OnRemoteDied(const wptr<IRemoteObject> &remote)
{
    std::lock_guard<std::recursive_mutex> guard(lock_);
    if (remote == nullptr) {
        APP_LOGE("remote is null");
        return nullptr;
    }
    sptr<IRemoteObject> object = remote.promote();
    if (!object) {
        APP_LOGE("object is null");
        return nullptr;
    }
    const auto &iter =
        std::find_if(appRunningRecordMap_.begin(), appRunningRecordMap_.end(), [&object](const auto &pair) {
            if (pair.second && pair.second->GetApplicationClient() != nullptr) {
                return pair.second->GetApplicationClient()->AsObject() == object;
            }
            return false;
        });
    if (iter != appRunningRecordMap_.end()) {
        auto appRecord = iter->second;
        appRunningRecordMap_.erase(iter);
        if (appRecord) {
            return appRecord;
        }
    }
    return nullptr;
}

const std::map<const int32_t, const std::shared_ptr<AppRunningRecord>> &AppRunningManager::GetAppRunningRecordMap()
{
    std::lock_guard<std::recursive_mutex> guard(lock_);
    return appRunningRecordMap_;
}

void AppRunningManager::RemoveAppRunningRecordById(const int32_t recordId)
{
    std::lock_guard<std::recursive_mutex> guard(lock_);
    appRunningRecordMap_.erase(recordId);
}

void AppRunningManager::ClearAppRunningRecordMap()
{
    std::lock_guard<std::recursive_mutex> guard(lock_);
    appRunningRecordMap_.clear();
}

void AppRunningManager::HandleTerminateTimeOut(int64_t eventId)
{
    APP_LOGI("%{public}s, called", __func__);
    auto abilityRecord = GetAbilityRunningRecord(eventId);
    if (!abilityRecord) {
        APP_LOGE("%{public}s, abilityRecord is nullptr", __func__);
        return;
    }
    auto abilityToken = abilityRecord->GetToken();
    auto appRecord = GetTerminatingAppRunningRecord(abilityToken);
    if (!appRecord) {
        APP_LOGE("%{public}s, appRecord is nullptr", __func__);
        return;
    }
    appRecord->AbilityTerminated(abilityToken);
    APP_LOGI("%{public}s, end", __func__);
}

std::shared_ptr<AppRunningRecord> AppRunningManager::GetTerminatingAppRunningRecord(
    const sptr<IRemoteObject> &abilityToken)
{
    std::lock_guard<std::recursive_mutex> guard(lock_);
    for (const auto &item : appRunningRecordMap_) {
        const auto &appRecord = item.second;
        if (appRecord && appRecord->GetAbilityByTerminateLists(abilityToken)) {
            return appRecord;
        }
    }
    return nullptr;
}

std::shared_ptr<AbilityRunningRecord> AppRunningManager::GetAbilityRunningRecord(const int64_t eventId)
{
    APP_LOGI("%{public}s, called", __func__);
    std::lock_guard<std::recursive_mutex> guard(lock_);
    for (auto &item : appRunningRecordMap_) {
        if (item.second) {
            auto abilityRecord = item.second->GetAbilityRunningRecord(eventId);
            if (abilityRecord) {
                return abilityRecord;
            }
        }
    }
    return nullptr;
}

std::shared_ptr<AppRunningRecord> AppRunningManager::GetAppRunningRecord(const int64_t eventId)
{
    APP_LOGI("%{public}s, called", __func__);
    std::lock_guard<std::recursive_mutex> guard(lock_);
    auto iter = std::find_if(appRunningRecordMap_.begin(), appRunningRecordMap_.end(), [&eventId](const auto &pair) {
        return pair.second->GetEventId() == eventId;
    });
    return ((iter == appRunningRecordMap_.end()) ? nullptr : iter->second);
}

void AppRunningManager::HandleAbilityAttachTimeOut(const sptr<IRemoteObject> &token)
{
    APP_LOGI("%{public}s, called", __func__);
    if (token == nullptr) {
        APP_LOGE("%{public}s, token is nullptr", __func__);
        return;
    }

    auto appRecord = GetAppRunningRecordByAbilityToken(token);
    if (!appRecord) {
        APP_LOGE("%{public}s, appRecord is nullptr", __func__);
        return;
    }

    if (appRecord->IsLastAbilityRecord(token)) {
        appRecord->SetTerminating();
    }

    appRecord->TerminateAbility(token, true);
}

void AppRunningManager::PrepareTerminate(const sptr<IRemoteObject> &token)
{
    APP_LOGI("%{public}s, called", __func__);
    if (token == nullptr) {
        APP_LOGE("%{public}s, token is nullptr", __func__);
        return;
    }

    auto appRecord = GetAppRunningRecordByAbilityToken(token);
    if (!appRecord) {
        APP_LOGE("%{public}s, appRecord is nullptr", __func__);
        return;
    }

    if (appRecord->IsLastAbilityRecord(token)) {
        appRecord->SetTerminating();
    }
}

void AppRunningManager::TerminateAbility(const sptr<IRemoteObject> &token)
{
    APP_LOGI("%{public}s, called", __func__);
    if (!token) {
        APP_LOGE("%{public}s, token is nullptr", __func__);
        return;
    }

    auto appRecord = GetAppRunningRecordByAbilityToken(token);
    if (!appRecord) {
        APP_LOGE("%{public}s, appRecord is nullptr", __func__);
        return;
    }

    if (appRecord->IsLastAbilityRecord(token) && !appRecord->IsKeepAliveApp()) {
        appRecord->SetTerminating();
    }

    appRecord->TerminateAbility(token, false);
}

bool AppRunningManager::CanRestartResidentProcCount(const std::string &processName)
{
    if (processName.empty()) {
        APP_LOGE("processName is empty!");
        return false;
    }

    auto iter = processRestartRecord_.find(processName);
    if (iter != processRestartRecord_.end()) {
        if(iter->second > 0) {
            APP_LOGI("restart count processName : [%{public}s] | num : [%{public}d]",
                processName.c_str(), iter->second);
            processRestartRecord_[processName] = --(iter->second);
            return true;
        }
    }
    return false;
}

bool AppRunningManager::InitRestartResidentProcRecord(const std::string &processName)
{
    if (processName.empty()) {
        APP_LOGE("processName is empty!");
        return false;
    }

    // Not counting the number of connection attempts of app spwan,
    // the number of restart attempts of a resident process is set to 15
    auto pair = processRestartRecord_.insert(std::make_pair(processName, RESTART_RESIDENT_PROCESS_MAX_TIMES));
    return pair.second;
}

std::shared_ptr<AppRunningRecord> AppRunningManager::GetAppRunningRecordByBundleName(const std::string &bundleName)
{
    APP_LOGI("%{public}s, kill bundle : [%{public}s]", __func__, bundleName.c_str());
    std::lock_guard<std::recursive_mutex> guard(lock_);
    for (const auto &item : appRunningRecordMap_) {
        const auto &appRecord = item.second;
        if (appRecord && appRecord->GetBundleName() == bundleName) {
            return appRecord;
        }
    }
    return nullptr;
}
}  // namespace AppExecFwk
}  // namespace OHOS