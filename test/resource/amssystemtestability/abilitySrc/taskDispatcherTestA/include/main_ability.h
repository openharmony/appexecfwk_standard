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

#ifndef AMS_KIT_SYSTEM_TEST_A_MAIN_ABILITY_H
#define AMS_KIT_SYSTEM_TEST_A_MAIN_ABILITY_H
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
        mapCase_[(int)TestFunc::GLOBAL] = {
            [this](int code) { GlobalCase1(code); },
            [this](int code) { GlobalCase2(code); },
            [this](int code) { GlobalCase3(code); },
            [this](int code) { GlobalCase4(code); },
            [this](int code) { GlobalCase5(code); },
            [this](int code) { GlobalCase6(code); },
            [this](int code) { GlobalCase7(code); },
            [this](int code) { GlobalCase8(code); },
            [this](int code) { GlobalCase9(code); },
            [this](int code) { GlobalCase10(code); },
            [this](int code) { GlobalCase11(code); },
            [this](int code) { GlobalCase12(code); },
            [this](int code) { GlobalCase13(code); },
            [this](int code) { GlobalCase14(code); },
            [this](int code) { GlobalCase15(code); },
            [this](int code) { GlobalCase16(code); },
            [this](int code) { GlobalCase17(code); },
            [this](int code) { GlobalCase18(code); },
            [this](int code) { GlobalCase19(code); },
            [this](int code) { GlobalCase20(code); },
            [this](int code) { GlobalCase21(code); },
            [this](int code) { GlobalCase22(code); },
            [this](int code) { GlobalCase23(code); },
            [this](int code) { GlobalCase24(code); },
            [this](int code) { GlobalCase25(code); },
            [this](int code) { GlobalCase26(code); },
            [this](int code) { GlobalCase27(code); },
            [this](int code) { GlobalCase28(code); },
            [this](int code) { GlobalCase29(code); },
            [this](int code) { GlobalCase30(code); },
            [this](int code) { GlobalCase31(code); },
            [this](int code) { GlobalCase32(code); },
            [this](int code) { GlobalCase33(code); },
            [this](int code) { GlobalCase34(code); },
            [this](int code) { GlobalCase35(code); },
            [this](int code) { GlobalCase36(code); },
            [this](int code) { GlobalCase37(code); },
            [this](int code) { GlobalCase38(code); },
            [this](int code) { GlobalCase39(code); },
            [this](int code) { GlobalCase40(code); },
            [this](int code) { GlobalCase41(code); },
            [this](int code) { GlobalCase42(code); },
            [this](int code) { GlobalCase43(code); },
            [this](int code) { GlobalCase44(code); },
            [this](int code) { GlobalCase45(code); },
            [this](int code) { GlobalCase46(code); },
            [this](int code) { GlobalCase47(code); },
            [this](int code) { GlobalCase48(code); },
            [this](int code) { GlobalCase49(code); },
            [this](int code) { GlobalCase50(code); },
            [this](int code) { GlobalCase51(code); },
            [this](int code) { GlobalCase52(code); },
            [this](int code) { GlobalCase53(code); },
            [this](int code) { GlobalCase54(code); },
            [this](int code) { GlobalCase55(code); },
            [this](int code) { GlobalCase56(code); },
            [this](int code) { GlobalCase57(code); },
            [this](int code) { GlobalCase58(code); },
            [this](int code) { GlobalCase59(code); },
            [this](int code) { GlobalCase60(code); },
            [this](int code) { GlobalCase61(code); },
            [this](int code) { GlobalCase62(code); },
            [this](int code) { GlobalCase63(code); },
            [this](int code) { GlobalCase64(code); },
            [this](int code) { GlobalCase65(code); },
            [this](int code) { GlobalCase66(code); },
            [this](int code) { GlobalCase67(code); },
            [this](int code) { GlobalCase68(code); },
            [this](int code) { GlobalCase69(code); },
            [this](int code) { GlobalCase70(code); },
            [this](int code) { GlobalCase71(code); },
            [this](int code) { GlobalCase72(code); },
            [this](int code) { GlobalCase73(code); },
            [this](int code) { GlobalCase74(code); },
            [this](int code) { GlobalCase75(code); },
            [this](int code) { GlobalCase76(code); },
            [this](int code) { GlobalCase77(code); },
            [this](int code) { GlobalCase78(code); },
            [this](int code) { GlobalCase79(code); },
            [this](int code) { GlobalCase80(code); },
            [this](int code) { GlobalCase81(code); },
            [this](int code) { GlobalCase82(code); },
            [this](int code) { GlobalCase83(code); },
            [this](int code) { GlobalCase84(code); },
            [this](int code) { GlobalCase85(code); },
            [this](int code) { GlobalCase86(code); },
            [this](int code) { GlobalCase87(code); },
            [this](int code) { GlobalCase88(code); },
            [this](int code) { GlobalCase89(code); },
            [this](int code) { GlobalCase90(code); },
            [this](int code) { GlobalCase91(code); },
        };
    }

    void initParallel()
    {
        mapCase_[(int)TestFunc::PARALLEL] = {
            [this](int code) { ParallelCase1(code); },
            [this](int code) { ParallelCase2(code); },
            [this](int code) { ParallelCase3(code); },
            [this](int code) { ParallelCase4(code); },
            [this](int code) { ParallelCase5(code); },
            [this](int code) { ParallelCase6(code); },
            [this](int code) { ParallelCase7(code); },
            [this](int code) { ParallelCase8(code); },
            [this](int code) { ParallelCase9(code); },
            [this](int code) { ParallelCase10(code); },
            [this](int code) { ParallelCase11(code); },
            [this](int code) { ParallelCase12(code); },
            [this](int code) { ParallelCase13(code); },
            [this](int code) { ParallelCase14(code); },
            [this](int code) { ParallelCase15(code); },
            [this](int code) { ParallelCase16(code); },
            [this](int code) { ParallelCase17(code); },
            [this](int code) { ParallelCase18(code); },
            [this](int code) { ParallelCase19(code); },
            [this](int code) { ParallelCase20(code); },
            [this](int code) { ParallelCase21(code); },
            [this](int code) { ParallelCase22(code); },
            [this](int code) { ParallelCase23(code); },
            [this](int code) { ParallelCase24(code); },
            [this](int code) { ParallelCase25(code); },
            [this](int code) { ParallelCase26(code); },
            [this](int code) { ParallelCase27(code); },
            [this](int code) { ParallelCase28(code); },
            [this](int code) { ParallelCase29(code); },
            [this](int code) { ParallelCase30(code); },
            [this](int code) { ParallelCase31(code); },
            [this](int code) { ParallelCase32(code); },
            [this](int code) { ParallelCase33(code); },
            [this](int code) { ParallelCase34(code); },
            [this](int code) { ParallelCase35(code); },
            [this](int code) { ParallelCase36(code); },
            [this](int code) { ParallelCase37(code); },
            [this](int code) { ParallelCase38(code); },
            [this](int code) { ParallelCase39(code); },
            [this](int code) { ParallelCase40(code); },
            [this](int code) { ParallelCase41(code); },
            [this](int code) { ParallelCase42(code); },
            [this](int code) { ParallelCase43(code); },
            [this](int code) { ParallelCase44(code); },
            [this](int code) { ParallelCase45(code); },
            [this](int code) { ParallelCase46(code); },
            [this](int code) { ParallelCase47(code); },
            [this](int code) { ParallelCase48(code); },
            [this](int code) { ParallelCase49(code); },
            [this](int code) { ParallelCase50(code); },
            [this](int code) { ParallelCase51(code); },
            [this](int code) { ParallelCase52(code); },
            [this](int code) { ParallelCase53(code); },
            [this](int code) { ParallelCase54(code); },
            [this](int code) { ParallelCase55(code); },
            [this](int code) { ParallelCase56(code); },
            [this](int code) { ParallelCase57(code); },
            [this](int code) { ParallelCase58(code); },
            [this](int code) { ParallelCase59(code); },
            [this](int code) { ParallelCase60(code); },
            [this](int code) { ParallelCase61(code); },
            [this](int code) { ParallelCase62(code); },
            [this](int code) { ParallelCase63(code); },
            [this](int code) { ParallelCase64(code); },
            [this](int code) { ParallelCase65(code); },
            [this](int code) { ParallelCase66(code); },
            [this](int code) { ParallelCase67(code); },
            [this](int code) { ParallelCase68(code); },
            [this](int code) { ParallelCase69(code); },
            [this](int code) { ParallelCase70(code); },
            [this](int code) { ParallelCase71(code); },
            [this](int code) { ParallelCase72(code); },
            [this](int code) { ParallelCase73(code); },
            [this](int code) { ParallelCase74(code); },
            [this](int code) { ParallelCase75(code); },
            [this](int code) { ParallelCase76(code); },
            [this](int code) { ParallelCase77(code); },
            [this](int code) { ParallelCase78(code); },
            [this](int code) { ParallelCase79(code); },
            [this](int code) { ParallelCase80(code); },
            [this](int code) { ParallelCase81(code); },
            [this](int code) { ParallelCase82(code); },
            [this](int code) { ParallelCase83(code); },
            [this](int code) { ParallelCase84(code); },
            [this](int code) { ParallelCase85(code); },
            [this](int code) { ParallelCase86(code); },
            [this](int code) { ParallelCase87(code); },
            [this](int code) { ParallelCase88(code); },
            [this](int code) { ParallelCase89(code); },
            [this](int code) { ParallelCase90(code); },
            [this](int code) { ParallelCase91(code); },
            [this](int code) { ParallelCase92(code); },
            [this](int code) { ParallelCase93(code); },
            [this](int code) { ParallelCase94(code); },
            [this](int code) { ParallelCase95(code); },
            [this](int code) { ParallelCase96(code); },
            [this](int code) { ParallelCase97(code); },
            [this](int code) { ParallelCase98(code); },
            [this](int code) { ParallelCase99(code); },
            [this](int code) { ParallelCase100(code); },
            [this](int code) { ParallelCase101(code); },
            [this](int code) { ParallelCase102(code); },
            [this](int code) { ParallelCase103(code); },
            [this](int code) { ParallelCase104(code); },
            [this](int code) { ParallelCase105(code); },
            [this](int code) { ParallelCase106(code); },
            [this](int code) { ParallelCase107(code); },
            [this](int code) { ParallelCase108(code); },
            [this](int code) { ParallelCase109(code); },
            [this](int code) { ParallelCase110(code); },
            [this](int code) { ParallelCase111(code); },
            [this](int code) { ParallelCase112(code); },
            [this](int code) { ParallelCase113(code); },
            [this](int code) { ParallelCase114(code); },
            [this](int code) { ParallelCase115(code); },
            [this](int code) { ParallelCase116(code); },
            [this](int code) { ParallelCase117(code); },
        };
    }
    void initSerial()
    {
        mapCase_[(int)TestFunc::SERIAL] = {
            [this](int code) { SerialCase1(code); },
            [this](int code) { SerialCase2(code); },
            [this](int code) { SerialCase3(code); },
            [this](int code) { SerialCase4(code); },
            [this](int code) { SerialCase5(code); },
            [this](int code) { SerialCase6(code); },
            [this](int code) { SerialCase7(code); },
            [this](int code) { SerialCase8(code); },
            [this](int code) { SerialCase9(code); },
            [this](int code) { SerialCase10(code); },
            [this](int code) { SerialCase11(code); },
            [this](int code) { SerialCase12(code); },
            [this](int code) { SerialCase13(code); },
            [this](int code) { SerialCase14(code); },
            [this](int code) { SerialCase15(code); },
            [this](int code) { SerialCase16(code); },
            [this](int code) { SerialCase17(code); },
            [this](int code) { SerialCase18(code); },
            [this](int code) { SerialCase19(code); },
            [this](int code) { SerialCase20(code); },
            [this](int code) { SerialCase21(code); },
            [this](int code) { SerialCase22(code); },
            [this](int code) { SerialCase23(code); },
            [this](int code) { SerialCase24(code); },
            [this](int code) { SerialCase25(code); },
            [this](int code) { SerialCase26(code); },
            [this](int code) { SerialCase27(code); },
            [this](int code) { SerialCase28(code); },
            [this](int code) { SerialCase29(code); },
            [this](int code) { SerialCase30(code); },
            [this](int code) { SerialCase31(code); },
            [this](int code) { SerialCase32(code); },
            [this](int code) { SerialCase33(code); },
            [this](int code) { SerialCase34(code); },
            [this](int code) { SerialCase35(code); },
            [this](int code) { SerialCase36(code); },
            [this](int code) { SerialCase37(code); },
            [this](int code) { SerialCase38(code); },
            [this](int code) { SerialCase39(code); },
            [this](int code) { SerialCase40(code); },
            [this](int code) { SerialCase41(code); },
            [this](int code) { SerialCase42(code); },
            [this](int code) { SerialCase43(code); },
            [this](int code) { SerialCase44(code); },
            [this](int code) { SerialCase45(code); },
            [this](int code) { SerialCase46(code); },
            [this](int code) { SerialCase47(code); },
            [this](int code) { SerialCase48(code); },
            [this](int code) { SerialCase49(code); },
            [this](int code) { SerialCase50(code); },
            [this](int code) { SerialCase51(code); },
            [this](int code) { SerialCase52(code); },
        };
    }
    void initSpec()
    {
        mapCase_[(int)TestFunc::SPEC] = {
            [this](int code) { SpecCase1(code); },
            [this](int code) { SpecCase2(code); },
            [this](int code) { SpecCase3(code); },
            [this](int code) { SpecCase4(code); },
            [this](int code) { SpecCase5(code); },
            [this](int code) { SpecCase6(code); },
            [this](int code) { SpecCase7(code); },
            [this](int code) { SpecCase8(code); },
            [this](int code) { SpecCase9(code); },
            [this](int code) { SpecCase10(code); },
            [this](int code) { SpecCase11(code); },
            [this](int code) { SpecCase12(code); },
            [this](int code) { SpecCase13(code); },
            [this](int code) { SpecCase14(code); },
            [this](int code) { SpecCase15(code); },
            [this](int code) { SpecCase16(code); },
            [this](int code) { SpecCase17(code); },
            [this](int code) { SpecCase18(code); },
            [this](int code) { SpecCase19(code); },
            [this](int code) { SpecCase20(code); },
            [this](int code) { SpecCase21(code); },
            [this](int code) { SpecCase22(code); },
            [this](int code) { SpecCase23(code); },
            [this](int code) { SpecCase24(code); },
            [this](int code) { SpecCase25(code); },
            [this](int code) { SpecCase26(code); },
            [this](int code) { SpecCase27(code); },
            [this](int code) { SpecCase28(code); },
            [this](int code) { SpecCase29(code); },
            [this](int code) { SpecCase30(code); },
            [this](int code) { SpecCase31(code); },
            [this](int code) { SpecCase32(code); },
            [this](int code) { SpecCase33(code); },
            [this](int code) { SpecCase34(code); },
            [this](int code) { SpecCase35(code); },
            [this](int code) { SpecCase36(code); },
            [this](int code) { SpecCase37(code); },
            [this](int code) { SpecCase38(code); },
            [this](int code) { SpecCase39(code); },
            [this](int code) { SpecCase40(code); },
            [this](int code) { SpecCase41(code); },
            [this](int code) { SpecCase42(code); },
            [this](int code) { SpecCase43(code); },
            [this](int code) { SpecCase44(code); },
            [this](int code) { SpecCase45(code); },
            [this](int code) { SpecCase46(code); },
            [this](int code) { SpecCase47(code); },
            [this](int code) { SpecCase48(code); },
            [this](int code) { SpecCase49(code); },
            [this](int code) { SpecCase50(code); },
            [this](int code) { SpecCase51(code); },
            [this](int code) { SpecCase52(code); },
        };
    }
    void initHybrid()
    {
        mapCase_[(int)TestFunc::HYBRID] = {
            [this](int code) { HybridCase1(code); },
            [this](int code) { HybridCase2(code); },
            [this](int code) { HybridCase3(code); },
            [this](int code) { HybridCase4(code); },
            [this](int code) { HybridCase5(code); },
            [this](int code) { HybridCase6(code); },
            [this](int code) { HybridCase7(code); },
            [this](int code) { HybridCase8(code); },
            [this](int code) { HybridCase9(code); },
            [this](int code) { HybridCase10(code); },
            [this](int code) { HybridCase11(code); },
            [this](int code) { HybridCase12(code); },
            [this](int code) { HybridCase13(code); },
            [this](int code) { HybridCase14(code); },
            [this](int code) { HybridCase15(code); },
            [this](int code) { HybridCase16(code); },
            [this](int code) { HybridCase17(code); },
            [this](int code) { HybridCase18(code); },
            [this](int code) { HybridCase19(code); },
            [this](int code) { HybridCase20(code); },
            [this](int code) { HybridCase21(code); },
            [this](int code) { HybridCase22(code); },
            [this](int code) { HybridCase23(code); },
            [this](int code) { HybridCase24(code); },
            [this](int code) { HybridCase25(code); },
            [this](int code) { HybridCase26(code); },
            [this](int code) { HybridCase27(code); },
        };
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
        mapCase_[(int)TestFunc::PRIORITY] = {
            [this](int code) { PriorityCase1(code); },
            [this](int code) { PriorityCase2(code); },
            [this](int code) { PriorityCase3(code); },
            [this](int code) { PriorityCase4(code); },
            [this](int code) { PriorityCase5(code); },
        };
    }
    void initCancelTask()
    {
        mapCase_[(int)TestFunc::CANCEL_TASK] = {
            [this](int code) { RevokeCase1(code); },
            [this](int code) { RevokeCase2(code); },
            [this](int code) { RevokeCase3(code); },
            [this](int code) { RevokeCase4(code); },
            [this](int code) { RevokeCase5(code); },
            [this](int code) { RevokeCase6(code); },
        };
    }
    void initExtra()
    {
        mapCase_[(int)TestFunc::EXTRA] = {
            [this](int code) { ExtraCase1(code); },
        };
    }

    void SubscribeEvent();
    void TestDispatcher(int apiIndex, int caseIndex, int code);
    int Dispatch(STtools::TestSetting outer, STtools::TestSetting inner);

    void GlobalCase1(int code);
    void GlobalCase2(int code);
    void GlobalCase3(int code);
    void GlobalCase4(int code);
    void GlobalCase5(int code);
    void GlobalCase6(int code);
    void GlobalCase7(int code);
    void GlobalCase8(int code);
    void GlobalCase9(int code);
    void GlobalCase10(int code);
    void GlobalCase11(int code);
    void GlobalCase12(int code);
    void GlobalCase13(int code);
    void GlobalCase14(int code);
    void GlobalCase15(int code);
    void GlobalCase16(int code);
    void GlobalCase17(int code);
    void GlobalCase18(int code);
    void GlobalCase19(int code);
    void GlobalCase20(int code);
    void GlobalCase21(int code);
    void GlobalCase22(int code);
    void GlobalCase23(int code);
    void GlobalCase24(int code);
    void GlobalCase25(int code);
    void GlobalCase26(int code);
    void GlobalCase27(int code);
    void GlobalCase28(int code);
    void GlobalCase29(int code);
    void GlobalCase30(int code);
    void GlobalCase31(int code);
    void GlobalCase32(int code);
    void GlobalCase33(int code);
    void GlobalCase34(int code);
    void GlobalCase35(int code);
    void GlobalCase36(int code);
    void GlobalCase37(int code);
    void GlobalCase38(int code);
    void GlobalCase39(int code);
    void GlobalCase40(int code);
    void GlobalCase41(int code);
    void GlobalCase42(int code);
    void GlobalCase43(int code);
    void GlobalCase44(int code);
    void GlobalCase45(int code);
    void GlobalCase46(int code);
    void GlobalCase47(int code);
    void GlobalCase48(int code);
    void GlobalCase49(int code);
    void GlobalCase50(int code);
    void GlobalCase51(int code);
    void GlobalCase52(int code);
    void GlobalCase53(int code);
    void GlobalCase54(int code);
    void GlobalCase55(int code);
    void GlobalCase56(int code);
    void GlobalCase57(int code);
    void GlobalCase58(int code);
    void GlobalCase59(int code);
    void GlobalCase60(int code);
    void GlobalCase61(int code);
    void GlobalCase62(int code);
    void GlobalCase63(int code);
    void GlobalCase64(int code);
    void GlobalCase65(int code);
    void GlobalCase66(int code);
    void GlobalCase67(int code);
    void GlobalCase68(int code);
    void GlobalCase69(int code);
    void GlobalCase70(int code);
    void GlobalCase71(int code);
    void GlobalCase72(int code);
    void GlobalCase73(int code);
    void GlobalCase74(int code);
    void GlobalCase75(int code);
    void GlobalCase76(int code);
    void GlobalCase77(int code);
    void GlobalCase78(int code);
    void GlobalCase79(int code);
    void GlobalCase80(int code);
    void GlobalCase81(int code);
    void GlobalCase82(int code);
    void GlobalCase83(int code);
    void GlobalCase84(int code);
    void GlobalCase85(int code);
    void GlobalCase86(int code);
    void GlobalCase87(int code);
    void GlobalCase88(int code);
    void GlobalCase89(int code);
    void GlobalCase90(int code);
    void GlobalCase91(int code);
    void ParallelCase1(int code);
    void ParallelCase2(int code);
    void ParallelCase3(int code);
    void ParallelCase4(int code);
    void ParallelCase5(int code);
    void ParallelCase6(int code);
    void ParallelCase7(int code);
    void ParallelCase8(int code);
    void ParallelCase9(int code);
    void ParallelCase10(int code);
    void ParallelCase11(int code);
    void ParallelCase12(int code);
    void ParallelCase13(int code);
    void ParallelCase14(int code);
    void ParallelCase15(int code);
    void ParallelCase16(int code);
    void ParallelCase17(int code);
    void ParallelCase18(int code);
    void ParallelCase19(int code);
    void ParallelCase20(int code);
    void ParallelCase21(int code);
    void ParallelCase22(int code);
    void ParallelCase23(int code);
    void ParallelCase24(int code);
    void ParallelCase25(int code);
    void ParallelCase26(int code);
    void ParallelCase27(int code);
    void ParallelCase28(int code);
    void ParallelCase29(int code);
    void ParallelCase30(int code);
    void ParallelCase31(int code);
    void ParallelCase32(int code);
    void ParallelCase33(int code);
    void ParallelCase34(int code);
    void ParallelCase35(int code);
    void ParallelCase36(int code);
    void ParallelCase37(int code);
    void ParallelCase38(int code);
    void ParallelCase39(int code);
    void ParallelCase40(int code);
    void ParallelCase41(int code);
    void ParallelCase42(int code);
    void ParallelCase43(int code);
    void ParallelCase44(int code);
    void ParallelCase45(int code);
    void ParallelCase46(int code);
    void ParallelCase47(int code);
    void ParallelCase48(int code);
    void ParallelCase49(int code);
    void ParallelCase50(int code);
    void ParallelCase51(int code);
    void ParallelCase52(int code);
    void ParallelCase53(int code);
    void ParallelCase54(int code);
    void ParallelCase55(int code);
    void ParallelCase56(int code);
    void ParallelCase57(int code);
    void ParallelCase58(int code);
    void ParallelCase59(int code);
    void ParallelCase60(int code);
    void ParallelCase61(int code);
    void ParallelCase62(int code);
    void ParallelCase63(int code);
    void ParallelCase64(int code);
    void ParallelCase65(int code);
    void ParallelCase66(int code);
    void ParallelCase67(int code);
    void ParallelCase68(int code);
    void ParallelCase69(int code);
    void ParallelCase70(int code);
    void ParallelCase71(int code);
    void ParallelCase72(int code);
    void ParallelCase73(int code);
    void ParallelCase74(int code);
    void ParallelCase75(int code);
    void ParallelCase76(int code);
    void ParallelCase77(int code);
    void ParallelCase78(int code);
    void ParallelCase79(int code);
    void ParallelCase80(int code);
    void ParallelCase81(int code);
    void ParallelCase82(int code);
    void ParallelCase83(int code);
    void ParallelCase84(int code);
    void ParallelCase85(int code);
    void ParallelCase86(int code);
    void ParallelCase87(int code);
    void ParallelCase88(int code);
    void ParallelCase89(int code);
    void ParallelCase90(int code);
    void ParallelCase91(int code);
    void ParallelCase92(int code);
    void ParallelCase93(int code);
    void ParallelCase94(int code);
    void ParallelCase95(int code);
    void ParallelCase96(int code);
    void ParallelCase97(int code);
    void ParallelCase98(int code);
    void ParallelCase99(int code);
    void ParallelCase100(int code);
    void ParallelCase101(int code);
    void ParallelCase102(int code);
    void ParallelCase103(int code);
    void ParallelCase104(int code);
    void ParallelCase105(int code);
    void ParallelCase106(int code);
    void ParallelCase107(int code);
    void ParallelCase108(int code);
    void ParallelCase109(int code);
    void ParallelCase110(int code);
    void ParallelCase111(int code);
    void ParallelCase112(int code);
    void ParallelCase113(int code);
    void ParallelCase114(int code);
    void ParallelCase115(int code);
    void ParallelCase116(int code);
    void ParallelCase117(int code);
    void SerialCase1(int code);
    void SerialCase2(int code);
    void SerialCase3(int code);
    void SerialCase4(int code);
    void SerialCase5(int code);
    void SerialCase6(int code);
    void SerialCase7(int code);
    void SerialCase8(int code);
    void SerialCase9(int code);
    void SerialCase10(int code);
    void SerialCase11(int code);
    void SerialCase12(int code);
    void SerialCase13(int code);
    void SerialCase14(int code);
    void SerialCase15(int code);
    void SerialCase16(int code);
    void SerialCase17(int code);
    void SerialCase18(int code);
    void SerialCase19(int code);
    void SerialCase20(int code);
    void SerialCase21(int code);
    void SerialCase22(int code);
    void SerialCase23(int code);
    void SerialCase24(int code);
    void SerialCase25(int code);
    void SerialCase26(int code);
    void SerialCase27(int code);
    void SerialCase28(int code);
    void SerialCase29(int code);
    void SerialCase30(int code);
    void SerialCase31(int code);
    void SerialCase32(int code);
    void SerialCase33(int code);
    void SerialCase34(int code);
    void SerialCase35(int code);
    void SerialCase36(int code);
    void SerialCase37(int code);
    void SerialCase38(int code);
    void SerialCase39(int code);
    void SerialCase40(int code);
    void SerialCase41(int code);
    void SerialCase42(int code);
    void SerialCase43(int code);
    void SerialCase44(int code);
    void SerialCase45(int code);
    void SerialCase46(int code);
    void SerialCase47(int code);
    void SerialCase48(int code);
    void SerialCase49(int code);
    void SerialCase50(int code);
    void SerialCase51(int code);
    void SerialCase52(int code);
    void SpecCase1(int code);
    void SpecCase2(int code);
    void SpecCase3(int code);
    void SpecCase4(int code);
    void SpecCase5(int code);
    void SpecCase6(int code);
    void SpecCase7(int code);
    void SpecCase8(int code);
    void SpecCase9(int code);
    void SpecCase10(int code);
    void SpecCase11(int code);
    void SpecCase12(int code);
    void SpecCase13(int code);
    void SpecCase14(int code);
    void SpecCase15(int code);
    void SpecCase16(int code);
    void SpecCase17(int code);
    void SpecCase18(int code);
    void SpecCase19(int code);
    void SpecCase20(int code);
    void SpecCase21(int code);
    void SpecCase22(int code);
    void SpecCase23(int code);
    void SpecCase24(int code);
    void SpecCase25(int code);
    void SpecCase26(int code);
    void SpecCase27(int code);
    void SpecCase28(int code);
    void SpecCase29(int code);
    void SpecCase30(int code);
    void SpecCase31(int code);
    void SpecCase32(int code);
    void SpecCase33(int code);
    void SpecCase34(int code);
    void SpecCase35(int code);
    void SpecCase36(int code);
    void SpecCase37(int code);
    void SpecCase38(int code);
    void SpecCase39(int code);
    void SpecCase40(int code);
    void SpecCase41(int code);
    void SpecCase42(int code);
    void SpecCase43(int code);
    void SpecCase44(int code);
    void SpecCase45(int code);
    void SpecCase46(int code);
    void SpecCase47(int code);
    void SpecCase48(int code);
    void SpecCase49(int code);
    void SpecCase50(int code);
    void SpecCase51(int code);
    void SpecCase52(int code);
    void HybridCase1(int code);
    void HybridCase2(int code);
    void HybridCase3(int code);
    void HybridCase4(int code);
    void HybridCase5(int code);
    void HybridCase6(int code);
    void HybridCase7(int code);
    void HybridCase8(int code);
    void HybridCase9(int code);
    void HybridCase10(int code);
    void HybridCase11(int code);
    void HybridCase12(int code);
    void HybridCase13(int code);
    void HybridCase14(int code);
    void HybridCase15(int code);
    void HybridCase16(int code);
    void HybridCase17(int code);
    void HybridCase18(int code);
    void HybridCase19(int code);
    void HybridCase20(int code);
    void HybridCase21(int code);
    void HybridCase22(int code);
    void HybridCase23(int code);
    void HybridCase24(int code);
    void HybridCase25(int code);
    void HybridCase26(int code);
    void HybridCase27(int code);
    void MultiAppCase1(int code);
    void MultiAppCase2(int code);
    void MultiAppCase3(int code);
    void MultiAppCase4(int code);
    void ExtraCase1(int code);
    void PriorityCase1(int code);
    void PriorityCase2(int code);
    void PriorityCase3(int code);
    void PriorityCase4(int code);
    void PriorityCase5(int code);
    void RevokeCase1(int code);
    void RevokeCase2(int code);
    void RevokeCase3(int code);
    void RevokeCase4(int code);
    void RevokeCase5(int code);
    void RevokeCase6(int code);

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

    void FillInDispathcer();
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
#endif  // AMS_KIT_SYSTEM_TEST_A_MAIN_ABILITY_H