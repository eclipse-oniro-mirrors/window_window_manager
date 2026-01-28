/*
 * Copyright (c) 2022 Huawei Device Co., Ltd.
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

#ifndef OHOS_JS_WINDOW_REGISTER_MANAGER_H
#define OHOS_JS_WINDOW_REGISTER_MANAGER_H
#include <map>
#include <mutex>
#include "js_window_listener.h"
#include "native_engine/native_engine.h"
#include "native_engine/native_reference.h"
#include "native_engine/native_value.h"
#include "refbase.h"
#include "window.h"
namespace OHOS {
namespace Rosen {
enum class RegisterListenerType : uint32_t {
    SYSTEM_BAR_TINT_CHANGE_CB,
    GESTURE_NAVIGATION_ENABLED_CHANGE_CB,
    WATER_MARK_FLAG_CHANGE_CB,
    WINDOW_SIZE_CHANGE_CB,
    SYSTEM_AVOID_AREA_CHANGE_CB,
    AVOID_AREA_CHANGE_CB,
    LIFECYCLE_EVENT_CB,
    WINDOW_EVENT_CB,
    KEYBOARD_HEIGHT_CHANGE_CB,
    KEYBOARD_WILL_SHOW_CB,
    KEYBOARD_WILL_HIDE_CB,
    KEYBOARD_DID_SHOW_CB,
    KEYBOARD_DID_HIDE_CB,
    TOUCH_OUTSIDE_CB,
    SCREENSHOT_EVENT_CB,
    SCREENSHOT_APP_EVENT_CB,
    DIALOG_TARGET_TOUCH_CB,
    DIALOG_DEATH_RECIPIENT_CB,
    WINDOW_STATUS_CHANGE_CB,
    WINDOW_STATUS_DID_CHANGE_CB,
    WINDOW_TITLE_BUTTON_RECT_CHANGE_CB,
    WINDOW_VISIBILITY_CHANGE_CB,
    OCCLUSION_STATE_CHANGE_CB,
    FRAME_METRICS_MEASURED_CHANGE_CB,
    WINDOW_DISPLAYID_CHANGE_CB,
    SYSTEM_DENSITY_CHANGE_CB,
    ACROSS_DISPLAYS_CHANGE_CB,
    WINDOW_NO_INTERACTION_DETECT_CB,
    WINDOW_RECT_CHANGE_CB,
    RECT_CHANGE_IN_GLOBAL_DISPLAY_CB,
    EXTENSION_SECURE_LIMIT_CHANGE_CB,
    SUB_WINDOW_CLOSE_CB,
    WINDOW_WILL_CLOSE_CB,
    WINDOW_STAGE_EVENT_CB,
    WINDOW_STAGE_LIFECYCLE_EVENT_CB,
    WINDOW_STAGE_CLOSE_CB,
    WINDOW_HIGHLIGHT_CHANGE_CB,
    WINDOW_ROTATION_CHANGE_CB,
    FREE_WINDOW_MODE_CHANGE_CB,
};

class JsWindowRegisterManager {
public:
    JsWindowRegisterManager();
    ~JsWindowRegisterManager();
    WmErrorCode RegisterListener(sptr<Window> window, std::string type,
        CaseType caseType, napi_env env, napi_value callback, napi_value parameter = nullptr);
    WmErrorCode UnregisterListener(sptr<Window> window, std::string type,
        CaseType caseType, napi_env env, napi_value value);
private:
    bool IsCallbackRegistered(napi_env env, std::string type, napi_value jsListenerObject);
    WmErrorCode ProcessWindowChangeRegister(sptr<JsWindowListener> listener, sptr<Window> window, bool isRegister,
        napi_env env, napi_value parameter = nullptr);
    WmErrorCode ProcessSystemAvoidAreaChangeRegister(sptr<JsWindowListener> listener, sptr<Window> window,
        bool isRegister, napi_env env, napi_value parameter = nullptr);
    WmErrorCode ProcessAvoidAreaChangeRegister(sptr<JsWindowListener> listener, sptr<Window> window, bool isRegister,
        napi_env env, napi_value parameter = nullptr);
    WmErrorCode ProcessLifeCycleEventRegister(sptr<JsWindowListener> listener, sptr<Window> window, bool isRegister,
        napi_env env, napi_value parameter = nullptr);
    WmErrorCode ProcessWindowStageLifeCycleEventRegister(const sptr<JsWindowListener>& listener,
        const sptr<Window>& window, bool isRegister, napi_env env, napi_value parameter = nullptr);
    WmErrorCode ProcessOccupiedAreaChangeRegister(sptr<JsWindowListener> listener, sptr<Window> window,
        bool isRegister, napi_env env, napi_value parameter = nullptr);
    WmErrorCode ProcessKeyboardWillShowRegister(sptr<JsWindowListener> listener, const sptr<Window>& window,
        bool isRegister, napi_env env, napi_value parameter);
    WmErrorCode ProcessKeyboardWillHideRegister(sptr<JsWindowListener> listener, const sptr<Window>& window,
        bool isRegister, napi_env env, napi_value parameter);
    WmErrorCode ProcessKeyboardDidShowRegister(sptr<JsWindowListener> listener, sptr<Window> window, bool isRegister,
        napi_env env, napi_value parameter = nullptr);
    WmErrorCode ProcessKeyboardDidHideRegister(sptr<JsWindowListener> listener, sptr<Window> window, bool isRegister,
        napi_env env, napi_value parameter = nullptr);
    WmErrorCode ProcessSystemBarChangeRegister(sptr<JsWindowListener> listener, sptr<Window> window, bool isRegister,
        napi_env env, napi_value parameter = nullptr);
    WmErrorCode ProcessTouchOutsideRegister(sptr<JsWindowListener> listener, sptr<Window> window, bool isRegister,
        napi_env env, napi_value parameter = nullptr);
    WmErrorCode ProcessScreenshotRegister(sptr<JsWindowListener> listener, sptr<Window> window, bool isRegister,
        napi_env env, napi_value parameter = nullptr);
    WmErrorCode ProcessScreenshotAppEventRegister(sptr<JsWindowListener> listener, sptr<Window> window, bool isRegister,
        napi_env env, napi_value parameter = nullptr);
    WmErrorCode ProcessDialogTargetTouchRegister(sptr<JsWindowListener> listener, sptr<Window> window, bool isRegister,
        napi_env env, napi_value parameter = nullptr);
    WmErrorCode ProcessDialogDeathRecipientRegister(sptr<JsWindowListener> listener, sptr<Window> window,
        bool isRegister, napi_env env, napi_value parameter = nullptr);
    WmErrorCode ProcessGestureNavigationEnabledChangeRegister(sptr<JsWindowListener> listener,
        sptr<Window> window, bool isRegister, napi_env env, napi_value parameter = nullptr);
    WmErrorCode ProcessWaterMarkFlagChangeRegister(sptr<JsWindowListener> listener,
        sptr<Window> window, bool isRegister, napi_env env, napi_value parameter = nullptr);
    WmErrorCode ProcessWindowVisibilityChangeRegister(sptr<JsWindowListener> listener, sptr<Window> window,
        bool isRegister, napi_env env, napi_value parameter = nullptr);
    WmErrorCode ProcessOcclusionStateChangeRegister(sptr<JsWindowListener> listener, sptr<Window> window,
        bool isRegister, napi_env env, napi_value parameter = nullptr);
    WmErrorCode ProcessFrameMetricsMeasuredChangeRegister(sptr<JsWindowListener> listener, sptr<Window> window,
        bool isRegister, napi_env env, napi_value parameter = nullptr);
    WmErrorCode ProcessDisplayIdChangeRegister(const sptr<JsWindowListener>& listener, const sptr<Window>& window,
        bool isRegister, napi_env env, napi_value parameter = nullptr);
    WmErrorCode ProcessSystemDensityChangeRegister(const sptr<JsWindowListener>& listener, const sptr<Window>& window,
        bool isRegister, napi_env env, napi_value parameter = nullptr);
    WmErrorCode ProcessAcrossDisplaysChangeRegister(const sptr<JsWindowListener>& listener, const sptr<Window>& window,
        bool isRegister, napi_env env, napi_value parameter = nullptr);
    WmErrorCode ProcessWindowNoInteractionRegister(sptr<JsWindowListener> listener, sptr<Window> window,
        bool isRegister, napi_env env, napi_value parameter = nullptr);
    WmErrorCode ProcessWindowStatusChangeRegister(sptr<JsWindowListener> listener, sptr<Window> window,
        bool isRegister, napi_env env, napi_value parameter = nullptr);
    WmErrorCode ProcessWindowStatusDidChangeRegister(sptr<JsWindowListener> listener, sptr<Window> window,
        bool isRegister, napi_env env, napi_value parameter = nullptr);
    WmErrorCode ProcessWindowTitleButtonRectChangeRegister(sptr<JsWindowListener> listener, sptr<Window> window,
        bool isRegister, napi_env env, napi_value parameter = nullptr);
    WmErrorCode ProcessWindowRectChangeRegister(sptr<JsWindowListener> listener, sptr<Window> window,
        bool isRegister, napi_env env, napi_value parameter = nullptr);
    WmErrorCode ProcessRectChangeInGlobalDisplayRegister(
        const sptr<JsWindowListener>& listener, const sptr<Window>& window,
        bool isRegister, napi_env env, napi_value parameter = nullptr);
    WmErrorCode ProcessExtensionSecureLimitChangeRegister(sptr<JsWindowListener> listener, sptr<Window> window,
        bool isRegister, napi_env env, napi_value parameter = nullptr);
    WmErrorCode ProcessSubWindowCloseRegister(sptr<JsWindowListener> listener, sptr<Window> window,
        bool isRegister, napi_env env, napi_value parameter = nullptr);
    WmErrorCode ProcessMainWindowCloseRegister(const sptr<JsWindowListener>& listener, const sptr<Window>& window,
        bool isRegister, napi_env env, napi_value parameter = nullptr);
    WmErrorCode ProcessWindowWillCloseRegister(const sptr<JsWindowListener>& listener, const sptr<Window>& window,
        bool isRegister, napi_env env, napi_value parameter = nullptr);
    WmErrorCode ProcessWindowHighlightChangeRegister(const sptr<JsWindowListener>& listener, const sptr<Window>& window,
        bool isRegister, napi_env env, napi_value parameter = nullptr);
    WmErrorCode ProcessWindowRotationChangeRegister(const sptr<JsWindowListener>& listener, const sptr<Window>& window,
        bool isRegister, napi_env env, napi_value parameter = nullptr);
    WmErrorCode ProcessFreeWindowModeChangeRegister(const sptr<JsWindowListener>& listener, const sptr<Window>& window,
        bool isRegister, napi_env env, napi_value parameter = nullptr);
    WmErrorCode ProcessListener(RegisterListenerType registerListenerType, CaseType caseType,
        const sptr<JsWindowListener>& windowManagerListener, const sptr<Window>& window, bool isRegister,
        napi_env env, napi_value parameter);
    std::map<std::string, std::map<std::shared_ptr<NativeReference>, sptr<JsWindowListener>>> jsCbMap_;
    std::mutex mtx_;
};
} // namespace Rosen
} // namespace OHOS

#endif