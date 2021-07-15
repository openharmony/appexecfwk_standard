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
#include "base_task_dispatcher.h"
#include "app_log_wrapper.h"

namespace OHOS {
namespace AppExecFwk {
std::atomic<int> BaseTaskDispatcher::SEQUENCE_(0);
BaseTaskDispatcher::BaseTaskDispatcher(const std::string &dispatcherName, const TaskPriority priority)
{
    std::string name = dispatcherName;
    if (name.size() == 0) {
        name = "Dispatcher-" + std::to_string(SEQUENCE_.fetch_add(1, std::memory_order_relaxed));
    }

    dispatcherName_ = name;
    taskPriority_ = priority;
}

ErrCode BaseTaskDispatcher::SyncDispatchBarrier(const std::shared_ptr<Runnable> &task)
{
    return SyncDispatch(task);
}
ErrCode BaseTaskDispatcher::AsyncDispatchBarrier(const std::shared_ptr<Runnable> &task)
{
    std::shared_ptr<Revocable> revocable = AsyncDispatch(task);
    if (revocable != nullptr) {
        return ERR_OK;
    }
    return ERR_APPEXECFWK_CHECK_FAILED;
}

std::shared_ptr<Group> BaseTaskDispatcher::CreateDispatchGroup()
{
    return std::make_shared<GroupImpl>();
}

std::shared_ptr<Revocable> BaseTaskDispatcher::AsyncGroupDispatch(
    const std::shared_ptr<Group> &group, const std::shared_ptr<Runnable> &task)
{
    return AsyncDispatch(task);
}

bool BaseTaskDispatcher::GroupDispatchWait(const std::shared_ptr<Group> &group, long timeout)
{
    if (group == nullptr) {
        APP_LOGE("BaseTaskDispatcher::GroupDispatchWait group is nullptr");
        return false;
    }
    std::shared_ptr<GroupImpl> groupImpl = CastToGroupImpl(group);
    if (groupImpl == nullptr) {
        APP_LOGE("BaseTaskDispatcher::GroupDispatchWait groupImpl is nullptr");
        return false;
    }
    return groupImpl->AwaitAllTasks(timeout);
}

ErrCode BaseTaskDispatcher::GroupDispatchNotify(
    const std::shared_ptr<Group> &group, const std::shared_ptr<Runnable> &task)
{
    bool flag = true;
    if (group == nullptr) {
        APP_LOGE("group cannot be null.");
        flag = false;
    }
    if (task == nullptr) {
        APP_LOGE("task cannot be null");
        flag = false;
    }
    const std::function<void()> asyncDispatch = std::bind(&BaseTaskDispatcher::AsyncDispatch, this, task);
    if (asyncDispatch == nullptr) {
        APP_LOGE("BaseTaskDispatcher::GroupDispatchNotify asyncDispatch is nullptr");
    }
    std::shared_ptr<Runnable> ptrCallback = std::make_shared<Runnable>(asyncDispatch);
    if (ptrCallback == nullptr) {
        APP_LOGE("BaseTaskDispatcher::GroupDispatchNotify runnable is nullptr");
        flag = false;
    }
    if (group == nullptr) {
        APP_LOGE("BaseTaskDispatcher::GroupDispatchNotify group is nullptr");
        flag = false;
    }
    std::shared_ptr<GroupImpl> groupImpl = CastToGroupImpl(group);
    if (groupImpl == nullptr) {
        APP_LOGE("BaseTaskDispatcher::GroupDispatchNotify groupImpl is nullptr");
        flag = false;
    }
    if (!flag) {
        return ERR_APPEXECFWK_CHECK_FAILED;
    }

    if (groupImpl->AddNotification(ptrCallback)) {
        return ERR_OK;
    };
    return ERR_APPEXECFWK_CHECK_FAILED;
}

ErrCode BaseTaskDispatcher::ApplyDispatch(const std::shared_ptr<IteratableTask<long>> &task, long iterations)
{
    if (task == nullptr) {
        APP_LOGE("task object is not set");
        return ERR_APPEXECFWK_CHECK_FAILED;
    }

    if (iterations <= 0) {
        APP_LOGE("iterations must giant than 0");
        return ERR_APPEXECFWK_CHECK_FAILED;
    }

    bool flag = true;
    for (long i = 0L; i < iterations; ++i) {
        std::shared_ptr<Runnable> ptrCallback = std::make_shared<Runnable>([task, i]() { (*task)(i); });
        if (ptrCallback == nullptr) {
            APP_LOGE("BaseTaskDispatcher::ApplyDispatch runnable is nullptr");
            return ERR_APPEXECFWK_CHECK_FAILED;
        }
        std::shared_ptr<Revocable> revocable = AsyncDispatch(ptrCallback);
        if (revocable == nullptr) {
            APP_LOGE("BaseTaskDispatcher::ApplyDispatch revocable is nullptr");
            flag = false;
        }
    }
    if (flag) {
        return ERR_OK;
    }
    return ERR_APPEXECFWK_CHECK_FAILED;
}

ErrCode BaseTaskDispatcher::Check(const std::shared_ptr<Runnable> &task) const
{
    if (task == nullptr) {
        APP_LOGE("dispatch task cannot be null.");
        return ERR_APPEXECFWK_CHECK_FAILED;
    }
    return ERR_OK;
}

std::shared_ptr<GroupImpl> BaseTaskDispatcher::CastToGroupImpl(const std::shared_ptr<Group> &group)
{
    std::shared_ptr<GroupImpl> groupImpl_ptr = std::static_pointer_cast<GroupImpl>(group);
    if (groupImpl_ptr != nullptr) {
        return groupImpl_ptr;
    }
    APP_LOGE("group cannot instance of groupImpl ");
    return nullptr;
}

std::shared_ptr<TaskExecuteInterceptor> BaseTaskDispatcher::GetInterceptor()
{
    return nullptr;
}

TaskPriority BaseTaskDispatcher::GetPriority() const
{
    return taskPriority_;
}

void BaseTaskDispatcher::TracePointBeforePost(
    std::shared_ptr<Task> &task, bool isAsyncTask, const std::string &dispatcherName) const
{

    if (task == nullptr) {
        APP_LOGE("tracePointBeforePost the task is null");
        return;
    }
    std::string taskType = isAsyncTask ? "ASYNC_TASK_STRING" : "SYNC_TASK_STRING";
    long seq = task->GetSequence();
    APP_LOGD("tracePointBeforePost log---TaskType:%{public}s,TaskSeq::%{public}ld,DispatcherName::%{public}s",
        taskType.c_str(),
        seq,
        dispatcherName.c_str());
}

void BaseTaskDispatcher::TracePointAfterPost(
    std::shared_ptr<Task> &task, bool isAsyncTask, const std::string &dispatcherName) const
{
    if (task == nullptr) {
        APP_LOGE("TracePointAfterPost the task is null");
        return;
    }
    std::string taskType = isAsyncTask ? "ASYNC_TASK_STRING" : "SYNC_TASK_STRING";
    long seq = task->GetSequence();
    APP_LOGD("tracePointBeforePost log---TaskType:%{public}s,TaskSeq::%{public}ld,DispatcherName::%{public}s",
        taskType.c_str(),
        seq,
        dispatcherName.c_str());
}
}  // namespace AppExecFwk
}  // namespace OHOS
