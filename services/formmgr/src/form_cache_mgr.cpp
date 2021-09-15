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

#include "app_log_wrapper.h"
#include "form_cache_mgr.h"

namespace OHOS {
namespace AppExecFwk {
FormCacheMgr::FormCacheMgr()
{
    APP_LOGI("create form cache manager instance");
}
FormCacheMgr::~FormCacheMgr()
{
    APP_LOGI("destroy form cache manager instance");
}

/**
 * @brief Get form data.
 * @param formId, Form id.
 * @param data, Cache data.
 * @return Returns true if this function is successfully called; returns false otherwise.
 */
bool FormCacheMgr::GetData(const int64_t formId, std::string &data) const
{
    APP_LOGI("get cache data");
    std::lock_guard<std::mutex> lock(cacheMutex_);
    if (cacheData_.empty()) {
        APP_LOGE("form cache is empty");
        return false;
    }
    auto formData = cacheData_.find(formId);
    if (formData == cacheData_.end()) {
        APP_LOGE("cache data not find");
        return false;
    }

    data = formData->second;

    return true;
}

/**
 * @brief Add form data.
 * @param formId, Form id.
 * @param data, Cache data.
 * @return Returns true if this function is successfully called; returns false otherwise.
 */
bool FormCacheMgr::AddData(const int64_t formId, const std::string &data)
{
    APP_LOGI("add new cache data");
    std::lock_guard<std::mutex> lock(cacheMutex_);
    std::pair<std::map<int64_t, std::string>::iterator, bool> retVal 
        = cacheData_.emplace(formId, data);

    return retVal.second;
}

/**
 * @brief Delete form data.
 * @param formId, Form id.
 * @return Returns true if this function is successfully called; returns false otherwise.
 */
bool FormCacheMgr::DeleteData(const int64_t formId)
{
    APP_LOGI("delete cache data");
    std::lock_guard<std::mutex> lock(cacheMutex_);
    auto formData = cacheData_.find(formId);
    if (formData == cacheData_.end()) {
        APP_LOGW("cache data is not exist");
        return true;
    }

    return cacheData_.erase(formId);
}

/**
 * @brief Update form data.
 * @param formId, Form id.
 * @param data, Cache data.
 * @return Returns true if this function is successfully called; returns false otherwise.
 */
bool FormCacheMgr::UpdateData(const int64_t formId, const std::string &data)
{
    APP_LOGI("update cache data");
    std::lock_guard<std::mutex> lock(cacheMutex_);
    auto formData = cacheData_.find(formId);
    if (formData == cacheData_.end()) {
        APP_LOGE("cache data is not exist");
        return false;
    }

    formData->second = data;
    return true;
}
/**
 * @brief Check if form data is exist or not.
 * @param formId, Form id.
 * @return Returns true if this function is successfully called; returns false otherwise.
 */
bool FormCacheMgr::IsExist(const int64_t formId) const
{
    APP_LOGI("get cache data");
    std::lock_guard<std::mutex> lock(cacheMutex_);
    if (cacheData_.empty()) {
        APP_LOGE("form cache is empty");
        return false;
    }
    auto formData = cacheData_.find(formId);
    if (formData == cacheData_.end()) {
        APP_LOGE("cache data not find");
        return false;
    }

    return true;
}
}  // namespace AppExecFwk
}  // namespace OHOS
