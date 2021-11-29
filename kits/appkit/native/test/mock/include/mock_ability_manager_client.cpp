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
#include "mock_ability_manager_client.h"

namespace OHOS {
namespace AAFwk {
std::shared_ptr<MockAbilityManagerClient> MockAbilityManagerClient::mockinstance_ = nullptr;
std::shared_ptr<MockAbilityManagerClient> MockAbilityManagerClient::GetInstance()
{
    if (mockinstance_ == nullptr) {
        mockinstance_ = std::make_shared<MockAbilityManagerClient>();
    }
    return mockinstance_;
}

std::shared_ptr<AbilityManagerClient> AbilityManagerClient::GetInstance()
{
    if (instance_ == nullptr) {
        instance_ = MockAbilityManagerClient::GetInstance();
    }
    return instance_;
}
}  // namespace AAFwk
}  // namespace OHOS