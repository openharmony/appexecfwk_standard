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
#include <getopt.h>
#include <unistd.h>
#include <cinttypes>

#include "appexecfwk_errors.h"
#include "app_log_wrapper.h"
#include "fms_command.h"
#include "if_system_ability_manager.h"
#include "ipc_skeleton.h"
#include "iservice_registry.h"
#include "string_ex.h"
#include "system_ability_definition.h"

using namespace OHOS::AAFwk;

namespace OHOS {
namespace AppExecFwk {
namespace {
const int COMMAND_QUERY_INVALID = -1;
const int COMMAND_QUERY_STORAGE = 0;
const int COMMAND_QUERY_NAME = 1;
const int COMMAND_QUERY_ID = 2;

// const std::string BUNDLE_NAME_EMPTY = "";

const std::string SHORT_OPTIONS = "hsn:i:";
const struct option LONG_OPTIONS[] = {
    {"help", no_argument, nullptr, 'h'},
    {"storage", no_argument, nullptr, 's'},
    {"bundle-name", required_argument, nullptr, 'n'},
    {"form-id", required_argument, nullptr, 'i'},
    {0, 0, 0, 0},
};
}  // namespace

FormMgrShellCommand::FormMgrShellCommand(int argc, char *argv[]) : ShellCommand(argc, argv, FM_TOOL_NAME)
{
}
ErrCode FormMgrShellCommand::init()
{
    return ERR_OK;
}
/**
 * @brief Create command map.
 */
ErrCode FormMgrShellCommand::CreateCommandMap()
{
    commandMap_ = {
        {"help", std::bind(&FormMgrShellCommand::RunAsHelpCommand, this)},
        {"query", std::bind(&FormMgrShellCommand::RunAsQueryCommand, this)},
        // {"dump", std::bind(&FormMgrShellCommand::RunAsDumpCommand, this)},
    };

    return OHOS::ERR_OK;
}
/**
 * @brief Create message map.
 */  
ErrCode FormMgrShellCommand::CreateMessageMap()
{
    messageMap_ = {
        //  error + message
        {
            ERR_APPEXECFWK_INSTALL_INTERNAL_ERROR,
            "error: fm query internal error.",
        },
        {
            ERR_APPEXECFWK_FORM_INVALID_PARAM,
            "error: fm query param error.",
        },
        {
            ERR_APPEXECFWK_FORM_PERMISSION_DENY,
            "error: fm query permission denied.",
        },
        {
            ERR_APPEXECFWK_FORM_NOT_EXIST_ID,
            "warning: fm query no form info.",
        },
        {
            ERR_APPEXECFWK_FORM_COMMON_CODE,
            "error: unknown.",
        },
    };

    return OHOS::ERR_OK;
}
/**
 * @brief Run help command.
 */ 
ErrCode FormMgrShellCommand::RunAsHelpCommand()
{
    resultReceiver_.append(FM_HELP_MSG);

    return OHOS::ERR_OK;
}
/**
 * @brief Run query form info command.
 */
ErrCode FormMgrShellCommand::RunAsQueryCommand()
{
    int32_t result = OHOS::ERR_OK;
    int32_t cmdFlag = COMMAND_QUERY_INVALID;
    int option = -1;
    int counter = 0;    
    std::string bundleName = "";
    int64_t formId = 0;

    while (true) {
        counter++;
        option = getopt_long(argc_, argv_, SHORT_OPTIONS.c_str(), LONG_OPTIONS, nullptr);
        APP_LOGI("option: %{public}d, optopt: %{public}d, optind: %{public}d", option, optopt, optind);
        if (optind < 0 || optind > argc_) {
            return OHOS::ERR_INVALID_VALUE;
        }

        for (int i = 0; i < argc_; i++) {
            APP_LOGI("argv_[%{public}d]: %{public}s", i, argv_[i]);
        }

        if (option == -1) {
            if (counter == 1) {
                // When scanning the first argument
                if (strcmp(argv_[optind], cmd_.c_str()) == 0) {
                    // 'fm query' with no option: fm query
                    // 'fm query' with a wrong argument: fm query xxx
                    APP_LOGE("'fm query' with no option.");
                    resultReceiver_.append(HELP_MSG_NO_OPTION + "\n");
                    result = OHOS::ERR_INVALID_VALUE;
                }
            }
            break;
        }
        
        if (option == '?') {
            result = HandleUnknownOption(optopt);
            break;
        }

        result = HandleNormalOption(option, bundleName, formId, cmdFlag);
        // if(result == OHOS::ERR_OK) {
        //     break;
        // }
    }

    if (result == OHOS::ERR_OK) {
        result = ExecuteQuery(bundleName, formId, cmdFlag);        
    } else {
        resultReceiver_.append(HELP_MSG_QUERY);
    }

    return result;
}
/**
 * @brief Handle command args.
 * @param optopt Command optopt.
 * @return Returns ERR_OK on success, others on failure.
 */
int32_t FormMgrShellCommand::HandleUnknownOption(const char optopt)
{   
    int32_t result = OHOS::ERR_OK;
    switch (optopt) {
        case 'n':
        case 'i': {
            // 'fm query -n' with no argument: fm query -n
            // 'fm query --bundle-name' with no argument: fm query --bundle-name
            // 'fm query -i' with no argument: fm query -i
            // 'fm query --form-id' with no argument: fm query --form-id
            APP_LOGE("'fm query' %{public}s with no argument.", argv_[optind - 1]);
            resultReceiver_.append("error: option '");
            resultReceiver_.append(argv_[optind - 1]);
            resultReceiver_.append("' requires a value.\n");
            result = OHOS::ERR_INVALID_VALUE;
            break;
        }
        case 0: {
            // 'fm query' with a unknown option: fm query --x
            // 'fm query' with a unknown option: fm query --xxx
            std::string unknownOption = "";
            std::string unknownOptionMsg = GetUnknownOptionMsg(unknownOption);
            APP_LOGE("'fm query' with a unknown option: %{public}s", unknownOption.c_str());
            resultReceiver_.append(unknownOptionMsg);
            result = OHOS::ERR_INVALID_VALUE;
            break;
        }
        default: {
            // 'fm query' with a unknown option: fm query -x
            // 'fm query' with a unknown option: fm query -xxx
            std::string unknownOption = "";
            std::string unknownOptionMsg = GetUnknownOptionMsg(unknownOption);
            APP_LOGE("'fm query' with a unknown option: %{public}s", unknownOption.c_str());
            resultReceiver_.append(unknownOptionMsg);
            result = OHOS::ERR_INVALID_VALUE;
            break;
        }
    }

    return result;
}
/**
 * @brief Handle command args.
 * @param option Command option.
 * @param bundleName BundleName.
 * @param formId The id of the form.
 * @param cmdFlag Command Flag.
 * @return Returns ERR_OK on success, others on failure.
 */
int32_t FormMgrShellCommand::HandleNormalOption(const int option, std::string &bundleName, int64_t &formId, int32_t &cmdFlag)
{
    APP_LOGI("%{public}s start, option: %{public}d", __func__, option);
    int32_t result = OHOS::ERR_OK;
    switch (option) {
        case 'h': {
            // 'fm query -h'
            // 'fm query --help'
            APP_LOGI("'fm query' %{public}s", argv_[optind - 1]);
            result = OHOS::ERR_INVALID_VALUE;
            break;
        }
        case 's': {
            // 'fm query -s'
            // 'fm query --storage'
            cmdFlag = COMMAND_QUERY_STORAGE;
            break;
        }
        case 'n': {
            // 'fm query -n <bundle-name>'
            // 'fm query --bundle-name <bundle-name>'
            if(optarg == nullptr) {
                resultReceiver_.append("error: option ");
                resultReceiver_.append("'-n'");
                resultReceiver_.append(" requires a value.\n");
                result = OHOS::ERR_INVALID_VALUE;
                break;
            }
            cmdFlag = COMMAND_QUERY_NAME;
            bundleName = optarg;
            break;
        }
        case 'i': {
            // 'fm query -i <form-id> '
            // 'fm query --form-id <form-id> '
            if(optarg == nullptr) {
                resultReceiver_.append("error: option ");
                resultReceiver_.append("'-i'");
                resultReceiver_.append(" requires a value.\n");
                result = OHOS::ERR_INVALID_VALUE;
                break;
            }
            cmdFlag = COMMAND_QUERY_ID;
            formId = std::stoll(optarg);
            break;
        }
        default: {
            result = OHOS::ERR_INVALID_VALUE;
            APP_LOGI("'fm query' invalid option.");
            break;
        }
    }
    APP_LOGI("%{public}s end, result: %{public}d", __func__, result);
    return result;
}
/**
 * @brief Execute query form info command.
 * @param bundleName BundleName.
 * @param formId The id of the form.
 * @param cmdFlag Command Flag.
 * @return Returns ERR_OK on success, others on failure.
 */
int32_t FormMgrShellCommand::ExecuteQuery(const std::string &bundleName, const int64_t formId, const int32_t cmdFlag)
{
    APP_LOGI("%{public}s start, bundleName: %{public}s, formId:%{public}" PRId64 "", __func__, bundleName.c_str(), formId);
    int32_t result = OHOS::ERR_OK;
    switch (cmdFlag) {
        case COMMAND_QUERY_STORAGE: {
            result = QueryStorageFormInfos();
            break;
        }
        case COMMAND_QUERY_NAME: {
            result = QueryFormInfoByBundleName(bundleName);
            break;
        }
        case COMMAND_QUERY_ID: {
            result = QueryFormInfoByFormId(formId);
            break;
        }
        default: {
            APP_LOGI("'fm query' invalid command.");
            break;
        }
        APP_LOGI("%{public}s end, cmdFlag: %{public}d", __func__, cmdFlag);
    }

    if (result == OHOS::ERR_OK) {
        resultReceiver_ = STRING_QUERY_FORM_INFO_OK + "\n" + resultReceiver_;
    } else if(result == OHOS::ERR_APPEXECFWK_FORM_NOT_EXIST_ID){
        resultReceiver_ = STRING_QUERY_FORM_INFO_OK + "\n";
        resultReceiver_.append(GetMessageFromCode(result));
    } else {
        resultReceiver_ = STRING_QUERY_FORM_INFO_NG + "\n";
        resultReceiver_.append(GetMessageFromCode(result));
    }

    return result;
}
/**
 * @brief Query all of form storage infos.
 * @return Returns ERR_OK on success, others on failure.
 */
int32_t FormMgrShellCommand::QueryStorageFormInfos()
{
    APP_LOGI("%{public}s start", __func__);
    
    int errCode = ConnectFms();
    if(errCode != OHOS::ERR_OK) {
        return errCode;
    }

    std::string formInfos;
    MessageParcel data;
    if (!WriteInterfaceToken(data)) {
        APP_LOGE("%{public}s, failed to write interface token", __func__);
        return ERR_APPEXECFWK_PARCEL_ERROR;
    }

    int result = GetStringInfo(IFormMgr::Message::FORM_MGR_STORAGE_FORM_INFOS, data, formInfos);
    if (result == ERR_OK) {
        resultReceiver_= formInfos;
    } else {
        APP_LOGE("'fm query' failed to query form info.");
    }
    APP_LOGI("%{public}s end, formInfo: %{public}s", __func__, resultReceiver_.c_str());

    return result;
}
/**
 * @brief Query form infos by bundleName.
 * @param bundleName BundleName.
 * @return Returns ERR_OK on success, others on failure.
 */
int32_t FormMgrShellCommand::QueryFormInfoByBundleName(const std::string& bundleName)
{
    APP_LOGI("%{public}s start, bundleName: %{public}s", __func__, bundleName.c_str());

    int errCode = ConnectFms();
    if(errCode != OHOS::ERR_OK) {
        return errCode;
    }

    std::string formInfos;
    MessageParcel data;
    if (!WriteInterfaceToken(data)) {
        APP_LOGE("%{public}s, failed to write interface token", __func__);
        return ERR_APPEXECFWK_PARCEL_ERROR;
    }

    if (!data.WriteString(bundleName)) {
        APP_LOGE("%{public}s, failed to write bundleName", __func__);
        return ERR_APPEXECFWK_PARCEL_ERROR;
    }

    int result = GetStringInfo(IFormMgr::Message::FORM_MGR_FORM_INFOS_BY_NAME, data, formInfos);
    if (result == ERR_OK) {
        APP_LOGI("%{public}s, DumpFormInfoByBundleName success", __func__);
        resultReceiver_ = formInfos;
    } else {
        APP_LOGE("'fm query' failed to query form info.");
    }
    APP_LOGI("%{public}s end, formInfo: %{public}s", __func__, resultReceiver_.c_str());

    return result;
}
/**
 * @brief Query form infos by form id.
 * @param formId The id of the form.
 * @return Returns ERR_OK on success, others on failure.
 */
int32_t FormMgrShellCommand::QueryFormInfoByFormId(const std::int64_t formId)
{
    APP_LOGI("%{public}s start, formId: %{public}" PRId64 "", __func__, formId);

    int errCode = ConnectFms();
    if(errCode != OHOS::ERR_OK) {
        return errCode;
    }

    std::string formInfo;
    MessageParcel data;
    if (!WriteInterfaceToken(data)) {
        APP_LOGE("%{public}s, failed to write interface token", __func__);
        return ERR_APPEXECFWK_PARCEL_ERROR;
    }
    if (!data.WriteInt64(formId)) {
        APP_LOGE("%{public}s, failed to write formId", __func__);
        return ERR_APPEXECFWK_PARCEL_ERROR;
    }

    int result = GetStringInfo(IFormMgr::Message::FORM_MGR_FORM_INFOS_BY_ID, data, formInfo);
    if (result == ERR_OK) {
        resultReceiver_ = formInfo;
    }  else if(result == OHOS::ERR_APPEXECFWK_FORM_NOT_EXIST_ID) {
        APP_LOGW("'fm query' no form info.");
    } else {
        APP_LOGE("'fm query' failed to query form info.");
    }
    APP_LOGI("%{public}s end, formInfo: %{public}s", __func__, resultReceiver_.c_str());

    return result;
}
bool FormMgrShellCommand::WriteInterfaceToken(MessageParcel &data)
{
    if (!data.WriteInterfaceToken(IFormMgr::GetDescriptor())) {
        APP_LOGE("%{public}s, failed to write interface token", __func__);
        return false;
    }
    return true;
}
int FormMgrShellCommand::GetStringInfo(IFormMgr::Message code, MessageParcel &data, std::string &stringInfo)
{
    int error;
    MessageParcel reply;
    error = SendTransactCmd(code, data, reply);
    if (error != ERR_OK) {
        return error;
    }

    error = reply.ReadInt32();
    if (error != ERR_OK) {
        APP_LOGE("%{public}s, failed to read reply result: %{public}d", __func__, error);
        return error;
    }
    std::vector<std::string> stringInfoList;
    if (!reply.ReadStringVector(&stringInfoList)) {
        APP_LOGE("%{public}s, failed to read string vector from reply", __func__);
        return false;
    }
    if (stringInfoList.empty()) {
        APP_LOGI("%{public}s, No string info", __func__);
        return ERR_APPEXECFWK_FORM_NOT_EXIST_ID;
    }
    for (auto &info : stringInfoList) {
        stringInfo += info;
    }
    APP_LOGD("%{public}s, get string info success", __func__);
    return ERR_OK;
}
int FormMgrShellCommand::SendTransactCmd(IFormMgr::Message code, MessageParcel &data, MessageParcel &reply)
{
    MessageOption option(MessageOption::TF_SYNC);

    if (!remoteObject_) {
        APP_LOGE("%{public}s, failed to get remote object, cmd: %{public}d", __func__, code);
        return ERR_APPEXECFWK_SERVICE_NOT_CONNECTED;
    }
    int32_t result = remoteObject_->SendRequest(static_cast<uint32_t>(code), data, reply, option);
    if (result != ERR_OK) {
        APP_LOGE("%{public}s, failed to SendRequest: %{public}d, cmd: %{public}d", __func__, result, code);
        return result; 
    }
    return ERR_OK;
}
/**
 * @brief Connect form manager service.
 * @return Returns ERR_OK on success, others on failure.
 */
int32_t FormMgrShellCommand::ConnectFms()
{
    if(remoteObject_ != nullptr) {
        return OHOS::ERR_OK;
    }
    sptr<ISystemAbilityManager> systemManager = SystemAbilityManagerClient::GetInstance().GetSystemAbilityManager();
    if (systemManager == nullptr) {
        APP_LOGE("%{private}s:fail to get system ability manager", __func__);
        return ERR_APPEXECFWK_FORM_GET_FMS_FAILED;
    }
    remoteObject_ = systemManager->GetSystemAbility(FORM_MGR_SERVICE_ID);
    if (remoteObject_ == nullptr) {
        APP_LOGE("%{private}s:fail to connect FormMgrService", __func__);
        return ERR_APPEXECFWK_FORM_GET_FMS_FAILED;
    }

    APP_LOGI("%{public}s end, get fms proxy success", __func__);
    return OHOS::ERR_OK;
}
}  // namespace AppExecFwk
}  // namespace OHOS