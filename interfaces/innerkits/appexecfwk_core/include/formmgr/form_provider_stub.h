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

#ifndef FOUNDATION_APPEXECFWK_INTERFACES_INNERKITS_APPEXECFWK_CORE_INCLUDE_FORMMGR_FORM_PROVIDER_STUB_H
#define FOUNDATION_APPEXECFWK_INTERFACES_INNERKITS_APPEXECFWK_CORE_INCLUDE_FORMMGR_FORM_PROVIDER_STUB_H

#include <map>

#include "form_provider_interface.h"
#include "iremote_object.h"
#include "iremote_stub.h"

namespace OHOS {
namespace AppExecFwk {
/**
 * @class FormProviderStub
 * form provider service stub.
 */
class FormProviderStub : public IRemoteStub<IFormProvider> {
public:
    FormProviderStub();
    virtual ~FormProviderStub();
    /**
     * @brief handle remote request.
     * @param data input param.
     * @param reply output param.
     * @param option message option.
     * @return Returns ERR_OK on success, others on failure.
     */
    virtual int OnRemoteRequest(
        uint32_t code, MessageParcel &data, MessageParcel &reply, MessageOption &option);

private:
    /**
     * @brief handle AcquireProviderFormInfo message.
     * @param data input param.
     * @param reply output param.
     * @return Returns ERR_OK on success, others on failure.
     */
    int HandleAcquireProviderFormInfo(MessageParcel &data, MessageParcel &reply);
    /**
     * @brief handle NotifyFormDelete message.
     * @param data input param.
     * @param reply output param.
     * @return Returns ERR_OK on success, others on failure.
     */
    int HandleNotifyFormDelete(MessageParcel &data, MessageParcel &reply);
    /**
     * @brief handle NotifyFormsDelete message.
     * @param data input param.
     * @param reply output param.
     * @return Returns ERR_OK on success, others on failure.
     */
    int HandleNotifyFormsDelete(MessageParcel &data, MessageParcel &reply);
    /**
     * @brief handle NotifyFormUpdate message.
     * @param data input param.
     * @param reply output param.
     * @return Returns ERR_OK on success, others on failure.
     */
    int HandleNotifyFormUpdate(MessageParcel &data, MessageParcel &reply);

    /**
     * @brief handle EventNotify message.
     * @param data input param.
     * @param reply output param.
     * @return Returns ERR_OK on success, others on failure.
     */
    int HandleEventNotify(MessageParcel &data, MessageParcel &reply);

    /**
     * @brief handle NotifyFormCastTempForm message.
     * @param data input param.
     * @param reply output param.
     * @return Returns ERR_OK on success, others on failure.
     */
    int HandleNotifyFormCastTempForm(MessageParcel &data, MessageParcel &reply);
    /**
     * @brief handle NotifyFormCastTempForm message.
     * @param data input param.
     * @param reply output param.
     * @return Returns ERR_OK on success, others on failure.
     */
    int HandleFireFormEvent(MessageParcel &data, MessageParcel &reply);
private:
    using FormProviderFunc = int32_t (FormProviderStub::*)(MessageParcel &data, MessageParcel &reply);
    std::map<uint32_t, FormProviderFunc> memberFuncMap_;

    DISALLOW_COPY_AND_MOVE(FormProviderStub);
};
}  // namespace AppExecFwk
}  // namespace OHOS
#endif  // FOUNDATION_APPEXECFWK_INTERFACES_INNERKITS_APPEXECFWK_CORE_INCLUDE_FORMMGR_FORM_PROVIDER_STUB_H
