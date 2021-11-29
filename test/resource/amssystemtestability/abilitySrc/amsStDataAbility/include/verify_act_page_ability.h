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

#ifndef VERIFY_ACT_PAGE_ABILITY_H
#define VERIFY_ACT_PAGE_ABILITY_H
#include "ability_loader.h"
#include "common_event.h"
#include "common_event_manager.h"
namespace OHOS {
namespace AppExecFwk {
namespace {
const int INSERT = 0;
const int DELETE = 1;
const int UPDATE = 2;
const int QUERY = 3;
const int GETFILETYPES = 4;
const int OPENFILE = 5;
}  // namespace
class CommentDataAbilityTest : public EventFwk::CommonEventSubscriber {
public:
    CommentDataAbilityTest(const EventFwk::CommonEventSubscribeInfo &sp) : EventFwk::CommonEventSubscriber(sp) {};
    virtual void OnReceiveEvent(const EventFwk::CommonEventData &data) override;

    std::weak_ptr<Context> DataAbility_;
};

class VerifyActPageAbility : public Ability {
public:
    virtual void OnStart(const Want &want) override;
    virtual void OnStop() override;
    virtual void OnActive() override;
    virtual void OnInactive() override;
    virtual void OnBackground() override;
    virtual void OnForeground(const Want &want) override;
    std::shared_ptr<CommentDataAbilityTest> subscriberDataAbility = nullptr;
};
}  // namespace AppExecFwk
}  // namespace OHOS
#endif  // VERIFY_ACT_PAGE_ABILITY_H