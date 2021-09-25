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

#ifndef FOUNDATION_APPEXECFWK_SERVICES_BUNDLEMGR_INCLUDE_BUNDLE_PERMISSIONS_CHANGED_MONITOR_H
#define FOUNDATION_APPEXECFWK_SERVICES_BUNDLEMGR_INCLUDE_BUNDLE_PERMISSIONS_CHANGED_MONITOR_H
#include <future>
#include "bundle_mgr_host.h"
#include "bundle_data_mgr.h"
#include "common_event_manager.h"
#include "common_event_support.h"
#include "common_event_subscriber.h"
#include "common_event_subscribe_info.h"
#include "bundle_util.h"

#include "bundle_data_mgr.h"
#include "json_serializer.h"
#include "app_log_wrapper.h"


#include "bundle_parser.h"
#include "installd_client.h"
#include "bundle_permission_mgr.h"

namespace OHOS {
namespace AppExecFwk {
class BundlePermissionsChangedMonitor : public EventFwk::CommonEventSubscriber {
public:
    BundlePermissionsChangedMonitor(const std::shared_ptr<BundleDataMgr> &dataMgr,
                const EventFwk::CommonEventSubscribeInfo& sp):CommonEventSubscriber(sp)
    {
        dataMgr_ = dataMgr;
    }
    ~BundlePermissionsChangedMonitor() {
        if(!dataMgr_) {
            dataMgr_.reset();
        }
    }
    void OnReceiveEvent(const EventFwk::CommonEventData &data)
    {
        OHOS::AAFwk::Want want = data.GetWant();
        std::string action = want.GetAction();
        int32_t uid = data.GetCode();
        if (dataMgr_ != nullptr && uid >=0) {
            dataMgr_->NotifyPermissionsChanged(uid);
        }
    }
private:
    std::shared_ptr<BundleDataMgr> dataMgr_;
};
}  // namespace AppExecFwk
}  // namespace OHOS
#endif  // FOUNDATION_APPEXECFWK_SERVICES_BUNDLEMGR_INCLUDE_BUNDLE_MONITOR_H