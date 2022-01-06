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

#include "app_mgr_service_inner.h"

#include <csignal>
#include <securec.h>
#include <sys/stat.h>

#include "app_log_wrapper.h"
#include "application_state_observer_stub.h"
#include "datetime_ex.h"
#include "perf_profile.h"

#include "app_process_data.h"
#include "bundle_constants.h"
#include "bytrace.h"
#include "common_event.h"
#include "common_event_manager.h"
#include "common_event_support.h"
#include "iremote_object.h"
#include "iservice_registry.h"
#include "permission/permission_kit.h"
#include "system_ability_definition.h"

namespace OHOS {
namespace AppExecFwk {
using namespace OHOS::Security;

namespace {
// NANOSECONDS mean 10^9 nano second
constexpr int64_t NANOSECONDS = 1000000000;
// MICROSECONDS mean 10^6 millias second
constexpr int64_t MICROSECONDS = 1000000;
// Kill process timeout setting
constexpr int KILL_PROCESS_TIMEOUT_MICRO_SECONDS = 1000;
// Kill process delaytime setting
constexpr int KILL_PROCESS_DELAYTIME_MICRO_SECONDS = 200;
const std::string CLASS_NAME = "ohos.app.MainThread";
const std::string FUNC_NAME = "main";
const std::string SO_PATH = "system/lib64/libmapleappkit.z.so";
const int32_t SIGNAL_KILL = 9;
const std::string REQ_PERMISSION = "ohos.permission.LOCATION_IN_BACKGROUND";
constexpr int32_t SYSTEM_UID = 1000;
#define ENUM_TO_STRING(s) #s
}  // namespace

using OHOS::AppExecFwk::Constants::PERMISSION_GRANTED;
using OHOS::AppExecFwk::Constants::PERMISSION_NOT_GRANTED;
using OHOS::AppExecFwk::Constants::ROOT_UID;

AppMgrServiceInner::AppMgrServiceInner()
    : appProcessManager_(std::make_shared<AppProcessManager>()),
      remoteClientManager_(std::make_shared<RemoteClientManager>()),
      appRunningManager_(std::make_shared<AppRunningManager>())
{}

AppMgrServiceInner::~AppMgrServiceInner()
{}

void AppMgrServiceInner::LoadAbility(const sptr<IRemoteObject> &token, const sptr<IRemoteObject> &preToken,
    const std::shared_ptr<AbilityInfo> &abilityInfo, const std::shared_ptr<ApplicationInfo> &appInfo)
{
    BYTRACE(BYTRACE_TAG_APP);
    if (!token || !abilityInfo || !appInfo) {
        APP_LOGE("param error");
        return;
    }
    if (abilityInfo->name.empty() || appInfo->name.empty()) {
        APP_LOGE("error abilityInfo or appInfo");
        return;
    }
    if (abilityInfo->applicationName != appInfo->name) {
        APP_LOGE("abilityInfo and appInfo have different appName, don't load for it");
        return;
    }

    auto processName = abilityInfo->process.empty() ? appInfo->bundleName : abilityInfo->process;
    APP_LOGI("processName = [%{public}s]", processName.c_str());

    auto appRecord = GetAppRunningRecordByProcessName(appInfo->name, processName, appInfo->uid);
    if (!appRecord) {
        RecordQueryResult result;
        int32_t defaultUid = 0;
        appRecord = GetOrCreateAppRunningRecord(token, appInfo, abilityInfo, processName, defaultUid, result);
        if (FAILED(result.error)) {
            APP_LOGE("create appRunningRecord failed");
            return;
        }
        appRecord->SetEventHandler(eventHandler_);
        if (preToken != nullptr) {
            auto abilityRecord = appRecord->GetAbilityRunningRecordByToken(token);
            abilityRecord->SetPreToken(preToken);
        }
        APP_LOGI("LoadAbility StartProcess appname [%{public}s] | bundename [%{public}s]", appRecord->GetName().c_str(),
            appRecord->GetBundleName().c_str());
        StartProcess(abilityInfo->applicationName, processName, appRecord, abilityInfo->applicationInfo.uid);
    } else {
        APP_LOGI("LoadAbility StartAbility appname [%{public}s] | bundename [%{public}s]", appRecord->GetName().c_str(),
            appRecord->GetBundleName().c_str());
        StartAbility(token, preToken, abilityInfo, appRecord);
    }
    PerfProfile::GetInstance().SetAbilityLoadEndTime(GetTickCount());
    PerfProfile::GetInstance().Dump();
    PerfProfile::GetInstance().Reset();
}

void AppMgrServiceInner::AttachApplication(const pid_t pid, const sptr<IAppScheduler> &app)
{
    BYTRACE(BYTRACE_TAG_APP);
    if (pid <= 0) {
        APP_LOGE("invalid pid:%{public}d", pid);
        return;
    }
    if (!app) {
        APP_LOGE("app client is null");
        return;
    }
    APP_LOGI("attach application pid:%{public}d", pid);
    auto appRecord = GetAppRunningRecordByPid(pid);
    if (!appRecord) {
        APP_LOGE("no such appRecord");
        return;
    }
    appRecord->SetApplicationClient(app);
    if (appRecord->GetState() == ApplicationState::APP_STATE_CREATE) {
        LaunchApplication(appRecord);
    }
    appRecord->RegisterAppDeathRecipient();
}

void AppMgrServiceInner::LaunchApplication(const std::shared_ptr<AppRunningRecord> &appRecord)
{
    if (!appRecord) {
        APP_LOGE("appRecord is null");
        return;
    }
    if (appRecord->GetState() != ApplicationState::APP_STATE_CREATE) {
        APP_LOGE("wrong app state");
        return;
    }
    appRecord->LaunchApplication();
    appRecord->SetState(ApplicationState::APP_STATE_READY);
    OptimizerAppStateChanged(appRecord, ApplicationState::APP_STATE_CREATE);

    // There is no ability when the resident process starts
    // The status of all resident processes is ready
    // There is no process of switching the foreground, waiting for his first ability to start
    if (appRecord->IsKeepAliveApp()) {
        appRecord->AddAbilityStage();
        return ;
    }
    appRecord->LaunchPendingAbilities();
}

void AppMgrServiceInner::AddAbilityStageDone(const int32_t recordId)
{
    auto appRecord = GetAppRunningRecordByAppRecordId(recordId);
    if (!appRecord) {
        APP_LOGE("get app record failed");
        return;
    }
    appRecord->AddAbilityStageDone();
}

void AppMgrServiceInner::ApplicationForegrounded(const int32_t recordId)
{
    BYTRACE(BYTRACE_TAG_APP);
    auto appRecord = GetAppRunningRecordByAppRecordId(recordId);
    if (!appRecord) {
        APP_LOGE("get app record failed");
        return;
    }
    appRecord->PopForegroundingAbilityTokens();
    ApplicationState appState = appRecord->GetState();
    if (appState == ApplicationState::APP_STATE_READY || appState == ApplicationState::APP_STATE_BACKGROUND) {
        appRecord->SetState(ApplicationState::APP_STATE_FOREGROUND);
        OptimizerAppStateChanged(appRecord, appState);
        OnAppStateChanged(appRecord, ApplicationState::APP_STATE_FOREGROUND);
    } else if (appState == ApplicationState::APP_STATE_SUSPENDED) {
        appRecord->SetState(ApplicationState::APP_STATE_BACKGROUND);
        OptimizerAppStateChanged(appRecord, ApplicationState::APP_STATE_SUSPENDED);
    } else {
        APP_LOGW("app name(%{public}s), app state(%{public}d)!",
            appRecord->GetName().c_str(),
            static_cast<ApplicationState>(appState));
    }

    // push the foregrounded app front of RecentAppList.
    PushAppFront(recordId);
    APP_LOGI("application is foregrounded");
}

void AppMgrServiceInner::ApplicationBackgrounded(const int32_t recordId)
{
    BYTRACE(BYTRACE_TAG_APP);
    auto appRecord = GetAppRunningRecordByAppRecordId(recordId);
    if (!appRecord) {
        APP_LOGE("get app record failed");
        return;
    }
    if (appRecord->GetState() == ApplicationState::APP_STATE_FOREGROUND) {
        appRecord->SetState(ApplicationState::APP_STATE_BACKGROUND);
        OptimizerAppStateChanged(appRecord, ApplicationState::APP_STATE_FOREGROUND);
        OnAppStateChanged(appRecord, ApplicationState::APP_STATE_BACKGROUND);
    } else if (appRecord->GetState() == ApplicationState::APP_STATE_SUSPENDED) {
        appRecord->SetState(ApplicationState::APP_STATE_BACKGROUND);
        OptimizerAppStateChanged(appRecord, ApplicationState::APP_STATE_SUSPENDED);
    } else {
        APP_LOGW("app name(%{public}s), app state(%{public}d)!",
            appRecord->GetName().c_str(),
            static_cast<ApplicationState>(appRecord->GetState()));
    }

    APP_LOGI("application is backgrounded");
}

void AppMgrServiceInner::ApplicationTerminated(const int32_t recordId)
{
    BYTRACE(BYTRACE_TAG_APP);
    auto appRecord = GetAppRunningRecordByAppRecordId(recordId);
    if (!appRecord) {
        APP_LOGE("get app record failed");
        return;
    }
    if (appRecord->GetState() != ApplicationState::APP_STATE_BACKGROUND) {
        APP_LOGE("current state is not background");
        return;
    }
    appRecord->SetState(ApplicationState::APP_STATE_TERMINATED);
    OptimizerAppStateChanged(appRecord, ApplicationState::APP_STATE_BACKGROUND);
    appRecord->RemoveAppDeathRecipient();
    OnAppStateChanged(appRecord, ApplicationState::APP_STATE_TERMINATED);
    appRunningManager_->RemoveAppRunningRecordById(recordId);
    RemoveAppFromRecentListById(recordId);
    OnProcessDied(appRecord);

    APP_LOGI("application is terminated");
}

int32_t AppMgrServiceInner::KillApplication(const std::string &bundleName)
{
    if (!appRunningManager_) {
        APP_LOGE("appRunningManager_ is nullptr");
        return ERR_NO_INIT;
    }

    // All means can not kill the resident process
    auto appRecord = appRunningManager_->GetAppRunningRecordByBundleName(bundleName);
    if (appRecord && appRecord->IsKeepAliveApp()) {
        return ERR_INVALID_VALUE;
    }

    int result = ERR_OK;
    int64_t startTime = SystemTimeMillis();
    std::list<pid_t> pids;
    if (!appRunningManager_->GetPidsByBundleName(bundleName, pids)) {
        APP_LOGI("The process corresponding to the package name did not start");
        return result;
    }
    if (WaitForRemoteProcessExit(pids, startTime)) {
        APP_LOGI("The remote process exited successfully ");
        NotifyAppStatus(bundleName, EventFwk::CommonEventSupport::COMMON_EVENT_PACKAGE_RESTARTED);
        return result;
    }
    for (auto iter = pids.begin(); iter != pids.end(); ++iter) {
        result = KillProcessByPid(*iter);
        if (result < 0) {
            APP_LOGE("KillApplication is fail bundleName: %{public}s pid: %{public}d", bundleName.c_str(), *iter);
            return result;
        }
    }
    NotifyAppStatus(bundleName, EventFwk::CommonEventSupport::COMMON_EVENT_PACKAGE_RESTARTED);
    return result;
}

int32_t AppMgrServiceInner::KillApplicationByUid(const std::string &bundleName, const int uid)
{
    if (!appRunningManager_) {
        APP_LOGE("appRunningManager_ is nullptr");
        return ERR_NO_INIT;
    }
    int result = ERR_OK;
    int64_t startTime = SystemTimeMillis();
    std::list<pid_t> pids;
    if (remoteClientManager_ == nullptr) {
        APP_LOGE("remoteClientManager_ fail");
        return ERR_NO_INIT;
    }
    auto bundleMgr_ = remoteClientManager_->GetBundleManager();
    if (bundleMgr_ == nullptr) {
        APP_LOGE("GetBundleManager fail");
        return ERR_NO_INIT;
    }
    APP_LOGI("uid value is %{public}d", uid);
    if (!appRunningManager_->GetPidsByBundleNameByUid(bundleName, uid, pids)) {
        APP_LOGI("The process corresponding to the package name did not start");
        return result;
    }
    if (WaitForRemoteProcessExit(pids, startTime)) {
        APP_LOGI("The remote process exited successfully ");
        return result;
    }
    for (auto iter = pids.begin(); iter != pids.end(); ++iter) {
        result = KillProcessByPid(*iter);
        if (result < 0) {
            APP_LOGE("KillApplication is fail bundleName: %{public}s pid: %{public}d", bundleName.c_str(), *iter);
            return result;
        }
    }
    return result;
}

int32_t AppMgrServiceInner::KillApplicationByUserId(const std::string &bundleName, const int userId)
{
    if (!appRunningManager_) {
        APP_LOGE("appRunningManager_ is nullptr");
        return ERR_NO_INIT;
    }
    int result = ERR_OK;
    int64_t startTime = SystemTimeMillis();
    std::list<pid_t> pids;
    if (remoteClientManager_ == nullptr) {
        APP_LOGE("remoteClientManager_ fail");
        return ERR_NO_INIT;
    }
    auto bundleMgr_ = remoteClientManager_->GetBundleManager();
    if (bundleMgr_ == nullptr) {
        APP_LOGE("GetBundleManager fail");
        return ERR_NO_INIT;
    }
    APP_LOGI("userId value is %{public}d", userId);
    int uid = bundleMgr_->GetUidByBundleName(bundleName, userId);
    APP_LOGI("uid value is %{public}d", uid);
    if (!appRunningManager_->GetPidsByBundleNameByUid(bundleName, uid, pids)) {
        APP_LOGI("The process corresponding to the package name did not start");
        return result;
    }
    if (WaitForRemoteProcessExit(pids, startTime)) {
        APP_LOGI("The remote process exited successfully ");
        return result;
    }
    for (auto iter = pids.begin(); iter != pids.end(); ++iter) {
        result = KillProcessByPid(*iter);
        if (result < 0) {
            APP_LOGE("KillApplication is fail bundleName: %{public}s pid: %{public}d", bundleName.c_str(), *iter);
            return result;
        }
    }
    return result;
}

void AppMgrServiceInner::ClearUpApplicationData(const std::string &bundleName, int32_t callerUid, pid_t callerPid)
{
    BYTRACE(BYTRACE_TAG_APP);
    ClearUpApplicationDataByUserId(bundleName, callerUid, callerPid, Constants::DEFAULT_USERID);
}

void AppMgrServiceInner::ClearUpApplicationDataByUserId(const std::string &bundleName,
    int32_t callerUid, pid_t callerPid, const int userId)
{
    if (callerPid <= 0) {
        APP_LOGE("invalid callerPid:%{public}d", callerPid);
        return;
    }
    if (callerUid <= 0) {
        APP_LOGE("invalid callerUid:%{public}d", callerUid);
        return;
    }
    auto bundleMgr_ = remoteClientManager_->GetBundleManager();
    if (bundleMgr_ == nullptr) {
        APP_LOGE("GetBundleManager fail");
        return;
    }

    int32_t clearUid = bundleMgr_->GetUidByBundleName(bundleName, userId);
    if (bundleMgr_->CheckIsSystemAppByUid(callerUid) || callerUid == clearUid) {
        // request to clear user information permission.
        int32_t result =
            Permission::PermissionKit::RemoveUserGrantedReqPermissions(bundleName, userId);
        if (result) {
            APP_LOGE("RemoveUserGrantedReqPermissions failed");
            return;
        }
        // 2.delete bundle side user data
        if (!bundleMgr_->CleanBundleDataFiles(bundleName, userId)) {
            APP_LOGE("Delete bundle side user data is fail");
            return;
        }
        // 3.kill application
        // 4.revoke user rights
        result = KillApplicationByUserId(bundleName, userId);
        if (result < 0) {
            APP_LOGE("Kill Application by bundle name is fail");
            return;
        }
        NotifyAppStatus(bundleName, EventFwk::CommonEventSupport::COMMON_EVENT_PACKAGE_DATA_CLEARED);
    }
}

int32_t AppMgrServiceInner::IsBackgroundRunningRestricted(const std::string &bundleName)
{
    auto bundleMgr_ = remoteClientManager_->GetBundleManager();
    if (bundleMgr_ == nullptr) {
        APP_LOGE("GetBundleManager fail");
        return ERR_DEAD_OBJECT;
    }
    return bundleMgr_->CheckPermission(bundleName, REQ_PERMISSION);
}

int32_t AppMgrServiceInner::GetAllRunningProcesses(std::vector<RunningProcessInfo> &info)
{
    auto bundleMgr_ = remoteClientManager_->GetBundleManager();
    if (bundleMgr_ == nullptr) {
        APP_LOGE("GetBundleManager fail");
        return ERR_DEAD_OBJECT;
    }
    // check permission
    for (const auto &item : appRunningManager_->GetAppRunningRecordMap()) {
        const auto &appRecord = item.second;
        RunningProcessInfo runningProcessInfo;
        runningProcessInfo.processName_ = appRecord->GetName();
        runningProcessInfo.pid_ = appRecord->GetPriorityObject()->GetPid();
        runningProcessInfo.uid_ = appRecord->GetUid();
        runningProcessInfo.state_ = static_cast<AppProcessState>(appRecord->GetState());
        info.emplace_back(runningProcessInfo);
    }
    return ERR_OK;
}

int32_t AppMgrServiceInner::KillProcessByPid(const pid_t pid) const
{
    int32_t ret = -1;
    if (pid > 0) {
        APP_LOGI("kill pid %{public}d", pid);
        ret = kill(pid, SIGNAL_KILL);
    }
    return ret;
}

bool AppMgrServiceInner::WaitForRemoteProcessExit(std::list<pid_t> &pids, const int64_t startTime)
{
    int64_t delayTime = SystemTimeMillis() - startTime;
    while (delayTime < KILL_PROCESS_TIMEOUT_MICRO_SECONDS) {
        if (CheckALLProcessExist(pids)) {
            return true;
        }
        usleep(KILL_PROCESS_DELAYTIME_MICRO_SECONDS);
        delayTime = SystemTimeMillis() - startTime;
    }
    return false;
}

bool AppMgrServiceInner::GetAllPids(std::list<pid_t> &pids)
{
    for (const auto &appTaskInfo : appProcessManager_->GetRecentAppList()) {
        if (appTaskInfo) {
            auto appRecord = GetAppRunningRecordByPid(appTaskInfo->GetPid());
            if (appRecord) {
                pids.push_back(appTaskInfo->GetPid());
                appRecord->ScheduleProcessSecurityExit();
            }
        }
    }
    return (pids.empty() ? false : true);
}

bool AppMgrServiceInner::process_exist(pid_t &pid)
{
    char pid_path[128] = {0};
    struct stat stat_buf;
    if (!pid) {
        return false;
    }
    if (snprintf_s(pid_path, sizeof(pid_path), sizeof(pid_path) - 1, "/proc/%d/status", pid) < 0) {
        return false;
    }
    if (stat(pid_path, &stat_buf) == 0) {
        return true;
    }
    return false;
}

bool AppMgrServiceInner::CheckALLProcessExist(std::list<pid_t> &pids)
{
    for (auto iter = pids.begin(); iter != pids.end(); ) {
        if (!process_exist(*iter) && pids.size() != 0) {
            pids.erase(iter);
            if (pids.empty()) {
                return true;
            }
        } else {
            iter++;
        }
    }
    return false;
}

int64_t AppMgrServiceInner::SystemTimeMillis()
{
    struct timespec t;
    t.tv_sec = 0;
    t.tv_nsec = 0;
    clock_gettime(CLOCK_MONOTONIC, &t);
    return (int64_t)((t.tv_sec) * NANOSECONDS + t.tv_nsec) / MICROSECONDS;
}

std::shared_ptr<AppRunningRecord> AppMgrServiceInner::GetAppRunningRecordByAppName(const std::string &appName) const
{
    return appRunningManager_->GetAppRunningRecordByAppName(appName);
}

std::shared_ptr<AppRunningRecord> AppMgrServiceInner::GetAppRunningRecordByProcessName(
    const std::string &appName, const std::string &processName, const int uid) const
{
    return appRunningManager_->GetAppRunningRecordByProcessName(appName, processName, uid);
}

std::shared_ptr<AppRunningRecord> AppMgrServiceInner::GetAppRunningRecordByPid(const pid_t pid) const
{
    return appRunningManager_->GetAppRunningRecordByPid(pid);
}

std::shared_ptr<AppRunningRecord> AppMgrServiceInner::GetOrCreateAppRunningRecord(const sptr<IRemoteObject> &token,
    const std::shared_ptr<ApplicationInfo> &appInfo, const std::shared_ptr<AbilityInfo> &abilityInfo,
    const std::string &processName, const int32_t uid, RecordQueryResult &result)
{
    BYTRACE(BYTRACE_TAG_APP);
    return appRunningManager_->GetOrCreateAppRunningRecord(token, appInfo, abilityInfo, processName, uid, result);
}

std::shared_ptr<AppRunningRecord> AppMgrServiceInner::GetOrCreateAppRunningRecord(
    const ApplicationInfo &appInfo, bool &appExist)
{
    return appRunningManager_->GetOrCreateAppRunningRecord(appInfo, appExist);
}

void AppMgrServiceInner::TerminateAbility(const sptr<IRemoteObject> &token)
{
    BYTRACE(BYTRACE_TAG_APP);
    APP_LOGD("AppMgrServiceInner::TerminateAbility begin");
    if (!token) {
        APP_LOGE("AppMgrServiceInner::TerminateAbility token is null!");
        return;
    }
    auto appRecord = GetAppRunningRecordByAbilityToken(token);
    if (!appRecord) {
        APP_LOGE("AppMgrServiceInner::TerminateAbility app is not exist!");
        return;
    }
    if (appRecord->GetState() == ApplicationState::APP_STATE_SUSPENDED) {
        appRecord->SetState(ApplicationState::APP_STATE_BACKGROUND);
        OptimizerAppStateChanged(appRecord, ApplicationState::APP_STATE_SUSPENDED);
    }

    if (appRunningManager_) {
        appRunningManager_->TerminateAbility(token);
    }
    APP_LOGD("AppMgrServiceInner::TerminateAbility end");
}

void AppMgrServiceInner::UpdateAbilityState(const sptr<IRemoteObject> &token, const AbilityState state)
{
    BYTRACE(BYTRACE_TAG_APP);
    if (!token) {
        APP_LOGE("token is null!");
        return;
    }
    if (state > AbilityState::ABILITY_STATE_BACKGROUND || state < AbilityState::ABILITY_STATE_FOREGROUND) {
        APP_LOGE("state is not foreground or background!");
        return;
    }
    auto appRecord = GetAppRunningRecordByAbilityToken(token);
    if (!appRecord) {
        APP_LOGE("app is not exist!");
        return;
    }
    auto abilityRecord = appRecord->GetAbilityRunningRecordByToken(token);
    if (!abilityRecord) {
        APP_LOGE("can not find ability record!");
        return;
    }
    if (state == abilityRecord->GetState()) {
        APP_LOGE("current state is already, no need update!");
        return;
    }
    if (appRecord->GetState() == ApplicationState::APP_STATE_SUSPENDED) {
        appRecord->SetState(ApplicationState::APP_STATE_BACKGROUND);
        OptimizerAppStateChanged(appRecord, ApplicationState::APP_STATE_SUSPENDED);
    }

    appRecord->UpdateAbilityState(token, state);
    APP_LOGD("end");
}

void AppMgrServiceInner::UpdateExtensionState(const sptr<IRemoteObject> &token, const ExtensionState state)
{
    if (!token) {
        APP_LOGE("token is null!");
        return;
    }
    auto appRecord = GetAppRunningRecordByAbilityToken(token);
    if (!appRecord) {
        APP_LOGE("app is not exist!");
        return;
    }
    auto abilityRecord = appRecord->GetAbilityRunningRecordByToken(token);
    if (!abilityRecord) {
        APP_LOGE("can not find ability record!");
        return;
    }
    appRecord->StateChangedNotifyObserver(abilityRecord, static_cast<int32_t>(state), false);
}

void AppMgrServiceInner::OnStop()
{
    appRunningManager_->ClearAppRunningRecordMap();
    CloseAppSpawnConnection();
}

ErrCode AppMgrServiceInner::OpenAppSpawnConnection()
{
    if (remoteClientManager_->GetSpawnClient()) {
        return remoteClientManager_->GetSpawnClient()->OpenConnection();
    }
    return ERR_APPEXECFWK_BAD_APPSPAWN_CLIENT;
}

void AppMgrServiceInner::CloseAppSpawnConnection() const
{
    if (remoteClientManager_->GetSpawnClient()) {
        remoteClientManager_->GetSpawnClient()->CloseConnection();
    }
}

SpawnConnectionState AppMgrServiceInner::QueryAppSpawnConnectionState() const
{
    if (remoteClientManager_->GetSpawnClient()) {
        return remoteClientManager_->GetSpawnClient()->QueryConnectionState();
    }
    return SpawnConnectionState::STATE_NOT_CONNECT;
}

const std::map<const int32_t, const std::shared_ptr<AppRunningRecord>> &AppMgrServiceInner::GetRecordMap() const
{
    return appRunningManager_->GetAppRunningRecordMap();
}

void AppMgrServiceInner::SetAppSpawnClient(std::shared_ptr<AppSpawnClient> spawnClient)
{
    remoteClientManager_->SetSpawnClient(std::move(spawnClient));
}

void AppMgrServiceInner::SetBundleManager(sptr<IBundleMgr> bundleManager)
{
    remoteClientManager_->SetBundleManager(bundleManager);
}

void AppMgrServiceInner::RegisterAppStateCallback(const sptr<IAppStateCallback> &callback)
{
    BYTRACE(BYTRACE_TAG_APP);
    if (callback != nullptr) {
        appStateCallbacks_.push_back(callback);
    }
}

void AppMgrServiceInner::StopAllProcess()
{
    BYTRACE(BYTRACE_TAG_APP);
    ClearRecentAppList();
    appRunningManager_->ClearAppRunningRecordMap();
}

void AppMgrServiceInner::AbilityBehaviorAnalysis(const sptr<IRemoteObject> &token, const sptr<IRemoteObject> &preToken,
    const int32_t visibility,       // 0:false,1:true
    const int32_t perceptibility,   // 0:false,1:true
    const int32_t connectionState)  // 0:false,1:true
{
    BYTRACE(BYTRACE_TAG_APP);
    if (!token) {
        APP_LOGE("token is null");
        return;
    }
    auto appRecord = GetAppRunningRecordByAbilityToken(token);
    if (!appRecord) {
        APP_LOGE("app record is not exist for ability token");
        return;
    }
    auto abilityRecord = appRecord->GetAbilityRunningRecordByToken(token);
    if (!abilityRecord) {
        APP_LOGE("ability record is not exist for ability previous token");
        return;
    }
    if (preToken) {
        abilityRecord->SetPreToken(preToken);
    }
    if (abilityRecord->GetVisibility() != visibility) {
        if (processOptimizerUBA_) {
            processOptimizerUBA_->OnAbilityVisibleChanged(abilityRecord);
        }
    }
    if (abilityRecord->GetPerceptibility() != perceptibility) {
        if (processOptimizerUBA_) {
            processOptimizerUBA_->OnAbilityPerceptibleChanged(abilityRecord);
        }
    }
    abilityRecord->SetVisibility(visibility);
    abilityRecord->SetPerceptibility(perceptibility);
    abilityRecord->SetConnectionState(connectionState);
    OptimizerAbilityStateChanged(abilityRecord, abilityRecord->GetState());
}

void AppMgrServiceInner::KillProcessByAbilityToken(const sptr<IRemoteObject> &token)
{
    BYTRACE(BYTRACE_TAG_APP);
    if (!token) {
        APP_LOGE("token is null");
        return;
    }
    auto appRecord = GetAppRunningRecordByAbilityToken(token);
    if (!appRecord) {
        APP_LOGE("app record is not exist for ability token");
        return;
    }
    std::list<pid_t> pids;
    pid_t pid = appRecord->GetPriorityObject()->GetPid();
    if (pid > 0) {
        pids.push_back(pid);
        appRecord->ScheduleProcessSecurityExit();
        if (!WaitForRemoteProcessExit(pids, SystemTimeMillis())) {
            int32_t result = KillProcessByPid(pid);
            if (result < 0) {
                APP_LOGE("KillProcessByAbilityToken kill process is fail");
                return;
            }
        }
    }
}

void AppMgrServiceInner::StartAbility(const sptr<IRemoteObject> &token, const sptr<IRemoteObject> &preToken,
    const std::shared_ptr<AbilityInfo> &abilityInfo, const std::shared_ptr<AppRunningRecord> &appRecord)
{
    BYTRACE(BYTRACE_TAG_APP);
    APP_LOGI("already create appRecord, just start ability");
    if (!appRecord) {
        APP_LOGE("appRecord is null");
        return;
    }

    if (abilityInfo->launchMode == LaunchMode::SINGLETON) {
        auto abilityRecord = appRecord->GetAbilityRunningRecord(abilityInfo->name);
        if (abilityRecord) {
            APP_LOGW("same ability info in singleton launch mode, will not add ability");
            return;
        }
    }

    auto ability = appRecord->GetAbilityRunningRecordByToken(token);
    if (ability && preToken) {
        APP_LOGE("Ability is already started");
        ability->SetPreToken(preToken);
        return;
    }

    ApplicationState appState = appRecord->GetState();
    if (appState == ApplicationState::APP_STATE_SUSPENDED) {
        appRecord->SetState(ApplicationState::APP_STATE_BACKGROUND);
        OptimizerAppStateChanged(appRecord, ApplicationState::APP_STATE_SUSPENDED);
    }

    ability = appRecord->AddAbility(token, abilityInfo);
    if (!ability) {
        APP_LOGE("add ability failed");
        return;
    }

    if (preToken != nullptr) {
        ability->SetPreToken(preToken);
    }

    if (appState == ApplicationState::APP_STATE_CREATE) {
        APP_LOGE("in create state, don't launch ability");
        return;
    }
    appRecord->LaunchAbility(ability);
}

std::shared_ptr<AppRunningRecord> AppMgrServiceInner::GetAppRunningRecordByAbilityToken(
    const sptr<IRemoteObject> &abilityToken) const
{
    return appRunningManager_->GetAppRunningRecordByAbilityToken(abilityToken);
}

void AppMgrServiceInner::UnsuspendApplication(const std::shared_ptr<AppRunningRecord> &appRecord)
{
    if (!appRecord) {
        APP_LOGE("app record is null");
        return;
    }

    APP_LOGI("%{public}s : app name is %{public}s , Uid is %{public}d",
        __func__,
        appRecord->GetName().c_str(),
        appRecord->GetUid());
    // Resume subscription via UID
    DelayedSingleton<EventFwk::CommonEvent>::GetInstance()->Unfreeze(appRecord->GetUid());
}

void AppMgrServiceInner::SuspendApplication(const std::shared_ptr<AppRunningRecord> &appRecord)
{
    if (!appRecord) {
        APP_LOGE("app record is null");
        return;
    }
    APP_LOGD("%{public}s : app name is %{public}s , Uid is %{public}d",
        __func__,
        appRecord->GetName().c_str(),
        appRecord->GetUid());
    // Temporary unsubscribe via UID
    appRecord->SetState(ApplicationState::APP_STATE_SUSPENDED);
    OptimizerAppStateChanged(appRecord, ApplicationState::APP_STATE_BACKGROUND);
    DelayedSingleton<EventFwk::CommonEvent>::GetInstance()->Freeze(appRecord->GetUid());
}

void AppMgrServiceInner::LowMemoryApplicationAlert(
    const std::shared_ptr<AppRunningRecord> &appRecord, const CgroupManager::LowMemoryLevel level)
{
    if (!appRecord) {
        APP_LOGE("app record is null");
        return;
    }
}

std::shared_ptr<AppRunningRecord> AppMgrServiceInner::GetAbilityOwnerApp(
    const std::shared_ptr<AbilityRunningRecord> &abilityRecord) const
{
    if (!abilityRecord) {
        APP_LOGE("ability record is null");
        return nullptr;
    }
    if (!abilityRecord->GetToken()) {
        APP_LOGE("ability token is null");
        return nullptr;
    }
    auto appRecord = GetAppRunningRecordByAbilityToken(abilityRecord->GetToken());
    if (!appRecord) {
        APP_LOGE("The app information corresponding to token does not exist");
        return nullptr;
    }
    return appRecord;
}

std::shared_ptr<AbilityRunningRecord> AppMgrServiceInner::GetAbilityRunningRecordByAbilityToken(
    const sptr<IRemoteObject> &abilityToken) const
{
    if (!abilityToken) {
        APP_LOGE("ability token is null");
        return nullptr;
    }
    auto appRecord = GetAppRunningRecordByAbilityToken(abilityToken);
    if (!appRecord) {
        APP_LOGE("The app information corresponding to token does not exist");
        return nullptr;
    }
    auto abilityRecord = appRecord->GetAbilityRunningRecordByToken(abilityToken);
    if (!abilityRecord) {
        APP_LOGE("The ability information corresponding to token does not exist");
        return nullptr;
    }
    return abilityRecord;
}

void AppMgrServiceInner::AbilityTerminated(const sptr<IRemoteObject> &token)
{
    BYTRACE(BYTRACE_TAG_APP);
    APP_LOGD("begin");
    if (!token) {
        APP_LOGE("token is null!");
        return;
    }

    auto appRecord = appRunningManager_->GetTerminatingAppRunningRecord(token);
    if (!appRecord) {
        APP_LOGE("app is not exist!");
        return;
    }

    appRecord->AbilityTerminated(token);
    APP_LOGD("end");
}

std::shared_ptr<AppRunningRecord> AppMgrServiceInner::GetAppRunningRecordByAppRecordId(const int32_t recordId) const
{
    const auto &iter = appRunningManager_->GetAppRunningRecordMap().find(recordId);
    if (iter != appRunningManager_->GetAppRunningRecordMap().end()) {
        return iter->second;
    }
    return nullptr;
}

void AppMgrServiceInner::OnAppStateChanged(
    const std::shared_ptr<AppRunningRecord> &appRecord, const ApplicationState state)
{
    APP_LOGD("begin, state:%{public}d", static_cast<int32_t>(state));
    if (!appRecord) {
        APP_LOGE("app record is null");
        return;
    }

    for (const auto &callback : appStateCallbacks_) {
        if (callback != nullptr) {
            callback->OnAppStateChanged(WrapAppProcessData(appRecord, state));
        }
    }

    if (state == ApplicationState::APP_STATE_FOREGROUND || state == ApplicationState::APP_STATE_BACKGROUND) {
        AppStateData data = WrapAppStateData(appRecord, state);
        APP_LOGD("OnForegroundApplicationChanged, size:%{public}d, name:%{public}s, uid:%{public}d, state:%{public}d",
            appStateObservers_.size(), data.bundleName.c_str(), data.uid, data.state);
        std::lock_guard<std::recursive_mutex> lockNotify(observerLock_);
        for (const auto &observer : appStateObservers_) {
            if (observer != nullptr) {
                observer->OnForegroundApplicationChanged(data);
            }
        }
    }
    APP_LOGD("end");
}

AppProcessData AppMgrServiceInner::WrapAppProcessData(const std::shared_ptr<AppRunningRecord> &appRecord,
    const ApplicationState state)
{
    AppProcessData processData;
    processData.appName = appRecord->GetName();
    processData.processName = appRecord->GetProcessName();
    processData.pid = appRecord->GetPriorityObject()->GetPid();
    processData.appState = state;
    processData.uid = appRecord->GetUid();
    return processData;
}

AppStateData AppMgrServiceInner::WrapAppStateData(const std::shared_ptr<AppRunningRecord> &appRecord,
    const ApplicationState state)
{
    AppStateData appStateData;
    appStateData.bundleName = appRecord->GetBundleName();
    appStateData.state = static_cast<int32_t>(state);
    appStateData.uid = appRecord->GetUid();
    return appStateData;
}

ProcessData AppMgrServiceInner::WrapProcessData(const std::shared_ptr<AppRunningRecord> &appRecord)
{
    ProcessData processData;
    processData.bundleName = appRecord->GetBundleName();
    processData.pid = appRecord->GetPriorityObject()->GetPid();
    processData.uid = appRecord->GetUid();
    return processData;
}

void AppMgrServiceInner::OnAbilityStateChanged(
    const std::shared_ptr<AbilityRunningRecord> &ability, const AbilityState state)
{
    APP_LOGD("begin, state:%{public}d", static_cast<int32_t>(state));
    if (!ability) {
        APP_LOGE("ability is null");
        return;
    }
    for (const auto &callback : appStateCallbacks_) {
        if (callback != nullptr) {
            callback->OnAbilityRequestDone(ability->GetToken(), state);
        }
    }
    APP_LOGD("end");
}

void AppMgrServiceInner::StateChangedNotifyObserver(const AbilityStateData abilityStateData, bool isAbility)
{
    std::lock_guard<std::recursive_mutex> lockNotify(observerLock_);
    APP_LOGD("bundle:%{public}s, ability:%{public}s, state:%{public}d, pid:%{public}d, uid:%{public}d",
        abilityStateData.bundleName.c_str(), abilityStateData.abilityName.c_str(),
        abilityStateData.abilityState, abilityStateData.pid, abilityStateData.uid);
    for (const auto &observer : appStateObservers_) {
        if (observer != nullptr) {
            if (isAbility) {
                observer->OnAbilityStateChanged(abilityStateData);
            } else {
                observer->OnExtensionStateChanged(abilityStateData);
            }
        }
    }
}

void AppMgrServiceInner::OnProcessCreated(const std::shared_ptr<AppRunningRecord> &appRecord)
{
    APP_LOGD("OnProcessCreated begin.");
    if (!appRecord) {
        APP_LOGE("app record is null");
        return;
    }
    ProcessData data = WrapProcessData(appRecord);
    APP_LOGD("OnProcessCreated, bundle:%{public}s, pid:%{public}d, uid:%{public}d, size:%{public}d",
        data.bundleName.c_str(), data.uid, data.pid, appStateObservers_.size());
    std::lock_guard<std::recursive_mutex> lockNotify(observerLock_);
    for (const auto &observer : appStateObservers_) {
        if (observer != nullptr) {
            observer->OnProcessCreated(data);
        }
    }
    APP_LOGD("end");
}

void AppMgrServiceInner::OnProcessDied(const std::shared_ptr<AppRunningRecord> &appRecord)
{
    APP_LOGD("OnProcessDied begin.");
    if (!appRecord) {
        APP_LOGE("app record is null");
        return;
    }
    ProcessData data = WrapProcessData(appRecord);
    APP_LOGD("OnProcessDied, bundle:%{public}s, pid:%{public}d, uid:%{public}d, size:%{public}d",
        data.bundleName.c_str(), data.uid, data.pid, appStateObservers_.size());
    std::lock_guard<std::recursive_mutex> lockNotify(observerLock_);
    for (const auto &observer : appStateObservers_) {
        if (observer != nullptr) {
            observer->OnProcessDied(data);
        }
    }
    APP_LOGD("end");
}

void AppMgrServiceInner::StartProcess(const std::string &appName, const std::string &processName,
    const std::shared_ptr<AppRunningRecord> &appRecord, const int uid)
{
    BYTRACE(BYTRACE_TAG_APP);
    if (!remoteClientManager_->GetSpawnClient() || !appRecord) {
        APP_LOGE("appSpawnClient or apprecord is null");
        return;
    }

    auto bundleMgr_ = remoteClientManager_->GetBundleManager();
    if (bundleMgr_ == nullptr) {
        APP_LOGE("GetBundleManager fail");
        return;
    }

    AppSpawnStartMsg startMsg;
    BundleInfo bundleInfo;
    std::vector<AppExecFwk::BundleInfo> bundleInfos;
    bool bundleMgrResult = bundleMgr_->GetBundleInfos(AppExecFwk::BundleFlag::GET_BUNDLE_WITH_ABILITIES, bundleInfos);
    if (!bundleMgrResult) {
        APP_LOGE("GetBundleInfo is fail");
        return;
    }
    std::string bundleName = appRecord->GetBundleName();
    auto isExist = [&bundleName, &uid](const AppExecFwk::BundleInfo &bundleInfo) {
        return bundleInfo.name == bundleName && bundleInfo.uid == uid;
    };
    auto bundleInfoIter = std::find_if(bundleInfos.begin(), bundleInfos.end(), isExist);
    if (bundleInfoIter == bundleInfos.end()) {
        APP_LOGE("Get target fail.");
        return;
    }
    startMsg.uid = (*bundleInfoIter).uid;
    startMsg.gid = (*bundleInfoIter).gid;

    bundleMgrResult = bundleMgr_->GetBundleGidsByUid(appRecord->GetBundleName(), uid, startMsg.gids);
    if (!bundleMgrResult) {
        APP_LOGE("GetBundleGids is fail");
        return;
    }
    startMsg.procName = processName;
    startMsg.soPath = SO_PATH;

    PerfProfile::GetInstance().SetAppForkStartTime(GetTickCount());
    pid_t pid = 0;
    ErrCode errCode = remoteClientManager_->GetSpawnClient()->StartProcess(startMsg, pid);
    if (FAILED(errCode)) {
        APP_LOGE("failed to spawn new app process, errCode %{public}08x", errCode);
        appRunningManager_->RemoveAppRunningRecordById(appRecord->GetRecordId());
        return;
    }
    APP_LOGI("newPid %{public}d", pid);
    appRecord->GetPriorityObject()->SetPid(pid);
    APP_LOGI("app uid %{public}d", startMsg.uid);
    appRecord->SetUid(startMsg.uid);
    OptimizerAppStateChanged(appRecord, ApplicationState::APP_STATE_CREATE);
    appRecord->SetAppMgrServiceInner(weak_from_this());
    OnAppStateChanged(appRecord, ApplicationState::APP_STATE_CREATE);
    AddAppToRecentList(appName, appRecord->GetProcessName(), pid, appRecord->GetRecordId());
    OnProcessCreated(appRecord);
    PerfProfile::GetInstance().SetAppForkEndTime(GetTickCount());
}

void AppMgrServiceInner::RemoveAppFromRecentList(const std::string &appName, const std::string &processName)
{
    int64_t startTime = 0;
    std::list<pid_t> pids;
    auto appTaskInfo = appProcessManager_->GetAppTaskInfoByProcessName(appName, processName);
    if (!appTaskInfo) {
        return;
    }
    auto appRecord = GetAppRunningRecordByPid(appTaskInfo->GetPid());
    if (!appRecord) {
        appProcessManager_->RemoveAppFromRecentList(appTaskInfo);
        return;
    }
    startTime = SystemTimeMillis();
    pids.push_back(appTaskInfo->GetPid());
    appRecord->ScheduleProcessSecurityExit();
    if (!WaitForRemoteProcessExit(pids, startTime)) {
        int32_t result = KillProcessByPid(appTaskInfo->GetPid());
        if (result < 0) {
            APP_LOGE("RemoveAppFromRecentList kill process is fail");
            return;
        }
    }
    appProcessManager_->RemoveAppFromRecentList(appTaskInfo);
}

const std::list<const std::shared_ptr<AppTaskInfo>> &AppMgrServiceInner::GetRecentAppList() const
{
    return appProcessManager_->GetRecentAppList();
}

void AppMgrServiceInner::ClearRecentAppList()
{
    int64_t startTime = 0;
    std::list<pid_t> pids;
    if (GetAllPids(pids)) {
        return;
    }
    startTime = SystemTimeMillis();
    if (WaitForRemoteProcessExit(pids, startTime)) {
        appProcessManager_->ClearRecentAppList();
        return;
    }
    for (auto iter = pids.begin(); iter != pids.end(); ++iter) {
        int32_t result = KillProcessByPid(*iter);
        if (result < 0) {
            APP_LOGE("ClearRecentAppList kill process is fail");
            return;
        }
    }
    appProcessManager_->ClearRecentAppList();
}

void AppMgrServiceInner::OnRemoteDied(const wptr<IRemoteObject> &remote)
{
    auto appRecord = appRunningManager_->OnRemoteDied(remote);
    if (appRecord) {
        for (const auto &item : appRecord->GetAbilities()) {
            const auto &abilityRecord = item.second;
            OptimizerAbilityStateChanged(abilityRecord, AbilityState::ABILITY_STATE_TERMINATED);
            appRecord->StateChangedNotifyObserver(abilityRecord,
                static_cast<int32_t>(AbilityState::ABILITY_STATE_TERMINATED), true);
        }
        OptimizerAppStateChanged(appRecord, ApplicationState::APP_STATE_TERMINATED);
        RemoveAppFromRecentListById(appRecord->GetRecordId());
        OnProcessDied(appRecord);
    }

    if (appRecord && appRecord->IsKeepAliveApp()) {
        std::string bundleName = appRecord->GetBundleName();
        APP_LOGI("[%{public}s] will be restartProcss!", bundleName.c_str());
        auto restartProcss = [appRecord, innerService = shared_from_this()]() {
            innerService->RestartResidentProcess(appRecord);
        };

        if (!eventHandler_) {
            APP_LOGE("eventHandler_ is nullptr");
            return;
        }
        eventHandler_->PostTask(restartProcss, "RestartResidentProcess");
    }
}

void AppMgrServiceInner::PushAppFront(const int32_t recordId)
{
    appProcessManager_->PushAppFront(recordId);
}

void AppMgrServiceInner::RemoveAppFromRecentListById(const int32_t recordId)
{
    appProcessManager_->RemoveAppFromRecentListById(recordId);
}

void AppMgrServiceInner::AddAppToRecentList(
    const std::string &appName, const std::string &processName, const pid_t pid, const int32_t recordId)
{
    appProcessManager_->AddAppToRecentList(appName, processName, pid, recordId);
}

const std::shared_ptr<AppTaskInfo> AppMgrServiceInner::GetAppTaskInfoById(const int32_t recordId) const
{
    return appProcessManager_->GetAppTaskInfoById(recordId);
}

void AppMgrServiceInner::AddAppDeathRecipient(const pid_t pid, const sptr<AppDeathRecipient> &appDeathRecipient) const
{
    BYTRACE(BYTRACE_TAG_APP);
    std::shared_ptr<AppRunningRecord> appRecord = GetAppRunningRecordByPid(pid);
    if (appRecord) {
        appRecord->SetAppDeathRecipient(appDeathRecipient);
    }
}

int32_t AppMgrServiceInner::ProcessOptimizerInit()
{
    processOptimizerUBA_ = std::make_shared<ProcessOptimizerUBA>(nullptr);
    bool isSuccess = processOptimizerUBA_->Init();
    if (!isSuccess) {
        processOptimizerUBA_.reset();
        processOptimizerUBA_ = nullptr;
        APP_LOGE("optimizer init is fail");
        return ERR_NO_INIT;
    }
    processOptimizerUBA_->AppSuspended =
        std::bind(&AppMgrServiceInner::SuspendApplication, this, std::placeholders::_1);
    // Register freeze callback function
    processOptimizerUBA_->AppResumed =
        std::bind(&AppMgrServiceInner::UnsuspendApplication, this, std::placeholders::_1);
    // Register freeze recovery callback function
    processOptimizerUBA_->AppLowMemoryAlert =
        std::bind(&AppMgrServiceInner::LowMemoryApplicationAlert, this, std::placeholders::_1, std::placeholders::_2);
    // Register low memory warning callback function
    processOptimizerUBA_->GetAbilityOwnerApp =
        std::bind(&AppMgrServiceInner::GetAbilityOwnerApp, this, std::placeholders::_1);
    // Register to get the application record callback of ability
    processOptimizerUBA_->GetAbilityByToken =
        std::bind(&AppMgrServiceInner::GetAbilityRunningRecordByAbilityToken, this, std::placeholders::_1);
    // Register to get the ability record through the token callback
    APP_LOGI("optimizer init is success");
    return ERR_OK;
}

void AppMgrServiceInner::OptimizerAbilityStateChanged(
    const std::shared_ptr<AbilityRunningRecord> &ability, const AbilityState state)
{
    if (!processOptimizerUBA_) {
        APP_LOGE("process optimizer is not init");
        return;
    }

    if ((ability->GetAbilityInfo()->type == AbilityType::PAGE) ||
        (ability->GetAbilityInfo()->type == AbilityType::DATA)) {
        if (ability->GetState() == AbilityState::ABILITY_STATE_CREATE) {
            processOptimizerUBA_->OnAbilityStarted(ability);
            APP_LOGI("optimizer OnAbilityStarted is called");
        } else if (ability->GetState() == AbilityState::ABILITY_STATE_TERMINATED) {
            processOptimizerUBA_->OnAbilityRemoved(ability);
            APP_LOGI("optimizer OnAbilityRemoved is called");
        } else {
            processOptimizerUBA_->OnAbilityStateChanged(ability, state);
            APP_LOGI("optimizer OnAbilityStateChanged is called");
        }
    } else if (ability->GetAbilityInfo()->type == AbilityType::SERVICE) {
        auto appRecord = GetAppRunningRecordByAbilityToken(ability->GetPreToken());
        if (!appRecord) {
            APP_LOGE("app record is not exist for ability token");
            return;
        }
        auto targetAbility = appRecord->GetAbilityRunningRecordByToken(ability->GetPreToken());
        if (!targetAbility) {
            APP_LOGE("ability record is not exist for ability previous token");
            return;
        }
        if (ability->GetConnectionState()) {
            // connect
            processOptimizerUBA_->OnAbilityConnected(ability, targetAbility);
            APP_LOGI("optimizer OnAbilityConnected is called");
        } else {
            // disconnect
            processOptimizerUBA_->OnAbilityDisconnected(ability, targetAbility);
            APP_LOGI("optimizer OnAbilityDisconnected is called");
        }
    } else {
        APP_LOGI("OptimizerAbilityStateChanged ability type is unknown");
    }

    if (ability->GetState() != state) {
        processOptimizerUBA_->OnAbilityStateChanged(ability, state);
        APP_LOGI("optimizer OnAbilityStateChanged is called");
    }
}

void AppMgrServiceInner::OptimizerAppStateChanged(
    const std::shared_ptr<AppRunningRecord> &appRecord, const ApplicationState state)
{
    if (!processOptimizerUBA_) {
        APP_LOGE("process optimizer is not init");
        return;
    }
    if (appRecord->GetState() == ApplicationState::APP_STATE_CREATE) {
        processOptimizerUBA_->OnAppAdded(appRecord);
        APP_LOGI("optimizer OnAppAdded is called");
    } else if (appRecord->GetState() == ApplicationState::APP_STATE_TERMINATED) {
        processOptimizerUBA_->OnAppRemoved(appRecord);
        APP_LOGI("optimizer OnAppRemoved is called");
    } else {
        processOptimizerUBA_->OnAppStateChanged(appRecord, state);
        APP_LOGI("optimizer OnAppStateChanged is called");
    }
}

void AppMgrServiceInner::SetAppFreezingTime(int time)
{
    if (!processOptimizerUBA_) {
        APP_LOGE("process optimizer is not init");
        return;
    }

    std::lock_guard<std::mutex> setFreezeTimeLock(serviceLock_);
    processOptimizerUBA_->SetAppFreezingTime(time);
}

void AppMgrServiceInner::GetAppFreezingTime(int &time)
{
    if (!processOptimizerUBA_) {
        APP_LOGE("process optimizer is not init");
        return;
    }
    std::lock_guard<std::mutex> getFreezeTimeLock(serviceLock_);
    processOptimizerUBA_->GetAppFreezingTime(time);
}

void AppMgrServiceInner::HandleTimeOut(const InnerEvent::Pointer &event)
{
    APP_LOGI("%{public}s", __func__);
    if (!appRunningManager_ || event == nullptr) {
        APP_LOGE("appRunningManager or event is nullptr");
        return;
    }
    switch (event->GetInnerEventId()) {
        case AMSEventHandler::TERMINATE_ABILITY_TIMEOUT_MSG:
            appRunningManager_->HandleTerminateTimeOut(event->GetParam());
            break;
        case AMSEventHandler::TERMINATE_APPLICATION_TIMEOUT_MSG:
            HandleTerminateApplicationTimeOut(event->GetParam());
            break;
        case AMSEventHandler::ADD_ABILITY_STAGE_INFO_TIMEOUT_MSG:
            HandleAddAbilityStageTimeOut(event->GetParam());
            break;
        default:
            break;
    }
}

void AppMgrServiceInner::SetEventHandler(const std::shared_ptr<AMSEventHandler> &handler)
{
    eventHandler_ = handler;
}

void AppMgrServiceInner::HandleAbilityAttachTimeOut(const sptr<IRemoteObject> &token)
{
    BYTRACE(BYTRACE_TAG_APP);
    APP_LOGI("%{public}s called", __func__);
    if (!appRunningManager_) {
        APP_LOGE("appRunningManager_ is nullptr");
        return;
    }
    appRunningManager_->HandleAbilityAttachTimeOut(token);
}

void AppMgrServiceInner::PrepareTerminate(const sptr<IRemoteObject> &token)
{
    APP_LOGI("%{public}s called", __func__);
    if (!appRunningManager_) {
        APP_LOGE("appRunningManager_ is nullptr");
        return;
    }
    appRunningManager_->PrepareTerminate(token);
}

void AppMgrServiceInner::HandleTerminateApplicationTimeOut(const int64_t eventId)
{
    APP_LOGI("%{public}s called", __func__);
    if (!appRunningManager_) {
        APP_LOGE("appRunningManager_ is nullptr");
        return;
    }
    auto appRecord = appRunningManager_->GetAppRunningRecord(eventId);
    if (!appRecord) {
        APP_LOGE("appRecord is nullptr");
        return;
    }
    appRecord->SetState(ApplicationState::APP_STATE_TERMINATED);
    OptimizerAppStateChanged(appRecord, ApplicationState::APP_STATE_BACKGROUND);
    appRecord->RemoveAppDeathRecipient();
    OnAppStateChanged(appRecord, ApplicationState::APP_STATE_TERMINATED);
    pid_t pid = appRecord->GetPriorityObject()->GetPid();
    if (pid > 0) {
        int32_t result = KillProcessByPid(pid);
        if (result < 0) {
            APP_LOGE("KillProcessByAbilityToken kill process is fail");
            return;
        }
    }
    appRunningManager_->RemoveAppRunningRecordById(appRecord->GetRecordId());
    RemoveAppFromRecentListById(appRecord->GetRecordId());
    OnProcessDied(appRecord);
}

void AppMgrServiceInner::HandleAddAbilityStageTimeOut(const int64_t eventId)
{
    APP_LOGI("%{public}s called add ability stage info time out!", __func__);
}

int AppMgrServiceInner::CompelVerifyPermission(const std::string &permission, int pid, int uid, std::string &message)
{
    APP_LOGI("%{public}s called", __func__);
    message = ENUM_TO_STRING(PERMISSION_NOT_GRANTED);
    if (!remoteClientManager_) {
        APP_LOGE("%{public}s remoteClientManager_ is nullptr", __func__);
        return ERR_NO_INIT;
    }
    if (permission.empty()) {
        APP_LOGI("permission is empty, PERMISSION_GRANTED");
        message = ENUM_TO_STRING(PERMISSION_GRANTED);
        return ERR_OK;
    }
    if (pid == getpid()) {
        APP_LOGI("pid is my pid, PERMISSION_GRANTED");
        message = ENUM_TO_STRING(PERMISSION_GRANTED);
        return ERR_OK;
    }
    int userId = Constants::DEFAULT_USERID;
    auto appRecord = GetAppRunningRecordByPid(pid);
    if (!appRecord) {
        APP_LOGE("%{public}s app record is nullptr", __func__);
        return PERMISSION_NOT_GRANTED;
    }
    auto bundleName = appRecord->GetBundleName();
    if (appRecord->GetCloneInfo()) {
        userId = Constants::C_UESRID;
    }
    auto bundleMgr = remoteClientManager_->GetBundleManager();
    if (bundleMgr == nullptr) {
        APP_LOGE("%{public}s GetBundleManager fail", __func__);
        return ERR_NO_INIT;
    }
    auto bmsUid = bundleMgr->GetUidByBundleName(bundleName, userId);
    if (bmsUid == ROOT_UID || bmsUid == SYSTEM_UID) {
        APP_LOGI("uid is root or system, PERMISSION_GRANTED");
        message = ENUM_TO_STRING(PERMISSION_GRANTED);
        return ERR_OK;
    }
    if (bmsUid != uid) {
        APP_LOGI("check uid != bms uid, PERMISSION_NOT_GRANTED");
        return PERMISSION_NOT_GRANTED;
    }
    auto result = bundleMgr->CheckPermissionByUid(bundleName, permission, userId);
    if (result != PERMISSION_GRANTED) {
        return PERMISSION_NOT_GRANTED;
    }
    message = ENUM_TO_STRING(PERMISSION_GRANTED);
    return ERR_OK;
}

void AppMgrServiceInner::LoadResidentProcess()
{
    if (!CheckRemoteClient()) {
        APP_LOGE("%{public}s GetBundleManager fail", __func__);
        return;
    }

    // Broadcast monitoring should be used

    std::vector<BundleInfo> infos;
    auto funRet = remoteClientManager_->GetBundleManager()->QueryKeepAliveBundleInfos(infos);
    if (!funRet) {
        APP_LOGE("%{public}s QueryKeepAliveBundleInfos fail!", __func__);
        return;
    }

    APP_LOGI("Get KeepAlive BundleInfo Size : [%{public}d]", static_cast<int>(infos.size()));
    StartResidentProcess(infos);
}

void AppMgrServiceInner::StartResidentProcess(const std::vector<BundleInfo> &infos)
{
    APP_LOGI("%{public}s", __func__);
    if (infos.empty()) {
        APP_LOGE("%{public}s infos is empty!", __func__);
        return;
    }

    // Later, when distinguishing here, there is a process with ability and time and space.
    std::shared_ptr<AppRunningRecord> appRecord(nullptr);
    auto CheckProcessIsExist = [&appRecord, innerService = shared_from_this()](const BundleInfo &iter) {
        bool appExist = false;
        appRecord = innerService->GetOrCreateAppRunningRecord(iter.applicationInfo, appExist);
        APP_LOGI("CheckProcessIsExist appExist : [%{public}d]", appExist);
        return appExist;
    };

    for (auto &iter : infos) {
        if (!CheckProcessIsExist(iter) && appRecord) {
            APP_LOGI("start bundle [%{public}s]", iter.applicationInfo.bundleName.c_str());
            // todo Should it be time-consuming, should it be made asynchronous?
            auto processName = iter.applicationInfo.process.empty() ? iter.applicationInfo.bundleName :
                iter.applicationInfo.process;
            APP_LOGI("processName = [%{public}s]", processName.c_str());
            StartEmptyResidentProcess(iter.applicationInfo.name, processName, appRecord, iter.applicationInfo.uid);
            appRecord->insertAbilityStageInfo(iter.hapModuleInfos);
        }
    }
}

void AppMgrServiceInner::StartEmptyResidentProcess(const std::string &appName, const std::string &processName,
    const std::shared_ptr<AppRunningRecord> &appRecord, const int uid)
{
    APP_LOGI("%{public}s appName [%{public}s] | processName [%{public}s]",
        __func__, appName.c_str(), processName.c_str());

    if (!CheckRemoteClient()) {
        return;
    }

    StartProcess(appName, processName, appRecord, uid);

    // If it is empty, the startup failed
    if (!appRecord || !appRunningManager_) {
        APP_LOGE("start process [%{public}s] failed!", processName.c_str());
        return;
    }
    appRecord->SetKeepAliveAppState();
    appRecord->SetEventHandler(eventHandler_);
    appRunningManager_->InitRestartResidentProcRecord(appRecord->GetProcessName());
    APP_LOGI("StartEmptyResidentProcess oK pid : [%{public}d], ", appRecord->GetPriorityObject()->GetPid());
}

bool AppMgrServiceInner::CheckRemoteClient()
{
    if (!remoteClientManager_) {
        APP_LOGE("remoteClientManager_ is null");
        return false;
    }

    if (!remoteClientManager_->GetSpawnClient()) {
        APP_LOGE("appSpawnClient is null");
        return false;
    }

    if (!remoteClientManager_->GetBundleManager()) {
        APP_LOGE("GetBundleManager fail");
        return false;
    }
    return true;
}

void AppMgrServiceInner::RestartResidentProcess(std::shared_ptr<AppRunningRecord> appRecord)
{
    if (!CheckRemoteClient() || !appRecord || !appRunningManager_) {
        APP_LOGE("%{public}s failed!", __func__);
        return;
    }

    if (!appRunningManager_->CanRestartResidentProcCount(appRecord->GetProcessName())) {
        APP_LOGE("%{public}s no restart times!", appRecord->GetProcessName().c_str());
        return ;
    }

    auto bundleMgr = remoteClientManager_->GetBundleManager();
    BundleInfo bundleInfo;
    if (!bundleMgr->GetBundleInfo(appRecord->GetBundleName(), BundleFlag::GET_BUNDLE_DEFAULT, bundleInfo)) {
        APP_LOGE("%{public}s GetBundleInfo fail", __func__);
        return;
    }
    std::vector<BundleInfo> infos;
    infos.emplace_back(bundleInfo);
    StartResidentProcess(infos);
}

void AppMgrServiceInner::NotifyAppStatus(const std::string &bundleName, const std::string &eventData)
{
    APP_LOGI("%{public}s called, bundle name is %{public}s, event is %{public}s",
        __func__, bundleName.c_str(), eventData.c_str());
    Want want;
    want.SetAction(eventData);
    ElementName element;
    element.SetBundleName(bundleName);
    want.SetElement(element);
    want.SetParam(Constants::USER_ID, 0);
    EventFwk::CommonEventData commonData {want};
    EventFwk::CommonEventManager::PublishCommonEvent(commonData);
}

int32_t AppMgrServiceInner::RegisterApplicationStateObserver(const sptr<IApplicationStateObserver> &observer)
{
    APP_LOGI("%{public}s begin", __func__);
    std::lock_guard<std::recursive_mutex> lockRegister(observerLock_);
    if (observer == nullptr) {
        APP_LOGE("Observer nullptr");
        return ERR_INVALID_VALUE;
    }
    if (ObserverExist(observer)) {
        APP_LOGE("Observer exist.");
        return ERR_INVALID_VALUE;
    }
    appStateObservers_.push_back(observer);
    APP_LOGI("%{public}s appStateObservers_ size:%{public}d", __func__, appStateObservers_.size());
    AddObserverDeathRecipient(observer);
    return ERR_OK;
}

int32_t AppMgrServiceInner::UnregisterApplicationStateObserver(const sptr<IApplicationStateObserver> &observer)
{
    APP_LOGI("%{public}s begin", __func__);
    std::lock_guard<std::recursive_mutex> lockUnregister(observerLock_);
    if (observer == nullptr) {
        APP_LOGE("Observer nullptr");
        return ERR_INVALID_VALUE;
    }
    std::vector<sptr<IApplicationStateObserver>>::iterator it;
    for (it = appStateObservers_.begin(); it != appStateObservers_.end(); it++) {
        if ((*it)->AsObject() == observer->AsObject()) {
            appStateObservers_.erase(it);
            APP_LOGI("%{public}s appStateObservers_ size:%{public}d", __func__, appStateObservers_.size());
            RemoveObserverDeathRecipient(observer);
            return ERR_OK;
        }
    }
    APP_LOGE("Observer not exist.");
    return ERR_INVALID_VALUE;
}

bool AppMgrServiceInner::ObserverExist(const sptr<IApplicationStateObserver> &observer)
{
    if (observer == nullptr) {
        APP_LOGE("Observer nullptr");
        return false;
    }
    for (int i = 0; i < appStateObservers_.size(); i++) {
        if (appStateObservers_[i]->AsObject() == observer->AsObject()) {
            return true;
        }
    }
    return false;
}

void AppMgrServiceInner::AddObserverDeathRecipient(const sptr<IApplicationStateObserver> &observer)
{
    APP_LOGI("%{public}s begin", __func__);
    if (observer == nullptr || observer->AsObject() == nullptr) {
        APP_LOGE("observer nullptr.");
        return;
    }
    auto it = recipientMap_.find(observer->AsObject());
    if (it != recipientMap_.end()) {
        APP_LOGE("This death recipient has been added.");
        return;
    } else {
        sptr<IRemoteObject::DeathRecipient> deathRecipient = new ApplicationStateObserverRecipient(
            std::bind(&AppMgrServiceInner::OnObserverDied, this, std::placeholders::_1));
        observer->AsObject()->AddDeathRecipient(deathRecipient);
        recipientMap_.emplace(observer->AsObject(), deathRecipient);
    }
}

void AppMgrServiceInner::RemoveObserverDeathRecipient(const sptr<IApplicationStateObserver> &observer)
{
    APP_LOGI("%{public}s begin", __func__);
    if (observer == nullptr || observer->AsObject() == nullptr) {
        APP_LOGE("observer nullptr.");
        return;
    }
    auto it = recipientMap_.find(observer->AsObject());
    if (it != recipientMap_.end()) {
        it->first->RemoveDeathRecipient(it->second);
        recipientMap_.erase(it);
        return;
    }
}

void AppMgrServiceInner::OnObserverDied(const wptr<IRemoteObject> &remote)
{
    APP_LOGI("%{public}s begin", __func__);
    auto object = remote.promote();
    if (object == nullptr) {
        APP_LOGE("observer nullptr.");
        return;
    }
    if (eventHandler_) {
        auto task = [object, appManager = this]() {appManager->HandleObserverDiedTask(object);};
        eventHandler_->PostTask(task, TASK_ON_CALLBACK_DIED);
    }
}

void AppMgrServiceInner::HandleObserverDiedTask(const sptr<IRemoteObject> &observer)
{
    APP_LOGI("Handle call back died task.");
    if (observer == nullptr) {
        APP_LOGE("observer nullptr.");
        return;
    }
    sptr<IApplicationStateObserver> object = iface_cast<IApplicationStateObserver>(observer);
    UnregisterApplicationStateObserver(object);
}

int32_t AppMgrServiceInner::GetForegroundApplications(std::vector<AppStateData> &list)
{
    APP_LOGI("%{public}s, begin.", __func__);
    appRunningManager_->GetForegroundApplications(list);
    return ERR_OK;
}
}  // namespace AppExecFwk
}  // namespace OHOS
