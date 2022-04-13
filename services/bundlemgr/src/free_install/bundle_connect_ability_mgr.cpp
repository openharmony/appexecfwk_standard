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

#include "bundle_connect_ability_mgr.h"

#include "app_log_wrapper.h"
#include "base_task_dispatcher.h"
#include "bundle_mgr_service.h"
#include "event_handler.h"
#include "event_runner.h"
#include "free_install_code.h"
#include "json_util.h"
#include "parcel.h"
#include "service_center_death_recipient.h"
#include "service_center_status_callback.h"
#include "string_ex.h"
#include "task_dispatcher.h"
#include "task_dispatcher_context.h"

namespace OHOS {
namespace AppExecFwk {
const std::u16string ATOMIC_SERVICE_STATUS_CALLBACK_TOKEN = u"ohos.aafwk.IAtomicServiceStatusCallback";
const std::string serviceCenterBundleName = "com.ohos.hag.famanager";
const std::string serviceCenterAbilityName = "com.ohos.hag.famanager.HapInstallServiceAbility";
constexpr uint32_t FREE_INSTALL_DONE = 0;
constexpr uint32_t CALLING_TYPE_HARMONY = 2;
constexpr uint32_t BIT_ONE_COMPATIBLE = 0;
constexpr uint32_t BIT_TWO_BACK_MODE = 1;
constexpr uint32_t BIT_THREE_CUSTOM = 0;
constexpr uint32_t BIT_ONE_FIVE_AZ_DEVICE = 0;
constexpr uint32_t BIT_ONE_SEVEN_SAME_BUNDLE = 0;
constexpr uint32_t BIT_TWO = 2;
constexpr uint32_t BIT_THREE = 4;
constexpr uint32_t BIT_FOUR = 8;
constexpr uint32_t BIT_FIVE = 16;
constexpr uint32_t BIT_SIX = 32;
constexpr uint32_t BIT_SEVEN = 64;
constexpr uint32_t OUT_TIME = 30000;
sptr<ServiceCenterStatusCallback> serviceCenterCallback;

BundleConnectAbilityMgr::BundleConnectAbilityMgr()
{
}

BundleConnectAbilityMgr::~BundleConnectAbilityMgr()
{
}

bool BundleConnectAbilityMgr::SilentInstallSafely(const TargetAbilityInfo &targetAbilityInfo, const Want &want,
    const sptr<IRemoteObject> &callerToken, int32_t userId)
{
    APP_LOGI("SilentInstallSafely");
    std::shared_ptr<TaskDispatcherContext> context = std::make_shared<TaskDispatcherContext>();
    TaskPriority defaultPriority = TaskPriority::DEFAULT;
    std::shared_ptr<TaskDispatcher> ptrGlobalTaskDispatcher = context->GetGlobalTaskDispatcher(defaultPriority);

    if (!ptrGlobalTaskDispatcher) {
        APP_LOGE("BundleConnectAbilityMgr::SilentInstallSafely ptrGlobalTaskDispatcher is nullptr");
        SendCallBack(
            FreeInstallErrorCode::FREE_INSTALL_UNDEFINED_ERROR, want, userId, targetAbilityInfo.targetInfo.transactId);
        return false;
    }
    std::shared_ptr<Runnable> func = std::make_shared<Runnable>([&]() {
        int32_t flag = ServiceCenterFunction::SERVICE_CENTER_CONNECT_SILENT_INSTALL;
        return RunnableFun(flag, targetAbilityInfo, want, userId, callerToken);
    });
    auto task = ptrGlobalTaskDispatcher->AsyncDispatch(func);
    if (!task) {
        APP_LOGE("BundleConnectAbilityMgr::SilentInstallSafely Asunc task is nullptr");
        SendCallBack(
            FreeInstallErrorCode::FREE_INSTALL_UNDEFINED_ERROR, want, userId, targetAbilityInfo.targetInfo.transactId);
        return false;
    }
    return true;
}

bool BundleConnectAbilityMgr::UpgradeCheckSafely(const TargetAbilityInfo &targetAbilityInfo, const Want &want,
    const sptr<IRemoteObject> &callerToken, int32_t userId)
{
    APP_LOGI("UpgradeCheckSafely");
    std::shared_ptr<TaskDispatcherContext> context = std::make_shared<TaskDispatcherContext>();
    TaskPriority defaultPriority = TaskPriority::DEFAULT;
    std::shared_ptr<TaskDispatcher> ptrGlobalTaskDispatcher = context->GetGlobalTaskDispatcher(defaultPriority);
    if (!ptrGlobalTaskDispatcher) {
        APP_LOGE("BundleConnectAbilityMgr::UpgradeCheckSafely ptrGlobalTaskDispatcher is nullptr");
        SendCallBack(
            FreeInstallErrorCode::FREE_INSTALL_UNDEFINED_ERROR, want, userId, targetAbilityInfo.targetInfo.transactId);
        return false;
    }
    std::shared_ptr<Runnable> func = std::make_shared<Runnable>([&]() {
        int32_t flag = ServiceCenterFunction::SERVICE_CENTER_CONNECT_UPGRADE_CHECK;
        return RunnableFun(flag, targetAbilityInfo, want, userId, callerToken);
    });

    auto task = ptrGlobalTaskDispatcher->AsyncDispatch(func);
    if (!task) {
        APP_LOGE("BundleConnectAbilityMgr::UpgradeCheckSafely Async task is nullptr");
        SendCallBack(
            FreeInstallErrorCode::FREE_INSTALL_UNDEFINED_ERROR, want, userId, targetAbilityInfo.targetInfo.transactId);
        return false;
    }
    return true;
}

bool BundleConnectAbilityMgr::UpgradeInstallSafely(const TargetAbilityInfo &targetAbilityInfo, const Want &want,
    const sptr<IRemoteObject> &callerToken, int32_t userId)
{
    APP_LOGI("UpgradeInstallSafely");
    std::shared_ptr<TaskDispatcherContext> context = std::make_shared<TaskDispatcherContext>();
    TaskPriority defaultPriority = TaskPriority::DEFAULT;
    std::shared_ptr<TaskDispatcher> ptrGlobalTaskDispatcher = context->GetGlobalTaskDispatcher(defaultPriority);

    if (!ptrGlobalTaskDispatcher) {
        APP_LOGE("BundleConnectAbilityMgr::UpgradeInstallSafely ptrGlobalTaskDispatcher is nullptr");
        SendCallBack(
            FreeInstallErrorCode::FREE_INSTALL_UNDEFINED_ERROR, want, userId, targetAbilityInfo.targetInfo.transactId);
        return false;
    }

    std::shared_ptr<Runnable> func = std::make_shared<Runnable>([&]() {
        int32_t flag = ServiceCenterFunction::SERVICE_CENTER_CONNECT_UPGRADE_INSTALL;
        return RunnableFun(flag, targetAbilityInfo, want, userId, callerToken);
    });
    auto task = ptrGlobalTaskDispatcher->AsyncDispatch(func);
    if (!task) {
        APP_LOGE("BundleConnectAbilityMgr::UpgradeInstallSafely Async task is nullptr");
        SendCallBack(
            FreeInstallErrorCode::FREE_INSTALL_UNDEFINED_ERROR, want, userId, targetAbilityInfo.targetInfo.transactId);
        return false;
    }
    return true;
}

bool BundleConnectAbilityMgr::RunnableFun(int32_t flag, const TargetAbilityInfo &targetAbilityInfo, const Want &want,
    int32_t userId, const sptr<IRemoteObject> &callerToken)
{
    APP_LOGI("RunnableFun start");
    Want serviceCenterWant;
    serviceCenterWant.SetElementName(serviceCenterBundleName, serviceCenterAbilityName);
    bool isConnectSuccess = ConnectAbility(serviceCenterWant, callerToken);
    if (!isConnectSuccess) {
        APP_LOGE("fail to connect ServiceCenter");
        SendCallBack(FreeInstallErrorCode::FREE_INSTALL_CONNECT_ERROR, want,
            userId, targetAbilityInfo.targetInfo.transactId);
        return false;
    } else {
        SendRequest(flag, targetAbilityInfo, want, userId);
        return true;
    }
}

void BundleConnectAbilityMgr::DisconnectAbility()
{
    if (serviceCenterConnection_ != nullptr) {
        APP_LOGI("DisconnectAbility");
        abilityMgrProxy_->DisconnectAbility(serviceCenterConnection_);
    }
}

bool BundleConnectAbilityMgr::ConnectAbility(const Want &want, const sptr<IRemoteObject> &callerToken)
{
    APP_LOGI("ConnectAbility start target bundle = %{public}s", want.GetBundle().c_str());
    std::unique_lock<std::mutex> lock(mutex_);
    if (connectState_ == ServiceCenterConnectState::SERVICE_CENTER_CONNECTING) {
        APP_LOGI("ConnectAbility await start SERVICE_CENTER_CONNECTING");
        cv_.wait(lock);
        APP_LOGI("ConnectAbility await end SERVICE_CENTER_CONNECTING");
    } else if (connectState_ == ServiceCenterConnectState::SERVICE_CENTER_DISCONNECTED) {
        connectState_ = ServiceCenterConnectState::SERVICE_CENTER_CONNECTING;
        if (!GetAbilityMgrProxy()) {
            cv_.notify_all();
            return false;
        }

        serviceCenterConnection_ = new (std::nothrow) ServiceCenterConnection(connectState_,
            cv_, freeInstallParamsMap_);
        if (serviceCenterConnection_ == nullptr) {
            APP_LOGE("mServiceCenterConnection is nullptr");
            cv_.notify_all();
            return false;
        }
        APP_LOGI("Start ConnectAbility");
        int result = abilityMgrProxy_->ConnectAbility(want, serviceCenterConnection_, callerToken);
        if (result == ERR_OK) {
            if (connectState_ != ServiceCenterConnectState::SERVICE_CENTER_CONNECTED) {
                APP_LOGI("ConnectAbility await start SERVICE_CENTER_CONNECTED");
                cv_.wait(lock);
                APP_LOGI("ConnectAbility await end SERVICE_CENTER_CONNECTED");
                serviceCenterRemoteObject_ = serviceCenterConnection_->GetRemoteObject();
            }
        } else {
            APP_LOGE("ConnectAbility fail result = %{public}d", result);
        }
    }

    APP_LOGI("ConnectAbility end");
    if (connectState_ == ServiceCenterConnectState::SERVICE_CENTER_CONNECTED) {
        return true;
    } else {
        connectState_ = ServiceCenterConnectState::SERVICE_CENTER_DISCONNECTED;
        return false;
    }
}

bool BundleConnectAbilityMgr::GetAbilityMgrProxy()
{
    if (abilityMgrProxy_ == nullptr) {
        abilityMgrProxy_ =
            iface_cast<AAFwk::IAbilityManager>(SystemAbilityHelper::GetSystemAbility(ABILITY_MGR_SERVICE_ID));
    }
    if ((abilityMgrProxy_ == nullptr) || (abilityMgrProxy_->AsObject() == nullptr)) {
        APP_LOGE("Failed to get system ability manager services ability");
        return false;
    }
    return true;
}

void BundleConnectAbilityMgr::SendCallBack(
    int32_t resultCode, const AAFwk::Want &want, int32_t userId, std::string transactId)
{
    sptr<IRemoteObject> amsCallBack = GetAbilityManagerServiceCallBack(transactId);
    if (amsCallBack == nullptr) {
        APP_LOGE("amsCallBack is nullptr");
        DisconnectAbility();
        return;
    }

    MessageParcel data;
    if (!data.WriteInterfaceToken(ATOMIC_SERVICE_STATUS_CALLBACK_TOKEN)) {
        APP_LOGE("Write interface token failed");
        return;
    }
    if (!data.WriteInt32(resultCode)) {
        APP_LOGE("Write result code error");
        return;
    }
    if (!data.WriteParcelable(&want)) {
        APP_LOGE("Write want failed");
        return;
    }
    if (!data.WriteInt32(userId)) {
        APP_LOGE("Write userId error");
        return;
    }
    MessageParcel reply;
    MessageOption option;
    
    if (amsCallBack->SendRequest(FREE_INSTALL_DONE, data, reply, option) != OHOS::NO_ERROR) {
        APP_LOGE("BundleConnectAbilityMgr::SendCallBack SendRequest failed");
    }

    if (freeInstallParamsMap_.erase(transactId) && freeInstallParamsMap_.size() == 0) {
        if (connectState_ == ServiceCenterConnectState::SERVICE_CENTER_CONNECTED) {
            APP_LOGD("Disconnect Ability.");
            DisconnectAbility();
        }
    }
}

void BundleConnectAbilityMgr::OnServiceCenterCall(std::string installResultStr)
{
    APP_LOGD("OnServiceCenterCall, installResultStr = %{public}s", installResultStr.c_str());
    InstallResult installResult;
    if (!ParseInfoFromJsonStr(installResultStr.c_str(), installResult)) {
        APP_LOGE("Parse info from json fail");
        return;
    }
    APP_LOGD("OnServiceCenterCall, retCode = %{public}d", installResult.result.retCode);
    FreeInstallParams freeInstallParams;
    auto node = freeInstallParamsMap_.find(installResult.result.transactId);
    if (node == freeInstallParamsMap_.end()) {
        APP_LOGE("can not find node");
        return;
    }
    freeInstallParams = node->second;
    freeInstallParams.handler.reset();
    int32_t resultCode = installResult.result.retCode;
    if (resultCode == ServiceCenterResultCode::FREE_INSTALL_DOWNLOADING) {
        APP_LOGD("ServiceCenter is downloading, downloadSize = %{public}d, totalSize = %{public}d",
            installResult.progress.downloadSize, installResult.progress.totalSize);
        return;
    }
    APP_LOGD("serviceCenterFunction = %{public}d", freeInstallParams.serviceCenterFunction);
    SendCallBack(resultCode, freeInstallParams.want, freeInstallParams.userId,
        installResult.result.transactId);
    APP_LOGD("OnServiceCenterCall end");
}

void BundleConnectAbilityMgr::OutTimeMonitor(std::string transactId)
{
    APP_LOGD("OutTimeMonitor, start");
    FreeInstallParams freeInstallParams;
    auto node = freeInstallParamsMap_.find(transactId);
    if (node == freeInstallParamsMap_.end()) {
        APP_LOGI("can not find node");
        return;
    }
    freeInstallParams = node->second;
    std::shared_ptr<AppExecFwk::EventHandler> handler = freeInstallParams.handler;
    if (handler == nullptr) {
        APP_LOGE("OutTimeMonitor, handler is nullptr");
        return;
    }
    auto RegisterEventListenerFunc = [this, freeInstallParams, transactId]() {
        this->SendCallBack(FreeInstallErrorCode::FREE_INSTALL_SERVICE_CENTER_TIMEOUT,
            freeInstallParams.want, freeInstallParams.userId, transactId);
        APP_LOGD("RegisterEventListenerFunc end");
    };
    handler->PostTask(RegisterEventListenerFunc, OUT_TIME, AppExecFwk::EventQueue::Priority::LOW);
}

void BundleConnectAbilityMgr::SendRequest(
    int32_t flag, const TargetAbilityInfo &targetAbilityInfo, const Want &want, int32_t userId)
{
    APP_LOGI("start to SendRequest");
    MessageParcel data;
    MessageParcel reply;
    MessageOption option(MessageOption::TF_ASYNC);
    const std::string dataString = GetJsonStrFromInfo(targetAbilityInfo);
    APP_LOGD("TargetAbilityInfo - > ToJsonString : %{public}s", dataString.c_str());

    if (!data.WriteString16(Str8ToStr16(dataString))) {
        APP_LOGE("%{public}s failed to WriteParcelable targetAbilityInfo", __func__);
        SendCallBack(FreeInstallErrorCode::FREE_INSTALL_UNDEFINED_ERROR, want, userId,
            targetAbilityInfo.targetInfo.transactId);
        return;
    }
    if (serviceCenterCallback == nullptr) {
        serviceCenterCallback = new ServiceCenterStatusCallback(weak_from_this());
    }
    if (!data.WriteRemoteObject(serviceCenterCallback)) {
        APP_LOGE("%{public}s failed to WriteRemoteObject callbcak", __func__);
        SendCallBack(FreeInstallErrorCode::FREE_INSTALL_UNDEFINED_ERROR, want, userId,
            targetAbilityInfo.targetInfo.transactId);
        return;
    }
    APP_LOGI("serviceCenterRemoteObject->SendRequest");
    serviceCenterRemoteObject_ = serviceCenterConnection_->GetRemoteObject();
    int32_t result = serviceCenterRemoteObject_->SendRequest(flag, data, reply, option);
    APP_LOGI("SendRequest result = %{public}d", result);
    if (result != ERR_OK) {
        APP_LOGE("failed to sendRequest, result = %{public}d", result);
        SendCallBack(
            FreeInstallErrorCode::FREE_INSTALL_CONNECT_ERROR, want, userId, targetAbilityInfo.targetInfo.transactId);
        return;
    }
    OutTimeMonitor(targetAbilityInfo.targetInfo.transactId);
}

sptr<IRemoteObject> BundleConnectAbilityMgr::GetAbilityManagerServiceCallBack(std::string transactId)
{
    APP_LOGI("start to GetAbilityManagerServiceCallBack");
    FreeInstallParams freeInstallParams;
    auto node = freeInstallParamsMap_.find(transactId);
    if (node == freeInstallParamsMap_.end()) {
        APP_LOGI("can not find node");
        return nullptr;
    }
    freeInstallParams = node->second;
    return freeInstallParams.callback;
}

void BundleConnectAbilityMgr::GetCallingInfo(InnerBundleInfo &innerBundleInfo,
    std::vector<std::string> &bundleNames, std::vector<std::string> &callingAppIds)
{
    bundleNames.emplace_back(innerBundleInfo.GetBundleName());
    callingAppIds.emplace_back(innerBundleInfo.GetBaseBundleInfo().appId);
}

bool ExistBundleNameInCallingBundles(std::string &bundleName, std::vector<std::string> &callingBundleNames)
{
    for (auto bundleNameItem : callingBundleNames) {
        if (bundleNameItem == bundleName) {
            return true;
        }
    }
    return false;
}

bool BundleConnectAbilityMgr::GetTargetAbilityInfo(const Want &want, InnerBundleInfo &innerBundleInfo,
    sptr<TargetAbilityInfo> &targetAbilityInfo, sptr<TargetInfo> &targetInfo)
{
    if (targetAbilityInfo == nullptr) {
        APP_LOGE("QueryAbilityInfo targetAbilityInfo is nullptr");
        return false;
    }
    ElementName element = want.GetElement();
    std::string bundleName = element.GetBundleName();
    std::string abilityName = element.GetAbilityName();
    std::string deviceId = element.GetDeviceID();
    std::vector<std::string> callingBundleNames;
    std::vector<std::string> callingAppids;
    targetAbilityInfo->targetExtSetting = want.GetStringParam("targetExtSetting");
    targetInfo->transactId = std::to_string(this->GetTransactId());
    targetInfo->bundleName = bundleName;
    targetInfo->moduleName = want.GetStringParam("moduleName");
    targetInfo->abilityName = abilityName;
    // make int from bits.
    targetInfo->flags = BIT_ONE_COMPATIBLE + BIT_TWO_BACK_MODE * BIT_TWO + BIT_THREE_CUSTOM * BIT_THREE +
                        deviceId.empty() * BIT_FOUR + BIT_ONE_FIVE_AZ_DEVICE * BIT_FIVE +
                        !ExistBundleNameInCallingBundles(bundleName, callingBundleNames) * BIT_SIX +
                        BIT_ONE_SEVEN_SAME_BUNDLE * BIT_SEVEN;
    targetInfo->callingUid = IPCSkeleton::GetCallingUid();
    targetInfo->callingAppType = CALLING_TYPE_HARMONY;
    targetAbilityInfo->targetInfo = *targetInfo;
    this->GetCallingInfo(innerBundleInfo, callingBundleNames, callingAppids);
    targetAbilityInfo->version = innerBundleInfo.GetVersionCode();
    targetInfo->callingBundleNames = callingBundleNames;
    targetInfo->callingAppIds = callingAppids;
    return true;
}

void BundleConnectAbilityMgr::CallAbilityManager(
    int32_t resultCode, const Want &want, int32_t userId, const sptr<IRemoteObject> &callBack)
{
    MessageParcel data;
    MessageParcel reply;
    MessageOption option;
    if (!data.WriteInterfaceToken(ATOMIC_SERVICE_STATUS_CALLBACK_TOKEN)) {
        APP_LOGE("Write interface token failed");
        return;
    }
    if (!data.WriteInt32(resultCode)) {
        APP_LOGE("Write result code error");
        return;
    }
    if (!data.WriteParcelable(&want)) {
        APP_LOGE("Write want failed");
        return;
    }
    if (!data.WriteInt32(userId)) {
        APP_LOGE("Write userId error");
        return;
    }

    if (callBack->SendRequest(FREE_INSTALL_DONE, data, reply, option) != OHOS::NO_ERROR) {
        APP_LOGE("BundleConnectAbilityMgr::CallAbilityManager SendRequest failed");
    }
}

bool BundleConnectAbilityMgr::CheckIsModuleNeedUpdate(
    InnerBundleInfo &innerBundleInfo, const Want &want, int32_t userId, const sptr<IRemoteObject> &callBack)
{
    if (innerBundleInfo.IsModuleNeedUpdate(want.GetStringParam("moduleName"))) {
        sptr<TargetAbilityInfo> targetAbilityInfo = new TargetAbilityInfo();
        sptr<TargetInfo> targetInfo = new TargetInfo();
        bool targetAbilityInfoResult = GetTargetAbilityInfo(want, innerBundleInfo, targetAbilityInfo, targetInfo);
        if (!targetAbilityInfoResult) {
            CallAbilityManager(FreeInstallErrorCode::FREE_INSTALL_UNDEFINED_ERROR, want, userId, callBack);
        } else {
            sptr<FreeInstallParams> freeInstallParams = new FreeInstallParams();
            freeInstallParams->callback = callBack;
            freeInstallParams->want = want;
            freeInstallParams->userId = userId;
            freeInstallParams->serviceCenterFunction = ServiceCenterFunction::SERVICE_CENTER_CONNECT_UPGRADE_INSTALL;
            auto runner = EventRunner::Create(true);
            freeInstallParams->handler = std::make_shared<AppExecFwk::EventHandler>(runner);
            freeInstallParamsMap_.emplace(targetInfo->transactId, *freeInstallParams);
            APP_LOGD("QueryAbilityInfo UpgradeInstallSafely");
            this->UpgradeInstallSafely(*targetAbilityInfo, want, nullptr, userId);
        }
        return true;
    }
    return false;
}

bool BundleConnectAbilityMgr::QueryAbilityInfo(const Want &want, int32_t flags,
    int32_t userId, AbilityInfo &abilityInfo, const sptr<IRemoteObject> &callBack)
{
    APP_LOGD("QueryAbilityInfo is start");
    ElementName element = want.GetElement();
    std::string bundleName = element.GetBundleName();
    std::string abilityName = element.GetAbilityName();
    int32_t resultCode = ServiceCenterResultCode::FREE_INSTALL_OK;
    if (bundleName == "" || abilityName == "") {
        resultCode = FreeInstallErrorCode::FREE_INSTALL_UNDEFINED_ERROR;
        CallAbilityManager(resultCode, want, userId, callBack);
        return false;
    }
    std::shared_ptr<BundleMgrService> bms = DelayedSingleton<BundleMgrService>::GetInstance();
    std::shared_ptr<BundleDataMgr> bundleDataMgr_ = bms->GetDataMgr();
    InnerBundleInfo innerBundleInfo;
    bool innerBundleInfoResult = bundleDataMgr_->GetInnerBundleInfoWithFlags(bundleName,
        flags, innerBundleInfo, userId);
    bool abilityInfoResult = bundleDataMgr_->QueryAbilityInfo(want, flags, userId, abilityInfo);
    if (innerBundleInfoResult && abilityInfoResult) {
        bool isModuleNeedUpdate = CheckIsModuleNeedUpdate(innerBundleInfo, want, userId, callBack);
        if (!isModuleNeedUpdate) {
            CallAbilityManager(resultCode, want, userId, callBack);
        }
        APP_LOGD("QueryAbilityInfo is ok");
        return true;
    }
    sptr<FreeInstallParams> freeInstallParams = new FreeInstallParams();
    freeInstallParams->callback = callBack;
    freeInstallParams->want = want;
    freeInstallParams->userId = userId;
    sptr<TargetAbilityInfo> targetAbilityInfo = new TargetAbilityInfo();
    sptr<TargetInfo> targetInfo = new TargetInfo();
    bool targetAbilityInfoResult = GetTargetAbilityInfo(want, innerBundleInfo, targetAbilityInfo, targetInfo);
    if (!targetAbilityInfoResult) {
        CallAbilityManager(FreeInstallErrorCode::FREE_INSTALL_UNDEFINED_ERROR, want, userId, callBack);
        return true;
    }
    auto runner = EventRunner::Create(true);
    freeInstallParams->handler = std::make_shared<AppExecFwk::EventHandler>(runner);
    freeInstallParams->serviceCenterFunction = ServiceCenterFunction::SERVICE_CENTER_CONNECT_SILENT_INSTALL;
    freeInstallParamsMap_.emplace(targetInfo->transactId, *freeInstallParams);
    APP_LOGD("QueryAbilityInfo SilentInstallSafely");
    this->SilentInstallSafely(*targetAbilityInfo, want, nullptr, userId);
    return false;
}

void BundleConnectAbilityMgr::UpgradeAtomicService(const Want &want, int32_t userId)
{
    APP_LOGD("start UpgradeAtomicService");
    std::shared_ptr<BundleMgrService> bms = DelayedSingleton<BundleMgrService>::GetInstance();
    std::shared_ptr<BundleDataMgr> bundleDataMgr_ = bms->GetDataMgr();
    ElementName element = want.GetElement();
    std::string bundleName = element.GetBundleName();
    InnerBundleInfo innerBundleInfo;
    sptr<TargetAbilityInfo> targetAbilityInfo = new TargetAbilityInfo();
    sptr<TargetInfo> targetInfo = new TargetInfo();
    sptr<FreeInstallParams> freeInstallParams = new FreeInstallParams();
    bundleDataMgr_->GetInnerBundleInfoWithFlags(bundleName, want.GetFlags(), innerBundleInfo, userId);
    GetTargetAbilityInfo(want, innerBundleInfo, targetAbilityInfo, targetInfo);
    freeInstallParams->want = want;
    freeInstallParams->userId = userId;
    freeInstallParams->serviceCenterFunction = ServiceCenterFunction::SERVICE_CENTER_CONNECT_UPGRADE_CHECK;
    auto runner = EventRunner::Create(true);
    freeInstallParams->handler = std::make_shared<AppExecFwk::EventHandler>(runner);
    freeInstallParamsMap_.emplace(targetInfo->transactId, *freeInstallParams);
    targetAbilityInfo->version = innerBundleInfo.GetVersionCode();
    this->UpgradeCheckSafely(*targetAbilityInfo, want, nullptr, userId);
}
}  // namespace AppExecFwk
}  // namespace OHOS