/*
 * Copyright (c) 2025 Huawei Device Co., Ltd.
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

#ifndef OHOS_ANI_WINDOW_LISTENER_H
#define OHOS_ANI_WINDOW_LISTENER_H

#include <map>
#include <mutex>

#include "ani_window_utils.h"
#include "class_var_definition.h"
#include "event_handler.h"
#include "refbase.h"
#include "window.h"
#include "window_manager.h"
#include "wm_common.h"

namespace OHOS {
namespace Rosen {

class AniWindowListener : public IWindowChangeListener,
                        public ISystemBarChangedListener,
                        public IAvoidAreaChangedListener,
                        public IWindowLifeCycle,
                        public IOccupiedAreaChangeListener,
                        public IKeyboardDidShowListener,
                        public IKeyboardDidHideListener,
                        public IKeyboardWillShowListener,
                        public IKeyboardWillHideListener,
                        public ITouchOutsideListener,
                        public IScreenshotListener,
                        public IDialogTargetTouchListener,
                        public IDialogDeathRecipientListener,
                        public IWaterMarkFlagChangedListener,
                        public IGestureNavigationEnabledChangedListener,
                        public IWindowVisibilityChangedListener,
                        public IOcclusionStateChangedListener,
                        public IFrameMetricsChangedListener,
                        public IWindowTitleButtonRectChangedListener,
                        public IWindowWillCloseListener,
                        public IWindowStatusChangeListener,
                        public IWindowNoInteractionListener,
                        public IWindowRectChangeListener,
                        public IMainWindowCloseListener,
                        public ISubWindowCloseListener,
                        public IWindowHighlightChangeListener,
                        public ISystemDensityChangeListener,
                        public IDisplayIdChangeListener,
                        public IWindowStageLifeCycle,
                        public IRectChangeInGlobalDisplayListener,
                        public IExtensionSecureLimitChangeListener,
                        public IWindowStatusDidChangeListener,
                        public IWindowRotationChangeListener,
                        public IAcrossDisplaysChangeListener,
                        public IScreenshotAppEventListener,
                        public IFreeWindowModeChangeListener {
public:
    AniWindowListener(ani_env* env, ani_vm* vm, ani_ref callback, CaseType caseType)
        : env_(env), vm_(vm), aniCallback_(callback), caseType_(caseType),
        weakRef_(wptr<AniWindowListener> (this)) {}
    ~AniWindowListener();
    ani_ref GetAniCallback() { return aniCallback_; }
    void SetAniCallback(ani_ref aniCallback) { aniCallback_ = aniCallback; }
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
    void OnSizeChange(const sptr<OccupiedAreaChangeInfo>& info,
        const std::shared_ptr<RSTransaction>& rsTransaction = nullptr) override;
    void OnKeyboardDidShow(const KeyboardPanelInfo& keyboardPanelInfo) override;
    void OnKeyboardWillShow(const KeyboardAnimationInfo& keyboardAnimationInfo,
        const KeyboardAnimationCurve& curve) override;
    void OnKeyboardWillHide(const KeyboardAnimationInfo& keyboardAnimationInfo,
        const KeyboardAnimationCurve& curve) override;
    void OnKeyboardDidHide(const KeyboardPanelInfo& keyboardPanelInfo) override;
    void OnTouchOutside() const override;
    void OnDialogTargetTouch() const override;
    void OnWindowNoInteractionCallback() override;
    void OnScreenshot() override;
    void OnDialogDeathRecipient() const override;
    void OnGestureNavigationEnabledUpdate(bool enable) override;
    void OnWaterMarkFlagUpdate(bool showWaterMark) override;
    void SetMainEventHandler();
    void SetTimeout(int64_t timeout) override;
    int64_t GetTimeout() const override;
    void OnWindowVisibilityChangedCallback(const bool isVisible) override;
    void OnOcclusionStateChanged(const WindowVisibilityState state) override;
    void OnFrameMetricsChanged(const FrameMetrics& metrics) override;
    void OnSystemDensityChanged(float density) override;
    void OnDisplayIdChanged(DisplayId displayId) override;

    void OnWindowStatusChange(WindowStatus status) override;
    void OnWindowTitleButtonRectChanged(const TitleButtonRect& titleButtonRect) override;
    void OnRectChange(Rect rect, WindowSizeChangeReason reason) override;
    void OnSubWindowClose(bool& terminateCloseProcess) override;
    void OnWindowHighlightChange(bool isHighlight) override;
    void OnMainWindowClose(bool& terminateCloseProcess) override;
    void OnRotationChange(const RotationChangeInfo& rotationChangeInfo,
        RotationChangeResult& rotationChangeResult) override;
    void OnRectChangeInGlobalDisplay(const Rect& rect, WindowSizeChangeReason reason) override;
    void OnSecureLimitChange(bool isLimit) override;
    void OnWindowStatusDidChange(WindowStatus status) override;
    void OnAcrossDisplaysChanged(bool isAcrossDisplays) override;
    void OnScreenshotAppEvent(ScreenshotEventType type) override;
    void OnFreeWindowModeChange(bool IsInFreeWindowMode) override;

    void AfterLifecycleForeground() override;
    void AfterLifecycleBackground() override;
    void AfterLifecycleResumed() override;
    void AfterLifecyclePaused() override;

private:
    void OnLastStrongRef(const void *) override;

    void KeyboardWillAnimateWithName(const KeyboardAnimationInfo& keyboardAnimationInfo,
        const char* fn, const KeyboardAnimationCurve& curve);

    Rect currRect_ = {0, 0, 0, 0};
    WindowState state_ {WindowState::STATE_INITIAL};
    int64_t noInteractionTimeout_ = 0;
    void LifeCycleCallback(LifeCycleEventType eventType);
    void WindowStageLifecycleCallback(WindowStageLifeCycleEventType eventType);
    ani_env* env_ = nullptr;
    ani_vm* vm_ = nullptr;
    ani_ref aniCallback_;
    CaseType caseType_ = CaseType::CASE_WINDOW;
    wptr<AniWindowListener> weakRef_ = nullptr;
    std::shared_ptr<AppExecFwk::EventHandler> eventHandler_ = nullptr;
    DEFINE_VAR_DEFAULT_FUNC_SET(bool, IsDeprecatedInterface, isDeprecatedInterface, false)
    RectChangeReason currentReason_ = RectChangeReason::UNDEFINED;
};
} // namespace Rosen
} // namespace OHOS
#endif /* OHOS_ANI_WINDOW_LISTENER_H */