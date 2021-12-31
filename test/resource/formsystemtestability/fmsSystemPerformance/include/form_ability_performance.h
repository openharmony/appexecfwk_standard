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

#ifndef _FORM_ABILITY_PERFORMANCE_H_
#define _FORM_ABILITY_PERFORMANCE_H_
#include <string>
#include <unistd.h>
#include <vector>
#include "ability_loader.h"
#include "common_event.h"
#include "common_event_manager.h"

using std::string;

namespace OHOS {
namespace AppExecFwk {
typedef bool (*FunctionPtr)();

class FormEventSubscriber;
class FormAbilityPerformance : public Ability {
public:
    void SubscribeEvent();
    void handleEvent(std::string action, std::string data);

    // Test case list
    void FMS_performanceTest_0100(std::string data);
    void FMS_performanceTest_0200(std::string data);
    void FMS_performanceTest_0300(std::string data);
    void FMS_performanceTest_0400(std::string data);
    void FMS_performanceTest_0500(std::string data);
    void FMS_performanceTest_0600(std::string data);
    void FMS_performanceTest_0700(std::string data);
    void FMS_performanceTest_1300(std::string data);
    void FMS_performanceTest_1400(std::string data);
    void FMS_performanceTest_1500(std::string data);
    void FMS_performanceTest_1600(std::string data);

    std::shared_ptr<FormEventSubscriber> subscriber_;

    class PerformanceFormCallback : public FormCallback {
    public:
        PerformanceFormCallback(std::string name, int code):caseName_(name), code_(code) { }
        virtual ~PerformanceFormCallback() = default;
        void OnAcquired(const int32_t result, const FormJsInfo &formJsInfo) const override;
        void OnUpdate(const int32_t result, const FormJsInfo &formJsInfo) const override;
        void OnFormUninstall(const int64_t formId) const override;

        std::string caseName_ = "";
        int code_ = 0;
        FormAbilityPerformance *ability_ = nullptr;
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
    using FormFunc = void (FormAbilityPerformance::*)(std::string data);
    std::map<std::string, FormFunc> memberFuncMap_;
    std::map<std::string, int> calledFuncMap_;
    std::shared_ptr<PerformanceFormCallback> callback_;
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

    FormAbilityPerformance *ability_;
};
}  // namespace AppExecFwk
}  // namespace OHOS
#endif  // _FORM_ABILITY_PERFORMANCE_H_