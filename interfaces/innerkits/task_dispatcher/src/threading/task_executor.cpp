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
#include "task_executor.h"

#include "app_log_wrapper.h"

namespace OHOS {
namespace AppExecFwk {
std::atomic<long> TaskExecutor::sequence(0);

TaskExecutor::TaskExecutor(const std::shared_ptr<WorkerPoolConfig> &config) : WorkerPool(config)
{
    terminated_ = false;
    taskCounter_ = 0;
    delayTasks_ = std::make_shared<DelayQueue>();
    pendingTasks_ = std::make_shared<BlockingQueue>();
}
TaskExecutor::~TaskExecutor()
{
    if ((consumer_) && consumer_->joinable()) {
        APP_LOGD("TaskExecutor::~TaskExecutor consumer is running");
        consumer_->join();
    }
    APP_LOGD("TaskExecutor::~TaskExecutor");
}

void TaskExecutor::Execute(const std::shared_ptr<Task> &task)
{
    APP_LOGD("TaskExecutor::Execute begin");
    task->SetSequence(GetAndIncrement(sequence));

    std::shared_ptr<TaskExecutor> executor = shared_from_this();

    if (AddWorker(executor, task) == false) {
        std::shared_ptr<PriorityTaskWrapper> priorityTaskWrapper =
            std::make_shared<PriorityTaskWrapper>(task->GetPriority(), task);
        if (pendingTasks_->Offer(priorityTaskWrapper) == false) {
            APP_LOGW("TaskExecutor::Execute rejected a task");
        }
    }
    APP_LOGD("TaskExecutor::Execute end");
}

ErrCode TaskExecutor::DoWorks(const std::shared_ptr<WorkerThread> &worker)
{
    APP_LOGD("TaskExecutor::DoWorks begin");
    if (worker == nullptr) {
        APP_LOGE("TaskExecutor::DoWorks worker is nullptr");
        return ERR_APPEXECFWK_CHECK_FAILED;
    }
    if (worker->GetThread() == nullptr) {
        APP_LOGE("TaskExecutor::DoWorks worker GetThread is nullptr");
        return ERR_APPEXECFWK_CHECK_FAILED;
    }

    std::shared_ptr<Task> task = worker->PollFirstTask();

    bool isInterrupted = false;
    bool done = false;
    while (((task != nullptr && done == false) || ((task = GetTask(worker)) != nullptr))) {
        APP_LOGD("TaskExecutor::DoWorks loop tasks.");

        BeforeRun(task);

        task->Run();

        AfterRun(task);

        worker->IncTaskCount();
        IncrementAndGet(taskCounter_);

        // loop condition
        done = true;
    }
    OnWorkerExit(worker, isInterrupted);
    APP_LOGD("TaskExecutor::DoWorks end");
    return ERR_OK;
}
std::shared_ptr<Task> TaskExecutor::GetTask(const std::shared_ptr<WorkerThread> &workerThread)
{
    bool isTimeout = false;
    std::shared_ptr<Task> nullRunnable = nullptr;
    std::shared_ptr<Task> next = nullptr;

    for (;;) {
        if (terminated_.load() && pendingTasks_->Empty()) {
            APP_LOGD("TaskExecutor::GetTask end: loop thread %{public}s is terminated",
                workerThread->GetThreadName().c_str());
            DecrementThread();
            return nullRunnable;
        }

        int workerCount = GetWorkCount();
        APP_LOGD("TaskExecutor::GetTask  workerCount:%{public}d, GetCoreThreadCount: %{public}d",
            workerCount,
            GetCoreThreadCount());
        bool needCheckTimeout = (workerCount > GetCoreThreadCount());
        if (isTimeout && needCheckTimeout && pendingTasks_->Empty()) {
            APP_LOGD("TaskExecutor::GetTask isTimeout is true");
            if (CompareAndDecNum(workerCount)) {
                APP_LOGD("TaskExecutor::GetTask end: loop thread %{public}s is timeout",
                    workerThread->GetThreadName().c_str());
                return nullRunnable;
            }
            continue;
        }

        APP_LOGD("TaskExecutor::GetTask need timeout=%{public}d", needCheckTimeout);
        std::shared_ptr<PriorityTaskWrapper> next =
            needCheckTimeout ? pendingTasks_->Poll(GetKeepAliveTime()) : pendingTasks_->Take();

        if (next != nullptr && next->task_ != nullptr) {
            APP_LOGD("TaskExecutor::GetTask end: loop thread %{public}s get next task",
                workerThread->GetThreadName().c_str());
            return next->task_;
        }
        isTimeout = true;
    }
}

void TaskExecutor::Terminate(bool force)
{
    APP_LOGD("TaskExecutor::Terminate begin");
    TerminateConsumer();
    ClosePool(force);
    APP_LOGD("TaskExecutor::Terminate end");
}

void TaskExecutor::AfterRun(const std::shared_ptr<Task> &task)
{
    task->AfterTaskExecute();
}

void TaskExecutor::BeforeRun(const std::shared_ptr<Task> &task)
{
    task->BeforeTaskExecute();
}

bool TaskExecutor::DelayExecute(const Runnable &task, long delayMs)
{
    if (delayMs <= 0) {
        task();
        APP_LOGD("TaskExecutor::DelayExecute end and delayMs less than 0");
        return true;
    }
    if (terminated_.load()) {
        APP_LOGI("TaskExecutor::DelayExecute end and terminate");
        return false;
    }
    std::shared_ptr<DelayTaskWrapper> delayTaskWrapper = std::make_shared<DelayTaskWrapper>(delayMs, task);
    if (delayTaskWrapper == nullptr) {
        APP_LOGI("TaskExecutor::DelayExecute end and delayTaskWrapper is nullptr");
        return false;
    }
    delayTasks_->Offer(delayTaskWrapper);
    return EnsureConsumeStarted();
}

void TaskExecutor::TerminateConsumer()
{

    std::unique_lock<std::mutex> lock(dataMutex_);
    terminated_.store(true);
    pendingTasks_->Stop();
    delayTasks_->Stop();

    if (consumer_ != nullptr) {
        if (consumer_->joinable()) {
            consumer_->join();
        }
        consumer_ = nullptr;
    }
}

bool TaskExecutor::EnsureConsumeStarted()
{
    if (consumer_ == nullptr) {
        {
            std::unique_lock<std::mutex> lock(dataMutex_);
            if (consumer_ == nullptr) {
                consumer_ = std::make_shared<std::thread>(&TaskExecutor::Consume, this);
                if (consumer_ == nullptr) {
                    APP_LOGD("TaskExecutor::EnsureConsumeStarted consumer_ is nullptr");
                    return false;
                }
                APP_LOGD("TaskExecutor::EnsureConsumeStarted start a delay task consumer");
            }
        }
    }
    return true;
}

void TaskExecutor::Consume()
{
    for (;;) {
        if (terminated_.load() && delayTasks_->Empty()) {
            APP_LOGD("TaskExecutor::Consume delay task is empty");
            break;
        }
        std::shared_ptr<DelayTaskWrapper> delayTaskWrapper = delayTasks_->Take();
        if (delayTaskWrapper == nullptr || delayTaskWrapper->runnable_ == nullptr) {
            APP_LOGD("TaskExecutor::Consume delayTaskWrapper is nullptr");
            return;
        };
        (delayTaskWrapper->runnable_)();
        APP_LOGD("TaskExecutor::Consume after run");
    }
}

int TaskExecutor::GetPendingTasksSize()
{
    return pendingTasks_->Size();
}

long TaskExecutor::GetTaskCounter()
{
    return taskCounter_.load();
}

long TaskExecutor::GetAndIncrement(std::atomic<long> &atomiclong)
{
    long ret = atomiclong.load();
    atomiclong.fetch_add(1, std::memory_order_relaxed);

    return ret;
}

long TaskExecutor::IncrementAndGet(std::atomic<long> &atomiclong)
{
    atomiclong.fetch_add(1, std::memory_order_relaxed);

    return atomiclong;
}
}  // namespace AppExecFwk
}  // namespace OHOS