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

#ifndef FOUNDATION_APPEXECFWK_SERVICES_FORMMGR_INCLUDE_FORM_MGR_SERVICE_H
#define FOUNDATION_APPEXECFWK_SERVICES_FORMMGR_INCLUDE_FORM_MGR_SERVICE_H

#include <codecvt>
#include <memory>
#include <singleton.h>
#include <system_ability.h>
#include <thread_ex.h>
#include <unordered_map>

#include "event_handler.h"
#include "form_mgr_stub.h"
#include "form_provider_data.h"
#include "form_sys_event_receiver.h"
#include "iremote_object.h"

namespace OHOS {
namespace AppExecFwk {
enum class ServiceRunningState { STATE_NOT_START, STATE_RUNNING };
/**
 * @class FormMgrService
 * FormMgrService provides a facility for managing form life cycle.
 */
class FormMgrService : public SystemAbility,
                       public FormMgrStub,
                       public std::enable_shared_from_this<FormMgrService> {
    DECLARE_DELAYED_SINGLETON(FormMgrService);
    DECLEAR_SYSTEM_ABILITY(FormMgrService);
public:
    /**
     * @brief Start envent for the form manager service.
     */
    void OnStart() override;
    /**
     * @brief Stop envent for the form manager service.
     */
    void OnStop() override;

    /**
     * @brief Add form with want, send want to form manager service.
     * @param formId The Id of the forms to add.
     * @param want The want of the form to add.
     * @param callerToken Caller ability token.
     * @param formInfo Form info.
     * @return Returns ERR_OK on success, others on failure.
     */
    int AddForm(const int64_t formId, const Want &want, const sptr<IRemoteObject> &callerToken,
    FormJsInfo &formInfo) override;

    /**
     * @brief Delete forms with formIds, send formIds to form manager service.
     * @param formId The Id of the forms to delete.
     * @param callerToken Caller ability token.
     * @return Returns ERR_OK on success, others on failure.
     */
    int DeleteForm(const int64_t formId, const sptr<IRemoteObject> &callerToken) override;

    /**
     * @brief Release forms with formIds, send formIds to form manager service.
     * @param formId The Id of the forms to release.
     * @param callerToken Caller ability token.
     * @param delCache Delete Cache or not.
     * @return Returns ERR_OK on success, others on failure.
     */
    int ReleaseForm(const int64_t formId, const sptr<IRemoteObject> &callerToken, const bool delCache) override;

    /**
     * @brief Update form with formId, send formId to form manager service.
     * @param formId The Id of the form to update.
     * @param bundleName Provider ability bundleName.
     * @param FormProviderData Form binding data.
     * @return Returns ERR_OK on success, others on failure.
     */
    int UpdateForm(const int64_t formId, const std::string &bundleName,
    const FormProviderData &FormProviderData) override;

    /**
     * @brief set next refresh time.
     * @param formId The id of the form.
     * @param nextTime next refresh time.
     * @return Returns ERR_OK on success, others on failure.
     */
    int SetNextRefreshTime(const int64_t formId, const int64_t nextTime) override;

    /**
     * @brief lifecycle update.
     * @param formIds formIds of hostclient.
     * @param callerToken Caller ability token.
     * @param updateType update type,enable or disable.
     * @return Returns true on success, false on failure.
     */
    int LifecycleUpdate(const std::vector<int64_t> &formIds, const sptr<IRemoteObject> &callerToken,
    const int32_t updateType) override;

    /**
     * @brief Request form with formId and want, send formId and want to form manager service.
     * @param formId The Id of the form to update.
     * @param callerToken Caller ability token.
     * @param want The want of the form to add.
     * @return Returns ERR_OK on success, others on failure.
     */
    int RequestForm(const int64_t formId, const sptr<IRemoteObject> &callerToken, const Want &want) override;

    /**
     * @brief Form visible/invisible notify, send formIds to form manager service.
     * @param formIds The Id list of the forms to notify.
     * @param callerToken Caller ability token.
     * @param formVisibleType The form visible type, including FORM_VISIBLE and FORM_INVISIBLE.
     * @return Returns ERR_OK on success, others on failure.
     */
    int NotifyWhetherVisibleForms(const std::vector<int64_t> &formIds, const sptr<IRemoteObject> &callerToken,
    const int32_t formVisibleType) override;

    /**
     * @brief temp form to normal form.
     * @param formId The Id of the form.
     * @param callerToken Caller ability token.
     * @return Returns ERR_OK on success, others on failure.
     */
    int CastTempForm(const int64_t formId, const sptr<IRemoteObject> &callerToken) override;

    /**
     * @brief Dump all of form storage infos.
     * @param formInfos All of form storage infos.
     * @return Returns ERR_OK on success, others on failure.
     */
    int DumpStorageFormInfos(std::string &formInfos) override;
    /**
     * @brief Dump form info by a bundle name.
     * @param bundleName The bundle name of form provider.
     * @param formInfos Form infos.
     * @return Returns ERR_OK on success, others on failure.
     */
    int DumpFormInfoByBundleName(const std::string &bundleName, std::string &formInfos) override;
    /**
     * @brief Dump form info by a bundle name.
     * @param formId The id of the form.
     * @param formInfo Form info.
     * @return Returns ERR_OK on success, others on failure.
     */
    int DumpFormInfoByFormId(const std::int64_t formId, std::string &formInfo) override;
    /**
     * @brief Dump form timer by form id.
     * @param formId The id of the form.
     * @param formInfo Form info.
     * @return Returns ERR_OK on success, others on failure.
     */
    int DumpFormTimerByFormId(const std::int64_t formId, std::string &isTimingService) override;
    /**
     * @brief Process js message event.
     * @param formId Indicates the unique id of form.
     * @param want information passed to supplier.
     * @param callerToken Caller ability token.
     * @return Returns true if execute success, false otherwise.
     */
    int MessageEvent(const int64_t formId, const Want &want, const sptr<IRemoteObject> &callerToken) override;

    /**
     * @brief Batch add forms to form records for st limit value test.
     * @param want The want of the form to add.
     * @return Returns ERR_OK on success, others on failure.
     */
    int BatchAddFormRecords(const Want &want) override;
    /**
     * @brief Clear form records for st limit value test.
     * @return Returns ERR_OK on success, others on failure.
     */
    int ClearFormRecords() override;

    /**
     * @brief Check whether if the form manager service is ready.
     * @return Returns true if the form manager service is ready; returns false otherwise.
     */
    bool IsReady() const;

    /**
     * @brief  Add forms to storage for st .
     * @param Want The Want of the form to add.
     * @return Returns ERR_OK on success, others on failure.
     */
    int DistributedDataAddForm(const Want &want) override;

    /**
     * @brief  Delete form form storage for st.
     * @param formId The formId of the form to delete.
     * @return Returns ERR_OK on success, others on failure.
     */
    int DistributedDataDeleteForm(const std::string &formId) override;
private:
    /**
     * @brief initialization of form manager service.
     */
    ErrCode Init();

    /**
     * @brief Permission check by callingUid.
     * @param formId the id of the form.
     * @return Returns true on success, false on failure.
     */
    bool CheckFormPermission();

    /**
     * @brief Permission check.
     * @param bundleName bundleName.
     * @return Returns true on success, false on failure.
     */
    bool CheckFormPermission(const std::string &bundleName) const;
private:
    ServiceRunningState state_;

    std::shared_ptr<EventRunner> runner_ = nullptr;
    std::shared_ptr<EventHandler> handler_ = nullptr;
    std::shared_ptr<FormSysEventReceiver> formSysEventReceiver_ = nullptr;

    bool resetFlag = false;

    mutable std::mutex instanceMutex_;

    sptr<IRemoteObject> remote = nullptr;

    static const int32_t ENABLE_FORM_UPDATE = 5;

    DISALLOW_COPY_AND_MOVE(FormMgrService);
};
static bool resetFlag = false;
}  // namespace AppExecFwk
}  // namespace OHOS
#endif  // FOUNDATION_APPEXECFWK_SERVICES_FORMMGR_INCLUDE_FORM_MGR_SERVICE_H
