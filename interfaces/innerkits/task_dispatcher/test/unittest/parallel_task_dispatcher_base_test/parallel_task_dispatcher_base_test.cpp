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
#include "parallel_task_dispatcher_base.h"
#undef private
#include "appexecfwk_errors.h"
#include "task_priority.h"
#include "task_executor.h"
#include "task.h"
#include "default_worker_pool_config.h"

using namespace testing;
using namespace testing::ext;
using namespace OHOS::AppExecFwk;
using namespace OHOS;

class ParallelTaskDispatcherBaseTest : public testing::Test {
public:
    static void SetUpTestCase();
    static void TearDownTestCase();
    void SetUp();
    void TearDown();
    std::shared_ptr<ParallelTaskDispatcherBase> CreateParallelTaskDispatcherBase(
        std::string name, TaskPriority priority);
};

void ParallelTaskDispatcherBaseTest::SetUpTestCase()
{}

void ParallelTaskDispatcherBaseTest::TearDownTestCase()
{}

void ParallelTaskDispatcherBaseTest::SetUp()
{}

void ParallelTaskDispatcherBaseTest::TearDown()
{}

std::shared_ptr<ParallelTaskDispatcherBase> CreateParallelTaskDispatcherBase(std::string name, TaskPriority priority)
{
    std::shared_ptr<DefaultWorkerPoolConfig> config_ = std::make_shared<DefaultWorkerPoolConfig>();
    std::shared_ptr<TaskExecutor> executor = std::make_shared<TaskExecutor>(config_);
    std::shared_ptr<ParallelTaskDispatcherBase> parallelTaskDispatcherBaseBase =
        std::make_shared<ParallelTaskDispatcherBase>(priority, executor, name);

    return parallelTaskDispatcherBaseBase;
}

/**
 * @tc.number: ParallelTaskDispatcherBase_ConstructorTest_001
 * @tc.name: Constructor
 * @tc.desc: Test  ParallelTaskDispatcherBase Constructor.
 */
HWTEST(ParallelTaskDispatcherBaseTest, ParallelTaskDispatcherBase_ConstructorTest_001, TestSize.Level0)
{
    auto name = std::string("ParallelTaskDispatcherBase_ConstructorTest_001");
    GTEST_LOG_(INFO) << name << " start";
    std::shared_ptr<ParallelTaskDispatcherBase> parallelTaskDispatcherBase =
        CreateParallelTaskDispatcherBase(name, TaskPriority::DEFAULT);
    EXPECT_EQ(parallelTaskDispatcherBase->GetPriority(), TaskPriority::DEFAULT);
    GTEST_LOG_(INFO) << name << " end";
}

/**
 * @tc.number: ParallelTaskDispatcherBase_SyncDispatchTest_001
 * @tc.name: SyncDispatch
 * @tc.desc: Test ParallelTaskDispatcherBase  sync dispatcher task.
 */
HWTEST(ParallelTaskDispatcherBaseTest, ParallelTaskDispatcherBase_SyncDispatchTest_001, TestSize.Level0)
{
    auto name = std::string("ParallelTaskDispatcherBase_SyncDispatchTest_001");
    GTEST_LOG_(INFO) << name << " start";
    std::shared_ptr<ParallelTaskDispatcherBase> parallelTaskDispatcherBase =
        CreateParallelTaskDispatcherBase(name, TaskPriority::DEFAULT);
    ErrCode errCode = parallelTaskDispatcherBase->SyncDispatch(nullptr);
    EXPECT_TRUE(errCode == ERR_APPEXECFWK_CHECK_FAILED);
    std::atomic<int> count(0);
    EXPECT_EQ(count.load(), 0);
    std::shared_ptr<Runnable> rptr = std::make_shared<Runnable>([&]() {
        std::this_thread::sleep_for(std::chrono::milliseconds(200));
        count.fetch_add(1);
        GTEST_LOG_(INFO) << name << " Runnable1";
    });
    parallelTaskDispatcherBase->SyncDispatch(rptr);
    EXPECT_EQ(count.load(), 1);

    std::shared_ptr<Runnable> rptr2 = std::make_shared<Runnable>([&]() {
        std::this_thread::sleep_for(std::chrono::milliseconds(100));
        count.fetch_add(1);
        GTEST_LOG_(INFO) << name << " Runnable2";
    });
    parallelTaskDispatcherBase->SyncDispatch(rptr2);
    EXPECT_EQ(count.load(), 2);
    GTEST_LOG_(INFO) << name << " end";
}

/**
 * @tc.number: ParallelTaskDispatcherBase_AsyncDispatchTest_001
 * @tc.name: AsyncDispatch
 * @tc.desc: Test ParallelTaskDispatcherBase  async dispatcher task.
 */
HWTEST(ParallelTaskDispatcherBaseTest, ParallelTaskDispatcherBase_AsyncDispatchTest_001, TestSize.Level0)
{
    auto name = std::string("ParallelTaskDispatcherBase_AsyncDispatchTest_001");
    GTEST_LOG_(INFO) << name << " start";
    std::shared_ptr<ParallelTaskDispatcherBase> parallelTaskDispatcherBase =
        CreateParallelTaskDispatcherBase(name, TaskPriority::DEFAULT);
    std::shared_ptr<Revocable> revocable = parallelTaskDispatcherBase->AsyncDispatch(nullptr);
    EXPECT_TRUE(revocable == nullptr);
    std::atomic<int> count(0);
    EXPECT_EQ(count.load(), 0);
    std::shared_ptr<Runnable> rptr = std::make_shared<Runnable>([&]() {
        std::this_thread::sleep_for(std::chrono::milliseconds(1000));
        count.fetch_add(1);
        GTEST_LOG_(INFO) << name << " Runnable1";
    });
    parallelTaskDispatcherBase->AsyncDispatch(rptr);
    EXPECT_LT(count.load(), 1);
    std::this_thread::sleep_for(std::chrono::milliseconds(2000));
    EXPECT_EQ(count.load(), 1);
    GTEST_LOG_(INFO) << name << " end";
}

/**
 * @tc.number: ParallelTaskDispatcherBase_DelayDispatchTest_001
 * @tc.name: DelayDispatch
 * @tc.desc: Test ParallelTaskDispatcherBase  delay dispatch tasks.
 */
HWTEST(ParallelTaskDispatcherBaseTest, ParallelTaskDispatcherBase_DelayDispatchTest_001, TestSize.Level0)
{
    auto name = std::string("ParallelTaskDispatcherBase_DelayDispatchTest_001");
    GTEST_LOG_(INFO) << name << " start";
    std::shared_ptr<ParallelTaskDispatcherBase> parallelTaskDispatcherBase =
        CreateParallelTaskDispatcherBase(name, TaskPriority::DEFAULT);
    std::atomic<int> count(0);
    EXPECT_EQ(count.load(), 0);
    std::shared_ptr<Runnable> rptr = std::make_shared<Runnable>([&]() {
        count.fetch_add(1);
        GTEST_LOG_(INFO) << name << " Runnable";
    });
    parallelTaskDispatcherBase->DelayDispatch(rptr, 1000);
    EXPECT_LT(count.load(), 1);
    std::this_thread::sleep_for(std::chrono::milliseconds(2000));
    EXPECT_EQ(count.load(), 1);
    GTEST_LOG_(INFO) << name << " end";
}

/**
 * @tc.number: ParallelTaskDispatcherBase_DelayDispatchTest_002
 * @tc.name: DelayDispatch
 * @tc.desc: Test ParallelTaskDispatcherBase  delay dispatch tasks.
 */
HWTEST(ParallelTaskDispatcherBaseTest, ParallelTaskDispatcherBase_DelayDispatchTest_002, TestSize.Level0)
{
    auto name = std::string("ParallelTaskDispatcherBase_DelayDispatchTest_002");
    GTEST_LOG_(INFO) << name << " start";
    std::shared_ptr<ParallelTaskDispatcherBase> parallelTaskDispatcherBase =
        CreateParallelTaskDispatcherBase(name, TaskPriority::DEFAULT);
    std::atomic<int> count(0);
    EXPECT_EQ(count.load(), 0);
    std::shared_ptr<Runnable> rptr = std::make_shared<Runnable>([&]() {
        count.fetch_add(1);
        GTEST_LOG_(INFO) << name << " Runnable";
    });
    std::shared_ptr<Revocable> revocable = parallelTaskDispatcherBase->DelayDispatch(nullptr, 1000);
    EXPECT_TRUE(revocable == nullptr);
    GTEST_LOG_(INFO) << name << " end";
}

/**
 * @tc.number: ParallelTaskDispatcherBase_AsyncGroupDispatchTest_001
 * @tc.name: AsyncGroupDispatch
 * @tc.desc: Test ParallelTaskDispatcherBase  add group.
 */
HWTEST(ParallelTaskDispatcherBaseTest, ParallelTaskDispatcherBase_AsyncGroupDispatchTest_001, TestSize.Level0)
{
    auto name = std::string("ParallelTaskDispatcherBase_AsyncGroupDispatchTest_001");
    GTEST_LOG_(INFO) << name << " start";
    std::shared_ptr<ParallelTaskDispatcherBase> parallelTaskDispatcherBase =
        CreateParallelTaskDispatcherBase(name, TaskPriority::DEFAULT);
    std::atomic<int> count(0);
    EXPECT_EQ(count.load(), 0);
    std::shared_ptr<Group> group1 = parallelTaskDispatcherBase->CreateDispatchGroup();
    std::shared_ptr<Runnable> group1Run1 = std::make_shared<Runnable>([&]() {
        std::this_thread::sleep_for(std::chrono::milliseconds(800));
        count.fetch_add(1);
        int index = count.load();
        EXPECT_EQ(index, 3);
        GTEST_LOG_(INFO) << name << " Group1 : Runnable1";
    });
    parallelTaskDispatcherBase->AsyncGroupDispatch(group1, group1Run1);
    std::shared_ptr<Runnable> group1Run2 = std::make_shared<Runnable>([&]() {
        count.fetch_add(1);
        GTEST_LOG_(INFO) << name << " Group1 : Runnable2";
    });
    parallelTaskDispatcherBase->AsyncGroupDispatch(group1, group1Run2);
    std::shared_ptr<Group> group2 = parallelTaskDispatcherBase->CreateDispatchGroup();
    std::shared_ptr<Runnable> group2Run = std::make_shared<Runnable>([&]() {
        count.fetch_add(1);
        GTEST_LOG_(INFO) << name << "  Group2 : Runnable";
    });
    parallelTaskDispatcherBase->AsyncGroupDispatch(group2, group2Run);
    EXPECT_LT(count.load(), 3);
    std::this_thread::sleep_for(std::chrono::milliseconds(1200));
    EXPECT_EQ(count.load(), 3);
    GTEST_LOG_(INFO) << name << " end";
}

/**
 * @tc.number: ParallelTaskDispatcherBase_AsyncGroupDispatchTest_002
 * @tc.name: AsyncGroupDispatch
 * @tc.desc: Test ParallelTaskDispatcherBase  add group.
 */
HWTEST(ParallelTaskDispatcherBaseTest, ParallelTaskDispatcherBase_AsyncGroupDispatchTest_002, TestSize.Level0)
{
    auto name = std::string("ParallelTaskDispatcherBase_AsyncGroupDispatchTest_002");
    GTEST_LOG_(INFO) << name << " start";
    std::shared_ptr<ParallelTaskDispatcherBase> parallelTaskDispatcherBase =
        CreateParallelTaskDispatcherBase(name, TaskPriority::DEFAULT);

    std::shared_ptr<Revocable> revocable = parallelTaskDispatcherBase->AsyncGroupDispatch(nullptr, nullptr);
    EXPECT_TRUE(revocable == nullptr);
    GTEST_LOG_(INFO) << name << " end";
}