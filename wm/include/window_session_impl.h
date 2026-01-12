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

#ifndef OHOS_ROSEN_WINDOW_SESSION_IMPL_H
#define OHOS_ROSEN_WINDOW_SESSION_IMPL_H

#include <atomic>

#include <shared_mutex>
#include <ability_context.h>
#include <event_handler.h>
#include <i_input_event_consumer.h>
#include <ui_content.h>
#include <feature/window_keyframe/rs_window_keyframe_node.h>
#include <ui/rs_surface_node.h>
#include <ui/rs_ui_director.h>
#include "display_manager.h"
#include "singleton_container.h"

#include "common/include/window_session_property.h"
#include "display_info.h"
#include "future_callback.h"
#include "interfaces/include/ws_common.h"
#include "interfaces/include/ws_common_inner.h"
#include "session/container/include/zidl/session_stage_stub.h"
#include "session/host/include/zidl/session_interface.h"
#include "vsync_station.h"
#include "window.h"
#include "window_helper.h"
#include "window_option.h"
#include "wm_common.h"
#include "wm_common_inner.h"
#include "floating_ball_template_info.h"
#include "lifecycle_future_callback.h"

namespace OHOS {
namespace Rosen {
namespace {
template<typename T1, typename T2, typename Ret>
using EnableIfSame = typename std::enable_if<std::is_same_v<T1, T2>, Ret>::type;

/*
 * DFX
 */
const std::string SET_UICONTENT_TIMEOUT_LISTENER_TASK_NAME = "SetUIContentTimeoutListener";
constexpr int64_t SET_UICONTENT_TIMEOUT_TIME_MS = 4000;
constexpr int64_t SET_UICONTENT_TIMEOUT_TIME_AFTER_FREEZE_MS = 5000;
static std::atomic<float> animationSpeed_ = 1.0f;
static std::atomic<bool> isEnableAnimationSpeed_ = false;
}

struct WindowTitleVisibleFlags {
    bool isMaximizeVisible = true;
    bool isMinimizeVisible = true;
    bool isSplitVisible = true;
    bool isCloseVisible = true;
};

struct CursorInfo {
    double left = -1.0;
    double top = -1.0;
    double width = -1.0;
    double height = -1.0;

    bool isInvalid() const
    {
        return width <= 0 || height <= 0;
    }
};

using IKBWillShowListener = IKeyboardWillShowListener;
using IKBWillHideListener = IKeyboardWillHideListener;

class WindowSessionImpl : public Window, public virtual SessionStageStub {
public:
    explicit WindowSessionImpl(const sptr<WindowOption>& option,
        const std::shared_ptr<RSUIContext>& rsUIContext = nullptr);
    ~WindowSessionImpl();

    static sptr<Window> Find(const std::string& name);
    static std::vector<sptr<Window>> GetSubWindow(int parentId);
    static sptr<WindowSessionImpl> GetWindowWithId(uint32_t windowId);

    virtual WMError Create(const std::shared_ptr<AbilityRuntime::Context>& context,
        const sptr<Rosen::ISession>& iSession,
        const std::string& identityToken = "", bool isModuleAbilityHookEnd = false) { return WMError::WM_OK; }

    /*
     * inherits from window
     */
    WMError Show(uint32_t reason = 0, bool withAnimation = false, bool withFocus = true) override;
    WMError Show(uint32_t reason, bool withAnimation, bool withFocus, bool waitAttach) override;
    WMError Hide(uint32_t reason = 0, bool withAnimation = false, bool isFromInnerkits = true) override;
    WMError Hide(uint32_t reason, bool withAnimation, bool isFromInnerkits, bool waitDetach) override;
    WMError Destroy(uint32_t reason = 0) override;
    virtual WMError Destroy(bool needNotifyServer, bool needClearListener = true, uint32_t reason = 0);
    WMError NapiSetUIContent(const std::string& contentInfo, napi_env env, napi_value storage,
        BackupAndRestoreType type, sptr<IRemoteObject> token, AppExecFwk::Ability* ability) override;
    WMError AniSetUIContent(const std::string& contentInfo, ani_env* env, ani_object storage,
        BackupAndRestoreType type, sptr<IRemoteObject> token, AppExecFwk::Ability* ability) override;
    WMError NapiSetUIContentByName(const std::string& contentName, napi_env env, napi_value storage,
        BackupAndRestoreType type, sptr<IRemoteObject> token, AppExecFwk::Ability* ability) override;
    WMError AniSetUIContentByName(const std::string& contentName, ani_env* env, ani_object storage,
        BackupAndRestoreType type, sptr<IRemoteObject> token, AppExecFwk::Ability* ability) override;
    WMError SetUIContentByName(const std::string& contentInfo, napi_env env, napi_value storage,
        AppExecFwk::Ability* ability) override;
    WMError SetUIContentByAbc(const std::string& abcPath, napi_env env, napi_value storage,
        AppExecFwk::Ability* ability) override;
    std::shared_ptr<RSSurfaceNode> GetSurfaceNode() const override;
    const std::shared_ptr<AbilityRuntime::Context> GetContext() const override;
    void SetContext(const std::shared_ptr<AbilityRuntime::Context>& context);
    Rect GetRequestRect() const override;
    Rect GetGlobalDisplayRect() const override;
    WMError ClientToGlobalDisplay(const Position& inPosition, Position& outPosition) const override;
    WMError GlobalDisplayToClient(const Position& inPosition, Position& outPosition) const override;
    WSError UpdateGlobalDisplayRectFromServer(const WSRect& rect, SizeChangeReason reason) override;
    WindowType GetType() const override;
    const std::string& GetWindowName() const override;
    WindowState GetWindowState() const override;
    WindowState GetRequestWindowState() const;
    WMError SetFocusable(bool isFocusable) override;
    WMError SetTouchable(bool isTouchable) override;

    /*
     * Window Hierarchy
     */
    WMError SetTopmost(bool topmost) override;
    bool IsTopmost() const override;
    WMError SetMainWindowTopmost(bool isTopmost) override;
    bool IsMainWindowTopmost() const override;
    void SetSubWindowZLevelToProperty();
    int32_t GetSubWindowZLevelByFlags(WindowType type, uint32_t windowFlags, bool isTopmost);
    WMError RaiseToAppTopOnDrag();
    bool IsApplicationModalSubWindowShowing(int32_t parentId);

    WMError SetResizeByDragEnabled(bool dragEnabled) override;
    WMError SetRaiseByClickEnabled(bool raiseEnabled) override;
    WMError SetMainWindowRaiseByClickEnabled(bool raiseEnabled) override;
    WMError HideNonSystemFloatingWindows(bool shouldHide) override;
    WMError SetSingleFrameComposerEnabled(bool enable) override;
    bool IsFloatingWindowAppType() const override;

    /*
     * PC Window
     */
    bool IsPcWindow() const override;
    bool IsPadWindow() const override;
    bool IsPhonePadOrPcWindow() const override;
    bool IsPcOrFreeMultiWindowCapabilityEnabled() const override;
    bool IsPcOrPadFreeMultiWindowMode() const override;
    bool IsPadAndNotFreeMultiWindowCompatibleMode() const override;
    bool IsSceneBoardEnabled() const override;
    bool GetCompatibleModeInPc() const override;
    void HookCompatibleModeAvoidAreaNotify() override;
    bool IsAdaptToCompatibleImmersive() const override;
    WMError SetWindowDelayRaiseEnabled(bool isEnabled) override;
    bool IsHitTitleBar(std::shared_ptr<MMI::PointerEvent>& pointerEvent) const override;
    bool IsWindowDelayRaiseEnabled() const override;
    WMError SetTitleButtonVisible(bool isMaximizeVisible, bool isMinimizeVisible, bool isSplitVisible,
        bool isCloseVisible) override;
    WMError SetSubWindowModal(bool isModal, ModalityType modalityType = ModalityType::WINDOW_MODALITY) override;
    WMError SetWindowModal(bool isModal) override;
    void SetTargetAPIVersion(uint32_t targetAPIVersion);
    uint32_t GetTargetAPIVersion() const;
    void NotifyClientWindowSize();
    bool IsFullScreenPcAppInPadMode() const;
    sptr<WindowSessionProperty> GetPropertyByContext() const;
    std::vector<Rect> GetAncoWindowHotAreas() override;

    /*
     * Compatible Mode
     */
    bool IsAdaptToSimulationScale() const;
    bool IsAdaptToProportionalScale() const;
    bool IsInCompatScaleMode() const;
    bool IsInCompatScaleStatus() const;
    WMError UpdateCompatScaleInfo(const Transform& transform);
    void SetCompatInfoInExtensionConfig(AAFwk::WantParams& want) const;
    bool IsAdaptToSubWindow() const;
    static void RegisterWindowScaleCallback();
    static WMError GetWindowScaleCoordinate(uint32_t windowId, CursorInfo& cursorInfo);
    static sptr<WindowSessionImpl> GetScaleWindow(uint32_t windowId);
    WMError RegisterUIContentCreateListener(const sptr<IUIContentCreateListener>& listener) override;
    WMError UnregisterUIContentCreateListener(const sptr<IUIContentCreateListener>& listener) override;
    WMError UpdateCompatibleStyleMode(CompatibleStyleMode mode) override;

    WMError SetWindowType(WindowType type) override;
    WMError SetBrightness(float brightness) override;
    virtual float GetBrightness() const override;
    void SetRequestedOrientation(Orientation orientation, bool needAnimation = true) override;
    WMError RegisterPreferredOrientationChangeListener(
        const sptr<IPreferredOrientationChangeListener>& listener) override;
    WMError UnregisterPreferredOrientationChangeListener(
        const sptr<IPreferredOrientationChangeListener>& listener) override;
    void NotifyPreferredOrientationChange(Orientation orientation) override;
    WMError RegisterOrientationChangeListener(const sptr<IWindowOrientationChangeListener>& listener) override;
    WMError UnregisterOrientationChangeListener(const sptr<IWindowOrientationChangeListener>& listener) override;
    bool GetTouchable() const override;
    uint32_t GetWindowId() const override;
    uint64_t GetDisplayId() const override;
    Rect GetRect() const override;
    bool GetFocusable() const override;
    std::string GetContentInfo(BackupAndRestoreType type = BackupAndRestoreType::CONTINUATION) override;
    WMError SetRestoredRouterStack(const std::string& routerStack) override;
    Ace::UIContent* GetUIContent() const override;
    std::shared_ptr<Ace::UIContent> GetUIContentSharedPtr() const;
    Ace::UIContent* GetUIContentWithId(uint32_t winId) const override;
    void OnNewWant(const AAFwk::Want& want) override;
    void RequestVsync(const std::shared_ptr<VsyncCallback>& vsyncCallback) override;
    int64_t GetVSyncPeriod() override;
    void FlushFrameRate(uint32_t rate, int32_t animatorExpectedFrameRate, uint32_t rateType = 0) override;
    void ConsumePointerEvent(const std::shared_ptr<MMI::PointerEvent>& pointerEvent) override;
    void ConsumeKeyEvent(std::shared_ptr<MMI::KeyEvent>& inputEvent) override;
    void ConsumeBackEvent() override;
    bool IsDialogSessionBackGestureEnabled() override;
    bool PreNotifyKeyEvent(const std::shared_ptr<MMI::KeyEvent>& keyEvent) override;
    bool OnPointDown(int32_t eventId, int32_t posX, int32_t posY) override;
    WMError SetIntentParam(const std::string& intentParam, const std::function<void()>& loadPageCallback,
        bool isColdStart) override;
    virtual void SetForceSplitConfigEnable(bool enableForceSplit) {};
    void SetForceSplitConfig(const AppForceLandscapeConfig& config);

    /*
     * inherits from session stage
     */
    WSError SetActive(bool active) override;
    WSError UpdateRect(const WSRect& rect, SizeChangeReason reason,
        const SceneAnimationConfig& config = { nullptr, ROTATE_ANIMATION_DURATION, 0,
            WindowAnimationCurve::LINEAR, {0.0f, 0.0f, 0.0f, 0.0f} },
        const std::map<AvoidAreaType, AvoidArea>& avoidAreas = {}) override;
    void UpdateDensity() override;
    WSError UpdateBrightness(float brightness) override { return WSError::WS_OK; }
    void SetUniqueVirtualPixelRatio(bool useUniqueDensity, float virtualPixelRatio) override;
    void UpdateAnimationSpeed(float speed) override;
    void UpdateAllWindowSpeed(float speed);
    WSError UpdateOrientation() override;
    WSError UpdateDisplayId(uint64_t displayId) override;
    WSError UpdateFocus(const sptr<FocusNotifyInfo>& focusNotifyInfo, bool isFocused) override;
    void UpdateFocusState(bool isFocused);
    bool IsFocused() const override;
    WMError RequestFocus() const override;
    WMError RequestFocusByClient(bool isFocused) const override;
    WSError UpdateWindowMode(WindowMode mode) override;
    WSError GetTopNavDestinationName(std::string& topNavDestName) override { return WSError::WS_OK; }
    WSError HandleBackEvent() override;
    KeyboardAnimationConfig GetKeyboardAnimationConfig() override;
    bool NotifyOnKeyPreImeEvent(const std::shared_ptr<MMI::KeyEvent>& keyEvent) override;
    void NotifyPointerEvent(const std::shared_ptr<MMI::PointerEvent>& pointerEvent) override;
    void NotifyKeyEvent(const std::shared_ptr<MMI::KeyEvent>& keyEvent, bool& isConsumed,
        bool notifyInputMethod = true) override;
    void NotifyOccupiedAreaChangeInfoInner(sptr<OccupiedAreaChangeInfo> info);
    void NotifyOccupiedAreaChangeInfo(sptr<OccupiedAreaChangeInfo> info,
        const std::shared_ptr<RSTransaction>& rsTransaction, const Rect& callingSessionRect,
        const std::map<AvoidAreaType, AvoidArea>& avoidAreas) override;
    void NotifyKeyboardWillShow(const KeyboardAnimationInfo& keyboardAnimationInfo);
    void NotifyKeyboardWillHide(const KeyboardAnimationInfo& keyboardAnimationInfo);
    virtual void NotifyKeyboardDidShow(const KeyboardPanelInfo& keyboardPanelInfo);
    virtual void NotifyKeyboardDidHide(const KeyboardPanelInfo& keyboardPanelInfo);
    void NotifyKeyboardAnimationCompleted(const KeyboardPanelInfo& keyboardPanelInfo) override;
    void NotifyKeyboardAnimationWillBegin(const KeyboardAnimationInfo& keyboardAnimationInfo,
        const std::shared_ptr<RSTransaction>& rsTransaction) override;
    void NotifyForegroundInteractiveStatus(bool interactive) override;
    void NotifyDisplayMove(DisplayId from, DisplayId to) override;
    void NotifyWindowCrossAxisChange(CrossAxisState state) override;

    WMError RegisterLifeCycleListener(const sptr<IWindowLifeCycle>& listener) override;
    WMError UnregisterLifeCycleListener(const sptr<IWindowLifeCycle>& listener) override;
    WMError RegisterDisplayMoveListener(sptr<IDisplayMoveListener>& listener) override;
    WMError UnregisterDisplayMoveListener(sptr<IDisplayMoveListener>& listener) override;
    WMError RegisterWindowChangeListener(const sptr<IWindowChangeListener>& listener) override;
    WMError UnregisterWindowChangeListener(const sptr<IWindowChangeListener>& listener) override;
    WMError RegisterWindowCrossAxisListener(const sptr<IWindowCrossAxisListener>& listener) override;
    WMError UnregisterWindowCrossAxisListener(const sptr<IWindowCrossAxisListener>& listener) override;
    void RegisterDialogDeathRecipientListener(const sptr<IDialogDeathRecipientListener>& listener) override;
    void UnregisterDialogDeathRecipientListener(const sptr<IDialogDeathRecipientListener>& listener) override;
    WMError RegisterDialogTargetTouchListener(const sptr<IDialogTargetTouchListener>& listener) override;
    WMError UnregisterDialogTargetTouchListener(const sptr<IDialogTargetTouchListener>& listener) override;
    WMError RegisterOccupiedAreaChangeListener(const sptr<IOccupiedAreaChangeListener>& listener) override;
    WMError UnregisterOccupiedAreaChangeListener(const sptr<IOccupiedAreaChangeListener>& listener) override;
    WMError RegisterKeyboardWillShowListener(const sptr<IKBWillShowListener>& listener) override;
    WMError UnregisterKeyboardWillShowListener(const sptr<IKBWillShowListener>& listener) override;
    WMError RegisterKeyboardWillHideListener(const sptr<IKBWillHideListener>& listener) override;
    WMError UnregisterKeyboardWillHideListener(const sptr<IKBWillHideListener>& listener) override;
    WMError RegisterKeyboardDidShowListener(const sptr<IKeyboardDidShowListener>& listener) override;
    WMError UnregisterKeyboardDidShowListener(const sptr<IKeyboardDidShowListener>& listener) override;
    WMError RegisterKeyboardDidHideListener(const sptr<IKeyboardDidHideListener>& listener) override;
    WMError UnregisterKeyboardDidHideListener(const sptr<IKeyboardDidHideListener>& listener) override;
    WMError RegisterTouchOutsideListener(const sptr<ITouchOutsideListener>& listener) override;
    WMError UnregisterTouchOutsideListener(const sptr<ITouchOutsideListener>& listener) override;
    WMError RegisterWindowVisibilityChangeListener(const IWindowVisibilityListenerSptr& listener) override;
    WMError UnregisterWindowVisibilityChangeListener(const IWindowVisibilityListenerSptr& listener) override;
    WMError RegisterOcclusionStateChangeListener(const sptr<IOcclusionStateChangedListener>& listener) override;
    WMError UnregisterOcclusionStateChangeListener(const sptr<IOcclusionStateChangedListener>& listener) override;
    WSError NotifyWindowOcclusionState(const WindowVisibilityState state) override;
    WMError RegisterFrameMetricsChangeListener(const sptr<IFrameMetricsChangedListener>& listener) override;
    WMError UnregisterFrameMetricsChangeListener(const sptr<IFrameMetricsChangedListener>& listener) override;
    void NotifyFrameMetrics(const Ace::FrameMetrics& info);
    WMError RegisterDisplayIdChangeListener(const IDisplayIdChangeListenerSptr& listener) override;
    WMError UnregisterDisplayIdChangeListener(const IDisplayIdChangeListenerSptr& listener) override;
    WMError RegisterSystemDensityChangeListener(const ISystemDensityChangeListenerSptr& listener) override;
    WMError UnregisterSystemDensityChangeListener(const ISystemDensityChangeListenerSptr& listener) override;
    WMError RegisterAcrossDisplaysChangeListener(const IAcrossDisplaysChangeListenerSptr& listener) override;
    WMError UnRegisterAcrossDisplaysChangeListener(const IAcrossDisplaysChangeListenerSptr& listener) override;
    WMError RegisterWindowNoInteractionListener(const IWindowNoInteractionListenerSptr& listener) override;
    WMError UnregisterWindowNoInteractionListener(const IWindowNoInteractionListenerSptr& listener) override;
    WMError RegisterWindowStageLifeCycleListener(const sptr<IWindowStageLifeCycle>& listener) override;
    WMError UnregisterWindowStageLifeCycleListener(const sptr<IWindowStageLifeCycle>& listener) override;
    WMError RegisterSystemBarPropertyListener(const sptr<ISystemBarPropertyListener>& listener) override;
    WMError UnregisterSystemBarPropertyListener(const sptr<ISystemBarPropertyListener>& listener) override;
    WMError RegisterFreeWindowModeChangeListener(const sptr<IFreeWindowModeChangeListener>& listener) override;
    WMError UnregisterFreeWindowModeChangeListener(const sptr<IFreeWindowModeChangeListener>& listener) override;
    void NotifySystemBarPropertyUpdate(WindowType type, const SystemBarProperty& property) override;
    void RegisterWindowDestroyedListener(const NotifyNativeWinDestroyFunc& func) override;
    void UnregisterWindowDestroyedListener() override { notifyNativeFunc_ = nullptr; }
    WMError RegisterScreenshotListener(const sptr<IScreenshotListener>& listener) override;
    WMError UnregisterScreenshotListener(const sptr<IScreenshotListener>& listener) override;
    WMError RegisterScreenshotAppEventListener(const IScreenshotAppEventListenerSptr& listener) override;
    WMError UnregisterScreenshotAppEventListener(const IScreenshotAppEventListenerSptr& listener) override;
    void SetAceAbilityHandler(const sptr<IAceAbilityHandler>& handler) override;
    void SetInputEventConsumer(const std::shared_ptr<IInputEventConsumer>& inputEventConsumer) override;
    void GetExtensionConfig(AAFwk::WantParams& want) const override;
    WMError OnExtensionMessage(uint32_t code, int32_t persistentId, const AAFwk::Want& data) override;
    virtual WMError HandleHostWindowRaise(uint32_t code, int32_t persistentId, const AAFwk::Want& data);
    virtual WMError HandleRegisterHostWindowRectChangeListener(uint32_t code, int32_t persistentId,
        const AAFwk::Want& data);
    virtual WMError HandleUnregisterHostWindowRectChangeListener(uint32_t code, int32_t persistentId,
        const AAFwk::Want& data);
    virtual WMError HandleRegisterHostRectChangeInGlobalDisplayListener(uint32_t code, int32_t persistentId,
        const AAFwk::Want& data);
    virtual WMError HandleUnregisterHostRectChangeInGlobalDisplayListener(uint32_t code, int32_t persistentId,
        const AAFwk::Want& data);
    virtual WMError HandleUIExtRegisterKeyboardDidShowListener(uint32_t code, int32_t persistentId,
        const AAFwk::Want& data);
    virtual WMError HandleUIExtUnregisterKeyboardDidShowListener(uint32_t code, int32_t persistentId,
        const AAFwk::Want& data);
    virtual WMError HandleUIExtRegisterKeyboardDidHideListener(uint32_t code, int32_t persistentId,
        const AAFwk::Want& data);
    virtual WMError HandleUIExtUnregisterKeyboardDidHideListener(uint32_t code, int32_t persistentId,
        const AAFwk::Want& data);

    WMError SetBackgroundColor(const std::string& color) override;
    virtual Orientation GetRequestedOrientation() override;

    int32_t GetParentId() const;
    int32_t GetPersistentId() const override;
    sptr<WindowSessionProperty> GetProperty() const;
    SystemSessionConfig GetSystemSessionConfig() const;
    sptr<ISession> GetHostSession() const;
    int32_t GetFloatingWindowParentId();
    void NotifyAfterForeground(bool needNotifyListeners = true,
        bool needNotifyUiContent = true, bool waitAttach = false);
    void GetAttachStateSyncResult(bool waitAttachState, bool afterForeground) const;
    void NotifyAfterBackground(bool needNotifyListeners = true, bool needNotifyUiContent = true);
    void NotifyAfterDidForeground(uint32_t reason = static_cast<uint32_t>(WindowStateChangeReason::NORMAL));
    void NotifyAfterDidBackground(uint32_t reason = static_cast<uint32_t>(WindowStateChangeReason::NORMAL));
    void NotifyForegroundFailed(WMError ret);
    void NotifyBackgroundFailed(WMError ret);
    WSError MarkProcessed(int32_t eventId) override;
    void UpdateTitleButtonVisibility();
    void HideTitleButton(bool& hideSplitButton, bool& hideMaximizeButton, bool& hideMinimizeButton,
        bool& hideCloseButton);
    WSError NotifyDestroy() override;
    WSError NotifyTransferComponentData(const AAFwk::WantParams& wantParams) override;
    WSErrorCode NotifyTransferComponentDataSync(const AAFwk::WantParams& wantParams,
        AAFwk::WantParams& reWantParams) override;
    void NotifyTouchDialogTarget(int32_t posX = 0, int32_t posY = 0) override;
    void NotifyScreenshot() override;
    void DumpSessionElementInfo(const std::vector<std::string>& params) override;
    // colorspace, gamut
    virtual bool IsSupportWideGamut() override;
    virtual void SetColorSpace(ColorSpace colorSpace) override;
    virtual ColorSpace GetColorSpace() override;
    WSError NotifyTouchOutside() override;
    WMError SetLandscapeMultiWindow(bool isLandscapeMultiWindow) override;
    WSError NotifyWindowVisibility(bool isVisible) override;
    WSError NotifyNoInteractionTimeout(const IWindowNoInteractionListenerSptr& listener);
    WMError TransferAccessibilityEvent(const Accessibility::AccessibilityEventInfo& info,
        int64_t uiExtensionIdLevel) override;
    WindowState state_ { WindowState::STATE_INITIAL };
    WindowState requestState_ { WindowState::STATE_INITIAL };
    WSError UpdateMaximizeMode(MaximizeMode mode) override;
    void NotifySessionForeground(uint32_t reason, bool withAnimation) override;
    void NotifySessionBackground(uint32_t reason, bool withAnimation, bool isFromInnerkits) override;
    WSError UpdateTitleInTargetPos(bool isShow, int32_t height) override;
    WSError NotifyDialogStateChange(bool isForeground) override;
    bool IsMainHandlerAvailable() const override;

    /*
     * PiP Window
     */
    WSError NotifyCloseExistPipWindow() override;
    WSError SetPipActionEvent(const std::string& action, int32_t status) override;
    WSError SetPiPControlEvent(WsPiPControlType controlType, WsPiPControlStatus status) override;
    WSError NotifyPipWindowSizeChange(double width, double height, double scale) override;
    WSError NotifyPiPActiveStatusChange(bool status) override;
    void UpdatePiPRect(const Rect& rect, WindowSizeChangeReason reason) override;
    void UpdatePiPControlStatus(PiPControlType controlType, PiPControlStatus status) override;
    void SetAutoStartPiP(bool isAutoStart, uint32_t priority, uint32_t width, uint32_t height) override;
    void UpdatePiPTemplateInfo(PiPTemplateInfo& pipTemplateInfo) override;
    WMError GetPiPSettingSwitchStatus(bool& switchStatus) const override;
    WMError SetPipParentWindowId(uint32_t windowId) const override;
    WMError IsPiPActive(bool& status) override;

    WMError UpdateFloatingBall(const FloatingBallTemplateBaseInfo& fbTemplateBaseInfo,
        const std::shared_ptr<Media::PixelMap>& icon) override;
    void NotifyPrepareCloseFloatingBall() override;
    WSError SendFbActionEvent(const std::string& action) override;
    WMError RestoreFbMainWindow(const std::shared_ptr<AAFwk::Want>& want) override;

    WMError GetFloatingBallWindowId(uint32_t& windowId) override;

    void SetDrawingContentState(bool drawingContentState);
    WMError RegisterWindowStatusChangeListener(const sptr<IWindowStatusChangeListener>& listener) override;
    WMError UnregisterWindowStatusChangeListener(const sptr<IWindowStatusChangeListener>& listener) override;
    WMError SetSpecificBarProperty(WindowType type, const SystemBarProperty& property) override;
    void SetLayoutTransform(const Transform& trans);
    Transform GetLayoutTransform() const override;

    /*
     * Window Decor
     */
    WMError SetDecorVisible(bool isVisible) override;
    WMError GetDecorVisible(bool& isVisible) override;
    WMError SetWindowTitleMoveEnabled(bool enable) override;
    WMError SetDecorHeight(int32_t decorHeight) override;
    WMError GetDecorHeight(int32_t& height) override;
    WMError SetDecorButtonStyle(const DecorButtonStyle& decorButtonStyle) override;
    WMError GetDecorButtonStyle(DecorButtonStyle& decorButtonStyle) override;
    WMError GetTitleButtonArea(TitleButtonRect& titleButtonRect) override;
    WMError SetAPPWindowLabel(const std::string& label) override;
    WMError SetAPPWindowIcon(const std::shared_ptr<Media::PixelMap>& icon) override;
    WMError SetWindowContainerColor(const std::string& activeColor, const std::string& inactiveColor) override;
    WMError SetWindowContainerModalColor(const std::string& activeColor, const std::string& inactiveColor) override;
    nlohmann::json SetContainerButtonStyle(const DecorButtonStyle& decorButtonStyle);
    void UpdateDecorEnable(bool needNotify = false, WindowMode mode = WindowMode::WINDOW_MODE_UNDEFINED);

    /*
     * Window Decor listener
     */
    WMError RegisterWindowTitleButtonRectChangeListener(
        const sptr<IWindowTitleButtonRectChangedListener>& listener) override;
    WMError UnregisterWindowTitleButtonRectChangeListener(
        const sptr<IWindowTitleButtonRectChangedListener>& listener) override;
    void NotifyWindowTitleButtonRectChange(TitleButtonRect titleButtonRect);
    WMError RegisterSubWindowCloseListeners(const sptr<ISubWindowCloseListener>& listener) override;
    WMError UnregisterSubWindowCloseListeners(const sptr<ISubWindowCloseListener>& listener) override;
    void NotifySubWindowClose(bool& terminateCloseProcess);
    WMError RegisterMainWindowCloseListeners(const sptr<IMainWindowCloseListener>& listener) override;
    WMError UnregisterMainWindowCloseListeners(const sptr<IMainWindowCloseListener>& listener) override;
    WMError NotifyMainWindowClose(bool& terminateCloseProcess);
    WMError RegisterWindowWillCloseListeners(const sptr<IWindowWillCloseListener>& listener) override;
    WMError UnRegisterWindowWillCloseListeners(const sptr<IWindowWillCloseListener>& listener) override;
    WMError NotifyWindowWillClose(sptr<Window> window);

    WSError GetUIContentRemoteObj(sptr<IRemoteObject>& uiContentRemoteObj) override;
    void RecoverSessionListener();
    void SetDefaultDisplayIdIfNeed();
    WMError RegisterWindowRectChangeListener(const sptr<IWindowRectChangeListener>& listener) override;
    WMError UnregisterWindowRectChangeListener(const sptr<IWindowRectChangeListener>& listener) override;
    WMError RegisterWindowTitleChangeListener(const sptr<IWindowTitleChangeListener>& listener) override;
    WMError UnregisterWindowTitleChangeListener(const sptr<IWindowTitleChangeListener>& listener) override;
    WMError RegisterWindowTitleOrHotAreasListener(const sptr<IWindowTitleOrHotAreasListener>& listener) override;
    WMError UnregisterWindowTitleOrHotAreasListener(const sptr<IWindowTitleOrHotAreasListener>& listener) override;
    WMError RegisterRectChangeInGlobalDisplayListener(
        const sptr<IRectChangeInGlobalDisplayListener>& listener) override;
    WMError UnregisterRectChangeInGlobalDisplayListener(
        const sptr<IRectChangeInGlobalDisplayListener>& listener) override;
    WMError RegisterExtensionSecureLimitChangeListener(
        const sptr<IExtensionSecureLimitChangeListener>& listener) override;
    WMError UnregisterExtensionSecureLimitChangeListener(
        const sptr<IExtensionSecureLimitChangeListener>& listener) override;
    virtual WMError GetCallingWindowWindowStatus(uint32_t callingWindowId, WindowStatus& windowStatus) const override;
    virtual WMError GetCallingWindowRect(uint32_t callingWindowId, Rect& rect) const override;
    virtual void SetUiDvsyncSwitch(bool dvsyncSwitch) override;
    virtual void SetTouchEvent(int32_t touchType) override;
    WMError SetContinueState(int32_t continueState) override;
    virtual WMError CheckAndModifyWindowRect(uint32_t& width, uint32_t& height)
    {
        return WMError::WM_ERROR_INVALID_PARAM;
    }

    /*
     * Multi Window
     */
    WSError SetSplitButtonVisible(bool isVisible) override;
    WMError GetIsMidScene(bool& isMidScene) override;
    WSError SendContainerModalEvent(const std::string& eventName, const std::string& eventValue) override;

    /*
     * Window Layout
     */
    WMError EnableDrag(bool enableDrag) override;
    WSError SetDragActivated(bool dragActivated) override;
    WSError SetEnableDragBySystem(bool enableDrag) override;
    bool IsWindowDraggable();
    float GetVirtualPixelRatio() override;
    CrossAxisState GetCrossAxisState() override;
    void RegisterKeyFrameCallback();
    WSError LinkKeyFrameNode() override;
    WSError SetStageKeyFramePolicy(const KeyFramePolicy& keyFramePolicy) override;
    WMError SetDragKeyFramePolicy(const KeyFramePolicy& keyFramePolicy) override;
    WMError RegisterWindowStatusDidChangeListener(const sptr<IWindowStatusDidChangeListener>& listener) override;
    WMError UnregisterWindowStatusDidChangeListener(const sptr<IWindowStatusDidChangeListener>& listener) override;
    WSError NotifyLayoutFinishAfterWindowModeChange(WindowMode mode) override { return WSError::WS_OK; }
    WMError UpdateWindowModeForUITest(int32_t updateMode) override { return WMError::WM_OK; }
    WSError NotifyAppHookWindowInfoUpdated() override { return WSError::WS_DO_NOTHING; }
    void SetNotifySizeChangeFlag(bool flag);

    /*
     * Free Multi Window
     */
    WMError RegisterSwitchFreeMultiWindowListener(const sptr<ISwitchFreeMultiWindowListener>& listener) override;
    WMError UnregisterSwitchFreeMultiWindowListener(const sptr<ISwitchFreeMultiWindowListener>& listener) override;
    void SetFreeMultiWindowMode(bool enable)
    {
        windowSystemConfig_.freeMultiWindowEnable_ = enable;
    }
    void SwitchSubWindow(bool freeMultiWindowEnable, int32_t parentId);
    void SwitchSystemWindow(bool freeMultiWindowEnable, int32_t parentId);

    /*
     * Window Immersive
     */
    WMError RegisterAvoidAreaChangeListener(const sptr<IAvoidAreaChangedListener>& listener) override;
    WMError UnregisterAvoidAreaChangeListener(const sptr<IAvoidAreaChangedListener>& listener) override;
    WMError SetAvoidAreaOption(uint32_t avoidAreaOption) override;
    WMError GetAvoidAreaOption(uint32_t& avoidAreaOption) override;
    void NotifyAvoidAreaChange(const sptr<AvoidArea>& avoidArea, AvoidAreaType type);
    WSError UpdateAvoidArea(const sptr<AvoidArea>& avoidArea, AvoidAreaType type) override;
    bool IsSystemWindow() const override { return WindowHelper::IsSystemWindow(GetType()); }
    bool IsAppWindow() const override { return WindowHelper::IsAppWindow(GetType()); }
    WindowType GetRootHostWindowType() const override { return rootHostWindowType_; }
    void SetRootHostWindowType(WindowType& rootHostWindowType) override { rootHostWindowType_ = rootHostWindowType; }
    WMError UpdateSystemBarProperties(const std::unordered_map<WindowType, SystemBarProperty>& systemBarProperties,
        const std::unordered_map<WindowType, SystemBarPropertyFlag>& systemBarPropertyFlags) override;
    void UpdateSpecificSystemBarEnabled(bool systemBarEnable, bool systemBarEnableAnimation,
        SystemBarProperty& property) override;
    WMError SetSystemBarProperty(WindowType type, const SystemBarProperty& property) override;

    /*
     * Window Property
     */
    WMError SetWindowDefaultDensityEnabled(bool enabled) override;
    void SetDefaultDensityEnabledValue(bool enabled);
    bool IsStageDefaultDensityEnabled();
    WSError NotifyDisplayIdChange(DisplayId displayId);
    WSError NotifyScreenshotAppEvent(ScreenshotEventType type) override;
    bool IsDeviceFeatureCapableFor(const std::string& feature) const override;
    bool IsDeviceFeatureCapableForFreeMultiWindow() const override;
    bool IsAnco() const override;

    /*
     * Window Input Event
     */
    WMError NotifyWatchGestureConsumeResult(int32_t keyCode, bool isConsumed);
    WMError NotifyWatchFocusActiveChange(bool isActive);
    WMError InjectTouchEvent(const std::shared_ptr<MMI::PointerEvent>& pointerEvent) override;
    void RegisterWatchFocusActiveChangeCallback();
    void NotifyConsumeResultToFloatWindow(const std::shared_ptr<MMI::KeyEvent>& keyEvent, bool isConsumed);

    /*
     * HightLight Window
     */
    bool GetExclusivelyHighlighted() const;
    WMError IsWindowHighlighted(bool& highlighted) const override;
    WMError SetExclusivelyHighlighted(bool isExclusivelyHighlighted) override;
    WMError RegisterWindowHighlightChangeListeners(const sptr<IWindowHighlightChangeListener>& listener) override;
    WMError UnregisterWindowHighlightChangeListeners(const sptr<IWindowHighlightChangeListener>& listener) override;
    WSError NotifyHighlightChange(const sptr<HighlightNotifyInfo>& highlightNotifyInfo, bool isHighlight) override;
    void NotifyHighlightChange(bool isHighlight);

    /*
     * PC Fold Screen
     */
    bool IsWaterfallModeEnabled() override;
    WMError RegisterWaterfallModeChangeListener(const sptr<IWaterfallModeChangeListener>& listener) override;
    WMError UnregisterWaterfallModeChangeListener(const sptr<IWaterfallModeChangeListener>& listener) override;

    /*
     * Window Rotation
     */
    WMError RegisterWindowRotationChangeListener(const sptr<IWindowRotationChangeListener>& listener) override;
    WMError UnregisterWindowRotationChangeListener(const sptr<IWindowRotationChangeListener>& listener) override;
    RotationChangeResult NotifyRotationChange(const RotationChangeInfo& rotationChangeInfo) override;
    WMError CheckMultiWindowRect(uint32_t& width, uint32_t& height);
    WSError SetCurrentRotation(int32_t currentRotation) override;
    void UpdateCurrentWindowOrientation(DisplayOrientation displayOrientation);
    DisplayOrientation GetCurrentWindowOrientation() const;
    Orientation ConvertUserOrientationToUserPageOrientation(Orientation orientation) const;
    Orientation ConvertInvalidOrientation();
    void SetUserRequestedOrientation(Orientation orientation) override;
    bool IsUserOrientation(Orientation orientation) const;
    bool IsUserPageOrientation(Orientation orientation) const;
    bool isNeededForciblySetOrientation(Orientation orientation) override;
    WMError SetFollowScreenChange(bool isFollowScreenChange) override;
    void BeginRSTransaction(const std::shared_ptr<RSTransaction>& rsTransaction) const;
    WSError NotifyPageRotationIsIgnored() override;
    WMError ConvertOrientationAndRotation(const RotationInfoType from, const RotationInfoType to,
        const int32_t value, int32_t& convertedValue) override;

    /*
     * UIExtension
     */
    WSError NotifyExtensionSecureLimitChange(bool isLimit) override;

    /*
     * RS Client Multi Instance
     */
    std::shared_ptr<RSUIDirector> GetRSUIDirector() const override;
    std::shared_ptr<RSUIContext> GetRSUIContext() const override;

    /*
     * Window LifeCycle
     */
    void NotifyLifecyclePausedStatus() override;
    void NotifyAppUseControlStatus(bool isUseControl) override;
    void NotifyAfterLifecycleForeground();
    void NotifyAfterLifecycleBackground();
    void NotifyAfterLifecycleResumed();
    void NotifyAfterLifecyclePaused();
    WMError GetRouterStackInfo(std::string& routerStackInfo) override;
    void SetNavDestinationInfo(const std::string& navDestinationInfo) override;

    WSError UpdateIsShowDecorInFreeMultiWindow(bool isShow) override;

protected:
    WMError Connect();
    bool IsWindowSessionInvalid() const;
    void NotifyWindowAfterUnfocused();
    void NotifyWindowAfterFocused();
    void NotifyAfterActive();
    void NotifyAfterInactive();
    void NotifyBeforeDestroy(std::string windowName);
    void NotifyAfterDestroy();
    template<typename T> WMError RegisterListener(std::vector<sptr<T>>& holder, const sptr<T>& listener);
    template<typename T> WMError UnregisterListener(std::vector<sptr<T>>& holder, const sptr<T>& listener);
    void ClearListenersById(int32_t persistentId);
    void NotifyDmsDisplayMove(DisplayId to);

    /*
     * Free Multi Window
     */
    void ClearSwitchFreeMultiWindowListenersById(int32_t persistentId);
    void NotifySwitchFreeMultiWindow(bool enable);

    void ClearVsyncStation();
    WMError WindowSessionCreateCheck();
    void UpdateDecorEnableToAce(bool isDecorEnable);
    void NotifyModeChange(WindowMode mode, bool hasDeco = true);
    void NotifyFreeWindowModeChange(bool isInFreeWindowMode);
    WMError UpdateProperty(WSPropertyChangeAction action);
    WMError SetBackgroundColor(uint32_t color);
    uint32_t GetBackgroundColor() const;
    virtual WMError SetLayoutFullScreenByApiVersion(bool status);
    virtual float GetVirtualPixelRatio(const sptr<DisplayInfo>& displayInfo);
    void UpdateViewportConfig(const Rect& rect, WindowSizeChangeReason reason,
        const std::shared_ptr<RSTransaction>& rsTransaction = nullptr,
        const sptr<DisplayInfo>& info = nullptr,
        const std::map<AvoidAreaType, AvoidArea>& avoidAreas = {});
    void NotifySizeChange(Rect rect, WindowSizeChangeReason reason);
    void NotifyGlobalDisplayRectChange(const Rect& rect, WindowSizeChangeReason reason);
    void NotifyUIExtHostWindowRectChangeListeners(const Rect rect, const WindowSizeChangeReason reason);
    void NotifyUIExtHostRectChangeInGlobalDisplayListeners(const Rect& rect, const WindowSizeChangeReason reason);
    static sptr<Window> FindWindowById(uint32_t winId);
    void NotifyWindowStatusChange(WindowMode mode);
    void NotifyTransformChange(const Transform& transForm) override;
    void NotifySingleHandTransformChange(const SingleHandTransform& singleHandTransform) override;
    bool IsKeyboardEvent(const std::shared_ptr<MMI::KeyEvent>& keyEvent) const;
    WMError HandleEscKeyEvent(const std::shared_ptr<MMI::KeyEvent>& keyEvent, bool& isConsumed);
    void DispatchKeyEventCallback(const std::shared_ptr<MMI::KeyEvent>& keyEvent, bool& isConsumed);
    bool IsVerticalOrientation(Orientation orientation) const;
    bool IsHorizontalOrientation(Orientation orientation) const;
    void CopyUniqueDensityParameter(sptr<WindowSessionImpl> parentWindow);
    sptr<WindowSessionImpl> FindMainWindowWithContext() const;
    sptr<WindowSessionImpl> FindExtensionWindowWithContext() const;

    WMError RegisterExtensionAvoidAreaChangeListener(const sptr<IAvoidAreaChangedListener>& listener);
    WMError UnregisterExtensionAvoidAreaChangeListener(const sptr<IAvoidAreaChangedListener>& listener);

    void RefreshNoInteractionTimeoutMonitor();
    WindowStatus GetWindowStatusInner(WindowMode mode);

    /*
     * PC Event Filter
     */
    bool FilterKeyEvent(const std::shared_ptr<MMI::KeyEvent>& keyEvent);
    bool FilterPointerEvent(const std::shared_ptr<MMI::PointerEvent>& pointerEvent);
    WMError SetKeyEventFilter(KeyEventFilterFunc filter) override;
    WMError ClearKeyEventFilter() override;
    WMError SetMouseEventFilter(MouseEventFilterFunc filter) override;
    WMError ClearMouseEventFilter() override;
    WMError SetTouchEventFilter(TouchEventFilterFunc filter) override;
    WMError ClearTouchEventFilter() override;

    /*
     * UIExtension
     */
    std::unordered_set<int32_t> rectChangeUIExtListenerIds_;
    std::unordered_set<int32_t> keyboardDidShowUIExtListenerIds_;
    std::unordered_set<int32_t> keyboardDidHideUIExtListenerIds_;
    std::unordered_set<int32_t> rectChangeInGlobalDisplayUIExtListenerIds_;
    std::unordered_map<int32_t, sptr<IKeyboardDidShowListener>> keyboardDidShowUIExtListeners_;
    std::unordered_map<int32_t, sptr<IKeyboardDidHideListener>> keyboardDidHideUIExtListeners_;
    bool followCreatorLifecycle_ = false;
    bool isHiddenFollowingUIExtension_ = false;
    void WriteKeyboardInfoToWant(AAFwk::Want& want, const KeyboardPanelInfo& keyboardPanelInfo) const;
    void ReadKeyboardInfoFromWant(const AAFwk::Want& want, KeyboardPanelInfo& keyboardPanelInfo) const;
    static std::set<sptr<WindowSessionImpl>>& GetWindowExtensionSessionSet();
    void SetIsHiddenFollowingUIExtension(bool isHiddenFollowingUIExtension)
    {
        isHiddenFollowingUIExtension_ = isHiddenFollowingUIExtension;
    }
    bool IsHiddenFollowingUIExtension() { return isHiddenFollowingUIExtension_; }

    /*
     * Sub Window
     */
    void UpdateSubWindowStateAndNotify(int32_t parentPersistentId, const WindowState newState);
    void UpdateSubWindowStateWithOptions(const StateChangeOption& option);
    void DestroySubWindow();
    bool IsSubWindowMaximizeSupported() const override;
    void UpdateSubWindowInfo(uint32_t subWindowLevel, const std::shared_ptr<AbilityRuntime::Context>& context);
    void GetSubWindows(int32_t parentPersistentId, std::vector<sptr<WindowSessionImpl>>& subWindows);
    void RemoveSubWindow(int32_t parentPersistentId);

    sptr<WindowOption> windowOption_;
    sptr<ISession> hostSession_;
    mutable std::mutex hostSessionMutex_;
    std::shared_ptr<Ace::UIContent> uiContent_;
    mutable std::shared_mutex uiContentMutex_;
    std::shared_ptr<AbilityRuntime::Context> context_;
    mutable std::shared_mutex contextMutex_;
    std::shared_ptr<RSSurfaceNode> surfaceNode_;

    sptr<WindowSessionProperty> property_;
    SystemSessionConfig windowSystemConfig_;
    NotifyNativeWinDestroyFunc notifyNativeFunc_;

    std::recursive_mutex mutex_;
    static std::map<std::string, std::pair<int32_t, sptr<WindowSessionImpl>>> windowSessionMap_;
    // protect windowSessionMap_
    static std::shared_mutex windowSessionMutex_;
    // protect g_windowExtensionSessionSet_
    static std::shared_mutex windowExtensionSessionMutex_;
    bool isSystembarPropertiesSet_ = false;
    bool isIgnoreSafeAreaNeedNotify_ = false;
    bool isIgnoreSafeArea_ = false;
    std::atomic_bool isFocused_ = false;
    std::atomic_bool isHighlighted_ = false;
    std::atomic_bool shouldReNotifyHighlight_ = false;
    static std::atomic<int64_t> updateFocusTimeStamp_;
    static std::atomic<int64_t> updateHighlightTimeStamp_;
    std::shared_ptr<AppExecFwk::EventHandler> handler_ = nullptr;
    bool shouldReNotifyFocus_ = false;
    std::shared_ptr<VsyncStation> vsyncStation_ = nullptr;
    std::shared_ptr<IInputEventConsumer> inputEventConsumer_;
    bool useUniqueDensity_ { false };
    float virtualPixelRatio_ { 1.0f };
    bool escKeyEventTriggered_ = false;
    bool escKeyHasDown_ { false };
    // Check whether the UIExtensionAbility process is started
    static bool isUIExtensionAbilityProcess_;
    WSError SwitchFreeMultiWindow(bool enable) override;
    std::string identityToken_ = { "" };
    void MakeSubOrDialogWindowDragableAndMoveble();
    bool IsFreeMultiWindowMode() const
    {
        return windowSystemConfig_.IsFreeMultiWindowMode();
    }

    /*
     * Sub Window
     */
    static std::recursive_mutex subWindowSessionMutex_;
    static std::map<int32_t, std::vector<sptr<WindowSessionImpl>>> subWindowSessionMap_;

    /*
     * Compatible Mode
     */
    float compatScaleX_ = 1.0f;
    float compatScaleY_ = 1.0f;
    std::atomic_bool isFullScreenInForceSplit_ { false };
    std::vector<sptr<IUIContentCreateListener>> uiContentCreateListeners_;

    /*
     * DFX
     */
    void SetUIContentComplete();
    void AddSetUIContentTimeoutCheck();
    void NotifySetUIContentComplete();
    std::atomic_bool setUIContentCompleted_ { false };
    void SetUIExtensionDestroyComplete();
    void SetUIExtensionDestroyCompleteInSubWindow();
    void AddSetUIExtensionDestroyTimeoutCheck();
    std::atomic_bool setUIExtensionDestroyCompleted_ { false };
    std::atomic_bool startUIExtensionDestroyTimer_ { false };
    enum TimeoutErrorCode : int32_t {
        SET_UICONTENT_TIMEOUT = 1000,
        SET_UIEXTENSION_DESTROY_TIMEOUT
    };

    /*
     * Window Lifecycle
     */
    mutable std::mutex appUseControlMutex_;
    bool hasFirstNotifyInteractive_ = false;
    bool isAppUseControl_ = false;
    bool interactive_ = true;
    bool isDidForeground_ = false;
    bool isInteractiveStateFlag_ = false;
    std::string intentParam_;
    std::function<void()> loadPageCallback_;
    bool isColdStart_ = true;
    bool isIntentColdStart_ = true;
    std::string navDestinationInfo_;
    sptr<LifecycleFutureCallback> lifecycleCallback_ = nullptr;

    /*
     * Window Layout
     */
    std::atomic_bool isDragTaskPostDone_ = true;
    void FlushLayoutSize(int32_t width, int32_t height) override;
    sptr<FutureCallback> layoutCallback_ = nullptr;
    sptr<FutureCallback> getTargetInfoCallback_ = nullptr;
    sptr<FutureCallback> getRotationResultFuture_ = nullptr;
    sptr<FutureCallback> updateRectCallback_ = nullptr;
    void UpdateVirtualPixelRatio(const sptr<Display>& display);
    WMError GetVirtualPixelRatio(float& vpr);
    void SetCurrentTransform(const Transform& transform);
    Transform GetCurrentTransform() const;
    void NotifyAfterUIContentReady();
    void SetNeedRenotifyTransform(bool isNeedRenotify) { needRenotifyTransform_.store(isNeedRenotify); }
    bool IsNeedRenotifyTransform() const { return needRenotifyTransform_.load(); }
    mutable std::recursive_mutex transformMutex_;
    std::atomic<CrossAxisState> crossAxisState_ = CrossAxisState::STATE_INVALID;
    bool IsValidCrossState(int32_t state) const;
    template <typename T>
    EnableIfSame<T, IWindowCrossAxisListener, std::vector<sptr<IWindowCrossAxisListener>>> GetListeners();
    void NotifyWindowStatusDidChange(WindowMode mode);
    void NotifyFirstValidLayoutUpdate(const Rect& preRect, const Rect& newRect);
    std::atomic_bool hasSetEnableDrag_ = false;
    void HookWindowSizeByHookWindowInfo(Rect& rect);
    void SetAppHookWindowInfo(const HookWindowInfo& hookWindowInfo);
    HookWindowInfo GetAppHookWindowInfo();
    virtual WMError GetAppHookWindowInfoFromServer(HookWindowInfo& hookWindowInfo) { return WMError::WM_OK; }

    /*
     * Window Immersive
     */
    virtual void UpdateDefaultStatusBarColor() { return; }
    WMError UpdateStatusBarColorByColorMode(uint32_t& contentColor);
    std::map<AvoidAreaType, AvoidArea> lastAvoidAreaMap_;
    uint32_t GetStatusBarHeight() const override;
    WindowType rootHostWindowType_ = WindowType::APP_MAIN_WINDOW_BASE;
    SystemBarSettingFlag systemBarSettingFlag_ = SystemBarSettingFlag::DEFAULT_SETTING;

    /*
     * PC Fold Screen
     */
    bool supportEnterWaterfallMode_ { false };
    std::atomic_bool isFullScreenWaterfallMode_ { false };
    std::atomic_bool isValidWaterfallMode_ { false };
    bool isFirstNotifyAcrossDisplays_ = true;
    bool isAcrossDisplays_ = false;
    WMError NotifyAcrossDisplaysChange(bool isAcrossDisplays);
    void NotifyWaterfallModeChange(bool isWaterfallMode);
    std::vector<sptr<IWaterfallModeChangeListener>> GetWaterfallModeChangeListeners();

    /*
     * Window Pattern
     */
    WMError NotifySnapshotUpdate() override;

    /*
     * Window Property
     */
    std::string colorMode_;
    bool hasDarkRes_;
    std::string specifiedColorMode_;
    WindowVisibilityState lastVisibilityState_ = WindowVisibilityState::WINDOW_VISIBILITY_STATE_TOTALLY_OCCUSION;
    std::unordered_set<std::string> containerColorList_;
    float lastSystemDensity_ = UNDEFINED_DENSITY;
    std::atomic<bool> isDefaultDensityEnabled_ = false;
    std::atomic<bool> defaultDensityEnabledStageConfig_ = false;
    WSError NotifySystemDensityChange(float density);
    void RegisterWindowInspectorCallback();
    uint32_t GetTargetAPIVersionByApplicationInfo() const;

    /*
     * Window Input Event
     */
    bool GetWatchGestureConsumed() const;
    void SetWatchGestureConsumed(bool isWatchGestureConsumed);
    bool dialogSessionBackGestureEnabled_ = false;

    /*
     * Window Rotation
     */
    int16_t rotationAnimationCount_ { 0 };
    void NotifyRotationAnimationEnd();

    /*
     * Keyboard
     */
    sptr<OccupiedAreaChangeInfo> occupiedAreaInfo_ = nullptr;

    /*
     * Window Decor
     */
    bool grayOutMaximizeButton_ = false;
    void NotifyTitleChange(bool isShow, int32_t height);
    
private:
    void InitPropertyFromOption(const sptr<WindowOption>& option);
    //Trans between colorGamut and colorSpace
    static ColorSpace GetColorSpaceFromSurfaceGamut(GraphicColorGamut colorGamut);
    static GraphicColorGamut GetSurfaceGamutFromColorSpace(ColorSpace colorSpace);

    template<typename T> EnableIfSame<T, IWindowLifeCycle, std::vector<sptr<IWindowLifeCycle>>> GetListeners();
    template<typename T>
    EnableIfSame<T, IWindowStageLifeCycle, std::vector<sptr<IWindowStageLifeCycle>>> GetListeners();
    template<typename T> EnableIfSame<T, IDisplayMoveListener, std::vector<sptr<IDisplayMoveListener>>> GetListeners();
    template<typename T>
    EnableIfSame<T, IWindowChangeListener, std::vector<sptr<IWindowChangeListener>>> GetListeners();
    template<typename T>
    EnableIfSame<T, IAvoidAreaChangedListener, std::vector<sptr<IAvoidAreaChangedListener>>> GetListeners();
    template<typename T>
    EnableIfSame<T, IDialogDeathRecipientListener, std::vector<sptr<IDialogDeathRecipientListener>>> GetListeners();
    template<typename T>
    EnableIfSame<T, IDialogTargetTouchListener, std::vector<sptr<IDialogTargetTouchListener>>> GetListeners();
    template<typename T>
    EnableIfSame<T, IOccupiedAreaChangeListener, std::vector<sptr<IOccupiedAreaChangeListener>>> GetListeners();
    template<typename T>
    EnableIfSame<T, IKeyboardDidShowListener, std::vector<sptr<IKeyboardDidShowListener>>> GetListeners();
    template<typename T>
    EnableIfSame<T, IKeyboardDidHideListener, std::vector<sptr<IKeyboardDidHideListener>>> GetListeners();
    template<typename T>
    EnableIfSame<T, IKBWillShowListener, std::vector<sptr<IKBWillShowListener>>> GetListeners();
    template<typename T>
    EnableIfSame<T, IKBWillHideListener, std::vector<sptr<IKBWillHideListener>>> GetListeners();
    template<typename T>
    EnableIfSame<T, IScreenshotListener, std::vector<sptr<IScreenshotListener>>> GetListeners();
    template<typename T>
    EnableIfSame<T, IScreenshotAppEventListener, std::vector<IScreenshotAppEventListenerSptr>> GetListeners();
    template<typename T>
    EnableIfSame<T, ITouchOutsideListener, std::vector<sptr<ITouchOutsideListener>>> GetListeners();
    template<typename T>
    EnableIfSame<T, IWindowVisibilityChangedListener, std::vector<IWindowVisibilityListenerSptr>> GetListeners();
    template<typename T>
    EnableIfSame<T, IDisplayIdChangeListener, std::vector<IDisplayIdChangeListenerSptr>> GetListeners();
    template<typename T>
    EnableIfSame<T, ISystemDensityChangeListener, std::vector<ISystemDensityChangeListenerSptr>> GetListeners();
    template<typename T>
    EnableIfSame<T, IAcrossDisplaysChangeListener, std::vector<IAcrossDisplaysChangeListenerSptr>> GetListeners();
    template<typename T>
    EnableIfSame<T, IWindowNoInteractionListener, std::vector<IWindowNoInteractionListenerSptr>> GetListeners();
    template<typename T> void ClearUselessListeners(std::map<int32_t, T>& listeners, int32_t persistentId);
    template<typename T> void ClearUselessListeners(std::unordered_map<int32_t, T>& listeners, int32_t persistentId);
    RSSurfaceNode::SharedPtr CreateSurfaceNode(const std::string& name, WindowType type);
    template<typename T>
    EnableIfSame<T, IWindowStatusChangeListener, std::vector<sptr<IWindowStatusChangeListener>>> GetListeners();
    template<typename T>
    EnableIfSame<T, IWindowStatusDidChangeListener, std::vector<sptr<IWindowStatusDidChangeListener>>> GetListeners();
    template<typename T>
    EnableIfSame<T, IWindowRectChangeListener, std::vector<sptr<IWindowRectChangeListener>>> GetListeners();
    template<typename T>
    EnableIfSame<T, IWindowTitleChangeListener, std::vector<sptr<IWindowTitleChangeListener>>> GetListeners();
    template<typename T>
    EnableIfSame<T, IWindowTitleOrHotAreasListener, std::vector<sptr<IWindowTitleOrHotAreasListener>>> GetListeners();
    template<typename T>
    EnableIfSame<T, IRectChangeInGlobalDisplayListener, std::vector<sptr<IRectChangeInGlobalDisplayListener>>>
        GetListeners();
    template<typename T>
    EnableIfSame<T, IExtensionSecureLimitChangeListener, std::vector<sptr<IExtensionSecureLimitChangeListener>>>
        GetListeners();
    template<typename T>
    EnableIfSame<T, IPreferredOrientationChangeListener, sptr<IPreferredOrientationChangeListener>> GetListeners();
    template<typename T>
    EnableIfSame<T, IWindowOrientationChangeListener, sptr<IWindowOrientationChangeListener>> GetListeners();
    template<typename T>
    EnableIfSame<T, ISwitchFreeMultiWindowListener, std::vector<sptr<ISwitchFreeMultiWindowListener>>> GetListeners();
    template<typename T>
    EnableIfSame<T, IWindowHighlightChangeListener, std::vector<sptr<IWindowHighlightChangeListener>>> GetListeners();
    template<typename T>
    EnableIfSame<T, ISystemBarPropertyListener, std::vector<sptr<ISystemBarPropertyListener>>> GetListeners();
    template<typename T>
    EnableIfSame<T, IWindowRotationChangeListener, std::vector<sptr<IWindowRotationChangeListener>>> GetListeners();
    template<typename T>
    EnableIfSame<T, IFreeWindowModeChangeListener, std::vector<sptr<IFreeWindowModeChangeListener>>> GetListeners();
    void NotifyAfterFocused();
    void NotifyUIContentFocusStatus();
    void NotifyAfterUnfocused(bool needNotifyUiContent = true);
    void NotifyAfterResumed();
    void NotifyAfterPaused();
    void NotifyUIContentHighlightStatus(bool isHighlighted);

    /*
     * Window Decor listener
     */
    template<typename T>
    EnableIfSame<T, IWindowTitleButtonRectChangedListener,
        std::vector<sptr<IWindowTitleButtonRectChangedListener>>> GetListeners();
    template<typename T>
    EnableIfSame<T, ISubWindowCloseListener, sptr<ISubWindowCloseListener>> GetListeners();
    template<typename T>
    EnableIfSame<T, IMainWindowCloseListener, sptr<IMainWindowCloseListener>> GetListeners();
    template<typename T>
    EnableIfSame<T, IWindowWillCloseListener, std::vector<sptr<IWindowWillCloseListener>>> GetListeners();
    std::unique_ptr<Ace::UIContent> UIContentCreate(AppExecFwk::Ability* ability, void* env, int isAni);
    Ace::UIContentErrorCode UIContentInitByName(Ace::UIContent*, const std::string&, void* storage, int isAni);
    template<typename T>
    Ace::UIContentErrorCode UIContentInit(Ace::UIContent*, T contentInfo, void* storage, int isAni);
    Ace::UIContentErrorCode UIContentRestore(Ace::UIContent*, const std::string& contentInfo, void* storage,
        Ace::ContentInfoType infoType, int isAni);
    WMError InitUIContent(const std::string& contentInfo, void* env, void* storage,
        WindowSetUIContentType setUIContentType, BackupAndRestoreType restoreType, AppExecFwk::Ability* ability,
        OHOS::Ace::UIContentErrorCode& aceRet, int isAni = 0);
    void UpdateConfigWhenSetUIContent();
    WMError SetUIContentInner(const std::string& contentInfo, void* env, void* storage,
        WindowSetUIContentType setUIContentType, BackupAndRestoreType restoreType, AppExecFwk::Ability* ability,
        int isAni = 0);
    std::shared_ptr<std::vector<uint8_t>> GetAbcContent(const std::string& abcPath);
    void RegisterUIContenCallback();
    inline void DestroyExistUIContent();
    std::string GetRestoredRouterStack();

    bool CheckIfNeedCommitRsTransaction(WindowSizeChangeReason wmReason);
    void UpdateRectForResizeAnimation(const Rect& wmRect, const Rect& preRect,
        WindowSizeChangeReason wmReason, const SceneAnimationConfig& config,
        const std::map<AvoidAreaType, AvoidArea>& avoidAreas = {});
    RSAnimationTimingCurve updateConfigCurve(const WindowAnimationCurve& configCurve,
        const std::array<float, ANIMATION_PARAM_SIZE> param);
    void UpdateRectForRotation(const Rect& wmRect, const Rect& preRect, WindowSizeChangeReason wmReason,
        const SceneAnimationConfig& config, const std::map<AvoidAreaType, AvoidArea>& avoidAreas = {});
    void UpdateRectForPageRotation(const Rect& wmRect, const Rect& preRect, WindowSizeChangeReason wmReason,
        const SceneAnimationConfig& config, const std::map<AvoidAreaType, AvoidArea>& avoidAreas = {});
    void UpdateRectForOtherReason(const Rect& wmRect, const Rect& preRect, WindowSizeChangeReason wmReason,
        const std::shared_ptr<RSTransaction>& rsTransaction = nullptr,
        const std::map<AvoidAreaType, AvoidArea>& avoidAreas = {});
    void UpdateRectForOtherReasonTask(const Rect& wmRect, const Rect& preRect, WindowSizeChangeReason wmReason,
        const std::shared_ptr<RSTransaction>& rsTransaction,
        const std::map<AvoidAreaType, AvoidArea>& avoidAreas = {});
    void SubmitNoInteractionMonitorTask(int32_t eventId, const IWindowNoInteractionListenerSptr& listener);
    virtual WMError GetAppForceLandscapeConfig(AppForceLandscapeConfig& config) { return WMError::WM_OK; };
    virtual WMError GetAppForceLandscapeConfigEnable(bool& enableForceSplit) { return WMError::WM_OK; };
    WSError NotifyAppForceLandscapeConfigUpdated() override;
    WSError NotifyAppForceLandscapeConfigEnableUpdated() override;
    void SetFrameLayoutCallbackEnable(bool enable);
    void UpdateFrameLayoutCallbackIfNeeded(WindowSizeChangeReason wmReason);
    bool IsNotifyInteractiveDuplicative(bool interactive);
    void SetUniqueVirtualPixelRatioForSub(bool useUniqueDensity, float virtualPixelRatio);
    bool IsWindowShouldDrag();
    bool CheckCanDragWindowType();

    /*
     * PC Window
     */
    void GetTitleButtonVisible(bool& hideMaximizeButton, bool& hideMinimizeButton, bool& hideSplitButton,
        bool& hideCloseButton);

    /*
     * PC Fold Screen
     */
    bool waterfallModeWhenEnterBackground_ { false };
    static std::mutex waterfallModeChangeListenerMutex_;
    static std::unordered_map<int32_t, std::vector<sptr<IWaterfallModeChangeListener>>> waterfallModeChangeListeners_;
    bool InitWaterfallMode();

    static std::recursive_mutex lifeCycleListenerMutex_;
    static std::recursive_mutex windowStageLifeCycleListenerMutex_;
    static std::recursive_mutex windowChangeListenerMutex_;
    static std::recursive_mutex windowCrossAxisListenerMutex_;
    static std::recursive_mutex avoidAreaChangeListenerMutex_;
    static std::recursive_mutex dialogDeathRecipientListenerMutex_;
    static std::recursive_mutex dialogTargetTouchListenerMutex_;
    static std::recursive_mutex occupiedAreaChangeListenerMutex_;
    static std::recursive_mutex keyboardWillShowListenerMutex_;
    static std::recursive_mutex keyboardWillHideListenerMutex_;
    static std::recursive_mutex keyboardDidShowListenerMutex_;
    static std::recursive_mutex keyboardDidHideListenerMutex_;
    static std::recursive_mutex screenshotListenerMutex_;
    static std::recursive_mutex touchOutsideListenerMutex_;
    static std::recursive_mutex windowVisibilityChangeListenerMutex_;
    static std::recursive_mutex windowNoInteractionListenerMutex_;
    static std::recursive_mutex windowStatusChangeListenerMutex_;
    static std::recursive_mutex windowStatusDidChangeListenerMutex_;
    static std::mutex displayMoveListenerMutex_;
    static std::mutex windowRectChangeListenerMutex_;
    static std::mutex windowTitleChangeListenerMutex_;
    static std::mutex windowTitleOrHotAreasListenerMutex_;
    static std::mutex rectChangeInGlobalDisplayListenerMutex_;
    static std::mutex secureLimitChangeListenerMutex_;
    static std::mutex switchFreeMultiWindowListenerMutex_;
    static std::mutex preferredOrientationChangeListenerMutex_;
    static std::mutex windowOrientationChangeListenerMutex_;
    static std::mutex highlightChangeListenerMutex_;
    static std::mutex systemBarPropertyListenerMutex_;
    static std::mutex windowRotationChangeListenerMutex_;
    static std::mutex freeWindowModeChangeListenerMutex_;
    static std::mutex occlusionStateChangeListenerMutex_;
    static std::unordered_map<int32_t,
        std::vector<sptr<IOcclusionStateChangedListener>>> occlusionStateChangeListeners_;
    static std::mutex frameMetricsChangeListenerMutex_;
    static std::unordered_map<int32_t, std::vector<sptr<IFrameMetricsChangedListener>>> frameMetricsChangeListeners_;
    static std::map<int32_t, std::vector<sptr<ISystemBarPropertyListener>>> systemBarPropertyListeners_;
    static std::map<int32_t, std::vector<sptr<IWindowLifeCycle>>> lifecycleListeners_;
    static std::map<int32_t, std::vector<sptr<IWindowStageLifeCycle>>> windowStageLifecycleListeners_;
    static std::map<int32_t, std::vector<sptr<IDisplayMoveListener>>> displayMoveListeners_;
    static std::map<int32_t, std::vector<sptr<IWindowChangeListener>>> windowChangeListeners_;
    static std::map<int32_t, std::vector<sptr<IWindowCrossAxisListener>>> windowCrossAxisListeners_;
    static std::map<int32_t, std::vector<sptr<IAvoidAreaChangedListener>>> avoidAreaChangeListeners_;
    static std::map<int32_t, std::vector<sptr<IDialogDeathRecipientListener>>> dialogDeathRecipientListeners_;
    static std::map<int32_t, std::vector<sptr<IDialogTargetTouchListener>>> dialogTargetTouchListener_;
    static std::map<int32_t, std::vector<sptr<IOccupiedAreaChangeListener>>> occupiedAreaChangeListeners_;
    static std::map<int32_t, std::vector<sptr<IKBWillShowListener>>> keyboardWillShowListeners_;
    static std::map<int32_t, std::vector<sptr<IKBWillHideListener>>> keyboardWillHideListeners_;
    static std::map<int32_t, std::vector<sptr<IKeyboardDidShowListener>>> keyboardDidShowListeners_;
    static std::map<int32_t, std::vector<sptr<IKeyboardDidHideListener>>> keyboardDidHideListeners_;
    static std::map<int32_t, std::vector<sptr<IScreenshotListener>>> screenshotListeners_;
    static std::recursive_mutex screenshotAppEventListenerMutex_;
    static std::unordered_map<int32_t, std::vector<IScreenshotAppEventListenerSptr>> screenshotAppEventListeners_;
    static std::map<int32_t, std::vector<sptr<ITouchOutsideListener>>> touchOutsideListeners_;
    static std::map<int32_t, std::vector<IWindowVisibilityListenerSptr>> windowVisibilityChangeListeners_;
    static std::mutex displayIdChangeListenerMutex_;
    static std::map<int32_t, std::vector<IDisplayIdChangeListenerSptr>> displayIdChangeListeners_;
    static std::mutex systemDensityChangeListenerMutex_;
    static std::unordered_map<int32_t, std::vector<ISystemDensityChangeListenerSptr>> systemDensityChangeListeners_;
    static std::recursive_mutex acrossDisplaysChangeListenerMutex_;
    static std::unordered_map<int32_t, std::vector<IAcrossDisplaysChangeListenerSptr>>
        acrossDisplaysChangeListeners_;
    static std::map<int32_t, std::vector<IWindowNoInteractionListenerSptr>> windowNoInteractionListeners_;
    static std::map<int32_t, std::vector<sptr<IWindowStatusChangeListener>>> windowStatusChangeListeners_;
    static std::map<int32_t, std::vector<sptr<IWindowStatusDidChangeListener>>> windowStatusDidChangeListeners_;
    static std::map<int32_t, std::vector<sptr<IWindowRectChangeListener>>> windowRectChangeListeners_;
    static std::map<int32_t, std::vector<sptr<IWindowTitleChangeListener>>> windowTitleChangeListeners_;
    static std::map<int32_t, std::vector<sptr<IWindowTitleOrHotAreasListener>>> windowTitleOrHotAreasListeners_;
    static std::map<int32_t, std::vector<sptr<IRectChangeInGlobalDisplayListener>>> rectChangeInGlobalDisplayListeners_;
    static std::map<int32_t, std::vector<sptr<IExtensionSecureLimitChangeListener>>> secureLimitChangeListeners_;
    static std::map<int32_t, std::vector<sptr<ISwitchFreeMultiWindowListener>>> switchFreeMultiWindowListeners_;
    static std::map<int32_t, sptr<IPreferredOrientationChangeListener>> preferredOrientationChangeListener_;
    static std::map<int32_t, sptr<IWindowOrientationChangeListener>> windowOrientationChangeListener_;
    static std::map<int32_t, std::vector<sptr<IWindowHighlightChangeListener>>> highlightChangeListeners_;
    static std::map<int32_t, std::vector<sptr<IWindowRotationChangeListener>>> windowRotationChangeListeners_;
    static std::map<int32_t, std::vector<sptr<IFreeWindowModeChangeListener>>> freeWindowModeChangeListeners_;

    // FA only
    sptr<IAceAbilityHandler> aceAbilityHandler_;

    std::atomic<int32_t> lastInteractionEventId_ { 0 };

    bool isMainHandlerAvailable_ = true;

    std::string subWindowTitle_ = { "" };
    std::string dialogTitle_ = { "" };
    WindowTitleVisibleFlags windowTitleVisibleFlags_;

    std::string restoredRouterStack_; // It was set and get in same thread, which is js thread.

    /*
     * Window Layout
     */
    void UpdateRectChangeListenerRegisterStatus();
    WSRect layoutRect_;
    std::atomic_bool windowSizeChanged_ = false;
    std::atomic_bool enableFrameLayoutFinishCb_ = false;
    std::atomic_bool dragActivated_ = true;
    WindowSizeChangeReason lastSizeChangeReason_ = WindowSizeChangeReason::END;
    bool postTaskDone_ = false;
    Transform layoutTransform_;
    SingleHandTransform singleHandTransform_;
    mutable std::mutex currentTransformMutex_;
    Transform currentTransform_;
    std::atomic_bool needRenotifyTransform_ = false;
    KeyFramePolicy keyFramePolicy_;
    std::atomic<WindowStatus> lastWindowStatus_ = WindowStatus::WINDOW_STATUS_UNDEFINED;
    std::atomic<WindowStatus> lastStatusWhenNotifyWindowStatusDidChange_ = WindowStatus::WINDOW_STATUS_UNDEFINED;
    std::atomic<bool> isFirstValidLayoutUpdate_ = true;
    SizeChangeReason globalDisplayRectSizeChangeReason_ = SizeChangeReason::END;
    std::shared_mutex hookWindowInfoMutex_;
    HookWindowInfo hookWindowInfo_;
    std::atomic_bool notifySizeChangeFlag_ = false;

    /*
     * Window Decor
     */
    DecorButtonStyle decorButtonStyle_;
    int32_t decorHeight_ = 0;

    /*
     * Window Decor listener
     */
    static std::recursive_mutex windowTitleButtonRectChangeListenerMutex_;
    static std::map<int32_t, std::vector<sptr<IWindowTitleButtonRectChangedListener>>>
        windowTitleButtonRectChangeListeners_;
    static std::mutex subWindowCloseListenersMutex_;
    static std::map<int32_t, sptr<ISubWindowCloseListener>> subWindowCloseListeners_;
    static std::mutex mainWindowCloseListenersMutex_;
    static std::map<int32_t, sptr<IMainWindowCloseListener>> mainWindowCloseListeners_;
    static std::recursive_mutex windowWillCloseListenersMutex_;
    static std::unordered_map<int32_t, std::vector<sptr<IWindowWillCloseListener>>> windowWillCloseListeners_;

    /*
     * Multi Window
     */
    bool isSplitButtonVisible_ = true;

    /*
     * PC Window
     */
    uint32_t targetAPIVersion_ = 0;
    std::shared_ptr<Media::PixelMap> iconCache_;

    /*
     * Window Input Event
     */
    bool isWatchGestureConsumed_ = false;

    /*
     * PC Event Filter
     */
    std::mutex keyEventFilterMutex_;
    KeyEventFilterFunc keyEventFilter_;
    std::mutex mouseEventFilterMutex_;
    MouseEventFilterFunc mouseEventFilter_;
    std::mutex touchEventFilterMutex_;
    TouchEventFilterFunc touchEventFilter_;

    /*
     * Window Scene
     */
    WSError NotifyWindowAttachStateChange(bool isAttach) override { return WSError::WS_OK; }

    /*
     * Window Rotation
     */
    void NotifyClientOrientationChange();
    void NotifyRotationChangeResult(RotationChangeResult rotationChangeResult) override;
    void NotifyRotationChangeResultInner(const RotationChangeInfo& rotationChangeInfo);
    DisplayOrientation windowOrientation_ = DisplayOrientation::UNKNOWN;

    /*
     * RS Client Multi Instance
     */
    std::shared_ptr<RSUIDirector> rsUIDirector_;
};
} // namespace Rosen
} // namespace OHOS

#endif // OHOS_ROSEN_WINDOW_SESSION_IMPL_H
