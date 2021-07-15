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
#ifndef TASK_DISPATCHER_TEST_INFO_H
#define TASK_DISPATCHER_TEST_INFO_H
namespace OHOS {
namespace AppExecFwk {
#include <string>

const std::string g_EVENT_REQU_FIRST = "requ_com_ohos_task_dispatcher_first";
const std::string g_EVENT_RESP_FIRST = "resp_com_ohos_task_dispatcher_first";
const std::string g_EVENT_REQU_SECOND = "requ_com_ohos_task_dispatcher_second";
const std::string g_EVENT_RESP_SECOND = "resp_com_ohos_task_dispatcher_second";
const std::string g_EVENT_REQU_FIRST_B = "requ_com_ohos_task_dispatcher_first_b";
const std::string g_EVENT_RESP_FIRST_B = "resp_com_ohos_task_dispatcher_first_b";
const std::string g_EVENT_REQU_SECOND_B = "requ_com_ohos_task_dispatcher_second_b";
const std::string g_EVENT_RESP_SECOND_B = "resp_com_ohos_task_dispatcher_second_b";

enum class TestFunc {
    GLOBAL,
    PARALLEL,
    SERIAL,
    SPEC,
    HYBRID,
    MULTI_APP,
    PRIORITY,
    CANCEL_TASK,
    EXTRA,
};

}  // namespace AppExecFwk
}  // namespace OHOS

#endif  // TASK_DISPATCHER_TEST_INFO_H