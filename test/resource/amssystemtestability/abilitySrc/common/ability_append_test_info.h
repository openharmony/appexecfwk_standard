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

#ifndef ABILITY_APPEND_TEST_INFO_H
#define ABILITY_APPEND_TEST_INFO_H

namespace OHOS {
namespace AppExecFwk {
const std::string g_EVENT_REQU_FIRST = "requ_com_ohos_amsst_appkit_first";
const std::string g_EVENT_RESP_FIRST = "resp_com_ohos_amsst_appkit_first";
const std::string g_EVENT_RESP_FIRST_LIFECYCLE = "resp_com_ohos_amsst_appkit_first_lifecycle";

const std::string g_EVENT_REQU_FIRSTB = "requ_com_ohos_amsst_appkit_firstb";
const std::string g_EVENT_RESP_FIRSTB = "resp_com_ohos_amsst_appkit_firstb";
const std::string g_EVENT_RESP_FIRSTB_LIFECYCLE = "resp_com_ohos_amsst_appkit_firstb_lifecycle";

const std::string g_EVENT_REQU_SECOND = "requ_com_ohos_amsst_appkit_second";
const std::string g_EVENT_RESP_SECOND = "resp_com_ohos_amsst_appkit_second";
const std::string g_EVENT_RESP_SECOND_LIFECYCLE = "resp_com_ohos_amsst_appkit_second_lifecycle";

const int MAIN_ABILITY_A_CODE = 100;
const int SECOND_ABILITY_A_CODE = 200;
const int MAIN_ABILITY_B_CODE = 300;

enum class AppendApi {
    OnSetCaller,
    TerminateAndRemoveMisson,
    GetString,
    GetStringArray,
    GetIntArray,
    GetPattern,
    GetColor,
    TerminateAbilityResult,
    GetDispalyOrientation,
    GetPreferencesDir,
    StartAbilities,
    GetColorMode,
    SetColorMode,
    IsFirstInMission,
    End
};

}  // namespace AppExecFwk
}  // namespace OHOS
#endif  // ABILITY_APPEND_TEST_INFO_H