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
#include "form_mgr_stub.h"
#include "ipc_skeleton.h"
#include "ipc_types.h"
#include "iremote_object.h"

namespace OHOS {
namespace AppExecFwk {
const int32_t LIMIT_PARCEL_SIZE = 1024;

void SplitString(const std::string &source, std::vector<std::string> &strings)
{
    int splitSize = (source.size() / LIMIT_PARCEL_SIZE);
    if ((source.size() % LIMIT_PARCEL_SIZE) != 0) {
        splitSize++;
    }
    APP_LOGD("the dump string split into %{public}d size", splitSize);
    for (int i = 0; i < splitSize; i++) {
        int32_t start = LIMIT_PARCEL_SIZE * i;
        strings.emplace_back(source.substr(start, LIMIT_PARCEL_SIZE));
    }
}

FormMgrStub::FormMgrStub()
{
    memberFuncMap_[static_cast<uint32_t>(IFormMgr::Message::FORM_MGR_ADD_FORM)] =
        &FormMgrStub::HandleAddForm;
    memberFuncMap_[static_cast<uint32_t>(IFormMgr::Message::FORM_MGR_DELETE_FORM)] =
        &FormMgrStub::HandleDeleteForm;
    memberFuncMap_[static_cast<uint32_t>(IFormMgr::Message::FORM_MGR_RELEASE_FORM)] =
        &FormMgrStub::HandleReleaseForm;
    memberFuncMap_[static_cast<uint32_t>(IFormMgr::Message::FORM_MGR_UPDATE_FORM)] =
        &FormMgrStub::HandleUpdateForm;
    memberFuncMap_[static_cast<uint32_t>(IFormMgr::Message::FORM_MGR_REQUEST_FORM)] =
        &FormMgrStub::HandleRequestForm;
    memberFuncMap_[static_cast<uint32_t>(IFormMgr::Message::FORM_MGR_NOTIFY_FORM_WHETHER_VISIBLE)] =
        &FormMgrStub::HandleNotifyWhetherVisibleForms;
    memberFuncMap_[static_cast<uint32_t>(IFormMgr::Message::FORM_MGR_CAST_TEMP_FORM)] =
        &FormMgrStub::HandleCastTempForm;
    memberFuncMap_[static_cast<uint32_t>(IFormMgr::Message::FORM_MGR_STORAGE_FORM_INFOS)] =
        &FormMgrStub::HandleDumpStorageFormInfos;
    memberFuncMap_[static_cast<uint32_t>(IFormMgr::Message::FORM_MGR_FORM_INFOS_BY_NAME)] =
        &FormMgrStub::HandleDumpFormInfoByBundleName;
    memberFuncMap_[static_cast<uint32_t>(IFormMgr::Message::FORM_MGR_FORM_INFOS_BY_ID)] =
        &FormMgrStub::HandleDumpFormInfoByFormId;
    memberFuncMap_[static_cast<uint32_t>(IFormMgr::Message::FORM_MGR_FORM_TIMER_INFO_BY_ID)] =
        &FormMgrStub::HandleDumpFormTimerByFormId;
    memberFuncMap_[static_cast<uint32_t>(IFormMgr::Message::FORM_MGR_SET_NEXT_REFRESH_TIME)] =
        &FormMgrStub::HandleSetNextRefreshTime;
    memberFuncMap_[static_cast<uint32_t>(IFormMgr::Message::FORM_MGR_LIFECYCLE_UPDATE)] =
        &FormMgrStub::HandleLifecycleUpdate;
    memberFuncMap_[static_cast<uint32_t>(IFormMgr::Message::FORM_MGR_MESSAGE_EVENT)] =
        &FormMgrStub::HandleMessageEvent;
    memberFuncMap_[static_cast<uint32_t>(IFormMgr::Message::FORM_MGR_BATCH_ADD_FORM_RECORDS_ST)] =
        &FormMgrStub::HandleBatchAddFormRecords;
    memberFuncMap_[static_cast<uint32_t>(IFormMgr::Message::FORM_MGR_CLEAR_FORM_RECORDS_ST)] =
        &FormMgrStub::HandleClearFormRecords;
}

FormMgrStub::~FormMgrStub()
{
    memberFuncMap_.clear();
}

/**
 * @brief handle remote request.
 * @param data input param.
 * @param reply output param.
 * @param option message option.
 * @return Returns ERR_OK on success, others on failure.
 */
int FormMgrStub::OnRemoteRequest(uint32_t code, MessageParcel &data, MessageParcel &reply, MessageOption &option)
{
    APP_LOGI("FormMgrStub::OnReceived, code = %{public}d, flags= %{public}d.", code, option.GetFlags());
    std::u16string descriptor = FormMgrStub::GetDescriptor();
    std::u16string remoteDescriptor = data.ReadInterfaceToken();
    if (descriptor != remoteDescriptor) {
        APP_LOGE("%{public}s failed, local descriptor is not equal to remote", __func__);
        return ERR_APPEXECFWK_FORM_INVALID_PARAM;
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
/**
 * @brief handle AddForm message.
 * @param data input param.
 * @param reply output param.
 * @return Returns ERR_OK on success, others on failure.
 */
int32_t FormMgrStub::HandleAddForm(MessageParcel &data, MessageParcel &reply)
{
    int64_t formId = data.ReadInt64();
    std::unique_ptr<Want> want(data.ReadParcelable<Want>());
    if (!want) {
        APP_LOGE("%{public}s, failed to ReadParcelable<FormReqInfo>", __func__);
        return ERR_APPEXECFWK_PARCEL_ERROR;
    }

    sptr<IRemoteObject> client = data.ReadParcelable<IRemoteObject>();
    if (client == nullptr) {
        APP_LOGE("%{public}s, failed to RemoteObject invalidate", __func__);
        return ERR_APPEXECFWK_PARCEL_ERROR;
    }

    FormJsInfo formInfo;
    int32_t result = AddForm(formId, *want, client, formInfo);
    reply.WriteInt32(result);
    reply.WriteParcelable(&formInfo);

    return result;
}
/**
 * @brief handle DeleteForm message.
 * @param data input param.
 * @param reply output param.
 * @return Returns ERR_OK on success, others on failure.
 */
int32_t FormMgrStub::HandleDeleteForm(MessageParcel &data, MessageParcel &reply)
{
    int64_t formId = data.ReadInt64();
    sptr<IRemoteObject> client = data.ReadParcelable<IRemoteObject>();
    if (client == nullptr) {
        return ERR_APPEXECFWK_PARCEL_ERROR;
    }
    int32_t result = DeleteForm(formId, client);
    reply.WriteInt32(result);
    return result;
}
/**
 * @brief handle ReleaseForm message.
 * @param data input param.
 * @param reply output param.
 * @return Returns ERR_OK on success, others on failure.
 */
int32_t FormMgrStub::HandleReleaseForm(MessageParcel &data, MessageParcel &reply)
{
    int64_t formId = data.ReadInt64();
    sptr<IRemoteObject> client = data.ReadParcelable<IRemoteObject>();
    if (client == nullptr) {
        return ERR_APPEXECFWK_PARCEL_ERROR;
    }
    bool delCache = data.ReadBool();
    
    int32_t result = ReleaseForm(formId, client, delCache);
    reply.WriteInt32(result);
    return result;
}
/**
 * @brief handle UpdateForm message.
 * @param data input param.
 * @param reply output param.
 * @return Returns ERR_OK on success, others on failure.
 */
int32_t FormMgrStub::HandleUpdateForm(MessageParcel &data, MessageParcel &reply)
{
    int64_t formId = data.ReadInt64();
    std::string bundleName = data.ReadString();
    std::unique_ptr<FormProviderData> formBindingData(data.ReadParcelable<FormProviderData>());
    int32_t result = UpdateForm(formId, bundleName, *formBindingData);
    reply.WriteInt32(result);
    return result;
}
/**
     * @brief handle SetNextRefreshTime message.
     * @param data input param.
     * @param reply output param.
     * @return Returns ERR_OK on success, others on failure.
     */
int32_t FormMgrStub::HandleSetNextRefreshTime(MessageParcel &data, MessageParcel &reply)
{
    int64_t formId = data.ReadInt64();
    int64_t nextTime = data.ReadInt64();
    int32_t result = SetNextRefreshTime(formId, nextTime);
    reply.WriteInt32(result);
    return result;
}
/**
 * @brief handle LifecycleUpdate message.
 * @param data input param.
 * @param reply output param.
 * @return Returns ERR_OK on success, others on failure.
 */
int32_t FormMgrStub::HandleLifecycleUpdate(MessageParcel &data, MessageParcel &reply)
{
    std::vector<int64_t> formIds;
    bool ret = data.ReadInt64Vector(&formIds);
    if (!ret) {
        return ERR_APPEXECFWK_PARCEL_ERROR;
    }
    sptr<IRemoteObject> client = data.ReadParcelable<IRemoteObject>();
    if (client == nullptr) {
        APP_LOGE("%{public}s, failed to ReadParcelable<IRemoteObject>", __func__);
        return ERR_APPEXECFWK_PARCEL_ERROR;
    }
    int32_t updateType = data.ReadInt32();
    int32_t result = LifecycleUpdate(formIds, client, updateType);
    reply.WriteInt32(result);
    return result;
}
/**
 * @brief handle RequestForm message.
 * @param data input param.
 * @param reply output param.
 * @return Returns ERR_OK on success, others on failure.
 */
int32_t FormMgrStub::HandleRequestForm(MessageParcel &data, MessageParcel &reply)
{
    APP_LOGI("%{public}s called.", __func__);

    int64_t formId = data.ReadInt64();

    sptr<IRemoteObject> client = data.ReadParcelable<IRemoteObject>();
    if (client == nullptr) {
        APP_LOGE("%{public}s, failed to ReadParcelable<IRemoteObject>", __func__);
        return ERR_APPEXECFWK_PARCEL_ERROR;
    }

    std::unique_ptr<Want> want(data.ReadParcelable<Want>());
    if (!want) {
        APP_LOGE("%{public}s, failed to ReadParcelable<Want>", __func__);
        return ERR_APPEXECFWK_PARCEL_ERROR;
    }

    int32_t result = RequestForm(formId, client, *want);
    reply.WriteInt32(result);
    return result;
}
/**
 * @brief handle NotifyVisibleForms message.
 * @param data input param.
 * @param reply output param.
 * @return Returns ERR_OK on success, others on failure.
 */
int32_t FormMgrStub::HandleNotifyWhetherVisibleForms(MessageParcel &data, MessageParcel &reply)
{
    std::vector<int64_t> formIds;
    bool ret = data.ReadInt64Vector(&formIds);
    if (!ret) {
        return ERR_APPEXECFWK_PARCEL_ERROR;
    }

    sptr<IRemoteObject> client = data.ReadParcelable<IRemoteObject>();
    if (client == nullptr) {
        return ERR_APPEXECFWK_PARCEL_ERROR;
    }
    int32_t formVisibleType = data.ReadInt32();

    int32_t result = NotifyWhetherVisibleForms(formIds, client, formVisibleType);
    reply.WriteInt32(result);
    return result;
}

/**
 * @brief handle CastTempForm message.
 * @param data input param.
 * @param reply output param.
 * @return Returns ERR_OK on success, others on failure.
 */
int32_t FormMgrStub::HandleCastTempForm(MessageParcel &data, MessageParcel &reply)
{
    int64_t formId = data.ReadInt64();
    sptr<IRemoteObject> client = data.ReadParcelable<IRemoteObject>();
    if (client == nullptr) {
        return ERR_APPEXECFWK_PARCEL_ERROR;
    }

    int32_t result = CastTempForm(formId, client);
    reply.WriteInt32(result);
    return result;
}
/**
 * @brief Handle DumpStorageFormInfos message.
 * @param data input param.
 * @param reply output param.
 * @return Returns ERR_OK on success, others on failure.
 */
int32_t FormMgrStub::HandleDumpStorageFormInfos(MessageParcel &data, MessageParcel &reply)
{
    std::string formInfos;
    int32_t result = DumpStorageFormInfos(formInfos);
    reply.WriteInt32(result);
    if (result == ERR_OK) {
        std::vector<std::string> dumpInfos;
        SplitString(formInfos, dumpInfos);
        if (!reply.WriteStringVector(dumpInfos)) {
            APP_LOGE("%{public}s, failed to WriteStringVector<dumpInfos>", __func__);
            return ERR_APPEXECFWK_PARCEL_ERROR;
        }
    }
    
    return result;
}
/**
 * @brief Handle DumpFormInfoByBundleName message.
 * @param data input param.
 * @param reply output param.
 * @return Returns ERR_OK on success, others on failure.
 */
int32_t FormMgrStub::HandleDumpFormInfoByBundleName(MessageParcel &data, MessageParcel &reply)
{
    std::string bundleName = data.ReadString();
    std::string formInfos;
    int32_t result = DumpFormInfoByBundleName(bundleName, formInfos);
    reply.WriteInt32(result);
    if (result == ERR_OK) {
        APP_LOGD("%{public}s, formInfos: %{public}s", __func__, formInfos.c_str());
        std::vector<std::string> dumpInfos;
        SplitString(formInfos, dumpInfos);
        if (!reply.WriteStringVector(dumpInfos)) {
            APP_LOGE("%{public}s, failed to WriteStringVector<dumpInfos>", __func__);
            return ERR_APPEXECFWK_PARCEL_ERROR;
        }
    }

    return result;
}
/**
 * @brief Handle DumpFormInfoByFormId message.
 * @param data input param.
 * @param reply output param.
 * @return Returns ERR_OK on success, others on failure.
 */
int32_t FormMgrStub::HandleDumpFormInfoByFormId(MessageParcel &data, MessageParcel &reply)
{
    int64_t formId = data.ReadInt64();
    std::string formInfo;
    int32_t result = DumpFormInfoByFormId(formId, formInfo);
    reply.WriteInt32(result);
    if (result == ERR_OK) {
        std::vector<std::string> dumpInfos;
        SplitString(formInfo, dumpInfos);
        if (!reply.WriteStringVector(dumpInfos)) {
            APP_LOGE("%{public}s, failed to WriteStringVector<dumpInfos>", __func__);
            return ERR_APPEXECFWK_PARCEL_ERROR;
        }
    }
    return result;
}
/**
 * @brief Handle DumpFormTimerByFormId message.
 * @param data input param.
 * @param reply output param.
 * @return Returns ERR_OK on success, others on failure.
 */
int32_t FormMgrStub::HandleDumpFormTimerByFormId(MessageParcel &data, MessageParcel &reply)
{
    int64_t formId = data.ReadInt64();
    std::string isTimingService;
    int32_t result = DumpFormTimerByFormId(formId, isTimingService);
    reply.WriteInt32(result);
    if (result == ERR_OK) {
        std::vector<std::string> dumpInfos;
        SplitString(isTimingService, dumpInfos);
        if (!reply.WriteStringVector(dumpInfos)) {
            APP_LOGE("%{public}s, failed to WriteStringVector<dumpInfos>", __func__);
            return ERR_APPEXECFWK_PARCEL_ERROR;
        }
    }
    return result;
}

/**
 * @brief Handle DumpFormInfoByFormId message.
 * @param data input param.
 * @param reply output param.
 * @return Returns ERR_OK on success, others on failure.
 */
int32_t FormMgrStub::HandleMessageEvent(MessageParcel &data, MessageParcel &reply)
{
    APP_LOGI("%{public}s called.", __func__);

    int64_t formId = data.ReadInt64();

    std::unique_ptr<Want> want(data.ReadParcelable<Want>());
    if (!want) {
        APP_LOGE("%{public}s, failed to ReadParcelable<Want>", __func__);
        return ERR_APPEXECFWK_PARCEL_ERROR;
    }

    sptr<IRemoteObject> client = data.ReadParcelable<IRemoteObject>();
    if (client == nullptr) {
        APP_LOGE("%{public}s, failed to ReadParcelable<IRemoteObject>", __func__);
        return ERR_APPEXECFWK_PARCEL_ERROR;
    }
    
    int32_t result = MessageEvent(formId, *want, client);
    reply.WriteInt32(result);
    return result;
}

/**
 * @brief Handle BatchAddFormRecords message.
 * @param data input param.
 * @param reply output param.
 * @return Returns ERR_OK on success, others on failure.
 */
int32_t FormMgrStub::HandleBatchAddFormRecords(MessageParcel &data, MessageParcel &reply)
{
    APP_LOGI("%{public}s called.", __func__);

    std::unique_ptr<Want> want(data.ReadParcelable<Want>());
    if (!want) {
        APP_LOGE("%{public}s, failed to ReadParcelable<Want>", __func__);
        return ERR_APPEXECFWK_PARCEL_ERROR;
    }

    int32_t result = BatchAddFormRecords(*want);
    reply.WriteInt32(result);
    return result;
}
/**
 * @brief Handle BatchDeleteFormRecords message.
 * @param data input param.
 * @param reply output param.
 * @return Returns ERR_OK on success, others on failure.
 */
int32_t FormMgrStub::HandleClearFormRecords(MessageParcel &data, MessageParcel &reply)
{
    APP_LOGI("%{public}s called.", __func__);  
    int32_t result = ClearFormRecords();
    reply.WriteInt32(result);
    return result;
}
}  // namespace AppExecFwk
}  // namespace OHOS
