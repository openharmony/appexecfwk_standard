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

#include "app_mgr_proxy.h"

#include "app_log_wrapper.h"
#include "appexecfwk_errors.h"

#include "ipc_types.h"
#include "iremote_object.h"

namespace OHOS {
namespace AppExecFwk {

AppMgrProxy::AppMgrProxy(const sptr<IRemoteObject> &impl) : IRemoteProxy<IAppMgr>(impl)
{}

bool AppMgrProxy::WriteInterfaceToken(MessageParcel &data)
{
    if (!data.WriteInterfaceToken(AppMgrProxy::GetDescriptor())) {
        APP_LOGE("write interface token failed");
        return false;
    }
    return true;
}

void AppMgrProxy::AttachApplication(const sptr<IRemoteObject> &obj)
{
    APP_LOGD("AppMgrProxy::AttachApplication start");
    MessageParcel data;
    MessageParcel reply;
    MessageOption option(MessageOption::TF_SYNC);
    if (!WriteInterfaceToken(data)) {
        return;
    }
    data.WriteParcelable(obj.GetRefPtr());
    sptr<IRemoteObject> remote = Remote();
    if (remote == nullptr) {
        APP_LOGE("Remote() is NULL");
        return;
    }
    int32_t ret =
        remote->SendRequest(static_cast<uint32_t>(IAppMgr::Message::AMS_APP_ATTACH_APPLICATION), data, reply, option);
    if (ret != NO_ERROR) {
        APP_LOGW("SendRequest is failed, error code: %{public}d", ret);
    }
    APP_LOGD("end");
}

void AppMgrProxy::ApplicationForegrounded(const int32_t recordId)
{
    APP_LOGD("start");
    MessageParcel data;
    MessageParcel reply;
    MessageOption option(MessageOption::TF_SYNC);
    if (!WriteInterfaceToken(data)) {
        return;
    }
    data.WriteInt32(recordId);
    sptr<IRemoteObject> remote = Remote();
    if (remote == nullptr) {
        APP_LOGE("Remote() is NULL");
        return;
    }
    int32_t ret = remote->SendRequest(
        static_cast<uint32_t>(IAppMgr::Message::AMS_APP_APPLICATION_FOREGROUNDED), data, reply, option);
    if (ret != NO_ERROR) {
        APP_LOGW("SendRequest is failed, error code: %{public}d", ret);
    }
    APP_LOGD("end");
}

void AppMgrProxy::ApplicationBackgrounded(const int32_t recordId)
{
    APP_LOGD("start");
    MessageParcel data;
    MessageParcel reply;
    MessageOption option(MessageOption::TF_SYNC);
    if (!WriteInterfaceToken(data)) {
        return;
    }
    data.WriteInt32(recordId);
    sptr<IRemoteObject> remote = Remote();
    if (remote == nullptr) {
        APP_LOGE("Remote() is NULL");
        return;
    }
    int32_t ret = remote->SendRequest(
        static_cast<uint32_t>(IAppMgr::Message::AMS_APP_APPLICATION_BACKGROUNDED), data, reply, option);
    if (ret != NO_ERROR) {
        APP_LOGW("SendRequest is failed, error code: %{public}d", ret);
    }
    APP_LOGD("end");
}

void AppMgrProxy::ApplicationTerminated(const int32_t recordId)
{
    APP_LOGD("start");
    MessageParcel data;
    MessageParcel reply;
    MessageOption option(MessageOption::TF_SYNC);
    if (!WriteInterfaceToken(data)) {
        return;
    }
    data.WriteInt32(recordId);
    sptr<IRemoteObject> remote = Remote();
    if (remote == nullptr) {
        APP_LOGE("Remote() is NULL");
        return;
    }
    int32_t ret = remote->SendRequest(
        static_cast<uint32_t>(IAppMgr::Message::AMS_APP_APPLICATION_TERMINATED), data, reply, option);
    if (ret != NO_ERROR) {
        APP_LOGW("SendRequest is failed, error code: %{public}d", ret);
    }
    APP_LOGD("end");
}

int32_t AppMgrProxy::CheckPermission(const int32_t recordId, const std::string &permission)
{
    APP_LOGD("start");

    MessageParcel data;
    MessageParcel reply;
    MessageOption option(MessageOption::TF_SYNC);
    if (!WriteInterfaceToken(data)) {
        return ERR_PERMISSION_DENIED;
    }
    data.WriteInt32(recordId);
    data.WriteString(permission);
    sptr<IRemoteObject> remote = Remote();
    if (remote == nullptr) {
        APP_LOGE("Remote() is NULL");
        return ERR_PERMISSION_DENIED;
    }
    int32_t ret =
        remote->SendRequest(static_cast<uint32_t>(IAppMgr::Message::AMS_APP_CHECK_PERMISSION), data, reply, option);
    if (ret != NO_ERROR) {
        APP_LOGE("SendRequest is failed, error code: %{public}d", ret);
        return ERR_PERMISSION_DENIED;
    }
    APP_LOGD("end");
    return reply.ReadInt32();
}

void AppMgrProxy::AbilityCleaned(const sptr<IRemoteObject> &token)
{
    APP_LOGD("start");
    MessageParcel data;
    MessageParcel reply;
    MessageOption option(MessageOption::TF_SYNC);
    if (!WriteInterfaceToken(data)) {
        return;
    }
    data.WriteParcelable(token.GetRefPtr());
    sptr<IRemoteObject> remote = Remote();
    if (remote == nullptr) {
        APP_LOGE("Remote() is NULL");
        return;
    }
    int32_t ret =
        remote->SendRequest(static_cast<uint32_t>(IAppMgr::Message::AMS_APP_ABILITY_CLEANED), data, reply, option);
    if (ret != NO_ERROR) {
        APP_LOGW("SendRequest is failed, error code: %{public}d", ret);
    }
    APP_LOGD("end");
}

sptr<IAmsMgr> AppMgrProxy::GetAmsMgr()
{
    APP_LOGD("begin to get Ams instance");
    MessageParcel data;
    MessageParcel reply;
    if (!WriteInterfaceToken(data)) {
        return nullptr;
    }
    if (!SendTransactCmd(IAppMgr::Message::AMS_APP_GET_MGR_INSTANCE, data, reply)) {
        return nullptr;
    }
    sptr<IRemoteObject> object = reply.ReadParcelable<IRemoteObject>();
    sptr<IAmsMgr> amsMgr = iface_cast<IAmsMgr>(object);
    if (!amsMgr) {
        APP_LOGE("ams instance is nullptr");
        return nullptr;
    }
    APP_LOGD("get ams instance success");
    return amsMgr;
}

int32_t AppMgrProxy::ClearUpApplicationData(const std::string &bundleName)
{
    APP_LOGD("start");
    MessageParcel data;
    MessageParcel reply;
    MessageOption option(MessageOption::TF_SYNC);
    if (!WriteInterfaceToken(data)) {
        return ERR_FLATTEN_OBJECT;
    }
    sptr<IRemoteObject> remote = Remote();
    if (remote == nullptr) {
        APP_LOGE("Remote() is NULL");
        return ERR_NULL_OBJECT;
    }
    if (!data.WriteString(bundleName)) {
        APP_LOGE("parcel WriteString failed");
        return ERR_FLATTEN_OBJECT;
    }
    int32_t ret = remote->SendRequest(
        static_cast<uint32_t>(IAppMgr::Message::AMS_APP_CLEAR_UP_APPLICATION_DATA), data, reply, option);
    if (ret != NO_ERROR) {
        APP_LOGW("SendRequest is failed, error code: %{public}d", ret);
        return ret;
    }
    APP_LOGD("end");
    return reply.ReadInt32();
}

int32_t AppMgrProxy::IsBackgroundRunningRestricted(const std::string &bundleName)
{
    APP_LOGD("start");
    MessageParcel data;
    MessageParcel reply;
    MessageOption option(MessageOption::TF_SYNC);
    if (!WriteInterfaceToken(data)) {
        return ERR_FLATTEN_OBJECT;
    }
    sptr<IRemoteObject> remote = Remote();
    if (remote == nullptr) {
        APP_LOGE("Remote() is NULL");
        return ERR_NULL_OBJECT;
    }
    if (!data.WriteString(bundleName)) {
        APP_LOGE("parcel WriteString failed");
        return ERR_FLATTEN_OBJECT;
    }
    int32_t ret = remote->SendRequest(
        static_cast<uint32_t>(IAppMgr::Message::AMS_APP_IS_BACKGROUND_RUNNING_RESTRICTED), data, reply, option);
    if (ret != NO_ERROR) {
        APP_LOGW("SendRequest is failed, error code: %{public}d", ret);
        return ret;
    }
    APP_LOGD("end");
    return reply.ReadInt32();
}

int32_t AppMgrProxy::GetAllRunningProcesses(std::vector<RunningProcessInfo> &info)
{
    APP_LOGD("start");
    MessageParcel data;
    MessageParcel reply;
    MessageOption option(MessageOption::TF_SYNC);
    if (!WriteInterfaceToken(data)) {
        return ERR_FLATTEN_OBJECT;
    }
    sptr<IRemoteObject> remote = Remote();
    if (remote == nullptr) {
        APP_LOGE("Remote() is NULL");
        return ERR_NULL_OBJECT;
    }
    if (!SendTransactCmd(IAppMgr::Message::AMS_APP_GET_ALL_RUNNING_PROCESSES, data, reply)) {
        return ERR_NULL_OBJECT;
    }
    auto error = GetParcelableInfos<RunningProcessInfo>(reply, info);
    if (error != NO_ERROR) {
        APP_LOGE("GetParcelableInfos fail, error: %{public}d", error);
        return error;
    }
    int result = reply.ReadInt32();
    APP_LOGD("end");
    return result;
}

bool AppMgrProxy::SendTransactCmd(IAppMgr::Message code, MessageParcel &data, MessageParcel &reply)
{
    MessageOption option(MessageOption::TF_SYNC);
    sptr<IRemoteObject> remote = Remote();
    if (!remote) {
        APP_LOGE("fail to send transact cmd %{public}d due to remote object", code);
        return false;
    }
    int32_t result = remote->SendRequest(static_cast<uint32_t>(code), data, reply, option);
    if (result != NO_ERROR) {
        APP_LOGE("receive error transact code %{public}d in transact cmd %{public}d", result, code);
        return false;
    }
    return true;
}

void AppMgrProxy::SetAppFreezingTime(int time)
{
    APP_LOGD("start");
    MessageParcel data;
    MessageParcel reply;
    MessageOption option(MessageOption::TF_SYNC);
    if (!WriteInterfaceToken(data)) {
        return;
    }

    if (!data.WriteInt32(time)) {
        APP_LOGE("parcel WriteInt32 failed");
        return;
    }
    if (!SendTransactCmd(IAppMgr::Message::AMS_APP_SET_APP_FREEZING_TIME, data, reply)) {
        APP_LOGE("SendTransactCmd faild");
        return;
    }
    APP_LOGD("end");

}

void AppMgrProxy::GetAppFreezingTime(int &time)
{
    APP_LOGD("start");
    MessageParcel data;
    MessageParcel reply;
    MessageOption option(MessageOption::TF_SYNC);
    if (!WriteInterfaceToken(data)) {
        APP_LOGE("WriteInterfaceToken faild");
        return;
    }

    if (!SendTransactCmd(IAppMgr::Message::AMS_APP_GET_APP_FREEZING_TIME, data, reply)) {
        APP_LOGE("SendTransactCmd faild");
        return;
    }
    time = reply.ReadInt32();
    APP_LOGE("get freeze time : %{public}d ", time);
}

template <typename T>
int AppMgrProxy::GetParcelableInfos(MessageParcel &reply, std::vector<T> &parcelableInfos)
{
    int32_t infoSize = reply.ReadInt32();
    for (int32_t i = 0; i < infoSize; i++) {
        std::unique_ptr<T> info(reply.ReadParcelable<T>());
        if (!info) {
            APP_LOGE("Read Parcelable infos failed");
            return ERR_INVALID_VALUE;
        }
        parcelableInfos.emplace_back(*info);
    }
    APP_LOGD("get parcelable infos success");
    return NO_ERROR;
}

}  // namespace AppExecFwk
}  // namespace OHOS
