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
#include <optional>
#include <shared_mutex>
#include <ability_context.h>
#include <event_handler.h>
#include <i_input_event_consumer.h>
#include <refbase.h>
#include <ui_content.h>
#include <ui/rs_surface_node.h>
#include "display_manager.h"
#include "singleton_container.h"

#include "common/include/window_session_property.h"
#include "interfaces/include/ws_common.h"
#include "interfaces/include/ws_common_inner.h"
#include "session/container/include/zidl/session_stage_stub.h"
#include "session/host/include/zidl/session_interface.h"
#include "vsync_station.h"
#include "window.h"
#include "window_option.h"
#include "wm_common.h"

namespace OHOS {
namespace Rosen {
namespace {
template<typename T1, typename T2, typename Ret>
using EnableIfSame = typename std::enable_if<std::is_same_v<T1, T2>, Ret>::type;
}

struct WindowTitleVisibleFlags {
    bool isMaximizeVisible = true;
    bool isMinimizeVisible = true;
    bool isSplitVisible = true;
};

class WindowSessionImpl : public Window, public virtual SessionStageStub {
public:
    explicit WindowSessionImpl(const sptr<WindowOption>& option);
    ~WindowSessionImpl();
    void ConsumePointerEvent(const std::shared_ptr<MMI::PointerEvent>& pointerEvent) override;
    void ConsumeKeyEvent(std::shared_ptr<MMI::KeyEvent>& inputEvent) override;
    bool PreNotifyKeyEvent(const std::shared_ptr<MMI::KeyEvent>& keyEvent) override;
    virtual bool NotifyOnKeyPreImeEvent(const std::shared_ptr<MMI::KeyEvent>& keyEvent) override;
    static sptr<Window> Find(const std::string& name);
    static std::vector<sptr<Window>> GetSubWindow(int parentId);
    // inherits from window
    virtual WMError Create(const std::shared_ptr<AbilityRuntime::Context>& context,
        const sptr<Rosen::ISession>& iSession, const std::string& identityToken = "");
    WMError Show(uint32_t reason = 0, bool withAnimation = false) override;
    WMError Hide(uint32_t reason = 0, bool withAnimation = false, bool isFromInnerkits = true) override;
    WMError Destroy() override;
    virtual WMError Destroy(bool needNotifyServer, bool needClearListener = true);
    WMError NapiSetUIContent(const std::string& contentInfo, napi_env env,
        napi_value storage, bool isdistributed, sptr<IRemoteObject> token, AppExecFwk::Ability* ability) override;
    WMError SetUIContentByName(const std::string& contentInfo, napi_env env, napi_value storage,
        AppExecFwk::Ability* ability) override;
    WMError SetUIContentByAbc(const std::string& abcPath, napi_env env, napi_value storage,
        AppExecFwk::Ability* ability) override;
    std::shared_ptr<RSSurfaceNode> GetSurfaceNode() const override;
    const std::shared_ptr<AbilityRuntime::Context> GetContext() const override;
    Rect GetRequestRect() const override;
    WindowType GetType() const override;
    const std::string& GetWindowName() const override;
    WindowState GetWindowState() const override;
    WindowState GetRequestWindowState() const;
    WMError SetFocusable(bool isFocusable) override;
    WMError SetTouchable(bool isTouchable) override;
    WMError SetTopmost(bool topmost) override;
    bool IsTopmost() const override;
    WMError SetResizeByDragEnabled(bool dragEnabled) override;
    WMError SetRaiseByClickEnabled(bool raiseEnabled) override;
    WMError HideNonSystemFloatingWindows(bool shouldHide) override;
    WMError SetSingleFrameComposerEnabled(bool enable) override;
    bool IsFloatingWindowAppType() const override;
    WMError SetWindowType(WindowType type) override;
    WMError SetBrightness(float brightness) override;
    virtual float GetBrightness() const override;
    void SetRequestedOrientation(Orientation orientation) override;
    bool GetTouchable() const override;
    uint32_t GetWindowId() const override;
    uint64_t GetDisplayId() const override;
    Rect GetRect() const override;
    bool GetFocusable() const override;
    std::string GetContentInfo() override;
    Ace::UIContent* GetUIContent() const override;
    std::shared_ptr<Ace::UIContent> GetUIContentSharedPtr() const;
    Ace::UIContent* GetUIContentWithId(uint32_t winId) const override;
    void OnNewWant(const AAFwk::Want& want) override;
    WMError SetAPPWindowLabel(const std::string& label) override;
    WMError SetAPPWindowIcon(const std::shared_ptr<Media::PixelMap>& icon) override;
    void RequestVsync(const std::shared_ptr<VsyncCallback>& vsyncCallback) override;
    int64_t GetVSyncPeriod() override;
    void FlushFrameRate(uint32_t rate, int32_t animatorExpectedFrameRate, uint32_t rateType = 0) override;
    // inherits from session stage
    WSError SetActive(bool active) override;
    WSError UpdateRect(const WSRect& rect, SizeChangeReason reason,
        const std::shared_ptr<RSTransaction>& rsTransaction = nullptr) override;
    void UpdateDensity() override;
    void SetUniqueVirtualPixelRatio(bool useUniqueDensity, float virtualPixelRatio) override;
    WSError UpdateOrientation() override;
    WSError UpdateDisplayId(uint64_t displayId) override;
    WSError UpdateFocus(bool focus) override;
    bool IsFocused() const override;
    WMError RequestFocus() const override;
    WSError UpdateWindowMode(WindowMode mode) override;
    WSError HandleBackEvent() override;
    WMError SetWindowGravity(WindowGravity gravity, uint32_t percent) override;
    WMError SetSystemBarProperty(WindowType type, const SystemBarProperty& property) override;
    KeyboardAnimationConfig GetKeyboardAnimationConfig() override;

    void NotifyPointerEvent(const std::shared_ptr<MMI::PointerEvent>& pointerEvent) override;
    void NotifyKeyEvent(const std::shared_ptr<MMI::KeyEvent>& keyEvent, bool& isConsumed,
        bool notifyInputMethod = true) override;
    void NotifyOccupiedAreaChangeInfoInner(sptr<OccupiedAreaChangeInfo> info);
    void NotifyOccupiedAreaChangeInfo(sptr<OccupiedAreaChangeInfo> info,
                                      const std::shared_ptr<RSTransaction>& rsTransaction = nullptr) override;
    void NotifyForegroundInteractiveStatus(bool interactive) override;
    void NotifyDisplayMove(DisplayId from, DisplayId to) override;

    WMError RegisterLifeCycleListener(const sptr<IWindowLifeCycle>& listener) override;
    WMError UnregisterLifeCycleListener(const sptr<IWindowLifeCycle>& listener) override;
    WMError RegisterDisplayMoveListener(sptr<IDisplayMoveListener>& listener) override;
    WMError UnregisterDisplayMoveListener(sptr<IDisplayMoveListener>& listener) override;
    WMError RegisterWindowChangeListener(const sptr<IWindowChangeListener>& listener) override;
    WMError UnregisterWindowChangeListener(const sptr<IWindowChangeListener>& listener) override;
    WMError RegisterAvoidAreaChangeListener(sptr<IAvoidAreaChangedListener>& listener) override;
    WMError UnregisterAvoidAreaChangeListener(sptr<IAvoidAreaChangedListener>& listener) override;
    void RegisterDialogDeathRecipientListener(const sptr<IDialogDeathRecipientListener>& listener) override;
    void UnregisterDialogDeathRecipientListener(const sptr<IDialogDeathRecipientListener>& listener) override;
    WMError RegisterDialogTargetTouchListener(const sptr<IDialogTargetTouchListener>& listener) override;
    WMError UnregisterDialogTargetTouchListener(const sptr<IDialogTargetTouchListener>& listener) override;
    WMError RegisterOccupiedAreaChangeListener(const sptr<IOccupiedAreaChangeListener>& listener) override;
    WMError UnregisterOccupiedAreaChangeListener(const sptr<IOccupiedAreaChangeListener>& listener) override;
    WMError RegisterTouchOutsideListener(const sptr<ITouchOutsideListener>& listener) override;
    WMError UnregisterTouchOutsideListener(const sptr<ITouchOutsideListener>& listener) override;
    WMError RegisterWindowVisibilityChangeListener(const IWindowVisibilityListenerSptr& listener) override;
    WMError UnregisterWindowVisibilityChangeListener(const IWindowVisibilityListenerSptr& listener) override;
    WMError RegisterWindowNoInteractionListener(const IWindowNoInteractionListenerSptr& listener) override;
    WMError UnregisterWindowNoInteractionListener(const IWindowNoInteractionListenerSptr& listener) override;
    void RegisterWindowDestroyedListener(const NotifyNativeWinDestroyFunc& func) override;
    void UnregisterWindowDestroyedListener() override { notifyNativeFunc_ = nullptr; }
    WMError RegisterScreenshotListener(const sptr<IScreenshotListener>& listener) override;
    WMError UnregisterScreenshotListener(const sptr<IScreenshotListener>& listener) override;
    void SetAceAbilityHandler(const sptr<IAceAbilityHandler>& handler) override;
    void SetInputEventConsumer(const std::shared_ptr<IInputEventConsumer>& inputEventConsumer) override;
    WMError SetTitleButtonVisible(bool isMaximizeVisible, bool isMinimizeVisible, bool isSplitVisible) override;

    WMError SetBackgroundColor(const std::string& color) override;
    virtual Orientation GetRequestedOrientation() override;

    int32_t GetParentId() const;
    int32_t GetPersistentId() const override;
    sptr<WindowSessionProperty> GetProperty() const;
    SystemSessionConfig GetSystemSessionConfig() const;
    sptr<ISession> GetHostSession() const;
    int32_t GetFloatingWindowParentId();
    void NotifyAfterForeground(bool needNotifyListeners = true, bool needNotifyUiContent = true);
    void NotifyAfterBackground(bool needNotifyListeners = true, bool needNotifyUiContent = true);
    void NotifyForegroundFailed(WMError ret);
    void NotifyBackgroundFailed(WMError ret);
    WSError MarkProcessed(int32_t eventId) override;
    void UpdateTitleButtonVisibility();
    WSError NotifyDestroy() override;
    WSError NotifyCloseExistPipWindow() override;
    WSError NotifyTransferComponentData(const AAFwk::WantParams& wantParams) override;
    WSErrorCode NotifyTransferComponentDataSync(const AAFwk::WantParams& wantParams,
        AAFwk::WantParams& reWantParams) override;
    void NotifyAvoidAreaChange(const sptr<AvoidArea>& avoidArea, AvoidAreaType type);
    WSError UpdateAvoidArea(const sptr<AvoidArea>& avoidArea, AvoidAreaType type) override;
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
    WSError SetPipActionEvent(const std::string& action, int32_t status) override;
    WSError SetPiPControlEvent(WsPiPControlType controlType, WsPiPControlStatus status) override;

    void UpdatePiPRect(const Rect& rect, WindowSizeChangeReason reason) override;
    void UpdatePiPControlStatus(PiPControlType controlType, PiPControlStatus status) override;
    void SetDrawingContentState(bool drawingContentState);
    WMError RegisterWindowStatusChangeListener(const sptr<IWindowStatusChangeListener>& listener) override;
    WMError UnregisterWindowStatusChangeListener(const sptr<IWindowStatusChangeListener>& listener) override;
    WMError SetSpecificBarProperty(WindowType type, const SystemBarProperty& property) override;
    virtual WMError SetSubWindowModal(bool isModal) override;
    virtual WMError SetDecorVisible(bool isVisible) override;
    virtual WMError SetDecorHeight(int32_t decorHeight) override;
    virtual WMError GetDecorHeight(int32_t& height) override;
    virtual WMError GetTitleButtonArea(TitleButtonRect& titleButtonRect) override;
    WSError GetUIContentRemoteObj(sptr<IRemoteObject>& uiContentRemoteObj) override;
    virtual WMError RegisterWindowTitleButtonRectChangeListener(
        const sptr<IWindowTitleButtonRectChangedListener>& listener) override;
    virtual WMError UnregisterWindowTitleButtonRectChangeListener(
        const sptr<IWindowTitleButtonRectChangedListener>& listener) override;
    void NotifyWindowTitleButtonRectChange(TitleButtonRect titleButtonRect);
    void RecoverSessionListener();
    void SetDefaultDisplayIdIfNeed();
    WMError RegisterWindowRectChangeListener(const sptr<IWindowRectChangeListener>& listener) override;
    WMError UnregisterWindowRectChangeListener(const sptr<IWindowRectChangeListener>& listener) override;
    WMError RegisterSubWindowCloseListeners(const sptr<ISubWindowCloseListener>& listener) override;
    WMError UnregisterSubWindowCloseListeners(const sptr<ISubWindowCloseListener>& listener) override;
    virtual WMError GetCallingWindowWindowStatus(WindowStatus& windowStatus) const override;
    virtual WMError GetCallingWindowRect(Rect& rect) const override;
    virtual void SetUiDvsyncSwitch(bool dvsyncSwitch) override;
    WMError SetContinueState(int32_t continueState) override;

protected:
    WMError Connect();
    bool IsWindowSessionInvalid() const;
    void NotifyWindowAfterUnfocused();
    void NotifyWindowAfterFocused();
    void NotifyAfterActive();
    void NotifyAfterInactive();
    void NotifyBeforeDestroy(std::string windowName);
    void NotifyAfterDestroy();
    void ClearListenersById(int32_t persistentId);
    void ClearVsyncStation();
    WMError WindowSessionCreateCheck();
    void UpdateDecorEnableToAce(bool isDecorEnable);
    void UpdateDecorEnable(bool needNotify = false, WindowMode mode = WindowMode::WINDOW_MODE_UNDEFINED);
    void NotifyModeChange(WindowMode mode, bool hasDeco = true);
    WMError UpdateProperty(WSPropertyChangeAction action);
    WMError SetBackgroundColor(uint32_t color);
    uint32_t GetBackgroundColor() const;
    virtual WMError SetLayoutFullScreenByApiVersion(bool status);
    virtual float GetVirtualPixelRatio(sptr<DisplayInfo> displayInfo);
    void UpdateViewportConfig(const Rect& rect, WindowSizeChangeReason reason,
        const std::shared_ptr<RSTransaction>& rsTransaction = nullptr);
    void NotifySizeChange(Rect rect, WindowSizeChangeReason reason);
    void NotifySubWindowClose(bool& terminateCloseProcess);
    static sptr<Window> FindWindowById(uint32_t winId);
    void NotifyWindowStatusChange(WindowMode mode);
    void NotifyTransformChange(const Transform& transForm) override;
    bool IsKeyboardEvent(const std::shared_ptr<MMI::KeyEvent>& keyEvent) const;
    void DispatchKeyEventCallback(const std::shared_ptr<MMI::KeyEvent>& keyEvent, bool& isConsumed);
    bool FilterKeyEvent(const std::shared_ptr<MMI::KeyEvent>& keyEvent);
    void RegisterFrameLayoutCallback();
    bool IsVerticalOrientation(Orientation orientation) const;
    void CopyUniqueDensityParameter(sptr<WindowSessionImpl> parentWindow);

    WMError RegisterExtensionAvoidAreaChangeListener(sptr<IAvoidAreaChangedListener>& listener);
    WMError UnregisterExtensionAvoidAreaChangeListener(sptr<IAvoidAreaChangedListener>& listener);

    void RefreshNoInteractionTimeoutMonitor();
    WindowStatus GetWindowStatusInner(WindowMode mode);

    sptr<ISession> hostSession_;
    mutable std::mutex hostSessionMutex_;
    std::shared_ptr<Ace::UIContent> uiContent_;
    mutable std::shared_mutex uiContentMutex_;
    std::shared_ptr<AbilityRuntime::Context> context_;
    std::shared_ptr<RSSurfaceNode> surfaceNode_;

    sptr<WindowSessionProperty> property_;
    SystemSessionConfig windowSystemConfig_;
    NotifyNativeWinDestroyFunc notifyNativeFunc_;

    std::recursive_mutex mutex_;
    static std::map<std::string, std::pair<int32_t, sptr<WindowSessionImpl>>> windowSessionMap_;
    // protect windowSessionMap_
    static std::shared_mutex windowSessionMutex_;
    static std::map<int32_t, std::vector<sptr<WindowSessionImpl>>> subWindowSessionMap_;
    bool isSystembarPropertiesSet_ = false;
    bool isIgnoreSafeAreaNeedNotify_ = false;
    bool isIgnoreSafeArea_ = false;
    bool isFocused_ { false };
    std::shared_ptr<AppExecFwk::EventHandler> handler_ = nullptr;
    bool shouldReNotifyFocus_ = false;
    std::shared_ptr<VsyncStation> vsyncStation_ = nullptr;
    std::shared_ptr<IInputEventConsumer> inputEventConsumer_;
    bool needRemoveWindowInputChannel_ = false;
    bool useUniqueDensity_ { false };
    float virtualPixelRatio_ { 1.0f };
    bool escKeyEventTriggered_ = false;
    // Check whether the UIExtensionAbility process is started
    static bool isUIExtensionAbilityProcess_;
    virtual WMError SetKeyEventFilter(KeyEventFilterFunc filter) override;
    virtual WMError ClearKeyEventFilter() override;
    WSError SwitchFreeMultiWindow(bool enable) override;
    std::string identityToken_ = { "" };
    void MakeSubOrDialogWindowDragableAndMoveble();
    std::atomic_bool enableSetBufferAvailableCallback_ = false;
    bool IsFreeMultiWindowMode() const
    {
        return windowSystemConfig_.IsFreeMultiWindowMode();
    }

    /*
     * Window Lifecycle
     */
    bool hasFirstNotifyInteractive_ = false;
    bool interactive_ = true;

private:
    //Trans between colorGamut and colorSpace
    static ColorSpace GetColorSpaceFromSurfaceGamut(GraphicColorGamut colorGamut);
    static GraphicColorGamut GetSurfaceGamutFromColorSpace(ColorSpace colorSpace);

    template<typename T> WMError RegisterListener(std::vector<sptr<T>>& holder, const sptr<T>& listener);
    template<typename T> WMError UnregisterListener(std::vector<sptr<T>>& holder, const sptr<T>& listener);
    template<typename T> EnableIfSame<T, IWindowLifeCycle, std::vector<sptr<IWindowLifeCycle>>> GetListeners();
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
    EnableIfSame<T, IScreenshotListener, std::vector<sptr<IScreenshotListener>>> GetListeners();
    template<typename T>
    EnableIfSame<T, ITouchOutsideListener, std::vector<sptr<ITouchOutsideListener>>> GetListeners();
    template<typename T>
    EnableIfSame<T, IWindowVisibilityChangedListener, std::vector<IWindowVisibilityListenerSptr>> GetListeners();
    template<typename T>
    EnableIfSame<T, IWindowNoInteractionListener, std::vector<IWindowNoInteractionListenerSptr>> GetListeners();
    template<typename T>
    EnableIfSame<T, IWindowTitleButtonRectChangedListener,
        std::vector<sptr<IWindowTitleButtonRectChangedListener>>> GetListeners();
    template<typename T> void ClearUselessListeners(std::map<int32_t, T>& listeners, int32_t persistentId);
    RSSurfaceNode::SharedPtr CreateSurfaceNode(std::string name, WindowType type);
    template<typename T>
    EnableIfSame<T, IWindowStatusChangeListener, std::vector<sptr<IWindowStatusChangeListener>>> GetListeners();
    template<typename T>
    EnableIfSame<T, IWindowRectChangeListener, std::vector<sptr<IWindowRectChangeListener>>> GetListeners();
    template<typename T>
    EnableIfSame<T, ISubWindowCloseListener, sptr<ISubWindowCloseListener>> GetListeners();
    void NotifyAfterFocused();
    void NotifyUIContentFocusStatus();
    void NotifyAfterUnfocused(bool needNotifyUiContent = true);
    void NotifyAfterResumed();
    void NotifyAfterPaused();

    WMError InitUIContent(const std::string& contentInfo, napi_env env, napi_value storage,
        WindowSetUIContentType type, AppExecFwk::Ability* ability, OHOS::Ace::UIContentErrorCode& aceRet);
    WMError SetUIContentInner(const std::string& contentInfo, napi_env env, napi_value storage,
        WindowSetUIContentType type, AppExecFwk::Ability* ability);
    std::shared_ptr<std::vector<uint8_t>> GetAbcContent(const std::string& abcPath);

    void UpdateRectForRotation(const Rect& wmRect, const Rect& preRect, WindowSizeChangeReason wmReason,
        const std::shared_ptr<RSTransaction>& rsTransaction = nullptr);
    void UpdateRectForOtherReason(const Rect& wmRect, const Rect& preRect, WindowSizeChangeReason wmReason,
        const std::shared_ptr<RSTransaction>& rsTransaction = nullptr);
    void NotifyRotationAnimationEnd();
    void SubmitNoInteractionMonitorTask(int32_t eventId, const IWindowNoInteractionListenerSptr& listener);
    bool IsUserOrientation(Orientation orientation) const;
    void GetTitleButtonVisible(bool isPC, bool &hideMaximizeButton, bool &hideMinimizeButton, bool &hideSplitButton);
    WMError GetAppForceLandscapeConfig(AppForceLandscapeConfig& config);
    void SetForceSplitEnable(bool isForceSplit, const std::string& homePage = "");
    bool IsNotifyInteractiveDuplicative(bool interactive);
    void SetUniqueVirtualPixelRatioForSub(bool useUniqueDensity, float virtualPixelRatio);

    static std::recursive_mutex lifeCycleListenerMutex_;
    static std::recursive_mutex windowChangeListenerMutex_;
    static std::recursive_mutex avoidAreaChangeListenerMutex_;
    static std::recursive_mutex dialogDeathRecipientListenerMutex_;
    static std::recursive_mutex dialogTargetTouchListenerMutex_;
    static std::recursive_mutex occupiedAreaChangeListenerMutex_;
    static std::recursive_mutex screenshotListenerMutex_;
    static std::recursive_mutex touchOutsideListenerMutex_;
    static std::recursive_mutex windowVisibilityChangeListenerMutex_;
    static std::recursive_mutex windowNoInteractionListenerMutex_;
    static std::recursive_mutex windowStatusChangeListenerMutex_;
    static std::recursive_mutex windowTitleButtonRectChangeListenerMutex_;
    static std::mutex displayMoveListenerMutex_;
    static std::mutex windowRectChangeListenerMutex_;
    static std::mutex subWindowCloseListenersMutex_;
    static std::map<int32_t, std::vector<sptr<IWindowLifeCycle>>> lifecycleListeners_;
    static std::map<int32_t, std::vector<sptr<IDisplayMoveListener>>> displayMoveListeners_;
    static std::map<int32_t, std::vector<sptr<IWindowChangeListener>>> windowChangeListeners_;
    static std::map<int32_t, std::vector<sptr<IAvoidAreaChangedListener>>> avoidAreaChangeListeners_;
    static std::map<int32_t, std::vector<sptr<IDialogDeathRecipientListener>>> dialogDeathRecipientListeners_;
    static std::map<int32_t, std::vector<sptr<IDialogTargetTouchListener>>> dialogTargetTouchListener_;
    static std::map<int32_t, std::vector<sptr<IOccupiedAreaChangeListener>>> occupiedAreaChangeListeners_;
    static std::map<int32_t, std::vector<sptr<IScreenshotListener>>> screenshotListeners_;
    static std::map<int32_t, std::vector<sptr<ITouchOutsideListener>>> touchOutsideListeners_;
    static std::map<int32_t, std::vector<IWindowVisibilityListenerSptr>> windowVisibilityChangeListeners_;
    static std::map<int32_t, std::vector<IWindowNoInteractionListenerSptr>> windowNoInteractionListeners_;
    static std::map<int32_t, std::vector<sptr<IWindowStatusChangeListener>>> windowStatusChangeListeners_;
    static std::map<int32_t, std::vector<sptr<IWindowTitleButtonRectChangedListener>>>
        windowTitleButtonRectChangeListeners_;
    static std::map<int32_t, std::vector<sptr<IWindowRectChangeListener>>> windowRectChangeListeners_;
    static std::map<int32_t, sptr<ISubWindowCloseListener>> subWindowCloseListeners_;

    // FA only
    sptr<IAceAbilityHandler> aceAbilityHandler_;

    std::atomic<int32_t> lastInteractionEventId_ { 0 };

    WindowSizeChangeReason lastSizeChangeReason_ = WindowSizeChangeReason::END;
    bool postTaskDone_ = false;
    int16_t rotationAnimationCount_ { 0 };
    bool isMainHandlerAvailable_ = true;

    std::string subWindowTitle_ = { "" };
    std::string dialogTitle_ = { "" };
    std::shared_mutex keyEventFilterMutex_;
    KeyEventFilterFunc keyEventFilter_;
    WindowTitleVisibleFlags windowTitleVisibleFlags_;
    sptr<WindowOption> windowOption_;
    std::atomic<bool> isUiContentDestructing_ = false;
};
} // namespace Rosen
} // namespace OHOS

#endif // OHOS_ROSEN_WINDOW_SESSION_IMPL_H
