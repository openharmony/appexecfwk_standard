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

// redefine private and protected since testcase need to invoke and test private function

#include <gtest/gtest.h>
#include "app_log_wrapper.h"
#include "base_task_dispatcher.h"
#include "task_dispatcher.h"
#include "task_dispatcher_context.h"

using namespace testing::ext;
using namespace OHOS::AppExecFwk;

class ParallelTaskDispatcherModuleTest : public testing::Test {
public:
    static void SetUpTestCase();
    static void TearDownTestCase();
    void SetUp();
    void TearDown();
    std::shared_ptr<ParallelTaskDispatcher> CreateMtParallelTaskDispatcher(std::string &name, TaskPriority priority);
    std::shared_ptr<TaskDispatcherContext> context;
};

void ParallelTaskDispatcherModuleTest::SetUpTestCase()
{}

void ParallelTaskDispatcherModuleTest::TearDownTestCase()
{}

void ParallelTaskDispatcherModuleTest::SetUp()
{
    context = std::make_shared<TaskDispatcherContext>();
}

void ParallelTaskDispatcherModuleTest::TearDown()
{
    context = nullptr;
}

std::shared_ptr<ParallelTaskDispatcher> ParallelTaskDispatcherModuleTest::CreateMtParallelTaskDispatcher(
    std::string &name, TaskPriority priority)
{
    return context->CreateParallelDispatcher(name, priority);
}

/**
 * @tc.number: ParallelTaskDispatcher_LifeCycleTest_001
 * @tc.name: Shutdown
 * @tc.desc: Test ParallelTaskDispatcher task shutdown.
 */
HWTEST_F(ParallelTaskDispatcherModuleTest, ParallelTaskDispatcher_LifeCycleTest_001, TestSize.Level1)
{
    auto name = std::string("ParallelTaskDispatcher_LifeCycleTest_001");
    GTEST_LOG_(INFO) << name + " start";
    std::shared_ptr<ParallelTaskDispatcher> ptr = CreateMtParallelTaskDispatcher(name, TaskPriority::DEFAULT);
    std::atomic<int> count(0);
    std::shared_ptr<Runnable> runnable = std::make_shared<Runnable>([&]() {
        count.fetch_add(1);
        GTEST_LOG_(INFO) << name << " GroupDispatchNotify Runnable";
    });
    ptr->AsyncDispatch(runnable);
    std::this_thread::sleep_for(std::chrono::milliseconds(200));
    context->Shutdown(true);
    GTEST_LOG_(INFO) << name + " end";
}

/**
 * @tc.number: ParallelTaskDispatcher_CreateParallelDispatcherTest_001
 * @tc.name: CreateParallelDispatcher
 * @tc.desc: Test ParallelTaskDispatcher create parallel dispatcher
 */
HWTEST_F(ParallelTaskDispatcherModuleTest, ParallelTaskDispatcher_CreateParallelDispatcherTest_001, TestSize.Level1)
{
    auto name = std::string("ParallelTaskDispatcher_CreateParallelDispatcherTest_001");
    GTEST_LOG_(INFO) << name + " start";
    std::shared_ptr<ParallelTaskDispatcher> ptr1 = CreateMtParallelTaskDispatcher(name, TaskPriority::DEFAULT);
    std::shared_ptr<ParallelTaskDispatcher> ptr2 = CreateMtParallelTaskDispatcher(name, TaskPriority::DEFAULT);
    EXPECT_FALSE((ptr1.get() == ptr2.get()));
    GTEST_LOG_(INFO) << name + " end";
}

/**
 * @tc.number: ParallelTaskDispatcher_SyncDispatchTest_001
 * @tc.name: SyncDispatch
 * @tc.desc: Test ParallelTaskDispatcher sync dispatch.
 */
HWTEST_F(ParallelTaskDispatcherModuleTest, ParallelTaskDispatcher_SyncDispatchTest_001, TestSize.Level1)
{
    auto name = std::string("ParallelTaskDispatcher_SyncDispatchTest_001");
    GTEST_LOG_(INFO) << name + " start";
    std::shared_ptr<ParallelTaskDispatcher> ptr = CreateMtParallelTaskDispatcher(name, TaskPriority::DEFAULT);
    std::atomic<int> count(0);
    EXPECT_TRUE(count.load() == 0);
    long sleep1 = 200;
    ptr->SyncDispatch(std::make_shared<Runnable>([&]() {
        auto time = std::chrono::milliseconds(sleep1);
        std::this_thread::sleep_for(time);
        int index = count.fetch_add(1);
        EXPECT_TRUE(index == 0);
        GTEST_LOG_(INFO) << name << " Runnable1";
    }));
    EXPECT_TRUE(count.load() == 1);
    long sleep2 = 100;
    ptr->SyncDispatch(std::make_shared<Runnable>([&]() {
        auto time = std::chrono::milliseconds(sleep2);
        std::this_thread::sleep_for(time);
        int index = count.fetch_add(1);
        EXPECT_TRUE(index == 1);
        GTEST_LOG_(INFO) << name << " Runnable2";
    }));
    EXPECT_TRUE(count.load() == 2);
    long wait = sleep1 + sleep2 + 100;
    auto time = std::chrono::milliseconds(wait);
    std::this_thread::sleep_for(time);
    EXPECT_TRUE(count.load() == 2);
    GTEST_LOG_(INFO) << name + " end";
}

/**
 * @tc.number: ParallelTaskDispatcher_AsyncDispatchTest_001
 * @tc.name: AsyncDispatch
 * @tc.desc: Test ParallelTaskDispatcher async dispatch.
 */
HWTEST_F(ParallelTaskDispatcherModuleTest, ParallelTaskDispatcher_AsyncDispatchTest_001, TestSize.Level0)
{
    auto name = std::string("ParallelTaskDispatcher_AsyncDispatchTest_001");
    GTEST_LOG_(INFO) << name << " start";
    std::shared_ptr<ParallelTaskDispatcher> ptr = CreateMtParallelTaskDispatcher(name, TaskPriority::DEFAULT);
    std::atomic<int> count(0);
    EXPECT_EQ(count.load(), 0);
    std::shared_ptr<Runnable> func = std::make_shared<Runnable>([&]() {
        std::this_thread::sleep_for(std::chrono::milliseconds(1000));
        count.fetch_add(1);
        GTEST_LOG_(INFO) << name << " Runnable1";
    });
    ptr->AsyncDispatch(func);
    EXPECT_LT(count.load(), 1);
    std::this_thread::sleep_for(std::chrono::milliseconds(2000));
    EXPECT_EQ(count.load(), 1);
    GTEST_LOG_(INFO) << name << " end";
}

/**
 * @tc.number: ParallelTaskDispatcher_AsyncGroupDispatchTest_001
 * @tc.name: AsyncGroup
 * @tc.desc: Test ParallelTaskDispatcher group.
 */
HWTEST_F(ParallelTaskDispatcherModuleTest, ParallelTaskDispatcher_AsyncGroupDispatchTest_001, TestSize.Level1)
{
    auto name = std::string("ParallelTaskDispatcher_AsyncGroupDispatchTest_001");
    GTEST_LOG_(INFO) << name + " start";
    std::shared_ptr<ParallelTaskDispatcher> ptr = CreateMtParallelTaskDispatcher(name, TaskPriority::DEFAULT);
    std::atomic<int> count(0);
    long sleep1 = 200;
    std::shared_ptr<Group> group = ptr->CreateDispatchGroup();

    ptr->AsyncGroupDispatch(group, std::make_shared<Runnable>([&]() {
        auto time = std::chrono::milliseconds(sleep1);
        std::this_thread::sleep_for(time);
        int index = count.fetch_add(1);
        EXPECT_TRUE(index == 1);
        GTEST_LOG_(INFO) << name << " AsyncGroupDispatch1  : sleep1 time=" << sleep1;
    }));

    long sleep2 = 100;
    ptr->AsyncGroupDispatch(group, std::make_shared<Runnable>([&]() {
        auto time = std::chrono::milliseconds(sleep2);
        std::this_thread::sleep_for(time);
        int index = count.fetch_add(1);
        EXPECT_TRUE(index == 0);
        GTEST_LOG_(INFO) << name << " AsyncGroupDispatch1  : sleep2 time=" << sleep2;
    }));
    std::shared_ptr<Runnable> runnable = std::make_shared<Runnable>([&]() {
        std::this_thread::sleep_for(std::chrono::milliseconds(150));
        int index = count.fetch_add(1);
        EXPECT_TRUE(index == 2);
        GTEST_LOG_(INFO) << name << " GroupDispatchNotify Runnable";
    });
    ptr->GroupDispatchNotify(group, runnable);
    EXPECT_TRUE(count.load() < 2);
    EXPECT_TRUE(ptr->GroupDispatchWait(group, 1000));
    std::this_thread::sleep_for(std::chrono::milliseconds(300));
    EXPECT_TRUE(count.load() == 3);
    GTEST_LOG_(INFO) << name + " end";
}

/**
 * @tc.number: ParallelTaskDispatcher_AsyncGroupDispatchTest_002
 * @tc.name: AsyncGroup
 * @tc.desc: Test ParallelTaskDispatcher group.
 */
HWTEST_F(ParallelTaskDispatcherModuleTest, ParallelTaskDispatcher_AsyncGroupDispatchTest_002, TestSize.Level1)
{
    auto name = std::string("ParallelTaskDispatcher_AsyncGroupDispatchTest_002");
    GTEST_LOG_(INFO) << name + " start";
    std::shared_ptr<ParallelTaskDispatcher> ptr = CreateMtParallelTaskDispatcher(name, TaskPriority::DEFAULT);
    std::shared_ptr<Group> group = ptr->CreateDispatchGroup();
    ptr->AsyncGroupDispatch(group, std::make_shared<Runnable>([&]() {
        auto time = std::chrono::milliseconds(200);
        std::this_thread::sleep_for(time);
        GTEST_LOG_(INFO) << name << " AsyncGroupDispatch1";
    }));

    ptr->AsyncGroupDispatch(group, std::make_shared<Runnable>([&]() {
        auto time = std::chrono::milliseconds(200);
        std::this_thread::sleep_for(time);
        GTEST_LOG_(INFO) << name << " AsyncGroupDispatch1";
    }));
    bool result = ptr->GroupDispatchWait(group, 10);
    EXPECT_EQ(result, false);
    auto time = std::chrono::milliseconds(600);
    std::this_thread::sleep_for(time);
    GTEST_LOG_(INFO) << name + " end";
}

/**
 * @tc.number: ParallelTaskDispatcher_ApplyDispatchTest_001
 * @tc.name: ApplyDispatch
 * @tc.desc: Test ParallelTaskDispatcher apply dispatch.
 */
HWTEST_F(ParallelTaskDispatcherModuleTest, ParallelTaskDispatcher_ApplyDispatchTest_001, TestSize.Level0)
{
    auto name = std::string("ParallelTaskDispatcher_ApplyDispatchTest_001");
    GTEST_LOG_(INFO) << name << " start";
    std::shared_ptr<ParallelTaskDispatcher> ptr = CreateMtParallelTaskDispatcher(name, TaskPriority::DEFAULT);
    // init
    std::atomic<int> count(0);
    const int repeatCount = 10;
    long sleep1 = 200;
    auto repeats1 = std::make_shared<IteratableTask<long>>([&](long index1) {
        auto time = std::chrono::milliseconds(sleep1);
        std::this_thread::sleep_for(time);
        int index = count.fetch_add(1);
        EXPECT_TRUE(index >= repeatCount);
        EXPECT_TRUE(index < repeatCount * 2);
        GTEST_LOG_(INFO) << name + " repeats1";
    });
    ptr->ApplyDispatch(repeats1, repeatCount);

    const long sleep2 = 10;
    auto repeats2 = std::make_shared<IteratableTask<long>>([&](long index1) {
        auto time = std::chrono::milliseconds(sleep2);
        std::this_thread::sleep_for(time);
        int index = count.fetch_add(1);
        EXPECT_TRUE(index < repeatCount);
        GTEST_LOG_(INFO) << name + " repeats2";
    });
    ptr->ApplyDispatch(repeats2, repeatCount);

    EXPECT_TRUE(count.load() < 20);
    long wait = sleep1 + sleep2 + 100;
    auto time = std::chrono::milliseconds(wait);
    std::this_thread::sleep_for(time);
    EXPECT_TRUE(count.load() == 20);
    GTEST_LOG_(INFO) << name << " end";
}

/**
 * @tc.number: ParallelTaskDispatcher_DelayDispatchTest_001
 * @tc.name: DelayDispatch
 * @tc.desc: Test ParallelTaskDispatcher delay dispatch.
 */
HWTEST_F(ParallelTaskDispatcherModuleTest, ParallelTaskDispatcher_DelayDispatchTest_001, TestSize.Level0)
{
    auto name = std::string("ParallelTaskDispatcher_DelayDispatchTest_001");
    GTEST_LOG_(INFO) << name << " start";
    std::shared_ptr<ParallelTaskDispatcher> ptr = CreateMtParallelTaskDispatcher(name, TaskPriority::DEFAULT);
    std::atomic<int> count(0);
    long sleep1 = 200;
    ptr->DelayDispatch(std::make_shared<Runnable>([&]() {
        auto time = std::chrono::milliseconds(sleep1);
        // execute second
        std::this_thread::sleep_for(time);
        int index = count.fetch_add(1);
        EXPECT_TRUE(index == 1);
        GTEST_LOG_(INFO) << name << " Runnable1";
    }),
        sleep1);

    long sleep2 = 100;
    ptr->DelayDispatch(std::make_shared<Runnable>([&]() {
        auto time = std::chrono::milliseconds(sleep2);
        // execute first
        std::this_thread::sleep_for(time);
        int index = count.fetch_add(1);
        EXPECT_TRUE(index == 0);
        GTEST_LOG_(INFO) << name << " Runnable2";
    }),
        sleep2);
    // printf("111111111111111111111111111111111111111111111111111111111111111111");
    EXPECT_TRUE(count.load() < 2);
    long wait = 1000;
    auto time = std::chrono::milliseconds(wait);
    std::this_thread::sleep_for(time);
    // printf("222222222222222222222222222222222222222222222222222222222222222222");
    EXPECT_TRUE(count.load() == 2);
    // printf("33333333333333333333333333333333333333333333333333333333333333333");
    // printf("context use count:%d", context.use_count());
    GTEST_LOG_(INFO) << name << " end";
}

/**
 * @tc.number: ParallelTaskDispatcher_AsyncDispatchBarrierTest_001
 * @tc.name: AsyncDispatchBarrier
 * @tc.desc: Test ParallelTaskDispatcher barrier.
 */
HWTEST_F(ParallelTaskDispatcherModuleTest, ParallelTaskDispatcher_AsyncDispatchBarrierTest_001, TestSize.Level0)
{
    auto name = std::string("ParallelTaskDispatcher_AsyncDispatchBarrierTest_001");
    GTEST_LOG_(INFO) << name << " start";
    std::shared_ptr<ParallelTaskDispatcher> ptr = CreateMtParallelTaskDispatcher(name, TaskPriority::DEFAULT);
    std::atomic<int> count(0);
    EXPECT_TRUE(count.load() == 0);
    auto group = ptr->CreateDispatchGroup();
    long sleepG1 = 200;
    ptr->AsyncGroupDispatch((group), std::make_shared<Runnable>([&]() {
        auto time = std::chrono::milliseconds(sleepG1);
        std::this_thread::sleep_for(time);
        int index = count.fetch_add(1);
        EXPECT_TRUE(index == 1);
        GTEST_LOG_(INFO) << name << "group Runnable1";
    }));
    long sleepG2 = 100;
    ptr->AsyncGroupDispatch((group), std::make_shared<Runnable>([&]() {
        auto time = std::chrono::milliseconds(sleepG2);
        std::this_thread::sleep_for(time);
        int index = count.fetch_add(1);
        EXPECT_TRUE(index == 0);
        GTEST_LOG_(INFO) << name << "group Runnable2";
    }));
    EXPECT_TRUE(count.load() == 0);
    ptr->AsyncDispatchBarrier(std::make_shared<Runnable>([&]() {
        EXPECT_TRUE(count.load() == 2);
        count.fetch_add(1);
        GTEST_LOG_(INFO) << name << "AsyncDispatchBarrier Runnable";
    }));
    GTEST_LOG_(INFO) << name << "AsyncDispatchBarrier After";

    auto time = std::chrono::milliseconds(1000);
    std::this_thread::sleep_for(time);
    EXPECT_TRUE(count.load() == 3);
    GTEST_LOG_(INFO) << name << " end";
}

/**
 * @tc.number: ParallelTaskDispatcher_SyncDispatchBarrierTest_001
 * @tc.name: SyncDispatchBarrier
 * @tc.desc: Test ParallelTaskDispatcher sync barrier.
 */
HWTEST_F(ParallelTaskDispatcherModuleTest, ParallelTaskDispatcher_SyncDispatchBarrierTest_001, TestSize.Level0)
{
    auto name = std::string("ParallelTaskDispatcher_SyncDispatchBarrierTest_001");
    GTEST_LOG_(INFO) << name << " start";
    std::shared_ptr<ParallelTaskDispatcher> ptr = CreateMtParallelTaskDispatcher(name, TaskPriority::DEFAULT);
    std::atomic<int> count(0);
    EXPECT_TRUE(count.load() == 0);
    auto group = ptr->CreateDispatchGroup();
    long sleepG1 = 200;
    ptr->AsyncGroupDispatch((group), std::make_shared<Runnable>([&]() {
        auto time = std::chrono::milliseconds(sleepG1);
        std::this_thread::sleep_for(time);
        int index = count.fetch_add(1);
        EXPECT_TRUE(index == 1);
        GTEST_LOG_(INFO) << name << "group Runnable1";
    }));
    long sleepG2 = 100;
    ptr->AsyncGroupDispatch((group), std::make_shared<Runnable>([&]() {
        auto time = std::chrono::milliseconds(sleepG2);
        std::this_thread::sleep_for(time);
        int index = count.fetch_add(1);
        EXPECT_TRUE(index == 0);
        GTEST_LOG_(INFO) << name << " group Runnable2";
    }));
    EXPECT_TRUE(count.load() == 0);
    ptr->SyncDispatchBarrier(std::make_shared<Runnable>([&]() {
        EXPECT_TRUE(count.load() == 2);
        count.fetch_add(1);
        GTEST_LOG_(INFO) << name << "SyncDispatchBarrier Runnable";
    }));
    GTEST_LOG_(INFO) << name << "SyncDispatchBarrier After";

    auto time = std::chrono::milliseconds(1000);
    std::this_thread::sleep_for(time);
    EXPECT_TRUE(count.load() == 3);
    GTEST_LOG_(INFO) << name << " end";
}
