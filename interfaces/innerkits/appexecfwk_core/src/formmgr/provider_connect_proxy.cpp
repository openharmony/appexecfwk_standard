
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

#include "app_log_wrapper.h"
#include "ipc_types.h"
#include "message_parcel.h"
#include "provider_connect_proxy.h"

namespace OHOS {
namespace AppExecFwk {
/**
 * @brief OnAbilityConnectDone, AbilityMs notify caller ability the result of connect.
 * @param element service ability's ElementName.
 * @param remoteObject the session proxy of service ability.
 * @param resultCode ERR_OK on success, others on failure.
 */
void ProviderConnectProxy::OnAbilityConnectDone(
    const AppExecFwk::ElementName &element, const sptr<IRemoteObject> &remoteObject, int resultCode)
{
    APP_LOGD("%{public}s, abilityName:%{public}s,resultCode:%{public}d", __func__, element.GetAbilityName().c_str(), resultCode);    
    int error;
    MessageParcel data;
    MessageParcel reply;
    MessageOption option;

    if (!WriteInterfaceToken(data)) {
        APP_LOGE("%{public}s, failed to write interface token", __func__);
        return;
    }

    if (!data.WriteParcelable(&element)) {
        APP_LOGE("%{public}s, failed to write element", __func__);
        return;
    }

    if (!data.WriteParcelable(remoteObject)) {
        APP_LOGE("%{public}s, failed to write remote object ", __func__);
        return;
    }

    if (!data.WriteInt32(resultCode)) {
        APP_LOGE("%{public}s, failed to write resultCode", __func__);
        return;
    }

    error = Remote()->SendRequest(IAbilityConnection::ON_ABILITY_CONNECT_DONE, data, reply, option);
    if (error != ERR_OK) {
        APP_LOGE("%{public}s, failed to SendRequest: %{public}d", __func__, error);
        return;
    }
}
/**
 * @brief OnAbilityDisconnectDone, AbilityMs notify caller ability the result of disconnect.
 * @param element service ability's ElementName.
 * @param resultCode ERR_OK on success, others on failure.
 */
void ProviderConnectProxy::OnAbilityDisconnectDone(const AppExecFwk::ElementName &element, int resultCode)
{
    APP_LOGD("%{public}s, element:%{public}s, resultCode:%{public}d", __func__, element.GetURI().c_str(), resultCode);
    int error;
    MessageParcel data;
    MessageParcel reply;
    MessageOption option;

    if (!WriteInterfaceToken(data)) {
        APP_LOGE("%{public}s, failed to write interface token", __func__);
        return;
    }
    if (!data.WriteParcelable(&element)) {
        APP_LOGE("%{public}s, failed to write element", __func__);
        return;
    }
    if (!data.WriteInt32(resultCode)) {
        APP_LOGE("%{public}s, failed to write resultCode", __func__);
        return;
    }

    error = Remote()->SendRequest(IAbilityConnection::ON_ABILITY_DISCONNECT_DONE, data, reply, option);
    if (error != ERR_OK) {
        APP_LOGE("%{public}s, failed to SendRequest: %{public}d", __func__, error);
        return;
    }
}

bool ProviderConnectProxy::WriteInterfaceToken(MessageParcel &data)
{
    if (!data.WriteInterfaceToken(ProviderConnectProxy::GetDescriptor())) {
        APP_LOGE("%{public}s, failed to write interface token", __func__);
        return false;
    }
    return true;
}
}  // namespace AppExecFwk
}  // namespace OHOS
