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
#ifndef FOUNDATION_APPEXECFWK_SERVICES_KERNAL_SYSTEM_MEMORY_INFO_H
#define FOUNDATION_APPEXECFWK_SERVICES_KERNAL_SYSTEM_MEMORY_INFO_H

#include <map>

namespace OHOS {
namespace AppExecFwk {
namespace SystemEnv {
class kernal_system_memory_Info {
public:
    kernal_system_memory_Info() = default;
    ~kernal_system_memory_Info() = default;
};

class KernelSystemMemoryInfo {
public:
    KernelSystemMemoryInfo() = default;
    ~KernelSystemMemoryInfo() = default;

    void init(std::map<std::string, std::string> &memInfo);

    int64_t GetMemTotal();
    int64_t GetMemFree();
    int64_t GetMemAvailable();
    int64_t GetBuffers();
    int64_t GetCached();
    int64_t GetSwapCached();

private:
    int64_t memTotal_ = 0;
    int64_t memFree_ = 0;
    int64_t memAvailable_ = 0;
    int64_t buffers_ = 0;
    int64_t cached_ = 0;
    int64_t swapCached_ = 0;
};
}  // namespace SystemEnv
}  // namespace AppExecFwk
}  // namespace OHOS
#endif // FOUNDATION_APPEXECFWK_SERVICES_KERNAL_SYSTEM_MEMORY_INFO_H