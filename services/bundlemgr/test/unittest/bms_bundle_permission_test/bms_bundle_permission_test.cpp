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
#include <gtest/gtest.h>
#include <string>
#include <sstream>

#include "bundle_info.h"
#include "bundle_installer_host.h"
#include "bundle_mgr_service.h"
#include "bundle_permission_mgr.h"
#include "bundle_verify_mgr.h"
#include "inner_bundle_info.h"
#include "permission_def.h"
#include "permission/permission_kit.h"

using namespace testing::ext;
using namespace std::chrono_literals;
using namespace OHOS;
using namespace OHOS::AppExecFwk;
using namespace OHOS::Security;

namespace {
const std::string BUNDLE_LABEL = "Hello, OHOS";
const std::string PROCESS_TEST = "test.process";
const std::string BUNDLE_DESCRIPTION = "example helloworld";
const std::string BUNDLE_VENDOR = "example";
const std::string BUNDLE_VERSION_NAME = "1.0.0.1";
const int32_t BUNDLE_MAX_SDK_VERSION = 0;
const int32_t BUNDLE_MIN_SDK_VERSION = 0;
const uint32_t BUNDLE_VERSION_CODE = 1001;
const std::string PACKAGE_NAME = "com.example.bundlekit.test.entry";
const std::string DEVICE_ID = "PHONE-001";
const std::string LABEL = "hello";
const std::string DESCRIPTION = "mainEntry";
const std::string ICON_PATH = "/data/data/icon.png";
const std::string ACTION = "action.system.home";
const std::string ENTITY = "entity.system.home";
const std::string CODE_PATH = "/data/accounts/account_0/com.example.bundlekit.test/code";
const std::string MAIN_ENTRY = "com.example.bundlekit.test.entry";
const std::string URI = "dataability://com.example.hiworld.himusic.UserADataAbility";

const std::string HAP_FILE_PATH = "/data/test/resource/bms/bundle_permission/sign.hap";
const std::string HAP_FILE_PATH1 = "/data/test/resource/bms/bundle_permission/unsign.hap";
const std::string ERROR_HAP_FILE_PATH = "/data/test/resource/bms/bundle_kit/error.hap";
const std::string META_DATA = "name";
const std::string FILES_DIR = "/data/accounts/account_0/appdata/com.example.bundlekit.test/files";
const std::string DATA_BASE_DIR = "/data/accounts/account_0/appdata/com.example.bundlekit.test/database";
const std::string CACHE_DIR = "/data/accounts/account_0/appdata/com.example.bundlekit.test/cache";
const std::string MOCK_DEFPERMISSION_NAME = "com.bundledefpermission.test";
const std::string MOCK_DEFPERMISSION_NAME_1 = "com.bundledefpermission1.test";
const std::string MOCK_DEFPERMISSION_NAME_2 = "com.bundledefpermission2.test";
const std::string MOCK_DEFPERMISSION_NAME_3 = "com.bundledefpermission3.test";
const std::string MOCK_DEFPERMISSION_NAME_4 = "com.bundledefpermission4.test";
const std::string MOCK_DEFPERMISSION_NAME_5 = "com.bundledefpermission5.test";
const std::string MOCK_DEFPERMISSION_NAME_6 = "com.bundledefpermission6.test";
const std::string MOCK_DEFPERMISSION_NAME_7 = "com.bundledefpermission7.test";
const std::string MOCK_REQPERMISSION_NAME = "com.bundlereqpermission.test";
const std::string MOCK_REQPERMISSION_NAME_1 = "com.bundlereqpermission1.test";
const std::string DEFPERMISSION_NAME1 = "com.myability.permission.MYPERMISSION1";
const std::string DEFPERMISSION_NAME2 = "com.myability.permission.MYPERMISSION2";
const std::string DEFPERMISSION_NAME3 = "com.myability.permission.MYPERMISSION3";
const std::string DEFPERMISSION_NAME_EMAIL = "ohos.permission.email";
const std::string DEFPERMISSION_NAME_APP = "ohos.permission.app";
const std::string DEFPERMISSION_NAME_WECHAT = "ohos.permission.wechat";
const std::string DEFPERMISSION_NAME_MUSIC = "ohos.permission.music";
const std::string DEFPERMISSION_NAME4 = "com.myability.permission.MYPERMISSION4";
const std::string DEFPERMISSION_NAME5 = "com.myability.permission.MYPERMISSION5";
const std::string DEFPERMISSION_NAME6 = "com.myability.permission.MYPERMISSION6";
const std::string DEFPERMISSION_NAME10 = "com.myability.permission.MYPERMISSION10";
const std::string DEFPERMISSION_NAME11 = "com.myability.permission.MYPERMISSION11";
const std::string DEFPERMISSION_NAME12 = "com.myability.permission.MYPERMISSION12";
const std::string DEFPERMISSION_NAME13 = "com.myability.permission.MYPERMISSION13";
const std::string DEFPERMISSION_NAME14 = "com.myability.permission.MYPERMISSION14";
const std::string DEFPERMISSION_NAME15 = "com.myability.permission.MYPERMISSION15";
const std::string DEFPERMISSION_NAME16 = "com.myability.permission.MYPERMISSION16";
const std::string DEFPERMISSION_NAME17 = "com.myability.permission.MYPERMISSION17";
const std::string DEFPERMISSION_NAME18 = "com.myability.permission.MYPERMISSION18";
const std::string DEF_LABEL1 = "MockDefPermissionBundleSystemGrant";
const std::string DEF_LABEL2 = "MockDefPermissionBundleUserGrant";
const std::string DEF_LABEL3 = "MockSameDefPermissionNameBundleSystemGrant";
const DefinePermission DEFPERMISSION_SYSTEM1 = {
    .name = DEFPERMISSION_NAME1,
    .grantMode = "system_grant",
    .availableScope = {"signature"},
    .label = DEF_LABEL1,
    .labelId = 1,
    .description = DEF_LABEL1,
    .descriptionId = 1
};
const DefinePermission DEFPERMISSION_SYSTEM2 = {
    .name = DEFPERMISSION_NAME2,
    .grantMode = "system_grant",
    .availableScope = {"signature"},
    .label = DEF_LABEL1,
    .labelId = 1,
    .description = DEF_LABEL1,
    .descriptionId = 1
};
const DefinePermission DEFPERMISSION_SYSTEM3 = {
    .name = DEFPERMISSION_NAME3,
    .grantMode = "system_grant",
    .availableScope = {"signature"},
    .label = DEF_LABEL1,
    .labelId = 1,
    .description = DEF_LABEL1,
    .descriptionId = 1
};
const DefinePermission DEFPERMISSION_UPDATE1 = {
    .name = DEFPERMISSION_NAME1,
    .grantMode = "user_grant",
    .availableScope = {"signature"},
    .label = DEF_LABEL2,
    .labelId = 10,
    .description = DEF_LABEL2,
    .descriptionId = 10
};
const DefinePermission DEFPERMISSION_UPDATE2 = {
    .name = DEFPERMISSION_NAME2,
    .grantMode = "user_grant",
    .availableScope = {"signature"},
    .label = DEF_LABEL2,
    .labelId = 10,
    .description = DEF_LABEL2,
    .descriptionId = 10
};
const DefinePermission DEFPERMISSION_UPDATE3 = {
    .name = DEFPERMISSION_NAME3,
    .grantMode = "user_grant",
    .availableScope = {"signature"},
    .label = DEF_LABEL2,
    .labelId = 10,
    .description = DEF_LABEL2,
    .descriptionId = 10
};
const DefinePermission DEFPERMISSION_EMAIL = {
    .name = DEFPERMISSION_NAME_EMAIL,
    .grantMode = "system_grant",
    .availableScope = {""},
    .label = DEF_LABEL1,
    .labelId = 1,
    .description = DEF_LABEL1,
    .descriptionId = 1
};
const DefinePermission DEFPERMISSION_MUSIC = {
    .name = DEFPERMISSION_NAME_MUSIC,
    .grantMode = "system_grant",
    .availableScope = {""},
    .label = DEF_LABEL1,
    .labelId = 1,
    .description = DEF_LABEL1,
    .descriptionId = 1
};
const DefinePermission DEFPERMISSION_APP = {
    .name = DEFPERMISSION_NAME_APP,
    .grantMode = "system_grant",
    .availableScope = {""},
    .label = DEF_LABEL1,
    .labelId = 1,
    .description = DEF_LABEL1,
    .descriptionId = 1
};
const DefinePermission DEFPERMISSION_WECHAT = {
    .name = DEFPERMISSION_NAME_WECHAT,
    .grantMode = "system_grant",
    .availableScope = {""},
    .label = DEF_LABEL1,
    .labelId = 1,
    .description = DEF_LABEL1,
    .descriptionId = 1
};
const DefinePermission DEFPERMISSION_NOAVAILIAVLE1 = {
    .name = DEFPERMISSION_NAME4,
    .grantMode = "system_grant",
    .availableScope = {""},
    .label = DEF_LABEL1,
    .labelId = 1,
    .description = DEF_LABEL1,
    .descriptionId = 1
};
const DefinePermission DEFPERMISSION_NOAVAILIAVLE2 = {
    .name = DEFPERMISSION_NAME5,
    .grantMode = "system_grant",
    .availableScope = {""},
    .label = DEF_LABEL1,
    .labelId = 1,
    .description = DEF_LABEL1,
    .descriptionId = 1
};
const DefinePermission DEFPERMISSION_NOAVAILIAVLE3 = {
    .name = DEFPERMISSION_NAME6,
    .grantMode = "system_grant",
    .availableScope = {""},
    .label = DEF_LABEL1,
    .labelId = 1,
    .description = DEF_LABEL1,
    .descriptionId = 1
};
const DefinePermission DEFPERMISSION_USERGRANT1 = {
    .name = DEFPERMISSION_NAME10,
    .grantMode = "user_grant",
    .availableScope = {"signature"},
    .label = DEF_LABEL2,
    .labelId = 1,
    .description = DEF_LABEL1,
    .descriptionId = 1
};
const DefinePermission DEFPERMISSION_USERGRANT2 = {
    .name = DEFPERMISSION_NAME11,
    .grantMode = "user_grant",
    .availableScope = {"signature"},
    .label = DEF_LABEL2,
    .labelId = 1,
    .description = DEF_LABEL1,
    .descriptionId = 1
};
const DefinePermission DEFPERMISSION_USERGRANT3 = {
    .name = DEFPERMISSION_NAME12,
    .grantMode = "user_grant",
    .availableScope = {"signature"},
    .label = DEF_LABEL2,
    .labelId = 1,
    .description = DEF_LABEL1,
    .descriptionId = 1
};
const DefinePermission DEFPERMISSION_USERGRANT4 = {
    .name = DEFPERMISSION_NAME13,
    .grantMode = "user_grant",
    .availableScope = {"signature"},
    .label = DEF_LABEL2,
    .labelId = 1,
    .description = DEF_LABEL1,
    .descriptionId = 1
};
const DefinePermission DEFPERMISSION_USERGRANT5 = {
    .name = DEFPERMISSION_NAME14,
    .grantMode = "user_grant",
    .availableScope = {"signature"},
    .label = DEF_LABEL2,
    .labelId = 1,
    .description = DEF_LABEL1,
    .descriptionId = 1
};
const DefinePermission DEFPERMISSION_USERGRANT6 = {
    .name = DEFPERMISSION_NAME15,
    .grantMode = "user_grant",
    .availableScope = {"signature"},
    .label = DEF_LABEL2,
    .labelId = 1,
    .description = DEF_LABEL1,
    .descriptionId = 1
};
const DefinePermission DEFPERMISSION_SAMENAME1 = {
    .name = DEFPERMISSION_NAME10,
    .grantMode = "system_grant",
    .availableScope = {"signature"},
    .label = DEF_LABEL3,
    .labelId = 1,
    .description = DEF_LABEL1,
    .descriptionId = 1
};
const DefinePermission DEFPERMISSION_SAMENAME2 = {
    .name = DEFPERMISSION_NAME11,
    .grantMode = "system_grant",
    .availableScope = {"signature"},
    .label = DEF_LABEL3,
    .labelId = 1,
    .description = DEF_LABEL1,
    .descriptionId = 1
};
const DefinePermission DEFPERMISSION_SAME2 = {
    .name = DEFPERMISSION_NAME17,
    .grantMode = "system_grant",
    .availableScope = {"signature"},
    .label = DEF_LABEL3,
    .labelId = 1,
    .description = DEF_LABEL1,
    .descriptionId = 1
};
const DefinePermission DEFPERMISSION_SAME1 = {
    .name = DEFPERMISSION_NAME16,
    .grantMode = "system_grant",
    .availableScope = {"signature"},
    .label = DEF_LABEL3,
    .labelId = 1,
    .description = DEF_LABEL1,
    .descriptionId = 1
};
const DefinePermission DEFPERMISSION_SAME3 = {
    .name = DEFPERMISSION_NAME18,
    .grantMode = "system_grant",
    .availableScope = {"signature"},
    .label = DEF_LABEL3,
    .labelId = 1,
    .description = DEF_LABEL1,
    .descriptionId = 1
};
}  // namespace
class BmsBundlePermissionTest : public testing::Test {
public:
    BmsBundlePermissionTest();
    ~BmsBundlePermissionTest();
    static void SetUpTestCase();
    static void TearDownTestCase();
    void SetUp();
    void TearDown();
    void MockDefPermissionBundleSystemGrant(InnerBundleInfo &innerbundleinfo);
    void MockDefPermissionBundleTestReqpermission(InnerBundleInfo &innerbundleinfo);
    void MockNoDefPermissionBundle(InnerBundleInfo &innerbundleinfo);
    void MockNoAvailableScopeDefPermissionBundle(InnerBundleInfo &innerbundleinfo);
    void MockDefPermissionBundleUserGrant(InnerBundleInfo &innerbundleinfo);
    void MockSameDefPermissionNameBundleSystemGrant(InnerBundleInfo &innerbundleinfo);
    void MockDefPermissionBundleSame(InnerBundleInfo &innerbundleinfo);
    void MockReqPermissionBundle(InnerBundleInfo &innerbundleinfo);
    void MockOtherReqPermissionBundle(InnerBundleInfo &innerbundleinfo);
    void MockUpdateDefPermissionBundleSystemGrant(InnerBundleInfo &innerbundleinfo);
    void CheckPermissionDef(const DefinePermission &defPermission, std::string bundlename,
        PermissionDef &permissionDef);
    void CheckErrPermissionDef(
        const DefinePermission &defPermission, std::string bundlename, PermissionDef &permissionDef);
    bool ConvertPermissionDef(
        Permission::PermissionDef &permDef, const DefinePermission &defPermission, const std::string &bundleName);
};

BmsBundlePermissionTest::BmsBundlePermissionTest()
{}

BmsBundlePermissionTest::~BmsBundlePermissionTest()
{}

void BmsBundlePermissionTest::SetUpTestCase()
{}

void BmsBundlePermissionTest::TearDownTestCase()
{}

void BmsBundlePermissionTest::SetUp()
{}

void BmsBundlePermissionTest::TearDown()
{}

void BmsBundlePermissionTest::MockDefPermissionBundleSystemGrant(InnerBundleInfo &innerbundleinfo)
{
    ApplicationInfo appInfo;
    appInfo.bundleName = MOCK_DEFPERMISSION_NAME;
    appInfo.name = MOCK_DEFPERMISSION_NAME;
    appInfo.deviceId = DEVICE_ID;
    appInfo.process = PROCESS_TEST;
    appInfo.label = BUNDLE_LABEL;
    appInfo.description = BUNDLE_DESCRIPTION;
    appInfo.codePath = CODE_PATH;
    appInfo.dataDir = FILES_DIR;
    appInfo.dataBaseDir = DATA_BASE_DIR;
    appInfo.cacheDir = CACHE_DIR;

    BundleInfo bundleInfo;
    bundleInfo.name = MOCK_DEFPERMISSION_NAME;
    bundleInfo.label = BUNDLE_LABEL;
    bundleInfo.description = BUNDLE_DESCRIPTION;
    bundleInfo.vendor = BUNDLE_VENDOR;
    bundleInfo.versionCode = BUNDLE_VERSION_CODE;
    bundleInfo.versionName = BUNDLE_VERSION_NAME;
    bundleInfo.minSdkVersion = BUNDLE_MIN_SDK_VERSION;
    bundleInfo.maxSdkVersion = BUNDLE_MAX_SDK_VERSION;
    bundleInfo.mainEntry = MAIN_ENTRY;
    bundleInfo.isKeepAlive = true;

    InnerModuleInfo moduleInfo;
    std::map<std::string, InnerModuleInfo> innerModuleInfos_;
    moduleInfo.defPermissions = {DEFPERMISSION_SYSTEM1, DEFPERMISSION_SYSTEM2, DEFPERMISSION_SYSTEM3};
    moduleInfo.modulePackage = PACKAGE_NAME;
    moduleInfo.moduleName = PACKAGE_NAME;
    moduleInfo.description = BUNDLE_DESCRIPTION;

    AppExecFwk::CustomizeData customizeData {
        "name",
        "value",
        "extra"
    };
    MetaData metaData {
        {customizeData}
    };
    moduleInfo.metaData = metaData;
    innerbundleinfo.SetCurrentModulePackage(PACKAGE_NAME);
    innerbundleinfo.SetBaseApplicationInfo(appInfo);
    innerbundleinfo.SetBaseBundleInfo(bundleInfo);
    innerbundleinfo.InsertInnerModuleInfo(PACKAGE_NAME, moduleInfo);
}

void BmsBundlePermissionTest::MockUpdateDefPermissionBundleSystemGrant(InnerBundleInfo &innerbundleinfo)
{
    ApplicationInfo appInfo;
    appInfo.bundleName = MOCK_DEFPERMISSION_NAME;
    appInfo.name = MOCK_DEFPERMISSION_NAME;
    appInfo.deviceId = DEVICE_ID;
    appInfo.process = PROCESS_TEST;
    appInfo.label = BUNDLE_LABEL;
    appInfo.description = BUNDLE_DESCRIPTION;
    appInfo.codePath = CODE_PATH;
    appInfo.dataDir = FILES_DIR;
    appInfo.dataBaseDir = DATA_BASE_DIR;
    appInfo.cacheDir = CACHE_DIR;

    BundleInfo bundleInfo;
    bundleInfo.name = MOCK_DEFPERMISSION_NAME;
    bundleInfo.label = BUNDLE_LABEL;
    bundleInfo.description = BUNDLE_DESCRIPTION;
    bundleInfo.vendor = BUNDLE_VENDOR;
    bundleInfo.versionCode = BUNDLE_VERSION_CODE;
    bundleInfo.versionName = BUNDLE_VERSION_NAME;
    bundleInfo.minSdkVersion = BUNDLE_MIN_SDK_VERSION;
    bundleInfo.maxSdkVersion = BUNDLE_MAX_SDK_VERSION;
    bundleInfo.mainEntry = MAIN_ENTRY;
    bundleInfo.isKeepAlive = true;

    InnerModuleInfo moduleInfo;
    std::map<std::string, InnerModuleInfo> innerModuleInfos_;
    moduleInfo.defPermissions = {DEFPERMISSION_UPDATE1, DEFPERMISSION_UPDATE2, DEFPERMISSION_UPDATE3};
    moduleInfo.modulePackage = PACKAGE_NAME;
    moduleInfo.moduleName = PACKAGE_NAME;
    moduleInfo.description = BUNDLE_DESCRIPTION;

    AppExecFwk::CustomizeData customizeData {
        "name",
        "value",
        "extra"
    };
    MetaData metaData {
        {customizeData}
    };
    moduleInfo.metaData = metaData;
    innerbundleinfo.SetCurrentModulePackage(PACKAGE_NAME);
    innerbundleinfo.SetBaseApplicationInfo(appInfo);
    innerbundleinfo.SetBaseBundleInfo(bundleInfo);
    innerbundleinfo.InsertInnerModuleInfo(PACKAGE_NAME, moduleInfo);
}

void BmsBundlePermissionTest::MockDefPermissionBundleTestReqpermission(InnerBundleInfo &innerbundleinfo)
{
    ApplicationInfo appInfo;
    appInfo.bundleName = MOCK_DEFPERMISSION_NAME_1;
    appInfo.name = MOCK_DEFPERMISSION_NAME_1;
    appInfo.deviceId = DEVICE_ID;
    appInfo.process = PROCESS_TEST;
    appInfo.label = BUNDLE_LABEL;
    appInfo.description = BUNDLE_DESCRIPTION;
    appInfo.codePath = CODE_PATH;
    appInfo.dataDir = FILES_DIR;
    appInfo.dataBaseDir = DATA_BASE_DIR;
    appInfo.cacheDir = CACHE_DIR;

    BundleInfo bundleInfo;
    bundleInfo.name = MOCK_DEFPERMISSION_NAME_1;
    bundleInfo.label = BUNDLE_LABEL;
    bundleInfo.description = BUNDLE_DESCRIPTION;
    bundleInfo.vendor = BUNDLE_VENDOR;
    bundleInfo.versionCode = BUNDLE_VERSION_CODE;
    bundleInfo.versionName = BUNDLE_VERSION_NAME;
    bundleInfo.minSdkVersion = BUNDLE_MIN_SDK_VERSION;
    bundleInfo.maxSdkVersion = BUNDLE_MAX_SDK_VERSION;
    bundleInfo.mainEntry = MAIN_ENTRY;
    bundleInfo.isKeepAlive = true;

    InnerModuleInfo moduleInfo;
    std::map<std::string, InnerModuleInfo> innerModuleInfos_;
    moduleInfo.defPermissions = {DEFPERMISSION_EMAIL, DEFPERMISSION_MUSIC, DEFPERMISSION_APP, DEFPERMISSION_WECHAT};
    moduleInfo.modulePackage = PACKAGE_NAME;
    moduleInfo.moduleName = PACKAGE_NAME;
    moduleInfo.description = BUNDLE_DESCRIPTION;

    AppExecFwk::CustomizeData customizeData {
        "name",
        "value",
        "extra"
    };
    MetaData metaData {
        {customizeData}
    };
    moduleInfo.metaData = metaData;
    innerbundleinfo.SetCurrentModulePackage(PACKAGE_NAME);
    innerbundleinfo.SetBaseApplicationInfo(appInfo);
    innerbundleinfo.SetBaseBundleInfo(bundleInfo);
    innerbundleinfo.InsertInnerModuleInfo(PACKAGE_NAME, moduleInfo);
}

void BmsBundlePermissionTest::MockNoDefPermissionBundle(InnerBundleInfo &innerbundleinfo)
{
    ApplicationInfo appInfo;
    appInfo.bundleName = MOCK_DEFPERMISSION_NAME_2;
    appInfo.name = MOCK_DEFPERMISSION_NAME_2;
    appInfo.deviceId = DEVICE_ID;
    appInfo.process = PROCESS_TEST;
    appInfo.label = BUNDLE_LABEL;
    appInfo.description = BUNDLE_DESCRIPTION;
    appInfo.codePath = CODE_PATH;
    appInfo.dataDir = FILES_DIR;
    appInfo.dataBaseDir = DATA_BASE_DIR;
    appInfo.cacheDir = CACHE_DIR;

    BundleInfo bundleInfo;
    bundleInfo.name = MOCK_DEFPERMISSION_NAME_2;
    bundleInfo.label = BUNDLE_LABEL;
    bundleInfo.description = BUNDLE_DESCRIPTION;
    bundleInfo.vendor = BUNDLE_VENDOR;
    bundleInfo.versionCode = BUNDLE_VERSION_CODE;
    bundleInfo.versionName = BUNDLE_VERSION_NAME;
    bundleInfo.minSdkVersion = BUNDLE_MIN_SDK_VERSION;
    bundleInfo.maxSdkVersion = BUNDLE_MAX_SDK_VERSION;
    bundleInfo.mainEntry = MAIN_ENTRY;
    bundleInfo.isKeepAlive = true;

    InnerModuleInfo moduleInfo;
    std::map<std::string, InnerModuleInfo> innerModuleInfos_;
    moduleInfo.modulePackage = PACKAGE_NAME;
    moduleInfo.moduleName = PACKAGE_NAME;
    moduleInfo.description = BUNDLE_DESCRIPTION;

    AppExecFwk::CustomizeData customizeData {
        "name",
        "value",
        "extra"
    };
    MetaData metaData {
        {customizeData}
    };
    moduleInfo.metaData = metaData;
    innerbundleinfo.SetCurrentModulePackage(PACKAGE_NAME);
    innerbundleinfo.SetBaseApplicationInfo(appInfo);
    innerbundleinfo.SetBaseBundleInfo(bundleInfo);
    innerbundleinfo.InsertInnerModuleInfo(PACKAGE_NAME, moduleInfo);
}

void BmsBundlePermissionTest::MockNoAvailableScopeDefPermissionBundle(InnerBundleInfo &innerbundleinfo)
{
    ApplicationInfo appInfo;
    appInfo.bundleName = MOCK_DEFPERMISSION_NAME_3;
    appInfo.name = MOCK_DEFPERMISSION_NAME_3;
    appInfo.deviceId = DEVICE_ID;
    appInfo.process = PROCESS_TEST;
    appInfo.label = BUNDLE_LABEL;
    appInfo.description = BUNDLE_DESCRIPTION;
    appInfo.codePath = CODE_PATH;
    appInfo.dataDir = FILES_DIR;
    appInfo.dataBaseDir = DATA_BASE_DIR;
    appInfo.cacheDir = CACHE_DIR;

    BundleInfo bundleInfo;
    bundleInfo.name = MOCK_DEFPERMISSION_NAME_3;
    bundleInfo.label = BUNDLE_LABEL;
    bundleInfo.description = BUNDLE_DESCRIPTION;
    bundleInfo.vendor = BUNDLE_VENDOR;
    bundleInfo.versionCode = BUNDLE_VERSION_CODE;
    bundleInfo.versionName = BUNDLE_VERSION_NAME;
    bundleInfo.minSdkVersion = BUNDLE_MIN_SDK_VERSION;
    bundleInfo.maxSdkVersion = BUNDLE_MAX_SDK_VERSION;
    bundleInfo.mainEntry = MAIN_ENTRY;
    bundleInfo.isKeepAlive = true;

    InnerModuleInfo moduleInfo;
    std::map<std::string, InnerModuleInfo> innerModuleInfos_;
    moduleInfo.defPermissions = {DEFPERMISSION_NOAVAILIAVLE1, DEFPERMISSION_NOAVAILIAVLE2,
        DEFPERMISSION_NOAVAILIAVLE3};
    moduleInfo.modulePackage = PACKAGE_NAME;
    moduleInfo.moduleName = PACKAGE_NAME;
    moduleInfo.description = BUNDLE_DESCRIPTION;

    AppExecFwk::CustomizeData customizeData {
        "name",
        "value",
        "extra"
    };
    MetaData metaData {
        {customizeData}
    };
    moduleInfo.metaData = metaData;
    innerbundleinfo.SetCurrentModulePackage(PACKAGE_NAME);
    innerbundleinfo.SetBaseApplicationInfo(appInfo);
    innerbundleinfo.SetBaseBundleInfo(bundleInfo);
    innerbundleinfo.InsertInnerModuleInfo(PACKAGE_NAME, moduleInfo);
}

void BmsBundlePermissionTest::MockDefPermissionBundleUserGrant(InnerBundleInfo &innerbundleinfo)
{
    ApplicationInfo appInfo;
    appInfo.bundleName = MOCK_DEFPERMISSION_NAME_4;
    appInfo.name = MOCK_DEFPERMISSION_NAME_4;
    appInfo.deviceId = DEVICE_ID;
    appInfo.process = PROCESS_TEST;
    appInfo.label = BUNDLE_LABEL;
    appInfo.description = BUNDLE_DESCRIPTION;
    appInfo.codePath = CODE_PATH;
    appInfo.dataDir = FILES_DIR;
    appInfo.dataBaseDir = DATA_BASE_DIR;
    appInfo.cacheDir = CACHE_DIR;

    BundleInfo bundleInfo;
    bundleInfo.name = MOCK_DEFPERMISSION_NAME_4;
    bundleInfo.label = BUNDLE_LABEL;
    bundleInfo.description = BUNDLE_DESCRIPTION;
    bundleInfo.vendor = BUNDLE_VENDOR;
    bundleInfo.versionCode = BUNDLE_VERSION_CODE;
    bundleInfo.versionName = BUNDLE_VERSION_NAME;
    bundleInfo.minSdkVersion = BUNDLE_MIN_SDK_VERSION;
    bundleInfo.maxSdkVersion = BUNDLE_MAX_SDK_VERSION;
    bundleInfo.mainEntry = MAIN_ENTRY;
    bundleInfo.isKeepAlive = true;

    InnerModuleInfo moduleInfo;
    std::map<std::string, InnerModuleInfo> innerModuleInfos_;
    moduleInfo.defPermissions = {DEFPERMISSION_USERGRANT1,
        DEFPERMISSION_USERGRANT2,
        DEFPERMISSION_USERGRANT3,
        DEFPERMISSION_USERGRANT4,
        DEFPERMISSION_USERGRANT5,
        DEFPERMISSION_USERGRANT6};
    moduleInfo.modulePackage = PACKAGE_NAME;
    moduleInfo.moduleName = PACKAGE_NAME;
    moduleInfo.description = BUNDLE_DESCRIPTION;

    AppExecFwk::CustomizeData customizeData {
        "name",
        "value",
        "extra"
    };
    MetaData metaData {
        {customizeData}
    };
    moduleInfo.metaData = metaData;
    innerbundleinfo.SetCurrentModulePackage(PACKAGE_NAME);
    innerbundleinfo.SetBaseApplicationInfo(appInfo);
    innerbundleinfo.SetBaseBundleInfo(bundleInfo);
    innerbundleinfo.InsertInnerModuleInfo(PACKAGE_NAME, moduleInfo);
}

void BmsBundlePermissionTest::MockSameDefPermissionNameBundleSystemGrant(InnerBundleInfo &innerbundleinfo)
{
    ApplicationInfo appInfo;
    appInfo.bundleName = MOCK_DEFPERMISSION_NAME_5;
    appInfo.name = MOCK_DEFPERMISSION_NAME_5;
    appInfo.deviceId = DEVICE_ID;
    appInfo.process = PROCESS_TEST;
    appInfo.label = BUNDLE_LABEL;
    appInfo.description = BUNDLE_DESCRIPTION;
    appInfo.codePath = CODE_PATH;
    appInfo.dataDir = FILES_DIR;
    appInfo.dataBaseDir = DATA_BASE_DIR;
    appInfo.cacheDir = CACHE_DIR;

    BundleInfo bundleInfo;
    bundleInfo.name = MOCK_DEFPERMISSION_NAME_5;
    bundleInfo.label = BUNDLE_LABEL;
    bundleInfo.description = BUNDLE_DESCRIPTION;
    bundleInfo.vendor = BUNDLE_VENDOR;
    bundleInfo.versionCode = BUNDLE_VERSION_CODE;
    bundleInfo.versionName = BUNDLE_VERSION_NAME;
    bundleInfo.minSdkVersion = BUNDLE_MIN_SDK_VERSION;
    bundleInfo.maxSdkVersion = BUNDLE_MAX_SDK_VERSION;
    bundleInfo.mainEntry = MAIN_ENTRY;
    bundleInfo.isKeepAlive = true;

    InnerModuleInfo moduleInfo;
    std::map<std::string, InnerModuleInfo> innerModuleInfos_;

    moduleInfo.defPermissions = {DEFPERMISSION_SAMENAME1, DEFPERMISSION_SAMENAME2};
    moduleInfo.modulePackage = PACKAGE_NAME;
    moduleInfo.moduleName = PACKAGE_NAME;
    moduleInfo.description = BUNDLE_DESCRIPTION;

    AppExecFwk::CustomizeData customizeData {
        "name",
        "value",
        "extra"
    };
    MetaData metaData {
        {customizeData}
    };
    moduleInfo.metaData = metaData;
    innerbundleinfo.SetCurrentModulePackage(PACKAGE_NAME);
    innerbundleinfo.SetBaseApplicationInfo(appInfo);
    innerbundleinfo.SetBaseBundleInfo(bundleInfo);
    innerbundleinfo.InsertInnerModuleInfo(PACKAGE_NAME, moduleInfo);
}

void BmsBundlePermissionTest::MockDefPermissionBundleSame(InnerBundleInfo &innerbundleinfo)
{
    ApplicationInfo appInfo;
    appInfo.bundleName = MOCK_DEFPERMISSION_NAME_7;
    appInfo.name = MOCK_DEFPERMISSION_NAME_7;
    appInfo.deviceId = DEVICE_ID;
    appInfo.process = PROCESS_TEST;
    appInfo.label = BUNDLE_LABEL;
    appInfo.description = BUNDLE_DESCRIPTION;
    appInfo.codePath = CODE_PATH;
    appInfo.dataDir = FILES_DIR;
    appInfo.dataBaseDir = DATA_BASE_DIR;
    appInfo.cacheDir = CACHE_DIR;

    BundleInfo bundleInfo;
    bundleInfo.name = MOCK_DEFPERMISSION_NAME_7;
    bundleInfo.label = BUNDLE_LABEL;
    bundleInfo.description = BUNDLE_DESCRIPTION;
    bundleInfo.vendor = BUNDLE_VENDOR;
    bundleInfo.versionCode = BUNDLE_VERSION_CODE;
    bundleInfo.versionName = BUNDLE_VERSION_NAME;
    bundleInfo.minSdkVersion = BUNDLE_MIN_SDK_VERSION;
    bundleInfo.maxSdkVersion = BUNDLE_MAX_SDK_VERSION;
    bundleInfo.mainEntry = MAIN_ENTRY;
    bundleInfo.isKeepAlive = true;

    InnerModuleInfo moduleInfo;
    std::map<std::string, InnerModuleInfo> innerModuleInfos_;
    moduleInfo.defPermissions = {DEFPERMISSION_SAME1, DEFPERMISSION_SAME2, DEFPERMISSION_SAME3};
    moduleInfo.modulePackage = PACKAGE_NAME;
    moduleInfo.moduleName = PACKAGE_NAME;
    moduleInfo.description = BUNDLE_DESCRIPTION;

    AppExecFwk::CustomizeData customizeData {
        "name",
        "value",
        "extra"
    };
    MetaData metaData {
        {customizeData}
    };
    moduleInfo.metaData = metaData;
    innerbundleinfo.SetCurrentModulePackage(PACKAGE_NAME);
    innerbundleinfo.SetBaseApplicationInfo(appInfo);
    innerbundleinfo.SetBaseBundleInfo(bundleInfo);
    innerbundleinfo.InsertInnerModuleInfo(PACKAGE_NAME, moduleInfo);
}

void BmsBundlePermissionTest::MockReqPermissionBundle(InnerBundleInfo &innerbundleinfo)
{
    ApplicationInfo appInfo;
    appInfo.bundleName = MOCK_REQPERMISSION_NAME;
    appInfo.name = MOCK_REQPERMISSION_NAME;
    appInfo.deviceId = DEVICE_ID;
    appInfo.process = PROCESS_TEST;
    appInfo.label = BUNDLE_LABEL;
    appInfo.description = BUNDLE_DESCRIPTION;
    appInfo.codePath = CODE_PATH;
    appInfo.dataDir = FILES_DIR;
    appInfo.dataBaseDir = DATA_BASE_DIR;
    appInfo.cacheDir = CACHE_DIR;

    BundleInfo bundleInfo;
    bundleInfo.name = MOCK_REQPERMISSION_NAME;
    bundleInfo.label = BUNDLE_LABEL;
    bundleInfo.description = BUNDLE_DESCRIPTION;
    bundleInfo.vendor = BUNDLE_VENDOR;
    bundleInfo.versionCode = BUNDLE_VERSION_CODE;
    bundleInfo.versionName = BUNDLE_VERSION_NAME;
    bundleInfo.minSdkVersion = BUNDLE_MIN_SDK_VERSION;
    bundleInfo.maxSdkVersion = BUNDLE_MAX_SDK_VERSION;
    bundleInfo.mainEntry = MAIN_ENTRY;
    bundleInfo.isKeepAlive = true;

    InnerModuleInfo moduleInfo;
    std::map<std::string, InnerModuleInfo> innerModuleInfos_;

    RequestPermission reqPermission1;
    reqPermission1.name = DEFPERMISSION_NAME_EMAIL;
    RequestPermission reqPermission2;
    reqPermission2.name = DEFPERMISSION_NAME_WECHAT;
    moduleInfo.requestPermissions = {reqPermission1, reqPermission2};
    moduleInfo.modulePackage = PACKAGE_NAME;
    moduleInfo.moduleName = PACKAGE_NAME;
    moduleInfo.description = BUNDLE_DESCRIPTION;

    AppExecFwk::CustomizeData customizeData {
        "name",
        "value",
        "extra"
    };
    MetaData metaData {
        {customizeData}
    };
    moduleInfo.metaData = metaData;
    innerbundleinfo.SetCurrentModulePackage(PACKAGE_NAME);
    innerbundleinfo.SetBaseApplicationInfo(appInfo);
    innerbundleinfo.SetBaseBundleInfo(bundleInfo);
    innerbundleinfo.InsertInnerModuleInfo(PACKAGE_NAME, moduleInfo);
}

void BmsBundlePermissionTest::MockOtherReqPermissionBundle(InnerBundleInfo &innerbundleinfo)
{
    ApplicationInfo appInfo;
    appInfo.bundleName = MOCK_REQPERMISSION_NAME_1;
    appInfo.name = MOCK_REQPERMISSION_NAME_1;
    appInfo.deviceId = DEVICE_ID;
    appInfo.process = PROCESS_TEST;
    appInfo.label = BUNDLE_LABEL;
    appInfo.description = BUNDLE_DESCRIPTION;
    appInfo.codePath = CODE_PATH;
    appInfo.dataDir = FILES_DIR;
    appInfo.dataBaseDir = DATA_BASE_DIR;
    appInfo.cacheDir = CACHE_DIR;

    BundleInfo bundleInfo;
    bundleInfo.name = MOCK_REQPERMISSION_NAME_1;
    bundleInfo.label = BUNDLE_LABEL;
    bundleInfo.description = BUNDLE_DESCRIPTION;
    bundleInfo.vendor = BUNDLE_VENDOR;
    bundleInfo.versionCode = BUNDLE_VERSION_CODE;
    bundleInfo.versionName = BUNDLE_VERSION_NAME;
    bundleInfo.minSdkVersion = BUNDLE_MIN_SDK_VERSION;
    bundleInfo.maxSdkVersion = BUNDLE_MAX_SDK_VERSION;
    bundleInfo.mainEntry = MAIN_ENTRY;
    bundleInfo.isKeepAlive = true;

    InnerModuleInfo moduleInfo;
    std::map<std::string, InnerModuleInfo> innerModuleInfos_;
    RequestPermission reqPermission3;
    reqPermission3.name = DEFPERMISSION_NAME_APP;
    RequestPermission reqPermission4;
    reqPermission4.name = DEFPERMISSION_NAME_MUSIC;
    moduleInfo.requestPermissions = {reqPermission3, reqPermission4};
    moduleInfo.modulePackage = PACKAGE_NAME;
    moduleInfo.moduleName = PACKAGE_NAME;
    moduleInfo.description = BUNDLE_DESCRIPTION;

    AppExecFwk::CustomizeData customizeData {
        "name",
        "value",
        "extra"
    };
    MetaData metaData {
        {customizeData}
    };
    moduleInfo.metaData = metaData;
    innerbundleinfo.SetCurrentModulePackage(PACKAGE_NAME);
    innerbundleinfo.SetBaseApplicationInfo(appInfo);
    innerbundleinfo.SetBaseBundleInfo(bundleInfo);
    innerbundleinfo.InsertInnerModuleInfo(PACKAGE_NAME, moduleInfo);
}

bool BmsBundlePermissionTest::ConvertPermissionDef(
    Permission::PermissionDef &permDef, const DefinePermission &defPermission, const std::string &bundleName)
{
    permDef.permissionName = defPermission.name;
    permDef.bundleName = bundleName;
    permDef.grantMode = [&defPermission]() -> int {
        if (defPermission.grantMode ==
            ProfileReader::BUNDLE_MODULE_PROFILE_KEY_DEF_PERMISSIONS_GRANTMODE_SYSTEM_GRANT) {
            return Permission::GrantMode::SYSTEM_GRANT;
        }
        return Permission::GrantMode::USER_GRANT;
    }();
    permDef.availableScope = [&defPermission]() -> int {
        uint flag = 0;
        if (std::find(defPermission.availableScope.begin(),
            defPermission.availableScope.end(),
            ProfileReader::BUNDLE_MODULE_PROFILE_KEY_DEF_PERMISSIONS_AVAILABLESCOPE_SIGNATURE) !=
            defPermission.availableScope.end()) {
            flag |= Permission::AvailableScope::AVAILABLE_SCOPE_SIGNATURE;
        }
        if (std::find(defPermission.availableScope.begin(),
            defPermission.availableScope.end(),
            ProfileReader::BUNDLE_MODULE_PROFILE_KEY_DEF_PERMISSIONS_AVAILABLESCOPE_RESTRICTED) !=
            defPermission.availableScope.end()) {
            flag |= Permission::AvailableScope::AVAILABLE_SCOPE_RESTRICTED;
        }
        if (flag == 0) {
            return Permission::AvailableScope::AVAILABLE_SCOPE_ALL;
        }
        return flag;
    }();
    permDef.label = defPermission.label;
    permDef.labelId = defPermission.labelId;
    permDef.description = defPermission.description;
    permDef.descriptionId = defPermission.descriptionId;
    return true;
}

void BmsBundlePermissionTest::CheckPermissionDef(
    const DefinePermission &defPermission, std::string bundlename, PermissionDef &permissionDef)
{
    Permission::PermissionDef permDef;
    ConvertPermissionDef(permDef, defPermission, bundlename);
    EXPECT_EQ(permissionDef.permissionName, permDef.permissionName);
    EXPECT_EQ(permissionDef.bundleName, bundlename);
    EXPECT_EQ(permissionDef.availableScope, permDef.availableScope);
    EXPECT_EQ(permissionDef.grantMode, permDef.grantMode);
    EXPECT_EQ(permissionDef.labelId, permDef.labelId);
    EXPECT_EQ(permissionDef.label, permDef.label);
    EXPECT_EQ(permissionDef.description, permDef.description);
    EXPECT_EQ(permissionDef.descriptionId, permDef.descriptionId);
}

void BmsBundlePermissionTest::CheckErrPermissionDef(
    const DefinePermission &defPermission, std::string bundlename, PermissionDef &permissionDef)
{
    Permission::PermissionDef permDef;
    ConvertPermissionDef(permDef, defPermission, bundlename);
    EXPECT_NE(permissionDef.permissionName, permDef.permissionName);
    EXPECT_NE(permissionDef.bundleName, bundlename);
    EXPECT_NE(permissionDef.availableScope, permDef.availableScope);
    EXPECT_NE(permissionDef.grantMode, permDef.grantMode);
    EXPECT_NE(permissionDef.labelId, permDef.labelId);
    EXPECT_NE(permissionDef.label, permDef.label);
    EXPECT_NE(permissionDef.description, permDef.description);
    EXPECT_NE(permissionDef.descriptionId, permDef.descriptionId);
}

/**
 * @tc.number: BmsBundlePermissionTest
 * Function: PermissionTest
 * @tc.name: test can add install Permission
 * @tc.desc: 1. system running normally
 *           2. test bundle package have no verify cannot install successfully
 */
HWTEST_F(BmsBundlePermissionTest, HapVerify_0200, Function | SmallTest | Level0)
{
    Verify::HapVerifyResult hapVerifyResult;
    bool result = BundleVerifyMgr::HapVerify(HAP_FILE_PATH1, hapVerifyResult);
    EXPECT_EQ(result, false);
}

/**
 * @tc.number: BmsBundlePermissionTest
 * Function: PermissionTest
 * @tc.name: test can add install Permission
 * @tc.desc: 1. system running normally
 *           2. test bundle package have defpermission can install defpermissions successfully
 */
HWTEST_F(BmsBundlePermissionTest, InstallPermissions_0100, Function | SmallTest | Level0)
{
    InnerBundleInfo innerbundleinfo;
    PermissionDef permissionDef;
    MockDefPermissionBundleSystemGrant(innerbundleinfo);
    auto bundlename = innerbundleinfo.GetBundleName();
    BundlePermissionMgr::InstallPermissions(innerbundleinfo);
    BundlePermissionMgr::GetPermissionDef(DEFPERMISSION_NAME1, permissionDef);
    CheckPermissionDef(DEFPERMISSION_SYSTEM1, bundlename, permissionDef);
    BundlePermissionMgr::GetPermissionDef(DEFPERMISSION_NAME2, permissionDef);
    CheckPermissionDef(DEFPERMISSION_SYSTEM2, bundlename, permissionDef);
    BundlePermissionMgr::GetPermissionDef(DEFPERMISSION_NAME3, permissionDef);
    CheckPermissionDef(DEFPERMISSION_SYSTEM3, bundlename, permissionDef);
    BundlePermissionMgr::UninstallPermissions(innerbundleinfo);
}

/**
 * @tc.number: BmsBundlePermissionTest
 * Function: PermissionTest
 * @tc.name: test can add install Permission
 * @tc.desc: 1. system running normally
 *           2. test have two bundle and add two bundle defpermission
 */
HWTEST_F(BmsBundlePermissionTest, InstallPermissions_0200, Function | SmallTest | Level0)
{
    InnerBundleInfo innerbundleinfo1;
    PermissionDef permissionDef1;
    MockDefPermissionBundleSystemGrant(innerbundleinfo1);
    auto bundlename1 = innerbundleinfo1.GetBundleName();
    BundlePermissionMgr::InstallPermissions(innerbundleinfo1);
    BundlePermissionMgr::GetPermissionDef(DEFPERMISSION_NAME1, permissionDef1);
    CheckPermissionDef(DEFPERMISSION_SYSTEM1, bundlename1, permissionDef1);
    BundlePermissionMgr::GetPermissionDef(DEFPERMISSION_NAME2, permissionDef1);
    CheckPermissionDef(DEFPERMISSION_SYSTEM2, bundlename1, permissionDef1);
    BundlePermissionMgr::GetPermissionDef(DEFPERMISSION_NAME3, permissionDef1);
    CheckPermissionDef(DEFPERMISSION_SYSTEM3, bundlename1, permissionDef1);
    InnerBundleInfo innerbundleinfo2;
    PermissionDef permissionDef2;
    MockDefPermissionBundleSame(innerbundleinfo2);
    auto bundlename2 = innerbundleinfo2.GetBundleName();
    BundlePermissionMgr::InstallPermissions(innerbundleinfo2);
    BundlePermissionMgr::GetPermissionDef(DEFPERMISSION_NAME16, permissionDef2);
    CheckPermissionDef(DEFPERMISSION_SAME1, bundlename2, permissionDef2);
    BundlePermissionMgr::GetPermissionDef(DEFPERMISSION_NAME17, permissionDef2);
    CheckPermissionDef(DEFPERMISSION_SAME2, bundlename2, permissionDef2);
    BundlePermissionMgr::GetPermissionDef(DEFPERMISSION_NAME18, permissionDef2);
    CheckPermissionDef(DEFPERMISSION_SAME3, bundlename2, permissionDef2);
    BundlePermissionMgr::UninstallPermissions(innerbundleinfo1);
    BundlePermissionMgr::UninstallPermissions(innerbundleinfo2);
}

/**
 * @tc.number: BmsBundlePermissionTest
 * Function: PermissionTest
 * @tc.name: test can add install Permission
 * @tc.desc: 1. system running normally
 *           2. test bundle package have reqpermission can install reqpermissions successfully
 */
HWTEST_F(BmsBundlePermissionTest, InstallPermissions_0300, Function | SmallTest | Level0)
{
    InnerBundleInfo innerbundleinfo1;
    PermissionDef permissionDef;
    MockDefPermissionBundleTestReqpermission(innerbundleinfo1);
    std::string bundlename = innerbundleinfo1.GetBundleName();
    BundlePermissionMgr::InstallPermissions(innerbundleinfo1);
    BundlePermissionMgr::GetPermissionDef(DEFPERMISSION_NAME_EMAIL, permissionDef);
    CheckPermissionDef(DEFPERMISSION_EMAIL, bundlename, permissionDef);
    BundlePermissionMgr::GetPermissionDef(DEFPERMISSION_NAME_MUSIC, permissionDef);
    CheckPermissionDef(DEFPERMISSION_MUSIC, bundlename, permissionDef);
    BundlePermissionMgr::GetPermissionDef(DEFPERMISSION_NAME_APP, permissionDef);
    CheckPermissionDef(DEFPERMISSION_APP, bundlename, permissionDef);
    BundlePermissionMgr::GetPermissionDef(DEFPERMISSION_NAME_WECHAT, permissionDef);
    CheckPermissionDef(DEFPERMISSION_WECHAT, bundlename, permissionDef);
    InnerBundleInfo innerbundleinfo2;
    MockReqPermissionBundle(innerbundleinfo2);
    BundlePermissionMgr::InstallPermissions(innerbundleinfo2);
    auto result = BundlePermissionMgr::VerifyPermission(MOCK_REQPERMISSION_NAME, DEFPERMISSION_NAME_WECHAT, 1);
    EXPECT_EQ(result, Permission::TypePermissionState::PERMISSION_GRANTED);
    BundlePermissionMgr::UninstallPermissions(innerbundleinfo1);
    BundlePermissionMgr::UninstallPermissions(innerbundleinfo2);
}

/**
 * @tc.number: BmsBundlePermissionTest
 * Function: PermissionTest
 * @tc.name: test can add install Permission
 * @tc.desc: 1. system running normally
 *           2. two test bundle package reqpermission add one bundle defpermission
 */
HWTEST_F(BmsBundlePermissionTest, InstallPermissions_0400, Function | SmallTest | Level0)
{
    PermissionDef permissionDef;
    InnerBundleInfo innerbundleinfo1;
    MockDefPermissionBundleTestReqpermission(innerbundleinfo1);
    auto bundlename = innerbundleinfo1.GetBundleName();
    BundlePermissionMgr::InstallPermissions(innerbundleinfo1);
    BundlePermissionMgr::GetPermissionDef(DEFPERMISSION_NAME_MUSIC, permissionDef);
    CheckPermissionDef(DEFPERMISSION_MUSIC, bundlename, permissionDef);
    BundlePermissionMgr::GetPermissionDef(DEFPERMISSION_NAME_EMAIL, permissionDef);
    CheckPermissionDef(DEFPERMISSION_EMAIL, bundlename, permissionDef);
    BundlePermissionMgr::GetPermissionDef(DEFPERMISSION_NAME_APP, permissionDef);
    CheckPermissionDef(DEFPERMISSION_APP, bundlename, permissionDef);
    BundlePermissionMgr::GetPermissionDef(DEFPERMISSION_NAME_WECHAT, permissionDef);
    CheckPermissionDef(DEFPERMISSION_WECHAT, bundlename, permissionDef);
    InnerBundleInfo innerbundleinfo2;
    MockReqPermissionBundle(innerbundleinfo2);
    BundlePermissionMgr::InstallPermissions(innerbundleinfo2);
    auto result1 = BundlePermissionMgr::VerifyPermission(MOCK_REQPERMISSION_NAME, DEFPERMISSION_NAME_WECHAT, 1);
    auto result2 = BundlePermissionMgr::VerifyPermission(MOCK_REQPERMISSION_NAME, DEFPERMISSION_NAME_EMAIL, 1);
    EXPECT_EQ(result1, Permission::TypePermissionState::PERMISSION_GRANTED);
    EXPECT_EQ(result2, Permission::TypePermissionState::PERMISSION_GRANTED);
    InnerBundleInfo innerbundleinfo3;
    MockOtherReqPermissionBundle(innerbundleinfo3);
    BundlePermissionMgr::InstallPermissions(innerbundleinfo3);
    auto result3 = BundlePermissionMgr::VerifyPermission(MOCK_REQPERMISSION_NAME_1, DEFPERMISSION_NAME_MUSIC, 1);
    auto result4 = BundlePermissionMgr::VerifyPermission(MOCK_REQPERMISSION_NAME_1, DEFPERMISSION_NAME_APP, 1);
    EXPECT_EQ(result3, Permission::TypePermissionState::PERMISSION_GRANTED);
    EXPECT_EQ(result4, Permission::TypePermissionState::PERMISSION_GRANTED);
    BundlePermissionMgr::UninstallPermissions(innerbundleinfo1);
    BundlePermissionMgr::UninstallPermissions(innerbundleinfo2);
    BundlePermissionMgr::UninstallPermissions(innerbundleinfo3);
}

/**
 * @tc.number: BmsBundlePermissionTest
 * Function: PermissionTest
 * @tc.name: test can add install Permission
 * @tc.desc: 1. system running normally
 *           2. test have NULL defpermission can not install defpermissions successfully
 */
HWTEST_F(BmsBundlePermissionTest, InstallPermissions_0500, Function | SmallTest | Level0)
{
    InnerBundleInfo innerbundleinfo;
    PermissionDef permissionDef;
    MockNoDefPermissionBundle(innerbundleinfo);
    auto bundlename = innerbundleinfo.GetBundleName();
    BundlePermissionMgr::InstallPermissions(innerbundleinfo);
    BundlePermissionMgr::GetPermissionDef(MOCK_REQPERMISSION_NAME_1, permissionDef);
    CheckErrPermissionDef(DEFPERMISSION_WECHAT, bundlename, permissionDef);
    BundlePermissionMgr::UninstallPermissions(innerbundleinfo);
}

/**
 * @tc.number: BmsBundlePermissionTest
 * Function: PermissionTest
 * @tc.name: test can install Permission can be add
 * @tc.desc: 1. system running normally
 *           2. test bundle package have no defPermission and cannot install reqpermisssion
 */
HWTEST_F(BmsBundlePermissionTest, InstallPermissions_0700, Function | SmallTest | Level0)
{
    InnerBundleInfo innerbundleinfo1;
    MockNoDefPermissionBundle(innerbundleinfo1);
    BundlePermissionMgr::InstallPermissions(innerbundleinfo1);
    PermissionDef permissionDef;
    auto bundlename = innerbundleinfo1.GetBundleName();
    CheckErrPermissionDef(DEFPERMISSION_WECHAT, bundlename, permissionDef);
    InnerBundleInfo innerbundleinfo2;
    MockReqPermissionBundle(innerbundleinfo2);
    BundlePermissionMgr::InstallPermissions(innerbundleinfo2);
    auto result = BundlePermissionMgr::VerifyPermission(MOCK_REQPERMISSION_NAME, DEFPERMISSION_NAME1, 1);
    EXPECT_EQ(result, Permission::TypePermissionState::PERMISSION_NOT_GRANTED);
    BundlePermissionMgr::UninstallPermissions(innerbundleinfo1);
    BundlePermissionMgr::UninstallPermissions(innerbundleinfo2);
}

/*
 * @tc.number: BmsBundlePermissionTest
 * Function: PermissionTest
 * @tc.name: test can install Permission can be add
 * @tc.desc: 1. system running normally
 *           2. two test bundle package have the same defpermission name
 */
HWTEST_F(BmsBundlePermissionTest, InstallPermissions_0800, Function | SmallTest | Level0)
{
    InnerBundleInfo innerbundleinfo1;
    PermissionDef permissionDef1;
    MockSameDefPermissionNameBundleSystemGrant(innerbundleinfo1);
    auto bundlename = innerbundleinfo1.GetBundleName();
    BundlePermissionMgr::InstallPermissions(innerbundleinfo1);
    BundlePermissionMgr::GetPermissionDef(DEFPERMISSION_NAME10, permissionDef1);
    CheckPermissionDef(DEFPERMISSION_SAMENAME1, bundlename, permissionDef1);
    BundlePermissionMgr::GetPermissionDef(DEFPERMISSION_NAME11, permissionDef1);
    CheckPermissionDef(DEFPERMISSION_SAMENAME2, bundlename, permissionDef1);
    InnerBundleInfo innerbundleinfo2;
    PermissionDef permissionDef2;
    MockDefPermissionBundleUserGrant(innerbundleinfo2);
    BundlePermissionMgr::InstallPermissions(innerbundleinfo2);
    BundlePermissionMgr::GetPermissionDef(DEFPERMISSION_NAME10, permissionDef2);
    EXPECT_NE(permissionDef2.label, DEF_LABEL2);
    EXPECT_NE(permissionDef2.grantMode, Permission::GrantMode::USER_GRANT);
    BundlePermissionMgr::GetPermissionDef(DEFPERMISSION_NAME11, permissionDef2);
    EXPECT_NE(permissionDef2.label, DEF_LABEL2);
    EXPECT_NE(permissionDef2.grantMode, Permission::GrantMode::USER_GRANT);
    auto bundlename1 = innerbundleinfo2.GetBundleName();
    BundlePermissionMgr::GetPermissionDef(DEFPERMISSION_NAME12, permissionDef2);
    CheckPermissionDef(DEFPERMISSION_USERGRANT3, bundlename1, permissionDef2);
    BundlePermissionMgr::GetPermissionDef(DEFPERMISSION_NAME13, permissionDef2);
    CheckPermissionDef(DEFPERMISSION_USERGRANT4, bundlename1, permissionDef2);
    BundlePermissionMgr::GetPermissionDef(DEFPERMISSION_NAME14, permissionDef2);
    CheckPermissionDef(DEFPERMISSION_USERGRANT5, bundlename1, permissionDef2);
    BundlePermissionMgr::GetPermissionDef(DEFPERMISSION_NAME15, permissionDef2);
    CheckPermissionDef(DEFPERMISSION_USERGRANT6, bundlename1, permissionDef2);
    BundlePermissionMgr::UninstallPermissions(innerbundleinfo1);
    BundlePermissionMgr::UninstallPermissions(innerbundleinfo2);
}

/**
 * @tc.number: BmsBundlePermissionTest
 * Function: PermissionTest
 * @tc.name: parse bundle package by config.json
 * @tc.desc: 1. system running normally
 *           2. test bundle package have defpermission can Uninstall defPermissions successfully
 */
HWTEST_F(BmsBundlePermissionTest, UninstallPermissions_0100, Function | SmallTest | Level0)
{
    InnerBundleInfo innerbundleinfo;
    PermissionDef permissionDef;
    PermissionDef permissionDef1;
    MockDefPermissionBundleSystemGrant(innerbundleinfo);
    auto bundlename = innerbundleinfo.GetBundleName();
    BundlePermissionMgr::InstallPermissions(innerbundleinfo);
    BundlePermissionMgr::GetPermissionDef(DEFPERMISSION_NAME1, permissionDef1);
    CheckPermissionDef(DEFPERMISSION_SYSTEM1, bundlename, permissionDef1);
    BundlePermissionMgr::GetPermissionDef(DEFPERMISSION_NAME2, permissionDef1);
    CheckPermissionDef(DEFPERMISSION_SYSTEM2, bundlename, permissionDef1);
    BundlePermissionMgr::GetPermissionDef(DEFPERMISSION_NAME3, permissionDef1);
    CheckPermissionDef(DEFPERMISSION_SYSTEM3, bundlename, permissionDef1);
    BundlePermissionMgr::UninstallPermissions(innerbundleinfo);
    auto bundlename1 = innerbundleinfo.GetBundleName();
    BundlePermissionMgr::GetPermissionDef(DEFPERMISSION_NAME1, permissionDef);
    CheckErrPermissionDef(DEFPERMISSION_SYSTEM1, bundlename1, permissionDef);
    BundlePermissionMgr::GetPermissionDef(DEFPERMISSION_NAME2, permissionDef);
    CheckErrPermissionDef(DEFPERMISSION_SYSTEM2, bundlename1, permissionDef);
    BundlePermissionMgr::GetPermissionDef(DEFPERMISSION_NAME3, permissionDef);
    CheckErrPermissionDef(DEFPERMISSION_SYSTEM3, bundlename1, permissionDef);
    BundlePermissionMgr::UninstallPermissions(innerbundleinfo);
}

/**
 * @tc.number: BmsBundlePermissionTest
 * Function: PermissionTest
 * @tc.name: parse bundle package by config.json
 * @tc.desc: 1. system running normally
 *           2. test bundle package have reqpermission can Uninstall reqPermissions successfully
 */
HWTEST_F(BmsBundlePermissionTest, UninstallPermissions_0200, Function | SmallTest | Level0)
{
    InnerBundleInfo innerbundleinfo;
    PermissionDef permissionDef;
    MockDefPermissionBundleTestReqpermission(innerbundleinfo);
    auto bundlename = innerbundleinfo.GetBundleName();
    BundlePermissionMgr::InstallPermissions(innerbundleinfo);
    BundlePermissionMgr::GetPermissionDef(DEFPERMISSION_NAME_EMAIL, permissionDef);
    CheckPermissionDef(DEFPERMISSION_EMAIL, bundlename, permissionDef);
    BundlePermissionMgr::GetPermissionDef(DEFPERMISSION_NAME_APP, permissionDef);
    CheckPermissionDef(DEFPERMISSION_APP, bundlename, permissionDef);
    BundlePermissionMgr::GetPermissionDef(DEFPERMISSION_NAME_WECHAT, permissionDef);
    CheckPermissionDef(DEFPERMISSION_WECHAT, bundlename, permissionDef);
    BundlePermissionMgr::GetPermissionDef(DEFPERMISSION_NAME_MUSIC, permissionDef);
    CheckPermissionDef(DEFPERMISSION_MUSIC, bundlename, permissionDef);
    InnerBundleInfo innerbundleinfo1;
    MockReqPermissionBundle(innerbundleinfo1);
    BundlePermissionMgr::InstallPermissions(innerbundleinfo1);
    auto result = BundlePermissionMgr::VerifyPermission(MOCK_REQPERMISSION_NAME, DEFPERMISSION_NAME_EMAIL, 1);
    EXPECT_EQ(result, Permission::TypePermissionState::PERMISSION_GRANTED);
    auto result1 = BundlePermissionMgr::VerifyPermission(MOCK_REQPERMISSION_NAME, DEFPERMISSION_NAME_WECHAT, 1);
    EXPECT_EQ(result1, Permission::TypePermissionState::PERMISSION_GRANTED);
    BundlePermissionMgr::UninstallPermissions(innerbundleinfo1);
    auto result2 = BundlePermissionMgr::VerifyPermission(MOCK_REQPERMISSION_NAME, DEFPERMISSION_NAME_EMAIL, 1);
    EXPECT_EQ(result2, Permission::TypePermissionState::PERMISSION_NOT_GRANTED);
    auto result3 = BundlePermissionMgr::VerifyPermission(MOCK_REQPERMISSION_NAME, DEFPERMISSION_NAME_WECHAT, 1);
    EXPECT_EQ(result3, Permission::TypePermissionState::PERMISSION_NOT_GRANTED);
    BundlePermissionMgr::UninstallPermissions(innerbundleinfo);
    BundlePermissionMgr::UninstallPermissions(innerbundleinfo1);
}

/**
 * @tc.number: BmsBundlePermissionTest
 * Function: PermissionTest
 * @tc.name: parse bundle package by config.json
 * @tc.desc: 1. system running normally
 *           2. two test bundle package have different defpermission can Uninstall one defPermissions successfully
 */
HWTEST_F(BmsBundlePermissionTest, UninstallPermissions_0300, Function | SmallTest | Level0)
{
    InnerBundleInfo innerbundleinfo;
    PermissionDef permissionDef;
    MockDefPermissionBundleSystemGrant(innerbundleinfo);
    auto bundlename = innerbundleinfo.GetBundleName();
    BundlePermissionMgr::InstallPermissions(innerbundleinfo);
    BundlePermissionMgr::GetPermissionDef(DEFPERMISSION_NAME1, permissionDef);
    CheckPermissionDef(DEFPERMISSION_SYSTEM1, bundlename, permissionDef);
    BundlePermissionMgr::GetPermissionDef(DEFPERMISSION_NAME2, permissionDef);
    CheckPermissionDef(DEFPERMISSION_SYSTEM2, bundlename, permissionDef);
    BundlePermissionMgr::GetPermissionDef(DEFPERMISSION_NAME3, permissionDef);
    CheckPermissionDef(DEFPERMISSION_SYSTEM3, bundlename, permissionDef);
    InnerBundleInfo innerbundleinfo1;
    PermissionDef permissionDef1;
    MockDefPermissionBundleSame(innerbundleinfo1);
    auto bundlename1 = innerbundleinfo1.GetBundleName();
    BundlePermissionMgr::InstallPermissions(innerbundleinfo1);
    BundlePermissionMgr::GetPermissionDef(DEFPERMISSION_NAME16, permissionDef1);
    CheckPermissionDef(DEFPERMISSION_SAME1, bundlename1, permissionDef1);
    BundlePermissionMgr::GetPermissionDef(DEFPERMISSION_NAME17, permissionDef1);
    CheckPermissionDef(DEFPERMISSION_SAME2, bundlename1, permissionDef1);
    BundlePermissionMgr::GetPermissionDef(DEFPERMISSION_NAME18, permissionDef1);
    CheckPermissionDef(DEFPERMISSION_SAME3, bundlename1, permissionDef1);
    PermissionDef permissionDef3;
    BundlePermissionMgr::UninstallPermissions(innerbundleinfo);
    auto bundlename2 = innerbundleinfo.GetBundleName();
    BundlePermissionMgr::GetPermissionDef(DEFPERMISSION_NAME1, permissionDef3);
    CheckErrPermissionDef(DEFPERMISSION_SYSTEM1, bundlename2, permissionDef3);
    BundlePermissionMgr::GetPermissionDef(DEFPERMISSION_NAME2, permissionDef3);
    CheckErrPermissionDef(DEFPERMISSION_SYSTEM2, bundlename2, permissionDef3);
    BundlePermissionMgr::GetPermissionDef(DEFPERMISSION_NAME3, permissionDef3);
    CheckErrPermissionDef(DEFPERMISSION_SYSTEM3, bundlename2, permissionDef3);
    BundlePermissionMgr::GetPermissionDef(DEFPERMISSION_NAME16, permissionDef1);
    CheckPermissionDef(DEFPERMISSION_SAME1, bundlename1, permissionDef1);
    BundlePermissionMgr::GetPermissionDef(DEFPERMISSION_NAME17, permissionDef1);
    CheckPermissionDef(DEFPERMISSION_SAME2, bundlename1, permissionDef1);
    BundlePermissionMgr::GetPermissionDef(DEFPERMISSION_NAME18, permissionDef1);
    CheckPermissionDef(DEFPERMISSION_SAME3, bundlename1, permissionDef1);
    BundlePermissionMgr::UninstallPermissions(innerbundleinfo);
    BundlePermissionMgr::UninstallPermissions(innerbundleinfo1);
}

/**
 * @tc.number: BmsBundlePermissionTest
 * Function: PermissionTest
 * @tc.name: parse bundle package by config.json
 * @tc.desc: 1. system running normally
 *           2. two test bundle package have the same reqpermission can Uninstall one req successfully
 */
HWTEST_F(BmsBundlePermissionTest, UninstallPermissions_0400, Function | SmallTest | Level0)
{
    InnerBundleInfo innerbundleinfo1;
    PermissionDef permissionDef1;
    MockDefPermissionBundleTestReqpermission(innerbundleinfo1);
    BundlePermissionMgr::InstallPermissions(innerbundleinfo1);
    auto bundlename = innerbundleinfo1.GetBundleName();
    BundlePermissionMgr::GetPermissionDef(DEFPERMISSION_NAME_EMAIL, permissionDef1);
    CheckPermissionDef(DEFPERMISSION_EMAIL, bundlename, permissionDef1);
    BundlePermissionMgr::GetPermissionDef(DEFPERMISSION_NAME_APP, permissionDef1);
    CheckPermissionDef(DEFPERMISSION_APP, bundlename, permissionDef1);
    BundlePermissionMgr::GetPermissionDef(DEFPERMISSION_NAME_MUSIC, permissionDef1);
    CheckPermissionDef(DEFPERMISSION_MUSIC, bundlename, permissionDef1);
    BundlePermissionMgr::GetPermissionDef(DEFPERMISSION_NAME_WECHAT, permissionDef1);
    CheckPermissionDef(DEFPERMISSION_WECHAT, bundlename, permissionDef1);
    InnerBundleInfo innerbundleinfo2;
    PermissionDef permissionDef2;
    MockReqPermissionBundle(innerbundleinfo2);
    BundlePermissionMgr::InstallPermissions(innerbundleinfo2);
    auto result1 = BundlePermissionMgr::VerifyPermission(MOCK_REQPERMISSION_NAME, DEFPERMISSION_NAME_EMAIL, 1);
    auto result2 = BundlePermissionMgr::VerifyPermission(MOCK_REQPERMISSION_NAME, DEFPERMISSION_NAME_WECHAT, 1);
    EXPECT_EQ(result1, Permission::TypePermissionState::PERMISSION_GRANTED);
    EXPECT_EQ(result2, Permission::TypePermissionState::PERMISSION_GRANTED);
    InnerBundleInfo innerbundleinfo3;
    PermissionDef permissionDef3;
    MockOtherReqPermissionBundle(innerbundleinfo3);
    BundlePermissionMgr::InstallPermissions(innerbundleinfo3);
    auto result3 = BundlePermissionMgr::VerifyPermission(MOCK_REQPERMISSION_NAME_1, DEFPERMISSION_NAME_APP, 1);
    auto result4 = BundlePermissionMgr::VerifyPermission(MOCK_REQPERMISSION_NAME_1, DEFPERMISSION_NAME_MUSIC, 1);
    EXPECT_EQ(result3, Permission::TypePermissionState::PERMISSION_GRANTED);
    EXPECT_EQ(result4, Permission::TypePermissionState::PERMISSION_GRANTED);
    BundlePermissionMgr::UninstallPermissions(innerbundleinfo2);
    auto result5 = BundlePermissionMgr::VerifyPermission(MOCK_REQPERMISSION_NAME, DEFPERMISSION_NAME_EMAIL, 1);
    auto result6 = BundlePermissionMgr::VerifyPermission(MOCK_REQPERMISSION_NAME, DEFPERMISSION_NAME_WECHAT, 1);
    auto result7 = BundlePermissionMgr::VerifyPermission(MOCK_REQPERMISSION_NAME_1, DEFPERMISSION_NAME_APP, 1);
    auto result8 = BundlePermissionMgr::VerifyPermission(MOCK_REQPERMISSION_NAME_1, DEFPERMISSION_NAME_MUSIC, 1);
    EXPECT_EQ(result5, Permission::TypePermissionState::PERMISSION_NOT_GRANTED);
    EXPECT_EQ(result6, Permission::TypePermissionState::PERMISSION_NOT_GRANTED);
    EXPECT_EQ(result7, Permission::TypePermissionState::PERMISSION_GRANTED);
    EXPECT_EQ(result8, Permission::TypePermissionState::PERMISSION_GRANTED);
    BundlePermissionMgr::UninstallPermissions(innerbundleinfo1);
    BundlePermissionMgr::UninstallPermissions(innerbundleinfo2);
    BundlePermissionMgr::UninstallPermissions(innerbundleinfo3);
}

/**
 * @tc.number: BmsBundlePermissionTest
 * Function: PermissionTest
 * @tc.name: parse bundle package by config.json
 * @tc.desc: 1. system running normally
 *           2. test bundle package have error bundleinfo cannot Uninstall defpermissionName
 */
HWTEST_F(BmsBundlePermissionTest, UninstallPermissions_0500, Function | SmallTest | Level0)
{
    InnerBundleInfo innerbundleinfo;
    PermissionDef permissionDef;
    MockDefPermissionBundleTestReqpermission(innerbundleinfo);
    BundlePermissionMgr::InstallPermissions(innerbundleinfo);
    auto bundlename = innerbundleinfo.GetBundleName();
    BundlePermissionMgr::GetPermissionDef(DEFPERMISSION_NAME_EMAIL, permissionDef);
    CheckPermissionDef(DEFPERMISSION_EMAIL, bundlename, permissionDef);
    BundlePermissionMgr::GetPermissionDef(DEFPERMISSION_NAME_APP, permissionDef);
    CheckPermissionDef(DEFPERMISSION_APP, bundlename, permissionDef);
    BundlePermissionMgr::GetPermissionDef(DEFPERMISSION_NAME_WECHAT, permissionDef);
    CheckPermissionDef(DEFPERMISSION_WECHAT, bundlename, permissionDef);
    BundlePermissionMgr::GetPermissionDef(DEFPERMISSION_NAME_MUSIC, permissionDef);
    CheckPermissionDef(DEFPERMISSION_MUSIC, bundlename, permissionDef);
    InnerBundleInfo innerbundleinfo1;
    PermissionDef permissionDef1;
    MockNoDefPermissionBundle(innerbundleinfo1);
    BundlePermissionMgr::UninstallPermissions(innerbundleinfo1);
    auto bundlename1 = innerbundleinfo.GetBundleName();
    BundlePermissionMgr::GetPermissionDef(DEFPERMISSION_NAME_EMAIL, permissionDef1);
    CheckPermissionDef(DEFPERMISSION_EMAIL, bundlename1, permissionDef1);
    BundlePermissionMgr::GetPermissionDef(DEFPERMISSION_NAME_APP, permissionDef1);
    CheckPermissionDef(DEFPERMISSION_APP, bundlename1, permissionDef1);
    BundlePermissionMgr::GetPermissionDef(DEFPERMISSION_NAME_WECHAT, permissionDef1);
    CheckPermissionDef(DEFPERMISSION_WECHAT, bundlename1, permissionDef1);
    BundlePermissionMgr::GetPermissionDef(DEFPERMISSION_NAME_MUSIC, permissionDef1);
    CheckPermissionDef(DEFPERMISSION_MUSIC, bundlename1, permissionDef1);
    BundlePermissionMgr::UninstallPermissions(innerbundleinfo1);
    BundlePermissionMgr::UninstallPermissions(innerbundleinfo);
}

/**
 * @tc.number: BmsBundlePermissionTest
 * Function: PermissionTest
 * @tc.name: parse bundle package by config.json
 * @tc.desc: 1. system running normally
 *           2. test bundle package Uninstall error bundle nonentity reqPermissions
 */
HWTEST_F(BmsBundlePermissionTest, UninstallPermissions_0600, Function | SmallTest | Level0)
{
    InnerBundleInfo innerbundleinfo;
    PermissionDef permissionDef;
    MockDefPermissionBundleTestReqpermission(innerbundleinfo);
    BundlePermissionMgr::InstallPermissions(innerbundleinfo);
    auto bundlename = innerbundleinfo.GetBundleName();
    BundlePermissionMgr::GetPermissionDef(DEFPERMISSION_NAME_MUSIC, permissionDef);
    CheckPermissionDef(DEFPERMISSION_MUSIC, bundlename, permissionDef);
    BundlePermissionMgr::GetPermissionDef(DEFPERMISSION_NAME_WECHAT, permissionDef);
    CheckPermissionDef(DEFPERMISSION_WECHAT, bundlename, permissionDef);
    BundlePermissionMgr::GetPermissionDef(DEFPERMISSION_NAME_APP, permissionDef);
    CheckPermissionDef(DEFPERMISSION_APP, bundlename, permissionDef);
    BundlePermissionMgr::GetPermissionDef(DEFPERMISSION_NAME_EMAIL, permissionDef);
    CheckPermissionDef(DEFPERMISSION_EMAIL, bundlename, permissionDef);
    InnerBundleInfo innerbundleinfo2;
    MockReqPermissionBundle(innerbundleinfo2);
    BundlePermissionMgr::InstallPermissions(innerbundleinfo2);
    auto result1 = BundlePermissionMgr::VerifyPermission(MOCK_REQPERMISSION_NAME, DEFPERMISSION_NAME_WECHAT, 1);
    EXPECT_EQ(result1, Permission::TypePermissionState::PERMISSION_GRANTED);
    auto result2 = BundlePermissionMgr::VerifyPermission(MOCK_REQPERMISSION_NAME, DEFPERMISSION_NAME_EMAIL, 1);
    EXPECT_EQ(result2, Permission::TypePermissionState::PERMISSION_GRANTED);
    InnerBundleInfo innerbundleinfo3;
    PermissionDef permissionDef1;
    MockNoDefPermissionBundle(innerbundleinfo3);
    BundlePermissionMgr::InstallPermissions(innerbundleinfo3);
    auto bundlename1 = innerbundleinfo3.GetBundleName();
    BundlePermissionMgr::GetPermissionDef(DEFPERMISSION_NAME1, permissionDef1);
    CheckErrPermissionDef(DEFPERMISSION_SYSTEM1, bundlename1, permissionDef1);
    BundlePermissionMgr::GetPermissionDef(DEFPERMISSION_NAME2, permissionDef1);
    CheckErrPermissionDef(DEFPERMISSION_SYSTEM2, bundlename1, permissionDef1);
    BundlePermissionMgr::GetPermissionDef(DEFPERMISSION_NAME3, permissionDef1);
    CheckErrPermissionDef(DEFPERMISSION_SYSTEM3, bundlename1, permissionDef1);
    BundlePermissionMgr::UninstallPermissions(innerbundleinfo3);
    auto result3 = BundlePermissionMgr::VerifyPermission(MOCK_REQPERMISSION_NAME, DEFPERMISSION_NAME_EMAIL, 1);
    EXPECT_EQ(result3, Permission::TypePermissionState::PERMISSION_GRANTED);
    auto result4 = BundlePermissionMgr::VerifyPermission(MOCK_REQPERMISSION_NAME, DEFPERMISSION_NAME_WECHAT, 1);
    EXPECT_EQ(result4, Permission::TypePermissionState::PERMISSION_GRANTED);
    BundlePermissionMgr::UninstallPermissions(innerbundleinfo);
    BundlePermissionMgr::UninstallPermissions(innerbundleinfo2);
    BundlePermissionMgr::UninstallPermissions(innerbundleinfo3);
}

/**
 * @tc.number: BmsBundlePermissionTest
 * Function: PermissionTest
 * @tc.name: parse bundle package by config.json
 * @tc.desc: 1. system running normally
 *           2. test bundle package can Update Permissions successfully
 */
HWTEST_F(BmsBundlePermissionTest, UpdatePermissions_0100, Function | SmallTest | Level0)
{
    InnerBundleInfo innerbundleinfo;
    PermissionDef permissionDef1;
    MockDefPermissionBundleSystemGrant(innerbundleinfo);
    BundlePermissionMgr::InstallPermissions(innerbundleinfo);
    auto bundlename = innerbundleinfo.GetBundleName();
    BundlePermissionMgr::GetPermissionDef(DEFPERMISSION_NAME1, permissionDef1);
    CheckPermissionDef(DEFPERMISSION_SYSTEM1, bundlename, permissionDef1);
    BundlePermissionMgr::GetPermissionDef(DEFPERMISSION_NAME2, permissionDef1);
    CheckPermissionDef(DEFPERMISSION_SYSTEM2, bundlename, permissionDef1);
    BundlePermissionMgr::GetPermissionDef(DEFPERMISSION_NAME3, permissionDef1);
    CheckPermissionDef(DEFPERMISSION_SYSTEM3, bundlename, permissionDef1);
    InnerBundleInfo innerbundleinfo1;
    PermissionDef permissionDef2;
    MockDefPermissionBundleSame(innerbundleinfo1);
    ErrCode result = BundlePermissionMgr::UpdatePermissions(innerbundleinfo1);
    auto bundlename1 = innerbundleinfo1.GetBundleName();
    BundlePermissionMgr::GetPermissionDef(DEFPERMISSION_NAME16, permissionDef2);
    CheckPermissionDef(DEFPERMISSION_SAME1, bundlename1, permissionDef2);
    BundlePermissionMgr::GetPermissionDef(DEFPERMISSION_NAME17, permissionDef2);
    CheckPermissionDef(DEFPERMISSION_SAME2, bundlename1, permissionDef2);
    BundlePermissionMgr::GetPermissionDef(DEFPERMISSION_NAME18, permissionDef2);
    CheckPermissionDef(DEFPERMISSION_SAME3, bundlename1, permissionDef2);
    EXPECT_EQ(result, true);
}

/**
 * @tc.number: BmsBundlePermissionTest
 * Function: PermissionTest
 * @tc.name: parse bundle package by config.json
 * @tc.desc: 1. system running normally
 *           2. test bundle package can CheckCalling Permissions successfully
 */
HWTEST_F(BmsBundlePermissionTest, CheckCallingPermission_0100, Function | SmallTest | Level0)
{
    InnerBundleInfo innerbundleinfo;
    MockDefPermissionBundleSystemGrant(innerbundleinfo);
    std::string permissionName = DEFPERMISSION_NAME1;
    ErrCode result = BundlePermissionMgr::CheckCallingPermission(permissionName);
    EXPECT_EQ(result, true);
}

/**
 * @tc.number: BmsBundlePermissionTest
 * Function: PermissionTest
 * @tc.name: parse bundle package by config.json
 * @tc.desc: 1. system running normally
 *           2. test bundle package can Verify Permissions successfully
 */
HWTEST_F(BmsBundlePermissionTest, VerifyPermission_0100, Function | SmallTest | Level0)
{
    InnerBundleInfo innerbundleinfo1;
    PermissionDef permissionDef1;
    MockDefPermissionBundleTestReqpermission(innerbundleinfo1);
    BundlePermissionMgr::InstallPermissions(innerbundleinfo1);
    BundlePermissionMgr::GetPermissionDef(DEFPERMISSION_NAME_EMAIL, permissionDef1);
    auto bundlename = innerbundleinfo1.GetBundleName();
    CheckPermissionDef(DEFPERMISSION_EMAIL, bundlename, permissionDef1);
    BundlePermissionMgr::GetPermissionDef(DEFPERMISSION_NAME_APP, permissionDef1);
    CheckPermissionDef(DEFPERMISSION_APP, bundlename, permissionDef1);
    BundlePermissionMgr::GetPermissionDef(DEFPERMISSION_NAME_MUSIC, permissionDef1);
    CheckPermissionDef(DEFPERMISSION_MUSIC, bundlename, permissionDef1);
    BundlePermissionMgr::GetPermissionDef(DEFPERMISSION_NAME_WECHAT, permissionDef1);
    CheckPermissionDef(DEFPERMISSION_WECHAT, bundlename, permissionDef1);
    InnerBundleInfo innerbundleinfo2;
    MockReqPermissionBundle(innerbundleinfo2);
    BundlePermissionMgr::InstallPermissions(innerbundleinfo2);
    auto result = BundlePermissionMgr::VerifyPermission(MOCK_REQPERMISSION_NAME, DEFPERMISSION_NAME_EMAIL, 1);
    auto result1 = BundlePermissionMgr::VerifyPermission(MOCK_REQPERMISSION_NAME, DEFPERMISSION_NAME_WECHAT, 1);
    EXPECT_EQ(result, Permission::TypePermissionState::PERMISSION_GRANTED);
    EXPECT_EQ(result1, Permission::TypePermissionState::PERMISSION_GRANTED);
}

/**
 * @tc.number: BmsBundlePermissionTest
 * Function: PermissionTest
 * @tc.name: parse bundle package by config.json
 * @tc.desc: 1. system running normally
 *           2. test bundle package have error permissionName
 */
HWTEST_F(BmsBundlePermissionTest, VerifyPermission_0200, Function | SmallTest | Level0)
{
    InnerBundleInfo innerbundleinfo1;
    PermissionDef permissionDef1;
    MockDefPermissionBundleTestReqpermission(innerbundleinfo1);
    BundlePermissionMgr::InstallPermissions(innerbundleinfo1);
    auto bundlename = innerbundleinfo1.GetBundleName();
    BundlePermissionMgr::GetPermissionDef(DEFPERMISSION_NAME_EMAIL, permissionDef1);
    CheckPermissionDef(DEFPERMISSION_EMAIL, bundlename, permissionDef1);
    BundlePermissionMgr::GetPermissionDef(DEFPERMISSION_NAME_APP, permissionDef1);
    CheckPermissionDef(DEFPERMISSION_APP, bundlename, permissionDef1);
    BundlePermissionMgr::GetPermissionDef(DEFPERMISSION_NAME_MUSIC, permissionDef1);
    CheckPermissionDef(DEFPERMISSION_MUSIC, bundlename, permissionDef1);
    BundlePermissionMgr::GetPermissionDef(DEFPERMISSION_NAME_WECHAT, permissionDef1);
    CheckPermissionDef(DEFPERMISSION_WECHAT, bundlename, permissionDef1);
    InnerBundleInfo innerbundleinfo2;
    MockReqPermissionBundle(innerbundleinfo2);
    BundlePermissionMgr::InstallPermissions(innerbundleinfo2);
    auto result = BundlePermissionMgr::VerifyPermission(MOCK_REQPERMISSION_NAME, "error", 1);
    EXPECT_EQ(result, Permission::TypePermissionState::PERMISSION_NOT_GRANTED);
}

/**
 * @tc.number: BmsBundlePermissionTest
 * Function: PermissionTest
 * @tc.name: parse bundle package by config.json
 * @tc.desc: 1. system running normally
 *           2. test bundle package can get one PermissionDef successfully
 */
HWTEST_F(BmsBundlePermissionTest, GetPermissionDef_0100, Function | SmallTest | Level0)
{
    InnerBundleInfo innerbundleinfo;
    MockDefPermissionBundleSystemGrant(innerbundleinfo);
    auto bundlename = innerbundleinfo.GetBundleName();
    PermissionDef permdef;
    std::string permissionName = DEFPERMISSION_NAME1;
    ErrCode result = BundlePermissionMgr::GetPermissionDef(permissionName, permdef);
    CheckPermissionDef(DEFPERMISSION_SYSTEM1, bundlename, permdef);
    EXPECT_EQ(result, true);
}

/**
 * @tc.number: BmsBundlePermissionTest
 * Function: PermissionTest
 * @tc.name: parse bundle package by config.json
 * @tc.desc: 1. system running normally
 *           2. test bundle package can get two PermissionDef successfully
 */
HWTEST_F(BmsBundlePermissionTest, GetPermissionDef_0200, Function | SmallTest | Level0)
{
    InnerBundleInfo innerbundleinfo;
    MockDefPermissionBundleSystemGrant(innerbundleinfo);
    auto bundlename = innerbundleinfo.GetBundleName();
    PermissionDef permdef1;
    std::string permissionName1 = DEFPERMISSION_NAME1;
    ErrCode result1 = BundlePermissionMgr::GetPermissionDef(permissionName1, permdef1);
    CheckPermissionDef(DEFPERMISSION_SYSTEM1, bundlename, permdef1);
    EXPECT_EQ(result1, true);
    PermissionDef permdef2;
    std::string permissionName2 = DEFPERMISSION_NAME2;
    ErrCode result2 = BundlePermissionMgr::GetPermissionDef(permissionName2, permdef2);
    CheckPermissionDef(DEFPERMISSION_SYSTEM2, bundlename, permdef2);
    EXPECT_EQ(result2, true);
}

/**
 * @tc.number: BmsBundlePermissionTest
 * Function: PermissionTest
 * @tc.name: parse bundle package by config.json
 * @tc.desc: 1. system running normally
 *           2. test two  bundle package can get all PermissionDef successfully
 */
HWTEST_F(BmsBundlePermissionTest, GetPermissionDef_0300, Function | SmallTest | Level0)
{
    InnerBundleInfo innerbundleinfo;
    MockDefPermissionBundleSystemGrant(innerbundleinfo);
    auto bundlename = innerbundleinfo.GetBundleName();
    InnerBundleInfo innerbundleinfo1;
    MockDefPermissionBundleSame(innerbundleinfo1);
    auto bundlename1 = innerbundleinfo1.GetBundleName();
    PermissionDef permdef;
    std::string permissionName1 = DEFPERMISSION_NAME1;
    ErrCode result1 = BundlePermissionMgr::GetPermissionDef(permissionName1, permdef);
    CheckPermissionDef(DEFPERMISSION_SYSTEM1, bundlename, permdef);
    EXPECT_EQ(result1, true);
    PermissionDef permdef1;
    std::string permissionName2 = DEFPERMISSION_NAME2;
    ErrCode result2 = BundlePermissionMgr::GetPermissionDef(permissionName2, permdef1);
    CheckPermissionDef(DEFPERMISSION_SYSTEM2, bundlename, permdef1);
    EXPECT_EQ(result2, true);
    PermissionDef permdef2;
    std::string permissionName3 = DEFPERMISSION_NAME3;
    ErrCode result3 = BundlePermissionMgr::GetPermissionDef(permissionName3, permdef2);
    CheckPermissionDef(DEFPERMISSION_SYSTEM3, bundlename, permdef2);
    EXPECT_EQ(result3, true);
    PermissionDef permdef3;
    std::string permissionName4 = DEFPERMISSION_NAME16;
    ErrCode result4 = BundlePermissionMgr::GetPermissionDef(permissionName4, permdef3);
    CheckPermissionDef(DEFPERMISSION_SAME1, bundlename1, permdef3);
    EXPECT_EQ(result4, true);
    PermissionDef permdef4;
    std::string permissionName5 = DEFPERMISSION_NAME17;
    ErrCode result5 = BundlePermissionMgr::GetPermissionDef(permissionName5, permdef4);
    CheckPermissionDef(DEFPERMISSION_SAME2, bundlename1, permdef4);
    EXPECT_EQ(result5, true);
    PermissionDef permdef5;
    std::string permissionName6 = DEFPERMISSION_NAME18;
    ErrCode result6 = BundlePermissionMgr::GetPermissionDef(permissionName6, permdef5);
    CheckPermissionDef(DEFPERMISSION_SAME3, bundlename1, permdef5);
    EXPECT_EQ(result6, true);
}

/**
 * @tc.number: BmsBundlePermissionTest
 * Function: PermissionTest
 * @tc.name: parse bundle package by config.json
 * @tc.desc: 1. system running normally
 *           2. test bundle package get error name to get permissiondef
 */
HWTEST_F(BmsBundlePermissionTest, GetPermissionDef_0400, Function | SmallTest | Level0)
{
    InnerBundleInfo innerbundleinfo;
    MockDefPermissionBundleSystemGrant(innerbundleinfo);
    auto bundlename = innerbundleinfo.GetBundleName();
    PermissionDef permdef;
    std::string permissionName = "1";
    ErrCode result = BundlePermissionMgr::GetPermissionDef(permissionName, permdef);
    CheckErrPermissionDef(DEFPERMISSION_SYSTEM1, bundlename, permdef);
    EXPECT_NE(permdef.permissionName, DEFPERMISSION_NAME1);
    EXPECT_NE(permdef.permissionName, DEFPERMISSION_NAME2);
    EXPECT_NE(permdef.permissionName, DEFPERMISSION_NAME3);
    EXPECT_NE(result, true);
}

/**
 * @tc.number: BmsBundlePermissionTest
 * Function: PermissionTest
 * @tc.name: parse bundle package by config.json
 * @tc.desc: 1. system running normally
 *           2. test bundle package  no name to get permissiondef
 */
HWTEST_F(BmsBundlePermissionTest, GetPermissionDef_0500, Function | SmallTest | Level0)
{
    InnerBundleInfo innerbundleinfo;
    MockDefPermissionBundleSystemGrant(innerbundleinfo);
    auto bundlename = innerbundleinfo.GetBundleName();
    PermissionDef permdef;
    std::string permissionName = "";
    ErrCode result = BundlePermissionMgr::GetPermissionDef(permissionName, permdef);
    CheckErrPermissionDef(DEFPERMISSION_SYSTEM1, bundlename, permdef);
    EXPECT_NE(permdef.permissionName, DEFPERMISSION_NAME1);
    EXPECT_NE(permdef.permissionName, DEFPERMISSION_NAME2);
    EXPECT_NE(permdef.permissionName, DEFPERMISSION_NAME3);
    EXPECT_EQ(result, false);
}

/**
 * @tc.number: BmsBundlePermissionTest
 * Function: PermissionTest
 * @tc.name: parse bundle package by config.json
 * @tc.desc: 1. system running normally
 *           2. test bundle package can Request Permission successfully
 */
HWTEST_F(BmsBundlePermissionTest, CanRequestPermission_0100, Function | SmallTest | Level0)
{
    InnerBundleInfo innerbundleinfo;
    MockDefPermissionBundleSystemGrant(innerbundleinfo);
    std::string permissionName = DEFPERMISSION_NAME3;
    int userId = '1';
    ErrCode result = BundlePermissionMgr::CanRequestPermission(innerbundleinfo.GetBundleName(), permissionName, userId);
    EXPECT_EQ(result, 0);
}

/**
 * @tc.number: BmsBundlePermissionTest
 * Function: PermissionTest
 * @tc.name: parse bundle package by config.json
 * @tc.desc: 1. system running normally
 *           2. test bundle package can Request Permission from user for successfully
 */
HWTEST_F(BmsBundlePermissionTest, RequestPermissionFromUser_0100, Function | SmallTest | Level0)
{
    InnerBundleInfo innerbundleinfo;
    MockDefPermissionBundleSystemGrant(innerbundleinfo);
    std::string permissionName = DEFPERMISSION_NAME3;
    int userId = '1';
    ErrCode result =
        BundlePermissionMgr::RequestPermissionFromUser(innerbundleinfo.GetBundleName(), permissionName, userId);
    EXPECT_EQ(result, true);
}
