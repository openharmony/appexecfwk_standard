# Copyright (c) 2021 Huawei Device Co., Ltd.
# Licensed under the Apache License, Version 2.0 (the "License");
# you may not use this file except in compliance with the License.
# You may obtain a copy of the License at
#
#      http://www.apache.org/licenses/LICENSE-2.0
#
#  Unless required by applicable law or agreed to in writing, software
#  distributed under the License is distributed on an "AS IS" BASIS,
#  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
#  See the License for the specific language governing permissions and
#  limitations under the License.

sleepSeconds=10
loop=4

echo "loop times: ${loop}"
for i in $(seq 1 ${loop})
do
    echo "==========loop: ${i} start=========="
    echo "kill foundation"
    pgrep foundation | xargs kill -9

    echo "sleep ${sleepSeconds} seconds"
    sleep ${sleepSeconds}
    ps -e|grep FormMgrService

    if [ $? -eq 0 ]; then
        echo "loop ${i}: FMS restart succeed"
    else
        echo "loop ${i}: FMS restart failed"
        exit
    fi
done

exit