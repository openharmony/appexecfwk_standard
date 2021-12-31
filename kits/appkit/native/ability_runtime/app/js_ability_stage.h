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

#ifndef ABILITY_RUNTIME_JS_ABILITY_STAGE_H
#define ABILITY_RUNTIME_JS_ABILITY_STAGE_H

#include "ability_stage.h"

class NativeReference;

namespace OHOS {
namespace AbilityRuntime {
class JsRuntime;
class JsAbilityStage : public AbilityStage {
public:
    static std::shared_ptr<AbilityStage> Create(
        const std::unique_ptr<Runtime>& runtime, const AppExecFwk::HapModuleInfo& hapModuleInfo);

    JsAbilityStage(JsRuntime& jsRuntime, std::unique_ptr<NativeReference>&& jsAbilityStageObj);
    ~JsAbilityStage() override;

    void Init(std::shared_ptr<Context> context) override;

    void OnCreate() const override;

private:
    JsRuntime& jsRuntime_;
    std::unique_ptr<NativeReference> jsAbilityStageObj_;
};
}  // namespace AbilityRuntime
}  // namespace OHOS
#endif  // ABILITY_RUNTIME_JS_ABILITY_STAGE_H