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

#ifndef FOUNDATION_APPEXECFWK_OHOS_FORM_PROVIDER_CLIENT_H
#define FOUNDATION_APPEXECFWK_OHOS_FORM_PROVIDER_CLIENT_H

#include <map>
#include <iremote_object.h>
#include <iremote_stub.h>
#include "form_provider_stub.h"

#include "want.h"

namespace OHOS {
namespace AppExecFwk {
/**
 * @class FormProviderStub
 * FormProviderStub.
 */
class MockFormProviderClient : public FormProviderStub {
public:
    MockFormProviderClient() = default;
    virtual ~MockFormProviderClient() = default;

private:
    /**
     * Acquire to give back an ProviderFormInfo. This is sync API.
     *
     * @param want, Indicates the {@link Want} structure containing form info.
     * @param callerToken, Caller ability token.
     * @return none.
     */
    virtual int AcquireProviderFormInfo(const int64_t formId, const Want &want, 
    const sptr<IRemoteObject> &callerToken) override;

    /**
     * Notify provider when the form was deleted.
     *
     * @param formId, The Id of the form.
     * @param callerToken, Caller ability token.
     * @return none.
     */
    virtual int NotifyFormDelete(const int64_t formId, const Want &want, 
    const sptr<IRemoteObject> &callerToken) override;

    /**
     * Notify provider when the form was deleted.
     *
     * @param formIds, The id list of forms.
     * @param want Indicates the structure containing form info.
     * @param callerToken, Caller ability token.
     * @return none.
     */
    virtual int NotifyFormsDelete(const std::vector<int64_t> &formIds, const Want &want, 
    const sptr<IRemoteObject> &callerToken) override;
    
    /**
     * @brief Notify provider when the form need update.
     * @param formId The Id of the form.
     * @param want Indicates the structure containing form info.
     * @param callerToken Caller ability token.
     */
    virtual int NotifyFormUpdate(const int64_t formId, const Want &want, 
    const sptr<IRemoteObject> &callerToken) override;

    /**
     * @brief Event notify when change the form visible.
     * 
     * @param formEvents The vector of form ids.
     * @param formVisibleType The form visible type, including FORM_VISIBLE and FORM_INVISIBLE.
     * @param want Indicates the structure containing form info.
     * @param callerToken Caller ability token.
     * @return Returns ERR_OK on success, others on failure.
     */
    virtual int EventNotify(const std::vector<int64_t> &formIds, const int32_t formVisibleType,
        const Want &want, const sptr<IRemoteObject> &callerToken) override;

    /**
     * Notify provider when the temp form was cast to normal form.
     *
     * @param formId, The Id of the form to update.
     * @param callerToken, Caller ability token.
     * @return none.
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
    DISALLOW_COPY_AND_MOVE(MockFormProviderClient);
};
}  // namespace AppExecFwk
}  // namespace OHOS
#endif  // FOUNDATION_APPEXECFWK_OHOS_FORM_PROVIDER_CLIENT_H
