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
#include "system_test_ability_util.h"
#include "status_receiver_host.h"
#include "iservice_registry.h"
namespace OHOS {
namespace STABUtil {
using namespace OHOS;
using namespace OHOS::AAFwk;
using namespace OHOS::AppExecFwk;
using namespace OHOS::MTUtil;
using namespace OHOS::STtools;
using namespace OHOS::EventFwk;

std::mutex STAbilityUtil::mutex_;
std::shared_ptr<STAbilityUtil> STAbilityUtil::instance_ = nullptr;

bool STAbilityUtil::PublishEvent(const std::string &eventName, const int &code, const std::string &data)
{
    Want want;
    want.SetAction(eventName);
    CommonEventData commonData;
    commonData.SetWant(want);
    commonData.SetCode(code);
    commonData.SetData(data);
    return CommonEventManager::PublishCommonEvent(commonData);
}

void STAbilityUtil::InstallHaps(vector_str &hapNames)
{
    for (auto hapName : hapNames) {
        Install(hapName);
    }
}

void STAbilityUtil::UninstallBundle(vector_str &bundleNames)
{
    for (auto bundleName : bundleNames) {
        Uninstall(bundleName);
    }
}

void STAbilityUtil::KillService(const std::string &serviceName)
{
    system(("kill -9 $(pidof " + serviceName + ") > /dev/null 2>&1").c_str());
}

void STAbilityUtil::StartService(const std::string &serviceName, const time_t &delay)
{
    system(("/system/bin/" + serviceName + "& > /dev/null 2>&1").c_str());
    std::this_thread::sleep_for(std::chrono::milliseconds(delay));
}

void STAbilityUtil::KillBundleProcess(vector_str &bundleNames)
{
    for (std::string bundleName : bundleNames) {
        KillService(bundleName);
    }
}

sptr<IAbilityManager> STAbilityUtil::GetAbilityManagerService()
{
    sptr<IRemoteObject> abilityMsObj =
        OHOS::DelayedSingleton<SaMgrClient>::GetInstance()->GetSystemAbility(ABILITY_MGR_SERVICE_ID);
    if (abilityMsObj == nullptr) {
        HILOG_ERROR("failed to get ability manager service");
        return nullptr;
    }
    return iface_cast<IAbilityManager>(abilityMsObj);
}

sptr<IAppMgr> STAbilityUtil::GetAppMgrService()
{
    sptr<IRemoteObject> appMsObj =
        OHOS::DelayedSingleton<SaMgrClient>::GetInstance()->GetSystemAbility(APP_MGR_SERVICE_ID);
    if (appMsObj == nullptr) {
        HILOG_ERROR("failed to get app manager service");
        return nullptr;
    }
    return iface_cast<IAppMgr>(appMsObj);
}

ErrCode STAbilityUtil::StartAbility(const Want &want, sptr<IAbilityManager> &abilityMs, const time_t &delay)
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

bool STAbilityUtil::StopAbility(const std::string &eventName, const int &code, const std::string &data)
{
    return PublishEvent(eventName, code, data);
}

ErrCode STAbilityUtil::StopServiceAbility(const Want &want, unsigned int usec)
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

ErrCode STAbilityUtil::ConnectAbility(const Want &want, const sptr<IAbilityConnection> &connect,
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

ErrCode STAbilityUtil::DisconnectAbility(const sptr<IAbilityConnection> &connect, unsigned int usec)
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

Want STAbilityUtil::MakeWant(std::string deviceId, std::string abilityName, std::string bundleName, MAP_STR_STR params)
{
    ElementName element(deviceId, bundleName, abilityName);
    Want want;
    want.SetElement(element);
    for (const auto &param : params) {
        want.SetParam(param.first, param.second);
    }
    return want;
}

Want STAbilityUtil::MakeWant(std::string deviceId, std::string abilityName, std::string bundleName, vector_str params)
{
    ElementName element(deviceId, bundleName, abilityName);
    Want want;
    want.SetElement(element);
    want.SetParam("operator", params);
    return want;
}

ErrCode STAbilityUtil::GetTopAbilityRecordId(int64_t &id, sptr<IAbilityManager> &abilityMs)
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

ErrCode STAbilityUtil::GetRunningProcessInfo(
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

ErrCode STAbilityUtil::KillApplication(const std::string &appName, sptr<IAppMgr> &appMs, const time_t &delay)
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

RunningProcessInfo STAbilityUtil::GetAppProcessInfoByName(
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

int STAbilityUtil::WaitCompleted(Event &event, const std::string &eventName, const int code, const int timeout)
{
    HILOG_INFO("WaitCompleted");
    return event.WaitingMessage(std::to_string(code) + eventName, timeout, false);
}

void STAbilityUtil::Completed(Event &event, const std::string &eventName, const int code)
{
    HILOG_INFO("Completed");
    return event.CompleteMessage(std::to_string(code) + eventName);
}

void STAbilityUtil::Completed(Event &event, const std::string &eventName, const int code, const std::string &data)
{
    HILOG_INFO("STAbilityUtil::Completed");
    return event.CompleteMessage(std::to_string(code) + eventName, data);
}

std::string STAbilityUtil::GetData(Event &event, const std::string &eventName, const int code)
{
    HILOG_INFO("STAbilityUtil::GetData");
    return event.GetData(std::to_string(code) + eventName);
}

void STAbilityUtil::CleanMsg(Event &event)
{
    HILOG_INFO("CleanMsg");
    return event.Clean();
}

std::vector<std::string> STAbilityUtil::SerializationStOperatorToVector(StOperator &ParentOperator)
{
    std::vector<std::string> vectorOperator;
    PushOperatorInVector(vectorOperator, ParentOperator);
    return vectorOperator;
}

void STAbilityUtil::DeserializationStOperatorFromVector(
    StOperator &ParentOperator, std::vector<std::string> &vectorOperator)
{
    PullOperatorFromVector(ParentOperator, vectorOperator);
}

void STAbilityUtil::PushOperatorInVector(std::vector<std::string> &vectorOperator, StOperator &ParentOperator)
{
    vectorOperator.emplace_back(std::to_string(ParentOperator.GetChildOperator().size()));
    vectorOperator.emplace_back(ParentOperator.GetAbilityType());
    vectorOperator.emplace_back(ParentOperator.GetBundleName());
    vectorOperator.emplace_back(ParentOperator.GetAbilityName());
    vectorOperator.emplace_back(ParentOperator.GetOperatorName());
    vectorOperator.emplace_back(ParentOperator.GetMessage());
    for (auto child : ParentOperator.GetChildOperator()) {
        PushOperatorInVector(vectorOperator, *child);
    }
}

void STAbilityUtil::PullOperatorFromVector(StOperator &ParentOperator, std::vector<std::string> &vectorOperator)
{
    int childnum = std::stoi(vectorOperator.front());
    vectorOperator.erase(vectorOperator.begin());
    std::string abilityType = vectorOperator.front();
    vectorOperator.erase(vectorOperator.begin());
    std::string bundleName = vectorOperator.front();
    vectorOperator.erase(vectorOperator.begin());
    std::string abilityName = vectorOperator.front();
    vectorOperator.erase(vectorOperator.begin());
    std::string operatorName = vectorOperator.front();
    vectorOperator.erase(vectorOperator.begin());
    std::string message = vectorOperator.front();
    vectorOperator.erase(vectorOperator.begin());
    ParentOperator.SetAbilityType(abilityType)
        .SetBundleName(bundleName)
        .SetAbilityName(abilityName)
        .SetOperatorName(operatorName)
        .SetMessage(message);
    for (int i = 0; i < childnum; i++) {
        auto child = std::make_shared<StOperator>();
        if (child == nullptr) {
            return;
        }
        ParentOperator.AddChildOperator(child);
        PullOperatorFromVector(*(child.get()), vectorOperator);
    }
}

int STAbilityUtil::RemoveStack(
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

class InstallToolStatusReceiver : public StatusReceiverHost {
public:
    InstallToolStatusReceiver();
    virtual ~InstallToolStatusReceiver() override;
    virtual void OnStatusNotify(const int progress) override;
    virtual void OnFinished(const int32_t resultCode, const std::string &resultMsg) override;
    static int TestWaitCompleted(Event &event, const std::string eventName, const int code, const int timeout = 10);
    static void TestCompleted(Event &event, const std::string &eventName, const int code);
    Event event_ = STtools::Event();

private:
    int iProgress_ = 0;
    DISALLOW_COPY_AND_MOVE(InstallToolStatusReceiver);
};
InstallToolStatusReceiver::InstallToolStatusReceiver()
{
    std::cout << "create status receiver instance" << std::endl;
}

InstallToolStatusReceiver::~InstallToolStatusReceiver()
{
    std::cout << "destroy status receiver instance" << std::endl;
}

void InstallToolStatusReceiver::OnStatusNotify(const int progress)
{
    iProgress_ = progress;
    std::cout << "destroy status receiver instance" << progress << std::endl;
}

void InstallToolStatusReceiver::OnFinished(const int32_t resultCode, const std::string &resultMsg)
{
    std::cout << "on finished result is " << resultCode << " " << resultMsg << std::endl;
    TestCompleted(event_, resultMsg, resultCode);
}

int InstallToolStatusReceiver::TestWaitCompleted(
    Event &event, const std::string eventName, const int code, const int timeout)
{
    std::cout << "TestWaitCompleted " << eventName << std::endl;
    return STAbilityUtil::WaitCompleted(event, eventName, code, timeout);
}
void InstallToolStatusReceiver::TestCompleted(Event &event, const std::string &eventName, const int code)
{
    std::cout << "TestCompleted " << eventName << std::endl;
    STAbilityUtil::Completed(event, eventName, code);
    return;
}

const std::string MSG_SUCCESS = "[SUCCESS]";
void STAbilityUtil::Install(const std::string &bundleFilePath, const InstallFlag installFlag)
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

void STAbilityUtil::Uninstall(const std::string &bundleName)
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

sptr<IBundleInstaller> STAbilityUtil::GetInstallerProxy()
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

sptr<IBundleMgr> STAbilityUtil::GetBundleMgrProxy()
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
}  // namespace STABUtil
}  // namespace OHOS