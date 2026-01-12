/*
 * Copyright (c) 2021-2023 Huawei Device Co., Ltd.
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

#ifndef OHOS_ROSEN_WINDOW_IMPL_H
#define OHOS_ROSEN_WINDOW_IMPL_H

#include <map>
#include <string>

#include <ability_context.h>
#include <i_input_event_consumer.h>
#include <key_event.h>
#include <refbase.h>
#include <ui_content.h>
#include <ui/rs_surface_node.h>
#include <ui/rs_ui_director.h>
#include <struct_multimodal.h>

#include "prepare_terminate_callback_stub.h"
#include "input_transfer_station.h"
#include "vsync_station.h"
#include "window.h"
#include "window_helper.h"
#include "window_property.h"
#include "window_transition_info.h"
#include "wm_common_inner.h"
#include "wm_common.h"

using OHOS::AppExecFwk::DisplayOrientation;

namespace OHOS {
namespace Rosen {
namespace {
template<typename T1, typename T2, typename Ret>
using EnableIfSame = typename std::enable_if<std::is_same_v<T1, T2>, Ret>::type;
}
union ColorParam {
#if BIG_ENDIANNESS
    struct {
        uint8_t alpha;
        uint8_t red;
        uint8_t green;
        uint8_t blue;
    } argb;
#else
    struct {
        uint8_t blue;
        uint8_t green;
        uint8_t red;
        uint8_t alpha;
    } argb;
#endif
    uint32_t value;
};

class WindowImpl : public Window, AAFwk::PrepareTerminateCallbackStub {
#define CALL_LIFECYCLE_LISTENER(windowLifecycleCb, listeners) \
    do {                                                      \
        for (auto& listener : (listeners)) {                  \
            if (listener != nullptr) {            \
                listener->windowLifecycleCb();    \
            }                                                 \
        }                                                     \
    } while (0)

#define CALL_LIFECYCLE_LISTENER_WITH_PARAM(windowLifecycleCb, listeners, param) \
    do {                                                                        \
        for (auto& listener : (listeners)) {                                    \
            if (listener != nullptr) {                              \
                listener->windowLifecycleCb(param);                 \
            }                                                                   \
        }                                                                       \
    } while (0)

#define CALL_UI_CONTENT(uiContentCb)                              \
    do {                                                          \
        std::lock_guard<std::recursive_mutex> lock(mutex_);       \
        if (uiContent_ != nullptr) {                              \
            uiContent_->uiContentCb();                            \
        }                                                         \
    } while (0)

public:
    explicit WindowImpl(const sptr<WindowOption>& option);
    ~WindowImpl();

    static sptr<Window> Find(const std::string& id);
    static sptr<Window> GetTopWindowWithContext(const std::shared_ptr<AbilityRuntime::Context>& context = nullptr);
    static sptr<Window> GetTopWindowWithId(uint32_t mainWinId);
    static sptr<Window> GetWindowWithId(uint32_t winId);
    static std::vector<sptr<Window>> GetSubWindow(uint32_t parantId);
    static void UpdateConfigurationForAll(const std::shared_ptr<AppExecFwk::Configuration>& configuration,
        const std::vector<std::shared_ptr<AbilityRuntime::Context>>& ignoreWindowContexts = {});
    static WMError GetWindowTypeForArkUI(WindowType parentWindowType, WindowType& windowType);
    virtual std::shared_ptr<RSSurfaceNode> GetSurfaceNode() const override;
    virtual Rect GetRect() const override;
    virtual Rect GetRequestRect() const override;
    virtual WindowType GetType() const override;
    virtual WindowMode GetWindowMode() const override;
    virtual float GetAlpha() const override;
    virtual WindowState GetWindowState() const override;
    virtual WMError SetFocusable(bool isFocusable) override;
    virtual bool GetFocusable() const override;
    virtual WMError SetTouchable(bool isTouchable) override;
    virtual bool GetTouchable() const override;
    virtual const std::string& GetWindowName() const override;
    virtual uint32_t GetWindowId() const override;
    uint64_t GetDisplayId() const override;
    virtual uint32_t GetWindowFlags() const override;
    uint32_t GetRequestWindowModeSupportType() const override;
    bool IsMainHandlerAvailable() const override;
    inline NotifyNativeWinDestroyFunc GetNativeDestroyCallback()
    {
        return notifyNativefunc_;
    }
    virtual WMError SetWindowType(WindowType type) override;
    virtual WMError SetWindowMode(WindowMode mode) override;
    virtual WMError SetAlpha(float alpha) override;
    virtual WMError SetTransform(const Transform& trans) override;
    virtual WMError AddWindowFlag(WindowFlag flag) override;
    virtual WMError RemoveWindowFlag(WindowFlag flag) override;
    virtual WMError SetWindowFlags(uint32_t flags) override;
    virtual const Transform& GetTransform() const override;
    virtual const Transform& GetZoomTransform() const;
    virtual WMError UpdateSurfaceNodeAfterCustomAnimation(bool isAdd) override;
    inline void SetWindowState(WindowState state)
    {
        state_ = state;
    }

    /*
     * Window Immersive
     */
    WMError GetAvoidAreaByType(AvoidAreaType type, AvoidArea& avoidArea, const Rect& rect = Rect::EMPTY_RECT,
        int32_t apiVersion = API_VERSION_INVALID) override;
    WMError SetAvoidAreaOption(uint32_t avoidAreaOption) override
        { return WMError::WM_ERROR_DEVICE_NOT_SUPPORT; }
    WMError GetAvoidAreaOption(uint32_t& avoidAreaOption) override
        { return WMError::WM_ERROR_DEVICE_NOT_SUPPORT; }
    SystemBarProperty GetSystemBarPropertyByType(WindowType type) const override;
    bool IsFullScreen() const override;
    bool IsLayoutFullScreen() const override;
    virtual WMError UpdateSystemBarProperty(bool status);
    WMError SetLayoutFullScreen(bool status) override;
    WMError SetFullScreen(bool status) override;
    bool IsSystemWindow() const override { return WindowHelper::IsSystemWindow(GetType()); }
    bool IsAppWindow() const override { return WindowHelper::IsAppWindow(GetType()); }
    WMError UpdateSystemBarProperties(const std::unordered_map<WindowType, SystemBarProperty>& systemBarProperties,
        const std::unordered_map<WindowType, SystemBarPropertyFlag>& systemBarPropertyFlags) override;
    WMError SetSystemBarProperty(WindowType type, const SystemBarProperty& property) override;
    void UpdateSpecificSystemBarEnabled(bool systemBarEnable, bool systemBarEnableAnimation,
        SystemBarProperty& property) override;
    WMError UpdateSystemBarPropertyForPage(WindowType type,
        const SystemBarProperty& systemBarProperty, const SystemBarPropertyFlag& systemBarPropertyFlag) override;

    WMError SetIntentParam(const std::string& intentParam, const std::function<void()>& loadPageCallback,
        bool isColdStart) override;
    WMError Create(uint32_t parentId,
        const std::shared_ptr<AbilityRuntime::Context>& context = nullptr);
    virtual WMError Destroy(uint32_t reason = 0) override;
    void SetShowWithOptions(bool showWithOptions) override;
    bool IsShowWithOptions() const override;
    virtual WMError Show(uint32_t reason = 0, bool withAnimation = false, bool withFocus = true) override;
    virtual WMError Show(uint32_t reason, bool withAnimation, bool withFocus, bool waitAttach) override;
    virtual WMError Hide(uint32_t reason = 0, bool withAnimation = false, bool isFromInnerkits = true) override;
    virtual WMError Hide(uint32_t reason, bool withAnimation, bool isFromInnerkits, bool waitDetach) override;
    virtual WMError MoveTo(int32_t x, int32_t y, bool isMoveToGlobal = false,
        MoveConfiguration moveConfiguration = {}) override;
    virtual WMError Resize(uint32_t width, uint32_t height,
        const RectAnimationConfig& rectAnimationConfig = {}) override;
    float GetVirtualPixelRatio() override;
    virtual WMError SetWindowGravity(WindowGravity gravity, uint32_t percent) override;
    virtual WMError SetKeepScreenOn(bool keepScreenOn) override;
    virtual bool IsKeepScreenOn() const override;
    virtual WMError SetTurnScreenOn(bool turnScreenOn) override;
    virtual bool IsTurnScreenOn() const override;
    virtual WMError SetBackgroundColor(const std::string& color) override;
    virtual WMError SetTransparent(bool isTransparent) override;
    virtual bool IsTransparent() const override;
    virtual WMError SetBrightness(float brightness) override;
    virtual float GetBrightness() const override;
    virtual WMError ChangeCallingWindowId(uint32_t windowId) override;
    virtual WMError SetPrivacyMode(bool isPrivacyMode) override;
    virtual bool IsPrivacyMode() const override;
    virtual void SetSystemPrivacyMode(bool isSystemPrivacyMode) override;
    virtual WMError DisableAppWindowDecor() override;
    virtual WMError BindDialogTarget(sptr<IRemoteObject> targetToken) override;
    WMError RaiseToAppTop() override;
    virtual WMError SetAspectRatio(float ratio) override;
    virtual WMError ResetAspectRatio() override;
    virtual WMError SetSnapshotSkip(bool isSkip) override;
    // window effect
    virtual WMError SetCornerRadius(float cornerRadius) override;
    virtual WMError SetShadowRadius(float radius) override;
    virtual WMError SetShadowColor(std::string color) override;
    virtual WMError SetShadowOffsetX(float offsetX) override;
    virtual WMError SetShadowOffsetY(float offsetY) override;
    virtual WMError SetBlur(float radius) override;
    virtual WMError SetBackdropBlur(float radius) override;
    virtual WMError SetBackdropBlurStyle(WindowBlurStyle blurStyle) override;

    virtual bool IsDecorEnable() const override;
    virtual WMError Maximize() override;
    virtual WMError MaximizeFloating() override;
    virtual WMError Minimize() override;
    virtual WMError Recover() override;
    virtual WMError Close() override;
    virtual void StartMove() override;
    virtual WMError SetGlobalMaximizeMode(MaximizeMode mode) override;
    virtual MaximizeMode GetGlobalMaximizeMode() const override;
    virtual WMError SetImmersiveModeEnabledState(bool enable) override;
    virtual bool GetImmersiveModeEnabledState() const override;

    virtual WMError RequestFocus() const override;
    virtual void SetInputEventConsumer(const std::shared_ptr<IInputEventConsumer>& inputEventConsumer) override;

    virtual WMError RegisterLifeCycleListener(const sptr<IWindowLifeCycle>& listener) override;
    virtual WMError RegisterWindowChangeListener(const sptr<IWindowChangeListener>& listener) override;
    virtual WMError UnregisterLifeCycleListener(const sptr<IWindowLifeCycle>& listener) override;
    virtual WMError UnregisterWindowChangeListener(const sptr<IWindowChangeListener>& listener) override;
    virtual WMError RegisterAvoidAreaChangeListener(const sptr<IAvoidAreaChangedListener>& listener) override;
    virtual WMError UnregisterAvoidAreaChangeListener(const sptr<IAvoidAreaChangedListener>& listener) override;
    virtual WMError RegisterDragListener(const sptr<IWindowDragListener>& listener) override;
    virtual WMError UnregisterDragListener(const sptr<IWindowDragListener>& listener) override;
    virtual WMError RegisterDisplayMoveListener(sptr<IDisplayMoveListener>& listener) override;
    virtual WMError UnregisterDisplayMoveListener(sptr<IDisplayMoveListener>& listener) override;
    virtual void RegisterWindowDestroyedListener(const NotifyNativeWinDestroyFunc& func) override;
    virtual void UnregisterWindowDestroyedListener() override { notifyNativefunc_ = nullptr; }
    virtual WMError RegisterOccupiedAreaChangeListener(const sptr<IOccupiedAreaChangeListener>& listener) override;
    virtual WMError UnregisterOccupiedAreaChangeListener(const sptr<IOccupiedAreaChangeListener>& listener) override;
    virtual WMError RegisterTouchOutsideListener(const sptr<ITouchOutsideListener>& listener) override;
    virtual WMError UnregisterTouchOutsideListener(const sptr<ITouchOutsideListener>& listener) override;
    virtual WMError RegisterAnimationTransitionController(
        const sptr<IAnimationTransitionController>& listener) override;
    virtual WMError RegisterScreenshotListener(const sptr<IScreenshotListener>& listener) override;
    virtual WMError UnregisterScreenshotListener(const sptr<IScreenshotListener>& listener) override;
    virtual WMError RegisterScreenshotAppEventListener(const IScreenshotAppEventListenerSptr& listener) override;
    virtual WMError UnregisterScreenshotAppEventListener(const IScreenshotAppEventListenerSptr& listener) override;
    virtual WMError RegisterDialogTargetTouchListener(const sptr<IDialogTargetTouchListener>& listener) override;
    virtual WMError UnregisterDialogTargetTouchListener(const sptr<IDialogTargetTouchListener>& listener) override;
    virtual void RegisterDialogDeathRecipientListener(const sptr<IDialogDeathRecipientListener>& listener) override;
    virtual void UnregisterDialogDeathRecipientListener(const sptr<IDialogDeathRecipientListener>& listener) override;
    virtual void SetAceAbilityHandler(const sptr<IAceAbilityHandler>& handler) override;
    virtual void SetRequestWindowModeSupportType(uint32_t windowModeSupportType) override;
    void UpdateRect(const struct Rect& rect, bool decoStatus, WindowSizeChangeReason reason,
        const std::shared_ptr<RSTransaction>& rsTransaction = nullptr);
    void ScheduleUpdateRectTask(const Rect& rectToAce, const Rect& lastOriRect, WindowSizeChangeReason reason,
        const std::shared_ptr<RSTransaction>& rsTransaction, const sptr<class Display>& display);
    void UpdateMode(WindowMode mode);
    void UpdateWindowModeSupportType(uint32_t windowModeSupportType);
    virtual void ConsumeKeyEvent(std::shared_ptr<MMI::KeyEvent>& inputEvent) override;
    virtual void ConsumePointerEvent(const std::shared_ptr<MMI::PointerEvent>& inputEvent) override;
    virtual void RequestVsync(const std::shared_ptr<VsyncCallback>& vsyncCallback) override;
    virtual int64_t GetVSyncPeriod() override;
    void UpdateFocusStatus(bool focused);
    virtual bool IsFocused() const override;
    virtual void UpdateConfiguration(const std::shared_ptr<AppExecFwk::Configuration>& configuration) override;
    void UpdateConfigurationForSpecified(const std::shared_ptr<AppExecFwk::Configuration>& configuration,
        const std::shared_ptr<Global::Resource::ResourceManager>& resourceManager) override;
    void UpdateAvoidArea(const sptr<AvoidArea>& avoidArea, AvoidAreaType type);
    void UpdateWindowState(WindowState state);
    WmErrorCode UpdateSubWindowStateAndNotify(uint32_t parentId);
    WmErrorCode UpdateWindowStateWhenShow();
    WmErrorCode UpdateWindowStateWhenHide();
    sptr<WindowProperty> GetWindowProperty();
    void UpdateDragEvent(const PointInfo& point, DragEvent event);
    void UpdateDisplayId(DisplayId from, DisplayId to);
    void UpdateOccupiedAreaChangeInfo(const sptr<OccupiedAreaChangeInfo>& info,
        const std::map<AvoidAreaType, AvoidArea>& avoidAreas,
        const std::shared_ptr<RSTransaction>& rsTransaction = nullptr);
    void UpdateActiveStatus(bool isActive);
    void NotifyTouchOutside();
    void NotifyScreenshot();
    WMError NotifyScreenshotAppEvent(ScreenshotEventType type);
    void NotifyTouchDialogTarget(int32_t posX = 0, int32_t posY = 0) override;
    void NotifyDestroy();
    void NotifyForeground();
    void NotifyMainWindowDidForeground(uint32_t reason);
    void NotifyBackground();
    void UpdateZoomTransform(const Transform& trans, bool isDisplayZoomOn);
    void PerformBack() override;
    void NotifyForegroundInteractiveStatus(bool interactive);
    void NotifyMMIServiceOnline(uint32_t winId);
    virtual bool PreNotifyKeyEvent(const std::shared_ptr<MMI::KeyEvent>& keyEvent) override;
    virtual WMError NapiSetUIContent(const std::string& contentInfo, napi_env env, napi_value storage,
        BackupAndRestoreType type, sptr<IRemoteObject> token, AppExecFwk::Ability* ability) override;
    virtual WMError AniSetUIContent(const std::string& contentInfo, ani_env* env, ani_object storage,
        BackupAndRestoreType type, sptr<IRemoteObject> token, AppExecFwk::Ability* ability) override;
    virtual WMError NapiSetUIContentByName(const std::string& contentName, napi_env env, napi_value storage,
        BackupAndRestoreType type, sptr<IRemoteObject> token, AppExecFwk::Ability* ability) override;
    virtual WMError AniSetUIContentByName(const std::string& contentName, ani_env* env, ani_object storage,
        BackupAndRestoreType type, sptr<IRemoteObject> token, AppExecFwk::Ability* ability) override;
    virtual WMError SetUIContentByName(const std::string& contentInfo, napi_env env, napi_value storage,
        AppExecFwk::Ability* ability) override;
    virtual WMError SetUIContentByAbc(const std::string& abcPath, napi_env env, napi_value storage,
        AppExecFwk::Ability* ability) override;
    virtual std::string GetContentInfo(BackupAndRestoreType type = BackupAndRestoreType::CONTINUATION) override;
    WMError SetRestoredRouterStack(const std::string& routerStack) override;
    virtual const std::shared_ptr<AbilityRuntime::Context> GetContext() const override;
    virtual Ace::UIContent* GetUIContent() const override;
    virtual Ace::UIContent* GetUIContentWithId(uint32_t winId) const override;
    virtual void OnNewWant(const AAFwk::Want& want) override;
    virtual void SetRequestedOrientation(Orientation orientation, bool needAnimation = true) override;
    virtual Orientation GetRequestedOrientation() override;
    virtual void SetNeedRemoveWindowInputChannel(bool needRemoveWindowInputChannel) override;
    virtual WMError SetTouchHotAreas(const std::vector<Rect>& rects) override;
    virtual void GetRequestedTouchHotAreas(std::vector<Rect>& rects) const override;
    virtual WMError SetAPPWindowLabel(const std::string& label) override;
    virtual WMError SetAPPWindowIcon(const std::shared_ptr<Media::PixelMap>& icon) override;

    // colorspace, gamut
    virtual bool IsSupportWideGamut() override;
    virtual void SetColorSpace(ColorSpace colorSpace) override;
    virtual ColorSpace GetColorSpace() override;

    virtual void DumpInfo(const std::vector<std::string>& params, std::vector<std::string>& info) override;
    virtual std::shared_ptr<Media::PixelMap> Snapshot() override;
    WMError SnapshotIgnorePrivacy(std::shared_ptr<Media::PixelMap>& pixelMap) override;
    virtual WMError NotifyMemoryLevel(int32_t level) override;
    virtual bool IsAllowHaveSystemSubWindow() override;
    virtual KeyboardAnimationConfig GetKeyboardAnimationConfig() override;
    void RestoreSplitWindowMode(uint32_t mode);

    virtual void SetNeedDefaultAnimation(bool needDefaultAnimation) override;

    virtual void DoPrepareTerminate() override;
    void PendingClose();

    WMError SetTextFieldAvoidInfo(double textFieldPositionY, double textFieldHeight) override;
    virtual WMError SetSystemBarProperties(const std::map<WindowType, SystemBarProperty>& properties,
        const std::map<WindowType, SystemBarPropertyFlag>& propertyFlags) override;
    virtual WMError GetSystemBarProperties(std::map<WindowType, SystemBarProperty>& properties) override;
    virtual WMError SetSpecificBarProperty(WindowType type, const SystemBarProperty& property) override;

    /*
     * Gesture Back
     */
    WMError SetGestureBackEnabled(bool enable) override { return WMError::WM_ERROR_DEVICE_NOT_SUPPORT; }
    WMError GetGestureBackEnabled(bool& enable) const override { return WMError::WM_ERROR_DEVICE_NOT_SUPPORT; }

    /*
     * Window Property
     */
    static void UpdateConfigurationSyncForAll(const std::shared_ptr<AppExecFwk::Configuration>& configuration);
    void UpdateConfigurationSync(const std::shared_ptr<AppExecFwk::Configuration>& configuration) override;
    void RegisterWindowInspectorCallback();
    uint32_t GetApiTargetVersion() const;
    WMError GetWindowPropertyInfo(WindowPropertyInfo& windowPropertyInfo) override;

    /*
     * Keyboard
     */
    WMError ShowKeyboard(uint32_t callingWindowId, uint64_t tgtDisplayId,
        KeyboardEffectOption effectOption) override;
    sptr<OccupiedAreaChangeInfo> occupiedAreaInfo_ = nullptr;

    /*
     * RS Client Multi Instance
     */
    std::shared_ptr<RSUIDirector> GetRSUIDirector() const override;
    std::shared_ptr<RSUIContext> GetRSUIContext() const override;

private:
    template<typename T> WMError RegisterListener(std::vector<sptr<T>>& holder, const sptr<T>& listener);
    template<typename T> WMError UnregisterListener(std::vector<sptr<T>>& holder, const sptr<T>& listener);
    template<typename T> void ClearUselessListeners(std::map<uint32_t, T>& listeners, uint32_t winId)
    {
        listeners.erase(winId);
    }
    template<typename T> void ClearUselessListeners(std::unordered_map<uint32_t, T>& listeners, uint32_t winId)
    {
        listeners.erase(winId);
    }
    template<typename T> EnableIfSame<T, IWindowLifeCycle, std::vector<sptr<IWindowLifeCycle>>> GetListeners();
    template<typename T>
    EnableIfSame<T, IWindowChangeListener, std::vector<sptr<IWindowChangeListener>>> GetListeners();
    template<typename T>
    EnableIfSame<T, IAvoidAreaChangedListener, std::vector<sptr<IAvoidAreaChangedListener>>> GetListeners();
    template<typename T> EnableIfSame<T, IDisplayMoveListener, std::vector<sptr<IDisplayMoveListener>>> GetListeners();
    template<typename T> EnableIfSame<T, IScreenshotListener, std::vector<sptr<IScreenshotListener>>> GetListeners();
    template<typename T>
    EnableIfSame<T, IScreenshotAppEventListener, std::vector<IScreenshotAppEventListenerSptr>> GetListeners();
    template<typename T>
    EnableIfSame<T, ITouchOutsideListener, std::vector<sptr<ITouchOutsideListener>>> GetListeners();
    template<typename T>
    EnableIfSame<T, IDialogTargetTouchListener, std::vector<sptr<IDialogTargetTouchListener>>> GetListeners();
    template<typename T> EnableIfSame<T, IWindowDragListener, std::vector<sptr<IWindowDragListener>>> GetListeners();
    template<typename T>
    EnableIfSame<T, IOccupiedAreaChangeListener, std::vector<sptr<IOccupiedAreaChangeListener>>> GetListeners();
    template<typename T>
    EnableIfSame<T, IDialogDeathRecipientListener, wptr<IDialogDeathRecipientListener>> GetListener();

    void NotifyAfterForeground(bool needNotifyListeners = true, bool needNotifyUiContent = true);
    void NotifyAfterBackground(bool needNotifyListeners = true, bool needNotifyUiContent = true);
    void NotifyAfterDidForeground(uint32_t reason);
    void NotifyAfterDidBackground(uint32_t reason);
    void NotifyAfterFocused();
    void NotifyAfterUnfocused(bool needNotifyUiContent = true);
    void NotifyAfterResumed();
    void NotifyAfterPaused();
    void NotifyBeforeDestroy(std::string windowName);
    void NotifyBeforeSubWindowDestroy(sptr<WindowImpl> window);
    void NotifyAfterActive();
    void NotifyAfterInactive();
    void NotifyForegroundFailed(WMError ret);
    void NotifyBackgroundFailed(WMError ret);
    bool IsStretchableReason(WindowSizeChangeReason reason);

    void InitWindowProperty(const sptr<WindowOption>& option);
    void ClearListenersById(uint32_t winId);
    void NotifySizeChange(Rect rect, WindowSizeChangeReason reason,
        const std::shared_ptr<RSTransaction>& rsTransaction = nullptr);
    void NotifyAvoidAreaChange(const sptr<AvoidArea>& avoidArea, AvoidAreaType type);
    void NotifyDisplayMoveChange(DisplayId from, DisplayId to);
    void NotifyOccupiedAreaChange(const sptr<OccupiedAreaChangeInfo>& info,
        const std::shared_ptr<RSTransaction>& rsTransaction = nullptr);
    void NotifyModeChange(WindowMode mode, bool hasDeco = true);
    void NotifyDragEvent(const PointInfo& point, DragEvent event);
    void ResetInputWindow(uint32_t winId);
    void DestroyDialogWindow();
    void DestroyFloatingWindow();
    void DestroySubWindow();
    void ClearVsyncStation();
    void SetDefaultOption(); // for api7
    bool IsWindowValid() const;
    static sptr<Window> FindWindowById(uint32_t WinId);
    void TransferPointerEvent(const std::shared_ptr<MMI::PointerEvent>& pointerEvent);
    void ConsumeMoveOrDragEvent(const std::shared_ptr<MMI::PointerEvent>& pointerEvent);
    void ReadyToMoveOrDragWindow(const std::shared_ptr<MMI::PointerEvent>& pointerEvent,
        const MMI::PointerEvent::PointerItem& pointerItem);
    void EndMoveOrDragWindow(int32_t posX, int32_t posY, int32_t pointId, int32_t sourceType);
    void ResetMoveOrDragState();
    bool IsPointerEventConsumed();
    bool IsPointInDragHotZone(int32_t startPointPosX, int32_t startPointPosY, int32_t sourceType);
    void AdjustWindowAnimationFlag(bool withAnimation = false);
    void MapFloatingWindowToAppIfNeeded();
    void MapDialogWindowToAppIfNeeded();
    WMError UpdateProperty(PropertyChangeAction action);
    WMError Destroy(bool needNotifyServer, bool needClearListener = true, uint32_t reason = 0);
    WMError SetBackgroundColor(uint32_t color);
    uint32_t GetBackgroundColor() const;
    void InitAbilityInfo();
    std::shared_ptr<AppExecFwk::AbilityInfo> GetOriginalAbilityInfo() const;
    void RecordLifeCycleExceptionEvent(LifeCycleEvent event, WMError errCode) const;
    std::string TransferLifeCycleEventToString(LifeCycleEvent type) const;
    Rect GetSystemAlarmWindowDefaultSize(Rect defaultRect);
    void HandleModeChangeHotZones(int32_t posX, int32_t posY);
    WMError NotifyWindowTransition(TransitionReason reason);
    void UpdatePointerEventForStretchableWindow(const std::shared_ptr<MMI::PointerEvent>& pointerEvent);
    void UpdateDragType(int32_t startPointPosX, int32_t startPointPosY);
    void HandleBackKeyPressedEvent(const std::shared_ptr<MMI::KeyEvent>& keyEvent);
    bool CheckCameraFloatingWindowMultiCreated(WindowType type);
    void GetConfigurationFromAbilityInfo();
    void UpdateTitleButtonVisibility();
    void SetWindowModeSupportType(uint32_t windowModeSupportType);
    uint32_t GetWindowModeSupportType() const;
    WMError PreProcessShow(uint32_t reason, bool withAnimation);
    bool NeedToStopShowing();
    void CalculateStartRectExceptHotZone(float virtualPixelRatio);
    void SetSystemConfig();
    void TransformSurfaceNode(const Transform& trans);
    bool IsAppMainOrSunOrFloatingWindow();
    bool IsAppMainOrSubOrFloatingWindow();
    WMError WindowCreateCheck(uint32_t parentId);
    uint32_t CalculatePointerDirection(int32_t pointerX, int32_t pointerY);
    void HandlePointerStyle(const std::shared_ptr<MMI::PointerEvent>& pointerEvent);
    RSSurfaceNode::SharedPtr CreateSurfaceNode(std::string name, WindowType type);
    void UpdateWindowStateUnfrozen();
    void UpdateViewportConfig(const Rect& rect, const sptr<class Display>& display, WindowSizeChangeReason reason,
        const std::shared_ptr<RSTransaction>& rsTransaction = nullptr,
        const std::map<AvoidAreaType, AvoidArea>& avoidAreas = {});
    void UpdateDecorEnable(bool needNotify = false);
    WMError SetFloatingMaximize(bool isEnter);
    void CustomHideAnimation();

    std::unique_ptr<Ace::UIContent> UIContentCreate(AppExecFwk::Ability* ability, void* env, int isAni);
    Ace::UIContentErrorCode UIContentInitByName(Ace::UIContent*, const std::string&, void* storage, int isAni);
    template<typename T>
    Ace::UIContentErrorCode UIContentInit(Ace::UIContent*, T contentInfo, void* storage, int isAni);
    template<typename T>
    Ace::UIContentErrorCode UIContentInit(Ace::UIContent*, T contentInfo, void* storage, const std::string& contentName,
        int isAni);
    Ace::UIContentErrorCode UIContentRestore(Ace::UIContent*, const std::string& contentInfo, void* storage,
        Ace::ContentInfoType infoType, int isAni);
    WMError SetUIContentInner(const std::string& contentInfo, void* env, void* storage,
        WindowSetUIContentType setUIContentType, BackupAndRestoreType restoreType, AppExecFwk::Ability* ability,
        int isAni = 0);
    std::shared_ptr<std::vector<uint8_t>> GetAbcContent(const std::string& abcPath);
    std::string GetRestoredRouterStack();

    // colorspace, gamut
    using ColorSpaceConvertMap = struct {
        ColorSpace colorSpace;
        GraphicColorGamut surfaceColorGamut;
    };
    static const ColorSpaceConvertMap colorSpaceConvertMap[];
    static ColorSpace GetColorSpaceFromSurfaceGamut(GraphicColorGamut colorGamut);
    static GraphicColorGamut GetSurfaceGamutFromColorSpace(ColorSpace colorSpace);
    void ChangePropertyByApiVersion();
    void PerfLauncherHotAreaIfNeed(const std::shared_ptr<MMI::PointerEvent>& pointerEvent);
    void SetDefaultDisplayIdIfNeed();

    static std::map<std::string, std::pair<uint32_t, sptr<Window>>> windowMap_;
    static std::map<uint32_t, std::vector<sptr<WindowImpl>>> subWindowMap_;
    static std::map<uint32_t, std::vector<sptr<WindowImpl>>> appFloatingWindowMap_;
    static std::map<uint32_t, std::vector<sptr<WindowImpl>>> appDialogWindowMap_;
    sptr<WindowProperty> property_;
    WindowState state_ { WindowState::STATE_INITIAL };
    WindowState subWindowState_ {WindowState::STATE_INITIAL};
    WindowTag windowTag_;
    bool showWithOptions_ = false;
    sptr<IAceAbilityHandler> aceAbilityHandler_;
    static std::map<uint32_t, std::vector<sptr<IScreenshotListener>>> screenshotListeners_;
    static std::map<uint32_t, std::vector<IScreenshotAppEventListenerSptr>> screenshotAppEventListeners_;
    static std::map<uint32_t, std::vector<sptr<ITouchOutsideListener>>> touchOutsideListeners_;
    static std::map<uint32_t, std::vector<sptr<IDialogTargetTouchListener>>> dialogTargetTouchListeners_;
    static std::map<uint32_t, std::vector<sptr<IWindowLifeCycle>>> lifecycleListeners_;
    static std::map<uint32_t, std::vector<sptr<IWindowChangeListener>>> windowChangeListeners_;
    static std::map<uint32_t, std::vector<sptr<IAvoidAreaChangedListener>>> avoidAreaChangeListeners_;
    std::vector<sptr<IWindowDragListener>> windowDragListeners_;
    std::vector<sptr<IDisplayMoveListener>> displayMoveListeners_;
    static std::map<uint32_t, std::vector<sptr<IOccupiedAreaChangeListener>>> occupiedAreaChangeListeners_;
    static std::map<uint32_t, sptr<IDialogDeathRecipientListener>> dialogDeathRecipientListener_;
    std::shared_ptr<IInputEventConsumer> inputEventConsumer_;
    NotifyNativeWinDestroyFunc notifyNativefunc_;
    std::shared_ptr<RSSurfaceNode> surfaceNode_;
    std::string name_;
    std::unique_ptr<Ace::UIContent> uiContent_;
    std::shared_ptr<AbilityRuntime::Context> context_;
    std::recursive_mutex mutex_;
    std::recursive_mutex windowStateMutex_;
    static std::recursive_mutex globalMutex_;
    static std::shared_mutex windowMapMutex_;
    const float SYSTEM_ALARM_WINDOW_WIDTH_RATIO = 0.8;
    const float SYSTEM_ALARM_WINDOW_HEIGHT_RATIO = 0.3;
    WindowSizeChangeReason lastSizeChangeReason_ = WindowSizeChangeReason::END;

    sptr<MoveDragProperty> moveDragProperty_;
    SystemConfig windowSystemConfig_;
    bool isOriginRectSet_ = false;
    bool needRemoveWindowInputChannel_ = false;
    bool isMainHandlerAvailable_ = true;
    bool isAppFloatingWindow_ = false;
    bool isFocused_ = false;
    uint32_t mouseStyleID_ = 0;
    bool isIgnoreSafeAreaNeedNotify_ = false;
    bool isIgnoreSafeArea_ = false;
    bool needDefaultAnimation_ = true;
    bool postTaskDone_ = false;
    static float ConvertRadiusToSigma(float radius)
    {
        constexpr float BlurSigmaScale = 0.57735f;
        return radius > 0.0f ? BlurSigmaScale * radius + SK_ScalarHalf : 0.0f;
    }
    std::shared_ptr<AppExecFwk::EventHandler> handler_ = nullptr;
    bool needNotifyFocusLater_ = false;
    bool escKeyEventTriggered_ = false;
    bool enableImmersiveMode_ = false;
    std::shared_ptr<VsyncStation> vsyncStation_ = nullptr;
    std::atomic<float> virtualPixelRatio_ = 1.0f;

    std::string restoredRouterStack_; // It was set and get in same thread, which is js thread.

    /*
     * Window animation
     */
    std::mutex transitionControllerMutex_;
    std::vector<sptr<IAnimationTransitionController>> animationTransitionControllers_;

    /*
     * RS Client Multi Instance
     */
    std::shared_ptr<RSUIDirector> rsUIDirector_;

    /*
     * Window Lifecycle
     */
    std::string intentParam_;
    std::function<void()> loadPageCallback_;
    bool isIntentColdStart_ = true;
};
} // namespace Rosen
} // namespace OHOS
#endif // OHOS_ROSEN_WINDOW_IMPL_H
