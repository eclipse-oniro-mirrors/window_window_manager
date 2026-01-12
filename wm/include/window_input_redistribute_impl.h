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

#ifndef OHOS_ROSEN_WINDOW_INPUT_REDISTRIBUTE_IMPL_H
#define OHOS_ROSEN_WINDOW_INPUT_REDISTRIBUTE_IMPL_H

#include <memory>
#include <mutex>
#include "input_manager.h"
#include "window_input_redistribute_client.h"
#include "wm_single_instance.h"

namespace OHOS {
namespace Rosen {

struct RecipientResources {
    std::vector<IInputEventRecipientInfo>& recipientVector;
    std::mutex& mtx;
};

class WindowInputRedistributeImpl : public RefBase {
    WM_DECLARE_SINGLE_INSTANCE_BASE(WindowInputRedistributeImpl);
public:
    void RegisterInputEventRedistribute(const IInputEventRecipientInfo& recipientInfo);
    void UnRegisterInputEventRedistribute(const IInputEventRecipientInfo& recipientInfo);
    bool SendEvent(InputRedistributeTiming timing, const std::shared_ptr<MMI::KeyEvent>& keyEvent);
    bool SendEvent(InputRedistributeTiming timing, const std::shared_ptr<MMI::PointerEvent>& pointerEvent);

private:
    void InsertRecipientCallback(const IInputEventRecipientInfo& recipientInfo);
    void RemoveRecipientCallback(const IInputEventRecipientInfo& recipientInfo);
    std::optional<RecipientResources> GetRecipientResources(InputEventType type,
        InputRedistributeTiming timing);

    std::vector<IInputEventRecipientInfo> recipientKeyBeforeComponentArray_;
    std::mutex mutexKeyBeforeComponent_;
    std::vector<IInputEventRecipientInfo> recipientKeyAfterComponentArray_;
    std::mutex mutexKeyAfterComponent_;

    std::vector<IInputEventRecipientInfo> recipientPointerBeforeComponentArray_;
    std::mutex mutexPointerBeforeComponent_;
    std::vector<IInputEventRecipientInfo> recipientPointerAfterComponentArray_;
    std::mutex mutexPointerAfterComponent_;

    using RecipientEntry = std::pair<std::reference_wrapper<std::vector<IInputEventRecipientInfo>>,
                                     std::reference_wrapper<std::mutex>>;
    std::map<std::pair<InputEventType, InputRedistributeTiming>, RecipientEntry> recipientMap_;

    WindowInputRedistributeImpl()
        : recipientMap_({
            {{InputEventType::KEY_EVENT, InputRedistributeTiming::REDISTRIBUTE_BEFORE_SEND_TO_COMPONENT},
             {std::ref(recipientKeyBeforeComponentArray_), std::ref(mutexKeyBeforeComponent_)}},
            {{InputEventType::KEY_EVENT, InputRedistributeTiming::REDISTRIBUTE_AFTER_SEND_TO_COMPONENT},
             {std::ref(recipientKeyAfterComponentArray_), std::ref(mutexKeyAfterComponent_)}},
            {{InputEventType::POINTER_EVENT, InputRedistributeTiming::REDISTRIBUTE_BEFORE_SEND_TO_COMPONENT},
             {std::ref(recipientPointerBeforeComponentArray_), std::ref(mutexPointerBeforeComponent_)}},
            {{InputEventType::POINTER_EVENT, InputRedistributeTiming::REDISTRIBUTE_AFTER_SEND_TO_COMPONENT},
             {std::ref(recipientPointerAfterComponentArray_), std::ref(mutexPointerAfterComponent_)}}
        }) {}
};
}
}

#endif //OHOS_ROSEN_WINDOW_INPUT_REDISTRIBUTE_IMPL_H
