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
 
abilityName=$1
bundleName=$2
sleepSeconds=20
MaxlinesInOneLoop=22000
linesInOnLoop=5000
loop=1
if [ -n "$3" ]; then
    loopKey=$3
    loop=$((10#${loopKey}))
fi
echo "loop times: ${loop}"
for i in $(seq 1 ${loop})
do
    echo "==========loop: ${i} start=========="
    echo "kill foundation"
    pgrep foundation | xargs kill -9

    echo "sleep ${sleepSeconds} seconds"
    sleep ${sleepSeconds}

    echo "check whether the FMS restart"
    /system/bin/aa start -d aa -a ${abilityName} -b ${bundleName};hilog -a ${MaxlinesInOneLoop} | tail -n ${linesInOnLoop}>>log_loop${loop}.txt

    if [ $? -eq 0 ]; then
        echo "loop ${i}: FMS restart succeed"
        echo "clean host"
        killall -9 ${bundleName}
    else
        echo "loop ${i}: FMS restart failed"
        exit
    fi
done

exit
