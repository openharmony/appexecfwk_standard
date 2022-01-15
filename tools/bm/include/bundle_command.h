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

#ifndef FOUNDATION_APPEXECFWK_STANDARD_TOOLS_BM_INCLUDE_BUNDLE_COMMAND_H
#define FOUNDATION_APPEXECFWK_STANDARD_TOOLS_BM_INCLUDE_BUNDLE_COMMAND_H

#include "shell_command.h"
#include "bundle_mgr_interface.h"
#include "bundle_installer_interface.h"

namespace OHOS {
namespace AppExecFwk {
namespace {
const std::string TOOL_NAME = "bm";

const std::string HELP_MSG = "usage: bm <command> <options>\n"
                             "These are common bm commands list:\n"
                             "  help         list available commands\n"
                             "  install      install a bundle with options\n"
                             "  uninstall    uninstall a bundle with options\n"
                             "  dump         dump the bundle info\n"
                             "  clean        clean the bundle data\n"
                             "  enable       enable the bundle\n"
                             "  disable      disable the bundle\n"
                             "  recover      recover the bundle\n"
                             "  query        query extensionInfo or profile\n";

const std::string HELP_MSG_INSTALL =
    "usage: bm install <options>\n"
    "options list:\n"
    "  -h, --help                                                 list available commands\n"
    "  -p, --bundle-path <hap-file-path>                          install a hap by a specified path\n"
    "  -p, --bundle-path <hap-file-path> <hap-file-path> ...      install some haps of one bundle by some hap paths\n"
    "  -p, --bundle-path <bundle-direction>                       install one bundle by a direction,\n"
    "                                                                     under which are some hap files\n"
    "  -r -p <bundle-file-path>                                   replace an existing bundle\n"
    "  -r --bundle-path <bundle-file-path>                        replace an existing bundle\n";

const std::string HELP_MSG_UNINSTALL =
    "usage: bm uninstall <options>\n"
    "options list:\n"
    "  -h, --help                           list available commands\n"
    "  -n, --bundle-name <bundle-name>      uninstall a bundle by bundle name\n"
    "  -m, --module-name <module-name>      uninstall a module by module name\n";

const std::string HELP_MSG_DUMP =
    "usage: bm dump <options>\n"
    "options list:\n"
    "  -h, --help                           list available commands\n"
    "  -a, --all                            list all bundles in system\n"
    "  -i, --bundle-info                    list all bundles info in system\n"
    "  -n, --bundle-name <bundle-name>      list the bundle info by a bundle name\n"
    "  -s, --shortcut-info                  list the shortcut info\n";

const std::string HELP_MSG_CLEAN =
    "usage: bm clean <options>\n"
    "options list:\n"
    "  -h, --help                                      list available commands\n"
    "  -n, --bundle-name  <bundle-name>                bundle name\n"
    "  -c, --cache                                     clean bundle cache files by bundle name\n"
    "  -d, --data                                      clean bundle data files by bundle name\n";

const std::string HELP_MSG_ENABLE =
    "usage: bm enable <options>\n"
    "options list:\n"
    "  -h, --help                             list available commands\n"
    "  -n, --bundle-name  <bundle-name>       enable bundle by bundle name\n"
    "  -a, --ability-name <ability-name>      enable ability by ability name\n";

const std::string HELP_MSG_DISABLE =
    "usage: bm disable <options>\n"
    "options list:\n"
    "  -h, --help                             list available commands\n"
    "  -n, --bundle-name  <bundle-name>       disable bundle by bundle name\n"
    "  -a, --ability-name <ability-name>      disable ability by ability name\n";

const std::string HELP_MSG_RECOVER =
    "usage: bm recover <options>\n"
    "options list:\n"
    "  -h, --help                             list available commands\n"
    "  -n, --bundle-name  <bundle-name>       recover bundle by bundle name\n";

const std::string HELP_MSG_QUERY =
    "usage: bm query <options>\n"
    "options list:\n"
    "  -h, --help                             list available commands\n"
    "  -n, --bundle-name <bundle-name>        query bundle by bundle name\n"
    "  -e, --element-name <element-name>      query info by element name\n"
    "  -m, --metadata-name <metadata-name>    query metadata by metadata name\n";

const std::string HELP_MSG_NO_BUNDLE_PATH_OPTION =
    "error: you must specify a bundle path with '-p' or '--bundle-path'.";

const std::string HELP_MSG_NO_BUNDLE_NAME_OPTION =
    "error: you must specify a bundle name with '-n' or '--bundle-name'.";

const std::string STRING_INSTALL_BUNDLE_OK = "install bundle successfully.";
const std::string STRING_INSTALL_BUNDLE_NG = "error: failed to install bundle.";

const std::string STRING_UNINSTALL_BUNDLE_OK = "uninstall bundle successfully.";
const std::string STRING_UNINSTALL_BUNDLE_NG = "error: failed to uninstall bundle.";

const std::string HELP_MSG_NO_DATA_OR_CACHE_OPTION =
    "error: you must specify '-c' or '-d' for 'bm clean' option.";
const std::string STRING_CLEAN_CACHE_BUNDLE_OK = "clean bundle cache files successfully.";
const std::string STRING_CLEAN_CACHE_BUNDLE_NG = "error: failed to clean bundle cache files.";

const std::string STRING_CLEAN_DATA_BUNDLE_OK = "clean bundle data files successfully.";
const std::string STRING_CLEAN_DATA_BUNDLE_NG = "error: failed to clean bundle data files.";

const std::string STRING_ENABLE_BUNDLE_OK = "enable bundle successfully.";
const std::string STRING_ENABLE_BUNDLE_NG = "error: failed to enable bundle.";

const std::string STRING_DISABLE_BUNDLE_OK = "disable bundle successfully.";
const std::string STRING_DISABLE_BUNDLE_NG = "error: failed to disable bundle.";

const std::string STRING_RECOVER_BUNDLE_OK = "recover bundle successfully.";
const std::string STRING_RECOVER_BUNDLE_NG = "error: failed to recover bundle.";

const std::string STRING_QUERY_BUNDLE_OK = "query bundle successfully.";
const std::string STRING_QUERY_BUNDLE_NG = "error: failed to query profile.";
const std::string STRING_QUERY_NEED_CORRECT_ARGUMENTS = "error: need correct arguments!";
const std::string NO_PROFILE = "no profile!\n";
}  // namespace

class BundleManagerShellCommand : public OHOS::AAFwk::ShellCommand {
public:
    BundleManagerShellCommand(int argc, char *argv[]);
    ~BundleManagerShellCommand() override
    {}

private:
    ErrCode CreateCommandMap() override;
    ErrCode CreateMessageMap() override;
    ErrCode init() override;

    sptr<IBundleMgr> GetBundleMgrProxy() const;
    sptr<IBundleInstaller> GetInstallerProxy() const;

    ErrCode RunAsHelpCommand();
    ErrCode RunAsInstallCommand();
    ErrCode RunAsUninstallCommand();
    ErrCode RunAsDumpCommand();
    ErrCode RunAsCleanCommand();
    ErrCode RunAsEnableCommand();
    ErrCode RunAsDisableCommand();
    ErrCode RunAsRecoverCommand();
    ErrCode RunAsQueryCommand();

    std::string DumpBundleList(int32_t userId) const;
    std::string DumpBundleInfo(const std::string &bundleName, int32_t userId) const;
    std::string DumpBundleInfos(int32_t userId) const;
    std::string DumpShortcutInfos(const std::string &bundleName, int32_t userId) const;

    int32_t InstallOperation(const std::vector<std::string> &bundlePaths, InstallParam &installParam) const;
    int32_t UninstallOperation(const std::string &bundleName, const std::string &moduleName,
                               InstallParam &installParam) const;

    ErrCode GetBundlePath(const std::string& param, std::vector<std::string>& bundlePaths) const;

    bool CleanBundleCacheFilesOperation(const std::string &bundleName) const;
    bool CleanBundleDataFilesOperation(const std::string &bundleName) const;

    bool SetApplicationEnabledOperation(const AbilityInfo &abilityInfo, bool isEnable) const;
    int32_t RecoverOperation(const std::string &bundleName, InstallParam &installParam) const;

    bool QueryOperation(const std::string &bundleName, const std::string &elementName,
        const std::string &metadataName, std::string &result) const;

    bool CheckArguments(const std::string &argument) const;

    sptr<IBundleMgr> bundleMgrProxy_;
    sptr<IBundleInstaller> bundleInstallerProxy_;
};
}  // namespace AppExecFwk
}  // namespace OHOS

#endif  // FOUNDATION_APPEXECFWK_STANDARD_TOOLS_BM_INCLUDE_BUNDLE_COMMAND_H