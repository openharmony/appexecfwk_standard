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

#ifndef FOUNDATION_APPEXECFWK_INTERFACES_INNERKITS_APPEXECFWK_CORE_INCLUDE_FORMMGR_FORM_PROVIDER_PROXY_H
#define FOUNDATION_APPEXECFWK_INTERFACES_INNERKITS_APPEXECFWK_CORE_INCLUDE_FORMMGR_FORM_PROVIDER_PROXY_H

#include "app_log_wrapper.h"
#include "form_provider_interface.h"
#include "iremote_proxy.h"

namespace OHOS {
namespace AppExecFwk {
/**
 * @class FormProviderProxy
 * FormProviderProxy is used to access form provider service.
 */
class FormProviderProxy : public IRemoteProxy<IFormProvider> {
public:
    explicit FormProviderProxy(const sptr<IRemoteObject> &impl) : IRemoteProxy<IFormProvider>(impl)
    {}

    virtual ~FormProviderProxy() = default;

    /**
     * @brief Acquire to give back an ProviderFormInfo. This is sync API.
     * @param formId The Id of the from.
     * @param want Indicates the {@link Want} structure containing form info.
     * @param callerToken Caller ability token.
     * @return Returns ERR_OK on success, others on failure.
     */
    virtual int AcquireProviderFormInfo(const int64_t formId, const Want &want,
    const sptr<IRemoteObject> &callerToken) override;

    /**
     * @brief Notify provider when the form was deleted.
     * @param formId The Id of the form.
     * @param want Indicates the structure containing form info.
     * @param callerToken Caller ability token.
     * @return Returns ERR_OK on success, others on failure.
     */
    virtual int NotifyFormDelete(const int64_t formId, const Want &want,
    const sptr<IRemoteObject> &callerToken) override;

    /**
     * @brief Notify provider when the forms was deleted.
     * @param formIds The id list of forms.
     * @param want Indicates the structure containing form info.
     * @param callerToken Caller ability token.
     * @return Returns ERR_OK on success, others on failure.
     */
    virtual int NotifyFormsDelete(const std::vector<int64_t> &formIds, const Want &want,
    const sptr<IRemoteObject> &callerToken) override;

    /**
     * @brief Notify provider when the form need update.
     * @param formId The Id of the form.
     * @param want Indicates the structure containing form info.
     * @param callerToken Caller ability token.
     * @return Returns ERR_OK on success, others on failure.
     */
    virtual int NotifyFormUpdate(const int64_t formId, const Want &want,
    const sptr<IRemoteObject> &callerToken) override;

    /**
     * @brief Event notify when change the form visible.
     *
     * @param formIds The vector of form ids.
     * @param formVisibleType The form visible type, including FORM_VISIBLE and FORM_INVISIBLE.
     * @param want Indicates the structure containing form info.
     * @param callerToken Caller ability token.
     * @return Returns ERR_OK on success, others on failure.
     */
    virtual int EventNotify(const std::vector<int64_t> &formIds, const int32_t formVisibleType,
        const Want &want, const sptr<IRemoteObject> &callerToken) override;

    /**
     * @brief Notify provider when the temp form was cast to normal form.
     * @param formId The Id of the form to update.
     * @param want Indicates the structure containing form info.
     * @param callerToken Caller ability token.
     * @return Returns ERR_OK on success, others on failure.
     */
    virtual int NotifyFormCastTempForm(const int64_t formId, const Want &want,
    const sptr<IRemoteObject> &callerToken) override;
    /**
     * @brief Fire message event to form provider.
     * @param formId The Id of the from.
     * @param message Event message.
     * @param want The want of the request.
     * @param callerToken Form provider proxy object.
     * @return Returns ERR_OK on success, others on failure.
     */
    virtual int FireFormEvent(const int64_t formId, const std::string &message, const Want &want,
    const sptr<IRemoteObject> &callerToken) override;

private:
    template<typename T>
    int GetParcelableInfos(MessageParcel &reply, std::vector<T> &parcelableInfos);
    bool WriteInterfaceToken(MessageParcel &data);

private:
    static inline BrokerDelegator<FormProviderProxy> delegator_;
};
}  // namespace AppExecFwk
}  // namespace OHOS
#endif  // FOUNDATION_APPEXECFWK_INTERFACES_INNERKITS_APPEXECFWK_CORE_INCLUDE_FORMMGR_FORM_PROVIDER_PROXY_H
