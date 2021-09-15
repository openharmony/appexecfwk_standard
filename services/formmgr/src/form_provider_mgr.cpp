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

#include <cinttypes>

#include "appexecfwk_errors.h"
#include "app_log_wrapper.h"
#include "form_ams_helper.h"
#include "form_batch_delete_connection.h"
#include "form_cache_mgr.h"
#include "form_constants.h"
#include "form_data_mgr.h"
#include "form_delete_connection.h"
#include "form_msg_event_connection.h"
#include "form_provider_mgr.h"
#include "form_record.h"
#include "form_refresh_connection.h"
#include "form_timer_mgr.h"
#include "power_mgr_client.h"

namespace OHOS {
namespace AppExecFwk {
FormProviderMgr::FormProviderMgr(){}
FormProviderMgr::~FormProviderMgr(){}
/**
 * @brief handle for acquire back from ams.
 * @param formId The id of the form.
 * @param formProviderInfo provider form info.
 * @return Returns ERR_OK on success, others on failure.
 */
ErrCode FormProviderMgr::AcquireForm(const int64_t formId, const FormProviderInfo &formProviderInfo)
{
    APP_LOGD("%{public}s start, formId:%{public}" PRId64 "", __func__, formId);

    if (formId <= 0) {
        APP_LOGE("%{public}s fail, formId should be greater than 0", __func__);
        return ERR_FORM_INVALID_PARAM;
    }

    FormRecord formRecord;
    bool isGetFormRecord = FormDataMgr::GetInstance().GetFormRecord(formId, formRecord);
    if (!isGetFormRecord) {
        APP_LOGE("%{public}s fail, not exist such form, formId:%{public}" PRId64 "", __func__, formId);
        return ERR_APPEXECFWK_FORM_INFO_NOT_EXIST;
    }
    
    FormHostRecord clientHost;
    bool isGetFormHostRecord = FormDataMgr::GetInstance().GetFormHostRecord(formId, clientHost);
    if (!isGetFormHostRecord) {
        APP_LOGE("%{public}s fail, clientHost is null", __func__);
        return ERR_APPEXECFWK_FORM_HOST_INFO_NOT_EXIST;
    }

    if (formRecord.isInited) {
        if (IsFormCached(formRecord)) {
            if (clientHost.Contains(formId)) {
                formRecord.formProviderInfo = formProviderInfo;
                clientHost.OnAcquire(formId, formRecord);
            }
        } else {
            Want want;
            RefreshForm(formId, want);
        }
        return ERR_OK;
    }
    formRecord.isInited = true;
    formRecord.needRefresh = false;
    FormDataMgr::GetInstance().SetFormCacheInited(formId, true);
    
    if (clientHost.Contains(formId)) {
        formRecord.formProviderInfo = formProviderInfo;
        clientHost.OnAcquire(formId, formRecord);
    }
    
    // we do not cache when data size is over 1k
    std::string jsonData = formProviderInfo.GetFormDataString(); // get json data
    APP_LOGD("%{public}s , jsonData is %{public}s.",  __func__, jsonData.c_str());
    if (jsonData.size() <= Constants::MAX_FORM_DATA_SIZE) {
        APP_LOGW("%{public}s, acquire js card, cache the card", __func__);
        FormCacheMgr::GetInstance().AddData(formId, formProviderInfo.GetFormDataString());
    }
    return ERR_OK;
}

/**
 * @brief Refresh form.
 * 
 * @param formId The form id.
 * @param want The want of the form to request.
 * @return Returns ERR_OK on success, others on failure.
 */
ErrCode FormProviderMgr::RefreshForm(const int64_t formId, const Want &want)
{
    APP_LOGI("%{public}s called, formId:%{public}" PRId64 ".", __func__, formId);
    FormRecord record;
    bool bGetRecord = FormDataMgr::GetInstance().GetFormRecord(formId, record);
    if (!bGetRecord) {
        APP_LOGE("%{public}s fail, not exist such form:%{public}" PRId64 "", __func__, formId);
        return ERR_APPEXECFWK_FORM_INFO_NOT_EXIST;
    }

    bool isTimerRefresh = want.GetBoolParam(Constants::KEY_IS_TIMER, false);
    Want newWant(want);
    newWant.RemoveParam(Constants::KEY_IS_TIMER);    

    if (isTimerRefresh) {
        FormDataMgr::GetInstance().SetCountTimerRefresh(formId, true);
    }

    bool screenOnFlag = PowerMgr::PowerMgrClient::GetInstance().IsScreenOn();
    if (!screenOnFlag) {
        FormDataMgr::GetInstance().SetNeedRefresh(formId, true);
        APP_LOGE("%{public}s fail, screen off, set refresh flag, do not refresh now", __func__);
        return ERR_OK;
    }

    bool needRefresh = FormDataMgr::GetInstance().IsEnableRefresh(formId);
    if (!needRefresh) {
        FormDataMgr::GetInstance().SetNeedRefresh(formId, true);
        APP_LOGE("%{public}s fail, no one needReresh, set refresh flag, do not refresh now", __func__);
        return ERR_OK;
    }

    FormRecord refreshRecord = GetFormAbilityInfo(record);
    refreshRecord.isInited = record.isInited;
    refreshRecord.versionUpgrade = record.versionUpgrade;
    refreshRecord.isCountTimerRefresh = isTimerRefresh;
    return ConnectAmsForRefresh(formId, refreshRecord, newWant, isTimerRefresh);
}

/**
 * @brief Connect ams for refresh form
 * 
 * @param formId The form id.
 * @param record Form data.
 * @param want The want of the form.
 * @param isTimerRefresh The flag of timer refresh.
 * @return Returns ERR_OK on success, others on failure.
 */
ErrCode FormProviderMgr::ConnectAmsForRefresh(const int64_t formId, const FormRecord &record, const Want &want, 
const bool isTimerRefresh)
{
    APP_LOGD("%{public}s called, bundleName:%{public}s, abilityName:%{public}s.",
        __func__, record.bundleName.c_str(), record.abilityName.c_str());

    sptr<IAbilityConnection> formRefreshConnection = new FormRefreshConnection(formId, want);
    Want connectWant;
    connectWant.AddFlags(Want::FLAG_ABILITY_FORM_ENABLED);
    connectWant.SetElementName(record.bundleName, record.abilityName);

    if (isTimerRefresh) {
        if (!FormTimerMgr::GetInstance().IsLimiterEnableRefresh(formId)) {
            APP_LOGE("%{public}s, timer refresh, already limit.", __func__);
            return ERR_APPEXECFWK_FORM_SUPPLIER_DEL_FAIL;
        }
    }

    ErrCode errorCode = FormAmsHelper::GetInstance().ConnectServiceAbility(connectWant, formRefreshConnection);
    if (errorCode != ERR_OK) {
        APP_LOGE("%{public}s, ConnectServiceAbility failed.", __func__);
        return errorCode;
    }
    
    if (record.isCountTimerRefresh) {
        IncreaseTimerRefreshCount(formId);
    }

    return ERR_OK;
}

/**
 * @brief Notify provider form delete.
 * @param formId The form id.
 * @param record Form information.
 * @return Function result and has other host flag.
 */
ErrCode FormProviderMgr::NotifyProviderFormDelete(const int64_t formId, const FormRecord &formRecord)
{
    if (formRecord.abilityName.empty()) {
        APP_LOGE("%{public}s, formRecord.abilityName is empty.", __func__);
        return ERR_CODE_COMMON;
    }

    if (formRecord.bundleName.empty()) {
        APP_LOGE("%{public}s, formRecord.bundleName is empty.", __func__);
        return ERR_CODE_COMMON;
    }

    APP_LOGD("%{public}s, connectAbility,bundleName:%{public}s, abilityName:%{public}s",
        __func__, formRecord.bundleName.c_str(), formRecord.abilityName.c_str());
    sptr<IAbilityConnection> formDeleteConnection = new FormDeleteConnection(formId);
    Want want;
    want.SetElementName(formRecord.bundleName, formRecord.abilityName);
    want.SetFlags(Want::FLAG_ABILITY_FORM_ENABLED);
    return FormAmsHelper::GetInstance().ConnectServiceAbility(want, formDeleteConnection);
}

/**
 * @brief Notify provider forms batch delete.
 * @param bundleName BundleName.
 * @param bundleName AbilityName.
 * @param formIds form id list.
 * @return Returns ERR_OK on success, others on failure.
 */
ErrCode FormProviderMgr::NotifyProviderFormsBatchDelete(const std::string &bundleName, const std::string &abilityName, 
const std::set<int64_t> &formIds)
{
    if (abilityName.empty()) {
        APP_LOGE("%{public}s error, abilityName is empty.",  __func__);
        return ERR_APPEXECFWK_FORM_INVALID_PARAM;
    }

    if (bundleName.empty()) {
        APP_LOGE("%{public}s error, bundleName is empty.",  __func__);
        return ERR_APPEXECFWK_FORM_INVALID_PARAM;
    }

    APP_LOGD("%{public}s, bundleName:%{public}s, abilityName:%{public}s",  __func__, bundleName.c_str(), 
    abilityName.c_str());
    sptr<IAbilityConnection> batchDeleteConnection = new FormBatchDeleteConnection(formIds);
    Want want;
    want.AddFlags(Want::FLAG_ABILITY_FORM_ENABLED);
    want.SetElementName(bundleName, abilityName);

    ErrCode errorCode = FormAmsHelper::GetInstance().ConnectServiceAbility(want, batchDeleteConnection);
    if (errorCode != ERR_OK) {
        APP_LOGE("%{public}s, ConnectServiceAbility failed.", __func__);
        return errorCode;
    }
    return ERR_OK;
}
/**
 * @brief Update form.
 * @param formId The form's id.

 * @param formProviderData form provider data.
 * @return Returns ERR_OK on success, others on failure.
 */
ErrCode FormProviderMgr::UpdateForm(const int64_t formId, const FormProviderInfo &formProviderInfo)
{
    // check exist and get the formRecord
    FormRecord formRecord;
    if (!FormDataMgr::GetInstance().GetFormRecord(formId, formRecord)) {
        APP_LOGE("%{public}s error, not exist such form:%{public}" PRId64 ".", __func__, formId);
        return ERR_NOT_EXIST_ID;
    }
    return UpdateForm(formId, formRecord, formProviderInfo.GetFormData());
}
/**
 * handle for update form event from provider.
 *
 * @param formId The id of the form.
 * @param formRecord The form's record.
 * @param formProviderData provider form info.
 * @return Returns ERR_OK on success, others on failure.
 */
ErrCode FormProviderMgr::UpdateForm(const int64_t formId, FormRecord &formRecord, 
const FormProviderData &formProviderData)
{
    APP_LOGI("%{public}s start", __func__);

    if (formRecord.versionUpgrade) {
        formRecord.formProviderInfo.SetFormData(formProviderData);
    } else {
        nlohmann::json addJsonData = formProviderData.GetData();
        formRecord.formProviderInfo.MergeData(addJsonData);
    }

    // formRecord init
    formRecord.isInited = true;
    formRecord.needRefresh = false;
    FormDataMgr::GetInstance().SetFormCacheInited(formId, true);
    
    // update form for host clients
    FormDataMgr::GetInstance().UpdateHostNeedRefresh(formId, true);

    bool screenOnFlag = PowerMgr::PowerMgrClient::GetInstance().IsScreenOn();
    if (screenOnFlag) {
        if (FormDataMgr::GetInstance().UpdateHostForm(formId, formRecord)) {
            FormDataMgr::GetInstance().SetVersionUpgrade(formId, false);
        }
    }
    // check if cache data size is less than 1k or not    
    std::string jsonData = formRecord.formProviderInfo.GetFormDataString(); // get json data
    APP_LOGD("%{public}s , jsonData is %{public}s.",  __func__, jsonData.c_str());
    if (jsonData.size() <= Constants::MAX_FORM_DATA_SIZE) {
        APP_LOGI("%{public}s, updateJsForm, data is less than 1k, cache data.", __func__);        
        FormCacheMgr::GetInstance().AddData(formId, jsonData);
    }

    // the update form is successfully
    return ERR_OK;
}
/**
 * @brief Process js message event.
 * @param formId Indicates the unique id of form.
 * @param record Form record.
 * @param want information passed to supplier.
 * @return Returns true if execute success, false otherwise.
 */
int FormProviderMgr::MessageEvent(const int64_t formId, const FormRecord &record, const Want &want)
{
    APP_LOGI("%{public}s called, formId:%{public}" PRId64 ".", __func__, formId);
   
    bool screenOnFlag = PowerMgr::PowerMgrClient::GetInstance().IsScreenOn();
    if (!screenOnFlag) {        
        APP_LOGW("%{public}s fail, screen off now", __func__);
        return ERR_APPEXECFWK_FORM_COMMON_CODE;
    }

    sptr<IAbilityConnection> formMsgEventConnection = new FormMsgEventConnection(formId, want);
    Want connectWant;
    connectWant.AddFlags(Want::FLAG_ABILITY_FORM_ENABLED);
    connectWant.SetElementName(record.bundleName, record.abilityName);

    ErrCode errorCode = FormAmsHelper::GetInstance().ConnectServiceAbility(connectWant, formMsgEventConnection);
    if (errorCode != ERR_OK) {
        APP_LOGE("%{public}s, ConnectServiceAbility failed.", __func__);
        return errorCode;
    }

    return ERR_OK;
}

/**
 * @brief Increase the timer refresh count.
 * 
 * @param formId The form id.
 */
void FormProviderMgr::IncreaseTimerRefreshCount(const int64_t formId)
{
    FormRecord record;
    if (FormDataMgr::GetInstance().GetFormRecord(formId, record)) {
        APP_LOGE("%{public}s failed, not exist such form:%{public}" PRId64 ".", __func__, formId);
        return;
    }

    if (record.isCountTimerRefresh) {
        FormDataMgr::GetInstance().SetCountTimerRefresh(formId, false);
        FormTimerMgr::GetInstance().IncreaseRefreshCount(formId);
    }
}
FormRecord FormProviderMgr::GetFormAbilityInfo(const FormRecord &record) const
{
    FormRecord newRecord;
    newRecord.bundleName = record.bundleName;
    newRecord.abilityName = record.abilityName;

    return newRecord;
}

bool FormProviderMgr::IsFormCached(const FormRecord &record)
{
    if (record.versionUpgrade) {
        return false;
    }
    return FormCacheMgr::GetInstance().IsExist(record.formId);
}
}  // namespace AppExecFwk
}  // namespace OHOS