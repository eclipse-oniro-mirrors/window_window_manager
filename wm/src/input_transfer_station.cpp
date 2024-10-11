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

#include "input_transfer_station.h"

#include <thread>
#include <event_handler.h>
#include "window_manager_hilog.h"
#include "wm_common_inner.h"
#include "gtx_input_event_sender.h"

namespace OHOS {
namespace Rosen {
namespace {
constexpr HiviewDFX::HiLogLabel LABEL = {LOG_CORE, HILOG_DOMAIN_WINDOW, "InputTransferStation"};
}
WM_IMPLEMENT_SINGLE_INSTANCE(InputTransferStation)

InputTransferStation::~InputTransferStation()
{
    std::lock_guard<std::mutex> lock(mtx_);
    destroyed_ = true;
}

void InputEventListener::OnInputEvent(std::shared_ptr<MMI::KeyEvent> keyEvent) const
{
    if (keyEvent == nullptr) {
        TLOGE(WmsLogTag::WMS_INPUT_KEY_FLOW, "KeyEvent is nullptr");
        return;
    }
    uint32_t windowId = static_cast<uint32_t>(keyEvent->GetAgentWindowId());
    static uint32_t eventId = 0;
    TLOGI(WmsLogTag::WMS_INPUT_KEY_FLOW, "eid:%{public}d,InputId:%{public}d,wid:%{public}u",
        eventId++, keyEvent->GetId(), windowId);
    auto channel = InputTransferStation::GetInstance().GetInputChannel(windowId);
    if (channel == nullptr) {
        keyEvent->MarkProcessed();
        TLOGE(WmsLogTag::WMS_INPUT_KEY_FLOW,
            "WindowInputChannel is nullptr InputTracking id:%{public}d windowId:%{public}u",
            keyEvent->GetId(), windowId);
        return;
    }
    channel->HandleKeyEvent(keyEvent);
}

void InputEventListener::OnInputEvent(std::shared_ptr<MMI::AxisEvent> axisEvent) const
{
    if (axisEvent == nullptr) {
        TLOGE(WmsLogTag::WMS_INPUT_KEY_FLOW, "AxisEvent is nullptr");
        return;
    }
    TLOGD(WmsLogTag::WMS_INPUT_KEY_FLOW, "Receive axisEvent, windowId: %{public}d", axisEvent->GetAgentWindowId());
    axisEvent->MarkProcessed();
}

void InputEventListener::OnInputEvent(std::shared_ptr<MMI::PointerEvent> pointerEvent) const
{
    if (pointerEvent == nullptr) {
        TLOGE(WmsLogTag::WMS_INPUT_KEY_FLOW, "PointerEvent is nullptr");
        return;
    }
    // If handling input event at server, client will receive pointEvent that the winId is -1, intercept log error
    uint32_t invalidId = static_cast<uint32_t>(-1);
    uint32_t windowId = static_cast<uint32_t>(pointerEvent->GetAgentWindowId());
    int32_t action = pointerEvent->GetPointerAction();
    if (action != MMI::PointerEvent::POINTER_ACTION_MOVE) {
        static uint32_t eventId = 0;
        TLOGI(WmsLogTag::WMS_INPUT_KEY_FLOW, "eid:%{public}d,InputId:%{public}d"
            ",wid:%{public}u,action:%{public}d", eventId++, pointerEvent->GetId(), windowId,
            pointerEvent->GetPointerAction());
    }
    auto channel = InputTransferStation::GetInstance().GetInputChannel(windowId);
    if (channel == nullptr) {
        if (windowId != invalidId) {
            TLOGE(WmsLogTag::WMS_INPUT_KEY_FLOW, "WindowInputChannel is nullptr InputTracking id:%{public}d "
                "windowId:%{public}u",
                pointerEvent->GetId(), windowId);
        }
        pointerEvent->MarkProcessed();
        return;
    }
    channel->HandlePointerEvent(pointerEvent);
    GtxInputEventSender::GetInstance().SetTouchEvent(channel->GetWindowRect(), pointerEvent);
}

void InputTransferStation::AddInputWindow(const sptr<Window>& window)
{
    if (IsRegisterToMMI()) {
        return;
    }

    uint32_t windowId = window->GetWindowId();
    TLOGD(WmsLogTag::WMS_EVENT, "Add input window, windowId: %{public}u", windowId);

    // INPUT_WINDOW_TYPE_SKIPPED should not set input consumer
    if (INPUT_WINDOW_TYPE_SKIPPED.find(window->GetType()) != INPUT_WINDOW_TYPE_SKIPPED.end()) {
        TLOGW(WmsLogTag::WMS_EVENT, "skip window for InputConsumer [id:%{public}u, type:%{public}d]",
            windowId, window->GetType());
        return;
    }
    sptr<WindowInputChannel> inputChannel = new WindowInputChannel(window);
    std::lock_guard<std::mutex> lock(mtx_);
    if (destroyed_) {
        TLOGW(WmsLogTag::WMS_EVENT, "Already destroyed");
        return;
    }
    windowInputChannels_.insert(std::make_pair(windowId, inputChannel));
    if (inputListener_ == nullptr) {
        TLOGD(WmsLogTag::WMS_EVENT, "Init input listener, IsMainHandlerAvailable: %{public}u",
            window->IsMainHandlerAvailable());
        std::shared_ptr<MMI::IInputEventConsumer> listener = std::make_shared<InputEventListener>(InputEventListener());
        auto mainEventRunner = AppExecFwk::EventRunner::GetMainEventRunner();
        if (mainEventRunner != nullptr && window->IsMainHandlerAvailable()) {
            TLOGD(WmsLogTag::WMS_EVENT, "MainEventRunner is available");
            eventHandler_ = std::make_shared<AppExecFwk::EventHandler>(mainEventRunner);
        } else {
            TLOGD(WmsLogTag::WMS_EVENT, "MainEventRunner is not available");
            eventHandler_ = AppExecFwk::EventHandler::Current();
            auto curThreadId = std::this_thread::get_id();
            if (!eventHandler_ || (mainEventRunner != nullptr &&
                mainEventRunner->GetThreadId() == *(reinterpret_cast<uint64_t*>(&curThreadId)))) {
                eventHandler_ = std::make_shared<AppExecFwk::EventHandler>(
                    AppExecFwk::EventRunner::Create(INPUT_AND_VSYNC_THREAD));
            }
        }
        MMI::InputManager::GetInstance()->SetWindowInputEventConsumer(listener, eventHandler_);
        TLOGI(WmsLogTag::WMS_EVENT, "SetWindowInputEventConsumer success, windowid:%{public}u", windowId);
        inputListener_ = listener;
    }
}

void InputTransferStation::RemoveInputWindow(uint32_t windowId)
{
    TLOGD(WmsLogTag::WMS_EVENT, "Remove input window, windowId: %{public}u", windowId);
    sptr<WindowInputChannel> inputChannel = nullptr;
    {
        std::lock_guard<std::mutex> lock(mtx_);
        if (destroyed_) {
            WLOGFW("Already destroyed");
            return;
        }
        auto iter = windowInputChannels_.find(windowId);
        if (iter != windowInputChannels_.end()) {
            inputChannel = iter->second;
            windowInputChannels_.erase(windowId);
        }
    }
    if (inputChannel != nullptr) {
        inputChannel->Destroy();
    } else {
        TLOGE(WmsLogTag::WMS_EVENT, "Can not find windowId: %{public}u", windowId);
    }
}

sptr<WindowInputChannel> InputTransferStation::GetInputChannel(uint32_t windowId)
{
    std::lock_guard<std::mutex> lock(mtx_);
    if (destroyed_) {
        TLOGW(WmsLogTag::WMS_EVENT, "Already destroyed");
        return nullptr;
    }
    auto iter = windowInputChannels_.find(windowId);
    if (iter == windowInputChannels_.end()) {
        return nullptr;
    }
    return iter->second;
}
}
}
