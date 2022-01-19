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
#include <memory>
#include <thread>

#include "zip.h"

namespace OHOS {
namespace AAFwk {
namespace LIBZIP {
using namespace testing::ext;

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
 * @tc.number:
 * @tc.name:
 * @tc.desc:
 */
HWTEST_F(ZipTest, AAFwk_LIBZIP_zip_0100_fourfile, Function | MediumTest | Level1)
{
    std::string src = "/ziptest/zipdata/";
    std::string dest = "/ziptest/result/fourfile.zip";

    OPTIONS options;
    Zip(FilePath(src), FilePath(dest), options, ZipCallBack, false);
    std::this_thread::sleep_for(std::chrono::milliseconds(5000));
}

/**
 * @tc.number:
 * @tc.name:
 * @tc.desc:
 */
HWTEST_F(ZipTest, AAFwk_LIBZIP_zip_0200_singlefile, Function | MediumTest | Level1)
{
    std::string src = "/ziptest/zipdata/zip1/zip1-1/zip1-1.cpp";
    std::string dest = "/ziptest/result/singlefile.zip";

    OPTIONS options;
    Zip(FilePath(src), FilePath(dest), options, ZipCallBack, false);
    std::this_thread::sleep_for(std::chrono::milliseconds(5000));
}

/**
 * @tc.number:
 * @tc.name:
 * @tc.desc:
 */
HWTEST_F(ZipTest, AAFwk_LIBZIP_unzip_0100, Function | MediumTest | Level1)
{
    std::string dest = "/ziptest/unzipdir/fourfile/";
    std::string src = "/ziptest/result/fourfile.zip";

    OPTIONS options;
    Unzip(FilePath(src), FilePath(dest), options, UnzipCallBack);
    std::this_thread::sleep_for(std::chrono::milliseconds(5000));
}

HWTEST_F(ZipTest, AAFwk_LIBZIP_unzip_single_0200, Function | MediumTest | Level1)
{
    std::string src = "/ziptest/result/singlefile.zip";
    std::string dest = "/ziptest/unzipdir/single/";

    OPTIONS options;
    Unzip(FilePath(src), FilePath(dest), options, UnzipCallBack);
    std::this_thread::sleep_for(std::chrono::milliseconds(5000));
}

}  // namespace LIBZIP
}  // namespace AAFwk
}  // namespace OHOS