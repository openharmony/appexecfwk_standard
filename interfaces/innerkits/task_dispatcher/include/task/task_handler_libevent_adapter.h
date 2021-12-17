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

#ifndef OHOS_APP_DISPATCHER_TASK_HANDLER_LIBEVENT_ADAPTER_H
#define OHOS_APP_DISPATCHER_TASK_HANDLER_LIBEVENT_ADAPTER_H

#include <string>
#include "task_dispatcher_handler.h"
#include "runnable.h"
#include "event_runner.h"
#include "event_handler.h"
#include "event_queue.h"
namespace OHOS {
namespace AppExecFwk {
class TaskHandlerLibeventAdapter final : public EventHandler, public TaskDispatcherHandler {
public:
    TaskHandlerLibeventAdapter(const std::shared_ptr<EventRunner> &eventRunner) : EventHandler(eventRunner){};
    virtual ~TaskHandlerLibeventAdapter(){};
    bool Dispatch(const std::shared_ptr<Runnable> &runnable) override
    {
        APP_LOGI("TaskHandlerLibeventAdapter::Dispatch called.");
        return EventHandler::PostTask(*(runnable.get()), EventQueue::Priority::HIGH);
    }

    bool Dispatch(const std::shared_ptr<Runnable> &runnable, long delayMs) override
    {
        APP_LOGI("TaskHandlerLibeventAdapter::Delay Dispatch called.");
        return EventHandler::PostTask(*(runnable.get()), delayMs, EventQueue::Priority::HIGH);
    }
    bool DispatchSync(const std::shared_ptr<Runnable> &runnable) override
    {
        APP_LOGI("TaskHandlerLibeventAdapter::Delay Dispatch Sync called.");
        return EventHandler::PostSyncTask(*(runnable.get()), EventQueue::Priority::HIGH);
    }
};
}  // namespace AppExecFwk
}  // namespace OHOS

#endif
