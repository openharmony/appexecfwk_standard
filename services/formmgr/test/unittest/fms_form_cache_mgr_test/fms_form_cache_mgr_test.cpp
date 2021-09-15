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
#define private public
#include "form_cache_mgr.h"
#undef private

using namespace testing::ext;
using namespace OHOS;
using namespace OHOS::AppExecFwk;

const int64_t PARAM_FORM_ID_FIRST = 1001;
const int64_t PARAM_FORM_ID_SECOND = 1002;

namespace {
class FmsFormCacheMgrTest : public testing::Test {
public:
    
    static void SetUpTestCase();
    static void TearDownTestCase();
    void SetUp();
    void TearDown();  

protected:
    FormCacheMgr formCacheMgr_;
};


void FmsFormCacheMgrTest::SetUpTestCase()
{}

void FmsFormCacheMgrTest::TearDownTestCase()
{}

void FmsFormCacheMgrTest::SetUp()
{}

void FmsFormCacheMgrTest::TearDown()
{}

/*
 * Feature: FormCacheMgr
 * Function: GetData
 * FunctionPoints: FormCacheMgr GetData interface
 * EnvConditions: Mobile that can run ohos test framework
 * CaseDescription: invoke GetData works by input key.
 */
HWTEST_F(FmsFormCacheMgrTest, FmsFormCacheMgrTest_001, TestSize.Level0)
{
    APP_LOGI("fms_form_cache_mgr_test_001 start");

    std::string dataResult = "";
    formCacheMgr_.cacheData_[PARAM_FORM_ID_FIRST]= "{'a':'1','b':'2'}";
    EXPECT_TRUE(formCacheMgr_.GetData(PARAM_FORM_ID_FIRST, dataResult));
    EXPECT_EQ("{'a':'1','b':'2'}", dataResult);

    GTEST_LOG_(INFO) << "fms_form_cache_mgr_test_001 end";
}

/*
 * Feature: FormCacheMgr
 * Function: GetData
 * FunctionPoints: FormCacheMgr GetData interface
 * EnvConditions: Mobile that can run ohos test framework
 * CaseDescription: can not get data by input key.
 */

HWTEST_F(FmsFormCacheMgrTest, FmsFormCacheMgrTest_002, TestSize.Level0)
{
    APP_LOGI("fms_form_cache_mgr_test_002 start");

    std::string dataResult = "";
    formCacheMgr_.cacheData_[PARAM_FORM_ID_FIRST]= "{'a':'1','b':'2'}";
    EXPECT_FALSE(formCacheMgr_.GetData(PARAM_FORM_ID_SECOND, dataResult));

    GTEST_LOG_(INFO) << "fms_form_cache_mgr_test_002 end";
}

/*
 * Feature: FormCacheMgr
 * Function: AddData
 * FunctionPoints: FormCacheMgr AddData interface
 * EnvConditions: Mobile that can run ohos test framework
 * CaseDescription: add data by input param.
 */
HWTEST_F(FmsFormCacheMgrTest, FmsFormCacheMgrTest_003, TestSize.Level0)
{
    APP_LOGI("fms_form_cache_mgr_test_003 start");

    std::string dataResult = "{'a':'1','b':'2'}";
    EXPECT_TRUE(formCacheMgr_.AddData(PARAM_FORM_ID_FIRST, dataResult));
    EXPECT_EQ(formCacheMgr_.cacheData_[PARAM_FORM_ID_FIRST], dataResult);

    GTEST_LOG_(INFO) << "fms_form_cache_mgr_test_003 end";
}

/*
 * Feature: FormCacheMgr
 * Function: AddData
 * FunctionPoints: FormCacheMgr AddData interface
 * EnvConditions: Mobile that can run ohos test framework
 * CaseDescription: cache contains data and add the new data by input param.
 */
HWTEST_F(FmsFormCacheMgrTest, FmsFormCacheMgrTest_004, TestSize.Level0)
{
    APP_LOGI("fms_form_cache_mgr_test_004 start");

    std::string dataResult1 = "{'a':'1','b':'2'}";
    std::string dataResult2 = "{'a':'2','b':'2'}";
    formCacheMgr_.AddData(PARAM_FORM_ID_FIRST, dataResult1);
    EXPECT_TRUE(formCacheMgr_.AddData(PARAM_FORM_ID_SECOND, dataResult2));
    EXPECT_EQ(formCacheMgr_.cacheData_[PARAM_FORM_ID_SECOND],dataResult2);

    GTEST_LOG_(INFO) << "fms_form_cache_mgr_test_004 end";
}

/*
 * Feature: FormCacheMgr
 * Function: AddData
 * FunctionPoints: FormCacheMgr AddData interface
 * EnvConditions: Mobile that can run ohos test framework
 * CaseDescription: add data but key conflict
 */
HWTEST_F(FmsFormCacheMgrTest, FmsFormCacheMgrTest_010, TestSize.Level0)
{
    APP_LOGI("fms_form_cache_mgr_test_010 start");

    std::string dataResult1 = "{'a':'1','b':'2'}";
    std::string dataResult2 = "{'a':'2','b':'2'}";
    formCacheMgr_.AddData(PARAM_FORM_ID_FIRST, dataResult1);
    EXPECT_FALSE(formCacheMgr_.AddData(PARAM_FORM_ID_FIRST, dataResult2));

    GTEST_LOG_(INFO) << "fms_form_cache_mgr_test_010 end";
}

/*
 * Feature: FormCacheMgr
 * Function: DeleteData
 * FunctionPoints: FormCacheMgr DeleteData interface
 * EnvConditions: Mobile that can run ohos test framework
 * CaseDescription: delete data by input key
 */
HWTEST_F(FmsFormCacheMgrTest, FmsFormCacheMgrTest_005, TestSize.Level0)
{
    APP_LOGI("fms_form_cache_mgr_test_005 start");

    std::string dataResult = "";
    std::string dataResult1 = "{'a':'1','b':'2'}";
    std::string dataResult2 = "{'a':'2','b':'2'}";
    formCacheMgr_.AddData(PARAM_FORM_ID_FIRST, dataResult1);
    formCacheMgr_.AddData(PARAM_FORM_ID_SECOND, dataResult2);
    EXPECT_TRUE(formCacheMgr_.DeleteData(PARAM_FORM_ID_SECOND));
    EXPECT_FALSE(formCacheMgr_.GetData(PARAM_FORM_ID_SECOND, dataResult));
    EXPECT_EQ(formCacheMgr_.cacheData_[PARAM_FORM_ID_FIRST], dataResult1);

    GTEST_LOG_(INFO) << "fms_form_cache_mgr_test_005 end";
}

/*
 * Feature: FormCacheMgr
 * Function: DeleteData
 * FunctionPoints: FormCacheMgr DeleteData interface
 * EnvConditions: Mobile that can run ohos test framework
 * CaseDescription: delete data but not exsit
 */
HWTEST_F(FmsFormCacheMgrTest, FmsFormCacheMgrTest_006, TestSize.Level0)
{
    APP_LOGI("fms_form_cache_mgr_test_006 start");

    EXPECT_TRUE(formCacheMgr_.DeleteData(PARAM_FORM_ID_SECOND));

    GTEST_LOG_(INFO) << "fms_form_cache_mgr_test_006 end";
}

/*
 * Feature: FormCacheMgr
 * Function: UpdateData
 * FunctionPoints: FormCacheMgr UpdateData interface
 * EnvConditions: Mobile that can run ohos test framework
 * CaseDescription: update cache's data by input param 
 */
HWTEST_F(FmsFormCacheMgrTest, FmsFormCacheMgrTest_007, TestSize.Level0)
{
    APP_LOGI("fms_form_cache_mgr_test_007 start");

    std::string dataResult = "";
    std::string dataResult1 = "{'a':'1','b':'2'}";
    std::string dataResult2 = "{'a':'2','b':'2'}";
    formCacheMgr_.AddData(PARAM_FORM_ID_FIRST, dataResult1);
    EXPECT_TRUE(formCacheMgr_.UpdateData(PARAM_FORM_ID_FIRST, dataResult2));
    EXPECT_EQ(formCacheMgr_.cacheData_[PARAM_FORM_ID_FIRST], dataResult2);

    GTEST_LOG_(INFO) << "fms_form_cache_mgr_test_007 end";
}

/*
 * Feature: FormCacheMgr
 * Function: UpdateData
 * FunctionPoints: FormCacheMgr UpdateData interface
 * EnvConditions: Mobile that can run ohos test framework
 * CaseDescription: update cache's data but not exsit key 
 */
HWTEST_F(FmsFormCacheMgrTest, FmsFormCacheMgrTest_008, TestSize.Level0)
{
    APP_LOGI("fms_form_cache_mgr_test_008 start");

    std::string dataResult1 = "{'a':'1','b':'2'}";
    EXPECT_FALSE(formCacheMgr_.UpdateData(PARAM_FORM_ID_FIRST, dataResult1));

    GTEST_LOG_(INFO) << "fms_form_cache_mgr_test_008 end";
}
}