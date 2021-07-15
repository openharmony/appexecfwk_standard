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
#include "group_impl.h"
#include "app_log_wrapper.h"
namespace OHOS {
namespace AppExecFwk {
GroupImpl::GroupImpl()
{
    count_.store(0);
}

/**
 * Wait all tasks associated to this group to be done.
 *
 * @param timeout is the max waiting time for jobs in group execute, in ms.
 *
 * @return true if successfully wait.
 */
bool GroupImpl::AwaitAllTasks(long timeout)
{
    if (count_.load() == 0) {
        APP_LOGD("GroupImpl::AwaitAllTasks number of count_ is zero");
        return true;
    }
    if (timeout <= 0L) {
        return false;
    }
    bool success = true;
    std::unique_lock<std::mutex> lock(dataMutex_);
    while (count_.load() > 0) {
        if (condition_.wait_for(lock, std::chrono::milliseconds(timeout)) == std::cv_status::timeout) {
            success = false;
            APP_LOGD("GroupImpl::awaitAllTasks timeout");
            break;
        }
        APP_LOGD("GroupImpl::awaitAllTasks success");
    }
    return success;
}

/**
 *  @brief Associates a task to this group.
 *
 * @return None
 */
void GroupImpl::Associate()
{
    APP_LOGD("GroupImpl::Associate called  add a task");
    // count_++;
    count_.fetch_add(1);
}
/**
 * @brief Notify group that a task is done or canceled.
 *
 * @return None
 */
void GroupImpl::NotifyTaskDone()
{
    APP_LOGD("GroupImpl::NotifyTaskDone called notify a task to complete");
    // int newValue = (--count_);
    count_.fetch_sub(1);
    int newValue = count_.load();
    if (newValue > 0) {
        return;
    }
    std::unique_lock<std::mutex> lock(dataMutex_);
    condition_.notify_all();
    DrainNotifications();
}
/**
 * @brief Adds the |notification| to notification list.
 * If all tasks are already done, |notification| will immediately be called on current thread.
 * Attention: If tasks are added just this time, it may not be considered.
 *
 * @param notification Called when all tasks done.
 *
 *  @return None
 */
bool GroupImpl::AddNotification(const std::shared_ptr<Runnable> &notification)
{
    if (count_.load() != 0) {
        std::unique_lock<std::mutex> lock(dataMutex_);
        if (notifications_.size() == MAX_TASK) {
            APP_LOGW("GroupImpl::AddNotification called maximun number of tasks exceeded");
            return false;
        }
        if (count_.load() != 0) {
            APP_LOGD("GroupImpl::AddNotification called add task");
            notifications_.push_back(notification);
            return true;
        }
    }
    if (notification) {
        (*notification)();
    }
    return true;
}
/**
 *@brief  Notify all tasks and remove from queue.
 * Attention: Notifications added after all tasks done is not guaranteed.
 *
 * @return None
 */
void GroupImpl::DrainNotifications()
{
    APP_LOGD("GroupImpl::DrainNotifications called task execution");
    while (notifications_.size() > 0) {
        std::shared_ptr<Runnable> notification = notifications_.front();
        notifications_.pop_front();
        (*notification)();
    }
}

}  // namespace AppExecFwk
}  // namespace OHOS
