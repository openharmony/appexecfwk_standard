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

#include "abs_shared_result_set.h"
#include "data_ability_predicates.h"
#include "values_bucket.h"

#include "app_log_wrapper.h"
#include "verify_act_page_ability.h"

namespace OHOS {
namespace AppExecFwk {
void VerifyActPageAbility::OnStart(const Want &want)
{
    APP_LOGI("----------VerifyActPageAbility::onStart begin----------");
    // Test DataAbility
    EventFwk::MatchingSkills MatchingSkillsDataAbility;
    MatchingSkillsDataAbility.AddEvent("Test_DataAbility");
    EventFwk::CommonEventSubscribeInfo subscriberInfoDataAbility(MatchingSkillsDataAbility);
    subscriberDataAbility = std::make_shared<CommentDataAbilityTest>(subscriberInfoDataAbility);
    EventFwk::CommonEventManager::SubscribeCommonEvent(subscriberDataAbility);
    subscriberDataAbility->DataAbility_ = Ability::GetContext();

    APP_LOGI("----------VerifyActPageAbility::onStart end----------");
    Ability::OnStart(want);
}
void VerifyActPageAbility::OnStop()
{
    APP_LOGI("----------VerifyActPageAbility::onStop begin----------");
    if (subscriberDataAbility != nullptr) {
        EventFwk::CommonEventManager::UnSubscribeCommonEvent(subscriberDataAbility);
    }
    APP_LOGI("----------VerifyActPageAbility::onStop end----------");
    APP_LOGI("----------VerifyActPageAbility::onStop TimerThreadDestoryBegin----------");
    APP_LOGI("----------VerifyActPageAbility::onStop TimerThreadDestoryEnd----------");
    Ability::OnStop();
}
void VerifyActPageAbility::OnActive()
{
    APP_LOGI("----------VerifyActPageAbility::OnActive begin----------");
    APP_LOGI("----------VerifyActPageAbility::OnActive end----------");
    Ability::OnActive();
}
void VerifyActPageAbility::OnInactive()
{
    APP_LOGI("----------VerifyActPageAbility::OnInactive begin----------");
    APP_LOGI("----------VerifyActPageAbility::OnInactive end----------");
    Ability::OnInactive();
}
void VerifyActPageAbility::OnBackground()
{
    APP_LOGI("----------VerifyActPageAbility::OnBackground begin----------");
    APP_LOGI("----------VerifyActPageAbility::OnBackground end----------");
    Ability::OnBackground();
}
void VerifyActPageAbility::OnForeground(const Want &want)
{
    APP_LOGI("----------VerifyActPageAbility::OnForeground begin----------");
    APP_LOGI("----------VerifyActPageAbility::OnForeground end----------");
    Ability::OnForeground(want);
}
void CommentDataAbilityTest::OnReceiveEvent(const EventFwk::CommonEventData &data)
{
    APP_LOGI("----------OnReceiveEvent---------DataAbility_%{public}ld", DataAbility_.use_count());
    auto dataAbility = DataAbility_.lock();
    if (dataAbility == nullptr) {
        return;
    }
    APP_LOGI("----------OnReceiveEvent----------dataAbility%{public}ld", dataAbility.use_count());
    std::shared_ptr<Uri> uri = std::make_shared<Uri>("dataability:///com.ix.VerifyActDataAbility");
    std::shared_ptr<DataAbilityHelper> helper = DataAbilityHelper::Creator(dataAbility, uri, false);
    if (helper != nullptr) {
        APP_LOGI("-----------OnReceiveEvent helper isn't null-----------");
        Uri uri_DataAbility("dataability:///com.ix.VerifyActDataAbility");
        APP_LOGI("-----------OnReceiveEvent data.GetCode() ====%{public}d ---------", data.GetCode());
        switch (data.GetCode()) {
            case INSERT: {
                NativeRdb::ValuesBucket valueBucket;
                int result = helper->Insert(uri_DataAbility, valueBucket);
                APP_LOGI("-----------insertTest::OnReceiveEvent result = %{public}d", result);
                break;
            }
            case DELETE: {
                NativeRdb::DataAbilityPredicates dataAbilityPredicate("dumy Predicates");
                int result = helper->Delete(uri_DataAbility, dataAbilityPredicate);
                APP_LOGI("-----------deleteTest::OnReceiveEvent result = %{public}d", result);
                break;
            }
            default:
                break;
        }
    }
}
REGISTER_AA(VerifyActPageAbility)
}  // namespace AppExecFwk
}  // namespace OHOS