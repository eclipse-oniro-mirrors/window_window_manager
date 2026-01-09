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

#ifndef OHOS_ROSEN_WINDOW_H
#define OHOS_ROSEN_WINDOW_H

#include <refbase.h>
#include <parcel.h>
#include <iremote_object.h>

#include "dm_common.h"
#include "wm_common.h"
#include "window_option.h"
#include "occupied_area_change_info.h"
#include "data_handler_interface.h"
#include "floating_ball_template_base_info.h"

typedef struct napi_env__* napi_env;
typedef struct napi_value__* napi_value;
typedef class __ani_object* ani_object;
#ifdef __cplusplus
typedef struct __ani_env ani_env;
#else
typedef const struct __ani_interaction_api *ani_env;
#endif

namespace OHOS::MMI {
class PointerEvent;
class KeyEvent;
class AxisEvent;
}
namespace OHOS::AppExecFwk {
class Configuration;
class Ability;
class ElementName;
enum class SupportWindowMode;
}

namespace OHOS::Global::Resource {
class ResourceManager;
} // namespace OHOS::Global::Resource

namespace OHOS::AbilityRuntime {
class AbilityContext;
class Context;
}

namespace OHOS::AAFwk {
class Want;
class WantParams;
}

namespace OHOS::Ace {
class UIContent;
class ViewportConfig;
}

namespace OHOS::Rosen {
struct ViewportConfigAndAvoidArea {
    std::shared_ptr<Ace::ViewportConfig> config;
    std::map<AvoidAreaType, AvoidArea> avoidAreas;
};
}

namespace OHOS::Media {
class PixelMap;
}

namespace OHOS::Accessibility {
class AccessibilityEventInfo;
}
namespace OHOS {
namespace Rosen {
using NotifyNativeWinDestroyFunc = std::function<void(std::string windowName)>;
using NotifyTransferComponentDataFunc = std::function<void(const AAFwk::WantParams& wantParams)>;
using NotifyTransferComponentDataForResultFunc = std::function<AAFwk::WantParams(const AAFwk::WantParams& wantParams)>;
using KeyEventFilterFunc = std::function<bool(const MMI::KeyEvent&)>;
using MouseEventFilterFunc = std::function<bool(const MMI::PointerEvent&)>;
using TouchEventFilterFunc = std::function<bool(const MMI::PointerEvent&)>;
class RSSurfaceNode;
class RSTransaction;
class RSUIContext;
class RSUIDirector;
class ISession;
class Window;
enum class ImageFit;

/**
 * @class IWindowLifeCycle
 *
 * @brief IWindowLifeCycle is a listener used to notify caller that lifecycle of window.
 */
class IWindowLifeCycle : virtual public RefBase {
public:
    /**
     * @brief Notify caller that window is on the forground.
     */
    virtual void AfterForeground() {}

    /**
     * @brief Notify caller that window is on the background.
     */
    virtual void AfterBackground() {}

    /**
     * @brief Notify caller that window is focused.
     */
    virtual void AfterFocused() {}

    /**
     * @brief Notify caller that window is unfocused.
     */
    virtual void AfterUnfocused() {}

    /**
     * @brief Notify caller the error code when window go forground failed.
     *
     * @param ret Error code when window go forground failed.
     */
    virtual void ForegroundFailed(int32_t ret) {}

    /**
     * @brief Notify caller the error code when window go background failed.
     *
     * @param ret Error code when window go background failed.
     */
    virtual void BackgroundFailed(int32_t ret) {}

    /**
     * @brief Notify caller that window is active.
     */
    virtual void AfterActive() {}

    /**
     * @brief Notify caller that window is inactive.
     */
    virtual void AfterInactive() {}

    /**
     * @brief Notify caller that window is resumed.
     */
    virtual void AfterResumed() {}

    /**
     * @brief Notify caller that window is paused.
     */
    virtual void AfterPaused() {}

    /**
     * @brief Notify caller that window is destroyed.
     */
    virtual void AfterDestroyed() {}

    /**
     * @brief Notify caller that window is already foreground.
     */
    virtual void AfterDidForeground() {}

    /**
     * @brief Notify caller that window is already background.
     */
    virtual void AfterDidBackground() {}
};

/**
 * @class IWindowStageLifeCycle
 *
 * @brief IWindowStageLifeCycle is a listener used to notify caller that lifecycle of window.
 */
class IWindowStageLifeCycle : virtual public RefBase {
public:
    /**
     * @brief Notify caller that window is on the forground.
     */
    virtual void AfterLifecycleForeground() {}

    /**
     * @brief Notify caller that window is on the background.
     */
    virtual void AfterLifecycleBackground() {}

    /**
     * @brief Notify caller that window is resumed.
     */
    virtual void AfterLifecycleResumed() {}

    /**
     * @brief Notify caller that window is paused.
     */
    virtual void AfterLifecyclePaused() {}
};

/**
 * @class IWindowAttachStateChangeListner
 *
 * @brief IWindowAttachStateChangeListner is used to observe the window attach or detach state changed.
 */
class IWindowAttachStateChangeListner : virtual public RefBase {
public:
    virtual void AfterAttached() {}
    virtual void AfterDetached() {}
};

/**
 * @class IWindowTitleChangeListener
 *
 * @brief IWindowTitleChangeListener is used to observe the title appear or disappear.
 */
class IWindowTitleChangeListener : virtual public RefBase {
public:
    virtual void OnTitleVisibilityChange(Rect& titleRect, bool visibility) {}
};

/**
 * @class IWindowTitleOrHotAreasListener
 *
 * @brief IWindowTitleOrHotAreasListener is used to observe the title appear or disappear.
 */
class IWindowTitleOrHotAreasListener : virtual public RefBase {
public:
    virtual void OnTitleOrHotAreasChange(std::vector<Rect>& hotAreas, bool visibility) {}
};

/**
 * @class IWindowChangeListener
 *
 * @brief IWindowChangeListener is used to observe the window size or window mode when window changed.
 */
class IWindowChangeListener : virtual public RefBase {
public:
    /**
     * @brief Notify caller when window size changed.
     *
     * @param Rect Rect of the current window.
     * @param reason Reason for window change.
     * @param rsTransaction Synchronization transaction for animation
     */
    virtual void OnSizeChange(Rect rect, WindowSizeChangeReason reason,
        const std::shared_ptr<RSTransaction>& rsTransaction = nullptr) {}

    /**
     * @brief Notify caller when window mode changed.
     *
     * @param mode Mode of the current window.
     * @param hasDeco Window has decoration or not.
     */
    virtual void OnModeChange(WindowMode mode, bool hasDeco = true) {}
};

class IWindowCrossAxisListener : virtual public RefBase {
public:
    /**
     * @brief Notify caller when window cross screen axis state changed.
     *
     * @param state is window across screen axis.
     */
    virtual void OnCrossAxisChange(CrossAxisState state) {}
};

/**
 * @class IWindowStatusChangeListener
 *
 * @brief IWindowStatusChangeListener is used to observe the window status when window status changed.
 */
class IWindowStatusChangeListener : virtual public RefBase {
public:
    /**
     * @brief Notify caller when window status changed.
     *
     * @param status Mode of the current window.
     */
    virtual void OnWindowStatusChange(WindowStatus status) {}
};

/**
 * @class IWindowStatusDidChangeListener
 *
 * @brief IWindowStatusDidChangeListener is used to observe the window status when window status changed
 *        and layout finished.
 */
class IWindowStatusDidChangeListener : virtual public RefBase {
public:
    /**
     * @brief Notify caller when window status changed.
     *
     * @param status Mode of the current window.
     */
    virtual void OnWindowStatusDidChange(WindowStatus status) {}
};

/**
 * @class IAvoidAreaChangedListener
 *
 * @brief IAvoidAreaChangedListener is used to observe the avoid area when avoid area size changed.
 */
class IAvoidAreaChangedListener : virtual public RefBase {
public:
    /**
     * @brief Notify caller when avoid area size changed.
     *
     * @param avoidArea Area needed to be avoided.
     * @param type Type of avoid area.
     * @param info Keyboard occupied area information.
     */
    virtual void OnAvoidAreaChanged(const AvoidArea avoidArea, AvoidAreaType type,
        const sptr<OccupiedAreaChangeInfo>& info = nullptr) {}
};

/**
 * @class IWindowDragListener
 *
 * @brief IWindowDragListener is used to observe the drag status when drag window.
 */
class IWindowDragListener : virtual public RefBase {
public:
    /**
     * @brief Notify caller when drag window.
     *
     * @param x X-axis when drag window.
     * @param y Y-axis when drag window.
     * @param event Drag type.
     */
    virtual void OnDrag(int32_t x, int32_t y, DragEvent event) {}
};

/**
 * @class IDisplayMoveListener
 *
 * @brief IDisplayMoveListener is used to observe display move status when display move.
 */
class IDisplayMoveListener : virtual public RefBase {
public:
    /**
     * @brief Notify caller when display move.
     *
     * @param from Display id before display start move.
     * @param to Display id after display move end.
     */
    virtual void OnDisplayMove(DisplayId from, DisplayId to) {}
};

/**
 * @class IDispatchInputEventListener
 *
 * @brief IDispatchInputEventListener is used to dispatch input event.
 */
class IDispatchInputEventListener : virtual public RefBase {
public:
    /**
     * @brief Dispatch PointerEvent.
     *
     * @param inputEvent Means PointerEvent.
     */
    virtual void OnDispatchPointerEvent(std::shared_ptr<MMI::PointerEvent>& inputEvent) {}

    /**
     * @brief Dispatch KeyEvent.
     *
     * @param inputEvent Means KeyEvent.
     */
    virtual void OnDispatchKeyEvent(std::shared_ptr<MMI::KeyEvent>& keyEvent) {}
};

/**
 * @class IOccupiedAreaChangeListener
 *
 * @brief IOccupiedAreaChangeListener is used to observe OccupiedArea change.
 */
class IOccupiedAreaChangeListener : virtual public RefBase {
public:
    /**
     * @brief Notify caller when OccupiedArea size change.
     *
     * @param info Occupied area info when occupied changed.
     * @param rsTransaction Animation transaction.
     */
    virtual void OnSizeChange(const sptr<OccupiedAreaChangeInfo>& info,
        const std::shared_ptr<RSTransaction>& rsTransaction = nullptr) {}
};

/**
 * @class IAceAbilityHandler
 *
 * @brief IAceAbilityHandler is used to control Ace Ability.
 */
class IAceAbilityHandler : virtual public RefBase {
public:
    /**
     * @brief Set BackgroundColor
     *
     * @param color Color of Background.
     */
    virtual void SetBackgroundColor(uint32_t color) {}

    /**
     * @brief Get BackgroundColor.
     *
     * @return Value of BackgroundColor and default color is white.
     */
    virtual uint32_t GetBackgroundColor() { return 0xffffffff; }
};

/**
 * @class IInputEventConsumer
 *
 * @brief IInputEventConsumer is a Listener to observe InputEvent consumed or not.
 */
class IInputEventConsumer {
public:
    /**
     * @brief Default construct func of IInputEventConsumer.
     */
    IInputEventConsumer() = default;

    /**
     * @brief Default Destructor func of IInputEventConsumer.
     */
    virtual ~IInputEventConsumer() = default;

    /**
     * @brief Observe KeyEvent of Multi-Model Input.
     *
     * @param keyEvent KeyEvent of Multi-Model Input.
     */
    virtual bool OnInputEvent(const std::shared_ptr<MMI::KeyEvent>& keyEvent) const { return false; }

    /**
     * @brief Observe PointerEvent of Multi-Model Input.
     *
     * @param pointerEvent PointerEvent of Multi-Model Input.
     */
    virtual bool OnInputEvent(const std::shared_ptr<MMI::PointerEvent>& pointerEvent) const { return false; }

    /**
     * @brief Observe axisEvent of Multi-Model Input.
     *
     * @param axisEvent AxisEvent of Multi-Model Input.
     */
    virtual bool OnInputEvent(const std::shared_ptr<MMI::AxisEvent>& axisEvent) const { return false; }
};

/**
 * @class ITouchOutsideListener
 *
 * @brief ITouchOutsideListener is a Listener to observe event when touch outside the window.
 */
class ITouchOutsideListener : virtual public RefBase {
public:
    /**
     * @brief Observe the event when touch outside the window.
     */
    virtual void OnTouchOutside() const {}
};

/**
 * @class IAnimationTransitionController
 *
 * @brief IAnimationTransitionController is a Listener to observe event about animation.
 */
class IAnimationTransitionController : virtual public RefBase {
public:
    /**
     * @brief Observe the event when animation show.
     */
    virtual void AnimationForShown() {}

    /**
     * @brief Observe the event when animation hide.
     */
    virtual void AnimationForHidden() {}
};

/**
 * @class IScreenshotListener
 *
 * @brief IScreenshotListener is a Listener to observe event when screenshot happened.
 */
class IScreenshotListener : virtual public RefBase {
public:
    /**
     * @brief Observe event when screenshot happened.
     */
    virtual void OnScreenshot() {}
};

/**
 * @class IScreenshotAppEventListener
 *
 * @brief IScreenshotAppEventListener is a Listener to observe event when screenshot happened.
 */
class IScreenshotAppEventListener : virtual public RefBase {
public:
    /**
     * @brief Observe event when screenshot happened.
     */
    virtual void OnScreenshotAppEvent(ScreenshotEventType type) {}
};
using IScreenshotAppEventListenerSptr = sptr<IScreenshotAppEventListener>;

/**
 * @class IDialogTargetTouchListener
 *
 * @brief IDialogTargetTouchListener is a Listener to observe event when touch dialog window.
 */
class IDialogTargetTouchListener : virtual public RefBase {
public:
    /**
     * @brief Observe event when touch dialog window.
     */
    virtual void OnDialogTargetTouch() const {}
};

/**
 * @class IDialogDeathRecipientListener
 *
 * @brief IDialogDeathRecipientListener is a Listener to observe event when mainwindow(bind to dialog) destroyed.
 */
class IDialogDeathRecipientListener : virtual public RefBase {
public:
    /**
     * @brief Observe event when mainwindow(bind to dialog) destroyed.
     */
    virtual void OnDialogDeathRecipient() const {}
};

/**
 * @class IWindowVisibilityChangedListener
 *
 * @brief Listener to observe one window visibility changed.
 */
class IWindowVisibilityChangedListener : virtual public RefBase {
public:
    virtual void OnWindowVisibilityChangedCallback(const bool isVisible) {};
};
using IWindowVisibilityListenerSptr = sptr<IWindowVisibilityChangedListener>;

/**
 * @class IOcclusionStateChangedListener
 *
 * @brief Listener to observe the window occlusion state changed.
 */
class IOcclusionStateChangedListener : virtual public RefBase {
public:
    virtual void OnOcclusionStateChanged(const WindowVisibilityState state) {}
};

/**
 * @class IFrameMetricsChangedListener
 *
 * @brief Listener to observe the window frame metrics changed.
 */
class IFrameMetricsChangedListener : virtual public RefBase {
public:
    virtual void OnFrameMetricsChanged(const FrameMetrics& metrics) {}
};

/**
 * @class IDisplayIdChangeListener
 *
 * @brief Listener to observe one window displayId changed.
 */
class IDisplayIdChangeListener : virtual public RefBase {
public:
    /**
     * @brief Notify caller when window displayId changed.
     */
    virtual void OnDisplayIdChanged(DisplayId displayId) {}
};
using IDisplayIdChangeListenerSptr = sptr<IDisplayIdChangeListener>;

/**
 * @class ISystemDensityChangeListener
 *
 * @brief Listener to observe system density associated with the window changed.
 */
class ISystemDensityChangeListener : virtual public RefBase {
public:
    /**
     * @brief Notify caller when system density changed.
     */
    virtual void OnSystemDensityChanged(float density) {}
};
using ISystemDensityChangeListenerSptr = sptr<ISystemDensityChangeListener>;

class IAcrossDisplaysChangeListener : virtual public RefBase {
public:
    /**
     * @brief Notify caller when system density changed.
     */
    virtual void OnAcrossDisplaysChanged(bool isAcrossDisplays) {}
};
using IAcrossDisplaysChangeListenerSptr = sptr<IAcrossDisplaysChangeListener>;

/**
 * @class IWindowNoInteractionListenerSptr
 *
 * @brief Listener to observe no interaction event for a long time of window.
 */
class IWindowNoInteractionListener : virtual public RefBase {
public:
    /**
     * @brief Observe event when no interaction for a long time.
     */
    virtual void OnWindowNoInteractionCallback() {};

    /**
     * @brief Set timeout of the listener.
     *
     * @param timeout.
     */
    virtual void SetTimeout(int64_t timeout) {};

    /**
     * @brief get timeout of the listener.
     *
     * @return timeout.
     */
    virtual int64_t GetTimeout() const { return 0;};
};
using IWindowNoInteractionListenerSptr = sptr<IWindowNoInteractionListener>;

/**
 * @class IWindowTitleButtonRectChangedListener
 *
 * @brief Listener to observe event when window size or the height of title bar changed.
 */
class IWindowTitleButtonRectChangedListener : virtual public RefBase {
public:
    /**
     * @brief Notify caller when window size or the height of title bar changed.
     * @param titleButtonRect An area of title buttons relative to the upper right corner of the window.
     */
    virtual void OnWindowTitleButtonRectChanged(const TitleButtonRect& titleButtonRect) {}
};

/**
 * @class IWindowRectChangeListener
 *
 * @brief IWindowRectChangeListener is used to observe the window rect and its changing reason when window changed.
 */
class IWindowRectChangeListener : virtual public RefBase {
public:
    /**
     * @brief Notify caller when window rect changed.
     *
     * @param Rect Rect of the current window.
     * @param reason Reason for window size change.
     */
    virtual void OnRectChange(Rect rect, WindowSizeChangeReason reason) {}
};

/**
 * @class IRectChangeInGlobalDisplayListener
 *
 * @brief Interface for observing window rectangle changes in global coordinates.
 */
class IRectChangeInGlobalDisplayListener : virtual public RefBase {
public:
    /**
     * @brief Called when the window rectangle changes in global coordinates.
     *
     * @param rect The updated rectangle of the window in global coordinates.
     * @param reason The reason for the window size or position change.
     */
    virtual void OnRectChangeInGlobalDisplay(const Rect& rect, WindowSizeChangeReason reason) {}
};

/**
 * @class IExtensionSecureLimitChangeListener
 *
 * @brief IExtensionSecureLimitChangeListener is used to observe the window secure limit and
 *        its change when limit changed.
 */
class IExtensionSecureLimitChangeListener : virtual public RefBase {
public:
    /**
     * @brief Notify caller when window nonsecure limit changed.
     *
     * @param isLimite Whether nonsecure windows is Limite.
     */
    virtual void OnSecureLimitChange(bool isLimit) {}
};

/**
 * @class ISubWindowCloseListener
 *
 * @brief ISubWindowCloseListener is used to observe the window rect and its changing reason when window changed.
 */
class ISubWindowCloseListener : virtual public RefBase {
public:
    /**
     * @brief Notify caller when subwindow closed.
     *
     * @param terminateCloseProcess Whather need to terminate the subwindow close process.
     */
    virtual void OnSubWindowClose(bool& terminateCloseProcess) {}
};

/**
 * @class IMainWindowCloseListener
 *
 * @brief IMainWindowCloseListener is used for preprocessing when the main window exits.
 */
class IMainWindowCloseListener : virtual public RefBase {
public:
    /**
     * @brief Notify caller when main window closed.
     *
     * @param terminateCloseProcess Whether need to terminate the main window close process.
     */
    virtual void OnMainWindowClose(bool& terminateCloseProcess) {}
};

/**
 * @class IWindowWillCloseListener
 *
 * @brief IWindowWillCloseListener is used for async preprocessing when the window exits.
 */
class IWindowWillCloseListener : virtual public RefBase {
public:
    /**
     * @brief Notify caller when window closed.
     *
     * @param terminateCloseProcess Whether need to terminate the window close process.
     */
    virtual void OnWindowWillClose(sptr<Window> window) {}
};
/**
 * @class IWindowHighlightChangeListener
 *
 * @brief IWindowHighlightChangeListener is a listener to observe event when highlight change of window.
 */
class IWindowHighlightChangeListener : virtual public RefBase {
public:
    /**
     * @brief Notify caller when highlight status changes.
     *
     * @param isHighlight Whether the window is highlighted.
     */
    virtual void OnWindowHighlightChange(bool isHighlight) {}
};

/**
 * @class ISwitchFreeMultiWindowListener
 *
 * @brief ISwitchFreeMultiWindowListener is used to observe the free multi window state when it changed.
 */
class ISwitchFreeMultiWindowListener : virtual public RefBase {
public:
    /**
     * @brief Notify caller when free multi window state changed.
     *
     * @param enable Whether free multi window state enabled.
     */
    virtual void OnSwitchFreeMultiWindow(bool enable) {}
};

/**
 * @class IKeyboardPanelInfoChangeListener
 *
 * @brief IKeyboardPanelInfoChangeListener is used to observe the keyboard panel info.
 */
class IKeyboardPanelInfoChangeListener : virtual public RefBase {
public:
    /**
     * @brief Notify caller when keyboard info changed.
     *
     * @param KeyboardPanelInfo keyboardPanelInfo of the keyboard panel;
     */
    virtual void OnKeyboardPanelInfoChanged(const KeyboardPanelInfo& keyboardPanelInfo) {}
};

/**
 * @class IKeyboardWillShowListener
 *
 * @brief IKeyboardWillShowListener is used to observe keyboard show animation begins.
 */
class IKeyboardWillShowListener : virtual public RefBase {
public:
    /**
     * @brief Notify the caller when keyboard show animation begins.
     */
    virtual void OnKeyboardWillShow(const KeyboardAnimationInfo& keyboardAnimationInfo,
        const KeyboardAnimationCurve& curve) {}
};

/**
 * @class IKeyboardWillHideListener
 *
 * @brief IKeyboardWillHideListener is used to observe keyboard hide animation begins.
 */
class IKeyboardWillHideListener : virtual public RefBase {
public:
    /**
     * @brief Notify the caller when keyboard hide animation begins.
     */
    virtual void OnKeyboardWillHide(const KeyboardAnimationInfo& keyboardAnimationInfo,
        const KeyboardAnimationCurve& curve) {}
};

/**
 * @class IKeyboardDidShowListener
 *
 * @brief IKeyboardDidShowListener is used to observe keyboard show animation completion.
 */
class IKeyboardDidShowListener : virtual public RefBase {
public:
    /**
     * @brief Notify the caller when keyboard show animation is completed.
     */
    virtual void OnKeyboardDidShow(const KeyboardPanelInfo& keyboardPanelInfo) {}
};

/**
 * @class IKeyboardDidHideListener
 *
 * @brief IKeyboardDidHideListener is used to observe keyboard hide animation completion.
 */
class IKeyboardDidHideListener : virtual public RefBase {
public:
    /**
     * @brief Notify the caller when keyboard hide animation is completed.
     */
    virtual void OnKeyboardDidHide(const KeyboardPanelInfo& keyboardPanelInfo) {}
};

/**
 * @class IWaterfallModeChangeListener
 *
 * @brief IWaterfallModeChangeListener is used to observe the waterfall mode.
 */
class IWaterfallModeChangeListener : virtual public RefBase {
public:
    /**
     * @brief Notify caller when waterfall mode changed.
     *
     * @param isWaterfallMode new waterfall mode.
     */
    virtual void OnWaterfallModeChange(bool isWaterfallMode) {}
};

/**
 * @class IPreferredOrientationChangeListener
 *
 * @brief listener of preferred orientation change which set by developer.
 */
class IPreferredOrientationChangeListener : virtual public RefBase {
public:
    /**
     * @brief Notify caller when orientation set by developer.
     *
     * @param ori the orientation set by developer.
     */
    virtual void OnPreferredOrientationChange(Orientation orientation) {}
};

/**
 * @class IWindowOrientationChangeListener
 *
 * @brief IWindowOrientationChangeListener is used to notify while window rotate.
 */
class IWindowOrientationChangeListener : virtual public RefBase {
public:
    /**
     * @brief Innerapi, notify caller while window rotate.
     */
    virtual void OnOrientationChange() {}
};

/**
 * @class ISystemBarPropertyListener
 *
 * @brief ISystemBarPropertyListener is used to notify while developer set SystemBarProperty.
 */
class ISystemBarPropertyListener : virtual public RefBase {
public:
    /**
     * @brief Innerapi, notify caller when developer set SystemBarProperty.
     */
    virtual void OnSystemBarPropertyUpdate(WindowType type, const SystemBarProperty& property) {}
};

/*
 * @class IWindowRotationChangeListener
 *
 * @brief IWindowRotationChangeListener is used to observe the window rotation change.
 */
class IWindowRotationChangeListener : virtual public RefBase {
public:
    /**
     * @brief Notify caller when window rotate
     *
     * @param rotationChangeInfo information of rotation
     * @param rotationChangeResult result of rotation
     *
     */
    virtual void OnRotationChange(const RotationChangeInfo& rotationChangeInfo,
        RotationChangeResult& rotationChangeResult) {}
};

/**
 * @class IFreeWindowModeChangeListener
 *
 * @brief IFreeWindowModeChangeListener is used to observe the free window mode when it changed.
 */
class IFreeWindowModeChangeListener : virtual public RefBase {
public:
    /**
     * @brief Notify caller when free window mode changed.
     *
     * @param isInFreeWindowMode Whether in free window mode.
     */
    virtual void OnFreeWindowModeChange(bool isInFreeWindowMode) {}
};

/**
 * @class IUIContentCreateListener
 *
 * @brief IUIContentCreateListener is used to observe the ui content create.
 */
class IUIContentCreateListener : virtual public RefBase {
public:
    /**
     * @brief Notify caller when ui content create.
     *
     * @param uiContent created UIContent object.
     */
    virtual void OnUIContentCreate(std::weak_ptr<Ace::UIContent> uiContent) {}
};

static WMError DefaultCreateErrCode = WMError::WM_OK;
class Window : virtual public RefBase {
public:
    /**
     * @brief create window, include main_window/sub_window/system_window
     *
     * @param windowName window name, identify window instance
     * @param option window propertion
     * @param context ability context
     * @return sptr<Window> If create window success,return window instance;Otherwise, return nullptr
     */
    static sptr<Window> Create(const std::string& windowName,
        sptr<WindowOption>& option, const std::shared_ptr<AbilityRuntime::Context>& context = nullptr,
        WMError& errCode = DefaultCreateErrCode,
        const std::shared_ptr<RSUIContext>& rsUiContext = nullptr);

    /**
     * @brief create main/uiextension window with session
     *
     * @param option window propertion
     * @param context ability context
     * @param iSession session token of window session
     * @param errCode error code of create window
     * @param identityToken identity token of sceneSession
     * @return sptr<Window> If create window success, return window instance; Otherwise, return nullptr
     */
    static sptr<Window> Create(sptr<WindowOption>& option, const std::shared_ptr<AbilityRuntime::Context>& context,
        const sptr<IRemoteObject>& iSession, WMError& errCode = DefaultCreateErrCode,
        const std::string& identityToken = "", bool isModuleAbilityHookEnd = false);

    /**
     * @brief get and verify windowType, include sub_window/system_window
     *
     * @param parentId parent window id
     * @param windowName current window name
     * @param parentWindowType parent window type
     * @param windowType current window type
     * @return WMError::WM_OK means check success, otherwise failed.
     */
    static WMError GetAndVerifyWindowTypeForArkUI(uint32_t parentId, const std::string& windowName,
        WindowType parentWindowType, WindowType& windowType);

    /**
     * @brief create pip window with session
     *
     * @param option window propertion
     * @param pipTemplateInfo pipTemplateInfo
     * @param context ability context
     * @param errCode error code of create pip window
     * @return sptr<Window> If create pip window success, return window instance; Otherwise, return nullptr
     */
    static sptr<Window> CreatePiP(sptr<WindowOption>& option, const PiPTemplateInfo& pipTemplateInfo,
        const std::shared_ptr<OHOS::AbilityRuntime::Context>& context, WMError& errCode = DefaultCreateErrCode);

    /**
     * @brief create fb window with session
     *
     * @param option window propertion
     * @param fbTemplateBaseInfo baseInfo of fb window
     * @param icon icon of fb window
     * @param context ability context
     * @param errCode error code of create fb window
     * @return sptr<Window> If create fb window success, return window instance; Otherwise, return nullptr
     */
    static sptr<Window> CreateFb(sptr<WindowOption>& option, const FloatingBallTemplateBaseInfo& fbTemplateBaseInfo,
        const std::shared_ptr<Media::PixelMap>& icon, const std::shared_ptr<OHOS::AbilityRuntime::Context>& context,
        WMError& errCode);

    /**
     * @brief find window by windowName
     *
     * @param windowName
     * @return sptr<Window> Return the window instance founded
     */
    static sptr<Window> Find(const std::string& windowName);

    /**
     * @brief Get parent main windowId, which is used for mainWindow,subWindow or dialog
     *
     * @param windowId window id that need to get parent main window
     * @return uint32_t Return the parent main window id
     */
    static uint32_t GetParentMainWindowId(uint32_t windowId);

    /**
     * @brief Get the final show window by context. Its implemented in api8
     *
     * @param context Indicates the context on which the window depends
     * @return sptr<Window>
     */
    static sptr<Window> GetTopWindowWithContext(const std::shared_ptr<AbilityRuntime::Context>& context = nullptr);

    /**
     * @brief Get the final show window by id. Its implemented in api8
     *
     * @param mainWinId main window id?
     * @return sptr<Window>
     */
    static sptr<Window> GetTopWindowWithId(uint32_t mainWinId);

    /**
     * @brief Get the main window by context.
     *
     * @param context Indicates the context on which the window depends
     * @return sptr<Window>
     */
    static sptr<Window> GetMainWindowWithContext(const std::shared_ptr<AbilityRuntime::Context>& context = nullptr);

    /**
     * @brief Get the all sub windows by parent
     *
     * @param parentId parent window id
     * @return std::vector<sptr<Window>>
     */
    static std::vector<sptr<Window>> GetSubWindow(uint32_t parentId);

    /**
     * @brief Update configuration for all windows
     *
     * @param configuration configuration for app
     * @param ignoreWindowContexts context of window which will be ignored
     */
    static void UpdateConfigurationForAll(const std::shared_ptr<AppExecFwk::Configuration>& configuration,
        const std::vector<std::shared_ptr<AbilityRuntime::Context>>& ignoreWindowContexts = {});

    /**
     * @brief Update theme configuration for all windows
     * @param configuration configuration for app
     */
    static void UpdateConfigurationSyncForAll(const std::shared_ptr<AppExecFwk::Configuration>& configuration);

    /**
     * @brief Get surface node from RS
     *
     * @return Surface node from RS
     */
    virtual std::shared_ptr<RSSurfaceNode> GetSurfaceNode() const { return nullptr; }

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

    /**
     * @brief Get ability context
     *
     * @return Ability context from AbilityRuntime
     */
    virtual const std::shared_ptr<AbilityRuntime::Context> GetContext() const { return nullptr; }

    /**
     * @brief Get the window show rect
     *
     * @return Rect of window
     */
    virtual Rect GetRect() const { return {}; }

    /**
     * @brief Get window default rect from window property.
     *
     * @return Rect of window.
     */
    virtual Rect GetRequestRect() const { return {}; }

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

    /**
     * @brief Get the window type
     *
     * @return Type of window
     */
    virtual WindowType GetType() const { return WindowType::WINDOW_TYPE_APP_MAIN_WINDOW; }

    /**
     * @brief Get the window mode.
     *
     * @return Mode of window.
     */
    virtual WindowMode GetWindowMode() const { return WindowMode::WINDOW_MODE_UNDEFINED; }

    /**
     * @brief Get alpha of window.
     *
     * @return Alpha of window.
     */
    virtual float GetAlpha() const { return 0.0f; }

    /**
     * @brief Get the name of window.
     *
     * @return Name of window.
     */
    virtual const std::string& GetWindowName() const
    {
        static const std::string name;
        return name;
    }

    /**
     * @brief Get id of window.
     *
     * @return ID of window.
     */
    virtual uint32_t GetWindowId() const { return INVALID_WINDOW_ID; }

    /**
     * @brief Get displayId of window.
     *
     * @return displayId of window.
     */
    virtual uint64_t GetDisplayId() const { return DISPLAY_ID_INVALID; }

    /**
     * @brief Get flag of window.
     *
     * @return Flag of window.
     */
    virtual uint32_t GetWindowFlags() const { return 0; }

    /**
     * @brief Get state of window.
     *
     * @return Current state of window.
     */
    virtual WindowState GetWindowState() const { return WindowState::STATE_INITIAL; }

    /**
     * @brief Set focusable property of window.
     *
     * @param isFocusable Window can be focused or not.
     * @return Errorcode of window.
     */
    virtual WMError SetFocusable(bool isFocusable) { return WMError::WM_OK; }

    /**
     * @brief Get focusable property of window.
     *
     * @return True means window can be focused, false means window cannot be focused.
     */
    virtual bool GetFocusable() const { return false; }

    /**
     * @brief Set touchable property of window.
     *
     * @param isTouchable Window can be touched or not.
     * @return Errorcode of window.
     */
    virtual WMError SetTouchable(bool isTouchable) { return WMError::WM_OK; }

    /**
     * @brief Get touchable property of window.
     *
     * @return True means window can be touched, false means window cannot be touched.
     */
    virtual bool GetTouchable() const { return false; }

    /**
     * @brief Set follow screen change property of window.
     *
     * @param isFollowScreenChange Window follow screen change.
     * @return WMError.
     */
    virtual WMError SetFollowScreenChange(bool isFollowScreenChange) { return WMError::WM_OK; }

    /**
     * @brief Get SystemBarProperty By WindowType.
     *
     * @param type Type of window.
     * @return Property of system bar.
     */
    virtual SystemBarProperty GetSystemBarPropertyByType(WindowType type) const { return {}; }

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
     * @brief judge this window is full screen.
     *
     * @return true If SetFullScreen(true) is called , return true.
     * @return false default return false
     */
    virtual bool IsFullScreen() const { return false; }

    /**
     * @brief judge window layout is full screen
     *
     * @return true this window layout is full screen
     * @return false this window layout is not full screen
     */
    virtual bool IsLayoutFullScreen() const { return false; }

    /**
     * @brief Set the Window Type
     *
     * @param type window type
     * @return WMError
     */
    virtual WMError SetWindowType(WindowType type) { return WMError::WM_OK; }

    /**
     * @brief Set the Window Mode
     *
     * @param mode window mode
     * @return WMError
     */
    virtual WMError SetWindowMode(WindowMode mode) { return WMError::WM_OK; }

    /**
     * @brief Set whether the window is topmost
     *
     * @param topmost whether window is topmost
     * @return WMError
     */
    virtual WMError SetTopmost(bool topmost) { return WMError::WM_OK; }

    /**
     * @brief Get whether window is topmost
     *
     * @return True means window is topmost
     */
    virtual bool IsTopmost() const { return false; }

    /**
     * @brief Set whether the main window is topmost
     *
     * @param isTopmost whether main window is topmost
     * @return WMError
     */
    virtual WMError SetMainWindowTopmost(bool isTopmost) { return WMError::WM_ERROR_DEVICE_NOT_SUPPORT; }

    /**
     * @brief Set static Image resource for recent.
     *
     * @param imgResourceId resourceId of static image.
     * @param imageFit imageFit of static image.
     * @return WM_OK means set success, others means failed.
     */
    virtual WMError SetImageForRecent(uint32_t imgResourceId, ImageFit imageFit)
    {
        return WMError::WM_ERROR_DEVICE_NOT_SUPPORT;
    }

    /**
     * @brief Set static Image resource for recent.
     *
     * @param pixelMap recent image.
     * @param imageFit imageFit of static image.
     * @return WM_OK means set success, others means failed.
     */
    virtual WMError SetImageForRecentPixelMap(const std::shared_ptr<Media::PixelMap>& pixelMap, ImageFit imageFit)
    {
        return WMError::WM_ERROR_DEVICE_NOT_SUPPORT;
    }

    /**
     * @brief Remove static Image resource for recent.
     *
     * @return WM_OK means set success, others means failed.
     */
    virtual WMError RemoveImageForRecent()
    {
        return WMError::WM_ERROR_DEVICE_NOT_SUPPORT;
    }

    /**
     * @brief Get whether main window is topmost
     *
     * @return True means main window is topmost
     */
    virtual bool IsMainWindowTopmost() const { return false; }

    /**
     * @brief Set sub window zLevel
     *
     * @param zLevel zLevel of sub window to specify the hierarchical relationship among sub windows
     * @return WM_OK means success, others mean set failed
     */
    virtual WMError SetSubWindowZLevel(int32_t zLevel) { return WMError::WM_ERROR_DEVICE_NOT_SUPPORT; }

    /**
     * @brief Get sub window zLevel
     *
     * @param zLevel sub window zLevel
     * @return WM_OK means success, others mean get failed
     */
    virtual WMError GetSubWindowZLevel(int32_t& zLevel) { return WMError::WM_ERROR_DEVICE_NOT_SUPPORT; }

    /**
     * @brief Set alpha of window.
     *
     * @param alpha Alpha of window.
     * @return WM_OK means success, others means set failed.
     */
    virtual WMError SetAlpha(float alpha) { return WMError::WM_OK; }

    /**
     * @brief Set transform of window property.
     *
     * @param trans Window Transform.
     * @return WMError
     */
    virtual WMError SetTransform(const Transform& trans) { return WMError::WM_OK; }

    /**
     * @brief Get transform of window property.
     *
     * @return Property of transform.
     */
    virtual const Transform& GetTransform() const
    {
        static const Transform trans;
        return trans;
    }

    /**
     * @brief Add window flag.
     *
     * @param flag Flag of window.
     * @return WM_OK means add success, others means failed.
     */
    virtual WMError AddWindowFlag(WindowFlag flag) { return WMError::WM_OK; }

    /**
     * @brief Remove window flag.
     *
     * @param flag Flag of window
     * @return WM_OK means remove success, others means failed.
     */
    virtual WMError RemoveWindowFlag(WindowFlag flag) { return WMError::WM_OK; }

    /**
     * @brief Set window flag.
     *
     * @param flags Flag of window
     * @return WM_OK means set success, others means failed.
     */
    virtual WMError SetWindowFlags(uint32_t flags) { return WMError::WM_OK; }

    /**
     * @brief Set the System Bar(include status bar and nav bar) Property
     *
     * @param type WINDOW_TYPE_STATUS_BAR or WINDOW_TYPE_NAVIGATION_BAR
     * @param property system bar prop,include content color, background color
     * @return WMError
     */
    virtual WMError SetSystemBarProperty(WindowType type, const SystemBarProperty& property) { return WMError::WM_OK; }

    /**
     * @brief Get the Avoid Area By Type object
     *
     * @param type avoid area type.@see reference
     * @param avoidArea
     * @param rect
     * @return WMError
     */
    virtual WMError GetAvoidAreaByType(AvoidAreaType type, AvoidArea& avoidArea,
        const Rect& rect = Rect::EMPTY_RECT, int32_t apiVersion = API_VERSION_INVALID) { return WMError::WM_OK; }
    
    /**
     * @brief Get the avoid area by type ignoring visibility
     *
     * @param type avoid area type.@see reference
     * @param avoidArea
     * @param rect
     * @return WMError
     */
    virtual WMError GetAvoidAreaByTypeIgnoringVisibility(AvoidAreaType type, AvoidArea& avoidArea,
        const Rect& rect = Rect::EMPTY_RECT) { return WMError::WM_ERROR_DEVICE_NOT_SUPPORT; }

    /**
     * @brief Set whether the system or app sub window can obtain area
     *
     * @param avoidAreaOption from low to high, the first bit means system window, the second bit means app sub window
     * @return WMError
     */
    virtual WMError SetAvoidAreaOption(uint32_t avoidAreaOption) { return WMError::WM_OK; }

    /**
     * @brief Get the Avoid Area of system or app sub window Enabled object
     *
     * @param avoidAreaOption from low to high, the first bit means system window, the second bit means app sub window
     * @return WMError
     */
    virtual WMError GetAvoidAreaOption(uint32_t& avoidAreaOption) { return WMError::WM_OK; }

    /**
     * @brief Is system window or not
     *
     * @return True means the window is system window, false means the window is not system window
     */
    virtual bool IsSystemWindow() const { return false; }

    /**
     * @brief Is app window or not
     *
     * @return True means the window is app window, false means the window is not app window
     */
    virtual bool IsAppWindow() const { return false; }

    /**
     * @brief Set this window layout full screen, with hide status bar and nav bar above on this window
     *
     * @param status
     * @return WMError
     */
    virtual WMError SetLayoutFullScreen(bool status) { return WMError::WM_OK; }

    /**
     * @brief Set the immersive layout properties
     *
     * @param isIgnoreSafeArea
     * @return WMError
     */
    virtual WMError SetIgnoreSafeArea(bool isIgnoreSafeArea) { return WMError::WM_OK; }

    /**
     * @brief Set whether the title bar and dock bar will show, when the mouse hovers over hot area.
     *
     * @param isTitleHoverShown
     * @param isDockHoverShown
     * @return WMError
     */
    virtual WMError SetTitleAndDockHoverShown(bool isTitleHoverShown = true,
        bool isDockHoverShown = true)
    {
        return WMError::WM_ERROR_DEVICE_NOT_SUPPORT;
    }

    /**
     * @brief Set this window full screen, with hide status bar and nav bar
     *
     * @param status if true, hide status bar and nav bar; Otherwise, show status bar and nav bar
     * @return WMError
     */
    virtual WMError SetFullScreen(bool status) { return WMError::WM_OK; }

    /**
     * @brief destroy window
     *
     * @return WMError
     */
    virtual WMError Destroy(uint32_t reason = 0) { return WMError::WM_OK; }

    /**
     * @brief Set a flag to distinguish whether the window is shown with options.
     *
     * @param showWithOptions Options that define the behavior of a window while it is showing.
     */
    virtual void SetShowWithOptions(bool showWithOptions) {}

    /**
     * @brief Get showWithOptions value.
     *
     * @return True means the window is shown with options, false means the opposite.
     */
    virtual bool IsShowWithOptions() const { return false; }

    /**
     * @brief Show window
     *
     * @param reason Reason for window state change.
     * @param withAnimation True means window show with animation, false means window show without animation.
     * @param withFocus True means window can get focus when it shows to foreground, false means the opposite;
     * @return WM_OK means window show success, others means failed.
     */
    virtual WMError Show(uint32_t reason = 0, bool withAnimation = false,
                         bool withFocus = true) { return WMError::WM_OK; }

    /**
     * @brief Show window
     *
     * @param reason Reason for window state change.
     * @param withAnimation True means window show with animation, false means window show without animation.
     * @param withFocus True means window can get focus when it shows to foreground, false means the opposite;
     * @param waitAttach True means window need waiting for attach when it, false means the opposite;
     * @return WM_OK means window show success, others means failed.
     */
    virtual WMError Show(uint32_t reason, bool withAnimation,
                         bool withFocus, bool waitAttach) { return WMError::WM_OK; }

    /**
     * @brief Resume window
     */
    virtual void Resume() {}

    /**
     * @brief Pause window
     */
    virtual void Pause() {}

    /**
     * @brief Hide window
     *
     * @param reason Reason for window state change.
     * @param withAnimation True means window show with animation, false means window show without animation.
     * @param isFromInnerkits True means remove command is from inner kits.
     * @return WM_OK means window hide success, others means failed.
     */
    virtual WMError Hide(uint32_t reason = 0, bool withAnimation = false, bool isFromInnerkits = true)
    {
        return WMError::WM_OK;
    }

    /**
     * @brief Hide window
     *
     * @param reason Reason for window state change.
     * @param withAnimation True means window show with animation, false means window show without animation.
     * @param isFromInnerkits True means remove command is from inner kits.
     * @param waitDetach True means window need waiting for detach, false means the opposite;
     * @return WM_OK means window hide success, others means failed.
     */
    virtual WMError Hide(uint32_t reason, bool withAnimation,
        bool isFromInnerkits, bool waitDetach)
    {
        return WMError::WM_OK;
    }

    /**
     * @brief notify window first frame drawing completed.
     *
     * @return WMError
     */
    virtual WMError NotifyDrawingCompleted() { return WMError::WM_OK; }

    /**
     * @brief notify window remove starting window.
     *
     * @return WMError
     */
    virtual WMError NotifyRemoveStartingWindow() { return WMError::WM_ERROR_DEVICE_NOT_SUPPORT; }

    /**
     * @brief move the window to (x, y)
     *
     * @param x
     * @param y
     * @param isMoveToGlobal Indicates move global flag
     * @param moveConfiguration Indicates the optional move configuration
     * @return WMError
     */
    virtual WMError MoveTo(int32_t x, int32_t y, bool isMoveToGlobal = false,
        MoveConfiguration moveConfiguration = {}) { return WMError::WM_OK; }

    /**
     * @brief move the window to (x, y)
     *
     * @param x
     * @param y
     * @param moveConfiguration Indicates the optional move configuration
     * @return WMError
     */
    virtual WMError MoveToAsync(int32_t x, int32_t y,
        MoveConfiguration moveConfiguration = {}) { return WMError::WM_ERROR_DEVICE_NOT_SUPPORT; }

    /**
     * @brief move the window to global (x, y)
     *
     * @param x
     * @param y
     * @param moveConfiguration Indicates the optional move configuration
     * @return WMError
     */
    virtual WMError MoveWindowToGlobal(int32_t x, int32_t y,
        MoveConfiguration moveConfiguration) { return WMError::WM_ERROR_DEVICE_NOT_SUPPORT; }

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

    /**
     * @brief Get window global scaled rect.
     *
     * @param Rect
     * @return WMError
     */
    virtual WMError GetGlobalScaledRect(Rect& globalScaledRect) { return WMError::WM_ERROR_DEVICE_NOT_SUPPORT; }

    /**
     * @brief resize the window instance (w,h)
     *
     * @param width
     * @param height
     * @return WMError
     */
    virtual WMError Resize(uint32_t width, uint32_t height,
        const RectAnimationConfig& rectAnimationConfig = {}) { return WMError::WM_OK; }

    /**
     * @brief resize the window instance (w,h)
     *
     * @param width
     * @param height
     * @return WMError
     */
    virtual WMError ResizeAsync(uint32_t width, uint32_t height) { return WMError::WM_ERROR_DEVICE_NOT_SUPPORT; }

    /**
     * @brief set the window gravity
     *
     * @param gravity
     * @param percent
     * @return WMError
     */
    virtual WMError SetWindowGravity(WindowGravity gravity, uint32_t percent) { return WMError::WM_OK; }

    /**
     * @brief Set the screen always on
     *
     * @param keepScreenOn
     * @return WMError
     */
    virtual WMError SetKeepScreenOn(bool keepScreenOn) { return WMError::WM_OK; }

    /**
     * @brief Extension told host to set the screen always on, only for NAPI call.
     *
     * @param keepScreenOn
     * @return WMError
     */
    virtual WMError ExtensionSetKeepScreenOn(bool keepScreenOn) { return WMError::WM_OK; }

    /**
     * @brief Get the screen is always on or not.
     *
     * @return True means screen is always on, false means the opposite.
     */
    virtual bool IsKeepScreenOn() const { return false; }

    /**
     * @brief Set the view screen always on or not.
     *
     * @param keepScreenOn
     * @return WMError
     */
    virtual WMError SetViewKeepScreenOn(bool keepScreenOn) { return WMError::WM_OK; }

    /**
     * @brief Get the view screen is always on or not.
     *
     * @return True means view screen is always on, false means the opposite.
     */
    virtual bool IsViewKeepScreenOn() const { return false; }

    /**
     * @brief Set the screen on
     *
     * @param turnScreenOn True means turn screen on, false means the opposite.
     * @return WM_OK means set success, others means set failed.
     */
    virtual WMError SetTurnScreenOn(bool turnScreenOn) { return WMError::WM_OK; }

    /**
     * @brief Get the screen is on or not.
     *
     * @return True means screen is on, false means screen is off.
     */
    virtual bool IsTurnScreenOn() const { return false; }

    /**
     * @brief Set Background color.
     *
     * @param color Background color.
     * @return WM_OK means set success, others means set failed.
     */
    virtual WMError SetBackgroundColor(const std::string& color) { return WMError::WM_OK; }

    /**
     * @brief Set transparent status.
     *
     * @param isTransparent True means set window transparent, false means the opposite.
     * @return WM_OK means set success, others means set failed.
     */
    virtual WMError SetTransparent(bool isTransparent) { return WMError::WM_OK; }

    /**
     * @brief Get transparent status.
     *
     * @return True means window is transparent, false means the opposite.
     */
    virtual bool IsTransparent() const { return false; }

    /**
     * @brief Set brightness value of window.
     *
     * @param brightness Brightness of window.
     * @return WM_OK means set success, others means set failed.
     */
    virtual WMError SetBrightness(float brightness) { return WMError::WM_OK; }

    /**
     * @brief Extension told host to set brightness value of window, only for NAPI call.
     *
     * @param brightness Brightness of window.
     * @return WM_OK means set success, others means set failed.
     */
    virtual WMError ExtensionSetBrightness(float brightness) { return WMError::WM_OK; }

    /**
     * @brief Get brightness value of window.
     *
     * @return Brightness value of window.
     */
    virtual float GetBrightness() const { return 0.0f; }

    /**
     * @brief Change calling window id.
     *
     * @param callingWindowId Window id.
     * @return WM_OK means change success, others means change failed.
     */
    virtual WMError ChangeCallingWindowId(uint32_t callingWindowId) { return WMError::WM_OK; }

    /**
     * @brief Set privacy mode of window.
     *
     * @param isPrivacyMode True means set window private, false means not set window private.
     * @return WM_OK means set success, others means set failed.
     */
    virtual WMError SetPrivacyMode(bool isPrivacyMode) { return WMError::WM_OK; }

    /**
     * @brief Get privacy property of window.
     *
     * @return True means window is private and cannot be screenshot or recorded.
     */
    virtual bool IsPrivacyMode() const { return false; }

    /**
     * @brief Set privacy mode by system.
     *
     * @param isSystemPrivacyMode True means set window private, false means not set window private.
     */
    virtual void SetSystemPrivacyMode(bool isSystemPrivacyMode) {}

    /**
     * @brief Bind Dialog window to target token.
     *
     * @param targetToken Window token of target.
     * @return WM_OK means set success, others means set failed.
     */
    virtual WMError BindDialogTarget(sptr<IRemoteObject> targetToken) { return WMError::WM_OK; }

    /**
     * @brief Set whether the dialog window responds to back gesture.
     *
     * @param isEnabled Responds to back gesture if true, or ignore back gesture if false.
     * @return WM_OK means set success, others means set failed.
     */
    virtual WMError SetDialogBackGestureEnabled(bool isEnabled)
    {
        return WMError::WM_ERROR_DEVICE_NOT_SUPPORT;
    }

    /**
     * @brief Raise zorder of window to the top of APP Mainwindow.
     *
     * @return WM_OK means raise success, others means raise failed.
     */
    virtual WMError RaiseToAppTop() { return WMError::WM_OK; }

    /**
     * @brief Set skip flag of snapshot.
     *
     * @param isSkip True means skip the snapshot, false means the opposite.
     * @return WM_OK means set success, others means set failed.
     */
    virtual WMError SetSnapshotSkip(bool isSkip) { return WMError::WM_OK; }

    // window effect
    /**
     * @brief Set corner radius of window.
     *
     * @param cornerRadius Corner radius of window
     * @return WM_OK means set success, others means set failed.
     */
    virtual WMError SetCornerRadius(float cornerRadius) { return WMError::WM_OK; }

    /**
     * @brief Sets corner radius of window.
     *
     * @param cornerRadius Corner radius of window.
     * @return WM_OK means set success, others means set failed.
     */
    virtual WMError SetWindowCornerRadius(float cornerRadius) { return WMError::WM_ERROR_DEVICE_NOT_SUPPORT; }

    /**
     * @brief Get corner radius of window.
     *
     * @param cornerRadius Corner radius of window.
     * @return WM_OK means set success, others means set failed.
     */
    virtual WMError GetWindowCornerRadius(float& cornerRadius) { return WMError::WM_ERROR_DEVICE_NOT_SUPPORT; }

    /**
     * @brief Set shadow radius of window.
     *
     * @param radius Shadow radius of window
     * @return WM_OK means set success, others means set failed.
     */
    virtual WMError SetShadowRadius(float radius) { return WMError::WM_OK; }

    /**
     * @brief Sync shadows to component.
     *
     * @param radius Shadows of window
     * @return WM_OK means set success, others means set failed.
     */
    virtual WMError SyncShadowsToComponent(const ShadowsInfo& shadowsInfo)
    {
        return WMError::WM_ERROR_DEVICE_NOT_SUPPORT;
    }

    /**
     * @brief Set shadow radius of window.
     *
     * @param radius Shadow radius of window.
     * @return WM_OK means set success, others means set failed.
     */
    virtual WMError SetWindowShadowRadius(float radius) { return WMError::WM_ERROR_DEVICE_NOT_SUPPORT; }

    /**
     * @brief Set shadow color of window.
     *
     * @param color Shadow color of window.
     * @return WM_OK means set success, others means set failed.
     */
    virtual WMError SetShadowColor(std::string color) { return WMError::WM_OK; }

    /**
     * @brief Set shadow X offset.
     *
     * @param offsetX Shadow x-axis offset.
     * @return WM_OK means set success, others means set failed.
     */
    virtual WMError SetShadowOffsetX(float offsetX) { return WMError::WM_OK; }

    /**
     * @brief Set shadow Y offset.
     *
     * @param offsetY Shadow y-axis offset.
     * @return WM_OK means set success, others means set failed.
     */
    virtual WMError SetShadowOffsetY(float offsetY) { return WMError::WM_OK; }

    /**
     * @brief Set blur property.
     *
     * @param radius Blur value.
     * @return WM_OK means set success, others means set failed.
     */
    virtual WMError SetBlur(float radius) { return WMError::WM_OK; }

    /**
     * @brief Set Backdrop blur property.
     *
     * @param radius Backdrop blur value.
     * @return WM_OK means set success, others means set failed.
     */
    virtual WMError SetBackdropBlur(float radius) { return WMError::WM_OK; }

    /**
     * @brief Set Backdrop blur style.
     *
     * @param blurStyle Backdrop blur value.
     * @return WM_OK means set success, others means set failed.
     */
    virtual WMError SetBackdropBlurStyle(WindowBlurStyle blurStyle) { return WMError::WM_OK; }

    /**
     * @brief Request to get focus.
     *
     * @return WM_OK means request success, others means request failed.
     */
    virtual WMError RequestFocus() const { return WMError::WM_OK; }

    /**
     * @brief Request to get focus or lose focus.
     *
     * @return WM_OK means request success, others means request failed.
     */
    virtual WMError RequestFocusByClient(bool isFocused) const { return WMError::WM_ERROR_DEVICE_NOT_SUPPORT; }

    /**
     * @brief Check current focus status.
     *
     * @return True means window is focused, false means window is unfocused.
     */
    virtual bool IsFocused() const { return false; }

    /**
     * @brief Check current UIExtensionComponent focus status.
     *
     * @return True means UIExtensionComponent is focused, false means UIExtensionComponent is unfocused.
     */
    virtual bool IsComponentFocused() const { return false; }

    /**
     * @brief Update surfaceNode after customAnimation.
     *
     * @param isAdd True means add custom animation, false means the opposite.
     * @return WM_OK means update success, others means update failed.
     */
    virtual WMError UpdateSurfaceNodeAfterCustomAnimation(bool isAdd) { return WMError::WM_OK; }

    /**
     * @brief Set InputEvent Consumer.
     *
     * @param inputEventConsumer Consume input event object.
     * @return WM_OK means set success, others means set failed.
     */
    virtual void SetInputEventConsumer(const std::shared_ptr<IInputEventConsumer>& inputEventConsumer) {}

    /**
     * @brief Consume KeyEvent from MMI.
     *
     * @param inputEvent Keyboard input event.
     */
    virtual void ConsumeKeyEvent(std::shared_ptr<MMI::KeyEvent>& inputEvent) {}

    /**
     * @brief Consume BackEvent by keyEvent with keyCode_back.
     */
    virtual void ConsumeBackEvent() {}

    /**
     * @brief Determine whether the dialog session back gesture is enabled.
     */
    virtual bool IsDialogSessionBackGestureEnabled()
    {
        return false;
    }

    /**
     * @brief Notify KeyEvent to arkui.
     *
     * @param inputEvent Keyboard input event
     */
    virtual bool PreNotifyKeyEvent(const std::shared_ptr<MMI::KeyEvent>& keyEvent) {return false;}

    /**
     * @brief Consume PointerEvent from MMI.
     *
     * @param inputEvent Pointer input event
     */
    virtual void ConsumePointerEvent(const std::shared_ptr<MMI::PointerEvent>& inputEvent) {}

    /**
     * @brief Request Vsync.
     *
     * @param vsyncCallback Callback of vsync.
     */
    virtual void RequestVsync(const std::shared_ptr<VsyncCallback>& vsyncCallback) {}

    /**
     * @brief get vsync period.
     *
     * @return vsync period.
     */
    virtual int64_t GetVSyncPeriod() { return 0; }

    /**
     * @brief flush frame rate of linker.
     *
     * @param rate frame rate.
     * @param animatorExpectedFrameRate animator expected frame rate.
     * @param rateType frame rate type.
     */
    virtual void FlushFrameRate(uint32_t rate, int32_t animatorExpectedFrameRate, uint32_t rateType) {}

    /**
     * @brief Update Configuration.
     *
     * @param configuration Window configuration.
     */
    virtual void UpdateConfiguration(const std::shared_ptr<AppExecFwk::Configuration>& configuration) {}

    /**
     * @brief Update configuration for specified window.
     *
     * @param configuration Window configuration.
     * @param resourceManager The resource manager
     */
    virtual void UpdateConfigurationForSpecified(const std::shared_ptr<AppExecFwk::Configuration>& configuration,
        const std::shared_ptr<Global::Resource::ResourceManager>& resourceManager) {}

    /**
     * @brief Update theme configuration.
     * @param configuration Window configuration.
     */
    virtual void UpdateConfigurationSync(const std::shared_ptr<AppExecFwk::Configuration>& configuration) {}

    /**
     * @brief Register window lifecycle listener.
     *
     * @param listener WindowLifeCycle listener.
     * @return WM_OK means register success, others means register failed.
     */
    virtual WMError RegisterLifeCycleListener(const sptr<IWindowLifeCycle>& listener) { return WMError::WM_OK; }

    /**
     * @brief Unregister window lifecycle listener.
     *
     * @param listener WindowLifeCycle listener.
     * @return WM_OK means unregister success, others means unregister failed.
     */
    virtual WMError UnregisterLifeCycleListener(const sptr<IWindowLifeCycle>& listener) { return WMError::WM_OK; }

    /**
     * @brief Register window lifecycle listener.
     *
     * @param listener WindowLifeCycle listener.
     * @return WM_OK means register success, others means register failed.
     */
    virtual WMError RegisterWindowStageLifeCycleListener(const sptr<IWindowStageLifeCycle>& listener)
    {
        return WMError::WM_OK;
    }

    /**
     * @brief Unregister window lifecycle listener.
     *
     * @param listener WindowLifeCycle listener.
     * @return WM_OK means unregister success, others means unregister failed.
     */
    virtual WMError UnregisterWindowStageLifeCycleListener(const sptr<IWindowStageLifeCycle>& listener)
    {
        return WMError::WM_OK;
    }

    /**
     * @brief Register window change listener.
     *
     * @param listener IWindowChangeListener.
     * @return WM_OK means register success, others means register failed.
     */
    virtual WMError RegisterWindowChangeListener(const sptr<IWindowChangeListener>& listener)
    {
        return WMError::WM_OK;
    }

    /**
     * @brief Unregister window change listener.
     *
     * @param listener IWindowChangeListener.
     * @return WM_OK means unregister success, others means unregister failed.
     */
    virtual WMError UnregisterWindowChangeListener(const sptr<IWindowChangeListener>& listener)
    {
        return WMError::WM_OK;
    }

    /**
     * @brief Register avoid area change listener.
     *
     * @param listener IAvoidAreaChangedListener.
     * @return WM_OK means register success, others means register failed.
     */
    virtual WMError RegisterAvoidAreaChangeListener(const sptr<IAvoidAreaChangedListener>& listener)
    {
        return WMError::WM_OK;
    }

    /**
     * @brief Unregister avoid area change listener.
     *
     * @param listener IAvoidAreaChangedListener.
     * @return WM_OK means unregister success, others means unregister failed.
     */
    virtual WMError UnregisterAvoidAreaChangeListener(const sptr<IAvoidAreaChangedListener>& listener)
    {
        return WMError::WM_OK;
    }

    /**
     * @brief Register window drag listener.
     *
     * @param listener IWindowDragListener.
     * @return WM_OK means register success, others means register failed.
     */
    virtual WMError RegisterDragListener(const sptr<IWindowDragListener>& listener) { return WMError::WM_OK; }

    /**
     * @brief Unregister window drag listener.
     *
     * @param listener IWindowDragListener.
     * @return WM_OK means unregister success, others means unregister failed.
     */
    virtual WMError UnregisterDragListener(const sptr<IWindowDragListener>& listener) { return WMError::WM_OK; }

    /**
     * @brief Register display move listener.
     *
     * @param listener IDisplayMoveListener.
     * @return WM_OK means register success, others means register failed.
     */
    virtual WMError RegisterDisplayMoveListener(sptr<IDisplayMoveListener>& listener) { return WMError::WM_OK; }

    /**
     * @brief Unregister display move listener.
     *
     * @param listener IDisplayMoveListener.
     * @return WM_OK means unregister success, others means unregister failed.
     */
    virtual WMError UnregisterDisplayMoveListener(sptr<IDisplayMoveListener>& listener) { return WMError::WM_OK; }

    /**
     * @brief Register window destroyed listener.
     *
     * @param func Function to notify window destroyed.
     */
    virtual void RegisterWindowDestroyedListener(const NotifyNativeWinDestroyFunc& func) {}

    /**
     * @brief Unregister window destroyed listener.
     *
     */
    virtual void UnregisterWindowDestroyedListener() {}

    /**
     * @brief Register Occupied Area Change listener.
     *
     * @param listener IOccupiedAreaChangeListener.
     * @return WM_OK means register success, others means register failed.
     */
    virtual WMError RegisterOccupiedAreaChangeListener(const sptr<IOccupiedAreaChangeListener>& listener)
    {
        return WMError::WM_OK;
    }

    /**
     * @brief Unregister occupied area change listener.
     *
     * @param listener IOccupiedAreaChangeListener.
     * @return WM_OK means unregister success, others means unregister failed.
     */
    virtual WMError UnregisterOccupiedAreaChangeListener(const sptr<IOccupiedAreaChangeListener>& listener)
    {
        return WMError::WM_OK;
    }

    /**
     * @brief Register touch outside listener.
     *
     * @param listener ITouchOutsideListener.
     * @return WM_OK means register success, others means register failed.
     */
    virtual WMError RegisterTouchOutsideListener(const sptr<ITouchOutsideListener>& listener) { return WMError::WM_OK; }

    /**
     * @brief Unregister touch outside listener.
     *
     * @param listener ITouchOutsideListener.
     * @return WM_OK means unregister success, others means unregister failed.
     */
    virtual WMError UnregisterTouchOutsideListener(const sptr<ITouchOutsideListener>& listener)
    {
        return WMError::WM_OK;
    }

    /**
     * @brief Register Animation Transition Controller listener.
     *
     * @param listener IAnimationTransitionController.
     * @return WM_OK means register success, others means register failed.
     */
    virtual WMError RegisterAnimationTransitionController(const sptr<IAnimationTransitionController>& listener)
    {
        return WMError::WM_OK;
    }

    /**
     * @brief Register screen shot listener.
     *
     * @param listener IScreenshotListener.
     * @return WM_OK means register success, others means register failed.
     */
    virtual WMError RegisterScreenshotListener(const sptr<IScreenshotListener>& listener) { return WMError::WM_OK; }

    /**
     * @brief Unregister screen shot listener.
     *
     * @param listener IScreenshotListener.
     * @return WM_OK means unregister success, others means unregister failed.
     */
    virtual WMError UnregisterScreenshotListener(const sptr<IScreenshotListener>& listener) { return WMError::WM_OK; }

    /**
     * @brief Register screen shot app event listener.
     *
     * @param listener IScreenshotAppEventListener.
     * @return WM_OK means register success, others means register failed.
     */
    virtual WMError RegisterScreenshotAppEventListener(
        const IScreenshotAppEventListenerSptr& listener) { return WMError::WM_ERROR_DEVICE_NOT_SUPPORT; }

    /**
     * @brief Unregister screen shot app event listener.
     *
     * @param listener IScreenshotAppEventListener.
     * @return WM_OK means unregister success, others means unregister failed.
     */
    virtual WMError UnregisterScreenshotAppEventListener(
        const IScreenshotAppEventListenerSptr& listener) { return WMError::WM_ERROR_DEVICE_NOT_SUPPORT; }

    /**
     * @brief Register dialog target touch listener.
     *
     * @param listener IDialogTargetTouchListener.
     * @return WM_OK means register success, others means register failed.
     */
    virtual WMError RegisterDialogTargetTouchListener(const sptr<IDialogTargetTouchListener>& listener)
    {
        return WMError::WM_OK;
    }

    /**
     * @brief Unregister dialog target touch listener.
     *
     * @param listener IDialogTargetTouchListener.
     * @return WM_OK means unregister success, others means unregister failed.
     */
    virtual WMError UnregisterDialogTargetTouchListener(const sptr<IDialogTargetTouchListener>& listener)
    {
        return WMError::WM_OK;
    }

    /**
     * @brief Register dialog death Recipient listener.
     *
     * @param listener IDialogDeathRecipientListener.
     */
    virtual void RegisterDialogDeathRecipientListener(const sptr<IDialogDeathRecipientListener>& listener) {}

    /**
     * @brief Unregister window death recipient listener.
     *
     * @param listener IDialogDeathRecipientListener.
     */
    virtual void UnregisterDialogDeathRecipientListener(const sptr<IDialogDeathRecipientListener>& listener) {}

    /**
     * @brief Notify touch dialog target.
     */
    virtual void NotifyTouchDialogTarget(int32_t posX = 0, int32_t posY = 0) {}

    /**
     * @brief Set ace ability handler.
     *
     * @param handler Ace ability handler.
     */
    virtual void SetAceAbilityHandler(const sptr<IAceAbilityHandler>& handler) {}

    /**
     * @brief set window ui content
     *
     * @param contentInfo content info path
     * @param env
     * @param storage
     * @param isDistributed
     * @param ability
     * @return WMError
     */
    virtual WMError NapiSetUIContent(const std::string& contentInfo, napi_env env, napi_value storage,
        BackupAndRestoreType type = BackupAndRestoreType::NONE, sptr<IRemoteObject> token = nullptr,
        AppExecFwk::Ability* ability = nullptr)
    {
        return WMError::WM_OK;
    }
    virtual WMError AniSetUIContent(const std::string& contentInfo, ani_env* env, ani_object storage,
        BackupAndRestoreType type = BackupAndRestoreType::NONE, sptr<IRemoteObject> token = nullptr,
        AppExecFwk::Ability* ability = nullptr)
    {
        return WMError::WM_OK;
    }

    /**
     * @brief set window ui content
     *
     * @param contentName content info path
     * @param env Napi or ani environment
     * @param storage Napi or ani storage
     * @param type restore type
     * @param token parent token
     * @param ability Ability instance
     * @return WMError
     */
    virtual WMError NapiSetUIContentByName(const std::string& contentName, napi_env env, napi_value storage,
        BackupAndRestoreType type = BackupAndRestoreType::NONE, sptr<IRemoteObject> token = nullptr,
        AppExecFwk::Ability* ability = nullptr) { return WMError::WM_OK; }
    virtual WMError AniSetUIContentByName(const std::string& contentName, ani_env* env, ani_object storage,
        BackupAndRestoreType type = BackupAndRestoreType::NONE, sptr<IRemoteObject> token = nullptr,
        AppExecFwk::Ability* ability = nullptr) { return WMError::WM_OK; }

    /**
     * @brief set window ui content
     *
     * @param contentInfo content info path
     * @param engine
     * @param storage
     * @param isDistributed
     * @param ability
     * @return WMError
     */
    virtual WMError SetUIContentByName(const std::string& contentInfo, napi_env env, napi_value storage,
        AppExecFwk::Ability* ability = nullptr)
    {
        return WMError::WM_OK;
    }

    /**
     * @brief set window ui content by abc
     *
     * @param abcPath abc path
     * @param env
     * @param storage
     * @param ability
     * @return WMError
     */
    virtual WMError SetUIContentByAbc(const std::string& abcPath, napi_env env, napi_value storage,
        AppExecFwk::Ability* ability = nullptr)
    {
        return WMError::WM_OK;
    }

    /**
     * @brief Get ui content info.
     *
     * @return UI content info.
     */
    virtual std::string GetContentInfo(BackupAndRestoreType type = BackupAndRestoreType::CONTINUATION)
    {
        return {};
    }

    /**
     * @brief Set uiability restored router stack.
     *
     * @return WMError.
     */
    virtual WMError SetRestoredRouterStack(const std::string& routerStack)
    {
        return WMError::WM_OK;
    }

    /**
     * @brief Get ui content object.
     *
     * @return UIContent object of ACE.
     */
    virtual Ace::UIContent* GetUIContent() const { return nullptr; }

    /**
     * @brief Get ui content object.
     *
     * @param winId window id.
     * @return UIContent object of ACE.
     */
    virtual Ace::UIContent* GetUIContentWithId(uint32_t winId) const { return nullptr; }

    /**
     * @brief Window handle new want.
     *
     * @param want Want object of AAFwk.
     */
    virtual void OnNewWant(const AAFwk::Want& want) {}

    /**
     * @brief Set requested orientation.
     *
     * @param Orientation Screen orientation.
     * @param animation true means window rotation needs animation. Otherwise not needed.
     */
    virtual void SetRequestedOrientation(Orientation orientation, bool needAnimation = true) {}

    /**
     * @brief Get the Target Orientation ConfigInfo.
     *
     * @param targetOrientation target Orientation.
     * @param targetProperties systemBar target properties
     * @param currentProperties systemBar current properties
     * @param targetViewportConfigAndAvoidArea target viewport config and avoidArea information.
     * @param currentViewportConfigAndAvoidArea current viewport config and avoidArea information.
     * @return WMError
     */
    virtual WMError GetTargetOrientationConfigInfo(Orientation targetOrientation,
        const std::map<WindowType, SystemBarProperty>& targetProperties,
        const std::map<WindowType, SystemBarProperty>& currentProperties,
        ViewportConfigAndAvoidArea& targetViewportConfigAndAvoidArea,
        ViewportConfigAndAvoidArea& currentViewportConfigAndAvoidArea)
    {
        return WMError::WM_ERROR_DEVICE_NOT_SUPPORT;
    }

    /**
     * @brief Register window orientation set by developer
     *
     * @param listener IPreferredOrientationChangeListener.
     * @return WM_OK means register success, others means register failed
     */
    virtual WMError RegisterPreferredOrientationChangeListener(
        const sptr<IPreferredOrientationChangeListener>& listener)
    {
        return WMError::WM_ERROR_DEVICE_NOT_SUPPORT;
    }

    /**
     * @brief Unregister window orientation set by developer
     *
     * @param listener IPreferredOrientationChangeListener.
     * @return WM_OK means register success, others means unregister failed
     */
    virtual WMError UnregisterPreferredOrientationChangeListener(
        const sptr<IPreferredOrientationChangeListener>& listener)
    {
        return WMError::WM_ERROR_DEVICE_NOT_SUPPORT;
    }

    /**
     * @brief Register window orientation change listener
     *
     * @param listener IWindowOrientationChangeListener.
     * @return WM_OK means register success, others means register failed
     */
    virtual WMError RegisterOrientationChangeListener(const sptr<IWindowOrientationChangeListener>& listener)
    {
        return WMError::WM_ERROR_DEVICE_NOT_SUPPORT;
    }

    /**
     * @brief Unregister window orientation change listener
     *
     * @param listener IWindowOrientationChangeListener.
     * @return WM_OK means unregister success, others means unregister failed
     */
    virtual WMError UnregisterOrientationChangeListener(const sptr<IWindowOrientationChangeListener>& listener)
    {
        return WMError::WM_ERROR_DEVICE_NOT_SUPPORT;
    }

    /**
     * @brief Notify caller window orientation set by developer
     *
     * @param orientation Orientation set by developer
     */
    virtual void NotifyPreferredOrientationChange(Orientation orientation) {}

    /**
     * @brief Set developer requested orientation.
     *
     * @param orientation Orientation set by developer
     */
    virtual void SetUserRequestedOrientation(Orientation orientation) {}

    /**
     * @brief Is needed forcibly set orientation.
     *
     * @param orientation Requested orientation.
     */
    virtual bool isNeededForciblySetOrientation(Orientation orientation) { return false; }

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
     * @brief Convert orientation and rotation between window and display
     *
     * @param from The type of the value to be converted.
     * @param to The target type of conversion.
     * @param value The value to be converted.
     * @param convertedValue The converted value.
     * @return WM_OK means convert success, others means convert failed.
     */
    virtual WMError ConvertOrientationAndRotation(const RotationInfoType from, const RotationInfoType to,
        const int32_t value, int32_t& convertedValue)
    {
        return WMError::WM_ERROR_DEVICE_NOT_SUPPORT;
    }

    /**
     * @brief Get requested orientation.
     *
     * @return Orientation screen orientation.
     */
    virtual Orientation GetRequestedOrientation() { return Orientation::UNSPECIFIED; }

    /**
     * @brief Set requested mode support info.
     *
     * @param windowModeSupportType Mode of window supported.
     */
    virtual void SetRequestWindowModeSupportType(uint32_t windowModeSupportType) {}
    /**
     * @brief Get requested mode support info.
     *
     * @return Enumeration values under WindowModeSupport.
     */
    virtual uint32_t GetRequestWindowModeSupportType() const { return 0; }
    /**
     * @brief Set touch hot areas.
     *
     * @param rects Hot areas of touching.
     * @return WM_OK means set success, others means set failed.
     */
    virtual WMError SetTouchHotAreas(const std::vector<Rect>& rects) { return WMError::WM_OK; }

    /**
     * @brief Set keyboard touch hot areas.
     *
     * @param hotAreas keyboard hot areas of touching.
     * @return WM_OK means set success, others means set failed.
     */
    virtual WMError SetKeyboardTouchHotAreas(const KeyboardTouchHotAreas& hotAreas) { return WMError::WM_OK; }

    /**
     * @brief Get requested touch hot areas.
     *
     * @param rects Hot areas of touching.
     */
    virtual void GetRequestedTouchHotAreas(std::vector<Rect>& rects) const {}

    /**
     * @brief Main handler available or not.
     *
     * @return True means main handler is available, false means the opposite.
     */
    virtual bool IsMainHandlerAvailable() const { return false; }

    /**
     * @brief Set window label name.
     *
     * @param label Window label name.
     * @return WM_OK means set success, others means set failed.
     */
    virtual WMError SetAPPWindowLabel(const std::string& label) { return WMError::WM_OK; }

    /**
     * @brief Set window icon.
     *
     * @param icon Window icon.
     * @return WM_OK means set success, others means set failed.
     */
    virtual WMError SetAPPWindowIcon(const std::shared_ptr<Media::PixelMap>& icon) { return WMError::WM_OK; }

    /**
     * @brief disable main window decoration. It must be callled before loadContent.
     *
     */
    virtual WMError DisableAppWindowDecor() { return WMError::WM_OK; }

    /**
     * @brief return window decoration is enabled. It is called by ACE
     *
     * @return true means window decoration is enabled. Otherwise disabled
     */
    virtual bool IsDecorEnable() const { return false; }

    /**
     * @brief maximize the main window. It is called by ACE when maximize button is clicked.
     *
     * @return WMError
     */
    virtual WMError Maximize() { return WMError::WM_OK; }

    /**
     * @brief maximize window with presentation enum.
     *
     * @param presentation the value means use presentation enum to layout when maximize window
     * @return WM_OK means maximize window ok, others means failed.
     */
    virtual WMError Maximize(MaximizePresentation presentation)
    {
        return WMError::WM_ERROR_DEVICE_NOT_SUPPORT;
    }

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

    /**
     * @brief maximize the main window according to MaximizeMode. called by ACE when maximize button is clicked.
     *
     * @return WMError
     */
    virtual WMError MaximizeFloating() {return WMError::WM_OK;}

    /**
     * @brief minimize the main window. It is called by ACE when minimize button is clicked.
     *
     * @return WMError
     */
    virtual WMError Minimize() { return WMError::WM_ERROR_DEVICE_NOT_SUPPORT; }

    /**
     * @brief recovery the main window. It is called by ACE when recovery button is clicked.
     *
     * @return WMError
     */
    virtual WMError Recover() { return WMError::WM_OK; }

    /**
     * @brief After the app main window is minimized, if the Ability is not in the backgroud state,
     * you can restore app main window.
     *
     * @return WMError
     */
    virtual WMError Restore() { return WMError::WM_ERROR_DEVICE_NOT_SUPPORT; }

    /**
     * @brief Restores the main window of current window to foreground.
     * Only TYPE_FLOAT can use this interface, when the main window in the background
     * need to be moved to foreground after the TYPE_FLOAT window is clicked
     *
     * @return WMError
     */
    virtual WMError RestoreMainWindow(const std::shared_ptr<AAFwk::WantParams>& wantParams)
    {
        return WMError::WM_ERROR_DEVICE_NOT_SUPPORT;
    }

    /**
     * @brief close the window. It is called by ACE when close button is clicked.
     *
     * @return WMError
     */
    virtual WMError Close() { return WMError::WM_OK; }

    /**
     * @brief close the window. There is no pre-close process.
     *
     * @return WMError
     */
    virtual WMError CloseDirectly() { return WMError::WM_ERROR_DEVICE_NOT_SUPPORT; }

    /**
     * @brief notify rotation change result.
     */
    virtual void NotifyRotationChangeResult(RotationChangeResult rotationChangeResult) {}

    /**
     * @brief start move main window. It is called by ACE when title is moved.
     *
     */
    virtual void StartMove() {}

    /**
     * @brief get main window move flag.
     *
     * @return true means main window is moving. Otherwise is not moving.
     */
    virtual bool IsStartMoving() { return false; }

    /**
     * @brief Start moving window. It is called by application.
     *
     * @return Errorcode of window.
     */
    virtual WmErrorCode StartMoveWindow() { return WmErrorCode::WM_ERROR_DEVICE_NOT_SUPPORT; }

    /**
     * @brief Start moving window. It is called by application.
     *
     * @param offsetX expected pointer position x-axis offset in window when start moving.
     * @param offsetY expected pointer position y-axis offset in window when start moving.
     * @return Error code of window.
     */
    virtual WmErrorCode StartMoveWindowWithCoordinate(int32_t offsetX,
        int32_t offsetY) { return WmErrorCode::WM_ERROR_DEVICE_NOT_SUPPORT; }

    /**
     * @brief Stop moving window. It is called by application. Support pc window and pad free multi-window.
     *
     * @return Error code of window.
     */
    virtual WmErrorCode StopMoveWindow() { return WmErrorCode::WM_ERROR_DEVICE_NOT_SUPPORT; }

    /**
     * @brief Set flag that need remove window input channel.
     *
     * @param needRemoveWindowInputChannel True means remove input channel, false means not remove.
     */
    virtual void SetNeedRemoveWindowInputChannel(bool needRemoveWindowInputChannel) {}

    /**
     * @brief set global window maximize mode. It is called by ACE when maximize mode changed.
     *
     * @param mode MODE_AVOID_SYSTEM_BAR - avoid statusbar and dockbar; MODE_FULL_FILL - fullfill the screen
     *
     * @return WMError
     */
    virtual WMError SetGlobalMaximizeMode(MaximizeMode mode) {return WMError::WM_OK;}

    /**
     * @brief get global window maximize mode.
     *
     * @return MaximizeMode
     */
    virtual MaximizeMode GetGlobalMaximizeMode() const {return MaximizeMode::MODE_FULL_FILL;}

    // colorspace, gamut
    /**
     * @brief Is support wide gamut or not.
     *
     * @return True means support wide gamut, false means not support.
     */
    virtual bool IsSupportWideGamut() { return false; }

    /**
     * @brief Set color space.
     *
     * @param colorSpace ColorSpace object.
     */
    virtual void SetColorSpace(ColorSpace colorSpace) {}

    /**
     * @brief Get color space object.
     *
     * @return ColorSpace object.
     */
    virtual ColorSpace GetColorSpace() { return ColorSpace::COLOR_SPACE_DEFAULT; }

    virtual void DumpInfo(const std::vector<std::string>& params, std::vector<std::string>& info) {}

    /**
     * @brief window snapshot
     *
     * @return std::shared_ptr<Media::PixelMap> snapshot pixel
     */
    virtual std::shared_ptr<Media::PixelMap> Snapshot() { return nullptr; }

    /**
     * @brief window snapshot
     *
     * @param pixelMap pixel map
     * @return the error code of this operation
     */
    virtual WMError Snapshot(
        std::shared_ptr<Media::PixelMap>& pixelMap) { return WMError::WM_ERROR_DEVICE_NOT_SUPPORT; }

    /**
     * @brief window SnapshotIgnorePrivacy
     *
     * @param pixelMap pixel map
     * @return the error code of this operation
     */
    virtual WMError SnapshotIgnorePrivacy(std::shared_ptr<Media::PixelMap>& pixelMap) { return WMError::WM_OK; }

    /**
     * @brief Handle and notify memory level.
     *
     * @param level memory level
     * @return the error code of window
     */
    virtual WMError NotifyMemoryLevel(int32_t level) { return WMError::WM_OK; }

    /**
     * @brief Update configuration for all windows
     *
     * @param configuration configuration for app
     */
    virtual bool IsAllowHaveSystemSubWindow() { return false; }

    /**
     * @brief Set aspect ratio of this window
     *
     * @param ratio the aspect ratio of window except decoration
     * @return WMError
     */
    virtual WMError SetAspectRatio(float ratio) { return WMError::WM_OK; }

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

    /**
     * @brief Unset aspect ratio
     * @return WMError
     */
    virtual WMError ResetAspectRatio() { return WMError::WM_OK; }

    /**
     * @brief Get keyboard animation config
     * @return KeyboardAnimationConfig
     */
    virtual KeyboardAnimationConfig GetKeyboardAnimationConfig() { return {}; }

    /**
     * @brief Set need default animation for window show and hide.
     *
     * @param needDefaultAnimation True means need default animation, false means not need.
     */
    virtual void SetNeedDefaultAnimation(bool needDefaultAnimation) {}

    /**
     * @brief Transfer Ability Result.
     * @return WMError
     */
    virtual WMError TransferAbilityResult(uint32_t resultCode, const AAFwk::Want& want) { return WMError::WM_OK; }

    /**
     * @brief Transfer UIExtension data to Extension Component.
     * @return WMError
     */
    virtual WMError TransferExtensionData(const AAFwk::WantParams& wantParams) { return WMError::WM_OK; }

    /**
     * @brief Register transfer component data callback.
     *
     * @param func Function to notify transfer component data.
     */
    virtual void RegisterTransferComponentDataListener(const NotifyTransferComponentDataFunc& func) {}

    /**
     * @brief Trigger BindUIExtensionModal callback.
     *        It needs to be called when the UIExtension frame node is set to asynchronously bind to the modal window.
     */
    virtual void TriggerBindModalUIExtension() {}

    /**
     * @brief Perform back event.
     *
     */
    virtual void PerformBack() {}

    /**
     * @brief Set the drag enabled flag of a window.
     *
     * @param dragEnabled true means the window can be resized by dragging, otherwise means the opposite.
     * @return Errorcode of window.
     */
    virtual WMError SetResizeByDragEnabled(bool dragEnabled) { return WMError::WM_ERROR_DEVICE_NOT_SUPPORT; }

    /**
     * @brief Set the raise enabled flag of a window.
     *
     * @param raiseEnabled true means the window can be raised by click, otherwise means the opposite.
     * @return Errorcode of window.
     */
    virtual WMError SetRaiseByClickEnabled(bool raiseEnabled) { return WMError::WM_ERROR_DEVICE_NOT_SUPPORT; }

    /**
     * @brief Set the raise enabled flag of a main window.
     *
     * @param raiseEnabled true means the main window can be raised by click, otherwise means the opposite.
     * @return Errorcode of window.
     */
    virtual WMError SetMainWindowRaiseByClickEnabled(bool raiseEnabled) { return WMError::WM_ERROR_DEVICE_NOT_SUPPORT; }

    /**
     * @brief Raise one app sub window above another.
     *
     * @return WM_OK means raise success, others means raise failed.
     */
    virtual WMError RaiseAboveTarget(int32_t subWindowId) { return WMError::WM_ERROR_DEVICE_NOT_SUPPORT; }

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
     * @brief Hide non-system floating windows.
     *
     * @param shouldHide true means the non-system windows should be hidden, otherwise means the opposite.
     * @return Errorcode of window.
     */
    virtual WMError HideNonSystemFloatingWindows(bool shouldHide) { return WMError::WM_ERROR_DEVICE_NOT_SUPPORT; }

    /**
     * @brief Is floating window of app type or not.
     *
     * @return True means floating window of app type, false means the opposite.
     */
    virtual bool IsFloatingWindowAppType() const { return false; }

    /**
     * @brief Is pc window or not.
     *
     * @return True means pc window, false means the opposite.
     */
    virtual bool IsPcWindow() const { return false; }

    /**
     * @brief Is pad window or not.
     *
     * @return True means pad window, false means the opposite.
     */
    virtual bool IsPadWindow() const { return false; }

    /**
     * @brief Is pc window or free multi window capility enabled or not.
     *
     * @return True means pc window or free multi window capility enabled, false means the opposite.
     */
    virtual bool IsPcOrFreeMultiWindowCapabilityEnabled() const { return false; }

    /**
     * @brief Is phone, pad, pc window or not.
     *
     * @return True means phone, pad or pc window, false means the opposite.
     */
    virtual bool IsPhonePadOrPcWindow() const { return false; }

    /**
     * @brief Is pc window or pad free multi-window.
     *
     * @return True means pc window or pad free multi-window, false means the opposite.
     */
    virtual bool IsPcOrPadFreeMultiWindowMode() const { return false; }

    /**
     * @brief Is pcAppInpad and pad window.
     *
     * @return True means pcAppInpad and pad window., false means the opposite.
     */
    virtual bool IsPadAndNotFreeMultiWindowCompatibleMode() const { return false; }

    /**
     * @brief Judge whether SceneBoard is enabled.
     *
     * @return True means SceneBoard is enabled, false means the opposite.
     */
    virtual bool IsSceneBoardEnabled() const { return false; }

    /**
     * @brief get compatible mode in pc.
     * @deprecated use IsAdaptToImmersive instead
     *
     * @return True means window is compatible mode in pc, false means the opposite.
     */
    virtual bool GetCompatibleModeInPc() const { return false; }

    /**
     * @brief Register transfer component data callback.
     *
     * @param func Function to notify transfer component data.
     */
    virtual void RegisterTransferComponentDataForResultListener(const NotifyTransferComponentDataForResultFunc& func) {}

    /**
     * @brief Set Text Field Avoid Info.
     *
     * @return Errorcode of window.
     */
    virtual WMError SetTextFieldAvoidInfo(double textFieldPositionY, double textFieldHeight) { return WMError::WM_OK; }

    /**
     * @brief Transfer accessibility event data
     *
     * @param func Function to notify transfer component data.
     */
    virtual WMError TransferAccessibilityEvent(const Accessibility::AccessibilityEventInfo& info,
        int64_t uiExtensionIdLevel) { return WMError::WM_OK; };

    /**
     * @brief Notify prepare to close window
     *
     * @return Errorcode of window.
     */
    virtual WMError NotifyPrepareClosePiPWindow() { return WMError::WM_OK; }

    /**
     * @brief update the pip window instance (w,h,r).
     *
     * @param width width of pip window.
     * @param height width of pip window.
     * @param reason reason of update.
     */
    virtual void UpdatePiPRect(const Rect& rect, WindowSizeChangeReason reason) {}

    /**
     * @brief update the pip control status.
     *
     * @param controlType pip control type.
     * @param status pip control status.
     */
    virtual void UpdatePiPControlStatus(PiPControlType controlType, PiPControlStatus status) {}

    /**
     * @brief set auto start status for window.
     *
     * @param isAutoStart true means auto start pip window when background, otherwise means the opposite.
     * @param priority 1 means height priority, 0 means low priority.
     * @param width width means width of the video content.
     * @param height height means height of the video content.
     */
    virtual void SetAutoStartPiP(bool isAutoStart, uint32_t priority, uint32_t width, uint32_t height) {}

    /**
     * @brief When get focused, keep the keyboard created by other windows, support system window and app subwindow.
     *
     * @param keepKeyboardFlag true means the keyboard should be preserved, otherwise means the opposite.
     * @return WM_OK means set keep keyboard flag success, others means failed.
     */
    virtual WmErrorCode KeepKeyboardOnFocus(bool keepKeyboardFlag)
    {
        return WmErrorCode::WM_ERROR_DEVICE_NOT_SUPPORT;
    }

    /**
     * @brief Register window visibility change listener.
     *
     * @param listener IWindowVisibilityChangedListener.
     * @return WM_OK means register success, others means register failed.
     */
    virtual WMError RegisterWindowVisibilityChangeListener(const IWindowVisibilityListenerSptr& listener)
    {
        return WMError::WM_ERROR_DEVICE_NOT_SUPPORT;
    }

    /**
     * @brief Unregister window visibility change listener.
     *
     * @param listener IWindowVisibilityChangedListener.
     * @return WM_OK means unregister success, others means unregister failed.
     */
    virtual WMError UnregisterWindowVisibilityChangeListener(const IWindowVisibilityListenerSptr& listener)
    {
        return WMError::WM_ERROR_DEVICE_NOT_SUPPORT;
    }

    /**
     * @brief Register window occlusion state change listener.
     *
     * @param listener IOcclusionStateChangedListener.
     * @return WM_OK means register success, others means register failed.
     */
    virtual WMError RegisterOcclusionStateChangeListener(const sptr<IOcclusionStateChangedListener>& listener)
    {
        return WMError::WM_ERROR_DEVICE_NOT_SUPPORT;
    }

    /**
     * @brief Unregister window occlusion state change listener.
     *
     * @param listener IOcclusionStateChangedListener.
     * @return WM_OK means unregister success, others means unregister failed.
     */
    virtual WMError UnregisterOcclusionStateChangeListener(const sptr<IOcclusionStateChangedListener>& listener)
    {
        return WMError::WM_ERROR_DEVICE_NOT_SUPPORT;
    }

    /**
     * @brief Register window frame metrics change listener.
     *
     * @param listener IFrameMetricsChangedListener.
     * @return WM_OK means register success, others means register failed.
     */
    virtual WMError RegisterFrameMetricsChangeListener(const sptr<IFrameMetricsChangedListener>& listener)
    {
        return WMError::WM_ERROR_DEVICE_NOT_SUPPORT;
    }

    /**
     * @brief Unregister window frame metrics change listener.
     *
     * @param listener IFrameMetricsChangedListener.
     * @return WM_OK means unregister success, others means unregister failed.
     */
    virtual WMError UnregisterFrameMetricsChangeListener(const sptr<IFrameMetricsChangedListener>& listener)
    {
        return WMError::WM_ERROR_DEVICE_NOT_SUPPORT;
    }

    /**
     * @brief Register window displayId change listener.
     *
     * @param listener IDisplayIdChangedListener.
     * @return WM_OK means register success, others means register failed.
     */
    virtual WMError RegisterDisplayIdChangeListener(
        const IDisplayIdChangeListenerSptr& listener) { return WMError::WM_ERROR_DEVICE_NOT_SUPPORT; }

    /**
     * @brief Unregister window displayId change listener.
     *
     * @param listener IDisplayIdChangedListener.
     * @return WM_OK means unregister success, others means unregister failed.
     */
    virtual WMError UnregisterDisplayIdChangeListener(
        const IDisplayIdChangeListenerSptr& listener) { return WMError::WM_ERROR_DEVICE_NOT_SUPPORT; }

    /**
     * @brief Register system density change listener.
     *
     * @param listener ISystemDensityChangedListener.
     * @return WM_OK means register success, others means register failed.
     */
    virtual WMError RegisterSystemDensityChangeListener(
        const ISystemDensityChangeListenerSptr& listener) { return WMError::WM_ERROR_DEVICE_NOT_SUPPORT; }

    /**
     * @brief Unregister system density change listener.
     *
     * @param listener ISystemDensityChangedListener.
     * @return WM_OK means unregister success, others means unregister failed.
     */
    virtual WMError UnregisterSystemDensityChangeListener(
        const ISystemDensityChangeListenerSptr& listener) { return WMError::WM_ERROR_DEVICE_NOT_SUPPORT; }

    /**
     * @brief Register main window full screen across multi display change listener.
     *
     * @param listener IAcrossDisplaysChangeListener.
     * @return WM_OK means register success, others means register failed.
     */
    virtual WMError RegisterAcrossDisplaysChangeListener(
        const IAcrossDisplaysChangeListenerSptr& listener) { return WMError::WM_ERROR_DEVICE_NOT_SUPPORT; }

    /**
     * @brief Unregister main window full screen across multi display change listener.
     *
     * @param listener IAcrossDisplaysChangeListener.
     * @return WM_OK means unregister success, others means unregister failed.
     */
    virtual WMError UnRegisterAcrossDisplaysChangeListener(
        const IAcrossDisplaysChangeListenerSptr& listener) { return WMError::WM_ERROR_DEVICE_NOT_SUPPORT; }

    /**
     * @brief Get the window limits of current window.
     *
     * @param windowLimits.
     * @param getVirtualPixel Returns windowLimits in virtual pixels if the param is true, otherwise in physical pixels.
     * @return WMError.
     */
    virtual WMError GetWindowLimits(WindowLimits& windowLimits, bool getVirtualPixel = false)
    {
        return WMError::WM_ERROR_DEVICE_NOT_SUPPORT;
    }

    /**
     * @brief Set the window limits of current window.
     *
     * @param windowLimits.
     * @return WMError.
     */
    virtual WMError SetWindowLimits(WindowLimits& windowLimits, bool isForcible = false)
    {
        return WMError::WM_ERROR_DEVICE_NOT_SUPPORT;
    }

    /**
     * @brief Register listener, if timeout(seconds) pass with no interaction, the listener will be executed.
     *
     * @param listener IWindowNoInteractionListenerSptr.
     * @return WM_OK means unregister success, others means unregister failed.
     */
    virtual WMError RegisterWindowNoInteractionListener(const IWindowNoInteractionListenerSptr& listener)
    {
        return WMError::WM_ERROR_DEVICE_NOT_SUPPORT;
    }

    /**
     * @brief Unregister window no interaction listener.
     *
     * @param listener IWindowNoInteractionListenerSptr.
     * @return WM_OK means unregister success, others means unregister failed.
     */
    virtual WMError UnregisterWindowNoInteractionListener(const IWindowNoInteractionListenerSptr& listener)
    {
        return WMError::WM_ERROR_DEVICE_NOT_SUPPORT;
    }

    /**
     * @brief Register window status change listener.
     *
     * @param listener IWindowStatusChangeListener.
     * @return WM_OK means register success, others means register failed.
     */
    virtual WMError RegisterWindowStatusChangeListener(const sptr<IWindowStatusChangeListener>& listener)
    {
        return WMError::WM_ERROR_DEVICE_NOT_SUPPORT;
    }

    /**
     * @brief Unregister window status change listener.
     *
     * @param listener IWindowStatusChangeListener.
     * @return WM_OK means unregister success, others means unregister failed.
     */
    virtual WMError UnregisterWindowStatusChangeListener(const sptr<IWindowStatusChangeListener>& listener)
    {
        return WMError::WM_ERROR_DEVICE_NOT_SUPPORT;
    }

    /**
     * @brief Register window status change listener.
     *
     * @param listener IWindowStatusDidChangeListener.
     * @return WM_OK means register success, others means register failed.
     */
    virtual WMError RegisterWindowStatusDidChangeListener(const sptr<IWindowStatusDidChangeListener>& listener)
    {
        return WMError::WM_ERROR_DEVICE_NOT_SUPPORT;
    }

    /**
     * @brief Unregister window status change listener.
     *
     * @param listener IWindowStatusDidChangeListener.
     * @return WM_OK means unregister success, others means unregister failed.
     */
    virtual WMError UnregisterWindowStatusDidChangeListener(const sptr<IWindowStatusDidChangeListener>& listener)
    {
        return WMError::WM_ERROR_DEVICE_NOT_SUPPORT;
    }

    /**
     * @brief Set Specific System Bar(include status bar and nav bar) Enable and Animation Properties
     *
     * @param systemBarEnable is system bar enabled
     * @param systemBarEnableAnimation is animation enabled
     * @param SystemBarProperty WINDOW_TYPE_STATUS_BAR or WINDOW_TYPE_NAVIGATION_BAR
     */
    virtual void UpdateSpecificSystemBarEnabled(bool systemBarEnable, bool systemBarEnableAnimation,
        SystemBarProperty& property) {}

    /**
     * @brief Set Specific System Bar(include status bar and nav bar) Enable and Animation Properties
     *
     * @param name type of system bar
     * @param enable is system bar enabled
     * @param enableAnimation is animation enabled
     */
    virtual WMError UpdateHostSpecificSystemBarEnabled(const std::string& name, bool enable, bool enableAnimation)
    {
        return WMError::WM_OK;
    }

    /*
     * @brief Update SystemBar Property For Page
     *
     * @param type type of system bar
     * @param property System Bar Property
     * @param systemBarPropertyFlag is System Bar Property Flag
     */
    virtual WMError UpdateSystemBarPropertyForPage(WindowType type,
        const SystemBarProperty& systemBarProperty, const SystemBarPropertyFlag& systemBarPropertyFlag)
    {
        return WMError::WM_OK;
    }

    /*
     * @brief Set System Bar Property for page
     *
     * @param type WINDOW_TYPE_STATUS_BAR or WINDOW_TYPE_NAVIGATION_INDICATOR
     * @param property System Bar Property
     * @return WMError
     */
    virtual WMError SetSystemBarPropertyForPage(WindowType type, std::optional<SystemBarProperty> property)
    {
        return WMError::WM_OK;
    }

    /*
     * @brief Set Status Bar Color For Page
     *
     * @param color Status Bar Color
     * @return WMError
     */
    virtual WMError SetStatusBarColorForPage(const std::optional<uint32_t> color)
    {
        return WMError::WM_OK;
    }

    /**
     * @brief Set Specific System Bar(include status bar and nav bar) Property
     *
     * @param type WINDOW_TYPE_STATUS_BAR or WINDOW_TYPE_NAVIGATION_BAR
     * @param property system bar prop,include content color, background color
     * @return WMError
     */
    virtual WMError SetSpecificBarProperty(WindowType type, const SystemBarProperty& property)
    {
        return WMError::WM_OK;
    }

    /**
     * @brief Set System Bar(include status bar and nav bar) Properties
     *
     * @param properties system bar properties
     * @param propertyFlags flags of system bar property
     * @return WMError
     */
    virtual WMError SetSystemBarProperties(const std::map<WindowType, SystemBarProperty>& properties,
        const std::map<WindowType, SystemBarPropertyFlag>& propertyFlags)
    {
        return WMError::WM_OK;
    }

    /**
     * @brief Get System Bar(include status bar and nav bar) Properties
     *
     * @param properties system bar properties got
     * @return WMError
     */
    virtual WMError GetSystemBarProperties(std::map<WindowType, SystemBarProperty>& properties)
    {
        return WMError::WM_OK;
    }

    /**
     * @brief Update System Bar (include status bar and nav bar) Properties by Flags
     *
     * @param systemBarProperties map of status bar and nav bar properties
     * @param systemBarPropertyFlags map of status bar and nav bar properties to be changed
     * @return WMError
     */
    virtual WMError UpdateSystemBarProperties(
        const std::unordered_map<WindowType, SystemBarProperty>& systemBarProperties,
        const std::unordered_map<WindowType, SystemBarPropertyFlag>& systemBarPropertyFlags)
    {
        return WMError::WM_OK;
    }

    /**
     * @brief Set the single frame composer enabled flag of a window.
     *
     * @param enable true means the single frame composer is enabled, otherwise means the opposite.
     * @return Errorcode of window.
     */
    virtual WMError SetSingleFrameComposerEnabled(bool enable) { return WMError::WM_ERROR_DEVICE_NOT_SUPPORT; }

    /**
     * @brief Set the visibility of window decor.
     *
     * @param isVisible whether the window decor is visible.
     * @return Errorcode of window.
     */
    virtual WMError SetDecorVisible(bool isVisible) { return WMError::WM_ERROR_DEVICE_NOT_SUPPORT; }

    /**
     * @brief Get the visibility of window decor.
     *
     * @param isVisible whether the window decor is visible.
     * @return Errorcode of window.
     */
    virtual WMError GetDecorVisible(bool& isVisible) { return WMError::WM_ERROR_DEVICE_NOT_SUPPORT; }

    /**
     * @brief Enable or disable move window by title bar.
     *
     * @param enable The value true means to enable window moving, and false means the opposite.
     * @return Errorcode of window.
     */
    virtual WMError SetWindowTitleMoveEnabled(bool enable) { return WMError::WM_ERROR_DEVICE_NOT_SUPPORT; }

    /**
     * @brief Set window container color.
     *
     * @param activeColor Background active color.
     * @param inactiveColor Background active color.
     * @return Errorcode of window.
     */
    virtual WMError SetWindowContainerColor(const std::string& activeColor, const std::string& inactiveColor)
    {
        return WMError::WM_ERROR_DEVICE_NOT_SUPPORT;
    }

    /**
     * @brief Set main window container color.
     *
     * @param activeColor Background active color.
     * @param inactiveColor Background active color.
     * @return Errorcode of window.
     */
    virtual WMError SetWindowContainerModalColor(const std::string& activeColor, const std::string& inactiveColor)
    {
        return WMError::WM_ERROR_DEVICE_NOT_SUPPORT;
    }

    /**
     * @brief Enable drag window.
     *
     * @param enableDrag The value true means to enable window dragging, and false means the opposite.
     * @return Errorcode of window.
     */
    virtual WMError EnableDrag(bool enableDrag) { return WMError::WM_ERROR_DEVICE_NOT_SUPPORT; }

    /**
     * @brief Set whether to display the maximize, minimize, split buttons of main window.
     *
     * @param isMaximizeVisible Display maximize button if true, or hide maximize button if false.
     * @param isMinimizeVisible Display minimize button if true, or hide minimize button if false.
     * @param isSplitVisible Display split button if true, or hide split button if false.
     * @param isCloseVisible Display close button if true, or hide close button if false.
     * @return Errorcode of window.
     */
    virtual WMError SetTitleButtonVisible(bool isMaximizeVisible, bool isMinimizeVisible, bool isSplitVisible,
        bool isCloseVisible)
    {
        return WMError::WM_ERROR_DEVICE_NOT_SUPPORT;
    }

    /**
     * @brief Set Window new title
     *
     * @param title Window new title
     * @return Errorcode of window.
     */
    virtual WMError SetWindowTitle(const std::string& title) { return WMError::WM_ERROR_DEVICE_NOT_SUPPORT; }

    /**
     * @brief Set decor height of window.
     *
     * @param decorHeight Decor height of window
     * @return WM_OK means set success, others means set failed.
     */
    virtual WMError SetDecorHeight(int32_t decorHeight) { return WMError::WM_ERROR_DEVICE_NOT_SUPPORT; }

    /**
     * @brief Get decor height of window.
     *
     * @return Decor height of window.
     */
    virtual WMError GetDecorHeight(int32_t& height) { return WMError::WM_ERROR_DEVICE_NOT_SUPPORT; }

    /**
     * @brief Set decor button style of window.
     *
     * @param style Decor style of the window
     * @return WM_OK means set success, others means set failed.
     */
    virtual WMError SetDecorButtonStyle(const DecorButtonStyle& style)
    {
        return WMError::WM_ERROR_DEVICE_NOT_SUPPORT;
    }

    /**
     * @brief Get decor button style of window.
     *
     * @param style Decor style of the window
     * @return WM_OK means set success, others means set failed.
     */
    virtual WMError GetDecorButtonStyle(DecorButtonStyle& style) { return WMError::WM_ERROR_DEVICE_NOT_SUPPORT; }

    /**
     * @brief Get the title buttons area of window.
     *
     * @param titleButtonRect.
     * @return WMError.
     */
    virtual WMError GetTitleButtonArea(TitleButtonRect& titleButtonRect)
    {
        return WMError::WM_ERROR_DEVICE_NOT_SUPPORT;
    }

    /**
     * @brief Register window title buttons change listener.
     *
     * @param listener IWindowTitleButtonRectChangedListener.
     * @return WM_OK means register success, others means register failed.
     */
    virtual WMError RegisterWindowTitleButtonRectChangeListener(
        const sptr<IWindowTitleButtonRectChangedListener>& listener)
    {
        return WMError::WM_ERROR_DEVICE_NOT_SUPPORT;
    }

    /**
     * @brief Unregister window title buttons change listener.
     *
     * @param listener IWindowTitleButtonRectChangedListener.
     * @return WM_OK means unregister success, others means unregister failed.
     */
    virtual WMError UnregisterWindowTitleButtonRectChangeListener(
        const sptr<IWindowTitleButtonRectChangedListener>& listener)
    {
        return WMError::WM_ERROR_DEVICE_NOT_SUPPORT;
    }

    /**
     * @brief Set whether to use default density.
     *
     * @param enabled bool.
     * @return WM_OK means set success, others means failed.
     */
    virtual WMError SetDefaultDensityEnabled(bool enabled) { return WMError::WM_ERROR_DEVICE_NOT_SUPPORT; }

    /**
     * @brief Get whether to use default density.
     *
     * @return True means use default density, window's layout not follow to system change, false means the opposite.
     */
    virtual bool GetDefaultDensityEnabled() { return false; }

    /**
     * @brief Set custom density of window.
     *
     * @param density the custom density of window.
     * @return WM_OK means set success, others means failed.
     */
    virtual WMError SetCustomDensity(
        float density, bool applyToSubWindow) { return WMError::WM_ERROR_DEVICE_NOT_SUPPORT; }

    /**
     * @brief Get custom density of window.
     *
     * @return custom density.
     */
    virtual float GetCustomDensity() const { return UNDEFINED_DENSITY; }

    /**
     * @brief UIExtension window call to set custom density, once called this method to set custom density,
     * UIExtension window will dinore FOLLOW_HOST_DPI and use specified density.
     *
     * @param density the custom density of UIExtension window.
     * @return WM_OK means set success, others means failed.
     */
    virtual WMError SetUIExtCustomDensity(const float density) { return WMError::WM_OK; }

    /**
     * @brief Get the window density of current window.
     *
     * @param densityInfo the struct representing system density, default density and custom density.
     * @return WMError.
     */
    virtual WMError GetWindowDensityInfo(WindowDensityInfo& densityInfo) { return WMError::WM_OK; }

    /**
     * @brief Set whether the current window follows the default density of its screen.
     *
     * @param enabled bool.
     * @return WM_OK means set success, others means failed.
     */
    virtual WMError SetWindowDefaultDensityEnabled(bool enabled) { return WMError::WM_ERROR_DEVICE_NOT_SUPPORT; }

    /**
     * @brief Get virtual pixel ratio.
     *
     * @return Value of PixelRatio obtained from displayInfo.
     */
    virtual float GetVirtualPixelRatio() { return 1.0f; }

    /**
     * @brief Hide None Secure Windows.
     *
     * @param shouldHide bool.
     * @return WMError
     */
    virtual WMError HideNonSecureWindows(bool shouldHide)
    {
        return WMError::WM_ERROR_DEVICE_NOT_SUPPORT;
    }

    /**
     * @brief Set water mark flag.
     *
     * @param isEnable bool.
     * @return WMError
     */
    virtual WMError SetWaterMarkFlag(bool isEnable)
    {
        return WMError::WM_ERROR_DEVICE_NOT_SUPPORT;
    }

    /**
     * @brief Hide the display content when snapshot.
     *
     * @param needHide bool.
     * @return WMError
     */
    virtual WMError HidePrivacyContentForHost(bool needHide)
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
     * @brief recovery the main window by function overloading. It is called by JsWindow.
     *
     * @param reason reason of update.
     * @return WMError
     */
    virtual WMError Recover(uint32_t reason) { return WMError::WM_ERROR_DEVICE_NOT_SUPPORT; }

    /**
     * @brief Set to automatically save the window rect.
     *
     * @param enabled Enable the window rect auto-save if true, otherwise means the opposite.
     * @return WM_OK means set success, others means failed.
     */
    virtual WMError SetWindowRectAutoSave(bool enabled,
        bool isSaveBySpecifiedFlag = false) { return WMError::WM_ERROR_DEVICE_NOT_SUPPORT; }

    /**
     * @brief Get whether the auto-save the window rect is enabled or not.
     *
     * @param enabled True means the window rect auto-save is enabled, otherwise means the opposite.
     * @return WM_OK means set success, others means failed.
     */
    virtual WMError IsWindowRectAutoSave(bool& enabled) { return WMError::WM_ERROR_DEVICE_NOT_SUPPORT; }

    /**
     * @brief Sets the supported window modes.
     *
     * @param supportedWindowModes Supported window modes of the window.
     * @param grayOutMaximizeButton Whether to gray out the window maximize button.
                                    The value true means to gray out the button, and false means the opposite.
     * @return WM_OK means set success, others means failed.
     */
    virtual WMError SetSupportedWindowModes(const std::vector<AppExecFwk::SupportWindowMode>& supportedWindowModes,
        bool grayOutMaximizeButton = false)
    {
        return WMError::WM_ERROR_DEVICE_NOT_SUPPORT;
    }

    /**
     * @brief Set whether the sub window supports simultaneous display on multiple screens
     *        when the parent window is dragged to move or dragged to zoom.
     *
     * @param enabled The value true means sub window supports simultaneous display on multiple screens
     *                when the parent window is dragged to move or dragged to zoom, and false means the opposite.
     * @return WM_OK means set success, others means failed.
     */
    virtual WMError SetFollowParentMultiScreenPolicy(bool enabled) { return WMError::WM_ERROR_DEVICE_NOT_SUPPORT;}

    /**
     * @brief Get the rect of host window.
     *
     * @param hostWindowId window Id of the host window.
     * @return Rect of window.
     */
    virtual Rect GetHostWindowRect(int32_t hostWindowId) { return {}; }

    /**
     * @brief Make multi-window become landscape or not.
     *
     * @param isLandscapeMultiWindow means whether multi-window's scale is landscape.
     * @return WMError WM_OK means set success, others means failed.
     */
    virtual WMError SetLandscapeMultiWindow(bool isLandscapeMultiWindow) { return WMError::WM_OK; }

    /**
     * @brief Register subwindow close listener.
     *
     * @param listener ISubWindowCloseListener.
     * @return WM_OK means register success, others means register failed.
     */
    virtual WMError RegisterSubWindowCloseListeners(
        const sptr<ISubWindowCloseListener>& listener) { return WMError::WM_ERROR_DEVICE_NOT_SUPPORT; }

    /**
     * @brief Unregister subwindow close listener.
     *
     * @param listener ISubWindowCloseListeners.
     * @return WM_OK means unregister success, others means unregister failed.
     */
    virtual WMError UnregisterSubWindowCloseListeners(
        const sptr<ISubWindowCloseListener>& listener) { return WMError::WM_ERROR_DEVICE_NOT_SUPPORT; }

    /**
     * @brief Register main window close listener.
     *
     * @param listener IMainWindowCloseListener.
     * @return WM_OK means register success, others means register failed.
     */
    virtual WMError RegisterMainWindowCloseListeners(
        const sptr<IMainWindowCloseListener>& listener) { return WMError::WM_ERROR_DEVICE_NOT_SUPPORT; }

    /**
     * @brief Unregister main window close listener.
     *
     * @param listener IMainWindowCloseListener.
     * @return WM_OK means unregister success, others means unregister failed.
     */
    virtual WMError UnregisterMainWindowCloseListeners(
        const sptr<IMainWindowCloseListener>& listener) { return WMError::WM_ERROR_DEVICE_NOT_SUPPORT; }

    /**
     * @brief Register window close async process listener.
     *
     * @param listener IWindowWillCloseListener.
     * @return WM_OK means register success, others means register failed.
     */
    virtual WMError RegisterWindowWillCloseListeners(
        const sptr<IWindowWillCloseListener>& listener) { return WMError::WM_ERROR_DEVICE_NOT_SUPPORT; }

    /**
     * @brief Unregister window close async process listener.
     *
     * @param listener IWindowWillCloseListener.
     * @return WM_OK means unregister success, others means unregister failed.
     */
    virtual WMError UnRegisterWindowWillCloseListeners(
        const sptr<IWindowWillCloseListener>& listener) { return WMError::WM_ERROR_DEVICE_NOT_SUPPORT; }

    /**
     * @brief Register switch free multi-window listener.
     *
     * @param listener ISwitchFreeMultiWindowListener.
     * @return WM_OK means register success, others means register failed.
     */
    virtual WMError RegisterSwitchFreeMultiWindowListener(
        const sptr<ISwitchFreeMultiWindowListener>& listener) { return WMError::WM_ERROR_DEVICE_NOT_SUPPORT; }

    /**
     * @brief Unregister switch free multi-window listener.
     *
     * @param listener ISwitchFreeMultiWindowListener.
     * @return WM_OK means unregister success, others means unregister failed.
     */
    virtual WMError UnregisterSwitchFreeMultiWindowListener(
        const sptr<ISwitchFreeMultiWindowListener>& listener) { return WMError::WM_ERROR_DEVICE_NOT_SUPPORT; }

    /**
     * @brief Set Shaped Window Mask.
     *
     * @param windowMask Mask of the shaped window.
     * @return WM_OK means set success, others means failed.
     */
    virtual WMError SetWindowMask(const std::vector<std::vector<uint32_t>>& windowMask)
    {
        return WMError::WM_ERROR_DEVICE_NOT_SUPPORT;
    }

    /**
     * @brief Register keyboard panel info change listener.
     *
     * @param listener IKeyboardPanelInfoChangeListener.
     * @return WM_OK means register success, others means register failed.
     */
    virtual WMError RegisterKeyboardPanelInfoChangeListener(const sptr<IKeyboardPanelInfoChangeListener>& listener)
    {
        return WMError::WM_OK;
    }

    /**
     * @brief Unregister keyboard panel info change listener.
     *
     * @param listener IKeyboardPanelInfoChangeListener.
     * @return WM_OK means unregister success, others means unregister failed.
     */
    virtual WMError UnregisterKeyboardPanelInfoChangeListener(const sptr<IKeyboardPanelInfoChangeListener>& listener)
    {
        return WMError::WM_OK;
    }

    /**
     * @brief Get window by id
     *
     * @param windId window id
     * @return sptr<Window>
     */
    static sptr<Window> GetWindowWithId(uint32_t windId);

    /**
     * @brief register keyEvent filter.
     *
     * @param KeyEventFilterFunc callback func when window recieve keyEvent
     * @return WMError
     */
    virtual WMError SetKeyEventFilter(KeyEventFilterFunc KeyEventFilterFunc)
    {
        return WMError::WM_ERROR_DEVICE_NOT_SUPPORT;
    }

    /**
     * @brief clear keyEvent filter.
     *
     * @return WMError
     */
    virtual WMError ClearKeyEventFilter() { return WMError::WM_ERROR_DEVICE_NOT_SUPPORT;}

    /**
     * @brief register mouseEvent filter.
     *
     * @param mouseEventFilterFunc callback func when window receive mouseEvent
     * @return WMError
     */
    virtual WMError SetMouseEventFilter(MouseEventFilterFunc mouseEventFilterFunc)
    {
        return WMError::WM_ERROR_DEVICE_NOT_SUPPORT;
    }

    /**
     * @brief clear mouseEvent filter.
     *
     * @return WMError
     */
    virtual WMError ClearMouseEventFilter() { return WMError::WM_ERROR_DEVICE_NOT_SUPPORT; }

    /**
     * @brief register touchEvent filter.
     *
     * @param touchEventFilterFunc callback func when window receive touchEvent
     * @return WMError
     */
    virtual WMError SetTouchEventFilter(TouchEventFilterFunc touchEventFilterFunc)
    {
        return WMError::WM_ERROR_DEVICE_NOT_SUPPORT;
    }

    /**
     * @brief clear touchEvent filter.
     *
     * @return WMError
     */
    virtual WMError ClearTouchEventFilter() { return WMError::WM_ERROR_DEVICE_NOT_SUPPORT; }

    /**
     * @brief Register window rect change listener.
     *
     * @param listener IWindowRectChangeListener.
     * @return WM_OK means register success, others means register failed.
     */
    virtual WMError RegisterWindowRectChangeListener(const sptr<IWindowRectChangeListener>& listener)
    {
        return WMError::WM_ERROR_DEVICE_NOT_SUPPORT;
    }

    /**
     * @brief Unregister window rect change listener.
     *
     * @param listener IWindowRectChangeListener.
     * @return WM_OK means unregister success, others means unregister failed.
     */
    virtual WMError UnregisterWindowRectChangeListener(const sptr<IWindowRectChangeListener>& listener)
    {
        return WMError::WM_ERROR_DEVICE_NOT_SUPPORT;
    }

    /**
     * @brief Register a listener to observe window rectangle changes in global coordinates.
     *
     * @param listener The listener to receive rectangle change notifications.
     * @return WMError WM_OK if registration succeeds; otherwise, an error code is returned.
     */
    virtual WMError RegisterRectChangeInGlobalDisplayListener(const sptr<IRectChangeInGlobalDisplayListener>& listener)
    {
        return WMError::WM_ERROR_DEVICE_NOT_SUPPORT;
    }

    /**
     * @brief Unregister a previously registered rectangle change listener in global coordinates.
     *
     * @param listener The listener to be unregistered.
     * @return WMError WM_OK if unregistration succeeds; otherwise, an error code is returned.
     */
    virtual WMError UnregisterRectChangeInGlobalDisplayListener(
        const sptr<IRectChangeInGlobalDisplayListener>& listener)
    {
        return WMError::WM_ERROR_DEVICE_NOT_SUPPORT;
    }

    /**
     * @brief UIExtension register host window rect change listener.
     *
     * @param listener IWindowRectChangeListener.
     * @return WM_OK means register success, others means register failed.
     */
    virtual WMError RegisterHostWindowRectChangeListener(const sptr<IWindowRectChangeListener>& listener)
    {
        return WMError::WM_ERROR_DEVICE_NOT_SUPPORT;
    }

    /**
     * @brief UIExtension unregister host window rect change listener.
     *
     * @param listener IWindowRectChangeListener.
     * @return WM_OK means unregister success, others means unregister failed.
     */
    virtual WMError UnregisterHostWindowRectChangeListener(const sptr<IWindowRectChangeListener>& listener)
    {
        return WMError::WM_ERROR_DEVICE_NOT_SUPPORT;
    }

    /**
     * @brief Register window nonsecure limit change listener.
     *
     * @param listener IExtensionSecureLimitChangeListener.
     * @return WM_OK means register success, others means register failed.
     */
    virtual WMError RegisterExtensionSecureLimitChangeListener(
        const sptr<IExtensionSecureLimitChangeListener>& listener)
    {
        return WMError::WM_ERROR_DEVICE_NOT_SUPPORT;
    }

    /**
     * @brief Unregister window nonsecure limit change listener.
     *
     * @param listener IExtensionSecureLimitChangeListener.
     * @return WM_OK means unregister success, others means unregister failed.
     */
    virtual WMError UnregisterExtensionSecureLimitChangeListener(
        const sptr<IExtensionSecureLimitChangeListener>& listener)
    {
        return WMError::WM_ERROR_DEVICE_NOT_SUPPORT;
    }

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
     * @brief get callingWindow windowStatus.
     * @param callingWindowId
     * @param windowStatus
     * @return WM_OK means get success, others means get Failed.
     */
    virtual WMError GetCallingWindowWindowStatus(uint32_t callingWindowId, WindowStatus& windowStatus) const
    {
        return WMError::WM_OK;
    }

    /**
     * @brief get callingWindow windowRect
     * @param callingWindowId
     * @param rect.
     * @return WM_OK means get success, others means get failed
     */
    virtual WMError GetCallingWindowRect(uint32_t callingWindowId, Rect& rect) const
    {
        return WMError::WM_OK;
    }

    /**
     * @brief Set gray scale of window
     * @param grayScale gray scale of window.
     * @return WM_OK means set success, others means set failed.
     */
    virtual WMError SetGrayScale(float grayScale) { return WMError::WM_ERROR_DEVICE_NOT_SUPPORT; }

    /**
     * @brief adjust keyboard layout
     * @param params
     * @return WM_OK means set success, others means set failed
     */
    virtual WMError AdjustKeyboardLayout(const KeyboardLayoutParams params) { return WMError::WM_OK; }

    /**
     * @brief Set the Dvsync Switch
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

    /**
     * @brief Set whether to enable immersive mode.
     * @param enable the value true means to enable immersive mode, and false means the opposite.
     * @return WM_OK means set success, others means set failed.
     */
    virtual WMError SetImmersiveModeEnabledState(bool enable) { return WMError::WM_OK; }

    /**
     * @brief Get whether the immersive mode is enabled or not.
     *
     * @return true means the immersive mode is enabled, and false means the opposite.
     */
    virtual bool GetImmersiveModeEnabledState() const { return true; }

    /**
     * @brief Get whether the window is in immersive layout or not.
     *
     * @return true means the window is in immersive layout, and false means the opposite.
     */
    virtual WMError IsImmersiveLayout(bool& isImmersiveLayout) const { return WMError::WM_ERROR_DEVICE_NOT_SUPPORT; }

    /**
     * @brief Get the height of status bar.
     *
     * @return the height of status bar.
     */
    virtual uint32_t GetStatusBarHeight() const { return 0; }

    /**
     * @brief Get whether the free multi-window mode is enabled or not.
     *
     * @return true means the free multi-window mode is enabled, and false means the opposite.
     */
    virtual bool GetFreeMultiWindowModeEnabledState() { return false; }

    /**
     * @brief Get the window status of current window.
     *
     * @param windowStatus
     * @return WMError.
     */
    virtual WMError GetWindowStatus(WindowStatus& windowStatus) { return WMError::WM_ERROR_DEVICE_NOT_SUPPORT; }

    /**
     * @brief Set the ContinueState of window.
     *
     * @param continueState of the window.
     * @return Errorcode of window.
     */
    virtual WMError SetContinueState(int32_t continueState) { return WMError::WM_DO_NOTHING; }

    /**
     * @brief Notify host that UIExtension timeout
     *
     * @param errorCode error code when UIExtension timeout
     */
    virtual void NotifyExtensionTimeout(int32_t errorCode) {}

    /**
     * @brief Get Data Handler of UIExtension
     */
    virtual std::shared_ptr<IDataHandler> GetExtensionDataHandler() const { return nullptr; }

    /**
     * @brief Get the real parent id of UIExtension
     *
     * @return Real parent id of UIExtension
     */
    virtual int32_t GetRealParentId() const { return static_cast<int32_t>(INVALID_WINDOW_ID); }

    /**
     * @brief Get the parent window type of UIExtension
     *
     * @return Parent window type of UIExtension
     */
    virtual WindowType GetParentWindowType() const { return WindowType::WINDOW_TYPE_APP_MAIN_WINDOW; }

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
     * @brief Notify modal UIExtension it may be covered
     *
     * @param byLoadContent True when called by loading content, false when called by creating non topmost subwindow
     */
    virtual void NotifyModalUIExtensionMayBeCovered(bool byLoadContent) {}

    /**
     * @brief Notify extension asynchronously
     *
     * @param notifyEvent event type
     * @return void
     */
    virtual void NotifyExtensionEventAsync(uint32_t notifyEvent) {}

    /**
     * @brief Get whether this window is the first level sub window of UIExtension.
     *
     * @return true - is the first sub window of UIExtension, false - is not the first sub window of UIExtension
     */
    virtual bool GetIsUIExtFirstSubWindow() const { return false; }

    /**
     * @brief Get whether this window is a sub window of any level of UIExtension.
     *
     * @return true - is UIExtension sub window, false - is not UIExtension sub window.
     */
    virtual bool GetIsUIExtAnySubWindow() const { return false; }

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
     * @brief this interface is invoked by the ACE to the native host.
     * @param eventName invoking event name, which is used to distinguish different invoking types.
     * @param value used to transfer parameters.
     * @return WM_OK means get success, others means get failed.
     */
    virtual WMError OnContainerModalEvent(const std::string& eventName,
        const std::string& value) { return WMError::WM_ERROR_DEVICE_NOT_SUPPORT; }

    /**
     * @brief Determine whether the window spans multiple screens and displays in full screen mode.
     *
     * @param isAcrossDisplays the value true means to span multiple screens, and false means the opposite.
     * @return WM_OK means success, others means failed.
     */
    virtual WMError IsMainWindowFullScreenAcrossDisplays(
        bool& isAcrossDisplays) { return WMError::WM_ERROR_DEVICE_NOT_SUPPORT; }

    /**
     * @brief Get the type of window.
     *
     * @return The string corresponding to the window.
     */
    virtual std::string GetClassType() const { return "Window"; }

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
     * @brief Get whether is mid scene.
     *
     * @return True - is mid scene, false - is not mid scene.
     */
    virtual WMError GetIsMidScene(bool& isMidScene) { return WMError::WM_OK; }

    /**
     * @brief Get layoutTransform of window uiContent.
     *
     * @return UiContent of layoutTransform.
     */
    virtual Transform GetLayoutTransform() const
    {
        static Transform trans;
        return trans;
    }

    /**
     * @brief Show keyboard window
     *
     * @param callingWindowId the id of calling window.
     * @param targetDisplayId the id of target display
     * @param effectOption Keyboard will show with special effect option.
     * @return WM_OK means window show success, others means failed.
     */
    virtual WMError ShowKeyboard(uint32_t callingWindowId, uint64_t targetDisplayId, KeyboardEffectOption effectOption
        = { KeyboardViewMode::NON_IMMERSIVE_MODE, KeyboardFlowLightMode::NONE, KeyboardGradientMode::NONE, 0 })
    {
        return WMError::WM_OK;
    }

    /**
     * @brief Change keyboard effect with option
     *
     * @param effectOption Keyboard will update to the special effect option.
     * @return WM_OK means effect update success, others means failed.
     */
    virtual WMError ChangeKeyboardEffectOption(KeyboardEffectOption effectOption)
    {
        return WMError::WM_OK;
    }

    /**
     * @brief Register window highlight change listener.
     *
     * @param listener IWindowHighlightChangeListener.
     * @return WM_OK means register success, others means register failed.
     */
    virtual WMError RegisterWindowHighlightChangeListeners(const sptr<IWindowHighlightChangeListener>& listener)
    {
        return WMError::WM_ERROR_DEVICE_NOT_SUPPORT;
    }

    /**
     * @brief Unregister window highlight change listener.
     *
     * @param listener IWindowHighlightChangeListener.
     * @return WM_OK means unregister success, others means unregister failed.
     */
    virtual WMError UnregisterWindowHighlightChangeListeners(const sptr<IWindowHighlightChangeListener>& listener)
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
     * @brief Get highlight property of window.
     *
     * @param highlighted True means the window is highlighted, and false means the opposite.
     * @return WM_OK means get success, others means get failed.
     */
    virtual WMError IsWindowHighlighted(bool& highlighted) const { return WMError::WM_ERROR_DEVICE_NOT_SUPPORT; }

    /**
     * @brief Get cross screen axis state.
     *
     * @return The cross screen axis state of the Window.
     */
    virtual CrossAxisState GetCrossAxisState() { return CrossAxisState::STATE_INVALID; }

    /**
     * @brief Register window screen axis state change listener.
     *
     * @param listener IWindowCrossAxisChangeListener.
     * @return WM_OK means register success, others means register failed.
     */
    virtual WMError RegisterWindowCrossAxisListener(const sptr<IWindowCrossAxisListener>& listener)
    {
        return WMError::WM_OK;
    }

    /**
     * @brief Unregister window screen axis state change listener.
     *
     * @param listener IWindowCrossAxisChangeListener.
     * @return WM_OK means unregister success, others means unregister failed.
     */
    virtual WMError UnregisterWindowCrossAxisListener(const sptr<IWindowCrossAxisListener>& listener)
    {
        return WMError::WM_OK;
    }

    /**
     * @brief Get custom extension param.
     *
     * @param want the want to store param.
     */
    virtual void GetExtensionConfig(AAFwk::WantParams& want) const {}

    /**
     * @brief Update custom extension param.
     *
     * @param want the want to update param.
     */
    virtual void UpdateExtensionConfig(const std::shared_ptr<AAFwk::Want>& want) {}

    /**
     * @brief Receive async IPC message from UIExtensionComponent.
     *
     * @param code the message code.
     * @param persistentId the persistent id of UIExtension.
     * @param data the data transfered from UIExtensionComponent.
     */
    virtual WMError OnExtensionMessage(uint32_t code, int32_t persistentId, const AAFwk::Want& data)
    {
        return WMError::WM_OK;
    }

    /**
     * @brief Query whether the waterfall mode is enabled or not.
     *
     * @return true means the waterfall mode is enabled, and false means the opposite.
     */
    virtual bool IsWaterfallModeEnabled() { return false; }

    /**
     * @brief Register waterfall mode change listener.
     *
     * @param listener IWaterfallModeChangeListener.
     * @return WM_OK means register success, others means register failed.
     */
    virtual WMError RegisterWaterfallModeChangeListener(const sptr<IWaterfallModeChangeListener>& listener)
    {
        return WMError::WM_OK;
    }

    /**
     * @brief Unregister waterfall mode change listener.
     *
     * @param listener IWaterfallModeChangeListener.
     * @return WM_OK means unregister success, others means unregister failed.
     */
    virtual WMError UnregisterWaterfallModeChangeListener(const sptr<IWaterfallModeChangeListener>& listener)
    {
        return WMError::WM_OK;
    }

    /**
     * @brief Register window scene attach or detach framenode listener.
     *
     * @param listener IWindowAttachStateChangeListner.
     * @return WM_OK means register success, others means register failed.
     */
    virtual WMError RegisterWindowAttachStateChangeListener(const sptr<IWindowAttachStateChangeListner>& listener)
    {
        return WMError::WM_OK;
    }

    /**
     * @brief Unregister window scene attach or detach framenode listener.
     *
     * @return WM_OK means unregister success
     */
    virtual WMError UnregisterWindowAttachStateChangeListener()
    {
        return WMError::WM_OK;
    }

    /**
     * @brief Register window rotation change listener.
     *
     * @param listener IWindowRotationChangeListener.
     * @return WM_OK means register success, others means register failed.
     */
    virtual WMError RegisterWindowRotationChangeListener(const sptr<IWindowRotationChangeListener>& listener)
    {
        return WMError::WM_ERROR_DEVICE_NOT_SUPPORT;
    }

    /**
     * @brief Unregister window rotation change listener.
     *
     * @param listener IWindowRotationChangeListener.
     * @return WM_OK means unregister success, others means unregister failed.
     */
    virtual WMError UnregisterWindowRotationChangeListener(const sptr<IWindowRotationChangeListener>& listener)
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
    virtual WMError SetFollowParentWindowLayoutEnabled(bool isFollow) { return WMError::WM_ERROR_DEVICE_NOT_SUPPORT; }

    /**
     * @brief Enable or disable window shadow.
     *
     * @param isEnabled Enable or disable window shadow.
     * @return WM_OK means set success.
     */
    virtual WMError SetWindowShadowEnabled(bool isEnabled) { return WMError::WM_ERROR_DEVICE_NOT_SUPPORT; }

    /**
     * @brief Get whether window shadow is enabled.
     *
     * @return True means window shadow is enabled.
     */
    virtual bool GetWindowShadowEnabled() const { return true; }

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
     * @brief Update the pipTemplateInfo.
     *
     * @param pipTemplateInfo the pipTemplateInfo of pip window
     */
    virtual void UpdatePiPTemplateInfo(PiPTemplateInfo& pipTemplateInfo) {}
    
    /**
     * @brief Register a listener for observing keyboard show animation begins.
     *
     * @param listener IKeyboardWillShowListener.
     * @return WM_OK means register success, others means register failed.
     */
    virtual WMError RegisterKeyboardWillShowListener(const sptr<IKeyboardWillShowListener>& listener)
    {
        return WMError::WM_ERROR_DEVICE_NOT_SUPPORT;
    }

    /**
     * @brief Unregister keyboard show animation start listener.
     *
     * @param listener IKeyboardWillShowListener.
     * @return WM_OK means unregister success, others means unregister failed.
     */
    virtual WMError UnregisterKeyboardWillShowListener(const sptr<IKeyboardWillShowListener>& listener)
    {
        return WMError::WM_ERROR_DEVICE_NOT_SUPPORT;
    }

    /**
     * @brief Register a listener for observing keyboard hide animation begins.
     *
     * @param listener IKeyboardWillHideListener.
     * @return WM_OK means register success, others means register failed.
     */
    virtual WMError RegisterKeyboardWillHideListener(const sptr<IKeyboardWillHideListener>& listener)
    {
        return WMError::WM_ERROR_DEVICE_NOT_SUPPORT;
    }
 
    /**
     * @brief Unregister keyboard hide animation start listener.
     *
     * @param listener IKeyboardWillHideListener.
     * @return WM_OK means unregister success, others means unregister failed.
     */
    virtual WMError UnregisterKeyboardWillHideListener(const sptr<IKeyboardWillHideListener>& listener)
    {
        return WMError::WM_ERROR_DEVICE_NOT_SUPPORT;
    }

    /**
     * @brief Register keyboard show animation completion listener.
     *
     * @param listener IKeyboardDidShowListener.
     * @return WM_OK means register success, others means register failed.
     */
    virtual WMError RegisterKeyboardDidShowListener(const sptr<IKeyboardDidShowListener>& listener)
    {
        return WMError::WM_ERROR_DEVICE_NOT_SUPPORT;
    }

    /**
     * @brief Unregister keyboard show animation completion listener.
     *
     * @param listener IKeyboardDidShowListener.
     * @return WM_OK means unregister success, others means unregister failed.
     */
    virtual WMError UnregisterKeyboardDidShowListener(const sptr<IKeyboardDidShowListener>& listener)
    {
        return WMError::WM_ERROR_DEVICE_NOT_SUPPORT;
    }

    /**
     * @brief Register keyboard hide animation completion listener.
     *
     * @param listener IKeyboardDidHideListener.
     * @return WM_OK means register success, others means register failed.
     */
    virtual WMError RegisterKeyboardDidHideListener(const sptr<IKeyboardDidHideListener>& listener)
    {
        return WMError::WM_ERROR_DEVICE_NOT_SUPPORT;
    }

    /**
     * @brief Unregister keyboard hide animation completion listener.
     *
     * @param listener IKeyboardDidHideListener.
     * @return WM_OK means unregister success, others means unregister failed.
     */
    virtual WMError UnregisterKeyboardDidHideListener(const sptr<IKeyboardDidHideListener>& listener)
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
     * @brief Set the bundleName, moduleName and abilityName of the hooked window.
     *
     * @param elementName includes bundleName, moduleName and abilityName of the hooked window.
     * @return WM_OK means set success.
     */
    virtual WMError SetHookTargetElementInfo(const AppExecFwk::ElementName& elementName)
    {
        return WMError::WM_ERROR_DEVICE_NOT_SUPPORT;
    }

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
     * @brief Use implict animation
     *
     * @param used used
     * @return Returns WMError::WM_OK if called success, otherwise failed.
     */
    virtual WMError UseImplicitAnimation(bool useImplicit) { return WMError::WM_ERROR_DEVICE_NOT_SUPPORT; }

    /** 
    * @brief Set intent param to arkui.
     *
     * @param intentParam intent param from ams.
     * @param loadPageCallback load page callback after send intent.
     * @param isColdStart Mark as cold start or not
     * @return WM_OK means set intent param success.
     */
    virtual WMError SetIntentParam(const std::string& intentParam, const std::function<void()>& loadPageCallback,
        bool isColdStart)
    {
        return WMError::WM_OK;
    }

    /**
     * @brief Set the source of subwindow.
     *
     * @param source 0 - defalut, 1 - arkui.
     * @return WM_OK means set success.
     */
    virtual WMError SetSubWindowSource(SubWindowSource source) { return WMError::WM_ERROR_DEVICE_NOT_SUPPORT; }

    /**
     * @brief Set the frameRect in a partial zoom-in scene.
     *
     * @param frameRect The original rect of frameBuffer before partial zoom-in.
     * @return WMError::WM_OK means set success, otherwise failed.
     */
    virtual WMError SetFrameRectForPartialZoomIn(const Rect& frameRect)
    {
        return WMError::WM_ERROR_DEVICE_NOT_SUPPORT;
    }

    /**
     * @brief Set the navDestinationInfo of atomicService to arkui.
     *
     * @param navDestinationInfo navDestinationInfo in atomicService hap
     */
    virtual void SetNavDestinationInfo(const std::string& navDestinationInfo) {}

    /**
     * @brief Inject a pointerEvent to arkui.
     *
     * @param pointerEvent PointerEvent of Multi-Model Input.
     * @return WM_OK means set success.
     */
    virtual WMError InjectTouchEvent(const std::shared_ptr<MMI::PointerEvent>& pointerEvent)
    {
        return WMError::WM_ERROR_SYSTEM_ABNORMALLY;
    }

    /**
     * @brief update the floating ball window instance.
     *
     * @param fbTemplateInfo the template info of the floating-ball.
     * @param icon the icon of the floating-ball.
     */
    virtual WMError UpdateFloatingBall(const FloatingBallTemplateBaseInfo& fbTemplateBaseInfo,
        const std::shared_ptr<Media::PixelMap>& icon)
    {
        return WMError::WM_OK;
    }
    
    /**
     * @brief Notify prepare to close window
     */
    virtual void NotifyPrepareCloseFloatingBall() {}
 
    /**
     * @brief restore floating ball ability.
     *
     * @param want the want of the ability.
     */
    virtual WMError RestoreFbMainWindow(const std::shared_ptr<AAFwk::Want>& want)
    {
        return WMError::WM_OK;
    }
 
    /**
     * @brief get windowId of floating-ball
     *
     * @param windowId the windowId of floating-ball.
     */
    virtual WMError GetFloatingBallWindowId(uint32_t& windowId)
    {
        return WMError::WM_OK;
    }

    /**
     * @brief UIExtension get host status bar content color, only for NAPI Call.
     *
     * @return content color
     */
    virtual uint32_t GetHostStatusBarContentColor() const { return 0; }

    /**
     * @brief Get autoStart picture-in-picture switch status of system setting.
     *
     * @param switchStatus autoStart picture-in-picture switch status.
     * @return WM_OK means get success.
     */
    virtual WMError GetPiPSettingSwitchStatus(bool& switchStatus) const { return WMError::WM_OK; }

    /**
     * @brief Get picture-in-picture isPipEnabled of system setting.
     *
     * @param isPipEnabled  picture-in-picture isPipEnabled value.
     * @return WM_OK means get success.
     */
    virtual WMError GetIsPipEnabled(bool& isPipEnabled) const { return WMError::WM_OK; }

    /**
     * @brief Set parent windowId of picture-in-picture window.
     *
     * @param windowId parent windowId of picture-in-picture window.
     * @return WMError::WM_OK means set success, otherwise failed.
     */
    virtual WMError SetPipParentWindowId(uint32_t windowId) const { return WMError::WM_OK; }

    /**
     * @brief get is pip active
     *
     * @return WMError
     */
    virtual WMError IsPiPActive(bool& status) { return WMError::WM_OK; }

    /**
     * @brief return true if current window is anco, otherwise return false
     */
    virtual bool IsAnco() const
    {
        return false;
    }

    /**
     * @brief special process on point down event
     */
    virtual bool OnPointDown(int32_t eventId, int32_t posX, int32_t posY)
    {
        return false;
    }

    /**
     * @brief notify window is full screen in force split mode.
     *
     * @param shouldFullScreen true means full screen, false means force split.
     */
    virtual void NotifyIsFullScreenInForceSplitMode(bool isFullScreen) {}

    /**
     * @brief Register a listener to listen ui content create.
     *
     * @param listener IUIContentCreateListener.
     * @return WM_OK means register success, others means register failed.
     */
    virtual WMError RegisterUIContentCreateListener(const sptr<IUIContentCreateListener>& listener)
    {
        return WMError::WM_OK;
    }

    /**
     * @brief Unregister a listener to listen ui content create.
     *
     * @param listener IUIContentCreateListener.
     * @return WM_OK means register success, others means register failed.
     */
    virtual WMError UnregisterUIContentCreateListener(const sptr<IUIContentCreateListener>& listener)
    {
        return WMError::WM_OK;
    }

    /**
     * @brief Update compatible style mode of the window.
     *
     * @param mode CompatibleStyleMode.
     * @return WM_OK means update mode success, others means failed.
     */
    virtual WMError UpdateCompatibleStyleMode(CompatibleStyleMode mode)
    {
        return WMError::WM_OK;
    }

    /**
     * @brief register a listener to listen whether the window title bar is show or hide.
     *
     * @param listener IWindowTitleChangeListener.
     * @return WM_OK means register success, others means register failed.
     */
    virtual WMError RegisterWindowTitleChangeListener(const sptr<IWindowTitleChangeListener>& listener)
    {
        return WMError::WM_OK;
    }
 
    /**
     * @brief Unregister the IWindowTitleChangeListener.
     *
     * @param listener IWindowTitleChangeListener.
     * @return WM_OK means unregister success, others means unregister failed.
     */
    virtual WMError UnregisterWindowTitleChangeListener(const sptr<IWindowTitleChangeListener>& listener)
    {
        return WMError::WM_OK;
    }

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
     * @return - Returns the status code of the execution.
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
     * @brief Lock the mouse cursor restricting it to a specified window area, and also control whether the cursor
     *        follows movement. Only supported by the focus window; the lock is automatically released when the
     *        window loses focus.
     *
     * @param windowId WindowId when window is created.
     * @param isCursorFollowMovement Set mouse cursor lock mode.
     * @return Returns the status code of the execution.
     */
    virtual WMError LockCursor(int32_t windowId, bool isCursorFollowMovement)
    {
        return WMError::WM_ERROR_DEVICE_NOT_SUPPORT;
    }

    /**
     * @brief Clear the window mouse cursor status. Revert to mouse cursor free movement mode.
     *
     * @param windowId WindowId when window is created.
     * @return Returns the status code of the execution.
     */
    virtual WMError UnlockCursor(int32_t windowId)
    {
        return WMError::WM_ERROR_DEVICE_NOT_SUPPORT;
    }
 
    /**
     * @brief Calculate whether the pointerEvent hits the title bar.
     *
     * @param hitTitleBar true means hit title bar success, false means not hit title bar.
     */
    virtual bool IsHitTitleBar(std::shared_ptr<MMI::PointerEvent>& pointerEvent) const { return false; }

    /**
     * @brief Calculate whether the pointerEvent hits the title bar.
     *
     * @param pointerEvent
     * @return true means hit title bar success, false means not hit title bar.
     */
    virtual bool IsHitHotAreas(std::shared_ptr<MMI::PointerEvent>& pointerEvent) { return false; }
 
    /**
     * @brief Get anco window hot areas.
     *
     * @param rects Hot areas of anco window.
     */
    virtual std::vector<Rect> GetAncoWindowHotAreas()
    {
        std::vector<Rect> rectAreas;
        return rectAreas;
    }

    /**
     * @brief Check if the current device is in free window mode.
     *
     * @return true means is in free window mode, false means not in free window mode.
     */
    virtual bool IsInFreeWindowMode() const { return false; }

    /**
     * @brief register a listener to listen whether the window is in free window mode.
     *
     * @param listener IFreeWindowModeChangeListener.
     * @return WM_OK means register success, others means register failed.
     */
    virtual WMError RegisterFreeWindowModeChangeListener(const sptr<IFreeWindowModeChangeListener>& listener)
    {
        return WMError::WM_OK;
    }
 
    /**
     * @brief Unregister the IFreeWindowModeChangeListener.
     *
     * @param listener IFreeWindowModeChangeListener.
     * @return WM_OK means unregister success, others means unregister failed.
     */
    virtual WMError UnregisterFreeWindowModeChangeListener(const sptr<IFreeWindowModeChangeListener>& listener)
    {
        return WMError::WM_OK;
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
     * @brief register a listener to listen the window title bar and window hot areas.
     *
     * @param listener IWindowTitleOrHotAreasListener.
     * @return WM_OK means register success, others means register failed.
     */
    virtual WMError RegisterWindowTitleOrHotAreasListener(const sptr<IWindowTitleOrHotAreasListener>& listener)
    {
        return WMError::WM_OK;
    }
 
    /**
     * @brief Unregister the IWindowTitleOrHotAreasListener.
     *
     * @param listener IWindowTitleOrHotAreasListener.
     * @return WM_OK means unregister success, others means unregister failed.
     */
    virtual WMError UnregisterWindowTitleOrHotAreasListener(const sptr<IWindowTitleOrHotAreasListener>& listener)
    {
        return WMError::WM_OK;
    }

    /**
     * @brief Set status bar color for uiExtension.
     *
     * @param color Color numeric to set.
     * @return WM_OK means set success, others means failed.
     */
    virtual WMError SetStatusBarColorForExtension(uint32_t color) { return WMError::WM_OK; }

    /**
     * @brief Flush vsync for prelaunch.
     */
    virtual void FlushVsync() {}
};
}
}
#endif // OHOS_ROSEN_WINDOW_H
