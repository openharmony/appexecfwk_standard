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

#ifndef FOUNDATION_APPEXECFWK_OHOS_FORM_HOST_CLIENT_H
#define FOUNDATION_APPEXECFWK_OHOS_FORM_HOST_CLIENT_H

#include <map>
#include <memory>
#include <mutex>
#include <vector>
#include "form_callback_interface.h"
#include "form_host_stub.h"

namespace OHOS {
namespace AppExecFwk {
/**
 * @class FormHostClient
 * The service of the form host.
 */
class HostForms;
class FormHostClient : public FormHostStub {
public:
    FormHostClient();
    virtual ~FormHostClient();

    /**
     * @brief Get FormHostClient instance.
     *
     * @return FormHostClient instance.
     */
    static sptr<FormHostClient> GetInstance();

    /**
     * @brief Add form.
     *
     * @param formCallback the host's form callback. 
     * @param formId The Id of the form. 
     * @return none.
     */
    void AddForm(std::shared_ptr<FormCallbackInterface> formCallback, const int64_t formId);

    /**
     * @brief Remove form.
     *
     * @param formCallback the host's form callback. 
     * @param formId The Id of the form. 
     * @return none.
     */
    void RemoveForm(std::shared_ptr<FormCallbackInterface> formCallback, const int64_t formId);

    /**
     * @brief Check whether the form exist in the formhosts.
     *
     * @param formId The Id of the form. 
     * @return Returns true if contains form; returns false otherwise.
     */
    bool ContainsForm(int64_t formId);

    /**
     * @brief Request to give back a form.
     *
     * @param formJsInfo Form js info.
     * @return none.
     */
    virtual void OnAcquired(const FormJsInfo &formJsInfo);

     /**
     * @brief Update form.
     *
     * @param formJsInfo Form js info.
     * @return none.
     */
    virtual void OnUpdate(const FormJsInfo &formJsInfo);

    /**
     * @brief UnInstall the forms.
     *
     * @param formIds The Id of the forms. 
     * @return none.
     */
    virtual void OnUninstall(const std::vector<int64_t> &formIds);

private:
    static sptr<FormHostClient> instance_;
    static std::mutex instanceMutex_;
    mutable std::mutex lockMutex_;
    std::vector<int64_t> keyVector_;
    std::map<int64_t, std::shared_ptr<FormCallbackInterface>> recordCallback_;
    std::map<int64_t, HostForms> recordHostForms_;
    int32_t key_ = 0;

private:
    /**
     * @brief Find callback by formId.
     *
     * @param formId The Id of the form.
     * @return target callback
     */
    std::shared_ptr<FormCallbackInterface> FindTargetCallback(int64_t formId);

    /**
     * @brief Find Key By form callback.
     *
     * @param formCallback The form callback. 
     * @return callback's key
     */
    int32_t FindKeyByCallback(std::shared_ptr<FormCallbackInterface> formCallback);

    /**
     * @brief Compare callback.
     *
     * @param formCallback1 The  form callback1. 
     * @param formCallback2 The callback to be compared with form callback1. 
     * @return Returns true if the two callback are equal to each other, returns false otherwise.
     */
    bool Compare(std::shared_ptr<FormCallbackInterface> formCallback1, std::shared_ptr<FormCallbackInterface> formCallback2);

    DISALLOW_COPY_AND_MOVE(FormHostClient);
};

class HostForms {
public:
    /**
     * @brief Add form by formId.
     * 
     * @param formId The Id of the form.
     */
    void AddForm(const int64_t formId) 
    {
        std::map<int64_t, bool>::iterator it = forms_.find(formId);
        if (it != forms_.end()) {
            return;
        }
        forms_.insert(std::pair<int64_t, bool>(formId, true));
    }

    /**
     * @brief Delete form by formId.
     * 
     * @param formId The Id of the form.
     */
    void DelForm(const int64_t formId) 
    {
        forms_.erase(formId);
    }

    /**
     * @brief Check whether the form is empty.
     */
    bool IsEmpty() 
    {
        return forms_.empty();
    }

    /**
     * @brief Check whether the form exist in the forms.
     *
     * @param formId The Id of the form. 
     * @return Returns true if contains form; returns false otherwise.
     */
    bool Contains(const int64_t formId) 
    {
        std::map<int64_t, bool>::iterator it = forms_.find(formId);
        return (it == forms_.end()) ? false : true;
    }

private:
    std::map<int64_t, bool> forms_;
};
}  // namespace AppExecFwk
}  // namespace OHOS
#endif  // FOUNDATION_APPEXECFWK_OHOS_FORM_HOST_CLIENT_H
