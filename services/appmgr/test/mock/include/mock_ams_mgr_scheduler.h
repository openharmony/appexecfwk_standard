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

#ifndef FOUNDATION_APPEXECFWK_SERVICES_APPMGR_TEST_UNITEST_MOCK_AMS_MGR_SCHEDULER_H
#define FOUNDATION_APPEXECFWK_SERVICES_APPMGR_TEST_UNITEST_MOCK_AMS_MGR_SCHEDULER_H

#include "gmock/gmock.h"
#include "ams_mgr_scheduler.h"

namespace OHOS {
namespace AppExecFwk {
class MockAmsMgrScheduler : public AmsMgrStub {
public:
    MOCK_METHOD5(LoadAbility,
        void(const sptr<IRemoteObject> &token, const sptr<IRemoteObject> &preToken,
            const std::shared_ptr<AbilityInfo> &abilityInfo, const std::shared_ptr<ApplicationInfo> &appInfo,
	    int32_t uid));
    MOCK_METHOD5(AbilityBehaviorAnalysis,
        void(const sptr<OHOS::IRemoteObject> &token, const sptr<OHOS::IRemoteObject> &preToken,
            const int32_t visibility, const int32_t perceptibility, const int32_t connectionState));
    MOCK_METHOD1(TerminateAbility, void(const sptr<IRemoteObject> &token));
    MOCK_METHOD2(UpdateAbilityState, void(const sptr<IRemoteObject> &token, const AbilityState state));
    MOCK_METHOD0(Reset, void());
    MOCK_METHOD1(KillProcessByAbilityToken, void(const sptr<IRemoteObject> &token));
    MOCK_METHOD1(KillApplication, int32_t(const std::string &bundleName));
    MOCK_METHOD2(KillApplicationByUid, int(const std::string &, const int uid));
    MOCK_METHOD0(IsReady, bool());
    MOCK_METHOD1(AbilityAttachTimeOut, void(const sptr<IRemoteObject> &token));
    MOCK_METHOD1(PrepareTerminate, void(const sptr<IRemoteObject> &token));
    MOCK_METHOD4(CompelVerifyPermission, int(const std::string &permission, int pid, int uid, std::string &message));
    MockAmsMgrScheduler() : AmsMgrStub() {};
    virtual ~MockAmsMgrScheduler(){};
    virtual void RegisterAppStateCallback(const sptr<IAppStateCallback> &callback) override
    {
        callback->OnAbilityRequestDone(nullptr, AbilityState::ABILITY_STATE_BACKGROUND);
        AppProcessData appProcessData;
        callback->OnAppStateChanged(appProcessData);
    }
};
}  // namespace AppExecFwk
}  // namespace OHOS

#endif
