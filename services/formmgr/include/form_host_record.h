/*
 * Copyright (c) 2021 Huawei Device Co., Ltd.
 * Licensed under the Apache License, Version 2.0 (the "License")_;
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

#ifndef FOUNDATION_APPEXECFWK_SERVICES_FORMMGR_INCLUDE_FORM_HOST_RECORD_H
#define FOUNDATION_APPEXECFWK_SERVICES_FORMMGR_INCLUDE_FORM_HOST_RECORD_H

#include <stdint.h>
#include <unordered_map>
#include <vector>
#include "app_log_wrapper.h"
#include "form_host_callback.h"
#include "form_item_info.h"
#include "form_record.h"
#include "iremote_object.h"

namespace OHOS {
namespace AppExecFwk {
/**
 * @class FormHostRecord
 * Form host data.
 */
class FormHostRecord {
public:
    /**
     * @brief Create form host record.
     * @param callback remote object.
     * @param callingUid Calling uid.
     */
    static FormHostRecord CreateRecord(const FormItemInfo &info, const sptr<IRemoteObject> &callback, int callingUid);
    /**
     * @brief Add form id.
     * @param formId The Id of the form.
     */
    void AddForm(int64_t formId);
    /**
     * @brief Delete form id.
     * @param formId The Id of the form.
     */
    void DelForm(int64_t formId);
    /**
     * @brief forms_ is empty or not.
     * @return forms_ is empty or not.
     */
    bool IsEmpty() const;
    /**
     * @brief formId is in forms_ or not.
     * @param formId The Id of the form.
     * @return formId is in forms_ or not.
     */
    bool Contains(int64_t formId) const;

    /**
     * @brief Set refresh enable flag.
     * @param formId The Id of the form.
     * @param flag True for enbale, false for disable.
     */
    void SetEnableRefresh(int64_t formId, bool flag);

    /**
     * @brief Refresh enable or not.
     * @param formId The Id of the form.
     * @return true on enbale, false on disable..
     */
    bool IsEnableRefresh(int64_t formId) const;

    /**
     * @brief Set need refresh enable flag.
     * @param formId The Id of the form.
     * @param flag True for enbale, false for disable.
     */
    void SetNeedRefresh(int64_t formId, bool flag);
    /**
     * @brief Need Refresh enable or not.
     * @param formId The Id of the form.
     * @return true on enbale, false on disable..
     */
    bool IsNeedRefresh(int64_t formId) const;

    /**
     * @brief Send form data to form host.
     * @param id The Id of the form.
     * @param record Form record.
     */
    void OnAcquire(int64_t id, const FormRecord &record);

    /**
     * @brief Update form data to form host.
     * @param id The Id of the form.
     * @param record Form record.
     */
    void OnUpdate(int64_t id, const FormRecord &record);
    /**
     * Send form uninstall message to form host.
     *
     * @param id The Id of the form.
     * @param record Form record.
     */
    void OnFormUninstalled(std::vector<int64_t> &formIds);

    /**
     * @brief Release resource.
     * @param id The Id of the form.
     * @param record Form record.
     */
    void CleanResource();
    /**
     * @brief Get callerUid_.
     * @return callerUid_.
     */
    int GetCallerUid() const 
    {
        return callerUid_;
    }
    /**
     * @brief Get clientStub_.
     * @return clientStub_.
     */
    sptr<IRemoteObject> GetClientStub() const;
    /**
     * @brief Get deathRecipient_.
     * @return deathRecipient_.
     */
    sptr<IRemoteObject::DeathRecipient> GetDeathRecipient() const;
    /**
     * @brief Set value of callerUid_.
     * @param callerUid Caller uid.
     */
    void SetCallerUid(const int callerUid);
    /**
     * @brief Set value of clientStub_.
     * @param clientStub remote object.
     */
    void SetClientStub(const sptr<IRemoteObject> &clientStub);
    /**
     * @brief Set value of clientImpl_.
     * @param clientImpl Form host callback object.
     */
    void SetClientImpl(const std::shared_ptr<FormHostCallback> &clientImpl);
    /**
     * @brief Set value of deathRecipient_.
     * @param clientImpl DeathRecipient object.
     */
    void SetDeathRecipient(const sptr<IRemoteObject::DeathRecipient> &deathRecipient);
    /**
     * @brief Add deathRecipient object to clientStub_.
     * @param deathRecipient DeathRecipient object.
     */
    void AddDeathRecipient(const sptr<IRemoteObject::DeathRecipient>& deathRecipient);
    /**
     * @brief Get hostBundleName_.
     * @return hostBundleName_.
     */
    std::string GetHostBundleName() const;
    /**
     * @brief Set hostBundleName_.
     * @param hostBandleName Host bundle name.
     */
    void SetHostBundleName(const std::string &hostBundleName);

private:
    int callerUid_ = 0;
    sptr<IRemoteObject> clientStub_ = nullptr;
    std::shared_ptr<FormHostCallback>  clientImpl_ = nullptr;
    sptr<IRemoteObject::DeathRecipient> deathRecipient_ = nullptr;
    std::unordered_map<int64_t, bool> forms_;
    std::unordered_map<int64_t, bool> needRefresh_;
    std::string hostBundleName_ = "";

    /**
     * @class ClientDeathRecipient
     * notices IRemoteBroker died.
     */
    class ClientDeathRecipient : public IRemoteObject::DeathRecipient {
    public:
        /**
         * @brief Constructor
         */
        ClientDeathRecipient() = default;
        ~ClientDeathRecipient() = default;
        /**
         * @brief handle remote object died event.
         * @param remote remote object.
         */
        void OnRemoteDied(const wptr<IRemoteObject> &remote) override;
    };
};
}  // namespace AppExecFwk
}  // namespace OHOS

#endif // FOUNDATION_APPEXECFWK_SERVICES_FORMMGR_INCLUDE_FORM_HOST_RECORD_H