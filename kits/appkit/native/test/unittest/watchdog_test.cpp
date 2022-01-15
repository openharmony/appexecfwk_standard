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

#include <gtest/gtest.h>
#include <thread>

#include "main_thread.h"
#include "watchdog.h"

using namespace testing::ext;
using namespace OHOS;
using namespace OHOS::AppExecFwk;

namespace OHOS {
namespace AppExecFwk {
const int USleepTime = 16000;
class WatchDogTest : public testing::Test {
public:
    WatchDogTest()
    {}
    ~WatchDogTest()
    {}
    static void SetUpTestCase(void);
    static void TearDownTestCase(void);
    void SetUp();
    void TearDown();
};

void WatchDogTest::SetUpTestCase(void)
{}

void WatchDogTest::TearDownTestCase(void)
{}

void WatchDogTest::SetUp(void)
{}

void WatchDogTest::TearDown(void)
{}

/**
 * @tc.number: AppExecFwk_WatchDog_GetAppMainThreadState_0100
 * @tc.name: GetAppMainThreadState
 * @tc.desc: Test the abnormal state of GetAppMainThreadState.
 */
HWTEST_F(WatchDogTest, AppExecFwk_WatchDog_GetAppMainThreadState_0100, Function | MediumTest | Level3)
{
    bool ret = WatchDog::GetAppMainThreadState();
    EXPECT_FALSE(ret);
}

/**
 * @tc.number: AppExecFwk_WatchDog_GetAppMainThreadState_0200
 * @tc.name: GetAppMainThreadState
 * @tc.desc: Test whether GetAppMainThreadState are called normally.
 */
HWTEST_F(WatchDogTest, AppExecFwk_WatchDog_GetAppMainThreadState_0200, Function | MediumTest | Level3)
{
    std::thread mainthread([&](){MainThread::Start();});
    mainthread.detach();
    std::this_thread::sleep_for(std::chrono::milliseconds(USleepTime));
    bool ret = WatchDog::GetAppMainThreadState();
    EXPECT_TRUE(ret);
}
}  // namespace AppExecFwk
}  // namespace OHOS