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
#ifndef OHOS_APPEXECFWK_WORKER_POOL_H
#define OHOS_APPEXECFWK_WORKER_POOL_H

#include <atomic>
#include <map>
#include <memory>
#include <mutex>
#include <string>
#include <vector>

#include "app_log_wrapper.h"
#include "runnable.h"
#include "task.h"
#include "thread_factory.h"
#include "worker_pool_config.h"
#include "work_thread.h"

namespace OHOS {
namespace AppExecFwk {
class WorkerPool {
public:
    WorkerPool(const std::shared_ptr<WorkerPoolConfig> &config);
    virtual ~WorkerPool();

    long GetKeepAliveTime(void) const;

    int GetCoreThreadCount(void) const;

    int GetMaxThreadCount(void) const;

    int GetWorkCount(void) const;

    std::map<std::string, long> GetWorkerThreadsInfo(void);

    bool AddWorker(const std::shared_ptr<Delegate> &delegate, const std::shared_ptr<Task> &task);

    void CreateGuardThread();

protected:
    void ClosePool(bool interrupt);

    void OnWorkerExit(const std::shared_ptr<WorkerThread> &worker, bool isInterrupted);

    virtual void AfterRun(const std::shared_ptr<Task> &task);

    virtual void BeforeRun(const std::shared_ptr<Task> &task);

    void DecrementThread(void);

    bool CompareAndDecNum(int expectCount);

private:
    bool Init(const std::shared_ptr<WorkerPoolConfig> &config);

    bool CheckConfigParams(const std::shared_ptr<WorkerPoolConfig> &config);

    bool CheckThreadCount(int maxThreadCount, int coreThreadCount);

    bool CheckMaxThreadCount(int maxThreadCount);

    bool CheckCoreThreadCount(int coreThreadCount);

    void InterruptWorkers(void);

    static unsigned int GetWorkingThreadNum(unsigned int ctl);

    static bool IsRunning(int ctl);

    static int GetStateFromControl(unsigned int ctl);

    static int CombineToControl(unsigned int state, unsigned int count);

    void AdvanceStateTo(unsigned int target);

    bool CompareAndIncThreadNum(int expect);

    bool CompareAndDecThreadNum(int expect);

    bool CompareAndSet(std::atomic<int> &atomicInt, int left, int right);

private:
    static const int THREAD_UPPER_LIMIT;
    static const int MAX_THREAD_LOWER_LIMIT;
    static const int CORE_THREAD_LOWER_LIMIT;
    static const int COUNT_BITS;
    static const unsigned int CAPACITY;
    static const int RUNNING;
    static const int CLOSING;
    static const int INTERRUPT;
    static const int CLEANED;
    static const int CLOSED;

    int thread_limit_ = 0;
    int core_thread_limit_ = 0;
    long alive_time_Limit_ = 0;

    std::vector<std::shared_ptr<WorkerThread>> pool_;
    std::atomic<int> control_;
    std::mutex poolLock_;
    std::shared_ptr<ThreadFactory> factory_;
    std::atomic<bool> initFlag_;
    std::vector<std::shared_ptr<WorkerThread>> exitPool_;
    std::atomic<bool> stop_;
    std::shared_ptr<std::thread> guardThread_;
    std::mutex exitPoolLock_;
    std::condition_variable exit_;

    std::mutex exitGuardLock_;
    std::condition_variable exitGuard_;
};
}  // namespace AppExecFwk
}  // namespace OHOS
#endif