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
#include "barrier_handler.h"
#undef private
#include "appexecfwk_errors.h"
#include "default_worker_pool_config.h"

using namespace testing;
using namespace testing::ext;
using namespace OHOS::AppExecFwk;
using namespace OHOS;

class BarrierHandlerTest : public testing::Test {
public:
    static void SetUpTestCase();
    static void TearDownTestCase();
    void SetUp();
    void TearDown();
};

void BarrierHandlerTest::SetUpTestCase()
{}

void BarrierHandlerTest::TearDownTestCase()
{}

void BarrierHandlerTest::SetUp()
{}

void BarrierHandlerTest::TearDown()
{}

std::shared_ptr<BarrierHandler> CreateBarrierHandler()
{
    std::shared_ptr<DefaultWorkerPoolConfig> config_ = std::make_shared<DefaultWorkerPoolConfig>();
    std::shared_ptr<TaskExecutor> executor = std::make_shared<TaskExecutor>(config_);
    std::shared_ptr<BarrierHandler> barrierHandler = std::make_shared<BarrierHandler>(executor);
    return barrierHandler;
}
/**
 * @tc.number: BarrierHandler_ConstructorTest_001
 * @tc.name: Constructor
 * @tc.desc: Test  BarrierHandlerTest Constructor.
 */
HWTEST(BarrierHandlerTest, BarrierHandler_ConstructorTest_001, TestSize.Level0)
{
    auto name = std::string("BarrierHandler_ConstructorTest_001");
    GTEST_LOG_(INFO) << name << " start";
    std::shared_ptr<BarrierHandler> barrierHandler = CreateBarrierHandler();
    EXPECT_NE(barrierHandler, nullptr);
    GTEST_LOG_(INFO) << name << " end";
}

/**
 * @tc.number: BarrierHandler_interceptTest_001
 * @tc.name: intercept
 * @tc.desc: Test  BarrierHandler intercept.
 */
HWTEST(BarrierHandlerTest, BarrierHandler_interceptTest_001, TestSize.Level0)
{
    auto name = std::string("BarrierHandler_interceptTest_001");
    GTEST_LOG_(INFO) << name << " start";
    std::shared_ptr<BarrierHandler> barrierHandler = CreateBarrierHandler();
    std::shared_ptr<Runnable> runnable = std::make_shared<Runnable>([&]() { GTEST_LOG_(INFO) << name << " Runnable"; });
    std::shared_ptr<Task> task = std::make_shared<Task>(runnable, TaskPriority::DEFAULT, nullptr);
    ErrCode result = barrierHandler->Intercept(task);
    task->Run();
    task->AfterTaskExecute();
    EXPECT_TRUE(result);
    GTEST_LOG_(INFO) << name << " end";
}

/**
 * @tc.number: BarrierHandler_interceptTest_002
 * @tc.name: intercept
 * @tc.desc: Test  BarrierHandler intercept.
 */
HWTEST(BarrierHandlerTest, BarrierHandler_interceptTest_002, TestSize.Level0)
{
    auto name = std::string("BarrierHandler_interceptTest_002");
    GTEST_LOG_(INFO) << name << " start";
    std::shared_ptr<BarrierHandler> barrierHandler = CreateBarrierHandler();
    // add task
    std::shared_ptr<Runnable> runnable = std::make_shared<Runnable>([&]() {
        std::this_thread::sleep_for(std::chrono::milliseconds(500));
        GTEST_LOG_(INFO) << name << " Runnable";
    });
    std::shared_ptr<Task> task = std::make_shared<Task>(runnable, TaskPriority::DEFAULT, nullptr);
    barrierHandler->Intercept(task);
    // add barrier
    std::shared_ptr<Runnable> barrierRunnable =
        std::make_shared<Runnable>([&]() { GTEST_LOG_(INFO) << name << " barrierTask "; });
    std::shared_ptr<Task> barrierTask = std::make_shared<Task>(barrierRunnable, TaskPriority::DEFAULT, nullptr);
    barrierHandler->AddBarrier(barrierTask);
    // add interceptTask
    std::shared_ptr<Runnable> interceptRunnable =
        std::make_shared<Runnable>([&]() { GTEST_LOG_(INFO) << name << " interceptRunnable"; });
    std::shared_ptr<Task> interceptTask = std::make_shared<Task>(interceptRunnable, TaskPriority::DEFAULT, nullptr);
    // intercept result
    bool result = barrierHandler->Intercept(interceptTask);
    EXPECT_TRUE(result);
    GTEST_LOG_(INFO) << name << " end";
}

/**
 * @tc.number: BarrierHandler_addBarrierTest_001
 * @tc.name: addBarrier
 * @tc.desc: Test  BarrierHandler addBarrier.
 */
HWTEST(BarrierHandlerTest, BarrierHandler_addBarrierTest_001, TestSize.Level0)
{
    auto name = std::string("BarrierHandler_interceptTest_002");
    GTEST_LOG_(INFO) << name << " start";
    std::shared_ptr<BarrierHandler> barrierHandler = CreateBarrierHandler();

    std::atomic<int> count(0);

    std::shared_ptr<Runnable> barrierRunnable = std::make_shared<Runnable>([&]() {
        std::this_thread::sleep_for(std::chrono::milliseconds(300));
        count.fetch_add(1);
        GTEST_LOG_(INFO) << name << " add barrier Task ";
    });
    std::shared_ptr<Task> barrierTask = std::make_shared<Task>(barrierRunnable, TaskPriority::DEFAULT, nullptr);
    barrierHandler->AddBarrier(barrierTask);
    EXPECT_EQ(count.load(), 0);
    std::this_thread::sleep_for(std::chrono::milliseconds(600));
    EXPECT_EQ(count.load(), 1);
    GTEST_LOG_(INFO) << name << " end";
}

/**
 * @tc.number: BarrierHandler_addBarrierTest_002
 * @tc.name: addBarrier
 * @tc.desc: Test  BarrierHandler barrier not execute now.
 */
HWTEST(BarrierHandlerTest, BarrierHandler_addBarrierTest_002, TestSize.Level0)
{
    auto name = std::string("BarrierHandler_addBarrierTest_002");
    GTEST_LOG_(INFO) << name << " start";
    std::shared_ptr<BarrierHandler> barrierHandler = CreateBarrierHandler();

    std::atomic<int> count(0);

    std::shared_ptr<Runnable> taskRun1 = std::make_shared<Runnable>([&]() {
        std::this_thread::sleep_for(std::chrono::milliseconds(300));
        count.fetch_add(1);
        GTEST_LOG_(INFO) << name << " add barrier Task ";
    });
    std::shared_ptr<Task> task1 = std::make_shared<Task>(taskRun1, TaskPriority::DEFAULT, nullptr);
    barrierHandler->Intercept(task1);
    std::shared_ptr<Runnable> taskRun2 = std::make_shared<Runnable>([&]() {
        count.fetch_add(1);
        GTEST_LOG_(INFO) << name << " add barrier Task ";
    });
    std::shared_ptr<Task> task2 = std::make_shared<Task>(taskRun2, TaskPriority::DEFAULT, nullptr);
    barrierHandler->Intercept(task2);

    std::shared_ptr<Runnable> barrierRunnable = std::make_shared<Runnable>([&]() {
        std::this_thread::sleep_for(std::chrono::milliseconds(300));
        EXPECT_EQ(count.load(), 2);
        GTEST_LOG_(INFO) << name << " add barrier Task ";
    });
    std::shared_ptr<Task> barrierTask = std::make_shared<Task>(barrierRunnable, TaskPriority::DEFAULT, nullptr);
    barrierHandler->AddBarrier(barrierTask);
    GTEST_LOG_(INFO) << name << " end";
}