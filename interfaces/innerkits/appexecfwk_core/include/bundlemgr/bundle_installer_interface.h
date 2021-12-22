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

#ifndef FOUNDATION_APPEXECFWK_INTERFACES_INNERKITS_APPEXECFWK_CORE_INCLUDE_BUNDLEMGR_BUNDLE_INSTALLER_INTERFACE_H
#define FOUNDATION_APPEXECFWK_INTERFACES_INNERKITS_APPEXECFWK_CORE_INCLUDE_BUNDLEMGR_BUNDLE_INSTALLER_INTERFACE_H

#include <vector>

#include "status_receiver_interface.h"
#include "install_param.h"

namespace OHOS {
namespace AppExecFwk {
class IBundleInstaller : public IRemoteBroker {
public:
    DECLARE_INTERFACE_DESCRIPTOR(u"ohos.appexecfwk.BundleInstaller");
    /**
     * @brief Installs an application, the final result will be notified from the statusReceiver object.
     * @attention Notice that the bundleFilePath should be an absolute path.
     * @param bundleFilePath Indicates the path for storing the ohos Ability Package (HAP) of the application
     *                       to install or update.
     * @param installParam Indicates the install parameters.
     * @param statusReceiver Indicates the callback object that using for notifing the install result.
     * @return Returns true if this function is successfully called; returns false otherwise.
     */
    virtual bool Install(const std::string &bundleFilePath, const InstallParam &installParam,
        const sptr<IStatusReceiver> &statusReceiver) = 0;
    /**
     * @brief Installs an application by bundleName, the final result will be notified from the statusReceiver object.
     * @param bundleName Indicates the bundleName of the application to install.
     * @param installParam Indicates the install parameters.
     * @param statusReceiver Indicates the callback object that using for notifing the install result.
     * @return Returns true if this function is successfully called; returns false otherwise.
     */
    virtual bool Recover(const std::string &bundleName, const InstallParam &installParam,
        const sptr<IStatusReceiver> &statusReceiver) = 0;

    /**
     * @brief Installs multiple haps, the final result will be notified from the statusReceiver object.
     * @attention Notice that the bundleFilePath should be an string vector of absolute paths.
     * @param bundleFilePaths Indicates the paths for storing the ohos Ability Packages (HAP) of the application
     *                       to install or update.
     * @param installParam Indicates the install parameters.
     * @param statusReceiver Indicates the callback object that using for notifing the install result.
     * @return Returns true if this function is successfully called; returns false otherwise.
     */
    virtual bool Install(const std::vector<std::string> &bundleFilePaths, const InstallParam &installParam,
        const sptr<IStatusReceiver> &statusReceiver) = 0;

    /**
     * @brief Uninstalls an application, the result will be notified from the statusReceiver object.
     * @param bundleName Indicates the bundle name of the application to uninstall.
     * @param installParam Indicates the uninstall parameters.
     * @param statusReceiver Indicates the callback object that using for notifing the uninstall result.
     * @return Returns true if this function is successfully called; returns false otherwise.
     */
    virtual bool Uninstall(const std::string &bundleName, const InstallParam &installParam,
        const sptr<IStatusReceiver> &statusReceiver) = 0;

    /**
     * @brief Uninstalls a module in an application, the result will be notified from the statusReceiver object.
     * @param bundleName Indicates the bundle name of the module to uninstall.
     * @param modulePackage Indicates the module package of the module to uninstall.
     * @param installParam Indicates the uninstall parameters.
     * @param statusReceiver Indicates the callback object that using for notifing the uninstall result.
     * @return Returns true if this function is successfully called; returns false otherwise.
     */
    virtual bool Uninstall(const std::string &bundleName, const std::string &modulePackage,
        const InstallParam &installParam, const sptr<IStatusReceiver> &statusReceiver) = 0;

    enum class Message {
        INSTALL = 0,
        INSTALL_MULTIPLE_HAPS,
        UNINSTALL,
        UNINSTALL_MODULE,
        RECOVER,
    };
};

#define PARCEL_WRITE_INTERFACE_TOKEN(parcel, token)                                 \
    do {                                                                            \
        bool ret = parcel.WriteInterfaceToken((token));                             \
        if (!ret) {                                                                 \
            APP_LOGE("fail to write interface token into the parcel!");             \
            return false;                                                           \
        }                                                                           \
    } while (0)

#define PARCEL_WRITE(parcel, type, value)                                           \
    do {                                                                            \
        bool ret = parcel.Write##type((value));                                     \
        if (!ret) {                                                                 \
            APP_LOGE("fail to write parameter into the parcel!");                   \
            return false;                                                           \
        }                                                                           \
    } while (0)
}  // namespace AppExecFwk
}  // namespace OHOS
#endif  // FOUNDATION_APPEXECFWK_INTERFACES_INNERKITS_APPEXECFWK_CORE_INCLUDE_BUNDLEMGR_BUNDLE_INSTALLER_INTERFACE_H