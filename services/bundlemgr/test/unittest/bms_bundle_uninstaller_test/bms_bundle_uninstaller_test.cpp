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

#include <chrono>
#include <thread>
#include <fstream>
#include <sys/stat.h>
#include <unistd.h>
#include <gtest/gtest.h>

#include "directory_ex.h"
#include "appexecfwk_errors.h"
#include "bundle_info.h"
#include "bundle_installer_host.h"
#include "bundle_mgr_service.h"
#include "bundle_data_storage_database.h"
#include "installd/installd_service.h"
#include "installd_client.h"
#include "mock_status_receiver.h"
#include "install_param.h"
#include "system_bundle_installer.h"

using namespace testing::ext;
using namespace std::chrono_literals;
using namespace OHOS;
using namespace OHOS::AppExecFwk;

namespace {

const std::string BUNDLE_NAME = "com.example.l3jsdemo";
const std::string MODULE_PACKAGE = "com.example.l3jsdemo";
const std::string MODULE_PACKAGE1 = "com.example.l3jsdemo1";
const std::string ERROR_BUNDLE_NAME = "com.example.bundle.uninstall.error";
const std::string ERROR_MODULE_PACKAGE_NAME = "com.example.module.uninstall.error";
const std::string BUNDLE_FILE_PATH = "/data/test/resource/bms/uninstall_bundle/right.hap";
const std::string BUNDLE_FILE_PATH1 = "/data/test/resource/bms/uninstall_bundle/right1.hap";
const std::string BUNDLE_DATA_DIR = "/data/accounts/account_0/appdata/com.example.l3jsdemo";
const std::string BUNDLE_CODE_DIR = "/data/accounts/account_0/applications/com.example.l3jsdemo";
const std::string MODULE_DATA_DIR = "/data/accounts/account_0/appdata/com.example.l3jsdemo/com.example.l3jsdemo";
const std::string MODULE_CODE_DIR = "/data/accounts/account_0/applications/com.example.l3jsdemo/com.example.l3jsdemo";
const std::string MODULE_DATA_DIR1 = "/data/accounts/account_0/appdata/com.example.l3jsdemo/com.example.l3jsdemo1";
const std::string MODULE_CODE_DIR1 = "/data/accounts/account_0/applications/com.example.l3jsdemo/com.example.l3jsdemo1";
const std::string ROOT_DIR = "/data/accounts";
const std::string DB_FILE_PATH = "/data/bundlemgr";
const int32_t ROOT_UID = 0;

}  // namespace

class BmsBundleUninstallerTest : public testing::Test {
public:
    BmsBundleUninstallerTest();
    ~BmsBundleUninstallerTest();

    static void SetUpTestCase();
    static void TearDownTestCase();
    void SetUp();
    void TearDown();

    ErrCode InstallBundle(const std::string &bundlePath) const;
    ErrCode UninstallBundle(const std::string &bundleName) const;
    ErrCode UninstallModule(const std::string &bundleName, const std::string &modulePackage) const;
    void CheckFileExist() const;
    void CheckModuleFileExist() const;
    void CheckModuleFileExist1() const;
    void CheckFileNonExist() const;
    void CheckModuleFileNonExist() const;
    void StopInstalldService() const;
    void StartInstalldService() const;
    void StartBundleService();
    void StopBundleService();
    void CheckBundleInfoExist() const;
    void CheckBundleInfoNonExist() const;
    const std::shared_ptr<BundleMgrService> GetBundleMgrService() const;
    void ClearBundleInfoInDb();
    void DeleteInstallFiles();
private:
    std::shared_ptr<InstalldService> installdService_ = std::make_unique<InstalldService>();
    std::shared_ptr<BundleMgrService> bundleMgrService_ = DelayedSingleton<BundleMgrService>::GetInstance();
};

BmsBundleUninstallerTest::BmsBundleUninstallerTest()
{}

BmsBundleUninstallerTest::~BmsBundleUninstallerTest()
{}

void BmsBundleUninstallerTest::SetUpTestCase()
{
    if (access(ROOT_DIR.c_str(), F_OK) != 0) {
        bool result = OHOS::ForceCreateDirectory(ROOT_DIR);
        EXPECT_TRUE(result);
    }
    if (chown(ROOT_DIR.c_str(), ROOT_UID, ROOT_UID) != 0) {
        EXPECT_TRUE(false);
    }
    if (chmod(ROOT_DIR.c_str(), S_IRWXU | S_IRGRP | S_IXGRP | S_IROTH | S_IXOTH) != 0) {
        EXPECT_TRUE(false);
    }
}

void BmsBundleUninstallerTest::TearDownTestCase()
{}

void BmsBundleUninstallerTest::SetUp()
{
    StartBundleService();
    StartInstalldService();
}

void BmsBundleUninstallerTest::TearDown()
{
    StopInstalldService();
    StopBundleService();
}

ErrCode BmsBundleUninstallerTest::InstallBundle(const std::string &bundlePath) const
{
    if (!bundleMgrService_) {
        return ERR_APPEXECFWK_INSTALL_INTERNAL_ERROR;
    }
    auto installer = bundleMgrService_->GetBundleInstaller();
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
    bool result = installer->Install(bundlePath, installParam, receiver);
    EXPECT_TRUE(result);
    return receiver->GetResultCode();
}

ErrCode BmsBundleUninstallerTest::UninstallBundle(const std::string &bundleName) const
{
    if (!bundleMgrService_) {
        return ERR_APPEXECFWK_UNINSTALL_BUNDLE_MGR_SERVICE_ERROR;
    }
    auto installer = bundleMgrService_->GetBundleInstaller();
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
    bool result = installer->Uninstall(bundleName, installParam, receiver);
    EXPECT_TRUE(result);
    return receiver->GetResultCode();
}

ErrCode BmsBundleUninstallerTest::UninstallModule(const std::string &bundleName, const std::string &modulePackage) const
{
    if (!bundleMgrService_) {
        return ERR_APPEXECFWK_UNINSTALL_BUNDLE_MGR_SERVICE_ERROR;
    }
    auto installer = bundleMgrService_->GetBundleInstaller();
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
    bool result = installer->Uninstall(bundleName, modulePackage, installParam, receiver);
    EXPECT_TRUE(result);
    return receiver->GetResultCode();
}

void BmsBundleUninstallerTest::CheckFileExist() const
{
    int bundleDataExist = access(BUNDLE_DATA_DIR.c_str(), F_OK);
    EXPECT_EQ(bundleDataExist, 0);

    int bundleCodeExist = access(BUNDLE_CODE_DIR.c_str(), F_OK);
    EXPECT_EQ(bundleCodeExist, 0);
}

void BmsBundleUninstallerTest::CheckModuleFileExist() const
{
    int moduleDataExist = access(MODULE_DATA_DIR.c_str(), F_OK);
    EXPECT_EQ(moduleDataExist, 0);

    int moduleCodeExist = access(MODULE_CODE_DIR.c_str(), F_OK);
    EXPECT_EQ(moduleCodeExist, 0);
}

void BmsBundleUninstallerTest::CheckModuleFileExist1() const
{
    int moduleDataExist1 = access(MODULE_DATA_DIR1.c_str(), F_OK);
    EXPECT_EQ(moduleDataExist1, 0);

    int moduleCodeExist1 = access(MODULE_CODE_DIR1.c_str(), F_OK);
    EXPECT_EQ(moduleCodeExist1, 0);
}

void BmsBundleUninstallerTest::CheckFileNonExist() const
{
    int bundleDataExist = access(BUNDLE_DATA_DIR.c_str(), F_OK);
    EXPECT_NE(bundleDataExist, 0);

    int bundleCodeExist = access(BUNDLE_CODE_DIR.c_str(), F_OK);
    EXPECT_NE(bundleCodeExist, 0);
}

void BmsBundleUninstallerTest::CheckModuleFileNonExist() const
{
    int moduleDataExist = access(MODULE_DATA_DIR.c_str(), F_OK);
    EXPECT_NE(moduleDataExist, 0);

    int moduleCodeExist = access(MODULE_CODE_DIR.c_str(), F_OK);
    EXPECT_NE(moduleCodeExist, 0);
}

void BmsBundleUninstallerTest::StopInstalldService() const
{
    installdService_->Stop();
    InstalldClient::GetInstance()->ResetInstalldProxy();
}

void BmsBundleUninstallerTest::StartInstalldService() const
{
    installdService_->Start();
}

void BmsBundleUninstallerTest::StartBundleService()
{
    if (!bundleMgrService_) {
        bundleMgrService_ = DelayedSingleton<BundleMgrService>::GetInstance();
    }
    if (bundleMgrService_) {
        bundleMgrService_->OnStart();
    }
}

void BmsBundleUninstallerTest::StopBundleService()
{
    if (bundleMgrService_) {
        bundleMgrService_->OnStop();
        bundleMgrService_ = nullptr;
    }
}

void BmsBundleUninstallerTest::CheckBundleInfoExist() const
{
    EXPECT_NE(bundleMgrService_, nullptr);
    auto dataMgr = bundleMgrService_->GetDataMgr();
    EXPECT_NE(dataMgr, nullptr);
    BundleInfo info;
    bool isBundleExist = dataMgr->GetBundleInfo(BUNDLE_NAME, BundleFlag::GET_BUNDLE_DEFAULT, info);
    EXPECT_TRUE(isBundleExist);
}

void BmsBundleUninstallerTest::CheckBundleInfoNonExist() const
{
    EXPECT_NE(bundleMgrService_, nullptr);
    auto dataMgr = bundleMgrService_->GetDataMgr();
    EXPECT_NE(dataMgr, nullptr);
    BundleInfo info;
    bool isBundleExist = dataMgr->GetBundleInfo(BUNDLE_NAME, BundleFlag::GET_BUNDLE_DEFAULT, info);
    EXPECT_FALSE(isBundleExist);
}

const std::shared_ptr<BundleMgrService> BmsBundleUninstallerTest::GetBundleMgrService() const
{
    return bundleMgrService_;
}

void BmsBundleUninstallerTest::ClearBundleInfoInDb()
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

void BmsBundleUninstallerTest::DeleteInstallFiles()
{
    DelayedSingleton<BundleMgrService>::DestroyInstance();
    OHOS::ForceRemoveDirectory(BUNDLE_DATA_DIR);
    OHOS::ForceRemoveDirectory(BUNDLE_CODE_DIR);
    ClearBundleInfoInDb();
    bundleMgrService_ = DelayedSingleton<BundleMgrService>::GetInstance();
}

/**
 * @tc.number: Bundle_Uninstall_0200
 * @tc.name: test the empty bundle name will return fail
 * @tc.desc: 1. the bundle name is empty
 *           2. the empty bundle name will return fail
 */
HWTEST_F(BmsBundleUninstallerTest, Bundle_Uninstall_0200, Function | SmallTest | Level0)
{
    ErrCode result = UninstallBundle("");
    EXPECT_EQ(result, ERR_APPEXECFWK_UNINSTALL_INVALID_NAME);
}

/**
 * @tc.number: Bundle_Uninstall_0300
 * @tc.name: test the error bundle name will return fail
 * @tc.desc: 1. the bundle name is error
 *           2. the error bundle name will return fail
 */
HWTEST_F(BmsBundleUninstallerTest, Bundle_Uninstall_0300, Function | SmallTest | Level0)
{
    ErrCode result = UninstallBundle(ERROR_BUNDLE_NAME);
    EXPECT_EQ(result, ERR_APPEXECFWK_UNINSTALL_MISSING_INSTALLED_BUNDLE);
}

/**
 * @tc.number: Module_Uninstall_0200
 * @tc.name: test the installed Module can be uninstalled
 * @tc.desc: 1. the Module name is empty
 *           2. the empty Module name will return fail
 */
HWTEST_F(BmsBundleUninstallerTest, Module_Uninstall_0200, Function | SmallTest | Level0)
{
    ErrCode result = UninstallModule(BUNDLE_NAME, "");
    EXPECT_EQ(result, ERR_APPEXECFWK_UNINSTALL_INVALID_NAME);
}

/**
 * @tc.number: Module_Uninstall_0300
 * @tc.name: test the error Module name will return fail.
 * @tc.desc: 1. the Module name is error.
 *           2. the error Module name will return fail.
 */
HWTEST_F(BmsBundleUninstallerTest, Module_Uninstall_0300, Function | SmallTest | Level0)
{
    ErrCode result = UninstallModule(BUNDLE_NAME, ERROR_MODULE_PACKAGE_NAME);
    EXPECT_EQ(result, ERR_APPEXECFWK_UNINSTALL_MISSING_INSTALLED_BUNDLE);
}
