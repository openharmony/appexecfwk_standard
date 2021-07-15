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
#include "task_dispatcher_tools.h"

namespace OHOS {
namespace STtools {
using namespace AppExecFwk;
using namespace EventFwk;

TaskList::TaskList(TestDispatcher dispatcher, std::shared_ptr<Context> &context, std::string name)
    : dispatcher(dispatcher), context(context), name(name){};
TaskList &TaskList::setDispatcherName(std::string name)
{
    this->name = name;
    return *this;
}
TaskList &TaskList::setDispatcher(TestDispatcher dispatcher)
{
    this->dispatcher = dispatcher;
    return *this;
};
TaskList &TaskList::setTaskPriority(TaskPriority taskPriority)
{
    this->taskPriority = taskPriority;
    return *this;
}
TaskList &TaskList::addOperation(TestOperation operation)
{
    this->dispatchList.push_back(operation);
    return *this;
};
TaskList &TaskList::setContext(std::shared_ptr<Context> &context)
{
    this->context = context;
    return *this;
};
std::shared_ptr<TaskDispatcher> TaskList::getDispatcher()
{
    makeDispatcher();
    return taskDispatcher;
};
TaskList &TaskList::addFunc(std::shared_ptr<Runnable> runnable)
{
    funcList.push(runnable);
    return *this;
};
TaskList &TaskList::addDelay(long delay)
{
    delayTimes.push(delay);
    return *this;
};
TaskList &TaskList::addApply(long apply)
{
    applyTimes.push(apply);
    return *this;
};
TaskList &TaskList::addRevokeTask(unsigned int num)
{
    revokeTaskNumber.push(num);
    return *this;
}
TaskList &TaskList::addWaitTime(long time)
{
    groupWaitTimes.push(time);
    return *this;
}
bool TaskList::executedTask()
{
    bool result = true;
    makeDispatcher();
    if (taskDispatcher == nullptr) {
        return false;
    }
    for (auto operation : dispatchList) {
        switch (operation) {
            case TestOperation::SYNC:
                executeSyncDispatch();
                break;
            case TestOperation::ASYNC:
                executeAsyncDispatch();
                break;
            case TestOperation::DELAY:
                executeDelayDispatch();
                break;
            case TestOperation::SYNC_BARRIER:
                executeSyncDispatchBarrier();
                break;
            case TestOperation::ASYNC_BARRIER:
                executeAsyncDispatchBarrier();
                break;
            case TestOperation::CREATE_GROUP:
                executeCreateDispatchGroup();
                break;
            case TestOperation::ASYNC_GROUP:
                executeAsyncGroupDispatch();
                break;
            case TestOperation::GROUP_WAIT:
                result = executeGroupDispatchWait();
                break;
            case TestOperation::GROUP_NOTIFY:
                executeGroupDispatchNotify();
                break;
            case TestOperation::APPLY:
                executeApplyDispatch();
                break;
            case TestOperation::REVOCABLE:
                result = executeRevokeTask();
                break;
            default:
                result = false;
                break;
        }
    }
    return result;
};

void TaskList::makeDispatcher()
{
    if (context == nullptr) {
        taskDispatcher = nullptr;
        return;
    }
    if (taskDispatcher != nullptr) {
        return;
    }
    switch (dispatcher) {
        case TestDispatcher::GLOBAL:
            taskDispatcher = context->GetGlobalTaskDispatcher(taskPriority);
            break;
        case TestDispatcher::PARALLEL:
            taskDispatcher = context->CreateParallelTaskDispatcher(name, taskPriority);
            break;
        case TestDispatcher::SERIAL:
            taskDispatcher = context->CreateSerialTaskDispatcher(name, taskPriority);
            break;
        case TestDispatcher::UI:
            taskDispatcher = context->GetUITaskDispatcher();
            break;
        case TestDispatcher::MAIN:
            taskDispatcher = context->GetMainTaskDispatcher();
            break;
        default:
            break;
    }
    return;
};

void TaskList::executeSyncDispatch()
{
    APP_LOGI("TaskList::executeSyncDispatch");
    if (funcList.size() == 0) {
        return;
    }
    APP_LOGI("TaskList::executeSyncDispatch execute");
    auto runnable = funcList.front();
    funcList.pop();
    taskDispatcher->SyncDispatch(runnable);
    return;
};
void TaskList::executeAsyncDispatch()
{
    APP_LOGI("TaskList::executeAsyncDispatch");
    if (funcList.size() == 0) {
        return;
    }
    APP_LOGI("TaskList::executeAsyncDispatch execute");
    auto runnable = funcList.front();
    funcList.pop();
    std::shared_ptr<Revocable> revocable = taskDispatcher->AsyncDispatch(runnable);
    revocableList.push_back(revocable);
    return;
};
void TaskList::executeDelayDispatch()
{
    APP_LOGI("TaskList::executeDelayDispatch");
    if (funcList.size() == 0 || delayTimes.size() == 0) {
        return;
    }
    APP_LOGI("TaskList::executeDelayDispatch execute");
    auto runnable = funcList.front();
    funcList.pop();
    std::shared_ptr<Revocable> revocable = taskDispatcher->DelayDispatch(runnable, delayTimes.front());
    delayTimes.pop();
    revocableList.push_back(revocable);
    return;
};
void TaskList::executeSyncDispatchBarrier()
{
    APP_LOGI("TaskList::executeSyncDispatchBarrier");
    if (funcList.size() == 0) {
        return;
    }
    APP_LOGI("TaskList::executeSyncDispatchBarrier execute");
    auto runnable = funcList.front();
    funcList.pop();
    taskDispatcher->SyncDispatchBarrier(runnable);
    return;
};
void TaskList::executeAsyncDispatchBarrier()
{
    APP_LOGI("TaskList::executeAsyncDispatchBarrier");
    if (funcList.size() == 0) {
        return;
    }
    APP_LOGI("TaskList::executeAsyncDispatchBarrier execute");
    auto runnable = funcList.front();
    funcList.pop();
    taskDispatcher->AsyncDispatchBarrier(runnable);
    return;
};
void TaskList::executeCreateDispatchGroup()
{
    APP_LOGI("TaskList::executeCreateDispatchGroup execute");
    group = taskDispatcher->CreateDispatchGroup();
    return;
};
void TaskList::executeAsyncGroupDispatch()
{
    APP_LOGI("TaskList::executeAsyncGroupDispatch");
    if (funcList.size() == 0) {
        return;
    }
    APP_LOGI("TaskList::executeAsyncGroupDispatch execute");
    auto runnable = funcList.front();
    funcList.pop();
    std::shared_ptr<Revocable> revocable = taskDispatcher->AsyncGroupDispatch(group, runnable);
    revocableList.push_back(revocable);
    return;
};
bool TaskList::executeGroupDispatchWait()
{
    APP_LOGI("TaskList::executeGroupDispatchWait");
    if (groupWaitTimes.size() == 0) {
        return false;
    }
    APP_LOGI("TaskList::executeGroupDispatchWait execute");
    long waitTimeout = groupWaitTimes.front();
    groupWaitTimes.pop();
    return taskDispatcher->GroupDispatchWait(group, waitTimeout);
};
void TaskList::executeGroupDispatchNotify()
{
    APP_LOGI("TaskList::executeGroupDispatchNotify");
    if (funcList.size() == 0) {
        return;
    }
    APP_LOGI("TaskList::executeGroupDispatchNotify execute");
    auto runnable = funcList.front();
    funcList.pop();
    taskDispatcher->GroupDispatchNotify(group, runnable);
};
void TaskList::executeApplyDispatch()
{
    APP_LOGI("TaskList::executeApplyDispatch");
    if (funcList.size() == 0 || applyTimes.size() == 0) {
        return;
    }
    APP_LOGI("TaskList::executeApplyDispatch execute");
    auto func = funcList.front();
    funcList.pop();
    auto runnable = std::make_shared<IteratableTask<long>>([func](long applyTime) { (*func.get())(); });
    taskDispatcher->ApplyDispatch(runnable, applyTimes.front());
    applyTimes.pop();
};
bool TaskList::executeRevokeTask()
{
    APP_LOGI("TaskList::executeRevokeTask");
    if (revokeTaskNumber.size() == 0 || revocableList.size() == 0) {
        if (revokeTaskNumber.size() > 0) {
            revokeTaskNumber.pop();
        }
        return false;
    }
    if (revocableList.size() < revokeTaskNumber.front()) {
        revokeTaskNumber.pop();
        return false;
    }
    APP_LOGI("TaskList::executeRevokeTask execute");
    bool result = revocableList[revokeTaskNumber.front() - 1]->Revoke();
    revokeTaskNumber.pop();
    return result;
}
}  // namespace STtools
}  // namespace OHOS