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

#include "distributed_bms.h"

#include <fstream>
#include <vector>

#include "app_log_wrapper.h"
#include "appexecfwk_errors.h"
#include "bundle_mgr_interface.h"
#include "bundle_mgr_proxy.h"
#include "iservice_registry.h"
#include "if_system_ability_manager.h"
#include "os_account_manager.h"
#include "parameter.h"
#include "system_ability_definition.h"

namespace OHOS {
namespace AppExecFwk {
namespace {
    const uint8_t DECODE_BUNBER_ONE = 1;
    const uint8_t DECODE_BUNBER_TWO = 2;
    const uint8_t DECODE_BUNBER_THREE = 3;
    const uint8_t DECODE_BUNBER_FOUR = 4;
    const uint8_t DECODE_BUNBER_SIX = 6;
    const uint8_t DECODE_BUNBER_FIFTEEN = 0xf;
    const uint8_t DECODE_BUNBER_SIXTY_THREE = 0x3F;
    const std::vector<char> DECODE_TABLE = {
        'A', 'B', 'C', 'D', 'E', 'F', 'G', 'H', 'I', 'J', 'K', 'L', 'M',
        'N', 'O', 'P', 'Q', 'R', 'S', 'T', 'U', 'V', 'W', 'X', 'Y', 'Z',
        'a', 'b', 'c', 'd', 'e', 'f', 'g', 'h', 'i', 'j', 'k', 'l', 'm',
        'n', 'o', 'p', 'q', 'r', 's', 't', 'u', 'v', 'w', 'x', 'y', 'z',
        '0', '1', '2', '3', '4', '5', '6', '7', '8', '9', '+', '/'
    };
}
REGISTER_SYSTEM_ABILITY_BY_ID(DistributedBms, DISTRIBUTED_BUNDLE_MGR_SERVICE_SYS_ABILITY_ID, true);

OHOS::sptr<OHOS::AppExecFwk::IBundleMgr> bundleMgr_ = nullptr;
std::mutex bundleMgrMutex_;

DistributedBms::DistributedBms(int32_t saId, bool runOnCreate) : SystemAbility(saId, runOnCreate)
{
    APP_LOGI("DistributedBms :%{public}s call", __func__);
}

DistributedBms::~DistributedBms()
{
    APP_LOGI("DistributedBms: DBundleMgrService");
}

void DistributedBms::OnStart()
{
    APP_LOGI("DistributedBms: OnStart");
    bool res = Publish(this);
    if (!res) {
        APP_LOGE("DistributedBms: OnStart failed");
    }
    APP_LOGI("DistributedBms: OnStart end");
}

void DistributedBms::OnStop()
{
    APP_LOGI("DistributedBms: OnStop");
}

static OHOS::sptr<OHOS::AppExecFwk::IBundleMgr> GetBundleMgr()
{
    if (bundleMgr_ == nullptr) {
        std::lock_guard<std::mutex> lock(bundleMgrMutex_);
        if (bundleMgr_ == nullptr) {
            auto systemAbilityManager = OHOS::SystemAbilityManagerClient::GetInstance().GetSystemAbilityManager();
            if (systemAbilityManager == nullptr) {
                APP_LOGE("GetBundleMgr GetSystemAbilityManager is null");
                return nullptr;
            }
            auto bundleMgrSa = systemAbilityManager->GetSystemAbility(OHOS::BUNDLE_MGR_SERVICE_SYS_ABILITY_ID);
            if (bundleMgrSa == nullptr) {
                APP_LOGE("GetBundleMgr GetSystemAbility is null");
                return nullptr;
            }
            auto bundleMgr = OHOS::iface_cast<IBundleMgr>(bundleMgrSa);
            if (bundleMgr == nullptr) {
                APP_LOGE("GetBundleMgr iface_cast get null");
            }
            bundleMgr_ = bundleMgr;
        }
    }
    return bundleMgr_;
}


static OHOS::sptr<OHOS::AppExecFwk::IDistributedBms> GetDistributedBundleMgr(const std::string &deviceId)
{
    APP_LOGI("GetDistributedBundleMgr");
    auto samgr = OHOS::SystemAbilityManagerClient::GetInstance().GetSystemAbilityManager();
    OHOS::sptr<OHOS::IRemoteObject> remoteObject;
    APP_LOGD("GetDistributedBundleMgr deviceId:%{public}s", deviceId.c_str());
    if (deviceId.empty()) {
        return nullptr;
    } else {
        APP_LOGI("GetDistributedBundleMgr get remote d-bms");
        remoteObject = samgr->CheckSystemAbility(OHOS::DISTRIBUTED_BUNDLE_MGR_SERVICE_SYS_ABILITY_ID, deviceId);
    }
    return OHOS::iface_cast<IDistributedBms>(remoteObject);
}

int32_t DistributedBms::GetRemoteAbilityInfo(
    const OHOS::AppExecFwk::ElementName &elementName, RemoteAbilityInfo &remoteAbilityInfo)
{
    APP_LOGD("DistributedBms GetRemoteAbilityInfo bundleName:%{public}s , abilityName:%{public}s",
        elementName.GetBundleName().c_str(), elementName.GetAbilityName().c_str());
    auto iDistBundleMgr = GetDistributedBundleMgr(elementName.GetDeviceID());
    if (!iDistBundleMgr) {
        APP_LOGE("GetDistributedBundleMgr failed");
        return ERR_APPEXECFWK_FAILED_GET_REMOTE_PROXY;
    } else {
        APP_LOGD("GetDistributedBundleMgr get remote d-bms");
        return iDistBundleMgr->GetAbilityInfo(elementName, remoteAbilityInfo);
    }
}

int32_t DistributedBms::GetRemoteAbilityInfos(
    const std::vector<ElementName> &elementNames, std::vector<RemoteAbilityInfo> &remoteAbilityInfos)
{
    APP_LOGD("DistributedBms GetRemoteAbilityInfos");
    auto iDistBundleMgr = GetDistributedBundleMgr(elementNames[0].GetDeviceID());
    if (!iDistBundleMgr) {
        APP_LOGE("GetDistributedBundleMgr failed");
        return ERR_APPEXECFWK_FAILED_GET_REMOTE_PROXY;
    } else {
        APP_LOGD("GetDistributedBundleMgr get remote d-bms");
        return iDistBundleMgr->GetAbilityInfos(elementNames, remoteAbilityInfos);
    }
}


int32_t DistributedBms::GetAbilityInfo(
    const OHOS::AppExecFwk::ElementName &elementName, RemoteAbilityInfo &remoteAbilityInfo)
{
    APP_LOGI("DistributedBms GetAbilityInfo bundleName:%{public}s , abilityName:%{public}s",
        elementName.GetBundleName().c_str(), elementName.GetAbilityName().c_str());
    auto iBundleMgr = GetBundleMgr();
    if (!iBundleMgr) {
        APP_LOGE("DistributedBms GetBundleMgr failed");
        return ERR_APPEXECFWK_FAILED_SERVICE_DIED;
    }
    int userId = -1;
    if (!GetCurrentUserId(userId)) {
        APP_LOGE("GetCurrentUserId failed");
        return ERR_APPEXECFWK_USER_NOT_EXIST;
    }
    BundleInfo bundleInfo;
    if (!iBundleMgr->GetBundleInfo(elementName.GetBundleName(), 1, bundleInfo, userId)) {
        APP_LOGE("DistributedBms GetBundleInfo failed");
        return ERR_APPEXECFWK_FAILED_GET_BUNDLE_INFO;
    }
    std::shared_ptr<Global::Resource::ResourceManager> resourceManager = nullptr;
    resourceManager = GetResourceManager(bundleInfo);
    if (resourceManager == nullptr) {
        APP_LOGE("DistributedBms InitResourceManager failed");
        return ERR_APPEXECFWK_FAILED_GET_RESOURCEMANAGER;
    }
    AbilityInfo abilityInfo;
    OHOS::AAFwk::Want want;
    want.SetElement(elementName);
    if (!iBundleMgr->QueryAbilityInfo(want, GET_ABILITY_INFO_WITH_APPLICATION, userId, abilityInfo)) {
        APP_LOGE("DistributedBms QueryAbilityInfo failed");
        return ERR_APPEXECFWK_FAILED_GET_ABILITY_INFO;
    }
    remoteAbilityInfo.elementName = elementName;
    OHOS::Global::Resource::RState errval =
        resourceManager->GetStringById(static_cast<uint32_t>(abilityInfo.labelId), remoteAbilityInfo.label);
    if (errval != OHOS::Global::Resource::RState::SUCCESS) {
        APP_LOGE("DistributedBms GetStringById failed");
        return ERR_APPEXECFWK_FAILED_GET_RESOURCEMANAGER;
    }
    std::string iconPath;
    OHOS::Global::Resource::RState iconPathErrval =
        resourceManager->GetMediaById(static_cast<uint32_t>(abilityInfo.iconId), iconPath);
    if (iconPathErrval != OHOS::Global::Resource::RState::SUCCESS) {
        APP_LOGE("DistributedBms GetStringById  iconPath failed");
        return ERR_APPEXECFWK_FAILED_GET_RESOURCEMANAGER;
    }
    if (!GetMediaBase64(iconPath, remoteAbilityInfo.icon)) {
        APP_LOGE("DistributedBms GetMediaBase64 failed");
        return ERR_APPEXECFWK_FAILED_GET_RESOURCEMANAGER;
    }
    APP_LOGD("DistributedBms GetAbilityInfo label:%{public}s", remoteAbilityInfo.label.c_str());
    APP_LOGD("DistributedBms GetAbilityInfo iconId:%{public}s", remoteAbilityInfo.icon.c_str());
    return OHOS::NO_ERROR;
}

int32_t DistributedBms::GetAbilityInfos(
    const std::vector<ElementName> &elementNames, std::vector<RemoteAbilityInfo> &remoteAbilityInfos)
{
    APP_LOGD("DistributedBms GetAbilityInfos");
    for (auto elementName : elementNames) {
        RemoteAbilityInfo remoteAbilityInfo;
        int32_t result = GetAbilityInfo(elementName, remoteAbilityInfo);
        if (result) {
            APP_LOGE("get AbilityInfo:%{public}s, %{public}s failed", elementName.GetBundleName().c_str(),
                elementName.GetAbilityName().c_str());
            return result;
        }
        remoteAbilityInfos.push_back(remoteAbilityInfo);
    }
    return OHOS::NO_ERROR;
}

std::shared_ptr<Global::Resource::ResourceManager> DistributedBms::GetResourceManager(
    const AppExecFwk::BundleInfo &bundleInfo)
{
    std::shared_ptr<Global::Resource::ResourceManager> resourceManager(Global::Resource::CreateResourceManager());
    APP_LOGD(
        "DistributedBms::InitResourceManager moduleResPaths count: %{public}zu", bundleInfo.moduleResPaths.size());
    for (auto moduleResPath : bundleInfo.moduleResPaths) {
        if (!moduleResPath.empty()) {
            APP_LOGE("DistributedBms::InitResourceManager length: %{public}zu, moduleResPath: %{public}s",
                moduleResPath.length(),
                moduleResPath.c_str());
            if (!resourceManager->AddResource(moduleResPath.c_str())) {
                APP_LOGE("DistributedBms::InitResourceManager AddResource failed");
            }
        }
    }

    std::unique_ptr<Global::Resource::ResConfig> resConfig(Global::Resource::CreateResConfig());
    resConfig->SetLocaleInfo("zh", "Hans", "CN");
    if (resConfig->GetLocaleInfo() != nullptr) {
        APP_LOGD("DistributedBms::InitResourceManager language: %{public}s, script: %{public}s, region: %{public}s,",
            resConfig->GetLocaleInfo()->getLanguage(),
            resConfig->GetLocaleInfo()->getScript(),
            resConfig->GetLocaleInfo()->getCountry());
    } else {
        APP_LOGE("DistributedBms::InitResourceManager language: GetLocaleInfo is null.");
    }
    resourceManager->UpdateResConfig(*resConfig);
    return resourceManager;
}

bool DistributedBms::GetMediaBase64(std::string &path, std::string &value)
{
    int len = 0;
    std::unique_ptr<char[]> tempData = LoadResourceFile(path, len);
    if (tempData == nullptr) {
        return false;
    }
    auto pos = path.find_last_of('.');
    std::string imgType;
    if (pos != std::string::npos) {
        imgType = path.substr(pos + 1);
    }
    std::unique_ptr<char[]> base64Data = EncodeBase64(tempData, len);
    value = "data:image/" + imgType + ";base64," + base64Data.get();
    return true;
}

std::unique_ptr<char[]> DistributedBms::LoadResourceFile(std::string &path, int &len)
{
    std::ifstream mediaStream(path, std::ios::binary);
    if (!mediaStream.is_open()) {
        return nullptr;
    }
    mediaStream.seekg(0, std::ios::end);
    len = mediaStream.tellg();
    std::unique_ptr<char[]> tempData = std::make_unique<char[]>(len);
    if (tempData == nullptr) {
        return nullptr;
    }
    mediaStream.seekg(0, std::ios::beg);
    mediaStream.read((tempData.get()), len);
    return tempData;
}

std::unique_ptr<char[]> DistributedBms::EncodeBase64(std::unique_ptr<char[]> &data, int srcLen)
{
    int len = (srcLen / DECODE_BUNBER_THREE) * DECODE_BUNBER_FOUR; // Split 3 bytes to 4 parts, each containing 6 bits.
    int outLen = ((srcLen % DECODE_BUNBER_THREE) != 0) ? (len + DECODE_BUNBER_FOUR) : len;
    const char *srcData = data.get();
    std::unique_ptr<char[]>  result = std::make_unique<char[]>(outLen + DECODE_BUNBER_ONE);
    char *dstData = result.get();
    int j = 0;
    int i = 0;
    for (; i < srcLen - DECODE_BUNBER_THREE; i += DECODE_BUNBER_THREE) {
        unsigned char byte1 = static_cast<unsigned char>(srcData[i]);
        unsigned char byte2 = static_cast<unsigned char>(srcData[i + DECODE_BUNBER_ONE]);
        unsigned char byte3 = static_cast<unsigned char>(srcData[i + DECODE_BUNBER_TWO]);
        dstData[j++] = DECODE_TABLE[byte1 >> DECODE_BUNBER_TWO];
        dstData[j++] =
            DECODE_TABLE[((byte1 & DECODE_BUNBER_THREE) << DECODE_BUNBER_FOUR) | (byte2 >> DECODE_BUNBER_FOUR)];
        dstData[j++] =
            DECODE_TABLE[((byte2 & DECODE_BUNBER_FIFTEEN) << DECODE_BUNBER_TWO) | (byte3 >> DECODE_BUNBER_SIX)];
        dstData[j++] = DECODE_TABLE[byte3 & DECODE_BUNBER_SIXTY_THREE];
    }
    if (srcLen % DECODE_BUNBER_THREE == DECODE_BUNBER_ONE) {
        unsigned char byte1 = static_cast<unsigned char>(srcData[i]);
        dstData[j++] = DECODE_TABLE[byte1 >> DECODE_BUNBER_TWO];
        dstData[j++] = DECODE_TABLE[(byte1 & DECODE_BUNBER_THREE) << DECODE_BUNBER_FOUR];
        dstData[j++] = '=';
        dstData[j++] = '=';
    } else {
        unsigned char byte1 = static_cast<unsigned char>(srcData[i]);
        unsigned char byte2 = static_cast<unsigned char>(srcData[i + DECODE_BUNBER_ONE]);
        dstData[j++] = DECODE_TABLE[byte1 >> DECODE_BUNBER_TWO];
        dstData[j++] =
            DECODE_TABLE[((byte1 & DECODE_BUNBER_THREE) << DECODE_BUNBER_FOUR) | (byte2 >> DECODE_BUNBER_FOUR)];
        dstData[j++] = DECODE_TABLE[(byte2 & DECODE_BUNBER_FIFTEEN) << DECODE_BUNBER_TWO];
        dstData[j++] = '=';
    }
    dstData[outLen] = '\0';

    return result;
}

bool DistributedBms::GetCurrentUserId(int &userId)
{
    std::vector<int> activeIds;
    int ret = AccountSA::OsAccountManager::QueryActiveOsAccountIds(activeIds);
    if (ret != 0) {
        APP_LOGE("QueryActiveOsAccountIds failed ret:%{public}d", ret);
        return false;
    }
    if (activeIds.empty()) {
        APP_LOGE("QueryActiveOsAccountIds activeIds empty");
        return false;
    }
    userId = activeIds[0];
    return true;
}
}
}