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
#ifndef FOUNDATION_APPEXECFWK_OHOS_GLOBAL_TASK_DISPATCHER_H
#define FOUNDATION_APPEXECFWK_OHOS_GLOBAL_TASK_DISPATCHER_H

#include <string>
#include "parallel_task_dispatcher_base.h"
#include "task_executor.h"
#include "task_priority.h"

namespace OHOS {
namespace AppExecFwk {
/**
 *  Dispatcher for global thread model.
 *
 *
 */
class GlobalTaskDispatcher : public ParallelTaskDispatcherBase {
private:
    static std::string DISPATCHER_NAME_;

public:
    GlobalTaskDispatcher(TaskPriority priority, std::shared_ptr<TaskExecutor> &executor);
    ~GlobalTaskDispatcher(){};
};

}  // namespace AppExecFwk
}  // namespace OHOS
#endif
