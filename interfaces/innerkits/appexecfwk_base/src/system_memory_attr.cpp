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

#include "system_memory_attr.h"
#include "app_log_wrapper.h"

namespace OHOS {
namespace AppExecFwk {
SystemMemoryAttr::SystemMemoryAttr() : availSysMem_(0), totalSysMem_(0), threshold_(0), isSysInlowMem_(false)
{}

bool SystemMemoryAttr::ReadFromParcel(Parcel &parcel)
{
    parcel.ReadInt64(availSysMem_);
    parcel.ReadInt64(totalSysMem_);
    parcel.ReadInt64(threshold_);
    isSysInlowMem_ = parcel.ReadBool();

    APP_LOGD("SystemMemoryAttr::ReadFromParcel %{public}zu %{public}zu %{public}zu %{public}s",
        static_cast<size_t>(availSysMem_),
        static_cast<size_t>(totalSysMem_),
        static_cast<size_t>(threshold_),
        (isSysInlowMem_ ? "true" : "false"));

    return true;
}

SystemMemoryAttr *SystemMemoryAttr::Unmarshalling(Parcel &parcel)
{
    SystemMemoryAttr *info = new (std::nothrow) SystemMemoryAttr();
    if (info == nullptr) {
        return nullptr;
    }

    if (!info->ReadFromParcel(parcel)) {
        delete info;
        info = nullptr;
    }
    return info;
}

bool SystemMemoryAttr::Marshalling(Parcel &parcel) const
{
    parcel.WriteInt64(availSysMem_);
    parcel.WriteInt64(totalSysMem_);
    parcel.WriteInt64(threshold_);
    parcel.WriteBool(isSysInlowMem_);

    APP_LOGD("SystemMemoryAttr::Marshalling %{public}zu %{public}zu %{public}zu %{public}s",
        static_cast<size_t>(availSysMem_),
        static_cast<size_t>(totalSysMem_),
        static_cast<size_t>(threshold_),
        (isSysInlowMem_ ? "true" : "false"));

    return true;
}
}  // namespace AppExecFwk
}  // namespace OHOS