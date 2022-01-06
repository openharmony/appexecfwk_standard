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

#include <gtest/gtest.h>
#include "mock_bundle_manager.h"

#include "ability_info.h"
#include "application_info.h"

namespace OHOS {
namespace AppExecFwk {
BundleMgrService::BundleMgrService()
{
    MakingPackageData();
}

bool BundleMgrProxy::QueryAbilityInfo(const AAFwk::Want &want, AbilityInfo &abilityInfo)
{
    ElementName eleName = want.GetElement();
    if (eleName.GetBundleName().empty()) {
        return false;
    }
    abilityInfo.visible = true;
    abilityInfo.name = eleName.GetAbilityName();
    abilityInfo.bundleName = eleName.GetBundleName();
    abilityInfo.applicationName = "Helloworld";
    return true;
}

bool BundleMgrProxy::QueryAbilityInfoByUri(const std::string &uri, AbilityInfo &abilityInfo)
{
    return false;
}

bool BundleMgrProxy::GetApplicationInfo(
    const std::string &appName, const ApplicationFlag flag, const int userId, ApplicationInfo &appInfo)
{
    if (appName.empty()) {
        return false;
    }
    appInfo.name = "Helloworld";
    appInfo.bundleName = "com.ohos.hiworld";
    return true;
}

std::string BundleMgrProxy::GetAppType(const std::string &bundleName)
{
    GTEST_LOG_(INFO) << " BundleMgrProxy::GetAppTyp";
    return "system";
}

int BundleMgrStub::OnRemoteRequest(uint32_t code, MessageParcel &data, MessageParcel &reply, MessageOption &option)
{
    return 0;
}

bool BundleMgrService::QueryAbilityInfo(const AAFwk::Want &want, AbilityInfo &abilityInfo)
{
    ElementName elementName = want.GetElement();
    if (elementName.GetBundleName().empty()) {
        return false;
    }
    if (std::string::npos != elementName.GetBundleName().find("service")) {
        abilityInfo.type = AppExecFwk::AbilityType::SERVICE;
    }
    abilityInfo.visible = true;
    abilityInfo.name = elementName.GetAbilityName();
    abilityInfo.bundleName = elementName.GetBundleName();
    abilityInfo.applicationName = elementName.GetBundleName();
    if (want.HasEntity(Want::ENTITY_HOME) && want.GetAction() == Want::ACTION_HOME) {
        abilityInfo.applicationInfo.isLauncherApp = true;
    } else {
        abilityInfo.applicationInfo.isLauncherApp = false;
    }
    return true;
}

bool BundleMgrService::QueryAbilityInfoByUri(const std::string &uri, AbilityInfo &abilityInfo)
{
    return false;
}

bool BundleMgrService::GetApplicationInfo(
    const std::string &appName, const ApplicationFlag flag, const int userId, ApplicationInfo &appInfo)
{
    if (appName.empty()) {
        return false;
    }
    appInfo.name = "Helloworld";
    appInfo.bundleName = "com.ohos.hiworld";
    return true;
}

std::string BundleMgrService::GetAppType(const std::string &bundleName)
{
    GTEST_LOG_(INFO) << " BundleMgrService::GetAppType";
    return "system";
}

bool BundleMgrService::GetHapModuleInfo(const AbilityInfo &abilityInfo, HapModuleInfo &hapModuleInfo)
{
    GTEST_LOG_(INFO) << " BundleMgrService::GetHapModuleInfo";
    hapModuleInfo.name = "Captain";
    return true;
}

bool BundleMgrProxy::GetHapModuleInfo(const AbilityInfo &abilityInfo, HapModuleInfo &hapModuleInfo)
{
    GTEST_LOG_(INFO) << " BundleMgrService::GetHapModuleInfo";
    hapModuleInfo.name = "Captain";
    return true;
}

bool BundleMgrService::GetBundleInfo(
    const std::string &bundleName, const BundleFlag flag, BundleInfo &bundleInfo, int32_t userId)
{
    int32_t userUid = 10001;
    int32_t userGid = 10001;
    bundleInfo.uid = userUid;
    bundleInfo.gid = userGid;
    return true;
}
bool BundleMgrService::GetBundleGids(const std::string &bundleName, std::vector<int> &gids)
{
    int32_t userGid1 = 10001;
    int32_t userGid2 = 10002;
    int32_t userGid3 = 10003;
    gids.push_back(userGid1);
    gids.push_back(userGid2);
    gids.push_back(userGid3);
    return true;
}

bool BundleMgrService::GetBundleInfos(
    const BundleFlag flag, std::vector<BundleInfo> &bundleInfos, int32_t userId)
{
    bundleInfos = bundleInfos_;
    return true;
}

bool BundleMgrService::GetBundleGidsByUid(
    const std::string &bundleName, const int &uid, std::vector<int> &gids)
{
    return true;
}

void BundleMgrService::PushTestHelloIndexAbility(int index)
{
    AbilityInfo info;
    info.name = "com.ohos.test.helloworld.MainAbility";
    info.bundleName = COM_OHOS_HELLO + std::to_string(index);
    info.applicationInfo.bundleName = COM_OHOS_HELLO + std::to_string(index);
    info.applicationName = "helloworld";
    info.applicationInfo.name = "helloworld";
    info.process = "p1";
    info.applicationInfo.uid = -1;
    info.deviceId = "deviceId";
    info.visible = true;

    BundleInfo bundleInfo;
    bundleInfo.name = COM_OHOS_HELLO + std::to_string(index);
    bundleInfo.uid = info.applicationInfo.uid;
    bundleInfo.abilityInfos.emplace_back(info);
    bundleInfo.applicationInfo = info.applicationInfo;
    bundleInfos_.emplace_back(bundleInfo);
}

void BundleMgrService::PushTestSpecialAbility()
{
    AbilityInfo info;
    info.name = "com.ohos.test.helloworld.MainAbility";
    info.bundleName = COM_OHOS_SPECIAL;
    info.applicationInfo.bundleName = COM_OHOS_SPECIAL;
    info.applicationName = "helloworld";
    info.applicationInfo.name = "helloworld";
    info.process = "p1";
    info.applicationInfo.uid = -1;
    info.deviceId = "deviceId";
    info.visible = true;

    BundleInfo bundleInfo;
    bundleInfo.name = COM_OHOS_SPECIAL;
    bundleInfo.uid = info.applicationInfo.uid;
    bundleInfo.abilityInfos.emplace_back(info);
    bundleInfo.applicationInfo = info.applicationInfo;
    bundleInfos_.emplace_back(bundleInfo);
}

void BundleMgrService::PushTestHelloAbility()
{
    AbilityInfo info;
    info.name = "com.ohos.test.helloworld.MainAbility";
    info.bundleName = COM_OHOS_HELLO;
    info.applicationInfo.bundleName = COM_OHOS_HELLO;
    info.applicationName = "helloworld";
    info.applicationInfo.name = "helloworld";
    info.process = "p1";
    info.applicationInfo.uid = -1;
    info.deviceId = "deviceId";
    info.visible = true;

    BundleInfo bundleInfo;
    bundleInfo.name = COM_OHOS_HELLO;
    bundleInfo.uid = info.applicationInfo.uid;
    bundleInfo.abilityInfos.emplace_back(info);
    bundleInfo.applicationInfo = info.applicationInfo;
    bundleInfos_.emplace_back(bundleInfo);
}

void BundleMgrService::MakingPackageData()
{
    PushTestSpecialAbility();
    PushTestHelloAbility();
    MakingResidentProcData();
    for (int i = 0; i<= APPLICATION_NUMHELLO; i++) {
        PushTestHelloIndexAbility(i);
    }
}

void BundleMgrService::MakingResidentProcData()
{
    int appUid = 2100;
    GTEST_LOG_(INFO) << "MakingResidentProcData()";
    BundleInfo bundleInfo;
    bundleInfo.uid = appUid;
    bundleInfo.name = "KeepAliveApplication";

    bundleInfos_.emplace_back(bundleInfo);
}
}  // namespace AppExecFwk
}  // namespace OHOS
