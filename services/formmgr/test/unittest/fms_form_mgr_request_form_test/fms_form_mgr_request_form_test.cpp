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

#include <gtest/gtest.h>
#include "app_log_wrapper.h"
#include "appexecfwk_errors.h"
#include "form_data_mgr.h"
#define private public
#include "form_mgr.h"
#undef private
#include "form_mgr_service.h"
#include "form_ams_helper.h"
#include "form_bms_helper.h"
#include "if_system_ability_manager.h"
#include "ipc_skeleton.h"
#include "iservice_registry.h"
#include "permission/permission_kit.h"
#include "permission/permission.h"
#include "running_process_info.h"
#include "mock_bundle_manager.h"
#include "mock_ability_manager.h"
#include "system_ability_definition.h"
#include "../../mock/include/mock_form_token.h"
#include "../../mock/include/mock_form_host_client.h"

using namespace testing::ext;
using namespace OHOS;
using namespace OHOS::AppExecFwk;
using namespace OHOS::Security;

namespace {
const std::string PERMISSION_NAME_REQUIRE_FORM = "ohos.permission.REQUIRE_FORM";
const std::string FORM_PROVIDER_BUNDLE_NAME = "com.form.provider.service";
const std::string FORM_PROVIDER_ABILITY_NAME = "com.form.provider.app.test";
const std::string FORM_HOST_BUNDLE_NAME = "com.form.host.app";
const std::string DEVICE_ID = "ohos-phone1";

const std::string DEF_LABEL1 = "PermissionFormRequireGrant";

class FmsFormMgrRequestFormTest : public testing::Test {
public:
    static void SetUpTestCase();
    static void TearDownTestCase();
    void SetUp();
    void TearDown();

protected:
    sptr<MockAbilityMgrService> mockAbilityMgrServ_;
    sptr<BundleMgrService> mockBundleMgr_;
    sptr<MockFormHostClient> token_;
    std::shared_ptr<FormMgrService> formyMgrServ_ = DelayedSingleton<FormMgrService>::GetInstance();
};

void FmsFormMgrRequestFormTest::SetUpTestCase()
{}

void FmsFormMgrRequestFormTest::TearDownTestCase()
{}

void FmsFormMgrRequestFormTest::SetUp()
{
    formyMgrServ_->OnStart();

    token_ = new (std::nothrow) MockFormHostClient();

    mockBundleMgr_ = new (std::nothrow) BundleMgrService();
    ASSERT_TRUE(mockBundleMgr_ != nullptr);
    FormBmsHelper::GetInstance().SetBundleManager(mockBundleMgr_);

    mockAbilityMgrServ_ = new (std::nothrow) MockAbilityMgrService();
    FormAmsHelper::GetInstance().SetAbilityManager(mockAbilityMgrServ_);
    // Permission install
    std::vector<Permission::PermissionDef> permList;
    Permission::PermissionDef permDef;
    permDef.permissionName = PERMISSION_NAME_REQUIRE_FORM;
    permDef.bundleName = FORM_PROVIDER_BUNDLE_NAME;
    permDef.grantMode = Permission::GrantMode::USER_GRANT;
    permDef.availableScope = Permission::AvailableScope::AVAILABLE_SCOPE_ALL;
    permDef.label = DEF_LABEL1;
    permDef.labelId = 1;
    permDef.description = DEF_LABEL1;
    permDef.descriptionId = 1;
    permList.emplace_back(permDef);
    Permission::PermissionKit::AddDefPermissions(permList);
    Permission::PermissionKit::AddUserGrantedReqPermissions(FORM_PROVIDER_BUNDLE_NAME,
        {PERMISSION_NAME_REQUIRE_FORM}, 0);
    Permission::PermissionKit::GrantUserGrantedPermission(FORM_PROVIDER_BUNDLE_NAME, PERMISSION_NAME_REQUIRE_FORM, 0);
}

void FmsFormMgrRequestFormTest::TearDown()
{}

/*
 * Feature: FormMgrService
 * Function: FormMgrClient
 * SubFunction: RequestForm Function
 * FunctionPoints: FormMgrClient RequestForm interface
 * EnvConditions: Mobile that can run ohos test framework
 * CaseDescription: Nomal case: Verify if FormMgrClient invoke RequestForm works.
 */
HWTEST_F(FmsFormMgrRequestFormTest, RequestForm_001, TestSize.Level0)
{
    APP_LOGI("fms_form_mgr_request_test_001 start");

    int64_t formId {0X0000FFFF00000000};
    int callingUid {0};
    // Create cache
    FormItemInfo record1;
    record1.SetFormId(formId);
    record1.SetProviderBundleName(FORM_HOST_BUNDLE_NAME);
    record1.SetAbilityName(FORM_PROVIDER_ABILITY_NAME);
    record1.SetTemporaryFlag(false);
    FormRecord retFormRec = FormDataMgr::GetInstance().AllotFormRecord(record1, callingUid);
    // Set database info.
    retFormRec.formUserUids.clear();
    // Set form host record
    FormItemInfo info;
    FormDataMgr::GetInstance().AllotFormHostRecord(info, token_, formId, callingUid);
    Want want;
    EXPECT_EQ(ERR_OK, FormMgr::GetInstance().RequestForm(formId, token_, want));

    token_->Wait();
    FormDataMgr::GetInstance().DeleteFormRecord(formId);
    FormDataMgr::GetInstance().DeleteHostRecord(token_, formId);
    APP_LOGI("fms_form_mgr_request_test_001 end");
}

/*
 * Feature: FormMgrService
 * Function: FormMgrClient
 * SubFunction: RequestForm Function
 * FunctionPoints: FormMgrClient RequestForm interface
 * EnvConditions: Mobile that can run ohos test framework
 * CaseDescription: Abnomal case: Verify invalid parameter.
 */
HWTEST_F(FmsFormMgrRequestFormTest, RequestForm_003, TestSize.Level0)
{
    APP_LOGI("fms_form_mgr_request_test_003 start");

    int64_t formId {0X00AAAAFF00000000};
    FormItemInfo record;
    int callingUid {0};
    record.SetFormId(formId);
    record.SetProviderBundleName(FORM_PROVIDER_BUNDLE_NAME);
    record.SetAbilityName(FORM_PROVIDER_ABILITY_NAME);
    record.SetTemporaryFlag(false);
    FormRecord retFormRec = FormDataMgr::GetInstance().AllotFormRecord(record, callingUid);

    Want want;
    EXPECT_EQ(ERR_APPEXECFWK_FORM_INVALID_PARAM, FormMgr::GetInstance().RequestForm(formId, token_, want));

    token_->Wait();
    FormDataMgr::GetInstance().DeleteFormRecord(formId);
    FormDataMgr::GetInstance().DeleteHostRecord(token_, formId);
    APP_LOGI("fms_form_mgr_request_test_003 end");
}

/*
 * Feature: FormMgrService
 * Function: FormMgrClient
 * SubFunction: RequestForm Function
 * FunctionPoints: FormMgrClient RequestForm interface
 * EnvConditions: Mobile that can run ohos test framework
 * CaseDescription: Abnomal case: Verify form not self.
 */
HWTEST_F(FmsFormMgrRequestFormTest, RequestForm_004, TestSize.Level0)
{
    APP_LOGI("fms_form_mgr_request_test_004 start");
    int64_t formId {0X000ABCFF00000000};
    int64_t fakeFormId {0X0ABCDEFF00000000};
    FormItemInfo record;
    int callingUid {0};
    record.SetFormId(formId);
    record.SetProviderBundleName(FORM_PROVIDER_BUNDLE_NAME);
    record.SetAbilityName(FORM_PROVIDER_ABILITY_NAME);
    record.SetTemporaryFlag(false);
    FormRecord retFormRec = FormDataMgr::GetInstance().AllotFormRecord(record, callingUid);

    FormItemInfo itemInfo;
    FormDataMgr::GetInstance().AllotFormHostRecord(itemInfo, token_, fakeFormId, 0);

    Want want;
    EXPECT_EQ(ERR_APPEXECFWK_FORM_OPERATION_NOT_SELF, FormMgr::GetInstance().RequestForm(formId, token_, want));

    token_->Wait();
    FormDataMgr::GetInstance().DeleteFormRecord(formId);
    FormDataMgr::GetInstance().DeleteHostRecord(token_, formId);
    APP_LOGI("fms_form_mgr_request_test_004 end");
}
}