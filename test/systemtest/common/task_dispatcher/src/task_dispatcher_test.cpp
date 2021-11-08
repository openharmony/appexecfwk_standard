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
#include "ability_lifecycle_executor.h"
#include "common_event_manager.h"
#include "st_ability_util.h"
#include "task_dispatcher_test_info.h"
#include "testConfigParser.h"

namespace {
using namespace OHOS;
using namespace OHOS::AppExecFwk;
using namespace OHOS::AAFwk;
using namespace OHOS::EventFwk;
using namespace OHOS::STtools;
using namespace testing::ext;
using namespace STABUtil;

using MAP_STR_STR = std::map<std::string, std::string>;
const std::string bundleNameFirst = "com.ohos.TaskDispatcherA";
const std::string bundleNameSecond = "com.ohos.TaskDispatcherB";
const std::string firstAbilityName = "MainAbility";
const std::string secondAbilityname = "SecondAbility";
constexpr int WAIT_TIME = 500;
constexpr int DELAY = 10;
int testCaseCode = 100;
std::vector<std::string> bundleNameList = {
    "com.ohos.TaskDispatcherA",
    "com.ohos.TaskDispatcherB",
};
std::vector<std::string> hapNameList = {
    "taskDispatcherTestA",
    "taskDispatcherTestB",
};

class TaskDispatcherTest : public testing::Test {
public:
    static void SetUpTestCase(void);
    static void TearDownTestCase(void);
    void SetUp();
    void TearDown();

    static bool SubscribeEvent();
    class AppEventSubscriber : public OHOS::EventFwk::CommonEventSubscriber {
    public:
        explicit AppEventSubscriber(const OHOS::EventFwk::CommonEventSubscribeInfo &sp) : CommonEventSubscriber(sp) {};
        virtual void OnReceiveEvent(const OHOS::EventFwk::CommonEventData &data) override;
        ~AppEventSubscriber(){};
    };

    void StartAbility(const std::string &abilityName, const std::string &bundleName);
    static OHOS::sptr<OHOS::AAFwk::IAbilityManager> abilityMs;
    static OHOS::STtools::Event event;
    static OHOS::StressTestLevel stLevel_;
    static std::shared_ptr<AppEventSubscriber> subscriber_;
};

Event TaskDispatcherTest::event = STtools::Event();
sptr<IAbilityManager> TaskDispatcherTest::abilityMs = nullptr;
StressTestLevel TaskDispatcherTest::stLevel_ {};
std::shared_ptr<TaskDispatcherTest::AppEventSubscriber> TaskDispatcherTest::subscriber_ = nullptr;

void TaskDispatcherTest::SetUpTestCase(void)
{
    if (!SubscribeEvent()) {
        GTEST_LOG_(INFO) << "SubscribeEvent error";
    }
    TestConfigParser tcp;
    tcp.ParseFromFile4StressTest(STRESS_TEST_CONFIG_FILE_PATH, stLevel_);
    std::cout << "stress test level : "
              << "AMS : " << stLevel_.AMSLevel << " "
              << "BMS : " << stLevel_.BMSLevel << " "
              << "CES : " << stLevel_.CESLevel << std::endl;
    stLevel_.AMSLevel = 1;
}
void TaskDispatcherTest::TearDownTestCase(void)
{
    CommonEventManager::UnSubscribeCommonEvent(subscriber_);
}

void TaskDispatcherTest::SetUp()
{
    STAbilityUtil::InstallHaps(hapNameList);
}

void TaskDispatcherTest::TearDown()
{
    STAbilityUtil::UninstallBundle(bundleNameList);
    STAbilityUtil::CleanMsg(event);
}

void TaskDispatcherTest::AppEventSubscriber::OnReceiveEvent(const CommonEventData &data)
{
    STAbilityUtil::Completed(event, data.GetWant().GetAction(), data.GetCode(), data.GetData());
}

bool TaskDispatcherTest::SubscribeEvent()
{
    std::vector<std::string> eventList = {
        g_EVENT_RESP_FIRST,
        g_EVENT_RESP_SECOND,
        g_EVENT_RESP_FIRST_B,
        g_EVENT_RESP_SECOND_B,
    };
    MatchingSkills matchingSkills;
    for (const auto &e : eventList) {
        matchingSkills.AddEvent(e);
    }
    CommonEventSubscribeInfo subscribeInfo(matchingSkills);
    subscribeInfo.SetPriority(1);
    auto subscriber = std::make_shared<AppEventSubscriber>(subscribeInfo);
    return CommonEventManager::SubscribeCommonEvent(subscriber);
}

void TaskDispatcherTest::StartAbility(const std::string &abilityName, const std::string &bundleName)
{
    MAP_STR_STR params;
    Want want = STAbilityUtil::MakeWant("device", abilityName, bundleName, params);
    ErrCode result = STAbilityUtil::StartAbility(want, abilityMs, WAIT_TIME);
    GTEST_LOG_(INFO) << "TaskDispatcherTest::StartAbility : " << result;
    if (bundleName == bundleNameSecond) {
        EXPECT_EQ(0, STAbilityUtil::WaitCompleted(event, g_EVENT_RESP_FIRST_B, 0, DELAY));
    } else {
        EXPECT_EQ(0, STAbilityUtil::WaitCompleted(event, g_EVENT_RESP_FIRST, 0, DELAY));
    }
}

/**
 * @tc.number    : TaskDispatcher_Global_00100
 * @tc.name      : global sync:parallel sync
 * @tc.desc      : two levels task dispatch
 */
HWTEST_F(TaskDispatcherTest, TaskDispatcher_Global_00100, Function | MediumTest | Level1)
{
    StartAbility(firstAbilityName, bundleNameFirst);
    std::string data = "Dispatcher_" + std::to_string((int)TestFunc::GLOBAL) + "_0";
    bool result = false;
    for (int i = 0; i < stLevel_.AMSLevel; i++) {
        STAbilityUtil::PublishEvent(g_EVENT_REQU_FIRST, ++testCaseCode, data);
        EXPECT_EQ(0, STAbilityUtil::WaitCompleted(event, g_EVENT_RESP_FIRST, testCaseCode, DELAY));
        data = STAbilityUtil::GetData(event, g_EVENT_RESP_FIRST, testCaseCode);
        result = data.compare("1") == 0;
        EXPECT_TRUE(result);
        if (!result && i > 0) {
            GTEST_LOG_(INFO) << "TaskDispatcher_Global_00100 : " << i;
            break;
        }
    }
}

/**
 * @tc.number    : TaskDispatcher_Global_00200
 * @tc.name      : global sync:parallel async
 * @tc.desc      : two levels task dispatch
 */
HWTEST_F(TaskDispatcherTest, TaskDispatcher_Global_00200, Function | MediumTest | Level1)
{
    StartAbility(firstAbilityName, bundleNameFirst);
    std::string data = "Dispatcher_" + std::to_string((int)TestFunc::GLOBAL) + "_1";
    bool result = false;
    for (int i = 0; i < stLevel_.AMSLevel; i++) {
        STAbilityUtil::PublishEvent(g_EVENT_REQU_FIRST, ++testCaseCode, data);
        EXPECT_EQ(0, STAbilityUtil::WaitCompleted(event, g_EVENT_RESP_FIRST, testCaseCode, DELAY));
        data = STAbilityUtil::GetData(event, g_EVENT_RESP_FIRST, testCaseCode);
        result = data.compare("1") == 0;
        EXPECT_TRUE(result);
        if (!result && i > 0) {
            GTEST_LOG_(INFO) << "TaskDispatcher_Global_00200 : " << i;
            break;
        }
    }
}

/**
 * @tc.number    : TaskDispatcher_Global_00300
 * @tc.name      : global sync:parallel delay
 * @tc.desc      : two levels task dispatch
 */
HWTEST_F(TaskDispatcherTest, TaskDispatcher_Global_00300, Function | MediumTest | Level1)
{
    StartAbility(firstAbilityName, bundleNameFirst);
    std::string data = "Dispatcher_" + std::to_string((int)TestFunc::GLOBAL) + "_2";
    bool result = false;
    for (int i = 0; i < stLevel_.AMSLevel; i++) {
        STAbilityUtil::PublishEvent(g_EVENT_REQU_FIRST, ++testCaseCode, data);
        EXPECT_EQ(0, STAbilityUtil::WaitCompleted(event, g_EVENT_RESP_FIRST, testCaseCode, DELAY));
        data = STAbilityUtil::GetData(event, g_EVENT_RESP_FIRST, testCaseCode);
        result = data.compare("1") == 0;
        EXPECT_TRUE(result);
        if (!result && i > 0) {
            GTEST_LOG_(INFO) << "TaskDispatcher_Global_00300 : " << i;
            break;
        }
    }
}

/**
 * @tc.number    : TaskDispatcher_Global_00400
 * @tc.name      : global sync:parallel group
 * @tc.desc      : two levels task dispatch
 */
HWTEST_F(TaskDispatcherTest, TaskDispatcher_Global_00400, Function | MediumTest | Level1)
{
    StartAbility(firstAbilityName, bundleNameFirst);
    std::string data = "Dispatcher_" + std::to_string((int)TestFunc::GLOBAL) + "_3";
    bool result = false;
    for (int i = 0; i < stLevel_.AMSLevel; i++) {
        STAbilityUtil::PublishEvent(g_EVENT_REQU_FIRST, ++testCaseCode, data);
        EXPECT_EQ(0, STAbilityUtil::WaitCompleted(event, g_EVENT_RESP_FIRST, testCaseCode, DELAY));
        data = STAbilityUtil::GetData(event, g_EVENT_RESP_FIRST, testCaseCode);
        result = data.compare("1") == 0;
        EXPECT_TRUE(result);
        if (!result && i > 0) {
            GTEST_LOG_(INFO) << "TaskDispatcher_Global_00400 : " << i;
            break;
        }
    }
}

/**
 * @tc.number    : TaskDispatcher_Global_00500
 * @tc.name      : global sync:parallel group wait
 * @tc.desc      : two levels task dispatch
 */
HWTEST_F(TaskDispatcherTest, TaskDispatcher_Global_00500, Function | MediumTest | Level1)
{
    StartAbility(firstAbilityName, bundleNameFirst);
    std::string data = "Dispatcher_" + std::to_string((int)TestFunc::GLOBAL) + "_4";
    bool result = false;
    for (int i = 0; i < stLevel_.AMSLevel; i++) {
        STAbilityUtil::PublishEvent(g_EVENT_REQU_FIRST, ++testCaseCode, data);
        EXPECT_EQ(0, STAbilityUtil::WaitCompleted(event, g_EVENT_RESP_FIRST, testCaseCode, DELAY));
        data = STAbilityUtil::GetData(event, g_EVENT_RESP_FIRST, testCaseCode);
        result = data.compare("1") == 0;
        EXPECT_TRUE(result);
        if (!result && i > 0) {
            GTEST_LOG_(INFO) << "TaskDispatcher_Global_00500 : " << i;
            break;
        }
    }
}

/**
 * @tc.number    : TaskDispatcher_Global_00600
 * @tc.name      : global sync:parallel group notify
 * @tc.desc      : two levels task dispatch
 */
HWTEST_F(TaskDispatcherTest, TaskDispatcher_Global_00600, Function | MediumTest | Level1)
{
    StartAbility(firstAbilityName, bundleNameFirst);
    std::string data = "Dispatcher_" + std::to_string((int)TestFunc::GLOBAL) + "_5";
    bool result = false;
    for (int i = 0; i < stLevel_.AMSLevel; i++) {
        STAbilityUtil::PublishEvent(g_EVENT_REQU_FIRST, ++testCaseCode, data);
        EXPECT_EQ(0, STAbilityUtil::WaitCompleted(event, g_EVENT_RESP_FIRST, testCaseCode, DELAY));
        data = STAbilityUtil::GetData(event, g_EVENT_RESP_FIRST, testCaseCode);
        result = data.compare("1") == 0;
        EXPECT_TRUE(result);
        if (!result && i > 0) {
            GTEST_LOG_(INFO) << "TaskDispatcher_Global_00600 : " << i;
            break;
        }
    }
}

/**
 * @tc.number    : TaskDispatcher_Global_00700
 * @tc.name      : global sync:parallel sync barrier
 * @tc.desc      : two levels task dispatch
 */
HWTEST_F(TaskDispatcherTest, TaskDispatcher_Global_00700, Function | MediumTest | Level1)
{
    StartAbility(firstAbilityName, bundleNameFirst);
    std::string data = "Dispatcher_" + std::to_string((int)TestFunc::GLOBAL) + "_6";
    bool result = false;
    for (int i = 0; i < stLevel_.AMSLevel; i++) {
        STAbilityUtil::PublishEvent(g_EVENT_REQU_FIRST, ++testCaseCode, data);
        EXPECT_EQ(0, STAbilityUtil::WaitCompleted(event, g_EVENT_RESP_FIRST, testCaseCode, DELAY));
        data = STAbilityUtil::GetData(event, g_EVENT_RESP_FIRST, testCaseCode);
        result = data.compare("1") == 0;
        EXPECT_TRUE(result);
        if (!result && i > 0) {
            GTEST_LOG_(INFO) << "TaskDispatcher_Global_00700 : " << i;
            break;
        }
    }
}

/**
 * @tc.number    : TaskDispatcher_Global_00800
 * @tc.name      : global sync:parallel async barrier
 * @tc.desc      : two levels task dispatch
 */
HWTEST_F(TaskDispatcherTest, TaskDispatcher_Global_00800, Function | MediumTest | Level1)
{
    StartAbility(firstAbilityName, bundleNameFirst);
    std::string data = "Dispatcher_" + std::to_string((int)TestFunc::GLOBAL) + "_7";
    bool result = false;
    for (int i = 0; i < stLevel_.AMSLevel; i++) {
        STAbilityUtil::PublishEvent(g_EVENT_REQU_FIRST, ++testCaseCode, data);
        EXPECT_EQ(0, STAbilityUtil::WaitCompleted(event, g_EVENT_RESP_FIRST, testCaseCode, DELAY));
        data = STAbilityUtil::GetData(event, g_EVENT_RESP_FIRST, testCaseCode);
        result = data.compare("1") == 0;
        EXPECT_TRUE(result);
        if (!result && i > 0) {
            GTEST_LOG_(INFO) << "TaskDispatcher_Global_00800 : " << i;
            break;
        }
    }
}

/**
 * @tc.number    : TaskDispatcher_Global_00900
 * @tc.name      : global sync:parallel apply
 * @tc.desc      : two levels task dispatch
 */
HWTEST_F(TaskDispatcherTest, TaskDispatcher_Global_00900, Function | MediumTest | Level1)
{
    StartAbility(firstAbilityName, bundleNameFirst);
    std::string data = "Dispatcher_" + std::to_string((int)TestFunc::GLOBAL) + "_8";
    bool result = false;
    for (int i = 0; i < stLevel_.AMSLevel; i++) {
        STAbilityUtil::PublishEvent(g_EVENT_REQU_FIRST, ++testCaseCode, data);
        EXPECT_EQ(0, STAbilityUtil::WaitCompleted(event, g_EVENT_RESP_FIRST, testCaseCode, DELAY));
        data = STAbilityUtil::GetData(event, g_EVENT_RESP_FIRST, testCaseCode);
        result = data.compare("1") == 0;
        EXPECT_TRUE(result);
        if (!result && i > 0) {
            GTEST_LOG_(INFO) << "TaskDispatcher_Global_00900 : " << i;
            break;
        }
    }
}

/**
 * @tc.number    : TaskDispatcher_Global_01000
 * @tc.name      : global sync:serial sync
 * @tc.desc      : two levels task dispatch
 */
HWTEST_F(TaskDispatcherTest, TaskDispatcher_Global_01000, Function | MediumTest | Level1)
{
    StartAbility(firstAbilityName, bundleNameFirst);
    std::string data = "Dispatcher_" + std::to_string((int)TestFunc::GLOBAL) + "_9";
    bool result = false;
    for (int i = 0; i < stLevel_.AMSLevel; i++) {
        STAbilityUtil::PublishEvent(g_EVENT_REQU_FIRST, ++testCaseCode, data);
        EXPECT_EQ(0, STAbilityUtil::WaitCompleted(event, g_EVENT_RESP_FIRST, testCaseCode, DELAY));
        data = STAbilityUtil::GetData(event, g_EVENT_RESP_FIRST, testCaseCode);
        result = data.compare("1") == 0;
        EXPECT_TRUE(result);
        if (!result && i > 0) {
            GTEST_LOG_(INFO) << "TaskDispatcher_Global_01000 : " << i;
            break;
        }
    }
}

/**
 * @tc.number    : TaskDispatcher_Global_01100
 * @tc.name      : global sync:serial async
 * @tc.desc      : two levels task dispatch
 */
HWTEST_F(TaskDispatcherTest, TaskDispatcher_Global_01100, Function | MediumTest | Level1)
{
    StartAbility(firstAbilityName, bundleNameFirst);
    std::string data = "Dispatcher_" + std::to_string((int)TestFunc::GLOBAL) + "_10";
    bool result = false;
    for (int i = 0; i < stLevel_.AMSLevel; i++) {
        STAbilityUtil::PublishEvent(g_EVENT_REQU_FIRST, ++testCaseCode, data);
        EXPECT_EQ(0, STAbilityUtil::WaitCompleted(event, g_EVENT_RESP_FIRST, testCaseCode, DELAY));
        data = STAbilityUtil::GetData(event, g_EVENT_RESP_FIRST, testCaseCode);
        result = data.compare("1") == 0;
        EXPECT_TRUE(result);
        if (!result && i > 0) {
            GTEST_LOG_(INFO) << "TaskDispatcher_Global_01100 : " << i;
            break;
        }
    }
}

/**
 * @tc.number    : TaskDispatcher_Global_01200
 * @tc.name      : global sync:serial delay
 * @tc.desc      : two levels task dispatch
 */
HWTEST_F(TaskDispatcherTest, TaskDispatcher_Global_01200, Function | MediumTest | Level1)
{
    StartAbility(firstAbilityName, bundleNameFirst);
    std::string data = "Dispatcher_" + std::to_string((int)TestFunc::GLOBAL) + "_11";
    bool result = false;
    for (int i = 0; i < stLevel_.AMSLevel; i++) {
        STAbilityUtil::PublishEvent(g_EVENT_REQU_FIRST, ++testCaseCode, data);
        EXPECT_EQ(0, STAbilityUtil::WaitCompleted(event, g_EVENT_RESP_FIRST, testCaseCode, DELAY));
        data = STAbilityUtil::GetData(event, g_EVENT_RESP_FIRST, testCaseCode);
        result = data.compare("1") == 0;
        EXPECT_TRUE(result);
        if (!result && i > 0) {
            GTEST_LOG_(INFO) << "TaskDispatcher_Global_01200 : " << i;
            break;
        }
    }
}

/**
 * @tc.number    : TaskDispatcher_Global_01300
 * @tc.name      : global sync:serial apply
 * @tc.desc      : two levels task dispatch
 */
HWTEST_F(TaskDispatcherTest, TaskDispatcher_Global_01300, Function | MediumTest | Level1)
{
    StartAbility(firstAbilityName, bundleNameFirst);
    std::string data = "Dispatcher_" + std::to_string((int)TestFunc::GLOBAL) + "_12";
    bool result = false;
    for (int i = 0; i < stLevel_.AMSLevel; i++) {
        STAbilityUtil::PublishEvent(g_EVENT_REQU_FIRST, ++testCaseCode, data);
        EXPECT_EQ(0, STAbilityUtil::WaitCompleted(event, g_EVENT_RESP_FIRST, testCaseCode, DELAY));
        data = STAbilityUtil::GetData(event, g_EVENT_RESP_FIRST, testCaseCode);
        result = data.compare("1") == 0;
        EXPECT_TRUE(result);
        if (!result && i > 0) {
            GTEST_LOG_(INFO) << "TaskDispatcher_Global_01300 : " << i;
            break;
        }
    }
}

/**
 * @tc.number    : TaskDispatcher_Global_01400
 * @tc.name      : global async:parallel sync
 * @tc.desc      : two levels task dispatch
 */
HWTEST_F(TaskDispatcherTest, TaskDispatcher_Global_01400, Function | MediumTest | Level1)
{
    StartAbility(firstAbilityName, bundleNameFirst);
    std::string data = "Dispatcher_" + std::to_string((int)TestFunc::GLOBAL) + "_13";
    bool result = false;
    for (int i = 0; i < stLevel_.AMSLevel; i++) {
        STAbilityUtil::PublishEvent(g_EVENT_REQU_FIRST, ++testCaseCode, data);
        EXPECT_EQ(0, STAbilityUtil::WaitCompleted(event, g_EVENT_RESP_FIRST, testCaseCode, DELAY));
        data = STAbilityUtil::GetData(event, g_EVENT_RESP_FIRST, testCaseCode);
        result = data.compare("1") == 0;
        EXPECT_TRUE(result);
        if (!result && i > 0) {
            GTEST_LOG_(INFO) << "TaskDispatcher_Global_01400 : " << i;
            break;
        }
    }
}

/**
 * @tc.number    : TaskDispatcher_Global_01500
 * @tc.name      : global async:parallel async
 * @tc.desc      : two levels task dispatch
 */
HWTEST_F(TaskDispatcherTest, TaskDispatcher_Global_01500, Function | MediumTest | Level1)
{
    StartAbility(firstAbilityName, bundleNameFirst);
    std::string data = "Dispatcher_" + std::to_string((int)TestFunc::GLOBAL) + "_14";
    bool result = false;
    for (int i = 0; i < stLevel_.AMSLevel; i++) {
        STAbilityUtil::PublishEvent(g_EVENT_REQU_FIRST, ++testCaseCode, data);
        EXPECT_EQ(0, STAbilityUtil::WaitCompleted(event, g_EVENT_RESP_FIRST, testCaseCode, DELAY));
        data = STAbilityUtil::GetData(event, g_EVENT_RESP_FIRST, testCaseCode);
        result = data.compare("1") == 0;
        EXPECT_TRUE(result);
        if (!result && i > 0) {
            GTEST_LOG_(INFO) << "TaskDispatcher_Global_01500 : " << i;
            break;
        }
    }
}

/**
 * @tc.number    : TaskDispatcher_Global_01600
 * @tc.name      : global async:parallel delay
 * @tc.desc      : two levels task dispatch
 */
HWTEST_F(TaskDispatcherTest, TaskDispatcher_Global_01600, Function | MediumTest | Level1)
{
    StartAbility(firstAbilityName, bundleNameFirst);
    std::string data = "Dispatcher_" + std::to_string((int)TestFunc::GLOBAL) + "_15";
    bool result = false;
    for (int i = 0; i < stLevel_.AMSLevel; i++) {
        STAbilityUtil::PublishEvent(g_EVENT_REQU_FIRST, ++testCaseCode, data);
        EXPECT_EQ(0, STAbilityUtil::WaitCompleted(event, g_EVENT_RESP_FIRST, testCaseCode, DELAY));
        data = STAbilityUtil::GetData(event, g_EVENT_RESP_FIRST, testCaseCode);
        result = data.compare("1") == 0;
        EXPECT_TRUE(result);
        if (!result && i > 0) {
            GTEST_LOG_(INFO) << "TaskDispatcher_Global_01600 : " << i;
            break;
        }
    }
}

/**
 * @tc.number    : TaskDispatcher_Global_01700
 * @tc.name      : global async:parallel group
 * @tc.desc      : two levels task dispatch
 */
HWTEST_F(TaskDispatcherTest, TaskDispatcher_Global_01700, Function | MediumTest | Level1)
{
    StartAbility(firstAbilityName, bundleNameFirst);
    std::string data = "Dispatcher_" + std::to_string((int)TestFunc::GLOBAL) + "_16";
    bool result = false;
    for (int i = 0; i < stLevel_.AMSLevel; i++) {
        STAbilityUtil::PublishEvent(g_EVENT_REQU_FIRST, ++testCaseCode, data);
        EXPECT_EQ(0, STAbilityUtil::WaitCompleted(event, g_EVENT_RESP_FIRST, testCaseCode, DELAY));
        data = STAbilityUtil::GetData(event, g_EVENT_RESP_FIRST, testCaseCode);
        result = data.compare("1") == 0;
        EXPECT_TRUE(result);
        if (!result && i > 0) {
            GTEST_LOG_(INFO) << "TaskDispatcher_Global_01700 : " << i;
            break;
        }
    }
}

/**
 * @tc.number    : TaskDispatcher_Global_01800
 * @tc.name      : global async:parallel group wait
 * @tc.desc      : two levels task dispatch
 */
HWTEST_F(TaskDispatcherTest, TaskDispatcher_Global_01800, Function | MediumTest | Level1)
{
    StartAbility(firstAbilityName, bundleNameFirst);
    std::string data = "Dispatcher_" + std::to_string((int)TestFunc::GLOBAL) + "_17";
    bool result = false;
    for (int i = 0; i < stLevel_.AMSLevel; i++) {
        STAbilityUtil::PublishEvent(g_EVENT_REQU_FIRST, ++testCaseCode, data);
        EXPECT_EQ(0, STAbilityUtil::WaitCompleted(event, g_EVENT_RESP_FIRST, testCaseCode, DELAY));
        data = STAbilityUtil::GetData(event, g_EVENT_RESP_FIRST, testCaseCode);
        result = data.compare("1") == 0;
        EXPECT_TRUE(result);
        if (!result && i > 0) {
            GTEST_LOG_(INFO) << "TaskDispatcher_Global_01800 : " << i;
            break;
        }
    }
}

/**
 * @tc.number    : TaskDispatcher_Global_01900
 * @tc.name      : global async:parallel group notify
 * @tc.desc      : two levels task dispatch
 */
HWTEST_F(TaskDispatcherTest, TaskDispatcher_Global_01900, Function | MediumTest | Level1)
{
    StartAbility(firstAbilityName, bundleNameFirst);
    std::string data = "Dispatcher_" + std::to_string((int)TestFunc::GLOBAL) + "_18";
    bool result = false;
    for (int i = 0; i < stLevel_.AMSLevel; i++) {
        STAbilityUtil::PublishEvent(g_EVENT_REQU_FIRST, ++testCaseCode, data);
        EXPECT_EQ(0, STAbilityUtil::WaitCompleted(event, g_EVENT_RESP_FIRST, testCaseCode, DELAY));
        data = STAbilityUtil::GetData(event, g_EVENT_RESP_FIRST, testCaseCode);
        result = data.compare("1") == 0;
        EXPECT_TRUE(result);
        if (!result && i > 0) {
            GTEST_LOG_(INFO) << "TaskDispatcher_Global_01900 : " << i;
            break;
        }
    }
}

/**
 * @tc.number    : TaskDispatcher_Global_02000
 * @tc.name      : global async:parallel sync barrier
 * @tc.desc      : two levels task dispatch
 */
HWTEST_F(TaskDispatcherTest, TaskDispatcher_Global_02000, Function | MediumTest | Level1)
{
    StartAbility(firstAbilityName, bundleNameFirst);
    std::string data = "Dispatcher_" + std::to_string((int)TestFunc::GLOBAL) + "_19";
    bool result = false;
    for (int i = 0; i < stLevel_.AMSLevel; i++) {
        STAbilityUtil::PublishEvent(g_EVENT_REQU_FIRST, ++testCaseCode, data);
        EXPECT_EQ(0, STAbilityUtil::WaitCompleted(event, g_EVENT_RESP_FIRST, testCaseCode, DELAY));
        data = STAbilityUtil::GetData(event, g_EVENT_RESP_FIRST, testCaseCode);
        result = data.compare("1") == 0;
        EXPECT_TRUE(result);
        if (!result && i > 0) {
            GTEST_LOG_(INFO) << "TaskDispatcher_Global_02000 : " << i;
            break;
        }
    }
}

/**
 * @tc.number    : TaskDispatcher_Global_02100
 * @tc.name      : global async:parallel async barrier
 * @tc.desc      : two levels task dispatch
 */
HWTEST_F(TaskDispatcherTest, TaskDispatcher_Global_02100, Function | MediumTest | Level1)
{
    StartAbility(firstAbilityName, bundleNameFirst);
    std::string data = "Dispatcher_" + std::to_string((int)TestFunc::GLOBAL) + "_20";
    bool result = false;
    for (int i = 0; i < stLevel_.AMSLevel; i++) {
        STAbilityUtil::PublishEvent(g_EVENT_REQU_FIRST, ++testCaseCode, data);
        EXPECT_EQ(0, STAbilityUtil::WaitCompleted(event, g_EVENT_RESP_FIRST, testCaseCode, DELAY));
        data = STAbilityUtil::GetData(event, g_EVENT_RESP_FIRST, testCaseCode);
        result = data.compare("1") == 0;
        EXPECT_TRUE(result);
        if (!result && i > 0) {
            GTEST_LOG_(INFO) << "TaskDispatcher_Global_02100 : " << i;
            break;
        }
    }
}

/**
 * @tc.number    : TaskDispatcher_Global_02200
 * @tc.name      : global async:parallel apply
 * @tc.desc      : two levels task dispatch
 */
HWTEST_F(TaskDispatcherTest, TaskDispatcher_Global_02200, Function | MediumTest | Level1)
{
    StartAbility(firstAbilityName, bundleNameFirst);
    std::string data = "Dispatcher_" + std::to_string((int)TestFunc::GLOBAL) + "_21";
    bool result = false;
    for (int i = 0; i < stLevel_.AMSLevel; i++) {
        STAbilityUtil::PublishEvent(g_EVENT_REQU_FIRST, ++testCaseCode, data);
        EXPECT_EQ(0, STAbilityUtil::WaitCompleted(event, g_EVENT_RESP_FIRST, testCaseCode, DELAY));
        data = STAbilityUtil::GetData(event, g_EVENT_RESP_FIRST, testCaseCode);
        result = data.compare("1") == 0;
        EXPECT_TRUE(result);
        if (!result && i > 0) {
            GTEST_LOG_(INFO) << "TaskDispatcher_Global_02200 : " << i;
            break;
        }
    }
}

/**
 * @tc.number    : TaskDispatcher_Global_02300
 * @tc.name      : global async:serial sync
 * @tc.desc      : two levels task dispatch
 */
HWTEST_F(TaskDispatcherTest, TaskDispatcher_Global_02300, Function | MediumTest | Level1)
{
    StartAbility(firstAbilityName, bundleNameFirst);
    std::string data = "Dispatcher_" + std::to_string((int)TestFunc::GLOBAL) + "_22";
    bool result = false;
    for (int i = 0; i < stLevel_.AMSLevel; i++) {
        STAbilityUtil::PublishEvent(g_EVENT_REQU_FIRST, ++testCaseCode, data);
        EXPECT_EQ(0, STAbilityUtil::WaitCompleted(event, g_EVENT_RESP_FIRST, testCaseCode, DELAY));
        data = STAbilityUtil::GetData(event, g_EVENT_RESP_FIRST, testCaseCode);
        result = data.compare("1") == 0;
        EXPECT_TRUE(result);
        if (!result && i > 0) {
            GTEST_LOG_(INFO) << "TaskDispatcher_Global_02300 : " << i;
            break;
        }
    }
}

/**
 * @tc.number    : TaskDispatcher_Global_02400
 * @tc.name      : global async:serial async
 * @tc.desc      : two levels task dispatch
 */
HWTEST_F(TaskDispatcherTest, TaskDispatcher_Global_02400, Function | MediumTest | Level1)
{
    StartAbility(firstAbilityName, bundleNameFirst);
    std::string data = "Dispatcher_" + std::to_string((int)TestFunc::GLOBAL) + "_23";
    bool result = false;
    for (int i = 0; i < stLevel_.AMSLevel; i++) {
        STAbilityUtil::PublishEvent(g_EVENT_REQU_FIRST, ++testCaseCode, data);
        EXPECT_EQ(0, STAbilityUtil::WaitCompleted(event, g_EVENT_RESP_FIRST, testCaseCode, DELAY));
        data = STAbilityUtil::GetData(event, g_EVENT_RESP_FIRST, testCaseCode);
        result = data.compare("1") == 0;
        EXPECT_TRUE(result);
        if (!result && i > 0) {
            GTEST_LOG_(INFO) << "TaskDispatcher_Global_02400 : " << i;
            break;
        }
    }
}

/**
 * @tc.number    : TaskDispatcher_Global_02500
 * @tc.name      : global async:serial delay
 * @tc.desc      : two levels task dispatch
 */
HWTEST_F(TaskDispatcherTest, TaskDispatcher_Global_02500, Function | MediumTest | Level1)
{
    StartAbility(firstAbilityName, bundleNameFirst);
    std::string data = "Dispatcher_" + std::to_string((int)TestFunc::GLOBAL) + "_24";
    bool result = false;
    for (int i = 0; i < stLevel_.AMSLevel; i++) {
        STAbilityUtil::PublishEvent(g_EVENT_REQU_FIRST, ++testCaseCode, data);
        EXPECT_EQ(0, STAbilityUtil::WaitCompleted(event, g_EVENT_RESP_FIRST, testCaseCode, DELAY));
        data = STAbilityUtil::GetData(event, g_EVENT_RESP_FIRST, testCaseCode);
        result = data.compare("1") == 0;
        EXPECT_TRUE(result);
        if (!result && i > 0) {
            GTEST_LOG_(INFO) << "TaskDispatcher_Global_02500 : " << i;
            break;
        }
    }
}

/**
 * @tc.number    : TaskDispatcher_Global_02600
 * @tc.name      : global async:serial apply
 * @tc.desc      : two levels task dispatch
 */
HWTEST_F(TaskDispatcherTest, TaskDispatcher_Global_02600, Function | MediumTest | Level1)
{
    StartAbility(firstAbilityName, bundleNameFirst);
    std::string data = "Dispatcher_" + std::to_string((int)TestFunc::GLOBAL) + "_25";
    bool result = false;
    for (int i = 0; i < stLevel_.AMSLevel; i++) {
        STAbilityUtil::PublishEvent(g_EVENT_REQU_FIRST, ++testCaseCode, data);
        EXPECT_EQ(0, STAbilityUtil::WaitCompleted(event, g_EVENT_RESP_FIRST, testCaseCode, DELAY));
        data = STAbilityUtil::GetData(event, g_EVENT_RESP_FIRST, testCaseCode);
        result = data.compare("1") == 0;
        EXPECT_TRUE(result);
        if (!result && i > 0) {
            GTEST_LOG_(INFO) << "TaskDispatcher_Global_02600 : " << i;
            break;
        }
    }
}

/**
 * @tc.number    : TaskDispatcher_Global_02700
 * @tc.name      : global delay:parallel sync
 * @tc.desc      : two levels task dispatch
 */
HWTEST_F(TaskDispatcherTest, TaskDispatcher_Global_02700, Function | MediumTest | Level1)
{
    StartAbility(firstAbilityName, bundleNameFirst);
    std::string data = "Dispatcher_" + std::to_string((int)TestFunc::GLOBAL) + "_26";
    bool result = false;
    for (int i = 0; i < stLevel_.AMSLevel; i++) {
        STAbilityUtil::PublishEvent(g_EVENT_REQU_FIRST, ++testCaseCode, data);
        EXPECT_EQ(0, STAbilityUtil::WaitCompleted(event, g_EVENT_RESP_FIRST, testCaseCode, DELAY));
        data = STAbilityUtil::GetData(event, g_EVENT_RESP_FIRST, testCaseCode);
        result = data.compare("1") == 0;
        EXPECT_TRUE(result);
        if (!result && i > 0) {
            GTEST_LOG_(INFO) << "TaskDispatcher_Global_02700 : " << i;
            break;
        }
    }
}

/**
 * @tc.number    : TaskDispatcher_Global_02800
 * @tc.name      : global delay:parallel async
 * @tc.desc      : two levels task dispatch
 */
HWTEST_F(TaskDispatcherTest, TaskDispatcher_Global_02800, Function | MediumTest | Level1)
{
    StartAbility(firstAbilityName, bundleNameFirst);
    std::string data = "Dispatcher_" + std::to_string((int)TestFunc::GLOBAL) + "_27";
    bool result = false;
    for (int i = 0; i < stLevel_.AMSLevel; i++) {
        STAbilityUtil::PublishEvent(g_EVENT_REQU_FIRST, ++testCaseCode, data);
        EXPECT_EQ(0, STAbilityUtil::WaitCompleted(event, g_EVENT_RESP_FIRST, testCaseCode, DELAY));
        data = STAbilityUtil::GetData(event, g_EVENT_RESP_FIRST, testCaseCode);
        result = data.compare("1") == 0;
        EXPECT_TRUE(result);
        if (!result && i > 0) {
            GTEST_LOG_(INFO) << "TaskDispatcher_Global_02800 : " << i;
            break;
        }
    }
}

/**
 * @tc.number    : TaskDispatcher_Global_02900
 * @tc.name      : global delay:parallel delay
 * @tc.desc      : two levels task dispatch
 */
HWTEST_F(TaskDispatcherTest, TaskDispatcher_Global_02900, Function | MediumTest | Level1)
{
    StartAbility(firstAbilityName, bundleNameFirst);
    std::string data = "Dispatcher_" + std::to_string((int)TestFunc::GLOBAL) + "_28";
    bool result = false;
    for (int i = 0; i < stLevel_.AMSLevel; i++) {
        STAbilityUtil::PublishEvent(g_EVENT_REQU_FIRST, ++testCaseCode, data);
        EXPECT_EQ(0, STAbilityUtil::WaitCompleted(event, g_EVENT_RESP_FIRST, testCaseCode, DELAY));
        data = STAbilityUtil::GetData(event, g_EVENT_RESP_FIRST, testCaseCode);
        result = data.compare("1") == 0;
        EXPECT_TRUE(result);
        if (!result && i > 0) {
            GTEST_LOG_(INFO) << "TaskDispatcher_Global_02900 : " << i;
            break;
        }
    }
}

/**
 * @tc.number    : TaskDispatcher_Global_03000
 * @tc.name      : global delay:parallel group
 * @tc.desc      : two levels task dispatch
 */
HWTEST_F(TaskDispatcherTest, TaskDispatcher_Global_03000, Function | MediumTest | Level1)
{
    StartAbility(firstAbilityName, bundleNameFirst);
    std::string data = "Dispatcher_" + std::to_string((int)TestFunc::GLOBAL) + "_29";
    bool result = false;
    for (int i = 0; i < stLevel_.AMSLevel; i++) {
        STAbilityUtil::PublishEvent(g_EVENT_REQU_FIRST, ++testCaseCode, data);
        EXPECT_EQ(0, STAbilityUtil::WaitCompleted(event, g_EVENT_RESP_FIRST, testCaseCode, DELAY));
        data = STAbilityUtil::GetData(event, g_EVENT_RESP_FIRST, testCaseCode);
        result = data.compare("1") == 0;
        EXPECT_TRUE(result);
        if (!result && i > 0) {
            GTEST_LOG_(INFO) << "TaskDispatcher_Global_03000 : " << i;
            break;
        }
    }
}

/**
 * @tc.number    : TaskDispatcher_Global_03100
 * @tc.name      : global delay:parallel group wait
 * @tc.desc      : two levels task dispatch
 */
HWTEST_F(TaskDispatcherTest, TaskDispatcher_Global_03100, Function | MediumTest | Level1)
{
    StartAbility(firstAbilityName, bundleNameFirst);
    std::string data = "Dispatcher_" + std::to_string((int)TestFunc::GLOBAL) + "_30";
    bool result = false;
    for (int i = 0; i < stLevel_.AMSLevel; i++) {
        STAbilityUtil::PublishEvent(g_EVENT_REQU_FIRST, ++testCaseCode, data);
        EXPECT_EQ(0, STAbilityUtil::WaitCompleted(event, g_EVENT_RESP_FIRST, testCaseCode, DELAY));
        data = STAbilityUtil::GetData(event, g_EVENT_RESP_FIRST, testCaseCode);
        result = data.compare("1") == 0;
        EXPECT_TRUE(result);
        if (!result && i > 0) {
            GTEST_LOG_(INFO) << "TaskDispatcher_Global_03100 : " << i;
            break;
        }
    }
}

/**
 * @tc.number    : TaskDispatcher_Global_03200
 * @tc.name      : global delay:parallel group notify
 * @tc.desc      : two levels task dispatch
 */
HWTEST_F(TaskDispatcherTest, TaskDispatcher_Global_03200, Function | MediumTest | Level1)
{
    StartAbility(firstAbilityName, bundleNameFirst);
    std::string data = "Dispatcher_" + std::to_string((int)TestFunc::GLOBAL) + "_31";
    bool result = false;
    for (int i = 0; i < stLevel_.AMSLevel; i++) {
        STAbilityUtil::PublishEvent(g_EVENT_REQU_FIRST, ++testCaseCode, data);
        EXPECT_EQ(0, STAbilityUtil::WaitCompleted(event, g_EVENT_RESP_FIRST, testCaseCode, DELAY));
        data = STAbilityUtil::GetData(event, g_EVENT_RESP_FIRST, testCaseCode);
        result = data.compare("1") == 0;
        EXPECT_TRUE(result);
        if (!result && i > 0) {
            GTEST_LOG_(INFO) << "TaskDispatcher_Global_03200 : " << i;
            break;
        }
    }
}

/**
 * @tc.number    : TaskDispatcher_Global_03300
 * @tc.name      : global delay:parallel sync barrier
 * @tc.desc      : two levels task dispatch
 */
HWTEST_F(TaskDispatcherTest, TaskDispatcher_Global_03300, Function | MediumTest | Level1)
{
    StartAbility(firstAbilityName, bundleNameFirst);
    std::string data = "Dispatcher_" + std::to_string((int)TestFunc::GLOBAL) + "_32";
    bool result = false;
    for (int i = 0; i < stLevel_.AMSLevel; i++) {
        STAbilityUtil::PublishEvent(g_EVENT_REQU_FIRST, ++testCaseCode, data);
        EXPECT_EQ(0, STAbilityUtil::WaitCompleted(event, g_EVENT_RESP_FIRST, testCaseCode, DELAY));
        data = STAbilityUtil::GetData(event, g_EVENT_RESP_FIRST, testCaseCode);
        result = data.compare("1") == 0;
        EXPECT_TRUE(result);
        if (!result && i > 0) {
            GTEST_LOG_(INFO) << "TaskDispatcher_Global_03300 : " << i;
            break;
        }
    }
}

/**
 * @tc.number    : TaskDispatcher_Global_03400
 * @tc.name      : global delay:parallel async barrier
 * @tc.desc      : two levels task dispatch
 */
HWTEST_F(TaskDispatcherTest, TaskDispatcher_Global_03400, Function | MediumTest | Level1)
{
    StartAbility(firstAbilityName, bundleNameFirst);
    std::string data = "Dispatcher_" + std::to_string((int)TestFunc::GLOBAL) + "_33";
    bool result = false;
    for (int i = 0; i < stLevel_.AMSLevel; i++) {
        STAbilityUtil::PublishEvent(g_EVENT_REQU_FIRST, ++testCaseCode, data);
        EXPECT_EQ(0, STAbilityUtil::WaitCompleted(event, g_EVENT_RESP_FIRST, testCaseCode, DELAY));
        data = STAbilityUtil::GetData(event, g_EVENT_RESP_FIRST, testCaseCode);
        result = data.compare("1") == 0;
        EXPECT_TRUE(result);
        if (!result && i > 0) {
            GTEST_LOG_(INFO) << "TaskDispatcher_Global_03400 : " << i;
            break;
        }
    }
}

/**
 * @tc.number    : TaskDispatcher_Global_03500
 * @tc.name      : global delay:parallel apply
 * @tc.desc      : two levels task dispatch
 */
HWTEST_F(TaskDispatcherTest, TaskDispatcher_Global_03500, Function | MediumTest | Level1)
{
    StartAbility(firstAbilityName, bundleNameFirst);
    std::string data = "Dispatcher_" + std::to_string((int)TestFunc::GLOBAL) + "_34";
    bool result = false;
    for (int i = 0; i < stLevel_.AMSLevel; i++) {
        STAbilityUtil::PublishEvent(g_EVENT_REQU_FIRST, ++testCaseCode, data);
        EXPECT_EQ(0, STAbilityUtil::WaitCompleted(event, g_EVENT_RESP_FIRST, testCaseCode, DELAY));
        data = STAbilityUtil::GetData(event, g_EVENT_RESP_FIRST, testCaseCode);
        result = data.compare("1") == 0;
        EXPECT_TRUE(result);
        if (!result && i > 0) {
            GTEST_LOG_(INFO) << "TaskDispatcher_Global_03500 : " << i;
            break;
        }
    }
}

/**
 * @tc.number    : TaskDispatcher_Global_03600
 * @tc.name      : global delay:serial sync
 * @tc.desc      : two levels task dispatch
 */
HWTEST_F(TaskDispatcherTest, TaskDispatcher_Global_03600, Function | MediumTest | Level1)
{
    StartAbility(firstAbilityName, bundleNameFirst);
    std::string data = "Dispatcher_" + std::to_string((int)TestFunc::GLOBAL) + "_35";
    bool result = false;
    for (int i = 0; i < stLevel_.AMSLevel; i++) {
        STAbilityUtil::PublishEvent(g_EVENT_REQU_FIRST, ++testCaseCode, data);
        EXPECT_EQ(0, STAbilityUtil::WaitCompleted(event, g_EVENT_RESP_FIRST, testCaseCode, DELAY));
        data = STAbilityUtil::GetData(event, g_EVENT_RESP_FIRST, testCaseCode);
        result = data.compare("1") == 0;
        EXPECT_TRUE(result);
        if (!result && i > 0) {
            GTEST_LOG_(INFO) << "TaskDispatcher_Global_03600 : " << i;
            break;
        }
    }
}

/**
 * @tc.number    : TaskDispatcher_Global_03700
 * @tc.name      : global delay:serial async
 * @tc.desc      : two levels task dispatch
 */
HWTEST_F(TaskDispatcherTest, TaskDispatcher_Global_03700, Function | MediumTest | Level1)
{
    StartAbility(firstAbilityName, bundleNameFirst);
    std::string data = "Dispatcher_" + std::to_string((int)TestFunc::GLOBAL) + "_36";
    bool result = false;
    for (int i = 0; i < stLevel_.AMSLevel; i++) {
        STAbilityUtil::PublishEvent(g_EVENT_REQU_FIRST, ++testCaseCode, data);
        EXPECT_EQ(0, STAbilityUtil::WaitCompleted(event, g_EVENT_RESP_FIRST, testCaseCode, DELAY));
        data = STAbilityUtil::GetData(event, g_EVENT_RESP_FIRST, testCaseCode);
        result = data.compare("1") == 0;
        EXPECT_TRUE(result);
        if (!result && i > 0) {
            GTEST_LOG_(INFO) << "TaskDispatcher_Global_03700 : " << i;
            break;
        }
    }
}

/**
 * @tc.number    : TaskDispatcher_Global_03800
 * @tc.name      : global delay:serial delay
 * @tc.desc      : two levels task dispatch
 */
HWTEST_F(TaskDispatcherTest, TaskDispatcher_Global_03800, Function | MediumTest | Level1)
{
    StartAbility(firstAbilityName, bundleNameFirst);
    std::string data = "Dispatcher_" + std::to_string((int)TestFunc::GLOBAL) + "_37";
    bool result = false;
    for (int i = 0; i < stLevel_.AMSLevel; i++) {
        STAbilityUtil::PublishEvent(g_EVENT_REQU_FIRST, ++testCaseCode, data);
        EXPECT_EQ(0, STAbilityUtil::WaitCompleted(event, g_EVENT_RESP_FIRST, testCaseCode, DELAY));
        data = STAbilityUtil::GetData(event, g_EVENT_RESP_FIRST, testCaseCode);
        result = data.compare("1") == 0;
        EXPECT_TRUE(result);
        if (!result && i > 0) {
            GTEST_LOG_(INFO) << "TaskDispatcher_Global_03800 : " << i;
            break;
        }
    }
}

/**
 * @tc.number    : TaskDispatcher_Global_03900
 * @tc.name      : global delay:serial apply
 * @tc.desc      : two levels task dispatch
 */
HWTEST_F(TaskDispatcherTest, TaskDispatcher_Global_03900, Function | MediumTest | Level1)
{
    StartAbility(firstAbilityName, bundleNameFirst);
    std::string data = "Dispatcher_" + std::to_string((int)TestFunc::GLOBAL) + "_38";
    bool result = false;
    for (int i = 0; i < stLevel_.AMSLevel; i++) {
        STAbilityUtil::PublishEvent(g_EVENT_REQU_FIRST, ++testCaseCode, data);
        EXPECT_EQ(0, STAbilityUtil::WaitCompleted(event, g_EVENT_RESP_FIRST, testCaseCode, DELAY));
        data = STAbilityUtil::GetData(event, g_EVENT_RESP_FIRST, testCaseCode);
        result = data.compare("1") == 0;
        EXPECT_TRUE(result);
        if (!result && i > 0) {
            GTEST_LOG_(INFO) << "TaskDispatcher_Global_03900 : " << i;
            break;
        }
    }
}

/**
 * @tc.number    : TaskDispatcher_Global_04000
 * @tc.name      : global group:parallel sync
 * @tc.desc      : two levels task dispatch
 */
HWTEST_F(TaskDispatcherTest, TaskDispatcher_Global_04000, Function | MediumTest | Level1)
{
    StartAbility(firstAbilityName, bundleNameFirst);
    std::string data = "Dispatcher_" + std::to_string((int)TestFunc::GLOBAL) + "_39";
    bool result = false;
    for (int i = 0; i < stLevel_.AMSLevel; i++) {
        STAbilityUtil::PublishEvent(g_EVENT_REQU_FIRST, ++testCaseCode, data);
        EXPECT_EQ(0, STAbilityUtil::WaitCompleted(event, g_EVENT_RESP_FIRST, testCaseCode, DELAY));
        data = STAbilityUtil::GetData(event, g_EVENT_RESP_FIRST, testCaseCode);
        result = data.compare("1") == 0;
        EXPECT_TRUE(result);
        if (!result && i > 0) {
            GTEST_LOG_(INFO) << "TaskDispatcher_Global_04000 : " << i;
            break;
        }
    }
}

/**
 * @tc.number    : TaskDispatcher_Global_04100
 * @tc.name      : global group:parallel async
 * @tc.desc      : two levels task dispatch
 */
HWTEST_F(TaskDispatcherTest, TaskDispatcher_Global_04100, Function | MediumTest | Level1)
{
    StartAbility(firstAbilityName, bundleNameFirst);
    std::string data = "Dispatcher_" + std::to_string((int)TestFunc::GLOBAL) + "_40";
    bool result = false;
    for (int i = 0; i < stLevel_.AMSLevel; i++) {
        STAbilityUtil::PublishEvent(g_EVENT_REQU_FIRST, ++testCaseCode, data);
        EXPECT_EQ(0, STAbilityUtil::WaitCompleted(event, g_EVENT_RESP_FIRST, testCaseCode, DELAY));
        data = STAbilityUtil::GetData(event, g_EVENT_RESP_FIRST, testCaseCode);
        result = data.compare("1") == 0;
        EXPECT_TRUE(result);
        if (!result && i > 0) {
            GTEST_LOG_(INFO) << "TaskDispatcher_Global_04100 : " << i;
            break;
        }
    }
}

/**
 * @tc.number    : TaskDispatcher_Global_04200
 * @tc.name      : global group:parallel delay
 * @tc.desc      : two levels task dispatch
 */
HWTEST_F(TaskDispatcherTest, TaskDispatcher_Global_04200, Function | MediumTest | Level1)
{
    StartAbility(firstAbilityName, bundleNameFirst);
    std::string data = "Dispatcher_" + std::to_string((int)TestFunc::GLOBAL) + "_41";
    bool result = false;
    for (int i = 0; i < stLevel_.AMSLevel; i++) {
        STAbilityUtil::PublishEvent(g_EVENT_REQU_FIRST, ++testCaseCode, data);
        EXPECT_EQ(0, STAbilityUtil::WaitCompleted(event, g_EVENT_RESP_FIRST, testCaseCode, DELAY));
        data = STAbilityUtil::GetData(event, g_EVENT_RESP_FIRST, testCaseCode);
        result = data.compare("1") == 0;
        EXPECT_TRUE(result);
        if (!result && i > 0) {
            GTEST_LOG_(INFO) << "TaskDispatcher_Global_04200 : " << i;
            break;
        }
    }
}

/**
 * @tc.number    : TaskDispatcher_Global_04300
 * @tc.name      : global group:parallel group
 * @tc.desc      : two levels task dispatch
 */
HWTEST_F(TaskDispatcherTest, TaskDispatcher_Global_04300, Function | MediumTest | Level1)
{
    StartAbility(firstAbilityName, bundleNameFirst);
    std::string data = "Dispatcher_" + std::to_string((int)TestFunc::GLOBAL) + "_42";
    bool result = false;
    for (int i = 0; i < stLevel_.AMSLevel; i++) {
        STAbilityUtil::PublishEvent(g_EVENT_REQU_FIRST, ++testCaseCode, data);
        EXPECT_EQ(0, STAbilityUtil::WaitCompleted(event, g_EVENT_RESP_FIRST, testCaseCode, DELAY));
        data = STAbilityUtil::GetData(event, g_EVENT_RESP_FIRST, testCaseCode);
        result = data.compare("1") == 0;
        EXPECT_TRUE(result);
        if (!result && i > 0) {
            GTEST_LOG_(INFO) << "TaskDispatcher_Global_04300 : " << i;
            break;
        }
    }
}

/**
 * @tc.number    : TaskDispatcher_Global_04400
 * @tc.name      : global group:parallel group wait
 * @tc.desc      : two levels task dispatch
 */
HWTEST_F(TaskDispatcherTest, TaskDispatcher_Global_04400, Function | MediumTest | Level1)
{
    StartAbility(firstAbilityName, bundleNameFirst);
    std::string data = "Dispatcher_" + std::to_string((int)TestFunc::GLOBAL) + "_43";
    bool result = false;
    for (int i = 0; i < stLevel_.AMSLevel; i++) {
        STAbilityUtil::PublishEvent(g_EVENT_REQU_FIRST, ++testCaseCode, data);
        EXPECT_EQ(0, STAbilityUtil::WaitCompleted(event, g_EVENT_RESP_FIRST, testCaseCode, DELAY));
        data = STAbilityUtil::GetData(event, g_EVENT_RESP_FIRST, testCaseCode);
        result = data.compare("1") == 0;
        EXPECT_TRUE(result);
        if (!result && i > 0) {
            GTEST_LOG_(INFO) << "TaskDispatcher_Global_04400 : " << i;
            break;
        }
    }
}

/**
 * @tc.number    : TaskDispatcher_Global_04500
 * @tc.name      : global group:parallel group notify
 * @tc.desc      : two levels task dispatch
 */
HWTEST_F(TaskDispatcherTest, TaskDispatcher_Global_04500, Function | MediumTest | Level1)
{
    StartAbility(firstAbilityName, bundleNameFirst);
    std::string data = "Dispatcher_" + std::to_string((int)TestFunc::GLOBAL) + "_44";
    bool result = false;
    for (int i = 0; i < stLevel_.AMSLevel; i++) {
        STAbilityUtil::PublishEvent(g_EVENT_REQU_FIRST, ++testCaseCode, data);
        EXPECT_EQ(0, STAbilityUtil::WaitCompleted(event, g_EVENT_RESP_FIRST, testCaseCode, DELAY));
        data = STAbilityUtil::GetData(event, g_EVENT_RESP_FIRST, testCaseCode);
        result = data.compare("1") == 0;
        EXPECT_TRUE(result);
        if (!result && i > 0) {
            GTEST_LOG_(INFO) << "TaskDispatcher_Global_04500 : " << i;
            break;
        }
    }
}

/**
 * @tc.number    : TaskDispatcher_Global_04600
 * @tc.name      : global group:parallel sync barrier
 * @tc.desc      : two levels task dispatch
 */
HWTEST_F(TaskDispatcherTest, TaskDispatcher_Global_04600, Function | MediumTest | Level1)
{
    StartAbility(firstAbilityName, bundleNameFirst);
    std::string data = "Dispatcher_" + std::to_string((int)TestFunc::GLOBAL) + "_45";
    bool result = false;
    for (int i = 0; i < stLevel_.AMSLevel; i++) {
        STAbilityUtil::PublishEvent(g_EVENT_REQU_FIRST, ++testCaseCode, data);
        EXPECT_EQ(0, STAbilityUtil::WaitCompleted(event, g_EVENT_RESP_FIRST, testCaseCode, DELAY));
        data = STAbilityUtil::GetData(event, g_EVENT_RESP_FIRST, testCaseCode);
        result = data.compare("1") == 0;
        EXPECT_TRUE(result);
        if (!result && i > 0) {
            GTEST_LOG_(INFO) << "TaskDispatcher_Global_04600 : " << i;
            break;
        }
    }
}

/**
 * @tc.number    : TaskDispatcher_Global_04700
 * @tc.name      : global group:parallel async barrier
 * @tc.desc      : two levels task dispatch
 */
HWTEST_F(TaskDispatcherTest, TaskDispatcher_Global_04700, Function | MediumTest | Level1)
{
    StartAbility(firstAbilityName, bundleNameFirst);
    std::string data = "Dispatcher_" + std::to_string((int)TestFunc::GLOBAL) + "_46";
    bool result = false;
    for (int i = 0; i < stLevel_.AMSLevel; i++) {
        STAbilityUtil::PublishEvent(g_EVENT_REQU_FIRST, ++testCaseCode, data);
        EXPECT_EQ(0, STAbilityUtil::WaitCompleted(event, g_EVENT_RESP_FIRST, testCaseCode, DELAY));
        data = STAbilityUtil::GetData(event, g_EVENT_RESP_FIRST, testCaseCode);
        result = data.compare("1") == 0;
        EXPECT_TRUE(result);
        if (!result && i > 0) {
            GTEST_LOG_(INFO) << "TaskDispatcher_Global_04700 : " << i;
            break;
        }
    }
}

/**
 * @tc.number    : TaskDispatcher_Global_04800
 * @tc.name      : global group:parallel apply
 * @tc.desc      : two levels task dispatch
 */
HWTEST_F(TaskDispatcherTest, TaskDispatcher_Global_04800, Function | MediumTest | Level1)
{
    StartAbility(firstAbilityName, bundleNameFirst);
    std::string data = "Dispatcher_" + std::to_string((int)TestFunc::GLOBAL) + "_47";
    bool result = false;
    for (int i = 0; i < stLevel_.AMSLevel; i++) {
        STAbilityUtil::PublishEvent(g_EVENT_REQU_FIRST, ++testCaseCode, data);
        EXPECT_EQ(0, STAbilityUtil::WaitCompleted(event, g_EVENT_RESP_FIRST, testCaseCode, DELAY));
        data = STAbilityUtil::GetData(event, g_EVENT_RESP_FIRST, testCaseCode);
        result = data.compare("1") == 0;
        EXPECT_TRUE(result);
        if (!result && i > 0) {
            GTEST_LOG_(INFO) << "TaskDispatcher_Global_04800 : " << i;
            break;
        }
    }
}

/**
 * @tc.number    : TaskDispatcher_Global_04900
 * @tc.name      : global group:serial sync
 * @tc.desc      : two levels task dispatch
 */
HWTEST_F(TaskDispatcherTest, TaskDispatcher_Global_04900, Function | MediumTest | Level1)
{
    StartAbility(firstAbilityName, bundleNameFirst);
    std::string data = "Dispatcher_" + std::to_string((int)TestFunc::GLOBAL) + "_48";
    bool result = false;
    for (int i = 0; i < stLevel_.AMSLevel; i++) {
        STAbilityUtil::PublishEvent(g_EVENT_REQU_FIRST, ++testCaseCode, data);
        EXPECT_EQ(0, STAbilityUtil::WaitCompleted(event, g_EVENT_RESP_FIRST, testCaseCode, DELAY));
        data = STAbilityUtil::GetData(event, g_EVENT_RESP_FIRST, testCaseCode);
        result = data.compare("1") == 0;
        EXPECT_TRUE(result);
        if (!result && i > 0) {
            GTEST_LOG_(INFO) << "TaskDispatcher_Global_04900 : " << i;
            break;
        }
    }
}

/**
 * @tc.number    : TaskDispatcher_Global_05000
 * @tc.name      : global group:serial async
 * @tc.desc      : two levels task dispatch
 */
HWTEST_F(TaskDispatcherTest, TaskDispatcher_Global_05000, Function | MediumTest | Level1)
{
    StartAbility(firstAbilityName, bundleNameFirst);
    std::string data = "Dispatcher_" + std::to_string((int)TestFunc::GLOBAL) + "_49";
    bool result = false;
    for (int i = 0; i < stLevel_.AMSLevel; i++) {
        STAbilityUtil::PublishEvent(g_EVENT_REQU_FIRST, ++testCaseCode, data);
        EXPECT_EQ(0, STAbilityUtil::WaitCompleted(event, g_EVENT_RESP_FIRST, testCaseCode, DELAY));
        data = STAbilityUtil::GetData(event, g_EVENT_RESP_FIRST, testCaseCode);
        result = data.compare("1") == 0;
        EXPECT_TRUE(result);
        if (!result && i > 0) {
            GTEST_LOG_(INFO) << "TaskDispatcher_Global_05000 : " << i;
            break;
        }
    }
}

/**
 * @tc.number    : TaskDispatcher_Global_05100
 * @tc.name      : global group:serial delay
 * @tc.desc      : two levels task dispatch
 */
HWTEST_F(TaskDispatcherTest, TaskDispatcher_Global_05100, Function | MediumTest | Level1)
{
    StartAbility(firstAbilityName, bundleNameFirst);
    std::string data = "Dispatcher_" + std::to_string((int)TestFunc::GLOBAL) + "_50";
    bool result = false;
    for (int i = 0; i < stLevel_.AMSLevel; i++) {
        STAbilityUtil::PublishEvent(g_EVENT_REQU_FIRST, ++testCaseCode, data);
        EXPECT_EQ(0, STAbilityUtil::WaitCompleted(event, g_EVENT_RESP_FIRST, testCaseCode, DELAY));
        data = STAbilityUtil::GetData(event, g_EVENT_RESP_FIRST, testCaseCode);
        result = data.compare("1") == 0;
        EXPECT_TRUE(result);
        if (!result && i > 0) {
            GTEST_LOG_(INFO) << "TaskDispatcher_Global_05100 : " << i;
            break;
        }
    }
}

/**
 * @tc.number    : TaskDispatcher_Global_05200
 * @tc.name      : global group:serial apply
 * @tc.desc      : two levels task dispatch
 */
HWTEST_F(TaskDispatcherTest, TaskDispatcher_Global_05200, Function | MediumTest | Level1)
{
    StartAbility(firstAbilityName, bundleNameFirst);
    std::string data = "Dispatcher_" + std::to_string((int)TestFunc::GLOBAL) + "_51";
    bool result = false;
    for (int i = 0; i < stLevel_.AMSLevel; i++) {
        STAbilityUtil::PublishEvent(g_EVENT_REQU_FIRST, ++testCaseCode, data);
        EXPECT_EQ(0, STAbilityUtil::WaitCompleted(event, g_EVENT_RESP_FIRST, testCaseCode, DELAY));
        data = STAbilityUtil::GetData(event, g_EVENT_RESP_FIRST, testCaseCode);
        result = data.compare("1") == 0;
        EXPECT_TRUE(result);
        if (!result && i > 0) {
            GTEST_LOG_(INFO) << "TaskDispatcher_Global_05200 : " << i;
            break;
        }
    }
}

/**
 * @tc.number    : TaskDispatcher_Global_05300
 * @tc.name      : global group wait:parallel sync
 * @tc.desc      : two levels task dispatch
 */
HWTEST_F(TaskDispatcherTest, TaskDispatcher_Global_05300, Function | MediumTest | Level1)
{
    StartAbility(firstAbilityName, bundleNameFirst);
    std::string data = "Dispatcher_" + std::to_string((int)TestFunc::GLOBAL) + "_52";
    bool result = false;
    for (int i = 0; i < stLevel_.AMSLevel; i++) {
        STAbilityUtil::PublishEvent(g_EVENT_REQU_FIRST, ++testCaseCode, data);
        EXPECT_EQ(0, STAbilityUtil::WaitCompleted(event, g_EVENT_RESP_FIRST, testCaseCode, DELAY));
        data = STAbilityUtil::GetData(event, g_EVENT_RESP_FIRST, testCaseCode);
        result = data.compare("1") == 0;
        EXPECT_TRUE(result);
        if (!result && i > 0) {
            GTEST_LOG_(INFO) << "TaskDispatcher_Global_05300 : " << i;
            break;
        }
    }
}

/**
 * @tc.number    : TaskDispatcher_Global_05400
 * @tc.name      : global group wait:parallel async
 * @tc.desc      : two levels task dispatch
 */
HWTEST_F(TaskDispatcherTest, TaskDispatcher_Global_05400, Function | MediumTest | Level1)
{
    StartAbility(firstAbilityName, bundleNameFirst);
    std::string data = "Dispatcher_" + std::to_string((int)TestFunc::GLOBAL) + "_53";
    bool result = false;
    for (int i = 0; i < stLevel_.AMSLevel; i++) {
        STAbilityUtil::PublishEvent(g_EVENT_REQU_FIRST, ++testCaseCode, data);
        EXPECT_EQ(0, STAbilityUtil::WaitCompleted(event, g_EVENT_RESP_FIRST, testCaseCode, DELAY));
        data = STAbilityUtil::GetData(event, g_EVENT_RESP_FIRST, testCaseCode);
        result = data.compare("1") == 0;
        EXPECT_TRUE(result);
        if (!result && i > 0) {
            GTEST_LOG_(INFO) << "TaskDispatcher_Global_05400 : " << i;
            break;
        }
    }
}

/**
 * @tc.number    : TaskDispatcher_Global_05500
 * @tc.name      : global group wait:parallel delay
 * @tc.desc      : two levels task dispatch
 */
HWTEST_F(TaskDispatcherTest, TaskDispatcher_Global_05500, Function | MediumTest | Level1)
{
    StartAbility(firstAbilityName, bundleNameFirst);
    std::string data = "Dispatcher_" + std::to_string((int)TestFunc::GLOBAL) + "_54";
    bool result = false;
    for (int i = 0; i < stLevel_.AMSLevel; i++) {
        STAbilityUtil::PublishEvent(g_EVENT_REQU_FIRST, ++testCaseCode, data);
        EXPECT_EQ(0, STAbilityUtil::WaitCompleted(event, g_EVENT_RESP_FIRST, testCaseCode, DELAY));
        data = STAbilityUtil::GetData(event, g_EVENT_RESP_FIRST, testCaseCode);
        result = data.compare("1") == 0;
        EXPECT_TRUE(result);
        if (!result && i > 0) {
            GTEST_LOG_(INFO) << "TaskDispatcher_Global_05500 : " << i;
            break;
        }
    }
}

/**
 * @tc.number    : TaskDispatcher_Global_05600
 * @tc.name      : global group wait:parallel group
 * @tc.desc      : two levels task dispatch
 */
HWTEST_F(TaskDispatcherTest, TaskDispatcher_Global_05600, Function | MediumTest | Level1)
{
    StartAbility(firstAbilityName, bundleNameFirst);
    std::string data = "Dispatcher_" + std::to_string((int)TestFunc::GLOBAL) + "_55";
    bool result = false;
    for (int i = 0; i < stLevel_.AMSLevel; i++) {
        STAbilityUtil::PublishEvent(g_EVENT_REQU_FIRST, ++testCaseCode, data);
        EXPECT_EQ(0, STAbilityUtil::WaitCompleted(event, g_EVENT_RESP_FIRST, testCaseCode, DELAY));
        data = STAbilityUtil::GetData(event, g_EVENT_RESP_FIRST, testCaseCode);
        result = data.compare("1") == 0;
        EXPECT_TRUE(result);
        if (!result && i > 0) {
            GTEST_LOG_(INFO) << "TaskDispatcher_Global_05600 : " << i;
            break;
        }
    }
}

/**
 * @tc.number    : TaskDispatcher_Global_05700
 * @tc.name      : global group wait:parallel group wait
 * @tc.desc      : two levels task dispatch
 */
HWTEST_F(TaskDispatcherTest, TaskDispatcher_Global_05700, Function | MediumTest | Level1)
{
    StartAbility(firstAbilityName, bundleNameFirst);
    std::string data = "Dispatcher_" + std::to_string((int)TestFunc::GLOBAL) + "_56";
    bool result = false;
    for (int i = 0; i < stLevel_.AMSLevel; i++) {
        STAbilityUtil::PublishEvent(g_EVENT_REQU_FIRST, ++testCaseCode, data);
        EXPECT_EQ(0, STAbilityUtil::WaitCompleted(event, g_EVENT_RESP_FIRST, testCaseCode, DELAY));
        data = STAbilityUtil::GetData(event, g_EVENT_RESP_FIRST, testCaseCode);
        result = data.compare("1") == 0;
        EXPECT_TRUE(result);
        if (!result && i > 0) {
            GTEST_LOG_(INFO) << "TaskDispatcher_Global_05700 : " << i;
            break;
        }
    }
}

/**
 * @tc.number    : TaskDispatcher_Global_05800
 * @tc.name      : global group wait:parallel group notify
 * @tc.desc      : two levels task dispatch
 */
HWTEST_F(TaskDispatcherTest, TaskDispatcher_Global_05800, Function | MediumTest | Level1)
{
    StartAbility(firstAbilityName, bundleNameFirst);
    std::string data = "Dispatcher_" + std::to_string((int)TestFunc::GLOBAL) + "_57";
    bool result = false;
    for (int i = 0; i < stLevel_.AMSLevel; i++) {
        STAbilityUtil::PublishEvent(g_EVENT_REQU_FIRST, ++testCaseCode, data);
        EXPECT_EQ(0, STAbilityUtil::WaitCompleted(event, g_EVENT_RESP_FIRST, testCaseCode, DELAY));
        data = STAbilityUtil::GetData(event, g_EVENT_RESP_FIRST, testCaseCode);
        result = data.compare("1") == 0;
        EXPECT_TRUE(result);
        if (!result && i > 0) {
            GTEST_LOG_(INFO) << "TaskDispatcher_Global_05800 : " << i;
            break;
        }
    }
}

/**
 * @tc.number    : TaskDispatcher_Global_05900
 * @tc.name      : global group wait:parallel sync barrier
 * @tc.desc      : two levels task dispatch
 */
HWTEST_F(TaskDispatcherTest, TaskDispatcher_Global_05900, Function | MediumTest | Level1)
{
    StartAbility(firstAbilityName, bundleNameFirst);
    std::string data = "Dispatcher_" + std::to_string((int)TestFunc::GLOBAL) + "_58";
    bool result = false;
    for (int i = 0; i < stLevel_.AMSLevel; i++) {
        STAbilityUtil::PublishEvent(g_EVENT_REQU_FIRST, ++testCaseCode, data);
        EXPECT_EQ(0, STAbilityUtil::WaitCompleted(event, g_EVENT_RESP_FIRST, testCaseCode, DELAY));
        data = STAbilityUtil::GetData(event, g_EVENT_RESP_FIRST, testCaseCode);
        result = data.compare("1") == 0;
        EXPECT_TRUE(result);
        if (!result && i > 0) {
            GTEST_LOG_(INFO) << "TaskDispatcher_Global_05900 : " << i;
            break;
        }
    }
}

/**
 * @tc.number    : TaskDispatcher_Global_06000
 * @tc.name      : global group wait:parallel async barrier
 * @tc.desc      : two levels task dispatch
 */
HWTEST_F(TaskDispatcherTest, TaskDispatcher_Global_06000, Function | MediumTest | Level1)
{
    StartAbility(firstAbilityName, bundleNameFirst);
    std::string data = "Dispatcher_" + std::to_string((int)TestFunc::GLOBAL) + "_59";
    bool result = false;
    for (int i = 0; i < stLevel_.AMSLevel; i++) {
        STAbilityUtil::PublishEvent(g_EVENT_REQU_FIRST, ++testCaseCode, data);
        EXPECT_EQ(0, STAbilityUtil::WaitCompleted(event, g_EVENT_RESP_FIRST, testCaseCode, DELAY));
        data = STAbilityUtil::GetData(event, g_EVENT_RESP_FIRST, testCaseCode);
        result = data.compare("1") == 0;
        EXPECT_TRUE(result);
        if (!result && i > 0) {
            GTEST_LOG_(INFO) << "TaskDispatcher_Global_06000 : " << i;
            break;
        }
    }
}

/**
 * @tc.number    : TaskDispatcher_Global_06100
 * @tc.name      : global group wait:parallel apply
 * @tc.desc      : two levels task dispatch
 */
HWTEST_F(TaskDispatcherTest, TaskDispatcher_Global_06100, Function | MediumTest | Level1)
{
    StartAbility(firstAbilityName, bundleNameFirst);
    std::string data = "Dispatcher_" + std::to_string((int)TestFunc::GLOBAL) + "_60";
    bool result = false;
    for (int i = 0; i < stLevel_.AMSLevel; i++) {
        STAbilityUtil::PublishEvent(g_EVENT_REQU_FIRST, ++testCaseCode, data);
        EXPECT_EQ(0, STAbilityUtil::WaitCompleted(event, g_EVENT_RESP_FIRST, testCaseCode, DELAY));
        data = STAbilityUtil::GetData(event, g_EVENT_RESP_FIRST, testCaseCode);
        result = data.compare("1") == 0;
        EXPECT_TRUE(result);
        if (!result && i > 0) {
            GTEST_LOG_(INFO) << "TaskDispatcher_Global_06100 : " << i;
            break;
        }
    }
}

/**
 * @tc.number    : TaskDispatcher_Global_06200
 * @tc.name      : global group wait:serial sync
 * @tc.desc      : two levels task dispatch
 */
HWTEST_F(TaskDispatcherTest, TaskDispatcher_Global_06200, Function | MediumTest | Level1)
{
    StartAbility(firstAbilityName, bundleNameFirst);
    std::string data = "Dispatcher_" + std::to_string((int)TestFunc::GLOBAL) + "_61";
    bool result = false;
    for (int i = 0; i < stLevel_.AMSLevel; i++) {
        STAbilityUtil::PublishEvent(g_EVENT_REQU_FIRST, ++testCaseCode, data);
        EXPECT_EQ(0, STAbilityUtil::WaitCompleted(event, g_EVENT_RESP_FIRST, testCaseCode, DELAY));
        data = STAbilityUtil::GetData(event, g_EVENT_RESP_FIRST, testCaseCode);
        result = data.compare("1") == 0;
        EXPECT_TRUE(result);
        if (!result && i > 0) {
            GTEST_LOG_(INFO) << "TaskDispatcher_Global_06200 : " << i;
            break;
        }
    }
}

/**
 * @tc.number    : TaskDispatcher_Global_06300
 * @tc.name      : global group wait:serial async
 * @tc.desc      : two levels task dispatch
 */
HWTEST_F(TaskDispatcherTest, TaskDispatcher_Global_06300, Function | MediumTest | Level1)
{
    StartAbility(firstAbilityName, bundleNameFirst);
    std::string data = "Dispatcher_" + std::to_string((int)TestFunc::GLOBAL) + "_62";
    bool result = false;
    for (int i = 0; i < stLevel_.AMSLevel; i++) {
        STAbilityUtil::PublishEvent(g_EVENT_REQU_FIRST, ++testCaseCode, data);
        EXPECT_EQ(0, STAbilityUtil::WaitCompleted(event, g_EVENT_RESP_FIRST, testCaseCode, DELAY));
        data = STAbilityUtil::GetData(event, g_EVENT_RESP_FIRST, testCaseCode);
        result = data.compare("1") == 0;
        EXPECT_TRUE(result);
        if (!result && i > 0) {
            GTEST_LOG_(INFO) << "TaskDispatcher_Global_06300 : " << i;
            break;
        }
    }
}

/**
 * @tc.number    : TaskDispatcher_Global_06400
 * @tc.name      : global group wait:serial delay
 * @tc.desc      : two levels task dispatch
 */
HWTEST_F(TaskDispatcherTest, TaskDispatcher_Global_06400, Function | MediumTest | Level1)
{
    StartAbility(firstAbilityName, bundleNameFirst);
    std::string data = "Dispatcher_" + std::to_string((int)TestFunc::GLOBAL) + "_63";
    bool result = false;
    for (int i = 0; i < stLevel_.AMSLevel; i++) {
        STAbilityUtil::PublishEvent(g_EVENT_REQU_FIRST, ++testCaseCode, data);
        EXPECT_EQ(0, STAbilityUtil::WaitCompleted(event, g_EVENT_RESP_FIRST, testCaseCode, DELAY));
        data = STAbilityUtil::GetData(event, g_EVENT_RESP_FIRST, testCaseCode);
        result = data.compare("1") == 0;
        EXPECT_TRUE(result);
        if (!result && i > 0) {
            GTEST_LOG_(INFO) << "TaskDispatcher_Global_06400 : " << i;
            break;
        }
    }
}

/**
 * @tc.number    : TaskDispatcher_Global_06500
 * @tc.name      : global group wait:serial apply
 * @tc.desc      : two levels task dispatch
 */
HWTEST_F(TaskDispatcherTest, TaskDispatcher_Global_06500, Function | MediumTest | Level1)
{
    StartAbility(firstAbilityName, bundleNameFirst);
    std::string data = "Dispatcher_" + std::to_string((int)TestFunc::GLOBAL) + "_64";
    bool result = false;
    for (int i = 0; i < stLevel_.AMSLevel; i++) {
        STAbilityUtil::PublishEvent(g_EVENT_REQU_FIRST, ++testCaseCode, data);
        EXPECT_EQ(0, STAbilityUtil::WaitCompleted(event, g_EVENT_RESP_FIRST, testCaseCode, DELAY));
        data = STAbilityUtil::GetData(event, g_EVENT_RESP_FIRST, testCaseCode);
        result = data.compare("1") == 0;
        EXPECT_TRUE(result);
        if (!result && i > 0) {
            GTEST_LOG_(INFO) << "TaskDispatcher_Global_06500 : " << i;
            break;
        }
    }
}

/**
 * @tc.number    : TaskDispatcher_Global_06600
 * @tc.name      : global group notify:parallel sync
 * @tc.desc      : two levels task dispatch
 */
HWTEST_F(TaskDispatcherTest, TaskDispatcher_Global_06600, Function | MediumTest | Level1)
{
    StartAbility(firstAbilityName, bundleNameFirst);
    std::string data = "Dispatcher_" + std::to_string((int)TestFunc::GLOBAL) + "_65";
    bool result = false;
    for (int i = 0; i < stLevel_.AMSLevel; i++) {
        STAbilityUtil::PublishEvent(g_EVENT_REQU_FIRST, ++testCaseCode, data);
        EXPECT_EQ(0, STAbilityUtil::WaitCompleted(event, g_EVENT_RESP_FIRST, testCaseCode, DELAY));
        data = STAbilityUtil::GetData(event, g_EVENT_RESP_FIRST, testCaseCode);
        result = data.compare("1") == 0;
        EXPECT_TRUE(result);
        if (!result && i > 0) {
            GTEST_LOG_(INFO) << "TaskDispatcher_Global_06600 : " << i;
            break;
        }
    }
}

/**
 * @tc.number    : TaskDispatcher_Global_06700
 * @tc.name      : global group notify:parallel async
 * @tc.desc      : two levels task dispatch
 */
HWTEST_F(TaskDispatcherTest, TaskDispatcher_Global_06700, Function | MediumTest | Level1)
{
    StartAbility(firstAbilityName, bundleNameFirst);
    std::string data = "Dispatcher_" + std::to_string((int)TestFunc::GLOBAL) + "_66";
    bool result = false;
    for (int i = 0; i < stLevel_.AMSLevel; i++) {
        STAbilityUtil::PublishEvent(g_EVENT_REQU_FIRST, ++testCaseCode, data);
        EXPECT_EQ(0, STAbilityUtil::WaitCompleted(event, g_EVENT_RESP_FIRST, testCaseCode, DELAY));
        data = STAbilityUtil::GetData(event, g_EVENT_RESP_FIRST, testCaseCode);
        result = data.compare("1") == 0;
        EXPECT_TRUE(result);
        if (!result && i > 0) {
            GTEST_LOG_(INFO) << "TaskDispatcher_Global_06700 : " << i;
            break;
        }
    }
}

/**
 * @tc.number    : TaskDispatcher_Global_06800
 * @tc.name      : global group notify:parallel delay
 * @tc.desc      : two levels task dispatch
 */
HWTEST_F(TaskDispatcherTest, TaskDispatcher_Global_06800, Function | MediumTest | Level1)
{
    StartAbility(firstAbilityName, bundleNameFirst);
    std::string data = "Dispatcher_" + std::to_string((int)TestFunc::GLOBAL) + "_67";
    bool result = false;
    for (int i = 0; i < stLevel_.AMSLevel; i++) {
        STAbilityUtil::PublishEvent(g_EVENT_REQU_FIRST, ++testCaseCode, data);
        EXPECT_EQ(0, STAbilityUtil::WaitCompleted(event, g_EVENT_RESP_FIRST, testCaseCode, DELAY));
        data = STAbilityUtil::GetData(event, g_EVENT_RESP_FIRST, testCaseCode);
        result = data.compare("1") == 0;
        EXPECT_TRUE(result);
        if (!result && i > 0) {
            GTEST_LOG_(INFO) << "TaskDispatcher_Global_06800 : " << i;
            break;
        }
    }
}

/**
 * @tc.number    : TaskDispatcher_Global_06900
 * @tc.name      : global group notify:parallel group
 * @tc.desc      : two levels task dispatch
 */
HWTEST_F(TaskDispatcherTest, TaskDispatcher_Global_06900, Function | MediumTest | Level1)
{
    StartAbility(firstAbilityName, bundleNameFirst);
    std::string data = "Dispatcher_" + std::to_string((int)TestFunc::GLOBAL) + "_68";
    bool result = false;
    for (int i = 0; i < stLevel_.AMSLevel; i++) {
        STAbilityUtil::PublishEvent(g_EVENT_REQU_FIRST, ++testCaseCode, data);
        EXPECT_EQ(0, STAbilityUtil::WaitCompleted(event, g_EVENT_RESP_FIRST, testCaseCode, DELAY));
        data = STAbilityUtil::GetData(event, g_EVENT_RESP_FIRST, testCaseCode);
        result = data.compare("1") == 0;
        EXPECT_TRUE(result);
        if (!result && i > 0) {
            GTEST_LOG_(INFO) << "TaskDispatcher_Global_06900 : " << i;
            break;
        }
    }
}

/**
 * @tc.number    : TaskDispatcher_Global_07000
 * @tc.name      : global group notify:parallel group wait
 * @tc.desc      : two levels task dispatch
 */
HWTEST_F(TaskDispatcherTest, TaskDispatcher_Global_07000, Function | MediumTest | Level1)
{
    StartAbility(firstAbilityName, bundleNameFirst);
    std::string data = "Dispatcher_" + std::to_string((int)TestFunc::GLOBAL) + "_69";
    bool result = false;
    for (int i = 0; i < stLevel_.AMSLevel; i++) {
        STAbilityUtil::PublishEvent(g_EVENT_REQU_FIRST, ++testCaseCode, data);
        EXPECT_EQ(0, STAbilityUtil::WaitCompleted(event, g_EVENT_RESP_FIRST, testCaseCode, DELAY));
        data = STAbilityUtil::GetData(event, g_EVENT_RESP_FIRST, testCaseCode);
        result = data.compare("1") == 0;
        EXPECT_TRUE(result);
        if (!result && i > 0) {
            GTEST_LOG_(INFO) << "TaskDispatcher_Global_07000 : " << i;
            break;
        }
    }
}

/**
 * @tc.number    : TaskDispatcher_Global_07100
 * @tc.name      : global group notify:parallel group notify
 * @tc.desc      : two levels task dispatch
 */
HWTEST_F(TaskDispatcherTest, TaskDispatcher_Global_07100, Function | MediumTest | Level1)
{
    StartAbility(firstAbilityName, bundleNameFirst);
    std::string data = "Dispatcher_" + std::to_string((int)TestFunc::GLOBAL) + "_70";
    bool result = false;
    for (int i = 0; i < stLevel_.AMSLevel; i++) {
        STAbilityUtil::PublishEvent(g_EVENT_REQU_FIRST, ++testCaseCode, data);
        EXPECT_EQ(0, STAbilityUtil::WaitCompleted(event, g_EVENT_RESP_FIRST, testCaseCode, DELAY));
        data = STAbilityUtil::GetData(event, g_EVENT_RESP_FIRST, testCaseCode);
        result = data.compare("1") == 0;
        EXPECT_TRUE(result);
        if (!result && i > 0) {
            GTEST_LOG_(INFO) << "TaskDispatcher_Global_07100 : " << i;
            break;
        }
    }
}

/**
 * @tc.number    : TaskDispatcher_Global_07200
 * @tc.name      : global group notify:parallel sync barrier
 * @tc.desc      : two levels task dispatch
 */
HWTEST_F(TaskDispatcherTest, TaskDispatcher_Global_07200, Function | MediumTest | Level1)
{
    StartAbility(firstAbilityName, bundleNameFirst);
    std::string data = "Dispatcher_" + std::to_string((int)TestFunc::GLOBAL) + "_71";
    bool result = false;
    for (int i = 0; i < stLevel_.AMSLevel; i++) {
        STAbilityUtil::PublishEvent(g_EVENT_REQU_FIRST, ++testCaseCode, data);
        EXPECT_EQ(0, STAbilityUtil::WaitCompleted(event, g_EVENT_RESP_FIRST, testCaseCode, DELAY));
        data = STAbilityUtil::GetData(event, g_EVENT_RESP_FIRST, testCaseCode);
        result = data.compare("1") == 0;
        EXPECT_TRUE(result);
        if (!result && i > 0) {
            GTEST_LOG_(INFO) << "TaskDispatcher_Global_07200 : " << i;
            break;
        }
    }
}

/**
 * @tc.number    : TaskDispatcher_Global_07300
 * @tc.name      : global group notify:parallel async barrier
 * @tc.desc      : two levels task dispatch
 */
HWTEST_F(TaskDispatcherTest, TaskDispatcher_Global_07300, Function | MediumTest | Level1)
{
    StartAbility(firstAbilityName, bundleNameFirst);
    std::string data = "Dispatcher_" + std::to_string((int)TestFunc::GLOBAL) + "_72";
    bool result = false;
    for (int i = 0; i < stLevel_.AMSLevel; i++) {
        STAbilityUtil::PublishEvent(g_EVENT_REQU_FIRST, ++testCaseCode, data);
        EXPECT_EQ(0, STAbilityUtil::WaitCompleted(event, g_EVENT_RESP_FIRST, testCaseCode, DELAY));
        data = STAbilityUtil::GetData(event, g_EVENT_RESP_FIRST, testCaseCode);
        result = data.compare("1") == 0;
        EXPECT_TRUE(result);
        if (!result && i > 0) {
            GTEST_LOG_(INFO) << "TaskDispatcher_Global_07300 : " << i;
            break;
        }
    }
}

/**
 * @tc.number    : TaskDispatcher_Global_07400
 * @tc.name      : global group notify:parallel apply
 * @tc.desc      : two levels task dispatch
 */
HWTEST_F(TaskDispatcherTest, TaskDispatcher_Global_07400, Function | MediumTest | Level1)
{
    StartAbility(firstAbilityName, bundleNameFirst);
    std::string data = "Dispatcher_" + std::to_string((int)TestFunc::GLOBAL) + "_73";
    bool result = false;
    for (int i = 0; i < stLevel_.AMSLevel; i++) {
        STAbilityUtil::PublishEvent(g_EVENT_REQU_FIRST, ++testCaseCode, data);
        EXPECT_EQ(0, STAbilityUtil::WaitCompleted(event, g_EVENT_RESP_FIRST, testCaseCode, DELAY));
        data = STAbilityUtil::GetData(event, g_EVENT_RESP_FIRST, testCaseCode);
        result = data.compare("1") == 0;
        EXPECT_TRUE(result);
        if (!result && i > 0) {
            GTEST_LOG_(INFO) << "TaskDispatcher_Global_07400 : " << i;
            break;
        }
    }
}

/**
 * @tc.number    : TaskDispatcher_Global_07500
 * @tc.name      : global group notify:serial sync
 * @tc.desc      : two levels task dispatch
 */
HWTEST_F(TaskDispatcherTest, TaskDispatcher_Global_07500, Function | MediumTest | Level1)
{
    StartAbility(firstAbilityName, bundleNameFirst);
    std::string data = "Dispatcher_" + std::to_string((int)TestFunc::GLOBAL) + "_74";
    bool result = false;
    for (int i = 0; i < stLevel_.AMSLevel; i++) {
        STAbilityUtil::PublishEvent(g_EVENT_REQU_FIRST, ++testCaseCode, data);
        EXPECT_EQ(0, STAbilityUtil::WaitCompleted(event, g_EVENT_RESP_FIRST, testCaseCode, DELAY));
        data = STAbilityUtil::GetData(event, g_EVENT_RESP_FIRST, testCaseCode);
        result = data.compare("1") == 0;
        EXPECT_TRUE(result);
        if (!result && i > 0) {
            GTEST_LOG_(INFO) << "TaskDispatcher_Global_07500 : " << i;
            break;
        }
    }
}

/**
 * @tc.number    : TaskDispatcher_Global_07600
 * @tc.name      : global group notify:serial async
 * @tc.desc      : two levels task dispatch
 */
HWTEST_F(TaskDispatcherTest, TaskDispatcher_Global_07600, Function | MediumTest | Level1)
{
    StartAbility(firstAbilityName, bundleNameFirst);
    std::string data = "Dispatcher_" + std::to_string((int)TestFunc::GLOBAL) + "_75";
    bool result = false;
    for (int i = 0; i < stLevel_.AMSLevel; i++) {
        STAbilityUtil::PublishEvent(g_EVENT_REQU_FIRST, ++testCaseCode, data);
        EXPECT_EQ(0, STAbilityUtil::WaitCompleted(event, g_EVENT_RESP_FIRST, testCaseCode, DELAY));
        data = STAbilityUtil::GetData(event, g_EVENT_RESP_FIRST, testCaseCode);
        result = data.compare("1") == 0;
        EXPECT_TRUE(result);
        if (!result && i > 0) {
            GTEST_LOG_(INFO) << "TaskDispatcher_Global_07600 : " << i;
            break;
        }
    }
}

/**
 * @tc.number    : TaskDispatcher_Global_07700
 * @tc.name      : global group notify:serial delay
 * @tc.desc      : two levels task dispatch
 */
HWTEST_F(TaskDispatcherTest, TaskDispatcher_Global_07700, Function | MediumTest | Level1)
{
    StartAbility(firstAbilityName, bundleNameFirst);
    std::string data = "Dispatcher_" + std::to_string((int)TestFunc::GLOBAL) + "_76";
    bool result = false;
    for (int i = 0; i < stLevel_.AMSLevel; i++) {
        STAbilityUtil::PublishEvent(g_EVENT_REQU_FIRST, ++testCaseCode, data);
        EXPECT_EQ(0, STAbilityUtil::WaitCompleted(event, g_EVENT_RESP_FIRST, testCaseCode, DELAY));
        data = STAbilityUtil::GetData(event, g_EVENT_RESP_FIRST, testCaseCode);
        result = data.compare("1") == 0;
        EXPECT_TRUE(result);
        if (!result && i > 0) {
            GTEST_LOG_(INFO) << "TaskDispatcher_Global_07700 : " << i;
            break;
        }
    }
}

/**
 * @tc.number    : TaskDispatcher_Global_07800
 * @tc.name      : global group notify:serial apply
 * @tc.desc      : two levels task dispatch
 */
HWTEST_F(TaskDispatcherTest, TaskDispatcher_Global_07800, Function | MediumTest | Level1)
{
    StartAbility(firstAbilityName, bundleNameFirst);
    std::string data = "Dispatcher_" + std::to_string((int)TestFunc::GLOBAL) + "_77";
    bool result = false;
    for (int i = 0; i < stLevel_.AMSLevel; i++) {
        STAbilityUtil::PublishEvent(g_EVENT_REQU_FIRST, ++testCaseCode, data);
        EXPECT_EQ(0, STAbilityUtil::WaitCompleted(event, g_EVENT_RESP_FIRST, testCaseCode, DELAY));
        data = STAbilityUtil::GetData(event, g_EVENT_RESP_FIRST, testCaseCode);
        result = data.compare("1") == 0;
        EXPECT_TRUE(result);
        if (!result && i > 0) {
            GTEST_LOG_(INFO) << "TaskDispatcher_Global_07800 : " << i;
            break;
        }
    }
}

/**
 * @tc.number    : TaskDispatcher_Global_07900
 * @tc.name      : global apply:parallel sync
 * @tc.desc      : two levels task dispatch
 */
HWTEST_F(TaskDispatcherTest, TaskDispatcher_Global_07900, Function | MediumTest | Level1)
{
    StartAbility(firstAbilityName, bundleNameFirst);
    std::string data = "Dispatcher_" + std::to_string((int)TestFunc::GLOBAL) + "_78";
    bool result = false;
    for (int i = 0; i < stLevel_.AMSLevel; i++) {
        STAbilityUtil::PublishEvent(g_EVENT_REQU_FIRST, ++testCaseCode, data);
        EXPECT_EQ(0, STAbilityUtil::WaitCompleted(event, g_EVENT_RESP_FIRST, testCaseCode, DELAY));
        data = STAbilityUtil::GetData(event, g_EVENT_RESP_FIRST, testCaseCode);
        result = data.compare("1") == 0;
        EXPECT_TRUE(result);
        if (!result && i > 0) {
            GTEST_LOG_(INFO) << "TaskDispatcher_Global_07900 : " << i;
            break;
        }
    }
}

/**
 * @tc.number    : TaskDispatcher_Global_08000
 * @tc.name      : global apply:parallel async
 * @tc.desc      : two levels task dispatch
 */
HWTEST_F(TaskDispatcherTest, TaskDispatcher_Global_08000, Function | MediumTest | Level1)
{
    StartAbility(firstAbilityName, bundleNameFirst);
    std::string data = "Dispatcher_" + std::to_string((int)TestFunc::GLOBAL) + "_79";
    bool result = false;
    for (int i = 0; i < stLevel_.AMSLevel; i++) {
        STAbilityUtil::PublishEvent(g_EVENT_REQU_FIRST, ++testCaseCode, data);
        EXPECT_EQ(0, STAbilityUtil::WaitCompleted(event, g_EVENT_RESP_FIRST, testCaseCode, DELAY));
        data = STAbilityUtil::GetData(event, g_EVENT_RESP_FIRST, testCaseCode);
        result = data.compare("1") == 0;
        EXPECT_TRUE(result);
        if (!result && i > 0) {
            GTEST_LOG_(INFO) << "TaskDispatcher_Global_08000 : " << i;
            break;
        }
    }
}

/**
 * @tc.number    : TaskDispatcher_Global_08100
 * @tc.name      : global apply:parallel delay
 * @tc.desc      : two levels task dispatch
 */
HWTEST_F(TaskDispatcherTest, TaskDispatcher_Global_08100, Function | MediumTest | Level1)
{
    StartAbility(firstAbilityName, bundleNameFirst);
    std::string data = "Dispatcher_" + std::to_string((int)TestFunc::GLOBAL) + "_80";
    bool result = false;
    for (int i = 0; i < stLevel_.AMSLevel; i++) {
        STAbilityUtil::PublishEvent(g_EVENT_REQU_FIRST, ++testCaseCode, data);
        EXPECT_EQ(0, STAbilityUtil::WaitCompleted(event, g_EVENT_RESP_FIRST, testCaseCode, DELAY));
        data = STAbilityUtil::GetData(event, g_EVENT_RESP_FIRST, testCaseCode);
        result = data.compare("1") == 0;
        EXPECT_TRUE(result);
        if (!result && i > 0) {
            GTEST_LOG_(INFO) << "TaskDispatcher_Global_08100 : " << i;
            break;
        }
    }
}

/**
 * @tc.number    : TaskDispatcher_Global_08200
 * @tc.name      : global apply:parallel group
 * @tc.desc      : two levels task dispatch
 */
HWTEST_F(TaskDispatcherTest, TaskDispatcher_Global_08200, Function | MediumTest | Level1)
{
    StartAbility(firstAbilityName, bundleNameFirst);
    std::string data = "Dispatcher_" + std::to_string((int)TestFunc::GLOBAL) + "_81";
    bool result = false;
    for (int i = 0; i < stLevel_.AMSLevel; i++) {
        STAbilityUtil::PublishEvent(g_EVENT_REQU_FIRST, ++testCaseCode, data);
        EXPECT_EQ(0, STAbilityUtil::WaitCompleted(event, g_EVENT_RESP_FIRST, testCaseCode, DELAY));
        data = STAbilityUtil::GetData(event, g_EVENT_RESP_FIRST, testCaseCode);
        result = data.compare("1") == 0;
        EXPECT_TRUE(result);
        if (!result && i > 0) {
            GTEST_LOG_(INFO) << "TaskDispatcher_Global_08200 : " << i;
            break;
        }
    }
}

/**
 * @tc.number    : TaskDispatcher_Global_08300
 * @tc.name      : global apply:parallel group wait
 * @tc.desc      : two levels task dispatch
 */
HWTEST_F(TaskDispatcherTest, TaskDispatcher_Global_08300, Function | MediumTest | Level1)
{
    StartAbility(firstAbilityName, bundleNameFirst);
    std::string data = "Dispatcher_" + std::to_string((int)TestFunc::GLOBAL) + "_82";
    bool result = false;
    for (int i = 0; i < stLevel_.AMSLevel; i++) {
        STAbilityUtil::PublishEvent(g_EVENT_REQU_FIRST, ++testCaseCode, data);
        EXPECT_EQ(0, STAbilityUtil::WaitCompleted(event, g_EVENT_RESP_FIRST, testCaseCode, DELAY));
        data = STAbilityUtil::GetData(event, g_EVENT_RESP_FIRST, testCaseCode);
        result = data.compare("1") == 0;
        EXPECT_TRUE(result);
        if (!result && i > 0) {
            GTEST_LOG_(INFO) << "TaskDispatcher_Global_08300 : " << i;
            break;
        }
    }
}

/**
 * @tc.number    : TaskDispatcher_Global_08400
 * @tc.name      : global apply:parallel group notify
 * @tc.desc      : two levels task dispatch
 */
HWTEST_F(TaskDispatcherTest, TaskDispatcher_Global_08400, Function | MediumTest | Level1)
{
    StartAbility(firstAbilityName, bundleNameFirst);
    std::string data = "Dispatcher_" + std::to_string((int)TestFunc::GLOBAL) + "_83";
    bool result = false;
    for (int i = 0; i < stLevel_.AMSLevel; i++) {
        STAbilityUtil::PublishEvent(g_EVENT_REQU_FIRST, ++testCaseCode, data);
        EXPECT_EQ(0, STAbilityUtil::WaitCompleted(event, g_EVENT_RESP_FIRST, testCaseCode, DELAY));
        data = STAbilityUtil::GetData(event, g_EVENT_RESP_FIRST, testCaseCode);
        result = data.compare("1") == 0;
        EXPECT_TRUE(result);
        if (!result && i > 0) {
            GTEST_LOG_(INFO) << "TaskDispatcher_Global_08400 : " << i;
            break;
        }
    }
}

/**
 * @tc.number    : TaskDispatcher_Global_08500
 * @tc.name      : global apply:parallel sync barrier
 * @tc.desc      : two levels task dispatch
 */
HWTEST_F(TaskDispatcherTest, TaskDispatcher_Global_08500, Function | MediumTest | Level1)
{
    StartAbility(firstAbilityName, bundleNameFirst);
    std::string data = "Dispatcher_" + std::to_string((int)TestFunc::GLOBAL) + "_84";
    bool result = false;
    for (int i = 0; i < stLevel_.AMSLevel; i++) {
        STAbilityUtil::PublishEvent(g_EVENT_REQU_FIRST, ++testCaseCode, data);
        EXPECT_EQ(0, STAbilityUtil::WaitCompleted(event, g_EVENT_RESP_FIRST, testCaseCode, DELAY));
        data = STAbilityUtil::GetData(event, g_EVENT_RESP_FIRST, testCaseCode);
        result = data.compare("1") == 0;
        EXPECT_TRUE(result);
        if (!result && i > 0) {
            GTEST_LOG_(INFO) << "TaskDispatcher_Global_08500 : " << i;
            break;
        }
    }
}

/**
 * @tc.number    : TaskDispatcher_Global_08600
 * @tc.name      : global apply:parallel async barrier
 * @tc.desc      : two levels task dispatch
 */
HWTEST_F(TaskDispatcherTest, TaskDispatcher_Global_08600, Function | MediumTest | Level1)
{
    StartAbility(firstAbilityName, bundleNameFirst);
    std::string data = "Dispatcher_" + std::to_string((int)TestFunc::GLOBAL) + "_85";
    bool result = false;
    for (int i = 0; i < stLevel_.AMSLevel; i++) {
        STAbilityUtil::PublishEvent(g_EVENT_REQU_FIRST, ++testCaseCode, data);
        EXPECT_EQ(0, STAbilityUtil::WaitCompleted(event, g_EVENT_RESP_FIRST, testCaseCode, DELAY));
        data = STAbilityUtil::GetData(event, g_EVENT_RESP_FIRST, testCaseCode);
        result = data.compare("1") == 0;
        EXPECT_TRUE(result);
        if (!result && i > 0) {
            GTEST_LOG_(INFO) << "TaskDispatcher_Global_08600 : " << i;
            break;
        }
    }
}

/**
 * @tc.number    : TaskDispatcher_Global_08700
 * @tc.name      : global apply:parallel apply
 * @tc.desc      : two levels task dispatch
 */
HWTEST_F(TaskDispatcherTest, TaskDispatcher_Global_08700, Function | MediumTest | Level1)
{
    StartAbility(firstAbilityName, bundleNameFirst);
    std::string data = "Dispatcher_" + std::to_string((int)TestFunc::GLOBAL) + "_86";
    bool result = false;
    for (int i = 0; i < stLevel_.AMSLevel; i++) {
        STAbilityUtil::PublishEvent(g_EVENT_REQU_FIRST, ++testCaseCode, data);
        EXPECT_EQ(0, STAbilityUtil::WaitCompleted(event, g_EVENT_RESP_FIRST, testCaseCode, DELAY));
        data = STAbilityUtil::GetData(event, g_EVENT_RESP_FIRST, testCaseCode);
        result = data.compare("1") == 0;
        EXPECT_TRUE(result);
        if (!result && i > 0) {
            GTEST_LOG_(INFO) << "TaskDispatcher_Global_08700 : " << i;
            break;
        }
    }
}

/**
 * @tc.number    : TaskDispatcher_Global_08800
 * @tc.name      : global apply:serial sync
 * @tc.desc      : two levels task dispatch
 */
HWTEST_F(TaskDispatcherTest, TaskDispatcher_Global_08800, Function | MediumTest | Level1)
{
    StartAbility(firstAbilityName, bundleNameFirst);
    std::string data = "Dispatcher_" + std::to_string((int)TestFunc::GLOBAL) + "_87";
    bool result = false;
    for (int i = 0; i < stLevel_.AMSLevel; i++) {
        STAbilityUtil::PublishEvent(g_EVENT_REQU_FIRST, ++testCaseCode, data);
        EXPECT_EQ(0, STAbilityUtil::WaitCompleted(event, g_EVENT_RESP_FIRST, testCaseCode, DELAY));
        data = STAbilityUtil::GetData(event, g_EVENT_RESP_FIRST, testCaseCode);
        result = data.compare("1") == 0;
        EXPECT_TRUE(result);
        if (!result && i > 0) {
            GTEST_LOG_(INFO) << "TaskDispatcher_Global_08800 : " << i;
            break;
        }
    }
}

/**
 * @tc.number    : TaskDispatcher_Global_08900
 * @tc.name      : global apply:serial async
 * @tc.desc      : two levels task dispatch
 */
HWTEST_F(TaskDispatcherTest, TaskDispatcher_Global_08900, Function | MediumTest | Level1)
{
    StartAbility(firstAbilityName, bundleNameFirst);
    std::string data = "Dispatcher_" + std::to_string((int)TestFunc::GLOBAL) + "_88";
    bool result = false;
    for (int i = 0; i < stLevel_.AMSLevel; i++) {
        STAbilityUtil::PublishEvent(g_EVENT_REQU_FIRST, ++testCaseCode, data);
        EXPECT_EQ(0, STAbilityUtil::WaitCompleted(event, g_EVENT_RESP_FIRST, testCaseCode, DELAY));
        data = STAbilityUtil::GetData(event, g_EVENT_RESP_FIRST, testCaseCode);
        result = data.compare("1") == 0;
        EXPECT_TRUE(result);
        if (!result && i > 0) {
            GTEST_LOG_(INFO) << "TaskDispatcher_Global_08900 : " << i;
            break;
        }
    }
}

/**
 * @tc.number    : TaskDispatcher_Global_09000
 * @tc.name      : global apply:serial delay
 * @tc.desc      : two levels task dispatch
 */
HWTEST_F(TaskDispatcherTest, TaskDispatcher_Global_09000, Function | MediumTest | Level1)
{
    StartAbility(firstAbilityName, bundleNameFirst);
    std::string data = "Dispatcher_" + std::to_string((int)TestFunc::GLOBAL) + "_89";
    bool result = false;
    for (int i = 0; i < stLevel_.AMSLevel; i++) {
        STAbilityUtil::PublishEvent(g_EVENT_REQU_FIRST, ++testCaseCode, data);
        EXPECT_EQ(0, STAbilityUtil::WaitCompleted(event, g_EVENT_RESP_FIRST, testCaseCode, DELAY));
        data = STAbilityUtil::GetData(event, g_EVENT_RESP_FIRST, testCaseCode);
        result = data.compare("1") == 0;
        EXPECT_TRUE(result);
        if (!result && i > 0) {
            GTEST_LOG_(INFO) << "TaskDispatcher_Global_09000 : " << i;
            break;
        }
    }
}

/**
 * @tc.number    : TaskDispatcher_Global_09100
 * @tc.name      : global apply:serial apply
 * @tc.desc      : two levels task dispatch
 */
HWTEST_F(TaskDispatcherTest, TaskDispatcher_Global_09100, Function | MediumTest | Level1)
{
    StartAbility(firstAbilityName, bundleNameFirst);
    std::string data = "Dispatcher_" + std::to_string((int)TestFunc::GLOBAL) + "_90";
    bool result = false;
    for (int i = 0; i < stLevel_.AMSLevel; i++) {
        STAbilityUtil::PublishEvent(g_EVENT_REQU_FIRST, ++testCaseCode, data);
        EXPECT_EQ(0, STAbilityUtil::WaitCompleted(event, g_EVENT_RESP_FIRST, testCaseCode, DELAY));
        data = STAbilityUtil::GetData(event, g_EVENT_RESP_FIRST, testCaseCode);
        result = data.compare("1") == 0;
        EXPECT_TRUE(result);
        if (!result && i > 0) {
            GTEST_LOG_(INFO) << "TaskDispatcher_Global_09100 : " << i;
            break;
        }
    }
}

/**
 * @tc.number    : TaskDispatcher_Parallel_00100
 * @tc.name      : parallel sync:parallel sync
 * @tc.desc      : two levels task dispatch
 */
HWTEST_F(TaskDispatcherTest, TaskDispatcher_Parallel_00100, Function | MediumTest | Level1)
{
    StartAbility(firstAbilityName, bundleNameFirst);
    std::string data = "Dispatcher_" + std::to_string((int)TestFunc::PARALLEL) + "_0";
    bool result = false;
    for (int i = 0; i < stLevel_.AMSLevel; i++) {
        STAbilityUtil::PublishEvent(g_EVENT_REQU_FIRST, ++testCaseCode, data);
        EXPECT_EQ(0, STAbilityUtil::WaitCompleted(event, g_EVENT_RESP_FIRST, testCaseCode, DELAY));
        data = STAbilityUtil::GetData(event, g_EVENT_RESP_FIRST, testCaseCode);
        result = data.compare("1") == 0;
        EXPECT_TRUE(result);
        if (!result && i > 0) {
            GTEST_LOG_(INFO) << "TaskDispatcher_Parallel_00100 : " << i;
            break;
        }
    }
}

/**
 * @tc.number    : TaskDispatcher_Parallel_00200
 * @tc.name      : parallel sync:parallel async
 * @tc.desc      : two levels task dispatch
 */
HWTEST_F(TaskDispatcherTest, TaskDispatcher_Parallel_00200, Function | MediumTest | Level1)
{
    StartAbility(firstAbilityName, bundleNameFirst);
    std::string data = "Dispatcher_" + std::to_string((int)TestFunc::PARALLEL) + "_1";
    bool result = false;
    for (int i = 0; i < stLevel_.AMSLevel; i++) {
        STAbilityUtil::PublishEvent(g_EVENT_REQU_FIRST, ++testCaseCode, data);
        EXPECT_EQ(0, STAbilityUtil::WaitCompleted(event, g_EVENT_RESP_FIRST, testCaseCode, DELAY));
        data = STAbilityUtil::GetData(event, g_EVENT_RESP_FIRST, testCaseCode);
        result = data.compare("1") == 0;
        EXPECT_TRUE(result);
        if (!result && i > 0) {
            GTEST_LOG_(INFO) << "TaskDispatcher_Parallel_00200 : " << i;
            break;
        }
    }
}

/**
 * @tc.number    : TaskDispatcher_Parallel_00300
 * @tc.name      : parallel sync:parallel delay
 * @tc.desc      : two levels task dispatch
 */
HWTEST_F(TaskDispatcherTest, TaskDispatcher_Parallel_00300, Function | MediumTest | Level1)
{
    StartAbility(firstAbilityName, bundleNameFirst);
    std::string data = "Dispatcher_" + std::to_string((int)TestFunc::PARALLEL) + "_2";
    bool result = false;
    for (int i = 0; i < stLevel_.AMSLevel; i++) {
        STAbilityUtil::PublishEvent(g_EVENT_REQU_FIRST, ++testCaseCode, data);
        EXPECT_EQ(0, STAbilityUtil::WaitCompleted(event, g_EVENT_RESP_FIRST, testCaseCode, DELAY));
        data = STAbilityUtil::GetData(event, g_EVENT_RESP_FIRST, testCaseCode);
        result = data.compare("1") == 0;
        EXPECT_TRUE(result);
        if (!result && i > 0) {
            GTEST_LOG_(INFO) << "TaskDispatcher_Parallel_00300 : " << i;
            break;
        }
    }
}

/**
 * @tc.number    : TaskDispatcher_Parallel_00400
 * @tc.name      : parallel sync:parallel group
 * @tc.desc      : two levels task dispatch
 */
HWTEST_F(TaskDispatcherTest, TaskDispatcher_Parallel_00400, Function | MediumTest | Level1)
{
    StartAbility(firstAbilityName, bundleNameFirst);
    std::string data = "Dispatcher_" + std::to_string((int)TestFunc::PARALLEL) + "_3";
    bool result = false;
    for (int i = 0; i < stLevel_.AMSLevel; i++) {
        STAbilityUtil::PublishEvent(g_EVENT_REQU_FIRST, ++testCaseCode, data);
        EXPECT_EQ(0, STAbilityUtil::WaitCompleted(event, g_EVENT_RESP_FIRST, testCaseCode, DELAY));
        data = STAbilityUtil::GetData(event, g_EVENT_RESP_FIRST, testCaseCode);
        result = data.compare("1") == 0;
        EXPECT_TRUE(result);
        if (!result && i > 0) {
            GTEST_LOG_(INFO) << "TaskDispatcher_Parallel_00400 : " << i;
            break;
        }
    }
}

/**
 * @tc.number    : TaskDispatcher_Parallel_00500
 * @tc.name      : parallel sync:parallel group wait
 * @tc.desc      : two levels task dispatch
 */
HWTEST_F(TaskDispatcherTest, TaskDispatcher_Parallel_00500, Function | MediumTest | Level1)
{
    StartAbility(firstAbilityName, bundleNameFirst);
    std::string data = "Dispatcher_" + std::to_string((int)TestFunc::PARALLEL) + "_4";
    bool result = false;
    for (int i = 0; i < stLevel_.AMSLevel; i++) {
        STAbilityUtil::PublishEvent(g_EVENT_REQU_FIRST, ++testCaseCode, data);
        EXPECT_EQ(0, STAbilityUtil::WaitCompleted(event, g_EVENT_RESP_FIRST, testCaseCode, DELAY));
        data = STAbilityUtil::GetData(event, g_EVENT_RESP_FIRST, testCaseCode);
        result = data.compare("1") == 0;
        EXPECT_TRUE(result);
        if (!result && i > 0) {
            GTEST_LOG_(INFO) << "TaskDispatcher_Parallel_00500 : " << i;
            break;
        }
    }
}

/**
 * @tc.number    : TaskDispatcher_Parallel_00600
 * @tc.name      : parallel sync:parallel group notify
 * @tc.desc      : two levels task dispatch
 */
HWTEST_F(TaskDispatcherTest, TaskDispatcher_Parallel_00600, Function | MediumTest | Level1)
{
    StartAbility(firstAbilityName, bundleNameFirst);
    std::string data = "Dispatcher_" + std::to_string((int)TestFunc::PARALLEL) + "_5";
    bool result = false;
    for (int i = 0; i < stLevel_.AMSLevel; i++) {
        STAbilityUtil::PublishEvent(g_EVENT_REQU_FIRST, ++testCaseCode, data);
        EXPECT_EQ(0, STAbilityUtil::WaitCompleted(event, g_EVENT_RESP_FIRST, testCaseCode, DELAY));
        data = STAbilityUtil::GetData(event, g_EVENT_RESP_FIRST, testCaseCode);
        result = data.compare("1") == 0;
        EXPECT_TRUE(result);
        if (!result && i > 0) {
            GTEST_LOG_(INFO) << "TaskDispatcher_Parallel_00600 : " << i;
            break;
        }
    }
}

/**
 * @tc.number    : TaskDispatcher_Parallel_00700
 * @tc.name      : parallel sync:parallel sync barrier
 * @tc.desc      : two levels task dispatch
 */
HWTEST_F(TaskDispatcherTest, TaskDispatcher_Parallel_00700, Function | MediumTest | Level1)
{
    StartAbility(firstAbilityName, bundleNameFirst);
    std::string data = "Dispatcher_" + std::to_string((int)TestFunc::PARALLEL) + "_6";
    bool result = false;
    for (int i = 0; i < stLevel_.AMSLevel; i++) {
        STAbilityUtil::PublishEvent(g_EVENT_REQU_FIRST, ++testCaseCode, data);
        EXPECT_EQ(0, STAbilityUtil::WaitCompleted(event, g_EVENT_RESP_FIRST, testCaseCode, DELAY));
        data = STAbilityUtil::GetData(event, g_EVENT_RESP_FIRST, testCaseCode);
        result = data.compare("1") == 0;
        EXPECT_TRUE(result);
        if (!result && i > 0) {
            GTEST_LOG_(INFO) << "TaskDispatcher_Parallel_00700 : " << i;
            break;
        }
    }
}

/**
 * @tc.number    : TaskDispatcher_Parallel_00800
 * @tc.name      : parallel sync:parallel async barrier
 * @tc.desc      : two levels task dispatch
 */
HWTEST_F(TaskDispatcherTest, TaskDispatcher_Parallel_00800, Function | MediumTest | Level1)
{
    StartAbility(firstAbilityName, bundleNameFirst);
    std::string data = "Dispatcher_" + std::to_string((int)TestFunc::PARALLEL) + "_7";
    bool result = false;
    for (int i = 0; i < stLevel_.AMSLevel; i++) {
        STAbilityUtil::PublishEvent(g_EVENT_REQU_FIRST, ++testCaseCode, data);
        EXPECT_EQ(0, STAbilityUtil::WaitCompleted(event, g_EVENT_RESP_FIRST, testCaseCode, DELAY));
        data = STAbilityUtil::GetData(event, g_EVENT_RESP_FIRST, testCaseCode);
        result = data.compare("1") == 0;
        EXPECT_TRUE(result);
        if (!result && i > 0) {
            GTEST_LOG_(INFO) << "TaskDispatcher_Parallel_00800 : " << i;
            break;
        }
    }
}

/**
 * @tc.number    : TaskDispatcher_Parallel_00900
 * @tc.name      : parallel sync:parallel apply
 * @tc.desc      : two levels task dispatch
 */
HWTEST_F(TaskDispatcherTest, TaskDispatcher_Parallel_00900, Function | MediumTest | Level1)
{
    StartAbility(firstAbilityName, bundleNameFirst);
    std::string data = "Dispatcher_" + std::to_string((int)TestFunc::PARALLEL) + "_8";
    bool result = false;
    for (int i = 0; i < stLevel_.AMSLevel; i++) {
        STAbilityUtil::PublishEvent(g_EVENT_REQU_FIRST, ++testCaseCode, data);
        EXPECT_EQ(0, STAbilityUtil::WaitCompleted(event, g_EVENT_RESP_FIRST, testCaseCode, DELAY));
        data = STAbilityUtil::GetData(event, g_EVENT_RESP_FIRST, testCaseCode);
        result = data.compare("1") == 0;
        EXPECT_TRUE(result);
        if (!result && i > 0) {
            GTEST_LOG_(INFO) << "TaskDispatcher_Parallel_00900 : " << i;
            break;
        }
    }
}

/**
 * @tc.number    : TaskDispatcher_Parallel_01000
 * @tc.name      : parallel sync:serial sync
 * @tc.desc      : two levels task dispatch
 */
HWTEST_F(TaskDispatcherTest, TaskDispatcher_Parallel_01000, Function | MediumTest | Level1)
{
    StartAbility(firstAbilityName, bundleNameFirst);
    std::string data = "Dispatcher_" + std::to_string((int)TestFunc::PARALLEL) + "_9";
    bool result = false;
    for (int i = 0; i < stLevel_.AMSLevel; i++) {
        STAbilityUtil::PublishEvent(g_EVENT_REQU_FIRST, ++testCaseCode, data);
        EXPECT_EQ(0, STAbilityUtil::WaitCompleted(event, g_EVENT_RESP_FIRST, testCaseCode, DELAY));
        data = STAbilityUtil::GetData(event, g_EVENT_RESP_FIRST, testCaseCode);
        result = data.compare("1") == 0;
        EXPECT_TRUE(result);
        if (!result && i > 0) {
            GTEST_LOG_(INFO) << "TaskDispatcher_Parallel_01000 : " << i;
            break;
        }
    }
}

/**
 * @tc.number    : TaskDispatcher_Parallel_01100
 * @tc.name      : parallel sync:serial async
 * @tc.desc      : two levels task dispatch
 */
HWTEST_F(TaskDispatcherTest, TaskDispatcher_Parallel_01100, Function | MediumTest | Level1)
{
    StartAbility(firstAbilityName, bundleNameFirst);
    std::string data = "Dispatcher_" + std::to_string((int)TestFunc::PARALLEL) + "_10";
    bool result = false;
    for (int i = 0; i < stLevel_.AMSLevel; i++) {
        STAbilityUtil::PublishEvent(g_EVENT_REQU_FIRST, ++testCaseCode, data);
        EXPECT_EQ(0, STAbilityUtil::WaitCompleted(event, g_EVENT_RESP_FIRST, testCaseCode, DELAY));
        data = STAbilityUtil::GetData(event, g_EVENT_RESP_FIRST, testCaseCode);
        result = data.compare("1") == 0;
        EXPECT_TRUE(result);
        if (!result && i > 0) {
            GTEST_LOG_(INFO) << "TaskDispatcher_Parallel_01100 : " << i;
            break;
        }
    }
}

/**
 * @tc.number    : TaskDispatcher_Parallel_01200
 * @tc.name      : parallel sync:serial delay
 * @tc.desc      : two levels task dispatch
 */
HWTEST_F(TaskDispatcherTest, TaskDispatcher_Parallel_01200, Function | MediumTest | Level1)
{
    StartAbility(firstAbilityName, bundleNameFirst);
    std::string data = "Dispatcher_" + std::to_string((int)TestFunc::PARALLEL) + "_11";
    bool result = false;
    for (int i = 0; i < stLevel_.AMSLevel; i++) {
        STAbilityUtil::PublishEvent(g_EVENT_REQU_FIRST, ++testCaseCode, data);
        EXPECT_EQ(0, STAbilityUtil::WaitCompleted(event, g_EVENT_RESP_FIRST, testCaseCode, DELAY));
        data = STAbilityUtil::GetData(event, g_EVENT_RESP_FIRST, testCaseCode);
        result = data.compare("1") == 0;
        EXPECT_TRUE(result);
        if (!result && i > 0) {
            GTEST_LOG_(INFO) << "TaskDispatcher_Parallel_01200 : " << i;
            break;
        }
    }
}

/**
 * @tc.number    : TaskDispatcher_Parallel_01300
 * @tc.name      : parallel sync:serial apply
 * @tc.desc      : two levels task dispatch
 */
HWTEST_F(TaskDispatcherTest, TaskDispatcher_Parallel_01300, Function | MediumTest | Level1)
{
    StartAbility(firstAbilityName, bundleNameFirst);
    std::string data = "Dispatcher_" + std::to_string((int)TestFunc::PARALLEL) + "_12";
    bool result = false;
    for (int i = 0; i < stLevel_.AMSLevel; i++) {
        STAbilityUtil::PublishEvent(g_EVENT_REQU_FIRST, ++testCaseCode, data);
        EXPECT_EQ(0, STAbilityUtil::WaitCompleted(event, g_EVENT_RESP_FIRST, testCaseCode, DELAY));
        data = STAbilityUtil::GetData(event, g_EVENT_RESP_FIRST, testCaseCode);
        result = data.compare("1") == 0;
        EXPECT_TRUE(result);
        if (!result && i > 0) {
            GTEST_LOG_(INFO) << "TaskDispatcher_Parallel_01300 : " << i;
            break;
        }
    }
}

/**
 * @tc.number    : TaskDispatcher_Parallel_01400
 * @tc.name      : parallel async:parallel sync
 * @tc.desc      : two levels task dispatch
 */
HWTEST_F(TaskDispatcherTest, TaskDispatcher_Parallel_01400, Function | MediumTest | Level1)
{
    StartAbility(firstAbilityName, bundleNameFirst);
    std::string data = "Dispatcher_" + std::to_string((int)TestFunc::PARALLEL) + "_13";
    bool result = false;
    for (int i = 0; i < stLevel_.AMSLevel; i++) {
        STAbilityUtil::PublishEvent(g_EVENT_REQU_FIRST, ++testCaseCode, data);
        EXPECT_EQ(0, STAbilityUtil::WaitCompleted(event, g_EVENT_RESP_FIRST, testCaseCode, DELAY));
        data = STAbilityUtil::GetData(event, g_EVENT_RESP_FIRST, testCaseCode);
        result = data.compare("1") == 0;
        EXPECT_TRUE(result);
        if (!result && i > 0) {
            GTEST_LOG_(INFO) << "TaskDispatcher_Parallel_01400 : " << i;
            break;
        }
    }
}

/**
 * @tc.number    : TaskDispatcher_Parallel_01500
 * @tc.name      : parallel async:parallel async
 * @tc.desc      : two levels task dispatch
 */
HWTEST_F(TaskDispatcherTest, TaskDispatcher_Parallel_01500, Function | MediumTest | Level1)
{
    StartAbility(firstAbilityName, bundleNameFirst);
    std::string data = "Dispatcher_" + std::to_string((int)TestFunc::PARALLEL) + "_14";
    bool result = false;
    for (int i = 0; i < stLevel_.AMSLevel; i++) {
        STAbilityUtil::PublishEvent(g_EVENT_REQU_FIRST, ++testCaseCode, data);
        EXPECT_EQ(0, STAbilityUtil::WaitCompleted(event, g_EVENT_RESP_FIRST, testCaseCode, DELAY));
        data = STAbilityUtil::GetData(event, g_EVENT_RESP_FIRST, testCaseCode);
        result = data.compare("1") == 0;
        EXPECT_TRUE(result);
        if (!result && i > 0) {
            GTEST_LOG_(INFO) << "TaskDispatcher_Parallel_01500 : " << i;
            break;
        }
    }
}

/**
 * @tc.number    : TaskDispatcher_Parallel_01600
 * @tc.name      : parallel async:parallel delay
 * @tc.desc      : two levels task dispatch
 */
HWTEST_F(TaskDispatcherTest, TaskDispatcher_Parallel_01600, Function | MediumTest | Level1)
{
    StartAbility(firstAbilityName, bundleNameFirst);
    std::string data = "Dispatcher_" + std::to_string((int)TestFunc::PARALLEL) + "_15";
    bool result = false;
    for (int i = 0; i < stLevel_.AMSLevel; i++) {
        STAbilityUtil::PublishEvent(g_EVENT_REQU_FIRST, ++testCaseCode, data);
        EXPECT_EQ(0, STAbilityUtil::WaitCompleted(event, g_EVENT_RESP_FIRST, testCaseCode, DELAY));
        data = STAbilityUtil::GetData(event, g_EVENT_RESP_FIRST, testCaseCode);
        result = data.compare("1") == 0;
        EXPECT_TRUE(result);
        if (!result && i > 0) {
            GTEST_LOG_(INFO) << "TaskDispatcher_Parallel_01600 : " << i;
            break;
        }
    }
}

/**
 * @tc.number    : TaskDispatcher_Parallel_01700
 * @tc.name      : parallel async:parallel group
 * @tc.desc      : two levels task dispatch
 */
HWTEST_F(TaskDispatcherTest, TaskDispatcher_Parallel_01700, Function | MediumTest | Level1)
{
    StartAbility(firstAbilityName, bundleNameFirst);
    std::string data = "Dispatcher_" + std::to_string((int)TestFunc::PARALLEL) + "_16";
    bool result = false;
    for (int i = 0; i < stLevel_.AMSLevel; i++) {
        STAbilityUtil::PublishEvent(g_EVENT_REQU_FIRST, ++testCaseCode, data);
        EXPECT_EQ(0, STAbilityUtil::WaitCompleted(event, g_EVENT_RESP_FIRST, testCaseCode, DELAY));
        data = STAbilityUtil::GetData(event, g_EVENT_RESP_FIRST, testCaseCode);
        result = data.compare("1") == 0;
        EXPECT_TRUE(result);
        if (!result && i > 0) {
            GTEST_LOG_(INFO) << "TaskDispatcher_Parallel_01700 : " << i;
            break;
        }
    }
}

/**
 * @tc.number    : TaskDispatcher_Parallel_01800
 * @tc.name      : parallel async:parallel group wait
 * @tc.desc      : two levels task dispatch
 */
HWTEST_F(TaskDispatcherTest, TaskDispatcher_Parallel_01800, Function | MediumTest | Level1)
{
    StartAbility(firstAbilityName, bundleNameFirst);
    std::string data = "Dispatcher_" + std::to_string((int)TestFunc::PARALLEL) + "_17";
    bool result = false;
    for (int i = 0; i < stLevel_.AMSLevel; i++) {
        STAbilityUtil::PublishEvent(g_EVENT_REQU_FIRST, ++testCaseCode, data);
        EXPECT_EQ(0, STAbilityUtil::WaitCompleted(event, g_EVENT_RESP_FIRST, testCaseCode, DELAY));
        data = STAbilityUtil::GetData(event, g_EVENT_RESP_FIRST, testCaseCode);
        result = data.compare("1") == 0;
        EXPECT_TRUE(result);
        if (!result && i > 0) {
            GTEST_LOG_(INFO) << "TaskDispatcher_Parallel_01800 : " << i;
            break;
        }
    }
}

/**
 * @tc.number    : TaskDispatcher_Parallel_01900
 * @tc.name      : parallel async:parallel group notify
 * @tc.desc      : two levels task dispatch
 */
HWTEST_F(TaskDispatcherTest, TaskDispatcher_Parallel_01900, Function | MediumTest | Level1)
{
    StartAbility(firstAbilityName, bundleNameFirst);
    std::string data = "Dispatcher_" + std::to_string((int)TestFunc::PARALLEL) + "_18";
    bool result = false;
    for (int i = 0; i < stLevel_.AMSLevel; i++) {
        STAbilityUtil::PublishEvent(g_EVENT_REQU_FIRST, ++testCaseCode, data);
        EXPECT_EQ(0, STAbilityUtil::WaitCompleted(event, g_EVENT_RESP_FIRST, testCaseCode, DELAY));
        data = STAbilityUtil::GetData(event, g_EVENT_RESP_FIRST, testCaseCode);
        result = data.compare("1") == 0;
        EXPECT_TRUE(result);
        if (!result && i > 0) {
            GTEST_LOG_(INFO) << "TaskDispatcher_Parallel_01900 : " << i;
            break;
        }
    }
}

/**
 * @tc.number    : TaskDispatcher_Parallel_02000
 * @tc.name      : parallel async:parallel sync barrier
 * @tc.desc      : two levels task dispatch
 */
HWTEST_F(TaskDispatcherTest, TaskDispatcher_Parallel_02000, Function | MediumTest | Level1)
{
    StartAbility(firstAbilityName, bundleNameFirst);
    std::string data = "Dispatcher_" + std::to_string((int)TestFunc::PARALLEL) + "_19";
    bool result = false;
    for (int i = 0; i < stLevel_.AMSLevel; i++) {
        STAbilityUtil::PublishEvent(g_EVENT_REQU_FIRST, ++testCaseCode, data);
        EXPECT_EQ(0, STAbilityUtil::WaitCompleted(event, g_EVENT_RESP_FIRST, testCaseCode, DELAY));
        data = STAbilityUtil::GetData(event, g_EVENT_RESP_FIRST, testCaseCode);
        result = data.compare("1") == 0;
        EXPECT_TRUE(result);
        if (!result && i > 0) {
            GTEST_LOG_(INFO) << "TaskDispatcher_Parallel_02000 : " << i;
            break;
        }
    }
}

/**
 * @tc.number    : TaskDispatcher_Parallel_02100
 * @tc.name      : parallel async:parallel async barrier
 * @tc.desc      : two levels task dispatch
 */
HWTEST_F(TaskDispatcherTest, TaskDispatcher_Parallel_02100, Function | MediumTest | Level1)
{
    StartAbility(firstAbilityName, bundleNameFirst);
    std::string data = "Dispatcher_" + std::to_string((int)TestFunc::PARALLEL) + "_20";
    bool result = false;
    for (int i = 0; i < stLevel_.AMSLevel; i++) {
        STAbilityUtil::PublishEvent(g_EVENT_REQU_FIRST, ++testCaseCode, data);
        EXPECT_EQ(0, STAbilityUtil::WaitCompleted(event, g_EVENT_RESP_FIRST, testCaseCode, DELAY));
        data = STAbilityUtil::GetData(event, g_EVENT_RESP_FIRST, testCaseCode);
        result = data.compare("1") == 0;
        EXPECT_TRUE(result);
        if (!result && i > 0) {
            GTEST_LOG_(INFO) << "TaskDispatcher_Parallel_02100 : " << i;
            break;
        }
    }
}

/**
 * @tc.number    : TaskDispatcher_Parallel_02200
 * @tc.name      : parallel async:parallel apply
 * @tc.desc      : two levels task dispatch
 */
HWTEST_F(TaskDispatcherTest, TaskDispatcher_Parallel_02200, Function | MediumTest | Level1)
{
    StartAbility(firstAbilityName, bundleNameFirst);
    std::string data = "Dispatcher_" + std::to_string((int)TestFunc::PARALLEL) + "_21";
    bool result = false;
    for (int i = 0; i < stLevel_.AMSLevel; i++) {
        STAbilityUtil::PublishEvent(g_EVENT_REQU_FIRST, ++testCaseCode, data);
        EXPECT_EQ(0, STAbilityUtil::WaitCompleted(event, g_EVENT_RESP_FIRST, testCaseCode, DELAY));
        data = STAbilityUtil::GetData(event, g_EVENT_RESP_FIRST, testCaseCode);
        result = data.compare("1") == 0;
        EXPECT_TRUE(result);
        if (!result && i > 0) {
            GTEST_LOG_(INFO) << "TaskDispatcher_Parallel_02200 : " << i;
            break;
        }
    }
}

/**
 * @tc.number    : TaskDispatcher_Parallel_02300
 * @tc.name      : parallel async:serial sync
 * @tc.desc      : two levels task dispatch
 */
HWTEST_F(TaskDispatcherTest, TaskDispatcher_Parallel_02300, Function | MediumTest | Level1)
{
    StartAbility(firstAbilityName, bundleNameFirst);
    std::string data = "Dispatcher_" + std::to_string((int)TestFunc::PARALLEL) + "_22";
    bool result = false;
    for (int i = 0; i < stLevel_.AMSLevel; i++) {
        STAbilityUtil::PublishEvent(g_EVENT_REQU_FIRST, ++testCaseCode, data);
        EXPECT_EQ(0, STAbilityUtil::WaitCompleted(event, g_EVENT_RESP_FIRST, testCaseCode, DELAY));
        data = STAbilityUtil::GetData(event, g_EVENT_RESP_FIRST, testCaseCode);
        result = data.compare("1") == 0;
        EXPECT_TRUE(result);
        if (!result && i > 0) {
            GTEST_LOG_(INFO) << "TaskDispatcher_Parallel_02300 : " << i;
            break;
        }
    }
}

/**
 * @tc.number    : TaskDispatcher_Parallel_02400
 * @tc.name      : parallel async:serial async
 * @tc.desc      : two levels task dispatch
 */
HWTEST_F(TaskDispatcherTest, TaskDispatcher_Parallel_02400, Function | MediumTest | Level1)
{
    StartAbility(firstAbilityName, bundleNameFirst);
    std::string data = "Dispatcher_" + std::to_string((int)TestFunc::PARALLEL) + "_23";
    bool result = false;
    for (int i = 0; i < stLevel_.AMSLevel; i++) {
        STAbilityUtil::PublishEvent(g_EVENT_REQU_FIRST, ++testCaseCode, data);
        EXPECT_EQ(0, STAbilityUtil::WaitCompleted(event, g_EVENT_RESP_FIRST, testCaseCode, DELAY));
        data = STAbilityUtil::GetData(event, g_EVENT_RESP_FIRST, testCaseCode);
        result = data.compare("1") == 0;
        EXPECT_TRUE(result);
        if (!result && i > 0) {
            GTEST_LOG_(INFO) << "TaskDispatcher_Parallel_02400 : " << i;
            break;
        }
    }
}

/**
 * @tc.number    : TaskDispatcher_Parallel_02500
 * @tc.name      : parallel async:serial delay
 * @tc.desc      : two levels task dispatch
 */
HWTEST_F(TaskDispatcherTest, TaskDispatcher_Parallel_02500, Function | MediumTest | Level1)
{
    StartAbility(firstAbilityName, bundleNameFirst);
    std::string data = "Dispatcher_" + std::to_string((int)TestFunc::PARALLEL) + "_24";
    bool result = false;
    for (int i = 0; i < stLevel_.AMSLevel; i++) {
        STAbilityUtil::PublishEvent(g_EVENT_REQU_FIRST, ++testCaseCode, data);
        EXPECT_EQ(0, STAbilityUtil::WaitCompleted(event, g_EVENT_RESP_FIRST, testCaseCode, DELAY));
        data = STAbilityUtil::GetData(event, g_EVENT_RESP_FIRST, testCaseCode);
        result = data.compare("1") == 0;
        EXPECT_TRUE(result);
        if (!result && i > 0) {
            GTEST_LOG_(INFO) << "TaskDispatcher_Parallel_02500 : " << i;
            break;
        }
    }
}

/**
 * @tc.number    : TaskDispatcher_Parallel_02600
 * @tc.name      : parallel async:serial apply
 * @tc.desc      : two levels task dispatch
 */
HWTEST_F(TaskDispatcherTest, TaskDispatcher_Parallel_02600, Function | MediumTest | Level1)
{
    StartAbility(firstAbilityName, bundleNameFirst);
    std::string data = "Dispatcher_" + std::to_string((int)TestFunc::PARALLEL) + "_25";
    bool result = false;
    for (int i = 0; i < stLevel_.AMSLevel; i++) {
        STAbilityUtil::PublishEvent(g_EVENT_REQU_FIRST, ++testCaseCode, data);
        EXPECT_EQ(0, STAbilityUtil::WaitCompleted(event, g_EVENT_RESP_FIRST, testCaseCode, DELAY));
        data = STAbilityUtil::GetData(event, g_EVENT_RESP_FIRST, testCaseCode);
        result = data.compare("1") == 0;
        EXPECT_TRUE(result);
        if (!result && i > 0) {
            GTEST_LOG_(INFO) << "TaskDispatcher_Parallel_02600 : " << i;
            break;
        }
    }
}

/**
 * @tc.number    : TaskDispatcher_Parallel_02700
 * @tc.name      : parallel delay:parallel sync
 * @tc.desc      : two levels task dispatch
 */
HWTEST_F(TaskDispatcherTest, TaskDispatcher_Parallel_02700, Function | MediumTest | Level1)
{
    StartAbility(firstAbilityName, bundleNameFirst);
    std::string data = "Dispatcher_" + std::to_string((int)TestFunc::PARALLEL) + "_26";
    bool result = false;
    for (int i = 0; i < stLevel_.AMSLevel; i++) {
        STAbilityUtil::PublishEvent(g_EVENT_REQU_FIRST, ++testCaseCode, data);
        EXPECT_EQ(0, STAbilityUtil::WaitCompleted(event, g_EVENT_RESP_FIRST, testCaseCode, DELAY));
        data = STAbilityUtil::GetData(event, g_EVENT_RESP_FIRST, testCaseCode);
        result = data.compare("1") == 0;
        EXPECT_TRUE(result);
        if (!result && i > 0) {
            GTEST_LOG_(INFO) << "TaskDispatcher_Parallel_02700 : " << i;
            break;
        }
    }
}

/**
 * @tc.number    : TaskDispatcher_Parallel_02800
 * @tc.name      : parallel delay:parallel async
 * @tc.desc      : two levels task dispatch
 */
HWTEST_F(TaskDispatcherTest, TaskDispatcher_Parallel_02800, Function | MediumTest | Level1)
{
    StartAbility(firstAbilityName, bundleNameFirst);
    std::string data = "Dispatcher_" + std::to_string((int)TestFunc::PARALLEL) + "_27";
    bool result = false;
    for (int i = 0; i < stLevel_.AMSLevel; i++) {
        STAbilityUtil::PublishEvent(g_EVENT_REQU_FIRST, ++testCaseCode, data);
        EXPECT_EQ(0, STAbilityUtil::WaitCompleted(event, g_EVENT_RESP_FIRST, testCaseCode, DELAY));
        data = STAbilityUtil::GetData(event, g_EVENT_RESP_FIRST, testCaseCode);
        result = data.compare("1") == 0;
        EXPECT_TRUE(result);
        if (!result && i > 0) {
            GTEST_LOG_(INFO) << "TaskDispatcher_Parallel_02800 : " << i;
            break;
        }
    }
}

/**
 * @tc.number    : TaskDispatcher_Parallel_02900
 * @tc.name      : parallel delay:parallel delay
 * @tc.desc      : two levels task dispatch
 */
HWTEST_F(TaskDispatcherTest, TaskDispatcher_Parallel_02900, Function | MediumTest | Level1)
{
    StartAbility(firstAbilityName, bundleNameFirst);
    std::string data = "Dispatcher_" + std::to_string((int)TestFunc::PARALLEL) + "_28";
    bool result = false;
    for (int i = 0; i < stLevel_.AMSLevel; i++) {
        STAbilityUtil::PublishEvent(g_EVENT_REQU_FIRST, ++testCaseCode, data);
        EXPECT_EQ(0, STAbilityUtil::WaitCompleted(event, g_EVENT_RESP_FIRST, testCaseCode, DELAY));
        data = STAbilityUtil::GetData(event, g_EVENT_RESP_FIRST, testCaseCode);
        result = data.compare("1") == 0;
        EXPECT_TRUE(result);
        if (!result && i > 0) {
            GTEST_LOG_(INFO) << "TaskDispatcher_Parallel_02900 : " << i;
            break;
        }
    }
}

/**
 * @tc.number    : TaskDispatcher_Parallel_03000
 * @tc.name      : parallel delay:parallel group
 * @tc.desc      : two levels task dispatch
 */
HWTEST_F(TaskDispatcherTest, TaskDispatcher_Parallel_03000, Function | MediumTest | Level1)
{
    StartAbility(firstAbilityName, bundleNameFirst);
    std::string data = "Dispatcher_" + std::to_string((int)TestFunc::PARALLEL) + "_29";
    bool result = false;
    for (int i = 0; i < stLevel_.AMSLevel; i++) {
        STAbilityUtil::PublishEvent(g_EVENT_REQU_FIRST, ++testCaseCode, data);
        EXPECT_EQ(0, STAbilityUtil::WaitCompleted(event, g_EVENT_RESP_FIRST, testCaseCode, DELAY));
        data = STAbilityUtil::GetData(event, g_EVENT_RESP_FIRST, testCaseCode);
        result = data.compare("1") == 0;
        EXPECT_TRUE(result);
        if (!result && i > 0) {
            GTEST_LOG_(INFO) << "TaskDispatcher_Parallel_03000 : " << i;
            break;
        }
    }
}

/**
 * @tc.number    : TaskDispatcher_Parallel_03100
 * @tc.name      : parallel delay:parallel group wait
 * @tc.desc      : two levels task dispatch
 */
HWTEST_F(TaskDispatcherTest, TaskDispatcher_Parallel_03100, Function | MediumTest | Level1)
{
    StartAbility(firstAbilityName, bundleNameFirst);
    std::string data = "Dispatcher_" + std::to_string((int)TestFunc::PARALLEL) + "_30";
    bool result = false;
    for (int i = 0; i < stLevel_.AMSLevel; i++) {
        STAbilityUtil::PublishEvent(g_EVENT_REQU_FIRST, ++testCaseCode, data);
        EXPECT_EQ(0, STAbilityUtil::WaitCompleted(event, g_EVENT_RESP_FIRST, testCaseCode, DELAY));
        data = STAbilityUtil::GetData(event, g_EVENT_RESP_FIRST, testCaseCode);
        result = data.compare("1") == 0;
        EXPECT_TRUE(result);
        if (!result && i > 0) {
            GTEST_LOG_(INFO) << "TaskDispatcher_Parallel_03100 : " << i;
            break;
        }
    }
}

/**
 * @tc.number    : TaskDispatcher_Parallel_03200
 * @tc.name      : parallel delay:parallel group notify
 * @tc.desc      : two levels task dispatch
 */
HWTEST_F(TaskDispatcherTest, TaskDispatcher_Parallel_03200, Function | MediumTest | Level1)
{
    StartAbility(firstAbilityName, bundleNameFirst);
    std::string data = "Dispatcher_" + std::to_string((int)TestFunc::PARALLEL) + "_31";
    bool result = false;
    for (int i = 0; i < stLevel_.AMSLevel; i++) {
        STAbilityUtil::PublishEvent(g_EVENT_REQU_FIRST, ++testCaseCode, data);
        EXPECT_EQ(0, STAbilityUtil::WaitCompleted(event, g_EVENT_RESP_FIRST, testCaseCode, DELAY));
        data = STAbilityUtil::GetData(event, g_EVENT_RESP_FIRST, testCaseCode);
        result = data.compare("1") == 0;
        EXPECT_TRUE(result);
        if (!result && i > 0) {
            GTEST_LOG_(INFO) << "TaskDispatcher_Parallel_03200 : " << i;
            break;
        }
    }
}

/**
 * @tc.number    : TaskDispatcher_Parallel_03300
 * @tc.name      : parallel delay:parallel sync barrier
 * @tc.desc      : two levels task dispatch
 */
HWTEST_F(TaskDispatcherTest, TaskDispatcher_Parallel_03300, Function | MediumTest | Level1)
{
    StartAbility(firstAbilityName, bundleNameFirst);
    std::string data = "Dispatcher_" + std::to_string((int)TestFunc::PARALLEL) + "_32";
    bool result = false;
    for (int i = 0; i < stLevel_.AMSLevel; i++) {
        STAbilityUtil::PublishEvent(g_EVENT_REQU_FIRST, ++testCaseCode, data);
        EXPECT_EQ(0, STAbilityUtil::WaitCompleted(event, g_EVENT_RESP_FIRST, testCaseCode, DELAY));
        data = STAbilityUtil::GetData(event, g_EVENT_RESP_FIRST, testCaseCode);
        result = data.compare("1") == 0;
        EXPECT_TRUE(result);
        if (!result && i > 0) {
            GTEST_LOG_(INFO) << "TaskDispatcher_Parallel_03300 : " << i;
            break;
        }
    }
}

/**
 * @tc.number    : TaskDispatcher_Parallel_03400
 * @tc.name      : parallel delay:parallel async barrier
 * @tc.desc      : two levels task dispatch
 */
HWTEST_F(TaskDispatcherTest, TaskDispatcher_Parallel_03400, Function | MediumTest | Level1)
{
    StartAbility(firstAbilityName, bundleNameFirst);
    std::string data = "Dispatcher_" + std::to_string((int)TestFunc::PARALLEL) + "_33";
    bool result = false;
    for (int i = 0; i < stLevel_.AMSLevel; i++) {
        STAbilityUtil::PublishEvent(g_EVENT_REQU_FIRST, ++testCaseCode, data);
        EXPECT_EQ(0, STAbilityUtil::WaitCompleted(event, g_EVENT_RESP_FIRST, testCaseCode, DELAY));
        data = STAbilityUtil::GetData(event, g_EVENT_RESP_FIRST, testCaseCode);
        result = data.compare("1") == 0;
        EXPECT_TRUE(result);
        if (!result && i > 0) {
            GTEST_LOG_(INFO) << "TaskDispatcher_Parallel_03400 : " << i;
            break;
        }
    }
}

/**
 * @tc.number    : TaskDispatcher_Parallel_03500
 * @tc.name      : parallel delay:parallel apply
 * @tc.desc      : two levels task dispatch
 */
HWTEST_F(TaskDispatcherTest, TaskDispatcher_Parallel_03500, Function | MediumTest | Level1)
{
    StartAbility(firstAbilityName, bundleNameFirst);
    std::string data = "Dispatcher_" + std::to_string((int)TestFunc::PARALLEL) + "_34";
    bool result = false;
    for (int i = 0; i < stLevel_.AMSLevel; i++) {
        STAbilityUtil::PublishEvent(g_EVENT_REQU_FIRST, ++testCaseCode, data);
        EXPECT_EQ(0, STAbilityUtil::WaitCompleted(event, g_EVENT_RESP_FIRST, testCaseCode, DELAY));
        data = STAbilityUtil::GetData(event, g_EVENT_RESP_FIRST, testCaseCode);
        result = data.compare("1") == 0;
        EXPECT_TRUE(result);
        if (!result && i > 0) {
            GTEST_LOG_(INFO) << "TaskDispatcher_Parallel_03500 : " << i;
            break;
        }
    }
}

/**
 * @tc.number    : TaskDispatcher_Parallel_03600
 * @tc.name      : parallel delay:serial sync
 * @tc.desc      : two levels task dispatch
 */
HWTEST_F(TaskDispatcherTest, TaskDispatcher_Parallel_03600, Function | MediumTest | Level1)
{
    StartAbility(firstAbilityName, bundleNameFirst);
    std::string data = "Dispatcher_" + std::to_string((int)TestFunc::PARALLEL) + "_35";
    bool result = false;
    for (int i = 0; i < stLevel_.AMSLevel; i++) {
        STAbilityUtil::PublishEvent(g_EVENT_REQU_FIRST, ++testCaseCode, data);
        EXPECT_EQ(0, STAbilityUtil::WaitCompleted(event, g_EVENT_RESP_FIRST, testCaseCode, DELAY));
        data = STAbilityUtil::GetData(event, g_EVENT_RESP_FIRST, testCaseCode);
        result = data.compare("1") == 0;
        EXPECT_TRUE(result);
        if (!result && i > 0) {
            GTEST_LOG_(INFO) << "TaskDispatcher_Parallel_03600 : " << i;
            break;
        }
    }
}

/**
 * @tc.number    : TaskDispatcher_Parallel_03700
 * @tc.name      : parallel delay:serial async
 * @tc.desc      : two levels task dispatch
 */
HWTEST_F(TaskDispatcherTest, TaskDispatcher_Parallel_03700, Function | MediumTest | Level1)
{
    StartAbility(firstAbilityName, bundleNameFirst);
    std::string data = "Dispatcher_" + std::to_string((int)TestFunc::PARALLEL) + "_36";
    bool result = false;
    for (int i = 0; i < stLevel_.AMSLevel; i++) {
        STAbilityUtil::PublishEvent(g_EVENT_REQU_FIRST, ++testCaseCode, data);
        EXPECT_EQ(0, STAbilityUtil::WaitCompleted(event, g_EVENT_RESP_FIRST, testCaseCode, DELAY));
        data = STAbilityUtil::GetData(event, g_EVENT_RESP_FIRST, testCaseCode);
        result = data.compare("1") == 0;
        EXPECT_TRUE(result);
        if (!result && i > 0) {
            GTEST_LOG_(INFO) << "TaskDispatcher_Parallel_03700 : " << i;
            break;
        }
    }
}

/**
 * @tc.number    : TaskDispatcher_Parallel_03800
 * @tc.name      : parallel delay:serial delay
 * @tc.desc      : two levels task dispatch
 */
HWTEST_F(TaskDispatcherTest, TaskDispatcher_Parallel_03800, Function | MediumTest | Level1)
{
    StartAbility(firstAbilityName, bundleNameFirst);
    std::string data = "Dispatcher_" + std::to_string((int)TestFunc::PARALLEL) + "_37";
    bool result = false;
    for (int i = 0; i < stLevel_.AMSLevel; i++) {
        STAbilityUtil::PublishEvent(g_EVENT_REQU_FIRST, ++testCaseCode, data);
        EXPECT_EQ(0, STAbilityUtil::WaitCompleted(event, g_EVENT_RESP_FIRST, testCaseCode, DELAY));
        data = STAbilityUtil::GetData(event, g_EVENT_RESP_FIRST, testCaseCode);
        result = data.compare("1") == 0;
        EXPECT_TRUE(result);
        if (!result && i > 0) {
            GTEST_LOG_(INFO) << "TaskDispatcher_Parallel_03800 : " << i;
            break;
        }
    }
}

/**
 * @tc.number    : TaskDispatcher_Parallel_03900
 * @tc.name      : parallel delay:serial apply
 * @tc.desc      : two levels task dispatch
 */
HWTEST_F(TaskDispatcherTest, TaskDispatcher_Parallel_03900, Function | MediumTest | Level1)
{
    StartAbility(firstAbilityName, bundleNameFirst);
    std::string data = "Dispatcher_" + std::to_string((int)TestFunc::PARALLEL) + "_38";
    bool result = false;
    for (int i = 0; i < stLevel_.AMSLevel; i++) {
        STAbilityUtil::PublishEvent(g_EVENT_REQU_FIRST, ++testCaseCode, data);
        EXPECT_EQ(0, STAbilityUtil::WaitCompleted(event, g_EVENT_RESP_FIRST, testCaseCode, DELAY));
        data = STAbilityUtil::GetData(event, g_EVENT_RESP_FIRST, testCaseCode);
        result = data.compare("1") == 0;
        EXPECT_TRUE(result);
        if (!result && i > 0) {
            GTEST_LOG_(INFO) << "TaskDispatcher_Parallel_03900 : " << i;
            break;
        }
    }
}

/**
 * @tc.number    : TaskDispatcher_Parallel_04000
 * @tc.name      : parallel group:parallel sync
 * @tc.desc      : two levels task dispatch
 */
HWTEST_F(TaskDispatcherTest, TaskDispatcher_Parallel_04000, Function | MediumTest | Level1)
{
    StartAbility(firstAbilityName, bundleNameFirst);
    std::string data = "Dispatcher_" + std::to_string((int)TestFunc::PARALLEL) + "_39";
    bool result = false;
    for (int i = 0; i < stLevel_.AMSLevel; i++) {
        STAbilityUtil::PublishEvent(g_EVENT_REQU_FIRST, ++testCaseCode, data);
        EXPECT_EQ(0, STAbilityUtil::WaitCompleted(event, g_EVENT_RESP_FIRST, testCaseCode, DELAY));
        data = STAbilityUtil::GetData(event, g_EVENT_RESP_FIRST, testCaseCode);
        result = data.compare("1") == 0;
        EXPECT_TRUE(result);
        if (!result && i > 0) {
            GTEST_LOG_(INFO) << "TaskDispatcher_Parallel_04000 : " << i;
            break;
        }
    }
}

/**
 * @tc.number    : TaskDispatcher_Parallel_04100
 * @tc.name      : parallel group:parallel async
 * @tc.desc      : two levels task dispatch
 */
HWTEST_F(TaskDispatcherTest, TaskDispatcher_Parallel_04100, Function | MediumTest | Level1)
{
    StartAbility(firstAbilityName, bundleNameFirst);
    std::string data = "Dispatcher_" + std::to_string((int)TestFunc::PARALLEL) + "_40";
    bool result = false;
    for (int i = 0; i < stLevel_.AMSLevel; i++) {
        STAbilityUtil::PublishEvent(g_EVENT_REQU_FIRST, ++testCaseCode, data);
        EXPECT_EQ(0, STAbilityUtil::WaitCompleted(event, g_EVENT_RESP_FIRST, testCaseCode, DELAY));
        data = STAbilityUtil::GetData(event, g_EVENT_RESP_FIRST, testCaseCode);
        result = data.compare("1") == 0;
        EXPECT_TRUE(result);
        if (!result && i > 0) {
            GTEST_LOG_(INFO) << "TaskDispatcher_Parallel_04100 : " << i;
            break;
        }
    }
}

/**
 * @tc.number    : TaskDispatcher_Parallel_04200
 * @tc.name      : parallel group:parallel delay
 * @tc.desc      : two levels task dispatch
 */
HWTEST_F(TaskDispatcherTest, TaskDispatcher_Parallel_04200, Function | MediumTest | Level1)
{
    StartAbility(firstAbilityName, bundleNameFirst);
    std::string data = "Dispatcher_" + std::to_string((int)TestFunc::PARALLEL) + "_41";
    bool result = false;
    for (int i = 0; i < stLevel_.AMSLevel; i++) {
        STAbilityUtil::PublishEvent(g_EVENT_REQU_FIRST, ++testCaseCode, data);
        EXPECT_EQ(0, STAbilityUtil::WaitCompleted(event, g_EVENT_RESP_FIRST, testCaseCode, DELAY));
        data = STAbilityUtil::GetData(event, g_EVENT_RESP_FIRST, testCaseCode);
        result = data.compare("1") == 0;
        EXPECT_TRUE(result);
        if (!result && i > 0) {
            GTEST_LOG_(INFO) << "TaskDispatcher_Parallel_04200 : " << i;
            break;
        }
    }
}

/**
 * @tc.number    : TaskDispatcher_Parallel_04300
 * @tc.name      : parallel group:parallel group
 * @tc.desc      : two levels task dispatch
 */
HWTEST_F(TaskDispatcherTest, TaskDispatcher_Parallel_04300, Function | MediumTest | Level1)
{
    StartAbility(firstAbilityName, bundleNameFirst);
    std::string data = "Dispatcher_" + std::to_string((int)TestFunc::PARALLEL) + "_42";
    bool result = false;
    for (int i = 0; i < stLevel_.AMSLevel; i++) {
        STAbilityUtil::PublishEvent(g_EVENT_REQU_FIRST, ++testCaseCode, data);
        EXPECT_EQ(0, STAbilityUtil::WaitCompleted(event, g_EVENT_RESP_FIRST, testCaseCode, DELAY));
        data = STAbilityUtil::GetData(event, g_EVENT_RESP_FIRST, testCaseCode);
        result = data.compare("1") == 0;
        EXPECT_TRUE(result);
        if (!result && i > 0) {
            GTEST_LOG_(INFO) << "TaskDispatcher_Parallel_04300 : " << i;
            break;
        }
    }
}

/**
 * @tc.number    : TaskDispatcher_Parallel_04400
 * @tc.name      : parallel group:parallel group wait
 * @tc.desc      : two levels task dispatch
 */
HWTEST_F(TaskDispatcherTest, TaskDispatcher_Parallel_04400, Function | MediumTest | Level1)
{
    StartAbility(firstAbilityName, bundleNameFirst);
    std::string data = "Dispatcher_" + std::to_string((int)TestFunc::PARALLEL) + "_43";
    bool result = false;
    for (int i = 0; i < stLevel_.AMSLevel; i++) {
        STAbilityUtil::PublishEvent(g_EVENT_REQU_FIRST, ++testCaseCode, data);
        EXPECT_EQ(0, STAbilityUtil::WaitCompleted(event, g_EVENT_RESP_FIRST, testCaseCode, DELAY));
        data = STAbilityUtil::GetData(event, g_EVENT_RESP_FIRST, testCaseCode);
        result = data.compare("1") == 0;
        EXPECT_TRUE(result);
        if (!result && i > 0) {
            GTEST_LOG_(INFO) << "TaskDispatcher_Parallel_04400 : " << i;
            break;
        }
    }
}

/**
 * @tc.number    : TaskDispatcher_Parallel_04500
 * @tc.name      : parallel group:parallel group notify
 * @tc.desc      : two levels task dispatch
 */
HWTEST_F(TaskDispatcherTest, TaskDispatcher_Parallel_04500, Function | MediumTest | Level1)
{
    StartAbility(firstAbilityName, bundleNameFirst);
    std::string data = "Dispatcher_" + std::to_string((int)TestFunc::PARALLEL) + "_44";
    bool result = false;
    for (int i = 0; i < stLevel_.AMSLevel; i++) {
        STAbilityUtil::PublishEvent(g_EVENT_REQU_FIRST, ++testCaseCode, data);
        EXPECT_EQ(0, STAbilityUtil::WaitCompleted(event, g_EVENT_RESP_FIRST, testCaseCode, DELAY));
        data = STAbilityUtil::GetData(event, g_EVENT_RESP_FIRST, testCaseCode);
        result = data.compare("1") == 0;
        EXPECT_TRUE(result);
        if (!result && i > 0) {
            GTEST_LOG_(INFO) << "TaskDispatcher_Parallel_04500 : " << i;
            break;
        }
    }
}

/**
 * @tc.number    : TaskDispatcher_Parallel_04600
 * @tc.name      : parallel group:parallel sync barrier
 * @tc.desc      : two levels task dispatch
 */
HWTEST_F(TaskDispatcherTest, TaskDispatcher_Parallel_04600, Function | MediumTest | Level1)
{
    StartAbility(firstAbilityName, bundleNameFirst);
    std::string data = "Dispatcher_" + std::to_string((int)TestFunc::PARALLEL) + "_45";
    bool result = false;
    for (int i = 0; i < stLevel_.AMSLevel; i++) {
        STAbilityUtil::PublishEvent(g_EVENT_REQU_FIRST, ++testCaseCode, data);
        EXPECT_EQ(0, STAbilityUtil::WaitCompleted(event, g_EVENT_RESP_FIRST, testCaseCode, DELAY));
        data = STAbilityUtil::GetData(event, g_EVENT_RESP_FIRST, testCaseCode);
        result = data.compare("1") == 0;
        EXPECT_TRUE(result);
        if (!result && i > 0) {
            GTEST_LOG_(INFO) << "TaskDispatcher_Parallel_04600 : " << i;
            break;
        }
    }
}

/**
 * @tc.number    : TaskDispatcher_Parallel_04700
 * @tc.name      : parallel group:parallel async barrier
 * @tc.desc      : two levels task dispatch
 */
HWTEST_F(TaskDispatcherTest, TaskDispatcher_Parallel_04700, Function | MediumTest | Level1)
{
    StartAbility(firstAbilityName, bundleNameFirst);
    std::string data = "Dispatcher_" + std::to_string((int)TestFunc::PARALLEL) + "_46";
    bool result = false;
    for (int i = 0; i < stLevel_.AMSLevel; i++) {
        STAbilityUtil::PublishEvent(g_EVENT_REQU_FIRST, ++testCaseCode, data);
        EXPECT_EQ(0, STAbilityUtil::WaitCompleted(event, g_EVENT_RESP_FIRST, testCaseCode, DELAY));
        data = STAbilityUtil::GetData(event, g_EVENT_RESP_FIRST, testCaseCode);
        result = data.compare("1") == 0;
        EXPECT_TRUE(result);
        if (!result && i > 0) {
            GTEST_LOG_(INFO) << "TaskDispatcher_Parallel_04700 : " << i;
            break;
        }
    }
}

/**
 * @tc.number    : TaskDispatcher_Parallel_04800
 * @tc.name      : parallel group:parallel apply
 * @tc.desc      : two levels task dispatch
 */
HWTEST_F(TaskDispatcherTest, TaskDispatcher_Parallel_04800, Function | MediumTest | Level1)
{
    StartAbility(firstAbilityName, bundleNameFirst);
    std::string data = "Dispatcher_" + std::to_string((int)TestFunc::PARALLEL) + "_47";
    bool result = false;
    for (int i = 0; i < stLevel_.AMSLevel; i++) {
        STAbilityUtil::PublishEvent(g_EVENT_REQU_FIRST, ++testCaseCode, data);
        EXPECT_EQ(0, STAbilityUtil::WaitCompleted(event, g_EVENT_RESP_FIRST, testCaseCode, DELAY));
        data = STAbilityUtil::GetData(event, g_EVENT_RESP_FIRST, testCaseCode);
        result = data.compare("1") == 0;
        EXPECT_TRUE(result);
        if (!result && i > 0) {
            GTEST_LOG_(INFO) << "TaskDispatcher_Parallel_04800 : " << i;
            break;
        }
    }
}

/**
 * @tc.number    : TaskDispatcher_Parallel_04900
 * @tc.name      : parallel group:serial sync
 * @tc.desc      : two levels task dispatch
 */
HWTEST_F(TaskDispatcherTest, TaskDispatcher_Parallel_04900, Function | MediumTest | Level1)
{
    StartAbility(firstAbilityName, bundleNameFirst);
    std::string data = "Dispatcher_" + std::to_string((int)TestFunc::PARALLEL) + "_48";
    bool result = false;
    for (int i = 0; i < stLevel_.AMSLevel; i++) {
        STAbilityUtil::PublishEvent(g_EVENT_REQU_FIRST, ++testCaseCode, data);
        EXPECT_EQ(0, STAbilityUtil::WaitCompleted(event, g_EVENT_RESP_FIRST, testCaseCode, DELAY));
        data = STAbilityUtil::GetData(event, g_EVENT_RESP_FIRST, testCaseCode);
        result = data.compare("1") == 0;
        EXPECT_TRUE(result);
        if (!result && i > 0) {
            GTEST_LOG_(INFO) << "TaskDispatcher_Parallel_04900 : " << i;
            break;
        }
    }
}

/**
 * @tc.number    : TaskDispatcher_Parallel_05000
 * @tc.name      : parallel group:serial async
 * @tc.desc      : two levels task dispatch
 */
HWTEST_F(TaskDispatcherTest, TaskDispatcher_Parallel_05000, Function | MediumTest | Level1)
{
    StartAbility(firstAbilityName, bundleNameFirst);
    std::string data = "Dispatcher_" + std::to_string((int)TestFunc::PARALLEL) + "_49";
    bool result = false;
    for (int i = 0; i < stLevel_.AMSLevel; i++) {
        STAbilityUtil::PublishEvent(g_EVENT_REQU_FIRST, ++testCaseCode, data);
        EXPECT_EQ(0, STAbilityUtil::WaitCompleted(event, g_EVENT_RESP_FIRST, testCaseCode, DELAY));
        data = STAbilityUtil::GetData(event, g_EVENT_RESP_FIRST, testCaseCode);
        result = data.compare("1") == 0;
        EXPECT_TRUE(result);
        if (!result && i > 0) {
            GTEST_LOG_(INFO) << "TaskDispatcher_Parallel_05000 : " << i;
            break;
        }
    }
}

/**
 * @tc.number    : TaskDispatcher_Parallel_05100
 * @tc.name      : parallel group:serial delay
 * @tc.desc      : two levels task dispatch
 */
HWTEST_F(TaskDispatcherTest, TaskDispatcher_Parallel_05100, Function | MediumTest | Level1)
{
    StartAbility(firstAbilityName, bundleNameFirst);
    std::string data = "Dispatcher_" + std::to_string((int)TestFunc::PARALLEL) + "_50";
    bool result = false;
    for (int i = 0; i < stLevel_.AMSLevel; i++) {
        STAbilityUtil::PublishEvent(g_EVENT_REQU_FIRST, ++testCaseCode, data);
        EXPECT_EQ(0, STAbilityUtil::WaitCompleted(event, g_EVENT_RESP_FIRST, testCaseCode, DELAY));
        data = STAbilityUtil::GetData(event, g_EVENT_RESP_FIRST, testCaseCode);
        result = data.compare("1") == 0;
        EXPECT_TRUE(result);
        if (!result && i > 0) {
            GTEST_LOG_(INFO) << "TaskDispatcher_Parallel_05100 : " << i;
            break;
        }
    }
}

/**
 * @tc.number    : TaskDispatcher_Parallel_05200
 * @tc.name      : parallel group:serial apply
 * @tc.desc      : two levels task dispatch
 */
HWTEST_F(TaskDispatcherTest, TaskDispatcher_Parallel_05200, Function | MediumTest | Level1)
{
    StartAbility(firstAbilityName, bundleNameFirst);
    std::string data = "Dispatcher_" + std::to_string((int)TestFunc::PARALLEL) + "_51";
    bool result = false;
    for (int i = 0; i < stLevel_.AMSLevel; i++) {
        STAbilityUtil::PublishEvent(g_EVENT_REQU_FIRST, ++testCaseCode, data);
        EXPECT_EQ(0, STAbilityUtil::WaitCompleted(event, g_EVENT_RESP_FIRST, testCaseCode, DELAY));
        data = STAbilityUtil::GetData(event, g_EVENT_RESP_FIRST, testCaseCode);
        result = data.compare("1") == 0;
        EXPECT_TRUE(result);
        if (!result && i > 0) {
            GTEST_LOG_(INFO) << "TaskDispatcher_Parallel_05200 : " << i;
            break;
        }
    }
}

/**
 * @tc.number    : TaskDispatcher_Parallel_05300
 * @tc.name      : parallel group wait:parallel sync
 * @tc.desc      : two levels task dispatch
 */
HWTEST_F(TaskDispatcherTest, TaskDispatcher_Parallel_05300, Function | MediumTest | Level1)
{
    StartAbility(firstAbilityName, bundleNameFirst);
    std::string data = "Dispatcher_" + std::to_string((int)TestFunc::PARALLEL) + "_52";
    bool result = false;
    for (int i = 0; i < stLevel_.AMSLevel; i++) {
        STAbilityUtil::PublishEvent(g_EVENT_REQU_FIRST, ++testCaseCode, data);
        EXPECT_EQ(0, STAbilityUtil::WaitCompleted(event, g_EVENT_RESP_FIRST, testCaseCode, DELAY));
        data = STAbilityUtil::GetData(event, g_EVENT_RESP_FIRST, testCaseCode);
        result = data.compare("1") == 0;
        EXPECT_TRUE(result);
        if (!result && i > 0) {
            GTEST_LOG_(INFO) << "TaskDispatcher_Parallel_05300 : " << i;
            break;
        }
    }
}

/**
 * @tc.number    : TaskDispatcher_Parallel_05400
 * @tc.name      : parallel group wait:parallel async
 * @tc.desc      : two levels task dispatch
 */
HWTEST_F(TaskDispatcherTest, TaskDispatcher_Parallel_05400, Function | MediumTest | Level1)
{
    StartAbility(firstAbilityName, bundleNameFirst);
    std::string data = "Dispatcher_" + std::to_string((int)TestFunc::PARALLEL) + "_53";
    bool result = false;
    for (int i = 0; i < stLevel_.AMSLevel; i++) {
        STAbilityUtil::PublishEvent(g_EVENT_REQU_FIRST, ++testCaseCode, data);
        EXPECT_EQ(0, STAbilityUtil::WaitCompleted(event, g_EVENT_RESP_FIRST, testCaseCode, DELAY));
        data = STAbilityUtil::GetData(event, g_EVENT_RESP_FIRST, testCaseCode);
        result = data.compare("1") == 0;
        EXPECT_TRUE(result);
        if (!result && i > 0) {
            GTEST_LOG_(INFO) << "TaskDispatcher_Parallel_05400 : " << i;
            break;
        }
    }
}

/**
 * @tc.number    : TaskDispatcher_Parallel_05500
 * @tc.name      : parallel group wait:parallel delay
 * @tc.desc      : two levels task dispatch
 */
HWTEST_F(TaskDispatcherTest, TaskDispatcher_Parallel_05500, Function | MediumTest | Level1)
{
    StartAbility(firstAbilityName, bundleNameFirst);
    std::string data = "Dispatcher_" + std::to_string((int)TestFunc::PARALLEL) + "_54";
    bool result = false;
    for (int i = 0; i < stLevel_.AMSLevel; i++) {
        STAbilityUtil::PublishEvent(g_EVENT_REQU_FIRST, ++testCaseCode, data);
        EXPECT_EQ(0, STAbilityUtil::WaitCompleted(event, g_EVENT_RESP_FIRST, testCaseCode, DELAY));
        data = STAbilityUtil::GetData(event, g_EVENT_RESP_FIRST, testCaseCode);
        result = data.compare("1") == 0;
        EXPECT_TRUE(result);
        if (!result && i > 0) {
            GTEST_LOG_(INFO) << "TaskDispatcher_Parallel_05500 : " << i;
            break;
        }
    }
}

/**
 * @tc.number    : TaskDispatcher_Parallel_05600
 * @tc.name      : parallel group wait:parallel group
 * @tc.desc      : two levels task dispatch
 */
HWTEST_F(TaskDispatcherTest, TaskDispatcher_Parallel_05600, Function | MediumTest | Level1)
{
    StartAbility(firstAbilityName, bundleNameFirst);
    std::string data = "Dispatcher_" + std::to_string((int)TestFunc::PARALLEL) + "_55";
    bool result = false;
    for (int i = 0; i < stLevel_.AMSLevel; i++) {
        STAbilityUtil::PublishEvent(g_EVENT_REQU_FIRST, ++testCaseCode, data);
        EXPECT_EQ(0, STAbilityUtil::WaitCompleted(event, g_EVENT_RESP_FIRST, testCaseCode, DELAY));
        data = STAbilityUtil::GetData(event, g_EVENT_RESP_FIRST, testCaseCode);
        result = data.compare("1") == 0;
        EXPECT_TRUE(result);
        if (!result && i > 0) {
            GTEST_LOG_(INFO) << "TaskDispatcher_Parallel_05600 : " << i;
            break;
        }
    }
}

/**
 * @tc.number    : TaskDispatcher_Parallel_05700
 * @tc.name      : parallel group wait:parallel group wait
 * @tc.desc      : two levels task dispatch
 */
HWTEST_F(TaskDispatcherTest, TaskDispatcher_Parallel_05700, Function | MediumTest | Level1)
{
    StartAbility(firstAbilityName, bundleNameFirst);
    std::string data = "Dispatcher_" + std::to_string((int)TestFunc::PARALLEL) + "_56";
    bool result = false;
    for (int i = 0; i < stLevel_.AMSLevel; i++) {
        STAbilityUtil::PublishEvent(g_EVENT_REQU_FIRST, ++testCaseCode, data);
        EXPECT_EQ(0, STAbilityUtil::WaitCompleted(event, g_EVENT_RESP_FIRST, testCaseCode, DELAY));
        data = STAbilityUtil::GetData(event, g_EVENT_RESP_FIRST, testCaseCode);
        result = data.compare("1") == 0;
        EXPECT_TRUE(result);
        if (!result && i > 0) {
            GTEST_LOG_(INFO) << "TaskDispatcher_Parallel_05700 : " << i;
            break;
        }
    }
}

/**
 * @tc.number    : TaskDispatcher_Parallel_05800
 * @tc.name      : parallel group wait:parallel group notify
 * @tc.desc      : two levels task dispatch
 */
HWTEST_F(TaskDispatcherTest, TaskDispatcher_Parallel_05800, Function | MediumTest | Level1)
{
    StartAbility(firstAbilityName, bundleNameFirst);
    std::string data = "Dispatcher_" + std::to_string((int)TestFunc::PARALLEL) + "_57";
    bool result = false;
    for (int i = 0; i < stLevel_.AMSLevel; i++) {
        STAbilityUtil::PublishEvent(g_EVENT_REQU_FIRST, ++testCaseCode, data);
        EXPECT_EQ(0, STAbilityUtil::WaitCompleted(event, g_EVENT_RESP_FIRST, testCaseCode, DELAY));
        data = STAbilityUtil::GetData(event, g_EVENT_RESP_FIRST, testCaseCode);
        result = data.compare("1") == 0;
        EXPECT_TRUE(result);
        if (!result && i > 0) {
            GTEST_LOG_(INFO) << "TaskDispatcher_Parallel_05800 : " << i;
            break;
        }
    }
}

/**
 * @tc.number    : TaskDispatcher_Parallel_05900
 * @tc.name      : parallel group wait:parallel sync barrier
 * @tc.desc      : two levels task dispatch
 */
HWTEST_F(TaskDispatcherTest, TaskDispatcher_Parallel_05900, Function | MediumTest | Level1)
{
    StartAbility(firstAbilityName, bundleNameFirst);
    std::string data = "Dispatcher_" + std::to_string((int)TestFunc::PARALLEL) + "_58";
    bool result = false;
    for (int i = 0; i < stLevel_.AMSLevel; i++) {
        STAbilityUtil::PublishEvent(g_EVENT_REQU_FIRST, ++testCaseCode, data);
        EXPECT_EQ(0, STAbilityUtil::WaitCompleted(event, g_EVENT_RESP_FIRST, testCaseCode, DELAY));
        data = STAbilityUtil::GetData(event, g_EVENT_RESP_FIRST, testCaseCode);
        result = data.compare("1") == 0;
        EXPECT_TRUE(result);
        if (!result && i > 0) {
            GTEST_LOG_(INFO) << "TaskDispatcher_Parallel_05900 : " << i;
            break;
        }
    }
}

/**
 * @tc.number    : TaskDispatcher_Parallel_06000
 * @tc.name      : parallel group wait:parallel async barrier
 * @tc.desc      : two levels task dispatch
 */
HWTEST_F(TaskDispatcherTest, TaskDispatcher_Parallel_06000, Function | MediumTest | Level1)
{
    StartAbility(firstAbilityName, bundleNameFirst);
    std::string data = "Dispatcher_" + std::to_string((int)TestFunc::PARALLEL) + "_59";
    bool result = false;
    for (int i = 0; i < stLevel_.AMSLevel; i++) {
        STAbilityUtil::PublishEvent(g_EVENT_REQU_FIRST, ++testCaseCode, data);
        EXPECT_EQ(0, STAbilityUtil::WaitCompleted(event, g_EVENT_RESP_FIRST, testCaseCode, DELAY));
        data = STAbilityUtil::GetData(event, g_EVENT_RESP_FIRST, testCaseCode);
        result = data.compare("1") == 0;
        EXPECT_TRUE(result);
        if (!result && i > 0) {
            GTEST_LOG_(INFO) << "TaskDispatcher_Parallel_06000 : " << i;
            break;
        }
    }
}

/**
 * @tc.number    : TaskDispatcher_Parallel_06100
 * @tc.name      : parallel group wait:parallel apply
 * @tc.desc      : two levels task dispatch
 */
HWTEST_F(TaskDispatcherTest, TaskDispatcher_Parallel_06100, Function | MediumTest | Level1)
{
    StartAbility(firstAbilityName, bundleNameFirst);
    std::string data = "Dispatcher_" + std::to_string((int)TestFunc::PARALLEL) + "_60";
    bool result = false;
    for (int i = 0; i < stLevel_.AMSLevel; i++) {
        STAbilityUtil::PublishEvent(g_EVENT_REQU_FIRST, ++testCaseCode, data);
        EXPECT_EQ(0, STAbilityUtil::WaitCompleted(event, g_EVENT_RESP_FIRST, testCaseCode, DELAY));
        data = STAbilityUtil::GetData(event, g_EVENT_RESP_FIRST, testCaseCode);
        result = data.compare("1") == 0;
        EXPECT_TRUE(result);
        if (!result && i > 0) {
            GTEST_LOG_(INFO) << "TaskDispatcher_Parallel_06100 : " << i;
            break;
        }
    }
}

/**
 * @tc.number    : TaskDispatcher_Parallel_06200
 * @tc.name      : parallel group wait:serial sync
 * @tc.desc      : two levels task dispatch
 */
HWTEST_F(TaskDispatcherTest, TaskDispatcher_Parallel_06200, Function | MediumTest | Level1)
{
    StartAbility(firstAbilityName, bundleNameFirst);
    std::string data = "Dispatcher_" + std::to_string((int)TestFunc::PARALLEL) + "_61";
    bool result = false;
    for (int i = 0; i < stLevel_.AMSLevel; i++) {
        STAbilityUtil::PublishEvent(g_EVENT_REQU_FIRST, ++testCaseCode, data);
        EXPECT_EQ(0, STAbilityUtil::WaitCompleted(event, g_EVENT_RESP_FIRST, testCaseCode, DELAY));
        data = STAbilityUtil::GetData(event, g_EVENT_RESP_FIRST, testCaseCode);
        result = data.compare("1") == 0;
        EXPECT_TRUE(result);
        if (!result && i > 0) {
            GTEST_LOG_(INFO) << "TaskDispatcher_Parallel_06200 : " << i;
            break;
        }
    }
}

/**
 * @tc.number    : TaskDispatcher_Parallel_06300
 * @tc.name      : parallel group wait:serial async
 * @tc.desc      : two levels task dispatch
 */
HWTEST_F(TaskDispatcherTest, TaskDispatcher_Parallel_06300, Function | MediumTest | Level1)
{
    StartAbility(firstAbilityName, bundleNameFirst);
    std::string data = "Dispatcher_" + std::to_string((int)TestFunc::PARALLEL) + "_62";
    bool result = false;
    for (int i = 0; i < stLevel_.AMSLevel; i++) {
        STAbilityUtil::PublishEvent(g_EVENT_REQU_FIRST, ++testCaseCode, data);
        EXPECT_EQ(0, STAbilityUtil::WaitCompleted(event, g_EVENT_RESP_FIRST, testCaseCode, DELAY));
        data = STAbilityUtil::GetData(event, g_EVENT_RESP_FIRST, testCaseCode);
        result = data.compare("1") == 0;
        EXPECT_TRUE(result);
        if (!result && i > 0) {
            GTEST_LOG_(INFO) << "TaskDispatcher_Parallel_06300 : " << i;
            break;
        }
    }
}

/**
 * @tc.number    : TaskDispatcher_Parallel_06400
 * @tc.name      : parallel group wait:serial delay
 * @tc.desc      : two levels task dispatch
 */
HWTEST_F(TaskDispatcherTest, TaskDispatcher_Parallel_06400, Function | MediumTest | Level1)
{
    StartAbility(firstAbilityName, bundleNameFirst);
    std::string data = "Dispatcher_" + std::to_string((int)TestFunc::PARALLEL) + "_63";
    bool result = false;
    for (int i = 0; i < stLevel_.AMSLevel; i++) {
        STAbilityUtil::PublishEvent(g_EVENT_REQU_FIRST, ++testCaseCode, data);
        EXPECT_EQ(0, STAbilityUtil::WaitCompleted(event, g_EVENT_RESP_FIRST, testCaseCode, DELAY));
        data = STAbilityUtil::GetData(event, g_EVENT_RESP_FIRST, testCaseCode);
        result = data.compare("1") == 0;
        EXPECT_TRUE(result);
        if (!result && i > 0) {
            GTEST_LOG_(INFO) << "TaskDispatcher_Parallel_06400 : " << i;
            break;
        }
    }
}

/**
 * @tc.number    : TaskDispatcher_Parallel_06500
 * @tc.name      : parallel group wait:serial apply
 * @tc.desc      : two levels task dispatch
 */
HWTEST_F(TaskDispatcherTest, TaskDispatcher_Parallel_06500, Function | MediumTest | Level1)
{
    StartAbility(firstAbilityName, bundleNameFirst);
    std::string data = "Dispatcher_" + std::to_string((int)TestFunc::PARALLEL) + "_64";
    bool result = false;
    for (int i = 0; i < stLevel_.AMSLevel; i++) {
        STAbilityUtil::PublishEvent(g_EVENT_REQU_FIRST, ++testCaseCode, data);
        EXPECT_EQ(0, STAbilityUtil::WaitCompleted(event, g_EVENT_RESP_FIRST, testCaseCode, DELAY));
        data = STAbilityUtil::GetData(event, g_EVENT_RESP_FIRST, testCaseCode);
        result = data.compare("1") == 0;
        EXPECT_TRUE(result);
        if (!result && i > 0) {
            GTEST_LOG_(INFO) << "TaskDispatcher_Parallel_06500 : " << i;
            break;
        }
    }
}

/**
 * @tc.number    : TaskDispatcher_Parallel_06600
 * @tc.name      : parallel group notify:parallel sync
 * @tc.desc      : two levels task dispatch
 */
HWTEST_F(TaskDispatcherTest, TaskDispatcher_Parallel_06600, Function | MediumTest | Level1)
{
    StartAbility(firstAbilityName, bundleNameFirst);
    std::string data = "Dispatcher_" + std::to_string((int)TestFunc::PARALLEL) + "_65";
    bool result = false;
    for (int i = 0; i < stLevel_.AMSLevel; i++) {
        STAbilityUtil::PublishEvent(g_EVENT_REQU_FIRST, ++testCaseCode, data);
        EXPECT_EQ(0, STAbilityUtil::WaitCompleted(event, g_EVENT_RESP_FIRST, testCaseCode, DELAY));
        data = STAbilityUtil::GetData(event, g_EVENT_RESP_FIRST, testCaseCode);
        result = data.compare("1") == 0;
        EXPECT_TRUE(result);
        if (!result && i > 0) {
            GTEST_LOG_(INFO) << "TaskDispatcher_Parallel_06600 : " << i;
            break;
        }
    }
}

/**
 * @tc.number    : TaskDispatcher_Parallel_06700
 * @tc.name      : parallel group notify:parallel async
 * @tc.desc      : two levels task dispatch
 */
HWTEST_F(TaskDispatcherTest, TaskDispatcher_Parallel_06700, Function | MediumTest | Level1)
{
    StartAbility(firstAbilityName, bundleNameFirst);
    std::string data = "Dispatcher_" + std::to_string((int)TestFunc::PARALLEL) + "_66";
    bool result = false;
    for (int i = 0; i < stLevel_.AMSLevel; i++) {
        STAbilityUtil::PublishEvent(g_EVENT_REQU_FIRST, ++testCaseCode, data);
        EXPECT_EQ(0, STAbilityUtil::WaitCompleted(event, g_EVENT_RESP_FIRST, testCaseCode, DELAY));
        data = STAbilityUtil::GetData(event, g_EVENT_RESP_FIRST, testCaseCode);
        result = data.compare("1") == 0;
        EXPECT_TRUE(result);
        if (!result && i > 0) {
            GTEST_LOG_(INFO) << "TaskDispatcher_Parallel_06700 : " << i;
            break;
        }
    }
}

/**
 * @tc.number    : TaskDispatcher_Parallel_06800
 * @tc.name      : parallel group notify:parallel delay
 * @tc.desc      : two levels task dispatch
 */
HWTEST_F(TaskDispatcherTest, TaskDispatcher_Parallel_06800, Function | MediumTest | Level1)
{
    StartAbility(firstAbilityName, bundleNameFirst);
    std::string data = "Dispatcher_" + std::to_string((int)TestFunc::PARALLEL) + "_67";
    bool result = false;
    for (int i = 0; i < stLevel_.AMSLevel; i++) {
        STAbilityUtil::PublishEvent(g_EVENT_REQU_FIRST, ++testCaseCode, data);
        EXPECT_EQ(0, STAbilityUtil::WaitCompleted(event, g_EVENT_RESP_FIRST, testCaseCode, DELAY));
        data = STAbilityUtil::GetData(event, g_EVENT_RESP_FIRST, testCaseCode);
        result = data.compare("1") == 0;
        EXPECT_TRUE(result);
        if (!result && i > 0) {
            GTEST_LOG_(INFO) << "TaskDispatcher_Parallel_06800 : " << i;
            break;
        }
    }
}

/**
 * @tc.number    : TaskDispatcher_Parallel_06900
 * @tc.name      : parallel group notify:parallel group
 * @tc.desc      : two levels task dispatch
 */
HWTEST_F(TaskDispatcherTest, TaskDispatcher_Parallel_06900, Function | MediumTest | Level1)
{
    StartAbility(firstAbilityName, bundleNameFirst);
    std::string data = "Dispatcher_" + std::to_string((int)TestFunc::PARALLEL) + "_68";
    bool result = false;
    for (int i = 0; i < stLevel_.AMSLevel; i++) {
        STAbilityUtil::PublishEvent(g_EVENT_REQU_FIRST, ++testCaseCode, data);
        EXPECT_EQ(0, STAbilityUtil::WaitCompleted(event, g_EVENT_RESP_FIRST, testCaseCode, DELAY));
        data = STAbilityUtil::GetData(event, g_EVENT_RESP_FIRST, testCaseCode);
        result = data.compare("1") == 0;
        EXPECT_TRUE(result);
        if (!result && i > 0) {
            GTEST_LOG_(INFO) << "TaskDispatcher_Parallel_06900 : " << i;
            break;
        }
    }
}

/**
 * @tc.number    : TaskDispatcher_Parallel_07000
 * @tc.name      : parallel group notify:parallel group wait
 * @tc.desc      : two levels task dispatch
 */
HWTEST_F(TaskDispatcherTest, TaskDispatcher_Parallel_07000, Function | MediumTest | Level1)
{
    StartAbility(firstAbilityName, bundleNameFirst);
    std::string data = "Dispatcher_" + std::to_string((int)TestFunc::PARALLEL) + "_69";
    bool result = false;
    for (int i = 0; i < stLevel_.AMSLevel; i++) {
        STAbilityUtil::PublishEvent(g_EVENT_REQU_FIRST, ++testCaseCode, data);
        EXPECT_EQ(0, STAbilityUtil::WaitCompleted(event, g_EVENT_RESP_FIRST, testCaseCode, DELAY));
        data = STAbilityUtil::GetData(event, g_EVENT_RESP_FIRST, testCaseCode);
        result = data.compare("1") == 0;
        EXPECT_TRUE(result);
        if (!result && i > 0) {
            GTEST_LOG_(INFO) << "TaskDispatcher_Parallel_07000 : " << i;
            break;
        }
    }
}

/**
 * @tc.number    : TaskDispatcher_Parallel_07100
 * @tc.name      : parallel group notify:parallel group notify
 * @tc.desc      : two levels task dispatch
 */
HWTEST_F(TaskDispatcherTest, TaskDispatcher_Parallel_07100, Function | MediumTest | Level1)
{
    StartAbility(firstAbilityName, bundleNameFirst);
    std::string data = "Dispatcher_" + std::to_string((int)TestFunc::PARALLEL) + "_70";
    bool result = false;
    for (int i = 0; i < stLevel_.AMSLevel; i++) {
        STAbilityUtil::PublishEvent(g_EVENT_REQU_FIRST, ++testCaseCode, data);
        EXPECT_EQ(0, STAbilityUtil::WaitCompleted(event, g_EVENT_RESP_FIRST, testCaseCode, DELAY));
        data = STAbilityUtil::GetData(event, g_EVENT_RESP_FIRST, testCaseCode);
        result = data.compare("1") == 0;
        EXPECT_TRUE(result);
        if (!result && i > 0) {
            GTEST_LOG_(INFO) << "TaskDispatcher_Parallel_07100 : " << i;
            break;
        }
    }
}

/**
 * @tc.number    : TaskDispatcher_Parallel_07200
 * @tc.name      : parallel group notify:parallel sync barrier
 * @tc.desc      : two levels task dispatch
 */
HWTEST_F(TaskDispatcherTest, TaskDispatcher_Parallel_07200, Function | MediumTest | Level1)
{
    StartAbility(firstAbilityName, bundleNameFirst);
    std::string data = "Dispatcher_" + std::to_string((int)TestFunc::PARALLEL) + "_71";
    bool result = false;
    for (int i = 0; i < stLevel_.AMSLevel; i++) {
        STAbilityUtil::PublishEvent(g_EVENT_REQU_FIRST, ++testCaseCode, data);
        EXPECT_EQ(0, STAbilityUtil::WaitCompleted(event, g_EVENT_RESP_FIRST, testCaseCode, DELAY));
        data = STAbilityUtil::GetData(event, g_EVENT_RESP_FIRST, testCaseCode);
        result = data.compare("1") == 0;
        EXPECT_TRUE(result);
        if (!result && i > 0) {
            GTEST_LOG_(INFO) << "TaskDispatcher_Parallel_07200 : " << i;
            break;
        }
    }
}

/**
 * @tc.number    : TaskDispatcher_Parallel_07300
 * @tc.name      : parallel group notify:parallel async barrier
 * @tc.desc      : two levels task dispatch
 */
HWTEST_F(TaskDispatcherTest, TaskDispatcher_Parallel_07300, Function | MediumTest | Level1)
{
    StartAbility(firstAbilityName, bundleNameFirst);
    std::string data = "Dispatcher_" + std::to_string((int)TestFunc::PARALLEL) + "_72";
    bool result = false;
    for (int i = 0; i < stLevel_.AMSLevel; i++) {
        STAbilityUtil::PublishEvent(g_EVENT_REQU_FIRST, ++testCaseCode, data);
        EXPECT_EQ(0, STAbilityUtil::WaitCompleted(event, g_EVENT_RESP_FIRST, testCaseCode, DELAY));
        data = STAbilityUtil::GetData(event, g_EVENT_RESP_FIRST, testCaseCode);
        result = data.compare("1") == 0;
        EXPECT_TRUE(result);
        if (!result && i > 0) {
            GTEST_LOG_(INFO) << "TaskDispatcher_Parallel_07300 : " << i;
            break;
        }
    }
}

/**
 * @tc.number    : TaskDispatcher_Parallel_07400
 * @tc.name      : parallel group notify:parallel apply
 * @tc.desc      : two levels task dispatch
 */
HWTEST_F(TaskDispatcherTest, TaskDispatcher_Parallel_07400, Function | MediumTest | Level1)
{
    StartAbility(firstAbilityName, bundleNameFirst);
    std::string data = "Dispatcher_" + std::to_string((int)TestFunc::PARALLEL) + "_73";
    bool result = false;
    for (int i = 0; i < stLevel_.AMSLevel; i++) {
        STAbilityUtil::PublishEvent(g_EVENT_REQU_FIRST, ++testCaseCode, data);
        EXPECT_EQ(0, STAbilityUtil::WaitCompleted(event, g_EVENT_RESP_FIRST, testCaseCode, DELAY));
        data = STAbilityUtil::GetData(event, g_EVENT_RESP_FIRST, testCaseCode);
        result = data.compare("1") == 0;
        EXPECT_TRUE(result);
        if (!result && i > 0) {
            GTEST_LOG_(INFO) << "TaskDispatcher_Parallel_07400 : " << i;
            break;
        }
    }
}

/**
 * @tc.number    : TaskDispatcher_Parallel_07500
 * @tc.name      : parallel group notify:serial sync
 * @tc.desc      : two levels task dispatch
 */
HWTEST_F(TaskDispatcherTest, TaskDispatcher_Parallel_07500, Function | MediumTest | Level1)
{
    StartAbility(firstAbilityName, bundleNameFirst);
    std::string data = "Dispatcher_" + std::to_string((int)TestFunc::PARALLEL) + "_74";
    bool result = false;
    for (int i = 0; i < stLevel_.AMSLevel; i++) {
        STAbilityUtil::PublishEvent(g_EVENT_REQU_FIRST, ++testCaseCode, data);
        EXPECT_EQ(0, STAbilityUtil::WaitCompleted(event, g_EVENT_RESP_FIRST, testCaseCode, DELAY));
        data = STAbilityUtil::GetData(event, g_EVENT_RESP_FIRST, testCaseCode);
        result = data.compare("1") == 0;
        EXPECT_TRUE(result);
        if (!result && i > 0) {
            GTEST_LOG_(INFO) << "TaskDispatcher_Parallel_07500 : " << i;
            break;
        }
    }
}

/**
 * @tc.number    : TaskDispatcher_Parallel_07600
 * @tc.name      : parallel group notify:serial async
 * @tc.desc      : two levels task dispatch
 */
HWTEST_F(TaskDispatcherTest, TaskDispatcher_Parallel_07600, Function | MediumTest | Level1)
{
    StartAbility(firstAbilityName, bundleNameFirst);
    std::string data = "Dispatcher_" + std::to_string((int)TestFunc::PARALLEL) + "_75";
    bool result = false;
    for (int i = 0; i < stLevel_.AMSLevel; i++) {
        STAbilityUtil::PublishEvent(g_EVENT_REQU_FIRST, ++testCaseCode, data);
        EXPECT_EQ(0, STAbilityUtil::WaitCompleted(event, g_EVENT_RESP_FIRST, testCaseCode, DELAY));
        data = STAbilityUtil::GetData(event, g_EVENT_RESP_FIRST, testCaseCode);
        result = data.compare("1") == 0;
        EXPECT_TRUE(result);
        if (!result && i > 0) {
            GTEST_LOG_(INFO) << "TaskDispatcher_Parallel_07600 : " << i;
            break;
        }
    }
}

/**
 * @tc.number    : TaskDispatcher_Parallel_07700
 * @tc.name      : parallel group notify:serial delay
 * @tc.desc      : two levels task dispatch
 */
HWTEST_F(TaskDispatcherTest, TaskDispatcher_Parallel_07700, Function | MediumTest | Level1)
{
    StartAbility(firstAbilityName, bundleNameFirst);
    std::string data = "Dispatcher_" + std::to_string((int)TestFunc::PARALLEL) + "_76";
    bool result = false;
    for (int i = 0; i < stLevel_.AMSLevel; i++) {
        STAbilityUtil::PublishEvent(g_EVENT_REQU_FIRST, ++testCaseCode, data);
        EXPECT_EQ(0, STAbilityUtil::WaitCompleted(event, g_EVENT_RESP_FIRST, testCaseCode, DELAY));
        data = STAbilityUtil::GetData(event, g_EVENT_RESP_FIRST, testCaseCode);
        result = data.compare("1") == 0;
        EXPECT_TRUE(result);
        if (!result && i > 0) {
            GTEST_LOG_(INFO) << "TaskDispatcher_Parallel_07700 : " << i;
            break;
        }
    }
}

/**
 * @tc.number    : TaskDispatcher_Parallel_07800
 * @tc.name      : parallel group notify:serial apply
 * @tc.desc      : two levels task dispatch
 */
HWTEST_F(TaskDispatcherTest, TaskDispatcher_Parallel_07800, Function | MediumTest | Level1)
{
    StartAbility(firstAbilityName, bundleNameFirst);
    std::string data = "Dispatcher_" + std::to_string((int)TestFunc::PARALLEL) + "_77";
    bool result = false;
    for (int i = 0; i < stLevel_.AMSLevel; i++) {
        STAbilityUtil::PublishEvent(g_EVENT_REQU_FIRST, ++testCaseCode, data);
        EXPECT_EQ(0, STAbilityUtil::WaitCompleted(event, g_EVENT_RESP_FIRST, testCaseCode, DELAY));
        data = STAbilityUtil::GetData(event, g_EVENT_RESP_FIRST, testCaseCode);
        result = data.compare("1") == 0;
        EXPECT_TRUE(result);
        if (!result && i > 0) {
            GTEST_LOG_(INFO) << "TaskDispatcher_Parallel_07800 : " << i;
            break;
        }
    }
}

/**
 * @tc.number    : TaskDispatcher_Parallel_07900
 * @tc.name      : parallel sync barrier:parallel sync
 * @tc.desc      : two levels task dispatch
 */
HWTEST_F(TaskDispatcherTest, TaskDispatcher_Parallel_07900, Function | MediumTest | Level1)
{
    StartAbility(firstAbilityName, bundleNameFirst);
    std::string data = "Dispatcher_" + std::to_string((int)TestFunc::PARALLEL) + "_78";
    bool result = false;
    for (int i = 0; i < stLevel_.AMSLevel; i++) {
        STAbilityUtil::PublishEvent(g_EVENT_REQU_FIRST, ++testCaseCode, data);
        EXPECT_EQ(0, STAbilityUtil::WaitCompleted(event, g_EVENT_RESP_FIRST, testCaseCode, DELAY));
        data = STAbilityUtil::GetData(event, g_EVENT_RESP_FIRST, testCaseCode);
        result = data.compare("1") == 0;
        EXPECT_TRUE(result);
        if (!result && i > 0) {
            GTEST_LOG_(INFO) << "TaskDispatcher_Parallel_07900 : " << i;
            break;
        }
    }
}

/**
 * @tc.number    : TaskDispatcher_Parallel_08000
 * @tc.name      : parallel sync barrier:parallel async
 * @tc.desc      : two levels task dispatch
 */
HWTEST_F(TaskDispatcherTest, TaskDispatcher_Parallel_08000, Function | MediumTest | Level1)
{
    StartAbility(firstAbilityName, bundleNameFirst);
    std::string data = "Dispatcher_" + std::to_string((int)TestFunc::PARALLEL) + "_79";
    bool result = false;
    for (int i = 0; i < stLevel_.AMSLevel; i++) {
        STAbilityUtil::PublishEvent(g_EVENT_REQU_FIRST, ++testCaseCode, data);
        EXPECT_EQ(0, STAbilityUtil::WaitCompleted(event, g_EVENT_RESP_FIRST, testCaseCode, DELAY));
        data = STAbilityUtil::GetData(event, g_EVENT_RESP_FIRST, testCaseCode);
        result = data.compare("1") == 0;
        EXPECT_TRUE(result);
        if (!result && i > 0) {
            GTEST_LOG_(INFO) << "TaskDispatcher_Parallel_08000 : " << i;
            break;
        }
    }
}

/**
 * @tc.number    : TaskDispatcher_Parallel_08100
 * @tc.name      : parallel sync barrier:parallel delay
 * @tc.desc      : two levels task dispatch
 */
HWTEST_F(TaskDispatcherTest, TaskDispatcher_Parallel_08100, Function | MediumTest | Level1)
{
    StartAbility(firstAbilityName, bundleNameFirst);
    std::string data = "Dispatcher_" + std::to_string((int)TestFunc::PARALLEL) + "_80";
    bool result = false;
    for (int i = 0; i < stLevel_.AMSLevel; i++) {
        STAbilityUtil::PublishEvent(g_EVENT_REQU_FIRST, ++testCaseCode, data);
        EXPECT_EQ(0, STAbilityUtil::WaitCompleted(event, g_EVENT_RESP_FIRST, testCaseCode, DELAY));
        data = STAbilityUtil::GetData(event, g_EVENT_RESP_FIRST, testCaseCode);
        result = data.compare("1") == 0;
        EXPECT_TRUE(result);
        if (!result && i > 0) {
            GTEST_LOG_(INFO) << "TaskDispatcher_Parallel_08100 : " << i;
            break;
        }
    }
}

/**
 * @tc.number    : TaskDispatcher_Parallel_08200
 * @tc.name      : parallel sync barrier:parallel group
 * @tc.desc      : two levels task dispatch
 */
HWTEST_F(TaskDispatcherTest, TaskDispatcher_Parallel_08200, Function | MediumTest | Level1)
{
    StartAbility(firstAbilityName, bundleNameFirst);
    std::string data = "Dispatcher_" + std::to_string((int)TestFunc::PARALLEL) + "_81";
    bool result = false;
    for (int i = 0; i < stLevel_.AMSLevel; i++) {
        STAbilityUtil::PublishEvent(g_EVENT_REQU_FIRST, ++testCaseCode, data);
        EXPECT_EQ(0, STAbilityUtil::WaitCompleted(event, g_EVENT_RESP_FIRST, testCaseCode, DELAY));
        data = STAbilityUtil::GetData(event, g_EVENT_RESP_FIRST, testCaseCode);
        result = data.compare("1") == 0;
        EXPECT_TRUE(result);
        if (!result && i > 0) {
            GTEST_LOG_(INFO) << "TaskDispatcher_Parallel_08200 : " << i;
            break;
        }
    }
}

/**
 * @tc.number    : TaskDispatcher_Parallel_08300
 * @tc.name      : parallel sync barrier:parallel group wait
 * @tc.desc      : two levels task dispatch
 */
HWTEST_F(TaskDispatcherTest, TaskDispatcher_Parallel_08300, Function | MediumTest | Level1)
{
    StartAbility(firstAbilityName, bundleNameFirst);
    std::string data = "Dispatcher_" + std::to_string((int)TestFunc::PARALLEL) + "_82";
    bool result = false;
    for (int i = 0; i < stLevel_.AMSLevel; i++) {
        STAbilityUtil::PublishEvent(g_EVENT_REQU_FIRST, ++testCaseCode, data);
        EXPECT_EQ(0, STAbilityUtil::WaitCompleted(event, g_EVENT_RESP_FIRST, testCaseCode, DELAY));
        data = STAbilityUtil::GetData(event, g_EVENT_RESP_FIRST, testCaseCode);
        result = data.compare("1") == 0;
        EXPECT_TRUE(result);
        if (!result && i > 0) {
            GTEST_LOG_(INFO) << "TaskDispatcher_Parallel_08300 : " << i;
            break;
        }
    }
}

/**
 * @tc.number    : TaskDispatcher_Parallel_08400
 * @tc.name      : parallel sync barrier:parallel group notify
 * @tc.desc      : two levels task dispatch
 */
HWTEST_F(TaskDispatcherTest, TaskDispatcher_Parallel_08400, Function | MediumTest | Level1)
{
    StartAbility(firstAbilityName, bundleNameFirst);
    std::string data = "Dispatcher_" + std::to_string((int)TestFunc::PARALLEL) + "_83";
    bool result = false;
    for (int i = 0; i < stLevel_.AMSLevel; i++) {
        STAbilityUtil::PublishEvent(g_EVENT_REQU_FIRST, ++testCaseCode, data);
        EXPECT_EQ(0, STAbilityUtil::WaitCompleted(event, g_EVENT_RESP_FIRST, testCaseCode, DELAY));
        data = STAbilityUtil::GetData(event, g_EVENT_RESP_FIRST, testCaseCode);
        result = data.compare("1") == 0;
        EXPECT_TRUE(result);
        if (!result && i > 0) {
            GTEST_LOG_(INFO) << "TaskDispatcher_Parallel_08400 : " << i;
            break;
        }
    }
}

/**
 * @tc.number    : TaskDispatcher_Parallel_08500
 * @tc.name      : parallel sync barrier:parallel sync barrier
 * @tc.desc      : two levels task dispatch
 */
HWTEST_F(TaskDispatcherTest, TaskDispatcher_Parallel_08500, Function | MediumTest | Level1)
{
    StartAbility(firstAbilityName, bundleNameFirst);
    std::string data = "Dispatcher_" + std::to_string((int)TestFunc::PARALLEL) + "_84";
    bool result = false;
    for (int i = 0; i < stLevel_.AMSLevel; i++) {
        STAbilityUtil::PublishEvent(g_EVENT_REQU_FIRST, ++testCaseCode, data);
        EXPECT_EQ(0, STAbilityUtil::WaitCompleted(event, g_EVENT_RESP_FIRST, testCaseCode, DELAY));
        data = STAbilityUtil::GetData(event, g_EVENT_RESP_FIRST, testCaseCode);
        result = data.compare("1") == 0;
        EXPECT_TRUE(result);
        if (!result && i > 0) {
            GTEST_LOG_(INFO) << "TaskDispatcher_Parallel_08500 : " << i;
            break;
        }
    }
}

/**
 * @tc.number    : TaskDispatcher_Parallel_08600
 * @tc.name      : parallel sync barrier:parallel async barrier
 * @tc.desc      : two levels task dispatch
 */
HWTEST_F(TaskDispatcherTest, TaskDispatcher_Parallel_08600, Function | MediumTest | Level1)
{
    StartAbility(firstAbilityName, bundleNameFirst);
    std::string data = "Dispatcher_" + std::to_string((int)TestFunc::PARALLEL) + "_85";
    bool result = false;
    for (int i = 0; i < stLevel_.AMSLevel; i++) {
        STAbilityUtil::PublishEvent(g_EVENT_REQU_FIRST, ++testCaseCode, data);
        EXPECT_EQ(0, STAbilityUtil::WaitCompleted(event, g_EVENT_RESP_FIRST, testCaseCode, DELAY));
        data = STAbilityUtil::GetData(event, g_EVENT_RESP_FIRST, testCaseCode);
        result = data.compare("1") == 0;
        EXPECT_TRUE(result);
        if (!result && i > 0) {
            GTEST_LOG_(INFO) << "TaskDispatcher_Parallel_08600 : " << i;
            break;
        }
    }
}

/**
 * @tc.number    : TaskDispatcher_Parallel_08700
 * @tc.name      : parallel sync barrier:parallel apply
 * @tc.desc      : two levels task dispatch
 */
HWTEST_F(TaskDispatcherTest, TaskDispatcher_Parallel_08700, Function | MediumTest | Level1)
{
    StartAbility(firstAbilityName, bundleNameFirst);
    std::string data = "Dispatcher_" + std::to_string((int)TestFunc::PARALLEL) + "_86";
    bool result = false;
    for (int i = 0; i < stLevel_.AMSLevel; i++) {
        STAbilityUtil::PublishEvent(g_EVENT_REQU_FIRST, ++testCaseCode, data);
        EXPECT_EQ(0, STAbilityUtil::WaitCompleted(event, g_EVENT_RESP_FIRST, testCaseCode, DELAY));
        data = STAbilityUtil::GetData(event, g_EVENT_RESP_FIRST, testCaseCode);
        result = data.compare("1") == 0;
        EXPECT_TRUE(result);
        if (!result && i > 0) {
            GTEST_LOG_(INFO) << "TaskDispatcher_Parallel_08700 : " << i;
            break;
        }
    }
}

/**
 * @tc.number    : TaskDispatcher_Parallel_08800
 * @tc.name      : parallel sync barrier:serial sync
 * @tc.desc      : two levels task dispatch
 */
HWTEST_F(TaskDispatcherTest, TaskDispatcher_Parallel_08800, Function | MediumTest | Level1)
{
    StartAbility(firstAbilityName, bundleNameFirst);
    std::string data = "Dispatcher_" + std::to_string((int)TestFunc::PARALLEL) + "_87";
    bool result = false;
    for (int i = 0; i < stLevel_.AMSLevel; i++) {
        STAbilityUtil::PublishEvent(g_EVENT_REQU_FIRST, ++testCaseCode, data);
        EXPECT_EQ(0, STAbilityUtil::WaitCompleted(event, g_EVENT_RESP_FIRST, testCaseCode, DELAY));
        data = STAbilityUtil::GetData(event, g_EVENT_RESP_FIRST, testCaseCode);
        result = data.compare("1") == 0;
        EXPECT_TRUE(result);
        if (!result && i > 0) {
            GTEST_LOG_(INFO) << "TaskDispatcher_Parallel_08800 : " << i;
            break;
        }
    }
}

/**
 * @tc.number    : TaskDispatcher_Parallel_08900
 * @tc.name      : parallel sync barrier:serial async
 * @tc.desc      : two levels task dispatch
 */
HWTEST_F(TaskDispatcherTest, TaskDispatcher_Parallel_08900, Function | MediumTest | Level1)
{
    StartAbility(firstAbilityName, bundleNameFirst);
    std::string data = "Dispatcher_" + std::to_string((int)TestFunc::PARALLEL) + "_88";
    bool result = false;
    for (int i = 0; i < stLevel_.AMSLevel; i++) {
        STAbilityUtil::PublishEvent(g_EVENT_REQU_FIRST, ++testCaseCode, data);
        EXPECT_EQ(0, STAbilityUtil::WaitCompleted(event, g_EVENT_RESP_FIRST, testCaseCode, DELAY));
        data = STAbilityUtil::GetData(event, g_EVENT_RESP_FIRST, testCaseCode);
        result = data.compare("1") == 0;
        EXPECT_TRUE(result);
        if (!result && i > 0) {
            GTEST_LOG_(INFO) << "TaskDispatcher_Parallel_08900 : " << i;
            break;
        }
    }
}

/**
 * @tc.number    : TaskDispatcher_Parallel_09000
 * @tc.name      : parallel sync barrier:serial delay
 * @tc.desc      : two levels task dispatch
 */
HWTEST_F(TaskDispatcherTest, TaskDispatcher_Parallel_09000, Function | MediumTest | Level1)
{
    StartAbility(firstAbilityName, bundleNameFirst);
    std::string data = "Dispatcher_" + std::to_string((int)TestFunc::PARALLEL) + "_89";
    bool result = false;
    for (int i = 0; i < stLevel_.AMSLevel; i++) {
        STAbilityUtil::PublishEvent(g_EVENT_REQU_FIRST, ++testCaseCode, data);
        EXPECT_EQ(0, STAbilityUtil::WaitCompleted(event, g_EVENT_RESP_FIRST, testCaseCode, DELAY));
        data = STAbilityUtil::GetData(event, g_EVENT_RESP_FIRST, testCaseCode);
        result = data.compare("1") == 0;
        EXPECT_TRUE(result);
        if (!result && i > 0) {
            GTEST_LOG_(INFO) << "TaskDispatcher_Parallel_09000 : " << i;
            break;
        }
    }
}

/**
 * @tc.number    : TaskDispatcher_Parallel_09100
 * @tc.name      : parallel sync barrier:serial apply
 * @tc.desc      : two levels task dispatch
 */
HWTEST_F(TaskDispatcherTest, TaskDispatcher_Parallel_09100, Function | MediumTest | Level1)
{
    StartAbility(firstAbilityName, bundleNameFirst);
    std::string data = "Dispatcher_" + std::to_string((int)TestFunc::PARALLEL) + "_90";
    bool result = false;
    for (int i = 0; i < stLevel_.AMSLevel; i++) {
        STAbilityUtil::PublishEvent(g_EVENT_REQU_FIRST, ++testCaseCode, data);
        EXPECT_EQ(0, STAbilityUtil::WaitCompleted(event, g_EVENT_RESP_FIRST, testCaseCode, DELAY));
        data = STAbilityUtil::GetData(event, g_EVENT_RESP_FIRST, testCaseCode);
        result = data.compare("1") == 0;
        EXPECT_TRUE(result);
        if (!result && i > 0) {
            GTEST_LOG_(INFO) << "TaskDispatcher_Parallel_09100 : " << i;
            break;
        }
    }
}

/**
 * @tc.number    : TaskDispatcher_Parallel_09200
 * @tc.name      : parallel async barrier:parallel sync
 * @tc.desc      : two levels task dispatch
 */
HWTEST_F(TaskDispatcherTest, TaskDispatcher_Parallel_09200, Function | MediumTest | Level1)
{
    StartAbility(firstAbilityName, bundleNameFirst);
    std::string data = "Dispatcher_" + std::to_string((int)TestFunc::PARALLEL) + "_91";
    bool result = false;
    for (int i = 0; i < stLevel_.AMSLevel; i++) {
        STAbilityUtil::PublishEvent(g_EVENT_REQU_FIRST, ++testCaseCode, data);
        EXPECT_EQ(0, STAbilityUtil::WaitCompleted(event, g_EVENT_RESP_FIRST, testCaseCode, DELAY));
        data = STAbilityUtil::GetData(event, g_EVENT_RESP_FIRST, testCaseCode);
        result = data.compare("1") == 0;
        EXPECT_TRUE(result);
        if (!result && i > 0) {
            GTEST_LOG_(INFO) << "TaskDispatcher_Parallel_09200 : " << i;
            break;
        }
    }
}

/**
 * @tc.number    : TaskDispatcher_Parallel_09300
 * @tc.name      : parallel async barrier:parallel async
 * @tc.desc      : two levels task dispatch
 */
HWTEST_F(TaskDispatcherTest, TaskDispatcher_Parallel_09300, Function | MediumTest | Level1)
{
    StartAbility(firstAbilityName, bundleNameFirst);
    std::string data = "Dispatcher_" + std::to_string((int)TestFunc::PARALLEL) + "_92";
    bool result = false;
    for (int i = 0; i < stLevel_.AMSLevel; i++) {
        STAbilityUtil::PublishEvent(g_EVENT_REQU_FIRST, ++testCaseCode, data);
        EXPECT_EQ(0, STAbilityUtil::WaitCompleted(event, g_EVENT_RESP_FIRST, testCaseCode, DELAY));
        data = STAbilityUtil::GetData(event, g_EVENT_RESP_FIRST, testCaseCode);
        result = data.compare("1") == 0;
        EXPECT_TRUE(result);
        if (!result && i > 0) {
            GTEST_LOG_(INFO) << "TaskDispatcher_Parallel_09300 : " << i;
            break;
        }
    }
}

/**
 * @tc.number    : TaskDispatcher_Parallel_09400
 * @tc.name      : parallel async barrier:parallel delay
 * @tc.desc      : two levels task dispatch
 */
HWTEST_F(TaskDispatcherTest, TaskDispatcher_Parallel_09400, Function | MediumTest | Level1)
{
    StartAbility(firstAbilityName, bundleNameFirst);
    std::string data = "Dispatcher_" + std::to_string((int)TestFunc::PARALLEL) + "_93";
    bool result = false;
    for (int i = 0; i < stLevel_.AMSLevel; i++) {
        STAbilityUtil::PublishEvent(g_EVENT_REQU_FIRST, ++testCaseCode, data);
        EXPECT_EQ(0, STAbilityUtil::WaitCompleted(event, g_EVENT_RESP_FIRST, testCaseCode, DELAY));
        data = STAbilityUtil::GetData(event, g_EVENT_RESP_FIRST, testCaseCode);
        result = data.compare("1") == 0;
        EXPECT_TRUE(result);
        if (!result && i > 0) {
            GTEST_LOG_(INFO) << "TaskDispatcher_Parallel_09400 : " << i;
            break;
        }
    }
}

/**
 * @tc.number    : TaskDispatcher_Parallel_09500
 * @tc.name      : parallel async barrier:parallel group
 * @tc.desc      : two levels task dispatch
 */
HWTEST_F(TaskDispatcherTest, TaskDispatcher_Parallel_09500, Function | MediumTest | Level1)
{
    StartAbility(firstAbilityName, bundleNameFirst);
    std::string data = "Dispatcher_" + std::to_string((int)TestFunc::PARALLEL) + "_94";
    bool result = false;
    for (int i = 0; i < stLevel_.AMSLevel; i++) {
        STAbilityUtil::PublishEvent(g_EVENT_REQU_FIRST, ++testCaseCode, data);
        EXPECT_EQ(0, STAbilityUtil::WaitCompleted(event, g_EVENT_RESP_FIRST, testCaseCode, DELAY));
        data = STAbilityUtil::GetData(event, g_EVENT_RESP_FIRST, testCaseCode);
        result = data.compare("1") == 0;
        EXPECT_TRUE(result);
        if (!result && i > 0) {
            GTEST_LOG_(INFO) << "TaskDispatcher_Parallel_09500 : " << i;
            break;
        }
    }
}

/**
 * @tc.number    : TaskDispatcher_Parallel_09600
 * @tc.name      : parallel async barrier:parallel group wait
 * @tc.desc      : two levels task dispatch
 */
HWTEST_F(TaskDispatcherTest, TaskDispatcher_Parallel_09600, Function | MediumTest | Level1)
{
    StartAbility(firstAbilityName, bundleNameFirst);
    std::string data = "Dispatcher_" + std::to_string((int)TestFunc::PARALLEL) + "_95";
    bool result = false;
    for (int i = 0; i < stLevel_.AMSLevel; i++) {
        STAbilityUtil::PublishEvent(g_EVENT_REQU_FIRST, ++testCaseCode, data);
        EXPECT_EQ(0, STAbilityUtil::WaitCompleted(event, g_EVENT_RESP_FIRST, testCaseCode, DELAY));
        data = STAbilityUtil::GetData(event, g_EVENT_RESP_FIRST, testCaseCode);
        result = data.compare("1") == 0;
        EXPECT_TRUE(result);
        if (!result && i > 0) {
            GTEST_LOG_(INFO) << "TaskDispatcher_Parallel_09600 : " << i;
            break;
        }
    }
}

/**
 * @tc.number    : TaskDispatcher_Parallel_09700
 * @tc.name      : parallel async barrier:parallel group notify
 * @tc.desc      : two levels task dispatch
 */
HWTEST_F(TaskDispatcherTest, TaskDispatcher_Parallel_09700, Function | MediumTest | Level1)
{
    StartAbility(firstAbilityName, bundleNameFirst);
    std::string data = "Dispatcher_" + std::to_string((int)TestFunc::PARALLEL) + "_96";
    bool result = false;
    for (int i = 0; i < stLevel_.AMSLevel; i++) {
        STAbilityUtil::PublishEvent(g_EVENT_REQU_FIRST, ++testCaseCode, data);
        EXPECT_EQ(0, STAbilityUtil::WaitCompleted(event, g_EVENT_RESP_FIRST, testCaseCode, DELAY));
        data = STAbilityUtil::GetData(event, g_EVENT_RESP_FIRST, testCaseCode);
        result = data.compare("1") == 0;
        EXPECT_TRUE(result);
        if (!result && i > 0) {
            GTEST_LOG_(INFO) << "TaskDispatcher_Parallel_09700 : " << i;
            break;
        }
    }
}

/**
 * @tc.number    : TaskDispatcher_Parallel_09800
 * @tc.name      : parallel async barrier:parallel sync barrier
 * @tc.desc      : two levels task dispatch
 */
HWTEST_F(TaskDispatcherTest, TaskDispatcher_Parallel_09800, Function | MediumTest | Level1)
{
    StartAbility(firstAbilityName, bundleNameFirst);
    std::string data = "Dispatcher_" + std::to_string((int)TestFunc::PARALLEL) + "_97";
    bool result = false;
    for (int i = 0; i < stLevel_.AMSLevel; i++) {
        STAbilityUtil::PublishEvent(g_EVENT_REQU_FIRST, ++testCaseCode, data);
        EXPECT_EQ(0, STAbilityUtil::WaitCompleted(event, g_EVENT_RESP_FIRST, testCaseCode, DELAY));
        data = STAbilityUtil::GetData(event, g_EVENT_RESP_FIRST, testCaseCode);
        result = data.compare("1") == 0;
        EXPECT_TRUE(result);
        if (!result && i > 0) {
            GTEST_LOG_(INFO) << "TaskDispatcher_Parallel_09800 : " << i;
            break;
        }
    }
}

/**
 * @tc.number    : TaskDispatcher_Parallel_09900
 * @tc.name      : parallel async barrier:parallel async barrier
 * @tc.desc      : two levels task dispatch
 */
HWTEST_F(TaskDispatcherTest, TaskDispatcher_Parallel_09900, Function | MediumTest | Level1)
{
    StartAbility(firstAbilityName, bundleNameFirst);
    std::string data = "Dispatcher_" + std::to_string((int)TestFunc::PARALLEL) + "_98";
    bool result = false;
    for (int i = 0; i < stLevel_.AMSLevel; i++) {
        STAbilityUtil::PublishEvent(g_EVENT_REQU_FIRST, ++testCaseCode, data);
        EXPECT_EQ(0, STAbilityUtil::WaitCompleted(event, g_EVENT_RESP_FIRST, testCaseCode, DELAY));
        data = STAbilityUtil::GetData(event, g_EVENT_RESP_FIRST, testCaseCode);
        result = data.compare("1") == 0;
        EXPECT_TRUE(result);
        if (!result && i > 0) {
            GTEST_LOG_(INFO) << "TaskDispatcher_Parallel_09900 : " << i;
            break;
        }
    }
}

/**
 * @tc.number    : TaskDispatcher_Parallel_10000
 * @tc.name      : parallel async barrier:parallel apply
 * @tc.desc      : two levels task dispatch
 */
HWTEST_F(TaskDispatcherTest, TaskDispatcher_Parallel_10000, Function | MediumTest | Level1)
{
    StartAbility(firstAbilityName, bundleNameFirst);
    std::string data = "Dispatcher_" + std::to_string((int)TestFunc::PARALLEL) + "_99";
    bool result = false;
    for (int i = 0; i < stLevel_.AMSLevel; i++) {
        STAbilityUtil::PublishEvent(g_EVENT_REQU_FIRST, ++testCaseCode, data);
        EXPECT_EQ(0, STAbilityUtil::WaitCompleted(event, g_EVENT_RESP_FIRST, testCaseCode, DELAY));
        data = STAbilityUtil::GetData(event, g_EVENT_RESP_FIRST, testCaseCode);
        result = data.compare("1") == 0;
        EXPECT_TRUE(result);
        if (!result && i > 0) {
            GTEST_LOG_(INFO) << "TaskDispatcher_Parallel_10000 : " << i;
            break;
        }
    }
}

/**
 * @tc.number    : TaskDispatcher_Parallel_10100
 * @tc.name      : parallel async barrier:serial sync
 * @tc.desc      : two levels task dispatch
 */
HWTEST_F(TaskDispatcherTest, TaskDispatcher_Parallel_10100, Function | MediumTest | Level1)
{
    StartAbility(firstAbilityName, bundleNameFirst);
    std::string data = "Dispatcher_" + std::to_string((int)TestFunc::PARALLEL) + "_100";
    bool result = false;
    for (int i = 0; i < stLevel_.AMSLevel; i++) {
        STAbilityUtil::PublishEvent(g_EVENT_REQU_FIRST, ++testCaseCode, data);
        EXPECT_EQ(0, STAbilityUtil::WaitCompleted(event, g_EVENT_RESP_FIRST, testCaseCode, DELAY));
        data = STAbilityUtil::GetData(event, g_EVENT_RESP_FIRST, testCaseCode);
        result = data.compare("1") == 0;
        EXPECT_TRUE(result);
        if (!result && i > 0) {
            GTEST_LOG_(INFO) << "TaskDispatcher_Parallel_10100 : " << i;
            break;
        }
    }
}

/**
 * @tc.number    : TaskDispatcher_Parallel_10200
 * @tc.name      : parallel async barrier:serial async
 * @tc.desc      : two levels task dispatch
 */
HWTEST_F(TaskDispatcherTest, TaskDispatcher_Parallel_10200, Function | MediumTest | Level1)
{
    StartAbility(firstAbilityName, bundleNameFirst);
    std::string data = "Dispatcher_" + std::to_string((int)TestFunc::PARALLEL) + "_101";
    bool result = false;
    for (int i = 0; i < stLevel_.AMSLevel; i++) {
        STAbilityUtil::PublishEvent(g_EVENT_REQU_FIRST, ++testCaseCode, data);
        EXPECT_EQ(0, STAbilityUtil::WaitCompleted(event, g_EVENT_RESP_FIRST, testCaseCode, DELAY));
        data = STAbilityUtil::GetData(event, g_EVENT_RESP_FIRST, testCaseCode);
        result = data.compare("1") == 0;
        EXPECT_TRUE(result);
        if (!result && i > 0) {
            GTEST_LOG_(INFO) << "TaskDispatcher_Parallel_10200 : " << i;
            break;
        }
    }
}

/**
 * @tc.number    : TaskDispatcher_Parallel_10300
 * @tc.name      : parallel async barrier:serial delay
 * @tc.desc      : two levels task dispatch
 */
HWTEST_F(TaskDispatcherTest, TaskDispatcher_Parallel_10300, Function | MediumTest | Level1)
{
    StartAbility(firstAbilityName, bundleNameFirst);
    std::string data = "Dispatcher_" + std::to_string((int)TestFunc::PARALLEL) + "_102";
    bool result = false;
    for (int i = 0; i < stLevel_.AMSLevel; i++) {
        STAbilityUtil::PublishEvent(g_EVENT_REQU_FIRST, ++testCaseCode, data);
        EXPECT_EQ(0, STAbilityUtil::WaitCompleted(event, g_EVENT_RESP_FIRST, testCaseCode, DELAY));
        data = STAbilityUtil::GetData(event, g_EVENT_RESP_FIRST, testCaseCode);
        result = data.compare("1") == 0;
        EXPECT_TRUE(result);
        if (!result && i > 0) {
            GTEST_LOG_(INFO) << "TaskDispatcher_Parallel_10300 : " << i;
            break;
        }
    }
}

/**
 * @tc.number    : TaskDispatcher_Parallel_10400
 * @tc.name      : parallel async barrier:serial apply
 * @tc.desc      : two levels task dispatch
 */
HWTEST_F(TaskDispatcherTest, TaskDispatcher_Parallel_10400, Function | MediumTest | Level1)
{
    StartAbility(firstAbilityName, bundleNameFirst);
    std::string data = "Dispatcher_" + std::to_string((int)TestFunc::PARALLEL) + "_103";
    bool result = false;
    for (int i = 0; i < stLevel_.AMSLevel; i++) {
        STAbilityUtil::PublishEvent(g_EVENT_REQU_FIRST, ++testCaseCode, data);
        EXPECT_EQ(0, STAbilityUtil::WaitCompleted(event, g_EVENT_RESP_FIRST, testCaseCode, DELAY));
        data = STAbilityUtil::GetData(event, g_EVENT_RESP_FIRST, testCaseCode);
        result = data.compare("1") == 0;
        EXPECT_TRUE(result);
        if (!result && i > 0) {
            GTEST_LOG_(INFO) << "TaskDispatcher_Parallel_10400 : " << i;
            break;
        }
    }
}

/**
 * @tc.number    : TaskDispatcher_Parallel_10500
 * @tc.name      : parallel apply:parallel sync
 * @tc.desc      : two levels task dispatch
 */
HWTEST_F(TaskDispatcherTest, TaskDispatcher_Parallel_10500, Function | MediumTest | Level1)
{
    StartAbility(firstAbilityName, bundleNameFirst);
    std::string data = "Dispatcher_" + std::to_string((int)TestFunc::PARALLEL) + "_104";
    bool result = false;
    for (int i = 0; i < stLevel_.AMSLevel; i++) {
        STAbilityUtil::PublishEvent(g_EVENT_REQU_FIRST, ++testCaseCode, data);
        EXPECT_EQ(0, STAbilityUtil::WaitCompleted(event, g_EVENT_RESP_FIRST, testCaseCode, DELAY));
        data = STAbilityUtil::GetData(event, g_EVENT_RESP_FIRST, testCaseCode);
        result = data.compare("1") == 0;
        EXPECT_TRUE(result);
        if (!result && i > 0) {
            GTEST_LOG_(INFO) << "TaskDispatcher_Parallel_10500 : " << i;
            break;
        }
    }
}

/**
 * @tc.number    : TaskDispatcher_Parallel_10600
 * @tc.name      : parallel apply:parallel async
 * @tc.desc      : two levels task dispatch
 */
HWTEST_F(TaskDispatcherTest, TaskDispatcher_Parallel_10600, Function | MediumTest | Level1)
{
    StartAbility(firstAbilityName, bundleNameFirst);
    std::string data = "Dispatcher_" + std::to_string((int)TestFunc::PARALLEL) + "_105";
    bool result = false;
    for (int i = 0; i < stLevel_.AMSLevel; i++) {
        STAbilityUtil::PublishEvent(g_EVENT_REQU_FIRST, ++testCaseCode, data);
        EXPECT_EQ(0, STAbilityUtil::WaitCompleted(event, g_EVENT_RESP_FIRST, testCaseCode, DELAY));
        data = STAbilityUtil::GetData(event, g_EVENT_RESP_FIRST, testCaseCode);
        result = data.compare("1") == 0;
        EXPECT_TRUE(result);
        if (!result && i > 0) {
            GTEST_LOG_(INFO) << "TaskDispatcher_Parallel_10600 : " << i;
            break;
        }
    }
}

/**
 * @tc.number    : TaskDispatcher_Parallel_10700
 * @tc.name      : parallel apply:parallel delay
 * @tc.desc      : two levels task dispatch
 */
HWTEST_F(TaskDispatcherTest, TaskDispatcher_Parallel_10700, Function | MediumTest | Level1)
{
    StartAbility(firstAbilityName, bundleNameFirst);
    std::string data = "Dispatcher_" + std::to_string((int)TestFunc::PARALLEL) + "_106";
    bool result = false;
    for (int i = 0; i < stLevel_.AMSLevel; i++) {
        STAbilityUtil::PublishEvent(g_EVENT_REQU_FIRST, ++testCaseCode, data);
        EXPECT_EQ(0, STAbilityUtil::WaitCompleted(event, g_EVENT_RESP_FIRST, testCaseCode, DELAY));
        data = STAbilityUtil::GetData(event, g_EVENT_RESP_FIRST, testCaseCode);
        result = data.compare("1") == 0;
        EXPECT_TRUE(result);
        if (!result && i > 0) {
            GTEST_LOG_(INFO) << "TaskDispatcher_Parallel_10700 : " << i;
            break;
        }
    }
}

/**
 * @tc.number    : TaskDispatcher_Parallel_10800
 * @tc.name      : parallel apply:parallel group
 * @tc.desc      : two levels task dispatch
 */
HWTEST_F(TaskDispatcherTest, TaskDispatcher_Parallel_10800, Function | MediumTest | Level1)
{
    StartAbility(firstAbilityName, bundleNameFirst);
    std::string data = "Dispatcher_" + std::to_string((int)TestFunc::PARALLEL) + "_107";
    bool result = false;
    for (int i = 0; i < stLevel_.AMSLevel; i++) {
        STAbilityUtil::PublishEvent(g_EVENT_REQU_FIRST, ++testCaseCode, data);
        EXPECT_EQ(0, STAbilityUtil::WaitCompleted(event, g_EVENT_RESP_FIRST, testCaseCode, DELAY));
        data = STAbilityUtil::GetData(event, g_EVENT_RESP_FIRST, testCaseCode);
        result = data.compare("1") == 0;
        EXPECT_TRUE(result);
        if (!result && i > 0) {
            GTEST_LOG_(INFO) << "TaskDispatcher_Parallel_10800 : " << i;
            break;
        }
    }
}

/**
 * @tc.number    : TaskDispatcher_Parallel_10900
 * @tc.name      : parallel apply:parallel group wait
 * @tc.desc      : two levels task dispatch
 */
HWTEST_F(TaskDispatcherTest, TaskDispatcher_Parallel_10900, Function | MediumTest | Level1)
{
    StartAbility(firstAbilityName, bundleNameFirst);
    std::string data = "Dispatcher_" + std::to_string((int)TestFunc::PARALLEL) + "_108";
    bool result = false;
    for (int i = 0; i < stLevel_.AMSLevel; i++) {
        STAbilityUtil::PublishEvent(g_EVENT_REQU_FIRST, ++testCaseCode, data);
        EXPECT_EQ(0, STAbilityUtil::WaitCompleted(event, g_EVENT_RESP_FIRST, testCaseCode, DELAY));
        data = STAbilityUtil::GetData(event, g_EVENT_RESP_FIRST, testCaseCode);
        result = data.compare("1") == 0;
        EXPECT_TRUE(result);
        if (!result && i > 0) {
            GTEST_LOG_(INFO) << "TaskDispatcher_Parallel_10900 : " << i;
            break;
        }
    }
}

/**
 * @tc.number    : TaskDispatcher_Parallel_11000
 * @tc.name      : parallel apply:parallel group notify
 * @tc.desc      : two levels task dispatch
 */
HWTEST_F(TaskDispatcherTest, TaskDispatcher_Parallel_11000, Function | MediumTest | Level1)
{
    StartAbility(firstAbilityName, bundleNameFirst);
    std::string data = "Dispatcher_" + std::to_string((int)TestFunc::PARALLEL) + "_109";
    bool result = false;
    for (int i = 0; i < stLevel_.AMSLevel; i++) {
        STAbilityUtil::PublishEvent(g_EVENT_REQU_FIRST, ++testCaseCode, data);
        EXPECT_EQ(0, STAbilityUtil::WaitCompleted(event, g_EVENT_RESP_FIRST, testCaseCode, DELAY));
        data = STAbilityUtil::GetData(event, g_EVENT_RESP_FIRST, testCaseCode);
        result = data.compare("1") == 0;
        EXPECT_TRUE(result);
        if (!result && i > 0) {
            GTEST_LOG_(INFO) << "TaskDispatcher_Parallel_11000 : " << i;
            break;
        }
    }
}

/**
 * @tc.number    : TaskDispatcher_Parallel_11100
 * @tc.name      : parallel apply:parallel sync barrier
 * @tc.desc      : two levels task dispatch
 */
HWTEST_F(TaskDispatcherTest, TaskDispatcher_Parallel_11100, Function | MediumTest | Level1)
{
    StartAbility(firstAbilityName, bundleNameFirst);
    std::string data = "Dispatcher_" + std::to_string((int)TestFunc::PARALLEL) + "_110";
    bool result = false;
    for (int i = 0; i < stLevel_.AMSLevel; i++) {
        STAbilityUtil::PublishEvent(g_EVENT_REQU_FIRST, ++testCaseCode, data);
        EXPECT_EQ(0, STAbilityUtil::WaitCompleted(event, g_EVENT_RESP_FIRST, testCaseCode, DELAY));
        data = STAbilityUtil::GetData(event, g_EVENT_RESP_FIRST, testCaseCode);
        result = data.compare("1") == 0;
        EXPECT_TRUE(result);
        if (!result && i > 0) {
            GTEST_LOG_(INFO) << "TaskDispatcher_Parallel_11100 : " << i;
            break;
        }
    }
}

/**
 * @tc.number    : TaskDispatcher_Parallel_11200
 * @tc.name      : parallel apply:parallel async barrier
 * @tc.desc      : two levels task dispatch
 */
HWTEST_F(TaskDispatcherTest, TaskDispatcher_Parallel_11200, Function | MediumTest | Level1)
{
    StartAbility(firstAbilityName, bundleNameFirst);
    std::string data = "Dispatcher_" + std::to_string((int)TestFunc::PARALLEL) + "_111";
    bool result = false;
    for (int i = 0; i < stLevel_.AMSLevel; i++) {
        STAbilityUtil::PublishEvent(g_EVENT_REQU_FIRST, ++testCaseCode, data);
        EXPECT_EQ(0, STAbilityUtil::WaitCompleted(event, g_EVENT_RESP_FIRST, testCaseCode, DELAY));
        data = STAbilityUtil::GetData(event, g_EVENT_RESP_FIRST, testCaseCode);
        result = data.compare("1") == 0;
        EXPECT_TRUE(result);
        if (!result && i > 0) {
            GTEST_LOG_(INFO) << "TaskDispatcher_Parallel_11200 : " << i;
            break;
        }
    }
}

/**
 * @tc.number    : TaskDispatcher_Parallel_11300
 * @tc.name      : parallel apply:parallel apply
 * @tc.desc      : two levels task dispatch
 */
HWTEST_F(TaskDispatcherTest, TaskDispatcher_Parallel_11300, Function | MediumTest | Level1)
{
    StartAbility(firstAbilityName, bundleNameFirst);
    std::string data = "Dispatcher_" + std::to_string((int)TestFunc::PARALLEL) + "_112";
    bool result = false;
    for (int i = 0; i < stLevel_.AMSLevel; i++) {
        STAbilityUtil::PublishEvent(g_EVENT_REQU_FIRST, ++testCaseCode, data);
        EXPECT_EQ(0, STAbilityUtil::WaitCompleted(event, g_EVENT_RESP_FIRST, testCaseCode, DELAY));
        data = STAbilityUtil::GetData(event, g_EVENT_RESP_FIRST, testCaseCode);
        result = data.compare("1") == 0;
        EXPECT_TRUE(result);
        if (!result && i > 0) {
            GTEST_LOG_(INFO) << "TaskDispatcher_Parallel_11300 : " << i;
            break;
        }
    }
}

/**
 * @tc.number    : TaskDispatcher_Parallel_11400
 * @tc.name      : parallel apply:serial sync
 * @tc.desc      : two levels task dispatch
 */
HWTEST_F(TaskDispatcherTest, TaskDispatcher_Parallel_11400, Function | MediumTest | Level1)
{
    StartAbility(firstAbilityName, bundleNameFirst);
    std::string data = "Dispatcher_" + std::to_string((int)TestFunc::PARALLEL) + "_113";
    bool result = false;
    for (int i = 0; i < stLevel_.AMSLevel; i++) {
        STAbilityUtil::PublishEvent(g_EVENT_REQU_FIRST, ++testCaseCode, data);
        EXPECT_EQ(0, STAbilityUtil::WaitCompleted(event, g_EVENT_RESP_FIRST, testCaseCode, DELAY));
        data = STAbilityUtil::GetData(event, g_EVENT_RESP_FIRST, testCaseCode);
        result = data.compare("1") == 0;
        EXPECT_TRUE(result);
        if (!result && i > 0) {
            GTEST_LOG_(INFO) << "TaskDispatcher_Parallel_11400 : " << i;
            break;
        }
    }
}

/**
 * @tc.number    : TaskDispatcher_Parallel_11500
 * @tc.name      : parallel apply:serial async
 * @tc.desc      : two levels task dispatch
 */
HWTEST_F(TaskDispatcherTest, TaskDispatcher_Parallel_11500, Function | MediumTest | Level1)
{
    StartAbility(firstAbilityName, bundleNameFirst);
    std::string data = "Dispatcher_" + std::to_string((int)TestFunc::PARALLEL) + "_114";
    bool result = false;
    for (int i = 0; i < stLevel_.AMSLevel; i++) {
        STAbilityUtil::PublishEvent(g_EVENT_REQU_FIRST, ++testCaseCode, data);
        EXPECT_EQ(0, STAbilityUtil::WaitCompleted(event, g_EVENT_RESP_FIRST, testCaseCode, DELAY));
        data = STAbilityUtil::GetData(event, g_EVENT_RESP_FIRST, testCaseCode);
        result = data.compare("1") == 0;
        EXPECT_TRUE(result);
        if (!result && i > 0) {
            GTEST_LOG_(INFO) << "TaskDispatcher_Parallel_11500 : " << i;
            break;
        }
    }
}

/**
 * @tc.number    : TaskDispatcher_Parallel_11600
 * @tc.name      : parallel apply:serial delay
 * @tc.desc      : two levels task dispatch
 */
HWTEST_F(TaskDispatcherTest, TaskDispatcher_Parallel_11600, Function | MediumTest | Level1)
{
    StartAbility(firstAbilityName, bundleNameFirst);
    std::string data = "Dispatcher_" + std::to_string((int)TestFunc::PARALLEL) + "_115";
    bool result = false;
    for (int i = 0; i < stLevel_.AMSLevel; i++) {
        STAbilityUtil::PublishEvent(g_EVENT_REQU_FIRST, ++testCaseCode, data);
        EXPECT_EQ(0, STAbilityUtil::WaitCompleted(event, g_EVENT_RESP_FIRST, testCaseCode, DELAY));
        data = STAbilityUtil::GetData(event, g_EVENT_RESP_FIRST, testCaseCode);
        result = data.compare("1") == 0;
        EXPECT_TRUE(result);
        if (!result && i > 0) {
            GTEST_LOG_(INFO) << "TaskDispatcher_Parallel_11600 : " << i;
            break;
        }
    }
}

/**
 * @tc.number    : TaskDispatcher_Parallel_11700
 * @tc.name      : parallel apply:serial apply
 * @tc.desc      : two levels task dispatch
 */
HWTEST_F(TaskDispatcherTest, TaskDispatcher_Parallel_11700, Function | MediumTest | Level1)
{
    StartAbility(firstAbilityName, bundleNameFirst);
    std::string data = "Dispatcher_" + std::to_string((int)TestFunc::PARALLEL) + "_116";
    bool result = false;
    for (int i = 0; i < stLevel_.AMSLevel; i++) {
        STAbilityUtil::PublishEvent(g_EVENT_REQU_FIRST, ++testCaseCode, data);
        EXPECT_EQ(0, STAbilityUtil::WaitCompleted(event, g_EVENT_RESP_FIRST, testCaseCode, DELAY));
        data = STAbilityUtil::GetData(event, g_EVENT_RESP_FIRST, testCaseCode);
        result = data.compare("1") == 0;
        EXPECT_TRUE(result);
        if (!result && i > 0) {
            GTEST_LOG_(INFO) << "TaskDispatcher_Parallel_11700 : " << i;
            break;
        }
    }
}

/**
 * @tc.number    : TaskDispatcher_Serial_00100
 * @tc.name      : serial sync:parallel sync
 * @tc.desc      : two levels task dispatch
 */
HWTEST_F(TaskDispatcherTest, TaskDispatcher_Serial_00100, Function | MediumTest | Level1)
{
    StartAbility(firstAbilityName, bundleNameFirst);
    std::string data = "Dispatcher_" + std::to_string((int)TestFunc::SERIAL) + "_0";
    bool result = false;
    for (int i = 0; i < stLevel_.AMSLevel; i++) {
        STAbilityUtil::PublishEvent(g_EVENT_REQU_FIRST, ++testCaseCode, data);
        EXPECT_EQ(0, STAbilityUtil::WaitCompleted(event, g_EVENT_RESP_FIRST, testCaseCode, DELAY));
        data = STAbilityUtil::GetData(event, g_EVENT_RESP_FIRST, testCaseCode);
        result = data.compare("1") == 0;
        EXPECT_TRUE(result);
        if (!result && i > 0) {
            GTEST_LOG_(INFO) << "TaskDispatcher_Serial_00100 : " << i;
            break;
        }
    }
}

/**
 * @tc.number    : TaskDispatcher_Serial_00200
 * @tc.name      : serial sync:parallel async
 * @tc.desc      : two levels task dispatch
 */
HWTEST_F(TaskDispatcherTest, TaskDispatcher_Serial_00200, Function | MediumTest | Level1)
{
    StartAbility(firstAbilityName, bundleNameFirst);
    std::string data = "Dispatcher_" + std::to_string((int)TestFunc::SERIAL) + "_1";
    bool result = false;
    for (int i = 0; i < stLevel_.AMSLevel; i++) {
        STAbilityUtil::PublishEvent(g_EVENT_REQU_FIRST, ++testCaseCode, data);
        EXPECT_EQ(0, STAbilityUtil::WaitCompleted(event, g_EVENT_RESP_FIRST, testCaseCode, DELAY));
        data = STAbilityUtil::GetData(event, g_EVENT_RESP_FIRST, testCaseCode);
        result = data.compare("1") == 0;
        EXPECT_TRUE(result);
        if (!result && i > 0) {
            GTEST_LOG_(INFO) << "TaskDispatcher_Serial_00200 : " << i;
            break;
        }
    }
}

/**
 * @tc.number    : TaskDispatcher_Serial_00300
 * @tc.name      : serial sync:parallel delay
 * @tc.desc      : two levels task dispatch
 */
HWTEST_F(TaskDispatcherTest, TaskDispatcher_Serial_00300, Function | MediumTest | Level1)
{
    StartAbility(firstAbilityName, bundleNameFirst);
    std::string data = "Dispatcher_" + std::to_string((int)TestFunc::SERIAL) + "_2";
    bool result = false;
    for (int i = 0; i < stLevel_.AMSLevel; i++) {
        STAbilityUtil::PublishEvent(g_EVENT_REQU_FIRST, ++testCaseCode, data);
        EXPECT_EQ(0, STAbilityUtil::WaitCompleted(event, g_EVENT_RESP_FIRST, testCaseCode, DELAY));
        data = STAbilityUtil::GetData(event, g_EVENT_RESP_FIRST, testCaseCode);
        result = data.compare("1") == 0;
        EXPECT_TRUE(result);
        if (!result && i > 0) {
            GTEST_LOG_(INFO) << "TaskDispatcher_Serial_00300 : " << i;
            break;
        }
    }
}

/**
 * @tc.number    : TaskDispatcher_Serial_00400
 * @tc.name      : serial sync:parallel group
 * @tc.desc      : two levels task dispatch
 */
HWTEST_F(TaskDispatcherTest, TaskDispatcher_Serial_00400, Function | MediumTest | Level1)
{
    StartAbility(firstAbilityName, bundleNameFirst);
    std::string data = "Dispatcher_" + std::to_string((int)TestFunc::SERIAL) + "_3";
    bool result = false;
    for (int i = 0; i < stLevel_.AMSLevel; i++) {
        STAbilityUtil::PublishEvent(g_EVENT_REQU_FIRST, ++testCaseCode, data);
        EXPECT_EQ(0, STAbilityUtil::WaitCompleted(event, g_EVENT_RESP_FIRST, testCaseCode, DELAY));
        data = STAbilityUtil::GetData(event, g_EVENT_RESP_FIRST, testCaseCode);
        result = data.compare("1") == 0;
        EXPECT_TRUE(result);
        if (!result && i > 0) {
            GTEST_LOG_(INFO) << "TaskDispatcher_Serial_00400 : " << i;
            break;
        }
    }
}

/**
 * @tc.number    : TaskDispatcher_Serial_00500
 * @tc.name      : serial sync:parallel group wait
 * @tc.desc      : two levels task dispatch
 */
HWTEST_F(TaskDispatcherTest, TaskDispatcher_Serial_00500, Function | MediumTest | Level1)
{
    StartAbility(firstAbilityName, bundleNameFirst);
    std::string data = "Dispatcher_" + std::to_string((int)TestFunc::SERIAL) + "_4";
    bool result = false;
    for (int i = 0; i < stLevel_.AMSLevel; i++) {
        STAbilityUtil::PublishEvent(g_EVENT_REQU_FIRST, ++testCaseCode, data);
        EXPECT_EQ(0, STAbilityUtil::WaitCompleted(event, g_EVENT_RESP_FIRST, testCaseCode, DELAY));
        data = STAbilityUtil::GetData(event, g_EVENT_RESP_FIRST, testCaseCode);
        result = data.compare("1") == 0;
        EXPECT_TRUE(result);
        if (!result && i > 0) {
            GTEST_LOG_(INFO) << "TaskDispatcher_Serial_00500 : " << i;
            break;
        }
    }
}

/**
 * @tc.number    : TaskDispatcher_Serial_00600
 * @tc.name      : serial sync:parallel group notify
 * @tc.desc      : two levels task dispatch
 */
HWTEST_F(TaskDispatcherTest, TaskDispatcher_Serial_00600, Function | MediumTest | Level1)
{
    StartAbility(firstAbilityName, bundleNameFirst);
    std::string data = "Dispatcher_" + std::to_string((int)TestFunc::SERIAL) + "_5";
    bool result = false;
    for (int i = 0; i < stLevel_.AMSLevel; i++) {
        STAbilityUtil::PublishEvent(g_EVENT_REQU_FIRST, ++testCaseCode, data);
        EXPECT_EQ(0, STAbilityUtil::WaitCompleted(event, g_EVENT_RESP_FIRST, testCaseCode, DELAY));
        data = STAbilityUtil::GetData(event, g_EVENT_RESP_FIRST, testCaseCode);
        result = data.compare("1") == 0;
        EXPECT_TRUE(result);
        if (!result && i > 0) {
            GTEST_LOG_(INFO) << "TaskDispatcher_Serial_00600 : " << i;
            break;
        }
    }
}

/**
 * @tc.number    : TaskDispatcher_Serial_00700
 * @tc.name      : serial sync:parallel sync barrier
 * @tc.desc      : two levels task dispatch
 */
HWTEST_F(TaskDispatcherTest, TaskDispatcher_Serial_00700, Function | MediumTest | Level1)
{
    StartAbility(firstAbilityName, bundleNameFirst);
    std::string data = "Dispatcher_" + std::to_string((int)TestFunc::SERIAL) + "_6";
    bool result = false;
    for (int i = 0; i < stLevel_.AMSLevel; i++) {
        STAbilityUtil::PublishEvent(g_EVENT_REQU_FIRST, ++testCaseCode, data);
        EXPECT_EQ(0, STAbilityUtil::WaitCompleted(event, g_EVENT_RESP_FIRST, testCaseCode, DELAY));
        data = STAbilityUtil::GetData(event, g_EVENT_RESP_FIRST, testCaseCode);
        result = data.compare("1") == 0;
        EXPECT_TRUE(result);
        if (!result && i > 0) {
            GTEST_LOG_(INFO) << "TaskDispatcher_Serial_00700 : " << i;
            break;
        }
    }
}

/**
 * @tc.number    : TaskDispatcher_Serial_00800
 * @tc.name      : serial sync:parallel async barrier
 * @tc.desc      : two levels task dispatch
 */
HWTEST_F(TaskDispatcherTest, TaskDispatcher_Serial_00800, Function | MediumTest | Level1)
{
    StartAbility(firstAbilityName, bundleNameFirst);
    std::string data = "Dispatcher_" + std::to_string((int)TestFunc::SERIAL) + "_7";
    bool result = false;
    for (int i = 0; i < stLevel_.AMSLevel; i++) {
        STAbilityUtil::PublishEvent(g_EVENT_REQU_FIRST, ++testCaseCode, data);
        EXPECT_EQ(0, STAbilityUtil::WaitCompleted(event, g_EVENT_RESP_FIRST, testCaseCode, DELAY));
        data = STAbilityUtil::GetData(event, g_EVENT_RESP_FIRST, testCaseCode);
        result = data.compare("1") == 0;
        EXPECT_TRUE(result);
        if (!result && i > 0) {
            GTEST_LOG_(INFO) << "TaskDispatcher_Serial_00800 : " << i;
            break;
        }
    }
}

/**
 * @tc.number    : TaskDispatcher_Serial_00900
 * @tc.name      : serial sync:parallel apply
 * @tc.desc      : two levels task dispatch
 */
HWTEST_F(TaskDispatcherTest, TaskDispatcher_Serial_00900, Function | MediumTest | Level1)
{
    StartAbility(firstAbilityName, bundleNameFirst);
    std::string data = "Dispatcher_" + std::to_string((int)TestFunc::SERIAL) + "_8";
    bool result = false;
    for (int i = 0; i < stLevel_.AMSLevel; i++) {
        STAbilityUtil::PublishEvent(g_EVENT_REQU_FIRST, ++testCaseCode, data);
        EXPECT_EQ(0, STAbilityUtil::WaitCompleted(event, g_EVENT_RESP_FIRST, testCaseCode, DELAY));
        data = STAbilityUtil::GetData(event, g_EVENT_RESP_FIRST, testCaseCode);
        result = data.compare("1") == 0;
        EXPECT_TRUE(result);
        if (!result && i > 0) {
            GTEST_LOG_(INFO) << "TaskDispatcher_Serial_00900 : " << i;
            break;
        }
    }
}

/**
 * @tc.number    : TaskDispatcher_Serial_01000
 * @tc.name      : serial sync:serial sync
 * @tc.desc      : two levels task dispatch
 */
HWTEST_F(TaskDispatcherTest, TaskDispatcher_Serial_01000, Function | MediumTest | Level1)
{
    StartAbility(firstAbilityName, bundleNameFirst);
    std::string data = "Dispatcher_" + std::to_string((int)TestFunc::SERIAL) + "_9";
    bool result = false;
    for (int i = 0; i < stLevel_.AMSLevel; i++) {
        STAbilityUtil::PublishEvent(g_EVENT_REQU_FIRST, ++testCaseCode, data);
        EXPECT_EQ(0, STAbilityUtil::WaitCompleted(event, g_EVENT_RESP_FIRST, testCaseCode, DELAY));
        data = STAbilityUtil::GetData(event, g_EVENT_RESP_FIRST, testCaseCode);
        result = data.compare("1") == 0;
        EXPECT_TRUE(result);
        if (!result && i > 0) {
            GTEST_LOG_(INFO) << "TaskDispatcher_Serial_01000 : " << i;
            break;
        }
    }
}

/**
 * @tc.number    : TaskDispatcher_Serial_01100
 * @tc.name      : serial sync:serial async
 * @tc.desc      : two levels task dispatch
 */
HWTEST_F(TaskDispatcherTest, TaskDispatcher_Serial_01100, Function | MediumTest | Level1)
{
    StartAbility(firstAbilityName, bundleNameFirst);
    std::string data = "Dispatcher_" + std::to_string((int)TestFunc::SERIAL) + "_10";
    bool result = false;
    for (int i = 0; i < stLevel_.AMSLevel; i++) {
        STAbilityUtil::PublishEvent(g_EVENT_REQU_FIRST, ++testCaseCode, data);
        EXPECT_EQ(0, STAbilityUtil::WaitCompleted(event, g_EVENT_RESP_FIRST, testCaseCode, DELAY));
        data = STAbilityUtil::GetData(event, g_EVENT_RESP_FIRST, testCaseCode);
        result = data.compare("1") == 0;
        EXPECT_TRUE(result);
        if (!result && i > 0) {
            GTEST_LOG_(INFO) << "TaskDispatcher_Serial_01100 : " << i;
            break;
        }
    }
}

/**
 * @tc.number    : TaskDispatcher_Serial_01200
 * @tc.name      : serial sync:serial delay
 * @tc.desc      : two levels task dispatch
 */
HWTEST_F(TaskDispatcherTest, TaskDispatcher_Serial_01200, Function | MediumTest | Level1)
{
    StartAbility(firstAbilityName, bundleNameFirst);
    std::string data = "Dispatcher_" + std::to_string((int)TestFunc::SERIAL) + "_11";
    bool result = false;
    for (int i = 0; i < stLevel_.AMSLevel; i++) {
        STAbilityUtil::PublishEvent(g_EVENT_REQU_FIRST, ++testCaseCode, data);
        EXPECT_EQ(0, STAbilityUtil::WaitCompleted(event, g_EVENT_RESP_FIRST, testCaseCode, DELAY));
        data = STAbilityUtil::GetData(event, g_EVENT_RESP_FIRST, testCaseCode);
        result = data.compare("1") == 0;
        EXPECT_TRUE(result);
        if (!result && i > 0) {
            GTEST_LOG_(INFO) << "TaskDispatcher_Serial_01200 : " << i;
            break;
        }
    }
}

/**
 * @tc.number    : TaskDispatcher_Serial_01300
 * @tc.name      : serial sync:serial apply
 * @tc.desc      : two levels task dispatch
 */
HWTEST_F(TaskDispatcherTest, TaskDispatcher_Serial_01300, Function | MediumTest | Level1)
{
    StartAbility(firstAbilityName, bundleNameFirst);
    std::string data = "Dispatcher_" + std::to_string((int)TestFunc::SERIAL) + "_12";
    bool result = false;
    for (int i = 0; i < stLevel_.AMSLevel; i++) {
        STAbilityUtil::PublishEvent(g_EVENT_REQU_FIRST, ++testCaseCode, data);
        EXPECT_EQ(0, STAbilityUtil::WaitCompleted(event, g_EVENT_RESP_FIRST, testCaseCode, DELAY));
        data = STAbilityUtil::GetData(event, g_EVENT_RESP_FIRST, testCaseCode);
        result = data.compare("1") == 0;
        EXPECT_TRUE(result);
        if (!result && i > 0) {
            GTEST_LOG_(INFO) << "TaskDispatcher_Serial_01300 : " << i;
            break;
        }
    }
}

/**
 * @tc.number    : TaskDispatcher_Serial_01400
 * @tc.name      : serial async:parallel sync
 * @tc.desc      : two levels task dispatch
 */
HWTEST_F(TaskDispatcherTest, TaskDispatcher_Serial_01400, Function | MediumTest | Level1)
{
    StartAbility(firstAbilityName, bundleNameFirst);
    std::string data = "Dispatcher_" + std::to_string((int)TestFunc::SERIAL) + "_13";
    bool result = false;
    for (int i = 0; i < stLevel_.AMSLevel; i++) {
        STAbilityUtil::PublishEvent(g_EVENT_REQU_FIRST, ++testCaseCode, data);
        EXPECT_EQ(0, STAbilityUtil::WaitCompleted(event, g_EVENT_RESP_FIRST, testCaseCode, DELAY));
        data = STAbilityUtil::GetData(event, g_EVENT_RESP_FIRST, testCaseCode);
        result = data.compare("1") == 0;
        EXPECT_TRUE(result);
        if (!result && i > 0) {
            GTEST_LOG_(INFO) << "TaskDispatcher_Serial_01400 : " << i;
            break;
        }
    }
}

/**
 * @tc.number    : TaskDispatcher_Serial_01500
 * @tc.name      : serial async:parallel async
 * @tc.desc      : two levels task dispatch
 */
HWTEST_F(TaskDispatcherTest, TaskDispatcher_Serial_01500, Function | MediumTest | Level1)
{
    StartAbility(firstAbilityName, bundleNameFirst);
    std::string data = "Dispatcher_" + std::to_string((int)TestFunc::SERIAL) + "_14";
    bool result = false;
    for (int i = 0; i < stLevel_.AMSLevel; i++) {
        STAbilityUtil::PublishEvent(g_EVENT_REQU_FIRST, ++testCaseCode, data);
        EXPECT_EQ(0, STAbilityUtil::WaitCompleted(event, g_EVENT_RESP_FIRST, testCaseCode, DELAY));
        data = STAbilityUtil::GetData(event, g_EVENT_RESP_FIRST, testCaseCode);
        result = data.compare("1") == 0;
        EXPECT_TRUE(result);
        if (!result && i > 0) {
            GTEST_LOG_(INFO) << "TaskDispatcher_Serial_01500 : " << i;
            break;
        }
    }
}

/**
 * @tc.number    : TaskDispatcher_Serial_01600
 * @tc.name      : serial async:parallel delay
 * @tc.desc      : two levels task dispatch
 */
HWTEST_F(TaskDispatcherTest, TaskDispatcher_Serial_01600, Function | MediumTest | Level1)
{
    StartAbility(firstAbilityName, bundleNameFirst);
    std::string data = "Dispatcher_" + std::to_string((int)TestFunc::SERIAL) + "_15";
    bool result = false;
    for (int i = 0; i < stLevel_.AMSLevel; i++) {
        STAbilityUtil::PublishEvent(g_EVENT_REQU_FIRST, ++testCaseCode, data);
        EXPECT_EQ(0, STAbilityUtil::WaitCompleted(event, g_EVENT_RESP_FIRST, testCaseCode, DELAY));
        data = STAbilityUtil::GetData(event, g_EVENT_RESP_FIRST, testCaseCode);
        result = data.compare("1") == 0;
        EXPECT_TRUE(result);
        if (!result && i > 0) {
            GTEST_LOG_(INFO) << "TaskDispatcher_Serial_01600 : " << i;
            break;
        }
    }
}

/**
 * @tc.number    : TaskDispatcher_Serial_01700
 * @tc.name      : serial async:parallel group
 * @tc.desc      : two levels task dispatch
 */
HWTEST_F(TaskDispatcherTest, TaskDispatcher_Serial_01700, Function | MediumTest | Level1)
{
    StartAbility(firstAbilityName, bundleNameFirst);
    std::string data = "Dispatcher_" + std::to_string((int)TestFunc::SERIAL) + "_16";
    bool result = false;
    for (int i = 0; i < stLevel_.AMSLevel; i++) {
        STAbilityUtil::PublishEvent(g_EVENT_REQU_FIRST, ++testCaseCode, data);
        EXPECT_EQ(0, STAbilityUtil::WaitCompleted(event, g_EVENT_RESP_FIRST, testCaseCode, DELAY));
        data = STAbilityUtil::GetData(event, g_EVENT_RESP_FIRST, testCaseCode);
        result = data.compare("1") == 0;
        EXPECT_TRUE(result);
        if (!result && i > 0) {
            GTEST_LOG_(INFO) << "TaskDispatcher_Serial_01700 : " << i;
            break;
        }
    }
}

/**
 * @tc.number    : TaskDispatcher_Serial_01800
 * @tc.name      : serial async:parallel group wait
 * @tc.desc      : two levels task dispatch
 */
HWTEST_F(TaskDispatcherTest, TaskDispatcher_Serial_01800, Function | MediumTest | Level1)
{
    StartAbility(firstAbilityName, bundleNameFirst);
    std::string data = "Dispatcher_" + std::to_string((int)TestFunc::SERIAL) + "_17";
    bool result = false;
    for (int i = 0; i < stLevel_.AMSLevel; i++) {
        STAbilityUtil::PublishEvent(g_EVENT_REQU_FIRST, ++testCaseCode, data);
        EXPECT_EQ(0, STAbilityUtil::WaitCompleted(event, g_EVENT_RESP_FIRST, testCaseCode, DELAY));
        data = STAbilityUtil::GetData(event, g_EVENT_RESP_FIRST, testCaseCode);
        result = data.compare("1") == 0;
        EXPECT_TRUE(result);
        if (!result && i > 0) {
            GTEST_LOG_(INFO) << "TaskDispatcher_Serial_01800 : " << i;
            break;
        }
    }
}

/**
 * @tc.number    : TaskDispatcher_Serial_01900
 * @tc.name      : serial async:parallel group notify
 * @tc.desc      : two levels task dispatch
 */
HWTEST_F(TaskDispatcherTest, TaskDispatcher_Serial_01900, Function | MediumTest | Level1)
{
    StartAbility(firstAbilityName, bundleNameFirst);
    std::string data = "Dispatcher_" + std::to_string((int)TestFunc::SERIAL) + "_18";
    bool result = false;
    for (int i = 0; i < stLevel_.AMSLevel; i++) {
        STAbilityUtil::PublishEvent(g_EVENT_REQU_FIRST, ++testCaseCode, data);
        EXPECT_EQ(0, STAbilityUtil::WaitCompleted(event, g_EVENT_RESP_FIRST, testCaseCode, DELAY));
        data = STAbilityUtil::GetData(event, g_EVENT_RESP_FIRST, testCaseCode);
        result = data.compare("1") == 0;
        EXPECT_TRUE(result);
        if (!result && i > 0) {
            GTEST_LOG_(INFO) << "TaskDispatcher_Serial_01900 : " << i;
            break;
        }
    }
}

/**
 * @tc.number    : TaskDispatcher_Serial_02000
 * @tc.name      : serial async:parallel sync barrier
 * @tc.desc      : two levels task dispatch
 */
HWTEST_F(TaskDispatcherTest, TaskDispatcher_Serial_02000, Function | MediumTest | Level1)
{
    StartAbility(firstAbilityName, bundleNameFirst);
    std::string data = "Dispatcher_" + std::to_string((int)TestFunc::SERIAL) + "_19";
    bool result = false;
    for (int i = 0; i < stLevel_.AMSLevel; i++) {
        STAbilityUtil::PublishEvent(g_EVENT_REQU_FIRST, ++testCaseCode, data);
        EXPECT_EQ(0, STAbilityUtil::WaitCompleted(event, g_EVENT_RESP_FIRST, testCaseCode, DELAY));
        data = STAbilityUtil::GetData(event, g_EVENT_RESP_FIRST, testCaseCode);
        result = data.compare("1") == 0;
        EXPECT_TRUE(result);
        if (!result && i > 0) {
            GTEST_LOG_(INFO) << "TaskDispatcher_Serial_02000 : " << i;
            break;
        }
    }
}

/**
 * @tc.number    : TaskDispatcher_Serial_02100
 * @tc.name      : serial async:parallel async barrier
 * @tc.desc      : two levels task dispatch
 */
HWTEST_F(TaskDispatcherTest, TaskDispatcher_Serial_02100, Function | MediumTest | Level1)
{
    StartAbility(firstAbilityName, bundleNameFirst);
    std::string data = "Dispatcher_" + std::to_string((int)TestFunc::SERIAL) + "_20";
    bool result = false;
    for (int i = 0; i < stLevel_.AMSLevel; i++) {
        STAbilityUtil::PublishEvent(g_EVENT_REQU_FIRST, ++testCaseCode, data);
        EXPECT_EQ(0, STAbilityUtil::WaitCompleted(event, g_EVENT_RESP_FIRST, testCaseCode, DELAY));
        data = STAbilityUtil::GetData(event, g_EVENT_RESP_FIRST, testCaseCode);
        result = data.compare("1") == 0;
        EXPECT_TRUE(result);
        if (!result && i > 0) {
            GTEST_LOG_(INFO) << "TaskDispatcher_Serial_02100 : " << i;
            break;
        }
    }
}

/**
 * @tc.number    : TaskDispatcher_Serial_02200
 * @tc.name      : serial async:parallel apply
 * @tc.desc      : two levels task dispatch
 */
HWTEST_F(TaskDispatcherTest, TaskDispatcher_Serial_02200, Function | MediumTest | Level1)
{
    StartAbility(firstAbilityName, bundleNameFirst);
    std::string data = "Dispatcher_" + std::to_string((int)TestFunc::SERIAL) + "_21";
    bool result = false;
    for (int i = 0; i < stLevel_.AMSLevel; i++) {
        STAbilityUtil::PublishEvent(g_EVENT_REQU_FIRST, ++testCaseCode, data);
        EXPECT_EQ(0, STAbilityUtil::WaitCompleted(event, g_EVENT_RESP_FIRST, testCaseCode, DELAY));
        data = STAbilityUtil::GetData(event, g_EVENT_RESP_FIRST, testCaseCode);
        result = data.compare("1") == 0;
        EXPECT_TRUE(result);
        if (!result && i > 0) {
            GTEST_LOG_(INFO) << "TaskDispatcher_Serial_02200 : " << i;
            break;
        }
    }
}

/**
 * @tc.number    : TaskDispatcher_Serial_02300
 * @tc.name      : serial async:serial sync
 * @tc.desc      : two levels task dispatch
 */
HWTEST_F(TaskDispatcherTest, TaskDispatcher_Serial_02300, Function | MediumTest | Level1)
{
    StartAbility(firstAbilityName, bundleNameFirst);
    std::string data = "Dispatcher_" + std::to_string((int)TestFunc::SERIAL) + "_22";
    bool result = false;
    for (int i = 0; i < stLevel_.AMSLevel; i++) {
        STAbilityUtil::PublishEvent(g_EVENT_REQU_FIRST, ++testCaseCode, data);
        EXPECT_EQ(0, STAbilityUtil::WaitCompleted(event, g_EVENT_RESP_FIRST, testCaseCode, DELAY));
        data = STAbilityUtil::GetData(event, g_EVENT_RESP_FIRST, testCaseCode);
        result = data.compare("1") == 0;
        EXPECT_TRUE(result);
        if (!result && i > 0) {
            GTEST_LOG_(INFO) << "TaskDispatcher_Serial_02300 : " << i;
            break;
        }
    }
}

/**
 * @tc.number    : TaskDispatcher_Serial_02400
 * @tc.name      : serial async:serial async
 * @tc.desc      : two levels task dispatch
 */
HWTEST_F(TaskDispatcherTest, TaskDispatcher_Serial_02400, Function | MediumTest | Level1)
{
    StartAbility(firstAbilityName, bundleNameFirst);
    std::string data = "Dispatcher_" + std::to_string((int)TestFunc::SERIAL) + "_23";
    bool result = false;
    for (int i = 0; i < stLevel_.AMSLevel; i++) {
        STAbilityUtil::PublishEvent(g_EVENT_REQU_FIRST, ++testCaseCode, data);
        EXPECT_EQ(0, STAbilityUtil::WaitCompleted(event, g_EVENT_RESP_FIRST, testCaseCode, DELAY));
        data = STAbilityUtil::GetData(event, g_EVENT_RESP_FIRST, testCaseCode);
        result = data.compare("1") == 0;
        EXPECT_TRUE(result);
        if (!result && i > 0) {
            GTEST_LOG_(INFO) << "TaskDispatcher_Serial_02400 : " << i;
            break;
        }
    }
}

/**
 * @tc.number    : TaskDispatcher_Serial_02500
 * @tc.name      : serial async:serial delay
 * @tc.desc      : two levels task dispatch
 */
HWTEST_F(TaskDispatcherTest, TaskDispatcher_Serial_02500, Function | MediumTest | Level1)
{
    StartAbility(firstAbilityName, bundleNameFirst);
    std::string data = "Dispatcher_" + std::to_string((int)TestFunc::SERIAL) + "_24";
    bool result = false;
    for (int i = 0; i < stLevel_.AMSLevel; i++) {
        STAbilityUtil::PublishEvent(g_EVENT_REQU_FIRST, ++testCaseCode, data);
        EXPECT_EQ(0, STAbilityUtil::WaitCompleted(event, g_EVENT_RESP_FIRST, testCaseCode, DELAY));
        data = STAbilityUtil::GetData(event, g_EVENT_RESP_FIRST, testCaseCode);
        result = data.compare("1") == 0;
        EXPECT_TRUE(result);
        if (!result && i > 0) {
            GTEST_LOG_(INFO) << "TaskDispatcher_Serial_02500 : " << i;
            break;
        }
    }
}

/**
 * @tc.number    : TaskDispatcher_Serial_02600
 * @tc.name      : serial async:serial apply
 * @tc.desc      : two levels task dispatch
 */
HWTEST_F(TaskDispatcherTest, TaskDispatcher_Serial_02600, Function | MediumTest | Level1)
{
    StartAbility(firstAbilityName, bundleNameFirst);
    std::string data = "Dispatcher_" + std::to_string((int)TestFunc::SERIAL) + "_25";
    bool result = false;
    for (int i = 0; i < stLevel_.AMSLevel; i++) {
        STAbilityUtil::PublishEvent(g_EVENT_REQU_FIRST, ++testCaseCode, data);
        EXPECT_EQ(0, STAbilityUtil::WaitCompleted(event, g_EVENT_RESP_FIRST, testCaseCode, DELAY));
        data = STAbilityUtil::GetData(event, g_EVENT_RESP_FIRST, testCaseCode);
        result = data.compare("1") == 0;
        EXPECT_TRUE(result);
        if (!result && i > 0) {
            GTEST_LOG_(INFO) << "TaskDispatcher_Serial_02600 : " << i;
            break;
        }
    }
}

/**
 * @tc.number    : TaskDispatcher_Serial_02700
 * @tc.name      : serial delay:parallel sync
 * @tc.desc      : two levels task dispatch
 */
HWTEST_F(TaskDispatcherTest, TaskDispatcher_Serial_02700, Function | MediumTest | Level1)
{
    StartAbility(firstAbilityName, bundleNameFirst);
    std::string data = "Dispatcher_" + std::to_string((int)TestFunc::SERIAL) + "_26";
    bool result = false;
    for (int i = 0; i < stLevel_.AMSLevel; i++) {
        STAbilityUtil::PublishEvent(g_EVENT_REQU_FIRST, ++testCaseCode, data);
        EXPECT_EQ(0, STAbilityUtil::WaitCompleted(event, g_EVENT_RESP_FIRST, testCaseCode, DELAY));
        data = STAbilityUtil::GetData(event, g_EVENT_RESP_FIRST, testCaseCode);
        result = data.compare("1") == 0;
        EXPECT_TRUE(result);
        if (!result && i > 0) {
            GTEST_LOG_(INFO) << "TaskDispatcher_Serial_02700 : " << i;
            break;
        }
    }
}

/**
 * @tc.number    : TaskDispatcher_Serial_02800
 * @tc.name      : serial delay:parallel async
 * @tc.desc      : two levels task dispatch
 */
HWTEST_F(TaskDispatcherTest, TaskDispatcher_Serial_02800, Function | MediumTest | Level1)
{
    StartAbility(firstAbilityName, bundleNameFirst);
    std::string data = "Dispatcher_" + std::to_string((int)TestFunc::SERIAL) + "_27";
    bool result = false;
    for (int i = 0; i < stLevel_.AMSLevel; i++) {
        STAbilityUtil::PublishEvent(g_EVENT_REQU_FIRST, ++testCaseCode, data);
        EXPECT_EQ(0, STAbilityUtil::WaitCompleted(event, g_EVENT_RESP_FIRST, testCaseCode, DELAY));
        data = STAbilityUtil::GetData(event, g_EVENT_RESP_FIRST, testCaseCode);
        result = data.compare("1") == 0;
        EXPECT_TRUE(result);
        if (!result && i > 0) {
            GTEST_LOG_(INFO) << "TaskDispatcher_Serial_02800 : " << i;
            break;
        }
    }
}

/**
 * @tc.number    : TaskDispatcher_Serial_02900
 * @tc.name      : serial delay:parallel delay
 * @tc.desc      : two levels task dispatch
 */
HWTEST_F(TaskDispatcherTest, TaskDispatcher_Serial_02900, Function | MediumTest | Level1)
{
    StartAbility(firstAbilityName, bundleNameFirst);
    std::string data = "Dispatcher_" + std::to_string((int)TestFunc::SERIAL) + "_28";
    bool result = false;
    for (int i = 0; i < stLevel_.AMSLevel; i++) {
        STAbilityUtil::PublishEvent(g_EVENT_REQU_FIRST, ++testCaseCode, data);
        EXPECT_EQ(0, STAbilityUtil::WaitCompleted(event, g_EVENT_RESP_FIRST, testCaseCode, DELAY));
        data = STAbilityUtil::GetData(event, g_EVENT_RESP_FIRST, testCaseCode);
        result = data.compare("1") == 0;
        EXPECT_TRUE(result);
        if (!result && i > 0) {
            GTEST_LOG_(INFO) << "TaskDispatcher_Serial_02900 : " << i;
            break;
        }
    }
}

/**
 * @tc.number    : TaskDispatcher_Serial_03000
 * @tc.name      : serial delay:parallel group
 * @tc.desc      : two levels task dispatch
 */
HWTEST_F(TaskDispatcherTest, TaskDispatcher_Serial_03000, Function | MediumTest | Level1)
{
    StartAbility(firstAbilityName, bundleNameFirst);
    std::string data = "Dispatcher_" + std::to_string((int)TestFunc::SERIAL) + "_29";
    bool result = false;
    for (int i = 0; i < stLevel_.AMSLevel; i++) {
        STAbilityUtil::PublishEvent(g_EVENT_REQU_FIRST, ++testCaseCode, data);
        EXPECT_EQ(0, STAbilityUtil::WaitCompleted(event, g_EVENT_RESP_FIRST, testCaseCode, DELAY));
        data = STAbilityUtil::GetData(event, g_EVENT_RESP_FIRST, testCaseCode);
        result = data.compare("1") == 0;
        EXPECT_TRUE(result);
        if (!result && i > 0) {
            GTEST_LOG_(INFO) << "TaskDispatcher_Serial_03000 : " << i;
            break;
        }
    }
}

/**
 * @tc.number    : TaskDispatcher_Serial_03100
 * @tc.name      : serial delay:parallel group wait
 * @tc.desc      : two levels task dispatch
 */
HWTEST_F(TaskDispatcherTest, TaskDispatcher_Serial_03100, Function | MediumTest | Level1)
{
    StartAbility(firstAbilityName, bundleNameFirst);
    std::string data = "Dispatcher_" + std::to_string((int)TestFunc::SERIAL) + "_30";
    bool result = false;
    for (int i = 0; i < stLevel_.AMSLevel; i++) {
        STAbilityUtil::PublishEvent(g_EVENT_REQU_FIRST, ++testCaseCode, data);
        EXPECT_EQ(0, STAbilityUtil::WaitCompleted(event, g_EVENT_RESP_FIRST, testCaseCode, DELAY));
        data = STAbilityUtil::GetData(event, g_EVENT_RESP_FIRST, testCaseCode);
        result = data.compare("1") == 0;
        EXPECT_TRUE(result);
        if (!result && i > 0) {
            GTEST_LOG_(INFO) << "TaskDispatcher_Serial_03100 : " << i;
            break;
        }
    }
}

/**
 * @tc.number    : TaskDispatcher_Serial_03200
 * @tc.name      : serial delay:parallel group notify
 * @tc.desc      : two levels task dispatch
 */
HWTEST_F(TaskDispatcherTest, TaskDispatcher_Serial_03200, Function | MediumTest | Level1)
{
    StartAbility(firstAbilityName, bundleNameFirst);
    std::string data = "Dispatcher_" + std::to_string((int)TestFunc::SERIAL) + "_31";
    bool result = false;
    for (int i = 0; i < stLevel_.AMSLevel; i++) {
        STAbilityUtil::PublishEvent(g_EVENT_REQU_FIRST, ++testCaseCode, data);
        EXPECT_EQ(0, STAbilityUtil::WaitCompleted(event, g_EVENT_RESP_FIRST, testCaseCode, DELAY));
        data = STAbilityUtil::GetData(event, g_EVENT_RESP_FIRST, testCaseCode);
        result = data.compare("1") == 0;
        EXPECT_TRUE(result);
        if (!result && i > 0) {
            GTEST_LOG_(INFO) << "TaskDispatcher_Serial_03200 : " << i;
            break;
        }
    }
}

/**
 * @tc.number    : TaskDispatcher_Serial_03300
 * @tc.name      : serial delay:parallel sync barrier
 * @tc.desc      : two levels task dispatch
 */
HWTEST_F(TaskDispatcherTest, TaskDispatcher_Serial_03300, Function | MediumTest | Level1)
{
    StartAbility(firstAbilityName, bundleNameFirst);
    std::string data = "Dispatcher_" + std::to_string((int)TestFunc::SERIAL) + "_32";
    bool result = false;
    for (int i = 0; i < stLevel_.AMSLevel; i++) {
        STAbilityUtil::PublishEvent(g_EVENT_REQU_FIRST, ++testCaseCode, data);
        EXPECT_EQ(0, STAbilityUtil::WaitCompleted(event, g_EVENT_RESP_FIRST, testCaseCode, DELAY));
        data = STAbilityUtil::GetData(event, g_EVENT_RESP_FIRST, testCaseCode);
        result = data.compare("1") == 0;
        EXPECT_TRUE(result);
        if (!result && i > 0) {
            GTEST_LOG_(INFO) << "TaskDispatcher_Serial_03300 : " << i;
            break;
        }
    }
}

/**
 * @tc.number    : TaskDispatcher_Serial_03400
 * @tc.name      : serial delay:parallel async barrier
 * @tc.desc      : two levels task dispatch
 */
HWTEST_F(TaskDispatcherTest, TaskDispatcher_Serial_03400, Function | MediumTest | Level1)
{
    StartAbility(firstAbilityName, bundleNameFirst);
    std::string data = "Dispatcher_" + std::to_string((int)TestFunc::SERIAL) + "_33";
    bool result = false;
    for (int i = 0; i < stLevel_.AMSLevel; i++) {
        STAbilityUtil::PublishEvent(g_EVENT_REQU_FIRST, ++testCaseCode, data);
        EXPECT_EQ(0, STAbilityUtil::WaitCompleted(event, g_EVENT_RESP_FIRST, testCaseCode, DELAY));
        data = STAbilityUtil::GetData(event, g_EVENT_RESP_FIRST, testCaseCode);
        result = data.compare("1") == 0;
        EXPECT_TRUE(result);
        if (!result && i > 0) {
            GTEST_LOG_(INFO) << "TaskDispatcher_Serial_03400 : " << i;
            break;
        }
    }
}

/**
 * @tc.number    : TaskDispatcher_Serial_03500
 * @tc.name      : serial delay:parallel apply
 * @tc.desc      : two levels task dispatch
 */
HWTEST_F(TaskDispatcherTest, TaskDispatcher_Serial_03500, Function | MediumTest | Level1)
{
    StartAbility(firstAbilityName, bundleNameFirst);
    std::string data = "Dispatcher_" + std::to_string((int)TestFunc::SERIAL) + "_34";
    bool result = false;
    for (int i = 0; i < stLevel_.AMSLevel; i++) {
        STAbilityUtil::PublishEvent(g_EVENT_REQU_FIRST, ++testCaseCode, data);
        EXPECT_EQ(0, STAbilityUtil::WaitCompleted(event, g_EVENT_RESP_FIRST, testCaseCode, DELAY));
        data = STAbilityUtil::GetData(event, g_EVENT_RESP_FIRST, testCaseCode);
        result = data.compare("1") == 0;
        EXPECT_TRUE(result);
        if (!result && i > 0) {
            GTEST_LOG_(INFO) << "TaskDispatcher_Serial_03500 : " << i;
            break;
        }
    }
}

/**
 * @tc.number    : TaskDispatcher_Serial_03600
 * @tc.name      : serial delay:serial sync
 * @tc.desc      : two levels task dispatch
 */
HWTEST_F(TaskDispatcherTest, TaskDispatcher_Serial_03600, Function | MediumTest | Level1)
{
    StartAbility(firstAbilityName, bundleNameFirst);
    std::string data = "Dispatcher_" + std::to_string((int)TestFunc::SERIAL) + "_35";
    bool result = false;
    for (int i = 0; i < stLevel_.AMSLevel; i++) {
        STAbilityUtil::PublishEvent(g_EVENT_REQU_FIRST, ++testCaseCode, data);
        EXPECT_EQ(0, STAbilityUtil::WaitCompleted(event, g_EVENT_RESP_FIRST, testCaseCode, DELAY));
        data = STAbilityUtil::GetData(event, g_EVENT_RESP_FIRST, testCaseCode);
        result = data.compare("1") == 0;
        EXPECT_TRUE(result);
        if (!result && i > 0) {
            GTEST_LOG_(INFO) << "TaskDispatcher_Serial_03600 : " << i;
            break;
        }
    }
}

/**
 * @tc.number    : TaskDispatcher_Serial_03700
 * @tc.name      : serial delay:serial async
 * @tc.desc      : two levels task dispatch
 */
HWTEST_F(TaskDispatcherTest, TaskDispatcher_Serial_03700, Function | MediumTest | Level1)
{
    StartAbility(firstAbilityName, bundleNameFirst);
    std::string data = "Dispatcher_" + std::to_string((int)TestFunc::SERIAL) + "_36";
    bool result = false;
    for (int i = 0; i < stLevel_.AMSLevel; i++) {
        STAbilityUtil::PublishEvent(g_EVENT_REQU_FIRST, ++testCaseCode, data);
        EXPECT_EQ(0, STAbilityUtil::WaitCompleted(event, g_EVENT_RESP_FIRST, testCaseCode, DELAY));
        data = STAbilityUtil::GetData(event, g_EVENT_RESP_FIRST, testCaseCode);
        result = data.compare("1") == 0;
        EXPECT_TRUE(result);
        if (!result && i > 0) {
            GTEST_LOG_(INFO) << "TaskDispatcher_Serial_03700 : " << i;
            break;
        }
    }
}

/**
 * @tc.number    : TaskDispatcher_Serial_03800
 * @tc.name      : serial delay:serial delay
 * @tc.desc      : two levels task dispatch
 */
HWTEST_F(TaskDispatcherTest, TaskDispatcher_Serial_03800, Function | MediumTest | Level1)
{
    StartAbility(firstAbilityName, bundleNameFirst);
    std::string data = "Dispatcher_" + std::to_string((int)TestFunc::SERIAL) + "_37";
    bool result = false;
    for (int i = 0; i < stLevel_.AMSLevel; i++) {
        STAbilityUtil::PublishEvent(g_EVENT_REQU_FIRST, ++testCaseCode, data);
        EXPECT_EQ(0, STAbilityUtil::WaitCompleted(event, g_EVENT_RESP_FIRST, testCaseCode, DELAY));
        data = STAbilityUtil::GetData(event, g_EVENT_RESP_FIRST, testCaseCode);
        result = data.compare("1") == 0;
        EXPECT_TRUE(result);
        if (!result && i > 0) {
            GTEST_LOG_(INFO) << "TaskDispatcher_Serial_03800 : " << i;
            break;
        }
    }
}

/**
 * @tc.number    : TaskDispatcher_Serial_03900
 * @tc.name      : serial delay:serial apply
 * @tc.desc      : two levels task dispatch
 */
HWTEST_F(TaskDispatcherTest, TaskDispatcher_Serial_03900, Function | MediumTest | Level1)
{
    StartAbility(firstAbilityName, bundleNameFirst);
    std::string data = "Dispatcher_" + std::to_string((int)TestFunc::SERIAL) + "_38";
    bool result = false;
    for (int i = 0; i < stLevel_.AMSLevel; i++) {
        STAbilityUtil::PublishEvent(g_EVENT_REQU_FIRST, ++testCaseCode, data);
        EXPECT_EQ(0, STAbilityUtil::WaitCompleted(event, g_EVENT_RESP_FIRST, testCaseCode, DELAY));
        data = STAbilityUtil::GetData(event, g_EVENT_RESP_FIRST, testCaseCode);
        result = data.compare("1") == 0;
        EXPECT_TRUE(result);
        if (!result && i > 0) {
            GTEST_LOG_(INFO) << "TaskDispatcher_Serial_03900 : " << i;
            break;
        }
    }
}

/**
 * @tc.number    : TaskDispatcher_Serial_04000
 * @tc.name      : serial apply:parallel sync
 * @tc.desc      : two levels task dispatch
 */
HWTEST_F(TaskDispatcherTest, TaskDispatcher_Serial_04000, Function | MediumTest | Level1)
{
    StartAbility(firstAbilityName, bundleNameFirst);
    std::string data = "Dispatcher_" + std::to_string((int)TestFunc::SERIAL) + "_39";
    bool result = false;
    for (int i = 0; i < stLevel_.AMSLevel; i++) {
        STAbilityUtil::PublishEvent(g_EVENT_REQU_FIRST, ++testCaseCode, data);
        EXPECT_EQ(0, STAbilityUtil::WaitCompleted(event, g_EVENT_RESP_FIRST, testCaseCode, DELAY));
        data = STAbilityUtil::GetData(event, g_EVENT_RESP_FIRST, testCaseCode);
        result = data.compare("1") == 0;
        EXPECT_TRUE(result);
        if (!result && i > 0) {
            GTEST_LOG_(INFO) << "TaskDispatcher_Serial_04000 : " << i;
            break;
        }
    }
}

/**
 * @tc.number    : TaskDispatcher_Serial_04100
 * @tc.name      : serial apply:parallel async
 * @tc.desc      : two levels task dispatch
 */
HWTEST_F(TaskDispatcherTest, TaskDispatcher_Serial_04100, Function | MediumTest | Level1)
{
    StartAbility(firstAbilityName, bundleNameFirst);
    std::string data = "Dispatcher_" + std::to_string((int)TestFunc::SERIAL) + "_40";
    bool result = false;
    for (int i = 0; i < stLevel_.AMSLevel; i++) {
        STAbilityUtil::PublishEvent(g_EVENT_REQU_FIRST, ++testCaseCode, data);
        EXPECT_EQ(0, STAbilityUtil::WaitCompleted(event, g_EVENT_RESP_FIRST, testCaseCode, DELAY));
        data = STAbilityUtil::GetData(event, g_EVENT_RESP_FIRST, testCaseCode);
        result = data.compare("1") == 0;
        EXPECT_TRUE(result);
        if (!result && i > 0) {
            GTEST_LOG_(INFO) << "TaskDispatcher_Serial_04100 : " << i;
            break;
        }
    }
}

/**
 * @tc.number    : TaskDispatcher_Serial_04200
 * @tc.name      : serial apply:parallel delay
 * @tc.desc      : two levels task dispatch
 */
HWTEST_F(TaskDispatcherTest, TaskDispatcher_Serial_04200, Function | MediumTest | Level1)
{
    StartAbility(firstAbilityName, bundleNameFirst);
    std::string data = "Dispatcher_" + std::to_string((int)TestFunc::SERIAL) + "_41";
    bool result = false;
    for (int i = 0; i < stLevel_.AMSLevel; i++) {
        STAbilityUtil::PublishEvent(g_EVENT_REQU_FIRST, ++testCaseCode, data);
        EXPECT_EQ(0, STAbilityUtil::WaitCompleted(event, g_EVENT_RESP_FIRST, testCaseCode, DELAY));
        data = STAbilityUtil::GetData(event, g_EVENT_RESP_FIRST, testCaseCode);
        result = data.compare("1") == 0;
        EXPECT_TRUE(result);
        if (!result && i > 0) {
            GTEST_LOG_(INFO) << "TaskDispatcher_Serial_04200 : " << i;
            break;
        }
    }
}

/**
 * @tc.number    : TaskDispatcher_Serial_04300
 * @tc.name      : serial apply:parallel group
 * @tc.desc      : two levels task dispatch
 */
HWTEST_F(TaskDispatcherTest, TaskDispatcher_Serial_04300, Function | MediumTest | Level1)
{
    StartAbility(firstAbilityName, bundleNameFirst);
    std::string data = "Dispatcher_" + std::to_string((int)TestFunc::SERIAL) + "_42";
    bool result = false;
    for (int i = 0; i < stLevel_.AMSLevel; i++) {
        STAbilityUtil::PublishEvent(g_EVENT_REQU_FIRST, ++testCaseCode, data);
        EXPECT_EQ(0, STAbilityUtil::WaitCompleted(event, g_EVENT_RESP_FIRST, testCaseCode, DELAY));
        data = STAbilityUtil::GetData(event, g_EVENT_RESP_FIRST, testCaseCode);
        result = data.compare("1") == 0;
        EXPECT_TRUE(result);
        if (!result && i > 0) {
            GTEST_LOG_(INFO) << "TaskDispatcher_Serial_04300 : " << i;
            break;
        }
    }
}

/**
 * @tc.number    : TaskDispatcher_Serial_04400
 * @tc.name      : serial apply:parallel group wait
 * @tc.desc      : two levels task dispatch
 */
HWTEST_F(TaskDispatcherTest, TaskDispatcher_Serial_04400, Function | MediumTest | Level1)
{
    StartAbility(firstAbilityName, bundleNameFirst);
    std::string data = "Dispatcher_" + std::to_string((int)TestFunc::SERIAL) + "_43";
    bool result = false;
    for (int i = 0; i < stLevel_.AMSLevel; i++) {
        STAbilityUtil::PublishEvent(g_EVENT_REQU_FIRST, ++testCaseCode, data);
        EXPECT_EQ(0, STAbilityUtil::WaitCompleted(event, g_EVENT_RESP_FIRST, testCaseCode, DELAY));
        data = STAbilityUtil::GetData(event, g_EVENT_RESP_FIRST, testCaseCode);
        result = data.compare("1") == 0;
        EXPECT_TRUE(result);
        if (!result && i > 0) {
            GTEST_LOG_(INFO) << "TaskDispatcher_Serial_04400 : " << i;
            break;
        }
    }
}

/**
 * @tc.number    : TaskDispatcher_Serial_04500
 * @tc.name      : serial apply:parallel group notify
 * @tc.desc      : two levels task dispatch
 */
HWTEST_F(TaskDispatcherTest, TaskDispatcher_Serial_04500, Function | MediumTest | Level1)
{
    StartAbility(firstAbilityName, bundleNameFirst);
    std::string data = "Dispatcher_" + std::to_string((int)TestFunc::SERIAL) + "_44";
    bool result = false;
    for (int i = 0; i < stLevel_.AMSLevel; i++) {
        STAbilityUtil::PublishEvent(g_EVENT_REQU_FIRST, ++testCaseCode, data);
        EXPECT_EQ(0, STAbilityUtil::WaitCompleted(event, g_EVENT_RESP_FIRST, testCaseCode, DELAY));
        data = STAbilityUtil::GetData(event, g_EVENT_RESP_FIRST, testCaseCode);
        result = data.compare("1") == 0;
        EXPECT_TRUE(result);
        if (!result && i > 0) {
            GTEST_LOG_(INFO) << "TaskDispatcher_Serial_04500 : " << i;
            break;
        }
    }
}

/**
 * @tc.number    : TaskDispatcher_Serial_04600
 * @tc.name      : serial apply:parallel sync barrier
 * @tc.desc      : two levels task dispatch
 */
HWTEST_F(TaskDispatcherTest, TaskDispatcher_Serial_04600, Function | MediumTest | Level1)
{
    StartAbility(firstAbilityName, bundleNameFirst);
    std::string data = "Dispatcher_" + std::to_string((int)TestFunc::SERIAL) + "_45";
    bool result = false;
    for (int i = 0; i < stLevel_.AMSLevel; i++) {
        STAbilityUtil::PublishEvent(g_EVENT_REQU_FIRST, ++testCaseCode, data);
        EXPECT_EQ(0, STAbilityUtil::WaitCompleted(event, g_EVENT_RESP_FIRST, testCaseCode, DELAY));
        data = STAbilityUtil::GetData(event, g_EVENT_RESP_FIRST, testCaseCode);
        result = data.compare("1") == 0;
        EXPECT_TRUE(result);
        if (!result && i > 0) {
            GTEST_LOG_(INFO) << "TaskDispatcher_Serial_04600 : " << i;
            break;
        }
    }
}

/**
 * @tc.number    : TaskDispatcher_Serial_04700
 * @tc.name      : serial apply:parallel async barrier
 * @tc.desc      : two levels task dispatch
 */
HWTEST_F(TaskDispatcherTest, TaskDispatcher_Serial_04700, Function | MediumTest | Level1)
{
    StartAbility(firstAbilityName, bundleNameFirst);
    std::string data = "Dispatcher_" + std::to_string((int)TestFunc::SERIAL) + "_46";
    bool result = false;
    for (int i = 0; i < stLevel_.AMSLevel; i++) {
        STAbilityUtil::PublishEvent(g_EVENT_REQU_FIRST, ++testCaseCode, data);
        EXPECT_EQ(0, STAbilityUtil::WaitCompleted(event, g_EVENT_RESP_FIRST, testCaseCode, DELAY));
        data = STAbilityUtil::GetData(event, g_EVENT_RESP_FIRST, testCaseCode);
        result = data.compare("1") == 0;
        EXPECT_TRUE(result);
        if (!result && i > 0) {
            GTEST_LOG_(INFO) << "TaskDispatcher_Serial_04700 : " << i;
            break;
        }
    }
}

/**
 * @tc.number    : TaskDispatcher_Serial_04800
 * @tc.name      : serial apply:parallel apply
 * @tc.desc      : two levels task dispatch
 */
HWTEST_F(TaskDispatcherTest, TaskDispatcher_Serial_04800, Function | MediumTest | Level1)
{
    StartAbility(firstAbilityName, bundleNameFirst);
    std::string data = "Dispatcher_" + std::to_string((int)TestFunc::SERIAL) + "_47";
    bool result = false;
    for (int i = 0; i < stLevel_.AMSLevel; i++) {
        STAbilityUtil::PublishEvent(g_EVENT_REQU_FIRST, ++testCaseCode, data);
        EXPECT_EQ(0, STAbilityUtil::WaitCompleted(event, g_EVENT_RESP_FIRST, testCaseCode, DELAY));
        data = STAbilityUtil::GetData(event, g_EVENT_RESP_FIRST, testCaseCode);
        result = data.compare("1") == 0;
        EXPECT_TRUE(result);
        if (!result && i > 0) {
            GTEST_LOG_(INFO) << "TaskDispatcher_Serial_04800 : " << i;
            break;
        }
    }
}

/**
 * @tc.number    : TaskDispatcher_Serial_04900
 * @tc.name      : serial apply:serial sync
 * @tc.desc      : two levels task dispatch
 */
HWTEST_F(TaskDispatcherTest, TaskDispatcher_Serial_04900, Function | MediumTest | Level1)
{
    StartAbility(firstAbilityName, bundleNameFirst);
    std::string data = "Dispatcher_" + std::to_string((int)TestFunc::SERIAL) + "_48";
    bool result = false;
    for (int i = 0; i < stLevel_.AMSLevel; i++) {
        STAbilityUtil::PublishEvent(g_EVENT_REQU_FIRST, ++testCaseCode, data);
        EXPECT_EQ(0, STAbilityUtil::WaitCompleted(event, g_EVENT_RESP_FIRST, testCaseCode, DELAY));
        data = STAbilityUtil::GetData(event, g_EVENT_RESP_FIRST, testCaseCode);
        result = data.compare("1") == 0;
        EXPECT_TRUE(result);
        if (!result && i > 0) {
            GTEST_LOG_(INFO) << "TaskDispatcher_Serial_04900 : " << i;
            break;
        }
    }
}

/**
 * @tc.number    : TaskDispatcher_Serial_05000
 * @tc.name      : serial apply:serial async
 * @tc.desc      : two levels task dispatch
 */
HWTEST_F(TaskDispatcherTest, TaskDispatcher_Serial_05000, Function | MediumTest | Level1)
{
    StartAbility(firstAbilityName, bundleNameFirst);
    std::string data = "Dispatcher_" + std::to_string((int)TestFunc::SERIAL) + "_49";
    bool result = false;
    for (int i = 0; i < stLevel_.AMSLevel; i++) {
        STAbilityUtil::PublishEvent(g_EVENT_REQU_FIRST, ++testCaseCode, data);
        EXPECT_EQ(0, STAbilityUtil::WaitCompleted(event, g_EVENT_RESP_FIRST, testCaseCode, DELAY));
        data = STAbilityUtil::GetData(event, g_EVENT_RESP_FIRST, testCaseCode);
        result = data.compare("1") == 0;
        EXPECT_TRUE(result);
        if (!result && i > 0) {
            GTEST_LOG_(INFO) << "TaskDispatcher_Serial_05000 : " << i;
            break;
        }
    }
}

/**
 * @tc.number    : TaskDispatcher_Serial_05100
 * @tc.name      : serial apply:serial delay
 * @tc.desc      : two levels task dispatch
 */
HWTEST_F(TaskDispatcherTest, TaskDispatcher_Serial_05100, Function | MediumTest | Level1)
{
    StartAbility(firstAbilityName, bundleNameFirst);
    std::string data = "Dispatcher_" + std::to_string((int)TestFunc::SERIAL) + "_50";
    bool result = false;
    for (int i = 0; i < stLevel_.AMSLevel; i++) {
        STAbilityUtil::PublishEvent(g_EVENT_REQU_FIRST, ++testCaseCode, data);
        EXPECT_EQ(0, STAbilityUtil::WaitCompleted(event, g_EVENT_RESP_FIRST, testCaseCode, DELAY));
        data = STAbilityUtil::GetData(event, g_EVENT_RESP_FIRST, testCaseCode);
        result = data.compare("1") == 0;
        EXPECT_TRUE(result);
        if (!result && i > 0) {
            GTEST_LOG_(INFO) << "TaskDispatcher_Serial_05100 : " << i;
            break;
        }
    }
}

/**
 * @tc.number    : TaskDispatcher_Serial_05200
 * @tc.name      : serial apply:serial apply
 * @tc.desc      : two levels task dispatch
 */
HWTEST_F(TaskDispatcherTest, TaskDispatcher_Serial_05200, Function | MediumTest | Level1)
{
    StartAbility(firstAbilityName, bundleNameFirst);
    std::string data = "Dispatcher_" + std::to_string((int)TestFunc::SERIAL) + "_51";
    bool result = false;
    for (int i = 0; i < stLevel_.AMSLevel; i++) {
        STAbilityUtil::PublishEvent(g_EVENT_REQU_FIRST, ++testCaseCode, data);
        EXPECT_EQ(0, STAbilityUtil::WaitCompleted(event, g_EVENT_RESP_FIRST, testCaseCode, DELAY));
        data = STAbilityUtil::GetData(event, g_EVENT_RESP_FIRST, testCaseCode);
        result = data.compare("1") == 0;
        EXPECT_TRUE(result);
        if (!result && i > 0) {
            GTEST_LOG_(INFO) << "TaskDispatcher_Serial_05200 : " << i;
            break;
        }
    }
}

/**
 * @tc.number    : TaskDispatcher_Spec_00100
 * @tc.name      : spec sync:parallel sync
 * @tc.desc      : two levels task dispatch
 */
HWTEST_F(TaskDispatcherTest, TaskDispatcher_Spec_00100, Function | MediumTest | Level1)
{
    StartAbility(firstAbilityName, bundleNameFirst);
    std::string data = "Dispatcher_" + std::to_string((int)TestFunc::SPEC) + "_0";
    bool result = false;
    for (int i = 0; i < stLevel_.AMSLevel; i++) {
        STAbilityUtil::PublishEvent(g_EVENT_REQU_FIRST, ++testCaseCode, data);
        EXPECT_EQ(0, STAbilityUtil::WaitCompleted(event, g_EVENT_RESP_FIRST, testCaseCode, DELAY));
        data = STAbilityUtil::GetData(event, g_EVENT_RESP_FIRST, testCaseCode);
        result = data.compare("1") == 0;
        EXPECT_TRUE(result);
        if (!result && i > 0) {
            GTEST_LOG_(INFO) << "TaskDispatcher_Spec_00100 : " << i;
            break;
        }
    }
}

/**
 * @tc.number    : TaskDispatcher_Spec_00200
 * @tc.name      : spec sync:parallel async
 * @tc.desc      : two levels task dispatch
 */
HWTEST_F(TaskDispatcherTest, TaskDispatcher_Spec_00200, Function | MediumTest | Level1)
{
    StartAbility(firstAbilityName, bundleNameFirst);
    std::string data = "Dispatcher_" + std::to_string((int)TestFunc::SPEC) + "_1";
    bool result = false;
    for (int i = 0; i < stLevel_.AMSLevel; i++) {
        STAbilityUtil::PublishEvent(g_EVENT_REQU_FIRST, ++testCaseCode, data);
        EXPECT_EQ(0, STAbilityUtil::WaitCompleted(event, g_EVENT_RESP_FIRST, testCaseCode, DELAY));
        data = STAbilityUtil::GetData(event, g_EVENT_RESP_FIRST, testCaseCode);
        result = data.compare("1") == 0;
        EXPECT_TRUE(result);
        if (!result && i > 0) {
            GTEST_LOG_(INFO) << "TaskDispatcher_Spec_00200 : " << i;
            break;
        }
    }
}

/**
 * @tc.number    : TaskDispatcher_Spec_00300
 * @tc.name      : spec sync:parallel delay
 * @tc.desc      : two levels task dispatch
 */
HWTEST_F(TaskDispatcherTest, TaskDispatcher_Spec_00300, Function | MediumTest | Level1)
{
    StartAbility(firstAbilityName, bundleNameFirst);
    std::string data = "Dispatcher_" + std::to_string((int)TestFunc::SPEC) + "_2";
    bool result = false;
    for (int i = 0; i < stLevel_.AMSLevel; i++) {
        STAbilityUtil::PublishEvent(g_EVENT_REQU_FIRST, ++testCaseCode, data);
        EXPECT_EQ(0, STAbilityUtil::WaitCompleted(event, g_EVENT_RESP_FIRST, testCaseCode, DELAY));
        data = STAbilityUtil::GetData(event, g_EVENT_RESP_FIRST, testCaseCode);
        result = data.compare("1") == 0;
        EXPECT_TRUE(result);
        if (!result && i > 0) {
            GTEST_LOG_(INFO) << "TaskDispatcher_Spec_00300 : " << i;
            break;
        }
    }
}

/**
 * @tc.number    : TaskDispatcher_Spec_00400
 * @tc.name      : spec sync:parallel group
 * @tc.desc      : two levels task dispatch
 */
HWTEST_F(TaskDispatcherTest, TaskDispatcher_Spec_00400, Function | MediumTest | Level1)
{
    StartAbility(firstAbilityName, bundleNameFirst);
    std::string data = "Dispatcher_" + std::to_string((int)TestFunc::SPEC) + "_3";
    bool result = false;
    for (int i = 0; i < stLevel_.AMSLevel; i++) {
        STAbilityUtil::PublishEvent(g_EVENT_REQU_FIRST, ++testCaseCode, data);
        EXPECT_EQ(0, STAbilityUtil::WaitCompleted(event, g_EVENT_RESP_FIRST, testCaseCode, DELAY));
        data = STAbilityUtil::GetData(event, g_EVENT_RESP_FIRST, testCaseCode);
        result = data.compare("1") == 0;
        EXPECT_TRUE(result);
        if (!result && i > 0) {
            GTEST_LOG_(INFO) << "TaskDispatcher_Spec_00400 : " << i;
            break;
        }
    }
}

/**
 * @tc.number    : TaskDispatcher_Spec_00500
 * @tc.name      : spec sync:parallel group wait
 * @tc.desc      : two levels task dispatch
 */
HWTEST_F(TaskDispatcherTest, TaskDispatcher_Spec_00500, Function | MediumTest | Level1)
{
    StartAbility(firstAbilityName, bundleNameFirst);
    std::string data = "Dispatcher_" + std::to_string((int)TestFunc::SPEC) + "_4";
    bool result = false;
    for (int i = 0; i < stLevel_.AMSLevel; i++) {
        STAbilityUtil::PublishEvent(g_EVENT_REQU_FIRST, ++testCaseCode, data);
        EXPECT_EQ(0, STAbilityUtil::WaitCompleted(event, g_EVENT_RESP_FIRST, testCaseCode, DELAY));
        data = STAbilityUtil::GetData(event, g_EVENT_RESP_FIRST, testCaseCode);
        result = data.compare("1") == 0;
        EXPECT_TRUE(result);
        if (!result && i > 0) {
            GTEST_LOG_(INFO) << "TaskDispatcher_Spec_00500 : " << i;
            break;
        }
    }
}

/**
 * @tc.number    : TaskDispatcher_Spec_00600
 * @tc.name      : spec sync:parallel group notify
 * @tc.desc      : two levels task dispatch
 */
HWTEST_F(TaskDispatcherTest, TaskDispatcher_Spec_00600, Function | MediumTest | Level1)
{
    StartAbility(firstAbilityName, bundleNameFirst);
    std::string data = "Dispatcher_" + std::to_string((int)TestFunc::SPEC) + "_5";
    bool result = false;
    for (int i = 0; i < stLevel_.AMSLevel; i++) {
        STAbilityUtil::PublishEvent(g_EVENT_REQU_FIRST, ++testCaseCode, data);
        EXPECT_EQ(0, STAbilityUtil::WaitCompleted(event, g_EVENT_RESP_FIRST, testCaseCode, DELAY));
        data = STAbilityUtil::GetData(event, g_EVENT_RESP_FIRST, testCaseCode);
        result = data.compare("1") == 0;
        EXPECT_TRUE(result);
        if (!result && i > 0) {
            GTEST_LOG_(INFO) << "TaskDispatcher_Spec_00600 : " << i;
            break;
        }
    }
}

/**
 * @tc.number    : TaskDispatcher_Spec_00700
 * @tc.name      : spec sync:parallel sync barrier
 * @tc.desc      : two levels task dispatch
 */
HWTEST_F(TaskDispatcherTest, TaskDispatcher_Spec_00700, Function | MediumTest | Level1)
{
    StartAbility(firstAbilityName, bundleNameFirst);
    std::string data = "Dispatcher_" + std::to_string((int)TestFunc::SPEC) + "_6";
    bool result = false;
    for (int i = 0; i < stLevel_.AMSLevel; i++) {
        STAbilityUtil::PublishEvent(g_EVENT_REQU_FIRST, ++testCaseCode, data);
        EXPECT_EQ(0, STAbilityUtil::WaitCompleted(event, g_EVENT_RESP_FIRST, testCaseCode, DELAY));
        data = STAbilityUtil::GetData(event, g_EVENT_RESP_FIRST, testCaseCode);
        result = data.compare("1") == 0;
        EXPECT_TRUE(result);
        if (!result && i > 0) {
            GTEST_LOG_(INFO) << "TaskDispatcher_Spec_00700 : " << i;
            break;
        }
    }
}

/**
 * @tc.number    : TaskDispatcher_Spec_00800
 * @tc.name      : spec sync:parallel async barrier
 * @tc.desc      : two levels task dispatch
 */
HWTEST_F(TaskDispatcherTest, TaskDispatcher_Spec_00800, Function | MediumTest | Level1)
{
    StartAbility(firstAbilityName, bundleNameFirst);
    std::string data = "Dispatcher_" + std::to_string((int)TestFunc::SPEC) + "_7";
    bool result = false;
    for (int i = 0; i < stLevel_.AMSLevel; i++) {
        STAbilityUtil::PublishEvent(g_EVENT_REQU_FIRST, ++testCaseCode, data);
        EXPECT_EQ(0, STAbilityUtil::WaitCompleted(event, g_EVENT_RESP_FIRST, testCaseCode, DELAY));
        data = STAbilityUtil::GetData(event, g_EVENT_RESP_FIRST, testCaseCode);
        result = data.compare("1") == 0;
        EXPECT_TRUE(result);
        if (!result && i > 0) {
            GTEST_LOG_(INFO) << "TaskDispatcher_Spec_00800 : " << i;
            break;
        }
    }
}

/**
 * @tc.number    : TaskDispatcher_Spec_00900
 * @tc.name      : spec sync:parallel apply
 * @tc.desc      : two levels task dispatch
 */
HWTEST_F(TaskDispatcherTest, TaskDispatcher_Spec_00900, Function | MediumTest | Level1)
{
    StartAbility(firstAbilityName, bundleNameFirst);
    std::string data = "Dispatcher_" + std::to_string((int)TestFunc::SPEC) + "_8";
    bool result = false;
    for (int i = 0; i < stLevel_.AMSLevel; i++) {
        STAbilityUtil::PublishEvent(g_EVENT_REQU_FIRST, ++testCaseCode, data);
        EXPECT_EQ(0, STAbilityUtil::WaitCompleted(event, g_EVENT_RESP_FIRST, testCaseCode, DELAY));
        data = STAbilityUtil::GetData(event, g_EVENT_RESP_FIRST, testCaseCode);
        result = data.compare("1") == 0;
        EXPECT_TRUE(result);
        if (!result && i > 0) {
            GTEST_LOG_(INFO) << "TaskDispatcher_Spec_00900 : " << i;
            break;
        }
    }
}

/**
 * @tc.number    : TaskDispatcher_Spec_01000
 * @tc.name      : spec sync:serial sync
 * @tc.desc      : two levels task dispatch
 */
HWTEST_F(TaskDispatcherTest, TaskDispatcher_Spec_01000, Function | MediumTest | Level1)
{
    StartAbility(firstAbilityName, bundleNameFirst);
    std::string data = "Dispatcher_" + std::to_string((int)TestFunc::SPEC) + "_9";
    bool result = false;
    for (int i = 0; i < stLevel_.AMSLevel; i++) {
        STAbilityUtil::PublishEvent(g_EVENT_REQU_FIRST, ++testCaseCode, data);
        EXPECT_EQ(0, STAbilityUtil::WaitCompleted(event, g_EVENT_RESP_FIRST, testCaseCode, DELAY));
        data = STAbilityUtil::GetData(event, g_EVENT_RESP_FIRST, testCaseCode);
        result = data.compare("1") == 0;
        EXPECT_TRUE(result);
        if (!result && i > 0) {
            GTEST_LOG_(INFO) << "TaskDispatcher_Spec_01000 : " << i;
            break;
        }
    }
}

/**
 * @tc.number    : TaskDispatcher_Spec_01100
 * @tc.name      : spec sync:serial async
 * @tc.desc      : two levels task dispatch
 */
HWTEST_F(TaskDispatcherTest, TaskDispatcher_Spec_01100, Function | MediumTest | Level1)
{
    StartAbility(firstAbilityName, bundleNameFirst);
    std::string data = "Dispatcher_" + std::to_string((int)TestFunc::SPEC) + "_10";
    bool result = false;
    for (int i = 0; i < stLevel_.AMSLevel; i++) {
        STAbilityUtil::PublishEvent(g_EVENT_REQU_FIRST, ++testCaseCode, data);
        EXPECT_EQ(0, STAbilityUtil::WaitCompleted(event, g_EVENT_RESP_FIRST, testCaseCode, DELAY));
        data = STAbilityUtil::GetData(event, g_EVENT_RESP_FIRST, testCaseCode);
        result = data.compare("1") == 0;
        EXPECT_TRUE(result);
        if (!result && i > 0) {
            GTEST_LOG_(INFO) << "TaskDispatcher_Spec_01100 : " << i;
            break;
        }
    }
}

/**
 * @tc.number    : TaskDispatcher_Spec_01200
 * @tc.name      : spec sync:serial delay
 * @tc.desc      : two levels task dispatch
 */
HWTEST_F(TaskDispatcherTest, TaskDispatcher_Spec_01200, Function | MediumTest | Level1)
{
    StartAbility(firstAbilityName, bundleNameFirst);
    std::string data = "Dispatcher_" + std::to_string((int)TestFunc::SPEC) + "_11";
    bool result = false;
    for (int i = 0; i < stLevel_.AMSLevel; i++) {
        STAbilityUtil::PublishEvent(g_EVENT_REQU_FIRST, ++testCaseCode, data);
        EXPECT_EQ(0, STAbilityUtil::WaitCompleted(event, g_EVENT_RESP_FIRST, testCaseCode, DELAY));
        data = STAbilityUtil::GetData(event, g_EVENT_RESP_FIRST, testCaseCode);
        result = data.compare("1") == 0;
        EXPECT_TRUE(result);
        if (!result && i > 0) {
            GTEST_LOG_(INFO) << "TaskDispatcher_Spec_01200 : " << i;
            break;
        }
    }
}

/**
 * @tc.number    : TaskDispatcher_Spec_01300
 * @tc.name      : spec sync:serial apply
 * @tc.desc      : two levels task dispatch
 */
HWTEST_F(TaskDispatcherTest, TaskDispatcher_Spec_01300, Function | MediumTest | Level1)
{
    StartAbility(firstAbilityName, bundleNameFirst);
    std::string data = "Dispatcher_" + std::to_string((int)TestFunc::SPEC) + "_12";
    bool result = false;
    for (int i = 0; i < stLevel_.AMSLevel; i++) {
        STAbilityUtil::PublishEvent(g_EVENT_REQU_FIRST, ++testCaseCode, data);
        EXPECT_EQ(0, STAbilityUtil::WaitCompleted(event, g_EVENT_RESP_FIRST, testCaseCode, DELAY));
        data = STAbilityUtil::GetData(event, g_EVENT_RESP_FIRST, testCaseCode);
        result = data.compare("1") == 0;
        EXPECT_TRUE(result);
        if (!result && i > 0) {
            GTEST_LOG_(INFO) << "TaskDispatcher_Spec_01300 : " << i;
            break;
        }
    }
}

/**
 * @tc.number    : TaskDispatcher_Spec_01400
 * @tc.name      : spec async:parallel sync
 * @tc.desc      : two levels task dispatch
 */
HWTEST_F(TaskDispatcherTest, TaskDispatcher_Spec_01400, Function | MediumTest | Level1)
{
    StartAbility(firstAbilityName, bundleNameFirst);
    std::string data = "Dispatcher_" + std::to_string((int)TestFunc::SPEC) + "_13";
    bool result = false;
    for (int i = 0; i < stLevel_.AMSLevel; i++) {
        STAbilityUtil::PublishEvent(g_EVENT_REQU_FIRST, ++testCaseCode, data);
        EXPECT_EQ(0, STAbilityUtil::WaitCompleted(event, g_EVENT_RESP_FIRST, testCaseCode, DELAY));
        data = STAbilityUtil::GetData(event, g_EVENT_RESP_FIRST, testCaseCode);
        result = data.compare("1") == 0;
        EXPECT_TRUE(result);
        if (!result && i > 0) {
            GTEST_LOG_(INFO) << "TaskDispatcher_Spec_01400 : " << i;
            break;
        }
    }
}

/**
 * @tc.number    : TaskDispatcher_Spec_01500
 * @tc.name      : spec async:parallel async
 * @tc.desc      : two levels task dispatch
 */
HWTEST_F(TaskDispatcherTest, TaskDispatcher_Spec_01500, Function | MediumTest | Level1)
{
    StartAbility(firstAbilityName, bundleNameFirst);
    std::string data = "Dispatcher_" + std::to_string((int)TestFunc::SPEC) + "_14";
    bool result = false;
    for (int i = 0; i < stLevel_.AMSLevel; i++) {
        STAbilityUtil::PublishEvent(g_EVENT_REQU_FIRST, ++testCaseCode, data);
        EXPECT_EQ(0, STAbilityUtil::WaitCompleted(event, g_EVENT_RESP_FIRST, testCaseCode, DELAY));
        data = STAbilityUtil::GetData(event, g_EVENT_RESP_FIRST, testCaseCode);
        result = data.compare("1") == 0;
        EXPECT_TRUE(result);
        if (!result && i > 0) {
            GTEST_LOG_(INFO) << "TaskDispatcher_Spec_01500 : " << i;
            break;
        }
    }
}

/**
 * @tc.number    : TaskDispatcher_Spec_01600
 * @tc.name      : spec async:parallel delay
 * @tc.desc      : two levels task dispatch
 */
HWTEST_F(TaskDispatcherTest, TaskDispatcher_Spec_01600, Function | MediumTest | Level1)
{
    StartAbility(firstAbilityName, bundleNameFirst);
    std::string data = "Dispatcher_" + std::to_string((int)TestFunc::SPEC) + "_15";
    bool result = false;
    for (int i = 0; i < stLevel_.AMSLevel; i++) {
        STAbilityUtil::PublishEvent(g_EVENT_REQU_FIRST, ++testCaseCode, data);
        EXPECT_EQ(0, STAbilityUtil::WaitCompleted(event, g_EVENT_RESP_FIRST, testCaseCode, DELAY));
        data = STAbilityUtil::GetData(event, g_EVENT_RESP_FIRST, testCaseCode);
        result = data.compare("1") == 0;
        EXPECT_TRUE(result);
        if (!result && i > 0) {
            GTEST_LOG_(INFO) << "TaskDispatcher_Spec_01600 : " << i;
            break;
        }
    }
}

/**
 * @tc.number    : TaskDispatcher_Spec_01700
 * @tc.name      : spec async:parallel group
 * @tc.desc      : two levels task dispatch
 */
HWTEST_F(TaskDispatcherTest, TaskDispatcher_Spec_01700, Function | MediumTest | Level1)
{
    StartAbility(firstAbilityName, bundleNameFirst);
    std::string data = "Dispatcher_" + std::to_string((int)TestFunc::SPEC) + "_16";
    bool result = false;
    for (int i = 0; i < stLevel_.AMSLevel; i++) {
        STAbilityUtil::PublishEvent(g_EVENT_REQU_FIRST, ++testCaseCode, data);
        EXPECT_EQ(0, STAbilityUtil::WaitCompleted(event, g_EVENT_RESP_FIRST, testCaseCode, DELAY));
        data = STAbilityUtil::GetData(event, g_EVENT_RESP_FIRST, testCaseCode);
        result = data.compare("1") == 0;
        EXPECT_TRUE(result);
        if (!result && i > 0) {
            GTEST_LOG_(INFO) << "TaskDispatcher_Spec_01700 : " << i;
            break;
        }
    }
}

/**
 * @tc.number    : TaskDispatcher_Spec_01800
 * @tc.name      : spec async:parallel group wait
 * @tc.desc      : two levels task dispatch
 */
HWTEST_F(TaskDispatcherTest, TaskDispatcher_Spec_01800, Function | MediumTest | Level1)
{
    StartAbility(firstAbilityName, bundleNameFirst);
    std::string data = "Dispatcher_" + std::to_string((int)TestFunc::SPEC) + "_17";
    bool result = false;
    for (int i = 0; i < stLevel_.AMSLevel; i++) {
        STAbilityUtil::PublishEvent(g_EVENT_REQU_FIRST, ++testCaseCode, data);
        EXPECT_EQ(0, STAbilityUtil::WaitCompleted(event, g_EVENT_RESP_FIRST, testCaseCode, DELAY));
        data = STAbilityUtil::GetData(event, g_EVENT_RESP_FIRST, testCaseCode);
        result = data.compare("1") == 0;
        EXPECT_TRUE(result);
        if (!result && i > 0) {
            GTEST_LOG_(INFO) << "TaskDispatcher_Spec_01800 : " << i;
            break;
        }
    }
}

/**
 * @tc.number    : TaskDispatcher_Spec_01900
 * @tc.name      : spec async:parallel group notify
 * @tc.desc      : two levels task dispatch
 */
HWTEST_F(TaskDispatcherTest, TaskDispatcher_Spec_01900, Function | MediumTest | Level1)
{
    StartAbility(firstAbilityName, bundleNameFirst);
    std::string data = "Dispatcher_" + std::to_string((int)TestFunc::SPEC) + "_18";
    bool result = false;
    for (int i = 0; i < stLevel_.AMSLevel; i++) {
        STAbilityUtil::PublishEvent(g_EVENT_REQU_FIRST, ++testCaseCode, data);
        EXPECT_EQ(0, STAbilityUtil::WaitCompleted(event, g_EVENT_RESP_FIRST, testCaseCode, DELAY));
        data = STAbilityUtil::GetData(event, g_EVENT_RESP_FIRST, testCaseCode);
        result = data.compare("1") == 0;
        EXPECT_TRUE(result);
        if (!result && i > 0) {
            GTEST_LOG_(INFO) << "TaskDispatcher_Spec_01900 : " << i;
            break;
        }
    }
}

/**
 * @tc.number    : TaskDispatcher_Spec_02000
 * @tc.name      : spec async:parallel sync barrier
 * @tc.desc      : two levels task dispatch
 */
HWTEST_F(TaskDispatcherTest, TaskDispatcher_Spec_02000, Function | MediumTest | Level1)
{
    StartAbility(firstAbilityName, bundleNameFirst);
    std::string data = "Dispatcher_" + std::to_string((int)TestFunc::SPEC) + "_19";
    bool result = false;
    for (int i = 0; i < stLevel_.AMSLevel; i++) {
        STAbilityUtil::PublishEvent(g_EVENT_REQU_FIRST, ++testCaseCode, data);
        EXPECT_EQ(0, STAbilityUtil::WaitCompleted(event, g_EVENT_RESP_FIRST, testCaseCode, DELAY));
        data = STAbilityUtil::GetData(event, g_EVENT_RESP_FIRST, testCaseCode);
        result = data.compare("1") == 0;
        EXPECT_TRUE(result);
        if (!result && i > 0) {
            GTEST_LOG_(INFO) << "TaskDispatcher_Spec_02000 : " << i;
            break;
        }
    }
}

/**
 * @tc.number    : TaskDispatcher_Spec_02100
 * @tc.name      : spec async:parallel async barrier
 * @tc.desc      : two levels task dispatch
 */
HWTEST_F(TaskDispatcherTest, TaskDispatcher_Spec_02100, Function | MediumTest | Level1)
{
    StartAbility(firstAbilityName, bundleNameFirst);
    std::string data = "Dispatcher_" + std::to_string((int)TestFunc::SPEC) + "_20";
    bool result = false;
    for (int i = 0; i < stLevel_.AMSLevel; i++) {
        STAbilityUtil::PublishEvent(g_EVENT_REQU_FIRST, ++testCaseCode, data);
        EXPECT_EQ(0, STAbilityUtil::WaitCompleted(event, g_EVENT_RESP_FIRST, testCaseCode, DELAY));
        data = STAbilityUtil::GetData(event, g_EVENT_RESP_FIRST, testCaseCode);
        result = data.compare("1") == 0;
        EXPECT_TRUE(result);
        if (!result && i > 0) {
            GTEST_LOG_(INFO) << "TaskDispatcher_Spec_02100 : " << i;
            break;
        }
    }
}

/**
 * @tc.number    : TaskDispatcher_Spec_02200
 * @tc.name      : spec async:parallel apply
 * @tc.desc      : two levels task dispatch
 */
HWTEST_F(TaskDispatcherTest, TaskDispatcher_Spec_02200, Function | MediumTest | Level1)
{
    StartAbility(firstAbilityName, bundleNameFirst);
    std::string data = "Dispatcher_" + std::to_string((int)TestFunc::SPEC) + "_21";
    bool result = false;
    for (int i = 0; i < stLevel_.AMSLevel; i++) {
        STAbilityUtil::PublishEvent(g_EVENT_REQU_FIRST, ++testCaseCode, data);
        EXPECT_EQ(0, STAbilityUtil::WaitCompleted(event, g_EVENT_RESP_FIRST, testCaseCode, DELAY));
        data = STAbilityUtil::GetData(event, g_EVENT_RESP_FIRST, testCaseCode);
        result = data.compare("1") == 0;
        EXPECT_TRUE(result);
        if (!result && i > 0) {
            GTEST_LOG_(INFO) << "TaskDispatcher_Spec_02200 : " << i;
            break;
        }
    }
}

/**
 * @tc.number    : TaskDispatcher_Spec_02300
 * @tc.name      : spec async:serial sync
 * @tc.desc      : two levels task dispatch
 */
HWTEST_F(TaskDispatcherTest, TaskDispatcher_Spec_02300, Function | MediumTest | Level1)
{
    StartAbility(firstAbilityName, bundleNameFirst);
    std::string data = "Dispatcher_" + std::to_string((int)TestFunc::SPEC) + "_22";
    bool result = false;
    for (int i = 0; i < stLevel_.AMSLevel; i++) {
        STAbilityUtil::PublishEvent(g_EVENT_REQU_FIRST, ++testCaseCode, data);
        EXPECT_EQ(0, STAbilityUtil::WaitCompleted(event, g_EVENT_RESP_FIRST, testCaseCode, DELAY));
        data = STAbilityUtil::GetData(event, g_EVENT_RESP_FIRST, testCaseCode);
        result = data.compare("1") == 0;
        EXPECT_TRUE(result);
        if (!result && i > 0) {
            GTEST_LOG_(INFO) << "TaskDispatcher_Spec_02300 : " << i;
            break;
        }
    }
}

/**
 * @tc.number    : TaskDispatcher_Spec_02400
 * @tc.name      : spec async:serial async
 * @tc.desc      : two levels task dispatch
 */
HWTEST_F(TaskDispatcherTest, TaskDispatcher_Spec_02400, Function | MediumTest | Level1)
{
    StartAbility(firstAbilityName, bundleNameFirst);
    std::string data = "Dispatcher_" + std::to_string((int)TestFunc::SPEC) + "_23";
    bool result = false;
    for (int i = 0; i < stLevel_.AMSLevel; i++) {
        STAbilityUtil::PublishEvent(g_EVENT_REQU_FIRST, ++testCaseCode, data);
        EXPECT_EQ(0, STAbilityUtil::WaitCompleted(event, g_EVENT_RESP_FIRST, testCaseCode, DELAY));
        data = STAbilityUtil::GetData(event, g_EVENT_RESP_FIRST, testCaseCode);
        result = data.compare("1") == 0;
        EXPECT_TRUE(result);
        if (!result && i > 0) {
            GTEST_LOG_(INFO) << "TaskDispatcher_Spec_02400 : " << i;
            break;
        }
    }
}

/**
 * @tc.number    : TaskDispatcher_Spec_02500
 * @tc.name      : spec async:serial delay
 * @tc.desc      : two levels task dispatch
 */
HWTEST_F(TaskDispatcherTest, TaskDispatcher_Spec_02500, Function | MediumTest | Level1)
{
    StartAbility(firstAbilityName, bundleNameFirst);
    std::string data = "Dispatcher_" + std::to_string((int)TestFunc::SPEC) + "_24";
    bool result = false;
    for (int i = 0; i < stLevel_.AMSLevel; i++) {
        STAbilityUtil::PublishEvent(g_EVENT_REQU_FIRST, ++testCaseCode, data);
        EXPECT_EQ(0, STAbilityUtil::WaitCompleted(event, g_EVENT_RESP_FIRST, testCaseCode, DELAY));
        data = STAbilityUtil::GetData(event, g_EVENT_RESP_FIRST, testCaseCode);
        result = data.compare("1") == 0;
        EXPECT_TRUE(result);
        if (!result && i > 0) {
            GTEST_LOG_(INFO) << "TaskDispatcher_Spec_02500 : " << i;
            break;
        }
    }
}

/**
 * @tc.number    : TaskDispatcher_Spec_02600
 * @tc.name      : spec async:serial apply
 * @tc.desc      : two levels task dispatch
 */
HWTEST_F(TaskDispatcherTest, TaskDispatcher_Spec_02600, Function | MediumTest | Level1)
{
    StartAbility(firstAbilityName, bundleNameFirst);
    std::string data = "Dispatcher_" + std::to_string((int)TestFunc::SPEC) + "_25";
    bool result = false;
    for (int i = 0; i < stLevel_.AMSLevel; i++) {
        STAbilityUtil::PublishEvent(g_EVENT_REQU_FIRST, ++testCaseCode, data);
        EXPECT_EQ(0, STAbilityUtil::WaitCompleted(event, g_EVENT_RESP_FIRST, testCaseCode, DELAY));
        data = STAbilityUtil::GetData(event, g_EVENT_RESP_FIRST, testCaseCode);
        result = data.compare("1") == 0;
        EXPECT_TRUE(result);
        if (!result && i > 0) {
            GTEST_LOG_(INFO) << "TaskDispatcher_Spec_02600 : " << i;
            break;
        }
    }
}

/**
 * @tc.number    : TaskDispatcher_Spec_02700
 * @tc.name      : spec delay:parallel sync
 * @tc.desc      : two levels task dispatch
 */
HWTEST_F(TaskDispatcherTest, TaskDispatcher_Spec_02700, Function | MediumTest | Level1)
{
    StartAbility(firstAbilityName, bundleNameFirst);
    std::string data = "Dispatcher_" + std::to_string((int)TestFunc::SPEC) + "_26";
    bool result = false;
    for (int i = 0; i < stLevel_.AMSLevel; i++) {
        STAbilityUtil::PublishEvent(g_EVENT_REQU_FIRST, ++testCaseCode, data);
        EXPECT_EQ(0, STAbilityUtil::WaitCompleted(event, g_EVENT_RESP_FIRST, testCaseCode, DELAY));
        data = STAbilityUtil::GetData(event, g_EVENT_RESP_FIRST, testCaseCode);
        result = data.compare("1") == 0;
        EXPECT_TRUE(result);
        if (!result && i > 0) {
            GTEST_LOG_(INFO) << "TaskDispatcher_Spec_02700 : " << i;
            break;
        }
    }
}

/**
 * @tc.number    : TaskDispatcher_Spec_02800
 * @tc.name      : spec delay:parallel async
 * @tc.desc      : two levels task dispatch
 */
HWTEST_F(TaskDispatcherTest, TaskDispatcher_Spec_02800, Function | MediumTest | Level1)
{
    StartAbility(firstAbilityName, bundleNameFirst);
    std::string data = "Dispatcher_" + std::to_string((int)TestFunc::SPEC) + "_27";
    bool result = false;
    for (int i = 0; i < stLevel_.AMSLevel; i++) {
        STAbilityUtil::PublishEvent(g_EVENT_REQU_FIRST, ++testCaseCode, data);
        EXPECT_EQ(0, STAbilityUtil::WaitCompleted(event, g_EVENT_RESP_FIRST, testCaseCode, DELAY));
        data = STAbilityUtil::GetData(event, g_EVENT_RESP_FIRST, testCaseCode);
        result = data.compare("1") == 0;
        EXPECT_TRUE(result);
        if (!result && i > 0) {
            GTEST_LOG_(INFO) << "TaskDispatcher_Spec_02800 : " << i;
            break;
        }
    }
}

/**
 * @tc.number    : TaskDispatcher_Spec_02900
 * @tc.name      : spec delay:parallel delay
 * @tc.desc      : two levels task dispatch
 */
HWTEST_F(TaskDispatcherTest, TaskDispatcher_Spec_02900, Function | MediumTest | Level1)
{
    StartAbility(firstAbilityName, bundleNameFirst);
    std::string data = "Dispatcher_" + std::to_string((int)TestFunc::SPEC) + "_28";
    bool result = false;
    for (int i = 0; i < stLevel_.AMSLevel; i++) {
        STAbilityUtil::PublishEvent(g_EVENT_REQU_FIRST, ++testCaseCode, data);
        EXPECT_EQ(0, STAbilityUtil::WaitCompleted(event, g_EVENT_RESP_FIRST, testCaseCode, DELAY));
        data = STAbilityUtil::GetData(event, g_EVENT_RESP_FIRST, testCaseCode);
        result = data.compare("1") == 0;
        EXPECT_TRUE(result);
        if (!result && i > 0) {
            GTEST_LOG_(INFO) << "TaskDispatcher_Spec_02900 : " << i;
            break;
        }
    }
}

/**
 * @tc.number    : TaskDispatcher_Spec_03000
 * @tc.name      : spec delay:parallel group
 * @tc.desc      : two levels task dispatch
 */
HWTEST_F(TaskDispatcherTest, TaskDispatcher_Spec_03000, Function | MediumTest | Level1)
{
    StartAbility(firstAbilityName, bundleNameFirst);
    std::string data = "Dispatcher_" + std::to_string((int)TestFunc::SPEC) + "_29";
    bool result = false;
    for (int i = 0; i < stLevel_.AMSLevel; i++) {
        STAbilityUtil::PublishEvent(g_EVENT_REQU_FIRST, ++testCaseCode, data);
        EXPECT_EQ(0, STAbilityUtil::WaitCompleted(event, g_EVENT_RESP_FIRST, testCaseCode, DELAY));
        data = STAbilityUtil::GetData(event, g_EVENT_RESP_FIRST, testCaseCode);
        result = data.compare("1") == 0;
        EXPECT_TRUE(result);
        if (!result && i > 0) {
            GTEST_LOG_(INFO) << "TaskDispatcher_Spec_03000 : " << i;
            break;
        }
    }
}

/**
 * @tc.number    : TaskDispatcher_Spec_03100
 * @tc.name      : spec delay:parallel group wait
 * @tc.desc      : two levels task dispatch
 */
HWTEST_F(TaskDispatcherTest, TaskDispatcher_Spec_03100, Function | MediumTest | Level1)
{
    StartAbility(firstAbilityName, bundleNameFirst);
    std::string data = "Dispatcher_" + std::to_string((int)TestFunc::SPEC) + "_30";
    bool result = false;
    for (int i = 0; i < stLevel_.AMSLevel; i++) {
        STAbilityUtil::PublishEvent(g_EVENT_REQU_FIRST, ++testCaseCode, data);
        EXPECT_EQ(0, STAbilityUtil::WaitCompleted(event, g_EVENT_RESP_FIRST, testCaseCode, DELAY));
        data = STAbilityUtil::GetData(event, g_EVENT_RESP_FIRST, testCaseCode);
        result = data.compare("1") == 0;
        EXPECT_TRUE(result);
        if (!result && i > 0) {
            GTEST_LOG_(INFO) << "TaskDispatcher_Spec_03100 : " << i;
            break;
        }
    }
}

/**
 * @tc.number    : TaskDispatcher_Spec_03200
 * @tc.name      : spec delay:parallel group notify
 * @tc.desc      : two levels task dispatch
 */
HWTEST_F(TaskDispatcherTest, TaskDispatcher_Spec_03200, Function | MediumTest | Level1)
{
    StartAbility(firstAbilityName, bundleNameFirst);
    std::string data = "Dispatcher_" + std::to_string((int)TestFunc::SPEC) + "_31";
    bool result = false;
    for (int i = 0; i < stLevel_.AMSLevel; i++) {
        STAbilityUtil::PublishEvent(g_EVENT_REQU_FIRST, ++testCaseCode, data);
        EXPECT_EQ(0, STAbilityUtil::WaitCompleted(event, g_EVENT_RESP_FIRST, testCaseCode, DELAY));
        data = STAbilityUtil::GetData(event, g_EVENT_RESP_FIRST, testCaseCode);
        result = data.compare("1") == 0;
        EXPECT_TRUE(result);
        if (!result && i > 0) {
            GTEST_LOG_(INFO) << "TaskDispatcher_Spec_03200 : " << i;
            break;
        }
    }
}

/**
 * @tc.number    : TaskDispatcher_Spec_03300
 * @tc.name      : spec delay:parallel sync barrier
 * @tc.desc      : two levels task dispatch
 */
HWTEST_F(TaskDispatcherTest, TaskDispatcher_Spec_03300, Function | MediumTest | Level1)
{
    StartAbility(firstAbilityName, bundleNameFirst);
    std::string data = "Dispatcher_" + std::to_string((int)TestFunc::SPEC) + "_32";
    bool result = false;
    for (int i = 0; i < stLevel_.AMSLevel; i++) {
        STAbilityUtil::PublishEvent(g_EVENT_REQU_FIRST, ++testCaseCode, data);
        EXPECT_EQ(0, STAbilityUtil::WaitCompleted(event, g_EVENT_RESP_FIRST, testCaseCode, DELAY));
        data = STAbilityUtil::GetData(event, g_EVENT_RESP_FIRST, testCaseCode);
        result = data.compare("1") == 0;
        EXPECT_TRUE(result);
        if (!result && i > 0) {
            GTEST_LOG_(INFO) << "TaskDispatcher_Spec_03300 : " << i;
            break;
        }
    }
}

/**
 * @tc.number    : TaskDispatcher_Spec_03400
 * @tc.name      : spec delay:parallel async barrier
 * @tc.desc      : two levels task dispatch
 */
HWTEST_F(TaskDispatcherTest, TaskDispatcher_Spec_03400, Function | MediumTest | Level1)
{
    StartAbility(firstAbilityName, bundleNameFirst);
    std::string data = "Dispatcher_" + std::to_string((int)TestFunc::SPEC) + "_33";
    bool result = false;
    for (int i = 0; i < stLevel_.AMSLevel; i++) {
        STAbilityUtil::PublishEvent(g_EVENT_REQU_FIRST, ++testCaseCode, data);
        EXPECT_EQ(0, STAbilityUtil::WaitCompleted(event, g_EVENT_RESP_FIRST, testCaseCode, DELAY));
        data = STAbilityUtil::GetData(event, g_EVENT_RESP_FIRST, testCaseCode);
        result = data.compare("1") == 0;
        EXPECT_TRUE(result);
        if (!result && i > 0) {
            GTEST_LOG_(INFO) << "TaskDispatcher_Spec_03400 : " << i;
            break;
        }
    }
}

/**
 * @tc.number    : TaskDispatcher_Spec_03500
 * @tc.name      : spec delay:parallel apply
 * @tc.desc      : two levels task dispatch
 */
HWTEST_F(TaskDispatcherTest, TaskDispatcher_Spec_03500, Function | MediumTest | Level1)
{
    StartAbility(firstAbilityName, bundleNameFirst);
    std::string data = "Dispatcher_" + std::to_string((int)TestFunc::SPEC) + "_34";
    bool result = false;
    for (int i = 0; i < stLevel_.AMSLevel; i++) {
        STAbilityUtil::PublishEvent(g_EVENT_REQU_FIRST, ++testCaseCode, data);
        EXPECT_EQ(0, STAbilityUtil::WaitCompleted(event, g_EVENT_RESP_FIRST, testCaseCode, DELAY));
        data = STAbilityUtil::GetData(event, g_EVENT_RESP_FIRST, testCaseCode);
        result = data.compare("1") == 0;
        EXPECT_TRUE(result);
        if (!result && i > 0) {
            GTEST_LOG_(INFO) << "TaskDispatcher_Spec_03500 : " << i;
            break;
        }
    }
}

/**
 * @tc.number    : TaskDispatcher_Spec_03600
 * @tc.name      : spec delay:serial sync
 * @tc.desc      : two levels task dispatch
 */
HWTEST_F(TaskDispatcherTest, TaskDispatcher_Spec_03600, Function | MediumTest | Level1)
{
    StartAbility(firstAbilityName, bundleNameFirst);
    std::string data = "Dispatcher_" + std::to_string((int)TestFunc::SPEC) + "_35";
    bool result = false;
    for (int i = 0; i < stLevel_.AMSLevel; i++) {
        STAbilityUtil::PublishEvent(g_EVENT_REQU_FIRST, ++testCaseCode, data);
        EXPECT_EQ(0, STAbilityUtil::WaitCompleted(event, g_EVENT_RESP_FIRST, testCaseCode, DELAY));
        data = STAbilityUtil::GetData(event, g_EVENT_RESP_FIRST, testCaseCode);
        result = data.compare("1") == 0;
        EXPECT_TRUE(result);
        if (!result && i > 0) {
            GTEST_LOG_(INFO) << "TaskDispatcher_Spec_03600 : " << i;
            break;
        }
    }
}

/**
 * @tc.number    : TaskDispatcher_Spec_03700
 * @tc.name      : spec delay:serial async
 * @tc.desc      : two levels task dispatch
 */
HWTEST_F(TaskDispatcherTest, TaskDispatcher_Spec_03700, Function | MediumTest | Level1)
{
    StartAbility(firstAbilityName, bundleNameFirst);
    std::string data = "Dispatcher_" + std::to_string((int)TestFunc::SPEC) + "_36";
    bool result = false;
    for (int i = 0; i < stLevel_.AMSLevel; i++) {
        STAbilityUtil::PublishEvent(g_EVENT_REQU_FIRST, ++testCaseCode, data);
        EXPECT_EQ(0, STAbilityUtil::WaitCompleted(event, g_EVENT_RESP_FIRST, testCaseCode, DELAY));
        data = STAbilityUtil::GetData(event, g_EVENT_RESP_FIRST, testCaseCode);
        result = data.compare("1") == 0;
        EXPECT_TRUE(result);
        if (!result && i > 0) {
            GTEST_LOG_(INFO) << "TaskDispatcher_Spec_03700 : " << i;
            break;
        }
    }
}

/**
 * @tc.number    : TaskDispatcher_Spec_03800
 * @tc.name      : spec delay:serial delay
 * @tc.desc      : two levels task dispatch
 */
HWTEST_F(TaskDispatcherTest, TaskDispatcher_Spec_03800, Function | MediumTest | Level1)
{
    StartAbility(firstAbilityName, bundleNameFirst);
    std::string data = "Dispatcher_" + std::to_string((int)TestFunc::SPEC) + "_37";
    bool result = false;
    for (int i = 0; i < stLevel_.AMSLevel; i++) {
        STAbilityUtil::PublishEvent(g_EVENT_REQU_FIRST, ++testCaseCode, data);
        EXPECT_EQ(0, STAbilityUtil::WaitCompleted(event, g_EVENT_RESP_FIRST, testCaseCode, DELAY));
        data = STAbilityUtil::GetData(event, g_EVENT_RESP_FIRST, testCaseCode);
        result = data.compare("1") == 0;
        EXPECT_TRUE(result);
        if (!result && i > 0) {
            GTEST_LOG_(INFO) << "TaskDispatcher_Spec_03800 : " << i;
            break;
        }
    }
}

/**
 * @tc.number    : TaskDispatcher_Spec_03900
 * @tc.name      : spec delay:serial apply
 * @tc.desc      : two levels task dispatch
 */
HWTEST_F(TaskDispatcherTest, TaskDispatcher_Spec_03900, Function | MediumTest | Level1)
{
    StartAbility(firstAbilityName, bundleNameFirst);
    std::string data = "Dispatcher_" + std::to_string((int)TestFunc::SPEC) + "_38";
    bool result = false;
    for (int i = 0; i < stLevel_.AMSLevel; i++) {
        STAbilityUtil::PublishEvent(g_EVENT_REQU_FIRST, ++testCaseCode, data);
        EXPECT_EQ(0, STAbilityUtil::WaitCompleted(event, g_EVENT_RESP_FIRST, testCaseCode, DELAY));
        data = STAbilityUtil::GetData(event, g_EVENT_RESP_FIRST, testCaseCode);
        result = data.compare("1") == 0;
        EXPECT_TRUE(result);
        if (!result && i > 0) {
            GTEST_LOG_(INFO) << "TaskDispatcher_Spec_03900 : " << i;
            break;
        }
    }
}

/**
 * @tc.number    : TaskDispatcher_Spec_04000
 * @tc.name      : spec apply:parallel sync
 * @tc.desc      : two levels task dispatch
 */
HWTEST_F(TaskDispatcherTest, TaskDispatcher_Spec_04000, Function | MediumTest | Level1)
{
    StartAbility(firstAbilityName, bundleNameFirst);
    std::string data = "Dispatcher_" + std::to_string((int)TestFunc::SPEC) + "_39";
    bool result = false;
    for (int i = 0; i < stLevel_.AMSLevel; i++) {
        STAbilityUtil::PublishEvent(g_EVENT_REQU_FIRST, ++testCaseCode, data);
        EXPECT_EQ(0, STAbilityUtil::WaitCompleted(event, g_EVENT_RESP_FIRST, testCaseCode, DELAY));
        data = STAbilityUtil::GetData(event, g_EVENT_RESP_FIRST, testCaseCode);
        result = data.compare("1") == 0;
        EXPECT_TRUE(result);
        if (!result && i > 0) {
            GTEST_LOG_(INFO) << "TaskDispatcher_Spec_04000 : " << i;
            break;
        }
    }
}

/**
 * @tc.number    : TaskDispatcher_Spec_04100
 * @tc.name      : spec apply:parallel async
 * @tc.desc      : two levels task dispatch
 */
HWTEST_F(TaskDispatcherTest, TaskDispatcher_Spec_04100, Function | MediumTest | Level1)
{
    StartAbility(firstAbilityName, bundleNameFirst);
    std::string data = "Dispatcher_" + std::to_string((int)TestFunc::SPEC) + "_40";
    bool result = false;
    for (int i = 0; i < stLevel_.AMSLevel; i++) {
        STAbilityUtil::PublishEvent(g_EVENT_REQU_FIRST, ++testCaseCode, data);
        EXPECT_EQ(0, STAbilityUtil::WaitCompleted(event, g_EVENT_RESP_FIRST, testCaseCode, DELAY));
        data = STAbilityUtil::GetData(event, g_EVENT_RESP_FIRST, testCaseCode);
        result = data.compare("1") == 0;
        EXPECT_TRUE(result);
        if (!result && i > 0) {
            GTEST_LOG_(INFO) << "TaskDispatcher_Spec_04100 : " << i;
            break;
        }
    }
}

/**
 * @tc.number    : TaskDispatcher_Spec_04200
 * @tc.name      : spec apply:parallel delay
 * @tc.desc      : two levels task dispatch
 */
HWTEST_F(TaskDispatcherTest, TaskDispatcher_Spec_04200, Function | MediumTest | Level1)
{
    StartAbility(firstAbilityName, bundleNameFirst);
    std::string data = "Dispatcher_" + std::to_string((int)TestFunc::SPEC) + "_41";
    bool result = false;
    for (int i = 0; i < stLevel_.AMSLevel; i++) {
        STAbilityUtil::PublishEvent(g_EVENT_REQU_FIRST, ++testCaseCode, data);
        EXPECT_EQ(0, STAbilityUtil::WaitCompleted(event, g_EVENT_RESP_FIRST, testCaseCode, DELAY));
        data = STAbilityUtil::GetData(event, g_EVENT_RESP_FIRST, testCaseCode);
        result = data.compare("1") == 0;
        EXPECT_TRUE(result);
        if (!result && i > 0) {
            GTEST_LOG_(INFO) << "TaskDispatcher_Spec_04200 : " << i;
            break;
        }
    }
}

/**
 * @tc.number    : TaskDispatcher_Spec_04300
 * @tc.name      : spec apply:parallel group
 * @tc.desc      : two levels task dispatch
 */
HWTEST_F(TaskDispatcherTest, TaskDispatcher_Spec_04300, Function | MediumTest | Level1)
{
    StartAbility(firstAbilityName, bundleNameFirst);
    std::string data = "Dispatcher_" + std::to_string((int)TestFunc::SPEC) + "_42";
    bool result = false;
    for (int i = 0; i < stLevel_.AMSLevel; i++) {
        STAbilityUtil::PublishEvent(g_EVENT_REQU_FIRST, ++testCaseCode, data);
        EXPECT_EQ(0, STAbilityUtil::WaitCompleted(event, g_EVENT_RESP_FIRST, testCaseCode, DELAY));
        data = STAbilityUtil::GetData(event, g_EVENT_RESP_FIRST, testCaseCode);
        result = data.compare("1") == 0;
        EXPECT_TRUE(result);
        if (!result && i > 0) {
            GTEST_LOG_(INFO) << "TaskDispatcher_Spec_04300 : " << i;
            break;
        }
    }
}

/**
 * @tc.number    : TaskDispatcher_Spec_04400
 * @tc.name      : spec apply:parallel group wait
 * @tc.desc      : two levels task dispatch
 */
HWTEST_F(TaskDispatcherTest, TaskDispatcher_Spec_04400, Function | MediumTest | Level1)
{
    StartAbility(firstAbilityName, bundleNameFirst);
    std::string data = "Dispatcher_" + std::to_string((int)TestFunc::SPEC) + "_43";
    bool result = false;
    for (int i = 0; i < stLevel_.AMSLevel; i++) {
        STAbilityUtil::PublishEvent(g_EVENT_REQU_FIRST, ++testCaseCode, data);
        EXPECT_EQ(0, STAbilityUtil::WaitCompleted(event, g_EVENT_RESP_FIRST, testCaseCode, DELAY));
        data = STAbilityUtil::GetData(event, g_EVENT_RESP_FIRST, testCaseCode);
        result = data.compare("1") == 0;
        EXPECT_TRUE(result);
        if (!result && i > 0) {
            GTEST_LOG_(INFO) << "TaskDispatcher_Spec_04400 : " << i;
            break;
        }
    }
}

/**
 * @tc.number    : TaskDispatcher_Spec_04500
 * @tc.name      : spec apply:parallel group notify
 * @tc.desc      : two levels task dispatch
 */
HWTEST_F(TaskDispatcherTest, TaskDispatcher_Spec_04500, Function | MediumTest | Level1)
{
    StartAbility(firstAbilityName, bundleNameFirst);
    std::string data = "Dispatcher_" + std::to_string((int)TestFunc::SPEC) + "_44";
    bool result = false;
    for (int i = 0; i < stLevel_.AMSLevel; i++) {
        STAbilityUtil::PublishEvent(g_EVENT_REQU_FIRST, ++testCaseCode, data);
        EXPECT_EQ(0, STAbilityUtil::WaitCompleted(event, g_EVENT_RESP_FIRST, testCaseCode, DELAY));
        data = STAbilityUtil::GetData(event, g_EVENT_RESP_FIRST, testCaseCode);
        result = data.compare("1") == 0;
        EXPECT_TRUE(result);
        if (!result && i > 0) {
            GTEST_LOG_(INFO) << "TaskDispatcher_Spec_04500 : " << i;
            break;
        }
    }
}

/**
 * @tc.number    : TaskDispatcher_Spec_04600
 * @tc.name      : spec apply:parallel sync barrier
 * @tc.desc      : two levels task dispatch
 */
HWTEST_F(TaskDispatcherTest, TaskDispatcher_Spec_04600, Function | MediumTest | Level1)
{
    StartAbility(firstAbilityName, bundleNameFirst);
    std::string data = "Dispatcher_" + std::to_string((int)TestFunc::SPEC) + "_45";
    bool result = false;
    for (int i = 0; i < stLevel_.AMSLevel; i++) {
        STAbilityUtil::PublishEvent(g_EVENT_REQU_FIRST, ++testCaseCode, data);
        EXPECT_EQ(0, STAbilityUtil::WaitCompleted(event, g_EVENT_RESP_FIRST, testCaseCode, DELAY));
        data = STAbilityUtil::GetData(event, g_EVENT_RESP_FIRST, testCaseCode);
        result = data.compare("1") == 0;
        EXPECT_TRUE(result);
        if (!result && i > 0) {
            GTEST_LOG_(INFO) << "TaskDispatcher_Spec_04600 : " << i;
            break;
        }
    }
}

/**
 * @tc.number    : TaskDispatcher_Spec_04700
 * @tc.name      : spec apply:parallel async barrier
 * @tc.desc      : two levels task dispatch
 */
HWTEST_F(TaskDispatcherTest, TaskDispatcher_Spec_04700, Function | MediumTest | Level1)
{
    StartAbility(firstAbilityName, bundleNameFirst);
    std::string data = "Dispatcher_" + std::to_string((int)TestFunc::SPEC) + "_46";
    bool result = false;
    for (int i = 0; i < stLevel_.AMSLevel; i++) {
        STAbilityUtil::PublishEvent(g_EVENT_REQU_FIRST, ++testCaseCode, data);
        EXPECT_EQ(0, STAbilityUtil::WaitCompleted(event, g_EVENT_RESP_FIRST, testCaseCode, DELAY));
        data = STAbilityUtil::GetData(event, g_EVENT_RESP_FIRST, testCaseCode);
        result = data.compare("1") == 0;
        EXPECT_TRUE(result);
        if (!result && i > 0) {
            GTEST_LOG_(INFO) << "TaskDispatcher_Spec_04700 : " << i;
            break;
        }
    }
}

/**
 * @tc.number    : TaskDispatcher_Spec_04800
 * @tc.name      : spec apply:parallel apply
 * @tc.desc      : two levels task dispatch
 */
HWTEST_F(TaskDispatcherTest, TaskDispatcher_Spec_04800, Function | MediumTest | Level1)
{
    StartAbility(firstAbilityName, bundleNameFirst);
    std::string data = "Dispatcher_" + std::to_string((int)TestFunc::SPEC) + "_47";
    bool result = false;
    for (int i = 0; i < stLevel_.AMSLevel; i++) {
        STAbilityUtil::PublishEvent(g_EVENT_REQU_FIRST, ++testCaseCode, data);
        EXPECT_EQ(0, STAbilityUtil::WaitCompleted(event, g_EVENT_RESP_FIRST, testCaseCode, DELAY));
        data = STAbilityUtil::GetData(event, g_EVENT_RESP_FIRST, testCaseCode);
        result = data.compare("1") == 0;
        EXPECT_TRUE(result);
        if (!result && i > 0) {
            GTEST_LOG_(INFO) << "TaskDispatcher_Spec_04800 : " << i;
            break;
        }
    }
}

/**
 * @tc.number    : TaskDispatcher_Spec_04900
 * @tc.name      : spec apply:serial sync
 * @tc.desc      : two levels task dispatch
 */
HWTEST_F(TaskDispatcherTest, TaskDispatcher_Spec_04900, Function | MediumTest | Level1)
{
    StartAbility(firstAbilityName, bundleNameFirst);
    std::string data = "Dispatcher_" + std::to_string((int)TestFunc::SPEC) + "_48";
    bool result = false;
    for (int i = 0; i < stLevel_.AMSLevel; i++) {
        STAbilityUtil::PublishEvent(g_EVENT_REQU_FIRST, ++testCaseCode, data);
        EXPECT_EQ(0, STAbilityUtil::WaitCompleted(event, g_EVENT_RESP_FIRST, testCaseCode, DELAY));
        data = STAbilityUtil::GetData(event, g_EVENT_RESP_FIRST, testCaseCode);
        result = data.compare("1") == 0;
        EXPECT_TRUE(result);
        if (!result && i > 0) {
            GTEST_LOG_(INFO) << "TaskDispatcher_Spec_04900 : " << i;
            break;
        }
    }
}

/**
 * @tc.number    : TaskDispatcher_Spec_05000
 * @tc.name      : spec apply:serial async
 * @tc.desc      : two levels task dispatch
 */
HWTEST_F(TaskDispatcherTest, TaskDispatcher_Spec_05000, Function | MediumTest | Level1)
{
    StartAbility(firstAbilityName, bundleNameFirst);
    std::string data = "Dispatcher_" + std::to_string((int)TestFunc::SPEC) + "_49";
    bool result = false;
    for (int i = 0; i < stLevel_.AMSLevel; i++) {
        STAbilityUtil::PublishEvent(g_EVENT_REQU_FIRST, ++testCaseCode, data);
        EXPECT_EQ(0, STAbilityUtil::WaitCompleted(event, g_EVENT_RESP_FIRST, testCaseCode, DELAY));
        data = STAbilityUtil::GetData(event, g_EVENT_RESP_FIRST, testCaseCode);
        result = data.compare("1") == 0;
        EXPECT_TRUE(result);
        if (!result && i > 0) {
            GTEST_LOG_(INFO) << "TaskDispatcher_Spec_05000 : " << i;
            break;
        }
    }
}

/**
 * @tc.number    : TaskDispatcher_Spec_05100
 * @tc.name      : spec apply:serial delay
 * @tc.desc      : two levels task dispatch
 */
HWTEST_F(TaskDispatcherTest, TaskDispatcher_Spec_05100, Function | MediumTest | Level1)
{
    StartAbility(firstAbilityName, bundleNameFirst);
    std::string data = "Dispatcher_" + std::to_string((int)TestFunc::SPEC) + "_50";
    bool result = false;
    for (int i = 0; i < stLevel_.AMSLevel; i++) {
        STAbilityUtil::PublishEvent(g_EVENT_REQU_FIRST, ++testCaseCode, data);
        EXPECT_EQ(0, STAbilityUtil::WaitCompleted(event, g_EVENT_RESP_FIRST, testCaseCode, DELAY));
        data = STAbilityUtil::GetData(event, g_EVENT_RESP_FIRST, testCaseCode);
        result = data.compare("1") == 0;
        EXPECT_TRUE(result);
        if (!result && i > 0) {
            GTEST_LOG_(INFO) << "TaskDispatcher_Spec_05100 : " << i;
            break;
        }
    }
}

/**
 * @tc.number    : TaskDispatcher_Spec_05200
 * @tc.name      : spec apply:serial apply
 * @tc.desc      : two levels task dispatch
 */
HWTEST_F(TaskDispatcherTest, TaskDispatcher_Spec_05200, Function | MediumTest | Level1)
{
    StartAbility(firstAbilityName, bundleNameFirst);
    std::string data = "Dispatcher_" + std::to_string((int)TestFunc::SPEC) + "_51";
    bool result = false;
    for (int i = 0; i < stLevel_.AMSLevel; i++) {
        STAbilityUtil::PublishEvent(g_EVENT_REQU_FIRST, ++testCaseCode, data);
        EXPECT_EQ(0, STAbilityUtil::WaitCompleted(event, g_EVENT_RESP_FIRST, testCaseCode, DELAY));
        data = STAbilityUtil::GetData(event, g_EVENT_RESP_FIRST, testCaseCode);
        result = data.compare("1") == 0;
        EXPECT_TRUE(result);
        if (!result && i > 0) {
            GTEST_LOG_(INFO) << "TaskDispatcher_Spec_05200 : " << i;
            break;
        }
    }
}
/**
 * @tc.number    : TaskDispatcher_Hybird_00100
 * @tc.name      : GlobalTaskDispatcher all operation
 * @tc.desc      : dispatcher all operation in GolbalTaskDispatcher
 */
HWTEST_F(TaskDispatcherTest, TaskDispatcher_Hybird_00100, Function | MediumTest | Level1)
{
    StartAbility(firstAbilityName, bundleNameFirst);
    std::string data = "Dispatcher_" + std::to_string((int)TestFunc::HYBRID) + "_0";
    bool result = false;
    for (int i = 0; i < stLevel_.AMSLevel; i++) {
        STAbilityUtil::PublishEvent(g_EVENT_REQU_FIRST, ++testCaseCode, data);
        EXPECT_EQ(0, STAbilityUtil::WaitCompleted(event, g_EVENT_RESP_FIRST, testCaseCode, DELAY));
        data = STAbilityUtil::GetData(event, g_EVENT_RESP_FIRST, testCaseCode);
        result = data.compare("1") == 0;
        EXPECT_TRUE(result);
        if (!result && i > 0) {
            GTEST_LOG_(INFO) << "TaskDispatcher_Global_00100 : " << i;
            break;
        }
    }
}
/**
 * @tc.number    : TaskDispatcher_Hybird_00200
 * @tc.name      : ParallelTaskDispatcher all operation
 * @tc.desc      : dispatcher all operation in ParallelTaskDispatcher
 */
HWTEST_F(TaskDispatcherTest, TaskDispatcher_Hybird_00200, Function | MediumTest | Level1)
{
    StartAbility(firstAbilityName, bundleNameFirst);
    std::string data = "Dispatcher_" + std::to_string((int)TestFunc::HYBRID) + "_1";
    bool result = false;
    for (int i = 0; i < stLevel_.AMSLevel; i++) {
        STAbilityUtil::PublishEvent(g_EVENT_REQU_FIRST, ++testCaseCode, data);
        EXPECT_EQ(0, STAbilityUtil::WaitCompleted(event, g_EVENT_RESP_FIRST, testCaseCode, DELAY));
        data = STAbilityUtil::GetData(event, g_EVENT_RESP_FIRST, testCaseCode);
        result = data.compare("1") == 0;
        EXPECT_TRUE(result);
        if (!result && i > 0) {
            GTEST_LOG_(INFO) << "TaskDispatcher_Hybird_00200 : " << i;
            break;
        }
    }
}
/**
 * @tc.number    : TaskDispatcher_Hybird_00300
 * @tc.name      : SerialTaskDispatcher all operation
 * @tc.desc      : dispatcher all operation in SerialTaskDispatcher
 */
HWTEST_F(TaskDispatcherTest, TaskDispatcher_Hybird_00300, Function | MediumTest | Level1)
{
    StartAbility(firstAbilityName, bundleNameFirst);
    std::string data = "Dispatcher_" + std::to_string((int)TestFunc::HYBRID) + "_2";
    bool result = false;
    for (int i = 0; i < stLevel_.AMSLevel; i++) {
        STAbilityUtil::PublishEvent(g_EVENT_REQU_FIRST, ++testCaseCode, data);
        EXPECT_EQ(0, STAbilityUtil::WaitCompleted(event, g_EVENT_RESP_FIRST, testCaseCode, DELAY));
        data = STAbilityUtil::GetData(event, g_EVENT_RESP_FIRST, testCaseCode);
        result = data.compare("1") == 0;
        EXPECT_TRUE(result);
        if (!result && i > 0) {
            GTEST_LOG_(INFO) << "TaskDispatcher_Hybird_00300 : " << i;
            break;
        }
    }
}
/**
 * @tc.number    : TaskDispatcher_Hybird_00400
 * @tc.name      : SpecTaskDispatcher all operation
 * @tc.desc      : dispatcher all operation in GolbalTaskDispatcher
 */
HWTEST_F(TaskDispatcherTest, TaskDispatcher_Hybird_00400, Function | MediumTest | Level1)
{
    StartAbility(firstAbilityName, bundleNameFirst);
    std::string data = "Dispatcher_" + std::to_string((int)TestFunc::HYBRID) + "_3";
    bool result = false;
    for (int i = 0; i < stLevel_.AMSLevel; i++) {
        STAbilityUtil::PublishEvent(g_EVENT_REQU_FIRST, ++testCaseCode, data);
        EXPECT_EQ(0, STAbilityUtil::WaitCompleted(event, g_EVENT_RESP_FIRST, testCaseCode, DELAY));
        data = STAbilityUtil::GetData(event, g_EVENT_RESP_FIRST, testCaseCode);
        result = data.compare("1") == 0;
        EXPECT_TRUE(result);
        if (!result && i > 0) {
            GTEST_LOG_(INFO) << "TaskDispatcher_Hybird_00400 : " << i;
            break;
        }
    }
}
/**
 * @tc.number    : TaskDispatcher_Hybird_00500
 * @tc.name      : MutiDispatcher hybird
 * @tc.desc      : dispatcher task in MutiDispatcher by unorder operations
 */
HWTEST_F(TaskDispatcherTest, TaskDispatcher_Hybird_00500, Function | MediumTest | Level1)
{
    StartAbility(firstAbilityName, bundleNameFirst);
    std::string data = "Dispatcher_" + std::to_string((int)TestFunc::HYBRID) + "_4";
    bool result = false;
    for (int i = 0; i < stLevel_.AMSLevel; i++) {
        STAbilityUtil::PublishEvent(g_EVENT_REQU_FIRST, ++testCaseCode, data);
        EXPECT_EQ(0, STAbilityUtil::WaitCompleted(event, g_EVENT_RESP_FIRST, testCaseCode, DELAY));
        data = STAbilityUtil::GetData(event, g_EVENT_RESP_FIRST, testCaseCode);
        result = data.compare("1") == 0;
        EXPECT_TRUE(result);
        if (!result && i > 0) {
            GTEST_LOG_(INFO) << "TaskDispatcher_Hybird_00500 : " << i;
            break;
        }
    }
}
/**
 * @tc.number    : TaskDispatcher_Hybird_00600
 * @tc.name      : MutiDispatcher hybird
 * @tc.desc      : dispatcher task in MutiDispatcher by unorder operations
 */
HWTEST_F(TaskDispatcherTest, TaskDispatcher_Hybird_00600, Function | MediumTest | Level1)
{
    StartAbility(firstAbilityName, bundleNameFirst);
    std::string data = "Dispatcher_" + std::to_string((int)TestFunc::HYBRID) + "_5";
    bool result = false;
    for (int i = 0; i < stLevel_.AMSLevel; i++) {
        STAbilityUtil::PublishEvent(g_EVENT_REQU_FIRST, ++testCaseCode, data);
        EXPECT_EQ(0, STAbilityUtil::WaitCompleted(event, g_EVENT_RESP_FIRST, testCaseCode, DELAY));
        data = STAbilityUtil::GetData(event, g_EVENT_RESP_FIRST, testCaseCode);
        result = data.compare("1") == 0;
        EXPECT_TRUE(result);
        if (!result && i > 0) {
            GTEST_LOG_(INFO) << "TaskDispatcher_Hybird_00600 : " << i;
            break;
        }
    }
}
/**
 * @tc.number    : TaskDispatcher_Hybird_00700
 * @tc.name      : MutiDispatcher hybird
 * @tc.desc      : dispatcher task in MutiDispatcher by unorder operations
 */
HWTEST_F(TaskDispatcherTest, TaskDispatcher_Hybird_00700, Function | MediumTest | Level1)
{
    StartAbility(firstAbilityName, bundleNameFirst);
    std::string data = "Dispatcher_" + std::to_string((int)TestFunc::HYBRID) + "_6";
    bool result = false;
    for (int i = 0; i < stLevel_.AMSLevel; i++) {
        STAbilityUtil::PublishEvent(g_EVENT_REQU_FIRST, ++testCaseCode, data);
        EXPECT_EQ(0, STAbilityUtil::WaitCompleted(event, g_EVENT_RESP_FIRST, testCaseCode, DELAY));
        data = STAbilityUtil::GetData(event, g_EVENT_RESP_FIRST, testCaseCode);
        result = data.compare("1") == 0;
        EXPECT_TRUE(result);
        if (!result && i > 0) {
            GTEST_LOG_(INFO) << "TaskDispatcher_Hybird_00700 : " << i;
            break;
        }
    }
}
/**
 * @tc.number    : TaskDispatcher_Hybird_00800
 * @tc.name      : MutiDispatcher hybird
 * @tc.desc      : dispatcher task in MutiDispatcher by unorder operations
 */
HWTEST_F(TaskDispatcherTest, TaskDispatcher_Hybird_00800, Function | MediumTest | Level1)
{
    StartAbility(firstAbilityName, bundleNameFirst);
    std::string data = "Dispatcher_" + std::to_string((int)TestFunc::HYBRID) + "_7";
    bool result = false;
    for (int i = 0; i < stLevel_.AMSLevel; i++) {
        STAbilityUtil::PublishEvent(g_EVENT_REQU_FIRST, ++testCaseCode, data);
        EXPECT_EQ(0, STAbilityUtil::WaitCompleted(event, g_EVENT_RESP_FIRST, testCaseCode, DELAY));
        data = STAbilityUtil::GetData(event, g_EVENT_RESP_FIRST, testCaseCode);
        result = data.compare("1") == 0;
        EXPECT_TRUE(result);
        if (!result && i > 0) {
            GTEST_LOG_(INFO) << "TaskDispatcher_Hybird_00800 : " << i;
            break;
        }
    }
}
/**
 * @tc.number    : TaskDispatcher_Hybird_00900
 * @tc.name      : MutiDispatcher hybird
 * @tc.desc      : dispatcher task in MutiDispatcher by unorder operations
 */
HWTEST_F(TaskDispatcherTest, TaskDispatcher_Hybird_00900, Function | MediumTest | Level1)
{
    StartAbility(firstAbilityName, bundleNameFirst);
    std::string data = "Dispatcher_" + std::to_string((int)TestFunc::HYBRID) + "_8";
    bool result = false;
    for (int i = 0; i < stLevel_.AMSLevel; i++) {
        STAbilityUtil::PublishEvent(g_EVENT_REQU_FIRST, ++testCaseCode, data);
        EXPECT_EQ(0, STAbilityUtil::WaitCompleted(event, g_EVENT_RESP_FIRST, testCaseCode, DELAY));
        data = STAbilityUtil::GetData(event, g_EVENT_RESP_FIRST, testCaseCode);
        result = data.compare("1") == 0;
        EXPECT_TRUE(result);
        if (!result && i > 0) {
            GTEST_LOG_(INFO) << "TaskDispatcher_Hybird_00900 : " << i;
            break;
        }
    }
}
/**
 * @tc.number    : TaskDispatcher_Hybird_01000
 * @tc.name      : MutiDispatcher hybird
 * @tc.desc      : dispatcher task in MutiDispatcher by unorder operations
 */
HWTEST_F(TaskDispatcherTest, TaskDispatcher_Hybird_01000, Function | MediumTest | Level1)
{
    StartAbility(firstAbilityName, bundleNameFirst);
    std::string data = "Dispatcher_" + std::to_string((int)TestFunc::HYBRID) + "_9";
    bool result = false;
    for (int i = 0; i < stLevel_.AMSLevel; i++) {
        STAbilityUtil::PublishEvent(g_EVENT_REQU_FIRST, ++testCaseCode, data);
        EXPECT_EQ(0, STAbilityUtil::WaitCompleted(event, g_EVENT_RESP_FIRST, testCaseCode, DELAY));
        data = STAbilityUtil::GetData(event, g_EVENT_RESP_FIRST, testCaseCode);
        result = data.compare("1") == 0;
        EXPECT_TRUE(result);
        if (!result && i > 0) {
            GTEST_LOG_(INFO) << "TaskDispatcher_Hybird_01000 : " << i;
            break;
        }
    }
}
/**
 * @tc.number    : TaskDispatcher_Hybird_01100
 * @tc.name      : MutiDispatcher hybird
 * @tc.desc      : dispatcher task in MutiDispatcher by unorder operations
 */
HWTEST_F(TaskDispatcherTest, TaskDispatcher_Hybird_01100, Function | MediumTest | Level1)
{
    StartAbility(firstAbilityName, bundleNameFirst);
    std::string data = "Dispatcher_" + std::to_string((int)TestFunc::HYBRID) + "_10";
    bool result = false;
    for (int i = 0; i < stLevel_.AMSLevel; i++) {
        STAbilityUtil::PublishEvent(g_EVENT_REQU_FIRST, ++testCaseCode, data);
        EXPECT_EQ(0, STAbilityUtil::WaitCompleted(event, g_EVENT_RESP_FIRST, testCaseCode, DELAY));
        data = STAbilityUtil::GetData(event, g_EVENT_RESP_FIRST, testCaseCode);
        result = data.compare("1") == 0;
        EXPECT_TRUE(result);
        if (!result && i > 0) {
            GTEST_LOG_(INFO) << "TaskDispatcher_Hybird_01100 : " << i;
            break;
        }
    }
}
/**
 * @tc.number    : TaskDispatcher_Hybird_01200
 * @tc.name      : MutiDispatcher hybird
 * @tc.desc      : dispatcher task in MutiDispatcher by unorder operations
 */
HWTEST_F(TaskDispatcherTest, TaskDispatcher_Hybird_01200, Function | MediumTest | Level1)
{
    StartAbility(firstAbilityName, bundleNameFirst);
    std::string data = "Dispatcher_" + std::to_string((int)TestFunc::HYBRID) + "_11";
    bool result = false;
    for (int i = 0; i < stLevel_.AMSLevel; i++) {
        STAbilityUtil::PublishEvent(g_EVENT_REQU_FIRST, ++testCaseCode, data);
        EXPECT_EQ(0, STAbilityUtil::WaitCompleted(event, g_EVENT_RESP_FIRST, testCaseCode, DELAY));
        data = STAbilityUtil::GetData(event, g_EVENT_RESP_FIRST, testCaseCode);
        result = data.compare("1") == 0;
        EXPECT_TRUE(result);
        if (!result && i > 0) {
            GTEST_LOG_(INFO) << "TaskDispatcher_Hybird_01200 : " << i;
            break;
        }
    }
}
/**
 * @tc.number    : TaskDispatcher_Hybird_01300
 * @tc.name      : MutiDispatcher hybird
 * @tc.desc      : dispatcher task in MutiDispatcher by unorder operations
 */
HWTEST_F(TaskDispatcherTest, TaskDispatcher_Hybird_01300, Function | MediumTest | Level1)
{
    StartAbility(firstAbilityName, bundleNameFirst);
    std::string data = "Dispatcher_" + std::to_string((int)TestFunc::HYBRID) + "_12";
    bool result = false;
    for (int i = 0; i < stLevel_.AMSLevel; i++) {
        STAbilityUtil::PublishEvent(g_EVENT_REQU_FIRST, ++testCaseCode, data);
        EXPECT_EQ(0, STAbilityUtil::WaitCompleted(event, g_EVENT_RESP_FIRST, testCaseCode, DELAY));
        data = STAbilityUtil::GetData(event, g_EVENT_RESP_FIRST, testCaseCode);
        result = data.compare("1") == 0;
        EXPECT_TRUE(result);
        if (!result && i > 0) {
            GTEST_LOG_(INFO) << "TaskDispatcher_Hybird_01300 : " << i;
            break;
        }
    }
}
/**
 * @tc.number    : TaskDispatcher_Hybird_01400
 * @tc.name      : MutiDispatcher hybird
 * @tc.desc      : dispatcher task in MutiDispatcher by unorder operations
 */
HWTEST_F(TaskDispatcherTest, TaskDispatcher_Hybird_01400, Function | MediumTest | Level1)
{
    StartAbility(firstAbilityName, bundleNameFirst);
    std::string data = "Dispatcher_" + std::to_string((int)TestFunc::HYBRID) + "_13";
    bool result = false;
    for (int i = 0; i < stLevel_.AMSLevel; i++) {
        STAbilityUtil::PublishEvent(g_EVENT_REQU_FIRST, ++testCaseCode, data);
        EXPECT_EQ(0, STAbilityUtil::WaitCompleted(event, g_EVENT_RESP_FIRST, testCaseCode, DELAY));
        data = STAbilityUtil::GetData(event, g_EVENT_RESP_FIRST, testCaseCode);
        result = data.compare("1") == 0;
        EXPECT_TRUE(result);
        if (!result && i > 0) {
            GTEST_LOG_(INFO) << "TaskDispatcher_Hybird_01400 : " << i;
            break;
        }
    }
}
/**
 * @tc.number    : TaskDispatcher_Hybird_01500
 * @tc.name      : MutiDispatcher hybird
 * @tc.desc      : dispatcher task in MutiDispatcher by unorder operations
 */
HWTEST_F(TaskDispatcherTest, TaskDispatcher_Hybird_01500, Function | MediumTest | Level1)
{
    StartAbility(firstAbilityName, bundleNameFirst);
    std::string data = "Dispatcher_" + std::to_string((int)TestFunc::HYBRID) + "_14";
    bool result = false;
    for (int i = 0; i < stLevel_.AMSLevel; i++) {
        STAbilityUtil::PublishEvent(g_EVENT_REQU_FIRST, ++testCaseCode, data);
        EXPECT_EQ(0, STAbilityUtil::WaitCompleted(event, g_EVENT_RESP_FIRST, testCaseCode, DELAY));
        data = STAbilityUtil::GetData(event, g_EVENT_RESP_FIRST, testCaseCode);
        result = data.compare("1") == 0;
        EXPECT_TRUE(result);
        if (!result && i > 0) {
            GTEST_LOG_(INFO) << "TaskDispatcher_Hybird_01500 : " << i;
            break;
        }
    }
}
/**
 * @tc.number    : TaskDispatcher_Hybird_01600
 * @tc.name      : MutiDispatcher hybird
 * @tc.desc      : dispatcher task in MutiDispatcher by unorder operations
 */
HWTEST_F(TaskDispatcherTest, TaskDispatcher_Hybird_01600, Function | MediumTest | Level1)
{
    StartAbility(firstAbilityName, bundleNameFirst);
    std::string data = "Dispatcher_" + std::to_string((int)TestFunc::HYBRID) + "_15";
    bool result = false;
    for (int i = 0; i < stLevel_.AMSLevel; i++) {
        STAbilityUtil::PublishEvent(g_EVENT_REQU_FIRST, ++testCaseCode, data);
        EXPECT_EQ(0, STAbilityUtil::WaitCompleted(event, g_EVENT_RESP_FIRST, testCaseCode, DELAY));
        data = STAbilityUtil::GetData(event, g_EVENT_RESP_FIRST, testCaseCode);
        result = data.compare("1") == 0;
        EXPECT_TRUE(result);
        if (!result && i > 0) {
            GTEST_LOG_(INFO) << "TaskDispatcher_Hybird_01600 : " << i;
            break;
        }
    }
}
/**
 * @tc.number    : TaskDispatcher_Hybird_01700
 * @tc.name      : MutiDispatcher hybird
 * @tc.desc      : dispatcher task in MutiDispatcher by unorder operations
 */
HWTEST_F(TaskDispatcherTest, TaskDispatcher_Hybird_01700, Function | MediumTest | Level1)
{
    StartAbility(firstAbilityName, bundleNameFirst);
    std::string data = "Dispatcher_" + std::to_string((int)TestFunc::HYBRID) + "_16";
    bool result = false;
    for (int i = 0; i < stLevel_.AMSLevel; i++) {
        STAbilityUtil::PublishEvent(g_EVENT_REQU_FIRST, ++testCaseCode, data);
        EXPECT_EQ(0, STAbilityUtil::WaitCompleted(event, g_EVENT_RESP_FIRST, testCaseCode, DELAY));
        data = STAbilityUtil::GetData(event, g_EVENT_RESP_FIRST, testCaseCode);
        result = data.compare("1") == 0;
        EXPECT_TRUE(result);
        if (!result && i > 0) {
            GTEST_LOG_(INFO) << "TaskDispatcher_Hybird_01700 : " << i;
            break;
        }
    }
}
/**
 * @tc.number    : TaskDispatcher_Hybird_01800
 * @tc.name      : MutiDispatcher hybird
 * @tc.desc      : dispatcher task in MutiDispatcher by unorder operations
 */
HWTEST_F(TaskDispatcherTest, TaskDispatcher_Hybird_01800, Function | MediumTest | Level1)
{
    StartAbility(firstAbilityName, bundleNameFirst);
    std::string data = "Dispatcher_" + std::to_string((int)TestFunc::HYBRID) + "_17";
    bool result = false;
    for (int i = 0; i < stLevel_.AMSLevel; i++) {
        STAbilityUtil::PublishEvent(g_EVENT_REQU_FIRST, ++testCaseCode, data);
        EXPECT_EQ(0, STAbilityUtil::WaitCompleted(event, g_EVENT_RESP_FIRST, testCaseCode, DELAY));
        data = STAbilityUtil::GetData(event, g_EVENT_RESP_FIRST, testCaseCode);
        result = data.compare("1") == 0;
        EXPECT_TRUE(result);
        if (!result && i > 0) {
            GTEST_LOG_(INFO) << "TaskDispatcher_Hybird_01800 : " << i;
            break;
        }
    }
}
/**
 * @tc.number    : TaskDispatcher_Hybird_01900
 * @tc.name      : MutiDispatcher hybird
 * @tc.desc      : dispatcher task in MutiDispatcher by unorder operations
 */
HWTEST_F(TaskDispatcherTest, TaskDispatcher_Hybird_01900, Function | MediumTest | Level1)
{
    StartAbility(firstAbilityName, bundleNameFirst);
    std::string data = "Dispatcher_" + std::to_string((int)TestFunc::HYBRID) + "_18";
    bool result = false;
    for (int i = 0; i < stLevel_.AMSLevel; i++) {
        STAbilityUtil::PublishEvent(g_EVENT_REQU_FIRST, ++testCaseCode, data);
        EXPECT_EQ(0, STAbilityUtil::WaitCompleted(event, g_EVENT_RESP_FIRST, testCaseCode, DELAY));
        data = STAbilityUtil::GetData(event, g_EVENT_RESP_FIRST, testCaseCode);
        result = data.compare("1") == 0;
        EXPECT_TRUE(result);
        if (!result && i > 0) {
            GTEST_LOG_(INFO) << "TaskDispatcher_Hybird_01900 : " << i;
            break;
        }
    }
}
/**
 * @tc.number    : TaskDispatcher_Hybird_02000
 * @tc.name      : MutiDispatcher hybird
 * @tc.desc      : dispatcher task in MutiDispatcher by unorder operations
 */
HWTEST_F(TaskDispatcherTest, TaskDispatcher_Hybird_02000, Function | MediumTest | Level1)
{
    StartAbility(firstAbilityName, bundleNameFirst);
    std::string data = "Dispatcher_" + std::to_string((int)TestFunc::HYBRID) + "_19";
    bool result = false;
    for (int i = 0; i < stLevel_.AMSLevel; i++) {
        STAbilityUtil::PublishEvent(g_EVENT_REQU_FIRST, ++testCaseCode, data);
        EXPECT_EQ(0, STAbilityUtil::WaitCompleted(event, g_EVENT_RESP_FIRST, testCaseCode, DELAY));
        data = STAbilityUtil::GetData(event, g_EVENT_RESP_FIRST, testCaseCode);
        result = data.compare("1") == 0;
        EXPECT_TRUE(result);
        if (!result && i > 0) {
            GTEST_LOG_(INFO) << "TaskDispatcher_Hybird_02000 : " << i;
            break;
        }
    }
}
/**
 * @tc.number    : TaskDispatcher_Hybird_02100
 * @tc.name      : MutiDispatcher hybird
 * @tc.desc      : dispatcher task in MutiDispatcher by unorder operations
 */
HWTEST_F(TaskDispatcherTest, TaskDispatcher_Hybird_02100, Function | MediumTest | Level1)
{
    StartAbility(firstAbilityName, bundleNameFirst);
    std::string data = "Dispatcher_" + std::to_string((int)TestFunc::HYBRID) + "_20";
    bool result = false;
    for (int i = 0; i < stLevel_.AMSLevel; i++) {
        STAbilityUtil::PublishEvent(g_EVENT_REQU_FIRST, ++testCaseCode, data);
        EXPECT_EQ(0, STAbilityUtil::WaitCompleted(event, g_EVENT_RESP_FIRST, testCaseCode, DELAY));
        data = STAbilityUtil::GetData(event, g_EVENT_RESP_FIRST, testCaseCode);
        result = data.compare("1") == 0;
        EXPECT_TRUE(result);
        if (!result && i > 0) {
            GTEST_LOG_(INFO) << "TaskDispatcher_Hybird_02100 : " << i;
            break;
        }
    }
}
/**
 * @tc.number    : TaskDispatcher_Hybird_02200
 * @tc.name      : MutiDispatcher hybird
 * @tc.desc      : dispatcher task in MutiDispatcher by unorder operations
 */
HWTEST_F(TaskDispatcherTest, TaskDispatcher_Hybird_02200, Function | MediumTest | Level1)
{
    StartAbility(firstAbilityName, bundleNameFirst);
    std::string data = "Dispatcher_" + std::to_string((int)TestFunc::HYBRID) + "_21";
    bool result = false;
    for (int i = 0; i < stLevel_.AMSLevel; i++) {
        STAbilityUtil::PublishEvent(g_EVENT_REQU_FIRST, ++testCaseCode, data);
        EXPECT_EQ(0, STAbilityUtil::WaitCompleted(event, g_EVENT_RESP_FIRST, testCaseCode, DELAY));
        data = STAbilityUtil::GetData(event, g_EVENT_RESP_FIRST, testCaseCode);
        result = data.compare("1") == 0;
        EXPECT_TRUE(result);
        if (!result && i > 0) {
            GTEST_LOG_(INFO) << "TaskDispatcher_Hybird_02200 : " << i;
            break;
        }
    }
}
/**
 * @tc.number    : TaskDispatcher_Hybird_00500
 * @tc.name      : MutiDispatcher hybird
 * @tc.desc      : dispatcher task in MutiDispatcher by unorder operations
 */
HWTEST_F(TaskDispatcherTest, TaskDispatcher_Hybird_02300, Function | MediumTest | Level1)
{
    StartAbility(firstAbilityName, bundleNameFirst);
    std::string data = "Dispatcher_" + std::to_string((int)TestFunc::HYBRID) + "_22";
    bool result = false;
    for (int i = 0; i < stLevel_.AMSLevel; i++) {
        STAbilityUtil::PublishEvent(g_EVENT_REQU_FIRST, ++testCaseCode, data);
        EXPECT_EQ(0, STAbilityUtil::WaitCompleted(event, g_EVENT_RESP_FIRST, testCaseCode, DELAY));
        data = STAbilityUtil::GetData(event, g_EVENT_RESP_FIRST, testCaseCode);
        result = data.compare("1") == 0;
        EXPECT_TRUE(result);
        if (!result && i > 0) {
            GTEST_LOG_(INFO) << "TaskDispatcher_Hybird_02300 : " << i;
            break;
        }
    }
}
/**
 * @tc.number    : TaskDispatcher_Hybird_02400
 * @tc.name      : MutiDispatcher hybird
 * @tc.desc      : dispatcher task in MutiDispatcher by unorder operations
 */
HWTEST_F(TaskDispatcherTest, TaskDispatcher_Hybird_02400, Function | MediumTest | Level1)
{
    StartAbility(firstAbilityName, bundleNameFirst);
    std::string data = "Dispatcher_" + std::to_string((int)TestFunc::HYBRID) + "_23";
    bool result = false;
    for (int i = 0; i < stLevel_.AMSLevel; i++) {
        STAbilityUtil::PublishEvent(g_EVENT_REQU_FIRST, ++testCaseCode, data);
        EXPECT_EQ(0, STAbilityUtil::WaitCompleted(event, g_EVENT_RESP_FIRST, testCaseCode, DELAY));
        data = STAbilityUtil::GetData(event, g_EVENT_RESP_FIRST, testCaseCode);
        result = data.compare("1") == 0;
        EXPECT_TRUE(result);
        if (!result && i > 0) {
            GTEST_LOG_(INFO) << "TaskDispatcher_Hybird_02400 : " << i;
            break;
        }
    }
}
/**
 * @tc.number    : TaskDispatcher_Hybird_02500
 * @tc.name      : MutiDispatcher hybird
 * @tc.desc      : dispatcher task in MutiDispatcher by unorder operations
 */
HWTEST_F(TaskDispatcherTest, TaskDispatcher_Hybird_02500, Function | MediumTest | Level1)
{
    StartAbility(firstAbilityName, bundleNameFirst);
    std::string data = "Dispatcher_" + std::to_string((int)TestFunc::HYBRID) + "_24";
    bool result = false;
    for (int i = 0; i < stLevel_.AMSLevel; i++) {
        STAbilityUtil::PublishEvent(g_EVENT_REQU_FIRST, ++testCaseCode, data);
        EXPECT_EQ(0, STAbilityUtil::WaitCompleted(event, g_EVENT_RESP_FIRST, testCaseCode, DELAY));
        data = STAbilityUtil::GetData(event, g_EVENT_RESP_FIRST, testCaseCode);
        result = data.compare("1") == 0;
        EXPECT_TRUE(result);
        if (!result && i > 0) {
            GTEST_LOG_(INFO) << "TaskDispatcher_Hybird_02500 : " << i;
            break;
        }
    }
}
/**
 * @tc.number    : TaskDispatcher_Hybird_02600
 * @tc.name      : MutiDispatcher hybird
 * @tc.desc      : dispatcher task in MutiDispatcher by unorder operations
 */
HWTEST_F(TaskDispatcherTest, TaskDispatcher_Hybird_02600, Function | MediumTest | Level1)
{
    StartAbility(firstAbilityName, bundleNameFirst);
    std::string data = "Dispatcher_" + std::to_string((int)TestFunc::HYBRID) + "_25";
    bool result = false;
    for (int i = 0; i < stLevel_.AMSLevel; i++) {
        STAbilityUtil::PublishEvent(g_EVENT_REQU_FIRST, ++testCaseCode, data);
        EXPECT_EQ(0, STAbilityUtil::WaitCompleted(event, g_EVENT_RESP_FIRST, testCaseCode, DELAY));
        data = STAbilityUtil::GetData(event, g_EVENT_RESP_FIRST, testCaseCode);
        result = data.compare("1") == 0;
        EXPECT_TRUE(result);
        if (!result && i > 0) {
            GTEST_LOG_(INFO) << "TaskDispatcher_Hybird_02600 : " << i;
            break;
        }
    }
}
/**
 * @tc.number    : TaskDispatcher_Hybird_02700
 * @tc.name      : MutiDispatcher hybird
 * @tc.desc      : dispatcher task in MutiDispatcher by unorder operations
 */
HWTEST_F(TaskDispatcherTest, TaskDispatcher_Hybird_02700, Function | MediumTest | Level1)
{
    StartAbility(firstAbilityName, bundleNameFirst);
    std::string data = "Dispatcher_" + std::to_string((int)TestFunc::HYBRID) + "_26";
    bool result = false;
    for (int i = 0; i < stLevel_.AMSLevel; i++) {
        STAbilityUtil::PublishEvent(g_EVENT_REQU_FIRST, ++testCaseCode, data);
        EXPECT_EQ(0, STAbilityUtil::WaitCompleted(event, g_EVENT_RESP_FIRST, testCaseCode, DELAY));
        data = STAbilityUtil::GetData(event, g_EVENT_RESP_FIRST, testCaseCode);
        result = data.compare("1") == 0;
        EXPECT_TRUE(result);
        if (!result && i > 0) {
            GTEST_LOG_(INFO) << "TaskDispatcher_Hybird_02700 : " << i;
            break;
        }
    }
}

/**
 * @tc.number    : TaskDispatcher_Extra_00100
 * @tc.name      : start ability in Dispatcher
 * @tc.desc      : start service ability in dispatcher
 */
HWTEST_F(TaskDispatcherTest, TaskDispatcher_Extra_00100, Function | MediumTest | Level1)
{
    StartAbility(firstAbilityName, bundleNameFirst);
    std::string data = "Dispatcher_" + std::to_string((int)TestFunc::EXTRA) + "_0";
    bool result = false;
    for (int i = 0; i < stLevel_.AMSLevel; i++) {
        STAbilityUtil::PublishEvent(g_EVENT_REQU_FIRST, ++testCaseCode, data);
        EXPECT_EQ(0, STAbilityUtil::WaitCompleted(event, g_EVENT_RESP_FIRST, testCaseCode, DELAY));
        data = STAbilityUtil::GetData(event, g_EVENT_RESP_FIRST, testCaseCode);
        result = data.compare("1") == 0;
        EXPECT_EQ(0, STAbilityUtil::WaitCompleted(event, g_EVENT_RESP_SECOND, 0, DELAY));
        EXPECT_TRUE(result);
        if (!result && i > 0) {
            GTEST_LOG_(INFO) << "TaskDispatcher_Rxtra_00100 : " << i;
            break;
        }
    }
}

/**
 * @tc.number    : TaskDispatcher_Priority_00100
 * @tc.name      : task Dispatcher priority
 * @tc.desc      : use dispatcher in different prioritys
 */
HWTEST_F(TaskDispatcherTest, TaskDispatcher_Priority_00100, Function | MediumTest | Level1)
{
    StartAbility(firstAbilityName, bundleNameFirst);
    std::string data = "Dispatcher_" + std::to_string((int)TestFunc::PRIORITY) + "_0";
    bool result = false;
    for (int i = 0; i < stLevel_.AMSLevel; i++) {
        STAbilityUtil::PublishEvent(g_EVENT_REQU_FIRST, ++testCaseCode, data);
        EXPECT_EQ(0, STAbilityUtil::WaitCompleted(event, g_EVENT_RESP_FIRST, testCaseCode, DELAY));
        data = STAbilityUtil::GetData(event, g_EVENT_RESP_FIRST, testCaseCode);
        result = data.compare("1") == 0;
        EXPECT_TRUE(result);
        if (!result && i > 0) {
            GTEST_LOG_(INFO) << "TaskDispatcher_Priority_00100 : " << i;
            break;
        }
    }
}

/**
 * @tc.number    : TaskDispatcher_Priority_00200
 * @tc.name      : task Dispatcher priority
 * @tc.desc      : use dispatcher in different prioritys
 */
HWTEST_F(TaskDispatcherTest, TaskDispatcher_Priority_00200, Function | MediumTest | Level1)
{
    StartAbility(firstAbilityName, bundleNameFirst);
    std::string data = "Dispatcher_" + std::to_string((int)TestFunc::PRIORITY) + "_1";
    bool result = false;
    for (int i = 0; i < stLevel_.AMSLevel; i++) {
        STAbilityUtil::PublishEvent(g_EVENT_REQU_FIRST, ++testCaseCode, data);
        EXPECT_EQ(0, STAbilityUtil::WaitCompleted(event, g_EVENT_RESP_FIRST, testCaseCode, DELAY));
        data = STAbilityUtil::GetData(event, g_EVENT_RESP_FIRST, testCaseCode);
        result = data.compare("1") == 0;
        EXPECT_TRUE(result);
        if (!result && i > 0) {
            GTEST_LOG_(INFO) << "TaskDispatcher_Priority_00200 : " << i;
            break;
        }
    }
}

/**
 * @tc.number    : TaskDispatcher_Priority_00300
 * @tc.name      : task Dispatcher priority
 * @tc.desc      : use dispatcher in different prioritys
 */
HWTEST_F(TaskDispatcherTest, TaskDispatcher_Priority_00300, Function | MediumTest | Level1)
{
    StartAbility(firstAbilityName, bundleNameFirst);
    std::string data = "Dispatcher_" + std::to_string((int)TestFunc::PRIORITY) + "_2";
    bool result = false;
    for (int i = 0; i < stLevel_.AMSLevel; i++) {
        STAbilityUtil::PublishEvent(g_EVENT_REQU_FIRST, ++testCaseCode, data);
        EXPECT_EQ(0, STAbilityUtil::WaitCompleted(event, g_EVENT_RESP_FIRST, testCaseCode, DELAY));
        data = STAbilityUtil::GetData(event, g_EVENT_RESP_FIRST, testCaseCode);
        result = data.compare("1") == 0;
        EXPECT_TRUE(result);
        if (!result && i > 0) {
            GTEST_LOG_(INFO) << "TaskDispatcher_Priority_00300 : " << i;
            break;
        }
    }
}

/**
 * @tc.number    : TaskDispatcher_Priority_00400
 * @tc.name      : task Dispatcher priority
 * @tc.desc      : use dispatcher in different prioritys
 */
HWTEST_F(TaskDispatcherTest, TaskDispatcher_Priority_00400, Function | MediumTest | Level1)
{
    StartAbility(firstAbilityName, bundleNameFirst);
    std::string data = "Dispatcher_" + std::to_string((int)TestFunc::PRIORITY) + "_3";
    bool result = false;
    for (int i = 0; i < stLevel_.AMSLevel; i++) {
        STAbilityUtil::PublishEvent(g_EVENT_REQU_FIRST, ++testCaseCode, data);
        EXPECT_EQ(0, STAbilityUtil::WaitCompleted(event, g_EVENT_RESP_FIRST, testCaseCode, DELAY));
        data = STAbilityUtil::GetData(event, g_EVENT_RESP_FIRST, testCaseCode);
        result = data.compare("1") == 0;
        EXPECT_TRUE(result);
        if (!result && i > 0) {
            GTEST_LOG_(INFO) << "TaskDispatcher_Priority_00400 : " << i;
            break;
        }
    }
}

/**
 * @tc.number    : TaskDispatcher_Priority_00500
 * @tc.name      : task Dispatcher priority
 * @tc.desc      : use dispatcher in different prioritys
 */
HWTEST_F(TaskDispatcherTest, TaskDispatcher_Priority_00500, Function | MediumTest | Level1)
{
    StartAbility(firstAbilityName, bundleNameFirst);
    std::string data = "Dispatcher_" + std::to_string((int)TestFunc::PRIORITY) + "_4";
    bool result = false;
    for (int i = 0; i < stLevel_.AMSLevel; i++) {
        STAbilityUtil::PublishEvent(g_EVENT_REQU_FIRST, ++testCaseCode, data);
        EXPECT_EQ(0, STAbilityUtil::WaitCompleted(event, g_EVENT_RESP_FIRST, testCaseCode, DELAY));
        data = STAbilityUtil::GetData(event, g_EVENT_RESP_FIRST, testCaseCode);
        result = data.compare("1") == 0;
        EXPECT_TRUE(result);
        if (!result && i > 0) {
            GTEST_LOG_(INFO) << "TaskDispatcher_Priority_00500 : " << i;
            break;
        }
    }
}

/**
 * @tc.number    : TaskDispatcher_CancelTask_00100
 * @tc.name      : cancel async task successed
 * @tc.desc      : cancel async task successed
 */
HWTEST_F(TaskDispatcherTest, TaskDispatcher_CancelTask_00100, Function | MediumTest | Level1)
{
    StartAbility(firstAbilityName, bundleNameFirst);
    std::string data = "Dispatcher_" + std::to_string((int)TestFunc::CANCEL_TASK) + "_0";
    bool result = false;
    for (int i = 0; i < stLevel_.AMSLevel; i++) {
        STAbilityUtil::PublishEvent(g_EVENT_REQU_FIRST, ++testCaseCode, data);
        EXPECT_EQ(0, STAbilityUtil::WaitCompleted(event, g_EVENT_RESP_FIRST, testCaseCode, DELAY));
        data = STAbilityUtil::GetData(event, g_EVENT_RESP_FIRST, testCaseCode);
        result = data.compare("1") == 0;
        EXPECT_TRUE(result);
        if (!result && i > 0) {
            GTEST_LOG_(INFO) << "TaskDispatcher_CancelTask_00100 : " << i;
            break;
        }
    }
}

/**
 * @tc.number    : TaskDispatcher_CancelTask_00200
 * @tc.name      : cancel async task failed
 * @tc.desc      : cancel async task failed
 */
HWTEST_F(TaskDispatcherTest, TaskDispatcher_CancelTask_00200, Function | MediumTest | Level1)
{
    StartAbility(firstAbilityName, bundleNameFirst);
    std::string data = "Dispatcher_" + std::to_string((int)TestFunc::CANCEL_TASK) + "_1";
    bool result = false;
    for (int i = 0; i < stLevel_.AMSLevel; i++) {
        STAbilityUtil::PublishEvent(g_EVENT_REQU_FIRST, ++testCaseCode, data);
        EXPECT_EQ(0, STAbilityUtil::WaitCompleted(event, g_EVENT_RESP_FIRST, testCaseCode, DELAY));
        data = STAbilityUtil::GetData(event, g_EVENT_RESP_FIRST, testCaseCode);
        result = data.compare("1") == 0;
        EXPECT_TRUE(result);
        if (!result && i > 0) {
            GTEST_LOG_(INFO) << "TaskDispatcher_CancelTask_00200 : " << i;
            break;
        }
    }
}

/**
 * @tc.number    : TaskDispatcher_CancelTask_00300
 * @tc.name      : cancel delay task successed
 * @tc.desc      : cancel delay task successed
 */
HWTEST_F(TaskDispatcherTest, TaskDispatcher_CancelTask_00300, Function | MediumTest | Level1)
{
    StartAbility(firstAbilityName, bundleNameFirst);
    std::string data = "Dispatcher_" + std::to_string((int)TestFunc::CANCEL_TASK) + "_2";
    bool result = false;
    for (int i = 0; i < stLevel_.AMSLevel; i++) {
        STAbilityUtil::PublishEvent(g_EVENT_REQU_FIRST, ++testCaseCode, data);
        EXPECT_EQ(0, STAbilityUtil::WaitCompleted(event, g_EVENT_RESP_FIRST, testCaseCode, DELAY));
        data = STAbilityUtil::GetData(event, g_EVENT_RESP_FIRST, testCaseCode);
        result = data.compare("1") == 0;
        EXPECT_TRUE(result);
        if (!result && i > 0) {
            GTEST_LOG_(INFO) << "TaskDispatcher_CancelTask_00300 : " << i;
            break;
        }
    }
}

/**
 * @tc.number    : TaskDispatcher_CancelTask_00400
 * @tc.name      : cancel delay task failed
 * @tc.desc      : cancel delay task failed
 */
HWTEST_F(TaskDispatcherTest, TaskDispatcher_CancelTask_00400, Function | MediumTest | Level1)
{
    StartAbility(firstAbilityName, bundleNameFirst);
    std::string data = "Dispatcher_" + std::to_string((int)TestFunc::CANCEL_TASK) + "_3";
    bool result = false;
    for (int i = 0; i < stLevel_.AMSLevel; i++) {
        STAbilityUtil::PublishEvent(g_EVENT_REQU_FIRST, ++testCaseCode, data);
        EXPECT_EQ(0, STAbilityUtil::WaitCompleted(event, g_EVENT_RESP_FIRST, testCaseCode, DELAY));
        data = STAbilityUtil::GetData(event, g_EVENT_RESP_FIRST, testCaseCode);
        result = data.compare("1") == 0;
        EXPECT_TRUE(result);
        if (!result && i > 0) {
            GTEST_LOG_(INFO) << "TaskDispatcher_CancelTask_00400 : " << i;
            break;
        }
    }
}

/**
 * @tc.number    : TaskDispatcher_CancelTask_00500
 * @tc.name      : cancel async_group task successed
 * @tc.desc      : cancel async_group task successed
 */
HWTEST_F(TaskDispatcherTest, TaskDispatcher_CancelTask_00500, Function | MediumTest | Level1)
{
    StartAbility(firstAbilityName, bundleNameFirst);
    std::string data = "Dispatcher_" + std::to_string((int)TestFunc::CANCEL_TASK) + "_4";
    bool result = false;
    for (int i = 0; i < stLevel_.AMSLevel; i++) {
        STAbilityUtil::PublishEvent(g_EVENT_REQU_FIRST, ++testCaseCode, data);
        EXPECT_EQ(0, STAbilityUtil::WaitCompleted(event, g_EVENT_RESP_FIRST, testCaseCode, DELAY));
        data = STAbilityUtil::GetData(event, g_EVENT_RESP_FIRST, testCaseCode);
        result = data.compare("1") == 0;
        EXPECT_TRUE(result);
        if (!result && i > 0) {
            GTEST_LOG_(INFO) << "TaskDispatcher_CancelTask_00500 : " << i;
            break;
        }
    }
}

/**
 * @tc.number    : TaskDispatcher_CancelTask_00600
 * @tc.name      : cancel async_group task failed
 * @tc.desc      : cancel async_group task failed
 */
HWTEST_F(TaskDispatcherTest, TaskDispatcher_CancelTask_00600, Function | MediumTest | Level1)
{
    StartAbility(firstAbilityName, bundleNameFirst);
    std::string data = "Dispatcher_" + std::to_string((int)TestFunc::CANCEL_TASK) + "_5";
    bool result = false;
    for (int i = 0; i < stLevel_.AMSLevel; i++) {
        STAbilityUtil::PublishEvent(g_EVENT_REQU_FIRST, ++testCaseCode, data);
        EXPECT_EQ(0, STAbilityUtil::WaitCompleted(event, g_EVENT_RESP_FIRST, testCaseCode, DELAY));
        data = STAbilityUtil::GetData(event, g_EVENT_RESP_FIRST, testCaseCode);
        result = data.compare("1") == 0;
        EXPECT_TRUE(result);
        if (!result && i > 0) {
            GTEST_LOG_(INFO) << "TaskDispatcher_CancelTask_00600 : " << i;
            break;
        }
    }
}
}  // namespace