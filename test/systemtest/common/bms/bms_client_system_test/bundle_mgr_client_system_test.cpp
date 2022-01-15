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
#include <fstream>
#include <future>
#include <vector>

#include <gtest/gtest.h>

#include "ability_info.h"
#include "app_log_wrapper.h"
#include "bundle_installer_interface.h"
#include "bundle_mgr_client.h"
#include "bundle_mgr_interface.h"
#include "common_event_manager.h"
#include "common_event_support.h"
#include "common_tool.h"
#include "extension_ability_info.h"
#include "hap_module_info.h"
#include "iservice_registry.h"
#include "status_receiver_host.h"
#include "system_ability_definition.h"

using namespace testing::ext;

namespace OHOS {
namespace AppExecFwk {
namespace {
const std::string RESOURCE_PATH =
    "/data/accounts/account_0/applications/com.example.testRes/com.example.testRes/assets/entry/resources.index";
const std::string THIRD_PATH = "/data/test/bms_bundle/";
const std::string BUNDLE_NAME = "com.example.testRes";
const std::string MSG_SUCCESS = "[SUCCESS]";
const std::string OPERATION_FAILURE = "Failure";
const std::string OPERATION_SUCCESS = "Success";
const int TIMEOUT = 10;
} // namespace

class StatusReceiverImpl : public StatusReceiverHost {
public:
    StatusReceiverImpl();
    virtual ~StatusReceiverImpl() override;
    virtual void OnStatusNotify(const int progress) override;
    virtual void OnFinished(const int32_t resultCode, const std::string &resultMsg) override;
    std::string GetResultMsg() const;

private:
    mutable std::promise<std::string> resultMsgSignal_;
    int iProgress_ = 0;

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

void StatusReceiverImpl::OnStatusNotify(const int progress)
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

class BundleMgrClientSystemTest : public testing::Test {
public:
    static void SetUpTestCase();
    static void TearDownTestCase();
    void SetUp();
    void TearDown();
    static void InstallBundle(
        const std::string &bundleFilePath, const InstallFlag installFlag, std::string &installMsg, const int userId);
    static void UninstallBundle(const std::string &bundleName, std::string &uninstallMsg, const int userId);
    static sptr<IBundleMgr> GetBundleMgrProxy();
    static sptr<IBundleInstaller> GetInstallerProxy();
};

void BundleMgrClientSystemTest::SetUpTestCase()
{}

void BundleMgrClientSystemTest::TearDownTestCase()
{}

void BundleMgrClientSystemTest::SetUp()
{}

void BundleMgrClientSystemTest::TearDown()
{}

sptr<IBundleMgr> BundleMgrClientSystemTest::GetBundleMgrProxy()
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

sptr<IBundleInstaller> BundleMgrClientSystemTest::GetInstallerProxy()
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

void BundleMgrClientSystemTest::InstallBundle(
    const std::string &bundleFilePath, const InstallFlag installFlag, std::string &installMsg, const int userId = 0)
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

void BundleMgrClientSystemTest::UninstallBundle(
    const std::string &bundleName, std::string &uninstallMsg, const int userId = 0)
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

/**
 * @tc.number: GetResourceConfigFile_001
 * @tc.name: GetResConfigFile
 * @tc.desc: Test the interface of GetResConfigFile
 *           1. metadata is empty
 */
HWTEST_F(BundleMgrClientSystemTest, GetResourceConfigFile_001, TestSize.Level1)
{
    auto name = std::string("GetResourceConfigFile_001");
    GTEST_LOG_(INFO) << name << " start";
    ExtensionAbilityInfo info;
    BundleMgrClient bundleMgrClient;
    std::string metadataName = "test_name";
    std::vector<std::string> profileInfo;
    auto ret = bundleMgrClient.GetResConfigFile(info, metadataName, profileInfo);
    EXPECT_FALSE(ret);
    GTEST_LOG_(INFO) << name << " end";
}

/**
 * @tc.number: GetResourceConfigFile_002
 * @tc.name: GetResConfigFile
 * @tc.desc: Test the interface of GetResConfigFile
 *           1. metadataName is empty
 */
HWTEST_F(BundleMgrClientSystemTest, GetResourceConfigFile_002, TestSize.Level1)
{
    auto name = std::string("GetResourceConfigFile_002");
    GTEST_LOG_(INFO) << name << " start";
    ExtensionAbilityInfo info;
    std::vector<Metadata> &metadata = info.metadata;
    Metadata data;
    data.name = "extensionName";
    data.value = "extensionValue";
    data.resource = "@profile:forms";
    metadata.emplace_back(data);
    info.resourcePath = "resourcePath";
    BundleMgrClient bundleMgrClient;
    std::string metadataName = "";
    std::vector<std::string> profileInfo;
    auto ret = bundleMgrClient.GetResConfigFile(info, metadataName, profileInfo);
    EXPECT_FALSE(ret);
    GTEST_LOG_(INFO) << name << " end";
}

/**
 * @tc.number: GetResourceConfigFile_003
 * @tc.name: GetResConfigFile
 * @tc.desc: Test the interface of GetResConfigFile
 *           1. metadataName cannot be found
 */
HWTEST_F(BundleMgrClientSystemTest, GetResourceConfigFile_003, TestSize.Level1)
{
    auto name = std::string("GetResourceConfigFile_003");
    GTEST_LOG_(INFO) << name << " start";
    ExtensionAbilityInfo info;
    std::vector<Metadata> &metadata = info.metadata;
    Metadata data;
    data.name = "extensionName";
    data.value = "extensionValue";
    data.resource = "@profile:forms";
    metadata.emplace_back(data);
    info.resourcePath = "resourcePath";
    BundleMgrClient bundleMgrClient;
    std::string metadataName = "extensionName1";
    std::vector<std::string> profileInfo;
    auto ret = bundleMgrClient.GetResConfigFile(info, metadataName, profileInfo);
    EXPECT_FALSE(ret);
    GTEST_LOG_(INFO) << name << " end";
}

/**
 * @tc.number: GetResourceConfigFile_004
 * @tc.name: GetResConfigFile
 * @tc.desc: Test the interface of GetResConfigFile
 *           1. extensionInfo is invalid
 */
HWTEST_F(BundleMgrClientSystemTest, GetResourceConfigFile_004, TestSize.Level1)
{
    auto name = std::string("GetResourceConfigFile_004");
    GTEST_LOG_(INFO) << name << " start";
    ExtensionAbilityInfo info;
    std::vector<Metadata> &metadata = info.metadata;
    Metadata data;
    data.name = "extensionName";
    data.value = "extensionValue";
    data.resource = "@profile:forms";
    metadata.emplace_back(data);
    info.resourcePath = "resourcePath";
    BundleMgrClient bundleMgrClient;
    std::string metadataName = "extensionName";
    std::vector<std::string> profileInfo;
    auto ret = bundleMgrClient.GetResConfigFile(info, metadataName, profileInfo);
    EXPECT_FALSE(ret);
    GTEST_LOG_(INFO) << name << " end";
}

/**
 * @tc.number: GetResourceConfigFile_005
 * @tc.name: GetResConfigFile
 * @tc.desc: Test the interface of GetResConfigFile
 *           1. metadata's resource is empty
 */
HWTEST_F(BundleMgrClientSystemTest, GetResourceConfigFile_005, TestSize.Level1)
{
    auto name = std::string("GetResourceConfigFile_005");
    GTEST_LOG_(INFO) << name << " start";
    ExtensionAbilityInfo info;
    std::vector<Metadata> &metadata = info.metadata;
    Metadata data;
    data.name = "extensionName";
    data.value = "extensionValue";
    metadata.emplace_back(data);
    info.resourcePath = "resourcePath";
    BundleMgrClient bundleMgrClient;
    std::string metadataName = "extensionName";
    std::vector<std::string> profileInfo;
    auto ret = bundleMgrClient.GetResConfigFile(info, metadataName, profileInfo);
    EXPECT_FALSE(ret);
    GTEST_LOG_(INFO) << name << " end";
}

/**
 * @tc.number: GetResourceConfigFile_006
 * @tc.name: GetResConfigFile
 * @tc.desc: Test the interface of GetResConfigFile
 *           1. metadata's resource file is not json format.
 */
HWTEST_F(BundleMgrClientSystemTest, GetResourceConfigFile_006, TestSize.Level1)
{
    auto name = std::string("GetResourceConfigFile_006");
    GTEST_LOG_(INFO) << name << " start";
    std::string bundleFilePath = THIRD_PATH + "bundleClient1.hap";
    std::string installMsg;
    InstallBundle(bundleFilePath, InstallFlag::NORMAL, installMsg);
    EXPECT_EQ(installMsg, "Success") << "install fail!" << bundleFilePath;

    ExtensionAbilityInfo info;
    std::vector<Metadata> &metadata = info.metadata;
    Metadata data;
    data.name = "ohos.forms";
    data.value = "forms";
    data.resource = "@profile:forms";
    metadata.emplace_back(data);
    info.resourcePath = RESOURCE_PATH;
    BundleMgrClient bundleMgrClient;
    std::string metadataName = "ohos.forms";
    std::vector<std::string> profileInfo;
    auto ret = bundleMgrClient.GetResConfigFile(info, metadataName, profileInfo);
    EXPECT_TRUE(ret);
    std::cout << profileInfo[0] << std::endl;

    std::string uninstallMsg;
    UninstallBundle(BUNDLE_NAME, uninstallMsg);
    EXPECT_EQ(uninstallMsg, "Success") << "uninstall fail!" << bundleFilePath;
    std::cout << "END GetResourceConfigFile_006" << std::endl;
    GTEST_LOG_(INFO) << name << " end";
}

/**
 * @tc.number: GetResourceConfigFile_007
 * @tc.name: GetResConfigFile
 * @tc.desc: Test the interface of GetResConfigFile
 *           1. profile's suffix is not .json.
 */
HWTEST_F(BundleMgrClientSystemTest, GetResourceConfigFile_007, TestSize.Level1)
{
    auto name = std::string("GetResourceConfigFile_007");
    GTEST_LOG_(INFO) << name << " start";
    std::string bundleFilePath = THIRD_PATH + "bundleClient2.hap";
    std::string installMsg;
    InstallBundle(bundleFilePath, InstallFlag::NORMAL, installMsg);
    EXPECT_EQ(installMsg, "Success") << "install fail!" << bundleFilePath;

    ExtensionAbilityInfo info;
    std::vector<Metadata> &metadata = info.metadata;
    Metadata data;
    data.name = "ohos.forms";
    data.value = "forms";
    data.resource = "@profile:forms";
    metadata.emplace_back(data);
    info.resourcePath = RESOURCE_PATH;
    BundleMgrClient bundleMgrClient;
    std::string metadataName = "ohos.forms";
    std::vector<std::string> profileInfo;
    auto ret = bundleMgrClient.GetResConfigFile(info, metadataName, profileInfo);
    EXPECT_FALSE(ret);
    EXPECT_EQ(profileInfo.size(), 0);

    std::string uninstallMsg;
    UninstallBundle(BUNDLE_NAME, uninstallMsg);
    EXPECT_EQ(uninstallMsg, "Success") << "uninstall fail!" << bundleFilePath;
    std::cout << "END GetResourceConfigFile_007" << std::endl;
    GTEST_LOG_(INFO) << name << " end";
}

/**
 * @tc.number: GetResourceConfigFile_008
 * @tc.name: GetResConfigFile
 * @tc.desc: Test the interface of GetResConfigFile
 *           1. metadata is empty
 */
HWTEST_F(BundleMgrClientSystemTest, GetResourceConfigFile_008, TestSize.Level1)
{
    auto name = std::string("GetResourceConfigFile_008");
    GTEST_LOG_(INFO) << name << " start";
    AbilityInfo info;
    BundleMgrClient bundleMgrClient;
    std::string metadataName = "test_name";
    std::vector<std::string> profileInfo;
    auto ret = bundleMgrClient.GetResConfigFile(info, metadataName, profileInfo);
    EXPECT_FALSE(ret);
    GTEST_LOG_(INFO) << name << " end";
}

/**
 * @tc.number: GetResourceConfigFile_009
 * @tc.name: GetResConfigFile
 * @tc.desc: Test the interface of GetResConfigFile
 *           1. metadataName is empty
 */
HWTEST_F(BundleMgrClientSystemTest, GetResourceConfigFile_009, TestSize.Level1)
{
    auto name = std::string("GetResourceConfigFile_009");
    GTEST_LOG_(INFO) << name << " start";
    AbilityInfo info;
    std::vector<Metadata> &metadata = info.metadata;
    Metadata data;
    data.name = "extensionName";
    data.value = "extensionValue";
    data.resource = "@profile:forms";
    metadata.emplace_back(data);
    info.resourcePath = "resourcePath";
    BundleMgrClient bundleMgrClient;
    std::string metadataName = "";
    std::vector<std::string> profileInfo;
    auto ret = bundleMgrClient.GetResConfigFile(info, metadataName, profileInfo);
    EXPECT_FALSE(ret);
    GTEST_LOG_(INFO) << name << " end";
}

/**
 * @tc.number: GetResourceConfigFile_010
 * @tc.name: GetResConfigFile
 * @tc.desc: Test the interface of GetResConfigFile
 *           1. metadataName cannot be found
 */
HWTEST_F(BundleMgrClientSystemTest, GetResourceConfigFile_010, TestSize.Level1)
{
    auto name = std::string("GetResourceConfigFile_010");
    GTEST_LOG_(INFO) << name << " start";
    AbilityInfo info;
    std::vector<Metadata> &metadata = info.metadata;
    Metadata data;
    data.name = "extensionName";
    data.value = "extensionValue";
    data.resource = "@profile:forms";
    metadata.emplace_back(data);
    info.resourcePath = "resourcePath";
    BundleMgrClient bundleMgrClient;
    std::string metadataName = "extensionName1";
    std::vector<std::string> profileInfo;
    auto ret = bundleMgrClient.GetResConfigFile(info, metadataName, profileInfo);
    EXPECT_FALSE(ret);
    GTEST_LOG_(INFO) << name << " end";
}

/**
 * @tc.number: GetResourceConfigFile_011
 * @tc.name: GetResConfigFile
 * @tc.desc: Test the interface of GetResConfigFile
 *           1. extensionInfo is invalid
 */
HWTEST_F(BundleMgrClientSystemTest, GetResourceConfigFile_011, TestSize.Level1)
{
    auto name = std::string("GetResourceConfigFile_011");
    GTEST_LOG_(INFO) << name << " start";
    AbilityInfo info;
    std::vector<Metadata> &metadata = info.metadata;
    Metadata data;
    data.name = "extensionName";
    data.value = "extensionValue";
    data.resource = "@profile:forms";
    metadata.emplace_back(data);
    info.resourcePath = "resourcePath";
    BundleMgrClient bundleMgrClient;
    std::string metadataName = "extensionName";
    std::vector<std::string> profileInfo;
    auto ret = bundleMgrClient.GetResConfigFile(info, metadataName, profileInfo);
    EXPECT_FALSE(ret);
    GTEST_LOG_(INFO) << name << " end";
}

/**
 * @tc.number: GetResourceConfigFile_012
 * @tc.name: GetResConfigFile
 * @tc.desc: Test the interface of GetResConfigFile
 *           1. metadata's resource is empty
 */
HWTEST_F(BundleMgrClientSystemTest, GetResourceConfigFile_012, TestSize.Level1)
{
    auto name = std::string("GetResourceConfigFile_012");
    GTEST_LOG_(INFO) << name << " start";
    AbilityInfo info;
    std::vector<Metadata> &metadata = info.metadata;
    Metadata data;
    data.name = "extensionName";
    data.value = "extensionValue";
    metadata.emplace_back(data);
    info.resourcePath = "resourcePath";
    BundleMgrClient bundleMgrClient;
    std::string metadataName = "extensionName";
    std::vector<std::string> profileInfo;
    auto ret = bundleMgrClient.GetResConfigFile(info, metadataName, profileInfo);
    EXPECT_FALSE(ret);
    GTEST_LOG_(INFO) << name << " end";
}

/**
 * @tc.number: GetResourceConfigFile_013
 * @tc.name: GetResConfigFile
 * @tc.desc: Test the interface of GetResConfigFile
 */
HWTEST_F(BundleMgrClientSystemTest, GetResourceConfigFile_013, TestSize.Level1)
{
    auto name = std::string("GetResourceConfigFile_013");
    GTEST_LOG_(INFO) << name << " start";

    std::string bundleFilePath = THIRD_PATH + "bundleClient1.hap";
    std::string installMsg;
    InstallBundle(bundleFilePath, InstallFlag::NORMAL, installMsg);
    EXPECT_EQ(installMsg, "Success") << "install fail!" << bundleFilePath;

    AbilityInfo info;
    std::vector<Metadata> &metadata = info.metadata;
    Metadata data;
    data.name = "ohos.forms";
    data.value = "forms";
    data.resource = "@profile:forms";
    metadata.emplace_back(data);
    info.resourcePath = RESOURCE_PATH;
    BundleMgrClient bundleMgrClient;
    std::string metadataName = "ohos.forms";
    std::vector<std::string> profileInfo;
    auto ret = bundleMgrClient.GetResConfigFile(info, metadataName, profileInfo);
    EXPECT_TRUE(ret);

    std::string uninstallMsg;
    UninstallBundle(BUNDLE_NAME, uninstallMsg);
    EXPECT_EQ(uninstallMsg, "Success") << "uninstall fail!" << bundleFilePath;
    std::cout << "END GetResourceConfigFile_013" << std::endl;
    GTEST_LOG_(INFO) << name << " end";
}

/**
 * @tc.number: GetResourceConfigFile_014
 * @tc.name: GetResConfigFile
 * @tc.desc: Test the interface of GetResConfigFile
 *           1. profile's suffix is not .json
 */
HWTEST_F(BundleMgrClientSystemTest, GetResourceConfigFile_014, TestSize.Level1)
{
    auto name = std::string("GetResourceConfigFile_014");
    GTEST_LOG_(INFO) << name << " start";

    std::string bundleFilePath = THIRD_PATH + "bundleClient2.hap";
    std::string installMsg;
    InstallBundle(bundleFilePath, InstallFlag::NORMAL, installMsg);
    EXPECT_EQ(installMsg, "Success") << "install fail!" << bundleFilePath;

    AbilityInfo info;
    std::vector<Metadata> &metadata = info.metadata;
    Metadata data;
    data.name = "ohos.forms";
    data.value = "forms";
    data.resource = "@profile:forms";
    metadata.emplace_back(data);
    info.resourcePath = RESOURCE_PATH;
    BundleMgrClient bundleMgrClient;
    std::string metadataName = "ohos.forms";
    std::vector<std::string> profileInfo;
    auto ret = bundleMgrClient.GetResConfigFile(info, metadataName, profileInfo);
    EXPECT_FALSE(ret);
    EXPECT_EQ(profileInfo.size(), 0);

    std::string uninstallMsg;
    UninstallBundle(BUNDLE_NAME, uninstallMsg);
    EXPECT_EQ(uninstallMsg, "Success") << "uninstall fail!" << bundleFilePath;
    std::cout << "END GetResourceConfigFile_014" << std::endl;
    GTEST_LOG_(INFO) << name << " end";
}


/**
 * @tc.number: GetResourceConfigFile_015
 * @tc.name: GetResConfigFile
 * @tc.desc: Test the interface of GetResConfigFile
 *           1. metadata is empty
 */
HWTEST_F(BundleMgrClientSystemTest, GetResourceConfigFile_015, TestSize.Level1)
{
    auto name = std::string("GetResourceConfigFile_015");
    GTEST_LOG_(INFO) << name << " start";
    HapModuleInfo info;
    BundleMgrClient bundleMgrClient;
    std::string metadataName = "test_name";
    std::vector<std::string> profileInfo;
    auto ret = bundleMgrClient.GetResConfigFile(info, metadataName, profileInfo);
    EXPECT_FALSE(ret);
    GTEST_LOG_(INFO) << name << " end";
}

/**
 * @tc.number: GetResourceConfigFile_016
 * @tc.name: GetResConfigFile
 * @tc.desc: Test the interface of GetResConfigFile
 *           1. metadataName is empty
 */
HWTEST_F(BundleMgrClientSystemTest, GetResourceConfigFile_016, TestSize.Level1)
{
    auto name = std::string("GetResourceConfigFile_009");
    GTEST_LOG_(INFO) << name << " start";
    HapModuleInfo info;
    std::vector<Metadata> &metadata = info.metadata;
    Metadata data;
    data.name = "extensionName";
    data.value = "extensionValue";
    data.resource = "@profile:forms";
    metadata.emplace_back(data);
    info.resourcePath = "resourcePath";
    BundleMgrClient bundleMgrClient;
    std::string metadataName = "";
    std::vector<std::string> profileInfo;
    auto ret = bundleMgrClient.GetResConfigFile(info, metadataName, profileInfo);
    EXPECT_FALSE(ret);
    GTEST_LOG_(INFO) << name << " end";
}

/**
 * @tc.number: GetResourceConfigFile_017
 * @tc.name: GetResConfigFile
 * @tc.desc: Test the interface of GetResConfigFile
 *           1. metadataName cannot be found
 */
HWTEST_F(BundleMgrClientSystemTest, GetResourceConfigFile_017, TestSize.Level1)
{
    auto name = std::string("GetResourceConfigFile_010");
    GTEST_LOG_(INFO) << name << " start";
    HapModuleInfo info;
    std::vector<Metadata> &metadata = info.metadata;
    Metadata data;
    data.name = "extensionName";
    data.value = "extensionValue";
    data.resource = "@profile:forms";
    metadata.emplace_back(data);
    info.resourcePath = "resourcePath";
    BundleMgrClient bundleMgrClient;
    std::string metadataName = "extensionName1";
    std::vector<std::string> profileInfo;
    auto ret = bundleMgrClient.GetResConfigFile(info, metadataName, profileInfo);
    EXPECT_FALSE(ret);
    GTEST_LOG_(INFO) << name << " end";
}

/**
 * @tc.number: GetResourceConfigFile_018
 * @tc.name: GetResConfigFile
 * @tc.desc: Test the interface of GetResConfigFile
 *           1. extensionInfo is invalid
 */
HWTEST_F(BundleMgrClientSystemTest, GetResourceConfigFile_018, TestSize.Level1)
{
    auto name = std::string("GetResourceConfigFile_018");
    GTEST_LOG_(INFO) << name << " start";
    HapModuleInfo info;
    std::vector<Metadata> &metadata = info.metadata;
    Metadata data;
    data.name = "extensionName";
    data.value = "extensionValue";
    data.resource = "@profile:forms";
    metadata.emplace_back(data);
    info.resourcePath = "resourcePath";
    BundleMgrClient bundleMgrClient;
    std::string metadataName = "extensionName";
    std::vector<std::string> profileInfo;
    auto ret = bundleMgrClient.GetResConfigFile(info, metadataName, profileInfo);
    EXPECT_FALSE(ret);
    GTEST_LOG_(INFO) << name << " end";
}

/**
 * @tc.number: GetResourceConfigFile_019
 * @tc.name: GetResConfigFile
 * @tc.desc: Test the interface of GetResConfigFile
 *           1. metadata's resource is empty
 */
HWTEST_F(BundleMgrClientSystemTest, GetResourceConfigFile_019, TestSize.Level1)
{
    auto name = std::string("GetResourceConfigFile_019");
    GTEST_LOG_(INFO) << name << " start";
    HapModuleInfo info;
    std::vector<Metadata> &metadata = info.metadata;
    Metadata data;
    data.name = "extensionName";
    data.value = "extensionValue";
    metadata.emplace_back(data);
    info.resourcePath = "resourcePath";
    BundleMgrClient bundleMgrClient;
    std::string metadataName = "extensionName";
    std::vector<std::string> profileInfo;
    auto ret = bundleMgrClient.GetResConfigFile(info, metadataName, profileInfo);
    EXPECT_FALSE(ret);
    GTEST_LOG_(INFO) << name << " end";
}

/**
 * @tc.number: GetResourceConfigFile_020
 * @tc.name: GetResConfigFile
 * @tc.desc: Test the interface of GetResConfigFile
 */
HWTEST_F(BundleMgrClientSystemTest, GetResourceConfigFile_020, TestSize.Level1)
{
    auto name = std::string("GetResourceConfigFile_020");
    GTEST_LOG_(INFO) << name << " start";

    std::string bundleFilePath = THIRD_PATH + "bundleClient1.hap";
    std::string installMsg;
    InstallBundle(bundleFilePath, InstallFlag::NORMAL, installMsg);
    EXPECT_EQ(installMsg, "Success") << "install fail!" << bundleFilePath;

    HapModuleInfo info;
    std::vector<Metadata> &metadata = info.metadata;
    Metadata data;
    data.name = "ohos.forms";
    data.value = "forms";
    data.resource = "@profile:forms";
    metadata.emplace_back(data);
    info.resourcePath = RESOURCE_PATH;
    BundleMgrClient bundleMgrClient;
    std::string metadataName = "ohos.forms";
    std::vector<std::string> profileInfo;
    auto ret = bundleMgrClient.GetResConfigFile(info, metadataName, profileInfo);
    EXPECT_TRUE(ret);

    std::string uninstallMsg;
    UninstallBundle(BUNDLE_NAME, uninstallMsg);
    EXPECT_EQ(uninstallMsg, "Success") << "uninstall fail!" << bundleFilePath;
    std::cout << "END GetResourceConfigFile_020" << std::endl;
    GTEST_LOG_(INFO) << name << " end";
}

/**
 * @tc.number: GetResourceConfigFile_021
 * @tc.name: GetResConfigFile
 * @tc.desc: Test the interface of GetResConfigFile
 *           1. profile's suffix is not .json
 */
HWTEST_F(BundleMgrClientSystemTest, GetResourceConfigFile_021, TestSize.Level1)
{
    auto name = std::string("GetResourceConfigFile_021");
    GTEST_LOG_(INFO) << name << " start";

    std::string bundleFilePath = THIRD_PATH + "bundleClient2.hap";
    std::string installMsg;
    InstallBundle(bundleFilePath, InstallFlag::NORMAL, installMsg);
    EXPECT_EQ(installMsg, "Success") << "install fail!" << bundleFilePath;

    HapModuleInfo info;
    std::vector<Metadata> &metadata = info.metadata;
    Metadata data;
    data.name = "ohos.forms";
    data.value = "forms";
    data.resource = "@profile:forms";
    metadata.emplace_back(data);
    info.resourcePath = RESOURCE_PATH;
    BundleMgrClient bundleMgrClient;
    std::string metadataName = "ohos.forms";
    std::vector<std::string> profileInfo;
    auto ret = bundleMgrClient.GetResConfigFile(info, metadataName, profileInfo);
    EXPECT_FALSE(ret);
    EXPECT_EQ(profileInfo.size(), 0);

    std::string uninstallMsg;
    UninstallBundle(BUNDLE_NAME, uninstallMsg);
    EXPECT_EQ(uninstallMsg, "Success") << "uninstall fail!" << bundleFilePath;
    std::cout << "END GetResourceConfigFile_021" << std::endl;
    GTEST_LOG_(INFO) << name << " end";
}
}  // namespace AppExecFwk
}  // namespace OHOS