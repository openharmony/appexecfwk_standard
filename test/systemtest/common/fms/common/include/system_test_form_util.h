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

#ifndef SYSTEM_TEST_FORM_UTIL_H
#define SYSTEM_TEST_FORM_UTIL_H

#include <memory>
#include <cstdio>
#include <thread>
#include <chrono>

#include "ability_manager_service.h"
#include "ability_manager_errors.h"
#include "app_mgr_service.h"
#include "bundle_installer_interface.h"
#include "bundle_mgr_interface.h"
#include "common_event.h"
#include "common_event_manager.h"
#include "form_event.h"
#include "form_mgr_interface.h"
#include "hilog_wrapper.h"
#include "iremote_proxy.h"
#include "sa_mgr_client.h"
#include "system_ability_definition.h"

namespace OHOS {
namespace STtools {
namespace {
using vector_str = std::vector<std::string>;
using MAP_STR_STR = std::map<std::string, std::string>;
}  // namespace
class SystemTestFormUtil {
public:
    SystemTestFormUtil() = default;
    ~SystemTestFormUtil() = default;

    static std::shared_ptr<SystemTestFormUtil> GetInstance();
    static void DestroyInstance();

    /**
     *
     * @param  {string} eventName                  : Sent FormEvent Name
     * @param  {int} code                  : Sent Code
     * @param  {string} data                  : Sent Data
     * @Introduction: Sent FormEvent
     */
    static bool PublishEvent(const std::string &eventName, const int &code, const std::string &data);

    /**
     *
     * @param  {vector<string>} hapNames                  : container containing multiple hap names
     * @Introduction: use bm install multiple hap
     */
    static void InstallHaps(vector_str &hapNames);

    /**
     *
     * @param  {vector<string>} bundleNames                  : container containing multiple bundle names
     * @Introduction: use bm install multiple bundle
     */
    static void UninstallBundle(vector_str &bundleNames);

    /**
     *
     * @param  {string} serviceName                  : process name
     * @Introduction: kill process
     */
    static void KillService(const std::string &serviceName);

    /**
     *
     * @param  {string} serviceName                  : executable file name
     * @param  {time_t} delay                  : Waiting time for executable to start(milliseconds)
     * @Introduction: start executable file
     */
    static void StartService(const std::string &serviceName, const time_t &delay = 0);

    /**
     *
     * @param  {vector<string>} bundleNames                  : Container Containing Multiple Bundle Names
     * @Introduction: start executable file
     */
    static void KillBundleProcess(vector_str &bundleNames);

    /**
     *
     * @Introduction: Get Ability manager Service.
     */
    static sptr<AAFwk::IAbilityManager> GetAbilityManagerService();

    /**
     *
     * @Introduction: Get App manager Service.
     */
    static sptr<AppExecFwk::IAppMgr> GetAppMgrService();

    /**
     *
     * @param  {Want} want                  : The want of the ability to start.
     * @param  {sptr<AAFwk::IAbilityManager>} abilityMs                  : Ability Manager Service ptr
     * @param  {time_t} delay                  : Waiting time for ability to start (milliseconds)
     * @Introduction: start ability
     */
    static ErrCode StartAbility(
        const AAFwk::Want &want, sptr<AAFwk::IAbilityManager> &abilityMs, const time_t &delay = 0);

    /**
     *
     * @param  {string} eventName                  : Sent FormEvent Name
     * @param  {int} code                  : Sent Code
     * @param  {string} data                  : Sent Data
     * @Introduction: Sent FormEvent to terminate app
     */
    static bool StopAbility(const std::string &eventName, const int &code, const std::string &data);
    /**
     *
     * @param  {string} deviceId                  : Device ID
     * @param  {string} abilityName                  : Ability Name
     * @param  {string} bundleName                  : Bundle Name
     * @param  {MAP_STR_STR} params                  : Params (SetParam)
     * @Introduction: Great Want
     */
    static AAFwk::Want MakeWant(
        std::string deviceId, std::string abilityName, std::string bundleName, MAP_STR_STR params = {});

    /**
     *
     * @param  {string} deviceId                  : Device ID
     * @param  {string} abilityName                  : Ability Name
     * @param  {string} bundleName                  : Bundle Name
     * @param  {vector_str} params                  : Params (SetParam)
     * @Introduction: Great Want
     */
    static AAFwk::Want MakeWant(
        std::string deviceId, std::string abilityName, std::string bundleName, vector_str params = {});

    /**
     *
     * @param  {int64_t} id                  : Ability Record ID
     * @param  {sptr<AAFwk::IAbilityManager>} abilityMs                  : Ability Manager Service ptr
     * @Introduction: Get Top AbilityRecord ID
     */
    static ErrCode GetTopAbilityRecordId(int64_t &id, sptr<AAFwk::IAbilityManager> &abilityMs);

    /**
     *
     * @param  {shared_ptr<RunningProcessInfo>} runningProcessInfo                  : Process Info
     * @param  {sptr<AppExecFwk::IAppMgr>} appMs                  : App Manager ptr
     * @param  {time_t} delay                  : Waiting time for ability to Get Process Info (milliseconds)
     * @Introduction: Get Top AbilityRecord ID
     */
    static ErrCode GetRunningProcessInfo(std::vector<AppExecFwk::RunningProcessInfo> &runningProcessInfo,
        sptr<AppExecFwk::IAppMgr> &appMs, const time_t &delay = 0);

    /**
     *
     * @param  {string} appName                  : app Name
     * @param  {sptr<AppExecFwk::IAppMgr>} appMs                  : App Manager ptr
     * @param  {time_t} delay                  : Waiting time for ability to KillApplication (milliseconds)
     * @Introduction: Get Top AbilityRecord ID
     */
    static ErrCode KillApplication(
        const std::string &appName, sptr<AppExecFwk::IAppMgr> &appMs, const time_t &delay = 0);

    /**
     *
     * @param  {string} processName                  : processName
     * @param  {sptr<AppExecFwk::IAppMgr>} appMs                  : App Manager ptr
     * @param  {time_t} delay                  : Waiting time for ability to GetAppProcessInfo (milliseconds)
     * @Introduction: Get Top AbilityRecord ID
     */
    static AppExecFwk::RunningProcessInfo GetAppProcessInfoByName(
        const std::string &processName, sptr<AppExecFwk::IAppMgr> &appMs, const time_t &delay = 0);

    /**
     *
     * @param  {FormEvent} event                   : FormEvent Class Object
     * @param  {string} eventName                  : The name of the event to wait for.
     * @param  {int} code                  : The code of the event to wait for.
     * @param  {int} timeout                  : Time of wait (seconds)
     * @Introduction: Judge whether the event is received in the event queue, if not, wait
     */
    static int WaitCompleted(
        STtools::FormEvent &event, const std::string &eventName, const int code, const int timeout = 15);

    /**
     *
     * @param  {FormEvent} event                   : FormEvent Class Object
     * @param  {string} eventName                  : The name of the event to wait for.
     * @param  {int} code                  : The code of the event to wait for.
     * @Introduction: Compare the received events, if not, join the event queue.
     */
    static void Completed(STtools::FormEvent &event, const std::string &eventName, const int code);

    /**
     *
     * @param  {FormEvent} event                   : FormEvent Class Object
     * @param  {string} eventName                  : The name of the event to wait for.
     * @param  {int} code                  : The code of the event to wait for.
     * @param  {string} data                  : The data of the event to wait for.
     * @Introduction: Compare the received events, if not, join the event queue.
     */
    static void Completed(STtools::FormEvent &event, const std::string &eventName, const int code,
    const std::string &data);

    /**
     *
     * @param  {FormEvent} event                   : FormEvent Class Object
     * @Introduction: Clean wait event.
     */
    static void CleanMsg(STtools::FormEvent &event);

    /**
     *
     * @param  {FormEvent} event                   : FormEvent Class Object
     * @param  {string} eventName                  : The name of the event to wait for.
     * @param  {int} code                  : The code of the event to wait for.
     * @Introduction: get the event data.
     */
    static std::string GetData(STtools::FormEvent &event, const std::string &eventName, const int code);

    /**
     *
     * @param  {Want} want                  : The want of the ability to start.
     * @param  {sptr<IAbilityConnection>} connect                  : Callback
     * @param  {time_t} delay                  : Waiting time for ability to start (milliseconds)
     * @Introduction: connect ability
     */
    static ErrCode ConnectAbility(const AAFwk::Want &want, const sptr<AAFwk::IAbilityConnection> &connect,
        const sptr<IRemoteObject> &callerToken, unsigned int usec = 0);

    /**
     *
     * @param  {Want} want                  : The want of the ability to start.
     * @param  {sptr<IAbilityConnection>} connect                  : Callback
     * @param  {time_t} delay                  : Waiting time for ability to start (milliseconds)
     * @Introduction: disconnect ability
     */
    static ErrCode DisconnectAbility(const sptr<AAFwk::IAbilityConnection> &connect, unsigned int usec = 0);

    /**
     *
     * @param  {Want} want                  : The want of the ability to start.
     * @param  {time_t} delay                  : Waiting time for ability to start (milliseconds)
     * @Introduction: stop service ability
     */
    static ErrCode StopServiceAbility(const AAFwk::Want &want, unsigned int usec = 0);

    /**
     * Remove the specified mission stack by stack id
     *
     * @param id.
     * @param  {sptr<AAFwk::IAbilityManager>} abilityMs                  : Ability Manager Service ptr
     * @param  {time_t} delay                  : Waiting time for ability to GetAppProcessInfo (milliseconds)
     * @return Returns ERR_OK on success, others on failure.
     */
    static int RemoveStack(int id, sptr<AAFwk::IAbilityManager> &abilityMs, const time_t &backHmoeDelay = 0,
        const time_t &removeDelay = 0);

    /**
     *
     * @param  {string} bundleFilePath                  : Hap File Name
     * @param  {AppExecFwk::InstallFlag} installFlag                  : install flag
     * @Introduction: use bms install
     */
    static void Install(
        const std::string &bundleFilePath, const AppExecFwk::InstallFlag installFlag = AppExecFwk::InstallFlag::NORMAL);

    /**
     *
     * @param  {string} bundleName                  : bundleName
     * @Introduction: use bm uninstall bundleName
     */
    static void Uninstall(const std::string &bundleName);

    /**
     * @brief Query all of form storage infos.
     * @return Returns all of form storage infos.
     */
    static std::string QueryStorageFormInfos();
    /**
     * @brief Query form infos by bundleName.
     * @param bundleName BundleName.
     * @return Returns form infos.
     */
    static std::string QueryFormInfoByBundleName(const std::string& bundleName);
    /**
     * @brief Query form infos by form id.
     * @param formId The id of the form.
     * @return form infos.
     */
    static std::string QueryFormInfoByFormId(const std::int64_t formId);
    /**
     * @brief Query form timer service by form id.
     * @param formId The id of the form.
     * @return Returns form timer service.
     */
    static std::string QueryFormTimerByFormId(const std::int64_t formId);

    /**
    * @brief Batch add forms to form records for st limit value test.
    * @param want The want of the form to add.
    * @return Returns forms count to add.
    */
    static int BatchAddFormRecords(const AAFwk::Want &want);
    /**
    * @brief Clear form records for st limit value test.
    * @return Returns forms count to delete.
    */
    static int ClearFormRecords();
private:
    /**
     *
     * @Introduction: get bundleManger object
     */
    static sptr<AppExecFwk::IBundleMgr> GetBundleMgrProxy();

    /**
     *
     * @Introduction: get bundleInstaller object
     */
    static sptr<AppExecFwk::IBundleInstaller> GetInstallerProxy();

    /**
     * @brief Get form manager service.
     * @return Returns form manager servic remoteObject.
     */
    static sptr<IRemoteObject> GetFmsService();

    static bool WriteInterfaceToken(MessageParcel &data);
    static int GetStringInfo(AppExecFwk::IFormMgr::Message code, MessageParcel &data, std::string &stringInfo);
    static int SendTransactCmd(AppExecFwk::IFormMgr::Message code, MessageParcel &data, MessageParcel &reply);

    static std::shared_ptr<SystemTestFormUtil> instance_;
};
}  // namespace STtools
}  // namespace OHOS
#endif  // SYSTEM_TEST_FORM_UTIL_H
