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

#ifndef AMS_ABILITY_VISIBLE_TEST_PAGE_A1
#define AMS_ABILITY_VISIBLE_TEST_PAGE_A1
#include "stpageabilityevent.h"
#include <string>
#include <thread>
#include "ability_connect_callback_stub.h"
#include "ability_connect_callback_proxy.h"
#include "abs_shared_result_set.h"
#include "ability_loader.h"
#include "app_log_wrapper.h"
#include "data_ability_predicates.h"
#include "values_bucket.h"

namespace OHOS {
namespace AppExecFwk {
using AbilityConnectionStub = OHOS::AAFwk::AbilityConnectionStub;
using Uri = OHOS::Uri;

class AbilityConnectCallback : public AbilityConnectionStub {
public:
    /**
     * OnAbilityConnectDone, AbilityMs notify caller ability the result of connect.
     *
     * @param element,.service ability's ElementName.
     * @param remoteObject,.the session proxy of service ability.
     * @param resultCode, ERR_OK on success, others on failure.
     */
    void OnAbilityConnectDone(
        const AppExecFwk::ElementName &element, const sptr<IRemoteObject> &remoteObject, int resultCode) override
    {
        APP_LOGI("AbilityConnectCallback::OnAbilityConnectDone:resultCode = %{public}d", resultCode);
    }

    /**
     * OnAbilityDisconnectDone, AbilityMs notify caller ability the result of disconnect.
     *
     * @param element,.service ability's ElementName.
     * @param resultCode, ERR_OK on success, others on failure.
     */
    void OnAbilityDisconnectDone(const AppExecFwk::ElementName &element, int resultCode) override
    {
        APP_LOGI("AbilityConnectCallback::OnAbilityDisconnectDone:resultCode = %{public}d", resultCode);
    }
};

class AmsAbilityVisibleTestPageA1 : public Ability {
protected:
    virtual void Init(const std::shared_ptr<AbilityInfo> &abilityInfo,
        const std::shared_ptr<OHOSApplication> &application, std::shared_ptr<AbilityHandler> &handler,
        const sptr<IRemoteObject> &token) override;
    virtual void OnStart(const Want &want) override;
    virtual void OnStop() override;
    virtual void OnActive() override;
    virtual void OnInactive() override;
    virtual void OnBackground() override;
    virtual void OnForeground(const Want &want) override;
    virtual void OnNewWant(const Want &want) override;

private:
    void Clear();
    void GetWantInfo(const Want &want);
    std::string Split(std::string &str, std::string delim);

    std::string targetType_;
    std::string targetBundle_;
    std::string targetAbility_;
    sptr<AbilityConnectCallback> stub_;
    STPageAbilityEvent pageAbilityEvent;
};
}  // namespace AppExecFwk
}  // namespace OHOS
#endif  // AMS_ABILITY_VISIBLE_TEST_PAGE_A1