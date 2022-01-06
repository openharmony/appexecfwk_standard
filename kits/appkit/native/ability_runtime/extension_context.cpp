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

#include "extension_context.h"

#include "hilog_wrapper.h"

namespace OHOS {
namespace AbilityRuntime {
void ExtensionContext::SetToken(const sptr<IRemoteObject> &token)
{
    if (token == nullptr) {
        HILOG_DEBUG("ExtensionContext::SetToken failed, application is nullptr");
        return;
    }
    token_ = token;
}

sptr<IRemoteObject> ExtensionContext::GetToken() const
{
    return token_;
}
}  // namespace AbilityRuntime
}  // namespace OHOS