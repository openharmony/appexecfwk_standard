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

#ifndef _FORM_ABILITY_A_H_
#define _FORM_ABILITY_A_H_
#include "ability_loader.h"
#include "common_event.h"
#include "common_event_manager.h"

namespace OHOS {
namespace AppExecFwk {
class FormEventSubscriber;
class FormAbilityA : public Ability {
public:
    ~FormAbilityA();
    void SubscribeEvent();
    void handleEvent(std::string action, std::string data);

    void FMS_deleteFormCommon(std::string strFormId);

    void FMS_acquireForm_0300(std::string data);
    void FMS_acquireForm_0400(std::string data);
    void FMS_acquireForm_0500(std::string data);
    void FMS_acquireForm_0600(std::string data);
    void FMS_acquireForm_0700(std::string data);
    void FMS_acquireForm_1000(std::string data);
    void FMS_acquireForm_1100(std::string data);
    void FMS_acquireForm_1200(std::string data);
    void FMS_acquireForm_1500_1(std::string data);
    void FMS_acquireForm_1600(std::string data);
    void FMS_acquireForm_1600_1(std::string data);
    void FMS_acquireForm_1800(std::string data);
    void FMS_acquireForm_1800_1(std::string data);
    void FMS_acquireForm_1900(std::string data);
    void FMS_acquireForm_2100(std::string data);
    void FMS_acquireForm_2200(std::string data);
    void FMS_acquireForm_2300(std::string data);
    void FMS_acquireForm_2400(std::string data);
    void FMS_acquireForm_2500(std::string data);
    void FMS_acquireForm_2600(std::string data);
    void FMS_acquireForm_2600_1(std::string data);
    void FMS_acquireForm_2700(std::string data);
    void FMS_acquireForm_2800(std::string data);
    void FMS_acquireForm_2900(std::string data);
    void FMS_acquireForm_2900_1(std::string data);
    void FMS_acquireForm_3000(std::string data);
    void FMS_acquireForm_3100(std::string data);
    void FMS_acquireForm_3300(std::string data);
    void FMS_acquireForm_3400(std::string data);
    void FMS_acquireForm_3500(std::string data);
    void FMS_acquireForm_tempForm(std::string data);
    void FMS_acquireFormBatch(std::string data);
    void FMS_acquireFormBatchB(std::string data);
    void FMS_acquireFormTempBatch(std::string data);
    void FMS_deleteFormBatch(std::string strFormId);


    std::shared_ptr<FormEventSubscriber> subscriber_;

    class AcquireFormCallback : public FormCallback {
    public:
        AcquireFormCallback(std::string name, int code):caseName_(name), code_(code)
        {
        }
        virtual ~AcquireFormCallback() = default;
        void OnAcquired(const int32_t result, const FormJsInfo &formJsInfo) const override;
        void OnUpdate(const int32_t result, const FormJsInfo &formJsInfo) const override;
        void OnFormUninstall(const int64_t formId) const override;

        std::string caseName_ = "";
        int code_ = 0;
        FormAbilityA *ability_;
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
private:
    void Clear();

    using FormFunc = void (FormAbilityA::*)(std::string data);
    std::map<std::string, FormFunc> memberFuncMap_;
    std::map<std::string, int> calledFuncMap_;
    std::shared_ptr<AcquireFormCallback> callback_;
};

class FormEventSubscriber : public EventFwk::CommonEventSubscriber {
public:
    FormEventSubscriber(const EventFwk::CommonEventSubscribeInfo &sp) : CommonEventSubscriber(sp)
    {
        ability_ = nullptr;
    }
    ~FormEventSubscriber()
    {
        ability_ = nullptr;
    }
    virtual void OnReceiveEvent(const EventFwk::CommonEventData &data) override;
    void KitTerminateAbility();

    FormAbilityA *ability_;
};
}  // namespace AppExecFwk
}  // namespace OHOS
#endif  // _FORM_ABILITY_A_H_