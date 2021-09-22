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
#define private public
#define protected public
#include "base_task_dispatcher.h"
#include "task_dispatcher_context.h"
#include "task_dispatcher.h"

#undef private
#undef protected
#include <gtest/gtest.h>
#include "app_log_wrapper.h"

using namespace testing::ext;
using namespace OHOS::AppExecFwk;

class SerialTaskDispatcherModuleTest : public testing::Test {
public:
    static void SetUpTestCase();
    static void TearDownTestCase();
    void SetUp();
    void TearDown();
    std::shared_ptr<SerialTaskDispatcher> CreateMtSerialTaskDispatcher(std::string &name, TaskPriority priority);
    std::shared_ptr<TaskDispatcherContext> context;
};

void SerialTaskDispatcherModuleTest::SetUpTestCase()
{}

void SerialTaskDispatcherModuleTest::TearDownTestCase()
{}

void SerialTaskDispatcherModuleTest::SetUp()
{
    context = std::make_shared<TaskDispatcherContext>();
}

void SerialTaskDispatcherModuleTest::TearDown()
{
    context = nullptr;
}

std::shared_ptr<SerialTaskDispatcher> SerialTaskDispatcherModuleTest::CreateMtSerialTaskDispatcher(
    std::string &name, TaskPriority priority)
{
    std::shared_ptr<SerialTaskDispatcher> ptrSerialTaskDispatcher = context->CreateSerialDispatcher(name, priority);
    return ptrSerialTaskDispatcher;
}

/**
 * @tc.number: SerialTaskDispatcher_LifeCycleTest_001
 * @tc.name: Shutdown
 * @tc.desc: Test SerialTaskDispatcher task shutdown.
 */
HWTEST_F(SerialTaskDispatcherModuleTest, SerialTaskDispatcher_LifeCycleTest_001, TestSize.Level1)
{
    auto name = std::string("SerialTaskDispatcher_LifeCycleTest_001");
    GTEST_LOG_(INFO) << name + " start";
    std::shared_ptr<SerialTaskDispatcher> ptr = CreateMtSerialTaskDispatcher(name, TaskPriority::DEFAULT);
    std::atomic<int> count(0);
    std::shared_ptr<Runnable> runnable = std::make_shared<Runnable>([&]() {
        std::this_thread::sleep_for(std::chrono::milliseconds(500));
        count.fetch_add(1);
        GTEST_LOG_(INFO) << name << " GroupDispatchNotify Runnable";
    });
    ptr->SyncDispatch(runnable);
    context->Shutdown(true);
    EXPECT_EQ(count.load(), 1);
    GTEST_LOG_(INFO) << name + " end";
}

/**
 * @tc.number: SerialTaskDispatcher_CreateSerialDispatcherTest_001
 * @tc.name: CreateSerialDispatcher
 * @tc.desc: Test SerialTaskDispatcher create serial dispatcher.
 */
HWTEST_F(SerialTaskDispatcherModuleTest, SerialTaskDispatcher_CreateSerialDispatcherTest_001, TestSize.Level1)
{
    auto name = std::string("SerialTaskDispatcher_CreateSerialDispatcherTest_001");
    GTEST_LOG_(INFO) << name + " start";
    std::shared_ptr<SerialTaskDispatcher> ptr1 = CreateMtSerialTaskDispatcher(name, TaskPriority::DEFAULT);
    std::shared_ptr<SerialTaskDispatcher> ptr2 = CreateMtSerialTaskDispatcher(name, TaskPriority::DEFAULT);
    EXPECT_FALSE((ptr1.get() == ptr2.get()));
    GTEST_LOG_(INFO) << name + " end";
}

/**
 * @tc.number: SerialTaskDispatcher_SyncDispatchTest_001
 * @tc.name: SyncDispatch
 * @tc.desc: Test SerialTaskDispatcher sync dispatch.
 */
HWTEST_F(SerialTaskDispatcherModuleTest, SerialTaskDispatcher_SyncDispatchTest_001, TestSize.Level1)
{
    auto name = std::string("SerialTaskDispatcher_SyncDispatch_002");
    GTEST_LOG_(INFO) << name << " start";

    std::shared_ptr<SerialTaskDispatcher> ptr = CreateMtSerialTaskDispatcher(name, TaskPriority::DEFAULT);
    std::atomic<int> count(0);
    EXPECT_TRUE(count.load() == 0);
    long sleep1 = 200;
    ptr->SyncDispatch(std::make_shared<Runnable>([&]() {
        auto time = std::chrono::milliseconds(sleep1);
        std::this_thread::sleep_for(time);
        int index = count.fetch_add(1);
        EXPECT_TRUE(index == 0);
        GTEST_LOG_(INFO) << name << " SyncDispatch1  : sleep1 time=" << sleep1;
    }));
    EXPECT_TRUE(count.load() == 1);

    long sleep2 = 100;
    ptr->SyncDispatch(std::make_shared<Runnable>([&]() {
        auto time = std::chrono::milliseconds(sleep2);
        std::this_thread::sleep_for(time);
        int index = count.fetch_add(1);
        EXPECT_TRUE(index == 1);
        GTEST_LOG_(INFO) << name << " SyncDispatch2  : sleep2 time=" << sleep2;
    }));
    EXPECT_TRUE(count.load() == 2);
    long wait = sleep1 + sleep2 + 100;
    auto time = std::chrono::milliseconds(wait);
    std::this_thread::sleep_for(time);
    EXPECT_TRUE(count.load() == 2);

    GTEST_LOG_(INFO) << name << " end";
}

/**
 * @tc.number: SerialTaskDispatcher_AsyncDispatchTest_001
 * @tc.name: AsyncDispatch
 * @tc.desc: Test SerialTaskDispatcher async dispatch.
 */
HWTEST_F(SerialTaskDispatcherModuleTest, SerialTaskDispatcher_AsyncDispatchTest_001, TestSize.Level1)
{
    auto name = std::string("SerialTaskDispatcher_AsyncDispatchTest_001");
    GTEST_LOG_(INFO) << name + " start";
    std::shared_ptr<SerialTaskDispatcher> ptr = CreateMtSerialTaskDispatcher(name, TaskPriority::DEFAULT);
    std::atomic<int> count(0);
    long sleep1 = 200;
    ptr->AsyncDispatch(std::make_shared<Runnable>([&]() {
        auto time = std::chrono::milliseconds(sleep1);
        std::this_thread::sleep_for(time);
        int index = count.fetch_add(1);
        EXPECT_TRUE(index == 0);
        GTEST_LOG_(INFO) << name << " AsyncDispatch1  : sleep2 time=" << sleep1;
    }));
    EXPECT_TRUE(count.load() < 1);
    long sleep2 = 100;
    ptr->AsyncDispatch(std::make_shared<Runnable>([&]() {
        auto time = std::chrono::milliseconds(sleep2);
        std::this_thread::sleep_for(time);
        int index = count.fetch_add(1);
        EXPECT_TRUE(index == 1);
        GTEST_LOG_(INFO) << name << " AsyncDispatch2  : sleep2 time=" << sleep2;
    }));
    EXPECT_TRUE(count.load() < 2);
    long wait = sleep1 + sleep2 + 100;
    auto time = std::chrono::milliseconds(wait);
    std::this_thread::sleep_for(time);
    EXPECT_TRUE(count.load() == 2);
    GTEST_LOG_(INFO) << name + " end";
}

/**
 * @tc.number: SerialTaskDispatcher_AsyncDispatchTest_002
 * @tc.name: AsyncDispatch
 * @tc.desc: Test SerialTaskDispatcher async dispatch.
 */
HWTEST_F(SerialTaskDispatcherModuleTest, SerialTaskDispatcher_AsyncDispatchTest_002, TestSize.Level1)
{
    auto name = std::string("SerialTaskDispatcher_AsyncDispatchTest_002");
    GTEST_LOG_(INFO) << name + " start";
    std::shared_ptr<SerialTaskDispatcher> ptr = CreateMtSerialTaskDispatcher(name, TaskPriority::DEFAULT);
    const int total = 300;
    std::atomic<int> count(0);
    long wait = 100;
    for (int i = 0; i < total; i++) {
        ptr->AsyncDispatch(std::make_shared<Runnable>([&count, &name]() {
            auto time = std::chrono::milliseconds(1);
            std::this_thread::sleep_for(time);
            int index = count.fetch_add(1);
            GTEST_LOG_(INFO) << "SerialTaskDispatcher_AsyncDispatchTest_002 task" + std::to_string(index) + "end";
        }));
        wait += 10;
    }
    EXPECT_TRUE(count.load() < total);
    auto time = std::chrono::milliseconds(wait + 1000);
    std::this_thread::sleep_for(time);
    EXPECT_TRUE(count.load() == total);
    GTEST_LOG_(INFO) << name + " end";
}

/**
 * @tc.number: SerialTaskDispatcher_DelayDispatchTest_001
 * @tc.name: DelayDispatch
 * @tc.desc: Test SerialTaskDispatcher delay dispatch.
 */
HWTEST_F(SerialTaskDispatcherModuleTest, SerialTaskDispatcher_DelayDispatchTest_001, TestSize.Level0)
{
    auto name = std::string("SerialTaskDispatcher_DelayDispatchTest_001");
    GTEST_LOG_(INFO) << name + " start";
    std::shared_ptr<SerialTaskDispatcher> dispatcher = CreateMtSerialTaskDispatcher(name, TaskPriority::DEFAULT);
    std::atomic<int> count(0);
    int sleep1 = 2000;
    dispatcher->DelayDispatch(std::make_shared<Runnable>([&count, sleep = sleep1]() {
        int index = count.fetch_add(1);
        EXPECT_TRUE(index == 2);
        GTEST_LOG_(INFO) << "SerialTaskDispatcher_DelayDispatchTest_001 task" + std::to_string(index) + "end, elapsed" +
                                std::to_string(sleep) + " ms";
    }),
        sleep1);
    EXPECT_TRUE(count.load() == 0);

    int sleep2 = 1000;
    dispatcher->DelayDispatch(std::make_shared<Runnable>([&count, sleep = sleep2]() {
        int index = count.fetch_add(1);
        EXPECT_TRUE(index == 0 || index == 1);
        GTEST_LOG_(INFO) << "SerialTaskDispatcher_DelayDispatchTest_001 task" + std::to_string(index) + "end, elapsed" +
                                std::to_string(sleep) + " ms";
    }),
        sleep2);
    EXPECT_TRUE(count.load() == 0);

    int sleep3 = 1000;
    dispatcher->DelayDispatch(std::make_shared<Runnable>([&count, sleep = sleep3]() {
        int index = count.fetch_add(1);
        EXPECT_TRUE(index == 0 || index == 1);
        GTEST_LOG_(INFO) << "SerialTaskDispatcher_DelayDispatchTest_001 task" + std::to_string(index) + "end, elapsed" +
                                std::to_string(sleep) + " ms";
    }),
        sleep3);
    EXPECT_TRUE(count.load() == 0);

    {
        long wait = 100;
        auto time = std::chrono::milliseconds(wait);
        std::this_thread::sleep_for(time);
    }
    {
        long wait = 1000;
        GTEST_LOG_(INFO) << ("wait for task, wait= " + std::to_string(wait) + " ms");
        auto time = std::chrono::milliseconds(wait);
        std::this_thread::sleep_for(time);
        EXPECT_TRUE(count.load() == 2);
    }
    {
        long wait = 500;
        GTEST_LOG_(INFO) << ("wait for task, wait= " + std::to_string(wait) + " ms");
        auto time = std::chrono::milliseconds(wait);
        std::this_thread::sleep_for(time);
        EXPECT_TRUE(count.load() == 2);
    }
    {
        long wait = 1000;
        GTEST_LOG_(INFO) << ("wait for task, wait= " + std::to_string(wait) + " ms");
        auto time = std::chrono::milliseconds(wait);
        std::this_thread::sleep_for(time);
        EXPECT_TRUE(count.load() == 3);
    }
    GTEST_LOG_(INFO) << name + " end";
}

/**
 * @tc.number: SerialTaskDispatcher_DelayDispatchTest_002
 * @tc.name: DelayDispatch
 * @tc.desc: Test SerialTaskDispatcher delay dispatch.
 */
HWTEST_F(SerialTaskDispatcherModuleTest, SerialTaskDispatcher_DelayDispatchTest_002, TestSize.Level0)
{
    auto name = std::string("SerialTaskDispatcher_DelayDispatchTest_003");
    GTEST_LOG_(INFO) << name + " start";
    std::shared_ptr<SerialTaskDispatcher> dispatcher = CreateMtSerialTaskDispatcher(name, TaskPriority::DEFAULT);
    int sleep1 = 2000;
    dispatcher->DelayDispatch(nullptr, sleep1);
    GTEST_LOG_(INFO) << name + " end";
}

/**
 * @tc.number: SerialTaskDispatcher_DelayDispatchTest_003
 * @tc.name: DelayDispatch
 * @tc.desc: Test SerialTaskDispatcher delay dispatch.
 */
HWTEST_F(SerialTaskDispatcherModuleTest, SerialTaskDispatcher_DelayDispatchTest_003, TestSize.Level0)
{
    auto name = std::string("SerialTaskDispatcher_DelayDispatchTest_004");
    GTEST_LOG_(INFO) << name + " start";
    std::shared_ptr<SerialTaskDispatcher> dispatcher = CreateMtSerialTaskDispatcher(name, TaskPriority::DEFAULT);
    std::atomic<int> count(0);
    int sleep1 = -2000;
    dispatcher->DelayDispatch(std::make_shared<Runnable>([&count, sleep = sleep1]() {
        int index = count.fetch_add(1);
        EXPECT_TRUE(index == 1 || index == 0);
        GTEST_LOG_(INFO) << "SerialTaskDispatcher_DelayDispatchTest_004 task" + std::to_string(index) + "end, elapsed" +
                                std::to_string(sleep) + " ms";
    }),
        sleep1);

    int sleep2 = -1000;
    dispatcher->DelayDispatch(std::make_shared<Runnable>([&count, sleep = sleep2]() {
        int index = count.fetch_add(1);
        EXPECT_TRUE(index == 0 || index == 1);
        GTEST_LOG_(INFO) << "SerialTaskDispatcher_DelayDispatchTest_004 task" + std::to_string(index) + "end, elapsed" +
                                std::to_string(sleep) + " ms";
    }),
        sleep2);
    {
        long wait = 100;
        GTEST_LOG_(INFO) << ("wait for task, wait= " + std::to_string(wait) + " ms");
        auto time = std::chrono::milliseconds(wait);
        std::this_thread::sleep_for(time);
        EXPECT_TRUE(count.load() == 2);
    }
    GTEST_LOG_(INFO) << name + " end";
}

/**
 * @tc.number: SerialTaskDispatcher_ApplyDispatchTest_001
 * @tc.name: ApplyDispatch
 * @tc.desc: Test SerialTaskDispatcher applay dispatch.
 */
HWTEST_F(SerialTaskDispatcherModuleTest, SerialTaskDispatcher_ApplyDispatchTest_001, TestSize.Level0)
{
    auto name = std::string("SerialTaskDispatcher_ApplyDispatchTest_001");
    GTEST_LOG_(INFO) << name + " start";
    std::shared_ptr<SerialTaskDispatcher> ptr = CreateMtSerialTaskDispatcher(name, TaskPriority::DEFAULT);
    std::atomic<int> count(0);
    long sleep1 = 101;
    auto repeats1 = std::make_shared<IteratableTask<long>>([&](long index) {
        auto time = std::chrono::milliseconds(sleep1);
        std::this_thread::sleep_for(time);
        count.fetch_add(1);
        GTEST_LOG_(INFO) << name << "  repeats1 sleep1=" << sleep1;
    });
    ptr->ApplyDispatch(repeats1, 10);

    long sleep2 = 100;
    auto repeats2 = std::make_shared<IteratableTask<long>>([&](long index) {
        auto time = std::chrono::milliseconds(sleep2);
        std::this_thread::sleep_for(time);
        count.fetch_add(1);
        GTEST_LOG_(INFO) << name << "  repeats2 sleep2=" << sleep2;
    });
    ptr->ApplyDispatch(repeats2, 10);

    EXPECT_TRUE(count.load() < 20);
    long wait = sleep1 * 10 + sleep2 * 10 + 100 + 100;
    GTEST_LOG_(INFO) << name << "  main wait time=" << wait;
    auto time = std::chrono::milliseconds(wait);
    std::this_thread::sleep_for(time);
    EXPECT_TRUE(count.load() == 20);
    GTEST_LOG_(INFO) << name + " end";
}

/**
 * @tc.number: SerialTaskDispatcher_AsyncDispatchBarrierTest_001
 * @tc.name: AsyncDispatchBarrier
 * @tc.desc: Test SerialTaskDispatcher barrier.
 */
HWTEST_F(SerialTaskDispatcherModuleTest, SerialTaskDispatcher_AsyncDispatchBarrierTest_001, TestSize.Level0)
{
    auto name = std::string("SerialTaskDispatcher_AsyncDispatchBarrierTest_001");
    GTEST_LOG_(INFO) << name << " start";
    std::shared_ptr<SerialTaskDispatcher> ptr = CreateMtSerialTaskDispatcher(name, TaskPriority::DEFAULT);
    std::atomic<int> count(0);
    EXPECT_TRUE(count.load() == 0);
    auto group = ptr->CreateDispatchGroup();
    long sleepG1 = 200;
    ptr->AsyncGroupDispatch((group), std::make_shared<Runnable>([&]() {
        auto time = std::chrono::milliseconds(sleepG1);
        std::this_thread::sleep_for(time);
        int index = count.fetch_add(1);
        EXPECT_TRUE(index == 1 || index == 0);
        GTEST_LOG_(INFO) << name << "group Runnable1";
    }));
    long sleepG2 = 100;
    ptr->AsyncDispatch(std::make_shared<Runnable>([&]() {
        auto time = std::chrono::milliseconds(sleepG2);
        std::this_thread::sleep_for(time);
        int index = count.fetch_add(1);
        EXPECT_TRUE(index == 1 || index == 0);
        GTEST_LOG_(INFO) << name << "group Runnable2";
    }));
    EXPECT_TRUE(count.load() == 0);
    ptr->AsyncDispatchBarrier(std::make_shared<Runnable>([&]() {
        EXPECT_TRUE(count.load() == 2);
        count.fetch_add(1);
        GTEST_LOG_(INFO) << name << "AsyncDispatchBarrier Runnable";
    }));
    ptr->AsyncDispatch(std::make_shared<Runnable>([&]() {
        count.fetch_add(1);
        GTEST_LOG_(INFO) << name << "group Runnable2";
    }));
    GTEST_LOG_(INFO) << name << "AsyncDispatchBarrier After";

    auto time = std::chrono::milliseconds(1000);
    std::this_thread::sleep_for(time);
    EXPECT_TRUE(count.load() == 4);
    GTEST_LOG_(INFO) << name << " end";
}

/**
 * @tc.number: SerialTaskDispatcher_SyncDispatchBarrierTest_001
 * @tc.name: SyncDispatchBarrier
 * @tc.desc: Test SerialTaskDispatcher sync barrier.
 */
HWTEST_F(SerialTaskDispatcherModuleTest, SerialTaskDispatcher_SyncDispatchBarrierTest_001, TestSize.Level0)
{
    auto name = std::string("SerialTaskDispatcher_SyncDispatchBarrierTest_001");
    GTEST_LOG_(INFO) << name << " start";
    std::shared_ptr<SerialTaskDispatcher> ptr = CreateMtSerialTaskDispatcher(name, TaskPriority::DEFAULT);
    std::atomic<int> count(0);
    EXPECT_TRUE(count.load() == 0);
    auto group = ptr->CreateDispatchGroup();
    long sleepG1 = 200;
    ptr->AsyncGroupDispatch((group), std::make_shared<Runnable>([&]() {
        auto time = std::chrono::milliseconds(sleepG1);
        std::this_thread::sleep_for(time);
        int index = count.fetch_add(1);
        EXPECT_TRUE(index == 1 || index == 0);
        GTEST_LOG_(INFO) << name << "group Runnable1";
    }));
    long sleepG2 = 100;
    ptr->AsyncDispatch(std::make_shared<Runnable>([&]() {
        auto time = std::chrono::milliseconds(sleepG2);
        std::this_thread::sleep_for(time);
        int index = count.fetch_add(1);
        EXPECT_TRUE(index == 1 || index == 0);
        GTEST_LOG_(INFO) << name << "AsyncDispatch Runnable2";
    }));
    EXPECT_TRUE(count.load() == 0);
    ptr->SyncDispatchBarrier(std::make_shared<Runnable>([&]() {
        EXPECT_TRUE(count.load() == 2);
        count.fetch_add(1);
        GTEST_LOG_(INFO) << name << "SyncDispatchBarrier Runnable";
    }));
    ptr->SyncDispatch(std::make_shared<Runnable>([&]() {
        count.fetch_add(1);
        GTEST_LOG_(INFO) << name << "SyncDispatch Runnable2";
    }));
    GTEST_LOG_(INFO) << name << "SyncDispatchBarrier After";

    auto time = std::chrono::milliseconds(1000);
    std::this_thread::sleep_for(time);
    EXPECT_TRUE(count.load() == 4);
    GTEST_LOG_(INFO) << name << " end";
}
