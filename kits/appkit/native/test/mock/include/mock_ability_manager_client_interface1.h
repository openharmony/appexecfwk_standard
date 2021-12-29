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

#ifndef OHOS_AAFWK_MOCK_ABILITY_MANAGER_CLIENT2_H
#define OHOS_AAFWK_MOCK_ABILITY_MANAGER_CLIENT2_H

#include <mutex>

#include "iremote_object.h"
#include "iremote_stub.h"
#include "ability_connect_callback_interface.h"
#include "ability_manager_errors.h"
#include "ability_scheduler_interface.h"
#include "ability_manager_interface.h"
#include "ability_manager_client.h"
#include "ability_context.h"
#include "want.h"

namespace OHOS {
namespace AAFwk {
class MockAbilityManagerClient : public AbilityManagerClient {
public:
    MockAbilityManagerClient();
    virtual ~MockAbilityManagerClient();

    ErrCode GetStartAbility();
    ErrCode GetTerminateAbility();
    ErrCode GetTerminateAbilityResult();
    ErrCode GetIsFirstInMission();
    ErrCode GetRemoveMissions();

    void SetStartAbility(ErrCode tValue);
    void SetTerminateAbility(ErrCode tValue);
    void SetTerminateAbilityResult(ErrCode tValue);
    void SetIsFirstInMission(ErrCode tValue);
    void SetRemoveMissions(ErrCode tValue);

    int GetTerminateAbilityValue();
    void SetTerminateAbilityValue(int nValue);

    int GetRemoveMissionsValue();
    void ChangeRemoveMissionsValue();

    static std::shared_ptr<MockAbilityManagerClient> mock_instance_;
    static bool mock_intanceIsNull_;

    static std::shared_ptr<MockAbilityManagerClient> GetInstance();
    static void SetInstanceNull(bool flag);

private:
    ErrCode startAbility_;
    ErrCode terminateAbility_;
    ErrCode terminateAbilityResult_;
    ErrCode isFirstInMission_;
    ErrCode removeMissionsE_;

    int terminateAbilityValue_;
    int removeMissions_;
};
}  // namespace AAFwk
}  // namespace OHOS

namespace OHOS {
namespace AppExecFwk {
class MockIBundleMgr : public IRemoteStub<IBundleMgr> {
public:
    MockIBundleMgr(){};
    virtual ~MockIBundleMgr(){};

    virtual bool GetApplicationInfo(
        const std::string &appName, const ApplicationFlag flag, const int userId, ApplicationInfo &appInfo)
    {
        return true;
    };
    virtual bool GetApplicationInfos(
        const ApplicationFlag flag, const int userId, std::vector<ApplicationInfo> &appInfos)
    {
        return true;
    };
    virtual bool GetBundleInfo(const std::string &bundleName, const BundleFlag flag, BundleInfo &bundleInfo)
    {
        return true;
    };
    virtual bool GetBundleInfos(const BundleFlag flag, std::vector<BundleInfo> &bundleInfos)
    {
        return true;
    };
    virtual int GetUidByBundleName(const std::string &bundleName, const int userId)
    {
        return 0;
    };
    virtual std::string GetAppIdByBundleName(const std::string &bundleName, const int userId)
    {
        return std::string("");
    };
    virtual bool GetBundleNameForUid(const int uid, std::string &bundleName)
    {
        return true;
    };
    virtual bool GetBundlesForUid(const int uid, std::vector<std::string> &bundleNames)
    {
        return true;
    };
    virtual bool GetNameForUid(const int uid, std::string &name)
    {
        return true;
    };
    virtual bool GetBundleGids(const std::string &bundleName, std::vector<int> &gids)
    {
        return true;
    };
    virtual std::string GetAppType(const std::string &bundleName)
    {
        return std::string("");
    };
    virtual bool CheckIsSystemAppByUid(const int uid)
    {
        return true;
    };
    virtual bool GetBundleInfosByMetaData(const std::string &metaData, std::vector<BundleInfo> &bundleInfos)
    {
        return true;
    };
    virtual bool QueryAbilityInfo(const Want &want, AbilityInfo &abilityInfo)
    {
        return true;
    };
    virtual bool QueryAbilityInfoByUri(const std::string &abilityUri, AbilityInfo &abilityInfo)
    {
        return true;
    };
    virtual bool QueryKeepAliveBundleInfos(std::vector<BundleInfo> &bundleInfos)
    {
        return true;
    };
    virtual std::string GetAbilityLabel(const std::string &bundleName, const std::string &className)
    {
        return std::string("");
    };
    virtual bool GetBundleArchiveInfo(const std::string &hapFilePath, const BundleFlag flag, BundleInfo &bundleInfo)
    {
        return true;
    };
    virtual bool GetHapModuleInfo(const AbilityInfo &abilityInfo, HapModuleInfo &hapModuleInfo)
    {
        return true;
    };
    virtual bool GetLaunchWantForBundle(const std::string &bundleName, Want &want)
    {
        return true;
    };
    virtual int CheckPublicKeys(const std::string &firstBundleName, const std::string &secondBundleName)
    {
        return 0;
    };
    virtual int CheckPermission(const std::string &bundleName, const std::string &permission)
    {
        return 0;
    };
    virtual bool GetPermissionDef(const std::string &permissionName, PermissionDef &permissionDef)
    {
        return true;
    };
    virtual bool GetAllPermissionGroupDefs(std::vector<PermissionDef> &permissionDefs)
    {
        return true;
    };
    virtual bool GetAppsGrantedPermissions(
        const std::vector<std::string> &permissions, std::vector<std::string> &appNames)
    {
        return true;
    };
    virtual bool HasSystemCapability(const std::string &capName)
    {
        return true;
    };
    virtual bool GetSystemAvailableCapabilities(std::vector<std::string> &systemCaps)
    {
        return true;
    };
    virtual bool IsSafeMode()
    {
        return true;
    };
    virtual bool CleanBundleCacheFiles(
        const std::string &bundleName, const sptr<ICleanCacheCallback> &cleanCacheCallback)
    {
        return true;
    };
    virtual bool CleanBundleDataFiles(const std::string &bundleName)
    {
        return true;
    };
    virtual bool RegisterBundleStatusCallback(const sptr<IBundleStatusCallback> &bundleStatusCallback)
    {
        return true;
    };
    virtual bool ClearBundleStatusCallback(const sptr<IBundleStatusCallback> &bundleStatusCallback)
    {
        return true;
    };
    virtual bool UnregisterBundleStatusCallback()
    {
        return true;
    };
    virtual bool DumpInfos(const DumpFlag flag, const std::string &bundleName, std::string &result)
    {
        return true;
    };
    virtual bool IsApplicationEnabled(const std::string &bundleName)
    {
        return true;
    };
    virtual bool SetApplicationEnabled(const std::string &bundleName, bool isEnable)
    {
        return true;
    };
    virtual bool IsAbilityEnabled(const AbilityInfo &abilityInfo)
    {
        return true;
    };
    virtual bool SetAbilityEnabled(const AbilityInfo &abilityInfo, bool isEnabled)
    {
        return true;
    };
    virtual std::string GetAbilityIcon(const std::string &bundleName, const std::string &className)
    {
        return std::string("");
    };
    virtual bool CanRequestPermission(
        const std::string &bundleName, const std::string &permissionName, const int userId)
    {
        return true;
    };
    virtual bool RequestPermissionFromUser(
        const std::string &bundleName, const std::string &permission, const int userId)
    {
        return true;
    };
    virtual bool RegisterAllPermissionsChanged(const sptr<OnPermissionChangedCallback> &callback)
    {
        return true;
    };
    virtual bool RegisterPermissionsChanged(
        const std::vector<int> &uids, const sptr<OnPermissionChangedCallback> &callback)
    {
        return true;
    };
    virtual bool UnregisterPermissionsChanged(const sptr<OnPermissionChangedCallback> &callback)
    {
        return true;
    };
    virtual bool GetAllFormsInfo(std::vector<FormInfo> &formInfos)
    {
        return true;
    };
    virtual bool GetFormsInfoByApp(const std::string &bundleName, std::vector<FormInfo> &formInfos)
    {
        return true;
    };
    virtual bool GetFormsInfoByModule(
        const std::string &bundleName, const std::string &moduleName, std::vector<FormInfo> &formInfos)
    {
        return true;
    };
    virtual sptr<IBundleInstaller> GetBundleInstaller()
    {
        return nullptr;
    };
    bool QueryAbilityInfos(const Want &want, std::vector<AbilityInfo> &abilityInfos)
    {
        return true;
    }
    bool GetShortcutInfos(const std::string &bundleName, std::vector<ShortcutInfo> &shortcutInfos)
    {
        return true;
    }
    bool GetModuleUsageRecords(const int32_t number, std::vector<ModuleUsageRecord> &moduleUsageRecords)
    {
        return true;
    }
    bool NotifyAbilityLifeStatus(
        const std::string &bundleName, const std::string &abilityName, const int64_t launchTime)
    {
        return true;
    }
};

class MockAbilityContextDeal : public ContextDeal {
public:
    MockAbilityContextDeal(){};
    virtual ~MockAbilityContextDeal(){};

    sptr<IBundleMgr> GetBundleManager() const override
    {
        return sptr<IBundleMgr>(new (std::nothrow) MockIBundleMgr());
    };
};
}  // namespace AppExecFwk
}  // namespace OHOS

#endif  // OHOS_AAFWK_ABILITY_MANAGER_H
