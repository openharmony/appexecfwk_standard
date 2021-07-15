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
#include "task_handler_libevent_adapter.h"
#include <gtest/gtest.h>
#include <string>
#include <unistd.h>
#include "runnable.h"
#include "event_runner.h"
#include "default_worker_pool_config.h"

using namespace testing;
using namespace testing::ext;
using namespace OHOS::AppExecFwk;

using OHOS::AppExecFwk::DefaultWorkerPoolConfig;
using OHOS::AppExecFwk::Revocable;
using OHOS::AppExecFwk::Runnable;
using OHOS::AppExecFwk::TaskPriority;
using OHOS::AppExecFwk::WorkerPoolConfig;

class TaskHandlerLibeventAdapterTest : public testing::Test {
public:
    static void SetUpTestCase();
    static void TearDownTestCase();
    void SetUp();
    void TearDown();
};

void TaskHandlerLibeventAdapterTest::SetUpTestCase()
{}

void TaskHandlerLibeventAdapterTest::TearDownTestCase()
{}

void TaskHandlerLibeventAdapterTest::SetUp()
{}

void TaskHandlerLibeventAdapterTest::TearDown()
{}
/**
 * @tc.number:TaskHandlerLibeventAdapter_0100
 * @tc.name: TaskHandlerLibeventAdapter
 * @tc.desc: 1.create TaskHandlerLibeventAdapter
 *           2.Dispatch successfully
 */
HWTEST(TaskHandlerLibeventAdapterTest, TaskHandlerLibeventAdapter_0100, Function | MediumTest | Level1)
{
    GTEST_LOG_(INFO) << "TaskHandlerLibeventAdapter_0100 start";
    std::shared_ptr<EventRunner> runner = EventRunner::Create(true);
    std::shared_ptr<TaskHandlerLibeventAdapter> handler = std::make_shared<TaskHandlerLibeventAdapter>(runner);
    handler->Dispatch(
        std::make_shared<Runnable>([&]() { GTEST_LOG_(INFO) << "TaskHandlerLibeventAdapter_0100 is run"; }));
    GTEST_LOG_(INFO) << "TaskHandlerLibeventAdapter_0100 end";
}
/**
 * @tc.number:Dispatch_0100
 * @tc.name:  Dispatch
 * @tc.desc: 1.create TaskHandlerLibeventAdapter
 *           2.Dispatch successfully
 */
HWTEST(TaskHandlerLibeventAdapterTest, Dispatch_0100, Function | MediumTest | Level1)
{
    GTEST_LOG_(INFO) << "Dispatch_0100 start";
    std::shared_ptr<EventRunner> runner = EventRunner::Create(true);
    std::shared_ptr<TaskHandlerLibeventAdapter> handler = std::make_shared<TaskHandlerLibeventAdapter>(runner);
    handler->Dispatch(std::make_shared<Runnable>([&]() { GTEST_LOG_(INFO) << "Dispatch_0100 is run"; }));
    GTEST_LOG_(INFO) << "Dispatch_0100 end";
}
/**
 * @tc.number:Dispatch_0200
 * @tc.name:  Dispatch
 * @tc.desc: 1.create TaskHandlerLibeventAdapter
 *           2.Dispatch have delay  successfully
 */
HWTEST(TaskHandlerLibeventAdapterTest, Dispatch_0200, Function | MediumTest | Level1)
{
    GTEST_LOG_(INFO) << "Dispatch_0200 start";
    std::shared_ptr<EventRunner> runner = EventRunner::Create(true);
    std::shared_ptr<TaskHandlerLibeventAdapter> handler = std::make_shared<TaskHandlerLibeventAdapter>(runner);
    handler->Dispatch(std::make_shared<Runnable>([&]() { GTEST_LOG_(INFO) << "Dispatch_0200 is run"; }), 300);
    GTEST_LOG_(INFO) << "Dispatch_0200 end";
}