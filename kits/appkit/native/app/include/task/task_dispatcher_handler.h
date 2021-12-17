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
#ifndef OHOS_APP_DISPATCHER_THREADING_TASK_DISPATCHER_HANDLER_H
#define OHOS_APP_DISPATCHER_THREADING_TASK_DISPATCHER_HANDLER_H

#include <memory>
#include "task.h"
namespace OHOS {
namespace AppExecFwk {
/**
 * Interface for handling post a task.
 *
 */
class TaskDispatcherHandler {
public:
    /**
     * Dispatch a task asynchronously.
     *
     * @param task The task to post.
     * @return true if successfully post a task, otherwise false.
     */
    virtual bool Dispatch(const std::shared_ptr<Runnable> &runnable) = 0;
    /**
     * Dispatch a task synchronously.
     *
     * @param task The task to post.
     * @return true if successfully post a task, otherwise false.
     */
    virtual bool DispatchSync(const std::shared_ptr<Runnable> &runnable) = 0;

    /**
     * Dispatch a task asynchronously at specific time point.
     *
     * @param task The task to post.
     * @param delayMs The delay time expected to post the task, in milli second.
     * @return true if successfully post a task, otherwise false.
     */
    virtual bool Dispatch(const std::shared_ptr<Runnable> &runnable, long delayMs) = 0;
};
}  // namespace AppExecFwk
}  // namespace OHOS

#endif
