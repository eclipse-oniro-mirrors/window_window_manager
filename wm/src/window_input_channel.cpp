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

#include "window_input_channel.h"
#ifdef IMF_ENABLE
#include <input_method_controller.h>
#endif // IMF_ENABLE
#include "window_manager_hilog.h"
#include "window_helper.h"

namespace OHOS {
namespace Rosen {
namespace {
constexpr HiviewDFX::HiLogLabel LABEL = {LOG_CORE, HILOG_DOMAIN_WINDOW, "InputChannel"};
}
WindowInputChannel::WindowInputChannel(const sptr<Window>& window): window_(window), isAvailable_(true)
{
}

WindowInputChannel::~WindowInputChannel()
{
    WLOGI("windowName: %{public}s, windowId: %{public}d", window_->GetWindowName().c_str(), window_->GetWindowId());
    window_->SetNeedRemoveWindowInputChannel(false);
}

void WindowInputChannel::DispatchKeyEventCallback(std::shared_ptr<MMI::KeyEvent>& keyEvent, bool consumed)
{
    if (keyEvent == nullptr) {
        WLOGFW("keyEvent is null");
        return;
    }

    if (consumed) {
        WLOGD("Input method has processed key event, id:%{public}d", keyEvent->GetId());
        return;
    }

    if (window_ != nullptr) {
        WLOGD("dispatch keyEvent to ACE");
        window_->ConsumeKeyEvent(keyEvent);
    } else {
        keyEvent->MarkProcessed();
    }
}

__attribute__((no_sanitize("cfi"))) void WindowInputChannel::HandleKeyEvent(
    std::shared_ptr<MMI::KeyEvent>& keyEvent)
{
    if (keyEvent == nullptr) {
        WLOGFE("keyEvent is nullptr");
        return;
    }

    if (window_ == nullptr) {
        TLOGE(WmsLogTag::WMS_EVENT, "window_ is nullptr");
        return;
    }
    WLOGFD("Receive key event, Id: %{public}u", window_->GetWindowId());
    if (window_->GetType() == WindowType::WINDOW_TYPE_DIALOG) {
        if (keyEvent->GetAgentWindowId() != keyEvent->GetTargetWindowId()) {
            window_->NotifyTouchDialogTarget();
            keyEvent->MarkProcessed();
            return;
        }
        if (keyEvent->GetKeyCode() == MMI::KeyEvent::KEYCODE_BACK) {
            if (keyEvent->GetKeyAction() == MMI::KeyEvent::KEY_ACTION_UP &&
                window_->IsDialogSessionBackGestureEnabled()) {
                window_->ConsumeBackEvent();
            }
            keyEvent->MarkProcessed();
            return;
        }
    }
    bool isConsumed = window_->PreNotifyKeyEvent(keyEvent);
    if (isConsumed) {
        TLOGI(WmsLogTag::WMS_EVENT, "PreNotifyKeyEvent id:%{public}d isConsumed:%{public}d",
            keyEvent->GetId(), static_cast<int>(isConsumed));
        return;
    }
#ifdef IMF_ENABLE
    bool isKeyboardEvent = IsKeyboardEvent(keyEvent);
    if (isKeyboardEvent) {
        WLOGD("Async dispatch keyEvent to input method");
        auto callback = [weakThis = wptr(this)] (std::shared_ptr<MMI::KeyEvent>& keyEvent, bool consumed) {
            auto promoteThis = weakThis.promote();
            if (promoteThis == nullptr) {
                keyEvent->MarkProcessed();
                WLOGFW("promoteThis is nullptr");
                return;
            }
            promoteThis->DispatchKeyEventCallback(keyEvent, consumed);
        };
        auto ret = MiscServices::InputMethodController::GetInstance()->DispatchKeyEvent(keyEvent, callback);
        if (ret != 0) {
            TLOGI(WmsLogTag::WMS_EVENT, "DispatchKeyEvent ret:%{public}d, id:%{public}d", ret, keyEvent->GetId());
            DispatchKeyEventCallback(keyEvent, false);
        }
        return;
    }
#endif // IMF_ENABLE
    WLOGD("dispatch keyEvent to ACE");
    window_->ConsumeKeyEvent(keyEvent);
}

void WindowInputChannel::ProcAncoEvent(std::shared_ptr<MMI::PointerEvent>& pointerEvent)
{
    auto action = pointerEvent->GetPointerAction();
    bool isPointDown = ((action == MMI::PointerEvent::POINTER_ACTION_DOWN) ||
        (action == MMI::PointerEvent::POINTER_ACTION_BUTTON_DOWN));
    if (!isPointDown) {
        return;
    }

    MMI::PointerEvent::PointerItem pointerItem;
    bool ret = pointerEvent->GetPointerItem(pointerEvent->GetPointerId(), pointerItem);
    if (!ret) {
        TLOGE(WmsLogTag::WMS_EVENT, "get item failed, %{public}d", pointerEvent->GetId());
        return;
    }
    
    auto x = pointerItem.GetDisplayX();
    auto y = pointerItem.GetDisplayY();
    window_->OnPointDown(pointerEvent->GetId(), x, y);
}

void WindowInputChannel::HandlePointerEvent(std::shared_ptr<MMI::PointerEvent>& pointerEvent)
{
    if (pointerEvent == nullptr) {
        TLOGE(WmsLogTag::WMS_EVENT, "pointerEvent is nullptr");
        return;
    }
    if (window_ == nullptr) {
        TLOGE(WmsLogTag::WMS_EVENT, "window_ is nullptr, id:%{public}d", pointerEvent->GetId());
        return;
    }

    bool isHitHotArea = window_->IsHitTitleBar(pointerEvent) || window_->IsHitHotAreas(pointerEvent);
    if (window_->IsAnco() && !isHitHotArea) {
        ProcAncoEvent(pointerEvent);
        pointerEvent->MarkProcessed();
        return;
    }
    
    auto action = pointerEvent->GetPointerAction();
    TLOGD(WmsLogTag::WMS_EVENT, "Receive pointer event, Id: %{public}u, action: %{public}d",
        window_->GetWindowId(), action);

    bool isPointDown = action == MMI::PointerEvent::POINTER_ACTION_DOWN ||
        action == MMI::PointerEvent::POINTER_ACTION_BUTTON_DOWN;
    MMI::PointerEvent::PointerItem pointerItem;
    bool isValidPointItem = pointerEvent->GetPointerItem(pointerEvent->GetPointerId(), pointerItem);
    if ((window_->GetType() == WindowType::WINDOW_TYPE_DIALOG ||
         WindowHelper::IsModalSubWindow(window_->GetType(), window_->GetWindowFlags()) ||
         WindowHelper::IsModalMainWindow(window_->GetType(), window_->GetWindowFlags())) &&
        pointerEvent->GetAgentWindowId() != pointerEvent->GetTargetWindowId() &&
        action != MMI::PointerEvent::POINTER_ACTION_PULL_UP) {
        if (isPointDown && isValidPointItem) {
            window_->NotifyTouchDialogTarget(pointerItem.GetDisplayX(), pointerItem.GetDisplayY());
        }
        pointerEvent->MarkProcessed();
        return;
    }
    TLOGD(WmsLogTag::WMS_EVENT, "Dispatch move event, windowId: %{public}u, action: %{public}d",
        window_->GetWindowId(), action);
    window_->ConsumePointerEvent(pointerEvent);
    window_->SetTouchEvent(action);
}

void WindowInputChannel::InjectTouchEvent(const std::shared_ptr<MMI::PointerEvent>& pointerEvent)
{
    if (pointerEvent == nullptr) {
        TLOGE(WmsLogTag::WMS_EVENT, "pointerEvent is nullptr at InjectTouchEvent");
        return;
    }
    if (window_ == nullptr) {
        TLOGE(WmsLogTag::WMS_EVENT, "window_ is nullptr, id:%{public}d", pointerEvent->GetId());
        return;
    }
    window_->InjectTouchEvent(pointerEvent);
}

void WindowInputChannel::Destroy()
{
    std::lock_guard<std::mutex> lock(mtx_);
    WLOGI("Destroy WindowInputChannel, windowId:%{public}u", window_->GetWindowId());
    isAvailable_ = false;
}

Rect WindowInputChannel::GetWindowRect()
{
    if (window_ == nullptr) {
        return { 0, 0, 0, 0 };
    }
    return window_->GetRect();
}

bool WindowInputChannel::IsKeyboardEvent(const std::shared_ptr<MMI::KeyEvent>& keyEvent) const
{
    int32_t keyCode = keyEvent->GetKeyCode();
    bool isKeyFN = (keyCode == MMI::KeyEvent::KEYCODE_FN);
    bool isKeyBack = (keyCode == MMI::KeyEvent::KEYCODE_BACK);
    bool isKeyboard = (keyCode >= MMI::KeyEvent::KEYCODE_0 && keyCode <= MMI::KeyEvent::KEYCODE_NUMPAD_RIGHT_PAREN);
    bool isKeySound = (keyCode == MMI::KeyEvent::KEYCODE_SOUND);
    TLOGD(WmsLogTag::WMS_EVENT, "isKeyFN:%{public}d, isKeyboard:%{public}d", isKeyFN, isKeyboard);
    return (isKeyFN || isKeyboard || isKeyBack || isKeySound);
}
}
}
