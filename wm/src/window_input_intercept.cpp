/*
 * Copyright (c) 2025-2025 Huawei Device Co., Ltd.
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

#include "window_input_intercept.h"
#include "window_input_intercept_consumer.h"
#include "window_manager_hilog.h"

namespace OHOS {
namespace Rosen {

WM_IMPLEMENT_SINGLE_INSTANCE(WindowInputIntercept)
void WindowInputIntercept::RegisterInputEventIntercept(const int32_t deviceId,
                                                       const std::shared_ptr<IInputEventInterceptConsumer>& consumer)
{
    if (consumer == nullptr) {
        TLOGE(WmsLogTag::WMS_EVENT, "GameController call RegisterInputEventIntercept failed. the consumer is nullptr");
        return;
    }
    std::lock_guard<std::mutex> lock(registerMutex_);
    auto iter = inputIntercept_.find(deviceId);
    if (iter == inputIntercept_.end()) {
        TLOGI(WmsLogTag::WMS_EVENT, "GameController call RegisterInputEventIntercept success."
                                    " the deviceId is %{public}d", deviceId);
        inputIntercept_.insert(std::make_pair(deviceId, consumer));
    }
}

void WindowInputIntercept::UnRegisterInputEventIntercept(const int32_t deviceId)
{
    std::lock_guard<std::mutex> lock(registerMutex_);
    auto iter = inputIntercept_.find(deviceId);
    if (iter != inputIntercept_.end()) {
        TLOGI(WmsLogTag::WMS_EVENT, "GameController call UnRegisterInputEventIntercept success."
                                    " the deviceId is %{public}d", deviceId);
        inputIntercept_.erase(deviceId);
    }
}


bool WindowInputIntercept::IsInputIntercept(const std::shared_ptr<MMI::KeyEvent>& keyEvent)
{
    if (keyEvent == nullptr) {
        TLOGW(WmsLogTag::WMS_EVENT, "IsInputInterceptByKeyEvent keyEvent is null.");
        return false;
    }
    std::shared_ptr<IInputEventInterceptConsumer> consumer = nullptr;
    {
        std::lock_guard<std::mutex> lock(registerMutex_);
        auto iter = inputIntercept_.find(keyEvent->GetDeviceId());
        if (iter == inputIntercept_.end()) {
            return false;
        }
        consumer = iter->second;
    }
    if (consumer == nullptr) {
        TLOGW(WmsLogTag::WMS_EVENT, "IsInputInterceptByKeyEvent consumer is null. the deviceId is %{public}d",
            keyEvent->GetDeviceId());
        return false;
    }

    // Transfer the KeyEvent to GameController
    consumer->OnInputEvent(keyEvent);
    return true;
}

bool WindowInputIntercept::IsInputIntercept(const std::shared_ptr<MMI::PointerEvent>& pointerEvent)
{
    if (pointerEvent == nullptr) {
        TLOGW(WmsLogTag::WMS_EVENT, "IsInputInterceptByPointerEvent pointerEvent is null.");
        return false;
    }
    // PC game controller request to intercept TOUCHSCREEN event.
    if (pointerEvent->GetSourceType() == MMI::PointerEvent::SOURCE_TYPE_TOUCHSCREEN) {
        TLOGW(WmsLogTag::WMS_EVENT, "Intercept pointer event");
        return false;
    }

    std::shared_ptr<IInputEventInterceptConsumer> consumer = nullptr;
    {
        std::lock_guard<std::mutex> lock(registerMutex_);
        auto iter = inputIntercept_.find(pointerEvent->GetDeviceId());
        if (iter == inputIntercept_.end()) {
            return false;
        }
        consumer = iter->second;
    }
    if (consumer == nullptr) {
        TLOGW(WmsLogTag::WMS_EVENT, "IsInputInterceptByPointerEvent consumer is null. the deviceId is %{public}d",
            pointerEvent->GetDeviceId());
        return false;
    }

    // Transfer the PointerEvent to GameController
    consumer->OnInputEvent(pointerEvent);
    return true;
}
}
}