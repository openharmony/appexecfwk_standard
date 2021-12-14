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

#ifndef FOUNDATION_APPEXECFWK_SERVICES_APPMGR_INCLUDE_APP_MGR_SERVICE_EVENT_HANDLER_H
#define FOUNDATION_APPEXECFWK_SERVICES_APPMGR_INCLUDE_APP_MGR_SERVICE_EVENT_HANDLER_H

#include "event_handler.h"

namespace OHOS {
namespace AppExecFwk {
class AppMgrServiceInner;

class AMSEventHandler : public EventHandler {
public:
    AMSEventHandler(const std::shared_ptr<EventRunner> &runner, const std::weak_ptr<AppMgrServiceInner> &appMgr);
    virtual ~AMSEventHandler() override;

    virtual void ProcessEvent(const InnerEvent::Pointer &event) override;

    static constexpr uint32_t TERMINATE_ABILITY_TIMEOUT_MSG = 0;
    static constexpr uint32_t TERMINATE_APPLICATION_TIMEOUT_MSG = 1;
    static constexpr uint32_t ADD_ABILITY_STAGE_INFO_TIMEOUT_MSG = 2;
    static constexpr uint32_t TERMINATE_ABILITY_TIMEOUT = 500;
    static constexpr uint32_t TERMINATE_APPLICATION_TIMEOUT = 500;
    static constexpr uint32_t ADD_ABILITY_STAGE_INFO_TIMEOUT = 3000; // ms
private:
    std::weak_ptr<AppMgrServiceInner> appMgr_;
};
}  // namespace AppExecFwk
}  // namespace OHOS
#endif  // FOUNDATION_APPEXECFWK_SERVICES_APPMGR_INCLUDE_APP_MGR_SERVICE_EVENT_HANDLER_H
