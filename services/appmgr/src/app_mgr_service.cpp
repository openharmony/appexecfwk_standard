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

#include "app_mgr_service.h"

#include <nlohmann/json.hpp>
#include <sys/types.h>

#include "datetime_ex.h"
#include "ipc_skeleton.h"
#include "system_ability_definition.h"

#include "app_death_recipient.h"
#include "app_log_wrapper.h"
#include "app_mgr_constants.h"
#include "perf_profile.h"

#include "system_environment_information.h"

namespace OHOS {
namespace AppExecFwk {
namespace {
static const int EXPERIENCE_MEM_THRESHOLD = 20;
static const float PERCENTAGE = 100.0;
const std::string TASK_ATTACH_APPLICATION = "AttachApplicationTask";
const std::string TASK_APPLICATION_FOREGROUNDED = "ApplicationForegroundedTask";
const std::string TASK_APPLICATION_BACKGROUNDED = "ApplicationBackgroundedTask";
const std::string TASK_APPLICATION_TERMINATED = "ApplicationTerminatedTask";
const std::string TASK_ABILITY_CLEANED = "AbilityCleanedTask";
const std::string TASK_ADD_APP_DEATH_RECIPIENT = "AddAppRecipientTask";
const std::string TASK_CLEAR_UP_APPLICATION_DATA = "ClearUpApplicationDataTask";
const std::string TASK_STARTUP_RESIDENT_PROCESS = "StartupResidentProcess";
const std::string TASK_ADD_ABILITY_STAGE_DONE = "AddAbilityStageDone";
}  // namespace

REGISTER_SYSTEM_ABILITY_BY_ID(AppMgrService, APP_MGR_SERVICE_ID, true);

AppMgrService::AppMgrService()
{
    appMgrServiceInner_ = std::make_shared<AppMgrServiceInner>();
    APP_LOGI("instance created with no para");
    PerfProfile::GetInstance().SetAmsLoadStartTime(GetTickCount());
}

AppMgrService::AppMgrService(const int32_t serviceId, bool runOnCreate) : SystemAbility(serviceId, runOnCreate)
{
    appMgrServiceInner_ = std::make_shared<AppMgrServiceInner>();
    APP_LOGI("instance created");
    PerfProfile::GetInstance().SetAmsLoadStartTime(GetTickCount());
}

AppMgrService::~AppMgrService()
{
    APP_LOGI("instance destroyed");
}

void AppMgrService::OnStart()
{
    APP_LOGI("ready to start service");
    if (appMgrServiceState_.serviceRunningState == ServiceRunningState::STATE_RUNNING) {
        APP_LOGW("failed to start service since it's already running");
        return;
    }

    ErrCode errCode = Init();
    if (FAILED(errCode)) {
        APP_LOGE("init failed, errCode: %{public}08x", errCode);
        return;
    }
    appMgrServiceState_.serviceRunningState = ServiceRunningState::STATE_RUNNING;
    APP_LOGI("start service success");
    PerfProfile::GetInstance().SetAmsLoadEndTime(GetTickCount());
    PerfProfile::GetInstance().Dump();
}

void AppMgrService::OnStop()
{
    APP_LOGI("ready to stop service");
    appMgrServiceState_.serviceRunningState = ServiceRunningState::STATE_NOT_START;
    handler_.reset();
    runner_.reset();
    if (appMgrServiceInner_) {
        appMgrServiceInner_->OnStop();
    }
    APP_LOGI("stop service success");
}

void AppMgrService::SetInnerService(const std::shared_ptr<AppMgrServiceInner> &innerService)
{
    appMgrServiceInner_ = innerService;
}

AppMgrServiceState AppMgrService::QueryServiceState()
{
    if (appMgrServiceInner_) {
        appMgrServiceState_.connectionState = appMgrServiceInner_->QueryAppSpawnConnectionState();
    }
    return appMgrServiceState_;
}

ErrCode AppMgrService::Init()
{
    APP_LOGI("ready to init");
    // start main thread message loop.
    runner_ = EventRunner::Create(Constants::APP_MGR_SERVICE_NAME);
    if (!runner_) {
        APP_LOGE("init failed due to create runner error");
        return ERR_INVALID_OPERATION;
    }
    if (!appMgrServiceInner_) {
        APP_LOGE("init failed without inner service");
        return ERR_INVALID_OPERATION;
    }
    handler_ = std::make_shared<AMSEventHandler>(runner_, appMgrServiceInner_);
    if (!handler_) {
        APP_LOGE("init failed without handler");
        return ERR_INVALID_OPERATION;
    }
    appMgrServiceInner_->SetEventHandler(handler_);
    ErrCode openErr = appMgrServiceInner_->OpenAppSpawnConnection();
    if (FAILED(openErr)) {
        APP_LOGW("failed to connect to AppSpawnDaemon! errCode: %{public}08x", openErr);
    }
    if (!Publish(this)) {
        APP_LOGE("failed to publish appmgrservice to systemAbilityMgr");
        return ERR_APPEXECFWK_SERVICE_NOT_CONNECTED;
    }
    amsMgrScheduler_ = new (std::nothrow) AmsMgrScheduler(appMgrServiceInner_, handler_);
    if (!amsMgrScheduler_) {
        APP_LOGE("init failed without ams scheduler");
        return ERR_INVALID_OPERATION;
    }
    if (appMgrServiceInner_->ProcessOptimizerInit() != ERR_OK) {
        APP_LOGE("init failed without process optimizer");
    }
    APP_LOGI("init success");
    return ERR_OK;
}

int32_t AppMgrService::CheckPermission(
    [[maybe_unused]] const int32_t recordId, [[maybe_unused]] const std::string &permission)
{
    APP_LOGI("check application's permission");

    return ERR_OK;
}

void AppMgrService::AttachApplication(const sptr<IRemoteObject> &app)
{
    if (!IsReady()) {
        return;
    }

    pid_t pid = IPCSkeleton::GetCallingPid();
    AddAppDeathRecipient(pid);
    std::function<void()> attachApplicationFunc =
        std::bind(&AppMgrServiceInner::AttachApplication, appMgrServiceInner_, pid, iface_cast<IAppScheduler>(app));
    handler_->PostTask(attachApplicationFunc, TASK_ATTACH_APPLICATION);
}

void AppMgrService::ApplicationForegrounded(const int32_t recordId)
{
    if (!IsReady()) {
        return;
    }
    std::function<void()> applicationForegroundedFunc =
        std::bind(&AppMgrServiceInner::ApplicationForegrounded, appMgrServiceInner_, recordId);
    handler_->PostTask(applicationForegroundedFunc, TASK_APPLICATION_FOREGROUNDED);
}

void AppMgrService::ApplicationBackgrounded(const int32_t recordId)
{
    if (!IsReady()) {
        return;
    }
    std::function<void()> applicationBackgroundedFunc =
        std::bind(&AppMgrServiceInner::ApplicationBackgrounded, appMgrServiceInner_, recordId);
    handler_->PostTask(applicationBackgroundedFunc, TASK_APPLICATION_BACKGROUNDED);
}

void AppMgrService::ApplicationTerminated(const int32_t recordId)
{
    if (!IsReady()) {
        return;
    }
    std::function<void()> applicationTerminatedFunc =
        std::bind(&AppMgrServiceInner::ApplicationTerminated, appMgrServiceInner_, recordId);
    handler_->PostTask(applicationTerminatedFunc, TASK_APPLICATION_TERMINATED);
}

void AppMgrService::AbilityCleaned(const sptr<IRemoteObject> &token)
{
    if (!IsReady()) {
        return;
    }
    std::function<void()> abilityCleanedFunc =
        std::bind(&AppMgrServiceInner::AbilityTerminated, appMgrServiceInner_, token);
    handler_->PostTask(abilityCleanedFunc, TASK_ABILITY_CLEANED);
}

bool AppMgrService::IsReady() const
{
    if (!appMgrServiceInner_) {
        APP_LOGE("appMgrServiceInner is null");
        return false;
    }
    if (!handler_) {
        APP_LOGE("handler is null");
        return false;
    }
    return true;
}

void AppMgrService::AddAppDeathRecipient(const pid_t pid) const
{
    if (!IsReady()) {
        return;
    }
    sptr<AppDeathRecipient> appDeathRecipient = new AppDeathRecipient();
    appDeathRecipient->SetEventHandler(handler_);
    appDeathRecipient->SetAppMgrServiceInner(appMgrServiceInner_);
    std::function<void()> addAppRecipientFunc =
        std::bind(&AppMgrServiceInner::AddAppDeathRecipient, appMgrServiceInner_, pid, appDeathRecipient);
    handler_->PostTask(addAppRecipientFunc, TASK_ADD_APP_DEATH_RECIPIENT);
}

void AppMgrService::StartupResidentProcess()
{
    if (!IsReady()) {
        return;
    }

    std::function <void()> startupResidentProcess =
        std::bind(&AppMgrServiceInner::LoadResidentProcess, appMgrServiceInner_);
    handler_->PostTask(startupResidentProcess, TASK_STARTUP_RESIDENT_PROCESS);
}

sptr<IAmsMgr> AppMgrService::GetAmsMgr()
{
    return amsMgrScheduler_;
}

int32_t AppMgrService::ClearUpApplicationData(const std::string &bundleName)
{
    if (!IsReady()) {
        return ERR_INVALID_OPERATION;
    }
    int32_t uid = IPCSkeleton::GetCallingUid();
    pid_t pid = IPCSkeleton::GetCallingPid();
    std::function<void()> clearUpApplicationDataFunc =
        std::bind(&AppMgrServiceInner::ClearUpApplicationData, appMgrServiceInner_, bundleName, uid, pid);
    handler_->PostTask(clearUpApplicationDataFunc, TASK_CLEAR_UP_APPLICATION_DATA);
    return ERR_OK;
}

int32_t AppMgrService::IsBackgroundRunningRestricted(const std::string &bundleName)
{
    if (!IsReady()) {
        return ERR_INVALID_OPERATION;
    }
    return appMgrServiceInner_->IsBackgroundRunningRestricted(bundleName);
}

int32_t AppMgrService::GetAllRunningProcesses(std::vector<RunningProcessInfo> &info)
{
    if (!IsReady()) {
        return ERR_INVALID_OPERATION;
    }
    return appMgrServiceInner_->GetAllRunningProcesses(info);
}

void AppMgrService::SetAppFreezingTime(int time)
{
    APP_LOGE("set app freeze time %{public}d ", time);
    if (!IsReady()) {
        return;
    }
    appMgrServiceInner_->SetAppFreezingTime(time);
}

void AppMgrService::GetAppFreezingTime(int &time)
{
    if (!IsReady()) {
        return;
    }
    appMgrServiceInner_->GetAppFreezingTime(time);
    APP_LOGE("get app freeze time %{public}d ", time);
}

/**
 * Get system memory information.
 * @param SystemMemoryAttr, memory information.
 */
void AppMgrService::GetSystemMemoryAttr(SystemMemoryAttr &memoryInfo, std::string &strConfig)
{
    SystemEnv::KernelSystemMemoryInfo systemMemInfo;
    SystemEnv::GetMemInfo(systemMemInfo);
    int memThreshold = 0;
    nlohmann::json memJson = nlohmann::json::parse(strConfig, nullptr, false);
    if (memJson.is_discarded()) {
        memThreshold = EXPERIENCE_MEM_THRESHOLD;
        APP_LOGE("%{public}s, discarded memThreshold = %{public}d", __func__, EXPERIENCE_MEM_THRESHOLD);
    } else {
        if (!memJson.contains("memoryThreshold")) {
            memThreshold = EXPERIENCE_MEM_THRESHOLD;
            APP_LOGE("%{public}s, memThreshold = %{public}d", __func__, EXPERIENCE_MEM_THRESHOLD);
        } else {
            memThreshold = memJson.at("memorythreshold").get<int>();
            APP_LOGI("memThreshold = %{public}d", memThreshold);
        }
    }

    memoryInfo.availSysMem_ = systemMemInfo.GetMemFree();
    memoryInfo.totalSysMem_ = systemMemInfo.GetMemTotal();
    memoryInfo.threshold_ = static_cast<int64_t>(memoryInfo.totalSysMem_ * memThreshold / PERCENTAGE);
    memoryInfo.isSysInlowMem_ = memoryInfo.availSysMem_ < memoryInfo.threshold_;
}

void AppMgrService::AddAbilityStageDone(const int32_t recordId)
{
    if (!IsReady()) {
        return;
    }
    std::function <void()> addAbilityStageDone =
        std::bind(&AppMgrServiceInner::AddAbilityStageDone, appMgrServiceInner_, recordId);
    handler_->PostTask(addAbilityStageDone, TASK_ADD_ABILITY_STAGE_DONE);
}

int32_t AppMgrService::RegisterApplicationStateObserver(const sptr<IApplicationStateObserver> &observer)
{
    APP_LOGI("%{public}s begin", __func__);
    if (!IsReady()) {
        APP_LOGE("%{public}s begin, not ready", __func__);
        return ERR_INVALID_OPERATION;
    }
    return appMgrServiceInner_->RegisterApplicationStateObserver(observer);
}

int32_t AppMgrService::UnregisterApplicationStateObserver(const sptr<IApplicationStateObserver> &observer)
{
    APP_LOGI("%{public}s begin", __func__);
    if (!IsReady()) {
        APP_LOGE("%{public}s begin, not ready", __func__);
        return ERR_INVALID_OPERATION;
    }
    return appMgrServiceInner_->UnregisterApplicationStateObserver(observer);
}

int32_t AppMgrService::GetForegroundApplications(std::vector<AppStateData> &list)
{
    APP_LOGI("%{public}s begin", __func__);
    if (!IsReady()) {
        APP_LOGE("%{public}s begin, not ready", __func__);
        return ERR_INVALID_OPERATION;
    }
    return appMgrServiceInner_->GetForegroundApplications(list);
}
}  // namespace AppExecFwk
}  // namespace OHOS
