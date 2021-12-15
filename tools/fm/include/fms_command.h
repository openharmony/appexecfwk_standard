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

#ifndef FOUNDATION_APPEXECFWK_STANDARD_TOOLS_FM_INCLUDE_FMS_COMMAND_H
#define FOUNDATION_APPEXECFWK_STANDARD_TOOLS_FM_INCLUDE_FMS_COMMAND_H

#include "shell_command.h"
#include "form_mgr_interface.h"
#include "iremote_proxy.h"

namespace OHOS {
namespace AppExecFwk {
namespace {
const std::string FM_TOOL_NAME = "fm";

const std::string FM_HELP_MSG = "usage: fm <command> <options>\n"
                             "These are common fm commands list:\n"
                             "  help         list available commands\n"
                             "  query        query form info with options\n";

const std::string HELP_MSG_QUERY =
    "usage: fm query <options>\n"
    "options list:\n"
    "  -h, --help                               list available commands\n"
    "  -s, --storage                            query form storage info\n"
    "  -n  <bundle-name>                        query form info by a bundle name\n"
    "  -i  <form-id>                            query form info by a form id\n";


const std::string HELP_MSG_NO_BUNDLE_PATH_OPTION =
    "error: you must specify a form id with '-1' or '--formid'.";

const std::string HELP_MSG_NO_BUNDLE_NAME_OPTION =
    "error: you must specify a bundle name with '-n' or '--name'.";

const std::string STRING_QUERY_FORM_INFO_OK = "query form info successfully.";
const std::string STRING_QUERY_FORM_INFO_NG = "error: failed to query form info.";
}  // namespace

class FormMgrShellCommand : public OHOS::AAFwk::ShellCommand {
public:
    FormMgrShellCommand(int argc, char *argv[]);
    ~FormMgrShellCommand() override
    {}

private:
    /**
     * @brief Create command map.
     */
    ErrCode CreateCommandMap() override;
    /**
     * @brief Create message map.
     */    
    ErrCode CreateMessageMap() override;
    /**
     * @brief init.
     */
    ErrCode init() override;
    /**
     * @brief Run help command.
     */
    ErrCode RunAsHelpCommand();
    /**
     * @brief Run query form info command.
     */
    ErrCode RunAsQueryCommand();
    // ErrCode RunAsDumpCommand();

    /**
     * @brief Query all of form storage infos.
     * @return Returns ERR_OK on success, others on failure.
     */
    int32_t QueryStorageFormInfos();
    /**
     * @brief Query form infos by bundleName.
     * @param bundleName BundleName.
     * @return Returns ERR_OK on success, others on failure.
     */
    int32_t QueryFormInfoByBundleName(const std::string& bundleName);
    /**
     * @brief Query form infos by form id.
     * @param formId The id of the form.
     * @return Returns ERR_OK on success, others on failure.
     */
    int32_t QueryFormInfoByFormId(const std::int64_t formId);
    /**
     * @brief Handle command args.
     * @param optopt Command optopt.
     * @return Returns ERR_OK on success, others on failure.
     */
    int32_t HandleUnknownOption(const char optopt);
    /**
     * @brief Handle command args.
     * @param option Command option.
     * @param bundleName BundleName.
     * @param formId The id of the form.
     * @param cmdFlag Command Flag.
     * @return Returns ERR_OK on success, others on failure.
     */
    int32_t HandleNormalOption(const int option, std::string &bundleName, int64_t &formId, int32_t &cmdFlag);

    /**
     * @brief Execute query form info command.
     * @param bundleName BundleName.
     * @param formId The id of the form.
     * @param cmdFlag Command Flag.
     * @return Returns ERR_OK on success, others on failure.
     */
    int32_t ExecuteQuery(const std::string &bundleName, const int64_t formId, const int32_t cmdFlag);

    /**
     * @brief Connect form manager service.
     * @return Returns ERR_OK on success, others on failure.
     */
    int32_t ConnectFms();

    bool WriteInterfaceToken(MessageParcel &data);
    int GetStringInfo(IFormMgr::Message code, MessageParcel &data, std::string &stringInfo);
    int SendTransactCmd(IFormMgr::Message code, MessageParcel &data, MessageParcel &reply);
private:
    sptr<IRemoteObject> remoteObject_;
};
}  // namespace AppExecFwk
}  // namespace OHOS

#endif  // FOUNDATION_APPEXECFWK_STANDARD_TOOLS_FM_INCLUDE_FMS_COMMAND_H