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

#include "process_optimizer_uba.h"
#include "app_log_wrapper.h"

namespace OHOS {
namespace AppExecFwk {
ProcessOptimizerUBA::BaseAbilityAction::BaseAbilityAction(const AbilityPtr &ability)
    : time_(Clock::now()), name_(ability->GetName())
{}

ProcessOptimizerUBA::TimePoint ProcessOptimizerUBA::BaseAbilityAction::GetTime() const
{
    return time_;
}

std::string ProcessOptimizerUBA::BaseAbilityAction::GetTimeString() const
{
    std::time_t tmp = Clock::to_time_t(time_);
    char *pTime = std::ctime(&tmp);
    return (pTime == nullptr) ? "" : std::string(pTime);
}

const std::string &ProcessOptimizerUBA::BaseAbilityAction::GetName() const
{
    return name_;
}

ProcessOptimizerUBA::StartAbilityAction::StartAbilityAction(const AbilityPtr &ability, const AbilityPtr &preAbility)
    : BaseAbilityAction(ability), preName_(preAbility ? preAbility->GetName() : std::string())
{}

const std::string &ProcessOptimizerUBA::StartAbilityAction::GetPreName() const
{
    return preName_;
}

ProcessOptimizerUBA::ConnectAbilityAction::ConnectAbilityAction(
    const AbilityPtr &ability, const AbilityPtr &targetAbility)
    : BaseAbilityAction(ability), targetName_(targetAbility->GetName())
{}

const std::string &ProcessOptimizerUBA::ConnectAbilityAction::GetTargetName() const
{
    return targetName_;
}

ProcessOptimizerUBA::DisconnectAbilityAction::DisconnectAbilityAction(
    const AbilityPtr &ability, const AbilityPtr &targetAbility)
    : BaseAbilityAction(ability), targetName_(targetAbility->GetName())
{}

const std::string &ProcessOptimizerUBA::DisconnectAbilityAction::GetTargetName() const
{
    return targetName_;
}

ProcessOptimizerUBA::ChangeAbilityStateAction::ChangeAbilityStateAction(
    const AbilityPtr &ability, const AbilityState oldState)
    : BaseAbilityAction(ability), oldState_(oldState), newState_(ability->GetState())
{}

AbilityState ProcessOptimizerUBA::ChangeAbilityStateAction::GetOldState() const
{
    return oldState_;
}

AbilityState ProcessOptimizerUBA::ChangeAbilityStateAction::GetNewState() const
{
    return newState_;
}

ProcessOptimizerUBA::ChangeAbilityVisible::ChangeAbilityVisible(const AbilityPtr &ability)
    : BaseAbilityAction(ability){};

ProcessOptimizerUBA::ChangeAbilityPerceptible::ChangeAbilityPerceptible(const AbilityPtr &ability)
    : BaseAbilityAction(ability){};

ProcessOptimizerUBA::RemoveAbilityAction::RemoveAbilityAction(const AbilityPtr &ability) : BaseAbilityAction(ability)
{}

ProcessOptimizerUBA::ProcessOptimizerUBA(
    const UbaServicePtr &ubaService, const LmksClientPtr &lmksClient, int suspendTimeout)
    : ProcessOptimizer(lmksClient, suspendTimeout), ubaService_(ubaService), abilityActionCount_(0)
{
    APP_LOGI("ProcessOptimizerUBA ProcessOptimizerUBA start !");
}

ProcessOptimizerUBA::~ProcessOptimizerUBA()
{
    if (abilityActionCount_ > 0) {
        CommitAbilityActions();
    }
}

void ProcessOptimizerUBA::OnAppAdded(const AppPtr &app)
{
    auto ubaService = GetUbaService();
    if (ubaService) {
        APP_LOGI("ubaService implement.");
    } else {
        ProcessOptimizer::OnAppAdded(app);
    }
}

void ProcessOptimizerUBA::OnAppRemoved(const AppPtr &app)
{
    auto ubaService = GetUbaService();
    if (ubaService) {
        APP_LOGI("ubaService implement.");
    } else {
        ProcessOptimizer::OnAppRemoved(app);
    }
}

void ProcessOptimizerUBA::OnAppStateChanged(const AppPtr &app, const ApplicationState oldState)
{
    auto ubaService = GetUbaService();
    if (ubaService) {
        APP_LOGI("ubaService implement.");
    } else {
        ProcessOptimizer::OnAppStateChanged(app, oldState);
    }
}

void ProcessOptimizerUBA::OnAbilityStarted(const AbilityPtr &ability)
{
    if (!ability) {
        APP_LOGE("invalid ability.");
        return;
    }

    AbilityPtr preAbility;
    auto preToken = ability->GetPreToken();

    if (GetAbilityByToken) {
        // 'preAbility' can be a nullptr.
        preAbility = GetAbilityByToken(preToken);
    } else {
        APP_LOGW(" 'GetAbilityByToken' is not registered.");
    }

    RecordAbilityAction<StartAbilityAction>(ability, preAbility);

    auto ubaService = GetUbaService();
    if (ubaService) {
        APP_LOGI("ubaService implement.");
    } else {
        ProcessOptimizer::OnAbilityStarted(ability);
    }
}

void ProcessOptimizerUBA::OnAbilityConnected(const AbilityPtr &ability, const AbilityPtr &targetAbility)
{
    if (!ability) {
        APP_LOGE("invalid ability.");
        return;
    }

    if (!targetAbility) {
        APP_LOGE("invalid targetAbility.");
        return;
    }

    RecordAbilityAction<ConnectAbilityAction>(ability, targetAbility);

    auto ubaService = GetUbaService();
    if (ubaService) {
        APP_LOGI("ubaService implement.");
    } else {
        ProcessOptimizer::OnAbilityConnected(ability, targetAbility);
    }
}

void ProcessOptimizerUBA::OnAbilityDisconnected(const AbilityPtr &ability, const AbilityPtr &targetAbility)
{
    if (!ability) {
        APP_LOGE("invalid ability.");
        return;
    }

    if (!targetAbility) {
        APP_LOGE("invalid targetAbility.");
        return;
    }

    RecordAbilityAction<DisconnectAbilityAction>(ability, targetAbility);

    auto ubaService = GetUbaService();
    if (ubaService) {
        APP_LOGI("ubaService implement.");
    } else {
        ProcessOptimizer::OnAbilityDisconnected(ability, targetAbility);
    }
}

void ProcessOptimizerUBA::OnAbilityStateChanged(const AbilityPtr &ability, const AbilityState oldState)
{
    if (!ability) {
        APP_LOGE("invalid ability.");
        return;
    }

    RecordAbilityAction<ChangeAbilityStateAction>(ability, oldState);
    auto ubaService = GetUbaService();
    if (ubaService) {
        APP_LOGI("ubaService implement.");
    } else {
        ProcessOptimizer::OnAbilityStateChanged(ability, oldState);
    }
}

void ProcessOptimizerUBA::OnAbilityVisibleChanged(const AbilityPtr &ability)
{
    if (!ability) {
        APP_LOGE("invalid ability.");
        return;
    }

    RecordAbilityAction<ChangeAbilityVisible>(ability);
    auto ubaService = GetUbaService();
    if (ubaService) {
        APP_LOGI("ubaService implement.");
    } else {
        ProcessOptimizer::OnAbilityVisibleChanged(ability);
    }
}

void ProcessOptimizerUBA::OnAbilityPerceptibleChanged(const AbilityPtr &ability)
{
    if (!ability) {
        APP_LOGE("invalid ability.");
        return;
    }

    RecordAbilityAction<ChangeAbilityPerceptible>(ability);
    auto ubaService = GetUbaService();
    if (ubaService) {
        APP_LOGI("ubaService implement.");
    } else {
        ProcessOptimizer::OnAbilityPerceptibleChanged(ability);
    }
}

void ProcessOptimizerUBA::OnAbilityRemoved(const AbilityPtr &ability)
{
    if (!ability) {
        APP_LOGE("invalid ability.");
        return;
    }

    RecordAbilityAction<RemoveAbilityAction>(ability);

    auto ubaService = GetUbaService();
    if (ubaService) {
        APP_LOGI("ubaService implement.");
    } else {
        ProcessOptimizer::OnAbilityRemoved(ability);
    }
}

void ProcessOptimizerUBA::OnLowMemoryAlert(const CgroupManager::LowMemoryLevel level)
{
    auto ubaService = GetUbaService();
    if (ubaService) {
        APP_LOGI("ubaService implement.");
    } else {
        ProcessOptimizer::OnLowMemoryAlert(level);
    }
}

void ProcessOptimizerUBA::CommitAbilityActions()
{
    auto n = abilityActionCount_;
    abilityActionCount_ = 0;
    auto ubaService = GetUbaService();
    if (!ubaService) {
        APP_LOGE("uba is not available.");
        return;
    }

    APP_LOGI("committing %{public}zu actions...", n);

    for (size_t i = 0; i < n; ++i) {
        auto &abilityAction = abilityActionCache_[i];

        auto startAbilityAction = std::get_if<StartAbilityAction>(&abilityAction);
        if (startAbilityAction) {
            CommitStartAbilityAction(*startAbilityAction, i);
            continue;
        }
        auto connectAbilityAction = std::get_if<ConnectAbilityAction>(&abilityAction);
        if (connectAbilityAction) {
            CommitConnectAbilityAction(*connectAbilityAction, i);
            continue;
        }
        auto disconnectAbilityAction = std::get_if<DisconnectAbilityAction>(&abilityAction);
        if (disconnectAbilityAction) {
            CommitDisconnectAbilityAction(*disconnectAbilityAction, i);
            continue;
        }
        auto changedAbilityStateAction = std::get_if<ChangeAbilityStateAction>(&abilityAction);
        if (changedAbilityStateAction) {
            CommitChangedAbilityStateAction(*changedAbilityStateAction, i);
            continue;
        }
        auto removeAbilityAction = std::get_if<RemoveAbilityAction>(&abilityAction);
        if (removeAbilityAction) {
            CommitRemoveAbilityAction(*removeAbilityAction, i);
            continue;
        }
    }
}

void ProcessOptimizerUBA::CommitStartAbilityAction(const StartAbilityAction& action, size_t index)
{
    APP_LOGI("  [%{public}zu] %{public}s ability '%{public}s' starts '%{public}s'",
        index,
        action.GetTimeString().c_str(),
        action.GetPreName().c_str(),
        action.GetName().c_str());
    // commit action
}

void ProcessOptimizerUBA::CommitConnectAbilityAction(const ConnectAbilityAction& action, size_t index)
{
    APP_LOGI("  [%{public}zu] %{public}s ability '%{public}s' connect to '%{public}s'",
        index,
        action.GetTimeString().c_str(),
        action.GetName().c_str(),
        action.GetTargetName().c_str());
    // commit action
}

void ProcessOptimizerUBA::CommitDisconnectAbilityAction(const DisconnectAbilityAction& action, size_t index)
{
    APP_LOGI("  [%{public}zu] %{public}s '%{public}s' ability disconnect with '%{public}s'",
        index,
        action.GetTimeString().c_str(),
        action.GetName().c_str(),
        action.GetTargetName().c_str());
    // commit action
}

void ProcessOptimizerUBA::CommitChangedAbilityStateAction(const ChangeAbilityStateAction& action, size_t index)
{
    APP_LOGI("  [%{public}zu] %{public}s ability '%{public}s' state changed from %{public}d to %{public}d.",
        index,
        action.GetTimeString().c_str(),
        action.GetName().c_str(),
        action.GetOldState(),
        action.GetNewState());
    // commit action
}

void ProcessOptimizerUBA::CommitRemoveAbilityAction(const RemoveAbilityAction& action, size_t index)
{
    APP_LOGI("  [%{public}zu] %{public}s '%{public}s' removed.",
        index,
        action.GetTimeString().c_str(),
        action.GetName().c_str());
    // commit action
}

UbaServicePtr ProcessOptimizerUBA::GetUbaService()
{
    if (ubaService_) {
        return ubaService_;
    }

    // try to get uba service here.
    return nullptr;
}

void ProcessOptimizerUBA::SetAppFreezingTime(int time)
{
    auto ubaService = GetUbaService();
    if (ubaService) {
        APP_LOGI("ubaService implement.");
    } else {
        ProcessOptimizer::SetAppFreezingTime(time);
    }
}

void ProcessOptimizerUBA::GetAppFreezingTime(int &time)
{
    auto ubaService = GetUbaService();
    if (ubaService) {
        APP_LOGI("ubaService implement.");
    } else {
        ProcessOptimizer::GetAppFreezingTime(time);
    }
}
}  // namespace AppExecFwk
}  // namespace OHOS
