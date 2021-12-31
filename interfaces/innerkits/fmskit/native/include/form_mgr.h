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

#ifndef FOUNDATION_APPEXECFWK_OHOS_FORM_MGR_H
#define FOUNDATION_APPEXECFWK_OHOS_FORM_MGR_H

#include <mutex>
#include <singleton.h>

#include "form_callback_interface.h"
#include "form_constants.h"
#include "form_death_callback.h"
#include "form_js_info.h"
#include "form_mgr_interface.h"
#include "form_provider_data.h"
#include "iremote_object.h"
#include "want.h"

namespace OHOS {
namespace AppExecFwk {
using OHOS::AAFwk::Want;

static volatile int recoverStatus_ = Constants::NOT_IN_RECOVERY;

/**
 * @class FormMgr
 * FormMgr is used to access form manager services.
 */
class FormMgr final : public DelayedRefSingleton<FormMgr> {
    DECLARE_DELAYED_REF_SINGLETON(FormMgr)
public:
    DISALLOW_COPY_AND_MOVE(FormMgr);
    /**
     * @brief Add form with want, send want to form manager service.
     * @param formId The Id of the forms to add.
     * @param want The want of the form to add.
     * @param callerToken Caller ability token.
     * @param formInfo Form info.
     * @return Returns ERR_OK on success, others on failure.
     */
    int AddForm(const int64_t formId, const Want &want, const sptr<IRemoteObject> &callerToken,
        FormJsInfo &formInfo);

    /**
     * @brief Delete forms with formIds, send formIds to form manager service.
     * @param formId The Id of the forms to delete.
     * @param callerToken Caller ability token.
     * @return Returns ERR_OK on success, others on failure.
     */
    int DeleteForm(const int64_t formId, const sptr<IRemoteObject> &callerToken);

    /**
     * @brief Release forms with formIds, send formIds to form manager service.
     * @param formId The Id of the forms to release.
     * @param callerToken Caller ability token.
     * @param delCache Delete Cache or not.
     * @return Returns ERR_OK on success, others on failure.
     */
    int ReleaseForm(const int64_t formId, const sptr<IRemoteObject> &callerToken, const bool delCache);

    /**
     * @brief Update form with formId, send formId to form manager service.
     * @param formId The Id of the form to update.
     * @param bundleName Provider ability bundleName.
     * @param formBindingData Form binding data.
     * @return Returns ERR_OK on success, others on failure.
     */
    int UpdateForm(const int64_t formId, const std::string &bundleName, const FormProviderData &formBindingData);

    /**
     * @brief Notify the form service that the form user's lifecycle is updated.
     *
     * This should be called when form user request form.
     *
     * @param formId Indicates the unique id of form.
     * @param callerToken Indicates the callback remote object of specified form user.
     * @param want information passed to supplier.
     * @return Returns true if execute success, false otherwise.
     */
    int RequestForm(const int64_t formId, const sptr<IRemoteObject> &callerToken, const Want &want);

    /**
     * @brief Form visible/invisible notify, send formIds to form manager service.
     * @param formIds The Id list of the forms to notify.
     * @param callerToken Caller ability token.
     * @param formVisibleType The form visible type, including FORM_VISIBLE and FORM_INVISIBLE.
     * @return Returns ERR_OK on success, others on failure.
     */
    int NotifyWhetherVisibleForms(const std::vector<int64_t> &formIds, const sptr<IRemoteObject> &callerToken, 
    const int32_t formVisibleType);
    
    /**
     * @brief temp form to normal form.
     * @param formId The Id of the form.
     * @param callerToken Caller ability token.
     * @return None.
     */
    int CastTempForm(const int64_t formId, const sptr<IRemoteObject> &callerToken);

    /**
     * @brief Dump all of form storage infos.
     * @param formInfos All of form storage infos.
     * @return Returns ERR_OK on success, others on failure.
     */
    int DumpStorageFormInfos(std::string &formInfos);
    /**
     * @brief Dump form info by a bundle name.
     * @param bundleName The bundle name of form provider.
     * @param formInfos Form infos.
     * @return Returns ERR_OK on success, others on failure.
     */
    int DumpFormInfoByBundleName(const std::string bundleName, std::string &formInfos);
    /**
     * @brief Dump form info by a bundle name.
     * @param formId The id of the form.
     * @param formInfo Form info.
     * @return Returns ERR_OK on success, others on failure.
     */
    int DumpFormInfoByFormId(const std::int64_t formId, std::string &formInfo);
    /**
     * @brief Dump form timer by form id.
     * @param formId The id of the form.
     * @param formInfo Form timer.
     * @return Returns ERR_OK on success, others on failure.
     */
    int DumpFormTimerByFormId(const std::int64_t formId, std::string &isTimingService);
    /**
     * @brief Process js message event.
     * @param formId Indicates the unique id of form.
     * @param want information passed to supplier.
     * @param callerToken Caller ability token.
     * @return Returns true if execute success, false otherwise.
     */
    int MessageEvent(const int64_t formId, const Want &want, const sptr<IRemoteObject> &callerToken);

    /**
     * @brief Get fms recoverStatus.
     *
     * @return The current recover status.
     */
    static int GetRecoverStatus();

    /**
     * @brief Register death callback.
     * 
     * @param formDeathCallback The death callback.
     */
    void RegisterDeathCallback(const std::shared_ptr<FormCallbackInterface> &formDeathCallback);

    /**
     * @brief UnRegister death callback.
     *
     * @param formDeathCallback The death callback.
     */
    void UnRegisterDeathCallback(const std::shared_ptr<FormCallbackInterface> &formDeathCallback);

    /**
     * @brief Set the next refresh time
     *
     * @param formId The id of the form.
     * @param nextTime Next refresh time
     * @return Returns ERR_OK on success, others on failure.
     */
    int SetNextRefreshTime(const int64_t formId, const int64_t nextTime);

    /**
     * @brief Lifecycle Update.
     * @param formIds The id of the forms.
     * @param callerToken Host client.
     * @param updateType Next refresh time.
     * @return Returns ERR_OK on success, others on failure.
     */
    int LifecycleUpdate(const std::vector<int64_t> &formIds, const sptr<IRemoteObject> &callerToken,
                        const int32_t updateType);

    /**
     * @brief Set fms recoverStatus.
     *
     * @param recoverStatus The recover status.
     */
    static void SetRecoverStatus(int recoverStatus);

    /**
     * @brief Set form mgr service for test.
     */
    void SetFormMgrService(sptr<IFormMgr> formMgrService);

    /**
     * @brief Get death recipient.
     * @return deathRecipient_.
     */
    sptr<IRemoteObject::DeathRecipient> GetDeathRecipient() const;

    /**
     * @brief Check whether the specified death callback is registered in form mgr.
     * @param formDeathCallback The specified death callback for checking.
     * @return Return true on success, false on failure.
     */
    bool CheckIsDeathCallbackRegistered(const std::shared_ptr<FormCallbackInterface> &formDeathCallback);

    /**
     * @brief Get the error message content.
     *
     * @param errCode Error code.
     * @return Message content.
     */
    std::string GetErrorMessage(int errCode);

    /**
     * @brief  Add forms to storage for st .
     * @param Want The Want of the form to add.
     * @return Returns ERR_OK on success, others on failure.
     */
    int DistributedDataAddForm(const Want &want);

    /**
     * @brief  Delete form form storage for st.
     * @param formId The formId of the form to delete.
     * @return Returns ERR_OK on success, others on failure.
     */
    int DistributedDataDeleteForm(const std::string &formId);
private:
    /**
     * @brief Connect form manager service.
     * @return Returns ERR_OK on success, others on failure.
     */
    ErrCode Connect();

    /**
    * @brief Reconnect form manager service once per 1000 milliseconds,
    *        until the connection succeeds or reaching the max retry times.
    * @return Returns true if execute success, false otherwise.
    */
    bool Reconnect();
    /**
     * @brief Reset proxy.
     * @param remote remote object.
     */
    void ResetProxy(const wptr<IRemoteObject> &remote);

public:
    friend class FormMgrDeathRecipient;

private:
    /**
     * @class FormMgrDeathRecipient
     * FormMgrDeathRecipient notices IRemoteBroker died.
     */
    class FormMgrDeathRecipient : public IRemoteObject::DeathRecipient {
    public:
        FormMgrDeathRecipient() = default;
        ~FormMgrDeathRecipient() = default;

        /**
         * @brief Notices IRemoteBroker died.
         * @param remote remote object.
         */
        void OnRemoteDied(const wptr<IRemoteObject> &remote) override;
    private:
        DISALLOW_COPY_AND_MOVE(FormMgrDeathRecipient);
    };

    std::mutex connectMutex_;
    sptr<IFormMgr> remoteProxy_;

    sptr<IRemoteObject::DeathRecipient> deathRecipient_ {nullptr};
    
    // True: need to get a new fms remote object,
    // False: no need to get a new fms remote object.
    volatile bool resetFlag_ = false;

    std::vector<std::shared_ptr<FormCallbackInterface>> formDeathCallbacks_;
};
}  // namespace AppExecFwk
}  // namespace OHOS
#endif  // FOUNDATION_APPEXECFWK_OHOS_FORM_MGR_H