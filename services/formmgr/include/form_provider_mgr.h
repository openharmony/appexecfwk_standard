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

#ifndef FOUNDATION_APPEXECFWK_SERVICES_FORMMGR_INCLUDE_FORM_PROVIDER_MGRR_H
#define FOUNDATION_APPEXECFWK_SERVICES_FORMMGR_INCLUDE_FORM_PROVIDER_MGRR_H
#include <set>
#include <singleton.h>
#include "form_provider_info.h"
#include "form_record.h"
#include "want.h"

namespace OHOS {
namespace AppExecFwk {
using Want = OHOS::AAFwk::Want;
/**
 * @class FormProviderMgr
 * Form provider manager.
 */
class FormProviderMgr  final  : public DelayedRefSingleton<FormProviderMgr> {
    DECLARE_DELAYED_REF_SINGLETON(FormProviderMgr)
public:
    DISALLOW_COPY_AND_MOVE(FormProviderMgr);

    /**
     * @brief handle for acquire back event from provider.
     * @param formId The id of the form.
     * @param providerFormInfo provider form info.
     * @return Returns ERR_OK on success, others on failure.
     */
    ErrCode AcquireForm(const int64_t formId, const FormProviderInfo &formProviderInfo);
    /**
     * @brief handle for update form event from provider.
     * @param formId The id of the form.
     * @param providerFormInfo provider form info.
     * @return Returns ERR_OK on success, others on failure.
     */
    ErrCode UpdateForm(const int64_t formId, const FormProviderInfo &formProviderInfo);
    /**
     * handle for update form event from provider.
     *
     * @param formId The id of the form.
     * @param formRecord The form's record.
     * @param formProviderData provider form info.
     * @return Returns ERR_OK on success, others on failure.
     */
    ErrCode UpdateForm(const int64_t formId, FormRecord &formRecord, const FormProviderData &formProviderData);
    /**
     * @brief Refresh form.
     * 
     * @param formId The form id.
     * @param want The want of the form to request.
     * @return Returns ERR_OK on success, others on failure.
     */
    ErrCode RefreshForm(const int64_t formId, const Want &want);
    /**
     * @brief Connect ams for refresh form
     * 
     * @param formId The form id.
     * @param record Form data.
     * @param want The want of the form.
     * @param isTimerRefresh The flag of timer refresh.
     * @return Returns ERR_OK on success, others on failure.
     */
    ErrCode ConnectAmsForRefresh(const int64_t formId, const FormRecord &record, const Want &want, 
    const bool isTimerRefresh);
    /**
     * @brief Notify provider form delete.
     * @param formId The form id.
     * @param record Form information.
     * @return Returns ERR_OK on success, others on failure.
     */
    ErrCode NotifyProviderFormDelete(const int64_t formId, const FormRecord &formRecord);
    /**
     * @brief Notify provider forms batch delete.
     * @param bundleName BundleName.
     * @param bundleName AbilityName.
     * @param formIds form id list.
     * @return Returns ERR_OK on success, others on failure.
     */
    ErrCode NotifyProviderFormsBatchDelete(const std::string &bundleName, const std::string &abilityName, 
    const std::set<int64_t> &formIds);
    /**
     * @brief Process js message event.
     * @param formId Indicates the unique id of form.
     * @param record Form record.
     * @param want information passed to supplier.
     * @return Returns true if execute success, false otherwise.
     */
    int MessageEvent(const int64_t formId, const FormRecord &record, const Want &want);
private:
    FormRecord GetFormAbilityInfo(const FormRecord &record) const;
    /**
     * @brief Increase the timer refresh count.
     * 
     * @param formId The form id.
     */
    void IncreaseTimerRefreshCount(const int64_t formId);
    bool IsFormCached(const FormRecord &record);
};
}  // namespace AppExecFwk
}  // namespace OHOS

#endif // FOUNDATION_APPEXECFWK_SERVICES_FORMMGR_INCLUDE_FORM_PROVIDER_MGRR_H