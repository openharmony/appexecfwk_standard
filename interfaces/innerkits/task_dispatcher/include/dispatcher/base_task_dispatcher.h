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
#ifndef OHOS_APP_DISPATCHER_BASE_TASK_DISPATCHER_H
#define OHOS_APP_DISPATCHER_BASE_TASK_DISPATCHER_H

#include <atomic>
#include <memory>
#include "revocable.h"
#include "runnable.h"
#include "task.h"
#include "group.h"
#include "group_impl.h"
#include "task_dispatcher.h"
#include "task_execute_interceptor.h"

namespace OHOS {
namespace AppExecFwk {
/**
 * Base implementation for interface of TaskDispatcher.
 */

class BaseTaskDispatcher : public TaskDispatcher {
public:
    BaseTaskDispatcher(const std::string &dispatcherName, const TaskPriority priority);
    virtual ~BaseTaskDispatcher(){};

    /**
     *  Called when post a task to the TaskDispatcher with waiting Attention: Call
     *  this function of Specific dispatcher on the corresponding thread will lock.
     *  @param runnable is the job to execute.
     */
    virtual ErrCode SyncDispatch(const std::shared_ptr<Runnable> &runnable) override = 0;

    /**
     *  Called when post a task to the TaskDispatcher without waiting.
     *
     *  @param runnable is the job to execute.
     *  @return an interface for revoke the task if it hasn't been invoked.
     *
     */
    virtual std::shared_ptr<Revocable> AsyncDispatch(const std::shared_ptr<Runnable> &runnable) override = 0;

    /**
     *  Called when post a task group to the TaskDispatcher and without waiting.
     *  @param runnable    is the job to execute.
     *  @param delayMs indicate the delay time to execute.
     *  @return an interface for revoke the task if it hasn't been invoked.
     */
    virtual std::shared_ptr<Revocable> DelayDispatch(
        const std::shared_ptr<Runnable> &runnable, long delayMs) override = 0;
    /**
     *  Set a barrier and meanwhile a sync task that execute after all tasks finished.
     *  @param runnable  is the job to execute after barrier.
     */
    ErrCode SyncDispatchBarrier(const std::shared_ptr<Runnable> &runnable) override;

    /**
     * Set a barrier and meanwhile an async task that execute after all tasks finished.
     *  @param runnable  is the job to execute after jobs in group.
     */
    ErrCode AsyncDispatchBarrier(const std::shared_ptr<Runnable> &runnable) override;
    /**
     * 	Create a group.
     *  @return a new created group.
     */
    std::shared_ptr<Group> CreateDispatchGroup() override;

    /**
     *  Called when post a task to the TaskDispatcher and relates it to a group
     *  without waiting.
     *  @param group related by task.
     *  @param runnable  is the job to execute.
     *  @return an interface for revoke the task if it hasn't been invoked.
     *
     */
    virtual std::shared_ptr<Revocable> AsyncGroupDispatch(
        const std::shared_ptr<Group> &group, const std::shared_ptr<Runnable> &runnable) override;

    /**
     *  Synchronously waiting all tasks in the group to be done.
     *  @param group   contains a serial of jobs.
     *  @param timeout is the max waiting time for jobs in group execute, in ms.
     *  @return true if all jobs in group has finished or false if timeout occurs.
     *
     */
    bool GroupDispatchWait(const std::shared_ptr<Group> &group, long timeout) override;

    /**
     *  Asynchronously waiting all tasks in the group to be done. |task| will be run after waiting.
     *
     *  @param group contains a serial of jobs.
     *  @param runnable  executes after all jobs in group finished.
     *
     */
    ErrCode GroupDispatchNotify(
        const std::shared_ptr<Group> &group, const std::shared_ptr<Runnable> &runnable) override;

    /**
     *  Called to dispatch |task| |iterations| times and wait.
     *
     *  @param task       is the job to execute multi times.
     *  @param iterations indicates times the task should be executed.
     *
     */
    ErrCode ApplyDispatch(const std::shared_ptr<IteratableTask<long>> &task, long iterations) override;

    /**
     *  Gets the priority.
     *  @return The priority.
     */
    TaskPriority GetPriority() const;

    /**
     *  Create span and set HiTraceId for task, and then tracePoint information
     *  @param task  of which the information to tracePoint
     *  @param isAsyncTask whether the task is async
     *  @param dispatcherName the name of dispatcher which post the task
     *  @return valid HiTraceId if set successfully
     *
     */
    void TracePointBeforePost(std::shared_ptr<Task> &task, bool isAsyncTask, const std::string &dispatcherName) const;

    /**
     *  TracePoint information after post task
     *
     *  @param task of which the information to tracePoint
     *  @param isAsyncTask whether the task is async
     *  @param dispatcherName the name of dispatcher which post the task
     *
     */
    void TracePointAfterPost(std::shared_ptr<Task> &task, bool isAsyncTask, const std::string &dispatcherName) const;

protected:
    /**
     *  Name of dispatcher.
     *
     */
    std::string dispatcherName_;

    TaskPriority taskPriority_;

protected:
    /**
     *  Check for the |task| argument.
     *
     *  @param task The task to check
     *
     */
    ErrCode Check(const std::shared_ptr<Runnable> &task) const;

    /**
     *  Check for the |group| argument.
     *
     *  @param group The group to check.
     *  @return GroupImpl
     *
     */
    std::shared_ptr<GroupImpl> CastToGroupImpl(const std::shared_ptr<Group> &group);

    /**
     *  Gets the task interceptor.
     *  Subclasses override this function to change the interceptor.
     *
     *  @return The TaskExecuteInterceptor.
     *
     */
    std::shared_ptr<TaskExecuteInterceptor> GetInterceptor();

private:
    static std::atomic<int> SEQUENCE_;
};
}  // namespace AppExecFwk
}  // namespace OHOS

#endif