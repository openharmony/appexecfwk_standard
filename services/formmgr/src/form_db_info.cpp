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

#include "form_db_info.h"

#include "json_util.h"

namespace OHOS {
namespace AppExecFwk {
namespace {
const std::string INNER_FORM_INFO_FORM_ID = "formId";
const std::string INNER_FORM_INFO_FORM_NAME = "formName";
const std::string INNER_FORM_INFO_BUNDLE_NAME = "bundleName";
const std::string INNER_FORM_INFO_MODULE_NAME = "moduleName";
const std::string INNER_FORM_INFO_ABILITY_NAME = "abilityName";
const std::string INNER_FORM_INFO_FORM_USER_UIDS = "formUserUids";
}

/**
 * @brief Transform the InnerFormInfo object to json.
 * @param jsonObject Indicates the obtained json object.
 * @return
 */
void InnerFormInfo::ToJson(nlohmann::json &jsonObject) const
{
    jsonObject[INNER_FORM_INFO_FORM_ID] = formDBInfo_.formId;
    jsonObject[INNER_FORM_INFO_FORM_NAME] = formDBInfo_.formName;
    jsonObject[INNER_FORM_INFO_BUNDLE_NAME] = formDBInfo_.bundleName;
    jsonObject[INNER_FORM_INFO_MODULE_NAME] = formDBInfo_.moduleName;
    jsonObject[INNER_FORM_INFO_ABILITY_NAME] = formDBInfo_.abilityName;
    jsonObject[INNER_FORM_INFO_FORM_USER_UIDS] = formDBInfo_.formUserUids;
}

/**
 * @brief Transform the json object to InnerFormInfo object.
 * @param jsonObject Indicates the obtained json object.
 * @return
 */
bool InnerFormInfo::FromJson(const nlohmann::json &jsonObject)
{
    const auto &jsonObjectEnd = jsonObject.end();
    int32_t parseResult = ERR_OK;
    GetValueIfFindKey<int64_t>(jsonObject,
        jsonObjectEnd,
        INNER_FORM_INFO_FORM_ID,
        formDBInfo_.formId,
        JsonType::NUMBER,
        false,
        parseResult,
        ArrayType::NOT_ARRAY);
    GetValueIfFindKey<std::string>(jsonObject,
        jsonObjectEnd,
        INNER_FORM_INFO_FORM_NAME,
        formDBInfo_.formName,
        JsonType::STRING,
        false,
        parseResult,
        ArrayType::NOT_ARRAY);
    GetValueIfFindKey<std::string>(jsonObject,
        jsonObjectEnd,
        INNER_FORM_INFO_BUNDLE_NAME,
        formDBInfo_.bundleName,
        JsonType::STRING,
        false,
        parseResult,
        ArrayType::NOT_ARRAY);
    GetValueIfFindKey<std::string>(jsonObject,
        jsonObjectEnd,
        INNER_FORM_INFO_MODULE_NAME,
        formDBInfo_.moduleName,
        JsonType::STRING,
        false,
        parseResult,
        ArrayType::NOT_ARRAY);
    GetValueIfFindKey<std::string>(jsonObject,
        jsonObjectEnd,
        INNER_FORM_INFO_ABILITY_NAME,
        formDBInfo_.abilityName,
        JsonType::STRING,
        false,
        parseResult,
        ArrayType::NOT_ARRAY);
    GetValueIfFindKey<std::vector<int32_t>>(jsonObject,
        jsonObjectEnd,
        INNER_FORM_INFO_FORM_USER_UIDS,
        formDBInfo_.formUserUids,
        JsonType::ARRAY,
        false,
        parseResult,
        ArrayType::NUMBER);
    return parseResult == ERR_OK;
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