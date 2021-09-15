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

#include "ability_manager_interface.h"
#include "appexecfwk_errors.h"
#include "app_log_wrapper.h"
#include "form_ams_helper.h"
#include "if_system_ability_manager.h"
#include "ipc_skeleton.h"
#include "iservice_registry.h"
#include "system_ability_definition.h"

namespace OHOS {
namespace AppExecFwk {
FormAmsHelper::FormAmsHelper(){}
FormAmsHelper::~FormAmsHelper(){}

/**
 * @brief acquire a form ability manager, if it not existed,
 * @return returns the ability manager ipc object, or nullptr for failed.
 */
sptr<AAFwk::IAbilityManager> FormAmsHelper::GetAbilityManager()
{
    if (abilityManager_ == nullptr) {
        sptr<ISystemAbilityManager> systemManager = SystemAbilityManagerClient::GetInstance().GetSystemAbilityManager();
        if (systemManager == nullptr) {
            APP_LOGE("%{public}s:fail to get registry", __func__);
            return nullptr;
        }
        sptr<IRemoteObject> remoteObject = systemManager->GetSystemAbility(ABILITY_MGR_SERVICE_ID);
        if (remoteObject == nullptr) {
            APP_LOGE("%{public}s:fail to connect AbilityMgrService", __func__);
            return nullptr;
        }
        APP_LOGD("connect AbilityMgrService success");

        abilityManager_ = iface_cast<AAFwk::IAbilityManager>(remoteObject);
    }
    
    return abilityManager_;
}

/**
 * @brief ConnectAbility, connect session with service ability.
 * @param want Special want for service type's ability.
 * @param connect Callback used to notify caller the result of connecting or disconnecting.
 * @return Returns ERR_OK on success, others on failure.
 */
ErrCode FormAmsHelper::ConnectServiceAbility(
    const Want &want, const sptr<IAbilityConnection> &connect) 
{
    APP_LOGI("%{public}s called.", __func__);
    sptr<IAbilityManager> ams = GetAbilityManager();
    if (ams == nullptr) {
        APP_LOGE("%{public}s:ability service not connect", __func__);
        return ERR_APPEXECFWK_FORM_BIND_PROVIDER_FAILED;
    }
    return ams->ConnectAbility(want, connect, nullptr);
}
/**
 * @brief DisConnectAbility, disconnect session with service ability.
 * @param want Special want for service type's ability.
 * @param connect Callback used to notify caller the result of connecting or disconnecting.
 * @return Returns ERR_OK on success, others on failure.
 */
ErrCode FormAmsHelper::DisConnectServiceAbility(const sptr<IAbilityConnection> &connect)
{    
    sptr<IAbilityManager> ams = GetAbilityManager();
    if (ams == nullptr) {
        APP_LOGE("%{public}s:ability service not connect", __func__);
        return ERR_APPEXECFWK_FORM_BIND_PROVIDER_FAILED;
    }
    return ams->DisconnectAbility(connect);
}
/**
 * @brief Add the ability manager instance for debug.
 * @param abilityManager the ability manager ipc object.
 */
void FormAmsHelper::SetAbilityManager(const sptr<AAFwk::IAbilityManager> &abilityManager)
{
    abilityManager_ = abilityManager;
}
}  // namespace AppExecFwk
}  // namespace OHOS