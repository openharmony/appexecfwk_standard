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
#ifndef FormManagerFuzzTest_H
#define FormManagerFuzzTest_H

#include <map>
#include <memory>
#include <string>
#include <unordered_map>

#include "nlohmann/json.hpp"
namespace OHOS {
namespace AppExecFwk {
class FuzzTestManager {
public:
    typedef std::shared_ptr<FuzzTestManager> Ptr;
    ~FuzzTestManager()
    {}
    static Ptr GetInstance()
    {
        if (fuzzTestInstance == nullptr) {
            fuzzTestInstance = std::shared_ptr<FuzzTestManager>(new FuzzTestManager);
        }
        return fuzzTestInstance;
    }

    void StartFuzzTest();

private:
    void SetJsonFunction(std::string);
    void SetExecutionTimes(uint16_t executionTimes);
    FuzzTestManager();
    FuzzTestManager(FuzzTestManager &) = delete;
    FuzzTestManager &operator=(const FuzzTestManager &) = delete;
    static Ptr fuzzTestInstance;
    uint16_t m_executionTimes {};
    std::unordered_map<std::string, int> remainderMap_ {};
    std::unordered_map<std::string, std::function<void()>> callFunctionMap_ {};

    void RegisterFormManagerAbility();
};
}  // namespace AppExecFwk
}  // namespace OHOS

#endif