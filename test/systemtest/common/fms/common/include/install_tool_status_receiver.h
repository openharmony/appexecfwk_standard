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

#ifndef INSTALL_TOOL_STATUS_RECEIVER_H
#define INSTALL_TOOL_STATUS_RECEIVER_H

#include "form_event.h"

namespace OHOS {
namespace STtools {
using namespace OHOS::AppExecFwk;
class InstallToolStatusReceiver : public StatusReceiverHost {
public:
    InstallToolStatusReceiver();
    virtual ~InstallToolStatusReceiver() override;
    virtual void OnStatusNotify(const int progress) override;
    virtual void OnFinished(const int32_t resultCode, const std::string &resultMsg) override;
    static int TestWaitCompleted(FormEvent &event, const std::string eventName, const int code, const int timeout = 10);
    static void TestCompleted(FormEvent &event, const std::string &eventName, const int code);
    FormEvent event_ = STtools::FormEvent();

private:
    int iProgress_ = 0;
    DISALLOW_COPY_AND_MOVE(InstallToolStatusReceiver);
};
InstallToolStatusReceiver::InstallToolStatusReceiver()
{
    std::cout << "create status receiver instance" << std::endl;
}

InstallToolStatusReceiver::~InstallToolStatusReceiver()
{
    std::cout << "destroy status receiver instance" << std::endl;
}

void InstallToolStatusReceiver::OnStatusNotify(const int progress)
{
    iProgress_ = progress;
    std::cout << "destroy status receiver instance" << progress << std::endl;
}

void InstallToolStatusReceiver::OnFinished(const int32_t resultCode, const std::string &resultMsg)
{
    std::cout << "on finished result is " << resultCode << " " << resultMsg << std::endl;
    TestCompleted(event_, resultMsg, resultCode);
}

int InstallToolStatusReceiver::TestWaitCompleted(
    FormEvent &event, const std::string eventName, const int code, const int timeout)
{
    std::cout << "TestWaitCompleted " << eventName << std::endl;
    return SystemTestFormUtil::WaitCompleted(event, eventName, code, timeout);
}
void InstallToolStatusReceiver::TestCompleted(FormEvent &event, const std::string &eventName, const int code)
{
    std::cout << "TestCompleted " << eventName << std::endl;
    SystemTestFormUtil::Completed(event, eventName, code);
    return;
}
}  // namespace AppExecFwk
}  // namespace OHOS
#endif  // INSTALL_TOOL_STATUS_RECEIVER_H