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
#include "task.h"
#include "app_log_wrapper.h"
namespace OHOS {
namespace AppExecFwk {

/**
 * @brief Constructs the object.
 * @param runnable The user task wrapped in.
 * @param priority The priority
 * @return None
 */
Task::Task(const std::shared_ptr<Runnable> &runnable, const TaskPriority priority,
    const std::shared_ptr<BaseTaskDispatcher> &baseTaskDispatcher)
    : taskListeners_()
{
    runnable_ = runnable;
    priority_ = priority;
    baseTaskDispatcher_ = baseTaskDispatcher;
    std::atomic_init(&state_, 0);
    APP_LOGD("Task::Task init listener count=%{public}zu", taskListeners_.Size());
}
Task::~Task()
{}

/**
 *@brief invoke the function to execute the task
 *@param None
 *@return None
 */
void Task::Run()
{
    APP_LOGD("Task::Run task run called start");
    if (runnable_ == nullptr) {
        APP_LOGI("Task::Run called runnable_ is null");
        return;
    }
    // Task cannot be reused.
    if (EnterExecute()) {
        APP_LOGD("Task::Run runnable_ Run called start");
        (*runnable_.get())();
    }
    APP_LOGD("Task::Run runnable_ Run called end");
}

/**
 *  @brief Gets the priority.
 *  @param None
 *  @return The priority.
 */
TaskPriority Task::GetPriority() const
{
    return priority_;
}

/**
 *  @brief Sets the sequence.
 *  @param None
 *  @param sequence The sequence
 *  @return None
 */
void Task::SetSequence(long sequence)
{
    sequence_ = sequence;
}

/**
 *@brief Gets the sequence.
 *@param None
 *@return The sequence.
 */
long Task::GetSequence() const
{
    return sequence_;
}

/**
 *@brief Revoke this task if hasn't run.
 *@param None
 *@return true if set revoked or already revoked. False if the task has start executing.
 */
bool Task::Revoke()
{
    if (runnable_ == nullptr) {
        APP_LOGI("Task.Revoke called runnable_ is null");
        return false;
    }
    RevokeResult result = SetRevoked();
    APP_LOGD("Task.Revoke result: %{public}u", result);
    if (result == SUCCESS) {
        OnTaskCanceled();
    }
    return (result == SUCCESS) || (result == ALREADY_REVOKED);
}

/**
 *@brief Adds a task listener.
 *@param listener The listener
 *@return None
 */
void Task::AddTaskListener(const std::shared_ptr<TaskListener> &listener)
{
    APP_LOGD("Task.AddTaskListener listener called start");
    taskListeners_.Offer(listener);
}

/**
 *@brief Called when task is about to run.
 *@param None
 *@return None
 */
void Task::BeforeTaskExecute()
{
    if ((state_ & REVOKED) != REVOKED) {
        ConcurrentQueueStatusUpdate(TaskStage::BEFORE_EXECUTE);
    }
}

/**
 *@brief Called when task is done.
 *@param None
 *@return None
 */
void Task::AfterTaskExecute()
{
    if ((state_ & EXECUTED) == EXECUTED) {
        APP_LOGD("Task.AfterTaskExecute taskStage called AFTER_EXECUTE");
        ConcurrentQueueStatusUpdate(TaskStage::AFTER_EXECUTE);
    }
}

/**
 *@brief Called when task is canceled.
 *@param None
 *@return None
 */
void Task::OnTaskCanceled()
{
    APP_LOGD("Task.OnTaskCanceled taskStage called REVOKED");
    ConcurrentQueueStatusUpdate(TaskStage::REVOKED);
}

/**
 *@brief Return true if not executed or revoked, and if not executed or revoked, ensure |EXECUTED| to be set.
 *@param None
 *@return bool
 */
bool Task::EnterExecute()
{
    int stateNotIn = EXECUTED | REVOKED;
    while (true) {
        int value = state_.load();
        if ((state_ & stateNotIn) == 0) {
            // Not executed or revoked
            if (state_.compare_exchange_strong(value, EXECUTED)) {
                APP_LOGD("Task.EnterExecute return success");
                return true;
            }
        } else {
            APP_LOGD("Task.EnterExecute return fail, state=%{public}d, stateNotIn=%{public}d", value, stateNotIn);
            return false;
        }
    }
}

Task::RevokeResult Task::SetRevoked()
{
    while (true) {
        int value = state_.load();
        if ((value & REVOKED) == REVOKED) {
            APP_LOGD("Task.SetRevoked return ALREADY_REVOKED");
            return ALREADY_REVOKED;
        }
        if ((value & EXECUTED) == 0) {
            if (state_.compare_exchange_strong(value, REVOKED)) {
                APP_LOGD("Task.SetRevoked return SUCCESS");
                return SUCCESS;
            }
        } else {
            APP_LOGD("Task.SetRevoked return FAIL");
            return FAIL;
        }
    }
}

void Task::ConcurrentQueueStatusUpdate(const TaskStage::TASKSTAGE taskstage)
{
    APP_LOGD("Task.ConcurrentQueueStatusUpdate taskListeners_ called start");
    for (auto iter = taskListeners_.Begin(); iter != taskListeners_.End(); iter++) {
        (*iter)->OnChanged(taskstage);
    }
    if (taskstage == TaskStage::TASKSTAGE::AFTER_EXECUTE) {
        taskListeners_.clear();
    }
}

bool Task::operator==(std::shared_ptr<Task> &rec) const
{
    return this->sequence_ == rec->sequence_ && this->state_ == rec->state_ && this->priority_ == rec->priority_ &&
           this->revocable_ == rec->revocable_ && this->runnable_ == rec->runnable_ &&
           this->baseTaskDispatcher_ == rec->baseTaskDispatcher_;
}

}  // namespace AppExecFwk
}  // namespace OHOS
