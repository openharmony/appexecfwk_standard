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
#include "form_ability_deleteform.h"
#include "app_log_wrapper.h"
#include "form_st_common_info.h"
#include "form_test_utils.h"
#include "system_test_form_util.h"

namespace {
using namespace OHOS::AAFwk;
using namespace OHOS::EventFwk;
}

namespace OHOS {
namespace AppExecFwk {
void FormAbilityDeleteForm::AcquireFormCallback::OnAcquired(const int32_t result, const FormJsInfo &formJsInfo) const
{
    APP_LOGI("%{public}s called", __func__);
    APP_LOGI("%{public}s receive formId:%{public}s", __func__, std::to_string(formJsInfo.formId).c_str());
    if (this->caseName_ == FORM_EVENT_RECV_DELETE_FORM_0600) {
        ability_->FMS_deleteForm_common(formJsInfo.formId, this->caseName_);
    }
}
void FormAbilityDeleteForm::AcquireFormCallback::OnUpdate(const int32_t result, const FormJsInfo &formJsInfo) const
{
    APP_LOGI("%{public}s called", __func__);
    APP_LOGI("%{public}s receive formData:%{public}s", __func__, formJsInfo.formData.c_str());
    if (this->caseName_ == FORM_EVENT_RECV_DELETE_FORM_0300 || this->caseName_ == FORM_EVENT_RECV_DELETE_FORM_0400
        || this->caseName_ == FORM_EVENT_RECV_DELETE_FORM_0700 || this->caseName_ == FORM_EVENT_RECV_DELETE_FORM_0800
        || this->caseName_ == FORM_EVENT_RECV_DELETE_FORM_0900 || this->caseName_ == FORM_EVENT_RECV_DELETE_FORM_1000
        || this->caseName_ == FORM_EVENT_RECV_DELETE_FORM_1100 || this->caseName_ == FORM_EVENT_RECV_DELETE_FORM_1200
        || this->caseName_ == FORM_EVENT_RECV_DELETE_FORM_1400 || this->caseName_ == FORM_EVENT_RECV_DELETE_FORM_1500
        || this->caseName_ == FORM_EVENT_RECV_DELETE_FORM_1501 || this->caseName_ == FORM_EVENT_RECV_DELETE_FORM_1600
        || this->caseName_ == FORM_EVENT_RECV_DELETE_FORM_1601 || this->caseName_ == FORM_EVENT_RECV_DELETE_FORM_1700
        || this->caseName_ == FORM_EVENT_RECV_DELETE_FORM_1701) {
        ability_->FMS_deleteForm_common(formJsInfo.formId, this->caseName_);
    }
}

void FormAbilityDeleteForm::AcquireFormCallback::OnFormUninstall(const int64_t formId) const
{
    APP_LOGI("%{public}s called", __func__);
}

void FormAbilityDeleteForm::FMS_deleteForm_common(int64_t formId, std::string caseName)
{
    APP_LOGI("%{public}s called, formId: %{public}s, caseName:%{public}s",
        __func__, std::to_string(formId).c_str(), caseName.c_str());
    if (caseName == FORM_EVENT_RECV_DELETE_FORM_0300) {
        DeleteForm_0300(formId);
    } else if (caseName == FORM_EVENT_RECV_DELETE_FORM_0400) {
        DeleteForm_0400(formId);
    } else if (caseName == FORM_EVENT_RECV_DELETE_FORM_0600) {
        DeleteForm_0600(formId);
    } else if (caseName == FORM_EVENT_RECV_DELETE_FORM_0700) {
        DeleteForm_0700(formId);
    } else if (caseName == FORM_EVENT_RECV_DELETE_FORM_0800) {
        DeleteForm_0800(formId);
    } else if (caseName == FORM_EVENT_RECV_DELETE_FORM_0900) {
        DeleteForm_0900(formId);
    } else if (caseName == FORM_EVENT_RECV_DELETE_FORM_1000) {
        DeleteForm_1000(formId);
    } else if (caseName == FORM_EVENT_RECV_DELETE_FORM_1100) {
        DeleteForm_1100(formId);
    } else if (caseName == FORM_EVENT_RECV_DELETE_FORM_1200) {
        DeleteForm_1200(formId);
    } else if (caseName == FORM_EVENT_RECV_DELETE_FORM_1400) {
        DeleteForm_1400(formId);
    } else if (caseName == FORM_EVENT_RECV_DELETE_FORM_1500) {
        DeleteForm_1500(formId);
    } else if (caseName == FORM_EVENT_RECV_DELETE_FORM_1501) {
        DeleteForm_1501(formId);
    } else if (caseName == FORM_EVENT_RECV_DELETE_FORM_1600) {
        DeleteForm_1600(formId);
    } else if (caseName == FORM_EVENT_RECV_DELETE_FORM_1601) {
        DeleteForm_1601(formId);
    } else if (caseName == FORM_EVENT_RECV_DELETE_FORM_1700) {
        DeleteForm_1700(formId);
    } else if (caseName == FORM_EVENT_RECV_DELETE_FORM_1701) {
        DeleteForm_1701(formId);
    } else {
        return;
    }
}

void FormAbilityDeleteForm::FMS_deleteForm_0300(std::string data)
{
    std::shared_ptr<AcquireFormCallback> callback =
        std::make_shared<AcquireFormCallback>(FORM_EVENT_RECV_DELETE_FORM_0300);
    callback->ability_ = this;
    // Set Want info begin
    Want want;
    want.SetParam(Constants::PARAM_FORM_DIMENSION_KEY, FORM_DIMENSION_1);
    want.SetParam(Constants::PARAM_FORM_NAME_KEY, PARAM_FORM_NAME1);
    want.SetParam(Constants::PARAM_MODULE_NAME_KEY, PARAM_PROVIDER_MODULE_NAME1);
    want.SetParam(Constants::PARAM_FORM_TEMPORARY_KEY, FORM_TEMP_FORM_FLAG_FALSE);
    want.SetElementName(FORM_TEST_DEVICEID, FORM_PROVIDER_BUNDLE_NAME1, FORM_PROVIDER_ABILITY_NAME1);
    // Set Want info end
    bool bResult = AcquireForm(0, want, callback);
    if (bResult) {
        APP_LOGI("%{public}s AcquireForm ok.", __func__);
    } else {
        APP_LOGE("%{public}s AcquireForm error.", __func__);
        FormTestUtils::PublishEvent(FORM_EVENT_RECV_DELETE_FORM_0300, EVENT_CODE_300, "");
    }
}
void FormAbilityDeleteForm::FMS_deleteForm_0400(std::string data)
{
    std::shared_ptr<AcquireFormCallback> callback =
        std::make_shared<AcquireFormCallback>(FORM_EVENT_RECV_DELETE_FORM_0400);
    callback->ability_ = this;
    // Set Want info begin
    Want want;
    want.SetParam(Constants::PARAM_FORM_DIMENSION_KEY, FORM_DIMENSION_1);
    want.SetParam(Constants::PARAM_FORM_NAME_KEY, PARAM_FORM_NAME1);
    want.SetParam(Constants::PARAM_MODULE_NAME_KEY, PARAM_PROVIDER_MODULE_NAME1);
    want.SetParam(Constants::PARAM_FORM_TEMPORARY_KEY, FORM_TEMP_FORM_FLAG_FALSE);
    want.SetElementName(FORM_TEST_DEVICEID, FORM_PROVIDER_BUNDLE_NAME1, FORM_PROVIDER_ABILITY_NAME1);
    // Set Want info end
    bool bResult = AcquireForm(0, want, callback);
    if (bResult) {
        APP_LOGI("%{public}s AcquireForm ok.", __func__);
    } else {
        APP_LOGE("%{public}s AcquireForm error.", __func__);
        FormTestUtils::PublishEvent(FORM_EVENT_RECV_DELETE_FORM_0400, EVENT_CODE_400, "");
    }
}

void FormAbilityDeleteForm::FMS_deleteForm_0500(std::string data)
{
    APP_LOGI("%{public}s start[%{public}s]", __func__, data.c_str());
    bool bResult = DeleteForm(atoll(data.c_str()));
    if (bResult) {
        APP_LOGI("%{public}s end", __func__);
        FormTestUtils::PublishEvent(FORM_EVENT_RECV_DELETE_FORM_0500, EVENT_CODE_500, "true");
    } else {
        APP_LOGE("%{public}s error", __func__);
        FormTestUtils::PublishEvent(FORM_EVENT_RECV_DELETE_FORM_0500, EVENT_CODE_500, "false");
    }
}

void FormAbilityDeleteForm::FMS_deleteForm_0600(std::string data)
{
    APP_LOGI("%{public}s start[%{public}s]", __func__, data.c_str());
    std::shared_ptr<AcquireFormCallback> callback =
        std::make_shared<AcquireFormCallback>(FORM_EVENT_RECV_DELETE_FORM_0600);
    callback->ability_ = this;
    // Set Want info begin
    Want want;
    want.SetParam(Constants::PARAM_FORM_DIMENSION_KEY, FORM_DIMENSION_1);
    want.SetParam(Constants::PARAM_FORM_NAME_KEY, PARAM_FORM_NAME1);
    want.SetParam(Constants::PARAM_MODULE_NAME_KEY, PARAM_PROVIDER_MODULE_NAME1);
    want.SetParam(Constants::PARAM_FORM_TEMPORARY_KEY, FORM_TEMP_FORM_FLAG_FALSE);
    want.SetElementName(FORM_TEST_DEVICEID, FORM_PROVIDER_BUNDLE_NAME1, FORM_PROVIDER_ABILITY_NAME1);
    // Set Want info end
    bool bResult = AcquireForm(atoll(data.c_str()), want, callback);
    if (bResult) {
        APP_LOGI("%{public}s AcquireForm ok.", __func__);
    } else {
        APP_LOGE("%{public}s AcquireForm error.", __func__);
        FormTestUtils::PublishEvent(FORM_EVENT_RECV_DELETE_FORM_0600, EVENT_CODE_600, "");
    }
}

void FormAbilityDeleteForm::FMS_deleteForm_0700(std::string data)
{
    std::shared_ptr<AcquireFormCallback> callback =
        std::make_shared<AcquireFormCallback>(FORM_EVENT_RECV_DELETE_FORM_0700);
    callback->ability_ = this;
    // Set Want info begin
    Want want;
    want.SetParam(Constants::PARAM_FORM_DIMENSION_KEY, FORM_DIMENSION_1);
    want.SetParam(Constants::PARAM_FORM_NAME_KEY, PARAM_FORM_NAME1);
    want.SetParam(Constants::PARAM_MODULE_NAME_KEY, PARAM_PROVIDER_MODULE_NAME1);
    want.SetParam(Constants::PARAM_FORM_TEMPORARY_KEY, FORM_TEMP_FORM_FLAG_FALSE);
    want.SetElementName(FORM_TEST_DEVICEID, FORM_PROVIDER_BUNDLE_NAME1, FORM_PROVIDER_ABILITY_NAME1);
    // Set Want info end
    bool bResult = AcquireForm(0, want, callback);
    if (bResult) {
        APP_LOGI("%{public}s AcquireForm ok.", __func__);
    } else {
        APP_LOGE("%{public}s AcquireForm error.", __func__);
        FormTestUtils::PublishEvent(FORM_EVENT_RECV_DELETE_FORM_0700, EVENT_CODE_700, "");
    }
}

void FormAbilityDeleteForm::FMS_deleteForm_0800(std::string data)
{
    std::shared_ptr<AcquireFormCallback> callback =
        std::make_shared<AcquireFormCallback>(FORM_EVENT_RECV_DELETE_FORM_0800);
    callback->ability_ = this;
    // Set Want info begin
    Want want;
    want.SetParam(Constants::PARAM_FORM_DIMENSION_KEY, FORM_DIMENSION_1);
    want.SetParam(Constants::PARAM_FORM_NAME_KEY, PARAM_FORM_NAME1);
    want.SetParam(Constants::PARAM_MODULE_NAME_KEY, PARAM_PROVIDER_MODULE_NAME1);
    want.SetParam(Constants::PARAM_FORM_TEMPORARY_KEY, FORM_TEMP_FORM_FLAG_FALSE);
    want.SetElementName(FORM_TEST_DEVICEID, FORM_PROVIDER_BUNDLE_NAME1, FORM_PROVIDER_ABILITY_NAME1);
    // Set Want info end
    bool bResult = AcquireForm(0, want, callback);
    if (bResult) {
        APP_LOGI("%{public}s AcquireForm ok.", __func__);
    } else {
        APP_LOGE("%{public}s AcquireForm error.", __func__);
        FormTestUtils::PublishEvent(FORM_EVENT_RECV_DELETE_FORM_0800, EVENT_CODE_800, "");
    }
}
void FormAbilityDeleteForm::FMS_deleteForm_0900(std::string data)
{
    std::shared_ptr<AcquireFormCallback> callback =
        std::make_shared<AcquireFormCallback>(FORM_EVENT_RECV_DELETE_FORM_0900);
    callback->ability_ = this;
    // Set Want info begin
    Want want;
    want.SetParam(Constants::PARAM_FORM_DIMENSION_KEY, FORM_DIMENSION_1);
    want.SetParam(Constants::PARAM_FORM_NAME_KEY, PARAM_FORM_NAME1);
    want.SetParam(Constants::PARAM_MODULE_NAME_KEY, PARAM_PROVIDER_MODULE_NAME1);
    want.SetParam(Constants::PARAM_FORM_TEMPORARY_KEY, FORM_TEMP_FORM_FLAG_FALSE);
    want.SetElementName(FORM_TEST_DEVICEID, FORM_PROVIDER_BUNDLE_NAME1, FORM_PROVIDER_ABILITY_NAME1);
    // Set Want info end
    bool bResult = AcquireForm(0, want, callback);
    if (bResult) {
        APP_LOGI("%{public}s AcquireForm ok.", __func__);
    } else {
        APP_LOGE("%{public}s AcquireForm error.", __func__);
        FormTestUtils::PublishEvent(FORM_EVENT_RECV_DELETE_FORM_0900, EVENT_CODE_900, "");
    }
}

void FormAbilityDeleteForm::FMS_deleteForm_1000(std::string data)
{
    std::shared_ptr<AcquireFormCallback> callback =
        std::make_shared<AcquireFormCallback>(FORM_EVENT_RECV_DELETE_FORM_1000);
    callback->ability_ = this;
    // Set Want info begin
    Want want;
    want.SetParam(Constants::PARAM_FORM_DIMENSION_KEY, FORM_DIMENSION_1);
    want.SetParam(Constants::PARAM_FORM_NAME_KEY, PARAM_FORM_NAME1);
    want.SetParam(Constants::PARAM_MODULE_NAME_KEY, PARAM_PROVIDER_MODULE_NAME1);
    want.SetParam(Constants::PARAM_FORM_TEMPORARY_KEY, FORM_TEMP_FORM_FLAG_FALSE);
    want.SetElementName(FORM_TEST_DEVICEID, FORM_PROVIDER_BUNDLE_NAME1, FORM_PROVIDER_ABILITY_NAME1);
    // Set Want info end
    bool bResult = AcquireForm(0, want, callback);
    if (bResult) {
        APP_LOGI("%{public}s AcquireForm ok.", __func__);
    } else {
        APP_LOGE("%{public}s AcquireForm error.", __func__);
        FormTestUtils::PublishEvent(FORM_EVENT_RECV_DELETE_FORM_1000, EVENT_CODE_1000, "");
    }
}

void FormAbilityDeleteForm::FMS_deleteForm_1100(std::string data)
{
    std::shared_ptr<AcquireFormCallback> callback =
        std::make_shared<AcquireFormCallback>(FORM_EVENT_RECV_DELETE_FORM_1100);
    callback->ability_ = this;
    // Set Want info begin
    Want want;
    want.SetParam(Constants::PARAM_FORM_DIMENSION_KEY, FORM_DIMENSION_1);
    want.SetParam(Constants::PARAM_FORM_NAME_KEY, PARAM_FORM_NAME1);
    want.SetParam(Constants::PARAM_MODULE_NAME_KEY, PARAM_PROVIDER_MODULE_NAME1);
    want.SetParam(Constants::PARAM_FORM_TEMPORARY_KEY, FORM_TEMP_FORM_FLAG_TRUE);
    want.SetElementName(FORM_TEST_DEVICEID, FORM_PROVIDER_BUNDLE_NAME1, FORM_PROVIDER_ABILITY_NAME1);
    // Set Want info end
    bool bResult = AcquireForm(0, want, callback);
    if (bResult) {
        APP_LOGI("%{public}s AcquireForm ok.", __func__);
    } else {
        APP_LOGE("%{public}s AcquireForm error.", __func__);
        FormTestUtils::PublishEvent(FORM_EVENT_RECV_DELETE_FORM_1100, EVENT_CODE_1100, "");
    }
}

void FormAbilityDeleteForm::FMS_deleteForm_1200(std::string data)
{
    std::shared_ptr<AcquireFormCallback> callback =
        std::make_shared<AcquireFormCallback>(FORM_EVENT_RECV_DELETE_FORM_1200);
    callback->ability_ = this;
    // Set Want info begin
    Want want;
    want.SetParam(Constants::PARAM_FORM_DIMENSION_KEY, FORM_DIMENSION_1);
    want.SetParam(Constants::PARAM_FORM_NAME_KEY, PARAM_FORM_NAME1);
    want.SetParam(Constants::PARAM_MODULE_NAME_KEY, PARAM_PROVIDER_MODULE_NAME1);
    if (data == "true") {
        want.SetParam(Constants::PARAM_FORM_TEMPORARY_KEY, FORM_TEMP_FORM_FLAG_TRUE);
    } else {
        want.SetParam(Constants::PARAM_FORM_TEMPORARY_KEY, FORM_TEMP_FORM_FLAG_FALSE);
    }
    want.SetElementName(FORM_TEST_DEVICEID, FORM_PROVIDER_BUNDLE_NAME1, FORM_PROVIDER_ABILITY_NAME1);
    // Set Want info end
    bool bResult = AcquireForm(0, want, callback);
    if (bResult) {
        APP_LOGI("%{public}s AcquireForm ok.", __func__);
    } else {
        APP_LOGE("%{public}s AcquireForm error.", __func__);
        FormTestUtils::PublishEvent(FORM_EVENT_RECV_DELETE_FORM_1200, EVENT_CODE_1200, "");
    }
}

void FormAbilityDeleteForm::FMS_deleteForm_1201(std::string data)
{
    APP_LOGI("%{public}s formId: %{public}s", __func__, data.c_str());
    bool bResult = DeleteForm(atoll(data.c_str()));
    if (bResult) {
        APP_LOGI("%{public}s DeleteForm end", __func__);
        FormTestUtils::PublishEvent(FORM_EVENT_RECV_DELETE_FORM_1201, EVENT_CODE_1201, "true");
    } else {
        APP_LOGE("%{public}s DeleteForm error", __func__);
        FormTestUtils::PublishEvent(FORM_EVENT_RECV_DELETE_FORM_1201, EVENT_CODE_1201, "false");
    }
}

void FormAbilityDeleteForm::FMS_deleteForm_1400(std::string data)
{
    std::shared_ptr<AcquireFormCallback> callback =
        std::make_shared<AcquireFormCallback>(FORM_EVENT_RECV_DELETE_FORM_1400);
    callback->ability_ = this;
    // Set Want info begin
    Want want;
    want.SetParam(Constants::PARAM_FORM_DIMENSION_KEY, FORM_DIMENSION_1);
    want.SetParam(Constants::PARAM_FORM_NAME_KEY, PARAM_FORM_NAME1);
    want.SetParam(Constants::PARAM_MODULE_NAME_KEY, PARAM_PROVIDER_MODULE_NAME1);
    if (data == "true") {
        want.SetParam(Constants::PARAM_FORM_TEMPORARY_KEY, FORM_TEMP_FORM_FLAG_TRUE);
    } else {
        want.SetParam(Constants::PARAM_FORM_TEMPORARY_KEY, FORM_TEMP_FORM_FLAG_FALSE);
    }
    want.SetElementName(FORM_TEST_DEVICEID, FORM_PROVIDER_BUNDLE_NAME1, FORM_PROVIDER_ABILITY_NAME1);
    // Set Want info end
    bool bResult = AcquireForm(0, want, callback);
    if (bResult) {
        APP_LOGI("%{public}s AcquireForm ok.", __func__);
    } else {
        APP_LOGE("%{public}s AcquireForm error.", __func__);
        FormTestUtils::PublishEvent(FORM_EVENT_RECV_DELETE_FORM_1400, EVENT_CODE_1400, "");
    }
}
void FormAbilityDeleteForm::FMS_deleteForm_1401(std::string data)
{
    APP_LOGI("%{public}s formId: %{public}s", __func__, data.c_str());
    bool bResult = DeleteForm(atoll(data.c_str()));
    if (bResult) {
        APP_LOGI("%{public}s DeleteForm end", __func__);
        FormTestUtils::PublishEvent(FORM_EVENT_RECV_DELETE_FORM_1401, EVENT_CODE_1401, "true");
    } else {
        APP_LOGE("%{public}s DeleteForm error", __func__);
        FormTestUtils::PublishEvent(FORM_EVENT_RECV_DELETE_FORM_1401, EVENT_CODE_1401, "false");
    }
}

void FormAbilityDeleteForm::FMS_deleteForm_1500(std::string data)
{
    std::shared_ptr<AcquireFormCallback> callback =
        std::make_shared<AcquireFormCallback>(FORM_EVENT_RECV_DELETE_FORM_1500);
    callback->ability_ = this;
    // Set Want info begin
    Want want;
    want.SetParam(Constants::PARAM_FORM_DIMENSION_KEY, FORM_DIMENSION_1);
    want.SetParam(Constants::PARAM_FORM_NAME_KEY, PARAM_FORM_NAME1);
    want.SetParam(Constants::PARAM_MODULE_NAME_KEY, PARAM_PROVIDER_MODULE_NAME1);
    want.SetParam(Constants::PARAM_FORM_TEMPORARY_KEY, FORM_TEMP_FORM_FLAG_TRUE);
    want.SetElementName(FORM_TEST_DEVICEID, FORM_PROVIDER_BUNDLE_NAME1, FORM_PROVIDER_ABILITY_NAME1);
    // Set Want info end
    bool bResult = AcquireForm(0, want, callback);
    if (bResult) {
        APP_LOGI("%{public}s AcquireForm ok.", __func__);
    } else {
        APP_LOGE("%{public}s AcquireForm error.", __func__);
        FormTestUtils::PublishEvent(FORM_EVENT_RECV_DELETE_FORM_1500, EVENT_CODE_1500, "");
    }
}

void FormAbilityDeleteForm::FMS_deleteForm_1501(std::string data)
{
    APP_LOGI("%{public}s formId: %{public}s", __func__, data.c_str());
    bool bResult = DeleteForm(atoll(data.c_str()));
    if (bResult) {
        APP_LOGI("%{public}s DeleteForm end", __func__);
        std::shared_ptr<AcquireFormCallback> callback =
            std::make_shared<AcquireFormCallback>(FORM_EVENT_RECV_DELETE_FORM_1501);
        callback->ability_ = this;
        // Set Want info begin
        Want want;
        want.SetParam(Constants::PARAM_FORM_DIMENSION_KEY, FORM_DIMENSION_1);
        want.SetParam(Constants::PARAM_FORM_NAME_KEY, PARAM_FORM_NAME1);
        want.SetParam(Constants::PARAM_MODULE_NAME_KEY, PARAM_PROVIDER_MODULE_NAME1);
        want.SetParam(Constants::PARAM_FORM_TEMPORARY_KEY, FORM_TEMP_FORM_FLAG_TRUE);
        want.SetElementName(FORM_TEST_DEVICEID, FORM_PROVIDER_BUNDLE_NAME1, FORM_PROVIDER_ABILITY_NAME1);
        // Set Want info end
        bool bResult = AcquireForm(0, want, callback);
        if (bResult) {
            APP_LOGI("%{public}s AcquireForm ok.", __func__);
        } else {
            APP_LOGE("%{public}s AcquireForm error.", __func__);
            FormTestUtils::PublishEvent(FORM_EVENT_RECV_DELETE_FORM_1501, EVENT_CODE_1510, "acquireError");
        }
    } else {
        APP_LOGE("%{public}s DeleteForm error", __func__);
        FormTestUtils::PublishEvent(FORM_EVENT_RECV_DELETE_FORM_1501, EVENT_CODE_1510, "deleteError");
    }
}
void FormAbilityDeleteForm::FMS_deleteForm_1502(std::string data)
{
    APP_LOGI("%{public}s formId: %{public}s", __func__, data.c_str());
    bool bResult = DeleteForm(atoll(data.c_str()));
    if (bResult) {
        APP_LOGI("%{public}s DeleteForm end", __func__);
        FormTestUtils::PublishEvent(FORM_EVENT_RECV_DELETE_FORM_1502, EVENT_CODE_1511, "true");
    } else {
        APP_LOGE("%{public}s DeleteForm error", __func__);
        FormTestUtils::PublishEvent(FORM_EVENT_RECV_DELETE_FORM_1502, EVENT_CODE_1511, "false");
    }
}
void FormAbilityDeleteForm::FMS_deleteForm_1600(std::string data)
{
    std::shared_ptr<AcquireFormCallback> callback =
        std::make_shared<AcquireFormCallback>(FORM_EVENT_RECV_DELETE_FORM_1600);
    callback->ability_ = this;
    // Set Want info begin
    Want want;
    want.SetParam(Constants::PARAM_FORM_DIMENSION_KEY, FORM_DIMENSION_1);
    want.SetParam(Constants::PARAM_FORM_NAME_KEY, PARAM_FORM_NAME1);
    want.SetParam(Constants::PARAM_MODULE_NAME_KEY, PARAM_PROVIDER_MODULE_NAME1);
    want.SetParam(Constants::PARAM_FORM_TEMPORARY_KEY, FORM_TEMP_FORM_FLAG_FALSE);
    want.SetElementName(FORM_TEST_DEVICEID, FORM_PROVIDER_BUNDLE_NAME1, FORM_PROVIDER_ABILITY_NAME1);
    // Set Want info end
    bool bResult = AcquireForm(0, want, callback);
    if (bResult) {
        APP_LOGI("%{public}s AcquireForm ok.", __func__);
    } else {
        APP_LOGE("%{public}s AcquireForm error.", __func__);
        FormTestUtils::PublishEvent(FORM_EVENT_RECV_DELETE_FORM_1600, EVENT_CODE_1600, "");
    }
}
void FormAbilityDeleteForm::FMS_deleteForm_1601(std::string data)
{
    APP_LOGI("%{public}s formId: %{public}s", __func__, data.c_str());
    bool bResult = DeleteForm(atoll(data.c_str()));
    if (bResult) {
        APP_LOGI("%{public}s DeleteForm end", __func__);
        std::shared_ptr<AcquireFormCallback> callback =
            std::make_shared<AcquireFormCallback>(FORM_EVENT_RECV_DELETE_FORM_1601);
        callback->ability_ = this;
        // Set Want info begin
        Want want;
        want.SetParam(Constants::PARAM_FORM_DIMENSION_KEY, FORM_DIMENSION_1);
        want.SetParam(Constants::PARAM_FORM_NAME_KEY, PARAM_FORM_NAME1);
        want.SetParam(Constants::PARAM_MODULE_NAME_KEY, PARAM_PROVIDER_MODULE_NAME1);
        want.SetParam(Constants::PARAM_FORM_TEMPORARY_KEY, FORM_TEMP_FORM_FLAG_FALSE);
        want.SetElementName(FORM_TEST_DEVICEID, FORM_PROVIDER_BUNDLE_NAME1, FORM_PROVIDER_ABILITY_NAME1);
        // Set Want info end
        bool bResult = AcquireForm(0, want, callback);
        if (bResult) {
            APP_LOGI("%{public}s AcquireForm ok.", __func__);
        } else {
            APP_LOGE("%{public}s AcquireForm error.", __func__);
            FormTestUtils::PublishEvent(FORM_EVENT_RECV_DELETE_FORM_1601, EVENT_CODE_1610, "acquireError");
        }
    } else {
        APP_LOGE("%{public}s DeleteForm error", __func__);
        FormTestUtils::PublishEvent(FORM_EVENT_RECV_DELETE_FORM_1601, EVENT_CODE_1610, "deleteError");
    }
}
void FormAbilityDeleteForm::FMS_deleteForm_1602(std::string data)
{
    APP_LOGI("%{public}s formId: %{public}s", __func__, data.c_str());
    bool bResult = DeleteForm(atoll(data.c_str()));
    if (bResult) {
        APP_LOGI("%{public}s DeleteForm end", __func__);
        FormTestUtils::PublishEvent(FORM_EVENT_RECV_DELETE_FORM_1602, EVENT_CODE_1611, "true");
    } else {
        APP_LOGE("%{public}s DeleteForm error", __func__);
        FormTestUtils::PublishEvent(FORM_EVENT_RECV_DELETE_FORM_1602, EVENT_CODE_1611, "false");
    }
}

void FormAbilityDeleteForm::FMS_deleteForm_1700(std::string data)
{
    std::shared_ptr<AcquireFormCallback> callback =
        std::make_shared<AcquireFormCallback>(FORM_EVENT_RECV_DELETE_FORM_1700);
    callback->ability_ = this;
    // Set Want info begin
    Want want;
    want.SetParam(Constants::PARAM_FORM_DIMENSION_KEY, FORM_DIMENSION_1);
    want.SetParam(Constants::PARAM_FORM_NAME_KEY, PARAM_FORM_NAME1);
    want.SetParam(Constants::PARAM_MODULE_NAME_KEY, PARAM_PROVIDER_MODULE_NAME1);
    want.SetParam(Constants::PARAM_FORM_TEMPORARY_KEY, FORM_TEMP_FORM_FLAG_FALSE);
    want.SetElementName(FORM_TEST_DEVICEID, FORM_PROVIDER_BUNDLE_NAME1, FORM_PROVIDER_ABILITY_NAME1);
    // Set Want info end
    bool bResult = AcquireForm(0, want, callback);
    if (bResult) {
        APP_LOGI("%{public}s AcquireForm ok.", __func__);
    } else {
        APP_LOGE("%{public}s AcquireForm error.", __func__);
        FormTestUtils::PublishEvent(FORM_EVENT_RECV_DELETE_FORM_1700, EVENT_CODE_1700, "");
    }
}

void FormAbilityDeleteForm::FMS_deleteForm_1701(std::string data)
{
    APP_LOGI("%{public}s formId: %{public}s", __func__, data.c_str());
    bool bResult = DeleteForm(atoll(data.c_str()));
    if (bResult) {
        APP_LOGI("%{public}s DeleteForm end", __func__);
        std::shared_ptr<AcquireFormCallback> callback =
            std::make_shared<AcquireFormCallback>(FORM_EVENT_RECV_DELETE_FORM_1701);
        callback->ability_ = this;
        // Set Want info begin
        Want want;
        want.SetParam(Constants::PARAM_FORM_DIMENSION_KEY, FORM_DIMENSION_1);
        want.SetParam(Constants::PARAM_FORM_NAME_KEY, PARAM_FORM_NAME1);
        want.SetParam(Constants::PARAM_MODULE_NAME_KEY, PARAM_PROVIDER_MODULE_NAME1);
        want.SetParam(Constants::PARAM_FORM_TEMPORARY_KEY, FORM_TEMP_FORM_FLAG_FALSE);
        want.SetElementName(FORM_TEST_DEVICEID, FORM_PROVIDER_BUNDLE_NAME1, FORM_PROVIDER_ABILITY_NAME1);
        // Set Want info end
        bool bResult = AcquireForm(0, want, callback);
        if (bResult) {
            APP_LOGI("%{public}s AcquireForm ok.", __func__);
        } else {
            APP_LOGE("%{public}s AcquireForm error.", __func__);
            FormTestUtils::PublishEvent(FORM_EVENT_RECV_DELETE_FORM_1701, EVENT_CODE_1701, "acquireError");
        }
    } else {
        APP_LOGE("%{public}s DeleteForm error", __func__);
        FormTestUtils::PublishEvent(FORM_EVENT_RECV_DELETE_FORM_1701, EVENT_CODE_1701, "deleteError");
    }
}
void FormAbilityDeleteForm::FMS_deleteForm_1702(std::string data)
{
    APP_LOGI("%{public}s formId: %{public}s", __func__, data.c_str());
    bool bResult = DeleteForm(atoll(data.c_str()));
    if (bResult) {
        APP_LOGI("%{public}s DeleteForm end", __func__);
        FormTestUtils::PublishEvent(FORM_EVENT_RECV_DELETE_FORM_1702, EVENT_CODE_1702, "true");
    } else {
        APP_LOGE("%{public}s DeleteForm error", __func__);
        FormTestUtils::PublishEvent(FORM_EVENT_RECV_DELETE_FORM_1702, EVENT_CODE_1702, "false");
    }
}
void FormAbilityDeleteForm::FMS_acquire_tempForm_batch(std::string data)
{
    APP_LOGI("%{public}s called", __func__);
    // Set Want info begin
    Want want;
    want.SetParam(Constants::PARAM_FORM_DIMENSION_KEY, FORM_DIMENSION_1);
    want.SetParam(Constants::PARAM_FORM_NAME_KEY, PARAM_FORM_NAME1);
    want.SetParam(Constants::PARAM_MODULE_NAME_KEY, PARAM_PROVIDER_MODULE_NAME1);
    want.SetParam(Constants::PARAM_FORM_TEMPORARY_KEY, FORM_TEMP_FORM_FLAG_TRUE);
    want.SetElementName(FORM_TEST_DEVICEID, FORM_PROVIDER_BUNDLE_NAME1, FORM_PROVIDER_ABILITY_NAME1);

    int formCount = std::stoi(data);
    APP_LOGI("%{public}s, formCount: %{public}d", __func__, formCount);
    want.SetParam(Constants::PARAM_FORM_ADD_COUNT, formCount);
    // Set Want info end
    int result = STtools::SystemTestFormUtil::BatchAddFormRecords(want);
    if (result == ERR_OK) {
        APP_LOGI("Batch add form end");
        FormTestUtils::PublishEvent(FORM_EVENT_RECV_ACQUIRE_TEMP_FORM_BATCH, EVENT_CODE_TEMP_BATCH, "true");
    } else {
        APP_LOGE("Batch add form error");
        FormTestUtils::PublishEvent(FORM_EVENT_RECV_ACQUIRE_TEMP_FORM_BATCH, EVENT_CODE_TEMP_BATCH, "false");
    }
}
void FormAbilityDeleteForm::FMS_acquireForm_batch(std::string data)
{
    APP_LOGI("%{public}s called", __func__);
    // Set Want info begin
    Want want;
    want.SetParam(Constants::PARAM_FORM_DIMENSION_KEY, FORM_DIMENSION_1);
    want.SetParam(Constants::PARAM_FORM_NAME_KEY, PARAM_FORM_NAME1);
    want.SetParam(Constants::PARAM_MODULE_NAME_KEY, PARAM_PROVIDER_MODULE_NAME1);
    want.SetParam(Constants::PARAM_FORM_TEMPORARY_KEY, FORM_TEMP_FORM_FLAG_FALSE);
    want.SetElementName(FORM_TEST_DEVICEID, FORM_PROVIDER_BUNDLE_NAME1, FORM_PROVIDER_ABILITY_NAME1);

    int formCount = std::stoi(data);
    APP_LOGI("%{public}s, formCount: %{public}d", __func__, formCount);
    want.SetParam(Constants::PARAM_FORM_ADD_COUNT, formCount);
    // Set Want info end
    int result = STtools::SystemTestFormUtil::BatchAddFormRecords(want);
    if (result == ERR_OK) {
        APP_LOGI("Batch add form end");
        FormTestUtils::PublishEvent(FORM_EVENT_RECV_ACQUIRE_FORM_BATCH, EVENT_CODE_BATCH, "true");
    } else {
        APP_LOGE("Batch add form error");
        FormTestUtils::PublishEvent(FORM_EVENT_RECV_ACQUIRE_FORM_BATCH, EVENT_CODE_BATCH, "false");
    }
}
void FormAbilityDeleteForm::FMS_deleteFormBatch(std::string strFormId)
{
    APP_LOGI("%{public}s called", __func__);
    int result = STtools::SystemTestFormUtil::ClearFormRecords();
    if (result == ERR_OK) {
        APP_LOGI("Clear form records end");
        FormTestUtils::PublishEvent(FORM_EVENT_RECV_CLEAR_FORM_BATCH, EVENT_CODE_CLEAR_BATCH, "true");
    } else {
        APP_LOGE("Clear form records error");
        FormTestUtils::PublishEvent(FORM_EVENT_RECV_CLEAR_FORM_BATCH, EVENT_CODE_CLEAR_BATCH, "false");
    }
}

void FormAbilityDeleteForm::DeleteForm_0300(int64_t form_id)
{
    bool bResult = DeleteForm(-1);
    Clear("FMS_deleteForm_0300", form_id);
    if (bResult) {
        APP_LOGI("%{public}s DeleteForm end", __func__);
        FormTestUtils::PublishEvent(FORM_EVENT_RECV_DELETE_FORM_0300, EVENT_CODE_300, "true");
    } else {
        APP_LOGE("%{public}s DeleteForm error", __func__);
        FormTestUtils::PublishEvent(FORM_EVENT_RECV_DELETE_FORM_0300, EVENT_CODE_300, "false");
    }
}

void FormAbilityDeleteForm::DeleteForm_0400(int64_t form_id)
{
    bool bResult = DeleteForm(0);
    Clear("FMS_deleteForm_0400", form_id);
    if (bResult) {
        APP_LOGI("%{public}s DeleteForm end", __func__);
        FormTestUtils::PublishEvent(FORM_EVENT_RECV_DELETE_FORM_0400, EVENT_CODE_400, "true");
    } else {
        APP_LOGE("%{public}s DeleteForm error", __func__);
        FormTestUtils::PublishEvent(FORM_EVENT_RECV_DELETE_FORM_0400, EVENT_CODE_400, "false");
    }
}

void FormAbilityDeleteForm::DeleteForm_0500(int64_t form_id)
{}

void FormAbilityDeleteForm::DeleteForm_0600(int64_t form_id)
{
    bool bResult = DeleteForm(form_id);
    if (bResult) {
        APP_LOGI("%{public}s DeleteForm end", __func__);
        FormTestUtils::PublishEvent(FORM_EVENT_RECV_DELETE_FORM_0600, EVENT_CODE_600, "true");
    } else {
        APP_LOGE("%{public}s DeleteForm error", __func__);
        FormTestUtils::PublishEvent(FORM_EVENT_RECV_DELETE_FORM_0600, EVENT_CODE_600, "false");
    }
}

void FormAbilityDeleteForm::DeleteForm_0700(int64_t form_id)
{
    bool bResult = DeleteForm(form_id);
    if (bResult) {
        APP_LOGI("%{public}s DeleteForm end", __func__);
        FormTestUtils::PublishEvent(FORM_EVENT_RECV_DELETE_FORM_0700, EVENT_CODE_700, std::to_string(form_id));
    } else {
        APP_LOGE("%{public}s DeleteForm error", __func__);
        FormTestUtils::PublishEvent(FORM_EVENT_RECV_DELETE_FORM_0700, EVENT_CODE_700, "false");
    }
}

void FormAbilityDeleteForm::DeleteForm_0800(int64_t form_id)
{
    bool bResult = DeleteForm(form_id);
    if (bResult) {
        APP_LOGI("%{public}s DeleteForm end", __func__);
        FormTestUtils::PublishEvent(FORM_EVENT_RECV_DELETE_FORM_0800, EVENT_CODE_800, std::to_string(form_id));
    } else {
        APP_LOGE("%{public}s DeleteForm error", __func__);
        FormTestUtils::PublishEvent(FORM_EVENT_RECV_DELETE_FORM_0800, EVENT_CODE_800, "false");
    }
}

void FormAbilityDeleteForm::DeleteForm_0900(int64_t form_id)
{
    bool bResult = DeleteForm(form_id);
    if (bResult) {
        APP_LOGI("%{public}s DeleteForm1 end", __func__);
        bool realResult = DeleteForm(form_id);
        if (realResult) {
            APP_LOGI("%{public}s DeleteForm2 end", __func__);
            FormTestUtils::PublishEvent(FORM_EVENT_RECV_DELETE_FORM_0900, EVENT_CODE_900, "true");
        } else {
            APP_LOGE("%{public}s DeleteForm2 error", __func__);
            FormTestUtils::PublishEvent(FORM_EVENT_RECV_DELETE_FORM_0900, EVENT_CODE_900, "false");
        }
    } else {
        APP_LOGE("%{public}s DeleteForm1 error", __func__);
        FormTestUtils::PublishEvent(FORM_EVENT_RECV_DELETE_FORM_0900, EVENT_CODE_900, "");
    }
}

void FormAbilityDeleteForm::DeleteForm_1000(int64_t form_id)
{
    bool bResult = ReleaseForm(form_id);
    if (bResult) {
        APP_LOGI("%{public}s ReleaseForm end", __func__);
        bool realResult = DeleteForm(form_id);
        if (realResult) {
            APP_LOGI("%{public}s DeleteForm end", __func__);
            FormTestUtils::PublishEvent(FORM_EVENT_RECV_DELETE_FORM_1000, EVENT_CODE_1000, "true");
        } else {
            APP_LOGE("%{public}s DeleteForm error", __func__);
            FormTestUtils::PublishEvent(FORM_EVENT_RECV_DELETE_FORM_1000, EVENT_CODE_1000, "false");
        }
    } else {
        APP_LOGE("%{public}s ReleaseForm error", __func__);
        FormTestUtils::PublishEvent(FORM_EVENT_RECV_DELETE_FORM_1000, EVENT_CODE_1000, "");
    }
}
void FormAbilityDeleteForm::DeleteForm_1100(int64_t form_id)
{
    bool bResult = DeleteForm(form_id);
    if (bResult) {
        APP_LOGI("%{public}s DeleteForm end", __func__);
        FormTestUtils::PublishEvent(FORM_EVENT_RECV_DELETE_FORM_1100, EVENT_CODE_1100, std::to_string(form_id));
    } else {
        APP_LOGE("%{public}s DeleteForm error", __func__);
        FormTestUtils::PublishEvent(FORM_EVENT_RECV_DELETE_FORM_1100, EVENT_CODE_1100, "false");
    }
}
void FormAbilityDeleteForm::DeleteForm_1200(int64_t form_id)
{
    FormTestUtils::PublishEvent(FORM_EVENT_RECV_DELETE_FORM_1200, EVENT_CODE_1200, std::to_string(form_id));
}
void FormAbilityDeleteForm::DeleteForm_1400(int64_t form_id)
{
    FormTestUtils::PublishEvent(FORM_EVENT_RECV_DELETE_FORM_1400, EVENT_CODE_1400, std::to_string(form_id));
}
void FormAbilityDeleteForm::DeleteForm_1500(int64_t form_id)
{
    FormTestUtils::PublishEvent(FORM_EVENT_RECV_DELETE_FORM_1500, EVENT_CODE_1500, std::to_string(form_id));
}
void FormAbilityDeleteForm::DeleteForm_1501(int64_t form_id)
{
    FormTestUtils::PublishEvent(FORM_EVENT_RECV_DELETE_FORM_1501, EVENT_CODE_1510, std::to_string(form_id));
}
void FormAbilityDeleteForm::DeleteForm_1600(int64_t form_id)
{
    FormTestUtils::PublishEvent(FORM_EVENT_RECV_DELETE_FORM_1600, EVENT_CODE_1600, std::to_string(form_id));
}
void FormAbilityDeleteForm::DeleteForm_1601(int64_t form_id)
{
    FormTestUtils::PublishEvent(FORM_EVENT_RECV_DELETE_FORM_1601, EVENT_CODE_1610, std::to_string(form_id));
}
void FormAbilityDeleteForm::DeleteForm_1700(int64_t form_id)
{
    FormTestUtils::PublishEvent(FORM_EVENT_RECV_DELETE_FORM_1700, EVENT_CODE_1700, std::to_string(form_id));
}
void FormAbilityDeleteForm::DeleteForm_1701(int64_t form_id)
{
    FormTestUtils::PublishEvent(FORM_EVENT_RECV_DELETE_FORM_1701, EVENT_CODE_1701, std::to_string(form_id));
}
void FormAbilityDeleteForm::OnStart(const Want &want)
{
    APP_LOGI("FormAbilityDeleteForm::onStart");
    Ability::OnStart(want);

    std::vector<std::string> eventList = {
        FORM_EVENT_REQ_DELETE_FORM_0300,
        FORM_EVENT_REQ_DELETE_FORM_0400,
        FORM_EVENT_REQ_DELETE_FORM_0500,
        FORM_EVENT_REQ_DELETE_FORM_0600,
        FORM_EVENT_REQ_DELETE_FORM_0700,
        FORM_EVENT_REQ_DELETE_FORM_0800,
        FORM_EVENT_REQ_DELETE_FORM_0900,
        FORM_EVENT_REQ_DELETE_FORM_1000,
        FORM_EVENT_REQ_DELETE_FORM_1100,
        FORM_EVENT_REQ_DELETE_FORM_1200,
        FORM_EVENT_REQ_DELETE_FORM_1201,
        FORM_EVENT_REQ_DELETE_FORM_1400,
        FORM_EVENT_REQ_DELETE_FORM_1401,
        FORM_EVENT_REQ_DELETE_FORM_1500,
        FORM_EVENT_REQ_DELETE_FORM_1501,
        FORM_EVENT_REQ_DELETE_FORM_1502,
        FORM_EVENT_REQ_DELETE_FORM_1600,
        FORM_EVENT_REQ_DELETE_FORM_1601,
        FORM_EVENT_REQ_DELETE_FORM_1602,
        FORM_EVENT_REQ_DELETE_FORM_1700,
        FORM_EVENT_REQ_DELETE_FORM_1701,
        FORM_EVENT_REQ_DELETE_FORM_1702,
        FORM_EVENT_REQ_ACQUIRE_FORM_BATCH,
        FORM_EVENT_REQ_ACQUIRE_TEMP_FORM_BATCH,
        FORM_EVENT_REQ_CLEAR_FORM_BATCH
    };
    SubscribeEvent(eventList);
}
void FormAbilityDeleteForm::OnActive()
{
    APP_LOGI("FormAbilityDeleteForm::OnActive");
    Ability::OnActive();
    std::string eventData = GetAbilityName() + FORM_ABILITY_STATE_ONACTIVE;
    FormTestUtils::PublishEvent(FORM_EVENT_ABILITY_ONACTIVED, 0, eventData);
}

void FormAbilityDeleteForm::OnStop()
{
    APP_LOGI("FormAbilityDeleteForm::OnStop");

    Ability::OnStop();
}
void FormAbilityDeleteForm::OnInactive()
{
    APP_LOGI("FormAbilityDeleteForm::OnInactive");

    Ability::OnInactive();
}
void FormAbilityDeleteForm::OnBackground()
{
    APP_LOGI("FormAbilityDeleteForm::OnBackground");

    Ability::OnBackground();
}
void FormAbilityDeleteForm::SubscribeEvent(const std::vector<std::string> &eventList)
{
    MatchingSkills matchingSkills;
    for (const auto &e : eventList) {
        matchingSkills.AddEvent(e);
    }
    CommonEventSubscribeInfo subscribeInfo(matchingSkills);
    subscribeInfo.SetPriority(1);
    subscriber_ = std::make_shared<FormEventSubscriberForDeleteForm>(subscribeInfo);
    subscriber_->ability_ = this;
    CommonEventManager::SubscribeCommonEvent(subscriber_);
}

// KitTest End
void FormAbilityDeleteForm::Init(const std::shared_ptr<AbilityInfo> &abilityInfo,
    const std::shared_ptr<OHOSApplication> &application, std::shared_ptr<AbilityHandler> &handler,
    const sptr<IRemoteObject> &token)
{
    APP_LOGI("FormAbilityDeleteForm::Init");
    Ability::Init(abilityInfo, application, handler, token);
    memberFuncMap_[FORM_EVENT_REQ_DELETE_FORM_0300] = &FormAbilityDeleteForm::FMS_deleteForm_0300;
    memberFuncMap_[FORM_EVENT_REQ_DELETE_FORM_0400] = &FormAbilityDeleteForm::FMS_deleteForm_0400;
    memberFuncMap_[FORM_EVENT_REQ_DELETE_FORM_0500] = &FormAbilityDeleteForm::FMS_deleteForm_0500;
    memberFuncMap_[FORM_EVENT_REQ_DELETE_FORM_0600] = &FormAbilityDeleteForm::FMS_deleteForm_0600;
    memberFuncMap_[FORM_EVENT_REQ_DELETE_FORM_0700] = &FormAbilityDeleteForm::FMS_deleteForm_0700;
    memberFuncMap_[FORM_EVENT_REQ_DELETE_FORM_0800] = &FormAbilityDeleteForm::FMS_deleteForm_0800;
    memberFuncMap_[FORM_EVENT_REQ_DELETE_FORM_0900] = &FormAbilityDeleteForm::FMS_deleteForm_0900;
    memberFuncMap_[FORM_EVENT_REQ_DELETE_FORM_1000] = &FormAbilityDeleteForm::FMS_deleteForm_1000;
    memberFuncMap_[FORM_EVENT_REQ_DELETE_FORM_1100] = &FormAbilityDeleteForm::FMS_deleteForm_1100;
    memberFuncMap_[FORM_EVENT_REQ_DELETE_FORM_1200] = &FormAbilityDeleteForm::FMS_deleteForm_1200;
    memberFuncMap_[FORM_EVENT_REQ_DELETE_FORM_1201] = &FormAbilityDeleteForm::FMS_deleteForm_1201;
    memberFuncMap_[FORM_EVENT_REQ_DELETE_FORM_1400] = &FormAbilityDeleteForm::FMS_deleteForm_1400;
    memberFuncMap_[FORM_EVENT_REQ_DELETE_FORM_1401] = &FormAbilityDeleteForm::FMS_deleteForm_1401;
    memberFuncMap_[FORM_EVENT_REQ_DELETE_FORM_1500] = &FormAbilityDeleteForm::FMS_deleteForm_1500;
    memberFuncMap_[FORM_EVENT_REQ_DELETE_FORM_1501] = &FormAbilityDeleteForm::FMS_deleteForm_1501;
    memberFuncMap_[FORM_EVENT_REQ_DELETE_FORM_1502] = &FormAbilityDeleteForm::FMS_deleteForm_1502;
    memberFuncMap_[FORM_EVENT_REQ_DELETE_FORM_1600] = &FormAbilityDeleteForm::FMS_deleteForm_1600;
    memberFuncMap_[FORM_EVENT_REQ_DELETE_FORM_1601] = &FormAbilityDeleteForm::FMS_deleteForm_1601;
    memberFuncMap_[FORM_EVENT_REQ_DELETE_FORM_1602] = &FormAbilityDeleteForm::FMS_deleteForm_1602;
    memberFuncMap_[FORM_EVENT_REQ_DELETE_FORM_1700] = &FormAbilityDeleteForm::FMS_deleteForm_1700;
    memberFuncMap_[FORM_EVENT_REQ_DELETE_FORM_1701] = &FormAbilityDeleteForm::FMS_deleteForm_1701;
    memberFuncMap_[FORM_EVENT_REQ_DELETE_FORM_1702] = &FormAbilityDeleteForm::FMS_deleteForm_1702;
    memberFuncMap_[FORM_EVENT_REQ_ACQUIRE_FORM_BATCH] = &FormAbilityDeleteForm::FMS_acquireForm_batch;
    memberFuncMap_[FORM_EVENT_REQ_ACQUIRE_TEMP_FORM_BATCH] = &FormAbilityDeleteForm::FMS_acquire_tempForm_batch;
    memberFuncMap_[FORM_EVENT_REQ_CLEAR_FORM_BATCH] = &FormAbilityDeleteForm::FMS_deleteFormBatch;
}

void FormAbilityDeleteForm::handleEvent(std::string action, std::string data)
{
    APP_LOGI("%{public}s called", __func__);
    auto itFunc = memberFuncMap_.find(action);
    if (itFunc != memberFuncMap_.end()) {
        auto memberFunc = itFunc->second;
        if (memberFunc != nullptr) {
            return (this->*memberFunc)(data);
        }
    }
}

void FormAbilityDeleteForm::Clear(std::string case_id, int64_t form_id)
{
    bool bResult = DeleteForm(form_id);
    if (bResult) {
        APP_LOGI("[%{public}s] clear ok", case_id.c_str());
    } else {
        APP_LOGE("[%{public}s] clear error", case_id.c_str());
    }
}

void FormEventSubscriberForDeleteForm::OnReceiveEvent(const CommonEventData &data)
{
    APP_LOGI("FormEventSubscriberForDeleteForm::OnReceiveEvent:event=%{public}s", data.GetWant().GetAction().c_str());
    APP_LOGI("FormEventSubscriberForDeleteForm::OnReceiveEvent:code=%{public}d", data.GetCode());
    auto eventName = data.GetWant().GetAction();
    ability_->handleEvent(eventName, data.GetData());
    CommonEventManager::UnSubscribeCommonEvent(ability_->subscriber_);
}

void FormEventSubscriberForDeleteForm::KitTerminateAbility()
{
    if (ability_ != nullptr) {
        ability_->TerminateAbility();
    }
}

REGISTER_AA(FormAbilityDeleteForm)
}  // namespace AppExecFwk
}  // namespace OHOS