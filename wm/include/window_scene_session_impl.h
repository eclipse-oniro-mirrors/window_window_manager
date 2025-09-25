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

#ifndef OHOS_ROSEN_WINDOW_SCENE_SESSION_IMPL_H
#define OHOS_ROSEN_WINDOW_SCENE_SESSION_IMPL_H

#include "window_session_impl.h"
#include "window_manager.h"

namespace OHOS {
namespace Rosen {
using NotifyWindowRecoverStateChangeFunc = std::function<void(bool isSpecificSession,
    const WindowRecoverState& state)>;

class WindowSceneSessionImpl : public WindowSessionImpl {
public:
    explicit WindowSceneSessionImpl(const sptr<WindowOption>& option);
    ~WindowSceneSessionImpl();
    WMError Create(const std::shared_ptr<AbilityRuntime::Context>& context,
        const sptr<Rosen::ISession>& iSession, const std::string& identityToken = "",
        bool isModuleAbilityHookEnd = false) override;
    WMError Show(uint32_t reason = 0, bool withAnimation = false, bool withFocus = true) override;
    WMError Show(uint32_t reason, bool withAnimation, bool withFocus, bool waitAttach) override;
    WMError ShowKeyboard(KeyboardEffectOption effectOption) override;
    WMError Hide(uint32_t reason, bool withAnimation, bool isFromInnerkits) override;
    WMError Hide(uint32_t reason, bool withAnimation, bool isFromInnerkits, bool waitDetach) override;
    WMError Destroy(bool needNotifyServer, bool needClearListener = true, uint32_t reason = 0) override;
    WMError DestroyHookWindow();
    WMError NotifyDrawingCompleted() override;
    WMError SetTextFieldAvoidInfo(double textFieldPositionY, double textFieldHeight) override;
    void PreProcessCreate();
    void SetDefaultProperty();
    WMError Minimize() override;
    void StartMove() override;
    bool IsStartMoving() override;
    WindowMode GetWindowMode() const override;
    WMError SetHookTargetElementInfo(const AppExecFwk::ElementName& elementName) override;

    /*
     * Window Layout
     */
    WMError MoveTo(int32_t x, int32_t y, bool isMoveToGlobal = false,
        MoveConfiguration moveConfiguration = {}) override;
    WMError MoveToAsync(int32_t x, int32_t y, MoveConfiguration moveConfiguration = {}) override;
    WMError MoveWindowToGlobal(int32_t x, int32_t y, MoveConfiguration moveConfiguration = {}) override;
    WMError MoveWindowToGlobalDisplay(int32_t x, int32_t y, MoveConfiguration moveConfiguration = {}) override;
    WMError GetGlobalScaledRect(Rect& globalScaledRect) override;
    WMError Resize(uint32_t width, uint32_t height,
        const RectAnimationConfig& rectAnimationConfig = {}) override;
    WMError ResizeAsync(uint32_t width, uint32_t height,
        const RectAnimationConfig& rectAnimationConfig = {}) override;
    WMError SetWindowAnchorInfo(const WindowAnchorInfo& windowAnchorInfo) override;
    WMError SetFollowParentWindowLayoutEnabled(bool isFollow) override;
    WSError NotifyLayoutFinishAfterWindowModeChange(WindowMode mode) override;
    WMError SetFrameRectForPartialZoomIn(const Rect& frameRect) override;
    WMError UpdateWindowModeForUITest(int32_t updateMode) override;
    WSError NotifyAppHookWindowInfoUpdated() override;

    /*
     * Window Hierarchy
     */
    WMError RaiseToAppTop() override;
    WMError RaiseAboveTarget(int32_t subWindowId) override;
    WMError SetSubWindowZLevel(int32_t zLevel) override;
    WMError GetSubWindowZLevel(int32_t& zLevel) override;
    WMError RaiseMainWindowAboveTarget(int32_t targetId) override;

    void PerformBack() override;
    WMError SetAspectRatio(float ratio) override;
    WMError ResetAspectRatio() override;
    WMError SetGlobalMaximizeMode(MaximizeMode mode) override;
    MaximizeMode GetGlobalMaximizeMode() const override;

    WMError BindDialogTarget(sptr<IRemoteObject> targetToken) override;
    WMError SetDialogBackGestureEnabled(bool isEnabled) override;
    WMError GetWindowLimits(WindowLimits& windowLimits) override;
    WMError SetWindowLimits(WindowLimits& windowLimits, bool isForce) override;
    static void UpdateConfigurationForAll(const std::shared_ptr<AppExecFwk::Configuration>& configuration,
        const std::vector<std::shared_ptr<AbilityRuntime::Context>>& ignoreWindowContexts = {});
    static sptr<Window> GetTopWindowWithContext(const std::shared_ptr<AbilityRuntime::Context>& context = nullptr);
    static sptr<Window> GetTopWindowWithId(uint32_t mainWinId);
    static sptr<Window> GetMainWindowWithContext(const std::shared_ptr<AbilityRuntime::Context>& context = nullptr);
    static sptr<WindowSessionImpl> GetMainWindowWithId(uint32_t mainWinId);
    // only main window, sub window, dialog window can use
    static int32_t GetParentMainWindowId(int32_t windowId);
    static WMError GetAndVerifyWindowTypeForArkUI(uint32_t parentId, const std::string& windowName,
        WindowType parentWindowType, WindowType& windowType);
    virtual void UpdateConfiguration(const std::shared_ptr<AppExecFwk::Configuration>& configuration) override;
    void UpdateConfigurationForSpecified(const std::shared_ptr<AppExecFwk::Configuration>& configuration,
        const std::shared_ptr<Global::Resource::ResourceManager>& resourceManager) override;
    WMError NotifyMemoryLevel(int32_t level) override;

    virtual WMError AddWindowFlag(WindowFlag flag) override;
    virtual WMError RemoveWindowFlag(WindowFlag flag) override;
    virtual WMError SetWindowFlags(uint32_t flags) override;
    virtual uint32_t GetWindowFlags() const override;

    virtual WMError SetTransparent(bool isTransparent) override;
    virtual WMError SetTurnScreenOn(bool turnScreenOn) override;
    virtual WMError SetKeepScreenOn(bool keepScreenOn) override;
    virtual WMError SetViewKeepScreenOn(bool keepScreenOn) override;
    virtual WMError SetWindowShadowEnabled(bool isEnabled) override;
    virtual WMError SetPrivacyMode(bool isPrivacyMode) override;
    virtual void SetSystemPrivacyMode(bool isSystemPrivacyMode) override;
    virtual WMError SetSnapshotSkip(bool isSkip) override;
    virtual std::shared_ptr<Media::PixelMap> Snapshot() override;
    WMError Snapshot(std::shared_ptr<Media::PixelMap>& pixelMap) override;
    WMError SnapshotIgnorePrivacy(std::shared_ptr<Media::PixelMap>& pixelMap) override;
    WMError SetTouchHotAreas(const std::vector<Rect>& rects) override;
    WMError SetKeyboardTouchHotAreas(const KeyboardTouchHotAreas& hotAreas) override;
    virtual WmErrorCode KeepKeyboardOnFocus(bool keepKeyboardFlag) override;
    virtual WMError SetCallingWindow(uint32_t callingSessionId) override;
    WMError ChangeKeyboardEffectOption(KeyboardEffectOption effectOption) override;

    virtual bool IsTransparent() const override;
    virtual bool IsTurnScreenOn() const override;
    virtual bool IsKeepScreenOn() const override;
    virtual bool IsViewKeepScreenOn() const override;
    virtual bool GetWindowShadowEnabled() const override;
    virtual bool IsPrivacyMode() const override;
    virtual bool IsLayoutFullScreen() const override;
    virtual bool IsFullScreen() const override;
    WMError RegisterAnimationTransitionController(const sptr<IAnimationTransitionController>& listener) override;
    void SetNeedDefaultAnimation(bool needDefaultAnimation) override;
    WMError SetTransform(const Transform& trans) override;
    const Transform& GetTransform() const override;
    WMError UpdateSurfaceNodeAfterCustomAnimation(bool isAdd) override;
    WMError SetAlpha(float alpha) override;
    void DumpSessionElementInfo(const std::vector<std::string>& params) override;
    WSError UpdateWindowMode(WindowMode mode) override;
    WSError GetTopNavDestinationName(std::string& topNavDestName) override;
    WSError UpdateTitleInTargetPos(bool isShow, int32_t height) override;
    void NotifySessionForeground(uint32_t reason, bool withAnimation) override;
    void NotifySessionBackground(uint32_t reason, bool withAnimation, bool isFromInnerkits) override;
    WMError NotifyPrepareClosePiPWindow() override;
    void UpdateSubWindowState(const WindowType& type, bool waitDetach = false);
    WMError SetSystemBarProperties(const std::map<WindowType, SystemBarProperty>& properties,
        const std::map<WindowType, SystemBarPropertyFlag>& propertyFlags) override;
    WMError GetSystemBarProperties(std::map<WindowType, SystemBarProperty>& properties) override;
    WMError SetSpecificBarProperty(WindowType type, const SystemBarProperty& property) override;
    WMError SetSystemBarPropertyForPage(WindowType type, std::optional<SystemBarProperty> property) override;
    void GetSystemBarPropertyForPage(const std::map<WindowType, SystemBarProperty>& properties,
        std::map<WindowType, SystemBarProperty>& pageProperties);
    void ConsumePointerEvent(const std::shared_ptr<MMI::PointerEvent>& pointerEvent) override;
    bool PreNotifyKeyEvent(const std::shared_ptr<MMI::KeyEvent>& keyEvent) override;
    WSError NotifyDialogStateChange(bool isForeground) override;
    WMError SetDefaultDensityEnabled(bool enabled) override;
    bool GetDefaultDensityEnabled() override;
    WMError HideNonSecureWindows(bool shouldHide) override;
    void UpdateDensity() override;
    WSError UpdateOrientation() override;
    WMError GetTargetOrientationConfigInfo(Orientation targetOrientation,
        const std::map<Rosen::WindowType, Rosen::SystemBarProperty>& properties, Ace::ViewportConfig& config,
        std::map<AvoidAreaType, AvoidArea>& avoidAreas) override;
    WSError NotifyTargetRotationInfo(OrientationInfo& info) override;
    WSError UpdateDisplayId(uint64_t displayId) override;
    WMError AdjustKeyboardLayout(const KeyboardLayoutParams params) override;
    WMError CheckAndModifyWindowRect(uint32_t& width, uint32_t& height) override;
    WMError GetAppForceLandscapeConfig(AppForceLandscapeConfig& config) override;
    WSError NotifyAppForceLandscapeConfigUpdated() override;

    /*
     * Sub Window
     */
    WMError SetParentWindow(int32_t newParentWindowId) override;
    WMError GetParentWindow(sptr<Window>& parentWindow) override;

    /*
     * PC Window
     */
    WMError SetWindowMask(const std::vector<std::vector<uint32_t>>& windowMask) override;
    WMError SetFollowParentMultiScreenPolicy(bool enabled) override;
    WMError UseImplicitAnimation(bool useImplicit) override;

    /*
     * PC Window Layout
     */
    WMError Restore() override;
    WMError SetTitleAndDockHoverShown(bool isTitleHoverShown = true,
        bool isDockHoverShown = true) override;
    WMError SetWindowRectAutoSave(bool enabled, bool isSaveBySpecifiedFlag = false) override;
    WMError IsWindowRectAutoSave(bool& enabled) override;
    WMError MaximizeFloating() override;
    WMError MaximizeForCompatibleMode();
    WMError RecoverForCompatibleMode();
    WMError Maximize() override;
    WMError Maximize(MaximizePresentation presentation) override;
    WMError Recover() override;
    WMError Recover(uint32_t reason) override;
    WSError UpdateMaximizeMode(MaximizeMode mode) override;
    WMError SetSupportedWindowModes(const std::vector<AppExecFwk::SupportWindowMode>& supportedWindowModes,
        bool grayOutMaximizeButton = false) override;
    WmErrorCode StartMoveWindow() override;
    WmErrorCode StartMoveWindowWithCoordinate(int32_t offsetX, int32_t offsetY) override;
    WmErrorCode StopMoveWindow() override;
    WMError SetSupportedWindowModesInner(const std::vector<AppExecFwk::SupportWindowMode>& supportedWindowModes);
    void UpdateWindowModeWhenSupportTypeChange(uint32_t windowModeSupportType);
    bool haveSetSupportedWindowModes_ = false;
    uint32_t pendingWindowModeSupportType_ { WindowModeSupport::WINDOW_MODE_SUPPORT_ALL };

    /*
     * Compatible Mode
     */
    WSError NotifyCompatibleModePropertyChange(const sptr<CompatibleModeProperty> property) override;
    void HookDecorButtonStyleInCompatibleMode(uint32_t contentColor);
    WSError PcAppInPadNormalClose() override;
    void HandleWindowLimitsInCompatibleMode(WindowSizeLimits& windowSizeLimits);

    /*
     * Free Multi Window
     */
    WSError SwitchFreeMultiWindow(bool enable) override;
    virtual bool GetFreeMultiWindowModeEnabledState() override;
    void UpdateImmersiveBySwitchMode(bool freeMultiWindowEnable);

    void NotifyKeyboardPanelInfoChange(const KeyboardPanelInfo& keyboardPanelInfo) override;
    virtual WMError SetImmersiveModeEnabledState(bool enable) override;
    virtual bool GetImmersiveModeEnabledState() const override;
    WMError IsImmersiveLayout(bool& isImmersiveLayout) const override;
    void NotifySessionFullScreen(bool fullScreen) override;
    WMError GetWindowStatus(WindowStatus& windowStatus) override;
    bool GetIsUIExtFirstSubWindow() const override;
    bool GetIsUIExtAnySubWindow() const override;

    /*
     * Gesture Back
     */
    WMError SetGestureBackEnabled(bool enable) override;
    WMError GetGestureBackEnabled(bool& enable) const override;

    /*
     * PC Fold Screen
     */
    WSError SetFullScreenWaterfallMode(bool isWaterfallMode) override;
    WSError SetSupportEnterWaterfallMode(bool isSupportEnter) override;
    WMError OnContainerModalEvent(const std::string& eventName, const std::string& value) override;

    /*
     * Window Property
     */
    WMError SetCornerRadius(float cornerRadius) override;
    WMError SetWindowCornerRadius(float cornerRadius) override;
    WMError GetWindowCornerRadius(float& cornerRadius) override;
    WMError SetShadowRadius(float radius) override;
    WMError SyncShadowsToComponent(const ShadowsInfo& shadowsInfo) override;
    WMError SetShadowColor(std::string color) override;
    WMError SetShadowOffsetX(float offsetX) override;
    WMError SetShadowOffsetY(float offsetY) override;
    WMError SetBlur(float radius) override;
    WMError SetBackdropBlur(float radius) override;
    WMError SetBackdropBlurStyle(WindowBlurStyle blurStyle) override;
    WMError SetWindowMode(WindowMode mode) override;
    WMError SetGrayScale(float grayScale) override;
    WMError SetWindowShadowRadius(float radius) override;
    static void UpdateConfigurationSyncForAll(const std::shared_ptr<AppExecFwk::Configuration>& configuration);
    void UpdateConfigurationSync(const std::shared_ptr<AppExecFwk::Configuration>& configuration) override;
    float GetCustomDensity() const override;
    WMError SetCustomDensity(float density, bool applyToSubWindow) override;
    WMError GetWindowDensityInfo(WindowDensityInfo& densityInfo) override;
    WMError IsMainWindowFullScreenAcrossDisplays(bool& isAcrossDisplays) override;
    WMError GetWindowPropertyInfo(WindowPropertyInfo& windowPropertyInfo) override;

    /*
     * Window Decor
     */
    WMError SetWindowTitle(const std::string& title) override;
    WMError DisableAppWindowDecor() override;
    bool IsDecorEnable() const override;
    WMError Close() override;
    WMError CloseDirectly() override;

    /*
     * Starting Window
     */
    WMError NotifyRemoveStartingWindow() override;

    /*
     * Window Scene
     */
    WMError RegisterWindowAttachStateChangeListener(
        const sptr<IWindowAttachStateChangeListner>& listener) override;
    WMError UnregisterWindowAttachStateChangeListener() override;

    /*
     * Window Immersive
     */
    WMError GetAvoidAreaByType(AvoidAreaType type, AvoidArea& avoidArea, const Rect& rect = Rect::EMPTY_RECT,
        int32_t apiVersion = API_VERSION_INVALID) override;
    SystemBarProperty GetSystemBarPropertyByType(WindowType type) const override;
    WMError SetSystemBarProperty(WindowType type, const SystemBarProperty& property) override;
    WMError SetLayoutFullScreen(bool status) override;
    WMError SetIgnoreSafeArea(bool isIgnoreSafeArea) override;
    WMError SetFullScreen(bool status) override;
    WMError UpdateSystemBarProperties(const std::unordered_map<WindowType, SystemBarProperty>& systemBarProperties,
        const std::unordered_map<WindowType, SystemBarPropertyFlag>& systemBarPropertyFlags) override;
    /*
     * Window Pattern
     */
    WMError SetImageForRecent(uint32_t imgResourceId, ImageFit imageFit) override;
    /**
     * Window Transition Animation For PC
     */
    WMError SetWindowTransitionAnimation(WindowTransitionType transitionType,
        const TransitionAnimation& animation) override;
    std::shared_ptr<TransitionAnimation> GetWindowTransitionAnimation(WindowTransitionType transitionType) override;

    /*
     * Window LifeCycle
     */
    void Resume() override;
    void Pause() override;

    WSError CloseSpecificScene() override;
    WMError SetSubWindowSource(SubWindowSource source) override;

protected:
    WMError CreateAndConnectSpecificSession();
    WMError CreateSystemWindow(WindowType type);
    sptr<WindowSessionImpl> FindParentSessionByParentId(uint32_t parentId);
    bool IsSessionMainWindow(uint32_t parentId);
    void LimitWindowSize(uint32_t& width, uint32_t& height);
    void LimitCameraFloatWindowMininumSize(uint32_t& width, uint32_t& height, float& vpr);
    void UpdateFloatingWindowSizeBySizeLimits(uint32_t& width, uint32_t& height) const;
    WMError NotifyWindowSessionProperty();
    WMError NotifyWindowNeedAvoid(bool status = false);
    WMError SetLayoutFullScreenByApiVersion(bool status) override;
    void UpdateWindowSizeLimits();
    WindowLimits GetSystemSizeLimits(uint32_t displayWidth, uint32_t displayHeight, float vpr);
    void GetConfigurationFromAbilityInfo();
    std::vector<AppExecFwk::SupportWindowMode> ExtractSupportWindowModeFromMetaData(
        const std::shared_ptr<AppExecFwk::AbilityInfo>& abilityInfo);
    std::vector<AppExecFwk::SupportWindowMode> ParseWindowModeFromMetaData(
        const std::string& supportModesInFreeMultiWindow);
    float GetVirtualPixelRatio(const sptr<DisplayInfo>& displayInfo) override;
    WMError NotifySpecificWindowSessionProperty(WindowType type, const SystemBarProperty& property);
    using SessionMap = std::map<std::string, std::pair<int32_t, sptr<WindowSessionImpl>>>;
    sptr<WindowSessionImpl> FindParentMainSession(uint32_t parentId, const SessionMap& sessionMap);

    /*
     * Window Recover
     */
    WMError RecoverAndConnectSpecificSession();
    WMError RecoverAndReconnectSceneSession();
    void RegisterWindowRecoverStateChangeListener();

private:
    WMError DestroyInner(bool needNotifyServer);
    WMError MainWindowCloseInner();
    WMError SyncDestroyAndDisconnectSpecificSession(int32_t persistentId);
    bool IsValidSystemWindowType(const WindowType& type);
    WMError CheckParmAndPermission();
    void TransformSurfaceNode(const Transform& trans);
    void AdjustWindowAnimationFlag(bool withAnimation = false);
    WMError UpdateAnimationFlagProperty(bool withAnimation);
    WMError UpdateWindowModeImmediately(WindowMode mode);
    uint32_t UpdateConfigVal(uint32_t minVal, uint32_t maxVal, uint32_t configVal, uint32_t defaultVal, float vpr);
    void UpdateWindowState();
    void UpdateNewSize();
    void fillWindowLimits(WindowLimits& windowLimits);
    void UpdateSupportWindowModesWhenSwitchFreeMultiWindow();
    void PendingUpdateSupportWindowModesWhenSwitchMultiWindow();
    void ConsumePointerEventInner(const std::shared_ptr<MMI::PointerEvent>& pointerEvent,
        MMI::PointerEvent::PointerItem& pointerItem, bool isHitTargetDraggable = false);
    void HandleEventForCompatibleMode(const std::shared_ptr<MMI::PointerEvent>& pointerEvent,
        MMI::PointerEvent::PointerItem& pointerItem);
    void HandleDownForCompatibleMode(const std::shared_ptr<MMI::PointerEvent>& pointerEvent,
        MMI::PointerEvent::PointerItem& pointerItem);
    void HandleMoveForCompatibleMode(const std::shared_ptr<MMI::PointerEvent>& pointerEvent,
        MMI::PointerEvent::PointerItem& pointerItem);
    void HandleUpForCompatibleMode(const std::shared_ptr<MMI::PointerEvent>& pointerEvent,
        MMI::PointerEvent::PointerItem& pointerItem);
    void ConvertPointForCompatibleMode(const std::shared_ptr<MMI::PointerEvent>& pointerEvent,
        MMI::PointerEvent::PointerItem& pointerItem, int32_t transferX);
    bool IsInMappingRegionForCompatibleMode(int32_t displayX, int32_t displayY);
    bool CheckTouchSlop(int32_t pointerId, int32_t x, int32_t y, int32_t threshold);
    void IgnoreClickEvent(const std::shared_ptr<MMI::PointerEvent>& pointerEvent);
    bool HandlePointDownEvent(const std::shared_ptr<MMI::PointerEvent>& pointerEvent,
        const MMI::PointerEvent::PointerItem& pointerItem);
    std::unique_ptr<Media::PixelMap> HandleWindowMask(const std::vector<std::vector<uint32_t>>& windowMask);
    void CalculateNewLimitsByLimits(
        WindowLimits& newLimits, WindowLimits& customizedLimits, float& virtualPixelRatio);
    void CalculateNewLimitsByRatio(WindowLimits& newLimits, WindowLimits& customizedLimits);
    void NotifyDisplayInfoChange(const sptr<DisplayInfo>& info = nullptr);
    void UpdateDensityInner(const sptr<DisplayInfo>& info = nullptr);
    sptr<DisplayInfo> GetDisplayInfo() const;

    /*
     * Window Input Event
     */
    void ResetSuperFoldDisplayY(const std::shared_ptr<MMI::PointerEvent>& pointerEvent);

    /*
     * Window Recover
     */
    void RegisterSessionRecoverListener(bool isSpecificSession);
    void OnWindowRecoverStateChange(bool isSpecificSession, const WindowRecoverState& state);
    void UpdateStartRecoverProperty(bool isSpecificSession);
    void UpdateFinishRecoverProperty(bool isSpecificSession);
    NotifyWindowRecoverStateChangeFunc windowRecoverStateChangeFunc_;

    /*
     * Window Layout
     */
    void CheckMoveConfiguration(MoveConfiguration& moveConfiguration);
    WMError GetAppHookWindowInfoFromServer(HookWindowInfo& hookWindowInfo) override;
    bool ShouldSkipSupportWindowModeCheck(uint32_t windowModeSupportType, WindowMode mode);

    /*
     * PC Window Layout
     */
    std::shared_ptr<MMI::PointerEvent> lastPointerEvent_ = nullptr;
    bool IsFullScreenSizeWindow(uint32_t width, uint32_t height);
    bool isResizedByLimit_ = false;

    /*
     * Window Immersive
     */
    void UpdateDefaultStatusBarColor();
    WMError MoveAndResizeKeyboard(const KeyboardLayoutParams& params);
    bool userLimitsSet_ = false;
    bool forceLimits_ = false;
    bool enableDefaultAnimation_ = true;
    sptr<IAnimationTransitionController> animationTransitionController_;
    uint32_t setSameSystembarPropertyCnt_ = 0;
    std::atomic<uint32_t> getAvoidAreaCnt_ = 0;
    std::atomic<bool> enableImmersiveMode_ = false;
    std::atomic<bool> cacheEnableImmersiveMode_ = false;
    bool titleHoverShowEnabled_ = true;
    bool dockHoverShowEnabled_ = true;
    void PreLayoutOnShow(WindowType type, const sptr<DisplayInfo>& info = nullptr);

    /*
     * Gesture Back
     */
    bool gestureBackEnabled_ = true;

    /*
     * Window Property
     */
    void InitSystemSessionDragEnable();
    bool IsSystemDensityChanged(const sptr<DisplayInfo>& displayInfo);
    bool IsDefaultDensityEnabled();
    float GetMainWindowCustomDensity();
    float customDensity_ = UNDEFINED_DENSITY;
    bool isEnableDefaultDensityWhenCreate_ = false;
    std::string specifiedColorMode_;
    WMError SetPcAppInpadSpecificSystemBarInvisible();
    WMError SetPcAppInpadOrientationLandscape();

    /*
     * Sub Window
     */
    void AddSubWindowMapForExtensionWindow();
    WMError GetParentSessionAndVerify(bool isToast, sptr<WindowSessionImpl>& parentSession);
    static WMError VerifySubWindowLevel(bool isToast, const sptr<WindowSessionImpl>& parentSession);
    WMError SetParentWindowInner(int32_t oldParentWindowId, const sptr<WindowSessionImpl>& newParentWindow);

    WMError RegisterKeyboardPanelInfoChangeListener(const sptr<IKeyboardPanelInfoChangeListener>& listener) override;
    WMError UnregisterKeyboardPanelInfoChangeListener(const sptr<IKeyboardPanelInfoChangeListener>& listener) override;
    static std::mutex keyboardPanelInfoChangeListenerMutex_;
    sptr<IKeyboardPanelInfoChangeListener> keyboardPanelInfoChangeListeners_ = nullptr;
    bool isOverTouchSlop_ = false;
    bool isDown_ = false;
    std::unordered_map<int32_t, std::vector<bool>> eventMapTriggerByDisplay_;
    std::unordered_map<int32_t, std::vector<int32_t>> eventMapDeltaXByDisplay_;
    std::unordered_map<int32_t, std::vector<PointInfo>> downPointerByDisplay_;

    /*
     * PC Fold Screen
     */
    std::atomic_bool isWaterfallToMaximize_ { false };
    std::atomic<WindowMode> lastWindowModeBeforeWaterfall_ { WindowMode::WINDOW_MODE_UNDEFINED };

    /*
     * Move Drag
     */
    bool CalcWindowShouldMove();
    bool CheckCanMoveWindowType();
    bool CheckCanMoveWindowTypeByDevice();
    bool CheckIsPcAppInPadFullScreenOnMobileWindowMode();
    AreaType GetDragAreaByDownEvent(const std::shared_ptr<MMI::PointerEvent>& pointerEvent,
        const MMI::PointerEvent::PointerItem& pointerItem);

    /*
     * PC Window
     */
    /* whether in process of delay raise during cross window drag and drop
     * only used when delay raise is enabled and hit draggable component
     */
    bool isExecuteDelayRaise_ = false;
    bool IsFullScreenEnable() const;

    /*
     * Window Input Event
     */
    int32_t superFoldOffsetY_ = -1; // calculate the total height of the display_B area and crease area.

    /*
     * Window Scene
     */
    static std::mutex windowAttachStateChangeListenerMutex_;
    sptr<IWindowAttachStateChangeListner> windowAttachStateChangeListener_;
    WSError NotifyWindowAttachStateChange(bool isAttach) override;

    /*
     * Window Rotation
     */
    Ace::ViewportConfig FillTargetOrientationConfig(
        const OrientationInfo& info, const sptr<DisplayInfo>& displayInfo, uint64_t displayId);

    /*
     * Window Lifecycle
     */
    void NotifyFreeMultiWindowModeResume();
    std::string TransferLifeCycleEventToString(LifeCycleEvent type) const;
    void RecordLifeCycleExceptionEvent(LifeCycleEvent event, WMError erCode) const;
    WindowLifeCycleInfo GetWindowLifecycleInfo() const;

    /**
     * Window Transition Animation For PC
     */
    std::mutex transitionAnimationConfigMutex_;

    /*
     * Window Decor
     */
    WMError GrayOutMaximizeButton(bool isGrayOut);
};
} // namespace Rosen
} // namespace OHOS
#endif // OHOS_ROSEN_WINDOW_SCENE_SESSION_IMPL_H
