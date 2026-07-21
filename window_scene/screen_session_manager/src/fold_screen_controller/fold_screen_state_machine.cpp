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

#include "fold_screen_controller/fold_screen_state_machine.h"

#include "window_manager_hilog.h"

namespace OHOS {
namespace Rosen {
FoldScreenStateMachine::FoldScreenStateMachine() = default;

FoldScreenStateMachine::~FoldScreenStateMachine() = default;

void FoldScreenStateMachine::RegistrationTransitionCallback(const std::shared_ptr<TransitionCallback>& callback)
{
    if (!callback) {
        return;
    }
    std::lock_guard<std::recursive_mutex> lock(mutex_);
    for (auto it = callbacks_.begin(); it != callbacks_.end();) {
        if (*it == callback) {
            return;
        } else {
            ++it;
        }
    }
    callbacks_.push_back(callback);
}

void FoldScreenStateMachine::UnRegistrationTransitionCallback(const std::shared_ptr<TransitionCallback>& callback)
{
    if (!callback) {
        return;
    }
    std::lock_guard<std::recursive_mutex> lock(mutex_);
    for (auto it = callbacks_.begin(); it != callbacks_.end();) {
        if (*it == callback) {
            callbacks_.erase(it);
            return;
        } else {
            ++it;
        }
    }
}

void FoldScreenStateMachine::TransitionTo(FoldScreenState state)
{
    std::lock_guard<std::recursive_mutex> lock(mutex_);
    for (const auto &callback : callbacks_) {
        if (callback) {
            callback->OnTransitionEnter(currState_, state);
        }
    }
    auto previous = currState_;
    currState_ = state;
    TLOGI(WmsLogTag::DMS, "state transition from %{public}u to %{public}u",
          static_cast<int32_t>(currState_), static_cast<int32_t>(state));
    for (const auto &callback : callbacks_) {
        if (callback) {
            callback->OnTransitionExit(previous, currState_);
        }
    }
}

FoldScreenState FoldScreenStateMachine::GetCurrentState() const
{
    return currState_;
}

std::string FoldScreenStateMachine::GenStateMachineInfo() const
{
    std::ostringstream oss;
    oss << "callbackCount: " << callbacks_.size()
        << ", currentState: " << static_cast<int32_t>(currState_) << ";";
    std::string info(oss.str());
    return info;
}
} // Rosen
} // OHOS