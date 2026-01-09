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

#include "session/host/include/ws_snapshot_helper.h"
#include "parameters.h"

namespace OHOS::Rosen {
namespace {
const bool CORRECTION_ENABLE = system::GetIntParameter<int32_t>("const.system.sensor_correction_enable", 0) == 1;
const std::unordered_map<int32_t, DisplayOrientation> ROTATION_TO_DISPLAYORIENTATION_MAP = {
    { PORTRAIT_ANGLE, DisplayOrientation::PORTRAIT },
    { LANDSCAPE_ANGLE, DisplayOrientation::LANDSCAPE },
    { PORTRAIT_INVERTED_ANGLE, DisplayOrientation::PORTRAIT_INVERTED },
    { LANDSCAPE_INVERTED_ANGLE, DisplayOrientation::LANDSCAPE_INVERTED }
};
}

WSSnapshotHelper* WSSnapshotHelper::GetInstance()
{
    static WSSnapshotHelper instance;
    return &instance;
}

int32_t WSSnapshotHelper::GetScreenStatus()
{
    std::lock_guard lock(statusMutex_);
    return GetInstance()->screenStatus_;
}

// LCOV_EXCL_START
int32_t WSSnapshotHelper::GetScreenStatus(FoldStatus foldStatus)
{
    if (foldStatus == FoldStatus::UNKNOWN || foldStatus == FoldStatus::FOLDED ||
        foldStatus == FoldStatus::FOLD_STATE_FOLDED_WITH_SECOND_EXPAND ||
        foldStatus == FoldStatus::FOLD_STATE_FOLDED_WITH_SECOND_HALF_FOLDED) {
        return SCREEN_FOLDED;
    } else if (foldStatus == FoldStatus::EXPAND || foldStatus == FoldStatus::HALF_FOLD) {
        return SCREEN_EXPAND;
    }
    return SCREEN_UNKNOWN;
}

DisplayOrientation WSSnapshotHelper::GetDisplayOrientation(int32_t rotation)
{
    auto it = ROTATION_TO_DISPLAYORIENTATION_MAP.find(rotation);
    if (it != ROTATION_TO_DISPLAYORIENTATION_MAP.end()) {
        return it->second;
    }
    return DisplayOrientation::PORTRAIT;
}

void WSSnapshotHelper::SetWindowScreenStatus(int32_t screenStatus)
{
    std::lock_guard lock(statusMutex_);
    GetInstance()->screenStatus_ = screenStatus;
}

void WSSnapshotHelper::SetWindowScreenStatus(FoldStatus foldStatus)
{
    SetWindowScreenStatus(GetScreenStatus(foldStatus));
}

void WSSnapshotHelper::SetWindowOrientationStatus(Rotation rotation)
{
    std::lock_guard lock(rotationMutex_);
    GetInstance()->screenRotation_ = rotation;
}

uint32_t WSSnapshotHelper::GetWindowRotation() const
{
    uint32_t rotation = 0;
    {
        std::lock_guard lock(rotationMutex_);
        rotation = static_cast<uint32_t>(GetInstance()->screenRotation_);
    }
    if (CORRECTION_ENABLE && GetInstance()->GetScreenStatus() == 0) {
        return (rotation + SECONDARY_EXPAND_OFFSET) % ROTATION_COUNT;
    }
    return rotation;
}
// LCOV_EXCL_STOP
} // namespace OHOS::Rosen