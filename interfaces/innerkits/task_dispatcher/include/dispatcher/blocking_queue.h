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
#ifndef OHOS_APPEXECFWK_BLOCKING_QUEUE_H
#define OHOS_APPEXECFWK_BLOCKING_QUEUE_H

#include <atomic>
#include <condition_variable>
#include <deque>
#include <functional>
#include <mutex>
#include <queue>

#include "app_log_wrapper.h"
#include "task.h"
#include "task_priority.h"

namespace OHOS {
namespace AppExecFwk {
struct PriorityTaskWrapper {
    PriorityTaskWrapper(const TaskPriority &priority, std::shared_ptr<Task> task) : task_(task)
    {
        switch (priority) {
            case TaskPriority::HIGH:
                priorityWeight_ = HIGH_PRIORITY_WEIGHT;
                break;
            case TaskPriority::DEFAULT:
                priorityWeight_ = DEFAULT_PRIORITY_WEIGHT;
                break;
            case TaskPriority::LOW:
                priorityWeight_ = LOW_PRIORITY_WEIGHT;
                break;
            default:
                priorityWeight_ = DEFAULT_PRIORITY_WEIGHT;
                break;
        }
    }
    PriorityTaskWrapper() = delete;
    ~PriorityTaskWrapper(){};

    int priorityWeight_;
    std::shared_ptr<Task> task_;
    static constexpr int HIGH_PRIORITY_WEIGHT = 2;
    static constexpr int DEFAULT_PRIORITY_WEIGHT = 1;
    static constexpr int LOW_PRIORITY_WEIGHT = 0;
};
struct CompareTaskPriority {
    bool operator()(const std::shared_ptr<PriorityTaskWrapper> &wrapper1,
        const std::shared_ptr<PriorityTaskWrapper> &wrapper2) const
    {
        return wrapper1->priorityWeight_ < wrapper2->priorityWeight_;
    }
};

class BlockingQueue {
public:
    BlockingQueue() : mutex_(), empty_(), stopFlag_(false){};
    ~BlockingQueue(){};

    bool Offer(const std::shared_ptr<PriorityTaskWrapper> &task)
    {
        std::unique_lock<std::mutex> lock(mutex_);
        queue_.push(task);
        empty_.notify_all();
        return true;
    }

    std::shared_ptr<PriorityTaskWrapper> Take()
    {
        std::unique_lock<std::mutex> lock(mutex_);
        while (queue_.empty() && !stopFlag_) {
            APP_LOGI("BlockingQueue::Take empty_wait");
            empty_.wait(lock);
        }

        std::shared_ptr<PriorityTaskWrapper> front = nullptr;
        if (!queue_.empty()) {
            front = queue_.top();
            queue_.pop();
        }
        return front;
    }

    std::shared_ptr<PriorityTaskWrapper> Poll(long deleyedMs)
    {
        auto const timeout = std::chrono::steady_clock::now() + std::chrono::milliseconds(deleyedMs);
        std::unique_lock<std::mutex> lock(mutex_);
        while (queue_.empty() && !stopFlag_) {
            if (empty_.wait_until(lock, timeout) == std::cv_status::timeout) {
                APP_LOGI("BlockingQueue::Poll timeout");
                break;
            }
        }
        std::shared_ptr<PriorityTaskWrapper> front = nullptr;
        if (!queue_.empty()) {
            front = queue_.top();
            queue_.pop();
        }
        return front;
    }

    size_t Size()
    {
        std::lock_guard<std::mutex> lock(mutex_);
        return queue_.size();
    }

    bool Empty()
    {
        std::unique_lock<std::mutex> lock(mutex_);
        return queue_.empty();
    }

    void Stop()
    {
        std::unique_lock<std::mutex> lock(mutex_);
        stopFlag_.store(true);
        empty_.notify_all();
    }

    BlockingQueue(const BlockingQueue &) = delete;
    BlockingQueue &operator=(const BlockingQueue &) = delete;

private:
    std::mutex mutex_;
    std::condition_variable empty_;
    std::priority_queue<std::shared_ptr<PriorityTaskWrapper>, std::deque<std::shared_ptr<PriorityTaskWrapper>>,
        CompareTaskPriority>
        queue_;
    std::atomic<bool> stopFlag_;
};
}  // namespace AppExecFwk
}  // namespace OHOS

#endif
