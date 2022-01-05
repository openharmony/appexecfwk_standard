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

#ifndef _FORM_ABILITY_RELEASEFORM_H_
#define _FORM_ABILITY_RELEASEFORM_H_
#include "ability_loader.h"
#include "common_event.h"
#include "common_event_manager.h"

namespace OHOS {
namespace AppExecFwk {
class FormEventSubscriberForReleaseForm;
class FormAbilityReleaseForm : public Ability {
public:
    void SubscribeEvent(const std::vector<std::string> &eventList);
    void handleEvent(std::string action, std::string data);
    void FMS_releaseForm_common(int64_t formId, std::string caseName, std::string lastFormId);
    void FMS_releaseForm_0300(std::string data);
    void FMS_releaseForm_0400(std::string data);
    void FMS_releaseForm_0500(std::string data);
    void FMS_releaseForm_0600(std::string data);
    void FMS_releaseForm_0700(std::string data);
    void FMS_releaseForm_0800(std::string data);
    void FMS_releaseForm_0900(std::string data);
    void FMS_releaseForm_1000(std::string data);
    void FMS_releaseForm_1100(std::string data);
    void FMS_releaseForm_1200(std::string data);
    void FMS_releaseForm_1300(std::string data);
    void FMS_releaseForm_1400(std::string data);
    void FMS_releaseForm_common_del(std::string data);
    void Clear(std::string case_id, int64_t form_id);

    std::shared_ptr<FormEventSubscriberForReleaseForm> subscriber_;

    class AcquireFormCallback : public FormCallback {
    public:
        AcquireFormCallback(std::string name, std::string id):caseName_(name), lastformId_(id)
        {
        }
        virtual ~AcquireFormCallback() = default;
        void OnAcquired(const int32_t result, const FormJsInfo &formJsInfo) const override;
        void OnUpdate(const int32_t result, const FormJsInfo &formJsInfo) const override;
        void OnFormUninstall(const int64_t formId) const override;
        FormAbilityReleaseForm *ability_ = nullptr;
        std::string caseName_;
        std::string lastformId_;
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
    void ReleaseForm_0300(int64_t form_id);
    void ReleaseForm_0400(int64_t form_id);
    void ReleaseForm_0500(int64_t form_id);
    void ReleaseForm_0600(int64_t form_id);
    void ReleaseForm_0700(int64_t form_id);
    void ReleaseForm_0800(int64_t form_id);
    void ReleaseForm_0900(int64_t form_id);
    void ReleaseForm_1000(int64_t form_id);
    void ReleaseForm_1100(int64_t form_id);
    void ReleaseForm_1200(int64_t form_id);
    void ReleaseForm_1300(int64_t form_id, std::string lastFormId);
    void ReleaseForm_1400(int64_t form_id);

    using FormFunc = void (FormAbilityReleaseForm::*)(std::string data);
    std::map<std::string, FormFunc> memberFuncMap_;
    std::shared_ptr<AcquireFormCallback> callback_;
};
class FormEventSubscriberForReleaseForm : public EventFwk::CommonEventSubscriber {
public:
    FormEventSubscriberForReleaseForm(const EventFwk::CommonEventSubscribeInfo &sp) : CommonEventSubscriber(sp)
    {
        ability_ = nullptr;
    }
    ~FormEventSubscriberForReleaseForm()
    {
        ability_ = nullptr;
    }
    virtual void OnReceiveEvent(const EventFwk::CommonEventData &data) override;
    void KitTerminateAbility();

    FormAbilityReleaseForm *ability_;
};
}  // namespace AppExecFwk
}  // namespace OHOS
#endif  // _FORM_ABILITY_RELEASEFORM_H_