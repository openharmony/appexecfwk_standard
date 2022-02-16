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

#include "bundle_user_mgr_host_impl.h"

#include "app_log_wrapper.h"
#include "bundle_mgr_service.h"
#include "bundle_promise.h"
#include "bundle_util.h"
#include "bytrace.h"
#include "status_receiver_host.h"

namespace OHOS {
namespace AppExecFwk {
std::atomic_int g_installedHapNum = 0;

class UserReceiverImpl : public StatusReceiverHost {
public:
    UserReceiverImpl() = default;
    virtual ~UserReceiverImpl() override = default;

    void SetBundlePromise(const std::shared_ptr<BundlePromise>& bundlePromise)
    {
        bundlePromise_ = bundlePromise;
    }

    void SetTotalHapNum(int32_t totalHapNum)
    {
        totalHapNum_ = totalHapNum;
    }

    virtual void OnStatusNotify(const int progress) override {}
    virtual void OnFinished(const int32_t resultCode, const std::string &resultMsg) override
    {
        g_installedHapNum++;
        APP_LOGD("OnFinished::resultCode:(%{public}d) resultMsg:(%{public}s).",
            resultCode, resultMsg.c_str());
        if (g_installedHapNum >= totalHapNum_ && bundlePromise_ != nullptr) {
            bundlePromise_->NotifyAllTasksExecuteFinished();
        }
    }
private:
    std::shared_ptr<BundlePromise> bundlePromise_ = nullptr;
    int32_t totalHapNum_ = INT32_MAX;
};

void BundleUserMgrHostImpl::CreateNewUser(int32_t userId)
{
    BYTRACE(BYTRACE_TAG_APP);
    APP_LOGD("CreateNewUser user(%{public}d) start.", userId);
    std::lock_guard<std::mutex> lock(bundleUserMgrMutex_);
    CheckInitialUser();
    auto dataMgr = GetDataMgrFromService();
    if (dataMgr == nullptr) {
        APP_LOGE("DataMgr is nullptr");
        return;
    }

    auto installer = GetBundleInstaller();
    if (installer == nullptr) {
        APP_LOGE("installer is nullptr");
        return;
    }

    if (dataMgr->HasUserId(userId)) {
        APP_LOGE("Has create user %{public}d.", userId);
        return;
    }

    dataMgr->AddUserId(userId);
    // Scan preset applications and parse package information.
    std::vector<PreInstallBundleInfo> preInstallBundleInfos;
    if (!dataMgr->LoadAllPreInstallBundleInfos(preInstallBundleInfos)) {
        APP_LOGE("LoadAllPreInstallBundleInfos failed.");
        return;
    }

    g_installedHapNum = 0;
    std::shared_ptr<BundlePromise> bundlePromise = std::make_shared<BundlePromise>();
    int32_t totalHapNum = preInstallBundleInfos.size();
    // Read apps installed by other users that are visible to all users
    for (const auto &info : preInstallBundleInfos) {
        std::vector<std::string> pathVec { info.GetBundlePaths() };
        InstallParam installParam;
        installParam.userId = userId;
        installParam.isPreInstallApp = true;
        installParam.installFlag = InstallFlag::NORMAL;
        sptr<UserReceiverImpl> userReceiverImpl(new (std::nothrow) UserReceiverImpl());
        userReceiverImpl->SetBundlePromise(bundlePromise);
        userReceiverImpl->SetTotalHapNum(totalHapNum);
        installer->Install(pathVec, installParam, userReceiverImpl);
    }

    if (g_installedHapNum < totalHapNum) {
        bundlePromise->WaitForAllTasksExecute();
    }
    APP_LOGD("CreateNewUser end userId: (%{public}d)", userId);
}

void BundleUserMgrHostImpl::RemoveUser(int32_t userId)
{
    BYTRACE(BYTRACE_TAG_APP);
    APP_LOGD("RemoveUser user(%{public}d) start.", userId);
    std::lock_guard<std::mutex> lock(bundleUserMgrMutex_);
    auto dataMgr = GetDataMgrFromService();
    if (dataMgr == nullptr) {
        APP_LOGE("DataMgr is nullptr");
        return;
    }

    auto installer = GetBundleInstaller();
    if (installer == nullptr) {
        APP_LOGE("installer is nullptr");
        return;
    }

    if (!dataMgr->HasUserId(userId)) {
        APP_LOGE("Has remove user %{public}d.", userId);
        return;
    }

    std::vector<BundleInfo> bundleInfos;
    if (!dataMgr->GetBundleInfos(BundleFlag::GET_BUNDLE_DEFAULT, bundleInfos, userId)) {
        APP_LOGE("get all bundle info failed when userId is %{public}d.", userId);
        dataMgr->RemoveUserId(userId);
        return;
    }

    g_installedHapNum = 0;
    std::shared_ptr<BundlePromise> bundlePromise = std::make_shared<BundlePromise>();
    uint32_t totalHapNum = bundleInfos.size();
    for (const auto &info : bundleInfos) {
        InstallParam installParam;
        installParam.userId = userId;
        installParam.forceExecuted = true;
        installParam.installFlag = InstallFlag::NORMAL;
        sptr<UserReceiverImpl> userReceiverImpl(new UserReceiverImpl());
        userReceiverImpl->SetBundlePromise(bundlePromise);
        userReceiverImpl->SetTotalHapNum(totalHapNum);
        installer->Uninstall(info.name, installParam, userReceiverImpl);
    }

    if (g_installedHapNum < totalHapNum) {
        bundlePromise->WaitForAllTasksExecute();
    }
    dataMgr->RemoveUserId(userId);
    APP_LOGD("RemoveUser end userId: (%{public}d)", userId);
}

void BundleUserMgrHostImpl::CheckInitialUser()
{
    auto dataMgr = GetDataMgrFromService();
    if (dataMgr == nullptr) {
        APP_LOGE("DataMgr is nullptr");
        return;
    }

    if (!dataMgr->HasInitialUserCreated()) {
        APP_LOGD("Bms initial user do not created successfully and wait.");
        std::shared_ptr<BundlePromise> bundlePromise = std::make_shared<BundlePromise>();
        dataMgr->SetBundlePromise(bundlePromise);
        bundlePromise->WaitForAllTasksExecute();
        APP_LOGD("Bms initial user created successfully.");
    }
}

const std::shared_ptr<BundleDataMgr> BundleUserMgrHostImpl::GetDataMgrFromService()
{
    return DelayedSingleton<BundleMgrService>::GetInstance()->GetDataMgr();
}

const sptr<IBundleInstaller> BundleUserMgrHostImpl::GetBundleInstaller()
{
    return DelayedSingleton<BundleMgrService>::GetInstance()->GetBundleInstaller();
}
}  // namespace AppExecFwk
}  // namespace OHOS
