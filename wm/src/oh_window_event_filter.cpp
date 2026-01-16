/*
 * Copyright (c) 2024 Huawei Device Co., Ltd.
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

#include "cstdint"
#include "functional"
#include "oh_window_comm.h"
#include "oh_window_event_filter.h"
#include "key_event.h"
#include "pointer_event.h"
#include "window.h"
#include "window_manager_hilog.h"
#include "wm_common.h"

using namespace OHOS::Rosen;

static const std::unordered_map<int32_t, Input_KeyEventAction> keyEventActionMap = {
    {OHOS::MMI::KeyEvent::KeyEvent::KEY_ACTION_CANCEL,    Input_KeyEventAction::KEY_ACTION_CANCEL },
    {OHOS::MMI::KeyEvent::KeyEvent::KEY_ACTION_DOWN,      Input_KeyEventAction::KEY_ACTION_DOWN   },
    {OHOS::MMI::KeyEvent::KeyEvent::KEY_ACTION_UP,        Input_KeyEventAction::KEY_ACTION_UP     },
};

static const std::unordered_map<int32_t, Input_TouchEventAction> touchEventActionMap = {
    {OHOS::MMI::PointerEvent::POINTER_ACTION_CANCEL,    Input_TouchEventAction::TOUCH_ACTION_CANCEL },
    {OHOS::MMI::PointerEvent::POINTER_ACTION_DOWN,      Input_TouchEventAction::TOUCH_ACTION_DOWN   },
    {OHOS::MMI::PointerEvent::POINTER_ACTION_MOVE,      Input_TouchEventAction::TOUCH_ACTION_MOVE   },
    {OHOS::MMI::PointerEvent::POINTER_ACTION_UP,        Input_TouchEventAction::TOUCH_ACTION_UP     },
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

KeyEventFilterFunc convert2Func(OH_NativeWindowManager_KeyEventFilter filter)
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
    OH_NativeWindowManager_KeyEventFilter filter)
{
    TLOGI(WmsLogTag::WMS_EVENT, "register keyEventCallback, wid:%{public}d", windowId);
    auto mainWindow = OHOS::Rosen::Window::GetWindowWithId(windowId);
    if (mainWindow == nullptr) {
        TLOGE(WmsLogTag::WMS_EVENT, "window is null, wid:%{public}d", windowId);
        return WindowManager_ErrorCode::INVAILD_WINDOW_ID;
    }
    auto res = mainWindow->SetKeyEventFilter(convert2Func(filter));

    return res == WMError::WM_OK ? WindowManager_ErrorCode::OK : WindowManager_ErrorCode::SERVICE_ERROR;
}

WindowManager_ErrorCode OH_NativeWindowManager_UnregisterKeyEventFilter(int32_t windowId)
{
    TLOGI(WmsLogTag::WMS_EVENT, "clear keyEventCallback, wid:%{public}d", windowId);
    auto mainWindow = OHOS::Rosen::Window::GetWindowWithId(windowId);
    if (mainWindow == nullptr) {
        TLOGE(WmsLogTag::WMS_EVENT, "window is null, wid:%{public}d", windowId);
        return WindowManager_ErrorCode::INVAILD_WINDOW_ID;
    }
    auto res = mainWindow->ClearKeyEventFilter();
    return res == WMError::WM_OK ? WindowManager_ErrorCode::OK : WindowManager_ErrorCode::SERVICE_ERROR;
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
    TLOGI(WmsLogTag::WMS_EVENT, "register mouseEventCallback, wid:%{public}d", windowId);
    auto window = OHOS::Rosen::Window::GetWindowWithId(windowId);
    if (window == nullptr) {
        TLOGE(WmsLogTag::WMS_EVENT, "window is null, wid:%{public}d", windowId);
        return WindowManager_ErrorCode::INVAILD_WINDOW_ID;
    }
    auto res = window->SetMouseEventFilter(convert2MouseEventFilterFunc(mouseEventFilter));
    return res == WMError::WM_OK ? WindowManager_ErrorCode::OK : WindowManager_ErrorCode::SERVICE_ERROR;
}

WindowManager_ErrorCode OH_NativeWindowManager_UnregisterMouseEventFilter(int32_t windowId)
{
    TLOGI(WmsLogTag::WMS_EVENT, "clear mouseEventCallback, wid:%{public}d", windowId);
    auto window = OHOS::Rosen::Window::GetWindowWithId(windowId);
    if (window == nullptr) {
        TLOGE(WmsLogTag::WMS_EVENT, "window is null, wid:%{public}d", windowId);
        return WindowManager_ErrorCode::INVAILD_WINDOW_ID;
    }
    auto res = window->ClearMouseEventFilter();
    return res == WMError::WM_OK ? WindowManager_ErrorCode::OK : WindowManager_ErrorCode::SERVICE_ERROR;
}

TouchEventFilterFunc convert2TouchEventFilterFunc(OH_NativeWindowManager_TouchEventFilter filter)
{
    return [filter](const OHOS::MMI::PointerEvent& event) {
        Input_TouchEvent* touchEvent = OH_Input_CreateTouchEvent();
        if (touchEvent == nullptr) {
            TLOGNE(WmsLogTag::WMS_EVENT, "create input touch event fail");
            return false;
        }
        OHOS::MMI::PointerEvent::PointerItem item;
        if (!event.GetPointerItem(event.GetPointerId(), item)) {
            TLOGNE(WmsLogTag::WMS_EVENT, "Can not get pointerItem for the pointer event");
            OH_Input_DestroyTouchEvent(&touchEvent);
            return false;
        }
        auto actionIter = touchEventActionMap.find(event.GetPointerAction());
        if (actionIter == touchEventActionMap.end()) {
            TLOGNI(WmsLogTag::WMS_EVENT, "unknown touch type");
            OH_Input_DestroyTouchEvent(&touchEvent);
            return false;
        }
        OH_Input_SetTouchEventAction(touchEvent, actionIter->second);
        OH_Input_SetTouchEventFingerId(touchEvent, event.GetPointerId());
        OH_Input_SetTouchEventDisplayX(touchEvent, item.GetDisplayX());
        OH_Input_SetTouchEventDisplayY(touchEvent, item.GetDisplayY());
        OH_Input_SetTouchEventActionTime(touchEvent, event.GetActionTime());
        OH_Input_SetTouchEventWindowId(touchEvent, event.GetTargetWindowId());
        OH_Input_SetTouchEventDisplayId(touchEvent, event.GetTargetDisplayId());
        OH_Input_SetTouchEventGlobalX(touchEvent, item.GetGlobalX());
        OH_Input_SetTouchEventGlobalY(touchEvent, item.GetGlobalY());
        OH_Input_SetTouchEventPressure(touchEvent, item.GetPressure());
        OH_Input_SetTouchEventWindowX(touchEvent, item.GetWindowX());
        OH_Input_SetTouchEventWindowY(touchEvent, item.GetWindowY());
        OH_Input_SetTouchEventToolType(touchEvent, static_cast<Input_TouchEventToolType>(item.GetToolType()));
        OH_Input_SetTouchEventDownTime(touchEvent, item.GetDownTime());
        bool res = filter(touchEvent);
        OH_Input_DestroyTouchEvent(&touchEvent);
        return res;
    };
}

WindowManager_ErrorCode OH_NativeWindowManager_RegisterTouchEventFilter(int32_t windowId,
    OH_NativeWindowManager_TouchEventFilter touchEventFilter)
{
    TLOGI(WmsLogTag::WMS_EVENT, "register touchEventCallback, wid:%{public}d", windowId);
    auto window = OHOS::Rosen::Window::GetWindowWithId(windowId);
    if (window == nullptr) {
        TLOGE(WmsLogTag::WMS_EVENT, "window is null, wid:%{public}d", windowId);
        return WindowManager_ErrorCode::INVAILD_WINDOW_ID;
    }
    auto res = window->SetTouchEventFilter(convert2TouchEventFilterFunc(touchEventFilter));
    return res == WMError::WM_OK ? WindowManager_ErrorCode::OK : WindowManager_ErrorCode::SERVICE_ERROR;
}

WindowManager_ErrorCode OH_NativeWindowManager_UnregisterTouchEventFilter(int32_t windowId)
{
    TLOGI(WmsLogTag::WMS_EVENT, "clear touchEventCallback, wid:%{public}d", windowId);
    auto window = OHOS::Rosen::Window::GetWindowWithId(windowId);
    if (window == nullptr) {
        TLOGE(WmsLogTag::WMS_EVENT, "window is null, wid:%{public}d", windowId);
        return WindowManager_ErrorCode::INVAILD_WINDOW_ID;
    }
    auto res = window->ClearTouchEventFilter();
    return res == WMError::WM_OK ? WindowManager_ErrorCode::OK : WindowManager_ErrorCode::SERVICE_ERROR;
}
