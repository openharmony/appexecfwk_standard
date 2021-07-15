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
#include "spec_dispatcher_config.h"
#include <gtest/gtest.h>
#include <string>
#include <thread>
#include <unistd.h>
#include "runnable.h"
#include "event_runner.h"
#include "default_worker_pool_config.h"

using namespace testing;
using namespace testing::ext;
using namespace OHOS::AppExecFwk;

using OHOS::AppExecFwk::DefaultWorkerPoolConfig;
using OHOS::AppExecFwk::TaskPriority;
using OHOS::AppExecFwk::WorkerPoolConfig;

class SpecDispatcherConfigTest : public testing::Test {
public:
    static void SetUpTestCase();
    static void TearDownTestCase();
    void SetUp();
    void TearDown();
};

void SpecDispatcherConfigTest::SetUpTestCase()
{}

void SpecDispatcherConfigTest::TearDownTestCase()
{}

void SpecDispatcherConfigTest::SetUp()
{}

void SpecDispatcherConfigTest::TearDown()
{}

/**
 * @tc.number:SpecDispatcherConfig_0100
 * @tc.name: SpecDispatcherConfig
 * @tc.desc: 1.create SpecDispatcherConfig
 *           2.GetName successfully
 *  *           3.GetPriority successfully
 */
HWTEST(SpecDispatcherConfigTest, SpecDispatcherConfig_0100, Function | MediumTest | Level1)
{
    GTEST_LOG_(INFO) << "SpecDispatcherConfig_0100 start";
    std::string dispatcherName = "SpecDispatcherConfig_0100";
    TaskPriority taskPriority = TaskPriority::DEFAULT;
    std::shared_ptr<SpecDispatcherConfig> config = std::make_shared<SpecDispatcherConfig>(dispatcherName, taskPriority);
    ASSERT_EQ(config->GetName(), dispatcherName);
    ASSERT_EQ(config->GetPriority(), TaskPriority::DEFAULT);
    GTEST_LOG_(INFO) << "SpecDispatcherConfig_0100 end";
}
/**
 * @tc.number:GetPriority_0100
 * @tc.name: GetPriority
 * @tc.desc: 1.create SpecDispatcherConfig
 *  *           2.GetPriority successfully
 */
HWTEST(SpecDispatcherConfigTest, GetPriority_0100, Function | MediumTest | Level1)
{
    GTEST_LOG_(INFO) << "SpecTaskDispatcher_0100 start";
    std::string dispatcherName = "GetPriority_0100";
    TaskPriority taskPriority = TaskPriority::DEFAULT;
    std::shared_ptr<SpecDispatcherConfig> config = std::make_shared<SpecDispatcherConfig>(dispatcherName, taskPriority);
    ASSERT_EQ(config->GetPriority(), TaskPriority::DEFAULT);
    GTEST_LOG_(INFO) << "GetPriority_0100 end";
}
/**
 * @tc.number:GetName_0100
 * @tc.name: GetName
 * @tc.desc: 1.create SpecDispatcherConfig
 *           2.GetName successfully
 */
HWTEST(SpecDispatcherConfigTest, GetName_0100, Function | MediumTest | Level1)
{
    GTEST_LOG_(INFO) << "GetName_0100 start";
    std::string dispatcherName = "GetName_0100";
    TaskPriority taskPriority = TaskPriority::DEFAULT;
    std::shared_ptr<SpecDispatcherConfig> config = std::make_shared<SpecDispatcherConfig>(dispatcherName, taskPriority);
    ASSERT_EQ(config->GetName(), dispatcherName);
    GTEST_LOG_(INFO) << "GetName_0100 end";
}
