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
#include "parallel_task_dispatcher.h"
#undef private
#include "task_priority.h"
#include "task_executor.h"
#include "task.h"
#include "default_worker_pool_config.h"
#include "appexecfwk_errors.h"

using namespace testing;
using namespace testing::ext;
using namespace OHOS::AppExecFwk;
using namespace OHOS;

class ParallelTaskDispatcherTest : public testing::Test {
public:
    static void SetUpTestCase();
    static void TearDownTestCase();
    void SetUp();
    void TearDown();
    std::shared_ptr<ParallelTaskDispatcher> CreateParallelTaskDispatcher(std::string name, TaskPriority priority);
};

void ParallelTaskDispatcherTest::SetUpTestCase()
{}

void ParallelTaskDispatcherTest::TearDownTestCase()
{}

void ParallelTaskDispatcherTest::SetUp()
{}

void ParallelTaskDispatcherTest::TearDown()
{}

std::shared_ptr<ParallelTaskDispatcher> CreateParallelTaskDispatcher(std::string name, TaskPriority priority)
{
    std::shared_ptr<DefaultWorkerPoolConfig> config_ = std::make_shared<DefaultWorkerPoolConfig>();
    std::shared_ptr<TaskExecutor> executor = std::make_shared<TaskExecutor>(config_);
    std::shared_ptr<ParallelTaskDispatcher> parallelTaskDispatcher =
        std::make_shared<ParallelTaskDispatcher>(name, priority, executor);
    return parallelTaskDispatcher;
}

/**
 * @tc.number: ParallelTaskDispatcher_ConstructorTest_001
 * @tc.name: Constructor
 * @tc.desc: Test ParallelTaskDispatcher Constructor.
 */
HWTEST(ParallelTaskDispatcherTest, ParallelTaskDispatcher_ConstructorTest_001, TestSize.Level0)
{
    auto name = std::string("Parallel_Task_Dispatcher_Test_001");
    GTEST_LOG_(INFO) << name << " start";
    std::shared_ptr<ParallelTaskDispatcher> parallelTaskDispatcher =
        CreateParallelTaskDispatcher(name, TaskPriority::DEFAULT);
    {
        EXPECT_EQ(parallelTaskDispatcher->GetPriority(), TaskPriority::DEFAULT);
    }
    GTEST_LOG_(INFO) << name << " end";
}

/**
 * @tc.number: ParallelTaskDispatcher_SyncDispatchBarrierTest_001
 * @tc.name: SyncDispatchBarrier
 * @tc.desc: Test ParallelTaskDispatcher SyncDispatchBarrier.
 */
HWTEST(ParallelTaskDispatcherTest, ParallelTaskDispatcher_SyncDispatchBarrierTest_001, TestSize.Level0)
{
    auto name = std::string("ParallelTaskDispatcher_SyncDispatchBarrierTest_001");
    GTEST_LOG_(INFO) << name << " start";
    std::shared_ptr<ParallelTaskDispatcher> parallelTaskDispatcher =
        CreateParallelTaskDispatcher(name, TaskPriority::DEFAULT);
    std::shared_ptr<Runnable> rptr = std::make_shared<Runnable>([&]() { GTEST_LOG_(INFO) << name << " Runnable"; });
    if (parallelTaskDispatcher->barrierHandler_ != nullptr) {
        int before = parallelTaskDispatcher->barrierHandler_->barrierQueue_.size();
        parallelTaskDispatcher->SyncDispatchBarrier(rptr);
        int after = parallelTaskDispatcher->barrierHandler_->barrierQueue_.size();
        EXPECT_LT(before, after);
    } else {
        GTEST_LOG_(INFO) << "parallelTaskDispatcher barrierHandler_ is null";
    }
    GTEST_LOG_(INFO) << name << " end";
}

/**
 * @tc.number: ParallelTaskDispatcher_SyncDispatchBarrierTest_002
 * @tc.name: SyncDispatchBarrier
 * @tc.desc: Test ParallelTaskDispatcher SyncDispatchBarrier.
 */
HWTEST(ParallelTaskDispatcherTest, ParallelTaskDispatcher_SyncDispatchBarrierTest_002, TestSize.Level0)
{
    auto name = std::string("ParallelTaskDispatcher_SyncDispatchBarrierTest_002");
    GTEST_LOG_(INFO) << name << " start";
    std::shared_ptr<ParallelTaskDispatcher> parallelTaskDispatcher =
        CreateParallelTaskDispatcher(name, TaskPriority::DEFAULT);
    ErrCode result = parallelTaskDispatcher->SyncDispatchBarrier(nullptr);
    ASSERT_TRUE(result == ERR_APPEXECFWK_CHECK_FAILED);
    GTEST_LOG_(INFO) << name << " end";
}

/**
 * @tc.number: ParallelTaskDispatcher_AsyncDispatchBarrierTest_001
 * @tc.name: AsyncDispatchBarrier
 * @tc.desc: Test ParallelTaskDispatcher AsyncDispatchBarrier.
 */
HWTEST(ParallelTaskDispatcherTest, ParallelTaskDispatcher_AsyncDispatchBarrierTest_001, TestSize.Level0)
{
    auto name = std::string("ParallelTaskDispatcher_AsyncDispatchBarrierTest_001");
    GTEST_LOG_(INFO) << name << " start";
    std::shared_ptr<ParallelTaskDispatcher> parallelTaskDispatcher =
        CreateParallelTaskDispatcher(name, TaskPriority::DEFAULT);
    std::shared_ptr<Runnable> rptr = std::make_shared<Runnable>([&]() { GTEST_LOG_(INFO) << name << " Runnable"; });
    if (parallelTaskDispatcher->barrierHandler_ != nullptr) {
        int before = parallelTaskDispatcher->barrierHandler_->barrierQueue_.size();
        parallelTaskDispatcher->AsyncDispatchBarrier(rptr);
        int after = parallelTaskDispatcher->barrierHandler_->barrierQueue_.size();
        EXPECT_LT(before, after);
    } else {
        GTEST_LOG_(INFO) << "parallelTaskDispatcher barrierHandler_ is null";
    }
    GTEST_LOG_(INFO) << name << " end";
}

/**
 * @tc.number: ParallelTaskDispatcher_AsyncDispatchBarrierTest_002
 * @tc.name: AsyncDispatchBarrier
 * @tc.desc: Test ParallelTaskDispatcher AsyncDispatchBarrier.
 */
HWTEST(ParallelTaskDispatcherTest, ParallelTaskDispatcher_AsyncDispatchBarrierTest_002, TestSize.Level0)
{
    auto name = std::string("ParallelTaskDispatcher_AsyncDispatchBarrierTest_002");
    GTEST_LOG_(INFO) << name << " start";
    std::shared_ptr<ParallelTaskDispatcher> parallelTaskDispatcher =
        CreateParallelTaskDispatcher(name, TaskPriority::DEFAULT);
    ErrCode result = parallelTaskDispatcher->AsyncDispatchBarrier(nullptr);
    ASSERT_TRUE(result == ERR_APPEXECFWK_CHECK_FAILED);
    GTEST_LOG_(INFO) << name << " end";
}
