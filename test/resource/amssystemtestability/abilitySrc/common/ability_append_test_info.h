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

const std::string g_EVENT_REQU_THIRD = "requ_com_ohos_amsst_appkit_third";
const std::string g_EVENT_RESP_THIRD = "resp_com_ohos_amsst_appkit_third";
const std::string g_EVENT_RESP_THIRD_LIFECYCLE = "resp_com_ohos_amsst_appkit_third_lifecycle";

const std::string g_EVENT_REQU_FOURTH = "requ_com_ohos_amsst_appkit_fourth";
const std::string g_EVENT_RESP_FOURTH = "resp_com_ohos_amsst_appkit_fourth";
const std::string g_EVENT_RESP_FOURTH_LIFECYCLE = "resp_com_ohos_amsst_appkit_fourth_lifecycle";

const std::string g_EVENT_REQU_FIFTH = "requ_com_ohos_amsst_appkit_fifth";
const std::string g_EVENT_RESP_FIFTH = "resp_com_ohos_amsst_appkit_fifth";
const std::string g_EVENT_RESP_FIFTH_LIFECYCLE = "resp_com_ohos_amsst_appkit_fifth_lifecycle";

const std::string g_EVENT_REQU_SIXTH = "requ_com_ohos_amsst_appkit_sixth";
const std::string g_EVENT_RESP_SIXTH = "resp_com_ohos_amsst_appkit_sixth";
const std::string g_EVENT_RESP_SIXTH_LIFECYCLE = "resp_com_ohos_amsst_appkit_sixth_lifecycle";

const std::string g_EVENT_REQU_MAIN = "requ_com_ohos_amsst_appkit_main";
const std::string g_EVENT_RESP_MAIN = "resp_com_ohos_amsst_appkit_main";
const std::string g_EVENT_RESP_MAIN_LIFECYCLE = "resp_com_ohos_amsst_appkit_main_lifecycle";

const std::string g_EVENT_REQU_MAIN_SUBSIDIARY = "requ_com_ohos_amsst_appkit_main_subsidiary";
const std::string g_EVENT_RESP_MAIN_SUBSIDIARY = "resp_com_ohos_amsst_appkit_main_subsidiary";
const std::string g_EVENT_RESP_MAIN_LIFECYCLE_SUBSIDIARY = "resp_com_ohos_amsst_appkit_main_lifecycle_subsidiary";

const std::string g_EVENT_REQU_SECOND_SUBSIDIARY = "requ_com_ohos_amsst_appkit_second_subsidiary";
const std::string g_EVENT_RESP_SECOND_SUBSIDIARY = "resp_com_ohos_amsst_appkit_second_subsidiary";
const std::string g_EVENT_RESP_SECOND_LIFECYCLE_SUBSIDIARY = "resp_com_ohos_amsst_appkit_second_lifecycle_subsidiary";

const int MAIN_ABILITY_A_CODE = 100;
const int SECOND_ABILITY_A_CODE = 200;
const int MAIN_ABILITY_B_CODE = 300;

const int EXPECT_ST_ZERO =   0;

const int MAIN_ABILITY_CODE =   100;
const int SECOND_ABILITY_CODE = 200;
const int THIRD_ABILITY_CODE =  300;
const int FOURTH_ABILITY_CODE = 400;
const int FIFTH_ABILITY_CODE =  500;
const int SIXTH_ABILITY_CODE =  600;

const int MAIN_ABILITY_CODE_SUBSIDIARY =   10100;
const int SECOND_ABILITY_CODE_SUBSIDIARY = 10200;

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

enum class MissionStackApi {
    LockMission = (int)AppendApi::End,
    End
};

enum class TestAbilityState {
    OnSaveAbilityState = (int)MissionStackApi::End,
    OnRestoreAbilityState,
    End
};
}  // namespace AppExecFwk
}  // namespace OHOS
#endif  // ABILITY_APPEND_TEST_INFO_H