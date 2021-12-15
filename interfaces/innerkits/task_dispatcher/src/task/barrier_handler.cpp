/*
 * Copyright (c) 2021 Huawei Device Co., Ltd.
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 * http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */
#include "barrier_handler.h"
#include "app_log_wrapper.h"

namespace OHOS {
namespace AppExecFwk {
BarrierHandler::BarrierHandler(const std::shared_ptr<TaskExecutor> &executor)
{
    executor_ = executor;
};

ErrCode BarrierHandler::AddBarrier(std::shared_ptr<Task> &barrierTask)
{
    APP_LOGI("BarrierHandler::AddBarrier start");
    if (ListenToTask(barrierTask) != ERR_OK) {
        APP_LOGE("BarrierHandler::AddBarrier listenToTask failed");
        return ERR_APPEXECFWK_CHECK_FAILED;
    };

    bool execNow = false;
    {
        std::unique_lock<std::mutex> lock(barrierLock_);
        std::shared_ptr<BarrierPair> pair = barrierQueue_.size() == 0 ? nullptr : barrierQueue_.back();
        if ((pair == nullptr) || ((!HasTask(pair->tasks_)) && (pair->barrier_ == nullptr))) {
            execNow = true;
            APP_LOGI("BarrierHandler::AddBarrier need execute now");
        }
        if ((pair == nullptr) || (pair->barrier_ != nullptr)) {
            std::set<std::shared_ptr<Task>> tmp;
            std::shared_ptr<BarrierPair> barrierPair = std::make_shared<BarrierPair>(tmp, barrierTask);
            if (barrierPair == nullptr) {
                APP_LOGE("BarrierHandler::AddBarrier barrierPair is nullptr");
                return ERR_APPEXECFWK_CHECK_FAILED;
            }
            barrierQueue_.push_back(barrierPair);
            APP_LOGI("BarrierHandler::AddBarrier barrierQueue push barrierPair");
        } else {
            pair->barrier_ = barrierTask;
        }
    }

    if (execNow) {
        APP_LOGI("BarrierHandler::AddBarrier execute task");
        executor_->Execute(barrierTask);
    }
    APP_LOGI("BarrierHandler::AddBarrier end");
    return ERR_OK;
}

ErrCode BarrierHandler::Intercept(std::shared_ptr<Task> &task)
{
    APP_LOGI("BarrierHandler::Intercept start");
    if (ListenToTask(task) != ERR_OK) {
        APP_LOGE("BarrierHandler::Intercept listenToTask failed");
        return ERR_APPEXECFWK_CHECK_FAILED;
    };

    // afterBarrier means is intercepted.
    bool intercepted = AddTaskAfterBarrier(task);
    if (intercepted) {
        APP_LOGI("BarrierHandler::Intercept intercepted a task.");
    }
    ErrCode result = intercepted ? ERR_APPEXECFWK_INTERCEPT_TASK_EXECUTE_SUCCESS : ERR_APPEXECFWK_CHECK_FAILED;
    APP_LOGI("BarrierHandler::Intercept end, result:%{public}d", result);
    return result;
}

ErrCode BarrierHandler::ListenToTask(std::shared_ptr<Task> &task)
{
    APP_LOGI("BarrierHandler::ListenToTask start");
    std::shared_ptr<MyTaskListener> ptrlistener = std::make_shared<MyTaskListener>();
    if (ptrlistener == nullptr) {
        APP_LOGE("BarrierHandler::listenToTask make shared MyTaskListener is nullptr");
        return ERR_APPEXECFWK_CHECK_FAILED;
    }
    const std::function<void()> onTaskDone = std::bind(&BarrierHandler::OnTaskDone, this, task);
    ptrlistener->Callback(onTaskDone);
    task->AddTaskListener(ptrlistener);
    APP_LOGI("BarrierHandler::ListenToTask end");
    return ERR_OK;
}

void BarrierHandler::OnTaskDone(std::shared_ptr<Task> &task)
{
    APP_LOGI("BarrierHandler::OnTaskDone start");
    // remove from head of queue.
    // Under the premise that task cannot be reused.
    bool removed = false;
    {
        std::unique_lock<std::mutex> lock(barrierLock_);
        std::shared_ptr<BarrierPair> barrierPair = barrierQueue_.size() == 0 ? nullptr : barrierQueue_.front();
        if (barrierPair != nullptr) {
            if (HasTask(barrierPair->tasks_)) {
                removed = barrierPair->tasks_.erase(task) == 0 ? false : true;
                if (barrierPair->tasks_.empty() && (barrierPair->barrier_ != nullptr)) {
                    APP_LOGI("Barrier.onTaskDone execute barrier task after task done.");
                    executor_->Execute(barrierPair->barrier_);
                }
            } else if (task == (barrierPair->barrier_)) {
                APP_LOGI("Barrier.onTaskDone remove a barrier.");
                barrierPair->barrier_ = nullptr;
                removed = true;
                // Driven to next barrier.
                // In one case (barrierQueue.size() == 1): call barrier, and no more task incoming.
                if (barrierQueue_.size() > 1) {
                    barrierQueue_.pop_front();
                    std::shared_ptr<BarrierPair> nextPair = barrierQueue_.front();
                    if (HasTask(nextPair->tasks_)) {
                        for (std::set<std::shared_ptr<Task>>::iterator it = nextPair->tasks_.begin();
                             it != nextPair->tasks_.end();
                             it++) {
                            executor_->Execute(*it);
                        }
                    } else if (nextPair->barrier_ != nullptr) {
                        APP_LOGI("Barrier.onTaskDone execute barrier task after barrier done.");
                        executor_->Execute(nextPair->barrier_);
                    } else {
                        APP_LOGW("Barrier.onTaskDone: Detected an empty node.");
                    }
                }
            }
        }
    }

    if (!removed) {
        APP_LOGI("Barrier.onTaskDone: Task remove failed.");
    }
    APP_LOGI("BarrierHandler::OnTaskDone end");
}

bool BarrierHandler::AddTaskAfterBarrier(std::shared_ptr<Task> &task)
{
    APP_LOGI("BarrierHandler::AddTaskAfterBarrier start");
    std::unique_lock<std::mutex> lock(barrierLock_);
    std::shared_ptr<BarrierPair> pair = barrierQueue_.size() == 0 ? nullptr : barrierQueue_.back();
    if ((pair == nullptr) || (pair->barrier_ != nullptr)) {
        std::shared_ptr<BarrierPair> tmp = std::make_shared<BarrierPair>(CreateTaskSet(task), nullptr);
        if (tmp == nullptr) {
            APP_LOGE("BarrierHandler::addTaskAfterBarrier make shared BarrierPair is nullptr");
            return false;
        }

        barrierQueue_.push_back(tmp);
    } else if (pair->tasks_.empty()) {
        pair->tasks_ = CreateTaskSet(task);
    } else {
        pair->tasks_.insert(task);
    }
    APP_LOGI("BarrierHandler::AddTaskAfterBarrier end");
    return (barrierQueue_.size() > 1);
}

bool BarrierHandler::HasTask(const std::set<std::shared_ptr<Task>> &tasks)
{
    return ((tasks.size() != 0) && !tasks.empty());
}

std::set<std::shared_ptr<Task>> BarrierHandler::CreateTaskSet(std::shared_ptr<Task> &firstTask)
{
    std::set<std::shared_ptr<Task>> taskSet;
    taskSet.insert(firstTask);
    return taskSet;
}
}  // namespace AppExecFwk
}  // namespace OHOS