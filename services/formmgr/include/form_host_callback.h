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

#ifndef FOUNDATION_APPEXECFWK_SERVICES_FORMMGR_INCLUDE_FORMMGR_FORM_HOST_CALLBACK_H
#define FOUNDATION_APPEXECFWK_SERVICES_FORMMGR_INCLUDE_FORMMGR_FORM_HOST_CALLBACK_H

#include <vector>

#include "form_js_info.h"
#include "form_record.h"
#include "ipc_types.h"
#include "iremote_object.h"

namespace OHOS {
namespace AppExecFwk {
/**
 * @class FormHostCallback
 * FormHost callback is used to call form host service.
 */
class FormHostCallback {
public:
    FormHostCallback() = default;
    virtual ~FormHostCallback() = default;
    /**
     * @brief Request to give back a Form.
     * @param formId The Id of the forms to create.
     * @param record Form info.
     * @param callerToken Caller ability token.
     * @return Returns ERR_OK on success, others on failure.
     */
    void OnAcquired(const int64_t formId, const FormRecord &record, const sptr<IRemoteObject> &callerToken);

     /**
     * @brief Form is updated.
     * @param formId The Id of the form to update.
     * @param record Form info.
     * @param callerToken Caller ability token.
     * @return Returns ERR_OK on success, others on failure.
     */
    void OnUpdate(const int64_t formId, const FormRecord &record, const sptr<IRemoteObject> &callerToken);

    /**
     * @brief Form provider is uninstalled.
     * @param formIds The Id list of the forms.
     * @param callerToken Caller ability token.
     * @return Returns ERR_OK on success, others on failure.
     */
    void OnUninstall(std::vector<int64_t> &formIds, const sptr<IRemoteObject> &callerToken);
};
}  // namespace AppExecFwk
}  // namespace OHOS

#endif // FOUNDATION_APPEXECFWK_SERVICES_FORMMGR_INCLUDE_FORMMGR_FORM_HOST_CALLBACK_H
