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

#ifndef OHOS_AAFWK_ABILITY_MOCK_BUNDLE_MANAGER_H
#define OHOS_AAFWK_ABILITY_MOCK_BUNDLE_MANAGER_H

#include <vector>

#include "ability_info.h"
#include "application_info.h"
#include "bundle_mgr_interface.h"
#include "gmock/gmock.h"
#include "iremote_proxy.h"
#include "iremote_stub.h"
#include "ohos/aafwk/content/want.h"

namespace OHOS {
namespace AppExecFwk {
class BundleMgrProxy : public IRemoteProxy<IBundleMgr> {
public:
    explicit BundleMgrProxy(const sptr<IRemoteObject> &impl) : IRemoteProxy<IBundleMgr>(impl)
    {}
    virtual ~BundleMgrProxy()
    {}
    MOCK_METHOD3(
        CanRequestPermission, bool(const std::string &bundleName, const std::string &permissionName, const int userId));
    MOCK_METHOD3(RequestPermissionFromUser,
        bool(const std::string &bundleName, const std::string &permission, const int userId));
    MOCK_METHOD2(GetNameForUid, bool(const int uid, std::string &name));
    MOCK_METHOD2(GetBundlesForUid, bool(const int uid, std::vector<std::string> &));
    MOCK_METHOD2(SetAbilityEnabled, bool(const AbilityInfo &, bool));
    MOCK_METHOD1(IsAbilityEnabled, bool(const AbilityInfo &));
    MOCK_METHOD2(GetAbilityIcon, std::string(const std::string &bundleName, const std::string &className));
    MOCK_METHOD1(RegisterAllPermissionsChanged, bool(const sptr<OnPermissionChangedCallback> &callback));
    MOCK_METHOD2(RegisterPermissionsChanged,
        bool(const std::vector<int> &uids, const sptr<OnPermissionChangedCallback> &callback));
    MOCK_METHOD1(UnregisterPermissionsChanged, bool(const sptr<OnPermissionChangedCallback> &callback));
    bool QueryAbilityInfo(const AAFwk::Want &want, AbilityInfo &abilityInfo) override
    {
        return true;
    }
    bool QueryAbilityInfoByUri(const std::string &uri, AbilityInfo &abilityInfo) override
    {
        return true;
    }

    std::string GetAppType(const std::string &bundleName) override
    {
        return "system";
    }

    virtual bool GetApplicationInfo(const std::string &appName, const ApplicationFlag flag, const int userId,
    ApplicationInfo &appInfo) override
    {
        return true;
    }
    virtual bool GetApplicationInfos(
        const ApplicationFlag flag, const int userId, std::vector<ApplicationInfo> &appInfos) override
    {
        return true;
    }

    virtual bool GetBundleInfos(const BundleFlag flag, std::vector<BundleInfo> &bundleInfos) override
    {
        return true;
    }
    virtual int GetUidByBundleName(const std::string &bundleName, const int userId) override
    {
        if (bundleName.compare("com.form.host.app600") == 0) {
            return 600;
        }
        return 0;
    }
    virtual std::string GetAppIdByBundleName(const std::string &bundleName, const int userId) override
    {
        return "";
    }
    virtual bool GetBundleNameForUid(const int uid, std::string &bundleName) override
    {
        bundleName = "com.form.provider.service";
        return true;
    }
    virtual bool GetBundleGids(const std::string &bundleName, std::vector<int> &gids) override
    {
        return true;
    }
    virtual bool GetBundleInfosByMetaData(const std::string &metaData, std::vector<BundleInfo> &bundleInfos) override
    {
        return true;
    }
    virtual bool QueryKeepAliveBundleInfos(std::vector<BundleInfo> &bundleInfos) override
    {
        return true;
    }
    virtual std::string GetAbilityLabel(const std::string &bundleName, const std::string &className) override
    {
        return "";
    }

    virtual bool GetBundleArchiveInfo(
        const std::string &hapFilePath, const BundleFlag flag, BundleInfo &bundleInfo) override
    {
        return true;
    }

    virtual bool GetLaunchWantForBundle(const std::string &bundleName, Want &want) override
    {
        return true;
    }

    virtual int CheckPublicKeys(const std::string &firstBundleName, const std::string &secondBundleName) override
    {
        return 0;
    }

    virtual int CheckPermission(const std::string &bundleName, const std::string &permission) override
    {
        return 0;
    }
    virtual bool GetPermissionDef(const std::string &permissionName, PermissionDef &permissionDef) override
    {
        return true;
    }
    virtual bool GetAllPermissionGroupDefs(std::vector<PermissionDef> &permissionDefs) override
    {
        return true;
    }
    virtual bool GetAppsGrantedPermissions(
        const std::vector<std::string> &permissions, std::vector<std::string> &appNames) override
    {
        return true;
    }
    virtual bool HasSystemCapability(const std::string &capName) override
    {
        return true;
    }
    virtual bool GetSystemAvailableCapabilities(std::vector<std::string> &systemCaps) override
    {
        return true;
    }
    virtual bool IsSafeMode() override
    {
        return true;
    }
    // clears cache data of a specified application.
    virtual bool CleanBundleCacheFiles(
        const std::string &bundleName, const sptr<ICleanCacheCallback> &cleanCacheCallback) override
    {
        return true;
    }
    virtual bool CleanBundleDataFiles(const std::string &bundleName) override
    {
        return true;
    }
    virtual bool RegisterBundleStatusCallback(const sptr<IBundleStatusCallback> &bundleStatusCallback) override
    {
        return true;
    }
    virtual bool ClearBundleStatusCallback(const sptr<IBundleStatusCallback> &bundleStatusCallback) override
    {
        return true;
    }
    // unregister callback of all application
    virtual bool UnregisterBundleStatusCallback() override
    {
        return true;
    }
    virtual bool DumpInfos(const DumpFlag flag, const std::string &bundleName, std::string &result) override
    {
        return true;
    }
    virtual sptr<IBundleInstaller> GetBundleInstaller() override
    {
        return nullptr;
    }

    /**
	 *  @brief Obtains information about the shortcuts of the application.
     *  @param bundleName Indicates the name of the bundle to shortcut.
     *  @param form Indicates the callback a list to shortcutinfo.
     *  @return Returns true if shortcutinfo get success
     */
    virtual bool GetShortcutInfos(const std::string &bundleName,std::vector<ShortcutInfo> &shortcut) override{
        return true;
    }

    /**
     * @brief Obtain the HAP module info of a specific ability.
     * @param abilityInfo Indicates the ability.
     * @param hapModuleInfo Indicates the obtained HapModuleInfo object.
     * @return Returns true if the HapModuleInfo is successfully obtained; returns false otherwise.
     */
    virtual bool GetHapModuleInfo(const AbilityInfo &abilityInfo, HapModuleInfo &hapModuleInfo) override
    {
        return true;
    }

    virtual bool CheckIsSystemAppByUid(const int uid) override
    {
        if (uid == 600) {
            return false;
        }
        return true;
    }
    virtual bool IsApplicationEnabled(const std::string &bundleName)override
    {
        return true;
    }
    virtual bool SetApplicationEnabled(const std::string &bundleName, bool isEnable) override
    {
        return true;
    }

    virtual bool GetBundleInfo(const std::string &bundleName, const BundleFlag flag, BundleInfo &bundleInfo) override;
    virtual bool GetAllFormsInfo(std::vector<FormInfo> &formInfo) override;
    virtual bool GetFormsInfoByApp(const std::string &bundleName, std::vector<FormInfo> &formInfo) override;
    virtual bool GetFormsInfoByModule(const std::string &bundleName, const std::string &moduleName,
    std::vector<FormInfo> &formInfo) override;
};

class BundleMgrStub : public IRemoteStub<IBundleMgr> {
public:
    DECLARE_INTERFACE_DESCRIPTOR(u"IBundleMgr");
    virtual int OnRemoteRequest(
        uint32_t code, MessageParcel &data, MessageParcel &reply, MessageOption &option) override;
};

class BundleMgrService : public BundleMgrStub {
public:
    MOCK_METHOD2(GetAppIdByBundleName, std::string(const std::string &bundleName, const int userId));
    MOCK_METHOD2(CheckPermission, int(const std::string &bundleName, const std::string &permission));
    MOCK_METHOD1(CleanBundleDataFiles, bool(const std::string &bundleName));
    MOCK_METHOD3(
        CanRequestPermission, bool(const std::string &bundleName, const std::string &permissionName, const int userId));
    MOCK_METHOD3(RequestPermissionFromUser,
        bool(const std::string &bundleName, const std::string &permission, const int userId));
    MOCK_METHOD2(GetNameForUid, bool(const int uid, std::string &name));
    MOCK_METHOD2(GetBundlesForUid, bool(const int uid, std::vector<std::string> &));
    MOCK_METHOD2(SetAbilityEnabled, bool(const AbilityInfo &, bool));
    MOCK_METHOD1(IsAbilityEnabled, bool(const AbilityInfo &));
    MOCK_METHOD2(GetAbilityIcon, std::string(const std::string &bundleName, const std::string &className));
    MOCK_METHOD1(RegisterAllPermissionsChanged, bool(const sptr<OnPermissionChangedCallback> &callback));
    MOCK_METHOD2(RegisterPermissionsChanged,
        bool(const std::vector<int> &uids, const sptr<OnPermissionChangedCallback> &callback));
    MOCK_METHOD1(UnregisterPermissionsChanged, bool(const sptr<OnPermissionChangedCallback> &callback));
    bool QueryAbilityInfo(const AAFwk::Want &want, AbilityInfo &abilityInfo) override;
    bool QueryAbilityInfoByUri(const std::string &uri, AbilityInfo &abilityInfo) override;

    std::string GetAppType(const std::string &bundleName) override;
    virtual int GetUidByBundleName(const std::string &bundleName, const int userId) override;

    virtual bool GetApplicationInfo(
        const std::string &appName, const ApplicationFlag flag, const int userId, ApplicationInfo &appInfo) override;
    virtual bool GetApplicationInfos(
        const ApplicationFlag flag, const int userId, std::vector<ApplicationInfo> &appInfos) override
    {
        return true;
    };
    virtual bool GetBundleInfo(const std::string &bundleName, const BundleFlag flag, BundleInfo &bundleInfo) override;
    virtual bool GetBundleInfos(const BundleFlag flag, std::vector<BundleInfo> &bundleInfos) override
    {
        return true;
    };
    virtual bool GetBundleNameForUid(const int uid, std::string &bundleName) override
    {
        bundleName = "com.form.provider.service";
        return true;
    };
    virtual bool GetBundleGids(const std::string &bundleName, std::vector<int> &gids) override;
    virtual bool GetBundleInfosByMetaData(const std::string &metaData, std::vector<BundleInfo> &bundleInfos) override
    {
        return true;
    };
    virtual bool QueryKeepAliveBundleInfos(std::vector<BundleInfo> &bundleInfos) override
    {
        return true;
    };
    virtual std::string GetAbilityLabel(const std::string &bundleName, const std::string &className) override
    {
        return "";
    };
    // obtains information about an application bundle contained in a ohos Ability Package (HAP).
    virtual bool GetBundleArchiveInfo(
        const std::string &hapFilePath, const BundleFlag flag, BundleInfo &bundleInfo) override
    {
        return true;
    };
    virtual bool GetHapModuleInfo(const AbilityInfo &abilityInfo, HapModuleInfo &hapModuleInfo) override
    {
        return true;
    }
    // obtains the Want for starting the main ability of an application based on the given bundle name.
    virtual bool GetLaunchWantForBundle(const std::string &bundleName, Want &want) override
    {
        return true;
    };
    // checks whether the publickeys of two bundles are the same.
    virtual int CheckPublicKeys(const std::string &firstBundleName, const std::string &secondBundleName) override
    {
        return 0;
    };
    // checks whether a specified bundle has been granted a specific permission.
    virtual bool GetPermissionDef(const std::string &permissionName, PermissionDef &permissionDef) override
    {
        return true;
    };
    virtual bool GetAllPermissionGroupDefs(std::vector<PermissionDef> &permissionDefs) override
    {
        return true;
    };
    virtual bool GetAppsGrantedPermissions(
        const std::vector<std::string> &permissions, std::vector<std::string> &appNames) override
    {
        return true;
    };
    virtual bool HasSystemCapability(const std::string &capName) override
    {
        return true;
    };
    virtual bool GetSystemAvailableCapabilities(std::vector<std::string> &systemCaps) override
    {
        return true;
    };
    virtual bool IsSafeMode() override
    {
        return true;
    };
    // clears cache data of a specified application.
    virtual bool CleanBundleCacheFiles(
        const std::string &bundleName, const sptr<ICleanCacheCallback> &cleanCacheCallback) override
    {
        return true;
    };

    virtual bool RegisterBundleStatusCallback(const sptr<IBundleStatusCallback> &bundleStatusCallback) override
    {
        return true;
    };
    virtual bool ClearBundleStatusCallback(const sptr<IBundleStatusCallback> &bundleStatusCallback) override
    {
        return true;
    };
    // unregister callback of all application
    virtual bool UnregisterBundleStatusCallback() override
    {
        return true;
    };
    virtual bool DumpInfos(const DumpFlag flag, const std::string &bundleName, std::string &result) override
    {
        return true;
    };
    virtual sptr<IBundleInstaller> GetBundleInstaller() override
    {
        return nullptr;
    };
    virtual bool IsApplicationEnabled(const std::string &bundleName) override
    {
        return true;
    };
    virtual bool CheckIsSystemAppByUid(const int uid) override
    {
        if (uid == 600) {
            return false;
        }

        return true;
    };
    virtual bool SetApplicationEnabled(const std::string &bundleName, bool isEnable) override
    {
        return true;
    };

    /**
	 *  @brief Obtains information about the shortcuts of the application.
     *  @param bundleName Indicates the name of the bundle to shortcut.
     *  @param form Indicates the callback a list to shortcutinfo.
     *  @return Returns true if shortcutinfo get success
     */
    virtual bool GetShortcutInfos(const std::string &bundleName,std::vector<ShortcutInfo> &shortcut) override{
        return true;
    }

    virtual bool GetAllFormsInfo(std::vector<FormInfo> &formInfo) override;
    virtual bool GetFormsInfoByApp(const std::string &bundleName, std::vector<FormInfo> &formInfo) override;
    virtual bool GetFormsInfoByModule(const std::string &bundleName, const std::string &moduleName,
    std::vector<FormInfo> &formInfo) override;
    virtual bool QueryAbilityInfos(const Want &want, std::vector<AbilityInfo> &abilityInfos) override
    {
         return true;
    };
    virtual bool GetModuleUsageRecords(
        const int32_t number, std::vector<ModuleUsageRecord> &moduleUsageRecords) override
    {
        return true;
    }
    virtual bool NotifyActivityLifeStatus(
        const std::string &bundleName, const std::string &abilityName, const int64_t launchTime) override
    {
        return true;
    }
};
}  // namespace AppExecFwk
}  // namespace OHOS

#endif  // OHOS_AAFWK_ABILITY_MOCK_BUNDLE_MANAGER_H
