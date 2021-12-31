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
#include <singleton.h>

#include "ability_local_record.h"
#include "context_impl.h"
#include "context.h"
#include "iremote_object.h"

namespace OHOS {
namespace AppExecFwk {
using namespace testing::ext;
using namespace OHOS;
using namespace OHOS::AppExecFwk;

class ContextImplTest : public testing::Test {
public:
    ContextImplTest() : contextImpl_(nullptr)
    {}
    ~ContextImplTest()
    {}
    std::shared_ptr<AbilityRuntime::ContextImpl> contextImpl_ = nullptr;
    static void SetUpTestCase(void);
    static void TearDownTestCase(void);
    void SetUp();
    void TearDown();
};

void ContextImplTest::SetUpTestCase(void)
{}

void ContextImplTest::TearDownTestCase(void)
{}

void ContextImplTest::SetUp(void)
{
    contextImpl_ = std::make_shared<AbilityRuntime::ContextImpl>();
}

void ContextImplTest::TearDown(void)
{}

/**
 * @tc.number: AppExecFwk_ContextImpl_GetBundleName_001
 * @tc.name: GetBundleName
 * @tc.desc: Test whether GetBundleName is called normally.
 * @tc.type: FUNC
 * @tc.require: SR000GH1HL
 */
HWTEST_F(ContextImplTest, AppExecFwk_ContextImpl_GetBundleName_001, Function | MediumTest | Level1)
{
    GTEST_LOG_(INFO) << "AppExecFwk_ContextImpl_GetBundleName_001 start";
    std::string bundleName = contextImpl_->GetBundleName();
    EXPECT_STREQ(bundleName.c_str(), "");
    GTEST_LOG_(INFO) << "AppExecFwk_ContextImpl_GetBundleName_001 end";
}

/**
 * @tc.number: AppExecFwk_ContextImpl_GetBundleName_002
 * @tc.name: GetBundleName
 * @tc.desc: Test whether GetBundleName is called normally.
 * @tc.type: FUNC
 * @tc.require: SR000GH1HL
 */
HWTEST_F(ContextImplTest, AppExecFwk_ContextImpl_GetBundleName_002, Function | MediumTest | Level1)
{
    GTEST_LOG_(INFO) << "AppExecFwk_ContextImpl_GetBundleName_002 start";
    std::shared_ptr<AppExecFwk::ApplicationInfo> applicationInfo = std::make_shared<AppExecFwk::ApplicationInfo>();
    applicationInfo->bundleName = "com.test";
    contextImpl_->SetApplicationInfo(applicationInfo);
    std::string bundleName = contextImpl_->GetBundleName();
    EXPECT_STREQ(bundleName.c_str(), "com.test");
    GTEST_LOG_(INFO) << "AppExecFwk_ContextImpl_GetBundleName_002 end";
}

/**
 * @tc.number: AppExecFwk_ContextImpl_SetApplicationInfo_001
 * @tc.name: SetApplicationInfo
 * @tc.desc: Test whether SetApplicationInfo is called normally.
 * @tc.type: FUNC
 * @tc.require: SR000GH1HL
 */
HWTEST_F(ContextImplTest, AppExecFwk_ContextImpl_SetApplicationInfo_001, Function | MediumTest | Level1)
{
    GTEST_LOG_(INFO) << "AppExecFwk_ContextImpl_SetApplicationInfo_001 start";
    std::shared_ptr<AppExecFwk::ApplicationInfo> applicationInfo = std::make_shared<AppExecFwk::ApplicationInfo>();
    contextImpl_->SetApplicationInfo(applicationInfo);
    EXPECT_NE(contextImpl_->GetApplicationInfo(), nullptr);
    GTEST_LOG_(INFO) << "AppExecFwk_ContextImpl_SetApplicationInfo_001 end";
}

/**
 * @tc.number: AppExecFwk_ContextImpl_GetApplicationInfo_001
 * @tc.name: GetApplicationInfo
 * @tc.desc: Test whether GetApplicationInfo is called normally.
 * @tc.type: FUNC
 * @tc.require: SR000GH1HL
 */
HWTEST_F(ContextImplTest, AppExecFwk_ContextImpl_GetApplicationInfo_001, Function | MediumTest | Level1)
{
    GTEST_LOG_(INFO) << "AppExecFwk_ContextImpl_GetApplicationInfo_001 start";
    EXPECT_NE(contextImpl_->GetApplicationInfo(), nullptr);
    GTEST_LOG_(INFO) << "AppExecFwk_ContextImpl_GetApplicationInfo_001 end";
}

/**
 * @tc.number: AppExecFwk_ContextImpl_GetApplicationContext_001
 * @tc.name: GetApplicationContext
 * @tc.desc: Test whether GetApplicationContext is called normally.
 * @tc.type: FUNC
 * @tc.require: SR000GH1HL
 */
HWTEST_F(ContextImplTest, AppExecFwk_ContextImpl_GetApplicationContext_001, Function | MediumTest | Level1)
{
    GTEST_LOG_(INFO) << "AppExecFwk_ContextImpl_GetApplicationContext_001 start";
    EXPECT_EQ(contextImpl_->GetApplicationContext(), nullptr);
    GTEST_LOG_(INFO) << "AppExecFwk_ContextImpl_GetApplicationContext_001 end";
}

/**
 * @tc.number: AppExecFwk_ContextImpl_SetParentContext_001
 * @tc.name: SetParentContext
 * @tc.desc: Test whether SetParentContext is called normally.
 * @tc.type: FUNC
 * @tc.require: SR000GH1HL
 */
HWTEST_F(ContextImplTest, AppExecFwk_ContextImpl_SetParentContext_001, Function | MediumTest | Level1)
{
    GTEST_LOG_(INFO) << "AppExecFwk_ContextImpl_SetParentContext_001 start";
    std::shared_ptr<AbilityRuntime::ContextImpl> contextImpl_ = std::make_shared<AbilityRuntime::ContextImpl>();
    contextImpl_->SetParentContext(contextImpl_);
    EXPECT_NE(contextImpl_->GetApplicationContext(), nullptr);
    GTEST_LOG_(INFO) << "AppExecFwk_ContextImpl_SetParentContext_001 end";
}

/**
 * @tc.number: AppExecFwk_ContextImpl_GetHapModuleInfo_001
 * @tc.name: GetHapModuleInfo
 * @tc.desc: Test whether GetHapModuleInfo is called normally.
 * @tc.type: FUNC
 * @tc.require: SR000GH1HL
 */
HWTEST_F(ContextImplTest, AppExecFwk_ContextImpl_GetHapModuleInfo_001, Function | MediumTest | Level1)
{
    GTEST_LOG_(INFO) << "AppExecFwk_ContextImpl_GetHapModuleInfo_001 start";
    EXPECT_EQ(contextImpl_->GetHapModuleInfo(), nullptr);
    GTEST_LOG_(INFO) << "AppExecFwk_ContextImpl_GetHapModuleInfo_001 end";
}
}  // namespace AppExecFwk
}