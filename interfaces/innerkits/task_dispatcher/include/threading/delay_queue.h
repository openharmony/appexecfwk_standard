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
#ifndef OHOS_APPEXECFWK_DELAY_QUEUE_H
#define OHOS_APPEXECFWK_DELAY_QUEUE_H

#include <atomic>
#include <condition_variable>
#include <deque>
#include <functional>
#include <mutex>
#include <queue>

#include "app_log_wrapper.h"

namespace OHOS {
namespace AppExecFwk {
struct DelayTaskWrapper {
    DelayTaskWrapper(long delayMs, std::function<void()> runnable) : runnable_(std::move(runnable))
    {
        auto now = std::chrono::high_resolution_clock::now();
        startTime_ = now + std::chrono::milliseconds(delayMs);
    }
    DelayTaskWrapper() = delete;
    ~DelayTaskWrapper(){};

    std::chrono::high_resolution_clock::time_point startTime_;
    std::function<void()> runnable_;
};

struct CompareTaskDelayMs {
    bool operator()(
        const std::shared_ptr<DelayTaskWrapper> &other1, const std::shared_ptr<DelayTaskWrapper> &other2) const
    {
        return other1->startTime_ > other2->startTime_;
    }
};

class DelayQueue {
public:
    DelayQueue() : stopFlag_(false){};
    ~DelayQueue(){};

    bool Offer(const std::shared_ptr<DelayTaskWrapper> &task)
    {
        std::unique_lock<std::mutex> lock(mutex_);
        taskQueue_.push(task);
        emptyWait_.notify_all();
        return true;
    }

    std::shared_ptr<DelayTaskWrapper> Take()
    {
        while (true) {
            std::unique_lock<std::mutex> lock(mutex_);
            while (taskQueue_.empty() && !stopFlag_) {
                APP_LOGD("DelayQueue::taskQueue_ is empty");
                emptyWait_.wait(lock);
            }

            if (taskQueue_.empty() && stopFlag_) {
                return nullptr;
            }

            std::shared_ptr<DelayTaskWrapper> front = taskQueue_.top();
            auto now = std::chrono::high_resolution_clock::now();
            while (now < front->startTime_) {
                emptyWait_.wait_until(lock, front->startTime_);

                now = std::chrono::high_resolution_clock::now();
                front = taskQueue_.top();
            }
            std::shared_ptr<DelayTaskWrapper> check = taskQueue_.top();
            if (check->startTime_ == front->startTime_) {
                taskQueue_.pop();
                return front;
            }
        }
    }

    size_t Size()
    {
        std::unique_lock<std::mutex> lock(mutex_);
        return taskQueue_.size();
    }

    bool Empty()
    {
        std::unique_lock<std::mutex> lock(mutex_);
        return taskQueue_.empty();
    }

    void Stop()
    {
        std::unique_lock<std::mutex> lock(mutex_);
        stopFlag_.store(true);
        emptyWait_.notify_all();
    }

    DelayQueue(const DelayQueue &) = delete;
    DelayQueue &operator=(const DelayQueue &) = delete;

private:
    std::mutex mutex_;
    std::condition_variable emptyWait_;
    std::priority_queue<std::shared_ptr<DelayTaskWrapper>, std::vector<std::shared_ptr<DelayTaskWrapper>>,
        CompareTaskDelayMs>
        taskQueue_;
    std::atomic<bool> stopFlag_;
};
}  // namespace AppExecFwk
}  // namespace OHOS

#endif
