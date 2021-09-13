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

#include "appexecfwk_errors.h"
#include "app_log_wrapper.h"
#include "form_acquire_connection.h"
#include "form_ams_helper.h"
#include "form_bms_helper.h"
#include "form_cache_mgr.h"
#include "form_cast_temp_connection.h"
#include "form_constants.h"
#include "form_data_mgr.h"
#include "form_delete_connection.h"
#include "form_db_cache.h"
#include "form_db_info.h"
#include "form_dump_mgr.h"
#include "form_event_notify_connection.h"
#include "form_mgr_adapter.h"
#include "form_provider_info.h"
#include "form_provider_mgr.h"
#include "form_refresh_connection.h"
#include "form_timer_mgr.h"
#include "if_system_ability_manager.h"
#include "ipc_skeleton.h"
#include "iservice_registry.h"
#include "ohos_account_kits.h"
#include "power_mgr_client.h"
#include "system_ability_definition.h"

namespace OHOS {
namespace AppExecFwk {
// /**
//  * @brief Get moduleName from packageName and abilityName.
//  * @param packageName Package name.
//  * @param abilityName Ability name.
//  * @return ModuleName.
//  */
// std::string GetFullClassName(const std::string& packageName, const std::string& abilityName)
// {
//     std::string fullName;
//     if (!abilityName.empty() && abilityName[0] == '.') {
//         fullName = packageName + abilityName;
//     } else {
//         fullName = abilityName;
//     }
//     return fullName;
// }

/**
 * @brief Add form with want, send want to form manager service.
 * @param formId The Id of the forms to add.
 * @param want The want of the form to add.
 * @param callerToken Caller ability token.
 * @param formInfo Form info.
 * @return Returns ERR_OK on success, others on failure.
 */
int FormMgrAdapter::AddForm(const int64_t formId, const Want &want, const sptr<IRemoteObject> &callerToken, 
FormJsInfo &formInfo)
{
    if (formId < 0 || callerToken == nullptr) {
        APP_LOGE("%{public}s fail, callerToken can not be NULL", __func__);
        return ERR_APPEXECFWK_FORM_INVALID_PARAM;
    }

    // check form count limit    
    bool tempFormFlag = want.GetBoolParam(Constants::PARAM_FORM_TEMPORARY_KEY, false);
    int callingUid = IPCSkeleton::GetCallingUid();
    int checkCode;
    if (tempFormFlag) {
        if (formId > 0) {
            //APP_LOGE("%{public}s fail, temp form id is invalid, formId:%{public}lld", __func__, formId);
            return ERR_APPEXECFWK_FORM_INVALID_PARAM;
        }
        checkCode = FormDataMgr::GetInstance().CheckTempEnoughForm();
    } else {
        checkCode = FormDataMgr::GetInstance().CheckEnoughForm(callingUid);
    }
    if (checkCode != 0) {
        APP_LOGE("%{public}s fail, too much forms in system", __func__);
        return checkCode;
    }

    // get from comfig info
    FormItemInfo formItemInfo;
    int32_t errCode = GetFormConfigInfo(want, formItemInfo);
    formItemInfo.SetFormId(formId);
    if (errCode != ERR_OK) {
        APP_LOGE("%{public}s fail, get form config info failed.", __func__);
        return errCode;
    }
    if (!formItemInfo.IsValidItem()) {
        APP_LOGE("%{public}s fail, input param itemInfo is invalid", __func__);
        return ERR_APPEXECFWK_FORM_GET_INFO_FAILED;
    }
    if (!FormDataMgr::GetInstance().GenerateUdidHash()) {
        APP_LOGE("%{public}s fail, generate udid hash failed", __func__);
        return ERR_APPEXECFWK_FORM_COMMON_CODE;
    }

    WantParams wantParams = want.GetParams();
    if (formId > 0) {
        return AllotFormById(formItemInfo, callerToken, wantParams, formInfo);
    } else {
        return AllotFormByInfo(formItemInfo, callerToken, wantParams, formInfo);
    }
}

/**
 * @brief Delete forms with formIds, send formIds to form manager service.
 * @param formId The Id of the forms to delete.
 * @param callerToken Caller ability token.
 * @return Returns ERR_OK on success, others on failure.
 */
int FormMgrAdapter::DeleteForm(const int64_t formId, const sptr<IRemoteObject> &callerToken)
{
    if (formId <= 0 || callerToken == nullptr) {
        APP_LOGE("%{public}s, deleteForm invalid param", __func__);
        return ERR_FORM_INVALID_PARAM;
    }

    int64_t matchedFormId = FormDataMgr::GetInstance().FindMatchedFormId(formId);
    if (FormDataMgr::GetInstance().ExistTempForm(matchedFormId)) {
        // delete temp form if receive delete form call
        return HandleDeleteTempForm(matchedFormId, callerToken);
    }
    return HandleDeleteForm(matchedFormId, callerToken);
}

/**
 * @brief Release forms with formIds, send formIds to form Mgr service.
 * @param formId The Id of the forms to release.
 * @param callerToken Caller ability token.
 * @param delCache Delete Cache or not.
 * @return Returns ERR_OK on success, others on failure.
 */
int FormMgrAdapter::ReleaseForm(const int64_t formId, const sptr<IRemoteObject> &callerToken, const bool delCache)
{
    APP_LOGI("%{public}s called.", __func__);

    if (formId <= 0 || callerToken == nullptr) {
        APP_LOGE("%{public}s, releaseForm invalid param", __func__);
        return ERR_FORM_INVALID_PARAM;
    }

    int64_t matchedFormId = FormDataMgr::GetInstance().FindMatchedFormId(formId);
    if (FormDataMgr::GetInstance().ExistTempForm(matchedFormId)) {
        // delete temp form if receive release form call
        return HandleDeleteTempForm(matchedFormId, callerToken);
    }

    if (delCache) {
        if (ErrCode result = HandleReleaseForm(matchedFormId, callerToken); result != ERR_OK) {
            APP_LOGE("%{public}s, release form error.", __func__);
            return result;
        }
    }

    if (!FormDataMgr::GetInstance().DeleteHostRecord(callerToken, matchedFormId)) {
        APP_LOGE("%{public}s, failed to remove host record", __func__);
        return ERR_APPEXECFWK_FORM_COMMON_CODE;
    }
    if (!FormTimerMgr::GetInstance().RemoveFormTimer(matchedFormId)) {
        APP_LOGE("%{public}s, remove timer error", __func__);
        return ERR_DELETE_FORM_TIMER;
    }
    return ERR_OK;
}

/**
 * @brief Handle release form.
 * @param formId The form id.
 * @param callerToken Caller ability token.
 * @return Returns ERR_OK on success, others on failure.
 */
ErrCode FormMgrAdapter::HandleReleaseForm(const int64_t formId, const sptr<IRemoteObject> &callerToken)
{
    APP_LOGI("%{public}s called.", __func__);
    if (!FormDataMgr::GetInstance().ExistFormRecord(formId)) {
        //APP_LOGE("%{public}s, not exist such db or temp form:%{public}lld", __func__, formId);
        return ERR_NOT_EXIST_ID;
    }

    FormHostRecord hostRecord;
    bool hasHostRec = FormDataMgr::GetInstance().GetMatchedHostClient(callerToken, hostRecord);
    bool isSelfId = hasHostRec && hostRecord.Contains(formId);
    if (!isSelfId) {
        //APP_LOGE("%{public}s, not self form:%{public}lld", __func__, formId);
        return ERR_OPERATION_FORM_NOT_SELF;
    }

    //APP_LOGD("%{public}s, release formRecords, formId: %{public}lld", __func__, formId);
    FormDataMgr::GetInstance().DeleteFormUserUid(formId, IPCSkeleton::GetCallingUid());
    if (!FormDataMgr::GetInstance().HasFormUserUids(formId)) {
        FormDataMgr::GetInstance().DeleteFormRecord(formId);
        if (!FormTimerMgr::GetInstance().RemoveFormTimer(formId)) {
            APP_LOGE("%{public}s, remove timer error", __func__);
            return ERR_DELETE_FORM_TIMER;
        }
    }
    return ERR_OK;
}

/**
 * @brief Handle delete form.
 * @param formId The form id.
 * @param callerToken Caller ability token.
 * @return Returns ERR_OK on success, others on failure.
 */
ErrCode FormMgrAdapter::HandleDeleteForm(const int64_t formId, const sptr<IRemoteObject> &callerToken)
{
    FormRecord dbRecord;
    if (FormDbCache::GetInstance().GetDBRecord(formId, dbRecord) != ERR_OK) {
        //APP_LOGE("%{public}s, not exist such db or temp form:%{public}lld", __func__, formId);
        return ERR_NOT_EXIST_ID;
    }

    int callingUid = IPCSkeleton::GetCallingUid();
    bool isSelfDbFormId = (std::find(dbRecord.formUserUids.begin(), dbRecord.formUserUids.end(), callingUid) != 
            dbRecord.formUserUids.end()) ? true : false;

    if (!isSelfDbFormId) {
        //APP_LOGE("%{public}s, not self form:%{public}lld", __func__, formId);
        return ERR_OPERATION_FORM_NOT_SELF;
    }

    if (ErrCode result = HandleDeleteFormCache(dbRecord, callingUid, formId); result != ERR_OK) {
        return result;
    }

    if (!FormDataMgr::GetInstance().DeleteHostRecord(callerToken, formId)) {
        APP_LOGE("%{public}s, failed to remove host record", __func__);
        return ERR_APPEXECFWK_FORM_COMMON_CODE;
    }

    return ERR_OK;
}

/**
 * @brief Handle delete temp form.
 * @param formId The form id.
 * @param callerToken Caller ability token.
 * @return Returns ERR_OK on success, others on failure.
 */
ErrCode FormMgrAdapter::HandleDeleteTempForm(const int64_t formId, const sptr<IRemoteObject> &callerToken)
{
    APP_LOGI("%{public}s called.", __func__);

    int uid = IPCSkeleton::GetCallingUid();
    FormRecord record;
    bool isFormRecExist = FormDataMgr::GetInstance().GetFormRecord(formId, record);
    bool isSelfTempFormId = false;
    if (isFormRecExist && record.formTempFlg) {
        isSelfTempFormId = (std::find(record.formUserUids.begin(), record.formUserUids.end(), uid) != 
            record.formUserUids.end()) ? true : false;
    }
    if (!isSelfTempFormId) {
        //APP_LOGE("%{public}s, not self form:%{public}lld", __func__, formId);
        return ERR_OPERATION_FORM_NOT_SELF;
    }

    FormDataMgr::GetInstance().DeleteFormUserUid(formId, uid);
    if (!FormDataMgr::GetInstance().HasFormUserUids(formId)) {
        int result = FormProviderMgr::GetInstance().NotifyProviderFormDelete(formId, record);
        if (result != ERR_OK) {
            APP_LOGE("%{public}s, failed!", __func__);
            FormDataMgr::GetInstance().AddFormUserUid(formId, uid);
            return result;
        }
        FormDataMgr::GetInstance().DeleteTempForm(formId);
        FormDataMgr::GetInstance().DeleteFormRecord(formId);
        if (!FormCacheMgr::GetInstance().DeleteData(formId)) {
            APP_LOGE("%{public}s, failed to remove cache data", __func__);
            return ERR_APPEXECFWK_FORM_COMMON_CODE;
        }
    }

    if (!FormDataMgr::GetInstance().DeleteHostRecord(callerToken, formId)) {
        APP_LOGE("%{public}s, failed to remove host record", __func__);
        return ERR_APPEXECFWK_FORM_COMMON_CODE;
    }

    //APP_LOGD("%{public}s, record.formUserUids size: %{public}d", __func__, record.formUserUids.size());
    return ERR_OK;
}

/**
 * @brief Handle delete form cache.
 * @param dbRecord Form storage information.
 * @param uid calling user id.
 * @param formId The form id.
 * @return Returns ERR_OK on success, others on failure.
 */
ErrCode FormMgrAdapter::HandleDeleteFormCache(FormRecord &dbRecord, const int uid, const int64_t formId)
{
    //APP_LOGD("%{public}s, delete formDBRecords, formId: %{public}lld", __func__, formId);
    auto iter = std::find(dbRecord.formUserUids.begin(), dbRecord.formUserUids.end(), uid);
    if (iter != dbRecord.formUserUids.end()) {
        dbRecord.formUserUids.erase(iter);
    }

    ErrCode result = ERR_OK;
    if (dbRecord.formUserUids.empty()) {
        result = FormProviderMgr::GetInstance().NotifyProviderFormDelete(formId, dbRecord);
        if (result != ERR_OK) {
            APP_LOGE("%{public}s, failed to notify provider form delete", __func__);
            return result;
        }
        if (!FormDataMgr::GetInstance().DeleteFormRecord(formId)) {
            APP_LOGE("%{public}s, failed to remove cache data", __func__);
            return ERR_APPEXECFWK_FORM_COMMON_CODE;
        }
        if (result = FormDbCache::GetInstance().DeleteFormInfo(formId); result != ERR_OK) {
            APP_LOGE("%{public}s, failed to remove db data", __func__);
            return result;
        }

        int32_t matchCount = FormDbCache::GetInstance().GetMatchCount(dbRecord.bundleName, dbRecord.moduleName);
        if (matchCount == 0) {
            FormBmsHelper::GetInstance().NotifyModuleRemovable(dbRecord.bundleName, dbRecord.moduleName);
        }

        if (!FormCacheMgr::GetInstance().DeleteData(formId)) {
            APP_LOGE("%{public}s, failed to remove cache data", __func__);
            return ERR_APPEXECFWK_FORM_COMMON_CODE;
        }
        if (!FormTimerMgr::GetInstance().RemoveFormTimer(formId)) {
            APP_LOGE("%{public}s, remove timer error", __func__);
            return ERR_DELETE_FORM_TIMER;
        }
        return ERR_OK;
    }

    if (result = FormDbCache::GetInstance().UpdateDBRecord(formId, dbRecord); result != ERR_OK) {
        return result;
    }

    //APP_LOGD("%{public}s, dbRecord.formUserUids size: %{public}d", __func__, dbRecord.formUserUids.size());
    FormBmsHelper::GetInstance().NotifyModuleNotRemovable(dbRecord.bundleName, dbRecord.moduleName);
    if (!FormDataMgr::GetInstance().DeleteFormUserUid(formId, uid)) {
        APP_LOGE("%{public}s, failed to remove form user uid", __func__);
        return ERR_APPEXECFWK_FORM_COMMON_CODE;
    }

    return result;
}


/**
 * @brief Update form with formId, send formId to form manager service.
 * @param formId The Id of the form to update.
 * @param bundleName Provider ability bundleName.
 * @param formProviderData form provider data.
 * @return Returns ERR_OK on success, others on failure.
 */
int FormMgrAdapter::UpdateForm(const int64_t formId, const std::string &bundleName, 
const FormProviderData &formProviderData)
{
    APP_LOGI("%{public}s start.", __func__);

    // check formId and bundleName
    if (formId <= 0 || bundleName.empty()) {
        APP_LOGE("%{public}s error, the passed in formId can't be negative or zero, bundleName is not empty.", 
        __func__);
        return ERR_FORM_INVALID_PARAM;
    }

    // get IBundleMgr
    sptr<IBundleMgr> iBundleMgr = FormBmsHelper::GetInstance().GetBundleMgr();
    if (iBundleMgr == nullptr) {
        APP_LOGE("%{public}s error, failed to get IBundleMgr.", __func__);
        return ERR_APPEXECFWK_FORM_GET_BUNDLE_FAILED;
    }

    // check bundle uid for permission
    int32_t userId {0};
    int32_t callingUid = IPCSkeleton::GetCallingUid();
    int32_t bundleUid = iBundleMgr->GetUidByBundleName(bundleName, userId);
    if (bundleUid != callingUid) {
        APP_LOGE("%{public}s error, permission denied, the updated form is not your own.", __func__);
        return ERR_FORM_INVALID_PARAM;
    }

    // find matched formId
    int64_t matchedFormId = FormDataMgr::GetInstance().FindMatchedFormId(formId);

    // check exist and get the formRecord
    FormRecord formRecord;
    if (!FormDataMgr::GetInstance().GetFormRecord(matchedFormId, formRecord)) {
        //APP_LOGE("%{public}s error, not exist such form:%{public}lld.", __func__, matchedFormId);
        return ERR_NOT_EXIST_ID;
    }

    // check then form under current user
    if (!FormDataMgr::GetInstance().IsCallingUidValid(formRecord.formUserUids)) {
        //APP_LOGE("%{public}s error, not under current user, formId:%{public}lld.", __func__, matchedFormId);
        return ERR_NOT_EXIST_ID;
    }

    // check bundleName match
    if (formRecord.bundleName.compare(bundleName) != 0) {
        APP_LOGE("%{public}s error, not match bundleName:%{public}s.", __func__, bundleName.c_str());
        return ERR_FORM_INVALID_PARAM;
    }

    // update Form
    return FormProviderMgr::GetInstance().UpdateForm(matchedFormId, formRecord, formProviderData);
}

/**
 * @brief Request form with formId and want, send formId and want to form manager service.
 * @param formId The Id of the form to update.
 * @param callerToken Caller ability token.
 * @param want The want of the form to request.
 * @return Returns ERR_OK on success, others on failure.
 */
int FormMgrAdapter::RequestForm(const int64_t formId, const sptr<IRemoteObject> &callerToken, const Want &want)
{
    APP_LOGI("%{public}s called.", __func__);

    if (callerToken == nullptr) {
        APP_LOGE("%{public}s fail, callerToken can not be NULL.", __func__);
        return ERR_APPEXECFWK_FORM_INVALID_PARAM;
    }

    int64_t matchedFormId = FormDataMgr::GetInstance().FindMatchedFormId(formId);

    if (!FormDataMgr::GetInstance().ExistFormRecord(matchedFormId)) {
        //APP_LOGE("%{public}s fail, not exist such formId:%{public}lld.", __func__, matchedFormId);
        return ERR_NOT_EXIST_ID;
    }

    FormHostRecord formHostRecord; 
    bool isHostExist = FormDataMgr::GetInstance().GetMatchedHostClient(callerToken, formHostRecord);
    if (!isHostExist) {
        APP_LOGE("%{public}s fail, cannot find target client.", __func__);
        return ERR_FORM_INVALID_PARAM;
    }
    
    if (!formHostRecord.Contains(matchedFormId)) {
        APP_LOGE("%{public}s fail, form is not self-owned.", __func__);
        return ERR_OPERATION_FORM_NOT_SELF;
    }

    APP_LOGI("%{public}s, find target client.", __func__);
    return FormProviderMgr::GetInstance().RefreshForm(matchedFormId, want);
}

/**
 * @brief Form visible/invisible notify, send formIds to form manager service.
 * 
 * @param formIds The vector of form Ids.
 * @param callerToken Caller ability token.
 * @param formVisibleType The form visible type, including FORM_VISIBLE and FORM_INVISIBLE.
 * @return Returns ERR_OK on success, others on failure.
 */
ErrCode FormMgrAdapter::NotifyWhetherVisibleForms(const std::vector<int64_t> &formIds, 
const sptr<IRemoteObject> &callerToken,
    const int32_t formVisibleType)
{
    APP_LOGI("%{public}s called.", __func__);

    if (callerToken == nullptr) {
        APP_LOGE("%{public}s fail, callerToken can not be NULL.", __func__);
        return ERR_APPEXECFWK_FORM_INVALID_PARAM;
    }

    sptr<IBundleMgr> iBundleMgr = FormBmsHelper::GetInstance().GetBundleMgr();
    if (iBundleMgr == nullptr) {
        APP_LOGE("%{public}s fail, failed to get IBundleMgr.", __func__);
        return ERR_APPEXECFWK_FORM_GET_BUNDLE_FAILED;
    }

    int64_t matchedFormId;
    std::map<std::string, std::vector<int64_t>> eventMaps;
    for (int64_t formId : formIds) {
        if (formId <= 0) {
            //APP_LOGW("%{public}s, formId %{public}lld is less than 0", __func__, formId);
            continue;
        }
        matchedFormId = FormDataMgr::GetInstance().FindMatchedFormId(formId);
        FormRecord formRecord;
        if (!FormDataMgr::GetInstance().GetFormRecord(matchedFormId, formRecord)) {
            //APP_LOGW("%{public}s fail, not exist such form, formId:%{public}lld.", __func__, matchedFormId);
            continue;
        }

        FormHostRecord formHostRecord;
        bool hasFormHostRecord = FormDataMgr::GetInstance().GetMatchedHostClient(callerToken, formHostRecord);
        if (!(hasFormHostRecord && formHostRecord.Contains(matchedFormId))) {
            //APP_LOGW("%{public}s fail, form is not belong to self, formId:%{public}lld.", __func__, matchedFormId);
            continue;
        }

        formRecord.formVisibleNotifyState = formVisibleType;
        if (!FormDataMgr::GetInstance().UpdateFormRecord(matchedFormId, formRecord)) {
            //APP_LOGW("%{public}s fail, set formVisibleNotifyState error, formId:%{public}lld.", 
            //__func__, matchedFormId);
            continue;
        }

        // If the form need refrsh flag is true and form visibleType is FORM_VISIBLE, refresh the form host.
        if (formRecord.needRefresh && formVisibleType == Constants::FORM_VISIBLE) {
            std::string cacheData;
            // If the form has business cache, refresh the form host.
            if (FormCacheMgr::GetInstance().GetData(matchedFormId, cacheData)) {
                formRecord.formProviderInfo.SetFormDataString(cacheData);
                formHostRecord.OnUpdate(matchedFormId, formRecord);
            }
        }

        // If the form provider is system app and the config item 'formVisibleNotify' is true,
        // notify the form provider that the current form is visible.
        BundleInfo bundleInfo;
        if (iBundleMgr->GetBundleInfo(formRecord.bundleName, BundleFlag::GET_BUNDLE_DEFAULT, bundleInfo)) {
            APP_LOGD("%{public}s, get bundle uid success", __func__);
            if (!iBundleMgr->CheckIsSystemAppByUid(bundleInfo.uid)) {
                //APP_LOGW("%{public}s fail, form provider is not system app, formId:%{public}lld.", __func__, matchedFormId);
                continue;
            }
        } else {
            APP_LOGW("%{public}s fail, can not get bundleInfo's uid", __func__);
            continue;
        }
        if (!formRecord.formVisibleNotify) {
            //APP_LOGW("%{public}s fail, the config item 'formVisibleNotify' is false, formId:%{public}lld.", 
            //__func__, matchedFormId);
            continue;
        }

        std::string providerKey = formRecord.bundleName + Constants::NAME_DELIMITER + formRecord.abilityName;
        auto iter = eventMaps.find(providerKey);
        if (iter == eventMaps.end()) {
            std::vector<int64_t> formEventsByProvider {matchedFormId};
            eventMaps.insert(std::make_pair(providerKey, formEventsByProvider));
        } else {
            iter->second.emplace_back(matchedFormId);
        }
    }

    for (auto iter = eventMaps.begin(); iter != eventMaps.end(); iter++) {
        if (HandleEventNotify(iter->first, iter->second, formVisibleType) != ERR_OK) {
            APP_LOGW("%{public}s fail, HandleEventNotify error, key is %{public}s.", __func__, iter->first.c_str());
        }
    }

    return ERR_OK;
}

/**
 * @brief Temp form to normal form.
 * @param formId The Id of the form.
 * @return Returns ERR_OK on success, others on failure.
 */
int FormMgrAdapter::CastTempForm(const int64_t formId, const sptr<IRemoteObject> &callerToken)
{
    if (formId <= 0 || callerToken == nullptr) {
        APP_LOGE("%{public}s, invalid param", __func__);
        return ERR_FORM_INVALID_PARAM;
    }

    int64_t matchedFormId = FormDataMgr::GetInstance().FindMatchedFormId(formId);
    if (!FormDataMgr::GetInstance().ExistFormRecord(matchedFormId) || 
        !FormDataMgr::GetInstance().ExistTempForm(matchedFormId)) {
        //APP_LOGE("%{public}s, not exist such temp form:%{public}lld", __func__, matchedFormId);
        return ERR_NOT_EXIST_ID;
    }

    FormHostRecord record;
    bool hasHostRec = FormDataMgr::GetInstance().GetFormHostRecord(matchedFormId, record);
    if (!hasHostRec || !record.Contains(matchedFormId)) {
        //APP_LOGE("%{public}s, not self form:%{public}lld", __func__, matchedFormId);
        return ERR_OPERATION_FORM_NOT_SELF;
    }

    int callingUid = IPCSkeleton::GetCallingUid();
    int checkCode = FormDataMgr::GetInstance().CheckEnoughForm(callingUid);
    if (checkCode != 0) {
        //APP_LOGE("%{public}s, %{public}lld failed,because if too mush forms", __func__, matchedFormId);
        return checkCode;
    }

    FormRecord formRecord;
    if (!FormDataMgr::GetInstance().GetFormRecord(matchedFormId, formRecord)) {
        //APP_LOGE("%{public}s fail, not exist such form:%{public}lld.", __func__, matchedFormId);
        return ERR_NOT_EXIST_ID;
    }
    int bindSupplierCheckCode = HandleCastTempForm(matchedFormId, formRecord);
    if (bindSupplierCheckCode != 0) {
        APP_LOGE("%{public}s, cast temp form bindSupplier failed", __func__);
        return bindSupplierCheckCode;
    }

    if (!FormDataMgr::GetInstance().DeleteTempForm(matchedFormId)) {
        //APP_LOGE("%{public}s fail, delete temp form error, formId:%{public}lld.", __func__, matchedFormId);
        return ERR_APPEXECFWK_FORM_INFO_NOT_EXIST;
    }
    if (!FormDataMgr::GetInstance().ModifyFormTempFlg(matchedFormId, false)) {
        //APP_LOGE("%{public}s fail, modify form temp flag error, formId:%{public}lld.", __func__, matchedFormId);
        return ERR_APPEXECFWK_FORM_INFO_NOT_EXIST;
    }
    if (!FormDataMgr::GetInstance().AddFormUserUid(matchedFormId, callingUid)) {
        //APP_LOGE("%{public}s fail, add form user uid error, formId:%{public}lld.", __func__, matchedFormId);
        return ERR_APPEXECFWK_FORM_INFO_NOT_EXIST;
    }
    if (std::find(formRecord.formUserUids.begin(), formRecord.formUserUids.end(),
        callingUid) == formRecord.formUserUids.end()) {
        formRecord.formUserUids.emplace_back(callingUid);
    }
    if (ErrCode errorCode = FormDbCache::GetInstance().UpdateDBRecord(matchedFormId, formRecord); errorCode != ERR_OK) {
        //APP_LOGE("%{public}s fail, update db record error, formId:%{public}lld.", __func__, matchedFormId);
        return errorCode;
    }

    // start timer
    return AddFormTimer(formRecord);
}
/**
 * @brief Handle cast temp form.
 * @param formId The form id.
 * @param record Form information.
 * @return Returns ERR_OK on success, others on failure.
 */
ErrCode FormMgrAdapter::HandleCastTempForm(const int64_t formId, const FormRecord &record)
{
    APP_LOGD("%{public}s, cast temp form to normal form, notify supplier, package:%{public}s, class:%{public}s",
        __func__, record.bundleName.c_str(), record.abilityName.c_str());
    sptr<IAbilityConnection> castTempConnection = new FormCastTempConnection(formId);
    
    Want want;
    want.AddFlags(Want::FLAG_ABILITY_FORM_ENABLED);
    want.SetElementName(record.bundleName, record.abilityName);
    return FormAmsHelper::GetInstance().ConnectServiceAbility(want, castTempConnection);
}
/**
 * @brief Dump all of form storage infos.
 * @param formInfos All of form storage infos.
 * @return Returns ERR_OK on success, others on failure.
 */
int FormMgrAdapter::DumpStorageFormInfos(std::string &formInfos) const
{
    std::vector<FormDBInfo> formDBInfos;
    FormDbCache::GetInstance().GetAllFormInfo(formDBInfos);
    if(formDBInfos.size() > 0) {
        std::sort(formDBInfos.begin(), formDBInfos.end(), [] (FormDBInfo &formDBInfoA, FormDBInfo &formDBInfoB) -> bool {
            return formDBInfoA.formId < formDBInfoB.formId;
        });
        FormDumpMgr::GetInstance().DumpStorageFormInfos(formDBInfos, formInfos);
        return ERR_OK;
    } else {
        return ERR_APPEXECFWK_FORM_INFO_NOT_EXIST;
    }
}
/**
 * @brief Dump form info by a bundle name.
 * @param bundleName The bundle name of form provider.
 * @param formInfos Form infos.
 * @return Returns ERR_OK on success, others on failure.
 */
int FormMgrAdapter::DumpFormInfoByBundleName(const std::string &bundleName, std::string &formInfos) const
{
    APP_LOGI("%{public}s called.", __func__);
    std::vector<FormRecord> formRecordInfos;
    if (FormDataMgr::GetInstance().GetFormRecord(bundleName, formRecordInfos)) {
        FormDumpMgr::GetInstance().DumpFormInfos(formRecordInfos, formInfos);
        return ERR_OK;
    } else {
        return ERR_APPEXECFWK_FORM_INFO_NOT_EXIST;
    }
}
/**
 * @brief Dump form info by a bundle name.
 * @param formId The id of the form.
 * @param formInfo Form info.
 * @return Returns ERR_OK on success, others on failure.
 */
int FormMgrAdapter::DumpFormInfoByFormId(const std::int64_t formId, std::string &formInfo) const
{
    APP_LOGI("%{public}s called.", __func__);
    int reply = ERR_APPEXECFWK_FORM_INFO_NOT_EXIST;

    FormRecord formRecord;
    if (FormDataMgr::GetInstance().GetFormRecord(formId, formRecord)) {
        FormDumpMgr::GetInstance().DumpFormInfo(formRecord, formInfo);
        reply = ERR_OK;
    }
    
    FormHostRecord formHostRecord;
    if (FormDataMgr::GetInstance().GetFormHostRecord(formId, formHostRecord)) {
        FormDumpMgr::GetInstance().DumpFormHostInfo(formHostRecord, formInfo);  
        reply = ERR_OK;
    }
    
    return reply;
}
/**
 * @brief Get form configure info.
 * @param want The want of the request.
 * @param formItemInfo Form configure info.
 * @return Returns ERR_OK on success, others on failure.
 */
ErrCode FormMgrAdapter::GetFormConfigInfo(const Want &want, FormItemInfo &formConfigInfo)
{
    APP_LOGD("GetFormConfigInfo start.");
    BundleInfo bundleInfo;
    std::string packageName;

    ErrCode errCode = GetBundleInfo(want, bundleInfo, packageName);
    if (errCode != ERR_OK) {
        APP_LOGE("addForm find bundle info failed");
        return errCode;
    }

    FormInfo formInfo;
    errCode = GetFormInfo(want, formInfo);
    if (errCode != ERR_OK) {
        APP_LOGE("addForm can not find target form info");
        return errCode;
    }

    errCode = GetFormItemInfo(want, bundleInfo, formInfo, formConfigInfo);
    if (errCode != ERR_OK) {
        APP_LOGE("get form item info failed.");
        return errCode;
    }
    formConfigInfo.SetPackageName(packageName);

    APP_LOGD("GetFormConfigInfo end.");
    return ERR_OK;
}
/**
 * @brief Allocate form by formId.
 * @param info Form configure info.
 * @param callerToken Caller ability token.
 * @param wantParams WantParams of the request.
 * @param formInfo Form info for form host.
 * @return Returns ERR_OK on success, others on failure.
 */   
ErrCode FormMgrAdapter::AllotFormById(const FormItemInfo &info, const sptr<IRemoteObject> &callerToken, 
const WantParams &wantParams, FormJsInfo &formInfo)
{
    int64_t formId = PaddingUDIDHash(info.GetFormId());
    FormRecord record;
    bool hasRecord = FormDataMgr::GetInstance().GetFormRecord(formId, record);
    if (hasRecord && record.formTempFlg) {
        APP_LOGE("%{public}s, addForm can not acquire temp form when select form id", __func__);
        return ERR_APPEXECFWK_FORM_COMMON_CODE;
    }

    if (hasRecord && FormDataMgr::GetInstance().IsCallingUidValid(record.formUserUids)) {
        if (!info.IsMatch(record)) {
            //APP_LOGE("%{public}s, formId and item info not match:%{public}lld", __func__, formId);
            return ERR_CFG_NOT_MATCH_ID;
        }
        return AddExistFormRecord(info, callerToken, record, formId, wantParams);
    }

    // find in db but not in cache
    FormRecord dbRecord;
    ErrCode getDbRet = FormDbCache::GetInstance().GetDBRecord(formId, dbRecord);
    if (getDbRet == ERR_OK && FormDataMgr::GetInstance().IsCallingUidValid(dbRecord.formUserUids)) {
        return AddNewFormRecord(info, formId, callerToken, wantParams, formInfo);
    }

    //APP_LOGI("%{public}s, addForm no such form %{public}lld", __func__, formId);

    // delete form data in provider
    FormRecord delRecord;
    delRecord.bundleName = info.GetProviderBundleName();
    delRecord.abilityName = info.GetAbilityName();
    FormProviderMgr::GetInstance().NotifyProviderFormDelete(formId, delRecord);

    return ERR_NOT_EXIST_ID;
}
int64_t FormMgrAdapter::PaddingUDIDHash(const int64_t formId) const
{
    // Compatible with int form id.
    if ((formId & 0xffffffff00000000L) == 0) {
        return FormDataMgr::GetInstance().GetUdidHash() | formId;
    }
    return formId;
}
ErrCode FormMgrAdapter::AddExistFormRecord(const FormItemInfo &info, const sptr<IRemoteObject> &callerToken,
                                           const FormRecord &record, const int64_t formId, const WantParams &wantParams)
{
    //APP_LOGI("%{public}s call, formId:%{public}lld", __func__, formId);
    // allot form host record
    int callingUid = IPCSkeleton::GetCallingUid();
    bool isCreated = FormDataMgr::GetInstance().AllotFormHostRecord(info, callerToken, formId, callingUid);
    if (!isCreated) {
        APP_LOGE("%{public}s fail, AllotFormHostRecord failed when no matched formRecord", __func__);
        return ERR_APPEXECFWK_FORM_COMMON_CODE;
    }

    FormRecord newRecord(record);
    if (newRecord.needRefresh) {
        newRecord.isInited = false;
        FormDataMgr::GetInstance().SetFormCacheInited(formId, false);
        // acquire formInfo from provider
        ErrCode errorCode = AcquireProviderFormInfoAsync(formId, info, wantParams);
        if (errorCode != ERR_OK) {
            APP_LOGE("%{public}s fail, AcquireProviderFormInfoAsync failed", __func__);
            return errorCode;
        }
    }

    // Add new form user uid.
    FormDataMgr::GetInstance().AddFormUserUid(formId, callingUid);
    if (std::find(newRecord.formUserUids.begin(), newRecord.formUserUids.end(), callingUid) == 
        newRecord.formUserUids.end()) {
        newRecord.formUserUids.emplace_back(callingUid);
    }

    // start update timer
    if (ErrCode errorCode = AddFormTimer(newRecord); errorCode != ERR_OK) {
        return errorCode;
    }

    if (!newRecord.formTempFlg) {
        return FormDbCache::GetInstance().UpdateDBRecord(formId, newRecord);
    }
    return ERR_OK;
}
/**
 * @brief Allocate form by form configure info.
 * @param info Form configure info.
 * @param callerToken Caller ability token.
 * @param wantParams WantParams of the request.
 * @param formInfo Form info for form host.
 * @return Returns ERR_OK on success, others on failure.
 */   
ErrCode FormMgrAdapter::AllotFormByInfo(const FormItemInfo &info, const sptr<IRemoteObject> &callerToken, 
const WantParams &wantParams, FormJsInfo &formInfo)
{
    // generate formId
    int64_t newFormId = FormDataMgr::GetInstance().GenerateFormId();
    if (newFormId < 0) {
        APP_LOGE("%{public}s fail, generateFormId no invalid formId", __func__);
        return ERR_APPEXECFWK_FORM_COMMON_CODE;
    }
    //APP_LOGD("newFormId:%{public}lld", newFormId);

    return AddNewFormRecord(info, newFormId, callerToken, wantParams, formInfo);
}

/**
 * @brief Add new form record.
 * @param info Form configure info.
 * @param formId The form id.
 * @param callerToken Caller ability token.
 * @param wantParams WantParams of the request.
 * @param formInfo Form info for form host.
 * @return Returns ERR_OK on success, others on failure.
 */
ErrCode FormMgrAdapter::AddNewFormRecord(const FormItemInfo &info, const int64_t formId, 
    const sptr<IRemoteObject> &callerToken, const WantParams &wantParams, FormJsInfo &formInfo)
{
    APP_LOGI("%{public}s start", __func__);
    FormItemInfo newInfo(info);
    newInfo.SetFormId(formId);
    // allot form host record
    int callingUid = IPCSkeleton::GetCallingUid();
    if (!FormDataMgr::GetInstance().AllotFormHostRecord(newInfo, callerToken, formId, callingUid)) {
        APP_LOGE("%{public}s fail, AllotFormHostRecord failed when no matched formRecord", __func__);
        return ERR_APPEXECFWK_FORM_COMMON_CODE;
    }

    // allot form record
    FormRecord formRecord = FormDataMgr::GetInstance().AllotFormRecord(newInfo, callingUid);

    // acquire formInfo from provider
    if (ErrCode errorCode = AcquireProviderFormInfoAsync(formId, newInfo, wantParams); errorCode != ERR_OK) {
        APP_LOGE("%{public}s fail, AcquireProviderFormInfoAsync failed", __func__);
        return errorCode;
    }
    
    // create form info for js
    FormDataMgr::GetInstance().CreateFormInfo(formId, formRecord, formInfo);

    // storage info
    if (!newInfo.IsTemporaryForm()) {
        if (ErrCode errorCode = FormDbCache::GetInstance().UpdateDBRecord(formId, formRecord); errorCode != ERR_OK) {
            return errorCode;
        }
    }

    // start update timer
    return AddFormTimer(formRecord);
}

/**
 * @brief Add form timer.
 * @param formRecord Form information.
 * @return Returns ERR_OK on success, others on failure.
 */
ErrCode FormMgrAdapter::AddFormTimer(const FormRecord &formRecord)
{
    APP_LOGI("%{public}s start", __func__);
    if (formRecord.isEnableUpdate && !formRecord.formTempFlg) {
        bool timerRet = false;
        if (formRecord.updateDuration > 0) {
            timerRet = FormTimerMgr::GetInstance().AddFormTimer(formRecord.formId, formRecord.updateDuration);
        } else {
            timerRet = FormTimerMgr::GetInstance().AddFormTimer(formRecord.formId, formRecord.updateAtHour, 
            formRecord.updateAtMin);
        }
        if (!timerRet) {
            APP_LOGE("%{public}s fail, add form timer failed", __func__);
            return ERR_APPEXECFWK_FORM_COMMON_CODE;
        }
    }
    APP_LOGI("%{public}s end", __func__);
    return ERR_OK;
}

/**
 * @brief Send event notify to form provider. The event notify type include FORM_VISIBLE and FORM_INVISIBLE.
 * 
 * @param providerKey The provider key string which consists of the provider bundle name and ability name.
 * @param formIdsByProvider The vector of form Ids which have the same provider.
 * @param formVisibleType The form visible type, including FORM_VISIBLE and FORM_INVISIBLE.
 * @return Returns ERR_OK on success, others on failure.
 */
ErrCode FormMgrAdapter::HandleEventNotify(const std::string &providerKey, const std::vector<int64_t> &formIdsByProvider,
    const int32_t formVisibleType)
{
    APP_LOGI("%{public}s called.", __func__);

    sptr<IAbilityConnection> formEventNotifyConnection = new FormEventNotifyConnection(formIdsByProvider, 
    formVisibleType);
    Want connectWant;
    connectWant.AddFlags(Want::FLAG_ABILITY_FORM_ENABLED);
    size_t position = providerKey.find(Constants::NAME_DELIMITER);
    connectWant.SetElementName(providerKey.substr(0, position), 
    providerKey.substr(position + Constants::NAME_DELIMITER.size()));

    ErrCode errorCode = FormAmsHelper::GetInstance().ConnectServiceAbility(connectWant, formEventNotifyConnection);
    if (errorCode != ERR_OK) {
        APP_LOGE("%{public}s fail, ConnectServiceAbility failed.", __func__);
        return errorCode;
    }

    return ERR_OK;
}

/**
 * @brief Acquire form data from form provider.
 * @param formId The Id of the form.
 * @param info Form configure info.
 * @param wantParams WantParams of the request.
 * @return Returns ERR_OK on success, others on failure.
 */
ErrCode FormMgrAdapter::AcquireProviderFormInfoAsync(const int64_t formId, const FormItemInfo &info, 
const WantParams &wantParams)
{
    if (formId <= 0) {
        APP_LOGE("%{public}s fail, formId should be greater than 0", __func__);
        return ERR_APPEXECFWK_FORM_INVALID_PARAM;
    }

    sptr<IAbilityConnection> formAcquireConnection 
        = new FormAcquireConnection(formId, info, wantParams);
    Want want;
    want.SetElementName(info.GetProviderBundleName(), info.GetAbilityName());
    want.AddFlags(Want::FLAG_ABILITY_FORM_ENABLED);
    return FormAmsHelper::GetInstance().ConnectServiceAbility(want, formAcquireConnection);
}

/**
 * @brief Get bundle info.
 * @param want The want of the request.
 * @param bundleInfo Bundle info.
 * @param packageName Package name.
 * @return Returns ERR_OK on success, others on failure.
 */
ErrCode FormMgrAdapter::GetBundleInfo(const AAFwk::Want &want, BundleInfo &bundleInfo, std::string &packageName)
{
    APP_LOGD("GetBundleMgr start.");
    std::string bundleName = want.GetElement().GetBundleName();
    std::string abilityName = want.GetElement().GetAbilityName();
    std::string deviceId = want.GetElement().GetDeviceID();
    std::string moduleName = want.GetStringParam(Constants::PARAM_MODULE_NAME_KEY);
    if (bundleName.empty() || abilityName.empty() || moduleName.empty()) {
        APP_LOGE("GetBundleInfo bundleName or abilityName or moduleName is invalid");
        return ERR_APPEXECFWK_FORM_INVALID_PARAM;
    }

    sptr<IBundleMgr> iBundleMgr = FormBmsHelper::GetInstance().GetBundleMgr();
    if (iBundleMgr == nullptr) {
        APP_LOGE("GetBundleMgr, failed to get IBundleMgr.");
        return ERR_APPEXECFWK_FORM_GET_BUNDLE_FAILED;
    }

    if (iBundleMgr->GetBundleInfo(bundleName, AppExecFwk::BundleFlag::GET_BUNDLE_WITH_ABILITIES, bundleInfo) != true) {
        APP_LOGE("GetBundleInfo, failed to get bundle info.");
        return ERR_APPEXECFWK_FORM_GET_INFO_FAILED;
    }

    bool moduleExist = false;
    for (const auto &moduleInfo : bundleInfo.moduleNames) {
        APP_LOGD("bundleInfo.moduleNames, module name:%{public}s", moduleInfo.c_str());
        if (moduleInfo.compare(moduleName) == 0) {
            moduleExist = true;
            break;
        }
    }
    if (!moduleExist) {
        APP_LOGE("GetBundleInfo no such module, name:%{public}s", moduleName.c_str());
        return ERR_APPEXECFWK_FORM_NO_SUCH_MODULE;
    }

    for (const auto &abilityInfo : bundleInfo.abilityInfos) {        
        // if (abilityInfo.moduleName.compare(GetFullClassName(abilityInfo.package, abilityName)) == 0) {
        //     packageName = abilityInfo.package;
        //     break;
        // }
        if (abilityInfo.deviceId == deviceId) {
            packageName = abilityInfo.package;
            break;
        }
    }

    if (packageName.empty()) {
        APP_LOGE("GetBundleInfo can not find target ability %{public}s", abilityName.c_str());
        return ERR_APPEXECFWK_FORM_NO_SUCH_ABILITY;
    }
    APP_LOGD("GetBundleMgr end.");
    return ERR_OK;
}
/**
 * @brief Get form info.
 * @param want The want of the request.
 * @param packageName Package name.
 * @param formInfo Form info.
 * @return Returns ERR_OK on success, others on failure.
 */
ErrCode FormMgrAdapter::GetFormInfo(const AAFwk::Want &want, FormInfo &formInfo)
{
    APP_LOGD("GetFormInfo start.");
    std::string bundleName = want.GetElement().GetBundleName();
    std::string abilityName = want.GetElement().GetAbilityName();
    std::string moduleName = want.GetStringParam(Constants::PARAM_MODULE_NAME_KEY);
    if (bundleName.empty() || abilityName.empty() || moduleName.empty()) {
        APP_LOGE("addForm bundleName or abilityName or moduleName is invalid");
        return ERR_APPEXECFWK_FORM_INVALID_PARAM;
    }
    sptr<IBundleMgr> iBundleMgr = FormBmsHelper::GetInstance().GetBundleMgr();
    if (iBundleMgr == nullptr) {
        APP_LOGE("GetFormInfo, failed to get IBundleMgr.");
        return ERR_APPEXECFWK_FORM_GET_BUNDLE_FAILED;
    }
    
    std::vector<FormInfo> formInfos;
    if (iBundleMgr->GetFormsInfoByModule(bundleName, moduleName, formInfos) == false) {
        APP_LOGE("GetFormsInfoByModule,  failed to get form config info.");
        return ERR_APPEXECFWK_FORM_GET_INFO_FAILED;
    }

    std::string formName = want.GetStringParam(Constants::PARAM_FORM_NAME_KEY);
    if (formName.empty()) {
        for (const auto &form : formInfos) {
            if (form.defaultFlag) {
                formInfo = form;
                formInfo.moduleName = moduleName;
                APP_LOGD("GetFormInfo end.");
                return ERR_OK;
            }
        }
    } else  {
        for (const auto &form : formInfos) {
            if (form.name == formName) {
                formInfo = form;
                formInfo.moduleName = moduleName;
                APP_LOGD("GetFormInfo end.");
                return ERR_OK;
            }
        }
    }
    APP_LOGE("failed to get form info failed.");
    return ERR_APPEXECFWK_FORM_GET_INFO_FAILED;
}
/**
 * @brief Get form configure info.
 * @param want The want of the request.
 * @param bundleInfo Bundle info.
 * @param formInfo Form info.
 * @param formItemInfo Form configure info.
 * @return Returns ERR_OK on success, others on failure.
 */
ErrCode FormMgrAdapter::GetFormItemInfo(const AAFwk::Want &want, const BundleInfo &bundleInfo, 
    const FormInfo &formInfo, FormItemInfo &formItemInfo)
{
    APP_LOGD("GetFormItemInfo start.");
    int32_t dimensionId = want.GetIntParam(Constants::PARAM_FORM_DIMENSION_KEY, formInfo.defaultDimension);
    if (!IsDimensionValid(formInfo, dimensionId)) {
        APP_LOGE("addForm, dimension is not valid");
        return ERR_APPEXECFWK_FORM_NO_SUCH_DIMENSION;
    }

    // int32_t orientation = want.GetIntParam(Constants::PARAM_FORM_ORIENTATION_KEY, Constants::ORIENTATION_PORTRAIT);
    // if (orientation != Constants::ORIENTATION_PORTRAIT && orientation != Constants::ORIENTATION_LANDSCAPE) {
    //     orientation = Constants::ORIENTATION_PORTRAIT;
    // }

    if (ErrCode ret = CreateFormItemInfo(bundleInfo, formInfo, formItemInfo); ret != ERR_OK) {
        return ret;
    }
    // formItemInfo.SetFormId(formId);
    // formItemInfo.orientation_ = orientation;
    formItemInfo.SetSpecificationId(dimensionId);
    formItemInfo.SetTemporaryFlag(want.GetBoolParam(Constants::PARAM_FORM_TEMPORARY_KEY, false));

    APP_LOGD("GetFormItemInfo end.");
    return ERR_OK;
}
/**
 * @brief Dimension valid check.
 * @param formInfo Form info.
 * @param dimensionId Dimension id.
 * @return Returns true on success, false on failure.
 */   
bool FormMgrAdapter::IsDimensionValid(const FormInfo &formInfo, int dimensionId) const
{
    if (formInfo.supportDimensions.empty()) {
        APP_LOGE("Js form, no support dimension.");
        return false;
    }

    for (size_t i = 0; i < formInfo.supportDimensions.size() && i < Constants::MAX_LAYOUT; i++) {
        int supportDimensionId = formInfo.supportDimensions[i];
        if (supportDimensionId == dimensionId) {
            return true;
        }
    }

    APP_LOGE("No matched dimension found.");
    return false;
}
/**
 * @brief Create form configure info.
 * @param bundleInfo Bundle info.
 * @param formInfo Form info.
 * @param itemInfo Form configure info.
 */
ErrCode FormMgrAdapter::CreateFormItemInfo(const BundleInfo &bundleInfo, const FormInfo &formInfo, 
FormItemInfo &itemInfo)
{
    itemInfo.SetProviderBundleName(bundleInfo.name);
    sptr<IBundleMgr> iBundleMgr = FormBmsHelper::GetInstance().GetBundleMgr();
    if (iBundleMgr == nullptr) {
        APP_LOGE("GetFormInfo, failed to get IBundleMgr.");
        return ERR_APPEXECFWK_FORM_GET_BUNDLE_FAILED;
    }
    std::string hostBundleName {};
    if (!iBundleMgr->GetBundleNameForUid(IPCSkeleton::GetCallingUid(), hostBundleName)) {
        APP_LOGE("GetFormsInfoByModule,  failed to get form config info.");
        return ERR_APPEXECFWK_FORM_GET_INFO_FAILED;
    }
    itemInfo.SetHostBundleName(hostBundleName);
    itemInfo.SetAbilityName(formInfo.abilityName);
    itemInfo.SetModuleName(formInfo.moduleName); // formInfo.moduleName: bundleMagr do not set
    itemInfo.SetFormName(formInfo.name);
    itemInfo.SetEnableUpdateFlag(formInfo.updateEnabled);
    itemInfo.SetUpdateDuration(formInfo.updateDuration);
    itemInfo.SetScheduledUpdateTime(formInfo.scheduledUpateTime);
    itemInfo.SetJsComponentName(formInfo.jsComponentName);
    itemInfo.SetFormVisibleNotify(formInfo.formVisibleNotify);

    for (const auto &abilityInfo : bundleInfo.abilityInfos) {
        if (abilityInfo.name == formInfo.abilityName) {
            itemInfo.SetAbilityModuleName(abilityInfo.moduleName);
        }
    }
    //APP_LOGI("%{public}s moduleInfos size: %{public}d", __func__, bundleInfo.applicationInfo.moduleInfos.size());
    for (const auto &item : bundleInfo.applicationInfo.moduleInfos) {
        APP_LOGI("%{public}s moduleInfos,  moduleName: %{public}s, moduleSourceDir: %{public}s", __func__, item.moduleName.c_str(), item.moduleSourceDir.c_str());
        if (formInfo.moduleName == item.moduleName) {
            itemInfo.AddHapSourceDirs(item.moduleSourceDir);
        }
        itemInfo.AddModuleInfo(item.moduleName, item.moduleSourceDir);
    }
    return ERR_OK;
}

/**
 * @brief set next refresh time.
 * @param formId The id of the form.
 * @param nextTime next refresh time.
 * @return Returns ERR_OK on success, others on failure.
 */
int FormMgrAdapter::SetNextRefreshTime(const int64_t formId, const int64_t nextTime)
{
    //APP_LOGI("%{public}s begin here, formId:%{public}lld,nextTime:%{public}lld", __func__, formId, nextTime);
    if (formId <= 0) {
        APP_LOGE("%{public}s form formId or bundleName is invalid", __func__);
        return ERR_FORM_INVALID_PARAM;
    }
    int64_t matchedFormId = FormDataMgr::GetInstance().FindMatchedFormId(formId);

    std::string bundleName;
    if (!GetBundleName(bundleName)) {
        return ERR_APPEXECFWK_FORM_INFO_NOT_EXIST;
    }

    FormRecord formRecord;
    if (!FormDataMgr::GetInstance().GetFormRecord(matchedFormId, formRecord)) {
        APP_LOGE("%{public}s, not found in formrecord.", __func__);
        return ERR_APPEXECFWK_FORM_INFO_NOT_EXIST;
    }
    // check bundleName
    if (bundleName != formRecord.bundleName) {
        APP_LOGE("%{public}s, not match bundleName:%{public}s", __func__, bundleName.c_str());
        return ERR_OPERATION_FORM_NOT_SELF;
    }

    return SetNextRefreshtTimeLocked(matchedFormId, nextTime);
}

/**
 * @brief get bundleName.
 * @param bundleName for output.
 * @return Returns true on success, others on failure.
 */ 
bool FormMgrAdapter::GetBundleName(std::string &bundleName)
{
    sptr<IBundleMgr> iBundleMgr = FormBmsHelper::GetInstance().GetBundleMgr();
    if (iBundleMgr == nullptr) {
        APP_LOGE("%{public}s, failed to get IBundleMgr.", __func__);
        return false;
    }

    int32_t uid = IPCSkeleton::GetCallingUid();
    if (!iBundleMgr->CheckIsSystemAppByUid(uid)) {
        APP_LOGE("%{public}s fail, form is not system app. uid:%{public}d", __func__, uid);
        return false;
    }

    bool result = iBundleMgr->GetBundleNameForUid(uid, bundleName);
    if (!result || bundleName.empty()) {
        APP_LOGE("%{public}s failed, cannot get bundle name by uid:%{public}d", __func__, uid);
        return false;
    }
    return true;
}

/**
 * @brief set next refresht time locked.
 * @param formId The form's id.
 * @param nextTime next refresh time.
 * @return Returns ERR_OK on success, others on failure.
 */
int FormMgrAdapter::SetNextRefreshtTimeLocked(const int64_t formId, const int64_t nextTime)
{
    APP_LOGE("SetNextRefreshtTimeLocked.");
    int32_t timerRefreshedCount = FormTimerMgr::GetInstance().GetRefreshCount(formId);
    if (timerRefreshedCount >= Constants::LIMIT_COUNT) {
        APP_LOGE("%{public}s, already refresh times:%{public}d", __func__, timerRefreshedCount);
        FormTimerMgr::GetInstance().MarkRemind(formId);
        return ERR_MAX_REFRESH;
    }

    if (!FormTimerMgr::GetInstance().SetNextRefreshTime(formId, nextTime)) {
        APP_LOGE("%{public}s failed", __func__);
        return ERR_CODE_COMMON;
    }

    return ERR_OK;
}

/**
 * @brief set next refresht time locked.
 * @param formId The form's id.
 * @param bundleName Provider ability bundleName.
 * @return Returns true or false.
 */
bool FormMgrAdapter::IsUpdateValid(const int64_t formId, const std::string &bundleName) 
{
    if (formId <= 0 || bundleName.empty()) {
        return false;
    }
    return true;
}

/**
 * @brief enable update form.
 * @param formIDs The id of the forms.
 * @param callerToken Caller ability token.
 * @return Returns ERR_OK on success, others on failure.
 */
int FormMgrAdapter::EnableUpdateForm(const std::vector<int64_t> formIDs, const sptr<IRemoteObject> &callerToken)
{
    APP_LOGI("enableUpdateForm");
    return HandleUpdateFormFlag(formIDs, callerToken, true);
}

/**
 * @brief disable update form.
 * @param formIDs The id of the forms.
 * @param callerToken Caller ability token.
 * @return Returns ERR_OK on success, others on failure.
 */
int FormMgrAdapter::DisableUpdateForm(const std::vector<int64_t> formIDs, const sptr<IRemoteObject> &callerToken)
{
    APP_LOGI("disableUpdateForm");
    return HandleUpdateFormFlag(formIDs, callerToken, false);
}

/**
 * @brief Process js message event.
 * @param formId Indicates the unique id of form.
 * @param want information passed to supplier.
 * @param callerToken Caller ability token.
 * @return Returns true if execute success, false otherwise.
 */
int FormMgrAdapter::MessageEvent(const int64_t formId, const Want &want, const sptr<IRemoteObject> &callerToken)
{
    APP_LOGI("%{public}s called.", __func__);
    if (formId <= 0) {
        APP_LOGE("%{public}s form formId or bundleName is invalid", __func__);
        return ERR_APPEXECFWK_FORM_INVALID_PARAM;
    }

    if (callerToken == nullptr) {
        APP_LOGE("%{public}s failed, callerToken can not be NULL", __func__);
        return ERR_APPEXECFWK_FORM_INVALID_PARAM;
    }

    if (!want.HasParameter(Constants::PARAM_MESSAGE_KEY)) {
        APP_LOGE("%{public}s failed, message info is not exist", __func__);
        return ERR_APPEXECFWK_FORM_INVALID_PARAM;
    }

    int64_t matchedFormId = FormDataMgr::GetInstance().FindMatchedFormId(formId);
    FormRecord record;
    bool bGetRecord = FormDataMgr::GetInstance().GetFormRecord(matchedFormId, record);
    if (!bGetRecord) {
        //APP_LOGE("%{public}s fail, not exist such form:%{public}lld", __func__, matchedFormId);
        return ERR_APPEXECFWK_FORM_INFO_NOT_EXIST;
    }
    
    FormHostRecord formHostRecord; 
    bool isHostExist = FormDataMgr::GetInstance().GetMatchedHostClient(callerToken, formHostRecord);
    if (!isHostExist) {
        APP_LOGE("%{public}s failed, cannot find target client.", __func__);
        return ERR_APPEXECFWK_FORM_HOST_INFO_NOT_EXIST;
    }

    if (!formHostRecord.Contains(matchedFormId)) {
        APP_LOGE("%{public}s failed, form is not self-owned.", __func__);
        return ERR_OPERATION_FORM_NOT_SELF;
    }

    APP_LOGI("%{public}s, find target client.", __func__);
    return FormProviderMgr::GetInstance().MessageEvent(matchedFormId, record, want);
}

int FormMgrAdapter::HandleUpdateFormFlag(std::vector<int64_t> formIds, const sptr<IRemoteObject> &callerToken, 
bool flag)
{
    APP_LOGI("%{public}s called.", __func__);
    if (formIds.empty() || callerToken == nullptr) {
        APP_LOGE("%{public}s, invalid param", __func__);
        return ERR_FORM_INVALID_PARAM;
    }
    std::vector<int64_t> refreshForms;
    int errCode =  FormDataMgr::GetInstance().UpdateHostFormFlag(formIds, callerToken, flag, refreshForms);
    if (errCode == ERR_OK && refreshForms.size() > 0) {
        for (const int64_t id : refreshForms) {
            //APP_LOGI("%{public}s, formRecord need refresh: %{public}lld", __func__, id);
            Want want;
            FormProviderMgr::GetInstance().RefreshForm(id, want);
        }
    }
    return errCode;
}

/**
 * @brief handle update form flag.
 * @param formIDs The id of the forms.
 * @param callerToken Caller ability token.
 * @param flag form flag.
 * @return Returns ERR_OK on success, others on failure.
 */
bool FormMgrAdapter::IsFormCached(const FormRecord record) {
    if (record.versionUpgrade) {
        return false;
    }
    return true;
}
}  // namespace AppExecFwk
}  // namespace OHOS
