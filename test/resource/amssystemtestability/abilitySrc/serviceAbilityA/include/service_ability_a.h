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
#ifndef MAIN_SERVICE_ABILITY_H
#define MAIN_SERVICE_ABILITY_H
#include "app_log_wrapper.h"
#include "iremote_broker.h"
#include "iremote_object.h"
#include "iremote_proxy.h"
#include "iremote_stub.h"
#include "want.h"
#include "ability.h"
#include "ability_loader.h"

using Want = OHOS::AAFwk::Want;
namespace OHOS {
namespace AppExecFwk {

class ServiceAbilityA : public Ability {
protected:
    virtual void OnStart(const Want &want);
    virtual void OnCommand(const AAFwk::Want &want, bool restart, int startId);
    virtual sptr<IRemoteObject> OnConnect(const Want &want);
    virtual void OnDisconnect(const Want &want);
    virtual void OnStop();
};

class IServiceRemoteTest : public IRemoteBroker {
public:
    DECLARE_INTERFACE_DESCRIPTOR(u"ohos.appexecfwk.ServiceRemoteTest");
    virtual int32_t ScheduleAdd(int32_t a, int32_t b) = 0;
    virtual int32_t ScheduleSub(int32_t a, int32_t b) = 0;
    enum {
        // ipc id for OnAbilityConnectDone
        ON_ABILITY_ADD = 0,

        // ipc id for OnAbilityDisConnectDone
        ON_ABILITY_SUB
    };
};

class MockServiceRemoteTest : public IRemoteStub<IServiceRemoteTest> {
public:
    MockServiceRemoteTest() = default;
    virtual ~MockServiceRemoteTest() = default;

    virtual int OnRemoteRequest(uint32_t code, MessageParcel &data, MessageParcel &reply, MessageOption &option)
    {
        return 0;
    }

private:
    DISALLOW_COPY_AND_MOVE(MockServiceRemoteTest);
};
}  // namespace AppExecFwk
}  // namespace OHOS
#endif  // MAIN_SERVICE_ABILITY_H