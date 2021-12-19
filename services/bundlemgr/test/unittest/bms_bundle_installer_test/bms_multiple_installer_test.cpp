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
#include <cstdio>
#include <dirent.h>
#include <fcntl.h>
#include <fstream>
#include <gtest/gtest.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>

#include "bundle_data_storage_database.h"
#include "bundle_info.h"
#include "bundle_installer_host.h"
#include "bundle_mgr_service.h"
#include "directory_ex.h"
#include "install_param.h"
#include "installd/installd_service.h"
#include "installd_client.h"
#include "mock_status_receiver.h"
#include "system_bundle_installer.h"

using namespace testing::ext;
using namespace std::chrono_literals;
using namespace OHOS::AppExecFwk;
using namespace OHOS::DistributedKv;
using namespace OHOS;
using OHOS::DelayedSingleton;

namespace {
const std::string BUNDLE_NAME = "com.example.l3jsdemo";
const std::string RESOURCE_ROOT_PATH = "/data/test/resource/bms/install_bundle/";
const std::string RESOURCE_TEST_PATH = "/data/test/resource/bms/install_bundle/test/";
const std::string INVALID_PATH = "/install_bundle/";
const std::string RIGHT_BUNDLE = "right.hap";
const std::string RIGHT_BUNDLE_FIRST = "first_right.hap";
const std::string RIGHT_BUNDLE_SECOND = "second_right.hap";
const std::string RIGHT_BUNDLE_THIRD = "third_right.hap";
const std::string RIGHT_BUNDLE_FOURTH = "fourth_right.hap";
const std::string RIGHT_BUNDLE_FIFTH = "fifth_right.hap";
const std::string RIGHT_BUNDLE_SIXTH = "sixth_right.hap";
const std::string RIGHT_BUNDLE_SEVENTH = "seventh_right.hap";
const std::string RIGHT_BUNDLE_TWELFTH = "twelfth_right.hap";
const std::string INVALID_BUNDLE = "nonfile.hap";
const std::string FORMAT_ERROR_BUNDLE = "format_error_profile.hap";
const std::string WRONG_BUNDLE_NAME = "wrong_bundle_name.ha";
const std::string ERROR_BUNDLE_PROFILE_FILE = "error_bundle_profile.hap";
const std::string BUNDLE_DATA_DIR = "/data/accounts/account_0/appdata/com.example.l3jsdemo";
const std::string BUNDLE_CODE_DIR = "/data/accounts/account_0/applications/com.example.l3jsdemo";
const std::string PACKAGE_NAME_FIRST = "com.example.l3jsdemo";
const std::string PACKAGE_NAME_SECOND = "com.example.l2jsdemo";
const std::string PACKAGE_NAME_THIRD = "com.example.l1jsdemo";
const std::string ROOT_DIR = "/data/accounts";
const int32_t ROOT_UID = 0;
const int32_t USERID = 0;
const std::string INSTALL_THREAD = "TestInstall";
const int32_t NUMBER_OF_PACKAGE = 2;
}  // namespace

class BmsMultipleInstallerTest : public testing::Test {
public:
    BmsMultipleInstallerTest();
    ~BmsMultipleInstallerTest();
    static void SetUpTestCase();
    static void TearDownTestCase();
    void SetUp();
    void TearDown();
    ErrCode InstallThirdPartyBundle(const std::string &filePath) const;
    ErrCode UpdateThirdPartyBundle(const std::string &filePath) const;
    ErrCode InstallThirdPartyMultipleBundles(const std::vector<std::string> &filePaths, bool &&flag) const;
    void CheckFileExist() const;
    void CheckFileNonExist() const;
    const std::shared_ptr<BundleDataMgr> GetBundleDataMgr() const;
    void StopInstalldService() const;
    void StopBundleService();
    void ClearBundleInfo() const;
    void CheckMultipleFileExist(const std::string &packageName) const;
    void IsContainModuleInfo(const BundleInfo &info, const std::string &packageName1,
        const std::string &packageName2) const;

private:
    std::shared_ptr<InstalldService> installdService_ = std::make_shared<InstalldService>();
    std::shared_ptr<BundleMgrService> bundleMgrService_ = DelayedSingleton<BundleMgrService>::GetInstance();
};

BmsMultipleInstallerTest::BmsMultipleInstallerTest()
{}

BmsMultipleInstallerTest::~BmsMultipleInstallerTest()
{}

ErrCode BmsMultipleInstallerTest::InstallThirdPartyBundle(const std::string &filePath) const
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

ErrCode BmsMultipleInstallerTest::InstallThirdPartyMultipleBundles(const std::vector<std::string> &filePaths,
    bool &&flag) const
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
    installParam.installFlag = flag ? InstallFlag::NORMAL : InstallFlag::REPLACE_EXISTING;
    bool result = installer->Install(filePaths, installParam, receiver);
    EXPECT_TRUE(result);
    return receiver->GetResultCode();
}

ErrCode BmsMultipleInstallerTest::UpdateThirdPartyBundle(const std::string &filePath) const
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

void BmsMultipleInstallerTest::SetUpTestCase()
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

void BmsMultipleInstallerTest::TearDownTestCase()
{
}

void BmsMultipleInstallerTest::SetUp()
{
    if (!installdService_->IsServiceReady()) {
        installdService_->Start();
    }
    if (!bundleMgrService_->IsServiceReady()) {
        bundleMgrService_->OnStart();
    }
}

void BmsMultipleInstallerTest::TearDown()
{
    OHOS::ForceRemoveDirectory(BUNDLE_DATA_DIR);
    OHOS::ForceRemoveDirectory(BUNDLE_CODE_DIR);
}

void BmsMultipleInstallerTest::CheckFileExist() const
{
    int bundleCodeExist = access(BUNDLE_CODE_DIR.c_str(), F_OK);
    EXPECT_EQ(bundleCodeExist, 0) << "the bundle code dir does not exists: " << BUNDLE_CODE_DIR;

    int bundleDataExist = access(BUNDLE_DATA_DIR.c_str(), F_OK);
    EXPECT_EQ(bundleDataExist, 0) << "the bundle data dir does not exists: " << BUNDLE_DATA_DIR;
}

void BmsMultipleInstallerTest::CheckFileNonExist() const
{
    int bundleCodeExist = access(BUNDLE_CODE_DIR.c_str(), F_OK);
    EXPECT_NE(bundleCodeExist, 0) << "the bundle code dir exists: " << BUNDLE_CODE_DIR;

    int bundleDataExist = access(BUNDLE_DATA_DIR.c_str(), F_OK);
    EXPECT_NE(bundleDataExist, 0) << "the bundle data dir exists: " << BUNDLE_DATA_DIR;
}

void BmsMultipleInstallerTest::CheckMultipleFileExist(const std::string &packageName) const
{
    int moduleCodeExist = access((BUNDLE_CODE_DIR + "/" + packageName).c_str(), F_OK);
    int moduleDataExist = access((BUNDLE_DATA_DIR + "/" + packageName).c_str(), F_OK);
    EXPECT_EQ(moduleCodeExist, 0);
    EXPECT_EQ(moduleDataExist, 0);
}

void BmsMultipleInstallerTest::IsContainModuleInfo(const BundleInfo &info, const std::string &packageName1,
    const std::string &packageName2) const
{
    EXPECT_NE(info.hapModuleNames.size(), 0);

    int cnt = 0;
    for_each(info.hapModuleNames.begin(), info.hapModuleNames.end(), [&](const auto &package)->decltype(auto) {
        if (package == packageName1 || package == packageName2) {
            cnt++;
        }
    });
    EXPECT_EQ(cnt, NUMBER_OF_PACKAGE);
}

const std::shared_ptr<BundleDataMgr> BmsMultipleInstallerTest::GetBundleDataMgr() const
{
    return bundleMgrService_->GetDataMgr();
}

void BmsMultipleInstallerTest::StopInstalldService() const
{
    if (installdService_->IsServiceReady()) {
        installdService_->Stop();
        InstalldClient::GetInstance()->ResetInstalldProxy();
    }
}

void BmsMultipleInstallerTest::StopBundleService()
{
    if (bundleMgrService_->IsServiceReady()) {
        bundleMgrService_->OnStop();
        bundleMgrService_.reset();
    }
}

void BmsMultipleInstallerTest::ClearBundleInfo() const
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

    // clear innerBundleInfo from data manager
    dataMgt->UpdateBundleInstallState(BUNDLE_NAME, InstallState::UNINSTALL_START);
    dataMgt->UpdateBundleInstallState(BUNDLE_NAME, InstallState::UNINSTALL_SUCCESS);

    InnerBundleInfo innerBundleInfo;
    ApplicationInfo applicationInfo;
    applicationInfo.bundleName = BUNDLE_NAME;
    innerBundleInfo.SetBaseApplicationInfo(applicationInfo);
    // clear innerBundleInfo from data storage
    bool result = dataStorage->DeleteStorageBundleInfo(Constants::CURRENT_DEVICE_ID, innerBundleInfo);
    EXPECT_TRUE(result) << "the bundle info in db clear fail: " << BUNDLE_NAME;
}

/**
 * @tc.number: ThirdPartyInstall_0100
 * @tc.name: test the right third party bundle file can be installed
 * @tc.desc: 1.the third party bundle file exists
 *           2.the third party bundle can be installed successfully and can get the bundle info
 * @tc.require: AR000GJ4KF
 */
HWTEST_F(BmsMultipleInstallerTest, ThirdPartyInstall_0100, Function | SmallTest | Level0)
{
    std::string bundleFile = RESOURCE_ROOT_PATH + RIGHT_BUNDLE_FIRST;
    ErrCode result = InstallThirdPartyBundle(bundleFile);
    EXPECT_EQ(result, ERR_OK);
    CheckFileExist();
    ClearBundleInfo();
}

/**
 * @tc.number: ThirdPartyInstall_0200
 * @tc.name: test the wrong third party bundle file can't be installed
 * @tc.desc: 1.the third party bundle file don't exists
 *           2.the third party bundle can't be installed and the result is fail
 * @tc.require: AR000GJ4KF
 */
HWTEST_F(BmsMultipleInstallerTest, ThirdPartyInstall_0200, Function | SmallTest | Level0)
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
 * @tc.require: AR000GJ4KF
 */
HWTEST_F(BmsMultipleInstallerTest, ThirdPartyInstall_0300, Function | SmallTest | Level0)
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
 * @tc.require: AR000GJ4KF
 */
HWTEST_F(BmsMultipleInstallerTest, ThirdPartyInstall_0400, Function | SmallTest | Level0)
{
    std::string wrongBundleName = RESOURCE_ROOT_PATH + WRONG_BUNDLE_NAME;
    ErrCode result = InstallThirdPartyBundle(wrongBundleName);
    EXPECT_EQ(result, ERR_APPEXECFWK_INSTALL_INVALID_HAP_NAME);
    CheckFileNonExist();
}

/**
 * @tc.number: ThirdPartyInstall_0500
 * @tc.name: test the error format bundle file can't be installed
 * @tc.desc: 1.the third party bundle format is error
 *           2.the third party bundle can't be installed and the result is fail
 * @tc.require: AR000GJ4KF
 */
HWTEST_F(BmsMultipleInstallerTest, ThirdPartyInstall_0500, Function | SmallTest | Level0)
{
    std::string errorFormat = RESOURCE_ROOT_PATH + FORMAT_ERROR_BUNDLE;
    ErrCode result = InstallThirdPartyBundle(errorFormat);
    EXPECT_EQ(result, ERR_APPEXECFWK_PARSE_NO_PROFILE);
    CheckFileNonExist();
}

/**
 * @tc.number: ThirdPartyInstall_0600
 * @tc.name: test the bundle file with invalid path will cause the result of install failure
 * @tc.desc: 1.the bundle file has invalid path
 *           2.the third party bundle can't be installed and the result is fail
 * @tc.require: AR000GJ4KF
 */
HWTEST_F(BmsMultipleInstallerTest, ThirdPartyInstall_0600, Function | SmallTest | Level0)
{
    std::string bundleFile = INVALID_PATH + RIGHT_BUNDLE;
    ErrCode result = InstallThirdPartyBundle(bundleFile);
    EXPECT_EQ(result, ERR_APPEXECFWK_INSTALL_FILE_PATH_INVALID);
    CheckFileNonExist();
}

/**
 * @tc.number: ThirdPartyInstall_0700
 * @tc.name: test the install will fail when installd service has error
 * @tc.desc: 1.the installd service has error
 *           2.the install result is fail
 * @tc.require: AR000GJ4KF
 */
HWTEST_F(BmsMultipleInstallerTest, ThirdPartyInstall_0700, Function | SmallTest | Level0)
{
    StopInstalldService();
    std::string bundleFile = RESOURCE_ROOT_PATH + RIGHT_BUNDLE;
    ErrCode result = InstallThirdPartyBundle(bundleFile);
    EXPECT_EQ(result, ERR_APPEXECFWK_INSTALLD_GET_PROXY_ERROR);
}

/**
 * @tc.number: ThirdPartyUpdateData_0100
 * @tc.name: test the right bundle file can be installed and update its info to bms
 * @tc.desc: 1.the ThirdParty bundle is available
 *           2.the right bundle can be installed and update its info to bms
 * @tc.require: AR000GJ4KF
 */
HWTEST_F(BmsMultipleInstallerTest, ThirdPartyUpdateData_0100, Function | SmallTest | Level0)
{
    ApplicationInfo info;
    auto dataMgr = GetBundleDataMgr();
    EXPECT_NE(dataMgr, nullptr);
    bool result = dataMgr->GetApplicationInfo(BUNDLE_NAME, ApplicationFlag::GET_BASIC_APPLICATION_INFO, USERID, info);
    EXPECT_FALSE(result);
    std::string bundleFile = RESOURCE_ROOT_PATH + RIGHT_BUNDLE_FIRST;
    ErrCode installResult = InstallThirdPartyBundle(bundleFile);
    EXPECT_EQ(installResult, ERR_OK);
    result = dataMgr->GetApplicationInfo(BUNDLE_NAME, ApplicationFlag::GET_BASIC_APPLICATION_INFO, USERID, info);
    EXPECT_TRUE(result);
    EXPECT_EQ(info.name, BUNDLE_NAME);
    ClearBundleInfo();
}

/**
 * @tc.number: ThirdPartyUpdateData_0200
 * @tc.name: test the wrong bundle file can't be installed and its info will not updated to bms
 * @tc.desc: 1.the ThirdParty bundle is wrong
 *           2.the wrong bundle can't be installed and its info will not updated to bms
 * @tc.require: AR000GJ4KF
 */
HWTEST_F(BmsMultipleInstallerTest, ThirdPartyUpdateData_0200, Function | SmallTest | Level0)
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

/**
 * @tc.number: ThirdPartyUpdateData_0300
 * @tc.name: test the already installed bundle can't be reinstalled and update its info to bms
 * @tc.desc: 1.the bundle is already installed
 *           2.the already installed  bundle can't be reinstalled and update its info to bms
 * @tc.require: AR000GJ4KF
 */
HWTEST_F(BmsMultipleInstallerTest, ThirdPartyUpdateData_0300, Function | SmallTest | Level0)
{
    auto dataMgr = GetBundleDataMgr();
    EXPECT_NE(dataMgr, nullptr);
    // prepare already install information.
    std::string bundleFile = RESOURCE_ROOT_PATH + RIGHT_BUNDLE_FIRST;
    ErrCode firstInstall = InstallThirdPartyBundle(bundleFile);
    EXPECT_EQ(firstInstall, ERR_OK);
    ApplicationInfo info;
    auto result = dataMgr->GetApplicationInfo(BUNDLE_NAME, ApplicationFlag::GET_BASIC_APPLICATION_INFO, USERID, info);
    EXPECT_TRUE(result);
    EXPECT_EQ(info.name, BUNDLE_NAME);
    ErrCode secondInstall = InstallThirdPartyBundle(bundleFile);
    EXPECT_EQ(secondInstall, ERR_APPEXECFWK_INSTALL_ALREADY_EXIST);
    ClearBundleInfo();
}

/**
 * @tc.number: ThirdPartyUpdateData_0400
 * @tc.name: test the already installed bundle can be reinstalled and update its info to bms
 * @tc.desc: 1.the bundle is already installed
 *           2.the already installed  bundle can be reinstalled and update its info to bms
 * @tc.require: AR000GJ4KF
 */
HWTEST_F(BmsMultipleInstallerTest, ThirdPartyUpdateData_0400, Function | SmallTest | Level0)
{
    auto dataMgr = GetBundleDataMgr();
    EXPECT_NE(dataMgr, nullptr);
    // prepare already install information.
    std::string bundleFile = RESOURCE_ROOT_PATH + RIGHT_BUNDLE_FIRST;
    ErrCode firstInstall = InstallThirdPartyBundle(bundleFile);
    EXPECT_EQ(firstInstall, ERR_OK);
    ApplicationInfo info;
    auto result = dataMgr->GetApplicationInfo(BUNDLE_NAME, ApplicationFlag::GET_BASIC_APPLICATION_INFO, USERID, info);
    EXPECT_TRUE(result);
    EXPECT_EQ(info.name, BUNDLE_NAME);
    ErrCode secondInstall = UpdateThirdPartyBundle(bundleFile);
    EXPECT_EQ(secondInstall, ERR_OK);
    ClearBundleInfo();
}

/**
 * @tc.number: ThirdPartyUpdateData_0500
 * @tc.name: test the already installing bundle can't be reinstalled and update its info to bms
 * @tc.desc: 1.the bundle is already installing
 *           2.the already installing bundle can't be reinstalled and update its info to bms
 * @tc.require: AR000GJ4KF
 */
HWTEST_F(BmsMultipleInstallerTest, ThirdPartyUpdateData_0500, Function | SmallTest | Level0)
{
    // prepare already install information.
    auto dataMgr = GetBundleDataMgr();
    EXPECT_NE(dataMgr, nullptr);
    dataMgr->UpdateBundleInstallState(BUNDLE_NAME, InstallState::INSTALL_START);
    // begin to  reinstall package
    std::string bundleFile = RESOURCE_ROOT_PATH + RIGHT_BUNDLE;
    ErrCode installResult = InstallThirdPartyBundle(bundleFile);
    EXPECT_EQ(installResult, ERR_APPEXECFWK_INSTALL_STATE_ERROR);
    // reset the install state
    dataMgr->UpdateBundleInstallState(BUNDLE_NAME, InstallState::INSTALL_FAIL);
}

/**
 * @tc.number: MultipleHapsInstall_0100
 * @tc.name: test the input array of the input file paths is empty, the installation is failed.
 * @tc.desc: 1.the input array is empty.
 *           2 the installation is failed.
 * @tc.require: AR000GJ4KF
 */
HWTEST_F(BmsMultipleInstallerTest, MultipleHapsInstall_0100, Function | SmallTest | Level0)
{
    std::vector<std::string> filePaths;
    auto installRes = InstallThirdPartyMultipleBundles(filePaths, true);
    EXPECT_EQ(installRes, ERR_APPEXECFWK_INSTALL_FILE_PATH_INVALID) << "installation is successful";
}

/**
 * @tc.number: MultipleHapsInstall_0200
 * @tc.name: test the input array of the input file paths has some empty paths, the installation is failed.
 * @tc.desc: 1.there is empty path in the input file path array.
 *           2.the installation result is failed.
 * @tc.require: AR000GJ4KF
 */
HWTEST_F(BmsMultipleInstallerTest, MultipleHapsInstall_0200, Function | SmallTest | Level0)
{
    std::vector<std::string> filePaths;
    auto bundleFile = RESOURCE_ROOT_PATH + RIGHT_BUNDLE_FIRST;
    filePaths.emplace_back(bundleFile);
    filePaths.emplace_back("");
    auto installRes = InstallThirdPartyMultipleBundles(filePaths, true);
    EXPECT_EQ(installRes, ERR_APPEXECFWK_INSTALL_FILE_PATH_INVALID) << "installation is successful";
}

/**
 * @tc.number: MultipleHapsInstall_0300
 * @tc.name: test the input array only has one valid file path
 * @tc.desc: 1.only one valid path in the input file path array
 *           2.the installation result is successful
 * @tc.require: AR000GJ4KF
 */
HWTEST_F(BmsMultipleInstallerTest, MultipleHapsInstall_0300, Function | SmallTest | Level0)
{
    std::vector<std::string> filePaths;
    auto bundleFile = RESOURCE_ROOT_PATH + RIGHT_BUNDLE_FIRST;
    filePaths.emplace_back(bundleFile);
    auto installRes = InstallThirdPartyMultipleBundles(filePaths, true);
    EXPECT_EQ(installRes, ERR_OK) << "installation is failed";
    CheckFileExist();
    ClearBundleInfo();
}

/**
 * @tc.number: MultipleHapsInstall_0400
 * @tc.name: test the wrong third party bundle file can't be installed
 * @tc.desc: 1.the third party bundle file don't exists
 *           2.the third party bundle can't be installed and the result is fail
 * @tc.require: AR000GJ4KF
 */
HWTEST_F(BmsMultipleInstallerTest, MultipleHapsInstall_0400, Function | SmallTest | Level0)
{
    std::vector<std::string> filePaths;
    std::string nonExistFile = RESOURCE_ROOT_PATH + INVALID_BUNDLE;
    filePaths.emplace_back(nonExistFile);
    ErrCode installRes = InstallThirdPartyMultipleBundles(filePaths, true);
    EXPECT_EQ(installRes, ERR_APPEXECFWK_INSTALL_FILE_PATH_INVALID) << "installation is successful";
    CheckFileNonExist();
}

/**
 * @tc.number: MultipleHapsInstall_0500
 * @tc.name: test the illegal bundleName file can't be installed
 * @tc.desc: 1.the third party bundle name is illegal
 *           2.the third party bundle can't be installed and the result is fail
 * @tc.require: AR000GJ4KF
 */
HWTEST_F(BmsMultipleInstallerTest, MultipleHapsInstall_0500, Function | SmallTest | Level0)
{
    std::vector<std::string> filePaths;
    std::string wrongBundleName = RESOURCE_ROOT_PATH + WRONG_BUNDLE_NAME;
    filePaths.emplace_back(wrongBundleName);
    ErrCode installRes = InstallThirdPartyMultipleBundles(filePaths, true);
    EXPECT_EQ(installRes, ERR_APPEXECFWK_INSTALL_INVALID_HAP_NAME);
    CheckFileNonExist();
}

/**
 * @tc.number: MultipleHapsInstall_0600
 * @tc.name: test the error format bundle file can't be installed
 * @tc.desc: 1.the third party bundle format is error
 *           2.the third party bundle can't be installed and the result is fail
 * @tc.require: AR000GJ4KF
 */
HWTEST_F(BmsMultipleInstallerTest, MultipleHapsInstall_0600, Function | SmallTest | Level0)
{
    std::vector<std::string> filePaths;
    std::string errorFormat = RESOURCE_ROOT_PATH + FORMAT_ERROR_BUNDLE;
    filePaths.emplace_back(errorFormat);
    ErrCode installRes = InstallThirdPartyMultipleBundles(filePaths, true);
    EXPECT_EQ(installRes, ERR_APPEXECFWK_PARSE_NO_PROFILE);
    CheckFileNonExist();
}

/**
 * @tc.number: MultipleHapsInstall_0700
 * @tc.name: test the bundle file with invalid path will cause the result of install failure
 * @tc.desc: 1.the bundle file has invalid path
 *           2.the third party bundle can't be installed and the result is fail
 * @tc.require: AR000GJ4KF
 */
HWTEST_F(BmsMultipleInstallerTest, MultipleHapsInstall_0700, Function | SmallTest | Level0)
{
    std::vector<std::string> filePaths;
    std::string bundleFile = INVALID_PATH + RIGHT_BUNDLE_FIRST;
    filePaths.emplace_back(bundleFile);
    ErrCode installRes = InstallThirdPartyMultipleBundles(filePaths, true);
    EXPECT_EQ(installRes, ERR_APPEXECFWK_INSTALL_FILE_PATH_INVALID);
    CheckFileNonExist();
}

/**
 * @tc.number: MultipleHapsInstall_0800
 * @tc.name: test the install will fail when installd service has error
 * @tc.desc: 1.the installd service has error
 *           2.the installation result is fail
 * @tc.require: AR000GJ4KF
 */
HWTEST_F(BmsMultipleInstallerTest, MultipleHapsInstall_0800, Function | SmallTest | Level0)
{
    StopInstalldService();
    std::vector<std::string> filePaths;
    std::string bundleFile = RESOURCE_ROOT_PATH + RIGHT_BUNDLE_FIRST;
    filePaths.emplace_back(bundleFile);
    ErrCode installRes = InstallThirdPartyMultipleBundles(filePaths, true);
    EXPECT_EQ(installRes, ERR_APPEXECFWK_INSTALLD_GET_PROXY_ERROR);
    CheckFileNonExist();
}

/**
 * @tc.number: MultipleHapsInstall_0900
 * @tc.name: test to install haps with different bundleName in the input file path array
 * @tc.desc: 1.the bundleName is differnet in the input file path array
 *           2.the installation result is fail
 * @tc.require: AR000GJ4KF
 */
HWTEST_F(BmsMultipleInstallerTest, MultipleHapsInstall_0900, Function | SmallTest | Level0)
{
    std::vector<std::string> filePaths;
    std::string firstBundleFile = RESOURCE_ROOT_PATH + RIGHT_BUNDLE_SECOND;
    std::string secondBundleFile = RESOURCE_ROOT_PATH + RIGHT_BUNDLE_THIRD;
    filePaths.emplace_back(firstBundleFile);
    filePaths.emplace_back(secondBundleFile);
    ErrCode installRes = InstallThirdPartyMultipleBundles(filePaths, true);
    EXPECT_EQ(installRes, ERR_APPEXECFWK_INSTALL_BUNDLENAME_NOT_SAME);
    CheckFileNonExist();
}

/**
 * @tc.number: MultipleHapsInstall_1000
 * @tc.name: test to install haps with different version code in the input file path array
 * @tc.desc: 1.the version code is differnet in the input file path array
 *           2.the installation result is fail
 * @tc.require: AR000GJ4KF
 */
HWTEST_F(BmsMultipleInstallerTest, MultipleHapsInstall_1000, Function | SmallTest | Level0)
{
    std::vector<std::string> filePaths;
    std::string firstBundleFile = RESOURCE_ROOT_PATH + RIGHT_BUNDLE_SECOND;
    std::string secondBundleFile = RESOURCE_ROOT_PATH + RIGHT_BUNDLE_FOURTH;
    filePaths.emplace_back(firstBundleFile);
    filePaths.emplace_back(secondBundleFile);
    ErrCode installRes = InstallThirdPartyMultipleBundles(filePaths, true);
    EXPECT_EQ(installRes, ERR_APPEXECFWK_INSTALL_VERSIONCODE_NOT_SAME);
    CheckFileNonExist();
}

/**
 * @tc.number: MultipleHapsInstall_1100
 * @tc.name: test to install haps with different version code in the input file path array
 * @tc.desc: 1.the version code is differnet in the input file path array
 *           2.the installation result is fail
 * @tc.require: AR000GJ4KF
 */
HWTEST_F(BmsMultipleInstallerTest, MultipleHapsInstall_1100, Function | SmallTest | Level0)
{
    auto dataMgr = GetBundleDataMgr();
    EXPECT_NE(dataMgr, nullptr);
    std::vector<std::string> filePaths;
    std::string firstBundleFile = RESOURCE_ROOT_PATH + RIGHT_BUNDLE_SECOND;
    std::string secondBundleFile = RESOURCE_ROOT_PATH + RIGHT_BUNDLE_TWELFTH;
    filePaths.emplace_back(firstBundleFile);
    filePaths.emplace_back(secondBundleFile);
    ErrCode installRes = InstallThirdPartyMultipleBundles(filePaths, true);
    EXPECT_EQ(installRes, ERR_OK);
    CheckFileExist();

    CheckMultipleFileExist(PACKAGE_NAME_THIRD);
    CheckMultipleFileExist(PACKAGE_NAME_SECOND);

    BundleInfo info;
    auto result = dataMgr->GetBundleInfo(BUNDLE_NAME, BundleFlag::GET_BUNDLE_DEFAULT, info);
    EXPECT_TRUE(result);
    IsContainModuleInfo(info, PACKAGE_NAME_THIRD, PACKAGE_NAME_SECOND);

    ClearBundleInfo();
}

/**
 * @tc.number: MultipleHapsInstall_1500
 * @tc.name: test to install haps which contain more than one entry hap in the array
 * @tc.desc: 1.there are more than one entry hap in the array
 *           2.the installation is failed
 * @tc.require: AR000GJ4KF
 */
HWTEST_F(BmsMultipleInstallerTest, MultipleHapsInstall_1500, Function | SmallTest | Level0)
{
    std::vector<std::string> filePaths;
    std::string firstBundleFile = RESOURCE_ROOT_PATH + RIGHT_BUNDLE_FIRST;
    std::string secondBundleFile = RESOURCE_ROOT_PATH + RIGHT_BUNDLE_FIFTH;
    filePaths.emplace_back(firstBundleFile);
    filePaths.emplace_back(secondBundleFile);
    ErrCode installRes = InstallThirdPartyMultipleBundles(filePaths, true);
    EXPECT_EQ(installRes, ERR_APPEXECFWK_INSTALL_INVALID_NUMBER_OF_ENTRY_HAP);
}

/**
 * @tc.number: MultipleHapsInstall_1700
 * @tc.name: test to install haps with valid paths in the input file path array
 * @tc.desc: 1.the vaild paths in the input file path array
 *           2.the installation result is successful
 * @tc.require: AR000GJ4KF
 */
HWTEST_F(BmsMultipleInstallerTest, MultipleHapsInstall_1700, Function | SmallTest | Level0)
{
    auto dataMgr = GetBundleDataMgr();
    EXPECT_NE(dataMgr, nullptr);
    std::vector<std::string> filePaths;
    std::string firstBundleFile = RESOURCE_ROOT_PATH + RIGHT_BUNDLE_FIRST;
    std::string secondBundleFile = RESOURCE_ROOT_PATH + RIGHT_BUNDLE_SECOND;
    filePaths.emplace_back(firstBundleFile);
    filePaths.emplace_back(secondBundleFile);
    ErrCode installRes = InstallThirdPartyMultipleBundles(filePaths, true);
    EXPECT_EQ(installRes, ERR_OK);
    CheckFileExist();

    CheckMultipleFileExist(PACKAGE_NAME_FIRST);
    CheckMultipleFileExist(PACKAGE_NAME_SECOND);

    BundleInfo info;
    auto result = dataMgr->GetBundleInfo(BUNDLE_NAME, BundleFlag::GET_BUNDLE_DEFAULT, info);
    EXPECT_TRUE(result);
    IsContainModuleInfo(info, PACKAGE_NAME_FIRST, PACKAGE_NAME_SECOND);

    ClearBundleInfo();
}

/**
 * @tc.number: MultipleHapsInstall_1500
 * @tc.name: test to install haps which are under a same direction putted in the array
 * @tc.desc: 1.the haps are placed under a same direction and put the direction in the array
 *           2.the hap files under this direction are valid
 *           3.the installation result is successful
 * @tc.require: AR000GJ4KF
 */
HWTEST_F(BmsMultipleInstallerTest, MultipleHapsInstall_2000, Function | SmallTest | Level0)
{
    auto dataMgr = GetBundleDataMgr();
    EXPECT_NE(dataMgr, nullptr);

    std::vector<std::string> filePaths;
    filePaths.emplace_back(RESOURCE_TEST_PATH);
    ErrCode installRes = InstallThirdPartyMultipleBundles(filePaths, true);
    EXPECT_EQ(installRes, ERR_OK);
    CheckFileExist();

    CheckMultipleFileExist(PACKAGE_NAME_FIRST);
    CheckMultipleFileExist(PACKAGE_NAME_SECOND);

    BundleInfo info;
    auto result = dataMgr->GetBundleInfo(BUNDLE_NAME, BundleFlag::GET_BUNDLE_DEFAULT, info);
    EXPECT_TRUE(result);
    IsContainModuleInfo(info, PACKAGE_NAME_FIRST, PACKAGE_NAME_SECOND);

    ClearBundleInfo();
}

/**
 * @tc.number: MultipleHapsInstall_2600
 * @tc.name: test to install haps with some of them having no sign info
 * @tc.desc: 1.sixth_rigth.hap has no sign info in the haps
 *           2.the installation result is failed
 * @tc.require: AR000GJ4KF
 */
HWTEST_F(BmsMultipleInstallerTest, MultipleHapsInstall_2600, Function | SmallTest | Level0)
{
    auto dataMgr = GetBundleDataMgr();
    EXPECT_NE(dataMgr, nullptr);
    std::vector<std::string> filePaths;
    std::string firstBundleFile = RESOURCE_ROOT_PATH + RIGHT_BUNDLE_FIRST;
    std::string secondBundleFile = RESOURCE_ROOT_PATH + RIGHT_BUNDLE_SIXTH;
    filePaths.emplace_back(firstBundleFile);
    filePaths.emplace_back(secondBundleFile);
    ErrCode installRes = InstallThirdPartyMultipleBundles(filePaths, true);
    EXPECT_EQ(installRes, ERR_APPEXECFWK_INSTALL_NO_SIGNATURE_INFO);
    CheckFileNonExist();

    ApplicationInfo info;
    auto result = dataMgr->GetApplicationInfo(BUNDLE_NAME, ApplicationFlag::GET_BASIC_APPLICATION_INFO, USERID, info);
    EXPECT_FALSE(result);
}

/**
 * @tc.number: MultipleHapsInstall_2700
 * @tc.name: test to install haps with different sign info in the input file path array
 * @tc.desc: 1.the sign info is differnet in the input file path array
 *           2.the installation result is fail
 * @tc.require: AR000GJ4KF
 */
HWTEST_F(BmsMultipleInstallerTest, MultipleHapsInstall_2700, Function | SmallTest | Level0)
{
    std::vector<std::string> filePaths;
    std::string firstBundleFile = RESOURCE_ROOT_PATH + RIGHT_BUNDLE;
    std::string secondBundleFile = RESOURCE_ROOT_PATH + RIGHT_BUNDLE_SEVENTH;
    filePaths.emplace_back(firstBundleFile);
    filePaths.emplace_back(secondBundleFile);
    ErrCode installRes = InstallThirdPartyMultipleBundles(filePaths, true);
    EXPECT_EQ(installRes, ERR_APPEXECFWK_INSTALL_SIGN_INFO_INCONSISTENT);
    CheckFileNonExist();
}

/**
 * @tc.number: MultipleHapsUpdateData_0100
 * @tc.name: test the hap files can be installed and update their info to bms
 * @tc.desc: 1.the ThirdParty bundles are available
 *           2.the haps can be installed and update their info to bms
 * @tc.require: AR000GJ4KF
 */
HWTEST_F(BmsMultipleInstallerTest, MultipleHapsUpdateData_0100, Function | SmallTest | Level0)
{
    ApplicationInfo info;
    auto dataMgr = GetBundleDataMgr();
    EXPECT_NE(dataMgr, nullptr);
    bool result = dataMgr->GetApplicationInfo(BUNDLE_NAME, ApplicationFlag::GET_BASIC_APPLICATION_INFO, USERID, info);
    EXPECT_FALSE(result);

    std::vector<std::string> filePaths;
    std::string firstBundleFile = RESOURCE_ROOT_PATH + RIGHT_BUNDLE_FIRST;
    std::string secondBundleFile = RESOURCE_ROOT_PATH + RIGHT_BUNDLE_SECOND;
    filePaths.emplace_back(firstBundleFile);
    filePaths.emplace_back(secondBundleFile);

    ErrCode installRes = InstallThirdPartyMultipleBundles(filePaths, true);
    EXPECT_EQ(installRes, ERR_OK);

    BundleInfo bundleInfo;
    result = dataMgr->GetBundleInfo(BUNDLE_NAME, BundleFlag::GET_BUNDLE_DEFAULT, bundleInfo);
    EXPECT_TRUE(result);

    IsContainModuleInfo(bundleInfo, PACKAGE_NAME_FIRST, PACKAGE_NAME_SECOND);

    ClearBundleInfo();
}

/**
 * @tc.number: MultipleHapsUpdateData_0200
 * @tc.name: test the input array containing wrong bundle file which can't be installed and
 *           their info will not updated to bms
 * @tc.desc: 1.the ThirdParty bundle is wrong
 *           2.the wrong bundle can't be installed and its info will not updated to bms
 * @tc.require: AR000GJ4KF
 */
HWTEST_F(BmsMultipleInstallerTest, MultipleHapsUpdateData_0200, Function | SmallTest | Level0)
{
    ApplicationInfo info;
    auto dataMgr = GetBundleDataMgr();
    EXPECT_NE(dataMgr, nullptr);
    bool result = dataMgr->GetApplicationInfo(BUNDLE_NAME, ApplicationFlag::GET_BASIC_APPLICATION_INFO, USERID, info);
    EXPECT_FALSE(result);

    std::vector<std::string> filePaths;
    std::string firstBundleFile = RESOURCE_ROOT_PATH + RIGHT_BUNDLE_FIRST;
    std::string wrongBundleName = RESOURCE_ROOT_PATH + WRONG_BUNDLE_NAME;
    filePaths.emplace_back(firstBundleFile);
    filePaths.emplace_back(wrongBundleName);

    ErrCode installRes = InstallThirdPartyMultipleBundles(filePaths, false);
    EXPECT_EQ(installRes, ERR_APPEXECFWK_INSTALL_INVALID_HAP_NAME);

    result = dataMgr->GetApplicationInfo(BUNDLE_NAME, ApplicationFlag::GET_BASIC_APPLICATION_INFO, USERID, info);
    EXPECT_FALSE(result);
}

/**
 * @tc.number: MultipleHapsUpdateData_0300
 * @tc.name: test the already installed bundle can't be reinstalled and update its info to bms
 * @tc.desc: 1.the bundle is already installed
 *           2.the already installed  bundle can't be reinstalled and update its info to bms
 * @tc.require: AR000GJ4KF
 */
HWTEST_F(BmsMultipleInstallerTest, MultipleHapsUpdateData_0300, Function | SmallTest | Level0)
{
    auto dataMgr = GetBundleDataMgr();
    EXPECT_NE(dataMgr, nullptr);

    ApplicationInfo info;
    bool result = dataMgr->GetApplicationInfo(BUNDLE_NAME, ApplicationFlag::GET_BASIC_APPLICATION_INFO, USERID, info);
    EXPECT_FALSE(result);

    // prepare already install information.
    std::vector<std::string> filePaths;
    std::string firstBundleFile = RESOURCE_ROOT_PATH + RIGHT_BUNDLE_FIRST;
    std::string secondBundleFile = RESOURCE_ROOT_PATH + RIGHT_BUNDLE_SECOND;
    filePaths.emplace_back(firstBundleFile);
    filePaths.emplace_back(secondBundleFile);

    ErrCode installRes = InstallThirdPartyMultipleBundles(filePaths, true);
    EXPECT_EQ(installRes, ERR_OK);

    BundleInfo bundleInfo;
    result = dataMgr->GetBundleInfo(BUNDLE_NAME, BundleFlag::GET_BUNDLE_DEFAULT, bundleInfo);
    EXPECT_TRUE(result);

    IsContainModuleInfo(bundleInfo, PACKAGE_NAME_FIRST, PACKAGE_NAME_SECOND);

    installRes = InstallThirdPartyMultipleBundles(filePaths, true);
    EXPECT_EQ(installRes, ERR_APPEXECFWK_INSTALL_ALREADY_EXIST);

    ClearBundleInfo();
}

/**
 * @tc.number: MultipleHapsUpdateData_0400
 * @tc.name: test the already installed bundle can be reinstalled and update its info to bms
 * @tc.desc: 1.the bundle is already installed
 *           2.the already installed  bundle can be reinstalled and update its info to bms
 * @tc.require: AR000GJ4KF
 */
HWTEST_F(BmsMultipleInstallerTest, MultipleHapsUpdateData_0400, Function | SmallTest | Level0)
{
    auto dataMgr = GetBundleDataMgr();
    EXPECT_NE(dataMgr, nullptr);

    ApplicationInfo info;
    bool result = dataMgr->GetApplicationInfo(BUNDLE_NAME, ApplicationFlag::GET_BASIC_APPLICATION_INFO, USERID, info);
    EXPECT_FALSE(result);

    // prepare already install information.
    std::vector<std::string> filePaths;
    std::string firstBundleFile = RESOURCE_ROOT_PATH + RIGHT_BUNDLE_FIRST;
    std::string secondBundleFile = RESOURCE_ROOT_PATH + RIGHT_BUNDLE_SECOND;
    filePaths.emplace_back(firstBundleFile);
    filePaths.emplace_back(secondBundleFile);

    ErrCode installRes = InstallThirdPartyMultipleBundles(filePaths, true);
    EXPECT_EQ(installRes, ERR_OK);

    BundleInfo bundleInfo;
    result = dataMgr->GetBundleInfo(BUNDLE_NAME, BundleFlag::GET_BUNDLE_DEFAULT, bundleInfo);
    EXPECT_TRUE(result);

    IsContainModuleInfo(bundleInfo, PACKAGE_NAME_FIRST, PACKAGE_NAME_SECOND);

    installRes = InstallThirdPartyMultipleBundles(filePaths, false);
    EXPECT_EQ(installRes, ERR_OK);

    ClearBundleInfo();
}

/**
 * @tc.number: MultipleHapsUpdateData_0500
 * @tc.name: test the installation is failed when install status is incorrect
 * @tc.desc: 1.install status is incorrect
 *           2.the haps are installed failed and their info will not update to bms
 * @tc.require: AR000GJ4KF
 */
HWTEST_F(BmsMultipleInstallerTest, MultipleHapsUpdateData_0500, Function | SmallTest | Level0)
{
    // prepare already install information.
    auto dataMgr = GetBundleDataMgr();
    EXPECT_NE(dataMgr, nullptr);
    dataMgr->UpdateBundleInstallState(BUNDLE_NAME, InstallState::INSTALL_START);
    // begin to  reinstall package
    std::vector<std::string> filePaths;
    std::string firstBundleFile = RESOURCE_ROOT_PATH + RIGHT_BUNDLE_FIRST;
    std::string secondBundleFile = RESOURCE_ROOT_PATH + RIGHT_BUNDLE_SECOND;
    filePaths.emplace_back(firstBundleFile);
    filePaths.emplace_back(secondBundleFile);

    ErrCode installResult = InstallThirdPartyMultipleBundles(filePaths, true);
    EXPECT_EQ(installResult, ERR_APPEXECFWK_INSTALL_STATE_ERROR);
    // reset the install state

    ApplicationInfo info;
    bool result = dataMgr->GetApplicationInfo(BUNDLE_NAME, ApplicationFlag::GET_BASIC_APPLICATION_INFO, USERID, info);
    EXPECT_FALSE(result);

    dataMgr->UpdateBundleInstallState(BUNDLE_NAME, InstallState::INSTALL_FAIL);
}