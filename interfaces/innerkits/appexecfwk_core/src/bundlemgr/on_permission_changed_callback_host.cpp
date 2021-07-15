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

#include "on_permission_changed_callback_host.h"

#include "ipc_types.h"
#include "string_ex.h"

#include "app_log_wrapper.h"

namespace OHOS {
namespace AppExecFwk {

OnPermissionChangedCallbackHost::OnPermissionChangedCallbackHost()
{
    APP_LOGI("create on permission changed host instance");
}

OnPermissionChangedCallbackHost::~OnPermissionChangedCallbackHost()
{
    APP_LOGI("destroy on permission changed host instance");
}

int OnPermissionChangedCallbackHost::OnRemoteRequest(
    uint32_t code, MessageParcel &data, MessageParcel &reply, MessageOption &option)
{
    APP_LOGD("permission changed host onReceived message, the message code is %{public}u", code);
    switch (code) {
        case static_cast<uint32_t>(OnPermissionChangedCallback::Message::ON_CHANGED): {
            int32_t uid = data.ReadInt32();
            OnChanged(uid);
            break;
        }
        default:
            return IPCObjectStub::OnRemoteRequest(code, data, reply, option);
    }
    return NO_ERROR;
}

}  // namespace AppExecFwk
}  // namespace OHOS
