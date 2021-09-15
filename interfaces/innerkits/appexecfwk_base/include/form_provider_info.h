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

#ifndef FOUNDATION_APPEXECFWK_OHOS_FORM_PROVIDER_INFO_H
#define FOUNDATION_APPEXECFWK_OHOS_FORM_PROVIDER_INFO_H

#include <string>
#include "form_provider_data.h"
#include "parcel.h"

namespace OHOS {
namespace AppExecFwk {
/**
 * @struct FormProviderInfo
 * Defines form provider info.
 */
class FormProviderInfo : public Parcelable {
public:
    FormProviderInfo() = default;
    ~FormProviderInfo() = default;
    /**
     * @brief Set the id of the form.
     * @param formId the id of the form.
     */
    inline void SetFormId(const int64_t formId)
    {
        formId_ = formId;
    }
    /**
     * @brief Get the id of the form.
     * @return the id of the form.
     */
    inline int64_t GetFormId() const
    {
        return formId_;
    }
    /**
     * @brief Set the form data.
     * @param formProviderData The form data.
     */
    inline void SetFormData(const FormProviderData &formProviderData)
    {
        jsBindingData_ = formProviderData;
    }

    /**
     * @brief Get the form data.
     * @return the form data.
     */
    inline FormProviderData GetFormData() const
    {
        return jsBindingData_;
    }
    /**
     * @brief Get the form data.
     * @return the form data.
     */
    inline std::string GetFormDataString() const
    {
        return jsBindingData_.GetDataString();
    }

    /**
     * @brief Set the upgrade flg.
     * @param upgradeFlg The upgrade flg.
     */
    inline void SetUpgradeFlg(const bool upgradeFlg)
    {
        upgradeFlg_ = upgradeFlg;
    }
    /**
     * @brief Get the upgrade flg.
     * @return the upgrade flg.
     */
    inline bool GetUpgradeFlg() const
    {
        return upgradeFlg_;
    }

    /**
     * @brief Set form date by string.
     * @param dataString string json data.
     */
    void SetFormDataString(std::string &dataString);

    /**
     * @brief Merge new data to FormProviderData.
     * @param addJsonData data to merge to FormProviderData
     */
    void MergeData(nlohmann::json &addJsonData);

    bool ReadFromParcel(Parcel &parcel);
    virtual bool Marshalling(Parcel &parcel) const override;
    static FormProviderInfo *Unmarshalling(Parcel &parcel);
private:
    int64_t formId_;
    FormProviderData jsBindingData_;
    bool upgradeFlg_;
};
}  // namespace AppExecFwk
}  // namespace OHOS
#endif  // FOUNDATION_APPEXECFWK_OHOS_FORM_PROVIDER_INFO_H
