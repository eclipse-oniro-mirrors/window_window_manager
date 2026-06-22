/*
 * Copyright (c) 2024 Huawei Device Co., Ltd.
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

#ifndef PC_FOLD_SCREEN_CONTROLLER_H
#define PC_FOLD_SCREEN_CONTROLLER_H

#include <chrono>
#include "session/host/include/pc_fold_screen_manager.h"

namespace OHOS::Rosen {

class SceneSession;
using RectRecordsVector =
    std::vector<std::pair<std::chrono::time_point<std::chrono::high_resolution_clock>, WSRect>>;

class PcFoldScreenController : public RefBase {
public:
    PcFoldScreenController(wptr<SceneSession> weakSession, int32_t persistentId);
    ~PcFoldScreenController();
    void OnConnect();
    bool IsHalfFolded(DisplayId displayId);
    bool IsAllowThrowSlip(DisplayId displayId);
    bool NeedFollowHandAnimation();
    void RecordStartMoveRect(const WSRect& rect, bool isStartFullScreen);
    void RecordStartMoveRectDirectly(const WSRect& rect, ThrowSlipMode throwSlipMode, const WSRectF& velocity,
        bool isStartFullScreen);
    void ResetRecords();
    void RecordMoveRects(const WSRect& rect);
    bool ThrowSlip(DisplayId displayId, WSRect& rect, int32_t topAvoidHeight, int32_t botAvoidHeight);
    void ThrowSlipFloatingRectDirectly(WSRect& rect, const WSRect& floatingRect,
        int32_t topAvoidHeight, int32_t botAvoidHeight);
    bool IsThrowSlipDirectly() const;
    bool IsStartFullScreen();
    void ResizeToFullScreen(WSRect& rect, int32_t topAvoidHeight, int32_t botAvoidHeight);

    // animation parameters
    RSAnimationTimingProtocol GetMovingTimingProtocol();
    RSAnimationTimingCurve GetMovingTimingCurve();
    RSAnimationTimingProtocol GetThrowSlipTimingProtocol();
    RSAnimationTimingCurve GetThrowSlipTimingCurve();

    void UpdateFullScreenWaterfallMode(bool isWaterfallMode);
    bool IsFullScreenWaterfallMode() const { return isFullScreenWaterfallMode_; }
    void SetFullScreenWaterfallMode(bool isFullScreenWaterfallMode);
    void UpdateRect();
    void RegisterFullScreenWaterfallModeChangeCallback(std::function<void(bool isWaterfallMode)>&& func);
    void UnregisterFullScreenWaterfallModeChangeCallback();
    void UpdateSupportEnterWaterfallMode();
    void MaskSupportEnterWaterfallMode();
    void UpdateSupportEnterWaterfallMode(bool isSupportEnterWaterfallMode);

private:
    int32_t GetPersistentId() const;
    DisplayId GetDisplayId();
    int32_t GetTitleHeight() const;
    WSRectF CalculateMovingVelocity();
    bool IsThrowSlipModeDirectly(ThrowSlipMode throwSlipMode) const;
    void ThrowSlipHiSysEvent(const std::string& bundleName, ScreenSide startSide,
        ThrowSlipWindowMode startWindowMode, ThrowSlipMode throwMode) const;

    bool IsSupportEnterWaterfallMode(SuperFoldStatus status, bool hasSystemKeyboard) const;
    void FoldStatusChangeForSupportEnterWaterfallMode(
        DisplayId displayId, SuperFoldStatus status, SuperFoldStatus prevStatus);
    void SystemKeyboardStatusChangeForSupportEnterWaterfallMode(
        DisplayId displayId, bool hasSystemKeyboard);

    wptr<SceneSession> weakSceneSession_ = nullptr;
    int32_t persistentId_;

    // use queue to calculate velocity
    mutable std::mutex moveMutex_;
    WSRect startMoveRect_;
    bool isStartFullScreen_ { false };
    bool isStartWaterfallMode_ { false };
    RectRecordsVector movingRectRecords_;
    ThrowSlipMode startThrowSlipMode_ { ThrowSlipMode::INVALID };
    WSRectF startVelocity_;
    // Above guarded by moveMutex_

    std::shared_ptr<FoldScreenStatusChangeCallback> onFoldScreenStatusChangeCallback_;
    std::shared_ptr<SystemKeyboardStatusChangeCallback> onSystemKeyboardStatusChangeCallback_;

    /*
     * Waterfall Mode
     * accessed on SSM thread
     */
    void ExecuteFullScreenWaterfallModeChangeCallback();
    bool isFullScreenWaterfallMode_ { false };
    bool lastSupportEnterWaterfallMode_ { false };
    bool supportEnterWaterfallMode_ { false };
    bool maskSupportEnterWaterfallMode_ { false };
    std::function<void(bool isWaterfallMode)> fullScreenWaterfallModeChangeCallback_ { nullptr };
};
} // namespace OHOS::Rosen

#endif // PC_FOLD_SCREEN_CONTROLLER_H