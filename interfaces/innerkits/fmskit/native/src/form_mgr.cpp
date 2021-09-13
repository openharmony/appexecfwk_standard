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

#include <thread>
#include "appexecfwk_errors.h"
#include "app_log_wrapper.h"
#include "form_mgr.h"
#include "if_system_ability_manager.h"
#include "ipc_skeleton.h"
#include "iservice_registry.h"
#include "string_ex.h"
#include "system_ability_definition.h"

namespace OHOS {
namespace AppExecFwk {
FormMgr::FormMgr(){}
FormMgr::~FormMgr()
{
    if (remoteProxy_ != nullptr) {
        auto remoteObject = remoteProxy_->AsObject();
        if (remoteObject != nullptr) {
            remoteObject->RemoveDeathRecipient(deathRecipient_);
        }
    }
}
/**
 * @brief Add form with want, send want to form manager service.
 * @param formId The Id of the forms to add.
 * @param want The want of the form to add.
 * @param callerToken Caller ability token.
 * @param formInfo Form info.
 * @return Returns ERR_OK on success, others on failure.
 */
int FormMgr::AddForm(const int64_t formId, const Want &want, const sptr<IRemoteObject> &callerToken, 
FormJsInfo &formInfo)
{
    APP_LOGI("%{public}s called.", __func__);

    int errCode = Connect();
    if (errCode != ERR_OK) {
        return errCode;
    }

    return remoteProxy_->AddForm(formId, want, callerToken, formInfo);
}

/**
 * @brief Delete forms with formIds, send formIds to form manager service.
 * @param formId The Id of the forms to delete.
 * @param callerToken Caller ability token.
 * @return Returns ERR_OK on success, others on failure.
 */
int FormMgr::DeleteForm(const int64_t formId, const sptr<IRemoteObject> &callerToken)
{
    APP_LOGI("%{public}s called.", __func__);

    int errCode = Connect();
    if (errCode != ERR_OK) {
        return errCode;
    }
  
    return remoteProxy_->DeleteForm(formId, callerToken);
}

/**
 * @brief Release forms with formIds, send formIds to form manager service.
 * @param formId The Id of the forms to release.
 * @param callerToken Caller ability token.
 * @param delCache Delete Cache or not.
 * @return Returns ERR_OK on success, others on failure.
 */
int FormMgr::ReleaseForm(const int64_t formId, const sptr<IRemoteObject> &callerToken, const bool delCache)
{
    APP_LOGI("%{public}s called.", __func__);

    int errCode = Connect();
    if (errCode != ERR_OK) {
        return errCode;
    }

    return remoteProxy_->ReleaseForm(formId, callerToken, delCache);
}

/**
 * @brief Update form with formId, send formId to form manager service.
 * @param formId The Id of the form to update.
 * @param bundleName Provider ability bundleName.
 * @param formBindingData Form binding data.
 * @return Returns ERR_OK on success, others on failure.
 */
int FormMgr::UpdateForm(const int64_t formId, const std::string &bundleName, const FormProviderData &formBindingData)
{
    APP_LOGI("%{public}s called.", __func__);

    int errCode = Connect();
    if (errCode != ERR_OK) {
        return errCode;
    }

    // update form
    return remoteProxy_->UpdateForm(formId, bundleName, formBindingData);
}

/**
 * @brief Notify the form service that the form user's lifecycle is updated.
 *
 * This should be called when form user request form.
 *
 * @param formId Indicates the unique id of form.
 * @param callerToken Indicates the callback remote object of specified form user.
 * @param want information passed to supplier.
 * @return Returns true if execute success, false otherwise.
 */
int FormMgr::RequestForm(const int64_t formId, const sptr<IRemoteObject> &callerToken, const Want &want)
{
    APP_LOGI("%{public}s called.", __func__);
    
    int errCode = Connect();
    if (errCode != ERR_OK) {
        return errCode;
    }

    return remoteProxy_->RequestForm(formId, callerToken, want);
}

/**
 * @brief Form visible/invisible notify, send formIds to form manager service.
 * @param formIds The Id list of the forms to notify.
 * @param callerToken Caller ability token.
 * @param formVisibleType The form visible type, including FORM_VISIBLE and FORM_INVISIBLE.
 * @return Returns ERR_OK on success, others on failure.
 */
int FormMgr::NotifyWhetherVisibleForms(const std::vector<int64_t> &formIds, const sptr<IRemoteObject> &callerToken, 
const int32_t formVisibleType)
{
    int errCode = Connect();
    if (errCode != ERR_OK) {
        return errCode;
    }

    return remoteProxy_->NotifyWhetherVisibleForms(formIds, callerToken, formVisibleType);
}

/**
 * @brief temp form to normal form.
 * @param formId The Id of the form.
 * @param callerToken Caller ability token.
 * @return None.
 */
int FormMgr::CastTempForm(const int64_t formId, const sptr<IRemoteObject> &callerToken)
{
    APP_LOGI("%{public}s called.", __func__);

    int errCode = Connect();
    if (errCode != ERR_OK) {
        return errCode;
    }
    
    return remoteProxy_->CastTempForm(formId, callerToken);
}

/**
 * @brief Dump all of form storage infos.
 * @param formInfos All of form storage infos.
 * @return Returns ERR_OK on success, others on failure.
 */
int FormMgr::DumpStorageFormInfos(std::string &formInfos)
{
    int errCode = Connect();
    if (errCode != ERR_OK) {
        return errCode;
    }
    
    return remoteProxy_->DumpStorageFormInfos(formInfos);
}
/**
 * @brief Dump form info by a bundle name.
 * @param bundleName The bundle name of form provider.
 * @param formInfos Form infos.
 * @return Returns ERR_OK on success, others on failure.
 */
int FormMgr::DumpFormInfoByBundleName(const std::string bundleName, std::string &formInfos)
{
    int errCode = Connect();
    if (errCode != ERR_OK) {
        return errCode;
    }
    
    return remoteProxy_->DumpFormInfoByBundleName(bundleName, formInfos);
}
/**
 * @brief Dump form info by a bundle name.
 * @param formId The id of the form.
 * @param formInfo Form info.
 * @return Returns ERR_OK on success, others on failure.
 */
int FormMgr::DumpFormInfoByFormId(const std::int64_t formId, std::string &formInfo)
{
    int errCode = Connect();
    if (errCode != ERR_OK) {
        return errCode;
    }
    
    return remoteProxy_->DumpFormInfoByFormId(formId, formInfo);
}
/**
 * @brief Process js message event.
 * @param formId Indicates the unique id of form.
 * @param want information passed to supplier.
 * @param callerToken Caller ability token.
 * @return Returns true if execute success, false otherwise.
 */
int FormMgr::MessageEvent(const int64_t formId, const Want &want, const sptr<IRemoteObject> &callerToken)
{
    int errCode = Connect();
    if (errCode != ERR_OK) {
        return errCode;
    }
    
    return remoteProxy_->MessageEvent(formId, want, callerToken);
}

/**
 * @brief Set Next Refresh Time.
 * @param formId The id of the form.
 * @param bundleName The bundle name of form provider.
 * @param nextTime Next refresh time.
 * @return Returns ERR_OK on success, others on failure.
 */
int FormMgr::SetNextRefreshTime(const int64_t formId, const int64_t nextTime)
{
    int errCode = Connect();
    if (errCode != ERR_OK) {
        return errCode;
    }
  
    return remoteProxy_->SetNextRefreshTime(formId, nextTime);
}

/**
 * @brief Lifecycle Update.
 * @param formIds The id of the forms.
 * @param callerToken Host client.
 * @param updateType Next refresh time.
 * @return Returns ERR_OK on success, others on failure.
 */
int FormMgr::LifecycleUpdate(const std::vector<int64_t> &formIds, const sptr<IRemoteObject> &callerToken, 
const int32_t updateType)
{
    int errCode = Connect();
    if (errCode != ERR_OK) {
        return errCode;
    }
  
    return remoteProxy_->LifecycleUpdate(formIds, callerToken, updateType);
}
/**
 * @brief Get fms recoverStatus.
 *
 * @return The current recover status.
 */
int FormMgr::GetRecoverStatus()
{
    APP_LOGI("%{public}s called.", __func__);

    return recoverStatus_;
}

/**
 * @brief Set fms recoverStatus.
 *
 * @param recoverStatus The recover status.
 */
void FormMgr::SetRecoverStatus(int recoverStatus)
{
    APP_LOGI("%{public}s called.", __func__);

    recoverStatus_ = recoverStatus;
}

/**
 * @brief Register death callback.
 * 
 * @param deathCallback Death callback.
 */
void FormMgr::RegisterDeathCallback(const std::shared_ptr<FormCallbackInterface> &formDeathCallback)
{
    APP_LOGI("%{public}s called.", __func__);

    if (formDeathCallback == nullptr) {
        APP_LOGE("%{public}s error, form death callback is nullptr.", __func__);
        return;
    }

    formDeathCallbacks_.emplace_back(formDeathCallback);
}

/**
 * @brief UnRegister death callback.
 * 
 * @param deathCallback Death callback.
 */
void FormMgr::UnRegisterDeathCallback(const std::shared_ptr<FormCallbackInterface> &formDeathCallback)
{
    APP_LOGI("%{public}s called.", __func__);

    if (formDeathCallback == nullptr) {
        APP_LOGE("%{public}s error, form death callback is nullptr.", __func__);
        return;
    }

    // Remove the specified death callback in the vector of death callback
    auto iter = std::find(formDeathCallbacks_.begin(), formDeathCallbacks_.end(), formDeathCallback);
    if (iter != formDeathCallbacks_.end()) {
        formDeathCallbacks_.erase(iter);
    }
    APP_LOGI("%{public}s end.", __func__);
}

/**
 * @brief Get death recipient.
 * @return deathRecipient_.
 */
sptr<IRemoteObject::DeathRecipient> FormMgr::GetDeathRecipient() const
{
    return deathRecipient_;
}

/**
 * @brief Check whether the specified death callback is registered in form mgr.
 * @param formDeathCallback The specified death callback for checking.
 * @return Return true on success, false on failure.
 */
bool FormMgr::CheckIsDeathCallbackRegistered(const std::shared_ptr<FormCallbackInterface> &formDeathCallback)
{
    APP_LOGI("%{public}s called.", __func__);

    auto iter = std::find(formDeathCallbacks_.begin(), formDeathCallbacks_.end(), formDeathCallback);
    if (iter != formDeathCallbacks_.end()) {
        return true;
    }

    return false;
}

/**
 * @brief Notices IRemoteBroker died.
 * @param remote remote object.
 */
void FormMgr::FormMgrDeathRecipient::OnRemoteDied(const wptr<IRemoteObject> &remote)
{
    APP_LOGI("%{public}s called.", __func__);

    if (remote == nullptr) {
        APP_LOGE("%{public}s failed, remote is nullptr.", __func__);
        return;
    }

    if (FormMgr::GetInstance().GetRecoverStatus() == Constants::IN_RECOVERING) {
        APP_LOGW("%{public}s, fms in recovering.", __func__);
        return;
    }
    // Reset proxy
    FormMgr::GetInstance().ResetProxy(remote);

    if (!FormMgr::GetInstance().Reconnect()) {
        APP_LOGE("%{public}s, form mgr service died, try to reconnect to fms failed.", __func__);
        FormMgr::GetInstance().SetRecoverStatus(Constants::RECOVER_FAIL);
        return;
    }

    // refresh form host.
    for (auto &deathCallback : FormMgr::GetInstance().formDeathCallbacks_) {
        deathCallback->OnDeathReceived();
    }
    FormMgr::GetInstance().SetRecoverStatus(Constants::NOT_IN_RECOVERY);
}

/**
 * @brief Reconnect form manager service once per 1000 milliseconds,
 *        until the connection succeeds or reaching the max retry times.
 * @return Returns true if execute success, false otherwise.
 */
bool FormMgr::Reconnect()
{
    APP_LOGI("%{public}s called.", __func__);

    for (int i = 0; i < Constants::MAX_RETRY_TIME; i++) {
        // Sleep 1000 milliseconds before reconnect.
        std::this_thread::sleep_for(std::chrono::milliseconds(Constants::SLEEP_TIME));

        // try to connect fms
        if (Connect() != ERR_OK) {
            APP_LOGE("%{public}s, get fms proxy fail, try again.", __func__);
            continue;
        }

        APP_LOGI("%{public}s, get fms proxy success.", __func__);
        return true;
    }

    return false;
}

/**
 * @brief Connect form manager service.
 * @return Returns ERR_OK on success, others on failure.
 */
ErrCode FormMgr::Connect()
{
    std::lock_guard<std::mutex> lock(connectMutex_);
    if (remoteProxy_ != nullptr && !resetFlag_) {
        return ERR_OK;
    }
    
    sptr<ISystemAbilityManager> systemManager = SystemAbilityManagerClient::GetInstance().GetSystemAbilityManager();
    if (systemManager == nullptr) {
        APP_LOGE("%{private}s:fail to get registry", __func__);
        return ERR_APPEXECFWK_FORM_GET_SYSMGR_FAILED;
    }
    sptr<IRemoteObject> remoteObject = systemManager->GetSystemAbility(FORM_MGR_SERVICE_ID);
    if (remoteObject == nullptr) {
        APP_LOGE("%{private}s:fail to connect FormMgrService", __func__);
        return ERR_APPEXECFWK_FORM_GET_FMS_FAILED;
    }
    deathRecipient_ = sptr<IRemoteObject::DeathRecipient>(new FormMgrDeathRecipient());
    if (deathRecipient_ == nullptr) {
        APP_LOGE("%{public}s :Failed to create FormMgrDeathRecipient!", __func__);
        return ERR_NO_MEMORY;
    }
    if ((remoteObject->IsProxyObject()) && (!remoteObject->AddDeathRecipient(deathRecipient_))) {
        APP_LOGE("%{public}s :Add death recipient to FormMgrService failed.", __func__);
        return ERR_APPEXECFWK_FORM_ADD_DEATH_RECIPIENT_FAILED;
    }

    remoteProxy_ = iface_cast<IFormMgr>(remoteObject);
    APP_LOGD("%{public}s :Connecting FormMgrService success.", __func__);
    return ERR_OK;
}

/**
 * @brief Reset proxy.
 * @param remote remote object.
 */
void FormMgr::ResetProxy(const wptr<IRemoteObject> &remote)
{
    APP_LOGI("%{public}s called.", __func__);

    std::lock_guard<std::mutex> lock(connectMutex_);
    if (remoteProxy_ == nullptr) {
        APP_LOGE("%{public}s failed, remote proxy is nullptr.", __func__);
        return;
    }

    // set formMgr's recover status to IN_RECOVERING.
    recoverStatus_ = Constants::IN_RECOVERING;

    // remove the death recipient
    auto serviceRemote = remoteProxy_->AsObject();
    if ((serviceRemote != nullptr) && (serviceRemote == remote.promote())) {
        serviceRemote->RemoveDeathRecipient(deathRecipient_);
    }
    // clearn the remote proxy
    remoteProxy_ = nullptr;
}

/**
 * @brief Set form mgr service for test.
 */
void FormMgr::SetFormMgrService(sptr<IFormMgr> formMgrService)
{
    APP_LOGI("%{public}s called.", __func__);
    remoteProxy_ = formMgrService;
}
}  // namespace AppExecFwk
}  // namespace OHOS
