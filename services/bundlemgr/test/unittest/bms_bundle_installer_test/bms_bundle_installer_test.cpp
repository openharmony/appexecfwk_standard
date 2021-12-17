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

#include <dirent.h>
#include <fcntl.h>
#include <gtest/gtest.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>
#include <fstream>
#include <chrono>
#include <cstdio>

#include "directory_ex.h"
#include "installd/installd_service.h"
#include "installd_client.h"
#include "system_bundle_installer.h"
#include "mock_status_receiver.h"
#include "install_param.h"
#include "bundle_info.h"
#include "bundle_installer_host.h"
#include "bundle_data_storage_database.h"
#include "bundle_mgr_service.h"

using namespace testing::ext;
using namespace std::chrono_literals;
using namespace OHOS::AppExecFwk;
using namespace OHOS::DistributedKv;
using namespace OHOS;
using OHOS::DelayedSingleton;

namespace {
const std::string BUNDLE_NAME = "com.example.l3jsdemo";
const std::string RESOURCE_ROOT_PATH = "/data/test/resource/bms/install_bundle/";
const std::string INVALID_PATH = "/install_bundle/";
const std::string RIGHT_BUNDLE = "right.hap";
const std::string INVALID_BUNDLE = "nonfile.hap";
const std::string FORMAT_ERROR_BUNDLE = "format_error_profile.hap";
const std::string WRONG_BUNDLE_NAME = "wrong_bundle_name.ha";
const std::string ERROR_BUNDLE_PROFILE_FILE = "error_bundle_profile.hap";
const std::string BUNDLE_DATA_DIR = "/data/accounts/account_0/appdata/com.example.l3jsdemo";
const std::string BUNDLE_CODE_DIR = "/data/accounts/account_0/applications/com.example.l3jsdemo";
const std::string ROOT_DIR = "/data/accounts";
const int32_t ROOT_UID = 0;
const int32_t USERID = 0;
const std::string INSTALL_THREAD = "TestInstall";
}  // namespace

class BmsBundleInstallerTest : public testing::Test {
public:
    BmsBundleInstallerTest();
    ~BmsBundleInstallerTest();
    static void SetUpTestCase();
    static void TearDownTestCase();
    void SetUp();
    void TearDown();
    bool InstallSystemBundle(const std::string &filePath) const;
    ErrCode InstallThirdPartyBundle(const std::string &filePath) const;
    ErrCode UpdateThirdPartyBundle(const std::string &filePath) const;
    void CheckFileExist() const;
    void CheckFileNonExist() const;
    const std::shared_ptr<BundleDataMgr> GetBundleDataMgr() const;
    const std::shared_ptr<BundleInstallerManager> GetBundleInstallerManager() const;
    void StopInstalldService() const;
    void StopBundleService();
    void CreateInstallerManager();
    void ClearBundleInfoInDb();

private:
    std::shared_ptr<BundleInstallerManager> manager_ = nullptr;
    std::shared_ptr<InstalldService> installdService_ = std::make_shared<InstalldService>();
    std::shared_ptr<BundleMgrService> bundleMgrService_ = DelayedSingleton<BundleMgrService>::GetInstance();
};

BmsBundleInstallerTest::BmsBundleInstallerTest()
{}

BmsBundleInstallerTest::~BmsBundleInstallerTest()
{}

bool BmsBundleInstallerTest::InstallSystemBundle(const std::string &filePath) const
{
    auto installer = std::make_unique<SystemBundleInstaller>(filePath);
    return installer->InstallSystemBundle(Constants::AppType::SYSTEM_APP);
}

ErrCode BmsBundleInstallerTest::InstallThirdPartyBundle(const std::string &filePath) const
{
    auto installer = DelayedSingleton<BundleMgrService>::GetInstance()->GetBundleInstaller();
    if (!installer) {
        EXPECT_FALSE(true) << "the installer is nullptr";
        return ERR_APPEXECFWK_INSTALL_INTERNAL_ERROR;
    }
    sptr<MockStatusReceiver> receiver = new (std::nothrow) MockStatusReceiver();
    if (!receiver) {
        EXPECT_FALSE(true) << "the receiver is nullptr";
        return ERR_APPEXECFWK_INSTALL_INTERNAL_ERROR;
    }
    InstallParam installParam;
    installParam.installFlag = InstallFlag::NORMAL;
    bool result = installer->Install(filePath, installParam, receiver);
    EXPECT_TRUE(result);
    return receiver->GetResultCode();
}

ErrCode BmsBundleInstallerTest::UpdateThirdPartyBundle(const std::string &filePath) const
{
    auto installer = DelayedSingleton<BundleMgrService>::GetInstance()->GetBundleInstaller();
    if (!installer) {
        EXPECT_FALSE(true) << "the installer is nullptr";
        return ERR_APPEXECFWK_INSTALL_INTERNAL_ERROR;
    }
    sptr<MockStatusReceiver> receiver = new (std::nothrow) MockStatusReceiver();
    if (!receiver) {
        EXPECT_FALSE(true) << "the receiver is nullptr";
        return ERR_APPEXECFWK_INSTALL_INTERNAL_ERROR;
    }
    InstallParam installParam;
    installParam.installFlag = InstallFlag::REPLACE_EXISTING;
    bool result = installer->Install(filePath, installParam, receiver);
    EXPECT_TRUE(result);
    return receiver->GetResultCode();
}

void BmsBundleInstallerTest::SetUpTestCase()
{
    if (access(ROOT_DIR.c_str(), F_OK) != 0) {
        bool result = OHOS::ForceCreateDirectory(ROOT_DIR);
        EXPECT_TRUE(result) << "fail to create root dir";
    }
    if (chown(ROOT_DIR.c_str(), ROOT_UID, ROOT_UID) != 0) {
        EXPECT_TRUE(false) << "fail to change root dir own ship";
    }
    if (chmod(ROOT_DIR.c_str(), S_IRWXU | S_IRGRP | S_IXGRP | S_IROTH | S_IXOTH) != 0) {
        EXPECT_TRUE(false) << "fail to change root dir mode";
    }
}

void BmsBundleInstallerTest::TearDownTestCase()
{}

void BmsBundleInstallerTest::SetUp()
{
    installdService_->Start();
    if (!bundleMgrService_->IsServiceReady()) {
        bundleMgrService_->OnStart();
    }
}

void BmsBundleInstallerTest::TearDown()
{
    StopInstalldService();
    StopBundleService();

    OHOS::ForceRemoveDirectory(BUNDLE_DATA_DIR);
    OHOS::ForceRemoveDirectory(BUNDLE_CODE_DIR);
}

void BmsBundleInstallerTest::CheckFileExist() const
{
    int bundleCodeExist = access(BUNDLE_CODE_DIR.c_str(), F_OK);
    EXPECT_EQ(bundleCodeExist, 0) << "the bundle code dir does not exists: " << BUNDLE_CODE_DIR;

    int bundleDataExist = access(BUNDLE_DATA_DIR.c_str(), F_OK);
    EXPECT_EQ(bundleDataExist, 0) << "the bundle data dir does not exists: " << BUNDLE_DATA_DIR;
}

void BmsBundleInstallerTest::CheckFileNonExist() const
{
    int bundleCodeExist = access(BUNDLE_CODE_DIR.c_str(), F_OK);
    EXPECT_NE(bundleCodeExist, 0) << "the bundle code dir exists: " << BUNDLE_CODE_DIR;

    int bundleDataExist = access(BUNDLE_DATA_DIR.c_str(), F_OK);
    EXPECT_NE(bundleDataExist, 0) << "the bundle data dir exists: " << BUNDLE_DATA_DIR;
}

const std::shared_ptr<BundleDataMgr> BmsBundleInstallerTest::GetBundleDataMgr() const
{
    return bundleMgrService_->GetDataMgr();
}

const std::shared_ptr<BundleInstallerManager> BmsBundleInstallerTest::GetBundleInstallerManager() const
{
    return manager_;
}

void BmsBundleInstallerTest::StopInstalldService() const
{
    installdService_->Stop();
    InstalldClient::GetInstance()->ResetInstalldProxy();
}

void BmsBundleInstallerTest::StopBundleService()
{
    if (bundleMgrService_ == nullptr) {
        return;
    }
    auto dataMgr = bundleMgrService_->GetDataMgr();
    EXPECT_NE(dataMgr, nullptr);
    dataMgr->UpdateBundleInstallState(BUNDLE_NAME, InstallState::UNINSTALL_START);
    dataMgr->UpdateBundleInstallState(BUNDLE_NAME, InstallState::UNINSTALL_SUCCESS);
    bundleMgrService_->OnStop();
    bundleMgrService_.reset();
}

void BmsBundleInstallerTest::CreateInstallerManager()
{
    if (manager_ != nullptr) {
        return;
    }
    auto installRunner = EventRunner::Create(INSTALL_THREAD);
    if (!installRunner) {
        return;
    }
    manager_ = std::make_shared<BundleInstallerManager>(installRunner);
    EXPECT_NE(nullptr, manager_);
}

void BmsBundleInstallerTest::ClearBundleInfoInDb()
{
    if (bundleMgrService_ == nullptr) {
        return;
    }
    auto dataMgt = bundleMgrService_->GetDataMgr();
    if (dataMgt == nullptr) {
        return;
    }
    auto dataStorage = dataMgt->GetDataStorage();
    if (dataStorage == nullptr) {
        return;
    }
    InnerBundleInfo innerBundleInfo;
    ApplicationInfo applicationInfo;
    applicationInfo.bundleName = BUNDLE_NAME;
    innerBundleInfo.SetBaseApplicationInfo(applicationInfo);
    bool result = dataStorage->DeleteStorageBundleInfo(Constants::CURRENT_DEVICE_ID, innerBundleInfo);
    EXPECT_TRUE(result) << "the bundle info in db clear fail: " << BUNDLE_NAME;
}

/**
 * @tc.number: SystemInstall_0200
 * @tc.name: test the wrong system bundle file can't be installed
 * @tc.desc: 1.the system bundle file don't exists
 *           2.the system bundle can't be installed and the result is fail
 */
HWTEST_F(BmsBundleInstallerTest, SystemInstall_0200, Function | SmallTest | Level0)
{
    std::string nonExistFile = RESOURCE_ROOT_PATH + INVALID_BUNDLE;
    bool result = InstallSystemBundle(nonExistFile);
    EXPECT_FALSE(result) << "the bundle file install success: " << nonExistFile;
    CheckFileNonExist();
}

/**
 * @tc.number: SystemInstall_0300
 * @tc.name: test the empty path can't be installed
 * @tc.desc: 1.the system bundle file path is empty
 *           2.the system bundle can't be installed and the result is fail
 */
HWTEST_F(BmsBundleInstallerTest, SystemInstall_0300, Function | SmallTest | Level0)
{
    bool result = InstallSystemBundle("");
    EXPECT_FALSE(result) << "the empty path install success";
    CheckFileNonExist();
}

/**
 * @tc.number: SystemInstall_0400
 * @tc.name: test the illegal bundleName file can't be installed
 * @tc.desc: 1.the system bundle name is illegal
 *           2.the system bundle can't be installed and the result is fail
 */
HWTEST_F(BmsBundleInstallerTest, SystemInstall_0400, Function | SmallTest | Level0)
{
    std::string wrongBundleName = RESOURCE_ROOT_PATH + WRONG_BUNDLE_NAME;
    bool result = InstallSystemBundle(wrongBundleName);
    EXPECT_FALSE(result) << "the wrong bundle file install success";
    CheckFileNonExist();
}

/**
 * @tc.number: SystemInstall_0500
 * @tc.name: test the error format bundle file can't be installed
 * @tc.desc: 1.the system bundle format is error
 *           2.the system bundle can't be installed and the result is fail
 */
HWTEST_F(BmsBundleInstallerTest, SystemInstall_0500, Function | SmallTest | Level0)
{
    std::string errorFormat = RESOURCE_ROOT_PATH + FORMAT_ERROR_BUNDLE;
    bool result = InstallSystemBundle(errorFormat);
    EXPECT_FALSE(result) << "the wrong format file install success";
    CheckFileNonExist();
}

/**
 * @tc.number: SystemInstall_0600
 * @tc.name: test the bundle file with invalid path will cause the result of install failure
 * @tc.desc: 1.the bundle file has invalid path
 *           2.the system bundle can't be installed and the result is fail
 */
HWTEST_F(BmsBundleInstallerTest, SystemInstall_0600, Function | SmallTest | Level0)
{
    std::string bundleFile = INVALID_PATH + RIGHT_BUNDLE;
    bool result = InstallSystemBundle(bundleFile);
    EXPECT_FALSE(result) << "the invalid path install success";
    CheckFileNonExist();
}

/**
 * @tc.number: SystemInstall_0700
 * @tc.name: test the install will fail when installd service has error
 * @tc.desc: 1.the installd service has error
 *           2.the install result is fail
 */
HWTEST_F(BmsBundleInstallerTest, SystemInstall_0700, Function | SmallTest | Level0)
{
    StopInstalldService();
    std::string bundleFile = RESOURCE_ROOT_PATH + RIGHT_BUNDLE;
    bool result = InstallSystemBundle(bundleFile);
    EXPECT_FALSE(result);
}

/**
 * @tc.number: SystemUpdateData_0400
 * @tc.name: test the already installing bundle can't be reinstalled and update its info to bms
 * @tc.desc: 1.the bundle is already installing.
 *           2.the already installing bundle can't be reinstalled and update its info to bms
 */
HWTEST_F(BmsBundleInstallerTest, SystemUpdateData_0400, Function | SmallTest | Level0)
{
    // prepare already install information.
    auto dataMgr = GetBundleDataMgr();
    EXPECT_NE(dataMgr, nullptr);
    dataMgr->UpdateBundleInstallState(BUNDLE_NAME, InstallState::INSTALL_START);
    // begin to  reinstall package
    std::string bundleFile = RESOURCE_ROOT_PATH + RIGHT_BUNDLE;
    bool installResult = InstallSystemBundle(bundleFile);
    EXPECT_FALSE(installResult);
    // reset the install state
    dataMgr->UpdateBundleInstallState(BUNDLE_NAME, InstallState::INSTALL_FAIL);
}

/**
 * @tc.number: CreateInstallTask_0200
 * @tc.name: test the installer manager can not create task while bundle invalid
 * @tc.desc: 1.the invalid bundle file exists
 *           2.install the invalid bundle failed
 */
HWTEST_F(BmsBundleInstallerTest, CreateInstallTask_0200, Function | SmallTest | Level0)
{
    CreateInstallerManager();
    sptr<MockStatusReceiver> receiver = new (std::nothrow) MockStatusReceiver();
    EXPECT_NE(receiver, nullptr);
    InstallParam installParam;
    std::string bundleFile = RESOURCE_ROOT_PATH + INVALID_BUNDLE;
    GetBundleInstallerManager()->CreateInstallTask(bundleFile, installParam, receiver);
    ErrCode result = receiver->GetResultCode();
    EXPECT_NE(ERR_OK, result);
}

/**
 * @tc.number: CreateUninstallTask_0200
 * @tc.name: test the installer manager can not create task while bundle invalid
 * @tc.desc: 1.the invalid bundle file exists
 *           2.uninstall the bundle failed
 */
HWTEST_F(BmsBundleInstallerTest, CreateUninstallTask_0200, Function | SmallTest | Level0)
{
    CreateInstallerManager();
    sptr<MockStatusReceiver> receiver = new (std::nothrow) MockStatusReceiver();
    EXPECT_NE(receiver, nullptr);
    InstallParam installParam;
    std::string bundleFile = RESOURCE_ROOT_PATH + INVALID_BUNDLE;
    GetBundleInstallerManager()->CreateUninstallTask(bundleFile, installParam, receiver);
    ErrCode result = receiver->GetResultCode();
    EXPECT_NE(ERR_OK, result);
}

/**
 * @tc.number: ThirdPartyInstall_0200
 * @tc.name: test the wrong third party bundle file can't be installed
 * @tc.desc: 1.the third party bundle file don't exists
 *           2.the third party bundle can't be installed and the result is fail
 */
HWTEST_F(BmsBundleInstallerTest, ThirdPartyInstall_0200, Function | SmallTest | Level0)
{
    std::string nonExistFile = RESOURCE_ROOT_PATH + INVALID_BUNDLE;
    ErrCode result = InstallThirdPartyBundle(nonExistFile);
    EXPECT_EQ(result, ERR_APPEXECFWK_INSTALL_FILE_PATH_INVALID);
    CheckFileNonExist();
}

/**
 * @tc.number: ThirdPartyInstall_0300
 * @tc.name: test the empty path can't be installed
 * @tc.desc: 1.the third party bundle file path is empty
 *           2.the third party bundle can't be installed and the result is fail
 */
HWTEST_F(BmsBundleInstallerTest, ThirdPartyInstall_0300, Function | SmallTest | Level0)
{
    ErrCode result = InstallThirdPartyBundle("");
    EXPECT_EQ(result, ERR_APPEXECFWK_INSTALL_FILE_PATH_INVALID);
    CheckFileNonExist();
}

/**
 * @tc.number: ThirdPartyInstall_0400
 * @tc.name: test the illegal bundleName file can't be installed
 * @tc.desc: 1.the third party bundle name is illegal
 *           2.the third party bundle can't be installed and the result is fail
 */
HWTEST_F(BmsBundleInstallerTest, ThirdPartyInstall_0400, Function | SmallTest | Level0)
{
    std::string wrongBundleName = RESOURCE_ROOT_PATH + WRONG_BUNDLE_NAME;
    ErrCode result = InstallThirdPartyBundle(wrongBundleName);
    EXPECT_EQ(result, ERR_APPEXECFWK_INSTALL_INVALID_HAP_NAME);
    CheckFileNonExist();
}

/**
 * @tc.number: ThirdPartyInstall_0600
 * @tc.name: test the bundle file with invalid path will cause the result of install failure
 * @tc.desc: 1.the bundle file has invalid path
 *           2.the third party bundle can't be installed and the result is fail
 */
HWTEST_F(BmsBundleInstallerTest, ThirdPartyInstall_0600, Function | SmallTest | Level0)
{
    std::string bundleFile = INVALID_PATH + RIGHT_BUNDLE;
    ErrCode result = InstallThirdPartyBundle(bundleFile);
    EXPECT_EQ(result, ERR_APPEXECFWK_INSTALL_FILE_PATH_INVALID);
    CheckFileNonExist();
}

/**
 * @tc.number: ThirdPartyUpdateData_0200
 * @tc.name: test the wrong bundle file can't be installed and its info will not updated to bms
 * @tc.desc: 1.the ThirdParty bundle is wrong
 *           2.the wrong bundle can't be installed and its info will not updated to bms
 */
HWTEST_F(BmsBundleInstallerTest, ThirdPartyUpdateData_0200, Function | SmallTest | Level0)
{
    ApplicationInfo info;
    auto dataMgr = GetBundleDataMgr();
    EXPECT_NE(dataMgr, nullptr);
    bool result = dataMgr->GetApplicationInfo(BUNDLE_NAME, ApplicationFlag::GET_BASIC_APPLICATION_INFO, USERID, info);
    EXPECT_FALSE(result);
    std::string wrongBundleName = RESOURCE_ROOT_PATH + WRONG_BUNDLE_NAME;
    ErrCode installResult = InstallThirdPartyBundle(wrongBundleName);
    EXPECT_EQ(installResult, ERR_APPEXECFWK_INSTALL_INVALID_HAP_NAME);
    result = dataMgr->GetApplicationInfo(BUNDLE_NAME, ApplicationFlag::GET_BASIC_APPLICATION_INFO, USERID, info);
    EXPECT_FALSE(result);
}