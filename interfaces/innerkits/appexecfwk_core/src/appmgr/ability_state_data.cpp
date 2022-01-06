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

#include "ability_state_data.h"

#include "app_log_wrapper.h"

namespace OHOS {
namespace AppExecFwk {
bool AbilityStateData::Marshalling(Parcel &parcel) const
{
    return (parcel.WriteString(bundleName) && parcel.WriteString(abilityName) &&
        parcel.WriteInt32(abilityState) &&
        parcel.WriteInt32(pid) && parcel.WriteInt32(uid) && parcel.WriteParcelable(token));
}

bool AbilityStateData::ReadFromParcel(Parcel &parcel)
{
    bundleName = parcel.ReadString();

    abilityName = parcel.ReadString();

    abilityState = parcel.ReadInt32();

    pid = parcel.ReadInt32();

    uid = parcel.ReadInt32();

    token = parcel.ReadParcelable<IRemoteObject>();

    return true;
}

AbilityStateData *AbilityStateData::Unmarshalling(Parcel &parcel)
{
    AbilityStateData *abilityStateData = new (std::nothrow) AbilityStateData();
    if (abilityStateData && !abilityStateData->ReadFromParcel(parcel)) {
        APP_LOGW("AbilityStateData failed, because ReadFromParcel failed");
        delete abilityStateData;
        abilityStateData = nullptr;
    }
    return abilityStateData;
}
}  // namespace AppExecFwk
}  // namespace OHOS
