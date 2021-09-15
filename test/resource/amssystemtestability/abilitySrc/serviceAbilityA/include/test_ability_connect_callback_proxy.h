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

#ifndef OHOS_AAFWK_TEST_ABILITY_CONNECT_CALLBACK_PROXY_H
#define OHOS_AAFWK_TEST_ABILITY_CONNECT_CALLBACK_PROXY_H

// #include "test_ability_connect_callback_interface.h"
#include "iremote_proxy.h"
#include "service_ability_a.h"

namespace OHOS {
namespace AppExecFwk {
/**
 * @class AbilityConnectProxy
 * AbilityConnect proxy.
 */
class TestAbilityConnectionProxy : public IRemoteProxy<IServiceRemoteTest> {
public:
    explicit TestAbilityConnectionProxy(const sptr<IRemoteObject> &impl) : IRemoteProxy<IServiceRemoteTest>(impl)
    {}

    virtual ~TestAbilityConnectionProxy()
    {}

    virtual int32_t ScheduleAdd(int32_t a, int32_t b) override;
    virtual int32_t ScheduleSub(int32_t a, int32_t b) override;

private:
    bool WriteInterfaceToken(MessageParcel &data);

private:
    static inline BrokerDelegator<TestAbilityConnectionProxy> delegator_;
};
}  // namespace AppExecFwk
}  // namespace OHOS
#endif  // OHOS_AAFWK_ABILITY_CONNECT_CALLBACK_PROXY_H