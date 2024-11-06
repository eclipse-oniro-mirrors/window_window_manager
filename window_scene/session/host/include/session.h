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

#include "accessibility_element_info.h"
#include "interfaces/include/ws_common.h"
#include "session/container/include/zidl/session_stage_interface.h"
#include "session/host/include/zidl/session_stub.h"
#include "session/host/include/scene_persistence.h"
#include "wm_common.h"
#include "occupied_area_change_info.h"
#include "window_visibility_info.h"
#include "pattern_detach_callback_interface.h"

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
class RSTransaction;
class RSSyncTransactionController;
using NotifySessionRectChangeFunc = std::function<void(const WSRect& rect, const SizeChangeReason& reason)>;
using NotifyPendingSessionActivationFunc = std::function<void(SessionInfo& info)>;
using NotifyChangeSessionVisibilityWithStatusBarFunc = std::function<void(SessionInfo& info, const bool visible)>;
using NotifySessionStateChangeFunc = std::function<void(const SessionState& state)>;
using NotifyBufferAvailableChangeFunc = std::function<void(const bool isAvailable)>;
using NotifySessionStateChangeNotifyManagerFunc = std::function<void(int32_t persistentId, const SessionState& state)>;
using NotifyRequestFocusStatusNotifyManagerFunc =
    std::function<void(int32_t persistentId, const bool isFocused, const bool byForeground, FocusChangeReason reason)>;
using NotifyBackPressedFunc = std::function<void(const bool needMoveToBackground)>;
using NotifySessionFocusableChangeFunc = std::function<void(const bool isFocusable)>;
using NotifySessionTouchableChangeFunc = std::function<void(const bool touchable)>;
using NotifyClickFunc = std::function<void()>;
using NotifyTerminateSessionFunc = std::function<void(const SessionInfo& info)>;
using NotifyTerminateSessionFuncNew =
    std::function<void(const SessionInfo& info, bool needStartCaller, bool isFromBroker)>;
using NotifyTerminateSessionFuncTotal = std::function<void(const SessionInfo& info, TerminateType terminateType)>;
using NofitySessionLabelUpdatedFunc = std::function<void(const std::string& label)>;
using NofitySessionIconUpdatedFunc = std::function<void(const std::string& iconPath)>;
using NotifySessionExceptionFunc = std::function<void(const SessionInfo& info, bool needRemoveSession)>;
using NotifySessionSnapshotFunc = std::function<void(const int32_t& persistentId)>;
using NotifyPendingSessionToForegroundFunc = std::function<void(const SessionInfo& info)>;
using NotifyPendingSessionToBackgroundForDelegatorFunc = std::function<void(const SessionInfo& info,
    bool shouldBackToCaller)>;
using NotifyRaiseToTopForPointDownFunc = std::function<void()>;
using NotifyUIRequestFocusFunc = std::function<void()>;
using NotifyUILostFocusFunc = std::function<void()>;
using NotifySessionInfoLockedStateChangeFunc = std::function<void(const bool lockedState)>;
using GetStateFromManagerFunc = std::function<bool(const ManagerState key)>;
using NotifySystemSessionPointerEventFunc = std::function<void(std::shared_ptr<MMI::PointerEvent> pointerEvent)>;
using NotifySessionInfoChangeNotifyManagerFunc = std::function<void(int32_t persistentid)>;
using NotifySystemSessionKeyEventFunc = std::function<bool(std::shared_ptr<MMI::KeyEvent> keyEvent,
    bool isPreImeEvent)>;
using NotifyContextTransparentFunc = std::function<void()>;
using NotifyFrameLayoutFinishFunc = std::function<void()>;
using AcquireRotateAnimationConfigFunc = std::function<void(RotateAnimationConfig& config)>;

class ILifecycleListener {
public:
    virtual void OnActivation() {}
    virtual void OnConnect() {}
    virtual void OnForeground() {}
    virtual void OnBackground() {}
    virtual void OnDisconnect() {}
    virtual void OnLayoutFinished() {}
    virtual void OnRemoveBlank() {}
    virtual void OnDrawingCompleted() {}
    virtual void OnExtensionDied() {}
    virtual void OnExtensionTimeout(int32_t errorCode) {}
    virtual void OnAccessibilityEvent(const Accessibility::AccessibilityEventInfo& info,
        int64_t uiExtensionIdLevel) {}
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

class Session : public SessionStub {
public:
    using Task = std::function<void()>;
    explicit Session(const SessionInfo& info);
    virtual ~Session() = default;
    bool isKeyboardPanelEnabled_ = false;
    void SetEventHandler(const std::shared_ptr<AppExecFwk::EventHandler>& handler,
        const std::shared_ptr<AppExecFwk::EventHandler>& exportHandler = nullptr);

    virtual WSError ConnectInner(const sptr<ISessionStage>& sessionStage, const sptr<IWindowEventChannel>& eventChannel,
        const std::shared_ptr<RSSurfaceNode>& surfaceNode, SystemSessionConfig& systemConfig,
        sptr<WindowSessionProperty> property = nullptr, sptr<IRemoteObject> token = nullptr,
        int32_t pid = -1, int32_t uid = -1, const std::string& identityToken = "");
    WSError Reconnect(const sptr<ISessionStage>& sessionStage, const sptr<IWindowEventChannel>& eventChannel,
        const std::shared_ptr<RSSurfaceNode>& surfaceNode, sptr<WindowSessionProperty> property = nullptr,
        sptr<IRemoteObject> token = nullptr, int32_t pid = -1, int32_t uid = -1);
    WSError Foreground(sptr<WindowSessionProperty> property, bool isFromClient = false,
        const std::string& identityToken = "") override;
    WSError Background(bool isFromClient = false, const std::string& identityToken = "") override;
    WSError Disconnect(bool isFromClient = false, const std::string& identityToken = "") override;
    WSError Show(sptr<WindowSessionProperty> property) override;
    WSError Hide() override;
    WSError DrawingCompleted() override;
    void ResetSessionConnectState();

    bool RegisterLifecycleListener(const std::shared_ptr<ILifecycleListener>& listener);
    bool UnregisterLifecycleListener(const std::shared_ptr<ILifecycleListener>& listener);

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
    void NotifyExtensionDied() override;
    void NotifyExtensionTimeout(int32_t errorCode) override;
    void NotifyTransferAccessibilityEvent(const Accessibility::AccessibilityEventInfo& info,
        int64_t uiExtensionIdLevel) override;

    virtual WSError TransferPointerEvent(const std::shared_ptr<MMI::PointerEvent>& pointerEvent,
        bool needNotifyClient = true);
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
    std::shared_ptr<RSSurfaceNode> GetSurfaceNode() const;
    void SetLeashWinSurfaceNode(std::shared_ptr<RSSurfaceNode> leashWinSurfaceNode);
    std::shared_ptr<RSSurfaceNode> GetLeashWinSurfaceNode() const;
    std::shared_ptr<Media::PixelMap> GetSnapshot() const;
    std::shared_ptr<Media::PixelMap> Snapshot(bool runInFfrt = false, const float scaleParam = 0.0f) const;
    void SaveSnapshot(bool useFfrt);
    SessionState GetSessionState() const;
    virtual void SetSessionState(SessionState state);
    void SetSessionInfoAncoSceneState(int32_t ancoSceneState);
    void SetSessionInfoTime(const std::string& time);
    void SetSessionInfoAbilityInfo(const std::shared_ptr<AppExecFwk::AbilityInfo>& abilityInfo);
    void SetSessionInfoWant(const std::shared_ptr<AAFwk::Want>& want);
    void SetSessionInfoProcessOptions(const std::shared_ptr<AAFwk::ProcessOptions>& processOptions);
    void ResetSessionInfoResultCode();
    void SetSessionInfoPersistentId(int32_t persistentId);
    void SetSessionInfoCallerPersistentId(int32_t callerPersistentId);
    void SetSessionInfoContinueState(ContinueState state);
    void SetSessionInfoLockedState(bool lockedState);
    void SetSessionInfoIsClearSession(bool isClearSession);
    void SetSessionInfoAffinity(std::string affinity);
    void GetCloseAbilityWantAndClean(AAFwk::Want& outWant);
    void SetSessionInfo(const SessionInfo& info);
    const SessionInfo& GetSessionInfo() const;
    void SetScreenId(uint64_t screenId);
    void SetScreenIdOnServer(uint64_t screenId);
    WindowType GetWindowType() const;
    float GetAspectRatio() const;
    WSError SetAspectRatio(float ratio) override;
    void SetFocusedOnShow(bool focusedOnShow);
    bool IsFocusedOnShow() const;
    WSError SetSessionProperty(const sptr<WindowSessionProperty>& property);
    sptr<WindowSessionProperty> GetSessionProperty() const;
    void SetSessionRect(const WSRect& rect);
    WSRect GetSessionRect() const;
    WSRect GetSessionGlobalRect() const;
    WMError GetGlobalScaledRect(Rect& globalScaledRect) override;
    void SetSessionGlobalRect(const WSRect& rect);
    void SetSessionRequestRect(const WSRect& rect);
    WSRect GetSessionRequestRect() const;
    std::string GetWindowName() const;
    WSRect GetLastLayoutRect() const;
    WSRect GetLayoutRect() const;

    virtual WSError SetActive(bool active);
    virtual WSError UpdateSizeChangeReason(SizeChangeReason reason);
    SizeChangeReason GetSizeChangeReason() const { return reason_; }
    virtual WSError UpdateRect(const WSRect& rect, SizeChangeReason reason,
        const std::string& updateReason, const std::shared_ptr<RSTransaction>& rsTransaction = nullptr);
    WSError UpdateDensity();
    WSError UpdateOrientation();

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
    void SetBufferAvailable(bool bufferAvailable);
    bool GetBufferAvailable() const;
    void SetNeedSnapshot(bool needSnapshot);
    virtual void SetExitSplitOnBackground(bool isExitSplitOnBackground);
    virtual bool IsExitSplitOnBackground() const;
    virtual bool NeedStartingWindowExitAnimation() const { return true; }

    void SetPendingSessionActivationEventListener(const NotifyPendingSessionActivationFunc& func);
    void SetChangeSessionVisibilityWithStatusBarEventListener(
        const NotifyChangeSessionVisibilityWithStatusBarFunc& func);
    void SetTerminateSessionListener(const NotifyTerminateSessionFunc& func);
    WSError TerminateSessionNew(const sptr<AAFwk::SessionInfo> info, bool needStartCaller, bool isFromBroker);
    void SetTerminateSessionListenerNew(const NotifyTerminateSessionFuncNew& func);
    void SetSessionExceptionListener(const NotifySessionExceptionFunc& func, bool fromJsScene);
    void SetSessionSnapshotListener(const NotifySessionSnapshotFunc& func);
    WSError TerminateSessionTotal(const sptr<AAFwk::SessionInfo> info, TerminateType terminateType);
    void SetTerminateSessionListenerTotal(const NotifyTerminateSessionFuncTotal& func);
    WSError Clear(bool needStartCaller = false);
    WSError SetSessionLabel(const std::string &label);
    void SetUpdateSessionLabelListener(const NofitySessionLabelUpdatedFunc& func);
    WSError SetSessionIcon(const std::shared_ptr<Media::PixelMap>& icon);
    void SetUpdateSessionIconListener(const NofitySessionIconUpdatedFunc& func);
    void SetSessionStateChangeListenser(const NotifySessionStateChangeFunc& func);
    void SetBufferAvailableChangeListener(const NotifyBufferAvailableChangeFunc& func);
    virtual void UnregisterSessionChangeListeners();
    void SetSessionStateChangeNotifyManagerListener(const NotifySessionStateChangeNotifyManagerFunc& func);
    void SetSessionInfoChangeNotifyManagerListener(const NotifySessionInfoChangeNotifyManagerFunc& func);
    void SetRequestFocusStatusNotifyManagerListener(const NotifyRequestFocusStatusNotifyManagerFunc& func);
    void SetNotifyUIRequestFocusFunc(const NotifyUIRequestFocusFunc& func);
    void SetNotifyUILostFocusFunc(const NotifyUILostFocusFunc& func);
    void SetGetStateFromManagerListener(const GetStateFromManagerFunc& func);

    void SetSystemConfig(const SystemSessionConfig& systemConfig);
    void SetSnapshotScale(const float snapshotScale);
    void SetBackPressedListenser(const NotifyBackPressedFunc& func);
    virtual WSError ProcessBackEvent(); // send back event to session_stage
    WSError MarkProcessed(int32_t eventId) override;

    sptr<ScenePersistence> GetScenePersistence() const;
    void SetParentSession(const sptr<Session>& session);
    sptr<Session> GetParentSession() const;
    sptr<Session> GetMainSession();
    void BindDialogToParentSession(const sptr<Session>& session);
    void RemoveDialogToParentSession(const sptr<Session>& session);
    std::vector<sptr<Session>> GetDialogVector() const;
    void ClearDialogVector();
    WSError NotifyDestroy();
    WSError NotifyCloseExistPipWindow();

    void SetPendingSessionToForegroundListener(const NotifyPendingSessionToForegroundFunc& func);
    WSError PendingSessionToForeground();
    void SetPendingSessionToBackgroundForDelegatorListener(const NotifyPendingSessionToBackgroundForDelegatorFunc&
        func);
    WSError PendingSessionToBackgroundForDelegator(bool shouldBackToCaller);

    void SetSessionFocusableChangeListener(const NotifySessionFocusableChangeFunc& func);
    void SetSessionTouchableChangeListener(const NotifySessionTouchableChangeFunc& func);
    void SetClickListener(const NotifyClickFunc& func);
    void NotifySessionFocusableChange(bool isFocusable);
    void NotifySessionTouchableChange(bool touchable);
    void NotifyClick();
    void NotifyRequestFocusStatusNotifyManager(bool isFocused, bool byForeground = true,
        FocusChangeReason reason = FocusChangeReason::DEFAULT);
    void NotifyUIRequestFocus();
    virtual void NotifyUILostFocus();
    bool GetStateFromManager(const ManagerState key);
    virtual void PresentFoucusIfNeed(int32_t pointerAcrion);
    virtual WSError UpdateFocus(bool isFocused);
    WSError NotifyFocusStatus(bool isFocused);
    WSError RequestFocus(bool isFocused) override;
    virtual WSError UpdateWindowMode(WindowMode mode);
    WSError SetCompatibleModeInPc(bool enable, bool isSupportDragInPcCompatibleMode);
    WSError SetIsPcAppInPad(bool enable);
    WSError SetCompatibleWindowSizeInPc(int32_t portraitWidth, int32_t portraitHeight,
        int32_t landscapeWidth, int32_t landscapeHeight);
    WSError SetCompatibleModeEnableInPad(bool enable);
    virtual WSError SetSystemSceneBlockingFocus(bool blocking);
    bool GetBlockingFocus() const;
    WSError SetFocusable(bool isFocusable);
    bool NeedNotify() const;
    void SetNeedNotify(bool needNotify);
    bool GetFocusable() const;
    bool IsFocused() const;
    WSError SetTouchable(bool touchable);
    bool GetTouchable() const;
    void SetForceTouchable(bool touchable);
    virtual void SetSystemTouchable(bool touchable);
    bool GetSystemTouchable() const;
    virtual WSError SetRSVisible(bool isVisible);
    bool GetRSVisible() const;
    bool GetFocused() const;
    WSError SetVisibilityState(WindowVisibilityState state);
    WindowVisibilityState GetVisibilityState() const;
    WSError SetDrawingContentState(bool isRSDrawing);
    bool GetDrawingContentState() const;
    WSError SetBrightness(float brightness);
    float GetBrightness() const;
    void NotifyOccupiedAreaChangeInfo(sptr<OccupiedAreaChangeInfo> info,
                                      const std::shared_ptr<RSTransaction>& rsTransaction = nullptr);
    void SetSessionInfoLockedStateChangeListener(const NotifySessionInfoLockedStateChangeFunc& func);
    void NotifySessionInfoLockedStateChange(bool lockedState);
    void SetContextTransparentFunc(const NotifyContextTransparentFunc& func);
    void NotifyContextTransparent();
    bool NeedCheckContextTransparent() const;
    void SetAcquireRotateAnimationConfigFunc(const AcquireRotateAnimationConfigFunc& func);

    bool IsSessionValid() const;
    bool IsActive() const;
    bool IsSystemActive() const;
    bool IsSystemSession() const;
    bool IsTerminated() const;
    bool IsSessionForeground() const;
    virtual bool IsAnco() const { return false; }
    virtual void SetBlankFlag(bool isAddBlank) {};
    virtual bool GetBlankFlag() const { return false; }
    virtual bool GetBufferAvailableCallbackEnable() const { return false; }

    sptr<IRemoteObject> dialogTargetToken_ = nullptr;
    int32_t GetWindowId() const;
    void SetAppIndex(const int32_t appIndex);
    int32_t GetAppIndex() const;
    void SetCallingPid(int32_t id);
    void SetCallingUid(int32_t id);
    int32_t GetCallingPid() const;
    int32_t GetCallingUid() const;
    void SetAbilityToken(sptr<IRemoteObject> token);
    sptr<IRemoteObject> GetAbilityToken() const;
    WindowMode GetWindowMode() const;

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
    void PostLifeCycleTask(Task &&task, const std::string& name, const LifeCycleTaskType& taskType);
    WSError UpdateMaximizeMode(bool isMaximize);
    void NotifySessionForeground(uint32_t reason, bool withAnimation);
    void NotifySessionBackground(uint32_t reason, bool withAnimation, bool isFromInnerkits);
    void HandlePointDownDialog();
    bool CheckDialogOnForeground();
    virtual void PresentFocusIfPointDown();
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

    virtual void HandleStyleEvent(MMI::WindowArea area) {};
    WSError SetPointerStyle(MMI::WindowArea area);
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
    WSError SwitchFreeMultiWindow(bool enable);
    virtual int32_t GetCustomDecorHeight()
    {
        return 0;
    };
    virtual bool CheckGetAvoidAreaAvailable(AvoidAreaType type) { return true; }

    virtual bool IsVisibleForeground() const;
    void SetIsStarting(bool isStarting);
    void SetUIStateDirty(bool dirty);
    void SetMainSessionUIStateDirty(bool dirty);
    bool GetUIStateDirty() const;
    void ResetDirtyFlags();
    static bool IsScbCoreEnabled();
    static void SetScbCoreEnabled(bool enabled);
    virtual bool IsNeedSyncScenePanelGlobalPosition() { return true; }

    /*
     * Window Layout
     */
    void SetClientRect(const WSRect& rect);
    WSRect GetClientRect() const;

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
    void StartLifeCycleTask(sptr<SessionLifeCycleTask> lifeCycleTask);
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
        auto syncTask = [&ret, &task, name]() {
            StartTraceForSyncTask(name);
            ret = task();
            FinishTraceForSyncTask();
        };
        handler_->PostSyncTask(std::move(syncTask), name, AppExecFwk::EventQueue::Priority::IMMEDIATE);
        return ret;
    }

    static std::shared_ptr<AppExecFwk::EventHandler> mainHandler_;
    int32_t persistentId_ = INVALID_SESSION_ID;
    std::atomic<SessionState> state_ = SessionState::STATE_DISCONNECT;
    SessionInfo sessionInfo_;
    std::recursive_mutex sessionInfoMutex_;
    std::shared_ptr<RSSurfaceNode> surfaceNode_;
    mutable std::mutex snapshotMutex_;
    std::shared_ptr<Media::PixelMap> snapshot_;
    sptr<ISessionStage> sessionStage_;
    std::mutex lifeCycleTaskQueueMutex_;
    std::list<sptr<SessionLifeCycleTask>> lifeCycleTaskQueue_;
    bool isActive_ = false;
    bool isSystemActive_ = false;
    WSRectF bounds_;
    Rotation rotation_;
    float offsetX_ = 0.0f;
    float offsetY_ = 0.0f;
    std::atomic_bool isExitSplitOnBackground_ = false;
    bool isVisible_ = false;

    NotifyPendingSessionActivationFunc pendingSessionActivationFunc_;
    NotifyChangeSessionVisibilityWithStatusBarFunc changeSessionVisibilityWithStatusBarFunc_;
    NotifySessionStateChangeFunc sessionStateChangeFunc_;
    NotifyBufferAvailableChangeFunc bufferAvailableChangeFunc_;
    NotifySessionInfoChangeNotifyManagerFunc sessionInfoChangeNotifyManagerFunc_;
    NotifySessionStateChangeNotifyManagerFunc sessionStateChangeNotifyManagerFunc_;
    NotifyRequestFocusStatusNotifyManagerFunc requestFocusStatusNotifyManagerFunc_;
    NotifyUIRequestFocusFunc requestFocusFunc_;
    NotifyUILostFocusFunc lostFocusFunc_;
    GetStateFromManagerFunc getStateFromManagerFunc_;
    NotifyBackPressedFunc backPressedFunc_;
    NotifySessionFocusableChangeFunc sessionFocusableChangeFunc_;
    NotifySessionTouchableChangeFunc sessionTouchableChangeFunc_;
    NotifyClickFunc clickFunc_;
    NotifyTerminateSessionFunc terminateSessionFunc_;
    NotifyTerminateSessionFuncNew terminateSessionFuncNew_;
    NotifyTerminateSessionFuncTotal terminateSessionFuncTotal_;
    NofitySessionLabelUpdatedFunc updateSessionLabelFunc_;
    NofitySessionIconUpdatedFunc updateSessionIconFunc_;
    std::shared_ptr<NotifySessionExceptionFunc> sessionExceptionFunc_;
    std::shared_ptr<NotifySessionExceptionFunc> jsSceneSessionExceptionFunc_;
    NotifySessionSnapshotFunc notifySessionSnapshotFunc_;
    NotifyPendingSessionToForegroundFunc pendingSessionToForegroundFunc_;
    NotifyPendingSessionToBackgroundForDelegatorFunc pendingSessionToBackgroundForDelegatorFunc_;
    NotifyRaiseToTopForPointDownFunc raiseToTopForPointDownFunc_;
    NotifySessionInfoLockedStateChangeFunc sessionInfoLockedStateChangeFunc_;
    NotifySystemSessionPointerEventFunc systemSessionPointerEventFunc_;
    NotifySystemSessionKeyEventFunc systemSessionKeyEventFunc_;
    NotifyContextTransparentFunc contextTransparentFunc_;
    NotifyFrameLayoutFinishFunc frameLayoutFinishFunc_;
    AcquireRotateAnimationConfigFunc acquireRotateAnimationConfigFunc_;
    SystemSessionConfig systemConfig_;
    bool needSnapshot_ = false;
    float snapshotScale_ = 0.5;
    sptr<ScenePersistence> scenePersistence_ = nullptr;

    /*
     * Window Layout
     */
    WSRect winRect_;
    WSRect clientRect_; // rect saved when prelayout or notify client to update rect
    WSRect lastLayoutRect_; // rect saved when go background
    WSRect layoutRect_; // rect of root view
    WSRect globalRect_; // globalRect include translate
    mutable std::mutex globalRectMutex_;
    SizeChangeReason reason_ = SizeChangeReason::UNDEFINED;
    NotifySessionRectChangeFunc sessionRectChangeFunc_;

    /*
     * Window ZOrder
     */
    uint32_t zOrder_ = 0;
    uint32_t lastZOrder_ = 0;

    uint32_t uiNodeId_ = 0;
    bool isFocused_ = false;
    bool blockingFocus_ {false};
    float aspectRatio_ = 0.0f;
    std::map<MMI::WindowArea, WSRectF> windowAreas_;
    bool isTerminating_ = false;
    float floatingScale_ = 1.0f;
    bool isDirty_ = false;
    std::recursive_mutex sizeChangeMutex_;
    float scaleX_ = 1.0f;
    float scaleY_ = 1.0f;
    float pivotX_ = 0.0f;
    float pivotY_ = 0.0f;
    bool scbKeepKeyboardFlag_ = false;
    mutable std::shared_mutex dialogVecMutex_;
    std::vector<sptr<Session>> dialogVec_;
    mutable std::shared_mutex parentSessionMutex_;
    sptr<Session> parentSession_;
    sptr<IWindowEventChannel> windowEventChannel_;

    mutable std::mutex pointerEventMutex_;
    mutable std::shared_mutex keyEventMutex_;
    bool rectChangeListenerRegistered_ = false;
    // only accessed on SSM thread
    uint32_t dirtyFlags_ = 0;
    bool isNeedSyncSessionRect_ { true }; // where need sync to session rect,  currently use in split drag

    bool isStarting_ = false;   // when start app, session is starting state until foreground
    std::atomic_bool mainUIStateDirty_ = false;
    static bool isScbCoreEnabled_;

private:
    void HandleDialogForeground();
    void HandleDialogBackground();
    WSError HandleSubWindowClick(int32_t action);
    void NotifyPointerEventToRs(int32_t pointAction);
    void SetWindowSessionProperty(const sptr<WindowSessionProperty>& property);

    template<typename T>
    bool RegisterListenerLocked(std::vector<std::shared_ptr<T>>& holder, const std::shared_ptr<T>& listener);
    template<typename T>
    bool UnregisterListenerLocked(std::vector<std::shared_ptr<T>>& holder, const std::shared_ptr<T>& listener);
    bool IsStateMatch(bool isAttach) const;
    bool IsSupportDetectWindow(bool isAttach);
    bool ShouldCreateDetectTask(bool isAttach, WindowMode windowMode) const;
    bool ShouldCreateDetectTaskInRecent(bool newShowRecent, bool oldShowRecent, bool isAttach) const;
    void CreateDetectStateTask(bool isAttach, WindowMode windowMode);
    int32_t GetRotateAnimationDuration();

    /*
     * Window Layout
     */
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

    mutable std::shared_mutex propertyMutex_;
    sptr<WindowSessionProperty> property_;

    mutable std::shared_mutex uiRequestFocusMutex_;
    mutable std::shared_mutex uiLostFocusMutex_;

    bool focusedOnShow_ = true;
    bool showRecent_ = false;
    bool bufferAvailable_ = false;

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
    std::atomic<bool> isAttach_{ false };
    sptr<IPatternDetachCallback> detachCallback_ = nullptr;

    std::shared_ptr<RSSurfaceNode> leashWinSurfaceNode_;
    mutable std::mutex leashWinSurfaceNodeMutex_;
    DetectTaskInfo detectTaskInfo_;
    mutable std::shared_mutex detectTaskInfoMutex_;
};
} // namespace OHOS::Rosen

#endif // OHOS_ROSEN_WINDOW_SCENE_SESSION_H
