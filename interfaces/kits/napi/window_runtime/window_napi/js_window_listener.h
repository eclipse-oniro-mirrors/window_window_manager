/*
 * Copyright (c) 2021-2022 Huawei Device Co., Ltd.
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

#ifndef OHOS_JS_WINDOW_LISTENER_H
#define OHOS_JS_WINDOW_LISTENER_H

#include <map>
#include <mutex>

#include "class_var_definition.h"
#include "js_window_utils.h"
#include "native_engine/native_engine.h"
#include "native_engine/native_value.h"
#include "refbase.h"
#include "window.h"
#include "window_manager.h"
#include "wm_common.h"

namespace OHOS {
namespace Rosen {
extern const std::string WINDOW_SIZE_CHANGE_CB;
extern const std::string SYSTEM_BAR_TINT_CHANGE_CB;
extern const std::string SYSTEM_AVOID_AREA_CHANGE_CB;
extern const std::string AVOID_AREA_CHANGE_CB;
extern const std::string LIFECYCLE_EVENT_CB;
extern const std::string WINDOW_STAGE_EVENT_CB;
extern const std::string WINDOW_STAGE_LIFECYCLE_EVENT_CB;
extern const std::string WINDOW_EVENT_CB;
extern const std::string KEYBOARD_HEIGHT_CHANGE_CB;
extern const std::string KEYBOARD_WILL_SHOW_CB;
extern const std::string KEYBOARD_WILL_HIDE_CB;
extern const std::string KEYBOARD_DID_SHOW_CB;
extern const std::string KEYBOARD_DID_HIDE_CB;
extern const std::string TOUCH_OUTSIDE_CB;
extern const std::string SCREENSHOT_EVENT_CB;
extern const std::string SCREENSHOT_APP_EVENT_CB;
extern const std::string DIALOG_TARGET_TOUCH_CB;
extern const std::string DIALOG_DEATH_RECIPIENT_CB;
extern const std::string GESTURE_NAVIGATION_ENABLED_CHANGE_CB;
extern const std::string WATER_MARK_FLAG_CHANGE_CB;
extern const std::string WINDOW_VISIBILITY_CHANGE_CB;
extern const std::string OCCLUSION_STATE_CHANGE_CB;
extern const std::string FRAME_METRICS_MEASURED_CHANGE_CB;
extern const std::string WINDOW_DISPLAYID_CHANGE_CB;
extern const std::string SYSTEM_DENSITY_CHANGE_CB;
extern const std::string ACROSS_DISPLAYS_CHANGE_CB;
extern const std::string WINDOW_STATUS_CHANGE_CB;
extern const std::string WINDOW_STATUS_DID_CHANGE_CB;
extern const std::string WINDOW_TITLE_BUTTON_RECT_CHANGE_CB;
extern const std::string WINDOW_NO_INTERACTION_DETECT_CB;
extern const std::string WINDOW_RECT_CHANGE_CB;
extern const std::string RECT_CHANGE_IN_GLOBAL_DISPLAY_CB;
extern const std::string SUB_WINDOW_CLOSE_CB;
extern const std::string WINDOW_STAGE_CLOSE_CB;
extern const std::string EXTENSION_SECURE_LIMIT_CHANGE_CB;
extern const std::string WINDOW_HIGHLIGHT_CHANGE_CB;
extern const std::string WINDOW_WILL_CLOSE_CB;
extern const std::string WINDOW_ROTATION_CHANGE_CB;
extern const std::string FREE_WINDOW_MODE_CHANGE_CB;

class JsWindowListener : public IWindowChangeListener,
                         public ISystemBarChangedListener,
                         public IAvoidAreaChangedListener,
                         public IWindowLifeCycle,
                         public IWindowStageLifeCycle,
                         public IOccupiedAreaChangeListener,
                         public ITouchOutsideListener,
                         public IScreenshotListener,
                         public IScreenshotAppEventListener,
                         public IDialogTargetTouchListener,
                         public IDialogDeathRecipientListener,
                         public IWaterMarkFlagChangedListener,
                         public IGestureNavigationEnabledChangedListener,
                         public IWindowVisibilityChangedListener,
                         public IOcclusionStateChangedListener,
                         public IFrameMetricsChangedListener,
                         public IDisplayIdChangeListener,
                         public ISystemDensityChangeListener,
                         public IAcrossDisplaysChangeListener,
                         public IWindowTitleButtonRectChangedListener,
                         public IWindowStatusChangeListener,
                         public IWindowStatusDidChangeListener,
                         public IWindowNoInteractionListener,
                         public IWindowRectChangeListener,
                         public IRectChangeInGlobalDisplayListener,
                         public IExtensionSecureLimitChangeListener,
                         public IWindowWillCloseListener,
                         public IMainWindowCloseListener,
                         public ISubWindowCloseListener,
                         public IWindowHighlightChangeListener,
                         public IWindowRotationChangeListener,
                         public IKeyboardWillShowListener,
                         public IKeyboardWillHideListener,
                         public IKeyboardDidShowListener,
                         public IKeyboardDidHideListener,
                         public IFreeWindowModeChangeListener {
public:
    JsWindowListener(napi_env env, std::shared_ptr<NativeReference> callback, CaseType caseType)
        : env_(env), jsCallBack_(callback), caseType_(caseType), weakRef_(wptr<JsWindowListener> (this)) {}
    ~JsWindowListener();
    void OnSystemBarPropertyChange(DisplayId displayId, const SystemBarRegionTints& tints) override;
    void OnSizeChange(Rect rect, WindowSizeChangeReason reason,
        const std::shared_ptr<RSTransaction>& rsTransaction = nullptr) override;
    void OnModeChange(WindowMode mode, bool hasDeco) override;
    void OnAvoidAreaChanged(const AvoidArea avoidArea, AvoidAreaType type,
        const sptr<OccupiedAreaChangeInfo>& info = nullptr) override;
    void AfterForeground() override;
    void AfterBackground() override;
    void AfterFocused() override;
    void AfterUnfocused() override;
    void AfterResumed() override;
    void AfterPaused() override;
    void AfterDestroyed() override;
    void AfterLifecycleForeground() override;
    void AfterLifecycleBackground() override;
    void AfterLifecycleResumed() override;
    void AfterLifecyclePaused() override;
    void OnSizeChange(const sptr<OccupiedAreaChangeInfo>& info,
        const std::shared_ptr<RSTransaction>& rsTransaction = nullptr) override;
    void OnKeyboardWillShow(const KeyboardAnimationInfo& keyboardAnimationInfo,
        const KeyboardAnimationCurve& curve) override;
    void OnKeyboardWillHide(const KeyboardAnimationInfo& keyboardAnimationInfo,
        const KeyboardAnimationCurve& curve) override;
    void OnKeyboardDidShow(const KeyboardPanelInfo& keyboardPanelInfo) override;
    void OnKeyboardDidHide(const KeyboardPanelInfo& keyboardPanelInfo) override;
    void OnTouchOutside() const override;
    void OnScreenshot() override;
    void OnScreenshotAppEvent(ScreenshotEventType type) override;
    void OnDialogTargetTouch() const override;
    void OnDialogDeathRecipient() const override;
    void OnGestureNavigationEnabledUpdate(bool enable) override;
    void OnWaterMarkFlagUpdate(bool showWaterMark) override;
    napi_value CallJsMethod(const char* methodName, napi_value const * argv = nullptr, size_t argc = 0);
    void SetMainEventHandler();
    void OnWindowVisibilityChangedCallback(const bool isVisible) override;
    void OnOcclusionStateChanged(const WindowVisibilityState state) override;
    void OnFrameMetricsChanged(const FrameMetrics& metrics) override;
    void OnDisplayIdChanged(DisplayId displayId) override;
    void OnSystemDensityChanged(float density) override;
    void OnAcrossDisplaysChanged(bool isAcrossDisplays) override;

    void OnWindowStatusChange(WindowStatus status) override;
    void OnWindowStatusDidChange(WindowStatus status) override;
    void OnWindowNoInteractionCallback() override;
    void OnWindowTitleButtonRectChanged(const TitleButtonRect& titleButtonRect) override;
    void SetTimeout(int64_t timeout) override;
    int64_t GetTimeout() const override;

    /*
     * Window Layout
     */
    void OnRectChange(Rect rect, WindowSizeChangeReason reason) override;
    void OnRectChangeInGlobalDisplay(const Rect& rect, WindowSizeChangeReason reason) override;
    void OnFreeWindowModeChange(bool isInFreeWindowMode) override;

    void OnSecureLimitChange(bool isLimit) override;
    void OnWindowHighlightChange(bool isHighlight) override;
    void OnRotationChange(const RotationChangeInfo& rotationChangeInfo,
        RotationChangeResult& rotationChangeResult) override;

    /*
     * Window Decor listener
     */
    void OnSubWindowClose(bool& terminateCloseProcess) override;
    void OnMainWindowClose(bool& terminateCloseProcess) override;
    void OnWindowWillClose(sptr<Window> window) override;
    WmErrorCode CanCancelUnregister(const std::string& eventType);

    /*
     * Window Decor listener
     */
    std::atomic<uint32_t> asyncCloseExecuteCount_ { 0 };

private:
    void OnLastStrongRef(const void *) override;

    /*
     * Window Decor listener
     */
    void InitAsyncCloseCallback(sptr<Window> window);

    void KeyboardWillAnimateWithName(const KeyboardAnimationInfo& keyboardAnimationInfo,
        const std::string& callBackName, const KeyboardAnimationCurve& curve);

    Rect currRect_ = {0, 0, 0, 0};
    WindowState state_ {WindowState::STATE_INITIAL};
    void LifeCycleCallBack(LifeCycleEventType eventType);
    void WindowStageLifecycleCallback(WindowStageLifeCycleEventType eventType);
    int64_t noInteractionTimeout_ = 0;
    napi_env env_ = nullptr;
    std::shared_ptr<NativeReference> jsCallBack_;
    CaseType caseType_ = CaseType::CASE_WINDOW;
    wptr<JsWindowListener> weakRef_ = nullptr;
    std::shared_ptr<AppExecFwk::EventHandler> eventHandler_ = nullptr;
    DEFINE_VAR_DEFAULT_FUNC_SET(bool, IsDeprecatedInterface, isDeprecatedInterface, false)
    RectChangeReason currentReason_ = RectChangeReason::UNDEFINED;

    /*
     * Window Decor listener
     */
    sptr<AsyncCallback> closeAsyncCallback_ = nullptr;
};
}  // namespace Rosen
}  // namespace OHOS
#endif /* OHOS_JS_WINDOW_LISTENER_H */
