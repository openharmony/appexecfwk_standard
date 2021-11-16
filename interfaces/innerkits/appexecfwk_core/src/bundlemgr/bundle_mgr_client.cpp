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

#include "app_log_wrapper.h"
#include "bundle_mgr_interface.h"
#include "iservice_registry.h"
#include "system_ability_definition.h"

#include "bundle_mgr_client.h"

namespace OHOS {
namespace AppExecFwk {
BundleMgrClient::BundleMgrClient()
{
    APP_LOGI("enter");
}

BundleMgrClient::~BundleMgrClient()
{
    APP_LOGI("enter");
}

bool BundleMgrClient::GetBundleNameForUid(const int uid, std::string &bundleName)
{
    APP_LOGI("enter");

    ErrCode result = Connect();
    if (result != ERR_OK) {
        APP_LOGE("failed to connect");
        return false;
    }

    auto bundleMgrProxy = iface_cast<IBundleMgr>(remoteObject_);
    if (bundleMgrProxy == nullptr) {
        APP_LOGE("failed to get bundle mgr proxy");
        return false;
    }

    return bundleMgrProxy->GetBundleNameForUid(uid, bundleName);
}

bool BundleMgrClient::GetBundleInfo(const std::string &bundleName, const BundleFlag flag, BundleInfo &bundleInfo)
{
    APP_LOGI("enter");

    ErrCode result = Connect();
    if (result != ERR_OK) {
        APP_LOGE("failed to connect");
        return false;
    }

    auto bundleMgrProxy = iface_cast<IBundleMgr>(remoteObject_);
    if (bundleMgrProxy == nullptr) {
        APP_LOGE("failed to get bundle mgr proxy");
        return false;
    }

    return bundleMgrProxy->GetBundleInfo(bundleName, flag, bundleInfo);
}

ErrCode BundleMgrClient::Connect()
{
    APP_LOGI("enter");

    std::lock_guard<std::mutex> lock(mutex_);
    if (remoteObject_ == nullptr) {
        sptr<ISystemAbilityManager> systemAbilityManager =
            SystemAbilityManagerClient::GetInstance().GetSystemAbilityManager();
        if (systemAbilityManager == nullptr) {
            APP_LOGE("failed to get system ability manager");
            return ERR_APPEXECFWK_SERVICE_NOT_CONNECTED;
        }

        remoteObject_ = systemAbilityManager->GetSystemAbility(BUNDLE_MGR_SERVICE_SYS_ABILITY_ID);
        if (remoteObject_ == nullptr) {
            APP_LOGE("failed to get bundle mgr service remote object");
            return ERR_APPEXECFWK_SERVICE_NOT_CONNECTED;
        }
    }

    APP_LOGI("end");

    return ERR_OK;
}
}  // namespace AppExecFwk
}  // namespace OHOS
