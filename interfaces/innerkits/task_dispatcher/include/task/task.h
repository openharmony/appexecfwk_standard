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
#ifndef OHOS_APP_DISPATCHER_TASK_H
#define OHOS_APP_DISPATCHER_TASK_H

#include <atomic>
#include <deque>
#include <memory>
#include <thread>
#include "concurrent_queue.h"
#include "revocable.h"
#include "runnable.h"
#include "task_listener.h"
#include "task_priority.h"
#include "task_stage.h"

namespace OHOS {
namespace AppExecFwk {
class BaseTaskDispatcher;
class Task : public Revocable {
public:
    /**
     * @brief Constructs the object.
     * @param runnable The user task wrapped in.
     * @param priority The priority
     * @return -
     */
    Task(const std::shared_ptr<Runnable> &runnable, const TaskPriority priority,
        const std::shared_ptr<BaseTaskDispatcher> &baseTaskDispatcher);

    virtual ~Task();

    /**
     *@brief invoke the function to execute the task
     *@param -
     *@return void
     */
    virtual void Run();

    /**
     *  @brief Gets the priority.
     *  @param -
     *  @return The priority.
     */
    TaskPriority GetPriority() const;

    /**
     *  @brief Sets the sequence.
     *  @param -
     *  @param sequence The sequence
     *  @return void
     */
    void SetSequence(long sequence);

    /**
     *@brief Gets the sequence.
     *@param -
     *@return The sequence.
     */
    long GetSequence() const;

    /**
     *@brief Revoke this task if hasn't run.
     *@param -
     *@return true if set revoked or already revoked. False if the task has start executing.
     */
    bool Revoke() override;

    /**
     *@brief Adds a task listener.
     *@param listener The listener
     *@return void
     */
    void AddTaskListener(const std::shared_ptr<TaskListener> &listener);

    /**
     *@brief Called when task is about to run.
     *@param -
     *@return void
     */
    void BeforeTaskExecute();

    /**
     *@brief Called when task is done.
     *@param -
     *@return void
     */
    void AfterTaskExecute();

    /**
     *@brief Called when task is canceled.
     *@param -
     *@return void
     */
    void OnTaskCanceled();

    /**
     *@brief Save traceId for using on task running thread.
     *@param hiTraceId The traceId.
     *@return void
     */
    // void SetTaskHiTraceId(HiTraceId &hiTraceId);

    bool operator==(std::shared_ptr<Task> &rec) const;

protected:
    /**
     *  The user task wrapped in.
     */
    std::shared_ptr<Runnable> runnable_;

private:
    using RevokeResult = enum {
        // Maybe already run.
        FAIL,
        // Convert |revoked| flag from false to true.
        SUCCESS,
        // The |revoked| flag is already set to true.
        ALREADY_REVOKED
    };

    /**
     *@brief Return true if not executed or revoked, and if not executed or revoked, ensure |EXECUTED| to be set.
     *@param -
     *@return bool
     */
    bool EnterExecute();

    RevokeResult SetRevoked();

    void ConcurrentQueueStatusUpdate(const TaskStage::TASKSTAGE taskstage);

private:
    const static int EXECUTED = (1 << 0);
    const static int REVOKED = (1 << 1);
    long sequence_;
    std::atomic<int> state_;
    TaskPriority priority_;
    std::shared_ptr<Revocable> revocable_ = nullptr;
    ConcurrentQueue<std::shared_ptr<TaskListener>> taskListeners_;
    std::shared_ptr<BaseTaskDispatcher> baseTaskDispatcher_;
};

}  // namespace AppExecFwk
}  // namespace OHOS

#endif  // OHOS_APP_DISPATCHER_TASK_H
