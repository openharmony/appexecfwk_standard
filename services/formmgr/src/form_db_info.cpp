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

#include "app_log_wrapper.h"
#include "form_db_info.h"
namespace OHOS {
namespace AppExecFwk {
namespace {
const std::string FORM_ID = "formId";
const std::string FORM_NAME = "formName";
const std::string BUNDLE_NAME = "bundleName";
const std::string MODULE_NAME = "moduleName";
const std::string ABILITY_NAME = "abilityName";
const std::string FORM_USER_UIDS = "formUserUids";
}

/**
 * @brief Transform the InnerFormInfo object to json.
 * @param jsonObject Indicates the obtained json object.
 * @return
 */
void InnerFormInfo::ToJson(nlohmann::json &jsonObject) const
{
    jsonObject[FORM_ID] = formDBInfo_.formId;
    jsonObject[FORM_NAME] = formDBInfo_.formName;
    jsonObject[BUNDLE_NAME] = formDBInfo_.bundleName;
    jsonObject[MODULE_NAME] = formDBInfo_.moduleName;
    jsonObject[ABILITY_NAME] = formDBInfo_.abilityName;
    jsonObject[FORM_USER_UIDS] = formDBInfo_.formUserUids;
}

/**
 * @brief Transform the json object to InnerFormInfo object.
 * @param jsonObject Indicates the obtained json object.
 * @return
 */
bool InnerFormInfo::FromJson(const nlohmann::json &jsonObject)
{
        formDBInfo_.formId = jsonObject.at(FORM_ID).get<int64_t>();
        formDBInfo_.formName = jsonObject.at(FORM_NAME).get<std::string>();
        formDBInfo_.bundleName = jsonObject.at(BUNDLE_NAME).get<std::string>();
        formDBInfo_.moduleName = jsonObject.at(MODULE_NAME).get<std::string>();
        formDBInfo_.abilityName = jsonObject.at(ABILITY_NAME).get<std::string>();
        formDBInfo_.formUserUids = jsonObject.at(FORM_USER_UIDS).get<std::vector<int32_t>>();

    return true;
}

void InnerFormInfo::AddUserUid(const int callingUid)
{
    auto iter = std::find(formDBInfo_.formUserUids.begin(), formDBInfo_.formUserUids.end(), callingUid);
    if (iter == formDBInfo_.formUserUids.end()) {
        formDBInfo_.formUserUids.push_back(callingUid);
    }    
}

bool InnerFormInfo::DeleteUserUid(const int callingUid)
{
    auto iter = std::find(formDBInfo_.formUserUids.begin(), formDBInfo_.formUserUids.end(), callingUid);
    if (iter == formDBInfo_.formUserUids.end()) {
        return false;
    }
    formDBInfo_.formUserUids.erase(iter);
    return true;
}
}  // namespace AppExecFwk
}  // namespace OHOS