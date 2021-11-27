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

#ifndef FOUNDATION_APPEXECFWK_INTERFACES_INNERKITS_APPEXECFWK_CORE_INCLUDE_APP_MGR_CLIENT_H
#define FOUNDATION_APPEXECFWK_INTERFACES_INNERKITS_APPEXECFWK_CORE_INCLUDE_APP_MGR_CLIENT_H

#include "iremote_object.h"
#include "refbase.h"

#include "ability_info.h"
#include "application_info.h"
#include "app_mgr_constants.h"
#include "iapp_state_callback.h"
#include "running_process_info.h"
#include "system_memory_attr.h"

namespace OHOS {
namespace AppExecFwk {
class AppServiceManager;

class AppMgrClient {
public:
    AppMgrClient();
    virtual ~AppMgrClient();

    /**
     * Load ability.
     *
     * @param token, Ability identify.
     * @param abilityInfo, Ability information.
     * @param appInfo, Application information.
     * @return Returns RESULT_OK on success, others on failure.
     */
    virtual AppMgrResultCode LoadAbility(const sptr<IRemoteObject> &token, const sptr<IRemoteObject> &preToken,
        const AbilityInfo &abilityInfo, const ApplicationInfo &appInfo);

    /**
     * Terminate ability.
     *
     * @param token Ability identify.
     *
     * @return Returns RESULT_OK on success, others on failure.
     */
    virtual AppMgrResultCode TerminateAbility(const sptr<IRemoteObject> &token);

    /**
     * Update ability state.
     *
     * @param token Ability identify.
     * @param state Ability running state.
     * @return Returns RESULT_OK on success, others on failure.
     */
    virtual AppMgrResultCode UpdateAbilityState(const sptr<IRemoteObject> &token, const AbilityState state);

    /**
     * Register Application state callback.
     *
     * @param callback IAppStateCallback
     * @return Returns RESULT_OK on success, others on failure.
     */
    virtual AppMgrResultCode RegisterAppStateCallback(const sptr<IAppStateCallback> &callback);

    /**
     * Connect service.
     *
     * @return Returns RESULT_OK on success, others on failure.
     */
    virtual AppMgrResultCode ConnectAppMgrService();

    /**
     * Ability manager resst.
     *
     * @return Returns RESULT_OK on success, others on failure.
     */
    virtual AppMgrResultCode Reset();

    /**
     * AbilityBehaviorAnalysis, ability behavior analysis assistant process optimization.
     *
     * @param token, the unique identification to start the ability.
     * @param preToken, the unique identification to call the ability.
     * @param visibility, the visibility information about windows info.
     * @param perceptibility, the Perceptibility information about windows info.
     * @param connectionState, the service ability connection state.
     * @return Returns RESULT_OK on success, others on failure.
     */
    virtual AppMgrResultCode AbilityBehaviorAnalysis(const sptr<IRemoteObject> &token,
        const sptr<IRemoteObject> &preToken, const int32_t visibility, const int32_t perceptibility,
        const int32_t connectionState);

    /**
     * KillProcessByAbilityToken, call KillProcessByAbilityToken() through proxy object,
     * kill the process by ability token.
     *
     * @param token, the unique identification to the ability.
     * @return Returns RESULT_OK on success, others on failure.
     */
    virtual AppMgrResultCode KillProcessByAbilityToken(const sptr<IRemoteObject> &token);

    /**
     * KillApplication, call KillApplication() through proxy object, kill the application.
     *
     * @param  bundleName, bundle name in Application record.
     * @return ERR_OK, return back success, others fail.
     */
    virtual AppMgrResultCode KillApplication(const std::string &bundleName);

    /**
     * ClearUpApplicationData, call ClearUpApplicationData() through proxy project,
     * clear the application data.
     *
     * @param bundleName, bundle name in Application record.
     * @return
     */
    virtual AppMgrResultCode ClearUpApplicationData(const std::string &bundleName);

    /**
     * GetAllRunningProcesses, call GetAllRunningProcesses() through proxy project.
     * Obtains information about application processes that are running on the device.
     *
     * @param info, app name in Application record.
     * @return ERR_OK ,return back success，others fail.
     */
    virtual AppMgrResultCode GetAllRunningProcesses(std::vector<RunningProcessInfo> &info);

    /**
     * SetAppSuspendTimes, Setting the Freezing Time of APP Background.
     *
     * @param time, The timeout recorded when the application enters the background .
     *
     * @return Success or Failure .
     */
    virtual AppMgrResultCode SetAppFreezingTime(int time);

    /**
     * GetAppFreezingTime, Getting the Freezing Time of APP Background.
     *
     * @param time, The timeout recorded when the application enters the background .
     *
     * @return Success or Failure .
     */
    virtual AppMgrResultCode GetAppFreezingTime(int &time);
    virtual void AbilityAttachTimeOut(const sptr<IRemoteObject> &token);

    virtual void PrepareTerminate(const sptr<IRemoteObject> &token);

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
    virtual int CompelVerifyPermission(const std::string &permission, int pid, int uid, std::string &message);

    /**
     * Get system memory information.
     * @param SystemMemoryAttr, memory information.
     * @param strConfig, params string.
     */
    virtual void GetSystemMemoryAttr(SystemMemoryAttr &memoryInfo, std::string &strConfig);

private:
    void SetServiceManager(std::unique_ptr<AppServiceManager> serviceMgr);

private:
    std::unique_ptr<AppServiceManager> serviceManager_;
    sptr<IRemoteObject> remote_;
};
}  // namespace AppExecFwk
}  // namespace OHOS
#endif  // FOUNDATION_APPEXECFWK_INTERFACES_INNERKITS_APPEXECFWK_CORE_INCLUDE_APP_MGR_CLIENT_H
