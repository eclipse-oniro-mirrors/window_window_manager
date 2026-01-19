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

#include "session/host/include/pc_fold_screen_controller.h"
#include <hisysevent.h>
#include <parameters.h>
#include "session/host/include/scene_session.h"
#include "window_helper.h"
#include "window_manager_hilog.h"
#include "wm_common_inner.h"
#include "wm_math.h"

namespace OHOS::Rosen {

namespace {
// moving
constexpr int32_t MOVING_RECORDS_SIZE_LIMIT = 5;
constexpr int32_t MOVING_RECORDS_TIME_LIMIT = 100;
constexpr float MOVING_DIRECTLY_BUFF = 2.0f;
constexpr WSRect ZERO_RECT = { 0, 0, 0, 0 };
constexpr WSRectF ZERO_RECTF = { 0.0f, 0.0f, 0.0f, 0.0f };

// arrange rule
constexpr int32_t MIN_DECOR_HEIGHT = 37;
constexpr int32_t MAX_DECOR_HEIGHT = 112;
} // namespace

PcFoldScreenController::PcFoldScreenController(wptr<SceneSession> weakSession, int32_t persistentId)
    : weakSceneSession_(std::move(weakSession)), persistentId_(persistentId) {}

PcFoldScreenController::~PcFoldScreenController()
{
    PcFoldScreenManager::GetInstance().UnregisterFoldScreenStatusChangeCallback(GetPersistentId());
    PcFoldScreenManager::GetInstance().UnregisterSystemKeyboardStatusChangeCallback(GetPersistentId());
}

void PcFoldScreenController::OnConnect()
{
    onFoldScreenStatusChangeCallback_ = std::make_shared<FoldScreenStatusChangeCallback>(
        [weakThis = wptr(this)](DisplayId displayId, SuperFoldStatus status, SuperFoldStatus prevStatus) {
            auto controller = weakThis.promote();
            if (controller == nullptr) {
                TLOGNE(WmsLogTag::WMS_LAYOUT_PC, "controller is nullptr");
                return;
            }
            auto sceneSession = controller->weakSceneSession_.promote();
            if (sceneSession == nullptr) {
                TLOGNE(WmsLogTag::WMS_LAYOUT_PC, "session is nullptr, id: %{public}d", controller->GetPersistentId());
                return;
            }
            sceneSession->PostTask([weakThis, displayId, status, prevStatus] {
                auto controller = weakThis.promote();
                if (controller == nullptr) {
                    TLOGNE(WmsLogTag::WMS_LAYOUT_PC, "controller is nullptr");
                    return;
                }
                if (status != prevStatus) {
                    controller->UpdateRect();
                }
                controller->FoldStatusChangeForSupportEnterWaterfallMode(displayId, status, prevStatus);
            });
        }
    );
    PcFoldScreenManager::GetInstance().RegisterFoldScreenStatusChangeCallback(GetPersistentId(),
        std::weak_ptr<FoldScreenStatusChangeCallback>(onFoldScreenStatusChangeCallback_));

    onSystemKeyboardStatusChangeCallback_ = std::make_shared<SystemKeyboardStatusChangeCallback>(
        [weakThis = wptr(this)](DisplayId displayId, bool hasSystemKeyboard) {
            auto controller = weakThis.promote();
            if (controller == nullptr) {
                TLOGNE(WmsLogTag::WMS_LAYOUT_PC, "controller is nullptr");
                return;
            }
            auto sceneSession = controller->weakSceneSession_.promote();
            if (sceneSession == nullptr) {
                TLOGNE(WmsLogTag::WMS_LAYOUT_PC, "session is nullptr, id: %{public}d", controller->GetPersistentId());
                return;
            }
            sceneSession->PostTask([weakThis, displayId, hasSystemKeyboard] {
                auto controller = weakThis.promote();
                if (controller == nullptr) {
                    TLOGNE(WmsLogTag::WMS_LAYOUT_PC, "controller is nullptr");
                    return;
                }
                controller->SystemKeyboardStatusChangeForSupportEnterWaterfallMode(displayId, hasSystemKeyboard);
            });
        }
    );
    PcFoldScreenManager::GetInstance().RegisterSystemKeyboardStatusChangeCallback(GetPersistentId(),
        std::weak_ptr<SystemKeyboardStatusChangeCallback>(onSystemKeyboardStatusChangeCallback_));

    supportEnterWaterfallMode_ = IsSupportEnterWaterfallMode(
        PcFoldScreenManager::GetInstance().GetScreenFoldStatus(GetDisplayId()),
        PcFoldScreenManager::GetInstance().HasSystemKeyboard());
    UpdateSupportEnterWaterfallMode();
}

bool PcFoldScreenController::IsSupportEnterWaterfallMode(SuperFoldStatus status, bool hasSystemKeyboard) const
{
    return status == SuperFoldStatus::HALF_FOLDED && !hasSystemKeyboard && !isFullScreenWaterfallMode_;
}

void PcFoldScreenController::FoldStatusChangeForSupportEnterWaterfallMode(
    DisplayId displayId, SuperFoldStatus status, SuperFoldStatus prevStatus)
{
    if (!PcFoldScreenManager::GetInstance().IsPcFoldScreen(GetDisplayId())) {
        TLOGE(WmsLogTag::WMS_LAYOUT_PC, "only main screen support water fall mode.");
        return;
    }
    lastSupportEnterWaterfallMode_ = supportEnterWaterfallMode_;
    supportEnterWaterfallMode_ = IsSupportEnterWaterfallMode(status,
        PcFoldScreenManager::GetInstance().HasSystemKeyboard());
    auto sceneSession = weakSceneSession_.promote();
    if (sceneSession == nullptr) {
        TLOGE(WmsLogTag::WMS_LAYOUT_PC, "session is nullptr, id: %{public}d", GetPersistentId());
        return;
    }
    if (!sceneSession->IsMissionHighlighted()) {
        return;
    }
    UpdateSupportEnterWaterfallMode();
}

void PcFoldScreenController::UpdateSupportEnterWaterfallMode(bool isSupportEnterWaterfallMode)
{
    TLOGD(WmsLogTag::WMS_LAYOUT_PC, "isSupportEnterWaterfallMode: %{public}d", isSupportEnterWaterfallMode);
    auto sceneSession = weakSceneSession_.promote();
    if (sceneSession == nullptr) {
        TLOGE(WmsLogTag::WMS_LAYOUT_PC, "session unavailable, id: %{public}d", GetPersistentId());
        return;
    }
    lastSupportEnterWaterfallMode_ = isSupportEnterWaterfallMode;
    supportEnterWaterfallMode_ = isSupportEnterWaterfallMode;
    sceneSession->SetSupportEnterWaterfallMode(isSupportEnterWaterfallMode);
}

void PcFoldScreenController::SystemKeyboardStatusChangeForSupportEnterWaterfallMode(
    DisplayId displayId, bool hasSystemKeyboard)
{
    lastSupportEnterWaterfallMode_ = supportEnterWaterfallMode_;
    supportEnterWaterfallMode_ = IsSupportEnterWaterfallMode(
        PcFoldScreenManager::GetInstance().GetScreenFoldStatus(displayId), hasSystemKeyboard);
    auto sceneSession = weakSceneSession_.promote();
    if (sceneSession == nullptr) {
        TLOGE(WmsLogTag::WMS_LAYOUT_PC, "session is nullptr, id: %{public}d", GetPersistentId());
        return;
    }
    if (!sceneSession->IsMissionHighlighted()) {
        return;
    }
    UpdateSupportEnterWaterfallMode();
}

bool PcFoldScreenController::IsHalfFolded(DisplayId displayId)
{
    return PcFoldScreenManager::GetInstance().IsHalfFolded(displayId);
}

bool PcFoldScreenController::IsAllowThrowSlip(DisplayId displayId)
{
    auto sceneSession = weakSceneSession_.promote();
    if (sceneSession == nullptr) {
        return false;
    }
    bool sessionValid = WindowHelper::IsMainWindow(sceneSession->GetWindowType()) || sceneSession->IsDecorEnable();
    return sessionValid && PcFoldScreenManager::GetInstance().IsHalfFolded(displayId) &&
           !PcFoldScreenManager::GetInstance().HasSystemKeyboard();
}

bool PcFoldScreenController::NeedFollowHandAnimation()
{
    static bool needFollowHandAnimation =
        system::GetParameter("persist.window.throw_slip_follow_animation.enabled", "1") == "1";
    std::unique_lock<std::mutex> lock(moveMutex_);
    return needFollowHandAnimation && (!isStartFullScreen_ || movingRectRecords_.size() > 0);
}

void PcFoldScreenController::RecordStartMoveRect(const WSRect& rect, bool isStartFullScreen)
{
    TLOGI(WmsLogTag::WMS_LAYOUT_PC, "rect: %{public}s, isStartFullScreen: %{public}d",
        rect.ToString().c_str(), isStartFullScreen);
    std::unique_lock<std::mutex> lock(moveMutex_);
    startMoveRect_ = rect;
    isStartFullScreen_ = isStartFullScreen;
    isStartWaterfallMode_ = isFullScreenWaterfallMode_;
    movingRectRecords_.clear();
    startThrowSlipMode_ = ThrowSlipMode::MOVE;
}

void PcFoldScreenController::RecordStartMoveRectDirectly(const WSRect& rect, ThrowSlipMode throwSlipMode,
    const WSRectF& velocity, bool isStartFullScreen)
{
    TLOGI(WmsLogTag::WMS_LAYOUT_PC, "rect: %{public}s, isStartFullScreen: %{public}d, velocity: %{public}s",
        rect.ToString().c_str(), isStartFullScreen, velocity.ToString().c_str());
    RecordStartMoveRect(rect, isStartFullScreen);
    std::unique_lock<std::mutex> lock(moveMutex_);
    startThrowSlipMode_ = throwSlipMode;
    startVelocity_ = velocity;
    startVelocity_.posY_ *= MOVING_DIRECTLY_BUFF;
}

void PcFoldScreenController::ResetRecords()
{
    TLOGD(WmsLogTag::WMS_LAYOUT_PC, "in");
    std::unique_lock<std::mutex> lock(moveMutex_);
    startMoveRect_ = ZERO_RECT;
    isStartFullScreen_ = false;
    isStartWaterfallMode_ = false;
    movingRectRecords_.clear();
    startThrowSlipMode_ = ThrowSlipMode::INVALID;
    startVelocity_ = ZERO_RECTF;
}

bool PcFoldScreenController::IsStartFullScreen()
{
    std::unique_lock<std::mutex> lock(moveMutex_);
    return isStartFullScreen_;
}

void PcFoldScreenController::RecordMoveRects(const WSRect& rect)
{
    auto time = std::chrono::high_resolution_clock::now();
    std::unique_lock<std::mutex> lock(moveMutex_);
    movingRectRecords_.push_back(std::make_pair(time, rect));
    TLOGD(WmsLogTag::WMS_LAYOUT_PC, "id: %{public}d, rect: %{public}s", GetPersistentId(), rect.ToString().c_str());
    // pop useless record
    while (movingRectRecords_.size() > MOVING_RECORDS_SIZE_LIMIT ||
           TimeHelper::GetDuration(movingRectRecords_[0].first, time) > MOVING_RECORDS_TIME_LIMIT) {
        movingRectRecords_.erase(movingRectRecords_.begin());
    }
    TLOGD(WmsLogTag::WMS_LAYOUT_PC, "records size: %{public}zu, duration: %{public}f", movingRectRecords_.size(),
        TimeHelper::GetDuration(movingRectRecords_[0].first, movingRectRecords_[movingRectRecords_.size() - 1].first));
}

/**
 * if move fast, window can be throwed to other side
 * @param rect: current rect. if throwed, move it to other side
 * @param topAvoidHeight: avoid status bar
 * @param botAvoidHeight: avoid dock
 */
bool PcFoldScreenController::ThrowSlip(DisplayId displayId, WSRect& rect,
    int32_t topAvoidHeight, int32_t botAvoidHeight)
{
    auto& manager = PcFoldScreenManager::GetInstance();
    if (!IsAllowThrowSlip(displayId)) {
        manager.ResetArrangeRule();
        return false;
    }
    WSRect startRect;
    bool startFullScreen;
    bool startWaterfallMode;
    ThrowSlipMode startThrowSlipMode;
    {
        std::unique_lock<std::mutex> lock(moveMutex_);
        manager.ResetArrangeRule(startMoveRect_);
        startRect = startMoveRect_;
        startFullScreen = isStartFullScreen_;
        startWaterfallMode = isStartWaterfallMode_;
        startThrowSlipMode = startThrowSlipMode_;
    }
    WSRect titleRect = { rect.posX_, rect.posY_, rect.width_,
        IsThrowSlipModeDirectly(startThrowSlipMode) ? rect.height_ : GetTitleHeight() };
    ScreenSide throwSide = manager.CalculateScreenSide(titleRect);
    const WSRectF& velocity = CalculateMovingVelocity();
    if (isFullScreenWaterfallMode_) {
        throwSide = MathHelper::GreatNotEqual(velocity.posY_, 0.0f) ? ScreenSide::FOLD_B : ScreenSide::FOLD_C;
    }
    if ((!startFullScreen && !manager.NeedDoThrowSlip(titleRect, velocity, throwSide)) ||
        (startFullScreen && !manager.NeedDoEasyThrowSlip(titleRect, startRect, velocity, throwSide))) {
        manager.ResetArrangeRule(throwSide);
        TLOGI(WmsLogTag::WMS_LAYOUT_PC, "no throw rect: %{public}s", rect.ToString().c_str());
        return false;
    }

    // correct to start rect and throw
    if (!startFullScreen) {
        rect.posX_ = startRect.posX_;
        rect.posY_ = startRect.posY_;
    }
    manager.ThrowSlipToOppositeSide(throwSide, rect, topAvoidHeight, botAvoidHeight, GetTitleHeight());
    manager.ResetArrangeRule(throwSide);

    // hisysevent
    ThrowSlipWindowMode startWindowMode = ThrowSlipWindowMode::FLOAT;
    if (startFullScreen) {
        startWindowMode = startWaterfallMode ? ThrowSlipWindowMode::FULLSCREEN_WATERFALLMODE :
                                               ThrowSlipWindowMode::FULLSCREEN;
    }
    auto sceneSession = weakSceneSession_.promote();
    std::string bundleName = sceneSession == nullptr ? "" : sceneSession->GetSessionInfo().bundleName_;
    ThrowSlipHiSysEvent(bundleName, throwSide, startWindowMode, startThrowSlipMode);
    TLOGI(WmsLogTag::WMS_LAYOUT_PC, "throw to rect: %{public}s", rect.ToString().c_str());
    return true;
}

void PcFoldScreenController::ThrowSlipHiSysEvent(const std::string& bundleName, ScreenSide startSide,
    ThrowSlipWindowMode startWindowMode, ThrowSlipMode throwMode) const
{
    int32_t ret = HiSysEventWrite(
        HiviewDFX::HiSysEvent::Domain::WINDOW_MANAGER,
        "THROW_SLIP",
        HiviewDFX::HiSysEvent::EventType::BEHAVIOR,
        "BUNDLE_NAME", bundleName,
        "START_SIDE", static_cast<int32_t>(startSide),
        "START_WINDOW_MODE", static_cast<int32_t>(startWindowMode),
        "THROW_MODE", static_cast<int32_t>(throwMode));
    if (ret != 0) {
        TLOGE(WmsLogTag::WMS_LAYOUT_PC, "write hisysevent error, ret: %{public}d", ret);
    }
}

void PcFoldScreenController::ThrowSlipFloatingRectDirectly(WSRect& rect, const WSRect& floatingRect,
    int32_t topAvoidHeight, int32_t botAvoidHeight)
{
    TLOGD(WmsLogTag::WMS_LAYOUT_PC, "rect: %{public}s, floating rect: %{public}s",
        rect.ToString().c_str(), floatingRect.ToString().c_str());
    auto& manager = PcFoldScreenManager::GetInstance();
    const ScreenSide side = manager.CalculateScreenSide(rect);
    const ScreenSide floatingSide = manager.CalculateScreenSide(floatingRect.posY_);
    rect = floatingRect;
    if (side == floatingSide) {
        return;
    }
    manager.ThrowSlipToOppositeSide(floatingSide,
        rect, topAvoidHeight, botAvoidHeight, GetTitleHeight());
}

bool PcFoldScreenController::IsThrowSlipDirectly() const
{
    std::unique_lock<std::mutex> lock(moveMutex_);
    return IsThrowSlipModeDirectly(startThrowSlipMode_);
}

bool PcFoldScreenController::IsThrowSlipModeDirectly(ThrowSlipMode throwSlipMode) const
{
    return throwSlipMode == ThrowSlipMode::BUTTON ||
        throwSlipMode == ThrowSlipMode::THREE_FINGERS_SWIPE ||
        throwSlipMode == ThrowSlipMode::FOUR_FINGERS_SWIPE ||
        throwSlipMode == ThrowSlipMode::FIVE_FINGERS_SWIPE;
}

/**
 * resize to fullscreen in one side considering avoid area
 * @param rect: resize in its side
 */
void PcFoldScreenController::ResizeToFullScreen(WSRect& rect, int32_t topAvoidHeight, int32_t botAvoidHeight)
{
    PcFoldScreenManager::GetInstance().ResizeToFullScreen(rect, topAvoidHeight, botAvoidHeight);
}

RSAnimationTimingProtocol PcFoldScreenController::GetMovingTimingProtocol()
{
    return PcFoldScreenManager::GetInstance().GetMovingTimingProtocol();
}

RSAnimationTimingCurve PcFoldScreenController::GetMovingTimingCurve()
{
    return PcFoldScreenManager::GetInstance().GetMovingTimingCurve();
}

RSAnimationTimingProtocol PcFoldScreenController::GetThrowSlipTimingProtocol()
{
    return PcFoldScreenManager::GetInstance().GetThrowSlipTimingProtocol();
}

RSAnimationTimingCurve PcFoldScreenController::GetThrowSlipTimingCurve()
{
    return PcFoldScreenManager::GetInstance().GetThrowSlipTimingCurve();
}

void PcFoldScreenController::UpdateFullScreenWaterfallMode(bool isWaterfallMode)
{
    auto sceneSession = weakSceneSession_.promote();
    if (sceneSession == nullptr) {
        TLOGE(WmsLogTag::WMS_LAYOUT_PC, "session is nullptr, id: %{public}d", GetPersistentId());
        return;
    }
    sceneSession->PostTask([weakThis = wptr(this), isWaterfallMode, where = __func__] {
        auto controller = weakThis.promote();
        if (controller == nullptr) {
            TLOGNE(WmsLogTag::WMS_LAYOUT_PC, "controller is nullptr");
            return;
        }
        if (controller->isFullScreenWaterfallMode_ == isWaterfallMode) {
            return;
        }
        if (isWaterfallMode && !controller->supportEnterWaterfallMode_) {
            TLOGNW(WmsLogTag::WMS_LAYOUT_PC, "%{public}s not support waterfall mode!", where);
        }
        TLOGNI(WmsLogTag::WMS_LAYOUT_PC, "%{public}s id: %{public}d, mode: %{public}d",
            where, controller->GetPersistentId(), isWaterfallMode);
        controller->isFullScreenWaterfallMode_ = isWaterfallMode;
        controller->ExecuteFullScreenWaterfallModeChangeCallback();
        controller->supportEnterWaterfallMode_ = controller->IsSupportEnterWaterfallMode(
            PcFoldScreenManager::GetInstance().GetScreenFoldStatus(controller->GetDisplayId()),
            PcFoldScreenManager::GetInstance().HasSystemKeyboard());
        controller->UpdateSupportEnterWaterfallMode();
    }, __func__);
}

void PcFoldScreenController::SetFullScreenWaterfallMode(bool isFullScreenWaterfallMode)
{
    isFullScreenWaterfallMode_ = isFullScreenWaterfallMode;
}

void PcFoldScreenController::RegisterFullScreenWaterfallModeChangeCallback(
    std::function<void(bool isWaterfallMode)>&& func)
{
    fullScreenWaterfallModeChangeCallback_ = std::move(func);
    ExecuteFullScreenWaterfallModeChangeCallback();
}

void PcFoldScreenController::UnregisterFullScreenWaterfallModeChangeCallback()
{
    fullScreenWaterfallModeChangeCallback_ = nullptr;
}

void PcFoldScreenController::MaskSupportEnterWaterfallMode()
{
    maskSupportEnterWaterfallMode_ = true;
    auto sceneSession = weakSceneSession_.promote();
    if (sceneSession == nullptr) {
        TLOGE(WmsLogTag::WMS_LAYOUT_PC, "session unavailable, id: %{public}d", GetPersistentId());
        return;
    }
    sceneSession->SetSupportEnterWaterfallMode(false);
}

void PcFoldScreenController::UpdateSupportEnterWaterfallMode()
{
    TLOGD(WmsLogTag::WMS_LAYOUT_PC, "last: %{public}d, curr: %{public}d",
        lastSupportEnterWaterfallMode_, supportEnterWaterfallMode_);
    if (maskSupportEnterWaterfallMode_ || (lastSupportEnterWaterfallMode_ == supportEnterWaterfallMode_)) {
        return;
    }
    auto sceneSession = weakSceneSession_.promote();
    if (sceneSession == nullptr) {
        TLOGE(WmsLogTag::WMS_LAYOUT_PC, "session unavailable, id: %{public}d", GetPersistentId());
        return;
    }
    lastSupportEnterWaterfallMode_ = supportEnterWaterfallMode_;
    sceneSession->SetSupportEnterWaterfallMode(supportEnterWaterfallMode_);
}

void PcFoldScreenController::ExecuteFullScreenWaterfallModeChangeCallback()
{
    if (fullScreenWaterfallModeChangeCallback_ == nullptr) {
        return;
    }
    fullScreenWaterfallModeChangeCallback_(isFullScreenWaterfallMode_);
    // notify client
    auto sceneSession = weakSceneSession_.promote();
    if (sceneSession == nullptr) {
        TLOGE(WmsLogTag::WMS_LAYOUT_PC, "session is nullptr, id: %{public}d", GetPersistentId());
        return;
    }
    if (sceneSession->sessionStage_ == nullptr) {
        TLOGE(WmsLogTag::WMS_LAYOUT_PC, "sessionStage is nullptr, id: %{public}d", GetPersistentId());
        return;
    }
    sceneSession->sessionStage_->SetFullScreenWaterfallMode(isFullScreenWaterfallMode_);

    // notify subSession client
    sceneSession->NotifySubSessionAcrossDisplaysChange(isFullScreenWaterfallMode_);
    sceneSession->NotifyFollowedParentWindowAcrossDisplaysChange(isFullScreenWaterfallMode_);
}

int32_t PcFoldScreenController::GetPersistentId() const
{
    return persistentId_;
}

DisplayId PcFoldScreenController::GetDisplayId()
{
    auto sceneSession = weakSceneSession_.promote();
    if (sceneSession == nullptr) {
        TLOGE(WmsLogTag::WMS_LAYOUT_PC, "session is nullptr, id: %{public}d", GetPersistentId());
        return SCREEN_ID_INVALID;
    }
    return sceneSession->GetScreenId();
}

/**
 * get height of title bar
 * @return: vp
 */
int32_t PcFoldScreenController::GetTitleHeight() const
{
    auto sceneSession = weakSceneSession_.promote();
    if (sceneSession == nullptr) {
        return MIN_DECOR_HEIGHT;
    }
    return std::clamp(sceneSession->GetCustomDecorHeight(), MIN_DECOR_HEIGHT, MAX_DECOR_HEIGHT);
}

WSRectF PcFoldScreenController::CalculateMovingVelocity()
{
    WSRectF velocity = { 0.0f, 0.0f, 0.0f, 0.0f };
    std::unique_lock<std::mutex> lock(moveMutex_);
    if (IsThrowSlipModeDirectly(startThrowSlipMode_)) {
        return startVelocity_;
    }
    uint32_t recordsSize = static_cast<uint32_t>(movingRectRecords_.size());
    if (recordsSize <= 1) {
        return velocity;
    }

    if (recordsSize >= 2) { // temp use 2 points
        auto rect0 = movingRectRecords_[0].second;
        auto rect1 = movingRectRecords_[recordsSize - 1].second;
        float duration = MathHelper::NonZero(TimeHelper::GetDuration(movingRectRecords_[0].first,
            movingRectRecords_[recordsSize - 1].first));
        if (!MathHelper::GreatNotEqual(duration, 0.0f)) {
            return velocity;
        }
        velocity.posX_ = static_cast<float>((rect1.posX_ - rect0.posX_) / static_cast<float>(duration));
        velocity.posY_ = static_cast<float>((rect1.posY_ - rect0.posY_) / static_cast<float>(duration));
        velocity.width_ = static_cast<float>((rect1.width_ - rect0.width_) / static_cast<float>(duration));
        velocity.height_ = static_cast<float>((rect1.height_ - rect0.height_) / static_cast<float>(duration));
        return velocity;
    }
    return velocity;
}

/**
 * The callback that is triggered when the screen fold status changes.
 */
void PcFoldScreenController::UpdateRect()
{
    auto sceneSession = weakSceneSession_.promote();
    if (sceneSession == nullptr) {
        TLOGE(WmsLogTag::WMS_LAYOUT_PC, "session is nullptr, id: %{public}d", GetPersistentId());
        return;
    }
    auto ret = sceneSession->NotifyClientToUpdateRect("ScreenFoldStatusChanged", nullptr);
    if (ret != WSError::WS_OK) {
        TLOGE(WmsLogTag::WMS_LAYOUT_PC, "NotifyClientToUpdateRect Fail, id: %{public}d", GetPersistentId());
    }
}
} // namespace OHOS::Rosen