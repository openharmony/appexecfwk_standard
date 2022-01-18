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
#include "errors.h"
#include "form_constants.h"
#include "form_supply_interface.h"
#include "mock_form_provider_client.h"
#include "string_ex.h"

namespace OHOS {
namespace AppExecFwk {
/**
 * Acquire to give back an ProviderFormInfo. This is sync API.
 *
 * @param want, The want of the form to create.
 * @param callerToken, Caller ability token.
 * @return none.
 */
int MockFormProviderClient::AcquireProviderFormInfo(const int64_t formId, const Want &want,
    const sptr<IRemoteObject> &callerToken)
{
    // avoid the user modify the number in onCreate

    APP_LOGD("Acquire provider form info");

    sptr<IFormSupply> formSupply = iface_cast<IFormSupply>(callerToken);
    if (formSupply == nullptr) {
        APP_LOGE("failed to get formSupplyProxy");
    }

    FormProviderInfo formProviderInfo;
    Want newWant(want);
    newWant.SetParam(Constants::ACQUIRE_TYPE, want.GetIntParam(Constants::ACQUIRE_TYPE, 0));
    newWant.SetParam(Constants::FORM_CONNECT_ID, want.GetLongParam(Constants::FORM_CONNECT_ID, 0));
    newWant.SetParam(Constants::FORM_SUPPLY_INFO, want.GetStringParam(Constants::FORM_SUPPLY_INFO));
    newWant.SetParam(Constants::PROVIDER_FLAG, true);
    newWant.SetParam(Constants::PARAM_FORM_IDENTITY_KEY, std::to_string(formId));
    formSupply->OnAcquire(formProviderInfo, newWant);
    return ERR_OK;
}

/**
 * Notify provider when the form was deleted.
 *
 * @param formId, The Id of the form.
 * @param callerToken, Caller ability token.
 * @return none.
 */
int MockFormProviderClient::NotifyFormDelete(const int64_t formId, const Want &want,
    const sptr<IRemoteObject> &callerToken)
{
    APP_LOGD("Notify form delete");
    return ERR_OK;
}
/**
 * Notify provider when the forms was deleted.
 *
 * @param formIds, The id list of forms.
 * @param want Indicates the structure containing form info.
 * @param callerToken, Caller ability token.
 * @return none.
 */
int MockFormProviderClient::NotifyFormsDelete(const std::vector<int64_t> &formIds, const Want &want,
    const sptr<IRemoteObject> &callerToken)
{
    APP_LOGD("Notify forms delete");
    return ERR_OK;
}

/**
 * @brief Notify provider when the form need update.
 * @param formId The Id of the form.
 * @param want Indicates the structure containing form info.
 * @param callerToken Caller ability token.
 */
int MockFormProviderClient::NotifyFormUpdate(const int64_t formId, const Want &want,
    const sptr<IRemoteObject> &callerToken)
{
    APP_LOGD("Notify form update");
    return ERR_OK;
}

/**
 * @brief Event notify when change the form visible.
 *
 * @param formEvents The vector of form ids.
 * @param formVisibleType The form visible type, including FORM_VISIBLE and FORM_INVISIBLE.
 * @param want Indicates the structure containing form info.
 * @param callerToken Caller ability token.
 * @return Returns ERR_OK on success, others on failure.
 */
int MockFormProviderClient::EventNotify(const std::vector<int64_t> &formIds, const int32_t formVisibleType,
    const Want &want, const sptr<IRemoteObject> &callerToken)
{
    APP_LOGD("Event notify");
    return ERR_OK;
}

/**
 * Notify provider when the temp form was cast to normal form.
 *
 * @param formId, The Id of the form to update.
 * @param callerToken, Caller ability token.
 * @return none.
 */
int MockFormProviderClient::NotifyFormCastTempForm(const int64_t formId, const Want &want,
    const sptr<IRemoteObject> &callerToken)
{
    APP_LOGD("Notify cast temp form");
    return ERR_OK;
}
/**
 * @brief Fire message event to form provider.
 * @param formId The Id of the from.
 * @param message Event message.
 * @param want The want of the request.
 * @param callerToken Form provider proxy object.
 * @return Returns ERR_OK on success, others on failure.
 */
int MockFormProviderClient::FireFormEvent(const int64_t formId, const std::string &message, const Want &want,
    const sptr<IRemoteObject> &callerToken)
{
    APP_LOGD("Fire form event");
    return ERR_OK;
}
}  // namespace AppExecFwk
}  // namespace OHOS
