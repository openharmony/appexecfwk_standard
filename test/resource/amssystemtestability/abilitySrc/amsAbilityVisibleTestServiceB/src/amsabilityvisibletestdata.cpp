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

#include "amsabilityvisibletestdata.h"
#include "app_log_wrapper.h"
#include "common_event.h"
#include "common_event_manager.h"
using namespace OHOS::EventFwk;

namespace OHOS {
namespace AppExecFwk {

const static int defenvntCode = 1;

AmsAbilityVisibleTestData::~AmsAbilityVisibleTestData()
{}

void AmsAbilityVisibleTestData::OnStart(const Want &want)
{
    APP_LOGI("AmsAbilityVisibleTestData::OnStart");
    pageAbilityEvent.SubscribeEvent(STEventName::g_eventList, shared_from_this());
    Ability::OnStart(want);
    std::string eventData = GetAbilityName() + STEventName::g_abilityStateOnStart;
    pageAbilityEvent.PublishEvent(STEventName::g_eventName, pageAbilityEvent.GetOnStartCount(), eventData);
}

int AmsAbilityVisibleTestData::Insert(const Uri &uri, const NativeRdb::ValuesBucket &value)
{
    APP_LOGI("AmsAbilityVisibleTestData::Insert");
    int result = Ability::Insert(uri, value);
    std::string eventData = GetAbilityName() + STEventName::g_abilityStateInsert;
    pageAbilityEvent.PublishEvent(STEventName::g_eventName, defenvntCode, eventData);
    return result;
}

int AmsAbilityVisibleTestData::Delete(const Uri &uri, const NativeRdb::DataAbilityPredicates &predicates)
{
    APP_LOGI("AmsAbilityVisibleTestData::Delete");
    int result = Ability::Delete(uri, predicates);
    std::string eventData = GetAbilityName() + STEventName::g_abilityStateDelete;
    pageAbilityEvent.PublishEvent(STEventName::g_eventName, defenvntCode, eventData);
    return result;
}

int AmsAbilityVisibleTestData::Update(
    const Uri &uri, const NativeRdb::ValuesBucket &value, const NativeRdb::DataAbilityPredicates &predicates)
{
    APP_LOGI("AmsAbilityVisibleTestData::Update");
    int result = Ability::Update(uri, value, predicates);
    std::string eventData = GetAbilityName() + STEventName::g_abilityStateUpdate;
    pageAbilityEvent.PublishEvent(STEventName::g_eventName, defenvntCode, eventData);
    return result;
}

std::shared_ptr<NativeRdb::AbsSharedResultSet> AmsAbilityVisibleTestData::Query(
    const Uri &uri, const std::vector<std::string> &columns, const NativeRdb::DataAbilityPredicates &predicates)
{
    APP_LOGI("AmsAbilityVisibleTestData::Query");
    std::shared_ptr<NativeRdb::AbsSharedResultSet> result = Ability::Query(uri, columns, predicates);
    std::string eventData = GetAbilityName() + STEventName::g_abilityStateQuery;
    pageAbilityEvent.PublishEvent(STEventName::g_eventName, defenvntCode, eventData);
    return result;
}

std::vector<std::string> AmsAbilityVisibleTestData::GetFileTypes(const Uri &uri, const std::string &mimeTypeFilter)
{
    APP_LOGI("AmsAbilityVisibleTestData::GetFileTypes");
    std::vector<std::string> result = Ability::GetFileTypes(uri, mimeTypeFilter);
    std::string eventData = GetAbilityName() + STEventName::g_abilityStateGetFileTypes;
    pageAbilityEvent.PublishEvent(STEventName::g_eventName, defenvntCode, eventData);
    return result;
}

int AmsAbilityVisibleTestData::OpenFile(const Uri &uri, const std::string &mode)
{
    APP_LOGI("AmsAbilityVisibleTestData::OpenFile");
    int result = Ability::OpenFile(uri, mode);
    std::string eventData = GetAbilityName() + STEventName::g_abilityStateOpenFile;
    pageAbilityEvent.PublishEvent(STEventName::g_eventName, defenvntCode, eventData);
    return result;
}

REGISTER_AA(AmsAbilityVisibleTestData);
}  // namespace AppExecFwk
}  // namespace OHOS