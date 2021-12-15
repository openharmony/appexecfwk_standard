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

#include "lmks_server.h"

#include <sys/socket.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <thread>
#include <unistd.h>

#include "securec.h"
#include "hilog/log.h"

namespace OHOS {
namespace LMKS {
namespace {
#ifdef __MUSL__
const std::string LMKS_SOCKET_NAME = "/dev/unix/socket/lmks";
#else
const std::string LMKS_SOCKET_NAME = "/dev/socket/lmks";
#endif
constexpr uint32_t LISTEN_CLIENTS = 5;          // 5: max num of clients
constexpr uint32_t WAIT_DELAY_US = 100 * 1000;  // 100ms

constexpr int LMKS_CMD_TARGET = 0;
constexpr int LMKS_CMD_PROCPRIO = 1;
constexpr int LMKS_CMD_PROCREMOVE = 2;
constexpr int LMKS_CMD_PROCPURGE = 3;
constexpr uid_t LMKS_ID_ROOT = 0;                  // chown owner
constexpr gid_t LMKS_ID_SYSTEM = 1000;             // chown group
constexpr mode_t SOCKET_PERM = 0666;               // root system can read and write lmks socket
constexpr struct timeval SOCKET_TIMEOUT = {5, 0};  // 5, 0: { 5 sec, 0 msec } for timeout
}  // namespace

using namespace OHOS::HiviewDFX;
static constexpr HiLogLabel LABEL = {LOG_CORE, 0, "LmksServer"};

LmksServer::LmksServer() : isStart_(false), socketFd_(-1), socketAddrLen_(0), lmksUtils_(nullptr)
{
    memset_s(&socketAddr_, sizeof(socketAddr_), 0, sizeof(socketAddr_));
}

LmksServer::~LmksServer()
{
    CloseSocket();
}

void LmksServer::StartServer()
{
    if (isStart_) {
        HiLog::Error(LABEL, "Lmks server has started.");
        return;
    }

    if (RegisterSocket() != 0) {
        HiLog::Error(LABEL, "Register error.");
        return;
    }

    isStart_ = true;
    if (lmksUtils_ == nullptr) {
        lmksUtils_ = std::make_shared<LmksUtils>();
    }

    while (1) {
        int connectFd = WaitConnection();
        if (connectFd < 0) {
            usleep(WAIT_DELAY_US);
            continue;
        }

        LMKS_PACKET cmds;
        int len = RecvSocketMessage(connectFd, (void *)cmds, sizeof(cmds));
        if (len <= 0) {
            HiLog::Error(LABEL, "Failed to read socket message, len %{public}d", len);
            close(connectFd);
            continue;
        }

        ProcessMessage(connectFd, cmds, len);
    }

    // close socket
    CloseSocket();
}

int LmksServer::RegisterSocket()
{
    if (socketFd_ > 0) {
        HiLog::Error(LABEL, "Lmks server has already register.");
        return -1;
    }

    socketFd_ = socket(AF_LOCAL, SOCK_SEQPACKET, 0);
    if (socketFd_ < 0) {
        HiLog::Error(LABEL, "Failed to create socket, err %{public}s", strerror(errno));
        return (-errno);
    }

    if (memset_s(&socketAddr_, sizeof(socketAddr_), 0, sizeof(socketAddr_)) != 0) {
        HiLog::Error(LABEL, "Failed to memset socket addr, err %{public}s", strerror(errno));
        return (-errno);
    }

    if (strcpy_s(socketAddr_.sun_path, sizeof(socketAddr_.sun_path), LMKS_SOCKET_NAME.c_str()) != 0) {
        HiLog::Error(LABEL, "Failed to snprint32_tf_s socket addr, err %{public}s", strerror(errno));
        return (-errno);
    }

    socketAddr_.sun_family = AF_LOCAL;
    socketAddrLen_ = offsetof(struct sockaddr_un, sun_path) + LMKS_SOCKET_NAME.length() + 1;

    if ((unlink(socketAddr_.sun_path) != 0) && (errno != ENOENT)) {
        HiLog::Error(LABEL, "Failed to unlink, err %{public}s", strerror(errno));
        return (-errno);
    }

    int reuseAddr = 0;
    if ((setsockopt(socketFd_, SOL_SOCKET, SO_REUSEADDR, &reuseAddr, sizeof(reuseAddr)) != 0) ||
        (setsockopt(socketFd_, SOL_SOCKET, SO_RCVTIMEO, &SOCKET_TIMEOUT, sizeof(SOCKET_TIMEOUT)) != 0) ||
        (setsockopt(socketFd_, SOL_SOCKET, SO_SNDTIMEO, &SOCKET_TIMEOUT, sizeof(SOCKET_TIMEOUT)) != 0)) {
        HiLog::Error(LABEL, "Failed to set opt of socket %d, err %{public}s", socketFd_, strerror(errno));
        return (-errno);
    }

    if (bind(socketFd_, reinterpret_cast<struct sockaddr *>(&socketAddr_), socketAddrLen_) < 0) {
        HiLog::Error(LABEL, "Failed to bind socket fd %d, err %{public}s", socketFd_, strerror(errno));
        return (-errno);
    }

    if (chown(socketAddr_.sun_path, LMKS_ID_ROOT, LMKS_ID_SYSTEM)) {
        HiLog::Error(LABEL, "Failed to chown socket. err %{public}s", strerror(errno));
        return (-errno);
    }

    if (chmod(socketAddr_.sun_path, SOCKET_PERM)) {
        HiLog::Error(LABEL, "Failed to chmod socket. err %{public}s", strerror(errno));
        if ((unlink(socketAddr_.sun_path) != 0) && (errno != ENOENT)) {
            HiLog::Error(LABEL, "Failed to unlink, err %{public}s", strerror(errno));
        }
        return (-errno);
    }

    if (listen(socketFd_, LISTEN_CLIENTS) != 0) {
        HiLog::Error(LABEL, "Failed to listen socket fd %d, err %{public}s", socketFd_, strerror(errno));
        return (-errno);
    }

    return 0;
}

int LmksServer::WaitConnection()
{
    if (socketFd_ < 0) {
        HiLog::Error(LABEL, "lmks server not register.");
        return -1;
    }

    struct sockaddr_un clientAddr;
    socklen_t clientLen = sizeof(clientAddr);

    if (memset_s(&clientAddr, clientLen, 0, clientLen) != 0) {
        HiLog::Warn(LABEL, "Failed to memset client addr, err %{public}s", strerror(errno));
    }

    int connFd = accept(socketFd_, reinterpret_cast<struct sockaddr *>(&clientAddr), &clientLen);
    if (connFd < 0) {
        HiLog::Warn(LABEL, "Accept warning %{public}s", strerror(errno));
        return (-errno);
    }

    if ((setsockopt(connFd, SOL_SOCKET, SO_RCVTIMEO, &SOCKET_TIMEOUT, sizeof(SOCKET_TIMEOUT)) < 0) ||
        (setsockopt(connFd, SOL_SOCKET, SO_SNDTIMEO, &SOCKET_TIMEOUT, sizeof(SOCKET_TIMEOUT)) < 0)) {
        HiLog::Error(LABEL, "Failed to set opt of Connection %d, err %{public}s", connFd, strerror(errno));
        close(connFd);
        return (-errno);
    }

    return connFd;
}

void LmksServer::CloseSocket()
{
    if (socketFd_ >= 0) {
        HiLog::Debug(LABEL, "Closed socket with fd %d", socketFd_);
        close(socketFd_);
        socketFd_ = -1;
    }
}

int LmksServer::SendSocketMessage(int connectFd, const void *buf, int len)
{
    if (connectFd < 0 || len <= 0 || buf == nullptr) {
        HiLog::Error(LABEL, "Invalid args: connect %d, len %d, buf might be nullptr", connectFd, len);
        return -1;
    }

    ssize_t rLen = TEMP_FAILURE_RETRY(send(connectFd, buf, len, 0));
    if (rLen < 0) {
        HiLog::Error(LABEL, "Send message from fd %d error %zd: %s", connectFd, rLen, strerror(errno));
        return (-errno);
    }

    return rLen;
}

int LmksServer::RecvSocketMessage(int connectFd, void *buf, int len)
{
    if (connectFd < 0 || len <= 0 || buf == nullptr) {
        HiLog::Error(LABEL, "Invalid args: connect %d, len %d, buf might be nullptr", connectFd, len);
        return -1;
    }

    if (memset_s(buf, len, 0, len) != 0) {
        HiLog::Error(LABEL, "Failed to memset read buf err %{public}s", strerror(errno));
        return (-errno);
    }

    ssize_t rLen = TEMP_FAILURE_RETRY(recv(connectFd, buf, len, 0));
    if (rLen < 0) {
        HiLog::Error(LABEL, "Read message from fd %d error %zd: %s", connectFd, rLen, strerror(errno));
        return (-errno);
    }

    return rLen;
}

void LmksServer::ProcessMessage(int connectFd, LMKS_PACKET cmds, int len)
{
    if (!isStart_ || (lmksUtils_ == nullptr)) {
        HiLog::Error(
            LABEL, "Lmks server not start isStart_ %{public}d lmksUtils_%{public}p", isStart_, lmksUtils_.get());
        close(connectFd);
        return;
    }

    if (connectFd < 0 || len <= 0) {
        HiLog::Error(LABEL, "Invalid args: len %d, connectFd %d", len, connectFd);
        close(connectFd);
        return;
    }

    int ret = -1;
    pid_t pid = 0;

    switch (cmds[0]) {
        case LMKS_CMD_TARGET:
            HiLog::Info(LABEL, "ProcessMessage LMKS_CMD_TARGET ");
            break;
        case LMKS_CMD_PROCPRIO:
            HiLog::Info(LABEL, "ProcessMessage LMKS_CMD_PROCPRIO");
            break;
        case LMKS_CMD_PROCREMOVE:
            HiLog::Info(LABEL, "ProcessMessage LMKS_CMD_PROCREMOVE");
            pid = cmds[1];
            ret = lmksUtils_->RemoveProcess(pid);
            if (SendSocketMessage(connectFd, &ret, sizeof(ret)) <= 0) {
                HiLog::Error(LABEL, "Failed to return the result of remove process");
            }
            break;
        case LMKS_CMD_PROCPURGE:
            HiLog::Info(LABEL, "ProcessMessage LMKS_CMD_PROCPURGE");
            break;
        default:
            HiLog::Error(LABEL, "Wrong cmd %d", cmds[0]);
            break;
    }

    // close connect fd.
    close(connectFd);
}
}  // namespace LMKS
}  // namespace OHOS
