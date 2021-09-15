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

#ifndef FOUNDATION_APPEXECFWK_INTERFACES_INNERKITS_APPEXECFWK_CORE_INCLUDE_FORMMGR_FORM_MGR_PROXY_H
#define FOUNDATION_APPEXECFWK_INTERFACES_INNERKITS_APPEXECFWK_CORE_INCLUDE_FORMMGR_FORM_MGR_PROXY_H

#include "form_mgr_interface.h"
#include "iremote_proxy.h"

namespace OHOS {
namespace AppExecFwk {
/**
 * @class FormMgrProxy
 * FormMgrProxy is used to access form manager service.
 */
class FormMgrProxy : public IRemoteProxy<IFormMgr> {
public:
    explicit FormMgrProxy(const sptr<IRemoteObject> &impl);
    virtual ~FormMgrProxy() = default;
    /**
     * @brief Add form with want, send want to form manager service.
     * @param formId The Id of the forms to add.
     * @param want The want of the form to add.
     * @param callerToken Caller ability token.
     * @param formInfo Form info.
     * @return Returns ERR_OK on success, others on failure.
     */
    virtual int AddForm(const int64_t formId, const Want &want, const sptr<IRemoteObject> &callerToken, 
    FormJsInfo &formInfo) override;

    /**
     * @brief Delete forms with formIds, send formIds to form manager service.
     * @param formId The Id of the forms to delete.
     * @param callerToken Caller ability token.
     * @return Returns ERR_OK on success, others on failure.
     */
    virtual int DeleteForm(const int64_t formId, const sptr<IRemoteObject> &callerToken) override;

     /**
     * @brief Release forms with formIds, send formIds to form manager service.
     * @param formId The Id of the forms to release.
     * @param callerToken Caller ability token.
     * @param delCache Delete Cache or not.
     * @return Returns ERR_OK on success, others on failure.
     */
    virtual int ReleaseForm(const int64_t formId, const sptr<IRemoteObject> &callerToken, const bool delCache) override;

    /**
     * @brief Update form with formId, send formId to form manager service.
     * @param formId The Id of the form to update.
     * @param bundleName Provider ability bundleName.
     * @param FormProviderData Form binding data.
     * @return Returns ERR_OK on success, others on failure.
     */
    virtual int UpdateForm(const int64_t formId, const std::string &bundleName, 
    const FormProviderData &FormProviderData) override;

    /**
     * @brief Set next refresh time.
     * @param formId The Id of the form to update.
     * @param nextTime Next refresh time.
     * @return Returns ERR_OK on success, others on failure.
     */
    virtual int SetNextRefreshTime(const int64_t formId, const int64_t nextTime) override;

    /**
     * @brief Lifecycle update.
     * @param formIds The Id of the forms.
     * @param callerToken Caller ability token.
     * @param updateType update type.
     * @return Returns ERR_OK on success, others on failure.
     */
    virtual int LifecycleUpdate(const std::vector<int64_t> &formIds, const sptr<IRemoteObject> &callerToken, 
    const int32_t updateType) override;

    /**
     * @brief Request form with formId and want, send formId and want to form manager service.
     * @param formId The Id of the form to update.
     * @param callerToken Caller ability token.
     * @param want The want of the form to add.
     * @return Returns ERR_OK on success, others on failure.
     */
    virtual int RequestForm(const int64_t formId, const sptr<IRemoteObject> &callerToken, const Want &want) override;

    /**
     * @brief Form visible/invisible notify, send formIds to form manager service.
     * @param formIds The Id list of the forms to notify.
     * @param callerToken Caller ability token.
     * @param formVisibleType The form visible type, including FORM_VISIBLE and FORM_INVISIBLE.
     * @return Returns ERR_OK on success, others on failure.
     */
    virtual int NotifyWhetherVisibleForms(const std::vector<int64_t> &formIds, const sptr<IRemoteObject> &callerToken, 
    const int32_t formVisibleType) override;
    
    /**
     * @brief temp form to normal form.
     * @param formId The Id of the form.
     * @param callerToken Caller ability token.
     * @return Returns ERR_OK on success, others on failure.
     */
    virtual int CastTempForm(const int64_t formId, const sptr<IRemoteObject> &callerToken) override;

    /**
     * @brief Dump all of form storage infos.
     * @param formInfos All of form storage infos.
     * @return Returns ERR_OK on success, others on failure.
     */
    virtual int DumpStorageFormInfos(std::string &formInfos) override;
    /**
     * @brief Dump form info by a bundle name.
     * @param bundleName The bundle name of form provider.
     * @param formInfos Form infos.
     * @return Returns ERR_OK on success, others on failure.
     */
    virtual int DumpFormInfoByBundleName(const std::string &bundleName, std::string &formInfos) override;
    /**
     * @brief Dump form info by a bundle name.
     * @param formId The id of the form.
     * @param formInfo Form info.
     * @return Returns ERR_OK on success, others on failure.
     */
    virtual int DumpFormInfoByFormId(const std::int64_t formId, std::string &formInfo) override;
    /**
     * @brief Process js message event.
     * @param formId Indicates the unique id of form.
     * @param want information passed to supplier.
     * @param callerToken Caller ability token.
     * @return Returns true if execute success, false otherwise.
     */
    virtual int MessageEvent(const int64_t formId, const Want &want, const sptr<IRemoteObject> &callerToken) override;
private:
    template<typename T>
    int GetParcelableInfos(MessageParcel &reply, std::vector<T> &parcelableInfos);
    bool WriteInterfaceToken(MessageParcel &data);
    template<typename T>
    int GetParcelableInfo(IFormMgr::Message code, MessageParcel &data, T &parcelableInfo);
    int SendTransactCmd(IFormMgr::Message code, MessageParcel &data, MessageParcel &reply);
    int GetStringInfo(IFormMgr::Message code, MessageParcel &data, std::string &stringInfo);
private:
    static inline BrokerDelegator<FormMgrProxy> delegator_;
};
}  // namespace AppExecFwk
}  // namespace OHOS
#endif  // FOUNDATION_APPEXECFWK_INTERFACES_INNERKITS_APPEXECFWK_CORE_INCLUDE_FORMMGR_FORM_MGR_PROXY_H
