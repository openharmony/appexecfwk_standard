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
#include "form_host_client.h"

namespace OHOS {
namespace AppExecFwk {
sptr<FormHostClient> FormHostClient::instance_ = nullptr;
std::mutex FormHostClient::instanceMutex_;

FormHostClient::FormHostClient()
{
}

FormHostClient::~FormHostClient()
{
}

/**
 * @brief Get FormHostClient instance.
 *
 * @return FormHostClient instance.
 */
sptr<FormHostClient> FormHostClient::GetInstance()
{
    if (instance_ == nullptr) {
        std::lock_guard<std::mutex> lock_l(instanceMutex_);
        if (instance_ == nullptr) {
            instance_ = new FormHostClient();
        }
    }
    return instance_;
}

/**
 * @brief Add form.
 *
 * @param formCallback the host's form callback. 
 * @param formId The Id of the form. 
 * @return none.
 */
void FormHostClient::AddForm(std::shared_ptr<FormCallbackInterface> formCallback, const int64_t formId)
{
    APP_LOGI("%{public}s called.", __func__);

    if (formId <= 0) {
        APP_LOGE("%{public}s error, the passed form id can't be negative or zero.", __func__);
        return;
    }

    {
        std::lock_guard<std::mutex> lock(lockMutex_);
        int64_t key = FindKeyByCallback(formCallback);
        if (key == -1) {
            HostForms hostForms;
            int64_t tempKey = key_;
            if (!keyVector_.empty()) {
                tempKey = keyVector_.back();
                keyVector_.pop_back();
            }
            hostForms.AddForm(formId);                  
            recordCallback_.insert(std::make_pair(tempKey, formCallback));
            recordHostForms_.insert(std::make_pair(tempKey, hostForms));

            if (tempKey == key_) {
                key_++;
            }
        } else {
            recordHostForms_[key].AddForm(formId);
        }
    }
}

/**
 * @brief Remove form.
 *
 * @param formCallback the host's form callback. 
 * @param formId The Id of the form. 
 * @return none.
 */
void FormHostClient::RemoveForm(std::shared_ptr<FormCallbackInterface> formCallback, const int64_t formId)
{
    APP_LOGI("%{public}s called.", __func__);

    if (formId <= 0 || formCallback == nullptr) {
        APP_LOGE("%{public}s, invalid param.", __func__);
        return;
    }

    {
        std::lock_guard<std::mutex> lock(lockMutex_);
        int64_t key = FindKeyByCallback(formCallback);
        if(key == -1) {
            APP_LOGE("%{public}s, failed to find callback.", __func__);
            return;
        }

        if (recordHostForms_[key].IsEmpty()) {
            recordCallback_.erase(key);
            recordHostForms_.erase(key);
            keyVector_.push_back(key);
            APP_LOGI("%{public}s, clear data.", __func__);
            return;
        }
        
        recordHostForms_[key].DelForm(formId);
        if (recordHostForms_[key].IsEmpty()) {
            recordCallback_.erase(key);
            recordHostForms_.erase(key);
            keyVector_.push_back(key);
            APP_LOGI("%{public}s, clear data.", __func__);
        }
    }
    APP_LOGI("%{public}s end.", __func__);
}

/**
 * @brief Check whether the form exist in the formhosts.
 *
 * @param formId The Id of the form. 
 * @return Returns true if contains form; returns false otherwise.
 */
bool FormHostClient::ContainsForm(int64_t formId)
{
    APP_LOGI("%{public}s called.", __func__);

    std::lock_guard<std::mutex> lock(lockMutex_);
    for (auto recordHostForm : recordHostForms_) {
        if (recordHostForm.second.Contains(formId)) {
            return true;
        }
    }
    return false;
}

/**
 * @brief Request to give back a form.
 *
 * @param formJsInfo Form js info.
 * @return none.
 */
void FormHostClient::OnAcquired(const FormJsInfo &formJsInfo)
{
    APP_LOGI("%{public}s called.", __func__);
    int64_t formId = formJsInfo.formId; 
    if (formId < 0) {
        APP_LOGE("%{public}s error, the passed form id can't be negative.", __func__);
        return;
    }
    std::shared_ptr<FormCallbackInterface> targetCallback = FindTargetCallback(formId);
    if (targetCallback == nullptr) {
        // APP_LOGE("%{public}s error, can't find target callback. formId: %{public}lld.", __func__, formId);
        return;
    }
    // APP_LOGI("%{public}s, formId: %{public}lld, data: %{public}s", __func__, formId, formJsInfo.formData.c_str());
    targetCallback->ProcessFormUpdate(formJsInfo);
}

/**
 * @brief Update form.
 *
 * @param formJsInfo Form js info.
 * @return none.
 */
void FormHostClient::OnUpdate(const FormJsInfo &formJsInfo)
{
    APP_LOGI("%{public}s called.", __func__);
    int64_t formId = formJsInfo.formId; 
    if (formId < 0) {
        APP_LOGE("%{public}s error, the passed form id can't be negative.", __func__);
        return;
    }
    std::shared_ptr<FormCallbackInterface> targetCallback = FindTargetCallback(formId);
    if (targetCallback == nullptr) {
        // APP_LOGE("%{public}s error, can't find target callback. formId: %{public}lld.", __func__, formId);
        return;
    }
    targetCallback->ProcessFormUpdate(formJsInfo);
}

/**
 * @brief UnInstall the forms.
 *
 * @param formIds The Id of the forms. 
 * @return none.
 */
void FormHostClient::OnUninstall(const std::vector<int64_t> &formIds)
{
    APP_LOGI("%{public}s called.", __func__);
    if (formIds.size() <= 0) {
        APP_LOGE("%{public}s error, formIds is empty.", __func__);
        return;
    }
    for(auto &formId : formIds) {
         if (formId < 0) {
            APP_LOGE("%{public}s error, the passed form id can't be negative.", __func__);
            continue;
        }
        std::shared_ptr<FormCallbackInterface> targetCallback = FindTargetCallback(formId);
        if (targetCallback == nullptr) {
            // APP_LOGE("%{public}s error, can't find target callback. formId: %{public}lld.", __func__, formId);
            continue;
        }
        targetCallback->ProcessFormUninstall(formId);
    }   
}

/**
 * @brief Find callback by formId.
 *
 * @param formId The Id of the form.
 * @return target callback
 */
std::shared_ptr<FormCallbackInterface> FormHostClient::FindTargetCallback(int64_t formId)
{
    std::lock_guard<std::mutex> lock(lockMutex_);
    for (auto record : recordHostForms_) {
        if (record.second.Contains(formId)) {
            return recordCallback_[record.first];
        }
    }
    return nullptr;
}

/**
 * @brief Find Key By form callback.
 *
 * @param formCallback The form callback. 
 * @return callback's key
 */
int32_t FormHostClient::FindKeyByCallback(std::shared_ptr<FormCallbackInterface> formCallback)
{
    for (auto recordCallback : recordCallback_) {
        if (Compare(recordCallback.second, formCallback)) {
            return recordCallback.first;
        }
    }
    return -1;
}

/**
 * @brief Compare form callback.
 *
 * @param formCallback1 The form callback. 
 * @param formCallback2 The form callback to be compared with form callback1. 
 * @return Returns true if the two form callback are equal to each other, returns false otherwise.
 */
bool FormHostClient::Compare(std::shared_ptr<FormCallbackInterface> formCallback1, 
    std::shared_ptr<FormCallbackInterface> formCallback2)
{
    return (formCallback1 == formCallback2) ? true : false;
}
}  // namespace AppExecFwk
}  // namespace OHOS
