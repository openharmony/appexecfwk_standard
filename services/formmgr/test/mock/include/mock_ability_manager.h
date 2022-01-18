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

#ifndef FOUNDATION_APPEXECFWK_SERVICES_BUNDLEMGR_TEST_MOCK_MOCK_APP_MGR_HOST_H
#define FOUNDATION_APPEXECFWK_SERVICES_BUNDLEMGR_TEST_MOCK_MOCK_APP_MGR_HOST_H

#include <iremote_object.h>
#include <iremote_stub.h>

#include "ability_manager_interface.h"
#include "mock_form_provider_client.h"
#include "semaphore_ex.h"

namespace OHOS {
namespace AppExecFwk {
class MockAbilityMgrProxy : public IRemoteProxy<AAFwk::IAbilityManager> {
public:
    explicit MockAbilityMgrProxy(const sptr<IRemoteObject> &impl) : IRemoteProxy<AAFwk::IAbilityManager>(impl)
    {}

    virtual ~MockAbilityMgrProxy() = default;
    virtual int UpdateConfiguration(const Configuration &config) override
    {
        return 0;
    }
    virtual int StartAbility(const AAFwk::Want &want, int requestCode = -1)
    {
        return 0;
    }
    virtual int StartAbility(const AAFwk::Want &want, const sptr<IRemoteObject> &callerToken, int requestCode = -1)
    {
        return 0;
    }
    virtual int TerminateAbility(
        const sptr<IRemoteObject> &token, int resultCode, const AAFwk::Want *resultWant = nullptr)
    {
        return 0;
    }
    virtual int ConnectAbility(
        const AAFwk::Want &want, const sptr<AAFwk::IAbilityConnection> &connect, const sptr<IRemoteObject> &callerToken)
    {
        return 0;
    }
    virtual int DisconnectAbility(const sptr<AAFwk::IAbilityConnection> &connect)
    {
        return 0;
    }
    virtual sptr<AAFwk::IAbilityScheduler> AcquireDataAbility(
        const Uri &uri, bool tryBind, const sptr<IRemoteObject> &callerToken)
    {
        return nullptr;
    }
    virtual int ReleaseDataAbility(
        sptr<AAFwk::IAbilityScheduler> dataAbilityScheduler, const sptr<IRemoteObject> &callerToken)
    {
        return 0;
    }
    virtual void AddWindowInfo(const sptr<IRemoteObject> &token, int32_t windowToken)
    {
        return;
    }
    virtual int AttachAbilityThread(const sptr<AAFwk::IAbilityScheduler> &scheduler, const sptr<IRemoteObject> &token)
    {
        return 0;
    }
    virtual int AbilityTransitionDone(const sptr<IRemoteObject> &token, int state, const PacMap &saveData)
    {
        return 0;
    }
    virtual int GetRecentMissions(
        const int32_t numMax, const int32_t flags, std::vector<AAFwk::AbilityMissionInfo> &recentList) override
    {
        return 0;
    }
    virtual int PowerOff() override
    {
        return 0;
    }
    virtual int PowerOn() override
    {
        return 0;
    }
    virtual int lockMission(int missionId)
    {
        return 0;
    }
    virtual int UnlockMission(int missionId) override
    {
        return 0;
    }
    virtual int SetMissionDescriptionInfo(
        const sptr<IRemoteObject> &token, const AAFwk::MissionDescriptionInfo &missionDescriptionInfo) override
    {
        return 0;
    }
    virtual int GetMissionLockModeState() override
    {
        return 0;
    }
    virtual sptr<AAFwk::IWantSender> GetWantSender(
        const AAFwk::WantSenderInfo &wantSenderInfo, const sptr<IRemoteObject> &callerToken) override
    {
        return nullptr;
    }
    virtual int SendWantSender(const sptr<AAFwk::IWantSender> &target, const AAFwk::SenderInfo &senderInfo) override
    {
        return 0;
    }
    virtual void CancelWantSender(const sptr<AAFwk::IWantSender> &sender) override
    {
        return;
    }
    virtual int GetPendingWantUid(const sptr<AAFwk::IWantSender> &target) override
    {
        return 0;
    }
    virtual int GetPendingWantUserId(const sptr<AAFwk::IWantSender> &target) override
    {
        return 0;
    }
    virtual std::string GetPendingWantBundleName(const sptr<AAFwk::IWantSender> &target) override
    {
        return "";
    }
    virtual int GetPendingWantCode(const sptr<AAFwk::IWantSender> &target) override
    {
        return 0;
    }
    virtual int GetPendingWantType(const sptr<AAFwk::IWantSender> &target) override
    {
        return 0;
    }
    virtual void RegisterCancelListener(
        const sptr<AAFwk::IWantSender> &sender, const sptr<AAFwk::IWantReceiver> &receiver) override
    {
        return;
    }
    virtual void UnregisterCancelListener(
        const sptr<AAFwk::IWantSender> &sender, const sptr<AAFwk::IWantReceiver> &receiver) override
    {
        return;
    }
    virtual int GetPendingRequestWant(const sptr<AAFwk::IWantSender> &target, std::shared_ptr<Want> &want) override
    {
        return 0;
    }
    virtual int ScheduleConnectAbilityDone(const sptr<IRemoteObject> &token, const sptr<IRemoteObject> &remoteObject)
    {
        return 0;
    }
    virtual int ScheduleDisconnectAbilityDone(const sptr<IRemoteObject> &token)
    {
        return 0;
    }
    virtual int ScheduleCommandAbilityDone(const sptr<IRemoteObject> &token)
    {
        return 0;
    }
    virtual void DumpState(const std::string &args, std::vector<std::string> &state)
    {
        return;
    }
    virtual int TerminateAbilityResult(const sptr<IRemoteObject> &token, int startId)
    {
        return 0;
    }
    virtual int StopServiceAbility(const AAFwk::Want &want)
    {
        return 0;
    }
    virtual int GetAllStackInfo(AAFwk::StackInfo &stackInfo)
    {
        return 0;
    }
    virtual int GetMissionSnapshot(const int32_t missionId, AAFwk::MissionPixelMap &missionPixelMap)
    {
        return 0;
    }
    virtual int MoveMissionToTop(int32_t missionId)
    {
        return 0;
    }
    /**
     * Requires that tasks associated with a given capability token be moved to the background
     *
     * @param token ability token
     * @param nonFirst If nonfirst is false and not the lowest ability of the mission, you cannot move mission to end
     * @return Returns ERR_OK on success, others on failure.
     */
    virtual int MoveMissionToEnd(const sptr<IRemoteObject> &token, const bool nonFirst)
    {
        return 0;
    }
    virtual int RemoveMission(int id)
    {
        return 0;
    }
    virtual int RemoveStack(int id)
    {
        return 0;
    }
    virtual int KillProcess(const std::string &bundleName)
    {
        return 0;
    }
    virtual int UninstallApp(const std::string &bundleName)
    {
        return 0;
    }
    virtual int TerminateAbilityByRecordId(const int64_t recordId = -1)
    {
        return 0;
    }
    virtual int TerminateAbilityByCaller(const sptr<IRemoteObject> &callerToken, int requestCode)
    {
        return 0;
    }
    /** Checks whether this ability is the first ability in a mission.
     * @param lostToken, the token of ability
     * @return Returns true is first in Mission.
     */
    virtual bool IsFirstInMission(const sptr<IRemoteObject> &token)
    {
        return 0;
    }
    /**
     * Checks whether a specified permission has been granted to the process identified by pid and uid
     *
     * @param permission Indicates the permission to check.
     * @param pid Indicates the ID of the process to check.
     * @param uid Indicates the UID of the process to check.
     * @param message Describe success or failure
     *
     * @return Returns ERR_OK on success, others on failure.
     */
    virtual int CompelVerifyPermission(const std::string &permission, int pid, int uid, std::string &message)
    {
        return 0;
    }

    virtual int SetShowOnLockScreen(bool isAllow) override
    {
        return 0;
    }
    int StartContinuation(const Want &want, const sptr<IRemoteObject> &abilityToken, int32_t status) override
    {
        return 0;
    }
    int NotifyContinuationResult(int32_t missionId, const int32_t result) override
    {
        return 0;
    }
    int ContinueMission(const std::string &srcDeviceId, const std::string &dstDeviceId,
        int32_t missionId, const sptr<IRemoteObject> &callBack, AAFwk::WantParams &wantParams) override
    {
        return 0;
    }
    int ContinueAbility(const std::string &deviceId, int32_t missionId) override
    {
        return 0;
    }
    void NotifyCompleteContinuation(const std::string &deviceId, int32_t sessionId, bool isSuccess) override
    {}

    virtual int LockMissionForCleanup(int32_t missionId) override
    {
        return 0;
    }

    virtual int UnlockMissionForCleanup(int32_t missionId) override
    {
        return 0;
    }

    virtual int RegisterMissionListener(const sptr<IMissionListener> &listener) override
    {
        return 0;
    }

    virtual int UnRegisterMissionListener(const sptr<IMissionListener> &listener) override
    {
        return 0;
    }

    virtual int GetMissionInfos(const std::string& deviceId, int32_t numMax,
        std::vector<MissionInfo> &missionInfos) override
    {
        return 0;
    }

    virtual int GetMissionInfo(const std::string& deviceId, int32_t missionId,
        MissionInfo &missionInfo) override
    {
        return 0;
    }

    virtual int CleanMission(int32_t missionId) override
    {
        return 0;
    }

    virtual int CleanAllMissions() override
    {
        return 0;
    }
    virtual int MoveMissionToFront(int32_t missionId) override
    {
        return 0;
    }
};

class MockAbilityMgrStub : public IRemoteStub<AAFwk::IAbilityManager> {
public:
    using Uri = OHOS::Uri;
    MockAbilityMgrStub() = default;
    virtual ~MockAbilityMgrStub() = default;

    virtual int OnRemoteRequest(
        uint32_t code, MessageParcel &data, MessageParcel &reply, MessageOption &option) override
    {
        return 0;
    }
};
class MockAbilityMgrService : public MockAbilityMgrStub {
public:
    void Wait()
    {
        sem_.Wait();
    }

    int Post()
    {
        sem_.Post();
        return 0;
    }

    void PostVoid()
    {
        sem_.Post();
    }

    virtual int UpdateConfiguration(const Configuration &config) override
    {
        return 0;
    }
    virtual int StartAbility(const AAFwk::Want &want, int requestCode = -1)
    {
        return 0;
    }
    virtual int StartAbility(const AAFwk::Want &want, const sptr<IRemoteObject> &callerToken, int requestCode = -1)
    {
        return 0;
    }
    virtual int TerminateAbility(
        const sptr<IRemoteObject> &token, int resultCode, const AAFwk::Want *resultWant = nullptr)
    {
        return 0;
    }
    virtual int MinimizeAbility(const sptr<IRemoteObject> &token) override
    {
        return 0;
    }
    virtual int ConnectAbility(
        const AAFwk::Want &want, const sptr<AAFwk::IAbilityConnection> &connect, const sptr<IRemoteObject> &callerToken)
    {
        connect->OnAbilityConnectDone(want.GetElement(), new (std::nothrow) MockFormProviderClient(), 0);
        return 0;
    }
    virtual int DisconnectAbility(const sptr<AAFwk::IAbilityConnection> &connect)
    {
        return 0;
    }
    virtual sptr<AAFwk::IAbilityScheduler> AcquireDataAbility(
        const Uri &uri, bool tryBind, const sptr<IRemoteObject> &callerToken)
    {
        return nullptr;
    }
    virtual int ReleaseDataAbility(
        sptr<AAFwk::IAbilityScheduler> dataAbilityScheduler, const sptr<IRemoteObject> &callerToken)
    {
        return 0;
    }
    virtual void AddWindowInfo(const sptr<IRemoteObject> &token, int32_t windowToken)
    {
        return;
    }
    virtual int AttachAbilityThread(const sptr<AAFwk::IAbilityScheduler> &scheduler, const sptr<IRemoteObject> &token)
    {
        return 0;
    }
    virtual int AbilityTransitionDone(const sptr<IRemoteObject> &token, int state, const PacMap &saveData)
    {
        return 0;
    }
    virtual int ScheduleConnectAbilityDone(const sptr<IRemoteObject> &token, const sptr<IRemoteObject> &remoteObject)
    {
        return 0;
    }
    virtual int ScheduleDisconnectAbilityDone(const sptr<IRemoteObject> &token)
    {
        return 0;
    }
    virtual int ScheduleCommandAbilityDone(const sptr<IRemoteObject> &token)
    {
        return 0;
    }
    virtual void DumpState(const std::string &args, std::vector<std::string> &state)
    {
        return;
    }
    virtual int TerminateAbilityResult(const sptr<IRemoteObject> &token, int startId)
    {
        return 0;
    }
    virtual int StopServiceAbility(const AAFwk::Want &want)
    {
        return 0;
    }
    virtual int GetAllStackInfo(AAFwk::StackInfo &stackInfo)
    {
        return 0;
    }
    virtual int GetRecentMissions(
        const int32_t numMax, const int32_t flags, std::vector<AAFwk::AbilityMissionInfo> &recentList) override
    {
        return 0;
    }
    virtual int PowerOff() override
    {
        return 0;
    }
    virtual int PowerOn() override
    {
        return 0;
    }
    virtual int LockMission(int missionId) override
    {
        return 0;
    }
    virtual int UnlockMission(int missionId) override
    {
        return 0;
    }
    virtual int SetMissionDescriptionInfo(
        const sptr<IRemoteObject> &token, const AAFwk::MissionDescriptionInfo &missionDescriptionInfo) override
    {
        return 0;
    }
    virtual int GetMissionLockModeState() override
    {
        return 0;
    }
    virtual sptr<AAFwk::IWantSender> GetWantSender(
        const AAFwk::WantSenderInfo &wantSenderInfo, const sptr<IRemoteObject> &callerToken) override
    {
        return nullptr;
    }
    virtual int SendWantSender(const sptr<AAFwk::IWantSender> &target, const AAFwk::SenderInfo &senderInfo) override
    {
        return 0;
    }
    virtual void CancelWantSender(const sptr<AAFwk::IWantSender> &sender) override
    {
        return;
    }
    virtual int GetPendingWantUid(const sptr<AAFwk::IWantSender> &target) override
    {
        return 0;
    }
    virtual int GetPendingWantUserId(const sptr<AAFwk::IWantSender> &target) override
    {
        return 0;
    }
    virtual std::string GetPendingWantBundleName(const sptr<AAFwk::IWantSender> &target) override
    {
        return "";
    }
    virtual int GetPendingWantCode(const sptr<AAFwk::IWantSender> &target) override
    {
        return 0;
    }
    virtual int GetPendingWantType(const sptr<AAFwk::IWantSender> &target) override
    {
        return 0;
    }
    virtual void RegisterCancelListener(
        const sptr<AAFwk::IWantSender> &sender, const sptr<AAFwk::IWantReceiver> &receiver) override
    {
        return;
    }
    virtual void UnregisterCancelListener(
        const sptr<AAFwk::IWantSender> &sender, const sptr<AAFwk::IWantReceiver> &receiver) override
    {
        return;
    }
    virtual int GetPendingRequestWant(const sptr<AAFwk::IWantSender> &target, std::shared_ptr<Want> &want) override
    {
        return 0;
    }
    virtual int GetMissionSnapshot(const int32_t missionId, AAFwk::MissionPixelMap &missionPixelMap)
    {
        return 0;
    }
    virtual int MoveMissionToTop(int32_t missionId)
    {
        return 0;
    }
    /**
     * Requires that tasks associated with a given capability token be moved to the background
     *
     * @param token ability token
     * @param nonFirst If nonfirst is false and not the lowest ability of the mission, you cannot move mission to end
     * @return Returns ERR_OK on success, others on failure.
     */
    virtual int MoveMissionToEnd(const sptr<IRemoteObject> &token, const bool nonFirst)
    {
        return 0;
    }
    virtual int RemoveMission(int id)
    {
        return 0;
    }
    virtual int RemoveStack(int id)
    {
        return 0;
    }
    virtual int KillProcess(const std::string &bundleName)
    {
        return 0;
    }
    virtual int UninstallApp(const std::string &bundleName)
    {
        return 0;
    }
    virtual int TerminateAbilityByRecordId(const int64_t recordId = -1)
    {
        return 0;
    }
    virtual int TerminateAbilityByCaller(const sptr<IRemoteObject> &callerToken, int requestCode)
    {
        return 0;
    }
    virtual int SetShowOnLockScreen(bool isAllow) override
    {
        return 0;
    }
    int MoveMissionToFloatingStack(const MissionOption &missionOption)
    {
        return 0;
    }
    int MoveMissionToSplitScreenStack(const MissionOption &primary, const MissionOption &secondary)
    {
        return 0;
    }
    int MinimizeMultiWindow(int missionId)
    {
        return 0;
    }
    int MaximizeMultiWindow(int missionId)
    {
        return 0;
    }
    int GetFloatingMissions(std::vector<AbilityMissionInfo> &list)
    {
        return 0;
    }
    int CloseMultiWindow(int missionId)
    {
        return 0;
    }
    int SetMissionStackSetting(const StackSetting &stackSetting)
    {
        return 0;
    }
    int StartAbility(const Want &want, const AbilityStartSetting &abilityStartSetting,
        const sptr<IRemoteObject> &callerToken, int requestCode = 0)
    {
        return 0;
    }
    int ChangeFocusAbility(const sptr<IRemoteObject> &lostFocusToken, const sptr<IRemoteObject> &getFocusToken)
    {
        return 0;
    }

    /** Checks whether this ability is the first ability in a mission.
     * @param lostToken, the token of ability
     * @return Returns true is first in Mission.
     */
    virtual bool IsFirstInMission(const sptr<IRemoteObject> &token)
    {
        return 0;
    }
    /**
     * Checks whether a specified permission has been granted to the process identified by pid and uid
     *
     * @param permission Indicates the permission to check.
     * @param pid Indicates the ID of the process to check.
     * @param uid Indicates the UID of the process to check.
     * @param message Describe success or failure
     *
     * @return Returns ERR_OK on success, others on failure.
     */
    virtual int CompelVerifyPermission(const std::string &permission, int pid, int uid, std::string &message)
    {
        return 0;
    }

    virtual void GetSystemMemoryAttr(AppExecFwk::SystemMemoryAttr &memoryInfo)
    {}

    int GetWantSenderInfo(const sptr<IWantSender> &target, std::shared_ptr<WantSenderInfo> &info) override
    {
        return 0;
    }
    int ClearUpApplicationData(const std::string &bundleName) override
    {
        return 0;
    }
    virtual int LockMissionForCleanup(int32_t missionId) override
    {
        return 0;
    }
        int StartContinuation(const Want &want, const sptr<IRemoteObject> &abilityToken, int32_t status) override
    {
        return 0;
    }
    int NotifyContinuationResult(int32_t missionId, const int32_t result) override
    {
        return 0;
    }
    int ContinueMission(const std::string &srcDeviceId, const std::string &dstDeviceId,
        int32_t missionId, const sptr<IRemoteObject> &callBack, AAFwk::WantParams &wantParams) override
    {
        return 0;
    }
    int ContinueAbility(const std::string &deviceId, int32_t missionId) override
    {
        return 0;
    }
    void NotifyCompleteContinuation(const std::string &deviceId, int32_t sessionId, bool isSuccess) override
    {}

    virtual int UnlockMissionForCleanup(int32_t missionId) override
    {
        return 0;
    }

    virtual int RegisterMissionListener(const sptr<IMissionListener> &listener) override
    {
        return 0;
    }

    virtual int UnRegisterMissionListener(const sptr<IMissionListener> &listener) override
    {
        return 0;
    }

    virtual int GetMissionInfos(const std::string& deviceId, int32_t numMax,
        std::vector<MissionInfo> &missionInfos) override
    {
        return 0;
    }

    virtual int GetMissionInfo(const std::string& deviceId, int32_t missionId,
        MissionInfo &missionInfo) override
    {
        return 0;
    }

    virtual int CleanMission(int32_t missionId) override
    {
        return 0;
    }

    virtual int CleanAllMissions() override
    {
        return 0;
    }

    virtual int MoveMissionToFront(int32_t missionId) override
    {
        return 0;
    }
    virtual int StartSyncRemoteMissions(const std::string& devId, bool fixConflict, int64_t tag) override
    {
        return 0;
    }
    virtual int StopSyncRemoteMissions(const std::string& devId) override
    {
        return 0;
    }
    virtual int RegisterMissionListener(const std::string &deviceId,
        const sptr<IRemoteMissionListener> &listener) override
    {
        return 0;
    }
    virtual int UnRegisterMissionListener(const std::string &deviceId,
        const sptr<IRemoteMissionListener> &listener) override
    {
        return 0;
    }
    virtual int GetAbilityRunningInfos(std::vector<AbilityRunningInfo> &info) override
    {
        return 0;
    }
    virtual int GetExtensionRunningInfos(int upperLimit, std::vector<ExtensionRunningInfo> &info) override
    {
        return 0;
    }
    virtual int GetProcessRunningInfos(std::vector<AppExecFwk::RunningProcessInfo> &info) override
    {
        return 0;
    }
    virtual int SetAbilityController(const sptr<AppExecFwk::IAbilityController> &abilityController,
        bool imAStabilityTest) override
    {
        return 0;
    }
    virtual bool IsUserAStabilityTest() override
    {
        return true;
    }
    virtual int StartUser(int userId) override
    {
        return 0;
    }
    virtual int StopUser(int userId, const sptr<IStopUserCallback> &callback) override
    {
        return 0;
    }
    virtual int GetMissionSnapshot(const std::string& deviceId, int32_t missionId, MissionSnapshot& snapshot) override
    {
        return 0;
    }
    virtual int RegisterSnapshotHandler(const sptr<ISnapshotHandler>& handler) override
    {
        return 0;
    }
private:
    Semaphore sem_;
};
}  // namespace AppExecFwk
}  // namespace OHOS
#endif  // FOUNDATION_APPEXECFWK_SERVICES_BUNDLEMGR_TEST_MOCK_MOCK_APP_MGR_HOST_H
