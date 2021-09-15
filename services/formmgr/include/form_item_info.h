/*
 * Copyright (c) 2021 Huawei Device Co., Ltd.
 * Licensed under the Apache License, Version 2.0 (the "License")_;
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

#ifndef FOUNDATION_APPEXECFWK_SERVICES_FORMMGR_INCLUDE_FORM_CONFIG_INFO_H
#define FOUNDATION_APPEXECFWK_SERVICES_FORMMGR_INCLUDE_FORM_CONFIG_INFO_H

#include <stdint.h>
#include <string>
#include <vector>
#include <unordered_map>
#include "form_record.h"

namespace OHOS {
namespace AppExecFwk {
class FormItemInfo {
public:
    /**
     * @brief Get formId_.
     * @return formId_.
     */
    int64_t GetFormId() const;
    /**
     * @brief Set value of formId_.
     * @param formId Form Id.
     */
    void SetFormId(int64_t formId);
    /**
     * @brief Get packageName_.
     * @return packageName_.
     */
    std::string GetPackageName() const;
    /**
     * @brief Set value of packageName_.
     * @param packageName Package name.
     */
    void SetPackageName(const std::string &packageName);
    /**
     * @brief Get providerBundleName_.
     * @return providerBundleName_.
     */
    std::string GetProviderBundleName() const;
    /**
     * @brief Set value of providerBundleName_.
     * @param providerBundleName Provider bundle Name.
     */
    void SetProviderBundleName(const std::string &providerBundleName_);
    /**
     * @brief Get hostBundleName_.
     * @return hostBundleName_.
     */
    std::string GetHostBundleName() const;
    /**
     * @brief Set value of hostBundleName_.
     * @param hostBundleName_ Host bundle Name.
     */
    void SetHostBundleName(const std::string &hostBundleName_);
    /**
     * @brief Get moduleName_.
     * @return moduleName_.
     */
    std::string GetModuleName() const;
    /**
     * @brief Set value of moduleName_.
     * @param moduleName Module Name.
     */
    void SetModuleName(const std::string &moduleName);
    /**
     * @brief Get abilityName_.
     * @return abilityName_.
     */
    std::string GetAbilityName() const;
    /**
     * @brief Set value of abilityName_.
     * @param abilityName Ability name.
     */
    void SetAbilityName(const std::string &abilityName);
    /**
     * @brief Get formName_.
     * @return formName_.
     */
    std::string GetFormName() const;
    /**
     * @brief Set value of formName_.
     * @param formName Form name.
     */   
    void SetFormName(const std::string &formName);
    /**
     * @brief Get jsComponentName_.
     * @return jsComponentName_.
     */
    std::string GetJsComponentName() const;
    /**
     * @brief Set value of jsComponentName_.
     * @param jsComponentName Js component name.
     */   
    void SetJsComponentName(const std::string &jsComponentName);
    /**
     * @brief Get abilityModuleName_.
     * @return abilityModuleName_.
     */
    std::string GetAbilityModuleName() const;
    /**
     * @brief Set value of abilityModuleName_.
     * @param abilityModuleName ability module name_.
     */  
    void SetAbilityModuleName(const std::string &abilityModuleName);
    /**
     * @brief Get specificationId_.
     * @return specificationId_.
     */
    int GetSpecificationId() const;
    /**
     * @brief Set value of specificationId_.
     * @param specificationId Specification id.
     */ 
    void SetSpecificationId(const int specificationId);

    /**
     * @brief Obtains the updageFlag.
     * @return Returns updageFlag.
     */
    bool IsEnableUpdateFlag() const;
    /**
     * @brief Set value of updateFlag_.
     * @param IsEnableUpdateFlag Enable update flag or not.
     */ 
    void SetEnableUpdateFlag(bool IsEnableUpdateFlag);
    /**
     * @brief Get updateDuration_.
     * @return updateDuration_.
     */
    int GetUpdateDuration() const;
    /**
     * @brief Set value of updateDuration_.
     * @param updateDuration Update duration.
     */ 
    void SetUpdateDuration(int updateDuration);
    /**
     * @brief Get scheduledUpdateTime_.
     * @return scheduledUpdateTime_.
     */
    std::string GetScheduledUpdateTime() const;
    /**
     * @brief Set value of scheduledUpdateTime_.
     * @param scheduledUpdateTime Scheduled update time.
     */ 
    void SetScheduledUpdateTime(const std::string &scheduledUpdateTime);
    /**
     * @brief Get hapSourceDirs_.
     * @param dirs Hap source dirs.
     * @return Returns true on success, false on failure.
     */
    bool GetHapSourceDirs(std::vector<std::string> &dirs) const;
    /**
     * @brief Add hap source dir.
     * @param hapSourceDir Hap source dir.
     */
    void AddHapSourceDirs(const std::string &hapSourceDir);
    /**
     * @brief Set value of hapSourceDirs_.
     * @param hapSourceDirs Hap source dirs.
     */
    void SetHapSourceDirs(const std::vector<std::string> &hapSourceDirs);
    /**
     * @brief Obtains the temporaryFlag.
     * @return Returns temporaryFlag.
     */
    bool IsTemporaryForm() const;
    /**
     * @brief Set value of temporaryFlag_.
     * @param temporaryFlag Temporary flag.
     */ 
    void SetTemporaryFlag(bool temporaryFlag);
    /**
     * @brief Obtains the hap source by ability module name.
     * @param moduleName ability module name
     * @return Returns hap source.
     */
    std::string GetHapSourceByModuleName(const std::string &moduleName) const;
    /**
     * @brief Add module info.
     * @param moduleName Module name.
     * @param moduleSourceDir Module source dir.
     */ 
    void AddModuleInfo(const std::string &moduleName, const std::string &moduleSourceDir);
    /**
     * @brief Check if item valid or not.
     * @return Valid or not
     */
    bool IsValidItem() const;
    /**
     * @brief Check if item match or not.
     * @return Match or not
     */
    bool IsMatch(const FormRecord &record) const;
    /**
     * @brief Check if form config same or not.
     * @return Same or not
     */
    bool IsSameFormConfig(const FormRecord &record) const;
    /**
     * @brief Check if visible notify or not.
     * @return visible notify or not
     */
    bool IsFormVisibleNotify() const;
    /**
     * @brief Set value of formVisibleNotify_.
     * @param isFormVisibleNotify visible notify or not.
     */
    void SetFormVisibleNotify(bool isFormVisibleNotify);
private:
    /**
     * @brief Equal or not.
     * @param left left string.
     * @param right right string.
     * @return Equal or not
     */
    bool IsEqual(const std::string &left, const std::string &right);
    
private:
    int64_t formId_ = -1;
    std::string packageName_;
    std::string providerBundleName_;
    std::string hostBundleName_;
    std::string moduleName_;
    std::string abilityName_;
    std::string formName_;
    int32_t specificationId_;
    bool updateFlag_;
    int32_t updateDuration_;
    std::string scheduledUpdateTime_;
    std::vector<std::string> hapSourceDirs_;
    bool temporaryFlag_;
    bool formVisibleNotify_;

    std::string jsComponentName_;
    std::string abilityModuleName_;
    std::unordered_map<std::string, std::string> moduleInfoMap_;
};
} // namespace AppExecFwk
} // namespace OHOS

#endif // FOUNDATION_APPEXECFWK_SERVICES_FORMMGR_INCLUDE_FORM_CONFIG_INFO_H
