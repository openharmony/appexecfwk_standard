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

#include "inner_bundle_info.h"

#include <regex>

#include "common_profile.h"

namespace OHOS {
namespace AppExecFwk {
namespace {

const std::string IS_SUPPORT_BACKUP = "isSupportBackup";
const std::string APP_TYPE = "appType";
const std::string UID = "uid";
const std::string GID = "gid";
const std::string BASE_DATA_DIR = "baseDataDir";
const std::string BUNDLE_STATUS = "bundleStatus";
const std::string BASE_APPLICATION_INFO = "baseApplicationInfo";
const std::string BASE_BUNDLE_INFO = "baseBundleInfo";
const std::string BASE_ABILITY_INFO = "baseAbilityInfos";
const std::string INNER_MODULE_INFO = "innerModuleInfos";
const std::string MAIN_ABILITY = "mainAbility";
const std::string MAIN_ABILITY_NAME = "mainAbilityName";
const std::string SKILL_INFOS = "skillInfos";
const std::string USER_ID = "userId_";
const std::string APP_FEATURE = "appFeature";
const std::string HAS_ENTRY = "hasEntry";
const std::string CAN_UNINSTALL = "canUninstall";
const std::string MODULE_PACKAGE = "modulePackage";
const std::string MODULE_PATH = "modulePath";
const std::string MODULE_NAME = "moduleName";
const std::string MODULE_DESCRIPTION = "description";
const std::string MODULE_DESCRIPTION_ID = "descriptionId";
const std::string MODULE_LABEL = "label";
const std::string MODULE_LABEL_ID = "labelId";
const std::string MODULE_DESCRIPTION_INSTALLATION_FREE = "installationFree";
const std::string MODULE_IS_ENTRY = "isEntry";
const std::string MODULE_METADATA = "metaData";
const std::string MODULE_COLOR_MODE = "colorMode";
const std::string MODULE_DISTRO = "distro";
const std::string MODULE_REQ_CAPABILITIES = "reqCapabilities";
const std::string MODULE_REQ_PERMS = "reqPermissions";
const std::string MODULE_DEF_PERMS = "defPermissions";
const std::string MODULE_DATA_DIR = "moduleDataDir";
const std::string MODULE_RES_PATH = "moduleResPath";
const std::string MODULE_ABILITY_KEYS = "abilityKeys";
const std::string MODULE_SKILL_KEYS = "skillKeys";
const std::string MODULE_FORMS = "formInfos";
const std::string MODULE_SHORTCUT = "shortcutInfos";
const std::string MODULE_COMMON_EVENT = "commonEvents";
const std::string MODULE_MAIN_ABILITY = "mainAbility";
const std::string NEW_BUNDLE_NAME = "newBundleName";
const std::string MODULE_SRC_PATH = "srcPath";
const std::string SCHEME_SEPARATOR = "://";
const std::string PORT_SEPARATOR = ":";
const std::string PATH_SEPARATOR = "/";
const std::string IS_PREINSTALL_APP = "isPreInstallApp";
const std::string INSTALL_MARK = "installMark";
const char WILDCARD = '*';
const std::string TYPE_WILDCARD = "*/*";
const std::string INNER_BUNDLE_USER_INFOS = "innerBundleUserInfos";

const std::string NameAndUserIdToKey(const std::string &bundleName, int32_t userId)
{
    return bundleName + Constants::FILE_UNDERLINE + std::to_string(userId);
}
}  // namespace

bool Skill::Match(const OHOS::AAFwk::Want &want) const
{
    bool matchAction = MatchAction(want.GetAction());
    if (!matchAction) {
        APP_LOGD("Action does not match");
        return false;
    }
    bool matchEntities = MatchEntities(want.GetEntities());
    if (!matchEntities) {
        APP_LOGD("Entities does not match");
        return false;
    }
    bool matchUriAndType = MatchUriAndType(want.GetUriString(), want.GetType());
    if (!matchUriAndType) {
        APP_LOGD("Uri or Type does not match");
        return false;
    }
    return true;
}

bool Skill::MatchLauncher(const OHOS::AAFwk::Want &want) const
{
    bool matchAction = MatchAction(want.GetAction());
    if (!matchAction) {
        APP_LOGD("Action does not match");
        return false;
    }
    bool matchEntities = MatchEntities(want.GetEntities());
    if (!matchEntities) {
        APP_LOGD("Entities does not match");
        return false;
    }
    return true;
}

bool Skill::MatchAction(const std::string &action) const
{
    // config actions empty, no match
    if (actions.empty()) {
        return false;
    }
    // config actions not empty, param empty, match
    if (action.empty()) {
        return true;
    }
    // config actions not empty, param not empty, if config actions contains param action, match
    return std::find(actions.cbegin(), actions.cend(), action) != actions.cend();
}

bool Skill::MatchEntities(const std::vector<std::string> &paramEntities) const
{
    // param entities empty, match
    if (paramEntities.empty()) {
        return true;
    }
    // config entities empty, param entities not empty, not match
    if (entities.empty()) {
        return false;
    }
    // config entities not empty, param entities not empty, if every param entity in config entities, match
    std::vector<std::string>::size_type size = paramEntities.size();
    for (std::vector<std::string>::size_type i = 0; i < size; i++) {
        bool ret = std::find(entities.cbegin(), entities.cend(), paramEntities[i]) == entities.cend();
        if (ret) {
            return false;
        }
    }
    return true;
}

bool Skill::MatchUriAndType(const std::string &uriString, const std::string &type) const
{
    if (uriString.empty() && type.empty()) {
        // case1 : param uri empty, param type empty
        if (uris.empty()) {
            return true;
        }
        for (const SkillUri &skillUri : uris) {
            if (skillUri.scheme.empty() && skillUri.type.empty()) {
                return true;
            }
        }
        return false;
    }
    if (uris.empty()) {
        return false;
    }
    if (!uriString.empty() && type.empty()) {
        // case2 : param uri not empty, param type empty
        for (const SkillUri &skillUri : uris) {
            if (MatchUri(uriString, skillUri) && skillUri.type.empty()) {
                return true;
            }
        }
        return false;
    } else if (uriString.empty() && !type.empty()) {
        // case3 : param uri empty, param type not empty
        for (const SkillUri &skillUri : uris) {
            if (skillUri.scheme.empty() && MatchType(type, skillUri.type)) {
                return true;
            }
        }
        return false;
    } else {
        // case4 : param uri not empty, param type not empty
        for (const SkillUri &skillUri : uris) {
            if (MatchUri(uriString, skillUri) && MatchType(type, skillUri.type)) {
                return true;
            }
        }
        return false;
    }
}

bool Skill::MatchUri(const std::string &uriString, const SkillUri &skillUri) const
{
    if (skillUri.scheme.empty()) {
        return uriString.empty();
    }
    if (skillUri.host.empty()) {
        return uriString == skillUri.scheme;
    }
    std::string skillUriString;
    skillUriString.append(skillUri.scheme).append(SCHEME_SEPARATOR).append(skillUri.host);
    if (!skillUri.port.empty()) {
        skillUriString.append(PORT_SEPARATOR).append(skillUri.port);
    }
    if (skillUri.path.empty() && skillUri.pathStartWith.empty() && skillUri.pathRegx.empty()) {
        return uriString == skillUriString;
    }
    skillUriString.append(PATH_SEPARATOR);
    // if one of path, pathStartWith, pathRegex match, then match
    if (!skillUri.path.empty()) {
        // path match
        std::string pathUri(skillUriString);
        pathUri.append(skillUri.path);
        if (uriString == pathUri) {
            return true;
        }
    }
    if (!skillUri.pathStartWith.empty()) {
        // pathStartWith match
        std::string pathStartWithUri(skillUriString);
        pathStartWithUri.append(skillUri.pathStartWith);
        if (uriString.find(pathStartWithUri) == 0) {
            return true;
        }
    }
    if (!skillUri.pathRegx.empty()) {
        // pathRegex match
        std::string pathRegxUri(skillUriString);
        pathRegxUri.append(skillUri.pathRegx);
        try {
            std::regex regex(pathRegxUri);
            if (regex_match(uriString, regex)) {
                return true;
            }
        } catch(...) {
            APP_LOGE("regex error");
        }
    }
    return false;
}

bool Skill::MatchType(const std::string &type, const std::string &skillUriType) const
{
    // type is not empty
    if (skillUriType.empty()) {
        return false;
    }
    if (type == TYPE_WILDCARD || skillUriType == TYPE_WILDCARD) {
        // param is */* or config is */*
        return true;
    }
    bool paramTypeRegex = type.back() == WILDCARD;
    if (paramTypeRegex) {
        // param is string/*
        std::string prefix = type.substr(0, type.length() - 1);
        return skillUriType.find(prefix) == 0;
    }
    bool typeRegex = skillUriType.back() == WILDCARD;
    if (typeRegex) {
        // config is string/*
        std::string prefix = skillUriType.substr(0, skillUriType.length() - 1);
        return type.find(prefix) == 0;
    } else {
        return type == skillUriType;
    }
}

InnerBundleInfo::InnerBundleInfo()
{
    APP_LOGD("inner bundle info instance is created");
}

InnerBundleInfo::~InnerBundleInfo()
{
    APP_LOGD("inner bundle info instance is destroyed");
}

void to_json(nlohmann::json &jsonObject, const Distro &distro)
{
    jsonObject = nlohmann::json {
            {ProfileReader::BUNDLE_MODULE_PROFILE_KEY_DELIVERY_WITH_INSTALL, distro.deliveryWithInstall},
            {ProfileReader::BUNDLE_MODULE_PROFILE_KEY_MODULE_NAME, distro.moduleName},
            {ProfileReader::BUNDLE_MODULE_PROFILE_KEY_MODULE_TYPE, distro.moduleType},
            {ProfileReader::BUNDLE_MODULE_PROFILE_KEY_MODULE_INSTALLATION_FREE, distro.installationFree}
    };
}

void to_json(nlohmann::json &jsonObject, const UsedScene &usedScene)
{
    jsonObject = nlohmann::json {
        {ProfileReader::BUNDLE_MODULE_PROFILE_KEY_REQ_PERMISSIONS_ABILITY, usedScene.ability},
        {ProfileReader::BUNDLE_MODULE_PROFILE_KEY_REQ_PERMISSIONS_WHEN, usedScene.when}
    };
}

void to_json(nlohmann::json &jsonObject, const ReqPermission &reqPermission)
{
    jsonObject = nlohmann::json {
        {ProfileReader::BUNDLE_MODULE_PROFILE_KEY_REQ_PERMISSIONS_NAME, reqPermission.name},
        {ProfileReader::BUNDLE_MODULE_PROFILE_KEY_REQ_PERMISSIONS_REASON, reqPermission.reason},
        {ProfileReader::BUNDLE_MODULE_PROFILE_KEY_REQ_PERMISSIONS_USEDSCENE, reqPermission.usedScene}
    };
}

void to_json(nlohmann::json &jsonObject, const DefPermission &defPermission)
{
    jsonObject = nlohmann::json {
        {ProfileReader::BUNDLE_MODULE_PROFILE_KEY_DEF_PERMISSIONS_NAME, defPermission.name},
        {ProfileReader::BUNDLE_MODULE_PROFILE_KEY_DEF_PERMISSIONS_GRANTMODE, defPermission.grantMode},
        {ProfileReader::BUNDLE_MODULE_PROFILE_KEY_DEF_PERMISSIONS_AVAILABLESCOPE, defPermission.availableScope},
        {ProfileReader::BUNDLE_MODULE_PROFILE_KEY_DEF_PERMISSIONS_LABEL, defPermission.label},
        {ProfileReader::BUNDLE_MODULE_PROFILE_KEY_DEF_PERMISSIONS_LABEL_ID, defPermission.labelId},
        {ProfileReader::BUNDLE_MODULE_PROFILE_KEY_DEF_PERMISSIONS_DESCRIPTION, defPermission.description},
        {ProfileReader::BUNDLE_MODULE_PROFILE_KEY_DEF_PERMISSIONS_DESCRIPTION_ID, defPermission.descriptionId}
    };
}

void to_json(nlohmann::json &jsonObject, const InnerModuleInfo &info)
{
    jsonObject = nlohmann::json {
        {MODULE_PACKAGE, info.modulePackage},
        {MODULE_NAME, info.moduleName},
        {MODULE_PATH, info.modulePath},
        {MODULE_DATA_DIR, info.moduleDataDir},
        {MODULE_RES_PATH, info.moduleResPath},
        {MODULE_IS_ENTRY, info.isEntry},
        {MODULE_METADATA, info.metaData},
        {MODULE_COLOR_MODE, info.colorMode},
        {MODULE_DISTRO, info.distro},
        {MODULE_DESCRIPTION, info.description},
        {MODULE_DESCRIPTION_ID, info.descriptionId},
        {MODULE_LABEL, info.label},
        {MODULE_LABEL_ID, info.labelId},
        {MODULE_DESCRIPTION_INSTALLATION_FREE, info.installationFree},
        {MODULE_REQ_CAPABILITIES, info.reqCapabilities},
        {MODULE_REQ_PERMS, info.reqPermissions},
        {MODULE_DEF_PERMS, info.defPermissions},
        {MODULE_ABILITY_KEYS, info.abilityKeys},
        {MODULE_SKILL_KEYS, info.skillKeys},
        {MODULE_MAIN_ABILITY, info.mainAbility},
        {MODULE_SRC_PATH, info.srcPath}
    };
}

void to_json(nlohmann::json &jsonObject, const SkillUri &uri)
{
    jsonObject = nlohmann::json {
        {ProfileReader::BUNDLE_MODULE_PROFILE_KEY_SCHEME, uri.scheme},
        {ProfileReader::BUNDLE_MODULE_PROFILE_KEY_HOST, uri.host},
        {ProfileReader::BUNDLE_MODULE_PROFILE_KEY_PORT, uri.port},
        {ProfileReader::BUNDLE_MODULE_PROFILE_KEY_PATH, uri.path},
        {ProfileReader::BUNDLE_MODULE_PROFILE_KEY_PATHSTARTWITH, uri.pathStartWith},
        {ProfileReader::BUNDLE_MODULE_PROFILE_KEY_PATHREGX, uri.pathRegx},
        {ProfileReader::BUNDLE_MODULE_PROFILE_KEY_TYPE, uri.type}
    };
}

void to_json(nlohmann::json &jsonObject, const Skill &skill)
{
    jsonObject = nlohmann::json {
        {ProfileReader::BUNDLE_MODULE_PROFILE_KEY_ACTIONS, skill.actions},
        {ProfileReader::BUNDLE_MODULE_PROFILE_KEY_ENTITIES, skill.entities},
        {ProfileReader::BUNDLE_MODULE_PROFILE_KEY_URIS, skill.uris}
    };
}

void to_json(nlohmann::json &jsonObject, const InstallMark &installMark)
{
    jsonObject = nlohmann::json {
        {ProfileReader::BUNDLE_INSTALL_MARK_BUNDLE, installMark.bundleName},
        {ProfileReader::BUNDLE_INSTALL_MARK_PACKAGE, installMark.packageName},
        {ProfileReader::BUNDLE_INSTALL_MARK_STATUS, installMark.status}
    };
}

void InnerBundleInfo::ToJson(nlohmann::json &jsonObject) const
{
    jsonObject[IS_SUPPORT_BACKUP] = isSupportBackup_;
    jsonObject[APP_TYPE] = appType_;
    jsonObject[BASE_DATA_DIR] = baseDataDir_;
    jsonObject[BUNDLE_STATUS] = bundleStatus_;
    jsonObject[BASE_APPLICATION_INFO] = baseApplicationInfo_;
    jsonObject[BASE_BUNDLE_INFO] = baseBundleInfo_;
    jsonObject[BASE_ABILITY_INFO] = baseAbilityInfos_;
    jsonObject[INNER_MODULE_INFO] = innerModuleInfos_;
    jsonObject[SKILL_INFOS] = skillInfos_;
    jsonObject[USER_ID] = userId_;
    jsonObject[MAIN_ABILITY] = mainAbility_;
    jsonObject[MAIN_ABILITY_NAME] = mainAbilityName_;
    jsonObject[APP_FEATURE] = appFeature_;
    jsonObject[HAS_ENTRY] = hasEntry_;
    jsonObject[MODULE_FORMS] = formInfos_;
    jsonObject[MODULE_SHORTCUT] = shortcutInfos_;
    jsonObject[NEW_BUNDLE_NAME] = newBundleName_;
    jsonObject[MODULE_COMMON_EVENT] = commonEvents_;
    jsonObject[CAN_UNINSTALL] = canUninstall_;
    jsonObject[IS_PREINSTALL_APP] = isPreInstallApp_;
    jsonObject[INSTALL_MARK] = mark_;
    jsonObject[INNER_BUNDLE_USER_INFOS] = innerBundleUserInfos_;
}

void from_json(const nlohmann::json &jsonObject, InnerModuleInfo &info)
{
    // these are not required fields.
    const auto &jsonObjectEnd = jsonObject.end();
    GetValueIfFindKey<std::string>(jsonObject,
        jsonObjectEnd,
        MODULE_PACKAGE,
        info.modulePackage,
        JsonType::STRING,
        false,
        ProfileReader::parseResult,
        ArrayType::NOT_ARRAY);
    GetValueIfFindKey<std::string>(jsonObject,
        jsonObjectEnd,
        MODULE_NAME,
        info.moduleName,
        JsonType::STRING,
        false,
        ProfileReader::parseResult,
        ArrayType::NOT_ARRAY);
    GetValueIfFindKey<std::string>(jsonObject,
        jsonObjectEnd,
        MODULE_PATH,
        info.modulePath,
        JsonType::STRING,
        false,
        ProfileReader::parseResult,
        ArrayType::NOT_ARRAY);
    GetValueIfFindKey<std::string>(jsonObject,
        jsonObjectEnd,
        MODULE_DATA_DIR,
        info.moduleDataDir,
        JsonType::STRING,
        false,
        ProfileReader::parseResult,
        ArrayType::NOT_ARRAY);
    GetValueIfFindKey<std::string>(jsonObject,
        jsonObjectEnd,
        MODULE_RES_PATH,
        info.moduleResPath,
        JsonType::STRING,
        false,
        ProfileReader::parseResult,
        ArrayType::NOT_ARRAY);
    GetValueIfFindKey<bool>(jsonObject,
        jsonObjectEnd,
        MODULE_IS_ENTRY,
        info.isEntry,
        JsonType::BOOLEAN,
        false,
        ProfileReader::parseResult,
        ArrayType::NOT_ARRAY);
    GetValueIfFindKey<MetaData>(jsonObject,
        jsonObjectEnd,
        MODULE_METADATA,
        info.metaData,
        JsonType::OBJECT,
        false,
        ProfileReader::parseResult,
        ArrayType::NOT_ARRAY);
    GetValueIfFindKey<ModuleColorMode>(jsonObject,
        jsonObjectEnd,
        MODULE_COLOR_MODE,
        info.colorMode,
        JsonType::NUMBER,
        false,
        ProfileReader::parseResult,
        ArrayType::NOT_ARRAY);
    GetValueIfFindKey<Distro>(jsonObject,
        jsonObjectEnd,
        MODULE_DISTRO,
        info.distro,
        JsonType::OBJECT,
        false,
        ProfileReader::parseResult,
        ArrayType::NOT_ARRAY);
    GetValueIfFindKey<std::string>(jsonObject,
        jsonObjectEnd,
        MODULE_DESCRIPTION,
        info.description,
        JsonType::STRING,
        false,
        ProfileReader::parseResult,
        ArrayType::NOT_ARRAY);
    GetValueIfFindKey<int32_t>(jsonObject,
        jsonObjectEnd,
        MODULE_DESCRIPTION_ID,
        info.descriptionId,
        JsonType::NUMBER,
        false,
        ProfileReader::parseResult,
        ArrayType::NOT_ARRAY);
    GetValueIfFindKey<std::string>(jsonObject,
        jsonObjectEnd,
        MODULE_LABEL,
        info.label,
        JsonType::STRING,
        false,
        ProfileReader::parseResult,
        ArrayType::NOT_ARRAY);
    GetValueIfFindKey<int32_t>(jsonObject,
        jsonObjectEnd,
        MODULE_LABEL_ID,
        info.labelId,
        JsonType::NUMBER,
        false,
        ProfileReader::parseResult,
        ArrayType::NOT_ARRAY);
    GetValueIfFindKey<std::string>(jsonObject,
        jsonObjectEnd,
        MODULE_MAIN_ABILITY,
        info.mainAbility,
        JsonType::STRING,
        false,
        ProfileReader::parseResult,
        ArrayType::NOT_ARRAY);
    GetValueIfFindKey<std::string>(jsonObject,
        jsonObjectEnd,
        MODULE_SRC_PATH,
        info.srcPath,
        JsonType::STRING,
        false,
        ProfileReader::parseResult,
        ArrayType::NOT_ARRAY);
    GetValueIfFindKey<bool>(jsonObject,
        jsonObjectEnd,
        MODULE_DESCRIPTION_INSTALLATION_FREE,
        info.installationFree,
        JsonType::BOOLEAN,
        false,
        ProfileReader::parseResult,
        ArrayType::NOT_ARRAY);
    GetValueIfFindKey<std::vector<std::string>>(jsonObject,
        jsonObjectEnd,
        MODULE_REQ_CAPABILITIES,
        info.reqCapabilities,
        JsonType::ARRAY,
        false,
        ProfileReader::parseResult,
        ArrayType::STRING);
    GetValueIfFindKey<std::vector<ReqPermission>>(jsonObject,
        jsonObjectEnd,
        MODULE_REQ_PERMS,
        info.reqPermissions,
        JsonType::ARRAY,
        false,
        ProfileReader::parseResult,
        ArrayType::OBJECT);
    GetValueIfFindKey<std::vector<DefPermission>>(jsonObject,
        jsonObjectEnd,
        MODULE_DEF_PERMS,
        info.defPermissions,
        JsonType::ARRAY,
        false,
        ProfileReader::parseResult,
        ArrayType::OBJECT);
    GetValueIfFindKey<std::vector<std::string>>(jsonObject,
        jsonObjectEnd,
        MODULE_ABILITY_KEYS,
        info.abilityKeys,
        JsonType::ARRAY,
        false,
        ProfileReader::parseResult,
        ArrayType::STRING);
    GetValueIfFindKey<std::vector<std::string>>(jsonObject,
        jsonObjectEnd,
        MODULE_SKILL_KEYS,
        info.skillKeys,
        JsonType::ARRAY,
        false,
        ProfileReader::parseResult,
        ArrayType::STRING);
}

void from_json(const nlohmann::json &jsonObject, SkillUri &uri)
{
    // these are required fields.
    const auto &jsonObjectEnd = jsonObject.end();
    GetValueIfFindKey<std::string>(jsonObject,
        jsonObjectEnd,
        ProfileReader::BUNDLE_MODULE_PROFILE_KEY_SCHEME,
        uri.scheme,
        JsonType::STRING,
        false,
        ProfileReader::parseResult,
        ArrayType::NOT_ARRAY);
    // these are not required fields.
    GetValueIfFindKey<std::string>(jsonObject,
        jsonObjectEnd,
        ProfileReader::BUNDLE_MODULE_PROFILE_KEY_HOST,
        uri.host,
        JsonType::STRING,
        false,
        ProfileReader::parseResult,
        ArrayType::NOT_ARRAY);
    GetValueIfFindKey<std::string>(jsonObject,
        jsonObjectEnd,
        ProfileReader::BUNDLE_MODULE_PROFILE_KEY_PORT,
        uri.port,
        JsonType::STRING,
        false,
        ProfileReader::parseResult,
        ArrayType::NOT_ARRAY);
    GetValueIfFindKey<std::string>(jsonObject,
        jsonObjectEnd,
        ProfileReader::BUNDLE_MODULE_PROFILE_KEY_PATH,
        uri.path,
        JsonType::STRING,
        false,
        ProfileReader::parseResult,
        ArrayType::NOT_ARRAY);
    GetValueIfFindKey<std::string>(jsonObject,
        jsonObjectEnd,
        ProfileReader::BUNDLE_MODULE_PROFILE_KEY_PATHSTARTWITH,
        uri.pathStartWith,
        JsonType::STRING,
        false,
        ProfileReader::parseResult,
        ArrayType::NOT_ARRAY);
    GetValueIfFindKey<std::string>(jsonObject,
        jsonObjectEnd,
        ProfileReader::BUNDLE_MODULE_PROFILE_KEY_PATHREGX,
        uri.pathRegx,
        JsonType::STRING,
        false,
        ProfileReader::parseResult,
        ArrayType::NOT_ARRAY);
    GetValueIfFindKey<std::string>(jsonObject,
        jsonObjectEnd,
        ProfileReader::BUNDLE_MODULE_PROFILE_KEY_TYPE,
        uri.type,
        JsonType::STRING,
        false,
        ProfileReader::parseResult,
        ArrayType::NOT_ARRAY);
}

void from_json(const nlohmann::json &jsonObject, Skill &skill)
{
    // these are not required fields.
    const auto &jsonObjectEnd = jsonObject.end();
    GetValueIfFindKey<std::vector<std::string>>(jsonObject,
        jsonObjectEnd,
        ProfileReader::BUNDLE_MODULE_PROFILE_KEY_ACTIONS,
        skill.actions,
        JsonType::ARRAY,
        false,
        ProfileReader::parseResult,
        ArrayType::STRING);
    GetValueIfFindKey<std::vector<std::string>>(jsonObject,
        jsonObjectEnd,
        ProfileReader::BUNDLE_MODULE_PROFILE_KEY_ENTITIES,
        skill.entities,
        JsonType::ARRAY,
        false,
        ProfileReader::parseResult,
        ArrayType::STRING);
    GetValueIfFindKey<std::vector<SkillUri>>(jsonObject,
        jsonObjectEnd,
        ProfileReader::BUNDLE_MODULE_PROFILE_KEY_URIS,
        skill.uris,
        JsonType::ARRAY,
        false,
        ProfileReader::parseResult,
        ArrayType::OBJECT);
}

void from_json(const nlohmann::json &jsonObject, Distro &distro)
{
    const auto &jsonObjectEnd = jsonObject.end();
    GetValueIfFindKey<bool>(jsonObject,
        jsonObjectEnd,
        ProfileReader::BUNDLE_MODULE_PROFILE_KEY_DELIVERY_WITH_INSTALL,
        distro.deliveryWithInstall,
        JsonType::BOOLEAN,
        true,
        ProfileReader::parseResult,
        ArrayType::NOT_ARRAY);
    GetValueIfFindKey<std::string>(jsonObject,
        jsonObjectEnd,
        ProfileReader::BUNDLE_MODULE_PROFILE_KEY_MODULE_NAME,
        distro.moduleName,
        JsonType::STRING,
        true,
        ProfileReader::parseResult,
        ArrayType::NOT_ARRAY);
    GetValueIfFindKey<std::string>(jsonObject,
        jsonObjectEnd,
        ProfileReader::BUNDLE_MODULE_PROFILE_KEY_MODULE_TYPE,
        distro.moduleType,
        JsonType::STRING,
        true,
        ProfileReader::parseResult,
        ArrayType::NOT_ARRAY);
    // mustFlag decide by distro.moduleType
    GetValueIfFindKey<bool>(jsonObject,
        jsonObjectEnd,
        ProfileReader::BUNDLE_MODULE_PROFILE_KEY_MODULE_INSTALLATION_FREE,
        distro.installationFree,
        JsonType::BOOLEAN,
        false,
        ProfileReader::parseResult,
        ArrayType::NOT_ARRAY);
}

void from_json(const nlohmann::json &jsonObject, UsedScene &usedScene)
{
    const auto &jsonObjectEnd = jsonObject.end();
    GetValueIfFindKey<std::vector<std::string>>(jsonObject,
        jsonObjectEnd,
        ProfileReader::BUNDLE_MODULE_PROFILE_KEY_REQ_PERMISSIONS_ABILITY,
        usedScene.ability,
        JsonType::ARRAY,
        false,
        ProfileReader::parseResult,
        ArrayType::STRING);
    GetValueIfFindKey<std::string>(jsonObject,
        jsonObjectEnd,
        ProfileReader::BUNDLE_MODULE_PROFILE_KEY_REQ_PERMISSIONS_WHEN,
        usedScene.when,
        JsonType::STRING,
        false,
        ProfileReader::parseResult,
        ArrayType::NOT_ARRAY);
}

void from_json(const nlohmann::json &jsonObject, ReqPermission &reqPermission)
{
    const auto &jsonObjectEnd = jsonObject.end();
    GetValueIfFindKey<std::string>(jsonObject,
        jsonObjectEnd,
        ProfileReader::BUNDLE_MODULE_PROFILE_KEY_REQ_PERMISSIONS_NAME,
        reqPermission.name,
        JsonType::STRING,
        true,
        ProfileReader::parseResult,
        ArrayType::NOT_ARRAY);
    GetValueIfFindKey<std::string>(jsonObject,
        jsonObjectEnd,
        ProfileReader::BUNDLE_MODULE_PROFILE_KEY_REQ_PERMISSIONS_REASON,
        reqPermission.reason,
        JsonType::STRING,
        false,
        ProfileReader::parseResult,
        ArrayType::NOT_ARRAY);
    GetValueIfFindKey<UsedScene>(jsonObject,
        jsonObjectEnd,
        ProfileReader::BUNDLE_MODULE_PROFILE_KEY_REQ_PERMISSIONS_USEDSCENE,
        reqPermission.usedScene,
        JsonType::OBJECT,
        false,
        ProfileReader::parseResult,
        ArrayType::NOT_ARRAY);
}

void from_json(const nlohmann::json &jsonObject, DefPermission &defPermission)
{
    const auto &jsonObjectEnd = jsonObject.end();
    GetValueIfFindKey<std::string>(jsonObject,
        jsonObjectEnd,
        ProfileReader::BUNDLE_MODULE_PROFILE_KEY_DEF_PERMISSIONS_NAME,
        defPermission.name,
        JsonType::STRING,
        true,
        ProfileReader::parseResult,
        ArrayType::NOT_ARRAY);
    GetValueIfFindKey<std::string>(jsonObject,
        jsonObjectEnd,
        ProfileReader::BUNDLE_MODULE_PROFILE_KEY_DEF_PERMISSIONS_GRANTMODE,
        defPermission.grantMode,
        JsonType::STRING,
        true,
        ProfileReader::parseResult,
        ArrayType::NOT_ARRAY);
    GetValueIfFindKey<std::vector<std::string>>(jsonObject,
        jsonObjectEnd,
        ProfileReader::BUNDLE_MODULE_PROFILE_KEY_DEF_PERMISSIONS_AVAILABLESCOPE,
        defPermission.availableScope,
        JsonType::ARRAY,
        false,
        ProfileReader::parseResult,
        ArrayType::STRING);
    GetValueIfFindKey<std::string>(jsonObject,
        jsonObjectEnd,
        ProfileReader::BUNDLE_MODULE_PROFILE_KEY_DEF_PERMISSIONS_LABEL,
        defPermission.label,
        JsonType::STRING,
        false,
        ProfileReader::parseResult,
        ArrayType::NOT_ARRAY);
    GetValueIfFindKey<int32_t>(jsonObject,
        jsonObjectEnd,
        ProfileReader::BUNDLE_MODULE_PROFILE_KEY_DEF_PERMISSIONS_LABEL_ID,
        defPermission.labelId,
        JsonType::NUMBER,
        false,
        ProfileReader::parseResult,
        ArrayType::NOT_ARRAY);
    GetValueIfFindKey<std::string>(jsonObject,
        jsonObjectEnd,
        ProfileReader::BUNDLE_MODULE_PROFILE_KEY_DEF_PERMISSIONS_DESCRIPTION,
        defPermission.description,
        JsonType::STRING,
        false,
        ProfileReader::parseResult,
        ArrayType::NOT_ARRAY);
    GetValueIfFindKey<int32_t>(jsonObject,
        jsonObjectEnd,
        ProfileReader::BUNDLE_MODULE_PROFILE_KEY_DEF_PERMISSIONS_DESCRIPTION_ID,
        defPermission.descriptionId,
        JsonType::NUMBER,
        false,
        ProfileReader::parseResult,
        ArrayType::NOT_ARRAY);
}

void from_json(const nlohmann::json &jsonObject, InstallMark &installMark)
{
    const auto &jsonObjectEnd = jsonObject.end();
    GetValueIfFindKey<std::string>(jsonObject,
        jsonObjectEnd,
        ProfileReader::BUNDLE_INSTALL_MARK_BUNDLE,
        installMark.bundleName,
        JsonType::STRING,
        false,
        ProfileReader::parseResult,
        ArrayType::NOT_ARRAY);
    GetValueIfFindKey<std::string>(jsonObject,
        jsonObjectEnd,
        ProfileReader::BUNDLE_INSTALL_MARK_PACKAGE,
        installMark.packageName,
        JsonType::STRING,
        false,
        ProfileReader::parseResult,
        ArrayType::NOT_ARRAY);
    GetValueIfFindKey<int32_t>(jsonObject,
        jsonObjectEnd,
        ProfileReader::BUNDLE_INSTALL_MARK_STATUS,
        installMark.status,
        JsonType::NUMBER,
        false,
        ProfileReader::parseResult,
        ArrayType::NOT_ARRAY);
}

int32_t InnerBundleInfo::FromJson(const nlohmann::json &jsonObject)
{
    const auto &jsonObjectEnd = jsonObject.end();
    GetValueIfFindKey<bool>(jsonObject,
        jsonObjectEnd,
        IS_SUPPORT_BACKUP,
        isSupportBackup_,
        JsonType::BOOLEAN,
        true,
        ProfileReader::parseResult,
        ArrayType::NOT_ARRAY);
    GetValueIfFindKey<Constants::AppType>(jsonObject,
        jsonObjectEnd,
        APP_TYPE,
        appType_,
        JsonType::NUMBER,
        true,
        ProfileReader::parseResult,
        ArrayType::NOT_ARRAY);
    GetValueIfFindKey<int>(jsonObject,
        jsonObjectEnd,
        UID,
        uid_,
        JsonType::NUMBER,
        false,
        ProfileReader::parseResult,
        ArrayType::NOT_ARRAY);
    GetValueIfFindKey<int>(jsonObject,
        jsonObjectEnd,
        GID,
        gid_,
        JsonType::NUMBER,
        false,
        ProfileReader::parseResult,
        ArrayType::NOT_ARRAY);
    GetValueIfFindKey<std::string>(jsonObject,
        jsonObjectEnd,
        BASE_DATA_DIR,
        baseDataDir_,
        JsonType::STRING,
        true,
        ProfileReader::parseResult,
        ArrayType::NOT_ARRAY);
    GetValueIfFindKey<BundleStatus>(jsonObject,
        jsonObjectEnd,
        BUNDLE_STATUS,
        bundleStatus_,
        JsonType::NUMBER,
        true,
        ProfileReader::parseResult,
        ArrayType::NOT_ARRAY);
    GetValueIfFindKey<BundleInfo>(jsonObject,
        jsonObjectEnd,
        BASE_BUNDLE_INFO,
        baseBundleInfo_,
        JsonType::OBJECT,
        true,
        ProfileReader::parseResult,
        ArrayType::NOT_ARRAY);
    GetValueIfFindKey<ApplicationInfo>(jsonObject,
        jsonObjectEnd,
        BASE_APPLICATION_INFO,
        baseApplicationInfo_,
        JsonType::OBJECT,
        true,
        ProfileReader::parseResult,
        ArrayType::NOT_ARRAY);
    GetValueIfFindKey<std::map<std::string, AbilityInfo>>(jsonObject,
        jsonObjectEnd,
        BASE_ABILITY_INFO,
        baseAbilityInfos_,
        JsonType::OBJECT,
        true,
        ProfileReader::parseResult,
        ArrayType::NOT_ARRAY);
    GetValueIfFindKey<std::map<std::string, InnerModuleInfo>>(jsonObject,
        jsonObjectEnd,
        INNER_MODULE_INFO,
        innerModuleInfos_,
        JsonType::OBJECT,
        true,
        ProfileReader::parseResult,
        ArrayType::NOT_ARRAY);
    GetValueIfFindKey<std::map<std::string, std::vector<Skill>>>(jsonObject,
        jsonObjectEnd,
        SKILL_INFOS,
        skillInfos_,
        JsonType::OBJECT,
        true,
        ProfileReader::parseResult,
        ArrayType::NOT_ARRAY);
    GetValueIfFindKey<int>(jsonObject,
        jsonObjectEnd,
        USER_ID,
        userId_,
        JsonType::NUMBER,
        true,
        ProfileReader::parseResult,
        ArrayType::NOT_ARRAY);
    GetValueIfFindKey<std::string>(jsonObject,
        jsonObjectEnd,
        MAIN_ABILITY,
        mainAbility_,
        JsonType::STRING,
        true,
        ProfileReader::parseResult,
        ArrayType::NOT_ARRAY);
    GetValueIfFindKey<std::string>(jsonObject,
        jsonObjectEnd,
        MAIN_ABILITY_NAME,
        mainAbilityName_,
        JsonType::STRING,
        false,
        ProfileReader::parseResult,
        ArrayType::NOT_ARRAY);
    GetValueIfFindKey<std::string>(jsonObject,
        jsonObjectEnd,
        APP_FEATURE,
        appFeature_,
        JsonType::STRING,
        true,
        ProfileReader::parseResult,
        ArrayType::NOT_ARRAY);
    GetValueIfFindKey<std::map<std::string, std::vector<FormInfo>>>(jsonObject,
        jsonObjectEnd,
        MODULE_FORMS,
        formInfos_,
        JsonType::OBJECT,
        true,
        ProfileReader::parseResult,
        ArrayType::NOT_ARRAY);
    GetValueIfFindKey<std::map<std::string, ShortcutInfo>>(jsonObject,
        jsonObjectEnd,
        MODULE_SHORTCUT,
        shortcutInfos_,
        JsonType::OBJECT,
        true,
        ProfileReader::parseResult,
        ArrayType::NOT_ARRAY);
    GetValueIfFindKey<std::map<std::string, CommonEventInfo>>(jsonObject,
        jsonObjectEnd,
        MODULE_COMMON_EVENT,
        commonEvents_,
        JsonType::OBJECT,
        true,
        ProfileReader::parseResult,
        ArrayType::NOT_ARRAY);
    GetValueIfFindKey<bool>(jsonObject,
        jsonObjectEnd,
        HAS_ENTRY,
        hasEntry_,
        JsonType::BOOLEAN,
        true,
        ProfileReader::parseResult,
        ArrayType::NOT_ARRAY);
    GetValueIfFindKey<std::string>(jsonObject,
        jsonObjectEnd,
        NEW_BUNDLE_NAME,
        newBundleName_,
        JsonType::STRING,
        true,
        ProfileReader::parseResult,
        ArrayType::NOT_ARRAY);
    GetValueIfFindKey<bool>(jsonObject,
        jsonObjectEnd,
        CAN_UNINSTALL,
        canUninstall_,
        JsonType::BOOLEAN,
        true,
        ProfileReader::parseResult,
        ArrayType::NOT_ARRAY);
    GetValueIfFindKey<bool>(jsonObject,
        jsonObjectEnd,
        IS_PREINSTALL_APP,
        isPreInstallApp_,
        JsonType::BOOLEAN,
        false,
        ProfileReader::parseResult,
        ArrayType::NOT_ARRAY);
    GetValueIfFindKey<InstallMark>(jsonObject,
        jsonObjectEnd,
        INSTALL_MARK,
        mark_,
        JsonType::OBJECT,
        false,
        ProfileReader::parseResult,
        ArrayType::NOT_ARRAY);
    int32_t isOldVersion = ERR_OK;
    GetValueIfFindKey<std::map<std::string, InnerBundleUserInfo>>(jsonObject,
        jsonObjectEnd,
        INNER_BUNDLE_USER_INFOS,
        innerBundleUserInfos_,
        JsonType::OBJECT,
        true,
        isOldVersion,
        ArrayType::NOT_ARRAY);
    int32_t ret = ProfileReader::parseResult;
    // need recover parse result to ERR_OK
    ProfileReader::parseResult = ERR_OK;
    if (ret == ERR_OK && isOldVersion == ERR_APPEXECFWK_PARSE_PROFILE_MISSING_PROP) {
        // To be compatible with the old database,
        // if the old data does not have bundleUserInfos,
        // the default user information needs to be constructed.
        BuildDefaultUserInfo();
    }
    return ret;
}

void InnerBundleInfo::BuildDefaultUserInfo()
{
    APP_LOGD("BuildDefaultUserInfo: bundleName: %{public}s.",
        baseApplicationInfo_.bundleName.c_str());
    InnerBundleUserInfo defaultInnerBundleUserInfo;
    defaultInnerBundleUserInfo.bundleUserInfo.userId = GetUserId();
    defaultInnerBundleUserInfo.uid = uid_;
    defaultInnerBundleUserInfo.gids.emplace_back(gid_);
    defaultInnerBundleUserInfo.installTime = baseBundleInfo_.installTime;
    defaultInnerBundleUserInfo.updateTime = baseBundleInfo_.updateTime;
    defaultInnerBundleUserInfo.bundleName = baseApplicationInfo_.bundleName;
    defaultInnerBundleUserInfo.bundleUserInfo.enabled = baseApplicationInfo_.enabled;
    AddInnerBundleUserInfo(defaultInnerBundleUserInfo);
}

std::optional<std::vector<Skill>> InnerBundleInfo::FindSkills(const std::string &keyName) const
{
    auto skillsInfo = skillInfos_.find(keyName);
    if (skillsInfo == skillInfos_.end()) {
        return std::nullopt;
    }
    auto &skills = skillsInfo->second;
    if (skills.empty()) {
        return std::nullopt;
    }
    return std::optional<std::vector<Skill>> {skills};
}

std::optional<HapModuleInfo> InnerBundleInfo::FindHapModuleInfo(const std::string &modulePackage, int32_t userId) const
{
    auto it = innerModuleInfos_.find(modulePackage);
    if (it == innerModuleInfos_.end()) {
        APP_LOGE("can not find module %{public}s", modulePackage.c_str());
        return std::nullopt;
    }
    HapModuleInfo hapInfo;
    hapInfo.name = it->second.modulePackage;
    hapInfo.moduleName = it->second.moduleName;
    hapInfo.description = it->second.description;
    hapInfo.supportedModes = baseApplicationInfo_.supportedModes;
    hapInfo.reqCapabilities = it->second.reqCapabilities;
    hapInfo.colorMode = it->second.colorMode;
    hapInfo.mainAbility = it->second.mainAbility;
    hapInfo.srcPath = it->second.srcPath;
    bool first = false;
    for (auto &ability : baseAbilityInfos_) {
        if (ability.first.find(modulePackage) != std::string::npos) {
            if (!first) {
                hapInfo.label = ability.second.label;
                hapInfo.iconPath = ability.second.iconPath;
                hapInfo.deviceTypes = ability.second.deviceTypes;
                first = true;
            }
            auto &abilityInfo = hapInfo.abilityInfos.emplace_back(ability.second);
            GetApplicationInfo(ApplicationFlag::GET_APPLICATION_INFO_WITH_PERMS, userId, abilityInfo.applicationInfo);
        }
    }
    return hapInfo;
}

std::optional<AbilityInfo> InnerBundleInfo::FindAbilityInfo(
    const std::string &bundleName, const std::string &abilityName, int32_t userId) const
{
    for (const auto &ability : baseAbilityInfos_) {
        auto abilityInfo = ability.second;
        if ((abilityInfo.bundleName == bundleName) && (abilityInfo.name == abilityName)) {
            GetApplicationInfo(
                ApplicationFlag::GET_APPLICATION_INFO_WITH_PERMS, userId, abilityInfo.applicationInfo);
            return abilityInfo;
        }
    }
    return std::nullopt;
}

std::optional<std::vector<AbilityInfo>> InnerBundleInfo::FindAbilityInfos(
    const std::string &bundleName, int32_t userId) const
{
    std::vector<AbilityInfo> abilitys;

    if (bundleName.empty()) {
        return std::nullopt;
    }

    for (const auto &ability : baseAbilityInfos_) {
        auto abilityInfo = ability.second;
        if ((abilityInfo.bundleName == bundleName)) {
            GetApplicationInfo(
                ApplicationFlag::GET_APPLICATION_INFO_WITH_PERMS, userId, abilityInfo.applicationInfo);
            abilitys.emplace_back(abilityInfo);
        }
    }
    if (!abilitys.empty()) {
        return abilitys;
    }

    return std::nullopt;
}

void InnerBundleInfo::FindAbilityInfosForClone(const std::string &bundleName,
    const std::string &abilityName, int32_t userId, std::vector<AbilityInfo> &abilitys)
{
    if (bundleName.empty()) {
        return;
    }

    for (auto &ability : baseAbilityInfos_) {
        APP_LOGE("FindAbilityInfosForClonekey = %{public}s", ability.first.c_str());
        auto abilityInfo = ability.second;
        if ((abilityInfo.bundleName == bundleName && (abilityInfo.name == abilityName))) {
            GetApplicationInfo(
                ApplicationFlag::GET_APPLICATION_INFO_WITH_PERMS, userId, abilityInfo.applicationInfo);
            abilitys.emplace_back(abilityInfo);
        }
    }
    return;
}

bool InnerBundleInfo::AddModuleInfo(const InnerBundleInfo &newInfo)
{
    if (newInfo.currentPackage_.empty()) {
        APP_LOGE("current package is empty");
        return false;
    }
    if (FindModule(newInfo.currentPackage_)) {
        APP_LOGE("current package %{public}s is exist", currentPackage_.c_str());
        return false;
    }
    if (!hasEntry_ && newInfo.HasEntry()) {
        hasEntry_ = true;
    }
    if (mainAbility_.empty() && !newInfo.mainAbility_.empty()) {
        UpdateBaseApplicationInfo(newInfo.baseApplicationInfo_);
        SetMainAbility(newInfo.mainAbility_);
        SetMainAbilityName(newInfo.mainAbilityName_);
    }
    AddInnerModuleInfo(newInfo.innerModuleInfos_);
    AddModuleAbilityInfo(newInfo.baseAbilityInfos_);
    AddModuleSkillInfo(newInfo.skillInfos_);
    AddModuleFormInfo(newInfo.formInfos_);
    AddModuleShortcutInfo(newInfo.shortcutInfos_);
    AddModuleCommonEvent(newInfo.commonEvents_);
    return true;
}

void InnerBundleInfo::UpdateVersionInfo(const InnerBundleInfo &newInfo)
{
    if (baseBundleInfo_.versionCode > newInfo.GetVersionCode()) {
        APP_LOGE("old version larger than new version");
        return;
    }
    baseBundleInfo_.vendor = newInfo.GetBaseBundleInfo().vendor;
    baseBundleInfo_.minSdkVersion = newInfo.GetBaseBundleInfo().minSdkVersion;
    baseBundleInfo_.maxSdkVersion = newInfo.GetBaseBundleInfo().maxSdkVersion;
    // version
    baseBundleInfo_.versionCode = newInfo.GetVersionCode();
    baseBundleInfo_.versionName = newInfo.GetBaseBundleInfo().versionName;
    // apiversion
    baseBundleInfo_.compatibleVersion = newInfo.GetBaseBundleInfo().compatibleVersion;
    baseBundleInfo_.targetVersion = newInfo.GetBaseBundleInfo().targetVersion;
    baseBundleInfo_.releaseType = newInfo.GetBaseBundleInfo().releaseType;
}

void InnerBundleInfo::updateCommonHapInfo(const InnerBundleInfo &newInfo)
{
    if (newInfo.currentPackage_.empty()) {
        APP_LOGE("no package in new info");
        return;
    }
    SetIsKeepData(newInfo.GetIsKeepData());
    SetIsSupportBackup(newInfo.GetIsSupportBackup());
}

void InnerBundleInfo::UpdateModuleInfo(const InnerBundleInfo &newInfo)
{
    if (newInfo.currentPackage_.empty()) {
        APP_LOGE("no package in new info");
        return;
    }
    innerModuleInfos_.erase(newInfo.currentPackage_);
    for (auto it = baseAbilityInfos_.begin(); it != baseAbilityInfos_.end();) {
        if (it->first.find(newInfo.currentPackage_) != std::string::npos) {
            skillInfos_.erase(it->first);
            formInfos_.erase(it->first);
            it = baseAbilityInfos_.erase(it);
        } else {
            ++it;
        }
    }
    for (auto it = shortcutInfos_.begin(); it != shortcutInfos_.end();) {
        if (it->first.find(newInfo.currentPackage_) != std::string::npos) {
            shortcutInfos_.erase(it++);
        } else {
            ++it;
        }
    }
    for (auto it = commonEvents_.begin(); it != commonEvents_.end();) {
        if (it->first.find(newInfo.currentPackage_) != std::string::npos) {
            commonEvents_.erase(it++);
        } else {
            ++it;
        }
    }
    if (!hasEntry_ && newInfo.HasEntry()) {
        hasEntry_ = true;
    }
    if (mainAbility_ == newInfo.mainAbility_) {
        UpdateBaseApplicationInfo(newInfo.baseApplicationInfo_);
    }
    AddInnerModuleInfo(newInfo.innerModuleInfos_);
    AddModuleAbilityInfo(newInfo.baseAbilityInfos_);
    AddModuleSkillInfo(newInfo.skillInfos_);
    AddModuleFormInfo(newInfo.formInfos_);
    AddModuleShortcutInfo(newInfo.shortcutInfos_);
    AddModuleCommonEvent(newInfo.commonEvents_);
}

void InnerBundleInfo::RemoveModuleInfo(const std::string &modulePackage)
{
    if (innerModuleInfos_.find(modulePackage) == innerModuleInfos_.end()) {
        APP_LOGE("can not find module %{public}s", modulePackage.c_str());
        return;
    }
    if (mainAbility_.find(modulePackage) != std::string::npos) {
        mainAbility_.clear();
        hasEntry_ = false;
        mainAbilityName_.clear();
    }
    for (auto it = innerModuleInfos_.begin(); it != innerModuleInfos_.end();) {
        if (it->first == modulePackage) {
            innerModuleInfos_.erase(it++);
        } else {
            ++it;
        }
    }
    for (auto it = baseAbilityInfos_.begin(); it != baseAbilityInfos_.end();) {
        if (it->first.find(modulePackage) != std::string::npos) {
            baseAbilityInfos_.erase(it++);
        } else {
            ++it;
        }
    }
    for (auto it = skillInfos_.begin(); it != skillInfos_.end();) {
        if (it->first.find(modulePackage) != std::string::npos) {
            skillInfos_.erase(it++);
        } else {
            ++it;
        }
    }
    for (auto it = formInfos_.begin(); it != formInfos_.end();) {
        if (it->first.find(modulePackage) != std::string::npos) {
            formInfos_.erase(it++);
        } else {
            ++it;
        }
    }
    for (auto it = shortcutInfos_.begin(); it != shortcutInfos_.end();) {
        if (it->first.find(modulePackage) != std::string::npos) {
            shortcutInfos_.erase(it++);
        } else {
            ++it;
        }
    }
    for (auto it = commonEvents_.begin(); it != commonEvents_.end();) {
        if (it->first.find(modulePackage) != std::string::npos) {
            commonEvents_.erase(it++);
        } else {
            ++it;
        }
    }
}

std::string InnerBundleInfo::ToString() const
{
    nlohmann::json j;
    j[IS_SUPPORT_BACKUP] = isSupportBackup_;
    j[APP_TYPE] = appType_;
    j[UID] = uid_;
    j[GID] = gid_;
    j[BASE_DATA_DIR] = baseDataDir_;
    j[BUNDLE_STATUS] = bundleStatus_;
    j[BASE_APPLICATION_INFO] = baseApplicationInfo_;
    j[BASE_BUNDLE_INFO] = baseBundleInfo_;
    j[BASE_ABILITY_INFO] = baseAbilityInfos_;
    j[INNER_MODULE_INFO] = innerModuleInfos_;
    j[SKILL_INFOS] = skillInfos_;
    j[USER_ID] = userId_;
    j[MAIN_ABILITY] = mainAbility_;
    j[MAIN_ABILITY_NAME] = mainAbilityName_;
    j[APP_FEATURE] = appFeature_;
    j[HAS_ENTRY] = hasEntry_;
    j[MODULE_FORMS] = formInfos_;
    j[MODULE_SHORTCUT] = shortcutInfos_;
    j[NEW_BUNDLE_NAME] = newBundleName_;
    j[MODULE_COMMON_EVENT] = commonEvents_;
    j[CAN_UNINSTALL] = canUninstall_;
    j[IS_PREINSTALL_APP] = isPreInstallApp_;
    j[INSTALL_MARK] = mark_;
    j[INNER_BUNDLE_USER_INFOS] = innerBundleUserInfos_;
    return j.dump();
}

void InnerBundleInfo::GetApplicationInfo(int32_t flags, int32_t userId, ApplicationInfo &appInfo) const
{
    InnerBundleUserInfo innerBundleUserInfo;
    if (!GetInnerBundleUserInfo(userId, innerBundleUserInfo)) {
        APP_LOGE("can not find userId %{public}d when get applicationInfo", userId);
        return;
    }

    appInfo = baseApplicationInfo_;
    appInfo.enabled = innerBundleUserInfo.bundleUserInfo.enabled;
    appInfo.uid = innerBundleUserInfo.uid;
    for (const auto &info : innerModuleInfos_) {
        ModuleInfo moduleInfo;
        moduleInfo.moduleName = info.second.moduleName;
        moduleInfo.moduleSourceDir = info.second.modulePath;
        appInfo.moduleInfos.emplace_back(moduleInfo);
        appInfo.moduleSourceDirs.emplace_back(info.second.modulePath);
        if (info.second.isEntry) {
            appInfo.entryDir = info.second.modulePath;
        }
        if ((static_cast<uint32_t>(flags) & GET_APPLICATION_INFO_WITH_PERMS) == GET_APPLICATION_INFO_WITH_PERMS) {
            std::transform(info.second.reqPermissions.begin(),
                info.second.reqPermissions.end(),
                std::back_inserter(appInfo.permissions),
                [](const auto &p) { return p.name; });
        }
        if ((static_cast<uint32_t>(flags) & GET_APPLICATION_INFO_WITH_METADATA) == GET_APPLICATION_INFO_WITH_METADATA
            && info.second.metaData.customizeData.size() > 0) {
            appInfo.metaData[info.second.moduleName] = info.second.metaData.customizeData;
        }
    }
}

void InnerBundleInfo::GetBundleInfo(int32_t flags, BundleInfo &bundleInfo, int32_t userId) const
{
    InnerBundleUserInfo innerBundleUserInfo;
    if (!GetInnerBundleUserInfo(userId, innerBundleUserInfo)) {
        APP_LOGE("can not find userId %{public}d when GetBundleInfo", userId);
        return;
    }

    bundleInfo = baseBundleInfo_;
    bundleInfo.uid = innerBundleUserInfo.uid;
    if (!innerBundleUserInfo.gids.empty()) {
        bundleInfo.gid = innerBundleUserInfo.gids[0];
    }

    bundleInfo.installTime = innerBundleUserInfo.installTime;
    bundleInfo.updateTime = innerBundleUserInfo.updateTime;
    GetApplicationInfo(ApplicationFlag::GET_BASIC_APPLICATION_INFO, userId, bundleInfo.applicationInfo);
    for (const auto &info : innerModuleInfos_) {
        if ((static_cast<uint32_t>(flags) & GET_BUNDLE_WITH_REQUESTED_PERMISSION)
            == GET_BUNDLE_WITH_REQUESTED_PERMISSION) {
            std::transform(info.second.reqPermissions.begin(),
                info.second.reqPermissions.end(),
                std::back_inserter(bundleInfo.reqPermissions),
                [](const auto &p) { return p.name; });
            std::transform(info.second.defPermissions.begin(),
                info.second.defPermissions.end(),
                std::back_inserter(bundleInfo.defPermissions),
                [](const auto &p) { return p.name; });
        }
        bundleInfo.hapModuleNames.emplace_back(info.second.modulePackage);
        auto hapmoduleinfo = FindHapModuleInfo(info.second.modulePackage, userId);
        if (hapmoduleinfo) {
            bundleInfo.hapModuleInfos.emplace_back(*hapmoduleinfo);
            bundleInfo.moduleNames.emplace_back(info.second.moduleName);
            bundleInfo.moduleDirs.emplace_back(info.second.modulePath);
            bundleInfo.modulePublicDirs.emplace_back(info.second.moduleDataDir);
            bundleInfo.moduleResPaths.emplace_back(info.second.moduleResPath);
        } else {
            APP_LOGE("can not find hapmoduleinfo %{public}s", info.second.moduleName.c_str());
        }
        if (info.second.isEntry) {
            bundleInfo.mainEntry = info.second.modulePackage;
            bundleInfo.entryModuleName = info.second.moduleName;
        }
    }
    GetBundleWithAbilities(flags, bundleInfo, userId);
}

void InnerBundleInfo::GetBundleWithAbilities(int32_t flags, BundleInfo &bundleInfo, int32_t userId) const
{
    if (static_cast<uint32_t>(flags) & GET_BUNDLE_WITH_ABILITIES) {
        for (auto &ability : baseAbilityInfos_) {
            if (!(static_cast<uint32_t>(flags) & GET_ABILITY_INFO_WITH_DISABLE)
                && !ability.second.enabled) {
                continue;
            }
            AbilityInfo abilityInfo = ability.second;
            GetApplicationInfo(ApplicationFlag::GET_BASIC_APPLICATION_INFO, userId, abilityInfo.applicationInfo);
            bundleInfo.abilityInfos.emplace_back(abilityInfo);
        }
    }
}

bool InnerBundleInfo::CheckSpecialMetaData(const std::string &metaData) const
{
    for (const auto &moduleInfo : innerModuleInfos_) {
        for (const auto &data : moduleInfo.second.metaData.customizeData) {
            if (metaData == data.name) {
                return true;
            }
        }
    }
    return false;
}

void InnerBundleInfo::GetFormsInfoByModule(const std::string &moduleName, std::vector<FormInfo> &formInfos) const
{
    for (const auto &data : formInfos_) {
        for (auto &form : data.second) {
            if (form.moduleName == moduleName) {
                formInfos.emplace_back(form);
            }
        }
    }
}

void InnerBundleInfo::GetFormsInfoByApp(std::vector<FormInfo> &formInfos) const
{
    for (const auto &data : formInfos_) {
        for (auto &form : data.second) {
            formInfos.emplace_back(form);
        }
    }
}

void InnerBundleInfo::GetShortcutInfos(std::vector<ShortcutInfo> &shortcutInfos) const
{
    for (const auto &shortcut : shortcutInfos_) {
        shortcutInfos.emplace_back(shortcut.second);
    }
}

void InnerBundleInfo::GetCommonEvents(const std::string &eventKey, std::vector<CommonEventInfo> &commonEvents) const
{
    CommonEventInfo item;
    for (const auto &commonEvent : commonEvents_) {
        for (const auto &event : commonEvent.second.events) {
            if (event == eventKey) {
                item = commonEvent.second;
                item.uid = GetUid();
                commonEvents.emplace_back(item);
                break;
            }
        }
    }
}

std::optional<InnerModuleInfo> InnerBundleInfo::GetInnerModuleInfoByModuleName(const std::string &moduleName) const
{
    for (const auto &innerModuleInfo : innerModuleInfos_) {
        if (innerModuleInfo.second.moduleName == moduleName) {
            return innerModuleInfo.second;
        }
    }
    return std::nullopt;
}

void InnerBundleInfo::GetModuleNames(std::vector<std::string> &moduleNames) const
{
    for (const auto &innerModuleInfo : innerModuleInfos_) {
        moduleNames.emplace_back(innerModuleInfo.second.moduleName);
    }
}

void InnerBundleInfo::RemoveInnerBundleUserInfo(int32_t userId)
{
    auto& key = NameAndUserIdToKey(GetBundleName(), userId);
    auto infoItem = innerBundleUserInfos_.find(key);
    if (infoItem == innerBundleUserInfos_.end()) {
        return;
    }

    innerBundleUserInfos_.erase(key);
}

void InnerBundleInfo::AddInnerBundleUserInfo(
    const InnerBundleUserInfo& innerBundleUserInfo)
{
    auto& key = NameAndUserIdToKey(
        GetBundleName(), innerBundleUserInfo.bundleUserInfo.userId);
    auto infoItem = innerBundleUserInfos_.find(key);
    if (infoItem == innerBundleUserInfos_.end()) {
        innerBundleUserInfos_.emplace(key, innerBundleUserInfo);
        return;
    }

    innerBundleUserInfos_[key] = innerBundleUserInfo;
}

bool InnerBundleInfo::GetInnerBundleUserInfo(
    int32_t userId, InnerBundleUserInfo& innerBundleUserInfo) const
{
    auto& key = NameAndUserIdToKey(GetBundleName(), userId);
    auto infoItem = innerBundleUserInfos_.find(key);
    if (infoItem == innerBundleUserInfos_.end()) {
        APP_LOGD("no this user %{public}s", key.c_str());
        return false;
    }

    innerBundleUserInfo = infoItem->second;
    return true;
}

bool InnerBundleInfo::HasInnerBundleUserInfo(int32_t userId) const
{
    auto& key = NameAndUserIdToKey(GetBundleName(), userId);
    auto infoItem = innerBundleUserInfos_.find(key);
    return infoItem != innerBundleUserInfos_.end();
}

void InnerBundleInfo::SetBundleInstallTime(const int64_t time, int32_t userId)
{
    auto& key = NameAndUserIdToKey(GetBundleName(), userId);
    auto infoItem = innerBundleUserInfos_.find(key);
    if (infoItem == innerBundleUserInfos_.end()) {
        return;
    }

    infoItem->second.installTime = time;
    infoItem->second.updateTime = time;
}

void InnerBundleInfo::SetBundleUpdateTime(const int64_t time, int32_t userId)
{
    auto& key = NameAndUserIdToKey(GetBundleName(), userId);
    auto infoItem = innerBundleUserInfos_.find(key);
    if (infoItem == innerBundleUserInfos_.end()) {
        return;
    }

    infoItem->second.updateTime = time;
}

void InnerBundleInfo::SetApplicationEnabled(bool enabled, int32_t userId)
{
    auto& key = NameAndUserIdToKey(GetBundleName(), userId);
    auto infoItem = innerBundleUserInfos_.find(key);
    if (infoItem == innerBundleUserInfos_.end()) {
        return;
    }

    infoItem->second.bundleUserInfo.enabled = enabled;
}

int32_t InnerBundleInfo::GetResponseUserId(int32_t requestUserId) const
{
    if (HasInnerBundleUserInfo(requestUserId)) {
        return requestUserId;
    }

    if (requestUserId < Constants::START_USERID) {
        APP_LOGE("requestUserId(%{public}d) less than start userId.", requestUserId);
        return Constants::INVALID_USERID;
    }

    int32_t responseUserId = Constants::INVALID_USERID;
    for (const auto &innerBundleUserInfo : innerBundleUserInfos_) {
        if (innerBundleUserInfo.second.bundleUserInfo.userId < Constants::START_USERID) {
            responseUserId = innerBundleUserInfo.second.bundleUserInfo.userId;
            break;
        }
    }

    return responseUserId;
}
}  // namespace AppExecFwk
}  // namespace OHOS
