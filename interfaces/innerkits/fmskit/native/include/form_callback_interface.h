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
#ifndef FOUNDATION_APPEXECFWK_OHOS_FORM_ABILITY_INTERFACE_H
#define FOUNDATION_APPEXECFWK_OHOS_FORM_ABILITY_INTERFACE_H

#include "form_js_info.h"

namespace OHOS {
namespace AppExecFwk {
class FormCallbackInterface {
public:
    /**
     * @brief Update form.
     *
     * @param formJsInfo Indicates the obtained {@code FormJsInfo} instance.
     */
    virtual void ProcessFormUpdate(const FormJsInfo &formJsInfo) = 0;

    /**
     * @brief Uninstall form.
     *
     * @param formId Indicates the ID of the form to uninstall.
     */
    virtual void ProcessFormUninstall(const int64_t formId) = 0;

    /**
     * @brief Form service death event.
     *
     */
    virtual void OnDeathReceived() = 0;
};
}  // namespace AppExecFwk
}  // namespace OHOS
#endif  // FOUNDATION_APPEXECFWK_OHOS_FORM_ABILITY_INTERFACE_H
