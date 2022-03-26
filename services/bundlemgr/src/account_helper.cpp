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

#include "account_helper.h"

#include "app_log_wrapper.h"

#ifdef ACCOUNT_ENABLE
#include "os_account_manager.h"
#endif

namespace OHOS {
namespace AppExecFwk {
int AccountHelper::IsOsAccountExists(const int id, bool &isOsAccountExists)
{
#ifdef ACCOUNT_ENABLE
    return AccountSA::OsAccountManager::IsOsAccountExists(id, isOsAccountExists);
#else
    APP_LOGI("ACCOUNT_ENABLE is false");
    // ACCOUNT_ENABLE is false, do nothing and return -1.
    return -1;
#endif
}
}  // namespace AppExecFwk
}  // namespace OHOS
