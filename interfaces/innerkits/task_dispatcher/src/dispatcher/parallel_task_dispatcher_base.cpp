/*
 * Copyright (c) 2021 Huawei Device Co., Ltd.
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 * http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */
#include "parallel_task_dispatcher_base.h"

#include "app_log_wrapper.h"
#include "appexecfwk_errors.h"

namespace OHOS {
namespace AppExecFwk {
std::string ParallelTaskDispatcherBase::DISPATCHER_TAG = "ParallelTaskDispatcherBase";
std::string ParallelTaskDispatcherBase::ASYNC_DISPATCHER_TAG = DISPATCHER_TAG + "::asyncDispatch";
std::string ParallelTaskDispatcherBase::SYNC_DISPATCHER_TAG = DISPATCHER_TAG + "::syncDispatch";
std::string ParallelTaskDispatcherBase::DELAY_DISPATCHER_TAG = DISPATCHER_TAG + "::delayDispatch";
std::string ParallelTaskDispatcherBase::ASYNC_GROUP_DISPATCHER_TAG = DISPATCHER_TAG + "::asyncGroupDispatch";
ParallelTaskDispatcherBase::ParallelTaskDispatcherBase(
    TaskPriority taskPriority, std::shared_ptr<TaskExecutor> &executor, const std::string &dispatcherName)
    : BaseTaskDispatcher(dispatcherName, taskPriority)
{
    executor_ = executor;
}
ErrCode ParallelTaskDispatcherBase::InterceptedExecute(std::shared_ptr<Task> &task)
{
    APP_LOGI("ParallelTaskDispatcherBase::InterceptedExecute start");
    if (executor_ == nullptr) {
        APP_LOGE("ParallelTaskDispatcherBase::InterceptedExecute executor_ is nullptr");
        return ERR_APPEXECFWK_CHECK_FAILED;
    }

    if ((GetInterceptor() != nullptr) &&
        GetInterceptor()->Intercept(task) == ERR_APPEXECFWK_INTERCEPT_TASK_EXECUTE_SUCCESS) {
        APP_LOGE("ParallelTaskDispatcherBase::InterceptedExecute intercept task execute success");
        return ERR_APPEXECFWK_INTERCEPT_TASK_EXECUTE_SUCCESS;
    }

    executor_->Execute(task);
    APP_LOGI("ParallelTaskDispatcherBase::InterceptedExecute end");
    return ERR_OK;
}

ErrCode ParallelTaskDispatcherBase::SyncDispatch(const std::shared_ptr<Runnable> &runnable)
{
    APP_LOGI("ParallelTaskDispatcherBase::SyncDispatch start");
    if (Check(runnable) != ERR_OK) {
        APP_LOGE("ParallelTaskDispatcherBase::SyncDispatch check failed");
        return ERR_APPEXECFWK_CHECK_FAILED;
    }

    std::shared_ptr<SyncTask> innerSyncTask = std::make_shared<SyncTask>(runnable, GetPriority(), shared_from_this());
    if (innerSyncTask == nullptr) {
        APP_LOGE("ParallelTaskDispatcherBase::SyncDispatch innerSyncTask is nullptr");
        return ERR_APPEXECFWK_CHECK_FAILED;
    }
    std::shared_ptr<Task> innerTask = std::static_pointer_cast<Task>(innerSyncTask);
    if (innerTask == nullptr) {
        APP_LOGE("ParallelTaskDispatcherBase::SyncDispatch innerTask is nullptr");
        return ERR_APPEXECFWK_CHECK_FAILED;
    }
    APP_LOGI("ParallelTaskDispatcherBase::SyncDispatch into new sync task");
    ErrCode execute = InterceptedExecute(innerTask);
    if (execute != ERR_OK) {
        APP_LOGE("ParallelTaskDispatcherBase::SyncDispatch execute failed");
        return execute;
    }
    innerSyncTask->WaitTask();
    APP_LOGI("ParallelTaskDispatcherBase::SyncDispatch end");
    return ERR_OK;
}

std::shared_ptr<Revocable> ParallelTaskDispatcherBase::AsyncDispatch(const std::shared_ptr<Runnable> &runnable)
{
    APP_LOGI("ParallelTaskDispatcherBase::AsyncDispatch start");
    if (Check(runnable) != ERR_OK) {
        APP_LOGE("ParallelTaskDispatcherBase::AsyncDispatch check failed.");
        return nullptr;
    }

    std::shared_ptr<Task> innerTask = std::make_shared<Task>(runnable, GetPriority(), shared_from_this());
    if (innerTask == nullptr) {
        APP_LOGE("ParallelTaskDispatcherBase::AsyncDispatch innerTask is nullptr.");
        return nullptr;
    }
    TracePointBeforePost(innerTask, true, ASYNC_DISPATCHER_TAG);
    APP_LOGI("ParallelTaskDispatcherBase::AsyncDispatch into new async task");
    ErrCode execute = InterceptedExecute(innerTask);
    if (execute != ERR_OK) {
        APP_LOGE("ParallelTaskDispatcherBase::AsyncDispatch execute failed");
        return nullptr;
    }
    APP_LOGI("ParallelTaskDispatcherBase::AsyncDispatch end");
    return innerTask;
}

std::shared_ptr<Revocable> ParallelTaskDispatcherBase::DelayDispatch(
    const std::shared_ptr<Runnable> &runnable, long delayMs)
{
    APP_LOGI("ParallelTaskDispatcherBase::DelayDispatch start");
    if (Check(runnable) != ERR_OK) {
        APP_LOGE("ParallelTaskDispatcherBase::DelayDispatch Check failed");
        return nullptr;
    }

    if (executor_ == nullptr) {
        APP_LOGE("ParallelTaskDispatcherBase::DelayDispatch executor_ is nullptr");
        return nullptr;
    }

    std::shared_ptr<Task> innerTask = std::make_shared<Task>(runnable, GetPriority(), shared_from_this());
    if (innerTask == nullptr) {
        APP_LOGE("ParallelTaskDispatcherBase::DelayDispatch innerTask is nullptr");
        return nullptr;
    }
    TracePointBeforePost(innerTask, true, DELAY_DISPATCHER_TAG);
    std::function<void()> callback =
        std::bind(&ParallelTaskDispatcherBase::InterceptedExecute, shared_from_this(), innerTask);
    bool executeFlag = executor_->DelayExecute(callback, delayMs);
    if (!executeFlag) {
        APP_LOGE("ParallelTaskDispatcherBase::DelayDispatch execute failed");
        return nullptr;
    }
    APP_LOGI("ParallelTaskDispatcherBase::DelayDispatch end");
    return innerTask;
}

std::shared_ptr<Revocable> ParallelTaskDispatcherBase::AsyncGroupDispatch(
    const std::shared_ptr<Group> &group, const std::shared_ptr<Runnable> &runnable)
{
    APP_LOGI("ParallelTaskDispatcherBase::AsyncGroupDispatch start");
    if (group == nullptr) {
        APP_LOGE("ParallelTaskDispatcherBase::AsyncGroupDispatch group is nullptr");
        return nullptr;
    }
    if (Check(runnable) != ERR_OK) {
        APP_LOGE("ParallelTaskDispatcherBase::AsyncGroupDispatch Check failed");
        return nullptr;
    }

    std::shared_ptr<GroupImpl> groupImpl = CastToGroupImpl(group);
    if (groupImpl == nullptr) {
        APP_LOGE("ParallelTaskDispatcherBase::AsyncGroupDispatch groupImpl is nullptr");
        return nullptr;
    }
    groupImpl->Associate();

    std::shared_ptr<Task> innerTask = std::make_shared<Task>(runnable, GetPriority(), shared_from_this());
    if (innerTask == nullptr) {
        APP_LOGE("ParallelTaskDispatcherBase::AsyncGroupDispatch innerTask is nullptr");
        return nullptr;
    }
    TracePointBeforePost(innerTask, true, ASYNC_GROUP_DISPATCHER_TAG);
    std::shared_ptr<MyTaskListener> ptrlistener = std::make_shared<MyTaskListener>();
    if (ptrlistener == nullptr) {
        APP_LOGE("ParallelTaskDispatcherBase::AsyncGroupDispatch ptrlistener is nullptr");
        return nullptr;
    }
    const std::function<void()> onTaskDone = std::bind(&GroupImpl::NotifyTaskDone, groupImpl);
    ptrlistener->Callback(onTaskDone);
    innerTask->AddTaskListener(ptrlistener);
    ErrCode execute = InterceptedExecute(innerTask);
    if (execute != ERR_OK) {
        APP_LOGE("ParallelTaskDispatcherBase::AsyncGroupDispatch execute failed");
        return nullptr;
    }
    APP_LOGI("ParallelTaskDispatcherBase::AsyncGroupDispatch end");
    return innerTask;
}

std::shared_ptr<TaskExecuteInterceptor> ParallelTaskDispatcherBase::GetInterceptor()
{
    return nullptr;
}

}  // namespace AppExecFwk
}  // namespace OHOS