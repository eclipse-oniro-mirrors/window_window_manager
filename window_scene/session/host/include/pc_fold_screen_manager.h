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

#ifndef PC_FOLD_SCREEN_MANAGER_H
#define PC_FOLD_SCREEN_MANAGER_H

#include <mutex>
#include <shared_mutex>
#include <tuple>
#include <unordered_map>

#include <animation/rs_animation_timing_curve.h>
#include <animation/rs_animation_timing_protocol.h>
#include "dm_common.h"
#include "interfaces/include/ws_common.h"
#include "wm_single_instance.h"

namespace OHOS::Rosen {
using FoldScreenStatusChangeCallback = std::function<void(DisplayId displayId,
    SuperFoldStatus status, SuperFoldStatus prevStatus)>;
using SystemKeyboardStatusChangeCallback = std::function<void(DisplayId displayId, bool hasSystemKeyboard)>;

enum class ScreenSide : uint8_t {
    EXPAND = 0,
    FOLD_B = 1,
    FOLD_C = 2,
};

enum class ThrowSlipWindowMode : int32_t {
    FLOAT = 0,
    FULLSCREEN = 1,
    FULLSCREEN_WATERFALLMODE = 2,
};

enum class ThrowSlipMode : int32_t {
    INVALID = -1,
    MOVE = 0,
    BUTTON = 2,
    THREE_FINGERS_SWIPE = 3,
    FOUR_FINGERS_SWIPE = 4,
    FIVE_FINGERS_SWIPE = 5,
};

class PcFoldScreenManager {
WM_DECLARE_SINGLE_INSTANCE(PcFoldScreenManager);
public:
    void UpdateFoldScreenStatus(DisplayId displayId, SuperFoldStatus status,
        const WSRect& defaultDisplayRect, const WSRect& virtualDisplayRect, const WSRect& foldCreaseRect);
    SuperFoldStatus GetScreenFoldStatus() const;
    SuperFoldStatus GetScreenFoldStatus(DisplayId displayId) const;
    bool IsPcFoldScreen(DisplayId displayId) const;
    bool IsHalfFolded(DisplayId displayId) const;
    bool IsHalfFoldedOnMainDisplay(DisplayId displayId) const;
    
    void UpdateSystemKeyboardStatus(bool hasSystemKeyboard);
    bool HasSystemKeyboard() const;

    int32_t GetVirtualDisplayPosY() const;
    std::tuple<WSRect, WSRect, WSRect> GetDisplayRects() const;

    // animation parameters
    RSAnimationTimingProtocol GetMovingTimingProtocol();
    RSAnimationTimingCurve GetMovingTimingCurve();
    RSAnimationTimingProtocol GetThrowSlipTimingProtocol();
    RSAnimationTimingCurve GetThrowSlipTimingCurve();

    ScreenSide CalculateScreenSide(const WSRect& rect);
    ScreenSide CalculateScreenSide(int32_t posY);
    bool IsCrossFoldCrease(const WSRect& rect);

    void ResetArrangeRule();
    void ResetArrangeRule(ScreenSide side);
    void ResetArrangeRule(const WSRect& rect);

    void ResizeToFullScreen(WSRect& rect, int32_t topAvoidHeight, int32_t botAvoidHeight);

    bool NeedDoThrowSlip(const WSRect& rect, const WSRectF& velocity, ScreenSide& throwSide);
    bool NeedDoEasyThrowSlip(const WSRect& rect, const WSRect& startRect,
        const WSRectF& velocity, ScreenSide& throwSide);
    bool CheckVelocityOrientation(const WSRect& rect, const WSRectF& velocity);
    WSRect CalculateThrowBacktracingRect(const WSRect& rect, const WSRectF& velocity);
    WSRect CalculateThrowEnd(const WSRect& rect, const WSRectF& velocity);
    bool ThrowSlipToOppositeSide(ScreenSide startSide, WSRect& rect,
        int32_t topAvoidHeight, int32_t botAvoidHeight, int32_t titleHeight);
    void MappingRectInScreenSide(ScreenSide side, WSRect& rect,
        int32_t topAvoidHeight, int32_t botAvoidHeight);
    void MappingRectInScreenSideWithArrangeRule(ScreenSide side, WSRect& rect,
        int32_t topAvoidHeight, int32_t botAvoidHeight, int32_t titleHeight);

    void ApplyInitArrangeRule(WSRect& rect, WSRect& lastArrangedRect,
        const WSRect& limitRect, int32_t titleHeight);
    void ApplyArrangeRule(WSRect& rect, WSRect& lastArrangedRect,
        const WSRect& limitRect, int32_t titleHeight);

    void RegisterFoldScreenStatusChangeCallback(int32_t persistentId,
        const std::weak_ptr<FoldScreenStatusChangeCallback>& func);
    void UnregisterFoldScreenStatusChangeCallback(int32_t persistentId);
    void RegisterSystemKeyboardStatusChangeCallback(int32_t persistentId,
        const std::weak_ptr<SystemKeyboardStatusChangeCallback>& func);
    void UnregisterSystemKeyboardStatusChangeCallback(int32_t persistentId);
    DisplayId GetDisplayId() const { return displayId_; }

private:
    void SetDisplayInfo(DisplayId displayId, SuperFoldStatus status);
    void SetDisplayRects(
        const WSRect& defaultDisplayRect, const WSRect& virtualDisplayRect, const WSRect& foldCreaseRect);
    float GetVpr() const;

    /*
     * fold screen property
     * if need, use map for multi fold screen
     */
    mutable std::shared_mutex displayInfoMutex_; // protect display infos
    DisplayId displayId_ { SCREEN_ID_INVALID };
    float vpr_ { 1.5f }; // display vp ratio
    SuperFoldStatus prevScreenFoldStatus_ { SuperFoldStatus::UNKNOWN };
    SuperFoldStatus screenFoldStatus_ { SuperFoldStatus::UNKNOWN };
    bool hasSystemKeyboard_ { false }; // bottom system keyboard
    // Above guarded by displayInfoMutex_

    mutable std::shared_mutex rectsMutex_; // protect rects
    WSRect defaultDisplayRect_;
    WSRect virtualDisplayRect_;
    WSRect foldCreaseRect_;
    // Above guarded by rectsMutex_

    /*
     * arranged rect
     * x,y: last arranged position
     * w,h: offset for next arranged position
     */
    std::mutex arrangedRectsMutex_;
    WSRect defaultArrangedRect_;
    WSRect virtualArrangedRect_;
    // Above guarded by arrangedRectsMutex_

    void ExecuteFoldScreenStatusChangeCallbacks(DisplayId displayId,
        SuperFoldStatus status, SuperFoldStatus prevStatus);
    void ExecuteSystemKeyboardStatusChangeCallbacks(DisplayId displayId, bool hasSystemKeyboard);
    std::mutex callbackMutex_;
    std::unordered_map<int32_t, std::weak_ptr<FoldScreenStatusChangeCallback>> foldScreenStatusChangeCallbacks_;
    std::unordered_map<int32_t, std::weak_ptr<SystemKeyboardStatusChangeCallback>>
        systemKeyboardStatusChangeCallbacks_;
};
} // namespace OHOS::Rosen

#endif // PC_FOLD_SCREEN_MANAGER_H
