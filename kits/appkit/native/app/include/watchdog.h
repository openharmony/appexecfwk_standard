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

#ifndef FOUNDATION_APPEXECFWK_WATCH_DOG_H
#define FOUNDATION_APPEXECFWK_WATCH_DOG_H

#include <string>
#include <mutex>
#include "event_handler.h"
#include "inner_event.h"

namespace OHOS {
namespace AppExecFwk {
const uint32_t MAIN_THREAD_IS_ALIVE = 0;
const uint32_t MAIN_THREAD_TIMEOUT_TIME = 3000;
const uint32_t INI_ZERO = 0;
const uint32_t INI_TIMER_FIRST_SECOND = 10;
const uint32_t INI_TIMER_SECOND = 3;
const std::string MAIN_THREAD_IS_ALIVE_MSG = "MAIN_THREAD_IS_ALIVE";
class WatchDog : public EventHandler {
public:
    WatchDog(const std::shared_ptr<EventRunner> &runner);
    virtual ~WatchDog() = default;

    /**
     *
     * @brief Process the event.
     *
     * @param event the event want to be processed.
     *
     */
    void ProcessEvent(const OHOS::AppExecFwk::InnerEvent::Pointer &event) override;
    static void Timer(int sig);

    /**
     *
     * @brief Init the Watchdog.
     *
     * @param mainHandler The handler of main thread.
     * @param watchDogHandler The handler of watchdog thread.
     */
    void Init(const std::shared_ptr<EventHandler> &mainHandler, const std::shared_ptr<WatchDog> &watchDogHandler);

    /**
     *
     * @brief Stop the mainthread function of watchdog.
     *
     */
    void Stop();

    /**
     *
     * @brief Get the eventHandler of watchdog thread.
     *
     * @return Returns the eventHandler of watchdog thread.
     */
    static std::shared_ptr<WatchDog> GetCurrentHandler();

    /**
     *
     * @brief Get the App main thread state.
     *
     * @return Returns the App main thread state.
     */
    static bool GetAppMainThreadState();

private:
    std::shared_ptr<EventRunner> watchDogRunner_;
    static bool appMainThreadIsAlive_;
    static std::shared_ptr<EventHandler> appMainHandler_;
    static std::shared_ptr<WatchDog> currentHandler_;
};
}  // namespace AppExecFwk
}  // namespace OHOS
#endif  // FOUNDATION_APPEXECFWK_WATCH_DOG_H
