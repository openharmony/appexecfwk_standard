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

#ifndef FOUNDATION_APPEXECFWK_SERVICES_FORMMGR_INCLUDE_FORM_TIMER_MGR_H
#define FOUNDATION_APPEXECFWK_SERVICES_FORMMGR_INCLUDE_FORM_TIMER_MGR_H

#include <array>
#include <chrono>
#include <ctime>
#include <limits.h>
#include <list>
#include <map>
#include <mutex>
#include <singleton.h>
#include <stdint.h>
#include <string>
#include <time.h>
#include <vector>

#include "common_event_subscriber.h"
#include "common_event_subscribe_info.h"
#include "form_refresh_limiter.h"
#include "form_timer.h"
#include "thread_pool.h"
#include "time_service_client.h"
// #include "timer.h"

namespace OHOS {
namespace AppExecFwk {
using namespace OHOS::Notification::WantAgent;
/**
 * @class FormTimerMgr
 * form timer task manager.
 */
class FormTimerMgr final : public DelayedRefSingleton<FormTimerMgr> {
DECLARE_DELAYED_REF_SINGLETON(FormTimerMgr)
public:
    DISALLOW_COPY_AND_MOVE(FormTimerMgr);
    /**
     * @brief Add form timer by timer task.
     * @param task The form timer task.
     * @return Returns true on success, false on failure.
     */
    bool AddFormTimer(const FormTimer &task);
    /**
     * @brief Add duration form timer.
     * @param formId The Id of the form.
     * @param updateDuration Update duration
     * @return Returns true on success, false on failure.
     */
    bool AddFormTimer(const int64_t formId, const long updateDuration);
    /**
     * @brief Add scheduled form timer.
     * @param formId The Id of the form.
     * @param updateAtHour Hour
     * @param updateAtMin Min
     * @return Returns true on success, false on failure.
     */
    bool AddFormTimer(const int64_t formId, const long updateAtHour, const long updateAtMin);
    /**
     * @brief Remove form timer by form id.
     * @param formId The Id of the form.
     * @return Returns true on success, false on failure.
     */
    bool RemoveFormTimer(const int64_t formId);
    /**
     * @brief Update form timer.
     * @param formId The Id of the form.
     * @param type Timer type.
     * @param timerCfg Timer config.
     * @return Returns true on success, false on failure.
     */
    bool UpdateFormTimer(const int64_t formId, const UpdateType &type, const FormTimerCfg &timerCfg);
    /**
     * @brief Is limiter enable refresh.
     * @param formId The Id of the form.
     * @return Returns true on success, false on failure.
     */
    bool IsLimiterEnableRefresh(const int64_t formId);
    /**
     * @brief Increase refresh count.
     * @param formId The Id of the form.
     */
    void IncreaseRefreshCount(const int64_t formId);
    /**
     * @brief Set next refresh time.
     * @param formId The Id of the form.
     * @param nextGapTime Next gap time.
     * @return Returns true on success, false on failure.
     */
    bool SetNextRefreshTime(const int64_t formId, const long nextGapTime);
    /**
     * @brief Get refresh count.
     * @param formId The Id of the form.
     * @return Returns refresh count.
     */
    int GetRefreshCount(const int64_t formId) const;
    /**
     * @brief Mark remind.
     * @param formId The Id of the form.
     * @return true or false.
     */
    void MarkRemind(const int64_t  formId);

    /**
     * @brief Handle system time changed.
     * @return Returns true on success, false on failure.
     */
    bool HandleSystemTimeChanged();
    /**
     * @brief Reset form limiter.
     * @return Returns true on success, false on failure.
     */
    bool HandleResetLimiter();
    /**
     * @brief Update attime trigger.
     * @param updateTime Update time.
     * @return Returns true on success, false on failure.
     */
    bool OnUpdateAtTrigger(long updateTime);
    /**
     * @brief Dynamic time trigger.
     * @param updateTime Update time.
     * @return Returns true on success, false on failure.
     */
    bool OnDynamicTimeTrigger(long updateTime);
    /**
     * @brief Get interval timer task.
     * @param formId The Id of the form.
     * @return Returns true on success, false on failure.
     */
    bool GetIntervalTimer(const int64_t formId, FormTimer &formTimer);
    /**
     * @brief Get update at timer.
     * @param formId The Id of the form.
     * @return Returns true on success, false on failure.
     */
    bool GetUpdateAtTimer(const int64_t formId, UpdateAtItem &updateAtItem);
    /**
     * @brief Get dynamic refresh item.
     * @param formId The Id of the form.
     * @return Returns true on success, false on failure.
     */
    bool GetDynamicItem(const int64_t formId, DynamicRefreshItem &dynamicItem);
private:
    /**
     * @brief Add update at timer.
     * @param task Update time task.
     * @return Returns true on success, false on failure.
     */
    bool AddUpdateAtTimer(const FormTimer &task);
    /**
     * @brief Add update at timer item.
     * @param task Update at timer item.
     */
    void AddUpdateAtItem(const UpdateAtItem &atItem);
    /**
     * @brief Add update interval timer task.
     * @param task Update interval timer task.
     * @return Returns true on success, false on failure.
     */
    bool AddIntervalTimer(const FormTimer &task);
    /**
     * @brief interval timer task timeout.
     */
    void OnIntervalTimeOut();
    /**
     * @brief Get remind tasks.
     * @param remindTasks Remind tasks.
     * @return Returns true on success, false on failure.
     */
    bool GetRemindTasks(std::vector<FormTimer> &remindTasks);
    /**
     * @brief Set enableFlag for interval timer task.
     * @param formId The Id of the form.
     * @param flag Enable flag.
     */
    void SetIntervalEnableFlag(int64_t formId, bool flag);
    /**
     * @brief Update Interval timer task value.
     * @param formId The Id of the form.
     * @param timerCfg task value.
     * @return Returns true on success, false on failure.
     */
    bool UpdateIntervalValue(const int64_t formId, const FormTimerCfg &timerCfg);
    /**
     * @brief Update update at timer task value.
     * @param formId The Id of the form.
     * @param timerCfg task value.
     * @return Returns true on success, false on failure.
     */
    bool UpdateAtTimerValue(const int64_t formId, const FormTimerCfg &timerCfg);
    /**
     * @brief Interval timer task to update at timer task.
     * @param formId The Id of the form.
     * @param timerCfg task value.
     * @return Returns true on success, false on failure.
     */
    bool IntervalToAtTimer(const int64_t formId, const FormTimerCfg &timerCfg);
    /**
     * @brief Update at timer task to interval timer task.
     * @param formId The Id of the form.
     * @param timerCfg task value.
     * @return Returns true on success, false on failure.
     */
    bool AtTimerToIntervalTimer(const int64_t formId, const FormTimerCfg &timerCfg);
    /**
     * @brief Delete interval timer task.
     * @param formId The Id of the form.
     * @return Returns true on success, false on failure.
     */
    bool DeleteIntervalTimer(const int64_t formId);
    /**
     * @brief Delete update at timer.
     * @param formId The Id of the form.
     * @return Returns true on success, false on failure.
     */
    bool DeleteUpdateAtTimer(const int64_t formId);
    /**
     * @brief Update at timer task alarm.
     * @return Returns true on success, false on failure.
     */
    bool UpdateAtTimerAlarm();
    /**
     * @brief Update limiter task alarm.
     * @return Returns true on success, false on failure.
     */
    bool UpdateLimiterAlarm();
    /**
     * @brief Clear limiter timer resource.
     */
    void ClearLimiterTimerResource();
    /**
     * @brief Delete dynamic refresh item.
     * @param formId The Id of the form.
     * @return Returns true on success, false on failure.
     */
    bool DeleteDynamicItem(const int64_t formId);
    /**
     * @brief Update dynamic refresh task alarm.
     * @return Returns true on success, false on failure.
     */
    bool UpdateDynamicAlarm();
    /**
     * @brief Clear dynamic refresh resource.
     */
    void ClearDynamicResource();
    /**
     * @brief Fint next at timer item.
     * @param nowTime Update time.
     * @param updateAtItem Next at timer item.
     * @return Returns true on success, false on failure.
     */
    bool FindNextAtTimerItem(const int nowTime, UpdateAtItem &updateAtItem);
    /**
     * @brief Clear update at timer resource.
     */
    void ClearUpdateAtTimerResource();

    /**
     * @brief Execute Form timer task.
     * @param task Form timer task.
     */
    void ExecTimerTask(const FormTimer &task);

    /**
     * @brief Init.
     */
    void Init();
    /**
     * @brief Ensure init interval timer resource.
     */
    void EnsureInitIntervalTimer();
    /**
     * @brief Clear interval timer resource.
     */
    void ClearIntervalTimer();
    /**
    * @brief Creat thread pool for timer task.
    */
    void CreatTaskThreadExecutor();
    /**
     * @brief Set enable flag.
     * @param formId The Id of the form.
     * @param flag Enable flag.
     */
    void SetEnableFlag(int64_t formId, bool flag);
    /**
     * @brief Get WantAgent.
     * @param updateAtTime The next update time.
     * @return Returns WantAgent.
     */
    std::shared_ptr<WantAgent> GetUpdateAtWantAgent(long updateAtTime);
    /**
     * @brief Get WantAgent.
     * @return Returns WantAgent.
     */
    std::shared_ptr<WantAgent> GetLimiterWantAgent();
    /**
     * @brief Get WantAgent.
     * @param nextTime The next update time.
     * @return Returns WantAgent.
     */
    std::shared_ptr<WantAgent> GetDynamicWantAgent(long nextTime);
private:
    /**
     * @class TimerReceiver
     * timer event receiver.
     */
    class TimerReceiver : public EventFwk::CommonEventSubscriber {
    public:
        TimerReceiver() = default;
        TimerReceiver(const EventFwk::CommonEventSubscribeInfo &subscriberInfo);
        virtual ~TimerReceiver() = default;
        /**
         * @brief Receive common event.
         * @param eventData Common event data.
         */
        virtual void OnReceiveEvent(const EventFwk::CommonEventData &eventData) override;
    };

    struct {
        bool operator()(DynamicRefreshItem a, DynamicRefreshItem b) const
        {
            return (a.settedTime > b.settedTime ? true : false);
        }
    } CompareDynamicRefreshItem;

    mutable std::mutex intervalMutex_;
    mutable std::mutex updateAtMutex_;
    mutable std::mutex dynamicMutex_;
    mutable std::mutex refreshMutex_;
    FormRefreshLimiter refreshLimiter_;
    std::map<int64_t, FormTimer> intervalTimerTasks_;
    std::list<UpdateAtItem> updateAtTimerTasks_;
    std::vector<DynamicRefreshItem> dynamicRefreshTasks_;
    std::shared_ptr<TimerReceiver> timerReceiver_ = nullptr;
    std::unique_ptr<ThreadPool> taskExecutor_ = nullptr;

    uint64_t intervalTimerId_ = 0L;
    uint64_t updateAtTimerId_ = 0L;
    uint64_t dynamicAlarmTimerId_ = 0L;
    uint64_t limiterTimerId_= 0L;

    std::shared_ptr<WantAgent> currentUpdateAtWantAgent = nullptr;
    std::shared_ptr<WantAgent> currentDynamicWantAgent = nullptr;
    std::shared_ptr<WantAgent> currentLimiterWantAgent = nullptr;

    long dynamicWakeUpTime_ = LONG_MAX;
    long atTimerWakeUpTime_ = LONG_MAX;
};
}  // namespace AppExecFwk
}  // namespace OHOS
#endif // FOUNDATION_APPEXECFWK_SERVICES_FORMMGR_INCLUDE_FORM_TIMER_MGR_H