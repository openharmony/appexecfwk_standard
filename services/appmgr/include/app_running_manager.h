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
#include <regex>

#include "iremote_object.h"
#include "refbase.h"

#include "app_running_record.h"
#include "ability_info.h"
#include "application_info.h"
#include "app_state_data.h"
#include "record_query_result.h"
#include "bundle_info.h"

namespace OHOS {
namespace AppExecFwk {
class AppRunningManager {
public:
    AppRunningManager();
    virtual ~AppRunningManager();
    /**
     * CreateAppRunningRecord, Get or create application record information.
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
    std::shared_ptr<AppRunningRecord> CreateAppRunningRecord(
        const std::shared_ptr<ApplicationInfo> &appInfo, const std::string &processName, const BundleInfo &bundleInfo);

    /**
     * CheckAppRunningRecordIsExist, Get process record by application name and process Name.
     *
     * @param appName, the application name.
     * @param processName, the process name.
     * @param uid, the process uid.
     *
     * @return process record.
     */
    std::shared_ptr<AppRunningRecord> CheckAppRunningRecordIsExist(const std::string &appName,
        const std::string &processName, const int uid, const BundleInfo &bundleInfo);

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
     * Get the pid of a non-resident process.
     *
     * @return Return true if found, otherwise return false.
     */
    bool ProcessExitByBundleName(const std::string &bundleName, std::list<pid_t> &pids);
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
    bool ProcessExitByBundleNameAndUid(const std::string &bundleName, const int uid, std::list<pid_t> &pids);
    bool GetPidsByUserId(int32_t userId, std::list<pid_t> &pids);

    void PrepareTerminate(const sptr<IRemoteObject> &token);

    std::shared_ptr<AppRunningRecord> GetTerminatingAppRunningRecord(const sptr<IRemoteObject> &abilityToken);
    void ClipStringContent(const std::regex &re, const std::string &sorce, std::string &afferCutStr);
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