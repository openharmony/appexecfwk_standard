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

#ifndef FOUNDATION_APPEXECFWK_SERVICES_APPMGR_INCLUDE_LMKS_SERVER_H
#define FOUNDATION_APPEXECFWK_SERVICES_APPMGR_INCLUDE_LMKS_SERVER_H

#include <string>
#include <sys/un.h>

#include "lmks_utils.h"

namespace OHOS {
namespace LMKS {

constexpr uint32_t MAX_LMKS_TARGETS = 6;                                                // max number of lmks command targets
constexpr uint32_t MAX_LMKS_TARGETS_SIZE = sizeof(int) * (MAX_LMKS_TARGETS * 2 + 1);    // max packet length

using LMKS_PACKET=int[MAX_LMKS_TARGETS_SIZE / sizeof(int)];

class LmksServer {
public:
    LmksServer();
    virtual ~LmksServer();

    void StartServer();

private:
    int RegisterSocket();
    int WaitConnection();
    int RecvSocketMessage(int connectFd, void *buf, int len);
    int SendSocketMessage(int connectFd, const void *buf, int len);
    void ProcessMessage(int connectFd, LMKS_PACKET cmds, int len);
    void CloseSocket();

private:
    bool isStart_;
    int socketFd_;
    struct sockaddr_un socketAddr_;
    uint32_t socketAddrLen_;
    std::shared_ptr<LmksUtils> lmksUtils_;
};
}  // namespace LMKS
}  // namespace OHOS

#endif  // FOUNDATION_APPEXECFWK_SERVICES_APPMGR_INCLUDE_LMKS_SERVER_H
