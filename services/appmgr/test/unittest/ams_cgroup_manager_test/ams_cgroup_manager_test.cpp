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
#include "cgroup_manager.h"
#undef private

#include <gtest/gtest.h>
#include <fcntl.h>
#include <sys/eventfd.h>
#include <unistd.h>
#include <sys/prctl.h>
#include <sys/capability.h>
#include <functional>
#include <sys/mount.h>

#include "nocopyable.h"
#include "app_log_wrapper.h"
#include "securec.h"

#define CG_CPUCTL_DIR "/dev/cpuctl"
#define CG_CPUCTL_DEFAULT_DIR CG_CPUCTL_DIR
#define CG_CPUCTL_DEFAULT_TASKS_PATH CG_CPUCTL_DEFAULT_DIR "/tasks"
#define CG_CPUCTL_BACKGROUND_DIR CG_CPUCTL_DIR "/background"
#define CG_CPUCTL_BACKGROUND_TASKS_PATH CG_CPUCTL_BACKGROUND_DIR "/tasks"

#define CG_CPUSET_DIR "/dev/cpuset"
#define CG_CPUSET_DEFAULT_DIR CG_CPUSET_DIR
#define CG_CPUSET_DEFAULT_TASKS_PATH CG_CPUSET_DEFAULT_DIR "/tasks"
#define CG_CPUSET_BACKGROUND_DIR CG_CPUSET_DIR "/background"
#define CG_CPUSET_BACKGROUND_TASKS_PATH CG_CPUSET_BACKGROUND_DIR "/tasks"

#define CG_FREEZER_DIR "/dev/freezer"
#define CG_FREEZER_FROZEN_DIR CG_FREEZER_DIR "/frozen"
#define CG_FREEZER_FROZEN_TASKS_PATH CG_FREEZER_FROZEN_DIR "/tasks"
#define CG_FREEZER_THAWED_DIR CG_FREEZER_DIR "/thawed"
#define CG_FREEZER_THAWED_TASKS_PATH CG_FREEZER_THAWED_DIR "/tasks"

#define CG_MEM_DIR "/dev/memcg"
#define CG_MEM_OOMCTL_PATH CG_MEM_DIR "/memory.oom_control"
#define CG_MEM_EVTCTL_PATH CG_MEM_DIR "/cgroup.event_control"
#define CG_MEM_PRESSURE_LEVEL_PATH CG_MEM_DIR "/memory.pressure_level"

constexpr int OPEN_FILE_MAX = 32768;

using namespace testing;
using namespace testing::ext;

namespace OHOS {
namespace AppExecFwk {

static int LOW_MEMORY_LEVEL = -1;

class AmsCgroupManagerTest : public testing::Test {
public:
    static void SetUpTestCase();
    static void TearDownTestCase();
    void SetUp();
    void TearDown();

    void OnLowMemoryAlert(const CgroupManager::LowMemoryLevel level);
    static void RenameCpusetDir();
    static void RecoveryCpusetDir();
    static void RenameCpuctlDir();
    static void RecoveryCpuctlDir();
    static void RenameFreezerFrozen();
    static void RecoveryFreezerFrozen();
};

void AmsCgroupManagerTest::OnLowMemoryAlert(const CgroupManager::LowMemoryLevel level)
{
    APP_LOGI("OnLowMemoryAlert level %{public}d", level);

    LOW_MEMORY_LEVEL = level;
}

void AmsCgroupManagerTest::RenameCpusetDir()
{
    // rename dir
    std::string command = "mv /dev/cpuset/background /dev/cpuset/background_tmp";
    system((char *)command.c_str());
}

void AmsCgroupManagerTest::RecoveryCpusetDir()
{
    // recovery dir
    std::string command = "mv /dev/cpuset/background_tmp /dev/cpuset/background";
    system((char *)command.c_str());
}

void AmsCgroupManagerTest::RenameCpuctlDir()
{
    // rename dir
    std::string command = "mv /dev/cpuctl/background /dev/cpuctl/background_tmp";
    system((char *)command.c_str());
}

void AmsCgroupManagerTest::RecoveryCpuctlDir()
{
    // recovery dir
    std::string command = "mv /dev/cpuctl/background_tmp /dev/cpuctl/background";
    system((char *)command.c_str());
}

void AmsCgroupManagerTest::RenameFreezerFrozen()
{
    // rename dir
    std::string command = "mv /dev/freezer/frozen /dev/freezer/frozen_tmp";
    system((char *)command.c_str());
}

void AmsCgroupManagerTest::RecoveryFreezerFrozen()
{
    // recovery dir
    std::string command = "mv /dev/freezer/frozen_tmp /dev/freezer/frozen";
    system((char *)command.c_str());
}

void AmsCgroupManagerTest::SetUpTestCase()
{}

void AmsCgroupManagerTest::TearDownTestCase()
{}

void AmsCgroupManagerTest::SetUp()
{
    if (DelayedSingleton<CgroupManager>::GetInstance()->LowMemoryAlert == nullptr) {
        DelayedSingleton<CgroupManager>::GetInstance()->LowMemoryAlert =
            std::bind(&AmsCgroupManagerTest::OnLowMemoryAlert, this, std::placeholders::_1);
    }
}
void AmsCgroupManagerTest::TearDown()
{}

/*
 * Feature: AMS
 * Function: CgroupManager
 * SubFunction: Init
 * FunctionPoints: initialization
 * EnvConditions: Mobile that can run ohos test framework.
 * CaseDescription: Verify the function Init can check the invalid eventHandler_
 */
HWTEST_F(AmsCgroupManagerTest, AmsCgroupManager_Init_001, TestSize.Level1)
{
    APP_LOGD("AmsCgroupManager_Init_001 start.");

    DelayedSingleton<CgroupManager>::GetInstance()->eventHandler_ =
        std::make_shared<EventHandler>(EventRunner::Create());
    EXPECT_TRUE(DelayedSingleton<CgroupManager>::GetInstance()->eventHandler_ != nullptr);

    EXPECT_EQ(false, DelayedSingleton<CgroupManager>::GetInstance()->Init());
    EXPECT_EQ(true, DelayedSingleton<CgroupManager>::GetInstance()->IsInited());

    // recover eventHandler_
    DelayedSingleton<CgroupManager>::GetInstance()->eventHandler_.reset();
    DelayedSingleton<CgroupManager>::GetInstance()->eventHandler_ = nullptr;
    EXPECT_EQ(false, DelayedSingleton<CgroupManager>::GetInstance()->IsInited());

    APP_LOGD("AmsCgroupManager_Init_001 end.");
}

/*
 * Feature: AMS
 * Function: CgroupManager
 * SubFunction: Init
 * FunctionPoints: initialization
 * EnvConditions: Mobile that can run ohos test framework.
 * CaseDescription: The verification function Init can check the failure of calling InitCpusetTasksFds
 */
HWTEST_F(AmsCgroupManagerTest, AmsCgroupManager_Init_002, TestSize.Level1)
{
    APP_LOGD("AmsCgroupManager_Init_002 start.");

    AmsCgroupManagerTest::RenameCpusetDir();

    EXPECT_EQ(false, DelayedSingleton<CgroupManager>::GetInstance()->Init());

    AmsCgroupManagerTest::RecoveryCpusetDir();

    APP_LOGD("AmsCgroupManager_Init_002 end.");
}

/*
 * Feature: AMS
 * Function: CgroupManager
 * SubFunction: Init
 * FunctionPoints: initialization
 * EnvConditions: Mobile that can run ohos test framework.
 * CaseDescription: The verification function Init can check the failure of calling InitCpuctlTasksFds
 */
HWTEST_F(AmsCgroupManagerTest, AmsCgroupManager_Init_003, TestSize.Level1)
{
    APP_LOGD("AmsCgroupManager_Init_003 start.");

    AmsCgroupManagerTest::RenameCpuctlDir();

    EXPECT_EQ(false, DelayedSingleton<CgroupManager>::GetInstance()->Init());

    AmsCgroupManagerTest::RecoveryCpuctlDir();

    APP_LOGD("AmsCgroupManager_Init_003 end.");
}
/*
 * Feature: AMS
 * Function: CgroupManager
 * SubFunction: Init
 * FunctionPoints: initialization
 * EnvConditions: Mobile that can run ohos test framework.
 * CaseDescription: The verification function Init can check the failure of calling InitFreezerTasksFds
 */
HWTEST_F(AmsCgroupManagerTest, AmsCgroupManager_Init_004, TestSize.Level1)
{
    APP_LOGD("AmsCgroupManager_Init_004 start.");

    AmsCgroupManagerTest::RenameFreezerFrozen();

    EXPECT_EQ(false, DelayedSingleton<CgroupManager>::GetInstance()->Init());

    AmsCgroupManagerTest::RecoveryFreezerFrozen();

    APP_LOGD("AmsCgroupManager_Init_004 end.");
}

/*
 * Feature: AMS
 * Function: CgroupManager
 * SubFunction: Init
 * FunctionPoints: initialization
 * EnvConditions: Mobile that can run ohos test framework.
 * CaseDescription: The verification function Init can check that the initialization is successful
 */
HWTEST_F(AmsCgroupManagerTest, AmsCgroupManager_Init_005, TestSize.Level1)
{
    APP_LOGD("AmsCgroupManager_Init_005 start.");

    EXPECT_EQ(true, DelayedSingleton<CgroupManager>::GetInstance()->Init());

    APP_LOGD("AmsCgroupManager_Init_005 end.");
}

/*
 * Feature: AMS
 * Function: CgroupManager
 * SubFunction: SetThreadSchedPolicy
 * FunctionPoints: set thread schedpolicy
 * EnvConditions: Mobile that can run ohos test framework.
 * CaseDescription: Verify the function SetThreadSchedPolicy can check the invalid eventHandler_
 */
HWTEST_F(AmsCgroupManagerTest, AmsCgroupManager_SetThreadSchedPolicy_001, TestSize.Level1)
{
    APP_LOGD("AmsCgroupManager_SetThreadSchedPolicy_001 start.");

    int tid = 1;
    DelayedSingleton<CgroupManager>::GetInstance()->eventHandler_ = nullptr;
    EXPECT_EQ(false,
        DelayedSingleton<CgroupManager>::GetInstance()->SetThreadSchedPolicy(
            tid, CgroupManager::SchedPolicy::SCHED_POLICY_BACKGROUND));

    APP_LOGD("AmsCgroupManager_SetThreadSchedPolicy_001 end.");
}

/*
 * Feature: AMS
 * Function: CgroupManager
 * SubFunction: SetThreadSchedPolicy
 * FunctionPoints: set thread schedpolicy
 * EnvConditions: Mobile that can run ohos test framework.
 * CaseDescription: Verify the function SetThreadSchedPolicy can check the invalid tid
 */
HWTEST_F(AmsCgroupManagerTest, AmsCgroupManager_SetThreadSchedPolicy_002, TestSize.Level1)
{
    APP_LOGD("AmsCgroupManager_SetThreadSchedPolicy_002 start.");

    int tid = 0;
    EXPECT_EQ(false,
        DelayedSingleton<CgroupManager>::GetInstance()->SetThreadSchedPolicy(
            tid, CgroupManager::SchedPolicy::SCHED_POLICY_BACKGROUND));

    APP_LOGD("AmsCgroupManager_SetThreadSchedPolicy_002 end.");
}

/*
 * Feature: AMS
 * Function: CgroupManager
 * SubFunction: SetThreadSchedPolicy
 * FunctionPoints: set thread schedpolicy
 * EnvConditions: Mobile that can run ohos test framework.
 * CaseDescription: Verify the function SetThreadSchedPolicy can check the invalid SchedPolicy
 */
HWTEST_F(AmsCgroupManagerTest, AmsCgroupManager_SetThreadSchedPolicy_003, TestSize.Level1)
{
    APP_LOGD("AmsCgroupManager_SetThreadSchedPolicy_003 start.");

    int tid = 2;
    int SchedPolicy = -1;
    EXPECT_EQ(false,
        DelayedSingleton<CgroupManager>::GetInstance()->SetThreadSchedPolicy(
            tid, CgroupManager::SchedPolicy::SCHED_POLICY_MAX));

    EXPECT_EQ(false,
        DelayedSingleton<CgroupManager>::GetInstance()->SetThreadSchedPolicy(
            tid, static_cast<CgroupManager::SchedPolicy>(SchedPolicy)));

    APP_LOGD("AmsCgroupManager_SetThreadSchedPolicy_003 end.");
}

/*
 * Feature: AMS
 * Function: CgroupManager
 * SubFunction: SetThreadSchedPolicy
 * FunctionPoints: set thread schedpolicy
 * EnvConditions: Mobile that can run ohos test framework.
 * CaseDescription: Verify the function SetThreadSchedPolicy can check the invalid freezerTasksFds_
 */
HWTEST_F(AmsCgroupManagerTest, AmsCgroupManager_SetThreadSchedPolicy_004, TestSize.Level1)
{
    APP_LOGD("AmsCgroupManager_SetThreadSchedPolicy_004 start.");

    int tid = 2;

    DelayedSingleton<CgroupManager>::GetInstance()
        ->freezerTasksFds_[CgroupManager::SchedPolicyFreezer::SCHED_POLICY_FREEZER_FROZEN] = -1;

    EXPECT_EQ(false,
        DelayedSingleton<CgroupManager>::GetInstance()->SetThreadSchedPolicy(
            tid, CgroupManager::SchedPolicy::SCHED_POLICY_FREEZED));

    APP_LOGD("AmsCgroupManager_SetThreadSchedPolicy_004 end.");
}

/*
 * Feature: AMS
 * Function: CgroupManager
 * SubFunction: SetThreadSchedPolicy
 * FunctionPoints: set thread schedpolicy
 * EnvConditions: Mobile that can run ohos test framework.
 * CaseDescription: Verify the function SetThreadSchedPolicy can check the invalid cpusetTasksFds_
 */
HWTEST_F(AmsCgroupManagerTest, AmsCgroupManager_SetThreadSchedPolicy_005, TestSize.Level1)
{
    APP_LOGD("AmsCgroupManager_SetThreadSchedPolicy_005 start.");

    int tid = 2;

    DelayedSingleton<CgroupManager>::GetInstance()
        ->cpusetTasksFds_[CgroupManager::SchedPolicy::SCHED_POLICY_BACKGROUND] = -1;

    EXPECT_EQ(false,
        DelayedSingleton<CgroupManager>::GetInstance()->SetThreadSchedPolicy(
            tid, CgroupManager::SchedPolicy::SCHED_POLICY_BACKGROUND));

    APP_LOGD("AmsCgroupManager_SetThreadSchedPolicy_005 end.");
}

/*
 * Feature: AMS
 * Function: CgroupManager
 * SubFunction: SetThreadSchedPolicy
 * FunctionPoints: set thread schedpolicy
 * EnvConditions: Mobile that can run ohos test framework.
 * CaseDescription: Verify the function SetThreadSchedPolicy can check the invalid cpuctlTasksFds_
 */
HWTEST_F(AmsCgroupManagerTest, AmsCgroupManager_SetThreadSchedPolicy_006, TestSize.Level1)
{
    APP_LOGD("AmsCgroupManager_SetThreadSchedPolicy_006 start.");

    int tid = 2;

    DelayedSingleton<CgroupManager>::GetInstance()->cpuctlTasksFds_[CgroupManager::SchedPolicy::SCHED_POLICY_DEFAULT] =
        -1;

    EXPECT_EQ(false,
        DelayedSingleton<CgroupManager>::GetInstance()->SetThreadSchedPolicy(
            tid, CgroupManager::SchedPolicy::SCHED_POLICY_DEFAULT));

    APP_LOGD("AmsCgroupManager_SetThreadSchedPolicy_006 end.");
}

/*
 * Feature: AMS
 * Function: CgroupManager
 * SubFunction: SetThreadSchedPolicy
 * FunctionPoints: set thread schedpolicy
 * EnvConditions: Mobile that can run ohos test framework.
 * CaseDescription: Verify the function SetThreadSchedPolicy can check the invalid freezerTasksFds_
 */
HWTEST_F(AmsCgroupManagerTest, AmsCgroupManager_SetThreadSchedPolicy_007, TestSize.Level1)
{
    APP_LOGD("AmsCgroupManager_SetThreadSchedPolicy_007 start.");

    int tid = 2;

    DelayedSingleton<CgroupManager>::GetInstance()
        ->freezerTasksFds_[CgroupManager::SchedPolicyFreezer::SCHED_POLICY_FREEZER_THAWED] = -1;

    EXPECT_EQ(false,
        DelayedSingleton<CgroupManager>::GetInstance()->SetThreadSchedPolicy(
            tid, CgroupManager::SchedPolicy::SCHED_POLICY_BACKGROUND));

    APP_LOGD("AmsCgroupManager_SetThreadSchedPolicy_007 end.");
}

/*
 * Feature: AMS
 * Function: CgroupManager
 * SubFunction: SetThreadSchedPolicy
 * FunctionPoints: set thread schedpolicy
 * EnvConditions: Mobile that can run ohos test framework.
 * CaseDescription: Verify that the function SetThreadSchedPolicy can be executed successfully
 */
HWTEST_F(AmsCgroupManagerTest, AmsCgroupManager_SetThreadSchedPolicy_008, TestSize.Level1)
{
    APP_LOGD("AmsCgroupManager_SetThreadSchedPolicy_008 start.");

    int tid = 1;

    DelayedSingleton<CgroupManager>::GetInstance()->eventHandler_ =
        std::make_shared<EventHandler>(EventRunner::Create());
    EXPECT_TRUE(DelayedSingleton<CgroupManager>::GetInstance()->eventHandler_ != nullptr);

    DelayedSingleton<CgroupManager>::GetInstance()
        ->cpusetTasksFds_[CgroupManager::SchedPolicyCpu::SCHED_POLICY_CPU_BACKGROUND] =
        open(CG_CPUSET_BACKGROUND_TASKS_PATH, O_RDWR);

    DelayedSingleton<CgroupManager>::GetInstance()
        ->cpuctlTasksFds_[CgroupManager::SchedPolicyCpu::SCHED_POLICY_CPU_BACKGROUND] =
        open(CG_CPUCTL_BACKGROUND_TASKS_PATH, O_RDWR);

    DelayedSingleton<CgroupManager>::GetInstance()
        ->freezerTasksFds_[CgroupManager::SchedPolicyFreezer::SCHED_POLICY_FREEZER_THAWED] =
        open(CG_FREEZER_THAWED_TASKS_PATH, O_RDWR);

    EXPECT_EQ(true,
        DelayedSingleton<CgroupManager>::GetInstance()->SetThreadSchedPolicy(
            tid, CgroupManager::SchedPolicy::SCHED_POLICY_BACKGROUND));

    APP_LOGD("AmsCgroupManager_SetThreadSchedPolicy_008 end.");
}

/*
 * Feature: AMS
 * Function: CgroupManager
 * SubFunction: SetProcessSchedPolicy
 * FunctionPoints: set process schedpolicy
 * EnvConditions: Mobile that can run ohos test framework.
 * CaseDescription: Verify the function SetProcessSchedPolicy can check the invalid eventHandler_
 */
HWTEST_F(AmsCgroupManagerTest, AmsCgroupManager_SetProcessSchedPolicy_001, TestSize.Level1)
{
    APP_LOGD("AmsCgroupManager_SetProcessSchedPolicy_001 start.");

    int pid = 2;
    DelayedSingleton<CgroupManager>::GetInstance()->eventHandler_ = nullptr;
    EXPECT_EQ(false,
        DelayedSingleton<CgroupManager>::GetInstance()->SetProcessSchedPolicy(
            pid, CgroupManager::SchedPolicy::SCHED_POLICY_DEFAULT));

    APP_LOGD("AmsCgroupManager_SetProcessSchedPolicy_001 end.");
}

/*
 * Feature: AMS
 * Function: CgroupManager
 * SubFunction: SetProcessSchedPolicy
 * FunctionPoints: set process schedpolicy
 * EnvConditions: Mobile that can run ohos test framework.
 * CaseDescription: Verify the function SetProcessSchedPolicy can check the invalid pid
 */
HWTEST_F(AmsCgroupManagerTest, AmsCgroupManager_SetProcessSchedPolicy_002, TestSize.Level1)
{
    APP_LOGD("AmsCgroupManager_SetProcessSchedPolicy_002 start.");

    int pid = 0;
    EXPECT_EQ(false,
        DelayedSingleton<CgroupManager>::GetInstance()->SetProcessSchedPolicy(
            pid, CgroupManager::SchedPolicy::SCHED_POLICY_BACKGROUND));

    APP_LOGD("AmsCgroupManager_SetProcessSchedPolicy_002 end.");
}

/*
 * Feature: AMS
 * Function: CgroupManager
 * SubFunction: SetProcessSchedPolicy
 * FunctionPoints: set process schedpolicy
 * EnvConditions: Mobile that can run ohos test framework.
 * CaseDescription: Verify the function SetProcessSchedPolicy can check the invalid SchedPolicy
 */
HWTEST_F(AmsCgroupManagerTest, AmsCgroupManager_SetProcessSchedPolicy_003, TestSize.Level1)
{
    APP_LOGD("AmsCgroupManager_SetProcessSchedPolicy_003 start.");

    int tid = 2;
    int SchedPolicy = -1;
    EXPECT_EQ(false,
        DelayedSingleton<CgroupManager>::GetInstance()->SetProcessSchedPolicy(
            tid, CgroupManager::SchedPolicy::SCHED_POLICY_MAX));

    EXPECT_EQ(false,
        DelayedSingleton<CgroupManager>::GetInstance()->SetProcessSchedPolicy(
            tid, static_cast<CgroupManager::SchedPolicy>(SchedPolicy)));

    APP_LOGD("AmsCgroupManager_SetProcessSchedPolicy_003 end.");
}

/*
 * Feature: AMS
 * Function: CgroupManager
 * SubFunction: SetProcessSchedPolicy
 * FunctionPoints: set process schedpolicy
 * EnvConditions: Mobile that can run ohos test framework.
 * CaseDescription: Verify that the function SetProcessSchedPolicy can be executed successfully
 */
HWTEST_F(AmsCgroupManagerTest, AmsCgroupManager_SetProcessSchedPolicy_004, TestSize.Level1)
{
    APP_LOGD("AmsCgroupManager_SetProcessSchedPolicy_004 start.");

    int tid = 2;
    DelayedSingleton<CgroupManager>::GetInstance()->eventHandler_ =
        std::make_shared<EventHandler>(EventRunner::Create());
    EXPECT_TRUE(DelayedSingleton<CgroupManager>::GetInstance()->eventHandler_ != nullptr);

    EXPECT_EQ(true,
        DelayedSingleton<CgroupManager>::GetInstance()->SetProcessSchedPolicy(
            tid, CgroupManager::SchedPolicy::SCHED_POLICY_DEFAULT));

    APP_LOGD("AmsCgroupManager_SetProcessSchedPolicy_004 end.");
}

/*
 * Feature: AMS
 * Function: CgroupManager
 * SubFunction: OnReadable
 * FunctionPoints: system low memory alert
 * EnvConditions: Mobile that can run ohos test framework.
 * CaseDescription: Verification function OnLowMemoryAlert in OnReadable returned level
 */
HWTEST_F(AmsCgroupManagerTest, AmsCgroupManager_OnReadable_001, TestSize.Level1)
{
    APP_LOGD("AmsCgroupManager_OnReadable_001 start.");

    uint64_t cnt = 100;
    int32_t fd[2] = {0, 0};

    if (pipe(fd) == -1) {
        GTEST_LOG_(WARNING) << "create pipe fail";
        return;
    }
    write(fd[1], &cnt, sizeof(uint64_t));

    DelayedSingleton<CgroupManager>::GetInstance()
        ->memoryEventFds_[CgroupManager::LowMemoryLevel::LOW_MEMORY_LEVEL_LOW] = fd[0];
    DelayedSingleton<CgroupManager>::GetInstance()->OnReadable(fd[0]);

    EXPECT_EQ(CgroupManager::LowMemoryLevel::LOW_MEMORY_LEVEL_LOW, LOW_MEMORY_LEVEL);

    APP_LOGD("AmsCgroupManager_OnReadable_001 end.");
}

/*
 * Feature: AMS
 * Function: CgroupManager
 * SubFunction: OnReadable
 * FunctionPoints: system low memory alert
 * EnvConditions: Mobile that can run ohos test framework.
 * CaseDescription: Verification function OnLowMemoryAlert in OnReadable returned level
 */
HWTEST_F(AmsCgroupManagerTest, AmsCgroupManager_OnReadable_002, TestSize.Level1)
{
    APP_LOGD("AmsCgroupManager_OnReadable_002 start ");

    uint64_t cnt = 100;
    int32_t fd[2] = {0, 0};

    if (pipe(fd) == -1) {
        GTEST_LOG_(WARNING) << "create pipe fail";
        return;
    }
    write(fd[1], &cnt, sizeof(uint64_t));
    DelayedSingleton<CgroupManager>::GetInstance()
        ->memoryEventFds_[CgroupManager::LowMemoryLevel::LOW_MEMORY_LEVEL_MEDIUM] = fd[0];

    DelayedSingleton<CgroupManager>::GetInstance()->OnReadable(fd[0]);

    EXPECT_EQ(CgroupManager::LowMemoryLevel::LOW_MEMORY_LEVEL_MEDIUM, LOW_MEMORY_LEVEL);

    APP_LOGD("AmsCgroupManager_OnReadable_002 end.");
}

/*
 * Feature: AMS
 * Function: CgroupManager
 * SubFunction: OnReadable
 * FunctionPoints: system low memory alert
 * EnvConditions: Mobile that can run ohos test framework.
 * CaseDescription: Verification function OnLowMemoryAlert in OnReadable returned level
 */
HWTEST_F(AmsCgroupManagerTest, AmsCgroupManager_OnReadable_003, TestSize.Level1)
{
    APP_LOGD("AmsCgroupManager_OnReadable_003 start.");

    uint64_t cnt = 100;
    int32_t fd[2] = {0, 0};

    if (pipe(fd) == -1) {
        GTEST_LOG_(WARNING) << "create pipe fail";
        return;
    }
    write(fd[1], &cnt, sizeof(uint64_t));
    DelayedSingleton<CgroupManager>::GetInstance()
        ->memoryEventFds_[CgroupManager::LowMemoryLevel::LOW_MEMORY_LEVEL_CRITICAL] = fd[0];

    DelayedSingleton<CgroupManager>::GetInstance()->OnReadable(fd[0]);

    EXPECT_EQ(CgroupManager::LowMemoryLevel::LOW_MEMORY_LEVEL_CRITICAL, LOW_MEMORY_LEVEL);

    APP_LOGD("AmsCgroupManager_OnReadable_003 end.");
}

/*
 * Feature: AMS
 * Function: CgroupManager
 * SubFunction: RegisterLowMemoryMonitor
 * FunctionPoints: registering low memory monitor
 * EnvConditions: Mobile that can run ohos test framework.
 * CaseDescription: Verify the function RegisterLowMemoryMonitor can check the invalid memoryEventControlFd
 */
HWTEST_F(AmsCgroupManagerTest, AmsCgroupManager_RegisterLowMemoryMonitor_001, TestSize.Level1)
{
    APP_LOGD("AmsCgroupManager_RegisterLowMemoryMonitor_001 start.");

    int memoryEventFds[CgroupManager::LowMemoryLevel::LOW_MEMORY_LEVEL_MAX] = {0};
    int memoryPressureFds[CgroupManager::LowMemoryLevel::LOW_MEMORY_LEVEL_MAX] = {0};
    int memoryEventControlFd = -1;
    auto eventHandler = std::make_shared<EventHandler>(EventRunner::Create());
    EXPECT_EQ(false,
        DelayedSingleton<CgroupManager>::GetInstance()->RegisterLowMemoryMonitor(memoryEventFds,
            memoryPressureFds,
            memoryEventControlFd,
            CgroupManager::LowMemoryLevel::LOW_MEMORY_LEVEL_LOW,
            eventHandler));

    APP_LOGD("AmsCgroupManager_RegisterLowMemoryMonitor_001 end.");
}

/*
 * Feature: AMS
 * Function: CgroupManager
 * SubFunction: RegisterLowMemoryMonitor
 * FunctionPoints: registering low memory monitor
 * EnvConditions: Mobile that can run ohos test framework.
 * CaseDescription:  Verify that the function RegisterLowMemoryMonitor can be executed successfully
 */
HWTEST_F(AmsCgroupManagerTest, AmsCgroupManager_RegisterLowMemoryMonitor_002, TestSize.Level1)
{
    APP_LOGD("AmsCgroupManager_RegisterLowMemoryMonitor_002 start.");

    int memoryEventFds[CgroupManager::LowMemoryLevel::LOW_MEMORY_LEVEL_MAX] = {1};
    int memoryPressureFds[CgroupManager::LowMemoryLevel::LOW_MEMORY_LEVEL_MAX] = {1};
    int memoryEventControlFd = 1;
    auto eventHandler = std::make_shared<EventHandler>(EventRunner::Create());
    EXPECT_EQ(true,
        DelayedSingleton<CgroupManager>::GetInstance()->RegisterLowMemoryMonitor(memoryEventFds,
            memoryPressureFds,
            memoryEventControlFd,
            CgroupManager::LowMemoryLevel::LOW_MEMORY_LEVEL_LOW,
            eventHandler));

    APP_LOGD("AmsCgroupManager_RegisterLowMemoryMonitor_002 end.");
}

/*
 * Feature: AMS
 * Function: CgroupManager
 * SubFunction: InitCpusetTasksFds
 * FunctionPoints: initialization cpusetTasksFds
 * EnvConditions: Mobile that can run ohos test framework.
 * CaseDescription: The verification function InitCpusetTasksFds initialization memoryPressureFds failed
 */
HWTEST_F(AmsCgroupManagerTest, AmsCgroupManager_InitCpusetTasksFds_001, TestSize.Level1)
{
    APP_LOGD("AmsCgroupManager_InitCpusetTasksFds_001 start.");

    AmsCgroupManagerTest::RenameCpusetDir();

    UniqueFd cpusetTasksFds[CgroupManager::SchedPolicy::SCHED_POLICY_MAX];
    EXPECT_EQ(false, DelayedSingleton<CgroupManager>::GetInstance()->InitCpusetTasksFds(cpusetTasksFds));

    AmsCgroupManagerTest::RecoveryCpusetDir();

    APP_LOGD("AmsCgroupManager_InitCpusetTasksFds_001 end.");
}

/*
 * Feature: AMS
 * Function: CgroupManager
 * SubFunction: InitCpusetTasksFds
 * FunctionPoints: initialization cpusetTasksFds
 * EnvConditions: Mobile that can run ohos test framework.
 * CaseDescription: The verification function InitCpusetTasksFds initialization memoryPressureFds successful
 */
HWTEST_F(AmsCgroupManagerTest, AmsCgroupManager_InitCpusetTasksFds_002, TestSize.Level1)
{
    APP_LOGD("AmsCgroupManager_InitCpusetTasksFds_002 start.");

    UniqueFd cpusetTasksFds[CgroupManager::SchedPolicy::SCHED_POLICY_MAX];
    EXPECT_EQ(true, DelayedSingleton<CgroupManager>::GetInstance()->InitCpusetTasksFds(cpusetTasksFds));

    APP_LOGD("AmsCgroupManager_InitCpusetTasksFds_002 end.");
}

/*
 * Feature: AMS
 * Function: CgroupManager
 * SubFunction: InitCpuctlTasksFds
 * FunctionPoints: initialization cpuctlTasksFds
 * EnvConditions: Mobile that can run ohos test framework.
 * CaseDescription: The verification function InitCpuctlTasksFds initialization memoryPressureFds failed
 */
HWTEST_F(AmsCgroupManagerTest, AmsCgroupManager_InitCpuctlTasksFds_001, TestSize.Level1)
{
    APP_LOGD("AmsCgroupManager_InitCpuctlTasksFds_001 start.");

    AmsCgroupManagerTest::RenameCpuctlDir();

    UniqueFd cpuctlTasksFds[CgroupManager::SchedPolicy::SCHED_POLICY_MAX];
    EXPECT_EQ(false, DelayedSingleton<CgroupManager>::GetInstance()->InitCpuctlTasksFds(cpuctlTasksFds));

    AmsCgroupManagerTest::RecoveryCpuctlDir();

    APP_LOGD("AmsCgroupManager_InitCpuctlTasksFds_001 end.");
}

/*
 * Feature: AMS
 * Function: CgroupManager
 * SubFunction: InitCpuctlTasksFds
 * FunctionPoints: initialization cpuctlTasksFds
 * EnvConditions: Mobile that can run ohos test framework.
 * CaseDescription: The verification function InitCpuctlTasksFds initialization memoryPressureFds successful
 */
HWTEST_F(AmsCgroupManagerTest, AmsCgroupManager_InitCpuctlTasksFds_002, TestSize.Level1)
{
    APP_LOGD("AmsCgroupManager_InitCpuctlTasksFds_002 start.");

    UniqueFd cpuctlTasksFds[CgroupManager::SchedPolicy::SCHED_POLICY_MAX];
    EXPECT_EQ(true, DelayedSingleton<CgroupManager>::GetInstance()->InitCpuctlTasksFds(cpuctlTasksFds));

    APP_LOGD("AmsCgroupManager_InitCpuctlTasksFds_002 end.");
}

/*
 * Feature: AMS
 * Function: CgroupManager
 * SubFunction: InitFreezerTasksFds
 * FunctionPoints: initialization freezerTasksFds
 * EnvConditions: Mobile that can run ohos test framework.
 * CaseDescription: The verification function InitFreezerTasksFds initialization memoryPressureFds failed
 */
HWTEST_F(AmsCgroupManagerTest, AmsCgroupManager_InitFreezerTasksFds_001, TestSize.Level1)
{
    APP_LOGD("AmsCgroupManager_InitFreezerTasksFds_001 start.");

    AmsCgroupManagerTest::RenameFreezerFrozen();

    UniqueFd freezerTasksFds[CgroupManager::SchedPolicyFreezer::SCHED_POLICY_FREEZER_MAX];
    EXPECT_EQ(false, DelayedSingleton<CgroupManager>::GetInstance()->InitFreezerTasksFds(freezerTasksFds));

    AmsCgroupManagerTest::RecoveryFreezerFrozen();

    APP_LOGD("AmsCgroupManager_InitFreezerTasksFds_001 end.");
}

/*
 * Feature: AMS
 * Function: CgroupManager
 * SubFunction: InitFreezerTasksFds
 * FunctionPoints: initialization freezerTasksFds
 * EnvConditions: Mobile that can run ohos test framework.
 * CaseDescription: The verification function InitFreezerTasksFds initialization memoryPressureFds successful
 */
HWTEST_F(AmsCgroupManagerTest, AmsCgroupManager_InitFreezerTasksFds_002, TestSize.Level1)
{
    APP_LOGD("AmsCgroupManager_InitFreezerTasksFds_002 start.");

    UniqueFd freezerTasksFds[CgroupManager::SchedPolicyFreezer::SCHED_POLICY_FREEZER_MAX];
    EXPECT_EQ(true, DelayedSingleton<CgroupManager>::GetInstance()->InitFreezerTasksFds(freezerTasksFds));

    APP_LOGD("AmsCgroupManager_InitFreezerTasksFds_002 end.");
}

/*
 * Feature: AMS
 * Function: CgroupManager
 * SubFunction: InitMemoryEventControlFd
 * FunctionPoints: initialization memoryEventControlFd
 * EnvConditions: Mobile that can run ohos test framework.
 * CaseDescription: The verification function InitMemoryEventControlFd initialization memoryPressureFds failed
 */
HWTEST_F(AmsCgroupManagerTest, AmsCgroupManager_InitMemoryEventControlFd_001, TestSize.Level1)
{
    APP_LOGD("AmsCgroupManager_InitMemoryEventControlFd_001 start.");

    int fds[OPEN_FILE_MAX] = {0};
    for (auto &fd : fds) {
        fd = open(CG_MEM_EVTCTL_PATH, O_WRONLY);
    }

    UniqueFd memoryEventControlFd;
    EXPECT_EQ(false, DelayedSingleton<CgroupManager>::GetInstance()->InitMemoryEventControlFd(memoryEventControlFd));

    for (auto fd : fds) {
        close(fd);
    }

    APP_LOGD("AmsCgroupManager_InitMemoryEventControlFd_001 end.");
}

/*
 * Feature: AMS
 * Function: CgroupManager
 * SubFunction: InitMemoryEventControlFd
 * FunctionPoints: initialization memoryEventControlFd
 * EnvConditions: Mobile that can run ohos test framework.
 * CaseDescription: The verification function InitMemoryEventControlFd initialization memoryPressureFds successful
 */
HWTEST_F(AmsCgroupManagerTest, AmsCgroupManager_InitMemoryEventControlFd_002, TestSize.Level1)
{
    APP_LOGD("AmsCgroupManager_InitMemoryEventControlFd_002 start.");

    UniqueFd memoryEventControlFd;
    EXPECT_EQ(true, DelayedSingleton<CgroupManager>::GetInstance()->InitMemoryEventControlFd(memoryEventControlFd));

    APP_LOGD("AmsCgroupManager_InitMemoryEventControlFd_002 end.");
}

/*
 * Feature: AMS
 * Function: CgroupManager
 * SubFunction: InitMemoryEventFds
 * FunctionPoints: initialization memoryEventFds
 * EnvConditions: Mobile that can run ohos test framework.
 * CaseDescription: The verification function InitMemoryEventFds initialization memoryPressureFds failed
 */
HWTEST_F(AmsCgroupManagerTest, AmsCgroupManager_InitMemoryEventFds_001, TestSize.Level1)
{
    APP_LOGD("AmsCgroupManager_InitMemoryEventFds_001 start.");

    int fds[OPEN_FILE_MAX] = {0};
    for (auto &fd : fds) {
        fd = eventfd(0, EFD_NONBLOCK);
    }

    UniqueFd memoryEventFds[CgroupManager::LowMemoryLevel::LOW_MEMORY_LEVEL_MAX];
    EXPECT_EQ(false, DelayedSingleton<CgroupManager>::GetInstance()->InitMemoryEventFds(memoryEventFds));

    for (auto fd : fds) {
        close(fd);
    }

    APP_LOGD("AmsCgroupManager_InitMemoryEventFds_001 end.");
}

/*
 * Feature: AMS
 * Function: CgroupManager
 * SubFunction: InitMemoryEventFds
 * FunctionPoints: initialization memoryEventFds
 * EnvConditions: Mobile that can run ohos test framework.
 * CaseDescription: The verification function InitMemoryEventFds initialization memoryPressureFds successful
 */
HWTEST_F(AmsCgroupManagerTest, AmsCgroupManager_InitMemoryEventFds_002, TestSize.Level1)
{
    APP_LOGD("AmsCgroupManager_InitMemoryEventFds_002 start.");

    UniqueFd memoryEventFds[CgroupManager::LowMemoryLevel::LOW_MEMORY_LEVEL_MAX];
    EXPECT_EQ(true, DelayedSingleton<CgroupManager>::GetInstance()->InitMemoryEventFds(memoryEventFds));

    APP_LOGD("AmsCgroupManager_InitMemoryEventFds_002 end.");
}

/*
 * Feature: AMS
 * Function: CgroupManager
 * SubFunction: InitMemoryPressureFds
 * FunctionPoints: initialization memoryPressureFds
 * EnvConditions: Mobile that can run ohos test framework.
 * CaseDescription: The verification function InitMemoryPressureFds initialization memoryPressureFds failed
 */
HWTEST_F(AmsCgroupManagerTest, AmsCgroupManager_InitMemoryPressureFds_001, TestSize.Level1)
{
    APP_LOGD("AmsCgroupManager_InitMemoryPressureFds_001 start.");

    int fds[OPEN_FILE_MAX] = {0};
    for (auto &fd : fds) {
        fd = open(CG_MEM_PRESSURE_LEVEL_PATH, O_RDONLY);
    }

    UniqueFd memoryPressureFds[CgroupManager::LowMemoryLevel::LOW_MEMORY_LEVEL_MAX];
    EXPECT_EQ(false, DelayedSingleton<CgroupManager>::GetInstance()->InitMemoryPressureFds(memoryPressureFds));

    for (auto fd : fds) {
        close(fd);
    }

    APP_LOGD("AmsCgroupManager_InitMemoryPressureFds_001 end.");
}

/*
 * Feature: AMS
 * Function: CgroupManager
 * SubFunction: SetCpusetSubsystem
 * FunctionPoints: set cpuset
 * EnvConditions: Mobile that can run ohos test framework.
 * CaseDescription: Verify the function SetCpusetSubsystem can check the invalid cpusetTasksFds_
 */
HWTEST_F(AmsCgroupManagerTest, AmsCgroupManager_SetCpusetSubsystem_001, TestSize.Level1)
{
    APP_LOGD("AmsCgroupManager_SetCpusetSubsystem_001 start.");

    int tid = 1;

    DelayedSingleton<CgroupManager>::GetInstance()->cpusetTasksFds_[CgroupManager::SchedPolicy::SCHED_POLICY_DEFAULT] =
        -1;

    EXPECT_EQ(false,
        DelayedSingleton<CgroupManager>::GetInstance()->SetCpusetSubsystem(
            tid, CgroupManager::SchedPolicy::SCHED_POLICY_DEFAULT));

    APP_LOGD("AmsCgroupManager_SetCpusetSubsystem_001 end.");
}

/*
 * Feature: AMS
 * Function: CgroupManager
 * SubFunction: SetCpusetSubsystem
 * FunctionPoints: set cpuset
 * EnvConditions: Mobile that can run ohos test framework.
 * CaseDescription: Verify that the function SetCpusetSubsystem can be called successfully
 */
HWTEST_F(AmsCgroupManagerTest, AmsCgroupManager_SetCpusetSubsystem_002, TestSize.Level1)
{
    APP_LOGD("AmsCgroupManager_SetCpusetSubsystem_002 start.");

    int tid = 1;

    DelayedSingleton<CgroupManager>::GetInstance()->cpusetTasksFds_[CgroupManager::SchedPolicy::SCHED_POLICY_DEFAULT] =
        open(CG_CPUCTL_DEFAULT_TASKS_PATH, O_RDWR);

    EXPECT_EQ(true,
        DelayedSingleton<CgroupManager>::GetInstance()->SetCpusetSubsystem(
            tid, CgroupManager::SchedPolicy::SCHED_POLICY_DEFAULT));

    APP_LOGD("AmsCgroupManager_SetCpusetSubsystem_002 end.");
}

/*
 * Feature: AMS
 * Function: CgroupManager
 * SubFunction: SetCpuctlSubsystem
 * FunctionPoints: set cpuctl
 * EnvConditions: Mobile that can run ohos test framework.
 * CaseDescription: Verify the function SetCpuctlSubsystem can check the invalid cpuctlTasksFds_
 */
HWTEST_F(AmsCgroupManagerTest, AmsCgroupManager_SetCpuctlSubsystem_001, TestSize.Level1)
{
    APP_LOGD("AmsCgroupManager_SetCpuctlSubsystem_001 start.");

    int tid = 1;

    DelayedSingleton<CgroupManager>::GetInstance()
        ->cpuctlTasksFds_[CgroupManager::SchedPolicyFreezer::SCHED_POLICY_FREEZER_FROZEN] = -1;

    EXPECT_EQ(false,
        DelayedSingleton<CgroupManager>::GetInstance()->SetCpuctlSubsystem(
            tid, CgroupManager::SchedPolicy::SCHED_POLICY_DEFAULT));

    APP_LOGD("AmsCgroupManager_SetCpuctlSubsystem_001 end.");
}

/*
 * Feature: AMS
 * Function: CgroupManager
 * SubFunction: SetCpuctlSubsystem
 * FunctionPoints: set cpuctl
 * EnvConditions: Mobile that can run ohos test framework.
 * CaseDescription: Verify that the function SetCpuctlSubsystem can be called successfully
 */
HWTEST_F(AmsCgroupManagerTest, AmsCgroupManager_SetCpuctlSubsystem_002, TestSize.Level1)
{
    APP_LOGD("AmsCgroupManager_SetCpuctlSubsystem_002 start.");

    int tid = 1;

    DelayedSingleton<CgroupManager>::GetInstance()
        ->cpuctlTasksFds_[CgroupManager::SchedPolicyFreezer::SCHED_POLICY_FREEZER_FROZEN] =
        open(CG_FREEZER_FROZEN_TASKS_PATH, O_RDWR);

    EXPECT_EQ(true,
        DelayedSingleton<CgroupManager>::GetInstance()->SetCpuctlSubsystem(
            tid, CgroupManager::SchedPolicy::SCHED_POLICY_DEFAULT));

    APP_LOGD("AmsCgroupManager_SetCpuctlSubsystem_002 end.");
}

/*
 * Feature: AMS
 * Function: CgroupManager
 * SubFunction: SetFreezerSubsystem
 * FunctionPoints: set frozen of freezer
 * EnvConditions: Mobile that can run ohos test framework.
 * CaseDescription: Verify the function SetFreezerSubsystem can check the invalid freezerTasksFds_
 */
HWTEST_F(AmsCgroupManagerTest, AmsCgroupManager_SetFreezerSubsystem_001, TestSize.Level1)
{
    APP_LOGD("AmsCgroupManager_SetFreezerSubsystem_001 start.");

    int tid = 1;

    DelayedSingleton<CgroupManager>::GetInstance()
        ->freezerTasksFds_[CgroupManager::SchedPolicyFreezer::SCHED_POLICY_FREEZER_FROZEN] = -1;

    EXPECT_EQ(false,
        DelayedSingleton<CgroupManager>::GetInstance()->SetFreezerSubsystem(
            tid, CgroupManager::SchedPolicyFreezer::SCHED_POLICY_FREEZER_FROZEN));

    APP_LOGD("AmsCgroupManager_SetFreezerSubsystem_001 end.");
}

/*
 * Feature: AMS
 * Function: CgroupManager
 * SubFunction: SetFreezerSubsystem
 * FunctionPoints: set frozen of freezer
 * EnvConditions: Mobile that can run ohos test framework.
 * CaseDescription: Verify that the function SetFreezerSubsystem can be called successfully
 */
HWTEST_F(AmsCgroupManagerTest, AmsCgroupManager_SetFreezerSubsystem_002, TestSize.Level1)
{
    APP_LOGD("AmsCgroupManager_SetFreezerSubsystem_002 start.");

    int tid = 1;

    DelayedSingleton<CgroupManager>::GetInstance()
        ->freezerTasksFds_[CgroupManager::SchedPolicyFreezer::SCHED_POLICY_FREEZER_FROZEN] =
        open(CG_FREEZER_FROZEN_TASKS_PATH, O_RDWR);

    EXPECT_EQ(true,
        DelayedSingleton<CgroupManager>::GetInstance()->SetFreezerSubsystem(
            tid, CgroupManager::SchedPolicyFreezer::SCHED_POLICY_FREEZER_FROZEN));

    APP_LOGD("AmsCgroupManager_SetFreezerSubsystem_002 end.");
}

}  // namespace AppExecFwk
}  // namespace OHOS