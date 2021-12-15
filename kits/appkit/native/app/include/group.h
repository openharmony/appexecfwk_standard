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
#ifndef OHOS_APP_DISPATCHER_GROUP_H
#define OHOS_APP_DISPATCHER_GROUP_H

namespace OHOS {
namespace AppExecFwk {
/**
 * Defines a group, which stands for a set of tasks semantically.
 * <p>Multiple tasks can be associated with a group so that they can be considered as a whole.
 * <p>This interface must be used with methods in {@link ohos.app.dispatcher.TaskDispatcher TaskDispatcher},
 * such as {@link ohos.app.dispatcher.TaskDispatcher#createDispatchGroup createDispatchGroup},
 * {@link ohos.app.dispatcher.TaskDispatcher#asyncGroupDispatch asyncGroupDispatch},
 * {@link ohos.app.dispatcher.TaskDispatcher#groupDispatchWait groupDispatchWait},
 * and {@link ohos.app.dispatcher.TaskDispatcher#groupDispatchNotify groupDispatchNotify}.
 * @since 1
 */
class Group {};
}  // namespace AppExecFwk
}  // namespace OHOS
#endif
