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

#ifndef OHOS_APP_DISPATCHER_THREADING_THREAD_FACTORY_H
#define OHOS_APP_DISPATCHER_THREADING_THREAD_FACTORY_H

#include "runnable.h"
#include <thread>
#include <memory>

namespace OHOS {
namespace AppExecFwk {

class Thread {
public:
    Thread() : thread_name_("Thread_0"), thread_(){};
    ~Thread() = default;
    Thread(const Thread &) = delete;
    Thread &operator=(const Thread &) = delete;

public:
    std::string thread_name_;
    std::shared_ptr<std::thread> thread_;
};

/**
 *  ThreadFactory is an interface for producing thread.
 */
class ThreadFactory {
public:
    ThreadFactory() = default;
    virtual ~ThreadFactory(){};
    /**
     *  Create a new Thread with |task| as the body of thread.
     *
     *  @param task The run body of the thread to create.
     *
     *  @return a new created thread. maybe null.
     *
     */
    virtual std::shared_ptr<Thread> Create() = 0;
};

}  // namespace AppExecFwk
}  // namespace OHOS
#endif
