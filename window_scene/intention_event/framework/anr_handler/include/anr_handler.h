/*
 * Copyright (c) 2023 Huawei Device Co., Ltd.
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 * http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#ifndef ANR_HANDLER_H
#define ANR_HANDLER_H

#include <memory>
#include <mutex>
#include <unordered_map>

#include "event_handler.h"
#include "nocopyable.h"
#include "singleton.h"
#include "session_stage_interface.h"
namespace OHOS {
namespace Rosen {
class ANRHandler {
    DECLARE_DELAYED_SINGLETON(ANRHandler);

public:
    DISALLOW_COPY_AND_MOVE(ANRHandler);

    void SetSessionStage(int32_t eventId, const wptr<ISessionStage> &sessionStage);
    void HandleEventConsumed(int32_t eventId, int64_t actionTime);
    void ClearDestroyedPersistentId(uint64_t persistentId);
private:
    void MarkProcessed();
    void SendEvent(int32_t eventId, int64_t delayTime);
    void SetAnrHandleState(int32_t eventId, bool status);
    void ClearExpiredEvents(int32_t eventId);
    uint64_t GetPersistentIdOfEvent(int32_t eventId);
    bool IsOnEventHandler(uint64_t persistentId);
private:
    std::recursive_mutex mutex_;
    std::shared_ptr<AppExecFwk::EventHandler> eventHandler_ { nullptr };
    struct ANRHandlerState {
        std::unordered_map<uint64_t, bool> sendStatus;
        int32_t currentEventIdToReceipt { -1 };
    };
    ANRHandlerState anrHandlerState_;
    std::unordered_map<int32_t, wptr<ISessionStage>> sessionStageMap_;
};
} // namespace Rosen
} // namespace OHOS
#endif // ANR_HANDLER_H