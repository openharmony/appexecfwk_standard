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

#ifndef OHOS_APP_DISPATCHER_THREADING_DEFAULT_THREAD_FACTORY_H
#define OHOS_APP_DISPATCHER_THREADING_DEFAULT_THREAD_FACTORY_H

#include <atomic>

#include "thread_factory.h"
#include "app_log_wrapper.h"

namespace OHOS {
namespace AppExecFwk {
/**
 *  DefaultThreadFactory is a default thread producer.
 *
 */
class DefaultThreadFactory : public ThreadFactory {
public:
    DefaultThreadFactory();
    virtual ~DefaultThreadFactory()
    {}
    virtual std::shared_ptr<Thread> Create() override;

private:
    std::atomic_int index_;
};
}  // namespace AppExecFwk
}  // namespace OHOS
#endif
