/*
 * Copyright (c) 2022 Huawei Device Co., Ltd.
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

#include <fstream>
#include <future>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>
#include <vector>

#include <gtest/gtest.h>

#include "app_log_wrapper.h"
#include "bundle_info.h"
#include "bundle_installer_interface.h"
#include "bundle_mgr_client.h"
#include "bundle_mgr_interface.h"
#include "common_event_manager.h"
#include "common_event_support.h"
#include "iservice_registry.h"
#include "status_receiver_host.h"
#include "system_ability_definition.h"

using namespace testing::ext;

namespace OHOS {
namespace AppExecFwk {
namespace {
const std::string THIRD_PATH = "/data/test/bms_bundle/";
const std::string BUNDLE_NAME = "com.example.ohosproject.hmservice";
const std::string MSG_SUCCESS = "[SUCCESS]";
const std::string OPERATION_FAILURE = "Failure";
const std::string OPERATION_SUCCESS = "Success";
const int32_t TIMEOUT = 10;
const int32_t DEFAULT_USERID = 100;
const int32_t DLP_TYPE_1 = 1;
const int32_t DLP_TYPE_2 = 2;
} // namespace

class StatusReceiverImpl : public StatusReceiverHost {
public:
    StatusReceiverImpl();
    virtual ~StatusReceiverImpl() override;
    virtual void OnStatusNotify(const int32_t progress) override;
    virtual void OnFinished(const int32_t resultCode, const std::string &resultMsg) override;
    std::string GetResultMsg() const;

private:
    mutable std::promise<std::string> resultMsgSignal_;
    int32_t iProgress_ = 0;

    DISALLOW_COPY_AND_MOVE(StatusReceiverImpl);
};

StatusReceiverImpl::StatusReceiverImpl()
{
    APP_LOGI("create status receiver instance");
}

StatusReceiverImpl::~StatusReceiverImpl()
{
    APP_LOGI("destroy status receiver instance");
}

void StatusReceiverImpl::OnStatusNotify(const int32_t progress)
{
    EXPECT_GT(progress, iProgress_);
    iProgress_ = progress;
    APP_LOGI("OnStatusNotify progress:%{public}d", progress);
}

void StatusReceiverImpl::OnFinished(const int32_t resultCode, const std::string &resultMsg)
{
    APP_LOGD("on finished result is %{public}d, %{public}s", resultCode, resultMsg.c_str());
    resultMsgSignal_.set_value(resultMsg);
}

std::string StatusReceiverImpl::GetResultMsg() const
{
    auto future = resultMsgSignal_.get_future();
    std::chrono::seconds timeout(TIMEOUT);
    if (future.wait_for(timeout) == std::future_status::timeout) {
        return OPERATION_FAILURE + " timeout";
    }
    std::string resultMsg = future.get();
    if (resultMsg == MSG_SUCCESS) {
        return OPERATION_SUCCESS;
    }
    return OPERATION_FAILURE + resultMsg;
}

class BundleMgrSandboxAppSystemTest : public testing::Test {
public:
    static void SetUpTestCase();
    static void TearDownTestCase();
    void SetUp();
    void TearDown();
    static void InstallBundle(
        const std::string &bundleFilePath, const InstallFlag installFlag, std::string &installMsg,
        const int32_t userId);
    static void UninstallBundle(const std::string &bundleName, std::string &uninstallMsg, const int32_t userId);
    static void CheckPathAreExist(const std::string &bundleName, int32_t userId, int32_t appIndex);
    static void CheckSandboxAppInfo(const std::string &bundleName, int32_t appIndex, int32_t userId);
    static sptr<IBundleMgr> GetBundleMgrProxy();
    static sptr<IBundleInstaller> GetInstallerProxy();
};

void BundleMgrSandboxAppSystemTest::SetUpTestCase()
{}

void BundleMgrSandboxAppSystemTest::TearDownTestCase()
{}

void BundleMgrSandboxAppSystemTest::SetUp()
{}

void BundleMgrSandboxAppSystemTest::TearDown()
{}

sptr<IBundleMgr> BundleMgrSandboxAppSystemTest::GetBundleMgrProxy()
{
    sptr<ISystemAbilityManager> systemAbilityManager =
        SystemAbilityManagerClient::GetInstance().GetSystemAbilityManager();
    if (!systemAbilityManager) {
        APP_LOGE("fail to get system ability mgr.");
        return nullptr;
    }

    sptr<IRemoteObject> remoteObject = systemAbilityManager->GetSystemAbility(BUNDLE_MGR_SERVICE_SYS_ABILITY_ID);
    if (!remoteObject) {
        APP_LOGE("fail to get bundle manager proxy.");
        return nullptr;
    }

    APP_LOGI("get bundle manager proxy success.");
    return iface_cast<IBundleMgr>(remoteObject);
}

sptr<IBundleInstaller> BundleMgrSandboxAppSystemTest::GetInstallerProxy()
{
    sptr<IBundleMgr> bundleMgrProxy = GetBundleMgrProxy();
    if (!bundleMgrProxy) {
        APP_LOGE("bundle mgr proxy is nullptr.");
        return nullptr;
    }

    sptr<IBundleInstaller> installerProxy = bundleMgrProxy->GetBundleInstaller();
    if (!installerProxy) {
        APP_LOGE("fail to get bundle installer proxy");
        return nullptr;
    }

    APP_LOGI("get bundle installer proxy success.");
    return installerProxy;
}

void BundleMgrSandboxAppSystemTest::InstallBundle(
    const std::string &bundleFilePath, const InstallFlag installFlag, std::string &installMsg,
    const int32_t userId = Constants::ALL_USERID)
{
    sptr<IBundleInstaller> installerProxy = GetInstallerProxy();
    if (!installerProxy) {
        APP_LOGE("get bundle installer Failure.");
        installMsg = "Failure";
        return;
    }

    InstallParam installParam;
    installParam.installFlag = installFlag;
    if (userId != 0) {
        installParam.userId = userId;
    }
    sptr<StatusReceiverImpl> statusReceiver(new (std::nothrow) StatusReceiverImpl());
    EXPECT_NE(statusReceiver, nullptr);
    bool installResult = installerProxy->Install(bundleFilePath, installParam, statusReceiver);
    EXPECT_TRUE(installResult);
    installMsg = statusReceiver->GetResultMsg();
}

void BundleMgrSandboxAppSystemTest::UninstallBundle(
    const std::string &bundleName, std::string &uninstallMsg, const int32_t userId = Constants::ALL_USERID)
{
    sptr<IBundleInstaller> installerProxy = GetInstallerProxy();
    if (!installerProxy) {
        APP_LOGE("get bundle installer Failure.");
        uninstallMsg = "Failure";
        return;
    }

    sptr<StatusReceiverImpl> statusReceiver(new (std::nothrow) StatusReceiverImpl());
    EXPECT_NE(statusReceiver, nullptr);
    InstallParam installParam;
    if (userId != 0) {
        installParam.userId = userId;
    }
    bool uninstallResult = installerProxy->Uninstall(bundleName, installParam, statusReceiver);
    EXPECT_TRUE(uninstallResult);
    uninstallMsg = statusReceiver->GetResultMsg();
}

void BundleMgrSandboxAppSystemTest::CheckPathAreExist(const std::string &bundleName, int32_t userId, int32_t appIndex)
{
    std::string innerDataPath = "/data/app/el1/" + std::to_string(userId) + Constants::PATH_SEPARATOR;
    auto dataPath = innerDataPath + "base/" + bundleName + Constants::FILE_UNDERLINE + std::to_string(appIndex);
    int32_t ret = access(dataPath.c_str(), F_OK);
    EXPECT_EQ(ret, 0);

    dataPath = innerDataPath + "database/" + bundleName + Constants::FILE_UNDERLINE + std::to_string(appIndex);
    ret = access(dataPath.c_str(), F_OK);
    EXPECT_EQ(ret, 0);

    innerDataPath = "/data/app/el2/" + std::to_string(userId) + Constants::PATH_SEPARATOR;
    dataPath = innerDataPath + "base" + bundleName + Constants::FILE_UNDERLINE + std::to_string(appIndex) + "cache";
    ret = access(dataPath.c_str(), F_OK);
    EXPECT_EQ(ret, 0);

    dataPath = innerDataPath + "base" + bundleName + Constants::FILE_UNDERLINE + std::to_string(appIndex) + "el3";
    ret = access(dataPath.c_str(), F_OK);
    EXPECT_EQ(ret, 0);

    dataPath = innerDataPath + "base" + bundleName + Constants::FILE_UNDERLINE + std::to_string(appIndex) + "el4";
    ret = access(dataPath.c_str(), F_OK);
    EXPECT_EQ(ret, 0);

    dataPath = innerDataPath + "base" + bundleName + Constants::FILE_UNDERLINE + std::to_string(appIndex) + "haps";
    ret = access(dataPath.c_str(), F_OK);
    EXPECT_EQ(ret, 0);

    dataPath =
        innerDataPath + "base" + bundleName + Constants::FILE_UNDERLINE + std::to_string(appIndex) + "preferences";
    ret = access(dataPath.c_str(), F_OK);
    EXPECT_EQ(ret, 0);

    dataPath = innerDataPath + "base" + bundleName + Constants::FILE_UNDERLINE + std::to_string(appIndex) + "temp";
    ret = access(dataPath.c_str(), F_OK);
    EXPECT_EQ(ret, 0);

    dataPath =
        innerDataPath + "database" + bundleName + Constants::FILE_UNDERLINE + std::to_string(appIndex) + "cache";
    ret = access(dataPath.c_str(), F_OK);
    EXPECT_EQ(ret, 0);
}

void BundleMgrSandboxAppSystemTest::CheckSandboxAppInfo(const std::string &bundleName, int32_t appIndex, int32_t userId)
{
    BundleInfo info;
    BundleMgrClient bundleMgrClient;
    bool ret = bundleMgrClient.GetSandboxBundleInfo(bundleName, appIndex, userId, info);
    EXPECT_TRUE(ret);

    BundleInfo info2;
    ret = bundleMgrClient.GetBundleInfo(bundleName, BundleFlag::GET_BUNDLE_DEFAULT, info2, DEFAULT_USERID);
    EXPECT_TRUE(ret);

    std::cout << "uid : " << info.uid << std::endl;
    EXPECT_NE(info.uid, info2.uid);
}

/**
 * @tc.number: InstallSandboxAppTest001
 * @tc.name: InstallSandboxApp
 * @tc.desc: Test the interface of InstallSandboxApp
 *           1. Install application 
 * @tc.require: AR000GNT9D
 */
HWTEST_F(BundleMgrSandboxAppSystemTest, InstallSandboxAppTest001, TestSize.Level1)
{
    auto name = std::string("InstallSandboxAppTest_001");
    GTEST_LOG_(INFO) << name << " start";
    std::string bundleFilePath = THIRD_PATH + "bundleClient1.hap";
    std::string installMsg;
    InstallBundle(bundleFilePath, InstallFlag::NORMAL, installMsg);
    EXPECT_EQ(installMsg, "Success") << "install fail!" << bundleFilePath;

    BundleMgrClient bundleMgrClient;
    int32_t ret = bundleMgrClient.InstallSandboxApp(BUNDLE_NAME, DLP_TYPE_1, DEFAULT_USERID);
    EXPECT_LT(0, ret);

    CheckPathAreExist(BUNDLE_NAME, DEFAULT_USERID, 1);
    CheckSandboxAppInfo(BUNDLE_NAME, 1, DEFAULT_USERID);

    auto res = bundleMgrClient.UninstallSandboxApp(BUNDLE_NAME, 1, DEFAULT_USERID);
    EXPECT_TRUE(res);

    GTEST_LOG_(INFO) << name << " end";
}

/**
 * @tc.number: InstallSandboxAppTest002
 * @tc.name: InstallSandboxApp
 * @tc.desc: Test the interface of InstallSandboxApp
 *           1. Install application 
 * @tc.require: AR000GNT9D
 */
HWTEST_F(BundleMgrSandboxAppSystemTest, InstallSandboxAppTest002, TestSize.Level1)
{
    auto name = std::string("InstallSandboxAppTest002");
    GTEST_LOG_(INFO) << name << " start";
    std::string bundleFilePath = THIRD_PATH + "bundleClient1.hap";
    std::string installMsg;
    InstallBundle(bundleFilePath, InstallFlag::NORMAL, installMsg);
    EXPECT_EQ(installMsg, "Success") << "install fail!" << bundleFilePath;

    BundleMgrClient bundleMgrClient;
    int32_t ret = bundleMgrClient.InstallSandboxApp(BUNDLE_NAME, DLP_TYPE_2, DEFAULT_USERID);
    EXPECT_LT(0, ret);

    CheckPathAreExist(BUNDLE_NAME, DEFAULT_USERID, 1);
    CheckSandboxAppInfo(BUNDLE_NAME, 1, DEFAULT_USERID);

    auto res = bundleMgrClient.UninstallSandboxApp(BUNDLE_NAME, 1, DEFAULT_USERID);
    EXPECT_TRUE(res);

    GTEST_LOG_(INFO) << name << " end";
}
} // AppExecFwk
} // OHOS