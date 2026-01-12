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

#ifndef OHOS_ROSEN_WINDOW_SCENE_SESSION_H
#define OHOS_ROSEN_WINDOW_SCENE_SESSION_H

#include <list>
#include <mutex>
#include <shared_mutex>
#include <vector>

#include <event_handler.h>

#include "dm_common.h"
#include "interfaces/include/ws_common.h"
#include "layout_controller.h"
#include "occupied_area_change_info.h"
#include "pattern_detach_callback_interface.h"
#include "session/container/include/zidl/session_stage_interface.h"
#include "session/host/include/zidl/session_stub.h"
#include "session/host/include/scene_persistence.h"
#include "thread_safety_annotations.h"
#include "vsync_station.h"
#include "window_visibility_info.h"
#include "wm_common.h"
#include "floating_ball_template_info.h"

namespace OHOS::MMI {
class PointerEvent;
class KeyEvent;
class AxisEvent;
enum class WindowArea;
} // namespace OHOS::MMI

namespace OHOS::Media {
class PixelMap;
} // namespace OHOS::Media

namespace OHOS::Rosen {
class RSSurfaceNode;
class RSUIContext;
class RSTransaction;
class Session;
using NotifySessionRectChangeFunc = std::function<void(const WSRect& rect,
    SizeChangeReason reason, DisplayId displayId, const RectAnimationConfig& rectAnimationConfig)>;
using NotifySessionWindowLimitsChangeFunc = std::function<void(const WindowLimits& windowLimits)>;
using NotifySessionDisplayIdChangeFunc = std::function<void(uint64_t displayId)>;
using NotifyUpdateFloatingBallFunc = std::function<void(const FloatingBallTemplateInfo& fbTemplateInfo)>;
using NotifyStopFloatingBallFunc = std::function<void()>;
using NotifyRestoreFloatingBallMainWindowFunc = std::function<void(const std::shared_ptr<AAFwk::Want>& want)>;
using NotifyRestoreFloatMainWindowFunc = std::function<void(const std::shared_ptr<AAFwk::WantParams>& wantParameters)>;
using NotifyPendingSessionActivationFunc = std::function<void(SessionInfo& info)>;
using NotifyBatchPendingSessionsActivationFunc = std::function<void(std::vector<std::shared_ptr<SessionInfo>>& info,
    const std::vector<std::shared_ptr<PendingSessionActivationConfig>>& configs)>;
using NotifyChangeSessionVisibilityWithStatusBarFunc = std::function<void(const SessionInfo& info, bool visible)>;
using NotifySessionStateChangeFunc = std::function<void(const SessionState& state)>;
using NotifyBufferAvailableChangeFunc = std::function<void(const bool isAvailable, bool startWindowInvisible)>;
using NotifySessionStateChangeNotifyManagerFunc = std::function<void(int32_t persistentId, const SessionState& state)>;
using NotifyRequestFocusStatusNotifyManagerFunc =
    std::function<void(int32_t persistentId, const bool isFocused, const bool byForeground, FocusChangeReason reason)>;
using NotifyBackPressedFunc = std::function<void(const bool needMoveToBackground)>;
using NotifySessionFocusableChangeFunc = std::function<void(const bool isFocusable)>;
using NotifySessionTouchableChangeFunc = std::function<void(const bool touchable)>;
using NotifyClickFunc = std::function<void(bool requestFocus, bool isClick)>;
using NotifyTerminateSessionFunc = std::function<void(const SessionInfo& info)>;
using NotifyTerminateSessionFuncNew =
    std::function<void(const SessionInfo& info, bool needStartCaller, bool isFromBroker, bool isForceClean)>;
using NotifyTerminateSessionFuncTotal = std::function<void(const SessionInfo& info, TerminateType terminateType)>;
using NofitySessionLabelUpdatedFunc = std::function<void(const std::string& label)>;
using NofitySessionIconUpdatedFunc = std::function<void(const std::string& iconPath)>;
using NotifySessionExceptionFunc =
    std::function<void(const SessionInfo& info, const ExceptionInfo& exceptionInfo, bool startFail)>;
using NotifySessionSnapshotFunc = std::function<void(const int32_t& persistentId)>;
using NotifyPendingSessionToForegroundFunc = std::function<void(const SessionInfo& info)>;
using NotifyPendingSessionToBackgroundFunc = std::function<void(const SessionInfo& info,
    const BackgroundParams& params)>;
using NotifyPendingSessionToBackgroundForDelegatorFunc = std::function<void(const SessionInfo& info,
    bool shouldBackToCaller)>;
using NotifyClickModalWindowOutsideFunc = std::function<void()>;
using NotifyRaiseMainWindowAboveTargetFunc = std::function<void(int32_t targetId)>;
using NotifyRaiseToTopForPointDownFunc = std::function<void()>;
using NotifyUIRequestFocusFunc = std::function<void()>;
using NotifyUILostFocusFunc = std::function<void()>;
using NotifySessionInfoLockedStateChangeFunc = std::function<void(const bool lockedState)>;
using NotifyDisplayIdChangedNotifyManagerFunc = std::function<void(int32_t persistentId, uint64_t displayId)>;
using GetStateFromManagerFunc = std::function<bool(const ManagerState key)>;
using NotifySystemSessionPointerEventFunc = std::function<void(std::shared_ptr<MMI::PointerEvent> pointerEvent)>;
using NotifySessionInfoChangeNotifyManagerFunc = std::function<void(int32_t persistentid)>;
using NotifySessionPropertyChangeNotifyManagerFunc =
    std::function<void(int32_t persistentid, WindowInfoKey windowInfoKey)>;
using NotifySystemSessionKeyEventFunc = std::function<bool(std::shared_ptr<MMI::KeyEvent> keyEvent,
    bool isPreImeEvent)>;
using NotifyContextTransparentFunc = std::function<void()>;
using NotifyFrameLayoutFinishFunc = std::function<void()>;
using NotifyClientDisplayIdChangeFunc = std::function<void(uint32_t windowId)>;
using VisibilityChangedDetectFunc = std::function<void(int32_t pid, bool isVisible, bool newIsVisible)>;
using AcquireRotateAnimationConfigFunc = std::function<void(RotateAnimationConfig& config)>;
using RequestVsyncFunc = std::function<void(const std::shared_ptr<VsyncCallback>& callback)>;
using NotifyWindowMovingFunc = std::function<void(DisplayId displayId, int32_t pointerX, int32_t pointerY)>;
using UpdateTransitionAnimationFunc = std::function<void(WindowTransitionType type, TransitionAnimation animation)>;
using NofitySessionLabelAndIconUpdatedFunc =
    std::function<void(const std::string& label, const std::shared_ptr<Media::PixelMap>& icon)>;
using NotifySessionGetTargetOrientationConfigInfoFunc = std::function<void(uint32_t targetOrientation)>;
using NotifyKeyboardStateChangeFunc = std::function<void(SessionState state, const KeyboardEffectOption& effectOption,
    const uint32_t callingSessionId, const DisplayId targetDisplayId)>;
using NotifyHighlightChangeFunc = std::function<void(bool isHighlight)>;
using NotifySurfaceBoundsChangeFunc = std::function<void(const WSRect& rect, bool isGlobal, bool needFlush)>;
using HasRequestedVsyncFunc = std::function<WSError(bool& hasRequestedVsync)>;
using RequestNextVsyncWhenModeChangeFunc = std::function<WSError(const std::shared_ptr<VsyncCallback>& vsyncCallback)>;
using NotifyClearSubSessionFunc = std::function<void(const int32_t subPersistentId)>;
using OutlineParamsChangeCallbackFunc = std::function<void(bool enabled, const OutlineStyleParams& outlineStyleParams)>;
using NotifyRestartAppFunc = std::function<void(const SessionInfo& info, int32_t callingPid)>;
using ProcessCallingSessionIdChangeFunc = std::function<void(uint32_t callingSessionId)>;
using GetRsCmdBlockingCountFunc = std::function<int32_t()>;
class ILifecycleListener {
public:
    virtual void OnActivation() {}
    virtual void OnConnect() {}
    virtual void OnForeground() {}
    virtual void OnBackground() {}
    virtual void OnDisconnect() {}
    virtual void OnLayoutFinished() {}
    virtual void OnRemoveBlank() {}
    virtual void OnAddSnapshot() {}
    virtual void OnRemoveSnapshot() {}
    virtual void OnDrawingCompleted() {}
    virtual void OnExtensionDied() {}
    virtual void OnExtensionDetachToDisplay() {}
    virtual void OnExtensionTimeout(int32_t errorCode) {}
    virtual void OnAccessibilityEvent(const Accessibility::AccessibilityEventInfo& info,
        int64_t uiExtensionIdLevel) {}
    virtual void OnAppRemoveStartingWindow() {}
    virtual void OnUpdateSnapshotWindow() {}
    virtual void OnPreLoadStartingWindowFinished() {}
    virtual void OnRestart() {}
};

enum class LifeCycleTaskType : uint32_t {
    START,
    STOP
};

enum class DetectTaskState : uint32_t {
    NO_TASK,
    ATTACH_TASK,
    DETACH_TASK
};

struct DetectTaskInfo {
    WindowMode taskWindowMode = WindowMode::WINDOW_MODE_UNDEFINED;
    DetectTaskState taskState = DetectTaskState::NO_TASK;
};

struct ControlInfo {
    bool isNeedControl;
    bool isControlRecentOnly;
};

const std::string ATTACH_EVENT_NAME { "wms::ReportWindowTimeout_Attach" };
const std::string DETACH_EVENT_NAME { "wms::ReportWindowTimeout_Detach" };

class Session : public SessionStub {
public:
    friend class HidumpController;
    using Task = std::function<void()>;
    explicit Session(const SessionInfo& info);
    virtual ~Session();
    bool isKeyboardPanelEnabled_ = false;
    virtual void SetEventHandler(const std::shared_ptr<AppExecFwk::EventHandler>& handler,
        const std::shared_ptr<AppExecFwk::EventHandler>& exportHandler = nullptr);

    /*
     * Window LifeCycle
     */
    virtual WSError ConnectInner(const sptr<ISessionStage>& sessionStage, const sptr<IWindowEventChannel>& eventChannel,
        const std::shared_ptr<RSSurfaceNode>& surfaceNode, SystemSessionConfig& systemConfig,
        sptr<WindowSessionProperty> property = nullptr, sptr<IRemoteObject> token = nullptr,
        int32_t pid = -1, int32_t uid = -1, const std::string& identityToken = "") REQUIRES(SCENE_GUARD);
    WSError Foreground(sptr<WindowSessionProperty> property, bool isFromClient = false,
        const std::string& identityToken = "") override;
    WSError Background(bool isFromClient = false, const std::string& identityToken = "") override;
    WSError Disconnect(bool isFromClient = false, const std::string& identityToken = "") override REQUIRES(SCENE_GUARD);
    WSError Show(sptr<WindowSessionProperty> property) override;
    WSError Hide() override;
    WSError DrawingCompleted() override;
    void ResetSessionConnectState() REQUIRES(SCENE_GUARD);
    void ResetIsActive();
    WSError PendingSessionToForeground();
    WSError PendingSessionToBackground(const BackgroundParams& params);
    WSError PendingSessionToBackgroundForDelegator(bool shouldBackToCaller);
    bool RegisterLifecycleListener(const std::shared_ptr<ILifecycleListener>& listener);
    bool UnregisterLifecycleListener(const std::shared_ptr<ILifecycleListener>& listener);
    void SetPendingSessionActivationEventListener(NotifyPendingSessionActivationFunc&& func);
    void SetBatchPendingSessionsActivationEventListener(NotifyBatchPendingSessionsActivationFunc&& func);
    void SetTerminateSessionListener(NotifyTerminateSessionFunc&& func);
    void SetTerminateSessionListenerNew(NotifyTerminateSessionFuncNew&& func);
    void SetSessionExceptionListener(NotifySessionExceptionFunc&& func, bool fromJsScene);
    void SetTerminateSessionListenerTotal(NotifyTerminateSessionFuncTotal&& func);
    void SetBackPressedListenser(NotifyBackPressedFunc&& func);
    void SetPendingSessionToForegroundListener(NotifyPendingSessionToForegroundFunc&& func);
    void SetPendingSessionToBackgroundListener(NotifyPendingSessionToBackgroundFunc&& func);
    void SetPendingSessionToBackgroundForDelegatorListener(NotifyPendingSessionToBackgroundForDelegatorFunc&& func);
    void SetSessionSnapshotListener(const NotifySessionSnapshotFunc& func);
    WSError TerminateSessionNew(const sptr<AAFwk::SessionInfo> info, bool needStartCaller, bool isFromBroker);
    WSError TerminateSessionTotal(const sptr<AAFwk::SessionInfo> info, TerminateType terminateType);
    std::string GetSessionLabel() const;
    void SetRestartAppListener(NotifyRestartAppFunc&& func);

    /*
     * App Use Control
     */
    virtual bool GetIsUseControlSession() const { return false; }
    virtual void SetIsUseControlSession(bool isUseControlSession) {}
    virtual void NotifyUpdateAppUseControl(ControlAppType type, const ControlInfo& controlInfo) {}

    /*
     * Window Recover
     */
    WSError Reconnect(const sptr<ISessionStage>& sessionStage, const sptr<IWindowEventChannel>& eventChannel,
        const std::shared_ptr<RSSurfaceNode>& surfaceNode, sptr<WindowSessionProperty> property = nullptr,
        sptr<IRemoteObject> token = nullptr, int32_t pid = -1, int32_t uid = -1) REQUIRES(SCENE_GUARD);

    /*
     * Callbacks for ILifecycleListener
     */
    void NotifyActivation();
    void NotifyConnect();
    void NotifyForeground();
    void NotifyBackground();
    void NotifyDisconnect();
    void NotifyLayoutFinished();
    void NotifyRemoveBlank();
    void NotifyAddSnapshot(bool useFfrt = false, bool needPersist = false, bool needSaveSnapshot = true);
    void NotifyRemoveSnapshot();
    void NotifyUpdateSnapshotWindow();
    void NotifyPreLoadStartingWindowFinished();
    void NotifyRestart();
    void NotifyExtensionDied() override;
    void NotifyExtensionTimeout(int32_t errorCode) override;
    void NotifyTransferAccessibilityEvent(const Accessibility::AccessibilityEventInfo& info,
        int64_t uiExtensionIdLevel) override;
    void NotifyExtensionDetachToDisplay() override;

    /*
     * Window Immersive
     */
    void UpdateStatusBarVisible(bool isStatusBarVisible) { isStatusBarVisible_ = isStatusBarVisible; }
    bool IsStatusBarVisible() const;

    /*
     * Cross Display Move Drag
     */
    std::shared_ptr<RSSurfaceNode> GetSurfaceNodeForMoveDrag() const;

    virtual WSError TransferPointerEvent(const std::shared_ptr<MMI::PointerEvent>& pointerEvent,
        bool needNotifyClient = true, bool isExecuteDelayRaise = false);
    virtual WSError TransferKeyEvent(const std::shared_ptr<MMI::KeyEvent>& keyEvent);

    virtual WSError NotifyClientToUpdateRect(const std::string& updateReason,
        std::shared_ptr<RSTransaction> rsTransaction) { return WSError::WS_OK; }
    WSError TransferBackPressedEventForConsumed(bool& isConsumed);
    WSError TransferKeyEventForConsumed(const std::shared_ptr<MMI::KeyEvent>& keyEvent, bool& isConsumed,
        bool isPreImeEvent = false);
    WSError TransferFocusActiveEvent(bool isFocusActive);
    WSError TransferFocusStateEvent(bool focusState);
    virtual WSError UpdateAvoidArea(const sptr<AvoidArea>& avoidArea, AvoidAreaType type) { return WSError::WS_OK; }

    int32_t GetPersistentId() const;
    int32_t GetCurrentRotation() const;
    void SetSurfaceNode(const std::shared_ptr<RSSurfaceNode>& surfaceNode);
    std::shared_ptr<RSSurfaceNode> GetSurfaceNode() const;
    std::shared_ptr<RSSurfaceNode> GetSurfaceNode(bool isUpdateContextBeforeGet);
    std::optional<NodeId> GetSurfaceNodeId() const;
    void SetLeashWinSurfaceNode(std::shared_ptr<RSSurfaceNode> leashWinSurfaceNode);
    std::shared_ptr<RSSurfaceNode> GetLeashWinSurfaceNode() const;

    /*
     * Window Scene Snapshot
     */
    std::shared_ptr<Media::PixelMap> GetSnapshot() const;
    std::shared_ptr<Media::PixelMap> GetPreloadSnapshot() const;
    std::shared_ptr<Media::PixelMap> Snapshot(
        bool runInFfrt = false, float scaleParam = 0.0f, bool useCurWindow = false) const;
    void ResetSnapshot();
    void RenameSnapshotFromOldPersistentId(int32_t oldPersistentId);
    void SaveSnapshot(bool useFfrt, bool needPersist = true,
        std::shared_ptr<Media::PixelMap> persistentPixelMap = nullptr, bool updateSnapshot = false,
        LifeCycleChangeReason reason = LifeCycleChangeReason::DEFAULT);
    bool CheckSurfaceNodeForSnapshot(std::shared_ptr<RSSurfaceNode> surfaceNode) const;
    bool GetNeedUseBlurSnapshot() const;
    void UpdateAppLockSnapshot(ControlAppType type, ControlInfo controlInfo);
    virtual bool GetIsPrivacyMode() const { return false; };
    void SetSnapshotPrivacyMode(bool privacyMode) { snapshotPrivacyMode_.store(privacyMode); };
    bool GetSnapshotPrivacyMode() const;
    std::atomic<bool> snapshotPrivacyMode_ { false };

    virtual void SetAppControlInfo(ControlAppType type, ControlInfo controlInfo) {};
    virtual bool GetAppControlInfo(ControlAppType type, ControlInfo& controlInfo) const
    {
        controlInfo = { .isNeedControl = false, .isControlRecentOnly = false };
        return false;
    };
    bool GetAppLockControl() const { return isAppLockControl_.load(); };
    void SetSaveSnapshotCallback(Task&& task)
    {
        if (task) {
            std::lock_guard lock(saveSnapshotCallbackMutex_);
            saveSnapshotCallback_ = std::move(task);
        }
    }
    void SetAddSnapshotCallback(Task&& task)
    {
        if (task) {
            std::lock_guard lock(addSnapshotCallbackMutex_);
            addSnapshotCallback_ = std::move(task);
        }
    }
    void SetEnableAddSnapshot(bool enableAddSnapshot = true);
    bool GetEnableAddSnapshot() const;

    SessionState GetSessionState() const;
    virtual void SetSessionState(SessionState state);
    void SetSessionInfoSupportedWindowModes(
        const std::vector<AppExecFwk::SupportWindowMode>& updatedWindowModes);
    void SetSessionInfoAncoSceneState(int32_t ancoSceneState);
    void SetSessionInfoTime(const std::string& time);
    void SetSessionInfoAbilityInfo(const std::shared_ptr<AppExecFwk::AbilityInfo>& abilityInfo);
    void SetSessionInfoWant(const std::shared_ptr<AAFwk::Want>& want);
    void ResetSessionInfoResultCode();
    void SetSessionInfoPersistentId(int32_t persistentId);
    void SetSessionInfoCallerPersistentId(int32_t callerPersistentId);
    void SetSessionInfoContinueState(ContinueState state);
    void SetSessionInfoLockedState(bool lockedState);
    void SetSessionInfoIsClearSession(bool isClearSession);
    void SetSessionInfoAffinity(std::string affinity);
    void GetCloseAbilityWantAndClean(AAFwk::Want& outWant);
    void SetSessionInfo(const SessionInfo& info);
    void SetSessionInfoWindowInputType(uint32_t windowInputType);
    void SetSessionInfoExpandInputFlag(uint32_t expandInputFlag);
    uint32_t GetSessionInfoExpandInputFlag() const;
    void SetSessionInfoCursorDragCount(int32_t count);
    int32_t GetSessionInfoCursorDragCount();
    void SetSessionInfoCursorDragFlag(bool value);
    bool GetSessionInfoCursorDragFlag();
    void SetSessionInfoSeparationTouchEnabled(bool value);
    bool GetSessionInfoSeparationTouchEnabled();
    void SetSessionInfoReceiveDragEventEnabled(bool value);
    bool GetSessionInfoReceiveDragEventEnabled();
    void SetSessionInfoAdvancedFeatureFlag(uint32_t bitPosition, bool value);
    bool GetSessionInfoAdvancedFeatureFlag(uint32_t bitPosition);
    void SetSessionInfoWindowMode(int32_t windowMode);
    const SessionInfo& GetSessionInfo() const;
    SessionInfo& EditSessionInfo();
    DisplayId GetScreenId() const;
    virtual void SetScreenId(uint64_t screenId);
    WindowType GetWindowType() const;
    float GetAspectRatio() const;
    WSError SetAspectRatio(float ratio) override;
    WSError SetSessionProperty(const sptr<WindowSessionProperty>& property);
    WSError SetSessionPropertyForReconnect(const sptr<WindowSessionProperty>& property);
    const sptr<WindowSessionProperty>& GetSessionProperty() const { return property_; }
    void SetSessionRect(const WSRect& rect);
    WSRect GetSessionRect() const;
    WSRect GetSessionGlobalRect() const;
    WSRect GetSessionScreenRelativeRect() const;
    WSRect GetSessionGlobalRectInMultiScreen() const;
    WMError GetGlobalScaledRect(Rect& globalScaledRect) override;
    void SetSessionGlobalRect(const WSRect& rect);
    void SetSessionRequestRect(const WSRect& rect);
    WSRect GetSessionRequestRect() const;
    void SetRequestRectAnimationConfig(const RectAnimationConfig& rectAnimationConfig);
    RectAnimationConfig GetRequestRectAnimationConfig() const;
    std::string GetWindowName() const;
    WSRect GetLastLayoutRect() const;
    WSRect GetLayoutRect() const;
    bool GetSkipSelfWhenShowOnVirtualScreen() const;
    DisplayId GetDisplayId() const { return GetSessionProperty()->GetDisplayId(); }
    void SetRestartApp(bool restartApp);
    bool GetRestartApp() const;
    void SetRestartInSameProcess(bool restartInSameProcess);
    bool GetRestartInSameProcess() const;
    void SetRestartCallerPersistentId(int32_t restartCallerPersistentId);
    int32_t GetRestartCallerPersistentId() const;

    virtual WSError SetActive(bool active);
    virtual WSError UpdateSizeChangeReason(SizeChangeReason reason);
    SizeChangeReason GetSizeChangeReason() const { return layoutController_->GetSizeChangeReason(); }
    bool IsDraggingReason(SizeChangeReason reason) const;
    virtual WSError UpdateRect(const WSRect& rect, SizeChangeReason reason,
        const std::string& updateReason, const std::shared_ptr<RSTransaction>& rsTransaction = nullptr);
    virtual WSError UpdateRectWithLayoutInfo(const WSRect& rect, SizeChangeReason reason,
        const std::string& updateReason, const std::shared_ptr<RSTransaction>& rsTransaction = nullptr,
        const std::map<AvoidAreaType, AvoidArea>& avoidAreas = {});
    WSError UpdateDensity();
    WSError UpdateOrientation();
    virtual bool IsDragMoving() const { return false; }
    virtual bool IsDragZooming() const { return false; }

    void SetShowRecent(bool showRecent);
    void SetSystemActive(bool systemActive);
    bool GetShowRecent() const;
    void SetOffset(float x, float y);
    float GetOffsetX() const;
    float GetOffsetY() const;
    void SetBounds(const WSRectF& bounds);
    WSRectF GetBounds();
    void SetRotation(Rotation rotation);
    Rotation GetRotation() const;
    void SetBufferAvailable(bool bufferAvailable, bool startWindowInvisible = false);
    bool GetBufferAvailable() const;
    void SetNeedSnapshot(bool needSnapshot);
    virtual void SetExitSplitOnBackground(bool isExitSplitOnBackground);
    virtual bool IsExitSplitOnBackground() const;
    virtual bool NeedStartingWindowExitAnimation() const { return true; }
    bool IsExitSplitOnBackgroundRecover();
    int32_t EncodeSnapShotRecoverValue(DisplayOrientation rotate);
    int32_t DecodeSnapShotRecoverValue(int32_t snapShotRecoverValue, SnapShotRecoverType snapShotRecoverType);

    void SetChangeSessionVisibilityWithStatusBarEventListener(
        NotifyChangeSessionVisibilityWithStatusBarFunc&& func);
    // Just terminate, not clear session
    WSError Clear(bool needStartCaller = false, bool isForceClean = false);
    WSError SetSessionLabel(const std::string& label);
    void UpdateSessionLabel(const std::string& label);
    void SetUpdateSessionLabelListener(const NofitySessionLabelUpdatedFunc& func);
    WSError SetSessionIcon(const std::shared_ptr<Media::PixelMap>& icon);
    void SetUpdateSessionIconListener(const NofitySessionIconUpdatedFunc& func);
    void SetSessionStateChangeListenser(const NotifySessionStateChangeFunc& func);
    void SetBufferAvailableChangeListener(const NotifyBufferAvailableChangeFunc& func);
    virtual void UnregisterSessionChangeListeners();
    void SetSessionStateChangeNotifyManagerListener(const NotifySessionStateChangeNotifyManagerFunc& func);
    void SetSessionInfoChangeNotifyManagerListener(const NotifySessionInfoChangeNotifyManagerFunc& func);
    void SetSessionPropertyChangeNotifyManagerListener(const NotifySessionPropertyChangeNotifyManagerFunc& func);
    void SetDisplayIdChangedNotifyManagerListener(const NotifyDisplayIdChangedNotifyManagerFunc& func);
    void SetRequestFocusStatusNotifyManagerListener(const NotifyRequestFocusStatusNotifyManagerFunc& func);
    void SetNotifyUIRequestFocusFunc(const NotifyUIRequestFocusFunc& func);
    void SetNotifyUILostFocusFunc(const NotifyUILostFocusFunc& func);
    void SetGetStateFromManagerListener(const GetStateFromManagerFunc& func);
    void SetClearSubSessionCallback(const NotifyClearSubSessionFunc& func);

    void SetSystemConfig(const SystemSessionConfig& systemConfig);
    void SetSnapshotScale(const float snapshotScale);
    virtual WSError ProcessBackEvent(); // send back event to session_stage

    sptr<ScenePersistence> GetScenePersistence() const;
    void SetParentSession(const sptr<Session>& session);
    sptr<Session> GetParentSession() const;
    void BindDialogToParentSession(const sptr<Session>& session);
    void RemoveDialogToParentSession(const sptr<Session>& session);
    std::vector<sptr<Session>> GetDialogVector() const;
    void ClearDialogVector();
    WSError NotifyDestroy();
    WSError NotifyAppForceLandscapeConfigUpdated();
    WSError NotifyCloseExistPipWindow();

    void SetSessionFocusableChangeListener(const NotifySessionFocusableChangeFunc& func);
    void SetSessionTouchableChangeListener(const NotifySessionTouchableChangeFunc& func);
    void SetClickListener(const NotifyClickFunc& func);
    void NotifySessionFocusableChange(bool isFocusable);
    void NotifySessionTouchableChange(bool touchable);
    void NotifyClick(bool requestFocus = true, bool isClick = true);
    bool GetStateFromManager(const ManagerState key);
    virtual void PresentFocusIfNeed(int32_t pointerAcrion, int32_t sourceType = 0);
    virtual WSError UpdateWindowMode(WindowMode mode);
    WSError SetAppSupportPhoneInPc(bool isSupportPhone);
    WSError SetCompatibleModeProperty(const sptr<CompatibleModeProperty> compatibleModeProperty);
    WSError PcAppInPadNormalClose();
    WSError SetIsPcAppInPad(bool enable);
    WSError SetPcAppInpadCompatibleMode(bool enabled);
    WSError SetPcAppInpadSpecificSystemBarInvisible(bool isPcAppInpadSpecificSystemBarInvisible);
    WSError SetPcAppInpadOrientationLandscape(bool isPcAppInpadOrientationLandscape);
    WSError SetMobileAppInPadLayoutFullScreen(bool isMobileAppInPadLayoutFullScreen);
    bool NeedNotify() const;
    void SetNeedNotify(bool needNotify);
    WSError SetTouchable(bool touchable);
    bool GetTouchable() const;
    void SetForceTouchable(bool touchable);
    virtual void SetSystemTouchable(bool touchable);
    bool GetSystemTouchable() const;
    bool GetWindowTouchableForMMI(DisplayId displayId) const;
    virtual WSError SetRSVisible(bool isVisible);
    bool GetRSVisible() const;
    WSError SetVisibilityState(WindowVisibilityState state);
    WindowVisibilityState GetVisibilityState() const;
    WSError SetDrawingContentState(bool isRSDrawing);
    bool GetDrawingContentState() const;
    WSError SetBrightness(float brightness);
    float GetBrightness() const;
    void NotifyOccupiedAreaChangeInfo(sptr<OccupiedAreaChangeInfo> info,
        const std::shared_ptr<RSTransaction>& rsTransaction = nullptr,
        const Rect& callingSessionRect = {}, const std::map<AvoidAreaType, AvoidArea>& = {});
    void SetSessionInfoLockedStateChangeListener(const NotifySessionInfoLockedStateChangeFunc& func);
    void NotifySessionInfoLockedStateChange(bool lockedState);
    void SetContextTransparentFunc(const NotifyContextTransparentFunc& func);
    void NotifyContextTransparent();
    bool NeedCheckContextTransparent() const;

    /*
     * Window Layout
     */
    bool UpdateWindowModeSupportType(const std::shared_ptr<AppExecFwk::AbilityInfo>& abilityInfo);

    /*
     * Window Rotate Animation
     */
    void SetAcquireRotateAnimationConfigFunc(const AcquireRotateAnimationConfigFunc& func);

    /*
     * Window Focus
     */
    virtual WSError SetSystemSceneBlockingFocus(bool blocking);
    bool GetBlockingFocus() const;
    WSError SetFocusable(bool isFocusable);
    bool GetFocusable() const;
    void SetFocusedOnShow(bool focusedOnShow); // Used when creating ability
    bool IsFocusedOnShow() const;
    WSError SetFocusableOnShow(bool isFocusableOnShow); // Used when showing window
    bool IsFocusableOnShow() const;
    virtual void SetSystemFocusable(bool systemFocusable); // Used by SCB
    bool GetSystemFocusable() const;
    bool CheckFocusable() const;
    void SetStartingBeforeVisible(bool isStartingBeforeVisible);
    bool GetStartingBeforeVisible() const;
    bool IsFocused() const;
    bool GetFocused() const;
    bool IsNeedRequestToTop() const;
    void NotifyClickIfNeed();
    virtual WSError UpdateFocus(bool isFocused);
    virtual void PresentFocusIfPointDown();
    WSError RequestFocus(bool isFocused) override;
    void NotifyRequestFocusStatusNotifyManager(bool isFocused, bool byForeground = true,
        FocusChangeReason reason = FocusChangeReason::DEFAULT);
    void NotifyUIRequestFocus();
    virtual void NotifyUILostFocus();
    WSError NotifyFocusStatus(const sptr<FocusNotifyInfo>& focusNotifyInfo, bool isFocused);
    void SetExclusivelyHighlighted(bool isExclusivelyHighlighted);
    virtual WSError UpdateHighlightStatus(const sptr<HighlightNotifyInfo>& highlightNotifyInfo, bool isHighlight,
        bool needBlockHighlightNotify);
    WSError NotifyHighlightChange(const sptr<HighlightNotifyInfo>& highlightNotifyInfo, bool isHighlight);
    WSError GetIsHighlighted(bool& isHighlighted) override;
    WSError HandlePointerEventForFocus(const std::shared_ptr<MMI::PointerEvent>& pointerEvent,
        bool isExecuteDelayRaise = false);
    bool HasParentSessionWithToken(const sptr<IRemoteObject>& token);

    /*
     * Multi Window
     */
    void SetIsMidScene(bool isMidScene);
    bool GetIsMidScene() const;
    WSError GetIsMidScene(bool& isMidScene) override;

    /*
     * Keyboard Window
     */
    bool CheckEmptyKeyboardAvoidAreaIfNeeded() const;
    void SetKeyboardStateChangeListener(const NotifyKeyboardStateChangeFunc& func);
    uint32_t GetDirtyFlags() const { return dirtyFlags_; }

    bool IsSessionValid() const;
    bool IsActive() const;
    bool IsSystemActive() const;
    bool IsSystemSession() const;
    bool IsTerminated() const;
    bool IsSessionForeground() const;
    bool IsSessionNotBackground() const;
    virtual bool IsAnco() const { return false; }
    virtual void SetBlank(bool isAddBlank) {}
    virtual bool GetBlank() const { return false; }
    virtual bool GetBufferAvailableCallbackEnable() const { return false; }

    sptr<IRemoteObject> dialogTargetToken_ = nullptr;
    int32_t GetWindowId() const;
    void SetAppIndex(const int32_t appIndex);
    int32_t GetAppIndex() const;
    void SetCallingPid(int32_t id) REQUIRES(SCENE_GUARD);
    void SetCallingUid(int32_t id);
    int32_t GetCallingPid() const;
    int32_t GetCallingUid() const;
    void SetAbilityToken(sptr<IRemoteObject> token);
    sptr<IRemoteObject> GetAbilityToken() const;
    WindowMode GetWindowMode() const;
    void SetCallingSessionIdSessionListenser(const ProcessCallingSessionIdChangeFunc&& func);

    /*
     * Window ZOrder
     */
    virtual void SetZOrder(uint32_t zOrder);
    uint32_t GetZOrder() const;
    uint32_t GetLastZOrder() const;

    void SetUINodeId(uint32_t uiNodeId);
    uint32_t GetUINodeId() const;
    virtual void SetFloatingScale(float floatingScale);
    float GetFloatingScale() const;
    virtual void SetScale(float scaleX, float scaleY, float pivotX, float pivotY);
    float GetScaleX() const;
    float GetScaleY() const;
    float GetPivotX() const;
    float GetPivotY() const;
    void SetSCBKeepKeyboard(bool scbKeepKeyboardFlag);
    bool GetSCBKeepKeyboardFlag() const;

    void SetRaiseToAppTopForPointDownFunc(const NotifyRaiseToTopForPointDownFunc& func);
    void SetFrameLayoutFinishListener(const NotifyFrameLayoutFinishFunc& func);
    void NotifyScreenshot();
    void RemoveLifeCycleTask(const LifeCycleTaskType& taskType);
    void ClearLifeCycleTask();
    void PostLifeCycleTask(Task &&task, const std::string& name, const LifeCycleTaskType& taskType);
    WSError UpdateMaximizeMode(bool isMaximize);
    void NotifySessionForeground(uint32_t reason, bool withAnimation);
    void NotifySessionBackground(uint32_t reason, bool withAnimation, bool isFromInnerkits);
    void HandlePointDownDialog();
    bool CheckDialogOnForeground();
    std::shared_ptr<Media::PixelMap> GetSnapshotPixelMap(const float oriScale = 1.0f, const float newScale = 1.0f);
    virtual std::vector<Rect> GetTouchHotAreas() const
    {
        return std::vector<Rect>();
    }

    virtual void SetTouchHotAreas(const std::vector<Rect>& touchHotAreas);

    void SetVpr(float vpr)
    {
        vpr_ = vpr;
    }

    bool operator==(const Session* session) const
    {
        if (session == nullptr) {
            return false;
        }
        return (persistentId_ == session->persistentId_ && callingPid_ == session->callingPid_);
    }

    bool operator!=(const Session* session) const
    {
        return !this->operator==(session);
    }

    const char* DumpPointerWindowArea(MMI::WindowArea area) const;
    WSRectF UpdateHotRect(const WSRect& rect);
    WSError RaiseToAppTopForPointDown();

    virtual void NotifyForegroundInteractiveStatus(bool interactive);
    WSError UpdateTitleInTargetPos(bool isShow, int32_t height);
    void SetNotifySystemSessionPointerEventFunc(const NotifySystemSessionPointerEventFunc& func);
    void SetNotifySystemSessionKeyEventFunc(const NotifySystemSessionKeyEventFunc& func);
    bool IsSystemInput();
    // ForegroundInteractiveStatus interface only for event use
    bool GetForegroundInteractiveStatus() const;
    virtual void SetForegroundInteractiveStatus(bool interactive);

    /*
     * Window Lifecycle
     */
    bool GetIsPendingToBackgroundState() const;
    void SetIsPendingToBackgroundState(bool isPendingToBackgroundState);
    bool IsActivatedAfterScreenLocked() const;
    void SetIsActivatedAfterScreenLocked(bool isActivatedAfterScreenLocked);
    void SetAttachState(bool isAttach, WindowMode windowMode = WindowMode::WINDOW_MODE_UNDEFINED);
    bool GetAttachState() const;
    void RegisterDetachCallback(const sptr<IPatternDetachCallback>& callback);

    SystemSessionConfig GetSystemConfig() const;
    void RectCheckProcess();
    virtual void RectCheck(uint32_t curWidth, uint32_t curHeight) {};
    void RectSizeCheckProcess(uint32_t curWidth, uint32_t curHeight, uint32_t minWidth,
        uint32_t minHeight, uint32_t maxFloatingWindowSize);
    DetectTaskInfo GetDetectTaskInfo() const;
    void SetDetectTaskInfo(const DetectTaskInfo& detectTaskInfo);
    WSError GetUIContentRemoteObj(sptr<IRemoteObject>& uiContentRemoteObj);
    void CreateWindowStateDetectTask(bool isAttach, WindowMode windowMode);
    void RegisterIsScreenLockedCallback(const std::function<bool()>& callback);
    std::string GetWindowDetectTaskName() const;
    void RemoveWindowDetectTask();
    WSError SwitchFreeMultiWindow(const SystemSessionConfig& config);

    virtual bool CheckGetAvoidAreaAvailable(AvoidAreaType type) { return true; }

    virtual bool IsVisibleForeground() const;
    virtual bool IsVisibleNotBackground() const;
    void SetIsStarting(bool isStarting);
    void SetUIStateDirty(bool dirty);
    void SetMainSessionUIStateDirty(bool dirty);
    bool GetUIStateDirty() const;
    static bool IsScbCoreEnabled();
    static void SetScbCoreEnabled(bool enabled);
    bool IsVisible() const;
    virtual bool IsNeedSyncScenePanelGlobalPosition() { return true; }
    void SetAppInstanceKey(const std::string& appInstanceKey);
    std::string GetAppInstanceKey() const;
    std::shared_ptr<AppExecFwk::AbilityInfo> GetSessionInfoAbilityInfo();

    /*
     * Starting Window
     */
    WSError RemoveStartingWindow() override;
    void SetEnableRemoveStartingWindow(bool enableRemoveStartingWindow);
    bool GetEnableRemoveStartingWindow() const;
    void SetAppBufferReady(bool appBufferReady);
    bool GetAppBufferReady() const;
    void SetUseStartingWindowAboveLocked(bool useStartingWindowAboveLocked);
    bool UseStartingWindowAboveLocked() const;
    void SetHidingStartingWindow(bool hidingStartWindow);
    bool GetHidingStartingWindow() const;
    WSError SetLeashWindowAlpha(bool hidingStartWindow);

    /*
     * Window Hierarchy
     */
    void ProcessClickModalWindowOutside(int32_t posX, int32_t posY);
    void SetClickModalWindowOutsideListener(NotifyClickModalWindowOutsideFunc&& func);

    /*
     * Window Layout
     */
    static bool IsBackgroundUpdateRectNotifyEnabled();
    static void SetBackgroundUpdateRectNotifyEnabled(const bool enabled);
    void SetClientRect(const WSRect& rect);
    WSRect GetClientRect() const;
    void ResetDirtyFlags();
    void SetDragActivated(bool dragActivated);
    void SetClientDragEnable(bool dragEnable);
    std::optional<bool> GetClientDragEnable() const;
    std::shared_ptr<AppExecFwk::EventHandler> GetEventHandler() const;
    WSError UpdateClientDisplayId(DisplayId displayId);
    DisplayId TransformGlobalRectToRelativeRect(WSRect& rect) const;
    void TransformRelativeRectToGlobalRect(WSRect& rect) const;
    void UpdateClientRectPosYAndDisplayId(WSRect& rect);
    bool IsDragAccessible() const;
    void SetSingleHandTransform(const SingleHandTransform& transform);
    SingleHandTransform GetSingleHandTransform() const;
    void SetSingleHandModeFlag(bool flag);
    bool SessionIsSingleHandMode();
    void SetClientDisplayId(DisplayId displayId);
    DisplayId GetClientDisplayId() const;
    virtual void RegisterNotifySurfaceBoundsChangeFunc(int32_t sessionId, NotifySurfaceBoundsChangeFunc&& func) {};
    virtual void UnregisterNotifySurfaceBoundsChangeFunc(int32_t sessionId) {};
    virtual bool IsAnyParentSessionDragMoving() const { return false; }
    virtual bool IsAnyParentSessionDragZooming() const { return false; }
    void SetHasRequestedVsyncFunc(HasRequestedVsyncFunc&& func);
    void SetRequestNextVsyncWhenModeChangeFunc(RequestNextVsyncWhenModeChangeFunc&& func);
    void SetGlobalDisplayRect(const WSRect& rect);
    WSRect GetGlobalDisplayRect() const;
    virtual WSError UpdateGlobalDisplayRect(const WSRect& rect, SizeChangeReason reason);
    WSError NotifyClientToUpdateGlobalDisplayRect(const WSRect& rect, SizeChangeReason reason);
    const sptr<LayoutController>& GetLayoutController() const { return layoutController_; }
    WSError NotifyAppHookWindowInfoUpdated();
    void NotifyWindowStatusDidChangeIfNeedWhenUpdateRect(SizeChangeReason reason);
    void SetGetRsCmdBlockingCountFunc(const GetRsCmdBlockingCountFunc& func);

    /*
     * Screen Lock
     */
    bool IsScreenLockWindow() const;

    /*
     * Free Multi Window
     */
    std::shared_ptr<Media::PixelMap> SetFreezeImmediately(float scale, bool isFreeze, float blur) const;

    /*
     * PC Window
     */
    sptr<Session> GetMainSession() const;
    sptr<Session> GetMainOrFloatSession() const;
    bool IsPcWindow() const;
    bool IsAncestorsSession(int32_t ancestorsId) const;

    /**
     * Window Property
     */
    WindowUIInfo GetWindowUIInfoForWindowInfo() const;
    WindowDisplayInfo GetWindowDisplayInfoForWindowInfo() const;
    WindowLayoutInfo GetWindowLayoutInfoForWindowInfo() const;
    WindowMetaInfo GetWindowMetaInfoForWindowInfo() const;
    void SetClientDisplayIdChangeListener(const NotifyClientDisplayIdChangeFunc& func);
    uint32_t GetPropertyDirtyFlags() const { return propertyDirtyFlags_; };
    void SetPropertyDirtyFlags(uint32_t dirtyFlags) { propertyDirtyFlags_ = dirtyFlags; }
    void AddPropertyDirtyFlags(uint32_t dirtyFlags) { propertyDirtyFlags_ |= dirtyFlags; }
    WSError NotifyScreenshotAppEvent(ScreenshotEventType type);
    WSError UpdateBrightness(float brightness);

    /*
     * Window Pattern
     */
    void SetBorderUnoccupied(bool borderUnoccupied = false);
    bool GetBorderUnoccupied() const;
    bool IsPersistentImageFit() const { return isPersistentImageFit_.load(); };
    void SetPersistentImageFit(int32_t imageFit);
    int32_t GetPersistentImageFit() const { return persistentImageFit_; };
    void DeletePersistentImageFit();
    void RecoverImageForRecent();
    bool SupportSnapshotAllSessionStatus() const;
    bool SupportCacheLockedSessionSnapshot() const;
    void ResetLockedCacheSnapshot();
    void InitSnapshotCapacity();
    SnapshotStatus GetScreenSnapshotStatus() const;
    SnapshotStatus GetSessionSnapshotStatus(LifeCycleChangeReason reason = LifeCycleChangeReason::DEFAULT) const;
    uint32_t GetWindowSnapshotOrientation() const;
    uint32_t GetLastOrientation() const;
    bool HasSnapshotFreeMultiWindow();
    bool HasSnapshot(SnapshotStatus key);
    bool HasSnapshot();
    virtual void RecoverSnapshotPersistence(const SessionInfo& info) {};
    virtual void ClearSnapshotPersistence() {};
    void SetHasSnapshot(SnapshotStatus key, DisplayOrientation rotate);
    std::string GetSnapshotPersistentKey(int32_t id);
    std::string GetSnapshotPersistentKey(int32_t id, SnapshotStatus key);
    void DeleteHasSnapshot();
    void DeleteHasSnapshot(SnapshotStatus key);
    void DeleteHasSnapshotFreeMultiWindow();
    void SetFreeMultiWindow();
    void SetBufferNameForPixelMap(const char* functionName, const std::shared_ptr<Media::PixelMap>& pixelMap);
    void SetPreloadingStartingWindow(bool preloading);
    bool GetPreloadingStartingWindow() const;
    void PreloadSnapshot();
    void ResetPreloadSnapshot();
    std::atomic<bool> freeMultiWindow_ { false };
    std::atomic<bool> isPersistentImageFit_ { false };
    std::atomic<int32_t> persistentImageFit_ = 0;

    /*
     * Specific Window
     */
    void SetWindowAnimationDuration(int32_t duration);

    /*
     * RS Client Multi Instance
     */
    std::shared_ptr<RSUIContext> GetRSUIContext(const char* caller = "");

    /*
     * Window highligt outline
     */
    void UpdateSessionOutline(bool enabled, const OutlineStyleParams& params);
    void SetOutlineParamsChangeCallback(OutlineParamsChangeCallbackFunc&& func);

    WSError SetIsShowDecorInFreeMultiWindow(bool isShow);

    /*
     * Prelaunch check
     */
    void SetPrelaunch();
    bool IsPrelaunch() const;

protected:
    class SessionLifeCycleTask : public virtual RefBase {
    public:
        SessionLifeCycleTask(const Task& task, const std::string& name, const LifeCycleTaskType& type)
            : task(task), name(name), type(type) {}
        Task task;
        const std::string name;
        LifeCycleTaskType type;
        std::chrono::steady_clock::time_point startTime = std::chrono::steady_clock::now();
        bool running = false;
    };
    void GeneratePersistentId(bool isExtension, int32_t persistentId);
    virtual void UpdateSessionState(SessionState state);
    void NotifySessionStateChange(const SessionState& state);
    void UpdateSessionTouchable(bool touchable);
    virtual WSError UpdateActiveStatus(bool isActive) { return WSError::WS_OK; }

    /*
     * Gesture Back
     */
    virtual void UpdateGestureBackEnabled() {}

    WSRectF UpdateTopBottomArea(const WSRectF& rect, MMI::WindowArea area);
    WSRectF UpdateLeftRightArea(const WSRectF& rect, MMI::WindowArea area);
    WSRectF UpdateInnerAngleArea(const WSRectF& rect, MMI::WindowArea area);
    virtual void UpdatePointerArea(const WSRect& rect);
    virtual bool CheckPointerEventDispatch(const std::shared_ptr<MMI::PointerEvent>& pointerEvent) const;
    bool IsTopDialog() const;
    void HandlePointDownDialog(int32_t pointAction);
    void NotifySessionInfoChange();
    void NotifySessionPropertyChange(WindowInfoKey windowInfoKey);
    void NotifyDisplayIdChanged(int32_t persistentId, uint64_t screenId);

    void PostTask(Task&& task, const std::string& name = "sessionTask", int64_t delayTime = 0);
    void PostExportTask(Task&& task, const std::string& name = "sessionExportTask", int64_t delayTime = 0);
    template<typename SyncTask, typename Return = std::invoke_result_t<SyncTask>>
    Return PostSyncTask(SyncTask&& task, const std::string& name = "sessionTask")
    {
        Return ret;
        if (!handler_ || handler_->GetEventRunner()->IsCurrentRunnerThread()) {
            StartTraceForSyncTask(name);
            ret = task();
            FinishTraceForSyncTask();
            return ret;
        }
        auto syncTask = [&ret, &task, &name] {
            StartTraceForSyncTask(name);
            ret = task();
            FinishTraceForSyncTask();
        };
        handler_->PostSyncTask(std::move(syncTask), name, AppExecFwk::EventQueue::Priority::IMMEDIATE);
        return ret;
    }
    /*
     * Window shadow surfaceNode
     */
    std::shared_ptr<RSSurfaceNode> GetShadowSurfaceNode() const;
    std::shared_ptr<RSSurfaceNode> GetLeashWinShadowSurfaceNode() const;
    std::shared_ptr<RSUIContext> GetRSShadowContext();
    std::shared_ptr<RSUIContext> GetRSLeashWinShadowContext();

    static std::shared_ptr<AppExecFwk::EventHandler> mainHandler_;
    int32_t persistentId_ = INVALID_SESSION_ID;
    std::atomic<SessionState> state_ = SessionState::STATE_DISCONNECT;
    SessionInfo sessionInfo_;
    std::recursive_mutex sessionInfoMutex_;
    mutable std::mutex surfaceNodeMutex_;
    std::shared_ptr<RSSurfaceNode> surfaceNode_;
    std::shared_ptr<RSSurfaceNode> shadowSurfaceNode_;
    mutable std::mutex preloadSnapshotMutex_;
    std::shared_ptr<Media::PixelMap> preloadSnapshot_;
    mutable std::mutex snapshotMutex_;
    std::shared_ptr<Media::PixelMap> snapshot_;
    std::atomic<bool> snapshotNeedCancel_ = false;
    sptr<ISessionStage> sessionStage_;
    std::mutex lifeCycleTaskQueueMutex_;
    std::list<sptr<SessionLifeCycleTask>> lifeCycleTaskQueue_;
    bool isActive_ = false;
    bool isSystemActive_ = false;
    WSRectF bounds_;
    Rotation rotation_ { Rotation::ROTATION_0 };
    float offsetX_ = 0.0f;
    float offsetY_ = 0.0f;
    std::atomic_bool isExitSplitOnBackground_ = false;
    bool isVisible_ = false;
    int32_t currentRotation_ = 0;
    std::string label_;

    NotifyChangeSessionVisibilityWithStatusBarFunc changeSessionVisibilityWithStatusBarFunc_;
    NotifySessionStateChangeFunc sessionStateChangeFunc_;
    NotifyBufferAvailableChangeFunc bufferAvailableChangeFunc_;
    NotifySessionInfoChangeNotifyManagerFunc sessionInfoChangeNotifyManagerFunc_;
    NotifySessionPropertyChangeNotifyManagerFunc sessionPropertyChangeNotifyManagerFunc_;
    NotifyDisplayIdChangedNotifyManagerFunc displayIdChangedNotifyManagerFunc_;
    NotifySessionStateChangeNotifyManagerFunc sessionStateChangeNotifyManagerFunc_;
    NotifyRequestFocusStatusNotifyManagerFunc requestFocusStatusNotifyManagerFunc_;
    NotifyUIRequestFocusFunc requestFocusFunc_;
    NotifyUILostFocusFunc lostFocusFunc_;
    GetStateFromManagerFunc getStateFromManagerFunc_;
    NotifySessionFocusableChangeFunc sessionFocusableChangeFunc_;
    NotifySessionTouchableChangeFunc sessionTouchableChangeFunc_;
    NotifyClickFunc clickFunc_;
    NofitySessionLabelUpdatedFunc updateSessionLabelFunc_;
    NofitySessionIconUpdatedFunc updateSessionIconFunc_;
    NotifySessionSnapshotFunc notifySessionSnapshotFunc_;
    NotifyRaiseToTopForPointDownFunc raiseToTopForPointDownFunc_;
    NotifySessionInfoLockedStateChangeFunc sessionInfoLockedStateChangeFunc_;
    NotifySystemSessionPointerEventFunc systemSessionPointerEventFunc_;
    NotifySystemSessionKeyEventFunc systemSessionKeyEventFunc_;
    NotifyContextTransparentFunc contextTransparentFunc_;
    NotifyFrameLayoutFinishFunc frameLayoutFinishFunc_;
    std::mutex highlightChangeFuncMutex_;
    NotifyHighlightChangeFunc highlightChangeFunc_;

    /*
     * Window LifeCycle
     */
    NotifyPendingSessionActivationFunc pendingSessionActivationFunc_;
    NotifyBatchPendingSessionsActivationFunc batchPendingSessionsActivationFunc_;
    NotifyPendingSessionToForegroundFunc pendingSessionToForegroundFunc_;
    NotifyPendingSessionToBackgroundFunc pendingSessionToBackgroundFunc_;
    NotifyPendingSessionToBackgroundForDelegatorFunc pendingSessionToBackgroundForDelegatorFunc_;
    NotifyBackPressedFunc backPressedFunc_;
    NotifyTerminateSessionFunc terminateSessionFunc_;
    NotifyTerminateSessionFuncNew terminateSessionFuncNew_;
    NotifyTerminateSessionFuncTotal terminateSessionFuncTotal_;
    NotifySessionExceptionFunc sessionExceptionFunc_;
    NotifySessionExceptionFunc jsSceneSessionExceptionFunc_;
    VisibilityChangedDetectFunc visibilityChangedDetectFunc_ GUARDED_BY(SCENE_GUARD);
    NofitySessionLabelAndIconUpdatedFunc updateSessionLabelAndIconFunc_;
    NotifySessionGetTargetOrientationConfigInfoFunc sessionGetTargetOrientationConfigInfoFunc_;
    NotifyClearSubSessionFunc clearSubSessionFunc_;
    NotifyRestartAppFunc restartAppFunc_;

    /*
     * Window Rotate Animation
     */
    AcquireRotateAnimationConfigFunc acquireRotateAnimationConfigFunc_;

    SystemSessionConfig systemConfig_;
    bool needSnapshot_ = false;
    float snapshotScale_ = 0.5;
    sptr<ScenePersistence> scenePersistence_ = nullptr;

    /*
     * Window Layout
     */
    static bool isBackgroundUpdateRectNotifyEnabled_;

    /**
     * @brief Vsync service entry used to schedule callbacks on vsync and
     *        query display vsync information (e.g. VSync period or FPS).
     */
    std::shared_ptr<VsyncStation> vsyncStation_ = nullptr;

    WSRect lastLayoutRect_; // rect saved when go background
    WSRect layoutRect_;     // rect of root view
    NotifySessionRectChangeFunc sessionRectChangeFunc_;
    NotifySessionWindowLimitsChangeFunc sessionWindowLimitsChangeFunc_;
    NotifySessionDisplayIdChangeFunc sessionDisplayIdChangeFunc_;
    NotifyUpdateFloatingBallFunc updateFloatingBallFunc_;
    NotifyStopFloatingBallFunc stopFloatingBallFunc_;
    NotifyRestoreFloatingBallMainWindowFunc restoreFloatingBallMainWindowFunc_;
    GetRsCmdBlockingCountFunc getRsCmdBlockingCountFunc_;
    NotifyRestoreFloatMainWindowFunc restoreFloatMainWindowFunc_;
    sptr<LayoutController> layoutController_ = nullptr;
    void SetClientScale(float scaleX, float scaleY, float pivotX, float pivotY);
    std::atomic<uint32_t> crossPlaneState_ = 0;
    DisplayId clientDisplayId_ = 0; // Window displayId on the client
    DisplayId configDisplayId_ = DISPLAY_ID_INVALID;
    SuperFoldStatus lastScreenFoldStatus_ = SuperFoldStatus::UNKNOWN;
    virtual bool IsNeedConvertToRelativeRect(
        SizeChangeReason reason = SizeChangeReason::UNDEFINED) const { return false; }
    bool IsDragStart() const { return isDragStart_; }
    void SetDragStart(bool isDragStart);
    std::vector<AppExecFwk::SupportWindowMode> ExtractSupportWindowModeFromMetaData(
        const std::shared_ptr<AppExecFwk::AbilityInfo>& abilityInfo);
    std::vector<AppExecFwk::SupportWindowMode> ParseWindowModeFromMetaData(
        const std::string& supportModesInFreeMultiWindow);
    HasRequestedVsyncFunc hasRequestedVsyncFunc_;
    RequestNextVsyncWhenModeChangeFunc requestNextVsyncWhenModeChangeFunc_;
    WSError RequestNextVsyncWhenModeChange();
    void OnVsyncReceivedAfterModeChanged();
    void InitVsyncCallbackForModeChangeAndRequestNextVsync();

    /*
     * Window ZOrder
     */
    uint32_t zOrder_ = 0;
    uint32_t lastZOrder_ = 0;

    /*
     * Window Focus
     */
    bool isFocused_ = false;
    bool blockingFocus_ { false };
    bool isHighlighted_ { false };

    uint32_t uiNodeId_ = 0;
    std::map<MMI::WindowArea, WSRectF> windowAreas_;
    bool isTerminating_ = false;
    SceneAnimationConfig sceneAnimationConfig_;
    float floatingScale_ = 1.0f;
    bool scbKeepKeyboardFlag_ = false;
    mutable std::shared_mutex dialogVecMutex_;
    std::vector<sptr<Session>> dialogVec_;
    mutable std::shared_mutex parentSessionMutex_;
    sptr<Session> parentSession_;
    sptr<IWindowEventChannel> windowEventChannel_;

    mutable std::mutex pointerEventMutex_;
    mutable std::shared_mutex keyEventMutex_;
    bool rectChangeListenerRegistered_ = false;

    /*
     * Window Hierarchy
     */
    NotifyRaiseMainWindowAboveTargetFunc onRaiseMainWindowAboveTarget_;
    NotifyClickModalWindowOutsideFunc clickModalWindowOutsideFunc_;

    /*
     * Window Pattern
     */
    std::atomic<bool> isAttach_ { false };
    std::atomic<bool> needNotifyAttachState_ = { false };
    uint32_t lastSnapshotScreen_ = SCREEN_UNKNOWN;
    SnapshotStatus capacity_ = defaultCapacity;

    /*
     * Window Pipeline
     */
    uint32_t dirtyFlags_ = 0; // only accessed on SSM thread
    bool isNeedSyncSessionRect_ { true }; // where need sync to session rect,  currently use in split drag
    bool isStarting_ = false;   // when start app, session is starting state until foreground
    std::atomic_bool mainUIStateDirty_ = false;
    static bool isScbCoreEnabled_;

    /*
     *CompatibleMode Window scale
     */
    uint32_t compatibleDragScaleFlags_ = 0;

    /*
     * Keyboard Window
     */
    NotifyKeyboardStateChangeFunc keyboardStateChangeFunc_;
    ProcessCallingSessionIdChangeFunc callingSessionIdChangeFunc_;

    /*
     * Window Property
     */
    uint32_t propertyDirtyFlags_ = 0;

private:
    void HandleDialogForeground();
    void HandleDialogBackground();
    WSError HandleSubWindowClick(int32_t action, int32_t sourceType, bool isExecuteDelayRaise = false);

    template<typename T>
    bool RegisterListenerLocked(std::vector<std::shared_ptr<T>>& holder, const std::shared_ptr<T>& listener);
    template<typename T>
    bool UnregisterListenerLocked(std::vector<std::shared_ptr<T>>& holder, const std::shared_ptr<T>& listener);
    bool IsStateMatch(bool isAttach) const;
    bool IsSupportDetectWindow(bool isAttach);
    bool ShouldCreateDetectTask(bool isAttach, WindowMode windowMode) const;
    bool ShouldCreateDetectTaskInRecent(bool newShowRecent, bool oldShowRecent, bool isAttach) const;
    void CreateDetectStateTask(bool isAttach, WindowMode windowMode);
    void PostSpecificSessionLifeCycleTimeoutTask(const std::string& eventName);   // only report for specific window
    bool IsNeedReportTimeout() const;

    /*
     * Window Rotate Animation
     */
    int32_t GetRotateAnimationDuration();

    /*
     * Window Property
     */
    NotifyClientDisplayIdChangeFunc clientDisplayIdChangeFunc_;
    void InitSessionPropertyWhenConnect(const sptr<WindowSessionProperty>& property);
    void InitSystemSessionDragEnable(const sptr<WindowSessionProperty>& property);

    void UpdateGravityWhenUpdateWindowMode(WindowMode mode);

    template<typename T1, typename T2, typename Ret>
    using EnableIfSame = typename std::enable_if<std::is_same_v<T1, T2>, Ret>::type;
    template<typename T>
    inline EnableIfSame<T, ILifecycleListener, std::vector<std::weak_ptr<ILifecycleListener>>> GetListeners()
    {
        std::vector<std::weak_ptr<ILifecycleListener>> lifecycleListeners;
        {
            std::lock_guard<std::recursive_mutex> lock(lifecycleListenersMutex_);
            for (auto& listener : lifecycleListeners_) {
                lifecycleListeners.push_back(listener);
            }
        }
        return lifecycleListeners;
    }

    std::recursive_mutex lifecycleListenersMutex_;
    std::vector<std::shared_ptr<ILifecycleListener>> lifecycleListeners_;
    std::shared_ptr<AppExecFwk::EventHandler> handler_;
    std::shared_ptr<AppExecFwk::EventHandler> exportHandler_;
    std::function<bool()> isScreenLockedCallback_;

    sptr<WindowSessionProperty> property_;

    /*
     * Window Focus
     */
    mutable std::shared_mutex uiRequestFocusMutex_;
    mutable std::shared_mutex uiLostFocusMutex_;
    bool focusedOnShow_ = true;
    std::atomic_bool systemFocusable_ = true;
    bool focusableOnShow_ = true; // if false, ignore request focus when session onAttach
    bool isStartingBeforeVisible_ = false;

    bool showRecent_ = false;
    bool bufferAvailable_ = false;

    /*
     * Window Immersive
     */
    bool isStatusBarVisible_ = true;

    /*
     * Multi Window
     */
    bool isMidScene_ = false;

    WSRect preRect_;
    int32_t callingPid_ = -1;
    int32_t callingUid_ = -1;
    int32_t appIndex_ = { 0 };
    std::string callingBundleName_ { "unknown" };
    bool isRSVisible_ {false};
    WindowVisibilityState visibilityState_ { WINDOW_LAYER_STATE_MAX};
    bool needNotify_ {true};
    bool isRSDrawing_ {false};
    sptr<IRemoteObject> abilityToken_ = nullptr;
    float vpr_ { 1.5f };
    bool forceTouchable_ { true };
    bool systemTouchable_ { true };
    std::atomic_bool foregroundInteractiveStatus_ { true };

    /*
     * Window Lifecycle
     */
    void RecordWindowStateAttachExceptionEvent(bool isAttached);
    bool SetLifeCycleTaskRunning(const sptr<SessionLifeCycleTask>& lifeCycleTask);

    std::atomic<bool> isPendingToBackgroundState_ { false };
    std::atomic<bool> isActivatedAfterScreenLocked_ { true };
    sptr<IPatternDetachCallback> detachCallback_ = nullptr;

    std::shared_ptr<RSSurfaceNode> leashWinSurfaceNode_;
    std::shared_ptr<RSSurfaceNode> leashWinShadowSurfaceNode_;
    mutable std::mutex leashWinSurfaceNodeMutex_;
    DetectTaskInfo detectTaskInfo_;
    mutable std::shared_mutex detectTaskInfoMutex_;

    /*
     * Starting Window
     */
    bool enableRemoveStartingWindow_ { false };
    bool appBufferReady_ { false };
    bool useStartingWindowAboveLocked_ { false };
    bool hidingStartWindow_ { false };

    /*
     * Window Layout
     */
    std::optional<bool> clientDragEnable_;
    bool dragActivated_ = true;
    SingleHandTransform singleHandTransform_;
    bool singleHandModeFlag_ = false;
    SingleHandScreenInfo singleHandScreenInfo_;
    bool isDragStart_ = { false };
    std::atomic_bool isWindowModeDirty_ = false;
    std::atomic<int32_t> timesToWaitForVsync_ = 0;
    SizeChangeReason globalDisplayRectSizeChangeReason_ = SizeChangeReason::END;

    /*
     * Screen Lock
     */
    bool isScreenLockWindow_ { false };

    /*
     * Window Scene Snapshot
     */
    std::atomic<bool> enableAddSnapshot_ = true;
    Task saveSnapshotCallback_ = []() {};
    Task addSnapshotCallback_ = []() {};
    std::mutex saveSnapshotCallbackMutex_;
    std::mutex addSnapshotCallbackMutex_;

    /*
     * Window Pattern
     */
    std::atomic<bool> isSnapshotBlur_ { false };
    std::atomic<bool> isAppLockControl_ { false };
    std::atomic<bool> preloadingStartingWindow_ { false };
    bool borderUnoccupied_ = false;
    uint32_t GetBackgroundColor() const;

    /*
     * Specific Window
     */
    int32_t windowAnimationDuration_;

    /*
     * RS Client Multi Instance
     */
    uint64_t screenIdOfRSUIContext_ = SCREEN_ID_INVALID;
    std::shared_ptr<RSUIContext> rsUIContext_;

    /*
     * Window highligt outline
     */
    bool isOutlineEnabled_ = false;
    OutlineStyleParams outlineStyleParams_;
    OutlineParamsChangeCallbackFunc outlineParamsChangeCallback_;

    /*
     * Prelaunch check
     */
    uint64_t prelaunchStart_ = 0;
    bool prelaunchEnable_ = false;
};
} // namespace OHOS::Rosen

#endif // OHOS_ROSEN_WINDOW_SCENE_SESSION_H