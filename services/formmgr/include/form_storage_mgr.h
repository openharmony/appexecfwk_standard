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

#ifndef FOUNDATION_APPEXECFWK_SERVICES_FORMMGR_INCLUDE_FORM_STORAGE_MGR_H
#define FOUNDATION_APPEXECFWK_SERVICES_FORMMGR_INCLUDE_FORM_STORAGE_MGR_H

#include <map>
#include <string>
#include <stdint.h>
#include <iostream>

#include "appexecfwk_errors.h"
#include "form_db_info.h"

namespace OHOS {
namespace AppExecFwk {
/**
 * @class FormStorageMgr
 * Form data storage.
 */
class FormStorageMgr {
public:
    FormStorageMgr() = default;
    virtual ~FormStorageMgr() = default;
    /**
     * @brief Load all form data from DB to innerFormInfos.
     * @param innerFormInfos Storage all form data.
     * @return Returns ERR_OK on success, others on failure.
     */
    ErrCode LoadFormData(std::vector<InnerFormInfo> &innerFormInfos) const;

    /**
     * @brief Get form data from DB to innerFormInfo with formId.
     * @param innerFormInfo Storage form data.
     * @return Returns ERR_OK on success, others on failure.
     */
    ErrCode GetStorageFormInfoById(const std::string &formId, InnerFormInfo &innerFormInfo) const;

    /**
     * @brief Save or update the form data in DB.
     * @param innerFormInfo Indicates the InnerFormInfo object to be save.
     * @return Returns ERR_OK on success, others on failure.
     */
    ErrCode SaveStorageFormInfo(const InnerFormInfo &innerFormInfo) const;

    /**
     * @brief Modify the form data in DB.
     * @param innerFormInfo Indicates the InnerFormInfo object to be Modify.
     * @return Returns ERR_OK on success, others on failure.
     */
    ErrCode ModifyStorageFormInfo(const InnerFormInfo &innerFormInfo) const;

    /**
     * @brief Delete the form data in DB.
     * @param formId The form data Id.
     * @return Returns ERR_OK on success, others on failure.
     */
    ErrCode DeleteStorageFormInfo(const std::string &formId) const;
};
}  // namespace AppExecFwk
}  // namespace OHOS

#endif // FOUNDATION_APPEXECFWK_SERVICES_FORMMGR_INCLUDE_FORM_STORAGE_MGR_H
