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

#ifndef OHOS_ROSEN_WINDOW_SUPER_FOLD_POLICY_H
#define OHOS_ROSEN_WINDOW_SUPER_FOLD_POLICY_H

#include "transaction/rs_interfaces.h"
#include "dm_common.h"
#include "screen_session_manager.h"

namespace OHOS::Rosen {
namespace {
    const uint32_t SWITCH_SCREEN_TASK_NUM = 2;
}
class SuperFoldPolicy {
WM_DECLARE_SINGLE_INSTANCE_BASE(SuperFoldPolicy);
public:
    SuperFoldPolicy() = default;
    ~SuperFoldPolicy() = default;
    bool IsFakeDisplayExist();
    bool IsNeedSetSnapshotRect(DisplayId displayId);
    Drawing::Rect GetSnapshotRect(DisplayId displayId, bool isCaptureFullOfScreen = false);
    DMRect GetRecordRect(const std::vector<ScreenId>& screenIds);
    ScreenId GetRealScreenId(const std::vector<ScreenId>& screenIds);
    DMError QueryMultiScreenCapture(const std::vector<ScreenId>& displayIdList, DMRect& rect);
    ScreenId SetScreenSwitchState(ScreenClosedState screenClosedState, bool isScreenOn);
    ScreenClosedState GetScreenClosedState() const;
    ScreenId GetCurrentScreenId();
    void SetCurrentScreenId(ScreenId screenId);
    DMError ChangeScreenDisplayMode(FoldDisplayMode displayMode);
    DMError ChangeScreenDisplayModeInner(FoldDisplayMode displayMode, bool isScreenOn);
    void SetLastCacheDisplayMode(FoldDisplayMode displayMode);
    bool CheckDisplayMode(FoldDisplayMode displayMode);
    void ReportFoldDisplayModeChange(FoldDisplayMode displayMode);
    FoldDisplayMode GetCurrentDisplayMode();
    void SetCurrentDisplayMode(FoldDisplayMode displayMode);
    void SetdisplayModeChangeStatus(bool status, bool isOnBootAnimation = false);
    void SetOnBootAnimation(bool onBootAnimation);
    void RecoverWhenBootAnimationExit();
    bool GetModeChangeRunningStatus();
    bool GetdisplayModeRunningStatus();
    void ChangeScreenDisplayModeToCoordination(bool isScreenOn);

private:
    std::atomic<ScreenClosedState> screenClosedState_ = ScreenClosedState::UNKNOWN;
    std::mutex currentScreenIdMutex_;
    ScreenId currentScreenId_ = { SCREEN_ID_INVALID };
    std::atomic<FoldDisplayMode> lastCachedisplayMode_ = FoldDisplayMode::UNKNOWN;
    std::atomic<FoldDisplayMode> currentDisplayMode_ = FoldDisplayMode::UNKNOWN;
    std::atomic<int> pengdingTask_{SWITCH_SCREEN_TASK_NUM};
    std::atomic<bool> displayModeChangeRunning_ = false;
    std::atomic<bool> onBootAnimation_ = true;
    std::chrono::steady_clock::time_point startTimePoint_ = std::chrono::steady_clock::now();
    std::chrono::steady_clock::time_point endTimePoint_ = std::chrono::steady_clock::now();
};
}
#endif // OHOS_ROSEN_WINDOW_SUPER_FOLD_POLICY_H
