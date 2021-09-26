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
#include "service_ability_a.h"
// #include "app_log_wrapper.h"
// #include "iremote_broker.h"
// #include "iremote_object.h"
// #include "iremote_proxy.h"
// #include "iremote_stub.h"
// #include "main_service_ability.h"
#include "test_ability_connection.h"

namespace OHOS {
namespace AppExecFwk {
void ServiceAbilityA::OnStart(const Want &want)
{
    APP_LOGI("ServiceAbilityA OnStart");
    Ability::OnStart(want);
}
void ServiceAbilityA::OnCommand(const AAFwk::Want &want, bool restart, int startId)
{
    APP_LOGI("ServiceAbilityA::OnCommand");
    Ability::OnCommand(want, restart, startId);
}
sptr<IRemoteObject> ServiceAbilityA::OnConnect(const Want &want)
{
    APP_LOGI("ServiceAbilityA::OnConnect");
    sptr<IServiceRemoteTest> remoteObject = sptr<IServiceRemoteTest>(new (std::nothrow) TestAbilityConnection());

    Ability::OnConnect(want);
    return remoteObject->AsObject();
}
void ServiceAbilityA::OnDisconnect(const Want &want)
{
    APP_LOGI("ServiceAbilityA::OnDisconnect");
    Ability::OnDisconnect(want);
}
void ServiceAbilityA::OnStop()
{
    APP_LOGI("ServiceAbilityA::OnStop");
    Ability::OnStop();
}
REGISTER_AA(ServiceAbilityA);
}  // namespace AppExecFwk
}  // namespace OHOS
