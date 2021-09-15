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

#ifndef FOUNDATION_APPEXECFWK_SERVICES_FORMMGR_INCLUDE_FORM_AMS_HELPER_H
#define FOUNDATION_APPEXECFWK_SERVICES_FORMMGR_INCLUDE_FORM_AMS_HELPER_H

#include <singleton.h>
#include <string>
#include "ability_connect_callback_interface.h"
#include "ability_manager_interface.h"
#include "iremote_object.h"
#include "uri.h"

namespace OHOS {
namespace AppExecFwk {
using IAbilityConnection = OHOS::AAFwk::IAbilityConnection;
using IAbilityManager = OHOS::AAFwk::IAbilityManager;
using Want = OHOS::AAFwk::Want;
/**
 * @class FormAmsHelper
 * Ams helpler.
 */
class FormAmsHelper final  : public DelayedRefSingleton<FormAmsHelper> {
    DECLARE_DELAYED_REF_SINGLETON(FormAmsHelper)
public:
    DISALLOW_COPY_AND_MOVE(FormAmsHelper);

    /**
     * @brief acquire a form ability manager if it not existed,
     * @return returns the ability manager ipc object or nullptr for failed.
     */
    sptr<IAbilityManager> GetAbilityManager();
    /**
     * @brief Connect session with service ability.
     * @param want Special want for service type's ability.
     * @param connect Callback used to notify caller the result of connecting or disconnecting.
     * @return Returns ERR_OK on success, others on failure.
     */
    ErrCode ConnectServiceAbility(
        const Want &want, const sptr<IAbilityConnection> &connect);
    /**
     * @brief Disconnect session with service ability.
     * @param want Special want for service type's ability.
     * @param connect Callback used to notify caller the result of connecting or disconnecting.
     * @return Returns ERR_OK on success, others on failure.
     */
    ErrCode DisConnectServiceAbility(const sptr<IAbilityConnection> &connect);
    /**
     * @brief Add the ability manager instance for debug.
     * @param abilityManager the ability manager ipc object.
     */
    void SetAbilityManager(const sptr<AAFwk::IAbilityManager> &abilityManager);
private:
    sptr<AAFwk::IAbilityManager> abilityManager_ = nullptr;
};
}  // namespace AppExecFwk
}  // namespace OHOS

#endif // FOUNDATION_APPEXECFWK_SERVICES_FORMMGR_INCLUDE_FORM_AMS_HELPER_H
