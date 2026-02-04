/*
 * Copyright (c) 2022-2024 Huawei Device Co., Ltd.
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

#include <ui_content.h>
#include "context.h"
#include "ui/rs_surface_node.h"
#include "vsync_station.h"
#include "window.h"
#include "window_property.h"

namespace OHOS::AbilityRuntime {
class Context;
}

namespace OHOS {
namespace Rosen {
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

class WindowImpl : public Window {
public:
    explicit WindowImpl(const sptr<WindowOption>& option);
    ~WindowImpl();
    static sptr<Window> Find(const std::string& id);
    static sptr<Window> GetTopWindowWithContext(const std::shared_ptr<AbilityRuntime::Context>& context = nullptr);
    static sptr<Window> GetTopWindowWithId(uint32_t mainWinId);
    static std::vector<sptr<Window>> GetSubWindow(uint32_t parantId);
    static void UpdateConfigurationForAll(const std::shared_ptr<AppExecFwk::Configuration>& configuration,
        const std::vector<std::shared_ptr<AbilityRuntime::Context>>& ignoreWindowContexts = {});
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
    virtual SystemBarProperty GetSystemBarPropertyByType(WindowType type) const override;
    virtual bool IsFullScreen() const override;
    virtual bool IsLayoutFullScreen() const override;
    virtual WMError SetWindowType(WindowType type) override;
    virtual WMError SetWindowMode(WindowMode mode) override;
    virtual WMError SetAlpha(float alpha) override;
    virtual WMError SetTransform(const Transform& trans) override;
    virtual WMError AddWindowFlag(WindowFlag flag) override;
    virtual WMError RemoveWindowFlag(WindowFlag flag) override;
    virtual WMError SetWindowFlags(uint32_t flags) override;
    virtual WMError SetSystemBarProperty(WindowType type, const SystemBarProperty& property) override;
    virtual WMError SetSpecificBarProperty(WindowType type, const SystemBarProperty& property) override;
    virtual WMError SetStatusBarColorForNavigation(const std::optional<uint32_t> color) override;
    WMError UpdateSystemBarPropertyForPage(WindowType type,
        const SystemBarProperty& systemBarProperty, const SystemBarPropertyFlag& systemBarPropertyFlag) override;
    virtual WMError GetSystemBarProperties(std::map<WindowType, SystemBarProperty>& properties) override;
    void UpdateSpecificSystemBarEnabled(bool systemBarEnable, bool systemBarEnableAnimation,
        SystemBarProperty& property) override;
    virtual WMError SetLayoutFullScreen(bool status) override;
    virtual WMError SetFullScreen(bool status) override;
    virtual const Transform& GetTransform() const override;
    virtual WMError UpdateSurfaceNodeAfterCustomAnimation(bool isAdd) override;
    virtual WMError GetAvoidAreaByType(AvoidAreaType type, AvoidArea& avoidArea) override;

    WMError Create(uint32_t parentId,
        const std::shared_ptr<AbilityRuntime::Context>& context = nullptr);
    virtual WMError Destroy() override;
    void SetShowWithOptions(bool showWithOptions) override;
    bool IsShowWithOptions() const override;
    virtual WMError Show(uint32_t reason = 0, bool withAnimation = false, bool withFocus = true) override;
    virtual WMError Show(uint32_t reason, bool withAnimation, bool withFocus, bool waitAttach) override;
    virtual WMError Hide(uint32_t reason = 0, bool withAnimation = false, bool isFromInnerkits = true) override;
    virtual WMError Hide(uint32_t reason, bool withAnimation, bool isFromInnerkits, bool waitDetach) override;
    virtual WMError MoveTo(int32_t x, int32_t y, bool isMoveToGlobal = false,
        MoveConfiguration moveConfiguration = {}) override;
    virtual WMError Resize(uint32_t width, uint32_t height) override;
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
    virtual WMError SetDialogBackGestureEnabled(bool isEnabled) override;
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

    virtual WMError Maximize() override;
    virtual WMError Minimize() override;
    virtual WMError Recover() override;
    virtual WMError Close() override;
    virtual void StartMove() override;

    virtual WMError RequestFocus() const override;
    virtual bool IsFocused() const override;
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
    virtual WMError RegisterOccupiedAreaChangeListener(const sptr<IOccupiedAreaChangeListener>& listener) override;
    virtual WMError UnregisterOccupiedAreaChangeListener(const sptr<IOccupiedAreaChangeListener>& listener) override;
    virtual WMError RegisterTouchOutsideListener(const sptr<ITouchOutsideListener>& listener) override;
    virtual WMError UnregisterTouchOutsideListener(const sptr<ITouchOutsideListener>& listener) override;
    virtual WMError RegisterAnimationTransitionController(
        const sptr<IAnimationTransitionController>& listener) override;
    virtual WMError RegisterScreenshotListener(const sptr<IScreenshotListener>& listener) override;
    virtual WMError UnregisterScreenshotListener(const sptr<IScreenshotListener>& listener) override;
    virtual WMError RegisterDialogTargetTouchListener(const sptr<IDialogTargetTouchListener>& listener) override;
    virtual WMError UnregisterDialogTargetTouchListener(const sptr<IDialogTargetTouchListener>& listener) override;
    virtual void RegisterDialogDeathRecipientListener(const sptr<IDialogDeathRecipientListener>& listener) override;
    virtual void UnregisterDialogDeathRecipientListener(const sptr<IDialogDeathRecipientListener>& listener) override;
    virtual void SetAceAbilityHandler(const sptr<IAceAbilityHandler>& handler) override;
    virtual void SetRequestWindowModeSupportType(uint32_t windowModeSupportType) override;
    virtual void ConsumeKeyEvent(const std::shared_ptr<MMI::KeyEvent>& inputEvent) override;
    virtual void ConsumeBackEvent() override;
    virtual void ConsumePointerEvent(const std::shared_ptr<MMI::PointerEvent>& inputEvent) override;
    virtual bool IsDialogSessionBackGestureEnabled() override;
    virtual void RequestVsync(const std::shared_ptr<VsyncCallback>& vsyncCallback) override;
    virtual int64_t GetVSyncPeriod() override;
    virtual void UpdateConfiguration(const std::shared_ptr<AppExecFwk::Configuration>& configuration) override;
    void UpdateConfigurationForSpecified(const std::shared_ptr<AppExecFwk::Configuration>& configuration,
        const std::shared_ptr<Global::Resource::ResourceManager>& resourceManager) override;
    void UpdateAvoidArea(const sptr<AvoidArea>& avoidArea, AvoidAreaType type) override;
    void NotifyTouchDialogTarget(int32_t posX = 0, int32_t posY = 0) override;
    WMError SetUIContentByName(const std::string& contentInfo, napi_env env, napi_value storage,
        AppExecFwk::Ability* ability) override;
    virtual WMError NapiSetUIContent(const std::string& contentInfo, napi_env env, napi_value storage,
        BackupAndRestoreType type, sptr<IRemoteObject> token, AppExecFwk::Ability* ability) override;
    virtual std::string GetContentInfo(BackupAndRestoreType type = BackupAndRestoreType::CONTINUATION) override;
    virtual const std::shared_ptr<AbilityRuntime::Context> GetContext() const override;
    virtual Ace::UIContent* GetUIContent() const override;
    virtual void OnNewWant(const AAFwk::Want& want) override;
    virtual void SetRequestedOrientation(Orientation) override;
    virtual void NotifyPreferredOrientationChange(Orientation orientation) override;
    virtual void SetUserRequestedOrientation(Orientation orientation) override;
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
    WMError RaiseToAppTop() override;
    virtual WMError SetAspectRatio(float ratio) override;
    virtual WMError ResetAspectRatio() override;
    virtual KeyboardAnimationConfig GetKeyboardAnimationConfig() override;

    virtual void SetNeedDefaultAnimation(bool needDefaultAnimation) override;

    virtual void SetViewportConfig(const Ace::ViewportConfig& config) override;
    virtual void UpdateViewportConfig() override;
    virtual void SetOrientation(Orientation orientation) override;
    virtual void SetSize(int32_t width, int32_t height) override;
    virtual void SetDensity(float density) override;

    virtual void CreateSurfaceNode(const std::string name, const SendRenderDataCallback& callback) override;
    virtual void SetContentInfoCallback(const ContentInfoCallback& callback) override;
    virtual WMError SetResizeByDragEnabled(bool dragEnabled) override;
    virtual WMError SetRaiseByClickEnabled(bool raiseEnabled) override;
    virtual WMError RaiseAboveTarget(int32_t subWindowId) override;
    virtual WMError HideNonSystemFloatingWindows(bool shouldHide) override;
    virtual WMError RegisterWindowVisibilityChangeListener(const WindowVisibilityListenerSptr& listener) override;
    virtual WMError UnregisterWindowVisibilityChangeListener(const WindowVisibilityListenerSptr& listener) override;
    virtual WMError RegisterSystemBarEnableListener(const sptr<IWindowSystemBarEnableListener>& listener) override;
    virtual WMError UnRegisterSystemBarEnableListener(const sptr<IWindowSystemBarEnableListener>& listener) override;
    virtual WMError RegisterIgnoreViewSafeAreaListener(const sptr<IIgnoreViewSafeAreaListener>& listener) override;
    virtual WMError UnRegisterIgnoreViewSafeAreaListener(const sptr<IIgnoreViewSafeAreaListener>& listener) override;
    virtual WmErrorCode KeepKeyboardOnFocus(bool keepKeyboardFlag) override;
    virtual WMError SetSingleFrameComposerEnabled(bool enable) override;
    virtual WMError SetLandscapeMultiWindow(bool isLandscapeMultiWindow) override;
    virtual void SetUiDvsyncSwitch(bool dvsyncSwitch) override;
    virtual void SetTouchEvent(int32_t touchType) override;
    virtual WMError UpdateSystemBarProperty(bool status);
    virtual WMError SetImmersiveModeEnabledState(bool enable) override;
    virtual bool GetImmersiveModeEnabledState() const override;

    /*
     * Window Property
     */
    static void UpdateConfigurationSyncForAll(const std::shared_ptr<AppExecFwk::Configuration>& configuration);
    void UpdateConfigurationSync(const std::shared_ptr<AppExecFwk::Configuration>& configuration) override;
    uint32_t GetApiTargetVersion() const;

private:
    static sptr<Window> FindWindowById(uint32_t windowId);
    template<typename T1, typename T2, typename Ret>
    using EnableIfSame = typename std::enable_if<std::is_same_v<T1, T2>, Ret>::type;
    template<typename T> WMError RegisterListener(std::vector<sptr<T>>& holder, const sptr<T>& listener);
    template<typename T> WMError UnregisterListener(std::vector<sptr<T>>& holder, const sptr<T>& listener);
    template<typename T>
    inline EnableIfSame<T, IWindowSystemBarEnableListener, std::vector<sptr<IWindowSystemBarEnableListener>>>
        GetListeners()
    {
        std::vector<sptr<IWindowSystemBarEnableListener>> systemBarEnableListeners;
        {
            std::lock_guard<std::mutex> lock(globalMutex_);
            for (auto& listener : systemBarEnableListeners_[GetWindowId()]) {
                systemBarEnableListeners.push_back(listener);
            }
        }
        return systemBarEnableListeners;
    }
    template<typename T>
    inline EnableIfSame<T, IIgnoreViewSafeAreaListener, std::vector<sptr<IIgnoreViewSafeAreaListener>>> GetListeners()
    {
        std::vector<sptr<IIgnoreViewSafeAreaListener>> ignoreSafeAreaListeners;
        {
            std::lock_guard<std::mutex> lock(globalMutex_);
            for (auto& listener : ignoreSafeAreaListeners_[GetWindowId()]) {
                ignoreSafeAreaListeners.push_back(listener);
            }
        }
        return ignoreSafeAreaListeners;
    }
    template<typename T>
    inline EnableIfSame<T, IAvoidAreaChangedListener, std::vector<sptr<IAvoidAreaChangedListener>>> GetListeners()
    {
        std::vector<sptr<IAvoidAreaChangedListener>> avoidAreaChangeListeners;
        {
            std::lock_guard<std::mutex> lock(globalMutex_);
            for (auto& listener : avoidAreaChangeListeners_[GetWindowId()]) {
                avoidAreaChangeListeners.push_back(listener);
            }
        }
        return avoidAreaChangeListeners;
    }
    void ClearListenersById(uint32_t winId);
    void NotifySystemBarChange(WindowType type, const SystemBarProperty& property);
    void NotifySetIgnoreSafeArea(bool value);
    void NotifyAvoidAreaChange(const sptr<AvoidArea>& avoidArea, AvoidAreaType type);
    static std::mutex globalMutex_;
    static std::map<std::string, std::pair<uint32_t, sptr<Window>>> windowMap_;
    static std::map<uint32_t, std::vector<sptr<WindowImpl>>> subWindowMap_;
    static std::map<uint32_t, std::vector<sptr<IWindowSystemBarEnableListener>>> systemBarEnableListeners_;
    static std::map<uint32_t, std::vector<sptr<IIgnoreViewSafeAreaListener>>> ignoreSafeAreaListeners_;
    static std::map<uint32_t, std::vector<sptr<IAvoidAreaChangedListener>>> avoidAreaChangeListeners_;
    WindowState state_ { WindowState::STATE_INITIAL };
    std::shared_ptr<RSSurfaceNode> surfaceNode_;
    std::shared_ptr<VsyncStation> vsyncStation_ = nullptr;
    std::shared_ptr<AbilityRuntime::Context> context_;
    std::string name_;
    std::unique_ptr<Ace::UIContent> uiContent_;
    KeyboardAnimationConfig keyboardAnimationConfig_;
    bool needRemoveWindowInputChannel_ = false;
    ContentInfoCallback contentInfoCallback_;
    Transform transform_;
    int32_t width_ = 0;
    int32_t height_ = 0;
    int32_t orientation_ = 0;
    float density_ = 1.0f;
    bool isIgnoreSafeArea_ = false;
    uint32_t windowId_ = 0;
    WindowMode windowMode_ = WindowMode::WINDOW_MODE_FULLSCREEN;
    sptr<WindowProperty> property_;
    mutable std::mutex mutex_;
    bool showWithOptions_ = false;
    std::unordered_map<WindowType, SystemBarProperty> sysBarPropMap_ {
        { WindowType::WINDOW_TYPE_STATUS_BAR,           SystemBarProperty(true, 0x00FFFFFF, 0xFF000000) },
        { WindowType::WINDOW_TYPE_NAVIGATION_BAR,       SystemBarProperty(true, 0x00FFFFFF, 0xFF000000) },
        { WindowType::WINDOW_TYPE_NAVIGATION_INDICATOR, SystemBarProperty(true, 0x00FFFFFF, 0xFF000000) },
    };
    std::unordered_map<AvoidAreaType, sptr<AvoidArea>> avoidAreaMap_ {
        { AvoidAreaType::TYPE_SYSTEM,               new AvoidArea() },
        { AvoidAreaType::TYPE_CUTOUT,               new AvoidArea() },
        { AvoidAreaType::TYPE_SYSTEM_GESTURE,       new AvoidArea() },
        { AvoidAreaType::TYPE_KEYBOARD,             new AvoidArea() },
        { AvoidAreaType::TYPE_NAVIGATION_INDICATOR, new AvoidArea() },
    };
};
} // namespace Rosen
} // namespace OHOS
#endif // OHOS_ROSEN_WINDOW_IMPL_H
