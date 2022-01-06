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

#ifndef FOUNDATION_APPEXECFWK_OHOS_FORM_JS_INFO_H
#define FOUNDATION_APPEXECFWK_OHOS_FORM_JS_INFO_H


#include <string>
#include "form_provider_data.h"
#include "form_info.h"
#include "parcel.h"

namespace OHOS {
namespace AppExecFwk {
/**
 * @struct FormJsInfo
 * Defines form js info.
 */
struct FormJsInfo : public Parcelable {
    int64_t formId;
    std::string formName;
    std::string bundleName;
    std::string abilityName;
    bool formTempFlg = false;
    std::string jsFormCodePath;
    std::string formData;
    FormProviderData formProviderData;

    std::string htmlPath;
    std::string cssPath;
    std::string jsPath;
    std::string fileReousePath;
    std::string formSrc;
    FormWindow formWindow;
    uint32_t versionCode = 0;
    std::string versionName;
    uint32_t compatibleVersion = 0;
    std::string icon;

    bool ReadFromParcel(Parcel &parcel);
    virtual bool Marshalling(Parcel &parcel) const override;
    static FormJsInfo *Unmarshalling(Parcel &parcel);
};
}  // namespace AppExecFwk
}  // namespace OHOS
#endif  // FOUNDATION_APPEXECFWK_OHOS_FORM_JS_INFO_H