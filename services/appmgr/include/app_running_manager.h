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

#ifndef FOUNDATION_APPEXECFWK_SERVICES_APPMGR_INCLUDE_APP_MGR_RUNNING_MANAGER_H
#define FOUNDATION_APPEXECFWK_SERVICES_APPMGR_INCLUDE_APP_MGR_RUNNING_MANAGER_H

#include <map>
#include <mutex>

#include "iremote_object.h"
#include "refbase.h"

#include "app_running_record.h"
#include "ability_info.h"
#include "application_info.h"
#include "app_state_data.h"
#include "record_query_result.h"

namespace OHOS {
namespace AppExecFwk {
class AppRunningManager {
public:
    AppRunningManager();
    virtual ~AppRunningManager();
    /**
     * GetOrCreateAppRunningRecord, Get or create application record information.
     *
     * @param token, the unique identification to start the ability.
     * @param abilityInfo, ability information.
     * @param appInfo, app information.
     * @param processName, app process name.
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
     * GetAppRunningRecordByAppName, Get process record by application name.
     *
     * @param appName, the application name.
     *
     * @return process record.
     */
    std::shared_ptr<AppRunningRecord> GetAppRunningRecordByAppName(const std::string &appName);

    /**
     * GetAppRunningRecordByProcessName, Get process record by application name and process Name.
     *
     * @param appName, the application name.
     * @param processName, the process name.
     * @param uid, the process uid.
     *
     * @return process record.
     */
    std::shared_ptr<AppRunningRecord> GetAppRunningRecordByProcessName(
        const std::string &appName, const std::string &processName, const int uid);

    /**
     * GetAppRunningRecordByPid, Get process record by application pid.
     *
     * @param pid, the application pid.
     *
     * @return process record.
     */
    std::shared_ptr<AppRunningRecord> GetAppRunningRecordByPid(const pid_t pid);

    /**
     * GetAppRunningRecordByAbilityToken, Get process record by ability token.
     *
     * @param abilityToken, the ability token.
     *
     * @return process record.
     */
    std::shared_ptr<AppRunningRecord> GetAppRunningRecordByAbilityToken(const sptr<IRemoteObject> &abilityToken);

    /**
     * OnRemoteDied, Equipment death notification.
     *
     * @param remote, Death client.
     * @return
     */
    std::shared_ptr<AppRunningRecord> OnRemoteDied(const wptr<IRemoteObject> &remote);

    /**
     * GetAppRunningRecordMap, Get application record list.
     *
     * @return the application record list.
     */
    const std::map<const int32_t, const std::shared_ptr<AppRunningRecord>> &GetAppRunningRecordMap();

    /**
     * RemoveAppRunningRecordById, Remove application information through application id.
     *
     * @param recordId, the application id.
     * @return
     */
    void RemoveAppRunningRecordById(const int32_t recordId);

    /**
     * ClearAppRunningRecordMap, Clear application record list.
     *
     * @return
     */
    void ClearAppRunningRecordMap();

    /**
     * Get Foreground Applications.
     *
     * @return Foreground Applications.
     */
    void GetForegroundApplications(std::vector<AppStateData> &list);

    void HandleTerminateTimeOut(int64_t eventId);
    void HandleAbilityAttachTimeOut(const sptr<IRemoteObject> &token);
    std::shared_ptr<AppRunningRecord> GetAppRunningRecord(const int64_t eventId);
    void TerminateAbility(const sptr<IRemoteObject> &token);
    bool GetPidsByBundleName(const std::string &bundleName, std::list<pid_t> &pids);
    bool GetPidsByBundleNameByUid(const std::string &bundleName, const int uid, std::list<pid_t> &pids);

    void PrepareTerminate(const sptr<IRemoteObject> &token);

    std::shared_ptr<AppRunningRecord> GetTerminatingAppRunningRecord(const sptr<IRemoteObject> &abilityToken);
    bool CanRestartResidentProcCount(const std::string &processName);
    bool InitRestartResidentProcRecord(const std::string &processName);
    std::shared_ptr<AppRunningRecord> GetAppRunningRecordByBundleName(const std::string &bundleName);
private:
    std::shared_ptr<AbilityRunningRecord> GetAbilityRunningRecord(const int64_t eventId);

private:
    std::map<const int32_t, const std::shared_ptr<AppRunningRecord>> appRunningRecordMap_;
    std::map<const std::string, int> processRestartRecord_;
    std::recursive_mutex lock_;
};
}  // namespace AppExecFwk
}  // namespace OHOS

#endif  // FOUNDATION_APPEXECFWK_SERVICES_APPMGR_INCLUDE_APP_MGR_RUNNING_MANAGER_H