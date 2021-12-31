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

#include "ability_manager.h"
#include "app_log_wrapper.h"
#include "singleton.h"
#include "sys_mgr_client.h"
#include "system_ability_definition.h"

namespace OHOS {
namespace AppExecFwk {
AbilityManager &AbilityManager::GetInstance()
{
    static AbilityManager abilityManager;
    return abilityManager;
}

void AbilityManager::StartAbility(const Want &want, int requestCode = -1)
{
    APP_LOGD("%s, %d", __func__, __LINE__);
    ErrCode error = AAFwk::AbilityManagerClient::GetInstance()->StartAbility(want, requestCode);
    if (error != ERR_OK) {
        APP_LOGE("%s failed, error : %d", __func__, error);
    }
}

void AbilityManager::MoveMissionToTop(int missionId)
{
    APP_LOGD("%s, %d", __func__, __LINE__);
    ErrCode error = AAFwk::AbilityManagerClient::GetInstance()->MoveMissionToTop(missionId);
    if (error != ERR_OK) {
        APP_LOGE("%s failed, error : %d", __func__, error);
    }
}

StackInfo AbilityManager::GetAllStackInfo() const
{
    APP_LOGD("%s, %d", __func__, __LINE__);
    StackInfo info;
    ErrCode error = AAFwk::AbilityManagerClient::GetInstance()->GetAllStackInfo(info);
    if (error != ERR_OK) {
        APP_LOGE("%s failed, error : %d", __func__, error);
    }

    return info;
}

std::vector<AbilityMissionInfo> AbilityManager::QueryRecentAbilityMissionInfo(int numMax, int flags) const
{
    APP_LOGD("%s, %d", __func__, __LINE__);
    std::vector<AbilityMissionInfo> info;
    ErrCode error = AAFwk::AbilityManagerClient::GetInstance()->GetRecentMissions(numMax, flags, info);
    if (error != ERR_OK) {
        APP_LOGE("%s failed, error : %d", __func__, error);
    }

    return info;
}

std::vector<AbilityMissionInfo> AbilityManager::QueryRunningAbilityMissionInfo(int numMax) const
{
    APP_LOGD("%s, %d", __func__, __LINE__);
    std::vector<AbilityMissionInfo> info;
    ErrCode error =
        AAFwk::AbilityManagerClient::GetInstance()->GetRecentMissions(numMax, RECENT_IGNORE_UNAVAILABLE, info);
    if (error != ERR_OK) {
        APP_LOGE("%s failed, error : %d", __func__, error);
    }

    return info;
}

void AbilityManager::RemoveMissions(const std::vector<int> &missionId)
{
    APP_LOGD("%s, %d", __func__, __LINE__);
    ErrCode error = AAFwk::AbilityManagerClient::GetInstance()->RemoveMissions(missionId);
    if (error != ERR_OK) {
        APP_LOGE("%s failed, error : %d", __func__, error);
    }
}

int32_t AbilityManager::ClearUpApplicationData(const std::string &bundleName)
{
    APP_LOGD("%s, %d", __func__, __LINE__);
    auto object = OHOS::DelayedSingleton<SysMrgClient>::GetInstance()->GetSystemAbility(APP_MGR_SERVICE_ID);
    sptr<IAppMgr> appMgr_ = iface_cast<IAppMgr>(object);
    if (appMgr_ == nullptr) {
        APP_LOGE("%s, appMgr_ is nullptr", __func__);
        return ERR_NULL_OBJECT;
    }

    return appMgr_->ClearUpApplicationData(bundleName);
}

std::vector<RunningProcessInfo> AbilityManager::GetAllRunningProcesses()
{
    APP_LOGD("%s, %d", __func__, __LINE__);
    auto object = OHOS::DelayedSingleton<SysMrgClient>::GetInstance()->GetSystemAbility(APP_MGR_SERVICE_ID);
    sptr<IAppMgr> appMgr_ = iface_cast<IAppMgr>(object);
    std::vector<RunningProcessInfo> info;
    if (appMgr_ == nullptr) {
        APP_LOGE("%s, appMgr_ is nullptr", __func__);
        return info;
    }

    appMgr_->GetAllRunningProcesses(info);
    return info;
}

int AbilityManager::KillProcessesByBundleName(const std::string &bundleName)
{
    APP_LOGD("%s, %d", __func__, __LINE__);
    ErrCode error = AAFwk::AbilityManagerClient::GetInstance()->KillProcess(bundleName);
    if (error != ERR_OK) {
        APP_LOGE("%s failed, error : %d", __func__, error);
        return error;
    }
    return ERR_OK;
}
}  // namespace AppExecFwk
}  // namespace OHOS