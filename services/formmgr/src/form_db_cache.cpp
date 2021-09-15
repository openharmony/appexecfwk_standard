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

#include "app_log_wrapper.h"
#include "appexecfwk_errors.h"
#include "form_bms_helper.h"
#include "form_db_cache.h"
#include "form_db_info.h"

namespace OHOS {
namespace AppExecFwk {
FormDbCache::FormDbCache()
{
    APP_LOGI("FormDbCache is created");
    dataStorage_ = std::make_unique<FormStorageMgr>();
    formDBInfos_.clear();
}

FormDbCache::~FormDbCache()
{
    APP_LOGI("FormDbCache is destroyed");
}

/**
 * @brief Load form data from DB to DbCache when starting.
 * @return Void.
 */
void FormDbCache::Start()
{
    APP_LOGI("%{public}s called.", __func__);
    std::lock_guard<std::mutex> lock(formDBInfosMutex_);
    std::vector<InnerFormInfo> innerFormInfos;
    innerFormInfos.clear();
    if (dataStorage_->LoadFormData(innerFormInfos) != ERR_OK) {
        APP_LOGE("%{public}s, LoadFormData failed.", __func__);
        return;
    }

    for (unsigned int i = 0; i < innerFormInfos.size(); i++) {
        FormDBInfo formDBInfo = innerFormInfos.at(i).GetFormDBInfo();
        formDBInfos_.emplace_back(formDBInfo);
    }
}

// void FormDbCache::Stop()
// {
//     APP_LOGI("stop");
//     dataStorage_->Stop();
// }

/**
 * @brief Save or update form data to DbCache and DB.
 * @param formDBInfo Form data.
 * @return Returns ERR_OK on success, others on failure.
 */
ErrCode FormDbCache::SaveFormInfo(const FormDBInfo &formDBInfo)
{
    APP_LOGI("%{public}s called, formId:%{public}" PRId64 "", __func__, formDBInfo.formId);
    std::lock_guard<std::mutex> lock(formDBInfosMutex_);
    auto iter = find(formDBInfos_.begin(), formDBInfos_.end(), formDBInfo);
    if (iter != formDBInfos_.end()) {
        if (iter->Compare(formDBInfo) == false) {
            APP_LOGW("%{public}s, need update, formId[%{public}" PRId64 "].", __func__, formDBInfo.formId);
            *iter = formDBInfo;
            InnerFormInfo innerFormInfo(formDBInfo);
            return dataStorage_->ModifyStorageFormInfo(innerFormInfo);
        } else {
            APP_LOGW("%{public}s, already exist, formId[%{public}" PRId64 "].", __func__, formDBInfo.formId);
            return ERR_OK;
        }
    } else {
        formDBInfos_.emplace_back(formDBInfo);
        InnerFormInfo innerFormInfo(formDBInfo);
        return dataStorage_->SaveStorageFormInfo(innerFormInfo);         
    }
}

/**
 * @brief Save or update form data to DbCache and DB.
 * @param formDBInfo Form data.
 * @return Returns ERR_OK on success, others on failure.(NoLock)
 */
ErrCode FormDbCache::SaveFormInfoNolock(const FormDBInfo &formDBInfo)
{
    APP_LOGI("%{public}s called, formId:%{public}" PRId64 "", __func__, formDBInfo.formId);
    auto iter = find(formDBInfos_.begin(), formDBInfos_.end(), formDBInfo);
    if (iter != formDBInfos_.end()) {
        if (iter->Compare(formDBInfo) == false) {
            APP_LOGW("%{public}s, need update, formId[%{public}" PRId64 "].", __func__, formDBInfo.formId);
            *iter = formDBInfo;
            InnerFormInfo innerFormInfo(formDBInfo);
            return dataStorage_->ModifyStorageFormInfo(innerFormInfo);
        } else {
            APP_LOGW("%{public}s, already exist, formId[%{public}" PRId64 "].", __func__, formDBInfo.formId);
            return ERR_OK;
        }
    } else {
        formDBInfos_.emplace_back(formDBInfo);
        InnerFormInfo innerFormInfo(formDBInfo);
        return dataStorage_->SaveStorageFormInfo(innerFormInfo);         
    }
}

/**
 * @brief Delete form data in DbCache and DB with formId.
 * @param formId form data Id.
 * @return Returns ERR_OK on success, others on failure.
 */   
ErrCode FormDbCache::DeleteFormInfo(int64_t formId)
{
    std::lock_guard<std::mutex> lock(formDBInfosMutex_);
    FormDBInfo tmpForm;
    tmpForm.formId = formId;
    auto iter = find(formDBInfos_.begin(), formDBInfos_.end(), tmpForm);
    if (iter == formDBInfos_.end()) {
        APP_LOGW("%{public}s, not find formId[%{public}" PRId64 "]", __func__, formId);
    } else {
        formDBInfos_.erase(iter);
    }
    if (dataStorage_->DeleteStorageFormInfo(std::to_string(formId)) == ERR_OK) {
        return ERR_OK;
    } else {
        return ERR_APPEXECFWK_FORM_JSON_DELETE_FAIL;
    }
}
/**
 * @brief Delete form data in DbCache and DB with formId.
 * @param formId form data Id.
 * @param removedDBForms Removed db form infos
 * @return Returns ERR_OK on success, others on failure.
 */
ErrCode FormDbCache::DeleteFormInfoByBundleName(const std::string &bundleName, std::vector<FormDBInfo> &removedDBForms)
{
    std::lock_guard<std::mutex> lock(formDBInfosMutex_);
    std::vector<FormDBInfo>::iterator itRecord;
    for (itRecord = formDBInfos_.begin(); itRecord != formDBInfos_.end();) {
        if (bundleName == itRecord->bundleName) {
            int64_t formId = itRecord->formId;            
            if (dataStorage_->DeleteStorageFormInfo(std::to_string(formId)) == ERR_OK) {
                removedDBForms.emplace_back(*itRecord);
                itRecord = formDBInfos_.erase(itRecord);
            } else {
                itRecord++;
            }
        } else {
            itRecord++;
        }
    }
    return ERR_OK;
}
// bool FormDbCache::BatchDeleteForms(std::vector<int64_t> formIds)
// {
//     std::lock_guard<std::mutex> lock(formDBInfosMutex_);
//     for (const auto& formId : formIds) {
//         FormDBInfo tmpForm;
//         tmpForm.formId_ = formId;
//         auto iter = find(formDBInfos_.begin(), formDBInfos_.end(), tmpForm);
//         if (iter != formDBInfos_.end()) {
//             formDBInfos_.erase(iter);
//         }
//     }
//     return dataStorage_->BatchDeleteForms(formIds);
// }

/**
 * @brief Get all form data from DbCache.
 * @param formDBInfos Storage all DbCache.
 * @return Void.
 */
void FormDbCache::GetAllFormInfo(std::vector<FormDBInfo> &formDBInfos)
{
    APP_LOGI("%{public}s called.", __func__);
    std::lock_guard<std::mutex> lock(formDBInfosMutex_);
    formDBInfos = formDBInfos_;
}

/**
 * @brief Get record from DB cache with formId
 * @param formId Form data Id
 * @param record Form data
 * @return Returns ERR_OK on success, others on failure.
 */
ErrCode FormDbCache::GetDBRecord(const int64_t formId, FormRecord &record) const
{
    std::lock_guard<std::mutex> lock(formDBInfosMutex_);
    for (const FormDBInfo &dbInfo : formDBInfos_) {
        if (dbInfo.formId == formId) {
            record.formName = dbInfo.formName;
            record.bundleName = dbInfo.bundleName;
            record.moduleName = dbInfo.moduleName;
            record.abilityName = dbInfo.abilityName;
            record.formUserUids = dbInfo.formUserUids;
            return ERR_OK;
        }    
    }
    APP_LOGE("%{public}s, not find formId[%{public}" PRId64 "]", __func__, formId);
    return ERR_APPEXECFWK_FORM_DBCACHE_FIND_FAIL;
}
/**
 * @brief Get record from DB cache with formId
 * @param formId Form data Id
 * @param record Form db data
 * @return Returns ERR_OK on success, others on failure.
 */
ErrCode FormDbCache::GetDBRecord(const int64_t formId, FormDBInfo &record) const
{
    std::lock_guard<std::mutex> lock(formDBInfosMutex_);
    for (const FormDBInfo &dbInfo : formDBInfos_) {
        if (dbInfo.formId == formId) {
            record = dbInfo;
            return ERR_OK;
        }    
    }
    APP_LOGE("%{public}s, not find formId[%{public}" PRId64 "]", __func__, formId);
    return ERR_APPEXECFWK_FORM_DBCACHE_FIND_FAIL;
}
/**
 * @brief Use record save or update DB data and DB cache with formId
 * @param formId Form data Id
 * @param record Form data
 * @return Returns ERR_OK on success, others on failure.
 */
ErrCode FormDbCache::UpdateDBRecord(const int64_t formId, const FormRecord &record) const
{
    FormDBInfo formDBInfo(formId, record);
    return FormDbCache::GetInstance().SaveFormInfo(formDBInfo);
    // FormBmsHelper::GetInstance().NotifyModuleNotRemovable(formDBInfo.bundleName, formDBInfo.moduleName);
}
/**
 * @brief Get no host db record.
 * @param uid The caller uid.
 * @param noHostFormDBList no host db record list.
 * @param foundFormsMap Form Id list.
 * @return Returns ERR_OK on success, others on failure.
 */
ErrCode FormDbCache::GetNoHostDBForms(const int uid, std::map<FormIdKey, std::set<int64_t>> &noHostFormDBList, 
std::map<int64_t, bool> &foundFormsMap)
{
    std::lock_guard<std::mutex> lock(formDBInfosMutex_);
    for (FormDBInfo& dbInfo : formDBInfos_) {
        if (dbInfo.Contains(uid)) {
            dbInfo.Remove(uid);
            if (dbInfo.formUserUids.empty()) {
                FormIdKey formIdKey;
                formIdKey.bundleName = dbInfo.bundleName;
                formIdKey.abilityName = dbInfo.abilityName;
                auto itIdsSet = noHostFormDBList.find(formIdKey);
                if (itIdsSet == noHostFormDBList.end()) {
                    std::set<int64_t> formIdsSet;
                    formIdsSet.emplace(dbInfo.formId);
                    noHostFormDBList.emplace(formIdKey, formIdsSet);
                } else {
                    itIdsSet->second.emplace(dbInfo.formId);
                }
            } else {
                foundFormsMap.emplace(dbInfo.formId, false);
                SaveFormInfoNolock(dbInfo);
                FormBmsHelper::GetInstance().NotifyModuleNotRemovable(dbInfo.bundleName, dbInfo.moduleName);
            }
        }
    }
    return ERR_OK;
}
/**
 * @brief Get match count by bundleName and moduleName.
 * @param bundleName BundleName.
 * @param moduleName ModuleName.
 * @return Returns match count.
 */
int FormDbCache::GetMatchCount(const std::string &bundleName, const std::string &moduleName)
{
    int32_t matchCount {0};
    std::vector<FormDBInfo> formDBInfos;
    std::lock_guard<std::mutex> lock(formDBInfosMutex_);
    for (FormDBInfo &dbInfo : formDBInfos_) {
        if (dbInfo.bundleName == bundleName && dbInfo.moduleName == moduleName) {
            ++matchCount;
        }
    }
    return matchCount;
}
} // namespace AppExecFwk
} // namespace OHOS
