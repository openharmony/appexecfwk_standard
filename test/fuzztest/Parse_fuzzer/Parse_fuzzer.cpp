/*
 * Copyright (c) 2022 Huawei Device Co., Ltd.
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

#include "Parse_fuzzer.h"

#include <cstddef>
#include <cstdint>
#include <iostream>

#include "bundle_parser.h"

using namespace OHOS::AppExecFwk;

namespace OHOS {
    bool DoSomethingInterestingWithMyAPI(const uint8_t* data, size_t size)
    {
        if (size <= 4) {
            return false;
        }

        auto pFile = fopen("myHap.hap", "wb");
        if (pFile == nullptr) {
            std::cout<< "fopen hap error!";
        }

        fputs(reinterpret_cast<const char*>(data), pFile);
        fclose(pFile);
        InnerBundleInfo info;
        BundleParser bundleParser;
        auto ret = bundleParser.Parse("myHap.hap", info);
        if (ret != ERR_OK) {
            std::cout << "parse bundle info failed";
            return false;
        }
        return true;
    }
}

/* Fuzzer entry point */
extern "C" int LLVMFuzzerTestOneInput(const uint8_t* data, size_t size)
{
    /* Run your code on data */
    OHOS::DoSomethingInterestingWithMyAPI(data, size);
    return 0;
}

