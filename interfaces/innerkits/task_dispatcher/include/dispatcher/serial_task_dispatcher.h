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

#ifndef OHOS_APP_DISPATCHER_SERIAL_TASK_DISPATCHER_H
#define OHOS_APP_DISPATCHER_SERIAL_TASK_DISPATCHER_H

#include <string>

#include "base_task_dispatcher.h"
#include "task.h"
#include "sync_task.h"
#include "task_executor.h"
#include "task_priority.h"
#include "revocable.h"
#include "task_listener.h"
#include "concurrent_queue.h"
#include "runnable.h"

namespace OHOS {
namespace AppExecFwk {

/**
 *  Dispatcher for serial thread model.
 */
class SerialTaskDispatcher : public BaseTaskDispatcher, public std::enable_shared_from_this<SerialTaskDispatcher> {
private:
    static std::string DISPATCHER_TAG;
    static std::string ASYNC_DISPATCHER_TAG;
    static std::string SYNC_DISPATCHER_TAG;
    static std::string DELAY_DISPATCHER_TAG;

    std::atomic<bool> running_;
    ConcurrentQueue<std::shared_ptr<Task>> workingTasks_;

    std::shared_ptr<TaskExecutor> executor_;
    std::mutex mutex_;

private:
    ErrCode OnNewTaskIn(std::shared_ptr<Task> &task);

    ErrCode Prepare(std::shared_ptr<Task> &task);

    /**
     *  Callback for task when finish.
     *
     */
    void OnTaskDone();

    bool Schedule();

    /**
     *  Do task in turn, until the task queue is empty.
     *
     *  @param isExhausted is an inaccurate judge indicate that the workingTasks is empty. If true, do double-check.
     *  @return true if has work remain to do else false.
     *
     */
    bool DoNext(bool isExhausted);

    void DoWork(std::shared_ptr<Task> &task);

public:
    /**
     *  Constructs a serial TaskDispatcher using priority.
     *
     *  @param dispatcherName is the identity for dispatcher. It's for debug purpose.
     *  @param priority is the priority for tasks dispatched by this Dispatcher.
     *  @param executor contains thread pool(s) for executing tasks concurrently.
     *
     */
    SerialTaskDispatcher(
        const std::string &dispatcherName, const TaskPriority priority, const std::shared_ptr<TaskExecutor> &executor);
    ~SerialTaskDispatcher(){};

    /**
     *  Gets waiting tasks count of SerialTaskDispatcher.
     *
     *  @return The waiting tasks count of SerialTaskDispatcher.
     *
     */
    int GetWorkingTasksSize();

    /**
     *  Gets name of SerialTaskDispatcher.
     *
     *  @return The name of SerialTaskDispatcher.
     *
     */
    std::string GetDispatcherName();

    /**
     *  Called when post a task to the TaskDispatcher with waiting Attention: Call
     *  this function of Specific dispatcher on the corresponding thread will lock.
     *
     *  @param runnable is the job to execute
     *
     */
    ErrCode SyncDispatch(const std::shared_ptr<Runnable> &runnable);

    /**
     *  Called when post a task to the TaskDispatcher without waiting
     *
     *  @param runnable is the job to execute
     *  @return an interface for revoke the task if it hasn't been invoked.
     *
     */
    std::shared_ptr<Revocable> AsyncDispatch(const std::shared_ptr<Runnable> &runnable);

    /**
     *  Called when post a task group to the TaskDispatcher and without waiting
     *
     *  @param runnable  is the job to execute
     *  @param delayMs indicate the delay time to execute
     *  @return an interface for revoke the task if it hasn't been invoked.
     *
     */
    std::shared_ptr<Revocable> DelayDispatch(const std::shared_ptr<Runnable> &runnable, long delayMs);
};

}  // namespace AppExecFwk
}  // namespace OHOS

#endif
