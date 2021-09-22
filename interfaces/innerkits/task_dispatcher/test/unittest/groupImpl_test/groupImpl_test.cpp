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
#include <thread>
#include "group_impl.h"

namespace OHOS {
namespace AppExecFwk {
using namespace testing::ext;
using namespace OHOS::AppExecFwk;

class GroupImplTest : public testing::Test {
public:
    static void SetUpTestCase(void);
    static void TearDownTestCase(void);
    void SetUp();
    void TearDown();
};

void GroupImplTest::SetUpTestCase(void)
{}

void GroupImplTest::TearDownTestCase(void)
{}

void GroupImplTest::SetUp(void)
{}

void GroupImplTest::TearDown(void)
{}

/**
 * @tc.number: AppExecFwk_GroupImpl_ConstructorTest_001
 * @tc.name: GroupImplConstructor
 * @tc.desc: Test GroupImpl Constructor.
 */
HWTEST(GroupImplTest, AppExecFwk_GroupImpl_ConstructorTest_001, TestSize.Level0)
{
    auto name = std::string("AppExecFwk_GroupImpl_ConstructorTest_001");
    GTEST_LOG_(INFO) << name << " start";
    std::shared_ptr<GroupImpl> groupPtr = std::make_shared<GroupImpl>();
    EXPECT_TRUE((groupPtr != nullptr));
    GTEST_LOG_(INFO) << name << " end";
}

/**
 * @tc.number: AppExecFwk_GroupImpl_AwaitAllTasks_001
 * @tc.name: AwaitAllTasks
 * @tc.desc: Test GroupImpl await all tasks done
 */
HWTEST(GroupImplTest, AppExecFwk_GroupImpl_AwaitAllTasks_001, TestSize.Level0)
{
    auto name = std::string("AppExecFwk_GroupImpl_AwaitAllTasks_001");
    GTEST_LOG_(INFO) << name << " start";
    std::shared_ptr<GroupImpl> groupPtr = std::make_shared<GroupImpl>();
    // add task
    groupPtr->Associate();
    groupPtr->Associate();
    // add notification
    std::shared_ptr<Runnable> notification1 =
        std::make_shared<Runnable>([]() { GTEST_LOG_(INFO) << "GroupImpl notification1"; });
    std::shared_ptr<Runnable> notification2 =
        std::make_shared<Runnable>([]() { GTEST_LOG_(INFO) << "GroupImpl notification2"; });
    groupPtr->AddNotification(notification1);
    groupPtr->AddNotification(notification2);
    // excute task
    std::thread excuteTask1([&]() { groupPtr->NotifyTaskDone(); });
    std::thread excuteTask2([&]() { groupPtr->NotifyTaskDone(); });
    excuteTask1.detach();
    excuteTask2.detach();
    groupPtr->AwaitAllTasks(1000);
    std::this_thread::sleep_for(std::chrono::milliseconds(3000));
    GTEST_LOG_(INFO) << name << " end";
}

/**
 * @tc.number: AppExecFwk_Task_Run_002
 * @tc.name: AwaitAllTasks
 * @tc.desc: Test GroupImpl await all tasks done
 */
HWTEST(GroupImplTest, AppExecFwk_GroupImpl_AwaitAllTasks_002, TestSize.Level0)
{
    auto name = std::string("AppExecFwk_Task_Run_002");
    GTEST_LOG_(INFO) << name << " start";
    std::shared_ptr<GroupImpl> groupPtr = std::make_shared<GroupImpl>();
    // add task
    groupPtr->Associate();
    // add notification
    std::shared_ptr<Runnable> notification =
        std::make_shared<Runnable>([]() { GTEST_LOG_(INFO) << "GroupImpl notification1"; });
    groupPtr->AddNotification(notification);
    bool result = groupPtr->AwaitAllTasks(1000);
    EXPECT_FALSE(result);
    GTEST_LOG_(INFO) << name << " end";
}

/**
 * @tc.number: AppExecFwk_GroupImpl_Associate_001
 * @tc.name: Associate
 * @tc.desc: Test GroupImpl add task
 */
HWTEST(GroupImplTest, AppExecFwk_GroupImpl_Associate_001, TestSize.Level0)
{
    auto name = std::string("AppExecFwk_GroupImpl_Associate_001");
    GTEST_LOG_(INFO) << name << " start";
    std::shared_ptr<GroupImpl> groupPtr = std::make_shared<GroupImpl>();
    // add task
    groupPtr->Associate();
    bool result = groupPtr->AwaitAllTasks(1000);
    EXPECT_FALSE(result);
    GTEST_LOG_(INFO) << name << " end";
}

/**
 * @tc.number: AppExecFwk_GroupImpl_NotifyTaskDone_001
 * @tc.name: NotifyTaskDone
 * @tc.desc: Test GroupImpl notify task done
 */
HWTEST(GroupImplTest, AppExecFwk_GroupImpl_NotifyTaskDone_001, TestSize.Level0)
{
    auto name = std::string("AppExecFwk_GroupImpl_NotifyTaskDone_001");
    GTEST_LOG_(INFO) << name << " start";
    std::shared_ptr<GroupImpl> groupPtr = std::make_shared<GroupImpl>();
    groupPtr->Associate();
    groupPtr->NotifyTaskDone();
    bool result = groupPtr->AwaitAllTasks(1000);
    EXPECT_TRUE(result);
    GTEST_LOG_(INFO) << name << " end";
}

/**
 * @tc.number: AppExecFwk_GroupImpl_AddNotification_001
 * @tc.name: AddNotification
 * @tc.desc: Test GroupImpl add a notification
 */
HWTEST(GroupImplTest, AppExecFwk_GroupImpl_AddNotification_001, TestSize.Level0)
{
    auto name = std::string("AppExecFwk_GroupImpl_AddNotification_001");
    GTEST_LOG_(INFO) << name << " start";
    std::shared_ptr<GroupImpl> groupPtr = std::make_shared<GroupImpl>();
    std::shared_ptr<Runnable> notification =
        std::make_shared<Runnable>([]() { GTEST_LOG_(INFO) << "GroupImpl AddNotification"; });
    bool result = groupPtr->AddNotification(notification);
    EXPECT_TRUE(result);
    GTEST_LOG_(INFO) << name << " end";
}

/**
 * @tc.number: AppExecFwk_GroupImpl_AddNotification_002
 * @tc.name: AddNotification
 * @tc.desc: Test GroupImpl max number of notifications
 */
HWTEST(GroupImplTest, AppExecFwk_GroupImpl_AddNotification_002, TestSize.Level0)
{
    auto name = std::string("AppExecFwk_GroupImpl_AddNotification_002");
    GTEST_LOG_(INFO) << name << " start";
    std::shared_ptr<GroupImpl> groupPtr = std::make_shared<GroupImpl>();
    for (int i = 0; i < 1000; i++) {
        std::shared_ptr<Runnable> notification =
            std::make_shared<Runnable>([&]() { GTEST_LOG_(INFO) << "GroupImpl AddNotification"; });
        groupPtr->AddNotification(notification);
    }
    std::shared_ptr<Runnable> notificationTask =
        std::make_shared<Runnable>([&]() { GTEST_LOG_(INFO) << "GroupImpl AddNotification"; });
    bool result = groupPtr->AddNotification(notificationTask);
    EXPECT_TRUE(result);
    GTEST_LOG_(INFO) << name << " end";
}

/**
 * @tc.number: AppExecFwk_GroupImpl_AddNotification_003
 * @tc.name: AddNotification
 * @tc.desc: Test GroupImpl max number of notifications
 */
HWTEST(GroupImplTest, AppExecFwk_GroupImpl_AddNotification_003, TestSize.Level0)
{
    auto name = std::string("AppExecFwk_GroupImpl_AddNotification_003");
    GTEST_LOG_(INFO) << name << " start";
    std::shared_ptr<GroupImpl> groupPtr = std::make_shared<GroupImpl>();
    groupPtr->Associate();
    for (int i = 0; i < 1000; i++) {
        std::shared_ptr<Runnable> notification =
            std::make_shared<Runnable>([&]() { GTEST_LOG_(INFO) << "GroupImpl AddNotification"; });
        groupPtr->AddNotification(notification);
    }
    std::shared_ptr<Runnable> notificationTask =
        std::make_shared<Runnable>([&]() { GTEST_LOG_(INFO) << "GroupImpl AddNotification"; });
    bool result = groupPtr->AddNotification(notificationTask);
    EXPECT_FALSE(result);
    GTEST_LOG_(INFO) << name << " end";
}

}  // namespace AppExecFwk
}  // namespace OHOS