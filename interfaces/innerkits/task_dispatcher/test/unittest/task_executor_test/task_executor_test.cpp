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

#include "task_executor.h"
#include "default_worker_pool_config.h"
#include "worker_pool_config.h"
#include "default_thread_factory.h"

namespace OHOS {
namespace AppExecFwk {
using namespace testing::ext;
using namespace OHOS;
using namespace OHOS::AppExecFwk;

class TaskExecutorTest : public testing::Test {
public:
    static void SetUpTestCase(void);
    static void TearDownTestCase(void);
    void SetUp();
    void TearDown();
    std::shared_ptr<TaskExecutor> CreateTaskExecutor();
};

void TaskExecutorTest::SetUpTestCase(void)
{}

void TaskExecutorTest::TearDownTestCase(void)
{}

void TaskExecutorTest::SetUp(void)
{}

void TaskExecutorTest::TearDown(void)
{}

std::shared_ptr<TaskExecutor> CreateTaskExecutor()
{
    const std::shared_ptr<WorkerPoolConfig> config = std::make_shared<DefaultWorkerPoolConfig>();
    const std::shared_ptr<TaskExecutor> testTaskExecutor = std::make_shared<TaskExecutor>(config);
    return testTaskExecutor;
}

/**
 * @tc.number: AppExecFwk_TaskExecutor_Constructor_0100
 * @tc.name: Constructor
 * @tc.desc: Test whether constructor are called normally
 */
HWTEST(TaskExecutorTest, AppExecFwk_TaskExecutor_Constructor_0100, Function | MediumTest | Level1)
{
    GTEST_LOG_(INFO) << "AppExecFwk_TaskExecutor_Constructor_0100 start";
    const std::shared_ptr<WorkerPoolConfig> config = std::make_shared<DefaultWorkerPoolConfig>();
    std::make_shared<TaskExecutor>(config);

    GTEST_LOG_(INFO) << "AppExecFwk_TaskExecutor_Constructor_0100 end";
}

/**
 * @tc.number: AppExecFwk_TaskExecutor_Execute_0100
 * @tc.name: Execute
 * @tc.desc: Test whether taskExecute are called normally,
 *           and verify whether the return value of getTaskCounter equal to 1
 *
 */
HWTEST(TaskExecutorTest, AppExecFwk_TaskExecutor_Execute_0100, Function | MediumTest | Level1)
{
    GTEST_LOG_(INFO) << "AppExecFwk_TaskExecutor_Execute_0100 start";

    const std::shared_ptr<TaskExecutor> testTaskExecutor = CreateTaskExecutor();
    Runnable TestTask([]() { GTEST_LOG_(INFO) << "AppExecFwk_TaskExecutor_Execute_0100 task"; });
    std::shared_ptr<Runnable> runnable = std::make_shared<Runnable>(TestTask);
    std::shared_ptr<Task> task = std::make_shared<Task>(runnable, TaskPriority::DEFAULT, nullptr);
    testTaskExecutor->Execute(task);
    sleep(1);
    long count = testTaskExecutor->GetTaskCounter();
    EXPECT_EQ(1, count);
    GTEST_LOG_(INFO) << "AppExecFwk_TaskExecutor_Execute_0100 end";
}

/**
 * @tc.number: AppExecFwk_TaskExecutor_Execute_0200
 * @tc.name: Execute
 * @tc.desc: Test whether taskExecute are called normally
 *
 */
HWTEST(TaskExecutorTest, AppExecFwk_TaskExecutor_Execute_0200, Function | MediumTest | Level1)
{
    GTEST_LOG_(INFO) << "AppExecFwk_TaskExecutor_Execute_0200 start";

    const std::shared_ptr<TaskExecutor> testTaskExecutor = CreateTaskExecutor();
    std::atomic<int> count(0);
    Runnable TestTask([&count]() {
        GTEST_LOG_(INFO) << "AppExecFwk_TaskExecutor_Execute_0200 task ";
        count++;
    });
    std::shared_ptr<Runnable> runnable = std::make_shared<Runnable>(TestTask);
    std::shared_ptr<Task> task = std::make_shared<Task>(runnable, TaskPriority::DEFAULT, nullptr);
    for (int i = 0; i < 33; i++) {
        testTaskExecutor->Execute(task);
    }
    sleep(1);
    GTEST_LOG_(INFO) << "AppExecFwk_TaskExecutor_Execute_0200 end";
}

/**
 * @tc.number: AppExecFwk_TaskExecutor_DelayExecute_0100
 * @tc.name: DelayExecute
 * @tc.desc: Test whether delayExecute and terminate are called normally
 *
 */
HWTEST(TaskExecutorTest, AppExecFwk_TaskExecutor_DelayExecute_0100, Function | MediumTest | Level1)
{
    GTEST_LOG_(INFO) << "AppExecFwk_TaskExecutor_DelayExecute_0100 start";

    const std::shared_ptr<TaskExecutor> testTaskExecutor = CreateTaskExecutor();
    testTaskExecutor->DelayExecute(
        []() { GTEST_LOG_(INFO) << "AppExecFwk_TaskExecutor_DelayExecute_0100 delay execute task001"; }, 3000);
    testTaskExecutor->DelayExecute(
        []() { GTEST_LOG_(INFO) << "AppExecFwk_TaskExecutor_DelayExecute_0100 delay execute task002"; }, 1000);
    sleep(1);
    testTaskExecutor->Terminate(true);
    GTEST_LOG_(INFO) << "AppExecFwk_TaskExecutor_DelayExecute_0100 end";
}

/**
 * @tc.number: AppExecFwk_TaskExecutor_DelayExecute_0200
 * @tc.name: DelayExecute
 * @tc.desc: Test whether delayExecute are called normally
 *
 */
HWTEST(TaskExecutorTest, AppExecFwk_TaskExecutor_DelayExecute_0200, Function | MediumTest | Level1)
{
    GTEST_LOG_(INFO) << "AppExecFwk_TaskExecutor_DelayExecute_0200 start";

    const std::shared_ptr<TaskExecutor> testTaskExecutor = CreateTaskExecutor();
    testTaskExecutor->DelayExecute(
        []() { GTEST_LOG_(INFO) << "AppExecFwk_TaskExecutor_DelayExecute_0200 delay execute task001"; }, 0);
    GTEST_LOG_(INFO) << "AppExecFwk_TaskExecutor_DelayExecute_0200 end";
}

/**
 * @tc.number: AppExecFwk_TaskExecutor_DelayExecute_0300
 * @tc.name: DelayExecute
 * @tc.desc: Test whether delayExecute are called normally
 *
 */
HWTEST(TaskExecutorTest, AppExecFwk_TaskExecutor_DelayExecute_0300, Function | MediumTest | Level1)
{
    GTEST_LOG_(INFO) << "AppExecFwk_TaskExecutor_DelayExecute_0300 start";

    const std::shared_ptr<TaskExecutor> testTaskExecutor = CreateTaskExecutor();
    testTaskExecutor->Terminate(true);
    sleep(1);
    testTaskExecutor->DelayExecute(
        []() { GTEST_LOG_(INFO) << "AppExecFwk_TaskExecutor_DelayExecute_0300 delay execute task"; }, 1000);

    GTEST_LOG_(INFO) << "AppExecFwk_TaskExecutor_DelayExecute_0300 end";
}

/**
 * @tc.number: AppExecFwk_TaskExecutor_Terminate_0100
 * @tc.name: Terminate
 * @tc.desc: Test whether terminate are called normally
 *
 */
HWTEST(TaskExecutorTest, AppExecFwk_TaskExecutor_Terminate_0100, Function | MediumTest | Level1)
{
    GTEST_LOG_(INFO) << "AppExecFwk_TaskExecutor_Terminate_0100 start";

    const std::shared_ptr<TaskExecutor> testTaskExecutor = CreateTaskExecutor();
    testTaskExecutor->Terminate(true);
    GTEST_LOG_(INFO) << "AppExecFwk_TaskExecutor_Terminate_0100 end";
}

/**
 * @tc.number: AppExecFwk_TaskExecutor_DoWorks_0100
 * @tc.name: DoWorks
 * @tc.desc: Test the abnormal state of doWorks
 *
 */
HWTEST(TaskExecutorTest, AppExecFwk_TaskExecutor_DoWorks_0100, Function | MediumTest | Level1)
{
    GTEST_LOG_(INFO) << "AppExecFwk_TaskExecutor_DoWorks_0100 start";

    const std::shared_ptr<TaskExecutor> testTaskExecutor = CreateTaskExecutor();
    Runnable TestTask([]() { GTEST_LOG_(INFO) << "AppExecFwk_TaskExecutor_DoWorks_0100 test task"; });
    std::shared_ptr<Runnable> runnable = std::make_shared<Runnable>(TestTask);
    TaskPriority priority = TaskPriority::DEFAULT;
    std::shared_ptr<Task> firstTask = std::make_shared<Task>(runnable, priority, nullptr);
    const std::shared_ptr<ThreadFactory> factory = std::make_shared<DefaultThreadFactory>();

    std::shared_ptr<WorkerThread> worker = std::make_shared<WorkerThread>(testTaskExecutor, firstTask, factory);
    testTaskExecutor->DoWorks(worker);
    sleep(1);
    GTEST_LOG_(INFO) << "AppExecFwk_TaskExecutor_DoWorks_0100 end";
}

/**
 * @tc.number: AppExecFwk_TaskExecutor_DoWorks_0200
 * @tc.name: DoWorks
 * @tc.desc: Test the abnormal state of doWorks
 *
 */
HWTEST(TaskExecutorTest, AppExecFwk_TaskExecutor_DoWorks_0200, Function | MediumTest | Level1)
{
    GTEST_LOG_(INFO) << "AppExecFwk_TaskExecutor_DoWorks_0200 start";

    const std::shared_ptr<TaskExecutor> testTaskExecutor = CreateTaskExecutor();
    testTaskExecutor->DoWorks(nullptr);

    GTEST_LOG_(INFO) << "AppExecFwk_TaskExecutor_DoWorks_0200 end";
}

/**
 * @tc.number: AppExecFwk_TaskExecutor_DoWorks_0300
 * @tc.name: DoWorks
 * @tc.desc: Test whether doWorks are called normally
 *
 */
HWTEST(TaskExecutorTest, AppExecFwk_TaskExecutor_DoWorks_0300, Function | MediumTest | Level1)
{
    GTEST_LOG_(INFO) << "AppExecFwk_TaskExecutor_DoWorks_0300 start";

    const std::shared_ptr<TaskExecutor> testTaskExecutor = CreateTaskExecutor();

    Runnable TestTask([]() { GTEST_LOG_(INFO) << "AppExecFwk_TaskExecutor_DoWorks_0300 test task"; });
    std::shared_ptr<Runnable> runnable = std::make_shared<Runnable>(TestTask);
    TaskPriority priority = TaskPriority::DEFAULT;
    std::shared_ptr<Task> firstTask = std::make_shared<Task>(runnable, priority, nullptr);
    const std::shared_ptr<ThreadFactory> factory = std::make_shared<DefaultThreadFactory>();
    std::shared_ptr<WorkerThread> worker = std::make_shared<WorkerThread>(testTaskExecutor, firstTask, factory);
    std::thread doWorksThread([&]() {
        worker->CreateThread();  // doWorks
    });

    doWorksThread.join();
    sleep(1);
    GTEST_LOG_(INFO) << "AppExecFwk_TaskExecutor_DoWorks_0300 end";
}

/**
 * @tc.number: AppExecFwk_TaskExecutor_GetPendingTasksSize_0100
 * @tc.name: GetPendingTasksSize
 * @tc.desc: Test whether the getPendingTasksSize return value equal to 0.
 *
 */
HWTEST(TaskExecutorTest, AppExecFwk_TaskExecutor_GetPendingTasksSize_0100, Function | MediumTest | Level1)
{
    GTEST_LOG_(INFO) << "AppExecFwk_TaskExecutor_GetPendingTasksSize_0100 start";

    const std::shared_ptr<TaskExecutor> testTaskExecutor = CreateTaskExecutor();
    int size = testTaskExecutor->GetPendingTasksSize();
    EXPECT_EQ(0, size);
    GTEST_LOG_(INFO) << "AppExecFwk_TaskExecutor_GetPendingTasksSize_0100 end";
}
}  // namespace AppExecFwk
}  // namespace OHOS