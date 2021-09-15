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

#ifndef FOUNDATION_APPEXECFWK_SERVICES_FORMMGR_INCLUDE_FORMMGR_FORM_SUPPLY_CALLBACK_H
#define FOUNDATION_APPEXECFWK_SERVICES_FORMMGR_INCLUDE_FORMMGR_FORM_SUPPLY_CALLBACK_H

#include "form_ability_connection.h"
#include "form_supply_stub.h"

namespace OHOS {
namespace AppExecFwk {
/**
 * @class FormSupplyCallback
 * form supply service stub.
 */
class FormSupplyCallback : public FormSupplyStub {
public:
    FormSupplyCallback() = default;
    virtual ~FormSupplyCallback() = default;
    static sptr<FormSupplyCallback> GetInstance();

    /**
     * @brief Accept form binding data from form provider.
     * @param providerFormInfo Form binding data.
     * @param want input data.
     * @return Returns ERR_OK on success, others on failure.
     */
    int OnAcquire(const FormProviderInfo &formInfo, const Want &want) override;

    /**
     * @brief Accept other event.
     * @param want input data.
     * @return Returns ERR_OK on success, others on failure.
     */
    int OnEventHandle(const Want &want) override;

    /**
     * @brief Save ability Connection for the callback.
     * @param connection ability connection.
     */
    void AddConnection(sptr<FormAbilityConnection> connection);
    /**
     * @brief Delete ability connection after the callback come.
     * @param connectId The ability connection id generated when save.
     */
    void RemoveConnection(long connectId);
private:
    static std::mutex mutex_;
    static sptr<FormSupplyCallback> instance_;
    
    mutable std::mutex conMutex_;
    std::map<long, sptr<FormAbilityConnection>> connections_;
    DISALLOW_COPY_AND_MOVE(FormSupplyCallback);
};
}  // namespace AppExecFwk
}  // namespace OHOS
#endif  // FOUNDATION_APPEXECFWK_SERVICES_FORMMGR_INCLUDE_FORMMGR_FORM_SUPPLY_CALLBACK_H
