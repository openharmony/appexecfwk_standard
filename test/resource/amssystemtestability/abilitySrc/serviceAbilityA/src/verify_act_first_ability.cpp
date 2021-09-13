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

#include "verify_act_first_ability.h"
#include "app_log_wrapper.h"

namespace OHOS {
namespace AppExecFwk {
void AbilityConnectionActFirst::OnAbilityConnectDone(
    const AppExecFwk::ElementName &element, const sptr<IRemoteObject> &remoteObject, int resultCode)
{
    APP_LOGI("AbilityConnectionActFirst OnAbilityConnectDone resultCode: %{public}d", resultCode);
    APP_LOGI("AbilityConnectionActFirst OnAbilityConnectDone %{public}s", element.GetAbilityName().c_str());
    APP_LOGI("AbilityConnectionActFirst OnAbilityConnectDone %{public}p", remoteObject.GetRefPtr());
}

void AbilityConnectionActFirst::OnAbilityDisconnectDone(const AppExecFwk::ElementName &element, int resultCode)
{
    APP_LOGI("AbilityConnectionActFirst OnAbilityDisconnectDone resultCode: %{public}d", resultCode);
    APP_LOGI("AbilityConnectionActFirst OnAbilityDisconnectDone : %{public}s", element.GetAbilityName().c_str());
}
void VerifyIOAbilityLifecycleCallbacks::OnAbilityStart(const std::shared_ptr<Ability> &ability)
{
    APP_LOGI("----------VerifyIOAbilityLifecycleCallbacks::OnAbilityStart called. AbilityName is %{public}s----------",
        ability->GetAbilityName().c_str());
}

void VerifyIOAbilityLifecycleCallbacks::OnAbilityInactive(const std::shared_ptr<Ability> &ability)
{
    APP_LOGI(
        "----------VerifyIOAbilityLifecycleCallbacks::OnAbilityInactive called. AbilityName is %{public}s----------",
        ability->GetAbilityName().c_str());
}

void VerifyIOAbilityLifecycleCallbacks::OnAbilityBackground(const std::shared_ptr<Ability> &ability)
{
    APP_LOGI(
        "----------VerifyIOAbilityLifecycleCallbacks::OnAbilityBackground called. AbilityName is %{public}s----------",
        ability->GetAbilityName().c_str());
}

void VerifyIOAbilityLifecycleCallbacks::OnAbilityForeground(const std::shared_ptr<Ability> &ability)
{
    APP_LOGI(
        "----------VerifyIOAbilityLifecycleCallbacks::OnAbilityForeground called. AbilityName is %{public}s----------",
        ability->GetAbilityName().c_str());
}

void VerifyIOAbilityLifecycleCallbacks::OnAbilityActive(const std::shared_ptr<Ability> &ability)
{
    APP_LOGI("----------VerifyIOAbilityLifecycleCallbacks::OnAbilityActive called. AbilityName is %{public}s----------",
        ability->GetAbilityName().c_str());
}

void VerifyIOAbilityLifecycleCallbacks::OnAbilityStop(const std::shared_ptr<Ability> &ability)
{
    APP_LOGI("----------VerifyIOAbilityLifecycleCallbacks::OnAbilityStop called. AbilityName is %{public}s----------",
        ability->GetAbilityName().c_str());
}
void VerifyIOAbilityLifecycleCallbacks::OnAbilitySaveState(const PacMap &outState)
{
    APP_LOGI("----------VerifyIOAbilityLifecycleCallbacks::OnAbilitySaveState called.----------");
}

void VerifyActFirstAbility::OnStart(const Want &want)
{
    APP_LOGI("----------VerifyActFirstAbility::onStart begin----------");
    std::shared_ptr<VerifyIOAbilityLifecycleCallbacks> callback = std::make_shared<VerifyIOAbilityLifecycleCallbacks>();
    Ability::GetApplication()->RegisterAbilityLifecycleCallbacks(callback);

    EventFwk::MatchingSkills MatchingSkillsFirst;
    MatchingSkillsFirst.AddEvent("Test_AbilityContextStartAbility");
    EventFwk::CommonEventSubscribeInfo subscriberInfoFirst(MatchingSkillsFirst);
    std::shared_ptr<AbilityContextStartAbilityTest> subscriberFirst =
        std::make_shared<AbilityContextStartAbilityTest>(subscriberInfoFirst);
    EventFwk::CommonEventManager::SubscribeCommonEvent(subscriberFirst);

    EventFwk::MatchingSkills MatchingSkillsService;
    MatchingSkillsService.AddEvent("Test_ServiceAbility");
    EventFwk::CommonEventSubscribeInfo subscriberInfoService(MatchingSkillsService);
    sptr<OHOS::AAFwk::IAbilityConnection> conne = new AbilityConnectionActFirst();
    std::shared_ptr<ConnectServiceAbilityTest> serviceSubscriber =
        std::make_shared<ConnectServiceAbilityTest>(subscriberInfoService, Ability::GetContext(), conne);
    EventFwk::CommonEventManager::SubscribeCommonEvent(serviceSubscriber);
    APP_LOGI("----------VerifyActFirstAbility::onStart end----------");

    Ability::OnStart(want);
}
void VerifyActFirstAbility::OnStop()
{
    APP_LOGI("----------VerifyActFirstAbility::onStop begin----------");
    APP_LOGI("----------VerifyActFirstAbility::onStop end----------");
    Ability::OnStop();
}
void VerifyActFirstAbility::OnActive()
{
    APP_LOGI("----------VerifyActFirstAbility::OnActive begin----------");
    APP_LOGI("----------VerifyActFirstAbility::OnActive end----------");
    Ability::OnActive();
}
void VerifyActFirstAbility::OnInactive()
{
    APP_LOGI("----------VerifyActFirstAbility::OnInactive begin----------");
    APP_LOGI("----------VerifyActFirstAbility::OnInactive end----------");
    Ability::OnInactive();
}
void VerifyActFirstAbility::OnBackground()
{
    APP_LOGI("----------VerifyActFirstAbility::OnBackground begin----------");
    APP_LOGI("----------VerifyActFirstAbility::OnBackground end----------");
    Ability::OnBackground();
}

void AbilityContextStartAbilityTest::OnReceiveEvent(const EventFwk::CommonEventData &data)
{
    APP_LOGI("---------- VerifyActFirstAbility AbilityContextStartAbilityTest begin----------");
    APP_LOGI("---------- VerifyActFirstAbility AbilityContextStartAbilityTest start ability----------");
    auto abilityContext = std::make_shared<AbilityContext>();
    int requestCode = -1;
    Want want;
    std::string targetBundle = "com.ix.ServiceaAbility";
    std::string targetAbility = "MainServiceAbilityDemo";
    want.SetElementName(targetBundle, targetAbility);
    abilityContext->StartAbility(want, requestCode);
    APP_LOGI("---------- VerifyActFirstAbility AbilityContextStartAbilityTest start ability end----------");
    APP_LOGI("---------- VerifyActFirstAbility AbilityContextStartAbilityTest end----------");
}

void ConnectServiceAbilityTest::OnReceiveEvent(const EventFwk::CommonEventData &data)
{
    if (abilityContext_ == nullptr) {
        APP_LOGI("----------ConnectServiceAbilityTest::OnReceiveEvent abilityContext_ == nullptr----------");
        return;
    }

    Want want_service;
    std::string targetBundle = "com.ix.ServiceaAbility";
    std::string targetAbility = "MainServiceAbilityDemo";
    want_service.SetElementName(targetBundle, targetAbility);
    switch (data.GetCode()) {
        case CONNECT_SERVICE_ABILITY: {
            auto ret = abilityContext_->ConnectAbility(want_service, conne_);
            APP_LOGI("connect ability return %{public}d", ret);
            break;
        }
        case DISCONNECT_SERVICE_ABILITY: {
            abilityContext_->DisconnectAbility(conne_);
            break;
        }
        case START_SERVICE_ABILITY: {
            abilityContext_->StartAbility(want_service, 0);
            break;
        }
        case STOP_SERVICE_ABILITY: {
            auto ret = abilityContext_->StopAbility(want_service);
            APP_LOGI("stop ability return %{public}d", ret);
            break;
        }
        default:
            APP_LOGI("---------- ConnectServiceAbilityTest OnReceiveEvent default----------");
            break;
    }
}
REGISTER_AA(VerifyActFirstAbility)
}  // namespace AppExecFwk
}  // namespace OHOS