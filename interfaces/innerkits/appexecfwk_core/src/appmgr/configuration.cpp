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

#include <nlohmann/json.hpp>
#include "string_ex.h"
#include "app_log_wrapper.h"
#include "configuration.h"
namespace {
    const std::string CONNECTION_SYMBOL {"#"};
    const std::string EMPTY_STRING {""};
}
namespace OHOS {
namespace AppExecFwk {
using json = nlohmann::json;

Configuration::Configuration()
{}

Configuration::Configuration(const Configuration &other)
{
    defaultDisplayId_ = other.defaultDisplayId_;
    configParameter_ = other.configParameter_;
}

Configuration& Configuration::operator= (const Configuration &other)
{
    if (this == &other) {
        return *this;
    }

    defaultDisplayId_ = other.defaultDisplayId_;
    configParameter_.clear();
    configParameter_ = other.configParameter_;
    return *this;
}

Configuration::~Configuration()
{}

void Configuration::MakeTheKey(std::string &getKey, int id, const std::string &param) const
{
    if (param.empty()) {
        return ;
    }

    getKey.clear();
    getKey += std::to_string(id);
    getKey += CONNECTION_SYMBOL;
    getKey += param;
    APP_LOGW(" getKey [%{public}s]", getKey.c_str());
}

bool Configuration::AddItem(int displayId, const std::string &key, const std::string &value)
{
    if (key.empty() || value.empty()) {
        return false;
    }

    std::string getKey;
    MakeTheKey(getKey, displayId, key);

    configParameter_[getKey] = value;
    return true;
}

std::string Configuration::GetItem(int displayId, const std::string &key) const
{
    if (key.empty()) {
        return EMPTY_STRING;
    }

    std::string getKey;
    MakeTheKey(getKey, displayId, key);

    auto iter = configParameter_.find(getKey);
    if (iter != configParameter_.end()) {
        return iter->second;
    }

    return EMPTY_STRING;
}

int Configuration::GetItemSize() const
{
    return configParameter_.size();
}

void Configuration::GetAllKey(std::vector<std::string> &keychain) const
{
    keychain.clear();
    for (const auto &it :configParameter_) {
        keychain.push_back(it.first);
    }
}

std::string Configuration::GetValue(const std::string &key) const
{
    auto iter = configParameter_.find(key);
    if (iter != configParameter_.end()) {
        return iter->second;
    }

    return EMPTY_STRING;
}

void Configuration::CompareDifferent(std::vector<std::string> &diffKeyV, const Configuration &other)
{
    if (other.GetItemSize() == 0) {
        return ;
    }

    diffKeyV.clear();
    std::vector<std::string> otherk;
    other.GetAllKey(otherk);
    for (const auto &iter : otherk) {
        APP_LOGW(" iter : [%{public}s] | Val: [%{public}s]", iter.c_str(), other.GetValue(iter).c_str());
        // Insert new content directly
        auto pair = configParameter_.insert(std::make_pair(iter, other.GetValue(iter)));
        if (pair.second) {
            diffKeyV.push_back(iter); // One of the changes this time
            continue;
        }
        // Compare what you already have
        if (!other.GetValue(iter).empty() && other.GetValue(iter) != GetValue(iter)) {
            diffKeyV.push_back(iter);
        }
    }
}

void Configuration::Merge(const std::string &mergeItemKey, const Configuration &other)
{
    auto myItem = GetValue(mergeItemKey);
    auto otherItem = other.GetValue(mergeItemKey);
    // myItem possible empty
    if (!otherItem.empty() && otherItem != myItem) {
        configParameter_[mergeItemKey] = otherItem;
    }
}

int Configuration::RemoveItem(int displayId, const std::string &key)
{
    if (key.empty()) {
        return 0;
    }

    std::string getKey;
    MakeTheKey(getKey, displayId, key);

    return configParameter_.erase(getKey);
}

bool Configuration::AddItem(const std::string &key, const std::string &value)
{
    if (key.empty() || value.empty()) {
        return false;
    }

    std::string getKey;
    MakeTheKey(getKey, defaultDisplayId_, key);

    configParameter_[getKey] = value;
    return true;
}

std::string Configuration::GetItem(const std::string &key) const
{
    if (key.empty()) {
        return EMPTY_STRING;
    }

    std::string getKey;
    MakeTheKey(getKey, defaultDisplayId_, key);

    auto iter = configParameter_.find(getKey);
    if (iter != configParameter_.end()) {
        return iter->second;
    }

    return EMPTY_STRING;
}

int Configuration::RemoveItem(const std::string &key)
{
    if (key.empty()) {
        return 0;
    }

    std::string getKey;
    MakeTheKey(getKey, defaultDisplayId_, key);

    return configParameter_.erase(getKey);
}

const std::string& Configuration::GetName() const
{
    json configArray(configParameter_);
    toStrintg_ = configArray.dump();
    return toStrintg_;
}

bool Configuration::ReadFromParcel(Parcel &parcel)
{
    APP_LOGW("ReadFromParcel");
    defaultDisplayId_ = parcel.ReadInt32();
    int32_t configSize = parcel.ReadInt32();
    std::vector<std::string> keys;
    std::vector<std::string> values;
    keys.clear();
    values.clear();
    parcel.ReadStringVector(&keys);
    parcel.ReadStringVector(&values);

    std::string key;
    std::string val;
    for (int32_t i = 0; i < configSize; i++) {
        key = keys.at(i);
        val = values.at(i);
        configParameter_.emplace(key, val);
    }
    return true;
}

Configuration *Configuration::Unmarshalling(Parcel &parcel)
{
    Configuration *Configuration = new (std::nothrow) OHOS::AppExecFwk::Configuration();
    if (Configuration && !Configuration->ReadFromParcel(parcel)) {
        delete Configuration;
        Configuration = nullptr;
    }
    return Configuration;
}

bool Configuration::Marshalling(Parcel &parcel) const
{
    APP_LOGW("Marshalling");
    std::vector<std::string> keys;
    std::vector<std::string> values;
    keys.clear();
    values.clear();
    parcel.WriteInt32(defaultDisplayId_);
    parcel.WriteInt32(configParameter_.size());
    for (const auto &config : configParameter_) {
        keys.emplace_back(config.first);
        values.emplace_back(config.second);
    }

    parcel.WriteStringVector(keys);
    parcel.WriteStringVector(values);
    return true;
}
}  // namespace AppExecFwk
}  // namespace OHOS