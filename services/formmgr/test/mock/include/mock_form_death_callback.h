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

#ifndef FOUNDATION_APPEXECFWK_SERVICES_MOCK_FORM_DEATH_CALLBACK_H
#define FOUNDATION_APPEXECFWK_SERVICES_MOCK_FORM_DEATH_CALLBACK_H

#include <codecvt>
#include <memory>
#include <singleton.h>
#include <thread_ex.h>
#include <unordered_map>
#include <vector>

#include "app_log_wrapper.h"
#include "event_handler.h"
#include "form_constants.h"
#include "form_callback_interface.h"
#include "form_mgr_stub.h"
#include "form_js_info.h"
#include "form_provider_data.h"
#include "ipc_types.h"
#include "iremote_object.h"

namespace OHOS {
namespace AppExecFwk {
/**
 * @class MockFormDeathCallback.
 * The MockFormDeathCallback for form mgr test.
 */
class MockFormDeathCallback : public FormCallbackInterface {
public:
    MockFormDeathCallback() = default;
    virtual ~MockFormDeathCallback() = default;

    void OnDeathReceived()
    {
        APP_LOGI("%{public}s called.", __func__);
    }

    /**
     * @brief Update form.
     *
     * @param formJsInfo Indicates the obtained {@code FormJsInfo} instance.
     */
    void ProcessFormUpdate(const FormJsInfo &formJsInfo)
    {
        APP_LOGI("%{public}s called.", __func__);
    }

    /**
     * @brief Uninstall form.
     *
     * @param formId Indicates the ID of the form to uninstall.
     */
    void ProcessFormUninstall(const int64_t formId)
    {
        APP_LOGI("%{public}s called.", __func__);
    }
};
}  // namespace AppExecFwk
}  // namespace OHOS
#endif  // FOUNDATION_APPEXECFWK_SERVICES_MOCK_FORM_DEATH_CALLBACK_H
