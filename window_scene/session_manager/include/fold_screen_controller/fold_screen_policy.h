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
#ifndef OHOS_ROSEN_WINDOW_SCENE_FOLD_SCREEN_POLICY_H
#define OHOS_ROSEN_WINDOW_SCENE_FOLD_SCREEN_POLICY_H

#include <refbase.h>
#include <mutex>

#include "dm_common.h"
#include "session/screen/include/screen_property.h"
#include "fold_screen_info.h"

namespace OHOS::Rosen {
const uint32_t FOLD_TO_EXPAND_TASK_NUM = 3;
class FoldScreenPolicy : public RefBase {
public:
    FoldScreenPolicy();
    virtual ~FoldScreenPolicy();

    virtual void ChangeScreenDisplayMode(FoldDisplayMode displayMode);
    virtual void SendSensorResult(FoldStatus foldStatus);
    virtual ScreenId GetCurrentScreenId();
    virtual sptr<FoldCreaseRegion> GetCurrentFoldCreaseRegion();
    virtual void LockDisplayStatus(bool locked);
    virtual void SetOnBootAnimation(bool onBootAnimation);
    virtual void UpdateForPhyScreenPropertyChange();
    void ClearState();
    FoldDisplayMode GetScreenDisplayMode();
    FoldStatus GetFoldStatus();
    void SetFoldStatus(FoldStatus foldStatus);

    ScreenId screenId_ { SCREEN_ID_INVALID };
    ScreenProperty screenProperty_;
    mutable std::recursive_mutex displayModeMutex_;
    FoldDisplayMode currentDisplayMode_ = FoldDisplayMode::UNKNOWN;
    FoldStatus currentFoldStatus_ = FoldStatus::UNKNOWN;
    FoldDisplayMode globalDisplayMode_ = FoldDisplayMode::UNKNOWN;
    FoldStatus globalFoldStatus_ = FoldStatus::UNKNOWN;
    sptr<FoldCreaseRegion> currentFoldCreaseRegion_ = nullptr;
    bool lockDisplayStatus_ = false;
    bool onBootAnimation_ = false;
    /*
     *    Avoid fold to expand process queues public interface
     */
    bool GetModeChangeRunningStatus();
    virtual void SetdisplayModeChangeStatus(bool status){};
    bool GetdisplayModeRunningStatus();
    FoldDisplayMode GetLastCacheDisplayMode();
    
protected:
    /*
     *    Avoid fold to expand process queues private variable
     */
    std::atomic<int> pengdingTask_{FOLD_TO_EXPAND_TASK_NUM};
    std::atomic<bool> displayModeChangeRunning_ = false;
    std::atomic<FoldDisplayMode> lastCachedisplayMode_ = FoldDisplayMode::UNKNOWN;
    std::chrono::steady_clock::time_point startTimePoint_ = std::chrono::steady_clock::now();
    std::chrono::steady_clock::time_point endTimePoint_ = std::chrono::steady_clock::now();
    void SetLastCacheDisplayMode(FoldDisplayMode mode);
    int64_t getFoldingElapsedMs();
};
} // namespace OHOS::Rosen
#endif //OHOS_ROSEN_WINDOW_SCENE_FOLD_SCREEN_POLICY_H