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

#ifndef FOUNDATION_APPEXECFWK_OHOS_FORM_HOST_CLIENT_H
#define FOUNDATION_APPEXECFWK_OHOS_FORM_HOST_CLIENT_H

#include <map>
// #include "ability.h"
#include "form_host_stub.h"
#include "iremote_object.h"
#include "iremote_stub.h"
#include "semaphore_ex.h"

namespace OHOS {
namespace AppExecFwk {
/**
 * @class MockFormHostClient
 * MockFormHostClient.
 */
class MockFormHostClient : public FormHostStub {
public:
    MockFormHostClient() = default;
    virtual ~MockFormHostClient() = default;

    void Wait()
    {
        sem_.Wait();
    }

    int Post()
    {
        sem_.Post();
        return 0;
    }

    void PostVoid()
    {
        sem_.Post();
    }

    // void AddForm(const Ability& slice, int64_t formId);

    // void RemoveForm(const Ability& slice, int64_t formId);

    // bool ContainsForm(int64_t formId);
    /**
     * Request to give back a Form.
     *
     * @param formId, The Id of the forms to create.
     * @param formInfo, Form info.
     * @return none.
     */
    virtual void OnAcquired(const FormJsInfo &formInfo) override;

     /**
     * Form is updated.
     *
     * @param formId, The Id of the form to update.
     * @param formInfo, Form info.
     * @return none.
     */
    virtual void OnUpdate(const FormJsInfo &formInfo) override;

    /**
     * Form provider is uninstalled.
     *
     * @param formIds, The Id list of the forms.
     * @return none.
     */
    virtual void OnUninstall(const std::vector<int64_t> &formIds) override;

private:
 
private:
    Semaphore sem_;
    DISALLOW_COPY_AND_MOVE(MockFormHostClient);
};
}  // namespace AppExecFwk
}  // namespace OHOS
#endif  // FOUNDATION_APPEXECFWK_OHOS_FORM_HOST_CLIENT_H
