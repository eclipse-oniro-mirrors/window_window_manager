/*
 * Copyright (c) 2023-2024 Huawei Device Co., Ltd.
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

#ifndef OHOS_ROSEN_WINDOW_H
#define OHOS_ROSEN_WINDOW_H

#include <parcel.h>
#include <refbase.h>
#include <pixel_map.h>
#include <iremote_object.h>

#include "wm_common.h"
#include "window_option.h"

typedef struct napi_env__* napi_env;
typedef struct napi_value__* napi_value;

namespace OHOS::MMI {
class PointerEvent;
class KeyEvent;
class AxisEvent;
}

namespace OHOS::AppExecFwk {
class Configuration;
class Ability;
enum class SupportWindowMode;
}

namespace OHOS::Global::Resource {
class ResourceManager;
} // namespace OHOS::Global::Resource

namespace OHOS::AbilityRuntime {
class AbilityContext;
class Context;
}

namespace OHOS::Ace {
class UIContent;
class ViewportConfig;
}

namespace OHOS::AAFwk {
class Want;
class WantParams;
}

namespace OHOS {
namespace Rosen {
class RSSurfaceNode;
class RSTransaction;
class RSUIContext;
class RSUIDirector;
using NotifyNativeWinDestroyFunc = std::function<void(std::string windowName)>;
using SendRenderDataCallback = bool (*)(const void*, const size_t, const int32_t, const int32_t, const uint64_t);
using ContentInfoCallback = std::function<void(std::string contentInfo)>;
enum class ImageFit;

class IWindowLifeCycle : virtual public RefBase {
};
class IWindowChangeListener : virtual public RefBase {
};
class IWindowSystemBarEnableListener : virtual public RefBase {
public:
    virtual WMError OnSetSpecificBarProperty(WindowType type, const SystemBarProperty& property) = 0;
};
class IIgnoreViewSafeAreaListener : virtual public RefBase {
public:
    virtual void SetIgnoreViewSafeArea(bool ignoreViewSafeArea) = 0;
};
class IAvoidAreaChangedListener : virtual public RefBase {
public:
    virtual void OnAvoidAreaChanged(const AvoidArea avoidArea, AvoidAreaType type) {}
};
class IWindowDragListener : virtual public RefBase {
};
class IDisplayMoveListener : virtual public RefBase {
};
class IDispatchInputEventListener : virtual public RefBase {
};
class OccupiedAreaChangeInfo : public Parcelable {
public:
    virtual bool Marshalling(Parcel& parcel) const override;
    static OccupiedAreaChangeInfo* Unmarshalling(Parcel& parcel);
};
class IOccupiedAreaChangeListener : virtual public RefBase {
};
class IAceAbilityHandler : virtual public RefBase {
};
class IInputEventConsumer {
};
class ITouchOutsideListener : virtual public RefBase {
};
class IAnimationTransitionController : virtual public RefBase {
public:
    virtual void AnimationForShown() = 0;
    virtual void AnimationForHidden() = 0;
};
class ISystemBarPropertyListener : virtual public RefBase {
public:
    virtual void OnSystemBarPropertyUpdate(WindowType type, const SystemBarProperty& property) {}
};
class IScreenshotListener : virtual public RefBase {
};
class IScreenshotAppEventListener : virtual public RefBase {
};
class IDialogTargetTouchListener : virtual public RefBase {
};
class IDialogDeathRecipientListener : virtual public RefBase {
};
class IWindowTitleButtonRectChangedListener : virtual public RefBase {
};
class IWindowVisibilityChangedListener : virtual public RefBase {
};

using WindowVisibilityListenerSptr = sptr<IWindowVisibilityChangedListener>;

class IWindowNoInteractionListener : virtual public RefBase {
};
using IWindowNoInteractionListenerSptr = sptr<IWindowNoInteractionListener>;

static WMError DefaultCreateErrCode = WMError::WM_OK;
class WINDOW_EXPORT Window : public RefBase {
public:
    static sptr<Window> Create(const std::string& windowName,
    sptr<WindowOption>& option, const std::shared_ptr<AbilityRuntime::Context>& context = nullptr,
    WMError& errCode = DefaultCreateErrCode, const std::shared_ptr<RSUIContext>& rsUIContext = nullptr);
    static sptr<Window> Find(const std::string& windowName);
    static sptr<Window> GetTopWindowWithContext(const std::shared_ptr<AbilityRuntime::Context>& context = nullptr);
    static sptr<Window> GetTopWindowWithId(uint32_t mainWinId);
    static std::vector<sptr<Window>> GetSubWindow(uint32_t parentId);
    static void UpdateConfigurationForAll(const std::shared_ptr<AppExecFwk::Configuration>& configuration,
       const std::vector<std::shared_ptr<AbilityRuntime::Context>>& ignoreWindowContexts = {});
    static void UpdateConfigurationSyncForAll(const std::shared_ptr<AppExecFwk::Configuration>& configuration);
    virtual std::shared_ptr<RSSurfaceNode> GetSurfaceNode() const = 0;

    /**
     * @brief Get the associated RSUIDirector instance
     *
     * @return std::shared_ptr<RSUIDirector> Shared pointer to the RSUIDirector instance,
     *         or nullptr if RS client multi-instance is disabled.
     */
    virtual std::shared_ptr<RSUIDirector> GetRSUIDirector() const { return nullptr; }

    /**
     * @brief Get the associated RSUIContext instance
     *
     * @return std::shared_ptr<RSUIContext> Shared pointer to the RSUIContext instance,
     *         or nullptr if RS client multi-instance is disabled.
     */
    virtual std::shared_ptr<RSUIContext> GetRSUIContext() const { return nullptr; }

    virtual const std::shared_ptr<AbilityRuntime::Context> GetContext() const = 0;
    virtual Rect GetRect() const = 0;
    virtual Rect GetRequestRect() const = 0;

    /**
     * @brief Get the window rectangle in global coordinates.
     *
     * @return The rectangle (position and size) of the window in global coordinates.
     */
    virtual Rect GetGlobalDisplayRect() const { return { 0, 0, 0, 0 }; }

    /**
     * @brief Convert a position from client (window-relative) coordinates to global coordinates.
     *
     * @param inPosition The position relative to the window.
     * @param outPosition [out] The corresponding position in global coordinates.
     * @return WMError::WM_OK on success, or appropriate error code on failure.
     */
    virtual WMError ClientToGlobalDisplay(const Position& inPosition, Position& outPosition) const
    {
        return WMError::WM_ERROR_DEVICE_NOT_SUPPORT;
    }

    /**
     * @brief Convert a position from global coordinates to client (window-relative) coordinates.
     *
     * @param inPosition The position in global coordinates.
     * @param outPosition [out] The corresponding position relative to the window.
     * @return WMError::WM_OK on success, or appropriate error code on failure.
     */
    virtual WMError GlobalDisplayToClient(const Position& inPosition, Position& outPosition) const
    {
        return WMError::WM_ERROR_DEVICE_NOT_SUPPORT;
    }

    virtual WindowType GetType() const = 0;
    virtual WindowMode GetWindowMode() const = 0;
    virtual float GetAlpha() const = 0;
    virtual const std::string& GetWindowName() const = 0;
    virtual uint32_t GetWindowId() const = 0;
    virtual uint64_t GetDisplayId() const = 0;
    virtual uint32_t GetWindowFlags() const = 0;
    virtual WindowState GetWindowState() const = 0;
    virtual WMError SetFocusable(bool isFocusable) = 0;
    virtual bool GetFocusable() const = 0;
    virtual WMError SetTouchable(bool isTouchable) = 0;
    virtual SystemBarProperty GetSystemBarPropertyByType(WindowType type) const = 0;
    virtual bool GetTouchable() const = 0;
    virtual bool IsLayoutFullScreen() const = 0;
    virtual bool IsFullScreen() const = 0;
    virtual WMError SetWindowMode(WindowMode mode) = 0;
    virtual WMError SetWindowType(WindowType type) = 0;
    virtual WMError SetFollowScreenChange(bool isFollowScreenChange) { return WMError::WM_ERROR_DEVICE_NOT_SUPPORT; }
    virtual WMError SetAlpha(float alpha) = 0;
    virtual WMError SetTransform(const Transform& trans) = 0;
    virtual const Transform& GetTransform() const = 0;
    virtual WMError RemoveWindowFlag(WindowFlag flag) = 0;
    virtual WMError AddWindowFlag(WindowFlag flag) = 0;
    virtual WMError SetWindowFlags(uint32_t flags) = 0;
    virtual WMError IsWindowRectAutoSave(bool& enabled) { return WMError::WM_ERROR_DEVICE_NOT_SUPPORT; }
    virtual WMError GetAvoidAreaByType(AvoidAreaType type, AvoidArea& avoidArea) = 0;
    virtual WMError GetAvoidAreaByTypeIgnoringVisibility(AvoidAreaType type, AvoidArea& avoidArea,
        const Rect& rect = Rect::EMPTY_RECT) { return WMError::WM_ERROR_DEVICE_NOT_SUPPORT; }
    virtual WMError SetAvoidAreaOption(uint32_t avoidAreaOption) { return WMError::WM_OK; }
    virtual WMError GetAvoidAreaOption(uint32_t& avoidAreaOption) { return WMError::WM_OK; }
    virtual WMError UpdateSystemBarProperties(
        const std::unordered_map<WindowType, SystemBarProperty>& systemBarProperties,
        const std::unordered_map<WindowType, SystemBarPropertyFlag>& systemBarPropertyFlags)
        { return WMError::WM_OK; }
    virtual void UpdateSpecificSystemBarEnabled(bool systemBarEnable, bool systemBarEnableAnimation,
        SystemBarProperty& property) {}
    virtual WMError UpdateSystemBarPropertyForPage(WindowType type,
        const SystemBarProperty& systemBarProperty, const SystemBarPropertyFlag& systemBarPropertyFlag)
    {
        return WMError::WM_OK;
    }
    virtual WMError SetSystemBarProperty(WindowType type, const SystemBarProperty& property) = 0;
    virtual WMError SetSpecificBarProperty(WindowType type, const SystemBarProperty& property) = 0;
    virtual WMError SetSystemBarProperties(const std::map<WindowType, SystemBarProperty>& properties,
        const std::map<WindowType, SystemBarPropertyFlag>& propertyFlags) = 0;
    virtual WMError GetSystemBarProperties(std::map<WindowType, SystemBarProperty>& properties) = 0;
    virtual WMError SetFullScreen(bool status) = 0;
    virtual WMError SetLayoutFullScreen(bool status) = 0;
    virtual WMError SetIgnoreSafeArea(bool isIgnoreSafeArea) { return WMError::WM_OK; }
    virtual WMError SetTitleAndDockHoverShown(bool titleHoverShowEnabled = true,
        bool dockHoverShowEnabled = true) { return WMError::WM_ERROR_DEVICE_NOT_SUPPORT; }
    virtual WMError Destroy() = 0;
    virtual void SetShowWithOptions(bool showWithOptions) {}
    virtual bool IsShowWithOptions() const { return false; }
    virtual WMError Show(uint32_t reason = 0, bool withAnimation = false, bool withFocus = true) = 0;
    virtual WMError Show(uint32_t reason, bool withAnimation, bool withFocus, bool waitAttach) = 0;
    virtual WMError Hide(uint32_t reason = 0, bool withAnimation = false, bool isFromInnerkits = true) = 0;
    virtual WMError Hide(uint32_t reason, bool withAnimation, bool isFromInnerkits, bool waitDetach) = 0;
    virtual WMError MoveTo(int32_t x, int32_t y, bool isMoveToGlobal = false,
        MoveConfiguration moveConfiguration = {}) = 0;
    virtual WMError MoveToAsync(int32_t x, int32_t y,
        MoveConfiguration moveConfiguration = {}) { return WMError::WM_ERROR_DEVICE_NOT_SUPPORT; }
    virtual WMError MoveWindowToGlobal(int32_t x, int32_t y,
        MoveConfiguration moveConfiguration) { return WMError::WM_OK; }

    /**
     * @brief Move the window to the specified position in global coordinates.
     *
     * @param x The target X-coordinate in global coordinates.
     * @param y The target Y-coordinate in global coordinates.
     * @param moveConfiguration Optional move configuration parameters.
     * @return WMError WM_OK if the move operation succeeds; otherwise, an error code is returned.
     */
    virtual WMError MoveWindowToGlobalDisplay(int32_t x, int32_t y, MoveConfiguration moveConfiguration = {})
    {
        return WMError::WM_ERROR_DEVICE_NOT_SUPPORT;
    }

    virtual WMError GetGlobalScaledRect(Rect& globalScaledRect) { return WMError::WM_ERROR_DEVICE_NOT_SUPPORT; }
    virtual WMError Resize(uint32_t width, uint32_t height, const RectAnimationConfig& rectAnimationConfig = {}) = 0;
    virtual WMError ResizeAsync(uint32_t width, uint32_t height,
        const RectAnimationConfig& rectAnimationConfig = {}) { return WMError::WM_ERROR_DEVICE_NOT_SUPPORT; }
    virtual WMError SetWindowGravity(WindowGravity gravity, uint32_t percent) = 0;
    virtual WMError SetKeepScreenOn(bool keepScreenOn) = 0;
    virtual bool IsKeepScreenOn() const = 0;
    virtual WMError SetTurnScreenOn(bool turnScreenOn) = 0;
    virtual bool IsTurnScreenOn() const = 0;
    virtual WMError SetBackgroundColor(const std::string& color) = 0;
    virtual WMError SetTransparent(bool isTransparent) = 0;
    virtual bool IsTransparent() const = 0;
    virtual WMError SetBrightness(float brightness) = 0;
    virtual float GetBrightness() const = 0;
    virtual WMError ChangeCallingWindowId(uint32_t windowId) = 0;
    virtual WMError SetPrivacyMode(bool isPrivacyMode) = 0;
    virtual bool IsPrivacyMode() const = 0;
    virtual void SetSystemPrivacyMode(bool isSystemPrivacyMode) = 0;
    virtual WMError BindDialogTarget(sptr<IRemoteObject> targetToken) = 0;
    virtual WMError SetDialogBackGestureEnabled(bool isEnabled) = 0;
    virtual WMError RaiseToAppTop() = 0;
    virtual WMError SetSnapshotSkip(bool isSkip) = 0;
    virtual WMError SetCornerRadius(float cornerRadius) = 0;
    virtual WMError SetWindowCornerRadius(float radius) { return WMError::WM_ERROR_DEVICE_NOT_SUPPORT; }
    virtual WMError GetWindowCornerRadius(float& radius) { return WMError::WM_ERROR_DEVICE_NOT_SUPPORT; }
    virtual WMError SetShadowRadius(float radius) = 0;
    virtual WMError SetWindowShadowRadius(float radius) { return WMError::WM_ERROR_DEVICE_NOT_SUPPORT; }
    virtual WMError SetShadowColor(std::string color) = 0;
    virtual WMError SetShadowOffsetX(float offsetX) = 0;
    virtual WMError SetShadowOffsetY(float offsetY) = 0;
    virtual WMError SyncShadowsToComponent(const ShadowsInfo& shadowsInfo)
    {
        return WMError::WM_ERROR_DEVICE_NOT_SUPPORT;
    }
    virtual WMError SetBlur(float radius) = 0;
    virtual WMError SetBackdropBlur(float radius) = 0;
    virtual WMError SetBackdropBlurStyle(WindowBlurStyle blurStyle) = 0;
    virtual WMError RequestFocus() const = 0;
    virtual WMError RequestFocusByClient(bool isFocused) { return WMError::WM_ERROR_DEVICE_NOT_SUPPORT; }
    virtual bool IsFocused() const = 0;
    virtual WMError UpdateSurfaceNodeAfterCustomAnimation(bool isAdd) = 0;
    virtual void SetInputEventConsumer(const std::shared_ptr<IInputEventConsumer>& inputEventConsumer) = 0;
    virtual void ConsumeKeyEvent(const std::shared_ptr<MMI::KeyEvent>& inputEvent) = 0;
    virtual void ConsumeBackEvent() {}
    virtual void ConsumePointerEvent(const std::shared_ptr<MMI::PointerEvent>& inputEvent) = 0;
    virtual bool IsDialogSessionBackGestureEnabled() { return false; }
    virtual void RequestVsync(const std::shared_ptr<VsyncCallback>& vsyncCallback) = 0;
    virtual int64_t GetVSyncPeriod() = 0;
    virtual void FlushFrameRate(uint32_t rate, int32_t animatorExpectedFrameRate, uint32_t rateType) {}
    virtual void UpdateConfiguration(const std::shared_ptr<AppExecFwk::Configuration>& configuration) = 0;
    virtual void UpdateConfigurationForSpecified(const std::shared_ptr<AppExecFwk::Configuration>& configuration,
        const std::shared_ptr<Global::Resource::ResourceManager>& resourceManager) = 0;
    virtual void UpdateConfigurationSync(const std::shared_ptr<AppExecFwk::Configuration>& configuration) = 0;
    virtual WMError RegisterLifeCycleListener(const sptr<IWindowLifeCycle>& listener) = 0;
    virtual WMError UnregisterLifeCycleListener(const sptr<IWindowLifeCycle>& listener) = 0;
    virtual WMError RegisterWindowChangeListener(const sptr<IWindowChangeListener>& listener) = 0;
    virtual WMError UnregisterWindowChangeListener(const sptr<IWindowChangeListener>& listener) = 0;
    virtual WMError RegisterAvoidAreaChangeListener(const sptr<IAvoidAreaChangedListener>& listener) = 0;
    virtual WMError UnregisterAvoidAreaChangeListener(const sptr<IAvoidAreaChangedListener>& listener) = 0;
    virtual WMError RegisterDragListener(const sptr<IWindowDragListener>& listener) = 0;
    virtual WMError UnregisterDragListener(const sptr<IWindowDragListener>& listener) = 0;
    virtual WMError RegisterDisplayMoveListener(sptr<IDisplayMoveListener>& listener) = 0;
    virtual WMError UnregisterDisplayMoveListener(sptr<IDisplayMoveListener>& listener) = 0;
    virtual void RegisterWindowDestroyedListener(const NotifyNativeWinDestroyFunc& func) = 0;
    virtual void UnregisterWindowDestroyedListener() {}
    virtual WMError RegisterOccupiedAreaChangeListener(const sptr<IOccupiedAreaChangeListener>& listener) = 0;
    virtual WMError UnregisterOccupiedAreaChangeListener(const sptr<IOccupiedAreaChangeListener>& listener) = 0;
    virtual WMError RegisterTouchOutsideListener(const sptr<ITouchOutsideListener>& listener) = 0;
    virtual WMError UnregisterTouchOutsideListener(const sptr<ITouchOutsideListener>& listener) = 0;
    virtual WMError RegisterAnimationTransitionController(const sptr<IAnimationTransitionController>& listener) = 0;
    virtual WMError RegisterScreenshotListener(const sptr<IScreenshotListener>& listener) = 0;
    virtual WMError UnregisterScreenshotListener(const sptr<IScreenshotListener>& listener) = 0;
    virtual WMError RegisterScreenshotAppEventListener(
        const sptr<IScreenshotAppEventListener>& listener) { return WMError::WM_ERROR_DEVICE_NOT_SUPPORT; };
    virtual WMError UnregisterScreenshotAppEventListener(
        const sptr<IScreenshotAppEventListener>& listener) { return WMError::WM_ERROR_DEVICE_NOT_SUPPORT; };
    virtual WMError RegisterDialogTargetTouchListener(const sptr<IDialogTargetTouchListener>& listener) = 0;
    virtual WMError UnregisterDialogTargetTouchListener(const sptr<IDialogTargetTouchListener>& listener) = 0;
    virtual void RegisterDialogDeathRecipientListener(const sptr<IDialogDeathRecipientListener>& listener) = 0;
    virtual void UnregisterDialogDeathRecipientListener(const sptr<IDialogDeathRecipientListener>& listener) = 0;
    virtual WMError RegisterSystemBarEnableListener(const sptr<IWindowSystemBarEnableListener>& listener) = 0;
    virtual WMError UnRegisterSystemBarEnableListener(const sptr<IWindowSystemBarEnableListener>& listener) = 0;
    virtual WMError RegisterIgnoreViewSafeAreaListener(const sptr<IIgnoreViewSafeAreaListener>& listener) = 0;
    virtual WMError UnRegisterIgnoreViewSafeAreaListener(const sptr<IIgnoreViewSafeAreaListener>& listener) = 0;
    virtual void NotifyTouchDialogTarget(int32_t posX = 0, int32_t posY = 0) = 0;
    virtual void SetAceAbilityHandler(const sptr<IAceAbilityHandler>& handler) = 0;
    virtual WMError NapiSetUIContent(const std::string& contentInfo, napi_env env, napi_value storage,
        BackupAndRestoreType type = BackupAndRestoreType::NONE, sptr<IRemoteObject> token = nullptr,
        AppExecFwk::Ability* ability = nullptr) = 0;
    virtual WMError NapiSetUIContentByName(const std::string& contentName, napi_env env, napi_value storage,
        BackupAndRestoreType type = BackupAndRestoreType::NONE, sptr<IRemoteObject> token = nullptr,
        AppExecFwk::Ability* ability = nullptr) { return WMError::WM_OK; }
    virtual WMError SetUIContentByName(const std::string& contentInfo, napi_env env, napi_value storage,
        AppExecFwk::Ability* ability = nullptr)
    {
        return WMError::WM_OK;
    }
    virtual WMError SetUIContentByAbc(const std::string& abcPath, napi_env env, napi_value storage,
        AppExecFwk::Ability* ability = nullptr)
    {
        return WMError::WM_OK;
    }
    virtual std::string GetContentInfo(BackupAndRestoreType type = BackupAndRestoreType::CONTINUATION) = 0;
    virtual Ace::UIContent* GetUIContent() const = 0;
    virtual void OnNewWant(const AAFwk::Want& want) = 0;
    virtual void SetRequestedOrientation(Orientation) = 0;
    virtual void NotifyPreferredOrientationChange(Orientation orientation) = 0;
    virtual void SetUserRequestedOrientation(Orientation orientation) = 0;
    virtual Orientation GetRequestedOrientation() = 0;
    virtual WMError ConvertOrientationAndRotation(const RotationInfoType from, const RotationInfoType to,
        const int32_t value, int32_t& convertedValue)
    {
        return WMError::WM_ERROR_DEVICE_NOT_SUPPORT;
    }
    virtual void SetRequestWindowModeSupportType(uint32_t windowModeSupportType) = 0;
    virtual uint32_t GetRequestWindowModeSupportType() const = 0;
    virtual WMError SetTouchHotAreas(const std::vector<Rect>& rects) = 0;
    virtual void GetRequestedTouchHotAreas(std::vector<Rect>& rects) const = 0;
    virtual bool IsMainHandlerAvailable() const = 0;
    virtual WMError SetAPPWindowLabel(const std::string& label) = 0;
    virtual WMError SetAPPWindowIcon(const std::shared_ptr<Media::PixelMap>& icon) = 0;
    virtual WMError DisableAppWindowDecor() = 0;
    virtual WMError Minimize() = 0;
    virtual WMError Maximize() = 0;
    virtual WMError Recover() = 0;
    virtual WMError Restore() { return WMError::WM_ERROR_DEVICE_NOT_SUPPORT; }
    virtual WMError RestoreMainWindow(const std::shared_ptr<AAFwk::WantParams>& wantParams)
    {
        return WMError::WM_ERROR_DEVICE_NOT_SUPPORT;
    }
    virtual WMError SetWindowRectAutoSave(bool enabled,
        bool isSaveBySpecifiedFlag = false) { return WMError::WM_ERROR_DEVICE_NOT_SUPPORT; }
    virtual WMError SetSupportedWindowModes(const std::vector<AppExecFwk::SupportWindowMode>& supportedWindowModes,
        bool grayOutMaximizeButton = false)
    {
        return WMError::WM_ERROR_DEVICE_NOT_SUPPORT;
    }
    virtual WMError SetImageForRecent(uint32_t imgResourceId, OHOS::Rosen::ImageFit ImageFit)
    {
        return WMError::WM_ERROR_DEVICE_NOT_SUPPORT;
    }
    virtual WMError SetImageForRecentPixelMap(const std::shared_ptr<Media::PixelMap>& pixelMap,
        OHOS::Rosen::ImageFit ImageFit)
    {
        return WMError::WM_ERROR_DEVICE_NOT_SUPPORT;
    }
    virtual WMError RemoveImageForRecent()
    {
        return WMError::WM_ERROR_DEVICE_NOT_SUPPORT;
    }
    virtual WMError SetFollowParentMultiScreenPolicy(bool enabled) { return WMError::WM_ERROR_DEVICE_NOT_SUPPORT; }
    virtual void StartMove() = 0;
    virtual WmErrorCode StartMoveWindow() { return WmErrorCode::WM_OK; }
    virtual WmErrorCode StartMoveWindowWithCoordinate(int32_t offsetX,
        int32_t offsetY) { return WmErrorCode::WM_OK; }
    virtual WmErrorCode StopMoveWindow() { return WmErrorCode::WM_OK; }
    virtual WMError Close() = 0;
    virtual void SetNeedRemoveWindowInputChannel(bool needRemoveWindowInputChannel) = 0;
    virtual bool IsSupportWideGamut() = 0;
    virtual void SetColorSpace(ColorSpace colorSpace) = 0;
    virtual ColorSpace GetColorSpace() = 0;
    virtual void DumpInfo(const std::vector<std::string>& params, std::vector<std::string>& info) = 0;
    virtual std::shared_ptr<Media::PixelMap> Snapshot() = 0;
    virtual WMError Snapshot(
        std::shared_ptr<Media::PixelMap>& pixelMap) { return WMError::WM_ERROR_DEVICE_NOT_SUPPORT; }
    virtual WMError SnapshotIgnorePrivacy(std::shared_ptr<Media::PixelMap>& pixelMap) = 0;
    virtual WMError NotifyMemoryLevel(int32_t level) = 0;
    virtual bool IsAllowHaveSystemSubWindow() = 0;
    virtual WMError SetAspectRatio(float ratio) = 0;

    /**
     * @brief Set content aspect ratio of the window.
     *
     * @param ratio The aspect ratio of window content (width divided by height).
     * @param isPersistent Whether to persist the aspect ratio setting.
     * @param needUpdateRect Whether to update the window rect after setting aspect ratio.
     * @return WMError::WM_OK on success, or appropriate error code on failure.
     */
    virtual WMError SetContentAspectRatio(
        float ratio, bool isPersistent, bool needUpdateRect) { return WMError::WM_ERROR_DEVICE_NOT_SUPPORT; }

    virtual WMError ResetAspectRatio() = 0;
    virtual KeyboardAnimationConfig GetKeyboardAnimationConfig() = 0;
    virtual void SetNeedDefaultAnimation(bool needDefaultAnimation) = 0;

    virtual void SetViewportConfig(const Ace::ViewportConfig& config) = 0;
    virtual void UpdateViewportConfig() = 0;
    virtual void SetOrientation(Orientation orientation) = 0;
    virtual void SetSize(int32_t width, int32_t height) = 0;
    virtual void SetDensity(float density) = 0;
    virtual WMError SetDefaultDensityEnabled(bool enabled) { return WMError::WM_ERROR_DEVICE_NOT_SUPPORT; }
    virtual bool GetDefaultDensityEnabled() { return false; }
    virtual WMError SetCustomDensity(
        float density, bool applyToSubWindow) { return WMError::WM_ERROR_DEVICE_NOT_SUPPORT; }
    virtual float GetCustomDensity() const { return UNDEFINED_DENSITY; }
    virtual WMError SetWindowShadowEnabled(bool isEnabled) { return WMError::WM_ERROR_DEVICE_NOT_SUPPORT; }
    virtual bool GetWindowShadowEnabled() const { return true; }
    virtual WMError SetWindowDefaultDensityEnabled(bool enabled) { return WMError::WM_ERROR_DEVICE_NOT_SUPPORT; }
    virtual WMError GetWindowDensityInfo(
        WindowDensityInfo& densityInfo) { return WMError::WM_ERROR_DEVICE_NOT_SUPPORT; }
    virtual WMError IsMainWindowFullScreenAcrossDisplays(
        bool& isAcrossDisplays) { return WMError::WM_ERROR_DEVICE_NOT_SUPPORT; }
    virtual float GetVirtualPixelRatio() { return 1.0f; }
    virtual void UpdateAvoidArea(const sptr<AvoidArea>& avoidArea, AvoidAreaType type);
    virtual void CreateSurfaceNode(const std::string name, const SendRenderDataCallback& callback) = 0;
    virtual void SetContentInfoCallback(const ContentInfoCallback& callback) = 0;
    virtual WMError SetResizeByDragEnabled(bool dragEnabled) = 0;
    virtual WMError SetRaiseByClickEnabled(bool raiseEnabled) = 0;
    virtual WMError SetMainWindowRaiseByClickEnabled(bool raiseEnabled) { return WMError::WM_ERROR_DEVICE_NOT_SUPPORT; }
    virtual WMError RaiseAboveTarget(int32_t subWindowId) = 0;
    virtual WMError SetTopmost(bool topmost) { return WMError::WM_OK; }
    virtual bool IsTopmost() const { return false; }
    virtual WMError SetMainWindowTopmost(bool isTopmost) { return WMError::WM_OK; }
    virtual bool IsMainWindowTopmost() const { return false; }
    virtual WMError HideNonSystemFloatingWindows(bool shouldHide) = 0;
    virtual bool IsFloatingWindowAppType() const { return false; }
    virtual bool IsPcWindow() const { return false; }
    virtual bool IsPadWindow() const { return false; }
    virtual bool IsPcOrFreeMultiWindowCapabilityEnabled() const { return false; }
    virtual bool IsPhonePadOrPcWindow() const { return false; }
    virtual bool IsPcOrPadFreeMultiWindowMode() const { return false; }
    virtual bool IsPadAndNotFreeMultiWindowCompatibleMode() const { return false; }
    virtual bool IsSceneBoardEnabled() const { return false; }
    virtual bool GetCompatibleModeInPc() const { return false; }
    virtual WmErrorCode KeepKeyboardOnFocus(bool keepKeyboardFlag) = 0;
    virtual WMError RegisterWindowVisibilityChangeListener(const WindowVisibilityListenerSptr& listener) = 0;
    virtual WMError UnregisterWindowVisibilityChangeListener(const WindowVisibilityListenerSptr& listener) = 0;
    virtual WMError SetWindowLimits(WindowLimits& windowLimits, bool isForcible = false) { return WMError::WM_OK; }
    virtual WMError GetWindowLimits(WindowLimits& windowLimits, bool getVirtualPixel = false) { return WMError::WM_OK; }
    virtual WMError EnableDrag(bool enableDrag) { return WMError::WM_ERROR_DEVICE_NOT_SUPPORT; }
    virtual WMError RegisterWindowNoInteractionListener(const IWindowNoInteractionListenerSptr& listener)
    {
        return WMError::WM_ERROR_DEVICE_NOT_SUPPORT;
    }
    virtual WMError UnregisterWindowNoInteractionListener(const IWindowNoInteractionListenerSptr& listener)
    {
        return WMError::WM_ERROR_DEVICE_NOT_SUPPORT;
    }
    virtual WMError SetSingleFrameComposerEnabled(bool enable) = 0;
    virtual WMError SetLandscapeMultiWindow(bool isLandscapeMultiWindow) = 0;
    virtual WMError SetDecorVisible(bool isVisible) { return WMError::WM_ERROR_DEVICE_NOT_SUPPORT; }
    virtual WMError GetDecorVisible(bool& isVisible) { return WMError::WM_ERROR_DEVICE_NOT_SUPPORT; }
    virtual WMError SetWindowTitleMoveEnabled(bool enable) { return WMError::WM_ERROR_DEVICE_NOT_SUPPORT; }
    virtual WMError SetWindowTitle(const std::string& title) { return WMError::WM_ERROR_DEVICE_NOT_SUPPORT; }
    virtual WMError SetTitleButtonVisible(bool isMaximizeVisible, bool isMinimizeVisible, bool isSplitVisible,
        bool isCloseVisible)
    {
        return WMError::WM_ERROR_DEVICE_NOT_SUPPORT;
    }
    virtual WMError SetDecorHeight(int32_t decorHeight) { return WMError::WM_ERROR_DEVICE_NOT_SUPPORT; }
    virtual WMError GetDecorHeight(int32_t& height) { return WMError::WM_ERROR_DEVICE_NOT_SUPPORT; }
    virtual WMError SetDecorButtonStyle(const DecorButtonStyle& style)
    {
        return WMError::WM_ERROR_DEVICE_NOT_SUPPORT;
    }
    virtual WMError GetDecorButtonStyle(DecorButtonStyle& style) { return WMError::WM_ERROR_DEVICE_NOT_SUPPORT; }
    virtual WMError GetTitleButtonArea(TitleButtonRect& titleButtonRect)
    {
        return WMError::WM_ERROR_DEVICE_NOT_SUPPORT;
    }
    virtual WMError SetWindowContainerColor(const std::string& activeColor, const std::string& inactiveColor)
    {
        return WMError::WM_ERROR_DEVICE_NOT_SUPPORT;
    }
    virtual WMError SetWindowContainerModalColor(const std::string& activeColor, const std::string& inactiveColor)
    {
        return WMError::WM_ERROR_DEVICE_NOT_SUPPORT;
    }
    virtual WMError RegisterWindowTitleButtonRectChangeListener(
        const sptr<IWindowTitleButtonRectChangedListener>& listener)
    {
        return WMError::WM_ERROR_DEVICE_NOT_SUPPORT;
    }
    virtual WMError UnregisterWindowTitleButtonRectChangeListener(
        const sptr<IWindowTitleButtonRectChangedListener>& listener)
    {
        return WMError::WM_ERROR_DEVICE_NOT_SUPPORT;
    }

    /**
     * @brief Set the application modality of main window.
     *
     * @param isModal bool.
     * @return WMError
     */
    virtual WMError SetWindowModal(bool isModal) { return WMError::WM_ERROR_DEVICE_NOT_SUPPORT; }

    /**
     * @brief Set the modality of sub window.
     *
     * @param isModal bool.
     * @param modalityType ModalityType.
     * @return WMError
     */
    virtual WMError SetSubWindowModal(bool isModal, ModalityType modalityType = ModalityType::WINDOW_MODALITY)
    {
        return WMError::WM_ERROR_DEVICE_NOT_SUPPORT;
    }

    /**
     * @brief Set sub window zLevel
     *
     * @param zLevel zLevel of sub window to specify the hierarchical relationship among sub windows
     * @return WM_OK means success, others mean set failed
     */
    virtual WMError SetSubWindowZLevel(int32_t zLevel)
    {
        return WMError::WM_ERROR_DEVICE_NOT_SUPPORT;
    }

    /**
     * @brief Get sub window zLevel
     *
     * @param zLevel sub window zLevel
     * @return WM_OK means success, others mean get failed
     */
    virtual WMError GetSubWindowZLevel(int32_t& zLevel)
    {
        return WMError::WM_ERROR_DEVICE_NOT_SUPPORT;
    }

    virtual WMError Recover(uint32_t reason = 0) { return WMError::WM_ERROR_DEVICE_NOT_SUPPORT; }

    virtual WMError Maximize(MaximizePresentation present) { return WMError::WM_ERROR_DEVICE_NOT_SUPPORT; }

    /**
     * @brief Maximize the window with the specified presentation mode and waterfall resident state.
     *
     * @param presentation The presentation mode used for window layout when maximizing.
     * @param waterfallState The waterfall resident state to apply when maximizing.
     * @return WMError::WM_OK on success, or appropriate error code on failure.
     */
    virtual WMError Maximize(MaximizePresentation presentation, WaterfallResidentState waterfallState)
    {
        return WMError::WM_ERROR_DEVICE_NOT_SUPPORT;
    }

    virtual WMError SetWindowMask(const std::vector<std::vector<uint32_t>>& windowMask)
    {
        return WMError::WM_ERROR_DEVICE_NOT_SUPPORT;
    }

    virtual WMError SetGrayScale(float grayScale) { return WMError::WM_ERROR_DEVICE_NOT_SUPPORT; }

    /**
     * @brief Set the Dvsync object
     *
     * @param dvsyncSwitch bool.
     * @return * void
     */
    virtual void SetUiDvsyncSwitch(bool dvsyncSwitch) {}

    /**
     * @brief Set touch event
     *
     * @param touchType int32_t.
     */
    virtual void SetTouchEvent(int32_t touchType) {}

    virtual WMError SetImmersiveModeEnabledState(bool enable) { return WMError::WM_OK; }

    virtual bool GetImmersiveModeEnabledState() const { return true; }

    virtual WMError IsImmersiveLayout(bool& isImmersiveLayout) const { return WMError::WM_ERROR_DEVICE_NOT_SUPPORT; }

    virtual WMError GetWindowStatus(WindowStatus& windowStatus) { return WMError::WM_ERROR_DEVICE_NOT_SUPPORT; }

    virtual void NotifyExtensionTimeout(int32_t errorCode) {}

    /**
     * @brief Set whether to enable gesture back.
     * @param enable the value true means to enable gesture back, and false means the opposite.
     * @return WM_OK means set success, others means set failed.
     */
    virtual WMError SetGestureBackEnabled(bool enable) { return WMError::WM_OK; }

    /**
     * @brief Get whether to enable gesture back.
     * @param enable the value true means to enable gesture back, and false means the opposite.
     * @return WM_OK means get success, others means get failed.
     */
    virtual WMError GetGestureBackEnabled(bool& enable) const { return WMError::WM_OK; }

    /**
     * @brief Flush layout size.
     *
     * @param width The width after layout
     * @param height The height after layout
     */
    virtual void FlushLayoutSize(int32_t width, int32_t height) {}

    /**
     * @brief Notify window manager to update snapshot.
     */
    virtual WMError NotifySnapshotUpdate() { return WMError::WM_OK; }

    /**
     * @brief notify window remove starting window.
     *
     * @return WMError
     */
    virtual WMError NotifyRemoveStartingWindow() { return WMError::WM_ERROR_DEVICE_NOT_SUPPORT; }

    /**
     * @brief Enable or disable window delay raise
     *
     * @param isEnabled Enable or disable window delay raise
     */
    virtual WMError SetWindowDelayRaiseEnabled(bool isEnabled) { return WMError::WM_ERROR_DEVICE_NOT_SUPPORT; }

    /**
     * @brief Get whether window delay raise is enabled
     *
     * @return True means window delay raise is enabled
     */
    virtual bool IsWindowDelayRaiseEnabled() const { return false; }

    /**
     * @brief Set whether to enable exclusively highlight.
     *
     * @param isExclusivelyHighlighted the value true means to exclusively highlight, and false means the opposite.
     * @return WM_OK means set success, others means set failed.
     */
    virtual WMError SetExclusivelyHighlighted(bool isExclusivelyHighlighted)
    {
        return WMError::WM_ERROR_DEVICE_NOT_SUPPORT;
    }

    /**
     * @brief Check whether current window has specified device feature.
     *
     * @param feature specified device feature
     * @return true means current window has specified device feature, false means not.
     */
    virtual bool IsDeviceFeatureCapableFor(const std::string& feature) const { return false; }

    /**
     * @brief Check whether current window has free-multi-window device feature.
     *
     * @return true means current window has free-multi-window feature, false means not.
     */
    virtual bool IsDeviceFeatureCapableForFreeMultiWindow() const { return false; }

    /**
     * @brief Get highlight property of window.
     *
     * @param highlighted True means the window is highlighted, and false means the opposite.
     * @return WM_OK means get success, others means get failed.
     */
    virtual WMError IsWindowHighlighted(bool& highlighted) const { return WMError::WM_ERROR_DEVICE_NOT_SUPPORT; }

    /**
     * @brief Get the root host window type of UIExtension.
     *
     * @return WindowType of the root host window.
     */
    virtual WindowType GetRootHostWindowType() const { return WindowType::WINDOW_TYPE_APP_MAIN_WINDOW; }

    /**
     * @brief Set the root host window type of UIExtension.
     *
     * @param WindowType of the root host window.
     */
    virtual void SetRootHostWindowType(WindowType& rootHostWindowType) {}

    /**
     * @brief Register SystemBarProperty listener.
     *
     * @param listener ISystemBarPropertyListener.
     * @return WM_OK means register success, others means register failed.
     */
    virtual WMError RegisterSystemBarPropertyListener(const sptr<ISystemBarPropertyListener>& listener)
    {
        return WMError::WM_ERROR_DEVICE_NOT_SUPPORT;
    }

    /**
     * @brief Unregister SystemBarProperty listener.
     *
     * @param listener ISystemBarPropertyListener.
     * @return WM_OK means unregister success, others means unregister failed.
     */
    virtual WMError UnregisterSystemBarPropertyListener(const sptr<ISystemBarPropertyListener>& listener)
    {
        return WMError::WM_ERROR_DEVICE_NOT_SUPPORT;
    }

    /**
     * @brief Notify SystemBarProperty listener.
     *
     * @param type The WindowType.
     * @param property new property value setted by developer.
     */
    virtual void NotifySystemBarPropertyUpdate(WindowType type, const SystemBarProperty& property) {}

    /**
     * @brief Get window state snapshot.
     *
     * @param winStateSnapshotJsonStr The state snapshot of specified window.
     * @return WM_OK means get success, others means get failed.
     */
    virtual WMError GetWindowStateSnapshot(std::string& winStateSnapshotJsonStr)
    {
        return WMError::WM_ERROR_DEVICE_NOT_SUPPORT;
    }

    /**
     * @brief Set the parent window of a sub window.
     *
     * @param newParentWindowId new parent window id.
     * @return WM_OK means set parent window success, others means failed.
     */
    virtual WMError SetParentWindow(int32_t newParentWindowId) { return WMError::WM_ERROR_DEVICE_NOT_SUPPORT; }

    /**
     * @brief Get the parent window of a sub window.
     *
     * @param parentWindow parent window.
     * @return WM_OK means get parent window success, others means failed.
     */
    virtual WMError GetParentWindow(sptr<Window>& parentWindow) { return WMError::WM_ERROR_DEVICE_NOT_SUPPORT; }

    /**
     * @brief Set window anchor info.
     *
     * @param windowAnchorInfo the windowAnchorInfo of subWindow.
     * @return WM_OK means set success.
     */
    virtual WMError SetWindowAnchorInfo(const WindowAnchorInfo& windowAnchorInfo)
    {
        return WMError::WM_ERROR_DEVICE_NOT_SUPPORT;
    }

    /**
     * @brief Set the feature of subwindow follow the layout of the parent window.
     *
     * @param isFollow true - follow, false - not follow.
     * @return WM_OK means set success.
     */
    virtual WMError SetFollowParentWindowLayoutEnabled(bool isFollow) { return WMError::WM_ERROR_SYSTEM_ABNORMALLY; }

    /**
     * @brief Check if the window supports transition animation and has permission to use it.
     *
     * @return WM_OK means transition animation is supported and allowed by the system.
     */
    virtual WMError IsTransitionAnimationSupported() const { return WMError::WM_ERROR_DEVICE_NOT_SUPPORT; }

    /**
     * @brief Set the transition animation.
     *
     * @param transitionType window transition type.
     * @param animation window transition animation.
     * @return WM_OK means set window transition animation success, others means failed.
     */
    virtual WMError SetWindowTransitionAnimation(WindowTransitionType transitionType,
        const TransitionAnimation& animation)
    {
        return WMError::WM_ERROR_DEVICE_NOT_SUPPORT;
    }

    /**
     * @brief Get the transition animation.
     *
     * @param transitionType window transition type.
     * @return nullptr means get failed.
     */
    virtual std::shared_ptr<TransitionAnimation> GetWindowTransitionAnimation(WindowTransitionType transitionType)
    {
        return nullptr;
    }

      /**
     * @brief Get is subwindow support maximize.
     *
     * @return true means subwindow support maximize, others means do not support.
     */
    virtual bool IsSubWindowMaximizeSupported() const { return false; }

    /**
     * @brief Set drag key frame policy.
     * effective order:
     *  1. resize when drag
     *  2. key frame
     *  3. default value
     *
     * @param keyFramePolicy param of key frame
     * @return WM_OK means get success, others means failed.
     */
    virtual WMError SetDragKeyFramePolicy(const KeyFramePolicy& keyFramePolicy)
    {
        return WMError::WM_ERROR_DEVICE_NOT_SUPPORT;
    }

    /**
     * @brief Get the window property of current window.
     *
     * @param windowPropertyInfo the window property struct.
     * @return WMError.
     */
    virtual WMError GetWindowPropertyInfo(WindowPropertyInfo& windowPropertyInfo) { return WMError::WM_OK; }

    /**
     * @brief notify avoid area for compatible mode app
     */
    virtual void HookCompatibleModeAvoidAreaNotify() {}

     /**
     * @brief The comaptible mode app adapt to immersive or not.
     *
     * @return true comptbleMode adapt to immersive, others means not.
     */
    virtual bool IsAdaptToCompatibleImmersive() const { return false; }

    /**
     * @brief Set the source of subwindow.
     *
     * @param source 0 - defalut, 1 - arkui.
     * @return WM_OK means set success.
     */
    virtual WMError SetSubWindowSource(SubWindowSource source) { return WMError::WM_ERROR_DEVICE_NOT_SUPPORT; }

    /**
     * @brief Raise main window above another.
     *
     * @param targetId Indicates the id of the target main window.
     * @return WM_OK means raise success, others means raise failed.
     */
    virtual WMError RaiseMainWindowAboveTarget(int32_t targetId)
    {
        return WMError::WM_ERROR_DEVICE_NOT_SUPPORT;
    }

    /**
     * @brief Set whether this window limits screen rotation when this window is shown.
     *
     * @param locked Screen rotation lock status to set.
     * @return WMError::WM_OK on success, others means failed.
     */
    virtual WMError SetRotationLocked(bool locked) { return WMError::WM_ERROR_DEVICE_NOT_SUPPORT; }
    
    /**
     * @brief Get whether this window limits screen rotation when this window is shown.
     * @param locked Screen rotation lock status to get.
     *
     * @return WMError::WM_OK on success, others means failed.
     */
    virtual WMError GetRotationLocked(bool& locked) { return WMError::WM_ERROR_DEVICE_NOT_SUPPORT; }

    /**
     * @brief Set whether the window receive drag event.
     *
     * @param enabled - whether the window receive drag event.
     *        True: - means default state, the window can receive drag event.
     *        False: - means the window can't receive drag event.
     * @return Returns the status code of the execution.
     */
    virtual WMError SetReceiveDragEventEnabled(bool enabled)
    {
        return WMError::WM_ERROR_DEVICE_NOT_SUPPORT;
    }

    /**
     * @brief  whether the window receive drag event.
     *
     * @return - The value true means the window can receive drag event, and false means the opposite.
     */
    virtual bool IsReceiveDragEventEnabled()
    {
        return true;
    }

    /**
     * @brief Set whether the window supports event separation capability.
     *        When the window doesn't support event separation capability:
     *        After the first finger touch the window,
     *        subsequent fingers' events will be sent to that window regardless of whether they click on it.
     *        If the first finger does not touch the window,
     *        the system will discard the events when subsequent fingers touch the window.
     *
     * @param enabled - Whether the window supports event separation capability.
     *        True: - means default state, the event will be sent to the window that the finger taps.
     *        False: - means the window doesn't support event separation capability.
     * @return - Promise that returns no value.
     */
    virtual WMError SetSeparationTouchEnabled(bool enabled)
    {
        return WMError::WM_ERROR_DEVICE_NOT_SUPPORT;
    }

    /**
     * @brief   Get whether the window supports event separation status.
     *
     * @return - The value true means the window supports event separation, and false means the opposite.
     */
    virtual bool IsSeparationTouchEnabled()
    {
        return true;
    }

    /**
     * @brief Flush vsync for prelaunch.
     */
    virtual void FlushVsync() {}
};
}
}
#endif // OHOS_ROSEN_WINDOW_H
