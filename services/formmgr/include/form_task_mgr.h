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

#ifndef FOUNDATION_APPEXECFWK_SERVICES_FORMMGR_INCLUDE_FORM_TASK_MGR_H
#define FOUNDATION_APPEXECFWK_SERVICES_FORMMGR_INCLUDE_FORM_TASK_MGR_H

#include <singleton.h>
#include <vector>

#include "event_handler.h"
#include "form_item_info.h"
#include "form_js_info.h"
#include "form_provider_info.h"
#include "form_record.h"
#include "ipc_types.h"
#include "iremote_object.h"
#include "form_record.h"
#include "want.h"

namespace OHOS {
namespace AppExecFwk {
using Want = OHOS::AAFwk::Want;
using WantParams = OHOS::AAFwk::WantParams;
/**
 * @class FormTaskMgr
 * form task manager.
 */
class FormTaskMgr final : public DelayedRefSingleton<FormTaskMgr> {
    DECLARE_DELAYED_REF_SINGLETON(FormTaskMgr)

public:
    DISALLOW_COPY_AND_MOVE(FormTaskMgr);

    /**
     * @brief SetEventHandler.
     * @param handler event handler
     */
    inline void SetEventHandler(const std::shared_ptr<AppExecFwk::EventHandler> &handler)
    {
        eventHandler_ = handler;
    }

    /**
     * @brief Acquire form data from form provider(task).
     * @param formId The Id of the form.
     * @param want The want of the request.
     * @param remoteObject Form provider proxy object.
     */
    void PostAcquireTask(const int64_t formId, const Want &want, const sptr<IRemoteObject> &remoteObject);

    /**
     * @brief Delete form data from form provider(task).
     * @param formId The Id of the form.
     * @param want The want of the request.
     * @param remoteObject Form provider proxy object.
     */
    void PostDeleteTask(const int64_t formId, const Want &want, const sptr<IRemoteObject> &remoteObject);
   /**
     * @brief Notify provider batch delete.
     * @param formIds The Id list.
     * @param want The want of the request.
     * @param remoteObject Form provider proxy object.
     */
    void PostProviderBatchDeleteTask(std::set<int64_t> &formIds, const Want &want, 
        const sptr<IRemoteObject> &remoteObject);
    /**
     * @brief Refresh form data from form provider(task).
     * 
     * @param formId The Id of the form.
     * @param want The want of the form.
     * @param remoteObject Form provider proxy object.
     * @return none.
     */
    void PostRefreshTask(const int64_t formId, const Want &want, const sptr<IRemoteObject> &remoteObject);

    /**
     * @brief Cast temp form data from form provider(task).
     * 
     * @param formId The Id of the form.
     * @param want The want of the request.
     * @param remoteObject Form provider proxy object.
     * @return none.
     */
    void PostCastTempTask(const int64_t formId, const Want &want, const sptr<IRemoteObject> &remoteObject);

    /**
     * @brief Post form data to form host(task) when acquire form.
     * @param formId The Id of the form.
     * @param callingUid Calling uid.
     * @param info Form configure info.
     * @param wantParams WantParams of the request.
     * @param remoteObject Form provider proxy object.
     */

    void PostAcquireTaskToHost(const int64_t formId, const FormRecord &record, const sptr<IRemoteObject> &remoteObject);

    /**
     * @brief Post form data to form host(task) when update form.
     * @param formId The Id of the form.
     * @param callingUid Calling uid.
     * @param info Form configure info.
     * @param wantParams WantParams of the request.
     * @param remoteObject Form provider proxy object.
     */
    void PostUpdateTaskToHost(const int64_t formId, const FormRecord &record, const sptr<IRemoteObject> &remoteObject);

    /**
     * @brief Handel form host died(task).
     * @param remoteHost Form host proxy object.
     */
    void PostHostDiedTask(const sptr<IRemoteObject> &remoteHost);

    /**
     * @brief Post event notify to form provider.
     * 
     * @param formEvent The vector of form ids.
     * @param formVisibleType The form visible type, including FORM_VISIBLE and FORM_INVISIBLE.
     * @param want The want of the form.
     * @param remoteObject The form provider proxy object.
     * @return none.
     */
    void PostEventNotifyTask(const std::vector<int64_t> &formEvent, const int32_t formVisibleType, const Want &want, 
        const sptr<IRemoteObject> &remoteObject);

    /**
     * @brief Post message event to form provider.
     * @param formId The Id of the from.
     * @param message Event message.
     * @param want The want of the request.
     * @param remoteObject Form provider proxy object.
     */
    void PostFormEventTask(const int64_t formId, const std::string &message, const Want &want, 
        const sptr<IRemoteObject> &remoteObject);

     /**
     * @brief Post uninstall message to form host(task).
     * @param formIds The Id list of the forms.
     * @param remoteObject Form provider proxy object.
     */
    void PostUninstallTaskToHost(const std::vector<int64_t> &formIds, const sptr<IRemoteObject> &remoteObject);
private:
    /**
     * @brief Acquire form data from form provider.
     * @param formId The Id of the from.
     * @param want The want of the request.
     * @param remoteObject Form provider proxy object.
     */
    void AcquireProviderFormInfo(const int64_t formId, const Want &want, const sptr<IRemoteObject> &remoteObject);

    /**
     * @brief Notify form provider for delete form.
     * @param formId The Id of the from.
     * @param want The want of the form.
     * @param remoteObject Form provider proxy object.
     * @return none.
     */
    void NotifyFormDelete(const int64_t formId, const Want &want, const sptr<IRemoteObject> &remoteObject);

    /**
     * @brief Notify form provider for updating form.
     * @param formId The Id of the from.
     * @param want The want of the form.
     * @param remoteObject Form provider proxy object.
     * @return none.
     */
    void NotifyFormUpdate(const int64_t formId, const Want &want, const sptr<IRemoteObject> &remoteObject);

    /**
     * @brief Event notify to form provider.
     * 
     * @param formEvents The vector of form ids.
     * @param formVisibleType The form visible type, including FORM_VISIBLE and FORM_INVISIBLE.
     * @param want The want of the form.
     * @param remoteObject The form provider proxy object.
     * @return none.
     */ 
    void EventNotify(const std::vector<int64_t> &formEvents, const int32_t formVisibleType, const Want &want, 
        const sptr<IRemoteObject> &remoteObject);

    /**
     * @brief Notify form provider for cast temp form.
     * 
     * @param formId The Id of the from.
     * @param want The want of the form.
     * @param remoteObject Form provider proxy object.
     * @return none.
     */
    void NotifyCastTemp(const int64_t formId, const Want &want, const sptr<IRemoteObject> &remoteObject);
    /**
     * @brief Post form data to form host when acquire form..
     * @param formId The Id of the form.
     * @param callingUid Calling uid.
     * @param info Form configure info.
     * @param wantParams WantParams of the request.
     * @param remoteObject Form provider proxy object.
     */
    void AcquireTaskToHost(const int64_t formId, const FormRecord &record, const sptr<IRemoteObject> &remoteObject);

    /**
     * @brief Post form data to form host when update form.
     * @param formId The Id of the form.
     * @param callingUid Calling uid.
     * @param info Form configure info.
     * @param wantParams WantParams of the request.
     * @param remoteObject Form provider proxy object.
     */
    void UpdateTaskToHost(const int64_t formId, const FormRecord &record, const sptr<IRemoteObject> &remoteObject);

    /**
     * @brief Handle form host died.
     * @param remoteHost Form host proxy object.
     */
    void HostDied(const sptr<IRemoteObject> &remoteHost);
    
    /**
     * @brief Post provider batch delete.
     * @param formIds The Id list.
     * @param want The want of the request.
     * @param remoteObject Form provider proxy object.
     */
    void ProviderBatchDelete(std::set<int64_t> &formIds, const Want &want, const sptr<IRemoteObject> &remoteObject);
    /**
     * @brief Fire message event to form provider.
     * @param formId The Id of the from.
     * @param message Event message.
     * @param want The want of the request.
     * @param remoteObject Form provider proxy object.
     */
    void FireFormEvent(const int64_t formId, const std::string &message, const Want &want, 
        const sptr<IRemoteObject> &remoteObject);

    /**
     * @brief Handle uninstall message.
     * @param formIds The Id list of the forms.
     * @param remoteObject Form provider proxy object.
     */
    void FormUninstall(const std::vector<int64_t> &formIds, const sptr<IRemoteObject> &remoteObject);

    /**
     * @brief Create form data for form host.
     * @param formId The Id of the form.
     * @param record Form record.
     * @return Form data.
     */
    FormJsInfo CreateFormJsInfo(const int64_t formId, const FormRecord &record);

private:
    std::shared_ptr<AppExecFwk::EventHandler> eventHandler_ = nullptr;
};
}  // namespace AppExecFwk
}  // namespace OHOS
#endif // FOUNDATION_APPEXECFWK_SERVICES_FORMMGR_INCLUDE_FORM_TASK_MGR_H
