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

#ifndef FOUNDATION_APPEXECFWK_INTERFACES_INNERKITS_APPEXECFWK_CORE_INCLUDE_FORMMGR_FORM_MGR_INTERFACE_H
#define FOUNDATION_APPEXECFWK_INTERFACES_INNERKITS_APPEXECFWK_CORE_INCLUDE_FORMMGR_FORM_MGR_INTERFACE_H

#include <vector>
#include "form_js_info.h"
#include "form_provider_data.h"
#include "ipc_types.h"
#include "iremote_broker.h"

#include "want.h"

namespace OHOS {
namespace AppExecFwk {
using OHOS::AAFwk::Want;

/**
 * @class IFormMgr
 * IFormMgr interface is used to access form manager service.
 */
class IFormMgr : public OHOS::IRemoteBroker {
public:
    DECLARE_INTERFACE_DESCRIPTOR(u"ohos.appexecfwk.FormMgr")

    /**
     * @brief Add form with want, send want to form manager service.
     * @param formId The Id of the forms to add.
     * @param want The want of the form to add.
     * @param callerToken Caller ability token.
     * @param formInfo Form info.
     * @return Returns ERR_OK on success, others on failure.
     */
    virtual int AddForm(const int64_t formId, const Want &want, const sptr<IRemoteObject> &callerToken, 
    FormJsInfo &formInfo) = 0;

    /**
     * @brief Delete forms with formIds, send formIds to form manager service.
     * @param formId The Id of the forms to delete.
     * @param callerToken Caller ability token.
     * @return Returns ERR_OK on success, others on failure.
     */
    virtual int DeleteForm(const int64_t formId, const sptr<IRemoteObject> &callerToken) = 0;

    /**
     * @brief Release forms with formIds, send formIds to form manager service.
     * @param formId The Id of the forms to release.
     * @param callerToken Caller ability token.
     * @param delCache Delete Cache or not.
     * @return Returns ERR_OK on success, others on failure.
     */
    virtual int ReleaseForm(const int64_t formId, const sptr<IRemoteObject> &callerToken, const bool delCache) = 0;

    /**
     * @brief Update form with formId, send formId to form manager service.
     * @param formId The Id of the form to update.
     * @param bundleName Provider ability bundleName.
     * @param formProviderData Form binding data.
     * @return Returns ERR_OK on success, others on failure.
     */
    virtual int UpdateForm(const int64_t formId, const std::string &bundleName, 
    const FormProviderData &formProviderData) = 0;

    /**
     * @brief Set next refresh time.
     * @param formId The Id of the form to update.
     * @param nextTime Next refresh time.
     * @return Returns ERR_OK on success, others on failure.
     */
    virtual int SetNextRefreshTime(const int64_t formId, const int64_t nextTime) = 0;

    /**
     * @brief Lifecycle update.
     * @param formIds The Id of the forms.
     * @param callerToken Caller ability token.
     * @param updateType update type.
     * @return Returns ERR_OK on success, others on failure.
     */
    virtual int LifecycleUpdate(const std::vector<int64_t> &formIds, const sptr<IRemoteObject> &callerToken, 
    const int32_t updateType) = 0;

    /**
     * @brief Request form with formId and want, send formId and want to form manager service.
     * @param formId The Id of the form to request.
     * @param callerToken Caller ability token.
     * @param want The want of the form to add.
     * @return Returns ERR_OK on success, others on failure.
     */
    virtual int RequestForm(const int64_t formId, const sptr<IRemoteObject> &callerToken, const Want &want) = 0;

    /**
     * @brief Form visible/invisible notify, send formIds to form manager service.
     * @param formIds The Id list of the forms to notify.
     * @param callerToken Caller ability token.
     * @param formVisibleType The form visible type, including FORM_VISIBLE and FORM_INVISIBLE.
     * @return Returns ERR_OK on success, others on failure.
     */
    virtual int NotifyWhetherVisibleForms(const std::vector<int64_t> &formIds, const sptr<IRemoteObject> &callerToken, 
    const int32_t formVisibleType) = 0;
    
    /**
     * @brief temp form to normal form.
     * @param formId The Id of the form.
     * @param callerToken Caller ability token.
     * @return Returns ERR_OK on success, others on failure.
     */
    virtual int CastTempForm(const int64_t formId, const sptr<IRemoteObject> &callerToken) = 0;

    /**
     * @brief Dump all of form storage infos.
     * @param formInfos All of form storage infos.
     * @return Returns ERR_OK on success, others on failure.
     */
    virtual int DumpStorageFormInfos(std::string &formInfos) = 0;
    /**
     * @brief Dump form info by a bundle name.
     * @param bundleName The bundle name of form provider.
     * @param formInfos Form infos.
     * @return Returns ERR_OK on success, others on failure.
     */
    virtual int DumpFormInfoByBundleName(const std::string &bundleName, std::string &formInfos) = 0;
    /**
     * @brief Dump form info by a bundle name.
     * @param formId The id of the form.
     * @param formInfo Form info.
     * @return Returns ERR_OK on success, others on failure.
     */
    virtual int DumpFormInfoByFormId(const std::int64_t formId, std::string &formInfo) = 0;
    /**
     * @brief Dump form timer by form id.
     * @param formId The id of the form.
     * @param formInfo Form timer.
     * @return Returns ERR_OK on success, others on failure.
     */
    virtual int DumpFormTimerByFormId(const std::int64_t formId, std::string &isTimingService) = 0;
    /**
     * @brief Process js message event.
     * @param formId Indicates the unique id of form.
     * @param want information passed to supplier.
     * @param callerToken Caller ability token.
     * @return Returns true if execute success, false otherwise.
     */
    virtual int MessageEvent(const int64_t formId, const Want &want, const sptr<IRemoteObject> &callerToken) = 0;

    /**
     * @brief Batch add forms to form records for st limit value test.
     * @param want The want of the form to add.
     * @return Returns ERR_OK on success, others on failure.
     */
    virtual int BatchAddFormRecords(const Want &want) = 0;
    /**
     * @brief Clear form records for st limit value test.
     * @return Returns ERR_OK on success, others on failure.
     */
    virtual int ClearFormRecords() = 0;

    enum class Message {
        // ipc id 1-1000 for kit
        // ipc id 1001-2000 for DMS
        // ipc id 2001-3000 for tools
        // ipc id for create (3001)
        FORM_MGR_ADD_FORM = 3001,
        FORM_MGR_ADD_FORM_OHOS,
        FORM_MGR_DELETE_FORM,
        FORM_MGR_UPDATE_FORM,
        FORM_MGR_LIFECYCLE_UPDATE,
        FORM_MGR_REQUEST_FORM,
        FORM_MGR_RELEASE_FORM,
        FORM_MGR_RELEASE_CACHED_FORM,
        FORM_MGR_CAST_TEMP_FORM,
        FORM_MGR_EVENT_NOTIFY,
        FORM_MGR_CHECK_AND_DELETE_INVALID_FORMS,
        FORM_MGR_SET_NEXT_REFRESH_TIME,
        FORM_MGR_ACQUIRE_FORM_STATE,
        FORM_MGR_NOTIFY_FORM_WHETHER_VISIBLE,
        FORM_MGR_STORAGE_FORM_INFOS,
        FORM_MGR_FORM_INFOS_BY_NAME,
        FORM_MGR_FORM_INFOS_BY_ID,
        FORM_MGR_FORM_TIMER_INFO_BY_ID,
        FORM_MGR_MESSAGE_EVENT,
        FORM_MGR_BATCH_ADD_FORM_RECORDS_ST,
        FORM_MGR_CLEAR_FORM_RECORDS_ST,
    };
};
}  // namespace AppExecFwk
}  // namespace OHOS
#endif  // FOUNDATION_APPEXECFWK_INTERFACES_INNERKITS_APPEXECFWK_CORE_INCLUDE_FORMMGR_FORM_MGR_INTERFACE_H
