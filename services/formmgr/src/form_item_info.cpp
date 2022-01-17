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
#include "app_log_wrapper.h"
#include "form_item_info.h"

namespace OHOS {
namespace AppExecFwk {
/**
 * @brief Get formId_.
 * @return formId_.
 */
int64_t FormItemInfo::GetFormId() const
{
    return formId_;
}
/**
 * @brief Get packageName_.
 * @return packageName_.
 */
std::string FormItemInfo::GetPackageName() const
{
    return packageName_;
}
/**
 * @brief Get providerBundleName_.
 * @return providerBundleName_.
 */
std::string FormItemInfo::GetProviderBundleName() const
{
    return providerBundleName_;
}
/**
 * @brief Get hostBundleName_.
 * @return hostBundleName_.
 */
std::string FormItemInfo::GetHostBundleName() const
{
    return hostBundleName_;
}
/**
 * @brief Get moduleName_.
 * @return moduleName_.
 */
std::string FormItemInfo::GetModuleName() const
{
    return moduleName_;
}
/**
 * @brief Get abilityName_.
 * @return abilityName_.
 */
std::string FormItemInfo::GetAbilityName() const
{
    return abilityName_;
}
/**
 * @brief Get formName_.
 * @return formName_.
 */
std::string FormItemInfo::GetFormName() const
{
    return formName_;
}
/**
 * @brief Get jsComponentName_.
 * @return jsComponentName_.
 */
std::string FormItemInfo::GetJsComponentName() const
{
    return jsComponentName_;
}
/**
 * @brief Get abilityModuleName_.
 * @return abilityModuleName_.
 */
std::string FormItemInfo::GetAbilityModuleName() const
{
    return abilityModuleName_;
}
/**
 * @brief Get specificationId_.
 * @return specificationId_.
 */
int FormItemInfo::GetSpecificationId() const
{
    return specificationId_;
}

/**
 * @brief Obtains the updageFlag.
 * @return Returns updageFlag.
 */
bool FormItemInfo::IsEnableUpdateFlag() const
{
    return updateFlag_;
}
/**
 * @brief Get updateDuration_.
 * @return updateDuration_.
 */
int FormItemInfo::GetUpdateDuration() const
{
    return updateDuration_;
}
/**
 * @brief Get scheduledUpdateTime_.
 * @return scheduledUpdateTime_.
 */
std::string FormItemInfo::GetScheduledUpdateTime() const
{
    return scheduledUpdateTime_;
}

/**
 * @brief Get hapSourceDirs_.
 * @param dirs Hap source dirs.
 * @return Returns true on success, false on failure.
 */
bool FormItemInfo::GetHapSourceDirs(std::vector<std::string> &dirs) const
{
    if (hapSourceDirs_.size() > 0) {
        dirs.assign(hapSourceDirs_.begin(), hapSourceDirs_.end());
        return true;
    }
    return false;
}
/**
 * @brief Set value of hapSourceDirs_.
 * @param hapSourceDirs Hap source dirs.
 */
void FormItemInfo::SetHapSourceDirs(const std::vector<std::string> &hapSourceDirs)
{
    hapSourceDirs_ = hapSourceDirs;
}
/**
 * @brief Obtains the temporaryFlag.
 * @return Returns temporaryFlag.
 */
bool FormItemInfo::IsTemporaryForm() const
{
    return temporaryFlag_;
}

/**
 * @brief Obtains the hap source by ability module name.
 * @param moduleName ability module name
 * @return Returns hap source.
 */
std::string FormItemInfo::GetHapSourceByModuleName(const std::string &moduleName) const
{
    auto iter = moduleInfoMap_.find(moduleName);
    if (iter != moduleInfoMap_.end()) {
        return iter->second;
    }
    return "";
}
/**
 * @brief Check if item valid or not.
 * @return Valid or not
 */
bool FormItemInfo::IsValidItem() const
{
    if (providerBundleName_.empty() || moduleName_.empty()
        || abilityName_.empty() || formName_.empty()) {
        return false;
    }
    return true;
}
/**
 * @brief Check if item match or not.
 * @return Match or not
 */
bool FormItemInfo::IsMatch(const FormRecord &record) const
{
    APP_LOGD("match data");
    APP_LOGD("FormRecord.bundleName : %{public}s", record.bundleName.c_str());
    APP_LOGD("FormRecord.moduleName : %{public}s", record.moduleName.c_str());
    APP_LOGD("FormRecord.abilityName : %{public}s", record.abilityName.c_str());
    APP_LOGD("FormRecord.formName : %{public}s", record.formName.c_str());
    APP_LOGD("FormRecord.specification : %{public}d", record.specification);

    APP_LOGD("FormItemInfo.providerBundleName_ : %{public}s", providerBundleName_.c_str());
    APP_LOGD("FormItemInfo.moduleName : %{public}s", moduleName_.c_str());
    APP_LOGD("FormItemInfo.abilityName : %{public}s", abilityName_.c_str());
    APP_LOGD("FormItemInfo.formName : %{public}s", formName_.c_str());
    APP_LOGD("FormItemInfo.specification : %{public}d", specificationId_);

    return (record.bundleName == providerBundleName_) && (record.moduleName == moduleName_)
        && (record.abilityName == abilityName_) && (record.formName == formName_)
        && (record.specification == specificationId_);
}
/**
 * @brief Check if form config same or not.
 * @return Same or not
 */
bool FormItemInfo::IsSameFormConfig(const FormRecord &record) const
{
    return (record.bundleName == providerBundleName_) && (record.moduleName == moduleName_)
        && (record.abilityName == abilityName_) && (record.formName == formName_);
}

/**
 * @brief Check if visible notify or not.
 * @return visible notify or not
 */
bool FormItemInfo::IsFormVisibleNotify() const
{
    return formVisibleNotify_;
}
/**
 * @brief Equal or not.
 * @param left left string.
 * @param right right string.
 * @return Equal or not
 */
bool FormItemInfo::IsEqual(const std::string &left, const std::string &right)
{
    return left == right;
}
/**
 * @brief Set value of formId_.
 * @param formId Form Id.
 */
void FormItemInfo::SetFormId(int64_t formId)
{
    formId_ = formId;
}
/**
 * @brief Set value of packageName_.
 * @param packageName Package name.
 */
void FormItemInfo::SetPackageName(const std::string &packageName)
{
    packageName_ = packageName;
}
/**
 * @brief Set value of providerBundleName_.
 * @param providerBundleName Provider bundle Name.
 */
void FormItemInfo::SetProviderBundleName(const std::string &providerBundleName)
{
    providerBundleName_ = providerBundleName;
}
/**
 * @brief Set value of hostBundleName_.
 * @param hostBundleName Host bundle Name.
 */
void FormItemInfo::SetHostBundleName(const std::string &hostBundleName)
{
    hostBundleName_ = hostBundleName;
}
/**
 * @brief Set value of moduleName_.
 * @param moduleName Module Name.
 */
void FormItemInfo::SetModuleName(const std::string &moduleName)
{
    moduleName_ = moduleName;
}
/**
 * @brief Set value of abilityName_.
 * @param abilityName Ability name.
 */
void FormItemInfo::SetAbilityName(const std::string &abilityName)
{
    abilityName_ = abilityName;
}
/**
 * @brief Set value of formName_.
 * @param formName Form name.
 */
void FormItemInfo::SetFormName(const std::string &formName)
{
    formName_ = formName;
}
/**
 * @brief Set value of jsComponentName_.
 * @param jsComponentName Js component name.
 */
void FormItemInfo::SetJsComponentName(const std::string &jsComponentName)
{
    jsComponentName_ = jsComponentName;
}
/**
 * @brief Set value of abilityModuleName_.
 * @param abilityModuleName ability module name_.
 */
void FormItemInfo::SetAbilityModuleName(const std::string &abilityModuleName)
{
    abilityModuleName_ = abilityModuleName;
}
/**
 * @brief Set value of specificationId_.
 * @param specificationId Specification id.
 */
void FormItemInfo::SetSpecificationId(const int specificationId)
{
    specificationId_ = specificationId;
}
/**
 * @brief Set value of updateFlag_.
 * @param IsEnableUpdateFlag Enable update flag or not.
 */
void FormItemInfo::SetEnableUpdateFlag(bool IsEnableUpdateFlag)
{
    updateFlag_ = IsEnableUpdateFlag;
}
/**
 * @brief Set value of updateDuration_.
 * @param updateDuration Update duration.
 */
void FormItemInfo::SetUpdateDuration(int updateDuration)
{
    updateDuration_ = updateDuration;
}
/**
 * @brief Set value of scheduledUpdateTime_.
 * @param scheduledUpdateTime Scheduled update time.
 */
void FormItemInfo::SetScheduledUpdateTime(const std::string &scheduledUpdateTime)
{
    scheduledUpdateTime_ = scheduledUpdateTime;
}
/**
 * @brief Add hap source dir.
 * @param hapSourceDir Hap source dir.
 */
void FormItemInfo::AddHapSourceDirs(const std::string &hapSourceDir)
{
    hapSourceDirs_.emplace_back(hapSourceDir);
}
/**
 * @brief Set value of temporaryFlag_.
 * @param temporaryFlag Temporary flag.
 */
void FormItemInfo::SetTemporaryFlag(bool temporaryFlag)
{
    temporaryFlag_ = temporaryFlag;
}
/**
 * @brief Add module info.
 * @param moduleName Module name.
 * @param moduleSourceDir Module source dir.
 */
void FormItemInfo::AddModuleInfo(const std::string &moduleName, const std::string &moduleSourceDir)
{
    moduleInfoMap_.emplace(std::make_pair(moduleName, moduleSourceDir));
}
/**
 * @brief Set value of formVisibleNotify_.
 * @param isFormVisibleNotify visible notify or not.
 */
void FormItemInfo::SetFormVisibleNotify(bool isFormVisibleNotify)
{
    formVisibleNotify_ = isFormVisibleNotify;
}
/**
 * @brief Get formSrc_.
 * @return formSrc_.
 */
std::string FormItemInfo::GetFormSrc() const
{
    return formSrc_;
}
/**
 * @brief Set value of formSrc_.
 * @param formSrc form src.
 */
void FormItemInfo::SetFormSrc(const std::string &formSrc)
{
    formSrc_ = formSrc;
}
/**
 * @brief Get formWindow_.
 * @return formWindow_.
 */
FormWindow FormItemInfo::GetFormWindow() const
{
    return formWindow_;
}
/**
 * @brief Set value of formWindow_.
 * @param formWindow form window.
 */
void FormItemInfo::SetFormWindow(const FormWindow &formWindow)
{
    formWindow_.autoDesignWidth = formWindow.autoDesignWidth;
    formWindow_.designWidth = formWindow.designWidth;
}
/**
 * @brief Get versionCode_.
 * @return versionCode_.
 */
uint32_t FormItemInfo::GetVersionCode() const
{
    return versionCode_;
}
/**
 * @brief Set value of versionCode_.
 * @param versionCode bundle version code.
 */
void FormItemInfo::SetVersionCode(const uint32_t versionCode)
{
    versionCode_ = versionCode;
}
/**
 * @brief Get versionName_.
 * @return versionName_.
 */
std::string FormItemInfo::GetVersionName() const
{
    return versionName_;
}
/**
 * @brief Set value of versionName_.
 * @param versionName bundle version name.
 */
void FormItemInfo::SetVersionName(const std::string &versionName)
{
    versionName_ = versionName;
}
/**
 * @brief Get compatibleVersion_.
 * @return compatibleVersion_.
 */
uint32_t FormItemInfo::GetCompatibleVersion() const
{
    return compatibleVersion_;
}
/**
 * @brief Set value of compatibleVersion_.
 * @param compatibleVersion bundle version name.
 */
void FormItemInfo::SetCompatibleVersion(const uint32_t &compatibleVersion)
{
    compatibleVersion_ = compatibleVersion;
}
/**
 * @brief Get icon_.
 * @return icon_.
 */
std::string FormItemInfo::GetIcon() const
{
    return icon_;
}
/**
 * @brief Set value of icon_.
 * @param icon bundle version name.
 */
void FormItemInfo::SetIcon(const std::string &icon)
{
    icon_ = icon;
}
}  // namespace AppExecFwk
}  // namespace OHOS