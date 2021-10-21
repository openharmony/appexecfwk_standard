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

#ifndef _FORM_ABILITY_COMMONB_H_
#define _FORM_ABILITY_COMMONB_H_
#include "ability_loader.h"
#include "common_event.h"
#include "common_event_manager.h"

namespace OHOS {
namespace AppExecFwk {
class FormEventSubscriberForCommonB;
class FormAbilityCommonB : public Ability {
public:
    ~FormAbilityCommonB();
    void SubscribeEvent();
    void handleEvent(std::string action, std::string data);
    void FMS_acquireForm(std::string data);
    void FMS_deleteForm(std::string data);
    void FMS_acquireForm_batch(std::string data);
    void FMS_deleteFormBatch(std::string data);
    std::shared_ptr<FormEventSubscriberForCommonB> subscriber_;

    class AcquireFormCallback : public FormCallback {
    public:
        AcquireFormCallback()
        {
        }
        virtual ~AcquireFormCallback() = default;
        void OnAcquired(const int32_t result, const FormJsInfo &formJsInfo) const override;
        void OnUpdate(const int32_t result, const FormJsInfo &formJsInfo) const override;
        void OnFormUninstall(const int64_t formId) const override;
    };
protected:
    virtual void Init(const std::shared_ptr<AbilityInfo> &abilityInfo,
        const std::shared_ptr<OHOSApplication> &application, std::shared_ptr<AbilityHandler> &handler,
        const sptr<IRemoteObject> &token) override;
    virtual void OnStart(const Want &want) override;
    virtual void OnStop() override;
    virtual void OnActive() override;
    virtual void OnInactive() override;
    virtual void OnBackground() override;

    using FormFunc = void (FormAbilityCommonB::*)(std::string data);
    std::map<std::string, FormFunc> memberFuncMap_;
    std::shared_ptr<AcquireFormCallback> callback_;
};

class FormEventSubscriberForCommonB : public EventFwk::CommonEventSubscriber {
public:
    FormEventSubscriberForCommonB(const EventFwk::CommonEventSubscribeInfo &sp) : CommonEventSubscriber(sp)
    {
        ability_ = nullptr;
    }
    ~FormEventSubscriberForCommonB()
    {
        ability_ = nullptr;
    }
    virtual void OnReceiveEvent(const EventFwk::CommonEventData &data) override;
    void KitTerminateAbility();

    FormAbilityCommonB *ability_;
};
}  // namespace AppExecFwk
}  // namespace OHOS
#endif  // _FORM_ABILITY_COMMONB_H_