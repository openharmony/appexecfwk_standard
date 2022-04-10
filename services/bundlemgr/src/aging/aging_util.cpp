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

#include "aging/aging_util.h"

#include "app_log_wrapper.h"
#include "aging/aging_bundle_info.h"
#include "aging/aging_constants.h"
#include "aging/aging_request.h"

namespace OHOS {
namespace AppExecFwk {
void AgingUtil::SortAgingBundles(std::vector<AgingBundleInfo> &bundles)
{
    sort(bundles.begin(), bundles.end(), SortTwoAgingBundleinfos);
}
bool AgingUtil::SortTwoAgingBundleinfos(AgingBundleInfo &bundle1, AgingBundleInfo &bundle2)
{
    int64_t currentTimeMs = GetNowSysTimeMs();
    int64_t unusedTime = GetUnusedTimeMsBaseOnCurrentTime(currentTimeMs, AgingConstants::TIME_10_DAYS);
    if (bundle1.GetRecentlyUsedTime() > unusedTime && bundle2.GetRecentlyUsedTime() > unusedTime) {
        return bundle1.GetDataBytes() > bundle2.GetDataBytes();
    } else if (bundle1.GetRecentlyUsedTime() > unusedTime && bundle2.GetRecentlyUsedTime() < unusedTime) {
        return false;
    } else if (bundle1.GetRecentlyUsedTime() < unusedTime && bundle2.GetRecentlyUsedTime() > unusedTime) {
        return true;
    } else if (bundle1.GetRecentlyUsedTime() < unusedTime && bundle2.GetRecentlyUsedTime() < unusedTime) {
        return bundle1.GetRecentlyUsedTime()  < bundle2.GetRecentlyUsedTime();
    }
    return false;
}

int64_t AgingUtil::GetUnusedTimeMsBaseOnCurrentTime(int64_t currentTimeMs, int32_t days)
{
    return currentTimeMs - days * AgingRequest::GetOneDayTimeMs();
}
}  //  namespace AppExecFwk
}  //  namespace OHOS