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

#include "application_context.h"
#include "event_runner.h"

namespace OHOS {
namespace AppExecFwk {
using namespace testing::ext;
using namespace OHOS;
using namespace OHOS::AppExecFwk;

class ApplicationContextForTaskDispatcherTest : public testing::Test {
public:
    ApplicationContextForTaskDispatcherTest()
    {}
    ~ApplicationContextForTaskDispatcherTest()
    {}
    static void SetUpTestCase(void);
    static void TearDownTestCase(void);
    void SetUp();
    void TearDown();
};

void ApplicationContextForTaskDispatcherTest::SetUpTestCase(void)
{}

void ApplicationContextForTaskDispatcherTest::TearDownTestCase(void)
{}

void ApplicationContextForTaskDispatcherTest::SetUp(void)
{}

void ApplicationContextForTaskDispatcherTest::TearDown(void)
{}

/**
 * @tc.number: AppExecFwk_ApplicationContext_CreateParallelTaskDispatcher_0100
 * @tc.name: CreateParallelTaskDispatcher
 * @tc.desc: Test whether attachbasecontext is called normally,
 *           and verify whether the return value of getbundlename is correct.
 */
HWTEST_F(ApplicationContextForTaskDispatcherTest, AppExecFwk_ApplicationContext_CreateParallelTaskDispatcher_0100,
    Function | MediumTest | Level1)
{
    GTEST_LOG_(INFO) << "AppExecFwk_ApplicationContext_CreateParallelTaskDispatcher_0100 start";
    std::shared_ptr<ApplicationContext> appcontext = std::make_shared<ApplicationContext>();
    std::string name("ParalleTest");
    TaskPriority high = TaskPriority::HIGH;
    EXPECT_TRUE((nullptr != appcontext->CreateParallelTaskDispatcher(name, high)));
    GTEST_LOG_(INFO) << "AppExecFwk_ApplicationContext_CreateParallelTaskDispatcher_0100 end";
}

/**
 * @tc.number: AppExecFwk_ApplicationContext_CreateSerialTaskDispatcher_0100
 * @tc.name: CreateSerialTaskDispatcher
 * @tc.desc: Test whether attachbasecontext is called normally,
 *           and verify whether the return value of getbundlename is correct.
 */
HWTEST_F(ApplicationContextForTaskDispatcherTest, AppExecFwk_ApplicationContext_CreateSerialTaskDispatcher_0100,
    Function | MediumTest | Level1)
{
    GTEST_LOG_(INFO) << "AppExecFwk_ApplicationContext_CreateSerialTaskDispatcher_0100 start";
    std::shared_ptr<ApplicationContext> appcontext = std::make_shared<ApplicationContext>();
    std::string name("ParalleTest");
    TaskPriority high = TaskPriority::HIGH;
    EXPECT_TRUE((nullptr != appcontext->CreateSerialTaskDispatcher(name, high)));
    GTEST_LOG_(INFO) << "AppExecFwk_ApplicationContext_CreateSerialTaskDispatcher_0100 end";
}

/**
 * @tc.number: AppExecFwk_ApplicationContext_GetGlobalTaskDispatcher_0100
 * @tc.name: GetGlobalTaskDispatcher
 * @tc.desc: Test whether attachbasecontext is called normally,
 *           and verify whether the return value of getbundlename is correct.
 */
HWTEST_F(ApplicationContextForTaskDispatcherTest, AppExecFwk_ApplicationContext_GetGlobalTaskDispatcher_0100,
    Function | MediumTest | Level1)
{
    GTEST_LOG_(INFO) << "AppExecFwk_ApplicationContext_GetGlobalTaskDispatcher_0100 start";
    std::shared_ptr<ApplicationContext> appcontext = std::make_shared<ApplicationContext>();
    TaskPriority high = TaskPriority::HIGH;
    EXPECT_TRUE((nullptr != appcontext->GetGlobalTaskDispatcher(high)));
    GTEST_LOG_(INFO) << "AppExecFwk_ApplicationContext_GetGlobalTaskDispatcher_0100 end";
}

}  // namespace AppExecFwk
}  // namespace OHOS