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

#ifndef _FORM_ABILITY_STRESS_H_
#define _FORM_ABILITY_STRESS_H_
#include "ability_loader.h"
#include "common_event.h"
#include "common_event_manager.h"
#include <string>
#include <unistd.h>
#include <vector>

using std::string;

namespace OHOS {
namespace AppExecFwk {
typedef bool (*FunctionPtr)();

class FormEventSubscriber;
class FormAbilityStress : public Ability {
public:
    void SubscribeEvent();
    void handleEvent(std::string action, std::string data);

    // Test case list
    void FMS_stressTest_0100(std::string data);
    void FMS_stressTest_0100_sub01(std::string form_id);
    void FMS_stressTest_0200(std::string data);
    void FMS_stressTest_0200_sub01(std::string form_id);
    void FMS_stressTest_0200_sub02(std::string form_id);
    void FMS_stressTest_0200_sub03(std::string form_id);
    void FMS_stressTest_0300(std::string data);
    void FMS_stressTest_0300_sub01(std::string form_id);
    void FMS_stressTest_0300_sub02(std::string form_id);
    void FMS_stressTest_0400(std::string data);
    void FMS_stressTest_0500(std::string data);
    void FMS_stressTest_0600(std::string data);
    void FMS_stressTest_0700(std::string data);
    void FMS_stressTest_0800(std::string data);
    void FMS_stressTest_0900(std::string data);
    void FMS_stressTest_1000(std::string data);
    void FMS_stressTest_1100(std::string data);
    void FMS_stressTest_1100_sub01(std::string form_id);
    void FMS_stressTest_1100_sub02(std::string form_id);
    void FMS_stressTest_1100_sub03(std::string form_id);
    void FMS_stressTest_1200(std::string data);
    void FMS_stressTest_1300(std::string data);
    void FMS_stressTest_1300_sub01(std::string form_id);
    void FMS_stressTest_1300_sub02(std::string form_id);
    void FMS_stressTest_1300_sub03(std::string form_id);
    void FMS_stressTest_1400(std::string data);
    void FMS_stressTest_1500(std::string data);
    void FMS_stressTest_1600(std::string data);
    void FMS_stressTest_1700(std::string data);
    void FMS_stressTest_1700_sub01(std::string data);
    void FMS_stressTest_1700_sub02(std::string data);
    void FMS_stressTest_1800(std::string data);

    std::shared_ptr<FormEventSubscriber> subscriber_;

    class StressFormCallback : public FormCallback {     
    public:   
        StressFormCallback(std::string name, int code):caseName_(name), code_(code) { }
        virtual ~StressFormCallback() = default;
        void OnAcquired(const int32_t result, const FormJsInfo &formJsInfo) const override;
        void OnUpdate(const int32_t result, const FormJsInfo &formJsInfo) const override;
        void OnFormUninstall(const int64_t formId) const override;

        std::string caseName_ = "";
        int code_ = 0;
        FormAbilityStress *ability_;
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
    using FormFunc = void (FormAbilityStress::*)(std::string data);
    std::map<std::string, FormFunc> memberFuncMap_;
    std::map<std::string, int> calledFuncMap_;
    std::shared_ptr<StressFormCallback> callback_;
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

    FormAbilityStress *ability_;
};
}  // namespace AppExecFwk
}  // namespace OHOS
#endif  // _FORM_ABILITY_STRESS_H_