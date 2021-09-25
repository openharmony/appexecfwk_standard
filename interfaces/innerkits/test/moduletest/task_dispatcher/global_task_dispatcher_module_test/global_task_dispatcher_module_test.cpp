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

using namespace testing;
using namespace testing::ext;
using namespace OHOS::AppExecFwk;

class GlobalTaskDispatcherModuleTest : public testing::Test {
public:
    static void SetUpTestCase();
    static void TearDownTestCase();
    void SetUp();
    void TearDown();
    std::shared_ptr<TaskDispatcher> GetMtGlobalTaskDispatcher(TaskPriority priority) const;
    std::shared_ptr<TaskDispatcherContext> context;
};

void GlobalTaskDispatcherModuleTest::SetUpTestCase()
{}

void GlobalTaskDispatcherModuleTest::TearDownTestCase()
{}

void GlobalTaskDispatcherModuleTest::SetUp()
{
    context = std::make_shared<TaskDispatcherContext>();
    printf("GlobalTaskDispatcherModuleTest::SetUp()\r\n");
}

void GlobalTaskDispatcherModuleTest::TearDown()
{
    printf("GlobalTaskDispatcherModuleTest::TearDown()\r\n");
    context = nullptr;
    printf("GlobalTaskDispatcherModuleTest::TearDown() after \r\n");
}

std::shared_ptr<TaskDispatcher> GlobalTaskDispatcherModuleTest::GetMtGlobalTaskDispatcher(TaskPriority priority) const
{
    std::shared_ptr<TaskDispatcher> ptrGlobalTaskDispatcher = context->GetGlobalTaskDispatcher(priority);
    return ptrGlobalTaskDispatcher;
}

/**
 * @tc.number: GetMtGlobalTaskDispatcherTest_001
 * @tc.name: GetGlobalTaskDispatcher
 * @tc.desc: Test GlobalTaskDispatcher get GlobalTaskDispatcher.
 */
HWTEST_F(GlobalTaskDispatcherModuleTest, GetMtGlobalTaskDispatcherTest_001, TestSize.Level1)
{
    auto name = std::string("GetMtGlobalTaskDispatcherTest_001");
    GTEST_LOG_(INFO) << name + " start";
    TaskPriority defaultPriority = TaskPriority::DEFAULT;
    std::shared_ptr<TaskDispatcher> defaultPtr1 = GetMtGlobalTaskDispatcher(defaultPriority);
    std::shared_ptr<TaskDispatcher> defaultPtr2 = GetMtGlobalTaskDispatcher(defaultPriority);
    EXPECT_EQ(defaultPtr1.get(), defaultPtr2.get());
    std::shared_ptr<TaskDispatcher> lowPtr1 = GetMtGlobalTaskDispatcher(TaskPriority::LOW);
    std::shared_ptr<TaskDispatcher> lowPtr2 = GetMtGlobalTaskDispatcher(TaskPriority::LOW);
    EXPECT_NE(lowPtr1.get(), defaultPtr1.get());
    EXPECT_EQ(lowPtr1.get(), lowPtr2.get());
    std::shared_ptr<TaskDispatcher> highPtr1 = GetMtGlobalTaskDispatcher(TaskPriority::HIGH);
    std::shared_ptr<TaskDispatcher> highPtr2 = GetMtGlobalTaskDispatcher(TaskPriority::HIGH);
    EXPECT_NE(defaultPtr1.get(), highPtr1.get());
    EXPECT_NE(lowPtr1.get(), highPtr1.get());
    EXPECT_EQ(highPtr1.get(), highPtr2.get());
    GTEST_LOG_(INFO) << name + " end";
}

/**
 * @tc.number: GlobalTaskDispatcher_SyncDispatchTest_001
 * @tc.name: SyncDispatch
 * @tc.desc: Test GlobalTaskDispatcher sync dispatch.
 */
HWTEST_F(GlobalTaskDispatcherModuleTest, GlobalTaskDispatcher_SyncDispatchTest_001, TestSize.Level1)
{
    auto name = std::string("GlobalTaskDispatcher_SyncDispatchTest_001");
    GTEST_LOG_(INFO) << name + " start";
    std::shared_ptr<TaskDispatcher> ptr = GetMtGlobalTaskDispatcher(TaskPriority::DEFAULT);
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
 * @tc.number: GlobalTaskDispatcher_AsyncDispatchTest_001
 * @tc.name: AsyncDispatch
 * @tc.desc: Test GlobalTaskDispatcher async dispatch.
 */
HWTEST_F(GlobalTaskDispatcherModuleTest, GlobalTaskDispatcher_AsyncDispatchTest_001, TestSize.Level0)
{
    auto name = std::string("GlobalTaskDispatcher_AsyncDispatchTest_001");
    GTEST_LOG_(INFO) << name << " start";
    std::shared_ptr<TaskDispatcher> ptr = GetMtGlobalTaskDispatcher(TaskPriority::DEFAULT);
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
 * @tc.number: GlobalTaskDispatcher_AsyncGroupDispatchTest_001
 * @tc.name: AsyncGroupDispatch
 * @tc.desc: Test GlobalTaskDispatcher group.
 */
HWTEST_F(GlobalTaskDispatcherModuleTest, GlobalTaskDispatcher_AsyncGroupDispatchTest_001, TestSize.Level1)
{
    auto name = std::string("GlobalTaskDispatcher_AsyncGroupDispatchTest_001");
    GTEST_LOG_(INFO) << name + " start";
    std::shared_ptr<TaskDispatcher> ptr = GetMtGlobalTaskDispatcher(TaskPriority::DEFAULT);
    // init
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
    EXPECT_TRUE(count.load() < 2);
    std::shared_ptr<Runnable> runnable = std::make_shared<Runnable>([&]() {
        std::this_thread::sleep_for(std::chrono::milliseconds(150));
        int index = count.fetch_add(1);
        EXPECT_TRUE(index == 2);
        GTEST_LOG_(INFO) << name << " GroupDispatchNotify Runnable";
    });
    ptr->GroupDispatchNotify(group, runnable);
    EXPECT_TRUE(ptr->GroupDispatchWait(group, 1000));
    std::this_thread::sleep_for(std::chrono::milliseconds(300));
    EXPECT_TRUE(count.load() == 3);
    GTEST_LOG_(INFO) << name + " end";
}

/**
 * @tc.number: GlobalTaskDispatcher_AsyncGroupDispatchTest_002
 * @tc.name: AsyncGroupDispatch
 * @tc.desc: Test GlobalTaskDispatcher group.
 */
HWTEST_F(GlobalTaskDispatcherModuleTest, GlobalTaskDispatcher_AsyncGroupDispatchTest_002, TestSize.Level1)
{
    auto name = std::string("GlobalTaskDispatcher_AsyncGroupDispatchTest_002");
    GTEST_LOG_(INFO) << name + " start";
    std::shared_ptr<TaskDispatcher> ptr = GetMtGlobalTaskDispatcher(TaskPriority::DEFAULT);
    long sleep1 = 200;
    std::shared_ptr<Group> group = ptr->CreateDispatchGroup();
    ptr->AsyncGroupDispatch(group, std::make_shared<Runnable>([&]() {
        auto time = std::chrono::milliseconds(sleep1);
        std::this_thread::sleep_for(time);
        GTEST_LOG_(INFO) << name << " AsyncGroupDispatch1  : sleep1 time=" << sleep1;
    }));

    long sleep2 = 100;
    ptr->AsyncGroupDispatch(group, std::make_shared<Runnable>([&]() {
        auto time = std::chrono::milliseconds(sleep2);
        std::this_thread::sleep_for(time);
        GTEST_LOG_(INFO) << name << " AsyncGroupDispatch1  : sleep2 time=" << sleep2;
    }));
    bool result = ptr->GroupDispatchWait(group, 10);
    EXPECT_EQ(false, result);
    GTEST_LOG_(INFO) << name + " end";
}

/**
 * @tc.number: GlobalTaskDispatcher_ApplyDispatchTest_001
 * @tc.name: ApplyDispatch
 * @tc.desc: Test GlobalTaskDispatcher apply dispatch.
 */
HWTEST_F(GlobalTaskDispatcherModuleTest, GlobalTaskDispatcher_ApplyDispatchTest_001, TestSize.Level0)
{
    auto name = std::string("GlobalTaskDispatcher_ApplyDispatchTest_001");
    GTEST_LOG_(INFO) << name << " start";
    std::shared_ptr<TaskDispatcher> ptr = GetMtGlobalTaskDispatcher(TaskPriority::DEFAULT);
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
 * @tc.number: GlobalTaskDispatcher_DelayDispatchTest_001
 * @tc.name: DelayDispatch
 * @tc.desc: Test GlobalTaskDispatcher delay dispatch.
 */
HWTEST_F(GlobalTaskDispatcherModuleTest, GlobalTaskDispatcher_DelayDispatchTest_001, TestSize.Level0)
{
    auto name = std::string("GlobalTaskDispatcher_DelayDispatchTest_001");
    GTEST_LOG_(INFO) << name << " start";
    std::shared_ptr<TaskDispatcher> ptr = GetMtGlobalTaskDispatcher(TaskPriority::DEFAULT);
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
    EXPECT_TRUE(count.load() < 2);
    long wait = 1000;
    auto time = std::chrono::milliseconds(wait);
    std::this_thread::sleep_for(time);
    EXPECT_TRUE(count.load() == 2);
    GTEST_LOG_(INFO) << name << " end";
}

/**
 * @tc.number: GlobalTaskDispatcher_SyncDispatchBarrierTest_001
 * @tc.name: AsyncDispatchBarrier
 * @tc.desc: Test GlobalTaskDispatcher barrier.
 */
HWTEST_F(GlobalTaskDispatcherModuleTest, GlobalTaskDispatcher_SyncDispatchBarrierTest_001, TestSize.Level0)
{
    auto name = std::string("GlobalTaskDispatcher_SyncDispatchBarrierTest_001");
    GTEST_LOG_(INFO) << name << " start";
    std::shared_ptr<TaskDispatcher> ptr = GetMtGlobalTaskDispatcher(TaskPriority::DEFAULT);
    std::atomic<int> count(0);
    EXPECT_TRUE(count.load() == 0);
    auto group = ptr->CreateDispatchGroup();
    long sleepG1 = 200;
    ptr->AsyncGroupDispatch((group), std::make_shared<Runnable>([&]() {
        auto time = std::chrono::milliseconds(sleepG1);
        std::this_thread::sleep_for(time);
        count.fetch_add(1);
        GTEST_LOG_(INFO) << name << "AsyncGroupDispatch Runnable";
    }));
    ptr->SyncDispatchBarrier(std::make_shared<Runnable>([&]() {
        count.fetch_add(1);
        GTEST_LOG_(INFO) << name << "SyncDispatchBarrier Runnable";
    }));
    ptr->SyncDispatch(std::make_shared<Runnable>([&]() {
        count.fetch_add(1);
        GTEST_LOG_(INFO) << name << "SyncDispatch Runnable";
    }));
    long wait = sleepG1 + 1000;
    auto time = std::chrono::milliseconds(wait);
    std::this_thread::sleep_for(time);
    EXPECT_TRUE(count.load() == 3);
    GTEST_LOG_(INFO) << name << " end";
}
