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

#ifndef OHOS_APP_DISPATCHER_THREADING_WORKER_THREAD_H
#define OHOS_APP_DISPATCHER_THREADING_WORKER_THREAD_H

#include <atomic>

#include "thread_factory.h"
#include "task.h"
#include "app_log_wrapper.h"
#include "appexecfwk_errors.h"

namespace OHOS {
namespace AppExecFwk {
class WorkerThread;
/**
 *  Interface for a work delegate.
 */
class Delegate {
public:
    /**
     *  Do works for WorkerThread.
     *
     *  @param worker is the thread-holder.
     *
     */
    virtual ErrCode DoWorks(const std::shared_ptr<WorkerThread> &worker) = 0;
};

/**
 *  WorkerThread is a thread with a loop which can execute incoming tasks.
 */
class WorkerThread : public std::enable_shared_from_this<WorkerThread> {
public:
    /**
     *  The construct function of the WorkerThread class.
     *
     *  @param delegate delegates run works for this thread.
     *  @param firstTask if not null, take it as the first task in this thread. It can be null.
     *  @param factory can create a new thread which had some attributes set.
     */
    WorkerThread(const std::shared_ptr<Delegate> &delegate, const std::shared_ptr<Task> &firstTask,
        const std::shared_ptr<ThreadFactory> &factory);
    virtual ~WorkerThread(){};

    void CreateThread();

    /**
     *  Increase the task counter of self.
     *
     */
    void IncTaskCount(void);

    /**
     *  Gets the thread name.
     *
     *  @return The thread name.
     *
     */
    std::string GetThreadName(void);

    /**
     *  Get and remove |firstTask| of self.
     *
     *  @return the |firstTask|.
     *
     */
    std::shared_ptr<Task> PollFirstTask(void);

    /**
     *  Get count of tasks which were done.
     *
     *  @return the count of tasks which were done.
     *
     */
    long GetTaskCounter(void);

    std::shared_ptr<Thread> GetThread(void);

    /**
     *  join thread
     *
     */
    void Join();

protected:
    std::shared_ptr<Thread> thread_;
    std::atomic_long task_counter_;

private:
    std::shared_ptr<Task> first_task_;
    std::shared_ptr<Delegate> delegate_;
    std::shared_ptr<ThreadFactory> factory_;
};
}  // namespace AppExecFwk
}  // namespace OHOS
#endif
