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

#ifndef FOUNDATION_APPEXECFWK_INTERFACES_INNERKITS_APPEXECFWK_BASE_INCLUDE_ABILITY_INFO_H
#define FOUNDATION_APPEXECFWK_INTERFACES_INNERKITS_APPEXECFWK_BASE_INCLUDE_ABILITY_INFO_H

#include <string>

#include "parcel.h"
#include "application_info.h"

namespace OHOS {
namespace AppExecFwk {

enum class AbilityType {
    UNKNOWN = 0,
    PAGE,
    SERVICE,
    DATA,
};

enum class AbilitySubType {
    UNSPECIFIED = 0,
    CA,
};

enum class DisplayOrientation {
    UNSPECIFIED = 0,
    LANDSCAPE,
    PORTRAIT,
    FOLLOWRECENT,
};

enum class LaunchMode {
    SINGLETON = 0,
    STANDARD,  // support more than one instance
    SINGLETOP,
};

struct Parameters {
    std::string description;
    std::string name;
    std::string type;
};

struct Results {
    std::string description;
    std::string name;
    std::string type;
};

struct CustomizeData {
    std::string name;
    std::string value;
    std::string extra;
};

struct MetaData {
    std::vector<Parameters> parameters;
    std::vector<Results> results;
	std::vector<CustomizeData> customizeData;
};

struct AbilityInfo;

/*
* According to Ability profile 1.0
*/
struct CompatibleAbilityInfo : public Parcelable {
    // deprecated: ability code class simple name, use 'className' instead.
    std::string package;
    std::string name;
    std::string label; // display name on screen.
    std::string description;
    std::string iconPath; // used as icon data (base64) for WEB Ability.
    std::string uri; // uri of ability.
    std::string moduleName; // indicates the name of the .hap package to which the capability belongs.
    std::string process;
    std::string targetAbility;
    std::string appName;
    std::string privacyUrl;
    std::string privacyName;
    std::string downloadUrl;
    std::string versionName;
    uint32_t backgroundModes = 0;
    uint32_t packageSize = 0; // The size of the package that AbilityInfo.uri points to.
    bool visible = false;
    bool formEnabled = false;
    bool multiUserShared = false;
    // deprecated: remove this field in new package format.
    AbilityType type = AbilityType::UNKNOWN;
    AbilitySubType subType = AbilitySubType::UNSPECIFIED;
    DisplayOrientation orientation = DisplayOrientation::UNSPECIFIED;
    LaunchMode launchMode = LaunchMode::STANDARD;
    std::vector<std::string> permissions;
    std::vector<std::string> deviceTypes;
    std::vector<std::string> deviceCapabilities;
    bool supportPipMode = false;
    bool grantPermission = false;
    std::string readPermission;
    std::string writePermission;
    std::string uriPermissionMode;
    std::string uriPermissionPath;
    bool directLaunch = true;

    // set when install
    std::string bundleName; // bundle name which has this ability.
    std::string className;  // the ability full class name.
    std::string originalClassName; // the original ability full class name
    std::string deviceId; // device UDID information.
    CompatibleApplicationInfo applicationInfo;

    // form widget info
    uint32_t formEntity = 1; // where form can be displayed
    int32_t minFormHeight = 0; // minimum height of ability.
    int32_t defaultFormHeight = 0; // default height of ability.
    int32_t minFormWidth = 0; // minimum width of ability.
    int32_t defaultFormWidth = 0; // default width of ability.

    uint32_t iconId = 0;
    uint32_t labelId = 0;
    uint32_t descriptionId = 0;
    bool enabled = true;

    bool ReadFromParcel(Parcel& parcel);
    virtual bool Marshalling(Parcel& parcel) const override;
    static CompatibleAbilityInfo* Unmarshalling(Parcel& parcel);

    void CopyToDest(CompatibleAbilityInfo& dest) const;
    void ConvertToAbilityInfo(AbilityInfo& abilityInfo) const;
};

// configuration information about an ability
struct AbilityInfo : public Parcelable {
    std::string name;  // ability name, only the main class name
    std::string label;
    std::string description;
    std::string iconPath;
    std::string theme;
    bool visible = false;
    std::string kind;  // ability category
    AbilityType type = AbilityType::UNKNOWN;
    DisplayOrientation orientation = DisplayOrientation::UNSPECIFIED;
    LaunchMode launchMode = LaunchMode::STANDARD;
    std::vector<std::string> permissions;

    std::string process;
    std::vector<std::string> deviceTypes;
    std::vector<std::string> deviceCapabilities;
    std::string uri;
    std::string targetAbility;
    ApplicationInfo applicationInfo;
    bool isLauncherAbility = false;
    bool isNativeAbility = false;
    bool enabled = false;
    std::string readPermission;
    std::string writePermission;
    uint32_t formEntity = 1;
    int32_t minFormHeight = 0;
    int32_t defaultFormHeight = 0;
    int32_t minFormWidth = 0;
    int32_t defaultFormWidth = 0;
    MetaData metaData;

    // set when install
    std::string package;  // the "module.package" in config.json
    std::string bundleName;
    std::string moduleName;       // the "module.name" in config.json
    std::string applicationName;  // the "bundlename" in config.json
    std::string deviceId;         // should auto-get self device id
    std::string codePath;         // ability main code path with name
    std::string resourcePath;     // resource path for resource init
    std::string libPath;          // ability library path without name, libPath->libDir

    bool ReadFromParcel(Parcel &parcel);
    virtual bool Marshalling(Parcel &parcel) const override;
    static AbilityInfo *Unmarshalling(Parcel &parcel);
    void Dump(std::string prefix, int fd);
    void ConvertToCompatiableAbilityInfo(CompatibleAbilityInfo& compatibleAbilityInfo) const;
};

}  // namespace AppExecFwk
}  // namespace OHOS
#endif  // FOUNDATION_APPEXECFWK_INTERFACES_INNERKITS_APPEXECFWK_BASE_INCLUDE_ABILITY_INFO_H
