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

#ifndef OHOS_APP_DISPATCHER_TASK_DISPATCHER_H
#define OHOS_APP_DISPATCHER_TASK_DISPATCHER_H

#include <memory>
#include "task_priority.h"
#include "revocable.h"
#include "runnable.h"
#include "group.h"
#include "appexecfwk_errors.h"
namespace OHOS {
namespace AppExecFwk {
template<typename T>
using IteratableTask = std::function<void(T)>;

class TaskDispatcher {
public:
    virtual ~TaskDispatcher() = default;

    /**
     * @brief Dispatches a task and waits until the task is complete in the current thread.
     *
     * @param runnable Indicates the task to be executed. This parameter includes all information required for the task
     *             execution.
     */
    virtual ErrCode SyncDispatch(const std::shared_ptr<Runnable> &runnable) = 0;

    /**
     * @brief Asynchronously dispatches a task.
     *
     * <p>This method dispatches a task and returns a value immediately without waiting for the task to execute.</p>
     *
     * @param runnable Indicates the task to be executed. This parameter includes all information required for the task
     *             execution.
     * @return Returns a {@link std::shared_ptr<OHOS::AppExecFwk::Revocable>} instance used for revoking the given task
     * if it has not been invoked.
     */
    virtual std::shared_ptr<Revocable> AsyncDispatch(const std::shared_ptr<Runnable> &runnable) = 0;

    /**
     * @brief Dispatches a task after the given delay.
     *
     * <p>This is an asynchronous execution and returns a value immediately without waiting. The task will be
     * dispatched to the queue after the given delay specified by {@code delayMs}, and its actual execution time
     * maybe later than the given time. The time when the task will be executed depends on the length of the queue
     * and how busy the thread pool is.
     * </p>
     *
     * @param runnable Indicates the task to be executed. This parameter includes all information required for the task
     *             execution.
     * @param delayMs Indicates the time period that the given task has to wait before being dispatched.
     * @return Returns a {@link std::shared_ptr<OHOS::AppExecFwk::Revocable>} instance used for revoking the given task
     *         if it has not been invoked.
     */
    virtual std::shared_ptr<Revocable> DelayDispatch(const std::shared_ptr<Runnable> &runnable, long delayMs) = 0;

    /**
     * @brief Sets an execution barrier in a task group for the given task and waits for the task to be executed after all
     * tasks in the task group are finished.
     *
     * <p>The current thread is blocked until the given task is complete. Incorrect use of this method may cause a
     * deadlock.
     * If too many threads are blocked, resources in the thread pool will be used up.</p>
     *
     * @param runnable Indicates the task to be executed.
     */
    virtual ErrCode SyncDispatchBarrier(const std::shared_ptr<Runnable> &runnable) = 0;

    /**
     * @brief Sets an execution barrier for the given task to be executed only after all tasks in the task group are
     * finished. This method does not wait for the given task to execute.
     *
     * <p>Task execution barriers are unavailable on global task dispatchers. Example code for setting a task
     * execution barrier:</p>
     *
     * <pre>{@code
     * std::shared_ptr<ParallelTaskDispatcher> parallelTaskDispatcher =
     *     context.CreateParallelTaskDispatcher(dispatcherName, taskPriority);
     * parallelTaskDispatcher->AsyncDispatch(std::make_shared<Runnable>([](){
     *     sleep(1000);
     *     // sleep enough time
     *     std::cout << "before barrier" << std::endl;
     * }));
     * parallelTaskDispatcher->AsyncDispatchBarrier(std::make_shared<Runnable>([]() {
     *     std::cout << "Barrier" << std::endl;
     * }));
     * parallelTaskDispatcher->AsyncDispatch(std::make_shared<Runnable>([]() {
     *     std::cout << "after barrier" << std::endl;
     * }));
     *
     * // Result will be: 1.before barrier; 2.Barrier; 3.after barrier.
     * }
     * </pre>
     *
     * @param runnable Indicates the task to be executed. This parameter includes all information required for the task
     *             execution.
     */
    virtual ErrCode AsyncDispatchBarrier(const std::shared_ptr<Runnable> &runnable) = 0;

    /**
     * @brief Creates a task group.
     *
     * <p>The {@link std::shared_ptr<OHOS::AppExecFwk::Group>} object returned by this method does not contain any task.
     * If you need to dispatch tasks asynchronously, you can use {@link #asyncGroupDispatch} to associate this object
     * with multiple tasks for monitoring the execution progress of all these tasks.</p>
     * <p>Example code for waiting for the completion of all download tasks in a task group:</p>
     * <pre> {@code
     * void groupTest(Context context) {
     *     std::shared_ptr<ParallelTaskDispatcher> parallelTaskDispatcher =
     *         context.CreateParallelTaskDispatcher(dispatcherName, taskPriority);
     *     // Creates a task group.
     *     std::shared_ptr<Group> group = context.CreateDispatchGroup();
     *     // Adds asynchronous tasks to the task group.
     *     dispatcher->AsyncGroupDispatch(group, []() {
     *         // Downloads a 10 MB file.
     *         download(url1);
     *         std::cout << "Finished downloading URL 1" << std::endl;
     *     });
     *
     *     std::cout << "1" << std::endl;
     *
     *     dispatcher->AsyncGroupDispatch(group, []() {
     *         // Downloads a 1 MB file.
     *         download(url2);
     *         std::cout << "Finished downloading URL 2" << std:endl;
     *     });
     *
     *     // Executes a new task until all tasks in the task group are complete.
     *     dispatcher->GroupDispatchNotify(group, []() {
     *         std::cout << "All tasks finished. Do shutdown." << std:endl;
     *     });
     *     std::cout << "2" << std::endl;
     * }
     *
     * @return Returns a {@link std::shared_ptr<Group>} object.
     */
    virtual std::shared_ptr<Group> CreateDispatchGroup() = 0;

    /**
     * @brief Adds a task to a task group asynchronously.
     *
     * <p>This method returns a value immediately without waiting for the task to execute.</p>
     *
     * @param group Indicates the group to which the task is added.
     * @param runnable Indicates the task to be executed. This parameter includes all information required for the task
     *             execution.
     * @return Returns a {@link std::shared_ptr<Revocable>} instance used for revoking the given task
     *         if it has not been invoked.
     */
    virtual std::shared_ptr<Revocable> AsyncGroupDispatch(
        const std::shared_ptr<Group> &group, const std::shared_ptr<Runnable> &runnable) = 0;

    /**
     * @brief Waits all tasks in a task group to finish.
     *
     * @param group Indicates the task group containing the waiting tasks.
     * @param timeout Indicates the maximum waiting time for a task.
     * @return Returns {@code true} if all tasks are correctly executed; returns {@code false} if any task times out.
     */
    virtual bool GroupDispatchWait(const std::shared_ptr<Group> &group, long timeout) = 0;

    /**
     * @brief Executes a task after all tasks in the task group are finished.
     *
     * @param group Indicates the task group containing the waiting tasks.
     * @param runnable Indicates the task to be executed after all tasks are finished.
     */
    virtual ErrCode GroupDispatchNotify(
        const std::shared_ptr<Group> &group, const std::shared_ptr<Runnable> &runnable) = 0;

    /**
     * @brief Executes a task multiple times without waiting.
     *
     * @param task Indicates the task to be executed. This parameter includes all information required for the task
     *             execution.
     * @param iterations Indicates the number of times the task is executed. The value must be a positive integer.
     */
    virtual ErrCode ApplyDispatch(const std::shared_ptr<IteratableTask<long>> &task, long iterations) = 0;
};
}  // namespace AppExecFwk
}  // namespace OHOS

#endif