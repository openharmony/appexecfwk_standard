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
#define private public
#include "global_task_dispatcher.h"
#undef private
#include "default_worker_pool_config.h"

using namespace testing;
using namespace testing::ext;
using namespace OHOS::AppExecFwk;

class GlobalTaskDispatcherTest : public testing::Test {
public:
    static void SetUpTestCase();
    static void TearDownTestCase();
    void SetUp();
    void TearDown();
};

void GlobalTaskDispatcherTest::SetUpTestCase()
{}

void GlobalTaskDispatcherTest::TearDownTestCase()
{}

void GlobalTaskDispatcherTest::SetUp()
{}

void GlobalTaskDispatcherTest::TearDown()
{}

/**
 * @tc.number: GlobalTaskDispatcher_ConstructorTest_001
 * @tc.name: Constructor
 * @tc.desc: Test  GlobalTaskDispatcherTest Constructor.
 */
HWTEST(GlobalTaskDispatcherTest, GlobalTaskDispatcher_ConstructorTest_001, TestSize.Level0)
{
    auto name = std::string("GlobalTaskDispatcher_ConstructorTest_001");
    GTEST_LOG_(INFO) << name << " start";
    std::shared_ptr<DefaultWorkerPoolConfig> config_ = std::make_shared<DefaultWorkerPoolConfig>();
    std::shared_ptr<TaskExecutor> executor = std::make_shared<TaskExecutor>(config_);
    std::shared_ptr<GlobalTaskDispatcher> globalTaskDispatcher =
        std::make_shared<GlobalTaskDispatcher>(TaskPriority::DEFAULT, executor);
    EXPECT_NE(globalTaskDispatcher, nullptr);
    GTEST_LOG_(INFO) << name << " end";
}