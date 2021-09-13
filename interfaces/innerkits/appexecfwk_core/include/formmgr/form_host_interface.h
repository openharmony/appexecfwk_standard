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

#ifndef FOUNDATION_APPEXECFWK_INTERFACES_INNERKITS_APPEXECFWK_CORE_INCLUDE_FORMMGR_FORM_HOST_INTERFACE_H
#define FOUNDATION_APPEXECFWK_INTERFACES_INNERKITS_APPEXECFWK_CORE_INCLUDE_FORMMGR_FORM_HOST_INTERFACE_H

#include <vector>

#include "form_js_info.h"
#include "ipc_types.h"
#include "iremote_broker.h"

namespace OHOS {
namespace AppExecFwk {
/**
 * @class IFormHost
 * IFormHost interface is used to access form host service.
 */
class IFormHost : public OHOS::IRemoteBroker {
public:
    DECLARE_INTERFACE_DESCRIPTOR(u"ohos.appexecfwk.FormHost");

    /**
     * @brief Request to give back a Form.
     * @param formInfo Form info.
     */
    virtual void OnAcquired(const FormJsInfo &formInfo) = 0;

     /**
     * @brief Form is updated.
     * @param formInfo Form info.
     */
    virtual void OnUpdate(const FormJsInfo &formInfo) = 0;

    /**
     * @brief Form provider is uninstalled.
     * @param formIds The Id list of the forms.
     */
    virtual void OnUninstall(const std::vector<int64_t> &formIds) = 0;

    enum class Message {
        // ipc id 1-1000 for kit
        // ipc id 1001-2000 for DMS
        // ipc id 2001-3000 for tools
        // ipc id for create (3001)
        FORM_HOST_ON_ACQUIRED = 3681,

        // ipc id for update (3682)
        FORM_HOST_ON_UPDATE,

        // ipc id for uninstall (3683)
        FORM_HOST_ON_UNINSTALL,
    };
};
}  // namespace AppExecFwk
}  // namespace OHOS

#endif // FOUNDATION_APPEXECFWK_INTERFACES_INNERKITS_APPEXECFWK_CORE_INCLUDE_FORMMGR_FORM_HOST_INTERFACE_H
