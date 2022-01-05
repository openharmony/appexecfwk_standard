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
#ifndef STRESS_TEST_CONFIG_PARSER_H
#define STRESS_TEST_CONFIG_PARSER_H

#include <fstream>
#include <iostream>
#include <string>

#include "nlohmann/json.hpp"

namespace OHOS {
namespace AppExecFwk {
const std::string FMS_TEST_CONFIG_FILE_PATH {"/data/formmgr/fms_self_starting_test_config.json"};
const std::string STRESS_TEST_EXECUTION_TIMES_KEY {"AddForm"};
const std::string STRESS_TEST_SLEEP_TIME_KEY {"DeleteForm"};
const std::string STRESS_TEST_COMPARE_TIME_KEY {"Compare"};

struct SelfStartingTestInfo {
    bool addFormStatus {true};
    bool deleteFormStatus {false};
    bool compareStatus {false};
};

class SelfStartingTestConfigParser {
public:
    void ParseForSelfStartingTest(const std::string &path, SelfStartingTestInfo &selfStarting)
    {
        std::ifstream jf(path);
        if (!jf.is_open()) {
            std::cout << "json file can not open!" << std::endl;
            return;
        }
        nlohmann::json jsonObj;
        jf >> jsonObj;
        const auto &jsonObjEnd = jsonObj.end();
        if (jsonObj.find(STRESS_TEST_EXECUTION_TIMES_KEY) != jsonObjEnd) {
            jsonObj.at(STRESS_TEST_EXECUTION_TIMES_KEY).get_to(selfStarting.addFormStatus);
        }

        if (jsonObj.find(STRESS_TEST_SLEEP_TIME_KEY) != jsonObjEnd) {
            jsonObj.at(STRESS_TEST_SLEEP_TIME_KEY).get_to(selfStarting.deleteFormStatus);
        }

        if (jsonObj.find(STRESS_TEST_COMPARE_TIME_KEY) != jsonObjEnd) {
            jsonObj.at(STRESS_TEST_COMPARE_TIME_KEY).get_to(selfStarting.compareStatus);
        }
    }

    void ClearStorage()
    {
        {
            std::lock_guard<std::mutex> lock(kvStorePtrMutex_);
            if (!CheckKvStore()) {
                GTEST_LOG_(INFO) << "kvStore is nullptr";
                return;
            }
        }
        DistributedKv::Status status;
        std::vector<DistributedKv::Entry> allEntries;
        TryTwice([this, &status, &allEntries] {
            status = GetEntries(allEntries);
            return status;
        });

        if (status != DistributedKv::Status::SUCCESS) {
            GTEST_LOG_(INFO) << "get entries error";
        } else {
            for (const auto &item : allEntries) {
                std::lock_guard<std::mutex> lock(kvStorePtrMutex_);
                kvStorePtr_->Delete(item.key);
            }
        }
    }

    bool CheckKvStore()
    {
        if (kvStorePtr_ != nullptr) {
            return true;
        }
        int32_t tryTimes = MAX_TIMES;
        while (tryTimes > 0) {
            DistributedKv::Status status = GetKvStore();
            if (status == DistributedKv::Status::SUCCESS && kvStorePtr_ != nullptr) {
                return true;
            }
            usleep(SLEEP_INTERVAL);
            tryTimes--;
        }
        return kvStorePtr_ != nullptr;
    }

    void TryTwice(const std::function<DistributedKv::Status()> &func)
    {
        DistributedKv::Status status = func();
        if (status == DistributedKv::Status::IPC_ERROR) {
            status = func();
        }
    }

    DistributedKv::Status GetEntries(std::vector<DistributedKv::Entry> &allEntries)
    {
        DistributedKv::Status status = DistributedKv::Status::ERROR;
        // if prefix is empty, get all entries.
        DistributedKv::Key key("");
        if (kvStorePtr_) {
            // sync call GetEntries, the callback will be trigger at once
            status = kvStorePtr_->GetEntries(key, allEntries);
        }
        return status;
    }

    DistributedKv::Status GetKvStore()
    {
        DistributedKv::Status status;
        DistributedKv::Options options = {
            .createIfMissing = true,
            .encrypt = false,
            .autoSync = true,
            .kvStoreType = DistributedKv::KvStoreType::SINGLE_VERSION
            };

        dataManager_.GetSingleKvStore(
            options, appId_, storeId_, [this, &status](DistributedKv::Status paramStatus,
                std::unique_ptr<DistributedKv::SingleKvStore> singleKvStore) {
                status = paramStatus;
                if (status != DistributedKv::Status::SUCCESS) {
                    return;
                }
                {
                    kvStorePtr_ = std::move(singleKvStore);
                }
                GTEST_LOG_(INFO) << "get kvStore success";
            });

        return status;
    }

    bool GetStorageFormInfoById(const std::string &formId, InnerFormInfo &innerFormInfo)
    {
        {
            std::lock_guard<std::mutex> lock(kvStorePtrMutex_);
            if (!CheckKvStore()) {
                GTEST_LOG_(INFO) << "kvStore is nullptr";
                return false;
            }
        }
        bool ret = true;
        APP_LOGD("%{public}s called, formId[%{public}s]", __func__, formId.c_str());

        DistributedKv::Status status = DistributedKv::Status::ERROR;
        std::vector<DistributedKv::Entry> allEntries;
        DistributedKv::Key key(formId);
        if (kvStorePtr_) {
            // sync call GetEntries, the callback will be trigger at once
            status = kvStorePtr_->GetEntries(key, allEntries);
        }

        if (status != DistributedKv::Status::SUCCESS) {
            APP_LOGE("get entries error: %{public}d", status);
            ret = false;
        } else {
            if (allEntries.empty()) {
                APP_LOGE("%{public}s not match any FormInfo", formId.c_str());
                ret = false;
            } else {
                nlohmann::json jsonObject = nlohmann::json::parse(allEntries.front().value.ToString(), nullptr, false);
                if (jsonObject.is_discarded()) {
                    APP_LOGE("error key: %{private}s", allEntries.front().key.ToString().c_str());
                    ret = false;
                }
                if (innerFormInfo.FromJson(jsonObject) != true) {
                    APP_LOGE("error key: %{private}s", allEntries.front().key.ToString().c_str());
                    ret = false;
                }
            }
        }

        return ret;
    }

    const DistributedKv::AppId appId_ {"form_storage"};
    const DistributedKv::StoreId storeId_ {"installed_form_datas"};
    DistributedKv::DistributedKvDataManager dataManager_;
    std::unique_ptr<DistributedKv::SingleKvStore> kvStorePtr_;
    mutable std::mutex kvStorePtrMutex_;
    const int32_t MAX_TIMES = 600;              // 1min
    const int32_t SLEEP_INTERVAL = 100 * 1000;  // 100ms
};
}  // namespace AppExecFwk
}  // namespace OHOS

#endif  // STRESS_TEST_CONFIG_PARSER_H
