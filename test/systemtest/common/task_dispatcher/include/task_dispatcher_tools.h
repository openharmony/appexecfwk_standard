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
#ifndef TASK_DISPATCHER_ST_TOOLS
#define TASK_DISPATCHER_ST_TOOLS

#include <functional>
#include <stdarg.h>
#include <vector>
#include <queue>

#include "common_event_manager.h"
#include "context.h"
#include "group.h"
#include "app_log_wrapper.h"
#include "task_dispatcher.h"
#include "task_priority.h"
#include "revocable.h"
#include "runnable.h"

namespace OHOS {
namespace STtools {
enum class TestDispatcher {
    GLOBAL = 0,
    PARALLEL,
    SERIAL,
    UI,
    MAIN,
};
enum class TestOperation {
    SYNC = 0,
    ASYNC,
    DELAY,
    SYNC_BARRIER,
    ASYNC_BARRIER,
    CREATE_GROUP,
    ASYNC_GROUP,
    GROUP_WAIT,
    GROUP_NOTIFY,
    APPLY,
    REVOCABLE,
};
struct TestSetting {
    long apply = 1;
    long delay = 0;
    long group_timeout = 0;
    bool sync_barrier = false;
    bool async_barrier = false;
    bool create_group = false;
    bool group_wait = false;
    bool group_notify = false;
    TestDispatcher dispatcher;
    TestOperation op;
};

class TaskList {
public:
    TaskList() = default;
    ~TaskList() = default;
    TaskList(TestDispatcher dispatcher, std::shared_ptr<AppExecFwk::Context> &context, std::string name);
    TaskList &setDispatcherName(std::string name);
    TaskList &setDispatcher(TestDispatcher dispatcher);
    TaskList &setTaskPriority(AppExecFwk::TaskPriority taskPriority);
    TaskList &addOperation(TestOperation operation);
    TaskList &setContext(std::shared_ptr<AppExecFwk::Context> &context);
    TaskList &addFunc(std::shared_ptr<AppExecFwk::Runnable> runnable);

    TaskList &addDelay(long delay);
    TaskList &addApply(long apply);
    TaskList &addRevokeTask(unsigned int num);
    TaskList &addWaitTime(long time);
    bool executedTask();

    std::shared_ptr<AppExecFwk::TaskDispatcher> getDispatcher();

private:
    void makeDispatcher();
    void executeSyncDispatch();
    void executeAsyncDispatch();
    void executeDelayDispatch();
    void executeSyncDispatchBarrier();
    void executeAsyncDispatchBarrier();
    void executeCreateDispatchGroup();
    void executeAsyncGroupDispatch();
    bool executeGroupDispatchWait();
    void executeGroupDispatchNotify();
    void executeApplyDispatch();
    bool executeRevokeTask();

private:
    TestDispatcher dispatcher = TestDispatcher::GLOBAL;
    AppExecFwk::TaskPriority taskPriority = AppExecFwk::TaskPriority::DEFAULT;
    std::vector<TestOperation> dispatchList;
    std::shared_ptr<AppExecFwk::Context> context = nullptr;
    std::shared_ptr<AppExecFwk::TaskDispatcher> taskDispatcher = nullptr;
    std::queue<std::shared_ptr<AppExecFwk::Runnable>> funcList;
    std::shared_ptr<AppExecFwk::Group> group = nullptr;
    std::vector<std::shared_ptr<AppExecFwk::Revocable>> revocableList;
    std::queue<long> delayTimes;
    std::queue<long> applyTimes;
    std::queue<long> groupWaitTimes;
    std::string name;
    std::queue<unsigned int> revokeTaskNumber;
};
}  // namespace STtools
}  // namespace OHOS
#endif  // TASK_DISPATCHER_ST_TOOLS