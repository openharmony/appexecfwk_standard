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

#include "worker_pool.h"
#include "default_worker_pool_config.h"
#include "worker_pool_config.h"
#include "work_thread.h"
#include "delay_execute_service.h"
#include "task.h"
#include "runnable.h"
#include "task_executor.h"

namespace OHOS {
namespace AppExecFwk {
using namespace testing::ext;
using namespace OHOS::AppExecFwk;

class WorkerPoolTest : public testing::Test {
public:
    static void SetUpTestCase(void);
    static void TearDownTestCase(void);
    void SetUp();
    void TearDown();
    std::shared_ptr<WorkerPool> createWorkerPool();
};

void WorkerPoolTest::SetUpTestCase(void)
{}

void WorkerPoolTest::TearDownTestCase(void)
{}

void WorkerPoolTest::SetUp(void)
{}

void WorkerPoolTest::TearDown(void)
{}

std::shared_ptr<WorkerPool> createWorkerPool()
{
    const std::shared_ptr<WorkerPoolConfig> config = std::make_shared<DefaultWorkerPoolConfig>();
    std::shared_ptr<WorkerPool> testWorkerPool = std::make_shared<WorkerPool>(config);
    return testWorkerPool;
}

/**
 * @tc.number: AppExecFwk_WorkerPool_Constructor_0100
 * @tc.name: Constructor
 * @tc.desc: Test whether constructor are called normally
 */
HWTEST(WorkerPoolTest, AppExecFwk_WorkerPool_Constructor_0100, Function | MediumTest | Level1)
{
    GTEST_LOG_(INFO) << "AppExecFwk_WorkerPool_Constructor_0100 start";
    const std::shared_ptr<WorkerPoolConfig> config = std::make_shared<DefaultWorkerPoolConfig>();
    std::make_shared<WorkerPool>(config);

    GTEST_LOG_(INFO) << "AppExecFwk_WorkerPool_Constructor_0100 end";
}

/**
 * @tc.number: AppExecFwk_WorkerPool_AddWorker_0100
 * @tc.name: AddWorker
 * @tc.desc: Test whether the addWorker return value is true.
 */
HWTEST(WorkerPoolTest, AppExecFwk_WorkerPool_AddWorker_0100, Function | MediumTest | Level1)
{
    GTEST_LOG_(INFO) << "AppExecFwk_WorkerPool_AddWorker_0100 start";
    Runnable TestTask([]() { GTEST_LOG_(INFO) << "AppExecFwk_WorkerPool_AddWorker_0100 task is running"; });
    const std::shared_ptr<WorkerPoolConfig> config = std::make_shared<DefaultWorkerPoolConfig>();
    const std::shared_ptr<TaskExecutor> delegate = std::make_shared<TaskExecutor>(config);
    const std::shared_ptr<Runnable> func = std::make_shared<Runnable>(TestTask);
    const std::shared_ptr<Task> task = std::make_shared<Task>(func, TaskPriority::DEFAULT, nullptr);
    std::shared_ptr<WorkerPool> testWorkerPool = createWorkerPool();
    bool addWorker = testWorkerPool->AddWorker(delegate, task);
    sleep(1);
    EXPECT_EQ(true, addWorker);

    GTEST_LOG_(INFO) << "AppExecFwk_WorkerPool_AddWorker_0100 end";
}

/**
 * @tc.number: AppExecFwk_WorkerPool_AddWorker_0200
 * @tc.name: AddWorker
 * @tc.desc: Test whether the addWorker return value is false.
 */
HWTEST(WorkerPoolTest, AppExecFwk_WorkerPool_AddWorker_0200, Function | MediumTest | Level1)
{
    GTEST_LOG_(INFO) << "AppExecFwk_WorkerPool_AddWorker_0200 start";
    Runnable TestTask([]() { GTEST_LOG_(INFO) << "AppExecFwk_WorkerPool_AddWorker_0200 task is running"; });
    const std::shared_ptr<TaskExecutor> delegate = nullptr;
    const std::shared_ptr<Runnable> func = std::make_shared<Runnable>(TestTask);
    const std::shared_ptr<Task> task = std::make_shared<Task>(func, TaskPriority::DEFAULT, nullptr);
    std::shared_ptr<WorkerPool> testWorkerPool = createWorkerPool();
    bool addWorker = testWorkerPool->AddWorker(delegate, task);
    EXPECT_EQ(false, addWorker);

    GTEST_LOG_(INFO) << "AppExecFwk_WorkerPool_AddWorker_0200 end";
}

/**
 * @tc.number: AppExecFwk_WorkerPool_AddWorker_0300
 * @tc.name: AddWorker
 * @tc.desc: Test whether the addWorker return value is false.
 */
HWTEST(WorkerPoolTest, AppExecFwk_WorkerPool_AddWorker_0300, Function | MediumTest | Level1)
{
    GTEST_LOG_(INFO) << "AppExecFwk_WorkerPool_AddWorker_0300 start";
    const std::shared_ptr<WorkerPoolConfig> config = std::make_shared<DefaultWorkerPoolConfig>();
    const std::shared_ptr<TaskExecutor> delegate = std::make_shared<TaskExecutor>(config);
    const std::shared_ptr<Task> task = nullptr;
    std::shared_ptr<WorkerPool> testWorkerPool = createWorkerPool();
    bool addWorker = testWorkerPool->AddWorker(delegate, task);
    EXPECT_EQ(false, addWorker);

    GTEST_LOG_(INFO) << "AppExecFwk_WorkerPool_AddWorker_0300 end";
}

/**
 * @tc.number: AppExecFwk_WorkerPool_GetKeepAliveTime_0100
 * @tc.name: GetKeepAliveTime
 * @tc.desc: Test whether the getKeepAliveTime return value equal to 50.
 */
HWTEST(WorkerPoolTest, AppExecFwk_WorkerPool_GetKeepAliveTime_0100, Function | MediumTest | Level1)
{
    GTEST_LOG_(INFO) << "AppExecFwk_WorkerPool_GetKeepAliveTime_0100 start";
    std::shared_ptr<WorkerPool> testWorkerPool = createWorkerPool();
    long keepAliveTime = testWorkerPool->GetKeepAliveTime();
    EXPECT_EQ(50, keepAliveTime);

    GTEST_LOG_(INFO) << "AppExecFwk_WorkerPool_GetKeepAliveTime_0100 end";
}

/**
 * @tc.number: AppExecFwk_WorkerPool_GetCoreThreadCount_0100
 * @tc.name: GetCoreThreadCount
 * @tc.desc: Test whether the getCoreThreadCount return value equal to 16.
 */
HWTEST(WorkerPoolTest, AppExecFwk_WorkerPool_GetCoreThreadCount_0100, Function | MediumTest | Level1)
{
    GTEST_LOG_(INFO) << "AppExecFwk_WorkerPool_GetCoreThreadCount_0100 start";
    std::shared_ptr<WorkerPool> testWorkerPool = createWorkerPool();
    int coreThreadCount = testWorkerPool->GetCoreThreadCount();
    EXPECT_EQ(16, coreThreadCount);

    GTEST_LOG_(INFO) << "AppExecFwk_WorkerPool_GetCoreThreadCount_0100 end";
}

/**
 * @tc.number: AppExecFwk_WorkerPool_GetMaxThreadCount_0100
 * @tc.name: GetMaxThreadCount
 * @tc.desc: Test whether the getMaxThreadCount return value equal to 32.
 */
HWTEST(WorkerPoolTest, AppExecFwk_WorkerPool_GetMaxThreadCount_0100, Function | MediumTest | Level1)
{
    GTEST_LOG_(INFO) << "AppExecFwk_WorkerPool_GetMaxThreadCount_0100 start";
    std::shared_ptr<WorkerPool> testWorkerPool = createWorkerPool();
    int maxThreadCount = testWorkerPool->GetMaxThreadCount();
    EXPECT_EQ(32, maxThreadCount);

    GTEST_LOG_(INFO) << "AppExecFwk_WorkerPool_GetMaxThreadCount_0100 end";
}

/**
 * @tc.number: AppExecFwk_WorkerPool_GetWorkCount_0100
 * @tc.name: GetWorkCount
 * @tc.desc: Test whether the getWorkCount return value equal to 1.
 */
HWTEST(WorkerPoolTest, AppExecFwk_WorkerPool_GetWorkCount_0100, Function | MediumTest | Level1)
{
    GTEST_LOG_(INFO) << "AppExecFwk_WorkerPool_GetWorkCount_0100 start";
    Runnable TestTask([]() { GTEST_LOG_(INFO) << "AppExecFwk_WorkerPool_GetWorkCount_0100 task is running"; });
    const std::shared_ptr<WorkerPoolConfig> config = std::make_shared<DefaultWorkerPoolConfig>();
    const std::shared_ptr<TaskExecutor> delegate = std::make_shared<TaskExecutor>(config);
    const std::shared_ptr<Runnable> func = std::make_shared<Runnable>(TestTask);
    const std::shared_ptr<Task> task = std::make_shared<Task>(func, TaskPriority::DEFAULT, nullptr);
    std::shared_ptr<WorkerPool> testWorkerPool = createWorkerPool();
    testWorkerPool->AddWorker(delegate, task);
    sleep(1);
    int workCount = testWorkerPool->GetWorkCount();
    EXPECT_EQ(1, workCount);

    GTEST_LOG_(INFO) << "AppExecFwk_WorkerPool_GetWorkCount_0100 end";
}

/**
 * @tc.number: AppExecFwk_WorkerPool_GetWorkerThreadsInfo_0100
 * @tc.name: GetWorkerThreadsInfo
 * @tc.desc: Test whether the getWorkerThreadsInfo return value equal to 1.
 */
HWTEST(WorkerPoolTest, AppExecFwk_WorkerPool_GetWorkerThreadsInfo_0100, Function | MediumTest | Level1)
{
    GTEST_LOG_(INFO) << "AppExecFwk_WorkerPool_GetWorkerThreadsInfo_0100 start";
    Runnable TestTask([]() { GTEST_LOG_(INFO) << "AppExecFwk_WorkerPool_GetWorkerThreadsInfo_0100 task is running"; });
    const std::shared_ptr<WorkerPoolConfig> config = std::make_shared<DefaultWorkerPoolConfig>();
    const std::shared_ptr<TaskExecutor> delegate = std::make_shared<TaskExecutor>(config);
    const std::shared_ptr<Runnable> func = std::make_shared<Runnable>(TestTask);
    const std::shared_ptr<Task> task = std::make_shared<Task>(func, TaskPriority::DEFAULT, nullptr);
    std::shared_ptr<WorkerPool> testWorkerPool = createWorkerPool();
    testWorkerPool->AddWorker(delegate, task);
    sleep(1);
    std::map<std::string, long> workerThreadsInfoMap = testWorkerPool->GetWorkerThreadsInfo();
    int size = workerThreadsInfoMap.size();
    EXPECT_EQ(1, size);

    GTEST_LOG_(INFO) << "AppExecFwk_WorkerPool_GetWorkerThreadsInfo_0100 end";
}

}  // namespace AppExecFwk
}  // namespace OHOS