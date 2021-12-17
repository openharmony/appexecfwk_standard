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
#include "context_deal.h"


namespace OHOS {
namespace AppExecFwk {
using namespace testing::ext;
using namespace OHOS;
using namespace OHOS::AppExecFwk;

class ContextDealForTaskDispatcherTest : public testing::Test {
public:
    ContextDealForTaskDispatcherTest()
    {}
    ~ContextDealForTaskDispatcherTest()
    {}
    static void SetUpTestCase(void);
    static void TearDownTestCase(void);
    void SetUp();
    void TearDown();
};

void ContextDealForTaskDispatcherTest::SetUpTestCase(void)
{}

void ContextDealForTaskDispatcherTest::TearDownTestCase(void)
{}

void ContextDealForTaskDispatcherTest::SetUp(void)
{}

void ContextDealForTaskDispatcherTest::TearDown(void)
{}

/**
 * @tc.number: AppExecFwk_ContextDeal_GetUITaskDispatcher_0100
 * @tc.name: GetUITaskDispatcher
 * @tc.desc: Test whether attachbasecontext is called normally,
 *           and verify whether the return value of getbundlename is correct.
 */
HWTEST_F(ContextDealForTaskDispatcherTest, AppExecFwk_ContextDeal_GetUITaskDispatcher_0100, Function | MediumTest | Level1)
{
    GTEST_LOG_(INFO) << "AppExecFwk_ContextDeal_GetUITaskDispatcher_0100 start";
    std::shared_ptr<EventRunner> runner = EventRunner::Create(false);
    std::shared_ptr<ContextDeal> contextdeal = std::make_shared<ContextDeal>();
    contextdeal->SetRunner(runner);
    EXPECT_TRUE((nullptr != contextdeal->GetUITaskDispatcher()));
    GTEST_LOG_(INFO) << "AppExecFwk_ContextDeal_GetUITaskDispatcher_0100 end";
}
/**
 * @tc.number: AppExecFwk_ContextDeal_GetUITaskDispatcher_0200
 * @tc.name: GetUITaskDispatcher
 * @tc.desc: Test the attachbasecontext call to verify that the return value of gethapmoduleinfo is correct.
 */
HWTEST_F(ContextDealForTaskDispatcherTest, AppExecFwk_ContextDeal_GetUITaskDispatcher_0200, Function | MediumTest | Level3)
{
    GTEST_LOG_(INFO) << "AppExecFwk_ContextDeal_GetUITaskDispatcher_0200 start";
    std::shared_ptr<ContextDeal> contextdeal = std::make_shared<ContextDeal>();
    EXPECT_TRUE((nullptr == contextdeal->GetUITaskDispatcher()));
    GTEST_LOG_(INFO) << "AppExecFwk_ContextDeal_GetUITaskDispatcher_0200 end";
}

/**
 * @tc.number: AppExecFwk_ContextDeal_GetMainTaskDispatcher_0100
 * @tc.name: GetMainTaskDispatcher
 * @tc.desc: Test whether attachbasecontext is called normally,
 *           and verify whether the return value of getbundlename is correct.
 */
HWTEST_F(ContextDealForTaskDispatcherTest, AppExecFwk_ContextDeal_GetMainTaskDispatcher_0100, Function | MediumTest | Level1)
{
    GTEST_LOG_(INFO) << "AppExecFwk_ContextDeal_GetMainTaskDispatcher_0100 start";
    std::shared_ptr<EventRunner> runner = EventRunner::Create(false);
    std::shared_ptr<ContextDeal> contextdeal = std::make_shared<ContextDeal>();
    contextdeal->SetRunner(runner);
    EXPECT_TRUE((nullptr != contextdeal->GetMainTaskDispatcher()));
    GTEST_LOG_(INFO) << "AppExecFwk_ContextDeal_GetMainTaskDispatcher_0100 end";
}
/**
 * @tc.number: AppExecFwk_ContextDeal_GetMainTaskDispatcher_0200
 * @tc.name: GetMainTaskDispatcher
 * @tc.desc: Test the attachbasecontext call to verify that the return value of gethapmoduleinfo is correct.
 */
HWTEST_F(ContextDealForTaskDispatcherTest, AppExecFwk_ContextDeal_GetMainTaskDispatcher_0200, Function | MediumTest | Level3)
{
    GTEST_LOG_(INFO) << "AppExecFwk_ContextDeal_GetMainTaskDispatcher_0200 start";
    std::shared_ptr<ContextDeal> contextdeal = std::make_shared<ContextDeal>();
    EXPECT_TRUE((nullptr == contextdeal->GetMainTaskDispatcher()));
    GTEST_LOG_(INFO) << "AppExecFwk_ContextDeal_GetMainTaskDispatcher_0200 end";
}

/**
 * @tc.number: AppExecFwk_ContextDeal_CreateParallelTaskDispatcher_0100
 * @tc.name: CreateParallelTaskDispatcher
 * @tc.desc: Test whether attachbasecontext is called normally,
 *           and verify whether the return value of getbundlename is correct.
 */
HWTEST_F(ContextDealForTaskDispatcherTest, AppExecFwk_ContextDeal_CreateParallelTaskDispatcher_0100, Function | MediumTest | Level1)
{
    GTEST_LOG_(INFO) << "AppExecFwk_ContextDeal_CreateParallelTaskDispatcher_0100 start";
    std::shared_ptr<ApplicationContext> appcontext = std::make_shared<ApplicationContext>();
    std::shared_ptr<ContextDeal> contextdeal = std::make_shared<ContextDeal>();
    contextdeal->SetApplicationContext(appcontext);
    std::string name("ParalleTest");
    TaskPriority high = TaskPriority::HIGH;
    EXPECT_TRUE((nullptr != contextdeal->CreateParallelTaskDispatcher(name, high)));
    GTEST_LOG_(INFO) << "AppExecFwk_ContextDeal_CreateParallelTaskDispatcher_0100 end";
}
/**
 * @tc.number: AppExecFwk_ContextDeal_CreateParallelTaskDispatcher_0200
 * @tc.name: CreateParallelTaskDispatcher
 * @tc.desc: Test the attachbasecontext call to verify that the return value of gethapmoduleinfo is correct.
 */
HWTEST_F(ContextDealForTaskDispatcherTest, AppExecFwk_ContextDeal_CreateParallelTaskDispatcher_0200, Function | MediumTest | Level3)
{
    GTEST_LOG_(INFO) << "AppExecFwk_ContextDeal_CreateParallelTaskDispatcher_0200 start";
    std::shared_ptr<ContextDeal> contextdeal = std::make_shared<ContextDeal>();
    std::string name("ParalleTest");
    TaskPriority high = TaskPriority::HIGH;
    EXPECT_TRUE((nullptr == contextdeal->CreateParallelTaskDispatcher(name, high)));
    GTEST_LOG_(INFO) << "AppExecFwk_ContextDeal_CreateParallelTaskDispatcher_0200 end";
}

/**
 * @tc.number: AppExecFwk_ContextDeal_CreateSerialTaskDispatcher_0100
 * @tc.name: CreateSerialTaskDispatcher
 * @tc.desc: Test whether attachbasecontext is called normally,
 *           and verify whether the return value of getbundlename is correct.
 */
HWTEST_F(ContextDealForTaskDispatcherTest, AppExecFwk_ContextDeal_CreateSerialTaskDispatcher_0100, Function | MediumTest | Level1)
{
    GTEST_LOG_(INFO) << "AppExecFwk_ContextDeal_CreateSerialTaskDispatcher_0100 start";
    std::shared_ptr<ApplicationContext> appcontext = std::make_shared<ApplicationContext>();
    std::shared_ptr<ContextDeal> contextdeal = std::make_shared<ContextDeal>();
    contextdeal->SetApplicationContext(appcontext);
    std::string name("ParalleTest");
    TaskPriority high = TaskPriority::HIGH;
    EXPECT_TRUE((nullptr != contextdeal->CreateSerialTaskDispatcher(name, high)));
    GTEST_LOG_(INFO) << "AppExecFwk_ContextDeal_CreateSerialTaskDispatcher_0100 end";
}
/**
 * @tc.number: AppExecFwk_ContextDeal_CreateSerialTaskDispatcher_0200
 * @tc.name: CreateSerialTaskDispatcher
 * @tc.desc: Test the attachbasecontext call to verify that the return value of gethapmoduleinfo is correct.
 */
HWTEST_F(ContextDealForTaskDispatcherTest, AppExecFwk_ContextDeal_CreateSerialTaskDispatcher_0200, Function | MediumTest | Level3)
{
    GTEST_LOG_(INFO) << "AppExecFwk_ContextDeal_CreateSerialTaskDispatcher_0200 start";
    std::shared_ptr<ContextDeal> contextdeal = std::make_shared<ContextDeal>();
    std::string name("Serial");
    TaskPriority high = TaskPriority::HIGH;
    EXPECT_TRUE((nullptr == contextdeal->CreateSerialTaskDispatcher(name, high)));
    GTEST_LOG_(INFO) << "AppExecFwk_ContextDeal_CreateSerialTaskDispatcher_0200 end";
}

/**
 * @tc.number: AppExecFwk_ContextDeal_GetGlobalTaskDispatcher_0100
 * @tc.name: GetGlobalTaskDispatcher
 * @tc.desc: Test whether attachbasecontext is called normally,
 *           and verify whether the return value of getbundlename is correct.
 */
HWTEST_F(ContextDealForTaskDispatcherTest, AppExecFwk_ContextDeal_GetGlobalTaskDispatcher_0100, Function | MediumTest | Level1)
{
    GTEST_LOG_(INFO) << "AppExecFwk_ContextDeal_GetGlobalTaskDispatcher_0100 start";
    std::shared_ptr<ApplicationContext> appcontext = std::make_shared<ApplicationContext>();
    std::shared_ptr<ContextDeal> contextdeal = std::make_shared<ContextDeal>();
    contextdeal->SetApplicationContext(appcontext);
    TaskPriority high = TaskPriority::HIGH;
    EXPECT_TRUE((nullptr != contextdeal->GetGlobalTaskDispatcher(high)));
    GTEST_LOG_(INFO) << "AppExecFwk_ContextDeal_GetGlobalTaskDispatcher_0100 end";
}
/**
 * @tc.number: AppExecFwk_ContextDeal_GetGlobalTaskDispatcher_0200
 * @tc.name: GetGlobalTaskDispatcher
 * @tc.desc: Test the attachbasecontext call to verify that the return value of gethapmoduleinfo is correct.
 */
HWTEST_F(ContextDealForTaskDispatcherTest, AppExecFwk_ContextDeal_GetGlobalTaskDispatcher_0200, Function | MediumTest | Level3)
{
    GTEST_LOG_(INFO) << "AppExecFwk_ContextDeal_GetGlobalTaskDispatcher_0200 start";
    std::shared_ptr<ContextDeal> contextdeal = std::make_shared<ContextDeal>();
    TaskPriority high = TaskPriority::HIGH;
    EXPECT_TRUE((nullptr == contextdeal->GetGlobalTaskDispatcher(high)));
    GTEST_LOG_(INFO) << "AppExecFwk_ContextDeal_GetGlobalTaskDispatcher_0200 end";
}
}  // namespace AppExecFwk
}  // namespace OHOS