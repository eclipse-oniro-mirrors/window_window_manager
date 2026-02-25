/*
 * Copyright (c) 2023 Huawei Device Co., Ltd.
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

#include "intention_event_manager.h"

#ifdef IMF_ENABLE
#include <input_method_controller.h>
#endif // IMF_ENABLE
#include "session_helper.h"
#include "session_manager/include/scene_session_manager.h"
#include "window_manager_hilog.h"
#include <hitrace_meter.h>
#include "parameters.h"
#include "xcollie/xcollie.h"

namespace OHOS {
namespace Rosen {
namespace {
constexpr HiviewDFX::HiLogLabel LABEL = { LOG_CORE, HILOG_DOMAIN_WINDOW, "IntentionEventManager" };
constexpr int32_t TRANSPARENT_FINGER_ID = 10000;
constexpr int32_t DELAY_TIME = 15;
constexpr unsigned int FREQUENT_CLICK_TIME_LIMIT = 3;
constexpr int FREQUENT_CLICK_COUNT_LIMIT = 8;
static const bool IS_BETA = OHOS::system::GetParameter("const.logsystem.versiontype", "").find("beta") !=
    std::string::npos;

void LogPointInfo(const std::shared_ptr<MMI::PointerEvent>& pointerEvent)
{
    if (pointerEvent == nullptr) {
        return;
    }

    uint32_t windowId = static_cast<uint32_t>(pointerEvent->GetTargetWindowId());
    TLOGD(WmsLogTag::WMS_EVENT, "point source:%{public}d", pointerEvent->GetSourceType());
    auto actionId = pointerEvent->GetPointerId();
    int32_t action = pointerEvent->GetPointerAction();
    if (action == MMI::PointerEvent::POINTER_ACTION_MOVE) {
        return;
    }

    MMI::PointerEvent::PointerItem item;
    if (pointerEvent->GetPointerItem(actionId, item)) {
        TLOGD(WmsLogTag::WMS_EVENT, "action point info:windowid:%{public}d,id:%{public}d,displayx:%{private}d,"
            "displayy:%{private}d,windowx:%{private}d,windowy:%{private}d,action:%{public}d pressure:"
            "%{public}f,tiltx:%{public}f,tiltY:%{public}f",
            windowId, actionId, item.GetDisplayX(), item.GetDisplayY(), item.GetWindowX(), item.GetWindowY(),
            pointerEvent->GetPointerAction(), item.GetPressure(), item.GetTiltX(), item.GetTiltY());
    }
    auto ids = pointerEvent->GetPointerIds();
    for (auto&& id :ids) {
        MMI::PointerEvent::PointerItem item;
        if (pointerEvent->GetPointerItem(id, item)) {
            TLOGD(WmsLogTag::WMS_EVENT, "all point info: id:%{public}d,x:%{private}d,y:%{private}d,"
                "isPressend:%{public}d,pressure:%{public}f,tiltX:%{public}f,tiltY:%{public}f",
            actionId, item.GetWindowX(), item.GetWindowY(), item.IsPressed(), item.GetPressure(),
            item.GetTiltX(), item.GetTiltY());
        }
    }
}
} // namespace

IntentionEventManager::IntentionEventManager() {}
IntentionEventManager::~IntentionEventManager() {}

IntentionEventManager::InputEventListener::~InputEventListener()
{
}

bool IntentionEventManager::EnableInputEventListener(Ace::UIContent* uiContent,
    std::shared_ptr<AppExecFwk::EventHandler> eventHandler, wptr<Window> window)
{
    if (uiContent == nullptr) {
        TLOGE(WmsLogTag::WMS_EVENT, "EnableInputEventListener uiContent is null");
        return false;
    }
    if (eventHandler == nullptr) {
        TLOGE(WmsLogTag::WMS_EVENT, "EnableInputEventListener eventHandler is null");
        return false;
    }
    auto listener =
        std::make_shared<IntentionEventManager::InputEventListener>(uiContent, eventHandler, window);
    MMI::InputManager::GetInstance()->SetWindowInputEventConsumer(listener, eventHandler);
    TLOGI(WmsLogTag::WMS_EVENT, "SetWindowInputEventConsumer success");

    if (IS_BETA) {
        // Xcollie's SetTimerCounter task is set with the params to record count and time of the input down event
        int id = HiviewDFX::XCollie::GetInstance().SetTimerCount("FREQUENT_CLICK_WARNING", FREQUENT_CLICK_TIME_LIMIT,
            FREQUENT_CLICK_COUNT_LIMIT);
    }
    return true;
}

void IntentionEventManager::InputEventListener::SetPointerEventStatus(
    int32_t fingerId, int32_t action, int32_t sourceType, const sptr<SceneSession>& sceneSession) const
{
    switch (action) {
        case MMI::PointerEvent::POINTER_ACTION_DOWN:
            sceneSession->SetFingerPointerDownStatus(fingerId);
            break;
        case MMI::PointerEvent::POINTER_ACTION_UP:
            sceneSession->RemoveFingerPointerDownStatus(fingerId);
            break;
        case MMI::PointerEvent::POINTER_ACTION_BUTTON_DOWN:
            sceneSession->SetMousePointerDownEventStatus(true);
            break;
        case MMI::PointerEvent::POINTER_ACTION_BUTTON_UP:
            sceneSession->SetMousePointerDownEventStatus(false);
            break;
        case MMI::PointerEvent::POINTER_ACTION_CANCEL:
            if (sourceType == MMI::PointerEvent::SOURCE_TYPE_MOUSE) {
                sceneSession->SetMousePointerDownEventStatus(false);
            } else {
                sceneSession->RemoveFingerPointerDownStatus(fingerId);
            }
            break;
        default:
            break;
    }
}

bool IntentionEventManager::InputEventListener::CheckPointerEvent(
    const std::shared_ptr<MMI::PointerEvent> pointerEvent) const
{
    if (pointerEvent == nullptr) {
        TLOGE(WmsLogTag::WMS_EVENT, "pointerEvent is null");
        return false;
    }
    HITRACE_METER_FMT(HITRACE_TAG_WINDOW_MANAGER, "IEM:PointerEvent id:%d action:%d",
        pointerEvent->GetId(), pointerEvent->GetPointerAction());
    if (uiContent_ == nullptr) {
        TLOGE(WmsLogTag::WMS_EVENT, "uiContent_ is null");
        pointerEvent->MarkProcessed();
        return false;
    }
    if (!SceneSessionManager::GetInstance().IsInputEventEnabled()) {
        TLOGW(WmsLogTag::WMS_EVENT, "inputEvent is disabled temporarily, eventId is %{public}d", pointerEvent->GetId());
        pointerEvent->MarkProcessed();
        return false;
    }
    return true;
}

void IntentionEventManager::InputEventListener::OnInputEvent(
    std::shared_ptr<MMI::PointerEvent> pointerEvent) const
{
    if (!CheckPointerEvent(pointerEvent)) {
        return;
    }
    LogPointInfo(pointerEvent);
    int32_t action = pointerEvent->GetPointerAction();
    uint32_t windowId = static_cast<uint32_t>(pointerEvent->GetTargetWindowId());
    auto sceneSession = SceneSessionManager::GetInstance().GetSceneSession(windowId);
    if (sceneSession == nullptr) {
        TLOGE(WmsLogTag::WMS_INPUT_KEY_FLOW, "The scene session is nullptr");
        pointerEvent->MarkProcessed();
        return;
    }
    auto dispatchTimes = pointerEvent->GetDispatchTimes();
    if (dispatchTimes > 0) {
        MMI::PointerEvent::PointerItem pointerItem;
        auto pointerId = pointerEvent->GetPointerId();
        if (pointerEvent->GetPointerItem(pointerId, pointerItem)) {
            pointerItem.SetPointerId(pointerId + dispatchTimes * TRANSPARENT_FINGER_ID);
            pointerEvent->UpdatePointerItem(pointerId, pointerItem);
            pointerEvent->SetPointerId(pointerId + dispatchTimes * TRANSPARENT_FINGER_ID);
        }
    }
    auto sourceType = pointerEvent->GetSourceType();
    if (action != MMI::PointerEvent::POINTER_ACTION_MOVE) {
        if (sourceType == MMI::PointerEvent::SOURCE_TYPE_MOUSE ||
            sourceType == MMI::PointerEvent::SOURCE_TYPE_TOUCHSCREEN) {
            SetPointerEventStatus(pointerEvent->GetPointerId(), action, sourceType, sceneSession);
        }
        static uint32_t eventId = 0;
        TLOGI(WmsLogTag::WMS_INPUT_KEY_FLOW, "eid:%{public}d,InputId:%{public}d,wid:%{public}u"
            ",wName:%{public}s,ac:%{public}d,sys:%{public}d", eventId++, pointerEvent->GetId(), windowId,
            sceneSession->GetSessionInfo().abilityName_.c_str(), action, sceneSession->GetSessionInfo().isSystem_);
    }
    if (sceneSession->GetSessionInfo().isSystem_) {
        sceneSession->SendPointerEventToUI(pointerEvent);
        auto window = window_.promote();
        if (window != nullptr) {
            HITRACE_METER_FMT(HITRACE_TAG_WINDOW_MANAGER, "SetTouchEvent, action:%d", action);
            window->SetTouchEvent(action);
        }
        // notify touchOutside and touchDown event
        if (action == MMI::PointerEvent::POINTER_ACTION_DOWN ||
            action == MMI::PointerEvent::POINTER_ACTION_BUTTON_DOWN) {
            MMI::PointerEvent::PointerItem pointerItem;
            if (pointerEvent->GetPointerItem(pointerEvent->GetPointerId(), pointerItem)) {
                sceneSession->ProcessPointDownSession(pointerItem.GetDisplayX(), pointerItem.GetDisplayY());
            }
            if (IS_BETA) {
                /*
                 * Triggers input down event recorded.
                 * If the special num of the events reached within the sepcial time interval,
                 * a panic behavior is reported.
                 */
                HiviewDFX::XCollie::GetInstance().TriggerTimerCount("FREQUENT_CLICK_WARNING", true, "");
            }
        }
    } else {
        // transfer pointer event for move and drag
        WSError ret = sceneSession->TransferPointerEvent(pointerEvent);
        if (sceneSession->GetWindowType() == WindowType::WINDOW_TYPE_SYSTEM_FLOAT) {
            sceneSession->NotifyOutsideDownEvent(pointerEvent);
        }
        if ((ret != WSError::WS_OK || static_cast<int32_t>(getprocpid()) != sceneSession->GetCallingPid()) &&
            pointerEvent != nullptr) {
            pointerEvent->MarkProcessed();
        }
    }
}

void IntentionEventManager::InputEventListener::SendKeyEventConsumedResultToSCB(
    const std::shared_ptr<MMI::KeyEvent>& keyEvent, bool isConsumed) const
{
    if ((keyEvent->GetKeyCode() == MMI::KeyEvent::KEYCODE_TAB ||
        keyEvent->GetKeyCode() == MMI::KeyEvent::KEYCODE_ENTER) &&
        keyEvent->GetKeyAction() == MMI::KeyEvent::KEY_ACTION_DOWN) {
        TLOGD(WmsLogTag::WMS_EVENT, "isConsumed:%{public}d", isConsumed);
        SceneSessionManager::GetInstance().NotifyWatchGestureConsumeResult(keyEvent->GetKeyCode(), isConsumed);
    }
}

void IntentionEventManager::InputEventListener::DispatchKeyEventCallback(
    int32_t focusedSessionId, std::shared_ptr<MMI::KeyEvent> keyEvent, bool consumed) const
{
    if (keyEvent == nullptr) {
        WLOGFW("keyEvent is null, focusedSessionId:%{public}" PRId32
            ", consumed:%{public}" PRId32, focusedSessionId, consumed);
        return;
    }

    if (consumed) {
        WLOGD("Input method has processed key event, id:%{public}" PRId32 ", focusedSessionId:%{public}" PRId32,
            keyEvent->GetId(), focusedSessionId);
        return;
    }

    auto focusedSceneSession = SceneSessionManager::GetInstance().GetSceneSession(focusedSessionId);
    if (focusedSceneSession == nullptr) {
        WLOGFE("focusedSceneSession is null");
        keyEvent->MarkProcessed();
        return;
    }

    if (uiContent_ == nullptr) {
        WLOGFE("uiContent_ is null");
        keyEvent->MarkProcessed();
        return;
    }

    SendKeyEventConsumedResultToSCB(keyEvent, focusedSceneSession->SendKeyEventToUI(keyEvent));
}

void IntentionEventManager::InputEventListener::OnInputEvent(std::shared_ptr<MMI::KeyEvent> keyEvent) const
{
    if (keyEvent == nullptr) {
        TLOGE(WmsLogTag::WMS_INPUT_KEY_FLOW, "The key event is nullptr");
        return;
    }
    HITRACE_METER_FMT(HITRACE_TAG_WINDOW_MANAGER, "IEM:KeyEvent id:%d", keyEvent->GetId());
    if (!SceneSessionManager::GetInstance().IsInputEventEnabled()) {
        TLOGD(WmsLogTag::WMS_INPUT_KEY_FLOW, "OnInputEvent is disabled temporarily");
        keyEvent->MarkProcessed();
        return;
    }
    auto focusedSessionId = SceneSessionManager::GetInstance().GetFocusedSessionId();
    if (focusedSessionId == INVALID_SESSION_ID) {
        TLOGE(WmsLogTag::WMS_INPUT_KEY_FLOW, "focusedSessionId is invalid");
        keyEvent->MarkProcessed();
        return;
    }
    auto focusedSceneSession = SceneSessionManager::GetInstance().GetSceneSession(focusedSessionId);
    if (focusedSceneSession == nullptr) {
        TLOGE(WmsLogTag::WMS_INPUT_KEY_FLOW, "focusedSceneSession is null");
        keyEvent->MarkProcessed();
        return;
    }
    auto isSystem = focusedSceneSession->GetSessionInfo().isSystem_;
    static uint32_t eventId = 0;
    TLOGI(WmsLogTag::WMS_INPUT_KEY_FLOW, "eid:%{public}d,InputId:%{public}d,wid:%{public}u"
        ",fid:%{public}d,sys:%{public}d",
        eventId++, keyEvent->GetId(), keyEvent->GetTargetWindowId(), focusedSessionId, isSystem);
    if (!isSystem) {
        WSError ret = focusedSceneSession->TransferKeyEvent(keyEvent);
        if ((ret != WSError::WS_OK || static_cast<int32_t>(getprocpid()) != focusedSceneSession->GetCallingPid()) &&
            keyEvent != nullptr) {
            keyEvent->MarkProcessed();
        }
        return;
    }
    bool isConsumed = focusedSceneSession->SendKeyEventToUI(keyEvent, true);
    if (isConsumed) {
        SendKeyEventConsumedResultToSCB(keyEvent, isConsumed);
        TLOGI(WmsLogTag::WMS_INPUT_KEY_FLOW, "SendToUI id:%{public}d isConsumed:%{public}d",
            keyEvent->GetId(), static_cast<int>(isConsumed));
        return;
    }
#ifdef IMF_ENABLE
    bool isKeyboardEvent = IsKeyboardEvent(keyEvent);
    if (isKeyboardEvent) {
        WLOGD("Async dispatch keyEvent to input method");
        auto callback = [this, focusedSessionId] (std::shared_ptr<MMI::KeyEvent>& keyEvent, bool consumed) {
            this->DispatchKeyEventCallback(focusedSessionId, keyEvent, consumed);
        };
        auto ret = MiscServices::InputMethodController::GetInstance()->DispatchKeyEvent(keyEvent, callback);
        if (ret != 0) {
            WLOGFE("DispatchKeyEvent failed, ret:%{public}d, id:%{public}d, focusedSessionId:%{public}d",
                ret, keyEvent->GetId(), focusedSessionId);
            DispatchKeyEventCallback(focusedSessionId, keyEvent, false);
        }
        return;
    }
#endif // IMF_ENABLE
    TLOGD(WmsLogTag::WMS_INPUT_KEY_FLOW, "Syetem window scene, transfer key event to root scene");
    if (uiContent_ == nullptr) {
        TLOGE(WmsLogTag::WMS_INPUT_KEY_FLOW, "uiContent_ is null");
        keyEvent->MarkProcessed();
        return;
    }
    SendKeyEventConsumedResultToSCB(keyEvent, focusedSceneSession->SendKeyEventToUI(keyEvent));
}

bool IntentionEventManager::InputEventListener::IsKeyboardEvent(
    const std::shared_ptr<MMI::KeyEvent>& keyEvent) const
{
    int32_t keyCode = keyEvent->GetKeyCode();
    bool isKeyFN = (keyCode == MMI::KeyEvent::KEYCODE_FN);
    bool isKeyBack = (keyCode == MMI::KeyEvent::KEYCODE_BACK);
    bool isKeyboard = (keyCode >= MMI::KeyEvent::KEYCODE_0 && keyCode <= MMI::KeyEvent::KEYCODE_NUMPAD_RIGHT_PAREN);
    bool isKeySound = (keyCode == MMI::KeyEvent::KEYCODE_SOUND);
    TLOGD(WmsLogTag::WMS_EVENT, "isKeyFN:%{public}d, isKeyboard:%{public}d", isKeyFN, isKeyboard);
    return (isKeyFN || isKeyboard || isKeyBack || isKeySound);
}

void IntentionEventManager::InputEventListener::OnInputEvent(
    std::shared_ptr<MMI::AxisEvent> axisEvent) const
{
    if (axisEvent == nullptr) {
        TLOGE(WmsLogTag::WMS_INPUT_KEY_FLOW, "axisEvent is nullptr");
        return;
    }
    if (uiContent_ == nullptr) {
        TLOGE(WmsLogTag::WMS_INPUT_KEY_FLOW, "uiContent_ is null");
        axisEvent->MarkProcessed();
        return;
    }
    if (!(uiContent_->ProcessAxisEvent(axisEvent))) {
        TLOGE(WmsLogTag::WMS_INPUT_KEY_FLOW, "The UI content consumes the axis event failed.");
        axisEvent->MarkProcessed();
    }
}

bool CreateAndEnableInputEventListener(Ace::UIContent* uiContent,
    std::shared_ptr<AppExecFwk::EventHandler> eventHandler, wptr<Window> window)
{
    TLOGD(WmsLogTag::WMS_EVENT, "in");
    return DelayedSingleton<IntentionEventManager>::GetInstance()->EnableInputEventListener(
        uiContent, eventHandler, window);
}
}
} // namespace OHOS::Rosen
