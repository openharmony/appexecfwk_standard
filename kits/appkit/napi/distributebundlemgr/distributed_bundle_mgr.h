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

#ifndef DISTRIBUTED_BUNDLE_MGR_H
#define DISTRIBUTED_BUNDLE_MGR_H
#include <vector>

#include "napi/native_api.h"
#include "napi/native_common.h"
#include "napi/native_node_api.h"
#include "application_info.h"
#include "element_name.h"
#include "remote_ability_info.h"

namespace OHOS {
namespace AppExecFwk {
struct ElementNameInfo {
    napi_env env = nullptr;
    napi_async_work asyncWork = nullptr;
    napi_deferred deferred = nullptr;
    napi_ref callbackRef = 0;
    OHOS::AppExecFwk::ElementName elementName;
    OHOS::AppExecFwk::RemoteAbilityInfo remoteAbilityInfo;
    int32_t errCode = 0;
    bool result = false;
    std::string errMssage;
};

struct ElementNameInfos {
    napi_env env = nullptr;
    napi_async_work asyncWork = nullptr;
    napi_deferred deferred = nullptr;
    napi_ref callbackRef = 0;
    std::vector<ElementName> elementNames;
    std::vector<RemoteAbilityInfo> remoteAbilityInfos;
    int32_t errCode = 0;
    bool result = false;
    std::string errMssage;
};

napi_value GetRemoteAbilityInfo(napi_env env, napi_callback_info info);
napi_value GetRemoteAbilityInfos(napi_env env, napi_callback_info info);
}  // namespace AppExecFwk
}  // namespace OHOS
#endif /* DISTRIBUTED_BUNDLE_MGR_H */