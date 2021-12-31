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
#include "work_thread.h"
#undef private
#include "app_log_wrapper.h"
#include "default_thread_factory.h"
#include "default_worker_pool_config.h"
#include "task_priority.h"
#include "task_executor.h"
#include "task.h"

using namespace testing;
using namespace testing::ext;
using namespace OHOS::AppExecFwk;

class WorkerThreadTest : public testing::Test {
public:
    static void SetUpTestCase()
    {}
    static void TearDownTestCase()
    {}
    void SetUp()
    {}
    void TearDown()
    {}
};

/*
 * Feature: WorkerThread
 * Function: SyncTask
 * SubFunction:
 * FunctionPoints:
 * EnvConditions:
 * CaseDescription: Verify if new SyncTask with the valid params will return default value.
 */
HWTEST(WorkerThreadTest, WorkerThread_ConstructorTest_001, TestSize.Level0)
{
    auto name = std::string("WorkerThread_ConstructorTest_001");
    GTEST_LOG_(INFO) << name + " start";
    std::atomic<int> count(0);
    auto factory = std::make_shared<DefaultThreadFactory>();
    auto taskRunner = std::make_shared<Runnable>([&count, &name]() {
        count++;
        GTEST_LOG_(INFO) << name + "task end";
    });
    auto priority = TaskPriority::DEFAULT;
    auto firstTask = std::make_shared<Task>(taskRunner, priority, nullptr);

    auto config = std::make_shared<DefaultWorkerPoolConfig>();
    auto delegate = std::make_shared<TaskExecutor>(config);

    {
        std::shared_ptr<WorkerThread> thread = std::make_shared<WorkerThread>(delegate, firstTask, factory);
        EXPECT_EQ(thread->GetTaskCounter(), 0);
        EXPECT_EQ(thread->GetThreadName(), "");
    }
    {
        std::shared_ptr<WorkerThread> thread = std::make_shared<WorkerThread>(delegate, firstTask, factory);
        EXPECT_EQ(thread->GetTaskCounter(), 0);
        EXPECT_EQ(thread->GetThreadName(), "");
    }

    GTEST_LOG_(INFO) << name + " end";
}

/*
 * Feature: WorkerThread
 * Function: CreateThread
 * SubFunction:
 * FunctionPoints:
 * EnvConditions:
 * CaseDescription: verify create thread object
 */
HWTEST(WorkerThreadTest, WorkerThread_CreateThreadTest_001, TestSize.Level0)
{
    auto name = std::string("WorkerThread_CreateThreadTest_001");
    GTEST_LOG_(INFO) << name + " start";
    auto factory = std::make_shared<DefaultThreadFactory>();
    std::atomic<int> count(0);
    auto taskRunner = std::make_shared<Runnable>([&count, &name]() {
        count++;
        GTEST_LOG_(INFO) << name + "task end";
    });
    auto priority = TaskPriority::DEFAULT;
    auto firstTask = std::make_shared<Task>(taskRunner, priority, nullptr);

    auto config = std::make_shared<DefaultWorkerPoolConfig>();
    auto delegate = std::make_shared<TaskExecutor>(config);

    {
        std::shared_ptr<WorkerThread> thread = std::make_shared<WorkerThread>(delegate, firstTask, factory);
        thread->CreateThread();
        sleep(1);
        EXPECT_EQ(thread->GetTaskCounter(), 1);
        EXPECT_EQ(thread->GetThreadName(), std::string("PoolThread-1"));
        EXPECT_TRUE(thread->GetThread() != nullptr);
    }
    {
        std::shared_ptr<WorkerThread> thread = std::make_shared<WorkerThread>(delegate, firstTask, factory);
        thread->CreateThread();
        sleep(1);
        EXPECT_EQ(thread->GetTaskCounter(), 1);
        EXPECT_EQ(thread->GetThreadName(), std::string("PoolThread-2"));
        EXPECT_TRUE(thread->GetThread() != nullptr);
    }

    GTEST_LOG_(INFO) << name + " end";
}

/*
 * Feature: WorkerThread
 * Function: IncTaskCount
 * SubFunction:
 * FunctionPoints:
 * EnvConditions:
 * CaseDescription: verify task count increcement
 */
HWTEST(WorkerThreadTest, WorkerThread_IncTaskCountTest_001, TestSize.Level0)
{
    auto name = std::string("WorkerThread_IncTaskCountTest_001");
    GTEST_LOG_(INFO) << name + " start";
    std::atomic<int> count(0);
    auto factory = std::make_shared<DefaultThreadFactory>();
    auto taskRunner = std::make_shared<Runnable>([&count, &name]() {
        count++;
        GTEST_LOG_(INFO) << name + "task end";
    });
    auto priority = TaskPriority::DEFAULT;
    auto firstTask = std::make_shared<Task>(taskRunner, priority, nullptr);

    auto config = std::make_shared<DefaultWorkerPoolConfig>();
    auto delegate = std::make_shared<TaskExecutor>(config);

    {
        std::shared_ptr<WorkerThread> thread = std::make_shared<WorkerThread>(delegate, firstTask, factory);
        EXPECT_EQ(thread->GetTaskCounter(), 0);
        thread->IncTaskCount();
        EXPECT_EQ(thread->GetTaskCounter(), 1);
        thread->IncTaskCount();
        EXPECT_EQ(thread->GetTaskCounter(), 2);
    }

    GTEST_LOG_(INFO) << name + " end";
}

/*
 * Feature: WorkerThread
 * Function: GetTaskCounter
 * SubFunction:
 * FunctionPoints:
 * EnvConditions:
 * CaseDescription: verify task count
 */
HWTEST(WorkerThreadTest, WorkerThread_GetTaskCounterTest_001, TestSize.Level0)
{
    auto name = std::string("WorkerThread_GetTaskCounterTest_001");
    GTEST_LOG_(INFO) << name + " start";
    std::atomic<int> count(0);
    auto factory = std::make_shared<DefaultThreadFactory>();
    auto taskRunner = std::make_shared<Runnable>([&count, &name]() {
        count++;
        GTEST_LOG_(INFO) << name + "task end";
    });
    auto priority = TaskPriority::DEFAULT;
    auto firstTask = std::make_shared<Task>(taskRunner, priority, nullptr);

    auto config = std::make_shared<DefaultWorkerPoolConfig>();
    auto delegate = std::make_shared<TaskExecutor>(config);

    {
        std::shared_ptr<WorkerThread> thread = std::make_shared<WorkerThread>(delegate, firstTask, factory);
        EXPECT_EQ(thread->GetTaskCounter(), 0);
        thread->IncTaskCount();
        EXPECT_EQ(thread->GetTaskCounter(), 1);
        thread->IncTaskCount();
        EXPECT_EQ(thread->GetTaskCounter(), 2);
    }

    GTEST_LOG_(INFO) << name + " end";
}

/*
 * Feature: WorkerThread
 * Function: GetThreadName
 * SubFunction:
 * FunctionPoints:
 * EnvConditions:
 * CaseDescription: verify thread name
 */
HWTEST(WorkerThreadTest, WorkerThread_GetThreadNameTest_001, TestSize.Level0)
{
    auto name = std::string("WorkerThread_GetThreadNameTest_001");
    GTEST_LOG_(INFO) << name + " start";
    std::atomic<int> count(0);
    auto factory = std::make_shared<DefaultThreadFactory>();
    auto taskRunner = std::make_shared<Runnable>([&count, &name]() {
        count++;
        GTEST_LOG_(INFO) << name + "task end";
    });
    auto priority = TaskPriority::DEFAULT;
    auto firstTask = std::make_shared<Task>(taskRunner, priority, nullptr);

    auto config = std::make_shared<DefaultWorkerPoolConfig>();
    auto delegate = std::make_shared<TaskExecutor>(config);
    {
        std::shared_ptr<WorkerThread> thread = std::make_shared<WorkerThread>(delegate, firstTask, factory);
        thread->CreateThread();
        sleep(1);
        EXPECT_EQ(thread->GetThreadName(), std::string("PoolThread-1"));
    }
    {
        std::shared_ptr<WorkerThread> thread = std::make_shared<WorkerThread>(delegate, firstTask, factory);
        thread->CreateThread();
        sleep(1);
        EXPECT_EQ(thread->GetThreadName(), std::string("PoolThread-2"));
    }
    {
        std::shared_ptr<WorkerThread> thread = std::make_shared<WorkerThread>(delegate, firstTask, factory);
        thread->CreateThread();
        sleep(1);
        EXPECT_EQ(thread->GetThreadName(), std::string("PoolThread-3"));
    }

    GTEST_LOG_(INFO) << name + " end";
}
HWTEST(WorkerThreadTest, WorkerThread_GetThreadNameTest_002, TestSize.Level0)
{
    auto name = std::string("WorkerThread_GetThreadNameTest_002");
    GTEST_LOG_(INFO) << name + " start";
    std::atomic<int> count(0);
    auto factory = std::make_shared<DefaultThreadFactory>();
    auto taskRunner = std::make_shared<Runnable>([&count, &name]() {
        count++;
        GTEST_LOG_(INFO) << name + "task end";
    });
    auto priority = TaskPriority::DEFAULT;
    auto firstTask = std::make_shared<Task>(taskRunner, priority, nullptr);

    auto config = std::make_shared<DefaultWorkerPoolConfig>();
    auto delegate = std::make_shared<TaskExecutor>(config);

    {
        std::shared_ptr<WorkerThread> thread = std::make_shared<WorkerThread>(delegate, firstTask, factory);
        EXPECT_EQ(thread->GetThreadName(), std::string(""));
    }

    GTEST_LOG_(INFO) << name + " end";
}

/*
 * Feature: WorkerThread
 * Function: PollFirstTask
 * SubFunction:
 * FunctionPoints:
 * EnvConditions:
 * CaseDescription: verify thread name
 */
HWTEST(WorkerThreadTest, WorkerThread_PollFirstTaskTest_001, TestSize.Level0)
{
    auto name = std::string("WorkerThread_PollFirstTaskTest_001");
    GTEST_LOG_(INFO) << name + " start";
    std::atomic<int> count(0);
    auto factory = std::make_shared<DefaultThreadFactory>();
    auto taskRunner = std::make_shared<Runnable>([&count, &name]() {
        count++;
        GTEST_LOG_(INFO) << name + "task end";
    });
    auto priority = TaskPriority::DEFAULT;
    auto firstTask = std::make_shared<Task>(taskRunner, priority, nullptr);

    auto config = std::make_shared<DefaultWorkerPoolConfig>();
    auto delegate = std::make_shared<TaskExecutor>(config);
    {
        std::shared_ptr<WorkerThread> thread = std::make_shared<WorkerThread>(delegate, firstTask, factory);
        thread->CreateThread();
        sleep(1);
        EXPECT_EQ(thread->PollFirstTask(), nullptr);
    }

    GTEST_LOG_(INFO) << name + " end";
}
HWTEST(WorkerThreadTest, WorkerThread_PollFirstTaskTest_002, TestSize.Level0)
{
    auto name = std::string("WorkerThread_PollFirstTaskTest_002");
    GTEST_LOG_(INFO) << name + " start";
    auto factory = std::make_shared<DefaultThreadFactory>();
    auto firstTask = nullptr;

    auto config = std::make_shared<DefaultWorkerPoolConfig>();
    auto delegate = std::make_shared<TaskExecutor>(config);

    {
        std::shared_ptr<WorkerThread> thread = std::make_shared<WorkerThread>(delegate, firstTask, factory);
        thread->CreateThread();
        sleep(1);
        EXPECT_EQ(thread->PollFirstTask(), firstTask);
    }

    GTEST_LOG_(INFO) << name + " end";
}

/*
 * Feature: WorkerThread
 * Function: GetThread
 * SubFunction:
 * FunctionPoints:
 * EnvConditions:
 * CaseDescription: verify thread name
 */
HWTEST(WorkerThreadTest, WorkerThread_GetThread_001, TestSize.Level0)
{
    auto name = std::string("WorkerThread_GetThread_001");
    GTEST_LOG_(INFO) << name + " start";
    std::atomic<int> count(0);
    auto factory = std::make_shared<DefaultThreadFactory>();
    auto taskRunner = std::make_shared<Runnable>([&count, &name]() {
        count++;
        GTEST_LOG_(INFO) << name + "task end";
    });
    auto priority = TaskPriority::DEFAULT;
    auto firstTask = std::make_shared<Task>(taskRunner, priority, nullptr);

    auto config = std::make_shared<DefaultWorkerPoolConfig>();
    auto delegate = std::make_shared<TaskExecutor>(config);

    {
        std::shared_ptr<WorkerThread> thread = std::make_shared<WorkerThread>(delegate, firstTask, factory);
        thread->CreateThread();
        sleep(1);
        EXPECT_TRUE(thread->GetThread() != nullptr);
    }

    GTEST_LOG_(INFO) << name + " end";
}
HWTEST(WorkerThreadTest, WorkerThread_GetThread_002, TestSize.Level0)
{
    auto name = std::string("WorkerThread_GetThread_002");
    GTEST_LOG_(INFO) << name + " start";
    auto factory = std::make_shared<DefaultThreadFactory>();
    auto firstTask = nullptr;

    auto config = std::make_shared<DefaultWorkerPoolConfig>();
    auto delegate = std::make_shared<TaskExecutor>(config);

    {
        std::shared_ptr<WorkerThread> thread = std::make_shared<WorkerThread>(delegate, firstTask, factory);
        EXPECT_EQ(thread->GetThread(), nullptr);
    }

    GTEST_LOG_(INFO) << name + " end";
}
