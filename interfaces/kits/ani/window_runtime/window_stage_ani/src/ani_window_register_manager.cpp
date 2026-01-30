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

#include "ani_window_register_manager.h"

#include "singleton_container.h"
#include "window_manager.h"
#include "window_manager_hilog.h"

namespace OHOS {
namespace Rosen {
using namespace AbilityRuntime;
namespace {
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
    {KEYBOARD_DID_SHOW_CB, RegisterListenerType::KEYBOARD_DID_SHOW_CB},
    {KEYBOARD_DID_HIDE_CB, RegisterListenerType::KEYBOARD_DID_HIDE_CB},
    {KEYBOARD_WILL_SHOW_CB, RegisterListenerType::KEYBOARD_WILL_SHOW_CB},
    {KEYBOARD_WILL_HIDE_CB, RegisterListenerType::KEYBOARD_WILL_HIDE_CB},
    {TOUCH_OUTSIDE_CB, RegisterListenerType::TOUCH_OUTSIDE_CB},
    {SCREENSHOT_EVENT_CB, RegisterListenerType::SCREENSHOT_EVENT_CB},
    {DIALOG_TARGET_TOUCH_CB, RegisterListenerType::DIALOG_TARGET_TOUCH_CB},
    {DIALOG_DEATH_RECIPIENT_CB, RegisterListenerType::DIALOG_DEATH_RECIPIENT_CB},
    {WINDOW_STATUS_CHANGE_CB, RegisterListenerType::WINDOW_STATUS_CHANGE_CB},
    {WINDOW_TITLE_BUTTON_RECT_CHANGE_CB, RegisterListenerType::WINDOW_TITLE_BUTTON_RECT_CHANGE_CB},
    {WINDOW_VISIBILITY_CHANGE_CB, RegisterListenerType::WINDOW_VISIBILITY_CHANGE_CB},
    {OCCLUSION_STATE_CHANGE_CB, RegisterListenerType::OCCLUSION_STATE_CHANGE_CB},
    {FRAME_METRICS_MEASURED_CHANGE_CB, RegisterListenerType::FRAME_METRICS_MEASURED_CHANGE_CB},
    {WINDOW_NO_INTERACTION_DETECT_CB, RegisterListenerType::WINDOW_NO_INTERACTION_DETECT_CB},
    {WINDOW_RECT_CHANGE_CB, RegisterListenerType::WINDOW_RECT_CHANGE_CB},
    {SUB_WINDOW_CLOSE_CB, RegisterListenerType::SUB_WINDOW_CLOSE_CB},
    {WINDOW_HIGHLIGHT_CHANGE_CB, RegisterListenerType::WINDOW_HIGHLIGHT_CHANGE_CB},
    {WINDOW_DISPLAYID_CHANGE_CB, RegisterListenerType::WINDOW_DISPLAYID_CHANGE_CB},
    {SYSTEM_DENSITY_CHANGE_CB, RegisterListenerType::SYSTEM_DENSITY_CHANGE_CB},
    {WINDOW_ROTATION_CHANGE_CB, RegisterListenerType::WINDOW_ROTATION_CHANGE_CB},
    {RECT_CHANGE_IN_GLOBAL_DISPLAY_CB, RegisterListenerType::RECT_CHANGE_IN_GLOBAL_DISPLAY_CB},
    {EXTENSION_SECURE_LIMIT_CHANGE_CB, RegisterListenerType::EXTENSION_SECURE_LIMIT_CHANGE_CB},
    {WINDOW_STATUS_DID_CHANGE_CB, RegisterListenerType::WINDOW_STATUS_DID_CHANGE_CB},
    {ACROSS_DISPLAYS_CHANGE_CB, RegisterListenerType::ACROSS_DISPLAYS_CHANGE_CB},
    {SCREENSHOT_APP_EVENT_CB, RegisterListenerType::SCREENSHOT_APP_EVENT_CB},
    {FREE_WINDOW_MODE_CHANGE_CB, RegisterListenerType::FREE_WINDOW_MODE_CHANGE_CB},
    {WINDOW_WILL_CLOSE_CB, RegisterListenerType::WINDOW_WILL_CLOSE_CB},
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

AniWindowRegisterManager::AniWindowRegisterManager()
{
}

AniWindowRegisterManager::~AniWindowRegisterManager()
{
}

WmErrorCode AniWindowRegisterManager::ProcessWindowChangeRegister(sptr<AniWindowListener> listener,
    sptr<Window> window, bool isRegister, ani_env* env)
{
    if (window == nullptr) {
        TLOGE(WmsLogTag::DEFAULT, "[ANI]Window is nullptr");
        return WmErrorCode::WM_ERROR_STATE_ABNORMALLY;
    }
    sptr<IWindowChangeListener> thisListener(listener);
    WmErrorCode ret = WmErrorCode::WM_OK;
    if (isRegister) {
        ret = WM_JS_TO_ERROR_CODE_MAP.at(window->RegisterWindowChangeListener(thisListener));
    } else {
        ret = WM_JS_TO_ERROR_CODE_MAP.at(window->UnregisterWindowChangeListener(thisListener));
    }
    return ret;
}

WmErrorCode AniWindowRegisterManager::ProcessSystemAvoidAreaChangeRegister(sptr<AniWindowListener> listener,
    sptr<Window> window, bool isRegister, ani_env* env)
{
    if (window == nullptr) {
        TLOGE(WmsLogTag::WMS_IMMS, "[ANI]Window is nullptr");
        return WmErrorCode::WM_ERROR_STATE_ABNORMALLY;
    }
    if (listener == nullptr) {
        TLOGE(WmsLogTag::WMS_IMMS, "[ANI]listener is nullptr");
        return WmErrorCode::WM_ERROR_STATE_ABNORMALLY;
    }
    listener->SetIsDeprecatedInterface(true);
    sptr<IAvoidAreaChangedListener> thisListener(listener);
    WmErrorCode ret = WmErrorCode::WM_OK;
    if (isRegister) {
        ret = WM_JS_TO_ERROR_CODE_MAP.at(window->RegisterAvoidAreaChangeListener(thisListener));
    } else {
        ret = WM_JS_TO_ERROR_CODE_MAP.at(window->UnregisterAvoidAreaChangeListener(thisListener));
    }
    return ret;
}

WmErrorCode AniWindowRegisterManager::ProcessAvoidAreaChangeRegister(sptr<AniWindowListener> listener,
    sptr<Window> window, bool isRegister, ani_env* env)
{
    if (window == nullptr) {
        TLOGE(WmsLogTag::DEFAULT, "[ANI]Window is nullptr");
        return WmErrorCode::WM_ERROR_STATE_ABNORMALLY;
    }
    sptr<IAvoidAreaChangedListener> thisListener(listener);
    WmErrorCode ret = WmErrorCode::WM_OK;
    if (isRegister) {
        ret = WM_JS_TO_ERROR_CODE_MAP.at(window->RegisterAvoidAreaChangeListener(thisListener));
    } else {
        ret = WM_JS_TO_ERROR_CODE_MAP.at(window->UnregisterAvoidAreaChangeListener(thisListener));
    }
    return ret;
}

WmErrorCode AniWindowRegisterManager::ProcessSystemDensityChangeRegister(sptr<AniWindowListener> listener,
    sptr<Window> window, bool isRegister, ani_env* env)
{
    if (window == nullptr) {
        TLOGE(WmsLogTag::DEFAULT, "[ANI]Window is nullptr");
        return WmErrorCode::WM_ERROR_STATE_ABNORMALLY;
    }
    sptr<ISystemDensityChangeListener> thisListener(listener);
    WmErrorCode ret = WmErrorCode::WM_OK;
    if (isRegister) {
        ret = WM_JS_TO_ERROR_CODE_MAP.at(window->RegisterSystemDensityChangeListener(thisListener));
    } else {
        ret = WM_JS_TO_ERROR_CODE_MAP.at(window->UnregisterSystemDensityChangeListener(thisListener));
    }
    return ret;
}

WmErrorCode AniWindowRegisterManager::ProcessDisplayIdChangeRegister(sptr<AniWindowListener> listener,
    sptr<Window> window, bool isRegister, ani_env* env)
{
    if (window == nullptr) {
        TLOGE(WmsLogTag::DEFAULT, "[ANI]Window is nullptr");
        return WmErrorCode::WM_ERROR_STATE_ABNORMALLY;
    }
    sptr<IDisplayIdChangeListener> thisListener(listener);
    WmErrorCode ret = WmErrorCode::WM_OK;
    if (isRegister) {
        ret = WM_JS_TO_ERROR_CODE_MAP.at(window->RegisterDisplayIdChangeListener(thisListener));
    } else {
        ret = WM_JS_TO_ERROR_CODE_MAP.at(window->UnregisterDisplayIdChangeListener(thisListener));
    }
    return ret;
}

WmErrorCode AniWindowRegisterManager::ProcessLifeCycleEventRegister(sptr<AniWindowListener> listener,
    sptr<Window> window, bool isRegister, ani_env* env)
{
    if (window == nullptr) {
        TLOGE(WmsLogTag::DEFAULT, "[ANI]Window is nullptr");
        return WmErrorCode::WM_ERROR_STATE_ABNORMALLY;
    }
    sptr<IWindowLifeCycle> thisListener(listener);
    WmErrorCode ret = WmErrorCode::WM_OK;
    if (isRegister) {
        ret = WM_JS_TO_ERROR_CODE_MAP.at(window->RegisterLifeCycleListener(thisListener));
    } else {
        ret = WM_JS_TO_ERROR_CODE_MAP.at(window->UnregisterLifeCycleListener(thisListener));
    }
    return ret;
}

WmErrorCode AniWindowRegisterManager::ProcessWindowStageLifeCycleEventRegister(sptr<AniWindowListener> listener,
    sptr<Window> window, bool isRegister, ani_env* env)
{
    if (window == nullptr) {
        TLOGE(WmsLogTag::DEFAULT, "[ANI]Window is nullptr");
        return WmErrorCode::WM_ERROR_STATE_ABNORMALLY;
    }
    sptr<IWindowStageLifeCycle> thisListener(listener);
    WmErrorCode ret = WmErrorCode::WM_OK;
    if (isRegister) {
        ret = WM_JS_TO_ERROR_CODE_MAP.at(window->RegisterWindowStageLifeCycleListener(thisListener));
    } else {
        ret = WM_JS_TO_ERROR_CODE_MAP.at(window->UnregisterWindowStageLifeCycleListener(thisListener));
    }
    return ret;
}

WmErrorCode AniWindowRegisterManager::ProcessOccupiedAreaChangeRegister(sptr<AniWindowListener> listener,
    sptr<Window> window, bool isRegister, ani_env* env)
{
    if (window == nullptr) {
        TLOGE(WmsLogTag::WMS_KEYBOARD, "[ANI]Window is nullptr");
        return WmErrorCode::WM_ERROR_STATE_ABNORMALLY;
    }
    sptr<IOccupiedAreaChangeListener> thisListener(listener);
    WmErrorCode ret = WmErrorCode::WM_OK;
    if (isRegister) {
        ret = WM_JS_TO_ERROR_CODE_MAP.at(window->RegisterOccupiedAreaChangeListener(thisListener));
    } else {
        ret = WM_JS_TO_ERROR_CODE_MAP.at(window->UnregisterOccupiedAreaChangeListener(thisListener));
    }
    return ret;
}

WmErrorCode AniWindowRegisterManager::ProcessKeyboardDidShowRegister(sptr<AniWindowListener> listener,
    sptr<Window> window, bool isRegister, ani_env* env)
{
    if (window == nullptr) {
        TLOGE(WmsLogTag::WMS_KEYBOARD, "[ANI]Window is nullptr");
        return WmErrorCode::WM_ERROR_STATE_ABNORMALLY;
    }
    sptr<IKeyboardDidShowListener> thisListener(listener);
    WmErrorCode ret = WmErrorCode::WM_OK;
    if (isRegister) {
        ret = WM_JS_TO_ERROR_CODE_MAP.at(window->RegisterKeyboardDidShowListener(thisListener));
    } else {
        ret = WM_JS_TO_ERROR_CODE_MAP.at(window->UnregisterKeyboardDidShowListener(thisListener));
    }
    return ret;
}

WmErrorCode AniWindowRegisterManager::ProcessKeyboardDidHideRegister(sptr<AniWindowListener> listener,
    sptr<Window> window, bool isRegister, ani_env* env)
{
    if (window == nullptr) {
        TLOGE(WmsLogTag::WMS_KEYBOARD, "[ANI]Window is nullptr");
        return WmErrorCode::WM_ERROR_STATE_ABNORMALLY;
    }
    sptr<IKeyboardDidHideListener> thisListener(listener);
    WmErrorCode ret = WmErrorCode::WM_OK;
    if (isRegister) {
        ret = WM_JS_TO_ERROR_CODE_MAP.at(window->RegisterKeyboardDidHideListener(thisListener));
    } else {
        ret = WM_JS_TO_ERROR_CODE_MAP.at(window->UnregisterKeyboardDidHideListener(thisListener));
    }
    return ret;
}

WmErrorCode AniWindowRegisterManager::ProcessKeyboardWillShowRegister(sptr<AniWindowListener> listener,
    const sptr<Window>& window, bool isRegister, ani_env* env)
{
    if (window == nullptr) {
        TLOGE(WmsLogTag::WMS_KEYBOARD, "[ANI]Window is nullptr");
        return WmErrorCode::WM_ERROR_STATE_ABNORMALLY;
    }
    sptr<IKeyboardWillShowListener> thisListener(listener);
    WmErrorCode ret = WmErrorCode::WM_OK;
    if (isRegister) {
        ret = WM_JS_TO_ERROR_CODE_MAP.at(window->RegisterKeyboardWillShowListener(thisListener));
    } else {
        ret = WM_JS_TO_ERROR_CODE_MAP.at(window->UnregisterKeyboardWillShowListener(thisListener));
    }
    return ret;
}

WmErrorCode AniWindowRegisterManager::ProcessKeyboardWillHideRegister(sptr<AniWindowListener> listener,
    const sptr<Window>& window, bool isRegister, ani_env* env)
{
    if (window == nullptr) {
        TLOGE(WmsLogTag::WMS_KEYBOARD, "[ANI]Window is nullptr");
        return WmErrorCode::WM_ERROR_STATE_ABNORMALLY;
    }
    sptr<IKeyboardWillHideListener> thisListener(listener);
    WmErrorCode ret = WmErrorCode::WM_OK;
    if (isRegister) {
        ret = WM_JS_TO_ERROR_CODE_MAP.at(window->RegisterKeyboardWillHideListener(thisListener));
    } else {
        ret = WM_JS_TO_ERROR_CODE_MAP.at(window->UnregisterKeyboardWillHideListener(thisListener));
    }
    return ret;
}

WmErrorCode AniWindowRegisterManager::ProcessSystemBarChangeRegister(sptr<AniWindowListener> listener,
    sptr<Window> window, bool isRegister, ani_env* env)
{
    sptr<ISystemBarChangedListener> thisListener(listener);
    WmErrorCode ret = WmErrorCode::WM_OK;
    if (isRegister) {
        ret = WM_JS_TO_ERROR_CODE_MAP.at(
            SingletonContainer::Get<WindowManager>().RegisterSystemBarChangedListener(thisListener));
    } else {
        ret = WM_JS_TO_ERROR_CODE_MAP.at(
            SingletonContainer::Get<WindowManager>().UnregisterSystemBarChangedListener(thisListener));
    }
    return ret;
}

WmErrorCode AniWindowRegisterManager::ProcessGestureNavigationEnabledChangeRegister(sptr<AniWindowListener> listener,
    sptr<Window> window, bool isRegister, ani_env* env)
{
    sptr<IGestureNavigationEnabledChangedListener> thisListener(listener);
    WmErrorCode ret;
    if (isRegister) {
        ret = WM_JS_TO_ERROR_CODE_MAP.at(
            SingletonContainer::Get<WindowManager>().RegisterGestureNavigationEnabledChangedListener(thisListener));
    } else {
        ret = WM_JS_TO_ERROR_CODE_MAP.at(
            SingletonContainer::Get<WindowManager>().UnregisterGestureNavigationEnabledChangedListener(thisListener));
    }
    return ret;
}

WmErrorCode AniWindowRegisterManager::ProcessWaterMarkFlagChangeRegister(sptr<AniWindowListener> listener,
    sptr<Window> window, bool isRegister, ani_env* env)
{
    sptr<IWaterMarkFlagChangedListener> thisListener(listener);
    WmErrorCode ret;
    if (isRegister) {
        ret = WM_JS_TO_ERROR_CODE_MAP.at(
            SingletonContainer::Get<WindowManager>().RegisterWaterMarkFlagChangedListener(thisListener));
    } else {
        ret = WM_JS_TO_ERROR_CODE_MAP.at(
            SingletonContainer::Get<WindowManager>().UnregisterWaterMarkFlagChangedListener(thisListener));
    }
    return ret;
}
WmErrorCode AniWindowRegisterManager::ProcessTouchOutsideRegister(sptr<AniWindowListener> listener,
    sptr<Window> window, bool isRegister, ani_env* env)
{
    TLOGI(WmsLogTag::WMS_EVENT, "called");
    if (window == nullptr) {
        return WmErrorCode::WM_ERROR_STATE_ABNORMALLY;
    }
    sptr<ITouchOutsideListener> thisListener(listener);
    WmErrorCode ret = WmErrorCode::WM_OK;
    if (isRegister) {
        ret = AniWindowUtils::ToErrorCode(window->RegisterTouchOutsideListener(thisListener));
    } else {
        ret = AniWindowUtils::ToErrorCode(window->UnregisterTouchOutsideListener(thisListener));
    }
    return ret;
}

WmErrorCode AniWindowRegisterManager::ProcessDialogTargetTouchRegister(sptr<AniWindowListener> listener,
    sptr<Window> window, bool isRegister, ani_env* env)
{
    if (window == nullptr) {
        return WmErrorCode::WM_ERROR_STATE_ABNORMALLY;
    }
    sptr<IDialogTargetTouchListener> thisListener(listener);
    WmErrorCode ret = WmErrorCode::WM_OK;
    if (isRegister) {
        ret = AniWindowUtils::ToErrorCode(window->RegisterDialogTargetTouchListener(thisListener));
    } else {
        ret = AniWindowUtils::ToErrorCode(window->UnregisterDialogTargetTouchListener(thisListener));
    }
    return ret;
}

WmErrorCode AniWindowRegisterManager::ProcessWindowNoInteractionRegister(sptr<AniWindowListener> listener,
    sptr<Window> window, bool isRegister, ani_env* env, ani_long timeout)
{
    if (window == nullptr) {
        return WmErrorCode::WM_ERROR_STATE_ABNORMALLY;
    }
    sptr<IWindowNoInteractionListener> thisListener(listener);
    if (!isRegister) {
        return AniWindowUtils::ToErrorCode(window->UnregisterWindowNoInteractionListener(thisListener));
    }
    constexpr ani_long secToMicrosecRatio = 1000;
    constexpr ani_long noInteractionMax = LLONG_MAX / secToMicrosecRatio;
    if (timeout <= 0 || (timeout > noInteractionMax)) {
        TLOGE(WmsLogTag::WMS_EVENT, "[ANI]invalid parameter: no-interaction-timeout %{public}" PRId64 " is not in "
            "(0s~%{public}" PRId64, timeout, noInteractionMax);
        return WmErrorCode::WM_ERROR_INVALID_PARAM;
    }
    thisListener->SetTimeout(timeout * secToMicrosecRatio);
    return AniWindowUtils::ToErrorCode(window->RegisterWindowNoInteractionListener(thisListener));
}

WmErrorCode AniWindowRegisterManager::ProcessWindowVisibilityChangeRegister(sptr<AniWindowListener> listener,
    sptr<Window> window, bool isRegister, ani_env* env)
{
    TLOGD(WmsLogTag::DEFAULT, "called");
    if (window == nullptr || listener == nullptr) {
        return WmErrorCode::WM_ERROR_STATE_ABNORMALLY;
    }
    sptr<IWindowVisibilityChangedListener> thisListener(listener);
    WmErrorCode ret = WmErrorCode::WM_OK;
    if (isRegister) {
        ret = WM_JS_TO_ERROR_CODE_MAP.at(window->RegisterWindowVisibilityChangeListener(thisListener));
    } else {
        ret = WM_JS_TO_ERROR_CODE_MAP.at(window->UnregisterWindowVisibilityChangeListener(thisListener));
    }
    return ret;
}

WmErrorCode AniWindowRegisterManager::ProcessOcclusionStateChangeRegister(const sptr<AniWindowListener>& listener,
    sptr<Window> window, bool isRegister, ani_env* env)
{
    if (window == nullptr || listener == nullptr) {
        TLOGE(WmsLogTag::WMS_ATTRIBUTE, "[ANI] window or listener is null");
        return WmErrorCode::WM_ERROR_STATE_ABNORMALLY;
    }
    WMError retCode = WMError::WM_OK;
    sptr<IOcclusionStateChangedListener> thisListener(listener);
    if (isRegister) {
        retCode = window->RegisterOcclusionStateChangeListener(thisListener);
    } else {
        retCode = window->UnregisterOcclusionStateChangeListener(thisListener);
    }
    TLOGI(WmsLogTag::WMS_ATTRIBUTE, "[ANI] retCode=%{public}d", static_cast<int32_t>(retCode));
    auto retErrCode = WmErrorCode::WM_ERROR_SYSTEM_ABNORMALLY;
    if (WM_JS_TO_ERROR_CODE_MAP.count(retCode) > 0) {
        retErrCode = WM_JS_TO_ERROR_CODE_MAP.at(retCode);
    }
    return retErrCode;
}

WmErrorCode AniWindowRegisterManager::ProcessFrameMetricsMeasuredChangeRegister(const sptr<AniWindowListener>& listener,
    sptr<Window> window, bool isRegister, ani_env* env)
{
    if (window == nullptr || listener == nullptr) {
        TLOGE(WmsLogTag::WMS_ATTRIBUTE, "[ANI] window or listener is null");
        return WmErrorCode::WM_ERROR_STATE_ABNORMALLY;
    }
    WMError retCode = WMError::WM_OK;
    sptr<IFrameMetricsChangedListener> thisListener(listener);
    if (isRegister) {
        retCode = window->RegisterFrameMetricsChangeListener(thisListener);
    } else {
        retCode = window->UnregisterFrameMetricsChangeListener(thisListener);
    }
    TLOGI(WmsLogTag::WMS_ATTRIBUTE, "[ANI] retCode=%{public}d", static_cast<int32_t>(retCode));
    auto retErrCode = WmErrorCode::WM_ERROR_STATE_ABNORMALLY;
    if (WM_JS_TO_ERROR_CODE_MAP.count(retCode) > 0) {
        retErrCode = WM_JS_TO_ERROR_CODE_MAP.at(retCode);
    }
    return retErrCode;
}

WmErrorCode AniWindowRegisterManager::ProcessScreenshotRegister(sptr<AniWindowListener> listener,
    sptr<Window> window, bool isRegister, ani_env* env)
{
    TLOGI(WmsLogTag::DEFAULT, "called");
    if (window == nullptr) {
        TLOGE(WmsLogTag::DEFAULT,
            "%{public}sregister screenshot listener failed. window is null", isRegister? "" : "un");
        return WmErrorCode::WM_ERROR_STATE_ABNORMALLY;
    }
    sptr<IScreenshotListener> thisListener(listener);
    WmErrorCode ret = WmErrorCode::WM_OK;
    if (isRegister) {
        ret = WM_JS_TO_ERROR_CODE_MAP.at(window->RegisterScreenshotListener(thisListener));
    } else {
        ret = WM_JS_TO_ERROR_CODE_MAP.at(window->UnregisterScreenshotListener(thisListener));
    }
    return ret;
}

WmErrorCode AniWindowRegisterManager::ProcessDialogDeathRecipientRegister(sptr<AniWindowListener> listener,
    sptr<Window> window, bool isRegister, ani_env* env)
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

WmErrorCode AniWindowRegisterManager::ProcessWindowTitleButtonRectChangeRegister(sptr<AniWindowListener> listener,
    sptr<Window> window, bool isRegister, ani_env* env)
{
    TLOGD(WmsLogTag::DEFAULT, "called");
    if (window == nullptr) {
        TLOGE(WmsLogTag::DEFAULT, "[ANI]Window is nullptr");
        return WmErrorCode::WM_ERROR_STATE_ABNORMALLY;
    }
    sptr<IWindowTitleButtonRectChangedListener> thisListener(listener);
    WmErrorCode ret = WmErrorCode::WM_OK;
    if (isRegister) {
        ret = WM_JS_TO_ERROR_CODE_MAP.at(window->RegisterWindowTitleButtonRectChangeListener(thisListener));
    } else {
        ret = WM_JS_TO_ERROR_CODE_MAP.at(window->UnregisterWindowTitleButtonRectChangeListener(thisListener));
    }
    return ret;
}

WmErrorCode AniWindowRegisterManager::ProcessWindowWillCloseRegister(const sptr<AniWindowListener>& listener,
    const sptr<Window>& window, bool isRegister, ani_env* env)
{
    TLOGD(WmsLogTag::DEFAULT, "called");
    if (window == nullptr) {
        TLOGE(WmsLogTag::DEFAULT, "[ANI]Window is nullptr");
        return WmErrorCode::WM_ERROR_STATE_ABNORMALLY;
    }
    sptr<IWindowWillCloseListener> thisListener(listener);
    WmErrorCode ret = WmErrorCode::WM_OK;
    if (isRegister) {
        ret = WM_JS_TO_ERROR_CODE_MAP.at(window->RegisterWindowWillCloseListeners(thisListener));
    } else {
        ret = WM_JS_TO_ERROR_CODE_MAP.at(window->UnRegisterWindowWillCloseListeners(thisListener));
    }
    return ret;
}

bool AniWindowRegisterManager::IsCallbackRegistered(ani_env* env, std::string type, ani_ref jsListenerObject)
{
    if (jsCbMap_.empty() || jsCbMap_.find(type) == jsCbMap_.end()) {
        TLOGI(WmsLogTag::DEFAULT, "[ANI]Method %{public}s has not been registerted", type.c_str());
        return false;
    }

    for (auto iter = jsCbMap_[type].begin(); iter != jsCbMap_[type].end(); ++iter) {
        ani_boolean isEquals = 0;
        env->Reference_StrictEquals(jsListenerObject, iter->first, &isEquals);
        if (isEquals) {
            TLOGE(WmsLogTag::DEFAULT, "[ANI]Method %{public}s has already been registered", type.c_str());
            return true;
        }
    }
    return false;
}

WmErrorCode AniWindowRegisterManager::RegisterListener(sptr<Window> window, const std::string& type,
    CaseType caseType, ani_env* env, ani_ref callback, ani_long timeout)
{
    std::lock_guard<std::mutex> lock(mtx_);
    if (IsCallbackRegistered(env, type, callback)) {
        return WmErrorCode::WM_OK;
    }
    auto iterCaseType = LISTENER_CODE_MAP.find(caseType);
    if (iterCaseType == LISTENER_CODE_MAP.end()) {
        TLOGE(WmsLogTag::DEFAULT, "[ANI]CaseType %{public}u is not supported", static_cast<uint32_t>(caseType));
        return WmErrorCode::WM_ERROR_STATE_ABNORMALLY;
    }
    auto iterCallbackType = iterCaseType->second.find(type);
    if (iterCallbackType == iterCaseType->second.end()) {
        TLOGE(WmsLogTag::DEFAULT, "[ANI]Type %{public}s is not supported", type.c_str());
        return WmErrorCode::WM_ERROR_STATE_ABNORMALLY;
    }
    RegisterListenerType listenerType = iterCallbackType->second;
    ani_ref cbRef{};
    if (env->GlobalReference_Create(callback, &cbRef) != ANI_OK) {
        TLOGE(WmsLogTag::DEFAULT, "[ANI]create global ref fail");
        return WmErrorCode::WM_ERROR_STATE_ABNORMALLY;
    };
    ani_vm* vm = nullptr;
    ani_status aniRet = env->GetVM(&vm);
    if (aniRet != ANI_OK || vm == nullptr) {
        TLOGE(WmsLogTag::DEFAULT, "[ANI]Get VM failed, ret: %{public}u", aniRet);
        return WmErrorCode::WM_ERROR_STATE_ABNORMALLY;
    }
    auto windowManagerListener = sptr<AniWindowListener>::MakeSptr(env, vm, cbRef, caseType);
    if (windowManagerListener == nullptr) {
        TLOGE(WmsLogTag::DEFAULT, "[ANI]New AniWindowListener failed");
        return WmErrorCode::WM_ERROR_STATE_ABNORMALLY;
    }
    windowManagerListener->SetMainEventHandler();
    WmErrorCode ret = ProcessListener(listenerType, caseType, windowManagerListener, window, true, env, timeout);
    if (ret != WmErrorCode::WM_OK) {
        TLOGE(WmsLogTag::DEFAULT, "[ANI]Register type %{public}s failed", type.c_str());
        return ret;
    }
    jsCbMap_[type][cbRef] = windowManagerListener;
    TLOGI(WmsLogTag::DEFAULT, "[ANI]Register type %{public}s success! callback map size: %{public}zu",
        type.c_str(), jsCbMap_[type].size());
    return WmErrorCode::WM_OK;
}

WmErrorCode AniWindowRegisterManager::ProcessWindowStageListener(RegisterListenerType registerListenerType,
    const sptr<AniWindowListener>& windowManagerListener, const sptr<Window>& window, bool isRegister, ani_env* env)
{
    switch (registerListenerType) {
        case RegisterListenerType::WINDOW_STAGE_EVENT_CB:
            return ProcessLifeCycleEventRegister(windowManagerListener, window, isRegister, env);
        case RegisterListenerType::WINDOW_STAGE_CLOSE_CB:
            return ProcessMainWindowCloseRegister(windowManagerListener, window, isRegister, env);
        case RegisterListenerType::WINDOW_STAGE_LIFECYCLE_EVENT_CB:
            return ProcessWindowStageLifeCycleEventRegister(windowManagerListener, window, isRegister, env);
        default:
            TLOGE(WmsLogTag::DEFAULT, "[ANI]RegisterListenerType %{public}u is not supported",
                static_cast<uint32_t>(registerListenerType));
            return WmErrorCode::WM_ERROR_INVALID_PARAM;
    }
}

WmErrorCode AniWindowRegisterManager::ProcessWindowListener(RegisterListenerType registerListenerType,
    const sptr<AniWindowListener>& windowManagerListener, const sptr<Window>& window, bool isRegister, ani_env* env,
    ani_long timeout)
{
    switch (static_cast<uint32_t>(registerListenerType)) {
        case static_cast<uint32_t>(RegisterListenerType::WINDOW_SIZE_CHANGE_CB):
            return ProcessWindowChangeRegister(windowManagerListener, window, isRegister, env);
        case static_cast<uint32_t>(RegisterListenerType::SYSTEM_AVOID_AREA_CHANGE_CB):
            return ProcessSystemAvoidAreaChangeRegister(windowManagerListener, window, isRegister, env);
        case static_cast<uint32_t>(RegisterListenerType::AVOID_AREA_CHANGE_CB):
            return ProcessAvoidAreaChangeRegister(windowManagerListener, window, isRegister, env);
        case static_cast<uint32_t>(RegisterListenerType::LIFECYCLE_EVENT_CB):
            return ProcessLifeCycleEventRegister(windowManagerListener, window, isRegister, env);
        case static_cast<uint32_t>(RegisterListenerType::WINDOW_EVENT_CB):
            return ProcessLifeCycleEventRegister(windowManagerListener, window, isRegister, env);
        case static_cast<uint32_t>(RegisterListenerType::KEYBOARD_HEIGHT_CHANGE_CB):
            return ProcessOccupiedAreaChangeRegister(windowManagerListener, window, isRegister, env);
        case static_cast<uint32_t>(RegisterListenerType::KEYBOARD_DID_SHOW_CB):
            return ProcessKeyboardDidShowRegister(windowManagerListener, window, isRegister, env);
        case static_cast<uint32_t>(RegisterListenerType::KEYBOARD_DID_HIDE_CB):
            return ProcessKeyboardDidHideRegister(windowManagerListener, window, isRegister, env);
        case static_cast<uint32_t>(RegisterListenerType::KEYBOARD_WILL_SHOW_CB):
            return ProcessKeyboardWillShowRegister(windowManagerListener, window, isRegister, env);
        case static_cast<uint32_t>(RegisterListenerType::KEYBOARD_WILL_HIDE_CB):
            return ProcessKeyboardWillHideRegister(windowManagerListener, window, isRegister, env);
        case static_cast<uint32_t>(RegisterListenerType::TOUCH_OUTSIDE_CB):
            return ProcessTouchOutsideRegister(windowManagerListener, window, isRegister, env);
        case static_cast<uint32_t>(RegisterListenerType::DIALOG_TARGET_TOUCH_CB):
            return ProcessDialogTargetTouchRegister(windowManagerListener, window, isRegister, env);
        case static_cast<uint32_t>(RegisterListenerType::WINDOW_NO_INTERACTION_DETECT_CB):
            return ProcessWindowNoInteractionRegister(windowManagerListener, window, isRegister, env, timeout);
        case static_cast<uint32_t>(RegisterListenerType::SCREENSHOT_EVENT_CB):
            return ProcessScreenshotRegister(windowManagerListener, window, isRegister, env);
        case static_cast<uint32_t>(RegisterListenerType::DIALOG_DEATH_RECIPIENT_CB):
            return ProcessDialogDeathRecipientRegister(windowManagerListener, window, isRegister, env);
        case static_cast<uint32_t>(RegisterListenerType::WINDOW_STATUS_CHANGE_CB):
            return ProcessWindowStatusChangeRegister(windowManagerListener, window, isRegister, env);
        case static_cast<uint32_t>(RegisterListenerType::WINDOW_TITLE_BUTTON_RECT_CHANGE_CB):
            return ProcessWindowTitleButtonRectChangeRegister(windowManagerListener, window, isRegister, env);
        case static_cast<uint32_t>(RegisterListenerType::WINDOW_VISIBILITY_CHANGE_CB):
            return ProcessWindowVisibilityChangeRegister(windowManagerListener, window, isRegister, env);
        case static_cast<uint32_t>(RegisterListenerType::OCCLUSION_STATE_CHANGE_CB):
            return ProcessOcclusionStateChangeRegister(windowManagerListener, window, isRegister, env);
        case static_cast<uint32_t>(RegisterListenerType::FRAME_METRICS_MEASURED_CHANGE_CB):
            return ProcessFrameMetricsMeasuredChangeRegister(windowManagerListener, window, isRegister, env);
        case static_cast<uint32_t>(RegisterListenerType::WINDOW_RECT_CHANGE_CB):
            return ProcessWindowRectChangeRegister(windowManagerListener, window, isRegister, env);
        case static_cast<uint32_t>(RegisterListenerType::SUB_WINDOW_CLOSE_CB):
            return ProcessSubWindowCloseRegister(windowManagerListener, window, isRegister, env);
        case static_cast<uint32_t>(RegisterListenerType::WINDOW_HIGHLIGHT_CHANGE_CB):
            return ProcessWindowHighlightChangeRegister(windowManagerListener, window, isRegister, env);
        case static_cast<uint32_t>(RegisterListenerType::SYSTEM_DENSITY_CHANGE_CB):
            return ProcessSystemDensityChangeRegister(windowManagerListener, window, isRegister, env);
        case static_cast<uint32_t>(RegisterListenerType::WINDOW_DISPLAYID_CHANGE_CB):
            return ProcessDisplayIdChangeRegister(windowManagerListener, window, isRegister, env);
        case static_cast<uint32_t>(RegisterListenerType::WINDOW_ROTATION_CHANGE_CB):
            return ProcessWindowRotationChangeRegister(windowManagerListener, window, isRegister, env);
        case static_cast<uint32_t>(RegisterListenerType::WINDOW_WILL_CLOSE_CB):
            return ProcessWindowWillCloseRegister(windowManagerListener, window, isRegister, env);
        case static_cast<uint32_t>(RegisterListenerType::RECT_CHANGE_IN_GLOBAL_DISPLAY_CB):
            return ProcessRectChangeInGlobalDisplayRegister(windowManagerListener, window, isRegister, env);
        case static_cast<uint32_t>(RegisterListenerType::EXTENSION_SECURE_LIMIT_CHANGE_CB):
            return ProcessExtensionSecureLimitChangeRegister(windowManagerListener, window, isRegister, env);
        case static_cast<uint32_t>(RegisterListenerType::WINDOW_STATUS_DID_CHANGE_CB):
            return ProcessWindowStatusDidChangeRegister(windowManagerListener, window, isRegister, env);
        case static_cast<uint32_t>(RegisterListenerType::ACROSS_DISPLAYS_CHANGE_CB):
            return ProcessAcrossDisplaysChangeRegister(windowManagerListener, window, isRegister, env);
        case static_cast<uint32_t>(RegisterListenerType::SCREENSHOT_APP_EVENT_CB):
            return ProcessScreenshotAppEventRegister(windowManagerListener, window, isRegister, env);
        case static_cast<uint32_t>(RegisterListenerType::FREE_WINDOW_MODE_CHANGE_CB):
            return ProcessFreeWindowModeChangeRegister(windowManagerListener, window, isRegister, env);
        default:
            TLOGE(WmsLogTag::DEFAULT, "[ANI]RegisterListenerType %{public}u is not supported",
                static_cast<uint32_t>(registerListenerType));
            return WmErrorCode::WM_ERROR_INVALID_PARAM;
    }
}

WmErrorCode AniWindowRegisterManager::ProcessWindowManagerListener(RegisterListenerType registerListenerType,
    const sptr<AniWindowListener>& windowManagerListener, const sptr<Window>& window, bool isRegister, ani_env* env)
{
    switch (static_cast<uint32_t>(registerListenerType)) {
        case static_cast<uint32_t>(RegisterListenerType::SYSTEM_BAR_TINT_CHANGE_CB):
            return ProcessSystemBarChangeRegister(windowManagerListener, window, isRegister, env);
        case static_cast<uint32_t>(RegisterListenerType::GESTURE_NAVIGATION_ENABLED_CHANGE_CB):
            return ProcessGestureNavigationEnabledChangeRegister(windowManagerListener, window, isRegister,
                env);
        case static_cast<uint32_t>(RegisterListenerType::WATER_MARK_FLAG_CHANGE_CB):
            return ProcessWaterMarkFlagChangeRegister(windowManagerListener, window, isRegister, env);
        default:
            TLOGE(WmsLogTag::DEFAULT, "[ANI]RegisterListenerType %{public}u is not supported",
                static_cast<uint32_t>(registerListenerType));
            return WmErrorCode::WM_ERROR_INVALID_PARAM;
    }
}

WmErrorCode AniWindowRegisterManager::ProcessListener(RegisterListenerType registerListenerType, CaseType caseType,
    const sptr<AniWindowListener>& windowManagerListener, const sptr<Window>& window, bool isRegister, ani_env* env,
    ani_long timeout)
{
    if (caseType == CaseType::CASE_WINDOW_MANAGER) {
        return ProcessWindowManagerListener(registerListenerType, windowManagerListener, window, isRegister, env);
    } else if (caseType == CaseType::CASE_WINDOW) {
        return ProcessWindowListener(registerListenerType, windowManagerListener, window, isRegister, env, timeout);
    } else if (caseType == CaseType::CASE_STAGE) {
        return ProcessWindowStageListener(registerListenerType, windowManagerListener, window, isRegister, env);
    }
    return WmErrorCode::WM_OK;
}

WmErrorCode AniWindowRegisterManager::UnregisterListener(sptr<Window> window, const std::string& type,
    CaseType caseType, ani_env* env, ani_ref callback)
{
    std::lock_guard<std::mutex> lock(mtx_);
    if (jsCbMap_.empty() || jsCbMap_.find(type) == jsCbMap_.end()) {
        TLOGW(WmsLogTag::DEFAULT, "[ANI]Type %{public}s was not registerted", type.c_str());
        return WmErrorCode::WM_OK;
    }
    auto iterCaseType = LISTENER_CODE_MAP.find(caseType);
    if (iterCaseType == LISTENER_CODE_MAP.end()) {
        TLOGE(WmsLogTag::DEFAULT, "[ANI]CaseType %{public}u is not supported", static_cast<uint32_t>(caseType));
        return WmErrorCode::WM_ERROR_STATE_ABNORMALLY;
    }
    auto iterCallbackType = iterCaseType->second.find(type);
    if (iterCallbackType == iterCaseType->second.end()) {
        TLOGE(WmsLogTag::DEFAULT, "[ANI]Type %{public}s is not supported", type.c_str());
        return WmErrorCode::WM_ERROR_STATE_ABNORMALLY;
    }
    RegisterListenerType listenerType = iterCallbackType->second;
    ani_boolean isUndef = ANI_FALSE;
    env->Reference_IsUndefined(callback, &isUndef);
    if (isUndef == ANI_TRUE) {
        TLOGI(WmsLogTag::DEFAULT, "[ANI]Unregister all callbck, type:%{public}s", type.c_str());
        for (auto it = jsCbMap_[type].begin(); it != jsCbMap_[type].end();) {
            WmErrorCode ret = ProcessListener(listenerType, caseType, it->second, window, false, env, 0);
            if (ret != WmErrorCode::WM_OK) {
                TLOGE(WmsLogTag::DEFAULT, "[ANI]Unregister type %{public}s failed, no value", type.c_str());
                return ret;
            }
            env->GlobalReference_Delete(it->second->GetAniCallback());
            it->second->SetAniCallback(nullptr);
            jsCbMap_[type].erase(it++);
        }
    } else {
        bool findFlag = false;
        for (auto it = jsCbMap_[type].begin(); it != jsCbMap_[type].end(); ++it) {
            ani_boolean isEquals = 0;
            env->Reference_StrictEquals(callback, it->first, &isEquals);
            TLOGI(WmsLogTag::DEFAULT, "[ANI]callback isEquals:%{public}d", static_cast<int32_t>(isEquals));
            if (!isEquals) {
                continue;
            }
            findFlag = true;
            WmErrorCode ret = ProcessListener(listenerType, caseType, it->second, window, false, env, 0);
            if (ret != WmErrorCode::WM_OK) {
                TLOGE(WmsLogTag::DEFAULT, "[ANI]Unregister type %{public}s failed", type.c_str());
                return ret;
            }
            env->GlobalReference_Delete(it->second->GetAniCallback());
            it->second->SetAniCallback(nullptr);
            jsCbMap_[type].erase(it);
            break;
        }
        if (!findFlag) {
            TLOGW(WmsLogTag::DEFAULT,
                "[ANI]Unregister type %{public}s failed because not found callback!", type.c_str());
            return WmErrorCode::WM_OK;
        }
    }
    TLOGI(WmsLogTag::DEFAULT, "[ANI]Unregister type %{public}s success! callback map size: %{public}zu",
        type.c_str(), jsCbMap_[type].size());
    // erase type when there is no callback in one type
    if (jsCbMap_[type].empty()) {
        jsCbMap_.erase(type);
    }
    return WmErrorCode::WM_OK;
}

WmErrorCode AniWindowRegisterManager::ProcessWindowStatusChangeRegister(sptr<AniWindowListener> listener,
    sptr<Window> window, bool isRegister, ani_env* env)
{
    if (window == nullptr) {
        TLOGE(WmsLogTag::DEFAULT, "[ANI]Window is nullptr");
        return WmErrorCode::WM_ERROR_STATE_ABNORMALLY;
    }
    sptr<IWindowStatusChangeListener> thisListener(listener);
    WmErrorCode ret = WmErrorCode::WM_OK;
    if (isRegister) {
        ret = WM_JS_TO_ERROR_CODE_MAP.at(window->RegisterWindowStatusChangeListener(thisListener));
    } else {
        ret = WM_JS_TO_ERROR_CODE_MAP.at(window->UnregisterWindowStatusChangeListener(thisListener));
    }
    return ret;
}

WmErrorCode AniWindowRegisterManager::ProcessWindowRectChangeRegister(sptr<AniWindowListener> listener,
    sptr<Window> window, bool isRegister, ani_env* env)
{
    if (window == nullptr) {
        return WmErrorCode::WM_ERROR_STATE_ABNORMALLY;
    }
    sptr<IWindowRectChangeListener> thisListener(listener);
    WmErrorCode ret = WmErrorCode::WM_OK;
    if (isRegister) {
        ret = WM_JS_TO_ERROR_CODE_MAP.at(window->RegisterWindowRectChangeListener(thisListener));
    } else {
        ret = WM_JS_TO_ERROR_CODE_MAP.at(window->UnregisterWindowRectChangeListener(thisListener));
    }
    return ret;
}

WmErrorCode AniWindowRegisterManager::ProcessSubWindowCloseRegister(sptr<AniWindowListener> listener,
    sptr<Window> window, bool isRegister, ani_env* env)
{
    if (window == nullptr) {
        return WmErrorCode::WM_ERROR_STATE_ABNORMALLY;
    }
    sptr<ISubWindowCloseListener> thisListener(listener);
    WmErrorCode ret = WmErrorCode::WM_OK;
    if (isRegister) {
        ret = WM_JS_TO_ERROR_CODE_MAP.at(window->RegisterSubWindowCloseListeners(thisListener));
    } else {
        ret = WM_JS_TO_ERROR_CODE_MAP.at(window->UnregisterSubWindowCloseListeners(thisListener));
    }
    return ret;
}

WmErrorCode AniWindowRegisterManager::ProcessWindowHighlightChangeRegister(sptr<AniWindowListener> listener,
    sptr<Window> window, bool isRegister, ani_env* env)
{
    if (window == nullptr) {
        return WmErrorCode::WM_ERROR_STATE_ABNORMALLY;
    }
    sptr<IWindowHighlightChangeListener> thisListener(listener);
    WmErrorCode ret = WmErrorCode::WM_OK;
    if (isRegister) {
        ret = WM_JS_TO_ERROR_CODE_MAP.at(window->RegisterWindowHighlightChangeListeners(thisListener));
    } else {
        ret = WM_JS_TO_ERROR_CODE_MAP.at(window->UnregisterWindowHighlightChangeListeners(thisListener));
    }
    return ret;
}

WmErrorCode AniWindowRegisterManager::ProcessMainWindowCloseRegister(const sptr<AniWindowListener>& listener,
    const sptr<Window>& window, bool isRegister, ani_env* env)
{
    if (window == nullptr) {
        return WmErrorCode::WM_ERROR_STATE_ABNORMALLY;
    }
    WmErrorCode ret = WmErrorCode::WM_OK;
    if (isRegister) {
        ret = WM_JS_TO_ERROR_CODE_MAP.at(window->RegisterMainWindowCloseListeners(listener));
    } else {
        ret = WM_JS_TO_ERROR_CODE_MAP.at(window->UnregisterMainWindowCloseListeners(listener));
    }
    return ret;
}

WmErrorCode AniWindowRegisterManager::ProcessWindowRotationChangeRegister(const sptr<AniWindowListener>& listener,
    const sptr<Window>& window, bool isRegister, ani_env* env)
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
        ret = WM_JS_TO_ERROR_CODE_MAP.at(window->RegisterWindowRotationChangeListener(thisListener));
    } else {
        ret = WM_JS_TO_ERROR_CODE_MAP.at(window->UnregisterWindowRotationChangeListener(thisListener));
    }
    return ret;
}

WmErrorCode AniWindowRegisterManager::ProcessRectChangeInGlobalDisplayRegister(const sptr<AniWindowListener>& listener,
    const sptr<Window>& window, bool isRegister, ani_env* env)
{
    if (window == nullptr || listener == nullptr) {
        return WmErrorCode::WM_ERROR_STATE_ABNORMALLY;
    }
    sptr<IRectChangeInGlobalDisplayListener> thisListener(listener);
    WMError ret = WMError::WM_OK;
    if (isRegister) {
        ret = window->RegisterRectChangeInGlobalDisplayListener(thisListener);
    } else {
        ret = window->UnregisterRectChangeInGlobalDisplayListener(thisListener);
    }
    return AniWindowUtils::ToErrorCode(ret);
}

WmErrorCode AniWindowRegisterManager::ProcessExtensionSecureLimitChangeRegister(const sptr<AniWindowListener>& listener,
    const sptr<Window>& window, bool isRegister, ani_env* env)
{
    if (window == nullptr || listener == nullptr) {
        return WmErrorCode::WM_ERROR_STATE_ABNORMALLY;
    }
    sptr<IExtensionSecureLimitChangeListener> thisListener(listener);
    WMError ret = WMError::WM_OK;
    if (isRegister) {
        ret = window->RegisterExtensionSecureLimitChangeListener(thisListener);
    } else {
        ret = window->UnregisterExtensionSecureLimitChangeListener(thisListener);
    }
    return AniWindowUtils::ToErrorCode(ret);
}

WmErrorCode AniWindowRegisterManager::ProcessWindowStatusDidChangeRegister(const sptr<AniWindowListener>& listener,
    const sptr<Window>& window, bool isRegister, ani_env* env)
{
    if (window == nullptr || listener == nullptr) {
        return WmErrorCode::WM_ERROR_STATE_ABNORMALLY;
    }
    sptr<IWindowStatusDidChangeListener> thisListener(listener);
    WMError ret = WMError::WM_OK;
    if (isRegister) {
        ret = window->RegisterWindowStatusDidChangeListener(thisListener);
    } else {
        ret = window->UnregisterWindowStatusDidChangeListener(thisListener);
    }
    return AniWindowUtils::ToErrorCode(ret);
}

WmErrorCode AniWindowRegisterManager::ProcessAcrossDisplaysChangeRegister(const sptr<AniWindowListener>& listener,
    const sptr<Window>& window, bool isRegister, ani_env* env)
{
    if (window == nullptr) {
        TLOGE(WmsLogTag::WMS_ATTRIBUTE, "window is null");
        return WmErrorCode::WM_ERROR_STATE_ABNORMALLY;
    }
    WmErrorCode ret = WmErrorCode::WM_OK;
    if (isRegister) {
        ret = WM_JS_TO_ERROR_CODE_MAP.at(window->RegisterAcrossDisplaysChangeListener(listener));
    } else {
        ret = WM_JS_TO_ERROR_CODE_MAP.at(window->UnRegisterAcrossDisplaysChangeListener(listener));
    }
    return ret;
}

WmErrorCode AniWindowRegisterManager::ProcessScreenshotAppEventRegister(const sptr<AniWindowListener>& listener,
    const sptr<Window>& window, bool isRegister, ani_env* env)
{
    if (window == nullptr) {
        TLOGE(WmsLogTag::WMS_ATTRIBUTE, "window is null");
        return WmErrorCode::WM_ERROR_STATE_ABNORMALLY;
    }
    WmErrorCode ret = WmErrorCode::WM_OK;
    if (isRegister) {
        ret = WM_JS_TO_ERROR_CODE_MAP.at(window->RegisterScreenshotAppEventListener(listener));
    } else {
        ret = WM_JS_TO_ERROR_CODE_MAP.at(window->UnregisterScreenshotAppEventListener(listener));
    }
    return ret;
}

WmErrorCode AniWindowRegisterManager::ProcessFreeWindowModeChangeRegister(const sptr<AniWindowListener>& listener,
    const sptr<Window>& window, bool isRegister, ani_env* env)
{
    if (window == nullptr) {
        return WmErrorCode::WM_ERROR_STATE_ABNORMALLY;
    }
    sptr<IFreeWindowModeChangeListener> thisListener(listener);
    WmErrorCode ret = WmErrorCode::WM_OK;
    if (isRegister) {
        ret = WM_JS_TO_ERROR_CODE_MAP.at(window->RegisterFreeWindowModeChangeListener(thisListener));
    } else {
        ret = WM_JS_TO_ERROR_CODE_MAP.at(window->UnregisterFreeWindowModeChangeListener(thisListener));
    }
    return ret;
}
} // namespace Rosen
} // namespace OHOS