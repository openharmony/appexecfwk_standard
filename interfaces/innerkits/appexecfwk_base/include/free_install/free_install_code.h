/*
 * Copyright (c) 2022 Huawei Device Co., Ltd.
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

#ifndef FOUNDATION_APPEXECFWK_INTERFACES_INNERKITS_APPEXECFWK_BASE_INCLUDE_FREE_INSTALL_CODE_H
#define FOUNDATION_APPEXECFWK_INTERFACES_INNERKITS_APPEXECFWK_BASE_INCLUDE_FREE_INSTALL_CODE_H

namespace OHOS {
namespace AppExecFwk {
enum FreeInstallErrorCode {
    FREE_INSTALL_CONNECT_ERROR = 0x600001,
    FREE_INSTALL_SERVICE_CENTER_CRASH = 0x600002,
    FREE_INSTALL_SERVICE_CENTER_TIMEOUT = 0x600003,
    FREE_INSTALL_UNDEFINED_ERROR = 0x600004,
};
}  // namespace AppExecFwk
}  // namespace OHOS
#endif  // FOUNDATION_APPEXECFWK_INTERFACES_INNERKITS_APPEXECFWK_BASE_INCLUDE_FREE_INSTALL_CODE_H