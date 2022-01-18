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

#ifndef FOUNDATION_APPEXECFWK_SERVICES_FORMMGR_INCLUDE_FORM_TIMER_REFRESH_LIMITER_H
#define FOUNDATION_APPEXECFWK_SERVICES_FORMMGR_INCLUDE_FORM_TIMER_REFRESH_LIMITER_H

#include <map>
#include <mutex>
#include <vector>
#include "form_timer.h"

namespace OHOS {
namespace AppExecFwk {
/**
 * @class FormRefreshLimiter
 * Form refresh limite manager.
 */
class FormRefreshLimiter {
public:
    /**
     * @brief Add form limit info by formId.
     * @param formId The form id.
     * @return Returns true on success, false on failure.
     */
    bool AddItem(int64_t formId);
    /**
     * @brief Delete form limit info by formId.
     * @param formId The form id.
     */
    void DeleteItem(int64_t formId);
    /**
     * @brief Reset limit info.
     */
    void ResetLimit();
    /**
     * @brief Refresh enable or not.
     * @param formId The form id.
     * @return Returns ERR_OK on success, others on failure.
     */
    bool IsEnableRefresh(int64_t formId);
    /**
     * @brief Get refresh count.
     * @param formId The form id.
     * @return refresh count.
     */
    int GetRefreshCount(int64_t formId) const;
    /**
     * @brief Increase refresh count.
     * @param formId The form id.
     */
    void Increase(int64_t formId);
    /**
     * @brief Mark remind flag.
     * @param formId The form id.
     */
    void MarkRemind(int64_t formId);
    /**
     * @brief Get remind list.
     * @return remind list.
     */
    std::vector<int64_t> GetRemindList() const;
    /**
     * @brief Get remind list and reset limit.
     * @return remind list.
     */
    std::vector<int64_t> GetRemindListAndResetLimit();
    /**
     * @brief Get item count.
     * @return Item count.
     */
    int GetItemCount() const;
private:
    mutable std::mutex limiterMutex_;
    std::map<int64_t, LimitInfo> limiterMap_;
};
}  // namespace AppExecFwk
}  // namespace OHOS

#endif // FOUNDATION_APPEXECFWK_SERVICES_FORMMGR_INCLUDE_FORM_TIMER_REFRESH_LIMITER_H