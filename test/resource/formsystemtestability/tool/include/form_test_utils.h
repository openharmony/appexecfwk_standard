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
#ifndef _FORM_TEST_UTILS_H_
#define _FORM_TEST_UTILS_H_
#include "ability_info.h"
#include "ability_lifecycle.h"
#include "application_info.h"
#include "process_info.h"
#include "want.h"

namespace OHOS {
namespace AppExecFwk {
const int OnStateChangedEventWant = LifeCycle::Event::UNDEFINED;
const int OnStateChangedEvent = (int)LifeCycle::Event::UNDEFINED + 1;
const int requestCodeForTerminate = 10;
const int requestCodeForResult = 20;

class FormTestUtils {
public:
    FormTestUtils() = default;
    virtual ~FormTestUtils() = default;
    static bool PublishEvent(const std::string &eventName, const int &code, const std::string &data);
    static Want MakeWant(std::string deviceId, std::string abilityName, std::string bundleName,
        std::map<std::string, std::string> params);
};
}  // namespace AppExecFwk
}  // namespace OHOS
#endif  // _FORM_TEST_UTILS_H_