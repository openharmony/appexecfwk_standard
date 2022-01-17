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

#ifndef FOUNDATION_APPEXECFWK_SERVICES_FORMMGR_INCLUDE_FORM_DATA_MGR_H
#define FOUNDATION_APPEXECFWK_SERVICES_FORMMGR_INCLUDE_FORM_DATA_MGR_H

#include <map>
#include <memory>
#include <mutex>
#include <set>
#include <singleton.h>
#include <string>

#include "form_host_record.h"
#include "form_id_key.h"
#include "form_info.h"
#include "form_item_info.h"
#include "form_js_info.h"
#include "form_record.h"
#include "iremote_object.h"

namespace OHOS {
namespace AppExecFwk {
/**
 * @class FormDataMgr
 * form data manager.
 */
class FormDataMgr final : public DelayedRefSingleton<FormDataMgr> {
DECLARE_DELAYED_REF_SINGLETON(FormDataMgr)
public:
    DISALLOW_COPY_AND_MOVE(FormDataMgr);
    /**
     * @brief Allot form info by item info.
     * @param formId The Id of the form.
     * @param formInfo Form item info.
     * @param callingUid The UID of the proxy.
     * @return Returns form record.
     */
    FormRecord AllotFormRecord(const FormItemInfo &formInfo, const int callingUid);
    /**
     * @brief Create form js info by form record.
     * @param formId The Id of the form.
     * @param record Form record.
     * @param formInfo Js info.
     * @return None.
     */
    void CreateFormInfo(const int64_t formId, const FormRecord &record, FormJsInfo &formInfo);
    /**
     * @brief Delete form js info by form record.
     * @param formId The Id of the form.
     * @return Returns true if this function is successfully called; returns false otherwise.
     */
    bool DeleteFormRecord(const int64_t formId);
    /**
     * @brief Clean removed forms for host.
     * @param removedFormIds The id list of the forms.
     */
    void CleanHostRemovedForms(const std::vector<int64_t> &removedFormIds);
    /**
     * @brief Allot form host record by caller token.
     * @param info The form item info.
     * @param callerToken callerToken
     * @param formId The Id of the form.
     * @param callingUid The UID of the proxy.
     * @return Returns true if this function is successfully called; returns false otherwise.
     */
    bool AllotFormHostRecord(const FormItemInfo &info, const sptr<IRemoteObject> &callerToken,
        const int64_t formId, const int callingUid);
    /**
     * @brief Check temp form count is max.
     * @return Returns ERR_OK if the temp form not reached; returns ERR_MAX_SYSTEM_TEMP_FORMS is reached.
     */
    int CheckTempEnoughForm() const;
    /**
     * @brief Check form count is max.
     * @param callingUid The UID of the proxy.
     * @return Returns true if this function is successfully called; returns false otherwise.
     */
    int CheckEnoughForm(const int callingUid) const;
    /**
     * @brief Delete temp form.
     * @param formId The Id of the form.
     * @return Returns true if this function is successfully called; returns false otherwise.
     */
    bool DeleteTempForm(const int64_t formId);
    /**
     * @brief Check temp form is exist.
     * @param formId The Id of the form.
     * @return Returns true if the temp form is exist; returns false is not exist.
     */
    bool ExistTempForm(const int64_t formId) const;
    /**
     * @brief Modify form temp flag by formId.
     * @param formId The Id of the form.
     * @param formTempFlg The form temp flag.
     * @return Returns true if this function is successfully called; returns false otherwise.
     */
    bool ModifyFormTempFlg(const int64_t formId, const bool formTempFlg);
    /**
     * @brief Add form user uid from form record.
     * @param formId The Id of the form.
     * @param formUserUid The form user uid.
     * @return Returns true if this function is successfully called; returns false otherwise.
     */
    bool AddFormUserUid(const int64_t formId, const int32_t formUserUid);
    /**
     * @brief Delete form user uid from form record.
     * @param formId The Id of the form.
     * @param uid calling user id.
     * @return Returns true if this function is successfully called; returns false otherwise.
     */
    bool DeleteFormUserUid(const int64_t formId, const int32_t uid);
    /**
     * @brief Update form record.
     * @param formId The Id of the form.
     * @param formRecord The form record.
     * @return Returns true if this function is successfully called; returns false otherwise.
     */
    bool UpdateFormRecord(const int64_t formId, const FormRecord &formRecord);
    /**
     * @brief Get form record.
     * @param formId The Id of the form.
     * @param formRecord The form record.
     * @return Returns true if this function is successfully called; returns false otherwise.
     */
    bool GetFormRecord(const int64_t formId, FormRecord &formRecord) const;
    /**
     * @brief Get form record.
     * @param bundleName Bundle name.
     * @param formInfos The form record list.
     * @return Returns true if this function is successfully called; returns false otherwise.
     */
    bool GetFormRecord(const std::string &bundleName, std::vector<FormRecord> &formInfos);
    /**
     * @brief Check form record is exist.
     * @param formId The Id of the form.
     * @return Returns true if the form record is exist; returns false is not exist.
     */
    bool ExistFormRecord(const int64_t formId) const;
    /**
     * @brief Has form user uids in form record.
     * @param formId The Id of the form.
     * @return Returns true if this form has form user uids; returns false is not has.
     */
    bool HasFormUserUids(const int64_t formId) const;
    /**
     * @brief Get form host record.
     * @param formId The id of the form.
     * @param formHostRecord The form host record.
     * @return Returns true if this function is successfully called; returns false otherwise.
     */
    bool GetFormHostRecord(const int64_t formId, FormHostRecord &formHostRecord) const;
    /**
     * @brief Delete form host record.
     * @param callerToken The client stub of the form host record.
     * @param formId The id of the form.
     * @return Returns true if this function is successfully called; returns false otherwise.
     */
    bool DeleteHostRecord(const sptr<IRemoteObject> &callerToken, const int64_t formId);
    /**
     * @brief Handle form host died.
     * @param remoteHost Form host proxy object.
     */
    void HandleHostDied(const sptr<IRemoteObject> &remoteHost);
    /**
     * @brief Refresh enable or not.
     * @param formId The Id of the form.
     * @return true on enbale, false on disable.
     */
    bool IsEnableRefresh(int64_t formId);
    /**
     * @brief Check calling uid is valid.
     * @param formUserUids The form user uids.
     * @return Returns true if this user uid is valid; returns false otherwise.
     */
    bool IsCallingUidValid(const std::vector<int32_t> &formUserUids) const;
    /**
     * @brief Generate udid.
     * @return Returns true if this function is successfully called; returns false otherwise.
     */
    bool GenerateUdidHash();
    /**
     * @brief Generate form id.
     * @return form id.
     */
    int64_t GenerateFormId();
    /**
     * @brief Get udid.
     * @return udid.
     */
    int64_t GetUdidHash() const;
    /**
     * @brief Set udid.
     * @param udidHash udid.
     */
    void SetUdidHash(const int64_t udidHash);

    /**
     * @brief Get the matched form host record by client stub.
     *
     * @param callerToken The client stub of the form host record.
     * @param formHostRecord The form host record.
     * @return Returns true if this function is successfully called, returns false otherwise.
     */
    bool GetMatchedHostClient(const sptr<IRemoteObject> &callerToken, FormHostRecord &formHostRecord) const;

    /**
     * @brief Set needRefresh for FormRecord.
     * @param formId The Id of the form.
     * @param needRefresh true or false.
     */
    void SetNeedRefresh(const int64_t formId, const bool needRefresh);
    /**
     * @brief Set isCountTimerRefresh for FormRecord.
     * @param formId The Id of the form.
     * @param countTimerRefresh true or false.
     */
    void SetCountTimerRefresh(const int64_t formId, const bool countTimerRefresh);
    /**
     * @brief Get updated form info.
     * @param record FormRecord.
     * @param targetForms Target forms.
     * @param updatedForm Updated form info.
     * @return Returns true on success, false on failure.
     */
    bool GetUpdatedForm(const FormRecord &record, const std::vector<FormInfo> &targetForms, FormInfo &updatedForm);
    /**
     * @brief Set isEnableUpdate for FormRecord.
     * @param formId The Id of the form.
     * @param enableUpdate true or false.
     */
    void SetEnableUpdate(const int64_t formId, const bool enableUpdate);
    /**
     * @brief Set update info for FormRecord.
     * @param formId The Id of the form.
     * @param enableUpdate true or false.
     * @param updateDuration Update duration.
     * @param updateAtHour Update at hour.
     * @param updateAtMin Update at minute.
     */
    void SetUpdateInfo(const int64_t formId, const bool enableUpdate, const long updateDuration,
    const int updateAtHour, const int updateAtMin);
    /**
     * @brief Clean removed form records.
     * @param bundleName BundleName.
     * @param removedForms The id list of the forms.
     */
    void CleanRemovedFormRecords(const std::string &bundleName, std::set<int64_t> &removedForms);
    /**
     * @brief Clean removed temp form records.
     * @param  bundleName BundleName.
     * @param removedForms The id list of the forms.
     */
    void CleanRemovedTempFormRecords(const std::string &bundleName, std::set<int64_t> &removedForms);
    /**
     * @brief Get recreate form records.
     * @param reCreateForms The id list of the forms.
     */
    void GetReCreateFormRecordsByBundleName(const std::string &bundleName, std::set<int64_t> &reCreateForms);
    /**
     * @brief Set form isInited = true.
     * @param formId The Id of the form.
     * @param isInited isInited property
     */
    void SetFormCacheInited(const int64_t formId, const bool isInited);
    /**
     * @brief Set versionUpgrade.
     * @param formId The Id of the form.
     * @param versionUpgrade true or false
     */
    void SetVersionUpgrade(const int64_t formId, const bool versionUpgrade);
    /**
     * @brief Update form for host clients.
     * @param formId The Id of the form.
     * @param needRefresh true or false
     */
    void UpdateHostNeedRefresh(const int64_t formId, const bool needRefresh);
    /**
     * @brief Update form for host clients.
     * @param formId The Id of the form.
     * @param formRecord The form info.
     * @return Returns true if form update, false if other.
     */
    bool UpdateHostForm(const int64_t formId, const FormRecord &formRecord);
    /**
     * @brief handle update form flag.
     * @param formIDs The id of the forms.
     * @param callerToken Caller ability token.
     * @param flag form flag.
     * @param refreshForms Refresh forms
     * @return Returns ERR_OK on success, others on failure.
     */
    int32_t UpdateHostFormFlag(std::vector<int64_t> formIds, const sptr<IRemoteObject> &callerToken,
    const bool flag, std::vector<int64_t> &refreshForms);
    /**
     * @brief Find matched form id.
     * @param formId The form id.
     * @return Matched form id.
     */
    int64_t FindMatchedFormId(const int64_t formId);
    /**
     * @brief Clear host data by uId.
     * @param uId The caller uId.
     */
    void ClearHostDataByUId(const int uId);
    /**
     * @brief Get no host temp forms.
     * @param uid The caller uid.
     * @param noHostTempFormsMap no host temp forms.
     * @param foundFormsMap Form Id list.
     */
    void GetNoHostTempForms(const int uid, std::map<FormIdKey, std::set<int64_t>> &noHostTempFormsMap,
        std::map<int64_t, bool> &foundFormsMap);

    /**
     * @brief Update form for host clients.
     * @param formId The Id of the form.
     * @param formProviderInfo FormProviderInfo object
     */
    void UpdateFormProviderInfo(const int64_t formId, const FormProviderInfo &formProviderInfo);

    /**
    * @brief Clear form records for st limit value test.
    */
    void ClearFormRecords();
private:
    /**
     * @brief Create form record.
     * @param formInfo The form item info.
     * @param callingUid The UID of the proxy.
     * @return Form record.
     */
    FormRecord CreateFormRecord(const FormItemInfo &formInfo, const int callingUid) const;
    /**
     * @brief Create host record.
     * @param info The form item info.
     * @param callerToken The UID of the proxy.
     * @param callingUid The UID of the proxy.
     * @param record The form host record.
     * @return Returns true if this function is successfully called; returns false otherwise.
     */
    bool CreateHostRecord(const FormItemInfo &info, const sptr<IRemoteObject> &callerToken,
    const int callingUid, FormHostRecord &record);
    /**
     * @brief Parse update config.
     * @param record The form record.
     * @param info The form item info.
     * @return None.
     */
    void ParseUpdateConfig(FormRecord &record, const FormItemInfo &info) const;
    /**
     * @brief Parse update interval config.
     * @param record The form record.
     * @param configDuration interval duration.
     */
    void ParseIntervalConfig(FormRecord &record, const int configDuration) const;
    /**
     * @brief Parse at time config.
     * @param record The form record.
     * @param info form item info.
     */
    void ParseAtTimerConfig(FormRecord &record, const FormItemInfo &info) const;
    /**
     * @brief Get the temp forms from host and delete temp form in cache.
     * @param record The form record.
     * @param recordTempForms Getted the temp forms.
     */
    void HandleHostDiedForTempForms(const FormHostRecord &record, std::vector<int64_t> &recordTempForms);
    /**
     * @brief Check if two forms is same or not.
     * @param record FormRecord.
     * @param formInfo FormInfo.
     * @return Returns true on success, false on failure.
     */
    bool IsSameForm(const FormRecord &record, const FormInfo &formInfo);
    /**
     * @brief handle update form flag.
     * @param formIDs The id of the forms.
     * @param callerToken Caller ability token.
     * @param flag form flag.
     * @return Returns ERR_OK on success, others on failure.
     */
    bool IsFormCached(const FormRecord record);
private:
    mutable std::mutex formRecordMutex_;
    mutable std::mutex formHostRecordMutex_;
    mutable std::mutex formTempMutex_;
    std::map<int64_t, FormRecord> formRecords_;
    std::vector<FormHostRecord> clientRecords_;
    std::vector<int64_t> tempForms_;
    int64_t udidHash_;
};
}  // namespace AppExecFwk
}  // namespace OHOS
#endif // FOUNDATION_APPEXECFWK_SERVICES_FORMMGR_INCLUDE_FORM_HOST_DATA_MGR_H
