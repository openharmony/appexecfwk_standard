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

#include "watchdog.h"
#include "app_log_wrapper.h"
#include <sys/time.h>
#include <signal.h>
#include <thread>

namespace OHOS {
namespace AppExecFwk {
std::shared_ptr<EventHandler> WatchDog::appMainHandler_ = nullptr;
std::shared_ptr<WatchDog> WatchDog::currentHandler_ = nullptr;
bool WatchDog::appMainThreadIsAlive_ = false;

WatchDog::WatchDog(const std::shared_ptr<EventRunner> &runner)
    : AppExecFwk::EventHandler(runner), watchDogRunner_(runner)
{}

/**
 *
 * @brief Process the event.
 *
 * @param event the event want to be processed.
 *
 */
void WatchDog::ProcessEvent(const OHOS::AppExecFwk::InnerEvent::Pointer &event)
{
    auto eventId = event->GetInnerEventId();
    if (eventId == MAIN_THREAD_IS_ALIVE) {
        WatchDog::appMainThreadIsAlive_ = true;
        currentHandler_->RemoveTask(MAIN_THREAD_IS_ALIVE_MSG);
    }
}

void WatchDog::Init(const std::shared_ptr<EventHandler> &mainHandler, const std::shared_ptr<WatchDog> &watchDogHandler)
{
    APP_LOGI("watchdog is run !");
    WatchDog::appMainHandler_ = mainHandler;
    WatchDog::currentHandler_ = watchDogHandler;
    struct itimerval tick;
    tick.it_value.tv_sec = INI_TIMER_FIRST_SECOND;
    tick.it_value.tv_usec = INI_ZERO;
    tick.it_interval.tv_sec = INI_TIMER_SECOND;
    tick.it_interval.tv_usec = INI_ZERO;

    signal(SIGALRM, &WatchDog::Timer);

    if(setitimer(ITIMER_REAL, &tick, NULL) < INI_ZERO) {
        APP_LOGE("Init WatchDog timer failed");
    }
}

void WatchDog::Stop()
{
    APP_LOGI("Watchdog is stop !");
    if(watchDogRunner_) {
        watchDogRunner_.reset();
    }
    if(currentHandler_) {
        currentHandler_.reset();
    }
}

std::shared_ptr<WatchDog> WatchDog::GetCurrentHandler()
{
    return currentHandler_;
}

bool WatchDog::GetAppMainThreadState()
{
    return appMainThreadIsAlive_;
}

void WatchDog::Timer(int sig)
{
    auto timeoutTask1 = [&]() {
        appMainThreadIsAlive_ = false;
        APP_LOGI("Waring : main thread is not response!");
    };
    currentHandler_->PostTask(timeoutTask1, MAIN_THREAD_IS_ALIVE_MSG, MAIN_THREAD_TIMEOUT_TIME);
    appMainHandler_->SendEvent(MAIN_THREAD_IS_ALIVE);
}
}  // namespace AppExecFwk
}  // namespace OHOS
