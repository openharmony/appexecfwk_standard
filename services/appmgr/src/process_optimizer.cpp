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

#include "process_optimizer.h"

#include <algorithm>
#include <cerrno>
#include <string>
#include "app_log_wrapper.h"

namespace OHOS {
namespace AppExecFwk {
using namespace std::placeholders;

namespace {
constexpr auto APP_SUSPEND_TIMER_NAME_PREFIX = "AppSuspendTimer";

constexpr int APP_OOM_ADJ_SYSTEM = -100;
// foreground process oom_adj
constexpr int APP_OOM_ADJ_FOREGROUND = 0;

// visible process oom_adj
constexpr int APP_OOM_ADJ_VISIBLE_MIN = 1;
constexpr int APP_OOM_ADJ_VISIBLE_MAX = 199;
constexpr int APP_OOM_ADJ_VISIBLE_MAX_VALUE = 1 * 1024;

// preceptible process oom_adj
constexpr int APP_OOM_ADJ_PERCEPTIBLE_MIN = 200;
constexpr int APP_OOM_ADJ_PERCEPTIBLE_MAX = 399;
constexpr int APP_OOM_ADJ_PERCEPTIBLE_MAX_VALUE = 4 * 1024;

// background process oom_adj
constexpr int APP_OOM_ADJ_BACKGROUND_MIN = 400;
constexpr int APP_OOM_ADJ_BACKGROUND_MAX = 599;
constexpr int APP_OOM_ADJ_BACKGROUND_MAX_VALUE = 16 * 1024;

// suspend process oom_adj
constexpr int APP_OOM_ADJ_SUSPEND_MIN = 600;
constexpr int APP_OOM_ADJ_SUSPEND_MAX = 799;
constexpr int APP_OOM_ADJ_SUSPEND_MAX_VALUE = 24 * 1024;

// empty process oom_adj
[[maybe_unused]] constexpr int APP_OOM_ADJ_EMPTY_MIN = 800;
constexpr int APP_OOM_ADJ_EMPTY_MAX = 999;
constexpr int APP_OOM_ADJ_EMPTY_MAX_VALUE = 32 * 1024;

constexpr int APP_OOM_ADJ_UNKNOWN = 1000;
constexpr int APP_OOM_ADJ_UNKNOWN_VALUE = 64 * 1024;

constexpr std::string_view SYSTEM_UI_BUNDLE_NAME = "com.ohos.systemui";

constexpr int TIME_ADVANCE_RATE = 1000;

// pressure level low
constexpr int LMKS_OOM_ADJ_LOW = 800;
// pressure level medium
constexpr int LMKS_OOM_ADJ_MEDIUM = 600;
// pressure level critical
constexpr int LMKS_OOM_ADJ_CRITICAL = 0;
constexpr int G_MEMORY_LEVEL[] = {LMKS_OOM_ADJ_LOW, LMKS_OOM_ADJ_MEDIUM, LMKS_OOM_ADJ_CRITICAL};
}  // namespace

ProcessOptimizer::ProcessOptimizer(const LmksClientPtr &lmksClient, int suspendTimeout)
    : lmksClient_(lmksClient), suspendTimeout_(suspendTimeout)
{}

ProcessOptimizer::~ProcessOptimizer()
{
    if (lmksClient_) {
        lmksClient_->ProcPurge();
    }
}

bool ProcessOptimizer::Init()
{
    APP_LOGI("initializing...");

    if (suspendTimeout_ < 0) {
        APP_LOGE("invalid suspend timeout.");
        return false;
    }

    if (eventHandler_) {
        APP_LOGE("already inited.");
        return false;
    }

    // Initializing cgroup manager.
    if (!DelayedSingleton<CgroupManager>::GetInstance()->IsInited()) {
        APP_LOGW("cgroup manager not inited.");
        if (!DelayedSingleton<CgroupManager>::GetInstance()->Init()) {
            APP_LOGE("cannot init cgroup manager.");
            return false;
        }
    }

    if (DelayedSingleton<CgroupManager>::GetInstance()->LowMemoryAlert) {
        APP_LOGW("cgroup manager 'LowMemoryWarning' already registered.");
    }

    DelayedSingleton<CgroupManager>::GetInstance()->LowMemoryAlert =
        std::bind(&ProcessOptimizer::OnLowMemoryAlert, this, _1);

    // Initializing lmks.
    LmksClientPtr lmksClientLocal = nullptr;
    LmksClient::Targets targets = {
        {APP_OOM_ADJ_VISIBLE_MAX_VALUE, APP_OOM_ADJ_VISIBLE_MAX},
        {APP_OOM_ADJ_PERCEPTIBLE_MAX_VALUE, APP_OOM_ADJ_PERCEPTIBLE_MAX},
        {APP_OOM_ADJ_BACKGROUND_MAX_VALUE, APP_OOM_ADJ_BACKGROUND_MAX},
        {APP_OOM_ADJ_SUSPEND_MAX_VALUE, APP_OOM_ADJ_SUSPEND_MAX},
        {APP_OOM_ADJ_EMPTY_MAX_VALUE, APP_OOM_ADJ_EMPTY_MAX},
        {APP_OOM_ADJ_UNKNOWN_VALUE, APP_OOM_ADJ_UNKNOWN},
    };

    lmksClientLocal.swap(lmksClient_);
    if (!lmksClientLocal) {
        lmksClientLocal = std::make_shared<LmksClient>();
        if (!lmksClientLocal) {
            APP_LOGE("failed to create lmks client.");
            return false;
        }
    }

    if (!lmksClientLocal->IsOpen()) {
        if (lmksClientLocal->Open()) {
            APP_LOGE("cannot open lmks connection.");
            return false;
        }
    }

    if (lmksClientLocal->Target(targets) != ERR_OK) {
        // print warning when lmks server not implement.
        APP_LOGW("cannot init lmks.");
    }
    lmksClient_ = lmksClientLocal;

    // Save initialized states.
    auto eventHandler = std::make_shared<EventHandler>(EventRunner::Create());
    if (!eventHandler) {
        APP_LOGE("no available event handler for current thread.");
        return false;
    }

    eventHandler_ = eventHandler;

    return true;
}

void ProcessOptimizer::OnAppAdded(const AppPtr &app)
{
    if (!app) {
        APP_LOGE("invalid app.");
        return;
    }

    if (app != appLru_.front()) {
        auto it = std::find(appLru_.begin(), appLru_.end(), app);
        if (it != appLru_.end()) {
            APP_LOGE("app '%{public}s' already existed.", app->GetName().c_str());
            appLru_.erase(it);
        }
        if (app->GetState() == ApplicationState::APP_STATE_FOREGROUND) {
            appLru_.push_front(app);
        } else {
            appLru_.push_back(app);
        }
    }
    // Initial sched policy has been already set by appspawn.
    UpdateAppOomAdj(app);
}

void ProcessOptimizer::OnAppRemoved(const AppPtr &app)
{
    if (!app) {
        APP_LOGE("invalid app.");
        return;
    }

    if (!lmksClient_) {
        APP_LOGE("invalid lmks client.");
        return;
    }

    // remove timer
    StopAppSuspendTimer(app);

    auto it = std::find(appLru_.begin(), appLru_.end(), app);
    if (it != appLru_.end()) {
        appLru_.erase(it);
    } else {
        APP_LOGE("app '%{public}s' is not existed.", app->GetName().c_str());
    }

    auto priorityObject = app->GetPriorityObject();
    if (!priorityObject) {
        APP_LOGE("invalid priority object.");
        return;
    }

    if (lmksClient_->ProcRemove(priorityObject->GetPid()) != ERR_OK) {
        APP_LOGE("failed to remove app '%{public}s'(%{publid}d) from lmks.",
            app->GetName().c_str(),
            priorityObject->GetPid());
    }
}

void ProcessOptimizer::OnAppStateChanged(const AppPtr &app, const ApplicationState oldState)
{
    if (!app) {
        APP_LOGE("invalid app.");
        return;
    }

    auto priorityObject = app->GetPriorityObject();
    if (!priorityObject) {
        APP_LOGE("invalid priority object.");
        return;
    }

    auto curState = app->GetState();
    APP_LOGD("ability state changed to %{public}d.", curState);
    if (curState == oldState) {
        APP_LOGW("no change.");
        return;
    }

    if (curState == ApplicationState::APP_STATE_FOREGROUND) {
        if (app != appLru_.front()) {
            auto it = std::find(appLru_.begin(), appLru_.end(), app);
            if (it != appLru_.end()) {
                appLru_.erase(it);
            } else {
                APP_LOGE("app '%{public}s' is not existed.", app->GetName().c_str());
            }
            appLru_.push_front(app);
        }
    }

    UpdateAppSchedPolicy(app);
    UpdateAppOomAdj(app);

    // only background also no visible and no preceptible can freezer
    if (curState == ApplicationState::APP_STATE_BACKGROUND &&
        (!priorityObject->GetVisibleStatus() && !priorityObject->GetPerceptibleStatus())) {
        StartAppSuspendTimer(app);
    }

    if (oldState == ApplicationState::APP_STATE_BACKGROUND && curState != ApplicationState::APP_STATE_SUSPENDED) {
        StopAppSuspendTimer(app);
    }
}

void ProcessOptimizer::OnAbilityStarted(const AbilityPtr &ability)
{
    if (!ability) {
        APP_LOGE("invalid ability.");
        return;
    }

    APP_LOGI("OnAbilityStarted.");
}

void ProcessOptimizer::OnAbilityConnected(const AbilityPtr &ability, const AbilityPtr &targetAbility)
{
    if (!ability) {
        APP_LOGE("invalid ability.");
        return;
    }

    if (!targetAbility) {
        APP_LOGE("invalid targetAbility.");
        return;
    }

    APP_LOGI("OnAbilityConnected end.");
}

void ProcessOptimizer::OnAbilityDisconnected(const AbilityPtr &ability, const AbilityPtr &targetAbility)
{
    if (!ability) {
        APP_LOGE("invalid ability.");
        return;
    }

    if (!targetAbility) {
        APP_LOGE("invalid targetAbility.");
        return;
    }

    APP_LOGI("OnAbilityDisconnected end.");
}

void ProcessOptimizer::OnAbilityStateChanged(const AbilityPtr &ability, const AbilityState oldState)
{
    if (!ability) {
        APP_LOGE("invalid ability.");
        return;
    }

    APP_LOGI("OnAbilityStateChanged end.");
}

void ProcessOptimizer::OnAbilityVisibleChanged(const AbilityPtr &ability)
{
    if (!ability) {
        APP_LOGE("invalid ability.");
        return;
    }

    APP_LOGI("OnAbilityVisibleChanged end.");
}

void ProcessOptimizer::OnAbilityPerceptibleChanged(const AbilityPtr &ability)
{
    if (!ability) {
        APP_LOGE("invalid ability.");
    }

    APP_LOGI("OnAbilityPerceptibleChanged end.");
}

void ProcessOptimizer::OnAbilityRemoved(const AbilityPtr &ability)
{
    if (!ability) {
        APP_LOGE("invalid ability.");
        return;
    }

    APP_LOGI("OnAbilityRemoved end.");
}

bool ProcessOptimizer::SetAppOomAdj(const AppPtr &app, int oomAdj)
{
    if (!app) {
        APP_LOGE("invalid app.");
        return false;
    }

    if (!LmksClient::CheckOomAdj(oomAdj)) {
        APP_LOGE("invalid oom adj %{public}d.", oomAdj);
        return false;
    }

    if (!lmksClient_) {
        APP_LOGE("invalid lmks client.");
        return false;
    }

    auto priorityObject = app->GetPriorityObject();
    if (!priorityObject) {
        APP_LOGE("invalid priority object.");
        return false;
    }

    if (priorityObject->GetCurAdj() == oomAdj) {
        APP_LOGW("oom adj has no change.");
        return true;
    }

    if (lmksClient_->ProcPrio(priorityObject->GetPid(), app->GetUid(), oomAdj) != ERR_OK) {
        // print warning when lmks server not implement.
        APP_LOGW("lmks proc prio failed.");
    }

    priorityObject->SetCurAdj(oomAdj);

    return true;
}

bool ProcessOptimizer::SetAppSchedPolicy(const AppPtr &app, const CgroupManager::SchedPolicy schedPolicy)
{
    if (!app) {
        APP_LOGE("invalid app.");
        return false;
    }

    auto priorityObject = app->GetPriorityObject();
    if (!priorityObject) {
        APP_LOGE("invalid priority object.");
        return false;
    }

    auto oldSchedPolicy = priorityObject->GetCurCgroup();
    if (oldSchedPolicy == schedPolicy) {
        APP_LOGW("no change.");
        return true;
    }

    bool result =
        DelayedSingleton<CgroupManager>::GetInstance()->SetProcessSchedPolicy(priorityObject->GetPid(), schedPolicy);
    if (result) {
        priorityObject->SetCurCgroup(schedPolicy);
        if (schedPolicy == CgroupManager::SCHED_POLICY_FREEZED) {
            if (AppSuspended) {
                AppSuspended(app);
            }
        } else if (oldSchedPolicy == CgroupManager::SCHED_POLICY_FREEZED) {
            if (AppResumed) {
                AppResumed(app);
            }
        }
    }

    return result;
}

void ProcessOptimizer::OnLowMemoryAlert(const CgroupManager::LowMemoryLevel level)
{
    APP_LOGI("OnLowMemoryAlert level %{public}d", level);
    // Find the oldest background app.
    for (auto it(appLru_.rbegin()); it != appLru_.rend(); ++it) {
        auto appRecord = (*it);
        if (appRecord && appRecord->GetState() == ApplicationState::APP_STATE_BACKGROUND) {
            APP_LOGI("OnLowMemoryAlert bundle:%{public}s, level:%{public}d",
                appRecord->GetBundleName().c_str(), level);
            AppLowMemoryAlert(*it, level);
            break;
        }
    }

    // send pid which will be killed.
    std::list<AppPtr>::iterator iter = appLru_.begin();
    while (iter != appLru_.end()) {
        auto appRecord = (*iter);
        if (!appRecord) {
            continue;
        }
        auto priorityObject = appRecord->GetPriorityObject();
        if (priorityObject != nullptr && priorityObject->GetCurAdj() >= G_MEMORY_LEVEL[level]) {
            auto pid = priorityObject->GetPid();
            if (pid <= 0) {
                APP_LOGE("pid %{public}d invalid", pid);
                iter = appLru_.erase(iter);
                continue;
            }

            APP_LOGI("remove pid (%{publid}d) from lmks, bundle:%{public}s", pid, appRecord->GetBundleName().c_str());
            if (lmksClient_->ProcRemove(pid) == ERR_OK) {
                iter = appLru_.erase(iter);
                continue;
            }
            APP_LOGE("remove error, pid:%{publid}d, bundle:%{public}s", pid, appRecord->GetBundleName().c_str());
        }
        iter++;
    }
}

bool ProcessOptimizer::UpdateAppOomAdj(const AppPtr &app)
{
    if (!app) {
        APP_LOGE("invalid app.");
        return false;
    }
    APP_LOGI("UpdateAppOomAdj bundleName[%{public}s] state[%{public}d] pid[%{public}d] curadj[%{public}d]",
        app->GetBundleName().c_str(),
        app->GetState(),
        app->GetPriorityObject()->GetPid(),
        app->GetPriorityObject()->GetCurAdj());

    auto priorityObject = app->GetPriorityObject();
    if (!priorityObject) {
        APP_LOGE("invalid priority object.");
        return false;
    }
    // special set launcher and systemui adj
    if (app->IsLauncherApp() || app->GetBundleName() == SYSTEM_UI_BUNDLE_NAME) {
        return SetAppOomAdj(app, APP_OOM_ADJ_SYSTEM);
    }

    auto state = app->GetState();
    if (state == ApplicationState::APP_STATE_FOREGROUND || state == ApplicationState::APP_STATE_CREATE ||
        state == ApplicationState::APP_STATE_READY) {
        return SetAppOomAdj(app, APP_OOM_ADJ_FOREGROUND);
    }

    int oomAdj;
    int oomAdjMax;
    if (!GetAppOomAdj(app, state, oomAdj, oomAdjMax)) {
        return false;
    }

    for (auto curApp : appLru_) {
        if (curApp->GetState() != state) {
            continue;
        }
        // adj of launcher and systemui is always APP_OOM_ADJ_SYSTEM
        if (curApp->IsLauncherApp() || curApp->GetBundleName() == SYSTEM_UI_BUNDLE_NAME) {
            continue;
        }

        SetAppOomAdj(curApp, oomAdj);
        if (oomAdj < oomAdjMax) {
            oomAdj += 1;
        }
    }
    return true;
}

bool ProcessOptimizer::GetAppOomAdj(const AppPtr &app, ApplicationState state, int &oomAdj, int &oomAdjMax)
{
    auto priorityObject = app->GetPriorityObject();
    if (!priorityObject) {
        APP_LOGE("invalid priority object.");
        return false;
    }

    switch (state) {
        case ApplicationState::APP_STATE_BACKGROUND:
            oomAdj = APP_OOM_ADJ_BACKGROUND_MIN;
            oomAdjMax = APP_OOM_ADJ_BACKGROUND_MAX;

            // perceptible oom_adj
            if (priorityObject->GetPerceptibleStatus()) {
                oomAdj = APP_OOM_ADJ_PERCEPTIBLE_MIN;
                oomAdjMax = APP_OOM_ADJ_PERCEPTIBLE_MAX;
            }

            // visible oom_adj
            if (priorityObject->GetVisibleStatus()) {
                oomAdj = APP_OOM_ADJ_VISIBLE_MIN;
                oomAdjMax = APP_OOM_ADJ_VISIBLE_MAX;
            }
            break;
        case ApplicationState::APP_STATE_SUSPENDED:
            oomAdj = APP_OOM_ADJ_SUSPEND_MIN;
            oomAdjMax = APP_OOM_ADJ_SUSPEND_MAX;
            break;
        default:
            oomAdj = APP_OOM_ADJ_UNKNOWN;
            oomAdjMax = APP_OOM_ADJ_UNKNOWN;
            break;
    }
    return true;
}

bool ProcessOptimizer::UpdateAppSchedPolicy(const AppPtr &app)
{
    if (!app) {
        APP_LOGE("invalid app.");
        return false;
    }

    bool ret = false;

    switch (app->GetState()) {
        case ApplicationState::APP_STATE_CREATE:
        case ApplicationState::APP_STATE_READY:
        case ApplicationState::APP_STATE_FOREGROUND:
            ret = SetAppSchedPolicy(app, CgroupManager::SCHED_POLICY_DEFAULT);
            break;
        case ApplicationState::APP_STATE_BACKGROUND:
            ret = SetAppSchedPolicy(app, CgroupManager::SCHED_POLICY_BACKGROUND);
            break;
        case ApplicationState::APP_STATE_SUSPENDED:
            // SUSPEND state should be set in 'ProcessOptimizer::SuspendApp()' (to be specific,
            // in 'AppSuspended' callback), and the sched policy has been set in it, so do nothing here.
            ret = true;
            break;
        default:
            ret = true;
            break;
    }
    return ret;
}

void ProcessOptimizer::StartAppSuspendTimer(const AppPtr &app)
{
    if (!app) {
        APP_LOGE("invalid app.");
        return;
    }

    APP_LOGI("starting suspend timer for app '%{public}s'...", app->GetName().c_str());

    if (!eventHandler_) {
        APP_LOGE("invalid event handler.");
        return;
    }

    auto timerName = GetAppSuspendTimerName(app);
    if (timerName.empty()) {
        APP_LOGE("invalid suspend timer name.");
        return;
    }

    auto it = suspendTimers_.find(timerName);
    if (it != suspendTimers_.end()) {
        APP_LOGW("app '%{public}s' suspend timer already started.", app->GetName().c_str());
        return;
    }

    suspendTimers_.emplace(timerName);

    eventHandler_->PostTask(
        [=]() {
            auto finder = suspendTimers_.find(timerName);
            if (finder != suspendTimers_.end()) {
                APP_LOGD("removing app '%{public}s' '%{public}s' suspend timer name...",
                    timerName.c_str(),
                    app->GetName().c_str());
                suspendTimers_.erase(finder);
            } else {
                APP_LOGE("invalid suspend timer for app '%{public}s' '%{public}s'.",
                    timerName.c_str(),
                    app->GetName().c_str());
            }
            SuspendApp(app);
        },
        timerName,
        suspendTimeout_);
}

void ProcessOptimizer::StopAppSuspendTimer(const AppPtr &app)
{
    if (!app) {
        APP_LOGE("invalid app.");
        return;
    }

    APP_LOGI("stopping suspend timer for app '%{public}s'...", app->GetName().c_str());

    if (!eventHandler_) {
        APP_LOGE("invalid event handler.");
        return;
    }

    auto timerName = GetAppSuspendTimerName(app);
    if (timerName.empty()) {
        APP_LOGE("invalid suspend timer name.");
        return;
    }

    auto it = suspendTimers_.find(timerName);
    if (it == suspendTimers_.end()) {
        APP_LOGW("app '%{public}s' suspend timer not started.", app->GetName().c_str());
        return;
    }

    suspendTimers_.erase(it);
    eventHandler_->RemoveTask(timerName);
}

void ProcessOptimizer::SuspendApp(const AppPtr &app)
{
    if (!app) {
        APP_LOGE("invalid app.");
        return;
    }

    APP_LOGI("suspending app '%{public}s'...", app->GetName().c_str());

    if (app->GetState() == ApplicationState::APP_STATE_SUSPENDED) {
        APP_LOGE("app '%{public}s' already suspended.", app->GetName().c_str());
        return;
    }

    if (!SetAppSchedPolicy(app, CgroupManager::SCHED_POLICY_FREEZED)) {
        APP_LOGE("failed to suspend app '%s'.", app->GetName().c_str());
    }
}

std::string ProcessOptimizer::GetAppSuspendTimerName(const AppPtr &app)
{
    std::string ret;

    if (app) {
        auto priorityObject = app->GetPriorityObject();
        if (priorityObject) {
            ret = APP_SUSPEND_TIMER_NAME_PREFIX + std::to_string(priorityObject->GetPid());
        }
    }

    return ret;
}

void ProcessOptimizer::SetAppFreezingTime(int time)
{
    APP_LOGE("input second time:[%{public}d]", time);

    if (time > APP_SUSPEND_TIMEOUT_MAX && time < 0) {
        APP_LOGE("input time error.");
        return;
    }

    suspendTimeout_ = time;
    // convert seconds to milliseconds
    suspendTimeout_ *= TIME_ADVANCE_RATE;
}

void ProcessOptimizer::GetAppFreezingTime(int &time)
{
    time = suspendTimeout_ / TIME_ADVANCE_RATE;
    APP_LOGE("current freez time:[%{public}d]", time);
    return;
}
}  // namespace AppExecFwk
}  // namespace OHOS
