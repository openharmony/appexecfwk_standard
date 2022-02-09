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

#ifndef FOUNDATION_APPEXECFWK_SERVICES_DBMS_INCLUDE_DISTRIBUTED_BMS_INTERFACE_H
#define FOUNDATION_APPEXECFWK_SERVICES_DBMS_INCLUDE_DISTRIBUTED_BMS_INTERFACE_H

#include <string>
#include <vector>

#include "element_name.h"
#include "iremote_broker.h"
#include "remote_ability_info.h"

namespace OHOS {
namespace AppExecFwk {
class IDistributedBms : public IRemoteBroker {
public:
    DECLARE_INTERFACE_DESCRIPTOR(u"ohos.appexecfwk.IDistributedbms");

    /**
     * @brief get remote ability info
     * @param elementName Indicates the elementName.
     * @param remoteAbilityInfo Indicates the remote ability info.
     * @return Returns true when get remote ability info success; returns false otherwise.
     */
    virtual bool GetRemoteAbilityInfo(
        const OHOS::AppExecFwk::ElementName &elementName, RemoteAbilityInfo &remoteAbilityInfo) = 0;

    /**
     * @brief get ability info
     * @param elementName Indicates the elementName.
     * @param remoteAbilityInfo Indicates the remote ability info.
     * @return Returns true when get remote ability info success; returns false otherwise.
     */
    virtual bool GetAbilityInfo(
        const OHOS::AppExecFwk::ElementName &elementName, RemoteAbilityInfo &remoteAbilityInfo) = 0;

    enum class Message {
        GET_REMOTE_ABILITY_INFO = 1,
        GET_ABILITY_INFO = 2,
    };
};
}  // namespace AppExecFwk
}  // namespace OHOS
#endif  // FOUNDATION_APPEXECFWK_SERVICES_DBMS_INCLUDE_DISTRIBUTED_BMS_INTERFACE_H