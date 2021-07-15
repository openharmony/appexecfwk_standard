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

#include "main_ability.h"
#include "app_log_wrapper.h"
#include "test_utils.h"
#include <algorithm>
#include <condition_variable>
#include <chrono>
#include <functional>

namespace OHOS {
namespace AppExecFwk {
using namespace OHOS::EventFwk;
using namespace OHOS::STtools;

int terminated_task_num = 0;
std::condition_variable cv;
std::mutex cv_mutex;
constexpr int testTaskCount = 3;
std::string innerSyncBarrierId = "innerSyncBarrierId";
std::string innerAsyncBarrierId = "innerAsyncBarrierId";
std::string innerGroupWaitId = "innerGroupWaitId";
std::string innerGroupNotifyId = "innerGroupNotifyId";
std::string outerSyncBarrierId = "outerSyncBarrierId";
std::string outerAsyncBarrierId = "outerAsyncBarrierId";
std::string outerGroupWaitId = "outerGroupWaitId";
std::string outerGroupNotifyId = "outerGroupNotifyId";

std::string delimiter = "_";
std::string innerDelimiter = "-";
std::string task_execution_sequence = delimiter;

bool Wait(const int task_num)
{
    APP_LOGI("-- -- -- -- -- --MainAbility::Wait");
    std::unique_lock<std::mutex> ulock(cv_mutex);
    using namespace std::chrono_literals;
    return cv.wait_for(ulock, 5000ms, [task_num] { return terminated_task_num == task_num; });
}

void TestTask(const std::string &task_id)
{
    {
        std::lock_guard<std::mutex> lock(cv_mutex);
        terminated_task_num++;
        task_execution_sequence += task_id + delimiter;
    }
    APP_LOGI("-- -- -- -- -- --MainAbility::TestTask: %{public}d %{public}s", terminated_task_num, task_id.c_str());
    TestUtils::PublishEvent(g_EVENT_RESP_FIRST_B, terminated_task_num, task_execution_sequence);
    cv.notify_one();
}

void Reset()
{
    APP_LOGI("-- -- -- -- -- --MainAbility::Reset");
    terminated_task_num = 0;
    task_execution_sequence = delimiter;
}

bool IsAscend(const std::vector<size_t> &vec)
{
    APP_LOGI("-- -- -- -- -- --MainAbility::IsAscend begin");
    auto pos = std::adjacent_find(std::begin(vec), std::end(vec), std::greater<size_t>());
    return pos == std::end(vec);
    APP_LOGI("-- -- -- -- -- --MainAbility::IsAscend end");
}

bool OuterTaskExecuted(TestSetting setting)
{
    APP_LOGI("-- -- -- -- -- --MainAbility::OuterTaskExecuted begin");
    std::string expectedTaskId;
    bool executed = true;
    for (int i = 0; i < testTaskCount; i++) {
        expectedTaskId = delimiter + std::to_string(i) + delimiter;
        executed = executed && (task_execution_sequence.find(expectedTaskId) != string::npos);
        if (!executed) {
            return executed;
        }
    }
    if (setting.sync_barrier) {
        expectedTaskId = delimiter + outerSyncBarrierId + delimiter;
        executed = executed && (task_execution_sequence.find(expectedTaskId) != string::npos);
    }
    if (setting.async_barrier) {
        expectedTaskId = delimiter + outerAsyncBarrierId + delimiter;
        executed = executed && (task_execution_sequence.find(expectedTaskId) != string::npos);
    }
    if (setting.group_notify) {
        expectedTaskId = delimiter + outerGroupNotifyId + delimiter;
        executed = executed && (task_execution_sequence.find(expectedTaskId) != string::npos);
    }
    APP_LOGI("-- -- -- -- -- --MainAbility::OuterTaskExecuted end");
    return executed;
}

bool InnerTaskExecuted(TestSetting setting)
{
    APP_LOGI("-- -- -- -- -- --MainAbility::InnerTaskExecuted begin");
    std::string expectedTaskId;
    bool executed = true;
    for (int i = 0; i < testTaskCount; i++) {
        for (int j = 0; j < testTaskCount; j++) {
            expectedTaskId = delimiter + std::to_string(i) + innerDelimiter + std::to_string(j) + delimiter;
            executed = executed && (task_execution_sequence.find(expectedTaskId) != string::npos);
            if (!executed) {
                return executed;
            }
        }
    }
    if (setting.sync_barrier) {
        expectedTaskId = delimiter + innerSyncBarrierId + delimiter;
        executed = executed && (task_execution_sequence.find(expectedTaskId) != string::npos);
    }
    if (setting.async_barrier) {
        expectedTaskId = delimiter + innerAsyncBarrierId + delimiter;
        executed = executed && (task_execution_sequence.find(expectedTaskId) != string::npos);
    }
    if (setting.group_notify) {
        expectedTaskId = delimiter + innerGroupNotifyId + delimiter;
        executed = executed && (task_execution_sequence.find(expectedTaskId) != string::npos);
    }
    APP_LOGI("-- -- -- -- -- --MainAbility::InnerTaskExecuted end");
    return executed;
}

void setTaskIndex(std::string taskId, std::vector<size_t> &taskIndex)
{
    APP_LOGI("-- -- -- -- -- --MainAbility::setTaskIndex begin");
    std::size_t indx = task_execution_sequence.find(taskId);
    if (indx != string::npos) {
        taskIndex.push_back(indx);
    }
    APP_LOGI("-- -- -- -- -- --MainAbility::setTaskIndex end");
}

void GetTaskIndex(std::vector<size_t> &outerTaskIndex, std::vector<std::vector<size_t>> &innerTaskIndex,
    const int outerCnt = testTaskCount, const int innerCnt = testTaskCount)
{
    APP_LOGI("-- -- -- -- -- --MainAbility::GetTaskIndex begin");
    std::string outerTaskId;
    std::string innerTaskId;
    outerTaskIndex.resize(outerCnt);
    APP_LOGI("-- -- -- -- -- --MainAbility::GetTaskIndex outersize : %{public}d", outerTaskIndex.size());
    innerTaskIndex.resize(outerCnt);
    APP_LOGI("-- -- -- -- -- --MainAbility::GetTaskIndex innersize : %{public}d", innerTaskIndex.size());
    for (auto &inner : innerTaskIndex) {
        inner.resize(innerCnt);
        APP_LOGI("-- -- -- -- -- --MainAbility::GetTaskIndex inner :%{public}d", inner.size());
    }
    APP_LOGI("-- -- -- -- -- --MainAbility::GetTaskIndex mid1");
    for (int i = 0; i < outerCnt; i++) {
        outerTaskId = delimiter + std::to_string(i) + delimiter;
        APP_LOGI("-- -- -- -- -- --MainAbility::GetTaskIndex mid2");
        outerTaskIndex[i] = task_execution_sequence.find(outerTaskId);
        APP_LOGI("-- -- -- -- -- --MainAbility::GetTaskIndex mid3");
        for (int j = 0; j < innerCnt; j++) {
            innerTaskId = delimiter + std::to_string(i) + innerDelimiter + std::to_string(j) + delimiter;
            APP_LOGI("-- -- -- -- -- --MainAbility::GetTaskIndex mid4");
            innerTaskIndex[i][j] = task_execution_sequence.find(innerTaskId);
            APP_LOGI("-- -- -- -- -- --MainAbility::GetTaskIndex mid5");
        }
        std::string taskId = innerSyncBarrierId + std::to_string(i);
        setTaskIndex(taskId, innerTaskIndex[i]);
        taskId = innerAsyncBarrierId + std::to_string(i);
        setTaskIndex(taskId, innerTaskIndex[i]);
        taskId = innerGroupNotifyId + std::to_string(i);
        setTaskIndex(taskId, innerTaskIndex[i]);
    }
    setTaskIndex(outerSyncBarrierId, outerTaskIndex);
    setTaskIndex(outerAsyncBarrierId, outerTaskIndex);
    setTaskIndex(outerGroupNotifyId, outerTaskIndex);
    APP_LOGI("-- -- -- -- -- --MainAbility::  end");
}

void MainAbility::Init(const std::shared_ptr<AbilityInfo> &abilityInfo,
    const std::shared_ptr<OHOSApplication> &application, std::shared_ptr<AbilityHandler> &handler,
    const sptr<IRemoteObject> &token)
{
    APP_LOGI("MainAbility::Init");
    Ability::Init(abilityInfo, application, handler, token);
}

MainAbility::~MainAbility()
{
    CommonEventManager::UnSubscribeCommonEvent(subscriber_);
}

void MainAbility::OnStart(const Want &want)
{
    APP_LOGI("MainAbility::onStart");
    SubscribeEvent();
    Ability::OnStart(want);
    TestUtils::PublishEvent(g_EVENT_RESP_FIRST_B, Ability::GetState(), "onStart");
}

void MainAbility::OnStop()
{
    APP_LOGI("MainAbility::OnStop");
    Ability::OnStop();
    CommonEventManager::UnSubscribeCommonEvent(subscriber_);
    TestUtils::PublishEvent(g_EVENT_RESP_FIRST_B, Ability::GetState(), "OnStop");
}

void MainAbility::OnActive()
{
    APP_LOGI("MainAbility::OnActive");
    Ability::OnActive();
    TestUtils::PublishEvent(g_EVENT_RESP_FIRST_B, 0, "OnActive");
}

void MainAbility::OnInactive()
{
    APP_LOGI("MainAbility::OnInactive");
    Ability::OnInactive();
    TestUtils::PublishEvent(g_EVENT_RESP_FIRST_B, Ability::GetState(), "OnInactive");
}

void MainAbility::OnBackground()
{
    APP_LOGI("MainAbility::OnBackground");
    Ability::OnBackground();
    TestUtils::PublishEvent(g_EVENT_RESP_FIRST_B, Ability::GetState(), "OnBackground");
}

void MainAbility::OnForeground(const Want &want)
{
    APP_LOGI("MainAbility::OnForeground");
    Ability::OnForeground(want);
    TestUtils::PublishEvent(g_EVENT_RESP_FIRST_B, Ability::GetState(), "OnForeground");
}

void MainAbility::SubscribeEvent()
{
    std::vector<std::string> eventList = {
        g_EVENT_REQU_FIRST_B,
    };
    MatchingSkills matchingSkills;
    for (const auto &e : eventList) {
        matchingSkills.AddEvent(e);
    }
    CommonEventSubscribeInfo subscribeInfo(matchingSkills);
    subscribeInfo.SetPriority(1);
    subscriber_ = std::make_shared<FirstEventSubscriber>(subscribeInfo);
    subscriber_->mainAbility = this;
    CommonEventManager::SubscribeCommonEvent(subscriber_);
}

void FirstEventSubscriber::OnReceiveEvent(const CommonEventData &data)
{
    APP_LOGI("FirstEventSubscriber::OnReceiveEvent:event=%{public}s", data.GetWant().GetAction().c_str());
    APP_LOGI("FirstEventSubscriber::OnReceiveEvent:data=%{public}s", data.GetData().c_str());
    APP_LOGI("FirstEventSubscriber::OnReceiveEvent:code=%{public}d", data.GetCode());
    auto eventName = data.GetWant().GetAction();
    if (std::strcmp(eventName.c_str(), g_EVENT_REQU_FIRST_B.c_str()) == 0) {
        auto target = data.GetData();
        auto caseInfo = TestUtils::split(target, "_");
        APP_LOGI("FirstEventSubscriber::OnReceiveEvent:caseInfo.size()=%{public}d", caseInfo.size());
        if (caseInfo.size() < 3) {
            return;
        }
        APP_LOGI("FirstEventSubscriber::OnReceiveEvent:caseInfo[0]=%{public}s", caseInfo[0].c_str());
        APP_LOGI("FirstEventSubscriber::OnReceiveEvent:caseInfo[1]=%{public}s", caseInfo[1].c_str());
        APP_LOGI("FirstEventSubscriber::OnReceiveEvent:caseInfo[2]=%{public}s", caseInfo[2].c_str());
        if (mapTestFunc_.find(caseInfo[0]) != mapTestFunc_.end()) {
            mapTestFunc_[caseInfo[0]](std::stoi(caseInfo[1]), std::stoi(caseInfo[2]), data.GetCode());
        } else {
            APP_LOGI("OnReceiveEvent: CommonEventData error(%{public}s)", target.c_str());
        }
    }
}

void MainAbility::TestDispatcher(int apiIndex, int caseIndex, int code)
{
    if (mapCase_.find(apiIndex) != mapCase_.end()) {
        if (caseIndex < (int)mapCase_[apiIndex].size()) {
            mapCase_[apiIndex][caseIndex](code);
        }
    }
}

int MainAbility::Dispatch(TestSetting outerSetting, TestSetting innerSetting)
{
    APP_LOGI("-- -- -- -- -- --MainAbility::Dispatch begin");
    std::string outerName = "outerDispatcher";
    std::string innerName = "innerDispatcher";

    std::string outerTaskId;
    auto context = GetContext();
    TaskList outerDispatcher{outerSetting.dispatcher, context, outerName};
    if (outerSetting.create_group) {
        outerDispatcher.addOperation(TestOperation::CREATE_GROUP);
    }
    for (int i = 0; i < testTaskCount; i++) {
        outerTaskId = std::to_string(i);
        auto outerTask = std::make_shared<Runnable>([=]() {
            auto context = this->GetContext();
            TaskList innerDispatcher{innerSetting.dispatcher, context, innerName + std::to_string(i)};
            if (innerSetting.create_group) {
                innerDispatcher.addOperation(TestOperation::CREATE_GROUP);
            }
            for (int j = 0; j < testTaskCount; j++) {
                std::string innerTaskId = outerTaskId + innerDelimiter + std::to_string(j);
                auto innerTask = std::make_shared<Runnable>([=]() { TestTask(innerTaskId); });
                innerDispatcher.addOperation(innerSetting.op);
                if (innerSetting.op == TestOperation::APPLY && innerSetting.apply > 0) {
                    innerDispatcher.addApply(innerSetting.apply);
                }
                if (innerSetting.op == TestOperation::DELAY && innerSetting.delay > 0) {
                    innerDispatcher.addDelay(innerSetting.delay);
                }
                innerDispatcher.addFunc(innerTask);
            }
            if (innerSetting.sync_barrier) {
                std::string taskId = innerSyncBarrierId + std::to_string(i);
                auto task = std::make_shared<Runnable>([=]() { TestTask(taskId); });
                innerDispatcher.addOperation(TestOperation::SYNC_BARRIER).addFunc(task);
            }
            if (innerSetting.async_barrier) {
                std::string taskId = innerAsyncBarrierId + std::to_string(i);
                auto task = std::make_shared<Runnable>([=]() { TestTask(taskId); });
                innerDispatcher.addOperation(TestOperation::ASYNC_BARRIER).addFunc(task);
            }
            if (innerSetting.group_wait) {
                innerDispatcher.addOperation(TestOperation::GROUP_WAIT).addWaitTime(innerSetting.group_timeout);
            }
            if (innerSetting.group_notify) {
                std::string taskId = innerGroupNotifyId + std::to_string(i);
                auto task = std::make_shared<Runnable>([=]() { TestTask(taskId); });
                innerDispatcher.addOperation(TestOperation::GROUP_NOTIFY).addFunc(task);
            }
            innerDispatcher.executedTask();
            TestTask(outerTaskId);
        });
        outerDispatcher.addOperation(outerSetting.op);
        if (outerSetting.op == TestOperation::APPLY && outerSetting.apply > 0) {
            outerDispatcher.addApply(outerSetting.apply);
        }
        if (outerSetting.op == TestOperation::DELAY && outerSetting.delay > 0) {
            outerDispatcher.addDelay(outerSetting.delay);
        }
        outerDispatcher.addFunc(outerTask);
    }
    if (outerSetting.sync_barrier) {
        auto task = std::make_shared<Runnable>([=]() { TestTask(outerSyncBarrierId); });
        outerDispatcher.addOperation(TestOperation::SYNC_BARRIER).addFunc(task);
    }
    if (outerSetting.async_barrier) {
        auto task = std::make_shared<Runnable>([=]() { TestTask(outerAsyncBarrierId); });
        outerDispatcher.addOperation(TestOperation::ASYNC_BARRIER).addFunc(task);
    }
    if (outerSetting.group_wait) {
        outerDispatcher.addOperation(TestOperation::GROUP_WAIT).addWaitTime(outerSetting.group_timeout);
    }
    if (outerSetting.group_notify) {
        auto task = std::make_shared<Runnable>([=]() { TestTask(outerGroupNotifyId); });
        outerDispatcher.addOperation(TestOperation::GROUP_NOTIFY).addFunc(task);
    }
    outerDispatcher.executedTask();
    int taskCount = 0;
    taskCount = (innerSetting.op == TestOperation::APPLY) ? (innerSetting.apply * testTaskCount) : testTaskCount;
    taskCount = (outerSetting.op == TestOperation::APPLY)
                    ? (innerSetting.apply * testTaskCount + innerSetting.apply * testTaskCount * taskCount)
                    : (testTaskCount + testTaskCount * taskCount);

    APP_LOGI("-- -- -- -- -- --MainAbility::Dispatch end");
    return taskCount;
}

static void addTaskFromList(TaskList &dispatcher, const std::vector<TestOperation> &operationList, int &taskId,
    const int apply = 0, const long delay = 0)
{
    for (auto op : operationList) {
        if (op == TestOperation::CREATE_GROUP) {
            dispatcher.addOperation(op);
            continue;
        }
        if (op == TestOperation::APPLY) {
            dispatcher.addApply(apply);
        }
        if (op == TestOperation::DELAY) {
            dispatcher.addDelay(delay);
        }
        dispatcher.addOperation(op).addFunc(std::make_shared<Runnable>([=]() { TestTask(std::to_string(taskId)); }));
        taskId++;
    }
}

void MainAbility::MultiAppCase1(int code)
{
    APP_LOGI("-- -- -- -- -- --MainAbility::%{public}s", __FUNCTION__);
    Reset();
    auto context = GetContext();
    TaskList globalDispatcher = TaskList{TestDispatcher::GLOBAL, context, "global"};
    std::vector<TestOperation> operationList = {
        TestOperation::ASYNC,
        TestOperation::ASYNC,
        TestOperation::ASYNC,
        TestOperation::ASYNC,
        TestOperation::ASYNC,
        TestOperation::ASYNC,
        TestOperation::ASYNC,
        TestOperation::ASYNC,
        TestOperation::ASYNC,
        TestOperation::ASYNC,
    };
    int taskId = 0;
    addTaskFromList(globalDispatcher, operationList, taskId);
    globalDispatcher.executedTask();
    Wait(taskId);
    bool result = task_execution_sequence.size() > 1;
    std::vector<size_t> outerTaskIndex;
    std::vector<std::vector<size_t>> innerTaskIndex;
    GetTaskIndex(outerTaskIndex, innerTaskIndex, taskId, 0);
    for (auto index : outerTaskIndex) {
        result = result && (index != std::string::npos);
    }
    TestUtils::PublishEvent(g_EVENT_RESP_FIRST_B, code, std::to_string(result));
}

void MainAbility::MultiAppCase2(int code)
{
    APP_LOGI("-- -- -- -- -- --MainAbility::%{public}s", __FUNCTION__);
    Reset();
    auto context = GetContext();
    TaskList globalDispatcher = TaskList{TestDispatcher::PARALLEL, context, "parallel"};
    std::vector<TestOperation> operationList = {
        TestOperation::ASYNC,
        TestOperation::ASYNC,
        TestOperation::ASYNC,
        TestOperation::ASYNC,
        TestOperation::ASYNC,
        TestOperation::ASYNC,
        TestOperation::ASYNC,
        TestOperation::ASYNC,
        TestOperation::ASYNC,
        TestOperation::ASYNC,
    };
    int taskId = 0;
    addTaskFromList(globalDispatcher, operationList, taskId);
    globalDispatcher.executedTask();
    Wait(taskId);
    bool result = task_execution_sequence.size() > 1;
    std::vector<size_t> outerTaskIndex;
    std::vector<std::vector<size_t>> innerTaskIndex;
    GetTaskIndex(outerTaskIndex, innerTaskIndex, taskId, 0);
    for (auto index : outerTaskIndex) {
        result = result && (index != std::string::npos);
    }
    TestUtils::PublishEvent(g_EVENT_RESP_FIRST_B, code, std::to_string(result));
}

void MainAbility::MultiAppCase3(int code)
{
    APP_LOGI("-- -- -- -- -- --MainAbility::%{public}s", __FUNCTION__);
    Reset();
    auto context = GetContext();
    TaskList globalDispatcher = TaskList{TestDispatcher::SERIAL, context, "serial"};
    std::vector<TestOperation> operationList = {
        TestOperation::ASYNC,
        TestOperation::ASYNC,
        TestOperation::ASYNC,
        TestOperation::ASYNC,
        TestOperation::ASYNC,
        TestOperation::ASYNC,
        TestOperation::ASYNC,
        TestOperation::ASYNC,
        TestOperation::ASYNC,
        TestOperation::ASYNC,
    };
    int taskId = 0;
    addTaskFromList(globalDispatcher, operationList, taskId);
    globalDispatcher.executedTask();
    Wait(taskId);
    bool result = task_execution_sequence.size() > 1;
    std::vector<size_t> outerTaskIndex;
    std::vector<std::vector<size_t>> innerTaskIndex;
    GetTaskIndex(outerTaskIndex, innerTaskIndex, taskId, 0);
    for (auto index : outerTaskIndex) {
        result = result && (index != std::string::npos);
    }
    TestUtils::PublishEvent(g_EVENT_RESP_FIRST_B, code, std::to_string(result));
}

void MainAbility::MultiAppCase4(int code)
{
    APP_LOGI("-- -- -- -- -- --MainAbility::%{public}s", __FUNCTION__);
    Reset();
    auto context = GetContext();
    TaskList globalDispatcher = TaskList{TestDispatcher::MAIN, context, "main"};
    std::vector<TestOperation> operationList = {
        TestOperation::ASYNC,
        TestOperation::ASYNC,
        TestOperation::ASYNC,
        TestOperation::ASYNC,
        TestOperation::ASYNC,
        TestOperation::ASYNC,
        TestOperation::ASYNC,
        TestOperation::ASYNC,
        TestOperation::ASYNC,
        TestOperation::ASYNC,
    };
    int taskId = 0;
    addTaskFromList(globalDispatcher, operationList, taskId);
    globalDispatcher.executedTask();
    Wait(taskId);
    bool result = task_execution_sequence.size() > 1;
    std::vector<size_t> outerTaskIndex;
    std::vector<std::vector<size_t>> innerTaskIndex;
    GetTaskIndex(outerTaskIndex, innerTaskIndex, taskId, 0);
    for (auto index : outerTaskIndex) {
        result = result && (index != std::string::npos);
    }
    TestUtils::PublishEvent(g_EVENT_RESP_FIRST_B, code, std::to_string(result));
}
REGISTER_AA(MainAbility)
}  // namespace AppExecFwk
}  // namespace OHOS
