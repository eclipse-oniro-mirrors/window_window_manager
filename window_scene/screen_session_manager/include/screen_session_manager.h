/*
 * Copyright (c) 2023 Huawei Device Co., Ltd.
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

#ifndef OHOS_ROSEN_WINDOW_SCENE_SCREEN_SESSION_MANAGER_H
#define OHOS_ROSEN_WINDOW_SCENE_SCREEN_SESSION_MANAGER_H

#include <shared_mutex>
#include <system_ability.h>
#include <mutex>
#include <condition_variable>

#include "common/include/task_scheduler.h"
#include "dm_common.h"
#include "event_tracker.h"
#include "session/screen/include/screen_session.h"
#include "zidl/screen_session_manager_stub.h"
#include "client_agent_container.h"
#include "session_display_power_controller.h"
#include "wm_single_instance.h"

#include "agent_death_recipient.h"
#include "screen.h"
#include "screen_cutout_controller.h"
#include "fold_screen_controller/fold_screen_controller.h"
#include "fold_screen_controller/fold_screen_sensor_manager.h"
#include "fold_screen_controller/super_fold_state_manager.h"

namespace OHOS::Rosen {
class RSInterfaces;

class ScreenSessionManager : public SystemAbility, public ScreenSessionManagerStub, public IScreenChangeListener {
DECLARE_SYSTEM_ABILITY(ScreenSessionManager)
WM_DECLARE_SINGLE_INSTANCE_BASE(ScreenSessionManager)

public:
    sptr<ScreenSession> GetScreenSession(ScreenId screenId) const;
    sptr<ScreenSession> GetDefaultScreenSession();
    std::vector<ScreenId> GetAllScreenIds();

    sptr<DisplayInfo> GetDefaultDisplayInfo() override;
    DMError SetScreenActiveMode(ScreenId screenId, uint32_t modeId) override;
    DMError SetVirtualPixelRatio(ScreenId screenId, float virtualPixelRatio) override;
    DMError SetVirtualPixelRatioSystem(ScreenId screenId, float virtualPixelRatio) override;
    DMError SetResolution(ScreenId screenId, uint32_t width, uint32_t height, float virtualPixelRatio) override;
    DMError GetDensityInCurResolution(ScreenId screenId, float& virtualPixelRatio) override;
    void NotifyScreenChanged(sptr<ScreenInfo> screenInfo, ScreenChangeEvent event);

    DMError GetScreenColorGamut(ScreenId screenId, ScreenColorGamut& colorGamut) override;
    DMError SetScreenColorGamut(ScreenId screenId, int32_t colorGamutIdx) override;
    DMError GetScreenGamutMap(ScreenId screenId, ScreenGamutMap& gamutMap) override;
    DMError SetScreenGamutMap(ScreenId screenId, ScreenGamutMap gamutMap) override;
    DMError SetScreenColorTransform(ScreenId screenId) override;

    DMError GetPixelFormat(ScreenId screenId, GraphicPixelFormat& pixelFormat) override;
    DMError SetPixelFormat(ScreenId screenId, GraphicPixelFormat pixelFormat) override;
    DMError GetSupportedHDRFormats(ScreenId screenId, std::vector<ScreenHDRFormat>& hdrFormats) override;
    DMError GetScreenHDRFormat(ScreenId screenId, ScreenHDRFormat& hdrFormat) override;
    DMError SetScreenHDRFormat(ScreenId screenId, int32_t modeIdx) override;
    DMError GetSupportedColorSpaces(ScreenId screenId, std::vector<GraphicCM_ColorSpaceType>& colorSpaces) override;
    DMError GetScreenColorSpace(ScreenId screenId, GraphicCM_ColorSpaceType& colorSpace) override;
    DMError SetScreenColorSpace(ScreenId screenId, GraphicCM_ColorSpaceType colorSpace) override;

    void DumpAllScreensInfo(std::string& dumpInfo) override;
    void DumpSpecialScreenInfo(ScreenId id, std::string& dumpInfo) override;

    virtual DMError RegisterDisplayManagerAgent(const sptr<IDisplayManagerAgent>& displayManagerAgent,
        DisplayManagerAgentType type) override;
    virtual DMError UnregisterDisplayManagerAgent(const sptr<IDisplayManagerAgent>& displayManagerAgent,
        DisplayManagerAgentType type) override;

    bool WakeUpBegin(PowerStateChangeReason reason) override;
    bool WakeUpEnd() override;
    bool SuspendBegin(PowerStateChangeReason reason) override;
    bool SuspendEnd() override;
    ScreenId GetInternalScreenId() override;
    bool SetScreenPowerById(ScreenId screenId, ScreenPowerState state, PowerStateChangeReason reason) override;
    bool SetDisplayState(DisplayState state) override;
    DisplayState GetDisplayState(DisplayId displayId) override;
    bool SetScreenBrightness(uint64_t screenId, uint32_t level) override;
    uint32_t GetScreenBrightness(uint64_t screenId) override;
    bool SetSpecifiedScreenPower(ScreenId screenId, ScreenPowerState state, PowerStateChangeReason reason) override;
    bool SetScreenPowerForAll(ScreenPowerState state, PowerStateChangeReason reason) override;
    ScreenPowerState GetScreenPower(ScreenId screenId) override;
    void NotifyDisplayEvent(DisplayEvent event) override;
    bool TryToCancelScreenOff() override;

    void RegisterDisplayChangeListener(sptr<IDisplayChangeListener> listener);
    bool NotifyDisplayPowerEvent(DisplayPowerEvent event, EventStatus status, PowerStateChangeReason reason);
    bool NotifyDisplayStateChanged(DisplayId id, DisplayState state);
    void NotifyScreenshot(DisplayId displayId);
    virtual ScreenId CreateVirtualScreen(VirtualScreenOption option,
                                         const sptr<IRemoteObject>& displayManagerAgent) override;
    virtual DMError SetVirtualScreenSurface(ScreenId screenId, sptr<IBufferProducer> surface) override;
    virtual DMError SetVirtualMirrorScreenCanvasRotation(ScreenId screenId, bool autoRotate) override;
    virtual DMError SetVirtualMirrorScreenScaleMode(ScreenId screenId, ScreenScaleMode scaleMode) override;
    virtual DMError DestroyVirtualScreen(ScreenId screenId) override;
    DMError ResizeVirtualScreen(ScreenId screenId, uint32_t width, uint32_t height) override;
    virtual DMError MakeMirror(ScreenId mainScreenId, std::vector<ScreenId> mirrorScreenIds,
        ScreenId& screenGroupId) override;
    virtual DMError MakeMirror(ScreenId mainScreenId, ScreenId mirrorScreenId,
        DMRect mainScreenRegion, ScreenId& screenGroupId) override;
    virtual DMError SetMultiScreenMode(ScreenId mainScreenId, ScreenId secondaryScreenId,
        MultiScreenMode screenMode) override;
    virtual DMError SetMultiScreenRelativePosition(MultiScreenPositionOptions mainScreenOptions,
        MultiScreenPositionOptions secondScreenOption) override;
    virtual DMError StopMirror(const std::vector<ScreenId>& mirrorScreenIds) override;
    DMError DisableMirror(bool disableOrNot) override;
    virtual DMError MakeExpand(std::vector<ScreenId> screenId, std::vector<Point> startPoint,
                               ScreenId& screenGroupId) override;
    virtual DMError StopExpand(const std::vector<ScreenId>& expandScreenIds) override;
    DMError MakeUniqueScreen(const std::vector<ScreenId>& screenIds) override;
    virtual sptr<ScreenGroupInfo> GetScreenGroupInfoById(ScreenId screenId) override;
    virtual void RemoveVirtualScreenFromGroup(std::vector<ScreenId> screens) override;
    virtual std::shared_ptr<Media::PixelMap> GetDisplaySnapshot(DisplayId displayId, DmErrorCode* errorCode) override;
    virtual std::shared_ptr<Media::PixelMap> GetSnapshotByPicker(Media::Rect &rect, DmErrorCode* errorCode) override;
    virtual sptr<DisplayInfo> GetDisplayInfoById(DisplayId displayId) override;
    sptr<DisplayInfo> GetDisplayInfoByScreen(ScreenId screenId) override;
    std::vector<DisplayId> GetAllDisplayIds() override;
    virtual sptr<ScreenInfo> GetScreenInfoById(ScreenId screenId) override;
    virtual DMError GetAllScreenInfos(std::vector<sptr<ScreenInfo>>& screenInfos) override;
    virtual DMError GetScreenSupportedColorGamuts(ScreenId screenId,
        std::vector<ScreenColorGamut>& colorGamuts) override;
    DMError IsScreenRotationLocked(bool& isLocked) override;
    DMError SetScreenRotationLocked(bool isLocked) override;
    DMError SetScreenRotationLockedFromJs(bool isLocked) override;
    DMError SetOrientation(ScreenId screenId, Orientation orientation) override;
    bool SetRotation(ScreenId screenId, Rotation rotationAfter, bool isFromWindow);
    void SetSensorSubscriptionEnabled();
    bool SetRotationFromWindow(Rotation targetRotation);
    sptr<SupportedScreenModes> GetScreenModesByDisplayId(DisplayId displayId);
    sptr<ScreenInfo> GetScreenInfoByDisplayId(DisplayId displayId);
    void NotifyDisplayCreate(sptr<DisplayInfo> displayInfo);
    void NotifyDisplayDestroy(DisplayId displayId);
    void NotifyAndPublishEvent(sptr<DisplayInfo> displayInfo, ScreenId screenId, sptr<ScreenSession> screenSession);
    void NotifyDisplayChanged(sptr<DisplayInfo> displayInfo, DisplayChangeEvent event);

    std::vector<ScreenId> GetAllScreenIds() const;
    const std::shared_ptr<RSDisplayNode> GetRSDisplayNodeByScreenId(ScreenId smsScreenId) const;
    std::shared_ptr<Media::PixelMap> GetScreenSnapshot(DisplayId displayId);

    sptr<ScreenSession> InitVirtualScreen(ScreenId smsScreenId, ScreenId rsId, VirtualScreenOption option);
    sptr<ScreenSession> InitAndGetScreen(ScreenId rsScreenId);
    bool InitAbstractScreenModesInfo(sptr<ScreenSession>& absScreen);
    std::vector<ScreenId> GetAllValidScreenIds(const std::vector<ScreenId>& screenIds) const;

    sptr<ScreenSessionGroup> AddToGroupLocked(sptr<ScreenSession> newScreen, bool isUnique = false);
    sptr<ScreenSessionGroup> AddAsFirstScreenLocked(sptr<ScreenSession> newScreen, bool isUnique = false);
    sptr<ScreenSessionGroup> AddAsSuccedentScreenLocked(sptr<ScreenSession> newScreen);
    sptr<ScreenSessionGroup> RemoveFromGroupLocked(sptr<ScreenSession> screen);
    sptr<ScreenSessionGroup> GetAbstractScreenGroup(ScreenId smsScreenId);

    void SetMultiScreenFrameControl(void);
    bool IsPhysicalScreenAndInUse(sptr<ScreenSession> screenSession) const;
    bool HandleFoldScreenSessionCreate(ScreenId screenId);

    void ChangeScreenGroup(sptr<ScreenSessionGroup> group, const std::vector<ScreenId>& screens,
             const std::vector<Point>& startPoints, bool filterScreen, ScreenCombination combination,
             DMRect mainScreenRegion = DMRect::NONE());

    bool RemoveChildFromGroup(sptr<ScreenSession> screen, sptr<ScreenSessionGroup> screenGroup);

    void AddScreenToGroup(sptr<ScreenSessionGroup> group,
        const std::vector<ScreenId>& addScreens, const std::vector<Point>& addChildPos,
        std::map<ScreenId, bool>& removeChildResMap);
    bool CheckScreenInScreenGroup(sptr<ScreenSession> screen) const;

    DMError SetMirror(ScreenId screenId, std::vector<ScreenId> screens, DMRect mainScreenRegion);
    DMError StopScreens(const std::vector<ScreenId>& screenIds, ScreenCombination stopCombination);

    void NotifyScreenConnected(sptr<ScreenInfo> screenInfo);
    void NotifyScreenDisconnected(ScreenId screenId);
    void NotifyScreenGroupChanged(const sptr<ScreenInfo>& screenInfo, ScreenGroupChangeEvent event);
    void NotifyScreenGroupChanged(const std::vector<sptr<ScreenInfo>>& screenInfo, ScreenGroupChangeEvent event);

    void NotifyPrivateSessionStateChanged(bool hasPrivate);
    void NotifyPrivateWindowListChanged(DisplayId id, std::vector<std::string> privacyWindowList);
    DMError HasPrivateWindow(DisplayId id, bool& hasPrivateWindow) override;
    bool ConvertScreenIdToRsScreenId(ScreenId screenId, ScreenId& rsScreenId) override;
    void UpdateDisplayHookInfo(int32_t uid, bool enable, const DMHookInfo& hookInfo) override;

    void OnScreenConnect(const sptr<ScreenInfo> screenInfo);
    void OnScreenDisconnect(ScreenId screenId);
    void OnScreenGroupChange(const std::string& trigger,
        const sptr<ScreenInfo>& screenInfo, ScreenGroupChangeEvent groupEvent);
    void OnScreenGroupChange(const std::string& trigger,
        const std::vector<sptr<ScreenInfo>>& screenInfos, ScreenGroupChangeEvent groupEvent);
    void OnScreenshot(sptr<ScreenshotInfo> info);
    bool IsMultiScreenCollaboration();
    sptr<CutoutInfo> GetCutoutInfo(DisplayId displayId) override;
    DMError HasImmersiveWindow(ScreenId screenId, bool& immersive) override;
    void SetDisplayBoundary(const sptr<ScreenSession> screenSession);

    void BlockScreenOnByCV(void);
    void BlockScreenOffByCV(void);
    bool BlockSetDisplayState(void);
    bool IsScreenLockSuspend(void);
    bool IsPreBrightAuthFail(void);
    void ScreenOffCVNotify(void);
    void DisablePowerOffRenderControl(ScreenId screenId) override;
    bool SetVirtualScreenStatus(ScreenId screenId, VirtualScreenStatus screenStatus) override;
    sptr<ScreenSession> GetOrCreateFakeScreenSession(sptr<ScreenSession> screenSession);
    void InitFakeScreenSession(sptr<ScreenSession> screenSession);

    // Fold Screen
    void SetFoldDisplayMode(const FoldDisplayMode displayMode) override;
    DMError SetFoldDisplayModeFromJs(const FoldDisplayMode displayMode) override;
    void SetDisplayNodeScreenId(ScreenId screenId, ScreenId displayNodeScreenId);

    void SetDisplayScale(ScreenId screenId, float scaleX, float scaleY,
        float pivotX, float pivotY) override;

    void SetFoldStatusLocked(bool locked) override;
    DMError SetFoldStatusLockedFromJs(bool locked) override;

    FoldDisplayMode GetFoldDisplayMode() override;

    bool IsFoldable() override;
    bool IsCaptured() override;

    FoldStatus GetFoldStatus() override;
    bool GetTentMode();

    bool SetScreenPower(ScreenPowerStatus status, PowerStateChangeReason reason);
    void SetScreenPowerForFold(ScreenPowerStatus status);
    void SetScreenPowerForFold(ScreenId screenId, ScreenPowerStatus status);
    void TriggerDisplayModeUpdate(FoldDisplayMode targetDisplayMode);
    void CallRsSetScreenPowerStatusSync(ScreenId screenId, ScreenPowerStatus status);
    void CallRsSetScreenPowerStatusSyncForFold(ScreenPowerStatus status);
    void TryToRecoverFoldDisplayMode(ScreenPowerStatus status);

    void SetKeyguardDrawnDoneFlag(bool flag);

    sptr<FoldCreaseRegion> GetCurrentFoldCreaseRegion() override;

    void NotifyFoldStatusChanged(FoldStatus foldStatus);
    void NotifyFoldAngleChanged(std::vector<float> foldAngles);
    int NotifyFoldStatusChanged(const std::string& statusParam);
    void NotifyDisplayModeChanged(FoldDisplayMode displayMode);
    void NotifyDisplayChangeInfoChanged(const sptr<DisplayChangeInfo>& info) override;
    void NotifyScreenMagneticStateChanged(bool isMagneticState);
    void OnTentModeChanged(bool isTentMode);
    void RegisterSettingDpiObserver();
    void RegisterExtendSettingDpiObserver();
    void RegisterSettingRotationObserver();

    void OnConnect(ScreenId screenId) override {}
    void OnDisconnect(ScreenId screenId) override {}
    void OnPropertyChange(const ScreenProperty& newProperty, ScreenPropertyChangeReason reason,
        ScreenId screenId) override;
    void OnPowerStatusChange(DisplayPowerEvent event, EventStatus status,
        PowerStateChangeReason reason) override;
    void OnSensorRotationChange(float sensorRotation, ScreenId screenId) override;
    void OnHoverStatusChange(int32_t hoverStatus, ScreenId screenId) override;
    void OnScreenOrientationChange(float screenOrientation, ScreenId screenId) override;
    void OnScreenRotationLockedChange(bool isLocked, ScreenId screenId) override;

    void SetHdrFormats(ScreenId screenId, sptr<ScreenSession>& session);
    void SetColorSpaces(ScreenId screenId, sptr<ScreenSession>& session);
    void SetClient(const sptr<IScreenSessionManagerClient>& client) override;
    ScreenProperty GetScreenProperty(ScreenId screenId) override;
    std::shared_ptr<RSDisplayNode> GetDisplayNode(ScreenId screenId) override;
    void UpdateScreenRotationProperty(ScreenId screenId, const RRect& bounds, float rotation,
        ScreenPropertyChangeType screenPropertyChangeType) override;
    uint32_t GetCurvedCompressionArea() override;
    ScreenProperty GetPhyScreenProperty(ScreenId screenId) override;
    void SetScreenPrivacyState(bool hasPrivate) override;
    void SetPrivacyStateByDisplayId(DisplayId id, bool hasPrivate) override;
    void SetScreenPrivacyWindowList(DisplayId id, std::vector<std::string> privacyWindowList) override;
    void UpdateAvailableArea(ScreenId screenId, DMRect area) override;
    int32_t SetScreenOffDelayTime(int32_t delay) override;
    DMError GetAvailableArea(DisplayId displayId, DMRect& area) override;
    void NotifyAvailableAreaChanged(DMRect area);
    void NotifyFoldToExpandCompletion(bool foldToExpand) override;
    bool GetSnapshotArea(Media::Rect &rect, DmErrorCode* errorCode, ScreenId &screenId);

    VirtualScreenFlag GetVirtualScreenFlag(ScreenId screenId) override;
    DMError SetVirtualScreenFlag(ScreenId screenId, VirtualScreenFlag screenFlag) override;

    DeviceScreenConfig GetDeviceScreenConfig() override;
    DMError SetVirtualScreenRefreshRate(ScreenId screenId, uint32_t refreshInterval) override;
    void SetVirtualScreenBlackList(ScreenId screenId, std::vector<uint64_t>& windowIdList) override;
    // notify scb virtual screen change
    void OnVirtualScreenChange(ScreenId screenId, ScreenEvent screenEvent);
    DMError VirtualScreenUniqueSwitch(const std::vector<ScreenId>& screenIds);
    void FixPowerStatus();
    void FoldScreenPowerInit();
    DMError ProxyForFreeze(const std::set<int32_t>& pidList, bool isProxy) override;
    DMError ResetAllFreezeStatus() override;

    void ReportFoldStatusToScb(std::vector<std::string>& screenFoldInfo);
    std::vector<DisplayPhysicalResolution> GetAllDisplayPhysicalResolution() override;

    void OnScreenExtendChange(ScreenId mainScreenId, ScreenId extendScreenId) override;
    void OnSuperFoldStatusChange(ScreenId screenId, SuperFoldStatus superFoldStatus) override;
    void SetDefaultScreenId(ScreenId defaultId);
    sptr<IScreenSessionManagerClient> GetClientProxy();
    void NotifyCastWhenScreenConnectChange(bool isConnected);
    void MultiScreenModeChange(const std::string& mainScreenId, const std::string& secondaryScreenId,
        const std::string& secondaryScreenMode);
    void SwitchScrollParam(FoldDisplayMode displayMode);
    void OnScreenChange(ScreenId screenId, ScreenEvent screenEvent);
    void SetCoordinationFlag(bool isCoordinationFlag);
    DMError SetVirtualScreenMaxRefreshRate(ScreenId id, uint32_t refreshRate,
        uint32_t& actualRefreshRate) override;

    void SetLastScreenMode(sptr<ScreenSession> firstSession, sptr<ScreenSession> secondarySession);
    /*
     * multi user
     */
    void SwitchUser() override;
    void SwitchScbNodeHandle(int32_t userId, int32_t newScbPid, bool coldBoot);
    void AddScbClientDeathRecipient(const sptr<IScreenSessionManagerClient>& scbClient, int32_t scbPid);
    void ScbClientDeathCallback(int32_t deathScbPid);
    void ScbStatusRecoveryWhenSwitchUser(std::vector<int32_t> oldScbPids, int32_t newScbPid);

    std::shared_ptr<Media::PixelMap> GetScreenCapture(const CaptureOption& captureOption,
        DmErrorCode* errorCode = nullptr) override;
    void OnScreenCaptureNotify(ScreenId mainScreenId, int32_t uid, const std::string& clientName) override;
    sptr<DisplayInfo> GetPrimaryDisplayInfo() override;
    std::shared_ptr<Media::PixelMap> GetDisplaySnapshotWithOption(const CaptureOption& captureOption,
        DmErrorCode* errorCode) override;

protected:
    ScreenSessionManager();
    virtual ~ScreenSessionManager() = default;

private:
    void OnStart() override;
    void Init();
    void LoadScreenSceneXml();
    void ConfigureScreenScene();
    void ConfigureDpi();
    void ConfigureCastParams();
    void ConfigureWaterfallDisplayCompressionParams();
    void ConfigureScreenSnapshotParams();
    void RegisterScreenChangeListener();
    void RegisterRefreshRateChangeListener();
    void OnHgmRefreshRateChange(uint32_t refreshRate);
    sptr<ScreenSession> GetOrCreateScreenSession(ScreenId screenId);
    void CreateScreenProperty(ScreenId screenId, ScreenProperty& property);
    void InitExtendScreenDensity(sptr<ScreenSession> session, ScreenProperty property);
    float CalcDefaultExtendScreenDensity(const ScreenProperty& property);
    sptr<ScreenSession> GetScreenSessionInner(ScreenId screenId, ScreenProperty property);
    sptr<ScreenSession> CreatePhysicalMirrorSessionInner(ScreenId screenId, ScreenId defaultScreenId,
        ScreenProperty property);
    void FreeDisplayMirrorNodeInner(const sptr<ScreenSession> mirrorSession);
    void MirrorSwitchNotify(ScreenId screenId);
    ScreenId GetDefaultScreenId();
    void AddVirtualScreenDeathRecipient(const sptr<IRemoteObject>& displayManagerAgent, ScreenId smsScreenId);
    void SendCastEvent(const bool &isPlugIn);
    void PhyMirrorConnectWakeupScreen();
    bool IsScreenRestored(sptr<ScreenSession> screenSession);
    bool GetIsCurrentInUseById(ScreenId screenId);
    bool GetMultiScreenInfo(MultiScreenMode& multiScreenMode,
        MultiScreenPositionOptions& mainScreenOption, MultiScreenPositionOptions& secondaryScreenOption);
    void RecoverMultiScreenInfoFromData(sptr<ScreenSession> screenSession);
    void HandleScreenConnectEvent(sptr<ScreenSession> screenSession, ScreenId screenId, ScreenEvent screenEvent);
    void HandleScreenDisconnectEvent(sptr<ScreenSession> screenSession, ScreenId screenId, ScreenEvent screenEvent);
    ScreenRotation ConvertOffsetToCorrectRotation(int32_t phyOffset);
    void MultiScreenModeChange(ScreenId mainScreenId, ScreenId secondaryScreenId, const std::string& operateType);
    void SetClientInner();
    void GetCurrentScreenPhyBounds(float& phyWidth, float& phyHeight, bool& isReset, const ScreenId& screenid);

    void NotifyDisplayStateChange(DisplayId defaultDisplayId, sptr<DisplayInfo> displayInfo,
        const std::map<DisplayId, sptr<DisplayInfo>>& displayInfoMap, DisplayStateChangeType type);
    void NotifyCaptureStatusChanged();
    DMError DoMakeMirror(ScreenId mainScreenId, std::vector<ScreenId> mirrorScreenIds,
        DMRect mainScreenRegion, ScreenId& screenGroupId);
    bool OnMakeExpand(std::vector<ScreenId> screenId, std::vector<Point> startPoint);
    bool OnRemoteDied(const sptr<IRemoteObject>& agent);
    std::string TransferTypeToString(ScreenType type) const;
    std::string TransferPropertyChangeTypeToString(ScreenPropertyChangeType type) const;
    void CheckAndSendHiSysEvent(const std::string& eventName, const std::string& bundleName) const;
    void HandlerSensor(ScreenPowerStatus status, PowerStateChangeReason reason);
    bool GetPowerStatus(ScreenPowerState state, PowerStateChangeReason reason, ScreenPowerStatus& status);
    DMError CheckDisplayMangerAgentTypeAndPermission(
        const sptr<IDisplayManagerAgent>& displayManagerAgent, DisplayManagerAgentType type);
    int Dump(int fd, const std::vector<std::u16string>& args) override;
    sptr<DisplayInfo> HookDisplayInfoByUid(sptr<DisplayInfo> displayInfo, const sptr<ScreenSession>& screenSession);
    DisplayId GetFakeDisplayId(sptr<ScreenSession> screenSession);
    DMError SetVirtualScreenSecurityExemption(ScreenId screenId, uint32_t pid,
        std::vector<uint64_t>& windowIdList) override;
    void GetInternalAndExternalSession(sptr<ScreenSession>& internalSession, sptr<ScreenSession>& externalSession);
#ifdef DEVICE_STATUS_ENABLE
    void SetDragWindowScreenId(ScreenId screenId, ScreenId displayNodeScreenId);
#endif // DEVICE_STATUS_ENABLE
    void ShowFoldStatusChangedInfo(int errCode, std::string& dumpInfo);
    void SetMirrorScreenIds(std::vector<ScreenId>& mirrorScreenIds);
    bool IsFreezed(const int32_t& agentPid, const DisplayManagerAgentType& agentType);
    void NotifyUnfreezed(const std::set<int32_t>& unfreezedPidList, const sptr<ScreenSession>& screenSession);
    void NotifyUnfreezedAgents(const int32_t& pid, const std::set<int32_t>& unfreezedPidList,
        const std::set<DisplayManagerAgentType>& pidAgentTypes, const sptr<ScreenSession>& screenSession);
    int NotifyPowerEventForDualDisplay(DisplayPowerEvent event, EventStatus status,
        PowerStateChangeReason reason);
    bool IsExtendMode();
    class ScreenIdManager {
    friend class ScreenSessionGroup;
    public:
        ScreenIdManager() = default;
        ~ScreenIdManager() = default;
        WM_DISALLOW_COPY_AND_MOVE(ScreenIdManager);
        ScreenId CreateAndGetNewScreenId(ScreenId rsScreenId);
        void UpdateScreenId(ScreenId rsScreenId, ScreenId smsScreenId);
        bool DeleteScreenId(ScreenId smsScreenId);
        bool HasRsScreenId(ScreenId smsScreenId) const;
        bool ConvertToRsScreenId(ScreenId, ScreenId&) const;
        ScreenId ConvertToRsScreenId(ScreenId) const;
        bool ConvertToSmsScreenId(ScreenId, ScreenId&) const;
        ScreenId ConvertToSmsScreenId(ScreenId) const;

    private:
        std::atomic<ScreenId> smsScreenCount_ { 1000 };
        std::map<ScreenId, ScreenId> rs2SmsScreenIdMap_;
        std::map<ScreenId, ScreenId> sms2RsScreenIdMap_;
        mutable std::shared_mutex screenIdMapMutex_;
    };

    EventTracker screenEventTracker_;
    RSInterfaces& rsInterface_;
    std::shared_ptr<TaskScheduler> taskScheduler_;
    std::shared_ptr<TaskScheduler> screenPowerTaskScheduler_;

    /*
     * multi user
     */
    std::mutex oldScbPidsMutex_;
    std::condition_variable scbSwitchCV_;
    int32_t currentUserId_ { 0 };
    int32_t currentScbPId_ { -1 };
    std::vector<int32_t> oldScbPids_ {};
    std::map<int32_t, sptr<IScreenSessionManagerClient>> clientProxyMap_;
    FoldDisplayMode oldScbDisplayMode_ = FoldDisplayMode::UNKNOWN;

    sptr<IScreenSessionManagerClient> clientProxy_;
    ClientAgentContainer<IDisplayManagerAgent, DisplayManagerAgentType> dmAgentContainer_;
    DeviceScreenConfig deviceScreenConfig_;
    std::vector<DisplayPhysicalResolution> allDisplayPhysicalResolution_ {};
    std::map<int32_t, std::set<DisplayManagerAgentType>> pidAgentTypeMap_;
    std::vector<float> lastFoldAngles_ {};
    sptr<DisplayChangeInfo> lastDisplayChangeInfo_;
    ScreenChangeEvent lastScreenChangeEvent_ = ScreenChangeEvent::UNKNOWN;
    std::mutex lastStatusUpdateMutex_;

    mutable std::recursive_mutex screenSessionMapMutex_;
    std::map<ScreenId, sptr<ScreenSession>> screenSessionMap_;
    std::recursive_mutex mutex_;
    std::recursive_mutex displayInfoMutex_;
    std::shared_mutex hookInfoMutex_;

    ScreenId defaultScreenId_ = SCREEN_ID_INVALID;
    ScreenIdManager screenIdManager_;

    std::atomic<ScreenId> defaultRsScreenId_ { SCREEN_ID_INVALID };
    std::map<sptr<IRemoteObject>, std::vector<ScreenId>> screenAgentMap_;
    std::map<ScreenId, sptr<ScreenSessionGroup>> smsScreenGroupMap_;
    std::map<uint32_t, DMHookInfo> displayHookMap_;

    bool isAutoRotationOpen_ = false;
    bool isExpandCombination_ = false;
    bool isScreenShot_ = false;
    bool isCoordinationFlag_ = false;
    uint32_t hdmiScreenCount_ = 0;
    uint32_t virtualScreenCount_ = 0;
    uint32_t currentExpandScreenCount_ = 0;
    sptr<AgentDeathRecipient> deathRecipient_ { nullptr };

    sptr<SessionDisplayPowerController> sessionDisplayPowerController_;
    sptr<ScreenCutoutController> screenCutoutController_;
    sptr<FoldScreenController> foldScreenController_;

    bool isDensityDpiLoad_ = false;
    float densityDpi_ { 1.0f };
    float subDensityDpi_ { 1.0f };
    std::atomic<uint32_t> cachedSettingDpi_ {0};

    uint32_t defaultDpi {0};
    uint32_t extendDefaultDpi_ {0};
    uint32_t defaultDeviceRotationOffset_ { 0 };

    bool isMultiScreenCollaboration_ = false;
    bool screenPrivacyStates = false;
    bool keyguardDrawnDone_ = true;
    bool needScreenOnWhenKeyguardNotify_ = false;
    bool gotScreenOffNotify_ = false;
    bool needScreenOffNotify_ = false;

    std::mutex screenOnMutex_;
    std::condition_variable screenOnCV_;
    std::mutex screenOffMutex_;
    std::condition_variable screenOffCV_;
    int32_t screenOffDelay_ {0};
    std::vector<ScreenId> mirrorScreenIds_;
    std::mutex snapBypickerMutex_;

    std::mutex freezedPidListMutex_;
    std::set<int32_t> freezedPidList_;

    std::atomic<PowerStateChangeReason> prePowerStateChangeReason_ =
        PowerStateChangeReason::STATE_CHANGE_REASON_UNKNOWN;
    std::atomic<PowerStateChangeReason> lastWakeUpReason_ = PowerStateChangeReason::STATE_CHANGE_REASON_INIT;
    std::atomic<PowerStateChangeReason> currentWakeUpReason_ = PowerStateChangeReason::STATE_CHANGE_REASON_INIT;
    std::atomic<bool> isScreenLockSuspend_ = false;
    std::atomic<bool> gotScreenlockFingerprint_ = false;
    std::atomic<bool> isPhyScreenConnected_ = false;

    // Fold Screen
    std::map<ScreenId, ScreenProperty> phyScreenPropMap_;
    mutable std::recursive_mutex phyScreenPropMapMutex_;
    static void BootFinishedCallback(const char *key, const char *value, void *context);
    std::function<void()> foldScreenPowerInit_ = nullptr;
    void HandleFoldScreenPowerInit();
    void SetFoldScreenPowerInit(std::function<void()> foldScreenPowerInit);
    void SetDpiFromSettingData();
    void SetDpiFromSettingData(bool isInternal);
    void SetRotateLockedFromSettingData();
    void NotifyClientProxyUpdateFoldDisplayMode(FoldDisplayMode displayMode);
    void UpdateDisplayScaleState(ScreenId screenId);
    void SetDisplayScaleInner(ScreenId screenId, const float& scaleX, const float& scaleY, const float& pivotX,
                                  const float& pivotY);
    void CalcDisplayNodeTranslateOnFoldableRotation(sptr<ScreenSession>& session, const float& scaleX,
                                                   const float& scaleY, const float& pivotX, const float& pivotY,
                                                   float& translateX, float& translateY);
    void CalcDisplayNodeTranslateOnRotation(sptr<ScreenSession>& session, const float& scaleX, const float& scaleY,
                                            const float& pivotX, const float& pivotY, float& translateX,
                                            float& translateY);
    void RegisterApplicationStateObserver();
    void SetPostureAndHallSensorEnabled();
    bool IsDefaultMirrorMode(ScreenId screenId);
    void SetCastFromSettingData();
    void RegisterCastObserver(std::vector<ScreenId>& mirrorScreenIds);
    void ExitCoordination(const std::string& reason);
    void UpdateDisplayState(std::vector<ScreenId> screenIds, DisplayState state);
    DisplayState lastDisplayState_ { DisplayState::UNKNOWN };

private:
    class ScbClientListenerDeathRecipient : public IRemoteObject::DeathRecipient {
        public:
            explicit ScbClientListenerDeathRecipient(int32_t scbPid)
                : scbPid_(scbPid)
            {}

            void OnRemoteDied(const wptr<IRemoteObject>& wptrDeath) override
            {
                ScreenSessionManager::GetInstance().ScbClientDeathCallback(scbPid_);
            }

        private:
            int32_t scbPid_;
    };
};
} // namespace OHOS::Rosen

#endif // OHOS_ROSEN_WINDOW_SCENE_SCREEN_SESSION_MANAGER_H
