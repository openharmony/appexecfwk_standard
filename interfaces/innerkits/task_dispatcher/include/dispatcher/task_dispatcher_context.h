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
#ifndef FOUNDATION_APPEXECFWK_OHOS_TASK_DISPATCHER_CONTEXT_H
#define FOUNDATION_APPEXECFWK_OHOS_TASK_DISPATCHER_CONTEXT_H

#include <string>
#include <vector>
#include <list>
#include <iostream>
#include <assert.h>
#include <map>
#include "default_worker_pool_config.h"
#include "global_task_dispatcher.h"
#include "parallel_task_dispatcher.h"
#include "serial_task_dispatcher.h"
#include "task_dispatcher.h"
#include "task_executor.h"
#include "task_priority.h"
#include "worker_pool_config.h"

namespace OHOS {
namespace AppExecFwk {
/**
 *  Dispatcher management for all kinds dispatchers and executor.
 *
 *
 */
class TaskDispatcherContext {
public:
    /**
     *  Constructs the object.
     *
     */
    TaskDispatcherContext();

    /**
     *  Constructs the object with parameter executor.
     *
     *  @param executor the TaskExecutor to set.
     *
     */
    TaskDispatcherContext(const std::shared_ptr<TaskExecutor> &executor);

    ~TaskDispatcherContext();

    /**
     *  Gets the worker pool configuration.
     *
     *  @return The worker pool configuration.
     *
     */
    std::shared_ptr<WorkerPoolConfig> GetWorkerPoolConfig() const;

    /**
     *  Gets WorkerThread information in WorkerPool.
     *
     *  @return The WorkerThread information in WorkerPool.
     *
     */
    std::map<std::string, long> GetWorkerThreadsInfo() const;

    /**
     *  Gets the serialDispatchers.
     *
     *  @return The serialDispatchers.
     *
     */
    std::map<std::shared_ptr<SerialTaskDispatcher>, std::string> GetSerialDispatchers() const;

    /**
     *  Gets waiting tasks count of TaskExecutor.
     *
     *  @return The waiting tasks count of TaskExecutor.
     *
     */
    int GetWaitingTasksCount() const;

    /**
     *  Gets finished tasks count of TaskExecutor.
     *
     *  @return The finished tasks count of TaskExecutor.
     *
     */
    long GetTaskCounter() const;

    /**
     *  Creates a serial dispatcher.
     *
     *  @param name The dispatcher name
     *  @param priority The priority of tasks
     *
     *  @return a new object of SerialTaskDispatcher
     *
     */
    std::shared_ptr<SerialTaskDispatcher> CreateSerialDispatcher(const std::string &name, TaskPriority priority);

    /**
     *  Creates a parallel dispatcher.
     *
     *  @param name The dispatcher name
     *  @param priority The priority of tasks
     *
     *  @return a new object of ParallelTaskDispatcher
     *
     */
    std::shared_ptr<ParallelTaskDispatcher> CreateParallelDispatcher(const std::string &name, TaskPriority priority);

    /**
     *  Gets the global task dispatcher.
     *
     *  @param priority The priority
     *
     *  @return A global task dispatcher which cannot be null.
     *
     */
    std::shared_ptr<TaskDispatcher> GetGlobalTaskDispatcher(TaskPriority priority);

    /**
     *  Shutdown all about task dispatcher.
     *
     *  @param force Indicate whether force close.
     *
     */
    ErrCode Shutdown(bool force);

private:
    static constexpr int HIGH_PRIORITY_INDEX = 0;

    static constexpr int DEFAULT_PRIORITY_INDEX = 1;

    static constexpr int LOW_PRIORITY_INDEX = 2;

    static constexpr int PRIORITY_COUNT = 3;

    mutable std::mutex mtx_;

    std::map<std::shared_ptr<SerialTaskDispatcher>, std::string> serialDispatchers_;

    std::vector<std::shared_ptr<TaskDispatcher>> globalDispatchers_;

    std::shared_ptr<WorkerPoolConfig> config_ = nullptr;

    std::shared_ptr<TaskExecutor> executor_ = nullptr;

private:
    int MapPriorityIndex(TaskPriority priority) const;
};

}  // namespace AppExecFwk
}  // namespace OHOS
#endif