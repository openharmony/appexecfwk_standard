/*
 * Copyright (c) 2021 Huawei Device Co., Ltd.
 * Licensed under the Apache License, Version 2.0 (the "License")_;
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

#ifndef FOUNDATION_APPEXECFWK_SERVICES_FORMMGR_INCLUDE_FORM_UTIL_H
#define FOUNDATION_APPEXECFWK_SERVICES_FORMMGR_INCLUDE_FORM_UTIL_H

#include "want.h"

namespace OHOS {
namespace AppExecFwk {
using Want = OHOS::AAFwk::Want;
/**
 * @class FormUtil
 * form utils.
 */
class FormUtil {
public:
    /**
     * @brief create want for form.
     * @param formName The name of the form.
     * @param specificationId specification id.
     * @param isTemporaryForm temporary form or not.
     * @param want The want of the form.
     */
    static void CreateFormWant(const std::string &formName, const int32_t specificationId, const bool isTemporaryForm,
    Want &want);
    /**
     * @brief create default want for form.
     * @param want The want of the form..
     * @param uri The uri.
     * @param userId user id.
     */
    static void CreateDefaultFormWant(Want &want, const std::string &uri, const int32_t userId);

    /**
     * @brief create udid for form.
     * @return udid.
     */
    static std::string GenerateUdid();

    /**
     * @brief create form id for form.
     * @param udidHash udid hash
     * @return new form id.
     */
    static int64_t GenerateFormId(int64_t udidHash);

    /**
     * @brief padding form id.
     * @param formId The id of the form.
     * @param udidHash udid hash.
     * @return new form id.
     */
    static int64_t PaddingUDIDHash(uint64_t formId, uint64_t udidHash);

    /**
     * @brief create udid hash.
     * @param udidHash udid hash.
     * @return Returns true on success, false on failure.
     */
    static bool GenerateUdidHash(int64_t &udidHash);
    /**
     * @brief Get current system nanosecond.
     * @return Current system nanosecond.
     */
    static long GetCurrentNanosecond();
    /**
     * @brief Get current system millisecond.
     * @return Current system millisecond.
     */
    static long GetCurrentMillisecond();
    /**
     * @brief Get millisecond from tm.
     * @param tmAtTime tm time.
     * @return Millisecond.
     */
    static long GetMillisecondFromTm(struct tm &tmAtTime);

    /**
     * @brief split string.
     * @param in string.
     * @param delim delimiter.
     * @return string list.
     */
    static std::vector<std::string> StringSplit(const std::string &in, const std::string &delim);
};
}  // namespace AppExecFwk
}  // namespace OHOS
#endif  // FOUNDATION_APPEXECFWK_SERVICES_FORMMGR_INCLUDE_FORM_UTIL_H