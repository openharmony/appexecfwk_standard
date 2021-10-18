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
 
sleepSeconds=20

echo "Install hap begin"
chmod 644 /system/app/fmsSystemTestSelfStartingA-signed.hap
bm install -p /system/app/fmsSystemTestSelfStartingA-signed.hap
sleep 1
chmod 644 /system/app/fmsSystemTestSelfStartingB-signed.hap
bm install -p /system/app/fmsSystemTestSelfStartingB-signed.hap
sleep 1
echo "Install hap end"
echo "acquire forms"
./FmsSelfStartingTest

if [ $? -eq 0 ]; then
    sleep 10
    echo "acquire forms succeed"
else
    echo "acquire forms failed"
    exit
fi

formStorage=`fm query -s`
echo "get form storage:"
echo "${formStorage}"

sleep 2
echo "kill foundation"
pgrep foundation | xargs kill -9

echo "sleep ${sleepSeconds} seconds"
sleep ${sleepSeconds}

newFormStorage=`fm query -s`
echo "get form storage after FMS restart:"
echo "${newFormStorage}"

if [ "${formStorage}" == "${newFormStorage}" ]; then
    echo "same form storage"
else
    echo "not same form storage"
fi

exit