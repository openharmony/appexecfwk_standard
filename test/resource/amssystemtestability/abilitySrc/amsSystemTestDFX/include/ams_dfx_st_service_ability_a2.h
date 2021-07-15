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

#ifndef AMS_DFX_ST_SERVICE_ABILITY_A2_
#define AMS_DFX_ST_SERVICE_ABILITY_A2_
#include "stpageabilityevent.h"
#include <string>
#include <thread>
#include <map>
#include "ability_connect_callback_stub.h"
#include "ability_connect_callback_proxy.h"
#include "ability_loader.h"
#include "common_event.h"
#include "common_event_manager.h"

namespace OHOS {
namespace AppExecFwk {

class AmsStAbilityDFXA2 : public Ability {
public:
    ~AmsStAbilityDFXA2();

protected:
    virtual void OnStart(const Want &want) override;
    virtual void OnStop() override;
    virtual void OnActive() override;
    virtual void OnInactive() override;
    virtual void OnBackground() override;
    virtual void OnNewWant(const Want &want) override;
    virtual void OnCommand(const AAFwk::Want &want, bool restart, int startId) override;
    virtual sptr<IRemoteObject> OnConnect(const Want &want) override;
    virtual void OnDisconnect(const Want &want) override;

private:
    STPageAbilityEvent pageAbilityEvent;
};
}  // namespace AppExecFwk
}  // namespace OHOS
#endif  // AMS_DFX_ST_SERVICE_ABILITY_A2_