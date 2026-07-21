/*
 * Copyright (c) 2024-2026 Huawei Device Co., Ltd.
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

#include "oh_window_event_filter.h"
#include "cstdint"
#include "functional"
#include "key_event.h"
#include "oh_window_comm.h"
#include "pointer_event.h"
#include "pointer_event_ndk.h"
#include "window.h"
#include "window_histogram_management.h"
#include "window_manager_hilog.h"
#include "wm_common.h"

using namespace OHOS::Rosen;

static const std::unordered_map<int32_t, Input_KeyEventAction> keyEventActionMap = {
    {OHOS::MMI::KeyEvent::KeyEvent::KEY_ACTION_CANCEL,    Input_KeyEventAction::KEY_ACTION_CANCEL },
    {OHOS::MMI::KeyEvent::KeyEvent::KEY_ACTION_DOWN,      Input_KeyEventAction::KEY_ACTION_DOWN   },
    {OHOS::MMI::KeyEvent::KeyEvent::KEY_ACTION_UP,        Input_KeyEventAction::KEY_ACTION_UP     },
};

static const std::unordered_map<int32_t, Input_MouseEventAction> mouseEventActionMap = {
    {OHOS::MMI::PointerEvent::POINTER_ACTION_CANCEL,         Input_MouseEventAction::MOUSE_ACTION_CANCEL      },
    {OHOS::MMI::PointerEvent::POINTER_ACTION_MOVE,           Input_MouseEventAction::MOUSE_ACTION_MOVE        },
    {OHOS::MMI::PointerEvent::POINTER_ACTION_BUTTON_DOWN,    Input_MouseEventAction::MOUSE_ACTION_BUTTON_DOWN },
    {OHOS::MMI::PointerEvent::POINTER_ACTION_BUTTON_UP,      Input_MouseEventAction::MOUSE_ACTION_BUTTON_UP   },
};

static const std::unordered_map<int32_t, Input_MouseEventButton> mouseEventButtonMap = {
    {OHOS::MMI::PointerEvent::BUTTON_NONE,             Input_MouseEventButton::MOUSE_BUTTON_NONE    },
    {OHOS::MMI::PointerEvent::MOUSE_BUTTON_LEFT,       Input_MouseEventButton::MOUSE_BUTTON_LEFT    },
    {OHOS::MMI::PointerEvent::MOUSE_BUTTON_MIDDLE,     Input_MouseEventButton::MOUSE_BUTTON_MIDDLE  },
    {OHOS::MMI::PointerEvent::MOUSE_BUTTON_RIGHT,      Input_MouseEventButton::MOUSE_BUTTON_RIGHT   },
    {OHOS::MMI::PointerEvent::MOUSE_BUTTON_FORWARD,    Input_MouseEventButton::MOUSE_BUTTON_FORWARD },
    {OHOS::MMI::PointerEvent::MOUSE_BUTTON_BACK,       Input_MouseEventButton::MOUSE_BUTTON_BACK    },
};

KeyEventFilterFunc convert2KeyEventFilterFunc(OH_NativeWindowManager_KeyEventFilter filter)
{
    return [filter](const OHOS::MMI::KeyEvent& keyEvent) {
        Input_KeyEvent* input = OH_Input_CreateKeyEvent();
        OH_Input_SetKeyEventKeyCode(input, keyEvent.GetKeyCode());
        auto iter = keyEventActionMap.find(keyEvent.GetKeyAction());
        if (iter == keyEventActionMap.end()) {
            return false;
        }
        OH_Input_SetKeyEventAction(input, iter->second);
        OH_Input_SetKeyEventActionTime(input, keyEvent.GetActionTime());
        OH_Input_SetKeyEventWindowId(input, keyEvent.GetTargetWindowId());
        OH_Input_SetKeyEventDisplayId(input, keyEvent.GetTargetDisplayId());
        return filter(input);
    };
}

WindowManager_ErrorCode OH_NativeWindowManager_RegisterKeyEventFilter(int32_t windowId,
    OH_NativeWindowManager_KeyEventFilter keyEventFilter)
{
    WindowManager_ErrorCode err;
    auto window = OHOS::Rosen::Window::GetWindowWithId(windowId);
    if (window == nullptr) {
        err = WindowManager_ErrorCode::INVALID_WINDOW_ID;
        HISTOGRAM_ENUMERATION_WINDOW_MANAGER_ERROR_CODE("ArkUI.window.registerKeyEventFilter", err);
        TLOGE(WmsLogTag::WMS_EVENT, "window is null, windowId=%{public}d", windowId);
        return err;
    }
    if (keyEventFilter == nullptr) {
        err = WindowManager_ErrorCode::WINDOW_MANAGER_ERRORCODE_INVALID_PARAM;
        HISTOGRAM_ENUMERATION_WINDOW_MANAGER_ERROR_CODE("ArkUI.window.registerKeyEventFilter", err);
        TLOGE(WmsLogTag::WMS_EVENT, "keyEventFilter is null");
        return err;
    }
    auto ret = window->SetKeyEventFilter(convert2KeyEventFilterFunc(keyEventFilter));
    if (ret != WMError::WM_OK) {
        err = WindowManager_ErrorCode::SERVICE_ERROR;
        HISTOGRAM_ENUMERATION_WINDOW_MANAGER_ERROR_CODE("ArkUI.window.registerKeyEventFilter", err);
        TLOGE(WmsLogTag::WMS_EVENT, "Set filter failed, the device is not supported");
        return err;
    }
    window->SaveNativeKeyEventFilter(keyEventFilter);
    TLOGD(WmsLogTag::WMS_EVENT, "register success, windowId=%{public}d", windowId);
    return WindowManager_ErrorCode::OK;
}

WindowManager_ErrorCode OH_NativeWindowManager_UnregisterKeyEventFilter(int32_t windowId)
{
    WindowManager_ErrorCode err;
    auto window = OHOS::Rosen::Window::GetWindowWithId(windowId);
    if (window == nullptr) {
        err = WindowManager_ErrorCode::INVALID_WINDOW_ID;
        HISTOGRAM_ENUMERATION_WINDOW_MANAGER_ERROR_CODE("ArkUI.window.unregisterKeyEventFilter", err);
        TLOGE(WmsLogTag::WMS_EVENT, "window is null, windowId=%{public}d", windowId);
        return err;
    }
    auto ret = window->ClearKeyEventFilter();
    if (ret != WMError::WM_OK) {
        err = WindowManager_ErrorCode::SERVICE_ERROR;
        HISTOGRAM_ENUMERATION_WINDOW_MANAGER_ERROR_CODE("ArkUI.window.unregisterKeyEventFilter", err);
        TLOGE(WmsLogTag::WMS_EVENT, "Clear filter failed, device is not supported");
        return err;
    }
    window->ClearNativeKeyEventFilter();
    TLOGD(WmsLogTag::WMS_EVENT, "clear success, windowId=%{public}d", windowId);
    return WindowManager_ErrorCode::OK;
}

MouseEventFilterFunc convert2MouseEventFilterFunc(OH_NativeWindowManager_MouseEventFilter filter)
{
    return [filter](const OHOS::MMI::PointerEvent& event) {
        Input_MouseEvent* mouseEvent = OH_Input_CreateMouseEvent();
        if (mouseEvent == nullptr) {
            TLOGNE(WmsLogTag::WMS_EVENT, "create input mouse event fail");
            return false;
        }
        OHOS::MMI::PointerEvent::PointerItem item;
        if (!event.GetPointerItem(event.GetPointerId(), item)) {
            TLOGNE(WmsLogTag::WMS_EVENT, "Can not get pointerItem for the pointer event");
            OH_Input_DestroyMouseEvent(&mouseEvent);
            return false;
        }
        auto actionIter = mouseEventActionMap.find(event.GetPointerAction());
        if (actionIter == mouseEventActionMap.end()) {
            TLOGNE(WmsLogTag::WMS_EVENT, "find mouse event action fail");
            OH_Input_DestroyMouseEvent(&mouseEvent);
            return false;
        }
        OH_Input_SetMouseEventAction(mouseEvent, actionIter->second);
        auto buttonIter = mouseEventButtonMap.find(event.GetButtonId());
        if (buttonIter == mouseEventButtonMap.end()) {
            TLOGNE(WmsLogTag::WMS_EVENT, "find mouse event button fail");
            OH_Input_DestroyMouseEvent(&mouseEvent);
            return false;
        }
        OH_Input_SetMouseEventButton(mouseEvent, buttonIter->second);
        OH_Input_SetMouseEventDisplayX(mouseEvent, item.GetDisplayX());
        OH_Input_SetMouseEventDisplayY(mouseEvent, item.GetDisplayY());
        OH_Input_SetMouseEventActionTime(mouseEvent, event.GetActionTime());
        OH_Input_SetMouseEventWindowId(mouseEvent, event.GetTargetWindowId());
        OH_Input_SetMouseEventDisplayId(mouseEvent, event.GetTargetDisplayId());
        OH_Input_SetMouseEventGlobalX(mouseEvent, item.GetGlobalX());
        OH_Input_SetMouseEventGlobalY(mouseEvent, item.GetGlobalY());
        bool res = filter(mouseEvent);
        OH_Input_DestroyMouseEvent(&mouseEvent);
        return res;
    };
}

WindowManager_ErrorCode OH_NativeWindowManager_RegisterMouseEventFilter(int32_t windowId,
    OH_NativeWindowManager_MouseEventFilter mouseEventFilter)
{
    WindowManager_ErrorCode err;
    auto window = OHOS::Rosen::Window::GetWindowWithId(windowId);
    if (window == nullptr) {
        err = WindowManager_ErrorCode::INVALID_WINDOW_ID;
        HISTOGRAM_ENUMERATION_WINDOW_MANAGER_ERROR_CODE("ArkUI.window.registerMouseEventFilter", err);
        TLOGE(WmsLogTag::WMS_EVENT, "window is null, windowId=%{public}d", windowId);
        return err;
    }
    if (mouseEventFilter == nullptr) {
        err = WindowManager_ErrorCode::WINDOW_MANAGER_ERRORCODE_INVALID_PARAM;
        HISTOGRAM_ENUMERATION_WINDOW_MANAGER_ERROR_CODE("ArkUI.window.registerMouseEventFilter", err);
        TLOGE(WmsLogTag::WMS_EVENT, "mouseEventFilter is null");
        return err;
    }
    auto ret = window->SetMouseEventFilter(convert2MouseEventFilterFunc(mouseEventFilter));
    if (ret != WMError::WM_OK) {
        err = WindowManager_ErrorCode::SERVICE_ERROR;
        HISTOGRAM_ENUMERATION_WINDOW_MANAGER_ERROR_CODE("ArkUI.window.registerMouseEventFilter", err);
        TLOGE(WmsLogTag::WMS_EVENT, "Set filter failed, the device is not supported");
        return err;
    }
    window->SaveNativeMouseEventFilter(mouseEventFilter);
    TLOGD(WmsLogTag::WMS_EVENT, "register success, windowId=%{public}d", windowId);
    return WindowManager_ErrorCode::OK;
}

WindowManager_ErrorCode OH_NativeWindowManager_UnregisterMouseEventFilter(int32_t windowId)
{
    WindowManager_ErrorCode err;
    auto window = OHOS::Rosen::Window::GetWindowWithId(windowId);
    if (window == nullptr) {
        err = WindowManager_ErrorCode::INVALID_WINDOW_ID;
        HISTOGRAM_ENUMERATION_WINDOW_MANAGER_ERROR_CODE("ArkUI.window.unregisterMouseEventFilter", err);
        TLOGE(WmsLogTag::WMS_EVENT, "window is null, windowId=%{public}d", windowId);
        return err;
    }
    auto ret = window->ClearMouseEventFilter();
    if (ret != WMError::WM_OK) {
        err = WindowManager_ErrorCode::SERVICE_ERROR;
        HISTOGRAM_ENUMERATION_WINDOW_MANAGER_ERROR_CODE("ArkUI.window.unregisterMouseEventFilter", err);
        TLOGE(WmsLogTag::WMS_EVENT, "Clear filter failed, device is not supported");
        return err;
    }
    window->ClearNativeMouseEventFilter();
    TLOGD(WmsLogTag::WMS_EVENT, "clear success, windowId=%{public}d", windowId);
    return WindowManager_ErrorCode::OK;
}

TouchEventFilterFunc convert2TouchEventFilterFunc(OH_NativeWindowManager_TouchEventFilter filter)
{
    return [filter](const OHOS::MMI::PointerEvent& event) {
        auto touchEvent = OH_Input_PointerEventToTouchEvent(event);
        if (touchEvent == nullptr) {
            TLOGNE(WmsLogTag::WMS_EVENT, "to touch event fail");
            return false;
        }
        bool res = filter(touchEvent);
        OH_Input_DestroyTouchEvent(&touchEvent);
        return res;
    };
}

WindowManager_ErrorCode OH_NativeWindowManager_RegisterTouchEventFilter(int32_t windowId,
    OH_NativeWindowManager_TouchEventFilter touchEventFilter)
{
    WindowManager_ErrorCode err;
    auto window = OHOS::Rosen::Window::GetWindowWithId(windowId);
    if (window == nullptr) {
        err = WindowManager_ErrorCode::INVALID_WINDOW_ID;
        HISTOGRAM_ENUMERATION_WINDOW_MANAGER_ERROR_CODE("ArkUI.window.registerTouchEventFilter", err);
        TLOGE(WmsLogTag::WMS_EVENT, "window is null, windowId=%{public}d", windowId);
        return err;
    }
    if (touchEventFilter == nullptr) {
        err = WindowManager_ErrorCode::WINDOW_MANAGER_ERRORCODE_INVALID_PARAM;
        HISTOGRAM_ENUMERATION_WINDOW_MANAGER_ERROR_CODE("ArkUI.window.registerTouchEventFilter", err);
        TLOGE(WmsLogTag::WMS_EVENT, "touchEventFilter is null");
        return err;
    }
    auto ret = window->SetTouchEventFilter(convert2TouchEventFilterFunc(touchEventFilter));
    if (ret != WMError::WM_OK) {
        err = WindowManager_ErrorCode::SERVICE_ERROR;
        HISTOGRAM_ENUMERATION_WINDOW_MANAGER_ERROR_CODE("ArkUI.window.registerTouchEventFilter", err);
        TLOGE(WmsLogTag::WMS_EVENT, "Set filter failed, the device is not supported");
        return err;
    }
    window->SaveNativeTouchEventFilter(touchEventFilter);
    TLOGD(WmsLogTag::WMS_EVENT, "register success, windowId=%{public}d", windowId);
    return WindowManager_ErrorCode::OK;
}

WindowManager_ErrorCode OH_NativeWindowManager_UnregisterTouchEventFilter(int32_t windowId)
{
    WindowManager_ErrorCode err;
    auto window = OHOS::Rosen::Window::GetWindowWithId(windowId);
    if (window == nullptr) {
        err = WindowManager_ErrorCode::INVALID_WINDOW_ID;
        HISTOGRAM_ENUMERATION_WINDOW_MANAGER_ERROR_CODE("ArkUI.window.unregisterTouchEventFilter", err);
        TLOGE(WmsLogTag::WMS_EVENT, "window is null, windowId=%{public}d", windowId);
        return err;
    }
    auto ret = window->ClearTouchEventFilter();
    if (ret != WMError::WM_OK) {
        err = WindowManager_ErrorCode::SERVICE_ERROR;
        HISTOGRAM_ENUMERATION_WINDOW_MANAGER_ERROR_CODE("ArkUI.window.unregisterTouchEventFilter", err);
        TLOGE(WmsLogTag::WMS_EVENT, "Clear filter failed, device is not supported");
        return err;
    }
    window->ClearNativeTouchEventFilter();
    TLOGD(WmsLogTag::WMS_EVENT, "clear success, windowId=%{public}d", windowId);
    return WindowManager_ErrorCode::OK;
}

WindowManager_ErrorCode OH_NativeWindowManager_GetKeyEventFilter(int32_t windowId,
    OH_NativeWindowManager_KeyEventFilter* outKeyEventFilter)
{
    auto window = OHOS::Rosen::Window::GetWindowWithId(windowId);
    if (window == nullptr) {
        TLOGE(WmsLogTag::WMS_EVENT, "window is null, windowId=%{public}d", windowId);
        return WindowManager_ErrorCode::INVALID_WINDOW_ID;
    }
    if (outKeyEventFilter == nullptr) {
        TLOGE(WmsLogTag::WMS_EVENT, "outKeyEventFilter is null");
        return WindowManager_ErrorCode::WINDOW_MANAGER_ERRORCODE_INVALID_PARAM;
    }
    NativeKeyEventFilter nativeFilter = window->GetNativeKeyEventFilter();
    if (nativeFilter == nullptr) {
        *outKeyEventFilter = nullptr;
        TLOGW(WmsLogTag::WMS_EVENT, "The filter is not found, windowId=%{public}d", windowId);
    } else {
        *outKeyEventFilter = nativeFilter;
        TLOGI(WmsLogTag::WMS_EVENT, "Get filter success, windowId=%{public}d", windowId);
    }
    return WindowManager_ErrorCode::OK;
}

WindowManager_ErrorCode OH_NativeWindowManager_GetMouseEventFilter(int32_t windowId,
    OH_NativeWindowManager_MouseEventFilter* outMouseEventFilter)
{
    auto window = OHOS::Rosen::Window::GetWindowWithId(windowId);
    if (window == nullptr) {
        TLOGE(WmsLogTag::WMS_EVENT, "window is null, windowId=%{public}d", windowId);
        return WindowManager_ErrorCode::INVALID_WINDOW_ID;
    }
    if (outMouseEventFilter == nullptr) {
        TLOGE(WmsLogTag::WMS_EVENT, "outMouseEventFilter is null");
        return WindowManager_ErrorCode::WINDOW_MANAGER_ERRORCODE_INVALID_PARAM;
    }
    NativeMouseEventFilter nativeFilter = window->GetNativeMouseEventFilter();
    if (nativeFilter == nullptr) {
        *outMouseEventFilter = nullptr;
        TLOGW(WmsLogTag::WMS_EVENT, "The filter is not found, windowId=%{public}d", windowId);
    } else {
        *outMouseEventFilter = nativeFilter;
        TLOGI(WmsLogTag::WMS_EVENT, "Get filter success, windowId=%{public}d", windowId);
    }
    return WindowManager_ErrorCode::OK;
}

WindowManager_ErrorCode OH_NativeWindowManager_GetTouchEventFilter(int32_t windowId,
    OH_NativeWindowManager_TouchEventFilter* outTouchEventFilter)
{
    auto window = OHOS::Rosen::Window::GetWindowWithId(windowId);
    if (window == nullptr) {
        TLOGE(WmsLogTag::WMS_EVENT, "window is null, windowId=%{public}d", windowId);
        return WindowManager_ErrorCode::INVALID_WINDOW_ID;
    }
    if (outTouchEventFilter == nullptr) {
        TLOGE(WmsLogTag::WMS_EVENT, "outTouchEventFilter is null");
        return WindowManager_ErrorCode::WINDOW_MANAGER_ERRORCODE_INVALID_PARAM;
    }
    NativeTouchEventFilter nativeFilter = window->GetNativeTouchEventFilter();
    if (nativeFilter == nullptr) {
        *outTouchEventFilter = nullptr;
        TLOGW(WmsLogTag::WMS_EVENT, "The filter is not found, windowId=%{public}d", windowId);
    } else {
        *outTouchEventFilter = nativeFilter;
        TLOGI(WmsLogTag::WMS_EVENT, "Get filter success, windowId=%{public}d", windowId);
    }
    return WindowManager_ErrorCode::OK;
}
