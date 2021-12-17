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
#include <singleton.h>
#include <gtest/gtest.h>
#include "gmock/gmock.h"

#include "mock_resourceManager_interface1.h"

#include "res_config.h"
#include <string>
#include <vector>
#include <map>

namespace OHOS {
namespace Global {
namespace Resource {
class ResourceManagerTestInstance : public ResourceManager2 {
public:
    ResourceManagerTestInstance(){};
    virtual ~ResourceManagerTestInstance(){};

    virtual bool AddResource(const char *path)
    {
        return false;
    };

    virtual RState UpdateResConfig(ResConfig &resConfig)
    {
        return ERROR;
    };

    virtual void GetResConfig(ResConfig &resConfig){};

    virtual RState GetStringById(uint32_t id, std::string &outValue)
    {
        auto iter = StringById_.find(id);
        if (iter == StringById_.end()) {
            return ERROR;
        }
        outValue = iter->second;
        return SUCCESS;
    };
    virtual void SetStringById(uint32_t id, std::string &inValue)
    {
        if (!StringById_.empty()) {
            StringById_.clear();
        }
        StringById_[id] = inValue;
    };

    virtual RState GetStringByName(const char *name, std::string &outValue)
    {
        return ERROR;
    };

    virtual RState GetStringFormatById(std::string &outValue, uint32_t id, ...)
    {
        return ERROR;
    };
    virtual void SetStringFormatById(std::string &inValue, uint32_t id, ...){};

    virtual RState GetStringFormatByName(std::string &outValue, const char *name, ...)
    {
        return ERROR;
    };

    virtual RState GetStringArrayById(uint32_t id, std::vector<std::string> &outValue)
    {
        auto iter = StringArrayById_.find(id);
        if (iter == StringArrayById_.end()) {
            return ERROR;
        }
        outValue = iter->second;
        return SUCCESS;
    };
    virtual void SetStringArrayById(uint32_t id, std::vector<std::string> &inValue)
    {
        if (!StringArrayById_.empty()) {
            StringArrayById_.clear();
        }
        StringArrayById_[id] = inValue;
    };

    virtual RState GetStringArrayByName(const char *name, std::vector<std::string> &outValue)
    {
        return ERROR;
    };

    virtual RState GetPatternById(uint32_t id, std::map<std::string, std::string> &outValue)
    {
        auto iter = PatternById_.find(id);
        if (iter == PatternById_.end()) {
            return ERROR;
        }
        outValue = iter->second;
        return SUCCESS;
    };
    virtual void SetPatternById(uint32_t id, std::map<std::string, std::string> &inValue)
    {
        if (!PatternById_.empty()) {
            PatternById_.clear();
        }
        PatternById_[id] = inValue;
    };

    virtual RState GetPatternByName(const char *name, std::map<std::string, std::string> &outValue)
    {
        return ERROR;
    };

    virtual RState GetPluralStringById(uint32_t id, int quantity, std::string &outValue)
    {
        return ERROR;
    };

    virtual RState GetPluralStringByName(const char *name, int quantity, std::string &outValue)
    {
        return ERROR;
    };

    virtual RState GetPluralStringByIdFormat(std::string &outValue, uint32_t id, int quantity, ...)
    {
        return ERROR;
    };

    virtual RState GetPluralStringByNameFormat(std::string &outValue, const char *name, int quantity, ...)
    {
        return ERROR;
    };

    virtual RState GetThemeById(uint32_t id, std::map<std::string, std::string> &outValue)
    {
        auto iter = ThemeById_.find(id);
        if (iter == ThemeById_.end()) {
            return ERROR;
        }
        outValue = iter->second;
        return SUCCESS;
    };
    virtual void SetThemeById(uint32_t id, std::map<std::string, std::string> &inValue)
    {
        if (!ThemeById_.empty()) {
            ThemeById_.clear();
        }
        ThemeById_[id] = inValue;
    };

    virtual RState GetThemeByName(const char *name, std::map<std::string, std::string> &outValue)
    {
        return ERROR;
    };

    virtual RState GetBooleanById(uint32_t id, bool &outValue)
    {
        return ERROR;
    };

    virtual RState GetBooleanByName(const char *name, bool &outValue)
    {
        return ERROR;
    };

    virtual RState GetIntegerById(uint32_t id, int &outValue)
    {
        return ERROR;
    };

    virtual RState GetIntegerByName(const char *name, int &outValue)
    {
        return ERROR;
    };

    virtual RState GetFloatById(uint32_t id, float &outValue)
    {
        return ERROR;
    };

    virtual RState GetFloatByName(const char *name, float &outValue)
    {
        return ERROR;
    };

    virtual RState GetIntArrayById(uint32_t id, std::vector<int> &outValue)
    {
        auto iter = IntArrayById_.find(id);
        if (iter == IntArrayById_.end()) {
            return ERROR;
        }
        outValue = iter->second;
        return SUCCESS;
    };
    virtual void SetIntArrayById(uint32_t id, std::vector<int> &inValue)
    {
        if (!IntArrayById_.empty()) {
            IntArrayById_.clear();
        }
        IntArrayById_[id] = inValue;
    };

    virtual RState GetIntArrayByName(const char *name, std::vector<int> &outValue)
    {
        return ERROR;
    };

    virtual RState GetColorById(uint32_t id, uint32_t &outValue)
    {
        auto iter = ColorById_.find(id);
        if (iter == ColorById_.end()) {
            return ERROR;
        }
        outValue = iter->second;
        return SUCCESS;
    };
    virtual void SetColorById(uint32_t id, uint32_t &inValue)
    {
        if (!ColorById_.empty()) {
            ColorById_.clear();
        }
        ColorById_[id] = inValue;
    };

    virtual RState GetColorByName(const char *name, uint32_t &outValue)
    {
        return ERROR;
    };

    virtual RState GetProfileById(uint32_t id, std::string &outValue)
    {
        return ERROR;
    };

    virtual RState GetProfileByName(const char *name, std::string &outValue)
    {
        return ERROR;
    };

    virtual RState GetMediaById(uint32_t id, std::string &outValue)
    {
        return ERROR;
    };

    virtual RState GetMediaByName(const char *name, std::string &outValue)
    {
        return ERROR;
    };

public:
    std::map<int, std::string> StringById_;
    std::map<int, std::string> StringFormatById_;
    std::map<int, std::vector<std::string>> StringArrayById_;
    std::map<int, std::map<std::string, std::string>> PatternById_;
    std::map<int, std::map<std::string, std::string>> ThemeById_;
    std::map<int, std::vector<int>> IntArrayById_;
    std::map<int, uint32_t> ColorById_;

    // static ResourceManagerTestInstance* instance;
    static std::shared_ptr<ResourceManagerTestInstance> instance;
};

std::shared_ptr<ResourceManagerTestInstance> ResourceManagerTestInstance::instance = nullptr;

std::shared_ptr<ResourceManager2> CreateResourceManager2()
{
    if (ResourceManagerTestInstance::instance == nullptr) { /*  */
        ResourceManagerTestInstance::instance = std::make_shared<ResourceManagerTestInstance>();
    }
    return ResourceManagerTestInstance::instance;
}
}  // namespace Resource
}  // namespace Global
}  // namespace OHOS