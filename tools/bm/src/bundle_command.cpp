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
#include "bundle_command.h"

#include <chrono>
#include <cstdlib>
#include <cstring>
#include <future>
#include <getopt.h>
#include <unistd.h>
#include <vector>

#include "app_log_wrapper.h"
#include "bundle_death_recipient.h"
#include "bundle_mgr_client.h"
#include "clean_cache_callback_host.h"
#include "if_system_ability_manager.h"
#include "iservice_registry.h"
#include "os_account_info.h"
#include "os_account_manager.h"
#include "parameter.h"
#include "status_receiver_impl.h"
#include "system_ability_definition.h"

namespace OHOS {
namespace AppExecFwk {
namespace {
const std::string BUNDLE_NAME_EMPTY = "";
const int32_t INDEX_OFFSET = 2;
const int32_t MAX_WAITING_TIME = 3000;
const int32_t DEVICE_UDID_LENGTH = 65;
const int32_t MAX_ARGUEMENTS_NUMBER = 3;
const std::string SHORT_OPTIONS = "hp:rfn:m:a:cdu:";
const struct option LONG_OPTIONS[] = {
    {"help", no_argument, nullptr, 'h'},
    {"bundle-path", required_argument, nullptr, 'p'},
    {"replace", no_argument, nullptr, 'r'},
    {"force", no_argument, nullptr, 'f'},
    {"bundle-name", required_argument, nullptr, 'n'},
    {"module-name", required_argument, nullptr, 'm'},
    {"ability-name", required_argument, nullptr, 'a'},
    {"bundle-info", no_argument, nullptr, 'i'},
    {"cache", no_argument, nullptr, 'c'},
    {"data", no_argument, nullptr, 'd'},
    {"user-id", required_argument, nullptr, 'u'},
    {nullptr, 0, nullptr, 0},
};

const std::string SHORT_OPTIONS_DUMP = "hn:aisu:d:";
const struct option LONG_OPTIONS_DUMP[] = {
    {"help", no_argument, nullptr, 'h'},
    {"bundle-name", required_argument, nullptr, 'n'},
    {"all", no_argument, nullptr, 'a'},
    {"bundle-info", no_argument, nullptr, 'i'},
    {"shortcut-info", no_argument, nullptr, 's'},
    {"user-id", required_argument, nullptr, 'u'},
    {"device-id", required_argument, nullptr, 'd'},
    {nullptr, 0, nullptr, 0},
};

const std::string SHORT_OPTIONS_QUERY = "hn:e:m:u:";
const struct option LONG_OPTIONS_QUERY[] = {
    {"help", no_argument, nullptr, 'h'},
    {"bundle-name", required_argument, nullptr, 'n'},
    {"element-name", required_argument, nullptr, 'e'},
    {"metadata-name", required_argument, nullptr, 'm'},
    {"user-id", required_argument, nullptr, 'u'},
    {nullptr, 0, nullptr, 0},
};

const std::string SHORT_OPTIONS_GET = "hu";
const struct option LONG_OPTIONS_GET[] = {
    {"help", no_argument, nullptr, 'h'},
    {"udid", no_argument, nullptr, 'u'},
    {nullptr, 0, nullptr, 0},
};
}  // namespace

class CleanCacheCallbackImpl : public CleanCacheCallbackHost {
public:
    CleanCacheCallbackImpl()
    {
        signal_ = std::make_shared<std::promise<bool>>();
    }
    virtual ~CleanCacheCallbackImpl() override
    {}
    virtual void OnCleanCacheFinished(bool error) override;
    bool GetResultCode();
private:
    std::shared_ptr<std::promise<bool>> signal_;
    DISALLOW_COPY_AND_MOVE(CleanCacheCallbackImpl);
};

void CleanCacheCallbackImpl::OnCleanCacheFinished(bool error)
{
    signal_->set_value(!error);
}

bool CleanCacheCallbackImpl::GetResultCode()
{
    auto future = signal_->get_future();
    std::chrono::milliseconds span(MAX_WAITING_TIME);
    if (future.wait_for(span) == std::future_status::timeout) {
        return false;
    }
    return future.get();
}

BundleManagerShellCommand::BundleManagerShellCommand(int argc, char *argv[]) : ShellCommand(argc, argv, TOOL_NAME)
{}

ErrCode BundleManagerShellCommand::CreateCommandMap()
{
    commandMap_ = {
        {"help", std::bind(&BundleManagerShellCommand::RunAsHelpCommand, this)},
        {"install", std::bind(&BundleManagerShellCommand::RunAsInstallCommand, this)},
        {"uninstall", std::bind(&BundleManagerShellCommand::RunAsUninstallCommand, this)},
        {"dump", std::bind(&BundleManagerShellCommand::RunAsDumpCommand, this)},
        {"clean", std::bind(&BundleManagerShellCommand::RunAsCleanCommand, this)},
        {"enable", std::bind(&BundleManagerShellCommand::RunAsEnableCommand, this)},
        {"disable", std::bind(&BundleManagerShellCommand::RunAsDisableCommand, this)},
        {"query", std::bind(&BundleManagerShellCommand::RunAsQueryCommand, this)},
        {"get", std::bind(&BundleManagerShellCommand::RunAsGetCommand, this)},
    };

    return OHOS::ERR_OK;
}

ErrCode BundleManagerShellCommand::CreateMessageMap()
{
    messageMap_ = {
        //  error + message
        {
            IStatusReceiver::ERR_INSTALL_INTERNAL_ERROR,
            "error: install internal error.",
        },
        {
            IStatusReceiver::ERR_INSTALL_HOST_INSTALLER_FAILED,
            "error: install host installer failed.",
        },
        {
            IStatusReceiver::ERR_INSTALL_PARSE_FAILED,
            "error: install parse failed.",
        },
        {
            IStatusReceiver::ERR_INSTALL_VERSION_DOWNGRADE,
            "error: install version downgrade.",
        },
        {
            IStatusReceiver::ERR_INSTALL_VERIFICATION_FAILED,
            "error: install verification failed.",
        },
        {
            IStatusReceiver::ERR_INSTALL_FAILED_INVALID_SIGNATURE_FILE_PATH,
            "error: signature file path is invalid.",
        },
        {
            IStatusReceiver::ERR_INSTALL_FAILED_BAD_BUNDLE_SIGNATURE_FILE,
            "error: cannot open signature file.",
        },
        {
            IStatusReceiver::ERR_INSTALL_FAILED_NO_BUNDLE_SIGNATURE,
            "error: no signature file.",
        },
        {
            IStatusReceiver::ERR_INSTALL_FAILED_VERIFY_APP_PKCS7_FAIL,
            "error: fail to verify pkcs7 file.",
        },
        {
            IStatusReceiver::ERR_INSTALL_FAILED_PROFILE_PARSE_FAIL,
            "error: fail to parse signature file.",
        },
        {
            IStatusReceiver::ERR_INSTALL_FAILED_APP_SOURCE_NOT_TRUESTED,
            "error: signature verification failed due to not trusted app source.",
        },
        {
            IStatusReceiver::ERR_INSTALL_FAILED_BAD_DIGEST,
            "error: signature verification failed due to not bad digest.",
        },
        {
            IStatusReceiver::ERR_INSTALL_FAILED_BUNDLE_INTEGRITY_VERIFICATION_FAILURE,
            "error: signature verification failed due to out of integrity.",
        },
        {
            IStatusReceiver::ERR_INSTALL_FAILED_FILE_SIZE_TOO_LARGE,
            "error: signature verification failed due to oversize file.",
        },
        {
            IStatusReceiver::ERR_INSTALL_FAILED_BAD_PUBLICKEY,
            "error: signature verification failed due to bad public key.",
        },
        {
            IStatusReceiver::ERR_INSTALL_FAILED_BAD_BUNDLE_SIGNATURE,
            "error: signature verification failed due to bad bundle signature.",
        },
        {
            IStatusReceiver::ERR_INSTALL_FAILED_NO_PROFILE_BLOCK_FAIL,
            "error: signature verification failed due to no profile block.",
        },
        {
            IStatusReceiver::ERR_INSTALL_FAILED_BUNDLE_SIGNATURE_VERIFICATION_FAILURE,
            "error: verify signature failed.",
        },
        {
            IStatusReceiver::ERR_INSTALL_FAILED_VERIFY_SOURCE_INIT_FAIL,
            "error: signature verification failed due to init source failed.",
        },
        {
            IStatusReceiver::ERR_INSTALL_FAILED_INCOMPATIBLE_SIGNATURE,
            "error: install incompatible signature info.",
        },
        {
            IStatusReceiver::ERR_INSTALL_FAILED_INCONSISTENT_SIGNATURE,
            "error: install sign info inconsistent.",
        },
        {
            IStatusReceiver::ERR_INSTALL_PARAM_ERROR,
            "error: install param error.",
        },
        {
            IStatusReceiver::ERR_INSTALL_PERMISSION_DENIED,
            "error: install permission denied.",
        },
        {
            IStatusReceiver::ERR_INSTALL_ENTRY_ALREADY_EXIST,
            "error: install entry already exist.",
        },
        {
            IStatusReceiver::ERR_INSTALL_STATE_ERROR,
            "error: install state error.",
        },
        {
            IStatusReceiver::ERR_INSTALL_FILE_PATH_INVALID,
            "error: install file path invalid.",
        },
        {
            IStatusReceiver::ERR_INSTALL_INVALID_HAP_NAME,
            "error: install invalid hap name.",
        },
        {
            IStatusReceiver::ERR_INSTALL_INVALID_BUNDLE_FILE,
            "error: install invalid bundle file.",
        },
        {
            IStatusReceiver::ERR_INSTALL_INVALID_HAP_SIZE,
            "error: install invalid hap size.",
        },
        {
            IStatusReceiver::ERR_INSTALL_GENERATE_UID_ERROR,
            "error: install generate uid error.",
        },
        {
            IStatusReceiver::ERR_INSTALL_INSTALLD_SERVICE_ERROR,
            "error: install installd service error.",
        },
        {
            IStatusReceiver::ERR_INSTALL_BUNDLE_MGR_SERVICE_ERROR,
            "error: install bundle mgr service error.",
        },
        {
            IStatusReceiver::ERR_INSTALL_ALREADY_EXIST,
            "error: install already exist.",
        },
        {
            IStatusReceiver::ERR_INSTALL_BUNDLENAME_NOT_SAME,
            "error: install bundle name not same",
        },
        {
            IStatusReceiver::ERR_INSTALL_VERSIONCODE_NOT_SAME,
            "error: install version code not same",
        },
        {
            IStatusReceiver::ERR_INSTALL_VERSIONNAME_NOT_SAME,
            "error: install version name not same",
        },
        {
            IStatusReceiver::ERR_INSTALL_VENDOR_NOT_SAME,
            "error: install vendor not same",
        },
        {
            IStatusReceiver::ERR_INSTALL_RELEASETYPE_TARGET_NOT_SAME,
            "error: install releaseType target not same",
        },
        {
            IStatusReceiver::ERR_INSTALL_RELEASETYPE_COMPATIBLE_NOT_SAME,
            "error: install releaseType compatible not same",
        },
        {
            IStatusReceiver::ERR_INSTALL_VERSION_NOT_COMPATIBLE,
            "error: install version not compatible",
        },
        {
            IStatusReceiver::ERR_INSTALL_INVALID_NUMBER_OF_ENTRY_HAP,
            "error: install invalid number of entry hap",
        },
        {
            IStatusReceiver::ERR_INSTALL_DISK_MEM_INSUFFICIENT,
            "error: install failed due to insufficient disk memory",
        },
        {
            IStatusReceiver::ERR_INSTALL_GRANT_REQUEST_PERMISSIONS_FAILED,
            "error: install failed due to grant request permissions failed",
        },
        {
            IStatusReceiver::ERR_INSTALL_UPDATE_HAP_TOKEN_FAILED,
            "error: install failed due to update hap token failed",
        },
        {
            IStatusReceiver::ERR_INSTALL_SINGLETON_NOT_SAME,
            "error: install failed due to singleton not same",
        },
        {
            IStatusReceiver::ERR_INSTALL_ZERO_USER_WITH_NO_SINGLETON,
            "error: install failed due to zero user can only install singleton app",
        },
        {
            IStatusReceiver::ERR_INSTALL_CHECK_SYSCAP_FAILED,
            "error: install failed due to check syscap filed",
        },

        {
            IStatusReceiver::ERR_INSTALL_PARSE_UNEXPECTED,
            "error: install parse unexpected.",
        },
        {
            IStatusReceiver::ERR_INSTALL_PARSE_MISSING_BUNDLE,
            "error: install parse missing bundle.",
        },
        {
            IStatusReceiver::ERR_INSTALL_PARSE_MISSING_ABILITY,
            "error: install parse missing ability.",
        },
        {
            IStatusReceiver::ERR_INSTALL_PARSE_NO_PROFILE,
            "error: install parse no profile.",
        },
        {
            IStatusReceiver::ERR_INSTALL_PARSE_BAD_PROFILE,
            "error: install parse bad profile.",
        },
        {
            IStatusReceiver::ERR_INSTALL_PARSE_PROFILE_PROP_TYPE_ERROR,
            "error: install parse profile prop type error.",
        },
        {
            IStatusReceiver::ERR_INSTALL_PARSE_PROFILE_MISSING_PROP,
            "error: install parse profile missing prop.",
        },
        {
            IStatusReceiver::ERR_INSTALL_PARSE_PROFILE_PROP_CHECK_ERROR,
            "error: install parse profile prop check error.",
        },
        {
            IStatusReceiver::ERR_INSTALL_PARSE_PERMISSION_ERROR,
            "error: install parse permission error.",
        },
        {
            IStatusReceiver::ERR_INSTALL_PARSE_RPCID_FAILED,
            "error: install parse syscap error.",
        },
        {
            IStatusReceiver::ERR_INSTALLD_PARAM_ERROR,
            "error: installd param error.",
        },
        {
            IStatusReceiver::ERR_INSTALLD_GET_PROXY_ERROR,
            "error: installd get proxy error.",
        },
        {
            IStatusReceiver::ERR_INSTALLD_CREATE_DIR_FAILED,
            "error: installd create dir failed.",
        },
        {
            IStatusReceiver::ERR_INSTALLD_CREATE_DIR_EXIST,
            "error: installd create dir exist.",
        },
        {
            IStatusReceiver::ERR_INSTALLD_CHOWN_FAILED,
            "error: installd chown failed.",
        },
        {
            IStatusReceiver::ERR_INSTALLD_REMOVE_DIR_FAILED,
            "error: installd remove dir failed.",
        },
        {
            IStatusReceiver::ERR_INSTALLD_EXTRACT_FILES_FAILED,
            "error: installd extract files failed.",
        },
        {
            IStatusReceiver::ERR_INSTALLD_RNAME_DIR_FAILED,
            "error: installd rename dir failed.",
        },
        {
            IStatusReceiver::ERR_INSTALLD_CLEAN_DIR_FAILED,
            "error: installd clean dir failed.",
        },

        {
            IStatusReceiver::ERR_UNINSTALL_SYSTEM_APP_ERROR,
            "error: uninstall system app error.",
        },
        {
            IStatusReceiver::ERR_UNINSTALL_KILLING_APP_ERROR,
            "error: uninstall killing app error.",
        },
        {
            IStatusReceiver::ERR_UNINSTALL_INVALID_NAME,
            "error: uninstall invalid name.",
        },
        {
            IStatusReceiver::ERR_UNINSTALL_PARAM_ERROR,
            "error: uninstall param error.",
        },
        {
            IStatusReceiver::ERR_UNINSTALL_PERMISSION_DENIED,
            "error: uninstall permission denied.",
        },
        {
            IStatusReceiver::ERR_UNINSTALL_BUNDLE_MGR_SERVICE_ERROR,
            "error: uninstall bundle mgr service error.",
        },
        {
            IStatusReceiver::ERR_UNINSTALL_MISSING_INSTALLED_BUNDLE,
            "error: uninstall missing installed bundle.",
        },
        {
            IStatusReceiver::ERR_UNINSTALL_MISSING_INSTALLED_MODULE,
            "error: uninstall missing installed module.",
        },
        {
            IStatusReceiver::ERR_FAILED_SERVICE_DIED,
            "error: bundle manager service is died",
        },
        {
            IStatusReceiver::ERR_FAILED_GET_INSTALLER_PROXY,
            "error: failed to get installer proxy",
        },
        {
            IStatusReceiver::ERR_USER_NOT_EXIST,
            "error: user not exist.",
        },
        {
            IStatusReceiver::ERR_USER_NOT_INSTALL_HAP,
            "error: user does not install the hap.",
        },
        {
            IStatusReceiver::ERR_OPERATION_TIME_OUT,
            "error: operation time out.",
        },
        {
            IStatusReceiver::ERR_UNKNOWN,
            "error: unknown.",
        }
    };

    return OHOS::ERR_OK;
}

ErrCode BundleManagerShellCommand::init()
{
    ErrCode result = OHOS::ERR_OK;

    if (!bundleMgrProxy_) {
        bundleMgrProxy_ = GetBundleMgrProxy();
        if (bundleMgrProxy_) {
            if (!bundleInstallerProxy_) {
                bundleInstallerProxy_ = bundleMgrProxy_->GetBundleInstaller();
            }
        }
    }

    if (!bundleMgrProxy_ || !bundleInstallerProxy_ || !bundleInstallerProxy_->AsObject()) {
        result = OHOS::ERR_INVALID_VALUE;
    }

    return result;
}

sptr<IBundleMgr> BundleManagerShellCommand::GetBundleMgrProxy() const
{
    sptr<ISystemAbilityManager> systemAbilityManager =
        SystemAbilityManagerClient::GetInstance().GetSystemAbilityManager();
    if (!systemAbilityManager) {
        APP_LOGE("failed to get system ability mgr.");
        return nullptr;
    }

    sptr<IRemoteObject> remoteObject = systemAbilityManager->GetSystemAbility(BUNDLE_MGR_SERVICE_SYS_ABILITY_ID);
    if (!remoteObject) {
        APP_LOGE("failed to get bundle manager proxy.");
        return nullptr;
    }

    APP_LOGD("get bundle manager proxy success.");
    return iface_cast<IBundleMgr>(remoteObject);
}

sptr<IBundleInstaller> BundleManagerShellCommand::GetInstallerProxy() const
{
    sptr<IBundleMgr> bundleMgrProxy = GetBundleMgrProxy();
    if (!bundleMgrProxy) {
        APP_LOGE("bundle mgr proxy is nullptr.");
        return nullptr;
    }

    sptr<IBundleInstaller> installerProxy = bundleMgrProxy->GetBundleInstaller();
    if (!installerProxy) {
        APP_LOGE("failed to get bundle installer proxy.");
        return nullptr;
    }

    APP_LOGD("get bundle installer proxy success.");
    return installerProxy;
}

ErrCode BundleManagerShellCommand::RunAsHelpCommand()
{
    resultReceiver_.append(HELP_MSG);

    return OHOS::ERR_OK;
}

ErrCode BundleManagerShellCommand::RunAsInstallCommand()
{
    int result = OHOS::ERR_OK;
    InstallFlag installFlag = InstallFlag::NORMAL;
    bool noCheckSignature = false;
    int option = -1;
    int counter = 0;
    std::vector<std::string> bundlePath;
    int index = 0;
    int32_t userId = Constants::ALL_USERID;
    while (true) {
        counter++;
        option = getopt_long(argc_, argv_, SHORT_OPTIONS.c_str(), LONG_OPTIONS, nullptr);
        APP_LOGD("option: %{public}d, optopt: %{public}d, optind: %{public}d", option, optopt, optind);
        if (optind < 0 || optind > argc_) {
            return OHOS::ERR_INVALID_VALUE;
        }
        if (option == -1) {
            if (counter == 1) {
                // When scanning the first argument
                if (strcmp(argv_[optind], cmd_.c_str()) == 0) {
                    // 'bm install' with no option: bm install
                    // 'bm install' with a wrong argument: bm install xxx
                    APP_LOGD("'bm install' with no option.");
                    resultReceiver_.append(HELP_MSG_NO_OPTION + "\n");
                    result = OHOS::ERR_INVALID_VALUE;
                }
            }
            break;
        }

        if (option == '?') {
            switch (optopt) {
                case 'p': {
                    // 'bm install -p' with no argument: bm install -p
                    // 'bm install --bundle-path' with no argument: bm install --bundle-path
                    APP_LOGD("'bm install' with no argument.");
                    resultReceiver_.append("error: option ");
                    resultReceiver_.append("requires a value.\n");
                    result = OHOS::ERR_INVALID_VALUE;
                    break;
                }
                case 'u': {
                    // 'bm install -u' with no argument: bm install -u
                    // 'bm install --user-id' with no argument: bm install --user-id
                    APP_LOGD("'bm install -u' with no argument.");
                    resultReceiver_.append("error: option ");
                    resultReceiver_.append("requires a value.\n");
                    result = OHOS::ERR_INVALID_VALUE;
                    break;
                }
                default: {
                    // 'bm install' with an unknown option: bm install -x
                    // 'bm install' with an unknown option: bm install -xxx
                    std::string unknownOption = "";
                    std::string unknownOptionMsg = GetUnknownOptionMsg(unknownOption);
                    APP_LOGD("'bm install' with an unknown option.");
                    resultReceiver_.append(unknownOptionMsg);
                    result = OHOS::ERR_INVALID_VALUE;
                    break;
                }
            }
            break;
        }

        switch (option) {
            case 'h': {
                // 'bm install -h'
                // 'bm install --help'
                APP_LOGD("'bm install %{public}s'", argv_[optind - 1]);
                result = OHOS::ERR_INVALID_VALUE;
                break;
            }
            case 'p': {
                // 'bm install -p <bundle-file-path>'
                // 'bm install --bundle-path <bundle-file-path>'
                APP_LOGD("'bm install %{public}s'", argv_[optind - 1]);
                if (GetBundlePath(optarg, bundlePath) != OHOS::ERR_OK) {
                    APP_LOGD("'bm install' with no argument.");
                    resultReceiver_.append("error: option ");
                    resultReceiver_.append("requires a correct value.\n");
                    return OHOS::ERR_INVALID_VALUE;
                }
                index = optind;
                break;
            }
            case 'r': {
                // 'bm install -r'
                // 'bm install --replace'
                installFlag = InstallFlag::REPLACE_EXISTING;
                break;
            }
            case 'f': {
                // 'bm install -f'
                // 'bm install --force'
                noCheckSignature = true;
                break;
            }
            case 'u': {
                // 'bm install -p <bundle-file-path> -u userId'
                // 'bm install --bundle-path <bundle-file-path> --user-id userId'
                APP_LOGD("'bm install %{public}s %{public}s'", argv_[optind - OFFSET_REQUIRED_ARGUMENT], optarg);
                userId = atoi(optarg);
                break;
            }
            default: {
                result = OHOS::ERR_INVALID_VALUE;
                break;
            }
        }
    }

    for (; index < argc_ && index >= INDEX_OFFSET; ++index) {
        if (argList_[index - INDEX_OFFSET] == "-r" || argList_[index - INDEX_OFFSET] == "-f" ||
            argList_[index - INDEX_OFFSET] == "--force" || argList_[index - INDEX_OFFSET] == "--replace" ||
            argList_[index - INDEX_OFFSET] == "-p" || argList_[index - INDEX_OFFSET] == "--bundle-path" ||
            argList_[index - INDEX_OFFSET] == "-u" || argList_[index - INDEX_OFFSET] == "--user-id") {
            break;
        }
        if (GetBundlePath(argList_[index - INDEX_OFFSET], bundlePath) != OHOS::ERR_OK) {
            bundlePath.clear();
            APP_LOGD("'bm install' with error arguments.");
            resultReceiver_.append("error value for the chosen option");
            result = OHOS::ERR_INVALID_VALUE;
        }
    }

    for (auto &path : bundlePath) {
        APP_LOGD("install hap path %{public}s", path.c_str());
    }

    if (result == OHOS::ERR_OK) {
        if (resultReceiver_ == "" && bundlePath.empty()) {
            // 'bm install ...' with no bundle path option
            APP_LOGD("'bm install' with no bundle path option.");
            resultReceiver_.append(HELP_MSG_NO_BUNDLE_PATH_OPTION + "\n");
            result = OHOS::ERR_INVALID_VALUE;
        }
    }

    if (result != OHOS::ERR_OK) {
        resultReceiver_.append(HELP_MSG_INSTALL);
    } else {
        InstallParam installParam;
        installParam.installFlag = installFlag;
        installParam.noCheckSignature = noCheckSignature;
        installParam.userId = userId;
        int32_t installResult = InstallOperation(bundlePath, installParam);
        if (installResult == OHOS::ERR_OK) {
            resultReceiver_ = STRING_INSTALL_BUNDLE_OK + "\n";
        } else {
            resultReceiver_ = STRING_INSTALL_BUNDLE_NG + "\n";
            resultReceiver_.append(GetMessageFromCode(installResult));
        }
    }

    return result;
}

ErrCode BundleManagerShellCommand::GetBundlePath(const std::string& param,
    std::vector<std::string>& bundlePaths) const
{
    if (param.empty()) {
        return OHOS::ERR_INVALID_VALUE;
    }
    if (param == "-f" || param == "--force" || param == "-r" || param == "--replace" || param == "-p" ||
        param == "--bundle-path" || param == "-u" || param == "--user-id") {
        return OHOS::ERR_INVALID_VALUE;
    }
    bundlePaths.emplace_back(param);
    return OHOS::ERR_OK;
}

ErrCode BundleManagerShellCommand::RunAsUninstallCommand()
{
    int result = OHOS::ERR_OK;
    int option = -1;
    int counter = 0;
    std::string bundleName = "";
    std::string moduleName = "";
    int32_t userId = Constants::ALL_USERID;
    while (true) {
        counter++;
        option = getopt_long(argc_, argv_, SHORT_OPTIONS.c_str(), LONG_OPTIONS, nullptr);
        APP_LOGD("option: %{public}d, optopt: %{public}d, optind: %{public}d", option, optopt, optind);
        if (optind < 0 || optind > argc_) {
            return OHOS::ERR_INVALID_VALUE;
        }
        if (option == -1) {
            if (counter == 1) {
                // When scanning the first argument
                if (strcmp(argv_[optind], cmd_.c_str()) == 0) {
                    // 'bm uninstall' with no option: bm uninstall
                    // 'bm uninstall' with a wrong argument: bm uninstall xxx
                    APP_LOGD("'bm uninstall' %{public}s", HELP_MSG_NO_OPTION.c_str());
                    resultReceiver_.append(HELP_MSG_NO_OPTION + "\n");
                    result = OHOS::ERR_INVALID_VALUE;
                }
            }
            break;
        }

        if (option == '?') {
            switch (optopt) {
                case 'n': {
                    // 'bm uninstall -n' with no argument: bm uninstall -n
                    // 'bm uninstall --bundle-name' with no argument: bm uninstall --bundle-name
                    APP_LOGD("'bm uninstall -n' with no argument.");
                    resultReceiver_.append("error: option ");
                    resultReceiver_.append("requires a value.\n");
                    result = OHOS::ERR_INVALID_VALUE;
                    break;
                }
                case 'm': {
                    // 'bm uninstall -m' with no argument: bm uninstall -m
                    // 'bm uninstall --module-name' with no argument: bm uninstall --module-name
                    APP_LOGD("'bm uninstall -m' with no argument.");
                    resultReceiver_.append("error: option ");
                    resultReceiver_.append("requires a value.\n");
                    result = OHOS::ERR_INVALID_VALUE;
                    break;
                }
                case 'u': {
                    // 'bm uninstall -n <bundleName> -u userId'
                    // 'bm uninstall --bundle-name <bundleName> --user-id userId'
                    APP_LOGD("'bm uninstall -u' with no argument.");
                    resultReceiver_.append("error: option ");
                    resultReceiver_.append("requires a value.\n");
                    result = OHOS::ERR_INVALID_VALUE;
                    break;
                }
                default: {
                    // 'bm uninstall' with an unknown option: bm uninstall -x
                    // 'bm uninstall' with an unknown option: bm uninstall -xxx
                    std::string unknownOption = "";
                    std::string unknownOptionMsg = GetUnknownOptionMsg(unknownOption);
                    APP_LOGD("'bm uninstall' with an unknown option.");
                    resultReceiver_.append(unknownOptionMsg);
                    result = OHOS::ERR_INVALID_VALUE;
                    break;
                }
            }
            break;
        }

        switch (option) {
            case 'h': {
                // 'bm uninstall -h'
                // 'bm uninstall --help'
                APP_LOGD("'bm uninstall %{public}s'", argv_[optind - 1]);
                result = OHOS::ERR_INVALID_VALUE;
                break;
            }
            case 'n': {
                // 'bm uninstall -n xxx'
                // 'bm uninstall --bundle-name xxx'
                APP_LOGD("'bm uninstall %{public}s %{public}s'", argv_[optind - OFFSET_REQUIRED_ARGUMENT], optarg);
                bundleName = optarg;
                break;
            }
            case 'm': {
                // 'bm uninstall -m xxx'
                // 'bm uninstall --module-name xxx'
                APP_LOGD("'bm uninstall %{public}s %{public}s'", argv_[optind - OFFSET_REQUIRED_ARGUMENT], optarg);
                moduleName = optarg;
                break;
            }
            case 'u': {
                // 'bm uninstall -n <bundleName> -u userId'
                // 'bm uninstall --bundle-name <bundleName> --user-id userId'
                APP_LOGD("'bm uninstall %{public}s %{public}s'", argv_[optind - OFFSET_REQUIRED_ARGUMENT], optarg);
                userId = atoi(optarg);
                break;
            }
            default: {
                result = OHOS::ERR_INVALID_VALUE;
                break;
            }
        }
    }

    if (result == OHOS::ERR_OK) {
        if (resultReceiver_ == "" && bundleName.size() == 0) {
            // 'bm uninstall ...' with no bundle name option
            APP_LOGD("'bm uninstall' with bundle name option.");
            resultReceiver_.append(HELP_MSG_NO_BUNDLE_NAME_OPTION + "\n");
            result = OHOS::ERR_INVALID_VALUE;
        }
    }

    if (result != OHOS::ERR_OK) {
        resultReceiver_.append(HELP_MSG_UNINSTALL);
    } else {
        InstallParam installParam;
        installParam.userId = userId;
        int32_t uninstallResult = UninstallOperation(bundleName, moduleName, installParam);
        if (uninstallResult == OHOS::ERR_OK) {
            resultReceiver_ = STRING_UNINSTALL_BUNDLE_OK + "\n";
        } else {
            resultReceiver_ = STRING_UNINSTALL_BUNDLE_NG + "\n";
            resultReceiver_.append(GetMessageFromCode(uninstallResult));
        }
    }

    return result;
}

ErrCode BundleManagerShellCommand::RunAsDumpCommand()
{
    int result = OHOS::ERR_OK;
    int option = -1;
    int counter = 0;
    std::string dumpResults = "";
    std::string bundleName = "";
    bool bundleDumpAll = false;
    bool bundleDumpInfos = false;
    bool bundleDumpInfo = false;
    bool bundleDumpShortcut = false;
    bool bundleDumpDistributedBundleInfo = false;
    std::string deviceId = "";
    int32_t userId = Constants::ALL_USERID;
    while (true) {
        counter++;
        option = getopt_long(argc_, argv_, SHORT_OPTIONS_DUMP.c_str(), LONG_OPTIONS_DUMP, nullptr);
        APP_LOGD("option: %{public}d, optopt: %{public}d, optind: %{public}d", option, optopt, optind);
        if (optind < 0 || optind > argc_) {
            return OHOS::ERR_INVALID_VALUE;
        }
        if (option == -1) {
            if (counter == 1) {
                // When scanning the first argument
                if (strcmp(argv_[optind], cmd_.c_str()) == 0) {
                    // 'bm dump' with no option: bm dump
                    // 'bm dump' with a wrong argument: bm dump xxx
                    APP_LOGD("'bm dump' %{public}s", HELP_MSG_NO_OPTION.c_str());
                    resultReceiver_.append(HELP_MSG_NO_OPTION + "\n");
                    result = OHOS::ERR_INVALID_VALUE;
                }
            }
            break;
        }
        if (option == '?') {
            switch (optopt) {
                case 'n': {
                    // 'bm dump -n' with no argument: bm dump -n
                    // 'bm dump --bundle-name' with no argument: bm dump --bundle-name
                    APP_LOGD("'bm dump -n' with no argument.");
                    resultReceiver_.append("error: option ");
                    resultReceiver_.append("requires a value.\n");
                    result = OHOS::ERR_INVALID_VALUE;
                    break;
                }
                case 'u': {
                    // 'bm dump -u' with no argument: bm dump -u
                    // 'bm dump --user-id' with no argument: bm dump --user-id
                    APP_LOGD("'bm dump -u' with no argument.");
                    resultReceiver_.append("error: option ");
                    resultReceiver_.append("requires a value.\n");
                    result = OHOS::ERR_INVALID_VALUE;
                    break;
                }
                case 'd': {
                    // 'bm dump -d' with no argument: bm dump -d
                    // 'bm dump --device-id' with no argument: bm dump --device-id
                    APP_LOGD("'bm dump -d' with no argument.");
                    resultReceiver_.append("error: option ");
                    resultReceiver_.append("requires a value.\n");
                    result = OHOS::ERR_INVALID_VALUE;
                    break;
                }
                default: {
                    // 'bm dump' with an unknown option: bm dump -x
                    // 'bm dump' with an unknown option: bm dump -xxx
                    std::string unknownOption = "";
                    std::string unknownOptionMsg = GetUnknownOptionMsg(unknownOption);
                    APP_LOGD("'bm dump' with an unknown option.");
                    resultReceiver_.append(unknownOptionMsg);
                    result = OHOS::ERR_INVALID_VALUE;
                    break;
                }
            }
            break;
        }
        switch (option) {
            case 'h': {
                // 'bm dump -h'
                // 'bm dump --help'
                APP_LOGD("'bm dump %{public}s'", argv_[optind - 1]);
                result = OHOS::ERR_INVALID_VALUE;
                break;
            }
            case 'a': {
                // 'bm dump -a'
                // 'bm dump --all'
                APP_LOGD("'bm dump %{public}s'", argv_[optind - 1]);
                bundleDumpAll = true;
                break;
            }
            case 'i': {
                // 'bm dump -i'
                // 'bm dump --bundle-info'
                APP_LOGD("'bm dump %{public}s'", argv_[optind - 1]);
                bundleDumpInfos = true;
                break;
            }
            case 'n': {
                // 'bm dump -n xxx'
                // 'bm dump --bundle-name xxx'
                APP_LOGD("'bm dump %{public}s %{public}s'", argv_[optind - OFFSET_REQUIRED_ARGUMENT], optarg);
                bundleName = optarg;
                bundleDumpInfo = true;
                break;
            }
            case 's': {
                // 'bm dump -n xxx -s'
                // 'bm dump --bundle-name xxx --shortcut-info'
                APP_LOGD("'bm dump %{public}s %{public}s'", argv_[optind - OFFSET_REQUIRED_ARGUMENT], optarg);
                bundleDumpShortcut = true;
                break;
            }
            case 'u': {
                // 'bm dump -n <bundleName> -u userId'
                // 'bm dump --bundle-name <bundleName> --user-id userId'
                APP_LOGD("'bm dump %{public}s %{public}s'", argv_[optind - OFFSET_REQUIRED_ARGUMENT], optarg);
                userId = atoi(optarg);
                break;
            }
            case 'd': {
                // 'bm dump -n <bundleName> -d deviceId'
                // 'bm dump --bundle-name <bundleName> --device-id deviceId'
                APP_LOGD("'bm dump %{public}s %{public}s'", argv_[optind - OFFSET_REQUIRED_ARGUMENT], optarg);
                deviceId = optarg;
                bundleDumpDistributedBundleInfo = true;
                break;
            }
            default: {
                result = OHOS::ERR_INVALID_VALUE;
                break;
            }
        }
    }
    if (result == OHOS::ERR_OK) {
        if ((resultReceiver_ == "") && bundleDumpShortcut && (bundleName.size() == 0)) {
            // 'bm dump -s ...' with no bundle name option
            APP_LOGD("'bm dump -s' with no bundle name option.");
            resultReceiver_.append(HELP_MSG_NO_BUNDLE_NAME_OPTION + "\n");
            result = OHOS::ERR_INVALID_VALUE;
        }
        if ((resultReceiver_ == "") && bundleDumpDistributedBundleInfo && (bundleName.size() == 0)) {
            // 'bm dump d ...' with no bundle name option
            APP_LOGD("'bm dump -d' with no bundle name option.");
            resultReceiver_.append(HELP_MSG_NO_BUNDLE_NAME_OPTION + "\n");
            result = OHOS::ERR_INVALID_VALUE;
        }
    }
    if (result != OHOS::ERR_OK) {
        resultReceiver_.append(HELP_MSG_DUMP);
    } else {
        APP_LOGD("dumpResults: %{public}s", dumpResults.c_str());
        if (bundleDumpShortcut) {
            dumpResults = DumpShortcutInfos(bundleName, userId);
        } else if (bundleDumpDistributedBundleInfo) {
            dumpResults = DumpDistributedBundleInfo(deviceId, userId, bundleName);
        } else if (bundleDumpAll) {
            dumpResults = DumpBundleList(userId);
        } else if (bundleDumpInfos) {
            dumpResults = DumpBundleInfos(userId);
        } else if (bundleDumpInfo) {
            dumpResults = DumpBundleInfo(bundleName, userId);
        }
        if (dumpResults.empty() || (dumpResults == "")) {
            dumpResults = HELP_MSG_DUMP_FAILED + "\n";
        }
        resultReceiver_.append(dumpResults);
    }

    return result;
}

ErrCode BundleManagerShellCommand::RunAsCleanCommand()
{
    int32_t result = OHOS::ERR_OK;
    int32_t option = -1;
    int32_t counter = 0;
    int32_t userId = Constants::UNSPECIFIED_USERID;
    bool cleanCache = false;
    bool cleanData = false;
    std::string bundleName = "";
    while (true) {
        counter++;
        option = getopt_long(argc_, argv_, SHORT_OPTIONS.c_str(), LONG_OPTIONS, nullptr);
        APP_LOGD("option: %{public}d, optopt: %{public}d, optind: %{public}d", option, optopt, optind);
        if (optind < 0 || optind > argc_) {
            return OHOS::ERR_INVALID_VALUE;
        }
        if (option == -1) {
            if (counter == 1) {
                // When scanning the first argument
                if (strcmp(argv_[optind], cmd_.c_str()) == 0) {
                    // 'bm clean' with no option: bm clean
                    // 'bm clean' with a wrong argument: bm clean xxx
                    APP_LOGD("'bm clean' %{public}s", HELP_MSG_NO_OPTION.c_str());

                    resultReceiver_.append(HELP_MSG_NO_OPTION + "\n");
                    result = OHOS::ERR_INVALID_VALUE;
                }
            }
            break;
        }

        if (option == '?') {
            switch (optopt) {
                case 'n': {
                    // 'bm clean -n' with no argument: bm clean -n
                    // 'bm clean --bundle-name' with no argument: bm clean --bundle-name
                    APP_LOGD("'bm clean -n' with no argument.");
                    resultReceiver_.append("error: option ");
                    resultReceiver_.append("requires a value.\n");
                    result = OHOS::ERR_INVALID_VALUE;
                    break;
                }
                case 'u': {
                    // 'bm clean -u' with no argument: bm clean -u
                    // 'bm clean --user-id' with no argument: bm clean --user-id
                    APP_LOGD("'bm clean -u' with no argument.");
                    resultReceiver_.append("error: option ");
                    resultReceiver_.append("requires a value.\n");
                    result = OHOS::ERR_INVALID_VALUE;
                    break;
                }
                default: {
                    // 'bm clean' with an unknown option: bm clear -x
                    // 'bm clean' with an unknown option: bm clear -xxx
                    std::string unknownOption = "";
                    std::string unknownOptionMsg = GetUnknownOptionMsg(unknownOption);
                    APP_LOGD("'bm clean' with an unknown option.");
                    resultReceiver_.append(unknownOptionMsg);
                    result = OHOS::ERR_INVALID_VALUE;
                    break;
                }
            }
            break;
        }

        switch (option) {
            case 'h': {
                // 'bm clean -h'
                // 'bm clean --help'
                APP_LOGD("'bm clean %{public}s'", argv_[optind - 1]);
                result = OHOS::ERR_INVALID_VALUE;
                break;
            }
            case 'n': {
                // 'bm clean -n xxx'
                // 'bm clean --bundle-name xxx'
                APP_LOGD("'bm clean %{public}s %{public}s'", argv_[optind - OFFSET_REQUIRED_ARGUMENT], optarg);
                bundleName = optarg;
                break;
            }
            case 'c': {
                // 'bm clean -c'
                // 'bm clean --cache'
                APP_LOGD("'bm clean %{public}s'", argv_[optind - OFFSET_REQUIRED_ARGUMENT]);
                cleanCache = (cleanData == true) ? false : true;
                break;
            }
            case 'd': {
                // 'bm clean -d'
                // 'bm clean --data'
                APP_LOGD("'bm clean %{public}s '", argv_[optind - OFFSET_REQUIRED_ARGUMENT]);
                cleanData = (cleanCache == true) ? false : true;
                break;
            }
            case 'u': {
                // 'bm clean -u userId'
                // 'bm clean --user-id userId'
                APP_LOGD("'bm install %{public}s %{public}s'", argv_[optind - OFFSET_REQUIRED_ARGUMENT], optarg);
                userId = atoi(optarg);
                break;
            }
            default: {
                result = OHOS::ERR_INVALID_VALUE;
                break;
            }
        }
    }

    if (result == OHOS::ERR_OK) {
        if (resultReceiver_ == "" && bundleName.size() == 0) {
            // 'bm clean ...' with no bundle name option
            APP_LOGD("'bm clean' with no bundle name option.");
            resultReceiver_.append(HELP_MSG_NO_BUNDLE_NAME_OPTION + "\n");
            result = OHOS::ERR_INVALID_VALUE;
        }
        if (!cleanCache && !cleanData) {
            APP_LOGD("'bm clean' with no '-c' or '-d' option.");
            resultReceiver_.append(HELP_MSG_NO_DATA_OR_CACHE_OPTION + "\n");
            result = OHOS::ERR_INVALID_VALUE;
        }
    }

    if (result != OHOS::ERR_OK) {
        resultReceiver_.append(HELP_MSG_CLEAN);
    } else {
        // bm clean -c
        if (cleanCache) {
            if (CleanBundleCacheFilesOperation(bundleName, userId)) {
                resultReceiver_ = STRING_CLEAN_CACHE_BUNDLE_OK + "\n";
            } else {
                resultReceiver_ = STRING_CLEAN_CACHE_BUNDLE_NG + "\n";
            }
        }
        // bm clean -d
        if (cleanData) {
            if (CleanBundleDataFilesOperation(bundleName, userId)) {
                resultReceiver_.append(STRING_CLEAN_DATA_BUNDLE_OK + "\n");
            } else {
                resultReceiver_.append(STRING_CLEAN_DATA_BUNDLE_NG + "\n");
            }
        }
    }
    return result;
}

ErrCode BundleManagerShellCommand::RunAsEnableCommand()
{
    int result = OHOS::ERR_OK;
    int option = -1;
    int counter = 0;
    std::string bundleName = "";
    std::string abilityName = "";
    int32_t userId = Constants::UNSPECIFIED_USERID;
    while (true) {
        counter++;
        option = getopt_long(argc_, argv_, SHORT_OPTIONS.c_str(), LONG_OPTIONS, nullptr);
        APP_LOGD("option: %{public}d, optopt: %{public}d, optind: %{public}d", option, optopt, optind);
        if (optind < 0 || optind > argc_) {
            return OHOS::ERR_INVALID_VALUE;
        }

        if (option == -1) {
            if (counter == 1) {
                // When scanning the first argument
                if (strcmp(argv_[optind], cmd_.c_str()) == 0) {
                    // 'bm enable' with no option: bm enable
                    // 'bm enable' with a wrong argument: bm enable xxx
                    APP_LOGD("'bm enable' with no option.");
                    resultReceiver_.append(HELP_MSG_NO_OPTION + "\n");
                    result = OHOS::ERR_INVALID_VALUE;
                }
            }
            break;
        }

        if (option == '?') {
            switch (optopt) {
                case 'n': {
                    // 'bm enable -n' with no argument: bm enable -n
                    // 'bm enable --bundle-name' with no argument: bm enable --bundle-name
                    APP_LOGD("'bm enable -n' with no argument.");
                    resultReceiver_.append("error: option ");
                    resultReceiver_.append("requires a value.\n");
                    result = OHOS::ERR_INVALID_VALUE;
                    break;
                }
                case 'a': {
                    // 'bm enable -a' with no argument: bm enable -a
                    // 'bm enable --ability-name' with no argument: bm enable --ability-name
                    APP_LOGD("'bm enable -a' with no argument.");
                    resultReceiver_.append("error: option ");
                    resultReceiver_.append("requires a value.\n");
                    result = OHOS::ERR_INVALID_VALUE;
                    break;
                }
                case 'u': {
                    // 'bm enable -u' with no argument: bm enable -u
                    // 'bm enable --user-id' with no argument: bm enable --user-id
                    APP_LOGD("'bm enable -u' with no argument.");
                    resultReceiver_.append("error: option ");
                    resultReceiver_.append("requires a value.\n");
                    result = OHOS::ERR_INVALID_VALUE;
                    break;
                }
                default: {
                    // 'bm enable' with an unknown option: bm enable -x
                    // 'bm enable' with an unknown option: bm enable -xxx
                    std::string unknownOption = "";
                    std::string unknownOptionMsg = GetUnknownOptionMsg(unknownOption);
                    APP_LOGD("'bm enable' with an unknown option.");
                    resultReceiver_.append(unknownOptionMsg);
                    result = OHOS::ERR_INVALID_VALUE;
                    break;
                }
            }
            break;
        }

        switch (option) {
            case 'h': {
                // 'bm enable-h'
                // 'bm enable --help'
                APP_LOGD("'bm enable %{public}s'", argv_[optind - 1]);
                result = OHOS::ERR_INVALID_VALUE;
                break;
            }
            case 'n': {
                // 'bm enable -n <bundle-name>'
                // 'bm enable --bundle-name <bundle-name>'
                bundleName = optarg;
                break;
            }
            case 'a': {
                // 'bm enable -a <ability-name>'
                // 'bm enable --ability-name <ability-name>'
                abilityName = optarg;
                break;
            }
            case 'u': {
                // 'bm enable -u userId'
                // 'bm enable --user-id userId'
                APP_LOGD("'bm enable %{public}s %{public}s'", argv_[optind - OFFSET_REQUIRED_ARGUMENT], optarg);
                userId = atoi(optarg);
                break;
            }
            default: {
                result = OHOS::ERR_INVALID_VALUE;
                break;
            }
        }
    }

    if (result == OHOS::ERR_OK) {
        if (resultReceiver_ == "" && bundleName.size() == 0) {
            // 'bm enable ...' with no bundle name option
            APP_LOGD("'bm enable' with no bundle name option.");

            resultReceiver_.append(HELP_MSG_NO_BUNDLE_NAME_OPTION + "\n");
            result = OHOS::ERR_INVALID_VALUE;
        }
    }

    if (result != OHOS::ERR_OK) {
        resultReceiver_.append(HELP_MSG_ENABLE);
    } else {
        AbilityInfo abilityInfo;
        abilityInfo.name = abilityName;
        abilityInfo.bundleName = bundleName;
        bool enableResult = SetApplicationEnabledOperation(abilityInfo, true, userId);
        if (enableResult == true) {
            resultReceiver_ = STRING_ENABLE_BUNDLE_OK + "\n";
        } else {
            resultReceiver_ = STRING_ENABLE_BUNDLE_NG + "\n";
        }
    }
    return result;
}

ErrCode BundleManagerShellCommand::RunAsDisableCommand()
{
    int result = OHOS::ERR_OK;
    int option = -1;
    int counter = 0;
    std::string bundleName = "";
    std::string abilityName = "";
    int32_t userId = Constants::UNSPECIFIED_USERID;
    while (true) {
        counter++;
        option = getopt_long(argc_, argv_, SHORT_OPTIONS.c_str(), LONG_OPTIONS, nullptr);
        APP_LOGD("option: %{public}d, optopt: %{public}d, optind: %{public}d", option, optopt, optind);
        if (optind < 0 || optind > argc_) {
            return OHOS::ERR_INVALID_VALUE;
        }
        if (option == -1) {
            if (counter == 1) {
                // When scanning the first argument
                if (strcmp(argv_[optind], cmd_.c_str()) == 0) {
                    // 'bm disable' with no option: bm disable
                    // 'bm disable' with a wrong argument: bm disable xxx
                    APP_LOGD("'bm disable' with no option.");
                    resultReceiver_.append(HELP_MSG_NO_OPTION + "\n");
                    result = OHOS::ERR_INVALID_VALUE;
                }
            }
            break;
        }
        if (option == '?') {
            switch (optopt) {
                case 'n': {
                    // 'bm disable -n' with no argument: bm disable -n
                    // 'bm disable --bundle-name' with no argument: bm disable --bundle-name
                    APP_LOGD("'bm disable' with no argument.");
                    resultReceiver_.append("error: option ");
                    resultReceiver_.append("requires a value.\n");
                    result = OHOS::ERR_INVALID_VALUE;
                    break;
                }
                case 'a': {
                    // 'bm disable -a' with no argument: bm disable -a
                    // 'bm disable --ability-name' with no argument: bm disable --ability-name
                    APP_LOGD("'bm disable -a' with no argument.");
                    resultReceiver_.append("error: option ");
                    resultReceiver_.append("requires a value.\n");
                    result = OHOS::ERR_INVALID_VALUE;
                    break;
                }
                case 'u': {
                    // 'bm disable -u' with no argument: bm disable -u
                    // 'bm disable --user-id' with no argument: bm disable --user-id
                    APP_LOGD("'bm disable -u' with no argument.");
                    resultReceiver_.append("error: option ");
                    resultReceiver_.append("requires a value.\n");
                    result = OHOS::ERR_INVALID_VALUE;
                    break;
                }
                default: {
                    // 'bm disable' with an unknown option: bm disable -x
                    // 'bm disable' with an unknown option: bm disable -xxx
                    std::string unknownOption = "";
                    std::string unknownOptionMsg = GetUnknownOptionMsg(unknownOption);
                    APP_LOGD("'bm disable' with an unknown option.");
                    resultReceiver_.append(unknownOptionMsg);
                    result = OHOS::ERR_INVALID_VALUE;
                    break;
                }
            }
            break;
        }
        switch (option) {
            case 'h': {
                // 'bm disable -h'
                // 'bm disable --help'
                APP_LOGD("'bm disable %{public}s'", argv_[optind - 1]);
                result = OHOS::ERR_INVALID_VALUE;
                break;
            }
            case 'n': {
                // 'bm disable -n <bundle-name>'
                // 'bm disable --bundle-name <bundle-name>'
                bundleName = optarg;
                break;
            }
            case 'a': {
                // 'bm disable -a <ability-name>'
                // 'bm disable --ability-name <ability-name>'
                abilityName = optarg;
                break;
            }
            case 'u': {
                // 'bm disable -u userId'
                // 'bm disable --user-id userId'
                APP_LOGD("'bm disable %{public}s %{public}s'", argv_[optind - OFFSET_REQUIRED_ARGUMENT], optarg);
                userId = atoi(optarg);
                break;
            }
            default: {
                result = OHOS::ERR_INVALID_VALUE;
                break;
            }
        }
    }
    if (result == OHOS::ERR_OK) {
        if (resultReceiver_ == "" && bundleName.size() == 0) {
            // 'bm disable ...' with no bundle name option
            APP_LOGD("'bm disable' with no bundle name option.");
            resultReceiver_.append(HELP_MSG_NO_BUNDLE_NAME_OPTION + "\n");
            result = OHOS::ERR_INVALID_VALUE;
        }
    }
    if (result != OHOS::ERR_OK) {
        resultReceiver_.append(HELP_MSG_DISABLE);
    } else {
        AbilityInfo abilityInfo;
        abilityInfo.name = abilityName;
        abilityInfo.bundleName = bundleName;
        bool enableResult = SetApplicationEnabledOperation(abilityInfo, false, userId);
        if (enableResult == true) {
            resultReceiver_ = STRING_DISABLE_BUNDLE_OK + "\n";
        } else {
            resultReceiver_ = STRING_DISABLE_BUNDLE_NG + "\n";
        }
    }
    return result;
}

ErrCode BundleManagerShellCommand::RunAsQueryCommand()
{
    int result = OHOS::ERR_OK;
    int option = -1;
    int counter = 0;
    std::string queryResults = "";
    std::string bundleName = "";
    std::string elementName = "";
    std::string metadataName = "";
    int32_t userId = Constants::ANY_USERID;
    while (true) {
        counter++;
        option = getopt_long(argc_, argv_, SHORT_OPTIONS_QUERY.c_str(), LONG_OPTIONS_QUERY, nullptr);
        APP_LOGD("option: %{public}d, optopt: %{public}d, optind: %{public}d", option, optopt, optind);
        if (optind < 0 || optind > argc_) {
            return OHOS::ERR_INVALID_VALUE;
        }
        if (option == -1) {
            if (counter == 1) {
                if (strcmp(argv_[optind], cmd_.c_str()) == 0) {
                    // 1.'bm query' with no option: bm query
                    // 2.'bm query' with a wrong argument: bm query -xxx
                    APP_LOGD("'bm query' %{public}s", HELP_MSG_NO_OPTION.c_str());
                    resultReceiver_.append(HELP_MSG_NO_OPTION + "\n");
                    result = OHOS::ERR_INVALID_VALUE;
                }
            }
            break;
        }
        if (option == '?') {
            switch (optopt) {
                case 'n': {
                    // 'bm query -n' with no argument: bm query -n
                    // 'bm query --bundle-name' with no argument: bm query --bundle-name
                    APP_LOGD("'bm query -n' with no argument.");
                    resultReceiver_.append("error: option ");
                    resultReceiver_.append("requires a value.\n");
                    result = OHOS::ERR_INVALID_VALUE;
                    break;
                }
                case 'e': {
                    APP_LOGD("'bm query -e' with no argument.");
                    resultReceiver_.append("error: option ");
                    resultReceiver_.append("requires a value.\n");
                    result = OHOS::ERR_INVALID_VALUE;
                    break;
                }
                case 'm': {
                    APP_LOGD("'bm query -m' with no argument.");
                    resultReceiver_.append("error: option ");
                    resultReceiver_.append("requires a value.\n");
                    result = OHOS::ERR_INVALID_VALUE;
                    break;
                }
                case 'u': {
                    APP_LOGD("'bm query -u' with no argument.");
                    resultReceiver_.append("error: option ");
                    resultReceiver_.append("requires a value.\n");
                    result = OHOS::ERR_INVALID_VALUE;
                    break;
                }
                default: {
                    // 'bm query' with an unknown option: bm dump -x
                    // 'bm query' with an unknown option: bm dump -xxx
                    std::string unknownOption = "";
                    std::string unknownOptionMsg = GetUnknownOptionMsg(unknownOption);
                    APP_LOGD("'bm dump' with an unknown option.");
                    resultReceiver_.append(unknownOptionMsg);
                    result = OHOS::ERR_INVALID_VALUE;
                    break;
                }
            }
            break;
        }
        switch (option) {
            case 'h': {
                result = OHOS::ERR_INVALID_VALUE;
                break;
            }
            case 'n': {
                APP_LOGD("'bm query bundleName is %{public}s'", optarg);
                bundleName = optarg;
                break;
            }
            case 'e': {
                APP_LOGD("'bm query elementName is %{public}s'", optarg);
                elementName = optarg;
                break;
            }
            case 'm': {
                APP_LOGD("'bm query metadataName is %{public}s'", optarg);
                metadataName = optarg;
                break;
            }
            case 'u': {
                APP_LOGD("'bm query userId is %{public}s'", optarg);
                userId = atoi(optarg);
                break;
            }
            default: {
                result = OHOS::ERR_INVALID_VALUE;
                break;
            }
        }
    }
    if (result != OHOS::ERR_OK) {
        resultReceiver_.append(HELP_MSG_QUERY);
        return result;
    }
    if (!QueryOperation(bundleName, elementName, metadataName, userId, queryResults)) {
        result = OHOS::ERR_INVALID_VALUE;
    }
    if (queryResults.empty()) {
        resultReceiver_.append(NO_PROFILE);
        return result;
    }
    resultReceiver_.append(queryResults);
    return result;
}

bool BundleManagerShellCommand::CheckArguments(const std::string &argument) const
{
    if (argument.empty() || argument == "-n" || argument == "-m" || argument == "-e" || argument == "bundle-name" ||
        argument == "element-name" || argument == "metadata-name") {
        return false;
    }
    return true;
}

ErrCode BundleManagerShellCommand::RunAsGetCommand()
{
    int result = OHOS::ERR_OK;
    int option = -1;
    int counter = 0;
    while (true) {
        counter++;
        if (argc_ > MAX_ARGUEMENTS_NUMBER) {
            resultReceiver_.append(HELP_MSG_GET);
            return OHOS::ERR_INVALID_VALUE;
        }
        option = getopt_long(argc_, argv_, SHORT_OPTIONS_GET.c_str(), LONG_OPTIONS_GET, nullptr);
        APP_LOGD("option: %{public}d, optopt: %{public}d, optind: %{public}d", option, optopt, optind);
        if (optind < 0 || optind > argc_) {
            return OHOS::ERR_INVALID_VALUE;
        }
        if (option == -1) {
            if (counter == 1) {
                if (strcmp(argv_[optind], cmd_.c_str()) == 0) {
                    // 1.'bm get' with no option: bm query
                    // 2.'bm get' with a wrong argument: bm get -xxx
                    APP_LOGD("'bm get' %{public}s", HELP_MSG_NO_OPTION.c_str());
                    resultReceiver_.append(HELP_MSG_NO_OPTION + "\n");
                    result = OHOS::ERR_INVALID_VALUE;
                }
            }
            break;
        }
        switch (option) {
            case 'h': {
                result = OHOS::ERR_INVALID_VALUE;
                break;
            }
            case 'u': {
                break;
            }
            default: {
                result = OHOS::ERR_INVALID_VALUE;
                resultReceiver_.append(STRING_INCORRECT_OPTION + "\n");
                break;
            }
        }
    }
    if (result != OHOS::ERR_OK) {
        resultReceiver_.append(HELP_MSG_GET);
        return result;
    }
    resultReceiver_.append(STRING_GET_UDID_OK + "\n");
    resultReceiver_.append(GetUdid() + "\n");
    return result;
}

std::string BundleManagerShellCommand::GetUdid() const
{
    char innerUdid[DEVICE_UDID_LENGTH] = { 0 };
    int ret = GetDevUdid(innerUdid, DEVICE_UDID_LENGTH);
    if (ret != 0) {
        APP_LOGE("GetUdid failed! ret = %{public}d.", ret);
        return STRING_GET_UDID_NG;
    }
    std::string udid = innerUdid;
    return udid;
}

std::string BundleManagerShellCommand::DumpBundleList(int32_t userId) const
{
    std::string dumpResults;
    bool dumpRet = bundleMgrProxy_->DumpInfos(
        DumpFlag::DUMP_BUNDLE_LIST, BUNDLE_NAME_EMPTY, userId, dumpResults);
    if (!dumpRet) {
        APP_LOGE("failed to dump bundle list.");
    }
    return dumpResults;
}

std::string BundleManagerShellCommand::DumpBundleInfos(int32_t userId) const
{
    std::string dumpResults;
    bool dumpRet = bundleMgrProxy_->DumpInfos(
        DumpFlag::DUMP_ALL_BUNDLE_INFO, BUNDLE_NAME_EMPTY, userId, dumpResults);
    if (!dumpRet) {
        APP_LOGE("failed to dump bundle infos.");
    }
    return dumpResults;
}

std::string BundleManagerShellCommand::DumpBundleInfo(const std::string &bundleName, int32_t userId) const
{
    std::string dumpResults;
    bool dumpRet = bundleMgrProxy_->DumpInfos(
        DumpFlag::DUMP_BUNDLE_INFO, bundleName, userId, dumpResults);
    if (!dumpRet) {
        APP_LOGE("failed to dump bundle info.");
    }
    return dumpResults;
}

std::string BundleManagerShellCommand::DumpShortcutInfos(const std::string &bundleName, int32_t userId) const
{
    std::string dumpResults;
    bool dumpRet = bundleMgrProxy_->DumpInfos(
        DumpFlag::DUMP_SHORTCUT_INFO, bundleName, userId, dumpResults);
    if (!dumpRet) {
        APP_LOGE("failed to dump shortcut infos.");
    }
    return dumpResults;
}

std::string BundleManagerShellCommand::DumpDistributedBundleInfo(const std::string &deviceId,
    int32_t userId, const std::string &bundleName)
{
    std::string dumpResults = "";
    DistributedBundleInfo bundleInfo;
    bool dumpRet = bundleMgrProxy_->GetDistributedBundleInfo(deviceId, userId, bundleName, bundleInfo);
    if (!dumpRet) {
        APP_LOGE("failed to dump distributed bundleInfo.");
    } else {
        dumpResults.append("distributed bundleInfo");
        dumpResults.append(":\n");
        dumpResults.append(bundleInfo.ToString());
        dumpResults.append("\n");
    }
    return dumpResults;
}

int32_t BundleManagerShellCommand::InstallOperation(const std::vector<std::string> &bundlePaths,
    InstallParam &installParam) const
{
    std::vector<std::string> realPathVec;
    for (auto &bundlePath : bundlePaths) {
        std::string absoluteBundlePath = "";
        if (bundlePath.empty()) {
            continue;
        }
        if (bundlePath.at(0) == '/') {
            // absolute path
            absoluteBundlePath.append(bundlePath);
        } else {
            // relative path
            char *currentPathPtr = getcwd(nullptr, 0);

            if (currentPathPtr != nullptr) {
                absoluteBundlePath.append(currentPathPtr);
                absoluteBundlePath.append('/' + bundlePath);

                free(currentPathPtr);
                currentPathPtr = nullptr;
            }
        }
        realPathVec.emplace_back(absoluteBundlePath);
    }

    sptr<StatusReceiverImpl> statusReceiver(new StatusReceiverImpl());
    sptr<BundleDeathRecipient> recipient(new BundleDeathRecipient(statusReceiver));
    bundleInstallerProxy_->AsObject()->AddDeathRecipient(recipient);
    bundleInstallerProxy_->Install(realPathVec, installParam, statusReceiver);
    return statusReceiver->GetResultCode();
}

int32_t BundleManagerShellCommand::UninstallOperation(
    const std::string &bundleName, const std::string &moduleName, InstallParam &installParam) const
{
    sptr<StatusReceiverImpl> statusReceiver(new StatusReceiverImpl());

    APP_LOGD("bundleName: %{public}s", bundleName.c_str());
    APP_LOGD("moduleName: %{public}s", moduleName.c_str());

    sptr<BundleDeathRecipient> recipient(new BundleDeathRecipient(statusReceiver));
    bundleInstallerProxy_->AsObject()->AddDeathRecipient(recipient);
    if (moduleName.size() != 0) {
        bundleInstallerProxy_->Uninstall(bundleName, moduleName, installParam, statusReceiver);
    } else {
        bundleInstallerProxy_->Uninstall(bundleName, installParam, statusReceiver);
    }

    return statusReceiver->GetResultCode();
}

bool BundleManagerShellCommand::CleanBundleCacheFilesOperation(const std::string &bundleName, int32_t userId) const
{
    userId = GetCurrentUserId(userId);
    sptr<CleanCacheCallbackImpl> cleanCacheCallBack(new CleanCacheCallbackImpl());
    bool cleanRet = bundleMgrProxy_->CleanBundleCacheFiles(bundleName, cleanCacheCallBack, userId);
    if (cleanRet) {
        return cleanCacheCallBack->GetResultCode();
    }
    APP_LOGE("clean bundle cache files operation failed");
    return cleanRet;
}

bool BundleManagerShellCommand::CleanBundleDataFilesOperation(const std::string &bundleName, int32_t userId) const
{
    APP_LOGD("bundleName: %{public}s, userId:%{public}d", bundleName.c_str(), userId);
    userId = GetCurrentUserId(userId);
    bool cleanRet = bundleMgrProxy_->CleanBundleDataFiles(bundleName, userId);
    if (!cleanRet) {
        APP_LOGE("clean bundle data files operation failed");
    }
    return cleanRet;
}

bool BundleManagerShellCommand::SetApplicationEnabledOperation(const AbilityInfo &abilityInfo,
    bool isEnable, int32_t userId) const
{
    APP_LOGD("bundleName: %{public}s", abilityInfo.bundleName.c_str());
    userId = GetCurrentUserId(userId);
    bool ret = false;
    if (abilityInfo.name.size() == 0) {
        ret = bundleMgrProxy_->SetApplicationEnabled(abilityInfo.bundleName, isEnable, userId);
    } else {
        ret = bundleMgrProxy_->SetAbilityEnabled(abilityInfo, isEnable, userId);
    }
    if (!ret) {
        if (isEnable) {
            APP_LOGE("enable bundle failed");
        } else {
            APP_LOGE("disable bundle failed");
        }
    }
    return ret;
}

bool BundleManagerShellCommand::QueryOperation(const std::string &bundleName, const std::string &elementName,
    const std::string &metadataName, const int32_t userId, std::string &result) const
{
    if (!CheckArguments(bundleName) || !CheckArguments(elementName) || !CheckArguments(metadataName)) {
        APP_LOGD("bundleName: %{public}s, elementName: %{public}s, metadataName: %{public}s", bundleName.c_str(),
            elementName.c_str(), metadataName.c_str());
        result += STRING_QUERY_NEED_CORRECT_ARGUMENTS;
        result += HELP_MSG_QUERY;
        return false;
    }
    BundleInfo info;
    auto ret = bundleMgrProxy_->GetBundleInfo(bundleName,
        GET_BUNDLE_WITH_ABILITIES | GET_BUNDLE_WITH_EXTENSION_INFO, info, userId);
    if (!ret) {
        APP_LOGE("getBundleInfo failed");
        result += STRING_QUERY_NEED_CORRECT_ARGUMENTS;
        result = HELP_MSG_QUERY;
        return false;
    }
    BundleMgrClient bundleClient;
    std::vector<std::string> profiles;
    auto spliceResult = [&](const auto &str)->void {
        result.append(str);
        result.append("\n");
    };
    auto matchInfo = [&](const auto &info)->void {
        if (elementName.compare(info.name) == 0) {
            if (bundleClient.GetResConfigFile(info, metadataName, profiles)) {
                for_each(profiles.begin(), profiles.end(), spliceResult);
            }
        }
    };
    for_each(info.hapModuleInfos.begin(), info.hapModuleInfos.end(), matchInfo);
    for_each(info.abilityInfos.begin(), info.abilityInfos.end(), matchInfo);
    for_each(info.extensionInfos.begin(), info.extensionInfos.end(), matchInfo);
    return true;
}

int32_t BundleManagerShellCommand::GetCurrentUserId(int32_t userId) const
{
    if (userId == Constants::UNSPECIFIED_USERID) {
        std::vector<int> activeIds;
        int32_t ret = AccountSA::OsAccountManager::QueryActiveOsAccountIds(activeIds);
        if (ret != 0) {
            APP_LOGE("QueryActiveOsAccountIds failed! ret = %{public}d.", ret);
            return userId;
        }
        if (activeIds.empty()) {
            APP_LOGE("QueryActiveOsAccountIds activeIds empty");
            return userId;
        }
        return activeIds[0];
    }
    return userId;
}
}  // namespace AppExecFwk
}  // namespace OHOS
