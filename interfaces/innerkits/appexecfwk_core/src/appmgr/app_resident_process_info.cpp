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

#include "app_resident_process_info.h"
#include "app_log_wrapper.h"

namespace OHOS {
namespace AppExecFwk {
AppResidentProcessInfo *AppResidentProcessInfo::Unmarshalling(Parcel &parcel)
{
    AppResidentProcessInfo *residentProcess = new (std::nothrow) OHOS::AppExecFwk::AppResidentProcessInfo();
    if (residentProcess && !residentProcess->ReadFromParcel(parcel)) {
        delete residentProcess;
        residentProcess = nullptr;
    }
    return residentProcess;
}

bool AppResidentProcessInfo::Marshalling(Parcel &parcel) const
{
    APP_LOGW("Marshalling");
    parcel.WriteBool(isKeepAliveApp_);
    parcel.WriteInt32(abilityStage_.size());
    for (auto &info : abilityStage_) {
        if (!parcel.WriteParcelable(&info)) {
            return false;
        }
    }
    return true;
}

bool AppResidentProcessInfo::ReadFromParcel(Parcel &parcel)
{
    isKeepAliveApp_ = parcel.ReadBool();
    auto size = parcel.ReadInt32();
    for (int32_t i = 0; i < size; i++) {
        std::unique_ptr<HapModuleInfo> hapModuleInfo(parcel.ReadParcelable<HapModuleInfo>());
        if (!hapModuleInfo) {
            APP_LOGE("ReadParcelable<AbilityInfo> failed");
            return false;
        }
        abilityStage_.emplace_back(*hapModuleInfo);
    }
    return true;
}
}  // namespace AppExecFwk
}  // namespace OHOS