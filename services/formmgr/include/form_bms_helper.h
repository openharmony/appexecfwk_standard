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

#ifndef FOUNDATION_APPEXECFWK_SERVICES_FORMMGR_INCLUDE_FORM_BMS_HELPER_H
#define FOUNDATION_APPEXECFWK_SERVICES_FORMMGR_INCLUDE_FORM_BMS_HELPER_H

#include <singleton.h>
#include "ability_connect_callback_interface.h"
#include "ability_manager_interface.h"
#include "bundle_info.h"
#include "bundle_mgr_interface.h"

namespace OHOS {
namespace AppExecFwk {
using Want = OHOS::AAFwk::Want;

/**
 * @class FormBmsHelper
 * Bms helpler.
 */
class FormBmsHelper final : public DelayedRefSingleton<FormBmsHelper> {
    DECLARE_DELAYED_REF_SINGLETON(FormBmsHelper)

public:
    DISALLOW_COPY_AND_MOVE(FormBmsHelper);

    /**
     * @brief Notify module removable.
     * @param bundleName Provider ability bundleName.
     * @param moduleName Provider ability moduleName.
     */
    void NotifyModuleRemovable(const std::string &bundleName, const std::string &moduleName);
    /**
     * @brief Notify module not removable.
     * @param bundleName Provider ability bundleName.
     * @param moduleName Provider ability moduleName.
     */
    void NotifyModuleNotRemovable(const std::string &bundleName, const std::string &moduleName) const;

    /**
     * @brief Acquire a bundle manager, if it not existed,
     * @return returns the bundle manager ipc object, or nullptr for failed.
     */
    sptr<IBundleMgr> GetBundleMgr();

    /**
     * @brief Add the bundle manager instance for debug.
     * @param bundleManager the bundle manager ipc object.
     */
    void SetBundleManager(const sptr<IBundleMgr> &bundleManager);

private:
    /**
     * @brief Generate module key.
     * @param bundleName Provider ability bundleName.
     * @param moduleName Provider ability moduleName.
     * @return Module key.
     */
    std::string GenerateModuleKey(const std::string &bundleName, const std::string &moduleName) const;

private:
    sptr<IBundleMgr> iBundleMgr_ = nullptr;
};
}  // namespace AppExecFwk
}  // namespace OHOS

#endif // FOUNDATION_APPEXECFWK_SERVICES_FORMMGR_INCLUDE_FORM_BMS_HELPER_H
