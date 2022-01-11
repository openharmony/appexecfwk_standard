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

#ifndef FOUNDATION_APPEXECFWK_SERVICES_BUNDLEMGR_INCLUDE_BUNDLE_PROMISE_H
#define FOUNDATION_APPEXECFWK_SERVICES_BUNDLEMGR_INCLUDE_BUNDLE_PROMISE_H

#include <future>

#include "app_log_wrapper.h"

namespace OHOS {
namespace AppExecFwk {
class BundlePromise {
public:
    BundlePromise() = default;
    ~BundlePromise() = default;
    /**
     * @brief Notify all tasks has executed finished.
     * @return
     */
    void NotifyAllTasksExecuteFinished()
    {
        APP_LOGD("Notify all tasks has executed finished.");
        promise_.set_value();
    }
    /**
     * @brief Wait for all tasks execute.
     * @return
     */
    void WaitForAllTasksExecute()
    {
        APP_LOGD("Wait for all tasks execute.");
        promise_.get_future().get();
    }
private:
    std::promise<void> promise_;
};
}  // namespace AppExecFwk
}  // namespace OHOS
#endif  // FOUNDATION_APPEXECFWK_SERVICES_BUNDLEMGR_INCLUDE_BUNDLE_PROMISE_H
