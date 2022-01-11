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

#ifndef FOUNDATION_APPEXECFWK_SERVICES_APPMGR_INCLUDE_APP_MGR_SERVICE_INNER_H
#define FOUNDATION_APPEXECFWK_SERVICES_APPMGR_INCLUDE_APP_MGR_SERVICE_INNER_H

#include <list>
#include <map>
#include <vector>

#include "iremote_object.h"
#include "refbase.h"

#include "ability_info.h"
#include "appexecfwk_errors.h"
#include "app_death_recipient.h"
#include "app_mgr_constants.h"
#include "app_record_id.h"
#include "app_running_record.h"
#include "app_scheduler_interface.h"
#include "app_spawn_client.h"
#include "app_task_info.h"
#include "iapp_state_callback.h"
#include "iapplication_state_observer.h"
#include "app_process_manager.h"
#include "remote_client_manager.h"
#include "app_running_manager.h"
#include "record_query_result.h"
#include "running_process_info.h"
#include "bundle_info.h"

#include "process_optimizer_uba.h"

#include "ohos/aafwk/content/want.h"

namespace OHOS {
namespace AppExecFwk {
using OHOS::AAFwk::Want;

class AppMgrServiceInner : public std::enable_shared_from_this<AppMgrServiceInner> {
public:
    AppMgrServiceInner();
    virtual ~AppMgrServiceInner();

    /**
     * LoadAbility, load the ability that needed to be started.
     *
     * @param token, the unique identification to start the ability.
     * @param preToken, the unique identification to call the ability.
     * @param abilityInfo, the ability information.
     * @param appInfo, the app information.
     *
     * @return
     */
    virtual void LoadAbility(const sptr<IRemoteObject> &token, const sptr<IRemoteObject> &preToken,
        const std::shared_ptr<AbilityInfo> &abilityInfo, const std::shared_ptr<ApplicationInfo> &appInfo,
        int32_t uid);

    /**
     * TerminateAbility, terminate the token ability.
     *
     * @param token, he unique identification to terminate the ability.
     *
     * @return
     */
    virtual void TerminateAbility(const sptr<IRemoteObject> &token);

    /**
     * UpdateAbilityState, update the ability status.
     *
     * @param token, the unique identification to update the ability.
     * @param state, ability status that needs to be updated.
     *
     * @return
     */
    virtual void UpdateAbilityState(const sptr<IRemoteObject> &token, const AbilityState state);

    /**
     * UpdateExtensionState, call UpdateExtensionState() through the proxy object, update the extension status.
     *
     * @param token, the unique identification to update the extension.
     * @param state, extension status that needs to be updated.
     */
    virtual void UpdateExtensionState(const sptr<IRemoteObject> &token, const ExtensionState state);

    /**
     * StateChangedNotifyObserver, Call ability state change.
     *
     * @param ability, the ability info.
     * @param state, the ability state.
     *
     * @return
     */
    void StateChangedNotifyObserver(const AbilityStateData abilityStateData, bool isAbility);

    /**
     * RegisterAppStateCallback, register the callback.
     *
     * @param callback, Ams register the callback.
     *
     * @return
     */
    virtual void RegisterAppStateCallback(const sptr<IAppStateCallback> &callback);

    /**
     * StopAllProcess, Terminate all processes.
     *
     * @return
     */
    virtual void StopAllProcess();

    /**
     * AbilityBehaviorAnalysis, ability behavior analysis assistant process optimization.
     *
     * @param token, the unique identification to start the ability.
     * @param preToken, the unique identification to call the ability.
     * @param visibility, the visibility information about windows info.
     * @param perceptibility, the Perceptibility information about windows info.
     * @param connectionState, the service ability connection state.
     * @return
     */
    virtual void AbilityBehaviorAnalysis(const sptr<IRemoteObject> &token, const sptr<IRemoteObject> &preToken,
        const int32_t visibility, const int32_t perceptibility, const int32_t connectionState);

    /**
     * KillProcessByAbilityToken, kill the process by ability token.
     *
     * @param token, the unique identification to the ability.
     * @return
     */
    virtual void KillProcessByAbilityToken(const sptr<IRemoteObject> &token);

    /**
     * KillProcessesByUserId, kill the processes by userId.
     *
     * @param userId, the user id.
     * @return
     */
    virtual void KillProcessesByUserId(int32_t userId);

    /**
     * AttachApplication, get all the information needed to start the Application
     * (data related to the Application ).
     *
     * @param app, information needed to start the Application.
     *
     * @return
     */
    virtual void AttachApplication(const pid_t pid, const sptr<IAppScheduler> &app);

    /**
     * ApplicationForegrounded, set the application to Foreground State.
     *
     * @param recordId, a unique record that identifies this Application from others.
     *
     * @return
     */
    virtual void ApplicationForegrounded(const int32_t recordId);

    /**
     * ApplicationBackgrounded, set the application to Backgrounded State.
     *
     * @param recordId, a unique record that identifies this Application from others.
     *
     * @return
     */
    virtual void ApplicationBackgrounded(const int32_t recordId);

    /**
     * ApplicationTerminated, terminate the application.
     *
     * @param recordId, a unique record that identifies this Application from others.
     *
     * @return
     */
    virtual void ApplicationTerminated(const int32_t recordId);

    /**
     * AbilityTerminated, terminate the ability.
     *
     * @param token, the unique identification to terminated the ability.
     *
     * @return
     */
    virtual void AbilityTerminated(const sptr<IRemoteObject> &token);

    /**
     * KillApplication, kill the application.
     *
     * @param  bundleName, bundle name in Application record.
     *
     * @return ERR_OK, return back success, others fail.
     */
    virtual int32_t KillApplication(const std::string &bundleName);

    /**
     * KillApplicationByUid, call KillApplicationByUid() through proxy object, kill the application.
     *
     * @param  bundleName, bundle name in Application record.
     * @param  uid, uid.
     * @return ERR_OK, return back success, others fail.
     */
    virtual int32_t KillApplicationByUid(const std::string &bundleName, const int uid);

    /**
     * ClearUpApplicationData, clear the application data.
     *
     * @param bundleName, bundle name in Application record.
     * @param callerUid, app uid in Application record.
     * @param callerPid, app pid in Application record.
     *
     * @return
     */
    virtual void ClearUpApplicationData(const std::string &bundleName, const int32_t callerUid, const pid_t callerPid);

    /**
     * IsBackgroundRunningRestricted, Checks whether the process of this application is forbidden
     * to run in the background.
     *
     * @param bundleName, bundle name in Application record.
     *
     * @return ERR_OK, return back success, others fail.
     */
    virtual int32_t IsBackgroundRunningRestricted(const std::string &bundleName);

    /**
     * GetAllRunningProcesses, Obtains information about application processes that are running on the device.
     *
     * @param info, app name in Application record.
     *
     * @return ERR_OK ,return back success，others fail.
     */
    virtual int32_t GetAllRunningProcesses(std::vector<RunningProcessInfo> &info);

    // Get AppRunningRecord according to appInfo. Create if not exists.
    // Create ability record if not exists and abilityInfo not null.
    // Return AppRunningRecord pointer if success get or create.
    // If error occurs, error code is in |result|

    /**
     * GetOrCreateAppRunningRecord, Get or create application record information.
     *
     * @param token, the unique identification to the ability.
     * @param abilityInfo, ability information.
     * @param appInfo, app information.
     * @param processName, the app process name.
     * @param uid, app uid in Application record.
     * @param result, If error occurs, error code is in |result|.
     *
     * @return AppRunningRecord pointer if success get or create.
     */
    std::shared_ptr<AppRunningRecord> GetOrCreateAppRunningRecord(const sptr<IRemoteObject> &token,
        const std::shared_ptr<ApplicationInfo> &appInfo, const std::shared_ptr<AbilityInfo> &abilityInfo,
        const std::string &processName, const int32_t uid, RecordQueryResult &result);

    std::shared_ptr<AppRunningRecord> GetOrCreateAppRunningRecord(const ApplicationInfo &appInfo, bool &appExist);
    /**
     * OnStop, Application management service stopped.
     *
     * @return
     */
    void OnStop();

    /**
     * OpenAppSpawnConnection, Open connection with appspwan.
     *
     * @return ERR_OK ,return back success，others fail.
     */
    virtual ErrCode OpenAppSpawnConnection();

    /**
     * CloseAppSpawnConnection, Close connection with appspwan.
     *
     * @return
     */
    virtual void CloseAppSpawnConnection() const;

    /**
     * QueryAppSpawnConnectionState, Query the connection status with appspwan.
     *
     * @return Returns the connection status with appspwan.
     */
    virtual SpawnConnectionState QueryAppSpawnConnectionState() const;

    /**
     * SetAppSpawnClient, Setting the client to connect with appspwan.
     *
     * @param spawnClient, the client to connect with appspwan.
     *
     * @return
     */
    void SetAppSpawnClient(std::shared_ptr<AppSpawnClient> spawnClient);

    // Schedule launch application with specified |appRecord|

    /**
     * LaunchApplication, Notify application to launch application.
     *
     * @param appRecord, the application record.
     *
     * @return
     */
    void LaunchApplication(const std::shared_ptr<AppRunningRecord> &appRecord);

    /**
     * Notice of AddAbilityStageInfo()
     *
     * @param recordId, the application record.
     */
    virtual void AddAbilityStageDone(const int32_t recordId);

    /**
     * GetRecordMap, Get all the ability information in the application record.
     *
     * @return all the ability information in the application record.
     */
    const std::map<const int32_t, const std::shared_ptr<AppRunningRecord>> &GetRecordMap() const;

    // functions to get special AppRunningRecord
    /**
     * GetAppRunningRecordByAppName, Get process record by application name.
     *
     * @param appName, the application name.
     *
     * @return process record.
     */
    std::shared_ptr<AppRunningRecord> GetAppRunningRecordByAppName(const std::string &appName) const;

    /**
     * GetAppRunningRecordByProcessName, Get process record by application name and process name.
     *
     * @param appName, the application name.
     * @param processName, the process name.
     * @param uid, the process uid.
     *
     * @return process record.
     */
    std::shared_ptr<AppRunningRecord> GetAppRunningRecordByProcessName(
        const std::string &appName, const std::string &processName, int32_t uid) const;

    /**
     * GetAppRunningRecordByPid, Get process record by application pid.
     *
     * @param pid, the application pid.
     *
     * @return process record.
     */
    std::shared_ptr<AppRunningRecord> GetAppRunningRecordByPid(const pid_t pid) const;

    /**
     * GetAppRunningRecordByAbilityToken, Get process record by ability token.
     *
     * @param abilityToken, the ability token.
     *
     * @return process record.
     */
    std::shared_ptr<AppRunningRecord> GetAppRunningRecordByAbilityToken(const sptr<IRemoteObject> &abilityToken) const;

    /**
     * GetAppRunningRecordByAppRecordId, Get process record by application id.
     *
     * @param recordId, the application id.
     *
     * @return process record.
     */
    std::shared_ptr<AppRunningRecord> GetAppRunningRecordByAppRecordId(const int32_t recordId) const;

    /**
     * OnAbilityStateChanged, Call ability state change.
     *
     * @param ability, the ability info.
     * @param state, the ability state.
     *
     * @return
     */
    void OnAbilityStateChanged(const std::shared_ptr<AbilityRunningRecord> &ability, const AbilityState state);

    /**
     * GetRecentAppList, Get a list of recent applications.
     *
     * @return a list of recent applications.
     */
    const std::list<const std::shared_ptr<AppTaskInfo>> &GetRecentAppList() const;

    /**
     * GetRecentAppList, Remove the corresponding latest application list data by applying the name.
     *
     * @param appName, the application name.
     * @param processName, the process name.
     *
     * @return
     */
    void RemoveAppFromRecentList(const std::string &appName, const std::string &processName);

    /**
     * GetRecentAppList, Clear recent application list.
     *
     * @return
     */
    void ClearRecentAppList();

    /**
     * OnRemoteDied, Equipment death notification.
     *
     * @param remote, Death client.
     * @return
     */
    void OnRemoteDied(const wptr<IRemoteObject> &remote);

    /**
     * AddAppDeathRecipient, Add monitoring death application record.
     *
     * @param pid, the application pid.
     * @param appDeathRecipient, Application death recipient list.
     *
     * @return
     */

    virtual void AddAppDeathRecipient(const pid_t pid, const sptr<AppDeathRecipient> &appDeathRecipient) const;
    /**
     * ProcessOptimizerInit, Process Optimizer init.
     *
     * @param
     * @return ERR_OK, return back success, others fail.
     */
    virtual int32_t ProcessOptimizerInit();

    /**
     * OptimizerAbilityStateChanged, Optimizer processing ability state changes.
     *
     * @param ability, the ability info.
     * @param state, the ability state before change.
     *
     * @return
     */
    virtual void OptimizerAbilityStateChanged(
        const std::shared_ptr<AbilityRunningRecord> &ability, const AbilityState state);

    /**
     * OptimizerAppStateChanged, Optimizer processing app state changes.
     *
     * @param appRecord, the app information.
     * @param state, the app before change.
     * @return
     */
    virtual void OptimizerAppStateChanged(
        const std::shared_ptr<AppRunningRecord> &appRecord, const ApplicationState state);

    /**
     * SetAppSuspendTimes, Setting the Freezing Time of APP Background.
     *
     * @param time, The timeout(second) recorded when the application enters the background .
     *
     * @return Success or Failure .
     */
    void SetAppFreezingTime(int time);

    /**
     * GetAppFreezingTime, Getting the Freezing Time of APP Background.
     *
     * @param time, The timeout(second) recorded when the application enters the background .
     *
     * @return Success or Failure .
     */
    void GetAppFreezingTime(int &time);
    void HandleTimeOut(const InnerEvent::Pointer &event);

    void SetEventHandler(const std::shared_ptr<AMSEventHandler> &handler);

    void HandleAbilityAttachTimeOut(const sptr<IRemoteObject> &token);

    void PrepareTerminate(const sptr<IRemoteObject> &token);

    /**
     * Checks whether a specified permission has been granted to the process identified by pid and uid
     *
     * @param permission Indicates the permission to check.
     * @param pid Indicates the ID of the process to check.
     * @param uid Indicates the UID of the process to check.
     * @param message Describe success or failure
     *
     * @return Returns ERR_OK on success, others on failure.
     */
    int CompelVerifyPermission(const std::string &permission, int pid, int uid, std::string &message);

    /**
     * SuspendApplication, Application state changed.
     *
     * @param appRecord, the app information.
     * @param state, the app state.
     */
    void OnAppStateChanged(const std::shared_ptr<AppRunningRecord> &appRecord, const ApplicationState state);

    /**
     * Start empty process
     */
    void LoadResidentProcess();

    void StartResidentProcess(const std::vector<BundleInfo> &v);

    bool CheckRemoteClient();

    /**
     * Register application or process state observer.
     * @param observer, ability token.
     * @return Returns ERR_OK on success, others on failure.
     */
    int32_t RegisterApplicationStateObserver(const sptr<IApplicationStateObserver> &observer);

    /**
     * Unregister application or process state observer.
     * @param observer, ability token.
     * @return Returns ERR_OK on success, others on failure.
     */
    int32_t UnregisterApplicationStateObserver(const sptr<IApplicationStateObserver> &observer);

    /**
     * Get Foreground Applications.
     *
     * @return Foreground Applications.
     */
    int32_t GetForegroundApplications(std::vector<AppStateData> &list);

private:

    void StartEmptyResidentProcess(const std::string &appName, const std::string &processName,
        const std::shared_ptr<AppRunningRecord> &appRecord, const int uid);

    void RestartResidentProcess(std::shared_ptr<AppRunningRecord> appRecord);

    /**
     * StartAbility, load the ability that needed to be started(Start on the basis of the original process).
     *  Start on a new boot process
     * @param token, the unique identification to start the ability.
     * @param preToken, the unique identification to call the ability.
     * @param abilityInfo, the ability information.
     * @param appInfo, the app information.
     *
     * @return
     */
    void StartAbility(const sptr<IRemoteObject> &token, const sptr<IRemoteObject> &preToken,
        const std::shared_ptr<AbilityInfo> &abilityInfo, const std::shared_ptr<AppRunningRecord> &appRecord);

    /**
     * UnsuspendApplication, Application process state switch to unsuspend.
     *
     * @param appRecord, the app information.
     *
     * @return
     */
    void UnsuspendApplication(const std::shared_ptr<AppRunningRecord> &appRecord);

    /**
     * SuspendApplication, Application process state switch to suspend.
     *
     * @param appRecord, the app information.
     *
     * @return
     */
    void SuspendApplication(const std::shared_ptr<AppRunningRecord> &appRecord);

    /**
     * LowMemoryApplicationAlert, Application low memory alert.
     *
     * @param appRecord, the app information.
     * @param level, the app low memory level.
     *
     * @return
     */
    void LowMemoryApplicationAlert(
        const std::shared_ptr<AppRunningRecord> &appRecord, const CgroupManager::LowMemoryLevel level);

    /**
     * GetAbilityOwnerApp, Get the process record of ability.
     *
     * @param abilityRecord, the ability information.
     *
     * @return process record.
     */
    std::shared_ptr<AppRunningRecord> GetAbilityOwnerApp(
        const std::shared_ptr<AbilityRunningRecord> &abilityRecord) const;

    /**
     * GetAbilityRunningRecordByAbilityToken, Get the ability record by token.
     *
     * @param abilityToken, the ability token.
     *
     * @return ability record.
     */
    std::shared_ptr<AbilityRunningRecord> GetAbilityRunningRecordByAbilityToken(
        const sptr<IRemoteObject> &abilityToken) const;

    /**
     * StartProcess, load the ability that needed to be started(Start on a new boot process).
     *
     * @param appName, the app name.
     * @param processName, the process name.
     * @param appRecord, the app information.
     * @param uid, the process uid.
     *
     * @return
     */
    void StartProcess(const std::string &appName, const std::string &processName,
        const std::shared_ptr<AppRunningRecord> &appRecord, int32_t uid);

    /**
     * PushAppFront, Adjust the latest application record to the top level.
     *
     * @param recordId, the app record id.
     *
     * @return
     */
    void PushAppFront(const int32_t recordId);

    /**
     * RemoveAppFromRecentListById, Remove the specified recent application record by application record id.
     *
     * @param recordId, the app record id.
     *
     * @return
     */
    void RemoveAppFromRecentListById(const int32_t recordId);

    /**
     * AddAppToRecentList, Add application to recent list.
     *
     * @param appName, the app name.
     * @param processName, the process name.
     * @param pid, the app pid.
     * @param recordId, the app record id.
     *
     * @return
     */
    void AddAppToRecentList(
        const std::string &appName, const std::string &processName, const pid_t pid, const int32_t recordId);

    /**
     * AddAppToRecentList, Get application task information through ID.
     *
     * @param recordId, the app record id.
     *
     * @return application task information.
     */
    const std::shared_ptr<AppTaskInfo> GetAppTaskInfoById(const int32_t recordId) const;

    /**
     * KillProcessByPid, Kill process by PID.
     *
     * @param pid_t, the app record pid.
     *
     * @return ERR_OK, return back success，others fail.
     */
    int32_t KillProcessByPid(const pid_t pid) const;

    /**
     * WaitForRemoteProcessExit, Wait for the process to exit normally.
     *
     * @param pids, process number collection to exit.
     * @param startTime, execution process security exit start time.
     *
     * @return true, return back success，others fail.
     */
    bool WaitForRemoteProcessExit(std::list<pid_t> &pids, const int64_t startTime);

    /**
     * GetAllPids, Get the corresponding pid collection.
     *
     * @param pids, process number collection to exit.
     *
     * @return true, return back success，others fail.
     */
    bool GetAllPids(std::list<pid_t> &pids);

    /**
     * process_exist, Judge whether the process exists.
     *
     * @param pids, process number collection to exit.
     *
     * @return true, return back existed，others non-existent.
     */
    bool process_exist(pid_t &pid);

    /**
     * CheckAllProcessExist, Determine whether all processes exist .
     *
     * @param pids, process number collection to exit.
     *
     * @return true, Returns that a process exists and all other processes do not exist.
     */
    bool CheckAllProcessExist(std::list<pid_t> &pids);

    /**
     * SystemTimeMillis, Get system time.
     *
     * @return the system time.
     */
    int64_t SystemTimeMillis();

    // Test add the bundle manager instance.
    void SetBundleManager(sptr<IBundleMgr> bundleManager);

    void HandleTerminateApplicationTimeOut(const int64_t eventId);
    
    void HandleAddAbilityStageTimeOut(const int64_t eventId);

    int32_t KillApplicationByUserId(const std::string &bundleName, const int userId);

    AppProcessData WrapAppProcessData(const std::shared_ptr<AppRunningRecord> &appRecord,
        const ApplicationState state);

    AppStateData WrapAppStateData(const std::shared_ptr<AppRunningRecord> &appRecord,
        const ApplicationState state);

    ProcessData WrapProcessData(const std::shared_ptr<AppRunningRecord> &appRecord);

    void AddObserverDeathRecipient(const sptr<IApplicationStateObserver> &observer);

    void RemoveObserverDeathRecipient(const sptr<IApplicationStateObserver> &observer);

    void OnObserverDied(const wptr<IRemoteObject> &remote);

    void HandleObserverDiedTask(const sptr<IRemoteObject> &observer);

    bool ObserverExist(const sptr<IApplicationStateObserver> &observer);

    void OnProcessCreated(const std::shared_ptr<AppRunningRecord> &appRecord);

    void OnProcessDied(const std::shared_ptr<AppRunningRecord> &appRecord);

private:
    /**
     * ClearUpApplicationData, clear the application data.
     *
     * @param bundleName, bundle name in Application record.
     * @param uid, app uid in Application record.
     * @param pid, app pid in Application record.
     * @param userId, userId.
     *
     * @return
     */
    void ClearUpApplicationDataByUserId(const std::string &bundleName,
        int32_t callerUid, pid_t callerPid, const int userId);

private:
    /**
     * Notify application status.
     *
     * @param bundleName Indicates the name of the bundle.
     * @param eventData Indicates the event defined by CommonEventSupport
     *
     * @return
     */
    void NotifyAppStatus(const std::string &bundleName, const std::string &eventData);

    const std::string TASK_ON_CALLBACK_DIED = "OnCallbackDiedTask";
    std::vector<sptr<IApplicationStateObserver>> appStateObservers_;
    std::map<sptr<IRemoteObject>, sptr<IRemoteObject::DeathRecipient>> recipientMap_;
    std::recursive_mutex observerLock_;
    std::vector<const sptr<IAppStateCallback>> appStateCallbacks_;
    std::shared_ptr<AppProcessManager> appProcessManager_;
    std::shared_ptr<RemoteClientManager> remoteClientManager_;
    std::shared_ptr<AppRunningManager> appRunningManager_;
    std::shared_ptr<ProcessOptimizerUBA> processOptimizerUBA_;
    std::shared_ptr<AMSEventHandler> eventHandler_;
    std::mutex serviceLock_;
};
}  // namespace AppExecFwk
}  // namespace OHOS
#endif  // FOUNDATION_APPEXECFWK_SERVICES_APPMGR_INCLUDE_APP_MGR_SERVICE_INNER_H
