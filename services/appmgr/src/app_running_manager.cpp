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

#include "datetime_ex.h"
#include "iremote_object.h"

#include "app_log_wrapper.h"
#include "appexecfwk_errors.h"
#include "perf_profile.h"

namespace OHOS {
namespace AppExecFwk {
namespace {
static const int32_t USER_UID_RANGE = 200000;

bool IsUidBelongsToUser(int32_t uid, int32_t userId)
{
    return (uid / USER_UID_RANGE) == userId;
}
}  // namespace

AppRunningManager::AppRunningManager()
{}
AppRunningManager::~AppRunningManager()
{}

std::shared_ptr<AppRunningRecord> AppRunningManager::CreateAppRunningRecord(
    const std::shared_ptr<ApplicationInfo> &appInfo, const std::string &processName, const BundleInfo &bundleInfo)
{
    std::lock_guard<std::recursive_mutex> guard(lock_);
    if (!appInfo) {
        APP_LOGE("param error");
        return nullptr;
    }

    if (processName.empty()) {
        APP_LOGE("processName error");
        return nullptr;
    }

    auto recordId = AppRecordId::Create();
    auto appRecord = std::make_shared<AppRunningRecord>(appInfo, recordId, processName);
    if (!appRecord) {
        return nullptr;
    }

    std::regex rule("[a-zA-Z.]+[-_#]{1}");
    std::string signCode;
    ClipStringContent(rule, bundleInfo.appId, signCode);

    APP_LOGI("Create processName : %{public}s | recordId : %{public}d | signCode : %{public}s",
        processName.c_str(), recordId, signCode.c_str());
    appRecord->SetSignCode(signCode);
    appRecord->SetJointUserId(bundleInfo.jointUserId);
    appRunningRecordMap_.emplace(recordId, appRecord);
    return appRecord;
}

std::shared_ptr<AppRunningRecord> AppRunningManager::CheckAppRunningRecordIsExist(const std::string &appName,
    const std::string &processName, const int uid, const BundleInfo &bundleInfo)
{
    APP_LOGI("CheckAppRunningRecordIsExist appName : %{public}s | processName : %{public}s | uid : %{public}d",
        appName.c_str(), processName.c_str(), uid);
    std::lock_guard<std::recursive_mutex> guard(lock_);

    std::regex rule("[a-zA-Z.]+[-_#]{1}");
    std::string signCode;
    auto jointUserId = bundleInfo.jointUserId;
    APP_LOGI("jointUserId : %{public}s", jointUserId.c_str());
    ClipStringContent(rule, bundleInfo.appId, signCode);

    auto FindSameProcess = [signCode, processName, jointUserId](const auto &pair) {
            return ((pair.second->GetSignCode() == signCode) &&
                    (pair.second->GetProcessName() == processName) &&
                    (pair.second->GetJointUserId() == jointUserId) &&
                    !(pair.second->IsTerminating()));
    };

    // If it is not empty, look for whether it can come in the same process
    if (jointUserId.empty()) {
        for (const auto &item : appRunningRecordMap_) {
            const auto &appRecord = item.second;
            APP_LOGI("appRecord->GetProcessName() : %{public}s", appRecord->GetProcessName().c_str());
            if (appRecord && appRecord->GetProcessName() == processName && !(appRecord->IsTerminating())) {
                auto appInfoList = appRecord->GetAppInfoList();
                APP_LOGI("appInfoList : %{public}zu", appInfoList.size());
                auto isExist = [&appName, &uid](const std::shared_ptr<ApplicationInfo> &appInfo) {
                    APP_LOGI("appInfo->name : %{public}s", appInfo->name.c_str());
                    return appInfo->name == appName && appInfo->uid == uid;
                };
                auto appInfoIter = std::find_if(appInfoList.begin(), appInfoList.end(), isExist);
                if (appInfoIter != appInfoList.end()) {
                    return appRecord;
                }
            }
        }
        return nullptr;
    }

    auto iter = std::find_if(appRunningRecordMap_.begin(), appRunningRecordMap_.end(), FindSameProcess);
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
            APP_LOGI("appRecord is exit");
            return appRecord;
        }
    }
    return nullptr;
}

bool AppRunningManager::ProcessExitByBundleName(const std::string &bundleName, std::list<pid_t> &pids)
{
    std::lock_guard<std::recursive_mutex> guard(lock_);
    for (const auto &item : appRunningRecordMap_) {
        const auto &appRecord = item.second;
        // condition [!appRecord->IsKeepAliveApp()] Is to not kill the resident process.
        // Before using this method, consider whether you need.
        if (appRecord && !appRecord->IsKeepAliveApp()) {
            pid_t pid = appRecord->GetPriorityObject()->GetPid();
            auto appInfoList = appRecord->GetAppInfoList();
            auto isExist = [&bundleName](const std::shared_ptr<ApplicationInfo> &appInfo) {
                return appInfo->bundleName == bundleName;
            };
            auto iter = std::find_if(appInfoList.begin(), appInfoList.end(), isExist);
            if (iter != appInfoList.end() && pid > 0) {
                pids.push_back(pid);
                appRecord->ScheduleProcessSecurityExit();
            }
        }
    }

    return !pids.empty();
}

bool AppRunningManager::GetPidsByUserId(int32_t userId, std::list<pid_t> &pids)
{
    std::lock_guard<std::recursive_mutex> guard(lock_);
    for (const auto &item : appRunningRecordMap_) {
        const auto &appRecord = item.second;
        if (appRecord && IsUidBelongsToUser(appRecord->GetUid(), userId)) {
            pid_t pid = appRecord->GetPriorityObject()->GetPid();
            if (pid > 0) {
                pids.push_back(pid);
                appRecord->ScheduleProcessSecurityExit();
            }
        }
    }

    return (pids.empty() ? false : true);
}

bool AppRunningManager::ProcessExitByBundleNameAndUid(
    const std::string &bundleName, const int uid, std::list<pid_t> &pids)
{
    std::lock_guard<std::recursive_mutex> guard(lock_);
    for (const auto &item : appRunningRecordMap_) {
        const auto &appRecord = item.second;
        if (appRecord) {
            auto appInfoList = appRecord->GetAppInfoList();
            auto isExist = [&bundleName, &uid](const std::shared_ptr<ApplicationInfo> &appInfo) {
                return appInfo->bundleName == bundleName && appInfo->uid == uid;
            };
            auto iter = std::find_if(appInfoList.begin(), appInfoList.end(), isExist);
            pid_t pid = appRecord->GetPriorityObject()->GetPid();
            if (iter != appInfoList.end() && pid > 0) {
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
        appRecord->SetApplicationClient(nullptr);
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
    APP_LOGI("Handle terminate timeout.");
    auto abilityRecord = GetAbilityRunningRecord(eventId);
    if (!abilityRecord) {
        APP_LOGE("abilityRecord is nullptr.");
        return;
    }
    auto abilityToken = abilityRecord->GetToken();
    auto appRecord = GetTerminatingAppRunningRecord(abilityToken);
    if (!appRecord) {
        APP_LOGE("appRecord is nullptr.");
        return;
    }
    appRecord->AbilityTerminated(abilityToken);
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
    APP_LOGI("Get ability running record by eventId.");
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
    APP_LOGI("Get app running record by eventId.");
    std::lock_guard<std::recursive_mutex> guard(lock_);
    auto iter = std::find_if(appRunningRecordMap_.begin(), appRunningRecordMap_.end(), [&eventId](const auto &pair) {
        return pair.second->GetEventId() == eventId;
    });
    return ((iter == appRunningRecordMap_.end()) ? nullptr : iter->second);
}

void AppRunningManager::HandleAbilityAttachTimeOut(const sptr<IRemoteObject> &token)
{
    APP_LOGI("Handle ability attach timeOut.");
    if (token == nullptr) {
        APP_LOGE("token is nullptr.");
        return;
    }

    auto appRecord = GetAppRunningRecordByAbilityToken(token);
    if (!appRecord) {
        APP_LOGE("appRecord is nullptr.");
        return;
    }

    std::shared_ptr<AbilityRunningRecord> abilityRecord = appRecord->GetAbilityRunningRecordByToken(token);
    if (abilityRecord) {
        abilityRecord->SetTerminating();
    }

    if (appRecord->IsLastAbilityRecord(token)) {
        appRecord->SetTerminating();
    }

    appRecord->TerminateAbility(token, true);
}

void AppRunningManager::PrepareTerminate(const sptr<IRemoteObject> &token)
{
    APP_LOGI("Prepare terminate.");
    if (token == nullptr) {
        APP_LOGE("token is nullptr.");
        return;
    }

    auto appRecord = GetAppRunningRecordByAbilityToken(token);
    if (!appRecord) {
        APP_LOGE("appRecord is nullptr.");
        return;
    }

    if (appRecord->IsLastAbilityRecord(token)) {
        appRecord->SetTerminating();
    }
}

void AppRunningManager::TerminateAbility(const sptr<IRemoteObject> &token)
{
    APP_LOGI("Terminate ability.");
    if (!token) {
        APP_LOGE("token is nullptr.");
        return;
    }

    auto appRecord = GetAppRunningRecordByAbilityToken(token);
    if (!appRecord) {
        APP_LOGE("appRecord is nullptr.");
        return;
    }

    if (appRecord->IsLastAbilityRecord(token) && !appRecord->IsKeepAliveApp()) {
        appRecord->SetTerminating();
    }

    appRecord->TerminateAbility(token, false);
}

void AppRunningManager::ClipStringContent(const std::regex &re, const std::string &sorce, std::string &afferCutStr)
{
    std::smatch basket;
    if (std::regex_search(sorce, basket, re)) {
        APP_LOGI("prefix str: [%{public}s]", basket.prefix().str().c_str());
        APP_LOGI("suffix str: [%{public}s]", basket.suffix().str().c_str());
        afferCutStr = basket.prefix().str() + basket.suffix().str();
    }
}

void AppRunningManager::GetForegroundApplications(std::vector<AppStateData> &list)
{
    APP_LOGI("%{public}s, begin.", __func__);
    std::lock_guard<std::recursive_mutex> guard(lock_);
    for (const auto &item : appRunningRecordMap_) {
        const auto &appRecord = item.second;
        if (appRecord && appRecord->GetState() == ApplicationState::APP_STATE_FOREGROUND) {
            AppStateData appData;
            appData.bundleName = appRecord->GetBundleName();
            appData.uid = appRecord->GetUid();
            appData.state = static_cast<int32_t>(ApplicationState::APP_STATE_FOREGROUND);
            list.push_back(appData);
            APP_LOGI("%{public}s, bundleName:%{public}s", __func__, appData.bundleName.c_str());
        }
    }
}
}  // namespace AppExecFwk
}  // namespace OHOS