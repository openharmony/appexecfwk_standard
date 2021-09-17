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
#include "appexecfwk_errors.h"
#include "form_provider_proxy.h"
#include "string_ex.h"

namespace OHOS {
namespace AppExecFwk {
/**
 * @brief Acquire to give back an ProviderFormInfo. This is sync API.
 * @param formId The Id of the from.
 * @param want Indicates the {@link Want} structure containing form info.
 * @param callerToken Caller ability token.
 * @return Returns ERR_OK on success, others on failure.
 */
int FormProviderProxy::AcquireProviderFormInfo(
    const int64_t formId, 
    const Want &want, 
    const sptr<IRemoteObject> &callerToken)
{
    int error;
    MessageParcel data;
    MessageParcel reply;
    MessageOption option;

    if (!WriteInterfaceToken(data)) {
        APP_LOGE("%{public}s, failed to write interface token", __func__);
        return ERR_APPEXECFWK_PARCEL_ERROR;
    }
    if (!data.WriteInt64(formId)) {
        APP_LOGE("%{public}s fail, write want error", __func__);
        return ERR_APPEXECFWK_PARCEL_ERROR;
    }
    if (!data.WriteParcelable(&want)) {
        APP_LOGE("%{public}s, failed to write want", __func__);
        return ERR_APPEXECFWK_PARCEL_ERROR;
    }

    if (!data.WriteParcelable(callerToken)) {
        APP_LOGE("%{public}s, failed to write callerToken", __func__);
        return ERR_APPEXECFWK_PARCEL_ERROR;
    }

    error = Remote()->SendRequest(
        static_cast<uint32_t>(IFormProvider::Message::FORM_ACQUIRE_PROVIDER_FORM_INFO), 
        data, 
        reply, 
        option);
    if (error != ERR_OK) {
        APP_LOGE("%{public}s, failed to SendRequest: %{public}d", __func__, error);
        return error;
    }
    return ERR_OK;
}

/**
 * @brief Notify provider when the form was deleted.
 * @param formIds The id list of forms.
 * @param want Indicates the structure containing form info.
 * @param callerToken Caller ability token.
 * @return Returns ERR_OK on success, others on failure.
 */
int FormProviderProxy::NotifyFormDelete(const int64_t formId, const Want &want, const sptr<IRemoteObject> &callerToken)
{
    int error;
    MessageParcel data;
    MessageParcel reply;
    MessageOption option;

    if (!WriteInterfaceToken(data)) {
        APP_LOGE("%{public}s, failed to write interface token", __func__);
        return ERR_APPEXECFWK_PARCEL_ERROR;
    }
    if (!data.WriteInt64(formId)) {
        APP_LOGE("%{public}s, failed to write formId", __func__);
        return ERR_APPEXECFWK_PARCEL_ERROR;
    }
    if (!data.WriteParcelable(&want)) {
        APP_LOGE("%{public}s, failed to write want", __func__);
        return ERR_APPEXECFWK_PARCEL_ERROR;
    }
    if (!data.WriteParcelable(callerToken)) {
        APP_LOGE("%{public}s, failed to write callerToken", __func__);
        return ERR_APPEXECFWK_PARCEL_ERROR;
    }

    error = Remote()->SendRequest(
        static_cast<uint32_t>(IFormProvider::Message::FORM_PROVIDER_NOTIFY_FORM_DELETE), 
        data, 
        reply, 
        option);
    if (error != ERR_OK) {
        APP_LOGE("%{public}s, failed to SendRequest: %{public}d", __func__, error);
        return error;
    }
    return ERR_OK;
}
/**
 * @brief Notify provider when the forms was deleted.
 * @param formIds The id list of forms.
 * @param want Indicates the structure containing form info.
 * @param callerToken Caller ability token.
 * @return Returns ERR_OK on success, others on failure.
 */
int FormProviderProxy::NotifyFormsDelete(
    const std::vector<int64_t> &formIds, 
    const Want &want, 
    const sptr<IRemoteObject> &callerToken)
{
    int error;
    MessageParcel data;
    MessageParcel reply;
    MessageOption option;

    if (!WriteInterfaceToken(data)) {
        APP_LOGE("%{public}s, failed to write interface token", __func__);
        return ERR_APPEXECFWK_PARCEL_ERROR;
    }
    if (!data.WriteInt64Vector(formIds)) {
        APP_LOGE("%{public}s, failed to write formIds", __func__);
        return ERR_APPEXECFWK_PARCEL_ERROR;
    }
    if (!data.WriteParcelable(&want)) {
        APP_LOGE("%{public}s, failed to write want", __func__);
        return ERR_APPEXECFWK_PARCEL_ERROR;
    }
    if (!data.WriteParcelable(callerToken)) {
        APP_LOGE("%{public}s, failed to write callerToken", __func__);
        return ERR_APPEXECFWK_PARCEL_ERROR;
    }

    error = Remote()->SendRequest(
        static_cast<uint32_t>(IFormProvider::Message::FORM_PROVIDER_NOTIFY_FORMS_DELETE), 
        data, 
        reply, 
        option);
    if (error != ERR_OK) {
        APP_LOGE("%{public}s, failed to SendRequest: %{public}d", __func__, error);
        return error;
    }
    return ERR_OK;
}

/**
 * @brief Notify provider when the form need update.
 * @param formId The Id of the form.
 * @param want Indicates the structure containing form info.
 * @param callerToken Caller ability token.
 * @return Returns ERR_OK on success, others on failure.
 */
int FormProviderProxy::NotifyFormUpdate(const int64_t formId, const Want &want, const sptr<IRemoteObject> &callerToken)
{
    int error;
    MessageParcel data;
    MessageParcel reply;
    MessageOption option;

    if (!WriteInterfaceToken(data)) {
        APP_LOGE("%{public}s, failed to write interface token.", __func__);
        return ERR_APPEXECFWK_PARCEL_ERROR;
    }
    if (!data.WriteInt64(formId)) {
        APP_LOGE("%{public}s, failed to write formId.", __func__);
        return ERR_APPEXECFWK_PARCEL_ERROR;
    }

    if (!data.WriteParcelable(&want)) {
        APP_LOGE("%{public}s, failed to write want.", __func__);
        return ERR_APPEXECFWK_PARCEL_ERROR;
    }

    if (!data.WriteParcelable(callerToken)) {
        APP_LOGE("%{public}s, failed to write callerToken.", __func__);
        return ERR_APPEXECFWK_PARCEL_ERROR;
    }

    error = Remote()->SendRequest(
        static_cast<uint32_t>(IFormProvider::Message::FORM_PROVIDER_NOTIFY_FORM_UPDATE), 
        data, 
        reply, 
        option);
    if (error != ERR_OK) {
        APP_LOGE("%{public}s, failed to SendRequest: %{public}d", __func__, error);
        return error;
    }
    return ERR_OK;
}

/**
 * @brief Event notify when change the form visible.
 * 
 * @param formIds The vector of form ids.
 * @param formVisibleType The form visible type, including FORM_VISIBLE and FORM_INVISIBLE.
 * @param want Indicates the structure containing form info.
 * @param callerToken Caller ability token.
 * @return Returns ERR_OK on success, others on failure.
 */
int FormProviderProxy::EventNotify(const std::vector<int64_t> &formIds, const int32_t formVisibleType,
    const Want &want, const sptr<IRemoteObject> &callerToken)
{
    int error;
    MessageParcel data;
    MessageParcel reply;
    MessageOption option;

    if (!WriteInterfaceToken(data)) {
        APP_LOGE("%{public}s, failed to write interface token.", __func__);
        return ERR_APPEXECFWK_PARCEL_ERROR;
    }

    if (!data.WriteInt64Vector(formIds)) {
        APP_LOGE("%{public}s, failed to write formIds.", __func__);
        return ERR_APPEXECFWK_PARCEL_ERROR;
    }

    if (!data.WriteInt32(formVisibleType)) {
        APP_LOGE("%{public}s, failed to write formVisibleType.", __func__);
        return ERR_APPEXECFWK_PARCEL_ERROR;
    }

    if (!data.WriteParcelable(&want)) {
        APP_LOGE("%{public}s, failed to write want.", __func__);
        return ERR_APPEXECFWK_PARCEL_ERROR;
    }

    if (!data.WriteParcelable(callerToken)) {
        APP_LOGE("%{public}s, failed to write callerToken.", __func__);
        return ERR_APPEXECFWK_PARCEL_ERROR;
    }

    error = Remote()->SendRequest(
        static_cast<uint32_t>(IFormProvider::Message::FORM_PROVIDER_EVENT_NOTIFY), 
        data, 
        reply, 
        option);
    if (error != ERR_OK) {
        APP_LOGE("%{public}s, failed to SendRequest: %{public}d", __func__, error);
        return error;
    }
    return ERR_OK;
}

/**
 * @brief Notify provider when the temp form was cast to normal form.
 * @param formId The Id of the form to update.
 * @param want Indicates the structure containing form info.
 * @param callerToken Caller ability token.
 * @return Returns ERR_OK on success, others on failure.
 */
int FormProviderProxy::NotifyFormCastTempForm(
    const int64_t formId, 
    const Want &want, 
    const sptr<IRemoteObject> &callerToken) 
{
    int error;
    MessageParcel data;
    MessageParcel reply;
    MessageOption option;

    if (!WriteInterfaceToken(data)) {
        APP_LOGE("%{public}s, failed to write interface token", __func__);
        return ERR_APPEXECFWK_PARCEL_ERROR;
    }

    if (!data.WriteInt64(formId)) {
        APP_LOGE("%{public}s, failed to write formId", __func__);
        return ERR_APPEXECFWK_PARCEL_ERROR;
    }
    if (!data.WriteParcelable(&want)) {
        APP_LOGE("%{public}s, failed to write want", __func__);
        return ERR_APPEXECFWK_PARCEL_ERROR;
    }
    if (!data.WriteParcelable(callerToken)) {
        APP_LOGE("%{public}s, failed to write callerToken", __func__);
        return ERR_APPEXECFWK_PARCEL_ERROR;
    }

    error = Remote()->SendRequest(
        static_cast<uint32_t>(IFormProvider::Message::FORM_PROVIDER_NOTIFY_TEMP_FORM_CAST), 
        data, 
        reply, 
        option);
    if (error != ERR_OK) {
        APP_LOGE("%{public}s, failed to SendRequest: %{public}d", __func__, error);
        return error;
    }
    return ERR_OK;
}

/**
 * @brief Fire message event to form provider.
 * @param formId The Id of the from.
 * @param message Event message.
 * @param want The want of the request.
 * @param callerToken Form provider proxy object.
 * @return Returns ERR_OK on success, others on failure.
 */
int FormProviderProxy::FireFormEvent(
    const int64_t formId, 
    const std::string &message, 
    const Want &want, 
    const sptr<IRemoteObject> &callerToken)
{
    MessageParcel data;
    if (!WriteInterfaceToken(data)) {
        APP_LOGE("%{public}s, failed to write interface token", __func__);
        return ERR_APPEXECFWK_PARCEL_ERROR;
    }
    if (!data.WriteInt64(formId)) {
        APP_LOGE("%{public}s, failed to write formId", __func__);
        return ERR_APPEXECFWK_PARCEL_ERROR;
    }
    if (!data.WriteString(message)) {
        APP_LOGE("%{public}s, failed to write message", __func__);
        return ERR_APPEXECFWK_PARCEL_ERROR;
    }
    if (!data.WriteParcelable(&want)) {
        APP_LOGE("%{public}s, failed to write want", __func__);
        return ERR_APPEXECFWK_PARCEL_ERROR;
    }
    if (!data.WriteParcelable(callerToken)) {
        APP_LOGE("%{public}s, failed to write callerToken", __func__);
        return ERR_APPEXECFWK_PARCEL_ERROR;
    }
    MessageParcel reply;
    MessageOption option;
    int error = Remote()->SendRequest(
        static_cast<uint32_t>(IFormProvider::Message::FORM_PROVIDER_NOTIFY_FORM_DELETE), 
        data, 
        reply, 
        option);
    if (error != ERR_OK) {
        APP_LOGE("%{public}s, failed to SendRequest: %{public}d", __func__, error);
        return error;
    }
    return ERR_OK;
}

template<typename T>
int  FormProviderProxy::GetParcelableInfos(MessageParcel &reply, std::vector<T> &parcelableInfos)
{
    int32_t infoSize = reply.ReadInt32();
    for (int32_t i = 0; i < infoSize; i++) {
        std::unique_ptr<T> info(reply.ReadParcelable<T>());
        if (!info) {
            APP_LOGE("%{public}s, failed to Read Parcelable infos", __func__);
            return ERR_NULL_OBJECT;
        }
        parcelableInfos.emplace_back(*info);
    }
    APP_LOGI("%{public}s, get parcelable infos success", __func__);
    return ERR_OK;
}

bool  FormProviderProxy::WriteInterfaceToken(MessageParcel &data)
{
    if (!data.WriteInterfaceToken(FormProviderProxy::GetDescriptor())) {
        APP_LOGE("%{public}s, failed to write interface token", __func__);
        return false;
    }
    return true;
}
}  // namespace AppExecFwk
}  // namespace OHOS
