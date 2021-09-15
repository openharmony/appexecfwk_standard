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
#include <thread>
#include <string>

#include "app_log_wrapper.h"
#include "appexecfwk_errors.h"
#define private public
#include "form_bms_helper.h"
#include "form_mgr_service.h"
#include "form_mgr_adapter.h"
#include "form_data_mgr.h"
#include "form_mgr.h"
#undef private
#include "form_constants.h"
#include "mock_form_host_client.h"
#include "mock_bundle_manager.h"
#include "permission/permission_def.h"
#include "permission/permission_kit.h"
#include "permission/permission.h"

using namespace testing::ext;
using namespace OHOS;
using namespace OHOS::AppExecFwk;

namespace {
const std::string FORM_HOST_BUNDLE_NAME = "com.form.provider.service";
const std::string FORM_PROVIDER_ABILITY_NAME = "com.form.provider.app.test.abiliy";
const std::string PERMISSION_NAME_REQUIRE_FORM = "ohos.permission.REQUIRE_FORM";
const std::string DEF_LABEL1 = "PermissionFormRequireGrant";

class FmsFormMgrNotifyInvisibleFormsTest : public testing::Test {
public:
    FmsFormMgrNotifyInvisibleFormsTest() : formMgrService_(nullptr)
    {}
    ~FmsFormMgrNotifyInvisibleFormsTest()
    {}
    static void SetUpTestCase(void);
    static void TearDownTestCase(void);
    void SetUp();
    void TearDown();
protected:
    sptr<OHOS::AppExecFwk::MockFormHostClient> token_;
    sptr<BundleMgrService> mockBundleMgr_;
    std::shared_ptr<FormMgrService> formMgrService_ = DelayedSingleton<FormMgrService>::GetInstance();
};
void FmsFormMgrNotifyInvisibleFormsTest::SetUpTestCase(void)
{}

void FmsFormMgrNotifyInvisibleFormsTest::TearDownTestCase(void)
{}

void FmsFormMgrNotifyInvisibleFormsTest::SetUp(void)
{
    formMgrService_ = std::make_shared<FormMgrService>();

    formMgrService_->OnStart();

    // mock BundleMgr
    mockBundleMgr_ = new (std::nothrow) BundleMgrService();
    ASSERT_TRUE(mockBundleMgr_ != nullptr);
    FormBmsHelper::GetInstance().SetBundleManager(mockBundleMgr_);    

    // token
    token_ = new (std::nothrow) OHOS::AppExecFwk::MockFormHostClient();

    // Permission install
    std::vector<OHOS::Security::Permission::PermissionDef> permList;
    OHOS::Security::Permission::PermissionDef permDef;
    permDef.permissionName = PERMISSION_NAME_REQUIRE_FORM;
    permDef.bundleName = FORM_HOST_BUNDLE_NAME;
    permDef.grantMode = OHOS::Security::Permission::GrantMode::USER_GRANT;
    permDef.availableScope = OHOS::Security::Permission::AvailableScope::AVAILABLE_SCOPE_ALL;
    permDef.label = DEF_LABEL1;
    permDef.labelId = 1;
    permDef.description = DEF_LABEL1;
    permDef.descriptionId = 1;
    permList.emplace_back(permDef);
    OHOS::Security::Permission::PermissionKit::AddDefPermissions(permList);
    OHOS::Security::Permission::PermissionKit::AddUserGrantedReqPermissions(FORM_HOST_BUNDLE_NAME, 
    {PERMISSION_NAME_REQUIRE_FORM}, 0);
    OHOS::Security::Permission::PermissionKit::GrantUserGrantedPermission(FORM_HOST_BUNDLE_NAME, 
    PERMISSION_NAME_REQUIRE_FORM, 0);
}

void FmsFormMgrNotifyInvisibleFormsTest::TearDown(void)
{}

/**
 * @tc.number: FmsFormMgrNotifyInvisibleFormsTest_NotifyInvisibleForms_001
 * @tc.name: NotifyInvisibleForms
 * @tc.desc: Verify that the return value is ERR_APPEXECFWK_FORM_PERMISSION_DENY.
 * @tc.info: permission denied.
 */
HWTEST_F(FmsFormMgrNotifyInvisibleFormsTest, FmsFormMgrNotifyInvisibleFormsTest_NotifyInvisibleForms_001, 
TestSize.Level0)
{
    GTEST_LOG_(INFO) << "FmsFormMgrNotifyInvisibleFormsTest_NotifyInvisibleForms_001 start";
    
    int64_t formId = 1;
    std::vector<int64_t> formIds;
    formIds.push_back(formId);

    // Remove Permission
    OHOS::Security::Permission::PermissionKit::RemoveDefPermissions(FORM_HOST_BUNDLE_NAME);
    OHOS::Security::Permission::PermissionKit::RemoveUserGrantedReqPermissions(FORM_HOST_BUNDLE_NAME, 0);
    OHOS::Security::Permission::PermissionKit::RemoveSystemGrantedReqPermissions(FORM_HOST_BUNDLE_NAME);

    EXPECT_EQ(ERR_APPEXECFWK_FORM_PERMISSION_DENY, FormMgr::GetInstance().NotifyWhetherVisibleForms( formIds, 
    token_, Constants::FORM_INVISIBLE ));
    GTEST_LOG_(INFO) << "FmsFormMgrNotifyInvisibleFormsTest_NotifyInvisibleForms_001 end";
}

/**
 * @tc.number: FmsFormMgrNotifyInvisibleFormsTest_NotifyInvisibleForms_002
 * @tc.name: NotifyInvisibleForms
 * @tc.desc: Verify that the return value is ERR_APPEXECFWK_FORM_INVALID_PARAM.
 * @tc.info: callerToken is nullptr.
 */
HWTEST_F(FmsFormMgrNotifyInvisibleFormsTest, FmsFormMgrNotifyInvisibleFormsTest_NotifyInvisibleForms_002, 
TestSize.Level0)
{
    GTEST_LOG_(INFO) << "FmsFormMgrNotifyInvisibleFormsTest_NotifyInvisibleForms_002 start";
    
    int64_t formId = 2;
    std::vector<int64_t> formIds;
    formIds.push_back(formId);

    // creat formRecords_
    FormItemInfo iteminfo;
    iteminfo.formId_ = formId;
    iteminfo.providerBundleName_ = FORM_HOST_BUNDLE_NAME;
    iteminfo.abilityName_ = FORM_PROVIDER_ABILITY_NAME;
    FormDataMgr::GetInstance().AllotFormRecord(iteminfo, 0);	

    // clear callerToken
    token_ = nullptr;

    EXPECT_EQ(ERR_APPEXECFWK_FORM_INVALID_PARAM, FormMgr::GetInstance().NotifyWhetherVisibleForms( formIds, 
    token_, Constants::FORM_INVISIBLE ));
    GTEST_LOG_(INFO) << "FmsFormMgrNotifyInvisibleFormsTest_NotifyInvisibleForms_002 end";
}

/**
 * @tc.number: FmsFormMgrNotifyInvisibleFormsTest_NotifyInvisibleForms_003
 * @tc.name: NotifyInvisibleForms
 * @tc.desc: Verify that the return value is ERR_OK.
 * @tc.info: form record is not found.
 */
HWTEST_F(FmsFormMgrNotifyInvisibleFormsTest, FmsFormMgrNotifyInvisibleFormsTest_NotifyInvisibleForms_003, 
TestSize.Level0)
{
    GTEST_LOG_(INFO) << "FmsFormMgrNotifyInvisibleFormsTest_NotifyInvisibleForms_003 start";
    
    int64_t formId = 3;
    std::vector<int64_t> formIds;
    formIds.push_back(formId);

    EXPECT_EQ(ERR_OK, FormMgr::GetInstance().NotifyWhetherVisibleForms( formIds, token_, Constants::FORM_INVISIBLE ));
    GTEST_LOG_(INFO) << "FmsFormMgrNotifyInvisibleFormsTest_NotifyInvisibleForms_003 end";
}

/**
 * @tc.number: FmsFormMgrNotifyInvisibleFormsTest_NotifyInvisibleForms_004
 * @tc.name: NotifyInvisibleForms
 * @tc.desc: Verify that the return value is ERR_OK.
 * @tc.info: host form record is not found.
 */
HWTEST_F(FmsFormMgrNotifyInvisibleFormsTest, FmsFormMgrNotifyInvisibleFormsTest_NotifyInvisibleForms_004, 
TestSize.Level0)
{
    GTEST_LOG_(INFO) << "FmsFormMgrNotifyInvisibleFormsTest_NotifyInvisibleForms_004 start";
    
    int64_t formId = 4;
    std::vector<int64_t> formIds;
    formIds.push_back(formId);

    // creat formRecords_
    FormItemInfo iteminfo;
    iteminfo.formId_ = formId;
    iteminfo.providerBundleName_ = FORM_HOST_BUNDLE_NAME;
    iteminfo.abilityName_ = FORM_PROVIDER_ABILITY_NAME;
    FormDataMgr::GetInstance().AllotFormRecord(iteminfo, 0);	

    EXPECT_EQ(ERR_OK, FormMgr::GetInstance().NotifyWhetherVisibleForms( formIds, token_, Constants::FORM_INVISIBLE ));
    GTEST_LOG_(INFO) << "FmsFormMgrNotifyInvisibleFormsTest_NotifyInvisibleForms_004 end";
}

/**
 * @tc.number: FmsFormMgrNotifyInvisibleFormsTest_NotifyInvisibleForms_005
 * @tc.name: NotifyInvisibleForms
 * @tc.desc: Verify that the return value is ERR_OK.
 * @tc.info: host form record is found, but formVisibleNotify is false.
 */
HWTEST_F(FmsFormMgrNotifyInvisibleFormsTest, FmsFormMgrNotifyInvisibleFormsTest_NotifyInvisibleForms_005, 
TestSize.Level0)
{
    GTEST_LOG_(INFO) << "FmsFormMgrNotifyInvisibleFormsTest_NotifyInvisibleForms_005 start";
    
    int64_t formId = 5;
    std::vector<int64_t> formIds;
    formIds.push_back(formId);

    // creat formRecords_
    FormItemInfo iteminfo;
    iteminfo.formId_ = formId;
    iteminfo.providerBundleName_ = FORM_HOST_BUNDLE_NAME;
    iteminfo.abilityName_ = FORM_PROVIDER_ABILITY_NAME;
    iteminfo.formVisibleNotify_ = false;
    FormDataMgr::GetInstance().AllotFormRecord(iteminfo, 0);	

    // creat clientRecords_
    FormDataMgr::GetInstance().AllotFormHostRecord(iteminfo, token_, formId, 0);

    EXPECT_EQ(ERR_OK, FormMgr::GetInstance().NotifyWhetherVisibleForms( formIds, token_, Constants::FORM_INVISIBLE ));
    GTEST_LOG_(INFO) << "FmsFormMgrNotifyInvisibleFormsTest_NotifyInvisibleForms_005 end";
}

/**
 * @tc.number: FmsFormMgrNotifyInvisibleFormsTest_NotifyInvisibleForms_006
 * @tc.name: NotifyInvisibleForms
 * @tc.desc: Verify that the return value is ERR_OK.
 * @tc.info: host form record is found, formVisibleNotify is true, it is a SystemApp.
 */
HWTEST_F(FmsFormMgrNotifyInvisibleFormsTest, FmsFormMgrNotifyInvisibleFormsTest_NotifyInvisibleForms_006, 
TestSize.Level0)
{
    GTEST_LOG_(INFO) << "FmsFormMgrNotifyInvisibleFormsTest_NotifyInvisibleForms_006 start";
    
    int64_t formId = 6;
    std::vector<int64_t> formIds;
    formIds.push_back(formId);

    // creat formRecords_
    FormItemInfo iteminfo;
    iteminfo.formId_ = formId;
    iteminfo.providerBundleName_ = FORM_HOST_BUNDLE_NAME;
    iteminfo.abilityName_ = FORM_PROVIDER_ABILITY_NAME;
    iteminfo.formVisibleNotify_ = true;
    FormDataMgr::GetInstance().AllotFormRecord(iteminfo, 0);	

    // creat clientRecords_
    FormDataMgr::GetInstance().AllotFormHostRecord(iteminfo, token_, formId, 0);

    EXPECT_EQ(ERR_OK, FormMgr::GetInstance().NotifyWhetherVisibleForms( formIds, token_, Constants::FORM_INVISIBLE ));
    GTEST_LOG_(INFO) << "FmsFormMgrNotifyInvisibleFormsTest_NotifyInvisibleForms_006 end";
}

/**
 * @tc.number: FmsFormMgrNotifyInvisibleFormsTest_NotifyInvisibleForms_007
 * @tc.name: NotifyInvisibleForms
 * @tc.desc: Verify that the return value is ERR_OK.
 * @tc.info: it is not a SystemApp.
 */
HWTEST_F(FmsFormMgrNotifyInvisibleFormsTest, FmsFormMgrNotifyInvisibleFormsTest_NotifyInvisibleForms_007, 
TestSize.Level0)
{
    GTEST_LOG_(INFO) << "FmsFormMgrNotifyInvisibleFormsTest_NotifyInvisibleForms_007 start";
    
    int64_t formId = 7;
    std::string mockBundleName = "com.form.host.app600";
    std::vector<int64_t> formIds;
    formIds.push_back(formId);

    // creat formRecords_
    FormItemInfo iteminfo;
    iteminfo.formId_ = formId;
    iteminfo.providerBundleName_ = mockBundleName;
    iteminfo.abilityName_ = FORM_PROVIDER_ABILITY_NAME;
    iteminfo.formVisibleNotify_ = true;
    FormDataMgr::GetInstance().AllotFormRecord(iteminfo, 0);	

    // creat clientRecords_
    FormDataMgr::GetInstance().AllotFormHostRecord(iteminfo, token_, formId, 0);

    // Permission install (mockBundleName)
    std::vector<OHOS::Security::Permission::PermissionDef> permList;
    OHOS::Security::Permission::PermissionDef permDef;
    permDef.permissionName = PERMISSION_NAME_REQUIRE_FORM;
    permDef.bundleName = mockBundleName;
    permDef.grantMode = OHOS::Security::Permission::GrantMode::USER_GRANT;
    permDef.availableScope = OHOS::Security::Permission::AvailableScope::AVAILABLE_SCOPE_ALL;
    permDef.label = DEF_LABEL1;
    permDef.labelId = 1;
    permDef.description = DEF_LABEL1;
    permDef.descriptionId = 1;
    permList.emplace_back(permDef);
    OHOS::Security::Permission::PermissionKit::AddDefPermissions(permList);
    OHOS::Security::Permission::PermissionKit::AddUserGrantedReqPermissions(mockBundleName, 
    {PERMISSION_NAME_REQUIRE_FORM}, 0);
    OHOS::Security::Permission::PermissionKit::GrantUserGrantedPermission(mockBundleName, 
    PERMISSION_NAME_REQUIRE_FORM, 0);

    EXPECT_EQ(ERR_OK, FormMgr::GetInstance().NotifyWhetherVisibleForms( formIds, token_, Constants::FORM_INVISIBLE ));
    GTEST_LOG_(INFO) << "FmsFormMgrNotifyInvisibleFormsTest_NotifyInvisibleForms_007 end";
}
}
