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
#ifndef PERFORMANCE_CONFIG_PARSER_H
#define PERFORMANCE_CONFIG_PARSER_H

#include <fstream>
#include <iostream>
#include <string>

#include "nlohmann/json.hpp"

namespace OHOS {
namespace AppExecFwk {
const std::string PERFORMANCE_CONFIG_FILE_PATH{"./fms_performance_config.json"};

const std::string PERFORMANCE_EXECUTION_TIMES_KEY{"ExecutionTimes"};

struct PerformanceLevel {
    int32_t executionTimesLevel{1};
};

class PerformanceConfigParser {
public:
    void ParseForPerformance(const std::string &path, PerformanceLevel &stlevel)
    {
        std::ifstream jf(path);
        if (!jf.is_open()) {
            std::cout << "json file can not open!" << std::endl;
            return;
        }
        nlohmann::json jsonObj;
        jf >> jsonObj;
        const auto &jsonObjEnd = jsonObj.end();
        if (jsonObj.find(PERFORMANCE_EXECUTION_TIMES_KEY) != jsonObjEnd) {
            jsonObj.at(PERFORMANCE_EXECUTION_TIMES_KEY).get_to(stlevel.executionTimesLevel);
            if (stlevel.executionTimesLevel == 0) {
                stlevel.executionTimesLevel = 1;
            }
        }
    }
};
}  // namespace AppExecFwk
}  // namespace OHOS

#endif  // PERFORMANCE_CONFIG_PARSER_H
