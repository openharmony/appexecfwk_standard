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

#include "app_lifecycle_deal.h"
#include "app_log_wrapper.h"
#include "bytrace.h"

namespace OHOS {
namespace AppExecFwk {
AppLifeCycleDeal::AppLifeCycleDeal()
{}

AppLifeCycleDeal::~AppLifeCycleDeal()
{}

void AppLifeCycleDeal::LaunchApplication(const AppLaunchData &launchData_)
{
    BYTRACE_NAME(BYTRACE_TAG_APP, __PRETTY_FUNCTION__);
    APP_LOGI("AppLifeCycleDeal ScheduleLaunchApplication");
    if (appThread_) {
        appThread_->ScheduleLaunchApplication(launchData_);
    }
}

void AppLifeCycleDeal::AddAbilityStageInfo(const HapModuleInfo &abilityStage)
{
    if (!appThread_) {
        APP_LOGE("appThread_ is nullptr");
        return;
    }

    appThread_->ScheduleAbilityStageInfo(abilityStage);
}

void AppLifeCycleDeal::LaunchAbility(const std::shared_ptr<AbilityRunningRecord> &ability)
{
    if (appThread_) {
        appThread_->ScheduleLaunchAbility(*(ability->GetAbilityInfo()), ability->GetToken());
    }
}

void AppLifeCycleDeal::ScheduleTerminate()
{
    if (!appThread_) {
        APP_LOGE("appThread_ is nullptr");
        return;
    }

    appThread_->ScheduleTerminateApplication();
}

void AppLifeCycleDeal::ScheduleForegroundRunning()
{
    if (!appThread_) {
        APP_LOGE("appThread_ is nullptr");
        return;
    }

    appThread_->ScheduleForegroundApplication();
}

void AppLifeCycleDeal::ScheduleBackgroundRunning()
{
    if (!appThread_) {
        APP_LOGE("appThread_ is nullptr");
        return;
    }

    appThread_->ScheduleBackgroundApplication();
}

void AppLifeCycleDeal::ScheduleTrimMemory(int32_t timeLevel)
{
    if (!appThread_) {
        APP_LOGE("appThread_ is nullptr");
        return;
    }

    appThread_->ScheduleShrinkMemory(timeLevel);
}

void AppLifeCycleDeal::LowMemoryWarning()
{
    if (!appThread_) {
        APP_LOGE("appThread_ is nullptr");
        return;
    }

    appThread_->ScheduleLowMemory();
}

void AppLifeCycleDeal::ScheduleCleanAbility(const sptr<IRemoteObject> &token)
{
    if (!appThread_) {
        APP_LOGE("appThread_ is nullptr");
        return;
    }
    appThread_->ScheduleCleanAbility(token);
}

void AppLifeCycleDeal::ScheduleProcessSecurityExit()
{
    if (!appThread_) {
        APP_LOGE("appThread_ is nullptr");
        return;
    }

    appThread_->ScheduleProcessSecurityExit();
}

void AppLifeCycleDeal::SetApplicationClient(const sptr<IAppScheduler> &thread)
{
    appThread_ = thread;
}

sptr<IAppScheduler> AppLifeCycleDeal::GetApplicationClient() const
{
    return appThread_;
}
}  // namespace AppExecFwk
}  // namespace OHOS