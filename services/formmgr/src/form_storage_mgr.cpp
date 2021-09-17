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

#include <cinttypes>
#include <dirent.h>
#include <fstream>
#include <iomanip> 
#include <sys/stat.h>
#include <sys/types.h>
#include <stdio.h>

#include "securec.h"
#include "app_log_wrapper.h"
#include "form_storage_mgr.h"
#include "string_ex.h"

namespace OHOS {
namespace AppExecFwk {
namespace {
const char* FORM_DB_DATA_BASE_FILE_DIR = "/data/formmgr";
const int32_t FORM_DB_DATA_BASE_FILE_PATH_LEN = 255;
}

/**
 * @brief Load form data from fileNamePath to innerFormInfos.
 * @param fileNamePath load file path.
 * @param innerFormInfos Save form data.
 * @return Returns true if the data is successfully loaded; returns false otherwise.
 */
static bool LoadFormDataFile(const char* fileNamePath, std::vector<InnerFormInfo> &innerFormInfos)
{
    bool ret = false;
    std::ifstream i(fileNamePath);
    if (!i.is_open()) {
        APP_LOGE("%{public}s, failed to open file[%{public}s]", __func__, fileNamePath);
        return false;
    }

    nlohmann::json jParse;
    i.seekg(0, std::ios::end);
    int len = static_cast<int>(i.tellg());
    if (len != 0) {
        i.seekg(0, std::ios::beg);
        i >> jParse;
        for (auto &it : jParse.items()) {
            InnerFormInfo innerFormInfo;
            if (innerFormInfo.FromJson(it.value())) {
                innerFormInfos.emplace_back(innerFormInfo);
            } else {
                APP_LOGE("%{public}s, failed to parse json, formId[%{public}s]", __func__, it.key().c_str());
            }
        }
        ret = true;
    } else {
        APP_LOGE("%{public}s, file[%{public}s] is empty", __func__, fileNamePath);
        ret = false;
    }
    i.close();
    return ret;
}

/**
 * @brief Load all form data from DB to innerFormInfos.
 * @param innerFormInfos Storage all form data.
 * @return Returns ERR_OK on success, others on failure.
 */
ErrCode FormStorageMgr::LoadFormData(std::vector<InnerFormInfo> &innerFormInfos) const
{
    APP_LOGI("%{public}s called.", __func__);
    DIR *dirptr = opendir(FORM_DB_DATA_BASE_FILE_DIR);
    if (dirptr == NULL) {
        APP_LOGE("%{public}s, opendir failed, should no formmgr dir", __func__);
        return ERR_APPEXECFWK_FORM_COMMON_CODE;
    }

    struct dirent *ptr;
    while ((ptr = readdir(dirptr)) != NULL) {
        APP_LOGI("%{public}s, readdir fileName[%{public}s]", __func__, ptr->d_name);
        if ((strcmp(ptr->d_name, ".") == 0) || (strcmp(ptr->d_name, "..") == 0)) {
            continue;
        }
        char fileNamePath[FORM_DB_DATA_BASE_FILE_PATH_LEN] = {0};
        sprintf_s(fileNamePath, FORM_DB_DATA_BASE_FILE_PATH_LEN, "%s/%s", FORM_DB_DATA_BASE_FILE_DIR, ptr->d_name);
        if (!LoadFormDataFile(fileNamePath, innerFormInfos)) {
            APP_LOGE("%{public}s, LoadFormDataFile failed, file[%{public}s]", __func__, ptr->d_name);
        }
    }
    APP_LOGI("%{public}s, readdir over", __func__);
    closedir(dirptr);
    return ERR_OK;
}

/**
 * @brief Get form data from DB to innerFormInfo with formId.
 * @param innerFormInfo Storage form data.
 * @return Returns ERR_OK on success, others on failure.
 */
ErrCode FormStorageMgr::GetStorageFormInfoById(const std::string &formId, InnerFormInfo &innerFormInfo) const
{
    ErrCode ret = ERR_OK;
    APP_LOGD("%{public}s called, formId[%{public}s]", __func__, formId.c_str());
    char fileNamePath[FORM_DB_DATA_BASE_FILE_PATH_LEN] = {0};
    sprintf_s(fileNamePath, FORM_DB_DATA_BASE_FILE_PATH_LEN, "%s/%s.json", FORM_DB_DATA_BASE_FILE_DIR, formId.c_str());
    std::ifstream i(fileNamePath);
    nlohmann::json jParse;
    if (!i.is_open()) {
        APP_LOGE("%{public}s, open failed, should no this file[%{public}s.json]", __func__, formId.c_str());
        return ERR_APPEXECFWK_FORM_COMMON_CODE;
    }
    APP_LOGD("%{public}s, open success file[%{public}s.json]", __func__, formId.c_str());
    i.seekg(0, std::ios::end);
    int len = static_cast<int>(i.tellg());
    if (len != 0) {
        i.seekg(0, std::ios::beg);
        i >> jParse;
        auto it = jParse.find(formId);
        if (it != jParse.end()) {
            if (innerFormInfo.FromJson(it.value()) == false) {
                APP_LOGE("%{public}s, fromJson parse failed formId[%{public}s]", __func__, it.key().c_str());
                ret = ERR_APPEXECFWK_FORM_COMMON_CODE;
            } else {
                ret = ERR_OK;
            }
        } else {
            APP_LOGE("%{public}s, not find formId[%{public}s]", __func__, formId.c_str());
            ret = ERR_APPEXECFWK_FORM_COMMON_CODE;
        }
    } else {
        APP_LOGE("%{public}s, file is empty formId[%{public}s]", __func__, formId.c_str());
        ret = ERR_APPEXECFWK_FORM_COMMON_CODE;
    }
    i.close();

    return ret;
}

/**
 * @brief Save or update the form data in DB.
 * @param innerFormInfo Indicates the InnerFormInfo object to be save.
 * @return Returns ERR_OK on success, others on failure.
 */
ErrCode FormStorageMgr::SaveStorageFormInfo(const InnerFormInfo &innerFormInfo) const
{
    APP_LOGI("%{public}s called, formId[%{public}" PRId64 "]", __func__, innerFormInfo.GetFormId());
    ErrCode ret = ERR_OK;
    std::string formId = std::to_string(innerFormInfo.GetFormId());

    DIR *dirptr = opendir(FORM_DB_DATA_BASE_FILE_DIR);
    if (dirptr == NULL) {
        APP_LOGW("%{public}s, failed to open dir", __func__);
        if (-1 == mkdir(FORM_DB_DATA_BASE_FILE_DIR, S_IRWXU)) {
            APP_LOGE("%{public}s, failed to create dir", __func__);
            return ERR_APPEXECFWK_FORM_COMMON_CODE;
        }
    } else {
        closedir(dirptr);
    }
    char tmpFilePath[FORM_DB_DATA_BASE_FILE_PATH_LEN] = {0};
    sprintf_s(tmpFilePath, FORM_DB_DATA_BASE_FILE_PATH_LEN, "%s/%s.json", FORM_DB_DATA_BASE_FILE_DIR, formId.c_str());
    std::fstream f(tmpFilePath);
    nlohmann::json jParse;
    if (!f.is_open()) {
        std::ofstream o(tmpFilePath); // if file not exist, should create file here
        if (!o.is_open()) {
            APP_LOGE("%{public}s, touch new file[%{public}s] failed", __func__, tmpFilePath);
            return ERR_APPEXECFWK_FORM_COMMON_CODE;
        }
        o.close();
        APP_LOGI("%{public}s, touch new file[%{public}s.json]", __func__, formId.c_str());
        f.open(tmpFilePath);
    }
    bool isExist = f.good();
    if (isExist) {
        nlohmann::json innerInfo;
        innerFormInfo.ToJson(innerInfo);
        f.seekg(0, std::ios::end);
        int len = static_cast<int>(f.tellg());
        if (len == 0) {
            nlohmann::json formRoot;
            formRoot[formId] = innerInfo;
            f << formRoot << std::endl;
        } else {
            APP_LOGE("%{public}s, file[%{public}s.json] is not empty", __func__, formId.c_str());
        }
    } else {
        APP_LOGE("%{public}s, touch new file[%{public}s] failed", __func__, formId.c_str());
        ret = ERR_APPEXECFWK_FORM_COMMON_CODE;
    }
    f.close();
    return ret;
}

/**
 * @brief Modify the form data in DB.
 * @param innerFormInfo Indicates the InnerFormInfo object to be Modify.
 * @return Returns ERR_OK on success, others on failure.
 */
ErrCode FormStorageMgr::ModifyStorageFormInfo(const InnerFormInfo &innerFormInfo) const
{
    APP_LOGI("%{public}s called, formId[%{public}" PRId64 "]", __func__, innerFormInfo.GetFormId());
    char fileNamePath[FORM_DB_DATA_BASE_FILE_PATH_LEN] = {0};
    sprintf_s(fileNamePath, FORM_DB_DATA_BASE_FILE_PATH_LEN, "%s/%" PRId64 ".json", FORM_DB_DATA_BASE_FILE_DIR, innerFormInfo.GetFormId());

    std::ofstream o(fileNamePath, std::ios_base::trunc | std::ios_base::out);
    if (!o.is_open()) {
        APP_LOGE("%{public}s, open failed file[%{public}s]", __func__, fileNamePath);
        return ERR_APPEXECFWK_FORM_COMMON_CODE;
    }

    nlohmann::json innerInfo;
    innerFormInfo.ToJson(innerInfo);
    nlohmann::json formRoot;
    std::string formId = std::to_string(innerFormInfo.GetFormId());

    formRoot[formId] = innerInfo;
    o << formRoot << std::endl;

    o.close();
    return ERR_OK;
}

/**
 * @brief Delete the form data in DB.
 * @param formId The form data Id.
 * @return Returns ERR_OK on success, others on failure.
 */
ErrCode FormStorageMgr::DeleteStorageFormInfo(const std::string &formId) const
{
    APP_LOGI("%{public}s called, formId[%{public}s]", __func__, formId.c_str());
    char fileNamePath[FORM_DB_DATA_BASE_FILE_PATH_LEN] = {0};
    sprintf_s(fileNamePath, FORM_DB_DATA_BASE_FILE_PATH_LEN, "%s/%s.json", FORM_DB_DATA_BASE_FILE_DIR, formId.c_str());

    if (std::remove(fileNamePath) != 0) {
        APP_LOGE("%{public}s, delete failed file[%{public}s]", __func__, fileNamePath);
        return ERR_APPEXECFWK_FORM_COMMON_CODE;
    }

    return ERR_OK;
}
}  // namespace AppExecFwk
}  // namespace OHOS
