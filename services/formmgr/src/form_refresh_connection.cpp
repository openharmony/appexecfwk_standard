
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
#include "form_constants.h"
#include "form_refresh_connection.h"
#include "form_supply_callback.h"
#include "form_task_mgr.h"
#include "ipc_types.h"
#include "message_parcel.h"
#include "want.h"

namespace OHOS {
namespace AppExecFwk {
FormRefreshConnection::FormRefreshConnection(const int64_t formId, const Want& want)
    :formId_(formId),  
    want_(want)
{
}
/**
 * @brief OnAbilityConnectDone, AbilityMs notify caller ability the result of connect.
 * 
 * @param element Service ability's ElementName.
 * @param remoteObject The session proxy of service ability.
 * @param resultCode ERR_OK on success, others on failure.
 * @return none.
 */
void FormRefreshConnection::OnAbilityConnectDone(
    const AppExecFwk::ElementName &element, const sptr<IRemoteObject> &remoteObject, int resultCode)
{
    APP_LOGI("%{public}s called.", __func__);

    if (resultCode != ERR_OK) {
        // APP_LOGE("%{public}s, abilityName:%{public}s, formId:%{public}lld, resultCode:%{public}d", 
        //    __func__, element.GetAbilityName().c_str(), formId_, resultCode);
        return;
    }
    FormSupplyCallback::GetInstance()->AddConnection(this);

    if (want_.HasParameter(Constants::PARAM_MESSAGE_KEY)) {
        std::string message = want_.GetStringParam(Constants::PARAM_MESSAGE_KEY);
        // FormTaskMgr::GetInstance()->FireFormEvent(formId, message, BuildDefaultWant(null, this), remoteObject);
    } else if (want_.HasParameter(Constants::RECREATE_FORM_KEY)) {
        Want cloneWant = Want(want_);
        cloneWant.RemoveParam(Constants::RECREATE_FORM_KEY);
        cloneWant.SetParam(Constants::ACQUIRE_TYPE, Constants::ACQUIRE_TYPE_RECREATE_FORM);
        cloneWant.SetParam(Constants::FORM_CONNECT_ID, this->GetConnectId());
        FormTaskMgr::GetInstance().PostAcquireTask(formId_, cloneWant, remoteObject);
    } else {
        Want want;
        want.SetParam(Constants::FORM_CONNECT_ID, this->GetConnectId());
        FormTaskMgr::GetInstance().PostRefreshTask(formId_, want, remoteObject);
    }
}
}  // namespace AppExecFwk
}  // namespace OHOS