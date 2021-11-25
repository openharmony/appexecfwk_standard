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
#include <cstdio>
#include <cstdlib>
#include <iostream>
#include <string>
#include <memory>
#include <regex>
#include <map>

#include "app_log_wrapper.h"
#include "securec.h"
#include "kernal_system_memory_Info.h"
#include "system_environment_information.h"

namespace OHOS {
namespace AppExecFwk {
namespace SystemEnv {
const int bytesKB = 1024;
void KernelSystemMemoryInfo::init(std::map<std::string, std::string> &memInfo)
{
    auto findData = [&] (const std::string& key) -> std::string {
        auto iter = memInfo.find(key);
        if (iter != memInfo.end()) {
            APP_LOGD("%{public}s, key[%{public}s] data[%{public}s]", __func__, key.c_str(), iter->second.c_str());
            return iter->second;
        } else {
            APP_LOGE("%{public}s, key[%{public}s]", __func__, key.c_str());
            return std::string("");
        }
    };

    memTotal_ = std::stoll(findData(std::string("MemTotal"))) * bytesKB;
    memFree_ = std::stoll(findData(std::string("MemFree"))) * bytesKB;
    memAvailable_ = std::stoll(findData(std::string("MemAvailable"))) * bytesKB;
    buffers_ = std::stoll(findData(std::string("Buffers"))) * bytesKB;
    cached_ = std::stoll(findData(std::string("Cached"))) * bytesKB;
    swapCached_ = std::stoll(findData(std::string("SwapCached"))) * bytesKB;
}

int64_t KernelSystemMemoryInfo::GetMemTotal()
{
    return memTotal_;
}

int64_t KernelSystemMemoryInfo::GetMemFree()
{
    return memFree_;
}

int64_t KernelSystemMemoryInfo::GetMemAvailable()
{
    return memAvailable_;
}

int64_t KernelSystemMemoryInfo::GetBuffers()
{
    return buffers_;
}

int64_t KernelSystemMemoryInfo::GetCached()
{
    return cached_;
}

int64_t KernelSystemMemoryInfo::GetSwapCached()
{
    return swapCached_;
}

static void RequestSystemMemoryInfo(std::map<std::string, std::string> &memInfo)
{
    std::regex rLabel("[\\w()]+");
    std::regex rData("\\d+");
    const int buffsize = 1024;
    char buff[buffsize] = {0};

    FILE *fp = popen("cat /proc/meminfo", "r");
    if (fp == nullptr) {
        APP_LOGE("%{public}s, open meminfo failed", __func__);
        return;
    }

    while (fgets(buff, sizeof(buff), fp) != nullptr) {
        std::string strbuf(buff);
        memset_s(buff, sizeof(buff), 0x00, sizeof(buff));
        std::smatch sm;
        std::smatch smData;
        bool flag = false;
        flag = std::regex_search(strbuf, sm, rLabel);
        if (!flag) {
            APP_LOGE("%{public}s, open meminfo failed", __func__);
            continue;
        }
        std::string strLabel = sm[0];
        strbuf = sm.suffix().str();
        flag = std::regex_search(strbuf, sm, rData);
        if (!flag) {
            APP_LOGE("%{public}s, open meminfo failed", __func__);
            continue;
        }
        std::string strData = sm[0];
        memInfo[strLabel] = strData;
    }

    pclose(fp);
    fp = nullptr;
}

void GetMemInfo(KernelSystemMemoryInfo &memInfo)
{
    std::map<std::string, std::string> memListInfo;
    RequestSystemMemoryInfo(memListInfo);
    memInfo.init(memListInfo);
}
}  // namespace SystemEnv
}  // namespace AppExecFwk
}  // namespace OHOS
