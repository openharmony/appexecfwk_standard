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
#ifndef UNINSTALL_APP_TEST_CONFIG_PARSER_H
#define UNINSTALL_APP_TEST_CONFIG_PARSER_H

#include <fstream>
#include <iostream>
#include <string>

#include "nlohmann/json.hpp"

namespace OHOS {
namespace AppExecFwk {
const std::string UNINSTALL_TEST_CONFIG_FILE_PATH{"./fms_uninstall_app_test_config.json"};

const std::string UNINSTALL_TEST_EXECUTION_TIMES_KEY{"ExecutionTimes"};
const std::string UNINSTALL_TEST_SLEEP_TIME_KEY{"SleepTime"};

struct UninstallAPPTestLevel {
    int32_t executionTimesLevel{1};
    int32_t sleepTime{3};
};

class UninstallAPPTestConfigParser {
public:
    void ParseForUninstallAPPTest(const std::string &path, UninstallAPPTestLevel &stlevel)
    {
        std::ifstream jf(path);
        if (!jf.is_open()) {
            std::cout << "json file can not open!" << std::endl;
            return;
        }
        nlohmann::json jsonObj;
        jf >> jsonObj;
        const auto &jsonObjEnd = jsonObj.end();
        if (jsonObj.find(UNINSTALL_TEST_EXECUTION_TIMES_KEY) != jsonObjEnd) {
            jsonObj.at(UNINSTALL_TEST_EXECUTION_TIMES_KEY).get_to(stlevel.executionTimesLevel);
            if (stlevel.executionTimesLevel == 0) {
                stlevel.executionTimesLevel = 1;
            }
        }

        if (jsonObj.find(UNINSTALL_TEST_SLEEP_TIME_KEY) != jsonObjEnd) {
            jsonObj.at(UNINSTALL_TEST_SLEEP_TIME_KEY).get_to(stlevel.sleepTime);
        }
    }
};
}  // namespace AppExecFwk
}  // namespace OHOS

#endif  // UNINSTALL_APP_TEST_CONFIG_PARSER_H
