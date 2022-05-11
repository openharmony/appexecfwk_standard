/*
 * Copyright (c) 2021-2022 Huawei Device Co., Ltd.
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

#ifndef FOUNDATION_APPEXECFWK_INTERFACES_INNERKITS_APPEXECFWK_BASE_INCLUDE_APPEXECFWK_ERRORS_H
#define FOUNDATION_APPEXECFWK_INTERFACES_INNERKITS_APPEXECFWK_BASE_INCLUDE_APPEXECFWK_ERRORS_H

#include "errors.h"

namespace OHOS {
enum {
    APPEXECFWK_MODULE_COMMON = 0x00,
    APPEXECFWK_MODULE_APPMGR = 0x01,
    APPEXECFWK_MODULE_BUNDLEMGR = 0x02,
    // Reserved 0x03 ~ 0x0f for new modules, Event related modules start from 0x10
    APPEXECFWK_MODULE_EVENTMGR = 0x10,
    APPEXECFWK_MODULE_HIDUMP = 0x11
};

// Error code for Common
constexpr ErrCode APPEXECFWK_COMMON_ERR_OFFSET = ErrCodeOffset(SUBSYS_APPEXECFWK, APPEXECFWK_MODULE_COMMON);
enum {
    ERR_APPEXECFWK_SERVICE_NOT_READY = APPEXECFWK_COMMON_ERR_OFFSET + 1,
    ERR_APPEXECFWK_SERVICE_NOT_CONNECTED,
    ERR_APPEXECFWK_INVALID_UID,
    ERR_APPEXECFWK_INVALID_PID,
    ERR_APPEXECFWK_PARCEL_ERROR,
    ERR_APPEXECFWK_FAILED_SERVICE_DIED,
    ERR_APPEXECFWK_OPERATION_TIME_OUT,
};

// Error code for AppMgr
constexpr ErrCode APPEXECFWK_APPMGR_ERR_OFFSET = ErrCodeOffset(SUBSYS_APPEXECFWK, APPEXECFWK_MODULE_APPMGR);
enum {
    ERR_APPEXECFWK_ASSEMBLE_START_MSG_FAILED = APPEXECFWK_APPMGR_ERR_OFFSET + 1,
    ERR_APPEXECFWK_CONNECT_APPSPAWN_FAILED,
    ERR_APPEXECFWK_BAD_APPSPAWN_CLIENT,
    ERR_APPEXECFWK_BAD_APPSPAWN_SOCKET,
    ERR_APPEXECFWK_SOCKET_READ_FAILED,
    ERR_APPEXECFWK_SOCKET_WRITE_FAILED
};

// Error code for BundleMgr
constexpr ErrCode APPEXECFWK_BUNDLEMGR_ERR_OFFSET = ErrCodeOffset(SUBSYS_APPEXECFWK, APPEXECFWK_MODULE_BUNDLEMGR);
enum {
    // the install error code from 0x0001 ~ 0x0020.
    ERR_APPEXECFWK_INSTALL_INTERNAL_ERROR = APPEXECFWK_BUNDLEMGR_ERR_OFFSET + 0x0001, // 8519681
    ERR_APPEXECFWK_INSTALL_HOST_INSTALLER_FAILED,
    ERR_APPEXECFWK_INSTALL_PARSE_FAILED,
    ERR_APPEXECFWK_INSTALL_VERSION_DOWNGRADE,
    ERR_APPEXECFWK_INSTALL_VERIFICATION_FAILED,
    ERR_APPEXECFWK_INSTALL_PARAM_ERROR,
    ERR_APPEXECFWK_INSTALL_PERMISSION_DENIED,
    ERR_APPEXECFWK_INSTALL_ENTRY_ALREADY_EXIST,
    ERR_APPEXECFWK_INSTALL_STATE_ERROR,
    ERR_APPEXECFWK_INSTALL_FILE_PATH_INVALID = 8519690,
    ERR_APPEXECFWK_INSTALL_INVALID_HAP_NAME,
    ERR_APPEXECFWK_INSTALL_INVALID_BUNDLE_FILE,
    ERR_APPEXECFWK_INSTALL_INVALID_HAP_SIZE,
    ERR_APPEXECFWK_INSTALL_GENERATE_UID_ERROR,
    ERR_APPEXECFWK_INSTALL_INSTALLD_SERVICE_ERROR,
    ERR_APPEXECFWK_INSTALL_BUNDLE_MGR_SERVICE_ERROR,
    ERR_APPEXECFWK_INSTALL_ALREADY_EXIST,
    ERR_APPEXECFWK_INSTALL_BUNDLENAME_NOT_SAME,
    ERR_APPEXECFWK_INSTALL_VERSIONCODE_NOT_SAME,
    ERR_APPEXECFWK_INSTALL_VERSIONNAME_NOT_SAME = 8519700,
    ERR_APPEXECFWK_INSTALL_MINCOMPATIBLE_VERSIONCODE_NOT_SAME,
    ERR_APPEXECFWK_INSTALL_VENDOR_NOT_SAME,
    ERR_APPEXECFWK_INSTALL_RELEASETYPE_TARGET_NOT_SAME,
    ERR_APPEXECFWK_INSTALL_RELEASETYPE_NOT_SAME,
    ERR_APPEXECFWK_INSTALL_RELEASETYPE_COMPATIBLE_NOT_SAME,
    ERR_APPEXECFWK_INSTALL_VERSION_NOT_COMPATIBLE,
    ERR_APPEXECFWK_INSTALL_INVALID_NUMBER_OF_ENTRY_HAP,
    ERR_APPEXECFWK_INSTALL_DISK_MEM_INSUFFICIENT,
    ERR_APPEXECFWK_INSTALL_GRANT_REQUEST_PERMISSIONS_FAILED,
    ERR_APPEXECFWK_INSTALL_UPDATE_HAP_TOKEN_FAILED = 8519710,
    ERR_APPEXECFWK_INSTALL_SINGLETON_NOT_SAME,
    ERR_APPEXECFWK_INSTALL_ZERO_USER_WITH_NO_SINGLETON,
    ERR_APPEXECFWK_INSTALL_CHECK_SYSCAP_FAILED,
    ERR_APPEXECFWK_INSTALL_APPTYPE_NOT_SAME,
    ERR_APPEXECFWK_INSTALL_URI_DUPLICATE,
    ERR_APPEXECFWK_INSTALL_TYPE_ERROR,

    // signature errcode
    ERR_APPEXECFWK_INSTALL_FAILED_INVALID_SIGNATURE_FILE_PATH = 8519717,
    ERR_APPEXECFWK_INSTALL_FAILED_BAD_BUNDLE_SIGNATURE_FILE,
    ERR_APPEXECFWK_INSTALL_FAILED_NO_BUNDLE_SIGNATURE,
    ERR_APPEXECFWK_INSTALL_FAILED_VERIFY_APP_PKCS7_FAIL,
    ERR_APPEXECFWK_INSTALL_FAILED_PROFILE_PARSE_FAIL,
    ERR_APPEXECFWK_INSTALL_FAILED_APP_SOURCE_NOT_TRUESTED,
    ERR_APPEXECFWK_INSTALL_FAILED_BAD_DIGEST,
    ERR_APPEXECFWK_INSTALL_FAILED_BUNDLE_INTEGRITY_VERIFICATION_FAILURE,
    ERR_APPEXECFWK_INSTALL_FAILED_FILE_SIZE_TOO_LARGE,
    ERR_APPEXECFWK_INSTALL_FAILED_BAD_PUBLICKEY,
    ERR_APPEXECFWK_INSTALL_FAILED_BAD_BUNDLE_SIGNATURE = 8519727,
    ERR_APPEXECFWK_INSTALL_FAILED_NO_PROFILE_BLOCK_FAIL,
    ERR_APPEXECFWK_INSTALL_FAILED_BUNDLE_SIGNATURE_VERIFICATION_FAILURE,
    ERR_APPEXECFWK_INSTALL_FAILED_VERIFY_SOURCE_INIT_FAIL,
    ERR_APPEXECFWK_INSTALL_FAILED_INCOMPATIBLE_SIGNATURE,
    ERR_APPEXECFWK_INSTALL_FAILED_INCONSISTENT_SIGNATURE,
    ERR_APPEXECFWK_INSTALL_FAILED_MODULE_NAME_EMPTY,
    ERR_APPEXECFWK_INSTALL_FAILED_MODULE_NAME_DUPLICATE,
    ERR_APPEXECFWK_INSTALL_FAILED_CHECK_HAP_HASH_PARAM,

    // install sandbox app
    ERR_APPEXECFWK_SANDBOX_INSTALL_INTERNAL_ERROR = 8519800,
    ERR_APPEXECFWK_SANDBOX_INSTALL_APP_NOT_EXISTED,
    ERR_APPEXECFWK_SANDBOX_INSTALL_PARAM_ERROR,
    ERR_APPEXECFWK_SANDBOX_INSTALL_WRITE_PARCEL_ERROR,
    ERR_APPEXECFWK_SANDBOX_INSTALL_READ_PARCEL_ERROR,
    ERR_APPEXECFWK_SANDBOX_INSTALL_SEND_REQUEST_ERROR,
    ERR_APPEXECFWK_SANDBOX_INSTALL_USER_NOT_EXIST,
    ERR_APPEXECFWK_SANDBOX_INSTALL_INVALID_APP_INDEX,
    ERR_APPEXECFWK_SANDBOX_INSTALL_NOT_INSTALLED_AT_SPECIFIED_USERID,
    ERR_APPEXECFWK_SANDBOX_INSTALL_NO_SANDBOX_APP_INFO,
    ERR_APPEXECFWK_SANDBOX_INSTALL_UNKNOWN_INSTALL_TYPE,
    ERR_APPEXECFWK_SANDBOX_INSTALL_DELETE_APP_INDEX_FAILED,

    ERR_APPEXECFWK_PARSE_UNEXPECTED = APPEXECFWK_BUNDLEMGR_ERR_OFFSET + 0x00c8, // 8519880
    ERR_APPEXECFWK_PARSE_MISSING_BUNDLE,
    ERR_APPEXECFWK_PARSE_MISSING_ABILITY,
    ERR_APPEXECFWK_PARSE_NO_PROFILE,
    ERR_APPEXECFWK_PARSE_BAD_PROFILE,
    ERR_APPEXECFWK_PARSE_PROFILE_PROP_TYPE_ERROR,
    ERR_APPEXECFWK_PARSE_PROFILE_MISSING_PROP,
    ERR_APPEXECFWK_PARSE_PERMISSION_ERROR,
    ERR_APPEXECFWK_PARSE_PROFILE_PROP_CHECK_ERROR,
    ERR_APPEXECFWK_PARSE_RPCID_FAILED,

    ERR_APPEXECFWK_INSTALLD_PARAM_ERROR,
    ERR_APPEXECFWK_INSTALLD_GET_PROXY_ERROR,
    ERR_APPEXECFWK_INSTALLD_CREATE_DIR_FAILED,
    ERR_APPEXECFWK_INSTALLD_CREATE_DIR_EXIST,
    ERR_APPEXECFWK_INSTALLD_CHOWN_FAILED,
    ERR_APPEXECFWK_INSTALLD_REMOVE_DIR_FAILED,
    ERR_APPEXECFWK_INSTALLD_EXTRACT_FILES_FAILED,
    ERR_APPEXECFWK_INSTALLD_RNAME_DIR_FAILED,
    ERR_APPEXECFWK_INSTALLD_CLEAN_DIR_FAILED,

    ERR_APPEXECFWK_UNINSTALL_SYSTEM_APP_ERROR,
    ERR_APPEXECFWK_UNINSTALL_KILLING_APP_ERROR,
    ERR_APPEXECFWK_UNINSTALL_INVALID_NAME,
    ERR_APPEXECFWK_UNINSTALL_PARAM_ERROR,
    ERR_APPEXECFWK_UNINSTALL_PERMISSION_DENIED,
    ERR_APPEXECFWK_UNINSTALL_BUNDLE_MGR_SERVICE_ERROR,
    ERR_APPEXECFWK_UNINSTALL_MISSING_INSTALLED_BUNDLE,
    ERR_APPEXECFWK_UNINSTALL_MISSING_INSTALLED_MODULE,

    ERR_APPEXECFWK_FAILED_GET_INSTALLER_PROXY,
    ERR_APPEXECFWK_FAILED_GET_BUNDLE_INFO,
    ERR_APPEXECFWK_FAILED_GET_ABILITY_INFO,
    ERR_APPEXECFWK_FAILED_GET_RESOURCEMANAGER,
    ERR_APPEXECFWK_FAILED_GET_REMOTE_PROXY,
    ERR_APPEXECFWK_PERMISSION_DENIED,

    ERR_APPEXECFWK_RECOVER_GET_BUNDLEPATH_ERROR = APPEXECFWK_BUNDLEMGR_ERR_OFFSET + 0x0201, // 8520193
    ERR_APPEXECFWK_RECOVER_INVALID_BUNDLE_NAME,

    ERR_APPEXECFWK_USER_NOT_EXIST = APPEXECFWK_BUNDLEMGR_ERR_OFFSET + 0x0301,
    ERR_APPEXECFWK_USER_CREATE_FALIED,
    ERR_APPEXECFWK_USER_REMOVE_FALIED,
    ERR_APPEXECFWK_USER_NOT_INSTALL_HAP,

    // error code in prebundle sacn
    ERR_APPEXECFWK_PARSE_FILE_FAILED
};

// Error code for Hidump
constexpr ErrCode APPEXECFWK_HIDUMP = ErrCodeOffset(SUBSYS_APPEXECFWK, APPEXECFWK_MODULE_HIDUMP);
enum {
    ERR_APPEXECFWK_HIDUMP_ERROR = APPEXECFWK_HIDUMP + 1,
    ERR_APPEXECFWK_HIDUMP_INVALID_ARGS,
    ERR_APPEXECFWK_HIDUMP_UNKONW,
    ERR_APPEXECFWK_HIDUMP_SERVICE_ERROR
};

}  // namespace OHOS

#endif  // FOUNDATION_APPEXECFWK_INTERFACES_INNERKITS_APPEXECFWK_BASE_INCLUDE_APPEXECFWK_ERRORS_H
