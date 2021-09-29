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

#include <cinttypes>
#include <chrono>
#include <ctime>
#include <regex>
#include <sys/time.h>

#include "app_log_wrapper.h"
#include "form_constants.h"
#include "form_util.h"
#include "ohos_account_kits.h"

namespace OHOS {
namespace AppExecFwk {
using namespace std;
using namespace std::chrono;

constexpr int64_t SEC_TO_NANOSEC = 1000000000;
constexpr int64_t SEC_TO_MILLISEC = 1000;
constexpr int64_t MILLISEC_TO_NANOSEC = 1000000;
constexpr int64_t INVALID_UDID_HASH = 0;
/**
 * @brief create want for form.
 * @param formName The name of the form.
 * @param specificationId specification id.
 * @param isTemporaryForm temporary form or not.
 * @param want The want of the form.
 */
void FormUtil::CreateFormWant(const std::string &formName, 
    const int32_t specificationId,  const bool isTemporaryForm, Want &want)
{
    want.SetParam(Constants::PARAM_FORM_NAME_KEY, formName);
    want.SetParam(Constants::PARAM_FORM_DIMENSION_KEY, specificationId);
    want.SetParam(Constants::PARAM_FORM_TEMPORARY_KEY, isTemporaryForm);
}

/**
 * @brief create default want for form.
 * @param want The want of the form..
 * @param uri The uri.
 * @param connectId connect id.
 */
void FormUtil::CreateDefaultFormWant(Want &want, const std::string &uri, const int32_t connectId)
{
    want.SetParam(Constants::FORM_CONNECT_ID, connectId);
    want.SetParam(Constants::FORM_SUPPLY_INFO, uri);
}

/**
 * @brief create udid for form.
 * @return udid.
 */
std::string FormUtil::GenerateUdid()
{
    char buf[256] = {0};
    return buf;
}

/**
 * @brief create form id for form.
 * @param udidHash udid hash
 * @return new form id.
 */
int64_t FormUtil::GenerateFormId(int64_t udidHash)
{
    struct timespec t;
    t.tv_sec = 0;
    t.tv_nsec = 0;
    clock_gettime(CLOCK_REALTIME, &t);

    int64_t elapsedTime { ((t.tv_sec) * SEC_TO_NANOSEC + t.tv_nsec) };
    size_t elapsedHash = std::hash<std::string>()(std::to_string(elapsedTime));
    APP_LOGI("%{public}s, GenerateFormId generate elapsed hash %{public}zu", __func__, elapsedHash);
    uint64_t unsignedudidHash = static_cast<int64_t>(udidHash);
    uint64_t formId = unsignedudidHash | (int32_t)(elapsedHash & 0x000000007fffffffL);
    APP_LOGI("%{public}s, GenerateFormId generate formId %{public}" PRId64 "", __func__, formId);
    return formId;
}

/**
 * @brief padding form id.
 * @param formId The id of the form.
 * @param udidHash udid hash
 * @return new form id.
 */
int64_t FormUtil::PaddingUDIDHash(uint64_t formId, uint64_t udidHash)
{
    // Compatible with int form id.
    if ((formId & 0xffffffff00000000L) == 0) {
        return udidHash | formId;
    }
    
    return formId;
}
/**
 * @brief create udid hash.
 * @param udidHash udid hash.
 * @return Returns true on success, false on failure.
 */
bool FormUtil::GenerateUdidHash(int64_t &udidHash)
{
    APP_LOGI("%{public}s start, udidHash: %{public}" PRId64 "", __func__, udidHash);
    if (udidHash != INVALID_UDID_HASH) {
        return true;
    }
 
    u_int64_t hashId = 0L;
    const int32_t thirtyTwo = 32;
    udidHash = (hashId & 0x0000000000ffffffL) << thirtyTwo;
    if (udidHash < 0) {
        udidHash = 0L;
    }
    APP_LOGI("%{public}s, FormAdapter generate hash %{public}" PRId64 "", __func__, udidHash);
    
    return true;
}
/**
 * @brief Get current system nanosecond.
 * @return Current system nanosecond.
 */
long FormUtil::GetCurrentNanosecond()
{
    struct timespec ts;
    ts.tv_sec = 0;
    ts.tv_nsec = 0;
    clock_gettime(CLOCK_REALTIME, &ts);
    return (ts.tv_sec * SEC_TO_NANOSEC + ts.tv_nsec);
}
/**
 * @brief Get current system millisecond.
 * @return Current system millisecond.
 */
long FormUtil::GetCurrentMillisecond()
{
    struct timespec ts;
    clock_gettime(CLOCK_REALTIME, &ts);
    return (ts.tv_sec * SEC_TO_MILLISEC + ts.tv_nsec / MILLISEC_TO_NANOSEC);
}
/**
 * @brief Get millisecond from tm.
 * @param tmAtTime tm time.
 * @return Millisecond.
 */
long FormUtil::GetMillisecondFromTm(struct tm &tmAtTime)
{
    time_t inputTime = mktime(&tmAtTime);
    if (inputTime == -1) {
        APP_LOGE("%{public}s fail, mktime failed.", __func__);
        return -1;
    }
    system_clock::time_point pointTime = system_clock::from_time_t(inputTime);
    auto timeMilliseconds = chrono::duration_cast<chrono::milliseconds>(pointTime.time_since_epoch());
    return timeMilliseconds.count();
}

/**
* @brief split string.
 * @param in string.
 * @param delim delimiter.
 * @return string list.
 */
std::vector<std::string> FormUtil::StringSplit(const std::string &in, const std::string &delim)
{
    std::regex reg { delim };
    return std::vector<std::string> {
        std::sregex_token_iterator(in.begin(), in.end(), reg, -1),
        std::sregex_token_iterator()
    };
}
}  // namespace AppExecFwk
}  // namespace OHOS