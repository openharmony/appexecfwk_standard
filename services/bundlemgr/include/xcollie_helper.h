/*
 * Copyright (c) 2022-2022 Huawei Device Co., Ltd.
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

#ifndef FOUNDATION_APPEXECFWK_SERVICES_BUNDLEMGR_INCLUDE_XCOLLIE_HELPER_H
#define FOUNDATION_APPEXECFWK_SERVICES_BUNDLEMGR_INCLUDE_XCOLLIE_HELPER_H

#include <string>

namespace OHOS {
namespace AppExecFwk {

class XcollieHelper {
public:
    // set timer
    // name : timer name
    // timeout : timeout, unit s
    // func : callback
    // arg : the callback's param
    // flag : timer timeout operation. the value can be:
    //                               XCOLLIE_FLAG_DEFAULT :do all callback function
    //                               XCOLLIE_FLAG_NOOP  : do nothing but the caller defined function
    //                               XCOLLIE_FLAG_LOG :  generate log file
    //                               XCOLLIE_FLAG_RECOVERY  : die when timeout
    // return: the timer id
    static int SetTimer(const std::string &name, unsigned int timeout, std::function<void (void *)> func, void *arg);

    // cancel timer
    // id: timer id
    static void CancelTimer(int id);
};

}  // namespace AppExecFwk
}  // namespace OHOS
#endif  // FOUNDATION_APPEXECFWK_SERVICES_BUNDLEMGR_INCLUDE_XCOLLIE_HELPER_H
