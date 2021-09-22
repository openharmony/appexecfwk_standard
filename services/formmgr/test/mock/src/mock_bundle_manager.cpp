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
#include "form_info.h"

namespace OHOS {
namespace AppExecFwk {
const std::string FORM_PROVIDER_BUNDLE_NAME = "com.form.provider.service";
const std::string PARAM_PROVIDER_PACKAGE_NAME = "com.form.provider.app.test.abiliy";
const std::string PARAM_PROVIDER_MODULE_NAME = "com.form.provider.app.test.abiliy";
const std::string FORM_PROVIDER_ABILITY_NAME = "com.form.provider.app.test.abiliy";
const std::string FORM_PROVIDER_MODULE_SOURCE_DIR = "";
const std::string FORM_JS_COMPOMENT_NAME = "jsComponentName";
const std::string PARAM_FORM_NAME = "com.form.name.test";
const std::string DEVICE_ID = "ohos-phone1";

bool BundleMgrProxy::GetBundleInfo(const std::string &bundleName, const BundleFlag flag, BundleInfo &bundleInfo)
{
    return true;
}

int BundleMgrService::GetUidByBundleName(const std::string &bundleName, const int userId)
{
    if (bundleName.compare("com.form.host.app600") == 0) {
        return 600;
    }
    return 0;
}

bool BundleMgrProxy::GetAllFormsInfo(std::vector<FormInfo> &formInfo)
{
    return true;
}
bool BundleMgrProxy::GetFormsInfoByApp(const std::string &bundleName, std::vector<FormInfo> &formInfo)
{
    return true;
}
bool BundleMgrProxy::GetFormsInfoByModule(const std::string &bundleName, const std::string &moduleName, 
    std::vector<FormInfo> &formInfo)
{
    return true;
}

int BundleMgrStub::OnRemoteRequest(uint32_t code, MessageParcel &data, MessageParcel &reply, MessageOption &option)
{
    return 0;
}

bool BundleMgrService::QueryAbilityInfo(const AAFwk::Want &want, AbilityInfo &abilityInfo)
{
    return true;
}

bool BundleMgrService::QueryAbilityInfoByUri(const std::string &uri, AbilityInfo &abilityInfo)
{
    return false;
}

bool BundleMgrService::GetApplicationInfo(
    const std::string &appName, const ApplicationFlag flag, const int userId, ApplicationInfo &appInfo)
{
    return true;
}

std::string BundleMgrService::GetAppType(const std::string &bundleName)
{
    return "system";
}

bool BundleMgrService::GetBundleInfo(const std::string &bundleName, const BundleFlag flag, BundleInfo &bundleInfo)
{
    std::vector<AbilityInfo> abilityInfos;
    ApplicationInfo applicationInfo;
    ModuleInfo moduleInfo;
    
    moduleInfo.moduleSourceDir = FORM_PROVIDER_MODULE_SOURCE_DIR;
    moduleInfo.moduleName = PARAM_PROVIDER_MODULE_NAME;
    bundleInfo.name = bundleName;
    applicationInfo.bundleName = bundleName;
    applicationInfo.moduleInfos.emplace_back(moduleInfo);
    bundleInfo.applicationInfo = applicationInfo;

    bundleInfo.moduleNames.emplace_back(PARAM_PROVIDER_MODULE_NAME);

    AbilityInfo abilityInfo;
    abilityInfo.name = FORM_PROVIDER_ABILITY_NAME;
    abilityInfo.package = PARAM_PROVIDER_PACKAGE_NAME;
    abilityInfo.moduleName = PARAM_PROVIDER_MODULE_NAME;
    abilityInfo.deviceId = DEVICE_ID;
    bundleInfo.abilityInfos.emplace_back(abilityInfo);

    return true;
}
bool BundleMgrService::GetBundleGids(const std::string &bundleName, std::vector<int> &gids)
{
    return true;
}

bool BundleMgrService::GetAllFormsInfo(std::vector<FormInfo> &formInfo)
{
    return true;
}
bool BundleMgrService::GetFormsInfoByApp(const std::string &bundleName, std::vector<FormInfo> &formInfo)
{
    FormInfo form;
    form.bundleName = bundleName;
    form.abilityName = FORM_PROVIDER_ABILITY_NAME;
    form.moduleName = PARAM_PROVIDER_MODULE_NAME;
    form.name = PARAM_FORM_NAME;
    form.updateEnabled = true;
    form.updateDuration = 1;
    form.scheduledUpateTime = "06:06";
    form.jsComponentName = FORM_JS_COMPOMENT_NAME;
    form.formVisibleNotify = true;
    form.supportDimensions = {1, 2};
    form.defaultDimension = 1;
    formInfo.emplace_back(form);
    return true;
}
bool BundleMgrService::GetFormsInfoByModule(const std::string &bundleName, const std::string &moduleName, 
    std::vector<FormInfo> &formInfo)
{
    FormInfo form;
    form.bundleName = bundleName;
    form.abilityName = FORM_PROVIDER_ABILITY_NAME;
    form.moduleName = PARAM_PROVIDER_MODULE_NAME;
    form.name = PARAM_FORM_NAME;
    form.updateEnabled = true;
    form.updateDuration = 1;
    form.scheduledUpateTime = "06:06";
    form.jsComponentName = FORM_JS_COMPOMENT_NAME;
    form.formVisibleNotify = true;
    form.supportDimensions = {1, 2};
    form.defaultDimension = 1;
    formInfo.emplace_back(form);
    return true;
}
}  // namespace AppExecFwk
}  // namespace OHOS
