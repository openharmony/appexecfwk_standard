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

#ifndef FOUNDATION_APPEXECFWK_INTERFACES_INNERKITS_APPEXECFWK_CORE_INCLUDE_FORMMGR_FORM_MGR_STUB_H
#define FOUNDATION_APPEXECFWK_INTERFACES_INNERKITS_APPEXECFWK_CORE_INCLUDE_FORMMGR_FORM_MGR_STUB_H

#include <map>

#include "form_mgr_interface.h"
#include "iremote_object.h"
#include "iremote_stub.h"

namespace OHOS {
namespace AppExecFwk {
/**
 * @class FormMgrStub
 * Form manager service stub.
 */
class FormMgrStub : public IRemoteStub<IFormMgr> {
public:
    FormMgrStub();
    virtual ~FormMgrStub();
    /**
     * @brief Handle remote request.
     * @param data input param.
     * @param reply output param.
     * @param option message option.
     * @return Returns ERR_OK on success, others on failure.
     */
    virtual int OnRemoteRequest(
        uint32_t code, MessageParcel &data, MessageParcel &reply, MessageOption &option);

private:
    /**
     * @brief Handle AddForm message.
     * @param data input param.
     * @param reply output param.
     * @return Returns ERR_OK on success, others on failure.
     */
    int32_t HandleAddForm(MessageParcel &data, MessageParcel &reply);
    /**
     * @brief Handle DeleteForm message.
     * @param data input param.
     * @param reply output param.
     * @return Returns ERR_OK on success, others on failure.
     */
    int32_t HandleDeleteForm(MessageParcel &data, MessageParcel &reply);
    /**
     * @brief Handle ReleaseForm message.
     * @param data input param.
     * @param reply output param.
     * @return Returns ERR_OK on success, others on failure.
     */
    int32_t HandleReleaseForm(MessageParcel &data, MessageParcel &reply);
    /**
     * @brief Handle UpdateForm message.
     * @param data input param.
     * @param reply output param.
     * @return Returns ERR_OK on success, others on failure.
     */
    int32_t HandleUpdateForm(MessageParcel &data, MessageParcel &reply);

    /**
     * @brief handle LifecycleUpdate message.
     * @param data input param.
     * @param reply output param.
     * @return Returns ERR_OK on success, others on failure.
     */
    int32_t HandleLifecycleUpdate(MessageParcel &data, MessageParcel &reply);
    /**
     * @brief handle SetNextRefreshTime message.
     * @param data input param.
     * @param reply output param.
     * @return Returns ERR_OK on success, others on failure.
     */
    int32_t HandleSetNextRefreshTime(MessageParcel &data, MessageParcel &reply);
    /**
     * @brief Handle RequestForm message.
     * @param data input param.
     * @param reply output param.
     * @return Returns ERR_OK on success, others on failure.
     */
    int32_t HandleRequestForm(MessageParcel &data, MessageParcel &reply);
    /**
     * @brief Handle NotifyWhetherVisibleForms message.
     * @param data input param.
     * @param reply output param.
     * @return Returns ERR_OK on success, others on failure.
     */
    int32_t HandleNotifyWhetherVisibleForms(MessageParcel &data, MessageParcel &reply);
    /**
     * @brief Handle CastTempForm message.
     * @param data input param.
     * @param reply output param.
     * @return Returns ERR_OK on success, others on failure.
     */
    int32_t HandleCastTempForm(MessageParcel &data, MessageParcel &reply);
    /**
     * @brief Handle DumpStorageFormInfos message.
     * @param data input param.
     * @param reply output param.
     * @return Returns ERR_OK on success, others on failure.
     */
    int32_t HandleDumpStorageFormInfos(MessageParcel &data, MessageParcel &reply);
    /**
     * @brief Handle DumpFormInfoByBundleName message.
     * @param data input param.
     * @param reply output param.
     * @return Returns ERR_OK on success, others on failure.
     */
    int32_t HandleDumpFormInfoByBundleName(MessageParcel &data, MessageParcel &reply);
    /**
     * @brief Handle DumpFormInfoByFormId message.
     * @param data input param.
     * @param reply output param.
     * @return Returns ERR_OK on success, others on failure.
     */
    int32_t HandleDumpFormInfoByFormId(MessageParcel &data, MessageParcel &reply);
    /**
     * @brief Handle DumpFormTimerByFormId message.
     * @param data input param.
     * @param reply output param.
     * @return Returns ERR_OK on success, others on failure.
     */
    int32_t HandleDumpFormTimerByFormId(MessageParcel &data, MessageParcel &reply);
    /**
     * @brief Handle DumpFormInfoByFormId message.
     * @param data input param.
     * @param reply output param.
     * @return Returns ERR_OK on success, others on failure.
     */
    int32_t HandleMessageEvent(MessageParcel &data, MessageParcel &reply);

    /**
     * @brief Handle BatchAddFormRecords message.
     * @param data input param.
     * @param reply output param.
     * @return Returns ERR_OK on success, others on failure.
     */
    int32_t HandleBatchAddFormRecords(MessageParcel &data, MessageParcel &reply);
    /**
     * @brief Handle ClearFormRecords message.
     * @param data input param.
     * @param reply output param.
     * @return Returns ERR_OK on success, others on failure.
     */
    int32_t HandleClearFormRecords(MessageParcel &data, MessageParcel &reply);
   /**
     * @brief Handle DistributedDataAddForm message.
     * @param data input param.
     * @param reply output param.
     * @return Returns ERR_OK on success, others on failure.
     */
    int32_t HandleDistributedDataAddForm(MessageParcel &data, MessageParcel &reply);
    /**
     * @brief Handle DistributedDataDeleteForm message.
     * @param data input param.
     * @param reply output param.
     * @return Returns ERR_OK on success, others on failure.
     */
    int32_t HandleDistributedDataDeleteForm(MessageParcel &data, MessageParcel &reply);
private:
    using FormMgrFunc = int32_t (FormMgrStub::*)(MessageParcel &data, MessageParcel &reply);
    std::map<uint32_t, FormMgrFunc> memberFuncMap_;

    DISALLOW_COPY_AND_MOVE(FormMgrStub);
};
}  // namespace AppExecFwk
}  // namespace OHOS
#endif  // FOUNDATION_APPEXECFWK_INTERFACES_INNERKITS_APPEXECFWK_CORE_INCLUDE_FORMMGR_FORM_MGR_STUB_H
