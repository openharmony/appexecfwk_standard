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
#ifndef OHOS_APP_DISPATCHER_GROUP_IMPL_H
#define OHOS_APP_DISPATCHER_GROUP_IMPL_H
#include <atomic>
#include <deque>
#include <mutex>
#include <condition_variable>
#include "group.h"
#include "runnable.h"

namespace OHOS {
namespace AppExecFwk {
/**
 *  A group used to associated with tasks.
 */
class GroupImpl : public Group {

public:
    GroupImpl();
    /**
     *  Wait all tasks associated to this group to be done.
     *  @param timeout is the max waiting time for jobs in group execute, in ms.
     *  @return true if successfully wait.
     */
    bool AwaitAllTasks(long timeout);

    /**
     *  Associates a task to this group.
     */
    void Associate();

    /**
     *  Notify group that a task is done or canceled.
     */
    void NotifyTaskDone();

    /**
     *  Adds the |notification| to notification list.
     *  If all tasks are already done, |notification| will immediately be called on current thread.
     *  Attention: If tasks are added just this time, it may not be considered.
     *  @param notification Called when all tasks done.
     */
    bool AddNotification(const std::shared_ptr<Runnable> &notification);

private:
    const static int MAX_TASK = 1000;
    std::atomic<int> count_;
    std::deque<std::shared_ptr<Runnable>> notifications_;
    std::mutex dataMutex_;
    std::condition_variable condition_;

private:
    /**
     *  Notify all tasks and remove from queue.
     *  Attention: Notifications added after all tasks done is not guaranteed.
     */
    void DrainNotifications();
};

}  // namespace AppExecFwk
}  // namespace OHOS
#endif
