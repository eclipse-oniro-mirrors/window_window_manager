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

#include "window_input_redistribute_impl.h"
#include "window_manager_hilog.h"

namespace OHOS {
namespace Rosen {
WM_IMPLEMENT_SINGLE_INSTANCE(WindowInputRedistributeImpl)

std::optional<RecipientResources> WindowInputRedistributeImpl::GetRecipientResources(InputEventType type,
    InputRedistributeTiming timing)
{
    auto it = recipientMap_.find(std::make_pair(type, timing));
    if (it == recipientMap_.end()) {
        TLOGE(WmsLogTag::WMS_EVENT, "find recipient failed for type %{public}d and timing %{public}d",
            static_cast<uint32_t>(type), static_cast<uint32_t>(timing));
        return std::nullopt;
    }

    const auto& [recipientVectorRef, mutexRef] = it->second;
    return RecipientResources{recipientVectorRef.get(), mutexRef.get()};
}

static void insertSorted(std::vector<IInputEventRecipientInfo>& vec, const IInputEventRecipientInfo& recipientInfo)
{
    auto it = std::find_if(vec.begin(), vec.end(), [&recipientInfo](const IInputEventRecipientInfo& info) {
        return info.identity == recipientInfo.identity;
    });
    if (it != vec.end()) {
        *it = recipientInfo;
        return;
    }

    auto insertPos = std::lower_bound(vec.begin(), vec.end(), recipientInfo,
    [](const IInputEventRecipientInfo& a, const IInputEventRecipientInfo& b) {
        return a.priority > b.priority;
    });
    vec.insert(insertPos, recipientInfo);
}

void WindowInputRedistributeImpl::InsertRecipientCallback(const IInputEventRecipientInfo& recipientInfo)
{
    auto resources = GetRecipientResources(recipientInfo.type, recipientInfo.timing);
    if (!resources) {
        return;
    }

    auto& [recipientComponentArray, mtx] = *resources;
    std::lock_guard<std::mutex> lock(mtx);
    insertSorted(recipientComponentArray, recipientInfo);
}

void WindowInputRedistributeImpl::RegisterInputEventRedistribute(const IInputEventRecipientInfo& recipientInfo)
{
    if (recipientInfo.recipient == nullptr || recipientInfo.identity >= InputRedistributeIdentity::IDENTITY_INVALID ||
        recipientInfo.timing >= InputRedistributeTiming::REDISTRIBUTE_INVALID_TIMING) {
        TLOGE(WmsLogTag::WMS_EVENT, "register recipient:%{public}d at invalid timing %{public}d",
            static_cast<uint32_t>(recipientInfo.identity), static_cast<uint32_t>(recipientInfo.timing));
        return;
    }
    TLOGI(WmsLogTag::WMS_EVENT, "register identity:%{public}d,timing:%{public}d",
        static_cast<uint32_t>(recipientInfo.identity), static_cast<uint32_t>(recipientInfo.timing));
    InsertRecipientCallback(recipientInfo);
}

void WindowInputRedistributeImpl::RemoveRecipientCallback(const IInputEventRecipientInfo& recipientInfo)
{
    auto resources = GetRecipientResources(recipientInfo.type, recipientInfo.timing);
    if (!resources) {
        return;
    }
    auto& [recipientComponentArray, mtx] = *resources;
    std::lock_guard<std::mutex> lock(mtx);

    auto identity = recipientInfo.identity;
    auto iter = std::remove_if(recipientComponentArray.begin(), recipientComponentArray.end(),
        [identity](const IInputEventRecipientInfo& info) {
            return info.identity == identity;
        });
    recipientComponentArray.erase(iter, recipientComponentArray.end());
}

void WindowInputRedistributeImpl::UnRegisterInputEventRedistribute(const IInputEventRecipientInfo& recipientInfo)
{
    if (recipientInfo.identity >= InputRedistributeIdentity::IDENTITY_INVALID ||
        recipientInfo.timing >= InputRedistributeTiming::REDISTRIBUTE_INVALID_TIMING) {
        TLOGE(WmsLogTag::WMS_EVENT, "unRegister recipient:%{public}d at invalid timing %{public}d",
            static_cast<uint32_t>(recipientInfo.identity), static_cast<uint32_t>(recipientInfo.timing));
        return;
    }
    TLOGI(WmsLogTag::WMS_EVENT, "unregister identity:%{public}d,timing:%{public}d",
        static_cast<uint32_t>(recipientInfo.identity), static_cast<uint32_t>(recipientInfo.timing));
    RemoveRecipientCallback(recipientInfo);
}

static void LogEvent(const std::shared_ptr<MMI::KeyEvent>& keyEvent, const InputRedistributeIdentity& identity)
{
    TLOGI(WmsLogTag::WMS_EVENT, "keyId:%{public}d,recipient:%{public}d",
        keyEvent->GetId(), static_cast<uint32_t>(identity));
}

static void LogEvent(const std::shared_ptr<MMI::PointerEvent>& pointerEvent, const InputRedistributeIdentity& identity)
{
    int32_t action = pointerEvent->GetPointerAction();
    if (action != MMI::PointerEvent::POINTER_ACTION_MOVE &&
        action != MMI::PointerEvent::POINTER_ACTION_PULL_MOVE &&
        action != MMI::PointerEvent::POINTER_ACTION_AXIS_UPDATE) {
        TLOGI(WmsLogTag::WMS_EVENT, "pointerId:%{public}d,recipient:%{public}d",
            pointerEvent->GetId(), static_cast<uint32_t>(identity));
    } else {
        TLOGD(WmsLogTag::WMS_EVENT, "pointerId:%{public}d,recipient:%{public}d",
            pointerEvent->GetId(), static_cast<uint32_t>(identity));
    }
}

template <typename EventType>
typename std::enable_if<
    std::is_same<EventType, MMI::KeyEvent>::value ||
    std::is_same<EventType, MMI::PointerEvent>::value, bool>::type processSendEvent(std::mutex& mtx,
    std::vector<IInputEventRecipientInfo>& recipientArray, const std::shared_ptr<EventType>& event)
{
    if (event == nullptr) {
        TLOGE(WmsLogTag::WMS_EVENT, "send event failed, event is nullptr");
        return false;
    }
    std::lock_guard<std::mutex> lock(mtx);
    for (const auto& recipientInfo : recipientArray) {
        std::shared_ptr<IInputEventRecipientCallback> recipient = recipientInfo.recipient;
        if (recipient == nullptr) {
            continue;
        }
        if (recipient->OnInputEvent(event) == InputAfterRedistributeBehavior::BEHAVIOR_INTERCEPT) {
            LogEvent(event, recipientInfo.identity);
            event->MarkProcessed();
            return true;
        }
    }
    return false;
}

bool WindowInputRedistributeImpl::SendEvent(
    InputRedistributeTiming timing, const std::shared_ptr<MMI::PointerEvent>& pointerEvent)
{
    if (timing == InputRedistributeTiming::REDISTRIBUTE_BEFORE_SEND_TO_COMPONENT) {
        return processSendEvent<MMI::PointerEvent>(mutexPointerBeforeComponent_,
            recipientPointerBeforeComponentArray_, pointerEvent);
    } else if (timing == InputRedistributeTiming::REDISTRIBUTE_AFTER_SEND_TO_COMPONENT) {
        return processSendEvent<MMI::PointerEvent>(mutexPointerAfterComponent_,
            recipientPointerAfterComponentArray_, pointerEvent);
    }
    TLOGE(WmsLogTag::WMS_EVENT, "send event failed, wrong timing");
    return false;
}

bool WindowInputRedistributeImpl::SendEvent(
    InputRedistributeTiming timing, const std::shared_ptr<MMI::KeyEvent>& keyEvent)
{
    if (timing == InputRedistributeTiming::REDISTRIBUTE_BEFORE_SEND_TO_COMPONENT) {
        return processSendEvent<MMI::KeyEvent>(mutexKeyBeforeComponent_,
            recipientKeyBeforeComponentArray_, keyEvent);
    } else if (timing == InputRedistributeTiming::REDISTRIBUTE_AFTER_SEND_TO_COMPONENT) {
        return processSendEvent<MMI::KeyEvent>(mutexKeyAfterComponent_,
            recipientKeyAfterComponentArray_, keyEvent);
    }

    TLOGE(WmsLogTag::WMS_EVENT, "send event failed, wrong timing");
    return false;
}
}
}