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
#include "app_log_wrapper.h"
#include "form_cache_mgr.h"
#include "form_dump_mgr.h"

namespace OHOS {
namespace AppExecFwk {
const std::string LINE_SEPARATOR = "\n";

FormDumpMgr::FormDumpMgr(){}
FormDumpMgr::~FormDumpMgr(){}
/**
 * @brief Dump all of form storage infos.
 * @param storageInfos Form storage infos
 * @param formInfos Form storage dump info.
 */
void FormDumpMgr::DumpStorageFormInfos(const std::vector<FormDBInfo> &storageInfos, std::string &formInfos) const
{
    for (const auto &info : storageInfos) {
        formInfos += "  FormId #" + std::to_string(info.formId);
        formInfos += "    formName [" + info.formName + "]";
        formInfos += "    bundleName [" + info.bundleName + "]";
        formInfos += "    moduleName [" + info.moduleName + "]";
        formInfos += "    abilityName [" + info.abilityName + "]";
        formInfos += "    formUserUids [";
        for (auto &uId : info.formUserUids) {
            formInfos += "      Uid [" + std::to_string(uId) + "]";
        }
        formInfos += "]" + LINE_SEPARATOR;
    }
}
/**
 * @brief Dump form infos.
 * @param formRecordInfos Form record infos.
 * @param formInfos Form dump infos.
 */
void FormDumpMgr::DumpFormInfos(const std::vector<FormRecord> &formRecordInfos, std::string &formInfos) const
{
    APP_LOGI("%{public}s called.", __func__);
    for (const auto &info : formRecordInfos) {
        formInfos += "  FormId [" + std::to_string(info.formId)  + "]";
        formInfos += "    formName [" + info.formName + "]";
        formInfos += "    bundleName [" + info.bundleName + "]";
        formInfos += "    moduleName [" + info.moduleName + "]";
        formInfos += "    abilityName [" + info.abilityName + "]";
        formInfos += "    isInited [" + std::to_string(info.isInited) + "]";
        formInfos += "    needRefresh [" + std::to_string(info.needRefresh) + "]";
        formInfos += "    isEnableUpdate [" + std::to_string(info.isEnableUpdate) + "]";
        formInfos += "    isCountTimerRefresh [" + std::to_string(info.isCountTimerRefresh) + "]";
        formInfos += "    specification [" + std::to_string(info.specification) + "]";
        formInfos += "    updateDuration [" + std::to_string(info.updateDuration) + "]";
        formInfos += "    updateAtHour [" + std::to_string(info.updateAtHour) + "]";
        formInfos += "    updateAtMin [" + std::to_string(info.updateAtMin) + "]";
        formInfos += "    formTempFlg [" + std::to_string(info.formTempFlg) + "]";
        formInfos += "    formVisibleNotify [" + std::to_string(info.formVisibleNotify) + "]";
        formInfos += "    formVisibleNotifyState [" + std::to_string(info.formVisibleNotifyState) + "]";

        if (info.hapSourceDirs.size() > 0) {
            formInfos += "    hapSourceDirs [";
            for (auto &hapDir : info.hapSourceDirs) {
                formInfos += "    hapSourceDir [" + hapDir + "]";
            }
            formInfos += "]";
        }

        if (info.formUserUids.size() > 0) {
            formInfos += "    formUserUids [";
            for (auto &uId : info.formUserUids) {
                formInfos += "    Uid [" + std::to_string(uId) + "]";
            }
            formInfos += "]";
        }  
        
        // formCacheData
        std::string strCacheData;
        if (FormCacheMgr::GetInstance().GetData(info.formId, strCacheData)) {
            formInfos += "    formCacheData [";
            formInfos += strCacheData;
            formInfos += "]" + LINE_SEPARATOR;
        }
    }

    APP_LOGI("%{public}s success. Form infos:%{public}s", __func__, formInfos.c_str());
}
/**
 * @brief Dump form infos.
 * @param formRecordInfo Form Host record info.
 * @param formInfo Form dump info.
 */
void FormDumpMgr::DumpFormHostInfo(const FormHostRecord &formHostRecord, std::string &formInfo) const
{
    APP_LOGI("%{public}s called.", __func__);
    formInfo += "  ================FormHostRecord=================";
    formInfo += "  callerUid [" + std::to_string(formHostRecord.GetCallerUid()) + "]";
    formInfo += "  hostBundleName [" + formHostRecord.GetHostBundleName() + "]";
    APP_LOGI("%{public}s success. Host Form infos:%{public}s", __func__, formInfo.c_str());
}

/**
 * @brief Dump form infos.
 * @param formRecordInfo Form record info.
 * @param formInfo Form dump info.
 */
void FormDumpMgr::DumpFormInfo(const FormRecord &formRecordInfo, std::string &formInfo) const
{
    APP_LOGI("%{public}s called.", __func__);
    formInfo += "  ================FormRecord=================";
    formInfo += "  FormId [" + std::to_string(formRecordInfo.formId) + "]";
    formInfo += "    formName [" + formRecordInfo.formName + "]";
    formInfo += "    bundleName [" + formRecordInfo.bundleName + "]";
    formInfo += "    moduleName [" + formRecordInfo.moduleName + "]";
    formInfo += "    abilityName [" + formRecordInfo.abilityName + "]";
    formInfo += "    isInited [" + std::to_string(formRecordInfo.isInited) + "]";
    formInfo += "    needRefresh [" + std::to_string(formRecordInfo.needRefresh) + "]";
    formInfo += "    isEnableUpdate [" + std::to_string(formRecordInfo.isEnableUpdate) + "]";
    formInfo += "    isCountTimerRefresh [" + std::to_string(formRecordInfo.isCountTimerRefresh) + "]";
    formInfo += "    specification [" + std::to_string(formRecordInfo.specification) + "]";
    formInfo += "    updateDuration [" + std::to_string(formRecordInfo.updateDuration) + "]";
    formInfo += "    updateAtHour [" + std::to_string(formRecordInfo.updateAtHour) + "]";
    formInfo += "    updateAtMin [" + std::to_string(formRecordInfo.updateAtMin) + "]";
    formInfo += "    formTempFlg [" + std::to_string(formRecordInfo.formTempFlg) + "]";
    formInfo += "    formVisibleNotify [" + std::to_string(formRecordInfo.formVisibleNotify) + "]";
    formInfo += "    formVisibleNotifyState [" + std::to_string(formRecordInfo.formVisibleNotifyState) + "]";
    formInfo += "    formSrc [" + formRecordInfo.formSrc + "]";
    formInfo += "    designWidth [" + std::to_string(formRecordInfo.formWindow.designWidth) + "]";
    formInfo += "    autoDesignWidth [" + std::to_string(formRecordInfo.formWindow.autoDesignWidth) + "]";
    formInfo += "    versionCode [" + std::to_string(formRecordInfo.versionCode) + "]";
    formInfo += "    versionName [" + formRecordInfo.versionName + "]";
    formInfo += "    compatibleVersion [" + std::to_string(formRecordInfo.compatibleVersion) + "]";
    formInfo += "    icon [" + formRecordInfo.icon + "]";

    if (formRecordInfo.hapSourceDirs.size() > 0) {
        formInfo += "    hapSourceDirs [";
        for (auto &hapDir : formRecordInfo.hapSourceDirs) {
            formInfo += "    hapSourceDir [" + hapDir + "]";
        }
        formInfo += "]";
    }

    if (formRecordInfo.formUserUids.size() > 0) {
        formInfo += "    formUserUids [";
        for (auto &uId : formRecordInfo.formUserUids) {
            formInfo += "    Uid [" + std::to_string(uId) + "]";
        }
        formInfo += "]";
    }
    
    // formCacheData
    std::string strCacheData;
    if (FormCacheMgr::GetInstance().GetData(formRecordInfo.formId, strCacheData)) {
        formInfo += "    formCacheData [";
        formInfo += strCacheData;
        formInfo += "]" + LINE_SEPARATOR;
    }

    APP_LOGI("%{public}s success. Form infos:%{public}s", __func__, formInfo.c_str());
}
}  // namespace AppExecFwk
}  // namespace OHOS