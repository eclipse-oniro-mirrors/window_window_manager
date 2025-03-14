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
const std::string WINDOW_SIZE_CHANGE_CB = "windowSizeChange";
const std::string SYSTEM_BAR_TINT_CHANGE_CB = "systemBarTintChange";
const std::string SYSTEM_AVOID_AREA_CHANGE_CB = "systemAvoidAreaChange";
const std::string AVOID_AREA_CHANGE_CB = "avoidAreaChange";
const std::string LIFECYCLE_EVENT_CB = "lifeCycleEvent";
const std::string WINDOW_STAGE_EVENT_CB = "windowStageEvent";
const std::string WINDOW_EVENT_CB = "windowEvent";
const std::string KEYBOARD_HEIGHT_CHANGE_CB = "keyboardHeightChange";
const std::string TOUCH_OUTSIDE_CB = "touchOutside";
const std::string SCREENSHOT_EVENT_CB = "screenshot";
const std::string DIALOG_TARGET_TOUCH_CB = "dialogTargetTouch";
const std::string DIALOG_DEATH_RECIPIENT_CB = "dialogDeathRecipient";
const std::string GESTURE_NAVIGATION_ENABLED_CHANGE_CB = "gestureNavigationEnabledChange";
const std::string WATER_MARK_FLAG_CHANGE_CB = "waterMarkFlagChange";
const std::string WINDOW_VISIBILITY_CHANGE_CB = "windowVisibilityChange";
const std::string WINDOW_STATUS_CHANGE_CB = "windowStatusChange";
const std::string WINDOW_TITLE_BUTTON_RECT_CHANGE_CB = "windowTitleButtonRectChange";
const std::string WINDOW_NO_INTERACTION_DETECT_CB = "noInteractionDetected";
const std::string WINDOW_RECT_CHANGE_CB = "windowRectChange";
const std::string SUB_WINDOW_CLOSE_CB = "subWindowClose";
const std::string WINDOW_STAGE_CLOSE_CB = "windowStageClose";

class AniWindowListener : public IWindowChangeListener,
                        public ISystemBarChangedListener,
                        public IAvoidAreaChangedListener,
                        public IWindowLifeCycle,
                        public IOccupiedAreaChangeListener,
                        public ITouchOutsideListener,
                        public IScreenshotListener,
                        public IDialogTargetTouchListener,
                        public IDialogDeathRecipientListener,
                        public IWaterMarkFlagChangedListener,
                        public IGestureNavigationEnabledChangedListener,
                        public IWindowVisibilityChangedListener,
                        public IWindowTitleButtonRectChangedListener,
                        public IWindowStatusChangeListener,
                        public IWindowNoInteractionListener,
                        public IWindowRectChangeListener,
                        public IMainWindowCloseListener,
                        public ISubWindowCloseListener {
public:
    AniWindowListener(ani_env* env, ani_ref callback, CaseType caseType)
        : env_(env), aniCallBack_(callback), caseType_(caseType), 
            weakRef_(wptr<AniWindowListener> (this)) {}
    ~AniWindowListener();
    void OnSystemBarPropertyChange(DisplayId displayId, const SystemBarRegionTints& tints) override;
    void OnSizeChange(Rect rect, WindowSizeChangeReason reason,
    const std::shared_ptr<RSTransaction>& rsTransaction = nullptr) override;
    void OnModeChange(WindowMode mode, bool hasDeco) override;
    void OnAvoidAreaChanged(const AvoidArea avoidArea, AvoidAreaType type) override;
    void AfterForeground() override;
    void AfterBackground() override;
    void AfterFocused() override;
    void AfterUnfocused() override;
    void AfterResumed() override;
    void AfterPaused() override;
    void AfterDestroyed() override;
    void OnSizeChange(const sptr<OccupiedAreaChangeInfo>& info,
    const std::shared_ptr<RSTransaction>& rsTransaction = nullptr) override;
    void OnTouchOutside() const override;
    void OnScreenshot() override;
    void OnDialogTargetTouch() const override;
    void OnDialogDeathRecipient() const override;
    void OnGestureNavigationEnabledUpdate(bool enable) override;
    void OnWaterMarkFlagUpdate(bool showWaterMark) override;
    void SetMainEventHandler();
    void OnWindowVisibilityChangedCallback(const bool isVisible) override;

    void OnWindowStatusChange(WindowStatus status) override;
    void OnWindowNoInteractionCallback() override;
    void OnWindowTitleButtonRectChanged(const TitleButtonRect& titleButtonRect) override;
    void OnRectChange(Rect rect, WindowSizeChangeReason reason) override;
    void OnSubWindowClose(bool& terminateCloseProcess) override;
    void OnMainWindowClose(bool& terminateCloseProcess) override;

private:
    void OnLastStrongRef(const void *) override;

    Rect currRect_ = {0, 0, 0, 0};
    WindowState state_ {WindowState::STATE_INITIAL};
    void LifeCycleCallBack(LifeCycleEventType eventType);
    ani_env* env_ = nullptr;
    ani_ref aniCallBack_;
    CaseType caseType_ = CaseType::CASE_WINDOW;
    wptr<AniWindowListener> weakRef_ = nullptr;
    std::shared_ptr<AppExecFwk::EventHandler> eventHandler_ = nullptr;
    DEFINE_VAR_DEFAULT_FUNC_SET(bool, IsDeprecatedInterface, isDeprecatedInterface, false)
    RectChangeReason currentReason_ = RectChangeReason::UNDEFINED;
};
} // namespace Rosen
} // namespace OHOS
#endif /* OHOS_ANI_WINDOW_LISTENER_H */