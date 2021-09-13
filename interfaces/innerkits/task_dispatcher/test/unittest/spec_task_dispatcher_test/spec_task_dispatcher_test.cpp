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

#define private public
#undef private
#include "spec_task_dispatcher.h"
#include <thread>
#include <unistd.h>
#include <string>
#include <gtest/gtest.h>
#include "task_dispatcher_context.h"
#include "task_dispatcher.h"
#include "serial_task_dispatcher.h"
#include "runnable.h"
#include "event_runner.h"

using namespace testing;
using namespace testing::ext;
using namespace OHOS::AppExecFwk;

using OHOS::AppExecFwk::DefaultWorkerPoolConfig;
using OHOS::AppExecFwk::IteratableTask;
using OHOS::AppExecFwk::Revocable;
using OHOS::AppExecFwk::Runnable;
using OHOS::AppExecFwk ::SpecTaskDispatcher;
using OHOS::AppExecFwk::TaskDispatcherContext;
using OHOS::AppExecFwk::TaskExecutor;
using OHOS::AppExecFwk::TaskPriority;
using OHOS::AppExecFwk::WorkerPoolConfig;

class SpecTaskDispatcherTest : public testing::Test {
public:
    static void SetUpTestCase();
    static void TearDownTestCase();
    void SetUp();
    void TearDown();
};

void SpecTaskDispatcherTest::SetUpTestCase()
{}

void SpecTaskDispatcherTest::TearDownTestCase()
{}

void SpecTaskDispatcherTest::SetUp()
{}

void SpecTaskDispatcherTest::TearDown()
{}
std::string Now()
{
    time_t now = std::time(0);
    char mbstr[10];
    std::strftime(mbstr, sizeof(mbstr), "%T", std::localtime(&now));
    return mbstr;
}

const std::string Prefix(const std::string &name)
{
    return std::string(">>> ") + Now() + std::string(" ") + name + std::string(": ");
}

std::shared_ptr<SpecTaskDispatcher> CreateSpecTaskDispatcher()
{
    std::string dispatcherName = SpecDispatcherConfig::MAIN;
    TaskPriority taskPriority = TaskPriority::HIGH;
    std::shared_ptr<SpecDispatcherConfig> config = std::make_shared<SpecDispatcherConfig>(dispatcherName, taskPriority);
    std::shared_ptr<EventRunner> runner = EventRunner::Create(true);
    std::shared_ptr<SpecTaskDispatcher> specTaskDispatcher = std::make_shared<SpecTaskDispatcher>(config, runner);
    GTEST_LOG_(INFO) << Prefix(dispatcherName) << " created";
    return specTaskDispatcher;
}

/**
 * @tc.number:SpecTaskDispatcher_0100
 * @tc.name: test SpecTaskDispatcher
 * @tc.desc: 1.create SpecTaskDispatcher
 *           2.GetPriority successfully
 */
HWTEST(SpecTaskDispatcherTest, SpecTaskDispatcher_0100, Function | MediumTest | Level1)
{
    GTEST_LOG_(INFO) << "SpecTaskDispatcher_0100 start";
    std::shared_ptr<SpecTaskDispatcher> ptr = CreateSpecTaskDispatcher();
    // same to func CreateSpecTaskDispatcher  TaskPriority
    EXPECT_EQ(ptr->GetPriority(), TaskPriority::HIGH);
    GTEST_LOG_(INFO) << "SpecTaskDispatcher_0100 end";
}

/**
 * @tc.number:SyncDispatch_0100
 * @tc.name: test SyncDispatch
 * @tc.desc: 1.create SpecTaskDispatcher
 *           2.SyncDispatch successfully
 */
HWTEST(SpecTaskDispatcherTest, SyncDispatch_0100, Function | MediumTest | Level1)
{
    GTEST_LOG_(INFO) << "SyncDispatch_0100 start";
    const std::string name("SyncDispatch_0100");
    GTEST_LOG_(INFO) << Prefix(name) << "begin thread " << std::this_thread::get_id();
    std::shared_ptr<SpecTaskDispatcher> ptr = CreateSpecTaskDispatcher();

    // task runs synchronized
    // use elapsed time to identify a task.
    std::atomic<int> count(0);
    EXPECT_TRUE(count.load() == 0);
    int sleep1 = 2000;
    ptr->SyncDispatch(std::make_shared<Runnable>([&count, sleep = sleep1]() {
        auto time = std::chrono::milliseconds(sleep);
        std::this_thread::sleep_for(time);
        int index = count.fetch_add(1);
        EXPECT_TRUE(index == 0);
        APP_LOGD("task %{public}d end, elapsed %{public}d ms", index, sleep);
    }));
    EXPECT_TRUE(count.load() == 1);

    int sleep2 = 100;
    ptr->SyncDispatch(std::make_shared<Runnable>([&count, sleep = sleep2]() {
        auto time = std::chrono::milliseconds(sleep);
        std::this_thread::sleep_for(time);
        int index = count.fetch_add(1);
        EXPECT_TRUE(index == 1);
        APP_LOGD("task %{public}d end, elapsed %{public}d ms", index, sleep);
    }));
    EXPECT_TRUE(count.load() == 2);

    int sleep3 = 1000;
    ptr->SyncDispatch(std::make_shared<Runnable>([&count, sleep = sleep3]() {
        auto time = std::chrono::milliseconds(sleep);
        std::this_thread::sleep_for(time);
        int index = count.fetch_add(1);
        EXPECT_TRUE(index == 2);
        APP_LOGD("task %{public}d end, elapsed %{public}d ms", index, sleep);
    }));
    EXPECT_TRUE(count.load() == 3);

    long wait = 1000;
    GTEST_LOG_(INFO) << ("wait for " + std::to_string(wait));
    auto time = std::chrono::milliseconds(wait);
    std::this_thread::sleep_for(time);

    GTEST_LOG_(INFO) << "SyncDispatch_0100 end";
}
/**
 * @tc.number:SyncDispatch_0200
 * @tc.name: test SyncDispatch
 * @tc.desc: 1.create SpecTaskDispatcher
 *           2.SyncDispatch Runnable is nullptr
 */
HWTEST(SpecTaskDispatcherTest, SyncDispatch_0200, Function | MediumTest | Level1)
{
    GTEST_LOG_(INFO) << "SyncDispatch_0200 start";
    const std::string name("SyncDispatch_0200");
    std::shared_ptr<SpecTaskDispatcher> ptr = CreateSpecTaskDispatcher();
    ptr->SyncDispatch(nullptr);
    GTEST_LOG_(INFO) << "SyncDispatch_0200 end";
}
/**
 * @tc.number:AsyncDispatch_0100
 * @tc.name: test AsyncDispatch
 * @tc.desc: 1.create SpecTaskDispatcher
 *           2.AsyncDispatch successfully
 */
HWTEST(SpecTaskDispatcherTest, AsyncDispatch_0100, Function | MediumTest | Level1)
{
    GTEST_LOG_(INFO) << "AsyncDispatch_0100 start";
    const std::string name("AsyncDispatch_0100");
    GTEST_LOG_(INFO) << Prefix(name) << "begin thread " << std::this_thread::get_id();
    std::shared_ptr<SpecTaskDispatcher> ptr = CreateSpecTaskDispatcher();
    // init
    // task runs synchronized
    // use elapsed time to identify a task.
    std::atomic<int> count(0);
    EXPECT_TRUE(count.load() == 0);
    int sleep1 = 2000;
    ptr->AsyncDispatch(std::make_shared<Runnable>([&count, sleep = sleep1]() {
        auto time = std::chrono::milliseconds(sleep);
        std::this_thread::sleep_for(time);
        int index = count.fetch_add(1);
        EXPECT_TRUE(index == 0);
        APP_LOGD("task %{public}d end, elapsed %{public}d ms", index, sleep);
    }));
    EXPECT_TRUE(count.load() == 0);

    int sleep2 = 1000;
    ptr->AsyncDispatch(std::make_shared<Runnable>([&count, sleep = sleep2]() {
        auto time = std::chrono::milliseconds(sleep);
        std::this_thread::sleep_for(time);
        int index = count.fetch_add(1);
        EXPECT_TRUE(index == 1);
        APP_LOGD("task %{public}d end, elapsed %{public}d ms", index, sleep);
    }));
    EXPECT_TRUE(count.load() == 0);

    int sleep3 = 1000;
    ptr->AsyncDispatch(std::make_shared<Runnable>([&count, sleep = sleep3]() {
        auto time = std::chrono::milliseconds(sleep);
        std::this_thread::sleep_for(time);
        int index = count.fetch_add(1);
        EXPECT_TRUE(index == 2);
        APP_LOGD("task %{public}d end, elapsed %{public}d ms", index, sleep);
    }));
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
        EXPECT_TRUE(count.load() == 0);
    }
    {
        long wait = 1000;
        GTEST_LOG_(INFO) << ("wait for task, wait= " + std::to_string(wait) + " ms");
        auto time = std::chrono::milliseconds(wait);
        std::this_thread::sleep_for(time);
        EXPECT_TRUE(count.load() == 1);
    }
    {
        long wait = 1000;
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

    GTEST_LOG_(INFO) << "AsyncDispatch_0100 end";
}
/**
 * @tc.number:AsyncDispatch_0200
 * @tc.name: test AsyncDispatch
 * @tc.desc: 1.create SpecTaskDispatcher
 *           2.AsyncDispatch Runnable is nullptr
 */
HWTEST(SpecTaskDispatcherTest, AsyncDispatch_0200, Function | MediumTest | Level1)
{
    GTEST_LOG_(INFO) << "AsyncDispatch_0200 start";
    const std::string name("AsyncDispatch_0200");
    std::shared_ptr<SpecTaskDispatcher> ptr = CreateSpecTaskDispatcher();
    ptr->AsyncDispatch(nullptr);
    GTEST_LOG_(INFO) << "AsyncDispatch_0200 end";
}
/**
 * @tc.number:DelayDispatch_0100
 * @tc.name: test DelayDispatch
 * @tc.desc: 1.create SpecTaskDispatcher
 *           2.DelayDispatch successfully
 */
HWTEST(SpecTaskDispatcherTest, DelayDispatch_0100, Function | MediumTest | Level1)
{
    GTEST_LOG_(INFO) << "DelayDispatch_0100 start";
    const std::string name("DelayDispatch");
    GTEST_LOG_(INFO) << Prefix(name) << "begin thread " << std::this_thread::get_id();
    std::shared_ptr<SpecTaskDispatcher> ptr = CreateSpecTaskDispatcher();

    // task runs delayed
    // use elapsed time to identify a task.
    std::atomic<int> count(0);
    int sleep1 = 2000;
    ptr->DelayDispatch(std::make_shared<Runnable>([&count, sleep = sleep1]() {
        // execute second
        int index = count.fetch_add(1);
        EXPECT_TRUE(index == 2);
        APP_LOGD("task %{public}d end, elapsed %{public}d ms", index, sleep);
    }),
        sleep1);
    EXPECT_TRUE(count.load() == 0);

    int sleep2 = 1000;
    ptr->DelayDispatch(std::make_shared<Runnable>([&count, sleep = sleep2]() {
        // execute first
        int index = count.fetch_add(1);
        EXPECT_TRUE(index == 0 || index == 1);
        APP_LOGD("task %{public}d end, elapsed %{public}d ms", index, sleep);
    }),
        sleep2);
    EXPECT_TRUE(count.load() == 0);

    int sleep3 = 1000;
    ptr->DelayDispatch(std::make_shared<Runnable>([&count, sleep = sleep3]() {
        // execute first
        int index = count.fetch_add(1);
        EXPECT_TRUE(index == 0 || index == 1);
        APP_LOGD("task %{public}d end, elapsed %{public}d ms", index, sleep);
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
    GTEST_LOG_(INFO) << "DelayDispatch_0100 end";
}

/**
 * @tc.number:DelayDispatch_0200
 * @tc.name: test DelayDispatch
 * @tc.desc: 1.create SpecTaskDispatcher
 *           2.DelayDispatch Runnable is nullptr
 */
HWTEST(SpecTaskDispatcherTest, DelayDispatch_0200, Function | MediumTest | Level1)
{
    GTEST_LOG_(INFO) << "DelayDispatch_0200 start";
    const std::string name("DelayDispatch_0200");
    std::shared_ptr<SpecTaskDispatcher> ptr = CreateSpecTaskDispatcher();
    ptr->DelayDispatch(nullptr, 200);
    GTEST_LOG_(INFO) << "DelayDispatch_0200 end";
}