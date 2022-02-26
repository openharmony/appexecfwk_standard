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

#include "bundle_installer_manager.h"
#include <cinttypes>

#include "datetime_ex.h"

#include "app_log_wrapper.h"
#include "appexecfwk_errors.h"
#include "xcollie/xcollie.h"
#include "xcollie/xcollie_define.h"
#include <thread>
#include <chrono>

namespace OHOS {
namespace AppExecFwk {
namespace {
const std::string ADD_INSTALLER_FAIL = "fail to add installer in bundle installer manager";
}

BundleInstallerManager::BundleInstallerManager(const std::shared_ptr<EventRunner> &runner) : EventHandler(runner)
{
    APP_LOGI("create bundle installer manager instance");
    installersPool_.Start(THREAD_NUMBER);
    installersPool_.SetMaxTaskNum(MAX_TASK_NUMBER);
}

BundleInstallerManager::~BundleInstallerManager()
{
    APP_LOGI("destroy bundle installer manager instance");
    installersPool_.Stop();
}

void BundleInstallerManager::ProcessEvent(const InnerEvent::Pointer &event)
{
    APP_LOGD("process event : %{public}u", event->GetInnerEventId());
    switch (event->GetInnerEventId()) {
        case REMOVE_BUNDLE_INSTALLER:
            RemoveInstaller(event->GetParam());
            break;
        default:
            APP_LOGW("the eventId is not supported");
    }
}

void BundleInstallerManager::CreateInstallTask(
    const std::string &bundleFilePath, const InstallParam &installParam, const sptr<IStatusReceiver> &statusReceiver)
{
    auto installer = CreateInstaller(statusReceiver);
    if (!installer) {
        APP_LOGE("create installer failed");
        return;
    }
    auto task = [installer, bundleFilePath, installParam] {
        int32_t timerId = HiviewDFX::XCollie::GetInstance().SetTimer("InstallTask", 1, nullptr, nullptr, HiviewDFX::XCOLLIE_FLAG_LOG);
        std::this_thread::sleep_for(std::chrono::seconds(1));
        installer->Install(bundleFilePath, installParam);
        HiviewDFX::XCollie::GetInstance().CancelTimer(timerId);
    };
    installersPool_.AddTask(task);
}

void BundleInstallerManager::CreateRecoverTask(
    const std::string &bundleName, const InstallParam &installParam, const sptr<IStatusReceiver> &statusReceiver)
{
    auto installer = CreateInstaller(statusReceiver);
    if (!installer) {
        APP_LOGE("create installer failed");
        return;
    }
    auto task = [installer, bundleName, installParam] { installer->Recover(bundleName, installParam); };
    installersPool_.AddTask(task);
}

void BundleInstallerManager::CreateInstallTask(const std::vector<std::string> &bundleFilePaths,
    const InstallParam &installParam, const sptr<IStatusReceiver> &statusReceiver)
{
    auto installer = CreateInstaller(statusReceiver);
    if (!installer) {
        APP_LOGE("create installer failed");
        return;
    }
    auto task = [installer, bundleFilePaths, installParam] {
        int32_t timerId = HiviewDFX::XCollie::GetInstance().SetTimer("InstallTask", 1, nullptr, nullptr, HiviewDFX::XCOLLIE_FLAG_LOG);
        std::this_thread::sleep_for(std::chrono::seconds(1));
        installer->Install(bundleFilePaths, installParam);
        HiviewDFX::XCollie::GetInstance().CancelTimer(timerId);
    };
    installersPool_.AddTask(task);
}

void BundleInstallerManager::CreateUninstallTask(
    const std::string &bundleName, const InstallParam &installParam, const sptr<IStatusReceiver> &statusReceiver)
{
    auto installer = CreateInstaller(statusReceiver);
    if (!installer) {
        APP_LOGE("create installer failed");
        return;
    }
    auto task = [installer, bundleName, installParam] { installer->Uninstall(bundleName, installParam); };
    installersPool_.AddTask(task);
}

void BundleInstallerManager::CreateUninstallTask(const std::string &bundleName, const std::string &modulePackage,
    const InstallParam &installParam, const sptr<IStatusReceiver> &statusReceiver)
{
    auto installer = CreateInstaller(statusReceiver);
    if (!installer) {
        APP_LOGE("create installer failed");
        return;
    }
    auto task = [installer, bundleName, modulePackage, installParam] {
        installer->Uninstall(bundleName, modulePackage, installParam);
    };
    installersPool_.AddTask(task);
}

std::shared_ptr<BundleInstaller> BundleInstallerManager::CreateInstaller(const sptr<IStatusReceiver> &statusReceiver)
{
    int64_t installerId = GetMicroTickCount();
    auto installer = std::make_shared<BundleInstaller>(installerId, shared_from_this(), statusReceiver);
    if (!installer) {
        APP_LOGE("create bundle installer failed");
        return nullptr;
    }
    bool isSuccess = false;
    {
        std::lock_guard<std::mutex> lock(mutex_);
        auto result = installers_.try_emplace(installer->GetInstallerId(), installer);
        isSuccess = result.second;
    }
    if (isSuccess) {
        APP_LOGD("add the specific %{public}" PRId64 " installer", installerId);
    } else {
        APP_LOGE("fail to add bundle installer");
        installer.reset();
        statusReceiver->OnFinished(ERR_APPEXECFWK_INSTALL_INTERNAL_ERROR, ADD_INSTALLER_FAIL);
    }
    return installer;
}

void BundleInstallerManager::RemoveInstaller(const int64_t installerId)
{
    APP_LOGD("start to remove installer the specific %{public}" PRId64 " installer", installerId);
    std::lock_guard<std::mutex> lock(mutex_);
    if (installers_.erase(installerId) > 0) {
        APP_LOGD("erase the specific %{public}" PRId64 " installer", installerId);
    }
}
}  // namespace AppExecFwk
}  // namespace OHOS