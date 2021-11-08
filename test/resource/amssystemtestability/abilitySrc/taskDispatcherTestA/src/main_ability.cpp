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
#include <regex>
#include <string>

namespace OHOS {
namespace AppExecFwk {
using namespace OHOS::EventFwk;
using namespace OHOS::STtools;
namespace {
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
constexpr int applyNum = 3;
constexpr int delayMs = 200;
constexpr int groupWait = 1000;
std::string delimiter = "_";
std::string innerDelimiter = "-";
std::string task_execution_sequence = delimiter;
std::vector<std::shared_ptr<AppExecFwk::TaskDispatcher>> allDispatchers;
std::mutex dispatcher_mutex;
constexpr int numZero = 0;
constexpr int numOne = 1;
constexpr int numTwo = 2;
constexpr int numThree = 3;
}  // namespace

bool Wait(const int task_num)
{
    std::unique_lock<std::mutex> ulock(cv_mutex);
    using namespace std::chrono_literals;
    bool result = cv.wait_for(ulock, 5000ms, [task_num] { return terminated_task_num == task_num; });
    if (result) {
        allDispatchers.clear();
    }
    return result;
}

void TestTask(const std::string task_id)
{
    {
        std::lock_guard<std::mutex> lock(cv_mutex);
        terminated_task_num++;
        task_execution_sequence += task_id + delimiter;
    }
    TestUtils::PublishEvent(g_EVENT_RESP_FIRST, terminated_task_num, task_execution_sequence);
    cv.notify_one();
}

void Reset()
{
    terminated_task_num = 0;
    task_execution_sequence = delimiter;
    allDispatchers.clear();
}

bool IsAscend(const std::vector<size_t> &vec)
{
    auto pos = std::adjacent_find(std::begin(vec), std::end(vec), std::greater<size_t>());
    return pos == std::end(vec);
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
    APP_LOGI("-- -- -- -- -- --MainAbility::OuterTaskExecuted end result:%{public}d", executed);
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
        for (int i = 0; i < testTaskCount; i++) {
            expectedTaskId = delimiter + innerSyncBarrierId + std::to_string(i) + delimiter;
            executed = executed && (task_execution_sequence.find(expectedTaskId) != string::npos);
        }
    }
    if (setting.async_barrier) {
        for (int i = 0; i < testTaskCount; i++) {
            expectedTaskId = delimiter + innerAsyncBarrierId + std::to_string(i) + delimiter;
            executed = executed && (task_execution_sequence.find(expectedTaskId) != string::npos);
        }
    }
    if (setting.group_notify) {
        for (int i = 0; i < testTaskCount; i++) {
            expectedTaskId = delimiter + innerGroupNotifyId + std::to_string(i) + delimiter;
            executed = executed && (task_execution_sequence.find(expectedTaskId) != string::npos);
        }
    }
    APP_LOGI("-- -- -- -- -- --MainAbility::InnerTaskExecuted end result:%{public}d", executed);
    return executed;
}

int countSubstr(std::string str, std::string substr)
{
    std::regex m(substr);
    std::sregex_iterator itr(str.begin(), str.end(), m);
    std::sregex_iterator end;
    return std::distance(itr, end);
}

int countSubstrOverlap(std::string str, std::string substr)
{
    int count = 0;
    std::string::size_type pos = 0;
    while ((pos = str.find(substr, pos)) != std::string::npos) {
        pos++;
        count++;
    }
    return count;
}

bool applyExecuted(const TestSetting &outerSetting, const TestSetting &innerSetting)
{
    bool result = true;
    bool isOuterApply = outerSetting.op == TestOperation::APPLY && outerSetting.apply > 0;
    bool isInnerApply = innerSetting.op == TestOperation::APPLY && innerSetting.apply > 0;
    if (!isOuterApply && !isInnerApply) {
        return result;
    }
    if (isOuterApply) {
        for (int i = 0; i < testTaskCount; i++) {
            std::string taskId = delimiter + std::to_string(i) + delimiter;
            result = result && countSubstrOverlap(task_execution_sequence, taskId) == outerSetting.apply;
        }
    }
    int innerTimes = 0;
    if (isInnerApply && isOuterApply) {
        innerTimes = outerSetting.apply * innerSetting.apply;
    } else if (isOuterApply) {
        innerTimes = outerSetting.apply;
    } else {
        innerTimes = innerSetting.apply;
    }

    for (int i = 0; i < testTaskCount; i++) {
        for (int j = 0; j < testTaskCount; j++) {
            std::string taskId = std::to_string(i) + innerDelimiter + std::to_string(j);
            result = result && countSubstr(task_execution_sequence, taskId) == innerTimes;
        }
    }
    return result;
}
bool OuterExecutedAfter(const std::string baseTaskId)
{
    bool result = true;
    string::size_type waitPos = task_execution_sequence.find(baseTaskId);
    for (int i = 0; i < testTaskCount; i++) {
        std::string taskId = delimiter + std::to_string(i) + delimiter;
        result = result && waitPos > task_execution_sequence.find(taskId);
    }
    return result;
}

bool InnerExecutedAfter(const std::string baseTaskId)
{
    bool result = true;
    for (int i = 0; i < testTaskCount; i++) {
        std::string waitTaskId = delimiter + baseTaskId + std::to_string(i) + delimiter;
        string::size_type waitPos = task_execution_sequence.find(waitTaskId);
        for (int j = 0; j < testTaskCount; j++) {
            std::string taskId = delimiter + std::to_string(i) + innerDelimiter + std::to_string(j) + delimiter;
            result = result && waitPos > task_execution_sequence.find(taskId);
        }
    }
    return result;
}

void setTaskIndex(std::string taskId, std::vector<size_t> &taskIndex)
{
    std::size_t indx = task_execution_sequence.find(taskId);
    if (indx != string::npos) {
        taskIndex.push_back(indx);
    }
}

void GetTaskIndex(std::vector<size_t> &outerTaskIndex, std::vector<std::vector<size_t>> &innerTaskIndex,
    const int outerCnt = testTaskCount, const int innerCnt = testTaskCount)
{
    std::string outerTaskId;
    std::string innerTaskId;
    outerTaskIndex.resize(outerCnt);
    innerTaskIndex.resize(outerCnt);
    for (auto &inner : innerTaskIndex) {
        inner.resize(innerCnt);
    }
    for (int i = 0; i < outerCnt; i++) {
        outerTaskId = delimiter + std::to_string(i) + delimiter;
        outerTaskIndex[i] = task_execution_sequence.find(outerTaskId);
        for (int j = 0; j < innerCnt; j++) {
            innerTaskId = delimiter + std::to_string(i) + innerDelimiter + std::to_string(j) + delimiter;
            innerTaskIndex[i][j] = task_execution_sequence.find(innerTaskId);
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
    APP_LOGI("-- -- -- -- -- --MainAbility::GetTaskIndex end");
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
    TestUtils::PublishEvent(g_EVENT_RESP_FIRST, Ability::GetState(), "onStart");
}

void MainAbility::OnStop()
{
    APP_LOGI("MainAbility::OnStop");
    Ability::OnStop();
    CommonEventManager::UnSubscribeCommonEvent(subscriber_);
    TestUtils::PublishEvent(g_EVENT_RESP_FIRST, Ability::GetState(), "OnStop");
}

void MainAbility::OnActive()
{
    APP_LOGI("MainAbility::OnActive");
    Ability::OnActive();
    TestUtils::PublishEvent(g_EVENT_RESP_FIRST, 0, "OnActive");
}

void MainAbility::OnInactive()
{
    APP_LOGI("MainAbility::OnInactive");
    Ability::OnInactive();
    TestUtils::PublishEvent(g_EVENT_RESP_FIRST, Ability::GetState(), "OnInactive");
}

void MainAbility::OnBackground()
{
    APP_LOGI("MainAbility::OnBackground");
    Ability::OnBackground();
    TestUtils::PublishEvent(g_EVENT_RESP_FIRST, Ability::GetState(), "OnBackground");
}

void MainAbility::OnForeground(const Want &want)
{
    APP_LOGI("MainAbility::OnForeground");
    Ability::OnForeground(want);
    TestUtils::PublishEvent(g_EVENT_RESP_FIRST, Ability::GetState(), "OnForeground");
}

void MainAbility::SubscribeEvent()
{
    std::vector<std::string> eventList = {
        g_EVENT_REQU_FIRST,
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
    // APP_LOGI("FirstEventSubscriber::OnReceiveEvent:event=%{public}s", data.GetWant().GetAction().c_str());
    // APP_LOGI("FirstEventSubscriber::OnReceiveEvent:data=%{public}s", data.GetData().c_str());
    // APP_LOGI("FirstEventSubscriber::OnReceiveEvent:code=%{public}d", data.GetCode());
    auto eventName = data.GetWant().GetAction();
    if (std::strcmp(eventName.c_str(), g_EVENT_REQU_FIRST.c_str()) == 0) {
        auto target = data.GetData();
        auto caseInfo = TestUtils::split(target, "_");
        APP_LOGI("FirstEventSubscriber::OnReceiveEvent:caseInfo.size()=%{public}zu", caseInfo.size());
        if (caseInfo.size() < numThree) {
            return;
        }
        if (mapTestFunc_.find(caseInfo[numZero]) != mapTestFunc_.end()) {
            mapTestFunc_[caseInfo[numZero]](std::stoi(caseInfo[numOne]), std::stoi(caseInfo[numTwo]), data.GetCode());
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

void SetInnerTask(TaskList &innerDispatcher, TestSetting innerSetting, std::string outerTaskId, int innerTaskSeq)
{
    std::string innerTaskId = outerTaskId + innerDelimiter + std::to_string(innerTaskSeq);
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

void SetInnerTaskOther(TaskList &innerDispatcher, TestSetting innerSetting, int outerTaskSeq)
{
    if (innerSetting.sync_barrier) {
        std::string taskId = innerSyncBarrierId + std::to_string(outerTaskSeq);
        auto task = std::make_shared<Runnable>([=]() { TestTask(taskId); });
        innerDispatcher.addOperation(TestOperation::SYNC_BARRIER).addFunc(task);
    }
    if (innerSetting.async_barrier) {
        std::string taskId = innerAsyncBarrierId + std::to_string(outerTaskSeq);
        auto task = std::make_shared<Runnable>([=]() { TestTask(taskId); });
        innerDispatcher.addOperation(TestOperation::ASYNC_BARRIER).addFunc(task);
    }
    if (innerSetting.group_wait) {
        innerDispatcher.addOperation(TestOperation::GROUP_WAIT).addWaitTime(innerSetting.group_timeout);
    }
    if (innerSetting.group_notify) {
        std::string taskId = innerGroupNotifyId + std::to_string(outerTaskSeq);
        auto task = std::make_shared<Runnable>([=]() { TestTask(taskId); });
        innerDispatcher.addOperation(TestOperation::GROUP_NOTIFY).addFunc(task);
    }
}

void SetOuterTaskOther(TaskList &outerDispatcher, TestSetting outerSetting)
{
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
}

int CountTask(TestSetting outerSetting, TestSetting innerSetting)
{
    int taskCount = 0;
    taskCount = (innerSetting.op == TestOperation::APPLY) ? (innerSetting.apply * testTaskCount) : testTaskCount;
    if (innerSetting.sync_barrier) {
        taskCount++;
    }
    if (innerSetting.async_barrier) {
        taskCount++;
    }
    if (innerSetting.group_notify) {
        taskCount++;
    }
    taskCount = (outerSetting.op == TestOperation::APPLY)
                    ? (outerSetting.apply * testTaskCount + outerSetting.apply * testTaskCount * taskCount)
                    : (testTaskCount + testTaskCount * taskCount);
    if (outerSetting.sync_barrier) {
        taskCount++;
    }
    if (outerSetting.async_barrier) {
        taskCount++;
    }
    if (outerSetting.group_notify) {
        taskCount++;
    }
    return taskCount;
}

int MainAbility::Dispatch(TestSetting outerSetting, TestSetting innerSetting)
{
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
                SetInnerTask(innerDispatcher, innerSetting, outerTaskId, j);
            }
            SetInnerTaskOther(innerDispatcher, innerSetting, i);
            innerDispatcher.executedTask();
            {
                std::lock_guard<std::mutex> lock(dispatcher_mutex);
                allDispatchers.push_back(innerDispatcher.getDispatcher());
            }
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
    SetOuterTaskOther(outerDispatcher, outerSetting);
    outerDispatcher.executedTask();
    {
        std::lock_guard<std::mutex> lock(dispatcher_mutex);
        allDispatchers.push_back(outerDispatcher.getDispatcher());
    }
    int taskCount = CountTask(outerSetting, innerSetting);
    return taskCount;
}

// level1:global, sync level2:parallel, sync
void MainAbility::GlobalCase1(int code)
{
    APP_LOGI("-- -- -- -- -- --MainAbility::%{public}s", __FUNCTION__);
    Reset();
    TestSetting outerSetting;
    TestSetting innerSetting;
    outerSetting.dispatcher = TestDispatcher::GLOBAL;
    innerSetting.dispatcher = TestDispatcher::PARALLEL;
    outerSetting.op = TestOperation::SYNC;
    innerSetting.op = TestOperation::SYNC;
    int taskCount = Dispatch(outerSetting, innerSetting);
    bool result = Wait(taskCount);
    result = result && task_execution_sequence.size() > 1;
    if (result) {
        result = result && applyExecuted(outerSetting, innerSetting);
        result = result && OuterTaskExecuted(outerSetting);
        result = result && InnerTaskExecuted(innerSetting);
        std::vector<size_t> outerTaskIndex;
        std::vector<std::vector<size_t>> innerTaskIndex;
        GetTaskIndex(outerTaskIndex, innerTaskIndex);
        result = result && IsAscend(outerTaskIndex);
        for (int i = 0; i < testTaskCount; i++) {
            result = result && IsAscend(innerTaskIndex[i]);
        }
        result = result && applyExecuted(outerSetting, innerSetting);
    }
    TestUtils::PublishEvent(g_EVENT_RESP_FIRST, code, std::to_string(result));
}

// level1:global, sync level2:parallel, async
void MainAbility::GlobalCase2(int code)
{
    APP_LOGI("-- -- -- -- -- --MainAbility::%{public}s", __FUNCTION__);
    Reset();
    TestSetting outerSetting;
    TestSetting innerSetting;
    outerSetting.dispatcher = TestDispatcher::GLOBAL;
    innerSetting.dispatcher = TestDispatcher::PARALLEL;
    outerSetting.op = TestOperation::SYNC;
    innerSetting.op = TestOperation::ASYNC;
    int taskCount = Dispatch(outerSetting, innerSetting);
    bool result = Wait(taskCount);
    result = result && task_execution_sequence.size() > 1;
    if (result) {
        result = result && applyExecuted(outerSetting, innerSetting);
        result = result && OuterTaskExecuted(outerSetting);
        result = result && InnerTaskExecuted(innerSetting);
        std::vector<size_t> outerTaskIndex;
        std::vector<std::vector<size_t>> innerTaskIndex;
        GetTaskIndex(outerTaskIndex, innerTaskIndex);
        result = result && IsAscend(outerTaskIndex);
    }
    TestUtils::PublishEvent(g_EVENT_RESP_FIRST, code, std::to_string(result));
}

// level1:global, sync level2:parallel, delay
void MainAbility::GlobalCase3(int code)
{
    APP_LOGI("-- -- -- -- -- --MainAbility::%{public}s", __FUNCTION__);
    Reset();
    TestSetting outerSetting;
    TestSetting innerSetting;
    outerSetting.dispatcher = TestDispatcher::GLOBAL;
    innerSetting.dispatcher = TestDispatcher::PARALLEL;
    innerSetting.delay = delayMs;
    outerSetting.op = TestOperation::SYNC;
    innerSetting.op = TestOperation::DELAY;
    int taskCount = Dispatch(outerSetting, innerSetting);
    bool result = Wait(taskCount);
    result = result && task_execution_sequence.size() > 1;
    if (result) {
        result = result && applyExecuted(outerSetting, innerSetting);
        result = result && OuterTaskExecuted(outerSetting);
        result = result && InnerTaskExecuted(innerSetting);
        std::vector<size_t> outerTaskIndex;
        std::vector<std::vector<size_t>> innerTaskIndex;
        GetTaskIndex(outerTaskIndex, innerTaskIndex);
        result = result && IsAscend(outerTaskIndex);
    }
    TestUtils::PublishEvent(g_EVENT_RESP_FIRST, code, std::to_string(result));
}

// level1:global, sync level2:parallel, group
void MainAbility::GlobalCase4(int code)
{
    APP_LOGI("-- -- -- -- -- --MainAbility::%{public}s", __FUNCTION__);
    Reset();
    TestSetting outerSetting;
    TestSetting innerSetting;
    outerSetting.dispatcher = TestDispatcher::GLOBAL;
    innerSetting.dispatcher = TestDispatcher::PARALLEL;
    innerSetting.create_group = true;
    outerSetting.op = TestOperation::SYNC;
    innerSetting.op = TestOperation::ASYNC_GROUP;
    int taskCount = Dispatch(outerSetting, innerSetting);
    bool result = Wait(taskCount);
    result = result && task_execution_sequence.size() > 1;
    if (result) {
        result = result && applyExecuted(outerSetting, innerSetting);
        result = result && OuterTaskExecuted(outerSetting);
        result = result && InnerTaskExecuted(innerSetting);
        std::vector<size_t> outerTaskIndex;
        std::vector<std::vector<size_t>> innerTaskIndex;
        GetTaskIndex(outerTaskIndex, innerTaskIndex);
        result = result && IsAscend(outerTaskIndex);
    }
    TestUtils::PublishEvent(g_EVENT_RESP_FIRST, code, std::to_string(result));
}

// level1:global, sync level2:parallel, group wait
void MainAbility::GlobalCase5(int code)
{
    APP_LOGI("-- -- -- -- -- --MainAbility::%{public}s", __FUNCTION__);
    Reset();
    TestSetting outerSetting;
    TestSetting innerSetting;
    outerSetting.dispatcher = TestDispatcher::GLOBAL;
    innerSetting.dispatcher = TestDispatcher::PARALLEL;
    innerSetting.group_wait = true;
    innerSetting.group_timeout = groupWait;
    innerSetting.create_group = true;
    outerSetting.op = TestOperation::SYNC;
    innerSetting.op = TestOperation::ASYNC_GROUP;
    int taskCount = Dispatch(outerSetting, innerSetting);
    bool result = Wait(taskCount);
    result = result && task_execution_sequence.size() > 1;
    if (result) {
        result = result && applyExecuted(outerSetting, innerSetting);
        result = result && OuterTaskExecuted(outerSetting);
        result = result && InnerTaskExecuted(innerSetting);
        std::vector<size_t> outerTaskIndex;
        std::vector<std::vector<size_t>> innerTaskIndex;
        GetTaskIndex(outerTaskIndex, innerTaskIndex);
        result = result && IsAscend(outerTaskIndex);
    }
    TestUtils::PublishEvent(g_EVENT_RESP_FIRST, code, std::to_string(result));
}

// level1:global, sync level2:parallel, group notify
void MainAbility::GlobalCase6(int code)
{
    APP_LOGI("-- -- -- -- -- --MainAbility::%{public}s", __FUNCTION__);
    Reset();
    TestSetting outerSetting;
    TestSetting innerSetting;
    outerSetting.dispatcher = TestDispatcher::GLOBAL;
    innerSetting.dispatcher = TestDispatcher::PARALLEL;
    innerSetting.create_group = true;
    innerSetting.group_notify = true;
    outerSetting.op = TestOperation::SYNC;
    innerSetting.op = TestOperation::ASYNC_GROUP;
    int taskCount = Dispatch(outerSetting, innerSetting);
    bool result = Wait(taskCount);
    result = result && task_execution_sequence.size() > 1;
    if (result) {
        result = result && applyExecuted(outerSetting, innerSetting);
        result = result && OuterTaskExecuted(outerSetting);
        result = result && InnerTaskExecuted(innerSetting);
        std::vector<size_t> outerTaskIndex;
        std::vector<std::vector<size_t>> innerTaskIndex;
        GetTaskIndex(outerTaskIndex, innerTaskIndex);
        result = result && IsAscend(outerTaskIndex);
        result = result && InnerExecutedAfter(innerGroupNotifyId);
    }
    TestUtils::PublishEvent(g_EVENT_RESP_FIRST, code, std::to_string(result));
}

// level1:global, sync level2:parallel, sync barrier
void MainAbility::GlobalCase7(int code)
{
    APP_LOGI("-- -- -- -- -- --MainAbility::%{public}s", __FUNCTION__);
    Reset();
    TestSetting outerSetting;
    TestSetting innerSetting;
    outerSetting.dispatcher = TestDispatcher::GLOBAL;
    innerSetting.dispatcher = TestDispatcher::PARALLEL;
    innerSetting.sync_barrier = true;
    innerSetting.create_group = true;
    outerSetting.op = TestOperation::SYNC;
    innerSetting.op = TestOperation::ASYNC_GROUP;
    int taskCount = Dispatch(outerSetting, innerSetting);
    bool result = Wait(taskCount);
    result = result && task_execution_sequence.size() > 1;
    if (result) {
        result = result && applyExecuted(outerSetting, innerSetting);
        result = result && OuterTaskExecuted(outerSetting);
        result = result && InnerTaskExecuted(innerSetting);
        std::vector<size_t> outerTaskIndex;
        std::vector<std::vector<size_t>> innerTaskIndex;
        GetTaskIndex(outerTaskIndex, innerTaskIndex);
        result = result && IsAscend(outerTaskIndex);
        result = result && InnerExecutedAfter(innerSyncBarrierId);
    }
    TestUtils::PublishEvent(g_EVENT_RESP_FIRST, code, std::to_string(result));
}

// level1:global, sync level2:parallel, async barrier
void MainAbility::GlobalCase8(int code)
{
    APP_LOGI("-- -- -- -- -- --MainAbility::%{public}s", __FUNCTION__);
    Reset();
    TestSetting outerSetting;
    TestSetting innerSetting;
    outerSetting.dispatcher = TestDispatcher::GLOBAL;
    innerSetting.dispatcher = TestDispatcher::PARALLEL;
    innerSetting.async_barrier = true;
    innerSetting.create_group = true;
    outerSetting.op = TestOperation::SYNC;
    innerSetting.op = TestOperation::ASYNC_GROUP;
    int taskCount = Dispatch(outerSetting, innerSetting);
    bool result = Wait(taskCount);
    result = result && task_execution_sequence.size() > 1;
    if (result) {
        result = result && applyExecuted(outerSetting, innerSetting);
        result = result && OuterTaskExecuted(outerSetting);
        result = result && InnerTaskExecuted(innerSetting);
        std::vector<size_t> outerTaskIndex;
        std::vector<std::vector<size_t>> innerTaskIndex;
        GetTaskIndex(outerTaskIndex, innerTaskIndex);
        result = result && IsAscend(outerTaskIndex);
        result = result && InnerExecutedAfter(innerAsyncBarrierId);
    }
    TestUtils::PublishEvent(g_EVENT_RESP_FIRST, code, std::to_string(result));
}

// level1:global, sync level2:parallel, apply
void MainAbility::GlobalCase9(int code)
{
    APP_LOGI("-- -- -- -- -- --MainAbility::%{public}s", __FUNCTION__);
    Reset();
    TestSetting outerSetting;
    TestSetting innerSetting;
    outerSetting.dispatcher = TestDispatcher::GLOBAL;
    innerSetting.dispatcher = TestDispatcher::PARALLEL;
    innerSetting.apply = applyNum;
    outerSetting.op = TestOperation::SYNC;
    innerSetting.op = TestOperation::APPLY;
    int taskCount = Dispatch(outerSetting, innerSetting);
    bool result = Wait(taskCount);
    result = result && task_execution_sequence.size() > 1;
    if (result) {
        result = result && applyExecuted(outerSetting, innerSetting);
        result = result && OuterTaskExecuted(outerSetting);
        result = result && InnerTaskExecuted(innerSetting);
        std::vector<size_t> outerTaskIndex;
        std::vector<std::vector<size_t>> innerTaskIndex;
        GetTaskIndex(outerTaskIndex, innerTaskIndex);
        result = result && IsAscend(outerTaskIndex);
    }
    TestUtils::PublishEvent(g_EVENT_RESP_FIRST, code, std::to_string(result));
}

// level1:global, sync level2:serial, sync
void MainAbility::GlobalCase10(int code)
{
    APP_LOGI("-- -- -- -- -- --MainAbility::%{public}s", __FUNCTION__);
    Reset();
    TestSetting outerSetting;
    TestSetting innerSetting;
    outerSetting.dispatcher = TestDispatcher::GLOBAL;
    innerSetting.dispatcher = TestDispatcher::SERIAL;
    outerSetting.op = TestOperation::SYNC;
    innerSetting.op = TestOperation::SYNC;
    int taskCount = Dispatch(outerSetting, innerSetting);
    bool result = Wait(taskCount);
    result = result && task_execution_sequence.size() > 1;
    if (result) {
        result = result && applyExecuted(outerSetting, innerSetting);
        result = result && OuterTaskExecuted(outerSetting);
        result = result && InnerTaskExecuted(innerSetting);
        std::vector<size_t> outerTaskIndex;
        std::vector<std::vector<size_t>> innerTaskIndex;
        GetTaskIndex(outerTaskIndex, innerTaskIndex);
        result = result && IsAscend(outerTaskIndex);
        for (int i = 0; i < testTaskCount; i++) {
            result = result && IsAscend(innerTaskIndex[i]);
        }
    }
    TestUtils::PublishEvent(g_EVENT_RESP_FIRST, code, std::to_string(result));
}

// level1:global, sync level2:serial, async
void MainAbility::GlobalCase11(int code)
{
    APP_LOGI("-- -- -- -- -- --MainAbility::%{public}s", __FUNCTION__);
    Reset();
    TestSetting outerSetting;
    TestSetting innerSetting;
    outerSetting.dispatcher = TestDispatcher::GLOBAL;
    innerSetting.dispatcher = TestDispatcher::SERIAL;
    outerSetting.op = TestOperation::SYNC;
    innerSetting.op = TestOperation::ASYNC;
    int taskCount = Dispatch(outerSetting, innerSetting);
    bool result = Wait(taskCount);
    result = result && task_execution_sequence.size() > 1;
    if (result) {
        result = result && applyExecuted(outerSetting, innerSetting);
        result = result && OuterTaskExecuted(outerSetting);
        result = result && InnerTaskExecuted(innerSetting);
        std::vector<size_t> outerTaskIndex;
        std::vector<std::vector<size_t>> innerTaskIndex;
        GetTaskIndex(outerTaskIndex, innerTaskIndex);
        result = result && IsAscend(outerTaskIndex);
        for (int i = 0; i < testTaskCount; i++) {
            result = result && IsAscend(innerTaskIndex[i]);
        }
    }
    TestUtils::PublishEvent(g_EVENT_RESP_FIRST, code, std::to_string(result));
}

// level1:global, sync level2:serial, delay
void MainAbility::GlobalCase12(int code)
{
    APP_LOGI("-- -- -- -- -- --MainAbility::%{public}s", __FUNCTION__);
    Reset();
    TestSetting outerSetting;
    TestSetting innerSetting;
    outerSetting.dispatcher = TestDispatcher::GLOBAL;
    innerSetting.dispatcher = TestDispatcher::SERIAL;
    innerSetting.delay = delayMs;
    outerSetting.op = TestOperation::SYNC;
    innerSetting.op = TestOperation::DELAY;
    int taskCount = Dispatch(outerSetting, innerSetting);
    bool result = Wait(taskCount);
    result = result && task_execution_sequence.size() > 1;
    if (result) {
        result = result && applyExecuted(outerSetting, innerSetting);
        result = result && OuterTaskExecuted(outerSetting);
        result = result && InnerTaskExecuted(innerSetting);
        std::vector<size_t> outerTaskIndex;
        std::vector<std::vector<size_t>> innerTaskIndex;
        GetTaskIndex(outerTaskIndex, innerTaskIndex);
        result = result && IsAscend(outerTaskIndex);
        for (int i = 0; i < testTaskCount; i++) {
            result = result && IsAscend(innerTaskIndex[i]);
        }
    }
    TestUtils::PublishEvent(g_EVENT_RESP_FIRST, code, std::to_string(result));
}

// level1:global, sync level2:serial, apply
void MainAbility::GlobalCase13(int code)
{
    APP_LOGI("-- -- -- -- -- --MainAbility::%{public}s", __FUNCTION__);
    Reset();
    TestSetting outerSetting;
    TestSetting innerSetting;
    outerSetting.dispatcher = TestDispatcher::GLOBAL;
    innerSetting.dispatcher = TestDispatcher::SERIAL;
    innerSetting.apply = applyNum;
    outerSetting.op = TestOperation::SYNC;
    innerSetting.op = TestOperation::APPLY;
    int taskCount = Dispatch(outerSetting, innerSetting);
    bool result = Wait(taskCount);
    result = result && task_execution_sequence.size() > 1;
    if (result) {
        result = result && applyExecuted(outerSetting, innerSetting);
        result = result && OuterTaskExecuted(outerSetting);
        result = result && InnerTaskExecuted(innerSetting);
        std::vector<size_t> outerTaskIndex;
        std::vector<std::vector<size_t>> innerTaskIndex;
        GetTaskIndex(outerTaskIndex, innerTaskIndex);
        result = result && IsAscend(outerTaskIndex);
    }
    TestUtils::PublishEvent(g_EVENT_RESP_FIRST, code, std::to_string(result));
}

// level1:global, async level2:parallel, sync
void MainAbility::GlobalCase14(int code)
{
    APP_LOGI("-- -- -- -- -- --MainAbility::%{public}s", __FUNCTION__);
    Reset();
    TestSetting outerSetting;
    TestSetting innerSetting;
    outerSetting.dispatcher = TestDispatcher::GLOBAL;
    innerSetting.dispatcher = TestDispatcher::PARALLEL;
    outerSetting.op = TestOperation::ASYNC;
    innerSetting.op = TestOperation::SYNC;
    int taskCount = Dispatch(outerSetting, innerSetting);
    bool result = Wait(taskCount);
    result = result && task_execution_sequence.size() > 1;
    if (result) {
        result = result && applyExecuted(outerSetting, innerSetting);
        result = result && OuterTaskExecuted(outerSetting);
        result = result && InnerTaskExecuted(innerSetting);
        std::vector<size_t> outerTaskIndex;
        std::vector<std::vector<size_t>> innerTaskIndex;
        GetTaskIndex(outerTaskIndex, innerTaskIndex);
        for (int i = 0; i < testTaskCount; i++) {
            result = result && IsAscend(innerTaskIndex[i]);
        }
    }
    TestUtils::PublishEvent(g_EVENT_RESP_FIRST, code, std::to_string(result));
}

// level1:global, async level2:parallel, async
void MainAbility::GlobalCase15(int code)
{
    APP_LOGI("-- -- -- -- -- --MainAbility::%{public}s", __FUNCTION__);
    Reset();
    TestSetting outerSetting;
    TestSetting innerSetting;
    outerSetting.dispatcher = TestDispatcher::GLOBAL;
    innerSetting.dispatcher = TestDispatcher::PARALLEL;
    outerSetting.op = TestOperation::ASYNC;
    innerSetting.op = TestOperation::ASYNC;
    int taskCount = Dispatch(outerSetting, innerSetting);
    bool result = Wait(taskCount);
    result = result && task_execution_sequence.size() > 1;
    if (result) {
        result = result && applyExecuted(outerSetting, innerSetting);
        result = result && OuterTaskExecuted(outerSetting);
        result = result && InnerTaskExecuted(innerSetting);
    }
    TestUtils::PublishEvent(g_EVENT_RESP_FIRST, code, std::to_string(result));
}

// level1:global, async level2:parallel, delay
void MainAbility::GlobalCase16(int code)
{
    APP_LOGI("-- -- -- -- -- --MainAbility::%{public}s", __FUNCTION__);
    Reset();
    TestSetting outerSetting;
    TestSetting innerSetting;
    outerSetting.dispatcher = TestDispatcher::GLOBAL;
    innerSetting.dispatcher = TestDispatcher::PARALLEL;
    innerSetting.delay = delayMs;
    outerSetting.op = TestOperation::ASYNC;
    innerSetting.op = TestOperation::DELAY;
    int taskCount = Dispatch(outerSetting, innerSetting);
    bool result = Wait(taskCount);
    result = result && task_execution_sequence.size() > 1;
    if (result) {
        result = result && applyExecuted(outerSetting, innerSetting);
        result = result && OuterTaskExecuted(outerSetting);
        result = result && InnerTaskExecuted(innerSetting);
    }
    TestUtils::PublishEvent(g_EVENT_RESP_FIRST, code, std::to_string(result));
}

// level1:global, async level2:parallel, group
void MainAbility::GlobalCase17(int code)
{
    APP_LOGI("-- -- -- -- -- --MainAbility::%{public}s", __FUNCTION__);
    Reset();
    TestSetting outerSetting;
    TestSetting innerSetting;
    outerSetting.dispatcher = TestDispatcher::GLOBAL;
    innerSetting.dispatcher = TestDispatcher::PARALLEL;
    innerSetting.create_group = true;
    outerSetting.op = TestOperation::ASYNC;
    innerSetting.op = TestOperation::ASYNC_GROUP;
    int taskCount = Dispatch(outerSetting, innerSetting);
    bool result = Wait(taskCount);
    result = result && task_execution_sequence.size() > 1;
    if (result) {
        result = result && applyExecuted(outerSetting, innerSetting);
        result = result && OuterTaskExecuted(outerSetting);
        result = result && InnerTaskExecuted(innerSetting);
    }
    TestUtils::PublishEvent(g_EVENT_RESP_FIRST, code, std::to_string(result));
}

// level1:global, async level2:parallel, group wait
void MainAbility::GlobalCase18(int code)
{
    APP_LOGI("-- -- -- -- -- --MainAbility::%{public}s", __FUNCTION__);
    Reset();
    TestSetting outerSetting;
    TestSetting innerSetting;
    outerSetting.dispatcher = TestDispatcher::GLOBAL;
    innerSetting.dispatcher = TestDispatcher::PARALLEL;
    innerSetting.group_wait = true;
    innerSetting.group_timeout = groupWait;
    innerSetting.create_group = true;
    outerSetting.op = TestOperation::ASYNC;
    innerSetting.op = TestOperation::ASYNC_GROUP;
    int taskCount = Dispatch(outerSetting, innerSetting);
    bool result = Wait(taskCount);
    result = result && task_execution_sequence.size() > 1;
    if (result) {
        result = result && applyExecuted(outerSetting, innerSetting);
        result = result && OuterTaskExecuted(outerSetting);
        result = result && InnerTaskExecuted(innerSetting);
    }
    TestUtils::PublishEvent(g_EVENT_RESP_FIRST, code, std::to_string(result));
}

// level1:global, async level2:parallel, group notify
void MainAbility::GlobalCase19(int code)
{
    APP_LOGI("-- -- -- -- -- --MainAbility::%{public}s", __FUNCTION__);
    Reset();
    TestSetting outerSetting;
    TestSetting innerSetting;
    outerSetting.dispatcher = TestDispatcher::GLOBAL;
    innerSetting.dispatcher = TestDispatcher::PARALLEL;
    innerSetting.create_group = true;
    innerSetting.group_notify = true;
    outerSetting.op = TestOperation::ASYNC;
    innerSetting.op = TestOperation::ASYNC_GROUP;
    int taskCount = Dispatch(outerSetting, innerSetting);
    bool result = Wait(taskCount);
    result = result && task_execution_sequence.size() > 1;
    if (result) {
        result = result && applyExecuted(outerSetting, innerSetting);
        result = result && OuterTaskExecuted(outerSetting);
        result = result && InnerTaskExecuted(innerSetting);
        result = result && InnerExecutedAfter(innerGroupNotifyId);
    }
    TestUtils::PublishEvent(g_EVENT_RESP_FIRST, code, std::to_string(result));
}

// level1:global, async level2:parallel, sync barrier
void MainAbility::GlobalCase20(int code)
{
    APP_LOGI("-- -- -- -- -- --MainAbility::%{public}s", __FUNCTION__);
    Reset();
    TestSetting outerSetting;
    TestSetting innerSetting;
    outerSetting.dispatcher = TestDispatcher::GLOBAL;
    innerSetting.dispatcher = TestDispatcher::PARALLEL;
    innerSetting.sync_barrier = true;
    innerSetting.create_group = true;
    outerSetting.op = TestOperation::ASYNC;
    innerSetting.op = TestOperation::ASYNC_GROUP;
    int taskCount = Dispatch(outerSetting, innerSetting);
    bool result = Wait(taskCount);
    result = result && task_execution_sequence.size() > 1;
    if (result) {
        result = result && applyExecuted(outerSetting, innerSetting);
        result = result && OuterTaskExecuted(outerSetting);
        result = result && InnerTaskExecuted(innerSetting);
        result = result && InnerExecutedAfter(innerSyncBarrierId);
    }
    TestUtils::PublishEvent(g_EVENT_RESP_FIRST, code, std::to_string(result));
}

// level1:global, async level2:parallel, async barrier
void MainAbility::GlobalCase21(int code)
{
    APP_LOGI("-- -- -- -- -- --MainAbility::%{public}s", __FUNCTION__);
    Reset();
    TestSetting outerSetting;
    TestSetting innerSetting;
    outerSetting.dispatcher = TestDispatcher::GLOBAL;
    innerSetting.dispatcher = TestDispatcher::PARALLEL;
    innerSetting.async_barrier = true;
    innerSetting.create_group = true;
    outerSetting.op = TestOperation::ASYNC;
    innerSetting.op = TestOperation::ASYNC_GROUP;
    int taskCount = Dispatch(outerSetting, innerSetting);
    bool result = Wait(taskCount);
    result = result && task_execution_sequence.size() > 1;
    if (result) {
        result = result && applyExecuted(outerSetting, innerSetting);
        result = result && OuterTaskExecuted(outerSetting);
        result = result && InnerTaskExecuted(innerSetting);
        result = result && InnerExecutedAfter(innerAsyncBarrierId);
    }
    TestUtils::PublishEvent(g_EVENT_RESP_FIRST, code, std::to_string(result));
}

// level1:global, async level2:parallel, apply
void MainAbility::GlobalCase22(int code)
{
    APP_LOGI("-- -- -- -- -- --MainAbility::%{public}s", __FUNCTION__);
    Reset();
    TestSetting outerSetting;
    TestSetting innerSetting;
    outerSetting.dispatcher = TestDispatcher::GLOBAL;
    innerSetting.dispatcher = TestDispatcher::PARALLEL;
    innerSetting.apply = applyNum;
    outerSetting.op = TestOperation::ASYNC;
    innerSetting.op = TestOperation::APPLY;
    int taskCount = Dispatch(outerSetting, innerSetting);
    bool result = Wait(taskCount);
    result = result && task_execution_sequence.size() > 1;
    if (result) {
        result = result && applyExecuted(outerSetting, innerSetting);
        result = result && OuterTaskExecuted(outerSetting);
        result = result && InnerTaskExecuted(innerSetting);
    }
    TestUtils::PublishEvent(g_EVENT_RESP_FIRST, code, std::to_string(result));
}

// level1:global, async level2:serial, sync
void MainAbility::GlobalCase23(int code)
{
    APP_LOGI("-- -- -- -- -- --MainAbility::%{public}s", __FUNCTION__);
    Reset();
    TestSetting outerSetting;
    TestSetting innerSetting;
    outerSetting.dispatcher = TestDispatcher::GLOBAL;
    innerSetting.dispatcher = TestDispatcher::SERIAL;
    outerSetting.op = TestOperation::ASYNC;
    innerSetting.op = TestOperation::SYNC;
    int taskCount = Dispatch(outerSetting, innerSetting);
    bool result = Wait(taskCount);
    result = result && task_execution_sequence.size() > 1;
    if (result) {
        result = result && applyExecuted(outerSetting, innerSetting);
        result = result && OuterTaskExecuted(outerSetting);
        result = result && InnerTaskExecuted(innerSetting);
        std::vector<size_t> outerTaskIndex;
        std::vector<std::vector<size_t>> innerTaskIndex;
        GetTaskIndex(outerTaskIndex, innerTaskIndex);
        for (int i = 0; i < testTaskCount; i++) {
            result = result && IsAscend(innerTaskIndex[i]);
        }
    }
    TestUtils::PublishEvent(g_EVENT_RESP_FIRST, code, std::to_string(result));
}

// level1:global, async level2:serial, async
void MainAbility::GlobalCase24(int code)
{
    APP_LOGI("-- -- -- -- -- --MainAbility::%{public}s", __FUNCTION__);
    Reset();
    TestSetting outerSetting;
    TestSetting innerSetting;
    outerSetting.dispatcher = TestDispatcher::GLOBAL;
    innerSetting.dispatcher = TestDispatcher::SERIAL;
    outerSetting.op = TestOperation::ASYNC;
    innerSetting.op = TestOperation::ASYNC;
    int taskCount = Dispatch(outerSetting, innerSetting);
    bool result = Wait(taskCount);
    result = result && task_execution_sequence.size() > 1;
    if (result) {
        result = result && applyExecuted(outerSetting, innerSetting);
        result = result && OuterTaskExecuted(outerSetting);
        result = result && InnerTaskExecuted(innerSetting);
        std::vector<size_t> outerTaskIndex;
        std::vector<std::vector<size_t>> innerTaskIndex;
        GetTaskIndex(outerTaskIndex, innerTaskIndex);
        for (int i = 0; i < testTaskCount; i++) {
            result = result && IsAscend(innerTaskIndex[i]);
        }
    }
    TestUtils::PublishEvent(g_EVENT_RESP_FIRST, code, std::to_string(result));
}

// level1:global, async level2:serial, delay
void MainAbility::GlobalCase25(int code)
{
    APP_LOGI("-- -- -- -- -- --MainAbility::%{public}s", __FUNCTION__);
    Reset();
    TestSetting outerSetting;
    TestSetting innerSetting;
    outerSetting.dispatcher = TestDispatcher::GLOBAL;
    innerSetting.dispatcher = TestDispatcher::SERIAL;
    innerSetting.delay = delayMs;
    outerSetting.op = TestOperation::ASYNC;
    innerSetting.op = TestOperation::DELAY;
    int taskCount = Dispatch(outerSetting, innerSetting);
    bool result = Wait(taskCount);
    result = result && task_execution_sequence.size() > 1;
    if (result) {
        result = result && applyExecuted(outerSetting, innerSetting);
        result = result && OuterTaskExecuted(outerSetting);
        result = result && InnerTaskExecuted(innerSetting);
        std::vector<size_t> outerTaskIndex;
        std::vector<std::vector<size_t>> innerTaskIndex;
        GetTaskIndex(outerTaskIndex, innerTaskIndex);
        for (int i = 0; i < testTaskCount; i++) {
            result = result && IsAscend(innerTaskIndex[i]);
        }
    }
    TestUtils::PublishEvent(g_EVENT_RESP_FIRST, code, std::to_string(result));
}

// level1:global, async level2:serial, apply
void MainAbility::GlobalCase26(int code)
{
    APP_LOGI("-- -- -- -- -- --MainAbility::%{public}s", __FUNCTION__);
    Reset();
    TestSetting outerSetting;
    TestSetting innerSetting;
    outerSetting.dispatcher = TestDispatcher::GLOBAL;
    innerSetting.dispatcher = TestDispatcher::SERIAL;
    innerSetting.apply = applyNum;
    outerSetting.op = TestOperation::ASYNC;
    innerSetting.op = TestOperation::APPLY;
    int taskCount = Dispatch(outerSetting, innerSetting);
    bool result = Wait(taskCount);
    result = result && task_execution_sequence.size() > 1;
    if (result) {
        result = result && applyExecuted(outerSetting, innerSetting);
        result = result && OuterTaskExecuted(outerSetting);
        result = result && InnerTaskExecuted(innerSetting);
    }
    TestUtils::PublishEvent(g_EVENT_RESP_FIRST, code, std::to_string(result));
}

// level1:global, delay level2:parallel, sync
void MainAbility::GlobalCase27(int code)
{
    APP_LOGI("-- -- -- -- -- --MainAbility::%{public}s", __FUNCTION__);
    Reset();
    TestSetting outerSetting;
    TestSetting innerSetting;
    outerSetting.dispatcher = TestDispatcher::GLOBAL;
    innerSetting.dispatcher = TestDispatcher::PARALLEL;
    outerSetting.delay = delayMs;
    outerSetting.op = TestOperation::DELAY;
    innerSetting.op = TestOperation::SYNC;
    int taskCount = Dispatch(outerSetting, innerSetting);
    bool result = Wait(taskCount);
    result = result && task_execution_sequence.size() > 1;
    if (result) {
        result = result && applyExecuted(outerSetting, innerSetting);
        result = result && OuterTaskExecuted(outerSetting);
        result = result && InnerTaskExecuted(innerSetting);
        std::vector<size_t> outerTaskIndex;
        std::vector<std::vector<size_t>> innerTaskIndex;
        GetTaskIndex(outerTaskIndex, innerTaskIndex);
        for (int i = 0; i < testTaskCount; i++) {
            result = result && IsAscend(innerTaskIndex[i]);
        }
    }
    TestUtils::PublishEvent(g_EVENT_RESP_FIRST, code, std::to_string(result));
}

// level1:global, delay level2:parallel, async
void MainAbility::GlobalCase28(int code)
{
    APP_LOGI("-- -- -- -- -- --MainAbility::%{public}s", __FUNCTION__);
    Reset();
    TestSetting outerSetting;
    TestSetting innerSetting;
    outerSetting.dispatcher = TestDispatcher::GLOBAL;
    innerSetting.dispatcher = TestDispatcher::PARALLEL;
    outerSetting.delay = delayMs;
    outerSetting.op = TestOperation::DELAY;
    innerSetting.op = TestOperation::ASYNC;
    int taskCount = Dispatch(outerSetting, innerSetting);
    bool result = Wait(taskCount);
    result = result && task_execution_sequence.size() > 1;
    if (result) {
        result = result && applyExecuted(outerSetting, innerSetting);
        result = result && OuterTaskExecuted(outerSetting);
        result = result && InnerTaskExecuted(innerSetting);
    }
    TestUtils::PublishEvent(g_EVENT_RESP_FIRST, code, std::to_string(result));
}

// level1:global, delay level2:parallel, delay
void MainAbility::GlobalCase29(int code)
{
    APP_LOGI("-- -- -- -- -- --MainAbility::%{public}s", __FUNCTION__);
    Reset();
    TestSetting outerSetting;
    TestSetting innerSetting;
    outerSetting.dispatcher = TestDispatcher::GLOBAL;
    innerSetting.dispatcher = TestDispatcher::PARALLEL;
    outerSetting.delay = delayMs;
    innerSetting.delay = delayMs;
    outerSetting.op = TestOperation::DELAY;
    innerSetting.op = TestOperation::DELAY;
    int taskCount = Dispatch(outerSetting, innerSetting);
    bool result = Wait(taskCount);
    result = result && task_execution_sequence.size() > 1;
    if (result) {
        result = result && applyExecuted(outerSetting, innerSetting);
        result = result && OuterTaskExecuted(outerSetting);
        result = result && InnerTaskExecuted(innerSetting);
    }
    TestUtils::PublishEvent(g_EVENT_RESP_FIRST, code, std::to_string(result));
}

// level1:global, delay level2:parallel, group
void MainAbility::GlobalCase30(int code)
{
    APP_LOGI("-- -- -- -- -- --MainAbility::%{public}s", __FUNCTION__);
    Reset();
    TestSetting outerSetting;
    TestSetting innerSetting;
    outerSetting.dispatcher = TestDispatcher::GLOBAL;
    innerSetting.dispatcher = TestDispatcher::PARALLEL;
    outerSetting.delay = delayMs;
    innerSetting.create_group = true;
    outerSetting.op = TestOperation::DELAY;
    innerSetting.op = TestOperation::ASYNC_GROUP;
    int taskCount = Dispatch(outerSetting, innerSetting);
    bool result = Wait(taskCount);
    result = result && task_execution_sequence.size() > 1;
    if (result) {
        result = result && applyExecuted(outerSetting, innerSetting);
        result = result && OuterTaskExecuted(outerSetting);
        result = result && InnerTaskExecuted(innerSetting);
    }
    TestUtils::PublishEvent(g_EVENT_RESP_FIRST, code, std::to_string(result));
}

// level1:global, delay level2:parallel, group wait
void MainAbility::GlobalCase31(int code)
{
    APP_LOGI("-- -- -- -- -- --MainAbility::%{public}s", __FUNCTION__);
    Reset();
    TestSetting outerSetting;
    TestSetting innerSetting;
    outerSetting.dispatcher = TestDispatcher::GLOBAL;
    innerSetting.dispatcher = TestDispatcher::PARALLEL;
    innerSetting.group_wait = true;
    innerSetting.group_timeout = groupWait;
    outerSetting.delay = delayMs;
    innerSetting.create_group = true;
    outerSetting.op = TestOperation::DELAY;
    innerSetting.op = TestOperation::ASYNC_GROUP;
    int taskCount = Dispatch(outerSetting, innerSetting);
    bool result = Wait(taskCount);
    result = result && task_execution_sequence.size() > 1;
    if (result) {
        result = result && applyExecuted(outerSetting, innerSetting);
        result = result && OuterTaskExecuted(outerSetting);
        result = result && InnerTaskExecuted(innerSetting);
    }
    TestUtils::PublishEvent(g_EVENT_RESP_FIRST, code, std::to_string(result));
}

// level1:global, delay level2:parallel, group notify
void MainAbility::GlobalCase32(int code)
{
    APP_LOGI("-- -- -- -- -- --MainAbility::%{public}s", __FUNCTION__);
    Reset();
    TestSetting outerSetting;
    TestSetting innerSetting;
    outerSetting.dispatcher = TestDispatcher::GLOBAL;
    innerSetting.dispatcher = TestDispatcher::PARALLEL;
    outerSetting.delay = delayMs;
    innerSetting.create_group = true;
    innerSetting.group_notify = true;
    outerSetting.op = TestOperation::DELAY;
    innerSetting.op = TestOperation::ASYNC_GROUP;
    int taskCount = Dispatch(outerSetting, innerSetting);
    bool result = Wait(taskCount);
    result = result && task_execution_sequence.size() > 1;
    if (result) {
        result = result && applyExecuted(outerSetting, innerSetting);
        result = result && OuterTaskExecuted(outerSetting);
        result = result && InnerTaskExecuted(innerSetting);
        result = result && InnerExecutedAfter(innerGroupNotifyId);
    }
    TestUtils::PublishEvent(g_EVENT_RESP_FIRST, code, std::to_string(result));
}

// level1:global, delay level2:parallel, sync barrier
void MainAbility::GlobalCase33(int code)
{
    APP_LOGI("-- -- -- -- -- --MainAbility::%{public}s", __FUNCTION__);
    Reset();
    TestSetting outerSetting;
    TestSetting innerSetting;
    outerSetting.dispatcher = TestDispatcher::GLOBAL;
    innerSetting.dispatcher = TestDispatcher::PARALLEL;
    outerSetting.delay = delayMs;
    innerSetting.sync_barrier = true;
    innerSetting.create_group = true;
    outerSetting.op = TestOperation::DELAY;
    innerSetting.op = TestOperation::ASYNC_GROUP;
    int taskCount = Dispatch(outerSetting, innerSetting);
    bool result = Wait(taskCount);
    result = result && task_execution_sequence.size() > 1;
    if (result) {
        result = result && applyExecuted(outerSetting, innerSetting);
        result = result && OuterTaskExecuted(outerSetting);
        result = result && InnerTaskExecuted(innerSetting);
        result = result && InnerExecutedAfter(innerSyncBarrierId);
    }
    TestUtils::PublishEvent(g_EVENT_RESP_FIRST, code, std::to_string(result));
}

// level1:global, delay level2:parallel, async barrier
void MainAbility::GlobalCase34(int code)
{
    APP_LOGI("-- -- -- -- -- --MainAbility::%{public}s", __FUNCTION__);
    Reset();
    TestSetting outerSetting;
    TestSetting innerSetting;
    outerSetting.dispatcher = TestDispatcher::GLOBAL;
    innerSetting.dispatcher = TestDispatcher::PARALLEL;
    outerSetting.delay = delayMs;
    innerSetting.async_barrier = true;
    innerSetting.create_group = true;
    outerSetting.op = TestOperation::DELAY;
    innerSetting.op = TestOperation::ASYNC_GROUP;
    int taskCount = Dispatch(outerSetting, innerSetting);
    bool result = Wait(taskCount);
    result = result && task_execution_sequence.size() > 1;
    if (result) {
        result = result && applyExecuted(outerSetting, innerSetting);
        result = result && OuterTaskExecuted(outerSetting);
        result = result && InnerTaskExecuted(innerSetting);
        result = result && InnerExecutedAfter(innerAsyncBarrierId);
    }
    TestUtils::PublishEvent(g_EVENT_RESP_FIRST, code, std::to_string(result));
}

// level1:global, delay level2:parallel, apply
void MainAbility::GlobalCase35(int code)
{
    APP_LOGI("-- -- -- -- -- --MainAbility::%{public}s", __FUNCTION__);
    Reset();
    TestSetting outerSetting;
    TestSetting innerSetting;
    outerSetting.dispatcher = TestDispatcher::GLOBAL;
    innerSetting.dispatcher = TestDispatcher::PARALLEL;
    outerSetting.delay = delayMs;
    innerSetting.apply = applyNum;
    outerSetting.op = TestOperation::DELAY;
    innerSetting.op = TestOperation::APPLY;
    int taskCount = Dispatch(outerSetting, innerSetting);
    bool result = Wait(taskCount);
    result = result && task_execution_sequence.size() > 1;
    if (result) {
        result = result && applyExecuted(outerSetting, innerSetting);
        result = result && OuterTaskExecuted(outerSetting);
        result = result && InnerTaskExecuted(innerSetting);
    }
    TestUtils::PublishEvent(g_EVENT_RESP_FIRST, code, std::to_string(result));
}

// level1:global, delay level2:serial, sync
void MainAbility::GlobalCase36(int code)
{
    APP_LOGI("-- -- -- -- -- --MainAbility::%{public}s", __FUNCTION__);
    Reset();
    TestSetting outerSetting;
    TestSetting innerSetting;
    outerSetting.dispatcher = TestDispatcher::GLOBAL;
    innerSetting.dispatcher = TestDispatcher::SERIAL;
    outerSetting.delay = delayMs;
    outerSetting.op = TestOperation::DELAY;
    innerSetting.op = TestOperation::SYNC;
    int taskCount = Dispatch(outerSetting, innerSetting);
    bool result = Wait(taskCount);
    result = result && task_execution_sequence.size() > 1;
    if (result) {
        result = result && applyExecuted(outerSetting, innerSetting);
        result = result && OuterTaskExecuted(outerSetting);
        result = result && InnerTaskExecuted(innerSetting);
        std::vector<size_t> outerTaskIndex;
        std::vector<std::vector<size_t>> innerTaskIndex;
        GetTaskIndex(outerTaskIndex, innerTaskIndex);
        for (int i = 0; i < testTaskCount; i++) {
            result = result && IsAscend(innerTaskIndex[i]);
        }
    }
    TestUtils::PublishEvent(g_EVENT_RESP_FIRST, code, std::to_string(result));
}

// level1:global, delay level2:serial, async
void MainAbility::GlobalCase37(int code)
{
    APP_LOGI("-- -- -- -- -- --MainAbility::%{public}s", __FUNCTION__);
    Reset();
    TestSetting outerSetting;
    TestSetting innerSetting;
    outerSetting.dispatcher = TestDispatcher::GLOBAL;
    innerSetting.dispatcher = TestDispatcher::SERIAL;
    outerSetting.delay = delayMs;
    outerSetting.op = TestOperation::DELAY;
    innerSetting.op = TestOperation::ASYNC;
    int taskCount = Dispatch(outerSetting, innerSetting);
    bool result = Wait(taskCount);
    result = result && task_execution_sequence.size() > 1;
    if (result) {
        result = result && applyExecuted(outerSetting, innerSetting);
        result = result && OuterTaskExecuted(outerSetting);
        result = result && InnerTaskExecuted(innerSetting);
        std::vector<size_t> outerTaskIndex;
        std::vector<std::vector<size_t>> innerTaskIndex;
        GetTaskIndex(outerTaskIndex, innerTaskIndex);
        for (int i = 0; i < testTaskCount; i++) {
            result = result && IsAscend(innerTaskIndex[i]);
        }
    }
    TestUtils::PublishEvent(g_EVENT_RESP_FIRST, code, std::to_string(result));
}

// level1:global, delay level2:serial, delay
void MainAbility::GlobalCase38(int code)
{
    APP_LOGI("-- -- -- -- -- --MainAbility::%{public}s", __FUNCTION__);
    Reset();
    TestSetting outerSetting;
    TestSetting innerSetting;
    outerSetting.dispatcher = TestDispatcher::GLOBAL;
    innerSetting.dispatcher = TestDispatcher::SERIAL;
    outerSetting.delay = delayMs;
    innerSetting.delay = delayMs;
    outerSetting.op = TestOperation::DELAY;
    innerSetting.op = TestOperation::DELAY;
    int taskCount = Dispatch(outerSetting, innerSetting);
    bool result = Wait(taskCount);
    result = result && task_execution_sequence.size() > 1;
    if (result) {
        result = result && applyExecuted(outerSetting, innerSetting);
        result = result && OuterTaskExecuted(outerSetting);
        result = result && InnerTaskExecuted(innerSetting);
        std::vector<size_t> outerTaskIndex;
        std::vector<std::vector<size_t>> innerTaskIndex;
        GetTaskIndex(outerTaskIndex, innerTaskIndex);
        for (int i = 0; i < testTaskCount; i++) {
            result = result && IsAscend(innerTaskIndex[i]);
        }
    }
    TestUtils::PublishEvent(g_EVENT_RESP_FIRST, code, std::to_string(result));
}

// level1:global, delay level2:serial, apply
void MainAbility::GlobalCase39(int code)
{
    APP_LOGI("-- -- -- -- -- --MainAbility::%{public}s", __FUNCTION__);
    Reset();
    TestSetting outerSetting;
    TestSetting innerSetting;
    outerSetting.dispatcher = TestDispatcher::GLOBAL;
    innerSetting.dispatcher = TestDispatcher::SERIAL;
    outerSetting.delay = delayMs;
    innerSetting.apply = applyNum;
    outerSetting.op = TestOperation::DELAY;
    innerSetting.op = TestOperation::APPLY;
    int taskCount = Dispatch(outerSetting, innerSetting);
    bool result = Wait(taskCount);
    result = result && task_execution_sequence.size() > 1;
    if (result) {
        result = result && applyExecuted(outerSetting, innerSetting);
        result = result && OuterTaskExecuted(outerSetting);
        result = result && InnerTaskExecuted(innerSetting);
    }
    TestUtils::PublishEvent(g_EVENT_RESP_FIRST, code, std::to_string(result));
}

// level1:global, group level2:parallel, sync
void MainAbility::GlobalCase40(int code)
{
    APP_LOGI("-- -- -- -- -- --MainAbility::%{public}s", __FUNCTION__);
    Reset();
    TestSetting outerSetting;
    TestSetting innerSetting;
    outerSetting.dispatcher = TestDispatcher::GLOBAL;
    innerSetting.dispatcher = TestDispatcher::PARALLEL;
    outerSetting.create_group = true;
    outerSetting.op = TestOperation::ASYNC_GROUP;
    innerSetting.op = TestOperation::SYNC;
    int taskCount = Dispatch(outerSetting, innerSetting);
    bool result = Wait(taskCount);
    result = result && task_execution_sequence.size() > 1;
    if (result) {
        result = result && applyExecuted(outerSetting, innerSetting);
        result = result && OuterTaskExecuted(outerSetting);
        result = result && InnerTaskExecuted(innerSetting);
        std::vector<size_t> outerTaskIndex;
        std::vector<std::vector<size_t>> innerTaskIndex;
        GetTaskIndex(outerTaskIndex, innerTaskIndex);
        for (int i = 0; i < testTaskCount; i++) {
            result = result && IsAscend(innerTaskIndex[i]);
        }
    }
    TestUtils::PublishEvent(g_EVENT_RESP_FIRST, code, std::to_string(result));
}

// level1:global, group level2:parallel, async
void MainAbility::GlobalCase41(int code)
{
    APP_LOGI("-- -- -- -- -- --MainAbility::%{public}s", __FUNCTION__);
    Reset();
    TestSetting outerSetting;
    TestSetting innerSetting;
    outerSetting.dispatcher = TestDispatcher::GLOBAL;
    innerSetting.dispatcher = TestDispatcher::PARALLEL;
    outerSetting.create_group = true;
    outerSetting.op = TestOperation::ASYNC_GROUP;
    innerSetting.op = TestOperation::ASYNC;
    int taskCount = Dispatch(outerSetting, innerSetting);
    bool result = Wait(taskCount);
    result = result && task_execution_sequence.size() > 1;
    if (result) {
        result = result && applyExecuted(outerSetting, innerSetting);
        result = result && OuterTaskExecuted(outerSetting);
        result = result && InnerTaskExecuted(innerSetting);
    }
    TestUtils::PublishEvent(g_EVENT_RESP_FIRST, code, std::to_string(result));
}

// level1:global, group level2:parallel, delay
void MainAbility::GlobalCase42(int code)
{
    APP_LOGI("-- -- -- -- -- --MainAbility::%{public}s", __FUNCTION__);
    Reset();
    TestSetting outerSetting;
    TestSetting innerSetting;
    outerSetting.dispatcher = TestDispatcher::GLOBAL;
    innerSetting.dispatcher = TestDispatcher::PARALLEL;
    innerSetting.delay = delayMs;
    outerSetting.create_group = true;
    outerSetting.op = TestOperation::ASYNC_GROUP;
    innerSetting.op = TestOperation::DELAY;
    int taskCount = Dispatch(outerSetting, innerSetting);
    bool result = Wait(taskCount);
    result = result && task_execution_sequence.size() > 1;
    if (result) {
        result = result && applyExecuted(outerSetting, innerSetting);
        result = result && OuterTaskExecuted(outerSetting);
        result = result && InnerTaskExecuted(innerSetting);
    }
    TestUtils::PublishEvent(g_EVENT_RESP_FIRST, code, std::to_string(result));
}

// level1:global, group level2:parallel, group
void MainAbility::GlobalCase43(int code)
{
    APP_LOGI("-- -- -- -- -- --MainAbility::%{public}s", __FUNCTION__);
    Reset();
    TestSetting outerSetting;
    TestSetting innerSetting;
    outerSetting.dispatcher = TestDispatcher::GLOBAL;
    innerSetting.dispatcher = TestDispatcher::PARALLEL;
    innerSetting.create_group = true;
    outerSetting.create_group = true;
    outerSetting.op = TestOperation::ASYNC_GROUP;
    innerSetting.op = TestOperation::ASYNC_GROUP;
    int taskCount = Dispatch(outerSetting, innerSetting);
    bool result = Wait(taskCount);
    result = result && task_execution_sequence.size() > 1;
    if (result) {
        result = result && applyExecuted(outerSetting, innerSetting);
        result = result && OuterTaskExecuted(outerSetting);
        result = result && InnerTaskExecuted(innerSetting);
    }
    TestUtils::PublishEvent(g_EVENT_RESP_FIRST, code, std::to_string(result));
}

// level1:global, group level2:parallel, group wait
void MainAbility::GlobalCase44(int code)
{
    APP_LOGI("-- -- -- -- -- --MainAbility::%{public}s", __FUNCTION__);
    Reset();
    TestSetting outerSetting;
    TestSetting innerSetting;
    outerSetting.dispatcher = TestDispatcher::GLOBAL;
    innerSetting.dispatcher = TestDispatcher::PARALLEL;
    innerSetting.group_wait = true;
    innerSetting.group_timeout = groupWait;
    innerSetting.create_group = true;
    outerSetting.create_group = true;
    outerSetting.op = TestOperation::ASYNC_GROUP;
    innerSetting.op = TestOperation::ASYNC_GROUP;
    int taskCount = Dispatch(outerSetting, innerSetting);
    bool result = Wait(taskCount);
    result = result && task_execution_sequence.size() > 1;
    if (result) {
        result = result && applyExecuted(outerSetting, innerSetting);
        result = result && OuterTaskExecuted(outerSetting);
        result = result && InnerTaskExecuted(innerSetting);
    }
    TestUtils::PublishEvent(g_EVENT_RESP_FIRST, code, std::to_string(result));
}

// level1:global, group level2:parallel, group notify
void MainAbility::GlobalCase45(int code)
{
    APP_LOGI("-- -- -- -- -- --MainAbility::%{public}s", __FUNCTION__);
    Reset();
    TestSetting outerSetting;
    TestSetting innerSetting;
    outerSetting.dispatcher = TestDispatcher::GLOBAL;
    innerSetting.dispatcher = TestDispatcher::PARALLEL;
    innerSetting.create_group = true;
    innerSetting.group_notify = true;
    outerSetting.create_group = true;
    outerSetting.op = TestOperation::ASYNC_GROUP;
    innerSetting.op = TestOperation::ASYNC_GROUP;
    int taskCount = Dispatch(outerSetting, innerSetting);
    bool result = Wait(taskCount);
    result = result && task_execution_sequence.size() > 1;
    if (result) {
        result = result && applyExecuted(outerSetting, innerSetting);
        result = result && OuterTaskExecuted(outerSetting);
        result = result && InnerTaskExecuted(innerSetting);
        result = result && InnerExecutedAfter(innerGroupNotifyId);
    }
    TestUtils::PublishEvent(g_EVENT_RESP_FIRST, code, std::to_string(result));
}

// level1:global, group level2:parallel, sync barrier
void MainAbility::GlobalCase46(int code)
{
    APP_LOGI("-- -- -- -- -- --MainAbility::%{public}s", __FUNCTION__);
    Reset();
    TestSetting outerSetting;
    TestSetting innerSetting;
    outerSetting.dispatcher = TestDispatcher::GLOBAL;
    innerSetting.dispatcher = TestDispatcher::PARALLEL;
    innerSetting.sync_barrier = true;
    innerSetting.create_group = true;
    outerSetting.create_group = true;
    outerSetting.op = TestOperation::ASYNC_GROUP;
    innerSetting.op = TestOperation::ASYNC_GROUP;
    int taskCount = Dispatch(outerSetting, innerSetting);
    bool result = Wait(taskCount);
    result = result && task_execution_sequence.size() > 1;
    if (result) {
        result = result && applyExecuted(outerSetting, innerSetting);
        result = result && OuterTaskExecuted(outerSetting);
        result = result && InnerTaskExecuted(innerSetting);
        result = result && InnerExecutedAfter(innerSyncBarrierId);
    }
    TestUtils::PublishEvent(g_EVENT_RESP_FIRST, code, std::to_string(result));
}

// level1:global, group level2:parallel, async barrier
void MainAbility::GlobalCase47(int code)
{
    APP_LOGI("-- -- -- -- -- --MainAbility::%{public}s", __FUNCTION__);
    Reset();
    TestSetting outerSetting;
    TestSetting innerSetting;
    outerSetting.dispatcher = TestDispatcher::GLOBAL;
    innerSetting.dispatcher = TestDispatcher::PARALLEL;
    innerSetting.async_barrier = true;
    innerSetting.create_group = true;
    outerSetting.create_group = true;
    outerSetting.op = TestOperation::ASYNC_GROUP;
    innerSetting.op = TestOperation::ASYNC_GROUP;
    int taskCount = Dispatch(outerSetting, innerSetting);
    bool result = Wait(taskCount);
    result = result && task_execution_sequence.size() > 1;
    if (result) {
        result = result && applyExecuted(outerSetting, innerSetting);
        result = result && OuterTaskExecuted(outerSetting);
        result = result && InnerTaskExecuted(innerSetting);
        result = result && InnerExecutedAfter(innerAsyncBarrierId);
    }
    TestUtils::PublishEvent(g_EVENT_RESP_FIRST, code, std::to_string(result));
}

// level1:global, group level2:parallel, apply
void MainAbility::GlobalCase48(int code)
{
    APP_LOGI("-- -- -- -- -- --MainAbility::%{public}s", __FUNCTION__);
    Reset();
    TestSetting outerSetting;
    TestSetting innerSetting;
    outerSetting.dispatcher = TestDispatcher::GLOBAL;
    innerSetting.dispatcher = TestDispatcher::PARALLEL;
    innerSetting.apply = applyNum;
    outerSetting.create_group = true;
    outerSetting.op = TestOperation::ASYNC_GROUP;
    innerSetting.op = TestOperation::APPLY;
    int taskCount = Dispatch(outerSetting, innerSetting);
    bool result = Wait(taskCount);
    result = result && task_execution_sequence.size() > 1;
    if (result) {
        result = result && applyExecuted(outerSetting, innerSetting);
        result = result && OuterTaskExecuted(outerSetting);
        result = result && InnerTaskExecuted(innerSetting);
    }
    TestUtils::PublishEvent(g_EVENT_RESP_FIRST, code, std::to_string(result));
}

// level1:global, group level2:serial, sync
void MainAbility::GlobalCase49(int code)
{
    APP_LOGI("-- -- -- -- -- --MainAbility::%{public}s", __FUNCTION__);
    Reset();
    TestSetting outerSetting;
    TestSetting innerSetting;
    outerSetting.dispatcher = TestDispatcher::GLOBAL;
    innerSetting.dispatcher = TestDispatcher::SERIAL;
    outerSetting.create_group = true;
    outerSetting.op = TestOperation::ASYNC_GROUP;
    innerSetting.op = TestOperation::SYNC;
    int taskCount = Dispatch(outerSetting, innerSetting);
    bool result = Wait(taskCount);
    result = result && task_execution_sequence.size() > 1;
    if (result) {
        result = result && applyExecuted(outerSetting, innerSetting);
        result = result && OuterTaskExecuted(outerSetting);
        result = result && InnerTaskExecuted(innerSetting);
        std::vector<size_t> outerTaskIndex;
        std::vector<std::vector<size_t>> innerTaskIndex;
        GetTaskIndex(outerTaskIndex, innerTaskIndex);
        for (int i = 0; i < testTaskCount; i++) {
            result = result && IsAscend(innerTaskIndex[i]);
        }
    }
    TestUtils::PublishEvent(g_EVENT_RESP_FIRST, code, std::to_string(result));
}

// level1:global, group level2:serial, async
void MainAbility::GlobalCase50(int code)
{
    APP_LOGI("-- -- -- -- -- --MainAbility::%{public}s", __FUNCTION__);
    Reset();
    TestSetting outerSetting;
    TestSetting innerSetting;
    outerSetting.dispatcher = TestDispatcher::GLOBAL;
    innerSetting.dispatcher = TestDispatcher::SERIAL;
    outerSetting.create_group = true;
    outerSetting.op = TestOperation::ASYNC_GROUP;
    innerSetting.op = TestOperation::ASYNC;
    int taskCount = Dispatch(outerSetting, innerSetting);
    bool result = Wait(taskCount);
    result = result && task_execution_sequence.size() > 1;
    if (result) {
        result = result && applyExecuted(outerSetting, innerSetting);
        result = result && OuterTaskExecuted(outerSetting);
        result = result && InnerTaskExecuted(innerSetting);
        std::vector<size_t> outerTaskIndex;
        std::vector<std::vector<size_t>> innerTaskIndex;
        GetTaskIndex(outerTaskIndex, innerTaskIndex);
        for (int i = 0; i < testTaskCount; i++) {
            result = result && IsAscend(innerTaskIndex[i]);
        }
    }
    TestUtils::PublishEvent(g_EVENT_RESP_FIRST, code, std::to_string(result));
}

// level1:global, group level2:serial, delay
void MainAbility::GlobalCase51(int code)
{
    APP_LOGI("-- -- -- -- -- --MainAbility::%{public}s", __FUNCTION__);
    Reset();
    TestSetting outerSetting;
    TestSetting innerSetting;
    outerSetting.dispatcher = TestDispatcher::GLOBAL;
    innerSetting.dispatcher = TestDispatcher::SERIAL;
    innerSetting.delay = delayMs;
    outerSetting.create_group = true;
    outerSetting.op = TestOperation::ASYNC_GROUP;
    innerSetting.op = TestOperation::DELAY;
    int taskCount = Dispatch(outerSetting, innerSetting);
    bool result = Wait(taskCount);
    result = result && task_execution_sequence.size() > 1;
    if (result) {
        result = result && applyExecuted(outerSetting, innerSetting);
        result = result && OuterTaskExecuted(outerSetting);
        result = result && InnerTaskExecuted(innerSetting);
        std::vector<size_t> outerTaskIndex;
        std::vector<std::vector<size_t>> innerTaskIndex;
        GetTaskIndex(outerTaskIndex, innerTaskIndex);
        for (int i = 0; i < testTaskCount; i++) {
            result = result && IsAscend(innerTaskIndex[i]);
        }
    }
    TestUtils::PublishEvent(g_EVENT_RESP_FIRST, code, std::to_string(result));
}

// level1:global, group level2:serial, apply
void MainAbility::GlobalCase52(int code)
{
    APP_LOGI("-- -- -- -- -- --MainAbility::%{public}s", __FUNCTION__);
    Reset();
    TestSetting outerSetting;
    TestSetting innerSetting;
    outerSetting.dispatcher = TestDispatcher::GLOBAL;
    innerSetting.dispatcher = TestDispatcher::SERIAL;
    innerSetting.apply = applyNum;
    outerSetting.create_group = true;
    outerSetting.op = TestOperation::ASYNC_GROUP;
    innerSetting.op = TestOperation::APPLY;
    int taskCount = Dispatch(outerSetting, innerSetting);
    bool result = Wait(taskCount);
    result = result && task_execution_sequence.size() > 1;
    if (result) {
        result = result && applyExecuted(outerSetting, innerSetting);
        result = result && OuterTaskExecuted(outerSetting);
        result = result && InnerTaskExecuted(innerSetting);
    }
    TestUtils::PublishEvent(g_EVENT_RESP_FIRST, code, std::to_string(result));
}

// level1:global, group wait level2:parallel, sync
void MainAbility::GlobalCase53(int code)
{
    APP_LOGI("-- -- -- -- -- --MainAbility::%{public}s", __FUNCTION__);
    Reset();
    TestSetting outerSetting;
    TestSetting innerSetting;
    outerSetting.dispatcher = TestDispatcher::GLOBAL;
    innerSetting.dispatcher = TestDispatcher::PARALLEL;
    outerSetting.group_wait = true;
    outerSetting.group_timeout = groupWait;
    outerSetting.create_group = true;
    outerSetting.op = TestOperation::ASYNC_GROUP;
    innerSetting.op = TestOperation::SYNC;
    int taskCount = Dispatch(outerSetting, innerSetting);
    bool result = Wait(taskCount);
    result = result && task_execution_sequence.size() > 1;
    if (result) {
        result = result && applyExecuted(outerSetting, innerSetting);
        result = result && OuterTaskExecuted(outerSetting);
        result = result && InnerTaskExecuted(innerSetting);
        std::vector<size_t> outerTaskIndex;
        std::vector<std::vector<size_t>> innerTaskIndex;
        GetTaskIndex(outerTaskIndex, innerTaskIndex);
        for (int i = 0; i < testTaskCount; i++) {
            result = result && IsAscend(innerTaskIndex[i]);
        }
    }
    TestUtils::PublishEvent(g_EVENT_RESP_FIRST, code, std::to_string(result));
}

// level1:global, group wait level2:parallel, async
void MainAbility::GlobalCase54(int code)
{
    APP_LOGI("-- -- -- -- -- --MainAbility::%{public}s", __FUNCTION__);
    Reset();
    TestSetting outerSetting;
    TestSetting innerSetting;
    outerSetting.dispatcher = TestDispatcher::GLOBAL;
    innerSetting.dispatcher = TestDispatcher::PARALLEL;
    outerSetting.group_wait = true;
    outerSetting.group_timeout = groupWait;
    outerSetting.create_group = true;
    outerSetting.op = TestOperation::ASYNC_GROUP;
    innerSetting.op = TestOperation::ASYNC;
    int taskCount = Dispatch(outerSetting, innerSetting);
    bool result = Wait(taskCount);
    result = result && task_execution_sequence.size() > 1;
    if (result) {
        result = result && applyExecuted(outerSetting, innerSetting);
        result = result && OuterTaskExecuted(outerSetting);
        result = result && InnerTaskExecuted(innerSetting);
    }
    TestUtils::PublishEvent(g_EVENT_RESP_FIRST, code, std::to_string(result));
}

// level1:global, group wait level2:parallel, delay
void MainAbility::GlobalCase55(int code)
{
    APP_LOGI("-- -- -- -- -- --MainAbility::%{public}s", __FUNCTION__);
    Reset();
    TestSetting outerSetting;
    TestSetting innerSetting;
    outerSetting.dispatcher = TestDispatcher::GLOBAL;
    innerSetting.dispatcher = TestDispatcher::PARALLEL;
    outerSetting.group_wait = true;
    outerSetting.group_timeout = groupWait;
    innerSetting.delay = delayMs;
    outerSetting.create_group = true;
    outerSetting.op = TestOperation::ASYNC_GROUP;
    innerSetting.op = TestOperation::DELAY;
    int taskCount = Dispatch(outerSetting, innerSetting);
    bool result = Wait(taskCount);
    result = result && task_execution_sequence.size() > 1;
    if (result) {
        result = result && applyExecuted(outerSetting, innerSetting);
        result = result && OuterTaskExecuted(outerSetting);
        result = result && InnerTaskExecuted(innerSetting);
    }
    TestUtils::PublishEvent(g_EVENT_RESP_FIRST, code, std::to_string(result));
}

// level1:global, group wait level2:parallel, group
void MainAbility::GlobalCase56(int code)
{
    APP_LOGI("-- -- -- -- -- --MainAbility::%{public}s", __FUNCTION__);
    Reset();
    TestSetting outerSetting;
    TestSetting innerSetting;
    outerSetting.dispatcher = TestDispatcher::GLOBAL;
    innerSetting.dispatcher = TestDispatcher::PARALLEL;
    outerSetting.group_wait = true;
    outerSetting.group_timeout = groupWait;
    innerSetting.create_group = true;
    outerSetting.create_group = true;
    outerSetting.op = TestOperation::ASYNC_GROUP;
    innerSetting.op = TestOperation::ASYNC_GROUP;
    int taskCount = Dispatch(outerSetting, innerSetting);
    bool result = Wait(taskCount);
    result = result && task_execution_sequence.size() > 1;
    if (result) {
        result = result && applyExecuted(outerSetting, innerSetting);
        result = result && OuterTaskExecuted(outerSetting);
        result = result && InnerTaskExecuted(innerSetting);
    }
    TestUtils::PublishEvent(g_EVENT_RESP_FIRST, code, std::to_string(result));
}

// level1:global, group wait level2:parallel, group wait
void MainAbility::GlobalCase57(int code)
{
    APP_LOGI("-- -- -- -- -- --MainAbility::%{public}s", __FUNCTION__);
    Reset();
    TestSetting outerSetting;
    TestSetting innerSetting;
    outerSetting.dispatcher = TestDispatcher::GLOBAL;
    innerSetting.dispatcher = TestDispatcher::PARALLEL;
    innerSetting.group_wait = true;
    innerSetting.group_timeout = groupWait;
    outerSetting.group_wait = true;
    outerSetting.group_timeout = groupWait;
    innerSetting.create_group = true;
    outerSetting.create_group = true;
    outerSetting.op = TestOperation::ASYNC_GROUP;
    innerSetting.op = TestOperation::ASYNC_GROUP;
    int taskCount = Dispatch(outerSetting, innerSetting);
    bool result = Wait(taskCount);
    result = result && task_execution_sequence.size() > 1;
    if (result) {
        result = result && applyExecuted(outerSetting, innerSetting);
        result = result && OuterTaskExecuted(outerSetting);
        result = result && InnerTaskExecuted(innerSetting);
    }
    TestUtils::PublishEvent(g_EVENT_RESP_FIRST, code, std::to_string(result));
}

// level1:global, group wait level2:parallel, group notify
void MainAbility::GlobalCase58(int code)
{
    APP_LOGI("-- -- -- -- -- --MainAbility::%{public}s", __FUNCTION__);
    Reset();
    TestSetting outerSetting;
    TestSetting innerSetting;
    outerSetting.dispatcher = TestDispatcher::GLOBAL;
    innerSetting.dispatcher = TestDispatcher::PARALLEL;
    outerSetting.group_wait = true;
    outerSetting.group_timeout = groupWait;
    innerSetting.create_group = true;
    innerSetting.group_notify = true;
    outerSetting.create_group = true;
    outerSetting.op = TestOperation::ASYNC_GROUP;
    innerSetting.op = TestOperation::ASYNC_GROUP;
    int taskCount = Dispatch(outerSetting, innerSetting);
    bool result = Wait(taskCount);
    result = result && task_execution_sequence.size() > 1;
    if (result) {
        result = result && applyExecuted(outerSetting, innerSetting);
        result = result && OuterTaskExecuted(outerSetting);
        result = result && InnerTaskExecuted(innerSetting);
        result = result && InnerExecutedAfter(innerGroupNotifyId);
    }
    TestUtils::PublishEvent(g_EVENT_RESP_FIRST, code, std::to_string(result));
}

// level1:global, group wait level2:parallel, sync barrier
void MainAbility::GlobalCase59(int code)
{
    APP_LOGI("-- -- -- -- -- --MainAbility::%{public}s", __FUNCTION__);
    Reset();
    TestSetting outerSetting;
    TestSetting innerSetting;
    outerSetting.dispatcher = TestDispatcher::GLOBAL;
    innerSetting.dispatcher = TestDispatcher::PARALLEL;
    outerSetting.group_wait = true;
    outerSetting.group_timeout = groupWait;
    innerSetting.sync_barrier = true;
    innerSetting.create_group = true;
    outerSetting.create_group = true;
    outerSetting.op = TestOperation::ASYNC_GROUP;
    innerSetting.op = TestOperation::ASYNC_GROUP;
    int taskCount = Dispatch(outerSetting, innerSetting);
    bool result = Wait(taskCount);
    result = result && task_execution_sequence.size() > 1;
    if (result) {
        result = result && applyExecuted(outerSetting, innerSetting);
        result = result && OuterTaskExecuted(outerSetting);
        result = result && InnerTaskExecuted(innerSetting);
        result = result && InnerExecutedAfter(innerSyncBarrierId);
    }
    TestUtils::PublishEvent(g_EVENT_RESP_FIRST, code, std::to_string(result));
}

// level1:global, group wait level2:parallel, async barrier
void MainAbility::GlobalCase60(int code)
{
    APP_LOGI("-- -- -- -- -- --MainAbility::%{public}s", __FUNCTION__);
    Reset();
    TestSetting outerSetting;
    TestSetting innerSetting;
    outerSetting.dispatcher = TestDispatcher::GLOBAL;
    innerSetting.dispatcher = TestDispatcher::PARALLEL;
    outerSetting.group_wait = true;
    outerSetting.group_timeout = groupWait;
    innerSetting.async_barrier = true;
    innerSetting.create_group = true;
    outerSetting.create_group = true;
    outerSetting.op = TestOperation::ASYNC_GROUP;
    innerSetting.op = TestOperation::ASYNC_GROUP;
    int taskCount = Dispatch(outerSetting, innerSetting);
    bool result = Wait(taskCount);
    result = result && task_execution_sequence.size() > 1;
    if (result) {
        result = result && applyExecuted(outerSetting, innerSetting);
        result = result && OuterTaskExecuted(outerSetting);
        result = result && InnerTaskExecuted(innerSetting);
        result = result && InnerExecutedAfter(innerAsyncBarrierId);
    }
    TestUtils::PublishEvent(g_EVENT_RESP_FIRST, code, std::to_string(result));
}

// level1:global, group wait level2:parallel, apply
void MainAbility::GlobalCase61(int code)
{
    APP_LOGI("-- -- -- -- -- --MainAbility::%{public}s", __FUNCTION__);
    Reset();
    TestSetting outerSetting;
    TestSetting innerSetting;
    outerSetting.dispatcher = TestDispatcher::GLOBAL;
    innerSetting.dispatcher = TestDispatcher::PARALLEL;
    outerSetting.group_wait = true;
    outerSetting.group_timeout = groupWait;
    innerSetting.apply = applyNum;
    outerSetting.create_group = true;
    outerSetting.op = TestOperation::ASYNC_GROUP;
    innerSetting.op = TestOperation::APPLY;
    int taskCount = Dispatch(outerSetting, innerSetting);
    bool result = Wait(taskCount);
    result = result && task_execution_sequence.size() > 1;
    if (result) {
        result = result && applyExecuted(outerSetting, innerSetting);
        result = result && OuterTaskExecuted(outerSetting);
        result = result && InnerTaskExecuted(innerSetting);
    }
    TestUtils::PublishEvent(g_EVENT_RESP_FIRST, code, std::to_string(result));
}

// level1:global, group wait level2:serial, sync
void MainAbility::GlobalCase62(int code)
{
    APP_LOGI("-- -- -- -- -- --MainAbility::%{public}s", __FUNCTION__);
    Reset();
    TestSetting outerSetting;
    TestSetting innerSetting;
    outerSetting.dispatcher = TestDispatcher::GLOBAL;
    innerSetting.dispatcher = TestDispatcher::SERIAL;
    outerSetting.group_wait = true;
    outerSetting.group_timeout = groupWait;
    outerSetting.create_group = true;
    outerSetting.op = TestOperation::ASYNC_GROUP;
    innerSetting.op = TestOperation::SYNC;
    int taskCount = Dispatch(outerSetting, innerSetting);
    bool result = Wait(taskCount);
    result = result && task_execution_sequence.size() > 1;
    if (result) {
        result = result && applyExecuted(outerSetting, innerSetting);
        result = result && OuterTaskExecuted(outerSetting);
        result = result && InnerTaskExecuted(innerSetting);
        std::vector<size_t> outerTaskIndex;
        std::vector<std::vector<size_t>> innerTaskIndex;
        GetTaskIndex(outerTaskIndex, innerTaskIndex);
        for (int i = 0; i < testTaskCount; i++) {
            result = result && IsAscend(innerTaskIndex[i]);
        }
    }
    TestUtils::PublishEvent(g_EVENT_RESP_FIRST, code, std::to_string(result));
}

// level1:global, group wait level2:serial, async
void MainAbility::GlobalCase63(int code)
{
    APP_LOGI("-- -- -- -- -- --MainAbility::%{public}s", __FUNCTION__);
    Reset();
    TestSetting outerSetting;
    TestSetting innerSetting;
    outerSetting.dispatcher = TestDispatcher::GLOBAL;
    innerSetting.dispatcher = TestDispatcher::SERIAL;
    outerSetting.group_wait = true;
    outerSetting.group_timeout = groupWait;
    outerSetting.create_group = true;
    outerSetting.op = TestOperation::ASYNC_GROUP;
    innerSetting.op = TestOperation::ASYNC;
    int taskCount = Dispatch(outerSetting, innerSetting);
    bool result = Wait(taskCount);
    result = result && task_execution_sequence.size() > 1;
    if (result) {
        result = result && applyExecuted(outerSetting, innerSetting);
        result = result && OuterTaskExecuted(outerSetting);
        result = result && InnerTaskExecuted(innerSetting);
        std::vector<size_t> outerTaskIndex;
        std::vector<std::vector<size_t>> innerTaskIndex;
        GetTaskIndex(outerTaskIndex, innerTaskIndex);
        for (int i = 0; i < testTaskCount; i++) {
            result = result && IsAscend(innerTaskIndex[i]);
        }
    }
    TestUtils::PublishEvent(g_EVENT_RESP_FIRST, code, std::to_string(result));
}

// level1:global, group wait level2:serial, delay
void MainAbility::GlobalCase64(int code)
{
    APP_LOGI("-- -- -- -- -- --MainAbility::%{public}s", __FUNCTION__);
    Reset();
    TestSetting outerSetting;
    TestSetting innerSetting;
    outerSetting.dispatcher = TestDispatcher::GLOBAL;
    innerSetting.dispatcher = TestDispatcher::SERIAL;
    outerSetting.group_wait = true;
    outerSetting.group_timeout = groupWait;
    innerSetting.delay = delayMs;
    outerSetting.create_group = true;
    outerSetting.op = TestOperation::ASYNC_GROUP;
    innerSetting.op = TestOperation::DELAY;
    int taskCount = Dispatch(outerSetting, innerSetting);
    bool result = Wait(taskCount);
    result = result && task_execution_sequence.size() > 1;
    if (result) {
        result = result && applyExecuted(outerSetting, innerSetting);
        result = result && OuterTaskExecuted(outerSetting);
        result = result && InnerTaskExecuted(innerSetting);
        std::vector<size_t> outerTaskIndex;
        std::vector<std::vector<size_t>> innerTaskIndex;
        GetTaskIndex(outerTaskIndex, innerTaskIndex);
        for (int i = 0; i < testTaskCount; i++) {
            result = result && IsAscend(innerTaskIndex[i]);
        }
    }
    TestUtils::PublishEvent(g_EVENT_RESP_FIRST, code, std::to_string(result));
}

// level1:global, group wait level2:serial, apply
void MainAbility::GlobalCase65(int code)
{
    APP_LOGI("-- -- -- -- -- --MainAbility::%{public}s", __FUNCTION__);
    Reset();
    TestSetting outerSetting;
    TestSetting innerSetting;
    outerSetting.dispatcher = TestDispatcher::GLOBAL;
    innerSetting.dispatcher = TestDispatcher::SERIAL;
    outerSetting.group_wait = true;
    outerSetting.group_timeout = groupWait;
    innerSetting.apply = applyNum;
    outerSetting.create_group = true;
    outerSetting.op = TestOperation::ASYNC_GROUP;
    innerSetting.op = TestOperation::APPLY;
    int taskCount = Dispatch(outerSetting, innerSetting);
    bool result = Wait(taskCount);
    result = result && task_execution_sequence.size() > 1;
    if (result) {
        result = result && applyExecuted(outerSetting, innerSetting);
        result = result && OuterTaskExecuted(outerSetting);
        result = result && InnerTaskExecuted(innerSetting);
    }
    TestUtils::PublishEvent(g_EVENT_RESP_FIRST, code, std::to_string(result));
}

// level1:global, group notify level2:parallel, sync
void MainAbility::GlobalCase66(int code)
{
    APP_LOGI("-- -- -- -- -- --MainAbility::%{public}s", __FUNCTION__);
    Reset();
    TestSetting outerSetting;
    TestSetting innerSetting;
    outerSetting.dispatcher = TestDispatcher::GLOBAL;
    innerSetting.dispatcher = TestDispatcher::PARALLEL;
    outerSetting.create_group = true;
    outerSetting.group_notify = true;
    outerSetting.op = TestOperation::ASYNC_GROUP;
    innerSetting.op = TestOperation::SYNC;
    int taskCount = Dispatch(outerSetting, innerSetting);
    bool result = Wait(taskCount);
    result = result && task_execution_sequence.size() > 1;
    if (result) {
        result = result && applyExecuted(outerSetting, innerSetting);
        result = result && OuterTaskExecuted(outerSetting);
        result = result && InnerTaskExecuted(innerSetting);
        std::vector<size_t> outerTaskIndex;
        std::vector<std::vector<size_t>> innerTaskIndex;
        GetTaskIndex(outerTaskIndex, innerTaskIndex);
        for (int i = 0; i < testTaskCount; i++) {
            result = result && IsAscend(innerTaskIndex[i]);
        }
        result = result && OuterExecutedAfter(outerGroupNotifyId);
    }
    TestUtils::PublishEvent(g_EVENT_RESP_FIRST, code, std::to_string(result));
}

// level1:global, group notify level2:parallel, async
void MainAbility::GlobalCase67(int code)
{
    APP_LOGI("-- -- -- -- -- --MainAbility::%{public}s", __FUNCTION__);
    Reset();
    TestSetting outerSetting;
    TestSetting innerSetting;
    outerSetting.dispatcher = TestDispatcher::GLOBAL;
    innerSetting.dispatcher = TestDispatcher::PARALLEL;
    outerSetting.create_group = true;
    outerSetting.group_notify = true;
    outerSetting.op = TestOperation::ASYNC_GROUP;
    innerSetting.op = TestOperation::ASYNC;
    int taskCount = Dispatch(outerSetting, innerSetting);
    bool result = Wait(taskCount);
    result = result && task_execution_sequence.size() > 1;
    if (result) {
        result = result && applyExecuted(outerSetting, innerSetting);
        result = result && OuterTaskExecuted(outerSetting);
        result = result && InnerTaskExecuted(innerSetting);
        result = result && OuterExecutedAfter(outerGroupNotifyId);
    }
    TestUtils::PublishEvent(g_EVENT_RESP_FIRST, code, std::to_string(result));
}

// level1:global, group notify level2:parallel, delay
void MainAbility::GlobalCase68(int code)
{
    APP_LOGI("-- -- -- -- -- --MainAbility::%{public}s", __FUNCTION__);
    Reset();
    TestSetting outerSetting;
    TestSetting innerSetting;
    outerSetting.dispatcher = TestDispatcher::GLOBAL;
    innerSetting.dispatcher = TestDispatcher::PARALLEL;
    innerSetting.delay = delayMs;
    outerSetting.create_group = true;
    outerSetting.group_notify = true;
    outerSetting.op = TestOperation::ASYNC_GROUP;
    innerSetting.op = TestOperation::DELAY;
    int taskCount = Dispatch(outerSetting, innerSetting);
    bool result = Wait(taskCount);
    result = result && task_execution_sequence.size() > 1;
    if (result) {
        result = result && applyExecuted(outerSetting, innerSetting);
        result = result && OuterTaskExecuted(outerSetting);
        result = result && InnerTaskExecuted(innerSetting);
        result = result && OuterExecutedAfter(outerGroupNotifyId);
    }
    TestUtils::PublishEvent(g_EVENT_RESP_FIRST, code, std::to_string(result));
}

// level1:global, group notify level2:parallel, group
void MainAbility::GlobalCase69(int code)
{
    APP_LOGI("-- -- -- -- -- --MainAbility::%{public}s", __FUNCTION__);
    Reset();
    TestSetting outerSetting;
    TestSetting innerSetting;
    outerSetting.dispatcher = TestDispatcher::GLOBAL;
    innerSetting.dispatcher = TestDispatcher::PARALLEL;
    innerSetting.create_group = true;
    outerSetting.create_group = true;
    outerSetting.group_notify = true;
    outerSetting.op = TestOperation::ASYNC_GROUP;
    innerSetting.op = TestOperation::ASYNC_GROUP;
    int taskCount = Dispatch(outerSetting, innerSetting);
    bool result = Wait(taskCount);
    result = result && task_execution_sequence.size() > 1;
    if (result) {
        result = result && applyExecuted(outerSetting, innerSetting);
        result = result && OuterTaskExecuted(outerSetting);
        result = result && InnerTaskExecuted(innerSetting);
        result = result && OuterExecutedAfter(outerGroupNotifyId);
    }
    TestUtils::PublishEvent(g_EVENT_RESP_FIRST, code, std::to_string(result));
}

// level1:global, group notify level2:parallel, group wait
void MainAbility::GlobalCase70(int code)
{
    APP_LOGI("-- -- -- -- -- --MainAbility::%{public}s", __FUNCTION__);
    Reset();
    TestSetting outerSetting;
    TestSetting innerSetting;
    outerSetting.dispatcher = TestDispatcher::GLOBAL;
    innerSetting.dispatcher = TestDispatcher::PARALLEL;
    innerSetting.group_wait = true;
    innerSetting.group_timeout = groupWait;
    innerSetting.create_group = true;
    outerSetting.create_group = true;
    outerSetting.group_notify = true;
    outerSetting.op = TestOperation::ASYNC_GROUP;
    innerSetting.op = TestOperation::ASYNC_GROUP;
    int taskCount = Dispatch(outerSetting, innerSetting);
    bool result = Wait(taskCount);
    result = result && task_execution_sequence.size() > 1;
    if (result) {
        result = result && applyExecuted(outerSetting, innerSetting);
        result = result && OuterTaskExecuted(outerSetting);
        result = result && InnerTaskExecuted(innerSetting);
        result = result && OuterExecutedAfter(outerGroupNotifyId);
    }
    TestUtils::PublishEvent(g_EVENT_RESP_FIRST, code, std::to_string(result));
}

// level1:global, group notify level2:parallel, group notify
void MainAbility::GlobalCase71(int code)
{
    APP_LOGI("-- -- -- -- -- --MainAbility::%{public}s", __FUNCTION__);
    Reset();
    TestSetting outerSetting;
    TestSetting innerSetting;
    outerSetting.dispatcher = TestDispatcher::GLOBAL;
    innerSetting.dispatcher = TestDispatcher::PARALLEL;
    innerSetting.create_group = true;
    innerSetting.group_notify = true;
    outerSetting.create_group = true;
    outerSetting.group_notify = true;
    outerSetting.op = TestOperation::ASYNC_GROUP;
    innerSetting.op = TestOperation::ASYNC_GROUP;
    int taskCount = Dispatch(outerSetting, innerSetting);
    bool result = Wait(taskCount);
    result = result && task_execution_sequence.size() > 1;
    if (result) {
        result = result && applyExecuted(outerSetting, innerSetting);
        result = result && OuterTaskExecuted(outerSetting);
        result = result && InnerTaskExecuted(innerSetting);
        result = result && OuterExecutedAfter(outerGroupNotifyId);
        result = result && InnerExecutedAfter(innerGroupNotifyId);
    }
    TestUtils::PublishEvent(g_EVENT_RESP_FIRST, code, std::to_string(result));
}

// level1:global, group notify level2:parallel, sync barrier
void MainAbility::GlobalCase72(int code)
{
    APP_LOGI("-- -- -- -- -- --MainAbility::%{public}s", __FUNCTION__);
    Reset();
    TestSetting outerSetting;
    TestSetting innerSetting;
    outerSetting.dispatcher = TestDispatcher::GLOBAL;
    innerSetting.dispatcher = TestDispatcher::PARALLEL;
    innerSetting.sync_barrier = true;
    innerSetting.create_group = true;
    outerSetting.create_group = true;
    outerSetting.group_notify = true;
    outerSetting.op = TestOperation::ASYNC_GROUP;
    innerSetting.op = TestOperation::ASYNC_GROUP;
    int taskCount = Dispatch(outerSetting, innerSetting);
    bool result = Wait(taskCount);
    result = result && task_execution_sequence.size() > 1;
    if (result) {
        result = result && applyExecuted(outerSetting, innerSetting);
        result = result && OuterTaskExecuted(outerSetting);
        result = result && InnerTaskExecuted(innerSetting);
        result = result && InnerExecutedAfter(innerSyncBarrierId);
        result = result && OuterExecutedAfter(outerGroupNotifyId);
    }
    TestUtils::PublishEvent(g_EVENT_RESP_FIRST, code, std::to_string(result));
}

// level1:global, group notify level2:parallel, async barrier
void MainAbility::GlobalCase73(int code)
{
    APP_LOGI("-- -- -- -- -- --MainAbility::%{public}s", __FUNCTION__);
    Reset();
    TestSetting outerSetting;
    TestSetting innerSetting;
    outerSetting.dispatcher = TestDispatcher::GLOBAL;
    innerSetting.dispatcher = TestDispatcher::PARALLEL;
    innerSetting.async_barrier = true;
    innerSetting.create_group = true;
    outerSetting.create_group = true;
    outerSetting.group_notify = true;
    outerSetting.op = TestOperation::ASYNC_GROUP;
    innerSetting.op = TestOperation::ASYNC_GROUP;
    int taskCount = Dispatch(outerSetting, innerSetting);
    bool result = Wait(taskCount);
    result = result && task_execution_sequence.size() > 1;
    if (result) {
        result = result && applyExecuted(outerSetting, innerSetting);
        result = result && OuterTaskExecuted(outerSetting);
        result = result && InnerTaskExecuted(innerSetting);
        result = result && InnerExecutedAfter(innerAsyncBarrierId);
        result = result && OuterExecutedAfter(outerGroupNotifyId);
    }
    TestUtils::PublishEvent(g_EVENT_RESP_FIRST, code, std::to_string(result));
}

// level1:global, group notify level2:parallel, apply
void MainAbility::GlobalCase74(int code)
{
    APP_LOGI("-- -- -- -- -- --MainAbility::%{public}s", __FUNCTION__);
    Reset();
    TestSetting outerSetting;
    TestSetting innerSetting;
    outerSetting.dispatcher = TestDispatcher::GLOBAL;
    innerSetting.dispatcher = TestDispatcher::PARALLEL;
    innerSetting.apply = applyNum;
    outerSetting.create_group = true;
    outerSetting.group_notify = true;
    outerSetting.op = TestOperation::ASYNC_GROUP;
    innerSetting.op = TestOperation::APPLY;
    int taskCount = Dispatch(outerSetting, innerSetting);
    bool result = Wait(taskCount);
    result = result && task_execution_sequence.size() > 1;
    if (result) {
        result = result && applyExecuted(outerSetting, innerSetting);
        result = result && OuterTaskExecuted(outerSetting);
        result = result && InnerTaskExecuted(innerSetting);
        result = result && OuterExecutedAfter(outerGroupNotifyId);
    }
    TestUtils::PublishEvent(g_EVENT_RESP_FIRST, code, std::to_string(result));
}

// level1:global, group notify level2:serial, sync
void MainAbility::GlobalCase75(int code)
{
    APP_LOGI("-- -- -- -- -- --MainAbility::%{public}s", __FUNCTION__);
    Reset();
    TestSetting outerSetting;
    TestSetting innerSetting;
    outerSetting.dispatcher = TestDispatcher::GLOBAL;
    innerSetting.dispatcher = TestDispatcher::SERIAL;
    outerSetting.create_group = true;
    outerSetting.group_notify = true;
    outerSetting.op = TestOperation::ASYNC_GROUP;
    innerSetting.op = TestOperation::SYNC;
    int taskCount = Dispatch(outerSetting, innerSetting);
    bool result = Wait(taskCount);
    result = result && task_execution_sequence.size() > 1;
    if (result) {
        result = result && applyExecuted(outerSetting, innerSetting);
        result = result && OuterTaskExecuted(outerSetting);
        result = result && InnerTaskExecuted(innerSetting);
        std::vector<size_t> outerTaskIndex;
        std::vector<std::vector<size_t>> innerTaskIndex;
        GetTaskIndex(outerTaskIndex, innerTaskIndex);
        for (int i = 0; i < testTaskCount; i++) {
            result = result && IsAscend(innerTaskIndex[i]);
        }
        result = result && OuterExecutedAfter(outerGroupNotifyId);
    }
    TestUtils::PublishEvent(g_EVENT_RESP_FIRST, code, std::to_string(result));
}

// level1:global, group notify level2:serial, async
void MainAbility::GlobalCase76(int code)
{
    APP_LOGI("-- -- -- -- -- --MainAbility::%{public}s", __FUNCTION__);
    Reset();
    TestSetting outerSetting;
    TestSetting innerSetting;
    outerSetting.dispatcher = TestDispatcher::GLOBAL;
    innerSetting.dispatcher = TestDispatcher::SERIAL;
    outerSetting.create_group = true;
    outerSetting.group_notify = true;
    outerSetting.op = TestOperation::ASYNC_GROUP;
    innerSetting.op = TestOperation::ASYNC;
    int taskCount = Dispatch(outerSetting, innerSetting);
    bool result = Wait(taskCount);
    result = result && task_execution_sequence.size() > 1;
    if (result) {
        result = result && applyExecuted(outerSetting, innerSetting);
        result = result && OuterTaskExecuted(outerSetting);
        result = result && InnerTaskExecuted(innerSetting);
        std::vector<size_t> outerTaskIndex;
        std::vector<std::vector<size_t>> innerTaskIndex;
        GetTaskIndex(outerTaskIndex, innerTaskIndex);
        for (int i = 0; i < testTaskCount; i++) {
            result = result && IsAscend(innerTaskIndex[i]);
        }
        result = result && OuterExecutedAfter(outerGroupNotifyId);
    }
    TestUtils::PublishEvent(g_EVENT_RESP_FIRST, code, std::to_string(result));
}

// level1:global, group notify level2:serial, delay
void MainAbility::GlobalCase77(int code)
{
    APP_LOGI("-- -- -- -- -- --MainAbility::%{public}s", __FUNCTION__);
    Reset();
    TestSetting outerSetting;
    TestSetting innerSetting;
    outerSetting.dispatcher = TestDispatcher::GLOBAL;
    innerSetting.dispatcher = TestDispatcher::SERIAL;
    innerSetting.delay = delayMs;
    outerSetting.create_group = true;
    outerSetting.group_notify = true;
    outerSetting.op = TestOperation::ASYNC_GROUP;
    innerSetting.op = TestOperation::DELAY;
    int taskCount = Dispatch(outerSetting, innerSetting);
    bool result = Wait(taskCount);
    result = result && task_execution_sequence.size() > 1;
    if (result) {
        result = result && applyExecuted(outerSetting, innerSetting);
        result = result && OuterTaskExecuted(outerSetting);
        result = result && InnerTaskExecuted(innerSetting);
        std::vector<size_t> outerTaskIndex;
        std::vector<std::vector<size_t>> innerTaskIndex;
        GetTaskIndex(outerTaskIndex, innerTaskIndex);
        for (int i = 0; i < testTaskCount; i++) {
            result = result && IsAscend(innerTaskIndex[i]);
        }
        result = result && OuterExecutedAfter(outerGroupNotifyId);
    }
    TestUtils::PublishEvent(g_EVENT_RESP_FIRST, code, std::to_string(result));
}

// level1:global, group notify level2:serial, apply
void MainAbility::GlobalCase78(int code)
{
    APP_LOGI("-- -- -- -- -- --MainAbility::%{public}s", __FUNCTION__);
    Reset();
    TestSetting outerSetting;
    TestSetting innerSetting;
    outerSetting.dispatcher = TestDispatcher::GLOBAL;
    innerSetting.dispatcher = TestDispatcher::SERIAL;
    innerSetting.apply = applyNum;
    outerSetting.create_group = true;
    outerSetting.group_notify = true;
    outerSetting.op = TestOperation::ASYNC_GROUP;
    innerSetting.op = TestOperation::APPLY;
    int taskCount = Dispatch(outerSetting, innerSetting);
    bool result = Wait(taskCount);
    result = result && task_execution_sequence.size() > 1;
    if (result) {
        result = result && applyExecuted(outerSetting, innerSetting);
        result = result && OuterTaskExecuted(outerSetting);
        result = result && InnerTaskExecuted(innerSetting);
        result = result && OuterExecutedAfter(outerGroupNotifyId);
    }
    TestUtils::PublishEvent(g_EVENT_RESP_FIRST, code, std::to_string(result));
}

// level1:global, apply level2:parallel, sync
void MainAbility::GlobalCase79(int code)
{
    APP_LOGI("-- -- -- -- -- --MainAbility::%{public}s", __FUNCTION__);
    Reset();
    TestSetting outerSetting;
    TestSetting innerSetting;
    outerSetting.dispatcher = TestDispatcher::GLOBAL;
    innerSetting.dispatcher = TestDispatcher::PARALLEL;
    outerSetting.apply = applyNum;
    outerSetting.op = TestOperation::APPLY;
    innerSetting.op = TestOperation::SYNC;
    int taskCount = Dispatch(outerSetting, innerSetting);
    bool result = Wait(taskCount);
    result = result && task_execution_sequence.size() > 1;
    if (result) {
        result = result && applyExecuted(outerSetting, innerSetting);
        result = result && OuterTaskExecuted(outerSetting);
        result = result && InnerTaskExecuted(innerSetting);
        std::vector<size_t> outerTaskIndex;
        std::vector<std::vector<size_t>> innerTaskIndex;
        GetTaskIndex(outerTaskIndex, innerTaskIndex);
        for (int i = 0; i < testTaskCount; i++) {
            result = result && IsAscend(innerTaskIndex[i]);
        }
    }
    TestUtils::PublishEvent(g_EVENT_RESP_FIRST, code, std::to_string(result));
}

// level1:global, apply level2:parallel, async
void MainAbility::GlobalCase80(int code)
{
    APP_LOGI("-- -- -- -- -- --MainAbility::%{public}s", __FUNCTION__);
    Reset();
    TestSetting outerSetting;
    TestSetting innerSetting;
    outerSetting.dispatcher = TestDispatcher::GLOBAL;
    innerSetting.dispatcher = TestDispatcher::PARALLEL;
    outerSetting.apply = applyNum;
    outerSetting.op = TestOperation::APPLY;
    innerSetting.op = TestOperation::ASYNC;
    int taskCount = Dispatch(outerSetting, innerSetting);
    bool result = Wait(taskCount);
    result = result && task_execution_sequence.size() > 1;
    if (result) {
        result = result && applyExecuted(outerSetting, innerSetting);
        result = result && OuterTaskExecuted(outerSetting);
        result = result && InnerTaskExecuted(innerSetting);
    }
    TestUtils::PublishEvent(g_EVENT_RESP_FIRST, code, std::to_string(result));
}

// level1:global, apply level2:parallel, delay
void MainAbility::GlobalCase81(int code)
{
    APP_LOGI("-- -- -- -- -- --MainAbility::%{public}s", __FUNCTION__);
    Reset();
    TestSetting outerSetting;
    TestSetting innerSetting;
    outerSetting.dispatcher = TestDispatcher::GLOBAL;
    innerSetting.dispatcher = TestDispatcher::PARALLEL;
    outerSetting.apply = applyNum;
    innerSetting.delay = delayMs;
    outerSetting.op = TestOperation::APPLY;
    innerSetting.op = TestOperation::DELAY;
    int taskCount = Dispatch(outerSetting, innerSetting);
    bool result = Wait(taskCount);
    result = result && task_execution_sequence.size() > 1;
    if (result) {
        result = result && applyExecuted(outerSetting, innerSetting);
        result = result && OuterTaskExecuted(outerSetting);
        result = result && InnerTaskExecuted(innerSetting);
    }
    TestUtils::PublishEvent(g_EVENT_RESP_FIRST, code, std::to_string(result));
}

// level1:global, apply level2:parallel, group
void MainAbility::GlobalCase82(int code)
{
    APP_LOGI("-- -- -- -- -- --MainAbility::%{public}s", __FUNCTION__);
    Reset();
    TestSetting outerSetting;
    TestSetting innerSetting;
    outerSetting.dispatcher = TestDispatcher::GLOBAL;
    innerSetting.dispatcher = TestDispatcher::PARALLEL;
    outerSetting.apply = applyNum;
    innerSetting.create_group = true;
    outerSetting.op = TestOperation::APPLY;
    innerSetting.op = TestOperation::ASYNC_GROUP;
    int taskCount = Dispatch(outerSetting, innerSetting);
    bool result = Wait(taskCount);
    result = result && task_execution_sequence.size() > 1;
    if (result) {
        result = result && applyExecuted(outerSetting, innerSetting);
        result = result && OuterTaskExecuted(outerSetting);
        result = result && InnerTaskExecuted(innerSetting);
    }
    TestUtils::PublishEvent(g_EVENT_RESP_FIRST, code, std::to_string(result));
}

// level1:global, apply level2:parallel, group wait
void MainAbility::GlobalCase83(int code)
{
    APP_LOGI("-- -- -- -- -- --MainAbility::%{public}s", __FUNCTION__);
    Reset();
    TestSetting outerSetting;
    TestSetting innerSetting;
    outerSetting.dispatcher = TestDispatcher::GLOBAL;
    innerSetting.dispatcher = TestDispatcher::PARALLEL;
    innerSetting.group_wait = true;
    innerSetting.group_timeout = groupWait;
    outerSetting.apply = applyNum;
    innerSetting.create_group = true;
    outerSetting.op = TestOperation::APPLY;
    innerSetting.op = TestOperation::ASYNC_GROUP;
    int taskCount = Dispatch(outerSetting, innerSetting);
    bool result = Wait(taskCount);
    result = result && task_execution_sequence.size() > 1;
    if (result) {
        result = result && applyExecuted(outerSetting, innerSetting);
        result = result && OuterTaskExecuted(outerSetting);
        result = result && InnerTaskExecuted(innerSetting);
    }
    TestUtils::PublishEvent(g_EVENT_RESP_FIRST, code, std::to_string(result));
}

// level1:global, apply level2:parallel, group notify
void MainAbility::GlobalCase84(int code)
{
    APP_LOGI("-- -- -- -- -- --MainAbility::%{public}s", __FUNCTION__);
    Reset();
    TestSetting outerSetting;
    TestSetting innerSetting;
    outerSetting.dispatcher = TestDispatcher::GLOBAL;
    innerSetting.dispatcher = TestDispatcher::PARALLEL;
    outerSetting.apply = applyNum;
    innerSetting.create_group = true;
    innerSetting.group_notify = true;
    outerSetting.op = TestOperation::APPLY;
    innerSetting.op = TestOperation::ASYNC_GROUP;
    int taskCount = Dispatch(outerSetting, innerSetting);
    bool result = Wait(taskCount);
    result = result && task_execution_sequence.size() > 1;
    if (result) {
        result = result && applyExecuted(outerSetting, innerSetting);
        result = result && OuterTaskExecuted(outerSetting);
        result = result && InnerTaskExecuted(innerSetting);
        result = result && InnerExecutedAfter(innerGroupNotifyId);
    }
    TestUtils::PublishEvent(g_EVENT_RESP_FIRST, code, std::to_string(result));
}

// level1:global, apply level2:parallel, sync barrier
void MainAbility::GlobalCase85(int code)
{
    APP_LOGI("-- -- -- -- -- --MainAbility::%{public}s", __FUNCTION__);
    Reset();
    TestSetting outerSetting;
    TestSetting innerSetting;
    outerSetting.dispatcher = TestDispatcher::GLOBAL;
    innerSetting.dispatcher = TestDispatcher::PARALLEL;
    outerSetting.apply = applyNum;
    innerSetting.sync_barrier = true;
    innerSetting.create_group = true;
    outerSetting.op = TestOperation::APPLY;
    innerSetting.op = TestOperation::ASYNC_GROUP;
    int taskCount = Dispatch(outerSetting, innerSetting);
    bool result = Wait(taskCount);
    result = result && task_execution_sequence.size() > 1;
    if (result) {
        result = result && applyExecuted(outerSetting, innerSetting);
        result = result && OuterTaskExecuted(outerSetting);
        result = result && InnerTaskExecuted(innerSetting);
        result = result && InnerExecutedAfter(innerSyncBarrierId);
    }
    TestUtils::PublishEvent(g_EVENT_RESP_FIRST, code, std::to_string(result));
}

// level1:global, apply level2:parallel, async barrier
void MainAbility::GlobalCase86(int code)
{
    APP_LOGI("-- -- -- -- -- --MainAbility::%{public}s", __FUNCTION__);
    Reset();
    TestSetting outerSetting;
    TestSetting innerSetting;
    outerSetting.dispatcher = TestDispatcher::GLOBAL;
    innerSetting.dispatcher = TestDispatcher::PARALLEL;
    outerSetting.apply = applyNum;
    innerSetting.async_barrier = true;
    innerSetting.create_group = true;
    outerSetting.op = TestOperation::APPLY;
    innerSetting.op = TestOperation::ASYNC_GROUP;
    int taskCount = Dispatch(outerSetting, innerSetting);
    bool result = Wait(taskCount);
    result = result && task_execution_sequence.size() > 1;
    if (result) {
        result = result && applyExecuted(outerSetting, innerSetting);
        result = result && OuterTaskExecuted(outerSetting);
        result = result && InnerTaskExecuted(innerSetting);
        result = result && InnerExecutedAfter(innerAsyncBarrierId);
    }
    TestUtils::PublishEvent(g_EVENT_RESP_FIRST, code, std::to_string(result));
}

// level1:global, apply level2:parallel, apply
void MainAbility::GlobalCase87(int code)
{
    APP_LOGI("-- -- -- -- -- --MainAbility::%{public}s", __FUNCTION__);
    Reset();
    TestSetting outerSetting;
    TestSetting innerSetting;
    outerSetting.dispatcher = TestDispatcher::GLOBAL;
    innerSetting.dispatcher = TestDispatcher::PARALLEL;
    outerSetting.apply = applyNum;
    innerSetting.apply = applyNum;
    outerSetting.op = TestOperation::APPLY;
    innerSetting.op = TestOperation::APPLY;
    int taskCount = Dispatch(outerSetting, innerSetting);
    bool result = Wait(taskCount);
    result = result && task_execution_sequence.size() > 1;
    if (result) {
        result = result && applyExecuted(outerSetting, innerSetting);
        result = result && OuterTaskExecuted(outerSetting);
        result = result && InnerTaskExecuted(innerSetting);
    }
    TestUtils::PublishEvent(g_EVENT_RESP_FIRST, code, std::to_string(result));
}

// level1:global, apply level2:serial, sync
void MainAbility::GlobalCase88(int code)
{
    APP_LOGI("-- -- -- -- -- --MainAbility::%{public}s", __FUNCTION__);
    Reset();
    TestSetting outerSetting;
    TestSetting innerSetting;
    outerSetting.dispatcher = TestDispatcher::GLOBAL;
    innerSetting.dispatcher = TestDispatcher::SERIAL;
    outerSetting.apply = applyNum;
    outerSetting.op = TestOperation::APPLY;
    innerSetting.op = TestOperation::SYNC;
    int taskCount = Dispatch(outerSetting, innerSetting);
    bool result = Wait(taskCount);
    result = result && task_execution_sequence.size() > 1;
    if (result) {
        result = result && applyExecuted(outerSetting, innerSetting);
        result = result && OuterTaskExecuted(outerSetting);
        result = result && InnerTaskExecuted(innerSetting);
        std::vector<size_t> outerTaskIndex;
        std::vector<std::vector<size_t>> innerTaskIndex;
        GetTaskIndex(outerTaskIndex, innerTaskIndex);
        for (int i = 0; i < testTaskCount; i++) {
            result = result && IsAscend(innerTaskIndex[i]);
        }
    }
    TestUtils::PublishEvent(g_EVENT_RESP_FIRST, code, std::to_string(result));
}

// level1:global, apply level2:serial, async
void MainAbility::GlobalCase89(int code)
{
    APP_LOGI("-- -- -- -- -- --MainAbility::%{public}s", __FUNCTION__);
    Reset();
    TestSetting outerSetting;
    TestSetting innerSetting;
    outerSetting.dispatcher = TestDispatcher::GLOBAL;
    innerSetting.dispatcher = TestDispatcher::SERIAL;
    outerSetting.apply = applyNum;
    outerSetting.op = TestOperation::APPLY;
    innerSetting.op = TestOperation::ASYNC;
    int taskCount = Dispatch(outerSetting, innerSetting);
    bool result = Wait(taskCount);
    result = result && task_execution_sequence.size() > 1;
    if (result) {
        result = result && applyExecuted(outerSetting, innerSetting);
        result = result && OuterTaskExecuted(outerSetting);
        result = result && InnerTaskExecuted(innerSetting);
        std::vector<size_t> outerTaskIndex;
        std::vector<std::vector<size_t>> innerTaskIndex;
        GetTaskIndex(outerTaskIndex, innerTaskIndex);
        for (int i = 0; i < testTaskCount; i++) {
            result = result && IsAscend(innerTaskIndex[i]);
        }
    }
    TestUtils::PublishEvent(g_EVENT_RESP_FIRST, code, std::to_string(result));
}

// level1:global, apply level2:serial, delay
void MainAbility::GlobalCase90(int code)
{
    APP_LOGI("-- -- -- -- -- --MainAbility::%{public}s", __FUNCTION__);
    Reset();
    TestSetting outerSetting;
    TestSetting innerSetting;
    outerSetting.dispatcher = TestDispatcher::GLOBAL;
    innerSetting.dispatcher = TestDispatcher::SERIAL;
    outerSetting.apply = applyNum;
    innerSetting.delay = delayMs;
    outerSetting.op = TestOperation::APPLY;
    innerSetting.op = TestOperation::DELAY;
    int taskCount = Dispatch(outerSetting, innerSetting);
    bool result = Wait(taskCount);
    result = result && task_execution_sequence.size() > 1;
    if (result) {
        result = result && applyExecuted(outerSetting, innerSetting);
        result = result && OuterTaskExecuted(outerSetting);
        result = result && InnerTaskExecuted(innerSetting);
        std::vector<size_t> outerTaskIndex;
        std::vector<std::vector<size_t>> innerTaskIndex;
        GetTaskIndex(outerTaskIndex, innerTaskIndex);
        for (int i = 0; i < testTaskCount; i++) {
            result = result && IsAscend(innerTaskIndex[i]);
        }
    }
    TestUtils::PublishEvent(g_EVENT_RESP_FIRST, code, std::to_string(result));
}

// level1:global, apply level2:serial, apply
void MainAbility::GlobalCase91(int code)
{
    APP_LOGI("-- -- -- -- -- --MainAbility::%{public}s", __FUNCTION__);
    Reset();
    TestSetting outerSetting;
    TestSetting innerSetting;
    outerSetting.dispatcher = TestDispatcher::GLOBAL;
    innerSetting.dispatcher = TestDispatcher::SERIAL;
    outerSetting.apply = applyNum;
    innerSetting.apply = applyNum;
    outerSetting.op = TestOperation::APPLY;
    innerSetting.op = TestOperation::APPLY;
    int taskCount = Dispatch(outerSetting, innerSetting);
    bool result = Wait(taskCount);
    result = result && task_execution_sequence.size() > 1;
    if (result) {
        result = result && applyExecuted(outerSetting, innerSetting);
        result = result && OuterTaskExecuted(outerSetting);
        result = result && InnerTaskExecuted(innerSetting);
    }
    TestUtils::PublishEvent(g_EVENT_RESP_FIRST, code, std::to_string(result));
}

// level1:parallel, sync level2:parallel, sync
void MainAbility::ParallelCase1(int code)
{
    APP_LOGI("-- -- -- -- -- --MainAbility::%{public}s", __FUNCTION__);
    Reset();
    TestSetting outerSetting;
    TestSetting innerSetting;
    outerSetting.dispatcher = TestDispatcher::PARALLEL;
    innerSetting.dispatcher = TestDispatcher::PARALLEL;
    outerSetting.op = TestOperation::SYNC;
    innerSetting.op = TestOperation::SYNC;
    int taskCount = Dispatch(outerSetting, innerSetting);
    bool result = Wait(taskCount);
    result = result && task_execution_sequence.size() > 1;
    if (result) {
        result = result && applyExecuted(outerSetting, innerSetting);
        result = result && OuterTaskExecuted(outerSetting);
        result = result && InnerTaskExecuted(innerSetting);
        std::vector<size_t> outerTaskIndex;
        std::vector<std::vector<size_t>> innerTaskIndex;
        GetTaskIndex(outerTaskIndex, innerTaskIndex);
        result = result && IsAscend(outerTaskIndex);
        for (int i = 0; i < testTaskCount; i++) {
            result = result && IsAscend(innerTaskIndex[i]);
        }
    }
    TestUtils::PublishEvent(g_EVENT_RESP_FIRST, code, std::to_string(result));
}

// level1:parallel, sync level2:parallel, async
void MainAbility::ParallelCase2(int code)
{
    APP_LOGI("-- -- -- -- -- --MainAbility::%{public}s", __FUNCTION__);
    Reset();
    TestSetting outerSetting;
    TestSetting innerSetting;
    outerSetting.dispatcher = TestDispatcher::PARALLEL;
    innerSetting.dispatcher = TestDispatcher::PARALLEL;
    outerSetting.op = TestOperation::SYNC;
    innerSetting.op = TestOperation::ASYNC;
    int taskCount = Dispatch(outerSetting, innerSetting);
    bool result = Wait(taskCount);
    result = result && task_execution_sequence.size() > 1;
    if (result) {
        result = result && applyExecuted(outerSetting, innerSetting);
        result = result && OuterTaskExecuted(outerSetting);
        result = result && InnerTaskExecuted(innerSetting);
        std::vector<size_t> outerTaskIndex;
        std::vector<std::vector<size_t>> innerTaskIndex;
        GetTaskIndex(outerTaskIndex, innerTaskIndex);
        result = result && IsAscend(outerTaskIndex);
    }
    TestUtils::PublishEvent(g_EVENT_RESP_FIRST, code, std::to_string(result));
}

// level1:parallel, sync level2:parallel, delay
void MainAbility::ParallelCase3(int code)
{
    APP_LOGI("-- -- -- -- -- --MainAbility::%{public}s", __FUNCTION__);
    Reset();
    TestSetting outerSetting;
    TestSetting innerSetting;
    outerSetting.dispatcher = TestDispatcher::PARALLEL;
    innerSetting.dispatcher = TestDispatcher::PARALLEL;
    innerSetting.delay = delayMs;
    outerSetting.op = TestOperation::SYNC;
    innerSetting.op = TestOperation::DELAY;
    int taskCount = Dispatch(outerSetting, innerSetting);
    bool result = Wait(taskCount);
    result = result && task_execution_sequence.size() > 1;
    if (result) {
        result = result && applyExecuted(outerSetting, innerSetting);
        result = result && OuterTaskExecuted(outerSetting);
        result = result && InnerTaskExecuted(innerSetting);
        std::vector<size_t> outerTaskIndex;
        std::vector<std::vector<size_t>> innerTaskIndex;
        GetTaskIndex(outerTaskIndex, innerTaskIndex);
        result = result && IsAscend(outerTaskIndex);
    }
    TestUtils::PublishEvent(g_EVENT_RESP_FIRST, code, std::to_string(result));
}

// level1:parallel, sync level2:parallel, group
void MainAbility::ParallelCase4(int code)
{
    APP_LOGI("-- -- -- -- -- --MainAbility::%{public}s", __FUNCTION__);
    Reset();
    TestSetting outerSetting;
    TestSetting innerSetting;
    outerSetting.dispatcher = TestDispatcher::PARALLEL;
    innerSetting.dispatcher = TestDispatcher::PARALLEL;
    innerSetting.create_group = true;
    outerSetting.op = TestOperation::SYNC;
    innerSetting.op = TestOperation::ASYNC_GROUP;
    int taskCount = Dispatch(outerSetting, innerSetting);
    bool result = Wait(taskCount);
    result = result && task_execution_sequence.size() > 1;
    if (result) {
        result = result && applyExecuted(outerSetting, innerSetting);
        result = result && OuterTaskExecuted(outerSetting);
        result = result && InnerTaskExecuted(innerSetting);
        std::vector<size_t> outerTaskIndex;
        std::vector<std::vector<size_t>> innerTaskIndex;
        GetTaskIndex(outerTaskIndex, innerTaskIndex);
        result = result && IsAscend(outerTaskIndex);
    }
    TestUtils::PublishEvent(g_EVENT_RESP_FIRST, code, std::to_string(result));
}

// level1:parallel, sync level2:parallel, group wait
void MainAbility::ParallelCase5(int code)
{
    APP_LOGI("-- -- -- -- -- --MainAbility::%{public}s", __FUNCTION__);
    Reset();
    TestSetting outerSetting;
    TestSetting innerSetting;
    outerSetting.dispatcher = TestDispatcher::PARALLEL;
    innerSetting.dispatcher = TestDispatcher::PARALLEL;
    innerSetting.group_wait = true;
    innerSetting.group_timeout = groupWait;
    innerSetting.create_group = true;
    outerSetting.op = TestOperation::SYNC;
    innerSetting.op = TestOperation::ASYNC_GROUP;
    int taskCount = Dispatch(outerSetting, innerSetting);
    bool result = Wait(taskCount);
    result = result && task_execution_sequence.size() > 1;
    if (result) {
        result = result && applyExecuted(outerSetting, innerSetting);
        result = result && OuterTaskExecuted(outerSetting);
        result = result && InnerTaskExecuted(innerSetting);
        std::vector<size_t> outerTaskIndex;
        std::vector<std::vector<size_t>> innerTaskIndex;
        GetTaskIndex(outerTaskIndex, innerTaskIndex);
        result = result && IsAscend(outerTaskIndex);
    }
    TestUtils::PublishEvent(g_EVENT_RESP_FIRST, code, std::to_string(result));
}

// level1:parallel, sync level2:parallel, group notify
void MainAbility::ParallelCase6(int code)
{
    APP_LOGI("-- -- -- -- -- --MainAbility::%{public}s", __FUNCTION__);
    Reset();
    TestSetting outerSetting;
    TestSetting innerSetting;
    outerSetting.dispatcher = TestDispatcher::PARALLEL;
    innerSetting.dispatcher = TestDispatcher::PARALLEL;
    innerSetting.create_group = true;
    innerSetting.group_notify = true;
    outerSetting.op = TestOperation::SYNC;
    innerSetting.op = TestOperation::ASYNC_GROUP;
    int taskCount = Dispatch(outerSetting, innerSetting);
    bool result = Wait(taskCount);
    result = result && task_execution_sequence.size() > 1;
    if (result) {
        result = result && applyExecuted(outerSetting, innerSetting);
        result = result && OuterTaskExecuted(outerSetting);
        result = result && InnerTaskExecuted(innerSetting);
        std::vector<size_t> outerTaskIndex;
        std::vector<std::vector<size_t>> innerTaskIndex;
        GetTaskIndex(outerTaskIndex, innerTaskIndex);
        result = result && IsAscend(outerTaskIndex);
        result = result && InnerExecutedAfter(innerGroupNotifyId);
    }
    TestUtils::PublishEvent(g_EVENT_RESP_FIRST, code, std::to_string(result));
}

// level1:parallel, sync level2:parallel, sync barrier
void MainAbility::ParallelCase7(int code)
{
    APP_LOGI("-- -- -- -- -- --MainAbility::%{public}s", __FUNCTION__);
    Reset();
    TestSetting outerSetting;
    TestSetting innerSetting;
    outerSetting.dispatcher = TestDispatcher::PARALLEL;
    innerSetting.dispatcher = TestDispatcher::PARALLEL;
    innerSetting.sync_barrier = true;
    innerSetting.create_group = true;
    outerSetting.op = TestOperation::SYNC;
    innerSetting.op = TestOperation::ASYNC_GROUP;
    int taskCount = Dispatch(outerSetting, innerSetting);
    bool result = Wait(taskCount);
    result = result && task_execution_sequence.size() > 1;
    if (result) {
        result = result && applyExecuted(outerSetting, innerSetting);
        result = result && OuterTaskExecuted(outerSetting);
        result = result && InnerTaskExecuted(innerSetting);
        std::vector<size_t> outerTaskIndex;
        std::vector<std::vector<size_t>> innerTaskIndex;
        GetTaskIndex(outerTaskIndex, innerTaskIndex);
        result = result && IsAscend(outerTaskIndex);
        result = result && InnerExecutedAfter(innerSyncBarrierId);
    }
    TestUtils::PublishEvent(g_EVENT_RESP_FIRST, code, std::to_string(result));
}

// level1:parallel, sync level2:parallel, async barrier
void MainAbility::ParallelCase8(int code)
{
    APP_LOGI("-- -- -- -- -- --MainAbility::%{public}s", __FUNCTION__);
    Reset();
    TestSetting outerSetting;
    TestSetting innerSetting;
    outerSetting.dispatcher = TestDispatcher::PARALLEL;
    innerSetting.dispatcher = TestDispatcher::PARALLEL;
    innerSetting.async_barrier = true;
    innerSetting.create_group = true;
    outerSetting.op = TestOperation::SYNC;
    innerSetting.op = TestOperation::ASYNC_GROUP;
    int taskCount = Dispatch(outerSetting, innerSetting);
    bool result = Wait(taskCount);
    result = result && task_execution_sequence.size() > 1;
    if (result) {
        result = result && applyExecuted(outerSetting, innerSetting);
        result = result && OuterTaskExecuted(outerSetting);
        result = result && InnerTaskExecuted(innerSetting);
        std::vector<size_t> outerTaskIndex;
        std::vector<std::vector<size_t>> innerTaskIndex;
        GetTaskIndex(outerTaskIndex, innerTaskIndex);
        result = result && IsAscend(outerTaskIndex);
        result = result && InnerExecutedAfter(innerAsyncBarrierId);
    }
    TestUtils::PublishEvent(g_EVENT_RESP_FIRST, code, std::to_string(result));
}

// level1:parallel, sync level2:parallel, apply
void MainAbility::ParallelCase9(int code)
{
    APP_LOGI("-- -- -- -- -- --MainAbility::%{public}s", __FUNCTION__);
    Reset();
    TestSetting outerSetting;
    TestSetting innerSetting;
    outerSetting.dispatcher = TestDispatcher::PARALLEL;
    innerSetting.dispatcher = TestDispatcher::PARALLEL;
    innerSetting.apply = applyNum;
    outerSetting.op = TestOperation::SYNC;
    innerSetting.op = TestOperation::APPLY;
    int taskCount = Dispatch(outerSetting, innerSetting);
    bool result = Wait(taskCount);
    result = result && task_execution_sequence.size() > 1;
    if (result) {
        result = result && applyExecuted(outerSetting, innerSetting);
        result = result && OuterTaskExecuted(outerSetting);
        result = result && InnerTaskExecuted(innerSetting);
        std::vector<size_t> outerTaskIndex;
        std::vector<std::vector<size_t>> innerTaskIndex;
        GetTaskIndex(outerTaskIndex, innerTaskIndex);
        result = result && IsAscend(outerTaskIndex);
    }
    TestUtils::PublishEvent(g_EVENT_RESP_FIRST, code, std::to_string(result));
}

// level1:parallel, sync level2:serial, sync
void MainAbility::ParallelCase10(int code)
{
    APP_LOGI("-- -- -- -- -- --MainAbility::%{public}s", __FUNCTION__);
    Reset();
    TestSetting outerSetting;
    TestSetting innerSetting;
    outerSetting.dispatcher = TestDispatcher::PARALLEL;
    innerSetting.dispatcher = TestDispatcher::SERIAL;
    outerSetting.op = TestOperation::SYNC;
    innerSetting.op = TestOperation::SYNC;
    int taskCount = Dispatch(outerSetting, innerSetting);
    bool result = Wait(taskCount);
    result = result && task_execution_sequence.size() > 1;
    if (result) {
        result = result && applyExecuted(outerSetting, innerSetting);
        result = result && OuterTaskExecuted(outerSetting);
        result = result && InnerTaskExecuted(innerSetting);
        std::vector<size_t> outerTaskIndex;
        std::vector<std::vector<size_t>> innerTaskIndex;
        GetTaskIndex(outerTaskIndex, innerTaskIndex);
        result = result && IsAscend(outerTaskIndex);
        for (int i = 0; i < testTaskCount; i++) {
            result = result && IsAscend(innerTaskIndex[i]);
        }
    }
    TestUtils::PublishEvent(g_EVENT_RESP_FIRST, code, std::to_string(result));
}

// level1:parallel, sync level2:serial, async
void MainAbility::ParallelCase11(int code)
{
    APP_LOGI("-- -- -- -- -- --MainAbility::%{public}s", __FUNCTION__);
    Reset();
    TestSetting outerSetting;
    TestSetting innerSetting;
    outerSetting.dispatcher = TestDispatcher::PARALLEL;
    innerSetting.dispatcher = TestDispatcher::SERIAL;
    outerSetting.op = TestOperation::SYNC;
    innerSetting.op = TestOperation::ASYNC;
    int taskCount = Dispatch(outerSetting, innerSetting);
    bool result = Wait(taskCount);
    result = result && task_execution_sequence.size() > 1;
    if (result) {
        result = result && applyExecuted(outerSetting, innerSetting);
        result = result && OuterTaskExecuted(outerSetting);
        result = result && InnerTaskExecuted(innerSetting);
        std::vector<size_t> outerTaskIndex;
        std::vector<std::vector<size_t>> innerTaskIndex;
        GetTaskIndex(outerTaskIndex, innerTaskIndex);
        result = result && IsAscend(outerTaskIndex);
        for (int i = 0; i < testTaskCount; i++) {
            result = result && IsAscend(innerTaskIndex[i]);
        }
    }
    TestUtils::PublishEvent(g_EVENT_RESP_FIRST, code, std::to_string(result));
}

// level1:parallel, sync level2:serial, delay
void MainAbility::ParallelCase12(int code)
{
    APP_LOGI("-- -- -- -- -- --MainAbility::%{public}s", __FUNCTION__);
    Reset();
    TestSetting outerSetting;
    TestSetting innerSetting;
    outerSetting.dispatcher = TestDispatcher::PARALLEL;
    innerSetting.dispatcher = TestDispatcher::SERIAL;
    innerSetting.delay = delayMs;
    outerSetting.op = TestOperation::SYNC;
    innerSetting.op = TestOperation::DELAY;
    int taskCount = Dispatch(outerSetting, innerSetting);
    bool result = Wait(taskCount);
    result = result && task_execution_sequence.size() > 1;
    if (result) {
        result = result && applyExecuted(outerSetting, innerSetting);
        result = result && OuterTaskExecuted(outerSetting);
        result = result && InnerTaskExecuted(innerSetting);
        std::vector<size_t> outerTaskIndex;
        std::vector<std::vector<size_t>> innerTaskIndex;
        GetTaskIndex(outerTaskIndex, innerTaskIndex);
        result = result && IsAscend(outerTaskIndex);
        for (int i = 0; i < testTaskCount; i++) {
            result = result && IsAscend(innerTaskIndex[i]);
        }
    }
    TestUtils::PublishEvent(g_EVENT_RESP_FIRST, code, std::to_string(result));
}

// level1:parallel, sync level2:serial, apply
void MainAbility::ParallelCase13(int code)
{
    APP_LOGI("-- -- -- -- -- --MainAbility::%{public}s", __FUNCTION__);
    Reset();
    TestSetting outerSetting;
    TestSetting innerSetting;
    outerSetting.dispatcher = TestDispatcher::PARALLEL;
    innerSetting.dispatcher = TestDispatcher::SERIAL;
    innerSetting.apply = applyNum;
    outerSetting.op = TestOperation::SYNC;
    innerSetting.op = TestOperation::APPLY;
    int taskCount = Dispatch(outerSetting, innerSetting);
    bool result = Wait(taskCount);
    result = result && task_execution_sequence.size() > 1;
    if (result) {
        result = result && applyExecuted(outerSetting, innerSetting);
        result = result && OuterTaskExecuted(outerSetting);
        result = result && InnerTaskExecuted(innerSetting);
        std::vector<size_t> outerTaskIndex;
        std::vector<std::vector<size_t>> innerTaskIndex;
        GetTaskIndex(outerTaskIndex, innerTaskIndex);
        result = result && IsAscend(outerTaskIndex);
    }
    TestUtils::PublishEvent(g_EVENT_RESP_FIRST, code, std::to_string(result));
}

// level1:parallel, async level2:parallel, sync
void MainAbility::ParallelCase14(int code)
{
    APP_LOGI("-- -- -- -- -- --MainAbility::%{public}s", __FUNCTION__);
    Reset();
    TestSetting outerSetting;
    TestSetting innerSetting;
    outerSetting.dispatcher = TestDispatcher::PARALLEL;
    innerSetting.dispatcher = TestDispatcher::PARALLEL;
    outerSetting.op = TestOperation::ASYNC;
    innerSetting.op = TestOperation::SYNC;
    int taskCount = Dispatch(outerSetting, innerSetting);
    bool result = Wait(taskCount);
    result = result && task_execution_sequence.size() > 1;
    if (result) {
        result = result && applyExecuted(outerSetting, innerSetting);
        result = result && OuterTaskExecuted(outerSetting);
        result = result && InnerTaskExecuted(innerSetting);
        std::vector<size_t> outerTaskIndex;
        std::vector<std::vector<size_t>> innerTaskIndex;
        GetTaskIndex(outerTaskIndex, innerTaskIndex);
        for (int i = 0; i < testTaskCount; i++) {
            result = result && IsAscend(innerTaskIndex[i]);
        }
    }
    TestUtils::PublishEvent(g_EVENT_RESP_FIRST, code, std::to_string(result));
}

// level1:parallel, async level2:parallel, async
void MainAbility::ParallelCase15(int code)
{
    APP_LOGI("-- -- -- -- -- --MainAbility::%{public}s", __FUNCTION__);
    Reset();
    TestSetting outerSetting;
    TestSetting innerSetting;
    outerSetting.dispatcher = TestDispatcher::PARALLEL;
    innerSetting.dispatcher = TestDispatcher::PARALLEL;
    outerSetting.op = TestOperation::ASYNC;
    innerSetting.op = TestOperation::ASYNC;
    int taskCount = Dispatch(outerSetting, innerSetting);
    bool result = Wait(taskCount);
    result = result && task_execution_sequence.size() > 1;
    if (result) {
        result = result && applyExecuted(outerSetting, innerSetting);
        result = result && OuterTaskExecuted(outerSetting);
        result = result && InnerTaskExecuted(innerSetting);
    }
    TestUtils::PublishEvent(g_EVENT_RESP_FIRST, code, std::to_string(result));
}

// level1:parallel, async level2:parallel, delay
void MainAbility::ParallelCase16(int code)
{
    APP_LOGI("-- -- -- -- -- --MainAbility::%{public}s", __FUNCTION__);
    Reset();
    TestSetting outerSetting;
    TestSetting innerSetting;
    outerSetting.dispatcher = TestDispatcher::PARALLEL;
    innerSetting.dispatcher = TestDispatcher::PARALLEL;
    innerSetting.delay = delayMs;
    outerSetting.op = TestOperation::ASYNC;
    innerSetting.op = TestOperation::DELAY;
    int taskCount = Dispatch(outerSetting, innerSetting);
    bool result = Wait(taskCount);
    result = result && task_execution_sequence.size() > 1;
    if (result) {
        result = result && applyExecuted(outerSetting, innerSetting);
        result = result && OuterTaskExecuted(outerSetting);
        result = result && InnerTaskExecuted(innerSetting);
    }
    TestUtils::PublishEvent(g_EVENT_RESP_FIRST, code, std::to_string(result));
}

// level1:parallel, async level2:parallel, group
void MainAbility::ParallelCase17(int code)
{
    APP_LOGI("-- -- -- -- -- --MainAbility::%{public}s", __FUNCTION__);
    Reset();
    TestSetting outerSetting;
    TestSetting innerSetting;
    outerSetting.dispatcher = TestDispatcher::PARALLEL;
    innerSetting.dispatcher = TestDispatcher::PARALLEL;
    innerSetting.create_group = true;
    outerSetting.op = TestOperation::ASYNC;
    innerSetting.op = TestOperation::ASYNC_GROUP;
    int taskCount = Dispatch(outerSetting, innerSetting);
    bool result = Wait(taskCount);
    result = result && task_execution_sequence.size() > 1;
    if (result) {
        result = result && applyExecuted(outerSetting, innerSetting);
        result = result && OuterTaskExecuted(outerSetting);
        result = result && InnerTaskExecuted(innerSetting);
    }
    TestUtils::PublishEvent(g_EVENT_RESP_FIRST, code, std::to_string(result));
}

// level1:parallel, async level2:parallel, group wait
void MainAbility::ParallelCase18(int code)
{
    APP_LOGI("-- -- -- -- -- --MainAbility::%{public}s", __FUNCTION__);
    Reset();
    TestSetting outerSetting;
    TestSetting innerSetting;
    outerSetting.dispatcher = TestDispatcher::PARALLEL;
    innerSetting.dispatcher = TestDispatcher::PARALLEL;
    innerSetting.group_wait = true;
    innerSetting.group_timeout = groupWait;
    innerSetting.create_group = true;
    outerSetting.op = TestOperation::ASYNC;
    innerSetting.op = TestOperation::ASYNC_GROUP;
    int taskCount = Dispatch(outerSetting, innerSetting);
    bool result = Wait(taskCount);
    result = result && task_execution_sequence.size() > 1;
    if (result) {
        result = result && applyExecuted(outerSetting, innerSetting);
        result = result && OuterTaskExecuted(outerSetting);
        result = result && InnerTaskExecuted(innerSetting);
    }
    TestUtils::PublishEvent(g_EVENT_RESP_FIRST, code, std::to_string(result));
}

// level1:parallel, async level2:parallel, group notify
void MainAbility::ParallelCase19(int code)
{
    APP_LOGI("-- -- -- -- -- --MainAbility::%{public}s", __FUNCTION__);
    Reset();
    TestSetting outerSetting;
    TestSetting innerSetting;
    outerSetting.dispatcher = TestDispatcher::PARALLEL;
    innerSetting.dispatcher = TestDispatcher::PARALLEL;
    innerSetting.create_group = true;
    innerSetting.group_notify = true;
    outerSetting.op = TestOperation::ASYNC;
    innerSetting.op = TestOperation::ASYNC_GROUP;
    int taskCount = Dispatch(outerSetting, innerSetting);
    bool result = Wait(taskCount);
    result = result && task_execution_sequence.size() > 1;
    if (result) {
        result = result && applyExecuted(outerSetting, innerSetting);
        result = result && OuterTaskExecuted(outerSetting);
        result = result && InnerTaskExecuted(innerSetting);
        result = result && InnerExecutedAfter(innerGroupNotifyId);
    }
    TestUtils::PublishEvent(g_EVENT_RESP_FIRST, code, std::to_string(result));
}

// level1:parallel, async level2:parallel, sync barrier
void MainAbility::ParallelCase20(int code)
{
    APP_LOGI("-- -- -- -- -- --MainAbility::%{public}s", __FUNCTION__);
    Reset();
    TestSetting outerSetting;
    TestSetting innerSetting;
    outerSetting.dispatcher = TestDispatcher::PARALLEL;
    innerSetting.dispatcher = TestDispatcher::PARALLEL;
    innerSetting.sync_barrier = true;
    innerSetting.create_group = true;
    outerSetting.op = TestOperation::ASYNC;
    innerSetting.op = TestOperation::ASYNC_GROUP;
    int taskCount = Dispatch(outerSetting, innerSetting);
    bool result = Wait(taskCount);
    result = result && task_execution_sequence.size() > 1;
    if (result) {
        result = result && applyExecuted(outerSetting, innerSetting);
        result = result && OuterTaskExecuted(outerSetting);
        result = result && InnerTaskExecuted(innerSetting);
        result = result && InnerExecutedAfter(innerSyncBarrierId);
    }
    TestUtils::PublishEvent(g_EVENT_RESP_FIRST, code, std::to_string(result));
}

// level1:parallel, async level2:parallel, async barrier
void MainAbility::ParallelCase21(int code)
{
    APP_LOGI("-- -- -- -- -- --MainAbility::%{public}s", __FUNCTION__);
    Reset();
    TestSetting outerSetting;
    TestSetting innerSetting;
    outerSetting.dispatcher = TestDispatcher::PARALLEL;
    innerSetting.dispatcher = TestDispatcher::PARALLEL;
    innerSetting.async_barrier = true;
    innerSetting.create_group = true;
    outerSetting.op = TestOperation::ASYNC;
    innerSetting.op = TestOperation::ASYNC_GROUP;
    int taskCount = Dispatch(outerSetting, innerSetting);
    bool result = Wait(taskCount);
    result = result && task_execution_sequence.size() > 1;
    if (result) {
        result = result && applyExecuted(outerSetting, innerSetting);
        result = result && OuterTaskExecuted(outerSetting);
        result = result && InnerTaskExecuted(innerSetting);
        result = result && InnerExecutedAfter(innerAsyncBarrierId);
    }
    TestUtils::PublishEvent(g_EVENT_RESP_FIRST, code, std::to_string(result));
}

// level1:parallel, async level2:parallel, apply
void MainAbility::ParallelCase22(int code)
{
    APP_LOGI("-- -- -- -- -- --MainAbility::%{public}s", __FUNCTION__);
    Reset();
    TestSetting outerSetting;
    TestSetting innerSetting;
    outerSetting.dispatcher = TestDispatcher::PARALLEL;
    innerSetting.dispatcher = TestDispatcher::PARALLEL;
    innerSetting.apply = applyNum;
    outerSetting.op = TestOperation::ASYNC;
    innerSetting.op = TestOperation::APPLY;
    int taskCount = Dispatch(outerSetting, innerSetting);
    bool result = Wait(taskCount);
    result = result && task_execution_sequence.size() > 1;
    if (result) {
        result = result && applyExecuted(outerSetting, innerSetting);
        result = result && OuterTaskExecuted(outerSetting);
        result = result && InnerTaskExecuted(innerSetting);
    }
    TestUtils::PublishEvent(g_EVENT_RESP_FIRST, code, std::to_string(result));
}

// level1:parallel, async level2:serial, sync
void MainAbility::ParallelCase23(int code)
{
    APP_LOGI("-- -- -- -- -- --MainAbility::%{public}s", __FUNCTION__);
    Reset();
    TestSetting outerSetting;
    TestSetting innerSetting;
    outerSetting.dispatcher = TestDispatcher::PARALLEL;
    innerSetting.dispatcher = TestDispatcher::SERIAL;
    outerSetting.op = TestOperation::ASYNC;
    innerSetting.op = TestOperation::SYNC;
    int taskCount = Dispatch(outerSetting, innerSetting);
    bool result = Wait(taskCount);
    result = result && task_execution_sequence.size() > 1;
    if (result) {
        result = result && applyExecuted(outerSetting, innerSetting);
        result = result && OuterTaskExecuted(outerSetting);
        result = result && InnerTaskExecuted(innerSetting);
        std::vector<size_t> outerTaskIndex;
        std::vector<std::vector<size_t>> innerTaskIndex;
        GetTaskIndex(outerTaskIndex, innerTaskIndex);
        for (int i = 0; i < testTaskCount; i++) {
            result = result && IsAscend(innerTaskIndex[i]);
        }
    }
    TestUtils::PublishEvent(g_EVENT_RESP_FIRST, code, std::to_string(result));
}

// level1:parallel, async level2:serial, async
void MainAbility::ParallelCase24(int code)
{
    APP_LOGI("-- -- -- -- -- --MainAbility::%{public}s", __FUNCTION__);
    Reset();
    TestSetting outerSetting;
    TestSetting innerSetting;
    outerSetting.dispatcher = TestDispatcher::PARALLEL;
    innerSetting.dispatcher = TestDispatcher::SERIAL;
    outerSetting.op = TestOperation::ASYNC;
    innerSetting.op = TestOperation::ASYNC;
    int taskCount = Dispatch(outerSetting, innerSetting);
    bool result = Wait(taskCount);
    result = result && task_execution_sequence.size() > 1;
    if (result) {
        result = result && applyExecuted(outerSetting, innerSetting);
        result = result && OuterTaskExecuted(outerSetting);
        result = result && InnerTaskExecuted(innerSetting);
        std::vector<size_t> outerTaskIndex;
        std::vector<std::vector<size_t>> innerTaskIndex;
        GetTaskIndex(outerTaskIndex, innerTaskIndex);
        for (int i = 0; i < testTaskCount; i++) {
            result = result && IsAscend(innerTaskIndex[i]);
        }
    }
    TestUtils::PublishEvent(g_EVENT_RESP_FIRST, code, std::to_string(result));
}

// level1:parallel, async level2:serial, delay
void MainAbility::ParallelCase25(int code)
{
    APP_LOGI("-- -- -- -- -- --MainAbility::%{public}s", __FUNCTION__);
    Reset();
    TestSetting outerSetting;
    TestSetting innerSetting;
    outerSetting.dispatcher = TestDispatcher::PARALLEL;
    innerSetting.dispatcher = TestDispatcher::SERIAL;
    innerSetting.delay = delayMs;
    outerSetting.op = TestOperation::ASYNC;
    innerSetting.op = TestOperation::DELAY;
    int taskCount = Dispatch(outerSetting, innerSetting);
    bool result = Wait(taskCount);
    result = result && task_execution_sequence.size() > 1;
    if (result) {
        result = result && applyExecuted(outerSetting, innerSetting);
        result = result && OuterTaskExecuted(outerSetting);
        result = result && InnerTaskExecuted(innerSetting);
        std::vector<size_t> outerTaskIndex;
        std::vector<std::vector<size_t>> innerTaskIndex;
        GetTaskIndex(outerTaskIndex, innerTaskIndex);
        for (int i = 0; i < testTaskCount; i++) {
            result = result && IsAscend(innerTaskIndex[i]);
        }
    }
    TestUtils::PublishEvent(g_EVENT_RESP_FIRST, code, std::to_string(result));
}

// level1:parallel, async level2:serial, apply
void MainAbility::ParallelCase26(int code)
{
    APP_LOGI("-- -- -- -- -- --MainAbility::%{public}s", __FUNCTION__);
    Reset();
    TestSetting outerSetting;
    TestSetting innerSetting;
    outerSetting.dispatcher = TestDispatcher::PARALLEL;
    innerSetting.dispatcher = TestDispatcher::SERIAL;
    innerSetting.apply = applyNum;
    outerSetting.op = TestOperation::ASYNC;
    innerSetting.op = TestOperation::APPLY;
    int taskCount = Dispatch(outerSetting, innerSetting);
    bool result = Wait(taskCount);
    result = result && task_execution_sequence.size() > 1;
    if (result) {
        result = result && applyExecuted(outerSetting, innerSetting);
        result = result && OuterTaskExecuted(outerSetting);
        result = result && InnerTaskExecuted(innerSetting);
    }
    TestUtils::PublishEvent(g_EVENT_RESP_FIRST, code, std::to_string(result));
}

// level1:parallel, delay level2:parallel, sync
void MainAbility::ParallelCase27(int code)
{
    APP_LOGI("-- -- -- -- -- --MainAbility::%{public}s", __FUNCTION__);
    Reset();
    TestSetting outerSetting;
    TestSetting innerSetting;
    outerSetting.dispatcher = TestDispatcher::PARALLEL;
    innerSetting.dispatcher = TestDispatcher::PARALLEL;
    outerSetting.delay = delayMs;
    outerSetting.op = TestOperation::DELAY;
    innerSetting.op = TestOperation::SYNC;
    int taskCount = Dispatch(outerSetting, innerSetting);
    bool result = Wait(taskCount);
    result = result && task_execution_sequence.size() > 1;
    if (result) {
        result = result && applyExecuted(outerSetting, innerSetting);
        result = result && OuterTaskExecuted(outerSetting);
        result = result && InnerTaskExecuted(innerSetting);
        std::vector<size_t> outerTaskIndex;
        std::vector<std::vector<size_t>> innerTaskIndex;
        GetTaskIndex(outerTaskIndex, innerTaskIndex);
        for (int i = 0; i < testTaskCount; i++) {
            result = result && IsAscend(innerTaskIndex[i]);
        }
    }
    TestUtils::PublishEvent(g_EVENT_RESP_FIRST, code, std::to_string(result));
}

// level1:parallel, delay level2:parallel, async
void MainAbility::ParallelCase28(int code)
{
    APP_LOGI("-- -- -- -- -- --MainAbility::%{public}s", __FUNCTION__);
    Reset();
    TestSetting outerSetting;
    TestSetting innerSetting;
    outerSetting.dispatcher = TestDispatcher::PARALLEL;
    innerSetting.dispatcher = TestDispatcher::PARALLEL;
    outerSetting.delay = delayMs;
    outerSetting.op = TestOperation::DELAY;
    innerSetting.op = TestOperation::ASYNC;
    int taskCount = Dispatch(outerSetting, innerSetting);
    bool result = Wait(taskCount);
    result = result && task_execution_sequence.size() > 1;
    if (result) {
        result = result && applyExecuted(outerSetting, innerSetting);
        result = result && OuterTaskExecuted(outerSetting);
        result = result && InnerTaskExecuted(innerSetting);
    }
    TestUtils::PublishEvent(g_EVENT_RESP_FIRST, code, std::to_string(result));
}

// level1:parallel, delay level2:parallel, delay
void MainAbility::ParallelCase29(int code)
{
    APP_LOGI("-- -- -- -- -- --MainAbility::%{public}s", __FUNCTION__);
    Reset();
    TestSetting outerSetting;
    TestSetting innerSetting;
    outerSetting.dispatcher = TestDispatcher::PARALLEL;
    innerSetting.dispatcher = TestDispatcher::PARALLEL;
    outerSetting.delay = delayMs;
    innerSetting.delay = delayMs;
    outerSetting.op = TestOperation::DELAY;
    innerSetting.op = TestOperation::DELAY;
    int taskCount = Dispatch(outerSetting, innerSetting);
    bool result = Wait(taskCount);
    result = result && task_execution_sequence.size() > 1;
    if (result) {
        result = result && applyExecuted(outerSetting, innerSetting);
        result = result && OuterTaskExecuted(outerSetting);
        result = result && InnerTaskExecuted(innerSetting);
    }
    TestUtils::PublishEvent(g_EVENT_RESP_FIRST, code, std::to_string(result));
}

// level1:parallel, delay level2:parallel, group
void MainAbility::ParallelCase30(int code)
{
    APP_LOGI("-- -- -- -- -- --MainAbility::%{public}s", __FUNCTION__);
    Reset();
    TestSetting outerSetting;
    TestSetting innerSetting;
    outerSetting.dispatcher = TestDispatcher::PARALLEL;
    innerSetting.dispatcher = TestDispatcher::PARALLEL;
    outerSetting.delay = delayMs;
    innerSetting.create_group = true;
    outerSetting.op = TestOperation::DELAY;
    innerSetting.op = TestOperation::ASYNC_GROUP;
    int taskCount = Dispatch(outerSetting, innerSetting);
    bool result = Wait(taskCount);
    result = result && task_execution_sequence.size() > 1;
    if (result) {
        result = result && applyExecuted(outerSetting, innerSetting);
        result = result && OuterTaskExecuted(outerSetting);
        result = result && InnerTaskExecuted(innerSetting);
    }
    TestUtils::PublishEvent(g_EVENT_RESP_FIRST, code, std::to_string(result));
}

// level1:parallel, delay level2:parallel, group wait
void MainAbility::ParallelCase31(int code)
{
    APP_LOGI("-- -- -- -- -- --MainAbility::%{public}s", __FUNCTION__);
    Reset();
    TestSetting outerSetting;
    TestSetting innerSetting;
    outerSetting.dispatcher = TestDispatcher::PARALLEL;
    innerSetting.dispatcher = TestDispatcher::PARALLEL;
    innerSetting.group_wait = true;
    innerSetting.group_timeout = groupWait;
    outerSetting.delay = delayMs;
    innerSetting.create_group = true;
    outerSetting.op = TestOperation::DELAY;
    innerSetting.op = TestOperation::ASYNC_GROUP;
    int taskCount = Dispatch(outerSetting, innerSetting);
    bool result = Wait(taskCount);
    result = result && task_execution_sequence.size() > 1;
    if (result) {
        result = result && applyExecuted(outerSetting, innerSetting);
        result = result && OuterTaskExecuted(outerSetting);
        result = result && InnerTaskExecuted(innerSetting);
    }
    TestUtils::PublishEvent(g_EVENT_RESP_FIRST, code, std::to_string(result));
}

// level1:parallel, delay level2:parallel, group notify
void MainAbility::ParallelCase32(int code)
{
    APP_LOGI("-- -- -- -- -- --MainAbility::%{public}s", __FUNCTION__);
    Reset();
    TestSetting outerSetting;
    TestSetting innerSetting;
    outerSetting.dispatcher = TestDispatcher::PARALLEL;
    innerSetting.dispatcher = TestDispatcher::PARALLEL;
    outerSetting.delay = delayMs;
    innerSetting.create_group = true;
    innerSetting.group_notify = true;
    outerSetting.op = TestOperation::DELAY;
    innerSetting.op = TestOperation::ASYNC_GROUP;
    int taskCount = Dispatch(outerSetting, innerSetting);
    bool result = Wait(taskCount);
    result = result && task_execution_sequence.size() > 1;
    if (result) {
        result = result && applyExecuted(outerSetting, innerSetting);
        result = result && OuterTaskExecuted(outerSetting);
        result = result && InnerTaskExecuted(innerSetting);
        result = result && InnerExecutedAfter(innerGroupNotifyId);
    }
    TestUtils::PublishEvent(g_EVENT_RESP_FIRST, code, std::to_string(result));
}

// level1:parallel, delay level2:parallel, sync barrier
void MainAbility::ParallelCase33(int code)
{
    APP_LOGI("-- -- -- -- -- --MainAbility::%{public}s", __FUNCTION__);
    Reset();
    TestSetting outerSetting;
    TestSetting innerSetting;
    outerSetting.dispatcher = TestDispatcher::PARALLEL;
    innerSetting.dispatcher = TestDispatcher::PARALLEL;
    outerSetting.delay = delayMs;
    innerSetting.sync_barrier = true;
    innerSetting.create_group = true;
    outerSetting.op = TestOperation::DELAY;
    innerSetting.op = TestOperation::ASYNC_GROUP;
    int taskCount = Dispatch(outerSetting, innerSetting);
    bool result = Wait(taskCount);
    result = result && task_execution_sequence.size() > 1;
    if (result) {
        result = result && applyExecuted(outerSetting, innerSetting);
        result = result && OuterTaskExecuted(outerSetting);
        result = result && InnerTaskExecuted(innerSetting);
        result = result && InnerExecutedAfter(innerSyncBarrierId);
    }
    TestUtils::PublishEvent(g_EVENT_RESP_FIRST, code, std::to_string(result));
}

// level1:parallel, delay level2:parallel, async barrier
void MainAbility::ParallelCase34(int code)
{
    APP_LOGI("-- -- -- -- -- --MainAbility::%{public}s", __FUNCTION__);
    Reset();
    TestSetting outerSetting;
    TestSetting innerSetting;
    outerSetting.dispatcher = TestDispatcher::PARALLEL;
    innerSetting.dispatcher = TestDispatcher::PARALLEL;
    outerSetting.delay = delayMs;
    innerSetting.async_barrier = true;
    innerSetting.create_group = true;
    outerSetting.op = TestOperation::DELAY;
    innerSetting.op = TestOperation::ASYNC_GROUP;
    int taskCount = Dispatch(outerSetting, innerSetting);
    bool result = Wait(taskCount);
    result = result && task_execution_sequence.size() > 1;
    if (result) {
        result = result && applyExecuted(outerSetting, innerSetting);
        result = result && OuterTaskExecuted(outerSetting);
        result = result && InnerTaskExecuted(innerSetting);
        result = result && InnerExecutedAfter(innerAsyncBarrierId);
    }
    TestUtils::PublishEvent(g_EVENT_RESP_FIRST, code, std::to_string(result));
}

// level1:parallel, delay level2:parallel, apply
void MainAbility::ParallelCase35(int code)
{
    APP_LOGI("-- -- -- -- -- --MainAbility::%{public}s", __FUNCTION__);
    Reset();
    TestSetting outerSetting;
    TestSetting innerSetting;
    outerSetting.dispatcher = TestDispatcher::PARALLEL;
    innerSetting.dispatcher = TestDispatcher::PARALLEL;
    outerSetting.delay = delayMs;
    innerSetting.apply = applyNum;
    outerSetting.op = TestOperation::DELAY;
    innerSetting.op = TestOperation::APPLY;
    int taskCount = Dispatch(outerSetting, innerSetting);
    bool result = Wait(taskCount);
    result = result && task_execution_sequence.size() > 1;
    if (result) {
        result = result && applyExecuted(outerSetting, innerSetting);
        result = result && OuterTaskExecuted(outerSetting);
        result = result && InnerTaskExecuted(innerSetting);
    }
    TestUtils::PublishEvent(g_EVENT_RESP_FIRST, code, std::to_string(result));
}

// level1:parallel, delay level2:serial, sync
void MainAbility::ParallelCase36(int code)
{
    APP_LOGI("-- -- -- -- -- --MainAbility::%{public}s", __FUNCTION__);
    Reset();
    TestSetting outerSetting;
    TestSetting innerSetting;
    outerSetting.dispatcher = TestDispatcher::PARALLEL;
    innerSetting.dispatcher = TestDispatcher::SERIAL;
    outerSetting.delay = delayMs;
    outerSetting.op = TestOperation::DELAY;
    innerSetting.op = TestOperation::SYNC;
    int taskCount = Dispatch(outerSetting, innerSetting);
    bool result = Wait(taskCount);
    result = result && task_execution_sequence.size() > 1;
    if (result) {
        result = result && applyExecuted(outerSetting, innerSetting);
        result = result && OuterTaskExecuted(outerSetting);
        result = result && InnerTaskExecuted(innerSetting);
        std::vector<size_t> outerTaskIndex;
        std::vector<std::vector<size_t>> innerTaskIndex;
        GetTaskIndex(outerTaskIndex, innerTaskIndex);
        for (int i = 0; i < testTaskCount; i++) {
            result = result && IsAscend(innerTaskIndex[i]);
        }
    }
    TestUtils::PublishEvent(g_EVENT_RESP_FIRST, code, std::to_string(result));
}

// level1:parallel, delay level2:serial, async
void MainAbility::ParallelCase37(int code)
{
    APP_LOGI("-- -- -- -- -- --MainAbility::%{public}s", __FUNCTION__);
    Reset();
    TestSetting outerSetting;
    TestSetting innerSetting;
    outerSetting.dispatcher = TestDispatcher::PARALLEL;
    innerSetting.dispatcher = TestDispatcher::SERIAL;
    outerSetting.delay = delayMs;
    outerSetting.op = TestOperation::DELAY;
    innerSetting.op = TestOperation::ASYNC;
    int taskCount = Dispatch(outerSetting, innerSetting);
    bool result = Wait(taskCount);
    result = result && task_execution_sequence.size() > 1;
    if (result) {
        result = result && applyExecuted(outerSetting, innerSetting);
        result = result && OuterTaskExecuted(outerSetting);
        result = result && InnerTaskExecuted(innerSetting);
        std::vector<size_t> outerTaskIndex;
        std::vector<std::vector<size_t>> innerTaskIndex;
        GetTaskIndex(outerTaskIndex, innerTaskIndex);
        for (int i = 0; i < testTaskCount; i++) {
            result = result && IsAscend(innerTaskIndex[i]);
        }
    }
    TestUtils::PublishEvent(g_EVENT_RESP_FIRST, code, std::to_string(result));
}

// level1:parallel, delay level2:serial, delay
void MainAbility::ParallelCase38(int code)
{
    APP_LOGI("-- -- -- -- -- --MainAbility::%{public}s", __FUNCTION__);
    Reset();
    TestSetting outerSetting;
    TestSetting innerSetting;
    outerSetting.dispatcher = TestDispatcher::PARALLEL;
    innerSetting.dispatcher = TestDispatcher::SERIAL;
    outerSetting.delay = delayMs;
    innerSetting.delay = delayMs;
    outerSetting.op = TestOperation::DELAY;
    innerSetting.op = TestOperation::DELAY;
    int taskCount = Dispatch(outerSetting, innerSetting);
    bool result = Wait(taskCount);
    result = result && task_execution_sequence.size() > 1;
    if (result) {
        result = result && applyExecuted(outerSetting, innerSetting);
        result = result && OuterTaskExecuted(outerSetting);
        result = result && InnerTaskExecuted(innerSetting);
        std::vector<size_t> outerTaskIndex;
        std::vector<std::vector<size_t>> innerTaskIndex;
        GetTaskIndex(outerTaskIndex, innerTaskIndex);
        for (int i = 0; i < testTaskCount; i++) {
            result = result && IsAscend(innerTaskIndex[i]);
        }
    }
    TestUtils::PublishEvent(g_EVENT_RESP_FIRST, code, std::to_string(result));
}

// level1:parallel, delay level2:serial, apply
void MainAbility::ParallelCase39(int code)
{
    APP_LOGI("-- -- -- -- -- --MainAbility::%{public}s", __FUNCTION__);
    Reset();
    TestSetting outerSetting;
    TestSetting innerSetting;
    outerSetting.dispatcher = TestDispatcher::PARALLEL;
    innerSetting.dispatcher = TestDispatcher::SERIAL;
    outerSetting.delay = delayMs;
    innerSetting.apply = applyNum;
    outerSetting.op = TestOperation::DELAY;
    innerSetting.op = TestOperation::APPLY;
    int taskCount = Dispatch(outerSetting, innerSetting);
    bool result = Wait(taskCount);
    result = result && task_execution_sequence.size() > 1;
    if (result) {
        result = result && applyExecuted(outerSetting, innerSetting);
        result = result && OuterTaskExecuted(outerSetting);
        result = result && InnerTaskExecuted(innerSetting);
    }
    TestUtils::PublishEvent(g_EVENT_RESP_FIRST, code, std::to_string(result));
}

// level1:parallel, group level2:parallel, sync
void MainAbility::ParallelCase40(int code)
{
    APP_LOGI("-- -- -- -- -- --MainAbility::%{public}s", __FUNCTION__);
    Reset();
    TestSetting outerSetting;
    TestSetting innerSetting;
    outerSetting.dispatcher = TestDispatcher::PARALLEL;
    innerSetting.dispatcher = TestDispatcher::PARALLEL;
    outerSetting.create_group = true;
    outerSetting.op = TestOperation::ASYNC_GROUP;
    innerSetting.op = TestOperation::SYNC;
    int taskCount = Dispatch(outerSetting, innerSetting);
    bool result = Wait(taskCount);
    result = result && task_execution_sequence.size() > 1;
    if (result) {
        result = result && applyExecuted(outerSetting, innerSetting);
        result = result && OuterTaskExecuted(outerSetting);
        result = result && InnerTaskExecuted(innerSetting);
        std::vector<size_t> outerTaskIndex;
        std::vector<std::vector<size_t>> innerTaskIndex;
        GetTaskIndex(outerTaskIndex, innerTaskIndex);
        for (int i = 0; i < testTaskCount; i++) {
            result = result && IsAscend(innerTaskIndex[i]);
        }
    }
    TestUtils::PublishEvent(g_EVENT_RESP_FIRST, code, std::to_string(result));
}

// level1:parallel, group level2:parallel, async
void MainAbility::ParallelCase41(int code)
{
    APP_LOGI("-- -- -- -- -- --MainAbility::%{public}s", __FUNCTION__);
    Reset();
    TestSetting outerSetting;
    TestSetting innerSetting;
    outerSetting.dispatcher = TestDispatcher::PARALLEL;
    innerSetting.dispatcher = TestDispatcher::PARALLEL;
    outerSetting.create_group = true;
    outerSetting.op = TestOperation::ASYNC_GROUP;
    innerSetting.op = TestOperation::ASYNC;
    int taskCount = Dispatch(outerSetting, innerSetting);
    bool result = Wait(taskCount);
    result = result && task_execution_sequence.size() > 1;
    if (result) {
        result = result && applyExecuted(outerSetting, innerSetting);
        result = result && OuterTaskExecuted(outerSetting);
        result = result && InnerTaskExecuted(innerSetting);
    }
    TestUtils::PublishEvent(g_EVENT_RESP_FIRST, code, std::to_string(result));
}

// level1:parallel, group level2:parallel, delay
void MainAbility::ParallelCase42(int code)
{
    APP_LOGI("-- -- -- -- -- --MainAbility::%{public}s", __FUNCTION__);
    Reset();
    TestSetting outerSetting;
    TestSetting innerSetting;
    outerSetting.dispatcher = TestDispatcher::PARALLEL;
    innerSetting.dispatcher = TestDispatcher::PARALLEL;
    innerSetting.delay = delayMs;
    outerSetting.create_group = true;
    outerSetting.op = TestOperation::ASYNC_GROUP;
    innerSetting.op = TestOperation::DELAY;
    int taskCount = Dispatch(outerSetting, innerSetting);
    bool result = Wait(taskCount);
    result = result && task_execution_sequence.size() > 1;
    if (result) {
        result = result && applyExecuted(outerSetting, innerSetting);
        result = result && OuterTaskExecuted(outerSetting);
        result = result && InnerTaskExecuted(innerSetting);
    }
    TestUtils::PublishEvent(g_EVENT_RESP_FIRST, code, std::to_string(result));
}

// level1:parallel, group level2:parallel, group
void MainAbility::ParallelCase43(int code)
{
    APP_LOGI("-- -- -- -- -- --MainAbility::%{public}s", __FUNCTION__);
    Reset();
    TestSetting outerSetting;
    TestSetting innerSetting;
    outerSetting.dispatcher = TestDispatcher::PARALLEL;
    innerSetting.dispatcher = TestDispatcher::PARALLEL;
    innerSetting.create_group = true;
    outerSetting.create_group = true;
    outerSetting.op = TestOperation::ASYNC_GROUP;
    innerSetting.op = TestOperation::ASYNC_GROUP;
    int taskCount = Dispatch(outerSetting, innerSetting);
    bool result = Wait(taskCount);
    result = result && task_execution_sequence.size() > 1;
    if (result) {
        result = result && applyExecuted(outerSetting, innerSetting);
        result = result && OuterTaskExecuted(outerSetting);
        result = result && InnerTaskExecuted(innerSetting);
    }
    TestUtils::PublishEvent(g_EVENT_RESP_FIRST, code, std::to_string(result));
}

// level1:parallel, group level2:parallel, group wait
void MainAbility::ParallelCase44(int code)
{
    APP_LOGI("-- -- -- -- -- --MainAbility::%{public}s", __FUNCTION__);
    Reset();
    TestSetting outerSetting;
    TestSetting innerSetting;
    outerSetting.dispatcher = TestDispatcher::PARALLEL;
    innerSetting.dispatcher = TestDispatcher::PARALLEL;
    innerSetting.group_wait = true;
    innerSetting.group_timeout = groupWait;
    innerSetting.create_group = true;
    outerSetting.create_group = true;
    outerSetting.op = TestOperation::ASYNC_GROUP;
    innerSetting.op = TestOperation::ASYNC_GROUP;
    int taskCount = Dispatch(outerSetting, innerSetting);
    bool result = Wait(taskCount);
    result = result && task_execution_sequence.size() > 1;
    if (result) {
        result = result && applyExecuted(outerSetting, innerSetting);
        result = result && OuterTaskExecuted(outerSetting);
        result = result && InnerTaskExecuted(innerSetting);
    }
    TestUtils::PublishEvent(g_EVENT_RESP_FIRST, code, std::to_string(result));
}

// level1:parallel, group level2:parallel, group notify
void MainAbility::ParallelCase45(int code)
{
    APP_LOGI("-- -- -- -- -- --MainAbility::%{public}s", __FUNCTION__);
    Reset();
    TestSetting outerSetting;
    TestSetting innerSetting;
    outerSetting.dispatcher = TestDispatcher::PARALLEL;
    innerSetting.dispatcher = TestDispatcher::PARALLEL;
    innerSetting.create_group = true;
    innerSetting.group_notify = true;
    outerSetting.create_group = true;
    outerSetting.op = TestOperation::ASYNC_GROUP;
    innerSetting.op = TestOperation::ASYNC_GROUP;
    int taskCount = Dispatch(outerSetting, innerSetting);
    bool result = Wait(taskCount);
    result = result && task_execution_sequence.size() > 1;
    if (result) {
        result = result && applyExecuted(outerSetting, innerSetting);
        result = result && OuterTaskExecuted(outerSetting);
        result = result && InnerTaskExecuted(innerSetting);
        result = result && InnerExecutedAfter(innerGroupNotifyId);
    }
    TestUtils::PublishEvent(g_EVENT_RESP_FIRST, code, std::to_string(result));
}

// level1:parallel, group level2:parallel, sync barrier
void MainAbility::ParallelCase46(int code)
{
    APP_LOGI("-- -- -- -- -- --MainAbility::%{public}s", __FUNCTION__);
    Reset();
    TestSetting outerSetting;
    TestSetting innerSetting;
    outerSetting.dispatcher = TestDispatcher::PARALLEL;
    innerSetting.dispatcher = TestDispatcher::PARALLEL;
    innerSetting.sync_barrier = true;
    innerSetting.create_group = true;
    outerSetting.create_group = true;
    outerSetting.op = TestOperation::ASYNC_GROUP;
    innerSetting.op = TestOperation::ASYNC_GROUP;
    int taskCount = Dispatch(outerSetting, innerSetting);
    bool result = Wait(taskCount);
    result = result && task_execution_sequence.size() > 1;
    if (result) {
        result = result && applyExecuted(outerSetting, innerSetting);
        result = result && OuterTaskExecuted(outerSetting);
        result = result && InnerTaskExecuted(innerSetting);
        result = result && InnerExecutedAfter(innerSyncBarrierId);
    }
    TestUtils::PublishEvent(g_EVENT_RESP_FIRST, code, std::to_string(result));
}

// level1:parallel, group level2:parallel, async barrier
void MainAbility::ParallelCase47(int code)
{
    APP_LOGI("-- -- -- -- -- --MainAbility::%{public}s", __FUNCTION__);
    Reset();
    TestSetting outerSetting;
    TestSetting innerSetting;
    outerSetting.dispatcher = TestDispatcher::PARALLEL;
    innerSetting.dispatcher = TestDispatcher::PARALLEL;
    innerSetting.async_barrier = true;
    innerSetting.create_group = true;
    outerSetting.create_group = true;
    outerSetting.op = TestOperation::ASYNC_GROUP;
    innerSetting.op = TestOperation::ASYNC_GROUP;
    int taskCount = Dispatch(outerSetting, innerSetting);
    bool result = Wait(taskCount);
    result = result && task_execution_sequence.size() > 1;
    if (result) {
        result = result && applyExecuted(outerSetting, innerSetting);
        result = result && OuterTaskExecuted(outerSetting);
        result = result && InnerTaskExecuted(innerSetting);
        result = result && InnerExecutedAfter(innerAsyncBarrierId);
    }
    TestUtils::PublishEvent(g_EVENT_RESP_FIRST, code, std::to_string(result));
}

// level1:parallel, group level2:parallel, apply
void MainAbility::ParallelCase48(int code)
{
    APP_LOGI("-- -- -- -- -- --MainAbility::%{public}s", __FUNCTION__);
    Reset();
    TestSetting outerSetting;
    TestSetting innerSetting;
    outerSetting.dispatcher = TestDispatcher::PARALLEL;
    innerSetting.dispatcher = TestDispatcher::PARALLEL;
    innerSetting.apply = applyNum;
    outerSetting.create_group = true;
    outerSetting.op = TestOperation::ASYNC_GROUP;
    innerSetting.op = TestOperation::APPLY;
    int taskCount = Dispatch(outerSetting, innerSetting);
    bool result = Wait(taskCount);
    result = result && task_execution_sequence.size() > 1;
    if (result) {
        result = result && applyExecuted(outerSetting, innerSetting);
        result = result && OuterTaskExecuted(outerSetting);
        result = result && InnerTaskExecuted(innerSetting);
    }
    TestUtils::PublishEvent(g_EVENT_RESP_FIRST, code, std::to_string(result));
}

// level1:parallel, group level2:serial, sync
void MainAbility::ParallelCase49(int code)
{
    APP_LOGI("-- -- -- -- -- --MainAbility::%{public}s", __FUNCTION__);
    Reset();
    TestSetting outerSetting;
    TestSetting innerSetting;
    outerSetting.dispatcher = TestDispatcher::PARALLEL;
    innerSetting.dispatcher = TestDispatcher::SERIAL;
    outerSetting.create_group = true;
    outerSetting.op = TestOperation::ASYNC_GROUP;
    innerSetting.op = TestOperation::SYNC;
    int taskCount = Dispatch(outerSetting, innerSetting);
    bool result = Wait(taskCount);
    result = result && task_execution_sequence.size() > 1;
    if (result) {
        result = result && applyExecuted(outerSetting, innerSetting);
        result = result && OuterTaskExecuted(outerSetting);
        result = result && InnerTaskExecuted(innerSetting);
        std::vector<size_t> outerTaskIndex;
        std::vector<std::vector<size_t>> innerTaskIndex;
        GetTaskIndex(outerTaskIndex, innerTaskIndex);
        for (int i = 0; i < testTaskCount; i++) {
            result = result && IsAscend(innerTaskIndex[i]);
        }
    }
    TestUtils::PublishEvent(g_EVENT_RESP_FIRST, code, std::to_string(result));
}

// level1:parallel, group level2:serial, async
void MainAbility::ParallelCase50(int code)
{
    APP_LOGI("-- -- -- -- -- --MainAbility::%{public}s", __FUNCTION__);
    Reset();
    TestSetting outerSetting;
    TestSetting innerSetting;
    outerSetting.dispatcher = TestDispatcher::PARALLEL;
    innerSetting.dispatcher = TestDispatcher::SERIAL;
    outerSetting.create_group = true;
    outerSetting.op = TestOperation::ASYNC_GROUP;
    innerSetting.op = TestOperation::ASYNC;
    int taskCount = Dispatch(outerSetting, innerSetting);
    bool result = Wait(taskCount);
    result = result && task_execution_sequence.size() > 1;
    if (result) {
        result = result && applyExecuted(outerSetting, innerSetting);
        result = result && OuterTaskExecuted(outerSetting);
        result = result && InnerTaskExecuted(innerSetting);
        std::vector<size_t> outerTaskIndex;
        std::vector<std::vector<size_t>> innerTaskIndex;
        GetTaskIndex(outerTaskIndex, innerTaskIndex);
        for (int i = 0; i < testTaskCount; i++) {
            result = result && IsAscend(innerTaskIndex[i]);
        }
    }
    TestUtils::PublishEvent(g_EVENT_RESP_FIRST, code, std::to_string(result));
}

// level1:parallel, group level2:serial, delay
void MainAbility::ParallelCase51(int code)
{
    APP_LOGI("-- -- -- -- -- --MainAbility::%{public}s", __FUNCTION__);
    Reset();
    TestSetting outerSetting;
    TestSetting innerSetting;
    outerSetting.dispatcher = TestDispatcher::PARALLEL;
    innerSetting.dispatcher = TestDispatcher::SERIAL;
    innerSetting.delay = delayMs;
    outerSetting.create_group = true;
    outerSetting.op = TestOperation::ASYNC_GROUP;
    innerSetting.op = TestOperation::DELAY;
    int taskCount = Dispatch(outerSetting, innerSetting);
    bool result = Wait(taskCount);
    result = result && task_execution_sequence.size() > 1;
    if (result) {
        result = result && applyExecuted(outerSetting, innerSetting);
        result = result && OuterTaskExecuted(outerSetting);
        result = result && InnerTaskExecuted(innerSetting);
        std::vector<size_t> outerTaskIndex;
        std::vector<std::vector<size_t>> innerTaskIndex;
        GetTaskIndex(outerTaskIndex, innerTaskIndex);
        for (int i = 0; i < testTaskCount; i++) {
            result = result && IsAscend(innerTaskIndex[i]);
        }
    }
    TestUtils::PublishEvent(g_EVENT_RESP_FIRST, code, std::to_string(result));
}

// level1:parallel, group level2:serial, apply
void MainAbility::ParallelCase52(int code)
{
    APP_LOGI("-- -- -- -- -- --MainAbility::%{public}s", __FUNCTION__);
    Reset();
    TestSetting outerSetting;
    TestSetting innerSetting;
    outerSetting.dispatcher = TestDispatcher::PARALLEL;
    innerSetting.dispatcher = TestDispatcher::SERIAL;
    innerSetting.apply = applyNum;
    outerSetting.create_group = true;
    outerSetting.op = TestOperation::ASYNC_GROUP;
    innerSetting.op = TestOperation::APPLY;
    int taskCount = Dispatch(outerSetting, innerSetting);
    bool result = Wait(taskCount);
    result = result && task_execution_sequence.size() > 1;
    if (result) {
        result = result && applyExecuted(outerSetting, innerSetting);
        result = result && OuterTaskExecuted(outerSetting);
        result = result && InnerTaskExecuted(innerSetting);
    }
    TestUtils::PublishEvent(g_EVENT_RESP_FIRST, code, std::to_string(result));
}

// level1:parallel, group wait level2:parallel, sync
void MainAbility::ParallelCase53(int code)
{
    APP_LOGI("-- -- -- -- -- --MainAbility::%{public}s", __FUNCTION__);
    Reset();
    TestSetting outerSetting;
    TestSetting innerSetting;
    outerSetting.dispatcher = TestDispatcher::PARALLEL;
    innerSetting.dispatcher = TestDispatcher::PARALLEL;
    outerSetting.group_wait = true;
    outerSetting.group_timeout = groupWait;
    outerSetting.create_group = true;
    outerSetting.op = TestOperation::ASYNC_GROUP;
    innerSetting.op = TestOperation::SYNC;
    int taskCount = Dispatch(outerSetting, innerSetting);
    bool result = Wait(taskCount);
    result = result && task_execution_sequence.size() > 1;
    if (result) {
        result = result && applyExecuted(outerSetting, innerSetting);
        result = result && OuterTaskExecuted(outerSetting);
        result = result && InnerTaskExecuted(innerSetting);
        std::vector<size_t> outerTaskIndex;
        std::vector<std::vector<size_t>> innerTaskIndex;
        GetTaskIndex(outerTaskIndex, innerTaskIndex);
        for (int i = 0; i < testTaskCount; i++) {
            result = result && IsAscend(innerTaskIndex[i]);
        }
    }
    TestUtils::PublishEvent(g_EVENT_RESP_FIRST, code, std::to_string(result));
}

// level1:parallel, group wait level2:parallel, async
void MainAbility::ParallelCase54(int code)
{
    APP_LOGI("-- -- -- -- -- --MainAbility::%{public}s", __FUNCTION__);
    Reset();
    TestSetting outerSetting;
    TestSetting innerSetting;
    outerSetting.dispatcher = TestDispatcher::PARALLEL;
    innerSetting.dispatcher = TestDispatcher::PARALLEL;
    outerSetting.group_wait = true;
    outerSetting.group_timeout = groupWait;
    outerSetting.create_group = true;
    outerSetting.op = TestOperation::ASYNC_GROUP;
    innerSetting.op = TestOperation::ASYNC;
    int taskCount = Dispatch(outerSetting, innerSetting);
    bool result = Wait(taskCount);
    result = result && task_execution_sequence.size() > 1;
    if (result) {
        result = result && applyExecuted(outerSetting, innerSetting);
        result = result && OuterTaskExecuted(outerSetting);
        result = result && InnerTaskExecuted(innerSetting);
    }
    TestUtils::PublishEvent(g_EVENT_RESP_FIRST, code, std::to_string(result));
}

// level1:parallel, group wait level2:parallel, delay
void MainAbility::ParallelCase55(int code)
{
    APP_LOGI("-- -- -- -- -- --MainAbility::%{public}s", __FUNCTION__);
    Reset();
    TestSetting outerSetting;
    TestSetting innerSetting;
    outerSetting.dispatcher = TestDispatcher::PARALLEL;
    innerSetting.dispatcher = TestDispatcher::PARALLEL;
    outerSetting.group_wait = true;
    outerSetting.group_timeout = groupWait;
    innerSetting.delay = delayMs;
    outerSetting.create_group = true;
    outerSetting.op = TestOperation::ASYNC_GROUP;
    innerSetting.op = TestOperation::DELAY;
    int taskCount = Dispatch(outerSetting, innerSetting);
    bool result = Wait(taskCount);
    result = result && task_execution_sequence.size() > 1;
    if (result) {
        result = result && applyExecuted(outerSetting, innerSetting);
        result = result && OuterTaskExecuted(outerSetting);
        result = result && InnerTaskExecuted(innerSetting);
    }
    TestUtils::PublishEvent(g_EVENT_RESP_FIRST, code, std::to_string(result));
}

// level1:parallel, group wait level2:parallel, group
void MainAbility::ParallelCase56(int code)
{
    APP_LOGI("-- -- -- -- -- --MainAbility::%{public}s", __FUNCTION__);
    Reset();
    TestSetting outerSetting;
    TestSetting innerSetting;
    outerSetting.dispatcher = TestDispatcher::PARALLEL;
    innerSetting.dispatcher = TestDispatcher::PARALLEL;
    outerSetting.group_wait = true;
    outerSetting.group_timeout = groupWait;
    innerSetting.create_group = true;
    outerSetting.create_group = true;
    outerSetting.op = TestOperation::ASYNC_GROUP;
    innerSetting.op = TestOperation::ASYNC_GROUP;
    int taskCount = Dispatch(outerSetting, innerSetting);
    bool result = Wait(taskCount);
    result = result && task_execution_sequence.size() > 1;
    if (result) {
        result = result && applyExecuted(outerSetting, innerSetting);
        result = result && OuterTaskExecuted(outerSetting);
        result = result && InnerTaskExecuted(innerSetting);
    }
    TestUtils::PublishEvent(g_EVENT_RESP_FIRST, code, std::to_string(result));
}

// level1:parallel, group wait level2:parallel, group wait
void MainAbility::ParallelCase57(int code)
{
    APP_LOGI("-- -- -- -- -- --MainAbility::%{public}s", __FUNCTION__);
    Reset();
    TestSetting outerSetting;
    TestSetting innerSetting;
    outerSetting.dispatcher = TestDispatcher::PARALLEL;
    innerSetting.dispatcher = TestDispatcher::PARALLEL;
    innerSetting.group_wait = true;
    innerSetting.group_timeout = groupWait;
    outerSetting.group_wait = true;
    outerSetting.group_timeout = groupWait;
    innerSetting.create_group = true;
    outerSetting.create_group = true;
    outerSetting.op = TestOperation::ASYNC_GROUP;
    innerSetting.op = TestOperation::ASYNC_GROUP;
    int taskCount = Dispatch(outerSetting, innerSetting);
    bool result = Wait(taskCount);
    result = result && task_execution_sequence.size() > 1;
    if (result) {
        result = result && applyExecuted(outerSetting, innerSetting);
        result = result && OuterTaskExecuted(outerSetting);
        result = result && InnerTaskExecuted(innerSetting);
    }
    TestUtils::PublishEvent(g_EVENT_RESP_FIRST, code, std::to_string(result));
}

// level1:parallel, group wait level2:parallel, group notify
void MainAbility::ParallelCase58(int code)
{
    APP_LOGI("-- -- -- -- -- --MainAbility::%{public}s", __FUNCTION__);
    Reset();
    TestSetting outerSetting;
    TestSetting innerSetting;
    outerSetting.dispatcher = TestDispatcher::PARALLEL;
    innerSetting.dispatcher = TestDispatcher::PARALLEL;
    outerSetting.group_wait = true;
    outerSetting.group_timeout = groupWait;
    innerSetting.create_group = true;
    innerSetting.group_notify = true;
    outerSetting.create_group = true;
    outerSetting.op = TestOperation::ASYNC_GROUP;
    innerSetting.op = TestOperation::ASYNC_GROUP;
    int taskCount = Dispatch(outerSetting, innerSetting);
    bool result = Wait(taskCount);
    result = result && task_execution_sequence.size() > 1;
    if (result) {
        result = result && applyExecuted(outerSetting, innerSetting);
        result = result && OuterTaskExecuted(outerSetting);
        result = result && InnerTaskExecuted(innerSetting);
        result = result && InnerExecutedAfter(innerGroupNotifyId);
    }
    TestUtils::PublishEvent(g_EVENT_RESP_FIRST, code, std::to_string(result));
}

// level1:parallel, group wait level2:parallel, sync barrier
void MainAbility::ParallelCase59(int code)
{
    APP_LOGI("-- -- -- -- -- --MainAbility::%{public}s", __FUNCTION__);
    Reset();
    TestSetting outerSetting;
    TestSetting innerSetting;
    outerSetting.dispatcher = TestDispatcher::PARALLEL;
    innerSetting.dispatcher = TestDispatcher::PARALLEL;
    outerSetting.group_wait = true;
    outerSetting.group_timeout = groupWait;
    innerSetting.sync_barrier = true;
    innerSetting.create_group = true;
    outerSetting.create_group = true;
    outerSetting.op = TestOperation::ASYNC_GROUP;
    innerSetting.op = TestOperation::ASYNC_GROUP;
    int taskCount = Dispatch(outerSetting, innerSetting);
    bool result = Wait(taskCount);
    result = result && task_execution_sequence.size() > 1;
    if (result) {
        result = result && applyExecuted(outerSetting, innerSetting);
        result = result && OuterTaskExecuted(outerSetting);
        result = result && InnerTaskExecuted(innerSetting);
        result = result && InnerExecutedAfter(innerSyncBarrierId);
    }
    TestUtils::PublishEvent(g_EVENT_RESP_FIRST, code, std::to_string(result));
}

// level1:parallel, group wait level2:parallel, async barrier
void MainAbility::ParallelCase60(int code)
{
    APP_LOGI("-- -- -- -- -- --MainAbility::%{public}s", __FUNCTION__);
    Reset();
    TestSetting outerSetting;
    TestSetting innerSetting;
    outerSetting.dispatcher = TestDispatcher::PARALLEL;
    innerSetting.dispatcher = TestDispatcher::PARALLEL;
    outerSetting.group_wait = true;
    outerSetting.group_timeout = groupWait;
    innerSetting.async_barrier = true;
    innerSetting.create_group = true;
    outerSetting.create_group = true;
    outerSetting.op = TestOperation::ASYNC_GROUP;
    innerSetting.op = TestOperation::ASYNC_GROUP;
    int taskCount = Dispatch(outerSetting, innerSetting);
    bool result = Wait(taskCount);
    result = result && task_execution_sequence.size() > 1;
    if (result) {
        result = result && applyExecuted(outerSetting, innerSetting);
        result = result && OuterTaskExecuted(outerSetting);
        result = result && InnerTaskExecuted(innerSetting);
        result = result && InnerExecutedAfter(innerAsyncBarrierId);
    }
    TestUtils::PublishEvent(g_EVENT_RESP_FIRST, code, std::to_string(result));
}

// level1:parallel, group wait level2:parallel, apply
void MainAbility::ParallelCase61(int code)
{
    APP_LOGI("-- -- -- -- -- --MainAbility::%{public}s", __FUNCTION__);
    Reset();
    TestSetting outerSetting;
    TestSetting innerSetting;
    outerSetting.dispatcher = TestDispatcher::PARALLEL;
    innerSetting.dispatcher = TestDispatcher::PARALLEL;
    outerSetting.group_wait = true;
    outerSetting.group_timeout = groupWait;
    innerSetting.apply = applyNum;
    outerSetting.create_group = true;
    outerSetting.op = TestOperation::ASYNC_GROUP;
    innerSetting.op = TestOperation::APPLY;
    int taskCount = Dispatch(outerSetting, innerSetting);
    bool result = Wait(taskCount);
    result = result && task_execution_sequence.size() > 1;
    if (result) {
        result = result && applyExecuted(outerSetting, innerSetting);
        result = result && OuterTaskExecuted(outerSetting);
        result = result && InnerTaskExecuted(innerSetting);
    }
    TestUtils::PublishEvent(g_EVENT_RESP_FIRST, code, std::to_string(result));
}

// level1:parallel, group wait level2:serial, sync
void MainAbility::ParallelCase62(int code)
{
    APP_LOGI("-- -- -- -- -- --MainAbility::%{public}s", __FUNCTION__);
    Reset();
    TestSetting outerSetting;
    TestSetting innerSetting;
    outerSetting.dispatcher = TestDispatcher::PARALLEL;
    innerSetting.dispatcher = TestDispatcher::SERIAL;
    outerSetting.group_wait = true;
    outerSetting.group_timeout = groupWait;
    outerSetting.create_group = true;
    outerSetting.op = TestOperation::ASYNC_GROUP;
    innerSetting.op = TestOperation::SYNC;
    int taskCount = Dispatch(outerSetting, innerSetting);
    bool result = Wait(taskCount);
    result = result && task_execution_sequence.size() > 1;
    if (result) {
        result = result && applyExecuted(outerSetting, innerSetting);
        result = result && OuterTaskExecuted(outerSetting);
        result = result && InnerTaskExecuted(innerSetting);
        std::vector<size_t> outerTaskIndex;
        std::vector<std::vector<size_t>> innerTaskIndex;
        GetTaskIndex(outerTaskIndex, innerTaskIndex);
        for (int i = 0; i < testTaskCount; i++) {
            result = result && IsAscend(innerTaskIndex[i]);
        }
    }
    TestUtils::PublishEvent(g_EVENT_RESP_FIRST, code, std::to_string(result));
}

// level1:parallel, group wait level2:serial, async
void MainAbility::ParallelCase63(int code)
{
    APP_LOGI("-- -- -- -- -- --MainAbility::%{public}s", __FUNCTION__);
    Reset();
    TestSetting outerSetting;
    TestSetting innerSetting;
    outerSetting.dispatcher = TestDispatcher::PARALLEL;
    innerSetting.dispatcher = TestDispatcher::SERIAL;
    outerSetting.group_wait = true;
    outerSetting.group_timeout = groupWait;
    outerSetting.create_group = true;
    outerSetting.op = TestOperation::ASYNC_GROUP;
    innerSetting.op = TestOperation::ASYNC;
    int taskCount = Dispatch(outerSetting, innerSetting);
    bool result = Wait(taskCount);
    result = result && task_execution_sequence.size() > 1;
    if (result) {
        result = result && applyExecuted(outerSetting, innerSetting);
        result = result && OuterTaskExecuted(outerSetting);
        result = result && InnerTaskExecuted(innerSetting);
        std::vector<size_t> outerTaskIndex;
        std::vector<std::vector<size_t>> innerTaskIndex;
        GetTaskIndex(outerTaskIndex, innerTaskIndex);
        for (int i = 0; i < testTaskCount; i++) {
            result = result && IsAscend(innerTaskIndex[i]);
        }
    }
    TestUtils::PublishEvent(g_EVENT_RESP_FIRST, code, std::to_string(result));
}

// level1:parallel, group wait level2:serial, delay
void MainAbility::ParallelCase64(int code)
{
    APP_LOGI("-- -- -- -- -- --MainAbility::%{public}s", __FUNCTION__);
    Reset();
    TestSetting outerSetting;
    TestSetting innerSetting;
    outerSetting.dispatcher = TestDispatcher::PARALLEL;
    innerSetting.dispatcher = TestDispatcher::SERIAL;
    outerSetting.group_wait = true;
    outerSetting.group_timeout = groupWait;
    innerSetting.delay = delayMs;
    outerSetting.create_group = true;
    outerSetting.op = TestOperation::ASYNC_GROUP;
    innerSetting.op = TestOperation::DELAY;
    int taskCount = Dispatch(outerSetting, innerSetting);
    bool result = Wait(taskCount);
    result = result && task_execution_sequence.size() > 1;
    if (result) {
        result = result && applyExecuted(outerSetting, innerSetting);
        result = result && OuterTaskExecuted(outerSetting);
        result = result && InnerTaskExecuted(innerSetting);
        std::vector<size_t> outerTaskIndex;
        std::vector<std::vector<size_t>> innerTaskIndex;
        GetTaskIndex(outerTaskIndex, innerTaskIndex);
        for (int i = 0; i < testTaskCount; i++) {
            result = result && IsAscend(innerTaskIndex[i]);
        }
    }
    TestUtils::PublishEvent(g_EVENT_RESP_FIRST, code, std::to_string(result));
}

// level1:parallel, group wait level2:serial, apply
void MainAbility::ParallelCase65(int code)
{
    APP_LOGI("-- -- -- -- -- --MainAbility::%{public}s", __FUNCTION__);
    Reset();
    TestSetting outerSetting;
    TestSetting innerSetting;
    outerSetting.dispatcher = TestDispatcher::PARALLEL;
    innerSetting.dispatcher = TestDispatcher::SERIAL;
    outerSetting.group_wait = true;
    outerSetting.group_timeout = groupWait;
    innerSetting.apply = applyNum;
    outerSetting.create_group = true;
    outerSetting.op = TestOperation::ASYNC_GROUP;
    innerSetting.op = TestOperation::APPLY;
    int taskCount = Dispatch(outerSetting, innerSetting);
    bool result = Wait(taskCount);
    result = result && task_execution_sequence.size() > 1;
    if (result) {
        result = result && applyExecuted(outerSetting, innerSetting);
        result = result && OuterTaskExecuted(outerSetting);
        result = result && InnerTaskExecuted(innerSetting);
    }
    TestUtils::PublishEvent(g_EVENT_RESP_FIRST, code, std::to_string(result));
}

// level1:parallel, group notify level2:parallel, sync
void MainAbility::ParallelCase66(int code)
{
    APP_LOGI("-- -- -- -- -- --MainAbility::%{public}s", __FUNCTION__);
    Reset();
    TestSetting outerSetting;
    TestSetting innerSetting;
    outerSetting.dispatcher = TestDispatcher::PARALLEL;
    innerSetting.dispatcher = TestDispatcher::PARALLEL;
    outerSetting.create_group = true;
    outerSetting.group_notify = true;
    outerSetting.op = TestOperation::ASYNC_GROUP;
    innerSetting.op = TestOperation::SYNC;
    int taskCount = Dispatch(outerSetting, innerSetting);
    bool result = Wait(taskCount);
    result = result && task_execution_sequence.size() > 1;
    if (result) {
        result = result && applyExecuted(outerSetting, innerSetting);
        result = result && OuterTaskExecuted(outerSetting);
        result = result && InnerTaskExecuted(innerSetting);
        std::vector<size_t> outerTaskIndex;
        std::vector<std::vector<size_t>> innerTaskIndex;
        GetTaskIndex(outerTaskIndex, innerTaskIndex);
        for (int i = 0; i < testTaskCount; i++) {
            result = result && IsAscend(innerTaskIndex[i]);
        }
        result = result && OuterExecutedAfter(outerGroupNotifyId);
    }
    TestUtils::PublishEvent(g_EVENT_RESP_FIRST, code, std::to_string(result));
}

// level1:parallel, group notify level2:parallel, async
void MainAbility::ParallelCase67(int code)
{
    APP_LOGI("-- -- -- -- -- --MainAbility::%{public}s", __FUNCTION__);
    Reset();
    TestSetting outerSetting;
    TestSetting innerSetting;
    outerSetting.dispatcher = TestDispatcher::PARALLEL;
    innerSetting.dispatcher = TestDispatcher::PARALLEL;
    outerSetting.create_group = true;
    outerSetting.group_notify = true;
    outerSetting.op = TestOperation::ASYNC_GROUP;
    innerSetting.op = TestOperation::ASYNC;
    int taskCount = Dispatch(outerSetting, innerSetting);
    bool result = Wait(taskCount);
    result = result && task_execution_sequence.size() > 1;
    if (result) {
        result = result && applyExecuted(outerSetting, innerSetting);
        result = result && OuterTaskExecuted(outerSetting);
        result = result && InnerTaskExecuted(innerSetting);
        result = result && OuterExecutedAfter(outerGroupNotifyId);
    }
    TestUtils::PublishEvent(g_EVENT_RESP_FIRST, code, std::to_string(result));
}

// level1:parallel, group notify level2:parallel, delay
void MainAbility::ParallelCase68(int code)
{
    APP_LOGI("-- -- -- -- -- --MainAbility::%{public}s", __FUNCTION__);
    Reset();
    TestSetting outerSetting;
    TestSetting innerSetting;
    outerSetting.dispatcher = TestDispatcher::PARALLEL;
    innerSetting.dispatcher = TestDispatcher::PARALLEL;
    innerSetting.delay = delayMs;
    outerSetting.create_group = true;
    outerSetting.group_notify = true;
    outerSetting.op = TestOperation::ASYNC_GROUP;
    innerSetting.op = TestOperation::DELAY;
    int taskCount = Dispatch(outerSetting, innerSetting);
    bool result = Wait(taskCount);
    result = result && task_execution_sequence.size() > 1;
    if (result) {
        result = result && applyExecuted(outerSetting, innerSetting);
        result = result && OuterTaskExecuted(outerSetting);
        result = result && InnerTaskExecuted(innerSetting);
        result = result && OuterExecutedAfter(outerGroupNotifyId);
    }
    TestUtils::PublishEvent(g_EVENT_RESP_FIRST, code, std::to_string(result));
}

// level1:parallel, group notify level2:parallel, group
void MainAbility::ParallelCase69(int code)
{
    APP_LOGI("-- -- -- -- -- --MainAbility::%{public}s", __FUNCTION__);
    Reset();
    TestSetting outerSetting;
    TestSetting innerSetting;
    outerSetting.dispatcher = TestDispatcher::PARALLEL;
    innerSetting.dispatcher = TestDispatcher::PARALLEL;
    innerSetting.create_group = true;
    outerSetting.create_group = true;
    outerSetting.group_notify = true;
    outerSetting.op = TestOperation::ASYNC_GROUP;
    innerSetting.op = TestOperation::ASYNC_GROUP;
    int taskCount = Dispatch(outerSetting, innerSetting);
    bool result = Wait(taskCount);
    result = result && task_execution_sequence.size() > 1;
    if (result) {
        result = result && applyExecuted(outerSetting, innerSetting);
        result = result && OuterTaskExecuted(outerSetting);
        result = result && InnerTaskExecuted(innerSetting);
        result = result && OuterExecutedAfter(outerGroupNotifyId);
    }
    TestUtils::PublishEvent(g_EVENT_RESP_FIRST, code, std::to_string(result));
}

// level1:parallel, group notify level2:parallel, group wait
void MainAbility::ParallelCase70(int code)
{
    APP_LOGI("-- -- -- -- -- --MainAbility::%{public}s", __FUNCTION__);
    Reset();
    TestSetting outerSetting;
    TestSetting innerSetting;
    outerSetting.dispatcher = TestDispatcher::PARALLEL;
    innerSetting.dispatcher = TestDispatcher::PARALLEL;
    innerSetting.group_wait = true;
    innerSetting.group_timeout = groupWait;
    innerSetting.create_group = true;
    outerSetting.create_group = true;
    outerSetting.group_notify = true;
    outerSetting.op = TestOperation::ASYNC_GROUP;
    innerSetting.op = TestOperation::ASYNC_GROUP;
    int taskCount = Dispatch(outerSetting, innerSetting);
    bool result = Wait(taskCount);
    result = result && task_execution_sequence.size() > 1;
    if (result) {
        result = result && applyExecuted(outerSetting, innerSetting);
        result = result && OuterTaskExecuted(outerSetting);
        result = result && InnerTaskExecuted(innerSetting);
        result = result && OuterExecutedAfter(outerGroupNotifyId);
    }
    TestUtils::PublishEvent(g_EVENT_RESP_FIRST, code, std::to_string(result));
}

// level1:parallel, group notify level2:parallel, group notify
void MainAbility::ParallelCase71(int code)
{
    APP_LOGI("-- -- -- -- -- --MainAbility::%{public}s", __FUNCTION__);
    Reset();
    TestSetting outerSetting;
    TestSetting innerSetting;
    outerSetting.dispatcher = TestDispatcher::PARALLEL;
    innerSetting.dispatcher = TestDispatcher::PARALLEL;
    innerSetting.create_group = true;
    innerSetting.group_notify = true;
    outerSetting.create_group = true;
    outerSetting.group_notify = true;
    outerSetting.op = TestOperation::ASYNC_GROUP;
    innerSetting.op = TestOperation::ASYNC_GROUP;
    int taskCount = Dispatch(outerSetting, innerSetting);
    bool result = Wait(taskCount);
    result = result && task_execution_sequence.size() > 1;
    if (result) {
        result = result && applyExecuted(outerSetting, innerSetting);
        result = result && OuterTaskExecuted(outerSetting);
        result = result && InnerTaskExecuted(innerSetting);
        result = result && OuterExecutedAfter(outerGroupNotifyId);
        result = result && InnerExecutedAfter(innerGroupNotifyId);
    }
    TestUtils::PublishEvent(g_EVENT_RESP_FIRST, code, std::to_string(result));
}

// level1:parallel, group notify level2:parallel, sync barrier
void MainAbility::ParallelCase72(int code)
{
    APP_LOGI("-- -- -- -- -- --MainAbility::%{public}s", __FUNCTION__);
    Reset();
    TestSetting outerSetting;
    TestSetting innerSetting;
    outerSetting.dispatcher = TestDispatcher::PARALLEL;
    innerSetting.dispatcher = TestDispatcher::PARALLEL;
    innerSetting.sync_barrier = true;
    innerSetting.create_group = true;
    outerSetting.create_group = true;
    outerSetting.group_notify = true;
    outerSetting.op = TestOperation::ASYNC_GROUP;
    innerSetting.op = TestOperation::ASYNC_GROUP;
    int taskCount = Dispatch(outerSetting, innerSetting);
    bool result = Wait(taskCount);
    result = result && task_execution_sequence.size() > 1;
    if (result) {
        result = result && applyExecuted(outerSetting, innerSetting);
        result = result && OuterTaskExecuted(outerSetting);
        result = result && InnerTaskExecuted(innerSetting);
        result = result && InnerExecutedAfter(innerSyncBarrierId);
        result = result && OuterExecutedAfter(outerGroupNotifyId);
    }
    TestUtils::PublishEvent(g_EVENT_RESP_FIRST, code, std::to_string(result));
}

// level1:parallel, group notify level2:parallel, async barrier
void MainAbility::ParallelCase73(int code)
{
    APP_LOGI("-- -- -- -- -- --MainAbility::%{public}s", __FUNCTION__);
    Reset();
    TestSetting outerSetting;
    TestSetting innerSetting;
    outerSetting.dispatcher = TestDispatcher::PARALLEL;
    innerSetting.dispatcher = TestDispatcher::PARALLEL;
    innerSetting.async_barrier = true;
    innerSetting.create_group = true;
    outerSetting.create_group = true;
    outerSetting.group_notify = true;
    outerSetting.op = TestOperation::ASYNC_GROUP;
    innerSetting.op = TestOperation::ASYNC_GROUP;
    int taskCount = Dispatch(outerSetting, innerSetting);
    bool result = Wait(taskCount);
    result = result && task_execution_sequence.size() > 1;
    if (result) {
        result = result && applyExecuted(outerSetting, innerSetting);
        result = result && OuterTaskExecuted(outerSetting);
        result = result && InnerTaskExecuted(innerSetting);
        result = result && InnerExecutedAfter(innerAsyncBarrierId);
        result = result && OuterExecutedAfter(outerGroupNotifyId);
    }
    TestUtils::PublishEvent(g_EVENT_RESP_FIRST, code, std::to_string(result));
}

// level1:parallel, group notify level2:parallel, apply
void MainAbility::ParallelCase74(int code)
{
    APP_LOGI("-- -- -- -- -- --MainAbility::%{public}s", __FUNCTION__);
    Reset();
    TestSetting outerSetting;
    TestSetting innerSetting;
    outerSetting.dispatcher = TestDispatcher::PARALLEL;
    innerSetting.dispatcher = TestDispatcher::PARALLEL;
    innerSetting.apply = applyNum;
    outerSetting.create_group = true;
    outerSetting.group_notify = true;
    outerSetting.op = TestOperation::ASYNC_GROUP;
    innerSetting.op = TestOperation::APPLY;
    int taskCount = Dispatch(outerSetting, innerSetting);
    bool result = Wait(taskCount);
    result = result && task_execution_sequence.size() > 1;
    if (result) {
        result = result && applyExecuted(outerSetting, innerSetting);
        result = result && OuterTaskExecuted(outerSetting);
        result = result && InnerTaskExecuted(innerSetting);
        result = result && OuterExecutedAfter(outerGroupNotifyId);
    }
    TestUtils::PublishEvent(g_EVENT_RESP_FIRST, code, std::to_string(result));
}

// level1:parallel, group notify level2:serial, sync
void MainAbility::ParallelCase75(int code)
{
    APP_LOGI("-- -- -- -- -- --MainAbility::%{public}s", __FUNCTION__);
    Reset();
    TestSetting outerSetting;
    TestSetting innerSetting;
    outerSetting.dispatcher = TestDispatcher::PARALLEL;
    innerSetting.dispatcher = TestDispatcher::SERIAL;
    outerSetting.create_group = true;
    outerSetting.group_notify = true;
    outerSetting.op = TestOperation::ASYNC_GROUP;
    innerSetting.op = TestOperation::SYNC;
    int taskCount = Dispatch(outerSetting, innerSetting);
    bool result = Wait(taskCount);
    result = result && task_execution_sequence.size() > 1;
    if (result) {
        result = result && applyExecuted(outerSetting, innerSetting);
        result = result && OuterTaskExecuted(outerSetting);
        result = result && InnerTaskExecuted(innerSetting);
        std::vector<size_t> outerTaskIndex;
        std::vector<std::vector<size_t>> innerTaskIndex;
        GetTaskIndex(outerTaskIndex, innerTaskIndex);
        for (int i = 0; i < testTaskCount; i++) {
            result = result && IsAscend(innerTaskIndex[i]);
        }
        result = result && OuterExecutedAfter(outerGroupNotifyId);
    }
    TestUtils::PublishEvent(g_EVENT_RESP_FIRST, code, std::to_string(result));
}

// level1:parallel, group notify level2:serial, async
void MainAbility::ParallelCase76(int code)
{
    APP_LOGI("-- -- -- -- -- --MainAbility::%{public}s", __FUNCTION__);
    Reset();
    TestSetting outerSetting;
    TestSetting innerSetting;
    outerSetting.dispatcher = TestDispatcher::PARALLEL;
    innerSetting.dispatcher = TestDispatcher::SERIAL;
    outerSetting.create_group = true;
    outerSetting.group_notify = true;
    outerSetting.op = TestOperation::ASYNC_GROUP;
    innerSetting.op = TestOperation::ASYNC;
    int taskCount = Dispatch(outerSetting, innerSetting);
    bool result = Wait(taskCount);
    result = result && task_execution_sequence.size() > 1;
    if (result) {
        result = result && applyExecuted(outerSetting, innerSetting);
        result = result && OuterTaskExecuted(outerSetting);
        result = result && InnerTaskExecuted(innerSetting);
        std::vector<size_t> outerTaskIndex;
        std::vector<std::vector<size_t>> innerTaskIndex;
        GetTaskIndex(outerTaskIndex, innerTaskIndex);
        for (int i = 0; i < testTaskCount; i++) {
            result = result && IsAscend(innerTaskIndex[i]);
        }
        result = result && OuterExecutedAfter(outerGroupNotifyId);
    }
    TestUtils::PublishEvent(g_EVENT_RESP_FIRST, code, std::to_string(result));
}

// level1:parallel, group notify level2:serial, delay
void MainAbility::ParallelCase77(int code)
{
    APP_LOGI("-- -- -- -- -- --MainAbility::%{public}s", __FUNCTION__);
    Reset();
    TestSetting outerSetting;
    TestSetting innerSetting;
    outerSetting.dispatcher = TestDispatcher::PARALLEL;
    innerSetting.dispatcher = TestDispatcher::SERIAL;
    innerSetting.delay = delayMs;
    outerSetting.create_group = true;
    outerSetting.group_notify = true;
    outerSetting.op = TestOperation::ASYNC_GROUP;
    innerSetting.op = TestOperation::DELAY;
    int taskCount = Dispatch(outerSetting, innerSetting);
    bool result = Wait(taskCount);
    result = result && task_execution_sequence.size() > 1;
    if (result) {
        result = result && applyExecuted(outerSetting, innerSetting);
        result = result && OuterTaskExecuted(outerSetting);
        result = result && InnerTaskExecuted(innerSetting);
        std::vector<size_t> outerTaskIndex;
        std::vector<std::vector<size_t>> innerTaskIndex;
        GetTaskIndex(outerTaskIndex, innerTaskIndex);
        for (int i = 0; i < testTaskCount; i++) {
            result = result && IsAscend(innerTaskIndex[i]);
        }
        result = result && OuterExecutedAfter(outerGroupNotifyId);
    }
    TestUtils::PublishEvent(g_EVENT_RESP_FIRST, code, std::to_string(result));
}

// level1:parallel, group notify level2:serial, apply
void MainAbility::ParallelCase78(int code)
{
    APP_LOGI("-- -- -- -- -- --MainAbility::%{public}s", __FUNCTION__);
    Reset();
    TestSetting outerSetting;
    TestSetting innerSetting;
    outerSetting.dispatcher = TestDispatcher::PARALLEL;
    innerSetting.dispatcher = TestDispatcher::SERIAL;
    innerSetting.apply = applyNum;
    outerSetting.create_group = true;
    outerSetting.group_notify = true;
    outerSetting.op = TestOperation::ASYNC_GROUP;
    innerSetting.op = TestOperation::APPLY;
    int taskCount = Dispatch(outerSetting, innerSetting);
    bool result = Wait(taskCount);
    result = result && task_execution_sequence.size() > 1;
    if (result) {
        result = result && applyExecuted(outerSetting, innerSetting);
        result = result && OuterTaskExecuted(outerSetting);
        result = result && InnerTaskExecuted(innerSetting);
        result = result && OuterExecutedAfter(outerGroupNotifyId);
    }
    TestUtils::PublishEvent(g_EVENT_RESP_FIRST, code, std::to_string(result));
}

// level1:parallel, sync barrier level2:parallel, sync
void MainAbility::ParallelCase79(int code)
{
    APP_LOGI("-- -- -- -- -- --MainAbility::%{public}s", __FUNCTION__);
    Reset();
    TestSetting outerSetting;
    TestSetting innerSetting;
    outerSetting.dispatcher = TestDispatcher::PARALLEL;
    innerSetting.dispatcher = TestDispatcher::PARALLEL;
    outerSetting.sync_barrier = true;
    outerSetting.create_group = true;
    outerSetting.op = TestOperation::ASYNC_GROUP;
    innerSetting.op = TestOperation::SYNC;
    int taskCount = Dispatch(outerSetting, innerSetting);
    bool result = Wait(taskCount);
    result = result && task_execution_sequence.size() > 1;
    if (result) {
        result = result && applyExecuted(outerSetting, innerSetting);
        result = result && OuterTaskExecuted(outerSetting);
        result = result && InnerTaskExecuted(innerSetting);
        std::vector<size_t> outerTaskIndex;
        std::vector<std::vector<size_t>> innerTaskIndex;
        GetTaskIndex(outerTaskIndex, innerTaskIndex);
        for (int i = 0; i < testTaskCount; i++) {
            result = result && IsAscend(innerTaskIndex[i]);
        }
        result = result && OuterExecutedAfter(outerSyncBarrierId);
    }
    TestUtils::PublishEvent(g_EVENT_RESP_FIRST, code, std::to_string(result));
}

// level1:parallel, sync barrier level2:parallel, async
void MainAbility::ParallelCase80(int code)
{
    APP_LOGI("-- -- -- -- -- --MainAbility::%{public}s", __FUNCTION__);
    Reset();
    TestSetting outerSetting;
    TestSetting innerSetting;
    outerSetting.dispatcher = TestDispatcher::PARALLEL;
    innerSetting.dispatcher = TestDispatcher::PARALLEL;
    outerSetting.sync_barrier = true;
    outerSetting.create_group = true;
    outerSetting.op = TestOperation::ASYNC_GROUP;
    innerSetting.op = TestOperation::ASYNC;
    int taskCount = Dispatch(outerSetting, innerSetting);
    bool result = Wait(taskCount);
    result = result && task_execution_sequence.size() > 1;
    if (result) {
        result = result && applyExecuted(outerSetting, innerSetting);
        result = result && OuterTaskExecuted(outerSetting);
        result = result && InnerTaskExecuted(innerSetting);
        result = result && OuterExecutedAfter(outerSyncBarrierId);
    }
    TestUtils::PublishEvent(g_EVENT_RESP_FIRST, code, std::to_string(result));
}

// level1:parallel, sync barrier level2:parallel, delay
void MainAbility::ParallelCase81(int code)
{
    APP_LOGI("-- -- -- -- -- --MainAbility::%{public}s", __FUNCTION__);
    Reset();
    TestSetting outerSetting;
    TestSetting innerSetting;
    outerSetting.dispatcher = TestDispatcher::PARALLEL;
    innerSetting.dispatcher = TestDispatcher::PARALLEL;
    innerSetting.delay = delayMs;
    outerSetting.sync_barrier = true;
    outerSetting.create_group = true;
    outerSetting.op = TestOperation::ASYNC_GROUP;
    innerSetting.op = TestOperation::DELAY;
    int taskCount = Dispatch(outerSetting, innerSetting);
    bool result = Wait(taskCount);
    result = result && task_execution_sequence.size() > 1;
    if (result) {
        result = result && applyExecuted(outerSetting, innerSetting);
        result = result && OuterTaskExecuted(outerSetting);
        result = result && InnerTaskExecuted(innerSetting);
        result = result && OuterExecutedAfter(outerSyncBarrierId);
    }
    TestUtils::PublishEvent(g_EVENT_RESP_FIRST, code, std::to_string(result));
}

// level1:parallel, sync barrier level2:parallel, group
void MainAbility::ParallelCase82(int code)
{
    APP_LOGI("-- -- -- -- -- --MainAbility::%{public}s", __FUNCTION__);
    Reset();
    TestSetting outerSetting;
    TestSetting innerSetting;
    outerSetting.dispatcher = TestDispatcher::PARALLEL;
    innerSetting.dispatcher = TestDispatcher::PARALLEL;
    innerSetting.create_group = true;
    outerSetting.sync_barrier = true;
    outerSetting.create_group = true;
    outerSetting.op = TestOperation::ASYNC_GROUP;
    innerSetting.op = TestOperation::ASYNC_GROUP;
    int taskCount = Dispatch(outerSetting, innerSetting);
    bool result = Wait(taskCount);
    result = result && task_execution_sequence.size() > 1;
    if (result) {
        result = result && applyExecuted(outerSetting, innerSetting);
        result = result && OuterTaskExecuted(outerSetting);
        result = result && InnerTaskExecuted(innerSetting);
        result = result && OuterExecutedAfter(outerSyncBarrierId);
    }
    TestUtils::PublishEvent(g_EVENT_RESP_FIRST, code, std::to_string(result));
}

// level1:parallel, sync barrier level2:parallel, group wait
void MainAbility::ParallelCase83(int code)
{
    APP_LOGI("-- -- -- -- -- --MainAbility::%{public}s", __FUNCTION__);
    Reset();
    TestSetting outerSetting;
    TestSetting innerSetting;
    outerSetting.dispatcher = TestDispatcher::PARALLEL;
    innerSetting.dispatcher = TestDispatcher::PARALLEL;
    innerSetting.group_wait = true;
    innerSetting.group_timeout = groupWait;
    innerSetting.create_group = true;
    outerSetting.sync_barrier = true;
    outerSetting.create_group = true;
    outerSetting.op = TestOperation::ASYNC_GROUP;
    innerSetting.op = TestOperation::ASYNC_GROUP;
    int taskCount = Dispatch(outerSetting, innerSetting);
    bool result = Wait(taskCount);
    result = result && task_execution_sequence.size() > 1;
    if (result) {
        result = result && applyExecuted(outerSetting, innerSetting);
        result = result && OuterTaskExecuted(outerSetting);
        result = result && InnerTaskExecuted(innerSetting);
        result = result && OuterExecutedAfter(outerSyncBarrierId);
    }
    TestUtils::PublishEvent(g_EVENT_RESP_FIRST, code, std::to_string(result));
}

// level1:parallel, sync barrier level2:parallel, group notify
void MainAbility::ParallelCase84(int code)
{
    APP_LOGI("-- -- -- -- -- --MainAbility::%{public}s", __FUNCTION__);
    Reset();
    TestSetting outerSetting;
    TestSetting innerSetting;
    outerSetting.dispatcher = TestDispatcher::PARALLEL;
    innerSetting.dispatcher = TestDispatcher::PARALLEL;
    innerSetting.create_group = true;
    innerSetting.group_notify = true;
    outerSetting.sync_barrier = true;
    outerSetting.create_group = true;
    outerSetting.op = TestOperation::ASYNC_GROUP;
    innerSetting.op = TestOperation::ASYNC_GROUP;
    int taskCount = Dispatch(outerSetting, innerSetting);
    bool result = Wait(taskCount);
    result = result && task_execution_sequence.size() > 1;
    if (result) {
        result = result && applyExecuted(outerSetting, innerSetting);
        result = result && OuterTaskExecuted(outerSetting);
        result = result && InnerTaskExecuted(innerSetting);
        result = result && OuterExecutedAfter(outerSyncBarrierId);
        result = result && InnerExecutedAfter(innerGroupNotifyId);
    }
    TestUtils::PublishEvent(g_EVENT_RESP_FIRST, code, std::to_string(result));
}

// level1:parallel, sync barrier level2:parallel, sync barrier
void MainAbility::ParallelCase85(int code)
{
    APP_LOGI("-- -- -- -- -- --MainAbility::%{public}s", __FUNCTION__);
    Reset();
    TestSetting outerSetting;
    TestSetting innerSetting;
    outerSetting.dispatcher = TestDispatcher::PARALLEL;
    innerSetting.dispatcher = TestDispatcher::PARALLEL;
    innerSetting.sync_barrier = true;
    innerSetting.create_group = true;
    outerSetting.sync_barrier = true;
    outerSetting.create_group = true;
    outerSetting.op = TestOperation::ASYNC_GROUP;
    innerSetting.op = TestOperation::ASYNC_GROUP;
    int taskCount = Dispatch(outerSetting, innerSetting);
    bool result = Wait(taskCount);
    result = result && task_execution_sequence.size() > 1;
    if (result) {
        result = result && applyExecuted(outerSetting, innerSetting);
        result = result && OuterTaskExecuted(outerSetting);
        result = result && InnerTaskExecuted(innerSetting);
        result = result && OuterExecutedAfter(outerSyncBarrierId);
        result = result && InnerExecutedAfter(innerSyncBarrierId);
    }
    TestUtils::PublishEvent(g_EVENT_RESP_FIRST, code, std::to_string(result));
}

// level1:parallel, sync barrier level2:parallel, async barrier
void MainAbility::ParallelCase86(int code)
{
    APP_LOGI("-- -- -- -- -- --MainAbility::%{public}s", __FUNCTION__);
    Reset();
    TestSetting outerSetting;
    TestSetting innerSetting;
    outerSetting.dispatcher = TestDispatcher::PARALLEL;
    innerSetting.dispatcher = TestDispatcher::PARALLEL;
    innerSetting.async_barrier = true;
    innerSetting.create_group = true;
    outerSetting.sync_barrier = true;
    outerSetting.create_group = true;
    outerSetting.op = TestOperation::ASYNC_GROUP;
    innerSetting.op = TestOperation::ASYNC_GROUP;
    int taskCount = Dispatch(outerSetting, innerSetting);
    bool result = Wait(taskCount);
    result = result && task_execution_sequence.size() > 1;
    if (result) {
        result = result && applyExecuted(outerSetting, innerSetting);
        result = result && OuterTaskExecuted(outerSetting);
        result = result && InnerTaskExecuted(innerSetting);
        result = result && OuterExecutedAfter(outerSyncBarrierId);
        result = result && InnerExecutedAfter(innerAsyncBarrierId);
    }
    TestUtils::PublishEvent(g_EVENT_RESP_FIRST, code, std::to_string(result));
}

// level1:parallel, sync barrier level2:parallel, apply
void MainAbility::ParallelCase87(int code)
{
    APP_LOGI("-- -- -- -- -- --MainAbility::%{public}s", __FUNCTION__);
    Reset();
    TestSetting outerSetting;
    TestSetting innerSetting;
    outerSetting.dispatcher = TestDispatcher::PARALLEL;
    innerSetting.dispatcher = TestDispatcher::PARALLEL;
    innerSetting.apply = applyNum;
    outerSetting.sync_barrier = true;
    outerSetting.create_group = true;
    outerSetting.op = TestOperation::ASYNC_GROUP;
    innerSetting.op = TestOperation::APPLY;
    int taskCount = Dispatch(outerSetting, innerSetting);
    bool result = Wait(taskCount);
    result = result && task_execution_sequence.size() > 1;
    if (result) {
        result = result && applyExecuted(outerSetting, innerSetting);
        result = result && OuterTaskExecuted(outerSetting);
        result = result && InnerTaskExecuted(innerSetting);
        result = result && OuterExecutedAfter(outerSyncBarrierId);
    }
    TestUtils::PublishEvent(g_EVENT_RESP_FIRST, code, std::to_string(result));
}

// level1:parallel, sync barrier level2:serial, sync
void MainAbility::ParallelCase88(int code)
{
    APP_LOGI("-- -- -- -- -- --MainAbility::%{public}s", __FUNCTION__);
    Reset();
    TestSetting outerSetting;
    TestSetting innerSetting;
    outerSetting.dispatcher = TestDispatcher::PARALLEL;
    innerSetting.dispatcher = TestDispatcher::SERIAL;
    outerSetting.sync_barrier = true;
    outerSetting.create_group = true;
    outerSetting.op = TestOperation::ASYNC_GROUP;
    innerSetting.op = TestOperation::SYNC;
    int taskCount = Dispatch(outerSetting, innerSetting);
    bool result = Wait(taskCount);
    result = result && task_execution_sequence.size() > 1;
    if (result) {
        result = result && applyExecuted(outerSetting, innerSetting);
        result = result && OuterTaskExecuted(outerSetting);
        result = result && InnerTaskExecuted(innerSetting);
        std::vector<size_t> outerTaskIndex;
        std::vector<std::vector<size_t>> innerTaskIndex;
        GetTaskIndex(outerTaskIndex, innerTaskIndex);
        for (int i = 0; i < testTaskCount; i++) {
            result = result && IsAscend(innerTaskIndex[i]);
        }
        result = result && OuterExecutedAfter(outerSyncBarrierId);
    }
    TestUtils::PublishEvent(g_EVENT_RESP_FIRST, code, std::to_string(result));
}

// level1:parallel, sync barrier level2:serial, async
void MainAbility::ParallelCase89(int code)
{
    APP_LOGI("-- -- -- -- -- --MainAbility::%{public}s", __FUNCTION__);
    Reset();
    TestSetting outerSetting;
    TestSetting innerSetting;
    outerSetting.dispatcher = TestDispatcher::PARALLEL;
    innerSetting.dispatcher = TestDispatcher::SERIAL;
    outerSetting.sync_barrier = true;
    outerSetting.create_group = true;
    outerSetting.op = TestOperation::ASYNC_GROUP;
    innerSetting.op = TestOperation::ASYNC;
    int taskCount = Dispatch(outerSetting, innerSetting);
    bool result = Wait(taskCount);
    result = result && task_execution_sequence.size() > 1;
    if (result) {
        result = result && applyExecuted(outerSetting, innerSetting);
        result = result && OuterTaskExecuted(outerSetting);
        result = result && InnerTaskExecuted(innerSetting);
        std::vector<size_t> outerTaskIndex;
        std::vector<std::vector<size_t>> innerTaskIndex;
        GetTaskIndex(outerTaskIndex, innerTaskIndex);
        for (int i = 0; i < testTaskCount; i++) {
            result = result && IsAscend(innerTaskIndex[i]);
        }
        result = result && OuterExecutedAfter(outerSyncBarrierId);
    }
    TestUtils::PublishEvent(g_EVENT_RESP_FIRST, code, std::to_string(result));
}

// level1:parallel, sync barrier level2:serial, delay
void MainAbility::ParallelCase90(int code)
{
    APP_LOGI("-- -- -- -- -- --MainAbility::%{public}s", __FUNCTION__);
    Reset();
    TestSetting outerSetting;
    TestSetting innerSetting;
    outerSetting.dispatcher = TestDispatcher::PARALLEL;
    innerSetting.dispatcher = TestDispatcher::SERIAL;
    innerSetting.delay = delayMs;
    outerSetting.sync_barrier = true;
    outerSetting.create_group = true;
    outerSetting.op = TestOperation::ASYNC_GROUP;
    innerSetting.op = TestOperation::DELAY;
    int taskCount = Dispatch(outerSetting, innerSetting);
    bool result = Wait(taskCount);
    result = result && task_execution_sequence.size() > 1;
    if (result) {
        result = result && applyExecuted(outerSetting, innerSetting);
        result = result && OuterTaskExecuted(outerSetting);
        result = result && InnerTaskExecuted(innerSetting);
        std::vector<size_t> outerTaskIndex;
        std::vector<std::vector<size_t>> innerTaskIndex;
        GetTaskIndex(outerTaskIndex, innerTaskIndex);
        for (int i = 0; i < testTaskCount; i++) {
            result = result && IsAscend(innerTaskIndex[i]);
        }
        result = result && OuterExecutedAfter(outerSyncBarrierId);
    }
    TestUtils::PublishEvent(g_EVENT_RESP_FIRST, code, std::to_string(result));
}

// level1:parallel, sync barrier level2:serial, apply
void MainAbility::ParallelCase91(int code)
{
    APP_LOGI("-- -- -- -- -- --MainAbility::%{public}s", __FUNCTION__);
    Reset();
    TestSetting outerSetting;
    TestSetting innerSetting;
    outerSetting.dispatcher = TestDispatcher::PARALLEL;
    innerSetting.dispatcher = TestDispatcher::SERIAL;
    innerSetting.apply = applyNum;
    outerSetting.sync_barrier = true;
    outerSetting.create_group = true;
    outerSetting.op = TestOperation::ASYNC_GROUP;
    innerSetting.op = TestOperation::APPLY;
    int taskCount = Dispatch(outerSetting, innerSetting);
    bool result = Wait(taskCount);
    result = result && task_execution_sequence.size() > 1;
    if (result) {
        result = result && applyExecuted(outerSetting, innerSetting);
        result = result && OuterTaskExecuted(outerSetting);
        result = result && InnerTaskExecuted(innerSetting);
        result = result && OuterExecutedAfter(outerSyncBarrierId);
    }
    TestUtils::PublishEvent(g_EVENT_RESP_FIRST, code, std::to_string(result));
}

// level1:parallel, async barrier level2:parallel, sync
void MainAbility::ParallelCase92(int code)
{
    APP_LOGI("-- -- -- -- -- --MainAbility::%{public}s", __FUNCTION__);
    Reset();
    TestSetting outerSetting;
    TestSetting innerSetting;
    outerSetting.dispatcher = TestDispatcher::PARALLEL;
    innerSetting.dispatcher = TestDispatcher::PARALLEL;
    outerSetting.async_barrier = true;
    outerSetting.create_group = true;
    outerSetting.op = TestOperation::ASYNC_GROUP;
    innerSetting.op = TestOperation::SYNC;
    int taskCount = Dispatch(outerSetting, innerSetting);
    bool result = Wait(taskCount);
    result = result && task_execution_sequence.size() > 1;
    if (result) {
        result = result && applyExecuted(outerSetting, innerSetting);
        result = result && OuterTaskExecuted(outerSetting);
        result = result && InnerTaskExecuted(innerSetting);
        std::vector<size_t> outerTaskIndex;
        std::vector<std::vector<size_t>> innerTaskIndex;
        GetTaskIndex(outerTaskIndex, innerTaskIndex);
        for (int i = 0; i < testTaskCount; i++) {
            result = result && IsAscend(innerTaskIndex[i]);
        }
        result = result && OuterExecutedAfter(outerAsyncBarrierId);
    }
    TestUtils::PublishEvent(g_EVENT_RESP_FIRST, code, std::to_string(result));
}

// level1:parallel, async barrier level2:parallel, async
void MainAbility::ParallelCase93(int code)
{
    APP_LOGI("-- -- -- -- -- --MainAbility::%{public}s", __FUNCTION__);
    Reset();
    TestSetting outerSetting;
    TestSetting innerSetting;
    outerSetting.dispatcher = TestDispatcher::PARALLEL;
    innerSetting.dispatcher = TestDispatcher::PARALLEL;
    outerSetting.async_barrier = true;
    outerSetting.create_group = true;
    outerSetting.op = TestOperation::ASYNC_GROUP;
    innerSetting.op = TestOperation::ASYNC;
    int taskCount = Dispatch(outerSetting, innerSetting);
    bool result = Wait(taskCount);
    result = result && task_execution_sequence.size() > 1;
    if (result) {
        result = result && applyExecuted(outerSetting, innerSetting);
        result = result && OuterTaskExecuted(outerSetting);
        result = result && InnerTaskExecuted(innerSetting);
        result = result && OuterExecutedAfter(outerAsyncBarrierId);
    }
    TestUtils::PublishEvent(g_EVENT_RESP_FIRST, code, std::to_string(result));
}

// level1:parallel, async barrier level2:parallel, delay
void MainAbility::ParallelCase94(int code)
{
    APP_LOGI("-- -- -- -- -- --MainAbility::%{public}s", __FUNCTION__);
    Reset();
    TestSetting outerSetting;
    TestSetting innerSetting;
    outerSetting.dispatcher = TestDispatcher::PARALLEL;
    innerSetting.dispatcher = TestDispatcher::PARALLEL;
    innerSetting.delay = delayMs;
    outerSetting.async_barrier = true;
    outerSetting.create_group = true;
    outerSetting.op = TestOperation::ASYNC_GROUP;
    innerSetting.op = TestOperation::DELAY;
    int taskCount = Dispatch(outerSetting, innerSetting);
    bool result = Wait(taskCount);
    result = result && task_execution_sequence.size() > 1;
    if (result) {
        result = result && applyExecuted(outerSetting, innerSetting);
        result = result && OuterTaskExecuted(outerSetting);
        result = result && InnerTaskExecuted(innerSetting);
        result = result && OuterExecutedAfter(outerAsyncBarrierId);
    }
    TestUtils::PublishEvent(g_EVENT_RESP_FIRST, code, std::to_string(result));
}

// level1:parallel, async barrier level2:parallel, group
void MainAbility::ParallelCase95(int code)
{
    APP_LOGI("-- -- -- -- -- --MainAbility::%{public}s", __FUNCTION__);
    Reset();
    TestSetting outerSetting;
    TestSetting innerSetting;
    outerSetting.dispatcher = TestDispatcher::PARALLEL;
    innerSetting.dispatcher = TestDispatcher::PARALLEL;
    innerSetting.create_group = true;
    outerSetting.async_barrier = true;
    outerSetting.create_group = true;
    outerSetting.op = TestOperation::ASYNC_GROUP;
    innerSetting.op = TestOperation::ASYNC_GROUP;
    int taskCount = Dispatch(outerSetting, innerSetting);
    bool result = Wait(taskCount);
    result = result && task_execution_sequence.size() > 1;
    if (result) {
        result = result && applyExecuted(outerSetting, innerSetting);
        result = result && OuterTaskExecuted(outerSetting);
        result = result && InnerTaskExecuted(innerSetting);
        result = result && OuterExecutedAfter(outerAsyncBarrierId);
    }
    TestUtils::PublishEvent(g_EVENT_RESP_FIRST, code, std::to_string(result));
}

// level1:parallel, async barrier level2:parallel, group wait
void MainAbility::ParallelCase96(int code)
{
    APP_LOGI("-- -- -- -- -- --MainAbility::%{public}s", __FUNCTION__);
    Reset();
    TestSetting outerSetting;
    TestSetting innerSetting;
    outerSetting.dispatcher = TestDispatcher::PARALLEL;
    innerSetting.dispatcher = TestDispatcher::PARALLEL;
    innerSetting.group_wait = true;
    innerSetting.group_timeout = groupWait;
    innerSetting.create_group = true;
    outerSetting.async_barrier = true;
    outerSetting.create_group = true;
    outerSetting.op = TestOperation::ASYNC_GROUP;
    innerSetting.op = TestOperation::ASYNC_GROUP;
    int taskCount = Dispatch(outerSetting, innerSetting);
    bool result = Wait(taskCount);
    result = result && task_execution_sequence.size() > 1;
    if (result) {
        result = result && applyExecuted(outerSetting, innerSetting);
        result = result && OuterTaskExecuted(outerSetting);
        result = result && InnerTaskExecuted(innerSetting);
        result = result && OuterExecutedAfter(outerAsyncBarrierId);
    }
    TestUtils::PublishEvent(g_EVENT_RESP_FIRST, code, std::to_string(result));
}

// level1:parallel, async barrier level2:parallel, group notify
void MainAbility::ParallelCase97(int code)
{
    APP_LOGI("-- -- -- -- -- --MainAbility::%{public}s", __FUNCTION__);
    Reset();
    TestSetting outerSetting;
    TestSetting innerSetting;
    outerSetting.dispatcher = TestDispatcher::PARALLEL;
    innerSetting.dispatcher = TestDispatcher::PARALLEL;
    innerSetting.create_group = true;
    innerSetting.group_notify = true;
    outerSetting.async_barrier = true;
    outerSetting.create_group = true;
    outerSetting.op = TestOperation::ASYNC_GROUP;
    innerSetting.op = TestOperation::ASYNC_GROUP;
    int taskCount = Dispatch(outerSetting, innerSetting);
    bool result = Wait(taskCount);
    result = result && task_execution_sequence.size() > 1;
    if (result) {
        result = result && applyExecuted(outerSetting, innerSetting);
        result = result && OuterTaskExecuted(outerSetting);
        result = result && InnerTaskExecuted(innerSetting);
        result = result && OuterExecutedAfter(outerAsyncBarrierId);
        result = result && InnerExecutedAfter(innerGroupNotifyId);
    }
    TestUtils::PublishEvent(g_EVENT_RESP_FIRST, code, std::to_string(result));
}

// level1:parallel, async barrier level2:parallel, sync barrier
void MainAbility::ParallelCase98(int code)
{
    APP_LOGI("-- -- -- -- -- --MainAbility::%{public}s", __FUNCTION__);
    Reset();
    TestSetting outerSetting;
    TestSetting innerSetting;
    outerSetting.dispatcher = TestDispatcher::PARALLEL;
    innerSetting.dispatcher = TestDispatcher::PARALLEL;
    innerSetting.sync_barrier = true;
    innerSetting.create_group = true;
    outerSetting.async_barrier = true;
    outerSetting.create_group = true;
    outerSetting.op = TestOperation::ASYNC_GROUP;
    innerSetting.op = TestOperation::ASYNC_GROUP;
    int taskCount = Dispatch(outerSetting, innerSetting);
    bool result = Wait(taskCount);
    result = result && task_execution_sequence.size() > 1;
    if (result) {
        result = result && applyExecuted(outerSetting, innerSetting);
        result = result && OuterTaskExecuted(outerSetting);
        result = result && InnerTaskExecuted(innerSetting);
        result = result && OuterExecutedAfter(outerAsyncBarrierId);
        result = result && InnerExecutedAfter(innerSyncBarrierId);
    }
    TestUtils::PublishEvent(g_EVENT_RESP_FIRST, code, std::to_string(result));
}

// level1:parallel, async barrier level2:parallel, async barrier
void MainAbility::ParallelCase99(int code)
{
    APP_LOGI("-- -- -- -- -- --MainAbility::%{public}s", __FUNCTION__);
    Reset();
    TestSetting outerSetting;
    TestSetting innerSetting;
    outerSetting.dispatcher = TestDispatcher::PARALLEL;
    innerSetting.dispatcher = TestDispatcher::PARALLEL;
    innerSetting.async_barrier = true;
    innerSetting.create_group = true;
    outerSetting.async_barrier = true;
    outerSetting.create_group = true;
    outerSetting.op = TestOperation::ASYNC_GROUP;
    innerSetting.op = TestOperation::ASYNC_GROUP;
    int taskCount = Dispatch(outerSetting, innerSetting);
    bool result = Wait(taskCount);
    result = result && task_execution_sequence.size() > 1;
    if (result) {
        result = result && applyExecuted(outerSetting, innerSetting);
        result = result && OuterTaskExecuted(outerSetting);
        result = result && InnerTaskExecuted(innerSetting);
        result = result && OuterExecutedAfter(outerAsyncBarrierId);
        result = result && InnerExecutedAfter(innerAsyncBarrierId);
    }
    TestUtils::PublishEvent(g_EVENT_RESP_FIRST, code, std::to_string(result));
}

// level1:parallel, async barrier level2:parallel, apply
void MainAbility::ParallelCase100(int code)
{
    APP_LOGI("-- -- -- -- -- --MainAbility::%{public}s", __FUNCTION__);
    Reset();
    TestSetting outerSetting;
    TestSetting innerSetting;
    outerSetting.dispatcher = TestDispatcher::PARALLEL;
    innerSetting.dispatcher = TestDispatcher::PARALLEL;
    innerSetting.apply = applyNum;
    outerSetting.async_barrier = true;
    outerSetting.create_group = true;
    outerSetting.op = TestOperation::ASYNC_GROUP;
    innerSetting.op = TestOperation::APPLY;
    int taskCount = Dispatch(outerSetting, innerSetting);
    bool result = Wait(taskCount);
    result = result && task_execution_sequence.size() > 1;
    if (result) {
        result = result && applyExecuted(outerSetting, innerSetting);
        result = result && OuterTaskExecuted(outerSetting);
        result = result && InnerTaskExecuted(innerSetting);
        result = result && OuterExecutedAfter(outerAsyncBarrierId);
    }
    TestUtils::PublishEvent(g_EVENT_RESP_FIRST, code, std::to_string(result));
}

// level1:parallel, async barrier level2:serial, sync
void MainAbility::ParallelCase101(int code)
{
    APP_LOGI("-- -- -- -- -- --MainAbility::%{public}s", __FUNCTION__);
    Reset();
    TestSetting outerSetting;
    TestSetting innerSetting;
    outerSetting.dispatcher = TestDispatcher::PARALLEL;
    innerSetting.dispatcher = TestDispatcher::SERIAL;
    outerSetting.async_barrier = true;
    outerSetting.create_group = true;
    outerSetting.op = TestOperation::ASYNC_GROUP;
    innerSetting.op = TestOperation::SYNC;
    int taskCount = Dispatch(outerSetting, innerSetting);
    bool result = Wait(taskCount);
    result = result && task_execution_sequence.size() > 1;
    if (result) {
        result = result && applyExecuted(outerSetting, innerSetting);
        result = result && OuterTaskExecuted(outerSetting);
        result = result && InnerTaskExecuted(innerSetting);
        std::vector<size_t> outerTaskIndex;
        std::vector<std::vector<size_t>> innerTaskIndex;
        GetTaskIndex(outerTaskIndex, innerTaskIndex);
        for (int i = 0; i < testTaskCount; i++) {
            result = result && IsAscend(innerTaskIndex[i]);
        }
        result = result && OuterExecutedAfter(outerAsyncBarrierId);
    }
    TestUtils::PublishEvent(g_EVENT_RESP_FIRST, code, std::to_string(result));
}

// level1:parallel, async barrier level2:serial, async
void MainAbility::ParallelCase102(int code)
{
    APP_LOGI("-- -- -- -- -- --MainAbility::%{public}s", __FUNCTION__);
    Reset();
    TestSetting outerSetting;
    TestSetting innerSetting;
    outerSetting.dispatcher = TestDispatcher::PARALLEL;
    innerSetting.dispatcher = TestDispatcher::SERIAL;
    outerSetting.async_barrier = true;
    outerSetting.create_group = true;
    outerSetting.op = TestOperation::ASYNC_GROUP;
    innerSetting.op = TestOperation::ASYNC;
    int taskCount = Dispatch(outerSetting, innerSetting);
    bool result = Wait(taskCount);
    result = result && task_execution_sequence.size() > 1;
    if (result) {
        result = result && applyExecuted(outerSetting, innerSetting);
        result = result && OuterTaskExecuted(outerSetting);
        result = result && InnerTaskExecuted(innerSetting);
        std::vector<size_t> outerTaskIndex;
        std::vector<std::vector<size_t>> innerTaskIndex;
        GetTaskIndex(outerTaskIndex, innerTaskIndex);
        for (int i = 0; i < testTaskCount; i++) {
            result = result && IsAscend(innerTaskIndex[i]);
        }
        result = result && OuterExecutedAfter(outerAsyncBarrierId);
    }
    TestUtils::PublishEvent(g_EVENT_RESP_FIRST, code, std::to_string(result));
}

// level1:parallel, async barrier level2:serial, delay
void MainAbility::ParallelCase103(int code)
{
    APP_LOGI("-- -- -- -- -- --MainAbility::%{public}s", __FUNCTION__);
    Reset();
    TestSetting outerSetting;
    TestSetting innerSetting;
    outerSetting.dispatcher = TestDispatcher::PARALLEL;
    innerSetting.dispatcher = TestDispatcher::SERIAL;
    innerSetting.delay = delayMs;
    outerSetting.async_barrier = true;
    outerSetting.create_group = true;
    outerSetting.op = TestOperation::ASYNC_GROUP;
    innerSetting.op = TestOperation::DELAY;
    int taskCount = Dispatch(outerSetting, innerSetting);
    bool result = Wait(taskCount);
    result = result && task_execution_sequence.size() > 1;
    if (result) {
        result = result && applyExecuted(outerSetting, innerSetting);
        result = result && OuterTaskExecuted(outerSetting);
        result = result && InnerTaskExecuted(innerSetting);
        std::vector<size_t> outerTaskIndex;
        std::vector<std::vector<size_t>> innerTaskIndex;
        GetTaskIndex(outerTaskIndex, innerTaskIndex);
        for (int i = 0; i < testTaskCount; i++) {
            result = result && IsAscend(innerTaskIndex[i]);
        }
        result = result && OuterExecutedAfter(outerAsyncBarrierId);
    }
    TestUtils::PublishEvent(g_EVENT_RESP_FIRST, code, std::to_string(result));
}

// level1:parallel, async barrier level2:serial, apply
void MainAbility::ParallelCase104(int code)
{
    APP_LOGI("-- -- -- -- -- --MainAbility::%{public}s", __FUNCTION__);
    Reset();
    TestSetting outerSetting;
    TestSetting innerSetting;
    outerSetting.dispatcher = TestDispatcher::PARALLEL;
    innerSetting.dispatcher = TestDispatcher::SERIAL;
    innerSetting.apply = applyNum;
    outerSetting.async_barrier = true;
    outerSetting.create_group = true;
    outerSetting.op = TestOperation::ASYNC_GROUP;
    innerSetting.op = TestOperation::APPLY;
    int taskCount = Dispatch(outerSetting, innerSetting);
    bool result = Wait(taskCount);
    result = result && task_execution_sequence.size() > 1;
    if (result) {
        result = result && applyExecuted(outerSetting, innerSetting);
        result = result && OuterTaskExecuted(outerSetting);
        result = result && InnerTaskExecuted(innerSetting);
        result = result && OuterExecutedAfter(outerAsyncBarrierId);
    }
    TestUtils::PublishEvent(g_EVENT_RESP_FIRST, code, std::to_string(result));
}

// level1:parallel, apply level2:parallel, sync
void MainAbility::ParallelCase105(int code)
{
    APP_LOGI("-- -- -- -- -- --MainAbility::%{public}s", __FUNCTION__);
    Reset();
    TestSetting outerSetting;
    TestSetting innerSetting;
    outerSetting.dispatcher = TestDispatcher::PARALLEL;
    innerSetting.dispatcher = TestDispatcher::PARALLEL;
    outerSetting.apply = applyNum;
    outerSetting.op = TestOperation::APPLY;
    innerSetting.op = TestOperation::SYNC;
    int taskCount = Dispatch(outerSetting, innerSetting);
    bool result = Wait(taskCount);
    result = result && task_execution_sequence.size() > 1;
    if (result) {
        result = result && applyExecuted(outerSetting, innerSetting);
        result = result && OuterTaskExecuted(outerSetting);
        result = result && InnerTaskExecuted(innerSetting);
        std::vector<size_t> outerTaskIndex;
        std::vector<std::vector<size_t>> innerTaskIndex;
        GetTaskIndex(outerTaskIndex, innerTaskIndex);
        for (int i = 0; i < testTaskCount; i++) {
            result = result && IsAscend(innerTaskIndex[i]);
        }
    }
    TestUtils::PublishEvent(g_EVENT_RESP_FIRST, code, std::to_string(result));
}

// level1:parallel, apply level2:parallel, async
void MainAbility::ParallelCase106(int code)
{
    APP_LOGI("-- -- -- -- -- --MainAbility::%{public}s", __FUNCTION__);
    Reset();
    TestSetting outerSetting;
    TestSetting innerSetting;
    outerSetting.dispatcher = TestDispatcher::PARALLEL;
    innerSetting.dispatcher = TestDispatcher::PARALLEL;
    outerSetting.apply = applyNum;
    outerSetting.op = TestOperation::APPLY;
    innerSetting.op = TestOperation::ASYNC;
    int taskCount = Dispatch(outerSetting, innerSetting);
    bool result = Wait(taskCount);
    result = result && task_execution_sequence.size() > 1;
    if (result) {
        result = result && applyExecuted(outerSetting, innerSetting);
        result = result && OuterTaskExecuted(outerSetting);
        result = result && InnerTaskExecuted(innerSetting);
    }
    TestUtils::PublishEvent(g_EVENT_RESP_FIRST, code, std::to_string(result));
}

// level1:parallel, apply level2:parallel, delay
void MainAbility::ParallelCase107(int code)
{
    APP_LOGI("-- -- -- -- -- --MainAbility::%{public}s", __FUNCTION__);
    Reset();
    TestSetting outerSetting;
    TestSetting innerSetting;
    outerSetting.dispatcher = TestDispatcher::PARALLEL;
    innerSetting.dispatcher = TestDispatcher::PARALLEL;
    outerSetting.apply = applyNum;
    innerSetting.delay = delayMs;
    outerSetting.op = TestOperation::APPLY;
    innerSetting.op = TestOperation::DELAY;
    int taskCount = Dispatch(outerSetting, innerSetting);
    bool result = Wait(taskCount);
    result = result && task_execution_sequence.size() > 1;
    if (result) {
        result = result && applyExecuted(outerSetting, innerSetting);
        result = result && OuterTaskExecuted(outerSetting);
        result = result && InnerTaskExecuted(innerSetting);
    }
    TestUtils::PublishEvent(g_EVENT_RESP_FIRST, code, std::to_string(result));
}

// level1:parallel, apply level2:parallel, group
void MainAbility::ParallelCase108(int code)
{
    APP_LOGI("-- -- -- -- -- --MainAbility::%{public}s", __FUNCTION__);
    Reset();
    TestSetting outerSetting;
    TestSetting innerSetting;
    outerSetting.dispatcher = TestDispatcher::PARALLEL;
    innerSetting.dispatcher = TestDispatcher::PARALLEL;
    outerSetting.apply = applyNum;
    innerSetting.create_group = true;
    outerSetting.op = TestOperation::APPLY;
    innerSetting.op = TestOperation::ASYNC_GROUP;
    int taskCount = Dispatch(outerSetting, innerSetting);
    bool result = Wait(taskCount);
    result = result && task_execution_sequence.size() > 1;
    if (result) {
        result = result && applyExecuted(outerSetting, innerSetting);
        result = result && OuterTaskExecuted(outerSetting);
        result = result && InnerTaskExecuted(innerSetting);
    }
    TestUtils::PublishEvent(g_EVENT_RESP_FIRST, code, std::to_string(result));
}

// level1:parallel, apply level2:parallel, group wait
void MainAbility::ParallelCase109(int code)
{
    APP_LOGI("-- -- -- -- -- --MainAbility::%{public}s", __FUNCTION__);
    Reset();
    TestSetting outerSetting;
    TestSetting innerSetting;
    outerSetting.dispatcher = TestDispatcher::PARALLEL;
    innerSetting.dispatcher = TestDispatcher::PARALLEL;
    innerSetting.group_wait = true;
    innerSetting.group_timeout = groupWait;
    outerSetting.apply = applyNum;
    innerSetting.create_group = true;
    outerSetting.op = TestOperation::APPLY;
    innerSetting.op = TestOperation::ASYNC_GROUP;
    int taskCount = Dispatch(outerSetting, innerSetting);
    bool result = Wait(taskCount);
    result = result && task_execution_sequence.size() > 1;
    if (result) {
        result = result && applyExecuted(outerSetting, innerSetting);
        result = result && OuterTaskExecuted(outerSetting);
        result = result && InnerTaskExecuted(innerSetting);
    }
    TestUtils::PublishEvent(g_EVENT_RESP_FIRST, code, std::to_string(result));
}

// level1:parallel, apply level2:parallel, group notify
void MainAbility::ParallelCase110(int code)
{
    APP_LOGI("-- -- -- -- -- --MainAbility::%{public}s", __FUNCTION__);
    Reset();
    TestSetting outerSetting;
    TestSetting innerSetting;
    outerSetting.dispatcher = TestDispatcher::PARALLEL;
    innerSetting.dispatcher = TestDispatcher::PARALLEL;
    outerSetting.apply = applyNum;
    innerSetting.create_group = true;
    innerSetting.group_notify = true;
    outerSetting.op = TestOperation::APPLY;
    innerSetting.op = TestOperation::ASYNC_GROUP;
    int taskCount = Dispatch(outerSetting, innerSetting);
    bool result = Wait(taskCount);
    result = result && task_execution_sequence.size() > 1;
    if (result) {
        result = result && applyExecuted(outerSetting, innerSetting);
        result = result && OuterTaskExecuted(outerSetting);
        result = result && InnerTaskExecuted(innerSetting);
        result = result && InnerExecutedAfter(innerGroupNotifyId);
    }
    TestUtils::PublishEvent(g_EVENT_RESP_FIRST, code, std::to_string(result));
}

// level1:parallel, apply level2:parallel, sync barrier
void MainAbility::ParallelCase111(int code)
{
    APP_LOGI("-- -- -- -- -- --MainAbility::%{public}s", __FUNCTION__);
    Reset();
    TestSetting outerSetting;
    TestSetting innerSetting;
    outerSetting.dispatcher = TestDispatcher::PARALLEL;
    innerSetting.dispatcher = TestDispatcher::PARALLEL;
    outerSetting.apply = applyNum;
    innerSetting.sync_barrier = true;
    innerSetting.create_group = true;
    outerSetting.op = TestOperation::APPLY;
    innerSetting.op = TestOperation::ASYNC_GROUP;
    int taskCount = Dispatch(outerSetting, innerSetting);
    bool result = Wait(taskCount);
    result = result && task_execution_sequence.size() > 1;
    if (result) {
        result = result && applyExecuted(outerSetting, innerSetting);
        result = result && OuterTaskExecuted(outerSetting);
        result = result && InnerTaskExecuted(innerSetting);
        result = result && InnerExecutedAfter(innerSyncBarrierId);
    }
    TestUtils::PublishEvent(g_EVENT_RESP_FIRST, code, std::to_string(result));
}

// level1:parallel, apply level2:parallel, async barrier
void MainAbility::ParallelCase112(int code)
{
    APP_LOGI("-- -- -- -- -- --MainAbility::%{public}s", __FUNCTION__);
    Reset();
    TestSetting outerSetting;
    TestSetting innerSetting;
    outerSetting.dispatcher = TestDispatcher::PARALLEL;
    innerSetting.dispatcher = TestDispatcher::PARALLEL;
    outerSetting.apply = applyNum;
    innerSetting.async_barrier = true;
    innerSetting.create_group = true;
    outerSetting.op = TestOperation::APPLY;
    innerSetting.op = TestOperation::ASYNC_GROUP;
    int taskCount = Dispatch(outerSetting, innerSetting);
    bool result = Wait(taskCount);
    result = result && task_execution_sequence.size() > 1;
    if (result) {
        result = result && applyExecuted(outerSetting, innerSetting);
        result = result && OuterTaskExecuted(outerSetting);
        result = result && InnerTaskExecuted(innerSetting);
        result = result && InnerExecutedAfter(innerAsyncBarrierId);
    }
    TestUtils::PublishEvent(g_EVENT_RESP_FIRST, code, std::to_string(result));
}

// level1:parallel, apply level2:parallel, apply
void MainAbility::ParallelCase113(int code)
{
    APP_LOGI("-- -- -- -- -- --MainAbility::%{public}s", __FUNCTION__);
    Reset();
    TestSetting outerSetting;
    TestSetting innerSetting;
    outerSetting.dispatcher = TestDispatcher::PARALLEL;
    innerSetting.dispatcher = TestDispatcher::PARALLEL;
    outerSetting.apply = applyNum;
    innerSetting.apply = applyNum;
    outerSetting.op = TestOperation::APPLY;
    innerSetting.op = TestOperation::APPLY;
    int taskCount = Dispatch(outerSetting, innerSetting);
    bool result = Wait(taskCount);
    result = result && task_execution_sequence.size() > 1;
    if (result) {
        result = result && applyExecuted(outerSetting, innerSetting);
        result = result && OuterTaskExecuted(outerSetting);
        result = result && InnerTaskExecuted(innerSetting);
    }
    TestUtils::PublishEvent(g_EVENT_RESP_FIRST, code, std::to_string(result));
}

// level1:parallel, apply level2:serial, sync
void MainAbility::ParallelCase114(int code)
{
    APP_LOGI("-- -- -- -- -- --MainAbility::%{public}s", __FUNCTION__);
    Reset();
    TestSetting outerSetting;
    TestSetting innerSetting;
    outerSetting.dispatcher = TestDispatcher::PARALLEL;
    innerSetting.dispatcher = TestDispatcher::SERIAL;
    outerSetting.apply = applyNum;
    outerSetting.op = TestOperation::APPLY;
    innerSetting.op = TestOperation::SYNC;
    int taskCount = Dispatch(outerSetting, innerSetting);
    bool result = Wait(taskCount);
    result = result && task_execution_sequence.size() > 1;
    if (result) {
        result = result && applyExecuted(outerSetting, innerSetting);
        result = result && OuterTaskExecuted(outerSetting);
        result = result && InnerTaskExecuted(innerSetting);
        std::vector<size_t> outerTaskIndex;
        std::vector<std::vector<size_t>> innerTaskIndex;
        GetTaskIndex(outerTaskIndex, innerTaskIndex);
        for (int i = 0; i < testTaskCount; i++) {
            result = result && IsAscend(innerTaskIndex[i]);
        }
    }
    TestUtils::PublishEvent(g_EVENT_RESP_FIRST, code, std::to_string(result));
}

// level1:parallel, apply level2:serial, async
void MainAbility::ParallelCase115(int code)
{
    APP_LOGI("-- -- -- -- -- --MainAbility::%{public}s", __FUNCTION__);
    Reset();
    TestSetting outerSetting;
    TestSetting innerSetting;
    outerSetting.dispatcher = TestDispatcher::PARALLEL;
    innerSetting.dispatcher = TestDispatcher::SERIAL;
    outerSetting.apply = applyNum;
    outerSetting.op = TestOperation::APPLY;
    innerSetting.op = TestOperation::ASYNC;
    int taskCount = Dispatch(outerSetting, innerSetting);
    bool result = Wait(taskCount);
    result = result && task_execution_sequence.size() > 1;
    if (result) {
        result = result && applyExecuted(outerSetting, innerSetting);
        result = result && OuterTaskExecuted(outerSetting);
        result = result && InnerTaskExecuted(innerSetting);
        std::vector<size_t> outerTaskIndex;
        std::vector<std::vector<size_t>> innerTaskIndex;
        GetTaskIndex(outerTaskIndex, innerTaskIndex);
        for (int i = 0; i < testTaskCount; i++) {
            result = result && IsAscend(innerTaskIndex[i]);
        }
    }
    TestUtils::PublishEvent(g_EVENT_RESP_FIRST, code, std::to_string(result));
}

// level1:parallel, apply level2:serial, delay
void MainAbility::ParallelCase116(int code)
{
    APP_LOGI("-- -- -- -- -- --MainAbility::%{public}s", __FUNCTION__);
    Reset();
    TestSetting outerSetting;
    TestSetting innerSetting;
    outerSetting.dispatcher = TestDispatcher::PARALLEL;
    innerSetting.dispatcher = TestDispatcher::SERIAL;
    outerSetting.apply = applyNum;
    innerSetting.delay = delayMs;
    outerSetting.op = TestOperation::APPLY;
    innerSetting.op = TestOperation::DELAY;
    int taskCount = Dispatch(outerSetting, innerSetting);
    bool result = Wait(taskCount);
    result = result && task_execution_sequence.size() > 1;
    if (result) {
        result = result && applyExecuted(outerSetting, innerSetting);
        result = result && OuterTaskExecuted(outerSetting);
        result = result && InnerTaskExecuted(innerSetting);
        std::vector<size_t> outerTaskIndex;
        std::vector<std::vector<size_t>> innerTaskIndex;
        GetTaskIndex(outerTaskIndex, innerTaskIndex);
        for (int i = 0; i < testTaskCount; i++) {
            result = result && IsAscend(innerTaskIndex[i]);
        }
    }
    TestUtils::PublishEvent(g_EVENT_RESP_FIRST, code, std::to_string(result));
}

// level1:parallel, apply level2:serial, apply
void MainAbility::ParallelCase117(int code)
{
    APP_LOGI("-- -- -- -- -- --MainAbility::%{public}s", __FUNCTION__);
    Reset();
    TestSetting outerSetting;
    TestSetting innerSetting;
    outerSetting.dispatcher = TestDispatcher::PARALLEL;
    innerSetting.dispatcher = TestDispatcher::SERIAL;
    outerSetting.apply = applyNum;
    innerSetting.apply = applyNum;
    outerSetting.op = TestOperation::APPLY;
    innerSetting.op = TestOperation::APPLY;
    int taskCount = Dispatch(outerSetting, innerSetting);
    bool result = Wait(taskCount);
    result = result && task_execution_sequence.size() > 1;
    if (result) {
        result = result && applyExecuted(outerSetting, innerSetting);
        result = result && OuterTaskExecuted(outerSetting);
        result = result && InnerTaskExecuted(innerSetting);
    }
    TestUtils::PublishEvent(g_EVENT_RESP_FIRST, code, std::to_string(result));
}

// level1:serial, sync level2:parallel, sync
void MainAbility::SerialCase1(int code)
{
    APP_LOGI("-- -- -- -- -- --MainAbility::%{public}s", __FUNCTION__);
    Reset();
    TestSetting outerSetting;
    TestSetting innerSetting;
    outerSetting.dispatcher = TestDispatcher::SERIAL;
    innerSetting.dispatcher = TestDispatcher::PARALLEL;
    outerSetting.op = TestOperation::SYNC;
    innerSetting.op = TestOperation::SYNC;
    int taskCount = Dispatch(outerSetting, innerSetting);
    bool result = Wait(taskCount);
    result = result && task_execution_sequence.size() > 1;
    if (result) {
        result = result && applyExecuted(outerSetting, innerSetting);
        result = result && OuterTaskExecuted(outerSetting);
        result = result && InnerTaskExecuted(innerSetting);
        std::vector<size_t> outerTaskIndex;
        std::vector<std::vector<size_t>> innerTaskIndex;
        GetTaskIndex(outerTaskIndex, innerTaskIndex);
        result = result && IsAscend(outerTaskIndex);
        for (int i = 0; i < testTaskCount; i++) {
            result = result && IsAscend(innerTaskIndex[i]);
        }
    }
    TestUtils::PublishEvent(g_EVENT_RESP_FIRST, code, std::to_string(result));
}

// level1:serial, sync level2:parallel, async
void MainAbility::SerialCase2(int code)
{
    APP_LOGI("-- -- -- -- -- --MainAbility::%{public}s", __FUNCTION__);
    Reset();
    TestSetting outerSetting;
    TestSetting innerSetting;
    outerSetting.dispatcher = TestDispatcher::SERIAL;
    innerSetting.dispatcher = TestDispatcher::PARALLEL;
    outerSetting.op = TestOperation::SYNC;
    innerSetting.op = TestOperation::ASYNC;
    int taskCount = Dispatch(outerSetting, innerSetting);
    bool result = Wait(taskCount);
    result = result && task_execution_sequence.size() > 1;
    if (result) {
        result = result && applyExecuted(outerSetting, innerSetting);
        result = result && OuterTaskExecuted(outerSetting);
        result = result && InnerTaskExecuted(innerSetting);
        std::vector<size_t> outerTaskIndex;
        std::vector<std::vector<size_t>> innerTaskIndex;
        GetTaskIndex(outerTaskIndex, innerTaskIndex);
        result = result && IsAscend(outerTaskIndex);
    }
    TestUtils::PublishEvent(g_EVENT_RESP_FIRST, code, std::to_string(result));
}

// level1:serial, sync level2:parallel, delay
void MainAbility::SerialCase3(int code)
{
    APP_LOGI("-- -- -- -- -- --MainAbility::%{public}s", __FUNCTION__);
    Reset();
    TestSetting outerSetting;
    TestSetting innerSetting;
    outerSetting.dispatcher = TestDispatcher::SERIAL;
    innerSetting.dispatcher = TestDispatcher::PARALLEL;
    innerSetting.delay = delayMs;
    outerSetting.op = TestOperation::SYNC;
    innerSetting.op = TestOperation::DELAY;
    int taskCount = Dispatch(outerSetting, innerSetting);
    bool result = Wait(taskCount);
    result = result && task_execution_sequence.size() > 1;
    if (result) {
        result = result && applyExecuted(outerSetting, innerSetting);
        result = result && OuterTaskExecuted(outerSetting);
        result = result && InnerTaskExecuted(innerSetting);
        std::vector<size_t> outerTaskIndex;
        std::vector<std::vector<size_t>> innerTaskIndex;
        GetTaskIndex(outerTaskIndex, innerTaskIndex);
        result = result && IsAscend(outerTaskIndex);
    }
    TestUtils::PublishEvent(g_EVENT_RESP_FIRST, code, std::to_string(result));
}

// level1:serial, sync level2:parallel, group
void MainAbility::SerialCase4(int code)
{
    APP_LOGI("-- -- -- -- -- --MainAbility::%{public}s", __FUNCTION__);
    Reset();
    TestSetting outerSetting;
    TestSetting innerSetting;
    outerSetting.dispatcher = TestDispatcher::SERIAL;
    innerSetting.dispatcher = TestDispatcher::PARALLEL;
    innerSetting.create_group = true;
    outerSetting.op = TestOperation::SYNC;
    innerSetting.op = TestOperation::ASYNC_GROUP;
    int taskCount = Dispatch(outerSetting, innerSetting);
    bool result = Wait(taskCount);
    result = result && task_execution_sequence.size() > 1;
    if (result) {
        result = result && applyExecuted(outerSetting, innerSetting);
        result = result && OuterTaskExecuted(outerSetting);
        result = result && InnerTaskExecuted(innerSetting);
        std::vector<size_t> outerTaskIndex;
        std::vector<std::vector<size_t>> innerTaskIndex;
        GetTaskIndex(outerTaskIndex, innerTaskIndex);
        result = result && IsAscend(outerTaskIndex);
    }
    TestUtils::PublishEvent(g_EVENT_RESP_FIRST, code, std::to_string(result));
}

// level1:serial, sync level2:parallel, group wait
void MainAbility::SerialCase5(int code)
{
    APP_LOGI("-- -- -- -- -- --MainAbility::%{public}s", __FUNCTION__);
    Reset();
    TestSetting outerSetting;
    TestSetting innerSetting;
    outerSetting.dispatcher = TestDispatcher::SERIAL;
    innerSetting.dispatcher = TestDispatcher::PARALLEL;
    innerSetting.group_wait = true;
    innerSetting.group_timeout = groupWait;
    innerSetting.create_group = true;
    outerSetting.op = TestOperation::SYNC;
    innerSetting.op = TestOperation::ASYNC_GROUP;
    int taskCount = Dispatch(outerSetting, innerSetting);
    bool result = Wait(taskCount);
    result = result && task_execution_sequence.size() > 1;
    if (result) {
        result = result && applyExecuted(outerSetting, innerSetting);
        result = result && OuterTaskExecuted(outerSetting);
        result = result && InnerTaskExecuted(innerSetting);
        std::vector<size_t> outerTaskIndex;
        std::vector<std::vector<size_t>> innerTaskIndex;
        GetTaskIndex(outerTaskIndex, innerTaskIndex);
        result = result && IsAscend(outerTaskIndex);
    }
    TestUtils::PublishEvent(g_EVENT_RESP_FIRST, code, std::to_string(result));
}

// level1:serial, sync level2:parallel, group notify
void MainAbility::SerialCase6(int code)
{
    APP_LOGI("-- -- -- -- -- --MainAbility::%{public}s", __FUNCTION__);
    Reset();
    TestSetting outerSetting;
    TestSetting innerSetting;
    outerSetting.dispatcher = TestDispatcher::SERIAL;
    innerSetting.dispatcher = TestDispatcher::PARALLEL;
    innerSetting.create_group = true;
    innerSetting.group_notify = true;
    outerSetting.op = TestOperation::SYNC;
    innerSetting.op = TestOperation::ASYNC_GROUP;
    int taskCount = Dispatch(outerSetting, innerSetting);
    bool result = Wait(taskCount);
    result = result && task_execution_sequence.size() > 1;
    if (result) {
        result = result && applyExecuted(outerSetting, innerSetting);
        result = result && OuterTaskExecuted(outerSetting);
        result = result && InnerTaskExecuted(innerSetting);
        std::vector<size_t> outerTaskIndex;
        std::vector<std::vector<size_t>> innerTaskIndex;
        GetTaskIndex(outerTaskIndex, innerTaskIndex);
        result = result && IsAscend(outerTaskIndex);
        result = result && InnerExecutedAfter(innerGroupNotifyId);
    }
    TestUtils::PublishEvent(g_EVENT_RESP_FIRST, code, std::to_string(result));
}

// level1:serial, sync level2:parallel, sync barrier
void MainAbility::SerialCase7(int code)
{
    APP_LOGI("-- -- -- -- -- --MainAbility::%{public}s", __FUNCTION__);
    Reset();
    TestSetting outerSetting;
    TestSetting innerSetting;
    outerSetting.dispatcher = TestDispatcher::SERIAL;
    innerSetting.dispatcher = TestDispatcher::PARALLEL;
    innerSetting.sync_barrier = true;
    innerSetting.create_group = true;
    outerSetting.op = TestOperation::SYNC;
    innerSetting.op = TestOperation::ASYNC_GROUP;
    int taskCount = Dispatch(outerSetting, innerSetting);
    bool result = Wait(taskCount);
    result = result && task_execution_sequence.size() > 1;
    if (result) {
        result = result && applyExecuted(outerSetting, innerSetting);
        result = result && OuterTaskExecuted(outerSetting);
        result = result && InnerTaskExecuted(innerSetting);
        std::vector<size_t> outerTaskIndex;
        std::vector<std::vector<size_t>> innerTaskIndex;
        GetTaskIndex(outerTaskIndex, innerTaskIndex);
        result = result && IsAscend(outerTaskIndex);
        result = result && InnerExecutedAfter(innerSyncBarrierId);
    }
    TestUtils::PublishEvent(g_EVENT_RESP_FIRST, code, std::to_string(result));
}

// level1:serial, sync level2:parallel, async barrier
void MainAbility::SerialCase8(int code)
{
    APP_LOGI("-- -- -- -- -- --MainAbility::%{public}s", __FUNCTION__);
    Reset();
    TestSetting outerSetting;
    TestSetting innerSetting;
    outerSetting.dispatcher = TestDispatcher::SERIAL;
    innerSetting.dispatcher = TestDispatcher::PARALLEL;
    innerSetting.async_barrier = true;
    innerSetting.create_group = true;
    outerSetting.op = TestOperation::SYNC;
    innerSetting.op = TestOperation::ASYNC_GROUP;
    int taskCount = Dispatch(outerSetting, innerSetting);
    bool result = Wait(taskCount);
    result = result && task_execution_sequence.size() > 1;
    if (result) {
        result = result && applyExecuted(outerSetting, innerSetting);
        result = result && OuterTaskExecuted(outerSetting);
        result = result && InnerTaskExecuted(innerSetting);
        std::vector<size_t> outerTaskIndex;
        std::vector<std::vector<size_t>> innerTaskIndex;
        GetTaskIndex(outerTaskIndex, innerTaskIndex);
        result = result && IsAscend(outerTaskIndex);
        result = result && InnerExecutedAfter(innerAsyncBarrierId);
    }
    TestUtils::PublishEvent(g_EVENT_RESP_FIRST, code, std::to_string(result));
}

// level1:serial, sync level2:parallel, apply
void MainAbility::SerialCase9(int code)
{
    APP_LOGI("-- -- -- -- -- --MainAbility::%{public}s", __FUNCTION__);
    Reset();
    TestSetting outerSetting;
    TestSetting innerSetting;
    outerSetting.dispatcher = TestDispatcher::SERIAL;
    innerSetting.dispatcher = TestDispatcher::PARALLEL;
    innerSetting.apply = applyNum;
    outerSetting.op = TestOperation::SYNC;
    innerSetting.op = TestOperation::APPLY;
    int taskCount = Dispatch(outerSetting, innerSetting);
    bool result = Wait(taskCount);
    result = result && task_execution_sequence.size() > 1;
    if (result) {
        result = result && applyExecuted(outerSetting, innerSetting);
        result = result && OuterTaskExecuted(outerSetting);
        result = result && InnerTaskExecuted(innerSetting);
        std::vector<size_t> outerTaskIndex;
        std::vector<std::vector<size_t>> innerTaskIndex;
        GetTaskIndex(outerTaskIndex, innerTaskIndex);
        result = result && IsAscend(outerTaskIndex);
    }
    TestUtils::PublishEvent(g_EVENT_RESP_FIRST, code, std::to_string(result));
}

// level1:serial, sync level2:serial, sync
void MainAbility::SerialCase10(int code)
{
    APP_LOGI("-- -- -- -- -- --MainAbility::%{public}s", __FUNCTION__);
    Reset();
    TestSetting outerSetting;
    TestSetting innerSetting;
    outerSetting.dispatcher = TestDispatcher::SERIAL;
    innerSetting.dispatcher = TestDispatcher::SERIAL;
    outerSetting.op = TestOperation::SYNC;
    innerSetting.op = TestOperation::SYNC;
    int taskCount = Dispatch(outerSetting, innerSetting);
    bool result = Wait(taskCount);
    result = result && task_execution_sequence.size() > 1;
    if (result) {
        result = result && applyExecuted(outerSetting, innerSetting);
        result = result && OuterTaskExecuted(outerSetting);
        result = result && InnerTaskExecuted(innerSetting);
        std::vector<size_t> outerTaskIndex;
        std::vector<std::vector<size_t>> innerTaskIndex;
        GetTaskIndex(outerTaskIndex, innerTaskIndex);
        result = result && IsAscend(outerTaskIndex);
        for (int i = 0; i < testTaskCount; i++) {
            result = result && IsAscend(innerTaskIndex[i]);
        }
    }
    TestUtils::PublishEvent(g_EVENT_RESP_FIRST, code, std::to_string(result));
}

// level1:serial, sync level2:serial, async
void MainAbility::SerialCase11(int code)
{
    APP_LOGI("-- -- -- -- -- --MainAbility::%{public}s", __FUNCTION__);
    Reset();
    TestSetting outerSetting;
    TestSetting innerSetting;
    outerSetting.dispatcher = TestDispatcher::SERIAL;
    innerSetting.dispatcher = TestDispatcher::SERIAL;
    outerSetting.op = TestOperation::SYNC;
    innerSetting.op = TestOperation::ASYNC;
    int taskCount = Dispatch(outerSetting, innerSetting);
    bool result = Wait(taskCount);
    result = result && task_execution_sequence.size() > 1;
    if (result) {
        result = result && applyExecuted(outerSetting, innerSetting);
        result = result && OuterTaskExecuted(outerSetting);
        result = result && InnerTaskExecuted(innerSetting);
        std::vector<size_t> outerTaskIndex;
        std::vector<std::vector<size_t>> innerTaskIndex;
        GetTaskIndex(outerTaskIndex, innerTaskIndex);
        result = result && IsAscend(outerTaskIndex);
        for (int i = 0; i < testTaskCount; i++) {
            result = result && IsAscend(innerTaskIndex[i]);
        }
    }
    TestUtils::PublishEvent(g_EVENT_RESP_FIRST, code, std::to_string(result));
}

// level1:serial, sync level2:serial, delay
void MainAbility::SerialCase12(int code)
{
    APP_LOGI("-- -- -- -- -- --MainAbility::%{public}s", __FUNCTION__);
    Reset();
    TestSetting outerSetting;
    TestSetting innerSetting;
    outerSetting.dispatcher = TestDispatcher::SERIAL;
    innerSetting.dispatcher = TestDispatcher::SERIAL;
    innerSetting.delay = delayMs;
    outerSetting.op = TestOperation::SYNC;
    innerSetting.op = TestOperation::DELAY;
    int taskCount = Dispatch(outerSetting, innerSetting);
    bool result = Wait(taskCount);
    result = result && task_execution_sequence.size() > 1;
    if (result) {
        result = result && applyExecuted(outerSetting, innerSetting);
        result = result && OuterTaskExecuted(outerSetting);
        result = result && InnerTaskExecuted(innerSetting);
        std::vector<size_t> outerTaskIndex;
        std::vector<std::vector<size_t>> innerTaskIndex;
        GetTaskIndex(outerTaskIndex, innerTaskIndex);
        result = result && IsAscend(outerTaskIndex);
        for (int i = 0; i < testTaskCount; i++) {
            result = result && IsAscend(innerTaskIndex[i]);
        }
    }
    TestUtils::PublishEvent(g_EVENT_RESP_FIRST, code, std::to_string(result));
}

// level1:serial, sync level2:serial, apply
void MainAbility::SerialCase13(int code)
{
    APP_LOGI("-- -- -- -- -- --MainAbility::%{public}s", __FUNCTION__);
    Reset();
    TestSetting outerSetting;
    TestSetting innerSetting;
    outerSetting.dispatcher = TestDispatcher::SERIAL;
    innerSetting.dispatcher = TestDispatcher::SERIAL;
    innerSetting.apply = applyNum;
    outerSetting.op = TestOperation::SYNC;
    innerSetting.op = TestOperation::APPLY;
    int taskCount = Dispatch(outerSetting, innerSetting);
    bool result = Wait(taskCount);
    result = result && task_execution_sequence.size() > 1;
    if (result) {
        result = result && applyExecuted(outerSetting, innerSetting);
        result = result && OuterTaskExecuted(outerSetting);
        result = result && InnerTaskExecuted(innerSetting);
        std::vector<size_t> outerTaskIndex;
        std::vector<std::vector<size_t>> innerTaskIndex;
        GetTaskIndex(outerTaskIndex, innerTaskIndex);
        result = result && IsAscend(outerTaskIndex);
    }
    TestUtils::PublishEvent(g_EVENT_RESP_FIRST, code, std::to_string(result));
}

// level1:serial, async level2:parallel, sync
void MainAbility::SerialCase14(int code)
{
    APP_LOGI("-- -- -- -- -- --MainAbility::%{public}s", __FUNCTION__);
    Reset();
    TestSetting outerSetting;
    TestSetting innerSetting;
    outerSetting.dispatcher = TestDispatcher::SERIAL;
    innerSetting.dispatcher = TestDispatcher::PARALLEL;
    outerSetting.op = TestOperation::ASYNC;
    innerSetting.op = TestOperation::SYNC;
    int taskCount = Dispatch(outerSetting, innerSetting);
    bool result = Wait(taskCount);
    result = result && task_execution_sequence.size() > 1;
    if (result) {
        result = result && applyExecuted(outerSetting, innerSetting);
        result = result && OuterTaskExecuted(outerSetting);
        result = result && InnerTaskExecuted(innerSetting);
        std::vector<size_t> outerTaskIndex;
        std::vector<std::vector<size_t>> innerTaskIndex;
        GetTaskIndex(outerTaskIndex, innerTaskIndex);
        result = result && IsAscend(outerTaskIndex);
        for (int i = 0; i < testTaskCount; i++) {
            result = result && IsAscend(innerTaskIndex[i]);
        }
    }
    TestUtils::PublishEvent(g_EVENT_RESP_FIRST, code, std::to_string(result));
}

// level1:serial, async level2:parallel, async
void MainAbility::SerialCase15(int code)
{
    APP_LOGI("-- -- -- -- -- --MainAbility::%{public}s", __FUNCTION__);
    Reset();
    TestSetting outerSetting;
    TestSetting innerSetting;
    outerSetting.dispatcher = TestDispatcher::SERIAL;
    innerSetting.dispatcher = TestDispatcher::PARALLEL;
    outerSetting.op = TestOperation::ASYNC;
    innerSetting.op = TestOperation::ASYNC;
    int taskCount = Dispatch(outerSetting, innerSetting);
    bool result = Wait(taskCount);
    result = result && task_execution_sequence.size() > 1;
    if (result) {
        result = result && applyExecuted(outerSetting, innerSetting);
        result = result && OuterTaskExecuted(outerSetting);
        result = result && InnerTaskExecuted(innerSetting);
        std::vector<size_t> outerTaskIndex;
        std::vector<std::vector<size_t>> innerTaskIndex;
        GetTaskIndex(outerTaskIndex, innerTaskIndex);
        result = result && IsAscend(outerTaskIndex);
    }
    TestUtils::PublishEvent(g_EVENT_RESP_FIRST, code, std::to_string(result));
}

// level1:serial, async level2:parallel, delay
void MainAbility::SerialCase16(int code)
{
    APP_LOGI("-- -- -- -- -- --MainAbility::%{public}s", __FUNCTION__);
    Reset();
    TestSetting outerSetting;
    TestSetting innerSetting;
    outerSetting.dispatcher = TestDispatcher::SERIAL;
    innerSetting.dispatcher = TestDispatcher::PARALLEL;
    innerSetting.delay = delayMs;
    outerSetting.op = TestOperation::ASYNC;
    innerSetting.op = TestOperation::DELAY;
    int taskCount = Dispatch(outerSetting, innerSetting);
    bool result = Wait(taskCount);
    result = result && task_execution_sequence.size() > 1;
    if (result) {
        result = result && applyExecuted(outerSetting, innerSetting);
        result = result && OuterTaskExecuted(outerSetting);
        result = result && InnerTaskExecuted(innerSetting);
        std::vector<size_t> outerTaskIndex;
        std::vector<std::vector<size_t>> innerTaskIndex;
        GetTaskIndex(outerTaskIndex, innerTaskIndex);
        result = result && IsAscend(outerTaskIndex);
    }
    TestUtils::PublishEvent(g_EVENT_RESP_FIRST, code, std::to_string(result));
}

// level1:serial, async level2:parallel, group
void MainAbility::SerialCase17(int code)
{
    APP_LOGI("-- -- -- -- -- --MainAbility::%{public}s", __FUNCTION__);
    Reset();
    TestSetting outerSetting;
    TestSetting innerSetting;
    outerSetting.dispatcher = TestDispatcher::SERIAL;
    innerSetting.dispatcher = TestDispatcher::PARALLEL;
    innerSetting.create_group = true;
    outerSetting.op = TestOperation::ASYNC;
    innerSetting.op = TestOperation::ASYNC_GROUP;
    int taskCount = Dispatch(outerSetting, innerSetting);
    bool result = Wait(taskCount);
    result = result && task_execution_sequence.size() > 1;
    if (result) {
        result = result && applyExecuted(outerSetting, innerSetting);
        result = result && OuterTaskExecuted(outerSetting);
        result = result && InnerTaskExecuted(innerSetting);
        std::vector<size_t> outerTaskIndex;
        std::vector<std::vector<size_t>> innerTaskIndex;
        GetTaskIndex(outerTaskIndex, innerTaskIndex);
        result = result && IsAscend(outerTaskIndex);
    }
    TestUtils::PublishEvent(g_EVENT_RESP_FIRST, code, std::to_string(result));
}

// level1:serial, async level2:parallel, group wait
void MainAbility::SerialCase18(int code)
{
    APP_LOGI("-- -- -- -- -- --MainAbility::%{public}s", __FUNCTION__);
    Reset();
    TestSetting outerSetting;
    TestSetting innerSetting;
    outerSetting.dispatcher = TestDispatcher::SERIAL;
    innerSetting.dispatcher = TestDispatcher::PARALLEL;
    innerSetting.group_wait = true;
    innerSetting.group_timeout = groupWait;
    innerSetting.create_group = true;
    outerSetting.op = TestOperation::ASYNC;
    innerSetting.op = TestOperation::ASYNC_GROUP;
    int taskCount = Dispatch(outerSetting, innerSetting);
    bool result = Wait(taskCount);
    result = result && task_execution_sequence.size() > 1;
    if (result) {
        result = result && applyExecuted(outerSetting, innerSetting);
        result = result && OuterTaskExecuted(outerSetting);
        result = result && InnerTaskExecuted(innerSetting);
        std::vector<size_t> outerTaskIndex;
        std::vector<std::vector<size_t>> innerTaskIndex;
        GetTaskIndex(outerTaskIndex, innerTaskIndex);
        result = result && IsAscend(outerTaskIndex);
    }
    TestUtils::PublishEvent(g_EVENT_RESP_FIRST, code, std::to_string(result));
}

// level1:serial, async level2:parallel, group notify
void MainAbility::SerialCase19(int code)
{
    APP_LOGI("-- -- -- -- -- --MainAbility::%{public}s", __FUNCTION__);
    Reset();
    TestSetting outerSetting;
    TestSetting innerSetting;
    outerSetting.dispatcher = TestDispatcher::SERIAL;
    innerSetting.dispatcher = TestDispatcher::PARALLEL;
    innerSetting.create_group = true;
    innerSetting.group_notify = true;
    outerSetting.op = TestOperation::ASYNC;
    innerSetting.op = TestOperation::ASYNC_GROUP;
    int taskCount = Dispatch(outerSetting, innerSetting);
    bool result = Wait(taskCount);
    result = result && task_execution_sequence.size() > 1;
    if (result) {
        result = result && applyExecuted(outerSetting, innerSetting);
        result = result && OuterTaskExecuted(outerSetting);
        result = result && InnerTaskExecuted(innerSetting);
        std::vector<size_t> outerTaskIndex;
        std::vector<std::vector<size_t>> innerTaskIndex;
        GetTaskIndex(outerTaskIndex, innerTaskIndex);
        result = result && IsAscend(outerTaskIndex);
        result = result && InnerExecutedAfter(innerGroupNotifyId);
    }
    TestUtils::PublishEvent(g_EVENT_RESP_FIRST, code, std::to_string(result));
}

// level1:serial, async level2:parallel, sync barrier
void MainAbility::SerialCase20(int code)
{
    APP_LOGI("-- -- -- -- -- --MainAbility::%{public}s", __FUNCTION__);
    Reset();
    TestSetting outerSetting;
    TestSetting innerSetting;
    outerSetting.dispatcher = TestDispatcher::SERIAL;
    innerSetting.dispatcher = TestDispatcher::PARALLEL;
    innerSetting.sync_barrier = true;
    innerSetting.create_group = true;
    outerSetting.op = TestOperation::ASYNC;
    innerSetting.op = TestOperation::ASYNC_GROUP;
    int taskCount = Dispatch(outerSetting, innerSetting);
    bool result = Wait(taskCount);
    result = result && task_execution_sequence.size() > 1;
    if (result) {
        result = result && applyExecuted(outerSetting, innerSetting);
        result = result && OuterTaskExecuted(outerSetting);
        result = result && InnerTaskExecuted(innerSetting);
        std::vector<size_t> outerTaskIndex;
        std::vector<std::vector<size_t>> innerTaskIndex;
        GetTaskIndex(outerTaskIndex, innerTaskIndex);
        result = result && IsAscend(outerTaskIndex);
        result = result && InnerExecutedAfter(innerSyncBarrierId);
    }
    TestUtils::PublishEvent(g_EVENT_RESP_FIRST, code, std::to_string(result));
}

// level1:serial, async level2:parallel, async barrier
void MainAbility::SerialCase21(int code)
{
    APP_LOGI("-- -- -- -- -- --MainAbility::%{public}s", __FUNCTION__);
    Reset();
    TestSetting outerSetting;
    TestSetting innerSetting;
    outerSetting.dispatcher = TestDispatcher::SERIAL;
    innerSetting.dispatcher = TestDispatcher::PARALLEL;
    innerSetting.async_barrier = true;
    innerSetting.create_group = true;
    outerSetting.op = TestOperation::ASYNC;
    innerSetting.op = TestOperation::ASYNC_GROUP;
    int taskCount = Dispatch(outerSetting, innerSetting);
    bool result = Wait(taskCount);
    result = result && task_execution_sequence.size() > 1;
    if (result) {
        result = result && applyExecuted(outerSetting, innerSetting);
        result = result && OuterTaskExecuted(outerSetting);
        result = result && InnerTaskExecuted(innerSetting);
        std::vector<size_t> outerTaskIndex;
        std::vector<std::vector<size_t>> innerTaskIndex;
        GetTaskIndex(outerTaskIndex, innerTaskIndex);
        result = result && IsAscend(outerTaskIndex);
        result = result && InnerExecutedAfter(innerAsyncBarrierId);
    }
    TestUtils::PublishEvent(g_EVENT_RESP_FIRST, code, std::to_string(result));
}

// level1:serial, async level2:parallel, apply
void MainAbility::SerialCase22(int code)
{
    APP_LOGI("-- -- -- -- -- --MainAbility::%{public}s", __FUNCTION__);
    Reset();
    TestSetting outerSetting;
    TestSetting innerSetting;
    outerSetting.dispatcher = TestDispatcher::SERIAL;
    innerSetting.dispatcher = TestDispatcher::PARALLEL;
    innerSetting.apply = applyNum;
    outerSetting.op = TestOperation::ASYNC;
    innerSetting.op = TestOperation::APPLY;
    int taskCount = Dispatch(outerSetting, innerSetting);
    bool result = Wait(taskCount);
    result = result && task_execution_sequence.size() > 1;
    if (result) {
        result = result && applyExecuted(outerSetting, innerSetting);
        result = result && OuterTaskExecuted(outerSetting);
        result = result && InnerTaskExecuted(innerSetting);
        std::vector<size_t> outerTaskIndex;
        std::vector<std::vector<size_t>> innerTaskIndex;
        GetTaskIndex(outerTaskIndex, innerTaskIndex);
        result = result && IsAscend(outerTaskIndex);
    }
    TestUtils::PublishEvent(g_EVENT_RESP_FIRST, code, std::to_string(result));
}

// level1:serial, async level2:serial, sync
void MainAbility::SerialCase23(int code)
{
    APP_LOGI("-- -- -- -- -- --MainAbility::%{public}s", __FUNCTION__);
    Reset();
    TestSetting outerSetting;
    TestSetting innerSetting;
    outerSetting.dispatcher = TestDispatcher::SERIAL;
    innerSetting.dispatcher = TestDispatcher::SERIAL;
    outerSetting.op = TestOperation::ASYNC;
    innerSetting.op = TestOperation::SYNC;
    int taskCount = Dispatch(outerSetting, innerSetting);
    bool result = Wait(taskCount);
    result = result && task_execution_sequence.size() > 1;
    if (result) {
        result = result && applyExecuted(outerSetting, innerSetting);
        result = result && OuterTaskExecuted(outerSetting);
        result = result && InnerTaskExecuted(innerSetting);
        std::vector<size_t> outerTaskIndex;
        std::vector<std::vector<size_t>> innerTaskIndex;
        GetTaskIndex(outerTaskIndex, innerTaskIndex);
        result = result && IsAscend(outerTaskIndex);
        for (int i = 0; i < testTaskCount; i++) {
            result = result && IsAscend(innerTaskIndex[i]);
        }
    }
    TestUtils::PublishEvent(g_EVENT_RESP_FIRST, code, std::to_string(result));
}

// level1:serial, async level2:serial, async
void MainAbility::SerialCase24(int code)
{
    APP_LOGI("-- -- -- -- -- --MainAbility::%{public}s", __FUNCTION__);
    Reset();
    TestSetting outerSetting;
    TestSetting innerSetting;
    outerSetting.dispatcher = TestDispatcher::SERIAL;
    innerSetting.dispatcher = TestDispatcher::SERIAL;
    outerSetting.op = TestOperation::ASYNC;
    innerSetting.op = TestOperation::ASYNC;
    int taskCount = Dispatch(outerSetting, innerSetting);
    bool result = Wait(taskCount);
    result = result && task_execution_sequence.size() > 1;
    if (result) {
        result = result && applyExecuted(outerSetting, innerSetting);
        result = result && OuterTaskExecuted(outerSetting);
        result = result && InnerTaskExecuted(innerSetting);
        std::vector<size_t> outerTaskIndex;
        std::vector<std::vector<size_t>> innerTaskIndex;
        GetTaskIndex(outerTaskIndex, innerTaskIndex);
        result = result && IsAscend(outerTaskIndex);
        for (int i = 0; i < testTaskCount; i++) {
            result = result && IsAscend(innerTaskIndex[i]);
        }
    }
    TestUtils::PublishEvent(g_EVENT_RESP_FIRST, code, std::to_string(result));
}

// level1:serial, async level2:serial, delay
void MainAbility::SerialCase25(int code)
{
    APP_LOGI("-- -- -- -- -- --MainAbility::%{public}s", __FUNCTION__);
    Reset();
    TestSetting outerSetting;
    TestSetting innerSetting;
    outerSetting.dispatcher = TestDispatcher::SERIAL;
    innerSetting.dispatcher = TestDispatcher::SERIAL;
    innerSetting.delay = delayMs;
    outerSetting.op = TestOperation::ASYNC;
    innerSetting.op = TestOperation::DELAY;
    int taskCount = Dispatch(outerSetting, innerSetting);
    bool result = Wait(taskCount);
    result = result && task_execution_sequence.size() > 1;
    if (result) {
        result = result && applyExecuted(outerSetting, innerSetting);
        result = result && OuterTaskExecuted(outerSetting);
        result = result && InnerTaskExecuted(innerSetting);
        std::vector<size_t> outerTaskIndex;
        std::vector<std::vector<size_t>> innerTaskIndex;
        GetTaskIndex(outerTaskIndex, innerTaskIndex);
        result = result && IsAscend(outerTaskIndex);
        for (int i = 0; i < testTaskCount; i++) {
            result = result && IsAscend(innerTaskIndex[i]);
        }
    }
    TestUtils::PublishEvent(g_EVENT_RESP_FIRST, code, std::to_string(result));
}

// level1:serial, async level2:serial, apply
void MainAbility::SerialCase26(int code)
{
    APP_LOGI("-- -- -- -- -- --MainAbility::%{public}s", __FUNCTION__);
    Reset();
    TestSetting outerSetting;
    TestSetting innerSetting;
    outerSetting.dispatcher = TestDispatcher::SERIAL;
    innerSetting.dispatcher = TestDispatcher::SERIAL;
    innerSetting.apply = applyNum;
    outerSetting.op = TestOperation::ASYNC;
    innerSetting.op = TestOperation::APPLY;
    int taskCount = Dispatch(outerSetting, innerSetting);
    bool result = Wait(taskCount);
    result = result && task_execution_sequence.size() > 1;
    if (result) {
        result = result && applyExecuted(outerSetting, innerSetting);
        result = result && OuterTaskExecuted(outerSetting);
        result = result && InnerTaskExecuted(innerSetting);
        std::vector<size_t> outerTaskIndex;
        std::vector<std::vector<size_t>> innerTaskIndex;
        GetTaskIndex(outerTaskIndex, innerTaskIndex);
        result = result && IsAscend(outerTaskIndex);
    }
    TestUtils::PublishEvent(g_EVENT_RESP_FIRST, code, std::to_string(result));
}

// level1:serial, delay level2:parallel, sync
void MainAbility::SerialCase27(int code)
{
    APP_LOGI("-- -- -- -- -- --MainAbility::%{public}s", __FUNCTION__);
    Reset();
    TestSetting outerSetting;
    TestSetting innerSetting;
    outerSetting.dispatcher = TestDispatcher::SERIAL;
    innerSetting.dispatcher = TestDispatcher::PARALLEL;
    outerSetting.delay = delayMs;
    outerSetting.op = TestOperation::DELAY;
    innerSetting.op = TestOperation::SYNC;
    int taskCount = Dispatch(outerSetting, innerSetting);
    bool result = Wait(taskCount);
    result = result && task_execution_sequence.size() > 1;
    if (result) {
        result = result && applyExecuted(outerSetting, innerSetting);
        result = result && OuterTaskExecuted(outerSetting);
        result = result && InnerTaskExecuted(innerSetting);
        std::vector<size_t> outerTaskIndex;
        std::vector<std::vector<size_t>> innerTaskIndex;
        GetTaskIndex(outerTaskIndex, innerTaskIndex);
        result = result && IsAscend(outerTaskIndex);
        for (int i = 0; i < testTaskCount; i++) {
            result = result && IsAscend(innerTaskIndex[i]);
        }
    }
    TestUtils::PublishEvent(g_EVENT_RESP_FIRST, code, std::to_string(result));
}

// level1:serial, delay level2:parallel, async
void MainAbility::SerialCase28(int code)
{
    APP_LOGI("-- -- -- -- -- --MainAbility::%{public}s", __FUNCTION__);
    Reset();
    TestSetting outerSetting;
    TestSetting innerSetting;
    outerSetting.dispatcher = TestDispatcher::SERIAL;
    innerSetting.dispatcher = TestDispatcher::PARALLEL;
    outerSetting.delay = delayMs;
    outerSetting.op = TestOperation::DELAY;
    innerSetting.op = TestOperation::ASYNC;
    int taskCount = Dispatch(outerSetting, innerSetting);
    bool result = Wait(taskCount);
    result = result && task_execution_sequence.size() > 1;
    if (result) {
        result = result && applyExecuted(outerSetting, innerSetting);
        result = result && OuterTaskExecuted(outerSetting);
        result = result && InnerTaskExecuted(innerSetting);
        std::vector<size_t> outerTaskIndex;
        std::vector<std::vector<size_t>> innerTaskIndex;
        GetTaskIndex(outerTaskIndex, innerTaskIndex);
        result = result && IsAscend(outerTaskIndex);
    }
    TestUtils::PublishEvent(g_EVENT_RESP_FIRST, code, std::to_string(result));
}

// level1:serial, delay level2:parallel, delay
void MainAbility::SerialCase29(int code)
{
    APP_LOGI("-- -- -- -- -- --MainAbility::%{public}s", __FUNCTION__);
    Reset();
    TestSetting outerSetting;
    TestSetting innerSetting;
    outerSetting.dispatcher = TestDispatcher::SERIAL;
    innerSetting.dispatcher = TestDispatcher::PARALLEL;
    outerSetting.delay = delayMs;
    innerSetting.delay = delayMs;
    outerSetting.op = TestOperation::DELAY;
    innerSetting.op = TestOperation::DELAY;
    int taskCount = Dispatch(outerSetting, innerSetting);
    bool result = Wait(taskCount);
    result = result && task_execution_sequence.size() > 1;
    if (result) {
        result = result && applyExecuted(outerSetting, innerSetting);
        result = result && OuterTaskExecuted(outerSetting);
        result = result && InnerTaskExecuted(innerSetting);
        std::vector<size_t> outerTaskIndex;
        std::vector<std::vector<size_t>> innerTaskIndex;
        GetTaskIndex(outerTaskIndex, innerTaskIndex);
        result = result && IsAscend(outerTaskIndex);
    }
    TestUtils::PublishEvent(g_EVENT_RESP_FIRST, code, std::to_string(result));
}

// level1:serial, delay level2:parallel, group
void MainAbility::SerialCase30(int code)
{
    APP_LOGI("-- -- -- -- -- --MainAbility::%{public}s", __FUNCTION__);
    Reset();
    TestSetting outerSetting;
    TestSetting innerSetting;
    outerSetting.dispatcher = TestDispatcher::SERIAL;
    innerSetting.dispatcher = TestDispatcher::PARALLEL;
    outerSetting.delay = delayMs;
    innerSetting.create_group = true;
    outerSetting.op = TestOperation::DELAY;
    innerSetting.op = TestOperation::ASYNC_GROUP;
    int taskCount = Dispatch(outerSetting, innerSetting);
    bool result = Wait(taskCount);
    result = result && task_execution_sequence.size() > 1;
    if (result) {
        result = result && applyExecuted(outerSetting, innerSetting);
        result = result && OuterTaskExecuted(outerSetting);
        result = result && InnerTaskExecuted(innerSetting);
        std::vector<size_t> outerTaskIndex;
        std::vector<std::vector<size_t>> innerTaskIndex;
        GetTaskIndex(outerTaskIndex, innerTaskIndex);
        result = result && IsAscend(outerTaskIndex);
    }
    TestUtils::PublishEvent(g_EVENT_RESP_FIRST, code, std::to_string(result));
}

// level1:serial, delay level2:parallel, group wait
void MainAbility::SerialCase31(int code)
{
    APP_LOGI("-- -- -- -- -- --MainAbility::%{public}s", __FUNCTION__);
    Reset();
    TestSetting outerSetting;
    TestSetting innerSetting;
    outerSetting.dispatcher = TestDispatcher::SERIAL;
    innerSetting.dispatcher = TestDispatcher::PARALLEL;
    innerSetting.group_wait = true;
    innerSetting.group_timeout = groupWait;
    outerSetting.delay = delayMs;
    innerSetting.create_group = true;
    outerSetting.op = TestOperation::DELAY;
    innerSetting.op = TestOperation::ASYNC_GROUP;
    int taskCount = Dispatch(outerSetting, innerSetting);
    bool result = Wait(taskCount);
    result = result && task_execution_sequence.size() > 1;
    if (result) {
        result = result && applyExecuted(outerSetting, innerSetting);
        result = result && OuterTaskExecuted(outerSetting);
        result = result && InnerTaskExecuted(innerSetting);
        std::vector<size_t> outerTaskIndex;
        std::vector<std::vector<size_t>> innerTaskIndex;
        GetTaskIndex(outerTaskIndex, innerTaskIndex);
        result = result && IsAscend(outerTaskIndex);
    }
    TestUtils::PublishEvent(g_EVENT_RESP_FIRST, code, std::to_string(result));
}

// level1:serial, delay level2:parallel, group notify
void MainAbility::SerialCase32(int code)
{
    APP_LOGI("-- -- -- -- -- --MainAbility::%{public}s", __FUNCTION__);
    Reset();
    TestSetting outerSetting;
    TestSetting innerSetting;
    outerSetting.dispatcher = TestDispatcher::SERIAL;
    innerSetting.dispatcher = TestDispatcher::PARALLEL;
    outerSetting.delay = delayMs;
    innerSetting.create_group = true;
    innerSetting.group_notify = true;
    outerSetting.op = TestOperation::DELAY;
    innerSetting.op = TestOperation::ASYNC_GROUP;
    int taskCount = Dispatch(outerSetting, innerSetting);
    bool result = Wait(taskCount);
    result = result && task_execution_sequence.size() > 1;
    if (result) {
        result = result && applyExecuted(outerSetting, innerSetting);
        result = result && OuterTaskExecuted(outerSetting);
        result = result && InnerTaskExecuted(innerSetting);
        std::vector<size_t> outerTaskIndex;
        std::vector<std::vector<size_t>> innerTaskIndex;
        GetTaskIndex(outerTaskIndex, innerTaskIndex);
        result = result && IsAscend(outerTaskIndex);
        result = result && InnerExecutedAfter(innerGroupNotifyId);
    }
    TestUtils::PublishEvent(g_EVENT_RESP_FIRST, code, std::to_string(result));
}

// level1:serial, delay level2:parallel, sync barrier
void MainAbility::SerialCase33(int code)
{
    APP_LOGI("-- -- -- -- -- --MainAbility::%{public}s", __FUNCTION__);
    Reset();
    TestSetting outerSetting;
    TestSetting innerSetting;
    outerSetting.dispatcher = TestDispatcher::SERIAL;
    innerSetting.dispatcher = TestDispatcher::PARALLEL;
    outerSetting.delay = delayMs;
    innerSetting.sync_barrier = true;
    innerSetting.create_group = true;
    outerSetting.op = TestOperation::DELAY;
    innerSetting.op = TestOperation::ASYNC_GROUP;
    int taskCount = Dispatch(outerSetting, innerSetting);
    bool result = Wait(taskCount);
    result = result && task_execution_sequence.size() > 1;
    if (result) {
        result = result && applyExecuted(outerSetting, innerSetting);
        result = result && OuterTaskExecuted(outerSetting);
        result = result && InnerTaskExecuted(innerSetting);
        std::vector<size_t> outerTaskIndex;
        std::vector<std::vector<size_t>> innerTaskIndex;
        GetTaskIndex(outerTaskIndex, innerTaskIndex);
        result = result && IsAscend(outerTaskIndex);
        result = result && InnerExecutedAfter(innerSyncBarrierId);
    }
    TestUtils::PublishEvent(g_EVENT_RESP_FIRST, code, std::to_string(result));
}

// level1:serial, delay level2:parallel, async barrier
void MainAbility::SerialCase34(int code)
{
    APP_LOGI("-- -- -- -- -- --MainAbility::%{public}s", __FUNCTION__);
    Reset();
    TestSetting outerSetting;
    TestSetting innerSetting;
    outerSetting.dispatcher = TestDispatcher::SERIAL;
    innerSetting.dispatcher = TestDispatcher::PARALLEL;
    outerSetting.delay = delayMs;
    innerSetting.async_barrier = true;
    innerSetting.create_group = true;
    outerSetting.op = TestOperation::DELAY;
    innerSetting.op = TestOperation::ASYNC_GROUP;
    int taskCount = Dispatch(outerSetting, innerSetting);
    bool result = Wait(taskCount);
    result = result && task_execution_sequence.size() > 1;
    if (result) {
        result = result && applyExecuted(outerSetting, innerSetting);
        result = result && OuterTaskExecuted(outerSetting);
        result = result && InnerTaskExecuted(innerSetting);
        std::vector<size_t> outerTaskIndex;
        std::vector<std::vector<size_t>> innerTaskIndex;
        GetTaskIndex(outerTaskIndex, innerTaskIndex);
        result = result && IsAscend(outerTaskIndex);
        result = result && InnerExecutedAfter(innerAsyncBarrierId);
    }
    TestUtils::PublishEvent(g_EVENT_RESP_FIRST, code, std::to_string(result));
}

// level1:serial, delay level2:parallel, apply
void MainAbility::SerialCase35(int code)
{
    APP_LOGI("-- -- -- -- -- --MainAbility::%{public}s", __FUNCTION__);
    Reset();
    TestSetting outerSetting;
    TestSetting innerSetting;
    outerSetting.dispatcher = TestDispatcher::SERIAL;
    innerSetting.dispatcher = TestDispatcher::PARALLEL;
    outerSetting.delay = delayMs;
    innerSetting.apply = applyNum;
    outerSetting.op = TestOperation::DELAY;
    innerSetting.op = TestOperation::APPLY;
    int taskCount = Dispatch(outerSetting, innerSetting);
    bool result = Wait(taskCount);
    result = result && task_execution_sequence.size() > 1;
    if (result) {
        result = result && applyExecuted(outerSetting, innerSetting);
        result = result && OuterTaskExecuted(outerSetting);
        result = result && InnerTaskExecuted(innerSetting);
        std::vector<size_t> outerTaskIndex;
        std::vector<std::vector<size_t>> innerTaskIndex;
        GetTaskIndex(outerTaskIndex, innerTaskIndex);
        result = result && IsAscend(outerTaskIndex);
    }
    TestUtils::PublishEvent(g_EVENT_RESP_FIRST, code, std::to_string(result));
}

// level1:serial, delay level2:serial, sync
void MainAbility::SerialCase36(int code)
{
    APP_LOGI("-- -- -- -- -- --MainAbility::%{public}s", __FUNCTION__);
    Reset();
    TestSetting outerSetting;
    TestSetting innerSetting;
    outerSetting.dispatcher = TestDispatcher::SERIAL;
    innerSetting.dispatcher = TestDispatcher::SERIAL;
    outerSetting.delay = delayMs;
    outerSetting.op = TestOperation::DELAY;
    innerSetting.op = TestOperation::SYNC;
    int taskCount = Dispatch(outerSetting, innerSetting);
    bool result = Wait(taskCount);
    result = result && task_execution_sequence.size() > 1;
    if (result) {
        result = result && applyExecuted(outerSetting, innerSetting);
        result = result && OuterTaskExecuted(outerSetting);
        result = result && InnerTaskExecuted(innerSetting);
        std::vector<size_t> outerTaskIndex;
        std::vector<std::vector<size_t>> innerTaskIndex;
        GetTaskIndex(outerTaskIndex, innerTaskIndex);
        result = result && IsAscend(outerTaskIndex);
        for (int i = 0; i < testTaskCount; i++) {
            result = result && IsAscend(innerTaskIndex[i]);
        }
    }
    TestUtils::PublishEvent(g_EVENT_RESP_FIRST, code, std::to_string(result));
}

// level1:serial, delay level2:serial, async
void MainAbility::SerialCase37(int code)
{
    APP_LOGI("-- -- -- -- -- --MainAbility::%{public}s", __FUNCTION__);
    Reset();
    TestSetting outerSetting;
    TestSetting innerSetting;
    outerSetting.dispatcher = TestDispatcher::SERIAL;
    innerSetting.dispatcher = TestDispatcher::SERIAL;
    outerSetting.delay = delayMs;
    outerSetting.op = TestOperation::DELAY;
    innerSetting.op = TestOperation::ASYNC;
    int taskCount = Dispatch(outerSetting, innerSetting);
    bool result = Wait(taskCount);
    result = result && task_execution_sequence.size() > 1;
    if (result) {
        result = result && applyExecuted(outerSetting, innerSetting);
        result = result && OuterTaskExecuted(outerSetting);
        result = result && InnerTaskExecuted(innerSetting);
        std::vector<size_t> outerTaskIndex;
        std::vector<std::vector<size_t>> innerTaskIndex;
        GetTaskIndex(outerTaskIndex, innerTaskIndex);
        result = result && IsAscend(outerTaskIndex);
        for (int i = 0; i < testTaskCount; i++) {
            result = result && IsAscend(innerTaskIndex[i]);
        }
    }
    TestUtils::PublishEvent(g_EVENT_RESP_FIRST, code, std::to_string(result));
}

// level1:serial, delay level2:serial, delay
void MainAbility::SerialCase38(int code)
{
    APP_LOGI("-- -- -- -- -- --MainAbility::%{public}s", __FUNCTION__);
    Reset();
    TestSetting outerSetting;
    TestSetting innerSetting;
    outerSetting.dispatcher = TestDispatcher::SERIAL;
    innerSetting.dispatcher = TestDispatcher::SERIAL;
    outerSetting.delay = delayMs;
    innerSetting.delay = delayMs;
    outerSetting.op = TestOperation::DELAY;
    innerSetting.op = TestOperation::DELAY;
    int taskCount = Dispatch(outerSetting, innerSetting);
    bool result = Wait(taskCount);
    result = result && task_execution_sequence.size() > 1;
    if (result) {
        result = result && applyExecuted(outerSetting, innerSetting);
        result = result && OuterTaskExecuted(outerSetting);
        result = result && InnerTaskExecuted(innerSetting);
        std::vector<size_t> outerTaskIndex;
        std::vector<std::vector<size_t>> innerTaskIndex;
        GetTaskIndex(outerTaskIndex, innerTaskIndex);
        result = result && IsAscend(outerTaskIndex);
        for (int i = 0; i < testTaskCount; i++) {
            result = result && IsAscend(innerTaskIndex[i]);
        }
    }
    TestUtils::PublishEvent(g_EVENT_RESP_FIRST, code, std::to_string(result));
}

// level1:serial, delay level2:serial, apply
void MainAbility::SerialCase39(int code)
{
    APP_LOGI("-- -- -- -- -- --MainAbility::%{public}s", __FUNCTION__);
    Reset();
    TestSetting outerSetting;
    TestSetting innerSetting;
    outerSetting.dispatcher = TestDispatcher::SERIAL;
    innerSetting.dispatcher = TestDispatcher::SERIAL;
    outerSetting.delay = delayMs;
    innerSetting.apply = applyNum;
    outerSetting.op = TestOperation::DELAY;
    innerSetting.op = TestOperation::APPLY;
    int taskCount = Dispatch(outerSetting, innerSetting);
    bool result = Wait(taskCount);
    result = result && task_execution_sequence.size() > 1;
    if (result) {
        result = result && applyExecuted(outerSetting, innerSetting);
        result = result && OuterTaskExecuted(outerSetting);
        result = result && InnerTaskExecuted(innerSetting);
        std::vector<size_t> outerTaskIndex;
        std::vector<std::vector<size_t>> innerTaskIndex;
        GetTaskIndex(outerTaskIndex, innerTaskIndex);
        result = result && IsAscend(outerTaskIndex);
    }
    TestUtils::PublishEvent(g_EVENT_RESP_FIRST, code, std::to_string(result));
}

// level1:serial, apply level2:parallel, sync
void MainAbility::SerialCase40(int code)
{
    APP_LOGI("-- -- -- -- -- --MainAbility::%{public}s", __FUNCTION__);
    Reset();
    TestSetting outerSetting;
    TestSetting innerSetting;
    outerSetting.dispatcher = TestDispatcher::SERIAL;
    innerSetting.dispatcher = TestDispatcher::PARALLEL;
    outerSetting.apply = applyNum;
    outerSetting.op = TestOperation::APPLY;
    innerSetting.op = TestOperation::SYNC;
    int taskCount = Dispatch(outerSetting, innerSetting);
    bool result = Wait(taskCount);
    result = result && task_execution_sequence.size() > 1;
    if (result) {
        result = result && applyExecuted(outerSetting, innerSetting);
        result = result && OuterTaskExecuted(outerSetting);
        result = result && InnerTaskExecuted(innerSetting);
        std::vector<size_t> outerTaskIndex;
        std::vector<std::vector<size_t>> innerTaskIndex;
        GetTaskIndex(outerTaskIndex, innerTaskIndex);
        for (int i = 0; i < testTaskCount; i++) {
            result = result && IsAscend(innerTaskIndex[i]);
        }
    }
    TestUtils::PublishEvent(g_EVENT_RESP_FIRST, code, std::to_string(result));
}

// level1:serial, apply level2:parallel, async
void MainAbility::SerialCase41(int code)
{
    APP_LOGI("-- -- -- -- -- --MainAbility::%{public}s", __FUNCTION__);
    Reset();
    TestSetting outerSetting;
    TestSetting innerSetting;
    outerSetting.dispatcher = TestDispatcher::SERIAL;
    innerSetting.dispatcher = TestDispatcher::PARALLEL;
    outerSetting.apply = applyNum;
    outerSetting.op = TestOperation::APPLY;
    innerSetting.op = TestOperation::ASYNC;
    int taskCount = Dispatch(outerSetting, innerSetting);
    bool result = Wait(taskCount);
    result = result && task_execution_sequence.size() > 1;
    if (result) {
        result = result && applyExecuted(outerSetting, innerSetting);
        result = result && OuterTaskExecuted(outerSetting);
        result = result && InnerTaskExecuted(innerSetting);
    }
    TestUtils::PublishEvent(g_EVENT_RESP_FIRST, code, std::to_string(result));
}

// level1:serial, apply level2:parallel, delay
void MainAbility::SerialCase42(int code)
{
    APP_LOGI("-- -- -- -- -- --MainAbility::%{public}s", __FUNCTION__);
    Reset();
    TestSetting outerSetting;
    TestSetting innerSetting;
    outerSetting.dispatcher = TestDispatcher::SERIAL;
    innerSetting.dispatcher = TestDispatcher::PARALLEL;
    outerSetting.apply = applyNum;
    innerSetting.delay = delayMs;
    outerSetting.op = TestOperation::APPLY;
    innerSetting.op = TestOperation::DELAY;
    int taskCount = Dispatch(outerSetting, innerSetting);
    bool result = Wait(taskCount);
    result = result && task_execution_sequence.size() > 1;
    if (result) {
        result = result && applyExecuted(outerSetting, innerSetting);
        result = result && OuterTaskExecuted(outerSetting);
        result = result && InnerTaskExecuted(innerSetting);
    }
    TestUtils::PublishEvent(g_EVENT_RESP_FIRST, code, std::to_string(result));
}

// level1:serial, apply level2:parallel, group
void MainAbility::SerialCase43(int code)
{
    APP_LOGI("-- -- -- -- -- --MainAbility::%{public}s", __FUNCTION__);
    Reset();
    TestSetting outerSetting;
    TestSetting innerSetting;
    outerSetting.dispatcher = TestDispatcher::SERIAL;
    innerSetting.dispatcher = TestDispatcher::PARALLEL;
    outerSetting.apply = applyNum;
    innerSetting.create_group = true;
    outerSetting.op = TestOperation::APPLY;
    innerSetting.op = TestOperation::ASYNC_GROUP;
    int taskCount = Dispatch(outerSetting, innerSetting);
    bool result = Wait(taskCount);
    result = result && task_execution_sequence.size() > 1;
    if (result) {
        result = result && applyExecuted(outerSetting, innerSetting);
        result = result && OuterTaskExecuted(outerSetting);
        result = result && InnerTaskExecuted(innerSetting);
    }
    TestUtils::PublishEvent(g_EVENT_RESP_FIRST, code, std::to_string(result));
}

// level1:serial, apply level2:parallel, group wait
void MainAbility::SerialCase44(int code)
{
    APP_LOGI("-- -- -- -- -- --MainAbility::%{public}s", __FUNCTION__);
    Reset();
    TestSetting outerSetting;
    TestSetting innerSetting;
    outerSetting.dispatcher = TestDispatcher::SERIAL;
    innerSetting.dispatcher = TestDispatcher::PARALLEL;
    innerSetting.group_wait = true;
    innerSetting.group_timeout = groupWait;
    outerSetting.apply = applyNum;
    innerSetting.create_group = true;
    outerSetting.op = TestOperation::APPLY;
    innerSetting.op = TestOperation::ASYNC_GROUP;
    int taskCount = Dispatch(outerSetting, innerSetting);
    bool result = Wait(taskCount);
    result = result && task_execution_sequence.size() > 1;
    if (result) {
        result = result && applyExecuted(outerSetting, innerSetting);
        result = result && OuterTaskExecuted(outerSetting);
        result = result && InnerTaskExecuted(innerSetting);
    }
    TestUtils::PublishEvent(g_EVENT_RESP_FIRST, code, std::to_string(result));
}

// level1:serial, apply level2:parallel, group notify
void MainAbility::SerialCase45(int code)
{
    APP_LOGI("-- -- -- -- -- --MainAbility::%{public}s", __FUNCTION__);
    Reset();
    TestSetting outerSetting;
    TestSetting innerSetting;
    outerSetting.dispatcher = TestDispatcher::SERIAL;
    innerSetting.dispatcher = TestDispatcher::PARALLEL;
    outerSetting.apply = applyNum;
    innerSetting.create_group = true;
    innerSetting.group_notify = true;
    outerSetting.op = TestOperation::APPLY;
    innerSetting.op = TestOperation::ASYNC_GROUP;
    int taskCount = Dispatch(outerSetting, innerSetting);
    bool result = Wait(taskCount);
    result = result && task_execution_sequence.size() > 1;
    if (result) {
        result = result && applyExecuted(outerSetting, innerSetting);
        result = result && OuterTaskExecuted(outerSetting);
        result = result && InnerTaskExecuted(innerSetting);
        result = result && InnerExecutedAfter(innerGroupNotifyId);
    }
    TestUtils::PublishEvent(g_EVENT_RESP_FIRST, code, std::to_string(result));
}

// level1:serial, apply level2:parallel, sync barrier
void MainAbility::SerialCase46(int code)
{
    APP_LOGI("-- -- -- -- -- --MainAbility::%{public}s", __FUNCTION__);
    Reset();
    TestSetting outerSetting;
    TestSetting innerSetting;
    outerSetting.dispatcher = TestDispatcher::SERIAL;
    innerSetting.dispatcher = TestDispatcher::PARALLEL;
    outerSetting.apply = applyNum;
    innerSetting.sync_barrier = true;
    innerSetting.create_group = true;
    outerSetting.op = TestOperation::APPLY;
    innerSetting.op = TestOperation::ASYNC_GROUP;
    int taskCount = Dispatch(outerSetting, innerSetting);
    bool result = Wait(taskCount);
    result = result && task_execution_sequence.size() > 1;
    if (result) {
        result = result && applyExecuted(outerSetting, innerSetting);
        result = result && OuterTaskExecuted(outerSetting);
        result = result && InnerTaskExecuted(innerSetting);
        result = result && InnerExecutedAfter(innerSyncBarrierId);
    }
    TestUtils::PublishEvent(g_EVENT_RESP_FIRST, code, std::to_string(result));
}

// level1:serial, apply level2:parallel, async barrier
void MainAbility::SerialCase47(int code)
{
    APP_LOGI("-- -- -- -- -- --MainAbility::%{public}s", __FUNCTION__);
    Reset();
    TestSetting outerSetting;
    TestSetting innerSetting;
    outerSetting.dispatcher = TestDispatcher::SERIAL;
    innerSetting.dispatcher = TestDispatcher::PARALLEL;
    outerSetting.apply = applyNum;
    innerSetting.async_barrier = true;
    innerSetting.create_group = true;
    outerSetting.op = TestOperation::APPLY;
    innerSetting.op = TestOperation::ASYNC_GROUP;
    int taskCount = Dispatch(outerSetting, innerSetting);
    bool result = Wait(taskCount);
    result = result && task_execution_sequence.size() > 1;
    if (result) {
        result = result && applyExecuted(outerSetting, innerSetting);
        result = result && OuterTaskExecuted(outerSetting);
        result = result && InnerTaskExecuted(innerSetting);
        result = result && InnerExecutedAfter(innerAsyncBarrierId);
    }
    TestUtils::PublishEvent(g_EVENT_RESP_FIRST, code, std::to_string(result));
}

// level1:serial, apply level2:parallel, apply
void MainAbility::SerialCase48(int code)
{
    APP_LOGI("-- -- -- -- -- --MainAbility::%{public}s", __FUNCTION__);
    Reset();
    TestSetting outerSetting;
    TestSetting innerSetting;
    outerSetting.dispatcher = TestDispatcher::SERIAL;
    innerSetting.dispatcher = TestDispatcher::PARALLEL;
    outerSetting.apply = applyNum;
    innerSetting.apply = applyNum;
    outerSetting.op = TestOperation::APPLY;
    innerSetting.op = TestOperation::APPLY;
    int taskCount = Dispatch(outerSetting, innerSetting);
    bool result = Wait(taskCount);
    result = result && task_execution_sequence.size() > 1;
    if (result) {
        result = result && applyExecuted(outerSetting, innerSetting);
        result = result && OuterTaskExecuted(outerSetting);
        result = result && InnerTaskExecuted(innerSetting);
    }
    TestUtils::PublishEvent(g_EVENT_RESP_FIRST, code, std::to_string(result));
}

// level1:serial, apply level2:serial, sync
void MainAbility::SerialCase49(int code)
{
    APP_LOGI("-- -- -- -- -- --MainAbility::%{public}s", __FUNCTION__);
    Reset();
    TestSetting outerSetting;
    TestSetting innerSetting;
    outerSetting.dispatcher = TestDispatcher::SERIAL;
    innerSetting.dispatcher = TestDispatcher::SERIAL;
    outerSetting.apply = applyNum;
    outerSetting.op = TestOperation::APPLY;
    innerSetting.op = TestOperation::SYNC;
    int taskCount = Dispatch(outerSetting, innerSetting);
    bool result = Wait(taskCount);
    result = result && task_execution_sequence.size() > 1;
    if (result) {
        result = result && applyExecuted(outerSetting, innerSetting);
        result = result && OuterTaskExecuted(outerSetting);
        result = result && InnerTaskExecuted(innerSetting);
        std::vector<size_t> outerTaskIndex;
        std::vector<std::vector<size_t>> innerTaskIndex;
        GetTaskIndex(outerTaskIndex, innerTaskIndex);
        for (int i = 0; i < testTaskCount; i++) {
            result = result && IsAscend(innerTaskIndex[i]);
        }
    }
    TestUtils::PublishEvent(g_EVENT_RESP_FIRST, code, std::to_string(result));
}

// level1:serial, apply level2:serial, async
void MainAbility::SerialCase50(int code)
{
    APP_LOGI("-- -- -- -- -- --MainAbility::%{public}s", __FUNCTION__);
    Reset();
    TestSetting outerSetting;
    TestSetting innerSetting;
    outerSetting.dispatcher = TestDispatcher::SERIAL;
    innerSetting.dispatcher = TestDispatcher::SERIAL;
    outerSetting.apply = applyNum;
    outerSetting.op = TestOperation::APPLY;
    innerSetting.op = TestOperation::ASYNC;
    int taskCount = Dispatch(outerSetting, innerSetting);
    bool result = Wait(taskCount);
    result = result && task_execution_sequence.size() > 1;
    if (result) {
        result = result && applyExecuted(outerSetting, innerSetting);
        result = result && OuterTaskExecuted(outerSetting);
        result = result && InnerTaskExecuted(innerSetting);
        std::vector<size_t> outerTaskIndex;
        std::vector<std::vector<size_t>> innerTaskIndex;
        GetTaskIndex(outerTaskIndex, innerTaskIndex);
        for (int i = 0; i < testTaskCount; i++) {
            result = result && IsAscend(innerTaskIndex[i]);
        }
    }
    TestUtils::PublishEvent(g_EVENT_RESP_FIRST, code, std::to_string(result));
}

// level1:serial, apply level2:serial, delay
void MainAbility::SerialCase51(int code)
{
    APP_LOGI("-- -- -- -- -- --MainAbility::%{public}s", __FUNCTION__);
    Reset();
    TestSetting outerSetting;
    TestSetting innerSetting;
    outerSetting.dispatcher = TestDispatcher::SERIAL;
    innerSetting.dispatcher = TestDispatcher::SERIAL;
    outerSetting.apply = applyNum;
    innerSetting.delay = delayMs;
    outerSetting.op = TestOperation::APPLY;
    innerSetting.op = TestOperation::DELAY;
    int taskCount = Dispatch(outerSetting, innerSetting);
    bool result = Wait(taskCount);
    result = result && task_execution_sequence.size() > 1;
    if (result) {
        result = result && applyExecuted(outerSetting, innerSetting);
        result = result && OuterTaskExecuted(outerSetting);
        result = result && InnerTaskExecuted(innerSetting);
        std::vector<size_t> outerTaskIndex;
        std::vector<std::vector<size_t>> innerTaskIndex;
        GetTaskIndex(outerTaskIndex, innerTaskIndex);
        for (int i = 0; i < testTaskCount; i++) {
            result = result && IsAscend(innerTaskIndex[i]);
        }
    }
    TestUtils::PublishEvent(g_EVENT_RESP_FIRST, code, std::to_string(result));
}

// level1:serial, apply level2:serial, apply
void MainAbility::SerialCase52(int code)
{
    APP_LOGI("-- -- -- -- -- --MainAbility::%{public}s", __FUNCTION__);
    Reset();
    TestSetting outerSetting;
    TestSetting innerSetting;
    outerSetting.dispatcher = TestDispatcher::SERIAL;
    innerSetting.dispatcher = TestDispatcher::SERIAL;
    outerSetting.apply = applyNum;
    innerSetting.apply = applyNum;
    outerSetting.op = TestOperation::APPLY;
    innerSetting.op = TestOperation::APPLY;
    int taskCount = Dispatch(outerSetting, innerSetting);
    bool result = Wait(taskCount);
    result = result && task_execution_sequence.size() > 1;
    if (result) {
        result = result && applyExecuted(outerSetting, innerSetting);
        result = result && OuterTaskExecuted(outerSetting);
        result = result && InnerTaskExecuted(innerSetting);
    }
    TestUtils::PublishEvent(g_EVENT_RESP_FIRST, code, std::to_string(result));
}

// level1:spec, sync level2:parallel, sync
void MainAbility::SpecCase1(int code)
{
    APP_LOGI("-- -- -- -- -- --MainAbility::%{public}s", __FUNCTION__);
    Reset();
    TestSetting outerSetting;
    TestSetting innerSetting;
    outerSetting.dispatcher = TestDispatcher::MAIN;
    innerSetting.dispatcher = TestDispatcher::PARALLEL;
    outerSetting.op = TestOperation::SYNC;
    innerSetting.op = TestOperation::SYNC;
    int taskCount = Dispatch(outerSetting, innerSetting);
    bool result = Wait(taskCount);
    result = result && task_execution_sequence.size() > 1;
    if (result) {
        result = result && applyExecuted(outerSetting, innerSetting);
        result = result && OuterTaskExecuted(outerSetting);
        result = result && InnerTaskExecuted(innerSetting);
        std::vector<size_t> outerTaskIndex;
        std::vector<std::vector<size_t>> innerTaskIndex;
        GetTaskIndex(outerTaskIndex, innerTaskIndex);
        result = result && IsAscend(outerTaskIndex);
        for (int i = 0; i < testTaskCount; i++) {
            result = result && IsAscend(innerTaskIndex[i]);
        }
    }
    TestUtils::PublishEvent(g_EVENT_RESP_FIRST, code, std::to_string(result));
}

// level1:spec, sync level2:parallel, async
void MainAbility::SpecCase2(int code)
{
    APP_LOGI("-- -- -- -- -- --MainAbility::%{public}s", __FUNCTION__);
    Reset();
    TestSetting outerSetting;
    TestSetting innerSetting;
    outerSetting.dispatcher = TestDispatcher::MAIN;
    innerSetting.dispatcher = TestDispatcher::PARALLEL;
    outerSetting.op = TestOperation::SYNC;
    innerSetting.op = TestOperation::ASYNC;
    int taskCount = Dispatch(outerSetting, innerSetting);
    bool result = Wait(taskCount);
    result = result && task_execution_sequence.size() > 1;
    if (result) {
        result = result && applyExecuted(outerSetting, innerSetting);
        result = result && OuterTaskExecuted(outerSetting);
        result = result && InnerTaskExecuted(innerSetting);
        std::vector<size_t> outerTaskIndex;
        std::vector<std::vector<size_t>> innerTaskIndex;
        GetTaskIndex(outerTaskIndex, innerTaskIndex);
        result = result && IsAscend(outerTaskIndex);
    }
    TestUtils::PublishEvent(g_EVENT_RESP_FIRST, code, std::to_string(result));
}

// level1:spec, sync level2:parallel, delay
void MainAbility::SpecCase3(int code)
{
    APP_LOGI("-- -- -- -- -- --MainAbility::%{public}s", __FUNCTION__);
    Reset();
    TestSetting outerSetting;
    TestSetting innerSetting;
    outerSetting.dispatcher = TestDispatcher::MAIN;
    innerSetting.dispatcher = TestDispatcher::PARALLEL;
    innerSetting.delay = delayMs;
    outerSetting.op = TestOperation::SYNC;
    innerSetting.op = TestOperation::DELAY;
    int taskCount = Dispatch(outerSetting, innerSetting);
    bool result = Wait(taskCount);
    result = result && task_execution_sequence.size() > 1;
    if (result) {
        result = result && applyExecuted(outerSetting, innerSetting);
        result = result && OuterTaskExecuted(outerSetting);
        result = result && InnerTaskExecuted(innerSetting);
        std::vector<size_t> outerTaskIndex;
        std::vector<std::vector<size_t>> innerTaskIndex;
        GetTaskIndex(outerTaskIndex, innerTaskIndex);
        result = result && IsAscend(outerTaskIndex);
    }
    TestUtils::PublishEvent(g_EVENT_RESP_FIRST, code, std::to_string(result));
}

// level1:spec, sync level2:parallel, group
void MainAbility::SpecCase4(int code)
{
    APP_LOGI("-- -- -- -- -- --MainAbility::%{public}s", __FUNCTION__);
    Reset();
    TestSetting outerSetting;
    TestSetting innerSetting;
    outerSetting.dispatcher = TestDispatcher::MAIN;
    innerSetting.dispatcher = TestDispatcher::PARALLEL;
    innerSetting.create_group = true;
    outerSetting.op = TestOperation::SYNC;
    innerSetting.op = TestOperation::ASYNC_GROUP;
    int taskCount = Dispatch(outerSetting, innerSetting);
    bool result = Wait(taskCount);
    result = result && task_execution_sequence.size() > 1;
    if (result) {
        result = result && applyExecuted(outerSetting, innerSetting);
        result = result && OuterTaskExecuted(outerSetting);
        result = result && InnerTaskExecuted(innerSetting);
        std::vector<size_t> outerTaskIndex;
        std::vector<std::vector<size_t>> innerTaskIndex;
        GetTaskIndex(outerTaskIndex, innerTaskIndex);
        result = result && IsAscend(outerTaskIndex);
    }
    TestUtils::PublishEvent(g_EVENT_RESP_FIRST, code, std::to_string(result));
}

// level1:spec, sync level2:parallel, group wait
void MainAbility::SpecCase5(int code)
{
    APP_LOGI("-- -- -- -- -- --MainAbility::%{public}s", __FUNCTION__);
    Reset();
    TestSetting outerSetting;
    TestSetting innerSetting;
    outerSetting.dispatcher = TestDispatcher::MAIN;
    innerSetting.dispatcher = TestDispatcher::PARALLEL;
    innerSetting.group_wait = true;
    innerSetting.group_timeout = groupWait;
    innerSetting.create_group = true;
    outerSetting.op = TestOperation::SYNC;
    innerSetting.op = TestOperation::ASYNC_GROUP;
    int taskCount = Dispatch(outerSetting, innerSetting);
    bool result = Wait(taskCount);
    result = result && task_execution_sequence.size() > 1;
    if (result) {
        result = result && applyExecuted(outerSetting, innerSetting);
        result = result && OuterTaskExecuted(outerSetting);
        result = result && InnerTaskExecuted(innerSetting);
        std::vector<size_t> outerTaskIndex;
        std::vector<std::vector<size_t>> innerTaskIndex;
        GetTaskIndex(outerTaskIndex, innerTaskIndex);
        result = result && IsAscend(outerTaskIndex);
    }
    TestUtils::PublishEvent(g_EVENT_RESP_FIRST, code, std::to_string(result));
}

// level1:spec, sync level2:parallel, group notify
void MainAbility::SpecCase6(int code)
{
    APP_LOGI("-- -- -- -- -- --MainAbility::%{public}s", __FUNCTION__);
    Reset();
    TestSetting outerSetting;
    TestSetting innerSetting;
    outerSetting.dispatcher = TestDispatcher::MAIN;
    innerSetting.dispatcher = TestDispatcher::PARALLEL;
    innerSetting.create_group = true;
    innerSetting.group_notify = true;
    outerSetting.op = TestOperation::SYNC;
    innerSetting.op = TestOperation::ASYNC_GROUP;
    int taskCount = Dispatch(outerSetting, innerSetting);
    bool result = Wait(taskCount);
    result = result && task_execution_sequence.size() > 1;
    if (result) {
        result = result && applyExecuted(outerSetting, innerSetting);
        result = result && OuterTaskExecuted(outerSetting);
        result = result && InnerTaskExecuted(innerSetting);
        std::vector<size_t> outerTaskIndex;
        std::vector<std::vector<size_t>> innerTaskIndex;
        GetTaskIndex(outerTaskIndex, innerTaskIndex);
        result = result && IsAscend(outerTaskIndex);
        result = result && InnerExecutedAfter(innerGroupNotifyId);
    }
    TestUtils::PublishEvent(g_EVENT_RESP_FIRST, code, std::to_string(result));
}

// level1:spec, sync level2:parallel, sync barrier
void MainAbility::SpecCase7(int code)
{
    APP_LOGI("-- -- -- -- -- --MainAbility::%{public}s", __FUNCTION__);
    Reset();
    TestSetting outerSetting;
    TestSetting innerSetting;
    outerSetting.dispatcher = TestDispatcher::MAIN;
    innerSetting.dispatcher = TestDispatcher::PARALLEL;
    innerSetting.sync_barrier = true;
    innerSetting.create_group = true;
    outerSetting.op = TestOperation::SYNC;
    innerSetting.op = TestOperation::ASYNC_GROUP;
    int taskCount = Dispatch(outerSetting, innerSetting);
    bool result = Wait(taskCount);
    result = result && task_execution_sequence.size() > 1;
    if (result) {
        result = result && applyExecuted(outerSetting, innerSetting);
        result = result && OuterTaskExecuted(outerSetting);
        result = result && InnerTaskExecuted(innerSetting);
        std::vector<size_t> outerTaskIndex;
        std::vector<std::vector<size_t>> innerTaskIndex;
        GetTaskIndex(outerTaskIndex, innerTaskIndex);
        result = result && IsAscend(outerTaskIndex);
        result = result && InnerExecutedAfter(innerSyncBarrierId);
    }
    TestUtils::PublishEvent(g_EVENT_RESP_FIRST, code, std::to_string(result));
}

// level1:spec, sync level2:parallel, async barrier
void MainAbility::SpecCase8(int code)
{
    APP_LOGI("-- -- -- -- -- --MainAbility::%{public}s", __FUNCTION__);
    Reset();
    TestSetting outerSetting;
    TestSetting innerSetting;
    outerSetting.dispatcher = TestDispatcher::MAIN;
    innerSetting.dispatcher = TestDispatcher::PARALLEL;
    innerSetting.async_barrier = true;
    innerSetting.create_group = true;
    outerSetting.op = TestOperation::SYNC;
    innerSetting.op = TestOperation::ASYNC_GROUP;
    int taskCount = Dispatch(outerSetting, innerSetting);
    bool result = Wait(taskCount);
    result = result && task_execution_sequence.size() > 1;
    if (result) {
        result = result && applyExecuted(outerSetting, innerSetting);
        result = result && OuterTaskExecuted(outerSetting);
        result = result && InnerTaskExecuted(innerSetting);
        std::vector<size_t> outerTaskIndex;
        std::vector<std::vector<size_t>> innerTaskIndex;
        GetTaskIndex(outerTaskIndex, innerTaskIndex);
        result = result && IsAscend(outerTaskIndex);
        result = result && InnerExecutedAfter(innerAsyncBarrierId);
    }
    TestUtils::PublishEvent(g_EVENT_RESP_FIRST, code, std::to_string(result));
}

// level1:spec, sync level2:parallel, apply
void MainAbility::SpecCase9(int code)
{
    APP_LOGI("-- -- -- -- -- --MainAbility::%{public}s", __FUNCTION__);
    Reset();
    TestSetting outerSetting;
    TestSetting innerSetting;
    outerSetting.dispatcher = TestDispatcher::MAIN;
    innerSetting.dispatcher = TestDispatcher::PARALLEL;
    innerSetting.apply = applyNum;
    outerSetting.op = TestOperation::SYNC;
    innerSetting.op = TestOperation::APPLY;
    int taskCount = Dispatch(outerSetting, innerSetting);
    bool result = Wait(taskCount);
    result = result && task_execution_sequence.size() > 1;
    if (result) {
        result = result && applyExecuted(outerSetting, innerSetting);
        result = result && OuterTaskExecuted(outerSetting);
        result = result && InnerTaskExecuted(innerSetting);
        std::vector<size_t> outerTaskIndex;
        std::vector<std::vector<size_t>> innerTaskIndex;
        GetTaskIndex(outerTaskIndex, innerTaskIndex);
        result = result && IsAscend(outerTaskIndex);
    }
    TestUtils::PublishEvent(g_EVENT_RESP_FIRST, code, std::to_string(result));
}

// level1:spec, sync level2:serial, sync
void MainAbility::SpecCase10(int code)
{
    APP_LOGI("-- -- -- -- -- --MainAbility::%{public}s", __FUNCTION__);
    Reset();
    TestSetting outerSetting;
    TestSetting innerSetting;
    outerSetting.dispatcher = TestDispatcher::MAIN;
    innerSetting.dispatcher = TestDispatcher::SERIAL;
    outerSetting.op = TestOperation::SYNC;
    innerSetting.op = TestOperation::SYNC;
    int taskCount = Dispatch(outerSetting, innerSetting);
    bool result = Wait(taskCount);
    result = result && task_execution_sequence.size() > 1;
    if (result) {
        result = result && applyExecuted(outerSetting, innerSetting);
        result = result && OuterTaskExecuted(outerSetting);
        result = result && InnerTaskExecuted(innerSetting);
        std::vector<size_t> outerTaskIndex;
        std::vector<std::vector<size_t>> innerTaskIndex;
        GetTaskIndex(outerTaskIndex, innerTaskIndex);
        result = result && IsAscend(outerTaskIndex);
        for (int i = 0; i < testTaskCount; i++) {
            result = result && IsAscend(innerTaskIndex[i]);
        }
    }
    TestUtils::PublishEvent(g_EVENT_RESP_FIRST, code, std::to_string(result));
}

// level1:spec, sync level2:serial, async
void MainAbility::SpecCase11(int code)
{
    APP_LOGI("-- -- -- -- -- --MainAbility::%{public}s", __FUNCTION__);
    Reset();
    TestSetting outerSetting;
    TestSetting innerSetting;
    outerSetting.dispatcher = TestDispatcher::MAIN;
    innerSetting.dispatcher = TestDispatcher::SERIAL;
    outerSetting.op = TestOperation::SYNC;
    innerSetting.op = TestOperation::ASYNC;
    int taskCount = Dispatch(outerSetting, innerSetting);
    bool result = Wait(taskCount);
    result = result && task_execution_sequence.size() > 1;
    if (result) {
        result = result && applyExecuted(outerSetting, innerSetting);
        result = result && OuterTaskExecuted(outerSetting);
        result = result && InnerTaskExecuted(innerSetting);
        std::vector<size_t> outerTaskIndex;
        std::vector<std::vector<size_t>> innerTaskIndex;
        GetTaskIndex(outerTaskIndex, innerTaskIndex);
        result = result && IsAscend(outerTaskIndex);
        for (int i = 0; i < testTaskCount; i++) {
            result = result && IsAscend(innerTaskIndex[i]);
        }
    }
    TestUtils::PublishEvent(g_EVENT_RESP_FIRST, code, std::to_string(result));
}

// level1:spec, sync level2:serial, delay
void MainAbility::SpecCase12(int code)
{
    APP_LOGI("-- -- -- -- -- --MainAbility::%{public}s", __FUNCTION__);
    Reset();
    TestSetting outerSetting;
    TestSetting innerSetting;
    outerSetting.dispatcher = TestDispatcher::MAIN;
    innerSetting.dispatcher = TestDispatcher::SERIAL;
    innerSetting.delay = delayMs;
    outerSetting.op = TestOperation::SYNC;
    innerSetting.op = TestOperation::DELAY;
    int taskCount = Dispatch(outerSetting, innerSetting);
    bool result = Wait(taskCount);
    result = result && task_execution_sequence.size() > 1;
    if (result) {
        result = result && applyExecuted(outerSetting, innerSetting);
        result = result && OuterTaskExecuted(outerSetting);
        result = result && InnerTaskExecuted(innerSetting);
        std::vector<size_t> outerTaskIndex;
        std::vector<std::vector<size_t>> innerTaskIndex;
        GetTaskIndex(outerTaskIndex, innerTaskIndex);
        result = result && IsAscend(outerTaskIndex);
        for (int i = 0; i < testTaskCount; i++) {
            result = result && IsAscend(innerTaskIndex[i]);
        }
    }
    TestUtils::PublishEvent(g_EVENT_RESP_FIRST, code, std::to_string(result));
}

// level1:spec, sync level2:serial, apply
void MainAbility::SpecCase13(int code)
{
    APP_LOGI("-- -- -- -- -- --MainAbility::%{public}s", __FUNCTION__);
    Reset();
    TestSetting outerSetting;
    TestSetting innerSetting;
    outerSetting.dispatcher = TestDispatcher::MAIN;
    innerSetting.dispatcher = TestDispatcher::SERIAL;
    innerSetting.apply = applyNum;
    outerSetting.op = TestOperation::SYNC;
    innerSetting.op = TestOperation::APPLY;
    int taskCount = Dispatch(outerSetting, innerSetting);
    bool result = Wait(taskCount);
    result = result && task_execution_sequence.size() > 1;
    if (result) {
        result = result && applyExecuted(outerSetting, innerSetting);
        result = result && OuterTaskExecuted(outerSetting);
        result = result && InnerTaskExecuted(innerSetting);
        std::vector<size_t> outerTaskIndex;
        std::vector<std::vector<size_t>> innerTaskIndex;
        GetTaskIndex(outerTaskIndex, innerTaskIndex);
        result = result && IsAscend(outerTaskIndex);
    }
    TestUtils::PublishEvent(g_EVENT_RESP_FIRST, code, std::to_string(result));
}

// level1:spec, async level2:parallel, sync
void MainAbility::SpecCase14(int code)
{
    APP_LOGI("-- -- -- -- -- --MainAbility::%{public}s", __FUNCTION__);
    Reset();
    TestSetting outerSetting;
    TestSetting innerSetting;
    outerSetting.dispatcher = TestDispatcher::MAIN;
    innerSetting.dispatcher = TestDispatcher::PARALLEL;
    outerSetting.op = TestOperation::ASYNC;
    innerSetting.op = TestOperation::SYNC;
    int taskCount = Dispatch(outerSetting, innerSetting);
    bool result = Wait(taskCount);
    result = result && task_execution_sequence.size() > 1;
    if (result) {
        result = result && applyExecuted(outerSetting, innerSetting);
        result = result && OuterTaskExecuted(outerSetting);
        result = result && InnerTaskExecuted(innerSetting);
        std::vector<size_t> outerTaskIndex;
        std::vector<std::vector<size_t>> innerTaskIndex;
        GetTaskIndex(outerTaskIndex, innerTaskIndex);
        for (int i = 0; i < testTaskCount; i++) {
            result = result && IsAscend(innerTaskIndex[i]);
        }
    }
    TestUtils::PublishEvent(g_EVENT_RESP_FIRST, code, std::to_string(result));
}

// level1:spec, async level2:parallel, async
void MainAbility::SpecCase15(int code)
{
    APP_LOGI("-- -- -- -- -- --MainAbility::%{public}s", __FUNCTION__);
    Reset();
    TestSetting outerSetting;
    TestSetting innerSetting;
    outerSetting.dispatcher = TestDispatcher::MAIN;
    innerSetting.dispatcher = TestDispatcher::PARALLEL;
    outerSetting.op = TestOperation::ASYNC;
    innerSetting.op = TestOperation::ASYNC;
    int taskCount = Dispatch(outerSetting, innerSetting);
    bool result = Wait(taskCount);
    result = result && task_execution_sequence.size() > 1;
    if (result) {
        result = result && applyExecuted(outerSetting, innerSetting);
        result = result && OuterTaskExecuted(outerSetting);
        result = result && InnerTaskExecuted(innerSetting);
    }
    TestUtils::PublishEvent(g_EVENT_RESP_FIRST, code, std::to_string(result));
}

// level1:spec, async level2:parallel, delay
void MainAbility::SpecCase16(int code)
{
    APP_LOGI("-- -- -- -- -- --MainAbility::%{public}s", __FUNCTION__);
    Reset();
    TestSetting outerSetting;
    TestSetting innerSetting;
    outerSetting.dispatcher = TestDispatcher::MAIN;
    innerSetting.dispatcher = TestDispatcher::PARALLEL;
    innerSetting.delay = delayMs;
    outerSetting.op = TestOperation::ASYNC;
    innerSetting.op = TestOperation::DELAY;
    int taskCount = Dispatch(outerSetting, innerSetting);
    bool result = Wait(taskCount);
    result = result && task_execution_sequence.size() > 1;
    if (result) {
        result = result && applyExecuted(outerSetting, innerSetting);
        result = result && OuterTaskExecuted(outerSetting);
        result = result && InnerTaskExecuted(innerSetting);
    }
    TestUtils::PublishEvent(g_EVENT_RESP_FIRST, code, std::to_string(result));
}

// level1:spec, async level2:parallel, group
void MainAbility::SpecCase17(int code)
{
    APP_LOGI("-- -- -- -- -- --MainAbility::%{public}s", __FUNCTION__);
    Reset();
    TestSetting outerSetting;
    TestSetting innerSetting;
    outerSetting.dispatcher = TestDispatcher::MAIN;
    innerSetting.dispatcher = TestDispatcher::PARALLEL;
    innerSetting.create_group = true;
    outerSetting.op = TestOperation::ASYNC;
    innerSetting.op = TestOperation::ASYNC_GROUP;
    int taskCount = Dispatch(outerSetting, innerSetting);
    bool result = Wait(taskCount);
    result = result && task_execution_sequence.size() > 1;
    if (result) {
        result = result && applyExecuted(outerSetting, innerSetting);
        result = result && OuterTaskExecuted(outerSetting);
        result = result && InnerTaskExecuted(innerSetting);
    }
    TestUtils::PublishEvent(g_EVENT_RESP_FIRST, code, std::to_string(result));
}

// level1:spec, async level2:parallel, group wait
void MainAbility::SpecCase18(int code)
{
    APP_LOGI("-- -- -- -- -- --MainAbility::%{public}s", __FUNCTION__);
    Reset();
    TestSetting outerSetting;
    TestSetting innerSetting;
    outerSetting.dispatcher = TestDispatcher::MAIN;
    innerSetting.dispatcher = TestDispatcher::PARALLEL;
    innerSetting.group_wait = true;
    innerSetting.group_timeout = groupWait;
    innerSetting.create_group = true;
    outerSetting.op = TestOperation::ASYNC;
    innerSetting.op = TestOperation::ASYNC_GROUP;
    int taskCount = Dispatch(outerSetting, innerSetting);
    bool result = Wait(taskCount);
    result = result && task_execution_sequence.size() > 1;
    if (result) {
        result = result && applyExecuted(outerSetting, innerSetting);
        result = result && OuterTaskExecuted(outerSetting);
        result = result && InnerTaskExecuted(innerSetting);
    }
    TestUtils::PublishEvent(g_EVENT_RESP_FIRST, code, std::to_string(result));
}

// level1:spec, async level2:parallel, group notify
void MainAbility::SpecCase19(int code)
{
    APP_LOGI("-- -- -- -- -- --MainAbility::%{public}s", __FUNCTION__);
    Reset();
    TestSetting outerSetting;
    TestSetting innerSetting;
    outerSetting.dispatcher = TestDispatcher::MAIN;
    innerSetting.dispatcher = TestDispatcher::PARALLEL;
    innerSetting.create_group = true;
    innerSetting.group_notify = true;
    outerSetting.op = TestOperation::ASYNC;
    innerSetting.op = TestOperation::ASYNC_GROUP;
    int taskCount = Dispatch(outerSetting, innerSetting);
    bool result = Wait(taskCount);
    result = result && task_execution_sequence.size() > 1;
    if (result) {
        result = result && applyExecuted(outerSetting, innerSetting);
        result = result && OuterTaskExecuted(outerSetting);
        result = result && InnerTaskExecuted(innerSetting);
        result = result && InnerExecutedAfter(innerGroupNotifyId);
    }
    TestUtils::PublishEvent(g_EVENT_RESP_FIRST, code, std::to_string(result));
}

// level1:spec, async level2:parallel, sync barrier
void MainAbility::SpecCase20(int code)
{
    APP_LOGI("-- -- -- -- -- --MainAbility::%{public}s", __FUNCTION__);
    Reset();
    TestSetting outerSetting;
    TestSetting innerSetting;
    outerSetting.dispatcher = TestDispatcher::MAIN;
    innerSetting.dispatcher = TestDispatcher::PARALLEL;
    innerSetting.sync_barrier = true;
    innerSetting.create_group = true;
    outerSetting.op = TestOperation::ASYNC;
    innerSetting.op = TestOperation::ASYNC_GROUP;
    int taskCount = Dispatch(outerSetting, innerSetting);
    bool result = Wait(taskCount);
    result = result && task_execution_sequence.size() > 1;
    if (result) {
        result = result && applyExecuted(outerSetting, innerSetting);
        result = result && OuterTaskExecuted(outerSetting);
        result = result && InnerTaskExecuted(innerSetting);
        result = result && InnerExecutedAfter(innerSyncBarrierId);
    }
    TestUtils::PublishEvent(g_EVENT_RESP_FIRST, code, std::to_string(result));
}

// level1:spec, async level2:parallel, async barrier
void MainAbility::SpecCase21(int code)
{
    APP_LOGI("-- -- -- -- -- --MainAbility::%{public}s", __FUNCTION__);
    Reset();
    TestSetting outerSetting;
    TestSetting innerSetting;
    outerSetting.dispatcher = TestDispatcher::MAIN;
    innerSetting.dispatcher = TestDispatcher::PARALLEL;
    innerSetting.async_barrier = true;
    innerSetting.create_group = true;
    outerSetting.op = TestOperation::ASYNC;
    innerSetting.op = TestOperation::ASYNC_GROUP;
    int taskCount = Dispatch(outerSetting, innerSetting);
    bool result = Wait(taskCount);
    result = result && task_execution_sequence.size() > 1;
    if (result) {
        result = result && applyExecuted(outerSetting, innerSetting);
        result = result && OuterTaskExecuted(outerSetting);
        result = result && InnerTaskExecuted(innerSetting);
        result = result && InnerExecutedAfter(innerAsyncBarrierId);
    }
    TestUtils::PublishEvent(g_EVENT_RESP_FIRST, code, std::to_string(result));
}

// level1:spec, async level2:parallel, apply
void MainAbility::SpecCase22(int code)
{
    APP_LOGI("-- -- -- -- -- --MainAbility::%{public}s", __FUNCTION__);
    Reset();
    TestSetting outerSetting;
    TestSetting innerSetting;
    outerSetting.dispatcher = TestDispatcher::MAIN;
    innerSetting.dispatcher = TestDispatcher::PARALLEL;
    innerSetting.apply = applyNum;
    outerSetting.op = TestOperation::ASYNC;
    innerSetting.op = TestOperation::APPLY;
    int taskCount = Dispatch(outerSetting, innerSetting);
    bool result = Wait(taskCount);
    result = result && task_execution_sequence.size() > 1;
    if (result) {
        result = result && applyExecuted(outerSetting, innerSetting);
        result = result && OuterTaskExecuted(outerSetting);
        result = result && InnerTaskExecuted(innerSetting);
    }
    TestUtils::PublishEvent(g_EVENT_RESP_FIRST, code, std::to_string(result));
}

// level1:spec, async level2:serial, sync
void MainAbility::SpecCase23(int code)
{
    APP_LOGI("-- -- -- -- -- --MainAbility::%{public}s", __FUNCTION__);
    Reset();
    TestSetting outerSetting;
    TestSetting innerSetting;
    outerSetting.dispatcher = TestDispatcher::MAIN;
    innerSetting.dispatcher = TestDispatcher::SERIAL;
    outerSetting.op = TestOperation::ASYNC;
    innerSetting.op = TestOperation::SYNC;
    int taskCount = Dispatch(outerSetting, innerSetting);
    bool result = Wait(taskCount);
    result = result && task_execution_sequence.size() > 1;
    if (result) {
        result = result && applyExecuted(outerSetting, innerSetting);
        result = result && OuterTaskExecuted(outerSetting);
        result = result && InnerTaskExecuted(innerSetting);
        std::vector<size_t> outerTaskIndex;
        std::vector<std::vector<size_t>> innerTaskIndex;
        GetTaskIndex(outerTaskIndex, innerTaskIndex);
        for (int i = 0; i < testTaskCount; i++) {
            result = result && IsAscend(innerTaskIndex[i]);
        }
    }
    TestUtils::PublishEvent(g_EVENT_RESP_FIRST, code, std::to_string(result));
}

// level1:spec, async level2:serial, async
void MainAbility::SpecCase24(int code)
{
    APP_LOGI("-- -- -- -- -- --MainAbility::%{public}s", __FUNCTION__);
    Reset();
    TestSetting outerSetting;
    TestSetting innerSetting;
    outerSetting.dispatcher = TestDispatcher::MAIN;
    innerSetting.dispatcher = TestDispatcher::SERIAL;
    outerSetting.op = TestOperation::ASYNC;
    innerSetting.op = TestOperation::ASYNC;
    int taskCount = Dispatch(outerSetting, innerSetting);
    bool result = Wait(taskCount);
    result = result && task_execution_sequence.size() > 1;
    if (result) {
        result = result && applyExecuted(outerSetting, innerSetting);
        result = result && OuterTaskExecuted(outerSetting);
        result = result && InnerTaskExecuted(innerSetting);
        std::vector<size_t> outerTaskIndex;
        std::vector<std::vector<size_t>> innerTaskIndex;
        GetTaskIndex(outerTaskIndex, innerTaskIndex);
        for (int i = 0; i < testTaskCount; i++) {
            result = result && IsAscend(innerTaskIndex[i]);
        }
    }
    TestUtils::PublishEvent(g_EVENT_RESP_FIRST, code, std::to_string(result));
}

// level1:spec, async level2:serial, delay
void MainAbility::SpecCase25(int code)
{
    APP_LOGI("-- -- -- -- -- --MainAbility::%{public}s", __FUNCTION__);
    Reset();
    TestSetting outerSetting;
    TestSetting innerSetting;
    outerSetting.dispatcher = TestDispatcher::MAIN;
    innerSetting.dispatcher = TestDispatcher::SERIAL;
    innerSetting.delay = delayMs;
    outerSetting.op = TestOperation::ASYNC;
    innerSetting.op = TestOperation::DELAY;
    int taskCount = Dispatch(outerSetting, innerSetting);
    bool result = Wait(taskCount);
    result = result && task_execution_sequence.size() > 1;
    if (result) {
        result = result && applyExecuted(outerSetting, innerSetting);
        result = result && OuterTaskExecuted(outerSetting);
        result = result && InnerTaskExecuted(innerSetting);
        std::vector<size_t> outerTaskIndex;
        std::vector<std::vector<size_t>> innerTaskIndex;
        GetTaskIndex(outerTaskIndex, innerTaskIndex);
        for (int i = 0; i < testTaskCount; i++) {
            result = result && IsAscend(innerTaskIndex[i]);
        }
    }
    TestUtils::PublishEvent(g_EVENT_RESP_FIRST, code, std::to_string(result));
}

// level1:spec, async level2:serial, apply
void MainAbility::SpecCase26(int code)
{
    APP_LOGI("-- -- -- -- -- --MainAbility::%{public}s", __FUNCTION__);
    Reset();
    TestSetting outerSetting;
    TestSetting innerSetting;
    outerSetting.dispatcher = TestDispatcher::MAIN;
    innerSetting.dispatcher = TestDispatcher::SERIAL;
    innerSetting.apply = applyNum;
    outerSetting.op = TestOperation::ASYNC;
    innerSetting.op = TestOperation::APPLY;
    int taskCount = Dispatch(outerSetting, innerSetting);
    bool result = Wait(taskCount);
    result = result && task_execution_sequence.size() > 1;
    if (result) {
        result = result && applyExecuted(outerSetting, innerSetting);
        result = result && OuterTaskExecuted(outerSetting);
        result = result && InnerTaskExecuted(innerSetting);
    }
    TestUtils::PublishEvent(g_EVENT_RESP_FIRST, code, std::to_string(result));
}

// level1:spec, delay level2:parallel, sync
void MainAbility::SpecCase27(int code)
{
    APP_LOGI("-- -- -- -- -- --MainAbility::%{public}s", __FUNCTION__);
    Reset();
    TestSetting outerSetting;
    TestSetting innerSetting;
    outerSetting.dispatcher = TestDispatcher::MAIN;
    innerSetting.dispatcher = TestDispatcher::PARALLEL;
    outerSetting.delay = delayMs;
    outerSetting.op = TestOperation::DELAY;
    innerSetting.op = TestOperation::SYNC;
    int taskCount = Dispatch(outerSetting, innerSetting);
    bool result = Wait(taskCount);
    result = result && task_execution_sequence.size() > 1;
    if (result) {
        result = result && applyExecuted(outerSetting, innerSetting);
        result = result && OuterTaskExecuted(outerSetting);
        result = result && InnerTaskExecuted(innerSetting);
        std::vector<size_t> outerTaskIndex;
        std::vector<std::vector<size_t>> innerTaskIndex;
        GetTaskIndex(outerTaskIndex, innerTaskIndex);
        for (int i = 0; i < testTaskCount; i++) {
            result = result && IsAscend(innerTaskIndex[i]);
        }
    }
    TestUtils::PublishEvent(g_EVENT_RESP_FIRST, code, std::to_string(result));
}

// level1:spec, delay level2:parallel, async
void MainAbility::SpecCase28(int code)
{
    APP_LOGI("-- -- -- -- -- --MainAbility::%{public}s", __FUNCTION__);
    Reset();
    TestSetting outerSetting;
    TestSetting innerSetting;
    outerSetting.dispatcher = TestDispatcher::MAIN;
    innerSetting.dispatcher = TestDispatcher::PARALLEL;
    outerSetting.delay = delayMs;
    outerSetting.op = TestOperation::DELAY;
    innerSetting.op = TestOperation::ASYNC;
    int taskCount = Dispatch(outerSetting, innerSetting);
    bool result = Wait(taskCount);
    result = result && task_execution_sequence.size() > 1;
    if (result) {
        result = result && applyExecuted(outerSetting, innerSetting);
        result = result && OuterTaskExecuted(outerSetting);
        result = result && InnerTaskExecuted(innerSetting);
    }
    TestUtils::PublishEvent(g_EVENT_RESP_FIRST, code, std::to_string(result));
}

// level1:spec, delay level2:parallel, delay
void MainAbility::SpecCase29(int code)
{
    APP_LOGI("-- -- -- -- -- --MainAbility::%{public}s", __FUNCTION__);
    Reset();
    TestSetting outerSetting;
    TestSetting innerSetting;
    outerSetting.dispatcher = TestDispatcher::MAIN;
    innerSetting.dispatcher = TestDispatcher::PARALLEL;
    outerSetting.delay = delayMs;
    innerSetting.delay = delayMs;
    outerSetting.op = TestOperation::DELAY;
    innerSetting.op = TestOperation::DELAY;
    int taskCount = Dispatch(outerSetting, innerSetting);
    bool result = Wait(taskCount);
    result = result && task_execution_sequence.size() > 1;
    if (result) {
        result = result && applyExecuted(outerSetting, innerSetting);
        result = result && OuterTaskExecuted(outerSetting);
        result = result && InnerTaskExecuted(innerSetting);
    }
    TestUtils::PublishEvent(g_EVENT_RESP_FIRST, code, std::to_string(result));
}

// level1:spec, delay level2:parallel, group
void MainAbility::SpecCase30(int code)
{
    APP_LOGI("-- -- -- -- -- --MainAbility::%{public}s", __FUNCTION__);
    Reset();
    TestSetting outerSetting;
    TestSetting innerSetting;
    outerSetting.dispatcher = TestDispatcher::MAIN;
    innerSetting.dispatcher = TestDispatcher::PARALLEL;
    outerSetting.delay = delayMs;
    innerSetting.create_group = true;
    outerSetting.op = TestOperation::DELAY;
    innerSetting.op = TestOperation::ASYNC_GROUP;
    int taskCount = Dispatch(outerSetting, innerSetting);
    bool result = Wait(taskCount);
    result = result && task_execution_sequence.size() > 1;
    if (result) {
        result = result && applyExecuted(outerSetting, innerSetting);
        result = result && OuterTaskExecuted(outerSetting);
        result = result && InnerTaskExecuted(innerSetting);
    }
    TestUtils::PublishEvent(g_EVENT_RESP_FIRST, code, std::to_string(result));
}

// level1:spec, delay level2:parallel, group wait
void MainAbility::SpecCase31(int code)
{
    APP_LOGI("-- -- -- -- -- --MainAbility::%{public}s", __FUNCTION__);
    Reset();
    TestSetting outerSetting;
    TestSetting innerSetting;
    outerSetting.dispatcher = TestDispatcher::MAIN;
    innerSetting.dispatcher = TestDispatcher::PARALLEL;
    innerSetting.group_wait = true;
    innerSetting.group_timeout = groupWait;
    outerSetting.delay = delayMs;
    innerSetting.create_group = true;
    outerSetting.op = TestOperation::DELAY;
    innerSetting.op = TestOperation::ASYNC_GROUP;
    int taskCount = Dispatch(outerSetting, innerSetting);
    bool result = Wait(taskCount);
    result = result && task_execution_sequence.size() > 1;
    if (result) {
        result = result && applyExecuted(outerSetting, innerSetting);
        result = result && OuterTaskExecuted(outerSetting);
        result = result && InnerTaskExecuted(innerSetting);
    }
    TestUtils::PublishEvent(g_EVENT_RESP_FIRST, code, std::to_string(result));
}

// level1:spec, delay level2:parallel, group notify
void MainAbility::SpecCase32(int code)
{
    APP_LOGI("-- -- -- -- -- --MainAbility::%{public}s", __FUNCTION__);
    Reset();
    TestSetting outerSetting;
    TestSetting innerSetting;
    outerSetting.dispatcher = TestDispatcher::MAIN;
    innerSetting.dispatcher = TestDispatcher::PARALLEL;
    outerSetting.delay = delayMs;
    innerSetting.create_group = true;
    innerSetting.group_notify = true;
    outerSetting.op = TestOperation::DELAY;
    innerSetting.op = TestOperation::ASYNC_GROUP;
    int taskCount = Dispatch(outerSetting, innerSetting);
    bool result = Wait(taskCount);
    result = result && task_execution_sequence.size() > 1;
    if (result) {
        result = result && applyExecuted(outerSetting, innerSetting);
        result = result && OuterTaskExecuted(outerSetting);
        result = result && InnerTaskExecuted(innerSetting);
        result = result && InnerExecutedAfter(innerGroupNotifyId);
    }
    TestUtils::PublishEvent(g_EVENT_RESP_FIRST, code, std::to_string(result));
}

// level1:spec, delay level2:parallel, sync barrier
void MainAbility::SpecCase33(int code)
{
    APP_LOGI("-- -- -- -- -- --MainAbility::%{public}s", __FUNCTION__);
    Reset();
    TestSetting outerSetting;
    TestSetting innerSetting;
    outerSetting.dispatcher = TestDispatcher::MAIN;
    innerSetting.dispatcher = TestDispatcher::PARALLEL;
    outerSetting.delay = delayMs;
    innerSetting.sync_barrier = true;
    innerSetting.create_group = true;
    outerSetting.op = TestOperation::DELAY;
    innerSetting.op = TestOperation::ASYNC_GROUP;
    int taskCount = Dispatch(outerSetting, innerSetting);
    bool result = Wait(taskCount);
    result = result && task_execution_sequence.size() > 1;
    if (result) {
        result = result && applyExecuted(outerSetting, innerSetting);
        result = result && OuterTaskExecuted(outerSetting);
        result = result && InnerTaskExecuted(innerSetting);
        result = result && InnerExecutedAfter(innerSyncBarrierId);
    }
    TestUtils::PublishEvent(g_EVENT_RESP_FIRST, code, std::to_string(result));
}

// level1:spec, delay level2:parallel, async barrier
void MainAbility::SpecCase34(int code)
{
    APP_LOGI("-- -- -- -- -- --MainAbility::%{public}s", __FUNCTION__);
    Reset();
    TestSetting outerSetting;
    TestSetting innerSetting;
    outerSetting.dispatcher = TestDispatcher::MAIN;
    innerSetting.dispatcher = TestDispatcher::PARALLEL;
    outerSetting.delay = delayMs;
    innerSetting.async_barrier = true;
    innerSetting.create_group = true;
    outerSetting.op = TestOperation::DELAY;
    innerSetting.op = TestOperation::ASYNC_GROUP;
    int taskCount = Dispatch(outerSetting, innerSetting);
    bool result = Wait(taskCount);
    result = result && task_execution_sequence.size() > 1;
    if (result) {
        result = result && applyExecuted(outerSetting, innerSetting);
        result = result && OuterTaskExecuted(outerSetting);
        result = result && InnerTaskExecuted(innerSetting);
        result = result && InnerExecutedAfter(innerAsyncBarrierId);
    }
    TestUtils::PublishEvent(g_EVENT_RESP_FIRST, code, std::to_string(result));
}

// level1:spec, delay level2:parallel, apply
void MainAbility::SpecCase35(int code)
{
    APP_LOGI("-- -- -- -- -- --MainAbility::%{public}s", __FUNCTION__);
    Reset();
    TestSetting outerSetting;
    TestSetting innerSetting;
    outerSetting.dispatcher = TestDispatcher::MAIN;
    innerSetting.dispatcher = TestDispatcher::PARALLEL;
    outerSetting.delay = delayMs;
    innerSetting.apply = applyNum;
    outerSetting.op = TestOperation::DELAY;
    innerSetting.op = TestOperation::APPLY;
    int taskCount = Dispatch(outerSetting, innerSetting);
    bool result = Wait(taskCount);
    result = result && task_execution_sequence.size() > 1;
    if (result) {
        result = result && applyExecuted(outerSetting, innerSetting);
        result = result && OuterTaskExecuted(outerSetting);
        result = result && InnerTaskExecuted(innerSetting);
    }
    TestUtils::PublishEvent(g_EVENT_RESP_FIRST, code, std::to_string(result));
}

// level1:spec, delay level2:serial, sync
void MainAbility::SpecCase36(int code)
{
    APP_LOGI("-- -- -- -- -- --MainAbility::%{public}s", __FUNCTION__);
    Reset();
    TestSetting outerSetting;
    TestSetting innerSetting;
    outerSetting.dispatcher = TestDispatcher::MAIN;
    innerSetting.dispatcher = TestDispatcher::SERIAL;
    outerSetting.delay = delayMs;
    outerSetting.op = TestOperation::DELAY;
    innerSetting.op = TestOperation::SYNC;
    int taskCount = Dispatch(outerSetting, innerSetting);
    bool result = Wait(taskCount);
    result = result && task_execution_sequence.size() > 1;
    if (result) {
        result = result && applyExecuted(outerSetting, innerSetting);
        result = result && OuterTaskExecuted(outerSetting);
        result = result && InnerTaskExecuted(innerSetting);
        std::vector<size_t> outerTaskIndex;
        std::vector<std::vector<size_t>> innerTaskIndex;
        GetTaskIndex(outerTaskIndex, innerTaskIndex);
        for (int i = 0; i < testTaskCount; i++) {
            result = result && IsAscend(innerTaskIndex[i]);
        }
    }
    TestUtils::PublishEvent(g_EVENT_RESP_FIRST, code, std::to_string(result));
}

// level1:spec, delay level2:serial, async
void MainAbility::SpecCase37(int code)
{
    APP_LOGI("-- -- -- -- -- --MainAbility::%{public}s", __FUNCTION__);
    Reset();
    TestSetting outerSetting;
    TestSetting innerSetting;
    outerSetting.dispatcher = TestDispatcher::MAIN;
    innerSetting.dispatcher = TestDispatcher::SERIAL;
    outerSetting.delay = delayMs;
    outerSetting.op = TestOperation::DELAY;
    innerSetting.op = TestOperation::ASYNC;
    int taskCount = Dispatch(outerSetting, innerSetting);
    bool result = Wait(taskCount);
    result = result && task_execution_sequence.size() > 1;
    if (result) {
        result = result && applyExecuted(outerSetting, innerSetting);
        result = result && OuterTaskExecuted(outerSetting);
        result = result && InnerTaskExecuted(innerSetting);
        std::vector<size_t> outerTaskIndex;
        std::vector<std::vector<size_t>> innerTaskIndex;
        GetTaskIndex(outerTaskIndex, innerTaskIndex);
        for (int i = 0; i < testTaskCount; i++) {
            result = result && IsAscend(innerTaskIndex[i]);
        }
    }
    TestUtils::PublishEvent(g_EVENT_RESP_FIRST, code, std::to_string(result));
}

// level1:spec, delay level2:serial, delay
void MainAbility::SpecCase38(int code)
{
    APP_LOGI("-- -- -- -- -- --MainAbility::%{public}s", __FUNCTION__);
    Reset();
    TestSetting outerSetting;
    TestSetting innerSetting;
    outerSetting.dispatcher = TestDispatcher::MAIN;
    innerSetting.dispatcher = TestDispatcher::SERIAL;
    outerSetting.delay = delayMs;
    innerSetting.delay = delayMs;
    outerSetting.op = TestOperation::DELAY;
    innerSetting.op = TestOperation::DELAY;
    int taskCount = Dispatch(outerSetting, innerSetting);
    bool result = Wait(taskCount);
    result = result && task_execution_sequence.size() > 1;
    if (result) {
        result = result && applyExecuted(outerSetting, innerSetting);
        result = result && OuterTaskExecuted(outerSetting);
        result = result && InnerTaskExecuted(innerSetting);
        std::vector<size_t> outerTaskIndex;
        std::vector<std::vector<size_t>> innerTaskIndex;
        GetTaskIndex(outerTaskIndex, innerTaskIndex);
        for (int i = 0; i < testTaskCount; i++) {
            result = result && IsAscend(innerTaskIndex[i]);
        }
    }
    TestUtils::PublishEvent(g_EVENT_RESP_FIRST, code, std::to_string(result));
}

// level1:spec, delay level2:serial, apply
void MainAbility::SpecCase39(int code)
{
    APP_LOGI("-- -- -- -- -- --MainAbility::%{public}s", __FUNCTION__);
    Reset();
    TestSetting outerSetting;
    TestSetting innerSetting;
    outerSetting.dispatcher = TestDispatcher::MAIN;
    innerSetting.dispatcher = TestDispatcher::SERIAL;
    outerSetting.delay = delayMs;
    innerSetting.apply = applyNum;
    outerSetting.op = TestOperation::DELAY;
    innerSetting.op = TestOperation::APPLY;
    int taskCount = Dispatch(outerSetting, innerSetting);
    bool result = Wait(taskCount);
    result = result && task_execution_sequence.size() > 1;
    if (result) {
        result = result && applyExecuted(outerSetting, innerSetting);
        result = result && OuterTaskExecuted(outerSetting);
        result = result && InnerTaskExecuted(innerSetting);
    }
    TestUtils::PublishEvent(g_EVENT_RESP_FIRST, code, std::to_string(result));
}

// level1:spec, apply level2:parallel, sync
void MainAbility::SpecCase40(int code)
{
    APP_LOGI("-- -- -- -- -- --MainAbility::%{public}s", __FUNCTION__);
    Reset();
    TestSetting outerSetting;
    TestSetting innerSetting;
    outerSetting.dispatcher = TestDispatcher::MAIN;
    innerSetting.dispatcher = TestDispatcher::PARALLEL;
    outerSetting.apply = applyNum;
    outerSetting.op = TestOperation::APPLY;
    innerSetting.op = TestOperation::SYNC;
    int taskCount = Dispatch(outerSetting, innerSetting);
    bool result = Wait(taskCount);
    result = result && task_execution_sequence.size() > 1;
    if (result) {
        result = result && applyExecuted(outerSetting, innerSetting);
        result = result && OuterTaskExecuted(outerSetting);
        result = result && InnerTaskExecuted(innerSetting);
        std::vector<size_t> outerTaskIndex;
        std::vector<std::vector<size_t>> innerTaskIndex;
        GetTaskIndex(outerTaskIndex, innerTaskIndex);
        for (int i = 0; i < testTaskCount; i++) {
            result = result && IsAscend(innerTaskIndex[i]);
        }
    }
    TestUtils::PublishEvent(g_EVENT_RESP_FIRST, code, std::to_string(result));
}

// level1:spec, apply level2:parallel, async
void MainAbility::SpecCase41(int code)
{
    APP_LOGI("-- -- -- -- -- --MainAbility::%{public}s", __FUNCTION__);
    Reset();
    TestSetting outerSetting;
    TestSetting innerSetting;
    outerSetting.dispatcher = TestDispatcher::MAIN;
    innerSetting.dispatcher = TestDispatcher::PARALLEL;
    outerSetting.apply = applyNum;
    outerSetting.op = TestOperation::APPLY;
    innerSetting.op = TestOperation::ASYNC;
    int taskCount = Dispatch(outerSetting, innerSetting);
    bool result = Wait(taskCount);
    result = result && task_execution_sequence.size() > 1;
    if (result) {
        result = result && applyExecuted(outerSetting, innerSetting);
        result = result && OuterTaskExecuted(outerSetting);
        result = result && InnerTaskExecuted(innerSetting);
    }
    TestUtils::PublishEvent(g_EVENT_RESP_FIRST, code, std::to_string(result));
}

// level1:spec, apply level2:parallel, delay
void MainAbility::SpecCase42(int code)
{
    APP_LOGI("-- -- -- -- -- --MainAbility::%{public}s", __FUNCTION__);
    Reset();
    TestSetting outerSetting;
    TestSetting innerSetting;
    outerSetting.dispatcher = TestDispatcher::MAIN;
    innerSetting.dispatcher = TestDispatcher::PARALLEL;
    outerSetting.apply = applyNum;
    innerSetting.delay = delayMs;
    outerSetting.op = TestOperation::APPLY;
    innerSetting.op = TestOperation::DELAY;
    int taskCount = Dispatch(outerSetting, innerSetting);
    bool result = Wait(taskCount);
    result = result && task_execution_sequence.size() > 1;
    if (result) {
        result = result && applyExecuted(outerSetting, innerSetting);
        result = result && OuterTaskExecuted(outerSetting);
        result = result && InnerTaskExecuted(innerSetting);
    }
    TestUtils::PublishEvent(g_EVENT_RESP_FIRST, code, std::to_string(result));
}

// level1:spec, apply level2:parallel, group
void MainAbility::SpecCase43(int code)
{
    APP_LOGI("-- -- -- -- -- --MainAbility::%{public}s", __FUNCTION__);
    Reset();
    TestSetting outerSetting;
    TestSetting innerSetting;
    outerSetting.dispatcher = TestDispatcher::MAIN;
    innerSetting.dispatcher = TestDispatcher::PARALLEL;
    outerSetting.apply = applyNum;
    innerSetting.create_group = true;
    outerSetting.op = TestOperation::APPLY;
    innerSetting.op = TestOperation::ASYNC_GROUP;
    int taskCount = Dispatch(outerSetting, innerSetting);
    bool result = Wait(taskCount);
    result = result && task_execution_sequence.size() > 1;
    if (result) {
        result = result && applyExecuted(outerSetting, innerSetting);
        result = result && OuterTaskExecuted(outerSetting);
        result = result && InnerTaskExecuted(innerSetting);
    }
    TestUtils::PublishEvent(g_EVENT_RESP_FIRST, code, std::to_string(result));
}

// level1:spec, apply level2:parallel, group wait
void MainAbility::SpecCase44(int code)
{
    APP_LOGI("-- -- -- -- -- --MainAbility::%{public}s", __FUNCTION__);
    Reset();
    TestSetting outerSetting;
    TestSetting innerSetting;
    outerSetting.dispatcher = TestDispatcher::MAIN;
    innerSetting.dispatcher = TestDispatcher::PARALLEL;
    innerSetting.group_wait = true;
    innerSetting.group_timeout = groupWait;
    outerSetting.apply = applyNum;
    innerSetting.create_group = true;
    outerSetting.op = TestOperation::APPLY;
    innerSetting.op = TestOperation::ASYNC_GROUP;
    int taskCount = Dispatch(outerSetting, innerSetting);
    bool result = Wait(taskCount);
    result = result && task_execution_sequence.size() > 1;
    if (result) {
        result = result && applyExecuted(outerSetting, innerSetting);
        result = result && OuterTaskExecuted(outerSetting);
        result = result && InnerTaskExecuted(innerSetting);
    }
    TestUtils::PublishEvent(g_EVENT_RESP_FIRST, code, std::to_string(result));
}

// level1:spec, apply level2:parallel, group notify
void MainAbility::SpecCase45(int code)
{
    APP_LOGI("-- -- -- -- -- --MainAbility::%{public}s", __FUNCTION__);
    Reset();
    TestSetting outerSetting;
    TestSetting innerSetting;
    outerSetting.dispatcher = TestDispatcher::MAIN;
    innerSetting.dispatcher = TestDispatcher::PARALLEL;
    outerSetting.apply = applyNum;
    innerSetting.create_group = true;
    innerSetting.group_notify = true;
    outerSetting.op = TestOperation::APPLY;
    innerSetting.op = TestOperation::ASYNC_GROUP;
    int taskCount = Dispatch(outerSetting, innerSetting);
    bool result = Wait(taskCount);
    result = result && task_execution_sequence.size() > 1;
    if (result) {
        result = result && applyExecuted(outerSetting, innerSetting);
        result = result && OuterTaskExecuted(outerSetting);
        result = result && InnerTaskExecuted(innerSetting);
        result = result && InnerExecutedAfter(innerGroupNotifyId);
    }
    TestUtils::PublishEvent(g_EVENT_RESP_FIRST, code, std::to_string(result));
}

// level1:spec, apply level2:parallel, sync barrier
void MainAbility::SpecCase46(int code)
{
    APP_LOGI("-- -- -- -- -- --MainAbility::%{public}s", __FUNCTION__);
    Reset();
    TestSetting outerSetting;
    TestSetting innerSetting;
    outerSetting.dispatcher = TestDispatcher::MAIN;
    innerSetting.dispatcher = TestDispatcher::PARALLEL;
    outerSetting.apply = applyNum;
    innerSetting.sync_barrier = true;
    innerSetting.create_group = true;
    outerSetting.op = TestOperation::APPLY;
    innerSetting.op = TestOperation::ASYNC_GROUP;
    int taskCount = Dispatch(outerSetting, innerSetting);
    bool result = Wait(taskCount);
    result = result && task_execution_sequence.size() > 1;
    if (result) {
        result = result && applyExecuted(outerSetting, innerSetting);
        result = result && OuterTaskExecuted(outerSetting);
        result = result && InnerTaskExecuted(innerSetting);
        result = result && InnerExecutedAfter(innerSyncBarrierId);
    }
    TestUtils::PublishEvent(g_EVENT_RESP_FIRST, code, std::to_string(result));
}

// level1:spec, apply level2:parallel, async barrier
void MainAbility::SpecCase47(int code)
{
    APP_LOGI("-- -- -- -- -- --MainAbility::%{public}s", __FUNCTION__);
    Reset();
    TestSetting outerSetting;
    TestSetting innerSetting;
    outerSetting.dispatcher = TestDispatcher::MAIN;
    innerSetting.dispatcher = TestDispatcher::PARALLEL;
    outerSetting.apply = applyNum;
    innerSetting.async_barrier = true;
    innerSetting.create_group = true;
    outerSetting.op = TestOperation::APPLY;
    innerSetting.op = TestOperation::ASYNC_GROUP;
    int taskCount = Dispatch(outerSetting, innerSetting);
    bool result = Wait(taskCount);
    result = result && task_execution_sequence.size() > 1;
    if (result) {
        result = result && applyExecuted(outerSetting, innerSetting);
        result = result && OuterTaskExecuted(outerSetting);
        result = result && InnerTaskExecuted(innerSetting);
        result = result && InnerExecutedAfter(innerAsyncBarrierId);
    }
    TestUtils::PublishEvent(g_EVENT_RESP_FIRST, code, std::to_string(result));
}

// level1:spec, apply level2:parallel, apply
void MainAbility::SpecCase48(int code)
{
    APP_LOGI("-- -- -- -- -- --MainAbility::%{public}s", __FUNCTION__);
    Reset();
    TestSetting outerSetting;
    TestSetting innerSetting;
    outerSetting.dispatcher = TestDispatcher::MAIN;
    innerSetting.dispatcher = TestDispatcher::PARALLEL;
    outerSetting.apply = applyNum;
    innerSetting.apply = applyNum;
    outerSetting.op = TestOperation::APPLY;
    innerSetting.op = TestOperation::APPLY;
    int taskCount = Dispatch(outerSetting, innerSetting);
    bool result = Wait(taskCount);
    result = result && task_execution_sequence.size() > 1;
    if (result) {
        result = result && applyExecuted(outerSetting, innerSetting);
        result = result && OuterTaskExecuted(outerSetting);
        result = result && InnerTaskExecuted(innerSetting);
    }
    TestUtils::PublishEvent(g_EVENT_RESP_FIRST, code, std::to_string(result));
}

// level1:spec, apply level2:serial, sync
void MainAbility::SpecCase49(int code)
{
    APP_LOGI("-- -- -- -- -- --MainAbility::%{public}s", __FUNCTION__);
    Reset();
    TestSetting outerSetting;
    TestSetting innerSetting;
    outerSetting.dispatcher = TestDispatcher::MAIN;
    innerSetting.dispatcher = TestDispatcher::SERIAL;
    outerSetting.apply = applyNum;
    outerSetting.op = TestOperation::APPLY;
    innerSetting.op = TestOperation::SYNC;
    int taskCount = Dispatch(outerSetting, innerSetting);
    bool result = Wait(taskCount);
    result = result && task_execution_sequence.size() > 1;
    if (result) {
        result = result && applyExecuted(outerSetting, innerSetting);
        result = result && OuterTaskExecuted(outerSetting);
        result = result && InnerTaskExecuted(innerSetting);
        std::vector<size_t> outerTaskIndex;
        std::vector<std::vector<size_t>> innerTaskIndex;
        GetTaskIndex(outerTaskIndex, innerTaskIndex);
        for (int i = 0; i < testTaskCount; i++) {
            result = result && IsAscend(innerTaskIndex[i]);
        }
    }
    TestUtils::PublishEvent(g_EVENT_RESP_FIRST, code, std::to_string(result));
}

// level1:spec, apply level2:serial, async
void MainAbility::SpecCase50(int code)
{
    APP_LOGI("-- -- -- -- -- --MainAbility::%{public}s", __FUNCTION__);
    Reset();
    TestSetting outerSetting;
    TestSetting innerSetting;
    outerSetting.dispatcher = TestDispatcher::MAIN;
    innerSetting.dispatcher = TestDispatcher::SERIAL;
    outerSetting.apply = applyNum;
    outerSetting.op = TestOperation::APPLY;
    innerSetting.op = TestOperation::ASYNC;
    int taskCount = Dispatch(outerSetting, innerSetting);
    bool result = Wait(taskCount);
    result = result && task_execution_sequence.size() > 1;
    if (result) {
        result = result && applyExecuted(outerSetting, innerSetting);
        result = result && OuterTaskExecuted(outerSetting);
        result = result && InnerTaskExecuted(innerSetting);
        std::vector<size_t> outerTaskIndex;
        std::vector<std::vector<size_t>> innerTaskIndex;
        GetTaskIndex(outerTaskIndex, innerTaskIndex);
        for (int i = 0; i < testTaskCount; i++) {
            result = result && IsAscend(innerTaskIndex[i]);
        }
    }
    TestUtils::PublishEvent(g_EVENT_RESP_FIRST, code, std::to_string(result));
}

// level1:spec, apply level2:serial, delay
void MainAbility::SpecCase51(int code)
{
    APP_LOGI("-- -- -- -- -- --MainAbility::%{public}s", __FUNCTION__);
    Reset();
    TestSetting outerSetting;
    TestSetting innerSetting;
    outerSetting.dispatcher = TestDispatcher::MAIN;
    innerSetting.dispatcher = TestDispatcher::SERIAL;
    outerSetting.apply = applyNum;
    innerSetting.delay = delayMs;
    outerSetting.op = TestOperation::APPLY;
    innerSetting.op = TestOperation::DELAY;
    int taskCount = Dispatch(outerSetting, innerSetting);
    bool result = Wait(taskCount);
    result = result && task_execution_sequence.size() > 1;
    if (result) {
        result = result && applyExecuted(outerSetting, innerSetting);
        result = result && OuterTaskExecuted(outerSetting);
        result = result && InnerTaskExecuted(innerSetting);
        std::vector<size_t> outerTaskIndex;
        std::vector<std::vector<size_t>> innerTaskIndex;
        GetTaskIndex(outerTaskIndex, innerTaskIndex);
        for (int i = 0; i < testTaskCount; i++) {
            result = result && IsAscend(innerTaskIndex[i]);
        }
    }
    TestUtils::PublishEvent(g_EVENT_RESP_FIRST, code, std::to_string(result));
}

// level1:spec, apply level2:serial, apply
void MainAbility::SpecCase52(int code)
{
    APP_LOGI("-- -- -- -- -- --MainAbility::%{public}s", __FUNCTION__);
    Reset();
    TestSetting outerSetting;
    TestSetting innerSetting;
    outerSetting.dispatcher = TestDispatcher::MAIN;
    innerSetting.dispatcher = TestDispatcher::SERIAL;
    outerSetting.apply = applyNum;
    innerSetting.apply = applyNum;
    outerSetting.op = TestOperation::APPLY;
    innerSetting.op = TestOperation::APPLY;
    int taskCount = Dispatch(outerSetting, innerSetting);
    bool result = Wait(taskCount);
    result = result && task_execution_sequence.size() > 1;
    if (result) {
        result = result && applyExecuted(outerSetting, innerSetting);
        result = result && OuterTaskExecuted(outerSetting);
        result = result && InnerTaskExecuted(innerSetting);
    }
    TestUtils::PublishEvent(g_EVENT_RESP_FIRST, code, std::to_string(result));
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
void MainAbility::HybridCase1(int code)
{
    APP_LOGI("-- -- -- -- -- --MainAbility::%{public}s", __FUNCTION__);
    Reset();
    auto context = GetContext();
    TaskList globalDispatcher = TaskList {TestDispatcher::GLOBAL, context, "global"};
    std::vector<TestOperation> operationList = {
        TestOperation::SYNC,
        TestOperation::ASYNC,
        TestOperation::DELAY,
        TestOperation::CREATE_GROUP,
        TestOperation::ASYNC_GROUP,
        TestOperation::GROUP_NOTIFY,
        TestOperation::APPLY,
    };
    int taskId = 0;
    const int apply = 2;
    const int delay = 10;
    addTaskFromList(globalDispatcher, operationList, taskId, apply, delay);
    globalDispatcher.executedTask();
    Wait(taskId + apply - 1);
    bool result = task_execution_sequence.size() > 1;
    std::vector<size_t> outerTaskIndex;
    std::vector<std::vector<size_t>> innerTaskIndex;
    GetTaskIndex(outerTaskIndex, innerTaskIndex, taskId, 0);
    for (auto index : outerTaskIndex) {
        result = result && (index != std::string::npos);
    }
    TestUtils::PublishEvent(g_EVENT_RESP_FIRST, code, std::to_string(result));
};
void MainAbility::HybridCase2(int code)
{
    APP_LOGI("-- -- -- -- -- --MainAbility::%{public}s", __FUNCTION__);
    Reset();
    auto context = GetContext();
    TaskList parallelDispatcher = TaskList {TestDispatcher::PARALLEL, context, "parallel"};
    std::vector<TestOperation> operationList = {
        TestOperation::SYNC,
        TestOperation::ASYNC,
        TestOperation::DELAY,
        TestOperation::CREATE_GROUP,
        TestOperation::ASYNC_GROUP,
        TestOperation::SYNC_BARRIER,
        TestOperation::CREATE_GROUP,
        TestOperation::ASYNC_GROUP,
        TestOperation::GROUP_NOTIFY,
        TestOperation::ASYNC_BARRIER,
        TestOperation::APPLY,
    };
    int taskId = 0;
    const int apply = 2;
    const int delay = 10;
    addTaskFromList(parallelDispatcher, operationList, taskId, apply, delay);
    parallelDispatcher.executedTask();
    Wait(taskId + apply - 1);
    bool result = task_execution_sequence.size() > 1;
    std::vector<size_t> outerTaskIndex;
    std::vector<std::vector<size_t>> innerTaskIndex;
    GetTaskIndex(outerTaskIndex, innerTaskIndex, taskId, 0);
    for (auto index : outerTaskIndex) {
        result = result && (index != std::string::npos);
    }
    TestUtils::PublishEvent(g_EVENT_RESP_FIRST, code, std::to_string(result));
};
void MainAbility::HybridCase3(int code)
{
    APP_LOGI("-- -- -- -- -- --MainAbility::%{public}s", __FUNCTION__);
    Reset();
    auto context = GetContext();
    TaskList serialDispatcher = TaskList {TestDispatcher::SERIAL, context, "serial"};
    std::vector<TestOperation> operationList = {
        TestOperation::SYNC,
        TestOperation::ASYNC,
        TestOperation::DELAY,
        TestOperation::APPLY,
    };
    int taskId = 0;
    const int apply = 2;
    const int delay = 10;
    addTaskFromList(serialDispatcher, operationList, taskId, apply, delay);
    serialDispatcher.executedTask();
    Wait(taskId + apply - 1);
    bool result = task_execution_sequence.size() > 1;
    std::vector<size_t> outerTaskIndex;
    std::vector<std::vector<size_t>> innerTaskIndex;
    GetTaskIndex(outerTaskIndex, innerTaskIndex, taskId, 0);
    for (auto index : outerTaskIndex) {
        result = result && (index != std::string::npos);
    }
    TestUtils::PublishEvent(g_EVENT_RESP_FIRST, code, std::to_string(result));
};
void MainAbility::HybridCase4(int code)
{
    APP_LOGI("-- -- -- -- -- --MainAbility::%{public}s", __FUNCTION__);
    Reset();
    auto context = GetContext();
    TaskList mainDispatcher = TaskList {TestDispatcher::MAIN, context, "main"};
    std::vector<TestOperation> operationList = {
        TestOperation::SYNC,
        TestOperation::ASYNC,
        TestOperation::DELAY,
        TestOperation::APPLY,
    };
    int taskId = 0;
    const int apply = 2;
    const int delay = 10;
    addTaskFromList(mainDispatcher, operationList, taskId, apply, delay);
    mainDispatcher.executedTask();
    Wait(taskId + apply - 1);
    bool result = task_execution_sequence.size() > 1;
    std::vector<size_t> outerTaskIndex;
    std::vector<std::vector<size_t>> innerTaskIndex;
    GetTaskIndex(outerTaskIndex, innerTaskIndex, taskId, 0);
    for (auto index : outerTaskIndex) {
        result = result && (index != std::string::npos);
    }
    TestUtils::PublishEvent(g_EVENT_RESP_FIRST, code, std::to_string(result));
};
void MainAbility::HybridCase5(int code)
{
    APP_LOGI("-- -- -- -- -- --MainAbility::%{public}s", __FUNCTION__);
    Reset();
    auto context = GetContext();
    TaskList globalDispatcher = TaskList {TestDispatcher::GLOBAL, context, "global"};
    TaskList parallelDispatcher1 = TaskList {TestDispatcher::PARALLEL, context, "parallel1"};
    TaskList parallelDispatcher2 = TaskList {TestDispatcher::PARALLEL, context, "parallel2"};
    TaskList serialDispatcher1 = TaskList {TestDispatcher::SERIAL, context, "serial1"};
    TaskList serialDispatcher2 = TaskList {TestDispatcher::SERIAL, context, "serial2"};
    TaskList mainDispatcher = TaskList {TestDispatcher::MAIN, context, "main"};
    int taskId = 0;
    globalDispatcher.addOperation(TestOperation::SYNC)
        .addFunc(std::make_shared<Runnable>([=]() { TestTask(std::to_string(taskId)); }))
        .executedTask();
    taskId++;
    parallelDispatcher1.addOperation(TestOperation::SYNC)
        .addFunc(std::make_shared<Runnable>([=]() { TestTask(std::to_string(taskId)); }))
        .executedTask();
    taskId++;
    parallelDispatcher2.addOperation(TestOperation::SYNC)
        .addFunc(std::make_shared<Runnable>([=]() { TestTask(std::to_string(taskId)); }))
        .executedTask();
    taskId++;
    serialDispatcher1.addOperation(TestOperation::SYNC)
        .addFunc(std::make_shared<Runnable>([=]() { TestTask(std::to_string(taskId)); }))
        .executedTask();
    taskId++;
    serialDispatcher2.addOperation(TestOperation::SYNC)
        .addFunc(std::make_shared<Runnable>([=]() { TestTask(std::to_string(taskId)); }))
        .executedTask();
    taskId++;
    mainDispatcher.addOperation(TestOperation::SYNC)
        .addFunc(std::make_shared<Runnable>([=]() { TestTask(std::to_string(taskId)); }))
        .executedTask();
    taskId++;
    Wait(taskId);
    bool result = task_execution_sequence.size() > 1;
    std::vector<size_t> outerTaskIndex;
    std::vector<std::vector<size_t>> innerTaskIndex;
    GetTaskIndex(outerTaskIndex, innerTaskIndex, taskId, 0);
    for (auto index : outerTaskIndex) {
        result = result && (index != std::string::npos);
    }
    TestUtils::PublishEvent(g_EVENT_RESP_FIRST, code, std::to_string(result));
};
void MainAbility::HybridCase6(int code)
{
    APP_LOGI("-- -- -- -- -- --MainAbility::%{public}s", __FUNCTION__);
    Reset();
    auto context = GetContext();
    TaskList globalDispatcher = TaskList {TestDispatcher::GLOBAL, context, "global"};
    TaskList parallelDispatcher1 = TaskList {TestDispatcher::PARALLEL, context, "parallel1"};
    TaskList parallelDispatcher2 = TaskList {TestDispatcher::PARALLEL, context, "parallel2"};
    TaskList serialDispatcher1 = TaskList {TestDispatcher::SERIAL, context, "serial1"};
    TaskList serialDispatcher2 = TaskList {TestDispatcher::SERIAL, context, "serial2"};
    TaskList mainDispatcher = TaskList {TestDispatcher::MAIN, context, "main"};
    int taskId = 0;
    globalDispatcher.addOperation(TestOperation::ASYNC)
        .addFunc(std::make_shared<Runnable>([=]() { TestTask(std::to_string(taskId)); }))
        .executedTask();
    taskId++;
    parallelDispatcher1.addOperation(TestOperation::ASYNC)
        .addFunc(std::make_shared<Runnable>([=]() { TestTask(std::to_string(taskId)); }))
        .executedTask();
    taskId++;
    parallelDispatcher2.addOperation(TestOperation::ASYNC)
        .addFunc(std::make_shared<Runnable>([=]() { TestTask(std::to_string(taskId)); }))
        .executedTask();
    taskId++;
    serialDispatcher1.addOperation(TestOperation::ASYNC)
        .addFunc(std::make_shared<Runnable>([=]() { TestTask(std::to_string(taskId)); }))
        .executedTask();
    taskId++;
    serialDispatcher2.addOperation(TestOperation::ASYNC)
        .addFunc(std::make_shared<Runnable>([=]() { TestTask(std::to_string(taskId)); }))
        .executedTask();
    taskId++;
    mainDispatcher.addOperation(TestOperation::ASYNC)
        .addFunc(std::make_shared<Runnable>([=]() { TestTask(std::to_string(taskId)); }))
        .executedTask();
    taskId++;
    Wait(taskId);
    bool result = task_execution_sequence.size() > 1;
    std::vector<size_t> outerTaskIndex;
    std::vector<std::vector<size_t>> innerTaskIndex;
    GetTaskIndex(outerTaskIndex, innerTaskIndex, taskId, 0);
    for (auto index : outerTaskIndex) {
        result = result && (index != std::string::npos);
    }
    TestUtils::PublishEvent(g_EVENT_RESP_FIRST, code, std::to_string(result));
};
void MainAbility::HybridCase7(int code)
{
    APP_LOGI("-- -- -- -- -- --MainAbility::%{public}s", __FUNCTION__);
    Reset();
    auto context = GetContext();
    TaskList globalDispatcher = TaskList {TestDispatcher::GLOBAL, context, "global"};
    TaskList parallelDispatcher1 = TaskList {TestDispatcher::PARALLEL, context, "parallel1"};
    TaskList parallelDispatcher2 = TaskList {TestDispatcher::PARALLEL, context, "parallel2"};
    TaskList serialDispatcher1 = TaskList {TestDispatcher::SERIAL, context, "serial1"};
    TaskList serialDispatcher2 = TaskList {TestDispatcher::SERIAL, context, "serial2"};
    TaskList mainDispatcher = TaskList {TestDispatcher::MAIN, context, "main"};
    int taskId = 0;
    const long delay = 10;
    globalDispatcher.addOperation(TestOperation::DELAY)
        .addFunc(std::make_shared<Runnable>([=]() { TestTask(std::to_string(taskId)); }))
        .addDelay(delay)
        .executedTask();
    taskId++;
    parallelDispatcher1.addOperation(TestOperation::DELAY)
        .addFunc(std::make_shared<Runnable>([=]() { TestTask(std::to_string(taskId)); }))
        .addDelay(delay)
        .executedTask();
    taskId++;
    parallelDispatcher2.addOperation(TestOperation::DELAY)
        .addFunc(std::make_shared<Runnable>([=]() { TestTask(std::to_string(taskId)); }))
        .addDelay(delay)
        .executedTask();
    taskId++;
    serialDispatcher1.addOperation(TestOperation::DELAY)
        .addFunc(std::make_shared<Runnable>([=]() { TestTask(std::to_string(taskId)); }))
        .addDelay(delay)
        .executedTask();
    taskId++;
    serialDispatcher2.addOperation(TestOperation::DELAY)
        .addFunc(std::make_shared<Runnable>([=]() { TestTask(std::to_string(taskId)); }))
        .addDelay(delay)
        .executedTask();
    taskId++;
    mainDispatcher.addOperation(TestOperation::DELAY)
        .addFunc(std::make_shared<Runnable>([=]() { TestTask(std::to_string(taskId)); }))
        .addDelay(delay)
        .executedTask();
    taskId++;
    Wait(taskId);
    bool result = task_execution_sequence.size() > 1;
    std::vector<size_t> outerTaskIndex;
    std::vector<std::vector<size_t>> innerTaskIndex;
    GetTaskIndex(outerTaskIndex, innerTaskIndex, taskId, 0);
    for (auto index : outerTaskIndex) {
        result = result && (index != std::string::npos);
    }
    TestUtils::PublishEvent(g_EVENT_RESP_FIRST, code, std::to_string(result));
};
void MainAbility::HybridCase8(int code)
{
    APP_LOGI("-- -- -- -- -- --MainAbility::%{public}s", __FUNCTION__);
    Reset();
    auto context = GetContext();
    TaskList globalDispatcher = TaskList {TestDispatcher::GLOBAL, context, "global"};
    TaskList parallelDispatcher1 = TaskList {TestDispatcher::PARALLEL, context, "parallel1"};
    TaskList parallelDispatcher2 = TaskList {TestDispatcher::PARALLEL, context, "parallel2"};
    TaskList serialDispatcher1 = TaskList {TestDispatcher::SERIAL, context, "serial1"};
    TaskList serialDispatcher2 = TaskList {TestDispatcher::SERIAL, context, "serial2"};
    TaskList mainDispatcher = TaskList {TestDispatcher::MAIN, context, "main"};
    int taskId = 0;
    globalDispatcher.addOperation(TestOperation::CREATE_GROUP)
        .addOperation(TestOperation::ASYNC_GROUP)
        .addFunc(std::make_shared<Runnable>([=]() { TestTask(std::to_string(taskId)); }))
        .executedTask();
    taskId++;
    parallelDispatcher1.addOperation(TestOperation::CREATE_GROUP)
        .addOperation(TestOperation::ASYNC_GROUP)
        .addFunc(std::make_shared<Runnable>([=]() { TestTask(std::to_string(taskId)); }))
        .executedTask();
    taskId++;
    parallelDispatcher2.addOperation(TestOperation::CREATE_GROUP)
        .addOperation(TestOperation::ASYNC_GROUP)
        .addFunc(std::make_shared<Runnable>([=]() { TestTask(std::to_string(taskId)); }))
        .executedTask();
    taskId++;
    serialDispatcher1.addOperation(TestOperation::SYNC)
        .addFunc(std::make_shared<Runnable>([=]() { TestTask(std::to_string(taskId)); }))
        .executedTask();
    taskId++;
    serialDispatcher2.addOperation(TestOperation::SYNC)
        .addFunc(std::make_shared<Runnable>([=]() { TestTask(std::to_string(taskId)); }))
        .executedTask();
    taskId++;
    mainDispatcher.addOperation(TestOperation::SYNC)
        .addFunc(std::make_shared<Runnable>([=]() { TestTask(std::to_string(taskId)); }))
        .executedTask();
    taskId++;
    Wait(taskId);
    bool result = task_execution_sequence.size() > 1;
    std::vector<size_t> outerTaskIndex;
    std::vector<std::vector<size_t>> innerTaskIndex;
    GetTaskIndex(outerTaskIndex, innerTaskIndex, taskId, 0);
    for (auto index : outerTaskIndex) {
        result = result && (index != std::string::npos);
    }
    TestUtils::PublishEvent(g_EVENT_RESP_FIRST, code, std::to_string(result));
};
void MainAbility::HybridCase9(int code)
{
    APP_LOGI("-- -- -- -- -- --MainAbility::%{public}s", __FUNCTION__);
    Reset();
    auto context = GetContext();
    TaskList globalDispatcher = TaskList {TestDispatcher::GLOBAL, context, "global"};
    TaskList parallelDispatcher1 = TaskList {TestDispatcher::PARALLEL, context, "parallel1"};
    TaskList parallelDispatcher2 = TaskList {TestDispatcher::PARALLEL, context, "parallel2"};
    TaskList serialDispatcher1 = TaskList {TestDispatcher::SERIAL, context, "serial1"};
    TaskList serialDispatcher2 = TaskList {TestDispatcher::SERIAL, context, "serial2"};
    TaskList mainDispatcher = TaskList {TestDispatcher::MAIN, context, "main"};
    int taskId = 0;
    globalDispatcher.addOperation(TestOperation::CREATE_GROUP)
        .addOperation(TestOperation::ASYNC_GROUP)
        .addFunc(std::make_shared<Runnable>([=]() { TestTask(std::to_string(taskId)); }))
        .executedTask();
    taskId++;
    parallelDispatcher1.addOperation(TestOperation::CREATE_GROUP)
        .addOperation(TestOperation::ASYNC_GROUP)
        .addFunc(std::make_shared<Runnable>([=]() { TestTask(std::to_string(taskId)); }))
        .executedTask();
    taskId++;
    parallelDispatcher2.addOperation(TestOperation::CREATE_GROUP)
        .addOperation(TestOperation::ASYNC_GROUP)
        .addFunc(std::make_shared<Runnable>([=]() { TestTask(std::to_string(taskId)); }))
        .executedTask();
    taskId++;
    serialDispatcher1.addOperation(TestOperation::ASYNC)
        .addFunc(std::make_shared<Runnable>([=]() { TestTask(std::to_string(taskId)); }))
        .executedTask();
    taskId++;
    serialDispatcher2.addOperation(TestOperation::ASYNC)
        .addFunc(std::make_shared<Runnable>([=]() { TestTask(std::to_string(taskId)); }))
        .executedTask();
    taskId++;
    mainDispatcher.addOperation(TestOperation::ASYNC)
        .addFunc(std::make_shared<Runnable>([=]() { TestTask(std::to_string(taskId)); }))
        .executedTask();
    taskId++;
    Wait(taskId);
    bool result = task_execution_sequence.size() > 1;
    std::vector<size_t> outerTaskIndex;
    std::vector<std::vector<size_t>> innerTaskIndex;
    GetTaskIndex(outerTaskIndex, innerTaskIndex, taskId, 0);
    for (auto index : outerTaskIndex) {
        result = result && (index != std::string::npos);
    }
    TestUtils::PublishEvent(g_EVENT_RESP_FIRST, code, std::to_string(result));
};
void MainAbility::HybridCase10(int code)
{
    APP_LOGI("-- -- -- -- -- --MainAbility::%{public}s", __FUNCTION__);
    Reset();
    auto context = GetContext();
    TaskList globalDispatcher = TaskList {TestDispatcher::GLOBAL, context, "global"};
    TaskList parallelDispatcher1 = TaskList {TestDispatcher::PARALLEL, context, "parallel1"};
    TaskList parallelDispatcher2 = TaskList {TestDispatcher::PARALLEL, context, "parallel2"};
    TaskList serialDispatcher1 = TaskList {TestDispatcher::SERIAL, context, "serial1"};
    TaskList serialDispatcher2 = TaskList {TestDispatcher::SERIAL, context, "serial2"};
    TaskList mainDispatcher = TaskList {TestDispatcher::MAIN, context, "main"};
    int taskId = 0;
    const long delay = 10;
    globalDispatcher.addOperation(TestOperation::CREATE_GROUP)
        .addOperation(TestOperation::ASYNC_GROUP)
        .addFunc(std::make_shared<Runnable>([=]() { TestTask(std::to_string(taskId)); }))
        .executedTask();
    taskId++;
    parallelDispatcher1.addOperation(TestOperation::CREATE_GROUP)
        .addOperation(TestOperation::ASYNC_GROUP)
        .addFunc(std::make_shared<Runnable>([=]() { TestTask(std::to_string(taskId)); }))
        .executedTask();
    taskId++;
    parallelDispatcher2.addOperation(TestOperation::CREATE_GROUP)
        .addOperation(TestOperation::ASYNC_GROUP)
        .addFunc(std::make_shared<Runnable>([=]() { TestTask(std::to_string(taskId)); }))
        .executedTask();
    taskId++;
    serialDispatcher1.addOperation(TestOperation::DELAY)
        .addFunc(std::make_shared<Runnable>([=]() { TestTask(std::to_string(taskId)); }))
        .addDelay(delay)
        .executedTask();
    taskId++;
    serialDispatcher2.addOperation(TestOperation::DELAY)
        .addFunc(std::make_shared<Runnable>([=]() { TestTask(std::to_string(taskId)); }))
        .addDelay(delay)
        .executedTask();
    taskId++;
    mainDispatcher.addOperation(TestOperation::DELAY)
        .addFunc(std::make_shared<Runnable>([=]() { TestTask(std::to_string(taskId)); }))
        .addDelay(delay)
        .executedTask();
    taskId++;
    Wait(taskId);
    bool result = task_execution_sequence.size() > 1;
    std::vector<size_t> outerTaskIndex;
    std::vector<std::vector<size_t>> innerTaskIndex;
    GetTaskIndex(outerTaskIndex, innerTaskIndex, taskId, 0);
    for (auto index : outerTaskIndex) {
        result = result && (index != std::string::npos);
    }
    TestUtils::PublishEvent(g_EVENT_RESP_FIRST, code, std::to_string(result));
};
void MainAbility::HybridCase11(int code)
{
    APP_LOGI("-- -- -- -- -- --MainAbility::%{public}s", __FUNCTION__);
    Reset();
    auto context = GetContext();
    TaskList globalDispatcher = TaskList {TestDispatcher::GLOBAL, context, "global"};
    TaskList parallelDispatcher1 = TaskList {TestDispatcher::PARALLEL, context, "parallel1"};
    TaskList parallelDispatcher2 = TaskList {TestDispatcher::PARALLEL, context, "parallel2"};
    TaskList serialDispatcher1 = TaskList {TestDispatcher::SERIAL, context, "serial1"};
    TaskList serialDispatcher2 = TaskList {TestDispatcher::SERIAL, context, "serial2"};
    TaskList mainDispatcher = TaskList {TestDispatcher::MAIN, context, "main"};
    int taskId = 0;
    globalDispatcher.addOperation(TestOperation::CREATE_GROUP)
        .addOperation(TestOperation::ASYNC_GROUP)
        .addFunc(std::make_shared<Runnable>([=]() { TestTask(std::to_string(taskId)); }))
        .addOperation(TestOperation::GROUP_NOTIFY)
        .addFunc(std::make_shared<Runnable>([=]() { TestTask(std::to_string(taskId + 1)); }))
        .executedTask();
    taskId += numTwo;
    parallelDispatcher1.addOperation(TestOperation::CREATE_GROUP)
        .addOperation(TestOperation::ASYNC_GROUP)
        .addFunc(std::make_shared<Runnable>([=]() { TestTask(std::to_string(taskId)); }))
        .addOperation(TestOperation::GROUP_NOTIFY)
        .addFunc(std::make_shared<Runnable>([=]() { TestTask(std::to_string(taskId + 1)); }))
        .executedTask();
    taskId += numTwo;
    parallelDispatcher2.addOperation(TestOperation::CREATE_GROUP)
        .addOperation(TestOperation::ASYNC_GROUP)
        .addFunc(std::make_shared<Runnable>([=]() { TestTask(std::to_string(taskId)); }))
        .addOperation(TestOperation::GROUP_NOTIFY)
        .addFunc(std::make_shared<Runnable>([=]() { TestTask(std::to_string(taskId + 1)); }))
        .executedTask();
    taskId += numTwo;
    serialDispatcher1.addOperation(TestOperation::ASYNC)
        .addFunc(std::make_shared<Runnable>([=]() { TestTask(std::to_string(taskId)); }))
        .executedTask();
    taskId++;
    serialDispatcher2.addOperation(TestOperation::ASYNC)
        .addFunc(std::make_shared<Runnable>([=]() { TestTask(std::to_string(taskId)); }))
        .executedTask();
    taskId++;
    mainDispatcher.addOperation(TestOperation::ASYNC)
        .addFunc(std::make_shared<Runnable>([=]() { TestTask(std::to_string(taskId)); }))
        .executedTask();
    taskId++;
    Wait(taskId);
    bool result = task_execution_sequence.size() > 1;
    std::vector<size_t> outerTaskIndex;
    std::vector<std::vector<size_t>> innerTaskIndex;
    GetTaskIndex(outerTaskIndex, innerTaskIndex, taskId, 0);
    for (auto index : outerTaskIndex) {
        result = result && (index != std::string::npos);
    }
    TestUtils::PublishEvent(g_EVENT_RESP_FIRST, code, std::to_string(result));
};
void MainAbility::HybridCase12(int code)
{
    APP_LOGI("-- -- -- -- -- --MainAbility::%{public}s", __FUNCTION__);
    Reset();
    auto context = GetContext();
    TaskList globalDispatcher = TaskList {TestDispatcher::GLOBAL, context, "global"};
    TaskList parallelDispatcher1 = TaskList {TestDispatcher::PARALLEL, context, "parallel1"};
    TaskList parallelDispatcher2 = TaskList {TestDispatcher::PARALLEL, context, "parallel2"};
    TaskList serialDispatcher1 = TaskList {TestDispatcher::SERIAL, context, "serial1"};
    TaskList serialDispatcher2 = TaskList {TestDispatcher::SERIAL, context, "serial2"};
    TaskList mainDispatcher = TaskList {TestDispatcher::MAIN, context, "main"};
    int taskId = 0;
    int taskCount = 0;
    const int apply = 2;
    globalDispatcher.addOperation(TestOperation::APPLY)
        .addFunc(std::make_shared<Runnable>([=]() { TestTask(std::to_string(taskId)); }))
        .addApply(apply)
        .executedTask();
    taskId++;
    taskCount += apply;
    parallelDispatcher1.addOperation(TestOperation::APPLY)
        .addFunc(std::make_shared<Runnable>([=]() { TestTask(std::to_string(taskId)); }))
        .addApply(apply)
        .executedTask();
    taskId++;
    taskCount += apply;
    parallelDispatcher2.addOperation(TestOperation::APPLY)
        .addFunc(std::make_shared<Runnable>([=]() { TestTask(std::to_string(taskId)); }))
        .addApply(apply)
        .executedTask();
    taskId++;
    taskCount += apply;
    serialDispatcher1.addOperation(TestOperation::APPLY)
        .addFunc(std::make_shared<Runnable>([=]() { TestTask(std::to_string(taskId)); }))
        .addApply(apply)
        .executedTask();
    taskId++;
    taskCount += apply;
    serialDispatcher2.addOperation(TestOperation::APPLY)
        .addFunc(std::make_shared<Runnable>([=]() { TestTask(std::to_string(taskId)); }))
        .addApply(apply)
        .executedTask();
    taskId++;
    taskCount += apply;
    mainDispatcher.addOperation(TestOperation::APPLY)
        .addFunc(std::make_shared<Runnable>([=]() { TestTask(std::to_string(taskId)); }))
        .addApply(apply)
        .executedTask();
    taskId++;
    taskCount += apply;
    Wait(taskCount);
    bool result = task_execution_sequence.size() > 1;
    std::vector<size_t> outerTaskIndex;
    std::vector<std::vector<size_t>> innerTaskIndex;
    GetTaskIndex(outerTaskIndex, innerTaskIndex, taskId, 0);
    for (auto index : outerTaskIndex) {
        result = result && (index != std::string::npos);
    }
    TestUtils::PublishEvent(g_EVENT_RESP_FIRST, code, std::to_string(result));
};
void MainAbility::HybridCase13(int code)
{
    APP_LOGI("-- -- -- -- -- --MainAbility::%{public}s", __FUNCTION__);
    Reset();
    auto context = GetContext();
    TaskList globalDispatcher = TaskList {TestDispatcher::GLOBAL, context, "global"};
    TaskList parallelDispatcher1 = TaskList {TestDispatcher::PARALLEL, context, "parallel1"};
    TaskList parallelDispatcher2 = TaskList {TestDispatcher::PARALLEL, context, "parallel2"};
    TaskList serialDispatcher1 = TaskList {TestDispatcher::SERIAL, context, "serial1"};
    TaskList serialDispatcher2 = TaskList {TestDispatcher::SERIAL, context, "serial2"};
    TaskList mainDispatcher = TaskList {TestDispatcher::MAIN, context, "main"};
    int taskId = 0;
    int taskCount = 0;
    const int apply = 1;
    const int delay = 10;
    globalDispatcher.addOperation(TestOperation::DELAY)
        .addFunc(std::make_shared<Runnable>([=]() { TestTask(std::to_string(taskId)); }))
        .addDelay(delay)
        .executedTask();
    taskId++;
    taskCount += apply;
    parallelDispatcher1.addOperation(TestOperation::SYNC)
        .addFunc(std::make_shared<Runnable>([=]() { TestTask(std::to_string(taskId)); }))
        .executedTask();
    taskId++;
    taskCount += apply;
    parallelDispatcher2.addOperation(TestOperation::ASYNC)
        .addFunc(std::make_shared<Runnable>([=]() { TestTask(std::to_string(taskId)); }))
        .executedTask();
    taskId++;
    taskCount += apply;
    serialDispatcher1.addOperation(TestOperation::DELAY)
        .addFunc(std::make_shared<Runnable>([=]() { TestTask(std::to_string(taskId)); }))
        .addDelay(delay)
        .executedTask();
    taskId++;
    taskCount += apply;
    serialDispatcher2.addOperation(TestOperation::ASYNC)
        .addFunc(std::make_shared<Runnable>([=]() { TestTask(std::to_string(taskId)); }))
        .executedTask();
    taskId++;
    taskCount += apply;
    mainDispatcher.addOperation(TestOperation::SYNC)
        .addFunc(std::make_shared<Runnable>([=]() { TestTask(std::to_string(taskId)); }))
        .executedTask();
    taskId++;
    taskCount += apply;
    Wait(taskCount);
    bool result = task_execution_sequence.size() > 1;
    std::vector<size_t> outerTaskIndex;
    std::vector<std::vector<size_t>> innerTaskIndex;
    GetTaskIndex(outerTaskIndex, innerTaskIndex, taskId, 0);
    for (auto index : outerTaskIndex) {
        result = result && (index != std::string::npos);
    }
    TestUtils::PublishEvent(g_EVENT_RESP_FIRST, code, std::to_string(result));
};
void MainAbility::HybridCase14(int code)
{
    APP_LOGI("-- -- -- -- -- --MainAbility::%{public}s", __FUNCTION__);
    Reset();
    auto context = GetContext();
    TaskList globalDispatcher = TaskList {TestDispatcher::GLOBAL, context, "global"};
    TaskList parallelDispatcher1 = TaskList {TestDispatcher::PARALLEL, context, "parallel1"};
    TaskList parallelDispatcher2 = TaskList {TestDispatcher::PARALLEL, context, "parallel2"};
    TaskList serialDispatcher1 = TaskList {TestDispatcher::SERIAL, context, "serial1"};
    TaskList serialDispatcher2 = TaskList {TestDispatcher::SERIAL, context, "serial2"};
    TaskList mainDispatcher = TaskList {TestDispatcher::MAIN, context, "main"};
    int taskId = 0;
    int taskCount = 0;
    const int apply = 2;
    const int delay = 10;
    globalDispatcher.addOperation(TestOperation::CREATE_GROUP)
        .addOperation(TestOperation::ASYNC_GROUP)
        .addFunc(std::make_shared<Runnable>([=]() { TestTask(std::to_string(taskId)); }))
        .executedTask();
    taskId++;
    taskCount++;
    parallelDispatcher1.addOperation(TestOperation::SYNC)
        .addFunc(std::make_shared<Runnable>([=]() { TestTask(std::to_string(taskId)); }))
        .executedTask();
    taskId++;
    taskCount++;
    parallelDispatcher2.addOperation(TestOperation::DELAY)
        .addFunc(std::make_shared<Runnable>([=]() { TestTask(std::to_string(taskId)); }))
        .addDelay(delay)
        .executedTask();
    taskId++;
    taskCount++;
    serialDispatcher1.addOperation(TestOperation::APPLY)
        .addFunc(std::make_shared<Runnable>([=]() { TestTask(std::to_string(taskId)); }))
        .addApply(apply)
        .executedTask();
    taskId++;
    taskCount += apply;
    serialDispatcher2.addOperation(TestOperation::DELAY)
        .addFunc(std::make_shared<Runnable>([=]() { TestTask(std::to_string(taskId)); }))
        .addDelay(delay)
        .executedTask();
    taskId++;
    taskCount++;
    mainDispatcher.addOperation(TestOperation::ASYNC)
        .addFunc(std::make_shared<Runnable>([=]() { TestTask(std::to_string(taskId)); }))
        .executedTask();
    taskId++;
    taskCount++;
    Wait(taskCount);
    bool result = task_execution_sequence.size() > 1;
    std::vector<size_t> outerTaskIndex;
    std::vector<std::vector<size_t>> innerTaskIndex;
    GetTaskIndex(outerTaskIndex, innerTaskIndex, taskId, 0);
    for (auto index : outerTaskIndex) {
        result = result && (index != std::string::npos);
    }
    TestUtils::PublishEvent(g_EVENT_RESP_FIRST, code, std::to_string(result));
};
void MainAbility::HybridCase15(int code)
{
    APP_LOGI("-- -- -- -- -- --MainAbility::%{public}s", __FUNCTION__);
    Reset();
    auto context = GetContext();
    TaskList globalDispatcher = TaskList {TestDispatcher::GLOBAL, context, "global"};
    TaskList parallelDispatcher1 = TaskList {TestDispatcher::PARALLEL, context, "parallel1"};
    TaskList parallelDispatcher2 = TaskList {TestDispatcher::PARALLEL, context, "parallel2"};
    TaskList serialDispatcher1 = TaskList {TestDispatcher::SERIAL, context, "serial1"};
    TaskList serialDispatcher2 = TaskList {TestDispatcher::SERIAL, context, "serial2"};
    TaskList mainDispatcher = TaskList {TestDispatcher::MAIN, context, "main"};
    int taskId = 0;
    int taskCount = 0;
    const int apply = 2;
    const int delay = 10;
    globalDispatcher.addOperation(TestOperation::CREATE_GROUP)
        .addOperation(TestOperation::ASYNC_GROUP)
        .addFunc(std::make_shared<Runnable>([=]() { TestTask(std::to_string(taskId)); }))
        .addOperation(TestOperation::GROUP_NOTIFY)
        .addFunc(std::make_shared<Runnable>([=]() { TestTask(std::to_string(taskId + 1)); }))
        .executedTask();
    taskId++;
    taskCount += numTwo;
    parallelDispatcher1.addOperation(TestOperation::SYNC)
        .addFunc(std::make_shared<Runnable>([=]() { TestTask(std::to_string(taskId)); }))
        .executedTask();
    taskId++;
    taskCount++;
    parallelDispatcher2.addOperation(TestOperation::CREATE_GROUP)
        .addOperation(TestOperation::ASYNC_GROUP)
        .addFunc(std::make_shared<Runnable>([=]() { TestTask(std::to_string(taskId)); }))
        .executedTask();
    taskId++;
    taskCount++;
    serialDispatcher1.addOperation(TestOperation::ASYNC)
        .addFunc(std::make_shared<Runnable>([=]() { TestTask(std::to_string(taskId)); }))
        .executedTask();
    taskId++;
    taskCount++;
    serialDispatcher2.addOperation(TestOperation::APPLY)
        .addFunc(std::make_shared<Runnable>([=]() { TestTask(std::to_string(taskId)); }))
        .addApply(apply)
        .executedTask();
    taskId++;
    taskCount += apply;
    mainDispatcher.addOperation(TestOperation::DELAY)
        .addFunc(std::make_shared<Runnable>([=]() { TestTask(std::to_string(taskId)); }))
        .addDelay(delay)
        .executedTask();
    taskId++;
    taskCount++;
    Wait(taskCount);
    bool result = task_execution_sequence.size() > 1;
    std::vector<size_t> outerTaskIndex;
    std::vector<std::vector<size_t>> innerTaskIndex;
    GetTaskIndex(outerTaskIndex, innerTaskIndex, taskId, 0);
    for (auto index : outerTaskIndex) {
        result = result && (index != std::string::npos);
    }
    TestUtils::PublishEvent(g_EVENT_RESP_FIRST, code, std::to_string(result));
};
void MainAbility::HybridCase16(int code)
{
    APP_LOGI("-- -- -- -- -- --MainAbility::%{public}s", __FUNCTION__);
    Reset();
    auto context = GetContext();
    TaskList globalDispatcher = TaskList {TestDispatcher::GLOBAL, context, "global"};
    TaskList parallelDispatcher1 = TaskList {TestDispatcher::PARALLEL, context, "parallel1"};
    TaskList parallelDispatcher2 = TaskList {TestDispatcher::PARALLEL, context, "parallel2"};
    TaskList serialDispatcher1 = TaskList {TestDispatcher::SERIAL, context, "serial1"};
    TaskList serialDispatcher2 = TaskList {TestDispatcher::SERIAL, context, "serial2"};
    TaskList mainDispatcher = TaskList {TestDispatcher::MAIN, context, "main"};
    int taskId = 0;
    int taskCount = 0;
    const int apply = 2;
    const int delay = 10;
    globalDispatcher.addOperation(TestOperation::APPLY)
        .addFunc(std::make_shared<Runnable>([=]() { TestTask(std::to_string(taskId)); }))
        .addApply(apply)
        .executedTask();
    taskId++;
    taskCount += apply;
    parallelDispatcher1.addOperation(TestOperation::SYNC)
        .addFunc(std::make_shared<Runnable>([=]() { TestTask(std::to_string(taskId)); }))
        .executedTask();
    taskId++;
    taskCount++;
    parallelDispatcher2.addOperation(TestOperation::CREATE_GROUP)
        .addOperation(TestOperation::ASYNC_GROUP)
        .addFunc(std::make_shared<Runnable>([=]() { TestTask(std::to_string(taskId)); }))
        .addOperation(TestOperation::GROUP_NOTIFY)
        .addFunc(std::make_shared<Runnable>([=]() { TestTask(std::to_string(taskId + 1)); }))
        .executedTask();
    taskId++;
    taskCount += numTwo;
    serialDispatcher1.addOperation(TestOperation::ASYNC)
        .addFunc(std::make_shared<Runnable>([=]() { TestTask(std::to_string(taskId)); }))
        .executedTask();
    taskId++;
    taskCount++;
    serialDispatcher2.addOperation(TestOperation::DELAY)
        .addFunc(std::make_shared<Runnable>([=]() { TestTask(std::to_string(taskId)); }))
        .addDelay(delay)
        .executedTask();
    taskId++;
    taskCount++;
    mainDispatcher.addOperation(TestOperation::APPLY)
        .addFunc(std::make_shared<Runnable>([=]() { TestTask(std::to_string(taskId)); }))
        .addApply(apply)
        .executedTask();
    taskId++;
    taskCount += apply;
    Wait(taskCount);
    bool result = task_execution_sequence.size() > 1;
    std::vector<size_t> outerTaskIndex;
    std::vector<std::vector<size_t>> innerTaskIndex;
    GetTaskIndex(outerTaskIndex, innerTaskIndex, taskId, 0);
    for (auto index : outerTaskIndex) {
        result = result && (index != std::string::npos);
    }
    TestUtils::PublishEvent(g_EVENT_RESP_FIRST, code, std::to_string(result));
};
void MainAbility::HybridCase17(int code)
{
    APP_LOGI("-- -- -- -- -- --MainAbility::%{public}s", __FUNCTION__);
    Reset();
    auto context = GetContext();
    TaskList globalDispatcher = TaskList {TestDispatcher::GLOBAL, context, "global"};
    TaskList parallelDispatcher1 = TaskList {TestDispatcher::PARALLEL, context, "parallel1"};
    TaskList parallelDispatcher2 = TaskList {TestDispatcher::PARALLEL, context, "parallel2"};
    TaskList serialDispatcher1 = TaskList {TestDispatcher::SERIAL, context, "serial1"};
    TaskList serialDispatcher2 = TaskList {TestDispatcher::SERIAL, context, "serial2"};
    TaskList mainDispatcher = TaskList {TestDispatcher::MAIN, context, "main"};
    int taskId = 0;
    int taskCount = 0;
    const int apply = 2;
    globalDispatcher.addOperation(TestOperation::ASYNC)
        .addFunc(std::make_shared<Runnable>([=]() { TestTask(std::to_string(taskId)); }))
        .executedTask();
    taskId++;
    taskCount++;
    parallelDispatcher1.addOperation(TestOperation::SYNC)
        .addFunc(std::make_shared<Runnable>([=]() { TestTask(std::to_string(taskId)); }))
        .executedTask();
    taskId++;
    taskCount++;
    parallelDispatcher2.addOperation(TestOperation::APPLY)
        .addFunc(std::make_shared<Runnable>([=]() { TestTask(std::to_string(taskId)); }))
        .addApply(apply)
        .executedTask();
    taskId++;
    taskCount += apply;
    serialDispatcher1.addOperation(TestOperation::ASYNC)
        .addFunc(std::make_shared<Runnable>([=]() { TestTask(std::to_string(taskId)); }))
        .executedTask();
    taskId++;
    taskCount++;
    serialDispatcher2.addOperation(TestOperation::ASYNC)
        .addFunc(std::make_shared<Runnable>([=]() { TestTask(std::to_string(taskId)); }))
        .executedTask();
    taskId++;
    taskCount++;
    mainDispatcher.addOperation(TestOperation::SYNC)
        .addFunc(std::make_shared<Runnable>([=]() { TestTask(std::to_string(taskId)); }))
        .executedTask();
    taskId++;
    taskCount++;
    Wait(taskCount);
    bool result = task_execution_sequence.size() > 1;
    std::vector<size_t> outerTaskIndex;
    std::vector<std::vector<size_t>> innerTaskIndex;
    GetTaskIndex(outerTaskIndex, innerTaskIndex, taskId, 0);
    for (auto index : outerTaskIndex) {
        result = result && (index != std::string::npos);
    }
    TestUtils::PublishEvent(g_EVENT_RESP_FIRST, code, std::to_string(result));
};
void MainAbility::HybridCase18(int code)
{
    APP_LOGI("-- -- -- -- -- --MainAbility::%{public}s", __FUNCTION__);
    Reset();
    auto context = GetContext();
    TaskList globalDispatcher = TaskList {TestDispatcher::GLOBAL, context, "global"};
    TaskList parallelDispatcher1 = TaskList {TestDispatcher::PARALLEL, context, "parallel1"};
    TaskList parallelDispatcher2 = TaskList {TestDispatcher::PARALLEL, context, "parallel2"};
    TaskList serialDispatcher1 = TaskList {TestDispatcher::SERIAL, context, "serial1"};
    TaskList serialDispatcher2 = TaskList {TestDispatcher::SERIAL, context, "serial2"};
    TaskList mainDispatcher = TaskList {TestDispatcher::MAIN, context, "main"};
    int taskId = 0;
    int taskCount = 0;
    const int apply = 2;
    const int delay = 10;
    globalDispatcher.addOperation(TestOperation::SYNC)
        .addFunc(std::make_shared<Runnable>([=]() { TestTask(std::to_string(taskId)); }))
        .executedTask();
    taskId++;
    taskCount++;
    parallelDispatcher1.addOperation(TestOperation::ASYNC)
        .addFunc(std::make_shared<Runnable>([=]() { TestTask(std::to_string(taskId)); }))
        .executedTask();
    taskId++;
    taskCount++;
    parallelDispatcher2.addOperation(TestOperation::DELAY)
        .addFunc(std::make_shared<Runnable>([=]() { TestTask(std::to_string(taskId)); }))
        .addDelay(delay)
        .executedTask();
    taskId++;
    taskCount++;
    serialDispatcher1.addOperation(TestOperation::APPLY)
        .addFunc(std::make_shared<Runnable>([=]() { TestTask(std::to_string(taskId)); }))
        .addApply(apply)
        .executedTask();
    taskId++;
    taskCount += apply;
    serialDispatcher2.addOperation(TestOperation::ASYNC)
        .addFunc(std::make_shared<Runnable>([=]() { TestTask(std::to_string(taskId)); }))
        .executedTask();
    taskId++;
    taskCount++;
    mainDispatcher.addOperation(TestOperation::ASYNC)
        .addFunc(std::make_shared<Runnable>([=]() { TestTask(std::to_string(taskId)); }))
        .executedTask();
    taskId++;
    taskCount++;
    Wait(taskCount);
    bool result = task_execution_sequence.size() > 1;
    std::vector<size_t> outerTaskIndex;
    std::vector<std::vector<size_t>> innerTaskIndex;
    GetTaskIndex(outerTaskIndex, innerTaskIndex, taskId, 0);
    for (auto index : outerTaskIndex) {
        result = result && (index != std::string::npos);
    }
    TestUtils::PublishEvent(g_EVENT_RESP_FIRST, code, std::to_string(result));
};
void MainAbility::HybridCase19(int code)
{
    APP_LOGI("-- -- -- -- -- --MainAbility::%{public}s", __FUNCTION__);
    Reset();
    auto context = GetContext();
    TaskList globalDispatcher = TaskList {TestDispatcher::GLOBAL, context, "global"};
    TaskList parallelDispatcher1 = TaskList {TestDispatcher::PARALLEL, context, "parallel1"};
    TaskList parallelDispatcher2 = TaskList {TestDispatcher::PARALLEL, context, "parallel2"};
    TaskList serialDispatcher1 = TaskList {TestDispatcher::SERIAL, context, "serial1"};
    TaskList serialDispatcher2 = TaskList {TestDispatcher::SERIAL, context, "serial2"};
    TaskList mainDispatcher = TaskList {TestDispatcher::MAIN, context, "main"};
    int taskId = 0;
    int taskCount = 0;
    const int apply = 2;
    const int delay = 10;
    globalDispatcher.addOperation(TestOperation::DELAY)
        .addFunc(std::make_shared<Runnable>([=]() { TestTask(std::to_string(taskId)); }))
        .addDelay(delay)
        .executedTask();
    taskId++;
    taskCount++;
    parallelDispatcher1.addOperation(TestOperation::ASYNC)
        .addFunc(std::make_shared<Runnable>([=]() { TestTask(std::to_string(taskId)); }))
        .executedTask();
    taskId++;
    taskCount++;
    parallelDispatcher2.addOperation(TestOperation::CREATE_GROUP)
        .addOperation(TestOperation::ASYNC_GROUP)
        .addFunc(std::make_shared<Runnable>([=]() { TestTask(std::to_string(taskId)); }))
        .executedTask();
    taskId++;
    taskCount++;
    serialDispatcher1.addOperation(TestOperation::SYNC)
        .addFunc(std::make_shared<Runnable>([=]() { TestTask(std::to_string(taskId)); }))
        .executedTask();
    taskId++;
    taskCount += apply;
    serialDispatcher2.addOperation(TestOperation::ASYNC)
        .addFunc(std::make_shared<Runnable>([=]() { TestTask(std::to_string(taskId)); }))
        .executedTask();
    taskId++;
    taskCount++;
    mainDispatcher.addOperation(TestOperation::DELAY)
        .addFunc(std::make_shared<Runnable>([=]() { TestTask(std::to_string(taskId)); }))
        .addDelay(delay)
        .executedTask();
    taskId++;
    taskCount++;
    Wait(taskCount);
    bool result = task_execution_sequence.size() > 1;
    std::vector<size_t> outerTaskIndex;
    std::vector<std::vector<size_t>> innerTaskIndex;
    GetTaskIndex(outerTaskIndex, innerTaskIndex, taskId, 0);
    for (auto index : outerTaskIndex) {
        result = result && (index != std::string::npos);
    }
    TestUtils::PublishEvent(g_EVENT_RESP_FIRST, code, std::to_string(result));
};
void MainAbility::HybridCase20(int code)
{
    APP_LOGI("-- -- -- -- -- --MainAbility::%{public}s", __FUNCTION__);
    Reset();
    auto context = GetContext();
    TaskList globalDispatcher = TaskList {TestDispatcher::GLOBAL, context, "global"};
    TaskList parallelDispatcher1 = TaskList {TestDispatcher::PARALLEL, context, "parallel1"};
    TaskList parallelDispatcher2 = TaskList {TestDispatcher::PARALLEL, context, "parallel2"};
    TaskList serialDispatcher1 = TaskList {TestDispatcher::SERIAL, context, "serial1"};
    TaskList serialDispatcher2 = TaskList {TestDispatcher::SERIAL, context, "serial2"};
    TaskList mainDispatcher = TaskList {TestDispatcher::MAIN, context, "main"};
    int taskId = 0;
    int taskCount = 0;
    const int apply = 2;
    const int delay = 10;
    globalDispatcher.addOperation(TestOperation::CREATE_GROUP)
        .addOperation(TestOperation::ASYNC_GROUP)
        .addFunc(std::make_shared<Runnable>([=]() { TestTask(std::to_string(taskId)); }))
        .executedTask();
    taskId++;
    taskCount++;
    parallelDispatcher1.addOperation(TestOperation::ASYNC)
        .addFunc(std::make_shared<Runnable>([=]() { TestTask(std::to_string(taskId)); }))
        .executedTask();
    taskId++;
    taskCount++;
    parallelDispatcher2.addOperation(TestOperation::CREATE_GROUP)
        .addOperation(TestOperation::ASYNC_GROUP)
        .addFunc(std::make_shared<Runnable>([=]() { TestTask(std::to_string(taskId)); }))
        .addOperation(TestOperation::GROUP_NOTIFY)
        .addFunc(std::make_shared<Runnable>([=]() { TestTask(std::to_string(taskId + 1)); }))
        .executedTask();
    taskId++;
    taskCount += numTwo;
    serialDispatcher1.addOperation(TestOperation::DELAY)
        .addFunc(std::make_shared<Runnable>([=]() { TestTask(std::to_string(taskId)); }))
        .addDelay(delay)
        .executedTask();
    taskId++;
    taskCount++;
    serialDispatcher2.addOperation(TestOperation::SYNC)
        .addFunc(std::make_shared<Runnable>([=]() { TestTask(std::to_string(taskId)); }))
        .executedTask();
    taskId++;
    taskCount++;
    mainDispatcher.addOperation(TestOperation::APPLY)
        .addFunc(std::make_shared<Runnable>([=]() { TestTask(std::to_string(taskId)); }))
        .addApply(apply)
        .executedTask();
    taskId++;
    taskCount += apply;
    Wait(taskCount);
    bool result = task_execution_sequence.size() > 1;
    std::vector<size_t> outerTaskIndex;
    std::vector<std::vector<size_t>> innerTaskIndex;
    GetTaskIndex(outerTaskIndex, innerTaskIndex, taskId, 0);
    for (auto index : outerTaskIndex) {
        result = result && (index != std::string::npos);
    }
    TestUtils::PublishEvent(g_EVENT_RESP_FIRST, code, std::to_string(result));
};
void MainAbility::HybridCase21(int code)
{
    APP_LOGI("-- -- -- -- -- --MainAbility::%{public}s", __FUNCTION__);
    Reset();
    auto context = GetContext();
    TaskList globalDispatcher = TaskList {TestDispatcher::GLOBAL, context, "global"};
    TaskList parallelDispatcher1 = TaskList {TestDispatcher::PARALLEL, context, "parallel1"};
    TaskList parallelDispatcher2 = TaskList {TestDispatcher::PARALLEL, context, "parallel2"};
    TaskList serialDispatcher1 = TaskList {TestDispatcher::SERIAL, context, "serial1"};
    TaskList serialDispatcher2 = TaskList {TestDispatcher::SERIAL, context, "serial2"};
    TaskList mainDispatcher = TaskList {TestDispatcher::MAIN, context, "main"};
    int taskId = 0;
    int taskCount = 0;
    const int apply = 2;
    const int delay = 10;
    globalDispatcher.addOperation(TestOperation::CREATE_GROUP)
        .addOperation(TestOperation::ASYNC_GROUP)
        .addFunc(std::make_shared<Runnable>([=]() { TestTask(std::to_string(taskId)); }))
        .addOperation(TestOperation::GROUP_NOTIFY)
        .addFunc(std::make_shared<Runnable>([=]() { TestTask(std::to_string(taskId + 1)); }))
        .executedTask();
    taskId++;
    taskCount += numTwo;
    parallelDispatcher1.addOperation(TestOperation::ASYNC)
        .addFunc(std::make_shared<Runnable>([=]() { TestTask(std::to_string(taskId)); }))
        .executedTask();
    taskId++;
    taskCount++;
    parallelDispatcher2.addOperation(TestOperation::APPLY)
        .addFunc(std::make_shared<Runnable>([=]() { TestTask(std::to_string(taskId)); }))
        .addApply(apply)
        .executedTask();
    taskId++;
    taskCount += apply;
    serialDispatcher1.addOperation(TestOperation::DELAY)
        .addFunc(std::make_shared<Runnable>([=]() { TestTask(std::to_string(taskId)); }))
        .addDelay(delay)
        .executedTask();
    taskId++;
    taskCount++;
    serialDispatcher2.addOperation(TestOperation::ASYNC)
        .addFunc(std::make_shared<Runnable>([=]() { TestTask(std::to_string(taskId)); }))
        .executedTask();
    taskId++;
    taskCount++;
    mainDispatcher.addOperation(TestOperation::SYNC)
        .addFunc(std::make_shared<Runnable>([=]() { TestTask(std::to_string(taskId)); }))
        .executedTask();
    taskId++;
    taskCount++;
    Wait(taskCount);
    bool result = task_execution_sequence.size() > 1;
    std::vector<size_t> outerTaskIndex;
    std::vector<std::vector<size_t>> innerTaskIndex;
    GetTaskIndex(outerTaskIndex, innerTaskIndex, taskId, 0);
    for (auto index : outerTaskIndex) {
        result = result && (index != std::string::npos);
    }
    TestUtils::PublishEvent(g_EVENT_RESP_FIRST, code, std::to_string(result));
};
void MainAbility::HybridCase22(int code)
{
    APP_LOGI("-- -- -- -- -- --MainAbility::%{public}s", __FUNCTION__);
    Reset();
    auto context = GetContext();
    TaskList globalDispatcher = TaskList {TestDispatcher::GLOBAL, context, "global"};
    TaskList parallelDispatcher1 = TaskList {TestDispatcher::PARALLEL, context, "parallel1"};
    TaskList parallelDispatcher2 = TaskList {TestDispatcher::PARALLEL, context, "parallel2"};
    TaskList serialDispatcher1 = TaskList {TestDispatcher::SERIAL, context, "serial1"};
    TaskList serialDispatcher2 = TaskList {TestDispatcher::SERIAL, context, "serial2"};
    TaskList mainDispatcher = TaskList {TestDispatcher::MAIN, context, "main"};
    int taskId = 0;
    int taskCount = 0;
    const int apply = 2;
    const int delay = 10;
    globalDispatcher.addOperation(TestOperation::APPLY)
        .addFunc(std::make_shared<Runnable>([=]() { TestTask(std::to_string(taskId)); }))
        .addApply(apply)
        .executedTask();
    taskId++;
    taskCount += apply;
    parallelDispatcher1.addOperation(TestOperation::DELAY)
        .addFunc(std::make_shared<Runnable>([=]() { TestTask(std::to_string(taskId)); }))
        .addDelay(delay)
        .executedTask();
    taskId++;
    taskCount++;
    parallelDispatcher2.addOperation(TestOperation::CREATE_GROUP)
        .addOperation(TestOperation::ASYNC_GROUP)
        .addFunc(std::make_shared<Runnable>([=]() { TestTask(std::to_string(taskId)); }))
        .addApply(apply)
        .executedTask();
    taskId++;
    taskCount++;
    serialDispatcher1.addOperation(TestOperation::SYNC)
        .addFunc(std::make_shared<Runnable>([=]() { TestTask(std::to_string(taskId)); }))
        .executedTask();
    taskId++;
    taskCount++;
    serialDispatcher2.addOperation(TestOperation::ASYNC)
        .addFunc(std::make_shared<Runnable>([=]() { TestTask(std::to_string(taskId)); }))
        .executedTask();
    taskId++;
    taskCount++;
    mainDispatcher.addOperation(TestOperation::DELAY)
        .addFunc(std::make_shared<Runnable>([=]() { TestTask(std::to_string(taskId)); }))
        .addDelay(delay)
        .executedTask();
    taskId++;
    taskCount++;
    Wait(taskCount);
    bool result = task_execution_sequence.size() > 1;
    std::vector<size_t> outerTaskIndex;
    std::vector<std::vector<size_t>> innerTaskIndex;
    GetTaskIndex(outerTaskIndex, innerTaskIndex, taskId, 0);
    for (auto index : outerTaskIndex) {
        result = result && (index != std::string::npos);
    }
    TestUtils::PublishEvent(g_EVENT_RESP_FIRST, code, std::to_string(result));
};
void MainAbility::HybridCase23(int code)
{
    APP_LOGI("-- -- -- -- -- --MainAbility::%{public}s", __FUNCTION__);
    Reset();
    auto context = GetContext();
    TaskList globalDispatcher = TaskList {TestDispatcher::GLOBAL, context, "global"};
    TaskList parallelDispatcher1 = TaskList {TestDispatcher::PARALLEL, context, "parallel1"};
    TaskList parallelDispatcher2 = TaskList {TestDispatcher::PARALLEL, context, "parallel2"};
    TaskList serialDispatcher1 = TaskList {TestDispatcher::SERIAL, context, "serial1"};
    TaskList serialDispatcher2 = TaskList {TestDispatcher::SERIAL, context, "serial2"};
    TaskList mainDispatcher = TaskList {TestDispatcher::MAIN, context, "main"};
    int taskId = 0;
    int taskCount = 0;
    const int apply = 2;
    const int delay = 10;
    globalDispatcher.addOperation(TestOperation::CREATE_GROUP)
        .addOperation(TestOperation::ASYNC_GROUP)
        .addFunc(std::make_shared<Runnable>([=]() { TestTask(std::to_string(taskId)); }))
        .executedTask();
    taskId++;
    taskCount++;
    parallelDispatcher1.addOperation(TestOperation::DELAY)
        .addFunc(std::make_shared<Runnable>([=]() { TestTask(std::to_string(taskId)); }))
        .addDelay(delay)
        .executedTask();
    taskId++;
    taskCount++;
    parallelDispatcher2.addOperation(TestOperation::CREATE_GROUP)
        .addOperation(TestOperation::ASYNC_GROUP)
        .addFunc(std::make_shared<Runnable>([=]() { TestTask(std::to_string(taskId)); }))
        .addOperation(TestOperation::GROUP_NOTIFY)
        .addFunc(std::make_shared<Runnable>([=]() { TestTask(std::to_string(taskId + 1)); }))
        .executedTask();
    taskId++;
    taskCount += numTwo;
    serialDispatcher1.addOperation(TestOperation::APPLY)
        .addFunc(std::make_shared<Runnable>([=]() { TestTask(std::to_string(taskId)); }))
        .executedTask();
    taskId++;
    taskCount += apply;
    serialDispatcher2.addOperation(TestOperation::DELAY)
        .addFunc(std::make_shared<Runnable>([=]() { TestTask(std::to_string(taskId)); }))
        .addDelay(delay)
        .executedTask();
    taskId++;
    taskCount++;
    mainDispatcher.addOperation(TestOperation::DELAY)
        .addFunc(std::make_shared<Runnable>([=]() { TestTask(std::to_string(taskId)); }))
        .addDelay(delay)
        .executedTask();
    taskId++;
    taskCount++;
    Wait(taskCount);
    bool result = task_execution_sequence.size() > 1;
    std::vector<size_t> outerTaskIndex;
    std::vector<std::vector<size_t>> innerTaskIndex;
    GetTaskIndex(outerTaskIndex, innerTaskIndex, taskId, 0);
    for (auto index : outerTaskIndex) {
        result = result && (index != std::string::npos);
    }
    TestUtils::PublishEvent(g_EVENT_RESP_FIRST, code, std::to_string(result));
};
void MainAbility::HybridCase24(int code)
{
    APP_LOGI("-- -- -- -- -- --MainAbility::%{public}s", __FUNCTION__);
    Reset();
    auto context = GetContext();
    TaskList globalDispatcher = TaskList {TestDispatcher::GLOBAL, context, "global"};
    TaskList parallelDispatcher1 = TaskList {TestDispatcher::PARALLEL, context, "parallel1"};
    TaskList parallelDispatcher2 = TaskList {TestDispatcher::PARALLEL, context, "parallel2"};
    TaskList serialDispatcher1 = TaskList {TestDispatcher::SERIAL, context, "serial1"};
    TaskList serialDispatcher2 = TaskList {TestDispatcher::SERIAL, context, "serial2"};
    TaskList mainDispatcher = TaskList {TestDispatcher::MAIN, context, "main"};
    int taskId = 0;
    int taskCount = 0;
    const long apply = 2;
    const long delay = 10;
    globalDispatcher.addOperation(TestOperation::CREATE_GROUP)
        .addOperation(TestOperation::ASYNC_GROUP)
        .addFunc(std::make_shared<Runnable>([=]() { TestTask(std::to_string(taskId)); }))
        .addOperation(TestOperation::GROUP_NOTIFY)
        .addFunc(std::make_shared<Runnable>([=]() { TestTask(std::to_string(taskId + 1)); }))
        .executedTask();
    taskId += numTwo;
    taskCount += numTwo;
    parallelDispatcher1.addOperation(TestOperation::DELAY)
        .addFunc(std::make_shared<Runnable>([=]() { TestTask(std::to_string(taskId)); }))
        .addDelay(delay)
        .executedTask();
    taskId++;
    taskCount++;
    parallelDispatcher2.addOperation(TestOperation::APPLY)
        .addApply(apply)
        .addFunc(std::make_shared<Runnable>([=]() { TestTask(std::to_string(taskId)); }))
        .executedTask();
    taskId++;
    taskCount += apply;
    serialDispatcher1.addOperation(TestOperation::SYNC)
        .addFunc(std::make_shared<Runnable>([=]() { TestTask(std::to_string(taskId)); }))
        .executedTask();
    taskId++;
    taskCount++;
    serialDispatcher2.addOperation(TestOperation::APPLY)
        .addApply(apply)
        .addFunc(std::make_shared<Runnable>([=]() { TestTask(std::to_string(taskId)); }))
        .addOperation(TestOperation::DELAY)
        .addDelay(delay)
        .executedTask();
    taskId++;
    taskCount += apply;
    mainDispatcher.addOperation(TestOperation::APPLY)
        .addApply(apply)
        .addFunc(std::make_shared<Runnable>([=]() { TestTask(std::to_string(taskId)); }))
        .addOperation(TestOperation::DELAY)
        .addDelay(delay)
        .executedTask();
    taskId++;
    taskCount += apply;
    Wait(taskCount);
    bool result = task_execution_sequence.size() > 1;
    std::vector<size_t> outerTaskIndex;
    std::vector<std::vector<size_t>> innerTaskIndex;
    GetTaskIndex(outerTaskIndex, innerTaskIndex, taskId, 0);
    for (auto index : outerTaskIndex) {
        result = result && (index != std::string::npos);
    }
    TestUtils::PublishEvent(g_EVENT_RESP_FIRST, code, std::to_string(result));
};
void MainAbility::HybridCase25(int code)
{
    APP_LOGI("-- -- -- -- -- --MainAbility::%{public}s", __FUNCTION__);
    Reset();
    auto context = GetContext();
    TaskList globalDispatcher = TaskList {TestDispatcher::GLOBAL, context, "global"};
    TaskList parallelDispatcher1 = TaskList {TestDispatcher::PARALLEL, context, "parallel1"};
    TaskList parallelDispatcher2 = TaskList {TestDispatcher::PARALLEL, context, "parallel2"};
    TaskList serialDispatcher1 = TaskList {TestDispatcher::SERIAL, context, "serial1"};
    TaskList serialDispatcher2 = TaskList {TestDispatcher::SERIAL, context, "serial2"};
    TaskList mainDispatcher = TaskList {TestDispatcher::MAIN, context, "main"};
    int taskId = 0;
    int taskCount = 0;
    globalDispatcher.addOperation(TestOperation::ASYNC)
        .addFunc(std::make_shared<Runnable>([=]() { TestTask(std::to_string(taskId)); }))
        .executedTask();
    taskId++;
    taskCount++;
    parallelDispatcher1.addOperation(TestOperation::CREATE_GROUP)
        .addOperation(TestOperation::ASYNC_GROUP)
        .addFunc(std::make_shared<Runnable>([=]() { TestTask(std::to_string(taskId)); }))
        .executedTask();
    taskId++;
    taskCount++;
    parallelDispatcher2.addOperation(TestOperation::CREATE_GROUP)
        .addOperation(TestOperation::ASYNC_GROUP)
        .addFunc(std::make_shared<Runnable>([=]() { TestTask(std::to_string(taskId)); }))
        .addOperation(TestOperation::GROUP_NOTIFY)
        .addFunc(std::make_shared<Runnable>([=]() { TestTask(std::to_string(taskId + 1)); }))
        .executedTask();
    taskId++;
    taskCount += numTwo;
    serialDispatcher1.addOperation(TestOperation::ASYNC)
        .addFunc(std::make_shared<Runnable>([=]() { TestTask(std::to_string(taskId)); }))
        .executedTask();
    taskId++;
    taskCount++;
    serialDispatcher2.addOperation(TestOperation::SYNC)
        .addFunc(std::make_shared<Runnable>([=]() { TestTask(std::to_string(taskId)); }))
        .executedTask();
    taskId++;
    taskCount++;
    mainDispatcher.addOperation(TestOperation::SYNC)
        .addFunc(std::make_shared<Runnable>([=]() { TestTask(std::to_string(taskId)); }))
        .executedTask();
    taskId++;
    taskCount++;
    Wait(taskCount);
    bool result = task_execution_sequence.size() > 1;
    std::vector<size_t> outerTaskIndex;
    std::vector<std::vector<size_t>> innerTaskIndex;
    GetTaskIndex(outerTaskIndex, innerTaskIndex, taskId, 0);
    for (auto index : outerTaskIndex) {
        result = result && (index != std::string::npos);
    }
    TestUtils::PublishEvent(g_EVENT_RESP_FIRST, code, std::to_string(result));
};
void MainAbility::HybridCase26(int code)
{
    APP_LOGI("-- -- -- -- -- --MainAbility::%{public}s", __FUNCTION__);
    Reset();
    auto context = GetContext();
    TaskList globalDispatcher = TaskList {TestDispatcher::GLOBAL, context, "global"};
    TaskList parallelDispatcher1 = TaskList {TestDispatcher::PARALLEL, context, "parallel1"};
    TaskList parallelDispatcher2 = TaskList {TestDispatcher::PARALLEL, context, "parallel2"};
    TaskList serialDispatcher1 = TaskList {TestDispatcher::SERIAL, context, "serial1"};
    TaskList serialDispatcher2 = TaskList {TestDispatcher::SERIAL, context, "serial2"};
    TaskList mainDispatcher = TaskList {TestDispatcher::MAIN, context, "main"};
    int taskId = 0;
    int taskCount = 0;
    const int apply = 2;
    globalDispatcher.addOperation(TestOperation::ASYNC)
        .addFunc(std::make_shared<Runnable>([=]() { TestTask(std::to_string(taskId)); }))
        .executedTask();
    taskId++;
    taskCount++;
    parallelDispatcher1.addOperation(TestOperation::CREATE_GROUP)
        .addOperation(TestOperation::ASYNC_GROUP)
        .addFunc(std::make_shared<Runnable>([=]() { TestTask(std::to_string(taskId)); }))
        .executedTask();
    taskId++;
    taskCount++;
    parallelDispatcher2.addOperation(TestOperation::APPLY)
        .addFunc(std::make_shared<Runnable>([=]() { TestTask(std::to_string(taskId)); }))
        .addApply(apply)
        .executedTask();
    taskId++;
    taskCount = taskCount + apply;
    serialDispatcher1.addOperation(TestOperation::ASYNC)
        .addFunc(std::make_shared<Runnable>([=]() { TestTask(std::to_string(taskId)); }))
        .executedTask();
    taskId++;
    taskCount++;
    serialDispatcher2.addOperation(TestOperation::SYNC)
        .addFunc(std::make_shared<Runnable>([=]() { TestTask(std::to_string(taskId)); }))
        .executedTask();
    taskId++;
    taskCount++;
    mainDispatcher.addOperation(TestOperation::ASYNC)
        .addFunc(std::make_shared<Runnable>([=]() { TestTask(std::to_string(taskId)); }))
        .executedTask();
    taskId++;
    taskCount++;
    Wait(taskCount);
    bool result = task_execution_sequence.size() > 1;
    std::vector<size_t> outerTaskIndex;
    std::vector<std::vector<size_t>> innerTaskIndex;
    GetTaskIndex(outerTaskIndex, innerTaskIndex, taskId, 0);
    for (auto index : outerTaskIndex) {
        result = result && (index != std::string::npos);
    }
    TestUtils::PublishEvent(g_EVENT_RESP_FIRST, code, std::to_string(result));
};
void MainAbility::HybridCase27(int code)
{
    APP_LOGI("-- -- -- -- -- --MainAbility::%{public}s", __FUNCTION__);
    Reset();
    auto context = GetContext();
    TaskList globalDispatcher = TaskList {TestDispatcher::GLOBAL, context, "global"};
    TaskList parallelDispatcher1 = TaskList {TestDispatcher::PARALLEL, context, "parallel1"};
    TaskList parallelDispatcher2 = TaskList {TestDispatcher::PARALLEL, context, "parallel2"};
    TaskList serialDispatcher1 = TaskList {TestDispatcher::SERIAL, context, "serial1"};
    TaskList serialDispatcher2 = TaskList {TestDispatcher::SERIAL, context, "serial2"};
    TaskList mainDispatcher = TaskList {TestDispatcher::MAIN, context, "main"};
    int taskId = 0;
    int taskCount = 0;
    const int apply = 2;
    const int delay = 10;
    globalDispatcher.addOperation(TestOperation::ASYNC)
        .addFunc(std::make_shared<Runnable>([=]() { TestTask(std::to_string(taskId)); }))
        .executedTask();
    taskId++;
    taskCount++;
    parallelDispatcher1.addOperation(TestOperation::CREATE_GROUP)
        .addOperation(TestOperation::ASYNC_GROUP)
        .addFunc(std::make_shared<Runnable>([=]() { TestTask(std::to_string(taskId)); }))
        .addOperation(TestOperation::GROUP_NOTIFY)
        .addFunc(std::make_shared<Runnable>([=]() { TestTask(std::to_string(taskId + 1)); }))
        .executedTask();
    taskId++;
    taskCount += numTwo;
    parallelDispatcher2.addOperation(TestOperation::APPLY)
        .addFunc(std::make_shared<Runnable>([=]() { TestTask(std::to_string(taskId)); }))
        .addApply(apply)
        .executedTask();
    taskId++;
    taskCount += apply;
    serialDispatcher1.addOperation(TestOperation::DELAY)
        .addFunc(std::make_shared<Runnable>([=]() { TestTask(std::to_string(taskId)); }))
        .addDelay(delay)
        .executedTask();
    taskId++;
    taskCount++;
    serialDispatcher2.addOperation(TestOperation::APPLY)
        .addFunc(std::make_shared<Runnable>([=]() { TestTask(std::to_string(taskId)); }))
        .addApply(apply)
        .executedTask();
    taskId++;
    taskCount += apply;
    mainDispatcher.addOperation(TestOperation::DELAY)
        .addFunc(std::make_shared<Runnable>([=]() { TestTask(std::to_string(taskId)); }))
        .addDelay(delay)
        .executedTask();
    taskId++;
    taskCount++;
    Wait(taskCount);
    bool result = task_execution_sequence.size() > 1;
    std::vector<size_t> outerTaskIndex;
    std::vector<std::vector<size_t>> innerTaskIndex;
    GetTaskIndex(outerTaskIndex, innerTaskIndex, taskId, 0);
    for (auto index : outerTaskIndex) {
        result = result && (index != std::string::npos);
    }
    TestUtils::PublishEvent(g_EVENT_RESP_FIRST, code, std::to_string(result));
};

void MainAbility::MultiAppCase1(int code)
{
    APP_LOGI("-- -- -- -- -- --MainAbility::%{public}s", __FUNCTION__);
    Reset();
    auto context = GetContext();
    TaskList globalDispatcher = TaskList {TestDispatcher::GLOBAL, context, "global"};
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
    TestUtils::PublishEvent(g_EVENT_RESP_FIRST, code, std::to_string(result));
}

void MainAbility::MultiAppCase2(int code)
{
    APP_LOGI("-- -- -- -- -- --MainAbility::%{public}s", __FUNCTION__);
    Reset();
    auto context = GetContext();
    TaskList parallelDispatcher = TaskList {TestDispatcher::PARALLEL, context, "parallel"};
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
    addTaskFromList(parallelDispatcher, operationList, taskId);
    parallelDispatcher.executedTask();
    Wait(taskId);
    bool result = task_execution_sequence.size() > 1;
    std::vector<size_t> outerTaskIndex;
    std::vector<std::vector<size_t>> innerTaskIndex;
    GetTaskIndex(outerTaskIndex, innerTaskIndex, taskId, 0);
    for (auto index : outerTaskIndex) {
        result = result && (index != std::string::npos);
    }
    TestUtils::PublishEvent(g_EVENT_RESP_FIRST, code, std::to_string(result));
}

void MainAbility::MultiAppCase3(int code)
{
    APP_LOGI("-- -- -- -- -- --MainAbility::%{public}s", __FUNCTION__);
    Reset();
    auto context = GetContext();
    TaskList serialDispatcher = TaskList {TestDispatcher::SERIAL, context, "serial"};
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
    addTaskFromList(serialDispatcher, operationList, taskId);
    serialDispatcher.executedTask();
    Wait(taskId);
    bool result = task_execution_sequence.size() > 1;
    std::vector<size_t> outerTaskIndex;
    std::vector<std::vector<size_t>> innerTaskIndex;
    GetTaskIndex(outerTaskIndex, innerTaskIndex, taskId, 0);
    for (auto index : outerTaskIndex) {
        result = result && (index != std::string::npos);
    }
    TestUtils::PublishEvent(g_EVENT_RESP_FIRST, code, std::to_string(result));
}

void MainAbility::MultiAppCase4(int code)
{
    APP_LOGI("-- -- -- -- -- --MainAbility::%{public}s", __FUNCTION__);
    Reset();
    auto context = GetContext();
    TaskList mainDispatcher = TaskList {TestDispatcher::MAIN, context, "main"};
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
    addTaskFromList(mainDispatcher, operationList, taskId);
    mainDispatcher.executedTask();
    Wait(taskId);
    bool result = task_execution_sequence.size() > 1;
    std::vector<size_t> outerTaskIndex;
    std::vector<std::vector<size_t>> innerTaskIndex;
    GetTaskIndex(outerTaskIndex, innerTaskIndex, taskId, 0);
    for (auto index : outerTaskIndex) {
        result = result && (index != std::string::npos);
    }
    TestUtils::PublishEvent(g_EVENT_RESP_FIRST, code, std::to_string(result));
}

void MainAbility::ExtraCase1(int code)
{
    APP_LOGI("-- -- -- -- -- --MainAbility::%{public}s", __FUNCTION__);
    Reset();
    auto context = GetContext();
    int taskId = 0;
    TaskList globalDispatcher = TaskList {TestDispatcher::GLOBAL, context, "global"};
    bool result = globalDispatcher.addOperation(TestOperation::SYNC)
        .addFunc(std::make_shared<Runnable>([=]() {
            std::string targetBundleName = "com.ohos.TaskDispatcherA";
            std::string targetAbility = "SecondAbility";
            Want want;
            want.SetElementName(targetBundleName, targetAbility);
            StartAbility(want);
            TestTask(std::to_string(taskId));
        }))
        .executedTask();
    taskId++;
    Wait(taskId);
    result = task_execution_sequence.size() > 1;
    TestUtils::PublishEvent(g_EVENT_RESP_FIRST, code, std::to_string(result));
}

void MainAbility::FillInDispathcer()
{
    APP_LOGI("-- -- -- -- -- --MainAbility::%{public}s", __FUNCTION__);
    const int fullThreadNum = 32;
    const int waitTime = 2;
    auto context = GetContext();
    TaskList parallelDispatcher = TaskList {TestDispatcher::PARALLEL, context, "parallel"};
    parallelDispatcher.setTaskPriority(AppExecFwk::TaskPriority::HIGH);
    parallelDispatcher.addOperation(TestOperation::ASYNC).addFunc(std::make_shared<Runnable>([=]() {
        sleep(waitTime - 1);
    }));
    for (int i = 1; i < fullThreadNum; i++) {
        parallelDispatcher.addOperation(TestOperation::ASYNC).addFunc(std::make_shared<Runnable>([=]() {
            sleep(waitTime);
        }));
    }
    parallelDispatcher.executedTask();
}
void MainAbility::PriorityCase1(int code)
{
    APP_LOGI("-- -- -- -- -- --MainAbility::%{public}s", __FUNCTION__);
    FillInDispathcer();
    Reset();
    auto context = GetContext();
    TaskList globalDispatcherLow = TaskList {TestDispatcher::GLOBAL, context, "globalLow"};
    TaskList globalDispatcherDefault = TaskList {TestDispatcher::GLOBAL, context, "globalDefault"};
    TaskList globalDispatcherHigh = TaskList {TestDispatcher::GLOBAL, context, "globalHigh"};
    int taskId = 0;
    globalDispatcherLow.setTaskPriority(AppExecFwk::TaskPriority::LOW)
        .addOperation(TestOperation::ASYNC)
        .addFunc(std::make_shared<Runnable>([=]() { TestTask(std::to_string(taskId)); }))
        .executedTask();
    taskId++;
    globalDispatcherDefault.setTaskPriority(AppExecFwk::TaskPriority::DEFAULT)
        .addOperation(TestOperation::ASYNC)
        .addFunc(std::make_shared<Runnable>([=]() { TestTask(std::to_string(taskId)); }))
        .executedTask();
    taskId++;
    globalDispatcherHigh.setTaskPriority(AppExecFwk::TaskPriority::HIGH)
        .addOperation(TestOperation::ASYNC)
        .addFunc(std::make_shared<Runnable>([=]() { TestTask(std::to_string(taskId)); }))
        .executedTask();
    taskId++;
    Wait(taskId);
    bool result = task_execution_sequence.size() > 1;
    std::vector<size_t> outerTaskIndex;
    std::vector<std::vector<size_t>> innerTaskIndex;
    GetTaskIndex(outerTaskIndex, innerTaskIndex, taskId, 0);
    for (auto index : outerTaskIndex) {
        result = result && (index != std::string::npos);
    }
    result = result && (outerTaskIndex[numZero] > outerTaskIndex[numOne]) &&
             (outerTaskIndex[numOne] > outerTaskIndex[numTwo]);
    TestUtils::PublishEvent(g_EVENT_RESP_FIRST, code, std::to_string(result));
}
void MainAbility::PriorityCase2(int code)
{
    APP_LOGI("-- -- -- -- -- --MainAbility::%{public}s", __FUNCTION__);
    FillInDispathcer();
    Reset();
    auto context = GetContext();
    TaskList parallelDispatcherLow = TaskList {TestDispatcher::PARALLEL, context, "parallelLow"};
    TaskList parallelDispatcherDefault = TaskList {TestDispatcher::PARALLEL, context, "parallelDefault"};
    TaskList parallelDispatcherHigh = TaskList {TestDispatcher::PARALLEL, context, "parallelHigh"};
    int taskId = 0;
    parallelDispatcherLow.setTaskPriority(AppExecFwk::TaskPriority::LOW)
        .addOperation(TestOperation::ASYNC)
        .addFunc(std::make_shared<Runnable>([=]() { TestTask(std::to_string(taskId)); }))
        .executedTask();
    taskId++;
    parallelDispatcherDefault.setTaskPriority(AppExecFwk::TaskPriority::DEFAULT)
        .addOperation(TestOperation::ASYNC)
        .addFunc(std::make_shared<Runnable>([=]() { TestTask(std::to_string(taskId)); }))
        .executedTask();
    taskId++;
    parallelDispatcherHigh.setTaskPriority(AppExecFwk::TaskPriority::HIGH)
        .addOperation(TestOperation::ASYNC)
        .addFunc(std::make_shared<Runnable>([=]() { TestTask(std::to_string(taskId)); }))
        .executedTask();
    taskId++;
    Wait(taskId);
    bool result = task_execution_sequence.size() > 1;
    std::vector<size_t> outerTaskIndex;
    std::vector<std::vector<size_t>> innerTaskIndex;
    GetTaskIndex(outerTaskIndex, innerTaskIndex, taskId, 0);
    for (auto index : outerTaskIndex) {
        result = result && (index != std::string::npos);
    }
    result = result && (outerTaskIndex[numZero] > outerTaskIndex[numOne]) &&
             (outerTaskIndex[numOne] > outerTaskIndex[numTwo]);
    TestUtils::PublishEvent(g_EVENT_RESP_FIRST, code, std::to_string(result));
}
void MainAbility::PriorityCase3(int code)
{
    APP_LOGI("-- -- -- -- -- --MainAbility::%{public}s", __FUNCTION__);
    FillInDispathcer();
    Reset();
    auto context = GetContext();
    TaskList serialDispatcherLow = TaskList {TestDispatcher::SERIAL, context, "serialLow"};
    TaskList serialDispatcherDefault = TaskList {TestDispatcher::SERIAL, context, "serialDefault"};
    TaskList serialDispatcherHigh = TaskList {TestDispatcher::SERIAL, context, "serialHigh"};
    int taskId = 0;
    serialDispatcherLow.setTaskPriority(AppExecFwk::TaskPriority::LOW)
        .addOperation(TestOperation::ASYNC)
        .addFunc(std::make_shared<Runnable>([=]() { TestTask(std::to_string(taskId)); }))
        .executedTask();
    taskId++;
    serialDispatcherDefault.setTaskPriority(AppExecFwk::TaskPriority::DEFAULT)
        .addOperation(TestOperation::ASYNC)
        .addFunc(std::make_shared<Runnable>([=]() { TestTask(std::to_string(taskId)); }))
        .executedTask();
    taskId++;
    serialDispatcherHigh.setTaskPriority(AppExecFwk::TaskPriority::HIGH)
        .addOperation(TestOperation::ASYNC)
        .addFunc(std::make_shared<Runnable>([=]() { TestTask(std::to_string(taskId)); }))
        .executedTask();
    taskId++;
    Wait(taskId);
    bool result = task_execution_sequence.size() > 1;
    std::vector<size_t> outerTaskIndex;
    std::vector<std::vector<size_t>> innerTaskIndex;
    GetTaskIndex(outerTaskIndex, innerTaskIndex, taskId, 0);
    for (auto index : outerTaskIndex) {
        result = result && (index != std::string::npos);
    }
    result = result && (outerTaskIndex[numZero] > outerTaskIndex[numOne]) &&
             (outerTaskIndex[numOne] > outerTaskIndex[numTwo]);
    TestUtils::PublishEvent(g_EVENT_RESP_FIRST, code, std::to_string(result));
}
void MainAbility::PriorityCase4(int code)
{
    APP_LOGI("-- -- -- -- -- --MainAbility::%{public}s", __FUNCTION__);
    FillInDispathcer();
    Reset();
    auto context = GetContext();
    TaskList parallelDispatcherLow = TaskList {TestDispatcher::PARALLEL, context, "parallelLow"};
    TaskList parallelDispatcherDefault = TaskList {TestDispatcher::PARALLEL, context, "parallelDefault"};
    TaskList mainDispatcherHigh = TaskList {TestDispatcher::MAIN, context, "mainHigh"};
    int taskId = 0;
    parallelDispatcherLow.setTaskPriority(AppExecFwk::TaskPriority::LOW)
        .addOperation(TestOperation::ASYNC)
        .addFunc(std::make_shared<Runnable>([=]() { TestTask(std::to_string(taskId)); }))
        .executedTask();
    taskId++;
    parallelDispatcherDefault.setTaskPriority(AppExecFwk::TaskPriority::DEFAULT)
        .addOperation(TestOperation::ASYNC)
        .addFunc(std::make_shared<Runnable>([=]() { TestTask(std::to_string(taskId)); }))
        .executedTask();
    taskId++;
    mainDispatcherHigh.addOperation(TestOperation::ASYNC)
        .addFunc(std::make_shared<Runnable>([=]() { TestTask(std::to_string(taskId)); }))
        .executedTask();
    taskId++;
    Wait(taskId);
    bool result = task_execution_sequence.size() > 1;
    std::vector<size_t> outerTaskIndex;
    std::vector<std::vector<size_t>> innerTaskIndex;
    GetTaskIndex(outerTaskIndex, innerTaskIndex, taskId, 0);
    for (auto index : outerTaskIndex) {
        result = result && (index != std::string::npos);
    }
    result = result && (outerTaskIndex[numZero] > outerTaskIndex[numOne]) &&
             (outerTaskIndex[numOne] > outerTaskIndex[numTwo]);
    TestUtils::PublishEvent(g_EVENT_RESP_FIRST, code, std::to_string(result));
}
void MainAbility::PriorityCase5(int code)
{
    APP_LOGI("-- -- -- -- -- --MainAbility::%{public}s", __FUNCTION__);
    FillInDispathcer();
    Reset();
    auto context = GetContext();
    TaskList globalDispatcherLow = TaskList {TestDispatcher::GLOBAL, context, "globallLow"};
    TaskList parallelDispatcherDefault = TaskList {TestDispatcher::PARALLEL, context, "parallelDefault"};
    TaskList globalDispatcherHigh = TaskList {TestDispatcher::GLOBAL, context, "globalHigh"};
    int taskId = 0;
    globalDispatcherLow.setTaskPriority(AppExecFwk::TaskPriority::LOW)
        .addOperation(TestOperation::ASYNC)
        .addFunc(std::make_shared<Runnable>([=]() { TestTask(std::to_string(taskId)); }))
        .executedTask();
    taskId++;
    parallelDispatcherDefault.setTaskPriority(AppExecFwk::TaskPriority::DEFAULT)
        .addOperation(TestOperation::ASYNC)
        .addFunc(std::make_shared<Runnable>([=]() { TestTask(std::to_string(taskId)); }))
        .executedTask();
    taskId++;
    globalDispatcherHigh.setTaskPriority(AppExecFwk::TaskPriority::HIGH)
        .addOperation(TestOperation::ASYNC)
        .addFunc(std::make_shared<Runnable>([=]() { TestTask(std::to_string(taskId)); }))
        .executedTask();
    taskId++;
    Wait(taskId);
    bool result = task_execution_sequence.size() > 1;
    std::vector<size_t> outerTaskIndex;
    std::vector<std::vector<size_t>> innerTaskIndex;
    GetTaskIndex(outerTaskIndex, innerTaskIndex, taskId, 0);
    for (auto index : outerTaskIndex) {
        result = result && (index != std::string::npos);
    }
    result = result && (outerTaskIndex[numZero] > outerTaskIndex[numOne]) &&
             (outerTaskIndex[numOne] > outerTaskIndex[numTwo]);
    TestUtils::PublishEvent(g_EVENT_RESP_FIRST, code, std::to_string(result));
}
void MainAbility::RevokeCase1(int code)
{
    APP_LOGI("-- -- -- -- -- --MainAbility::%{public}s", __FUNCTION__);
    FillInDispathcer();
    Reset();
    auto context = GetContext();
    TaskList parallelDispatcher = TaskList {TestDispatcher::GLOBAL, context, "parallel"};
    int taskId = 0;
    bool result = parallelDispatcher.addOperation(TestOperation::ASYNC)
        .addFunc(std::make_shared<Runnable>([=]() { TestTask(std::to_string(taskId)); }))
        .addOperation(TestOperation::REVOCABLE)
        .addRevokeTask(1)
        .executedTask();
    taskId++;
    result = !result || Wait(taskId);
    result = result || task_execution_sequence.size() > 1;

    TestUtils::PublishEvent(g_EVENT_RESP_FIRST, code, std::to_string(!result));
}
void MainAbility::RevokeCase2(int code)
{
    APP_LOGI("-- -- -- -- -- --MainAbility::%{public}s", __FUNCTION__);
    Reset();
    auto context = GetContext();
    TaskList parallelDispatcher = TaskList {TestDispatcher::GLOBAL, context, "parallel"};
    int taskId = 0;
    bool result = parallelDispatcher.addOperation(TestOperation::ASYNC)
        .addFunc(std::make_shared<Runnable>([=]() { TestTask(std::to_string(taskId)); }))
        .executedTask();
    taskId++;
    result = result && Wait(taskId);
    result = result && !parallelDispatcher.addOperation(TestOperation::REVOCABLE).addRevokeTask(1).executedTask();
    result = result && task_execution_sequence.size() > 1;

    TestUtils::PublishEvent(g_EVENT_RESP_FIRST, code, std::to_string(result));
}
void MainAbility::RevokeCase3(int code)
{
    APP_LOGI("-- -- -- -- -- --MainAbility::%{public}s", __FUNCTION__);
    Reset();
    auto context = GetContext();
    TaskList parallelDispatcher = TaskList {TestDispatcher::GLOBAL, context, "parallel"};
    int taskId = 0;
    bool result = parallelDispatcher.addOperation(TestOperation::DELAY)
        .addFunc(std::make_shared<Runnable>([=]() { TestTask(std::to_string(taskId)); }))
        .addDelay(delayMs)
        .addOperation(TestOperation::REVOCABLE)
        .addRevokeTask(1)
        .executedTask();
    taskId++;
    result = !result || Wait(taskId);
    result = result || task_execution_sequence.size() > 1;

    TestUtils::PublishEvent(g_EVENT_RESP_FIRST, code, std::to_string(!result));
}
void MainAbility::RevokeCase4(int code)
{
    APP_LOGI("-- -- -- -- -- --MainAbility::%{public}s", __FUNCTION__);
    Reset();
    auto context = GetContext();
    TaskList parallelDispatcher = TaskList {TestDispatcher::GLOBAL, context, "parallel"};
    int taskId = 0;
    bool result = parallelDispatcher.addOperation(TestOperation::DELAY)
        .addFunc(std::make_shared<Runnable>([=]() { TestTask(std::to_string(taskId)); }))
        .addDelay(delayMs)
        .executedTask();
    taskId++;
    result = result && Wait(taskId);
    result = result && !parallelDispatcher.addOperation(TestOperation::REVOCABLE).addRevokeTask(1).executedTask();
    result = result && task_execution_sequence.size() > 1;

    TestUtils::PublishEvent(g_EVENT_RESP_FIRST, code, std::to_string(result));
}
void MainAbility::RevokeCase5(int code)
{
    APP_LOGI("-- -- -- -- -- --MainAbility::%{public}s", __FUNCTION__);
    FillInDispathcer();
    Reset();
    auto context = GetContext();
    TaskList parallelDispatcher = TaskList {TestDispatcher::GLOBAL, context, "parallel"};
    int taskId = 0;
    bool result = parallelDispatcher.addOperation(TestOperation::CREATE_GROUP)
        .addOperation(TestOperation::ASYNC_GROUP)
        .addFunc(std::make_shared<Runnable>([=]() { TestTask(std::to_string(taskId)); }))
        .addOperation(TestOperation::REVOCABLE)
        .addRevokeTask(1)
        .executedTask();
    taskId++;
    result = !result || Wait(taskId);
    result = result || task_execution_sequence.size() > 1;

    TestUtils::PublishEvent(g_EVENT_RESP_FIRST, code, std::to_string(!result));
}
void MainAbility::RevokeCase6(int code)
{
    APP_LOGI("-- -- -- -- -- --MainAbility::%{public}s", __FUNCTION__);
    Reset();
    auto context = GetContext();
    TaskList parallelDispatcher = TaskList {TestDispatcher::GLOBAL, context, "parallel"};
    int taskId = 0;
    bool result = parallelDispatcher.addOperation(TestOperation::CREATE_GROUP)
        .addOperation(TestOperation::ASYNC_GROUP)
        .addFunc(std::make_shared<Runnable>([=]() { TestTask(std::to_string(taskId)); }))
        .executedTask();
    taskId++;
    result = result && Wait(taskId);
    result = result && !parallelDispatcher.addOperation(TestOperation::REVOCABLE).addRevokeTask(1).executedTask();
    result = result && task_execution_sequence.size() > 1;

    TestUtils::PublishEvent(g_EVENT_RESP_FIRST, code, std::to_string(result));
}
REGISTER_AA(MainAbility)
}  // namespace AppExecFwk
}  // namespace OHOS
