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
#include "cgroup_manager.h"
#include <dirent.h>
#include <errno.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/eventfd.h>
#include <sys/types.h>
#include <unistd.h>
#include "app_log_wrapper.h"
#include "event_handler.h"
#include "securec.h"

namespace OHOS {
namespace AppExecFwk {
namespace {
class ScopeGuard final {
public:
    using Function = std::function<void()>;

public:
    explicit ScopeGuard(Function fn) : fn_(fn), dismissed_(false)
    {}

    ~ScopeGuard()
    {
        if (!dismissed_) {
            fn_();
        }
    }

public:
    void Dismiss()
    {
        dismissed_ = true;
    }

private:
    Function fn_;
    bool dismissed_ = false;
};
}  // namespace

CgroupManager::CgroupManager() : memoryEventControlFd_(-1)
{}

CgroupManager::~CgroupManager()
{}

bool CgroupManager::Init()
{
    APP_LOGD("Init start.");
    return true;
}

bool CgroupManager::IsInited() const
{
    APP_LOGD("IsInited start.");
    return true;
}

bool CgroupManager::SetThreadSchedPolicy(int tid, SchedPolicy schedPolicy)
{
    APP_LOGD("SetThreadSchedPolicy start.");
    return true;
}

bool CgroupManager::SetProcessSchedPolicy(int pid, SchedPolicy schedPolicy)
{
    APP_LOGD("SetProcessSchedPolicy start.");
    return true;
}

void CgroupManager::OnReadable(int32_t fd)
{
    APP_LOGD("OnReadable start.");
    return;
}

bool CgroupManager::RegisterLowMemoryMonitor(const int memoryEventFds[LOW_MEMORY_LEVEL_MAX],
    const int memoryPressureFds[LOW_MEMORY_LEVEL_MAX], const int memoryEventControlFd, const LowMemoryLevel level,
    const std::shared_ptr<EventHandler> &eventHandler)
{
    APP_LOGD("RegisterLowMemoryMonitor start.");
    return true;
}

bool CgroupManager::InitCpusetTasksFds(UniqueFd cpusetTasksFds[SCHED_POLICY_CPU_MAX])
{
    APP_LOGD("InitCpusetTasksFds start.");
    return true;
}

bool CgroupManager::InitCpuctlTasksFds(UniqueFd cpuctlTasksFds[SCHED_POLICY_CPU_MAX])
{
    APP_LOGD("InitCpuctlTasksFds start.");
    return true;
}

bool CgroupManager::InitFreezerTasksFds(UniqueFd freezerTasksFds[SCHED_POLICY_FREEZER_MAX])
{
    APP_LOGD("InitFreezerTasksFds start.");
    return true;
}

bool CgroupManager::InitMemoryEventControlFd(UniqueFd &memoryEventControlFd)
{
    APP_LOGD("InitMemoryEventControlFd start.");
    return true;
}

bool CgroupManager::InitMemoryEventFds(UniqueFd memoryEventFds[LOW_MEMORY_LEVEL_MAX])
{
    APP_LOGD("InitMemoryEventFds start.");
    return true;
}

bool CgroupManager::InitMemoryPressureFds(UniqueFd memoryPressureFds[LOW_MEMORY_LEVEL_MAX])
{
    APP_LOGD("InitMemoryPressureFds start.");
    return true;
}

bool CgroupManager::SetCpusetSubsystem(const int tid, const SchedPolicy schedPolicy)
{
    APP_LOGD("SetCpusetSubsystem start.");
    return true;
}

bool CgroupManager::SetCpuctlSubsystem(const int tid, const SchedPolicy schedPolicy)
{
    APP_LOGD("SetCpuctlSubsystem start.");
    return true;
}

bool CgroupManager::SetFreezerSubsystem(const int tid, const SchedPolicyFreezer state)
{
    APP_LOGD("SetFreezerSubsystem start.");
    return true;
}
}  // namespace AppExecFwk
}  // namespace OHOS
