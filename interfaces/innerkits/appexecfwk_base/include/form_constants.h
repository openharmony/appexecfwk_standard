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

#ifndef FOUNDATION_APPEXECFWK_INTERFACES_INNERKITS_APPEXECFWK_BASE_INCLUDE_FORM_CONSTANTS_H
#define FOUNDATION_APPEXECFWK_INTERFACES_INNERKITS_APPEXECFWK_BASE_INCLUDE_FORM_CONSTANTS_H

#include <map>
#include <string>

namespace OHOS {
namespace AppExecFwk {
namespace Constants {
    const std::string PERMISSION_REQUIRE_FORM = "ohos.permission.REQUIRE_FORM";
    const std::string PARAM_FORM_IDENTITY_KEY = "ohos.extra.param.key.form_identity";
    const std::string PARAM_FORM_CALLING_IDENTITY_KEY = "ohos.extra.param.key.form_calling_identity";
    const std::string PARAM_MODULE_NAME_KEY = "ohos.extra.param.key.module_name";
    const std::string PARAM_FORM_NAME_KEY = "ohos.extra.param.key.form_name";
    const std::string PARAM_FORM_DIMENSION_KEY = "ohos.extra.param.key.form_dimension";
    const std::string PARAM_MESSAGE_KEY = "ohos.extra.param.key.message";
    const std::string PARAM_FORM_TEMPORARY_KEY = "ohos.extra.param.key.form_temporary";
    const int32_t ONE_HOST_MAX_FORM_SIZE = 256;
    const std::string RECREATE_FORM_KEY = "ohos.extra.param.key.recreate";
    const std::string PARAM_FORM_CUSTOMIZE_KEY = "ohos.extra.param.key.form_customize";
    const std::string PARAM_FORM_ORIENTATION_KEY = "ohos.extra.param.key.form_orientation";
    const int32_t ORIENTATION_PORTRAIT = 1;
    const int32_t ORIENTATION_LANDSCAPE = 2;
    const std::string PARAM_FORM_ABILITY_NAME_KEY = "abilityName";

    const std::string KEY_IS_TIMER = "isTimerRefresh";
    const std::string SYSTEM_PARAM_FORM_UPDATE_TIME = "persist.sys.fms.form.update.time";
    const std::string SYSTEM_PARAM_FORM_REFRESH_MIN_TIME = "persist.sys.fms.form.refresh.min.time";    
    const std::string ACTION_UPDATEATTIMER = "form_update_at_timer";
    const std::string KEY_WAKEUP_TIME = "wakeUpTime";
    const std::string KEY_ACTION_TYPE = "form_update_action_type";
    const int TYPE_RESET_LIMIT = 1;
    const int TYPE_STATIC_UPDATE = 2;
    const int TYPE_DYNAMIC_UPDATE = 3;
    const long ABS_REFRESH_MS = 2500;
    
    // The form events type which means that the form becomes visible.
    const int32_t FORM_VISIBLE = 1;
    // The form events type which means that the form becomes invisible.
    const int32_t FORM_INVISIBLE = 2;

    // The default user id
    const int32_t DEFAULT_USER_ID = 0;

    // The max uid of system app.
    const int32_t MAX_SYSTEM_APP_UID = 10000;

    const int MAX_HOUR = 23;
    const int MAX_MININUTE = 59;
    const int MIN_TIME = 0;
    const int HOUR_PER_DAY = 24;
    const int MIN_PER_HOUR = 60;
    const long TIME_1000 = 1000;
    const long TIME_1000000 = 1000000;
    const long TIME_CONVERSION = 30 * 60 * TIME_1000;
    const int MIN_CONFIG_DURATION = 1; // 1 means 30 min
    const int MAX_CONFIG_DURATION = 2 * 24 * 7; // one week
    const long MIN_PERIOD = MIN_CONFIG_DURATION * TIME_CONVERSION; // 30 min in ms unit
    const long MAX_PERIOD = MAX_CONFIG_DURATION * TIME_CONVERSION; // 1 week in ms unit
    const long ABS_TIME = 5 * TIME_1000; // 5s abs time
    const int WORK_POOL_SIZE = 4;
    const std::string TIME_DELIMETER = ":";
    const int UPDATE_AT_CONFIG_COUNT = 2;

    const int LIMIT_COUNT = 50;
    const int MIN_NEXT_TIME = 300; // seconds

    // The max retry times of reconnection.
    const int32_t MAX_RETRY_TIME = 30;
    // The time interval between reconnections(milliseconds).
    const int32_t SLEEP_TIME = 1000;

    const int64_t MS_PER_SECOND = 1000;

    // form host bundlename
    const std::string PARAM_FORM_HOST_BUNDLENAME_KEY = "form_host_bundlename";

    // form manager service bundlename
    const std::string PARAM_FORM_MANAGER_SERVICE_BUNDLENAME_KEY = "form_manager_service_bundlename";

    // PROVIDER_FLAG false:ProviderFormInfo is null;true: ProviderFormInfo not null
    const std::string PROVIDER_FLAG = "provider_flag";
    const std::string FORM_CONNECT_ID = "form_connect_id";
    const std::string FORM_SUPPLY_INFO = "form_supply_info";

    // the delimiter between bundleName and abilityName
    const std::string NAME_DELIMITER = "::";

    const std::string PARAM_FORM_ADD_COUNT = "form.add.count";

    const size_t MAX_LAYOUT = 8;
    const std::map<int32_t, std::string> DIMENSION_MAP = {
        {1, "1*2"},
        {2, "2*2"},
        {3, "2*4"},
        {4, "4*4"},
    };
    const int MAX_FORMS = 512;
    const int MAX_RECORD_PER_APP = 256;
    const int MAX_TEMP_FORMS = 256;
    const int MAX_FORM_DATA_SIZE = 1024;

    const int NOT_IN_RECOVERY = 0;
    const int RECOVER_FAIL = 1;
    const int IN_RECOVERING = 2;
    
    const int FLAG_HAS_OBJECT = 1;
    const int FLAG_NO_OBJECT = 0;

    const int DEATH_RECIPIENT_FLAG = 0;
    const int MAX_VISIBLE_NOTIFY_LIST = 32;
    
    const std::string ACQUIRE_TYPE = "form_acquire_form";
    const int ACQUIRE_TYPE_CREATE_FORM = 1;
    const int ACQUIRE_TYPE_RECREATE_FORM = 2;

    const int DELETE_FORM = 3;
    const int RELEASE_FORM = 8;
    const int RELEASE_CACHED_FORM = 9;

    const int64_t INVALID_UDID_HASH = 0L;

    enum class FormMgrMessage {
        // ipc id 1-1000 for kit
        // ipc id 1001-2000 for DMS
        // ipc id 2001-3000 for tools
        // ipc id for add form (3001)
        FORM_MGR_ADD_FORM = 3001,

        // ipc id for delete form (3002)
        FORM_MGR_DELETE_FORM,

        // ipc id for form done release form (3003)
        FORM_MGR_RELEASE_FORM,

        // ipc id for connecting update form (3004)
        FORM_MGR_UPDATE_FORM,

        // ipc id for form visible notify (3005)
        FORM_MGR_NOTIFY_FORM_VISIBLE,

        // ipc id for form invisible notify (3006)
        FORM_MGR_NOTIFY_FORM_INVISIBLE,

        // ipc id for refreshing data cache (3007)
        FORM_MGR_CAST_TEMP_FORM,
               
        // ipc id 2001-3000 for tools
        // ipc id for dumping state (2001)
        FORM_MGR_DUMP_STATE = 2001,
    };
}  // namespace Constants
}  // namespace AppExecFwk
}  // namespace OHOS
#endif  // FOUNDATION_APPEXECFWK_INTERFACES_INNERKITS_APPEXECFWK_BASE_INCLUDE_FORM_CONSTANTS_H
