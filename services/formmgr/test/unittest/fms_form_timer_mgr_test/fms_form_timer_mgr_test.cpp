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
#include <chrono>
#include <gtest/gtest.h>

#include "common_event.h"
#include "common_event_manager.h"
#include "common_event_support.h"
#include "form_constants.h"
#include "form_refresh_limiter.h"
#include "form_timer_mgr.h"

using namespace testing::ext;
using namespace OHOS;
using namespace OHOS::AppExecFwk;

namespace {
const int64_t PARAM_FORM_ID_VALUE_1 = 20210712;
const int64_t PARAM_FORM_ID_VALUE_2 = 20210713;
const int64_t PARAM_FORM_ID_VALUE_3 = 20210714;
const int64_t PARAM_FORM_ID_VALUE_4 = 20210715;
const int64_t PARAM_FORM_ID_VALUE_5 = 20210716;
const int64_t PARAM_FORM_ID_VALUE_6 = 20210717;

class FmsFormTimerMgrTest : public testing::Test {
public:
    static void SetUpTestCase();
    static void TearDownTestCase();
    void SetUp();
    void TearDown();

protected:
};

void FmsFormTimerMgrTest::SetUpTestCase() {}
void FmsFormTimerMgrTest::TearDownTestCase() {}
void FmsFormTimerMgrTest::SetUp() {}
void FmsFormTimerMgrTest::TearDown() {}

/**
 * @tc.number: Fms_FormTimerMgr_0001
 * @tc.name: AddFormTimer.
 * @tc.desc: Add duration form timer.
 */
HWTEST_F(FmsFormTimerMgrTest, Fms_FormTimerMgr_0001, Function | MediumTest | Level1)
{
    GTEST_LOG_(INFO) << "Fms_FormTimerMgr_0001 start";
    bool isOk = FormTimerMgr::GetInstance().AddFormTimer(PARAM_FORM_ID_VALUE_1,  1 * Constants::MIN_PERIOD);
    EXPECT_EQ(isOk, true);
    GTEST_LOG_(INFO) << "Fms_FormTimerMgr_0001 end";
}

/**
 * @tc.number: Fms_FormTimerMgr_0002
 * @tc.name: AddFormTimer.
 * @tc.desc: Add scheduled form timer.
 */
HWTEST_F(FmsFormTimerMgrTest, Fms_FormTimerMgr_0002, Function | MediumTest | Level1)
{
    GTEST_LOG_(INFO) << "Fms_FormTimerMgr_0002 start";
    bool isOk = FormTimerMgr::GetInstance().AddFormTimer(PARAM_FORM_ID_VALUE_2,  2, 50);
    EXPECT_EQ(isOk, true);
    GTEST_LOG_(INFO) << "Fms_FormTimerMgr_0002 end";
}

/**
 * @tc.number: Fms_FormTimerMgr_0003
 * @tc.name: RemoveFormTimer.
 * @tc.desc: Delete form timer.
 */
HWTEST_F(FmsFormTimerMgrTest, Fms_FormTimerMgr_0003, Function | MediumTest | Level1)
{
    GTEST_LOG_(INFO) << "Fms_FormTimerMgr_0003 start";
    bool isAddOk1 = FormTimerMgr::GetInstance().AddFormTimer(PARAM_FORM_ID_VALUE_3,  336 * Constants::MIN_PERIOD);
    EXPECT_EQ(isAddOk1, true);
    bool isAddOk2 = FormTimerMgr::GetInstance().AddFormTimer(PARAM_FORM_ID_VALUE_4,  3, 30);
    EXPECT_EQ(isAddOk2, true);
    bool isDelOk1 = FormTimerMgr::GetInstance().RemoveFormTimer(PARAM_FORM_ID_VALUE_3);
    EXPECT_EQ(isDelOk1, true);
    bool isDelOk2 = FormTimerMgr::GetInstance().RemoveFormTimer(PARAM_FORM_ID_VALUE_4);
    EXPECT_EQ(isDelOk2, true);
    GTEST_LOG_(INFO) << "Fms_FormTimerMgr_0003 end";
}
/**
 * @tc.number: Fms_FormTimerMgr_0004
 * @tc.name: UpdateFormTimer.
 * @tc.desc: Update form timer(TYPE_INTERVAL_CHANGE).
 */
HWTEST_F(FmsFormTimerMgrTest, Fms_FormTimerMgr_0004, Function | MediumTest | Level1)
{
    GTEST_LOG_(INFO) << "Fms_FormTimerMgr_0004 start";
    bool isAddOk1 = FormTimerMgr::GetInstance().AddFormTimer(PARAM_FORM_ID_VALUE_1,  3 * Constants::MIN_PERIOD);
    EXPECT_EQ(isAddOk1, true);

    // TYPE_INTERVAL_CHANGE
    FormTimerCfg timerCfg1;
    timerCfg1.enableUpdate = true;
    timerCfg1.updateDuration = 2 * Constants::MIN_PERIOD;
    bool isUpdateOk1 = FormTimerMgr::GetInstance().UpdateFormTimer(PARAM_FORM_ID_VALUE_1,
    UpdateType::TYPE_INTERVAL_CHANGE, timerCfg1);
    EXPECT_EQ(isUpdateOk1, true);
    GTEST_LOG_(INFO) << "Fms_FormTimerMgr_0004 end";
}
/**
 * @tc.number: Fms_FormTimerMgr_0005
 * @tc.name: UpdateFormTimer.
 * @tc.desc: Update form timer(TYPE_ATTIME_CHANGE).
 */
HWTEST_F(FmsFormTimerMgrTest, Fms_FormTimerMgr_0005, Function | MediumTest | Level1)
{
    GTEST_LOG_(INFO) << "Fms_FormTimerMgr_0005 start";
    bool isAddOk2 = FormTimerMgr::GetInstance().AddFormTimer(PARAM_FORM_ID_VALUE_2,  3, 30);
    EXPECT_EQ(isAddOk2, true);

    // TYPE_ATTIME_CHANGE
    FormTimerCfg timerCfg2;
    timerCfg2.enableUpdate = true;
    timerCfg2.updateAtHour = 6;
    timerCfg2.updateAtMin = 55;
    bool isUpdateOk2 = FormTimerMgr::GetInstance().UpdateFormTimer(PARAM_FORM_ID_VALUE_2,
    UpdateType::TYPE_ATTIME_CHANGE,  timerCfg2);
    EXPECT_EQ(isUpdateOk2, true);

    GTEST_LOG_(INFO) << "Fms_FormTimerMgr_0005 end";
}
/**
 * @tc.number: Fms_FormTimerMgr_0006
 * @tc.name: UpdateFormTimer.
 * @tc.desc: Update form timer(TYPE_INTERVAL_TO_ATTIME).
 */
HWTEST_F(FmsFormTimerMgrTest, Fms_FormTimerMgr_0006, Function | MediumTest | Level1)
{
    GTEST_LOG_(INFO) << "Fms_FormTimerMgr_0006 start";
    bool isAddOk3 = FormTimerMgr::GetInstance().AddFormTimer(PARAM_FORM_ID_VALUE_3,  6 * Constants::MIN_PERIOD);
    EXPECT_EQ(isAddOk3, true);

    // TYPE_INTERVAL_TO_ATTIME
    FormTimerCfg timerCfg3;
    timerCfg3.enableUpdate = true;
    timerCfg3.updateAtHour = 8;
    timerCfg3.updateAtMin = 25;
    bool isUpdateOk3 = FormTimerMgr::GetInstance().UpdateFormTimer(PARAM_FORM_ID_VALUE_3,
    UpdateType::TYPE_INTERVAL_TO_ATTIME,  timerCfg3);
    EXPECT_EQ(isUpdateOk3, true);
    GTEST_LOG_(INFO) << "Fms_FormTimerMgr_0006 end";
}
/**
 * @tc.number: Fms_FormTimerMgr_0007
 * @tc.name: UpdateFormTimer.
 * @tc.desc: Update form timer(TYPE_ATTIME_TO_INTERVAL).
 */
HWTEST_F(FmsFormTimerMgrTest, Fms_FormTimerMgr_0007, Function | MediumTest | Level1)
{
    GTEST_LOG_(INFO) << "Fms_FormTimerMgr_0007 start";
    bool isAddOk4 = FormTimerMgr::GetInstance().AddFormTimer(PARAM_FORM_ID_VALUE_4,  10, 30);
    EXPECT_EQ(isAddOk4, true);

    // TYPE_ATTIME_TO_INTERVAL
    FormTimerCfg timerCfg4;
    timerCfg4.enableUpdate = true;
    timerCfg4.updateDuration = 5 * Constants::MIN_PERIOD;
    bool isUpdateOk4 = FormTimerMgr::GetInstance().UpdateFormTimer(PARAM_FORM_ID_VALUE_4,
    UpdateType::TYPE_ATTIME_TO_INTERVAL,  timerCfg4);
    EXPECT_EQ(isUpdateOk4, true);
    GTEST_LOG_(INFO) << "Fms_FormTimerMgr_0007 end";
}
/**
 * @tc.number: Fms_FormTimerMgr_0008
 * @tc.name: HandleSystemTimeChanged.
 * @tc.desc: Handle system time changed.
 */
HWTEST_F(FmsFormTimerMgrTest, Fms_FormTimerMgr_0008, Function | MediumTest | Level1)
{
    GTEST_LOG_(INFO) << "Fms_FormTimerMgr_0008 start";
    bool isAddOk5 = FormTimerMgr::GetInstance().AddFormTimer(PARAM_FORM_ID_VALUE_1,  11, 30);
    EXPECT_EQ(isAddOk5, true);
    bool isAddOk6 = FormTimerMgr::GetInstance().HandleSystemTimeChanged();
    EXPECT_EQ(isAddOk6, true);

    GTEST_LOG_(INFO) << "Fms_FormTimerMgr_0008 end";
}

/**
 * @tc.number: Fms_FormTimerMgr_0009
 * @tc.name: TimerReceiver::OnReceiveEvent.
 * @tc.desc: Receive common event(COMMON_EVENT_TIMEZONE_CHANGED).
 */
HWTEST_F(FmsFormTimerMgrTest, Fms_FormTimerMgr_0009, Function | MediumTest | Level1)
{
    GTEST_LOG_(INFO) << "Fms_FormTimerMgr_0009 start";
    bool isAddOk5 = FormTimerMgr::GetInstance().AddFormTimer(PARAM_FORM_ID_VALUE_2,  11, 30);
    EXPECT_EQ(isAddOk5, true);
    bool isAddOk6 = FormTimerMgr::GetInstance().HandleSystemTimeChanged();
    EXPECT_EQ(isAddOk6, true);

    GTEST_LOG_(INFO) << "Fms_FormTimerMgr_0009 end";
}
/**
 * @tc.number: Fms_FormTimerMgr_0010
 * @tc.name: HandleResetLimiter.
 * @tc.desc: Handle reset limiter.
 */
HWTEST_F(FmsFormTimerMgrTest, Fms_FormTimerMgr_0010, Function | MediumTest | Level1)
{
    GTEST_LOG_(INFO) << "Fms_FormTimerMgr_0010 start";
    bool isAddOk5 = FormTimerMgr::GetInstance().AddFormTimer(PARAM_FORM_ID_VALUE_3,  16, 30);
    EXPECT_EQ(isAddOk5, true);
    bool isAddOk6 = FormTimerMgr::GetInstance().HandleResetLimiter();
    EXPECT_EQ(isAddOk6, true);

    GTEST_LOG_(INFO) << "Fms_FormTimerMgr_0010 end";
}

/**
 * @tc.number: Fms_FormTimerMgr_0011
 * @tc.name: OnUpdateAtTrigger.
 * @tc.desc:handle attimer update.
 */
HWTEST_F(FmsFormTimerMgrTest, Fms_FormTimerMgr_0011, Function | MediumTest | Level1)
{
    GTEST_LOG_(INFO) << "Fms_FormTimerMgr_0011 start";
    bool isAddOk5 = FormTimerMgr::GetInstance().AddFormTimer(PARAM_FORM_ID_VALUE_4,  11, 30);
    EXPECT_EQ(isAddOk5, true);

    bool isAddOk6 = FormTimerMgr::GetInstance().OnUpdateAtTrigger(90L);
    EXPECT_EQ(isAddOk6, true);

    GTEST_LOG_(INFO) << "Fms_FormTimerMgr_0011 end";
}
/**
 * @tc.number: Fms_FormTimerMgr_0012
 * @tc.name: OnDynamicTimeTrigger.
 * @tc.desc: handle dynamic update.
 */
HWTEST_F(FmsFormTimerMgrTest, Fms_FormTimerMgr_0012, Function | MediumTest | Level1)
{
    GTEST_LOG_(INFO) << "Fms_FormTimerMgr_0012 start";
    bool isAddOk5 = FormTimerMgr::GetInstance().AddFormTimer(PARAM_FORM_ID_VALUE_5,  11, 30);
    EXPECT_EQ(isAddOk5, true);

    bool isAddOk6 = FormTimerMgr::GetInstance().OnDynamicTimeTrigger(90L);
    EXPECT_EQ(isAddOk6, true);

    GTEST_LOG_(INFO) << "Fms_FormTimerMgr_0012 end";
}

/**
 * @tc.number: Fms_FormTimerMgr_0013
 * @tc.name: AddFormTimer.
 * @tc.desc: Add duration form timer.
 */
HWTEST_F(FmsFormTimerMgrTest, Fms_FormTimerMgr_0013, Function | MediumTest | Level1)
{
    GTEST_LOG_(INFO) << "Fms_FormTimerMgr_0013 start";
    bool isOk = FormTimerMgr::GetInstance().AddFormTimer(PARAM_FORM_ID_VALUE_1,  0);
    EXPECT_EQ(isOk, false);
    GTEST_LOG_(INFO) << "Fms_FormTimerMgr_0013 end";
}

/**
 * @tc.number: Fms_FormTimerMgr_0014
 * @tc.name: AddFormTimer.
 * @tc.desc: Add scheduled form timer.
 */
HWTEST_F(FmsFormTimerMgrTest, Fms_FormTimerMgr_0014, Function | MediumTest | Level1)
{
    GTEST_LOG_(INFO) << "Fms_FormTimerMgr_0014 start";
    bool isOk = FormTimerMgr::GetInstance().AddFormTimer(PARAM_FORM_ID_VALUE_2,  0, 60);
    EXPECT_EQ(isOk, false);
    GTEST_LOG_(INFO) << "Fms_FormTimerMgr_0014 end";
}

/**
 * @tc.number: Fms_FormTimerMgr_0015
 * @tc.name: UpdateFormTimer.
 * @tc.desc: Update form timer(TYPE_INTERVAL_CHANGE).
 */
HWTEST_F(FmsFormTimerMgrTest, Fms_FormTimerMgr_0015, Function | MediumTest | Level1)
{
    GTEST_LOG_(INFO) << "Fms_FormTimerMgr_0015 start";
    bool isAddOk1 = FormTimerMgr::GetInstance().AddFormTimer(PARAM_FORM_ID_VALUE_1,  3 * Constants::MIN_PERIOD);
    EXPECT_EQ(isAddOk1, true);

    // TYPE_INTERVAL_CHANGE
    FormTimerCfg timerCfg1;
    timerCfg1.enableUpdate = true;
    timerCfg1.updateDuration = 0;
    bool isUpdateOk1 = FormTimerMgr::GetInstance().UpdateFormTimer(PARAM_FORM_ID_VALUE_1,
    UpdateType::TYPE_INTERVAL_CHANGE, timerCfg1);
    EXPECT_EQ(isUpdateOk1, false);
    GTEST_LOG_(INFO) << "Fms_FormTimerMgr_0015 end";
}
/**
 * @tc.number: Fms_FormTimerMgr_0016
 * @tc.name: UpdateFormTimer.
 * @tc.desc: Update form timer(TYPE_ATTIME_CHANGE).
 */
HWTEST_F(FmsFormTimerMgrTest, Fms_FormTimerMgr_0016, Function | MediumTest | Level1)
{
    GTEST_LOG_(INFO) << "Fms_FormTimerMgr_0016 start";
    bool isAddOk2 = FormTimerMgr::GetInstance().AddFormTimer(PARAM_FORM_ID_VALUE_2,  3, 30);
    EXPECT_EQ(isAddOk2, true);

    // TYPE_ATTIME_CHANGE
    FormTimerCfg timerCfg2;
    timerCfg2.enableUpdate = true;
    timerCfg2.updateAtHour = 0;
    timerCfg2.updateAtMin = 60;
    bool isUpdateOk2 = FormTimerMgr::GetInstance().UpdateFormTimer(PARAM_FORM_ID_VALUE_2,
    UpdateType::TYPE_ATTIME_CHANGE,  timerCfg2);
    EXPECT_EQ(isUpdateOk2, false);

    GTEST_LOG_(INFO) << "Fms_FormTimerMgr_0016 end";
}
/**
 * @tc.number: Fms_FormTimerMgr_0017
 * @tc.name: UpdateFormTimer.
 * @tc.desc: Update form timer(TYPE_INTERVAL_TO_ATTIME).
 */
HWTEST_F(FmsFormTimerMgrTest, Fms_FormTimerMgr_0017, Function | MediumTest | Level1)
{
    GTEST_LOG_(INFO) << "Fms_FormTimerMgr_0017 start";
    bool isAddOk3 = FormTimerMgr::GetInstance().AddFormTimer(PARAM_FORM_ID_VALUE_3,  6 * Constants::MIN_PERIOD);
    EXPECT_EQ(isAddOk3, true);

    // TYPE_INTERVAL_TO_ATTIME
    FormTimerCfg timerCfg3;
    timerCfg3.enableUpdate = true;
    timerCfg3.updateAtHour = 0;
    timerCfg3.updateAtMin = 60;
    bool isUpdateOk3 = FormTimerMgr::GetInstance().UpdateFormTimer(PARAM_FORM_ID_VALUE_3,
    UpdateType::TYPE_INTERVAL_TO_ATTIME,  timerCfg3);
    EXPECT_EQ(isUpdateOk3, false);
    GTEST_LOG_(INFO) << "Fms_FormTimerMgr_0017 end";
}
/**
 * @tc.number: Fms_FormTimerMgr_0018
 * @tc.name: UpdateFormTimer.
 * @tc.desc: Update form timer(TYPE_ATTIME_TO_INTERVAL).
 */
HWTEST_F(FmsFormTimerMgrTest, Fms_FormTimerMgr_0018, Function | MediumTest | Level1)
{
    GTEST_LOG_(INFO) << "Fms_FormTimerMgr_0018 start";
    bool isAddOk4 = FormTimerMgr::GetInstance().AddFormTimer(PARAM_FORM_ID_VALUE_4,  10, 30);
    EXPECT_EQ(isAddOk4, true);

    // TYPE_ATTIME_TO_INTERVAL
    FormTimerCfg timerCfg4;
    timerCfg4.enableUpdate = true;
    timerCfg4.updateDuration = 0;
    bool isUpdateOk4 = FormTimerMgr::GetInstance().UpdateFormTimer(PARAM_FORM_ID_VALUE_4,
    UpdateType::TYPE_ATTIME_TO_INTERVAL,  timerCfg4);
    EXPECT_EQ(isUpdateOk4, false);
    GTEST_LOG_(INFO) << "Fms_FormTimerMgr_0018 end";
}

/**
 * @tc.number: Fms_FormTimerMgr_0019
 * @tc.name: FormRefreshLimiter::AddItem.
 * @tc.desc: AddItem success.
 */
HWTEST_F(FmsFormTimerMgrTest, Fms_FormTimerMgr_0019, Function | MediumTest | Level1)
{
    GTEST_LOG_(INFO) << "Fms_FormTimerMgr_0019 start";

    FormRefreshLimiter refreshLimiter;
    bool isAddOk = refreshLimiter.AddItem(PARAM_FORM_ID_VALUE_1);
    EXPECT_EQ(isAddOk, true);

    EXPECT_EQ(refreshLimiter.GetItemCount(), 1);

    GTEST_LOG_(INFO) << "Fms_FormTimerMgr_0019 end";
}

/**
 * @tc.number: Fms_FormTimerMgr_0020
 * @tc.name: FormRefreshLimiter::DeleteItem.
 * @tc.desc: DeleteItem success.
 */
HWTEST_F(FmsFormTimerMgrTest, Fms_FormTimerMgr_0020, Function | MediumTest | Level1)
{
    GTEST_LOG_(INFO) << "Fms_FormTimerMgr_0020 start";

    FormRefreshLimiter refreshLimiter;
    refreshLimiter.DeleteItem(PARAM_FORM_ID_VALUE_6);
    EXPECT_EQ(refreshLimiter.GetItemCount(), 0);
    GTEST_LOG_(INFO) << "Fms_FormTimerMgr_0020 end";
}

/**
 * @tc.number: Fms_FormTimerMgr_0021
 * @tc.name: FormRefreshLimiter::IsEnableRefresh.
 * @tc.desc: IsEnableRefresh.
 */
HWTEST_F(FmsFormTimerMgrTest, Fms_FormTimerMgr_0021, Function | MediumTest | Level1)
{
    GTEST_LOG_(INFO) << "Fms_FormTimerMgr_0021 start";

    FormRefreshLimiter refreshLimiter;
    bool isAddOk = refreshLimiter.AddItem(PARAM_FORM_ID_VALUE_6);
    EXPECT_EQ(isAddOk, true);
    bool isEnableRefresh = refreshLimiter.IsEnableRefresh(PARAM_FORM_ID_VALUE_6);
    EXPECT_EQ(isEnableRefresh, true);

    GTEST_LOG_(INFO) << "Fms_FormTimerMgr_0021 end";
}

/**
 * @tc.number: Fms_FormTimerMgr_0022
 * @tc.name: FormRefreshLimiter::Increase.
 * @tc.desc: Increase refreshCount.
 */
HWTEST_F(FmsFormTimerMgrTest, Fms_FormTimerMgr_0022, Function | MediumTest | Level1)
{
    GTEST_LOG_(INFO) << "Fms_FormTimerMgr_0022 start";

    FormRefreshLimiter refreshLimiter;
    bool isAddOk = refreshLimiter.AddItem(PARAM_FORM_ID_VALUE_6);
    EXPECT_EQ(isAddOk, true);

    refreshLimiter.Increase(PARAM_FORM_ID_VALUE_6);

    int count = refreshLimiter.GetRefreshCount(PARAM_FORM_ID_VALUE_6);
    EXPECT_EQ(count, 1);

    GTEST_LOG_(INFO) << "Fms_FormTimerMgr_0022 end";
}

/**
 * @tc.number: Fms_FormTimerMgr_0023
 * @tc.name: FormRefreshLimiter::ResetLimit.
 * @tc.desc: ResetLimit.
 */
HWTEST_F(FmsFormTimerMgrTest, Fms_FormTimerMgr_0023, Function | MediumTest | Level1)
{
    GTEST_LOG_(INFO) << "Fms_FormTimerMgr_0023 start";

    FormRefreshLimiter refreshLimiter;
    bool isAddOk = refreshLimiter.AddItem(PARAM_FORM_ID_VALUE_6);
    EXPECT_EQ(isAddOk, true);

    refreshLimiter.Increase(PARAM_FORM_ID_VALUE_6);

    refreshLimiter.ResetLimit();

    int count = refreshLimiter.GetRefreshCount(PARAM_FORM_ID_VALUE_6);
    EXPECT_EQ(count, 0);

    GTEST_LOG_(INFO) << "Fms_FormTimerMgr_0023 end";
}

/**
 * @tc.number: Fms_FormTimerMgr_0024
 * @tc.name: FormRefreshLimiter::Increase.
 * @tc.desc: report refresh to 50 count.
 */
HWTEST_F(FmsFormTimerMgrTest, Fms_FormTimerMgr_0024, Function | MediumTest | Level1)
{
    GTEST_LOG_(INFO) << "Fms_FormTimerMgr_0024 start";

    FormRefreshLimiter refreshLimiter;
    bool isAddOk = refreshLimiter.AddItem(PARAM_FORM_ID_VALUE_6);
    EXPECT_EQ(isAddOk, true);
    for (int iIndex = 0; iIndex < Constants::LIMIT_COUNT; iIndex++) {
        refreshLimiter.Increase(PARAM_FORM_ID_VALUE_6);
    }

    int count = refreshLimiter.GetRefreshCount(PARAM_FORM_ID_VALUE_6);
    EXPECT_EQ(count, Constants::LIMIT_COUNT);

    GTEST_LOG_(INFO) << "Fms_FormTimerMgr_0024 end";
}

/**
 * @tc.number: Fms_FormTimerMgr_0025
 * @tc.name: FormRefreshLimiter::IsEnableRefresh.
 * @tc.desc: report refresh to 50 count.
 */
HWTEST_F(FmsFormTimerMgrTest, Fms_FormTimerMgr_0025, Function | MediumTest | Level1)
{
    GTEST_LOG_(INFO) << "Fms_FormTimerMgr_0025 start";

    FormRefreshLimiter refreshLimiter;
    bool isAddOk = refreshLimiter.AddItem(PARAM_FORM_ID_VALUE_6);
    EXPECT_EQ(isAddOk, true);
    for (int iIndex = 0; iIndex < Constants::LIMIT_COUNT + 1; iIndex++) {
        refreshLimiter.Increase(PARAM_FORM_ID_VALUE_6);
    }

    bool isEnableRefresh = refreshLimiter.IsEnableRefresh(PARAM_FORM_ID_VALUE_6);
    EXPECT_EQ(isEnableRefresh, false);

    GTEST_LOG_(INFO) << "Fms_FormTimerMgr_0025 end";
}

/**
 * @tc.number: Fms_FormTimerMgr_0026
 * @tc.name: FormRefreshLimiter::MarkRemind.
 * @tc.desc: Mark remind when refresh count >= 50.
 */
HWTEST_F(FmsFormTimerMgrTest, Fms_FormTimerMgr_0026, Function | MediumTest | Level1)
{
    GTEST_LOG_(INFO) << "Fms_FormTimerMgr_0026 start";

    FormRefreshLimiter refreshLimiter;
    bool isAddOk = refreshLimiter.AddItem(PARAM_FORM_ID_VALUE_6);
    EXPECT_EQ(isAddOk, true);
    for (int iIndex = 0; iIndex < Constants::LIMIT_COUNT + 1; iIndex++) {
        refreshLimiter.Increase(PARAM_FORM_ID_VALUE_6);
    }

    refreshLimiter.MarkRemind(PARAM_FORM_ID_VALUE_6);

    std::vector<int64_t> vIdlist = refreshLimiter.GetRemindList();
    EXPECT_EQ(vIdlist.size() > 0, true);

    GTEST_LOG_(INFO) << "Fms_FormTimerMgr_0026 end";
}

/**
 * @tc.number: Fms_FormTimerMgr_0027
 * @tc.name: OnIntervalTimeOut.
 * @tc.desc: Interval timer timeout.
 */
HWTEST_F(FmsFormTimerMgrTest, Fms_FormTimerMgr_0027, Function | MediumTest | Level1)
{
    GTEST_LOG_(INFO) << "Fms_FormTimerMgr_0027 start";
    bool isAddOk4 = FormTimerMgr::GetInstance().AddFormTimer(PARAM_FORM_ID_VALUE_6,  10, 30);
    EXPECT_EQ(isAddOk4, true);

    std::this_thread::sleep_for(std::chrono::milliseconds(Constants::MIN_PERIOD));

    GTEST_LOG_(INFO) << "Fms_FormTimerMgr_0027 end";
}
}
