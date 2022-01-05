
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
#include "ipc_types.h"
#include "message_parcel.h"
#include "provider_connect_stub.h"

namespace OHOS {
namespace AppExecFwk {
/**
 * @brief handle remote request.
 * @param data input param.
 * @param reply output param.
 * @param option message option.
 * @return Returns ERR_OK on success, others on failure.
 */
int ProviderConnectStub::OnRemoteRequest(
    uint32_t code, MessageParcel &data, MessageParcel &reply, MessageOption &option)
{
    std::u16string descriptor = ProviderConnectStub::GetDescriptor();
    std::u16string remoteDescriptor = data.ReadInterfaceToken();
    if (descriptor != remoteDescriptor) {
        APP_LOGI("%{public}s failed, local descriptor is not equal to remote", __func__);
        return ERR_INVALID_STATE;
    }

    auto element = data.ReadParcelable<AppExecFwk::ElementName>();
    switch (code) {
        case IAbilityConnection::ON_ABILITY_CONNECT_DONE: {
            if (element == nullptr) {
                APP_LOGE("%{public}s failed, callback stub receive element is nullptr", __func__);
                return ERR_APPEXECFWK_PARCEL_ERROR;
            }
            auto remoteObject = data.ReadRemoteObject();
            auto resultCode = data.ReadInt32();
            OnAbilityConnectDone(*element, remoteObject, resultCode);
            delete element;
            return ERR_OK;
        }
        case IAbilityConnection::ON_ABILITY_DISCONNECT_DONE: {
            if (element == nullptr) {
                APP_LOGE("%{public}s failed, callback stub receive element is nullptr", __func__);
                return ERR_APPEXECFWK_PARCEL_ERROR;
            }
            auto resultCode = data.ReadInt32();
            OnAbilityDisconnectDone(*element, resultCode);
            delete element;
            return ERR_OK;
        }
        default: {
            if (element != nullptr) {
                delete element;
            }
            return IPCObjectStub::OnRemoteRequest(code, data, reply, option);
        }
    }
}
}  // namespace AppExecFwk
}  // namespace OHOS
