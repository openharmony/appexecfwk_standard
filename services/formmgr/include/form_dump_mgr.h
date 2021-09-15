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

#ifndef FOUNDATION_APPEXECFWK_SERVICES_FORMMGR_INCLUDE_FORM_DUMP_MGR_H
#define FOUNDATION_APPEXECFWK_SERVICES_FORMMGR_INCLUDE_FORM_DUMP_MGR_H

#include <singleton.h>

#include "form_db_info.h"
#include "form_host_record.h"
#include "form_dump_mgr.h"

namespace OHOS {
namespace AppExecFwk {
/**
 * @class FormDumpMgr
 * Form dump mgr.
 */
class FormDumpMgr final  : public DelayedRefSingleton<FormDumpMgr> {
    DECLARE_DELAYED_REF_SINGLETON(FormDumpMgr)
public:
    DISALLOW_COPY_AND_MOVE(FormDumpMgr);

    /**
     * @brief Dump all of form storage infos.
     * @param storageInfos Form storage infos
     * @param formInfos Form storage dump info.
     */
    void DumpStorageFormInfos(const std::vector<FormDBInfo> &storageInfos, std::string &formInfos) const;
    /**
     * @brief Dump form infos.
     * @param formRecordInfos Form record infos.
     * @param formInfos Form dump infos.
     */
    void DumpFormInfos(const std::vector<FormRecord> &formRecordInfos, std::string &formInfos) const;
    /**
     * @brief Dump form infos.
     * @param formRecordInfo Form Host record info.
     * @param formInfo Form dump info.
     */
    void DumpFormHostInfo(const FormHostRecord &formHostRecord, std::string &formInfo) const;
    /**
     * @brief Dump form infos.
     * @param formRecordInfo Form record info.
     * @param formInfo Form dump info.
     */
    void DumpFormInfo(const FormRecord &formRecordInfo, std::string &formInfo) const;
};
}  // namespace AppExecFwk
}  // namespace OHOS

#endif // FOUNDATION_APPEXECFWK_SERVICES_FORMMGR_INCLUDE_FORM_DUMP_MGR_H
