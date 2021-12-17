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
#include "worker_pool.h"

#include <memory>
#include "default_thread_factory.h"

namespace OHOS {
namespace AppExecFwk {
const int WorkerPool::THREAD_UPPER_LIMIT = 256;
const int WorkerPool::MAX_THREAD_LOWER_LIMIT = 1;
const int WorkerPool::CORE_THREAD_LOWER_LIMIT = 0;
const int WorkerPool::COUNT_BITS = sizeof(int) * __CHAR_BIT__ - 3;
const unsigned int WorkerPool::CAPACITY = (1 << COUNT_BITS) - 1;
const int WorkerPool::RUNNING = (-(1 << COUNT_BITS));
const int WorkerPool::CLOSING = (0 << COUNT_BITS);
const int WorkerPool::INTERRUPT = (1 << COUNT_BITS);
const int WorkerPool::CLEANED = (2 << COUNT_BITS);
const int WorkerPool::CLOSED = (3 << COUNT_BITS);

WorkerPool::WorkerPool(const std::shared_ptr<WorkerPoolConfig> &config)
{
    control_ = CombineToControl(RUNNING, 0);
    pool_.clear();
    exitPool_.clear();
    WorkerPool::factory_ = std::make_shared<DefaultThreadFactory>();
    initFlag_.store(Init(config));
    stop_.store(false);
}

WorkerPool::~WorkerPool()
{
    control_ = 0;

    APP_LOGI("WorkerPool::~WorkerPool");
}

bool WorkerPool::Init(const std::shared_ptr<WorkerPoolConfig> &config)
{
    if (CheckConfigParams(config) == false) {
        APP_LOGE("WorkerPool::checkConfigParams  parameters are illegal");
        return false;
    }

    thread_limit_ = config->GetMaxThreadCount();
    core_thread_limit_ = config->GetCoreThreadCount();
    long keepAliveTime = config->GetKeepAliveTime();
    alive_time_Limit_ = keepAliveTime > 0 ? keepAliveTime : 0;

    return true;
}

bool WorkerPool::CheckConfigParams(const std::shared_ptr<WorkerPoolConfig> &config)
{
    if (config == nullptr) {
        APP_LOGE("WorkerPool::CheckConfigParams config is nullptr");
        return false;
    }

    int maxThreadCount = config->GetMaxThreadCount();
    int coreThreadCount = config->GetCoreThreadCount();
    if (!CheckThreadCount(maxThreadCount, coreThreadCount)) {
        APP_LOGE("WorkerPool::CheckConfigParams parameters are illegal, maxThreadCount %{public}d is less than "
                 "coreThreadCount %{public}d",
            maxThreadCount,
            coreThreadCount);
        return false;
    }
    if (!CheckMaxThreadCount(maxThreadCount)) {
        APP_LOGE("WorkerPool::CheckConfigParams maxThreadCount %{public}d is illegal", maxThreadCount);
        return false;
    }
    if (!CheckCoreThreadCount(coreThreadCount)) {
        APP_LOGE("WorkerPool::CheckConfigParams coreThreadCount %{public}d is illegal", coreThreadCount);
        return false;
    }
    return true;
}

bool WorkerPool::CheckThreadCount(int maxThreadCount, int coreThreadCount)
{
    return maxThreadCount >= coreThreadCount;
}

bool WorkerPool::CheckMaxThreadCount(int maxThreadCount)
{
    if ((maxThreadCount > THREAD_UPPER_LIMIT) || (maxThreadCount < MAX_THREAD_LOWER_LIMIT)) {
        return false;
    }
    return true;
}

bool WorkerPool::CheckCoreThreadCount(int coreThreadCount)
{
    if ((coreThreadCount > THREAD_UPPER_LIMIT) || (coreThreadCount < CORE_THREAD_LOWER_LIMIT)) {
        return false;
    }
    return true;
}

long WorkerPool::GetKeepAliveTime(void) const
{
    return alive_time_Limit_;
}

int WorkerPool::GetCoreThreadCount(void) const
{
    return core_thread_limit_;
}

int WorkerPool::GetMaxThreadCount(void) const
{
    return thread_limit_;
}

int WorkerPool::GetWorkCount(void) const
{
    unsigned int value = control_.load();
    return GetWorkingThreadNum(value);
}

std::map<std::string, long> WorkerPool::GetWorkerThreadsInfo(void)
{
    std::unique_lock<std::mutex> mLock(poolLock_);
    std::map<std::string, long> workerThreadsInfo;

    for (auto it = pool_.begin(); it != pool_.end(); it++) {
        if ((*it) != nullptr) {
            workerThreadsInfo.emplace((*it)->GetThreadName(), (*it)->GetTaskCounter());
        }
    }
    return workerThreadsInfo;
}

void WorkerPool::ClosePool(bool interrupt)
{
    APP_LOGI("WorkerPool::ClosePool begin interrupt=%{public}d", interrupt);
    std::unique_lock<std::mutex> mLock(poolLock_);

    AdvanceStateTo(CLOSING);
    InterruptWorkers();

    APP_LOGI("WorkerPool::ClosePool end");
}

void WorkerPool::InterruptWorkers(void)
{
    APP_LOGI("WorkerPool::InterruptWorkers begin");
    if (guardThread_ == nullptr) {
        APP_LOGE("WorkerPool::InterruptWorkers guardThread is nullptr");
        return;
    }
    poolLock_.unlock();

    {
        std::unique_lock<std::mutex> lock(exitPoolLock_);
        stop_.store(true);
        exit_.notify_all();
    }

    {
        std::unique_lock<std::mutex> lock(exitPoolLock_);
        exitGuard_.wait(lock);
        if (guardThread_->joinable()) {
            APP_LOGI("WorkerPool::InterruptWorkers guardThread_ joinable");
            guardThread_->join();
            // Prevent manual call again
            guardThread_ = nullptr;
        }
    }

    APP_LOGI("WorkerPool::InterruptWorkers end");
}

void WorkerPool::CreateGuardThread()
{
    APP_LOGI("WorkerPool::CreateGuardThread START");
    if (guardThread_ != nullptr) {
        APP_LOGW("WorkerPool::CreateGuardThread guardThread_ is not nullptr");
        return;
    }
    auto guardTask = [&]() {
        while (true) {
            {
                std::unique_lock<std::mutex> lock(exitPoolLock_);
                if (!exitPool_.empty()) {
                    exitPool_.front()->Join();
                    exitPool_.erase(exitPool_.begin());
                } else {
                    exit_.wait(lock, [this] {
                        return this->stop_.load() || !this->exitPool_.empty();
                    });  // return 防止先notify 后wait
                }
            }
            if (stop_.load() && exitPool_.empty() && pool_.empty()) {
                exitGuard_.notify_all();
                APP_LOGI("WorkerPool::CreateGuardThread break while");
                break;
            }
        }
        APP_LOGI("WorkerPool::CreateGuardThread STOP");
    };

    guardThread_ = std::make_shared<std::thread>(guardTask);
}

bool WorkerPool::AddWorker(const std::shared_ptr<Delegate> &delegate, const std::shared_ptr<Task> &task)
{
    bool added = false;
    if (!initFlag_.load()) {
        APP_LOGE("WorkerPool::AddWorker workPool init failed");
        return added;
    }
    if (factory_ == nullptr) {
        APP_LOGE("WorkerPool::AddWorker factory_ is nullptr");
        return added;
    }
    if (task == nullptr) {
        APP_LOGE("WorkerPool::AddWorker task is nullptr");
        return added;
    }
    if (delegate == nullptr) {
        APP_LOGE("WorkerPool::AddWorker delegate is nullptr");
        return added;
    }
    std::unique_lock<std::mutex> mLock(poolLock_);
    std::shared_ptr<WorkerThread> newThread = nullptr;

    for(;;){
        unsigned int value = control_.load();
        int num = GetWorkingThreadNum(value);
        if (num >= thread_limit_) {
            APP_LOGI("WorkerPool::AddWorker thread count exceed limits, num=%{public}d, limits=%{public}d",
                num,
                thread_limit_);
            break;
        }
        if (!IsRunning(value)) {
            APP_LOGI("WorkerPool::AddWorker thread pool is not running. value=%{public}d, closing=%{public}d, "
                     "count_bits=%{public}d",
                value,
                CLOSING,
                COUNT_BITS);
            break;
        }

        if (CompareAndIncThreadNum(num)) {
            newThread = std::make_shared<WorkerThread>(delegate, task, factory_);
            if (newThread == nullptr) {
                APP_LOGE("WorkerPool::AddWorker create thread fail");
                break;
            }

            newThread->CreateThread();

            APP_LOGI("WorkerPool::AddWorker create new thread");

            pool_.emplace_back(newThread);
            APP_LOGI("WorkerPool::AddWorker pool_ add thread ,POOL SIZE: %{public}zu", pool_.size());

            added = true;
            break;
        }

        APP_LOGW("WorkerPool::AddWorker set thread state error. retry. ");
    }
    return added;
}

void WorkerPool::OnWorkerExit(const std::shared_ptr<WorkerThread> &worker, bool isInterrupted)
{
    std::unique_lock<std::mutex> mLock(poolLock_);
    APP_LOGI("WorkerPool::OnWorkerExit start, pool size: %{public}zu", pool_.size());
    for (auto it = pool_.begin(); it != pool_.end(); it++) {
        if ((*it).get() == worker.get()) {
            APP_LOGI("WorkerPool::OnWorkerExit erase current, size=%{public}zu, threads=%{public}d",
                pool_.size(),
                GetWorkingThreadNum(control_.load()));
            {
                std::unique_lock<std::mutex> lock(exitPoolLock_);
                exitPool_.emplace_back(worker);
                APP_LOGI("WorkerPool::OnWorkerExit exit notify all");
                exit_.notify_all();
            }
            pool_.erase(it);

            break;
        }
    }
    APP_LOGI("WorkerPool::OnWorkerExit end");
}

void WorkerPool::AfterRun(const std::shared_ptr<Task> &task)
{}

void WorkerPool::BeforeRun(const std::shared_ptr<Task> &task)
{}

unsigned int WorkerPool::GetWorkingThreadNum(unsigned int ctl)
{
    return ctl & CAPACITY;
}

bool WorkerPool::IsRunning(int ctl)
{
    return ctl < CLOSING;
}

int WorkerPool::GetStateFromControl(unsigned int ctl)
{
    return ctl & ~CAPACITY;
}

void WorkerPool::AdvanceStateTo(unsigned int target)
{
    APP_LOGI("WorkerPool::AdvanceStateTo begin");
    for(;;){
        unsigned int current = control_.load();
        if ((current >= target) ||
            CompareAndSet(control_, current, CombineToControl(target, GetWorkingThreadNum(current)))) {
            APP_LOGI("WorkerPool::AdvanceStateTo break");
            break;
        }
    }
    APP_LOGI("WorkerPool::AdvanceStateTo end");
}

int WorkerPool::CombineToControl(unsigned int state, unsigned int count)
{
    return state | count;
}

bool WorkerPool::CompareAndIncThreadNum(int expect)
{
    unsigned int ctl = control_.load();
    int state = GetStateFromControl(ctl);
    return CompareAndSet(control_, ctl, CombineToControl(state, expect + 1));
}

void WorkerPool::DecrementThread(void)
{
    APP_LOGI("WorkerPool::DecrementThread begin");
    int curr = control_.load();
    while (!CompareAndDecThreadNum(curr)) {
        curr = control_.load();
    }
    APP_LOGI("WorkerPool::DecrementThread end");
}

bool WorkerPool::CompareAndDecThreadNum(int expect)
{
    return CompareAndSet(control_, expect, expect - 1);
}

bool WorkerPool::CompareAndDecNum(int expectCount)
{
    unsigned int curr = control_.load();
    int state = GetStateFromControl(curr);
    int expectControl = CombineToControl(state, expectCount);
    return CompareAndDecThreadNum(expectControl);
}

bool WorkerPool::CompareAndSet(std::atomic<int> &atomicInt, int expect, int desire)
{
    return atomicInt.compare_exchange_strong(expect, desire);
}
}  // namespace AppExecFwk
}  // namespace OHOS