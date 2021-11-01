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
#include <iostream>
#include <numeric>
#include <sstream>

#include "app_log_wrapper.h"
#include "form_st_common_info.h"
#include "form_test_utils.h"
#include "form_ability_stress.h"

namespace {
    using namespace OHOS::AAFwk;
    using namespace OHOS::EventFwk;
}

namespace OHOS {
namespace AppExecFwk {
const int ONE = 1;
const int TWO = 2;
std::vector<std::string> eventList = {
    FORM_EVENT_REQ_STRESS_TEST_0100, FORM_EVENT_REQ_STRESS_TEST_0100_01, FORM_EVENT_REQ_STRESS_TEST_0200,
    FORM_EVENT_REQ_STRESS_TEST_0200_01, FORM_EVENT_REQ_STRESS_TEST_0200_02, FORM_EVENT_REQ_STRESS_TEST_0200_03,
    FORM_EVENT_REQ_STRESS_TEST_0300, FORM_EVENT_REQ_STRESS_TEST_0300_01, FORM_EVENT_REQ_STRESS_TEST_0300_02,
    FORM_EVENT_REQ_STRESS_TEST_0400, FORM_EVENT_REQ_STRESS_TEST_0500,
    FORM_EVENT_REQ_STRESS_TEST_1100, FORM_EVENT_REQ_STRESS_TEST_1100_01, FORM_EVENT_REQ_STRESS_TEST_1100_02,
    FORM_EVENT_REQ_STRESS_TEST_1100_03, FORM_EVENT_REQ_STRESS_TEST_1300, FORM_EVENT_REQ_STRESS_TEST_1300_01,
    FORM_EVENT_REQ_STRESS_TEST_1300_02, FORM_EVENT_REQ_STRESS_TEST_1300_03,
    FORM_EVENT_REQ_STRESS_TEST_1700, FORM_EVENT_REQ_STRESS_TEST_1700_01, FORM_EVENT_REQ_STRESS_TEST_1700_02,
    FORM_EVENT_REQ_STRESS_TEST_1800,
};

static std::string g_bundleName = "com.form.formsystemtestservicea";
static std::string g_moduleName = "formmodule001";
void FormAbilityStress::StressFormCallback::OnAcquired(const int32_t result, const FormJsInfo &formJsInfo) const
{
    APP_LOGI("%{public}s called", __func__);
    APP_LOGI("%{public}s formId: %{public}s", __func__, std::to_string(formJsInfo.formId).c_str());
    APP_LOGI("%{public}s bundleName: %{public}s", __func__, formJsInfo.bundleName.c_str());
    APP_LOGI("%{public}s abilityName: %{public}s", __func__, formJsInfo.abilityName.c_str());
    APP_LOGI("%{public}s formName: %{public}s", __func__, formJsInfo.formName.c_str());
    APP_LOGI("%{public}s formTempFlg: %{public}d", __func__, formJsInfo.formTempFlg);
    APP_LOGI("%{public}s jsFormCodePath: %{public}s", __func__, formJsInfo.jsFormCodePath.c_str());
    APP_LOGI("%{public}s formData: %{public}s", __func__, formJsInfo.formData.c_str());
    APP_LOGI("%{public}s formProviderData GetDataString: %{public}s",
        __func__, formJsInfo.formProviderData.GetDataString().c_str());

    APP_LOGI("%{public}s caseName_: %{public}s, code_: %{public}d", __func__, this->caseName_.c_str(), this->code_);

    if (this->caseName_ == FORM_EVENT_RECV_STRESS_TEST_0100) {
        FormTestUtils::PublishEvent(this->caseName_, EVENT_CODE_101, std::to_string(formJsInfo.formId));
    } else if (this->caseName_ == FORM_EVENT_RECV_STRESS_TEST_0200) {
        FormTestUtils::PublishEvent(this->caseName_, EVENT_CODE_201, std::to_string(formJsInfo.formId));
    } else if (this->caseName_ == FORM_EVENT_RECV_STRESS_TEST_0200_02) {
        FormTestUtils::PublishEvent(this->caseName_, EVENT_CODE_204, "true");
        ability_->FMS_stressTest_0200_sub03(std::to_string(formJsInfo.formId));
    } else if (this->caseName_ == FORM_EVENT_RECV_STRESS_TEST_0300) {
        FormTestUtils::PublishEvent(this->caseName_, EVENT_CODE_301, std::to_string(formJsInfo.formId));
    } else if (this->caseName_ == FORM_EVENT_RECV_STRESS_TEST_0400) {
        FormTestUtils::PublishEvent(this->caseName_, EVENT_CODE_401, std::to_string(formJsInfo.formId));
    } else if (this->caseName_ == FORM_EVENT_RECV_STRESS_TEST_1100) {
        FormTestUtils::PublishEvent(this->caseName_, EVENT_CODE_1101, std::to_string(formJsInfo.formId));
    } else if (this->caseName_ == FORM_EVENT_RECV_STRESS_TEST_1300) {
        FormTestUtils::PublishEvent(this->caseName_, EVENT_CODE_1301, std::to_string(formJsInfo.formId));
    } else if (this->caseName_ == FORM_EVENT_RECV_STRESS_TEST_1700) {
        FormTestUtils::PublishEvent(this->caseName_, EVENT_CODE_1701, std::to_string(formJsInfo.formId));
    } else if (this->caseName_ == FORM_EVENT_RECV_STRESS_TEST_1800) {
        FormTestUtils::PublishEvent(this->caseName_, EVENT_CODE_1801, std::to_string(formJsInfo.formId));
    } else {
        FormTestUtils::PublishEvent(this->caseName_, this->code_ + ONE, std::to_string(formJsInfo.formId));
    }
}

void FormAbilityStress::StressFormCallback::OnUpdate(const int32_t result, const FormJsInfo &formJsInfo) const
{
    APP_LOGI("%{public}s called", __func__);
    APP_LOGI("%{public}s formId: %{public}s", __func__, std::to_string(formJsInfo.formId).c_str());
    APP_LOGI("%{public}s bundleName: %{public}s", __func__, formJsInfo.bundleName.c_str());
    APP_LOGI("%{public}s abilityName: %{public}s", __func__, formJsInfo.abilityName.c_str());
    APP_LOGI("%{public}s formName: %{public}s", __func__, formJsInfo.formName.c_str());
    APP_LOGI("%{public}s formTempFlg: %{public}d", __func__, formJsInfo.formTempFlg);
    APP_LOGI("%{public}s jsFormCodePath: %{public}s", __func__, formJsInfo.jsFormCodePath.c_str());
    APP_LOGI("%{public}s formData: %{public}s", __func__, formJsInfo.formData.c_str());
    APP_LOGI("%{public}s formProviderData GetDataString: %{public}s",
        __func__, formJsInfo.formProviderData.GetDataString().c_str());

    APP_LOGI("%{public}s caseName_: %{public}s, code_: %{public}d", __func__, this->caseName_.c_str(), this->code_);

    if (this->caseName_ == FORM_EVENT_RECV_STRESS_TEST_0100) {
        FormTestUtils::PublishEvent(this->caseName_, EVENT_CODE_102, "true");
        ability_->FMS_stressTest_0100_sub01(std::to_string(formJsInfo.formId));
    } else if (this->caseName_ == FORM_EVENT_RECV_STRESS_TEST_0200) {
        FormTestUtils::PublishEvent(this->caseName_, EVENT_CODE_202, "true");
        ability_->FMS_stressTest_0200_sub01(std::to_string(formJsInfo.formId));
    } else if (this->caseName_ == FORM_EVENT_RECV_STRESS_TEST_0300) {
        FormTestUtils::PublishEvent(this->caseName_, EVENT_CODE_302, "true");
        ability_->FMS_stressTest_0300_sub01(std::to_string(formJsInfo.formId));
    } else if (this->caseName_ == FORM_EVENT_RECV_STRESS_TEST_0400) {
        FormTestUtils::PublishEvent(this->caseName_, EVENT_CODE_402, "true");
    } else if (this->caseName_ == FORM_EVENT_RECV_STRESS_TEST_1100) {
        FormTestUtils::PublishEvent(this->caseName_, EVENT_CODE_1102, "true");
        ability_->FMS_stressTest_1100_sub01(std::to_string(formJsInfo.formId));
    } else if (this->caseName_ == FORM_EVENT_RECV_STRESS_TEST_1300) {
        FormTestUtils::PublishEvent(this->caseName_, EVENT_CODE_1302, "true");
        ability_->FMS_stressTest_1300_sub01(std::to_string(formJsInfo.formId));
    } else if (this->caseName_ == FORM_EVENT_RECV_STRESS_TEST_1700) {
        FormTestUtils::PublishEvent(this->caseName_, EVENT_CODE_1702, "true");
    } else if (this->caseName_ == FORM_EVENT_RECV_STRESS_TEST_1800) {
        FormTestUtils::PublishEvent(this->caseName_, EVENT_CODE_1802, "true");
    } else {
        FormTestUtils::PublishEvent(this->caseName_, this->code_ + TWO, "true");
    }
}

void FormAbilityStress::StressFormCallback::OnFormUninstall(const int64_t formId) const
{
    APP_LOGI("%{public}s called", __func__);
}

void FormAbilityStress::FMS_stressTest_0100(std::string data)
{
    std::shared_ptr<StressFormCallback> callback =
    std::make_shared<StressFormCallback>(FORM_EVENT_RECV_STRESS_TEST_0100, EVENT_CODE_100);
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
        APP_LOGI("[FMS_stressTest_0100] AcquireForm end");
    } else {
        APP_LOGE("[FMS_stressTest_0100] AcquireForm error");
        FormTestUtils::PublishEvent(FORM_EVENT_RECV_STRESS_TEST_0100, EVENT_CODE_100, "false");
    }
}

void FormAbilityStress::FMS_stressTest_0100_sub01(std::string form_id)
{
    bool bResult = DeleteForm(atoll(form_id.c_str()));
    if (bResult) {
        APP_LOGI("[FMS_stressTest_0100] DeleteForm end");
        FormTestUtils::PublishEvent(FORM_EVENT_RECV_STRESS_TEST_0100_01, EVENT_CODE_103, "true");
    } else {
        APP_LOGE("[FMS_stressTest_0100] DeleteForm error");
        FormTestUtils::PublishEvent(FORM_EVENT_RECV_STRESS_TEST_0100_01, EVENT_CODE_103, "false");
    }
}

void FormAbilityStress::FMS_stressTest_0200(std::string data)
{
    std::shared_ptr<StressFormCallback> callback =
    std::make_shared<StressFormCallback>(FORM_EVENT_RECV_STRESS_TEST_0200, EVENT_CODE_200);
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
        APP_LOGI("[FMS_stressTest_0200] AcquireForm end");
    } else {
        APP_LOGE("[FMS_stressTest_0200] AcquireForm error");
        FormTestUtils::PublishEvent(FORM_EVENT_RECV_STRESS_TEST_0200, EVENT_CODE_200, "false");
    }
}

void FormAbilityStress::FMS_stressTest_0200_sub01(std::string form_id)
{
    bool isReleaseCache = false;
    bool bResult = ReleaseForm(atoll(form_id.c_str()), isReleaseCache);
    if (bResult) {
        APP_LOGI("[FMS_stressTest_0200] ReleaseForm end");
        FormTestUtils::PublishEvent(FORM_EVENT_RECV_STRESS_TEST_0200_01, EVENT_CODE_203, "true");
    } else {
        APP_LOGE("[FMS_stressTest_0200] ReleaseForm error");
        FormTestUtils::PublishEvent(FORM_EVENT_RECV_STRESS_TEST_0200_01, EVENT_CODE_203, "false");
    }

    FormAbilityStress::FMS_stressTest_0200_sub02(form_id);
}

void FormAbilityStress::FMS_stressTest_0200_sub02(std::string form_id)
{
    sleep(3);
    std::shared_ptr<StressFormCallback> callback =
    std::make_shared<StressFormCallback>(FORM_EVENT_RECV_STRESS_TEST_0200_02, EVENT_CODE_204);
    callback->ability_ = this;
    // Set Want info begin
    Want want;
    want.SetParam(Constants::PARAM_FORM_DIMENSION_KEY, FORM_DIMENSION_1);
    want.SetParam(Constants::PARAM_FORM_NAME_KEY, PARAM_FORM_NAME1);
    want.SetParam(Constants::PARAM_MODULE_NAME_KEY, PARAM_PROVIDER_MODULE_NAME1);
    want.SetParam(Constants::PARAM_FORM_TEMPORARY_KEY, FORM_TEMP_FORM_FLAG_FALSE);
    want.SetElementName(FORM_TEST_DEVICEID, FORM_PROVIDER_BUNDLE_NAME1, FORM_PROVIDER_ABILITY_NAME1);
    // Set Want info end
    bool bResult = AcquireForm(atoll(form_id.c_str()), want, callback);
    if (bResult) {
        APP_LOGI("[FMS_stressTest_0200] AcquireForm sub02 end");
    } else {
        APP_LOGE("[FMS_stressTest_0200] AcquireForm sub02 error");
        FormTestUtils::PublishEvent(FORM_EVENT_RECV_STRESS_TEST_0200_02, EVENT_CODE_204, "false");
    }
}

void FormAbilityStress::FMS_stressTest_0200_sub03(std::string form_id)
{
    bool bResult = DeleteForm(atoll(form_id.c_str()));
    if (bResult) {
        APP_LOGI("[FMS_stressTest_0200] DeleteForm end");
        FormTestUtils::PublishEvent(FORM_EVENT_RECV_STRESS_TEST_0200_03, EVENT_CODE_205, "true");
    } else {
        APP_LOGE("[FMS_stressTest_0200] DeleteForm error");
        FormTestUtils::PublishEvent(FORM_EVENT_RECV_STRESS_TEST_0200_03, EVENT_CODE_205, "false");
    }
}

void FormAbilityStress::FMS_stressTest_0300(std::string data)
{
    std::shared_ptr<StressFormCallback> callback =
    std::make_shared<StressFormCallback>(FORM_EVENT_RECV_STRESS_TEST_0300, EVENT_CODE_300);
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
        APP_LOGI("[FMS_stressTest_0300] AcquireForm end");
    } else {
        APP_LOGE("[FMS_stressTest_0300] AcquireForm error");
        FormTestUtils::PublishEvent(FORM_EVENT_RECV_STRESS_TEST_0300, EVENT_CODE_300, "false");
    }
}

void FormAbilityStress::FMS_stressTest_0300_sub01(std::string form_id)
{
    bool bResult = CastTempForm(atoll(form_id.c_str()));
    if (bResult) {
        APP_LOGI("[FMS_stressTest_0300] CastTempForm end");
        FormTestUtils::PublishEvent(FORM_EVENT_RECV_STRESS_TEST_0300_01, EVENT_CODE_303, "true");
    } else {
        APP_LOGE("[FMS_stressTest_0300] CastTempForm error");
        FormTestUtils::PublishEvent(FORM_EVENT_RECV_STRESS_TEST_0300_01, EVENT_CODE_303, "false");
    }

    FormAbilityStress::FMS_stressTest_0300_sub02(form_id);
}

void FormAbilityStress::FMS_stressTest_0300_sub02(std::string form_id)
{
    bool bResult = DeleteForm(atoll(form_id.c_str()));
    if (bResult) {
        APP_LOGI("[FMS_stressTest_0300] DeleteForm end");
        FormTestUtils::PublishEvent(FORM_EVENT_RECV_STRESS_TEST_0300_02, EVENT_CODE_304, "true");
    } else {
        APP_LOGE("[FMS_stressTest_0300] DeleteForm error");
        FormTestUtils::PublishEvent(FORM_EVENT_RECV_STRESS_TEST_0300_02, EVENT_CODE_304, "false");
    }
}

void FormAbilityStress::FMS_stressTest_0400(std::string data)
{
}

void FormAbilityStress::FMS_stressTest_0500(std::string data)
{
}

void FormAbilityStress::FMS_stressTest_0600(std::string data)
{
}

void FormAbilityStress::FMS_stressTest_0700(std::string data)
{
}

void FormAbilityStress::FMS_stressTest_1000(std::string data)
{
}

void FormAbilityStress::FMS_stressTest_1100(std::string data)
{
    std::shared_ptr<StressFormCallback> callback =
    std::make_shared<StressFormCallback>(FORM_EVENT_RECV_STRESS_TEST_1100, EVENT_CODE_1100);
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
        APP_LOGI("[FMS_stressTest_1100] AcquireForm end");
    } else {
        APP_LOGE("[FMS_stressTest_1100] AcquireForm error");
        FormTestUtils::PublishEvent(FORM_EVENT_RECV_STRESS_TEST_1100, EVENT_CODE_1100, "false");
    }
}

void FormAbilityStress::FMS_stressTest_1100_sub01(std::string form_id)
{
    std::vector<int64_t> formIds;
    formIds.push_back(atoll(form_id.c_str()));
    bool bResult = NotifyInvisibleForms(formIds);
    if (bResult) {
        APP_LOGI("[FMS_stressTest_1100] NotifyInvisibleForms end");
        FormTestUtils::PublishEvent(FORM_EVENT_RECV_STRESS_TEST_1100_01, EVENT_CODE_1103, "true");
    } else {
        APP_LOGE("[FMS_stressTest_1100] NotifyInvisibleForms error");
        FormTestUtils::PublishEvent(FORM_EVENT_RECV_STRESS_TEST_1100_01, EVENT_CODE_1103, "false");
    }

    FormAbilityStress::FMS_stressTest_1100_sub02(form_id);
}

void FormAbilityStress::FMS_stressTest_1100_sub02(std::string form_id)
{
    std::vector<int64_t> formIds;
    formIds.push_back(atoll(form_id.c_str()));
    bool bResult = NotifyVisibleForms(formIds);
    if (bResult) {
        APP_LOGI("[FMS_stressTest_1100] NotifyVisibleForms end");
        FormTestUtils::PublishEvent(FORM_EVENT_RECV_STRESS_TEST_1100_02, EVENT_CODE_1104, "true");
    } else {
        APP_LOGE("[FMS_stressTest_1100] NotifyVisibleForms error");
        FormTestUtils::PublishEvent(FORM_EVENT_RECV_STRESS_TEST_1100_02, EVENT_CODE_1104, "false");
    }

    FormAbilityStress::FMS_stressTest_1100_sub03(form_id);
}

void FormAbilityStress::FMS_stressTest_1100_sub03(std::string form_id)
{
    bool bResult = DeleteForm(atoll(form_id.c_str()));
    if (bResult) {
        APP_LOGI("[FMS_stressTest_1100] DeleteForm end");
        FormTestUtils::PublishEvent(FORM_EVENT_RECV_STRESS_TEST_1100_03, EVENT_CODE_1105, "true");
    } else {
        APP_LOGE("[FMS_stressTest_1100] DeleteForm error");
        FormTestUtils::PublishEvent(FORM_EVENT_RECV_STRESS_TEST_1100_03, EVENT_CODE_1105, "false");
    }
}

void FormAbilityStress::FMS_stressTest_1200(std::string data)
{
}

void FormAbilityStress::FMS_stressTest_1300(std::string data)
{
    std::shared_ptr<StressFormCallback> callback =
    std::make_shared<StressFormCallback>(FORM_EVENT_RECV_STRESS_TEST_1300, EVENT_CODE_1300);
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
        APP_LOGI("[FMS_stressTest_1300] AcquireForm end");
    } else {
        APP_LOGE("[FMS_stressTest_1300] AcquireForm error");
        FormTestUtils::PublishEvent(FORM_EVENT_RECV_STRESS_TEST_1300, EVENT_CODE_1300, "false");
    }
}

void FormAbilityStress::FMS_stressTest_1300_sub01(std::string form_id)
{
    std::vector<int64_t> formIds;
    formIds.push_back(atoll(form_id.c_str()));
    bool bResult = EnableUpdateForm(formIds);
    if (bResult) {
        APP_LOGI("[FMS_stressTest_1300] EnableUpdateForm end");
        FormTestUtils::PublishEvent(FORM_EVENT_RECV_STRESS_TEST_1300_01, EVENT_CODE_1303, "true");
    } else {
        APP_LOGE("[FMS_stressTest_1300] EnableUpdateForm error");
        FormTestUtils::PublishEvent(FORM_EVENT_RECV_STRESS_TEST_1300_01, EVENT_CODE_1303, "false");
    }

    FormAbilityStress::FMS_stressTest_1300_sub02(form_id);
}

void FormAbilityStress::FMS_stressTest_1300_sub02(std::string form_id)
{
    std::vector<int64_t> formIds;
    formIds.push_back(atoll(form_id.c_str()));
    bool bResult = DisableUpdateForm(formIds);
    if (bResult) {
        APP_LOGI("[FMS_stressTest_1300] DisableUpdateForm end");
        FormTestUtils::PublishEvent(FORM_EVENT_RECV_STRESS_TEST_1300_02, EVENT_CODE_1304, "true");
    } else {
        APP_LOGE("[FMS_stressTest_1300] DisableUpdateForm error");
        FormTestUtils::PublishEvent(FORM_EVENT_RECV_STRESS_TEST_1300_02, EVENT_CODE_1304, "false");
    }

    FormAbilityStress::FMS_stressTest_1300_sub03(form_id);
}

void FormAbilityStress::FMS_stressTest_1300_sub03(std::string form_id)
{
    bool bResult = DeleteForm(atoll(form_id.c_str()));
    if (bResult) {
        APP_LOGI("[FMS_stressTest_1300] DeleteForm end");
        FormTestUtils::PublishEvent(FORM_EVENT_RECV_STRESS_TEST_1300_03, EVENT_CODE_1305, "true");
    } else {
        APP_LOGE("[FMS_stressTest_1300] DeleteForm error");
        FormTestUtils::PublishEvent(FORM_EVENT_RECV_STRESS_TEST_1300_03, EVENT_CODE_1305, "false");
    }
}

void FormAbilityStress::FMS_stressTest_1400(std::string data)
{
}

void FormAbilityStress::FMS_stressTest_1500(std::string data)
{
}

void FormAbilityStress::FMS_stressTest_1600(std::string data)
{
}

void FormAbilityStress::FMS_stressTest_1700(std::string data)
{
    std::vector<FormInfo> formInfos;
    bool bResult = GetAllFormsInfo(formInfos);
    if (bResult) {
        APP_LOGI("[FMS_stressTest_1700] GetAllFormsInfo end");
        FormTestUtils::PublishEvent(FORM_EVENT_RECV_STRESS_TEST_1700, EVENT_CODE_1700, "true");
    } else {
        APP_LOGE("[FMS_stressTest_1700] GetAllFormsInfo error");
        FormTestUtils::PublishEvent(FORM_EVENT_RECV_STRESS_TEST_1700, EVENT_CODE_1700, "false");
    }

    FormAbilityStress::FMS_stressTest_1700_sub01(data);
}

void FormAbilityStress::FMS_stressTest_1700_sub01(std::string data)
{
    std::vector<FormInfo> formInfos;
    bool bResult = GetFormsInfoByApp(g_bundleName, formInfos);
    if (bResult) {
        APP_LOGI("[FMS_stressTest_1700] GetFormsInfoByApp end");
        FormTestUtils::PublishEvent(FORM_EVENT_RECV_STRESS_TEST_1700_01, EVENT_CODE_1701, "true");
    } else {
        APP_LOGE("[FMS_stressTest_1700] GetFormsInfoByApp error");
        FormTestUtils::PublishEvent(FORM_EVENT_RECV_STRESS_TEST_1700_01, EVENT_CODE_1701, "false");
    }

    FormAbilityStress::FMS_stressTest_1700_sub02(data);
}

void FormAbilityStress::FMS_stressTest_1700_sub02(std::string data)
{
    std::vector<FormInfo> formInfos;
    bool bResult = GetFormsInfoByModule(g_bundleName, g_moduleName, formInfos);
    if (bResult) {
        APP_LOGI("[FMS_stressTest_1700] GetFormsInfoByModule end");
        FormTestUtils::PublishEvent(FORM_EVENT_RECV_STRESS_TEST_1700_02, EVENT_CODE_1702, "true");
    } else {
        APP_LOGE("[FMS_stressTest_1700] GetFormsInfoByModule error");
        FormTestUtils::PublishEvent(FORM_EVENT_RECV_STRESS_TEST_1700_02, EVENT_CODE_1702, "false");
    }
}

void FormAbilityStress::FMS_stressTest_1800(std::string data)
{
    bool bResult = CheckFMSReady();
    if (bResult) {
        APP_LOGI("[FMS_stressTest_1800] CheckFMSReady end");
        FormTestUtils::PublishEvent(FORM_EVENT_RECV_STRESS_TEST_1800, EVENT_CODE_1800, "true");
    } else {
        APP_LOGE("[FMS_stressTest_1800] CheckFMSReady error");
        FormTestUtils::PublishEvent(FORM_EVENT_RECV_STRESS_TEST_1800, EVENT_CODE_1800, "false");
    }
}

void FormAbilityStress::OnStart(const Want &want)
{
    APP_LOGI("FormAbilityStress::onStart");
    Ability::OnStart(want);
}

void FormAbilityStress::OnActive()
{
    APP_LOGI("FormAbilityStress::OnActive");
    Ability::OnActive();
    std::string eventData = GetAbilityName() + FORM_ABILITY_STATE_ONACTIVE;
    FormTestUtils::PublishEvent(FORM_EVENT_ABILITY_ONACTIVED, 0, eventData);
}

void FormAbilityStress::OnStop()
{
    APP_LOGI("FormAbilityStress::OnStop");

    Ability::OnStop();
    CommonEventManager::UnSubscribeCommonEvent(subscriber_);
}

void FormAbilityStress::OnInactive()
{
    APP_LOGI("FormAbilityStress::OnInactive");

    Ability::OnInactive();
}

void FormAbilityStress::OnBackground()
{
    APP_LOGI("FormAbilityStress::OnBackground");

    Ability::OnBackground();
}

void FormAbilityStress::SubscribeEvent()
{
    APP_LOGI("FormAbilityStress::SubscribeEvent");
    MatchingSkills matchingSkills;
    for (const auto &e : eventList) {
        matchingSkills.AddEvent(e);
    }
    CommonEventSubscribeInfo subscribeInfo(matchingSkills);
    subscribeInfo.SetPriority(1);
    subscriber_ = std::make_shared<FormEventSubscriber>(subscribeInfo);
    subscriber_->ability_ = this;
    CommonEventManager::SubscribeCommonEvent(subscriber_);
}

// KitTest End
void FormAbilityStress::Init(const std::shared_ptr<AbilityInfo> &abilityInfo,
    const std::shared_ptr<OHOSApplication> &application, std::shared_ptr<AbilityHandler> &handler,
    const sptr<IRemoteObject> &token)
{
    APP_LOGI("FormAbilityStress::Init");
    Ability::Init(abilityInfo, application, handler, token);

    memberFuncMap_[FORM_EVENT_REQ_STRESS_TEST_0100] = &FormAbilityStress::FMS_stressTest_0100;
    memberFuncMap_[FORM_EVENT_REQ_STRESS_TEST_0100_01] = &FormAbilityStress::FMS_stressTest_0100_sub01;
    memberFuncMap_[FORM_EVENT_REQ_STRESS_TEST_0200] = &FormAbilityStress::FMS_stressTest_0200;
    memberFuncMap_[FORM_EVENT_REQ_STRESS_TEST_0200_01] = &FormAbilityStress::FMS_stressTest_0200_sub01;
    memberFuncMap_[FORM_EVENT_REQ_STRESS_TEST_0200_02] = &FormAbilityStress::FMS_stressTest_0200_sub02;
    memberFuncMap_[FORM_EVENT_REQ_STRESS_TEST_0200_03] = &FormAbilityStress::FMS_stressTest_0200_sub03;
    memberFuncMap_[FORM_EVENT_REQ_STRESS_TEST_0300] = &FormAbilityStress::FMS_stressTest_0300;
    memberFuncMap_[FORM_EVENT_REQ_STRESS_TEST_0300_01] = &FormAbilityStress::FMS_stressTest_0300_sub01;
    memberFuncMap_[FORM_EVENT_REQ_STRESS_TEST_0300_02] = &FormAbilityStress::FMS_stressTest_0300_sub02;
    memberFuncMap_[FORM_EVENT_REQ_STRESS_TEST_0400] = &FormAbilityStress::FMS_stressTest_0400;
    memberFuncMap_[FORM_EVENT_REQ_STRESS_TEST_0500] = &FormAbilityStress::FMS_stressTest_0500;
    memberFuncMap_[FORM_EVENT_REQ_STRESS_TEST_1100] = &FormAbilityStress::FMS_stressTest_1100;
    memberFuncMap_[FORM_EVENT_REQ_STRESS_TEST_1100_01] = &FormAbilityStress::FMS_stressTest_1100_sub01;
    memberFuncMap_[FORM_EVENT_REQ_STRESS_TEST_1100_02] = &FormAbilityStress::FMS_stressTest_1100_sub02;
    memberFuncMap_[FORM_EVENT_REQ_STRESS_TEST_1100_03] = &FormAbilityStress::FMS_stressTest_1100_sub03;
    memberFuncMap_[FORM_EVENT_REQ_STRESS_TEST_1300] = &FormAbilityStress::FMS_stressTest_1300;
    memberFuncMap_[FORM_EVENT_REQ_STRESS_TEST_1300_01] = &FormAbilityStress::FMS_stressTest_1300_sub01;
    memberFuncMap_[FORM_EVENT_REQ_STRESS_TEST_1300_02] = &FormAbilityStress::FMS_stressTest_1300_sub02;
    memberFuncMap_[FORM_EVENT_REQ_STRESS_TEST_1300_03] = &FormAbilityStress::FMS_stressTest_1300_sub03;
    memberFuncMap_[FORM_EVENT_REQ_STRESS_TEST_1700] = &FormAbilityStress::FMS_stressTest_1700;
    memberFuncMap_[FORM_EVENT_REQ_STRESS_TEST_1700_01] = &FormAbilityStress::FMS_stressTest_1700_sub01;
    memberFuncMap_[FORM_EVENT_REQ_STRESS_TEST_1700_02] = &FormAbilityStress::FMS_stressTest_1700_sub02;
    memberFuncMap_[FORM_EVENT_REQ_STRESS_TEST_1800] = &FormAbilityStress::FMS_stressTest_1800;

    SubscribeEvent();
}

void FormAbilityStress::handleEvent(std::string action, std::string data)
{
    APP_LOGI("%{public}s called", __func__);
    if (calledFuncMap_.find(action) != calledFuncMap_.end()) {
        return;
    }
    calledFuncMap_.emplace(action, 0);
    auto itFunc = memberFuncMap_.find(action);
    if (itFunc != memberFuncMap_.end()) {
        auto memberFunc = itFunc->second;
        if (memberFunc != nullptr) {
            return (this->*memberFunc)(data);
        }
    }
}

void FormAbilityStress::Clear()
{
}

void FormEventSubscriber::OnReceiveEvent(const CommonEventData &data)
{
    APP_LOGI("FormEventSubscriber::OnReceiveEvent:event=%{public}s, code=%{public}d, data=%{public}s",
        data.GetWant().GetAction().c_str(), data.GetCode(), data.GetData().c_str());
    auto eventName = data.GetWant().GetAction();
    ability_->handleEvent(eventName, data.GetData());
    CommonEventManager::UnSubscribeCommonEvent(ability_->subscriber_);
}

void FormEventSubscriber::KitTerminateAbility()
{
    if (ability_ != nullptr) {
        ability_->TerminateAbility();
    }
}

REGISTER_AA(FormAbilityStress)
}  // namespace AppExecFwk
}  // namespace OHOS