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
#include "task_dispatcher_context.h"
#undef private
#include "default_worker_pool_config.h"
#include "task.h"
#include "task_priority.h"
#include "task_executor.h"

using namespace testing;
using namespace testing::ext;
using namespace OHOS::AppExecFwk;

class TaskDispatcherContextTest : public testing::Test {
public:
    static void SetUpTestCase();
    static void TearDownTestCase();
    void SetUp();
    void TearDown();
};

void TaskDispatcherContextTest::SetUpTestCase()
{}

void TaskDispatcherContextTest::TearDownTestCase()
{}

void TaskDispatcherContextTest::SetUp()
{}

void TaskDispatcherContextTest::TearDown()
{}

/**
 * @tc.number:TaskDispatcherContext_0100
 * @tc.name: createTaskDispatcherContext
 * @tc.desc: 1.create default TaskDispatcherContext
 *           2.GetWorkerPoolConfig successfully
 */
HWTEST(TaskDispatcherContextTest, TaskDispatcherContext_0100, Function | MediumTest | Level1)
{
    GTEST_LOG_(INFO) << "TaskDispatcherContext_0100 start";
    std::shared_ptr<TaskDispatcherContext> taskDispatcherContext = std::make_shared<TaskDispatcherContext>();
    std::shared_ptr<WorkerPoolConfig> config = taskDispatcherContext->GetWorkerPoolConfig();
    EXPECT_EQ(config.get()->GetMaxThreadCount(), 32);
    GTEST_LOG_(INFO) << "TaskDispatcherContext_0100 end";
}

/**
 * @tc.number:TaskDispatcherContext_0200
 * @tc.name: createTaskDispatcherContext
 * @tc.desc: 1.create  TaskDispatcherContext by TaskExecutor
 *           2.GetWorkerThreadsInfo successfully
 */
HWTEST(TaskDispatcherContextTest, TaskDispatcherContext_0200, Function | MediumTest | Level1)
{
    GTEST_LOG_(INFO) << "TaskDispatcherContext_0200 start";
    const std::shared_ptr<WorkerPoolConfig> config = std::make_shared<DefaultWorkerPoolConfig>();
    const std::shared_ptr<TaskExecutor> testTaskExecutor = std::make_shared<TaskExecutor>(config);
    std::shared_ptr<TaskDispatcherContext> taskDispatcherContext =
        std::make_shared<TaskDispatcherContext>(testTaskExecutor);
    std::map<std::string, long> workerThreadsInfo = taskDispatcherContext->GetWorkerThreadsInfo();
    bool flag = (workerThreadsInfo.size() == 0);
    EXPECT_TRUE(flag);
    GTEST_LOG_(INFO) << "TaskDispatcherContext_0200 end";
}

/**
 * @tc.number:GetWorkerPoolConfig_0100
 * @tc.name: createTaskDispatcherContext
 * @tc.desc: 1.create default TaskDispatcherContext
 *           2.GetWorkerPoolConfig successfully
 */
HWTEST(TaskDispatcherContextTest, GetWorkerPoolConfig_0100, Function | MediumTest | Level1)
{
    GTEST_LOG_(INFO) << "GetWorkerPoolConfig_0100 start";
    std::shared_ptr<TaskDispatcherContext> taskDispatcherContext = std::make_shared<TaskDispatcherContext>();
    std::shared_ptr<WorkerPoolConfig> config = taskDispatcherContext->GetWorkerPoolConfig();
    EXPECT_EQ(config.get()->GetMaxThreadCount(), 32);
    GTEST_LOG_(INFO) << "GetWorkerPoolConfig_0100 end";
}
/**
 * @tc.number:GetWorkerThreadsInfo_0100
 * @tc.name: GetWorkerThreadsInfo
 * @tc.desc: 1.create  TaskDispatcherContext by TaskExecutor,TaskExecutor is nullptr
 *           2.GetWorkerThreadsInfo successfully
 */
HWTEST(TaskDispatcherContextTest, GetWorkerThreadsInfo_0100, Function | MediumTest | Level1)
{
    GTEST_LOG_(INFO) << "GetWorkerThreadsInfo_0100 start";
    const std::shared_ptr<WorkerPoolConfig> config = std::make_shared<DefaultWorkerPoolConfig>();
    std::shared_ptr<TaskDispatcherContext> taskDispatcherContext = std::make_shared<TaskDispatcherContext>(nullptr);
    std::map<std::string, long> workerThreadsInfo = taskDispatcherContext->GetWorkerThreadsInfo();
    bool flag = (workerThreadsInfo.size() == 0);
    EXPECT_TRUE(flag);
    GTEST_LOG_(INFO) << "GetWorkerThreadsInfo_0100 end";
}
/**
 * @tc.number:GetWorkerThreadsInfo_0200
 * @tc.name: GetWorkerThreadsInfo
 * @tc.desc: 1.create  TaskDispatcherContext by TaskExecutor
 *           2.GetWorkerThreadsInfo successfully
 */
HWTEST(TaskDispatcherContextTest, GetWorkerThreadsInfo_0200, Function | MediumTest | Level1)
{
    GTEST_LOG_(INFO) << "TaskDispatcherContext_0200 start";
    const std::shared_ptr<WorkerPoolConfig> config = std::make_shared<DefaultWorkerPoolConfig>();
    const std::shared_ptr<TaskExecutor> testTaskExecutor = std::make_shared<TaskExecutor>(config);
    std::shared_ptr<TaskDispatcherContext> taskDispatcherContext =
        std::make_shared<TaskDispatcherContext>(testTaskExecutor);
    std::map<std::string, long> workerThreadsInfo = taskDispatcherContext->GetWorkerThreadsInfo();
    bool flag = (workerThreadsInfo.size() == 0);
    EXPECT_TRUE(flag);
    GTEST_LOG_(INFO) << "TaskDispatcherContext_0200 end";
}
/**
 * @tc.number:CreateSerialDispatcher_0100
 * @tc.name: CreateSerialDispatcher
 * @tc.desc: 1.CreateSerialDispatcher
 *           2.GetSerialDispatchers successfully
 */
HWTEST(TaskDispatcherContextTest, CreateSerialDispatcher_0100, Function | MediumTest | Level1)
{
    GTEST_LOG_(INFO) << "CreateSerialDispatcher_0100 start";

    std::shared_ptr<TaskDispatcherContext> taskDispatcherContext = std::make_shared<TaskDispatcherContext>();
    std::string dispatcherName = "test-serial-dispatcher";
    TaskPriority taskPriority = TaskPriority::DEFAULT;
    std::shared_ptr<SerialTaskDispatcher> ptrSerialTaskDispatcher =
        taskDispatcherContext->CreateSerialDispatcher(dispatcherName, taskPriority);
    EXPECT_EQ(ptrSerialTaskDispatcher->GetDispatcherName(), dispatcherName);
    GTEST_LOG_(INFO) << "CreateSerialDispatcher_0100 end";
}
/**
 * @tc.number:GetSerialDispatchers_0100
 * @tc.name: GetSerialDispatchers
 * @tc.desc: 1.CreateSerialDispatcher
 *           2.GetSerialDispatchers successfully
 */
HWTEST(TaskDispatcherContextTest, GetSerialDispatchers_0100, Function | MediumTest | Level1)
{
    GTEST_LOG_(INFO) << "GetSerialDispatchers_0100 start";

    std::shared_ptr<TaskDispatcherContext> taskDispatcherContext = std::make_shared<TaskDispatcherContext>();
    std::string dispatcherName = "test-serial-dispatcher";
    TaskPriority taskPriority = TaskPriority::DEFAULT;
    std::shared_ptr<SerialTaskDispatcher> ptrSerialTaskDispatcher =
        taskDispatcherContext->CreateSerialDispatcher(dispatcherName, taskPriority);

    std::shared_ptr<SerialTaskDispatcher> ptrSerialTaskDispatcherNew =
        taskDispatcherContext->CreateSerialDispatcher(dispatcherName, taskPriority);

    std::map<std::shared_ptr<SerialTaskDispatcher>, std::string> serialDispatchers =
        taskDispatcherContext->GetSerialDispatchers();
    bool flag = (serialDispatchers.size() == 2);
    EXPECT_TRUE(flag);
    std::map<std::shared_ptr<SerialTaskDispatcher>, std::string>::iterator iter;
    for (iter = serialDispatchers.begin(); iter != serialDispatchers.end(); iter++) {
        EXPECT_EQ(iter->second, dispatcherName);
    }
    GTEST_LOG_(INFO) << "GetSerialDispatchers_0100 end";
}
/**
 * @tc.number:CreateParallelDispatcher_0100
 * @tc.name: CreateParallelDispatcher
 * @tc.desc: 1.CreateParallelDispatcher
 *           2.GetPriority successfully
 */
HWTEST(TaskDispatcherContextTest, CreateParallelDispatcher_0100, Function | MediumTest | Level1)
{
    GTEST_LOG_(INFO) << "CreateParallelDispatcher_0100 start";
    std::shared_ptr<TaskDispatcherContext> taskDispatcherContext = std::make_shared<TaskDispatcherContext>();
    std::string dispatcherName = "test-parallel-dispatcher";
    TaskPriority taskPriority = TaskPriority::HIGH;
    std::shared_ptr<ParallelTaskDispatcher> ptrSerialTaskDispatcher =
        taskDispatcherContext->CreateParallelDispatcher(dispatcherName, taskPriority);
    EXPECT_EQ(ptrSerialTaskDispatcher->GetPriority(), taskPriority);
    GTEST_LOG_(INFO) << "CreateParallelDispatcher_0100 end";
}

/**
 * @tc.number:GetGlobalTaskDispatcher_0100
 * @tc.name: GetGlobalTaskDispatcher
 * @tc.desc: 1.GetGlobalTaskDispatcher
 */
HWTEST(TaskDispatcherContextTest, GetGlobalTaskDispatcher_0100, Function | MediumTest | Level1)
{
    GTEST_LOG_(INFO) << "GetGlobalTaskDispatcher_0100 start";
    std::shared_ptr<TaskDispatcherContext> taskDispatcherContext = std::make_shared<TaskDispatcherContext>();
    TaskPriority taskPriority = TaskPriority::HIGH;
    // case 1:GetGlobalTaskDispatcher dispatcher is null
    std::shared_ptr<TaskDispatcher> ptr = taskDispatcherContext->GetGlobalTaskDispatcher(taskPriority);
    GTEST_LOG_(INFO) << "GetGlobalTaskDispatcher_0100 end";
}
/**
 * @tc.number:GetGlobalTaskDispatcher_0200
 * @tc.name: GetGlobalTaskDispatcher
 * @tc.desc: 1.GetGlobalTaskDispatcher
 */
HWTEST(TaskDispatcherContextTest, GetGlobalTaskDispatcher_0200, Function | MediumTest | Level1)
{
    GTEST_LOG_(INFO) << "GetGlobalTaskDispatcher_0200 start";
    std::shared_ptr<TaskDispatcherContext> taskDispatcherContext = std::make_shared<TaskDispatcherContext>();
    TaskPriority taskPriority = TaskPriority::HIGH;
    // case 1:GetGlobalTaskDispatcher dispatcher is null
    std::shared_ptr<TaskDispatcher> ptr = taskDispatcherContext->GetGlobalTaskDispatcher(taskPriority);
    // case 2:GetGlobalTaskDispatcher dispatcher is not null
    std::shared_ptr<TaskDispatcher> ptr1 = taskDispatcherContext->GetGlobalTaskDispatcher(taskPriority);
    GTEST_LOG_(INFO) << "GetGlobalTaskDispatcher_0200 end";
}
/**
 * @tc.number:Shutdown_0100
 * @tc.name: Shutdown
 * @tc.desc: 1.Shutdown
 */
HWTEST(TaskDispatcherContextTest, Shutdown_0100, Function | MediumTest | Level1)
{
    GTEST_LOG_(INFO) << "Shutdown_0100 start";
    const std::shared_ptr<WorkerPoolConfig> config = std::make_shared<DefaultWorkerPoolConfig>();
    const std::shared_ptr<TaskExecutor> testTaskExecutor = std::make_shared<TaskExecutor>(config);
    testTaskExecutor->DelayExecute([]() { GTEST_LOG_(INFO) << "Shutdown_0100  delay execute task"; }, 10);
    std::shared_ptr<TaskDispatcherContext> taskDispatcherContext =
        std::make_shared<TaskDispatcherContext>(testTaskExecutor);
    taskDispatcherContext->Shutdown(true);
    GTEST_LOG_(INFO) << "Shutdown_0100 end";
}

/**
 * @tc.number:GetWaitingTasksCount_0100
 * @tc.name: GetWaitingTasksCount
 * @tc.desc: 1.create  TaskDispatcherContext by TaskExecutor,TaskExecutor is nullptr
 *           2.GetWaitingTasksCount  eq 0
 */
HWTEST(TaskDispatcherContextTest, GetWaitingTasksCount_0100, Function | MediumTest | Level1)
{
    GTEST_LOG_(INFO) << "GetWaitingTasksCount_0100 start";
    const std::shared_ptr<TaskExecutor> testTaskExecutor = nullptr;
    std::shared_ptr<TaskDispatcherContext> taskDispatcherContext =
        std::make_shared<TaskDispatcherContext>(testTaskExecutor);
    EXPECT_EQ(taskDispatcherContext->GetWaitingTasksCount(), 0);
    GTEST_LOG_(INFO) << "GetWaitingTasksCount_0100 end";
}

/**
 * @tc.number:GetWaitingTasksCount_0200
 * @tc.name: GetWaitingTasksCount
 * @tc.desc: 1.create  TaskDispatcherContext by TaskExecutor,TaskExecutor is not  nullptr
 *           2.GetWaitingTasksCount  eq 0
 */
HWTEST(TaskDispatcherContextTest, GetWaitingTasksCount_0200, Function | MediumTest | Level1)
{
    GTEST_LOG_(INFO) << "GetWaitingTasksCount_0200 start";
    const std::shared_ptr<WorkerPoolConfig> config = std::make_shared<DefaultWorkerPoolConfig>();
    const std::shared_ptr<TaskExecutor> testTaskExecutor = std::make_shared<TaskExecutor>(config);
    std::shared_ptr<TaskDispatcherContext> taskDispatcherContext =
        std::make_shared<TaskDispatcherContext>(testTaskExecutor);
    EXPECT_EQ(taskDispatcherContext->GetWaitingTasksCount(), 0);
    GTEST_LOG_(INFO) << "GetWaitingTasksCount_0200 end";
}
/**
 * @tc.number:GetTaskCounter_0100
 * @tc.name: GetTaskCounter
 * @tc.desc: 1.create  TaskDispatcherContext by TaskExecutor
 *           2.GetWaitingTasksCount  eq 0
 */
HWTEST(TaskDispatcherContextTest, GetTaskCounter_0100, Function | MediumTest | Level1)
{
    GTEST_LOG_(INFO) << "GetTaskCounter_0100 start";
    const std::shared_ptr<WorkerPoolConfig> config = std::make_shared<DefaultWorkerPoolConfig>();
    const std::shared_ptr<TaskExecutor> testTaskExecutor = std::make_shared<TaskExecutor>(config);
    std::shared_ptr<TaskDispatcherContext> taskDispatcherContext =
        std::make_shared<TaskDispatcherContext>(testTaskExecutor);
    EXPECT_EQ(taskDispatcherContext->GetTaskCounter(), 0);
    GTEST_LOG_(INFO) << "GetTaskCounter_0100 end";
}