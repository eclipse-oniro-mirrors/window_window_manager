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

#ifndef OHOS_ROSEN_WINDOW_FOCUS_CONTROLLER_H
#define OHOS_ROSEN_WINDOW_FOCUS_CONTROLLER_H

#include <string>
#include <unordered_map>
#include <unordered_set>
#include <shared_mutex>

#include "dm_common.h"
#include "focus_change_info.h"
#include "wm_common.h"
#include "ws_common.h"
#include "window_manager_hilog.h"

namespace OHOS {
namespace Rosen {
class FocusGroup : public RefBase {
public:
    explicit FocusGroup(DisplayId displayGroupId) : displayGroupId_(displayGroupId) {}

    int32_t GetFocusedSessionId() const { return focusedSessionId_; }
    int32_t GetLastFocusedSessionId() const { return lastFocusedSessionId_; }
    int32_t GetLastFocusedAppSessionId() const { return lastFocusedAppSessionId_; }
    bool GetNeedBlockNotifyFocusStatusUntilForeground() const { return needBlockNotifyFocusStatusUntilForeground_; }
    bool GetNeedBlockNotifyUnfocusStatus() const { return needBlockNotifyUnfocusStatus_; }
    DisplayId GetDisplayGroupId() const { return displayGroupId_; }
    int64_t GetUpdateFocusTimeStamp() const { return updateFocusTimeStamp_; }
    void SetFocusedSessionId(int32_t persistentId) { focusedSessionId_ = persistentId; }
    void SetLastFocusedSessionId(int32_t persistentId) { lastFocusedSessionId_ = persistentId; }
    void SetLastFocusedAppSessionId(int32_t persistentId) { lastFocusedAppSessionId_ = persistentId; }
    void SetNeedBlockNotifyFocusStatusUntilForeground(bool needBlock)
    {
        needBlockNotifyFocusStatusUntilForeground_ = needBlock;
    }
    void SetNeedBlockNotifyUnfocusStatus(bool needBlock) { needBlockNotifyUnfocusStatus_ = needBlock; }
    void SetUpdateFocusTimeStamp(int64_t timeStamp) { updateFocusTimeStamp_ = timeStamp; }
    WSError UpdateFocusedSessionId(int32_t persistentId);
    WSError UpdateFocusedAppSessionId(int32_t persistentId);
    std::unordered_set<DisplayId> displayIds_;

private:
    int32_t focusedSessionId_ = INVALID_SESSION_ID;
    int32_t lastFocusedSessionId_ = INVALID_SESSION_ID;
    int32_t lastFocusedAppSessionId_ = INVALID_SESSION_ID;
    bool needBlockNotifyFocusStatusUntilForeground_ { false };
    bool needBlockNotifyUnfocusStatus_ { false };
    DisplayId displayGroupId_ = DISPLAY_ID_INVALID;
    int64_t updateFocusTimeStamp_ = INVALID_TIME_STAMP;
};

class WindowFocusController : public RefBase {
public:
    WindowFocusController() noexcept;
    ~WindowFocusController() = default;

    DisplayId GetDisplayGroupId(DisplayId displayId);
    WSError AddFocusGroup(DisplayGroupId displayGroupId, DisplayId displayId);
    WSError RemoveFocusGroup(DisplayGroupId displayGroupId, DisplayId displayId);
    int32_t GetFocusedSessionId(DisplayId displayId);
    sptr<FocusGroup> GetFocusGroup(DisplayId displayId = DEFAULT_DISPLAY_ID);
    std::vector<std::pair<DisplayId, int32_t>> GetAllFocusedSessionList() const;
    WSError UpdateFocusedSessionId(DisplayId displayId, int32_t persistentId);
    WSError UpdateFocusedAppSessionId(DisplayId displayId, int32_t persistentId);
    void LogDisplayIds();
    std::unordered_map<DisplayId, DisplayGroupId> GetDisplayId2GroupIdMap();
    void GetAllFocusGroup(std::unordered_map<DisplayGroupId, sptr<FocusGroup>>& focusGroupMap);

private:
    sptr<FocusGroup> GetFocusGroupInner(DisplayId displayId);

    std::unordered_map<DisplayGroupId, sptr<FocusGroup>> focusGroupMap_;
    std::unordered_map<DisplayId, DisplayGroupId> displayId2GroupIdMap_;
    std::unordered_map<DisplayId, DisplayGroupId> deletedDisplayId2GroupIdMap_;
    mutable std::mutex focusGroupMapMutex_;
    mutable std::mutex displayId2GroupIdMapMutex_;
};
}
}
#endif // OHOS_ROSEN_WINDOW_FOCUS_CONTROLLER_H
