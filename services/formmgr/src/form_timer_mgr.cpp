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

#include "appexecfwk_errors.h"
#include "app_log_wrapper.h"
#include "common_event_manager.h"
#include "common_event_support.h"
#include "form_constants.h"
#include "form_provider_mgr.h"
#include "form_refresh_limiter.h"
#include "form_timer_mgr.h"
#include "form_util.h"
#include "want.h"

namespace OHOS {
namespace AppExecFwk {
FormTimerMgr::FormTimerMgr()
{
    Init();
}
FormTimerMgr::~FormTimerMgr()
{
    ClearIntervalTimer();
}
/**
 * @brief Add form timer by timer task.
 * @param task The form timer task.
 * @return Returns true on success, false on failure.
 */
bool FormTimerMgr::AddFormTimer(const FormTimer &task)
{
    //APP_LOGI("%{public}s, formId: %{public}lld", __func__, task.formId);
    if (task.isUpdateAt) {
        if (task.hour >= Constants::MIN_TIME && task.hour <= Constants::MAX_HOUR && task.min >= Constants::MIN_TIME && 
        task.min <= Constants::MAX_MININUTE) {
            return AddUpdateAtTimer(task);            
        } else {
            APP_LOGE("%{public}s failed, update at time is invalid", __func__);
            return false;
        }
    } else {
        if (task.period >= Constants::MIN_PERIOD && task.period <= Constants::MAX_PERIOD &&
         (task.period % Constants::MIN_PERIOD) == 0) {
            return AddIntervalTimer(task);
        } else {
            APP_LOGE("%{public}s failed, interval time is invalid", __func__);
            return false;            
        }        
    }
}
/**
 * @brief Add duration form timer.
 * @param formId The Id of the form.
 * @param updateDuration Update duration
 * @return Returns true on success, false on failure.
 */
bool FormTimerMgr::AddFormTimer(const int64_t formId, const long updateDuration)
{
    FormTimer timerTask(formId, updateDuration);
    return AddFormTimer(timerTask);
}
/**
 * @brief Add scheduled form timer.
 * @param formId The Id of the form.
 * @param updateAtHour Hour
 * @param updateAtMin Min
 * @return Returns true on success, false on failure.
 */
bool FormTimerMgr::AddFormTimer(const int64_t formId, const long updateAtHour, const long updateAtMin)
{
    FormTimer timerTask(formId, updateAtHour, updateAtMin);
    return AddFormTimer(timerTask);
}
/**
 * @brief Remove form timer by form id.
 * @param formId The Id of the form.
 * @return Returns true on success, false on failure.
 */
bool FormTimerMgr::RemoveFormTimer(const int64_t formId)
{
    //APP_LOGI("%{public}s, task: %{public}lld", __func__, formId);

    if (!DeleteIntervalTimer(formId)) {
        DeleteUpdateAtTimer(formId);
    }
        
    DeleteDynamicItem(formId);
    refreshLimiter_.DeleteItem(formId);

    return true;
}
/**
 * @brief Update form timer.
 * @param formId The Id of the form.
 * @param type Timer type.
 * @param timerCfg Timer config.
 * @return Returns true on success, false on failure.
 */
bool FormTimerMgr::UpdateFormTimer(const int64_t formId, const UpdateType &type, const FormTimerCfg &timerCfg)
{
    if (!timerCfg.enableUpdate) {
        APP_LOGW("%{public}s, enableUpdate is false", __func__);
        return false;
    }

    switch (type) {
        case UpdateType::TYPE_INTERVAL_CHANGE: {
            return UpdateIntervalValue(formId, timerCfg);
        }
        case UpdateType::TYPE_ATTIME_CHANGE: {
            return UpdateAtTimerValue(formId, timerCfg);
        }
        case UpdateType::TYPE_INTERVAL_TO_ATTIME: {
            return IntervalToAtTimer(formId, timerCfg);
        }
        case UpdateType::TYPE_ATTIME_TO_INTERVAL: {
            return AtTimerToIntervalTimer(formId, timerCfg);
        }
        default: {
            APP_LOGE("%{public}s failed, invalid UpdateType", __func__);
            return false;
        }
    }
}
/**
 * @brief Update Interval timer task value.
 * @param formId The Id of the form.
 * @param timerCfg task value.
 * @return Returns true on success, false on failure.
 */
bool FormTimerMgr::UpdateIntervalValue(const int64_t formId, const FormTimerCfg &timerCfg)
{
    if (timerCfg.updateDuration < Constants::MIN_PERIOD || timerCfg.updateDuration > Constants::MAX_PERIOD
        || (timerCfg.updateDuration % Constants::MIN_PERIOD) != 0) {
        APP_LOGE("%{public}s failed, invalid param", __func__);
        return false;
    }

    std::lock_guard<std::mutex> lock(intervalMutex_);
    auto intervalTask = intervalTimerTasks_.find(formId);
    if (intervalTask != intervalTimerTasks_.end()) {
        intervalTask->second.period = timerCfg.updateDuration;
        return true;
    } else {
        APP_LOGE("%{public}s failed, the interval timer is not exist", __func__);
        return false;
    }
}
/**
 * @brief Update update at timer task value.
 * @param formId The Id of the form.
 * @param timerCfg task value.
 * @return Returns true on success, false on failure.
 */
bool FormTimerMgr::UpdateAtTimerValue(const int64_t formId, const FormTimerCfg &timerCfg)
{
    if (timerCfg.updateAtHour < Constants::MIN_TIME || timerCfg.updateAtHour > Constants::MAX_HOUR
        || timerCfg.updateAtMin < Constants::MIN_TIME || timerCfg.updateAtMin > Constants::MAX_MININUTE) {
        APP_LOGE("%{public}s failed, time is invalid", __func__);
        return false;
    }
    {
        std::lock_guard<std::mutex> lock(updateAtMutex_);
        std::list<UpdateAtItem>::iterator itItem;
        UpdateAtItem changedItem;
        for (itItem = updateAtTimerTasks_.begin(); itItem != updateAtTimerTasks_.end(); itItem++) {
            if (itItem->refreshTask.formId == formId) {
                changedItem = *itItem;
                updateAtTimerTasks_.erase(itItem);
                break;
            }
        }

        if (changedItem.refreshTask.formId == 0) {
            APP_LOGE("%{public}s failed, the update at timer is not exist", __func__);
            return false;
        }
        changedItem.refreshTask.hour = timerCfg.updateAtHour;
        changedItem.refreshTask.min = timerCfg.updateAtMin;
        changedItem.updateAtTime = changedItem.refreshTask.hour * Constants::MIN_PER_HOUR + changedItem.refreshTask.min;
        AddUpdateAtItem(changedItem);
    }

    UpdateAtTimerAlarm();
    return true;
}
/**
 * @brief Interval timer task to update at timer task.
 * @param formId The Id of the form.
 * @param timerCfg task value.
 * @return Returns true on success, false on failure.
 */
bool FormTimerMgr::IntervalToAtTimer(const int64_t formId, const FormTimerCfg &timerCfg)
{
    if (timerCfg.updateAtHour < Constants::MIN_TIME || timerCfg.updateAtHour > Constants::MAX_HOUR
        || timerCfg.updateAtMin < Constants::MIN_TIME || timerCfg.updateAtMin > Constants::MAX_MININUTE) {
        APP_LOGE("%{public}s failed, time is invalid", __func__);
        return false;
    }

    std::lock_guard<std::mutex> lock(intervalMutex_);
    FormTimer timerTask;
    auto intervalTask = intervalTimerTasks_.find(formId);     
    if (intervalTask != intervalTimerTasks_.end()) {
        timerTask = intervalTask->second;
        intervalTimerTasks_.erase(intervalTask);

        timerTask.isUpdateAt = true;
        timerTask.hour = timerCfg.updateAtHour;
        timerTask.min = timerCfg.updateAtMin;
        if (!AddUpdateAtTimer(timerTask)) {
            APP_LOGE("%{public}s, failed to add update at timer", __func__);
            return false;
        }
        return true;
    } else {
        APP_LOGE("%{public}s failed, the interval timer is not exist", __func__);
        return false;
    }
}
/**
 * @brief Update at timer task to interval timer task.
 * @param formId The Id of the form.
 * @param timerCfg task value.
 * @return Returns true on success, false on failure.
 */
bool FormTimerMgr::AtTimerToIntervalTimer(const int64_t formId, const FormTimerCfg &timerCfg)
{
    if (timerCfg.updateDuration < Constants::MIN_PERIOD || timerCfg.updateDuration > Constants::MAX_PERIOD
        || (timerCfg.updateDuration % Constants::MIN_PERIOD) != 0) {
        APP_LOGE("%{public}s failed, time is invalid", __func__);
        return false;
    }

    UpdateAtItem targetItem;
    {
        std::lock_guard<std::mutex> lock(updateAtMutex_);
        std::list<UpdateAtItem>::iterator itItem;
        for (itItem = updateAtTimerTasks_.begin(); itItem != updateAtTimerTasks_.end(); itItem++) {
            if (itItem->refreshTask.formId == formId) {
                targetItem = *itItem;
                updateAtTimerTasks_.erase(itItem);
                break;
            }
        }
    }

    UpdateAtTimerAlarm();

    if (targetItem.refreshTask.formId == 0) {
        APP_LOGE("%{public}s failed, the update at timer is not exist", __func__);
        return false;
    }
    targetItem.refreshTask.isUpdateAt = false;
    targetItem.refreshTask.period = timerCfg.updateDuration;
    targetItem.refreshTask.refreshTime = LONG_MAX;
    if (!AddIntervalTimer(targetItem.refreshTask)) {
        APP_LOGE("%{public}s, failed to add interval timer", __func__);
        return false;
    }
    return true;
}
/**
 * @brief Is limiter enable refresh.
 * @param formId The Id of the form.
 * @return Returns true on success, false on failure.
 */
bool FormTimerMgr::IsLimiterEnableRefresh(const int64_t formId)
{
    return refreshLimiter_.IsEnableRefresh(formId);
}
/**
 * @brief Increase refresh count.
 * @param formId The Id of the form.
 */
void FormTimerMgr::IncreaseRefreshCount(const int64_t formId)
{
    refreshLimiter_.Increase(formId);
}
/**
 * @brief Set next refresh time.
 * @param formId The Id of the form.
 * @param nextGapTime Next gap time.
 * @return Returns true on success, false on failure.
 */
bool FormTimerMgr::SetNextRefreshTime(const int64_t formId, const long nextGapTime)
{
    if (nextGapTime < Constants::MIN_NEXT_TIME) {
        APP_LOGE("%{public}s failed, nextGapTime is invalid, nextGapTime:%{public}ld", __func__, nextGapTime);
        return false;
    }
    auto timeSinceEpoch = std::chrono::steady_clock::now().time_since_epoch();
    auto timeInSec = std::chrono::duration_cast<std::chrono::milliseconds>(timeSinceEpoch).count();
    int64_t refreshTime = timeInSec + nextGapTime * Constants::MS_PER_SECOND;
    std::lock_guard<std::mutex> lock(refreshMutex_);
    bool isExist = false;
    for (auto &refreshItem: dynamicRefreshTasks_) {
        if (refreshItem.formId == formId) {  
            refreshItem.settedTime = refreshTime;
            isExist = true;
            break;
        }
    }
    if (!isExist) {
        DynamicRefreshItem theItem;
        theItem.formId = formId;
        theItem.settedTime = refreshTime;
        dynamicRefreshTasks_.emplace_back(theItem);
    }
    std::sort(dynamicRefreshTasks_.begin(), dynamicRefreshTasks_.end(), CompareDynamicRefreshItem);
    if (!UpdateDynamicAlarm()) {
        APP_LOGE("%{public}s, failed to UpdateDynamicAlarm", __func__);
        return false;
    }
    refreshLimiter_.AddItem(formId);
    SetEnableFlag(formId, false);
    
    return true;
}

void FormTimerMgr::SetEnableFlag(int64_t formId, bool flag) 
{
    // try interval list
    auto iter = intervalTimerTasks_.find(formId); 
    if (iter != intervalTimerTasks_.end()) {
        iter->second.isEnable = flag;
        //APP_LOGI("%{public}s, formId:%{public}lld, isEnable:%{public}d", __func__, formId, flag ? 1 : 0);
        return;
    }
}

/**
 * @brief Get refresh count.
 * @param formId The Id of the form.
 * @return Returns refresh count.
 */
int FormTimerMgr::GetRefreshCount(const int64_t formId) const
{
    return refreshLimiter_.GetRefreshCount(formId);
}
/**
 * @brief Mark remind.
 * @param formId The Id of the form.
 * @return true or false.
 */
void FormTimerMgr::MarkRemind(const int64_t  formId)
{
    refreshLimiter_.MarkRemind(formId);
}
/**
 * @brief Add update at timer.
 * @param task Update time task.
 * @return Returns true on success, false on failure.
 */  
bool FormTimerMgr::AddUpdateAtTimer(const FormTimer &task)
{
    APP_LOGI("%{public}s start", __func__);
    {
        std::lock_guard<std::mutex> lock(updateAtMutex_);
        for (auto &updateAtTimer : updateAtTimerTasks_) {
            if (updateAtTimer.refreshTask.formId == task.formId) {
                //APP_LOGW("%{public}s, already exist formTimer, formId:%{public}lld task", __func__, task.formId);
                return true;
            }
        }

        UpdateAtItem atItem;
        atItem.refreshTask = task;
        atItem.updateAtTime = task.hour * Constants::MIN_PER_HOUR + task.min;

        AddUpdateAtItem(atItem);
    }

    if (!UpdateAtTimerAlarm()) {
        APP_LOGE("%{public}s, failed to update alarm.", __func__);
        return false;
    }

    return refreshLimiter_.AddItem(task.formId);
}
/**
 * @brief Add update interval timer task.
 * @param task Update interval timer task.
 * @return Returns true on success, false on failure.
 */ 
bool FormTimerMgr::AddIntervalTimer(const FormTimer &task)
{
    APP_LOGI("%{public}s start", __func__);
    {
        std::lock_guard<std::mutex> lock(intervalMutex_);
        EnsureInitIntervalTimer();
        if (intervalTimerTasks_.find(task.formId) != intervalTimerTasks_.end()) {
            //APP_LOGW("%{public}s, already exist formTimer, formId:%{public}lld task", __func__, task.formId);
            return true;
        }
        intervalTimerTasks_.emplace(task.formId, task);
    }
    UpdateLimiterAlarm();
    return refreshLimiter_.AddItem(task.formId);
}
/**
 * @brief Add update at timer item.
 * @param task Update at timer item.
 */ 
void FormTimerMgr::AddUpdateAtItem(const UpdateAtItem &atItem)
{
    if (updateAtTimerTasks_.empty()) {
        updateAtTimerTasks_.emplace_back(atItem);
        return;
    }

    UpdateAtItem firstItem = updateAtTimerTasks_.front();
    if (atItem.updateAtTime < firstItem.updateAtTime) {
        updateAtTimerTasks_.emplace_front(atItem);
        return;
    }

    bool isInsert = false;
    std::list<UpdateAtItem>::iterator itItem;
    for (itItem = updateAtTimerTasks_.begin(); itItem != updateAtTimerTasks_.end(); itItem++) {
        if (atItem.updateAtTime < itItem->updateAtTime) {
            updateAtTimerTasks_.insert(itItem, atItem);
            isInsert = true;
            break;
        }
    }

    if (!isInsert) {
        updateAtTimerTasks_.emplace_back(atItem);
    }
}
/**
 * @brief Handle system time changed.
 */
void FormTimerMgr::HandleSystemTimeChanged()
{
    APP_LOGI("%{public}s start", __func__);
    if (!updateAtTimerTasks_.empty()) {
        UpdateAtTimerAlarm();
    }
    APP_LOGI("%{public}s end", __func__);
}
/**
 * @brief Reset form limiter.
 */
void FormTimerMgr::HandleResetLimiter()
{
    APP_LOGI("%{public}s start", __func__);

    std::vector<FormTimer> remindTasks;
    bool bGetTasks = GetRemindTasks(remindTasks);
    if (bGetTasks) {
        APP_LOGI("%{public}s failed, remind when reset limiter", __func__);
        for (auto &task : remindTasks) {
            ExecTimerTask(task);
        }
    }
    APP_LOGI("%{public}s end", __func__);
}
/**
 * @brief Update attime trigger.
 * @param updateTime Update time.
 */
void FormTimerMgr::OnUpdateAtTrigger(long updateTime)
{
    APP_LOGI("%{public}s start, updateTime:%{public}ld", __func__, updateTime);
    std::vector<UpdateAtItem> updateList;
    {
        std::lock_guard<std::mutex> lock(updateAtMutex_);        
        std::list<UpdateAtItem>::iterator itItem;
        for (itItem = updateAtTimerTasks_.begin(); itItem != updateAtTimerTasks_.end(); itItem++) {
            if (itItem->updateAtTime == updateTime && itItem->refreshTask.isEnable) {
                updateList.emplace_back(*itItem);
            }
        }
    }

    UpdateAtTimerAlarm();

    if (!updateList.empty()) {
        APP_LOGI("%{public}s, update at timer triggered, trigged time: %{public}ld", __func__, updateTime);
        for (auto &item : updateList) {
            ExecTimerTask(item.refreshTask);
        }
    }
    APP_LOGI("%{public}s end", __func__);
}
/**
 * @brief Dynamic time trigger.
 * @param updateTime Update time.
 */
void FormTimerMgr::OnDynamicTimeTrigger(long updateTime)
{
    APP_LOGI("%{public}s start, updateTime:%{public}ld", __func__, updateTime);
    std::vector<FormTimer> updateList;
    {
        std::lock_guard<std::mutex> lock(dynamicMutex_);
        auto timeSinceEpoch = std::chrono::steady_clock::now().time_since_epoch();
        auto timeInSec = std::chrono::duration_cast<std::chrono::milliseconds>(timeSinceEpoch).count();
        long markedTime = timeInSec + Constants::ABS_REFRESH_MS;        
        std::vector<DynamicRefreshItem>::iterator itItem;
        for (itItem = dynamicRefreshTasks_.begin(); itItem != dynamicRefreshTasks_.end();) {
            if (itItem->settedTime <= updateTime || itItem->settedTime <= markedTime) {
                if (refreshLimiter_.IsEnableRefresh(itItem->formId)) {
                    FormTimer timerTask(itItem->formId, true);
                    updateList.emplace_back(timerTask);
                }
                itItem = dynamicRefreshTasks_.erase(itItem);
                SetIntervalEnableFlag(itItem->formId, true);
            } else {
                itItem++;
            }            
        }
        std::sort(dynamicRefreshTasks_.begin(), dynamicRefreshTasks_.end(),  CompareDynamicRefreshItem);
    }

    UpdateDynamicAlarm();
    if (!updateList.empty()) {
        APP_LOGI("%{public}s triggered, trigged time: %{public}ld", __func__, updateTime);
        for (auto &task : updateList) {
            ExecTimerTask(task);
        }
    }
    APP_LOGI("%{public}s end", __func__);
}
/**
 * @brief Get remind tasks.
 * @param remindTasks Remind tasks.
 * @return Returns true on success, false on failure.
 */
bool FormTimerMgr::GetRemindTasks(std::vector<FormTimer> &remindTasks)
{
    APP_LOGI("%{public}s start", __func__);
    std::vector<int64_t> remindList = refreshLimiter_.GetRemindListAndResetLimit();
    for (int64_t id : remindList) {
        FormTimer formTimer(id, false);
        remindTasks.emplace_back(formTimer);
    }

    UpdateLimiterAlarm();

    if(remindTasks.size() > 0) {
        return true;
    } else {
        return false;
    }
    APP_LOGI("%{public}s end", __func__);
}
/**
 * @brief Set enableFlag for interval timer task.
 * @param formId The Id of the form.
 * @param flag Enable flag.
 */
void FormTimerMgr::SetIntervalEnableFlag(int64_t formId, bool flag)
{
    std::lock_guard<std::mutex> lock(intervalMutex_);
    // try interval list
    auto refreshTask = intervalTimerTasks_.find(formId);
    if (refreshTask != intervalTimerTasks_.end()) {
        refreshTask->second.isEnable = flag;
        //APP_LOGI("%{public}s, formId:%{public}lld, isEnable:%{public}d", __func__, formId, flag ? 1 : 0);
        return;
    }
}
/**
 * @brief Delete interval timer task.
 * @param formId The Id of the form.
 * @return Returns true on success, false on failure.
 */
bool FormTimerMgr::DeleteIntervalTimer(const int64_t formId)
{
    APP_LOGI("%{public}s start", __func__);
    bool isExist = false;
    std::lock_guard<std::mutex> lock(intervalMutex_);
    auto intervalTask = intervalTimerTasks_.find(formId);
    if (intervalTask != intervalTimerTasks_.end()) {
        intervalTimerTasks_.erase(intervalTask);
        isExist = true;
    }

    if (intervalTimerTasks_.empty()) {
        ClearIntervalTimer();
    }
    APP_LOGI("%{public}s end", __func__);
    return isExist;
}
/**
 * @brief Delete update at timer.
 * @param formId The Id of the form.
 */
void FormTimerMgr::DeleteUpdateAtTimer(const int64_t formId)
{
    APP_LOGI("%{public}s start", __func__);
    {
        std::lock_guard<std::mutex> lock(updateAtMutex_);        
        std::list<UpdateAtItem>::iterator itItem;
        for (itItem = updateAtTimerTasks_.begin(); itItem != updateAtTimerTasks_.end(); itItem++) {
            if (itItem->refreshTask.formId == formId) {
                updateAtTimerTasks_.erase(itItem);
                break;
            }
        }
    }

    UpdateAtTimerAlarm();

    APP_LOGI("%{public}s end", __func__);
}
/**
 * @brief Delete dynamic refresh item.
 * @param formId The Id of the form.
 */ 
void FormTimerMgr::DeleteDynamicItem(const int64_t formId)
{
    APP_LOGI("%{public}s start", __func__);
    std::lock_guard<std::mutex> lock(dynamicMutex_);       
    std::vector<DynamicRefreshItem>::iterator itItem;
    for (itItem = dynamicRefreshTasks_.begin(); itItem != dynamicRefreshTasks_.end();) {
        if (itItem->formId == formId) {
            dynamicRefreshTasks_.erase(itItem);
            SetIntervalEnableFlag(itItem->formId, true);
            break;
        }        
    }
    std::sort(dynamicRefreshTasks_.begin(), dynamicRefreshTasks_.end(),  CompareDynamicRefreshItem);
    
    UpdateDynamicAlarm();
    APP_LOGI("%{public}s end", __func__);
}
/**
* @brief interval timer task timeout.
*/ 
void FormTimerMgr::OnIntervalTimeOut()
{
    APP_LOGI("%{public}s start", __func__);
    std::lock_guard<std::mutex> lock(intervalMutex_);
    std::vector<FormTimer> updateList;
    long currentTime = FormUtil::GetCurrentNanosecond() / Constants::TIME_1000000;
    for (auto &intervalPair : intervalTimerTasks_) {
        FormTimer &intervalTask = intervalPair.second;
        if ((intervalTask.refreshTime == LONG_MAX || (currentTime - intervalTask.refreshTime) >= intervalTask.period ||
            std::abs((currentTime - intervalTask.refreshTime) - intervalTask.period) < Constants::ABS_TIME) &&
            intervalTask.isEnable && refreshLimiter_.IsEnableRefresh(intervalTask.formId)) {
            intervalTask.refreshTime = currentTime;
            updateList.emplace_back(intervalTask);
        }
    }  

    if (!updateList.empty()) {
        for (auto &task : updateList) {
            ExecTimerTask(task);
        }
    }
    APP_LOGI("%{public}s end", __func__);
}
/**
 * @brief Update at timer task alarm.
 * @return Returns true on success, false on failure.
 */ 
bool FormTimerMgr::UpdateAtTimerAlarm()
{
    APP_LOGI("%{public}s start", __func__);
    // AlarmManager* alarm = GetAlarmManagerLocked();
    // if (alarm == nullptr) {
    //     APP_LOGE("%{public}s failed, failed to get alarm manager, can not updateAlarm", __func__);
    //     return false;
    // }
    struct tm tmAtTime = {0};
    auto tt = std::chrono::system_clock::to_time_t(std::chrono::system_clock::now());
    struct tm* ptm = localtime_r(&tt, &tmAtTime);
    if (ptm == nullptr) {
        APP_LOGE("%{public}s failed, localtime error", __func__);
        return false;
    }

    int nowAtTime = tmAtTime.tm_hour * Constants::MIN_PER_HOUR + tmAtTime.tm_min;
    long currentTime = FormUtil::GetCurrentMillisecond();
    UpdateAtItem findedItem;
    bool bFinded = FindNextAtTimerItem(nowAtTime, findedItem);
    if (!bFinded) {    
        ClearUpdateAtTimerResource();    
        APP_LOGI("%{public}s, no update at task in system now.", __func__);
        return true;
    }

    int nextWakeUpTime = findedItem.updateAtTime;
    tmAtTime.tm_sec = 0;
    tmAtTime.tm_hour = findedItem.refreshTask.hour;
    tmAtTime.tm_min = findedItem.refreshTask.min;
    long selectTime = FormUtil::GetMillisecondFromTm(tmAtTime);    
    if (selectTime < currentTime) {
        tmAtTime.tm_mday += 1;
        nextWakeUpTime += (Constants::HOUR_PER_DAY * Constants::MIN_PER_HOUR);
    }

    if (nextWakeUpTime == atTimerWakeUpTime_) {
        APP_LOGW("%{public}s end, wakeUpTime not change, no need update alarm.", __func__);
        return true;
    }

    // PendingIntent pendingIntent = getPendingIntent(findedItem.updateAtTime);
    // if (pendingIntent == null) {
    //     HiLog.error(LABEL_LOG, "create pendingIntent failed.");
    //     return false;
    // }
    atTimerWakeUpTime_ = nextWakeUpTime;
    // if (currentPendingIntent != null) {
    //     alarm.cancel(currentPendingIntent);
    // }
    // currentPendingIntent = pendingIntent;
    // alarm.setExact(AlarmManager.RTC_WAKEUP, FormUtil::GetMillisecondFromTm(tmAtTime), pendingIntent);
    APP_LOGI("%{public}s end", __func__);
    return true;
}
/**
 * @brief Update limiter task alarm.
 * @return Returns true on success, false on failure.
 */ 
bool FormTimerMgr::UpdateLimiterAlarm()
{
    APP_LOGI("%{public}s start", __func__);
    // AlarmManager alarm = getAlarmManagerLocked();
    // if (alarm == null) {
    //     HiLog.error(LABEL_LOG, "faied to get alarm manager, can not updateLimiterAlarm");
    //     return false;
    // }

    // PendingIntent pendingIntent = getLimiterPendingIntent();
    // if (pendingIntent == null) {
    //     HiLog.error(LABEL_LOG, "create limiterPendingIntent failed.");
    //     return false;
    // }

    // if (limiterPendingIntent != null) {
    //     alarm.cancel(limiterPendingIntent);
    // }
    // limiterPendingIntent = pendingIntent;

    // Calendar calendar = Calendar.getInstance();
    // calendar.set(Calendar.DATE, Calendar.getInstance().get(Calendar.DATE));
    // calendar.set(Calendar.HOUR_OF_DAY, MAX_HOUR);
    // calendar.set(Calendar.MINUTE, MAX_MININUTE);
    // calendar.set(Calendar.SECOND, MAX_SECOND);
    // calendar.set(Calendar.MILLISECOND, LIMIT_MS);
    // alarm.setExactAndAllowWhileIdle(AlarmManager.RTC_WAKEUP, calendar.getTimeInMillis(), pendingIntent);
    APP_LOGI("%{public}s end", __func__);
    return true;
}
/**
 * @brief Update dynamic refresh task alarm.
 * @return Returns true on success, false on failure.
 */ 
bool FormTimerMgr::UpdateDynamicAlarm()
{
    APP_LOGI("%{public}s start", __func__);
    if (dynamicRefreshTasks_.empty()) {
        ClearDynamicResource();
        return true;
    }

    bool needUpdate = false;
    DynamicRefreshItem firstTask = dynamicRefreshTasks_.at(0);
    if (dynamicWakeUpTime_ != firstTask.settedTime) {
        dynamicWakeUpTime_ = firstTask.settedTime;
        needUpdate = true;
    }

    if (!needUpdate) {
        APP_LOGE("%{public}s failed, no need to  UpdateDynamicAlarm.", __func__);
        return true;
    }

    // AlarmManager alarm = getAlarmManagerLocked();
    // if (alarm == null) {
    //     HiLog.error(LABEL_LOG, "faied to get alarm manager, can not UpdateDynamicAlarm.");
    //     return false;
    // }

    // PendingIntent pendingIntent = getDynamicPendingIntent(dynamicWakeUpTime);
    // if (pendingIntent == null) {
    //     HiLog.error(LABEL_LOG, "create dynamic pendingIntent failed.", __func__);
    //     return false;
    // }
    // if (dynamicPendingIntent != null) {
    //     alarm.cancel(dynamicPendingIntent);
    // }
    // dynamicPendingIntent = pendingIntent;
    // alarm.setExactAndAllowWhileIdle(AlarmManager.ELAPSED_REALTIME_WAKEUP, dynamicWakeUpTime, pendingIntent);
    APP_LOGI("%{public}s end", __func__);

    return true;
}
/**
 * @brief Clear dynamic refresh resource.
 */ 
void FormTimerMgr::ClearDynamicResource()
{
    APP_LOGI("%{public}s start", __func__);
    // AlarmManager alarm = getAlarmManagerLocked();
    // if (alarm == null) {
    //     return;
    // }

    // if (dynamicPendingIntent != null) {
    //     alarm.cancel(dynamicPendingIntent);
    //     dynamicPendingIntent = null;
    // }
    dynamicWakeUpTime_ = LONG_MAX;
    APP_LOGI("%{public}s end", __func__);
}
/**
 * @brief Fint next at timer item.
 * @param nowTime Update time.
 * @param updateAtItem Next at timer item.
 * @return Returns true on success, false on failure.
 */
bool FormTimerMgr::FindNextAtTimerItem(const int nowTime, UpdateAtItem &updateAtItem)
{
    APP_LOGI("%{public}s start", __func__);
    if (updateAtTimerTasks_.empty()) {
        APP_LOGW("%{public}s, updateAtTimerTasks_ is empty", __func__);
        return false;
    }

    std::lock_guard<std::mutex> lock(updateAtMutex_);        
    std::list<UpdateAtItem>::iterator itItem;
    for (itItem = updateAtTimerTasks_.begin(); itItem != updateAtTimerTasks_.end(); itItem++) {
        if (itItem->updateAtTime > nowTime) {
            updateAtItem = *itItem;
            break;
        }
    }

    if (itItem == updateAtTimerTasks_.end()) {
        updateAtItem = updateAtTimerTasks_.front();
    }
    APP_LOGI("%{public}s end", __func__);
    return true;
}
/**
 * @brief Clear update at timer resource.
 */ 
void FormTimerMgr::ClearUpdateAtTimerResource()
{
    APP_LOGI("%{public}s start", __func__);
    if (!updateAtTimerTasks_.empty()) {
        APP_LOGW("%{public}s, updateAtTimerTasks_ is not empty", __func__);
        return;
    }

    // AlarmManager alarm = getAlarmManagerLocked();
    // if (alarm == null) {
    //     return;
    // }

    // if (currentPendingIntent != null) {
    //     alarm.cancel(currentPendingIntent);
    //     currentPendingIntent = null;
    // }
    atTimerWakeUpTime_ = LONG_MAX;

    APP_LOGI("%{public}s end", __func__);
}
/**
 * @brief Ensure init interval timer resource.
 */
void FormTimerMgr::EnsureInitIntervalTimer()
{
    if (intervalTimer_ != NULL) {
        return;
    }

    APP_LOGI("%{public}s, init base timer task", __func__);
    intervalTimer_ = new Utils::Timer("interval timer");
    uint32_t iResult = intervalTimer_->Setup();
    if (iResult != ERR_OK) {
        APP_LOGE("%{public}s failed, init base timer task error", __func__);
        return;
    }
    auto timeCallback = std::bind(&FormTimerMgr::OnIntervalTimeOut, this);
    intervalTimer_->Register(timeCallback, Constants::MIN_PERIOD);
    
    APP_LOGI("%{public}s end", __func__);
}
/**
 * @brief Clear interval timer resource.
 */
void FormTimerMgr::ClearIntervalTimer()
{
    APP_LOGI("%{public}s start", __func__);
    if (intervalTimer_ != nullptr) {
        APP_LOGI("%{public}s clear interval timer start", __func__);
        intervalTimer_->Shutdown();
        delete intervalTimer_;
        intervalTimer_ = nullptr;
        APP_LOGI("%{public}s clear interval timer end", __func__);
    }
    APP_LOGI("%{public}s end", __func__);
}
/**
 * @brief Get thread pool for timer task.
 */
OHOS::ThreadPool* FormTimerMgr::GetTaskThreadExecutor()
{
    APP_LOGI("%{public}s start", __func__);
    if (taskExecutor_ == nullptr) {
        taskExecutor_ = new OHOS::ThreadPool("timer task thread");
        taskExecutor_->Start(Constants::WORK_POOL_SIZE);
    }
    APP_LOGI("%{public}s end", __func__);
    return taskExecutor_;
}

/**
 * @brief Execute Form timer task.
 * @param timerTask Form timer task.
 */ 
void FormTimerMgr::ExecTimerTask(const FormTimer &timerTask)
{
    APP_LOGI("%{public}s start", __func__);
    OHOS::ThreadPool* Executor = GetTaskThreadExecutor();
    if (Executor != nullptr) {
        APP_LOGI("%{public}s run", __func__);
        AAFwk::Want want;
        if (timerTask.isCountTimer) {
            want.SetParam(Constants::KEY_IS_TIMER, true);
        }
        auto task = std::bind(&FormProviderMgr::RefreshForm, &FormProviderMgr::GetInstance(), timerTask.formId, want);
        Executor->AddTask(task);
    }
    APP_LOGI("%{public}s end", __func__);
}

/**
 * @brief Init.
 */
void FormTimerMgr::Init()
{
    APP_LOGI("%{public}s start", __func__);
    timerReceiver_ = nullptr;
    EventFwk::MatchingSkills matchingSkills;
    matchingSkills.AddEvent(Constants::ACTION_UPDATEATTIMER);
    matchingSkills.AddEvent(EventFwk::CommonEventSupport::COMMON_EVENT_TIME_CHANGED);
    matchingSkills.AddEvent(EventFwk::CommonEventSupport::COMMON_EVENT_TIMEZONE_CHANGED);
    
    // init TimerReceiver
    EventFwk::CommonEventSubscribeInfo subscribeInfo(matchingSkills);
    // subscribeInfo.SetPermission(permissin);
    timerReceiver_ = std::make_shared<TimerReceiver>(subscribeInfo);
    EventFwk::CommonEventManager::SubscribeCommonEvent(timerReceiver_);

    intervalTimer_ = nullptr;
    taskExecutor_ = nullptr;

    APP_LOGI("%{public}s end", __func__);
}

/**
 * @brief Receiver Constructor.
 * @param subscriberInfo Subscriber info.
 */
FormTimerMgr::TimerReceiver::TimerReceiver(const EventFwk::CommonEventSubscribeInfo &subscriberInfo) 
    : EventFwk::CommonEventSubscriber(subscriberInfo)
{}
/**
 * @brief Receive common event.
 * @param eventData Common event data.
 */
void FormTimerMgr::TimerReceiver::OnReceiveEvent(const EventFwk::CommonEventData &eventData)
{
    AAFwk::Want want = eventData.GetWant();
    std::string action = want.GetAction();

    APP_LOGI("%{public}s, action:%{public}s.", __func__, action.c_str());

    if (action == EventFwk::CommonEventSupport::COMMON_EVENT_TIME_CHANGED
        || action == EventFwk::CommonEventSupport::COMMON_EVENT_TIMEZONE_CHANGED) {        
        FormTimerMgr::GetInstance().HandleSystemTimeChanged();
    } else if (action == Constants::ACTION_UPDATEATTIMER) {
        int type = want.GetIntParam(Constants::KEY_ACTION_TYPE, Constants::TYPE_STATIC_UPDATE);
        if (type == Constants::TYPE_RESET_LIMIT) {
            FormTimerMgr::GetInstance().HandleResetLimiter();
        } else if (type == Constants::TYPE_STATIC_UPDATE) {
            long updateTime = want.GetLongParam(Constants::KEY_WAKEUP_TIME, -1);
            if (updateTime < 0) {
                APP_LOGE("%{public}s failed, invalid updateTime:%{public}ld.", __func__, updateTime);
                return;
            }
            FormTimerMgr::GetInstance().OnUpdateAtTrigger(updateTime);
        } else if (type == Constants::TYPE_DYNAMIC_UPDATE) {
            long updateTime = want.GetLongParam(Constants::KEY_WAKEUP_TIME, 0);
            if (updateTime <= 0) {
                APP_LOGE("%{public}s failed, invalid updateTime:%{public}ld.", __func__, updateTime);
                return;
            }
            FormTimerMgr::GetInstance().OnDynamicTimeTrigger(updateTime);
        } else {
            APP_LOGE("%{public}s failed, invalid type when action is update at timer.", __func__);
        }
    } else {
        APP_LOGE("%{public}s failed, invalid action.", __func__);
    }
}
}  // namespace AppExecFwk
}  // namespace OHOS
