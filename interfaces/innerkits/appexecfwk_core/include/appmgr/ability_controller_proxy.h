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

#ifndef FOUNDATION_APPEXECFWK_INTERFACES_INNERKITS_APPEXECFWK_CORE_INCLUDE_APPMGR_ABILITY_CONTROLLER_PROXY_H
#define FOUNDATION_APPEXECFWK_INTERFACES_INNERKITS_APPEXECFWK_CORE_INCLUDE_APPMGR_ABILITY_CONTROLLER_PROXY_H

#include "iremote_proxy.h"
#include "iability_controller.h"

namespace OHOS {
namespace AppExecFwk {
/**
 * @brief Testing interface to monitor what is happening in ability manager while tests are running.
 */
class AbilityControllerProxy : public IRemoteProxy<IAbilityController> {
public:
    explicit AbilityControllerProxy(const sptr<IRemoteObject> &impl);
    virtual ~AbilityControllerProxy() = default;

    /**
     * The system is trying to start an ability. Return true to allow, or false to reject.
     *
     * @param want The want of ability to start.
     * @param bundleName The bundleName of ability to start.
     */
    virtual bool AbilityStarting(const Want &want, const std::string &bundleName) override;

    /**
     * The system is trying to return to an ability. Return true to allow, or false to reject.
     *
     * @param bundleName The bundleName of ability to return.
     */
    virtual bool AbilityResuming(const std::string &bundleName) override;

private:
    bool WriteInterfaceToken(MessageParcel &data);
    static inline BrokerDelegator<AbilityControllerProxy> delegator_;
};
}  // namespace AppExecFwk
}  // namespace OHOS
#endif
