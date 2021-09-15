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
#include "base_task_dispatcher.h"
#include <gtest/gtest.h>
#include <string>
#include <thread>
#include <unistd.h>
#include "task_dispatcher_context.h"
#include "task_dispatcher.h"
#include "serial_task_dispatcher.h"
#include "runnable.h"

using namespace testing;
using namespace testing::ext;
using namespace OHOS::AppExecFwk;

using OHOS::AppExecFwk ::BaseTaskDispatcher;
using OHOS::AppExecFwk::DefaultWorkerPoolConfig;
using OHOS::AppExecFwk::Group;
using OHOS::AppExecFwk::GroupImpl;
using OHOS::AppExecFwk::IteratableTask;
using OHOS::AppExecFwk::Revocable;
using OHOS::AppExecFwk::Runnable;
using OHOS::AppExecFwk ::SerialTaskDispatcher;
using OHOS::AppExecFwk::TaskDispatcher;
using OHOS::AppExecFwk::TaskDispatcherContext;
using OHOS::AppExecFwk::TaskExecutor;
using OHOS::AppExecFwk::TaskPriority;
using OHOS::AppExecFwk::WorkerPoolConfig;

class BaseTaskDispatcherTest : public testing::Test {
public:
    void SetUp();
    void TearDown();
    std::shared_ptr<SerialTaskDispatcher> CreateSerialTaskDispatcher();
    std::shared_ptr<TaskDispatcherContext> context;
    std::shared_ptr<SerialTaskDispatcher> ptrSerialTaskDispatcher;
    static void SetUpTestCase();
    static void TearDownTestCase();
};

void BaseTaskDispatcherTest::SetUpTestCase()
{}

void BaseTaskDispatcherTest::TearDownTestCase()
{}

void BaseTaskDispatcherTest::SetUp()
{
    context = std::make_shared<TaskDispatcherContext>();
    std::string dispatcherName = "test-serial-dispatcher";
    TaskPriority taskPriority = TaskPriority::DEFAULT;
    ptrSerialTaskDispatcher = context->CreateSerialDispatcher(dispatcherName, taskPriority);
}

void BaseTaskDispatcherTest::TearDown()
{
    context = nullptr;
    ptrSerialTaskDispatcher = nullptr;
}

const std::string Prefix(const std::string &name)
{
    return std::string(">>> prefix :") + std::string(" ") + name + std::string(": ");
}

/**
 * @tc.number: CreateBaseTaskDispatcherTest_0100
 * @tc.name: test create BaseTaskDispatcher
 * @tc.desc: 1.create BaseTaskDispatcher
 *           2.get TaskPriority successfully
 */
HWTEST_F(BaseTaskDispatcherTest, CreateBaseTaskDispatcher_0100, Function | MediumTest | Level1)
{
    GTEST_LOG_(INFO) << "CreateBaseTaskDispatcher_0100 start";
    int priority = ptrSerialTaskDispatcher->GetPriority();
    EXPECT_EQ(priority, TaskPriority::DEFAULT);
    GTEST_LOG_(INFO) << "CreateBaseTaskDispatcher_0100 end";
}

/**
 * @tc.number: SyncDispatchBarrier_0100
 * @tc.name: test SyncDispatchBarrier
 * @tc.desc: 1.create SerialTaskDispatcher
 *           2.SyncDispatchBarrier successfully
 */
HWTEST_F(BaseTaskDispatcherTest, SyncDispatchBarrier_0100, Function | MediumTest | Level1)
{
    GTEST_LOG_(INFO) << "SyncDispatchBarrier_0100 start";
    const std::string name("SyncDispatchBarrier_0100");
    GTEST_LOG_(INFO) << Prefix(name) << "begin thread " << std::this_thread::get_id();
    // init
    std::atomic<int> count(0);
    EXPECT_EQ(count.load(), 0);
    long sleep1 = 200;
    ptrSerialTaskDispatcher->SyncDispatchBarrier(std::make_shared<Runnable>([&]() {
        auto time = std::chrono::milliseconds(sleep1);
        std::this_thread::sleep_for(time);
        int index = count.fetch_add(1);
        EXPECT_EQ(index, 0);
        GTEST_LOG_(INFO) << Prefix(name) << "SyncDispatchBarrier-" << 1 << " thread " << std::this_thread::get_id()
                         << ", sleep for " << sleep1 << " ms";
    }));

    // serial execute
    EXPECT_EQ(count.load(), 1);

    long sleep2 = 100;
    ptrSerialTaskDispatcher->SyncDispatchBarrier(std::make_shared<Runnable>([&]() {
        auto time = std::chrono::milliseconds(sleep2);
        std::this_thread::sleep_for(time);
        int index = count.fetch_add(1);
        EXPECT_EQ(index, 1);
        GTEST_LOG_(INFO) << Prefix(name) << "SyncDispatchBarrier-" << 2 << " thread " << std::this_thread::get_id()
                         << ", sleep for " << sleep2 << " ms";
    }));

    // serial execute
    EXPECT_EQ(count.load(), 2);
    long wait = sleep1 + sleep2 + 100;
    GTEST_LOG_(INFO) << Prefix(name) << "wait for " << (wait);
    auto time = std::chrono::milliseconds(wait);
    std::this_thread::sleep_for(time);
    EXPECT_EQ(count.load(), 2);
    GTEST_LOG_(INFO) << "sync thread end " << std::this_thread::get_id();
    GTEST_LOG_(INFO) << Prefix(name) << "end ";
    GTEST_LOG_(INFO) << "SyncDispatchBarrier_0100 end";
}
/**
 * @tc.number: AsyncDispatchBarrier_0100
 * @tc.name: test AsyncDispatchBarrier
 * @tc.desc: 1.create SerialTaskDispatcher
 * 2.AsyncDispatchBarrier successfully
 */
HWTEST_F(BaseTaskDispatcherTest, AsyncDispatchBarrier_0100, Function | MediumTest | Level1)
{
    GTEST_LOG_(INFO) << "AsyncDispatchBarrier_0100 start";
    const std::string name("AsyncDispatchBarrier_0100");
    GTEST_LOG_(INFO) << Prefix(name) << "begin thread " << std::this_thread::get_id();
    std::atomic<int> count(0);
    long sleep1 = 200;
    ptrSerialTaskDispatcher->AsyncDispatchBarrier(std::make_shared<Runnable>([&]() {
        auto time = std::chrono::milliseconds(sleep1);
        std::this_thread::sleep_for(time);
        int index = count.fetch_add(1);
        EXPECT_EQ(index, 0);
        GTEST_LOG_(INFO) << Prefix(name) << "AsyncDispatchBarrier-" << index << " thread " << std::this_thread::get_id()
                         << ", sleep for " << sleep1 << " ms";
    }));

    // async execute
    EXPECT_LT(count.load(), 1);

    long sleep2 = 100;
    ptrSerialTaskDispatcher->AsyncDispatchBarrier(std::make_shared<Runnable>([&]() {
        auto time = std::chrono::milliseconds(sleep2);
        std::this_thread::sleep_for(time);
        int index = count.fetch_add(1);
        EXPECT_EQ(index, 1);
        GTEST_LOG_(INFO) << Prefix(name) << "AsyncDispatchBarrier-" << index << " thread " << std::this_thread::get_id()
                         << ", sleep for " << sleep2 << " ms";
    }));
    EXPECT_LT(count.load(), 2);
    long wait = sleep1 + sleep2 + 100;
    GTEST_LOG_(INFO) << Prefix(name) << "wait for " << (wait);
    auto time = std::chrono::milliseconds(wait);
    std::this_thread::sleep_for(time);
    EXPECT_EQ(count.load(), 2);
    GTEST_LOG_(INFO) << Prefix(name) << "end ";
    GTEST_LOG_(INFO) << "AsyncDispatchBarrier_0100 end";
}

/**
 * @tc.number: CreateDispatchGroup_0100
 * @tc.name: test CreateDispatchGroup
 * @tc.desc: 1.create SerialTaskDispatcher
 *           2.CreateDispatchGroup successfully
 */
HWTEST_F(BaseTaskDispatcherTest, CreateDispatchGroup_0100, Function | MediumTest | Level1)
{
    GTEST_LOG_(INFO) << "CreateDispatchGroup_0100 start";
    std::shared_ptr<Group> gptr = ptrSerialTaskDispatcher->CreateDispatchGroup();
    EXPECT_TRUE(ptrSerialTaskDispatcher->GroupDispatchWait(gptr, 10));
    GTEST_LOG_(INFO) << "CreateDispatchGroup_0100 end";
}

/**
 * @tc.number: AsyncGroupDispatch_0100
 * @tc.name: test AsyncGroupDispatch
 * @tc.desc: 1.create SerialTaskDispatcher
 *           2.AsyncGroupDispatch successfully
 */
HWTEST_F(BaseTaskDispatcherTest, AsyncGroupDispatch_0100, Function | MediumTest | Level1)
{
    GTEST_LOG_(INFO) << "AsyncGroupDispatch_0100 start";
    const std::string name("AsyncGroupDispatch_0100");
    GTEST_LOG_(INFO) << Prefix(name) << "begin thread " << std::this_thread::get_id();
    std::shared_ptr<Group> gptr = ptrSerialTaskDispatcher->CreateDispatchGroup();
    std::atomic<int> count(0);
    long sleep1 = 200;
    ptrSerialTaskDispatcher->AsyncGroupDispatch(gptr, std::make_shared<Runnable>([&count, &sleep1, &name]() {
        auto time = std::chrono::milliseconds(sleep1);
        std::this_thread::sleep_for(time);
        int index = count.fetch_add(1);
        EXPECT_EQ(index, 0);
        GTEST_LOG_(INFO) << Prefix(name) << "AsyncGroupDispatch-" << index << " thread " << std::this_thread::get_id()
                         << ", sleep for " << sleep1 << " ms";
    }));

    // async execute
    EXPECT_LT(count.load(), 1);

    long sleep2 = 100;
    ptrSerialTaskDispatcher->AsyncGroupDispatch(gptr, std::make_shared<Runnable>([&count, &sleep2, &name]() {
        auto time = std::chrono::milliseconds(sleep2);
        std::this_thread::sleep_for(time);
        int index = count.fetch_add(1);
        EXPECT_EQ(index, 1);
        GTEST_LOG_(INFO) << Prefix(name) << "AsyncGroupDispatch-" << index << " thread " << std::this_thread::get_id()
                         << ", sleep for " << sleep2 << " ms";
    }));
    EXPECT_LT(count.load(), 2);
    long wait = sleep1 + sleep2 + 100;
    GTEST_LOG_(INFO) << Prefix(name) << "wait for " << (wait);
    auto time = std::chrono::milliseconds(wait);
    std::this_thread::sleep_for(time);
    EXPECT_EQ(count.load(), 2);
    GTEST_LOG_(INFO) << Prefix(name) << "end ";
    GTEST_LOG_(INFO) << "AsyncGroupDispatch_0100 end";
}

/**
 * @tc.number: ApplyDispatch_0100
 * @tc.name: test ApplyDispatch
 * @tc.desc: 1.create SerialTaskDispatcher
 *           2.ApplyDispatch successfully
 */
HWTEST_F(BaseTaskDispatcherTest, ApplyDispatch_0100, Function | MediumTest | Level1)
{
    GTEST_LOG_(INFO) << "ApplyDispatch_0100 start";

    const std::string name("ApplyDispatch_0100");
    GTEST_LOG_(INFO) << Prefix(name) << "begin thread " << std::this_thread::get_id();

    std::atomic<int> count(0);
    long sleep1 = 101;
    auto repeats1 = std::make_shared<IteratableTask<long>>([&](long index) {
        auto time = std::chrono::milliseconds(sleep1);
        std::this_thread::sleep_for(time);
        count.fetch_add(1);
        GTEST_LOG_(INFO) << Prefix(name) << "repeat1-" << index << ", thread " << std::this_thread::get_id()
                         << ", sleep for " << sleep1 << " ms";
    });
    ptrSerialTaskDispatcher->ApplyDispatch(repeats1, 10);

    long sleep2 = 100;
    auto repeats2 = std::make_shared<IteratableTask<long>>([&](long index) {
        auto time = std::chrono::milliseconds(sleep2);
        std::this_thread::sleep_for(time);
        count.fetch_add(1);
        GTEST_LOG_(INFO) << Prefix(name) << "repeat2-" << index << ", thread " << std::this_thread::get_id()
                         << ", sleep for " << sleep2 << " ms";
    });
    ptrSerialTaskDispatcher->ApplyDispatch(repeats2, 10);
    // async execute
    EXPECT_LT(count.load(), 20);
    long wait = sleep1 * 10 + sleep2 * 10 + 100 + 100;
    GTEST_LOG_(INFO) << Prefix(name) << "wait for " << (wait);
    auto time = std::chrono::milliseconds(wait);
    std::this_thread::sleep_for(time);
    EXPECT_EQ(count.load(), 20);
    GTEST_LOG_(INFO) << Prefix(name) << "end ";
    GTEST_LOG_(INFO) << "ApplyDispatch_0100 end";
}

/**
 * @tc.number: ApplyDispatch_0200
 * @tc.name: test ApplyDispatch
 * @tc.desc: 1.create SerialTaskDispatcher
 *           2.ApplyDispatch iterations <0
 */
HWTEST_F(BaseTaskDispatcherTest, ApplyDispatch_0200, Function | MediumTest | Level1)
{
    GTEST_LOG_(INFO) << "ApplyDispatch_0200 start";

    const std::string name("ApplyDispatch_0200");
    GTEST_LOG_(INFO) << Prefix(name) << "begin thread " << std::this_thread::get_id();
    ptrSerialTaskDispatcher->ApplyDispatch(nullptr, 10);
    GTEST_LOG_(INFO) << Prefix(name) << "end ";
    GTEST_LOG_(INFO) << "ApplyDispatch_0200 end";
}
/**
 * @tc.number: ApplyDispatch_0300
 * @tc.name: test ApplyDispatch
 * @tc.desc: 1.create SerialTaskDispatcher
 *           2.ApplyDispatch task is nullptr
 */
HWTEST_F(BaseTaskDispatcherTest, ApplyDispatch_0300, Function | MediumTest | Level1)
{
    GTEST_LOG_(INFO) << "ApplyDispatch_0300 start";

    const std::string name("ApplyDispatch_0300");
    GTEST_LOG_(INFO) << Prefix(name) << "begin thread " << std::this_thread::get_id();
    auto repeats1 = std::make_shared<IteratableTask<long>>(
        [&](long index) { GTEST_LOG_(INFO) << "Error:ApplyDispatch_0300 is run "; });
    ptrSerialTaskDispatcher->ApplyDispatch(repeats1, -1);
    GTEST_LOG_(INFO) << Prefix(name) << "end ";
    GTEST_LOG_(INFO) << "ApplyDispatch_0300 end";
}
/**
 * @tc.number:GroupDispatchWait_0100
 * @tc.name: test GroupDispatchWait
 * @tc.desc: 1.create SerialTaskDispatcher
 * 2.GroupDispatchWait successfully
 */
HWTEST_F(BaseTaskDispatcherTest, GroupDispatchWait_0100, Function | MediumTest | Level1)
{
    GTEST_LOG_(INFO) << "GroupDispatchWait_0100 start";
    std::shared_ptr<Group> gptr = ptrSerialTaskDispatcher->CreateDispatchGroup();
    EXPECT_TRUE(ptrSerialTaskDispatcher->GroupDispatchWait(gptr, 0));
    GTEST_LOG_(INFO) << "GroupDispatchWait_0100 end";
}

/**
 * @tc.number:GroupDispatchNotify_0100
 * @tc.name: test GroupDispatchNotify
 * @tc.desc: 1.create SerialTaskDispatcher
 * 2.GroupDispatchNotify successfully
 */
HWTEST_F(BaseTaskDispatcherTest, GroupDispatchNotify_0100, Function | MediumTest | Level1)
{
    GTEST_LOG_(INFO) << "GroupDispatchNotify_0100 start";
    std::atomic<int> count(0);
    std::shared_ptr<Group> gptr = ptrSerialTaskDispatcher->CreateDispatchGroup();
    count.fetch_add(1);
    std::function<void()> task = [&]() {
        count.fetch_add(1);
        GTEST_LOG_(INFO) << "GroupDispatchNotify_0100 ：notify task is run";
    };
    std::shared_ptr<Runnable> rptr = std::make_shared<Runnable>(task);
    EXPECT_EQ(count.load(), 1);
    GTEST_LOG_(INFO) << "GroupDispatchNotify_0100 end";
}

/**
 * @tc.number:GetPriority_0100
 * @tc.name: test GetPriority
 * @tc.desc: 1.create SerialTaskDispatcher
 *           2.GetPriority successfully
 */
HWTEST_F(BaseTaskDispatcherTest, GetPriority_0100, Function | MediumTest | Level1)
{
    GTEST_LOG_(INFO) << "GetPriority_0100 start";
    EXPECT_EQ(ptrSerialTaskDispatcher->GetPriority(), TaskPriority::DEFAULT);
    GTEST_LOG_(INFO) << "GetPriority_0100 end";
}
/**
 * @tc.number:TracePointBeforePost_0100
 * @tc.name:  TracePointBeforePost
 * @tc.desc:  1.create SerialTaskDispatcher
 *            2.TracePointBeforePost successfully
 */
HWTEST_F(BaseTaskDispatcherTest, TracePointBeforePost_0100, Function | MediumTest | Level1)
{
    GTEST_LOG_(INFO) << "TracePointBeforePost_0100 start";
    Runnable TestTask([]() { GTEST_LOG_(INFO) << "TracePointBeforePost_0100 task"; });
    std::shared_ptr<Runnable> runnable = std::make_shared<Runnable>(TestTask);
    std::shared_ptr<Task> task = std::make_shared<Task>(runnable, TaskPriority::DEFAULT, ptrSerialTaskDispatcher);
    std::string dispatcherName = "TracePointBeforePost_0100 ";
    ptrSerialTaskDispatcher->TracePointBeforePost(task, true, dispatcherName);
    GTEST_LOG_(INFO) << "TracePointBeforePost_0100 end";
}

/**
 * @tc.number:TracePointAfterPost_0100
 * @tc.name:  TracePointAfterPost
 * @tc.desc:  1.create SerialTaskDispatcher
 *            2.TracePointAfterPost successfully
 */
HWTEST_F(BaseTaskDispatcherTest, TracePointAfterPost_0100, Function | MediumTest | Level1)
{
    GTEST_LOG_(INFO) << "TracePointAfterPost_0100 start";
    Runnable TestTask([]() { GTEST_LOG_(INFO) << "TracePointBeforePost_0100 task"; });
    std::shared_ptr<Runnable> runnable = std::make_shared<Runnable>(TestTask);
    std::shared_ptr<Task> task = std::make_shared<Task>(runnable, TaskPriority::DEFAULT, ptrSerialTaskDispatcher);
    std::string dispatcherName = "TracePointAfterPost_0100 ";
    ptrSerialTaskDispatcher->TracePointAfterPost(task, false, dispatcherName);
    GTEST_LOG_(INFO) << "TracePointAfterPost_0100 end";
}
