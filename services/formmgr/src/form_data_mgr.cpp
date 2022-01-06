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
#include "form_cache_mgr.h"
#include "form_constants.h"
#include "form_data_mgr.h"
#include "form_provider_mgr.h"
#include "form_util.h"
#include "ipc_skeleton.h"


namespace OHOS {
namespace AppExecFwk {
FormDataMgr::FormDataMgr()
{
    APP_LOGI("create form data manager instance");
    udidHash_ = 0L;
}
FormDataMgr::~FormDataMgr()
{
    APP_LOGI("destroy form data manager instance");
}

/**
 * @brief Allot form info by item info.
 * @param formId The Id of the form.
 * @param formInfo Form item info.
 * @param callingUid The UID of the proxy.
 * @return Returns form record.
 */
FormRecord FormDataMgr::AllotFormRecord(const FormItemInfo &formInfo, const int callingUid)
{
    APP_LOGI("%{public}s, allot form info", __func__);
    if (formInfo.IsTemporaryForm() && !ExistTempForm(formInfo.GetFormId())) {
        std::lock_guard<std::mutex> lock(formTempMutex_);        
        tempForms_.emplace_back(formInfo.GetFormId());
    }
    FormRecord record;
    {        
        std::lock_guard<std::mutex> lock(formRecordMutex_);        
        if (formRecords_.empty()) { // formRecords_ is empty, create a new one    
            APP_LOGD("%{public}s, form info not exist", __func__);
            record = CreateFormRecord(formInfo, callingUid);
            formRecords_.emplace(formInfo.GetFormId(), record);
        } else {
            auto info = formRecords_.find(formInfo.GetFormId());
            if (info == formRecords_.end()) {
                APP_LOGD("%{public}s, form info not find", __func__);
                record = CreateFormRecord(formInfo, callingUid);
                formRecords_.emplace(formInfo.GetFormId(), record);
            } else {
                record = info->second;
            }
        }
    }
    APP_LOGI("%{public}s end", __func__);
    return record;
}
/**
 * @brief Delete form js info by form record.
 * @param formId The Id of the form.
 * @return Returns true if this function is successfully called; returns false otherwise.
 */
bool FormDataMgr::DeleteFormRecord(const int64_t formId) 
{
    APP_LOGI("%{public}s, delete form info", __func__);
    std::lock_guard<std::mutex> lock(formRecordMutex_);
    auto iter = formRecords_.find(formId);
    if (iter == formRecords_.end()) {
        APP_LOGE("%{public}s, form info is not exist", __func__);
        return true;
    }
    formRecords_.erase(iter);
    return true;
}
/**
 * @brief Allot form host record by caller token.
 * @param info The form item info.
 * @param callerToken callerToken
 * @param formId The Id of the form.
 * @param callingUid The UID of the proxy.
 * @param record Form host record. 
 * @return Returns true if this function is successfully called; returns false otherwise.
 */
bool FormDataMgr::AllotFormHostRecord(const FormItemInfo &info, const sptr<IRemoteObject> &callerToken,
    const int64_t formId, const int callingUid)
{
    APP_LOGI("%{public}s, allot form Host info", __func__);
    std::lock_guard<std::mutex> lock(formHostRecordMutex_);
    for (auto &record : clientRecords_) {
        if (callerToken == record.GetClientStub()) {
            record.AddForm(formId);
            APP_LOGI("%{public}s end", __func__);
            return true;
        }
    }
    FormHostRecord hostRecord;
    bool isCreated = CreateHostRecord(info, callerToken, callingUid, hostRecord);
    if (isCreated) {
        hostRecord.AddForm(formId);
        clientRecords_.emplace_back(hostRecord);
        APP_LOGI("%{public}s end", __func__);
        return true;
    }
    APP_LOGI("%{public}s end", __func__);
    return false;
}
/**
 * @brief Create host record.
 * @param info The form item info.
 * @param callerToken The UID of the proxy.
 * @param callingUid The UID of the proxy.
 * @param record The form host record.
 * @return Returns true if this function is successfully called; returns false otherwise.
 */
bool FormDataMgr::CreateHostRecord(const FormItemInfo &info, const sptr<IRemoteObject> &callerToken, 
    const int callingUid, FormHostRecord& record) 
{
    if (callerToken == nullptr) {
        APP_LOGE("%{public}s, invalid param", __func__);
        return false;
    }

    record = FormHostRecord::CreateRecord(info, callerToken, callingUid);
    return true;
}
/**
 * @brief Create form record.
 * @param formInfo The form item info.
 * @param callingUid The UID of the proxy.
 * @return Form record.
 */
FormRecord FormDataMgr::CreateFormRecord(const FormItemInfo &formInfo, const int callingUid) const
{
    APP_LOGI("%{public}s, create form info", __func__);
    FormRecord newRecord;
    newRecord.formId = formInfo.GetFormId();
    newRecord.packageName = formInfo.GetPackageName();
    newRecord.bundleName = formInfo.GetProviderBundleName();
    newRecord.moduleName = formInfo.GetModuleName();
    newRecord.abilityName = formInfo.GetAbilityName();
    newRecord.formName = formInfo.GetFormName();
    newRecord.specification = formInfo.GetSpecificationId();
    newRecord.isEnableUpdate = formInfo.IsEnableUpdateFlag();
    newRecord.formTempFlg = formInfo.IsTemporaryForm();
    newRecord.formVisibleNotify = formInfo.IsFormVisibleNotify();
    newRecord.jsFormCodePath = formInfo.GetHapSourceByModuleName(newRecord.moduleName);
    newRecord.formSrc = formInfo.GetFormSrc();
    newRecord.formWindow = formInfo.GetFormWindow();
    newRecord.versionName = formInfo.GetVersionName();
    newRecord.versionCode = formInfo.GetVersionCode();
    newRecord.compatibleVersion = formInfo.GetCompatibleVersion();
    newRecord.icon = formInfo.GetIcon();

    newRecord.formVisibleNotifyState = 0;
    if (newRecord.isEnableUpdate) {
        ParseUpdateConfig(newRecord, formInfo);
    }
    if (std::find(newRecord.formUserUids.begin(), newRecord.formUserUids.end(),
        callingUid) == newRecord.formUserUids.end()) {
        newRecord.formUserUids.emplace_back(callingUid);
    }

    formInfo.GetHapSourceDirs(newRecord.hapSourceDirs);
    APP_LOGI("%{public}s end", __func__);
    return newRecord;
}
/**
 * @brief Create form js info by form record.
 * @param formId The Id of the form.
 * @param record Form record.
 * @param formInfo Js info.
 * @return None.
 */
void FormDataMgr::CreateFormInfo(const int64_t formId, const FormRecord &record, FormJsInfo &formInfo)
{
    formInfo.formId = formId;
    formInfo.bundleName = record.bundleName;
    formInfo.abilityName = record.abilityName;
    formInfo.formName = record.formName;
    formInfo.formTempFlg = record.formTempFlg;
    formInfo.formSrc = record.formSrc;
    formInfo.formWindow = record.formWindow;
    formInfo.versionCode = record.versionCode;
    formInfo.versionName = record.versionName;
    formInfo.compatibleVersion = record.compatibleVersion;
    formInfo.icon = record.icon;
}
/**
 * @brief Check temp form count is max.
 * @return Returns ERR_OK if the temp form not reached; returns ERR_MAX_SYSTEM_TEMP_FORMS is reached.
 */
int FormDataMgr::CheckTempEnoughForm() const
{
    if (tempForms_.size() >= Constants::MAX_TEMP_FORMS) {
        APP_LOGW("%{public}s, already exist %{public}d temp forms in system", __func__, Constants::MAX_TEMP_FORMS);
        return ERR_APPEXECFWK_FORM_MAX_SYSTEM_TEMP_FORMS;
    }
    return ERR_OK;
}
/**
 * @brief Check form count is max.
 * @param callingUid The UID of the proxy.
 * @return Returns true if this function is successfully called; returns false otherwise.
 */
int FormDataMgr::CheckEnoughForm(const int callingUid) const
{
    APP_LOGI("%{public}s, callingUid: %{public}d", __func__, callingUid);

    if (formRecords_.size() - tempForms_.size() >= Constants::MAX_FORMS) {
        APP_LOGW("%{public}s, already exist %{public}d forms in system", __func__, Constants::MAX_FORMS);
        return ERR_APPEXECFWK_FORM_MAX_SYSTEM_FORMS;
    }

    int callingUidFormCounts = 0;
    for (auto &recordPair : formRecords_) {
        FormRecord record = recordPair.second;
        if (IsCallingUidValid(record.formUserUids) && !record.formTempFlg) {
            for (auto &userUid : record.formUserUids) {
                if (userUid == callingUid) {
                    if (++callingUidFormCounts >= Constants::MAX_RECORD_PER_APP) {
                        APP_LOGW("%{public}s, already use %{public}d forms", __func__, Constants::MAX_RECORD_PER_APP);
                        return ERR_APPEXECFWK_FORM_MAX_FORMS_PER_CLIENT;
                    }
                }
            }
        }
    }
    return ERR_OK;
}
/**
 * @brief Delete temp form.
 * @param formId The Id of the form.
 * @return Returns true if this function is successfully called; returns false otherwise.
 */
bool FormDataMgr::DeleteTempForm(const int64_t formId)
{
    std::lock_guard<std::mutex> lock(formTempMutex_);
    auto iter = std::find(tempForms_.begin(), tempForms_.end(), formId);
    if (iter == tempForms_.end()) {
        APP_LOGE("%{public}s, temp form is not exist", __func__);
        return false;
    }
    tempForms_.erase(iter);
    return true;
}
/**
 * @brief Check temp form is exist.
 * @param formId The Id of the form.
 * @return Returns true if the temp form is exist; returns false is not exist.
 */
bool FormDataMgr::ExistTempForm(const int64_t formId) const
{
    return (std::find(tempForms_.begin(), tempForms_.end(), formId) != tempForms_.end());
}
/**
 * @brief Check calling uid is valid.
 * @param formUserUids The form user uids.
 * @return Returns true if this user uid is valid; returns false otherwise.
 */
bool FormDataMgr::IsCallingUidValid(const std::vector<int32_t> &formUserUids) const
{
    if (formUserUids.size() != 0) {
        for (auto &userUid : formUserUids) {
            if (userUid == IPCSkeleton::GetCallingUid()) {
                return true;
            }
        } 
    }
    return false;
}
/**
 * @brief Modify form temp flag by formId.
 * @param formId The Id of the form.
 * @param formTempFlg The form temp flag.
 * @return Returns true if this function is successfully called; returns false otherwise.
 */
bool FormDataMgr::ModifyFormTempFlg(const int64_t formId, const bool formTempFlg)
{
    APP_LOGI("%{public}s, modify form temp flag by formId", __func__);
    std::lock_guard<std::mutex> lock(formRecordMutex_);
    if (!ExistFormRecord(formId)) {
        APP_LOGE("%{public}s, form info is not exist", __func__);
        return false;
    }
    formRecords_[formId].formTempFlg = formTempFlg;
    return true;
}
/**
 * @brief Add form user uid from form record.
 * @param formId The Id of the form.
 * @param formRecord The form record.
 * @return Returns true if this function is successfully called; returns false otherwise.
 */
bool FormDataMgr::AddFormUserUid(const int64_t formId, const int32_t formUserUid)
{
    APP_LOGI("%{public}s, add form user uid by formId", __func__);
    std::lock_guard<std::mutex> lock(formRecordMutex_);
    if (!ExistFormRecord(formId)) {
        APP_LOGE("%{public}s, form info is not exist", __func__);
        return false;
    }
    if (std::find(formRecords_[formId].formUserUids.begin(), formRecords_[formId].formUserUids.end(),
        formUserUid) == formRecords_[formId].formUserUids.end()) {
        formRecords_[formId].formUserUids.emplace_back(formUserUid);
    }
    return true;
}
/**
 * @brief Delete form user uid from form record.
 * @param formId The Id of the form.
 * @param uid calling user id.
 * @return Returns true if this function is successfully called; returns false otherwise.
 */
bool FormDataMgr::DeleteFormUserUid(const int64_t formId, const int32_t uid)
{
    APP_LOGI("%{public}s, delete form user uid from form record", __func__);
    std::lock_guard<std::mutex> lock(formRecordMutex_);
    if (ExistFormRecord(formId)) {
        auto iter = std::find(formRecords_.at(formId).formUserUids.begin(), 
            formRecords_.at(formId).formUserUids.end(), uid);
        if (iter != formRecords_.at(formId).formUserUids.end()) {
            formRecords_.at(formId).formUserUids.erase(iter);
        }
        return true;
    } else {
        APP_LOGE("%{public}s, form info not find", __func__);
        return false;
    }
}
/**
 * @brief Update form record.
 * @param formId The Id of the form.
 * @param formRecord The form record.
 * @return Returns true if this function is successfully called; returns false otherwise.
 */
bool FormDataMgr::UpdateFormRecord(const int64_t formId, const FormRecord &formRecord)
{
    APP_LOGI("%{public}s, get form record by formId", __func__);
    std::lock_guard<std::mutex> lock(formRecordMutex_);
    auto info = formRecords_.find(formId);
    if (info != formRecords_.end()) {
        formRecords_[formId] = formRecord;
        return true;
    }
    return false;
}
/**
 * @brief Get form record.
 * @param formId The Id of the form.
 * @param formRecord The form record.
 * @return Returns true if this function is successfully called; returns false otherwise.
 */
bool FormDataMgr::GetFormRecord(const int64_t formId, FormRecord &formRecord) const
{
    APP_LOGI("%{public}s, get form record by formId", __func__);
    std::lock_guard<std::mutex> lock(formRecordMutex_);
    auto info = formRecords_.find(formId);
    if (info == formRecords_.end()) {
        APP_LOGE("%{public}s, form info not find", __func__);
        return false;
    }
    formRecord = info->second;
 
    APP_LOGI("%{public}s, get form record successfully", __func__);
    return true;
}
/**
 * @brief Get form record.
 * @param bundleName Bundle name.
 * @param formInfos The form record.
 * @return Returns true if this function is successfully called; returns false otherwise.
 */
bool FormDataMgr::GetFormRecord(const std::string &bundleName, std::vector<FormRecord> &formInfos)
{
    APP_LOGI("%{public}s, get form record by bundleName", __func__);
    std::lock_guard<std::mutex> lock(formRecordMutex_);
    std::map<int64_t, FormRecord>::iterator itFormRecord;
    for (itFormRecord = formRecords_.begin(); itFormRecord != formRecords_.end(); itFormRecord++) {
        if (bundleName == itFormRecord->second.bundleName) {
            formInfos.emplace_back(itFormRecord->second);
        }
    }
    if (formInfos.size() > 0) {        
        return true;
    } else {
        APP_LOGI("%{public}s, form info not find", __func__);
        return false;
    }
}
/**
 * @brief Check form record is exist.
 * @param formId The Id of the form.
 * @return Returns true if the form record is exist; returns false is not exist.
 */
bool FormDataMgr::ExistFormRecord(const int64_t formId) const
{
    APP_LOGI("%{public}s, check form record is exist", __func__);
    return (formRecords_.count(formId) > 0);
}
/**
 * @brief Has form user uids in form record.
 * @param formId The Id of the form.
 * @return Returns true if this form has form user uids; returns false is not has.
 */
bool FormDataMgr::HasFormUserUids(const int64_t formId) const
{
    APP_LOGI("%{public}s, check form has user uids", __func__);
    FormRecord record;
    if (GetFormRecord(formId, record)) {
        return record.formUserUids.empty() ? false : true;
    }
    return false;
}
/**
 * @brief Get form host record.
 * @param formId The id of the form.
 * @param formHostRecord The form host record.
 * @return Returns true if this function is successfully called; returns false otherwise.
 */
bool FormDataMgr::GetFormHostRecord(const int64_t formId, FormHostRecord &formHostRecord) const
{
    APP_LOGI("%{public}s, get form host record by formId", __func__);
    std::lock_guard<std::mutex> lock(formHostRecordMutex_);
    for (auto &record : clientRecords_) {
        if (record.Contains(formId)) {
            formHostRecord = record;
            return true;
        }
    }
    
    APP_LOGE("%{public}s, form host record not find", __func__);
    return false;
}
/**
 * @brief Delete form host record.
 * @param callerToken The client stub of the form host record.
 * @param formId The id of the form.
 * @return Returns true if this function is successfully called; returns false otherwise.
 */
bool FormDataMgr::DeleteHostRecord(const sptr<IRemoteObject> &callerToken, const int64_t formId)
{
    APP_LOGI("%{public}s start, delete form host record", __func__);
    std::lock_guard<std::mutex> lock(formHostRecordMutex_);
    std::vector<FormHostRecord>::iterator iter;
    for (iter = clientRecords_.begin(); iter != clientRecords_.end(); ++iter) {
        if (callerToken == iter->GetClientStub()) {
            iter->DelForm(formId);
            if (iter->IsEmpty()) {
                iter->CleanResource();
                iter = clientRecords_.erase(iter);
            }
            break;
        }
    }
    APP_LOGI("%{public}s end", __func__);
    return true;
}
/**
 * @brief Clean removed forms form host.
 * @param removedFormIds The id list of the forms.
 */
void FormDataMgr::CleanHostRemovedForms(const std::vector<int64_t> &removedFormIds)
{
    APP_LOGI("%{public}s start, delete form host record by formId list", __func__);
    std::vector<int64_t> matchedIds;
    std::lock_guard<std::mutex> lock(formHostRecordMutex_);
    std::vector<FormHostRecord>::iterator itHostRecord;
    for (itHostRecord = clientRecords_.begin(); itHostRecord != clientRecords_.end(); itHostRecord++) {
        for (const int64_t& formId : removedFormIds) {
            if (itHostRecord->Contains(formId)) {
                matchedIds.emplace_back(formId);
                itHostRecord->DelForm(formId);
            }
        }
        if (!matchedIds.empty()) {
            APP_LOGI("%{public}s, OnFormUninstalled called", __func__);
            itHostRecord->OnFormUninstalled(matchedIds);
        }
    }

    APP_LOGI("%{public}s end", __func__);
}
/**
 * @brief Handle form host died.
 * @param remoteHost Form host proxy object.
 */
void FormDataMgr::HandleHostDied(const sptr<IRemoteObject> &remoteHost)
{
    std::vector<int64_t> recordTempForms;
    {
        std::lock_guard<std::mutex> lock(formHostRecordMutex_);
        std::vector<FormHostRecord>::iterator itHostRecord;
        for (itHostRecord = clientRecords_.begin(); itHostRecord != clientRecords_.end(); ) {
            if (remoteHost == itHostRecord->GetClientStub()) {
                HandleHostDiedForTempForms(*itHostRecord, recordTempForms);
                APP_LOGI("find died client, remove it");
                itHostRecord->CleanResource();
                itHostRecord = clientRecords_.erase(itHostRecord);
                break;
            } else {
                itHostRecord++;
            }
        }
    }
    {
        std::lock_guard<std::mutex> lock(formRecordMutex_);
        std::map<int64_t, FormRecord>::iterator itFormRecord;
        for (itFormRecord = formRecords_.begin(); itFormRecord != formRecords_.end(); ) {
            int64_t formId = itFormRecord->first;
            // if temp form, remove it
            if (std::find(recordTempForms.begin(), recordTempForms.end(), formId) != recordTempForms.end()) {
                FormRecord formRecord = itFormRecord->second;
                itFormRecord = formRecords_.erase(itFormRecord);
                FormProviderMgr::GetInstance().NotifyProviderFormDelete(formId, formRecord);
            } else {
                itFormRecord++;
            }
        }
    }
}

/**
 * @brief Get the temp forms from host and delete temp form in cache.
 * @param record The form record.
 * @param recordTempForms Getted the temp forms.
 */
void FormDataMgr::HandleHostDiedForTempForms(const FormHostRecord &record, std::vector<int64_t> &recordTempForms)
{
    std::lock_guard<std::mutex> lock(formTempMutex_);
    std::vector<int64_t>::iterator itForm;
    for (itForm = tempForms_.begin(); itForm != tempForms_.end(); ) {
        if (record.Contains(*itForm)) {
            recordTempForms.emplace_back(*itForm);
            itForm = tempForms_.erase(itForm);
        } else {
            itForm++;
        }
    }
}

/**
 * @brief Refresh enable or not.
 * @param formId The Id of the form.
 * @return true on enbale, false on disable.
 */
bool FormDataMgr::IsEnableRefresh(int64_t formId)
{
    std::lock_guard<std::mutex> lock(formHostRecordMutex_);
    for (auto &record : clientRecords_) {
        if (record.IsEnableRefresh(formId)) {
            return true;
        }
    }

    return false;
}
/**
 * @brief Generate form id.
 * @return form id.
 */
int64_t FormDataMgr::GenerateFormId()
{
    // generate udidHash_
    if (udidHash_ < 0) {
        APP_LOGE("%{public}s fail, generateFormId no invalid udidHash_", __func__);
        return ERR_APPEXECFWK_FORM_COMMON_CODE;
    }
    return FormUtil::GenerateFormId(udidHash_);
}
/**
 * @brief Generate udid.
 * @return Returns true if this function is successfully called; returns false otherwise.
 */
bool FormDataMgr::GenerateUdidHash()
{
    if (udidHash_ != Constants::INVALID_UDID_HASH) {
        return true;
    }
    
    bool bGenUdid = FormUtil::GenerateUdidHash(udidHash_);
    if (!bGenUdid) {
        APP_LOGE("%{public}s, Failed to generate udid.", __func__);
        return false;
    }
    
    return true;
}
/**
 * @brief Get udid.
 * @return udid.
 */
int64_t FormDataMgr::GetUdidHash() const
{
    return udidHash_;
}
/**
 * @brief Set udid.
 * @param udidHash udid.
 */
void FormDataMgr::SetUdidHash(const int64_t udidHash)
{
    udidHash_ = udidHash;
}

/**
 * @brief Get the matched form host record by client stub.
 * 
 * @param callerToken The client stub of the form host record.
 * @param formHostRecord The form host record.
 * @return Returns true if this function is successfully called, returns false otherwise.
 */
bool FormDataMgr::GetMatchedHostClient(const sptr<IRemoteObject> &callerToken, FormHostRecord &formHostRecord) const
{
    APP_LOGI("%{public}s, get the matched form host record by client stub.", __func__);
    std::lock_guard<std::mutex> lock(formHostRecordMutex_);
    for (const FormHostRecord &record : clientRecords_) {
        if (callerToken == record.GetClientStub()) {
            formHostRecord = record;
            return true;
        }
    }

    APP_LOGE("%{public}s, form host record not find.", __func__);
    return false;
}

/**
 * @brief Set needRefresh for FormRecord.
 * @param formId The Id of the form.
 * @param needRefresh true or false.
 */
void FormDataMgr::SetNeedRefresh(const int64_t formId, const bool needRefresh)
{
    std::lock_guard<std::mutex> lock(formRecordMutex_);
    auto itFormRecord = formRecords_.find(formId);
    if (itFormRecord == formRecords_.end()) {
        APP_LOGE("%{public}s, form info not find", __func__);
        return;
    }
    itFormRecord->second.needRefresh = needRefresh;
}

/**
 * @brief Set isCountTimerRefresh for FormRecord.
 * @param formId The Id of the form.
 * @param countTimerRefresh true or false.
 */
void FormDataMgr::SetCountTimerRefresh(const int64_t formId, const bool countTimerRefresh)
{
    std::lock_guard<std::mutex> lock(formRecordMutex_);
    auto itFormRecord = formRecords_.find(formId);
    if (itFormRecord == formRecords_.end()) {
        APP_LOGE("%{public}s, form info not find", __func__);
        return;
    }
    itFormRecord->second.isCountTimerRefresh = countTimerRefresh;
}

/**
 * @brief Get updated form.
 * @param record FormRecord.
 * @param targetForms Target forms. 
 * @param updatedForm Updated formnfo.
 * @return Returns true on success, false on failure.
 */
bool FormDataMgr::GetUpdatedForm(
    const FormRecord &record, 
    const std::vector<FormInfo> &targetForms, 
    FormInfo &updatedForm)
{
    if (targetForms.empty()) {
        APP_LOGE("%{public}s error, targetForms is empty.", __func__);
        return false;
    }

    for (const FormInfo &item : targetForms) {
        if (IsSameForm(record, item)) {
            updatedForm = item;
            APP_LOGD("%{public}s, find matched form.", __func__);
            return true;
        }
    }
    return false;
}
/**
 * @brief Set isEnableUpdate for FormRecord.
 * @param formId The Id of the form.
 * @param enableUpdate true or false.
 */
void FormDataMgr::SetEnableUpdate(const int64_t formId, const bool enableUpdate)
{
    std::lock_guard<std::mutex> lock(formRecordMutex_);
    auto itFormRecord = formRecords_.find(formId);
    if (itFormRecord == formRecords_.end()) {
        APP_LOGE("%{public}s, form info not find", __func__);
        return;
    }
    itFormRecord->second.isEnableUpdate = enableUpdate;    
}
/**
 * @brief Set update info for FormRecord.
 * @param formId The Id of the form.
 * @param enableUpdate true or false.
 * @param updateDuration Update duration.
 * @param updateAtHour Update at hour.
 * @param updateAtMin Update at minute.
 */
void FormDataMgr::SetUpdateInfo(
    const int64_t formId, 
    const bool enableUpdate, 
    const long updateDuration, 
    const int updateAtHour, 
    const int updateAtMin)
{
    std::lock_guard<std::mutex> lock(formRecordMutex_);
    auto itFormRecord = formRecords_.find(formId);
    if (itFormRecord == formRecords_.end()) {
        APP_LOGE("%{public}s, form info not find", __func__);
        return;
    }

    itFormRecord->second.isEnableUpdate = enableUpdate;
    itFormRecord->second.updateDuration = updateDuration;
    itFormRecord->second.updateAtHour = updateAtHour;
    itFormRecord->second.updateAtMin = updateAtMin;
}
/**
 * @brief Check if two forms is same or not.
 * @param record FormRecord.
 * @param formInfo FormInfo.
 * @return Returns true on success, false on failure.
 */
bool FormDataMgr::IsSameForm(const FormRecord &record, const FormInfo &formInfo)
{
    if (record.bundleName == formInfo.bundleName
        && record.moduleName == formInfo.moduleName
        && record.abilityName == formInfo.abilityName
        && record.formName == formInfo.name
        && std::find(formInfo.supportDimensions.begin(), formInfo.supportDimensions.end(), record.specification)
        != formInfo.supportDimensions.end()) {
        return true;
    }

    return false;
}
/**
 * @brief Clean removed form records.
 * @param removedForms The id list of the forms.
 */
void FormDataMgr::CleanRemovedFormRecords(const std::string &bundleName, std::set<int64_t> &removedForms)
{
    APP_LOGI("%{public}s, clean removed form records", __func__);
    std::lock_guard<std::mutex> lock(formRecordMutex_);
    std::map<int64_t, FormRecord>::iterator itFormRecord;
    for (itFormRecord = formRecords_.begin(); itFormRecord != formRecords_.end(); ) {
        auto itForm = std::find(removedForms.begin(), removedForms.end(), itFormRecord->first);
        if (itForm != removedForms.end()) {
            itFormRecord = formRecords_.erase(itFormRecord);
        } else {
            itFormRecord++;
        }
    }
}
/**
 * @brief Clean removed temp form records.
 * @param  bundleName BundleName.
 * @param removedForms The id list of the forms.
 */
void FormDataMgr::CleanRemovedTempFormRecords(const std::string &bundleName, std::set<int64_t> &removedForms)
{
    APP_LOGI("%{public}s, clean removed form records", __func__);
    std::set<int64_t> removedTempForms;
    {
        std::lock_guard<std::mutex> lock(formRecordMutex_);
        std::map<int64_t, FormRecord>::iterator itFormRecord;
        for (itFormRecord = formRecords_.begin(); itFormRecord != formRecords_.end(); ) {
            if (itFormRecord->second.formTempFlg && bundleName == itFormRecord->second.bundleName) {
                removedTempForms.emplace(itFormRecord->second.formId);
                itFormRecord = formRecords_.erase(itFormRecord);
            } else {
                itFormRecord++;
            }
        }
    }
    
    if (removedTempForms.size() > 0) {
        std::lock_guard<std::mutex> lock(formTempMutex_);
        std::vector<int64_t>::iterator itTemp;
        for (itTemp = tempForms_.begin();itTemp != tempForms_.end(); ) {
            if (removedTempForms.find(*itTemp) != removedTempForms.end()) {
                itTemp = tempForms_.erase(itTemp);
            } else {
                itTemp++;
            }
        }
        removedForms.merge(removedTempForms);
    }
}
/**
 * @brief Get recreate form records.
 * @param reCreateForms The id list of the forms.
 */
void FormDataMgr::GetReCreateFormRecordsByBundleName(const std::string &bundleName, std::set<int64_t> &reCreateForms)
{
    std::lock_guard<std::mutex> lock(formRecordMutex_);
    std::map<int64_t, FormRecord>::iterator itFormRecord;
    for (itFormRecord = formRecords_.begin(); itFormRecord != formRecords_.end(); itFormRecord++) {
        if (bundleName == itFormRecord->second.bundleName) {
            reCreateForms.emplace(itFormRecord->second.formId);
        }
    }
}
/**
 * @brief Set form isInited = true.
 * @param formId The Id of the form.
 * @param isInited isInited property
 */
void FormDataMgr::SetFormCacheInited(const int64_t formId, bool isInited)
{
    std::lock_guard<std::mutex> lock(formRecordMutex_);
    auto itFormRecord = formRecords_.find(formId);
    if (itFormRecord == formRecords_.end()) {
        APP_LOGE("%{public}s, form info not find", __func__);
        return;
    }
    itFormRecord->second.isInited = isInited;
    itFormRecord->second.needRefresh = !isInited;
}
/**
 * @brief Set versionUpgrade.
 * @param formId The Id of the form.
 * @param versionUpgrade true or false
 */
void FormDataMgr::SetVersionUpgrade(const int64_t formId, const bool versionUpgrade)
{
    std::lock_guard<std::mutex> lock(formRecordMutex_);
    auto itFormRecord = formRecords_.find(formId);
    if (itFormRecord == formRecords_.end()) {
        APP_LOGE("%{public}s, form info not find", __func__);
        return;
    }
    itFormRecord->second.versionUpgrade = versionUpgrade;
}
/**
 * @brief Update form for host clients.
 * @param formId The Id of the form.
 * @param needRefresh true or false
 */
void FormDataMgr::UpdateHostNeedRefresh(const int64_t formId, const bool needRefresh)
{
    std::lock_guard<std::mutex> lock(formHostRecordMutex_);
    std::vector<FormHostRecord>::iterator itHostRecord;
    for (itHostRecord = clientRecords_.begin(); itHostRecord != clientRecords_.end(); itHostRecord++) {
        if (itHostRecord->Contains(formId)) {
            itHostRecord->SetNeedRefresh(formId, needRefresh);
        }
    }
}

/**
 * @brief Update form for host clients.
 * @param formId The Id of the form.
 * @param formProviderInfo FormProviderInfo object
 */
void FormDataMgr::UpdateFormProviderInfo(const int64_t formId, const FormProviderInfo &formProviderInfo)
{
    std::lock_guard<std::mutex> lock(formRecordMutex_);
    auto itFormRecord = formRecords_.find(formId);
    if (itFormRecord == formRecords_.end()) {
        APP_LOGE("%{public}s, form info not find", __func__);
        return;
    }
    itFormRecord->second.formProviderInfo = formProviderInfo;
}
/**
 * @brief Update form for host clients.
 * @param formId The Id of the form.
 * @param formRecord The form info.
 * @return Returns true if form update, false if other. 
 */
bool FormDataMgr::UpdateHostForm(const int64_t formId, const FormRecord &formRecord)
{
    bool isUpdated = false;
    std::lock_guard<std::mutex> lock(formHostRecordMutex_);
    std::vector<FormHostRecord>::iterator itHostRecord;
    for (itHostRecord = clientRecords_.begin(); itHostRecord != clientRecords_.end(); itHostRecord++) {
        if (itHostRecord->IsEnableRefresh(formId)) {
            // update form
            itHostRecord->OnUpdate(formId, formRecord);
            // set needRefresh
            itHostRecord->SetNeedRefresh(formId, false);
            isUpdated = true;
        }
    }
    return isUpdated;
}
/**
 * @brief handle update form flag.
 * @param formIDs The id of the forms.
 * @param callerToken Caller ability token.
 * @param flag form flag.
 * @param refreshForms Refresh forms
 * @return Returns ERR_OK on success, others on failure.
 */
int32_t FormDataMgr::UpdateHostFormFlag(
    std::vector<int64_t> formIds, 
    const sptr<IRemoteObject> &callerToken, 
    const bool flag, 
    std::vector<int64_t> &refreshForms)
{
    APP_LOGI("%{public}s start, flag: %{public}d", __func__, flag);
    std::lock_guard<std::mutex> lock(formHostRecordMutex_);
    std::vector<FormHostRecord>::iterator itHostRecord;
    for (itHostRecord = clientRecords_.begin(); itHostRecord != clientRecords_.end(); itHostRecord++) {
        if (callerToken == itHostRecord->GetClientStub()) {
            for (const int64_t formId : formIds) {
                if (formId <= 0) {
                    APP_LOGW("%{public}s, formId %{public}" PRId64 " is less than 0", __func__, formId);
                    continue;
                }

                int64_t matchedFormId = FindMatchedFormId(formId);
                if (!itHostRecord->Contains(matchedFormId)) {
                    APP_LOGW("%{public}s, form %{public}" PRId64 " is not owned by this client, don't need to update flag", 
                    __func__, formId);
                    continue;
                }

                itHostRecord->SetEnableRefresh(matchedFormId, flag);
                // set disable
                if (!flag) {
                    APP_LOGI("%{public}s, flag is disable", __func__);
                    continue;
                }
                FormRecord formRecord;
                if (GetFormRecord(matchedFormId, formRecord)) {
                    if (formRecord.needRefresh) {
                        APP_LOGI("%{public}s, formRecord need refresh", __func__);
                        refreshForms.emplace_back(matchedFormId);
                        continue;
                    }
                } else {
                    APP_LOGW("%{public}s, not exist such form:%{public}" PRId64 "", __func__, matchedFormId);
                    continue;
                }

                // if set enable flag, should check whether to refresh form
                if (!itHostRecord->IsNeedRefresh(matchedFormId)) {
                    APP_LOGI("%{public}s, host need not refresh", __func__);
                    continue;
                }

                if (IsFormCached(formRecord)) {
                    APP_LOGI("%{public}s, form cached", __func__);
                    itHostRecord->OnUpdate(matchedFormId, formRecord);
                    itHostRecord->SetNeedRefresh(matchedFormId, false);
                } else {
                    APP_LOGI("%{public}s, form no cache", __func__);
                    refreshForms.emplace_back(matchedFormId);
                    continue;
                }
            }
            APP_LOGI("%{public}s end.", __func__);
            return ERR_OK;
        }
    }
    APP_LOGE("%{public}s, can't find target client", __func__);
    return ERR_APPEXECFWK_FORM_INVALID_PARAM;
}
/**
 * @brief Find matched form id.
 * @param formId The form id.
 * @return Matched form id.
 */
int64_t FormDataMgr::FindMatchedFormId(const int64_t formId)
{
    uint64_t unsignedFormId = static_cast<uint64_t>(formId);
    if ((unsignedFormId & 0xffffffff00000000L) != 0) {
        return formId;
    }
    std::lock_guard<std::mutex> lock(formRecordMutex_);
    std::map<int64_t, FormRecord>::iterator itFormRecord;
    for (itFormRecord = formRecords_.begin(); itFormRecord != formRecords_.end(); itFormRecord++) {
        uint64_t unsignedFormId = static_cast<uint64_t>(formId);
        uint64_t unsignedItFormRecordFirst = static_cast<uint64_t>(itFormRecord->first);
        if ((unsignedItFormRecordFirst & 0x00000000ffffffffL) == (unsignedFormId & 0x00000000ffffffffL)) {
            return itFormRecord->first;
        }
    }
    return formId;
}

/**
 * @brief Clear host data by uId.
 * @param uId The caller uId.
 */
void FormDataMgr::ClearHostDataByUId(const int uId)
{
    std::lock_guard<std::mutex> lock(formHostRecordMutex_);
    std::vector<FormHostRecord>::iterator itHostRecord;
    for (itHostRecord = clientRecords_.begin(); itHostRecord != clientRecords_.end(); ) {
        if (itHostRecord->GetCallerUid() == uId) {
            itHostRecord->CleanResource();
            itHostRecord = clientRecords_.erase(itHostRecord);
        } else {
            itHostRecord++;
        }
    }
}
/**
 * @brief Get no host temp forms.
 * @param uid The caller uid.
 * @param noHostTempFormsMap no host temp forms.
 * @param foundFormsMap Form Id list.
 */
void FormDataMgr::GetNoHostTempForms(
    const int uid, std::map<FormIdKey, 
    std::set<int64_t>> &noHostTempFormsMap, 
    std::map<int64_t, bool> &foundFormsMap)
{
    std::lock_guard<std::mutex> lock(formRecordMutex_);
    std::map<int64_t, FormRecord>::iterator itFormRecord;
    for (itFormRecord = formRecords_.begin(); itFormRecord != formRecords_.end(); itFormRecord++) {
        if (itFormRecord->second.formTempFlg) {
            auto itUid = std::find(itFormRecord->second.formUserUids.begin(), 
                itFormRecord->second.formUserUids.end(), uid);
            if (itUid != itFormRecord->second.formUserUids.end()) {
                itFormRecord->second.formUserUids.erase(itUid);
                if (itFormRecord->second.formUserUids.empty()) {
                    FormIdKey formIdKey;
                    formIdKey.bundleName = itFormRecord->second.bundleName;
                    formIdKey.abilityName = itFormRecord->second.abilityName;
                    formIdKey.moduleName = "";
                    formIdKey.formName = "";
                    formIdKey.specificationId = 0;
                    formIdKey.orientation = 0;
                    auto itIdsSet = noHostTempFormsMap.find(formIdKey);
                    if (itIdsSet == noHostTempFormsMap.end()) {
                        std::set<int64_t> formIdsSet;
                        formIdsSet.emplace(itFormRecord->second.formId);
                        noHostTempFormsMap.emplace(formIdKey, formIdsSet);
                    } else {
                        itIdsSet->second.emplace(itFormRecord->second.formId);
                    }
                }
            } else {
                foundFormsMap.emplace(itFormRecord->second.formId, false);
            }
        }
    }
}
/**
 * @brief Parse update config.
 * @param record The form record.
 * @param info The form item info.
 */
void FormDataMgr::ParseUpdateConfig(FormRecord &record, const FormItemInfo &info) const
{
    int configDuration = info.GetUpdateDuration();
    if (configDuration > 0) {
        ParseIntervalConfig(record, configDuration);
    } else {
        ParseAtTimerConfig(record, info);
    }
}

/**
 * @brief Parse update interval config.
 * @param record The form record.
 * @param configDuration interval duration.
 */
void FormDataMgr::ParseIntervalConfig(FormRecord &record, const int configDuration) const
{
    APP_LOGI("%{public}s, configDuration:%{public}d", __func__, configDuration);
    if (configDuration <= Constants::MIN_CONFIG_DURATION) {
        record.updateDuration = Constants::MIN_PERIOD;
    } else if (configDuration >= Constants::MAX_CONFIG_DURATION) {
        record.updateDuration = Constants::MAX_PERIOD;
    } else {
        record.updateDuration = configDuration * Constants::TIME_CONVERSION;
    }
    APP_LOGI("%{public}s end", __func__);
}

/**
 * @brief Parse at time config.
 * @param record The form record.
 * @param info form item info.
 */
void FormDataMgr::ParseAtTimerConfig(FormRecord &record, const FormItemInfo &info) const
{
    record.isEnableUpdate = false;
    record.updateDuration = 0;
    std::string configAtTime = info.GetScheduledUpdateTime();
    APP_LOGI("%{public}s, parseAsUpdateAt updateAt:%{public}s", __func__, configAtTime.c_str());
    if (configAtTime.empty()) {
        return;
    }

    std::vector<std::string> temp = FormUtil::StringSplit(configAtTime, Constants::TIME_DELIMETER);
    if (temp.size() != Constants::UPDATE_AT_CONFIG_COUNT) {
        APP_LOGE("%{public}s, invalid config", __func__);
        return;
    }
    int hour = -1;
    int min = -1;
    hour = std::stoi(temp[0]);
    min = std::stoi(temp[1]);
    if (hour < Constants::MIN_TIME || hour > Constants::MAX_HOUR || min < Constants::MIN_TIME || min >
        Constants::MAX_MININUTE) {
        APP_LOGE("%{public}s, time is invalid", __func__);
        return;
    }
    record.updateAtHour = hour;
    record.updateAtMin = min;
    record.isEnableUpdate = true;
}
/**
 * @brief handle update form flag.
 * @param formIDs The id of the forms.
 * @param callerToken Caller ability token.
 * @param flag form flag.
 * @return Returns ERR_OK on success, others on failure.
 */
bool FormDataMgr::IsFormCached(const FormRecord record)
{
    if (record.versionUpgrade) {
        return false;
    }
    return FormCacheMgr::GetInstance().IsExist(record.formId);
}

/**
 * @brief Clear form records for st limit value test.
 */
void FormDataMgr::ClearFormRecords()
{
    {
        std::lock_guard<std::mutex> lock(formRecordMutex_);
        formRecords_.clear();
    }
    {
        std::lock_guard<std::mutex> lock(formTempMutex_);
        tempForms_.clear();
    }
}
}  // namespace AppExecFwk
}  // namespace OHOS
