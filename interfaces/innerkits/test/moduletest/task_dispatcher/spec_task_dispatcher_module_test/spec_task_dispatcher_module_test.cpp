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
#include "task_dispatcher_context.h"
#include "task_dispatcher.h"
#include "spec_task_dispatcher.h"

using namespace testing::ext;
using namespace OHOS::AppExecFwk;

class SpecTaskDispatcherModuleTest : public testing::Test {
public:
    static void SetUpTestCase();
    static void TearDownTestCase();
    void SetUp();
    void TearDown();
    std::shared_ptr<SpecTaskDispatcher> ptrSpecTaskDispatcher;
};

void SpecTaskDispatcherModuleTest::SetUpTestCase()
{}

void SpecTaskDispatcherModuleTest::TearDownTestCase()
{}

void SpecTaskDispatcherModuleTest::SetUp()
{
    std::string dispatcherName = SpecDispatcherConfig::MAIN;
    TaskPriority taskPriority = TaskPriority::HIGH;
    std::shared_ptr<SpecDispatcherConfig> config = std::make_shared<SpecDispatcherConfig>(dispatcherName, taskPriority);
    std::shared_ptr<EventRunner> runner = EventRunner::Create(true);
    ptrSpecTaskDispatcher = std::make_shared<SpecTaskDispatcher>(config, runner);
}

void SpecTaskDispatcherModuleTest::TearDown()
{
    ptrSpecTaskDispatcher = nullptr;
}

/**
 * @tc.number: SpecTaskDispatcher_SyncDispatchTest_001
 * @tc.name: SyncDispatch
 * @tc.desc: Test SpecTaskDispatcher sync dispatch.
 */
HWTEST_F(SpecTaskDispatcherModuleTest, SpecTaskDispatcher_SyncDispatchTest_001, TestSize.Level1)
{
    auto name = std::string("SpecTaskDispatcher_SyncDispatchTest_001");
    GTEST_LOG_(INFO) << name << " start";

    std::atomic<int> count(0);
    EXPECT_TRUE(count.load() == 0);
    long sleep1 = 200;
    ptrSpecTaskDispatcher->SyncDispatch(std::make_shared<Runnable>([&]() {
        auto time = std::chrono::milliseconds(sleep1);
        std::this_thread::sleep_for(time);
        int index = count.fetch_add(1);
        EXPECT_TRUE(index == 0);
        GTEST_LOG_(INFO) << name << " SyncDispatch1  : sleep1 time=" << sleep1;
    }));
    // serial execute
    EXPECT_TRUE(count.load() == 1);

    long sleep2 = 100;
    ptrSpecTaskDispatcher->SyncDispatch(std::make_shared<Runnable>([&]() {
        auto time = std::chrono::milliseconds(sleep2);
        std::this_thread::sleep_for(time);
        int index = count.fetch_add(1);
        EXPECT_TRUE(index == 1);
        GTEST_LOG_(INFO) << name << " SyncDispatch2  : sleep2 time=" << sleep2;
    }));
    // serial execute
    EXPECT_TRUE(count.load() == 2);
    long wait = sleep1 + sleep2 + 100;
    auto time = std::chrono::milliseconds(wait);
    std::this_thread::sleep_for(time);
    EXPECT_TRUE(count.load() == 2);
    GTEST_LOG_(INFO) << name << " end";
}

/**
 * @tc.number: SpecTaskDispatcher_AsyncDispatchTest_001
 * @tc.name: AsyncDispatch
 * @tc.desc: Test SpecTaskDispatcher async dispatch.
 */
HWTEST_F(SpecTaskDispatcherModuleTest, SpecTaskDispatcher_AsyncDispatchTest_001, TestSize.Level1)
{
    auto name = std::string("SpecTaskDispatcher_AsyncDispatchTest_001");
    GTEST_LOG_(INFO) << name + " start";

    std::atomic<int> count(0);
    long sleep1 = 200;
    ptrSpecTaskDispatcher->AsyncDispatch(std::make_shared<Runnable>([&]() {
        auto time = std::chrono::milliseconds(sleep1);
        std::this_thread::sleep_for(time);
        int index = count.fetch_add(1);
        EXPECT_TRUE(index == 0);
        GTEST_LOG_(INFO) << name << " AsyncDispatch1  : sleep2 time=" << sleep1;
    }));
    // async execute
    EXPECT_TRUE(count.load() < 1);
    long sleep2 = 100;
    ptrSpecTaskDispatcher->AsyncDispatch(std::make_shared<Runnable>([&]() {
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

void DelayDispatcherTestWait(std::atomic<int> &count){
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
}

/**
 * @tc.number: SpecTaskDispatcher_DelayDispatchTest_001
 * @tc.name: DelayDispatch
 * @tc.desc: Test SpecTaskDispatcher delay dispatch.
 */
HWTEST_F(SpecTaskDispatcherModuleTest, SpecTaskDispatcher_DelayDispatchTest_001, TestSize.Level0)
{
    auto name = std::string("SpecTaskDispatcher_DelayDispatchTest_001");
    GTEST_LOG_(INFO) << name + " start";

    std::atomic<int> count(0);
    int sleep1 = 2000;
    const std::shared_ptr<Runnable> runnanle = std::make_shared<Runnable>([&count, sleep = sleep1]() {
        int index = count.fetch_add(1);
        EXPECT_TRUE(index == 2);
        GTEST_LOG_(INFO) << "SpecTaskDispatcher_DelayDispatchTest_001 task" + std::to_string(index) + "end, elapsed" +
                                std::to_string(sleep) + " ms";
    });
    ptrSpecTaskDispatcher->DelayDispatch(runnanle, sleep1);
    EXPECT_TRUE(count.load() == 0);

    int sleep2 = 1000;
    ptrSpecTaskDispatcher->DelayDispatch(std::make_shared<Runnable>([&count, sleep = sleep2]() {
        int index = count.fetch_add(1);
        EXPECT_TRUE(index == 0 || index == 1);
        GTEST_LOG_(INFO) << "SpecTaskDispatcher_DelayDispatchTest_001 task" + std::to_string(index) + "end, elapsed" +
                                std::to_string(sleep) + " ms";
    }),
        sleep2);
    EXPECT_TRUE(count.load() == 0);

    int sleep3 = 1000;
    ptrSpecTaskDispatcher->DelayDispatch(std::make_shared<Runnable>([&count, sleep = sleep3]() {
        int index = count.fetch_add(1);
        EXPECT_TRUE(index == 0 || index == 1);
        GTEST_LOG_(INFO) << "SpecTaskDispatcher_DelayDispatchTest_001 task" + std::to_string(index) + "end, elapsed" +
                                std::to_string(sleep) + " ms";
    }),
        sleep3);
    EXPECT_TRUE(count.load() == 0);
    DelayDispatcherTestWait(count);
    GTEST_LOG_(INFO) << name + " end";
}

/**
 * @tc.number: SpecTaskDispatcher_DelayDispatchTest_002
 * @tc.name: DelayDispatch
 * @tc.desc: Test SpecTaskDispatcher delay dispatch.
 */
HWTEST_F(SpecTaskDispatcherModuleTest, SpecTaskDispatcher_DelayDispatchTest_002, TestSize.Level0)
{
    auto name = std::string("SpecTaskDispatcher_DelayDispatchTest_002");
    GTEST_LOG_(INFO) << name + " start";

    int sleep1 = 2000;
    ptrSpecTaskDispatcher->DelayDispatch(std::make_shared<Runnable>(nullptr), sleep1);
    GTEST_LOG_(INFO) << name + " end";
}

/**
 * @tc.number: SpecTaskDispatcher_DelayDispatchTest_003
 * @tc.name: DelayDispatch
 * @tc.desc: Test SpecTaskDispatcher delay dispatch.
 */
HWTEST_F(SpecTaskDispatcherModuleTest, SpecTaskDispatcher_DelayDispatchTest_003, TestSize.Level0)
{
    auto name = std::string("SpecTaskDispatcher_DelayDispatchTest_003");
    GTEST_LOG_(INFO) << name + " start";

    int sleep1 = 2000;
    ptrSpecTaskDispatcher->DelayDispatch(nullptr, sleep1);
    GTEST_LOG_(INFO) << name + " end";
}

/**
 * @tc.number: SpecTaskDispatcher_DelayDispatchTest_004
 * @tc.name: DelayDispatch
 * @tc.desc: Test SpecTaskDispatcher delay dispatch.
 */
HWTEST_F(SpecTaskDispatcherModuleTest, SpecTaskDispatcher_DelayDispatchTest_004, TestSize.Level0)
{
    auto name = std::string("SpecTaskDispatcher_DelayDispatchTest_004");
    GTEST_LOG_(INFO) << name + " start";

    std::atomic<int> count(0);
    int sleep1 = -2000;
    ptrSpecTaskDispatcher->DelayDispatch(std::make_shared<Runnable>([&count, sleep = sleep1]() {
        int index = count.fetch_add(1);
        EXPECT_TRUE(index == 1 || index == 0);
        GTEST_LOG_(INFO) << "SpecTaskDispatcher_DelayDispatchTest_004 task" + std::to_string(index) + "end, elapsed" +
                                std::to_string(sleep) + " ms";
    }),
        sleep1);

    int sleep2 = -1000;
    ptrSpecTaskDispatcher->DelayDispatch(std::make_shared<Runnable>([&count, sleep = sleep2]() {
        int index = count.fetch_add(1);
        EXPECT_TRUE(index == 0 || index == 1);
        GTEST_LOG_(INFO) << "SpecTaskDispatcher_DelayDispatchTest_004 task" + std::to_string(index) + "end, elapsed" +
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
 * @tc.number: SpecTaskDispatcher_ApplyDispatchTest_001
 * @tc.name: ApplyDispatch
 * @tc.desc: Test SpecTaskDispatcher apply dispatch.
 */
HWTEST_F(SpecTaskDispatcherModuleTest, SpecTaskDispatcher_ApplyDispatchTest_001, TestSize.Level0)
{
    auto name = std::string("SpecTaskDispatcher_ApplyDispatchTest_001");
    GTEST_LOG_(INFO) << name + " start";

    std::atomic<int> count(0);
    long sleep1 = 101;
    auto repeats1 = std::make_shared<IteratableTask<long>>([&](long index) {
        auto time = std::chrono::milliseconds(sleep1);
        std::this_thread::sleep_for(time);
        count.fetch_add(1);
        GTEST_LOG_(INFO) << name << "  repeats1 sleep1=" << sleep1;
    });
    ptrSpecTaskDispatcher->ApplyDispatch(repeats1, 10);

    long sleep2 = 100;
    auto repeats2 = std::make_shared<IteratableTask<long>>([&](long index) {
        auto time = std::chrono::milliseconds(sleep2);
        std::this_thread::sleep_for(time);
        count.fetch_add(1);
        GTEST_LOG_(INFO) << name << "  repeats2 sleep2=" << sleep2;
    });
    ptrSpecTaskDispatcher->ApplyDispatch(repeats2, 10);

    // async execute
    EXPECT_TRUE(count.load() < 20);
    long wait = sleep1 * 10 + sleep2 * 10 + 100 + 100;
    GTEST_LOG_(INFO) << name << "  main wait time=" << wait;
    auto time = std::chrono::milliseconds(wait);
    std::this_thread::sleep_for(time);
    EXPECT_TRUE(count.load() == 20);
    GTEST_LOG_(INFO) << name + " end";
}