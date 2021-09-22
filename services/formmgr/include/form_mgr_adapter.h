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

#ifndef FOUNDATION_APPEXECFWK_SERVICES_FORMMGR_INCLUDE_FORM_MGR_ADAPTER_H
#define FOUNDATION_APPEXECFWK_SERVICES_FORMMGR_INCLUDE_FORM_MGR_ADAPTER_H

#include <mutex>
#include <singleton.h>

#include "bundle_info.h"
#include "bundle_mgr_interface.h"
#include "form_info.h"
#include "form_host_record.h"
#include "form_item_info.h"
#include "form_js_info.h"
#include "form_provider_data.h"
#include "form_db_info.h"
#include "ipc_types.h"
#include "iremote_object.h"
#include "want.h"

namespace OHOS {
namespace AppExecFwk {
using Want = OHOS::AAFwk::Want;
using WantParams = OHOS::AAFwk::WantParams;
/**
 * @class FormMgrAdapter
 * Form request handler from form host.
 */
class FormMgrAdapter  final : public DelayedRefSingleton<FormMgrAdapter> {
DECLARE_DELAYED_REF_SINGLETON(FormMgrAdapter)
public:
    DISALLOW_COPY_AND_MOVE(FormMgrAdapter);

    /**
     * @brief Add form with want, send want to form manager service.
     * @param formId The Id of the forms to add.
     * @param want The want of the form to add.
     * @param callerToken Caller ability token.
     * @param formInfo Form info.
     * @return Returns ERR_OK on success, others on failure.
     */
    int AddForm(const int64_t formId, const Want &want, const sptr<IRemoteObject> &callerToken, FormJsInfo &formInfo);

    /**
     * @brief Delete forms with formIds, send formIds to form manager service.
     * @param formId The Id of the forms to delete.
     * @param callerToken Caller ability token.
     * @return Returns ERR_OK on success, others on failure.
     */
    int DeleteForm(const int64_t formId, const sptr<IRemoteObject> &callerToken);

    /**
     * @brief Release forms with formIds, send formIds to form Mgr service.
     * @param formId The Id of the forms to release.
     * @param callerToken Caller ability token.
     * @param delCache Delete Cache or not.
     * @return Returns ERR_OK on success, others on failure.
     */
    int ReleaseForm(const int64_t formId, const sptr<IRemoteObject> &callerToken, const bool delCache);

    /**
     * @brief Update form with formId, send formId to form manager service.
     * @param formId The Id of the form to update.
     * @param bundleName Provider ability bundleName.
     * @param formProviderData form provider data.
     * @return Returns ERR_OK on success, others on failure.
     */
    int UpdateForm(const int64_t formId, const std::string &bundleName, const FormProviderData &formProviderData);

    /**
     * @brief Request form with formId and want, send formId and want to form manager service.
     * 
     * @param formId The Id of the form to update.
     * @param callerToken Caller ability token.
     * @param want The want of the form to request.
     * @return Returns ERR_OK on success, others on failure.
     */
    int RequestForm(const int64_t formId, const sptr<IRemoteObject> &callerToken, const Want &want);

    /**
     * @brief Form visible/invisible notify, send formIds to form manager service.
     * 
     * @param formIds The vector of form Ids.
     * @param callerToken Caller ability token.
     * @param formVisibleType The form visible type, including FORM_VISIBLE and FORM_INVISIBLE.
     * @return Returns ERR_OK on success, others on failure.
     */
    ErrCode NotifyWhetherVisibleForms(const std::vector<int64_t> &formIds, const sptr<IRemoteObject> &callerToken, 
    const int32_t formVisibleType);
    
    /**
     * @brief temp form to normal form.
     * @param formId The Id of the form.
     * @param callerToken Caller ability token.
     * @return Returns ERR_OK on success, others on failure.
     */
    int CastTempForm(const int64_t formId, const sptr<IRemoteObject> &callerToken);

    /**
     * @brief Dump all of form storage infos.
     * @param formInfos All of form storage infos.
     * @return Returns ERR_OK on success, others on failure.
     */
    int DumpStorageFormInfos(std::string &formInfos) const;
    /**
     * @brief Dump form info by a bundle name.
     * @param bundleName The bundle name of form provider.
     * @param formInfos Form infos.
     * @return Returns ERR_OK on success, others on failure.
     */
    int DumpFormInfoByBundleName(const std::string &bundleName, std::string &formInfos) const;
    /**
     * @brief Dump form info by a bundle name.
     * @param formId The id of the form.
     * @param formInfo Form info.
     * @return Returns ERR_OK on success, others on failure.
     */
    int DumpFormInfoByFormId(const std::int64_t formId, std::string &formInfo) const;

    /**
     * @brief set next refresh time.
     * @param formId The id of the form.
     * @param nextTime next refresh time.
     * @return Returns ERR_OK on success, others on failure.
     */
    int SetNextRefreshTime(const int64_t formId, const int64_t nextTime);

    /**
     * @brief enable update form.
     * @param formIDs The id of the forms.
     * @param callerToken Caller ability token.
     * @return Returns ERR_OK on success, others on failure.
     */
    int EnableUpdateForm(const std::vector<int64_t> formIDs, const sptr<IRemoteObject> &callerToken);

    /**
     * @brief disable update form.
     * @param formIDs The id of the forms.
     * @param callerToken Caller ability token.
     * @return Returns ERR_OK on success, others on failure.
     */
    int DisableUpdateForm(const std::vector<int64_t> formIDs, const sptr<IRemoteObject> &callerToken);

    /**
     * @brief Process js message event.
     * @param formId Indicates the unique id of form.
     * @param want information passed to supplier.
     * @param callerToken Caller ability token.
     * @return Returns true if execute success, false otherwise.
     */
    int MessageEvent(const int64_t formId, const Want &want, const sptr<IRemoteObject> &callerToken);

    /**
     * @brief Acquire form data from form provider.
     * @param formId The Id of the from.
     * @param want The want of the request.
     * @param remoteObject Form provider proxy object.
     */
    void AcquireProviderFormInfo(const int64_t formId, const Want &want, const sptr<IRemoteObject> &remoteObject);
    /**
     * @brief Notify form provider for delete form.
     * @param formId The Id of the from.
     * @param want The want of the form.
     * @param remoteObject Form provider proxy object.
     * @return none.
     */
    void NotifyFormDelete(const int64_t formId, const Want &want, const sptr<IRemoteObject> &remoteObject);
private:
    /**
     * @brief Get form configure info.
     * @param want The want of the request.
     * @param formItemInfo Form configure info.
     * @return Returns ERR_OK on success, others on failure.
     */
    ErrCode GetFormConfigInfo(const Want& want, FormItemInfo &formItemInfo);
    /**
     * @brief Get bundle info.
     * @param want The want of the request.
     * @param bundleInfo Bundle info.
     * @param packageName Package name.
     * @return Returns ERR_OK on success, others on failure.
     */
    ErrCode GetBundleInfo(const AAFwk::Want &want, BundleInfo &bundleInfo, std::string &packageName);
    /**
     * @brief Get form info.
     * @param want The want of the request.
     * @param formInfo Form info.
     * @return Returns ERR_OK on success, others on failure.
     */
    ErrCode GetFormInfo(const AAFwk::Want &want, FormInfo &formInfo);
    /**
     * @brief Get form configure info.
     * @param want The want of the request.
     * @param bundleInfo Bundle info.
     * @param formInfo Form info.
     * @param formItemInfo Form configure info.
     * @return Returns ERR_OK on success, others on failure.
     */
    ErrCode GetFormItemInfo(const AAFwk::Want &want, const BundleInfo &bundleInfo, const FormInfo &formInfo, 
    FormItemInfo &formItemInfo);
    /**
     * @brief Dimension valid check.
     * @param formInfo Form info.
     * @param dimensionId Dimension id.
     * @return Returns true on success, false on failure.
     */    
    bool IsDimensionValid(const FormInfo &formInfo, int dimensionId) const;
    /**
     * @brief Create form configure info.
     * @param bundleInfo Bundle info.
     * @param formInfo Form info.
     * @param itemInfo Form configure info.
     * @return Returns ERR_OK on success, others on failure.
     */
    ErrCode CreateFormItemInfo(const BundleInfo& bundleInfo, const FormInfo& formInfo, FormItemInfo& itemInfo);
    /**
     * @brief Allocate form by formId.
     * @param info Form configure info.
     * @param callerToken Caller ability token.
     * @param wantParams WantParams of the request.
     * @param formInfo Form info for form host.
     * @return Returns ERR_OK on success, others on failure.
     */   
    ErrCode AllotFormById(const FormItemInfo &info, const sptr<IRemoteObject> &callerToken, 
    const WantParams &wantParams, FormJsInfo &formInfo);
    /**
     * @brief Allocate form by form configure info.
     * @param info Form configure info.
     * @param callerToken Caller ability token.
     * @param wantParams WantParams of the request.
     * @param formInfo Form info for form host.
     * @return Returns ERR_OK on success, others on failure.
     */   
    ErrCode AllotFormByInfo(const FormItemInfo &info, const sptr<IRemoteObject> &callerToken, 
    const WantParams& wantParams, FormJsInfo &formInfo);
    /**
     * @brief Acquire form data from form provider.
     * @param formId The Id of the form..
     * @param info Form configure info.
     * @param wantParams WantParams of the request.
     * @return Returns ERR_OK on success, others on failure.
     */  
    ErrCode AcquireProviderFormInfoAsync(const int64_t formId, const FormItemInfo &info, const WantParams &wantParams);

    /**
     * @brief Handle release form.
     * @param formId The form id.
     * @param callerToken Caller ability token.
     * @return Returns ERR_OK on success, others on failure.
     */
    ErrCode HandleReleaseForm(const int64_t formId, const sptr<IRemoteObject> &callerToken);

    /**
     * @brief Handle delete form.
     * @param formId The form id.
     * @param callerToken Caller ability token.
     * @return Returns ERR_OK on success, others on failure.
     */
    ErrCode HandleDeleteForm(const int64_t formId, const sptr<IRemoteObject> &callerToken);

    /**
     * @brief Handle delete temp form.
     * @param formId The form id.
     * @param callerToken Caller ability token.
     * @return Returns ERR_OK on success, others on failure.
     */
    ErrCode HandleDeleteTempForm(const int64_t formId, const sptr<IRemoteObject> &callerToken);

    /**
     * @brief Handle delete form storage.
     * @param dbRecord Form storage information.
     * @param uid calling user id.
     * @param formId The form id.
     * @return Function result and has other host flag.
     */
    ErrCode HandleDeleteFormCache(FormRecord &dbRecord, const int uid, const int64_t formId);

    /**
     * @brief Padding udid hash.
     * @param formId The form id.
     * @return Padded form id.
     */
    int64_t PaddingUDIDHash(const int64_t formId) const;

    /**
     * @brief Add existed form record.
     * @param info Form configure info.
     * @param callerToken Caller ability token.
     * @param record Form data.
     * @param formId The form id.
     * @param wantParams WantParams of the request.
     * @return Returns ERR_OK on success, others on failure.
     */
    ErrCode AddExistFormRecord(const FormItemInfo &info, const sptr<IRemoteObject> &callerToken,
        const FormRecord &record, const int64_t formId, const WantParams &wantParams);

    /**
     * @brief Add new form record.
     * @param info Form configure info.
     * @param formId The form id.
     * @param callerToken Caller ability token.
     * @param wantParams WantParams of the request.
     * @param formInfo Form info for form host.
     * @return Returns ERR_OK on success, others on failure.
     */
    ErrCode AddNewFormRecord(const FormItemInfo &info, const int64_t formId, 
        const sptr<IRemoteObject> &callerToken, const WantParams &wantParams, FormJsInfo &formInfo);

    /**
     * @brief Send event notify to form provider. The event notify type include FORM_VISIBLE and FORM_INVISIBLE.
     * 
     * @param providerKey The provider key string which consists of the provider bundle name and ability name.
     * @param formIdsByProvider The map of form Ids and their event type which have the same provider.
     * @param formVisibleType The form visible type, including FORM_VISIBLE and FORM_INVISIBLE.
     * @return Returns ERR_OK on success, others on failure.
     */
    ErrCode HandleEventNotify(const std::string &providerKey, const std::vector<int64_t> &formIdsByProvider,
        const int32_t formVisibleType);

    /**
     * @brief Increase the timer refresh count.
     * 
     * @param formId The form id.
     * @return none.
     */
    void IncreaseTimerRefreshCount(const int64_t formId);

    /**
     * @brief handle update form flag.
     * @param formIDs The id of the forms.
     * @param callerToken Caller ability token.
     * @param flag form flag.
     * @return Returns ERR_OK on success, others on failure.
     */
    int HandleUpdateFormFlag(const std::vector<int64_t> formIds, 
    const sptr<IRemoteObject> &callerToken, const bool flag);

    /**
     * @brief handle update form flag.
     * @param formIDs The id of the forms.
     * @param callerToken Caller ability token.
     * @param flag form flag.
     * @return Returns ERR_OK on success, others on failure.
     */
    bool IsFormCached(const FormRecord record);
    
    /**
     * @brief set next refresht time locked.
     * @param formId The form's id.
     * @param nextTime next refresh time.
     * @return Returns ERR_OK on success, others on failure.
     */
    int SetNextRefreshtTimeLocked(const int64_t formId, const int64_t nextTime);

    /**
     * @brief set next refresht time locked.
     * @param formId The form's id.
     * @param bundleName Provider ability bundleName.
     * @return Returns true or false.
     */
    bool IsUpdateValid(const int64_t formId, const std::string &bundleName);
    /**
     * @brief Handle cast temp form.
     * @param formId The form id.
     * @param record Form information.
     * @return Returns ERR_OK on success, others on failure.
     */
    ErrCode HandleCastTempForm(const int64_t formId, const FormRecord &record);

    /**
     * @brief Add form timer.
     * @param formRecord Form information.
     * @return Returns ERR_OK on success, others on failure.
     */
    ErrCode AddFormTimer(const FormRecord &formRecord);

    /**
     * @brief get bundleName.
     * @param bundleName for output.
     * @return Returns true on success, others on failure.
     */    
    bool GetBundleName(std::string &bundleName);
};
}  // namespace AppExecFwk
}  // namespace OHOS

#endif // FOUNDATION_APPEXECFWK_SERVICES_FORMMGR_INCLUDE_FORM_MGR_ADAPTER_H
