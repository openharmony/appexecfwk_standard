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

#include <fcntl.h>
#include <fstream>
#include <future>
#include <gtest/gtest.h>

#include "app_log_wrapper.h"
#include "../include/fuzz_test_manager.h"
#include "../include/get_param.h"
#include "nlohmann/json.hpp"

using namespace testing::ext;

namespace OHOS {
namespace AppExecFwk {
class FormFuzzTest : public testing::Test {
public:
    static void SetUpTestCase();
    static void TearDownTestCase();
    void SetUp();
    void TearDown();
};

void FormFuzzTest::SetUpTestCase()
{
    std::cout << "FormFuzzTest SetUpTestCase" << std::endl;
}

void FormFuzzTest::TearDownTestCase()
{
    std::cout << "FormFuzzTest TearDownTestCase" << std::endl;
}

void FormFuzzTest::SetUp()
{
    std::cout << "FormFuzzTest SetUp" << std::endl;
}

void FormFuzzTest::TearDown()
{
    std::cout << "FormFuzzTest TearDown" << std::endl;
}

/**
 * @tc.number: FMS_fuzztest_0100
 * @tc.name: form manager service stress test
 * @tc.desc: 1.under root,there is a config.json
 *           2../FormManagerFuzzTest; hilog > xxxx.log
 */
HWTEST_F(FormFuzzTest, FMS_fuzztest_0100, Function | MediumTest | Level1)
{
    std::cout << "Begin FMS_fuzztest_0100" << std::endl;

    FormManagerFuzzTest::GetInstance()->StartFuzzTest();

    std::cout << "End FMS_fuzztest_0100" << std::endl;
}
}  // namespace AppExecFwk
}  // namespace OHOS