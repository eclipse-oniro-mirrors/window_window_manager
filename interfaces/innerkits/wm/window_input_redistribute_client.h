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

#ifndef OHOS_ROSEN_WINDOW_INPUT_REDISTRIBUTE_CLIENT_H
#define OHOS_ROSEN_WINDOW_INPUT_REDISTRIBUTE_CLIENT_H

#include "window.h"

namespace OHOS {
namespace Rosen {
constexpr uint32_t DEFAULT_REDISTRIBUTE_PRIORITY = 1000;

/**
 * @enum InputRedistributeIdentity
 *
 * @brief the identity identifier of the input event receiving module.
 */
enum class InputRedistributeIdentity : uint32_t {
    IDENTITY_MEDIA_CONTROLLER = 1000,
    IDENTITY_TOUCH_PREDICTOR = 1001,
    IDENTITY_SENSEFLOW = 1002,
    IDENTITY_INVALID,
};

/**
 * @enum InputRedistributeTiming
 *
 * @brief the timing of event redistribution: before or after the component.
 */
enum class InputRedistributeTiming : uint32_t {
    REDISTRIBUTE_BEFORE_SEND_TO_COMPONENT = 0,
    REDISTRIBUTE_AFTER_SEND_TO_COMPONENT,
    REDISTRIBUTE_INVALID_TIMING,
};

/**
 * @enum InputAfterRedistributeBehavior
 *
 * @brief the module's behavior after receiving input,
 *  if return BEHAVIOR_INTERCEPT, it will not be distribute downwards.
 */
enum class InputAfterRedistributeBehavior : uint32_t {
    BEHAVIOR_NORMAL = 0,
    BEHAVIOR_INTERCEPT,
};

enum class InputEventType : uint32_t {
    KEY_EVENT = 0,
    POINTER_EVENT,
    ALL,
    INVALID,
};

class IInputEventRecipientCallback {
public:
    IInputEventRecipientCallback() = default;
    virtual ~IInputEventRecipientCallback() = default;
    virtual InputAfterRedistributeBehavior OnInputEvent(const std::shared_ptr<MMI::PointerEvent>& pointerEvent) = 0;
    virtual InputAfterRedistributeBehavior OnInputEvent(const std::shared_ptr<MMI::KeyEvent>& keyEvent) = 0;
};

/**
 * Recipient info.
 * @param identity identity identifier.
 * @param timing the timing of event redistribution.
 * @param type the type of input event.
 * @param priority redistribute event priority of the identity.
 * @param recipient recipient of the input event.
 */
struct IInputEventRecipientInfo {
    InputRedistributeIdentity identity = InputRedistributeIdentity::IDENTITY_INVALID;
    InputRedistributeTiming timing = InputRedistributeTiming::REDISTRIBUTE_INVALID_TIMING;
    InputEventType type = InputEventType::INVALID;
    uint32_t priority = DEFAULT_REDISTRIBUTE_PRIORITY;
    std::shared_ptr<IInputEventRecipientCallback> recipient = nullptr;
};

/**
 * @class WindowInputRedistributeClient
 *
 * @brief Used by all module want receive input event.
 */
class WindowInputRedistributeClient {
public:
    /**
     * Register the redistribution of input events.
     * @param recipientInfo recipient info.
     */
    static void RegisterInputEventRedistribute(const IInputEventRecipientInfo& recipientInfo);

    /**
     * Unregister the redistribution of input events.
     * @param recipientInfo recipient info.
     */
    static void UnRegisterInputEventRedistribute(const IInputEventRecipientInfo& recipientInfo);

    /**
     * Send key event.
     * @param keyEvent keyEvent of Multi-Model Input.
     */
    static void SendInputEvent(const std::shared_ptr<MMI::KeyEvent>& keyEvent);

    /**
     * Send pointer event.
     * @param pointerEvent pointerEvent of Multi-Model Input.
     */
    static void SendInputEvent(const std::shared_ptr<MMI::PointerEvent>& pointerEvent);
};
}
}

#endif //OHOS_ROSEN_WINDOW_INPUT_REDISTRIBUTE_CLIENT_H
