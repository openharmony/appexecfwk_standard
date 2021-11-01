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
#define private public
#define protected public

#include <csignal>
#include <unistd.h>

#include "ability.h"
#include "hilog_wrapper.h"
#include "../include/fuzz_test_config_parser.h"
#include "../include/get_param.h"
#include "../include/fuzz_test_manager.h"

#undef private
#undef protected
using namespace OHOS::AppExecFwk;
namespace OHOS {
namespace AppExecFwk {
std::shared_ptr<FuzzTestManager> FuzzTestManager::fuzzTestInstance = nullptr;
const std::string formManagerAbilityKitName = "FormManagerAbility";

// RegisterFormManagerAbility
void FuzzTestManager::RegisterFormManagerAbility()
{
    callFunctionMap_[formManagerAbilityKitName + "ReleaseForm"] = []() {
        std::shared_ptr<AppExecFwk::Ability> formManagerAbility = GetParamAbility();
        formManagerAbility->ReleaseForm(GetS64Param(), GetBoolParam());
    };
    callFunctionMap_[formManagerAbilityKitName + "DeleteForm"] = []() {
        std::shared_ptr<AppExecFwk::Ability> formManagerAbility = GetParamAbility();
        formManagerAbility->DeleteForm(GetS64Param());
    };
    callFunctionMap_[formManagerAbilityKitName + "AcquireForm"] = []() {
        std::shared_ptr<AppExecFwk::Ability> formManagerAbility = GetParamAbility();
        formManagerAbility->AcquireForm(GetS64Param(), GetParamWant(), GetParamFormCallback());
    };
    callFunctionMap_[formManagerAbilityKitName + "UpdateForm"] = []() {
        std::shared_ptr<AppExecFwk::Ability> formManagerAbility = GetParamAbility();
        formManagerAbility->UpdateForm(GetS64Param(), GetParamFormProviderData());
    };
    callFunctionMap_[formManagerAbilityKitName + "CastTempForm"] = []() {
        std::shared_ptr<AppExecFwk::Ability> formManagerAbility = GetParamAbility();
        formManagerAbility->CastTempForm(GetS64Param());
    };
    callFunctionMap_[formManagerAbilityKitName + "NotifyVisibleForms"] = []() {
        std::shared_ptr<AppExecFwk::Ability> formManagerAbility = GetParamAbility();
        formManagerAbility->NotifyVisibleForms(GetS64VectorParam());
    };
    callFunctionMap_[formManagerAbilityKitName + "NotifyInvisibleForms"] = []() {
        std::shared_ptr<AppExecFwk::Ability> formManagerAbility = GetParamAbility();
        formManagerAbility->NotifyInvisibleForms(GetS64VectorParam());
    };
    callFunctionMap_[formManagerAbilityKitName + "SetFormNextRefreshTime"] = []() {
        std::shared_ptr<AppExecFwk::Ability> formManagerAbility = GetParamAbility();
        formManagerAbility->SetFormNextRefreshTime(GetS64Param(), GetS64Param());
    };
    callFunctionMap_[formManagerAbilityKitName + "RequestForm"] = []() {
        std::shared_ptr<AppExecFwk::Ability> formManagerAbility = GetParamAbility();
        formManagerAbility->RequestForm(GetS64Param());
    };
    callFunctionMap_[formManagerAbilityKitName + "EnableUpdateForm"] = []() {
        std::shared_ptr<AppExecFwk::Ability> formManagerAbility = GetParamAbility();
        formManagerAbility->EnableUpdateForm(GetS64VectorParam());
    };
    callFunctionMap_[formManagerAbilityKitName + "DisableUpdateForm"] = []() {
        std::shared_ptr<AppExecFwk::Ability> formManagerAbility = GetParamAbility();
        formManagerAbility->DisableUpdateForm(GetS64VectorParam());
    };
    callFunctionMap_[formManagerAbilityKitName + "CheckFMSReady"] = []() {
        std::shared_ptr<AppExecFwk::Ability> formManagerAbility = GetParamAbility();
        formManagerAbility->CheckFMSReady();
    };
    callFunctionMap_[formManagerAbilityKitName + "GetAllFormsInfo"] = []() {
        std::shared_ptr<AppExecFwk::Ability> formManagerAbility = GetParamAbility();
        std::vector<OHOS::AppExecFwk::FormInfo> formInfos;
        formManagerAbility->GetAllFormsInfo(formInfos);
    };
    callFunctionMap_[formManagerAbilityKitName + "GetFormsInfoByApp"] = []() {
        std::shared_ptr<AppExecFwk::Ability> formManagerAbility = GetParamAbility();
        string bundleName = GetStringParam();
        std::vector<OHOS::AppExecFwk::FormInfo> formInfos;
        formManagerAbility->GetFormsInfoByApp(bundleName, formInfos);
    };
    callFunctionMap_[formManagerAbilityKitName + "GetFormsInfoByModule"] = []() {
        std::shared_ptr<AppExecFwk::Ability> formManagerAbility = GetParamAbility();
        string bundleName = GetStringParam();
        string moduleName = GetStringParam();
        std::vector<OHOS::AppExecFwk::FormInfo> formInfos;
        formManagerAbility->GetFormsInfoByModule(bundleName, moduleName, formInfos);
    };
        callFunctionMap_[formManagerAbilityKitName + "ProcessFormUpdate"] = []() {
        std::shared_ptr<AppExecFwk::Ability> formManagerAbility = GetParamAbility();
        formManagerAbility->ProcessFormUpdate(GetParamFormJsInfo());
    };
    callFunctionMap_[formManagerAbilityKitName + "ProcessFormUninstall"] = []() {
        std::shared_ptr<AppExecFwk::Ability> formManagerAbility = GetParamAbility();
        formManagerAbility->ProcessFormUninstall(GetS64Param());
    };
    callFunctionMap_[formManagerAbilityKitName + "OnDeathReceived"] = []() {
        std::shared_ptr<AppExecFwk::Ability> formManagerAbility = GetParamAbility();
        formManagerAbility->OnDeathReceived();
    };
    callFunctionMap_[formManagerAbilityKitName + "OnCreate"] = []() {
        std::shared_ptr<AppExecFwk::Ability> formManagerAbility = GetParamAbility();
        formManagerAbility->OnCreate(GetParamWant());
    };
    callFunctionMap_[formManagerAbilityKitName + "OnDelete"] = []() {
        std::shared_ptr<AppExecFwk::Ability> formManagerAbility = GetParamAbility();
        formManagerAbility->OnDelete(GetS64Param());
    };
    callFunctionMap_[formManagerAbilityKitName + "OnCastTemptoNormal"] = []() {
        std::shared_ptr<AppExecFwk::Ability> formManagerAbility = GetParamAbility();
        formManagerAbility->OnCastTemptoNormal(GetS64Param());
    };
    callFunctionMap_[formManagerAbilityKitName + "OnUpdate"] = []() {
        std::shared_ptr<AppExecFwk::Ability> formManagerAbility = GetParamAbility();
        formManagerAbility->OnUpdate(GetS64Param());
    };
    callFunctionMap_[formManagerAbilityKitName + "OnVisibilityChanged"] = []() {
        std::shared_ptr<AppExecFwk::Ability> formManagerAbility = GetParamAbility();
        formManagerAbility->OnVisibilityChanged(GetS64S32MapParam());
    };
    callFunctionMap_[formManagerAbilityKitName + "OnTriggerEvent"] = []() {
        std::shared_ptr<AppExecFwk::Ability> formManagerAbility = GetParamAbility();
        formManagerAbility->OnTriggerEvent(GetS64Param(), GetStringParam());
    };
}

FuzzTestManager::FuzzTestManager()
{
    RegisterFormManagerAbility();
}

void FuzzTestManager::SetJsonFunction(std::string functionName)
{
    remainderMap_.emplace(functionName, m_executionTimes);
}

void FuzzTestManager::SetExecutionTimes(uint16_t executionTimes)
{
    m_executionTimes = executionTimes;
    for_each(remainderMap_.begin(), remainderMap_.end(),
        [executionTimes](std::unordered_map<std::string, int>::reference executionTimesTemp) {
        executionTimesTemp.second = executionTimes;
    });
}

int GetRandomInt(int minNum, int maxNum)
{
    return GetU16Param() % (maxNum - minNum + 1) + minNum;
}

void action(int a)
{
    std::cout << "Interrupt signal (" << a << ") received.\n";
}

void FuzzTestManager::StartFuzzTest()
{
    std::cout << __func__ << std::endl;
    OHOS::FuzzTestConfigParser jsonParser;
    OHOS::FuzzTestData tempData;

    std::cout << "parseFromFile start" << std::endl;
    jsonParser.ParseForFuzzTest(FUZZ_TEST_CONFIG_FILE_PATH, tempData);
    std::cout << " ExecutionTimes :" << tempData.mainLoopFlag << std::endl;
    for_each(tempData.methodVec.begin(), tempData.methodVec.end(), [this](std::vector<std::string>::reference s) {
        SetJsonFunction(s);
    });
    SetExecutionTimes(tempData.mainLoopFlag);

    std::vector<std::string> index;
    std::unordered_map<std::string, int>::iterator it = remainderMap_.begin();
    while (it != remainderMap_.end()) {
        if (it->second <= 0) {
            it = remainderMap_.erase(it);
        } else {
            index.push_back(it->first);
            it++;
        }
    }

    std::cout << remainderMap_.size() << "--------fuzz test start--------" << callFunctionMap_.size() << std::endl;
    for (;remainderMap_.size() > 0;) {
        std::string functionName;
        int offset = GetRandomInt(0, index.size() - 1);
        functionName = index[offset];
        if (callFunctionMap_.find(functionName) != callFunctionMap_.end()) {
            std::cout << "call function : " << functionName << std::endl;
            callFunctionMap_[functionName]();
            std::cout << "function end  :" << functionName << std::endl;
        } else {
            std::cout << "can't find function : " << functionName << std::endl;
        }
        remainderMap_[functionName]--;
        if (remainderMap_[functionName] <= 0) {
            remainderMap_.erase(functionName);
            index.erase(index.begin() + offset);
        };
    }
    std::cout << remainderMap_.size() << "--------fuzz test end--------" << std::endl;
}
}  // namespace AppExecFwk
}  // namespace OHOS
