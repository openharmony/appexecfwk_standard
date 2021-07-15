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
#ifndef FOUNDATION_APPEXECFWK_OHOS_PARALLEL_TASK_DISPATCHER_BASE_H
#define FOUNDATION_APPEXECFWK_OHOS_PARALLEL_TASK_DISPATCHER_BASE_H

#include <assert.h>
#include <list>
#include <iostream>
#include <string>
#include <vector>

#include "base_task_dispatcher.h"
#include "group.h"
#include "revocable.h"
#include "runnable.h"
#include "sync_task.h"
#include "task.h"
#include "task_priority.h"
#include "task_executor.h"
#include "task_execute_interceptor.h"

namespace OHOS {
namespace AppExecFwk {
/**
 *  Base implementation for parallel TaskDispatcher
 *
 *
 */
class ParallelTaskDispatcherBase : public BaseTaskDispatcher,
                                   public std::enable_shared_from_this<ParallelTaskDispatcherBase> {
public:
    // ParallelTaskDispatcherBase();

    ParallelTaskDispatcherBase(
        TaskPriority taskPriority, std::shared_ptr<TaskExecutor> &executor, const std::string &dispatcherName);
    virtual ~ParallelTaskDispatcherBase() = default;
    /**
     *  Called when post a task to the TaskDispatcher with waiting Attention: Call
     *  this function of Specific dispatcher on the corresponding thread will lock.
     *
     *  @param runnable is the job to execute
     *
     */
    ErrCode SyncDispatch(const std::shared_ptr<Runnable> &runnable) override;

    /**
     *  Called when post a task to the TaskDispatcher without waiting
     *
     *  @param runnable is the job to execute
     *  @return an interface for revoke the task if it hasn't been invoked.
     *
     */
    std::shared_ptr<Revocable> AsyncDispatch(const std::shared_ptr<Runnable> &runnable) override;

    /**
     *  Called when post a task group to the TaskDispatcher and without waiting
     *
     *  @param runnable is the job to execute
     *  @param delayMs indicate the delay time to execute
     *  @return an interface for revoke the task if it hasn't been invoked.
     *
     */
    std::shared_ptr<Revocable> DelayDispatch(const std::shared_ptr<Runnable> &runnable, long delayMs) override;

    /**
     *  Called when post a task to the TaskDispatcher and relates it to a group
     *  without waiting.
     *
     *  @param group related by task.
     *  @param runnable  is the job to execute.
     *  @return an interface for revoke the task if it hasn't been invoked.
     *
     */
    std::shared_ptr<Revocable> AsyncGroupDispatch(
        const std::shared_ptr<Group> &group, const std::shared_ptr<Runnable> &runnable) override;

protected:
    virtual std::shared_ptr<TaskExecuteInterceptor> GetInterceptor();

    std::shared_ptr<TaskExecutor> executor_ = nullptr;

private:
    static std::string DISPATCHER_TAG;
    static std::string ASYNC_DISPATCHER_TAG;
    static std::string SYNC_DISPATCHER_TAG;
    static std::string DELAY_DISPATCHER_TAG;
    static std::string ASYNC_GROUP_DISPATCHER_TAG;

    ErrCode InterceptedExecute(std::shared_ptr<Task> &task);

    class MyTaskListener : public TaskListener {
    private:
        std::function<void()> callback_;

    public:
        void OnChanged(const TaskStage &stage)
        {
            if (stage.IsDone()) {
                APP_LOGD("task done.");
                callback_();
            }
        }
        // set callback function
        void Callback(const std::function<void()> &callbackFunction)
        {
            callback_ = std::move(callbackFunction);
        }
    };
};

}  // namespace AppExecFwk
}  // namespace OHOS
#endif