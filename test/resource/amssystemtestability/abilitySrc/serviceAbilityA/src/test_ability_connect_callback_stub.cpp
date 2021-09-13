
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

#include "test_ability_connect_callback_stub.h"

#include "test_ability_connect_callback_proxy.h"
#include "hilog_wrapper.h"
#include "ipc_types.h"
#include "message_parcel.h"
#include "want.h"

namespace OHOS {
namespace AppExecFwk {
bool TestAbilityConnectionProxy::WriteInterfaceToken(MessageParcel &data)
{
    if (!data.WriteInterfaceToken(TestAbilityConnectionProxy::GetDescriptor())) {
        HILOG_ERROR("write interface token failed");
        return false;
    }
    return true;
}

int32_t TestAbilityConnectionProxy::ScheduleAdd(int32_t a, int32_t b)
{
    HILOG_DEBUG("%s, a:%d,b:%d", __func__, a, b);

    int error;
    MessageParcel data;
    MessageParcel reply;
    MessageOption option;

    if (!WriteInterfaceToken(data)) {
        return -1;
    }

    if (!data.WriteInt32(a)) {
        HILOG_ERROR("connect done element error");
        return -1;
    }

    if (!data.WriteInt32(b)) {
        HILOG_ERROR("connect done element error");
        return -1;
    }

    error = Remote()->SendRequest(IServiceRemoteTest::ON_ABILITY_ADD, data, reply, option);
    if (error != NO_ERROR) {
        HILOG_ERROR("connect done fail, error: %d", error);

        int32_t result = 0;
        if (!reply.ReadInt32(result)) {
            HILOG_ERROR("fail to ReadInt32 fd");
        }
        return result;
    }
    return -1;
}

int32_t TestAbilityConnectionProxy::ScheduleSub(int32_t a, int32_t b)
{
    HILOG_DEBUG("%s, a:%d, b:%d", __func__, a, b);
    int error;
    MessageParcel data;
    MessageParcel reply;
    MessageOption option;

    if (!WriteInterfaceToken(data)) {
        return -1;
    }

    if (!data.WriteInt32(a)) {
        HILOG_ERROR("connect done element error");
        return -1;
    }

    if (!data.WriteInt32(b)) {
        HILOG_ERROR("connect done element error");
        return -1;
    }
    error = Remote()->SendRequest(IServiceRemoteTest::ON_ABILITY_SUB, data, reply, option);
    if (error != NO_ERROR) {
        HILOG_ERROR("disconnect done fail, error: %d", error);

        int32_t result = 0;
        if (!reply.ReadInt32(result)) {
            HILOG_ERROR("fail to ReadInt32 fd");
        }
        return result;
    }
    return -1;
}

TestAbilityConnectionStub::TestAbilityConnectionStub()
{}

TestAbilityConnectionStub::~TestAbilityConnectionStub()
{}

int TestAbilityConnectionStub::OnRemoteRequest(
    uint32_t code, MessageParcel &data, MessageParcel &reply, MessageOption &option)
{
    std::u16string descriptor = TestAbilityConnectionStub::GetDescriptor();
    std::u16string remoteDescriptor = data.ReadInterfaceToken();
    if (descriptor != remoteDescriptor) {
        HILOG_INFO("local descriptor is not equal to remote");
        return ERR_INVALID_STATE;
    }

    switch (code) {
        case IServiceRemoteTest::ON_ABILITY_ADD: {
            int a = 0;
            if (!data.ReadInt32(a)) {
                HILOG_ERROR("fail to ReadInt32 fd");
            }

            int b = 0;
            if (!data.ReadInt32(b)) {
                HILOG_ERROR("fail to ReadInt32 fd");
            }

            uint32_t res = ScheduleAdd(a, b);
            reply.WriteInt32(res);

            return NO_ERROR;
        }
        default: {
            return IPCObjectStub::OnRemoteRequest(code, data, reply, option);
        }
    }
}

}  // namespace AppExecFwk
}  // namespace OHOS
