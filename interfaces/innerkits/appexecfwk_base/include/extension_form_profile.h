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

#ifndef FOUNDATION_APPEXECFWK_INTERFACES_INNERKITS_APPEXECFWK_BASE_INCLUDE_EXTENSION_FORM_PROFILE_H
#define FOUNDATION_APPEXECFWK_INTERFACES_INNERKITS_APPEXECFWK_BASE_INCLUDE_EXTENSION_FORM_PROFILE_H

#include <string>
#include <vector>
#include "appexecfwk_errors.h"
#include "extension_form_info.h"

namespace OHOS {
namespace AppExecFwk {
class ExtensionFormProfile {
public:
    /**
     * @brief Transform the form profile to ExtensionFormInfos.
     * @param formProfile Indicates the string of the form profile.
     * @param info Indicates the obtained ExtensionFormProfileInfo.
     * @return Returns ERR_OK if the information transformed successfully; returns error code otherwise.
     */
    static ErrCode TransformTo(const std::string &formProfile, std::vector<ExtensionFormInfo> &infos);
};

}  // namespace AppExecFwk
}  // namespace OHOS
#endif // FOUNDATION_APPEXECFWK_INTERFACES_INNERKITS_APPEXECFWK_BASE_INCLUDE_EXTENSION_FORM_PROFILE_H
