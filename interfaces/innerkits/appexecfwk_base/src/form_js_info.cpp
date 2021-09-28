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

#include "form_js_info.h"
#include "string_ex.h"

namespace OHOS {
namespace AppExecFwk {
bool FormJsInfo::ReadFromParcel(Parcel &parcel)
{
    formId = parcel.ReadInt64();
    formName = Str16ToStr8(parcel.ReadString16());
    bundleName = Str16ToStr8(parcel.ReadString16());
    abilityName = Str16ToStr8(parcel.ReadString16());

    formTempFlg = parcel.ReadBool();
    jsFormCodePath = Str16ToStr8(parcel.ReadString16());
    formData = Str16ToStr8(parcel.ReadString16());

    auto bindingData = parcel.ReadParcelable<FormProviderData>();
    if (nullptr == bindingData){
        return false;
    }
    formProviderData = *bindingData;
    return true;
}

FormJsInfo *FormJsInfo::Unmarshalling(Parcel &parcel)
{
    FormJsInfo *formJsInfo = new (std::nothrow) FormJsInfo();
    if (formJsInfo && !formJsInfo->ReadFromParcel(parcel)) {
        delete formJsInfo;
        formJsInfo = nullptr;
    }
    return formJsInfo;
}

bool FormJsInfo::Marshalling(Parcel &parcel) const
{
    // write formId
    if (!parcel.WriteInt64(formId)) {
        return false;
    }
    // write formName
    if (!parcel.WriteString16(Str8ToStr16(formName))) {
        return false;
    }
    // write bundleName
    if (!parcel.WriteString16(Str8ToStr16(bundleName))) {
        return false;
    }
    // write abilityName
    if (!parcel.WriteString16(Str8ToStr16(abilityName))) {
        return false;
    }

    // write tempFlag
    if (!parcel.WriteBool(formTempFlg)) {
        return false;
    }

    // write jsFormCodePath
    if (!parcel.WriteString16(Str8ToStr16(jsFormCodePath))) {
        return false;
    }

    // write formData
    if (!parcel.WriteString16(Str8ToStr16(formData))) {
        return false;
    }

    // write formProviderData
    if (!parcel.WriteParcelable(&formProviderData)) {
        return false;
    }

    return true;
}
}  // namespace AppExecFwk
}  // namespace OHOS