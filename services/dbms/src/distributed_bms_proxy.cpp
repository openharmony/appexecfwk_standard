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

#include "distributed_bms_proxy.h"

#include "app_log_wrapper.h"
#include "parcel_macro.h"

namespace OHOS {
namespace AppExecFwk {
DistributedBmsProxy::DistributedBmsProxy(const sptr<IRemoteObject> &object) : IRemoteProxy<IDistributedBms>(object)
{
    APP_LOGI("DistributedBmsProxy instance is created");
}

DistributedBmsProxy::~DistributedBmsProxy()
{
    APP_LOGI("DistributedBmsProxy instance is destroyed");
}

bool DistributedBmsProxy::GetRemoteAbilityInfo(
    const OHOS::AppExecFwk::ElementName &elementName, RemoteAbilityInfo &remoteAbilityInfo)
{
    APP_LOGD("DistributedBmsProxy GetRemoteAbilityInfo");
    MessageParcel data;
    MessageParcel reply;
    if (!data.WriteParcelable(&elementName)) {
        APP_LOGE("DistributedBmsProxy GetRemoteAbilityInfo write elementName error");
        return false;
    }

    if (!GetParcelableInfo<RemoteAbilityInfo>(
            IDistributedBms::Message::GET_REMOTE_ABILITY_INFO, data, remoteAbilityInfo)) {
        APP_LOGE("fail to query ability info mutiparam from server");
        return false;
    }
    return true;
}

bool DistributedBmsProxy::SendRequest(IDistributedBms::Message code, MessageParcel &data, MessageParcel &reply)
{
    APP_LOGD("DistributedBmsProxy SendRequest");
    sptr<IRemoteObject> remote = Remote();
    MessageOption option(MessageOption::TF_SYNC);
    if (remote == nullptr) {
        APP_LOGE("fail to send %{public}d cmd to service due to remote object is null", code);
        return false;
    }
    int32_t result = remote->SendRequest(static_cast<uint32_t>(code), data, reply, option);
    if (result != OHOS::NO_ERROR) {
        APP_LOGE("fail to send %{public}d cmd to service due to transact error", code);
        return false;
    }
    return true;
}

template<typename T>
bool DistributedBmsProxy::GetParcelableInfo(IDistributedBms::Message code, MessageParcel &data, T &parcelableInfo)
{
    MessageParcel reply;
    if (!SendRequest(code, data, reply)) {
        return false;
    }

    if (!reply.ReadBool()) {
        APP_LOGE("reply result false");
        return false;
    }

    std::unique_ptr<T> info(reply.ReadParcelable<T>());
    if (!info) {
        APP_LOGE("readParcelableInfo failed");
        return false;
    }
    parcelableInfo = *info;
    APP_LOGD("get parcelable info success");
    return true;
}
}  // namespace AppExecFwk
}  // namespace OHOS