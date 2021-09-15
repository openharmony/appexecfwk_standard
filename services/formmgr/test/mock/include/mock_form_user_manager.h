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

#ifndef FOUNDATION_APPEXECFWK_SERVICES_FORMMGR_TEST_MOCK_INCLUDE_MOCK_FORM_USER_MANAGER_H
#define FOUNDATION_APPEXECFWK_SERVICES_FORMMGR_TEST_MOCK_INCLUDE_MOCK_FORM_USER_MANAGER_H


#include <vector>

#include "form_info.h"
#include "iremote_proxy.h"
#include "iremote_stub.h"
#include "form_mgr_interface.h"

namespace OHOS {
namespace AppExecFwk {
class IFormUserMgr : public IRemoteBroker {
public:
    DECLARE_INTERFACE_DESCRIPTOR(u"ohos.appexecfwk.FormUserMgr");
    /**
     * Update form with formId, send formId to form manager service.
     *
     * @param formId, The Id of the form to update.
        * @param bundleName, Provider ability bundleName.
     * @return Returns ERR_OK on success, others on failure.
     */
    virtual int UpdateForm(const int64_t formId, const FmsFormInfo &formInfo) = 0;
};

class FormUserMgrProxy : public IRemoteProxy<IFormUserMgr> {
public:
    explicit FormUserMgrProxy(const sptr<IRemoteObject> &impl) : IRemoteProxy<IFormUserMgr>(impl)
    {}
    virtual ~FormUserMgrProxy()
    {}
    /**
     * Update form with formId, send formId to form manager service.
     *
     * @param formId, The Id of the form to update.
        * @param bundleName, Provider ability bundleName.
     * @return Returns ERR_OK on success, others on failure.
     */
    virtual int UpdateForm(const int64_t formId, const FmsFormInfo &formInfo) override
    {
        return ERR_OK;
    }
};

class FormUserMgrStub : public IRemoteStub<IFormUserMgr> {
public:
    FormUserMgrStub();
    virtual ~FormUserMgrStub();
    virtual int OnRemoteRequest(
        uint32_t code, MessageParcel &data, MessageParcel &reply, MessageOption &option) override;
private:
    int32_t HandleUpdateForm(MessageParcel &data, MessageParcel &reply);
};

class FormUserMgrService : public FormUserMgrStub {
public:
    void OnStart() override;
    void OnStop() override;

    /**
     * Update form with formId, send formId to form manager service.
     *
     * @param formId, The Id of the form to update.
        * @param bundleName, Provider ability bundleName.
     * @return Returns ERR_OK on success, others on failure.
     */
    virtual int UpdateForm(const int64_t formId, const FmsFormInfo &formInfo) override
    {
        return ERR_OK;
    }
};
}  // namespace AppExecFwk
}  // namespace OHOS

#endif  // FOUNDATION_APPEXECFWK_SERVICES_FORMMGR_TEST_MOCK_INCLUDE_MOCK_FORM_USER_MANAGER_H
