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

#ifndef OHOS_ROSEN_SCREEN_SESSION_MANAGER_CLIENT_H
#define OHOS_ROSEN_SCREEN_SESSION_MANAGER_CLIENT_H

#include <map>
#include <mutex>
#include <set>

#include <common/rs_rect.h>

#include "display_change_listener.h"
#include "display_change_info.h"
#include "dm_common.h"
#include "session/screen/include/screen_session.h"
#include "ffrt_queue_helper.h"
#include "interfaces/include/ws_common.h"
#include "wm_single_instance.h"
#include "zidl/screen_session_manager_client_stub.h"
#include "zidl/screen_session_manager_interface.h"

namespace OHOS::Rosen {
using ScreenInfoChangeClientListener = std::function<void(uint64_t)>;

class IScreenConnectionListener {
public:
    virtual void OnScreenConnected(const sptr<ScreenSession>& screenSession) = 0;
    virtual void OnScreenDisconnected(const sptr<ScreenSession>& screenSession) = 0;
};

class IScreenConnectionChangeListener : public RefBase {
public:
    virtual void OnScreenConnected(const sptr<ScreenSession>& screenSession) = 0;
    virtual void OnScreenDisconnected(const sptr<ScreenSession>& screenSession) = 0;
};

class ITentModeListener {
public:
    virtual void OnTentModeChange(const TentMode tentMode) = 0;
};

class ScreenSessionManagerClient : public ScreenSessionManagerClientStub {
WM_DECLARE_SINGLE_INSTANCE_BASE(ScreenSessionManagerClient)

public:
    void RegisterScreenConnectionListener(IScreenConnectionListener* listener);
    void RegisterDisplayChangeListener(const sptr<IDisplayChangeListener>& listener);
    void RegisterScreenConnectionChangeListener(const sptr<IScreenConnectionChangeListener>& listener);
    void ExtraDestroyScreen(ScreenId screenId);

    sptr<ScreenSession> GetScreenSession(ScreenId screenId) const;
    sptr<ScreenSession> GetScreenSessionExtra(ScreenId screenId) const;
    std::map<ScreenId, ScreenProperty> GetAllScreensProperties() const;
    FoldDisplayMode GetFoldDisplayMode() const;

    void UpdateScreenRotationProperty(ScreenId screenId, const RRect& bounds, ScreenDirectionInfo directionInfo,
        ScreenPropertyChangeType screenPropertyChangeType);
    uint32_t GetCurvedCompressionArea();
    ScreenProperty GetPhyScreenProperty(ScreenId screenId);
    void SetScreenPrivacyState(bool hasPrivate);
    void SetPrivacyStateByDisplayId(std::unordered_map<DisplayId, bool>& privacyBundleDisplayId);
    void SetScreenPrivacyWindowList(std::unordered_map<DisplayId, std::vector<std::string>>& privacyBundleList);
    void NotifyDisplayChangeInfoChanged(const sptr<DisplayChangeInfo>& info);
    void OnDisplayStateChanged(DisplayId defaultDisplayId, sptr<DisplayInfo> displayInfo,
        const std::map<DisplayId, sptr<DisplayInfo>>& displayInfoMap, DisplayStateChangeType type) override;
    void OnScreenshot(DisplayId displayId) override;
    void OnImmersiveStateChanged(ScreenId screenId, bool& immersive) override;
    void OnGetSurfaceNodeIdsFromMissionIdsChanged(std::vector<uint64_t>& missionIds,
        std::vector<uint64_t>& surfaceNodeIds, const std::vector<uint32_t>& needWindowTypeList = {},
        bool isNeedForceCheck = false) override;
    void OnSetSurfaceNodeIdsChanged(DisplayId displayId, const std::vector<uint64_t>& surfaceNodeIds) override;
    void OnVirtualScreenDisconnected(DisplayId displayId) override;
    void OnUpdateFoldDisplayMode(FoldDisplayMode displayMode) override;
    void UpdateAvailableArea(ScreenId screenId, DMRect area);
    void UpdateSuperFoldAvailableArea(ScreenId screenId, DMRect bArea, DMRect cArea);
    void UpdateSuperFoldExpandAvailableArea(ScreenId screenId, DMRect area);
    int32_t SetScreenOffDelayTime(int32_t delay);
    int32_t SetScreenOnDelayTime(int32_t delay);
    void SetCameraStatus(int32_t cameraStatus, int32_t cameraPosition);
    void NotifyFoldToExpandCompletion(bool foldToExpand);
    void NotifyScreenConnectCompletion(ScreenId screenId);
    void NotifyAodOpCompletion(AodOP operation, int32_t result);
    void RecordEventFromScb(std::string description, bool needRecordEvent);
    FoldStatus GetFoldStatus();
    SuperFoldStatus GetSuperFoldStatus();
    float GetSuperRotation();
    void SetLandscapeLockStatus(bool isLocked);
    ExtendScreenConnectStatus GetExtendScreenConnectStatus();
    std::shared_ptr<Media::PixelMap> GetScreenSnapshot(ScreenId screenId, float scaleX, float scaleY);
    DeviceScreenConfig GetDeviceScreenConfig();
    sptr<ScreenSession> GetScreenSessionById(const ScreenId id);
    ScreenId GetDefaultScreenId();
    bool IsFoldable();
    void SetVirtualPixelRatioSystem(ScreenId screenId, float virtualPixelRatio) override;
    void UpdateDisplayHookInfo(int32_t uid, bool enable, const DMHookInfo& hookInfo);
    void GetDisplayHookInfo(int32_t uid, DMHookInfo& hookInfo) const;
    void NotifyIsFullScreenInForceSplitMode(int32_t uid, bool isFullScreen);
    void SetForceCloseHdr(ScreenId screenId, bool isForceCloseHdr);

    void RegisterSwitchingToAnotherUserFunction(std::function<void()>&& func);
    void SwitchingCurrentUser();
    void DisconnectAllExternalScreen();
    void SwitchUserCallback(std::vector<int32_t> oldScbPids, int32_t currentScbPid) override;

    void OnFoldStatusChangedReportUE(const std::vector<std::string>& screenFoldInfo) override;

    void UpdateDisplayScale(ScreenId id, float scaleX, float scaleY, float pivotX, float pivotY, float translateX,
                            float translateY);
    bool OnExtendDisplayNodeChange(ScreenId firstId, ScreenId secondId) override;
    bool OnCreateScreenSessionOnly(ScreenId screenId, ScreenId rsId,
        const std::string& name, bool isExtend) override;
    bool OnMainDisplayNodeChange(ScreenId mainScreenId, ScreenId extendScreenId, ScreenId extendRSId) override;
    void SetScreenCombination(ScreenId mainScreenId, ScreenId extendScreenId,
        ScreenCombination extendCombination) override;
    std::string OnDumperClientScreenSessions() override;
    void SetDefaultMultiScreenModeWhenSwitchUser();
    void NotifyExtendScreenCreateFinish();
    void NotifyExtendScreenDestroyFinish();
    void NotifyScreenMaskAppear();
    void NotifySwitchUserAnimationFinish(const std::string& description);
    void NotifySwitchUserAnimationFinishByWindow();
    void RegisterSwitchUserAnimationNotification(const std::string& description);
    void OnAnimationFinish() override;
    void OnTentModeChange(TentMode tentMode) override;
    void SetInternalClipToBounds(ScreenId screenId, bool clipToBounds) override;
    DMError SetPrimaryDisplaySystemDpi(float dpi);
    void FreezeScreen(ScreenId screenId, bool isFreeze);
    std::shared_ptr<Media::PixelMap> GetScreenSnapshotWithAllWindows(ScreenId screenId, float scaleX, float scaleY,
        bool isNeedCheckDrmAndSurfaceLock);
    void OnScreenPropertyChanged(ScreenId screenId, float rotation, RRect bounds);
    bool OnFoldPropertyChange(ScreenId screenId, const ScreenProperty& property,
        ScreenPropertyChangeReason reason, FoldDisplayMode displayMode, ScreenProperty& midProperty) override;
    void RegisterTentModeChangeListener(ITentModeListener* listener);

    /*
     * RS Client Multi Instance
     */
    std::shared_ptr<RSUIDirector> GetRSUIDirector(ScreenId screenId);
    std::shared_ptr<RSUIContext> GetRSUIContext(ScreenId screenId);
    bool GetSupportsFocus(DisplayId displayId);

protected:
    ScreenSessionManagerClient() = default;
    virtual ~ScreenSessionManagerClient() = default;

private:
    void ConnectToServer();
    bool CheckIfNeedConnectScreen(SessionOption option);
    void OnScreenConnectionChanged(SessionOption option, ScreenEvent screenEvent) override;
    bool HandleScreenConnection(SessionOption option);
    void HandleScreenDisconnectEvent(SessionOption option, ScreenEvent screenEvent);
    bool HandleScreenDisconnection(SessionOption option);
    void NotifyClientScreenConnect(sptr<ScreenSession>& screenSession);
    void OnPropertyChanged(ScreenId screenId,
        const ScreenProperty& property, ScreenPropertyChangeReason reason) override;
    void OnPowerStatusChanged(DisplayPowerEvent event, EventStatus status,
        PowerStateChangeReason reason) override;
    void OnSensorRotationChanged(ScreenId screenId, float sensorRotation, bool isSwitchUser) override;
    void OnHoverStatusChanged(ScreenId screenId, int32_t hoverStatus, bool needRotate = true) override;
    void OnScreenOrientationChanged(ScreenId screenId, float screenOrientation) override;
    void OnScreenRotationLockedChanged(ScreenId screenId, bool isLocked) override;
    void OnCameraBackSelfieChanged(ScreenId screenId, bool isCameraBackSelfie) override;
    void OnScreenExtendChanged(ScreenId mainScreenId, ScreenId extendScreenId) override;
    void OnSuperFoldStatusChanged(ScreenId screenId, SuperFoldStatus superFoldStatus) override;
    void OnSecondaryReflexionChanged(ScreenId screenId, bool isSecondaryReflexion) override;
    void OnExtendScreenConnectStatusChanged(ScreenId screenId,
        ExtendScreenConnectStatus extendScreenConnectStatus) override;
    void OnBeforeScreenPropertyChanged(FoldStatus foldStatus) override;
    void OnScreenModeChanged(ScreenModeChangeEvent screenModeChangeEvent) override;

    void SetDisplayNodeScreenId(ScreenId screenId, ScreenId displayNodeScreenId) override;
    void ScreenCaptureNotify(ScreenId mainScreenId, int32_t uid, const std::string& clientName) override;

    void NotifyScreenConnect(const sptr<ScreenSession>& screenSession);
    void NotifyScreenDisconnect(const sptr<ScreenSession>& screenSession);
    void UpdatePropertyWhenSwitchUser(const sptr <ScreenSession>& screenSession,
        float rotation, RRect bounds, ScreenId screenId);
    sptr<ScreenSession> CreateTempScreenSession(
        ScreenId screenId, ScreenId rsId, const std::shared_ptr<RSDisplayNode>& displayNode);

    void UpdateWidthAndHeight(const sptr<ScreenSession>& screenSession, const RRect* bounds, ScreenId screenId);

    mutable std::mutex screenSessionMapMutex_;
    std::map<ScreenId, sptr<ScreenSession>> screenSessionMap_;
    std::map<ScreenId, sptr<ScreenSession>> extraScreenSessionMap_;
    std::function<void()> switchingToAnotherUserFunc_ = nullptr;

    sptr<IScreenSessionManager> screenSessionManager_;

    std::shared_ptr<FfrtQueueHelper> ffrtQueueHelper_ = std::make_shared<FfrtQueueHelper>();

    IScreenConnectionListener* screenConnectionListener_;
    ITentModeListener* tentModeListener_;
    std::atomic<TentMode> tentMode_ = TentMode::UNKNOWN;
    sptr<IScreenConnectionChangeListener> screenConnectionChangeListener_;
    sptr<IDisplayChangeListener> displayChangeListener_;
    FoldDisplayMode displayMode_ = FoldDisplayMode::UNKNOWN;
    SuperFoldStatus currentstate_ = SuperFoldStatus::UNKNOWN;

    std::mutex screenEventMutex_;
    std::unordered_set<ScreenId> connectedScreenSet_;
    std::set<std::string> animateFinishDescriptionSet_;
    std::set<std::string> animateFinishNotificationSet_;
    mutable std::shared_mutex animateFinishDescriptionSetMutex_;
    mutable std::mutex animateFinishNotificationSetMutex_;
};
} // namespace OHOS::Rosen

#endif // OHOS_ROSEN_SCREEN_SESSION_MANAGER_CLIENT_STUB_H
