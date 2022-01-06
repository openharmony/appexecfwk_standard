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

#include <cinttypes>

#include "app_log_wrapper.h"
#include "form_constants.h"
#include "form_data_mgr.h"
#include "form_host_interface.h"
#include "form_item_info.h"
#include "form_mgr_adapter.h"
#include "form_provider_interface.h"
#include "form_supply_callback.h"
#include "form_task_mgr.h"
#include "form_util.h"

namespace OHOS {
namespace AppExecFwk {
const int FORM_TASK_DELAY_TIME = 6; // ms
FormTaskMgr::FormTaskMgr() {}
FormTaskMgr::~FormTaskMgr() {}
/**
 * @brief Acquire form data from form provider(task).
 * @param formId The Id of the form.
 * @param want The want of the request.
 * @param remoteObject Form provider proxy object.
 */
void FormTaskMgr::PostAcquireTask(const int64_t formId, const Want &want, const sptr<IRemoteObject> &remoteObject)
{
    if (eventHandler_ == nullptr) {
        APP_LOGE("%{public}s fail, eventhandler invalidate", __func__);
        return;
    }
    std::function<void()> acquireProviderFormInfoFunc = std::bind(&FormTaskMgr::AcquireProviderFormInfo,
        this, formId, want, remoteObject);
    eventHandler_->PostTask(acquireProviderFormInfoFunc, FORM_TASK_DELAY_TIME);
}
/**
 * @brief Delete form data from form provider(task).
 * @param formId The Id of the form.
 * @param want The want of the request.
 * @param remoteObject Form provider proxy object.
 */
void FormTaskMgr::PostDeleteTask(const int64_t formId, const Want &want, const sptr<IRemoteObject> &remoteObject)
{
    if (eventHandler_ == nullptr) {
        APP_LOGE("%{public}s fail, eventhandler invalidate", __func__);
        return;
    }
    std::function<void()> notifyFormDeleteFunc = std::bind(&FormTaskMgr::NotifyFormDelete,
        this, formId, want, remoteObject);
    eventHandler_->PostTask(notifyFormDeleteFunc, FORM_TASK_DELAY_TIME);
}

/**
 * @brief Refresh form data from form provider(task).
 * 
 * @param formId The Id of the form.
 * @param want The want of the form.
 * @param remoteObject Form provider proxy object.
 * @return none.
 */
void FormTaskMgr::PostRefreshTask(const int64_t formId, const Want &want, const sptr<IRemoteObject> &remoteObject)
{
    if (eventHandler_ == nullptr) {
        APP_LOGE("%{public}s fail, eventhandler invalidate.", __func__);
        return;
    }
    std::function<void()> notifyFormUpdateFunc = std::bind(&FormTaskMgr::NotifyFormUpdate,
        this, formId, want, remoteObject);
    eventHandler_->PostTask(notifyFormUpdateFunc, FORM_TASK_DELAY_TIME);
}

/**
 * @brief Cast temp form data from form provider(task).
 * 
 * @param formId The Id of the form.
 * @param want The want of the form.
 * @param remoteObject Form provider proxy object.
 * @return none.
 */
void FormTaskMgr::PostCastTempTask(const int64_t formId, const Want &want, const sptr<IRemoteObject> &remoteObject)
{
    if (eventHandler_ == nullptr) {
        APP_LOGE("%{public}s fail, eventhandler invalidate", __func__);
        return;
    }
    std::function<void()> notifyCastTempFunc = std::bind(&FormTaskMgr::NotifyCastTemp,
        this, formId, want, remoteObject);
    eventHandler_->PostTask(notifyCastTempFunc, FORM_TASK_DELAY_TIME);
}

/**
 * @brief Post form data to form host(task) when acquire form.
 * @param formId The Id of the form.
 * @param callingUid Calling uid.
 * @param info Form configure info.
 * @param wantParams WantParams of the request.
 * @param remoteObject Form provider proxx object.
 */
void FormTaskMgr::PostAcquireTaskToHost(const int64_t formId, const FormRecord &record, 
    const sptr<IRemoteObject> &remoteObject)
{
    if (eventHandler_ == nullptr) {
        APP_LOGE("%{public}s fail, eventhandler invalidate", __func__);
        return;
    }
    std::function<void()> acquireTaskToHostFunc = std::bind(&FormTaskMgr::AcquireTaskToHost,
        this, formId, record, remoteObject);
    eventHandler_->PostTask(acquireTaskToHostFunc, FORM_TASK_DELAY_TIME);
}

/**
 * @brief Post form data to form host(task) when update form.
 * @param formId The Id of the form.
 * @param callingUid Calling uid.
 * @param info Form configure info.
 * @param wantParams WantParams of the request.
 * @param remoteObject Form provider proxx object.
 */
void FormTaskMgr::PostUpdateTaskToHost(const int64_t formId, const FormRecord &record, 
    const sptr<IRemoteObject> &remoteObject)
{
    APP_LOGI("%{public}s called.", __func__);

    if (eventHandler_ == nullptr) {
        APP_LOGE("%{public}s fail, eventhandler invalidate.", __func__);
        return;
    }

    APP_LOGD("%{public}s, post the task of updateTaskToHostFunc.", __func__);
    std::function<void()> updateTaskToHostFunc = std::bind(&FormTaskMgr::UpdateTaskToHost,
        this, formId, record, remoteObject);
    eventHandler_->PostTask(updateTaskToHostFunc, FORM_TASK_DELAY_TIME);
}

/**
 * @brief Acquire form data from form provider.
 * @param formId The Id of the form.
 * @param info Form configure info.
 * @param wantParams WantParams of the request.
 * @param remoteObject Form provider proxx object.
 */
/**
 * @brief Handel form host died(task).
 * @param remoteHost Form host proxy object.
 */
void FormTaskMgr::PostHostDiedTask(const sptr<IRemoteObject> &remoteHost)
{
    if (eventHandler_ == nullptr) {
        APP_LOGE("%{public}s fail, eventhandler invalidate", __func__);
        return;
    }
    std::function<void()> postTaskFunc = std::bind(&FormTaskMgr::HostDied,
        this, remoteHost);
    eventHandler_->PostTask(postTaskFunc, FORM_TASK_DELAY_TIME);
}

/**
 * @brief Post event notify to form provider.
 * 
 * @param formEvent The vector of form ids.
 * @param formVisibleType The form visible type, including FORM_VISIBLE and FORM_INVISIBLE.
 * @param want The want of the form.
 * @param remoteObject The form provider proxy object.
 * @return none.
 */
void FormTaskMgr::PostEventNotifyTask(const std::vector<int64_t> &formEvent, const int32_t formVisibleType, 
    const Want &want, const sptr<IRemoteObject> &remoteObject)
{
    if (eventHandler_ == nullptr) {
        APP_LOGE("%{public}s fail, eventhandler invalidate.", __func__);
        return;
    }
    std::function<void()> eventNotifyFunc = std::bind(&FormTaskMgr::EventNotify,
        this, formEvent, formVisibleType, want, remoteObject);
    eventHandler_->PostTask(eventNotifyFunc, FORM_TASK_DELAY_TIME);
}
/**
 * @brief Post provider batch delete.
 * @param formIds The Id list.
 * @param want The want of the request.
 * @param remoteObject Form provider proxy object.
 */
void FormTaskMgr::PostProviderBatchDeleteTask(std::set<int64_t> &formIds, const Want &want, 
    const sptr<IRemoteObject> &remoteObject)
{
    if (eventHandler_ == nullptr) {
        APP_LOGE("%{public}s fail, eventhandler invalidate.", __func__);
        return;
    }
    std::function<void()> batchDeleteFunc = std::bind(&FormTaskMgr::ProviderBatchDelete,
        this, formIds, want, remoteObject);
    eventHandler_->PostTask(batchDeleteFunc, FORM_TASK_DELAY_TIME);
}
/**
 * @brief Post message event to form provider.
 * @param formId The Id of the from.
 * @param message Event message.
 * @param want The want of the request.
 * @param remoteObject Form provider proxy object.
 */
void FormTaskMgr::PostFormEventTask(const int64_t formId, const std::string &message, 
    const Want &want, const sptr<IRemoteObject> &remoteObject)
{
    if (eventHandler_ == nullptr) {
        APP_LOGE("%{public}s fail, eventhandler invalidate.", __func__);
        return;
    }
    std::function<void()> formEventFunc = std::bind(&FormTaskMgr::FireFormEvent,
        this, formId, message, want, remoteObject);
    eventHandler_->PostTask(formEventFunc, FORM_TASK_DELAY_TIME);
}

/**
 * @brief Post uninstall message to form host(task).
 * @param formIds The Id list of the forms.
 * @param remoteObject Form provider proxy object.
 */
void FormTaskMgr::PostUninstallTaskToHost(const std::vector<int64_t> &formIds, const sptr<IRemoteObject> &remoteObject)
{
    APP_LOGI("%{public}s start", __func__);
    if (eventHandler_ == nullptr) {
        APP_LOGE("%{public}s fail, eventhandler invalidate.", __func__);
        return;
    }
    std::function<void()> uninstallFunc = std::bind(&FormTaskMgr::FormUninstall,
        this, formIds, remoteObject);
    eventHandler_->PostTask(uninstallFunc, FORM_TASK_DELAY_TIME);
    APP_LOGI("%{public}s end", __func__);
}

/**
 * @brief Acquire form data from form provider.
 * @param formId The Id of the from.
 * @param want The want of the request.
 * @param remoteObject Form provider proxy object.
 */
void FormTaskMgr::AcquireProviderFormInfo(const int64_t formId, const Want &want, 
    const sptr<IRemoteObject> &remoteObject)
{   
    FormMgrAdapter::GetInstance().AcquireProviderFormInfo(formId, want, remoteObject);
}

/**
 * @brief Notify form provider for delete form.
 * 
 * @param formId The Id of the from.
 * @param want The want of the form.
 * @param remoteObject Form provider proxy object.
 * @return none.
 */
void FormTaskMgr::NotifyFormDelete(const int64_t formId, const Want &want, const sptr<IRemoteObject> &remoteObject)
{
    FormMgrAdapter::GetInstance().NotifyFormDelete(formId, want, remoteObject);
}

/**
 * @brief Notify form provider for updating form.
 * 
 * @param formId The Id of the from.
 * @param want The want of the form.
 * @param remoteObject Form provider proxy object.
 * @return none.
 */
void FormTaskMgr::NotifyFormUpdate(const int64_t formId, const Want &want, const sptr<IRemoteObject> &remoteObject)
{
    APP_LOGI("%{public}s called.", __func__);

    long connectId = want.GetLongParam(Constants::FORM_CONNECT_ID, 0);
    sptr<IFormProvider> formProviderProxy = iface_cast<IFormProvider>(remoteObject);
    if (formProviderProxy == nullptr) {
        FormSupplyCallback::GetInstance()->RemoveConnection(connectId);
        APP_LOGE("%{public}s fail, failed to get formProviderProxy", __func__);
        return;
    }
    int error = formProviderProxy->NotifyFormUpdate(formId, want, FormSupplyCallback::GetInstance());
    if (error != ERR_OK) {
        FormSupplyCallback::GetInstance()->RemoveConnection(connectId);
        APP_LOGE("%{public}s fail, Failed to notify form update.", __func__);
    }
}

/**
 * @brief Event notify to form provider.
 * 
 * @param formEvents The vector of form ids.
 * @param formVisibleType The form visible type, including FORM_VISIBLE and FORM_INVISIBLE.
 * @param want The want of the form.
 * @param remoteObject The form provider proxy object.
 * @return none.
 */ 
void FormTaskMgr::EventNotify(const std::vector<int64_t> &formEvents, const int32_t formVisibleType, 
    const Want &want, const sptr<IRemoteObject> &remoteObject)
{
    APP_LOGI("%{public}s called.", __func__);

    long connectId = want.GetLongParam(Constants::FORM_CONNECT_ID, 0);
    sptr<IFormProvider> formProviderProxy = iface_cast<IFormProvider>(remoteObject);
    if (formProviderProxy == nullptr) {
        FormSupplyCallback::GetInstance()->RemoveConnection(connectId);
        APP_LOGE("%{public}s fail, failed to get formProviderProxy", __func__);
        return;
    }
    
    int error = formProviderProxy->EventNotify(formEvents, formVisibleType, want, FormSupplyCallback::GetInstance());
    if (error != ERR_OK) {
        FormSupplyCallback::GetInstance()->RemoveConnection(connectId);
        APP_LOGE("%{public}s fail, Failed to send event notify.", __func__);
    }
}

/**
 * @brief Notify form provider for cast temp form.
 * 
 * @param formId The Id of the from.
 * @param want The want of the form.
 * @param remoteObject Form provider proxy object.
 * @return none.
 */
void FormTaskMgr::NotifyCastTemp(const int64_t formId, const Want &want, const sptr<IRemoteObject> &remoteObject)
{
    APP_LOGI("%{public}s called.", __func__);

    long connectId = want.GetLongParam(Constants::FORM_CONNECT_ID, 0);
    sptr<IFormProvider> formProviderProxy = iface_cast<IFormProvider>(remoteObject);
    if (formProviderProxy == nullptr) {
        FormSupplyCallback::GetInstance()->RemoveConnection(connectId);
        APP_LOGE("%{public}s fail, failed to get formProviderProxy", __func__);
        return;
    }

    int error = formProviderProxy->NotifyFormCastTempForm(formId, want, FormSupplyCallback::GetInstance());
    if (error != ERR_OK) {
        FormSupplyCallback::GetInstance()->RemoveConnection(connectId);
        APP_LOGE("%{public}s fail, Failed to get acquire provider form info", __func__);
    }
}

/**
 * @brief Post form data to form host when acquire form.
 * @param formId The Id of the form.
 * @param callingUid Calling uid.
 * @param info Form configure info.
 * @param wantParams WantParams of the request.
 * @param remoteObject Form provider proxx object.
 */
void FormTaskMgr::AcquireTaskToHost(const int64_t formId, const FormRecord &record, 
    const sptr<IRemoteObject> &remoteObject)
{
    APP_LOGI("FormTaskMgr AcquireTaskToHost, formId:%{public}" PRId64 "", formId);

    sptr<IFormHost> remoteFormHost = iface_cast<IFormHost>(remoteObject);
    if (remoteFormHost == nullptr) {
        APP_LOGE("%{public}s fail, Failed to get form host proxy", __func__);
        return;
    }

    APP_LOGD("FormTaskMgr remoteFormHost OnAcquired");
    remoteFormHost->OnAcquired(CreateFormJsInfo(formId, record));
}

/**
 * @brief Post form data to form host when update form.
 * @param formId The Id of the form.
 * @param callingUid Calling uid.
 * @param info Form configure info.
 * @param wantParams WantParams of the request.
 * @param remoteObject Form provider proxx object.
 */
void FormTaskMgr::UpdateTaskToHost(const int64_t formId, const FormRecord &record, 
    const sptr<IRemoteObject> &remoteObject)
{
    APP_LOGI("%{public}s start.", __func__);

    sptr<IFormHost> remoteFormHost = iface_cast<IFormHost>(remoteObject);
    if (remoteFormHost == nullptr) {
        APP_LOGE("%{public}s fail, Failed to get form host proxy.", __func__);
        return;
    }

    APP_LOGD("%{public}s, FormTaskMgr remoteFormHost OnUpdate.", __func__);
    remoteFormHost->OnUpdate(CreateFormJsInfo(formId, record));

    APP_LOGI("%{public}s end.", __func__);
}

/**
 * @brief Handle form host died.
 * @param remoteHost Form host proxy object.
 */
void FormTaskMgr::HostDied(const sptr<IRemoteObject> &remoteHost)
{
    APP_LOGI("%{public}s, remote client died event", __func__);
    if (remoteHost == nullptr) {
        APP_LOGI("%{public}s, remote client died, invalid param", __func__);
        return;
    }
    FormDataMgr::GetInstance().HandleHostDied(remoteHost);
}
/**
 * @brief Post provider batch delete.
 * @param formIds The Id list.
 * @param want The want of the request.
 * @param remoteObject Form provider proxy object.
 */
void FormTaskMgr::ProviderBatchDelete(std::set<int64_t> &formIds, const Want &want, 
    const sptr<IRemoteObject> &remoteObject)
{
    APP_LOGI("%{public}s called.", __func__);
    long connectId = want.GetLongParam(Constants::FORM_CONNECT_ID, 0);
    sptr<IFormProvider> formProviderProxy = iface_cast<IFormProvider>(remoteObject);
    if (formProviderProxy == nullptr) {
        FormSupplyCallback::GetInstance()->RemoveConnection(connectId);
        APP_LOGE("%{public}s fail, Failed to get formProviderProxy", __func__);
        return;
    } 
    std::vector<int64_t> vFormIds;
    vFormIds.assign(formIds.begin(), formIds.end());
    int error = formProviderProxy->NotifyFormsDelete(vFormIds, want, FormSupplyCallback::GetInstance());
    if (error != ERR_OK) {
        FormSupplyCallback::GetInstance()->RemoveConnection(connectId);
        APP_LOGE("%{public}s failed", __func__);
    }
}
/**
 * @brief Fire message event to form provider.
 * @param formId The Id of the from.
 * @param message Event message.
 * @param want The want of the request.
 * @param remoteObject Form provider proxy object.
 */
void FormTaskMgr::FireFormEvent(const int64_t formId, const std::string &message, const Want &want, 
    const sptr<IRemoteObject> &remoteObject)
{
    APP_LOGI("%{public}s start", __func__);
    long connectId = want.GetLongParam(Constants::FORM_CONNECT_ID, 0);
    sptr<IFormProvider> formProviderProxy = iface_cast<IFormProvider>(remoteObject);
    if (formProviderProxy == nullptr) {
        FormSupplyCallback::GetInstance()->RemoveConnection(connectId);
        APP_LOGE("%{public}s, Failed to get formProviderProxy", __func__);
        return;
    }

    int error = formProviderProxy->FireFormEvent(formId, message, want, FormSupplyCallback::GetInstance());
    if (error != ERR_OK) {
        FormSupplyCallback::GetInstance()->RemoveConnection(connectId);
        APP_LOGE("%{public}s, Failed to fire message event to form provider", __func__);
    }
    APP_LOGI("%{public}s end", __func__);
}

/**
 * @brief Handle uninstall message.
 * @param formIds The Id list of the forms.
 * @param remoteObject Form provider proxy object.
 */
void FormTaskMgr::FormUninstall(const std::vector<int64_t> &formIds, 
    const sptr<IRemoteObject> &remoteObject)
{
    APP_LOGI("%{public}s start", __func__);
    sptr<IFormHost> remoteFormHost = iface_cast<IFormHost>(remoteObject);
    if (remoteFormHost == nullptr) {
        APP_LOGE("%{public}s fail, Failed to get form host proxy.", __func__);
        return;
    }

    remoteFormHost->OnUninstall(formIds);

    APP_LOGI("%{public}s end", __func__);
}

/**
 * @brief Create form data for form host.
 * @param formId The Id of the form.
 * @param record Form record.
 * @return Form data.
 */
FormJsInfo FormTaskMgr::CreateFormJsInfo(const int64_t formId, const FormRecord &record)
{
    APP_LOGI("%{public}s start", __func__);
    FormJsInfo form;
    form.formId = formId;
    form.bundleName = record.bundleName;
    form.abilityName = record.abilityName;
    form.formName = record.formName;
    form.formTempFlg = record.formTempFlg;
    form.jsFormCodePath = record.jsFormCodePath;
    form.formData = record.formProviderInfo.GetFormDataString();
    form.formProviderData = record.formProviderInfo.GetFormData();
    form.formSrc = record.formSrc;
    form.formWindow = record.formWindow;
    APP_LOGI("%{public}s end, jsPath: %{public}s, data: %{public}s", __func__,
        form.jsFormCodePath.c_str(), form.formData.c_str());
    return form;
}
}  // namespace AppExecFwk
}  // namespace OHOS
