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

#include "bundle_mgr_interface.h"
#include "hilog_wrapper.h"
#include "if_system_ability_manager.h"
#include "iservice_registry.h"
#include "sys_mgr_client.h"
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

DistributedBms::DistributedBms(int32_t saId, bool runOnCreate) : SystemAbility(saId, runOnCreate)
{
    HILOG_INFO("DistributedBms :%{public}s call", __func__);
}

DistributedBms::~DistributedBms()
{
    HILOG_INFO("DistributedBms: DBundleMgrService");
}

void DistributedBms::OnStart()
{
    HILOG_INFO("DistributedBms: OnStart");
    bool res = Publish(this);
    if (!res) {
        HILOG_ERROR("DistributedBms: OnStart failed");
    }
    HILOG_INFO("DistributedBms: OnStart end");
}

void DistributedBms::OnStop()
{
    HILOG_INFO("DistributedBms: OnStop");
}

static OHOS::sptr<OHOS::AppExecFwk::IBundleMgr> GetBundleMgr()
{
    HILOG_DEBUG("DistributedBms::GetBundleManager begin");
    auto bundleObj =
        OHOS::DelayedSingleton<AppExecFwk::SysMrgClient>::GetInstance()->GetSystemAbility(
            BUNDLE_MGR_SERVICE_SYS_ABILITY_ID);
    if (bundleObj == nullptr) {
        HILOG_ERROR("failed to get bundle manager service");
        return nullptr;
    }
    sptr<AppExecFwk::IBundleMgr> bms = iface_cast<AppExecFwk::IBundleMgr>(bundleObj);
    HILOG_DEBUG("DistributedBms::GetBundleManager end");
    return bms;
}

bool DistributedBms::GetRemoteAbilityInfo(
    const OHOS::AppExecFwk::ElementName &elementName, RemoteAbilityInfo &remoteAbilityInfo)
{
    HILOG_INFO("DistributedBms GetRemoteAbilityInfo bundleName:%{public}s , abilityName:%{public}s",
        elementName.GetBundleName().c_str(), elementName.GetAbilityName().c_str());
    auto iBundleMgr = GetBundleMgr();
    if (!iBundleMgr) {
        HILOG_ERROR("DistributedBms GetBundleMgr failed");
        return false;
    }
    BundleInfo bundleInfo;
    if (!iBundleMgr->GetBundleInfo(elementName.GetBundleName(), 1, bundleInfo, 0)) {
        HILOG_ERROR("DistributedBms GetBundleInfo failed");
        return false;
    }
    std::shared_ptr<Global::Resource::ResourceManager> resourceManager = nullptr;
    resourceManager = GetResourceManager(bundleInfo);
    if (resourceManager == nullptr) {
        HILOG_ERROR("DistributedBms InitResourceManager failed");
        return false;
    }
    AbilityInfo abilityInfo;
    OHOS::AAFwk::Want want;
    want.SetElement(elementName);
    if (!iBundleMgr->QueryAbilityInfo(want, abilityInfo)) {
        HILOG_ERROR("DistributedBms QueryAbilityInfo failed");
        return false;
    }
    remoteAbilityInfo.elementName = elementName;
    OHOS::Global::Resource::RState errval =
        resourceManager->GetStringById(static_cast<uint32_t>(abilityInfo.labelId), remoteAbilityInfo.label);
    if (errval != OHOS::Global::Resource::RState::SUCCESS) {
        HILOG_ERROR("DistributedBms GetStringById failed");
        return false;
    }
    std::string iconPath;
    OHOS::Global::Resource::RState iconPathErrval =
        resourceManager->GetMediaById(static_cast<uint32_t>(abilityInfo.iconId), iconPath);
    if (iconPathErrval != OHOS::Global::Resource::RState::SUCCESS) {
        HILOG_ERROR("DistributedBms GetStringById  iconPath failed");
        return false;
    }
    if (!GetMediaBase64(iconPath, remoteAbilityInfo.icon)) {
        HILOG_ERROR("DistributedBms GetMediaBase64 failed");
        return false;
    }
    HILOG_DEBUG("DistributedBms GetRemoteAbilityInfo label:%{public}s", remoteAbilityInfo.label.c_str());
    HILOG_DEBUG("DistributedBms GetRemoteAbilityInfo iconId:%{public}s", remoteAbilityInfo.icon.c_str());
    return true;
}

std::shared_ptr<Global::Resource::ResourceManager> DistributedBms::GetResourceManager(
    const AppExecFwk::BundleInfo &bundleInfo)
{
    std::shared_ptr<Global::Resource::ResourceManager> resourceManager(Global::Resource::CreateResourceManager());
    HILOG_DEBUG(
        "DistributedBms::InitResourceManager moduleResPaths count: %{public}zu", bundleInfo.moduleResPaths.size());
    for (auto moduleResPath : bundleInfo.moduleResPaths) {
        if (!moduleResPath.empty()) {
            HILOG_ERROR("DistributedBms::InitResourceManager length: %{public}zu, moduleResPath: %{public}s",
                moduleResPath.length(),
                moduleResPath.c_str());
            if (!resourceManager->AddResource(moduleResPath.c_str())) {
                HILOG_ERROR("DistributedBms::InitResourceManager AddResource failed");
            }
        }
    }

    std::unique_ptr<Global::Resource::ResConfig> resConfig(Global::Resource::CreateResConfig());
    resConfig->SetLocaleInfo("zh", "Hans", "CN");
    if (resConfig->GetLocaleInfo() != nullptr) {
        HILOG_DEBUG("DistributedBms::InitResourceManager language: %{public}s, script: %{public}s, region: %{public}s,",
            resConfig->GetLocaleInfo()->getLanguage(),
            resConfig->GetLocaleInfo()->getScript(),
            resConfig->GetLocaleInfo()->getCountry());
    } else {
        HILOG_ERROR("DistributedBms::InitResourceManager language: GetLocaleInfo is null.");
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
}
}