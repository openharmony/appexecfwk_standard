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

#ifndef FOUNDATION_APPEXECFWK_INTERFACES_INNERKITS_APPEXECFWK_CORE_INCLUDE_BUNDLEMGR_BUNDLE_MGR_CLIENT_H
#define FOUNDATION_APPEXECFWK_INTERFACES_INNERKITS_APPEXECFWK_CORE_INCLUDE_BUNDLEMGR_BUNDLE_MGR_CLIENT_H

#include "appexecfwk_errors.h"
#include "iremote_object.h"
#include "singleton.h"

namespace OHOS {
namespace AppExecFwk {
class BundleMgrClient : public DelayedSingleton<BundleMgrClient> {
public:
    BundleMgrClient();
    virtual ~BundleMgrClient();

    bool GetBundleNameForUid(const int uid, std::string &bundleName);

private:
    ErrCode Connect();

private:
    std::mutex mutex_;
    sptr<IRemoteObject> remoteObject_;
};
}  // namespace AppExecFwk
}  // namespace OHOS

#endif  // FOUNDATION_APPEXECFWK_INTERFACES_INNERKITS_APPEXECFWK_CORE_INCLUDE_BUNDLEMGR_BUNDLE_MGR_CLIENT_H
