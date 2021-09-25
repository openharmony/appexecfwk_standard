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

#include "lmks_utils.h"

#include <csignal>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>

#include "securec.h"
#include "hilog/log.h"

namespace OHOS {
namespace LMKS {
namespace {
constexpr int PROC_PATH_MAX = 256;
constexpr int PROC_LINE_MAX = 128;
}  // namespace

using namespace OHOS::HiviewDFX;
static constexpr HiLogLabel LABEL = {LOG_CORE, 0, "LmksUtils"};

LmksUtils::LmksUtils()
{}

LmksUtils::~LmksUtils()
{}

int LmksUtils::RemoveProcess(pid_t pid)
{
    auto procName = GetProcName(pid);
    if (procName.empty()) {
        HiLog::Error(LABEL, "pid %{public}d process name emptry", pid);
        return -1;
    }

    auto procSize = GetProcSize(pid);
    if (procSize <= 0) {
        HiLog::Error(LABEL, "pid %{public}d process size error", pid);
        return -1;
    }

    // kill process
    int ret = kill(pid, SIGKILL);
    if (ret) {
        HiLog::Warn(LABEL, "kill pid %{public}d err %{public}s", pid, strerror(errno));
        return (-errno);
    } else {
        HiLog::Info(
            LABEL, "kill pid %{public}d success, name %{public}s size %{public}d", pid, procName.c_str(), procSize);
    }

    return 0;
}

std::string LmksUtils::GetProcName(pid_t pid)
{
    std::string name = "";

    if (pid < 1) {
        HiLog::Warn(LABEL, "invalid pid %{public}d.", pid);
        return name;
    }

    char path[PROC_PATH_MAX];
    char line[PROC_LINE_MAX];
    int fd = -1;
    ssize_t ret = -1;

    if (memset_s(path, sizeof(path), 0x00, sizeof(path)) != 0) {
        HiLog::Error(LABEL, "memset_s path err %{public}s", strerror(errno));
        return name;
    }

    if (memset_s(line, sizeof(line), 0x00, sizeof(line)) != 0) {
        HiLog::Error(LABEL, "memset_s line err %{public}s", strerror(errno));
        return name;
    }

    if (snprintf_s(path, PROC_PATH_MAX, PROC_PATH_MAX - 1, "/proc/%d/cmdline", pid) < 0) {
        HiLog::Error(LABEL, "snprintf_s cmdline err %{public}s", strerror(errno));
        return name;
    }

    fd = open(path, O_RDONLY | O_CLOEXEC);
    if (fd == -1) {
        HiLog::Error(LABEL, "open path[%{public}s] err %{public}s", path, strerror(errno));
        return name;
    }

    ret = ReadAll(fd, line, sizeof(line) - 1);
    if (ret < 0) {
        close(fd);
        return name;
    }

    if (strlen(line) + 1 <= PROC_LINE_MAX && strlen(line) != 0 && line[strlen(line)] == '\0') {
        name = line;
    } else {
        HiLog::Error(LABEL, "cmdline no data");
    }

    close(fd);
    return name;
}

int LmksUtils::GetProcSize(pid_t pid)
{
    int rss = 0;

    if (pid <= 0) {
        HiLog::Error(LABEL, "invalid pid %{public}d.", pid);
        return rss;
    }

    char path[PROC_PATH_MAX];
    char line[PROC_LINE_MAX];
    int fd = -1;
    int total = -1;
    ssize_t ret = -1;

    if (memset_s(path, sizeof(path), 0x00, sizeof(path)) != 0) {
        HiLog::Error(LABEL, "memset_s path err %{public}s", strerror(errno));
        return -1;
    }

    if (memset_s(line, sizeof(line), 0x00, sizeof(line)) != 0) {
        HiLog::Error(LABEL, "memset_s line err %{public}s", strerror(errno));
        return -1;
    }

    if (snprintf_s(path, PROC_PATH_MAX, PROC_PATH_MAX - 1, "/proc/%d/statm", pid) < 0) {
        HiLog::Error(LABEL, "snprintf_s statm err %{public}s", strerror(errno));
        return rss;
    }

    fd = open(path, O_RDONLY | O_CLOEXEC);
    if (fd == -1) {
        HiLog::Error(LABEL, "open path[%{public}s] err %{public}s", path, strerror(errno));
        return -1;
    }

    ret = ReadAll(fd, line, sizeof(line) - 1);
    if (ret < 0) {
        close(fd);
        return -1;
    }

    if ((strlen(line) + 1 <= PROC_LINE_MAX && strlen(line) != 0) && (sscanf_s(line, "%d %d ", &total, &rss) > 0)) {
        HiLog::Info(LABEL, "pid %{public}d total %{public}d rss %{public}d", pid, total, rss);
    } else {
        HiLog::Error(LABEL, "strlen or sscanf_s err %{public}s", strerror(errno));
        rss = 0;
    }

    close(fd);
    return rss;
}

ssize_t LmksUtils::ReadAll(int fd, char *buf, size_t maxLen)
{
    ssize_t ret = 0;
    off_t offSet = 0;

    while (maxLen > 0) {
        ssize_t rc = TEMP_FAILURE_RETRY(pread(fd, buf, maxLen, offSet));
        if (rc == 0) {
            break;
        }
        if (rc == -1) {
            HiLog::Error(LABEL, "pread err %{public}s", strerror(errno));
            return -1;
        }
        ret += rc;
        buf += rc;
        offSet += rc;
        maxLen -= rc;
    }

    return ret;
}
}  // namespace LMKS
}  // namespace OHOS
