/*
 * Copyright (c) 2022 Huawei Device Co., Ltd.
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
#include <memory>
#include <thread>

#include "zip.h"

namespace OHOS {
namespace AAFwk {
namespace LIBZIP {
using namespace testing::ext;

namespace {
define BASE_PATH   "/data/accounts/account_0/appdata/"\
                   "com.example.zlib/com.example.zlib/com.example.zlib.MainAbility/files/"
}  // namespac
class ZipTest : public testing::Test {
public:
    ZipTest()
    {}
    ~ZipTest()
    {}

    static void SetUpTestCase(void);
    static void TearDownTestCase(void);
    void SetUp();
    void TearDown();
};

void ZipTest::SetUpTestCase(void)
{}

void ZipTest::TearDownTestCase(void)
{}

void ZipTest::SetUp()
{}

void ZipTest::TearDown()
{}

void ZipCallBack(int result)
{
    printf("--Zip--callback--result=%d--\n", result);
}
void UnzipCallBack(int result)
{
    printf("--UnZip--callback--result=%d--\n", result);
}

/**
 * @tc.number: AAFwk_LIBZIP_zip_0100_8file
 * @tc.name: zip_0100_8file
 * @tc.desc:
 */
HWTEST_F(ZipTest, AAFwk_LIBZIP_zip_0100_8file, Function | MediumTest | Level1)
{
    std::string src = BASE_PATH + "test";
    std::string dest = BASE_PATH + "result/8file.zip";

    OPTIONS options;
    Zip(FilePath(src), FilePath(dest), options, ZipCallBack, false);
    std::this_thread::sleep_for(std::chrono::milliseconds(5000));
}

/**
 * @tc.number: AAFwk_LIBZIP_zip_0200_1file
 * @tc.name: zip_0200_1file
 * @tc.desc:
 */
HWTEST_F(ZipTest, AAFwk_LIBZIP_zip_0200_1file, Function | MediumTest | Level1)
{
    std::string src = BASE_PATH + "test/01";
    std::string dest = BASE_PATH + "result/1file.zip";

    OPTIONS options;
    Zip(FilePath(src), FilePath(dest), options, ZipCallBack, false);
    std::this_thread::sleep_for(std::chrono::milliseconds(5000));
}

/**
 * @tc.number: AAFwk_LIBZIP_zip_0100_zip1file
 * @tc.name: zip_0100_zip1file
 * @tc.desc:
 */
HWTEST_F(ZipTest, AAFwk_LIBZIP_zip_0100_zip1file, Function | MediumTest | Level1)
{
    std::string src = BASE_PATH + "test/01/zip1.txt";
    std::string dest = BASE_PATH + "result/zip1file.zip";

    OPTIONS options;
    Zip(FilePath(src), FilePath(dest), options, ZipCallBack, false);
    std::this_thread::sleep_for(std::chrono::milliseconds(5000));
}

/**
 * @tc.number: AAFwk_LIBZIP_unzip_0100_8file
 * @tc.name: unzip_0100_8file
 * @tc.desc:
 */
HWTEST_F(ZipTest, AAFwk_LIBZIP_unzip_0100_8file, Function | MediumTest | Level1)
{
    std::string src = BASE_PATH + "result/8file.zip";
    std::string dest = BASE_PATH + "unzip/01";

    OPTIONS options;
    Unzip(FilePath(src), FilePath(dest), options, UnzipCallBack);
    std::this_thread::sleep_for(std::chrono::milliseconds(5000));
}
/**
 * @tc.number: AAFwk_LIBZIP_unzip_single_0200_1file
 * @tc.name: unzip_single_0200_1file
 * @tc.desc:
 */
HWTEST_F(ZipTest, AAFwk_LIBZIP_unzip_single_0200_1file, Function | MediumTest | Level1)
{
    std::string src = BASE_PATH + "result/1file.zip";
    std::string dest = BASE_PATH + "unzip/02";

    OPTIONS options;
    Unzip(FilePath(src), FilePath(dest), options, UnzipCallBack);
    std::this_thread::sleep_for(std::chrono::milliseconds(5000));
}
/**
 * @tc.number: AAFwk_LIBZIP_zip_0100_zip1file
 * @tc.name: zip_0100_zip1file
 * @tc.desc:
 */
HWTEST_F(ZipTest, AAFwk_LIBZIP_unzip_0100_zip1file, Function | MediumTest | Level1)
{
    std::string src = BASE_PATH + "result/zip1file.zip";
    std::string dest = BASE_PATH + "unzip/zip1file";

    OPTIONS options;
    Unzip(FilePath(src), FilePath(dest), options, UnzipCallBack);
    std::this_thread::sleep_for(std::chrono::milliseconds(5000));
}
}  // namespace LIBZIP
}  // namespace AAFwk
}  // namespace OHOS