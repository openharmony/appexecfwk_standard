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

#ifndef _FORM_ABILITY_DELETEFORM_H_
#define _FORM_ABILITY_DELETEFORM_H_
#include "ability_loader.h"
#include "common_event.h"
#include "common_event_manager.h"

namespace OHOS {
namespace AppExecFwk {
class FormEventSubscriberForDeleteForm;
class FormAbilityDeleteForm : public Ability {
public:
    void SubscribeEvent(const std::vector<std::string> &eventList);
    void handleEvent(std::string action, std::string data);
    void FMS_deleteForm_common(int64_t formId, std::string caseName);
    void FMS_deleteForm_0300(std::string data);
    void FMS_deleteForm_0400(std::string data);
    void FMS_deleteForm_0500(std::string data);
    void FMS_deleteForm_0600(std::string data);
    void FMS_deleteForm_0700(std::string data);
    void FMS_deleteForm_0800(std::string data);
    void FMS_deleteForm_0900(std::string data);
    void FMS_deleteForm_1000(std::string data);
    void FMS_deleteForm_1100(std::string data);
    void FMS_deleteForm_1200(std::string data);
    void FMS_deleteForm_1201(std::string data);
    void FMS_deleteForm_1400(std::string data);
    void FMS_deleteForm_1401(std::string data);
    void FMS_deleteForm_1500(std::string data);
    void FMS_deleteForm_1501(std::string data);
    void FMS_deleteForm_1502(std::string data);
    void FMS_deleteForm_1600(std::string data);
    void FMS_deleteForm_1601(std::string data);
    void FMS_deleteForm_1602(std::string data);
    void FMS_deleteForm_1700(std::string data);
    void FMS_deleteForm_1701(std::string data);
    void FMS_deleteForm_1702(std::string data);
    void FMS_acquire_tempForm_batch(std::string data);
    void FMS_acquireForm_batch(std::string data);
    void FMS_deleteFormBatch(std::string data);

    std::shared_ptr<FormEventSubscriberForDeleteForm> subscriber_;
    class AcquireFormCallback : public FormCallback {
    public:
        AcquireFormCallback(std::string name):caseName_(name)
        {
        }
        virtual ~AcquireFormCallback() = default;
        void OnAcquired(const int32_t result, const FormJsInfo &formJsInfo) const override;
        void OnUpdate(const int32_t result, const FormJsInfo &formJsInfo) const override;
        void OnFormUninstall(const int64_t formId) const override;
        FormAbilityDeleteForm *ability_ = nullptr;
        std::string caseName_;
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
    void Clear(std::string case_id, int64_t form_id);
    void DeleteForm_0300(int64_t form_id);
    void DeleteForm_0400(int64_t form_id);
    void DeleteForm_0500(int64_t form_id);
    void DeleteForm_0600(int64_t form_id);
    void DeleteForm_0700(int64_t form_id);
    void DeleteForm_0800(int64_t form_id);
    void DeleteForm_0900(int64_t form_id);
    void DeleteForm_1000(int64_t form_id);
    void DeleteForm_1100(int64_t form_id);
    void DeleteForm_1200(int64_t form_id);
    void DeleteForm_1400(int64_t form_id);
    void DeleteForm_1500(int64_t form_id);
    void DeleteForm_1501(int64_t form_id);
    void DeleteForm_1600(int64_t form_id);
    void DeleteForm_1601(int64_t form_id);
    void DeleteForm_1700(int64_t form_id);
    void DeleteForm_1701(int64_t form_id);

    using FormFunc = void (FormAbilityDeleteForm::*)(std::string data);
    std::map<std::string, FormFunc> memberFuncMap_;
    std::shared_ptr<AcquireFormCallback> callback_;
};
class FormEventSubscriberForDeleteForm : public EventFwk::CommonEventSubscriber {
public:
    FormEventSubscriberForDeleteForm(const EventFwk::CommonEventSubscribeInfo &sp) : CommonEventSubscriber(sp)
    {
        ability_ = nullptr;
    }
    ~FormEventSubscriberForDeleteForm()
    {
        ability_ = nullptr;
    }
    virtual void OnReceiveEvent(const EventFwk::CommonEventData &data) override;
    void KitTerminateAbility();

    FormAbilityDeleteForm *ability_;
};
}  // namespace AppExecFwk
}  // namespace OHOS
#endif  // _FORM_ABILITY_DELETEFORM_H_