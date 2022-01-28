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

#include "app_log_wrapper.h"
#include "ability_info.h"
#include "bundle_constants.h"
#include "bundle_info.h"
#include "inner_bundle_info.h"
#include "json_constants.h"
#include "json_serializer.h"
#include "nlohmann/json.hpp"

using namespace testing::ext;
using namespace OHOS::AppExecFwk;
using namespace OHOS::AppExecFwk::JsonConstants;

namespace {
const std::string NORMAL_BUNDLE_NAME{"com.example.test"};
}  // namespace

class BmsBundleDataStorageDatabaseTest : public testing::Test {
public:
    BmsBundleDataStorageDatabaseTest();
    ~BmsBundleDataStorageDatabaseTest();
    static void SetUpTestCase();
    static void TearDownTestCase();
    void SetUp();
    void TearDown();

protected:
    enum class InfoType {
        BUNDLE_INFO,
        APPLICATION_INFO,
        ABILITY_INFO,
    };
    void CheckInvalidPropDeserialize(const nlohmann::json infoJson, const InfoType infoType) const;

protected:
    nlohmann::json innerBundleInfoJson_ = R"(
        {
            "appFeature": "ohos_system_app",
            "appType": 2,
            "baseAbilityInfos": {
                "com.ohos.launchercom.ohos.launchercom.ohos.launcher.MainAbility": {
                    "applicationName": "com.ohos.launcher",
                    "bundleName": "com.ohos.launcher",
                    "codePath": "",
                    "description": "$string:mainability_description",
                    "theme": "mytheme",
                    "deviceCapabilities": [],
                    "deviceId": "",
                    "deviceTypes": [
                        "phone"
                    ],
                    "iconPath": "$media:icon",
                    "enabled": true,
                    "readPermission": "readPermission",
                    "writePermission": "writePermission",
                    "configChanges": [
                        "locale"
                    ],
                    "formEnabled": true,
                    "formEntity": 1,
                    "minFormHeight": 0,
                    "defaultFormHeight": 100,
                    "minFormWidth": 0,
                    "defaultFormWidth": 200,
                    "metaData": {
                        "customizeData": [{
                            "name" : "string",
                            "value" : "string",
                            "extra" : "$string:customizeData_description"
                        }]
                    },
                    "backgroundModes": 1,
                    "isLauncherAbility": true,
                    "isNativeAbility": false,
                    "supportPipMode" : false,
                    "kind": "page",
                    "label": "Launcher",
                    "launchMode": 0,
                    "libPath": "",
                    "moduleName": ".MyApplication",
                    "name": "com.ohos.launcher.MainAbility",
                    "orientation": 0,
                    "package": "com.ohos.launcher",
                    "permissions": [],
                    "process": "",
                    "resourcePath": "/data/accounts/account_0/applications/com.ohos.launcher/com.ohos.launcher/assets/launcher/resources.index",
                    "targetAbility": "",
                    "type": 1,
                    "uri": "",
                    "visible": false,
                    "labelId": 1,
                    "descriptionId": 1,
                    "iconId": 1,
                    "srcLanguage": "C++",
                    "srcPath": ""
                }
            },
            "baseApplicationInfo": {
                "bundleName": "com.ohos.launcher",
                "cacheDir": "/data/accounts/account_0/appdata/com.ohos.launcher/cache",
                "codePath": "/data/accounts/account_0/applications/com.ohos.launcher",
                "dataBaseDir": "/data/accounts/account_0/appdata/com.ohos.launcher/database",
                "dataDir": "/data/accounts/account_0/appdata/com.ohos.launcher/files",
                "description": "$string:mainability_description",
                "descriptionId": 16777217,
                "deviceId": "PHONE-001",
                "enabled": true,
                "removable": true,
                "entryDir": "",
                "flags": 0,
                "iconId": 16777218,
                "iconPath": "$media:icon",
                "isLauncherApp": true,
                "isSystemApp": false,
                "label": "Launcher",
                "labelId": 0,
                "moduleInfos": [],
                "moduleSourceDirs": [],
                "name": "com.ohos.launcher",
                "permissions": [],
                "process": "",
                "signatureKey": "",
                "supportedModes": 0,
                "debug": true,
                "isCloned": true,
                "uid": 10000,
                "signatureKey": ""
            },
            "baseBundleInfo": {
                "abilityInfos": [],
                "appId": "BNtg4JBClbl92Rgc3jm/RfcAdrHXaM8F0QOiwVEhnV5ebE5jNIYnAx+weFRT3QTyUjRNdhmc2aAzWyi+5t5CoBM=",
                "applicationInfo": {
                    "bundleName": "",
                    "cacheDir": "",
                    "codePath": "",
                    "dataBaseDir": "",
                    "dataDir": "",
                    "description": "",
                    "descriptionId": 0,
                    "deviceId": "",
                    "enabled": false,
                    "removable": true,
                    "entryDir": "",
                    "flags": 0,
                    "iconId": 0,
                    "iconPath": "",
                    "isLauncherApp": false,
                    "isSystemApp": false,
                    "label": "",
                    "labelId": 0,
                    "moduleInfos": [],
                    "moduleSourceDirs": [],
                    "name": "",
                    "permissions": [],
                    "process": "",
                    "signatureKey": "",
                    "supportedModes": 0,
                    "debug": true,
                    "isCloned": true,
                    "uid": 10000,
                    "signatureKey": ""
                },
                "compatibleVersion": 3,
                "cpuAbi": "",
                "defPermissions": [],
                "description": "",
                "entryModuleName": "",
                "gid": 10000,
                "hapModuleNames": [],
                "installTime": 17921,
                "isKeepAlive": false,
                "isNativeApp": false,
                "isDifferentName":false,
                "jointUserId": "",
                "label": "Launcher",
                "mainEntry": "",
                "maxSdkVersion": 0,
                "minSdkVersion": 0,
                "moduleDirs": [],
                "moduleNames": [],
                "modulePublicDirs": [],
                "moduleResPaths": [],
                "name": "com.ohos.launcher",
                "releaseType": "Release",
                "reqPermissions": [],
                "seInfo": "",
                "targetVersion": 3,
                "uid": 10000,
                "updateTime": 17921,
                "vendor": "ohos",
                "versionCode": 1,
                "versionName": "1.0",
                "singleUser": true
            },
            "baseDataDir": "/data/accounts/account_0/appdata/com.ohos.launcher",
            "bundleStatus": 1,
            "hasEntry": true,
            "innerModuleInfos": {
                "com.ohos.launcher": {
                    "abilityKeys": [
                        "com.ohos.launchercom.ohos.launchercom.ohos.launcher.MainAbility"
                    ],
                    "defPermissions": [],
                    "description": "",
                    "distro": {
                        "deliveryWithInstall": true,
                        "moduleName": "launcher",
                        "moduleType": "entry"
                    },
                    "isEntry": true,
                    "metaData": {
                        "customizeData": []
                    },
                    "moduleDataDir": "/data/accounts/account_0/appdata/com.ohos.launcher/com.ohos.launcher",
                    "moduleName": ".MyApplication",
                    "modulePackage": "com.ohos.launcher",
                    "modulePath": "/data/accounts/account_0/applications/com.ohos.launcher/com.ohos.launcher",
                    "moduleResPath": "/data/accounts/account_0/applications/com.ohos.launcher/com.ohos.launcher/assets/launcher/resources.index",
                    "reqCapabilities": [],
                    "reqPermissions": [],
                    "colorMode": -1,
                    "skillKeys": [
                        "com.ohos.launchercom.ohos.launchercom.ohos.launcher.MainAbility"
                    ],
                    "srcPath": ""
                }
            },
            "isSupportBackup": false,
            "mainAbility": "com.ohos.launchercom.ohos.launchercom.ohos.launcher.MainAbility",
            "skillInfos": {
                "com.ohos.launchercom.ohos.launchercom.ohos.launcher.MainAbility": [
                    {
                        "actions": [
                            "action.system.home",
                            "com.ohos.action.main"
                        ],
                        "entities": [
                            "entity.system.home",
                            "flag.home.intent.from.system"
                        ],
                        "uris": []
                    }
                ]
            },
            "formInfos": {
                "com.ohos.launchercom.ohos.launchercom.ohos.launcher.MainAbility": [
                    {
                        "package": "com.ohos.launcher",
                        "bundleName": "com.ohos.launcher",
                        "originalBundleName": "com.ohos.launcher",
                        "relatedBundleName": "com.ohos.launcher",
                        "moduleName": "launcher",
                        "abilityName": "com.ohos.launcher.MainAbility",
                        "name": "Form_JS",
                        "description": "It's JS Form",
                        "jsComponentName": "com.ohos.launcher",
                        "deepLink": "com.example.myapplication.fa/.MainAbility",
                        "formConfigAbility": "com.example.myapplication.fa/.MainAbility",
                        "scheduledUpdateTime": "21:05",
                        "descriptionId": 125,
                        "updateDuration": 1,
                        "defaultDimension": 1,
                        "defaultFlag": true,
                        "formVisibleNotify": true,
                        "updateEnabled": true,
                        "type": 0,
                        "colorMode": 0,
                        "supportDimensions": [
                            1
                        ],
                        "landscapeLayouts": [],
                        "portraitLayouts": [],
                        "customizeData": [
                            {
                                "name": "originWidgetName",
                                "value": "com.weather.testWidget"
                            }
                        ]
                    }
                ]
            },
            "shortcutInfos": {
                "com.example.myapplication1com.example.myapplication.h1id": {
                    "bundleName": "com.example.myapplication1",
                    "disableMessage": "",
                    "hostAbility": "",
                    "icon": "$string:mainability_description",
                    "id": "id",
                    "intents": [
                        {
                            "targetBundle": "com.example.myapplication1",
                            "targetClass": "com.example.myapplication.MainAbility"
                        }
                    ],
                    "isEnables": false,
                    "isHomeShortcut": false,
                    "isStatic": false,
                    "label": "$string:mainability_description"
                }
            },
            "commonEvents": {
                "com.example.myapplication1com.example.myapplication.h1id": {
                    "name": ".MainAbility",
                    "bundleName": "com.example.myapplication1",
                    "permission": "permission_test",
                    "uid": -1,
                    "data": [
                        "data_one",
                        "data_two"
                    ],
                    "type": [
                        "type_one",
                        "typea_two"
                    ],
                    "events": [
                        "events_one",
                        "events_two"
                    ]
                }
            },
            "userId_": 0,
            "isPreInstallApp": true,
            "canUninstall": true,
            "newBundleName": "com.example.myapplication1#20010001",
            "innerBundleUserInfos": {
                "com.ohos.launcher_0": {
                    "accessTokenId":-1,
                    "bundleName":"com.ohos.launcher",
                    "bundleUserInfo": {
                        "enabled":true,
                        "abilities":[

                        ],
                        "disabledAbilities":[

                        ],
                        "userId":0
                    },
                    "gids":[
                        10000
                    ],
                    "installTime":17921,
                    "uid":10000,
                    "updateTime":17921
                }
            }
        }
    )"_json;

    nlohmann::json moduleInfoJson_ = R"(
        {
            "moduleName": "entry",
            "moduleSourceDir": ""
        }
    )"_json;
    const std::string deviceId_ = Constants::CURRENT_DEVICE_ID;
    const std::string BASE_ABILITY_INFO = "baseAbilityInfos";
    // need modify with innerBundleInfoJson_
    const std::string abilityName = "com.ohos.launchercom.ohos.launchercom.ohos.launcher.MainAbility";
    const std::string BASE_BUNDLE_INFO = "baseBundleInfo";
    const std::string BASE_APPLICATION_INFO = "baseApplicationInfo";
};

BmsBundleDataStorageDatabaseTest::BmsBundleDataStorageDatabaseTest()
{}

BmsBundleDataStorageDatabaseTest::~BmsBundleDataStorageDatabaseTest()
{}

void BmsBundleDataStorageDatabaseTest::CheckInvalidPropDeserialize(
    const nlohmann::json infoJson, const InfoType infoType) const
{
    APP_LOGI("deserialize infoJson = %{public}s", infoJson.dump().c_str());
    nlohmann::json innerBundleInfoJson;
    nlohmann::json bundleInfoJson = innerBundleInfoJson_.at(BASE_BUNDLE_INFO);

    switch (infoType) {
        case InfoType::BUNDLE_INFO: {
            bundleInfoJson = infoJson;
            BundleInfo bundleInfo = infoJson;
            break;
        }
        case InfoType::APPLICATION_INFO: {
            bundleInfoJson["appInfo"] = infoJson;
            ApplicationInfo applicationInfo = infoJson;
            break;
        }
        case InfoType::ABILITY_INFO: {
            bundleInfoJson["abilityInfos"].push_back(infoJson);
            AbilityInfo abilityInfo = infoJson;
            break;
        }
        default:
            break;
    }

    innerBundleInfoJson["baseBundleInfo"] = bundleInfoJson;
    InnerBundleInfo fromJsonInfo;
    EXPECT_FALSE(fromJsonInfo.FromJson(innerBundleInfoJson));
}

void BmsBundleDataStorageDatabaseTest::SetUpTestCase()
{}

void BmsBundleDataStorageDatabaseTest::TearDownTestCase()
{}

void BmsBundleDataStorageDatabaseTest::SetUp()
{}

void BmsBundleDataStorageDatabaseTest::TearDown()
{}

/**
 * @tc.number: BundleInfoJsonSerializer_0100
 * @tc.name: save bundle installation information to persist storage
 * @tc.desc: 1.system running normally
 *           2.successfully serialize and deserialize all right props in BundleInfo
 * @tc.require: AR000GJUIR
 */
HWTEST_F(BmsBundleDataStorageDatabaseTest, BundleInfoJsonSerializer_0100, Function | SmallTest | Level1)
{
    nlohmann::json sourceInfoJson = innerBundleInfoJson_.at(BASE_BUNDLE_INFO);
    // deserialize BundleInfo from json
    BundleInfo fromJsonInfo = sourceInfoJson;
    // serialize fromJsonInfo to json
    nlohmann::json toJsonObject = fromJsonInfo;

    EXPECT_TRUE(toJsonObject.dump() == sourceInfoJson.dump());
}

/**
 * @tc.number: BundleInfoJsonSerializer_0200
 * @tc.name: save bundle installation information to persist storage
 * @tc.desc: 1.system running normally
 *           2.test can catch deserialize error for type error for name prop in BundleInfo
 * @tc.require: AR000GJUIR
 */
HWTEST_F(BmsBundleDataStorageDatabaseTest, BundleInfoJsonSerializer_0200, Function | SmallTest | Level1)
{
    nlohmann::json typeErrorProps;
    typeErrorProps["name"] = NOT_STRING_TYPE;
    typeErrorProps["label"] = NOT_STRING_TYPE;
    typeErrorProps["description"] = NOT_STRING_TYPE;
    typeErrorProps["vendor"] = NOT_STRING_TYPE;
    typeErrorProps["mainEntry"] = NOT_STRING_TYPE;
    typeErrorProps["versionName"] = NOT_STRING_TYPE;
    typeErrorProps["versionCode"] = NOT_NUMBER_TYPE;
    typeErrorProps["minSdkVersion"] = NOT_NUMBER_TYPE;
    typeErrorProps["minSdkVersion"] = NOT_NUMBER_TYPE;

    for (nlohmann::json::iterator iter = typeErrorProps.begin(); iter != typeErrorProps.end(); iter++) {
        for (auto valueIter = iter.value().begin(); valueIter != iter.value().end(); valueIter++) {
            nlohmann::json infoJson = innerBundleInfoJson_.at(BASE_BUNDLE_INFO);
            infoJson[iter.key()] = valueIter.value();
        }
    }
}

/**
 * @tc.number: AbilityInfoJsonSerializer_0100
 * @tc.name: save bundle installation information to persist storage
 * @tc.desc: 1.system running normally
 *           2.successfully serialize and deserialize all right props in AbilityInfo
 * @tc.require: AR000GJUIR
 */
HWTEST_F(BmsBundleDataStorageDatabaseTest, AbilityInfoJsonSerializer_0100, Function | SmallTest | Level1)
{
    nlohmann::json sourceInfoJson = innerBundleInfoJson_.at(BASE_ABILITY_INFO).at(abilityName);
    // deserialize AbilityInfo from json
    AbilityInfo fromJsonInfo = sourceInfoJson;
    // serialize fromJsonInfo to json
    nlohmann::json toJsonObject = fromJsonInfo;
    EXPECT_TRUE(toJsonObject.dump() == sourceInfoJson.dump());
}

/**
 * @tc.number: AbilityInfoJsonSerializer_0200
 * @tc.name: save bundle installation information to persist storage
 * @tc.desc: 1.system running normally
 *           2.test can catch deserialize error for type error for name prop in AbilityInfo
 * @tc.require: AR000GJUIR
 */
HWTEST_F(BmsBundleDataStorageDatabaseTest, AbilityInfoJsonSerializer_0200, Function | SmallTest | Level1)
{
    nlohmann::json typeErrorProps;
    typeErrorProps["package"] = NOT_STRING_TYPE;
    typeErrorProps["name"] = NOT_STRING_TYPE;
    typeErrorProps["bundleName"] = NOT_STRING_TYPE;
    typeErrorProps["applicationName"] = NOT_STRING_TYPE;
    typeErrorProps["label"] = NOT_STRING_TYPE;
    typeErrorProps["description"] = NOT_STRING_TYPE;
    typeErrorProps["iconPath"] = NOT_STRING_TYPE;
    typeErrorProps["visible"] = NOT_BOOL_TYPE;
    typeErrorProps["kind"] = NOT_STRING_TYPE;
    typeErrorProps["type"] = NOT_NUMBER_TYPE;
    typeErrorProps["orientation"] = NOT_NUMBER_TYPE;
    typeErrorProps["launchMode"] = NOT_NUMBER_TYPE;
    typeErrorProps["codePath"] = NOT_STRING_TYPE;
    typeErrorProps["resourcePath"] = NOT_STRING_TYPE;
    typeErrorProps["libPath"] = NOT_STRING_TYPE;

    for (nlohmann::json::iterator iter = typeErrorProps.begin(); iter != typeErrorProps.end(); iter++) {
        for (auto valueIter = iter.value().begin(); valueIter != iter.value().end(); valueIter++) {
            APP_LOGD("deserialize check prop key = %{public}s, type = %{public}s",
                iter.key().c_str(),
                valueIter.key().c_str());
            nlohmann::json infoJson = innerBundleInfoJson_.at(BASE_ABILITY_INFO).at(abilityName);
            infoJson[iter.key()] = valueIter.value();
        }
    }
}

/**
 * @tc.number: ApplicationInfoJsonSerializer_0100
 * @tc.name: save bundle installation information to persist storage
 * @tc.desc: 1.system running normally
 *           2.successfully serialize and deserialize all right props in ApplicationInfo
 * @tc.require: AR000GJUIR
 */
HWTEST_F(BmsBundleDataStorageDatabaseTest, ApplicationInfoJsonSerializer_0100, Function | SmallTest | Level1)
{
    nlohmann::json sourceInfoJson = innerBundleInfoJson_.at(BASE_APPLICATION_INFO);
    // deserialize ApplicationInfo from json
    ApplicationInfo fromJsonInfo = sourceInfoJson;
    // serialize fromJsonInfo to json
    nlohmann::json toJsonObject = fromJsonInfo;

    EXPECT_TRUE(toJsonObject.dump() == sourceInfoJson.dump());
}

/**
 * @tc.number: ApplicationInfoJsonSerializer_0200
 * @tc.name: save bundle installation information to persist storage
 * @tc.desc: 1.system running normally
 *           2.test can catch deserialize error for type error for name prop in ApplicationInfo
 * @tc.require: AR000GJUIR
 */
HWTEST_F(BmsBundleDataStorageDatabaseTest, ApplicationInfoJsonSerializer_0200, Function | SmallTest | Level1)
{
    nlohmann::json typeErrorProps;
    typeErrorProps["name"] = NOT_STRING_TYPE;
    typeErrorProps["bundleName"] = NOT_STRING_TYPE;
    typeErrorProps["sandboxId"] = NOT_NUMBER_TYPE;
    typeErrorProps["signatureKey"] = NOT_STRING_TYPE;

    for (nlohmann::json::iterator iter = typeErrorProps.begin(); iter != typeErrorProps.end(); iter++) {
        for (auto valueIter = iter.value().begin(); valueIter != iter.value().end(); valueIter++) {
            APP_LOGD("deserialize check prop key = %{public}s, type = %{public}s",
                iter.key().c_str(),
                valueIter.key().c_str());
            nlohmann::json infoJson = innerBundleInfoJson_.at(BASE_APPLICATION_INFO);
            infoJson[iter.key()] = valueIter.value();
        }
    }
}

/**
 * @tc.number: ModuleInfoJsonSerializer_0100
 * @tc.name: save bundle installation information to persist storage
 * @tc.desc: 1.system running normally
 *           2.successfully serialize and deserialize all right props in ModuleInfo
 * @tc.require: AR000GJUIR
 */
HWTEST_F(BmsBundleDataStorageDatabaseTest, ModuleInfoJsonSerializer_0100, Function | SmallTest | Level1)
{
    nlohmann::json sourceInfoJson = moduleInfoJson_;
    // deserialize ModuleInfo from json
    ModuleInfo fromJsonInfo = sourceInfoJson;
    // serialize fromJsonInfo to json
    nlohmann::json toJsonObject = fromJsonInfo;

    EXPECT_TRUE(toJsonObject.dump() == sourceInfoJson.dump());
}

/**
 * @tc.number: SaveData_0100
 * @tc.name: save bundle installation information to persist storage
 * @tc.desc: 1.system running normally and no saved any bundle data
 *           2.successfully save a new bundle installation information for the first time
 * @tc.require: AR000GJUIR
 */
HWTEST_F(BmsBundleDataStorageDatabaseTest, SaveData_0100, Function | SmallTest | Level0)
{
    InnerBundleInfo innerBundleInfo;
    EXPECT_EQ(innerBundleInfo.FromJson(innerBundleInfoJson_), OHOS::ERR_OK);
}