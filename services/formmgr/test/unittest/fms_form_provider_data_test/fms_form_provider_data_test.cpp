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
#include <dirent.h>
#include <fstream>
#include <gtest/gtest.h>
#include <string>
#include <thread>

#include "app_log_wrapper.h"
#define private public
#include "form_provider_data.h"
#undef private
#include "nlohmann/json.hpp"

using namespace testing::ext;
using namespace OHOS;
using namespace OHOS::AppExecFwk;

namespace {
const std::string FORM_DB_DATA_BASE_FILE_DIR = "/data/formmgr";
const int32_t four = 4;
const int32_t ten = 10;
const int32_t eleven = 11;

class FmsFormProviderDataTest : public testing::Test {
public:
    void SetUp();
    void Test();
    bool InitJsonData();
    bool InitJsonData2();
    bool CreateJsonFileByJsonData1(const nlohmann::json &jsonData);
    bool CreateJsonFileByJsonData2(const nlohmann::json &jsonData);
    bool CreateMergeJsonFileByJsonData3(const nlohmann::json &jsonData);

    nlohmann::json jsonData_;
};
void FmsFormProviderDataTest::SetUp()
{
    DIR *dirptr = opendir(FORM_DB_DATA_BASE_FILE_DIR.c_str());
    if (dirptr == nullptr) {
        APP_LOGW("%{public}s, opendir is fail", __func__);
        if (-1 == mkdir(FORM_DB_DATA_BASE_FILE_DIR.c_str(), S_IRWXU)) {
            APP_LOGE("%{public}s, dir create fail", __func__);
            return;
        }
    } else {
        closedir(dirptr);
    }
}

bool FmsFormProviderDataTest::InitJsonData()
{
    nlohmann::json tmpJson;
    tmpJson["name"] = "li";
    tmpJson["age"] = ten;
    jsonData_["0"] = tmpJson;
    return true;
}

bool FmsFormProviderDataTest::InitJsonData2()
{
    nlohmann::json tmpJson;
    tmpJson["name"] = "wang";
    tmpJson["age"] = eleven;
    jsonData_["1"] = tmpJson;
    return true;
}

bool FmsFormProviderDataTest::CreateJsonFileByJsonData1(const nlohmann::json &jsonData)
{
    std::ofstream o("/data/formmgr/ByJsonFile1.json");
    o.close();

    std::fstream f("/data/formmgr/ByJsonFile1.json");
    if (f.good() == false) {
        return false;
    }

    f << std::setw(four) << jsonData << std::endl;

    f.close();
    return true;
}

bool FmsFormProviderDataTest::CreateJsonFileByJsonData2(const nlohmann::json &jsonData)
{
    std::ofstream o("/data/formmgr/ByJsonFile2.json");
    o.close();

    std::fstream f("/data/formmgr/ByJsonFile2.json");
    if (f.good() == false) {
        return false;
    }

    f << std::setw(four) << jsonData << std::endl;

    f.close();
    return true;
}

bool FmsFormProviderDataTest::CreateMergeJsonFileByJsonData3(const nlohmann::json &jsonData)
{
    std::ofstream o("/data/formmgr/ByJsonFile3.json");
    o.close();

    std::fstream f("/data/formmgr/ByJsonFile3.json");
    if (f.good() == false) {
        return false;
    }

    f << std::setw(four) << jsonData << std::endl;

    f.close();
    return true;
}

HWTEST_F(FmsFormProviderDataTest, FmsFormProviderDataTest_001, TestSize.Level0) // create
{
    GTEST_LOG_(INFO) << "FmsFormProviderDataTest_001 start";
    EXPECT_EQ(true, InitJsonData());
    FormProviderData formProviderData(jsonData_);
    EXPECT_EQ(true, CreateJsonFileByJsonData1(formProviderData.jsonFormProviderData_));
    GTEST_LOG_(INFO) << "FmsFormProviderDataTest_001 end";
}

HWTEST_F(FmsFormProviderDataTest, FmsFormProviderDataTest_002, TestSize.Level0) // test constructor with string
{
    GTEST_LOG_(INFO) << "FmsFormProviderDataTest_002 start";
    EXPECT_EQ(true, InitJsonData());
    FormProviderData formProviderData(jsonData_.dump());
    EXPECT_EQ(true, CreateJsonFileByJsonData2(formProviderData.jsonFormProviderData_));
    GTEST_LOG_(INFO) << "FmsFormProviderDataTest_002 end";
}

HWTEST_F(FmsFormProviderDataTest, FmsFormProviderDataTest_003, TestSize.Level0) // test GetDataString
{
    GTEST_LOG_(INFO) << "FmsFormProviderDataTest_003 start";
    EXPECT_EQ(true, InitJsonData());
    FormProviderData formProviderData(jsonData_);
    GTEST_LOG_(INFO) << "print:" <<formProviderData.GetDataString();
    GTEST_LOG_(INFO) << "FmsFormProviderDataTest_003 end";
}

HWTEST_F(FmsFormProviderDataTest, FmsFormProviderDataTest_004, TestSize.Level0) // test MergeData
{
    GTEST_LOG_(INFO) << "FmsFormProviderDataTest_004 start";
    EXPECT_EQ(true, InitJsonData());
    FormProviderData formProviderData(jsonData_);
    EXPECT_EQ(true, InitJsonData2());
    formProviderData.MergeData(jsonData_);
    EXPECT_EQ(true, CreateMergeJsonFileByJsonData3(formProviderData.jsonFormProviderData_));
    GTEST_LOG_(INFO) << "FmsFormProviderDataTest_004 end";
}
}