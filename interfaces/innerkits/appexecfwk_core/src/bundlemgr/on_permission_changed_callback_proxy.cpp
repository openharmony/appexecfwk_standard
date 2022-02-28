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

#include "on_permission_changed_callback_proxy.h"

#include "ipc_types.h"
#include "parcel.h"
#include "string_ex.h"

#include "app_log_wrapper.h"
#include "appexecfwk_errors.h"

namespace OHOS {
namespace AppExecFwk {
OnPermissionChangedCallbackProxy::OnPermissionChangedCallbackProxy(const sptr<IRemoteObject> &object)
    : IRemoteProxy<OnPermissionChangedCallback>(object)
{
    APP_LOGI("create on permission changed callback proxy instance");
}

OnPermissionChangedCallbackProxy::~OnPermissionChangedCallbackProxy()
{
    APP_LOGI("destroy on permission changed callback proxy instance");
}

void OnPermissionChangedCallbackProxy::OnChanged(const int32_t uid)
{
    APP_LOGI("on permission changed %{public}d", uid);
    MessageParcel data;
    MessageParcel reply;
    MessageOption option(MessageOption::TF_SYNC);
    if (!data.WriteInterfaceToken(OnPermissionChangedCallbackProxy::GetDescriptor())) {
        APP_LOGE("fail to OnChanged due to write MessageParcel fail");
        return;
    }

    if (!data.WriteInt32(uid)) {
        APP_LOGE("fail to call OnChanged, for write resultCode failed");
        return;
    }

    sptr<IRemoteObject> remote = Remote();
    if (!remote) {
        APP_LOGE("fail to call OnChanged, for Remote() is nullptr");
        return;
    }

    int32_t ret = remote->SendRequest(
        static_cast<int32_t>(OnPermissionChangedCallback::Message::ON_CHANGED), data, reply, option);
    if (ret != NO_ERROR) {
        APP_LOGW("fail to call OnChanged, for transact is failed, error code is: %{public}d", ret);
    }
}
}  // namespace AppExecFwk
}  // namespace OHOS
