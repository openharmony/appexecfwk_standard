/*
 * Copyright (c) 2022 Huawei Device Co., Ltd.
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

#ifndef FOUNDATION_APPEXECFWK_SERVICES_BUNDLE_SANDBOX_EXCEPTION_HANDLE_H
#define FOUNDATION_APPEXECFWK_SERVICES_BUNDLE_SANDBOX_EXCEPTION_HANDLE_H

#include "bundle_data_storage_database.h"

namespace OHOS {
namespace AppExecFwk {
class BundleSandboxExceptionHandler final {
public:
    explicit BundleSandboxExceptionHandler(const std::shared_ptr<BundleDataStorageDatabase> &dataStorage);
    ~BundleSandboxExceptionHandler();

    /**
     * @brief to remove the sandbox data dir when the bms service inits
     * @param bundleName indicates the bundle name of the application.
     */
    void RemoveSandboxAppDataDir(const std::string &bundleName);

private:
    static void ScanDataDir(const std::string &bundleName);
    static void RemoveDataDir(const std::string &bundleName, const std::string &pathDir);

    std::weak_ptr<BundleDataStorageDatabase> dataStorage_;
};
}  // namespace AppExecFwk
}  // namespace OHOS
#endif // FOUNDATION_APPEXECFWK_SERVICES_BUNDLE_SANDBOX_EXCEPTION_HANDLE_H

