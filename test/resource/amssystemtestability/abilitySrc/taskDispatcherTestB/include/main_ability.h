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

#ifndef AMS_KIT_SYSTEM_TEST_B_MAIN_ABILITY_H
#define AMS_KIT_SYSTEM_TEST_B_MAIN_ABILITY_H
#include "ability.h"
#include "ability_loader.h"
#include "common_event.h"
#include "common_event_manager.h"
#include "task_dispatcher_tools.h"
#include "task_dispatcher_test_info.h"

namespace OHOS {
namespace AppExecFwk {
using namespace OHOS::EventFwk;
class FirstEventSubscriber;
class MainAbility : public Ability {
public:
    MainAbility()
    {
        initGlobal();
        initParallel();
        initSerial();
        initSpec();
        initHybrid();
        initMultiApp();
        initPriority();
        initCancelTask();
        initExtra();
    }
    void initGlobal()
    {
        mapCase_[(int)TestFunc::GLOBAL] = {};
    }
    void initParallel()
    {
        mapCase_[(int)TestFunc::PARALLEL] = {};
    }
    void initSerial()
    {
        mapCase_[(int)TestFunc::SERIAL] = {};
    }
    void initSpec()
    {
        mapCase_[(int)TestFunc::SPEC] = {};
    }
    void initHybrid()
    {
        mapCase_[(int)TestFunc::HYBRID] = {};
    }
    void initMultiApp()
    {
        mapCase_[(int)TestFunc::MULTI_APP] = {
            [this](int code) { MultiAppCase1(code); },
            [this](int code) { MultiAppCase2(code); },
            [this](int code) { MultiAppCase3(code); },
            [this](int code) { MultiAppCase4(code); },
        };
    }
    void initPriority()
    {
        mapCase_[(int)TestFunc::PRIORITY] = {};
    }
    void initCancelTask()
    {
        mapCase_[(int)TestFunc::CANCEL_TASK] = {};
    }
    void initExtra()
    {
        mapCase_[(int)TestFunc::EXTRA] = {};
    }

    void SubscribeEvent();
    void TestDispatcher(int apiIndex, int caseIndex, int code);
    int Dispatch(STtools::TestSetting outer, STtools::TestSetting inner);

    void MultiAppCase1(int code);
    void MultiAppCase2(int code);
    void MultiAppCase3(int code);
    void MultiAppCase4(int code);

    std::unordered_map<int, std::vector<std::function<void(int)>>> mapCase_;
    ~MainAbility();

protected:
    void Init(const std::shared_ptr<AbilityInfo> &abilityInfo, const std::shared_ptr<OHOSApplication> &application,
        std::shared_ptr<AbilityHandler> &handler, const sptr<IRemoteObject> &token) override;
    virtual void OnStart(const Want &want) override;
    virtual void OnStop() override;
    virtual void OnActive() override;
    virtual void OnInactive() override;
    virtual void OnBackground() override;
    virtual void OnForeground(const Want &want) override;
    std::shared_ptr<FirstEventSubscriber> subscriber_;
};
class FirstEventSubscriber : public EventFwk::CommonEventSubscriber {
public:
    explicit FirstEventSubscriber(const EventFwk::CommonEventSubscribeInfo &sp) : CommonEventSubscriber(sp)
    {
        mapTestFunc_ = {
            {"Dispatcher",
                [this](int apiIndex, int caseIndex, int code) { TestDispatcher(apiIndex, caseIndex, code); }},
        };
        mainAbility = nullptr;
    }

    void TestDispatcher(int apiIndex, int caseIndex, int code)
    {
        mainAbility->TestDispatcher(apiIndex, caseIndex, code);
    }

    virtual void OnReceiveEvent(const EventFwk::CommonEventData &data);

    MainAbility *mainAbility;
    std::unordered_map<std::string, std::function<void(int, int, int)>> mapTestFunc_;
    ~FirstEventSubscriber() = default;
};
}  // namespace AppExecFwk
}  // namespace OHOS
#endif  // AMS_KIT_SYSTEM_TEST_B_MAIN_ABILITY_H