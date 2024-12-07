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
}

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
}

namespace OHOS {
namespace Rosen {
class RSSurfaceNode;
class RSTransaction;
using NotifyNativeWinDestroyFunc = std::function<void(std::string windowName)>;
using SendRenderDataCallback = bool (*)(const void*, const size_t, const int32_t, const int32_t, const uint64_t);
using ContentInfoCallback = std::function<void(std::string contentInfo)>;

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
class IScreenshotListener : virtual public RefBase {
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
    WMError& errCode = DefaultCreateErrCode);
    static sptr<Window> Find(const std::string& windowName);
    static sptr<Window> GetTopWindowWithContext(const std::shared_ptr<AbilityRuntime::Context>& context = nullptr);
    static sptr<Window> GetTopWindowWithId(uint32_t mainWinId);
    static std::vector<sptr<Window>> GetSubWindow(uint32_t parentId);
    static void UpdateConfigurationForAll(const std::shared_ptr<AppExecFwk::Configuration>& configuration);
    virtual std::shared_ptr<RSSurfaceNode> GetSurfaceNode() const = 0;
    virtual const std::shared_ptr<AbilityRuntime::Context> GetContext() const = 0;
    virtual Rect GetRect() const = 0;
    virtual Rect GetRequestRect() const = 0;
    virtual WindowType GetType() const = 0;
    virtual WindowMode GetMode() const = 0;
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
    virtual WMError SetAlpha(float alpha) = 0;
    virtual WMError SetTransform(const Transform& trans) = 0;
    virtual const Transform& GetTransform() const = 0;
    virtual WMError RemoveWindowFlag(WindowFlag flag) = 0;
    virtual WMError AddWindowFlag(WindowFlag flag) = 0;
    virtual WMError SetWindowFlags(uint32_t flags) = 0;
    virtual WMError GetAvoidAreaByType(AvoidAreaType type, AvoidArea& avoidArea) = 0;
    virtual WMError SetSystemBarProperty(WindowType type, const SystemBarProperty& property) = 0;
    virtual WMError SetSpecificBarProperty(WindowType type, const SystemBarProperty& property) = 0;
    virtual WMError SetSystemBarProperties(const std::map<WindowType, SystemBarProperty>& properties,
        const std::map<WindowType, SystemBarPropertyFlag>& propertyFlags) = 0;
    virtual WMError GetSystemBarProperties(std::map<WindowType, SystemBarProperty>& properties) = 0;
    virtual WMError SetFullScreen(bool status) = 0;
    virtual WMError SetLayoutFullScreen(bool status) = 0;
    virtual WMError Destroy() = 0;
    virtual WMError Show(uint32_t reason = 0, bool withAnimation = false, bool withFocus = true) = 0;
    virtual WMError Hide(uint32_t reason = 0, bool withAnimation = false, bool isFromInnerkits = true) = 0;
    virtual WMError MoveTo(int32_t x, int32_t y, bool isMoveToGlobal = false) = 0;
    virtual WMError MoveToAsync(int32_t x, int32_t y) { return WMError::WM_ERROR_DEVICE_NOT_SUPPORT; }
    virtual WMError MoveWindowToGlobal(int32_t x, int32_t y) { return WMError::WM_OK; }
    virtual WMError GetGlobalScaledRect(Rect& globalScaledRect) { return WMError::WM_ERROR_DEVICE_NOT_SUPPORT; }
    virtual WMError Resize(uint32_t width, uint32_t height) = 0;
    virtual WMError ResizeAsync(uint32_t width, uint32_t height) { return WMError::WM_ERROR_DEVICE_NOT_SUPPORT; }
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
    virtual WMError SetCallingWindow(uint32_t windowId) = 0;
    virtual WMError SetPrivacyMode(bool isPrivacyMode) = 0;
    virtual bool IsPrivacyMode() const = 0;
    virtual void SetSystemPrivacyMode(bool isSystemPrivacyMode) = 0;
    virtual WMError BindDialogTarget(sptr<IRemoteObject> targetToken) = 0;
    virtual WMError SetDialogBackGestureEnabled(bool isEnabled) = 0;
    virtual WMError RaiseToAppTop() = 0;
    virtual WMError SetSnapshotSkip(bool isSkip) = 0;
    virtual WMError SetCornerRadius(float cornerRadius) = 0;
    virtual WMError SetShadowRadius(float radius) = 0;
    virtual WMError SetShadowColor(std::string color) = 0;
    virtual WMError SetShadowOffsetX(float offsetX) = 0;
    virtual WMError SetShadowOffsetY(float offsetY) = 0;
    virtual WMError SetBlur(float radius) = 0;
    virtual WMError SetBackdropBlur(float radius) = 0;
    virtual WMError SetBackdropBlurStyle(WindowBlurStyle blurStyle) = 0;
    virtual WMError RequestFocus() const = 0;
    virtual WMError RequestFocusByClient(bool isFocused) { return WMError::WM_ERROR_DEVICE_NOT_SUPPORT; }
    virtual bool IsFocused() const = 0;
    virtual WMError UpdateSurfaceNodeAfterCustomAnimation(bool isAdd) = 0;
    virtual void SetInputEventConsumer(const std::shared_ptr<IInputEventConsumer>& inputEventConsumer) = 0;
    virtual void ConsumeKeyEvent(const std::shared_ptr<MMI::KeyEvent>& inputEvent) = 0;
    virtual void ConsumePointerEvent(const std::shared_ptr<MMI::PointerEvent>& inputEvent) = 0;
    virtual void RequestVsync(const std::shared_ptr<VsyncCallback>& vsyncCallback) = 0;
    virtual int64_t GetVSyncPeriod() = 0;
    virtual void FlushFrameRate(uint32_t rate, int32_t animatorExpectedFrameRate, uint32_t rateType) {}
    virtual void UpdateConfiguration(const std::shared_ptr<AppExecFwk::Configuration>& configuration) = 0;
    virtual WMError RegisterLifeCycleListener(const sptr<IWindowLifeCycle>& listener) = 0;
    virtual WMError UnregisterLifeCycleListener(const sptr<IWindowLifeCycle>& listener) = 0;
    virtual WMError RegisterWindowChangeListener(const sptr<IWindowChangeListener>& listener) = 0;
    virtual WMError UnregisterWindowChangeListener(const sptr<IWindowChangeListener>& listener) = 0;
    virtual WMError RegisterAvoidAreaChangeListener(sptr<IAvoidAreaChangedListener>& listener) = 0;
    virtual WMError UnregisterAvoidAreaChangeListener(sptr<IAvoidAreaChangedListener>& listener) = 0;
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
    virtual WMError NapiSetUIContent(const std::string& contentInfo, napi_env env,
        napi_value storage, bool isDistributed = false, sptr<IRemoteObject> token = nullptr,
        AppExecFwk::Ability* ability = nullptr) = 0;
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
    virtual std::string GetContentInfo() = 0;
    virtual Ace::UIContent* GetUIContent() const = 0;
    virtual void OnNewWant(const AAFwk::Want& want) = 0;
    virtual void SetRequestedOrientation(Orientation) = 0;
    virtual Orientation GetRequestedOrientation() = 0;
    virtual void SetRequestModeSupportInfo(uint32_t modeSupportInfo) = 0;
    virtual uint32_t GetRequestModeSupportInfo() const = 0;
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
    virtual void StartMove() = 0;
    virtual WMError Close() = 0;
    virtual void SetNeedRemoveWindowInputChannel(bool needRemoveWindowInputChannel) = 0;
    virtual bool IsSupportWideGamut() = 0;
    virtual void SetColorSpace(ColorSpace colorSpace) = 0;
    virtual ColorSpace GetColorSpace() = 0;
    virtual void DumpInfo(const std::vector<std::string>& params, std::vector<std::string>& info) = 0;
    virtual std::shared_ptr<Media::PixelMap> Snapshot() = 0;
    virtual WMError NotifyMemoryLevel(int32_t level) = 0;
    virtual bool IsAllowHaveSystemSubWindow() = 0;
    virtual WMError SetAspectRatio(float ratio) = 0;
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
    virtual float GetVirtualPixelRatio() { return 1.0f; }
    virtual void UpdateAvoidArea(const sptr<AvoidArea>& avoidArea, AvoidAreaType type);
    virtual void CreateSurfaceNode(const std::string name, const SendRenderDataCallback& callback) = 0;
    virtual void SetContentInfoCallback(const ContentInfoCallback& callback) = 0;
    virtual WMError SetResizeByDragEnabled(bool dragEnabled) = 0;
    virtual WMError SetRaiseByClickEnabled(bool raiseEnabled) = 0;
    virtual WMError RaiseAboveTarget(int32_t subWindowId) = 0;
    virtual WMError SetTopmost(bool topmost) { return WMError::WM_OK; }
    virtual bool IsTopmost() const { return false; }
    virtual WMError HideNonSystemFloatingWindows(bool shouldHide) = 0;
    virtual bool IsFloatingWindowAppType() const { return false; }
    virtual bool IsPcOrPadCapabilityEnabled() const { return false; }
    virtual WmErrorCode KeepKeyboardOnFocus(bool keepKeyboardFlag) = 0;
    virtual WMError RegisterWindowVisibilityChangeListener(const WindowVisibilityListenerSptr& listener) = 0;
    virtual WMError UnregisterWindowVisibilityChangeListener(const WindowVisibilityListenerSptr& listener) = 0;
    virtual WMError SetWindowLimits(WindowLimits& windowLimits) { return WMError::WM_OK; };
    virtual WMError GetWindowLimits(WindowLimits& windowLimits) { return WMError::WM_OK; };
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
    virtual WMError SetTitleButtonVisible(bool isMaximizeVisible, bool isMinimizeVisible, bool isSplitVisible)
    {
        return WMError::WM_ERROR_DEVICE_NOT_SUPPORT;
    }
    virtual WMError SetDecorHeight(int32_t decorHeight) { return WMError::WM_ERROR_DEVICE_NOT_SUPPORT; }
    virtual WMError GetDecorHeight(int32_t& height) { return WMError::WM_ERROR_DEVICE_NOT_SUPPORT; }
    virtual WMError GetTitleButtonArea(TitleButtonRect& titleButtonRect)
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
     * @brief Set the modality of window.
     *
     * @param isModal bool.
     * @return WMError
     */
    virtual WMError SetSubWindowModal(bool isModal)
    {
        return WMError::WM_ERROR_DEVICE_NOT_SUPPORT;
    }
    virtual WMError Recover(uint32_t reason = 0) { return WMError::WM_ERROR_DEVICE_NOT_SUPPORT; };

    virtual WMError Maximize(MaximizePresentation present) { return WMError::WM_ERROR_DEVICE_NOT_SUPPORT; }

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

    virtual WMError SetImmersiveModeEnabledState(bool enable) { return WMError::WM_OK; }

    virtual bool GetImmersiveModeEnabledState() const { return true; }

    virtual WMError GetWindowStatus(WindowStatus& windowStatus) { return WMError::WM_ERROR_DEVICE_NOT_SUPPORT; }

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
    virtual WMError GetGestureBackEnabled(bool& enable) { return WMError::WM_OK; }
};
}
}
#endif // OHOS_ROSEN_WINDOW_H
