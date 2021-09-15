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
#include "form_constants.h"
#include "form_supply_stub.h"
#include "ipc_skeleton.h"
#include "ipc_types.h"
#include "iremote_object.h"

namespace OHOS {
namespace AppExecFwk {
FormSupplyStub::FormSupplyStub()
{
    memberFuncMap_[static_cast<uint32_t>(IFormSupply::Message::TRANSACTION_FORM_ACQUIRED)] =
        &FormSupplyStub::HandleOnAcquire;
    memberFuncMap_[static_cast<uint32_t>(IFormSupply::Message::TRANSACTION_EVENT_HANDLE)] =
        &FormSupplyStub::HandleOnEventHandle;
}

FormSupplyStub::~FormSupplyStub()
{
    memberFuncMap_.clear();
}
/**
 * @brief handle remote request.
 * @param data input param.
 * @param reply output param.
 * @param option message option.
 * @return Returns ERR_OK on success, others on failure.
 */
int FormSupplyStub::OnRemoteRequest(uint32_t code, MessageParcel &data, MessageParcel &reply, MessageOption &option)
{
    APP_LOGI("FormSupplyStub::OnReceived, code = %{public}d, flags= %{public}d.", code, option.GetFlags());
    std::u16string descriptor = FormSupplyStub::GetDescriptor();
    std::u16string remoteDescriptor = data.ReadInterfaceToken();
    if (descriptor != remoteDescriptor) {
        APP_LOGE("%{public}s failed, local descriptor is not equal to remote", __func__);
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
/**
 * @brief handle OnAcquire message.
 * @param data input param.
 * @param reply output param.
 * @return Returns ERR_OK on success, others on failure.
 */
int FormSupplyStub::HandleOnAcquire(MessageParcel &data, MessageParcel &reply)
{
    std::unique_ptr<Want> want(data.ReadParcelable<Want>());
    if (!want) {
        APP_LOGE("%{public}s, failed to ReadParcelable<Want>", __func__);
        reply.WriteInt32(ERR_INVALID_VALUE);
        return ERR_INVALID_VALUE;
    }

    int errCode = ERR_OK;
    do {
        errCode = want->GetIntParam(Constants::PROVIDER_FLAG, ERR_OK);
        if (errCode != ERR_OK) {
            APP_LOGE("%{public}s, provider error", __func__); 
            break;
        }
        std::unique_ptr<FormProviderInfo> formInfo(data.ReadParcelable<FormProviderInfo>());
        if (formInfo == nullptr) {
            APP_LOGE("%{public}s, failed to ReadParcelable<FormProviderInfo>", __func__);      
            errCode = ERR_INVALID_VALUE;
            break;
        }
        int32_t result = OnAcquire(*formInfo, *want);
        reply.WriteInt32(result);
        return result;
    } while (true);       
    
    FormProviderInfo formProviderInfo;
    want->SetParam(Constants::PROVIDER_FLAG, errCode);
    OnAcquire(formProviderInfo, *want);
    reply.WriteInt32(errCode);
    return errCode;
}
/**
 * @brief handle OnEventHandle message.
 * @param data input param.
 * @param reply output param.
 * @return Returns ERR_OK on success, others on failure.
 */
int FormSupplyStub::HandleOnEventHandle(MessageParcel &data, MessageParcel &reply)
{
    std::unique_ptr<Want> want(data.ReadParcelable<Want>());
    if (!want) {
        APP_LOGE("%{public}s, failed to ReadParcelable<Want>", __func__);
        reply.WriteInt32(ERR_INVALID_VALUE);
        return ERR_INVALID_VALUE;
    }

    int32_t result = OnEventHandle(*want);
    reply.WriteInt32(result);
    return result;
}
}  // namespace AppExecFwk
}  // namespace OHOS