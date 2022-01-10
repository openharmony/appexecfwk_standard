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

#include "app_mgr_stub.h"

#include "ipc_skeleton.h"
#include "ipc_types.h"
#include "iremote_object.h"

#include "ability_info.h"
#include "app_log_wrapper.h"
#include "app_mgr_proxy.h"
#include "app_scheduler_interface.h"
#include "appexecfwk_errors.h"
#include "bytrace.h"
#include "iapp_state_callback.h"

namespace OHOS {
namespace AppExecFwk {
AppMgrStub::AppMgrStub()
{
    memberFuncMap_[static_cast<uint32_t>(IAppMgr::Message::APP_ATTACH_APPLICATION)] =
        &AppMgrStub::HandleAttachApplication;
    memberFuncMap_[static_cast<uint32_t>(IAppMgr::Message::APP_APPLICATION_FOREGROUNDED)] =
        &AppMgrStub::HandleApplicationForegrounded;
    memberFuncMap_[static_cast<uint32_t>(IAppMgr::Message::APP_APPLICATION_BACKGROUNDED)] =
        &AppMgrStub::HandleApplicationBackgrounded;
    memberFuncMap_[static_cast<uint32_t>(IAppMgr::Message::APP_APPLICATION_TERMINATED)] =
        &AppMgrStub::HandleApplicationTerminated;
    memberFuncMap_[static_cast<uint32_t>(IAppMgr::Message::APP_CHECK_PERMISSION)] =
        &AppMgrStub::HandleCheckPermission;
    memberFuncMap_[static_cast<uint32_t>(IAppMgr::Message::APP_ABILITY_CLEANED)] =
        &AppMgrStub::HandleAbilityCleaned;
    memberFuncMap_[static_cast<uint32_t>(IAppMgr::Message::APP_GET_MGR_INSTANCE)] = &AppMgrStub::HandleGetAmsMgr;
    memberFuncMap_[static_cast<uint32_t>(IAppMgr::Message::APP_CLEAR_UP_APPLICATION_DATA)] =
        &AppMgrStub::HandleClearUpApplicationData;
    memberFuncMap_[static_cast<uint32_t>(IAppMgr::Message::APP_IS_BACKGROUND_RUNNING_RESTRICTED)] =
        &AppMgrStub::HandleIsBackgroundRunningRestricted;
    memberFuncMap_[static_cast<uint32_t>(IAppMgr::Message::APP_GET_ALL_RUNNING_PROCESSES)] =
        &AppMgrStub::HandleGetAllRunningProcesses;
    memberFuncMap_[static_cast<uint32_t>(IAppMgr::Message::APP_SET_APP_FREEZING_TIME)] =
        &AppMgrStub::HandleSetAppFreezingTime;
    memberFuncMap_[static_cast<uint32_t>(IAppMgr::Message::APP_GET_APP_FREEZING_TIME)] =
        &AppMgrStub::HandleGetAppFreezingTime;
    memberFuncMap_[static_cast<uint32_t>(IAppMgr::Message::APP_GET_SYSTEM_MEMORY_ATTR)] =
        &AppMgrStub::HandleGetSystemMemoryAttr;
    memberFuncMap_[static_cast<uint32_t>(IAppMgr::Message::APP_ADD_ABILITY_STAGE_INFO_DONE)] =
        &AppMgrStub::HandleAddAbilityStageDone;
    memberFuncMap_[static_cast<uint32_t>(IAppMgr::Message::STARTUP_RESIDENT_PROCESS)] =
        &AppMgrStub::HandleStartupResidentProcess;
    memberFuncMap_[static_cast<uint32_t>(IAppMgr::Message::REGISTER_APPLICATION_STATE_OBSERVER)] =
        &AppMgrStub::HandleRegisterApplicationStateObserver;
    memberFuncMap_[static_cast<uint32_t>(IAppMgr::Message::UNREGISTER_APPLICATION_STATE_OBSERVER)] =
        &AppMgrStub::HandleUnregisterApplicationStateObserver;
    memberFuncMap_[static_cast<uint32_t>(IAppMgr::Message::GET_FOREGROUND_APPLICATIONS)] =
        &AppMgrStub::HandleGetForegroundApplications;
}

AppMgrStub::~AppMgrStub()
{
    memberFuncMap_.clear();
}

int AppMgrStub::OnRemoteRequest(uint32_t code, MessageParcel &data, MessageParcel &reply, MessageOption &option)
{
    APP_LOGI("AppMgrStub::OnReceived, code = %{public}d, flags= %{public}d.", code, option.GetFlags());
    std::u16string descriptor = AppMgrStub::GetDescriptor();
    std::u16string remoteDescriptor = data.ReadInterfaceToken();
    if (descriptor != remoteDescriptor) {
        APP_LOGE("local descriptor is not equal to remote");
        return ERR_INVALID_STATE;
    }

    auto itFunc = memberFuncMap_.find(code);
    if (itFunc != memberFuncMap_.end()) {
        auto memberFunc = itFunc->second;
        if (memberFunc != nullptr) {
            return (this->*memberFunc)(data, reply);
        }
    }
    return IPCObjectStub::OnRemoteRequest(code, data, reply, option);
}

int32_t AppMgrStub::HandleAttachApplication(MessageParcel &data, MessageParcel &reply)
{
    BYTRACE(BYTRACE_TAG_APP);
    sptr<IRemoteObject> client = data.ReadParcelable<IRemoteObject>();
    AttachApplication(client);
    return NO_ERROR;
}

int32_t AppMgrStub::HandleApplicationForegrounded(MessageParcel &data, MessageParcel &reply)
{
    BYTRACE(BYTRACE_TAG_APP);
    ApplicationForegrounded(data.ReadInt32());
    return NO_ERROR;
}

int32_t AppMgrStub::HandleApplicationBackgrounded(MessageParcel &data, MessageParcel &reply)
{
    BYTRACE(BYTRACE_TAG_APP);
    ApplicationBackgrounded(data.ReadInt32());
    return NO_ERROR;
}

int32_t AppMgrStub::HandleApplicationTerminated(MessageParcel &data, MessageParcel &reply)
{
    BYTRACE(BYTRACE_TAG_APP);
    ApplicationTerminated(data.ReadInt32());
    return NO_ERROR;
}

int32_t AppMgrStub::HandleCheckPermission(MessageParcel &data, MessageParcel &reply)
{
    BYTRACE(BYTRACE_TAG_APP);
    int32_t recordId = data.ReadInt32();
    std::string permission = data.ReadString();
    int32_t result = CheckPermission(recordId, permission);
    reply.WriteInt32(result);
    return NO_ERROR;
}

int32_t AppMgrStub::HandleAbilityCleaned(MessageParcel &data, MessageParcel &reply)
{
    BYTRACE(BYTRACE_TAG_APP);
    sptr<IRemoteObject> token = data.ReadParcelable<IRemoteObject>();
    AbilityCleaned(token);
    return NO_ERROR;
}

int32_t AppMgrStub::HandleGetAmsMgr(MessageParcel &data, MessageParcel &reply)
{
    BYTRACE(BYTRACE_TAG_APP);
    int32_t result = NO_ERROR;
    sptr<IAmsMgr> amsMgr = GetAmsMgr();
    if (!amsMgr) {
        APP_LOGE("abilitymgr instance is nullptr");
        result = ERR_NO_INIT;
    } else {
        if (!reply.WriteParcelable(amsMgr->AsObject())) {
            APP_LOGE("failed to reply abilitymgr instance to client, for write parcel error");
            result = ERR_APPEXECFWK_PARCEL_ERROR;
        }
    }
    reply.WriteInt32(result);
    return NO_ERROR;
}

int32_t AppMgrStub::HandleClearUpApplicationData(MessageParcel &data, MessageParcel &reply)
{
    BYTRACE(BYTRACE_TAG_APP);
    std::string bundleName = data.ReadString();
    int32_t result = ClearUpApplicationData(bundleName);
    reply.WriteInt32(result);
    return NO_ERROR;
}

int32_t AppMgrStub::HandleIsBackgroundRunningRestricted(MessageParcel &data, MessageParcel &reply)
{
    BYTRACE(BYTRACE_TAG_APP);
    std::string bundleName = data.ReadString();
    int32_t result = IsBackgroundRunningRestricted(bundleName);
    reply.WriteInt32(result);
    return NO_ERROR;
}

int32_t AppMgrStub::HandleGetAllRunningProcesses(MessageParcel &data, MessageParcel &reply)
{
    BYTRACE(BYTRACE_TAG_APP);
    std::vector<RunningProcessInfo> info;
    auto result = GetAllRunningProcesses(info);
    reply.WriteInt32(info.size());
    for (auto &it : info) {
        if (!reply.WriteParcelable(&it)) {
            return ERR_INVALID_VALUE;
        }
    }
    if (!reply.WriteInt32(result)) {
        return ERR_INVALID_VALUE;
    }
    return NO_ERROR;
}

int32_t AppMgrStub::HandleSetAppFreezingTime(MessageParcel &data, MessageParcel &reply)
{
    BYTRACE(BYTRACE_TAG_APP);
    SetAppFreezingTime(data.ReadInt32());
    return NO_ERROR;
}

int32_t AppMgrStub::HandleGetAppFreezingTime(MessageParcel &data, MessageParcel &reply)
{
    BYTRACE(BYTRACE_TAG_APP);
    int time = 0;
    GetAppFreezingTime(time);
    reply.WriteInt32(time);
    return NO_ERROR;
}

int32_t AppMgrStub::HandleGetSystemMemoryAttr(MessageParcel &data, MessageParcel &reply)
{
    BYTRACE(BYTRACE_TAG_APP);
    SystemMemoryAttr memoryInfo;
    std::string strConfig;
    data.ReadString(strConfig);
    GetSystemMemoryAttr(memoryInfo, strConfig);
    if (reply.WriteParcelable(&memoryInfo)) {
        APP_LOGE("want write failed.");
        return ERR_INVALID_VALUE;
    }
    return NO_ERROR;
}

int32_t AppMgrStub::HandleAddAbilityStageDone(MessageParcel &data, MessageParcel &reply)
{
    int32_t recordId = data.ReadInt32();
    AddAbilityStageDone(recordId);
    return NO_ERROR;
}

int32_t AppMgrStub::HandleStartupResidentProcess(MessageParcel &data, MessageParcel &reply)
{
    StartupResidentProcess();
    return NO_ERROR;
}

int32_t AppMgrStub::HandleRegisterApplicationStateObserver(MessageParcel &data, MessageParcel &reply)
{
    auto callback = iface_cast<AppExecFwk::IApplicationStateObserver>(data.ReadParcelable<IRemoteObject>());
    int32_t result = RegisterApplicationStateObserver(callback);
    reply.WriteInt32(result);
    return NO_ERROR;
}

int32_t AppMgrStub::HandleUnregisterApplicationStateObserver(MessageParcel &data, MessageParcel &reply)
{
    auto callback = iface_cast<AppExecFwk::IApplicationStateObserver>(data.ReadParcelable<IRemoteObject>());
    int32_t result = UnregisterApplicationStateObserver(callback);
    reply.WriteInt32(result);
    return NO_ERROR;
}


int32_t AppMgrStub::HandleGetForegroundApplications(MessageParcel &data, MessageParcel &reply)
{
    std::vector<AppStateData> appStateDatas;
    int32_t result = GetForegroundApplications(appStateDatas);
    reply.WriteInt32(appStateDatas.size());
    for (auto &it : appStateDatas) {
        if (!reply.WriteParcelable(&it)) {
            return ERR_INVALID_VALUE;
        }
    }
    if (!reply.WriteInt32(result)) {
        return ERR_INVALID_VALUE;
    }
    return result;
}

int32_t AppMgrStub::RegisterApplicationStateObserver(const sptr<IApplicationStateObserver> &observer)
{
    return NO_ERROR;
}

int32_t AppMgrStub::UnregisterApplicationStateObserver(const sptr<IApplicationStateObserver> &observer)
{
    return NO_ERROR;
}

int32_t AppMgrStub::GetForegroundApplications(std::vector<AppStateData> &list)
{
    return NO_ERROR;
}
}  // namespace AppExecFwk
}  // namespace OHOS
