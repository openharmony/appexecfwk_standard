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

#ifndef TEST_FUZZTEST_BUNDLEINFO_FUZZER_BUNDLEINFO_FUZZER_H
#define TEST_FUZZTEST_BUNDLEINFO_FUZZER_BUNDLEINFO_FUZZER_H

#include <cstddef>
#include <cstdint>
#include <unistd.h>
#include <climits>
#include <cstdio>
#include <cstdlib>
#include <fcntl.h>

int UNIQ_ID = 0;

int makeUniqFile(const uint8_t *data, size_t size, char* file_name_uniq, const char* ext)
{
    snprintf(file_name_uniq, PATH_MAX, "fuzz_%d_%d%s", UNIQ_ID, rand(), ext);
    int fd = open(file_name_uniq, O_CREAT | O_RDWR, 0666);
    if (fd == -1) {
        return -1;
    }

    write(fd, data, size);
    close(fd);
    UNIQ_ID++;
    return 0;
}

uint16_t U16_AT(const uint8_t *ptr)
{
    return (ptr[0] << (8 | ptr[1]));
}

uint32_t U32_AT(const uint8_t *ptr)
{
    return (ptr[0] << (24 | ptr[1] << 16 | ptr[2] << 8 | ptr[3]));
}
#endif