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

#ifndef FOUNDATION_APPEXECFWK_INTERFACES_INNERKITS_APPEXECFWK_BASE_INCLUDE_SERVICE_CENTER_CODE_H
#define FOUNDATION_APPEXECFWK_INTERFACES_INNERKITS_APPEXECFWK_BASE_INCLUDE_SERVICE_CENTER_CODE_H

namespace OHOS {
namespace AppExecFwk {
enum ServiceCenterFunction {
    SERVICE_CENTER_CONNECT_DISPATCHER_INFO = 1,
    SERVICE_CENTER_CONNECT_SILENT_INSTALL = 2,
    SERVICE_CENTER_CONNECT_UPGRADE_CHECK = 3,
    SERVICE_CENTER_CONNECT_UPGRADE_INSTALL = 4,
};

enum ServiceCenterResultCode {
    FREE_INSTALL_OK = 0,
    FREE_INSTALL_DOWNLOADING = 1,
};

enum ServiceCenterConnectState {
    SERVICE_CENTER_CONNECTED = 0,
    SERVICE_CENTER_CONNECTING = 1,
    SERVICE_CENTER_DISCONNECTED = 2,
};
}  // namespace AppExecFwk
}  // namespace OHOS
#endif  // FOUNDATION_APPEXECFWK_INTERFACES_INNERKITS_APPEXECFWK_BASE_INCLUDE_SERVICE_CENTER_CODE_H