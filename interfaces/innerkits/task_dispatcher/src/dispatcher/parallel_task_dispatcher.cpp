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
#include "parallel_task_dispatcher.h"
#include "app_log_wrapper.h"

namespace OHOS {
namespace AppExecFwk {
const std::string ParallelTaskDispatcher::DISPATCHER_TAG = "ParallelTaskDispatcher";
const std::string ParallelTaskDispatcher::ASYNC_DISPATCHER_BARRIER_TAG = DISPATCHER_TAG + "::asyncDispatchBarrier";
const std::string ParallelTaskDispatcher::SYNC_DISPATCHER_BARRIER_TAG = DISPATCHER_TAG + "::syncDispatchBarrier";
ParallelTaskDispatcher::ParallelTaskDispatcher(
    const std::string &name, TaskPriority priority, std::shared_ptr<TaskExecutor> &executor)
    : ParallelTaskDispatcherBase(priority, executor, name)
{
    barrierHandler_ = std::make_shared<BarrierHandler>(executor);
}

std::shared_ptr<TaskExecuteInterceptor> ParallelTaskDispatcher::GetInterceptor()
{
    return barrierHandler_;
}

ErrCode ParallelTaskDispatcher::SyncDispatchBarrier(const std::shared_ptr<Runnable> &runnable)
{
    APP_LOGI("ParallelTaskDispatcher::SyncDispatchBarrier start");
    if (Check(runnable) != ERR_OK) {
        APP_LOGE("ParallelTaskDispatcher::SyncDispatchBarrier Check failed");
        return ERR_APPEXECFWK_CHECK_FAILED;
    }

    std::shared_ptr<SyncTask> innerSyncTask = std::make_shared<SyncTask>(runnable, GetPriority(), shared_from_this());
    if (innerSyncTask == nullptr) {
        APP_LOGE("ParallelTaskDispatcher::SyncDispatchBarrier innerSyncTask is nullptr");
        return ERR_APPEXECFWK_CHECK_FAILED;
    }
    std::shared_ptr<Task> innerTask = std::static_pointer_cast<Task>(innerSyncTask);
    if (innerTask == nullptr) {
        APP_LOGE("ParallelTaskDispatcher::SyncDispatchBarrier innerTask is nullptr");
        return ERR_APPEXECFWK_CHECK_FAILED;
    }
    APP_LOGD("ParallelTaskDispatcher::SyncDispatchBarrier into new sync task");
    if (barrierHandler_ == nullptr) {
        APP_LOGE("ParallelTaskDispatcher::SyncDispatchBarrier barrierHandler_ is nullptr");
        return ERR_APPEXECFWK_CHECK_FAILED;
    }
    barrierHandler_->AddBarrier(innerTask);
    innerSyncTask->WaitTask();
    APP_LOGD("ParallelTaskDispatcher::SyncDispatchBarrier end");
    return ERR_OK;
}

ErrCode ParallelTaskDispatcher::AsyncDispatchBarrier(const std::shared_ptr<Runnable> &runnable)
{
    APP_LOGI("ParallelTaskDispatcher::AsyncDispatchBarrier start");
    if (Check(runnable) != ERR_OK) {
        APP_LOGE("ParallelTaskDispatcher::AsyncDispatchBarrier check failed");
        return ERR_APPEXECFWK_CHECK_FAILED;
    }
    if (barrierHandler_ == nullptr) {
        APP_LOGE("ParallelTaskDispatcher::AsyncDispatchBarrier barrierHandler_ is nullptr");
        return ERR_APPEXECFWK_CHECK_FAILED;
    }
    std::shared_ptr<Task> innerTask = std::make_shared<Task>(runnable, GetPriority(), shared_from_this());
    APP_LOGI("ParallelTaskDispatcher::AsyncDispatchBarrier into new async task");
    if (innerTask == nullptr) {
        APP_LOGE("ParallelTaskDispatcher::AsyncDispatchBarrier innerTask is nullptr");
        return ERR_APPEXECFWK_CHECK_FAILED;
    }

    barrierHandler_->AddBarrier(innerTask);
    APP_LOGI("ParallelTaskDispatcher::AsyncDispatchBarrier end");
    return ERR_OK;
}
}  // namespace AppExecFwk
}  // namespace OHOS