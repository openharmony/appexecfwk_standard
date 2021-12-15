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
#ifndef OHOS_APP_DISPATCHER_TASK_TASK_STAGE_H
#define OHOS_APP_DISPATCHER_TASK_TASK_STAGE_H

namespace OHOS {
namespace AppExecFwk {
class TaskStage {
public:
    TaskStage() : index_(0)
    {}
    TaskStage(int index)
    {
        index_ = index;
    }
    ~TaskStage() = default;
    /**
     *  Enumeration for task execute stage.
     *  Attention: |REVOKED| is conflict with |AFTER_EXECUTE| and |BEFORE_EXECUTE|, which means,
     *             once |REVOKED|, the other stage will not be notified. So use |isDone| for judging.
     */
    enum TASKSTAGE { BEFORE_EXECUTE = 0, AFTER_EXECUTE = 1, REVOKED = 2 };
    /**
     * Gets the index.
     * @return The index.
     */
    int GetIndex() const
    {
        return index_;
    }

    /**
     * Determines if done.
     * @return True if the task has been finished(maybe canceled). False if it's going to execute.
     */
    bool IsDone() const
    {
        return ((index_ == AFTER_EXECUTE) || (index_ == REVOKED));
    }

private:
    int index_;
};
}  // namespace AppExecFwk
}  // namespace OHOS
#endif
