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
#include "form_constants.h"
#include "form_supply_proxy.h"
#include "string_ex.h"

namespace OHOS {
namespace AppExecFwk {
/**
 * @brief Send form binding data from form provider to fms.
 * @param providerFormInfo Form binding data.
 * @param want input data.
 * @return Returns ERR_OK on success, others on failure.
 */
int FormSupplyProxy::OnAcquire(const FormProviderInfo &formInfo, const Want& want)
{
    MessageParcel data;
    if (!WriteInterfaceToken(data)) {
        APP_LOGE("%{public}s, failed to WriteInterfaceToken", __func__);
        return ERR_APPEXECFWK_PARCEL_ERROR;
    }
    if (!data.WriteParcelable(&want)) {
        APP_LOGE("%{public}s, failed to write want", __func__);
        return ERR_APPEXECFWK_PARCEL_ERROR;
    }
    
    if (want.GetIntParam(Constants::PROVIDER_FLAG, ERR_OK) == ERR_OK) {
        if (!data.WriteParcelable(&formInfo)) {
            APP_LOGE("%{public}s, failed to write formInfo", __func__);
            return ERR_APPEXECFWK_PARCEL_ERROR;
        }
    }

    MessageParcel reply;
    MessageOption option;
    int error = Remote()->SendRequest(static_cast<uint32_t>(IFormSupply::Message::TRANSACTION_FORM_ACQUIRED), 
    data, reply, option);
    if (error != ERR_OK) {
        APP_LOGE("%{public}s, failed to SendRequest: %{public}d", __func__, error);
    }
    return error;
}


/**
 * @brief Send other event  to fms.
 * @param want input data.
 * @return Returns ERR_OK on success, others on failure.
 */
int FormSupplyProxy::OnEventHandle(const Want& want)
{
    MessageParcel data;

    if (!WriteInterfaceToken(data)) {
        APP_LOGE("%{public}s, failed to write interface token", __func__);
        return ERR_APPEXECFWK_PARCEL_ERROR;
    }

    if (!data.WriteParcelable(&want)) {
        APP_LOGE("%{public}s, failed to write want", __func__);
        return ERR_APPEXECFWK_PARCEL_ERROR;
    }

    MessageParcel reply;
    MessageOption option;
    int error = Remote()->SendRequest(static_cast<uint32_t>(IFormSupply::Message::TRANSACTION_EVENT_HANDLE), 
    data, reply, option);
    if (error != ERR_OK) {
        APP_LOGE("%{public}s, failed to SendRequest: %{public}d", __func__, error);
    }
    return error;
}

template<typename T>
int  FormSupplyProxy::GetParcelableInfos(MessageParcel &reply, std::vector<T> &parcelableInfos)
{
    int32_t infoSize = reply.ReadInt32();
    for (int32_t i = 0; i < infoSize; i++) {
        std::unique_ptr<T> info(reply.ReadParcelable<T>());
        if (!info) {
            APP_LOGE("%{public}s, failed to ReadParcelable<T>", __func__);
            return ERR_INVALID_VALUE;
        }
        parcelableInfos.emplace_back(*info);
    }
    APP_LOGI("get parcelable infos success");
    return ERR_OK;
}

bool  FormSupplyProxy::WriteInterfaceToken(MessageParcel &data)
{
    if (!data.WriteInterfaceToken(FormSupplyProxy::GetDescriptor())) {
        APP_LOGE("%{public}s, failed to write interface token failed", __func__);
        return false;
    }
    return true;
}
}  // namespace AppExecFwk
}  // namespace OHOS
