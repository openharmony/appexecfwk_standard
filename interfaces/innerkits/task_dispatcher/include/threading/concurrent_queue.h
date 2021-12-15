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

#ifndef OHOS_APPEXECFWK_CONCURRENT_QUEUE_H
#define OHOS_APPEXECFWK_CONCURRENT_QUEUE_H

#include <condition_variable>
#include <queue>
#include <vector>
#include "app_log_wrapper.h"

namespace OHOS {
namespace AppExecFwk {
template<typename T>
class ConcurrentQueue {
public:
    ConcurrentQueue() : empty_(){};
    ~ConcurrentQueue() = default;

    /**
     * get data without block
     * @param task
     * @return true
     */
    bool Offer(const T &task)
    {
        std::unique_lock<std::mutex> lock(mutex_);
        queue_.push_back(task);
        empty_.notify_all();
        return true;
    }

    /**
     * get data with block
     * @param task
     * @return data
     */
    T Take()
    {
        std::unique_lock<std::mutex> lock(mutex_);
        while (queue_.empty()) {
            APP_LOGI("ConcurrentQueue::Take blocked");
            empty_.wait(lock);
        }

        T front(queue_.front());
        queue_.pop_front();
        return front;
    }
    T Poll()
    {
        std::unique_lock<std::mutex> lock(mutex_);
        if (queue_.empty()) {
            APP_LOGI("ConcurrentQueue::Poll empty");
            return nullptr;
        }
        T front(queue_.front());
        queue_.pop_front();
        return front;
    }

    size_t Size()
    {
        return queue_.size();
    }
    size_t Empty()
    {
        return queue_.empty();
    }
    auto Begin()
    {
        return queue_.begin();
    }

    auto End()
    {
        return queue_.end();
    }
    void clear()
    {
        std::unique_lock<std::mutex> lock(mutex_);
        queue_.clear();
    }

    ConcurrentQueue(const ConcurrentQueue &) = delete;
    ConcurrentQueue &operator=(const ConcurrentQueue &) = delete;

private:
    mutable std::mutex mutex_;
    std::condition_variable empty_;
    std::deque<T> queue_;
};
}  // namespace AppExecFwk
}  // namespace OHOS

#endif
