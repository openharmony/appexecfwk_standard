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
#include <string>
#include <memory>
#include <set>
#include "ability_start_setting.h"
#include "parcel.h"

namespace OHOS {
namespace AppExecFwk {
using namespace testing::ext;
using namespace OHOS;
using namespace OHOS::AppExecFwk;
using namespace OHOS::AAFwk;
class AbilityStartSettingTest : public testing::Test {
public:
    AbilityStartSettingTest() : abilityStartSetting_(nullptr)
    {}
    ~AbilityStartSettingTest()
    {}
    std::shared_ptr<AbilityStartSetting> abilityStartSetting_ = nullptr;

    static void SetUpTestCase(void);
    static void TearDownTestCase(void);
    void SetUp();
    void TearDown();
};

void AbilityStartSettingTest::SetUpTestCase(void)
{}

void AbilityStartSettingTest::TearDownTestCase(void)
{}

void AbilityStartSettingTest::SetUp(void)
{
    abilityStartSetting_ = AbilityStartSetting::GetEmptySetting();
}

void AbilityStartSettingTest::TearDown(void)
{}

/**
 * @tc.number: AppExecFwk_ContextDeal_StartAbility_0100
 * @tc.name: IsEmpty
 * @tc.desc: Test whether startability is called normally.
 */
HWTEST_F(AbilityStartSettingTest, AppExecFwk_IsEmpty_0100, Function | MediumTest | Level1)
{
    if (abilityStartSetting_ == nullptr) {
        return;
    }
    EXPECT_EQ(true, abilityStartSetting_->IsEmpty());
    abilityStartSetting_->AddProperty(AbilityStartSetting::BOUNDS_KEY, std::string("system_bounds_test"));
    abilityStartSetting_->AddProperty(
        AbilityStartSetting::WINDOW_DISPLAY_ID_KEY, std::string("system_window_display_key_test"));
    abilityStartSetting_->AddProperty(AbilityStartSetting::WINDOW_MODE_KEY, std::string("system_mode_key_test"));
    EXPECT_EQ(false, abilityStartSetting_->IsEmpty());
    std::set<std::string> set = abilityStartSetting_->GetPropertiesKey();
    EXPECT_EQ((unsigned int)3, set.size());
    std::set<std::string>::iterator it = set.begin();
    if (it != set.end()) {
        EXPECT_EQ(*it, AbilityStartSetting::BOUNDS_KEY.c_str());
        GTEST_LOG_(INFO) << "AppExecFwk_AbilityStartSetting_0100 test 1";
    }
    it++;
    if (it != set.end()) {
        EXPECT_EQ(*it, AbilityStartSetting::WINDOW_DISPLAY_ID_KEY.c_str());
        GTEST_LOG_(INFO) << "AppExecFwk_AbilityStartSetting_0100 test 2";
    }
    it++;
    if (it != set.end()) {
        EXPECT_EQ(*it, AbilityStartSetting::WINDOW_MODE_KEY.c_str());
        GTEST_LOG_(INFO) << "AppExecFwk_AbilityStartSetting_0100 test 3";
    }
    Parcel parcel;
    EXPECT_EQ(true, abilityStartSetting_->Marshalling(parcel));

    AbilityStartSetting *setting = AbilityStartSetting::Unmarshalling(parcel);
    std::shared_ptr<AbilityStartSetting> startSetting(setting);

    if (startSetting == nullptr) {
        return;
    }
    std::set<std::string> unmarshallingSet = startSetting->GetPropertiesKey();
    EXPECT_EQ((unsigned int)3, unmarshallingSet.size());
    it = unmarshallingSet.begin();
    if (it != unmarshallingSet.end()) {
        EXPECT_EQ(*it, AbilityStartSetting::BOUNDS_KEY.c_str());
        GTEST_LOG_(INFO) << "AppExecFwk_AbilityStartSetting_0100 test 4";
    }
    it++;
    if (it != unmarshallingSet.end()) {
        EXPECT_EQ(*it, AbilityStartSetting::WINDOW_DISPLAY_ID_KEY.c_str());
        GTEST_LOG_(INFO) << "AppExecFwk_AbilityStartSetting_0100 test 5";
    }
    it++;
    if (it != unmarshallingSet.end()) {
        EXPECT_EQ(*it, AbilityStartSetting::WINDOW_MODE_KEY.c_str());
        GTEST_LOG_(INFO) << "AppExecFwk_AbilityStartSetting_0100 test 6";
    }
}
}  // namespace AppExecFwk
}  // namespace OHOS