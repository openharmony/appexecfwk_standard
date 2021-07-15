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
#ifndef OHOS_APPEXECFWK_DEFAULT_WORKER_POOL_CONFIG_H
#define OHOS_APPEXECFWK_DEFAULT_WORKER_POOL_CONFIG_H

#include "worker_pool_config.h"

namespace OHOS {
namespace AppExecFwk {
class DefaultWorkerPoolConfig final : public WorkerPoolConfig {
private:
    static constexpr int DEFAULT_MAX_THREAD_COUNT = 32;

    static constexpr int DEFAULT_CORE_THREAD_COUNT = 16;

    static constexpr long DEFAULT_KEEP_ALIVE_TIME = 50;

public:
    DefaultWorkerPoolConfig() = default;
    virtual ~DefaultWorkerPoolConfig() = default;

    int GetMaxThreadCount(void) const override;

    int GetCoreThreadCount(void) const override;

    long GetKeepAliveTime(void) const override;
};
}  // namespace AppExecFwk
}  // namespace OHOS
#endif
