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
#include "form_ability_releaseform.h"
#include "app_log_wrapper.h"
#include "form_st_common_info.h"
#include "form_test_utils.h"

namespace {
using namespace OHOS::AAFwk;
using namespace OHOS::EventFwk;
}

namespace OHOS {
namespace AppExecFwk {
void FormAbilityReleaseForm::AcquireFormCallback::OnAcquired(const int32_t result, const FormJsInfo &formJsInfo) const
{
    APP_LOGI("%{public}s called", __func__);
    APP_LOGI("%{public}s receive formId:%{public}s", __func__, std::to_string(formJsInfo.formId).c_str());
    if (this->caseName_ == FORM_EVENT_RECV_RELEASE_FORM_0700 || this->caseName_ == FORM_EVENT_RECV_RELEASE_FORM_1000) {
        ability_->FMS_releaseForm_common(formJsInfo.formId, this->caseName_, this->lastformId_);
    }
}
void FormAbilityReleaseForm::AcquireFormCallback::OnUpdate(const int32_t result, const FormJsInfo &formJsInfo) const
{
    APP_LOGI("%{public}s called", __func__);
    APP_LOGI("%{public}s receive formData:%{public}s", __func__, formJsInfo.formData.c_str());
    if (this->caseName_ == FORM_EVENT_RECV_RELEASE_FORM_0300 || this->caseName_ == FORM_EVENT_RECV_RELEASE_FORM_0400
        || this->caseName_ == FORM_EVENT_RECV_RELEASE_FORM_0600 || this->caseName_ == FORM_EVENT_RECV_RELEASE_FORM_0800
        || this->caseName_ == FORM_EVENT_RECV_RELEASE_FORM_0900 || this->caseName_ == FORM_EVENT_RECV_RELEASE_FORM_1100
        || this->caseName_ == FORM_EVENT_RECV_RELEASE_FORM_1200 || this->caseName_ == FORM_EVENT_RECV_RELEASE_FORM_1300
        || this->caseName_ == FORM_EVENT_RECV_RELEASE_FORM_1400) {
        ability_->FMS_releaseForm_common(formJsInfo.formId, this->caseName_, this->lastformId_);
    }
}

void FormAbilityReleaseForm::AcquireFormCallback::OnFormUninstall(const int64_t formId) const
{
    APP_LOGI("%{public}s called", __func__);
}

void FormAbilityReleaseForm::FMS_releaseForm_common(int64_t formId, std::string caseName, std::string lastFormId)
{
    APP_LOGI("%{public}s called, formId: %{public}s, caseName:%{public}s, lastFormId:%{public}s",
        __func__, std::to_string(formId).c_str(), caseName.c_str(), lastFormId.c_str());
    if (caseName == FORM_EVENT_RECV_RELEASE_FORM_0300) {
        ReleaseForm_0300(formId);
    } else if (caseName == FORM_EVENT_RECV_RELEASE_FORM_0400) {
        ReleaseForm_0400(formId);
    } else if (caseName == FORM_EVENT_RECV_RELEASE_FORM_0600) {
        ReleaseForm_0600(formId);
    } else if (caseName == FORM_EVENT_RECV_RELEASE_FORM_0700) {
        ReleaseForm_0700(formId);
    } else if (caseName == FORM_EVENT_RECV_RELEASE_FORM_0800) {
        ReleaseForm_0800(formId);
    } else if (caseName == FORM_EVENT_RECV_RELEASE_FORM_0900) {
        ReleaseForm_0900(formId);
    } else if (caseName == FORM_EVENT_RECV_RELEASE_FORM_1000) {
        ReleaseForm_1000(formId);
    } else if (caseName == FORM_EVENT_RECV_RELEASE_FORM_1100) {
        ReleaseForm_1100(formId);
    } else if (caseName == FORM_EVENT_RECV_RELEASE_FORM_1200) {
        ReleaseForm_1200(formId);
    } else if (caseName == FORM_EVENT_RECV_RELEASE_FORM_1300) {
        ReleaseForm_1300(formId, lastFormId);
    } else if (caseName == FORM_EVENT_RECV_RELEASE_FORM_1400) {
        ReleaseForm_1400(formId);
    } else {
        return;
    }
}

void FormAbilityReleaseForm::FMS_releaseForm_0300(std::string data)
{
    std::shared_ptr<AcquireFormCallback> callback =
        std::make_shared<AcquireFormCallback>(FORM_EVENT_RECV_RELEASE_FORM_0300, data);
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
        FormTestUtils::PublishEvent(FORM_EVENT_RECV_RELEASE_FORM_0300, EVENT_CODE_300, "");
    }
}
void FormAbilityReleaseForm::FMS_releaseForm_0400(std::string data)
{
    std::shared_ptr<AcquireFormCallback> callback =
        std::make_shared<AcquireFormCallback>(FORM_EVENT_RECV_RELEASE_FORM_0400, data);
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
        FormTestUtils::PublishEvent(FORM_EVENT_RECV_RELEASE_FORM_0400, EVENT_CODE_400, "");
    }
}

void FormAbilityReleaseForm::FMS_releaseForm_0500(std::string data)
{
    APP_LOGI("%{public}s start[%{public}s]", __func__, data.c_str());
    bool bResult = ReleaseForm(atoll(data.c_str()), true);
    if (bResult) {
        APP_LOGI("%{public}s end", __func__);
        FormTestUtils::PublishEvent(FORM_EVENT_RECV_RELEASE_FORM_0500, EVENT_CODE_500, "true");
    } else {
        APP_LOGE("%{public}s error", __func__);
        FormTestUtils::PublishEvent(FORM_EVENT_RECV_RELEASE_FORM_0500, EVENT_CODE_500, "false");
    }
}

void FormAbilityReleaseForm::FMS_releaseForm_0600(std::string data)
{
    std::shared_ptr<AcquireFormCallback> callback =
        std::make_shared<AcquireFormCallback>(FORM_EVENT_RECV_RELEASE_FORM_0600, data);
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
        FormTestUtils::PublishEvent(FORM_EVENT_RECV_RELEASE_FORM_0600, EVENT_CODE_600, "");
    }
}

void FormAbilityReleaseForm::FMS_releaseForm_0700(std::string data)
{
    std::shared_ptr<AcquireFormCallback> callback =
        std::make_shared<AcquireFormCallback>(FORM_EVENT_RECV_RELEASE_FORM_0700, data);
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
        FormTestUtils::PublishEvent(FORM_EVENT_RECV_RELEASE_FORM_0700, EVENT_CODE_700, "");
    }
}

void FormAbilityReleaseForm::FMS_releaseForm_0800(std::string data)
{
    std::shared_ptr<AcquireFormCallback> callback =
        std::make_shared<AcquireFormCallback>(FORM_EVENT_RECV_RELEASE_FORM_0800, data);
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
        FormTestUtils::PublishEvent(FORM_EVENT_RECV_RELEASE_FORM_0800, EVENT_CODE_800, "");
    }
}
void FormAbilityReleaseForm::FMS_releaseForm_0900(std::string data)
{
    std::shared_ptr<AcquireFormCallback> callback =
        std::make_shared<AcquireFormCallback>(FORM_EVENT_RECV_RELEASE_FORM_0900, data);
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
        FormTestUtils::PublishEvent(FORM_EVENT_RECV_RELEASE_FORM_0900, EVENT_CODE_900, "");
    }
}

void FormAbilityReleaseForm::FMS_releaseForm_1000(std::string data)
{
    std::shared_ptr<AcquireFormCallback> callback =
        std::make_shared<AcquireFormCallback>(FORM_EVENT_RECV_RELEASE_FORM_1000, data);
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
        FormTestUtils::PublishEvent(FORM_EVENT_RECV_RELEASE_FORM_1000, EVENT_CODE_1000, "");
    }
}

void FormAbilityReleaseForm::FMS_releaseForm_1100(std::string data)
{
    std::shared_ptr<AcquireFormCallback> callback =
        std::make_shared<AcquireFormCallback>(FORM_EVENT_RECV_RELEASE_FORM_1100, data);
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
        FormTestUtils::PublishEvent(FORM_EVENT_RECV_RELEASE_FORM_1100, EVENT_CODE_1100, "");
    }
}

void FormAbilityReleaseForm::FMS_releaseForm_1200(std::string data)
{
    std::shared_ptr<AcquireFormCallback> callback =
        std::make_shared<AcquireFormCallback>(FORM_EVENT_RECV_RELEASE_FORM_1200, data);
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
        FormTestUtils::PublishEvent(FORM_EVENT_RECV_RELEASE_FORM_1200, EVENT_CODE_1200, "");
    }
}

void FormAbilityReleaseForm::FMS_releaseForm_1300(std::string data)
{
    std::shared_ptr<AcquireFormCallback> callback =
        std::make_shared<AcquireFormCallback>(FORM_EVENT_RECV_RELEASE_FORM_1300, data);
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
        FormTestUtils::PublishEvent(FORM_EVENT_RECV_RELEASE_FORM_1300, EVENT_CODE_1300, "");
    }
}

void FormAbilityReleaseForm::FMS_releaseForm_1400(std::string data)
{
    std::shared_ptr<AcquireFormCallback> callback =
        std::make_shared<AcquireFormCallback>(FORM_EVENT_RECV_RELEASE_FORM_1400, data);
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
        FormTestUtils::PublishEvent(FORM_EVENT_RECV_RELEASE_FORM_1400, EVENT_CODE_1400, "");
    }
}
void FormAbilityReleaseForm::FMS_releaseForm_common_del(std::string data)
{
    APP_LOGI("%{public}s formId: %{public}s", __func__, data.c_str());
    bool bResult = DeleteForm(atoll(data.c_str()));
    if (bResult) {
        APP_LOGI("%{public}s DeleteForm end", __func__);
        FormTestUtils::PublishEvent(FORM_EVENT_RECV_RELEASE_FORM_COMMON_DEL, EVENT_CODE_101, "true");
    } else {
        APP_LOGE("%{public}s DeleteForm error", __func__);
        FormTestUtils::PublishEvent(FORM_EVENT_RECV_RELEASE_FORM_COMMON_DEL, EVENT_CODE_101, "false");
    }
}

void FormAbilityReleaseForm::ReleaseForm_0300(int64_t form_id)
{
    int64_t formId = -1;
    bool isReleaseCache = true;
    bool bResult = ReleaseForm(formId, isReleaseCache);
    Clear("FMS_releaseForm_0300", form_id);
    if (bResult) {
        APP_LOGI("%{public}s ReleaseForm end", __func__);
        FormTestUtils::PublishEvent(FORM_EVENT_RECV_RELEASE_FORM_0300, EVENT_CODE_300, "true");
    } else {
        APP_LOGE("%{public}s ReleaseForm error", __func__);
        FormTestUtils::PublishEvent(FORM_EVENT_RECV_RELEASE_FORM_0300, EVENT_CODE_300, "false");
    }
}

void FormAbilityReleaseForm::ReleaseForm_0400(int64_t form_id)
{
    int64_t formId = 0;
    bool isReleaseCache = true;
    bool bResult = ReleaseForm(formId, isReleaseCache);
    Clear("FMS_releaseForm_0400", form_id);
    if (bResult) {
        APP_LOGI("%{public}s ReleaseForm end", __func__);
        FormTestUtils::PublishEvent(FORM_EVENT_RECV_RELEASE_FORM_0400, EVENT_CODE_400, "true");
    } else {
        APP_LOGE("%{public}s ReleaseForm error", __func__);
        FormTestUtils::PublishEvent(FORM_EVENT_RECV_RELEASE_FORM_0400, EVENT_CODE_400, "false");
    }
}

void FormAbilityReleaseForm::ReleaseForm_0500(int64_t form_id)
{}

void FormAbilityReleaseForm::ReleaseForm_0600(int64_t form_id)
{
    bool bResult = DeleteForm(form_id);
    if (bResult) {
        bool isReleaseCache = true;
        bool realResult = ReleaseForm(form_id, isReleaseCache);
        if (realResult) {
            APP_LOGI("%{public}s ReleaseForm end", __func__);
            FormTestUtils::PublishEvent(FORM_EVENT_RECV_RELEASE_FORM_0600, EVENT_CODE_600, "true");
        } else {
            APP_LOGE("%{public}s ReleaseForm error", __func__);
            FormTestUtils::PublishEvent(FORM_EVENT_RECV_RELEASE_FORM_0600, EVENT_CODE_600, "false");
        }
    } else {
        APP_LOGE("%{public}s DeleteForm error", __func__);
        FormTestUtils::PublishEvent(FORM_EVENT_RECV_RELEASE_FORM_0600, EVENT_CODE_600, "");
    }
}

void FormAbilityReleaseForm::ReleaseForm_0700(int64_t form_id)
{
    bool isReleaseCache = false;
    bool bResult = ReleaseForm(form_id, isReleaseCache);
    if (bResult) {
        APP_LOGI("%{public}s ReleaseForm end", __func__);
        FormTestUtils::PublishEvent(FORM_EVENT_RECV_RELEASE_FORM_0700, EVENT_CODE_700, "true");
    } else {
        APP_LOGE("%{public}s ReleaseForm error", __func__);
        FormTestUtils::PublishEvent(FORM_EVENT_RECV_RELEASE_FORM_0700, EVENT_CODE_700, "false");
    }
}
void FormAbilityReleaseForm::ReleaseForm_0800(int64_t form_id)
{
    bool isReleaseCache = false;
    bool bResult = ReleaseForm(form_id, isReleaseCache);
    if (bResult) {
        APP_LOGI("%{public}s ReleaseForm end", __func__);
        FormTestUtils::PublishEvent(FORM_EVENT_RECV_RELEASE_FORM_0800, EVENT_CODE_800, std::to_string(form_id));
    } else {
        APP_LOGE("%{public}s ReleaseForm error", __func__);
        FormTestUtils::PublishEvent(FORM_EVENT_RECV_RELEASE_FORM_0800, EVENT_CODE_800, "false");
    }
}
void FormAbilityReleaseForm::ReleaseForm_0900(int64_t form_id)
{
    bool isReleaseCache = false;
    bool bResult = ReleaseForm(form_id, isReleaseCache);
    if (bResult) {
        APP_LOGI("%{public}s ReleaseForm end", __func__);
        FormTestUtils::PublishEvent(FORM_EVENT_RECV_RELEASE_FORM_0900, EVENT_CODE_900, std::to_string(form_id));
    } else {
        APP_LOGE("%{public}s ReleaseForm error", __func__);
        FormTestUtils::PublishEvent(FORM_EVENT_RECV_RELEASE_FORM_0900, EVENT_CODE_900, "false");
    }
}
void FormAbilityReleaseForm::ReleaseForm_1000(int64_t form_id)
{
    bool isReleaseCache = true;
    bool bResult = ReleaseForm(form_id, isReleaseCache);
    if (bResult) {
        APP_LOGI("%{public}s ReleaseForm end", __func__);
        FormTestUtils::PublishEvent(FORM_EVENT_RECV_RELEASE_FORM_1000, EVENT_CODE_1000, "true");
    } else {
        APP_LOGE("%{public}s ReleaseForm error", __func__);
        FormTestUtils::PublishEvent(FORM_EVENT_RECV_RELEASE_FORM_1000, EVENT_CODE_1000, "false");
    }
}
void FormAbilityReleaseForm::ReleaseForm_1100(int64_t form_id)
{
    bool isReleaseCache = true;
    bool bResult = ReleaseForm(form_id, isReleaseCache);
    if (bResult) {
        APP_LOGI("%{public}s ReleaseForm end", __func__);
        FormTestUtils::PublishEvent(FORM_EVENT_RECV_RELEASE_FORM_1100, EVENT_CODE_1100, std::to_string(form_id));
    } else {
        APP_LOGE("%{public}s ReleaseForm error", __func__);
        FormTestUtils::PublishEvent(FORM_EVENT_RECV_RELEASE_FORM_1100, EVENT_CODE_1100, "false");
    }
}
void FormAbilityReleaseForm::ReleaseForm_1200(int64_t form_id)
{
    bool isReleaseCache = true;
    bool bResult = ReleaseForm(form_id, isReleaseCache);
    if (bResult) {
        APP_LOGI("%{public}s ReleaseForm end", __func__);
        FormTestUtils::PublishEvent(FORM_EVENT_RECV_RELEASE_FORM_1200, EVENT_CODE_1200, std::to_string(form_id));
    } else {
        APP_LOGE("%{public}s ReleaseForm error", __func__);
        FormTestUtils::PublishEvent(FORM_EVENT_RECV_RELEASE_FORM_1200, EVENT_CODE_1200, "false");
    }
}
void FormAbilityReleaseForm::ReleaseForm_1300(int64_t form_id, std::string lastFormId)
{
    if (lastFormId == "") {
        FormTestUtils::PublishEvent(FORM_EVENT_RECV_RELEASE_FORM_1300, EVENT_CODE_1300, std::to_string(form_id));
    } else {
        bool isReleaseCache = true;
        bool bResult1 = ReleaseForm(form_id, isReleaseCache);
        bool bResult2 = ReleaseForm(atoll(lastFormId.c_str()), isReleaseCache);
        Clear("FMS_releaseForm_1300", form_id);
        Clear("FMS_releaseForm_1300", atoll(lastFormId.c_str()));
        if (bResult1 && bResult2) {
            APP_LOGI("%{public}s ReleaseForm end", __func__);
            FormTestUtils::PublishEvent(FORM_EVENT_RECV_RELEASE_FORM_1300, EVENT_CODE_1300, "true");
        } else {
            APP_LOGE("%{public}s ReleaseForm error", __func__);
            FormTestUtils::PublishEvent(FORM_EVENT_RECV_RELEASE_FORM_1300, EVENT_CODE_1300, "false");
        }
    }
}
void FormAbilityReleaseForm::ReleaseForm_1400(int64_t form_id)
{
    bool isReleaseCache = true;
    bool bResult = ReleaseForm(form_id, isReleaseCache);
    Clear("FMS_releaseForm_1400", form_id);
    if (bResult) {
        APP_LOGI("%{public}s ReleaseForm end", __func__);
        FormTestUtils::PublishEvent(FORM_EVENT_RECV_RELEASE_FORM_1400, EVENT_CODE_1400, "true");
    } else {
        APP_LOGE("%{public}s ReleaseForm error", __func__);
        FormTestUtils::PublishEvent(FORM_EVENT_RECV_RELEASE_FORM_1400, EVENT_CODE_1400, "false");
    }
}

void FormAbilityReleaseForm::OnStart(const Want &want)
{
    APP_LOGI("FormAbilityReleaseForm::onStart");
    Ability::OnStart(want);

    std::vector<std::string> eventList = {
        FORM_EVENT_REQ_RELEASE_FORM_0300,
        FORM_EVENT_REQ_RELEASE_FORM_0400,
        FORM_EVENT_REQ_RELEASE_FORM_0500,
        FORM_EVENT_REQ_RELEASE_FORM_0600,
        FORM_EVENT_REQ_RELEASE_FORM_0700,
        FORM_EVENT_REQ_RELEASE_FORM_0800,
        FORM_EVENT_REQ_RELEASE_FORM_0900,
        FORM_EVENT_REQ_RELEASE_FORM_1000,
        FORM_EVENT_REQ_RELEASE_FORM_1100,
        FORM_EVENT_REQ_RELEASE_FORM_1200,
        FORM_EVENT_REQ_RELEASE_FORM_1300,
        FORM_EVENT_REQ_RELEASE_FORM_1400,
        FORM_EVENT_REQ_RELEASE_FORM_COMMON_DEL
    };
    SubscribeEvent(eventList);
}
void FormAbilityReleaseForm::OnActive()
{
    APP_LOGI("FormAbilityReleaseForm::OnActive");
    Ability::OnActive();
    std::string eventData = GetAbilityName() + FORM_ABILITY_STATE_ONACTIVE;
    FormTestUtils::PublishEvent(FORM_EVENT_ABILITY_ONACTIVED, 0, eventData);
}

void FormAbilityReleaseForm::OnStop()
{
    APP_LOGI("FormAbilityReleaseForm::OnStop");

    Ability::OnStop();
}
void FormAbilityReleaseForm::OnInactive()
{
    APP_LOGI("FormAbilityReleaseForm::OnInactive");

    Ability::OnInactive();
}
void FormAbilityReleaseForm::OnBackground()
{
    APP_LOGI("FormAbilityReleaseForm::OnBackground");

    Ability::OnBackground();
}
void FormAbilityReleaseForm::SubscribeEvent(const std::vector<std::string> &eventList)
{
    MatchingSkills matchingSkills;
    for (const auto &e : eventList) {
        matchingSkills.AddEvent(e);
    }
    CommonEventSubscribeInfo subscribeInfo(matchingSkills);
    subscribeInfo.SetPriority(1);
    subscriber_ = std::make_shared<FormEventSubscriberForReleaseForm>(subscribeInfo);
    subscriber_->ability_ = this;
    CommonEventManager::SubscribeCommonEvent(subscriber_);
}

// KitTest End
void FormAbilityReleaseForm::Init(const std::shared_ptr<AbilityInfo> &abilityInfo,
    const std::shared_ptr<OHOSApplication> &application, std::shared_ptr<AbilityHandler> &handler,
    const sptr<IRemoteObject> &token)
{
    APP_LOGI("FormAbilityReleaseForm::Init");
    Ability::Init(abilityInfo, application, handler, token);
    memberFuncMap_[FORM_EVENT_REQ_RELEASE_FORM_0300] = &FormAbilityReleaseForm::FMS_releaseForm_0300;
    memberFuncMap_[FORM_EVENT_REQ_RELEASE_FORM_0400] = &FormAbilityReleaseForm::FMS_releaseForm_0400;
    memberFuncMap_[FORM_EVENT_REQ_RELEASE_FORM_0500] = &FormAbilityReleaseForm::FMS_releaseForm_0500;
    memberFuncMap_[FORM_EVENT_REQ_RELEASE_FORM_0600] = &FormAbilityReleaseForm::FMS_releaseForm_0600;
    memberFuncMap_[FORM_EVENT_REQ_RELEASE_FORM_0700] = &FormAbilityReleaseForm::FMS_releaseForm_0700;
    memberFuncMap_[FORM_EVENT_REQ_RELEASE_FORM_0800] = &FormAbilityReleaseForm::FMS_releaseForm_0800;
    memberFuncMap_[FORM_EVENT_REQ_RELEASE_FORM_0900] = &FormAbilityReleaseForm::FMS_releaseForm_0900;
    memberFuncMap_[FORM_EVENT_REQ_RELEASE_FORM_1000] = &FormAbilityReleaseForm::FMS_releaseForm_1000;
    memberFuncMap_[FORM_EVENT_REQ_RELEASE_FORM_1100] = &FormAbilityReleaseForm::FMS_releaseForm_1100;
    memberFuncMap_[FORM_EVENT_REQ_RELEASE_FORM_1200] = &FormAbilityReleaseForm::FMS_releaseForm_1200;
    memberFuncMap_[FORM_EVENT_REQ_RELEASE_FORM_1300] = &FormAbilityReleaseForm::FMS_releaseForm_1300;
    memberFuncMap_[FORM_EVENT_REQ_RELEASE_FORM_1400] = &FormAbilityReleaseForm::FMS_releaseForm_1400;
    memberFuncMap_[FORM_EVENT_REQ_RELEASE_FORM_COMMON_DEL] = &FormAbilityReleaseForm::FMS_releaseForm_common_del;
}

void FormAbilityReleaseForm::handleEvent(std::string action, std::string data)
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

void FormAbilityReleaseForm::Clear(std::string case_id, int64_t form_id)
{
    bool bResult = DeleteForm(form_id);
    if (bResult) {
        APP_LOGI("[%{public}s] clear ok", case_id.c_str());
    } else {
        APP_LOGE("[%{public}s] clear error", case_id.c_str());
    }
}

void FormEventSubscriberForReleaseForm::OnReceiveEvent(const CommonEventData &data)
{
    APP_LOGI("FormEventSubscriberForReleaseForm::OnReceiveEvent:event=%{public}s", data.GetWant().GetAction().c_str());
    APP_LOGI("FormEventSubscriberForReleaseForm::OnReceiveEvent:code=%{public}d", data.GetCode());
    auto eventName = data.GetWant().GetAction();
    ability_->handleEvent(eventName, data.GetData());
    if ((data.GetCode() == EVENT_CODE_1300 && data.GetData() == "") ||
        eventName == FORM_EVENT_REQ_RELEASE_FORM_0700 || eventName == FORM_EVENT_REQ_RELEASE_FORM_0800 ||
        eventName == FORM_EVENT_REQ_RELEASE_FORM_0900 || eventName == FORM_EVENT_REQ_RELEASE_FORM_1000 ||
        eventName == FORM_EVENT_REQ_RELEASE_FORM_1100 || eventName == FORM_EVENT_REQ_RELEASE_FORM_1200) {
        return;
    } else {
        CommonEventManager::UnSubscribeCommonEvent(ability_->subscriber_);
    }
}

void FormEventSubscriberForReleaseForm::KitTerminateAbility()
{
    if (ability_ != nullptr) {
        ability_->TerminateAbility();
    }
}

REGISTER_AA(FormAbilityReleaseForm)
}  // namespace AppExecFwk
}  // namespace OHOS