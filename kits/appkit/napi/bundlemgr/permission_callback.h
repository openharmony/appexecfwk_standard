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

#ifndef PERMISSION_CALLBACK_H
#define PERMISSION_CALLBACK_H

#include <future>
#include "napi/native_api.h"
#include "napi/native_node_api.h"

#include "nocopyable.h"
#include "on_permission_changed_callback_host.h"

struct CallbackInfo {
    napi_env env;
    napi_ref callback = 0;
    int32_t uid;
};

class PermissionCallback : public OHOS::AppExecFwk::OnPermissionChangedCallbackHost {
public:
    PermissionCallback(napi_env env, napi_ref callback);
    virtual ~PermissionCallback();
    virtual void OnChanged(const int32_t uid) override;


private:
    napi_env env_;
    napi_ref callback_;
    DISALLOW_COPY_AND_MOVE(PermissionCallback);
};

#endif  // PERMISSION_CALLBACK_H