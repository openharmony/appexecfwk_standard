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

#define private public
#include "lmks_client.h"
#undef private

#include <gtest/gtest.h>
#include <fcntl.h>
#include <sys/eventfd.h>
#include <unistd.h>
#include <sys/socket.h>

#include "app_log_wrapper.h"

using namespace testing::ext;

namespace OHOS {
namespace AppExecFwk {

class AmsLmksClientTest : public testing::Test {
public:
    static void SetUpTestCase();
    static void TearDownTestCase();
    void SetUp();
    void TearDown();
};

void AmsLmksClientTest::SetUpTestCase()
{}
void AmsLmksClientTest::TearDownTestCase()
{}
void AmsLmksClientTest::SetUp()
{}
void AmsLmksClientTest::TearDown()
{}

/*
 * Feature: AMS
 * Function: LmksClient
 * SubFunction: Open
 * FunctionPoints: open socket
 * EnvConditions: Mobile that can run ohos test framework.
 * CaseDescription: Verify the function Open can check the invalid socket_
 */
HWTEST_F(AmsLmksClientTest, AmsLmksClientTest_Open_001, TestSize.Level0)
{
    APP_LOGD("AmsLmksClientTest_Open_001 start.");

    std::unique_ptr<LmksClient> amsLmksClient = std::make_unique<LmksClient>();
    amsLmksClient->socket_ = 0;
    EXPECT_EQ(-1, amsLmksClient->Open());

    APP_LOGD("AmsLmksClientTest_Open_001 end.");
}

/*
 * Feature: AMS
 * Function: LmksClient
 * SubFunction: Open
 * FunctionPoints: open socket
 * EnvConditions: Mobile that can run ohos test framework.
 * CaseDescription: open socket success
 */
HWTEST_F(AmsLmksClientTest, AmsLmksClientTest_Open_002, TestSize.Level0)
{
    APP_LOGD("AmsLmksClientTest_Open_002 start.");

    std::unique_ptr<LmksClient> amsLmksClient = std::make_unique<LmksClient>();
    amsLmksClient->socket_ = -1;
    EXPECT_EQ(ERR_OK, amsLmksClient->Open());

    APP_LOGD("AmsLmksClientTest_Open_002 end.");
}

/*
 * Feature: AMS
 * Function: LmksClient
 * SubFunction: Target
 * FunctionPoints: Oom adj mapping table value of the initialization process
 * EnvConditions: Mobile that can run ohos test framework.
 * CaseDescription: Verify the function Target can check the invalid targets
 */
HWTEST_F(AmsLmksClientTest, AmsLmksClientTest_Target_001, TestSize.Level0)
{
    APP_LOGD("AmsLmksClientTest_Target_001 start.");

    std::vector<std::pair<int, int>> targets;
    std::unique_ptr<LmksClient> amsLmksClient = std::make_unique<LmksClient>();

    EXPECT_EQ(-EINVAL, amsLmksClient->Target(targets));

    APP_LOGD("AmsLmksClientTest_Target_001 end.");
}

/*
 * Feature: AMS
 * Function: LmksClient
 * SubFunction: Target
 * FunctionPoints: Oom adj mapping table value of the initialization process
 * EnvConditions: Mobile that can run ohos test framework.
 * CaseDescription: Verify the function Target can check the invalid targets
 */
HWTEST_F(AmsLmksClientTest, AmsLmksClientTest_Target_002, TestSize.Level0)
{
    APP_LOGD("AmsLmksClientTest_Target_002 start.");

    std::vector<std::pair<int, int>> targets{{1, 2}, {3, 4}, {5, 6}, {7, 8}, {9, 10}, {11, 12}, {13, 14}};
    std::unique_ptr<LmksClient> amsLmksClient = std::make_unique<LmksClient>();

    EXPECT_EQ(-EINVAL, amsLmksClient->Target(targets));

    APP_LOGD("AmsLmksClientTest_Target_002 end.");
}

/*
 * Feature: AMS
 * Function: LmksClient
 * SubFunction: Target
 * FunctionPoints: Oom adj mapping table value of the initialization process
 * EnvConditions: Mobile that can run ohos test framework.
 * CaseDescription: Verify the function Target can check the invalid targets
 */
HWTEST_F(AmsLmksClientTest, AmsLmksClientTest_Target_003, TestSize.Level0)
{
    APP_LOGD("AmsLmksClientTest_Target_003 start.");

    std::vector<std::pair<int, int>> targets{{-1, 2}};
    std::unique_ptr<LmksClient> amsLmksClient = std::make_unique<LmksClient>();

    EXPECT_EQ(-EINVAL, amsLmksClient->Target(targets));

    APP_LOGD("AmsLmksClientTest_Target_003 end.");
}

/*
 * Feature: AMS
 * Function: LmksClient
 * SubFunction: Target
 * FunctionPoints: Oom adj mapping table value of the initialization process
 * EnvConditions: Mobile that can run ohos test framework.
 * CaseDescription: Verify the function Target can check the invalid targets
 */
HWTEST_F(AmsLmksClientTest, AmsLmksClientTest_Target_004, TestSize.Level0)
{
    APP_LOGD("AmsLmksClientTest_Target_004 start.");

    std::vector<std::pair<int, int>> targets{{1, 1100}};
    std::unique_ptr<LmksClient> amsLmksClient = std::make_unique<LmksClient>();

    EXPECT_EQ(-EINVAL, amsLmksClient->Target(targets));

    APP_LOGD("AmsLmksClientTest_Target_004 end.");
}

/*
 * Feature: AMS
 * Function: LmksClient
 * SubFunction: Target
 * FunctionPoints: Oom adj mapping table value of the initialization process
 * EnvConditions: Mobile that can run ohos test framework.
 * CaseDescription: Verify that the function Target can be executed normally
 */
HWTEST_F(AmsLmksClientTest, AmsLmksClientTest_Target_005, TestSize.Level0)
{
    APP_LOGD("AmsLmksClientTest_Target_005 start.");

    std::vector<std::pair<int, int>> targets{{1, 10}};
    std::unique_ptr<LmksClient> amsLmksClient = std::make_unique<LmksClient>();

    EXPECT_EQ(ERR_OK, amsLmksClient->Target(targets));

    APP_LOGD("AmsLmksClientTest_Target_005 end.");
}

/*
 * Feature: AMS
 * Function: LmksClient
 * SubFunction: ProcPrio
 * FunctionPoints: Update the oom adj value of pid
 * EnvConditions: Mobile that can run ohos test framework.
 * CaseDescription: Verify the function ProcPrio can check the invalid pid
 */
HWTEST_F(AmsLmksClientTest, AmsLmksClientTest_ProcPrio_001, TestSize.Level0)
{
    APP_LOGD("AmsLmksClientTest_ProcPrio_001 start.");

    std::unique_ptr<LmksClient> amsLmksClient = std::make_unique<LmksClient>();
    pid_t pid = -1;
    uid_t uid = 1;
    int oomAdj = 0;
    EXPECT_EQ(-EINVAL, amsLmksClient->ProcPrio(pid, uid, oomAdj));

    APP_LOGD("AmsLmksClientTest_ProcPrio_001 end.");
}

/*
 * Feature: AMS
 * Function: LmksClient
 * SubFunction: ProcPrio
 * FunctionPoints: Update the oom adj value of pid
 * EnvConditions: Mobile that can run ohos test framework.
 * CaseDescription: Verify the function ProcPrio can check the invalid oomAdj
 */
HWTEST_F(AmsLmksClientTest, AmsLmksClientTest_ProcPrio_002, TestSize.Level0)
{
    APP_LOGD("AmsLmksClientTest_ProcPrio_002 start.");

    std::unique_ptr<LmksClient> amsLmksClient = std::make_unique<LmksClient>();
    pid_t pid = 1;
    uid_t uid = 1;
    int oomAdj = 1100;
    EXPECT_EQ(-EINVAL, amsLmksClient->ProcPrio(pid, uid, oomAdj));

    APP_LOGD("AmsLmksClientTest_ProcPrio_002 end.");
}

/*
 * Feature: AMS
 * Function: LmksClient
 * SubFunction: ProcPrio
 * FunctionPoints: Update the oom adj value of pid
 * EnvConditions: Mobile that can run ohos test framework.
 * CaseDescription: Verify that the function ProcPrio can be executed normally
 */
HWTEST_F(AmsLmksClientTest, AmsLmksClientTest_ProcPrio_003, TestSize.Level0)
{
    APP_LOGD("AmsLmksClientTest_ProcPrio_003 start.");

    std::unique_ptr<LmksClient> amsLmksClient = std::make_unique<LmksClient>();
    pid_t pid = 1;
    uid_t uid = 1;
    int oomAdj = 10;
    EXPECT_EQ(ERR_OK, amsLmksClient->ProcPrio(pid, uid, oomAdj));

    APP_LOGD("AmsLmksClientTest_ProcPrio_003 end.");
}

/*
 * Feature: AMS
 * Function: LmksClient
 * SubFunction: ProcRemove
 * FunctionPoints: remove process
 * EnvConditions: Mobile that can run ohos test framework.
 * CaseDescription: Verify the function ProcRemove can check the invalid pid
 */
HWTEST_F(AmsLmksClientTest, AmsLmksClientTest_ProcRemove_001, TestSize.Level0)
{
    APP_LOGD("AmsLmksClientTest_ProcRemove_001 start.");

    std::unique_ptr<LmksClient> amsLmksClient = std::make_unique<LmksClient>();
    pid_t pid = -1;
    EXPECT_EQ(-EINVAL, amsLmksClient->ProcRemove(pid));

    APP_LOGD("AmsLmksClientTest_ProcRemove_001 end.");
}

/*
 * Feature: AMS
 * Function: LmksClient
 * SubFunction: ProcRemove
 * FunctionPoints: remove process
 * EnvConditions: Mobile that can run ohos test framework.
 * CaseDescription: Verify the function ProcRemove success to remove process
 */
HWTEST_F(AmsLmksClientTest, AmsLmksClientTest_ProcRemove_002, TestSize.Level0)
{
    APP_LOGD("AmsLmksClientTest_ProcRemove_002 start.");

    std::unique_ptr<LmksClient> amsLmksClient = std::make_unique<LmksClient>();
    pid_t pid = 1;
    EXPECT_EQ(0, amsLmksClient->ProcRemove(pid));

    APP_LOGD("AmsLmksClientTest_ProcRemove_002 end.");
}

/*
 * Feature: AMS
 * Function: LmksClient
 * SubFunction: Write
 * FunctionPoints: send data
 * EnvConditions: Mobile that can run ohos test framework.
 * CaseDescription: Verify the function Write can check the invalid buf
 */
HWTEST_F(AmsLmksClientTest, AmsLmksClientTest_Write_001, TestSize.Level0)
{
    APP_LOGD("AmsLmksClientTest_Write_001 start.");

    std::unique_ptr<LmksClient> amsLmksClient = std::make_unique<LmksClient>();
    int32_t *buf = nullptr;
    size_t len = 2;
    EXPECT_EQ(false, amsLmksClient->Write(buf, len));

    APP_LOGD("AmsLmksClientTest_Write_001 end.");
}

/*
 * Feature: AMS
 * Function: LmksClient
 * SubFunction: Write
 * FunctionPoints: send data
 * EnvConditions: Mobile that can run ohos test framework.
 * CaseDescription: Verify the function Write can check the invalid len
 */
HWTEST_F(AmsLmksClientTest, AmsLmksClientTest_Write_002, TestSize.Level0)
{
    APP_LOGD("AmsLmksClientTest_Write_002 start.");

    std::unique_ptr<LmksClient> amsLmksClient = std::make_unique<LmksClient>();
    int32_t buf[2] = {1, 2};
    size_t len = -1;
    EXPECT_EQ(false, amsLmksClient->Write(buf, len));

    APP_LOGD("AmsLmksClientTest_Write_002 end.");
}

/*
 * Feature: AMS
 * Function: LmksClient
 * SubFunction: Read
 * FunctionPoints: Receive data
 * EnvConditions: Mobile that can run ohos test framework.
 * CaseDescription: Verify the function Read can check the invalid buf
 */
HWTEST_F(AmsLmksClientTest, AmsLmksClientTest_Read_001, TestSize.Level0)
{
    APP_LOGD("AmsLmksClientTest_Read_001 start.");

    std::unique_ptr<LmksClient> amsLmksClient = std::make_unique<LmksClient>();
    int32_t *buf = nullptr;
    size_t len = 2;
    EXPECT_EQ(false, amsLmksClient->Read(buf, len));

    APP_LOGD("AmsLmksClientTest_Read_001 end.");
}

/*
 * Feature: AMS
 * Function: LmksClient
 * SubFunction: Read
 * FunctionPoints: Receive data
 * EnvConditions: Mobile that can run ohos test framework.
 * CaseDescription: Verify the function Read can check the invalid len
 */
HWTEST_F(AmsLmksClientTest, AmsLmksClientTest_Read_002, TestSize.Level0)
{
    APP_LOGD("AmsLmksClientTest_Read_002 start.");

    std::unique_ptr<LmksClient> amsLmksClient = std::make_unique<LmksClient>();
    int32_t buf[2] = {1, 2};
    size_t len = -1;
    EXPECT_EQ(false, amsLmksClient->Read(buf, len));

    APP_LOGD("AmsLmksClientTest_Read_002 end.");
}

/*
 * Feature: AMS
 * Function: LmksClient
 * SubFunction: Close
 * FunctionPoints: close socket
 * EnvConditions: Mobile that can run ohos test framework.
 * CaseDescription: Check whether the close function closes the socket
 */
HWTEST_F(AmsLmksClientTest, AmsLmksClientTest_Close_001, TestSize.Level0)
{
    APP_LOGD("AmsLmksClientTest_Close_001 start.");

    std::unique_ptr<LmksClient> amsLmksClient = std::make_unique<LmksClient>();
    amsLmksClient->Close();
    EXPECT_EQ(false, amsLmksClient->IsOpen());

    APP_LOGD("AmsLmksClientTest_Close_001 end.");
}

}  // namespace AppExecFwk
}  // namespace OHOS