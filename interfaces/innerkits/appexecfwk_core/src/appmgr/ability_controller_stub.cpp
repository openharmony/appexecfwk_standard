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

#include "ability_controller_stub.h"
#include "appexecfwk_errors.h"
#include "app_log_wrapper.h"
#include "ipc_types.h"
#include "iremote_object.h"

namespace OHOS {
namespace AppExecFwk {
AbilityControllerStub::AbilityControllerStub()
{
    memberFuncMap_[static_cast<uint32_t>(
        IAbilityController::Message::TRANSACT_ON_ABILITY_STARTING)] =
        &AbilityControllerStub::HandleAbilityStarting;
    memberFuncMap_[static_cast<uint32_t>(
        IAbilityController::Message::TRANSACT_ON_ABILITY_RESUMING)] =
        &AbilityControllerStub::HandleAbilityResuming;
}

AbilityControllerStub::~AbilityControllerStub()
{
    memberFuncMap_.clear();
}

int AbilityControllerStub::OnRemoteRequest(
    uint32_t code, MessageParcel &data, MessageParcel &reply, MessageOption &option)
{
    APP_LOGI("AbilityControllerStub::OnReceived, code = %{public}d, flags= %{public}d.", code, option.GetFlags());
    std::u16string descriptor = AbilityControllerStub::GetDescriptor();
    std::u16string remoteDescriptor = data.ReadInterfaceToken();
    if (descriptor != remoteDescriptor) {
        APP_LOGE("local descriptor is not equal to remote");
        return ERR_INVALID_STATE;
    }

    auto itFunc = memberFuncMap_.find(code);
    if (itFunc != memberFuncMap_.end()) {
        auto memberFunc = itFunc->second;
        if (memberFunc != nullptr) {
            return (this->*memberFunc)(data, reply);
        }
    }
    return IPCObjectStub::OnRemoteRequest(code, data, reply, option);
}

bool AbilityControllerStub::AbilityStarting(const Want &want, const std::string &bundleName)
{
    return true;
}

bool AbilityControllerStub::AbilityResuming(const std::string &bundleName)
{
    return true;
}

int32_t AbilityControllerStub::HandleAbilityStarting(MessageParcel &data, MessageParcel &reply)
{
    APP_LOGI("HandleAbilityStarting");
    std::unique_ptr<Want> want(data.ReadParcelable<Want>());
    if (!want) {
        APP_LOGE("ReadParcelable<Want> failed");
        return ERR_APPEXECFWK_PARCEL_ERROR;
    }
    std::string pkg = data.ReadString();
    bool ret = AbilityStarting(*want, pkg);
    reply.WriteBool(ret);
    return NO_ERROR;
}

int32_t AbilityControllerStub::HandleAbilityResuming(MessageParcel &data, MessageParcel &reply)
{
    APP_LOGI("HandleAbilityResuming");
    std::string pkg = data.ReadString();
    bool ret = AbilityResuming(pkg);
    reply.WriteBool(ret);
    return NO_ERROR;
}
}  // namespace AppExecFwk
}  // namespace OHOS
