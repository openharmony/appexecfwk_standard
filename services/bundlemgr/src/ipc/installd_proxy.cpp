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

#include "ipc/installd_proxy.h"

#include "ipc_types.h"

#include "app_log_wrapper.h"
#include "bundle_constants.h"
#include "parcel_macro.h"
#include "string_ex.h"

namespace OHOS {
namespace AppExecFwk {

namespace {
constexpr int32_t WAIT_TIME = 3000;
}

InstalldProxy::InstalldProxy(const sptr<IRemoteObject> &object) : IRemoteProxy<IInstalld>(object)
{
    APP_LOGI("installd proxy instance is created");
}

InstalldProxy::~InstalldProxy()
{
    APP_LOGI("installd proxy instance is destroyed");
}

ErrCode InstalldProxy::CreateBundleDir(const std::string &bundleDir)
{
    MessageParcel data;
    INSTALLD_PARCEL_WRITE_INTERFACE_TOKEN(data, (GetDescriptor()));
    INSTALLD_PARCEL_WRITE(data, String16, Str8ToStr16(bundleDir));

    MessageParcel reply;
    MessageOption option;
    return TransactInstalldCmd(IInstalld::Message::CREATE_BUNDLE_DIR, data, reply, option);
}

ErrCode InstalldProxy::ExtractModuleFiles(const std::string &srcModulePath, const std::string &targetPath)
{
    MessageParcel data;
    INSTALLD_PARCEL_WRITE_INTERFACE_TOKEN(data, (GetDescriptor()));
    INSTALLD_PARCEL_WRITE(data, String16, Str8ToStr16(srcModulePath));
    INSTALLD_PARCEL_WRITE(data, String16, Str8ToStr16(targetPath));

    MessageParcel reply;
    MessageOption option;
    return TransactInstalldCmd(IInstalld::Message::EXTRACT_MODULE_FILES, data, reply, option);
}

ErrCode InstalldProxy::RenameModuleDir(const std::string &oldPath, const std::string &newPath)
{
    MessageParcel data;
    INSTALLD_PARCEL_WRITE_INTERFACE_TOKEN(data, (GetDescriptor()));
    INSTALLD_PARCEL_WRITE(data, String16, Str8ToStr16(oldPath));
    INSTALLD_PARCEL_WRITE(data, String16, Str8ToStr16(newPath));

    MessageParcel reply;
    MessageOption option;
    return TransactInstalldCmd(IInstalld::Message::RENAME_MODULE_DIR, data, reply, option);
}

ErrCode InstalldProxy::CreateBundleDataDir(const std::string &bundleDir, const int uid, const int gid)
{
    MessageParcel data;
    INSTALLD_PARCEL_WRITE_INTERFACE_TOKEN(data, (GetDescriptor()));
    INSTALLD_PARCEL_WRITE(data, String16, Str8ToStr16(bundleDir));
    INSTALLD_PARCEL_WRITE(data, Int32, uid);
    INSTALLD_PARCEL_WRITE(data, Int32, gid);

    MessageParcel reply;
    MessageOption option;
    return TransactInstalldCmd(IInstalld::Message::CREATE_BUNDLE_DATA_DIR, data, reply, option);
}

ErrCode InstalldProxy::CreateModuleDataDir(
    const std::string &ModuleDir, const std::vector<std::string> &abilityDirs, const int uid, const int gid)
{
    MessageParcel data;
    INSTALLD_PARCEL_WRITE_INTERFACE_TOKEN(data, (GetDescriptor()));
    INSTALLD_PARCEL_WRITE(data, String16, Str8ToStr16(ModuleDir));
    INSTALLD_PARCEL_WRITE(data, StringVector, abilityDirs);
    INSTALLD_PARCEL_WRITE(data, Int32, uid);
    INSTALLD_PARCEL_WRITE(data, Int32, gid);

    MessageParcel reply;
    MessageOption option;
    return TransactInstalldCmd(IInstalld::Message::CREATE_MODULE_DATA_DIR, data, reply, option);
}

ErrCode InstalldProxy::RemoveDir(const std::string &dir)
{
    MessageParcel data;
    INSTALLD_PARCEL_WRITE_INTERFACE_TOKEN(data, (GetDescriptor()));
    INSTALLD_PARCEL_WRITE(data, String16, Str8ToStr16(dir));

    MessageParcel reply;
    MessageOption option(MessageOption::TF_SYNC);
    return TransactInstalldCmd(IInstalld::Message::REMOVE_DIR, data, reply, option);
}

ErrCode InstalldProxy::CleanBundleDataDir(const std::string &bundleDir)
{
    MessageParcel data;
    INSTALLD_PARCEL_WRITE_INTERFACE_TOKEN(data, (GetDescriptor()));
    INSTALLD_PARCEL_WRITE(data, String16, Str8ToStr16(bundleDir));

    MessageParcel reply;
    MessageOption option(MessageOption::TF_SYNC, WAIT_TIME);
    return TransactInstalldCmd(IInstalld::Message::CLEAN_BUNDLE_DATA_DIR, data, reply, option);
}

ErrCode InstalldProxy::TransactInstalldCmd(uint32_t code, MessageParcel &data, MessageParcel &reply,
    MessageOption &option)
{
    sptr<IRemoteObject> remote = Remote();
    if (remote == nullptr) {
        APP_LOGE("fail to send %{public}d cmd to service due to remote object is null", code);
        return ERR_APPEXECFWK_INSTALL_INSTALLD_SERVICE_ERROR;
    }

    if (remote->SendRequest(code, data, reply, option) != OHOS::NO_ERROR) {
        APP_LOGE("fail to send %{public}d request to service due to transact error", code);
        return ERR_APPEXECFWK_INSTALL_INSTALLD_SERVICE_ERROR;
    }
    return reply.ReadInt32();
}
}  // namespace AppExecFwk
}  // namespace OHOS