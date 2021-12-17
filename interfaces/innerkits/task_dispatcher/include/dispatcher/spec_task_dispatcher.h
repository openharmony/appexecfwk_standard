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
#ifndef OHOS_APP_DISPATCHER_SPEC_TASK_DISPATCHER_H
#define OHOS_APP_DISPATCHER_SPEC_TASK_DISPATCHER_H

#include <string>
#include "base_task_dispatcher.h"
#include "task.h"
#include "sync_task.h"
#include "revocable.h"
#include "runnable.h"
#include "task/task_dispatcher_handler.h"
#include "event_runner.h"
#include "spec_dispatcher_config.h"

namespace OHOS {
namespace AppExecFwk {
class EventRunner;
class SpecTaskDispatcher : public BaseTaskDispatcher, public std::enable_shared_from_this<SpecTaskDispatcher> {
public:
    /**
     *  constructor for special task dispatchers
     *
     *  @param config which is the config of this dispatcher
     *  @param runner event runner
     *
     */
    SpecTaskDispatcher(std::shared_ptr<SpecDispatcherConfig> config, std::shared_ptr<EventRunner> runner);
    ~SpecTaskDispatcher(){};

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
     *  @param runnable is the job to execute
     *  @param delayMs indicate the delay time in milliseconds to execute
     *  @return an interface for revoke the task if it hasn't been invoked.
     *
     */
    std::shared_ptr<Revocable> DelayDispatch(const std::shared_ptr<Runnable> &runnable, long delayMs);

private:
    static std::string DISPATCHER_TAG;

    static std::string ASYNC_DISPATCHER_TAG;

    static std::string SYNC_DISPATCHER_TAG;

    static std::string DELAY_DISPATCHER_TAG;

    std::shared_ptr<TaskDispatcherHandler> handler_;
};
}  // namespace AppExecFwk
}  // namespace OHOS
#endif
