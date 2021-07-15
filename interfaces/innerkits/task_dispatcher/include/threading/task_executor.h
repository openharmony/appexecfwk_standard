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
#ifndef OHOS_APPEXECFWK_TASK_EXECUTOR_H
#define OHOS_APPEXECFWK_TASK_EXECUTOR_H

#include <atomic>
#include <mutex>
#include <thread>

#include "appexecfwk_errors.h"

#include "blocking_queue.h"
#include "delay_execute_service.h"
#include "delay_queue.h"
#include "task.h"
#include "work_thread.h"
#include "worker_pool.h"

namespace OHOS {
namespace AppExecFwk {
class TaskExecutor : public WorkerPool,
                     public DelayExecuteService,
                     public Delegate,
                     public std::enable_shared_from_this<TaskExecutor> {
public:
    TaskExecutor(const std::shared_ptr<WorkerPoolConfig> &config);
    virtual ~TaskExecutor();

    void Execute(const std::shared_ptr<Task> &task);

    ErrCode DoWorks(const std::shared_ptr<WorkerThread> &worker) override;

    void Terminate(bool force);

    bool DelayExecute(const Runnable &runnable, long delayMs) override;

    int GetPendingTasksSize();

    long GetTaskCounter();

protected:
    void AfterRun(const std::shared_ptr<Task> &task) override;
    void BeforeRun(const std::shared_ptr<Task> &task) override;

private:
    std::shared_ptr<Task> GetTask(const std::shared_ptr<WorkerThread> &workerThread);

    void TerminateConsumer();

    bool EnsureConsumeStarted();

    void Consume();

    long GetAndIncrement(std::atomic<long> &atomiclong);

    long IncrementAndGet(std::atomic<long> &atomiclong);

private:
    static std::atomic<long> sequence;

    std::shared_ptr<DelayQueue> delayTasks_;

    std::shared_ptr<std::thread> consumer_;

    std::shared_ptr<BlockingQueue> pendingTasks_;

    std::atomic<bool> terminated_;

    std::atomic<long> taskCounter_;

    std::mutex dataMutex_;
};
}  // namespace AppExecFwk
}  // namespace OHOS
#endif