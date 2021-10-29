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

#ifndef FORM_ST_COMMON_INFO_H
#define FORM_ST_COMMON_INFO_H

namespace OHOS {
namespace AppExecFwk {
const std::string PERMISSION_NAME_REQUIRE_FORM = "ohos.permission.REQUIRE_FORM";
const std::string FORM_TEST_DEVICEID = "0";

const std::string FORM_ABILITY_STATE_ONACTIVE = ":OnActive";
const std::string FORM_EVENT_ABILITY_ONACTIVED = "fms_req_acquireForm_ability_actived";

const int FORM_DIMENSION_1 = 1;
const bool FORM_TEMP_FORM_FLAG_FALSE = false;
const bool FORM_TEMP_FORM_FLAG_TRUE = true;

// provider ability
const std::string FORM_PROVIDER_BUNDLE_NAME1 = "com.form.formsystemtestservicea";
const std::string FORM_PROVIDER_BUNDLE_NAME2 = "com.form.formsystemtestserviceb";
const std::string FORM_PROVIDER_BUNDLE_NAME3 = "com.form.formsystemtestservicec";
const std::string FORM_PROVIDER_BUNDLE_NAME4 = "com.form.formsystemtestserviced";
const std::string FORM_PROVIDER_BUNDLE_NAME5 = "com.form.formsystemtestservicee";
const std::string FORM_PROVIDER_ABILITY_NAME1 = "FormStServiceAbilityA";
const std::string FORM_PROVIDER_ABILITY_NAME2 = "FormStServiceAbilityB";
const std::string FORM_PROVIDER_ABILITY_NAME3 = "FormStServiceAbilityC";
const std::string FORM_PROVIDER_ABILITY_NAME4 = "FormStServiceAbilityD";
const std::string FORM_PROVIDER_ABILITY_NAME5 = "FormStServiceAbilityE";
const std::string PARAM_PROVIDER_MODULE_NAME1 = "formmodule001";
const std::string PARAM_PROVIDER_MODULE_NAME2 = "formmodule001";
const std::string PARAM_FORM_NAME1 = "Form_Js001";
const std::string PARAM_FORM_NAME2 = "Form_Js002";

// add Form
const std::string FORM_EVENT_REQ_ACQUIRE_FORM_TEMP = "fms_req_acquireForm_temp";
const std::string FORM_EVENT_RECV_ACQUIRE_FORM_TEMP = "fms_recv_acquireForm_temp";
const std::string FORM_EVENT_REQ_DELETE_FORM_COMMON = "fms_req_deleteForm_common";
const std::string FORM_EVENT_RECV_DELETE_FORM_COMMON = "fms_recv_deleteForm_common";
const std::string FORM_EVENT_REQ_ACQUIRE_FORM_0100 = "fms_req_acquireForm_0100";
const std::string FORM_EVENT_RECV_ACQUIRE_FORM_0100 = "fms_recv_acquireForm_0100";
const std::string FORM_EVENT_REQ_ACQUIRE_FORM_0200 = "fms_req_acquireForm_0200";
const std::string FORM_EVENT_RECV_ACQUIRE_FORM_0200 = "fms_recv_acquireForm_0200";
const std::string FORM_EVENT_REQ_ACQUIRE_FORM_0300 = "fms_req_acquireForm_0300";
const std::string FORM_EVENT_RECV_ACQUIRE_FORM_0300 = "fms_recv_acquireForm_0300";
const std::string FORM_EVENT_REQ_ACQUIRE_FORM_0400 = "fms_req_acquireForm_0400";
const std::string FORM_EVENT_RECV_ACQUIRE_FORM_0400 = "fms_recv_acquireForm_0400";
const std::string FORM_EVENT_REQ_ACQUIRE_FORM_0500 = "fms_req_acquireForm_0500";
const std::string FORM_EVENT_RECV_ACQUIRE_FORM_0500 = "fms_recv_acquireForm_0500";
const std::string FORM_EVENT_REQ_ACQUIRE_FORM_0600 = "fms_req_acquireForm_0600";
const std::string FORM_EVENT_RECV_ACQUIRE_FORM_0600 = "fms_recv_acquireForm_0600";
const std::string FORM_EVENT_REQ_ACQUIRE_FORM_0700 = "fms_req_acquireForm_0700";
const std::string FORM_EVENT_RECV_ACQUIRE_FORM_0700 = "fms_recv_acquireForm_0700";
const std::string FORM_EVENT_REQ_ACQUIRE_FORM_1000 = "fms_req_acquireForm_1000";
const std::string FORM_EVENT_RECV_ACQUIRE_FORM_1000 = "fms_recv_acquireForm_1000";

const std::string FORM_EVENT_REQ_ACQUIRE_FORM_1100 = "fms_req_acquireForm_1100";
const std::string FORM_EVENT_RECV_ACQUIRE_FORM_1100 = "fms_recv_acquireForm_1100";
const std::string FORM_EVENT_REQ_ACQUIRE_FORM_1200 = "fms_req_acquireForm_1200";
const std::string FORM_EVENT_RECV_ACQUIRE_FORM_1200 = "fms_recv_acquireForm_1200";
const std::string FORM_EVENT_REQ_ACQUIRE_FORM_1400 = "fms_req_acquireForm_1400";
const std::string FORM_EVENT_RECV_ACQUIRE_FORM_1400 = "fms_recv_acquireForm_1400";
const std::string FORM_EVENT_REQ_ACQUIRE_FORM_1400_1 = "fms_req_acquireForm_1400_1";
const std::string FORM_EVENT_RECV_ACQUIRE_FORM_1400_1 = "fms_recv_acquireForm_1400_1";
const std::string FORM_EVENT_REQ_ACQUIRE_FORM_1500 = "fms_req_acquireForm_1500";
const std::string FORM_EVENT_RECV_ACQUIRE_FORM_1500 = "fms_recv_acquireForm_1500";
const std::string FORM_EVENT_REQ_ACQUIRE_FORM_1500_1 = "fms_req_acquireForm_1500_1";
const std::string FORM_EVENT_RECV_ACQUIRE_FORM_1500_1 = "fms_recv_acquireForm_1500_1";
const std::string FORM_EVENT_REQ_ACQUIRE_FORM_1600 = "fms_req_acquireForm_1600";
const std::string FORM_EVENT_RECV_ACQUIRE_FORM_1600 = "fms_recv_acquireForm_1600";
const std::string FORM_EVENT_REQ_ACQUIRE_FORM_1600_1 = "fms_req_acquireForm_1600_1";
const std::string FORM_EVENT_RECV_ACQUIRE_FORM_1600_1 = "fms_recv_acquireForm_1600_1";
const std::string FORM_EVENT_REQ_ACQUIRE_FORM_1800 = "fms_req_acquireForm_1800";
const std::string FORM_EVENT_RECV_ACQUIRE_FORM_1800 = "fms_recv_acquireForm_1800";


const std::string FORM_EVENT_REQ_ACQUIRE_FORM_1800_1 = "fms_req_acquireForm_1800_1";
const std::string FORM_EVENT_RECV_ACQUIRE_FORM_1800_1 = "fms_recv_acquireForm_1800_1";
const std::string FORM_EVENT_REQ_ACQUIRE_FORM_1800_2 = "fms_req_acquireForm_1800_2";
const std::string FORM_EVENT_RECV_ACQUIRE_FORM_1800_2 = "fms_recv_acquireForm_1800_2";
const std::string FORM_EVENT_REQ_ACQUIRE_FORM_1800_3 = "fms_req_acquireForm_1800_3";
const std::string FORM_EVENT_RECV_ACQUIRE_FORM_1800_3 = "fms_recv_acquireForm_1800_3";
const std::string FORM_EVENT_REQ_ACQUIRE_FORM_1900 = "fms_req_acquireForm_1900";
const std::string FORM_EVENT_RECV_ACQUIRE_FORM_1900 = "fms_recv_acquireForm_1900";
const std::string FORM_EVENT_REQ_ACQUIRE_FORM_2100 = "fms_req_acquireForm_2100";
const std::string FORM_EVENT_RECV_ACQUIRE_FORM_2100 = "fms_recv_acquireForm_2100";
const std::string FORM_EVENT_REQ_ACQUIRE_FORM_2100_1 = "fms_req_acquireForm_2100_1";
const std::string FORM_EVENT_RECV_ACQUIRE_FORM_2100_1 = "fms_recv_acquireForm_2100_1";
const std::string FORM_EVENT_REQ_ACQUIRE_FORM_2200 = "fms_req_acquireForm_2200";
const std::string FORM_EVENT_RECV_ACQUIRE_FORM_2200 = "fms_recv_acquireForm_2200";
const std::string FORM_EVENT_REQ_ACQUIRE_FORM_2300 = "fms_req_acquireForm_2300";
const std::string FORM_EVENT_RECV_ACQUIRE_FORM_2300 = "fms_recv_acquireForm_2300";
const std::string FORM_EVENT_REQ_ACQUIRE_FORM_2400 = "fms_req_acquireForm_2400";
const std::string FORM_EVENT_RECV_ACQUIRE_FORM_2400 = "fms_recv_acquireForm_2400";
const std::string FORM_EVENT_REQ_ACQUIRE_FORM_2400_1 = "fms_req_acquireForm_2400_1";
const std::string FORM_EVENT_RECV_ACQUIRE_FORM_2400_1 = "fms_recv_acquireForm_2400_1";
const std::string FORM_EVENT_REQ_ACQUIRE_FORM_2500 = "fms_req_acquireForm_2500";
const std::string FORM_EVENT_RECV_ACQUIRE_FORM_2500 = "fms_recv_acquireForm_2500";
const std::string FORM_EVENT_REQ_ACQUIRE_FORM_2600 = "fms_req_acquireForm_2600";
const std::string FORM_EVENT_RECV_ACQUIRE_FORM_2600 = "fms_recv_acquireForm_2600";
const std::string FORM_EVENT_REQ_ACQUIRE_FORM_2600_1 = "fms_req_acquireForm_2600_1";
const std::string FORM_EVENT_RECV_ACQUIRE_FORM_2600_1 = "fms_recv_acquireForm_2600_1";
const std::string FORM_EVENT_REQ_ACQUIRE_FORM_2700 = "fms_req_acquireForm_2700";
const std::string FORM_EVENT_RECV_ACQUIRE_FORM_2700 = "fms_recv_acquireForm_2700";
const std::string FORM_EVENT_REQ_ACQUIRE_FORM_2800 = "fms_req_acquireForm_2800";
const std::string FORM_EVENT_RECV_ACQUIRE_FORM_2800 = "fms_recv_acquireForm_2800";
const std::string FORM_EVENT_REQ_ACQUIRE_FORM_2900 = "fms_req_acquireForm_2900";
const std::string FORM_EVENT_RECV_ACQUIRE_FORM_2900 = "fms_recv_acquireForm_2900";
const std::string FORM_EVENT_REQ_ACQUIRE_FORM_2900_1 = "fms_req_acquireForm_2900_1";
const std::string FORM_EVENT_RECV_ACQUIRE_FORM_2900_1 = "fms_recv_acquireForm_2900_1";
const std::string FORM_EVENT_REQ_ACQUIRE_FORM_3000 = "fms_req_acquireForm_3000";
const std::string FORM_EVENT_RECV_ACQUIRE_FORM_3000 = "fms_recv_acquireForm_3000";
const std::string FORM_EVENT_REQ_ACQUIRE_FORM_3100 = "fms_req_acquireForm_3100";
const std::string FORM_EVENT_RECV_ACQUIRE_FORM_3100 = "fms_recv_acquireForm_3100";
const std::string FORM_EVENT_REQ_ACQUIRE_FORM_3200 = "fms_req_acquireForm_3200";
const std::string FORM_EVENT_RECV_ACQUIRE_FORM_3200 = "fms_recv_acquireForm_3200";
const std::string FORM_EVENT_REQ_ACQUIRE_FORM_3300 = "fms_req_acquireForm_3300";
const std::string FORM_EVENT_RECV_ACQUIRE_FORM_3300 = "fms_recv_acquireForm_3300";
const std::string FORM_EVENT_REQ_ACQUIRE_FORM_BATCH = "fms_req_acquireForm_batch";
const std::string FORM_EVENT_RECV_ACQUIRE_FORM_BATCH = "fms_recv_acquireForm_batch";
const std::string FORM_EVENT_REQ_ACQUIRE_FORM_BATCH_B = "fms_req_acquireForm_batch_b";
const std::string FORM_EVENT_RECV_ACQUIRE_FORM_BATCH_B = "fms_recv_acquireForm_batch_b";
const std::string FORM_EVENT_REQ_ACQUIRE_TEMP_FORM_BATCH = "fms_req_acquireFormTemp_batch";
const std::string FORM_EVENT_RECV_ACQUIRE_TEMP_FORM_BATCH = "fms_recv_acquireFormTemp_batch";
const std::string FORM_EVENT_REQ_CLEAR_FORM_BATCH = "fms_req_clearForm_batch";
const std::string FORM_EVENT_RECV_CLEAR_FORM_BATCH = "fms_recv_clearForm_batch";


// Delete Form
const std::string FORM_EVENT_REQ_DELETE_FORM_0100 = "fms_req_deleteForm_0100";
const std::string FORM_EVENT_RECV_DELETE_FORM_0100 = "fms_recv_deleteForm_0100";
const std::string FORM_EVENT_REQ_DELETE_FORM_0200 = "fms_req_deleteForm_0200";
const std::string FORM_EVENT_RECV_DELETE_FORM_0200 = "fms_recv_deleteForm_0200";
const std::string FORM_EVENT_REQ_DELETE_FORM_0300 = "fms_req_deleteForm_0300";
const std::string FORM_EVENT_RECV_DELETE_FORM_0300 = "fms_recv_deleteForm_0300";
const std::string FORM_EVENT_REQ_DELETE_FORM_0400 = "fms_req_deleteForm_0400";
const std::string FORM_EVENT_RECV_DELETE_FORM_0400 = "fms_recv_deleteForm_0400";
const std::string FORM_EVENT_REQ_DELETE_FORM_0500 = "fms_req_deleteForm_0500";
const std::string FORM_EVENT_RECV_DELETE_FORM_0500 = "fms_recv_deleteForm_0500";
const std::string FORM_EVENT_REQ_DELETE_FORM_0600 = "fms_req_deleteForm_0600";
const std::string FORM_EVENT_RECV_DELETE_FORM_0600 = "fms_recv_deleteForm_0600";
const std::string FORM_EVENT_REQ_DELETE_FORM_0700 = "fms_req_deleteForm_0700";
const std::string FORM_EVENT_RECV_DELETE_FORM_0700 = "fms_recv_deleteForm_0700";
const std::string FORM_EVENT_REQ_DELETE_FORM_0800 = "fms_req_deleteForm_0800";
const std::string FORM_EVENT_RECV_DELETE_FORM_0800 = "fms_recv_deleteForm_0800";
const std::string FORM_EVENT_REQ_DELETE_FORM_0900 = "fms_req_deleteForm_0900";
const std::string FORM_EVENT_RECV_DELETE_FORM_0900 = "fms_recv_deleteForm_0900";
const std::string FORM_EVENT_REQ_DELETE_FORM_1000 = "fms_req_deleteForm_1000";
const std::string FORM_EVENT_RECV_DELETE_FORM_1000 = "fms_recv_deleteForm_1000";
const std::string FORM_EVENT_REQ_DELETE_FORM_1100 = "fms_req_deleteForm_1100";
const std::string FORM_EVENT_RECV_DELETE_FORM_1100 = "fms_recv_deleteForm_1100";
const std::string FORM_EVENT_REQ_DELETE_FORM_1200 = "fms_req_deleteForm_1200";
const std::string FORM_EVENT_RECV_DELETE_FORM_1200 = "fms_recv_deleteForm_1200";
const std::string FORM_EVENT_REQ_DELETE_FORM_1201 = "fms_req_deleteForm_1201";
const std::string FORM_EVENT_RECV_DELETE_FORM_1201 = "fms_recv_deleteForm_1201";
const std::string FORM_EVENT_REQ_DELETE_FORM_1400 = "fms_req_deleteForm_1400";
const std::string FORM_EVENT_RECV_DELETE_FORM_1400 = "fms_recv_deleteForm_1400";
const std::string FORM_EVENT_REQ_DELETE_FORM_1401 = "fms_req_deleteForm_1401";
const std::string FORM_EVENT_RECV_DELETE_FORM_1401 = "fms_recv_deleteForm_1401";
const std::string FORM_EVENT_REQ_DELETE_FORM_1500 = "fms_req_deleteForm_1500";
const std::string FORM_EVENT_RECV_DELETE_FORM_1500 = "fms_recv_deleteForm_1500";
const std::string FORM_EVENT_REQ_DELETE_FORM_1501 = "fms_req_deleteForm_1501";
const std::string FORM_EVENT_RECV_DELETE_FORM_1501 = "fms_recv_deleteForm_1501";
const std::string FORM_EVENT_REQ_DELETE_FORM_1502 = "fms_req_deleteForm_1502";
const std::string FORM_EVENT_RECV_DELETE_FORM_1502 = "fms_recv_deleteForm_1502";
const std::string FORM_EVENT_REQ_DELETE_FORM_1600 = "fms_req_deleteForm_1600";
const std::string FORM_EVENT_RECV_DELETE_FORM_1600 = "fms_recv_deleteForm_1600";
const std::string FORM_EVENT_REQ_DELETE_FORM_1601 = "fms_req_deleteForm_1601";
const std::string FORM_EVENT_RECV_DELETE_FORM_1601 = "fms_recv_deleteForm_1601";
const std::string FORM_EVENT_REQ_DELETE_FORM_1602 = "fms_req_deleteForm_1602";
const std::string FORM_EVENT_RECV_DELETE_FORM_1602 = "fms_recv_deleteForm_1602";
const std::string FORM_EVENT_REQ_DELETE_FORM_1700 = "fms_req_deleteForm_1700";
const std::string FORM_EVENT_RECV_DELETE_FORM_1700 = "fms_recv_deleteForm_1700";
const std::string FORM_EVENT_REQ_DELETE_FORM_1701 = "fms_req_deleteForm_1701";
const std::string FORM_EVENT_RECV_DELETE_FORM_1701 = "fms_recv_deleteForm_1701";
const std::string FORM_EVENT_REQ_DELETE_FORM_1702 = "fms_req_deleteForm_1702";
const std::string FORM_EVENT_RECV_DELETE_FORM_1702 = "fms_recv_deleteForm_1702";
// OnDelete
const std::string COMMON_EVENT_ON_DELETE = "usual.event.ondelete";
typedef enum {
    FORM_EVENT_TRIGGER_RESULT_NG = 0,
    FORM_EVENT_TRIGGER_RESULT_OK = 1,
} FORM_EVENT_TRIGGER_RESULT;

// Release Form
const std::string FORM_EVENT_REQ_RELEASE_FORM_0100 = "fms_req_releaseForm_0100";
const std::string FORM_EVENT_RECV_RELEASE_FORM_0100 = "fms_recv_releaseForm_0100";
const std::string FORM_EVENT_REQ_RELEASE_FORM_0200 = "fms_req_releaseForm_0200";
const std::string FORM_EVENT_RECV_RELEASE_FORM_0200 = "fms_recv_releaseForm_0200";
const std::string FORM_EVENT_REQ_RELEASE_FORM_0300 = "fms_req_releaseForm_0300";
const std::string FORM_EVENT_RECV_RELEASE_FORM_0300 = "fms_recv_releaseForm_0300";
const std::string FORM_EVENT_REQ_RELEASE_FORM_0400 = "fms_req_releaseForm_0400";
const std::string FORM_EVENT_RECV_RELEASE_FORM_0400 = "fms_recv_releaseForm_0400";
const std::string FORM_EVENT_REQ_RELEASE_FORM_0500 = "fms_req_releaseForm_0500";
const std::string FORM_EVENT_RECV_RELEASE_FORM_0500 = "fms_recv_releaseForm_0500";
const std::string FORM_EVENT_REQ_RELEASE_FORM_0600 = "fms_req_releaseForm_0600";
const std::string FORM_EVENT_RECV_RELEASE_FORM_0600 = "fms_recv_releaseForm_0600";
const std::string FORM_EVENT_REQ_RELEASE_FORM_0700 = "fms_req_releaseForm_0700";
const std::string FORM_EVENT_RECV_RELEASE_FORM_0700 = "fms_recv_releaseForm_0700";
const std::string FORM_EVENT_REQ_RELEASE_FORM_0800 = "fms_req_releaseForm_0800";
const std::string FORM_EVENT_RECV_RELEASE_FORM_0800 = "fms_recv_releaseForm_0800";
const std::string FORM_EVENT_REQ_RELEASE_FORM_0900 = "fms_req_releaseForm_0900";
const std::string FORM_EVENT_RECV_RELEASE_FORM_0900 = "fms_recv_releaseForm_0900";
const std::string FORM_EVENT_REQ_RELEASE_FORM_1000 = "fms_req_releaseForm_1000";
const std::string FORM_EVENT_RECV_RELEASE_FORM_1000 = "fms_recv_releaseForm_1000";
const std::string FORM_EVENT_REQ_RELEASE_FORM_1100 = "fms_req_releaseForm_1100";
const std::string FORM_EVENT_RECV_RELEASE_FORM_1100 = "fms_recv_releaseForm_1100";
const std::string FORM_EVENT_REQ_RELEASE_FORM_1200 = "fms_req_releaseForm_1200";
const std::string FORM_EVENT_RECV_RELEASE_FORM_1200 = "fms_recv_releaseForm_1200";
const std::string FORM_EVENT_REQ_RELEASE_FORM_1300 = "fms_req_releaseForm_1300";
const std::string FORM_EVENT_RECV_RELEASE_FORM_1300 = "fms_recv_releaseForm_1300";
const std::string FORM_EVENT_REQ_RELEASE_FORM_1400 = "fms_req_releaseForm_1400";
const std::string FORM_EVENT_RECV_RELEASE_FORM_1400 = "fms_recv_releaseForm_1400";
const std::string FORM_EVENT_REQ_RELEASE_FORM_COMMON_DEL = "fms_req_releaseForm_common_del";
const std::string FORM_EVENT_RECV_RELEASE_FORM_COMMON_DEL = "fms_recv_releaseForm_common_del";

// Stress Test
const std::string FORM_EVENT_REQ_STRESS_TEST_0100 = "fms_req_stressTest_0100";
const std::string FORM_EVENT_RECV_STRESS_TEST_0100 = "fms_recv_stressTest_0100";
const std::string FORM_EVENT_REQ_STRESS_TEST_0100_01 = "fms_req_stressTest_0100_01";
const std::string FORM_EVENT_RECV_STRESS_TEST_0100_01 = "fms_recv_stressTest_0100_01";
const std::string FORM_EVENT_REQ_STRESS_TEST_0200 = "fms_req_stressTest_0200";
const std::string FORM_EVENT_RECV_STRESS_TEST_0200 = "fms_recv_stressTest_0200";
const std::string FORM_EVENT_REQ_STRESS_TEST_0200_01 = "fms_req_stressTest_0200_01";
const std::string FORM_EVENT_RECV_STRESS_TEST_0200_01 = "fms_recv_stressTest_0200_01";
const std::string FORM_EVENT_REQ_STRESS_TEST_0200_02 = "fms_req_stressTest_0200_02";
const std::string FORM_EVENT_RECV_STRESS_TEST_0200_02 = "fms_recv_stressTest_0200_02";
const std::string FORM_EVENT_REQ_STRESS_TEST_0200_03 = "fms_req_stressTest_0200_03";
const std::string FORM_EVENT_RECV_STRESS_TEST_0200_03 = "fms_recv_stressTest_0200_03";
const std::string FORM_EVENT_REQ_STRESS_TEST_0300 = "fms_req_stressTest_0300";
const std::string FORM_EVENT_RECV_STRESS_TEST_0300 = "fms_recv_stressTest_0300";
const std::string FORM_EVENT_REQ_STRESS_TEST_0300_01 = "fms_req_stressTest_0300_01";
const std::string FORM_EVENT_RECV_STRESS_TEST_0300_01 = "fms_recv_stressTest_0300_01";
const std::string FORM_EVENT_REQ_STRESS_TEST_0300_02 = "fms_req_stressTest_0300_02";
const std::string FORM_EVENT_RECV_STRESS_TEST_0300_02 = "fms_recv_stressTest_0300_02";
const std::string FORM_EVENT_REQ_STRESS_TEST_0400 = "fms_req_stressTest_0400";
const std::string FORM_EVENT_RECV_STRESS_TEST_0400 = "fms_recv_stressTest_0400";
const std::string FORM_EVENT_REQ_STRESS_TEST_0500 = "fms_req_stressTest_0500";
const std::string FORM_EVENT_RECV_STRESS_TEST_0500 = "fms_recv_stressTest_0500";
const std::string FORM_EVENT_REQ_STRESS_TEST_1100 = "fms_req_stressTest_1100";
const std::string FORM_EVENT_RECV_STRESS_TEST_1100 = "fms_recv_stressTest_1100";
const std::string FORM_EVENT_REQ_STRESS_TEST_1100_01 = "fms_req_stressTest_1100_01";
const std::string FORM_EVENT_RECV_STRESS_TEST_1100_01 = "fms_recv_stressTest_1100_01";
const std::string FORM_EVENT_REQ_STRESS_TEST_1100_02 = "fms_req_stressTest_1100_02";
const std::string FORM_EVENT_RECV_STRESS_TEST_1100_02 = "fms_recv_stressTest_1100_02";
const std::string FORM_EVENT_REQ_STRESS_TEST_1100_03 = "fms_req_stressTest_1100_03";
const std::string FORM_EVENT_RECV_STRESS_TEST_1100_03 = "fms_recv_stressTest_1100_03";
const std::string FORM_EVENT_REQ_STRESS_TEST_1300 = "fms_req_stressTest_1300";
const std::string FORM_EVENT_RECV_STRESS_TEST_1300 = "fms_recv_stressTest_1300";
const std::string FORM_EVENT_REQ_STRESS_TEST_1300_01 = "fms_req_stressTest_1300_01";
const std::string FORM_EVENT_RECV_STRESS_TEST_1300_01 = "fms_recv_stressTest_1300_01";
const std::string FORM_EVENT_REQ_STRESS_TEST_1300_02 = "fms_req_stressTest_1300_02";
const std::string FORM_EVENT_RECV_STRESS_TEST_1300_02 = "fms_recv_stressTest_1300_02";
const std::string FORM_EVENT_REQ_STRESS_TEST_1300_03 = "fms_req_stressTest_1300_03";
const std::string FORM_EVENT_RECV_STRESS_TEST_1300_03 = "fms_recv_stressTest_1300_03";
const std::string FORM_EVENT_REQ_STRESS_TEST_1700 = "fms_req_stressTest_1700";
const std::string FORM_EVENT_RECV_STRESS_TEST_1700 = "fms_recv_stressTest_1700";
const std::string FORM_EVENT_REQ_STRESS_TEST_1700_01 = "fms_req_stressTest_1700_01";
const std::string FORM_EVENT_RECV_STRESS_TEST_1700_01 = "fms_recv_stressTest_1700_01";
const std::string FORM_EVENT_REQ_STRESS_TEST_1700_02 = "fms_req_stressTest_1700_02";
const std::string FORM_EVENT_RECV_STRESS_TEST_1700_02 = "fms_recv_stressTest_1700_02";
const std::string FORM_EVENT_REQ_STRESS_TEST_1800 = "fms_req_stressTest_1800";
const std::string FORM_EVENT_RECV_STRESS_TEST_1800 = "fms_recv_stressTest_1800";

// CommonA(acquire one normal form)
const std::string FORM_EVENT_REQ_ONE_NORMAL_FORM = "fms_req_one_normal_form";
const std::string FORM_EVENT_RECV_ONE_NORMAL_FORM = "fms_recv_one_normal_form";
const std::string FORM_EVENT_REQ_ONE_NORMAL_FORM_DEL = "fms_req_one_normal_form_del";
const std::string FORM_EVENT_RECV_ONE_NORMAL_FORM_DEL = "fms_recv_one_normal_form_del";

// CommonB(acquire one normal form)
const std::string FORM_EVENT_REQ_ONE_NORMAL_FORM_B = "fms_req_one_normal_form_b";
const std::string FORM_EVENT_RECV_ONE_NORMAL_FORM_B = "fms_recv_one_normal_form_b";
const std::string FORM_EVENT_REQ_ONE_NORMAL_FORM_B_DEL = "fms_req_one_normal_form_b_del";
const std::string FORM_EVENT_RECV_ONE_NORMAL_FORM_B_DEL = "fms_recv_one_normal_form_b_del";

// Performance Test
const std::string FORM_EVENT_REQ_PERFORMANCE_TEST_0100 = "fms_req_performanceTest_0100";
const std::string FORM_EVENT_RECV_PERFORMANCE_TEST_0100 = "fms_recv_performanceTest_0100";
const std::string FORM_EVENT_REQ_PERFORMANCE_TEST_0200 = "fms_req_performanceTest_0200";
const std::string FORM_EVENT_RECV_PERFORMANCE_TEST_0200 = "fms_recv_performanceTest_0200";
const std::string FORM_EVENT_REQ_PERFORMANCE_TEST_0300 = "fms_req_performanceTest_0300";
const std::string FORM_EVENT_RECV_PERFORMANCE_TEST_0300 = "fms_recv_performanceTest_0300";
const std::string FORM_EVENT_REQ_PERFORMANCE_TEST_0400 = "fms_req_performanceTest_0400";
const std::string FORM_EVENT_RECV_PERFORMANCE_TEST_0400 = "fms_recv_performanceTest_0400";
const std::string FORM_EVENT_REQ_PERFORMANCE_TEST_0500 = "fms_req_performanceTest_0500";
const std::string FORM_EVENT_RECV_PERFORMANCE_TEST_0500 = "fms_recv_performanceTest_0500";
const std::string FORM_EVENT_REQ_PERFORMANCE_TEST_0600 = "fms_req_performanceTest_0600";
const std::string FORM_EVENT_RECV_PERFORMANCE_TEST_0600 = "fms_recv_performanceTest_0600";
const std::string FORM_EVENT_REQ_PERFORMANCE_TEST_0700 = "fms_req_performanceTest_0700";
const std::string FORM_EVENT_RECV_PERFORMANCE_TEST_0700 = "fms_recv_performanceTest_0700";
const std::string FORM_EVENT_REQ_PERFORMANCE_TEST_1300 = "fms_req_performanceTest_1300";
const std::string FORM_EVENT_RECV_PERFORMANCE_TEST_1300 = "fms_recv_performanceTest_1300";
const std::string FORM_EVENT_REQ_PERFORMANCE_TEST_1400 = "fms_req_performanceTest_1400";
const std::string FORM_EVENT_RECV_PERFORMANCE_TEST_1400 = "fms_recv_performanceTest_1400";
const std::string FORM_EVENT_REQ_PERFORMANCE_TEST_1500 = "fms_req_performanceTest_1500";
const std::string FORM_EVENT_RECV_PERFORMANCE_TEST_1500 = "fms_recv_performanceTest_1500";
const std::string FORM_EVENT_REQ_PERFORMANCE_TEST_1600 = "fms_req_performanceTest_1600";
const std::string FORM_EVENT_RECV_PERFORMANCE_TEST_1600 = "fms_recv_performanceTest_1600";

// Self-Starting Test
const std::string FORM_EVENT_REQ_SELF_STARTING_TEST_0100 = "fms_req_selfStartingTest_0100";
const std::string FORM_EVENT_RECV_SELF_STARTING_TEST_0100 = "fms_recv_selfStartingTest_0100";
const std::string FORM_EVENT_REQ_SELF_STARTING_TEST_0200 = "fms_req_selfStartingTest_0200";
const std::string FORM_EVENT_RECV_SELF_STARTING_TEST_0200 = "fms_recv_selfStartingTest_0200";
const std::string FORM_EVENT_REQ_SELF_STARTING_TEST_0300 = "fms_req_selfStartingTest_0300";
const std::string FORM_EVENT_RECV_SELF_STARTING_TEST_0300 = "fms_recv_selfStartingTest_0300";

// common event data code
const int EVENT_CODE_100 = 100;
const int EVENT_CODE_101 = 101;
const int EVENT_CODE_200 = 200;
const int EVENT_CODE_300 = 300;
const int EVENT_CODE_400 = 400;
const int EVENT_CODE_500 = 500;
const int EVENT_CODE_600 = 600;
const int EVENT_CODE_700 = 700;
const int EVENT_CODE_800 = 800;
const int EVENT_CODE_900 = 900;
const int EVENT_CODE_999 = 999;
const int EVENT_CODE_1000 = 1000;
const int EVENT_CODE_1100 = 1100;
const int EVENT_CODE_1101 = 1101;
const int EVENT_CODE_1200 = 1200;
const int EVENT_CODE_1201 = 1201;
const int EVENT_CODE_1234 = 1234;
const int EVENT_CODE_1300 = 1300;
const int EVENT_CODE_1400 = 1400;
const int EVENT_CODE_1401 = 1401;
const int EVENT_CODE_1500 = 1500;
const int EVENT_CODE_1510 = 1510;
const int EVENT_CODE_1511 = 1511;
const int EVENT_CODE_1600 = 1600;
const int EVENT_CODE_1601 = 1601;
const int EVENT_CODE_1610 = 1610;
const int EVENT_CODE_1611 = 1611;
const int EVENT_CODE_1700 = 1700;
const int EVENT_CODE_1701 = 1701;
const int EVENT_CODE_1702 = 1702;
const int EVENT_CODE_1800 = 1800;
const int EVENT_CODE_1801 = 1801;
const int EVENT_CODE_1810 = 1810;
const int EVENT_CODE_1811 = 1811;
const int EVENT_CODE_1820 = 1820;
const int EVENT_CODE_1821 = 1821;
const int EVENT_CODE_1830 = 1830;
const int EVENT_CODE_1831 = 1831;
const int EVENT_CODE_1900 = 1900;
const int EVENT_CODE_1901 = 1901;
const int EVENT_CODE_2000 = 2000;
const int EVENT_CODE_2100 = 2100;
const int EVENT_CODE_2200 = 2200;
const int EVENT_CODE_2300 = 2300;
const int EVENT_CODE_2400 = 2400;
const int EVENT_CODE_2401 = 2401;
const int EVENT_CODE_2410 = 2410;
const int EVENT_CODE_2500 = 2500;
const int EVENT_CODE_2600 = 2600;
const int EVENT_CODE_2610 = 2610;
const int EVENT_CODE_2611 = 2611;
const int EVENT_CODE_2700 = 2700;
const int EVENT_CODE_2800 = 2800;
const int EVENT_CODE_2900 = 2900;
const int EVENT_CODE_2901 = 2901;
const int EVENT_CODE_2910 = 2910;
const int EVENT_CODE_2911 = 2911;
const int EVENT_CODE_3000 = 3000;
const int EVENT_CODE_3100 = 3100;
const int EVENT_CODE_3101 = 3101;
const int EVENT_CODE_3200 = 3200;
const int EVENT_CODE_3300 = 3300;
const int EVENT_CODE_3301 = 3301;

const int EVENT_CODE_BATCH = 9000;
const int EVENT_CODE_BATCH_B = 9010;
const int EVENT_CODE_TEMP_BATCH = 9020;
const int EVENT_CODE_CLEAR_BATCH = 9030;

const int EVENT_CODE_TEMP = 9900;
const int EVENT_CODE_TEMP_1 = 9901;
}  // namespace AppExecFwk
}  // namespace OHOS
#endif  // FORM_ST_COMMON_INFO_H