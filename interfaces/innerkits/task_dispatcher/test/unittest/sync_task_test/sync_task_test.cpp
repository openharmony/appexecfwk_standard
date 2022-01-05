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
#include "sync_task.h"
#undef private
#include "app_log_wrapper.h"
#include "default_worker_pool_config.h"
#include "task.h"
#include "task_priority.h"
#include "task_executor.h"

using namespace testing;
using namespace testing::ext;
using namespace OHOS::AppExecFwk;

class SyncTaskTest : public testing::Test {
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
 * Feature: SyncTask
 * Function: SyncTask
 * SubFunction:
 * FunctionPoints:
 * EnvConditions:
 * CaseDescription: Verify if new SyncTask with the valid params will return default value.
 */
HWTEST(SyncTaskTest, SyncTask_ConstructorTest_001, TestSize.Level0)
{
    auto name = std::string("SyncTask_ConstructorTest_001");
    GTEST_LOG_(INFO) << name + " start";
    auto runnable = std::make_shared<Runnable>([]() {
        long wait = 200;
        auto time = std::chrono::milliseconds(wait);
        std::this_thread::sleep_for(time);
        APP_LOGD("task %{public}d end", 0);
    });
    auto priorityLow = TaskPriority::LOW;
    auto priorityHigh = TaskPriority::HIGH;

    {
        SyncTask task(runnable, priorityLow, nullptr);
        EXPECT_EQ(task.GetPriority(), priorityLow);
    }
    {
        SyncTask task(runnable, priorityHigh, nullptr);
        EXPECT_EQ(task.GetPriority(), priorityHigh);
    }

    GTEST_LOG_(INFO) << name + " end";
}

/*
 * Feature: SyncTask
 * Function: Run
 * SubFunction:
 * FunctionPoints:
 * EnvConditions:
 * CaseDescription: verify Run
 */
HWTEST(SyncTaskTest, SyncTask_RunTest_001, TestSize.Level0)
{
    auto name = std::string("SyncTask_RunTest_001");
    GTEST_LOG_(INFO) << name + " start";
    std::atomic<int> count(0);
    auto runnable = std::make_shared<Runnable>([&count]() {
        long wait = 200;
        auto time = std::chrono::milliseconds(wait);
        std::this_thread::sleep_for(time);
        count++;
        APP_LOGD("task %{public}d end", count.load());
    });
    auto priority = TaskPriority::DEFAULT;
    {
        SyncTask task(runnable, priority, nullptr);
        // run task
        Runnable taskRunner = std::bind(&SyncTask::Run, &task);
        std::thread taskThread = std::thread(taskRunner);

        EXPECT_EQ(count.load(), 0);
        taskThread.join();
        EXPECT_EQ(count.load(), 1);
    }

    GTEST_LOG_(INFO) << name + " end";
}

/*
 * Feature: SyncTask
 * Function: WaitTask
 * SubFunction:
 * FunctionPoints:
 * EnvConditions:
 * CaseDescription: verify wait task complete
 */
HWTEST(SyncTaskTest, SyncTask_WaitTest_001, TestSize.Level0)
{
    auto name = std::string("SyncTask_WaitTest_001");
    GTEST_LOG_(INFO) << name + " start";
    std::atomic<int> count(0);
    auto runnable = std::make_shared<Runnable>([&count]() {
        long wait = 1000;
        auto time = std::chrono::milliseconds(wait);
        std::this_thread::sleep_for(time);
        count++;
        APP_LOGD("task %{public}d end", count.load());
    });
    auto priority = TaskPriority::LOW;
    {
        SyncTask task(runnable, priority, nullptr);
        // run task
        std::thread taskThread = std::thread(std::bind(&SyncTask::Run, &task));
        std::thread taskWaitThread = std::thread(std::bind(&SyncTask::WaitTask, &task));

        EXPECT_EQ(count.load(), 0);
        taskThread.join();
        taskWaitThread.join();
        EXPECT_EQ(count.load(), 1);
    }

    GTEST_LOG_(INFO) << name + " end";
}
