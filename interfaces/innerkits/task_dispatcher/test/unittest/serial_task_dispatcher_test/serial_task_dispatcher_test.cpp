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
#include "serial_task_dispatcher.h"
#undef private
#include "app_log_wrapper.h"
#include "appexecfwk_errors.h"
#include "default_worker_pool_config.h"
#include "task.h"
#include "task_executor.h"
#include "task_priority.h"

using namespace testing;
using namespace testing::ext;
using namespace OHOS::AppExecFwk;
using namespace OHOS;

class SerialTaskDispatcherTest : public testing::Test {
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

HWTEST(SerialTaskDispatcherTest, SerialTaskDispatcher_SyncDispatchTest_1000, TestSize.Level0)
{
    auto name = std::string("SerialTaskDispatcher_SyncDispatchTest_1000");
    GTEST_LOG_(INFO) << name + " start";
    auto config = std::make_shared<DefaultWorkerPoolConfig>();
    auto executor = std::make_shared<TaskExecutor>(config);
    auto priority = TaskPriority::HIGH;
    std::shared_ptr<SerialTaskDispatcher> dispatcher = std::make_shared<SerialTaskDispatcher>(name, priority, executor);

    std::atomic<int> count(0);
    EXPECT_TRUE(count.load() == 0);
    dispatcher->SyncDispatch(std::make_shared<Runnable>([&count, &name]() {
        int index = count.fetch_add(1);
        EXPECT_TRUE(index == 0);
        GTEST_LOG_(INFO) << name + " task " + std::to_string(index) + " end";
    }));

    EXPECT_TRUE(count.load() == 1);

    GTEST_LOG_(INFO) << name + " end";
}

/*
 * Feature: SerialTaskDispatch
 * Function: SerialTaskDispatch
 * SubFunction:
 * FunctionPoints:
 * EnvConditions:
 * CaseDescription: Verify if new SerialTaskDispatch with the valid params will return default value.
 */
HWTEST(SerialTaskDispatcherTest, SerialTaskDispatcher_ConstructorTest_001, TestSize.Level0)
{
    auto name = std::string("SerialTaskDispatcher_ConstructorTest_001");
    GTEST_LOG_(INFO) << name + " start";
    auto config = std::make_shared<DefaultWorkerPoolConfig>();
    auto executor = std::make_shared<TaskExecutor>(config);
    auto priority = TaskPriority::DEFAULT;

    {
        std::shared_ptr<SerialTaskDispatcher> dispatcher =
            std::make_shared<SerialTaskDispatcher>(name, priority, executor);

        EXPECT_EQ(dispatcher->GetDispatcherName(), name);
        EXPECT_EQ(dispatcher->GetPriority(), priority);
        EXPECT_EQ(dispatcher->GetWorkingTasksSize(), 0);
    }

    GTEST_LOG_(INFO) << name + " end";
}
HWTEST(SerialTaskDispatcherTest, SerialTaskDispatcher_ConstructorTest_002, TestSize.Level0)
{
    auto name = std::string("SerialTaskDispatcher_ConstructorTest_002");
    GTEST_LOG_(INFO) << name + " start";
    auto config = std::make_shared<DefaultWorkerPoolConfig>();
    auto executor = std::make_shared<TaskExecutor>(config);
    auto priority = TaskPriority::LOW;

    {
        std::shared_ptr<SerialTaskDispatcher> dispatcher =
            std::make_shared<SerialTaskDispatcher>(name, priority, executor);

        EXPECT_EQ(dispatcher->GetDispatcherName(), name);
        EXPECT_EQ(dispatcher->GetPriority(), priority);
        EXPECT_EQ(dispatcher->GetWorkingTasksSize(), 0);
    }

    GTEST_LOG_(INFO) << name + " end";
}
HWTEST(SerialTaskDispatcherTest, SerialTaskDispatcher_ConstructorTest_003, TestSize.Level0)
{
    auto name = std::string("SerialTaskDispatcher_ConstructorTest_003");
    GTEST_LOG_(INFO) << name + " start";
    auto config = std::make_shared<DefaultWorkerPoolConfig>();
    auto executor = std::make_shared<TaskExecutor>(config);
    auto priority = TaskPriority::HIGH;

    {
        std::shared_ptr<SerialTaskDispatcher> dispatcher =
            std::make_shared<SerialTaskDispatcher>(name, priority, executor);

        EXPECT_EQ(dispatcher->GetDispatcherName(), name);
        EXPECT_EQ(dispatcher->GetPriority(), priority);
        EXPECT_EQ(dispatcher->GetWorkingTasksSize(), 0);
    }

    GTEST_LOG_(INFO) << name + " end";
}
HWTEST(SerialTaskDispatcherTest, SerialTaskDispatcher_ConstructorTest_004, TestSize.Level0)
{
    auto name = std::string("SerialTaskDispatcher_ConstructorTest_004");
    GTEST_LOG_(INFO) << name + " start";
    auto config = std::make_shared<DefaultWorkerPoolConfig>();
    auto executor = std::make_shared<TaskExecutor>(config);
    auto priority = TaskPriority::HIGH;

    {
        std::shared_ptr<SerialTaskDispatcher> dispatcher =
            std::make_shared<SerialTaskDispatcher>("", priority, executor);

        EXPECT_EQ(dispatcher->GetDispatcherName(), "Dispatcher-0");
        EXPECT_EQ(dispatcher->GetPriority(), priority);
        EXPECT_EQ(dispatcher->GetWorkingTasksSize(), 0);
    }

    {
        std::shared_ptr<SerialTaskDispatcher> dispatcher =
            std::make_shared<SerialTaskDispatcher>("", priority, executor);

        EXPECT_EQ(dispatcher->GetDispatcherName(), "Dispatcher-1");
        EXPECT_EQ(dispatcher->GetPriority(), priority);
        EXPECT_EQ(dispatcher->GetWorkingTasksSize(), 0);
    }

    GTEST_LOG_(INFO) << name + " end";
}
HWTEST(SerialTaskDispatcherTest, SerialTaskDispatcher_ConstructorTest_005, TestSize.Level0)
{
    auto name = std::string("SerialTaskDispatcher_ConstructorTest_005");
    GTEST_LOG_(INFO) << name + " start";
    auto config = std::make_shared<DefaultWorkerPoolConfig>();
    auto executor = std::make_shared<TaskExecutor>(config);
    auto priority = TaskPriority::HIGH;

    {
        std::shared_ptr<SerialTaskDispatcher> dispatcher =
            std::make_shared<SerialTaskDispatcher>("", priority, executor);

        EXPECT_EQ(dispatcher->GetDispatcherName(), "Dispatcher-2");
        EXPECT_EQ(dispatcher->GetPriority(), priority);
        EXPECT_EQ(dispatcher->GetWorkingTasksSize(), 0);
    }
    {
        std::shared_ptr<SerialTaskDispatcher> dispatcher =
            std::make_shared<SerialTaskDispatcher>("", priority, executor);
        EXPECT_EQ(dispatcher->GetDispatcherName(), "Dispatcher-3");
        EXPECT_EQ(dispatcher->GetPriority(), priority);
        EXPECT_EQ(dispatcher->GetWorkingTasksSize(), 0);
    }

    GTEST_LOG_(INFO) << name + " end";
}

/*
 * Feature: SerialTaskDispatch
 * Function: GetWorkingTasksSize
 * SubFunction:
 * FunctionPoints:
 * EnvConditions:
 * CaseDescription: verify working task size changes
 */
HWTEST(SerialTaskDispatcherTest, SerialTaskDispatcher_GetWorkingTasksSizeTest_001, TestSize.Level0)
{
    auto name = std::string("SerialTaskDispatcher_GetWorkingTasksSizeTest_001");
    GTEST_LOG_(INFO) << name + " start";
    auto config = std::make_shared<DefaultWorkerPoolConfig>();
    auto executor = std::make_shared<TaskExecutor>(config);
    auto priority = TaskPriority::HIGH;
    std::shared_ptr<SerialTaskDispatcher> dispatcher = std::make_shared<SerialTaskDispatcher>("", priority, executor);

    EXPECT_EQ(dispatcher->GetDispatcherName(), std::string("Dispatcher-4"));
    EXPECT_EQ(dispatcher->GetPriority(), priority);
    EXPECT_EQ(dispatcher->GetWorkingTasksSize(), 0);

    dispatcher->AsyncDispatch(std::make_shared<Runnable>([]() {
        long wait = 2000;
        auto time = std::chrono::milliseconds(wait);
        std::this_thread::sleep_for(time);
        GTEST_LOG_(INFO) << "SerialTaskDispatcher_GetWorkingTasksSizeTest_001 task0 end ";
    }));
    dispatcher->AsyncDispatch(std::make_shared<Runnable>([]() {
        long wait = 4000;
        auto time = std::chrono::milliseconds(wait);
        std::this_thread::sleep_for(time);
        GTEST_LOG_(INFO) << "SerialTaskDispatcher_GetWorkingTasksSizeTest_001 task1 end ";
    }));
    {
        long wait = 500;
        auto time = std::chrono::milliseconds(wait);
        std::this_thread::sleep_for(time);
        GTEST_LOG_(INFO) << name + " size: " + std::to_string(dispatcher->GetWorkingTasksSize());
        EXPECT_EQ(dispatcher->GetWorkingTasksSize(), 2 - 1);
    }
    {
        long wait = 2000;
        auto time = std::chrono::milliseconds(wait);
        std::this_thread::sleep_for(time);
        GTEST_LOG_(INFO) << name + " size: " + std::to_string(dispatcher->GetWorkingTasksSize());
        EXPECT_EQ(dispatcher->GetWorkingTasksSize(), 1 - 1);
    }
    {
        long wait = 2000;
        auto time = std::chrono::milliseconds(wait);
        std::this_thread::sleep_for(time);
        GTEST_LOG_(INFO) << name + " size: " + std::to_string(dispatcher->GetWorkingTasksSize());
        EXPECT_EQ(dispatcher->GetWorkingTasksSize(), 0);
    }
    long wait = 5000;
    auto time = std::chrono::milliseconds(wait);
    std::this_thread::sleep_for(time);

    GTEST_LOG_(INFO) << name + " end";
}
HWTEST(SerialTaskDispatcherTest, SerialTaskDispatcher_GetWorkingTasksSizeTest_002, TestSize.Level0)
{
    auto name = std::string("SerialTaskDispatcher_GetWorkingTasksSizeTest_002");
    GTEST_LOG_(INFO) << name + " start";
    auto config = std::make_shared<DefaultWorkerPoolConfig>();
    auto executor = std::make_shared<TaskExecutor>(config);
    auto priority = TaskPriority::HIGH;
    std::shared_ptr<SerialTaskDispatcher> dispatcher = std::make_shared<SerialTaskDispatcher>("", priority, executor);

    EXPECT_EQ(dispatcher->GetDispatcherName(), std::string("Dispatcher-5"));
    EXPECT_EQ(dispatcher->GetPriority(), priority);
    EXPECT_EQ(dispatcher->GetWorkingTasksSize(), 0);

    int times = 100;
    for (int i = 0; i < times; i++) {
        dispatcher->AsyncDispatch(std::make_shared<Runnable>([i]() {
            long wait = 200 + i / 2;
            auto time = std::chrono::milliseconds(wait);
            std::this_thread::sleep_for(time);
            GTEST_LOG_(INFO) << "SerialTaskDispatcher_GetWorkingTasksSizeTest_002 task" + std::to_string(i) + "end";
        }));
    }

    {
        long wait = 100;
        auto time = std::chrono::milliseconds(wait);
        std::this_thread::sleep_for(time);
        GTEST_LOG_(INFO) << name + " size: " + std::to_string(dispatcher->GetWorkingTasksSize());
        EXPECT_EQ(dispatcher->GetWorkingTasksSize(), times - 1);
    }
    {
        long wait = 300 * (long)times;
        auto time = std::chrono::milliseconds(wait);
        std::this_thread::sleep_for(time);
        GTEST_LOG_(INFO) << name + " size: " + std::to_string(dispatcher->GetWorkingTasksSize());
        EXPECT_EQ(dispatcher->GetWorkingTasksSize(), 0);
    }

    GTEST_LOG_(INFO) << name + " end";
}

/*
 * Feature: SerialTaskDispatch
 * Function: SyncDispatch
 * SubFunction:
 * FunctionPoints:
 * EnvConditions:
 * CaseDescription: verify synchronized task‘s executing sequence: serial. do wait task.
 */
HWTEST(SerialTaskDispatcherTest, SerialTaskDispatcher_SyncDispatchTest_001, TestSize.Level0)
{
    auto name = std::string("SerialTaskDispatcher_SyncDispatchTest_001");
    GTEST_LOG_(INFO) << name + " start";
    auto config = std::make_shared<DefaultWorkerPoolConfig>();
    auto executor = std::make_shared<TaskExecutor>(config);
    auto priority = TaskPriority::HIGH;
    std::shared_ptr<SerialTaskDispatcher> dispatcher = std::make_shared<SerialTaskDispatcher>("", priority, executor);

    std::atomic<int> count(0);
    EXPECT_TRUE(count.load() == 0);
    int sleep1 = 2000;
    dispatcher->SyncDispatch(std::make_shared<Runnable>([&count, sleep = sleep1]() {
        auto time = std::chrono::milliseconds(sleep);
        std::this_thread::sleep_for(time);
        int index = count.fetch_add(1);
        EXPECT_TRUE(index == 0);
        GTEST_LOG_(INFO) << "SerialTaskDispatcher_SyncDispatchTest_001 task" + std::to_string(index) +
                                " end, elapsed " + std::to_string(sleep);
    }));
    EXPECT_TRUE(count.load() == 1);

    int sleep2 = 100;
    dispatcher->SyncDispatch(std::make_shared<Runnable>([&count, sleep = sleep2]() {
        auto time = std::chrono::milliseconds(sleep);
        std::this_thread::sleep_for(time);
        int index = count.fetch_add(1);
        EXPECT_TRUE(index == 1);
        GTEST_LOG_(INFO) << "SerialTaskDispatcher_SyncDispatchTest_001 task" + std::to_string(index) +
                                " end, elapsed " + std::to_string(sleep);
    }));
    EXPECT_TRUE(count.load() == 2);

    int sleep3 = 1000;
    dispatcher->SyncDispatch(std::make_shared<Runnable>([&count, sleep = sleep3]() {
        auto time = std::chrono::milliseconds(sleep);
        std::this_thread::sleep_for(time);
        int index = count.fetch_add(1);
        EXPECT_TRUE(index == 2);
        GTEST_LOG_(INFO) << "SerialTaskDispatcher_SyncDispatchTest_001 task" + std::to_string(index) +
                                " end, elapsed " + std::to_string(sleep);
    }));
    EXPECT_TRUE(count.load() == 3);

    long wait = 1000;
    GTEST_LOG_(INFO) << ("wait for " + std::to_string(wait));
    auto time = std::chrono::milliseconds(wait);
    std::this_thread::sleep_for(time);

    GTEST_LOG_(INFO) << name + " end";
}
HWTEST(SerialTaskDispatcherTest, SerialTaskDispatcher_SyncDispatchTest_002, TestSize.Level0)
{
    auto name = std::string("SerialTaskDispatcher_SyncDispatchTest_002");
    GTEST_LOG_(INFO) << name + " start";
    auto config = std::make_shared<DefaultWorkerPoolConfig>();
    auto executor = std::make_shared<TaskExecutor>(config);
    auto priority = TaskPriority::HIGH;
    std::shared_ptr<SerialTaskDispatcher> dispatcher = std::make_shared<SerialTaskDispatcher>("", priority, executor);

    ErrCode errCode = dispatcher->SyncDispatch(nullptr);
    EXPECT_TRUE(errCode == ERR_APPEXECFWK_CHECK_FAILED);

    GTEST_LOG_(INFO) << name + " end";
}

/*
 * Feature: SerialTaskDispatch
 * Function: AsyncDispatch
 * SubFunction:
 * FunctionPoints:
 * EnvConditions:
 * CaseDescription: verify asynchronized task‘s executing sequence: serial. donot wait task.
 */
HWTEST(SerialTaskDispatcherTest, SerialTaskDispatcher_AsyncDispatchTest_001, TestSize.Level0)
{
    auto name = std::string("SerialTaskDispatcher_AsyncDispatchTest_001");
    GTEST_LOG_(INFO) << name + " start";
    auto config = std::make_shared<DefaultWorkerPoolConfig>();
    auto executor = std::make_shared<TaskExecutor>(config);
    auto priority = TaskPriority::HIGH;
    std::shared_ptr<SerialTaskDispatcher> dispatcher = std::make_shared<SerialTaskDispatcher>("", priority, executor);

    std::atomic<int> count(0);
    EXPECT_TRUE(count.load() == 0);
    int sleep1 = 2000;
    dispatcher->AsyncDispatch(std::make_shared<Runnable>([&count, sleep = sleep1]() {
        auto time = std::chrono::milliseconds(sleep);
        std::this_thread::sleep_for(time);
        int index = count.fetch_add(1);
        EXPECT_TRUE(index == 0);
        GTEST_LOG_(INFO) << "SerialTaskDispatcher_AsyncDispatchTest_001 task" + std::to_string(index) +
                                " end, elapsed " + std::to_string(sleep);
    }));
    EXPECT_TRUE(count.load() == 0);

    int sleep2 = 1000;
    dispatcher->AsyncDispatch(std::make_shared<Runnable>([&count, sleep = sleep2]() {
        auto time = std::chrono::milliseconds(sleep);
        std::this_thread::sleep_for(time);
        int index = count.fetch_add(1);
        EXPECT_TRUE(index == 1);
        GTEST_LOG_(INFO) << "SerialTaskDispatcher_AsyncDispatchTest_001 task" + std::to_string(index) +
                                " end, elapsed " + std::to_string(sleep);
    }));
    EXPECT_TRUE(count.load() == 0);

    int sleep3 = 1000;
    dispatcher->AsyncDispatch(std::make_shared<Runnable>([&count, sleep = sleep3]() {
        auto time = std::chrono::milliseconds(sleep);
        std::this_thread::sleep_for(time);
        int index = count.fetch_add(1);
        EXPECT_TRUE(index == 2);
        GTEST_LOG_(INFO) << "SerialTaskDispatcher_AsyncDispatchTest_001 task" + std::to_string(index) +
                                " end, elapsed " + std::to_string(sleep);
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

    GTEST_LOG_(INFO) << name + " end";
}
HWTEST(SerialTaskDispatcherTest, SerialTaskDispatcher_AsyncDispatchTest_002, TestSize.Level0)
{
    auto name = std::string("SerialTaskDispatcher_AsyncDispatchTest_002");
    GTEST_LOG_(INFO) << name + " start";
    auto config = std::make_shared<DefaultWorkerPoolConfig>();
    auto executor = std::make_shared<TaskExecutor>(config);
    auto priority = TaskPriority::HIGH;
    std::shared_ptr<SerialTaskDispatcher> dispatcher = std::make_shared<SerialTaskDispatcher>("", priority, executor);

    std::shared_ptr<Revocable> revocable = dispatcher->AsyncDispatch(nullptr);
    EXPECT_TRUE(revocable == nullptr);

    GTEST_LOG_(INFO) << name + " end";
}

/*
 * Feature: SerialTaskDispatch
 * Function: DelayDispatch
 * SubFunction:
 * FunctionPoints:
 * EnvConditions:
 * CaseDescription: verify asynchronized task‘s executing sequence: delay, serial. donot wait task.
 */
HWTEST(SerialTaskDispatcherTest, SerialTaskDispatcher_DelayDispatchTest_001, TestSize.Level0)
{
    auto name = std::string("SerialTaskDispatcher_DelayDispatchTest_001");
    GTEST_LOG_(INFO) << name + " start";
    auto config = std::make_shared<DefaultWorkerPoolConfig>();
    auto executor = std::make_shared<TaskExecutor>(config);
    auto priority = TaskPriority::HIGH;
    std::shared_ptr<SerialTaskDispatcher> dispatcher = std::make_shared<SerialTaskDispatcher>("", priority, executor);

    std::atomic<int> count(0);
    int sleep1 = 2000;
    dispatcher->DelayDispatch(std::make_shared<Runnable>([&count, sleep = sleep1]() {
        int index = count.fetch_add(1);
        EXPECT_TRUE(index == 2);
        GTEST_LOG_(INFO) << "SerialTaskDispatcher_DelayDispatchTest_001 task" + std::to_string(index) +
                                " end, elapsed " + std::to_string(sleep);
    }),
        sleep1);
    EXPECT_TRUE(count.load() == 0);

    int sleep2 = 1000;
    dispatcher->DelayDispatch(std::make_shared<Runnable>([&count, sleep = sleep2]() {
        int index = count.fetch_add(1);
        EXPECT_TRUE(index == 0 || index == 1);
        GTEST_LOG_(INFO) << "SerialTaskDispatcher_DelayDispatchTest_001 task" + std::to_string(index) +
                                " end, elapsed " + std::to_string(sleep);
    }),
        sleep2);
    EXPECT_TRUE(count.load() == 0);

    int sleep3 = 1000;
    dispatcher->DelayDispatch(std::make_shared<Runnable>([&count, sleep = sleep3]() {
        int index = count.fetch_add(1);
        EXPECT_TRUE(index == 0 || index == 1);
        GTEST_LOG_(INFO) << "SerialTaskDispatcher_DelayDispatchTest_001 task" + std::to_string(index) +
                                " end, elapsed " + std::to_string(sleep);
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
        GTEST_LOG_(INFO) << ("wait for task, load= " + std::to_string(count.load()));
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

HWTEST(SerialTaskDispatcherTest, SerialTaskDispatcher_DelayDispatchTest_002, TestSize.Level0)
{
    auto name = std::string("SerialTaskDispatcher_DelayDispatchTest_002");
    GTEST_LOG_(INFO) << name + " start";
    auto config = std::make_shared<DefaultWorkerPoolConfig>();
    auto executor = std::make_shared<TaskExecutor>(config);
    auto priority = TaskPriority::HIGH;
    std::shared_ptr<SerialTaskDispatcher> dispatcher = std::make_shared<SerialTaskDispatcher>("", priority, executor);

    int sleep1 = 2000;
    dispatcher->DelayDispatch(nullptr, sleep1);

    GTEST_LOG_(INFO) << name + " end";
}

HWTEST(SerialTaskDispatcherTest, SerialTaskDispatcher_DelayDispatchTest_003, TestSize.Level0)
{
    auto name = std::string("SerialTaskDispatcher_DelayDispatchTest_003");
    GTEST_LOG_(INFO) << name + " start";
    auto config = std::make_shared<DefaultWorkerPoolConfig>();
    auto executor = std::make_shared<TaskExecutor>(config);
    auto priority = TaskPriority::HIGH;
    std::shared_ptr<SerialTaskDispatcher> dispatcher = std::make_shared<SerialTaskDispatcher>("", priority, executor);

    std::atomic<int> count(0);
    int sleep1 = -2000;
    dispatcher->DelayDispatch(std::make_shared<Runnable>([&count, sleep = sleep1]() {
        int index = count.fetch_add(1);
        EXPECT_TRUE(index == 1 || index == 0);
        GTEST_LOG_(INFO) << "SerialTaskDispatcher_DelayDispatchTest_003 task" + std::to_string(index) +
                                " end, elapsed " + std::to_string(sleep);
    }),
        sleep1);

    int sleep2 = -1000;
    dispatcher->DelayDispatch(std::make_shared<Runnable>([&count, sleep = sleep2]() {
        int index = count.fetch_add(1);
        EXPECT_TRUE(index == 0 || index == 1);
        GTEST_LOG_(INFO) << "SerialTaskDispatcher_DelayDispatchTest_003 task" + std::to_string(index) +
                                " end, elapsed " + std::to_string(sleep);
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
