/*
 * Copyright (c) 2021-2022 Huawei Device Co., Ltd.
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

#include "bundle_mgr_proxy.h"

#include "ipc_types.h"
#include "parcel.h"
#include "string_ex.h"

#include "app_log_wrapper.h"
#include "appexecfwk_errors.h"
#include "bundle_constants.h"
#include "bytrace.h"
#include "json_util.h"
#include "securec.h"

namespace OHOS {
namespace AppExecFwk {
namespace {
const int32_t ASHMEM_LEN = 16;

inline void ClearAshMem(sptr<Ashmem> &optMem)
{
    if (optMem != nullptr) {
        optMem->UnmapAshmem();
        optMem->CloseAshmem();
    }
}

bool ParseStr(const char *buf, const int itemLen, int index, std::string &result)
{
    APP_LOGD("ParseStr itemLen:%{public}d  index:%{public}d.", itemLen, index);
    if (buf == nullptr || itemLen <= 0 || index < 0) {
        APP_LOGE("param invalid.");
        return false;
    }

    char item[itemLen + 1];
    if (strncpy_s(item, sizeof(item), buf + index, itemLen) != 0) {
        APP_LOGE("ParseStr failed due to strncpy_s error.");
        return false;
    }

    std::string str(item, 0, itemLen);
    result = str;
    return true;
}
}
BundleMgrProxy::BundleMgrProxy(const sptr<IRemoteObject> &impl) : IRemoteProxy<IBundleMgr>(impl)
{
    APP_LOGI("create bundle mgr proxy instance");
}

BundleMgrProxy::~BundleMgrProxy()
{
    APP_LOGI("destroy create bundle mgr proxy instance");
}

bool BundleMgrProxy::GetApplicationInfo(
    const std::string &appName, const ApplicationFlag flag, const int userId, ApplicationInfo &appInfo)
{
    BYTRACE_NAME(BYTRACE_TAG_APP, __PRETTY_FUNCTION__);
    APP_LOGD("begin to GetApplicationInfo of %{public}s", appName.c_str());
    if (appName.empty()) {
        APP_LOGE("fail to GetApplicationInfo due to params empty");
        return false;
    }

    MessageParcel data;
    if (!data.WriteInterfaceToken(GetDescriptor())) {
        APP_LOGE("fail to GetApplicationInfo due to write MessageParcel fail");
        return false;
    }
    if (!data.WriteString(appName)) {
        APP_LOGE("fail to GetApplicationInfo due to write appName fail");
        return false;
    }
    if (!data.WriteInt32(static_cast<int>(flag))) {
        APP_LOGE("fail to GetApplicationInfo due to write flag fail");
        return false;
    }
    if (!data.WriteInt32(userId)) {
        APP_LOGE("fail to GetApplicationInfo due to write userId fail");
        return false;
    }

    if (!GetParcelableInfo<ApplicationInfo>(IBundleMgr::Message::GET_APPLICATION_INFO, data, appInfo)) {
        APP_LOGE("fail to GetApplicationInfo from server");
        return false;
    }
    return true;
}

bool BundleMgrProxy::GetApplicationInfo(
    const std::string &appName, int32_t flags, int32_t userId, ApplicationInfo &appInfo)
{
    BYTRACE_NAME(BYTRACE_TAG_APP, __PRETTY_FUNCTION__);
    APP_LOGD("begin to GetApplicationInfo of %{public}s", appName.c_str());
    if (appName.empty()) {
        APP_LOGE("fail to GetApplicationInfo due to params empty");
        return false;
    }

    MessageParcel data;
    if (!data.WriteInterfaceToken(GetDescriptor())) {
        APP_LOGE("fail to GetApplicationInfo due to write MessageParcel fail");
        return false;
    }
    if (!data.WriteString(appName)) {
        APP_LOGE("fail to GetApplicationInfo due to write appName fail");
        return false;
    }
    if (!data.WriteInt32(flags)) {
        APP_LOGE("fail to GetApplicationInfo due to write flag fail");
        return false;
    }
    if (!data.WriteInt32(userId)) {
        APP_LOGE("fail to GetApplicationInfo due to write userId fail");
        return false;
    }

    if (!GetParcelableInfo<ApplicationInfo>(IBundleMgr::Message::GET_APPLICATION_INFO_WITH_INT_FLAGS, data, appInfo)) {
        APP_LOGE("fail to GetApplicationInfo from server");
        return false;
    }
    return true;
}

bool BundleMgrProxy::GetApplicationInfos(
    const ApplicationFlag flag, int userId, std::vector<ApplicationInfo> &appInfos)
{
    BYTRACE_NAME(BYTRACE_TAG_APP, __PRETTY_FUNCTION__);
    APP_LOGD("begin to get GetApplicationInfos of specific userId id %{private}d", userId);
    MessageParcel data;
    if (!data.WriteInterfaceToken(GetDescriptor())) {
        APP_LOGE("fail to GetApplicationInfo due to write MessageParcel fail");
        return false;
    }
    if (!data.WriteInt32(static_cast<int>(flag))) {
        APP_LOGE("fail to GetApplicationInfo due to write flag fail");
        return false;
    }
    if (!data.WriteInt32(userId)) {
        APP_LOGE("fail to GetApplicationInfos due to write userId error");
        return false;
    }

    if (!GetParcelableInfos<ApplicationInfo>(IBundleMgr::Message::GET_APPLICATION_INFOS, data, appInfos)) {
        APP_LOGE("fail to GetApplicationInfos from server");
        return false;
    }
    return true;
}

bool BundleMgrProxy::GetApplicationInfos(
    int32_t flags, int32_t userId, std::vector<ApplicationInfo> &appInfos)
{
    BYTRACE_NAME(BYTRACE_TAG_APP, __PRETTY_FUNCTION__);
    APP_LOGD("begin to get GetApplicationInfos of specific userId id %{private}d", userId);
    MessageParcel data;
    if (!data.WriteInterfaceToken(GetDescriptor())) {
        APP_LOGE("fail to GetApplicationInfo due to write MessageParcel fail");
        return false;
    }
    if (!data.WriteInt32(flags)) {
        APP_LOGE("fail to GetApplicationInfo due to write flag fail");
        return false;
    }
    if (!data.WriteInt32(userId)) {
        APP_LOGE("fail to GetApplicationInfos due to write userId error");
        return false;
    }

    if (!GetParcelableInfos<ApplicationInfo>(IBundleMgr::Message::GET_APPLICATION_INFOS_WITH_INT_FLAGS,
        data, appInfos)) {
        APP_LOGE("fail to GetApplicationInfos from server");
        return false;
    }
    return true;
}

bool BundleMgrProxy::GetBundleInfo(
    const std::string &bundleName, const BundleFlag flag, BundleInfo &bundleInfo, int32_t userId)
{
    BYTRACE_NAME(BYTRACE_TAG_APP, __PRETTY_FUNCTION__);
    APP_LOGD("begin to get bundle info of %{public}s", bundleName.c_str());
    if (bundleName.empty()) {
        APP_LOGE("fail to GetBundleInfo due to params empty");
        return false;
    }

    MessageParcel data;
    if (!data.WriteInterfaceToken(GetDescriptor())) {
        APP_LOGE("fail to GetBundleInfo due to write InterfaceToken fail");
        return false;
    }
    if (!data.WriteString(bundleName)) {
        APP_LOGE("fail to GetBundleInfo due to write bundleName fail");
        return false;
    }
    if (!data.WriteInt32(static_cast<int>(flag))) {
        APP_LOGE("fail to GetBundleInfo due to write flag fail");
        return false;
    }
    if (!data.WriteInt32(static_cast<int>(userId))) {
        APP_LOGE("fail to GetBundleInfo due to write userId fail");
        return false;
    }

    if (!GetParcelableInfo<BundleInfo>(IBundleMgr::Message::GET_BUNDLE_INFO, data, bundleInfo)) {
        APP_LOGE("fail to GetBundleInfo from server");
        return false;
    }
    return true;
}

bool BundleMgrProxy::GetBundleInfo(
    const std::string &bundleName, int32_t flags, BundleInfo &bundleInfo, int32_t userId)
{
    BYTRACE_NAME(BYTRACE_TAG_APP, __PRETTY_FUNCTION__);
    APP_LOGD("begin to get bundle info of %{public}s", bundleName.c_str());
    if (bundleName.empty()) {
        APP_LOGE("fail to GetBundleInfo due to params empty");
        return false;
    }

    MessageParcel data;
    if (!data.WriteInterfaceToken(GetDescriptor())) {
        APP_LOGE("fail to GetBundleInfo due to write InterfaceToken fail");
        return false;
    }
    if (!data.WriteString(bundleName)) {
        APP_LOGE("fail to GetBundleInfo due to write bundleName fail");
        return false;
    }
    if (!data.WriteInt32(flags)) {
        APP_LOGE("fail to GetBundleInfo due to write flag fail");
        return false;
    }
    if (!data.WriteInt32(static_cast<int>(userId))) {
        APP_LOGE("fail to GetBundleInfo due to write userId fail");
        return false;
    }

    if (!GetParcelableInfo<BundleInfo>(IBundleMgr::Message::GET_BUNDLE_INFO_WITH_INT_FLAGS, data, bundleInfo)) {
        APP_LOGE("fail to GetBundleInfo from server");
        return false;
    }
    return true;
}

bool BundleMgrProxy::GetBundleInfos(
    const BundleFlag flag, std::vector<BundleInfo> &bundleInfos, int32_t userId)
{
    BYTRACE_NAME(BYTRACE_TAG_APP, __PRETTY_FUNCTION__);
    APP_LOGD("begin to get bundle infos");
    MessageParcel data;
    if (!data.WriteInterfaceToken(GetDescriptor())) {
        APP_LOGE("fail to GetBundleInfos due to write InterfaceToken fail");
        return false;
    }
    if (!data.WriteInt32(static_cast<int>(flag))) {
        APP_LOGE("fail to GetBundleInfos due to write flag fail");
        return false;
    }
    if (!data.WriteInt32(static_cast<int>(userId))) {
        APP_LOGE("fail to GetBundleInfo due to write userId fail");
        return false;
    }

    if (!GetParcelableInfosFromAshmem<BundleInfo>(
        IBundleMgr::Message::GET_BUNDLE_INFOS, data, bundleInfos)) {
        APP_LOGE("fail to GetBundleInfos from server");
        return false;
    }
    return true;
}

bool BundleMgrProxy::GetBundleInfos(
    int32_t flags, std::vector<BundleInfo> &bundleInfos, int32_t userId)
{
    BYTRACE_NAME(BYTRACE_TAG_APP, __PRETTY_FUNCTION__);
    APP_LOGD("begin to get bundle infos");
    MessageParcel data;
    if (!data.WriteInterfaceToken(GetDescriptor())) {
        APP_LOGE("fail to GetBundleInfos due to write InterfaceToken fail");
        return false;
    }
    if (!data.WriteInt32(flags)) {
        APP_LOGE("fail to GetBundleInfos due to write flag fail");
        return false;
    }
    if (!data.WriteInt32(static_cast<int>(userId))) {
        APP_LOGE("fail to GetBundleInfo due to write userId fail");
        return false;
    }

    if (!GetParcelableInfosFromAshmem<BundleInfo>(
        IBundleMgr::Message::GET_BUNDLE_INFOS_WITH_INT_FLAGS, data, bundleInfos)) {
        APP_LOGE("fail to GetBundleInfos from server");
        return false;
    }
    return true;
}

int BundleMgrProxy::GetUidByBundleName(const std::string &bundleName, const int userId)
{
    if (bundleName.empty()) {
        APP_LOGE("failed to GetUidByBundleName due to bundleName empty");
        return Constants::INVALID_UID;
    }
    BYTRACE_NAME(BYTRACE_TAG_APP, __PRETTY_FUNCTION__);
    APP_LOGD("begin to get uid of %{public}s, userId : %{public}d", bundleName.c_str(), userId);

    MessageParcel data;
    if (!data.WriteInterfaceToken(GetDescriptor())) {
        APP_LOGE("failed to GetUidByBundleName due to write InterfaceToken fail");
        return Constants::INVALID_UID;
    }
    if (!data.WriteString(bundleName)) {
        APP_LOGE("failed to GetUidByBundleName due to write bundleName fail");
        return Constants::INVALID_UID;
    }
    if (!data.WriteInt32(userId)) {
        APP_LOGE("failed to GetUidByBundleName due to write uid fail");
        return Constants::INVALID_UID;
    }

    MessageParcel reply;
    if (!SendTransactCmd(IBundleMgr::Message::GET_UID_BY_BUNDLE_NAME, data, reply)) {
        APP_LOGE("failed to GetUidByBundleName from server");
        return Constants::INVALID_UID;
    }
    int32_t uid = reply.ReadInt32();
    APP_LOGD("uid is %{public}d", uid);
    return uid;
}

std::string BundleMgrProxy::GetAppIdByBundleName(const std::string &bundleName, const int userId)
{
    if (bundleName.empty()) {
        APP_LOGE("failed to GetAppIdByBundleName due to bundleName empty");
        return Constants::EMPTY_STRING;
    }
    BYTRACE_NAME(BYTRACE_TAG_APP, __PRETTY_FUNCTION__);
    APP_LOGD("begin to get appId of %{public}s", bundleName.c_str());

    MessageParcel data;
    if (!data.WriteInterfaceToken(GetDescriptor())) {
        APP_LOGE("failed to GetAppIdByBundleName due to write InterfaceToken fail");
        return Constants::EMPTY_STRING;
    }
    if (!data.WriteString(bundleName)) {
        APP_LOGE("failed to GetAppIdByBundleName due to write bundleName fail");
        return Constants::EMPTY_STRING;
    }
    if (!data.WriteInt32(userId)) {
        APP_LOGE("failed to GetAppIdByBundleName due to write uid fail");
        return Constants::EMPTY_STRING;
    }

    MessageParcel reply;
    if (!SendTransactCmd(IBundleMgr::Message::GET_APPID_BY_BUNDLE_NAME, data, reply)) {
        APP_LOGE("failed to GetAppIdByBundleName from server");
        return Constants::EMPTY_STRING;
    }
    std::string appId = reply.ReadString();
    APP_LOGD("appId is %{public}s", appId.c_str());
    return appId;
}

bool BundleMgrProxy::GetBundleNameForUid(const int uid, std::string &bundleName)
{
    BYTRACE_NAME(BYTRACE_TAG_APP, __PRETTY_FUNCTION__);
    APP_LOGI("begin to GetBundleNameForUid of %{public}d", uid);
    MessageParcel data;
    if (!data.WriteInterfaceToken(GetDescriptor())) {
        APP_LOGE("fail to GetBundleNameForUid due to write InterfaceToken fail");
        return false;
    }
    if (!data.WriteInt32(uid)) {
        APP_LOGE("fail to GetBundleNameForUid due to write uid fail");
        return false;
    }

    MessageParcel reply;
    if (!SendTransactCmd(IBundleMgr::Message::GET_BUNDLE_NAME_FOR_UID, data, reply)) {
        APP_LOGE("fail to GetBundleNameForUid from server");
        return false;
    }
    if (!reply.ReadBool()) {
        APP_LOGE("reply result false");
        return false;
    }
    bundleName = reply.ReadString();
    return true;
}

bool BundleMgrProxy::GetBundlesForUid(const int uid, std::vector<std::string> &bundleNames)
{
    BYTRACE_NAME(BYTRACE_TAG_APP, __PRETTY_FUNCTION__);
    APP_LOGI("begin to GetBundlesForUid of %{public}d", uid);
    MessageParcel data;
    if (!data.WriteInterfaceToken(GetDescriptor())) {
        APP_LOGE("fail to GetBundlesForUid due to write InterfaceToken fail");
        return false;
    }
    if (!data.WriteInt32(uid)) {
        APP_LOGE("fail to GetBundlesForUid due to write uid fail");
        return false;
    }

    MessageParcel reply;
    if (!SendTransactCmd(IBundleMgr::Message::GET_BUNDLES_FOR_UID, data, reply)) {
        APP_LOGE("fail to GetBundlesForUid from server");
        return false;
    }
    if (!reply.ReadBool()) {
        APP_LOGE("reply result false");
        return false;
    }
    if (!reply.ReadStringVector(&bundleNames)) {
        APP_LOGE("fail to GetBundlesForUid from reply");
        return false;
    }
    return true;
}

bool BundleMgrProxy::GetNameForUid(const int uid, std::string &name)
{
    BYTRACE_NAME(BYTRACE_TAG_APP, __PRETTY_FUNCTION__);
    APP_LOGI("begin to GetNameForUid of %{public}d", uid);
    MessageParcel data;
    if (!data.WriteInterfaceToken(GetDescriptor())) {
        APP_LOGE("fail to GetNameForUid due to write InterfaceToken fail");
        return false;
    }
    if (!data.WriteInt32(uid)) {
        APP_LOGE("fail to GetNameForUid due to write uid fail");
        return false;
    }

    MessageParcel reply;
    if (!SendTransactCmd(IBundleMgr::Message::GET_NAME_FOR_UID, data, reply)) {
        APP_LOGE("fail to GetNameForUid from server");
        return false;
    }
    if (!reply.ReadBool()) {
        APP_LOGE("reply result false");
        return false;
    }
    name = reply.ReadString();
    return true;
}

bool BundleMgrProxy::GetBundleGids(const std::string &bundleName, std::vector<int> &gids)
{
    BYTRACE_NAME(BYTRACE_TAG_APP, __PRETTY_FUNCTION__);
    APP_LOGI("begin to GetBundleGids of %{public}s", bundleName.c_str());
    MessageParcel data;
    if (!data.WriteInterfaceToken(GetDescriptor())) {
        APP_LOGE("fail to GetBundleGids due to write InterfaceToken fail");
        return false;
    }
    if (!data.WriteString(bundleName)) {
        APP_LOGE("fail to GetBundleGids due to write bundleName fail");
        return false;
    }

    MessageParcel reply;
    if (!SendTransactCmd(IBundleMgr::Message::GET_BUNDLE_GIDS, data, reply)) {
        APP_LOGE("fail to GetBundleGids from server");
        return false;
    }
    if (!reply.ReadBool()) {
        APP_LOGE("reply result false");
        return false;
    }
    if (!reply.ReadInt32Vector(&gids)) {
        APP_LOGE("fail to GetBundleGids from reply");
        return false;
    }
    return true;
}

bool BundleMgrProxy::GetBundleGidsByUid(const std::string &bundleName, const int &uid, std::vector<int> &gids)
{
    BYTRACE_NAME(BYTRACE_TAG_APP, __PRETTY_FUNCTION__);
    APP_LOGI("begin to GetBundleGidsByUid of %{public}s", bundleName.c_str());
    MessageParcel data;
    if (!data.WriteInterfaceToken(GetDescriptor())) {
        APP_LOGE("fail to GetBundleGidsByUid due to write InterfaceToken fail");
        return false;
    }
    if (!data.WriteString(bundleName)) {
        APP_LOGE("fail to GetBundleGidsByUid due to write bundleName fail");
        return false;
    }
    if (!data.WriteInt32(uid)) {
        APP_LOGE("fail to GetBundleGidsByUid due to write uid fail");
        return false;
    }

    MessageParcel reply;
    if (!SendTransactCmd(IBundleMgr::Message::GET_BUNDLE_GIDS_BY_UID, data, reply)) {
        APP_LOGE("fail to GetBundleGidsByUid from server");
        return false;
    }
    if (!reply.ReadBool()) {
        APP_LOGE("reply result false");
        return false;
    }
    if (!reply.ReadInt32Vector(&gids)) {
        APP_LOGE("fail to GetBundleGidsByUid from reply");
        return false;
    }
    return true;
}

std::string BundleMgrProxy::GetAppType(const std::string &bundleName)
{
    if (bundleName.empty()) {
        APP_LOGE("failed to GetAppType due to bundleName empty");
        return Constants::EMPTY_STRING;
    }
    BYTRACE_NAME(BYTRACE_TAG_APP, __PRETTY_FUNCTION__);
    APP_LOGD("begin to GetAppType of %{public}s", bundleName.c_str());

    MessageParcel data;
    if (!data.WriteInterfaceToken(GetDescriptor())) {
        APP_LOGE("failed to GetAppType due to write InterfaceToken fail");
        return Constants::EMPTY_STRING;
    }
    if (!data.WriteString(bundleName)) {
        APP_LOGE("failed to GetAppType due to write bundleName fail");
        return Constants::EMPTY_STRING;
    }

    MessageParcel reply;
    if (!SendTransactCmd(IBundleMgr::Message::GET_APP_TYPE, data, reply)) {
        APP_LOGE("failed to GetAppType from server");
        return Constants::EMPTY_STRING;
    }
    std::string appType = reply.ReadString();
    APP_LOGD("appType is %{public}s", appType.c_str());
    return appType;
}

bool BundleMgrProxy::CheckIsSystemAppByUid(const int uid)
{
    BYTRACE_NAME(BYTRACE_TAG_APP, __PRETTY_FUNCTION__);
    APP_LOGI("begin to CheckIsSystemAppByUid of %{public}d", uid);
    MessageParcel data;
    if (!data.WriteInterfaceToken(GetDescriptor())) {
        APP_LOGE("fail to CheckIsSystemAppByUid due to write InterfaceToken fail");
        return false;
    }
    if (!data.WriteInt32(uid)) {
        APP_LOGE("fail to CheckIsSystemAppByUid due to write uid fail");
        return false;
    }

    MessageParcel reply;
    if (!SendTransactCmd(IBundleMgr::Message::CHECK_IS_SYSTEM_APP_BY_UID, data, reply)) {
        APP_LOGE("fail to CheckIsSystemAppByUid from server");
        return false;
    }
    return reply.ReadBool();
}

bool BundleMgrProxy::GetBundleInfosByMetaData(const std::string &metaData, std::vector<BundleInfo> &bundleInfos)
{
    BYTRACE_NAME(BYTRACE_TAG_APP, __PRETTY_FUNCTION__);
    APP_LOGI("begin to GetBundleInfosByMetaData of %{public}s", metaData.c_str());
    if (metaData.empty()) {
        APP_LOGE("fail to GetBundleInfosByMetaData due to params empty");
        return false;
    }

    MessageParcel data;
    if (!data.WriteInterfaceToken(GetDescriptor())) {
        APP_LOGE("fail to GetBundleInfosByMetaData due to write InterfaceToken fail");
        return false;
    }
    if (!data.WriteString(metaData)) {
        APP_LOGE("fail to GetBundleInfosByMetaData due to write metaData fail");
        return false;
    }

    if (!GetParcelableInfos<BundleInfo>(IBundleMgr::Message::GET_BUNDLE_INFOS_BY_METADATA, data, bundleInfos)) {
        APP_LOGE("fail to GetBundleInfosByMetaData from server");
        return false;
    }
    return true;
}

bool BundleMgrProxy::QueryAbilityInfo(const Want &want, AbilityInfo &abilityInfo)
{
    BYTRACE_NAME(BYTRACE_TAG_APP, __PRETTY_FUNCTION__);
    MessageParcel data;
    if (!data.WriteInterfaceToken(GetDescriptor())) {
        APP_LOGE("fail to QueryAbilityInfo due to write MessageParcel fail");
        return false;
    }
    if (!data.WriteParcelable(&want)) {
        APP_LOGE("fail to QueryAbilityInfo due to write want fail");
        return false;
    }

    if (!GetParcelableInfo<AbilityInfo>(IBundleMgr::Message::QUERY_ABILITY_INFO, data, abilityInfo)) {
        APP_LOGE("fail to query ability info from server");
        return false;
    }
    return true;
}

bool BundleMgrProxy::QueryAbilityInfo(const Want &want, int32_t flags, int32_t userId, AbilityInfo &abilityInfo)
{
    BYTRACE_NAME(BYTRACE_TAG_APP, __PRETTY_FUNCTION__);
    MessageParcel data;
    if (!data.WriteInterfaceToken(GetDescriptor())) {
        APP_LOGE("fail to QueryAbilityInfo mutiparam due to write MessageParcel fail");
        return false;
    }
    if (!data.WriteParcelable(&want)) {
        APP_LOGE("fail to QueryAbilityInfo mutiparam due to write want fail");
        return false;
    }
    if (!data.WriteInt32(flags)) {
        APP_LOGE("fail to QueryAbilityInfo mutiparam due to write flags fail");
        return false;
    }
    if (!data.WriteInt32(userId)) {
        APP_LOGE("fail to QueryAbilityInfo mutiparam due to write userId error");
        return false;
    }

    if (!GetParcelableInfo<AbilityInfo>(IBundleMgr::Message::QUERY_ABILITY_INFO_MUTI_PARAM, data, abilityInfo)) {
        APP_LOGE("fail to query ability info mutiparam from server");
        return false;
    }
    return true;
}

bool BundleMgrProxy::QueryAbilityInfos(const Want &want, std::vector<AbilityInfo> &abilityInfos)
{
    BYTRACE_NAME(BYTRACE_TAG_APP, __PRETTY_FUNCTION__);
    MessageParcel data;
    if (!data.WriteInterfaceToken(GetDescriptor())) {
        APP_LOGE("fail to QueryAbilityInfos due to write MessageParcel fail");
        return false;
    }
    if (!data.WriteParcelable(&want)) {
        APP_LOGE("fail to QueryAbilityInfos due to write want fail");
        return false;
    }

    if (!GetParcelableInfos<AbilityInfo>(IBundleMgr::Message::QUERY_ABILITY_INFOS, data, abilityInfos)) {
        APP_LOGE("fail to QueryAbilityInfos from server");
        return false;
    }
    return true;
}

bool BundleMgrProxy::QueryAbilityInfos(
    const Want &want, int32_t flags, int32_t userId, std::vector<AbilityInfo> &abilityInfos)
{
    BYTRACE_NAME(BYTRACE_TAG_APP, __PRETTY_FUNCTION__);
    MessageParcel data;
    if (!data.WriteInterfaceToken(GetDescriptor())) {
        APP_LOGE("fail to QueryAbilityInfos mutiparam due to write MessageParcel fail");
        return false;
    }
    if (!data.WriteParcelable(&want)) {
        APP_LOGE("fail to QueryAbilityInfos mutiparam due to write want fail");
        return false;
    }
    if (!data.WriteInt32(flags)) {
        APP_LOGE("fail to QueryAbilityInfos mutiparam due to write flags fail");
        return false;
    }
    if (!data.WriteInt32(userId)) {
        APP_LOGE("fail to QueryAbilityInfos mutiparam due to write userId error");
        return false;
    }

    if (!GetParcelableInfos<AbilityInfo>(IBundleMgr::Message::QUERY_ABILITY_INFOS_MUTI_PARAM, data, abilityInfos)) {
        APP_LOGE("fail to QueryAbilityInfos mutiparam from server");
        return false;
    }
    return true;
}

bool BundleMgrProxy::QueryAllAbilityInfos(const Want &want, int32_t userId, std::vector<AbilityInfo> &abilityInfos)
{
    BYTRACE_NAME(BYTRACE_TAG_APP, __PRETTY_FUNCTION__);
    MessageParcel data;
    if (!data.WriteInterfaceToken(GetDescriptor())) {
        APP_LOGE("fail to QueryAbilityInfo due to write MessageParcel fail");
        return false;
    }
    if (!data.WriteParcelable(&want)) {
        APP_LOGE("fail to QueryAbilityInfos mutiparam due to write want fail");
        return false;
    }
    if (!data.WriteInt32(userId)) {
        APP_LOGE("fail to QueryAbilityInfo due to write want fail");
        return false;
    }

    if (!GetParcelableInfos<AbilityInfo>(IBundleMgr::Message::QUERY_ALL_ABILITY_INFOS, data, abilityInfos)) {
        APP_LOGE("fail to QueryAbilityInfos from server");
        return false;
    }
    return true;
}

bool BundleMgrProxy::QueryAbilityInfosForClone(const Want &want, std::vector<AbilityInfo> &abilityInfos)
{
    BYTRACE_NAME(BYTRACE_TAG_APP, __PRETTY_FUNCTION__);
    MessageParcel data;
    if (!data.WriteInterfaceToken(GetDescriptor())) {
        APP_LOGE("fail to QueryAbilityInfo due to write MessageParcel fail");
        return false;
    }
    if (!data.WriteParcelable(&want)) {
        APP_LOGE("fail to QueryAbilityInfo due to write want fail");
        return false;
    }

    if (!GetParcelableInfos<AbilityInfo>(IBundleMgr::Message::QUERY_ABILITY_INFOS_FOR_CLONE, data, abilityInfos)) {
        APP_LOGE("fail to QueryAbilityInfos from server");
        return false;
    }
    return true;
}

bool BundleMgrProxy::QueryAbilityInfoByUri(const std::string &abilityUri, AbilityInfo &abilityInfo)
{
    BYTRACE_NAME(BYTRACE_TAG_APP, __PRETTY_FUNCTION__);
    MessageParcel data;
    if (!data.WriteInterfaceToken(GetDescriptor())) {
        APP_LOGE("fail to QueryAbilityInfoByUri due to write MessageParcel fail");
        return false;
    }
    if (!data.WriteString(abilityUri)) {
        APP_LOGE("fail to QueryAbilityInfoByUri due to write abilityUri fail");
        return false;
    }

    if (!GetParcelableInfo<AbilityInfo>(IBundleMgr::Message::QUERY_ABILITY_INFO_BY_URI, data, abilityInfo)) {
        APP_LOGE("fail to QueryAbilityInfoByUri from server");
        return false;
    }
    return true;
}

bool BundleMgrProxy::QueryAbilityInfosByUri(const std::string &abilityUri, std::vector<AbilityInfo> &abilityInfos)
{
    BYTRACE_NAME(BYTRACE_TAG_APP, __PRETTY_FUNCTION__);
    MessageParcel data;
    if (!data.WriteInterfaceToken(GetDescriptor())) {
        APP_LOGE("fail to QueryAbilityInfosByUri due to write MessageParcel fail");
        return false;
    }
    if (!data.WriteString(abilityUri)) {
        APP_LOGE("fail to QueryAbilityInfosByUri due to write abilityUri fail");
        return false;
    }

    if (!GetParcelableInfos<AbilityInfo>(IBundleMgr::Message::QUERY_ABILITY_INFOS_BY_URI, data, abilityInfos)) {
        APP_LOGE("fail to QueryAbilityInfosByUri from server");
        return false;
    }
    return true;
}

bool BundleMgrProxy::QueryAbilityInfoByUri(
    const std::string &abilityUri, int32_t userId, AbilityInfo &abilityInfo)
{
    MessageParcel data;
    if (!data.WriteInterfaceToken(GetDescriptor())) {
        APP_LOGE("fail to QueryAbilityInfoByUri due to write MessageParcel fail");
        return false;
    }
    if (!data.WriteString(abilityUri)) {
        APP_LOGE("fail to QueryAbilityInfoByUri due to write abilityUri fail");
        return false;
    }
    if (!data.WriteInt32(userId)) {
        APP_LOGE("fail to QueryAbilityInfo due to write want fail");
        return false;
    }

    if (!GetParcelableInfo<AbilityInfo>(
        IBundleMgr::Message::QUERY_ABILITY_INFO_BY_URI_FOR_USERID, data, abilityInfo)) {
        APP_LOGE("fail to QueryAbilityInfoByUri from server");
        return false;
    }
    return true;
}

bool BundleMgrProxy::QueryKeepAliveBundleInfos(std::vector<BundleInfo> &bundleInfos)
{
    BYTRACE_NAME(BYTRACE_TAG_APP, __PRETTY_FUNCTION__);
    APP_LOGI("begin to QueryKeepAliveBundleInfos");

    MessageParcel data;
    if (!data.WriteInterfaceToken(GetDescriptor())) {
        APP_LOGE("fail to QueryKeepAliveBundleInfos due to write InterfaceToken fail");
        return false;
    }

    if (!GetParcelableInfos<BundleInfo>(IBundleMgr::Message::QUERY_KEEPALIVE_BUNDLE_INFOS, data, bundleInfos)) {
        APP_LOGE("fail to QueryKeepAliveBundleInfos from server");
        return false;
    }
    return true;
}

std::string BundleMgrProxy::GetAbilityLabel(const std::string &bundleName, const std::string &className)
{
    BYTRACE_NAME(BYTRACE_TAG_APP, __PRETTY_FUNCTION__);
    APP_LOGI("begin to get bundle info of %{public}s", bundleName.c_str());
    if (bundleName.empty()) {
        APP_LOGE("fail to GetAbilityLabel due to params empty");
        return Constants::EMPTY_STRING;
    }

    MessageParcel data;
    if (!data.WriteInterfaceToken(GetDescriptor())) {
        APP_LOGE("fail to GetAbilityLabel due to write InterfaceToken fail");
        return Constants::EMPTY_STRING;
    }
    if (!data.WriteString(bundleName)) {
        APP_LOGE("fail to GetAbilityLabel due to write bundleName fail");
        return Constants::EMPTY_STRING;
    }
    if (!data.WriteString(className)) {
        APP_LOGE("fail to GetAbilityLabel due to write className fail");
        return Constants::EMPTY_STRING;
    }

    MessageParcel reply;
    if (!SendTransactCmd(IBundleMgr::Message::GET_ABILITY_LABEL, data, reply)) {
        APP_LOGE("fail to GetAbilityLabel from server");
        return Constants::EMPTY_STRING;
    }
    return reply.ReadString();
}

bool BundleMgrProxy::GetBundleArchiveInfo(const std::string &hapFilePath, const BundleFlag flag, BundleInfo &bundleInfo)
{
    BYTRACE_NAME(BYTRACE_TAG_APP, __PRETTY_FUNCTION__);
    APP_LOGD("begin to GetBundleArchiveInfo of %{private}s", hapFilePath.c_str());
    if (hapFilePath.empty()) {
        APP_LOGE("fail to GetBundleArchiveInfo due to params empty");
        return false;
    }

    MessageParcel data;
    if (!data.WriteInterfaceToken(GetDescriptor())) {
        APP_LOGE("fail to GetBundleArchiveInfo due to write InterfaceToken fail");
        return false;
    }
    if (!data.WriteString(hapFilePath)) {
        APP_LOGE("fail to GetBundleArchiveInfo due to write hapFilePath fail");
        return false;
    }
    if (!data.WriteInt32(static_cast<int>(flag))) {
        APP_LOGE("fail to GetBundleArchiveInfo due to write flag fail");
        return false;
    }

    if (!GetParcelableInfo<BundleInfo>(IBundleMgr::Message::GET_BUNDLE_ARCHIVE_INFO, data, bundleInfo)) {
        APP_LOGE("fail to GetBundleArchiveInfo from server");
        return false;
    }
    return true;
}

bool BundleMgrProxy::GetBundleArchiveInfo(const std::string &hapFilePath, int32_t flags, BundleInfo &bundleInfo)
{
    BYTRACE_NAME(BYTRACE_TAG_APP, __PRETTY_FUNCTION__);
    APP_LOGD("begin to GetBundleArchiveInfo with int flags of %{private}s", hapFilePath.c_str());
    if (hapFilePath.empty()) {
        APP_LOGE("fail to GetBundleArchiveInfo due to params empty");
        return false;
    }

    MessageParcel data;
    if (!data.WriteInterfaceToken(GetDescriptor())) {
        APP_LOGE("fail to GetBundleArchiveInfo due to write InterfaceToken fail");
        return false;
    }
    if (!data.WriteString(hapFilePath)) {
        APP_LOGE("fail to GetBundleArchiveInfo due to write hapFilePath fail");
        return false;
    }
    if (!data.WriteInt32(flags)) {
        APP_LOGE("fail to GetBundleArchiveInfo due to write flags fail");
        return false;
    }

    if (!GetParcelableInfo<BundleInfo>(IBundleMgr::Message::GET_BUNDLE_ARCHIVE_INFO_WITH_INT_FLAGS, data, bundleInfo)) {
        APP_LOGE("fail to GetBundleArchiveInfo from server");
        return false;
    }
    return true;
}

bool BundleMgrProxy::GetHapModuleInfo(const AbilityInfo &abilityInfo, HapModuleInfo &hapModuleInfo)
{
    BYTRACE_NAME(BYTRACE_TAG_APP, __PRETTY_FUNCTION__);
    APP_LOGI("begin to GetHapModuleInfo of %{public}s", abilityInfo.package.c_str());
    if (abilityInfo.bundleName.empty() || abilityInfo.package.empty()) {
        APP_LOGE("fail to GetHapModuleInfo due to params empty");
        return false;
    }

    MessageParcel data;
    if (!data.WriteInterfaceToken(GetDescriptor())) {
        APP_LOGE("fail to GetHapModuleInfo due to write InterfaceToken fail");
        return false;
    }
    if (!data.WriteParcelable(&abilityInfo)) {
        APP_LOGE("fail to GetHapModuleInfo due to write abilityInfo fail");
        return false;
    }

    if (!GetParcelableInfo<HapModuleInfo>(IBundleMgr::Message::GET_HAP_MODULE_INFO, data, hapModuleInfo)) {
        APP_LOGE("fail to GetHapModuleInfo from server");
        return false;
    }
    return true;
}

bool BundleMgrProxy::GetLaunchWantForBundle(const std::string &bundleName, Want &want)
{
    BYTRACE_NAME(BYTRACE_TAG_APP, __PRETTY_FUNCTION__);
    APP_LOGI("begin to GetLaunchWantForBundle of %{public}s", bundleName.c_str());
    if (bundleName.empty()) {
        APP_LOGE("fail to GetHapModuleInfo due to params empty");
        return false;
    }

    MessageParcel data;
    if (!data.WriteInterfaceToken(GetDescriptor())) {
        APP_LOGE("fail to GetLaunchWantForBundle due to write InterfaceToken fail");
        return false;
    }
    if (!data.WriteString(bundleName)) {
        APP_LOGE("fail to GetLaunchWantForBundle due to write bundleName fail");
        return false;
    }

    if (!GetParcelableInfo<Want>(IBundleMgr::Message::GET_LAUNCH_WANT_FOR_BUNDLE, data, want)) {
        APP_LOGE("fail to GetLaunchWantForBundle from server");
        return false;
    }
    return true;
}

int BundleMgrProxy::CheckPublicKeys(const std::string &firstBundleName, const std::string &secondBundleName)
{
    BYTRACE_NAME(BYTRACE_TAG_APP, __PRETTY_FUNCTION__);
    APP_LOGI(
        "begin to CheckPublicKeys of %{public}s and %{public}s", firstBundleName.c_str(), secondBundleName.c_str());
    if (firstBundleName.empty() || secondBundleName.empty()) {
        APP_LOGE("fail to CheckPublicKeys due to params empty");
        return Constants::SIGNATURE_UNKNOWN_BUNDLE;
    }

    MessageParcel data;
    if (!data.WriteInterfaceToken(GetDescriptor())) {
        APP_LOGE("fail to GetBundleInfo due to write MessageParcel fail");
        return Constants::SIGNATURE_UNKNOWN_BUNDLE;
    }
    if (!data.WriteString(firstBundleName)) {
        APP_LOGE("fail to GetBundleInfo due to write firstBundleName fail");
        return Constants::SIGNATURE_UNKNOWN_BUNDLE;
    }
    if (!data.WriteString(secondBundleName)) {
        APP_LOGE("fail to GetBundleInfo due to write secondBundleName fail");
        return Constants::SIGNATURE_UNKNOWN_BUNDLE;
    }

    MessageParcel reply;
    if (!SendTransactCmd(IBundleMgr::Message::CHECK_PUBLICKEYS, data, reply)) {
        APP_LOGE("fail to CheckPublicKeys from server");
        return Constants::SIGNATURE_UNKNOWN_BUNDLE;
    }
    return reply.ReadInt32();
}

int BundleMgrProxy::CheckPermission(const std::string &bundleName, const std::string &permission)
{
    BYTRACE_NAME(BYTRACE_TAG_APP, __PRETTY_FUNCTION__);
    APP_LOGI("begin to CheckPublicKeys of %{public}s and %{public}s", bundleName.c_str(), permission.c_str());
    if (bundleName.empty() || permission.empty()) {
        APP_LOGE("fail to CheckPermission due to params empty");
        return Constants::PERMISSION_NOT_GRANTED;
    }

    MessageParcel data;
    if (!data.WriteInterfaceToken(GetDescriptor())) {
        APP_LOGE("fail to CheckPermission due to write InterfaceToken fail");
        return Constants::PERMISSION_NOT_GRANTED;
    }
    if (!data.WriteString(bundleName)) {
        APP_LOGE("fail to CheckPermission due to write bundleName fail");
        return Constants::PERMISSION_NOT_GRANTED;
    }
    if (!data.WriteString(permission)) {
        APP_LOGE("fail to CheckPermission due to write permission fail");
        return Constants::PERMISSION_NOT_GRANTED;
    }

    MessageParcel reply;
    if (!SendTransactCmd(IBundleMgr::Message::CHECK_PERMISSION, data, reply)) {
        APP_LOGE("fail to CheckPermission from server");
        return Constants::PERMISSION_NOT_GRANTED;
    }
    return reply.ReadInt32();
}

int BundleMgrProxy::CheckPermissionByUid(const std::string &bundleName, const std::string &permission, const int userId)
{
    BYTRACE_NAME(BYTRACE_TAG_APP, __PRETTY_FUNCTION__);
    APP_LOGI("begin to CheckPublicKeys of %{public}s and %{public}s", bundleName.c_str(), permission.c_str());
    if (bundleName.empty() || permission.empty()) {
        APP_LOGE("fail to CheckPermissionByUid due to params empty");
        return Constants::PERMISSION_NOT_GRANTED;
    }

    MessageParcel data;
    if (!data.WriteInterfaceToken(GetDescriptor())) {
        APP_LOGE("fail to CheckPermissionByUid due to write InterfaceToken fail");
        return Constants::PERMISSION_NOT_GRANTED;
    }
    if (!data.WriteString(bundleName)) {
        APP_LOGE("fail to CheckPermissionByUid due to write bundleName fail");
        return Constants::PERMISSION_NOT_GRANTED;
    }
    if (!data.WriteString(permission)) {
        APP_LOGE("fail to CheckPermissionByUid due to write permission fail");
        return Constants::PERMISSION_NOT_GRANTED;
    }
    if (!data.WriteInt32(userId)) {
        APP_LOGE("fail to CheckPermissionByUid due to write userId fail");
        return Constants::PERMISSION_NOT_GRANTED;
    }

    MessageParcel reply;
    if (!SendTransactCmd(IBundleMgr::Message::CHECK_PERMISSION_BY_UID, data, reply)) {
        APP_LOGE("fail to CheckPermissionByUid from server");
        return Constants::PERMISSION_NOT_GRANTED;
    }
    return reply.ReadInt32();
}

bool BundleMgrProxy::GetPermissionDef(const std::string &permissionName, PermissionDef &permissionDef)
{
    BYTRACE_NAME(BYTRACE_TAG_APP, __PRETTY_FUNCTION__);
    APP_LOGI("begin to GetPermissionDef of %{public}s", permissionName.c_str());
    if (permissionName.empty()) {
        APP_LOGE("fail to GetPermissionDef due to params empty");
        return false;
    }

    MessageParcel data;
    if (!data.WriteInterfaceToken(GetDescriptor())) {
        APP_LOGE("fail to GetPermissionDef due to write InterfaceToken fail");
        return false;
    }
    if (!data.WriteString(permissionName)) {
        APP_LOGE("fail to GetPermissionDef due to write permissionName fail");
        return false;
    }

    if (!GetParcelableInfo<PermissionDef>(IBundleMgr::Message::GET_PERMISSION_DEF, data, permissionDef)) {
        APP_LOGE("fail to GetPermissionDef from server");
        return false;
    }
    return true;
}

bool BundleMgrProxy::GetAllPermissionGroupDefs(std::vector<PermissionDef> &permissionDefs)
{
    BYTRACE_NAME(BYTRACE_TAG_APP, __PRETTY_FUNCTION__);
    APP_LOGI("begin to GetPermissionDefs");
    MessageParcel data;
    if (!data.WriteInterfaceToken(GetDescriptor())) {
        APP_LOGE("fail to GetAllPermissionGroupDefs due to write InterfaceToken fail");
        return false;
    }

    if (!GetParcelableInfos<PermissionDef>(IBundleMgr::Message::GET_ALL_PERMISSION_GROUP_DEFS, data, permissionDefs)) {
        APP_LOGE("fail to GetAllPermissionGroupDefs from server");
        return false;
    }
    return true;
}

bool BundleMgrProxy::GetAppsGrantedPermissions(
    const std::vector<std::string> &permissions, std::vector<std::string> &appNames)
{
    BYTRACE_NAME(BYTRACE_TAG_APP, __PRETTY_FUNCTION__);
    APP_LOGI("begin to GetAppsGrantedPermissions");
    MessageParcel data;
    if (!data.WriteInterfaceToken(GetDescriptor())) {
        APP_LOGE("fail to GetAppsGrantedPermissions due to write InterfaceToken fail");
        return false;
    }
    if (!data.WriteStringVector(permissions)) {
        APP_LOGE("fail to GetAppsGrantedPermissions due to write permissions fail");
        return false;
    }

    MessageParcel reply;
    if (!SendTransactCmd(IBundleMgr::Message::GET_APPS_GRANTED_PERMISSIONS, data, reply)) {
        APP_LOGE("fail to GetAppsGrantedPermissions from server");
        return false;
    }

    if (!reply.ReadStringVector(&appNames)) {
        APP_LOGE("fail to GetAppsGrantedPermissions from reply");
        return false;
    }

    return true;
}

bool BundleMgrProxy::HasSystemCapability(const std::string &capName)
{
    BYTRACE_NAME(BYTRACE_TAG_APP, __PRETTY_FUNCTION__);
    APP_LOGI("begin to HasSystemCapability of %{public}s", capName.c_str());
    if (capName.empty()) {
        APP_LOGE("fail to HasSystemCapability due to params empty");
        return false;
    }

    MessageParcel data;
    if (!data.WriteInterfaceToken(GetDescriptor())) {
        APP_LOGE("fail to HasSystemCapability due to write InterfaceToken fail");
        return false;
    }
    if (!data.WriteString(capName)) {
        APP_LOGE("fail to HasSystemCapability due to write capName fail");
        return false;
    }

    MessageParcel reply;
    if (!SendTransactCmd(IBundleMgr::Message::HAS_SYSTEM_CAPABILITY, data, reply)) {
        APP_LOGE("fail to HasSystemCapability from server");
        return false;
    }
    return reply.ReadBool();
}

bool BundleMgrProxy::GetSystemAvailableCapabilities(std::vector<std::string> &systemCaps)
{
    BYTRACE_NAME(BYTRACE_TAG_APP, __PRETTY_FUNCTION__);
    APP_LOGI("begin to GetSystemAvailableCapabilities");
    MessageParcel data;
    if (!data.WriteInterfaceToken(GetDescriptor())) {
        APP_LOGE("fail to GetSystemAvailableCapabilities due to write InterfaceToken fail");
        return false;
    }

    MessageParcel reply;
    if (!SendTransactCmd(IBundleMgr::Message::GET_SYSTEM_AVAILABLE_CAPABILITIES, data, reply)) {
        APP_LOGE("fail to GetSystemAvailableCapabilities from server");
        return false;
    }

    if (!reply.ReadStringVector(&systemCaps)) {
        APP_LOGE("fail to GetSystemAvailableCapabilities from reply");
        return false;
    }

    return true;
}

bool BundleMgrProxy::IsSafeMode()
{
    BYTRACE_NAME(BYTRACE_TAG_APP, __PRETTY_FUNCTION__);
    APP_LOGI("begin to IsSafeMode");
    MessageParcel data;
    if (!data.WriteInterfaceToken(GetDescriptor())) {
        APP_LOGE("fail to IsSafeMode due to write InterfaceToken fail");
        return false;
    }

    MessageParcel reply;
    if (!SendTransactCmd(IBundleMgr::Message::IS_SAFE_MODE, data, reply)) {
        APP_LOGE("fail to IsSafeMode from server");
        return false;
    }
    return reply.ReadBool();
}

bool BundleMgrProxy::CleanBundleCacheFiles(
    const std::string &bundleName, const sptr<ICleanCacheCallback> &cleanCacheCallback, int32_t userId)
{
    BYTRACE_NAME(BYTRACE_TAG_APP, __PRETTY_FUNCTION__);
    APP_LOGI("begin to CleanBundleCacheFiles of %{public}s", bundleName.c_str());
    if (bundleName.empty() || !cleanCacheCallback) {
        APP_LOGE("fail to CleanBundleCacheFiles due to params error");
        return false;
    }

    MessageParcel data;
    if (!data.WriteInterfaceToken(GetDescriptor())) {
        APP_LOGE("fail to CleanBundleCacheFiles due to write InterfaceToken fail");
        return false;
    }
    if (!data.WriteString(bundleName)) {
        APP_LOGE("fail to CleanBundleCacheFiles due to write bundleName fail");
        return false;
    }
    if (!data.WriteParcelable(cleanCacheCallback->AsObject())) {
        APP_LOGE("fail to CleanBundleCacheFiles, for write parcel failed");
        return false;
    }
    if (!data.WriteInt32(userId)) {
        APP_LOGE("fail to CleanBundleCacheFiles due to write userId fail");
        return false;
    }

    MessageParcel reply;
    if (!SendTransactCmd(IBundleMgr::Message::CLEAN_BUNDLE_CACHE_FILES, data, reply)) {
        APP_LOGE("fail to CleanBundleCacheFiles from server");
        return false;
    }
    return reply.ReadBool();
}

bool BundleMgrProxy::CleanBundleDataFiles(const std::string &bundleName, const int userId)
{
    BYTRACE_NAME(BYTRACE_TAG_APP, __PRETTY_FUNCTION__);
    APP_LOGI("begin to CleanBundleDataFiles of %{public}s", bundleName.c_str());
    if (bundleName.empty()) {
        APP_LOGE("fail to CleanBundleDataFiles due to params empty");
        return false;
    }

    MessageParcel data;
    if (!data.WriteInterfaceToken(GetDescriptor())) {
        APP_LOGE("fail to CleanBundleDataFiles due to write InterfaceToken fail");
        return false;
    }
    if (!data.WriteString(bundleName)) {
        APP_LOGE("fail to CleanBundleDataFiles due to write hapFilePath fail");
        return false;
    }
    if (!data.WriteInt32(userId)) {
        APP_LOGE("fail to CleanBundleDataFiles due to write userId fail");
        return false;
    }

    MessageParcel reply;
    if (!SendTransactCmd(IBundleMgr::Message::CLEAN_BUNDLE_DATA_FILES, data, reply)) {
        APP_LOGE("fail to CleanBundleDataFiles from server");
        return false;
    }
    return reply.ReadBool();
}

bool BundleMgrProxy::RegisterBundleStatusCallback(const sptr<IBundleStatusCallback> &bundleStatusCallback)
{
    BYTRACE_NAME(BYTRACE_TAG_APP, __PRETTY_FUNCTION__);
    APP_LOGI("begin to RegisterBundleStatusCallback");
    if (!bundleStatusCallback || bundleStatusCallback->GetBundleName().empty()) {
        APP_LOGE("fail to RegisterBundleStatusCallback");
        return false;
    }

    MessageParcel data;
    if (!data.WriteInterfaceToken(GetDescriptor())) {
        APP_LOGE("fail to RegisterBundleStatusCallback due to write InterfaceToken fail");
        return false;
    }
    if (!data.WriteString(bundleStatusCallback->GetBundleName())) {
        APP_LOGE("fail to RegisterBundleStatusCallback due to write bundleName fail");
        return false;
    }
    if (!data.WriteParcelable(bundleStatusCallback->AsObject())) {
        APP_LOGE("fail to RegisterBundleStatusCallback, for write parcel failed");
        return false;
    }

    MessageParcel reply;
    if (!SendTransactCmd(IBundleMgr::Message::REGISTER_BUNDLE_STATUS_CALLBACK, data, reply)) {
        APP_LOGE("fail to RegisterBundleStatusCallback from server");
        return false;
    }
    return reply.ReadBool();
}

bool BundleMgrProxy::ClearBundleStatusCallback(const sptr<IBundleStatusCallback> &bundleStatusCallback)
{
    BYTRACE_NAME(BYTRACE_TAG_APP, __PRETTY_FUNCTION__);
    APP_LOGI("begin to ClearBundleStatusCallback");
    if (!bundleStatusCallback) {
        APP_LOGE("fail to ClearBundleStatusCallback, for bundleStatusCallback is nullptr");
        return false;
    }

    MessageParcel data;
    if (!data.WriteInterfaceToken(GetDescriptor())) {
        APP_LOGE("fail to ClearBundleStatusCallback due to write InterfaceToken fail");
        return false;
    }
    if (!data.WriteParcelable(bundleStatusCallback->AsObject())) {
        APP_LOGE("fail to ClearBundleStatusCallback, for write parcel failed");
        return false;
    }

    MessageParcel reply;
    if (!SendTransactCmd(IBundleMgr::Message::CLEAR_BUNDLE_STATUS_CALLBACK, data, reply)) {
        APP_LOGE("fail to CleanBundleCacheFiles from server");
        return false;
    }
    return reply.ReadBool();
}

bool BundleMgrProxy::UnregisterBundleStatusCallback()
{
    BYTRACE_NAME(BYTRACE_TAG_APP, __PRETTY_FUNCTION__);
    APP_LOGI("begin to UnregisterBundleStatusCallback");
    MessageParcel data;
    if (!data.WriteInterfaceToken(GetDescriptor())) {
        APP_LOGE("fail to UnregisterBundleStatusCallback due to write InterfaceToken fail");
        return false;
    }

    MessageParcel reply;
    if (!SendTransactCmd(IBundleMgr::Message::UNREGISTER_BUNDLE_STATUS_CALLBACK, data, reply)) {
        APP_LOGE("fail to UnregisterBundleStatusCallback from server");
        return false;
    }
    return reply.ReadBool();
}

bool BundleMgrProxy::DumpInfos(
    const DumpFlag flag, const std::string &bundleName, int32_t userId, std::string &result)
{
    BYTRACE_NAME(BYTRACE_TAG_APP, __PRETTY_FUNCTION__);
    APP_LOGD("begin to dump");
    MessageParcel data;
    if (!data.WriteInterfaceToken(GetDescriptor())) {
        APP_LOGE("fail to dump due to write InterfaceToken fail");
        return false;
    }
    if (!data.WriteInt32(static_cast<int>(flag))) {
        APP_LOGE("fail to dump due to write flag fail");
        return false;
    }
    if (!data.WriteString(bundleName)) {
        APP_LOGE("fail to dump due to write bundleName fail");
        return false;
    }
    if (!data.WriteInt32(static_cast<int>(userId))) {
        APP_LOGE("fail to dump due to write userId fail");
        return false;
    }

    MessageParcel reply;
    if (!SendTransactCmd(IBundleMgr::Message::DUMP_INFOS, data, reply)) {
        APP_LOGE("fail to dump from server");
        return false;
    }
    if (!reply.ReadBool()) {
        APP_LOGE("readParcelableInfo failed");
        return false;
    }
    std::vector<std::string> dumpInfos;
    if (!reply.ReadStringVector(&dumpInfos)) {
        APP_LOGE("fail to dump from reply");
        return false;
    }
    for (auto &dumpinfo : dumpInfos) {
        result += dumpinfo;
    }
    return true;
}

bool BundleMgrProxy::IsApplicationEnabled(const std::string &bundleName)
{
    BYTRACE_NAME(BYTRACE_TAG_APP, __PRETTY_FUNCTION__);
    APP_LOGI("begin to IsApplicationEnabled of %{public}s", bundleName.c_str());
    if (bundleName.empty()) {
        APP_LOGE("fail to IsApplicationEnabled due to params empty");
        return false;
    }

    MessageParcel data;
    if (!data.WriteInterfaceToken(GetDescriptor())) {
        APP_LOGE("fail to IsApplicationEnabled due to write InterfaceToken fail");
        return false;
    }
    if (!data.WriteString(bundleName)) {
        APP_LOGE("fail to IsApplicationEnabled due to write bundleName fail");
        return false;
    }

    MessageParcel reply;
    if (!SendTransactCmd(IBundleMgr::Message::IS_APPLICATION_ENABLED, data, reply)) {
        APP_LOGE("fail to IsApplicationEnabled from server");
        return false;
    }
    return reply.ReadBool();
}

bool BundleMgrProxy::SetApplicationEnabled(const std::string &bundleName, bool isEnable, int32_t userId)
{
    BYTRACE_NAME(BYTRACE_TAG_APP, __PRETTY_FUNCTION__);
    APP_LOGI("begin to SetApplicationEnabled of %{public}s", bundleName.c_str());
    if (bundleName.empty()) {
        APP_LOGE("fail to SetApplicationEnabled due to params empty");
        return false;
    }

    MessageParcel data;
    if (!data.WriteInterfaceToken(GetDescriptor())) {
        APP_LOGE("fail to SetApplicationEnabled due to write InterfaceToken fail");
        return false;
    }
    if (!data.WriteString(bundleName)) {
        APP_LOGE("fail to SetApplicationEnabled due to write bundleName fail");
        return false;
    }
    if (!data.WriteBool(isEnable)) {
        APP_LOGE("fail to IsApplicationEnabled due to write isEnable fail");
        return false;
    }
    if (!data.WriteInt32(userId)) {
        APP_LOGE("fail to IsApplicationEnabled due to write userId fail");
        return false;
    }

    MessageParcel reply;
    if (!SendTransactCmd(IBundleMgr::Message::SET_APPLICATION_ENABLED, data, reply)) {
        APP_LOGE("fail to SetApplicationEnabled from server");
        return false;
    }
    return reply.ReadBool();
}

bool BundleMgrProxy::IsAbilityEnabled(const AbilityInfo &abilityInfo)
{
    BYTRACE_NAME(BYTRACE_TAG_APP, __PRETTY_FUNCTION__);
    APP_LOGI("begin to IsAbilityEnabled of %{public}s", abilityInfo.name.c_str());
    if (abilityInfo.name.empty()) {
        APP_LOGE("fail to IsAbilityEnabled due to params empty");
        return false;
    }

    MessageParcel data;
    if (!data.WriteInterfaceToken(GetDescriptor())) {
        APP_LOGE("fail to IsAbilityEnabled due to write InterfaceToken fail");
        return false;
    }
    if (!data.WriteParcelable(&abilityInfo)) {
        APP_LOGE("fail to IsAbilityEnabled due to write abilityInfo fail");
        return false;
    }

    MessageParcel reply;
    if (!SendTransactCmd(IBundleMgr::Message::IS_ABILITY_ENABLED, data, reply)) {
        APP_LOGE("fail to IsAbilityEnabled from server");
        return false;
    }
    return reply.ReadBool();
}

bool BundleMgrProxy::SetAbilityEnabled(const AbilityInfo &abilityInfo, bool isEnabled, int32_t userId)
{
    BYTRACE_NAME(BYTRACE_TAG_APP, __PRETTY_FUNCTION__);
    APP_LOGI("begin to SetAbilityEnabled of %{public}s", abilityInfo.name.c_str());
    if (abilityInfo.name.empty()) {
        APP_LOGE("fail to SetAbilityEnabled due to params empty");
        return false;
    }

    MessageParcel data;
    if (!data.WriteInterfaceToken(GetDescriptor())) {
        APP_LOGE("fail to SetAbilityEnabled due to write InterfaceToken fail");
        return false;
    }
    if (!data.WriteParcelable(&abilityInfo)) {
        APP_LOGE("fail to SetAbilityEnabled due to write abilityInfo fail");
        return false;
    }
    if (!data.WriteBool(isEnabled)) {
        APP_LOGE("fail to SetAbilityEnabled due to write isEnabled fail");
        return false;
    }
    if (!data.WriteInt32(userId)) {
        APP_LOGE("fail to SetAbilityEnabled due to write userId fail");
        return false;
    }

    MessageParcel reply;
    if (!SendTransactCmd(IBundleMgr::Message::SET_ABILITY_ENABLED, data, reply)) {
        APP_LOGE("fail to SetAbilityEnabled from server");
        return false;
    }
    return reply.ReadBool();
}

bool BundleMgrProxy::GetAbilityInfo(
    const std::string &bundleName, const std::string &abilityName, AbilityInfo &abilityInfo)
{
    BYTRACE_NAME(BYTRACE_TAG_APP, __PRETTY_FUNCTION__);
    APP_LOGD("bundleName :%{public}s, abilityName :%{public}s", bundleName.c_str(), abilityName.c_str());
    if (bundleName.empty() || abilityName.empty()) {
        APP_LOGE("fail to GetAbilityInfo due to params empty");
        return false;
    }

    MessageParcel data;
    if (!data.WriteInterfaceToken(GetDescriptor())) {
        APP_LOGE("fail to GetAbilityInfo due to write MessageParcel fail");
        return false;
    }
    if (!data.WriteString(bundleName)) {
        APP_LOGE("fail to GetAbilityInfo due to write bundleName fail");
        return false;
    }
    if (!data.WriteString(abilityName)) {
        APP_LOGE("fail to GetAbilityInfo due to write flag fail");
        return false;
    }

    if (!GetParcelableInfo<AbilityInfo>(IBundleMgr::Message::GET_ABILITY_INFO, data, abilityInfo)) {
        APP_LOGE("fail to GetAbilityInfo from server");
        return false;
    }
    return true;
}

std::string BundleMgrProxy::GetAbilityIcon(const std::string &bundleName, const std::string &className)
{
    BYTRACE_NAME(BYTRACE_TAG_APP, __PRETTY_FUNCTION__);
    APP_LOGI("begin to get bundle info of %{public}s", bundleName.c_str());
    if (bundleName.empty()) {
        APP_LOGE("fail to GetAbilityIcon due to params empty");
        return Constants::EMPTY_STRING;
    }

    MessageParcel data;
    if (!data.WriteInterfaceToken(GetDescriptor())) {
        APP_LOGE("fail to GetAbilityIcon due to write InterfaceToken fail");
        return Constants::EMPTY_STRING;
    }
    if (!data.WriteString(bundleName)) {
        APP_LOGE("fail to GetAbilityIcon due to write bundleName fail");
        return Constants::EMPTY_STRING;
    }
    if (!data.WriteString(className)) {
        APP_LOGE("fail to GetAbilityIcon due to write className fail");
        return Constants::EMPTY_STRING;
    }

    MessageParcel reply;
    if (!SendTransactCmd(IBundleMgr::Message::GET_ABILITY_ICON, data, reply)) {
        APP_LOGE("fail to GetAbilityIcon from server");
        return Constants::EMPTY_STRING;
    }
    return reply.ReadString();
}

#ifdef SUPPORT_GRAPHICS
std::shared_ptr<Media::PixelMap> BundleMgrProxy::GetAbilityPixelMapIcon(const std::string &bundleName,
    const std::string &abilityName)
{
    BYTRACE_NAME(BYTRACE_TAG_APP, __PRETTY_FUNCTION__);
    APP_LOGI("begin to get ability pixelmap icon of %{public}s, %{public}s", bundleName.c_str(), abilityName.c_str());
    if (bundleName.empty() || abilityName.empty()) {
        APP_LOGE("fail to GetAbilityPixelMapIcon due to params empty");
        return nullptr;
    }

    MessageParcel data;
    if (!data.WriteInterfaceToken(GetDescriptor())) {
        APP_LOGE("fail to GetAbilityPixelMapIcon due to write InterfaceToken fail");
        return nullptr;
    }
    if (!data.WriteString(bundleName)) {
        APP_LOGE("fail to GetAbilityPixelMapIcon due to write bundleName fail");
        return nullptr;
    }
    if (!data.WriteString(abilityName)) {
        APP_LOGE("fail to GetAbilityPixelMapIcon due to write abilityName fail");
        return nullptr;
    }
    MessageParcel reply;
    if (!SendTransactCmd(IBundleMgr::Message::GET_ABILITY_PIXELMAP_ICON, data, reply)) {
        APP_LOGE("SendTransactCmd result false");
        return nullptr;
    }
    if (!reply.ReadBool()) {
        APP_LOGE("reply result false");
        return nullptr;
    }
    std::shared_ptr<Media::PixelMap> info(reply.ReadParcelable<Media::PixelMap>());
    if (!info) {
        APP_LOGE("readParcelableInfo failed");
        return nullptr;
    }
    APP_LOGD("get ability pixelmap icon success");
    return info;
}
#endif

sptr<IBundleInstaller> BundleMgrProxy::GetBundleInstaller()
{
    BYTRACE_NAME(BYTRACE_TAG_APP, __PRETTY_FUNCTION__);
    APP_LOGD("begin to get bundle installer");
    MessageParcel data;
    MessageParcel reply;
    if (!data.WriteInterfaceToken(GetDescriptor())) {
        APP_LOGE("fail to GetBundleInstaller due to write InterfaceToken fail");
        return nullptr;
    }
    if (!SendTransactCmd(IBundleMgr::Message::GET_BUNDLE_INSTALLER, data, reply)) {
        return nullptr;
    }

    sptr<IRemoteObject> object = reply.ReadParcelable<IRemoteObject>();
    sptr<IBundleInstaller> installer = iface_cast<IBundleInstaller>(object);
    if (!installer) {
        APP_LOGE("bundle installer is nullptr");
    }

    APP_LOGD("get bundle installer success");
    return installer;
}

sptr<IBundleUserMgr> BundleMgrProxy::GetBundleUserMgr()
{
    BYTRACE_NAME(BYTRACE_TAG_APP, __PRETTY_FUNCTION__);
    MessageParcel data;
    MessageParcel reply;
    if (!data.WriteInterfaceToken(GetDescriptor())) {
        APP_LOGE("fail to get bundle user mgr due to write InterfaceToken fail");
        return nullptr;
    }
    if (!SendTransactCmd(IBundleMgr::Message::GET_BUNDLE_USER_MGR, data, reply)) {
        return nullptr;
    }

    sptr<IRemoteObject> object = reply.ReadParcelable<IRemoteObject>();
    sptr<IBundleUserMgr> bundleUserMgr = iface_cast<IBundleUserMgr>(object);
    if (!bundleUserMgr) {
        APP_LOGE("bundleUserMgr is nullptr");
    }

    return bundleUserMgr;
}

bool BundleMgrProxy::CanRequestPermission(
    const std::string &bundleName, const std::string &permissionName, const int userId)
{
    BYTRACE_NAME(BYTRACE_TAG_APP, __PRETTY_FUNCTION__);
    APP_LOGI("begin to CanRequestPermission of %{public}s and %{public}s", bundleName.c_str(), permissionName.c_str());
    if (bundleName.empty() || permissionName.empty()) {
        APP_LOGE("fail to CanRequestPermission due to params empty");
        return false;
    }

    MessageParcel data;
    if (!data.WriteInterfaceToken(GetDescriptor())) {
        APP_LOGE("fail to CanRequestPermission due to write InterfaceToken fail");
        return false;
    }
    if (!data.WriteString(bundleName)) {
        APP_LOGE("fail to CanRequestPermission due to write bundleName fail");
        return false;
    }
    if (!data.WriteString(permissionName)) {
        APP_LOGE("fail to CanRequestPermission due to write permission fail");
        return false;
    }
    if (!data.WriteInt32(userId)) {
        APP_LOGE("fail to CanRequestPermission due to write userId fail");
        return false;
    }

    MessageParcel reply;
    if (!SendTransactCmd(IBundleMgr::Message::CAN_REQUEST_PERMISSION, data, reply)) {
        APP_LOGE("fail to CanRequestPermission from server");
        return false;
    }
    return reply.ReadBool();
}

bool BundleMgrProxy::RequestPermissionFromUser(
    const std::string &bundleName, const std::string &permission, const int userId)
{
    BYTRACE_NAME(BYTRACE_TAG_APP, __PRETTY_FUNCTION__);
    APP_LOGI("begin to RequestPermissionsFromUser of %{public}s", bundleName.c_str());
    if (bundleName.empty() || permission.empty()) {
        APP_LOGE("fail to RequestPermissionsFromUser due to params empty");
        return false;
    }

    MessageParcel data;
    if (!data.WriteInterfaceToken(GetDescriptor())) {
        APP_LOGE("fail to RequestPermissionsFromUser due to write InterfaceToken fail");
        return false;
    }
    if (!data.WriteString(bundleName)) {
        APP_LOGE("fail to RequestPermissionsFromUser due to write bundleName fail");
        return false;
    }
    if (!data.WriteString(permission)) {
        APP_LOGE("fail to RequestPermissionsFromUser due to write permission fail");
        return false;
    }
    if (!data.WriteInt32(userId)) {
        APP_LOGE("fail to RequestPermissionsFromUser due to write userId fail");
        return false;
    }

    MessageParcel reply;
    if (!SendTransactCmd(IBundleMgr::Message::REQUEST_PERMISSION_FROM_USER, data, reply)) {
        APP_LOGE("fail to RequestPermissionsFromUser from server");
        return false;
    }
    return reply.ReadBool();
}

bool BundleMgrProxy::RegisterAllPermissionsChanged(const sptr<OnPermissionChangedCallback> &callback)
{
    BYTRACE_NAME(BYTRACE_TAG_APP, __PRETTY_FUNCTION__);
    APP_LOGI("begin to RegisterAllPermissionsChanged");
    if (!callback) {
        APP_LOGE("fail to RegisterAllPermissionsChanged");
        return false;
    }

    MessageParcel data;
    if (!data.WriteInterfaceToken(GetDescriptor())) {
        APP_LOGE("fail to RegisterAllPermissionsChanged due to write InterfaceToken fail");
        return false;
    }
    if (!data.WriteParcelable(callback->AsObject())) {
        APP_LOGE("fail to RegisterAllPermissionsChanged, for write parcel failed");
        return false;
    }

    MessageParcel reply;
    if (!SendTransactCmd(IBundleMgr::Message::REGISTER_ALL_PERMISSIONS_CHANGED, data, reply)) {
        APP_LOGE("fail to RegisterAllPermissionsChanged from server");
        return false;
    }
    return reply.ReadBool();
}

bool BundleMgrProxy::RegisterPermissionsChanged(
    const std::vector<int> &uids, const sptr<OnPermissionChangedCallback> &callback)
{
    BYTRACE_NAME(BYTRACE_TAG_APP, __PRETTY_FUNCTION__);
    APP_LOGI("begin to RegisterAllPermissionsChanged");
    if (!callback || uids.empty()) {
        APP_LOGE("fail to RegisterAllPermissionsChanged");
        return false;
    }

    MessageParcel data;
    if (!data.WriteInterfaceToken(GetDescriptor())) {
        APP_LOGE("fail to RegisterAllPermissionsChanged due to write InterfaceToken fail");
        return false;
    }
    if (!data.WriteInt32Vector(uids)) {
        APP_LOGE("fail to RegisterAllPermissionsChanged due to write permissions fail");
        return false;
    }
    if (!data.WriteParcelable(callback->AsObject())) {
        APP_LOGE("fail to RegisterAllPermissionsChanged, for write parcel failed");
        return false;
    }

    MessageParcel reply;
    if (!SendTransactCmd(IBundleMgr::Message::REGISTER_PERMISSIONS_CHANGED, data, reply)) {
        APP_LOGE("fail to RegisterAllPermissionsChanged from server");
        return false;
    }
    return reply.ReadBool();
}

bool BundleMgrProxy::UnregisterPermissionsChanged(const sptr<OnPermissionChangedCallback> &callback)
{
    BYTRACE_NAME(BYTRACE_TAG_APP, __PRETTY_FUNCTION__);
    APP_LOGI("begin to UnregisterPermissionsChanged");
    if (!callback) {
        APP_LOGE("fail to UnregisterPermissionsChanged, for callback is nullptr");
        return false;
    }

    MessageParcel data;
    if (!data.WriteInterfaceToken(GetDescriptor())) {
        APP_LOGE("fail to UnregisterPermissionsChanged due to write InterfaceToken fail");
        return false;
    }
    if (!data.WriteParcelable(callback->AsObject())) {
        APP_LOGE("fail to UnregisterPermissionsChanged, for write parcel failed");
        return false;
    }

    MessageParcel reply;
    if (!SendTransactCmd(IBundleMgr::Message::UNREGISTER_PERMISSIONS_CHANGED, data, reply)) {
        APP_LOGE("fail to UnregisterPermissionsChanged from server");
        return false;
    }
    return reply.ReadBool();
}

bool BundleMgrProxy::GetAllFormsInfo(std::vector<FormInfo> &formInfos)
{
    BYTRACE_NAME(BYTRACE_TAG_APP, __PRETTY_FUNCTION__);
    MessageParcel data;
    if (!data.WriteInterfaceToken(GetDescriptor())) {
        APP_LOGE("fail to GetAllFormsInfo due to write MessageParcel fail");
        return false;
    }

    if (!GetParcelableInfos<FormInfo>(IBundleMgr::Message::GET_ALL_FORMS_INFO, data, formInfos)) {
        APP_LOGE("fail to GetAllFormsInfo from server");
        return false;
    }
    return true;
}

bool BundleMgrProxy::GetFormsInfoByApp(const std::string &bundleName, std::vector<FormInfo> &formInfos)
{
    BYTRACE_NAME(BYTRACE_TAG_APP, __PRETTY_FUNCTION__);
    if (bundleName.empty()) {
        APP_LOGE("fail to GetFormsInfoByApp due to params empty");
        return false;
    }

    MessageParcel data;
    if (!data.WriteInterfaceToken(GetDescriptor())) {
        APP_LOGE("fail to GetFormsInfoByApp due to write MessageParcel fail");
        return false;
    }
    if (!data.WriteString(bundleName)) {
        APP_LOGE("fail to GetFormsInfoByApp due to write bundleName fail");
        return false;
    }
    if (!GetParcelableInfos<FormInfo>(IBundleMgr::Message::GET_FORMS_INFO_BY_APP, data, formInfos)) {
        APP_LOGE("fail to GetFormsInfoByApp from server");
        return false;
    }
    return true;
}

bool BundleMgrProxy::GetFormsInfoByModule(
    const std::string &bundleName, const std::string &moduleName, std::vector<FormInfo> &formInfos)
{
    BYTRACE_NAME(BYTRACE_TAG_APP, __PRETTY_FUNCTION__);
    if (bundleName.empty() || moduleName.empty()) {
        APP_LOGE("fail to GetFormsByModule due to params empty");
        return false;
    }

    MessageParcel data;
    if (!data.WriteInterfaceToken(GetDescriptor())) {
        APP_LOGE("fail to GetFormsInfoByModule due to write MessageParcel fail");
        return false;
    }

    if (!data.WriteString(bundleName)) {
        APP_LOGE("fail to GetFormsInfoByModule due to write bundleName fail");
        return false;
    }

    if (!data.WriteString(moduleName)) {
        APP_LOGE("fail to GetFormsInfoByModule due to write moduleName fail");
        return false;
    }

    if (!GetParcelableInfos<FormInfo>(IBundleMgr::Message::GET_FORMS_INFO_BY_MODULE, data, formInfos)) {
        APP_LOGE("fail to GetFormsInfoByModule from server");
        return false;
    }
    return true;
}

bool BundleMgrProxy::GetShortcutInfos(const std::string &bundleName, std::vector<ShortcutInfo> &shortcutInfos)
{
    BYTRACE_NAME(BYTRACE_TAG_APP, __PRETTY_FUNCTION__);
    if (bundleName.empty()) {
        APP_LOGE("fail to GetShortcutInfos due to params empty");
        return false;
    }

    MessageParcel data;
    if (!data.WriteInterfaceToken(GetDescriptor())) {
        APP_LOGE("fail to GetShortcutInfos due to write MessageParcel fail");
        return false;
    }

    if (!data.WriteString(bundleName)) {
        APP_LOGE("fail to GetShortcutInfos due to write bundleName fail");
        return false;
    }

    if (!GetParcelableInfos<ShortcutInfo>(IBundleMgr::Message::GET_SHORTCUT_INFO, data, shortcutInfos)) {
        APP_LOGE("fail to GetShortcutInfos from server");
        return false;
    }
    return true;
}

bool BundleMgrProxy::GetAllCommonEventInfo(const std::string &eventKey, std::vector<CommonEventInfo> &commonEventInfos)
{
    BYTRACE_NAME(BYTRACE_TAG_APP, __PRETTY_FUNCTION__);
    if (eventKey.empty()) {
        APP_LOGE("fail to GetAllCommonEventInfo due to eventKey empty");
        return false;
    }

    MessageParcel data;
    if (!data.WriteInterfaceToken(GetDescriptor())) {
        APP_LOGE("fail to GetAllCommonEventInfo due to write MessageParcel fail");
        return false;
    }

    if (!data.WriteString(eventKey)) {
        APP_LOGE("fail to GetAllCommonEventInfo due to write eventKey fail");
        return false;
    }

    if (!GetParcelableInfos<CommonEventInfo>(IBundleMgr::Message::GET_ALL_COMMON_EVENT_INFO, data, commonEventInfos)) {
        APP_LOGE("fail to GetAllCommonEventInfo from server");
        return false;
    }
    return true;
}

bool BundleMgrProxy::GetModuleUsageRecords(const int32_t number, std::vector<ModuleUsageRecord> &moduleUsageRecords)
{
    BYTRACE_NAME(BYTRACE_TAG_APP, __PRETTY_FUNCTION__);
    MessageParcel data;
    if (!data.WriteInterfaceToken(GetDescriptor())) {
        APP_LOGE("fail to GetModuleUsageRecords due to write MessageParcel fail");
        return false;
    }

    if (!data.WriteInt32(number)) {
        APP_LOGE("fail to GetModuleUsageRecords due to write number fail");
        return false;
    }

    if (!GetParcelableInfos<ModuleUsageRecord>(
            IBundleMgr::Message::GET_MODULE_USAGE_RECORD, data, moduleUsageRecords)) {
        APP_LOGE("fail to GetModuleUsageRecords from server");
        return false;
    }
    return true;
}

bool BundleMgrProxy::NotifyAbilityLifeStatus(
    const std::string &bundleName, const std::string &abilityName, const int64_t launchTime, const int uid)
{
    BYTRACE_NAME(BYTRACE_TAG_APP, __PRETTY_FUNCTION__);
    APP_LOGI("begin to NotifyAbilityLifeStatus of %{public}s", abilityName.c_str());
    if (bundleName.empty() || abilityName.empty()) {
        APP_LOGE("fail to NotifyAbilityLifeStatus due to params empty");
        return false;
    }

    MessageParcel data;
    if (!data.WriteInterfaceToken(GetDescriptor())) {
        APP_LOGE("fail to NotifyAbilityLifeStatus due to write InterfaceToken fail");
        return false;
    }
    if (!data.WriteString(bundleName)) {
        APP_LOGE("fail to NotifyAbilityLifeStatus due to write bundleName fail");
        return false;
    }
    if (!data.WriteString(abilityName)) {
        APP_LOGE("fail to NotifyAbilityLifeStatus due to write abilityName fail");
        return false;
    }
    if (!data.WriteInt64(launchTime)) {
        APP_LOGE("fail to NotifyAbilityLifeStatus due to write launchTime fail");
        return false;
    }
    if (!data.WriteInt32(uid)) {
        APP_LOGE("fail to NotifyAbilityLifeStatus due to write uid fail");
        return false;
    }

    MessageParcel reply;
    if (!SendTransactCmd(IBundleMgr::Message::NOTIFY_ABILITY_LIFE_STATUS, data, reply)) {
        APP_LOGE("fail to NotifyAbilityLifeStatus from server");
        return false;
    }
    return reply.ReadBool();
}

bool BundleMgrProxy::CheckBundleNameInAllowList(const std::string &bundleName)
{
    BYTRACE_NAME(BYTRACE_TAG_APP, __PRETTY_FUNCTION__);
    APP_LOGI("begin to Check BundleName In AllowList");
    if (bundleName.empty()) {
        APP_LOGE("fail to Check BundleName In AllowList due to params empty");
        return false;
    }
    MessageParcel data;
    if (!data.WriteInterfaceToken(GetDescriptor())) {
        APP_LOGE("fail to Check BundleName In AllowList due to write MessageParcel fail");
        return false;
    }
    if (!data.WriteString(bundleName)) {
        APP_LOGE("fail to Check BundleName In AllowList due to write bundleName fail");
        return false;
    }
    MessageParcel reply;
    if (!SendTransactCmd(IBundleMgr::Message::BUNDLE_CLONE, data, reply)) {
        APP_LOGE("fail to Check BundleName In AllowList from server");
        return false;
    }
    return reply.ReadBool();
}

bool BundleMgrProxy::BundleClone(const std::string &bundleName)
{
    BYTRACE_NAME(BYTRACE_TAG_APP, __PRETTY_FUNCTION__);
    APP_LOGI("begin to bundle clone");
    if (bundleName.empty()) {
        APP_LOGE("fail to bundle clone due to params empty");
        return false;
    }
    MessageParcel data;
    if (!data.WriteInterfaceToken(GetDescriptor())) {
        APP_LOGE("fail to bundle clone due to write MessageParcel fail");
        return false;
    }
    if (!data.WriteString(bundleName)) {
        APP_LOGE("fail to bundle clone due to write bundleName fail");
        return false;
    }
    MessageParcel reply;
    if (!SendTransactCmd(IBundleMgr::Message::BUNDLE_CLONE, data, reply)) {
        APP_LOGE("fail to bundle clone from server");
        return false;
    }
    return reply.ReadBool();
}

bool BundleMgrProxy::RemoveClonedBundle(const std::string &bundleName, const int32_t uid)
{
    BYTRACE_NAME(BYTRACE_TAG_APP, __PRETTY_FUNCTION__);
    APP_LOGI("begin to remove cloned bundle");
    if (bundleName.empty()) {
        APP_LOGE("fail to remove cloned bundle due to params empty");
        return false;
    }
    MessageParcel data;
    if (!data.WriteInterfaceToken(GetDescriptor())) {
        APP_LOGE("fail to remove cloned bundle due to write MessageParcel fail");
        return false;
    }
    if (!data.WriteString(bundleName)) {
        APP_LOGE("fail to remove cloned bundle due to write bundleName fail");
        return false;
    }
    if (!data.WriteInt32(uid)) {
        APP_LOGE("fail to  remove cloned bundle due to write uid fail");
        return false;
    }
    MessageParcel reply;
    if (!SendTransactCmd(IBundleMgr::Message::REMOVE_CLONED_BUNDLE, data, reply)) {
        APP_LOGE("fail to remove cloned bundle from server");
        return false;
    }
    return reply.ReadBool();
}

bool BundleMgrProxy::GetDistributedBundleInfo(
    const std::string &networkId, int32_t userId, const std::string &bundleName,
    DistributedBundleInfo &distributedBundleInfo)
{
    APP_LOGI("begin to GetDistributedBundleInfo of %{public}s", bundleName.c_str());
    if (networkId.empty() || bundleName.empty()) {
        APP_LOGE("fail to GetDistributedBundleInfo due to params empty");
        return false;
    }
    MessageParcel data;
    if (!data.WriteInterfaceToken(GetDescriptor())) {
        APP_LOGE("fail to GetDistributedBundleInfo due to write MessageParcel fail");
        return false;
    }

    if (!data.WriteString(networkId)) {
        APP_LOGE("fail to GetDistributedBundleInfo due to write networkId fail");
        return false;
    }

    if (!data.WriteInt32(userId)) {
        APP_LOGE("fail to GetDistributedBundleInfo due to write userId fail");
        return false;
    }

    if (!data.WriteString(bundleName)) {
        APP_LOGE("fail to GetDistributedBundleInfo due to write bundleName fail");
        return false;
    }
    MessageParcel reply;
    if (!GetParcelableInfo<DistributedBundleInfo>(
            IBundleMgr::Message::GET_DISTRIBUTE_BUNDLE_INFO, data, distributedBundleInfo)) {
        APP_LOGE("fail to GetDistributedBundleInfo from server");
        return false;
    }
    return true;
}

std::string BundleMgrProxy::GetAppPrivilegeLevel(const std::string &bundleName, int32_t userId)
{
    APP_LOGD("begin to GetAppPrivilegeLevel of %{public}s", bundleName.c_str());
    if (bundleName.empty()) {
        APP_LOGE("fail to GetAppPrivilegeLevel due to params empty");
        return Constants::EMPTY_STRING;
    }
    MessageParcel data;
    if (!data.WriteInterfaceToken(GetDescriptor())) {
        APP_LOGE("fail to GetAppPrivilegeLevel due to write InterfaceToken fail");
        return Constants::EMPTY_STRING;
    }
    if (!data.WriteString(bundleName)) {
        APP_LOGE("fail to GetAppPrivilegeLevel due to write bundleName fail");
        return Constants::EMPTY_STRING;
    }
    if (!data.WriteInt32(userId)) {
        APP_LOGE("fail to GetAppPrivilegeLevel due to write userId fail");
        return Constants::EMPTY_STRING;
    }

    MessageParcel reply;
    if (!SendTransactCmd(IBundleMgr::Message::GET_APPLICATION_PRIVILEGE_LEVEL, data, reply)) {
        APP_LOGE("fail to GetAppPrivilegeLevel from server");
        return Constants::EMPTY_STRING;
    }
    return reply.ReadString();
}

bool BundleMgrProxy::QueryExtensionAbilityInfos(const Want &want, const int32_t &flag, const int32_t &userId,
    std::vector<ExtensionAbilityInfo> &extensionInfos)
{
    MessageParcel data;
    if (!data.WriteInterfaceToken(GetDescriptor())) {
        APP_LOGE("fail to QueryExtensionAbilityInfos due to write InterfaceToken fail");
        return false;
    }
    if (!data.WriteParcelable(&want)) {
        APP_LOGE("fail to QueryExtensionAbilityInfos due to write want fail");
        return false;
    }
    if (!data.WriteInt32(flag)) {
        APP_LOGE("fail to QueryExtensionAbilityInfos due to write flag fail");
        return false;
    }
    if (!data.WriteInt32(userId)) {
        APP_LOGE("fail to QueryExtensionAbilityInfos due to write userId fail");
        return false;
    }

    if (!GetParcelableInfos(IBundleMgr::Message::QUERY_EXTENSION_INFO_WITHOUT_TYPE, data, extensionInfos)) {
        APP_LOGE("fail to obtain extensionInfos");
        return false;
    }
    return true;
}

bool BundleMgrProxy::QueryExtensionAbilityInfos(const Want &want, const ExtensionAbilityType &extensionType,
    const int32_t &flag, const int32_t &userId, std::vector<ExtensionAbilityInfo> &extensionInfos)
{
    MessageParcel data;
    if (!data.WriteInterfaceToken(GetDescriptor())) {
        APP_LOGE("fail to QueryExtensionAbilityInfos due to write InterfaceToken fail");
        return false;
    }
    if (!data.WriteParcelable(&want)) {
        APP_LOGE("fail to QueryExtensionAbilityInfos due to write want fail");
        return false;
    }
    if (!data.WriteInt32(static_cast<int32_t>(extensionType))) {
        APP_LOGE("fail to QueryExtensionAbilityInfos due to write type fail");
        return false;
    }
    if (!data.WriteInt32(flag)) {
        APP_LOGE("fail to QueryExtensionAbilityInfos due to write flag fail");
        return false;
    }
    if (!data.WriteInt32(userId)) {
        APP_LOGE("fail to QueryExtensionAbilityInfos due to write userId fail");
        return false;
    }

    if (!GetParcelableInfos(IBundleMgr::Message::QUERY_EXTENSION_INFO, data, extensionInfos)) {
        APP_LOGE("fail to obtain extensionInfos");
        return false;
    }
    return true;
}

bool BundleMgrProxy::QueryExtensionAbilityInfos(const ExtensionAbilityType &extensionType, const int32_t &userId,
    std::vector<ExtensionAbilityInfo> &extensionInfos)
{
    MessageParcel data;
    if (!data.WriteInterfaceToken(GetDescriptor())) {
        APP_LOGE("fail to QueryExtensionAbilityInfos due to write InterfaceToken fail");
        return false;
    }
    if (!data.WriteInt32(static_cast<int32_t>(extensionType))) {
        APP_LOGE("fail to QueryExtensionAbilityInfos due to write type fail");
        return false;
    }
    if (!data.WriteInt32(userId)) {
        APP_LOGE("fail to QueryExtensionAbilityInfos due to write userId fail");
        return false;
    }

    if (!GetParcelableInfos(IBundleMgr::Message::QUERY_EXTENSION_INFO_BY_TYPE, data, extensionInfos)) {
        APP_LOGE("fail to obtain extensionInfos");
        return false;
    }
    return true;
}

bool BundleMgrProxy::VerifyCallingPermission(const std::string &permission)
{
    APP_LOGD("VerifyCallingPermission begin");
    MessageParcel data;
    if (!data.WriteInterfaceToken(GetDescriptor())) {
        APP_LOGE("fail to VerifyCallingPermission due to write InterfaceToken fail");
        return false;
    }

    if (!data.WriteString(permission)) {
        APP_LOGE("fail to VerifyCallingPermission due to write bundleName fail");
        return false;
    }

    MessageParcel reply;
    if (!SendTransactCmd(IBundleMgr::Message::VERIFY_CALLING_PERMISSION, data, reply)) {
        APP_LOGE("fail to sendRequest");
        return false;
    }
    return reply.ReadBool();
}

std::vector<std::string> BundleMgrProxy::GetAccessibleAppCodePaths(int32_t userId)
{
    APP_LOGD("GetAccessibleAppCodePaths begin");
    MessageParcel data;
    std::vector<std::string> vec;
    if (!data.WriteInterfaceToken(GetDescriptor())) {
        APP_LOGE("fail to GetAccessibleAppCodePaths due to write InterfaceToken fail");
        return vec;
    }

    if (!data.WriteInt32(userId)) {
        APP_LOGE("fail to GetAccessibleAppCodePaths due to write userId fail");
        return vec;
    }

    MessageParcel reply;
    if (!SendTransactCmd(IBundleMgr::Message::GET_ACCESSIBLE_APP_CODE_PATH, data, reply)) {
        APP_LOGE("fail to sendRequest");
        return vec;
    }

    if (!reply.ReadBool()) {
        APP_LOGE("reply result false");
        return vec;
    }

    if (!reply.ReadStringVector(&vec)) {
        APP_LOGE("fail to GetAccessibleAppCodePaths from reply");
        return vec;
    }
    return vec;
}

bool BundleMgrProxy::QueryExtensionAbilityInfoByUri(const std::string &uri, int32_t userId,
    ExtensionAbilityInfo &extensionAbilityInfo)
{
    APP_LOGD("begin to QueryExtensionAbilityInfoByUri");
    BYTRACE_NAME(BYTRACE_TAG_APP, __PRETTY_FUNCTION__);
    if (uri.empty()) {
        APP_LOGE("uri is empty");
        return false;
    }
    MessageParcel data;
    if (!data.WriteInterfaceToken(GetDescriptor())) {
        APP_LOGE("failed to QueryExtensionAbilityInfoByUri due to write MessageParcel fail");
        return false;
    }
    if (!data.WriteString(uri)) {
        APP_LOGE("failed to QueryExtensionAbilityInfoByUri due to write uri fail");
        return false;
    }
    if (!data.WriteInt32(userId)) {
        APP_LOGE("failed to QueryExtensionAbilityInfoByUri due to write userId fail");
        return false;
    }

    if (!GetParcelableInfo<ExtensionAbilityInfo>(
        IBundleMgr::Message::QUERY_EXTENSION_ABILITY_INFO_BY_URI, data, extensionAbilityInfo)) {
        APP_LOGE("failed to QueryExtensionAbilityInfoByUri from server");
        return false;
    }
    return true;
}

template<typename T>
bool BundleMgrProxy::GetParcelableInfo(IBundleMgr::Message code, MessageParcel &data, T &parcelableInfo)
{
    MessageParcel reply;
    if (!SendTransactCmd(code, data, reply)) {
        return false;
    }

    if (!reply.ReadBool()) {
        APP_LOGE("reply result false");
        return false;
    }

    std::unique_ptr<T> info(reply.ReadParcelable<T>());
    if (!info) {
        APP_LOGE("readParcelableInfo failed");
        return false;
    }
    parcelableInfo = *info;
    APP_LOGD("get parcelable info success");
    return true;
}

template<typename T>
bool BundleMgrProxy::GetParcelableInfos(IBundleMgr::Message code, MessageParcel &data, std::vector<T> &parcelableInfos)
{
    MessageParcel reply;
    if (!SendTransactCmd(code, data, reply)) {
        return false;
    }

    if (!reply.ReadBool()) {
        APP_LOGE("readParcelableInfo failed");
        return false;
    }

    int32_t infoSize = reply.ReadInt32();
    for (int32_t i = 0; i < infoSize; i++) {
        std::unique_ptr<T> info(reply.ReadParcelable<T>());
        if (!info) {
            APP_LOGE("Read Parcelable infos failed");
            return false;
        }
        parcelableInfos.emplace_back(*info);
    }
    APP_LOGD("get parcelable infos success");
    return true;
}

template <typename T>
bool BundleMgrProxy::GetParcelableInfosFromAshmem(
    IBundleMgr::Message code, MessageParcel &data, std::vector<T> &parcelableInfos)
{
    APP_LOGD("Get parcelable vector from ashmem");
    MessageParcel reply;
    if (!SendTransactCmd(code, data, reply)) {
        return false;
    }

    if (!reply.ReadBool()) {
        APP_LOGE("ReadParcelableInfo failed");
        return false;
    }

    int32_t infoSize = reply.ReadInt32();
    sptr<Ashmem> ashmem = reply.ReadAshmem();
    if (ashmem == nullptr) {
        APP_LOGE("Ashmem is nullptr");
        return false;
    }

    bool ret = ashmem->MapReadOnlyAshmem();
    if (!ret) {
        APP_LOGE("Map read only ashmem fail");
        ClearAshMem(ashmem);
        return false;
    }

    int32_t offset = 0;
    const char* dataStr = static_cast<const char*>(
        ashmem->ReadFromAshmem(ashmem->GetAshmemSize(), offset));
    if (dataStr == nullptr) {
        APP_LOGE("Data is nullptr when read from ashmem");
        ClearAshMem(ashmem);
        return false;
    }

    while (infoSize > 0) {
        std::string lenStr;
        if (!ParseStr(dataStr, ASHMEM_LEN, offset, lenStr)) {
            APP_LOGE("Parse lenStr fail");
            ClearAshMem(ashmem);
            return false;
        }

        int strLen = atoi(lenStr.c_str());
        offset += ASHMEM_LEN;
        std::string infoStr;
        if (!ParseStr(dataStr, strLen, offset, infoStr)) {
            APP_LOGE("Parse infoStr fail");
            ClearAshMem(ashmem);
            return false;
        }

        T info;
        if (!ParseInfoFromJsonStr(infoStr.c_str(), info)) {
            APP_LOGE("Parse info from json fail");
            ClearAshMem(ashmem);
            return false;
        }

        parcelableInfos.emplace_back(info);
        infoSize--;
        offset += strLen;
    }

    APP_LOGD("Get parcelable vector from ashmem success");
    return true;
}

bool BundleMgrProxy::SendTransactCmd(IBundleMgr::Message code, MessageParcel &data, MessageParcel &reply)
{
    MessageOption option(MessageOption::TF_SYNC);

    sptr<IRemoteObject> remote = Remote();
    if (!remote) {
        APP_LOGE("fail to send transact cmd %{public}d due to remote object", code);
        return false;
    }
    int32_t result = remote->SendRequest(static_cast<uint32_t>(code), data, reply, option);
    if (result != NO_ERROR) {
        APP_LOGE("receive error transact code %{public}d in transact cmd %{public}d", result, code);
        return false;
    }
    return true;
}
}  // namespace AppExecFwk
}  // namespace OHOS