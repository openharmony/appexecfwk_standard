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

#include "lmks_client.h"

#include <cerrno>
#include <memory>
#include <string>
#include <sys/socket.h>
#include <sys/time.h>
#include <sys/un.h>
#include <thread>
#include <unistd.h>

#include "app_log_wrapper.h"
#include "event_handler.h"
#include "securec.h"
#include "unique_fd.h"

namespace OHOS {
namespace AppExecFwk {
namespace {
constexpr int LMKS_SOCKET_TIMEOUT = 3;

constexpr size_t LMKS_START_VALUE = 1;
constexpr size_t LMKS_MULTIPLE = 2;
constexpr size_t LMKS_MAX_TARGETS = 6;

constexpr int LMKS_CMD_TARGET = 0;
constexpr int LMKS_CMD_PROCPRIO = 1;
constexpr int LMKS_CMD_PROCREMOVE = 2;
constexpr int LMKS_CMD_PROCPURGE = 3;

constexpr int APP_OOM_ADJ_MIN = -1000;
constexpr int APP_OOM_ADJ_MAX = 1000;
constexpr int LMKS_SOCKET_PATH_MAX = 108;

constexpr std::string_view LMKS_SOCKET_PATH("/dev/socket/lmks");
static_assert(LMKS_SOCKET_PATH.size() < LMKS_SOCKET_PATH_MAX);
}  // namespace

LmksClient::LmksClient() : socket_(-1)
{}

LmksClient::~LmksClient()
{
    if (IsOpen()) {
        Close();
    }
}

int32_t LmksClient::Open()
{
    APP_LOGI("connecting lmks.");

    if (socket_ >= 0) {
        APP_LOGE("already connected.");
        return -1;
    }

    UniqueFd sk(socket(PF_LOCAL, SOCK_SEQPACKET, 0));
    if (sk.Get() < 0) {
        APP_LOGE("failed to create local socket %{public}d.", errno);
        return (-errno);
    }

    struct timeval timeOut = {.tv_sec = LMKS_SOCKET_TIMEOUT, .tv_usec = 0};
    int fd = sk.Get();
    if (fd < 0) {
        APP_LOGE("fd is negative.");
        return -1;
    }
    if ((setsockopt(fd, SOL_SOCKET, SO_RCVTIMEO, &timeOut, sizeof(timeOut)) != 0) ||
        (setsockopt(fd, SOL_SOCKET, SO_SNDTIMEO, &timeOut, sizeof(timeOut)) != 0)) {
        APP_LOGE("failed to set local socket timeout %{public}s.", strerror(errno));
        return (-errno);
    }

    struct sockaddr_un addr;
    int32_t ret;
    ret = memset_s(&addr, sizeof(addr), 0, sizeof(addr));
    if (ret != EOK) {
        APP_LOGE("failed to clear local socket addr.");
        return ret;
    }

    ret = memcpy_s(addr.sun_path, LMKS_SOCKET_PATH_MAX, LMKS_SOCKET_PATH.data(), LMKS_SOCKET_PATH.size());
    if (ret != EOK) {
        APP_LOGE("failed to make local socket path.");
        return ret;
    }

    addr.sun_family = AF_LOCAL;
    socklen_t addrLen = offsetof(struct sockaddr_un, sun_path) + LMKS_SOCKET_PATH.size() + 1;
    if (connect(sk, reinterpret_cast<struct sockaddr *>(&addr), addrLen) < 0) {
        APP_LOGE("failed to connect to lmks %{public}s.", strerror(errno));
        return (-errno);
    }

    socket_ = sk.Release();

    return ERR_OK;
}

void LmksClient::Close()
{
    APP_LOGI("closing lmks.");

    if (socket_ < 0) {
        APP_LOGE("not connected.");
        return;
    }

    close(socket_);
    socket_ = -1;
}

bool LmksClient::IsOpen() const
{
    return socket_ >= 0;
}

int32_t LmksClient::Target(const Targets &targets)
{
    APP_LOGI("Target enter");

    if (targets.empty() || targets.size() > LMKS_MAX_TARGETS) {
        APP_LOGE("empty target or too many targets. %{public}zu", targets.size());
        return (-EINVAL);
    }

    int i = 0;
    int32_t buf[LMKS_START_VALUE + LMKS_MULTIPLE * LMKS_MAX_TARGETS];
    buf[i++] = LMKS_CMD_TARGET;

    for (auto target : targets) {
        if (target.first < 0 || !CheckOomAdj(target.second)) {
            APP_LOGE("invalid target: %{public}d %{public}d", target.first, target.second);
            return (-EINVAL);
        }
        buf[i++] = target.first;
        buf[i++] = target.second;
    }

    return Write(buf, i * sizeof(int32_t)) ? ERR_OK : -1;
}

int32_t LmksClient::ProcPrio(pid_t pid, uid_t uid, int oomAdj)
{
    APP_LOGI("ProcPrio enter");

    if (pid < 0 || uid < 0 || !CheckOomAdj(oomAdj)) {
        APP_LOGE("invalid parameter: %{public}d %{public}d %{public}d.", pid, uid, oomAdj);
        return (-EINVAL);
    }

    int32_t buf[4] = {LMKS_CMD_PROCPRIO, pid, uid, oomAdj};

    return Write(buf, sizeof(buf)) ? ERR_OK : -1;
}

int32_t LmksClient::ProcRemove(pid_t pid)
{
    APP_LOGI("ProcRemove enter");

    if (pid < 1) {
        APP_LOGE("invalid pid %{public}d.", pid);
        return (-EINVAL);
    }

    int32_t buf[2] = {LMKS_CMD_PROCREMOVE, pid};
    if (!Write(buf, sizeof(buf))) {
        APP_LOGE("failed to write");
        return -1;
    }

    LmksClientMsg msg;
    if (!Read(msg.resultBuf, sizeof(int32_t))) {
        APP_LOGE("failed to read");
        return -1;
    }

    if (msg.result != 0) {
        APP_LOGE("failed to remove process");
        return -1;
    }

    APP_LOGI("success to remove process");
    return 0;
}

bool LmksClient::ProcPurge()
{
    APP_LOGI("ProcPurge enter");

    int32_t cmd = LMKS_CMD_PROCPURGE;

    return Write(reinterpret_cast<void *>(&cmd), sizeof(cmd));
}

bool LmksClient::CheckOomAdj(int v)
{
    return (APP_OOM_ADJ_MIN <= v && v <= APP_OOM_ADJ_MAX);
}

bool LmksClient::Write(const void *buf, size_t len)
{
    if (buf == nullptr || len < 1) {
        APP_LOGE("invalid parameter.");
        return false;
    }

    constexpr int retryTimes = 5;
    for (int i = 0; i < retryTimes; ++i) {
        if (socket_ < 0 && !Open()) {
            std::this_thread::yield();
            continue;
        }
        int rLen = TEMP_FAILURE_RETRY(send(socket_, buf, len, 0));
        if (rLen <= 0) {
            APP_LOGE("failed to send data to lmks err %{public}s.", strerror(errno));
            Close();
            std::this_thread::yield();
        } else {
            return true;
        }
    }

    return false;
}

bool LmksClient::Read(void *buf, size_t len)
{
    if (buf == nullptr || len < 1) {
        APP_LOGE("invalid parameter. len %zu", len);
        return false;
    }

    constexpr int retryTimes = 5;
    for (int i = 0; i < retryTimes; ++i) {
        if (socket_ < 0 && !Open()) {
            std::this_thread::yield();
            continue;
        }
        int rLen = TEMP_FAILURE_RETRY(recv(socket_, buf, len, 0));
        if (rLen <= 0) {
            APP_LOGE("failed to recv data from lmks err %{public}s.", strerror(errno));
            Close();
            std::this_thread::yield();
        } else {
            return true;
        }
    }

    return false;
}
}  // namespace AppExecFwk
}  // namespace OHOS
