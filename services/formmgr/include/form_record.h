/*
 * Copyright (c) 2021 Huawei Device Co., Ltd.
 * Licensed under the Apache License, Version 2.0 (the "License")_;
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

#ifndef FOUNDATION_APPEXECFWK_SERVICES_FORMMGR_INCLUDE_FORM_RECORD_H
#define FOUNDATION_APPEXECFWK_SERVICES_FORMMGR_INCLUDE_FORM_RECORD_H

#include <vector>
#include "form_provider_info.h"

namespace OHOS {
namespace AppExecFwk {
/**
 * @class FormRecord
 * Form data.
 */
class FormRecord {
public:
    int64_t formId;
    bool isInited = false;
    bool needFreeInstall = false;
    bool versionUpgrade = false;
    bool needRefresh = false;
    bool isCountTimerRefresh = false;
    std::string packageName = "";
    std::string bundleName = "";
    std::string moduleName = "";
    std::string abilityName = "";
    std::string formName = "";
    int32_t specification;
    bool isEnableUpdate = false;
    long updateDuration = false;
    int updateAtHour = 0;
    int updateAtMin = 0;
    FormProviderInfo formProviderInfo;
    std::vector<std::string> hapSourceDirs;
    std::string jsFormCodePath = "";
    bool formTempFlg = false;
    std::vector<int> formUserUids;
    bool formVisibleNotify = false;
    int formVisibleNotifyState = 0;
};
}  // namespace AppExecFwk
}  // namespace OHOS

#endif // FOUNDATION_APPEXECFWK_SERVICES_FORMMGR_INCLUDE_FORM_RECORD_H
