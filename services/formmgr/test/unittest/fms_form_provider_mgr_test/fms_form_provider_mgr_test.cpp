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
#include "form_ams_helper.h"
#include "form_bms_helper.h"
#define private public
#include "form_data_mgr.h"
#include "form_db_cache.h"
#include "form_refresh_limiter.h"
#include "form_host_interface.h"
#include "form_mgr.h"
#undef private
#include "form_mgr_service.h"
#include "form_provider_mgr.h"
#include "if_system_ability_manager.h"
#include "inner_bundle_info.h"
#include "ipc_skeleton.h"
#include "iservice_registry.h"

#include "mock_ability_manager.h"
#include "mock_bundle_manager.h"
#include "mock_form_host_client.h"
#include "permission/permission_kit.h"
#include "permission/permission.h"
#include "running_process_info.h"
#include "system_ability_definition.h"

using namespace testing::ext;
using namespace OHOS;
using namespace OHOS::AppExecFwk;
using namespace OHOS::Security;

namespace {
const std::string PERMISSION_NAME_REQUIRE_FORM = "ohos.permission.REQUIRE_FORM";
const std::string PARAM_PROVIDER_PACKAGE_NAME = "com.form.provider.app.test.abiliy";
const std::string FORM_PROVIDER_BUNDLE_NAME = "com.form.provider.service";
const std::string PARAM_PROVIDER_MODULE_NAME = "com.form.provider.app.test.abiliy";
const std::string FORM_PROVIDER_ABILITY_NAME = "com.form.provider.app.test.abiliy";
const std::string PARAM_FORM_NAME = "com.form.name.test";

const std::string FORM_JS_COMPOMENT_NAME = "jsComponentName";
const std::string FORM_PROVIDER_MODULE_SOURCE_DIR = "";

const std::string FORM_HOST_BUNDLE_NAME = "com.form.host.app";

const std::string DEVICE_ID = "ohos-phone1";
const std::string DEF_LABEL1 = "PermissionFormRequireGrant";

class FmsFormProviderMgrTest : public testing::Test {
public:
    static void SetUpTestCase();
    static void TearDownTestCase();
    void SetUp();
    void TearDown();

protected:
    sptr<MockFormHostClient> token_;
    std::shared_ptr<FormMgrService> formyMgrServ_ = DelayedSingleton<FormMgrService>::GetInstance();
    
    sptr<BundleMgrService> mockBundleMgr_;
    sptr<MockAbilityMgrService> mockAbilityMgrServ_;
};

void FmsFormProviderMgrTest::SetUpTestCase()
{}

void FmsFormProviderMgrTest::TearDownTestCase()
{}

void FmsFormProviderMgrTest::SetUp()
{
    // APP_LOGI("fms_form_mgr_client_test_001 setup");
    formyMgrServ_->OnStart();

    mockBundleMgr_ = new (std::nothrow) BundleMgrService();
    EXPECT_TRUE(mockBundleMgr_ != nullptr);
    FormBmsHelper::GetInstance().SetBundleManager(mockBundleMgr_);
    
    mockAbilityMgrServ_ = new (std::nothrow) MockAbilityMgrService();
    FormAmsHelper::GetInstance().SetAbilityManager(mockAbilityMgrServ_);
 
    // APP_LOGI("fms_form_mgr_client_test_001 FormMgrService started");
    token_ = new (std::nothrow) MockFormHostClient();

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
    Permission::PermissionKit::AddUserGrantedReqPermissions(FORM_PROVIDER_BUNDLE_NAME, {PERMISSION_NAME_REQUIRE_FORM},
    0);
    Permission::PermissionKit::GrantUserGrantedPermission(FORM_PROVIDER_BUNDLE_NAME, PERMISSION_NAME_REQUIRE_FORM, 0);
}

void FmsFormProviderMgrTest::TearDown()
{}

/*
 * Feature: FmsFormProviderMgr
 * Function: FormMgr
 * SubFunction: AcquireForm Function
 * FunctionPoints: FormMgr AcquireForm interface
 * EnvConditions: Mobile that can run ohos test framework
 * CaseDescription: Verify if  AcquireForm works with invalid formid.
 */

HWTEST_F(FmsFormProviderMgrTest, AcquireForm_001, TestSize.Level0)
{
    GTEST_LOG_(INFO) << "fms_form_mgr_provider_test_001 start";
    int64_t formId = 0x114514aa00000000;
    FormProviderInfo formProviderInfo;
    EXPECT_EQ(ERR_FORM_INVALID_PARAM, FormProviderMgr::GetInstance().AcquireForm(-114514L, formProviderInfo));
    int callingUid {0};
    FormItemInfo record;
    record.SetFormId(formId);
    FormRecord realFormRecord = FormDataMgr::GetInstance().AllotFormRecord(record, callingUid);
    FormItemInfo info;
    FormDataMgr::GetInstance().AllotFormHostRecord(info, token_, formId, callingUid);
    GTEST_LOG_(INFO) << "fms_form_mgr_provider_test_001 end";
}

/*
 * Feature: FmsFormProviderMgr
 * Function: FormMgr
 * SubFunction: AcquireForm Function
 * FunctionPoints: FormMgr AcquireForm interface
 * EnvConditions: Mobile that can run ohos test framework
 * CaseDescription: Verify if  AcquireForm works without formrecord.
 */

HWTEST_F(FmsFormProviderMgrTest, AcquireForm_002, TestSize.Level0)
{
    GTEST_LOG_(INFO) << "fms_form_mgr_provider_test_002 start";
    int64_t formId = 0x11451aaa00000000;
    FormProviderInfo formProviderInfo;
    EXPECT_EQ(ERR_APPEXECFWK_FORM_INFO_NOT_EXIST,FormProviderMgr::GetInstance().AcquireForm(formId,formProviderInfo));
    int callingUid {0};
    FormItemInfo record;
    record.SetFormId(formId);
    FormRecord realFormRecord = FormDataMgr::GetInstance().AllotFormRecord(record, callingUid);
    FormItemInfo info;
    FormDataMgr::GetInstance().AllotFormHostRecord(info, token_, formId, callingUid);
    GTEST_LOG_(INFO) << "fms_form_mgr_provider_test_002 end";
}


/*
 * Feature: FmsFormProviderMgr
 * Function: FormMgr
 * SubFunction: AcquireForm Function
 * FunctionPoints: FormMgr AcquireForm interface
 * EnvConditions: Mobile that can run ohos test framework
 * CaseDescription: Verify if  AcquireForm works without form host record.
 */

HWTEST_F(FmsFormProviderMgrTest, AcquireForm_003, TestSize.Level0)
{
    GTEST_LOG_(INFO) << "fms_form_mgr_provider_test_003 start";
    int64_t formId = 0x1145aaaa00000000;
    FormProviderInfo formProviderInfo;
    int callingUid {0};
    FormItemInfo record;
    record.SetFormId(formId);
    FormRecord realFormRecord = FormDataMgr::GetInstance().AllotFormRecord(record, callingUid);
    EXPECT_EQ(ERR_APPEXECFWK_FORM_HOST_INFO_NOT_EXIST,
    FormProviderMgr::GetInstance().AcquireForm(formId,formProviderInfo));
    FormItemInfo info;
    FormDataMgr::GetInstance().AllotFormHostRecord(info, token_, formId, callingUid);
    GTEST_LOG_(INFO) << "fms_form_mgr_provider_test_003 end";
}


/*
 * Feature: FmsFormProviderMgr
 * Function: FormMgr
 * SubFunction: RefreshForm Function
 * FunctionPoints: FormMgr RefreshForm interface
 * EnvConditions: Mobile that can run ohos test framework
 * CaseDescription: Verify if  RefreshForm works without form host record.
 */

HWTEST_F(FmsFormProviderMgrTest, RefreshForm_001, TestSize.Level0)
{
    GTEST_LOG_(INFO) << "fms_form_mgr_provider_test_004 start";
    int64_t formId = 0x1145aaaa00001200;
    Want want;
    int callingUid {0};
    EXPECT_EQ(ERR_APPEXECFWK_FORM_INFO_NOT_EXIST, FormProviderMgr::GetInstance().RefreshForm(formId, want));
    FormItemInfo record;
    record.SetFormId(formId);
    record.SetModuleName(PARAM_FORM_NAME);
    record.SetAbilityName(FORM_PROVIDER_ABILITY_NAME);
    FormRecord realFormRecord = FormDataMgr::GetInstance().AllotFormRecord(record, callingUid);
    FormItemInfo info;
    FormDataMgr::GetInstance().AllotFormHostRecord(info, token_, formId, callingUid);
    GTEST_LOG_(INFO) << "fms_form_mgr_provider_test_004 end";
}

/*
 * Feature: FmsFormProviderMgr
 * Function: FormMgr
 * SubFunction: RefreshForm Function
 * FunctionPoints: FormMgr RefreshForm interface
 * EnvConditions: Mobile that can run ohos test framework
 * CaseDescription: Verify if  RefreshForm works without form host record.
 */

HWTEST_F(FmsFormProviderMgrTest, RefreshForm_002, TestSize.Level0)
{
    GTEST_LOG_(INFO) << "fms_form_mgr_provider_test_005 start";
    int64_t formId = 0x114514aa00000000;
    Want want;
    want.SetParam(Constants::KEY_IS_TIMER, true);
    int callingUid {0};
    FormItemInfo record;
    record.SetFormId(formId);
    record.SetModuleName(PARAM_FORM_NAME);
    record.SetAbilityName(FORM_PROVIDER_ABILITY_NAME);
    FormRecord realFormRecord = FormDataMgr::GetInstance().AllotFormRecord(record, callingUid);
    FormItemInfo info;
    FormDataMgr::GetInstance().AllotFormHostRecord(info, token_, formId, callingUid);
    EXPECT_EQ(ERR_APPEXECFWK_FORM_SUPPLIER_DEL_FAIL, FormProviderMgr::GetInstance().RefreshForm(formId, want));
    GTEST_LOG_(INFO) << "fms_form_mgr_provider_test_005 end";
}
}