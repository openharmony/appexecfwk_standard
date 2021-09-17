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
#include "form_host_record.h"
#include "form_task_mgr.h"

namespace OHOS {
namespace AppExecFwk {
/**
 * @brief Add form id.
 * @param formId The Id of the form.
 */
void FormHostRecord::AddForm(int64_t formId)
{
    if (forms_.find(formId) != forms_.end()) {
        return;
    }
    forms_[formId] = true;
}
/**
 * @brief Delete form id.
 * @param formId The Id of the form.
 */
void FormHostRecord::DelForm(int64_t formId)
{
    forms_.erase(formId);
}
/**
 * @brief forms_ is empty or not.
 * @return forms_ is empty or not.
 */
bool FormHostRecord::IsEmpty() const
{
    return forms_.empty();
}
/**
 * @brief formId is in forms_ or not.
 * @param formId The Id of the form.
 * @return formId is in forms_ or not.
 */
bool FormHostRecord::Contains(int64_t formId) const
{
    return forms_.find(formId) != forms_.end();
}

/**
 * @brief Set refresh enable flag.
 * @param formId The Id of the form.
 * @param flag True for enbale, false for disable.
 */
void FormHostRecord::SetEnableRefresh(int64_t formId, bool flag)
{
    if (forms_.find(formId) == forms_.end()) {
        return;
    }
    forms_[formId] = flag;
}
/**
 * @brief Refresh enable or not.
 * @param formId The Id of the form.
 * @return true on enbale, false on disable..
 */
bool FormHostRecord::IsEnableRefresh(int64_t formId) const
{
    auto result = forms_.find(formId);
    if (result != forms_.end()) {
        return result->second;
    }
    return false;
}
/**
 * @brief Set need refresh enable flag.
 * @param formId The Id of the form.
 * @param flag True for enbale, false for disable.
 */
void FormHostRecord::SetNeedRefresh(int64_t formId, bool flag)
{
    needRefresh_[formId] = flag;
}
/**
 * @brief Need Refresh enable or not.
 * @param formId The Id of the form.
 * @return true on enbale, false on disable..
 */
bool FormHostRecord::IsNeedRefresh(int64_t formId) const
{
    auto result = needRefresh_.find(formId);
    if (result != needRefresh_.end()) {
        return result->second;
    }
    return false;
}
/**
 * @brief Get clientStub_.
 * @return clientStub_.
 */
sptr<IRemoteObject> FormHostRecord::GetClientStub() const
{
    return clientStub_;
}

/**
 * @brief Send form data to form host.
 * @param id The Id of the form.
 * @param record Form record.
 */
void FormHostRecord::OnAcquire(int64_t id, const FormRecord &record)
{
    APP_LOGD("FormHostRecord OnAcquire");
    if (clientImpl_ == nullptr) {
        APP_LOGE("%{public}s: clientImpl_ can not be NULL", __func__);
        return;
    }

    clientImpl_->OnAcquired(id, record, clientStub_);
}

/**
 * @brief Update form data to form host.
 * @param id The Id of the form.
 * @param record Form record.
 */
void FormHostRecord::OnUpdate(int64_t id, const FormRecord &record)
{
    APP_LOGI("%{public}s start.", __func__);

    if (clientImpl_ == nullptr) {
        APP_LOGE("%{public}s: clientImpl_ can not be null.", __func__);
        return;
    }

    clientImpl_->OnUpdate(id, record, clientStub_);
}

/**
 * @brief Send form uninstall message to form host.
 * @param id The Id of the form.
 * @param record Form record.
 */
void FormHostRecord::OnFormUninstalled(std::vector<int64_t> &formIds)
{
    APP_LOGI("%{public}s start.", __func__);

    if (clientImpl_ == nullptr) {
        APP_LOGE("%{public}s: clientImpl_ can not be null.", __func__);
        return;
    }
    clientImpl_->OnUninstall(formIds, clientStub_);
}
/**
 * @brief Release resource.
 * @param id The Id of the form.
 * @param record Form record.
 */
void FormHostRecord::CleanResource()
{
    if (clientStub_ != nullptr && deathRecipient_ != nullptr) {
        clientStub_->RemoveDeathRecipient(deathRecipient_);
        clientStub_ = nullptr;
        deathRecipient_ = nullptr;
    }
}
/**
 * @brief Set value of callerUid_.
 * @param callerUid Caller uid.
 */
void FormHostRecord::SetCallerUid(const int callerUid)
{
    callerUid_ = callerUid;
}
/**
 * @brief Set value of clientStub_.
 * @param clientStub remote object.
 */
void FormHostRecord::SetClientStub(const sptr<IRemoteObject> &clientStub)
{
    clientStub_ = clientStub;
}
/**
 * @brief Set value of clientImpl_.
 * @param clientImpl Form host callback object.
 */
void FormHostRecord::SetClientImpl(const std::shared_ptr<FormHostCallback> &clientImpl)
{
    clientImpl_ = clientImpl;
}
/**
 * @brief Get deathRecipient_.
 * @return deathRecipient_.
 */
sptr<IRemoteObject::DeathRecipient> FormHostRecord::GetDeathRecipient() const
{
    return deathRecipient_;
}
/**
 * @brief Set value of deathRecipient_.
 * @param clientImpl DeathRecipient object.
 */
void FormHostRecord::SetDeathRecipient(const sptr<IRemoteObject::DeathRecipient> &deathRecipient)
{
    deathRecipient_ = deathRecipient;
}
/**
 * @brief Add deathRecipient object to clientStub_.
 * @param deathRecipient DeathRecipient object.
 */
void FormHostRecord::AddDeathRecipient(const sptr<IRemoteObject::DeathRecipient> &deathRecipient)
{
    clientStub_->AddDeathRecipient(deathRecipient);
}

/**
 * @brief Create form host record.
 * @param info The form item info.
 * @param callback remote object.
 * @param callingUid Calling uid.
 */
FormHostRecord FormHostRecord::CreateRecord(const FormItemInfo &info, 
    const sptr<IRemoteObject> &callback, int callingUid) 
{  
    FormHostRecord record;
    record.SetHostBundleName(info.GetHostBundleName());
    record.SetCallerUid(callingUid);
    record.SetClientStub(callback);
    record.SetClientImpl(std::make_shared<FormHostCallback>());
    record.SetDeathRecipient(new FormHostRecord::ClientDeathRecipient());
    record.AddDeathRecipient(record.GetDeathRecipient());

    return record;
}

/**
 * @brief handle remote object died event.
 * @param remote remote object.
 */
void FormHostRecord::ClientDeathRecipient::OnRemoteDied(const wptr<IRemoteObject> &remote)
{
    APP_LOGD("Form remote died");
    FormTaskMgr::GetInstance().PostHostDiedTask(remote.promote());
}

/**
 * @brief Get hostBundleName_.
 * @return hostBundleName_.
 */
std::string FormHostRecord::GetHostBundleName() const
{
    return hostBundleName_;
}
/**
 * @brief Set hostBundleName_.
 * @param hostBandleName Host bundle name.
 */
void FormHostRecord::SetHostBundleName(const std::string &hostBundleName)
{
    hostBundleName_ = hostBundleName;
}
}  // namespace AppExecFwk
}  // namespace OHOS
