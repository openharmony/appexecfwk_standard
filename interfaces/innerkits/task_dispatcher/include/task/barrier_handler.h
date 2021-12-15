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
#ifndef OHOS_APP_DISPATCHER_BARRIER_HANDLER_H
#define OHOS_APP_DISPATCHER_BARRIER_HANDLER_H

#include <set>
#include "appexecfwk_errors.h"
#include "task.h"
#include "task_listener.h"
#include "task_executor.h"
#include "task_execute_interceptor.h"
#include "task_stage.h"

namespace OHOS {
namespace AppExecFwk {
class BarrierHandler : public TaskExecuteInterceptor {
public:
    BarrierHandler(const std::shared_ptr<TaskExecutor> &executor);
    ~BarrierHandler() = default;
    /**
     * Intercept executing a task.
     */
    ErrCode Intercept(std::shared_ptr<Task> &task) override;

    /**
     * Adds a task with barrier semantics.
     */
    ErrCode AddBarrier(std::shared_ptr<Task> &barrierTask);

private:
    ErrCode ListenToTask(std::shared_ptr<Task> &task);
    void OnTaskDone(std::shared_ptr<Task> &task);
    bool AddTaskAfterBarrier(std::shared_ptr<Task> &task);
    bool HasTask(const std::set<std::shared_ptr<Task>> &tasks);
    std::set<std::shared_ptr<Task>> CreateTaskSet(std::shared_ptr<Task> &firstTask);

private:
    class BarrierPair {
    public:
        std::set<std::shared_ptr<Task>> tasks_;
        std::shared_ptr<Task> barrier_;

        BarrierPair(const std::set<std::shared_ptr<Task>> &tasks, const std::shared_ptr<Task> &barrier)
        {
            tasks_ = tasks;
            barrier_ = barrier;
        };
        ~BarrierPair() = default;
    };

    class MyTaskListener : public TaskListener {
    private:
        std::function<void()> callback_;

    public:
        void OnChanged(const TaskStage &stage)
        {
            if (stage.IsDone()) {
                APP_LOGI("BarrierHandler task done.");
                callback_();
            }
        };
        // set callback function
        void Callback(const std::function<void()> &callbackFunction)
        {
            callback_ = std::move(callbackFunction);
        };
    };

    std::mutex barrierLock_;
    std::shared_ptr<TaskExecutor> executor_;
    std::deque<std::shared_ptr<BarrierPair>> barrierQueue_;
};
}  // namespace AppExecFwk
}  // namespace OHOS

#endif