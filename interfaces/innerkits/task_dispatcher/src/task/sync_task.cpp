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

#include "sync_task.h"
#include <thread>

namespace OHOS {
namespace AppExecFwk {
// Help to calculate hash code of object.
template<typename T>
inline size_t CalculateHashCode(const T &obj)
{
    std::hash<T> calculateHashCode;
    return calculateHashCode(obj);
}

SyncTask::SyncTask(const std::shared_ptr<Runnable> &runnable, TaskPriority priority,
    const std::shared_ptr<BaseTaskDispatcher> &baseTaskDispatcher)
    : Task(runnable, priority, baseTaskDispatcher)
{
    executed_.store(false);
}

void SyncTask::Run()
{
    std::unique_lock<std::mutex> lock(mutex_);
    auto threadId = std::this_thread::get_id();
    APP_LOGI("SyncTask::Run begin thread=%{public}zu", CalculateHashCode(threadId));
    Task::Run();
    executed_.store(true);

    // |waitTask| may have been multi invoked.
    condition_variable_.notify_all();
    APP_LOGI("SyncTask::Run end thread=%{public}zu", CalculateHashCode(threadId));
}

void SyncTask::WaitTask()
{
    auto threadId = std::this_thread::get_id();
    APP_LOGI("SyncTask::WaitTask begin thread=%{public}zu", CalculateHashCode(threadId));

    std::unique_lock<std::mutex> lock(mutex_);
    while (executed_ == false) {
        condition_variable_.wait(lock);
    }

    APP_LOGI("SyncTask::WaitTask end thread=%{public}zu", CalculateHashCode(threadId));
}

}  // namespace AppExecFwk
}  // namespace OHOS
