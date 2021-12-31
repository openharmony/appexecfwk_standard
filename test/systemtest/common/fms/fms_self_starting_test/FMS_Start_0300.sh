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
testfile="/data/formmgr/fms_self_starting_test_config.json"
sleepSeconds=10

rm -rf /data/formmgr
mkdir /data/formmgr

chown system /data/formmgr
chgrp system /data/formmgr

touch ${testfile}
echo \{\"AddForm\":true,\"DeleteForm\":false,\"Compare\":false\} > ${testfile};
chown system ${testfile}
chgrp system ${testfile}

sleep 2
echo "acquire forms"
./FmsSelfStartingTest

sleep 2
echo "kill foundation"
pgrep foundation | xargs kill -9

echo "sleep ${sleepSeconds} seconds"
sleep ${sleepSeconds}

sleep 2
beforeKill=`pgrep foundation`
echo ${beforeKill} > "/data/formmgr/beforeKill.txt"
echo "kill foundation"
pgrep foundation | xargs kill -9

echo "sleep ${sleepSeconds} seconds"
sleep ${sleepSeconds}

afterKill=`pgrep foundation`
echo ${afterKill} > "/data/formmgr/afterKill.txt"
if [ "${beforeKill}" == "${afterKill}" ]; then
    echo "process has not changed"
else
    echo "process has changed"
fi

rm -f ${testfile}
touch ${testfile}
echo \{\"AddForm\":false,\"DeleteForm\":false,\"Compare\":true\} > ${testfile};
chown system ${testfile}
chgrp system ${testfile}
sleep 12
echo "compare forms"
./FmsSelfStartingTest

rm -f ${testfile}
touch ${testfile}
echo \{\"AddForm\":false,\"DeleteForm\":true,\"Compare\":false\} > ${testfile};
chown system ${testfile}
chgrp system ${testfile}
sleep 2
echo "delete forms"
./FmsSelfStartingTest

rm -rf /data/formmgr

exit
