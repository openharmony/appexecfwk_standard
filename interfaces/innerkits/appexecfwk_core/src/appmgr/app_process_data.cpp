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

#include "app_process_data.h"

#include "app_log_wrapper.h"

#include "nlohmann/json.hpp"
#include "string_ex.h"
#include "parcel_macro.h"

namespace OHOS {
namespace AppExecFwk {
namespace {
bool ReadFromParcelAppData(std::vector<AppData> &appDatas, Parcel &parcel)
{
    int32_t appDataSize;
    READ_PARCEL_AND_RETURN_FALSE_IF_FAIL(Int32, parcel, appDataSize);
    for (auto i = 0; i < appDataSize; i++) {
        AppData appDataInfo;
        std::string appName = Str16ToStr8(parcel.ReadString16());
        int32_t uid = parcel.ReadInt32();
        appDataInfo.appName = appName;
        appDataInfo.uid = uid;
        appDatas.emplace_back(appDataInfo);
    }
    return true;
}
}  // namespace

bool AppProcessData::Marshalling(Parcel &parcel) const
{
    WRITE_PARCEL_AND_RETURN_FALSE_IF_FAIL(String16, parcel, Str8ToStr16(processName));

    WRITE_PARCEL_AND_RETURN_FALSE_IF_FAIL(Int32, parcel, static_cast<int32_t>(appState));

    WRITE_PARCEL_AND_RETURN_FALSE_IF_FAIL(Int32, parcel, pid);

    const auto appDataSize = static_cast<int32_t>(appDatas.size());
    WRITE_PARCEL_AND_RETURN_FALSE_IF_FAIL(Int32, parcel, appDataSize);
    for (auto i = 0; i < appDataSize; i++) {
        WRITE_PARCEL_AND_RETURN_FALSE_IF_FAIL(String16, parcel, Str8ToStr16(appDatas[i].appName));
        WRITE_PARCEL_AND_RETURN_FALSE_IF_FAIL(Int32, parcel, appDatas[i].uid);
    }

    return true;
}

bool AppProcessData::ReadFromParcel(Parcel &parcel)
{
    processName = Str16ToStr8(parcel.ReadString16());

    appState = static_cast<ApplicationState>(parcel.ReadInt32());

    pid = parcel.ReadInt32();

    ReadFromParcelAppData(appDatas, parcel);

    return true;
}

AppProcessData *AppProcessData::Unmarshalling(Parcel &parcel)
{
    AppProcessData *appProcessData = new (std::nothrow) AppProcessData();
    if (appProcessData && !appProcessData->ReadFromParcel(parcel)) {
        APP_LOGW("failed, because ReadFromParcel failed");
        delete appProcessData;
        appProcessData = nullptr;
    }
    return appProcessData;
}
}  // namespace AppExecFwk
}  // namespace OHOS
