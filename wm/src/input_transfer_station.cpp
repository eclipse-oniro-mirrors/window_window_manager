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

#include <dlfcn.h>
#include <thread>
#include <event_handler.h>
#include "window_manager_hilog.h"
#include "wm_common_inner.h"
#include "gtx_input_event_sender.h"
#include <hitrace_meter.h>
#include "window_input_intercept.h"
#include "window_input_redistribute_impl.h"
#include "window_event_subscriber.h"
#include "sys_cap_util.h"

namespace OHOS {
namespace Rosen {
namespace {
constexpr HiviewDFX::HiLogLabel LABEL = {LOG_CORE, HILOG_DOMAIN_WINDOW, "InputTransferStation"};
}

const std::string GAME_CONTROLLER_SO_PATH = "/system/lib64/libgamecontroller_event.z.so";
const std::string TOUCH_PREDICTOR_SO_PATH = "/system/lib64/libtouch_predictor.so";

static std::unique_ptr<void, void(*)(void*)> gameControllerHandle_{nullptr, [](void* handle) {
    if (handle) {
        dlclose(handle);
        TLOGI(WmsLogTag::WMS_EVENT, "[dlclose] GameController unloaded");
    }
}};

static std::unique_ptr<void, void(*)(void*)> touchPredictorHandle_{nullptr, [](void* handle) {
    if (handle) {
        dlclose(handle);
        TLOGI(WmsLogTag::WMS_EVENT, "[dlclose] TouchPredictor unloaded");
    }
}};

WM_IMPLEMENT_SINGLE_INSTANCE(InputTransferStation)

InputTransferStation::~InputTransferStation()
{
    std::lock_guard<std::mutex> lock(mtx_);
    destroyed_ = true;
}

void InputEventListener::HandleInputEvent(std::shared_ptr<MMI::KeyEvent> keyEvent) const
{
    if (keyEvent == nullptr) {
        TLOGE(WmsLogTag::WMS_INPUT_KEY_FLOW, "KeyEvent is nullptr");
        return;
    }
    HITRACE_METER_FMT(HITRACE_TAG_WINDOW_MANAGER, "IEL:KeyEvent id:%d", keyEvent->GetId());
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

void InputEventListener::HandleInputEvent(std::shared_ptr<MMI::PointerEvent> pointerEvent) const
{
    if (pointerEvent == nullptr) {
        TLOGE(WmsLogTag::WMS_INPUT_KEY_FLOW, "PointerEvent is nullptr");
        return;
    }
    HITRACE_METER_FMT(HITRACE_TAG_WINDOW_MANAGER, "IEL:PointerEvent id:%d action:%d",
        pointerEvent->GetId(), pointerEvent->GetPointerAction());
    if (pointerEvent->HasFlag(MMI::InputEvent::EVENT_FLAG_GESTURE_SUPPLEMENT)) {
        MMI::InputManager::GetInstance()->TransformTouchEventToMouseEvent(pointerEvent);
        TLOGD(WmsLogTag::WMS_INPUT_KEY_FLOW, "transfer event %{public}d to mouse event",
            pointerEvent->GetId());
    }
    // If handling input event at server, client will receive pointEvent that the winId is -1, intercept log error
    uint32_t invalidId = static_cast<uint32_t>(-1);
    uint32_t windowId = static_cast<uint32_t>(pointerEvent->GetAgentWindowId());
    int32_t action = pointerEvent->GetPointerAction();
    if (action != MMI::PointerEvent::POINTER_ACTION_MOVE) {
        static uint32_t eventId = 0;
        TLOGI(WmsLogTag::WMS_INPUT_KEY_FLOW, "eid:%{public}d,InputId:%{public}d"
            ",wid:%{public}u,ac:%{public}d", eventId++, pointerEvent->GetId(), windowId,
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
    WindowInputRedistributeImpl::GetInstance().SendEvent(
        InputRedistributeTiming::REDISTRIBUTE_AFTER_SEND_TO_COMPONENT, pointerEvent);
    GtxInputEventSender::GetInstance().SetTouchEvent(channel->GetWindowRect(), pointerEvent);
}

void InputEventListener::OnInputEvent(std::shared_ptr<MMI::KeyEvent> keyEvent) const
{
    if (WindowInputIntercept::GetInstance().IsInputIntercept(keyEvent)) {
        return;
    }
    if (WindowInputRedistributeImpl::GetInstance().SendEvent(
        InputRedistributeTiming::REDISTRIBUTE_BEFORE_SEND_TO_COMPONENT, keyEvent)) {
        return;
    }
    HandleInputEvent(keyEvent);
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
    if (WindowInputIntercept::GetInstance().IsInputIntercept(pointerEvent)) {
        return;
    }
    if (WindowInputRedistributeImpl::GetInstance().SendEvent(
        InputRedistributeTiming::REDISTRIBUTE_BEFORE_SEND_TO_COMPONENT, pointerEvent)) {
        return;
    }
    HandleInputEvent(pointerEvent);
}

void InputTransferStation::LoadTouchPredictor()
{
    if (isTouchPredictorLoaded_) {
        return;
    }
    TLOGI(WmsLogTag::WMS_EVENT, "in");
    isTouchPredictorLoaded_ = true;
    void* handle = dlopen(TOUCH_PREDICTOR_SO_PATH.c_str(), RTLD_LAZY | RTLD_GLOBAL);
    if (handle) {
        TLOGI(WmsLogTag::WMS_EVENT, "dlopen TouchPredictor success");
        touchPredictorHandle_.reset(handle);
    } else {
        TLOGW(WmsLogTag::WMS_EVENT, "dlopen %{public}s", dlerror());
    }
}

void InputTransferStation::LoadGameController()
{
    TLOGI(WmsLogTag::WMS_EVENT, "in");
    isGameControllerLoaded_ = true;
    void* handle = dlopen(GAME_CONTROLLER_SO_PATH.c_str(), RTLD_LAZY | RTLD_GLOBAL);
    if (handle) {
        TLOGI(WmsLogTag::WMS_EVENT, "dlopen GameController success");
        gameControllerHandle_.reset(handle);
    } else {
        TLOGW(WmsLogTag::WMS_EVENT, "dlopen %{public}s failed", dlerror());
    }
}

void InputTransferStation::AddInputWindow(const sptr<Window>& window)
{
    uint32_t windowId = window->GetWindowId();
    TLOGD(WmsLogTag::WMS_EVENT, "Add input window, windowId: %{public}u", windowId);

    // INPUT_WINDOW_TYPE_SKIPPED should not set input consumer
    if (INPUT_WINDOW_TYPE_SKIPPED.find(window->GetType()) != INPUT_WINDOW_TYPE_SKIPPED.end()) {
        TLOGW(WmsLogTag::WMS_EVENT, "skip window for InputConsumer [id:%{public}u, type:%{public}d]",
            windowId, window->GetType());
        return;
    }

    if (windowInputChannels_.count(windowId) == 0) {
        sptr<WindowInputChannel> inputChannel = new WindowInputChannel(window);
        std::lock_guard<std::mutex> lock(mtx_);
        if (destroyed_) {
            TLOGW(WmsLogTag::WMS_EVENT, "Already destroyed");
            return;
        }
        windowInputChannels_.insert(std::make_pair(windowId, inputChannel));
    }

    if (inputListener_ == nullptr) {
        TLOGD(WmsLogTag::WMS_EVENT, "Init input listener, IsMainHandlerAvailable: %{public}u",
            window->IsMainHandlerAvailable());
        std::shared_ptr<InputEventListener> listener = std::make_shared<InputEventListener>();
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
        TLOGI(WmsLogTag::WMS_EVENT, "SetWindowInputEventConsumer success, wid:%{public}u", windowId);
        inputListener_ = listener;
    } else {
        auto ret = MMI::InputManager::GetInstance()->SetWindowInputEventConsumer(inputListener_, eventHandler_);
        TLOGI(WmsLogTag::WMS_EVENT, "SetWindowInputEventConsumer %{public}u, wid:%{public}u", ret, windowId);
    }
    if (!isGameControllerLoaded_) {
        LoadGameController();
    }
    WindowEventSubscribeProxy::GetInstance()->SetBundleName(SysCapUtil::GetBundleName());
    WindowEventSubscribeProxy::GetInstance()->SubscribeEvent();
}

void InputTransferStation::RemoveInputWindow(uint32_t windowId)
{
    TLOGI(WmsLogTag::WMS_EVENT, "wid: %{public}u", windowId);
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

void InputTransferStation::InjectTouchEvent(const std::shared_ptr<MMI::PointerEvent>& pointerEvent)
{
    if (pointerEvent == nullptr) {
        TLOGE(WmsLogTag::WMS_INPUT_KEY_FLOW, "PointerEvent is nullptr");
        return;
    }
    uint32_t invalidId = static_cast<uint32_t>(-1);
    uint32_t windowId = static_cast<uint32_t>(pointerEvent->GetAgentWindowId());
    auto channel = GetInputChannel(windowId);
    if (channel == nullptr) {
        if (windowId != invalidId) {
            TLOGE(WmsLogTag::WMS_INPUT_KEY_FLOW, "WindowInputChannel is nullptr InputTracking id:%{public}d "
                "windowId:%{public}u",
                pointerEvent->GetId(), windowId);
        }
        pointerEvent->MarkProcessed();
        return;
    }
    channel->InjectTouchEvent(pointerEvent);
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

void InputTransferStation::HandleInputEvent(const std::shared_ptr<MMI::KeyEvent>& keyEvent)
{
    if (inputListener_ == nullptr) {
        return;
    }
    inputListener_->HandleInputEvent(keyEvent);
}

void InputTransferStation::HandleInputEvent(const std::shared_ptr<MMI::PointerEvent>& pointerEvent)
{
    if (inputListener_ == nullptr) {
        return;
    }
    inputListener_->HandleInputEvent(pointerEvent);
}
}
}
