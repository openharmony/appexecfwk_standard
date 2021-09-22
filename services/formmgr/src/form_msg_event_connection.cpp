
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

#include <cinttypes>

#include "appexecfwk_errors.h"
#include "app_log_wrapper.h"
#include "form_constants.h"
#include "form_msg_event_connection.h"
#include "form_supply_callback.h"
#include "form_task_mgr.h"
#include "ipc_types.h"
#include "message_parcel.h"
#include "want.h"

namespace OHOS {
namespace AppExecFwk {
FormMsgEventConnection::FormMsgEventConnection(const int64_t formId, const Want& want,
    const std::string &bundleName, const std::string &abilityName)
    :formId_(formId),  
    want_(want)
{
    SetProviderKey(bundleName, abilityName);
}
/**
 * @brief OnAbilityConnectDone, AbilityMs notify caller ability the result of connect.
 * 
 * @param element Service ability's ElementName.
 * @param remoteObject The session proxy of service ability.
 * @param resultCode ERR_OK on success, others on failure.
 * @return none.
 */
void FormMsgEventConnection::OnAbilityConnectDone(
    const AppExecFwk::ElementName &element, const sptr<IRemoteObject> &remoteObject, int resultCode)
{
    APP_LOGI("%{public}s called.", __func__);
    if (resultCode != ERR_OK) {
        APP_LOGE("%{public}s, abilityName:%{public}s, formId:%{public}" PRId64 ", resultCode:%{public}d", 
            __func__, element.GetAbilityName().c_str(), formId_, resultCode);
        return;
    }
    FormSupplyCallback::GetInstance()->AddConnection(this);

    if (want_.HasParameter(Constants::PARAM_MESSAGE_KEY)) {
        std::string message = want_.GetStringParam(Constants::PARAM_MESSAGE_KEY);
        Want eventWant = Want(want_);
        eventWant.SetParam(Constants::FORM_CONNECT_ID, this->GetConnectId());
        FormTaskMgr::GetInstance().PostFormEventTask(formId_, message, eventWant, remoteObject);
    } else {
        APP_LOGE("%{public}s error, message info is not exist", __func__);
    }
}
}  // namespace AppExecFwk
}  // namespace OHOS