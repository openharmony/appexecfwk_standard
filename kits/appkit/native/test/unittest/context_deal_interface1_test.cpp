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

#include <singleton.h>
#include <gtest/gtest.h>
#include "gmock/gmock.h"

#include "context_deal.h"
#include "ohos_application.h"
#include "ability_handler.h"
#include "ability_info.h"
#include "ability.h"

#include "mock_ability_manager_client_interface1.h"
#include "mock_resourceManager_interface1.h"

namespace OHOS {
namespace AppExecFwk {
using namespace testing::ext;

class ContextDealInterfaceTest : public testing::Test {
public:
    ContextDealInterfaceTest()
    {}
    ~ContextDealInterfaceTest()
    {}
    static void SetUpTestCase(void);
    static void TearDownTestCase(void);
    void SetUp();
    void TearDown();
};

void ContextDealInterfaceTest::SetUpTestCase(void)
{}
void ContextDealInterfaceTest::TearDownTestCase(void)
{}
void ContextDealInterfaceTest::SetUp()
{}
void ContextDealInterfaceTest::TearDown()
{}

/**
 * @tc.number: AppExecFwk_ContextDeal_SetColorMode_0100
 * @tc.name: SetColorMode
 * @tc.desc: Test the attachbasecontext call to verify that the return value of SetColorMode is correct.
 */
HWTEST_F(ContextDealInterfaceTest, AppExecFwk_ContextDeal_SetColorMode_0100, Function | MediumTest | Level1)
{
    GTEST_LOG_(INFO) << "AppExecFwk_ContextDeal_SetColorMode_0100 start";
    std::shared_ptr<AbilityInfo> abilityInfo = std::make_shared<AbilityInfo>();
    std::shared_ptr<ContextDeal> contextDeal = std::make_shared<MockAbilityContextDeal>();
    int testValue = static_cast<int>(OHOS::AppExecFwk::ModuleColorMode::DARK);
    contextDeal->SetAbilityInfo(abilityInfo);
    contextDeal->SetColorMode(testValue);
    EXPECT_EQ(testValue, contextDeal->GetColorMode());
    GTEST_LOG_(INFO) << "AppExecFwk_ContextDeal_SetColorMode_0100 end";
}

/**
 * @tc.number: AppExecFwk_ContextDeal_SetColorMode_0200
 * @tc.name: SetColorMode
 * @tc.desc: Test the attachbasecontext call to verify that the return value of SetColorMode is correct.
 */
HWTEST_F(ContextDealInterfaceTest, AppExecFwk_ContextDeal_SetColorMode_0200, Function | MediumTest | Level3)
{
    GTEST_LOG_(INFO) << "AppExecFwk_ContextDeal_SetColorMode_0200 start";
    std::shared_ptr<ContextDeal> contextDeal = std::make_shared<ContextDeal>();
    int testValue = static_cast<int>(OHOS::AppExecFwk::ModuleColorMode::DARK);
    contextDeal->SetColorMode(testValue);
    EXPECT_GT(testValue, contextDeal->GetColorMode());
    GTEST_LOG_(INFO) << "AppExecFwk_ContextDeal_SetColorMode_0200 end";
}

/**
 * @tc.number: AppExecFwk_ContextDeal_GetColorMode_0100
 * @tc.name: GetColorMode
 * @tc.desc: Test the attachbasecontext call to verify that the return value of GetColorMode is correct.
 */
HWTEST_F(ContextDealInterfaceTest, AppExecFwk_ContextDeal_GetColorMode_0100, Function | MediumTest | Level1)
{
    GTEST_LOG_(INFO) << "AppExecFwk_ContextDeal_GetColorMode_0100 start";
    std::shared_ptr<AbilityInfo> abilityInfo = std::make_shared<AbilityInfo>();
    std::shared_ptr<ContextDeal> contextDeal = std::make_shared<MockAbilityContextDeal>();
    int testValue = static_cast<int>(OHOS::AppExecFwk::ModuleColorMode::DARK);
    contextDeal->SetAbilityInfo(abilityInfo);
    contextDeal->SetColorMode(testValue);
    EXPECT_EQ(testValue, contextDeal->GetColorMode());
    GTEST_LOG_(INFO) << "AppExecFwk_ContextDeal_GetColorMode_0100 end";
}

/**
 * @tc.number: AppExecFwk_ContextDeal_GetColorMode_0200
 * @tc.name: GetColorMode
 * @tc.desc: Test the attachbasecontext call to verify that the return value of GetColorMode is correct.
 */
HWTEST_F(ContextDealInterfaceTest, AppExecFwk_ContextDeal_GetColorMode_0200, Function | MediumTest | Level3)
{
    GTEST_LOG_(INFO) << "AppExecFwk_ContextDeal_GetColorMode_0200 start";
    std::shared_ptr<ContextDeal> contextDeal = std::make_shared<ContextDeal>();
    int testValue = static_cast<int>(OHOS::AppExecFwk::ModuleColorMode::DARK);
    contextDeal->SetColorMode(testValue);
    EXPECT_GT(testValue, contextDeal->GetColorMode());
    GTEST_LOG_(INFO) << "AppExecFwk_ContextDeal_GetColorMode_0200 end";
}

/**
 * @tc.number: AppExecFwk_ContextDeal_SetTheme_0100
 * @tc.name: SetTheme
 * @tc.desc: Test the attachbasecontext call to verify that the return value of SetTheme is correct.
 */
HWTEST_F(ContextDealInterfaceTest, AppExecFwk_ContextDeal_SetTheme_0100, Function | MediumTest | Level1)
{
    GTEST_LOG_(INFO) << "AppExecFwk_ContextDeal_SetTheme_0100 start";
    int testValue = 1;
    std::map<std::string, std::string> testList = {
        {"Pattern1", "PatternA"}, {"Pattern2", "PatternB"}, {"Pattern3", "PatternC"}};
    std::shared_ptr<Global::Resource::ResourceManager2> resourceManager(Global::Resource::CreateResourceManager2());
    std::shared_ptr<ContextDeal> contextDeal = std::make_shared<ContextDeal>();
    resourceManager->SetThemeById(testValue, testList);
    contextDeal->initResourceManager(resourceManager);
    contextDeal->SetTheme(testValue);
    GTEST_LOG_(INFO) << "AppExecFwk_ContextDeal_SetTheme_0100 end";
}

/**
 * @tc.number: AppExecFwk_ContextDeal_SetTheme_0200
 * @tc.name: SetTheme
 * @tc.desc: Test the attachbasecontext call to verify that the return value of SetTheme is correct.
 */
HWTEST_F(ContextDealInterfaceTest, AppExecFwk_ContextDeal_SetTheme_0200, Function | MediumTest | Level3)
{
    GTEST_LOG_(INFO) << "AppExecFwk_ContextDeal_SetTheme_0200 start";
    int testValue = 1;
    std::shared_ptr<ContextDeal> contextDeal = std::make_shared<ContextDeal>();
    contextDeal->SetTheme(testValue);
    GTEST_LOG_(INFO) << "AppExecFwk_ContextDeal_SetTheme_0200 end";
}

/**
 * @tc.number: AppExecFwk_ContextDeal_GetPreferencesDir_0100
 * @tc.name: GetPreferencesDir
 * @tc.desc: Test the attachbasecontext call to verify that the return value of GetPreferencesDir is correct.
 */
HWTEST_F(ContextDealInterfaceTest, AppExecFwk_ContextDeal_GetPreferencesDir_0100, Function | MediumTest | Level1)
{
    GTEST_LOG_(INFO) << "AppExecFwk_ContextDeal_GetPreferencesDir_0100 start";
    std::shared_ptr<AbilityInfo> abilityInfo = std::make_shared<AbilityInfo>();
    std::shared_ptr<ApplicationInfo> applicationInfo = std::make_shared<ApplicationInfo>();
    std::shared_ptr<ContextDeal> contextDeal = std::make_shared<ContextDeal>();
    const std::string testValue = "./1234test/preferences";
    abilityInfo->name = "test1234.1234test";
    applicationInfo->dataDir = ".";
    contextDeal->SetAbilityInfo(abilityInfo);
    contextDeal->SetApplicationInfo(applicationInfo);
    EXPECT_STREQ(testValue.c_str(), contextDeal->GetPreferencesDir().c_str());
    GTEST_LOG_(INFO) << "AppExecFwk_ContextDeal_GetPreferencesDir_0100 end";
}

/**
 * @tc.number: AppExecFwk_ContextDeal_GetPreferencesDir_0101
 * @tc.name: GetPreferencesDir
 * @tc.desc: Test the attachbasecontext call to verify that the return value of GetPreferencesDir is correct.
 */
HWTEST_F(ContextDealInterfaceTest, AppExecFwk_ContextDeal_GetPreferencesDir_0101, Function | MediumTest | Level1)
{
    GTEST_LOG_(INFO) << "AppExecFwk_ContextDeal_GetPreferencesDir_0101 start";
    std::shared_ptr<AbilityInfo> abilityInfo = std::make_shared<AbilityInfo>();
    std::shared_ptr<ApplicationInfo> applicationInfo = std::make_shared<ApplicationInfo>();
    std::shared_ptr<ContextDeal> contextDeal = std::make_shared<ContextDeal>();
    const std::string testValue = "./1234test/preferences";
    abilityInfo->name = "1234test";
    abilityInfo->isNativeAbility = true;
    applicationInfo->dataDir = ".";
    contextDeal->SetAbilityInfo(abilityInfo);
    contextDeal->SetApplicationInfo(applicationInfo);
    EXPECT_STREQ(testValue.c_str(), contextDeal->GetPreferencesDir().c_str());
    GTEST_LOG_(INFO) << "AppExecFwk_ContextDeal_GetPreferencesDir_0101 end";
}

/**
 * @tc.number: AppExecFwk_ContextDeal_GetPreferencesDir_0200
 * @tc.name: GetPreferencesDir
 * @tc.desc: Test the attachbasecontext call to verify that the return value of GetPreferencesDir is correct.
 */
HWTEST_F(ContextDealInterfaceTest, AppExecFwk_ContextDeal_GetPreferencesDir_0200, Function | MediumTest | Level3)
{
    GTEST_LOG_(INFO) << "AppExecFwk_ContextDeal_GetPreferencesDir_0200 start";
    std::shared_ptr<AbilityInfo> abilityInfo = std::make_shared<AbilityInfo>();
    std::shared_ptr<ApplicationInfo> applicationInfo = std::make_shared<ApplicationInfo>();
    std::shared_ptr<ContextDeal> contextDeal = std::make_shared<ContextDeal>();
    const std::string testValue = "";
    abilityInfo->name = "1234test";
    abilityInfo->isNativeAbility = false;
    applicationInfo->dataDir = ".";
    contextDeal->SetAbilityInfo(abilityInfo);
    contextDeal->SetApplicationInfo(applicationInfo);
    EXPECT_STREQ(testValue.c_str(), contextDeal->GetPreferencesDir().c_str());
    GTEST_LOG_(INFO) << "AppExecFwk_ContextDeal_GetPreferencesDir_0200 end";
}

/**
 * @tc.number: AppExecFwk_ContextDeal_GetPattern_0100
 * @tc.name: GetPattern
 * @tc.desc: Test the attachbasecontext call to verify that the return value of GetPattern is correct.
 */
HWTEST_F(ContextDealInterfaceTest, AppExecFwk_ContextDeal_GetPattern_0100, Function | MediumTest | Level1)
{
    GTEST_LOG_(INFO) << "AppExecFwk_ContextDeal_GetPattern_0100 start";
    int testValue = 0;
    std::map<std::string, std::string> testList = {
        {"Pattern1", "PatternA"}, {"Pattern2", "PatternB"}, {"Pattern3", "PatternC"}};
    std::shared_ptr<Global::Resource::ResourceManager2> resourceManager(Global::Resource::CreateResourceManager2());
    std::shared_ptr<ContextDeal> contextDeal = std::make_shared<ContextDeal>();
    resourceManager->SetPatternById(testValue, testList);
    contextDeal->initResourceManager(resourceManager);
    contextDeal->SetPattern(testValue);
    std::map<std::string, std::string> retVal = contextDeal->GetPattern();
    EXPECT_TRUE((retVal == testList));
    GTEST_LOG_(INFO) << "AppExecFwk_ContextDeal_GetPattern_0100 end";
}

/**
 * @tc.number: AppExecFwk_ContextDeal_GetTheme_0100
 * @tc.name: GetTheme
 * @tc.desc: Test the attachbasecontext call to verify that the return value of GetTheme is correct.
 */
HWTEST_F(ContextDealInterfaceTest, AppExecFwk_ContextDeal_GetTheme_0100, Function | MediumTest | Level1)
{
    GTEST_LOG_(INFO) << "AppExecFwk_ContextDeal_GetTheme_0100 start";
    int testValue = 0;
    std::map<std::string, std::string> testList = {{"Theme1", "ThemeA"}, {"Theme2", "ThemeB"}, {"Theme3", "ThemeC"}};
    std::shared_ptr<Global::Resource::ResourceManager2> resourceManager(Global::Resource::CreateResourceManager2());
    std::shared_ptr<ContextDeal> contextDeal = std::make_shared<ContextDeal>();
    resourceManager->SetThemeById(testValue, testList);
    contextDeal->initResourceManager(resourceManager);
    std::map<std::string, std::string> retVal = contextDeal->GetTheme();

    GTEST_LOG_(INFO) << "AppExecFwk_ContextDeal_GetTheme_0100 end";
}

/**
 * @tc.number: AppExecFwk_ContextDeal_GetString_ById_0100
 * @tc.name: GetString
 * @tc.desc: Test the attachbasecontext call to verify that the return value of GetString is correct.
 */
HWTEST_F(ContextDealInterfaceTest, AppExecFwk_ContextDeal_GetString_ById_0100, Function | MediumTest | Level1)
{
    GTEST_LOG_(INFO) << "AppExecFwk_ContextDeal_GetString_ById_0100 start";
    std::shared_ptr<Global::Resource::ResourceManager2> resourceManager(Global::Resource::CreateResourceManager2());
    std::shared_ptr<ContextDeal> contextDeal = std::make_shared<ContextDeal>();
    int testCount = 0;
    std::string testValue = "GetString test";
    resourceManager->SetStringById(testCount, testValue);
    contextDeal->initResourceManager(resourceManager);
    EXPECT_STREQ(testValue.c_str(), contextDeal->GetString(testCount).c_str());
    GTEST_LOG_(INFO) << "AppExecFwk_ContextDeal_GetString_ById_0100 end";
}

/**
 * @tc.number: AppExecFwk_ContextDeal_GetString_ByIdAndFormat_0100
 * @tc.name: GetString
 * @tc.desc: Test the attachbasecontext call to verify that the return value of GetString is correct.
 */
HWTEST_F(ContextDealInterfaceTest, AppExecFwk_ContextDeal_GetString_ByIdAndFormat_0100, Function | MediumTest | Level1)
{
    GTEST_LOG_(INFO) << "AppExecFwk_ContextDeal_GetString_ByIdAndFormat_0100 start";
    GTEST_LOG_(INFO) << "AppExecFwk_ContextDeal_GetString_ByIdAndFormat_0100 end";
}

/**
 * @tc.number: AppExecFwk_ContextDeal_GetStringArray_0100
 * @tc.name: GetStringArray
 * @tc.desc: Test the attachbasecontext call to verify that the return value of GetStringArray is correct.
 */
HWTEST_F(ContextDealInterfaceTest, AppExecFwk_ContextDeal_GetStringArray_0100, Function | MediumTest | Level1)
{
    GTEST_LOG_(INFO) << "AppExecFwk_ContextDeal_GetStringArray_0100 start";
    std::shared_ptr<Global::Resource::ResourceManager2> resourceManager(Global::Resource::CreateResourceManager2());
    std::shared_ptr<ContextDeal> contextDeal = std::make_shared<ContextDeal>();
    int testCount = 0;
    std::vector<std::string> testValue = {"123", "456", "789"};
    resourceManager->SetStringArrayById(testCount, testValue);
    contextDeal->initResourceManager(resourceManager);
    std::vector<std::string> retVal = contextDeal->GetStringArray(testCount);
    EXPECT_TRUE((retVal == testValue));
    GTEST_LOG_(INFO) << "AppExecFwk_ContextDeal_GetStringArray_0100 end";
}

/**
 * @tc.number: AppExecFwk_ContextDeal_GetStringArray_0200
 * @tc.name: GetStringArray
 * @tc.desc: Test the attachbasecontext call to verify that the return value of GetStringArray is correct.
 */
HWTEST_F(ContextDealInterfaceTest, AppExecFwk_ContextDeal_GetStringArray_0200, Function | MediumTest | Level3)
{
    GTEST_LOG_(INFO) << "AppExecFwk_ContextDeal_GetStringArray_0200 start";
    std::shared_ptr<Global::Resource::ResourceManager2> resourceManager(Global::Resource::CreateResourceManager2());
    std::shared_ptr<ContextDeal> contextDeal = std::make_shared<ContextDeal>();
    int testCount = 0;
    std::vector<std::string> testValue = {"123", "456", "789"};
    resourceManager->SetStringArrayById(testCount + 1, testValue);
    contextDeal->initResourceManager(resourceManager);
    std::vector<std::string> retVal = contextDeal->GetStringArray(testCount);
    EXPECT_FALSE((retVal == testValue));
    GTEST_LOG_(INFO) << "AppExecFwk_ContextDeal_GetStringArray_0200 end";
}

/**
 * @tc.number: AppExecFwk_ContextDeal_GetIntArray_0100
 * @tc.name: GetIntArray
 * @tc.desc: Test the attachbasecontext call to verify that the return value of GetIntArray is correct.
 */
HWTEST_F(ContextDealInterfaceTest, AppExecFwk_ContextDeal_GetIntArray_0100, Function | MediumTest | Level1)
{
    GTEST_LOG_(INFO) << "AppExecFwk_ContextDeal_GetIntArray_0100 start";
    std::shared_ptr<Global::Resource::ResourceManager2> resourceManager(Global::Resource::CreateResourceManager2());
    std::shared_ptr<ContextDeal> contextDeal = std::make_shared<ContextDeal>();
    int testCount = 0;
    std::vector<int> testValue = {1, 2, 3, 4, 5, 6, 7, 8, 9, 0};
    resourceManager->SetIntArrayById(testCount, testValue);
    contextDeal->initResourceManager(resourceManager);
    std::vector<int> retVal = contextDeal->GetIntArray(testCount);
    EXPECT_TRUE((retVal == testValue));
    GTEST_LOG_(INFO) << "AppExecFwk_ContextDeal_GetIntArray_0100 end";
}

/**
 * @tc.number: AppExecFwk_ContextDeal_GetIntArray_0200
 * @tc.name: GetIntArray
 * @tc.desc: Test the attachbasecontext call to verify that the return value of GetIntArray is correct.
 */
HWTEST_F(ContextDealInterfaceTest, AppExecFwk_ContextDeal_GetIntArray_0200, Function | MediumTest | Level3)
{
    GTEST_LOG_(INFO) << "AppExecFwk_ContextDeal_GetIntArray_0200 start";
    std::shared_ptr<Global::Resource::ResourceManager2> resourceManager(Global::Resource::CreateResourceManager2());
    std::shared_ptr<ContextDeal> contextDeal = std::make_shared<ContextDeal>();
    int testCount = 0;
    std::vector<int> testValue = {1, 2, 3, 4, 5, 6, 7, 8, 9, 0};
    resourceManager->SetIntArrayById(testCount + 1, testValue);
    contextDeal->initResourceManager(resourceManager);
    std::vector<int> retVal = contextDeal->GetIntArray(testCount);
    EXPECT_FALSE((retVal == testValue));
    GTEST_LOG_(INFO) << "AppExecFwk_ContextDeal_GetIntArray_0200 end";
}

/**
 * @tc.number: AppExecFwk_ContextDeal_GetColor_0100
 * @tc.name: GetColor
 * @tc.desc: Test the attachbasecontext call to verify that the return value of GetColor is correct.
 */
HWTEST_F(ContextDealInterfaceTest, AppExecFwk_ContextDeal_GetColor_0100, Function | MediumTest | Level1)
{
    GTEST_LOG_(INFO) << "AppExecFwk_ContextDeal_GetColor_0100 start";
    std::shared_ptr<Global::Resource::ResourceManager2> resourceManager(Global::Resource::CreateResourceManager2());
    std::shared_ptr<ContextDeal> contextDeal = std::make_shared<ContextDeal>();
    int testCount = 0;
    uint32_t testValue = 1;
    const int testVal = static_cast<int>(testValue);
    resourceManager->SetColorById(testCount, testValue);
    contextDeal->initResourceManager(resourceManager);
    EXPECT_EQ(testVal, contextDeal->GetColor(testCount));
    GTEST_LOG_(INFO) << "AppExecFwk_ContextDeal_GetColor_0100 end";
}

/**
 * @tc.number: AppExecFwk_ContextDeal_GetColor_0200
 * @tc.name: GetColor
 * @tc.desc: Test the attachbasecontext call to verify that the return value of GetColor is correct.
 */
HWTEST_F(ContextDealInterfaceTest, AppExecFwk_ContextDeal_GetColor_0200, Function | MediumTest | Level3)
{
    GTEST_LOG_(INFO) << "AppExecFwk_ContextDeal_GetColor_0200 start";
    std::shared_ptr<Global::Resource::ResourceManager2> resourceManager(Global::Resource::CreateResourceManager2());
    std::shared_ptr<ContextDeal> contextDeal = std::make_shared<ContextDeal>();
    int testCount = 0;
    uint32_t testValue = 1;
    const int testVal = static_cast<int>(testValue);
    resourceManager->SetColorById(testCount + 1, testValue);
    contextDeal->initResourceManager(resourceManager);
    EXPECT_NE(testVal, contextDeal->GetColor(testCount));
    GTEST_LOG_(INFO) << "AppExecFwk_ContextDeal_GetColor_0200 end";
}

/**
 * @tc.number: AppExecFwk_ContextDeal_GetThemeId_0100
 * @tc.name: GetThemeId
 * @tc.desc: Test the attachbasecontext call to verify that the return value of GetThemeId is correct.
 */
HWTEST_F(ContextDealInterfaceTest, AppExecFwk_ContextDeal_GetThemeId_0100, Function | MediumTest | Level1)
{
    GTEST_LOG_(INFO) << "AppExecFwk_ContextDeal_GetThemeId_0100 start";
    GTEST_LOG_(INFO) << "AppExecFwk_ContextDeal_GetThemeId_0100 end";
}

/**
 * @tc.number: AppExecFwk_ContextDeal_GetDisplayOrientation_0100
 * @tc.name: GetDisplayOrientation
 * @tc.desc: Test the attachbasecontext call to verify that the return value of GetDisplayOrientation is correct.
 */
HWTEST_F(ContextDealInterfaceTest, AppExecFwk_ContextDeal_GetDisplayOrientation_0100, Function | MediumTest | Level1)
{
    GTEST_LOG_(INFO) << "AppExecFwk_ContextDeal_GetDisplayOrientation_0100 start";
    std::shared_ptr<ContextDeal> contextDeal = std::make_shared<ContextDeal>();
    std::shared_ptr<AbilityInfo> abilityInfo = std::make_shared<AbilityInfo>();
    int testValue = 1;
    abilityInfo->orientation = static_cast<DisplayOrientation>(testValue);
    contextDeal->SetAbilityInfo(abilityInfo);
    EXPECT_EQ(testValue, contextDeal->GetDisplayOrientation());
    GTEST_LOG_(INFO) << "AppExecFwk_ContextDeal_GetDisplayOrientation_0100 end";
}

/**
 * @tc.number: AppExecFwk_ContextDeal_SetPattern_0100
 * @tc.name: SetPattern
 * @tc.desc: Test the attachbasecontext call to verify that the return value of SetPattern is correct.
 */
HWTEST_F(ContextDealInterfaceTest, AppExecFwk_ContextDeal_SetPattern_0100, Function | MediumTest | Level1)
{
    GTEST_LOG_(INFO) << "AppExecFwk_ContextDeal_SetPattern_0100 start";
    std::shared_ptr<Global::Resource::ResourceManager2> resourceManager(Global::Resource::CreateResourceManager2());
    std::shared_ptr<ContextDeal> contextDeal = std::make_shared<ContextDeal>();
    int testCount = 0;
    std::map<std::string, std::string> testList = {{"123", "abc"}, {"456", "def"}};
    resourceManager->SetPatternById(testCount, testList);
    contextDeal->initResourceManager(resourceManager);
    contextDeal->SetPattern(testCount);
    std::map<std::string, std::string> retVal;
    EXPECT_FALSE((retVal == testList));
    retVal = contextDeal->GetPattern();
    EXPECT_TRUE((retVal == testList));
    GTEST_LOG_(INFO) << "AppExecFwk_ContextDeal_SetPattern_0100 end";
}

/**
 * @tc.number: AppExecFwk_ContextDeal_TerminateAndRemoveMission_0100
 * @tc.name: TerminateAndRemoveMission
 * @tc.desc: Test the attachbasecontext call to verify that the return value of TerminateAbility is correct.
 */
HWTEST_F(
    ContextDealInterfaceTest, AppExecFwk_ContextDeal_TerminateAndRemoveMission_0100, Function | MediumTest | Level1)
{
    GTEST_LOG_(INFO) << "AppExecFwk_ContextDeal_TerminateAndRemoveMission_0100 start";
    const ErrCode mockValue = ERR_OK;
    OHOS::AAFwk::MockAbilityManagerClient::GetInstance()->SetRemoveMissions(mockValue);
    std::shared_ptr<ContextDeal> contextDeal = std::make_shared<ContextDeal>();
    std::shared_ptr<AAFwk::LifeCycleStateInfo> lifeCycleStateInfo = std::make_shared<AAFwk::LifeCycleStateInfo>();
    int testValue = OHOS::AAFwk::MockAbilityManagerClient::GetInstance()->GetRemoveMissionsValue();
    lifeCycleStateInfo->missionId = 0;
    contextDeal->SetLifeCycleStateInfo(*lifeCycleStateInfo);
    contextDeal->TerminateAndRemoveMission();
    EXPECT_NE(testValue, AAFwk::MockAbilityManagerClient::GetInstance()->GetRemoveMissionsValue());
    GTEST_LOG_(INFO) << "AppExecFwk_ContextDeal_TerminateAndRemoveMission_0100 end";
}

/**
 * @tc.number: AppExecFwk_ContextDeal_TerminateAndRemoveMission_0200
 * @tc.name: TerminateAndRemoveMission
 * @tc.desc: Test the attachbasecontext call to verify that the return value of TerminateAbility is correct.
 */
HWTEST_F(
    ContextDealInterfaceTest, AppExecFwk_ContextDeal_TerminateAndRemoveMission_0200, Function | MediumTest | Level3)
{
    GTEST_LOG_(INFO) << "AppExecFwk_ContextDeal_TerminateAndRemoveMission_0200 start";
    const ErrCode mockValue = ERR_OK;
    OHOS::AAFwk::MockAbilityManagerClient::GetInstance()->SetRemoveMissions(mockValue);
    OHOS::AAFwk::MockAbilityManagerClient::SetInstanceNull(false);
    std::shared_ptr<ContextDeal> contextDeal = std::make_shared<ContextDeal>();
    std::shared_ptr<AAFwk::LifeCycleStateInfo> lifeCycleStateInfo = std::make_shared<AAFwk::LifeCycleStateInfo>();
    int testValue = OHOS::AAFwk::MockAbilityManagerClient::GetInstance()->GetRemoveMissionsValue();
    lifeCycleStateInfo->missionId = 0;
    contextDeal->SetLifeCycleStateInfo(*lifeCycleStateInfo);
    contextDeal->TerminateAndRemoveMission();
    EXPECT_EQ(testValue, AAFwk::MockAbilityManagerClient::GetInstance()->GetRemoveMissionsValue());
    OHOS::AAFwk::MockAbilityManagerClient::SetInstanceNull(true);
    GTEST_LOG_(INFO) << "AppExecFwk_ContextDeal_TerminateAndRemoveMission_0200 end";
}

/**
 * @tc.number: AppExecFwk_ContextDeal_TerminateAndRemoveMission_0300
 * @tc.name: TerminateAndRemoveMission
 * @tc.desc: Test the attachbasecontext call to verify that the return value of TerminateAbility is correct.
 */
HWTEST_F(
    ContextDealInterfaceTest, AppExecFwk_ContextDeal_TerminateAndRemoveMission_0300, Function | MediumTest | Level3)
{
    GTEST_LOG_(INFO) << "AppExecFwk_ContextDeal_TerminateAndRemoveMission_0300 start";
    const ErrCode mockValue = ERR_INVALID_VALUE;
    OHOS::AAFwk::MockAbilityManagerClient::GetInstance()->SetRemoveMissions(mockValue);
    std::shared_ptr<ContextDeal> contextDeal = std::make_shared<ContextDeal>();
    std::shared_ptr<AAFwk::LifeCycleStateInfo> lifeCycleStateInfo = std::make_shared<AAFwk::LifeCycleStateInfo>();
    int testValue = OHOS::AAFwk::MockAbilityManagerClient::GetInstance()->GetRemoveMissionsValue();
    lifeCycleStateInfo->missionId = 0;
    contextDeal->SetLifeCycleStateInfo(*lifeCycleStateInfo);
    contextDeal->TerminateAndRemoveMission();
    EXPECT_EQ(testValue, AAFwk::MockAbilityManagerClient::GetInstance()->GetRemoveMissionsValue());
    GTEST_LOG_(INFO) << "AppExecFwk_ContextDeal_TerminateAndRemoveMission_0300 end";
}

}  // namespace AppExecFwk
}  // namespace OHOS
