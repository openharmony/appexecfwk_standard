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

#include "appexecfwk_errors.h"
#include "app_log_wrapper.h"
#include "form_host_callback.h"
#include "form_host_interface.h"
#include "form_task_mgr.h"
#include "string_ex.h"

namespace OHOS {
namespace AppExecFwk {
/**
 * @brief Request to give back a Form.
 * @param formId The Id of the forms to create.
 * @param record Form record.
 * @param callerToken Caller ability token.
 * @return Returns ERR_OK on success, others on failure.
 */
void FormHostCallback::OnAcquired(const int64_t formId, const FormRecord& record, 
const sptr<IRemoteObject> &callerToken)
{
    //APP_LOGD("FormHostCallback OnAcquired, formId:%{public}lld", formId);
    FormTaskMgr::GetInstance().PostAcquireTaskToHost(formId, record, callerToken);
}


/**
* @brief Form is updated.
* @param formId The Id of the form to update.
* @param record Form record.
* @param callerToken Caller ability token.
* @return Returns ERR_OK on success, others on failure.
*/
void FormHostCallback::OnUpdate(const int64_t formId, const FormRecord &record, const sptr<IRemoteObject> &callerToken)
{
    APP_LOGI("%{public}s start.", __func__);

    // check formId
    if (formId < 0) {
        //APP_LOGE("%{public}s: OnUpdate invalid param, formId:%{public}lld.", __func__, formId);
        return;
    }

    if (callerToken == nullptr) {
        APP_LOGE("%{public}s: callerToken can not be NULL", __func__);
        return;
    }

    // post updateTask to host
    FormTaskMgr::GetInstance().PostUpdateTaskToHost(formId, record, callerToken);
}

/**
 * @brief Form provider is uninstalled
 * @param formIds The Id list of the forms.
 * @param callerToken Caller ability token.
 * @return Returns ERR_OK on success, others on failure.
 */
void  FormHostCallback::OnUninstall(std::vector<int64_t> &formIds, const sptr<IRemoteObject> &callerToken)
{
    // check formId
    if (formIds.size() <= 0) {
        APP_LOGE("%{public}s: OnUninstall invalid param, formIds is empty.", __func__);
        return;
    }

    if (callerToken == nullptr) {
        APP_LOGE("%{public}s: callerToken can not be NULL", __func__);
        return;
    }
    // post updateTask to host
    FormTaskMgr::GetInstance().PostUninstallTaskToHost(formIds, callerToken);
}
}  // namespace AppExecFwk
}  // namespace OHOS
