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

#include <string>
#include "task.h"

namespace OHOS {
namespace AppExecFwk {
using namespace testing::ext;
using namespace OHOS::AppExecFwk;

class MyTaskListener : public TaskListener {
public:
    void OnChanged(const TaskStage &stage)
    {
        stage_ = stage.GetIndex();
    }
    int stage_;
};

class TaskTest : public testing::Test {
public:
    static void SetUpTestCase(void);
    static void TearDownTestCase(void);
    void SetUp();
    void TearDown();
    std::shared_ptr<Task> CreateTask(std::string &runStr, TaskPriority priority);
};

void TaskTest::SetUpTestCase(void)
{}

void TaskTest::TearDownTestCase(void)
{}

void TaskTest::SetUp(void)
{}

void TaskTest::TearDown(void)
{}

std::shared_ptr<Task> CreateTask(std::string &runStr, TaskPriority priority)
{
    std::shared_ptr<Runnable> runnable = std::make_shared<Runnable>([&]() { GTEST_LOG_(INFO) << runStr; });
    std::shared_ptr<Task> task = std::make_shared<Task>(runnable, priority, nullptr);
    return task;
};

/**
 * @tc.number: AppExecFwk_Task_ConstructorTest_001
 * @tc.name: TaskConstructor
 * @tc.desc: Test Task Constructor.
 */
HWTEST(TaskTest, AppExecFwk_Task_ConstructorTest_001, TestSize.Level0)
{
    auto name = std::string("AppExecFwk_Task_ConstructorTest_001");
    GTEST_LOG_(INFO) << name << " start";

    std::string highTaskRun = "Task_Run: HIGH Task";
    TaskPriority highPriority = TaskPriority::HIGH;
    std::shared_ptr<Task> highTask = CreateTask(highTaskRun, highPriority);
    EXPECT_TRUE((highTask != nullptr));
    EXPECT_EQ(highPriority, highTask->GetPriority());
    highTask->Run();

    std::string defaultTaskRun = "Task_Run: DEFAULT Task";
    TaskPriority defaultPriority = TaskPriority::DEFAULT;
    std::shared_ptr<Task> defaultTask = CreateTask(defaultTaskRun, defaultPriority);
    EXPECT_TRUE((defaultTask != nullptr));
    EXPECT_EQ(defaultPriority, defaultTask->GetPriority());
    defaultTask->Run();

    std::string lowTaskRun = "Task_Run: DEFAULT Task";
    TaskPriority lowPriority = TaskPriority::LOW;
    std::shared_ptr<Task> lowTask = CreateTask(lowTaskRun, lowPriority);
    EXPECT_TRUE((lowTask != nullptr));
    EXPECT_EQ(lowPriority, lowTask->GetPriority());
    lowTask->Run();

    GTEST_LOG_(INFO) << name << " end";
}

/**
 * @tc.number: AppExecFwk_Task_Run_001
 * @tc.name: Run
 * @tc.desc: Test task executor Runnable
 */
HWTEST(TaskTest, AppExecFwk_Task_Run_001, TestSize.Level0)
{
    auto name = std::string("AppExecFwk_Task_Run_001");
    GTEST_LOG_(INFO) << name << " start";
    std::string taskRun = "Task_Run:  task run";
    std::shared_ptr<Task> task = CreateTask(taskRun, TaskPriority::DEFAULT);
    task->Run();
    EXPECT_FALSE(task->Revoke());
    GTEST_LOG_(INFO) << name << " end";
}

/**
 * @tc.number: AppExecFwk_Task_Sequence_001
 * @tc.name: GetSequence,SetSequence
 * @tc.desc: Test get sequence and set sequence
 */
HWTEST(TaskTest, AppExecFwk_Task_Sequence_001, TestSize.Level0)
{
    auto name = std::string("AppExecFwk_Task_Sequence_001");
    GTEST_LOG_(INFO) << name << " start";
    std::string taskRun = "Task_Run:  Task_Sequence";
    std::shared_ptr<Task> task = CreateTask(taskRun, TaskPriority::DEFAULT);
    long sequence = 10;
    task->SetSequence(sequence);
    EXPECT_EQ(10, task->GetSequence());
    GTEST_LOG_(INFO) << name << " end";
}

/**
 * @tc.number: AppExecFwk_Task_GetPriority_001
 * @tc.name: GetPriority
 * @tc.desc: Test get priority
 */
HWTEST(TaskTest, AppExecFwk_Task_GetPriority_001, TestSize.Level0)
{
    auto name = std::string("AppExecFwk_Task_GetPriority_001");
    GTEST_LOG_(INFO) << name << " start";
    std::string taskRun = "Task_Run: GetPriority DEFAULT Task";
    std::shared_ptr<Task> task = CreateTask(taskRun, TaskPriority::LOW);
    EXPECT_EQ(TaskPriority::LOW, task->GetPriority());
    GTEST_LOG_(INFO) << name << " end";
}

/**
 * @tc.number: AppExecFwk_Task_Revoke_001
 * @tc.name: Revoke
 * @tc.desc: Test task revoke true
 */
HWTEST(TaskTest, AppExecFwk_Task_Revoke_001, TestSize.Level0)
{
    auto name = std::string("AppExecFwk_Task_Revoke_001");
    GTEST_LOG_(INFO) << name << " start";
    std::string taskRun = "Task_Run: Revoke  Task";
    std::shared_ptr<Task> task = CreateTask(taskRun, TaskPriority::DEFAULT);
    EXPECT_TRUE(task->Revoke());
    GTEST_LOG_(INFO) << name << " end";
}

/**
 * @tc.number: AppExecFwk_Task_Revoke_002
 * @tc.name: Revoke
 * @tc.desc: Test task revoke false
 */
HWTEST(TaskTest, AppExecFwk_Task_Revoke_002, TestSize.Level0)
{
    auto name = std::string("AppExecFwk_Task_Revoke_002");
    GTEST_LOG_(INFO) << name << " start";
    std::string taskRun = "Task_Run: Revoke  Task";
    std::shared_ptr<Task> task = CreateTask(taskRun, TaskPriority::DEFAULT);
    task->Run();
    EXPECT_FALSE(task->Revoke());
    GTEST_LOG_(INFO) << name << " end";
}

/**
 * @tc.number: AppExecFwk_Task_AddTaskListener_001
 * @tc.name: AddTaskListener
 * @tc.desc: Test task add task listener
 */
HWTEST(TaskTest, AppExecFwk_Task_AddTaskListener_001, TestSize.Level0)
{
    auto name = std::string("AppExecFwk_Task_AddTaskListener_001");
    GTEST_LOG_(INFO) << name << " start";
    std::string taskRun = "Task_Run: AddTaskListener";
    std::shared_ptr<Task> task = CreateTask(taskRun, TaskPriority::DEFAULT);
    std::shared_ptr<MyTaskListener> taskListener1 = std::make_shared<MyTaskListener>();
    task->AddTaskListener(taskListener1);
    std::shared_ptr<MyTaskListener> taskListener2 = std::make_shared<MyTaskListener>();
    task->AddTaskListener(taskListener2);
    task->AfterTaskExecute();
    EXPECT_EQ(taskListener1->stage_, 0);
    EXPECT_EQ(taskListener2->stage_, 0);
    task->Run();
    task->AfterTaskExecute();
    EXPECT_EQ(taskListener1->stage_, 1);
    EXPECT_EQ(taskListener2->stage_, 1);
    GTEST_LOG_(INFO) << name << " end";
}

/**
 * @tc.number: AppExecFwk_Task_BeforeTaskExecute_001
 * @tc.name: BeforeTaskExecute
 * @tc.desc: Test task before task execute
 */
HWTEST(TaskTest, AppExecFwk_Task_BeforeTaskExecute_001, TestSize.Level0)
{
    auto name = std::string("AppExecFwk_Task_BeforeTaskExecute_001");
    GTEST_LOG_(INFO) << name << " start";
    std::string taskRun = "Task_Run: BeforeTaskExecute";
    std::shared_ptr<Task> task = CreateTask(taskRun, TaskPriority::DEFAULT);
    std::shared_ptr<MyTaskListener> taskListener = std::make_shared<MyTaskListener>();
    task->AddTaskListener(taskListener);
    task->BeforeTaskExecute();
    EXPECT_EQ(taskListener->stage_, TaskStage::BEFORE_EXECUTE);
    GTEST_LOG_(INFO) << name << " end";
}

/**
 * @tc.number: AppExecFwk_Task_AfterTaskExecute_001
 * @tc.name: AfterTaskExecute
 * @tc.desc: Test after task execute
 */
HWTEST(TaskTest, AppExecFwk_Task_AfterTaskExecute_001, TestSize.Level0)
{
    auto name = std::string("AppExecFwk_Task_AfterTaskExecute_001");
    GTEST_LOG_(INFO) << name << " start";
    std::string taskRun = "Task_Run: AfterTaskExecute";
    std::shared_ptr<Task> task = CreateTask(taskRun, TaskPriority::DEFAULT);
    std::shared_ptr<MyTaskListener> taskListener = std::make_shared<MyTaskListener>();
    task->AddTaskListener(taskListener);
    task->AfterTaskExecute();
    EXPECT_EQ(taskListener->stage_, 0);
    task->Run();
    task->AfterTaskExecute();
    EXPECT_EQ(taskListener->stage_, 1);
    GTEST_LOG_(INFO) << name << " end";
}

/**
 * @tc.number: AppExecFwk_Task_OnTaskCanceled_001
 * @tc.name: OnTaskCanceled
 * @tc.desc: Test task canceled
 */
HWTEST(TaskTest, AppExecFwk_Task_OnTaskCanceled_001, TestSize.Level0)
{
    auto name = std::string("AppExecFwk_Task_OnTaskCanceled_001");
    GTEST_LOG_(INFO) << name << " start";
    std::string taskRun = "Task_Run: OnTaskCanceled";
    std::shared_ptr<Task> task = CreateTask(taskRun, TaskPriority::DEFAULT);
    std::shared_ptr<MyTaskListener> taskListener = std::make_shared<MyTaskListener>();
    task->AddTaskListener(taskListener);
    task->OnTaskCanceled();
    EXPECT_EQ(taskListener->stage_, 2);
    GTEST_LOG_(INFO) << name << " end";
}
}  // namespace AppExecFwk
}  // namespace OHOS