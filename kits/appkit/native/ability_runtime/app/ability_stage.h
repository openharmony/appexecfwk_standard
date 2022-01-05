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

#ifndef ABILITY_RUNTIME_ABILITY_STAGE_H
#define ABILITY_RUNTIME_ABILITY_STAGE_H

#include <map>
#include <memory>
#include <string>

#include "ability_local_record.h"
#include "ability_runtime/context/context.h"

namespace OHOS {
namespace AbilityRuntime {
class Runtime;
/**
 * @brief the hap level entity
 *
 * To share the data amony the abilities of a hap,
 * and supply lifecycle func for the developers.
 *
 */
class AbilityStage {
public:
    static std::shared_ptr<AbilityStage> Create(
        const std::unique_ptr<Runtime>& runtime, const AppExecFwk::HapModuleInfo& hapModuleInfo);

    AbilityStage() = default;
    virtual ~AbilityStage() = default;
    virtual void OnCreate() const;
    virtual void OnDestory() const;
    virtual void Init(std::shared_ptr<Context> context);
    std::shared_ptr<Context> GetContext() const;
    void AddAbility(const sptr<IRemoteObject> &token, std::shared_ptr<AppExecFwk::AbilityLocalRecord> abilityRecord);
    void RemoveAbility(const sptr<IRemoteObject> &token);
    bool ContainsAbility();
private:
    std::shared_ptr<Context> context_;
    std::map<sptr<IRemoteObject>, std::shared_ptr<AppExecFwk::AbilityLocalRecord>> abilityRecords_;
};
}  // namespace AbilityRuntime
}  // namespace OHOS
#endif  // ABILITY_RUNTIME_ABILITY_STAGE_H