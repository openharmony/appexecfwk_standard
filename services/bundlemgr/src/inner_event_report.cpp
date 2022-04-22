/*
 * Copyright (c) 2022-2022 Huawei Device Co., Ltd.
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

#include "inner_event_report.h"

#include "app_log_wrapper.h"
#include "hisysevent.h"

namespace OHOS {
namespace AppExecFwk {
namespace {
// event type
const std::string BUNDLE_INSTALL_EXCEPTION = "BUNDLE_INSTALL_EXCEPTION";
const std::string BUNDLE_UNINSTALL_EXCEPTION = "BUNDLE_UNINSTALL_EXCEPTION";
const std::string BUNDLE_UPDATE_EXCEPTION = "BUNDLE_UPDATE_EXCEPTION";
const std::string PRE_BUNDLE_RECOVER_EXCEPTION = "PRE_BUNDLE_RECOVER_EXCEPTION";
const std::string BUNDLE_STATE_CHANGE_EXCEPTION = "BUNDLE_STATE_CHANGE_EXCEPTION";
const std::string BUNDLE_CLEAN_CACHE_EXCEPTION = "BUNDLE_CLEAN_CACHE_EXCEPTION";

const std::string BOOT_SCAN_START = "BOOT_SCAN_START";
const std::string BOOT_SCAN_END = "BOOT_SCAN_END";
const std::string BUNDLE_INSTALL = "BUNDLE_INSTALL";
const std::string BUNDLE_UNINSTALL = "BUNDLE_UNINSTALL";
const std::string BUNDLE_UPDATE = "BUNDLE_UPDATE";
const std::string PRE_BUNDLE_RECOVER = "PRE_BUNDLE_RECOVER";
const std::string BUNDLE_COMPONENT_STATE_CHANGE = "BUNDLE_COMPONENT_STATE_CHANGE";
const std::string BUNDLE_CLEAN_CACHE = "BUNDLE_CLEAN_CACHE";

// event params
const std::string EVENT_PARAM_USERID = "USERID";
const std::string EVENT_PARAM_UID = "UID";
const std::string EVENT_PARAM_BUNDLE_NAME = "BUNDLE_NAME";
const std::string EVENT_PARAM_ERROR_CODE = "ERROR_CODE";
const std::string EVENT_PARAM_ABILITY_NAME = "ABILITY_NAME";
const std::string EVENT_PARAM_TIME = "TIME";
const std::string EVENT_PARAM_VERSION = "VERSION";
const std::string EVENT_PARAM_OLD_VERSION = "OLD_VERSION";
const std::string EVENT_PARAM_NEW_VERSION = "NEW_VERSION";
const std::string EVENT_PARAM_PERIOD = "PERIOD";
const std::string EVENT_PARAM_CLEAN_TYPE = "CLEAN_TYPE";
const std::string EVENT_PARAM_INSTALL_TYPE = "INSTALL_TYPE";
const std::string EVENT_PARAM_STATE = "STATE";

const std::string FREE_INSTALL_TYPE = "FreeInstall";
const std::string PRE_BUNDLE_INSTALL_TYPE = "PreBundleInstall";
const std::string NORMAL_INSTALL_TYPE = "normalInstall";
const std::string NORMAL_PERIOD = "Normal";
const std::string BOOT_PERIOD = "Boot";
const std::string REBOOT_PERIOD = "Reboot";
const std::string CREATE_USER_PERIOD = "CreateUser";
const std::string REMOVE_USER_PERIOD = "CreateUser";
const std::string CLEAN_CACHE = "cleanCache";
const std::string CLEAN_DATA = "cleanData";
const std::string ENABLE = "enable";
const std::string DISABLE = "disable";

const std::unordered_map<InstallPeriod, std::string> INSTALL_PERIOD_STR_MAP = {
    { InstallPeriod::NORMAL, NORMAL_PERIOD },
    { InstallPeriod::BOOT, BOOT_PERIOD },
    { InstallPeriod::REBOOT, REBOOT_PERIOD },
    { InstallPeriod::CREATE_USER, CREATE_USER_PERIOD },
    { InstallPeriod::REMOVE_USER, REMOVE_USER_PERIOD },
};
}

std::unordered_map<BMSEventType, void (*)(const EventInfo& eventInfo)>
    InnerEventReport::bmsSysEventMap_ = {
        { BMSEventType::BUNDLE_INSTALL_EXCEPTION,
            [](const EventInfo& eventInfo) {
                InnerSendBundleInstallExceptionEvent(eventInfo);
            } },
        { BMSEventType::BUNDLE_UNINSTALL_EXCEPTION,
            [](const EventInfo& eventInfo) {
                InnerSendBundleUninstallExceptionEvent(eventInfo);
            } },
        { BMSEventType::BUNDLE_UPDATE_EXCEPTION,
            [](const EventInfo& eventInfo) {
                InnerSendBundleUpdateExceptionEvent(eventInfo);
            } },
        { BMSEventType::PRE_BUNDLE_RECOVER_EXCEPTION,
            [](const EventInfo& eventInfo) {
                InnerSendPreBundleRecoverExceptionEvent(eventInfo);
            } },
        { BMSEventType::BUNDLE_STATE_CHANGE_EXCEPTION,
            [](const EventInfo& eventInfo) {
                InnerSendBundleComponentChangeExceptionEvent(eventInfo);
            } },
        { BMSEventType::BUNDLE_CLEAN_CACHE_EXCEPTION,
            [](const EventInfo& eventInfo) {
                InnerSendBundleCleanCacheExceptionEvent(eventInfo);
            } },
        { BMSEventType::BOOT_SCAN_START,
            [](const EventInfo& eventInfo) {
                InnerSendBootScanStartEvent(eventInfo);
            } },
        { BMSEventType::BOOT_SCAN_END,
            [](const EventInfo& eventInfo) {
                InnerSendBootScanEndEvent(eventInfo);
            } },
        { BMSEventType::BUNDLE_INSTALL,
            [](const EventInfo& eventInfo) {
                InnerSendBundleInstallEvent(eventInfo);
            } },
        { BMSEventType::BUNDLE_UNINSTALL,
            [](const EventInfo& eventInfo) {
                InnerSendBundleUninstallEvent(eventInfo);
            } },
        { BMSEventType::BUNDLE_UPDATE,
            [](const EventInfo& eventInfo) {
                InnerSendBundleUpdateEvent(eventInfo);
            } },
        { BMSEventType::PRE_BUNDLE_RECOVER,
            [](const EventInfo& eventInfo) {
                InnerSendPreBundleRecoverEvent(eventInfo);
            } },
        { BMSEventType::BUNDLE_COMPONENT_STATE_CHANGE,
            [](const EventInfo& eventInfo) {
                InnerSendBundleComponentChangeEvent(eventInfo);
            } },
        { BMSEventType::BUNDLE_CLEAN_CACHE,
            [](const EventInfo& eventInfo) {
                InnerSendBundleCleanCacheEvent(eventInfo);
            } },
    };

void InnerEventReport::SendSystemEvent(BMSEventType bmsEventType, const EventInfo& eventInfo)
{
    auto iter = bmsSysEventMap_.find(bmsEventType);
    if (iter == bmsSysEventMap_.end()) {
        return;
    }

    iter->second(eventInfo);
}

void InnerEventReport::InnerSendBundleInstallExceptionEvent(const EventInfo& eventInfo)
{
    std::string installType = NORMAL_INSTALL_TYPE;
    if (eventInfo.isFreeInstallMode) {
        installType = FREE_INSTALL_TYPE;
    } else if (eventInfo.isPreInstallApp) {
        installType = PRE_BUNDLE_INSTALL_TYPE;
    }

    InnerEventWrite(
        BUNDLE_INSTALL_EXCEPTION,
        HiSysEventType::FAULT,
        EVENT_PARAM_USERID, eventInfo.userId,
        EVENT_PARAM_BUNDLE_NAME, eventInfo.bundleName,
        EVENT_PARAM_INSTALL_TYPE, installType,
        EVENT_PARAM_ERROR_CODE, eventInfo.errCode);
}

void InnerEventReport::InnerSendBundleUninstallExceptionEvent(const EventInfo& eventInfo)
{
    std::string installType = NORMAL_INSTALL_TYPE;
    if (eventInfo.isFreeInstallMode) {
        installType = FREE_INSTALL_TYPE;
    } else if (eventInfo.isPreInstallApp) {
        installType = PRE_BUNDLE_INSTALL_TYPE;
    }

    InnerEventWrite(
        BUNDLE_UNINSTALL_EXCEPTION,
        HiSysEventType::FAULT,
        EVENT_PARAM_USERID, eventInfo.userId,
        EVENT_PARAM_BUNDLE_NAME, eventInfo.bundleName,
        EVENT_PARAM_INSTALL_TYPE, installType,
        EVENT_PARAM_ERROR_CODE, eventInfo.errCode);
}

void InnerEventReport::InnerSendBundleUpdateExceptionEvent(const EventInfo& eventInfo)
{
    std::string installType = NORMAL_INSTALL_TYPE;
    if (eventInfo.isFreeInstallMode) {
        installType = FREE_INSTALL_TYPE;
    } else if (eventInfo.isPreInstallApp) {
        installType = PRE_BUNDLE_INSTALL_TYPE;
    }

    InnerEventWrite(
        BUNDLE_UPDATE_EXCEPTION,
        HiSysEventType::FAULT,
        EVENT_PARAM_USERID, eventInfo.userId,
        EVENT_PARAM_BUNDLE_NAME, eventInfo.bundleName,
        EVENT_PARAM_INSTALL_TYPE, installType,
        EVENT_PARAM_OLD_VERSION, eventInfo.oldVersionCode,
        EVENT_PARAM_NEW_VERSION, eventInfo.curVersionCode,
        EVENT_PARAM_ERROR_CODE, eventInfo.errCode);
}

void InnerEventReport::InnerSendPreBundleRecoverExceptionEvent(const EventInfo& eventInfo)
{
    InnerEventWrite(
        PRE_BUNDLE_RECOVER_EXCEPTION,
        HiSysEventType::FAULT,
        EVENT_PARAM_USERID, eventInfo.userId,
        EVENT_PARAM_BUNDLE_NAME, eventInfo.bundleName,
        EVENT_PARAM_ERROR_CODE, eventInfo.errCode);
}

void InnerEventReport::InnerSendBundleComponentChangeExceptionEvent(const EventInfo& eventInfo)
{
    InnerEventWrite(
        BUNDLE_STATE_CHANGE_EXCEPTION,
        HiSysEventType::FAULT,
        EVENT_PARAM_USERID, eventInfo.userId,
        EVENT_PARAM_BUNDLE_NAME, eventInfo.bundleName,
        EVENT_PARAM_ABILITY_NAME, eventInfo.abilityName);
}

void InnerEventReport::InnerSendBundleCleanCacheExceptionEvent(const EventInfo& eventInfo)
{
    std::string cleanType = eventInfo.isCleanCache ? CLEAN_CACHE : CLEAN_DATA;
    InnerEventWrite(
        BUNDLE_CLEAN_CACHE_EXCEPTION,
        HiSysEventType::FAULT,
        EVENT_PARAM_USERID, eventInfo.userId,
        EVENT_PARAM_BUNDLE_NAME, eventInfo.bundleName,
        EVENT_PARAM_CLEAN_TYPE, cleanType);
}

void InnerEventReport::InnerSendBootScanStartEvent(const EventInfo& eventInfo)
{
    InnerEventWrite(
        BOOT_SCAN_START,
        HiSysEventType::BEHAVIOR,
        EVENT_PARAM_TIME, eventInfo.timeStamp);
}

void InnerEventReport::InnerSendBootScanEndEvent(const EventInfo& eventInfo)
{
    InnerEventWrite(
        BOOT_SCAN_END,
        HiSysEventType::BEHAVIOR,
        EVENT_PARAM_TIME, eventInfo.timeStamp);
}

void InnerEventReport::InnerSendBundleInstallEvent(const EventInfo& eventInfo)
{
    std::string installType = NORMAL_INSTALL_TYPE;
    if (eventInfo.isFreeInstallMode) {
        installType = FREE_INSTALL_TYPE;
    } else if (eventInfo.isPreInstallApp) {
        installType = PRE_BUNDLE_INSTALL_TYPE;
    }

    std::string installPeriod;
    auto iter = INSTALL_PERIOD_STR_MAP.find(eventInfo.preBundlePeriod);
    if (iter != INSTALL_PERIOD_STR_MAP.end()) {
        installPeriod = iter->second;
    }

    InnerEventWrite(
        BUNDLE_INSTALL,
        HiSysEventType::BEHAVIOR,
        EVENT_PARAM_USERID, eventInfo.userId,
        EVENT_PARAM_BUNDLE_NAME, eventInfo.bundleName,
        EVENT_PARAM_VERSION, eventInfo.curVersionCode,
        EVENT_PARAM_INSTALL_TYPE, installType,
        EVENT_PARAM_PERIOD, installPeriod);
}

void InnerEventReport::InnerSendBundleUninstallEvent(const EventInfo& eventInfo)
{
    std::string installType = NORMAL_INSTALL_TYPE;
    if (eventInfo.isFreeInstallMode) {
        installType = FREE_INSTALL_TYPE;
    } else if (eventInfo.isPreInstallApp) {
        installType = PRE_BUNDLE_INSTALL_TYPE;
    }

    InnerEventWrite(
        BUNDLE_UNINSTALL,
        HiSysEventType::BEHAVIOR,
        EVENT_PARAM_USERID, eventInfo.userId,
        EVENT_PARAM_BUNDLE_NAME, eventInfo.bundleName,
        EVENT_PARAM_VERSION, eventInfo.curVersionCode,
        EVENT_PARAM_INSTALL_TYPE, installType);
}

void InnerEventReport::InnerSendBundleUpdateEvent(const EventInfo& eventInfo)
{
    std::string installType = NORMAL_INSTALL_TYPE;
    if (eventInfo.isFreeInstallMode) {
        installType = FREE_INSTALL_TYPE;
    } else if (eventInfo.isPreInstallApp) {
        installType = PRE_BUNDLE_INSTALL_TYPE;
    }

    InnerEventWrite(
        BUNDLE_UPDATE,
        HiSysEventType::BEHAVIOR,
        EVENT_PARAM_USERID, eventInfo.userId,
        EVENT_PARAM_BUNDLE_NAME, eventInfo.bundleName,
        EVENT_PARAM_VERSION, eventInfo.curVersionCode,
        EVENT_PARAM_INSTALL_TYPE, installType);
}

void InnerEventReport::InnerSendPreBundleRecoverEvent(const EventInfo& eventInfo)
{
    InnerEventWrite(
        PRE_BUNDLE_RECOVER,
        HiSysEventType::BEHAVIOR,
        EVENT_PARAM_USERID, eventInfo.userId,
        EVENT_PARAM_BUNDLE_NAME, eventInfo.bundleName,
        EVENT_PARAM_VERSION, eventInfo.curVersionCode);
}

void InnerEventReport::InnerSendBundleComponentChangeEvent(const EventInfo& eventInfo)
{
    std::string state = eventInfo.isEnable ? ENABLE : DISABLE;
    InnerEventWrite(
        BUNDLE_COMPONENT_STATE_CHANGE,
        HiSysEventType::BEHAVIOR,
        EVENT_PARAM_USERID, eventInfo.userId,
        EVENT_PARAM_BUNDLE_NAME, eventInfo.bundleName,
        EVENT_PARAM_ABILITY_NAME, eventInfo.abilityName,
        EVENT_PARAM_STATE, state);
}

void InnerEventReport::InnerSendBundleCleanCacheEvent(const EventInfo& eventInfo)
{
    std::string cleanType = eventInfo.isCleanCache ? CLEAN_CACHE : CLEAN_DATA;
    InnerEventWrite(
        BUNDLE_CLEAN_CACHE,
        HiSysEventType::BEHAVIOR,
        EVENT_PARAM_USERID, eventInfo.userId,
        EVENT_PARAM_BUNDLE_NAME, eventInfo.bundleName,
        EVENT_PARAM_CLEAN_TYPE, cleanType);
}

template<typename... Types>
void InnerEventReport::InnerEventWrite(
    const std::string &eventName,
    HiSysEventType type,
    Types... keyValues)
{
    OHOS::HiviewDFX::HiSysEvent::Write(
        OHOS::HiviewDFX::HiSysEvent::Domain::APPEXECFWK,
        eventName,
        static_cast<OHOS::HiviewDFX::HiSysEvent::EventType>(type),
        keyValues...);
}
}  // namespace AppExecFwk
}  // namespace OHOS