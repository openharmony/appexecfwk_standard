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
#ifndef FOUNDATION_APPEXECFWK_SERVICES_FORMMGR_INCLUDE_FORM_CACHE_MGR_H
#define FOUNDATION_APPEXECFWK_SERVICES_FORMMGR_INCLUDE_FORM_CACHE_MGR_H

#include <map>
#include <memory>
#include <mutex>
#include <singleton.h>
#include <string>

namespace OHOS {
namespace AppExecFwk {
/**
 * @class FormCacheMgr
 * Form cache data manager.
 */
class FormCacheMgr final : public DelayedRefSingleton<FormCacheMgr> {
DECLARE_DELAYED_REF_SINGLETON(FormCacheMgr)
public:
    DISALLOW_COPY_AND_MOVE(FormCacheMgr);

    /**
     * @brief Get form data.
     * @param formId Form id.
     * @param data Cache data.
     * @return Returns true if this function is successfully called; returns false otherwise.
     */
    bool GetData(const int64_t formId, std::string &data) const;

    /**
     * @brief Add form data.
     * @param formId Form id.
     * @param data Cache data.
     * @return Returns true if this function is successfully called; returns false otherwise.
     */
    bool AddData(const int64_t formId, const std::string &data);

    /**
     * @brief Delete form data.
     * @param formId Form id.
     * @return Returns true if this function is successfully called; returns false otherwise.
     */
    bool DeleteData(const int64_t formId);

    /**
     * @brief update form data.
     * @param formId Form id.
     * @param data Cache data.
     * @return Returns true if this function is successfully called; returns false otherwise.
     */
    bool UpdateData(const int64_t formId, const std::string &data);
    /**
     * @brief Check if form data is exist or not.
     * @param formId, Form id.
     * @return Returns true if this function is successfully called; returns false otherwise.
     */
    bool IsExist(const int64_t formId) const;
private:
    mutable std::mutex cacheMutex_;
    std::map<int64_t, std::string> cacheData_;
};
}  // namespace AppExecFwk
}  // namespace OHOS

#endif // FOUNDATION_APPEXECFWK_SERVICES_FORMMGR_INCLUDE_FORM_CACHE_MGR_H
