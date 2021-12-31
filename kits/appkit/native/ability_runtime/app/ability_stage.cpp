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

#include "ability_stage.h"

#include "hilog_wrapper.h"
#include "js_ability_stage.h"
#include "runtime.h"

namespace OHOS {
namespace AbilityRuntime {
std::shared_ptr<AbilityStage> AbilityStage::Create(
    const std::unique_ptr<Runtime>& runtime, const AppExecFwk::HapModuleInfo& hapModuleInfo)
{
    if (!runtime) {
        return std::make_shared<AbilityStage>();
    }

    switch (runtime->GetLanguage()) {
        case Runtime::Language::JS:
            return JsAbilityStage::Create(runtime, hapModuleInfo);

        default:
            return std::make_shared<AbilityStage>();
    }
}

void AbilityStage::OnCreate() const
{
    HILOG_INFO("AbilityStage::OnCreate come");
}

void AbilityStage::OnDestory() const
{
    HILOG_INFO("AbilityStage::OnDestory come");
}

std::shared_ptr<Context> AbilityStage::GetContext() const
{
    return context_;
}

void AbilityStage::Init(std::shared_ptr<Context> context)
{
    this->context_ = context;
}

void AbilityStage::AddAbility(const sptr<IRemoteObject> &token,
    std::shared_ptr<AppExecFwk::AbilityLocalRecord> abilityRecord)
{
    if (token == nullptr) {
        HILOG_ERROR("AbilityStage::AddAbility failed, token is nullptr");
        return;
    }

    if (abilityRecord == nullptr) {
        HILOG_ERROR("AbilityStage::AddAbility failed, abilityRecord is nullptr");
        return;
    }

    abilityRecords_[token] = abilityRecord;
}

void AbilityStage::RemoveAbility(const sptr<IRemoteObject> &token)
{
    if (token == nullptr) {
        HILOG_ERROR("AbilityStage::RemoveAbility failed, token is nullptr");
        return;
    }
    abilityRecords_.erase(token);
}

bool AbilityStage::ContainsAbility()
{
    return abilityRecords_.size() > 0;
}
}  // namespace AbilityRuntime
}  // namespace OHOS
