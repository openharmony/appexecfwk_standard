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

#include "ams_mgr_proxy.h"
#include "ipc_types.h"
#include "iremote_object.h"
#include "string_ex.h"

#include "app_log_wrapper.h"
#include "appexecfwk_errors.h"

namespace OHOS {
namespace AppExecFwk {
AmsMgrProxy::AmsMgrProxy(const sptr<IRemoteObject> &impl) : IRemoteProxy<IAmsMgr>(impl)
{}

bool AmsMgrProxy::WriteInterfaceToken(MessageParcel &data)
{
    if (!data.WriteInterfaceToken(AmsMgrProxy::GetDescriptor())) {
        APP_LOGE("write interface token failed");
        return false;
    }
    return true;
}

void AmsMgrProxy::LoadAbility(const sptr<IRemoteObject> &token, const sptr<IRemoteObject> &preToken,
    const std::shared_ptr<AbilityInfo> &abilityInfo, const std::shared_ptr<ApplicationInfo> &appInfo)
{
    APP_LOGD("start");
    if (!abilityInfo || !appInfo) {
        APP_LOGE("param error");
        return;
    }

    MessageParcel data;
    MessageParcel reply;
    MessageOption option(MessageOption::TF_SYNC);
    if (!WriteInterfaceToken(data)) {
        return;
    }
    data.WriteParcelable(token);
    data.WriteParcelable(preToken);
    data.WriteParcelable(abilityInfo.get());
    data.WriteParcelable(appInfo.get());
    sptr<IRemoteObject> remote = Remote();
    if (remote == nullptr) {
        APP_LOGE("Remote() is NULL");
        return;
    }
    int32_t ret = remote->SendRequest(static_cast<uint32_t>(IAmsMgr::Message::LOAD_ABILITY), data, reply, option);
    if (ret != NO_ERROR) {
        APP_LOGW("SendRequest is failed, error code: %{public}d", ret);
    }
    APP_LOGD("end");
}

void AmsMgrProxy::TerminateAbility(const sptr<IRemoteObject> &token)
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
        remote->SendRequest(static_cast<uint32_t>(IAmsMgr::Message::TERMINATE_ABILITY), data, reply, option);
    if (ret != NO_ERROR) {
        APP_LOGW("SendRequest is failed, error code: %{public}d", ret);
    }
    APP_LOGD("end");
}

void AmsMgrProxy::UpdateAbilityState(const sptr<IRemoteObject> &token, const AbilityState state)
{
    APP_LOGD("start");
    MessageParcel data;
    MessageParcel reply;
    MessageOption option(MessageOption::TF_SYNC);
    if (!WriteInterfaceToken(data)) {
        return;
    }
    data.WriteParcelable(token.GetRefPtr());
    data.WriteInt32(static_cast<int32_t>(state));
    sptr<IRemoteObject> remote = Remote();
    if (remote == nullptr) {
        APP_LOGE("Remote() is NULL");
        return;
    }
    int32_t ret =
        remote->SendRequest(static_cast<uint32_t>(IAmsMgr::Message::UPDATE_ABILITY_STATE), data, reply, option);
    if (ret != NO_ERROR) {
        APP_LOGW("SendRequest is failed, error code: %{public}d", ret);
    }
    APP_LOGD("end");
}

void AmsMgrProxy::UpdateExtensionState(const sptr<IRemoteObject> &token, const ExtensionState state)
{
    APP_LOGD("UpdateExtensionState start");
    MessageParcel data;
    MessageParcel reply;
    MessageOption option(MessageOption::TF_SYNC);
    if (!WriteInterfaceToken(data)) {
        return;
    }
    data.WriteParcelable(token.GetRefPtr());
    data.WriteInt32(static_cast<int32_t>(state));
    sptr<IRemoteObject> remote = Remote();
    if (remote == nullptr) {
        APP_LOGE("Remote() is NULL");
        return;
    }
    int32_t ret =
        remote->SendRequest(static_cast<uint32_t>(IAmsMgr::Message::UPDATE_EXTENSION_STATE), data, reply, option);
    if (ret != NO_ERROR) {
        APP_LOGW("SendRequest is failed, error code: %{public}d", ret);
    }
    APP_LOGD("end");
}

void AmsMgrProxy::RegisterAppStateCallback(const sptr<IAppStateCallback> &callback)
{
    APP_LOGD("start");
    if (!callback) {
        APP_LOGE("callback is nullptr");
        return;
    }

    MessageParcel data;
    MessageParcel reply;
    MessageOption option(MessageOption::TF_SYNC);
    if (!WriteInterfaceToken(data)) {
        return;
    }
    data.WriteParcelable(callback->AsObject());
    sptr<IRemoteObject> remote = Remote();
    if (remote == nullptr) {
        APP_LOGE("Remote() is nullptr");
        return;
    }
    int32_t ret = remote->SendRequest(
        static_cast<uint32_t>(IAmsMgr::Message::REGISTER_APP_STATE_CALLBACK), data, reply, option);
    if (ret != NO_ERROR) {
        APP_LOGW("SendRequest is failed, error code: %{public}d", ret);
    }
    APP_LOGD("end");
}

void AmsMgrProxy::Reset()
{
    APP_LOGD("start");
    MessageParcel data;
    MessageParcel reply;
    MessageOption option(MessageOption::TF_SYNC);
    if (!WriteInterfaceToken(data)) {
        return;
    }
    sptr<IRemoteObject> remote = Remote();
    if (remote == nullptr) {
        APP_LOGE("Remote() is NULL");
        return;
    }
    int32_t ret = remote->SendRequest(static_cast<uint32_t>(IAmsMgr::Message::RESET), data, reply, option);
    if (ret != NO_ERROR) {
        APP_LOGW("SendRequest is failed, error code: %{public}d", ret);
    }
    APP_LOGD("end");
}

void AmsMgrProxy::AbilityBehaviorAnalysis(const sptr<IRemoteObject> &token, const sptr<IRemoteObject> &preToken,
    const int32_t visibility, const int32_t perceptibility, const int32_t connectionState)
{
    APP_LOGD("start");
    MessageParcel data;
    MessageParcel reply;
    MessageOption option(MessageOption::TF_SYNC);
    if (!WriteInterfaceToken(data)) {
        return;
    }
    data.WriteParcelable(token.GetRefPtr());
    data.WriteParcelable(preToken.GetRefPtr());
    data.WriteInt32(static_cast<int32_t>(visibility));
    data.WriteInt32(static_cast<int32_t>(perceptibility));
    data.WriteInt32(static_cast<int32_t>(connectionState));
    sptr<IRemoteObject> remote = Remote();
    if (remote == nullptr) {
        APP_LOGE("Remote() is NULL");
        return;
    }
    int32_t ret =
        remote->SendRequest(static_cast<uint32_t>(IAmsMgr::Message::TERMINATE_ABILITY), data, reply, option);
    if (ret != NO_ERROR) {
        APP_LOGW("SendRequest is failed, error code: %{public}d", ret);
    }
    APP_LOGD("end");
}

void AmsMgrProxy::KillProcessByAbilityToken(const sptr<IRemoteObject> &token)
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
        remote->SendRequest(static_cast<uint32_t>(IAmsMgr::Message::TERMINATE_ABILITY), data, reply, option);
    if (ret != NO_ERROR) {
        APP_LOGW("SendRequest is failed, error code: %{public}d", ret);
    }
    APP_LOGD("end");
}

void AmsMgrProxy::KillProcessesByUserId(int32_t userId)
{
    APP_LOGD("start");
    MessageParcel data;
    MessageParcel reply;
    MessageOption option(MessageOption::TF_SYNC);
    if (!WriteInterfaceToken(data)) {
        return;
    }
    if (!data.WriteInt32(userId)) {
        APP_LOGE("parcel WriteInt32 failed");
        return;
    }
    sptr<IRemoteObject> remote = Remote();
    if (remote == nullptr) {
        APP_LOGE("Remote() is NULL");
        return;
    }
    int32_t ret =
        remote->SendRequest(static_cast<uint32_t>(IAmsMgr::Message::KILL_PROCESSES_BY_USERID), data, reply, option);
    if (ret != NO_ERROR) {
        APP_LOGW("SendRequest is failed, error code: %{public}d", ret);
    }
    APP_LOGD("end");
}

int32_t AmsMgrProxy::KillApplication(const std::string &bundleName)
{
    APP_LOGD("start");
    MessageParcel data;
    MessageParcel reply;
    MessageOption option(MessageOption::TF_SYNC);
    if (!WriteInterfaceToken(data)) {
        return ERR_INVALID_DATA;
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
    int32_t ret =
        remote->SendRequest(static_cast<uint32_t>(IAmsMgr::Message::KILL_APPLICATION), data, reply, option);
    if (ret != NO_ERROR) {
        APP_LOGW("SendRequest is failed, error code: %{public}d", ret);
        return ret;
    }
    return reply.ReadInt32();
}

int32_t AmsMgrProxy::KillApplicationByUid(const std::string &bundleName, const int uid)
{
    APP_LOGD("start");
    MessageParcel data;
    MessageParcel reply;
    MessageOption option(MessageOption::TF_SYNC);
    if (!WriteInterfaceToken(data)) {
        return ERR_INVALID_DATA;
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
    if (!data.WriteInt32(uid)) {
        APP_LOGE("uid write failed.");
        return ERR_FLATTEN_OBJECT;
    }
    int32_t ret =
        remote->SendRequest(static_cast<uint32_t>(IAmsMgr::Message::KILL_APPLICATION_BYUID), data, reply, option);
    if (ret != NO_ERROR) {
        APP_LOGW("SendRequest is failed, error code: %{public}d", ret);
        return ret;
    }
    return reply.ReadInt32();
}

void AmsMgrProxy::AbilityAttachTimeOut(const sptr<IRemoteObject> &token)
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
        remote->SendRequest(static_cast<uint32_t>(IAmsMgr::Message::ABILITY_ATTACH_TIMEOUT), data, reply, option);
    if (ret != NO_ERROR) {
        APP_LOGW("SendRequest is failed, error code: %{public}d", ret);
    }
    APP_LOGD("end");
}

void AmsMgrProxy::PrepareTerminate(const sptr<IRemoteObject> &token)
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
        remote->SendRequest(static_cast<uint32_t>(IAmsMgr::Message::PREPARE_TERMINATE_ABILITY),
            data, reply, option);
    if (ret != NO_ERROR) {
        APP_LOGW("SendRequest is failed, error code: %{public}d", ret);
    }
    APP_LOGD("end");
}

int AmsMgrProxy::CompelVerifyPermission(const std::string &permission, int pid, int uid, std::string &message)
{
    APP_LOGD("start");
    MessageParcel data;
    MessageParcel reply;
    MessageOption option(MessageOption::TF_SYNC);
    if (!WriteInterfaceToken(data)) {
        return ERR_INVALID_DATA;
    }
    if (!data.WriteString16(Str8ToStr16(permission)) || !data.WriteInt32(pid) || !data.WriteInt32(uid)) {
        APP_LOGE("%{public}s, write failed", __func__);
        return ERR_INVALID_DATA;
    }
    sptr<IRemoteObject> remote = Remote();
    if (remote == nullptr) {
        APP_LOGE("Remote() is NULL");
        return ERR_NULL_OBJECT;
    }
    auto ret =
        remote->SendRequest(static_cast<uint32_t>(IAmsMgr::Message::COMPEL_VERIFY_PERMISSION), data, reply, option);
    if (ret != NO_ERROR) {
        APP_LOGW("SendRequest is failed, error code: %{public}d", ret);
        return ERR_INVALID_DATA;
    }
    message = Str16ToStr8(reply.ReadString16());
    return reply.ReadInt32();
}
}  // namespace AppExecFwk
}  // namespace OHOS
