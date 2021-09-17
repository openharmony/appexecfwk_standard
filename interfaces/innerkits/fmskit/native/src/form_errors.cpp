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

#include "appexecfwk_errors.h"
#include "form_errors.h"

namespace OHOS {
namespace AppExecFwk {
FormErrors::FormErrors()
{
    InitErrorMessageMap();
}
FormErrors::~FormErrors()
{
}

/**
 * @brief Get the error message content.
 * 
 * @param errCode Error code.
 * @return Message content.
 */
std::string FormErrors::GetErrorMessage(int errCode)
{
    if(errorMessageMap_.find(errCode) != errorMessageMap_.end()) {
        return errorMessageMap_[errCode];
    }
    else {
        return errorMessageMap_[ERR_APPEXECFWK_FORM_COMMON_CODE];
    }
}

/**
 * @brief Init error message map object.
 * 
 */
void FormErrors::InitErrorMessageMap()
{
    errorMessageMap_ =  { //  error + message        
        {
            ERR_APPEXECFWK_FORM_COMMON_CODE, "some internal server error occurs.",
        },
        {
            ERR_APPEXECFWK_FORM_PERMISSION_DENY, 
            "check permission deny, need to request ohos.permission.REQUIRE_FORM.",
        },
        {
            ERR_APPEXECFWK_FORM_GET_INFO_FAILED, "can't get form info by the formName.",
        },
        {
            ERR_APPEXECFWK_FORM_GET_BUNDLE_FAILED, "the requested bundle name does not exist.",
        },
        {
            ERR_APPEXECFWK_FORM_INVALID_PARAM, "invalid params received on operating form.",
        },
        {
            ERR_APPEXECFWK_FORM_CFG_NOT_MATCH_ID, "the form id and form config are not matched.",
        },
        {
            ERR_APPEXECFWK_FORM_NOT_EXIST_ID, "the requested form id is not existed on fms.",
        },
        {
            ERR_APPEXECFWK_FORM_BIND_PROVIDER_FAILED, "fms bind provider failed.",
        },
        {
            ERR_APPEXECFWK_FORM_MAX_SYSTEM_FORMS, "exceed max forms in system, current limit is 512.",
        },
        {
            ERR_APPEXECFWK_FORM_EXCEED_INSTANCES_PER_FORM, "exceed max instances per form, limit is 32.",
        },
        {
            ERR_APPEXECFWK_FORM_OPERATION_NOT_SELF, 
            "the form to be operated is not self-owned or has been deleted already.",
        },
        {
            ERR_APPEXECFWK_FORM_PROVIDER_DEL_FAIL, "fms notify provider to delete failed.",
        },
        {
            ERR_APPEXECFWK_FORM_MAX_FORMS_PER_CLIENT, "exceed max forms per client, limit is 256.",
        },
        {
            ERR_APPEXECFWK_FORM_MAX_SYSTEM_TEMP_FORMS, "exceed max temp forms in system, limit is 256.",
        },
        {
            ERR_APPEXECFWK_FORM_NO_SUCH_MODULE, "the module not exist in the bundle.",
        },
        {
            ERR_APPEXECFWK_FORM_NO_SUCH_ABILITY, "the ability not exist in the module.",
        },
        {
            ERR_APPEXECFWK_FORM_NO_SUCH_DIMENSION, "the dimension not exist in the form.",
        },
        {
            ERR_APPEXECFWK_FORM_FA_NOT_INSTALLED, "the ability not installed,need install first.",
        },
        {
            ERR_APPEXECFWK_FORM_MAX_REQUEST, "too many request,try again later.",
        },
        {
            ERR_APPEXECFWK_FORM_MAX_REFRESH, "already refresh 50 times.",
        },
        {
            ERR_APPEXECFWK_FORM_GET_BMS_FAILED, "get bms rpc failed.",
        },
        {
            ERR_APPEXECFWK_FORM_GET_FMS_FAILED, "get fms rpc failed.",
        },
        {
            ERR_APPEXECFWK_FORM_SEND_FMS_MSG, "send request to fms failed.",
        },
        {
            ERR_APPEXECFWK_FORM_FORM_DUPLICATE_ADDED, "form do not support acquire same id twice.",
        },
        {
            ERR_APPEXECFWK_FORM_IN_RECOVER, "form is in recover status, can't do action on form.",
        },
        {
            ERR_APPEXECFWK_FORM_GET_SYSMGR_FAILED, "get system manager failed.",
        },
        {
            ERR_APPEXECFWK_PARCEL_ERROR, "read parcelable info failed.",
        },
    };
}

}  // namespace AppExecFwk
}  // namespace OHOS