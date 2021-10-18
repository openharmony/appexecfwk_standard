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
#include "system_test_form_util.h"
#include "iservice_registry.h"
#include "status_receiver_host.h"
#include "install_tool_status_receiver.h"

namespace OHOS {
namespace STtools {
using namespace OHOS;
using namespace OHOS::AAFwk;
using namespace OHOS::AppExecFwk;
using namespace OHOS::EventFwk;

std::shared_ptr<SystemTestFormUtil> SystemTestFormUtil::instance_ = nullptr;

bool SystemTestFormUtil::PublishEvent(const std::string &eventName, const int &code, const std::string &data)
{
    Want want;
    want.SetAction(eventName);
    CommonEventData commonData;
    commonData.SetWant(want);
    commonData.SetCode(code);
    commonData.SetData(data);
    return CommonEventManager::PublishCommonEvent(commonData);
}

void SystemTestFormUtil::InstallHaps(vector_str &hapNames)
{
    for (auto hapName : hapNames) {
        Install(hapName);
    }
}

void SystemTestFormUtil::UninstallBundle(vector_str &bundleNames)
{
    for (auto bundleName : bundleNames) {
        Uninstall(bundleName);
    }
}

void SystemTestFormUtil::KillService(const std::string &serviceName)
{
    system(("kill -9 $(pidof " + serviceName + ") > /dev/null 2>&1").c_str());
}

void SystemTestFormUtil::StartService(const std::string &serviceName, const time_t &delay)
{
    system(("/system/bin/" + serviceName + "& > /dev/null 2>&1").c_str());
    std::this_thread::sleep_for(std::chrono::milliseconds(delay));
}

void SystemTestFormUtil::KillBundleProcess(vector_str &bundleNames)
{
    for (std::string bundleName : bundleNames) {
        KillService(bundleName);
    }
}

sptr<IAbilityManager> SystemTestFormUtil::GetAbilityManagerService()
{
    sptr<IRemoteObject> abilityMsObj =
        OHOS::DelayedSingleton<SaMgrClient>::GetInstance()->GetSystemAbility(ABILITY_MGR_SERVICE_ID);
    if (abilityMsObj == nullptr) {
        HILOG_ERROR("failed to get ability manager service");
        return nullptr;
    }
    return iface_cast<IAbilityManager>(abilityMsObj);
}

sptr<IAppMgr> SystemTestFormUtil::GetAppMgrService()
{
    sptr<IRemoteObject> appMsObj =
        OHOS::DelayedSingleton<SaMgrClient>::GetInstance()->GetSystemAbility(APP_MGR_SERVICE_ID);
    if (appMsObj == nullptr) {
        HILOG_ERROR("failed to get app manager service");
        return nullptr;
    }
    return iface_cast<IAppMgr>(appMsObj);
}

ErrCode SystemTestFormUtil::StartAbility(const Want &want, sptr<IAbilityManager> &abilityMs, const time_t &delay)
{
    ErrCode result = OHOS::ERR_OK;
    abilityMs = GetAbilityManagerService();
    if (abilityMs == nullptr) {
        result = OHOS::ERR_INVALID_VALUE;
        HILOG_ERROR("failed to get ability manager service");
        return result;
    }

    result = abilityMs->StartAbility(want);
    std::this_thread::sleep_for(std::chrono::milliseconds(delay));
    if (result == OHOS::ERR_OK) {
        HILOG_INFO("start ability successfully.");
    } else {
        HILOG_INFO("failed to start ability.");
    }

    return result;
}

bool SystemTestFormUtil::StopAbility(const std::string &eventName, const int &code, const std::string &data)
{
    return PublishEvent(eventName, code, data);
}

ErrCode SystemTestFormUtil::StopServiceAbility(const Want &want, unsigned int usec)
{
    ErrCode result = OHOS::ERR_OK;

    sptr<IAbilityManager> abilityMs = GetAbilityManagerService();
    if (abilityMs == nullptr) {
        result = OHOS::ERR_INVALID_VALUE;
        HILOG_ERROR("failed to get ability manager service");
        return result;
    }

    result = abilityMs->StopServiceAbility(want);
    if (result == OHOS::ERR_OK) {
        HILOG_INFO("stop service ability successfully.");
    } else {
        HILOG_INFO("failed to stop service ability.");
    }

    return result;
}

ErrCode SystemTestFormUtil::ConnectAbility(const Want &want, const sptr<IAbilityConnection> &connect,
    const sptr<IRemoteObject> &callerToken, unsigned int usec)
{
    ErrCode result = OHOS::ERR_OK;

    sptr<IAbilityManager> abilityMs = GetAbilityManagerService();
    if (abilityMs == nullptr) {
        result = OHOS::ERR_INVALID_VALUE;
        HILOG_ERROR("failed to get ability manager service");
        return result;
    }

    result = abilityMs->ConnectAbility(want, connect, callerToken);
    if (result == OHOS::ERR_OK) {
        HILOG_INFO("connect ability successfully.");
    } else {
        HILOG_INFO("failed to connect ability.");
    }

    return result;
}

ErrCode SystemTestFormUtil::DisconnectAbility(const sptr<IAbilityConnection> &connect, unsigned int usec)
{
    ErrCode result = OHOS::ERR_OK;

    sptr<IAbilityManager> abilityMs = GetAbilityManagerService();
    if (abilityMs == nullptr) {
        result = OHOS::ERR_INVALID_VALUE;
        HILOG_ERROR("failed to get ability manager service");
        return result;
    }

    result = abilityMs->DisconnectAbility(connect);
    if (result == OHOS::ERR_OK) {
        HILOG_INFO("StopServiceAbility successfully.");
    } else {
        HILOG_INFO("failed to StopServiceAbility.");
    }

    return result;
}

Want SystemTestFormUtil::MakeWant(std::string deviceId, std::string abilityName, std::string bundleName,
    MAP_STR_STR params)
{
    ElementName element(deviceId, bundleName, abilityName);
    Want want;
    want.SetElement(element);
    for (const auto &param : params) {
        want.SetParam(param.first, param.second);
    }
    return want;
}

Want SystemTestFormUtil::MakeWant(std::string deviceId, std::string abilityName, std::string bundleName,
    vector_str params)
{
    ElementName element(deviceId, bundleName, abilityName);
    Want want;
    want.SetElement(element);
    want.SetParam("operator", params);
    return want;
}

ErrCode SystemTestFormUtil::GetTopAbilityRecordId(int64_t &id, sptr<IAbilityManager> &abilityMs)
{
    ErrCode result = OHOS::ERR_OK;
    id = -1;
    abilityMs = GetAbilityManagerService();
    if (abilityMs == nullptr) {
        result = OHOS::ERR_INVALID_VALUE;
        HILOG_ERROR("failed to get ability manager service");
        return result;
    }
    StackInfo stackInfo;
    abilityMs->GetAllStackInfo(stackInfo);
    MissionStackInfo defaultMissionStack;
    for (const auto &stackInfo : stackInfo.missionStackInfos) {
        if (stackInfo.id == 1) {  // DEFAULT_MISSION_STACK_ID = 1
            defaultMissionStack = stackInfo;
            break;
        }
    }
    if (!defaultMissionStack.missionRecords.empty() &&
        !defaultMissionStack.missionRecords.begin()->abilityRecordInfos.empty()) {
        id = defaultMissionStack.missionRecords.begin()->abilityRecordInfos.begin()->id;
    }
    return result;
}

ErrCode SystemTestFormUtil::GetRunningProcessInfo(
    std::vector<RunningProcessInfo> &runningProcessInfo, sptr<IAppMgr> &appMs, const time_t &delay)
{
    ErrCode result = ERR_OK;
    appMs = GetAppMgrService();
    if (appMs == nullptr) {
        result = OHOS::ERR_INVALID_VALUE;
        HILOG_ERROR("failed to get app manager service");
        return result;
    }
    std::this_thread::sleep_for(std::chrono::milliseconds(delay));
    result = appMs->GetAllRunningProcesses(runningProcessInfo);
    if (result == ERR_OK) {
        HILOG_INFO("get running process info successfully.");
    } else {
        HILOG_INFO("failed to get running process info.");
    }
    return result;
}

ErrCode SystemTestFormUtil::KillApplication(const std::string &appName, sptr<IAppMgr> &appMs, const time_t &delay)
{
    ErrCode result = ERR_OK;
    appMs = GetAppMgrService();
    if (appMs == nullptr) {
        result = OHOS::ERR_INVALID_VALUE;
        HILOG_ERROR("failed to get app manager service");
        return result;
    }
    result = appMs->GetAmsMgr()->KillApplication(appName);
    std::this_thread::sleep_for(std::chrono::milliseconds(delay));
    if (result == ERR_OK) {
        HILOG_INFO("kill application:%{public}s successfully.", appName.c_str());
    } else {
        HILOG_INFO("failed to kill application:%{public}s.", appName.c_str());
    }
    return result;
}

RunningProcessInfo SystemTestFormUtil::GetAppProcessInfoByName(
    const std::string &processName, sptr<IAppMgr> &appMs, const time_t &delay)
{
    RunningProcessInfo appProcessInfo;
    appProcessInfo.pid_ = 0;
    std::vector<RunningProcessInfo> runningProcessInfo;
    if (ERR_OK == GetRunningProcessInfo(runningProcessInfo, appMs, delay)) {
        for (const auto &info : runningProcessInfo) {
            if (processName == info.processName_) {
                appProcessInfo = info;
            }
        }
    }
    return appProcessInfo;
}

int SystemTestFormUtil::WaitCompleted(FormEvent &event, const std::string &eventName, const int code, const int timeout)
{
    HILOG_INFO("WaitCompleted");
    return event.WaitingMessage(std::to_string(code) + eventName, timeout, false);
}

void SystemTestFormUtil::Completed(FormEvent &event, const std::string &eventName, const int code)
{
    HILOG_INFO("Completed");
    return event.CompleteMessage(std::to_string(code) + eventName);
}

void SystemTestFormUtil::Completed(FormEvent &event, const std::string &eventName, const int code,
    const std::string &data)
{
    HILOG_INFO("SystemTestFormUtil::Completed");
    return event.CompleteMessage(std::to_string(code) + eventName, data);
}

std::string SystemTestFormUtil::GetData(FormEvent &event, const std::string &eventName, const int code)
{
    HILOG_INFO("SystemTestFormUtil::GetData");
    return event.GetData(std::to_string(code) + eventName);
}

void SystemTestFormUtil::CleanMsg(FormEvent &event)
{
    HILOG_INFO("CleanMsg");
    return event.Clean();
}

int SystemTestFormUtil::RemoveStack(
    int id, sptr<AAFwk::IAbilityManager> &abilityMs, const time_t &backHmoeDelay, const time_t &removeDelay)
{
    Want wantEntity;
    wantEntity.AddEntity(Want::FLAG_HOME_INTENT_FROM_SYSTEM);
    StartAbility(wantEntity, abilityMs);
    std::this_thread::sleep_for(std::chrono::milliseconds(backHmoeDelay));
    if (!abilityMs) {
        HILOG_INFO("RemoveStack abilityMs nullptr");
        return OHOS::ERR_INVALID_VALUE;
    }
    int result = abilityMs->RemoveStack(id);
    std::this_thread::sleep_for(std::chrono::milliseconds(removeDelay));
    return result;
}

const std::string MSG_SUCCESS = "[SUCCESS]";
void SystemTestFormUtil::Install(const std::string &bundleFilePath, const InstallFlag installFlag)
{
    std::string bundlePath = "/system/vendor/" + bundleFilePath + ".hap";
    std::string installMsg = "";
    sptr<IBundleInstaller> installerProxy = GetInstallerProxy();
    if (!installerProxy) {
        std::cout << "get bundle installer Failure." << std::endl;
        installMsg = "Failure";
        return;
    }

    InstallParam installParam;
    installParam.installFlag = installFlag;
    sptr<InstallToolStatusReceiver> statusReceiver(new (std::nothrow) InstallToolStatusReceiver());
    if (statusReceiver == nullptr) {
        std::cout << "get statusReceiver Failure." << std::endl;
        return;
    }
    bool installResult = installerProxy->Install(bundlePath, installParam, statusReceiver);
    if (!installResult) {
        installMsg = "Failure";
        return;
    }
    if (InstallToolStatusReceiver::TestWaitCompleted(statusReceiver->event_, MSG_SUCCESS, 0) == 0) {
        installMsg = "Success";
    } else {
        installMsg = "Failure";
    }
}

void SystemTestFormUtil::Uninstall(const std::string &bundleName)
{
    std::string uninstallMsg = "";
    sptr<IBundleInstaller> installerProxy = GetInstallerProxy();
    if (!installerProxy) {
        std::cout << "get bundle installer Failure." << std::endl;
        uninstallMsg = "Failure";
        return;
    }
    if (bundleName.empty()) {
        std::cout << "bundelname is null." << std::endl;
        uninstallMsg = "Failure";
    } else {
        sptr<InstallToolStatusReceiver> statusReceiver(new (std::nothrow) InstallToolStatusReceiver());
        if (statusReceiver == nullptr) {
            std::cout << "get statusReceiver Failure." << std::endl;
            uninstallMsg = "Failure";
            return;
        }
        InstallParam installParam;
        bool uninstallResult = installerProxy->Uninstall(bundleName, installParam, statusReceiver);
        if (!uninstallResult) {
            std::cout << "Uninstall Failure." << std::endl;
            uninstallMsg = "Failure";
            return;
        }
        if (InstallToolStatusReceiver::TestWaitCompleted(statusReceiver->event_, MSG_SUCCESS, 0) == 0) {
            uninstallMsg = "Success";
        } else {
            uninstallMsg = "Failure";
        }
    }
}

sptr<IBundleInstaller> SystemTestFormUtil::GetInstallerProxy()
{
    sptr<IBundleMgr> bundleMgrProxy = GetBundleMgrProxy();
    if (!bundleMgrProxy) {
        std::cout << "bundle mgr proxy is nullptr." << std::endl;
        return nullptr;
    }

    sptr<IBundleInstaller> installerProxy = bundleMgrProxy->GetBundleInstaller();
    if (!installerProxy) {
        std::cout << "fail to get bundle installer proxy" << std::endl;
        return nullptr;
    }
    std::cout << "get bundle installer proxy success." << std::endl;
    return installerProxy;
}

sptr<IBundleMgr> SystemTestFormUtil::GetBundleMgrProxy()
{
    sptr<ISystemAbilityManager> systemAbilityManager =
        SystemAbilityManagerClient::GetInstance().GetSystemAbilityManager();
    if (!systemAbilityManager) {
        std::cout << "fail to get system ability mgr." << std::endl;
        return nullptr;
    }

    sptr<IRemoteObject> remoteObject = systemAbilityManager->GetSystemAbility(BUNDLE_MGR_SERVICE_SYS_ABILITY_ID);
    if (!remoteObject) {
        std::cout << "fail to get bundle manager proxy." << std::endl;
        return nullptr;
    }

    std::cout << "get bundle manager proxy success." << std::endl;
    return iface_cast<IBundleMgr>(remoteObject);
}

/**
 * @brief Query all of form storage infos.
 * @return Returns all of form storage infos.
 */
std::string SystemTestFormUtil::QueryStorageFormInfos()
{
    std::cout << "QueryStorageFormInfos called." << std::endl;

    std::string formInfos = "";
    MessageParcel data;
    if (!WriteInterfaceToken(data)) {
        std::cout << "QueryStorageFormInfos, failed to write interface token." << std::endl;
        return "QueryStorageFormInfos, failed to write interface token.";
    }

    int result = GetStringInfo(AppExecFwk::IFormMgr::Message::FORM_MGR_STORAGE_FORM_INFOS, data, formInfos);
    if (result != ERR_OK) {
        std::cout << "QueryStorageFormInfos, failed to query form info." << std::endl;
    }

    std::cout << "QueryStorageFormInfos end." << std::endl;

    return formInfos;
}
/**
 * @brief Query form infos by bundleName.
 * @param bundleName BundleName.
 * @return Returns form infos.
 */
std::string SystemTestFormUtil::QueryFormInfoByBundleName(const std::string& bundleName)
{
    std::cout << "QueryFormInfoByBundleName called." << std::endl;

    std::string formInfos = "";
    MessageParcel data;
    if (!WriteInterfaceToken(data)) {
        std::cout << "QueryFormInfoByBundleName, failed to write interface token." << std::endl;
        return "QueryFormInfoByBundleName, failed to write interface token.";
    }

    if (!data.WriteString(bundleName)) {
        std::cout << "QueryFormInfoByBundleName, failed to write bundleName." << std::endl;
        return "QueryFormInfoByBundleName, failed to write bundleName.";
    }

    int result = GetStringInfo(AppExecFwk::IFormMgr::Message::FORM_MGR_FORM_INFOS_BY_NAME, data, formInfos);
    if (result != ERR_OK) {
        std::cout << "QueryFormInfoByBundleName, failed to query form info." << std::endl;
    }

    std::cout << "QueryFormInfoByBundleName end." << std::endl;

    return formInfos;
}
/**
 * @brief Query form infos by form id.
 * @param formId The id of the form.
 * @return Returns form infos.
 */
std::string SystemTestFormUtil::QueryFormInfoByFormId(const std::int64_t formId)
{
    std::cout << "QueryFormInfoByFormId called." << std::endl;

    std::string formInfo = "";
    MessageParcel data;
    if (!WriteInterfaceToken(data)) {
        std::cout << "QueryFormInfoByFormId, failed to write interface token." << std::endl;
        return "QueryFormInfoByFormId, failed to write interface token.";
    }
    if (!data.WriteInt64(formId)) {
        std::cout << "QueryFormInfoByFormId, failed to write formId." << std::endl;
        return "QueryFormInfoByFormId, failed to write formId.";
    }

    int result = GetStringInfo(AppExecFwk::IFormMgr::Message::FORM_MGR_FORM_INFOS_BY_ID, data, formInfo);
    if (result != ERR_OK) {
        std::cout << "QueryFormInfoByFormId, failed to query form info." << std::endl;
    }

    std::cout << "QueryFormInfoByFormId end." << std::endl;

    return formInfo;
}
/**
 * @brief Query form timer service by form id.
 * @param formId The id of the form.
 * @return Returns form timer service.
 */
std::string SystemTestFormUtil::QueryFormTimerByFormId(const std::int64_t formId)
{
    std::cout << "QueryFormTimerByFormId called." << std::endl;

    std::string isTimingService;
    MessageParcel data;
    if (!WriteInterfaceToken(data)) {
        std::cout << "QueryFormTimerByFormId, failed to write interface token." << std::endl;
        return "QueryFormTimerByFormId, failed to write interface token.";
    }
    if (!data.WriteInt64(formId)) {
        std::cout << "QueryFormTimerByFormId, failed to write formId." << std::endl;
        return "QueryFormTimerByFormId, failed to write formId.";
    }

    int result = GetStringInfo(AppExecFwk::IFormMgr::Message::FORM_MGR_FORM_TIMER_INFO_BY_ID, data, isTimingService);
    if (result != ERR_OK) {
        std::cout << "QueryFormTimerByFormId, failed to query timer info." << std::endl;
    }

    std::cout << "QueryFormTimerByFormId end." << std::endl;

    return isTimingService;
}

/**
 * @brief Batch add forms to form records for st limit value test.
 * @param want The want of the form to add.
 * @return Returns forms count to add.
 */
int SystemTestFormUtil::BatchAddFormRecords(const AAFwk::Want &want)
{
    sptr<IRemoteObject> remoteObject = GetFmsService();
    if (!remoteObject) {
        std::cout << "SendTransactCmd, failed to get remote object." << std::endl;
        return ERR_APPEXECFWK_SERVICE_NOT_CONNECTED;
    }

    MessageParcel data;
    if (!WriteInterfaceToken(data)) {
        std::cout << "BatchAddFormRecords, failed to write interface token." << std::endl;
        return ERR_APPEXECFWK_PARCEL_ERROR;
    }
    if (!data.WriteParcelable(&want)) {
        std::cout << "BatchAddFormRecords, failed to write want." << std::endl;
        return ERR_APPEXECFWK_PARCEL_ERROR;
    }

    MessageParcel reply;
    MessageOption option;
    int error = remoteObject->SendRequest(
        static_cast<uint32_t>(IFormMgr::Message::FORM_MGR_BATCH_ADD_FORM_RECORDS_ST), 
        data, 
        reply, 
        option);
    if (error != ERR_OK) {
        std::cout << "BatchAddFormRecords, failed to SendRequest." << std::endl;
        return ERR_APPEXECFWK_FORM_SEND_FMS_MSG;
    }
    return reply.ReadInt32();
}

/**
 * @brief Clear form records for st limit value test.
 * @return Returns forms count to delete.
 */
int SystemTestFormUtil::ClearFormRecords()
{
    sptr<IRemoteObject> remoteObject = GetFmsService();
    if (!remoteObject) {
        std::cout << "SendTransactCmd, failed to get remote object." << std::endl;
        return ERR_APPEXECFWK_SERVICE_NOT_CONNECTED;
    }

    MessageParcel data;
    if (!WriteInterfaceToken(data)) {
        std::cout << "BatchAddFormRecords, failed to write interface token." << std::endl;
        return ERR_APPEXECFWK_PARCEL_ERROR;
    }
    
    MessageParcel reply;
    MessageOption option;
    int error = remoteObject->SendRequest(
        static_cast<uint32_t>(IFormMgr::Message::FORM_MGR_CLEAR_FORM_RECORDS_ST), 
        data, 
        reply, 
        option);
    if (error != ERR_OK) {
        std::cout << "ClearFormRecords, failed to SendRequest." << std::endl;
        return ERR_APPEXECFWK_FORM_SEND_FMS_MSG;
    }
    return reply.ReadInt32();
}
bool SystemTestFormUtil::WriteInterfaceToken(MessageParcel &data)
{
    if (!data.WriteInterfaceToken(AppExecFwk::IFormMgr::GetDescriptor())) {
        std::cout << "WriteInterfaceToken, failed to write interface token." << std::endl;
        return false;
    }
    return true;
}
int SystemTestFormUtil::GetStringInfo(AppExecFwk::IFormMgr::Message code, MessageParcel &data, std::string &stringInfo)
{
    int error;
    MessageParcel reply;
    error = SendTransactCmd(code, data, reply);
    if (error != ERR_OK) {
        return error;
    }

    error = reply.ReadInt32();
    if (error != ERR_OK) {
        std::cout << "GetStringInfo, failed to read reply result." << std::endl;
        return error;
    }
    std::vector<std::string> stringInfoList;
    if (!reply.ReadStringVector(&stringInfoList)) {
        std::cout << "GetStringInfo, failed to read string vector from reply." << std::endl;
        return ERR_APPEXECFWK_FORM_NOT_EXIST_ID;
    }
    if (stringInfoList.empty()) {
        std::cout << "GetStringInfo, No string info." << std::endl;
        return ERR_APPEXECFWK_FORM_NOT_EXIST_ID;
    }
    for (auto &info : stringInfoList) {
        stringInfo += info;
    }
    std::cout << "GetStringInfo end." << std::endl;
    return ERR_OK;
}
int SystemTestFormUtil::SendTransactCmd(AppExecFwk::IFormMgr::Message code, MessageParcel &data, MessageParcel &reply)
{
    sptr<IRemoteObject> remoteObject = GetFmsService();
    if (!remoteObject) {
        std::cout << "SendTransactCmd, failed to get remote object." << std::endl;
        return ERR_APPEXECFWK_SERVICE_NOT_CONNECTED;
    }

    MessageOption option(MessageOption::TF_SYNC);
    int32_t result = remoteObject->SendRequest(static_cast<uint32_t>(code), data, reply, option);
    if (result != ERR_OK) {
        std::cout << "SendTransactCmd, failed to SendRequest." << std::endl;
        return result;
    }
    return ERR_OK;
}
/**
 * @brief Get form manager service.
 * @return Returns form manager servic remoteObject.
 */
sptr<IRemoteObject> SystemTestFormUtil::GetFmsService()
{
    sptr<ISystemAbilityManager> systemManager = SystemAbilityManagerClient::GetInstance().GetSystemAbilityManager();
    if (systemManager == nullptr) {
        std::cout << "SendTransactCmd, failed to get system ability manager." << std::endl;
        return nullptr;
    }
    sptr<IRemoteObject> remoteObject = systemManager->GetSystemAbility(FORM_MGR_SERVICE_ID);
    
    std::cout << "SendTransactCmd end." << std::endl;
    return remoteObject;
}
}  // namespace STtools
}  // namespace OHOS