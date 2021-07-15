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
#ifndef FOUNDATION_APPEXECFWK_OHOS_PARALLEL_TASK_DISPATCHER_H
#define FOUNDATION_APPEXECFWK_OHOS_PARALLEL_TASK_DISPATCHER_H

#include <string>
#include <vector>
#include <list>
#include <iostream>
#include <assert.h>
#include "parallel_task_dispatcher_base.h"
#include "runnable.h"
#include "sync_task.h"
#include "task_priority.h"
#include "task_execute_interceptor.h"
#include "task_executor.h"
#include "barrier_handler.h"

namespace OHOS {
namespace AppExecFwk {
/**
 *  Customed parallel TaskDispatcher which means it can be created multi times.
 *
 *
 */
class ParallelTaskDispatcher : public ParallelTaskDispatcherBase {
public:
    // ParallelTaskDispatcher();

    ParallelTaskDispatcher(const std::string &name, TaskPriority priority, std::shared_ptr<TaskExecutor> &executor);
    ~ParallelTaskDispatcher(){};

    /**
     *  Set a barrier and meanwhile a sync task that execute after all tasks finished.
     *
     *  @param runnable  is the job to execute after barrier.
     *  @throws NullPointerException if |task| is null.
     *
     */
    ErrCode SyncDispatchBarrier(const std::shared_ptr<Runnable> &runnable);

    /**
     *  Set a barrier and meanwhile an async task that execute after all tasks finished.
     *
     *  @param runnable  is the job to execute after jobs in group.
     *  @throws NullPointerException if |task| is null.
     *
     */
    ErrCode AsyncDispatchBarrier(const std::shared_ptr<Runnable> &runnable);

protected:
    std::shared_ptr<TaskExecuteInterceptor> GetInterceptor();

private:
    static const std::string DISPATCHER_TAG;
    static const std::string ASYNC_DISPATCHER_BARRIER_TAG;
    static const std::string SYNC_DISPATCHER_BARRIER_TAG;
    std::shared_ptr<BarrierHandler> barrierHandler_ = nullptr;
};

}  // namespace AppExecFwk
}  // namespace OHOS
#endif