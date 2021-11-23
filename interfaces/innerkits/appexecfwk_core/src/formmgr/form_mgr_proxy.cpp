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
#include "app_log_wrapper.h"
#include "form_mgr_proxy.h"
#include "string_ex.h"

namespace OHOS {
namespace AppExecFwk {
FormMgrProxy::FormMgrProxy(const sptr<IRemoteObject> &impl) : IRemoteProxy<IFormMgr>(impl)
{}
/**
 * @brief Add form with want, send want to form manager service.
 * @param formId The Id of the forms to add.
 * @param want The want of the form to add.
 * @param callerToken Caller ability token.
 * @param formInfo Form info.
 * @return Returns ERR_OK on success, others on failure.
 */
int FormMgrProxy::AddForm(
    const int64_t formId, 
    const Want &want, 
    const sptr<IRemoteObject> &callerToken, 
    FormJsInfo &formInfo)
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
    if (!data.WriteParcelable(&want)) {
        APP_LOGE("%{public}s, failed to write want", __func__);
        return ERR_APPEXECFWK_PARCEL_ERROR;
    }

    if (!data.WriteParcelable(callerToken)) {
        APP_LOGE("%{public}s, failed to write callerToken", __func__);
        return ERR_APPEXECFWK_PARCEL_ERROR;
    }

    int error = GetParcelableInfo<FormJsInfo>(IFormMgr::Message::FORM_MGR_ADD_FORM, data, formInfo);
    if (error != ERR_OK) {
        APP_LOGE("%{public}s, failed to SendRequest: %{public}d", __func__, error);      
    }

    return error;
}

/**
 * @brief Delete forms with formIds, send formIds to form manager service.
 * @param formId The Id of the forms to delete.
 * @param callerToken Caller ability token.
 * @return Returns ERR_OK on success, others on failure.
 */
int FormMgrProxy::DeleteForm(const int64_t formId, const sptr<IRemoteObject> &callerToken)
{
    MessageParcel data;
    if (!WriteInterfaceToken(data)) {
        APP_LOGE("%{public}s, failed to write interface token", __func__);
        return ERR_APPEXECFWK_PARCEL_ERROR;
    }
    if (!data.WriteInt64(formId)) {
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
        static_cast<uint32_t>(IFormMgr::Message::FORM_MGR_DELETE_FORM), 
        data, 
        reply, 
        option);
    if (error != ERR_OK) {
        APP_LOGE("%{public}s, failed to SendRequest: %{public}d", __func__, error);
        return ERR_APPEXECFWK_FORM_SEND_FMS_MSG;
    }
    return reply.ReadInt32();    
}

/**
 * @brief Release forms with formIds, send formIds to form manager service.
 * @param formId The Id of the forms to release.
 * @param callerToken Caller ability token.
 * @param delCache Delete Cache or not.
 * @return Returns ERR_OK on success, others on failure.
 */
int FormMgrProxy::ReleaseForm(const int64_t formId, const sptr<IRemoteObject> &callerToken, const bool delCache)
{
    MessageParcel data;
    if (!WriteInterfaceToken(data)) {
        APP_LOGE("%{public}s, failed to write interface token", __func__);
        return ERR_APPEXECFWK_PARCEL_ERROR;
    }
    if (!data.WriteInt64(formId)) {
        APP_LOGE("%{public}s, failed to write want", __func__);
        return ERR_APPEXECFWK_PARCEL_ERROR;
    }
    if (!data.WriteParcelable(callerToken)) {
        APP_LOGE("%{public}s, failed to write callerToken", __func__);
        return ERR_APPEXECFWK_PARCEL_ERROR;
    }
    if (!data.WriteBool(delCache)) {
        APP_LOGE("%{public}s, failed to write delCache", __func__);
        return ERR_APPEXECFWK_PARCEL_ERROR;
    }       
        
    MessageParcel reply;
    MessageOption option;
    int error = Remote()->SendRequest(
        static_cast<uint32_t>(IFormMgr::Message::FORM_MGR_RELEASE_FORM), 
        data, 
        reply,
        option);
    if (error != ERR_OK) {
        APP_LOGE("%{public}s, failed to SendRequest: %{public}d", __func__, error);
        return ERR_APPEXECFWK_FORM_SEND_FMS_MSG;
    }
    return reply.ReadInt32();
}

/**
 * @brief Update form with formId, send formId to form manager service.
 * @param formId The Id of the form to update.
 * @param bundleName Provider ability bundleName.
 * @param FormProviderData Form binding data.
 * @return Returns ERR_OK on success, others on failure.
 */
int FormMgrProxy::UpdateForm(
    const int64_t formId, 
    const std::string &bundleName, 
    const FormProviderData &FormProviderData)
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

    if (!data.WriteString(bundleName)) {
        APP_LOGE("%{public}s, failed to write bundleName", __func__);
        return ERR_APPEXECFWK_PARCEL_ERROR;
    }
    if (!data.WriteParcelable(&FormProviderData)) {
        APP_LOGE("%{public}s, failed to write formBindingData", __func__);
        return ERR_APPEXECFWK_PARCEL_ERROR;
    }
    MessageParcel reply;
    MessageOption option;
    int error = Remote()->SendRequest(
        static_cast<uint32_t>(IFormMgr::Message::FORM_MGR_UPDATE_FORM), 
        data, 
        reply, 
        option);
    if (error != ERR_OK) {
        APP_LOGE("%{public}s, failed to SendRequest: %{public}d", __func__, error);
        return ERR_APPEXECFWK_FORM_SEND_FMS_MSG;
    }
    return reply.ReadInt32();
}

/**
 * @brief Set next refresh time.
 * @param formId The Id of the form to update.
 * @param bundleName Provider ability bundleName.
 * @param nextTime Next refresh time.
 * @return Returns ERR_OK on success, others on failure.
 */
int FormMgrProxy::SetNextRefreshTime(const int64_t formId, const int64_t nextTime)
{
    MessageParcel data;
    MessageParcel reply;

    if (!WriteInterfaceToken(data)) {
        APP_LOGE("%{public}s, failed to write interface token", __func__);
        return ERR_APPEXECFWK_PARCEL_ERROR;
    }
    if (!data.WriteInt64(formId)) {
        APP_LOGE("%{public}s, failed to write formId", __func__);
        return ERR_APPEXECFWK_PARCEL_ERROR;
    }
    if (!data.WriteInt64(nextTime)) {
        APP_LOGE("%{public}s, failed to write nextTime", __func__);
        return ERR_APPEXECFWK_PARCEL_ERROR;
    }
    MessageOption option;
    int error = Remote()->SendRequest(
        static_cast<uint32_t>(IFormMgr::Message::FORM_MGR_SET_NEXT_REFRESH_TIME), 
        data, 
        reply, 
        option);
    if (error != ERR_OK) {
        APP_LOGE("%{public}s, failed to SendRequest: %{public}d", __func__, error);
        return ERR_APPEXECFWK_FORM_SEND_FMS_MSG;
    }
    return reply.ReadInt32();
}
/**
 * @brief Lifecycle update.
 * @param formIds The Id of the forms.
 * @param callerToken Caller ability token.
 * @param updateType update type.
 * @return Returns ERR_OK on success, others on failure.
 */
int FormMgrProxy::LifecycleUpdate(
    const std::vector<int64_t> &formIds, 
    const sptr<IRemoteObject> &callerToken, 
    const int32_t updateType)
{
    MessageParcel data;
    MessageParcel reply;

    if (!WriteInterfaceToken(data)) {
        APP_LOGE("%{public}s, failed to write interface token", __func__);
        return ERR_APPEXECFWK_PARCEL_ERROR;
    }
    if (!data.WriteInt64Vector(formIds)) {
        APP_LOGE("%{public}s, failed to write formId", __func__);
        return ERR_APPEXECFWK_PARCEL_ERROR;
    }
    if (!data.WriteParcelable(callerToken)) {
        APP_LOGE("%{public}s, failed to write bundleName", __func__);
        return ERR_APPEXECFWK_PARCEL_ERROR;
    }
    if (!data.WriteInt32(updateType)) {
        APP_LOGE("%{public}s, failed to write nextTime", __func__);
        return ERR_APPEXECFWK_PARCEL_ERROR;
    }
    MessageOption option;
    int error = Remote()->SendRequest(
        static_cast<uint32_t>(IFormMgr::Message::FORM_MGR_LIFECYCLE_UPDATE), 
        data, 
        reply, 
        option);
    if (error != ERR_OK) {
        APP_LOGE("%{public}s, failed to SendRequest: %{public}d", __func__, error);
        return ERR_APPEXECFWK_FORM_SEND_FMS_MSG;
    }

    return reply.ReadInt32();
}
/**
 * @brief Request form with formId and want, send formId and want to form manager service.
 * @param formId The Id of the form to update.
 * @param callerToken Caller ability token.
 * @param want The want of the form to add.
 * @return Returns ERR_OK on success, others on failure.
 */
int FormMgrProxy::RequestForm(const int64_t formId, const sptr<IRemoteObject> &callerToken, const Want &want)
{
    APP_LOGI("%{public}s called.", __func__);

    MessageParcel data;
    if (!WriteInterfaceToken(data)) {
        APP_LOGE("%{public}s, failed to write interface token", __func__);
        return ERR_APPEXECFWK_PARCEL_ERROR;
    }
    if (!data.WriteInt64(formId)) {
        APP_LOGE("%{public}s, failed to write formId", __func__);
        return ERR_APPEXECFWK_PARCEL_ERROR;
    }
    if (!data.WriteParcelable(callerToken)) {
        APP_LOGE("%{public}s, failed to write callerToken", __func__);
        return ERR_APPEXECFWK_PARCEL_ERROR;
    }
    if (!data.WriteParcelable(&want)) {
        APP_LOGE("%{public}s, failed to write want", __func__);
        return ERR_APPEXECFWK_PARCEL_ERROR;
    }

    MessageParcel reply;
    MessageOption option;
    int error = Remote()->SendRequest(
        static_cast<uint32_t>(IFormMgr::Message::FORM_MGR_REQUEST_FORM), 
        data, 
        reply, 
        option);
    if (error != ERR_OK) {
        APP_LOGE("%{public}s, failed to SendRequest: %{public}d", __func__, error);
        return ERR_APPEXECFWK_FORM_SEND_FMS_MSG;
    }
    return reply.ReadInt32();
}

/**
 * @brief Form visible/invisible notify, send formIds to form manager service.
 * @param formIds The Id list of the forms to notify.
 * @param callerToken Caller ability token.
 * @param formVisibleType The form visible type, including FORM_VISIBLE and FORM_INVISIBLE.
 * @return Returns ERR_OK on success, others on failure.
 */
int FormMgrProxy::NotifyWhetherVisibleForms(
    const std::vector<int64_t> &formIds, 
    const sptr<IRemoteObject> &callerToken, 
    const int32_t formVisibleType)
{
    MessageParcel data;
    if (!WriteInterfaceToken(data)) {
        APP_LOGE("%{public}s, failed to write interface token", __func__);
        return ERR_APPEXECFWK_PARCEL_ERROR;
    }

    if (!data.WriteInt64Vector(formIds)) {
        APP_LOGE("%{public}s, failed to write formIds", __func__);
        return ERR_APPEXECFWK_PARCEL_ERROR;
    }

    if (!data.WriteParcelable(callerToken)) {
        APP_LOGE("%{public}s, failed to write callerToken", __func__);
        return ERR_APPEXECFWK_PARCEL_ERROR;
    }

    if (!data.WriteInt32(formVisibleType)) {
        APP_LOGE("%{public}s, failed to write formVisibleType", __func__);
        return ERR_APPEXECFWK_PARCEL_ERROR;
    }

    MessageParcel reply;
    MessageOption option;
    int error = Remote()->SendRequest(
        static_cast<uint32_t>(IFormMgr::Message::FORM_MGR_NOTIFY_FORM_WHETHER_VISIBLE), 
        data, 
        reply, 
        option);
    if (error != ERR_OK) {
        APP_LOGE("%{public}s, failed to SendRequest: %{public}d", __func__, error);
        return ERR_APPEXECFWK_FORM_SEND_FMS_MSG;
    }
    return reply.ReadInt32();   
}

/**
 * @brief temp form to normal form.
 * @param formId The Id of the form.
 * @param callerToken Caller ability token.
 * @return Returns ERR_OK on success, others on failure.
 */
int FormMgrProxy::CastTempForm(const int64_t formId, const sptr<IRemoteObject> &callerToken)
{
    MessageParcel data;
    if (!WriteInterfaceToken(data)) {
        APP_LOGE("%{public}s, failed to write interface token", __func__);
        return ERR_APPEXECFWK_PARCEL_ERROR;
    }
    if (!data.WriteInt64(formId)) {
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
        static_cast<uint32_t>(
            IFormMgr::Message::FORM_MGR_CAST_TEMP_FORM), 
            data, 
            reply, 
            option);
    if (error != ERR_OK) {
        APP_LOGE("%{public}s, failed to SendRequest: %{public}d", __func__, error);
        return ERR_APPEXECFWK_FORM_SEND_FMS_MSG;
    }
    return reply.ReadInt32();
}
/**
 * @brief Dump all of form storage infos.
 * @param formInfos All of form storage infos.
 * @return Returns ERR_OK on success, others on failure.
 */
int FormMgrProxy::DumpStorageFormInfos(std::string &formInfos)
{
    MessageParcel data;
    if (!WriteInterfaceToken(data)) {
        APP_LOGE("%{public}s, failed to write interface token", __func__);
        return ERR_APPEXECFWK_PARCEL_ERROR;
    }

    int error = GetStringInfo(IFormMgr::Message::FORM_MGR_STORAGE_FORM_INFOS, data, formInfos);
    if (error != ERR_OK) {
        APP_LOGE("%{public}s, failed to GetStringInfo: %{public}d", __func__, error);     
    }

    return error;
}
/**
 * @brief Dump form info by a bundle name.
 * @param bundleName The bundle name of form provider.
 * @param formInfos Form infos.
 * @return Returns ERR_OK on success, others on failure.
 */
int FormMgrProxy::DumpFormInfoByBundleName(const std::string &bundleName, std::string &formInfos)
{
    MessageParcel data;
    if (!WriteInterfaceToken(data)) {
        APP_LOGE("%{public}s, failed to write interface token", __func__);
        return ERR_APPEXECFWK_PARCEL_ERROR;
    }

    if (!data.WriteString(bundleName)) {
        APP_LOGE("%{public}s, failed to write bundleName", __func__);
        return ERR_APPEXECFWK_PARCEL_ERROR;
    }

    int error = GetStringInfo(IFormMgr::Message::FORM_MGR_FORM_INFOS_BY_NAME, data, formInfos);
    if (error != ERR_OK) {
        APP_LOGE("%{public}s, failed to GetStringInfo: %{public}d", __func__, error);          
    }

    return error;
}
/**
 * @brief Dump form info by a bundle name.
 * @param formId The id of the form.
 * @param formInfo Form info.
 * @return Returns ERR_OK on success, others on failure.
 */
int FormMgrProxy::DumpFormInfoByFormId(const std::int64_t formId, std::string &formInfo)
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

    int error = GetStringInfo(IFormMgr::Message::FORM_MGR_FORM_INFOS_BY_ID, data, formInfo);
    if (error != ERR_OK) {
        APP_LOGE("%{public}s, failed to GetStringInfo: %{public}d", __func__, error);          
    }

    return error;
}
/**
 * @brief Dump timer info by form id.
 * @param formId The id of the form.
 * @param formInfo Form timer info.
 * @return Returns ERR_OK on success, others on failure.
 */
int FormMgrProxy::DumpFormTimerByFormId(const std::int64_t formId, std::string &isTimingService)
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

    int error = GetStringInfo(IFormMgr::Message::FORM_MGR_FORM_TIMER_INFO_BY_ID, data, isTimingService);
    if (error != ERR_OK) {
        APP_LOGE("%{public}s, failed to GetStringInfo: %{public}d", __func__, error);          
    }

    return error;
}
/**
 * @brief Process js message event.
 * @param formId Indicates the unique id of form.
 * @param want information passed to supplier.
 * @param callerToken Caller ability token.
 * @return Returns true if execute success, false otherwise.
 */
int FormMgrProxy::MessageEvent(const int64_t formId, const Want &want, const sptr<IRemoteObject> &callerToken)
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
        static_cast<uint32_t>(IFormMgr::Message::FORM_MGR_MESSAGE_EVENT), 
        data, 
        reply, 
        option);
    if (error != ERR_OK) {
        APP_LOGE("%{public}s, failed to SendRequest: %{public}d", __func__, error);
        return ERR_APPEXECFWK_FORM_SEND_FMS_MSG;
    }
    return reply.ReadInt32();
}

/**
 * @brief Batch add forms to form records for st limit value test.
 * @param want The want of the form to add.
 * @return Returns ERR_OK on success, others on failure.
 */
int FormMgrProxy::BatchAddFormRecords(const Want &want)
{
    // must not implement, just for st
    return ERR_OK;
}
/**
 * @brief Clear form records for st limit value test.
 * @return Returns ERR_OK on success, others on failure.
 */
int FormMgrProxy::ClearFormRecords()
{
    // must not implement, just for st
    return ERR_OK;
}

template<typename T>
int  FormMgrProxy::GetParcelableInfos(MessageParcel &reply, std::vector<T> &parcelableInfos)
{
    int32_t infoSize = reply.ReadInt32();
    for (int32_t i = 0; i < infoSize; i++) {
        std::unique_ptr<T> info(reply.ReadParcelable<T>());
        if (!info) {
            APP_LOGE("%{public}s, failed to Read Parcelable infos", __func__);
            return ERR_APPEXECFWK_PARCEL_ERROR;
        }
        parcelableInfos.emplace_back(*info);
    }
    APP_LOGI("get parcelable infos success");
    return ERR_OK;
}
bool  FormMgrProxy::WriteInterfaceToken(MessageParcel &data)
{
    if (!data.WriteInterfaceToken(IFormMgr::GetDescriptor())) {
        APP_LOGE("%{public}s, failed to write interface token", __func__);
        return false;
    }
    return true;
}
int FormMgrProxy::GetStringInfo(IFormMgr::Message code, MessageParcel &data, std::string &stringInfo)
{
    int error;
    MessageParcel reply;
    error = SendTransactCmd(code, data, reply);
    if (error != ERR_OK) {
        return error;
    }

    error = reply.ReadInt32();
    if (error != ERR_OK) {
        APP_LOGE("%{public}s, failed to read reply result", __func__);
        return error;
    }
    std::vector<std::string> stringInfoList;
    if (!reply.ReadStringVector(&stringInfoList)) {
        APP_LOGE("%{public}s, failed to read string vector from reply", __func__);
        return false;
    }
    if (stringInfoList.empty()) {
        APP_LOGI("%{public}s, No string info", __func__);
        return ERR_APPEXECFWK_FORM_COMMON_CODE;
    }
    for (auto &info : stringInfoList) {
        stringInfo += info;
    }
    APP_LOGD("get string info success");
    return ERR_OK;
}
template<typename T>
int FormMgrProxy::GetParcelableInfo(IFormMgr::Message code, MessageParcel &data, T &parcelableInfo)
{
    int error;
    MessageParcel reply;
    error = SendTransactCmd(code, data, reply);
    if (error != ERR_OK) {
        return error;
    }

    error = reply.ReadInt32();
    if (error != ERR_OK) {
        APP_LOGE("%{public}s, failed to read reply result", __func__);
        return error;
    }

    std::unique_ptr<T> info(reply.ReadParcelable<T>());
    if (!info) {
        APP_LOGE("%{public}s, failed to readParcelableInfo", __func__);
        return ERR_APPEXECFWK_PARCEL_ERROR;
    }
    parcelableInfo = *info;
    APP_LOGD("get parcelable info success");
    return ERR_OK;
}
int FormMgrProxy::SendTransactCmd(IFormMgr::Message code, MessageParcel &data, MessageParcel &reply)
{
    MessageOption option(MessageOption::TF_SYNC);

    sptr<IRemoteObject> remote = Remote();
    if (!remote) {
        APP_LOGE("%{public}s, failed to get remote object, cmd: %{public}d", __func__, code);
        return ERR_APPEXECFWK_SERVICE_NOT_CONNECTED;
    }
    int32_t result = remote->SendRequest(static_cast<uint32_t>(code), data, reply, option);
    if (result != ERR_OK) {
        APP_LOGE("%{public}s, failed to SendRequest: %{public}d, cmd: %{public}d", __func__, result, code);
        return ERR_APPEXECFWK_FORM_SEND_FMS_MSG; 
    }
    return ERR_OK;
}

/**
 * @brief  Add forms to storage for st .
 * @param Want The formDBInfo of the form to add.
 * @return Returns ERR_OK on success, others on failure.
 */
int FormMgrProxy::DistributedDataAddForm(const Want &want)
{
    MessageParcel data;
    if (!WriteInterfaceToken(data)) {
        APP_LOGE("%{public}s, failed to write interface token", __func__);
        return ERR_APPEXECFWK_PARCEL_ERROR;
    }
    if (!data.WriteParcelable(&want)) {
        APP_LOGE("%{public}s, failed to write want", __func__);
        return ERR_APPEXECFWK_PARCEL_ERROR;
    }

    MessageParcel reply;
    MessageOption option;
    int error = Remote()->SendRequest(
        static_cast<uint32_t>(IFormMgr::Message::FORM_MGR_DISTRIBUTED_DATA_ADD_FORM__ST),
        data,
        reply,
        option);
    if (error != ERR_OK) {
        APP_LOGE("%{public}s, failed to SendRequest: %{public}d", __func__, error);
        return ERR_APPEXECFWK_FORM_SEND_FMS_MSG;
    }
    return reply.ReadInt32();
}

/**
 * @brief  Delete form form storage for st.
 * @param formId The formId of the form to delete.
 * @return Returns ERR_OK on success, others on failure.
 */
int FormMgrProxy::DistributedDataDeleteForm(const std::string &formId)
{
    MessageParcel data;
    if (!WriteInterfaceToken(data)) {
        APP_LOGE("%{public}s, failed to write interface token", __func__);
        return ERR_APPEXECFWK_PARCEL_ERROR;
    }
    if (!data.WriteInt64(std::stoll(formId))) {
        APP_LOGE("%{public}s, failed to write formId", __func__);
        return ERR_APPEXECFWK_PARCEL_ERROR;
    }

    MessageParcel reply;
    MessageOption option;
    int error = Remote()->SendRequest(
        static_cast<uint32_t>(IFormMgr::Message::FORM_MGR_DISTRIBUTED_DATA_DELETE_FORM__ST),
        data,
        reply,
        option);
    if (error != ERR_OK) {
        APP_LOGE("%{public}s, failed to SendRequest: %{public}d", __func__, error);
        return ERR_APPEXECFWK_FORM_SEND_FMS_MSG;
    }
    return reply.ReadInt32();
}
}  // namespace AppExecFwk
}  // namespace OHOS
