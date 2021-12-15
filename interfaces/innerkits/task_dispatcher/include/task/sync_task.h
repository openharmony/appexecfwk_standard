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

#ifndef OHOS_APP_DISPATCHER_TASK_SYNC_TASK_H
#define OHOS_APP_DISPATCHER_TASK_SYNC_TASK_H

#include <condition_variable>
#include <mutex>

#include "runnable.h"
#include "task_priority.h"
#include "task.h"
#include "app_log_wrapper.h"
#include "base_task_dispatcher.h"

namespace OHOS {
namespace AppExecFwk {
/**
 *  SyncTask is the synchronized task which provide synchronized method.
 */
class SyncTask final : public Task {
public:
    SyncTask(const std::shared_ptr<Runnable> &runnable, TaskPriority priority,
        const std::shared_ptr<BaseTaskDispatcher> &baseTaskDispatcher);
    ~SyncTask() = default;
    void Run() override;

    void WaitTask();

private:
    std::atomic<bool> executed_;
    std::mutex mutex_;
    std::condition_variable condition_variable_;
};
}  // namespace AppExecFwk
}  // namespace OHOS
#endif
