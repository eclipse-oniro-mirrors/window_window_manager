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

#include "js_window_register_manager.h"
#include "singleton_container.h"
#include "window_manager.h"
#include "window_manager_hilog.h"
#include "js_runtime_utils.h"

namespace OHOS {
namespace Rosen {
using namespace AbilityRuntime;
namespace {
constexpr HiviewDFX::HiLogLabel LABEL = {LOG_CORE, HILOG_DOMAIN_WINDOW, "JsRegisterManager"};

const std::map<std::string, RegisterListenerType> WINDOW_MANAGER_LISTENER_MAP {
    // white register list for window manager
    {SYSTEM_BAR_TINT_CHANGE_CB, RegisterListenerType::SYSTEM_BAR_TINT_CHANGE_CB},
    {GESTURE_NAVIGATION_ENABLED_CHANGE_CB, RegisterListenerType::GESTURE_NAVIGATION_ENABLED_CHANGE_CB},
    {WATER_MARK_FLAG_CHANGE_CB, RegisterListenerType::WATER_MARK_FLAG_CHANGE_CB},
};
const std::map<std::string, RegisterListenerType> WINDOW_LISTENER_MAP {
    // white register list for window
    {WINDOW_SIZE_CHANGE_CB, RegisterListenerType::WINDOW_SIZE_CHANGE_CB},
    {SYSTEM_AVOID_AREA_CHANGE_CB, RegisterListenerType::SYSTEM_AVOID_AREA_CHANGE_CB},
    {AVOID_AREA_CHANGE_CB, RegisterListenerType::AVOID_AREA_CHANGE_CB},
    {LIFECYCLE_EVENT_CB, RegisterListenerType::LIFECYCLE_EVENT_CB},
    {WINDOW_EVENT_CB, RegisterListenerType::WINDOW_EVENT_CB},
    {KEYBOARD_HEIGHT_CHANGE_CB, RegisterListenerType::KEYBOARD_HEIGHT_CHANGE_CB},
    {KEYBOARD_WILL_SHOW_CB, RegisterListenerType::KEYBOARD_WILL_SHOW_CB},
    {KEYBOARD_WILL_HIDE_CB, RegisterListenerType::KEYBOARD_WILL_HIDE_CB},
    {KEYBOARD_DID_SHOW_CB, RegisterListenerType::KEYBOARD_DID_SHOW_CB},
    {KEYBOARD_DID_HIDE_CB, RegisterListenerType::KEYBOARD_DID_HIDE_CB},
    {TOUCH_OUTSIDE_CB, RegisterListenerType::TOUCH_OUTSIDE_CB},
    {SCREENSHOT_EVENT_CB, RegisterListenerType::SCREENSHOT_EVENT_CB},
    {SCREENSHOT_APP_EVENT_CB, RegisterListenerType::SCREENSHOT_APP_EVENT_CB},
    {DIALOG_TARGET_TOUCH_CB, RegisterListenerType::DIALOG_TARGET_TOUCH_CB},
    {DIALOG_DEATH_RECIPIENT_CB, RegisterListenerType::DIALOG_DEATH_RECIPIENT_CB},
    {WINDOW_STATUS_CHANGE_CB, RegisterListenerType::WINDOW_STATUS_CHANGE_CB},
    {WINDOW_STATUS_DID_CHANGE_CB, RegisterListenerType::WINDOW_STATUS_DID_CHANGE_CB},
    {WINDOW_TITLE_BUTTON_RECT_CHANGE_CB, RegisterListenerType::WINDOW_TITLE_BUTTON_RECT_CHANGE_CB},
    {WINDOW_VISIBILITY_CHANGE_CB, RegisterListenerType::WINDOW_VISIBILITY_CHANGE_CB},
    {OCCLUSION_STATE_CHANGE_CB, RegisterListenerType::OCCLUSION_STATE_CHANGE_CB},
    {FRAME_METRICS_MEASURED_CHANGE_CB, RegisterListenerType::FRAME_METRICS_MEASURED_CHANGE_CB},
    {WINDOW_DISPLAYID_CHANGE_CB, RegisterListenerType::WINDOW_DISPLAYID_CHANGE_CB},
    {SYSTEM_DENSITY_CHANGE_CB, RegisterListenerType::SYSTEM_DENSITY_CHANGE_CB},
    {ACROSS_DISPLAYS_CHANGE_CB, RegisterListenerType::ACROSS_DISPLAYS_CHANGE_CB},
    {WINDOW_NO_INTERACTION_DETECT_CB, RegisterListenerType::WINDOW_NO_INTERACTION_DETECT_CB},
    {WINDOW_RECT_CHANGE_CB, RegisterListenerType::WINDOW_RECT_CHANGE_CB},
    {RECT_CHANGE_IN_GLOBAL_DISPLAY_CB, RegisterListenerType::RECT_CHANGE_IN_GLOBAL_DISPLAY_CB},
    {EXTENSION_SECURE_LIMIT_CHANGE_CB, RegisterListenerType::EXTENSION_SECURE_LIMIT_CHANGE_CB},
    {SUB_WINDOW_CLOSE_CB, RegisterListenerType::SUB_WINDOW_CLOSE_CB},
    {WINDOW_HIGHLIGHT_CHANGE_CB, RegisterListenerType::WINDOW_HIGHLIGHT_CHANGE_CB},
    {WINDOW_WILL_CLOSE_CB, RegisterListenerType::WINDOW_WILL_CLOSE_CB},
    {WINDOW_ROTATION_CHANGE_CB, RegisterListenerType::WINDOW_ROTATION_CHANGE_CB},
    {FREE_WINDOW_MODE_CHANGE_CB, RegisterListenerType::FREE_WINDOW_MODE_CHANGE_CB},
};
const std::map<std::string, RegisterListenerType> WINDOW_STAGE_LISTENER_MAP {
    // white register list for window stage
    {WINDOW_STAGE_EVENT_CB, RegisterListenerType::WINDOW_STAGE_EVENT_CB},
    {WINDOW_STAGE_CLOSE_CB, RegisterListenerType::WINDOW_STAGE_CLOSE_CB},
    {WINDOW_STAGE_LIFECYCLE_EVENT_CB, RegisterListenerType::WINDOW_STAGE_LIFECYCLE_EVENT_CB},
};

const std::map<CaseType, std::map<std::string, RegisterListenerType>> LISTENER_CODE_MAP {
    {CaseType::CASE_WINDOW_MANAGER, WINDOW_MANAGER_LISTENER_MAP},
    {CaseType::CASE_WINDOW, WINDOW_LISTENER_MAP},
    {CaseType::CASE_STAGE, WINDOW_STAGE_LISTENER_MAP},
};
}

JsWindowRegisterManager::JsWindowRegisterManager()
{
}

JsWindowRegisterManager::~JsWindowRegisterManager()
{
}

WmErrorCode JsWindowRegisterManager::ProcessWindowChangeRegister(sptr<JsWindowListener> listener,
    sptr<Window> window, bool isRegister, napi_env env, napi_value parameter)
{
    if (window == nullptr) {
        WLOGFE("Window is nullptr");
        return WmErrorCode::WM_ERROR_STATE_ABNORMALLY;
    }
    sptr<IWindowChangeListener> thisListener(listener);
    WmErrorCode ret = WmErrorCode::WM_OK;
    if (isRegister) {
        ret = MappingWmErrorCodeSafely(window->RegisterWindowChangeListener(thisListener));
    } else {
        ret = MappingWmErrorCodeSafely(window->UnregisterWindowChangeListener(thisListener));
    }
    return ret;
}

WmErrorCode JsWindowRegisterManager::ProcessSystemAvoidAreaChangeRegister(sptr<JsWindowListener> listener,
    sptr<Window> window, bool isRegister, napi_env env, napi_value parameter)
{
    if (window == nullptr) {
        TLOGE(WmsLogTag::WMS_IMMS, "Window is nullptr");
        return WmErrorCode::WM_ERROR_STATE_ABNORMALLY;
    }
    if (listener == nullptr) {
        TLOGE(WmsLogTag::WMS_IMMS, "listener is nullptr");
        return WmErrorCode::WM_ERROR_STATE_ABNORMALLY;
    }
    listener->SetIsDeprecatedInterface(true);
    sptr<IAvoidAreaChangedListener> thisListener(listener);
    WmErrorCode ret = WmErrorCode::WM_OK;
    if (isRegister) {
        ret = MappingWmErrorCodeSafely(window->RegisterAvoidAreaChangeListener(thisListener));
    } else {
        ret = MappingWmErrorCodeSafely(window->UnregisterAvoidAreaChangeListener(thisListener));
    }
    return ret;
}

WmErrorCode JsWindowRegisterManager::ProcessAvoidAreaChangeRegister(sptr<JsWindowListener> listener,
    sptr<Window> window, bool isRegister, napi_env env, napi_value parameter)
{
    if (window == nullptr) {
        WLOGFE("Window is nullptr");
        return WmErrorCode::WM_ERROR_STATE_ABNORMALLY;
    }
    sptr<IAvoidAreaChangedListener> thisListener(listener);
    WmErrorCode ret = WmErrorCode::WM_OK;
    if (isRegister) {
        ret = MappingWmErrorCodeSafely(window->RegisterAvoidAreaChangeListener(thisListener));
    } else {
        ret = MappingWmErrorCodeSafely(window->UnregisterAvoidAreaChangeListener(thisListener));
    }
    return ret;
}

WmErrorCode JsWindowRegisterManager::ProcessLifeCycleEventRegister(sptr<JsWindowListener> listener,
    sptr<Window> window, bool isRegister, napi_env env, napi_value parameter)
{
    if (window == nullptr) {
        WLOGFE("Window is nullptr");
        return WmErrorCode::WM_ERROR_STATE_ABNORMALLY;
    }
    sptr<IWindowLifeCycle> thisListener(listener);
    WmErrorCode ret = WmErrorCode::WM_OK;
    if (isRegister) {
        ret = MappingWmErrorCodeSafely(window->RegisterLifeCycleListener(thisListener));
    } else {
        ret = MappingWmErrorCodeSafely(window->UnregisterLifeCycleListener(thisListener));
    }
    return ret;
}

WmErrorCode JsWindowRegisterManager::ProcessWindowStageLifeCycleEventRegister(const sptr<JsWindowListener>& listener,
    const sptr<Window>& window, bool isRegister, napi_env env, napi_value parameter)
{
    if (window == nullptr) {
        TLOGE(WmsLogTag::WMS_LIFE, "Window is nullptr");
        return WmErrorCode::WM_ERROR_STATE_ABNORMALLY;
    }
    if (!window->IsSceneBoardEnabled()) {
        TLOGE(WmsLogTag::WMS_LIFE, "sceneBoard is not enabled");
        return WmErrorCode::WM_ERROR_DEVICE_NOT_SUPPORT;
    }
    sptr<IWindowStageLifeCycle> thisListener(listener);
    WmErrorCode ret = WmErrorCode::WM_OK;
    if (isRegister) {
        ret = MappingWmErrorCodeSafely(window->RegisterWindowStageLifeCycleListener(thisListener));
    } else {
        ret = MappingWmErrorCodeSafely(window->UnregisterWindowStageLifeCycleListener(thisListener));
    }
    return ret;
}

WmErrorCode JsWindowRegisterManager::ProcessOccupiedAreaChangeRegister(sptr<JsWindowListener> listener,
    sptr<Window> window, bool isRegister, napi_env env, napi_value parameter)
{
    if (window == nullptr) {
        WLOGFE("Window is nullptr");
        return WmErrorCode::WM_ERROR_STATE_ABNORMALLY;
    }
    sptr<IOccupiedAreaChangeListener> thisListener(listener);
    WmErrorCode ret = WmErrorCode::WM_OK;
    if (isRegister) {
        ret = MappingWmErrorCodeSafely(window->RegisterOccupiedAreaChangeListener(thisListener));
    } else {
        ret = MappingWmErrorCodeSafely(window->UnregisterOccupiedAreaChangeListener(thisListener));
    }
    return ret;
}

WmErrorCode JsWindowRegisterManager::ProcessKeyboardWillShowRegister(sptr<JsWindowListener> listener,
    const sptr<Window>& window, bool isRegister, napi_env env, napi_value parameter)
{
    if (window == nullptr) {
        TLOGE(WmsLogTag::WMS_KEYBOARD, "Window is nullptr");
        return WmErrorCode::WM_ERROR_STATE_ABNORMALLY;
    }
    sptr<IKeyboardWillShowListener> thisListener(listener);
    if (isRegister) {
        return MappingWmErrorCodeSafely(window->RegisterKeyboardWillShowListener(thisListener));
    } else {
        return MappingWmErrorCodeSafely(window->UnregisterKeyboardWillShowListener(thisListener));
    }
}

WmErrorCode JsWindowRegisterManager::ProcessKeyboardWillHideRegister(sptr<JsWindowListener> listener,
    const sptr<Window>& window, bool isRegister, napi_env env, napi_value parameter)
{
    if (window == nullptr) {
        TLOGE(WmsLogTag::WMS_KEYBOARD, "Window is nullptr");
        return WmErrorCode::WM_ERROR_STATE_ABNORMALLY;
    }
    sptr<IKeyboardWillHideListener> thisListener(listener);
    WmErrorCode ret = WmErrorCode::WM_OK;
    if (isRegister) {
        ret = MappingWmErrorCodeSafely(window->RegisterKeyboardWillHideListener(thisListener));
    } else {
        ret = MappingWmErrorCodeSafely(window->UnregisterKeyboardWillHideListener(thisListener));
    }
    return ret;
}

WmErrorCode JsWindowRegisterManager::ProcessKeyboardDidShowRegister(sptr<JsWindowListener> listener,
    sptr<Window> window, bool isRegister, napi_env env, napi_value parameter)
{
    if (window == nullptr) {
        TLOGE(WmsLogTag::WMS_KEYBOARD, "Window is nullptr");
        return WmErrorCode::WM_ERROR_STATE_ABNORMALLY;
    }
    sptr<IKeyboardDidShowListener> thisListener(listener);
    WmErrorCode ret = WmErrorCode::WM_OK;
    if (isRegister) {
        ret = MappingWmErrorCodeSafely(window->RegisterKeyboardDidShowListener(thisListener));
    } else {
        ret = MappingWmErrorCodeSafely(window->UnregisterKeyboardDidShowListener(thisListener));
    }
    return ret;
}

WmErrorCode JsWindowRegisterManager::ProcessKeyboardDidHideRegister(sptr<JsWindowListener> listener,
    sptr<Window> window, bool isRegister, napi_env env, napi_value parameter)
{
    if (window == nullptr) {
        TLOGE(WmsLogTag::WMS_KEYBOARD, "Window is nullptr");
        return WmErrorCode::WM_ERROR_STATE_ABNORMALLY;
    }
    sptr<IKeyboardDidHideListener> thisListener(listener);
    WmErrorCode ret = WmErrorCode::WM_OK;
    if (isRegister) {
        ret = MappingWmErrorCodeSafely(window->RegisterKeyboardDidHideListener(thisListener));
    } else {
        ret = MappingWmErrorCodeSafely(window->UnregisterKeyboardDidHideListener(thisListener));
    }
    return ret;
}

WmErrorCode JsWindowRegisterManager::ProcessSystemBarChangeRegister(sptr<JsWindowListener> listener,
    sptr<Window> window, bool isRegister, napi_env env, napi_value parameter)
{
    sptr<ISystemBarChangedListener> thisListener(listener);
    WmErrorCode ret = WmErrorCode::WM_OK;
    if (isRegister) {
        ret = MappingWmErrorCodeSafely(
            SingletonContainer::Get<WindowManager>().RegisterSystemBarChangedListener(thisListener));
    } else {
        ret = MappingWmErrorCodeSafely(
            SingletonContainer::Get<WindowManager>().UnregisterSystemBarChangedListener(thisListener));
    }
    return ret;
}

WmErrorCode JsWindowRegisterManager::ProcessGestureNavigationEnabledChangeRegister(sptr<JsWindowListener> listener,
    sptr<Window> window, bool isRegister, napi_env env, napi_value parameter)
{
    sptr<IGestureNavigationEnabledChangedListener> thisListener(listener);
    WmErrorCode ret;
    if (isRegister) {
        ret = MappingWmErrorCodeSafely(
            SingletonContainer::Get<WindowManager>().RegisterGestureNavigationEnabledChangedListener(thisListener));
    } else {
        ret = MappingWmErrorCodeSafely(
            SingletonContainer::Get<WindowManager>().UnregisterGestureNavigationEnabledChangedListener(thisListener));
    }
    return ret;
}

WmErrorCode JsWindowRegisterManager::ProcessWaterMarkFlagChangeRegister(sptr<JsWindowListener> listener,
    sptr<Window> window, bool isRegister, napi_env env, napi_value parameter)
{
    sptr<IWaterMarkFlagChangedListener> thisListener(listener);
    WmErrorCode ret;
    if (isRegister) {
        ret = MappingWmErrorCodeSafely(
            SingletonContainer::Get<WindowManager>().RegisterWaterMarkFlagChangedListener(thisListener));
    } else {
        ret = MappingWmErrorCodeSafely(
            SingletonContainer::Get<WindowManager>().UnregisterWaterMarkFlagChangedListener(thisListener));
    }
    return ret;
}
WmErrorCode JsWindowRegisterManager::ProcessTouchOutsideRegister(sptr<JsWindowListener> listener,
    sptr<Window> window, bool isRegister, napi_env env, napi_value parameter)
{
    WLOGI("called");
    if (window == nullptr) {
        return WmErrorCode::WM_ERROR_STATE_ABNORMALLY;
    }
    sptr<ITouchOutsideListener> thisListener(listener);
    WmErrorCode ret = WmErrorCode::WM_OK;
    if (isRegister) {
        ret = MappingWmErrorCodeSafely(window->RegisterTouchOutsideListener(thisListener));
    } else {
        ret = MappingWmErrorCodeSafely(window->UnregisterTouchOutsideListener(thisListener));
    }
    return ret;
}

WmErrorCode JsWindowRegisterManager::ProcessWindowVisibilityChangeRegister(sptr<JsWindowListener> listener,
    sptr<Window> window, bool isRegister, napi_env env, napi_value parameter)
{
    WLOGD("called");
    if (window == nullptr || listener == nullptr) {
        return WmErrorCode::WM_ERROR_STATE_ABNORMALLY;
    }
    sptr<IWindowVisibilityChangedListener> thisListener(listener);
    WmErrorCode ret = WmErrorCode::WM_OK;
    if (isRegister) {
        ret = MappingWmErrorCodeSafely(window->RegisterWindowVisibilityChangeListener(thisListener));
    } else {
        ret = MappingWmErrorCodeSafely(window->UnregisterWindowVisibilityChangeListener(thisListener));
    }
    return ret;
}

WmErrorCode JsWindowRegisterManager::ProcessOcclusionStateChangeRegister(sptr<JsWindowListener> listener,
    sptr<Window> window, bool isRegister, napi_env env, napi_value parameter)
{
    if (window == nullptr || listener == nullptr) {
        TLOGE(WmsLogTag::WMS_ATTRIBUTE, "window or listener is null");
        return WmErrorCode::WM_ERROR_STATE_ABNORMALLY;
    }
    WMError retCode = WMError::WM_OK;
    sptr<IOcclusionStateChangedListener> thisListener(listener);
    if (isRegister) {
        retCode = window->RegisterOcclusionStateChangeListener(thisListener);
    } else {
        retCode = window->UnregisterOcclusionStateChangeListener(thisListener);
    }
    TLOGI(WmsLogTag::WMS_ATTRIBUTE, "retCode=%{public}d", static_cast<int32_t>(retCode));
    auto retErrCode = WmErrorCode::WM_ERROR_SYSTEM_ABNORMALLY;
    if (WM_JS_TO_ERROR_CODE_MAP.count(retCode) > 0) {
        retErrCode = WM_JS_TO_ERROR_CODE_MAP.at(retCode);
    }
    return retErrCode;
}

WmErrorCode JsWindowRegisterManager::ProcessFrameMetricsMeasuredChangeRegister(sptr<JsWindowListener> listener,
    sptr<Window> window, bool isRegister, napi_env env, napi_value parameter)
{
    if (window == nullptr || listener == nullptr) {
        TLOGE(WmsLogTag::WMS_ATTRIBUTE, "window or listener is null");
        return WmErrorCode::WM_ERROR_STATE_ABNORMALLY;
    }
    WMError retCode = WMError::WM_OK;
    sptr<IFrameMetricsChangedListener> thisListener(listener);
    if (isRegister) {
        retCode = window->RegisterFrameMetricsChangeListener(thisListener);
    } else {
        retCode = window->UnregisterFrameMetricsChangeListener(thisListener);
    }
    TLOGI(WmsLogTag::WMS_ATTRIBUTE, "retCode=%{public}d", static_cast<int32_t>(retCode));
    auto retErrCode = WmErrorCode::WM_ERROR_STATE_ABNORMALLY;
    if (WM_JS_TO_ERROR_CODE_MAP.count(retCode) > 0) {
        retErrCode = WM_JS_TO_ERROR_CODE_MAP.at(retCode);
    }
    return retErrCode;
}

WmErrorCode JsWindowRegisterManager::ProcessDisplayIdChangeRegister(const sptr<JsWindowListener>& listener,
    const sptr<Window>& window, bool isRegister, napi_env env, napi_value parameter)
{
    TLOGD(WmsLogTag::WMS_ATTRIBUTE, "in");
    if (window == nullptr || listener == nullptr) {
        TLOGE(WmsLogTag::WMS_ATTRIBUTE, "window or listener is nullptr");
        return WmErrorCode::WM_ERROR_STATE_ABNORMALLY;
    }
    IDisplayIdChangeListenerSptr thisListener(listener);
    WmErrorCode ret = isRegister ?
        MappingWmErrorCodeSafely(window->RegisterDisplayIdChangeListener(thisListener)) :
        MappingWmErrorCodeSafely(window->UnregisterDisplayIdChangeListener(thisListener));
    return ret;
}

WmErrorCode JsWindowRegisterManager::ProcessSystemDensityChangeRegister(const sptr<JsWindowListener>& listener,
    const sptr<Window>& window, bool isRegister, napi_env env, napi_value parameter)
{
    TLOGD(WmsLogTag::WMS_ATTRIBUTE, "in");
    if (window == nullptr || listener == nullptr) {
        TLOGE(WmsLogTag::WMS_ATTRIBUTE, "window or listener is nullptr");
        return WmErrorCode::WM_ERROR_STATE_ABNORMALLY;
    }
    return isRegister ?
        MappingWmErrorCodeSafely(window->RegisterSystemDensityChangeListener(listener)) :
        MappingWmErrorCodeSafely(window->UnregisterSystemDensityChangeListener(listener));
}

WmErrorCode JsWindowRegisterManager::ProcessAcrossDisplaysChangeRegister(const sptr<JsWindowListener>& listener,
    const sptr<Window>& window, bool isRegister, napi_env env, napi_value parameter)
{
    TLOGD(WmsLogTag::WMS_ATTRIBUTE, "in");
    if (window == nullptr) {
        TLOGE(WmsLogTag::WMS_ATTRIBUTE, "window is null");
        return WmErrorCode::WM_ERROR_STATE_ABNORMALLY;
    }
    if (listener == nullptr) {
        TLOGE(WmsLogTag::WMS_ATTRIBUTE, "listener is null");
        return WmErrorCode::WM_ERROR_STATE_ABNORMALLY;
    }
    return isRegister ?
        MappingWmErrorCodeSafely(window->RegisterAcrossDisplaysChangeListener(listener)) :
        MappingWmErrorCodeSafely(window->UnRegisterAcrossDisplaysChangeListener(listener));
}

WmErrorCode JsWindowRegisterManager::ProcessWindowNoInteractionRegister(sptr<JsWindowListener> listener,
    sptr<Window> window, bool isRegister, napi_env env, napi_value parameter)
{
    if (window == nullptr) {
        return WmErrorCode::WM_ERROR_STATE_ABNORMALLY;
    }
    sptr<IWindowNoInteractionListener> thisListener(listener);

    if (!isRegister) {
        return MappingWmErrorCodeSafely(window->UnregisterWindowNoInteractionListener(thisListener));
    }

    int64_t timeout = 0;
    if (parameter == nullptr || !ConvertFromJsNumber(env, parameter, timeout)) {
        WLOGFE("Failed to convert parameter to timeout");
        return WmErrorCode::WM_ERROR_INVALID_PARAM;
    }

    constexpr int64_t secToMicrosecRatio = 1000;
    constexpr int64_t noInteractionMax = LLONG_MAX / secToMicrosecRatio;
    if (timeout <= 0 || (timeout > noInteractionMax)) {
        WLOGFE("invalid parameter: no-interaction-timeout %{public}" PRId64 " is not in(0s~%{public}" PRId64,
            timeout, noInteractionMax);
        return WmErrorCode::WM_ERROR_INVALID_PARAM;
    }

    thisListener->SetTimeout(timeout * secToMicrosecRatio);

    return MappingWmErrorCodeSafely(window->RegisterWindowNoInteractionListener(thisListener));
}

WmErrorCode JsWindowRegisterManager::ProcessScreenshotRegister(sptr<JsWindowListener> listener,
    sptr<Window> window, bool isRegister, napi_env env, napi_value parameter)
{
    WLOGI("called");
    if (window == nullptr) {
        WLOGFE("%{public}sregister screenshot listener failed. window is null", isRegister? "" : "un");
        return WmErrorCode::WM_ERROR_STATE_ABNORMALLY;
    }
    sptr<IScreenshotListener> thisListener(listener);
    WmErrorCode ret = WmErrorCode::WM_OK;
    if (isRegister) {
        ret = MappingWmErrorCodeSafely(window->RegisterScreenshotListener(thisListener));
    } else {
        ret = MappingWmErrorCodeSafely(window->UnregisterScreenshotListener(thisListener));
    }
    return ret;
}

WmErrorCode JsWindowRegisterManager::ProcessScreenshotAppEventRegister(sptr<JsWindowListener> listener,
    sptr<Window> window, bool isRegister, napi_env env, napi_value parameter)
{
    TLOGD(WmsLogTag::WMS_ATTRIBUTE, "in");
    if (window == nullptr) {
        TLOGE(WmsLogTag::WMS_ATTRIBUTE, "window is null");
        return WmErrorCode::WM_ERROR_STATE_ABNORMALLY;
    }
    if (listener == nullptr) {
        TLOGE(WmsLogTag::WMS_ATTRIBUTE, "listener is null");
        return WmErrorCode::WM_ERROR_STATE_ABNORMALLY;
    }
    return isRegister ?
        MappingWmErrorCodeSafely(window->RegisterScreenshotAppEventListener(listener)) :
        MappingWmErrorCodeSafely(window->UnregisterScreenshotAppEventListener(listener));
}

WmErrorCode JsWindowRegisterManager::ProcessDialogTargetTouchRegister(sptr<JsWindowListener> listener,
    sptr<Window> window, bool isRegister, napi_env env, napi_value parameter)
{
    if (window == nullptr) {
        return WmErrorCode::WM_ERROR_STATE_ABNORMALLY;
    }
    sptr<IDialogTargetTouchListener> thisListener(listener);
    WmErrorCode ret = WmErrorCode::WM_OK;
    if (isRegister) {
        ret = MappingWmErrorCodeSafely(window->RegisterDialogTargetTouchListener(thisListener));
    } else {
        ret = MappingWmErrorCodeSafely(window->UnregisterDialogTargetTouchListener(thisListener));
    }
    return ret;
}

WmErrorCode JsWindowRegisterManager::ProcessDialogDeathRecipientRegister(sptr<JsWindowListener> listener,
    sptr<Window> window, bool isRegister, napi_env env, napi_value parameter)
{
    if (window == nullptr) {
        return WmErrorCode::WM_ERROR_STATE_ABNORMALLY;
    }
    sptr<IDialogDeathRecipientListener> thisListener(listener);
    if (isRegister) {
        window->RegisterDialogDeathRecipientListener(thisListener);
    } else {
        window->UnregisterDialogDeathRecipientListener(thisListener);
    }
    return WmErrorCode::WM_OK;
}

WmErrorCode JsWindowRegisterManager::ProcessWindowTitleButtonRectChangeRegister(sptr<JsWindowListener> listener,
    sptr<Window> window, bool isRegister, napi_env env, napi_value parameter)
{
    WLOGD("called");
    if (window == nullptr) {
        WLOGFE("Window is nullptr");
        return WmErrorCode::WM_ERROR_STATE_ABNORMALLY;
    }
    sptr<IWindowTitleButtonRectChangedListener> thisListener(listener);
    WmErrorCode ret = WmErrorCode::WM_OK;
    if (isRegister) {
        ret = MappingWmErrorCodeSafely(window->RegisterWindowTitleButtonRectChangeListener(thisListener));
    } else {
        ret = MappingWmErrorCodeSafely(window->UnregisterWindowTitleButtonRectChangeListener(thisListener));
    }
    return ret;
}

bool JsWindowRegisterManager::IsCallbackRegistered(napi_env env, std::string type, napi_value jsListenerObject)
{
    if (jsCbMap_.empty() || jsCbMap_.find(type) == jsCbMap_.end()) {
        TLOGD(WmsLogTag::DEFAULT, "Method %{public}s has not been registerted", type.c_str());
        return false;
    }

    for (auto iter = jsCbMap_[type].begin(); iter != jsCbMap_[type].end(); ++iter) {
        bool isEquals = false;
        napi_strict_equals(env, jsListenerObject, iter->first->GetNapiValue(), &isEquals);
        if (isEquals) {
            WLOGFE("Method %{public}s has already been registered", type.c_str());
            return true;
        }
    }
    return false;
}

WmErrorCode JsWindowRegisterManager::RegisterListener(sptr<Window> window, std::string type,
    CaseType caseType, napi_env env, napi_value callback, napi_value parameter)
{
    std::lock_guard<std::mutex> lock(mtx_);
    if (IsCallbackRegistered(env, type, callback)) {
        return WmErrorCode::WM_OK;
    }
    auto iterCaseType = LISTENER_CODE_MAP.find(caseType);
    if (iterCaseType == LISTENER_CODE_MAP.end()) {
        WLOGFE("CaseType %{public}u is not supported", static_cast<uint32_t>(caseType));
        return WmErrorCode::WM_ERROR_STATE_ABNORMALLY;
    }
    auto iterCallbackType = iterCaseType->second.find(type);
    if (iterCallbackType == iterCaseType->second.end()) {
        WLOGFE("Type %{public}s is not supported", type.c_str());
        return WmErrorCode::WM_ERROR_STATE_ABNORMALLY;
    }
    RegisterListenerType listenerType = iterCallbackType->second;
    napi_ref result = nullptr;
    napi_create_reference(env, callback, 1, &result);
    std::shared_ptr<NativeReference> callbackRef(reinterpret_cast<NativeReference*>(result));
    sptr<JsWindowListener> windowManagerListener = new(std::nothrow) JsWindowListener(env, callbackRef, caseType);
    if (windowManagerListener == nullptr) {
        WLOGFE("New JsWindowListener failed");
        return WmErrorCode::WM_ERROR_STATE_ABNORMALLY;
    }
    windowManagerListener->SetMainEventHandler();
    WmErrorCode ret = ProcessListener(listenerType, caseType, windowManagerListener, window, true,
        env, parameter);
    if (ret != WmErrorCode::WM_OK) {
        WLOGFE("Register type %{public}s failed", type.c_str());
        return ret;
    }
    jsCbMap_[type][callbackRef] = windowManagerListener;
    TLOGI(WmsLogTag::DEFAULT, "[%{public}s, %{public}zu]", type.c_str(), jsCbMap_[type].size());
    return WmErrorCode::WM_OK;
}

WmErrorCode JsWindowRegisterManager::ProcessListener(RegisterListenerType registerListenerType, CaseType caseType,
    const sptr<JsWindowListener>& windowManagerListener, const sptr<Window>& window, bool isRegister,
    napi_env env, napi_value parameter)
{
    if (caseType == CaseType::CASE_WINDOW_MANAGER) {
        switch (static_cast<uint32_t>(registerListenerType)) {
            case static_cast<uint32_t>(RegisterListenerType::SYSTEM_BAR_TINT_CHANGE_CB):
                return ProcessSystemBarChangeRegister(windowManagerListener, window, isRegister, env, parameter);
            case static_cast<uint32_t>(RegisterListenerType::GESTURE_NAVIGATION_ENABLED_CHANGE_CB):
                return ProcessGestureNavigationEnabledChangeRegister(windowManagerListener, window, isRegister,
                    env, parameter);
            case static_cast<uint32_t>(RegisterListenerType::WATER_MARK_FLAG_CHANGE_CB):
                return ProcessWaterMarkFlagChangeRegister(windowManagerListener, window, isRegister, env, parameter);
            default:
                WLOGFE("RegisterListenerType %{public}u is not supported",
                    static_cast<uint32_t>(registerListenerType));
                return WmErrorCode::WM_ERROR_INVALID_PARAM;
        }
    } else if (caseType == CaseType::CASE_WINDOW) {
        switch (static_cast<uint32_t>(registerListenerType)) {
            case static_cast<uint32_t>(RegisterListenerType::WINDOW_SIZE_CHANGE_CB):
                return ProcessWindowChangeRegister(windowManagerListener, window, isRegister, env, parameter);
            case static_cast<uint32_t>(RegisterListenerType::SYSTEM_AVOID_AREA_CHANGE_CB):
                return ProcessSystemAvoidAreaChangeRegister(windowManagerListener, window, isRegister, env, parameter);
            case static_cast<uint32_t>(RegisterListenerType::AVOID_AREA_CHANGE_CB):
                return ProcessAvoidAreaChangeRegister(windowManagerListener, window, isRegister, env, parameter);
            case static_cast<uint32_t>(RegisterListenerType::LIFECYCLE_EVENT_CB):
                return ProcessLifeCycleEventRegister(windowManagerListener, window, isRegister, env, parameter);
            case static_cast<uint32_t>(RegisterListenerType::WINDOW_EVENT_CB):
                return ProcessLifeCycleEventRegister(windowManagerListener, window, isRegister, env, parameter);
            case static_cast<uint32_t>(RegisterListenerType::KEYBOARD_HEIGHT_CHANGE_CB):
                return ProcessOccupiedAreaChangeRegister(windowManagerListener, window, isRegister, env, parameter);
            case static_cast<uint32_t>(RegisterListenerType::KEYBOARD_WILL_SHOW_CB):
                return ProcessKeyboardWillShowRegister(windowManagerListener, window, isRegister, env, parameter);
            case static_cast<uint32_t>(RegisterListenerType::KEYBOARD_WILL_HIDE_CB):
                return ProcessKeyboardWillHideRegister(windowManagerListener, window, isRegister, env, parameter);
            case static_cast<uint32_t>(RegisterListenerType::KEYBOARD_DID_SHOW_CB):
                return ProcessKeyboardDidShowRegister(windowManagerListener, window, isRegister, env, parameter);
            case static_cast<uint32_t>(RegisterListenerType::KEYBOARD_DID_HIDE_CB):
                return ProcessKeyboardDidHideRegister(windowManagerListener, window, isRegister, env, parameter);
            case static_cast<uint32_t>(RegisterListenerType::TOUCH_OUTSIDE_CB):
                return ProcessTouchOutsideRegister(windowManagerListener, window, isRegister, env, parameter);
            case static_cast<uint32_t>(RegisterListenerType::SCREENSHOT_EVENT_CB):
                return ProcessScreenshotRegister(windowManagerListener, window, isRegister, env, parameter);
            case static_cast<uint32_t>(RegisterListenerType::SCREENSHOT_APP_EVENT_CB):
                return ProcessScreenshotAppEventRegister(windowManagerListener, window, isRegister, env, parameter);
            case static_cast<uint32_t>(RegisterListenerType::DIALOG_TARGET_TOUCH_CB):
                return ProcessDialogTargetTouchRegister(windowManagerListener, window, isRegister, env, parameter);
            case static_cast<uint32_t>(RegisterListenerType::DIALOG_DEATH_RECIPIENT_CB):
                return ProcessDialogDeathRecipientRegister(windowManagerListener, window, isRegister, env, parameter);
            case static_cast<uint32_t>(RegisterListenerType::WINDOW_STATUS_CHANGE_CB):
                return ProcessWindowStatusChangeRegister(windowManagerListener, window, isRegister, env, parameter);
            case static_cast<uint32_t>(RegisterListenerType::WINDOW_STATUS_DID_CHANGE_CB):
                return ProcessWindowStatusDidChangeRegister(windowManagerListener, window, isRegister, env, parameter);
            case static_cast<uint32_t>(RegisterListenerType::WINDOW_TITLE_BUTTON_RECT_CHANGE_CB):
                return ProcessWindowTitleButtonRectChangeRegister(windowManagerListener, window, isRegister, env,
                    parameter);
            case static_cast<uint32_t>(RegisterListenerType::WINDOW_VISIBILITY_CHANGE_CB):
                return ProcessWindowVisibilityChangeRegister(windowManagerListener, window, isRegister, env, parameter);
            case static_cast<uint32_t>(RegisterListenerType::OCCLUSION_STATE_CHANGE_CB):
                return ProcessOcclusionStateChangeRegister(windowManagerListener, window, isRegister, env, parameter);
            case static_cast<uint32_t>(RegisterListenerType::FRAME_METRICS_MEASURED_CHANGE_CB):
                return ProcessFrameMetricsMeasuredChangeRegister(windowManagerListener, window, isRegister, env,
                    parameter);
            case static_cast<uint32_t>(RegisterListenerType::WINDOW_DISPLAYID_CHANGE_CB):
                return ProcessDisplayIdChangeRegister(windowManagerListener, window, isRegister, env, parameter);
            case static_cast<uint32_t>(RegisterListenerType::SYSTEM_DENSITY_CHANGE_CB):
                return ProcessSystemDensityChangeRegister(windowManagerListener, window, isRegister, env, parameter);
            case static_cast<uint32_t>(RegisterListenerType::ACROSS_DISPLAYS_CHANGE_CB):
                return ProcessAcrossDisplaysChangeRegister(windowManagerListener, window, isRegister, env, parameter);
            case static_cast<uint32_t>(RegisterListenerType::WINDOW_NO_INTERACTION_DETECT_CB):
                return ProcessWindowNoInteractionRegister(windowManagerListener, window, isRegister, env, parameter);
            case static_cast<uint32_t>(RegisterListenerType::WINDOW_RECT_CHANGE_CB):
                return ProcessWindowRectChangeRegister(windowManagerListener, window, isRegister, env, parameter);
            case static_cast<uint32_t>(RegisterListenerType::RECT_CHANGE_IN_GLOBAL_DISPLAY_CB):
                return ProcessRectChangeInGlobalDisplayRegister(
                    windowManagerListener, window, isRegister, env, parameter);
            case static_cast<uint32_t>(RegisterListenerType::EXTENSION_SECURE_LIMIT_CHANGE_CB):
                return ProcessExtensionSecureLimitChangeRegister(
                    windowManagerListener, window, isRegister, env, parameter);
            case static_cast<uint32_t>(RegisterListenerType::SUB_WINDOW_CLOSE_CB):
                return ProcessSubWindowCloseRegister(windowManagerListener, window, isRegister, env, parameter);
            case static_cast<uint32_t>(RegisterListenerType::WINDOW_HIGHLIGHT_CHANGE_CB):
                return ProcessWindowHighlightChangeRegister(windowManagerListener, window, isRegister, env, parameter);
            case static_cast<uint32_t>(RegisterListenerType::WINDOW_WILL_CLOSE_CB):
                return ProcessWindowWillCloseRegister(windowManagerListener, window, isRegister, env, parameter);
            case static_cast<uint32_t>(RegisterListenerType::WINDOW_ROTATION_CHANGE_CB):
                return ProcessWindowRotationChangeRegister(windowManagerListener, window, isRegister, env, parameter);
            case static_cast<uint32_t>(RegisterListenerType::FREE_WINDOW_MODE_CHANGE_CB):
                return ProcessFreeWindowModeChangeRegister(windowManagerListener, window, isRegister, env, parameter);
            default:
                WLOGFE("RegisterListenerType %{public}u is not supported",
                    static_cast<uint32_t>(registerListenerType));
                return WmErrorCode::WM_ERROR_INVALID_PARAM;
        }
    } else if (caseType == CaseType::CASE_STAGE) {
        switch (registerListenerType) {
            case RegisterListenerType::WINDOW_STAGE_EVENT_CB:
                return ProcessLifeCycleEventRegister(windowManagerListener, window, isRegister, env, parameter);
            case RegisterListenerType::WINDOW_STAGE_CLOSE_CB:
                return ProcessMainWindowCloseRegister(windowManagerListener, window, isRegister, env, parameter);
            case RegisterListenerType::WINDOW_STAGE_LIFECYCLE_EVENT_CB:
                return ProcessWindowStageLifeCycleEventRegister(windowManagerListener, window, isRegister, env,
                    parameter);
            default:
                TLOGE(WmsLogTag::DEFAULT, "RegisterListenerType %{public}u is not supported",
                    static_cast<uint32_t>(registerListenerType));
                return WmErrorCode::WM_ERROR_INVALID_PARAM;
        }
    }
    return WmErrorCode::WM_OK;
}

WmErrorCode JsWindowRegisterManager::UnregisterListener(sptr<Window> window, std::string type,
    CaseType caseType, napi_env env, napi_value value)
{
    std::lock_guard<std::mutex> lock(mtx_);
    if (jsCbMap_.empty() || jsCbMap_.find(type) == jsCbMap_.end()) {
        WLOGFW("Type %{public}s was not registerted", type.c_str());
        return WmErrorCode::WM_OK;
    }
    auto iterCaseType = LISTENER_CODE_MAP.find(caseType);
    if (iterCaseType == LISTENER_CODE_MAP.end()) {
        WLOGFE("CaseType %{public}u is not supported", static_cast<uint32_t>(caseType));
        return WmErrorCode::WM_ERROR_STATE_ABNORMALLY;
    }
    auto iterCallbackType = iterCaseType->second.find(type);
    if (iterCallbackType == iterCaseType->second.end()) {
        WLOGFE("Type %{public}s is not supported", type.c_str());
        return WmErrorCode::WM_ERROR_STATE_ABNORMALLY;
    }
    RegisterListenerType listenerType = iterCallbackType->second;
    if (value == nullptr) {
        for (auto it = jsCbMap_[type].begin(); it != jsCbMap_[type].end();) {
            if (it->second->CanCancelUnregister(type) != WmErrorCode::WM_OK) {
                return WmErrorCode::WM_ERROR_STATE_ABNORMALLY;
            }
            WmErrorCode ret = ProcessListener(listenerType, caseType, it->second, window,
                false, env, nullptr);
            if (ret != WmErrorCode::WM_OK) {
                WLOGFE("Unregister type %{public}s failed, no value", type.c_str());
                return ret;
            }
            jsCbMap_[type].erase(it++);
        }
    } else {
        bool findFlag = false;
        for (auto it = jsCbMap_[type].begin(); it != jsCbMap_[type].end(); ++it) {
            bool isEquals = false;
            napi_strict_equals(env, value, it->first->GetNapiValue(), &isEquals);
            if (!isEquals) {
                continue;
            }
            if (it->second->CanCancelUnregister(type) != WmErrorCode::WM_OK) {
                return WmErrorCode::WM_ERROR_STATE_ABNORMALLY;
            }
            findFlag = true;
            WmErrorCode ret = ProcessListener(listenerType, caseType, it->second, window,
                false, env, nullptr);
            if (ret != WmErrorCode::WM_OK) {
                WLOGFE("Unregister type %{public}s failed", type.c_str());
                return ret;
            }
            jsCbMap_[type].erase(it);
            break;
        }
        if (!findFlag) {
            WLOGFW("Unregister type %{public}s failed because not found callback!", type.c_str());
            return WmErrorCode::WM_OK;
        }
    }
    WLOGI("Unregister type %{public}s success! callback map size: %{public}zu",
        type.c_str(), jsCbMap_[type].size());
    // erase type when there is no callback in one type
    if (jsCbMap_[type].empty()) {
        jsCbMap_.erase(type);
    }
    return WmErrorCode::WM_OK;
}

WmErrorCode JsWindowRegisterManager::ProcessWindowStatusChangeRegister(sptr<JsWindowListener> listener,
    sptr<Window> window, bool isRegister, napi_env env, napi_value parameter)
{
    if (window == nullptr) {
        WLOGFE("Window is nullptr");
        return WmErrorCode::WM_ERROR_STATE_ABNORMALLY;
    }
    sptr<IWindowStatusChangeListener> thisListener(listener);
    WmErrorCode ret = WmErrorCode::WM_OK;
    if (isRegister) {
        ret = MappingWmErrorCodeSafely(window->RegisterWindowStatusChangeListener(thisListener));
    } else {
        ret = MappingWmErrorCodeSafely(window->UnregisterWindowStatusChangeListener(thisListener));
    }
    return ret;
}

WmErrorCode JsWindowRegisterManager::ProcessWindowStatusDidChangeRegister(sptr<JsWindowListener> listener,
    sptr<Window> window, bool isRegister, napi_env env, napi_value parameter)
{
    if (window == nullptr) {
        TLOGE(WmsLogTag::WMS_LAYOUT, "Window is null");
        return WmErrorCode::WM_ERROR_STATE_ABNORMALLY;
    }
    sptr<IWindowStatusDidChangeListener> thisListener(listener);
    WmErrorCode ret = WmErrorCode::WM_OK;
    if (isRegister) {
        ret = MappingWmErrorCodeSafely(window->RegisterWindowStatusDidChangeListener(thisListener));
    } else {
        ret = MappingWmErrorCodeSafely(window->UnregisterWindowStatusDidChangeListener(thisListener));
    }
    return ret;
}

WmErrorCode JsWindowRegisterManager::ProcessWindowRectChangeRegister(sptr<JsWindowListener> listener,
    sptr<Window> window, bool isRegister, napi_env env, napi_value parameter)
{
    if (window == nullptr) {
        return WmErrorCode::WM_ERROR_STATE_ABNORMALLY;
    }
    sptr<IWindowRectChangeListener> thisListener(listener);
    WmErrorCode ret = WmErrorCode::WM_OK;
    if (isRegister) {
        ret = MappingWmErrorCodeSafely(window->RegisterWindowRectChangeListener(thisListener));
    } else {
        ret = MappingWmErrorCodeSafely(window->UnregisterWindowRectChangeListener(thisListener));
    }
    return ret;
}

WmErrorCode JsWindowRegisterManager::ProcessRectChangeInGlobalDisplayRegister(
    const sptr<JsWindowListener>& listener, const sptr<Window>& window,
    bool isRegister, napi_env env, napi_value parameter)
{
    if (!window) {
        TLOGE(WmsLogTag::WMS_LAYOUT, "window is nullptr");
        return WmErrorCode::WM_ERROR_STATE_ABNORMALLY;
    }
    WMError ret = WMError::WM_OK;
    if (isRegister) {
        ret = window->RegisterRectChangeInGlobalDisplayListener(listener);
    } else {
        ret = window->UnregisterRectChangeInGlobalDisplayListener(listener);
    }
    auto it = WM_JS_TO_ERROR_CODE_MAP.find(ret);
    return it != WM_JS_TO_ERROR_CODE_MAP.end() ? it->second : WmErrorCode::WM_ERROR_STATE_ABNORMALLY;
}

WmErrorCode JsWindowRegisterManager::ProcessExtensionSecureLimitChangeRegister(sptr<JsWindowListener> listener,
    sptr<Window> window, bool isRegister, napi_env env, napi_value parameter)
{
    if (window == nullptr) {
        return WmErrorCode::WM_ERROR_STATE_ABNORMALLY;
    }
    sptr<IExtensionSecureLimitChangeListener> thisListener(listener);
    WmErrorCode ret = WmErrorCode::WM_OK;
    if (isRegister) {
        ret = MappingWmErrorCodeSafely(window->RegisterExtensionSecureLimitChangeListener(thisListener));
    } else {
        ret = MappingWmErrorCodeSafely(window->UnregisterExtensionSecureLimitChangeListener(thisListener));
    }
    return ret;
}

WmErrorCode JsWindowRegisterManager::ProcessSubWindowCloseRegister(sptr<JsWindowListener> listener,
    sptr<Window> window, bool isRegister, napi_env env, napi_value parameter)
{
    if (window == nullptr) {
        return WmErrorCode::WM_ERROR_STATE_ABNORMALLY;
    }
    sptr<ISubWindowCloseListener> thisListener(listener);
    WmErrorCode ret = WmErrorCode::WM_OK;
    if (isRegister) {
        ret = MappingWmErrorCodeSafely(window->RegisterSubWindowCloseListeners(thisListener));
    } else {
        ret = MappingWmErrorCodeSafely(window->UnregisterSubWindowCloseListeners(thisListener));
    }
    return ret;
}

WmErrorCode JsWindowRegisterManager::ProcessMainWindowCloseRegister(const sptr<JsWindowListener>& listener,
    const sptr<Window>& window, bool isRegister, napi_env env, napi_value parameter)
{
    if (window == nullptr) {
        return WmErrorCode::WM_ERROR_STATE_ABNORMALLY;
    }
    WmErrorCode ret = isRegister ?
        MappingWmErrorCodeSafely(window->RegisterMainWindowCloseListeners(listener)) :
        MappingWmErrorCodeSafely(window->UnregisterMainWindowCloseListeners(listener));
    return ret;
}

WmErrorCode JsWindowRegisterManager::ProcessWindowWillCloseRegister(const sptr<JsWindowListener>& listener,
    const sptr<Window>& window, bool isRegister, napi_env env, napi_value parameter)
{
    if (window == nullptr) {
        return WmErrorCode::WM_ERROR_STATE_ABNORMALLY;
    }
    WmErrorCode ret = isRegister ?
        MappingWmErrorCodeSafely(window->RegisterWindowWillCloseListeners(listener)) :
        MappingWmErrorCodeSafely(window->UnRegisterWindowWillCloseListeners(listener));
    return ret;
}

WmErrorCode JsWindowRegisterManager::ProcessWindowHighlightChangeRegister(const sptr<JsWindowListener>& listener,
    const sptr<Window>& window, bool isRegister, napi_env env, napi_value parameter)
{
    if (window == nullptr) {
        return WmErrorCode::WM_ERROR_STATE_ABNORMALLY;
    }
    sptr<IWindowHighlightChangeListener> thisListener(listener);
    WmErrorCode ret = WmErrorCode::WM_OK;
    if (isRegister) {
        ret = MappingWmErrorCodeSafely(window->RegisterWindowHighlightChangeListeners(thisListener));
    } else {
        ret = MappingWmErrorCodeSafely(window->UnregisterWindowHighlightChangeListeners(thisListener));
    }
    return ret;
}

WmErrorCode JsWindowRegisterManager::ProcessWindowRotationChangeRegister(const sptr<JsWindowListener>& listener,
    const sptr<Window>& window, bool isRegister, napi_env env, napi_value parameter)
{
    if (window == nullptr || listener == nullptr) {
        return WmErrorCode::WM_ERROR_STATE_ABNORMALLY;
    }
    sptr<IWindowRotationChangeListener> thisListener(listener);
    WmErrorCode ret = WmErrorCode::WM_OK;
    if (window->IsPcWindow()) {
        return WmErrorCode::WM_ERROR_DEVICE_NOT_SUPPORT;
    }
    if (isRegister) {
        ret = MappingWmErrorCodeSafely(window->RegisterWindowRotationChangeListener(thisListener));
    } else {
        ret = MappingWmErrorCodeSafely(window->UnregisterWindowRotationChangeListener(thisListener));
    }
    return ret;
}

WmErrorCode JsWindowRegisterManager::ProcessFreeWindowModeChangeRegister(const sptr<JsWindowListener>& listener,
    const sptr<Window>& window, bool isRegister, napi_env env, napi_value parameter)
{
    if (window == nullptr || listener == nullptr) {
        return WmErrorCode::WM_ERROR_STATE_ABNORMALLY;
    }
    sptr<IFreeWindowModeChangeListener> thisListener(listener);
    WmErrorCode ret = WmErrorCode::WM_OK;
    if (isRegister) {
        ret = MappingWmErrorCodeSafely(window->RegisterFreeWindowModeChangeListener(thisListener));
    } else {
        ret = MappingWmErrorCodeSafely(window->UnregisterFreeWindowModeChangeListener(thisListener));
    }
    return ret;
}
} // namespace Rosen
} // namespace OHOS