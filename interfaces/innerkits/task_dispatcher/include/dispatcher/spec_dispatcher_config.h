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
#ifndef OHOS_APP_DISPATCHER_SPEC_DISPATCHER_CONFIG_H
#define OHOS_APP_DISPATCHER_SPEC_DISPATCHER_CONFIG_H

#include <string>
#include "task_priority.h"

namespace OHOS {
namespace AppExecFwk {
class SpecDispatcherConfig {
public:
    static constexpr const char *MAIN = "DispatcherMain";
    static constexpr const char *UI = "DispatcherUI";

public:
    SpecDispatcherConfig(std::string name, TaskPriority priority)
    {
        name_ = name;
        priority_ = priority;
    }
    ~SpecDispatcherConfig(){};
    std::string GetName()
    {
        return name_;
    }

    TaskPriority GetPriority()
    {
        return priority_;
    }

private:
    std::string name_;
    TaskPriority priority_;
};
}  // namespace AppExecFwk
}  // namespace OHOS

#endif
