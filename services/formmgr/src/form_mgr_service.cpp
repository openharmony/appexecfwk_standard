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
#include <functional>
#include <memory>
#include <string>
#include <unistd.h>

#include "appexecfwk_errors.h"
#include "app_log_wrapper.h"
#include "form_ams_helper.h"
#include "form_bms_helper.h"
#include "form_constants.h"
#include "form_data_mgr.h"
#include "form_db_cache.h"
#include "form_mgr_service.h"
#include "form_mgr_adapter.h"
#include "form_task_mgr.h"
#include "form_timer_mgr.h"
#include "ipc_skeleton.h"
#include "iservice_registry.h"
#include "permission/permission_kit.h"
#include "permission/permission.h"
#include "string_ex.h"
#include "system_ability_definition.h"

namespace OHOS {
namespace AppExecFwk {
using namespace std::chrono;
using PermissionKit = OHOS::Security::Permission::PermissionKit;
using PermissionState = OHOS::Security::Permission::PermissionState;

const bool REGISTER_RESULT =
    SystemAbility::MakeAndRegisterAbility(DelayedSingleton<FormMgrService>::GetInstance().get());

const std::string NAME_FORM_MGR_SERVICE = "FormMgrService";

FormMgrService::FormMgrService()
    : SystemAbility(FORM_MGR_SERVICE_ID, true),
      state_(ServiceRunningState::STATE_NOT_START),
      runner_(nullptr),
      handler_(nullptr)
{
}

FormMgrService::~FormMgrService()
{}

bool FormMgrService::IsReady() const
{
    if (state_ != ServiceRunningState::STATE_RUNNING) {
        return false;
    }
    if (!handler_) {
        APP_LOGE("%{public}s fail, handler is null", __func__);
        return false;
    }

    return true;
}

/**
 * @brief Add form with want, send want to form manager service.
 * @param formId The Id of the forms to add.
 * @param want The want of the form to add.
 * @param callerToken Caller ability token.
 * @param formInfo Form info.
 * @return Returns ERR_OK on success, others on failure.
 */
int FormMgrService::AddForm(const int64_t formId, const Want &want, 
    const sptr<IRemoteObject> &callerToken, FormJsInfo &formInfo)
{
    if (!CheckFormPermission()) {
        APP_LOGE("%{public}s fail, add form permission denied", __func__);
        return ERR_APPEXECFWK_FORM_PERMISSION_DENY;
    }
    return FormMgrAdapter::GetInstance().AddForm(formId, want, callerToken, formInfo);
}

/**
 * @brief Delete forms with formIds, send formIds to form manager service.
 * @param formId The Id of the forms to delete.
 * @param callerToken Caller ability token.
 * @return Returns ERR_OK on success, others on failure.
 */
int FormMgrService::DeleteForm(const int64_t formId, const sptr<IRemoteObject> &callerToken)
{
    if (!CheckFormPermission()) {
        APP_LOGE("%{public}s fail, delete form permission denied", __func__);
        return ERR_APPEXECFWK_FORM_PERMISSION_DENY;
    }
        
    return FormMgrAdapter::GetInstance().DeleteForm(formId, callerToken);
}

/**
 * @brief Release forms with formIds, send formIds to form manager service.
 * @param formId The Id of the forms to release.
 * @param callerToken Caller ability token.
 * @param delCache Delete Cache or not.
 * @return Returns ERR_OK on success, others on failure.
 */
int FormMgrService::ReleaseForm(const int64_t formId, const sptr<IRemoteObject> &callerToken, const bool delCache)
{
    if (!CheckFormPermission()) {
        APP_LOGE("%{public}s fail, release form permission denied", __func__);
        return ERR_APPEXECFWK_FORM_PERMISSION_DENY;
    }
    
    return FormMgrAdapter::GetInstance().ReleaseForm(formId, callerToken, delCache);
}

/**
 * @brief Update form with formId, send formId to form manager service.
 * @param formId The Id of the form to update.
 * @param bundleName Provider ability bundleName.
 * @param formBindingData Form binding data.
 * @return Returns ERR_OK on success, others on failure.
 */
int FormMgrService::UpdateForm(const int64_t formId, 
    const std::string &bundleName, const FormProviderData &formBindingData)
{
    if (!CheckFormPermission()) {
        APP_LOGE("%{public}s fail, update form permission denied", __func__);
        return ERR_APPEXECFWK_FORM_PERMISSION_DENY;
    }
    return FormMgrAdapter::GetInstance().UpdateForm(formId, bundleName, formBindingData);
}

/**
 * @brief Request form with formId and want, send formId and want to form manager service.
 * @param formId The Id of the form to update.
 * @param callerToken Caller ability token.
 * @param want The want of the form to add.
 * @return Returns ERR_OK on success, others on failure.
 */
int FormMgrService::RequestForm(const int64_t formId, const sptr<IRemoteObject> &callerToken, const Want &want)
{
    APP_LOGI("%{public}s called.", __func__);

    if (!CheckFormPermission()) {
        APP_LOGE("%{public}s fail, request form permission denied", __func__);
        return ERR_APPEXECFWK_FORM_PERMISSION_DENY;
    }

    return FormMgrAdapter::GetInstance().RequestForm(formId, callerToken, want);
}

/**
 * @brief set next refresh time.
 * @param formId The id of the form.
 * @param bundleManager the bundle manager ipc object.
 * @param nextTime next refresh time.
 * @return Returns ERR_OK on success, others on failure.
 */
int FormMgrService::SetNextRefreshTime(const int64_t formId, const int64_t nextTime)
{
    APP_LOGI("%{public}s called.", __func__);

    return FormMgrAdapter::GetInstance().SetNextRefreshTime(formId, nextTime);
}


/**
 * @brief Form visible/invisible notify, send formIds to form manager service.
 * @param formIds The Id list of the forms to notify.
 * @param callerToken Caller ability token.
 * @param formVisibleType The form visible type, including FORM_VISIBLE and FORM_INVISIBLE.
 * @return Returns ERR_OK on success, others on failure.
 */
int FormMgrService::NotifyWhetherVisibleForms(const std::vector<int64_t> &formIds, 
    const sptr<IRemoteObject> &callerToken, const int32_t formVisibleType)
{
    if (!CheckFormPermission()) {
        APP_LOGE("%{public}s fail, event notify visible permission denied", __func__);
        return ERR_APPEXECFWK_FORM_PERMISSION_DENY;
    }
   
    return FormMgrAdapter::GetInstance().NotifyWhetherVisibleForms(formIds, callerToken, formVisibleType);
}

/**
 * @brief temp form to normal form.
 * @param formId The Id of the form.
 * @param callerToken Caller ability token.
 * @return Returns ERR_OK on success, others on failure.
 */
int FormMgrService::CastTempForm(const int64_t formId, const sptr<IRemoteObject> &callerToken)
{
    if (!CheckFormPermission()) {
        APP_LOGE("%{public}s fail, cast temp form permission denied", __func__);
        return ERR_APPEXECFWK_FORM_PERMISSION_DENY;
    }

    return FormMgrAdapter::GetInstance().CastTempForm(formId, callerToken);
}

/**
 * @brief lifecycle update.
 * @param formIds formIds of hostclient.
 * @param callerToken Caller ability token.
 * @param updateType update type,enable or disable.
 * @return Returns true on success, false on failure.
 */
int FormMgrService::LifecycleUpdate(const std::vector<int64_t> &formIds, 
    const sptr<IRemoteObject> &callerToken, const int32_t updateType)
{
    APP_LOGI("lifecycleUpdate.");

    if (!CheckFormPermission()) {
        APP_LOGE("%{public}s fail, delete form permission denied", __func__);
        return ERR_APPEXECFWK_FORM_PERMISSION_DENY;
    }
 
    if (updateType == ENABLE_FORM_UPDATE) {
        return FormMgrAdapter::GetInstance().EnableUpdateForm(formIds, callerToken);
    } else {
        return FormMgrAdapter::GetInstance().DisableUpdateForm(formIds, callerToken);
    }
}
/**
 * @brief Dump all of form storage infos.
 * @param formInfos All of form storage infos.
 * @return Returns ERR_OK on success, others on failure.
 */
int FormMgrService::DumpStorageFormInfos(std::string &formInfos)
{
    return FormMgrAdapter::GetInstance().DumpStorageFormInfos(formInfos);
}
/**
 * @brief Dump form info by a bundle name.
 * @param bundleName The bundle name of form provider.
 * @param formInfos Form infos.
 * @return Returns ERR_OK on success, others on failure.
 */
int FormMgrService::DumpFormInfoByBundleName(const std::string &bundleName, std::string &formInfos)
{
    return FormMgrAdapter::GetInstance().DumpFormInfoByBundleName(bundleName, formInfos);
}
/**
 * @brief Dump form info by a bundle name.
 * @param formId The id of the form.
 * @param formInfo Form info.
 * @return Returns ERR_OK on success, others on failure.
 */
int FormMgrService::DumpFormInfoByFormId(const std::int64_t formId, std::string &formInfo)
{
    return FormMgrAdapter::GetInstance().DumpFormInfoByFormId(formId, formInfo);
}
/**
 * @brief Process js message event.
 * @param formId Indicates the unique id of form.
 * @param want information passed to supplier.
 * @param callerToken Caller ability token.
 * @return Returns true if execute success, false otherwise.
 */
int FormMgrService::MessageEvent(const int64_t formId, const Want &want, const sptr<IRemoteObject> &callerToken)
{
    APP_LOGI("%{public}s called.", __func__);

    if (!CheckFormPermission()) {
        APP_LOGE("%{public}s fail, request form permission denied", __func__);
        return ERR_APPEXECFWK_FORM_PERMISSION_DENY;
    }

    return FormMgrAdapter::GetInstance().MessageEvent(formId, want, callerToken);
}
/**
 * @brief Start envent for the form manager service.
 */
void FormMgrService::OnStart()
{
    if (state_ == ServiceRunningState::STATE_RUNNING) {
        APP_LOGW("%{public}s fail, Failed to start service since it's already running", __func__);
        return;
    }

    APP_LOGI("Form Mgr Service start...");
    ErrCode errCode = Init();
    if (errCode != ERR_OK) {
        APP_LOGE("%{public}s fail, Failed to init, errCode: %{public}08x", __func__, errCode);
        return;
    }

    state_ = ServiceRunningState::STATE_RUNNING;

    APP_LOGI("Form Mgr Service start success.");
}
/**
 * @brief Stop envent for the form manager service.
 */
void FormMgrService::OnStop()
{
    APP_LOGI("stop service");

    state_ = ServiceRunningState::STATE_NOT_START;

    if (handler_) {
        handler_.reset();
    }

    if (runner_) {
        runner_.reset();
    }
}
/**
 * @brief initialization of form manager service.
 */
ErrCode FormMgrService::Init()
{
    runner_ = EventRunner::Create(NAME_FORM_MGR_SERVICE);
    if (!runner_) {
        APP_LOGE("%{public}s fail, Failed to init due to create runner error", __func__);
        return ERR_INVALID_OPERATION;
    }
    handler_ = std::make_shared<EventHandler>(runner_);
    if (!handler_) {
        APP_LOGE("%{public}s fail, Failed to init due to create handler error", __func__);
        return ERR_INVALID_OPERATION;
    }
    FormTaskMgr::GetInstance().SetEventHandler(handler_);
    FormAmsHelper::GetInstance().SetEventHandler(handler_);
    /* Publish service maybe failed, so we need call this function at the last,
     * so it can't affect the TDD test program */
    bool ret = Publish(DelayedSingleton<FormMgrService>::GetInstance().get());   
    if (!ret) {
        APP_LOGE("%{public}s fail, FormMgrService::Init Publish failed!", __func__);
        return ERR_INVALID_OPERATION;
    }

    FormDbCache::GetInstance().Start();

    APP_LOGI("init success");
    return ERR_OK;
}
/**
 * @brief Permission check by callingUid.
 * @param formId the id of the form.
 * @return Returns true on success, false on failure.
 */
bool FormMgrService::CheckFormPermission()
{
    sptr<IBundleMgr> iBundleMgr = FormBmsHelper::GetInstance().GetBundleMgr();
    if (iBundleMgr == nullptr) {
        APP_LOGE("%{public}s, failed to get IBundleMgr.", __func__);
        return false;
    }

    int32_t uid = IPCSkeleton::GetCallingUid();
    if (!iBundleMgr->CheckIsSystemAppByUid(uid)) {
        APP_LOGE("%{public}s fail, form is not system app. uid:%{public}d", __func__, uid);
        return false;
    }

    std::string bundleName;
    bool result = iBundleMgr->GetBundleNameForUid(uid, bundleName);
    if (!result || bundleName.empty()) {
        APP_LOGE("%{public}s failed, cannot get bundle name by uid:%{public}d", __func__, uid);
        return false;
    }

    return CheckFormPermission(bundleName);
}

bool FormMgrService::CheckFormPermission(const std::string &bundleName) const
{
    if (bundleName.empty()) {
        APP_LOGE("%{public}s fail, bundleName can not be empty", __func__);
        return false;
    }
    int result = PermissionKit::VerifyPermission(bundleName, Constants::PERMISSION_REQUIRE_FORM, 0);
    if (result != PermissionState::PERMISSION_GRANTED) {
        APP_LOGW("permission = %{public}s, bundleName = %{public}s, result = %{public}d", 
            Constants::PERMISSION_REQUIRE_FORM.c_str(), bundleName.c_str(), result);
    }
    return result == PermissionState::PERMISSION_GRANTED;
}
}  // namespace AppExecFwk
}  // namespace OHOS
