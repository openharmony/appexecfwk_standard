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

#include "distributed_bms_host.h"

#include "app_log_wrapper.h"
#include "appexecfwk_errors.h"
#include "bundle_constants.h"
#include "remote_ability_info.h"

namespace OHOS {
namespace AppExecFwk {
DistributedBmsHost::DistributedBmsHost()
{
    APP_LOGI("DistributedBmsHost instance is created");
}

DistributedBmsHost::~DistributedBmsHost()
{
    APP_LOGI("DistributedBmsHost instance is destroyed");
}

int DistributedBmsHost::OnRemoteRequest(uint32_t code, MessageParcel &data, MessageParcel &reply, MessageOption &option)
{
    APP_LOGI("DistributedBmsHost receives message from client, code = %{public}d, flags = %{public}d", code,
        option.GetFlags());
    switch (code) {
        case static_cast<uint32_t>(IDistributedBms::Message::GET_REMOTE_ABILITY_INFO): {
            std::unique_ptr<ElementName> elementName(data.ReadParcelable<ElementName>());
            if (!elementName) {
                APP_LOGE("ReadParcelable<elementName> failed");
                return ERR_APPEXECFWK_PARCEL_ERROR;
            }
            RemoteAbilityInfo remoteAbilityInfo;
            bool ret = GetRemoteAbilityInfo(*elementName, remoteAbilityInfo);
            if (!reply.WriteBool(ret)) {
                APP_LOGE("GetRemoteAbilityInfo write failed");
                return ERR_APPEXECFWK_PARCEL_ERROR;
            }
            if (ret) {
                if (!reply.WriteParcelable(&remoteAbilityInfo)) {
                    APP_LOGE("GetRemoteAbilityInfo write failed");
                    return ERR_APPEXECFWK_PARCEL_ERROR;
                }
            }
            break;
        }
        default:
            APP_LOGW("DistributedBmsHost receives unknown code, code = %{public}d", code);
            return IPCObjectStub::OnRemoteRequest(code, data, reply, option);
    }
    APP_LOGI("DistributedBmsHost finish to process message from client");
    return NO_ERROR;
}
}  // namespace AppExecFwk
}  // namespace OHOS