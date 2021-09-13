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

#include "work_thread.h"

namespace OHOS {
namespace AppExecFwk {
WorkerThread::WorkerThread(const std::shared_ptr<Delegate> &delegate, const std::shared_ptr<Task> &firstTask,
    const std::shared_ptr<ThreadFactory> &factory)
{
    task_counter_.store(0);

    first_task_ = firstTask;
    delegate_ = delegate;
    factory_ = factory;
}

void WorkerThread::Join()
{
    if ((thread_) && (thread_->thread_) && thread_->thread_->joinable()) {
        APP_LOGI("WorkerThread::Join joinable thread");
        thread_->thread_->join();
    }
    APP_LOGI("WorkerThread::Join end");
}

void WorkerThread::CreateThread()
{
    thread_ = factory_->Create();
    auto task = [&]() {
        if (delegate_ != nullptr) {
            auto ptr = shared_from_this();
            delegate_->DoWorks(ptr);
        };
    };

    // start a thread to run task function.
    thread_->thread_ = std::make_shared<std::thread>(task);
    APP_LOGI("WorkerThread::CreateThread start thread. ");
}

void WorkerThread::IncTaskCount()
{
    task_counter_.fetch_add(1, std::memory_order_relaxed);
}

std::string WorkerThread::GetThreadName()
{
    if (thread_ != nullptr) {
        return thread_->thread_name_;
    } else {
        return std::string("");
    }
}

std::shared_ptr<Task> WorkerThread::PollFirstTask(void)
{
    std::shared_ptr<Task> ret(nullptr);
    if (first_task_ != nullptr) {
        ret.swap(first_task_);
    }
    return ret;
}

long WorkerThread::GetTaskCounter(void)
{
    long value = task_counter_;
    return value;
}

std::shared_ptr<Thread> WorkerThread::GetThread(void)
{
    return thread_;
}

}  // namespace AppExecFwk
}  // namespace OHOS
