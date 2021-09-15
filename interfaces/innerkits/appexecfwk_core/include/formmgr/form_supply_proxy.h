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

#ifndef FOUNDATION_APPEXECFWK_INTERFACES_INNERKITS_APPEXECFWK_CORE_INCLUDE_FORMMGR_FORM_SUPPLY_PROXY_H
#define FOUNDATION_APPEXECFWK_INTERFACES_INNERKITS_APPEXECFWK_CORE_INCLUDE_FORMMGR_FORM_SUPPLY_PROXY_H

#include "app_log_wrapper.h"
#include "form_supply_interface.h"
#include "iremote_proxy.h"

namespace OHOS {
namespace AppExecFwk {
/**
 * @class FormSupplyProxy
 * FormSupplyProxy is used to access form supply service.
 */
class FormSupplyProxy : public IRemoteProxy<IFormSupply> {
public:
    explicit FormSupplyProxy(const sptr<IRemoteObject> &impl) : IRemoteProxy<IFormSupply>(impl)
    {}

    virtual ~FormSupplyProxy() = default;

    /**
     * @brief Send form binding data from form provider to fms.
     * @param providerFormInfo Form binding data.
     * @param want input data.
     * @return Returns ERR_OK on success, others on failure.
     */
    virtual int OnAcquire(const FormProviderInfo &formInfo, const Want &want) override;

    /**
     * @brief Send other event  to fms.
     * @param want input data.
     * @return Returns ERR_OK on success, others on failure.
     */
    virtual int OnEventHandle(const Want &want) override;

private:
    template<typename T>
    int GetParcelableInfos(MessageParcel &reply, std::vector<T> &parcelableInfos);
    bool WriteInterfaceToken(MessageParcel &data);

private:
    static inline BrokerDelegator<FormSupplyProxy> delegator_;
};
}  // namespace AppExecFwk
}  // namespace OHOS
#endif  // FOUNDATION_APPEXECFWK_INTERFACES_INNERKITS_APPEXECFWK_CORE_INCLUDE_FORMMGR_FORM_SUPPLY_PROXY_H
