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

#include "session/host/include/move_drag_controller.h"

#include <hitrace_meter.h>
#include <pointer_event.h>
#include <transaction/rs_transaction.h>
#include <ui/rs_surface_node.h>

#include <cinttypes>

#include "display_manager.h"
#include "input_manager.h"
#include "screen_session_manager_client/include/screen_session_manager_client.h"
#include "session/host/include/scene_persistent_storage.h"
#include "session/host/include/scene_session.h"
#include "session/host/include/session_utils.h"
#include "session_helper.h"
#include "window_helper.h"
#include "window_manager_hilog.h"
#include "wm_common_inner.h"

#ifdef RES_SCHED_ENABLE
#include "res_sched_client.h"
#include "res_type.h"
#endif

namespace OHOS::Rosen {
namespace {
constexpr HiviewDFX::HiLogLabel LABEL = {LOG_CORE, HILOG_DOMAIN_WINDOW, "MoveDragController"};
}

MoveDragController::MoveDragController(int32_t persistentId, WindowType winType)
{
    persistentId_ = persistentId;
    winType_ = winType;
}

void MoveDragController::OnConnect(ScreenId id)
{
    TLOGW(WmsLogTag::WMS_LAYOUT, "Moving or dragging is interrupt due to new screen %{public}" PRIu64 " connection.",
        id);
    moveDragIsInterrupted_ = true;
}

void MoveDragController::OnDisconnect(ScreenId id)
{
    TLOGW(WmsLogTag::WMS_LAYOUT, "Moving or dragging is interrupt due to screen %{public}" PRIu64 " disconnection.",
        id);
    moveDragIsInterrupted_ = true;
}

void MoveDragController::OnChange(ScreenId id)
{
    TLOGW(WmsLogTag::WMS_LAYOUT, "Moving or dragging is interrupt due to screen %{public}" PRIu64 " change.", id);
    moveDragIsInterrupted_ = true;
}

void MoveDragController::RegisterMoveDragCallback(const MoveDragCallback& callBack)
{
    moveDragCallback_ = callBack;
}

void MoveDragController::NotifyWindowInputPidChange(bool isServerPid)
{
    if (pidChangeCallback_) {
        pidChangeCallback_(persistentId_, isServerPid);
        WLOGFI("id: %{public}d, isServerPid:%{public}d", persistentId_, isServerPid);
    }
}

bool MoveDragController::HasPointDown()
{
    return hasPointDown_;
}

void MoveDragController::SetStartMoveFlag(bool flag)
{
    if (flag && (!hasPointDown_ || isStartDrag_)) {
        WLOGFD("StartMove, but has not pointed down or is dragging, hasPointDown_: %{public}d, isStartFlag: %{public}d",
            hasPointDown_, isStartDrag_);
        return;
    }
    NotifyWindowInputPidChange(flag);
    isStartMove_ = flag;
    ResSchedReportData(OHOS::ResourceSchedule::ResType::RES_TYPE_MOVE_WINDOW, flag);
    WLOGFI("SetStartMoveFlag, isStartMove_: %{public}d id:%{public}d", isStartMove_, persistentId_);
}

void MoveDragController::SetMovable(bool isMovable)
{
    isMovable_ = isMovable;
}

void MoveDragController::SetNotifyWindowPidChangeCallback(const NotifyWindowPidChangeCallback& callback)
{
    pidChangeCallback_ = callback;
}

bool MoveDragController::GetStartMoveFlag() const
{
    WLOGFD("GetStartMoveFlag, isStartMove_: %{public}d id:%{public}d", isStartMove_, persistentId_);
    return isStartMove_;
}

bool MoveDragController::GetStartDragFlag() const
{
    return isStartDrag_;
}

uint64_t MoveDragController::GetMoveDragStartDisplayId() const
{
    return moveDragStartDisplayId_;
}

uint64_t MoveDragController::GetMoveDragEndDisplayId() const
{
    return moveDragEndDisplayId_;
}

uint64_t MoveDragController::GetInitParentNodeId() const
{
    return initParentNodeId_;
}

std::set<uint64_t> MoveDragController::GetDisplayIdsDuringMoveDrag()
{
    std::lock_guard<std::mutex> lock(displayIdSetDuringMoveDragMutex_);
    return displayIdSetDuringMoveDrag_;
}

bool MoveDragController::GetMovable() const
{
    return isMovable_;
}

void MoveDragController::SetTargetRect(const WSRect& rect)
{
    moveDragProperty_.targetRect_ = rect;
}

WSRect MoveDragController::GetOriginalRect() const
{
    return moveDragProperty_.originalRect_;
}

WSRect MoveDragController::GetTargetRect(TargetRectCoordinate coordinate) const
{
    DisplayId relatedDisplayId = DISPLAY_ID_INVALID;
    switch (coordinate) {
        case TargetRectCoordinate::GLOBAL:
            return {moveDragProperty_.targetRect_.posX_ + originalDisplayOffsetX_,
                    moveDragProperty_.targetRect_.posY_ + originalDisplayOffsetY_,
                    moveDragProperty_.targetRect_.width_,
                    moveDragProperty_.targetRect_.height_};
        case TargetRectCoordinate::RELATED_TO_START_DISPLAY:
            return moveDragProperty_.targetRect_;
        case TargetRectCoordinate::RELATED_TO_END_DISPLAY:
            relatedDisplayId = moveDragEndDisplayId_;
            break;
        default:
            return moveDragProperty_.targetRect_;
    }
    return GetTargetRectByDisplayId(relatedDisplayId);
}

WSRect MoveDragController::GetTargetRectByDisplayId(DisplayId displayId) const
{
    sptr<ScreenSession> screenSession =
        ScreenSessionManagerClient::GetInstance().GetScreenSessionById(displayId);
    if (!screenSession) {
        TLOGW(WmsLogTag::WMS_LAYOUT, "Screen session is null, return relative coordinates.");
        return moveDragProperty_.targetRect_;
    }
    ScreenProperty screenProperty = screenSession->GetScreenProperty();
    int32_t currentDisplayOffsetX = static_cast<int32_t>(screenProperty.GetStartX());
    int32_t currentDisplayOffsetY = static_cast<int32_t>(screenProperty.GetStartY());
    return {moveDragProperty_.targetRect_.posX_ + originalDisplayOffsetX_ - currentDisplayOffsetX,
            moveDragProperty_.targetRect_.posY_ + originalDisplayOffsetY_ - currentDisplayOffsetY,
            moveDragProperty_.targetRect_.width_,
            moveDragProperty_.targetRect_.height_};
}

void MoveDragController::InitMoveDragProperty()
{
    moveDragProperty_ = {-1, -1, -1, -1, -1, -1, {0, 0, 0, 0}, {0, 0, 0, 0}};
}

void MoveDragController::SetMoveInputBarFlag(bool moveInputBarFlag)
{
    moveInputBarFlag_ = moveInputBarFlag;
}

bool MoveDragController::GetMoveInputBarFlag()
{
    return moveInputBarFlag_;
}

void MoveDragController::SetInputBarCrossScreen(bool crossScreen)
{
    inputBarCrossScreen_ = crossScreen;
}

bool MoveDragController::GetInputBarCrossScreen()
{
    return inputBarCrossScreen_;
}

void MoveDragController::InitCrossDisplayProperty(DisplayId displayId, uint64_t initParentNodeId)
{
    DMError error = ScreenManager::GetInstance().RegisterScreenListener(this);
    if (error != DMError::DM_OK) {
        TLOGW(WmsLogTag::WMS_LAYOUT, "Register ScreenListener false.");
    }
    {
        std::lock_guard<std::mutex> lock(displayIdSetDuringMoveDragMutex_);
        displayIdSetDuringMoveDrag_.insert(displayId);
    }
    moveDragStartDisplayId_ = displayId;
    initParentNodeId_ = initParentNodeId;
    sptr<ScreenSession> screenSession =
        ScreenSessionManagerClient::GetInstance().GetScreenSessionById(moveDragStartDisplayId_);
    if (!screenSession) {
        return;
    }
    ScreenProperty screenProperty = screenSession->GetScreenProperty();
    originalDisplayOffsetX_ = static_cast<int32_t>(screenProperty.GetStartX());
    originalDisplayOffsetY_ = static_cast<int32_t>(screenProperty.GetStartY());
    TLOGI(WmsLogTag::WMS_LAYOUT, "moveDragStartDisplayId: %{public}" PRIu64 ", "
        "originalDisplayOffsetX: %{public}d, originalDisplayOffsetY: %{public}d",
        moveDragStartDisplayId_, originalDisplayOffsetX_, originalDisplayOffsetY_);
}

void MoveDragController::ResetCrossMoveDragProperty()
{
    moveDragProperty_ = {-1, -1, -1, -1, -1, -1, {0, 0, 0, 0}, {0, 0, 0, 0}};
    DMError error = ScreenManager::GetInstance().UnregisterScreenListener(this);
    if (error != DMError::DM_OK) {
        TLOGW(WmsLogTag::WMS_LAYOUT, "Register ScreenListener false.");
    }
    {
        std::lock_guard<std::mutex> lock(displayIdSetDuringMoveDragMutex_);
        displayIdSetDuringMoveDrag_.clear();
    }
    moveDragStartDisplayId_ = DISPLAY_ID_INVALID;
    moveInputBarStartDisplayId_ = DISPLAY_ID_INVALID;
    moveDragEndDisplayId_ = DISPLAY_ID_INVALID;
    initParentNodeId_ = -1;
    originalDisplayOffsetX_ = 0;
    originalDisplayOffsetY_ = 0;
    moveDragIsInterrupted_ = false;
    moveInputBarFlag_ = false;
}

void MoveDragController::SetOriginalValue(int32_t pointerId, int32_t pointerType, int32_t pointerPosX,
    int32_t pointerPosY, int32_t pointerWindowX, int32_t pointerWindowY, const WSRect& winRect)
{
    moveDragProperty_.pointerId_ = pointerId;
    moveDragProperty_.pointerType_ = pointerType;
    moveDragProperty_.originalPointerPosX_ = pointerPosX;
    moveDragProperty_.originalPointerPosY_ = pointerPosY;
    moveDragProperty_.originalPointerWindowX_ = pointerWindowX;
    moveDragProperty_.originalPointerWindowY_ = pointerWindowY;
    moveDragProperty_.originalRect_ = winRect;
}

WSRect MoveDragController::GetFullScreenToFloatingRect(const WSRect& originalRect, const WSRect& windowRect)
{
    if (moveTempProperty_.isEmpty()) {
        TLOGI(WmsLogTag::WMS_LAYOUT, "move temporary property is empty");
        return originalRect;
    }
    if (originalRect.width_ == 0) {
        TLOGI(WmsLogTag::WMS_LAYOUT, "original rect witch is zero");
        return windowRect;
    }
    // Drag and drop to full screen in proportion
    float newPosX = static_cast<float>(windowRect.width_) / static_cast<float>(originalRect.width_) *
        static_cast<float>(moveTempProperty_.lastDownPointerPosX_ - originalRect.posX_);
    WSRect targetRect = {
        moveTempProperty_.lastDownPointerPosX_ - static_cast<int32_t>(newPosX),
        originalRect.posY_,
        windowRect.width_,
        windowRect.height_,
    };
    TLOGI(WmsLogTag::WMS_LAYOUT, "target rect [%{public}d,%{public}d,%{public}u,%{public}u]", targetRect.posX_,
        targetRect.posY_, targetRect.width_, targetRect.height_);
    return targetRect;
}

void MoveDragController::SetAspectRatio(float ratio)
{
    aspectRatio_ = ratio;
}

bool MoveDragController::ConsumeMoveEvent(const std::shared_ptr<MMI::PointerEvent>& pointerEvent,
    const WSRect& originalRect)
{
    if (pointerEvent == nullptr) {
        WLOGE("ConsumeMoveEvent stop because of nullptr");
        return false;
    }
    if (GetStartDragFlag()) {
        WLOGFI("the window is being resized");
        return false;
    }
    int32_t pointerId = pointerEvent->GetPointerId();
    int32_t startPointerId = moveDragProperty_.pointerId_;
    int32_t startPointerType = moveDragProperty_.pointerType_;
    if ((startPointerId != -1 && startPointerId != pointerId) ||
        (startPointerType != -1 && pointerEvent->GetSourceType() != startPointerType)) {
        WLOGFI("block unnecessary pointer event inside the window");
        return false;
    }
    MMI::PointerEvent::PointerItem pointerItem;
    int32_t sourceType = pointerEvent->GetSourceType();
    if (!pointerEvent->GetPointerItem(pointerId, pointerItem) ||
        (sourceType == MMI::PointerEvent::SOURCE_TYPE_MOUSE &&
         (pointerEvent->GetButtonId() != MMI::PointerEvent::MOUSE_BUTTON_LEFT && !GetStartMoveFlag()))) {
        WLOGFD("invalid pointerEvent id: %{public}d", persistentId_);
        return false;
    }

    UpdateMoveTempProperty(pointerEvent);

    int32_t action = pointerEvent->GetPointerAction();
    if (!GetStartMoveFlag()) {
        if (action == MMI::PointerEvent::POINTER_ACTION_DOWN ||
            action == MMI::PointerEvent::POINTER_ACTION_BUTTON_DOWN) {
            WLOGFD("Move event hasPointDown");
            hasPointDown_ = true;
        } else if (action == MMI::PointerEvent::POINTER_ACTION_UP ||
            action == MMI::PointerEvent::POINTER_ACTION_BUTTON_UP ||
            action == MMI::PointerEvent::POINTER_ACTION_CANCEL) {
            WLOGFD("Reset hasPointDown_ when point up or cancel");
            hasPointDown_ = false;
        }
        WLOGFD("No need to move action id: %{public}d", action);
        return false;
    }

    SizeChangeReason reason = SizeChangeReason::DRAG_MOVE;
    bool ret = true;
    switch (action) {
        case MMI::PointerEvent::POINTER_ACTION_MOVE: {
            if (moveDragIsInterrupted_) {
                MoveDragInterrupted();
                return true;
            }
            reason = SizeChangeReason::DRAG_MOVE;
            uint32_t oldWindowDragHotAreaType = windowDragHotAreaType_;
            UpdateHotAreaType(pointerEvent);
            ProcessWindowDragHotAreaFunc(oldWindowDragHotAreaType != windowDragHotAreaType_, reason);
            break;
        }
        case MMI::PointerEvent::POINTER_ACTION_UP:
        case MMI::PointerEvent::POINTER_ACTION_BUTTON_UP:
        case MMI::PointerEvent::POINTER_ACTION_CANCEL:
        case MMI::PointerEvent::POINTER_ACTION_DOWN:
        case MMI::PointerEvent::POINTER_ACTION_BUTTON_DOWN: {
            if (!hasPointDown_) {
                return true;
            }
            if (moveDragIsInterrupted_) {
                MoveDragInterrupted();
                return true;
            }
            reason = SizeChangeReason::DRAG_END;
            SetStartMoveFlag(false);
            hasPointDown_ = false;
            moveDragEndDisplayId_ = static_cast<uint64_t>(pointerEvent->GetTargetDisplayId());
            ProcessWindowDragHotAreaFunc(windowDragHotAreaType_ != WINDOW_HOT_AREA_TYPE_UNDEFINED, reason);
            // The Pointer up event sent to the ArkUI.
            ret = false;
            break;
        }
        default:
            break;
    }

    if (moveInputBarFlag_ && CalcMoveInputBarRect(pointerEvent, originalRect)) {
        ProcessSessionRectChange(reason);
        return ret;
    }

    if (CalcMoveTargetRect(pointerEvent, originalRect)) {
        ProcessSessionRectChange(reason);
    }
    return ret;
}

void MoveDragController::ProcessWindowDragHotAreaFunc(bool isSendHotAreaMessage, SizeChangeReason reason)
{
    if (isSendHotAreaMessage) {
        WLOGFI("ProcessWindowDragHotAreaFunc start, isSendHotAreaMessage: %{public}u, reason: %{public}d",
            isSendHotAreaMessage, reason);
    }
    if (windowDragHotAreaFunc_ && isSendHotAreaMessage) {
        windowDragHotAreaFunc_(hotAreaDisplayId_, windowDragHotAreaType_, reason);
    }
}

void MoveDragController::UpdateGravityWhenDrag(const std::shared_ptr<MMI::PointerEvent>& pointerEvent,
    const std::shared_ptr<RSSurfaceNode>& surfaceNode)
{
    if (surfaceNode == nullptr || pointerEvent == nullptr || type_ == AreaType::UNDEFINED) {
        return;
    }
    if (pointerEvent->GetPointerAction() == MMI::PointerEvent::POINTER_ACTION_DOWN ||
        pointerEvent->GetPointerAction() == MMI::PointerEvent::POINTER_ACTION_BUTTON_DOWN) {
        Gravity dragGravity = GRAVITY_MAP.at(type_);
        if (dragGravity >= Gravity::TOP && dragGravity <= Gravity::BOTTOM_RIGHT) {
            WLOGFI("begin SetFrameGravity:%{public}d, type:%{public}d", dragGravity, type_);
            surfaceNode->SetFrameGravity(dragGravity);
            RSTransaction::FlushImplicitTransaction();
        }
        return;
    }
    if (pointerEvent->GetPointerAction() == MMI::PointerEvent::POINTER_ACTION_BUTTON_UP ||
        pointerEvent->GetPointerAction() == MMI::PointerEvent::POINTER_ACTION_UP ||
        pointerEvent->GetPointerAction() == MMI::PointerEvent::POINTER_ACTION_CANCEL) {
        surfaceNode->SetFrameGravity(Gravity::TOP_LEFT);
        RSTransaction::FlushImplicitTransaction();
        WLOGFI("recover gravity to TOP_LEFT");
    }
}

void MoveDragController::CalcDragTargetRect(const std::shared_ptr<MMI::PointerEvent>& pointerEvent,
                                            SizeChangeReason reason)
{
    if (reason == SizeChangeReason::DRAG_START) {
        moveDragProperty_.targetRect_ = moveDragProperty_.originalRect_;
        TLOGD(WmsLogTag::WMS_LAYOUT, "drag rect:%{public}s", moveDragProperty_.targetRect_.ToString().c_str());
        return;
    }
    std::pair<int32_t, int32_t> trans = CalcUnifiedTranslate(pointerEvent);
    if (!WindowHelper::IsSystemWindow(winType_) ||
        static_cast<uint64_t>(pointerEvent->GetTargetDisplayId()) == moveDragStartDisplayId_) {
        moveDragProperty_.targetRect_ =
            MathHelper::GreatNotEqual(aspectRatio_, NEAR_ZERO) ?
            CalcFixedAspectRatioTargetRect(type_, trans.first, trans.second,
            aspectRatio_, moveDragProperty_.originalRect_):
            CalcFreeformTargetRect(type_, trans.first, trans.second, moveDragProperty_.originalRect_);
    }
    TLOGD(WmsLogTag::WMS_LAYOUT, "drag rect:%{public}s, tranX:%{public}d, tranY:%{public}d",
        moveDragProperty_.targetRect_.ToString().c_str(), trans.first, trans.second);
}

bool MoveDragController::ConsumeDragEvent(const std::shared_ptr<MMI::PointerEvent>& pointerEvent,
    const WSRect& originalRect, const sptr<WindowSessionProperty> property, const SystemSessionConfig& sysConfig)
{
    if (!CheckDragEventLegal(pointerEvent, property)) {
        return false;
    }
    int32_t pointerId = pointerEvent->GetPointerId();
    MMI::PointerEvent::PointerItem pointerItem;
    if (!pointerEvent->GetPointerItem(pointerId, pointerItem)) {
        WLOGE("Get PointerItem failed");
        return false;
    }
    SizeChangeReason reason = SizeChangeReason::UNDEFINED;
    switch (pointerEvent->GetPointerAction()) {
        case MMI::PointerEvent::POINTER_ACTION_BUTTON_DOWN:
        case MMI::PointerEvent::POINTER_ACTION_DOWN: {
            if (!EventDownInit(pointerEvent, originalRect, property, sysConfig)) {
                return false;
            }
            reason = SizeChangeReason::DRAG_START;
            ResSchedReportData(OHOS::ResourceSchedule::ResType::RES_TYPE_RESIZE_WINDOW, true);
            break;
        }
        case MMI::PointerEvent::POINTER_ACTION_MOVE: {
            if (moveDragIsInterrupted_) {
                MoveDragInterrupted();
                return true;
            }
            reason = SizeChangeReason::DRAG;
            break;
        }
        case MMI::PointerEvent::POINTER_ACTION_UP:
        case MMI::PointerEvent::POINTER_ACTION_BUTTON_UP:
        case MMI::PointerEvent::POINTER_ACTION_CANCEL: {
            if (!hasPointDown_) {
                return true;
            }
            auto screenRect = GetScreenRectById(moveDragStartDisplayId_);
            if (moveDragIsInterrupted_ || screenRect == WSRect{-1, -1, -1, -1}) {
                MoveDragInterrupted();
                return true;
            }
            reason = SizeChangeReason::DRAG_END;
            isStartDrag_ = false;
            hasPointDown_ = false;
            moveDragEndDisplayId_ = GetTargetRect(TargetRectCoordinate::GLOBAL).IsOverlap(screenRect) ?
                moveDragStartDisplayId_ : static_cast<uint64_t>(pointerEvent->GetTargetDisplayId());
            ResSchedReportData(OHOS::ResourceSchedule::ResType::RES_TYPE_RESIZE_WINDOW, false);
            NotifyWindowInputPidChange(isStartDrag_);
            break;
        }
        default:
            return false;
    }
    CalcDragTargetRect(pointerEvent, reason);
    ProcessSessionRectChange(reason);
    return true;
}

void MoveDragController::MoveDragInterrupted()
{
    TLOGI(WmsLogTag::WMS_LAYOUT, "Screen anomaly, MoveDrag has been interrupted.");
    SizeChangeReason reason = SizeChangeReason::DRAG_END;
    hasPointDown_ = false;
    if (isStartDrag_) {
        isStartDrag_ = false;
        ResSchedReportData(OHOS::ResourceSchedule::ResType::RES_TYPE_RESIZE_WINDOW, false);
        NotifyWindowInputPidChange(isStartDrag_);
    };
    if (GetStartMoveFlag()) {
        SetStartMoveFlag(false);
        ProcessWindowDragHotAreaFunc(windowDragHotAreaType_ != WINDOW_HOT_AREA_TYPE_UNDEFINED, reason);
    };
    moveDragEndDisplayId_ = moveDragStartDisplayId_;
    moveDragProperty_.targetRect_ = moveDragProperty_.originalRect_;
    ProcessSessionRectChange(reason);
}

WSRect MoveDragController::GetScreenRectById(DisplayId displayId)
{
    sptr<ScreenSession> screenSession = ScreenSessionManagerClient::GetInstance().GetScreenSessionById(displayId);
    if (!screenSession) {
        TLOGI(WmsLogTag::WMS_LAYOUT, "ScreenSession id null.");
        return WSRect{-1, -1, -1, -1};
    }
    ScreenProperty screenProperty = screenSession->GetScreenProperty();
    WSRect screenRect = {
        screenProperty.GetStartX(),
        screenProperty.GetStartY(),
        screenProperty.GetBounds().rect_.GetWidth(),
        screenProperty.GetBounds().rect_.GetHeight(),
    };
    return screenRect;
}

void MoveDragController::SetMoveAvailableArea(DMRect& area)
{
    moveAvailableArea_.posX_ = area.posX_;
    moveAvailableArea_.posY_ = area.posY_;
    moveAvailableArea_.width_ = area.width_;
    moveAvailableArea_.height_ = area.height_;
}

void MoveDragController::UpdateMoveAvailableArea(DisplayId targetDisplayId)
{
    DMRect availableArea;
    sptr<Display> display = Rosen::DisplayManager::GetInstance().GetDisplayById(targetDisplayId);
    if (display == nullptr) {
        TLOGD(WmsLogTag::WMS_KEYBOARD, "Failed to get display");
        return;
    }
    DMError ret = display->GetAvailableArea(availableArea);
    SetMoveAvailableArea(availableArea);
}

void MoveDragController::SetMoveInputBarStartDisplayId(DisplayId displayId)
{
    moveInputBarStartDisplayId_ = displayId;
}

DisplayId MoveDragController::GetMoveInputBarStartDisplayId()
{
    return moveInputBarStartDisplayId_;
}

void MoveDragController::SetCurrentScreenProperty(DisplayId targetDisplayId)
{
    sptr<ScreenSession> currentScreenSession =
        ScreenSessionManagerClient::GetInstance().GetScreenSessionById(targetDisplayId);
    if (currentScreenSession == nullptr) {
        TLOGW(WmsLogTag::WMS_KEYBOARD, "Screen session is null");
        return;
    }
    ScreenProperty currentScreenProperty = currentScreenSession->GetScreenProperty();
    screenSizeProperty_.currentDisplayStartX_ = currentScreenProperty.GetStartX();
    screenSizeProperty_.currentDisplayStartY_ = currentScreenProperty.GetStartY();
    screenSizeProperty_.currentDisplayX_ = currentScreenProperty.GetBounds().rect_.left_;
    screenSizeProperty_.currentDisplayY_ = currentScreenProperty.GetBounds().rect_.top_;
    screenSizeProperty_.width_ = currentScreenProperty.GetBounds().rect_.width_;
    screenSizeProperty_.height_ = currentScreenProperty.GetBounds().rect_.height_;
}

void MoveDragController::ResetCurrentScreenProperty()
{
    screenSizeProperty_.currentDisplayStartX_ = 0;
    screenSizeProperty_.currentDisplayStartY_ = 0;
    screenSizeProperty_.currentDisplayX_ = 0;
    screenSizeProperty_.currentDisplayY_ = 0;
    screenSizeProperty_.width_ = 0;
    screenSizeProperty_.height_ = 0;
}

std::pair<int32_t, int32_t> MoveDragController::CalcUnifiedTranslate(
    const std::shared_ptr<MMI::PointerEvent>& pointerEvent)
{
    int32_t pointerId = pointerEvent->GetPointerId();
    MMI::PointerEvent::PointerItem pointerItem;
    pointerEvent->GetPointerItem(pointerId, pointerItem);
    sptr<ScreenSession> screenSession = ScreenSessionManagerClient::GetInstance().GetScreenSessionById(
        static_cast<uint64_t>(pointerEvent->GetTargetDisplayId()));
    if (!screenSession) {
        return std::make_pair(0, 0);
    }
    ScreenProperty screenProperty = screenSession->GetScreenProperty();
    // calculate trans in unified coordinates
    int32_t currentDisplayTranX = static_cast<int32_t>(screenProperty.GetStartX());
    int32_t currentDisplayTranY = static_cast<int32_t>(screenProperty.GetStartY());
    int32_t tranX = (pointerItem.GetDisplayX() + currentDisplayTranX) -
        (moveDragProperty_.originalPointerPosX_ + originalDisplayOffsetX_);
    int32_t tranY = (pointerItem.GetDisplayY() + currentDisplayTranY) -
        (moveDragProperty_.originalPointerPosY_ + originalDisplayOffsetY_);
    return std::make_pair(tranX, tranY);
}

void MoveDragController::AdjustXYByAvailableArea(int32_t& x, int32_t& y)
{
    if (x <= moveAvailableArea_.posX_) {
        x = moveAvailableArea_.posX_;
    }

    if (x >= moveAvailableArea_.posX_ + moveAvailableArea_.width_ - moveDragProperty_.originalRect_.width_) {
        x = moveAvailableArea_.posX_ + moveAvailableArea_.width_ - moveDragProperty_.originalRect_.width_;
    }

    if (y <= moveAvailableArea_.posY_) {
        y = moveAvailableArea_.posY_;
    }

    if (y >= moveAvailableArea_.posY_ + moveAvailableArea_.height_ - moveDragProperty_.originalRect_.height_) {
        y = moveAvailableArea_.posY_ + moveAvailableArea_.height_ - moveDragProperty_.originalRect_.height_;
    }
}

MouseMoveDirection MoveDragController::CalcMouseMoveDirection(DisplayId lastDisplayId, DisplayId currentDisplayId)
{
    sptr<ScreenSession> lastScreenSession =
        ScreenSessionManagerClient::GetInstance().GetScreenSessionById(lastDisplayId);
    sptr<ScreenSession> currentScreenSession =
        ScreenSessionManagerClient::GetInstance().GetScreenSessionById(currentDisplayId);
    if (!lastScreenSession || !currentScreenSession) {
        TLOGW(WmsLogTag::WMS_KEYBOARD, "Screen session is null, return default mouse move direction.");
        return MouseMoveDirection::UNKNOWN;
    }

    ScreenProperty lastScreenProperty = lastScreenSession->GetScreenProperty();
    ScreenProperty currentScreenProperty = currentScreenSession->GetScreenProperty();

    int32_t lastOriginStartX = lastScreenProperty.GetStartX();
    int32_t lastOriginStartY = lastScreenProperty.GetStartY();
    int32_t currentOriginStartX = currentScreenProperty.GetStartX();
    int32_t currentOriginStartY = currentScreenProperty.GetStartY();

    uint32_t lastScreenWidth = lastScreenProperty.GetBounds().rect_.width_;
    uint32_t lastScreenHeight = lastScreenProperty.GetBounds().rect_.height_;
    uint32_t currentScreenWidth = currentScreenProperty.GetBounds().rect_.width_;
    uint32_t currentScreenHeight = currentScreenProperty.GetBounds().rect_.height_;

    if (currentOriginStartX == lastOriginStartX + lastScreenWidth) {
        return MouseMoveDirection::LEFT_TO_RIGHT;
    } else if (currentOriginStartX == lastOriginStartX - currentScreenWidth) {
        return MouseMoveDirection::RIGHT_TO_LEFT;
    } else if (currentOriginStartY == lastOriginStartY + lastScreenHeight) {
        return MouseMoveDirection::UP_TO_BOTTOM;
    } else if (currentOriginStartY == lastOriginStartY - currentScreenHeight) {
        return MouseMoveDirection::BOTTOM_TO_UP;
    }

    return MouseMoveDirection::UNKNOWN;
}

void MoveDragController::SetOriginalDisplayOffset(int32_t offsetX, int32_t offsetY)
{
    originalDisplayOffsetX_ = offsetX;
    originalDisplayOffsetY_ = offsetY;
}

void MoveDragController::SetInputBarCrossAttr(MouseMoveDirection mouseMoveDirection, DisplayId targetDisplayId)
{
    if (mouseMoveDirection == MouseMoveDirection::LEFT_TO_RIGHT ||
        mouseMoveDirection == MouseMoveDirection::RIGHT_TO_LEFT) {
        UpdateMoveAvailableArea(targetDisplayId);
    }
    SetInputBarCrossScreen(true);
    moveInputBarStartDisplayId_ = targetDisplayId;
    SetOriginalDisplayOffset(screenSizeProperty_.currentDisplayStartX_, screenSizeProperty_.currentDisplayStartY_);
    ResetCurrentScreenProperty();
}

void MoveDragController::InitializeMoveDragPropertyNotValid(const std::shared_ptr<MMI::PointerEvent>& pointerEvent,
                                                            const WSRect& originalRect)
{
    MMI::PointerEvent::PointerItem pointerItem;
    int32_t pointerId = pointerEvent->GetPointerId();
    pointerEvent->GetPointerItem(pointerId, pointerItem);

    int32_t pointerDisplayX = pointerItem.GetDisplayX();
    int32_t pointerDisplayY = pointerItem.GetDisplayY();
    moveDragProperty_.pointerId_ = pointerId;
    moveDragProperty_.pointerType_ = pointerEvent->GetSourceType();
    moveDragProperty_.originalPointerPosX_ = pointerDisplayX;
    moveDragProperty_.originalPointerPosY_ = pointerDisplayY;
    int32_t pointerWindowX = pointerItem.GetWindowX();
    int32_t pointerWindowY = pointerItem.GetWindowY();
    moveDragProperty_.originalRect_ = originalRect;
    moveDragProperty_.originalRect_.posX_ = pointerDisplayX - pointerWindowX;
    moveDragProperty_.originalRect_.posY_ = pointerDisplayY - pointerWindowY;
}

bool MoveDragController::CheckAndInitializeMoveDragProperty(const std::shared_ptr<MMI::PointerEvent>& pointerEvent,
                                                            const WSRect& originalRect)
{
    if (moveDragProperty_.isEmpty()) {
        InitializeMoveDragPropertyNotValid(pointerEvent, originalRect);
        return false;
    }
    return true;
}

void MoveDragController::HandleLeftToRightCross(int32_t pointerDisplayX,
                                                int32_t pointerDisplayY,
                                                int32_t& moveDragFinalX,
                                                int32_t& moveDragFinalY,
                                                DisplayId targetDisplayId)
{
    if (pointerDisplayX > moveDragProperty_.originalPointerWindowX_) {
        moveDragFinalX = pointerDisplayX - moveDragProperty_.originalPointerWindowX_;
    } else {
        moveDragFinalX = 0;
    }
    moveDragFinalY = pointerDisplayY - moveDragProperty_.originalPointerWindowY_;
    SetInputBarCrossAttr(MouseMoveDirection::LEFT_TO_RIGHT, targetDisplayId);
}

void MoveDragController::HandleRightToLeftCross(int32_t pointerDisplayX,
                                                int32_t pointerDisplayY,
                                                int32_t& moveDragFinalX,
                                                int32_t& moveDragFinalY,
                                                DisplayId targetDisplayId)
{
    if (pointerDisplayX <= screenSizeProperty_.width_ - moveDragProperty_.originalRect_.width_ +
                               moveDragProperty_.originalPointerWindowX_) {
        moveDragFinalX = pointerDisplayX - moveDragProperty_.originalPointerWindowX_;
    } else {
        moveDragFinalX = screenSizeProperty_.width_ - moveDragProperty_.originalRect_.width_;
    }
    moveDragFinalY = pointerDisplayY - moveDragProperty_.originalPointerWindowY_;
    SetInputBarCrossAttr(MouseMoveDirection::RIGHT_TO_LEFT, targetDisplayId);
}

void MoveDragController::HandleUpToBottomCross(int32_t pointerDisplayX,
                                               int32_t pointerDisplayY,
                                               int32_t& moveDragFinalX,
                                               int32_t& moveDragFinalY,
                                               DisplayId targetDisplayId)
{
    UpdateMoveAvailableArea(targetDisplayId);
    int32_t statusBarHeight = moveAvailableArea_.posY_ - screenSizeProperty_.currentDisplayY_;
    if (pointerDisplayY >= statusBarHeight + moveDragProperty_.originalPointerWindowY_) {
        moveDragFinalY = pointerDisplayY - moveDragProperty_.originalPointerWindowY_;
    } else {
        moveDragFinalY = statusBarHeight;
    }
    moveDragFinalX = pointerDisplayX - moveDragProperty_.originalPointerWindowX_;
    SetInputBarCrossAttr(MouseMoveDirection::UP_TO_BOTTOM, targetDisplayId);
}

void MoveDragController::HandleBottomToUpCross(int32_t pointerDisplayX,
                                               int32_t pointerDisplayY,
                                               int32_t& moveDragFinalX,
                                               int32_t& moveDragFinalY,
                                               DisplayId targetDisplayId)
{
    UpdateMoveAvailableArea(targetDisplayId);
    int32_t dockBarHeight =
        screenSizeProperty_.currentDisplayY_ - moveAvailableArea_.posY_ - moveAvailableArea_.height_;
    if (pointerDisplayY <= screenSizeProperty_.height_ - dockBarHeight - moveDragProperty_.originalPointerWindowY_) {
        moveDragFinalY = pointerDisplayY - moveDragProperty_.originalPointerWindowY_;
    } else {
        moveDragFinalY = screenSizeProperty_.height_ - dockBarHeight - moveDragProperty_.originalPointerWindowY_;
    }
    moveDragFinalX = pointerDisplayX - moveDragProperty_.originalPointerWindowX_;
    SetInputBarCrossAttr(MouseMoveDirection::BOTTOM_TO_UP, targetDisplayId);
}

void MoveDragController::CalcMoveForSameDisplay(const std::shared_ptr<MMI::PointerEvent>& pointerEvent,
                                                int32_t& moveDragFinalX,
                                                int32_t& moveDragFinalY)
{
    MMI::PointerEvent::PointerItem pointerItem;
    int32_t pointerId = pointerEvent->GetPointerId();
    pointerEvent->GetPointerItem(pointerId, pointerItem);

    int32_t pointerDisplayX = pointerItem.GetDisplayX();
    int32_t pointerDisplayY = pointerItem.GetDisplayY();

    moveDragFinalX = pointerDisplayX - moveDragProperty_.originalPointerWindowX_;
    moveDragFinalY = pointerDisplayY - moveDragProperty_.originalPointerWindowY_;
    AdjustXYByAvailableArea(moveDragFinalX, moveDragFinalY);
}

bool MoveDragController::CalcMoveInputBarRect(const std::shared_ptr<MMI::PointerEvent>& pointerEvent,
                                              const WSRect& originalRect)
{
    if (!CheckAndInitializeMoveDragProperty(pointerEvent, originalRect)) {
        return false;
    }

    MMI::PointerEvent::PointerItem pointerItem;
    int32_t pointerId = pointerEvent->GetPointerId();
    pointerEvent->GetPointerItem(pointerId, pointerItem);
    DisplayId targetDisplayId = static_cast<uint64_t>(pointerEvent->GetTargetDisplayId());
    int32_t moveDragFinalX = 0;
    int32_t moveDragFinalY = 0;
    int32_t pointerDisplayX = pointerItem.GetDisplayX();
    int32_t pointerDisplayY = pointerItem.GetDisplayY();

    if (targetDisplayId == moveInputBarStartDisplayId_) {
        CalcMoveForSameDisplay(pointerEvent, moveDragFinalX, moveDragFinalY);
    } else {
        MouseMoveDirection mouseMoveDirection =
            CalcMouseMoveDirection(moveInputBarStartDisplayId_, pointerEvent->GetTargetDisplayId());
        if (screenSizeProperty_.isEmpty()) {
            SetCurrentScreenProperty(targetDisplayId);
        }

        switch (mouseMoveDirection) {
            case MouseMoveDirection::LEFT_TO_RIGHT:
                HandleLeftToRightCross(
                    pointerDisplayX, pointerDisplayY, moveDragFinalX, moveDragFinalY, targetDisplayId);
                break;
            case MouseMoveDirection::RIGHT_TO_LEFT:
                HandleRightToLeftCross(
                    pointerDisplayX, pointerDisplayY, moveDragFinalX, moveDragFinalY, targetDisplayId);
                break;
            case MouseMoveDirection::UP_TO_BOTTOM:
                HandleUpToBottomCross(
                    pointerDisplayX, pointerDisplayY, moveDragFinalX, moveDragFinalY, targetDisplayId);
                break;
            case MouseMoveDirection::BOTTOM_TO_UP:
                HandleBottomToUpCross(
                    pointerDisplayX, pointerDisplayY, moveDragFinalX, moveDragFinalY, targetDisplayId);
                break;
            default:
                moveDragFinalX = moveDragProperty_.targetRect_.posX_;
                moveDragFinalY = moveDragProperty_.targetRect_.posY_;
                break;
        }
    }
    moveDragProperty_.targetRect_ = { moveDragFinalX, moveDragFinalY, originalRect.width_, originalRect.height_ };
    TLOGD(WmsLogTag::WMS_KEYBOARD, "move rect: %{public}s", moveDragProperty_.targetRect_.ToString().c_str());
    return true;
}

bool MoveDragController::CalcMoveTargetRect(const std::shared_ptr<MMI::PointerEvent>& pointerEvent,
    const WSRect& originalRect)
{
    MMI::PointerEvent::PointerItem pointerItem;
    int32_t pointerId = pointerEvent->GetPointerId();
    pointerEvent->GetPointerItem(pointerId, pointerItem);
    if (moveDragProperty_.isEmpty()) {
        int32_t pointerDisplayX = pointerItem.GetDisplayX();
        int32_t pointerDisplayY = pointerItem.GetDisplayY();
        moveDragProperty_.pointerId_ = pointerId;
        moveDragProperty_.pointerType_ = pointerEvent->GetSourceType();
        moveDragProperty_.originalPointerPosX_ = pointerDisplayX;
        moveDragProperty_.originalPointerPosY_ = pointerDisplayY;
        int32_t pointerWindowX = pointerItem.GetWindowX();
        int32_t pointerWindowY = pointerItem.GetWindowY();
        moveDragProperty_.originalRect_ = originalRect;
        moveDragProperty_.originalRect_.posX_ = pointerDisplayX - pointerWindowX;
        moveDragProperty_.originalRect_.posY_ = pointerDisplayY - pointerWindowY;
        return false;
    };
    if (!WindowHelper::IsSystemWindow(winType_) ||
        static_cast<uint64_t>(pointerEvent->GetTargetDisplayId()) == moveDragStartDisplayId_) {
        std::pair<int32_t, int32_t> trans = CalcUnifiedTranslate(pointerEvent);
        moveDragProperty_.targetRect_ = {
            moveDragProperty_.originalRect_.posX_ + trans.first,
            moveDragProperty_.originalRect_.posY_ + trans.second,
            originalRect.width_,
            originalRect.height_
        };
    }
    TLOGD(WmsLogTag::WMS_LAYOUT, "move rect: %{public}s", moveDragProperty_.targetRect_.ToString().c_str());
    return true;
}

bool MoveDragController::EventDownInit(const std::shared_ptr<MMI::PointerEvent>& pointerEvent,
    const WSRect& originalRect, const sptr<WindowSessionProperty> property, const SystemSessionConfig& sysConfig)
{
    const auto& sourceType = pointerEvent->GetSourceType();
    if (sourceType == MMI::PointerEvent::SOURCE_TYPE_MOUSE &&
        pointerEvent->GetButtonId() != MMI::PointerEvent::MOUSE_BUTTON_LEFT) {
        TLOGD(WmsLogTag::WMS_LAYOUT, "Mouse click event but not left click");
        return false;
    }
    HITRACE_METER_FMT(HITRACE_TAG_WINDOW_MANAGER, "MoveDragController::EventDownInit");
    int32_t pointerId = pointerEvent->GetPointerId();
    MMI::PointerEvent::PointerItem pointerItem;
    pointerEvent->GetPointerItem(pointerId, pointerItem);
    InitMoveDragProperty();
    hasPointDown_ = true;
    moveDragProperty_.originalRect_ = originalRect;
    auto display = DisplayManager::GetInstance().GetDisplayById(
        static_cast<uint64_t>(pointerEvent->GetTargetDisplayId()));
    if (display) {
        vpr_ = display->GetVirtualPixelRatio();
    } else {
        vpr_ = 1.5f;  // 1.5f: default virtual pixel ratio
    }
    int outside = (sourceType == MMI::PointerEvent::SOURCE_TYPE_MOUSE) ? HOTZONE_POINTER * vpr_ : HOTZONE_TOUCH * vpr_;
    type_ = SessionHelper::GetAreaType(pointerItem.GetWindowX(), pointerItem.GetWindowY(), sourceType, outside, vpr_,
        moveDragProperty_.originalRect_);
    if (type_ == AreaType::UNDEFINED) {
        return false;
    }
    InitDecorValue(property, sysConfig);
    limits_ = property->GetWindowLimits();
    moveDragProperty_.pointerId_ = pointerEvent->GetPointerId();
    moveDragProperty_.pointerType_ = sourceType;
    moveDragProperty_.originalPointerPosX_ = pointerItem.GetDisplayX();
    moveDragProperty_.originalPointerPosY_ = pointerItem.GetDisplayY();
    if (aspectRatio_ <= NEAR_ZERO) {
        CalcFreeformTranslateLimits(type_);
    }
    moveDragProperty_.originalRect_.posX_ = pointerItem.GetDisplayX() - pointerItem.GetWindowX();
    moveDragProperty_.originalRect_.posY_ = pointerItem.GetDisplayY() - pointerItem.GetWindowY();
    mainMoveAxis_ = AxisType::UNDEFINED;
    isStartDrag_ = true;
    NotifyWindowInputPidChange(isStartDrag_);
    return true;
}

WSRect MoveDragController::CalcFreeformTargetRect(AreaType type, int32_t tranX, int32_t tranY, WSRect originalRect)
{
    WSRect targetRect = originalRect;
    FixTranslateByLimits(tranX, tranY);
    TLOGD(WmsLogTag::WMS_LAYOUT, "areaType:%{public}u", type);
    if (static_cast<uint32_t>(type) & static_cast<uint32_t>(AreaType::LEFT)) {
        targetRect.posX_ += tranX;
        targetRect.width_ -= tranX;
    } else if (static_cast<uint32_t>(type) & static_cast<uint32_t>(AreaType::RIGHT)) {
        targetRect.width_ += tranX;
    }
    if (static_cast<uint32_t>(type) & static_cast<uint32_t>(AreaType::TOP)) {
        targetRect.posY_ += tranY;
        targetRect.height_ -= tranY;
    } else if (static_cast<uint32_t>(type) & static_cast<uint32_t>(AreaType::BOTTOM)) {
        targetRect.height_ += tranY;
    }
    // check current ratio limits
    if (targetRect.height_ == 0) {
        return targetRect;
    }
    float curRatio = static_cast<float>(targetRect.width_) / static_cast<float>(targetRect.height_);
    if (!MathHelper::GreatNotEqual(limits_.minRatio_, curRatio) &&
        !MathHelper::GreatNotEqual(curRatio, limits_.maxRatio_)) {
        return targetRect;
    }
    float newRatio = MathHelper::LessNotEqual(curRatio, limits_.minRatio_) ? limits_.minRatio_ : limits_.maxRatio_;
    if (MathHelper::NearZero(newRatio)) {
        return targetRect;
    }
    if ((static_cast<uint32_t>(type) & static_cast<uint32_t>(AreaType::LEFT)) ||
        (static_cast<uint32_t>(type) & static_cast<uint32_t>(AreaType::RIGHT))) {
        targetRect.height_ = static_cast<int32_t>(static_cast<float>(targetRect.width_) / newRatio);
    } else {
        targetRect.width_ = static_cast<int32_t>(static_cast<float>(targetRect.height_) * newRatio);
    }
    TLOGD(WmsLogTag::WMS_LAYOUT, "curRatio:%{public}f, newRatio:%{public}f", curRatio, newRatio);
    return targetRect;
}

WSRect MoveDragController::CalcFixedAspectRatioTargetRect(AreaType type, int32_t tranX, int32_t tranY,
    float aspectRatio, WSRect originalRect)
{
    TLOGD(WmsLogTag::WMS_LAYOUT, "in");
    int32_t posX = originalRect.posX_;
    int32_t posY = originalRect.posY_;
    int32_t width = static_cast<int32_t>(originalRect.width_);
    int32_t height = static_cast<int32_t>(originalRect.height_);
    if (mainMoveAxis_ == AxisType::UNDEFINED) {
        if (!InitMainAxis(type, tranX, tranY)) {
            return originalRect;
        }
    }

    TLOGD(WmsLogTag::WMS_LAYOUT, "ratio:%{public}f, areaType:%{public}u", aspectRatio, type);
    ConvertXYByAspectRatio(tranX, tranY, aspectRatio);
    FixTranslateByLimits(tranX, tranY);
    switch (type) {
        case AreaType::LEFT_TOP: {
            return {posX + tranX, posY + tranY, width - tranX, height - tranY};
        }
        case AreaType::RIGHT_TOP: {
            return {posX, posY + (mainMoveAxis_ == AxisType::X_AXIS ? (-tranY) : (tranY)),
                    width + (mainMoveAxis_ == AxisType::X_AXIS ? (tranX) : (-tranX)),
                    height + (mainMoveAxis_ == AxisType::X_AXIS ? (tranY) : (-tranY))};
        }
        case AreaType::RIGHT_BOTTOM: {
            return {posX, posY, width + tranX, height + tranY};
        }
        case AreaType::LEFT_BOTTOM: {
            return {posX + (mainMoveAxis_ == AxisType::X_AXIS ? (tranX) : (-tranX)), posY,
                    width - (mainMoveAxis_ == AxisType::X_AXIS ? (tranX) : (-tranX)),
                    height - (mainMoveAxis_ == AxisType::X_AXIS ? (tranY) : (-tranY))};
        }
        case AreaType::LEFT: {
            return {posX + tranX, posY, width - tranX, height - tranY};
        }
        case AreaType::TOP: {
            return {posX, posY + tranY, width - tranX, height - tranY};
        }
        case AreaType::RIGHT: {
            return {posX, posY, width + tranX, height + tranY};
        }
        case AreaType::BOTTOM: {
            return {posX, posY, width + tranX, height + tranY};
        }
        default:
            break;
    }
    return originalRect;
}

void MoveDragController::CalcFreeformTranslateLimits(AreaType type)
{
    TLOGD(WmsLogTag::WMS_LAYOUT, "areaType:%{public}u, minWidth:%{public}u, maxWidth:%{public}u, "
        "minHeight:%{public}u, maxHeight:%{public}u", type,
        limits_.minWidth_, limits_.maxWidth_, limits_.minHeight_, limits_.maxHeight_);
    if (static_cast<uint32_t>(type) & static_cast<uint32_t>(AreaType::LEFT)) {
        minTranX_ = moveDragProperty_.originalRect_.width_ - static_cast<int32_t>(limits_.maxWidth_);
        maxTranX_ = moveDragProperty_.originalRect_.width_ - static_cast<int32_t>(limits_.minWidth_);
    } else if (static_cast<uint32_t>(type) & static_cast<uint32_t>(AreaType::RIGHT)) {
        minTranX_ = static_cast<int32_t>(limits_.minWidth_) - moveDragProperty_.originalRect_.width_;
        maxTranX_ = static_cast<int32_t>(limits_.maxWidth_) - moveDragProperty_.originalRect_.width_;
    }
    if (static_cast<uint32_t>(type) & static_cast<uint32_t>(AreaType::TOP)) {
        minTranY_ = moveDragProperty_.originalRect_.height_ - static_cast<int32_t>(limits_.maxHeight_);
        maxTranY_ = moveDragProperty_.originalRect_.height_ - static_cast<int32_t>(limits_.minHeight_);
    } else if (static_cast<uint32_t>(type) & static_cast<uint32_t>(AreaType::BOTTOM)) {
        minTranY_ = static_cast<int32_t>(limits_.minHeight_) - moveDragProperty_.originalRect_.height_;
        maxTranY_ = static_cast<int32_t>(limits_.maxHeight_) - moveDragProperty_.originalRect_.height_;
    }
}

void MoveDragController::CalcFixedAspectRatioTranslateLimits(AreaType type)
{
    int32_t minW = static_cast<int32_t>(limits_.minWidth_);
    int32_t maxW = static_cast<int32_t>(limits_.maxWidth_);
    int32_t minH = static_cast<int32_t>(limits_.minHeight_);
    int32_t maxH = static_cast<int32_t>(limits_.maxHeight_);
    if (isDecorEnable_) {
        if (SessionUtils::ToLayoutWidth(minW, vpr_) < SessionUtils::ToLayoutHeight(minH, vpr_) * aspectRatio_) {
            minW = SessionUtils::ToWinWidth(SessionUtils::ToLayoutHeight(minH, vpr_) * aspectRatio_, vpr_);
            minH = SessionUtils::ToLayoutHeight(minH, vpr_);
        } else {
            minH = SessionUtils::ToWinHeight(SessionUtils::ToLayoutWidth(minW, vpr_) / aspectRatio_, vpr_);
            minW = SessionUtils::ToLayoutWidth(minW, vpr_);
        }
        if (SessionUtils::ToLayoutWidth(maxW, vpr_) < SessionUtils::ToLayoutHeight(maxH, vpr_) * aspectRatio_) {
            maxH = SessionUtils::ToWinHeight(SessionUtils::ToLayoutWidth(maxW, vpr_) / aspectRatio_, vpr_);
            maxW = SessionUtils::ToLayoutWidth(maxW, vpr_);
        } else {
            maxW = SessionUtils::ToWinWidth(SessionUtils::ToLayoutHeight(maxH, vpr_) * aspectRatio_, vpr_);
            maxH = SessionUtils::ToLayoutHeight(maxH, vpr_);
        }
    } else {
        // width = height * aspectRatio
        if (minW < minH * aspectRatio_) {
            minW = minH * aspectRatio_;
        } else {
            minH = minW / aspectRatio_;
        }
        if (maxW < maxH * aspectRatio_) {
            maxH = maxW / aspectRatio_;
        } else {
            maxW = maxH * aspectRatio_;
        }
    }
    if (static_cast<uint32_t>(type) & static_cast<uint32_t>(AreaType::LEFT)) {
        minTranX_ = static_cast<int32_t>(moveDragProperty_.originalRect_.width_) - maxW;
        maxTranX_ = static_cast<int32_t>(moveDragProperty_.originalRect_.width_) - minW;
        minTranY_ = minH - static_cast<int32_t>(moveDragProperty_.originalRect_.height_);
        maxTranY_ = maxH - static_cast<int32_t>(moveDragProperty_.originalRect_.height_);
    } else if (static_cast<uint32_t>(type) & static_cast<uint32_t>(AreaType::RIGHT)) {
        minTranX_ = minW - static_cast<int32_t>(moveDragProperty_.originalRect_.width_);
        maxTranX_ = maxW - static_cast<int32_t>(moveDragProperty_.originalRect_.width_);
        minTranY_ = minH - static_cast<int32_t>(moveDragProperty_.originalRect_.height_);
        maxTranY_ = maxH - static_cast<int32_t>(moveDragProperty_.originalRect_.height_);
    } else if (static_cast<uint32_t>(type) & static_cast<uint32_t>(AreaType::TOP)) {
        minTranX_ = minW - static_cast<int32_t>(moveDragProperty_.originalRect_.width_);
        maxTranX_ = maxW - static_cast<int32_t>(moveDragProperty_.originalRect_.width_);
        minTranY_ = static_cast<int32_t>(moveDragProperty_.originalRect_.height_) - maxH;
        maxTranY_ = static_cast<int32_t>(moveDragProperty_.originalRect_.height_) - minH;
    } else if (static_cast<uint32_t>(type) & static_cast<uint32_t>(AreaType::BOTTOM)) {
        minTranX_ = minW - static_cast<int32_t>(moveDragProperty_.originalRect_.width_);
        maxTranX_ = maxW - static_cast<int32_t>(moveDragProperty_.originalRect_.width_);
        minTranY_ = minH - static_cast<int32_t>(moveDragProperty_.originalRect_.height_);
        maxTranY_ = maxH - static_cast<int32_t>(moveDragProperty_.originalRect_.height_);
    }
}

void MoveDragController::FixTranslateByLimits(int32_t& tranX, int32_t& tranY)
{
    if (tranX < minTranX_) {
        tranX = minTranX_;
    } else if (tranX > maxTranX_) {
        tranX = maxTranX_;
    }
    if (tranY < minTranY_) {
        tranY = minTranY_;
    } else if (tranY > maxTranY_) {
        tranY = maxTranY_;
    }
    TLOGD(WmsLogTag::WMS_LAYOUT, "tranX:%{public}d, tranY:%{public}d, minTranX:%{public}d, maxTranX:%{public}d, "
        "minTranY:%{public}d, maxTranY:%{public}d", tranX, tranY, minTranX_, maxTranX_, minTranY_, maxTranY_);
}

bool MoveDragController::InitMainAxis(AreaType type, int32_t tranX, int32_t tranY)
{
    if (type == AreaType::LEFT || type == AreaType::RIGHT) {
        mainMoveAxis_ = AxisType::X_AXIS;
    } else if (type == AreaType::TOP || type == AreaType::BOTTOM) {
        mainMoveAxis_ = AxisType::Y_AXIS;
    } else if (tranX == 0 && tranY == 0) {
        return false;
    } else {
        mainMoveAxis_ = (std::abs(tranX) > std::abs(tranY)) ? AxisType::X_AXIS : AxisType::Y_AXIS;
    }
    CalcFixedAspectRatioTranslateLimits(type);
    return true;
}

void MoveDragController::ConvertXYByAspectRatio(int32_t& tx, int32_t& ty, float aspectRatio)
{
    if (mainMoveAxis_ == AxisType::X_AXIS) {
        ty = tx / aspectRatio;
    } else if (mainMoveAxis_ == AxisType::Y_AXIS) {
        tx = ty * aspectRatio;
    }
    return;
}

void MoveDragController::InitDecorValue(const sptr<WindowSessionProperty> property,
    const SystemSessionConfig& sysConfig)
{
    auto windowType = property->GetWindowType();
    bool isMainWindow = WindowHelper::IsMainWindow(windowType);
    bool isSubWindow = WindowHelper::IsSubWindow(windowType);
    bool isDialogWindow = WindowHelper::IsDialogWindow(windowType);
    isDecorEnable_ = (isMainWindow || ((isSubWindow || isDialogWindow) && property->IsDecorEnable())) &&
        sysConfig.isSystemDecorEnable_ &&
        WindowHelper::IsWindowModeSupported(sysConfig.decorWindowModeSupportType_, property->GetWindowMode());
}

void MoveDragController::ProcessSessionRectChange(SizeChangeReason reason)
{
    if (moveDragCallback_) {
        moveDragCallback_(reason);
    }
}

float MoveDragController::GetVirtualPixelRatio() const
{
    float vpr = 1.5;
    auto displayInfo = DisplayManager::GetInstance().GetDefaultDisplay();
    if (displayInfo != nullptr) {
        vpr = displayInfo->GetVirtualPixelRatio();
    }
    WLOGFD("vpr: %{public}f", vpr);
    return vpr;
}

void MoveDragController::UpdateDragType(int32_t startPointPosX, int32_t startPointPosY)
{
    if (startPointPosX > rectExceptCorner_.posX_ &&
        (startPointPosX < rectExceptCorner_.posX_ + static_cast<int32_t>(rectExceptCorner_.width_))) {
        dragType_ = DragType::DRAG_BOTTOM_OR_TOP;
    } else if (startPointPosY > rectExceptCorner_.posY_ &&
        (startPointPosY < rectExceptCorner_.posY_ + static_cast<int32_t>(rectExceptCorner_.height_))) {
        dragType_ = DragType::DRAG_LEFT_OR_RIGHT;
    } else if ((startPointPosX <= rectExceptCorner_.posX_ && startPointPosY <= rectExceptCorner_.posY_) ||
        (startPointPosX >= rectExceptCorner_.posX_ + static_cast<int32_t>(rectExceptCorner_.width_) &&
        startPointPosY >= rectExceptCorner_.posY_ + static_cast<int32_t>(rectExceptCorner_.height_))) {
        dragType_ = DragType::DRAG_LEFT_TOP_CORNER;
    } else {
        dragType_ = DragType::DRAG_RIGHT_TOP_CORNER;
    }
}

bool MoveDragController::IsPointInDragHotZone(int32_t startPointPosX, int32_t startPointPosY, int32_t sourceType,
    const WSRect& winRect)
{
    // calculate rect with hotzone
    Rect rectWithHotzone;
    rectWithHotzone.posX_ = winRect.posX_ - static_cast<int32_t>(HOTZONE_POINTER);
    rectWithHotzone.posY_ = winRect.posY_ - static_cast<int32_t>(HOTZONE_POINTER);
    rectWithHotzone.width_ = winRect.width_ + HOTZONE_POINTER * 2u;    // double hotZone
    rectWithHotzone.height_ = winRect.height_ + HOTZONE_POINTER * 2u;  // double hotZone

    if (sourceType == MMI::PointerEvent::SOURCE_TYPE_MOUSE &&
        !WindowHelper::IsPointInTargetRectWithBound(startPointPosX, startPointPosY, rectWithHotzone)) {
        return false;
    } else if ((!WindowHelper::IsPointInTargetRect(startPointPosX, startPointPosY, rectExceptFrame_)) ||
        (!WindowHelper::IsPointInWindowExceptCorner(startPointPosX, startPointPosY, rectExceptCorner_))) {
        return true;
    }
    return false;
}

void MoveDragController::CalculateStartRectExceptHotZone(float vpr, const WSRect& winRect)
{
    rectExceptFrame_.posX_ = winRect.posX_ + static_cast<int32_t>(WINDOW_FRAME_WIDTH * vpr);
    rectExceptFrame_.posY_ = winRect.posY_ + static_cast<int32_t>(WINDOW_FRAME_WIDTH * vpr);
    rectExceptFrame_.width_ = winRect.width_ - static_cast<uint32_t>((WINDOW_FRAME_WIDTH + WINDOW_FRAME_WIDTH) * vpr);
    rectExceptFrame_.height_ = winRect.height_ - static_cast<uint32_t>((WINDOW_FRAME_WIDTH + WINDOW_FRAME_WIDTH) * vpr);

    rectExceptCorner_.posX_ = winRect.posX_ + static_cast<int32_t>(WINDOW_FRAME_CORNER_WIDTH * vpr);
    rectExceptCorner_.posY_ = winRect.posY_ + static_cast<int32_t>(WINDOW_FRAME_CORNER_WIDTH * vpr);
    rectExceptCorner_.width_ =
        winRect.width_ - static_cast<uint32_t>((WINDOW_FRAME_CORNER_WIDTH + WINDOW_FRAME_CORNER_WIDTH) * vpr);
    rectExceptCorner_.height_ =
        winRect.height_ - static_cast<uint32_t>((WINDOW_FRAME_CORNER_WIDTH + WINDOW_FRAME_CORNER_WIDTH) * vpr);
}

WSError MoveDragController::UpdateMoveTempProperty(const std::shared_ptr<MMI::PointerEvent>& pointerEvent)
{
    int32_t pointerId = pointerEvent->GetPointerId();
    int32_t startPointerId = moveTempProperty_.pointerId_;
    int32_t pointerType = pointerEvent->GetSourceType();
    int32_t startPointerType = moveDragProperty_.pointerType_;
    MMI::PointerEvent::PointerItem pointerItem;
    int32_t sourceType = pointerEvent->GetSourceType();
    if (!pointerEvent->GetPointerItem(pointerId, pointerItem) ||
        (sourceType == MMI::PointerEvent::SOURCE_TYPE_MOUSE &&
        pointerEvent->GetButtonId() != MMI::PointerEvent::MOUSE_BUTTON_LEFT)) {
        WLOGFW("invalid pointerEvent");
        return WSError::WS_ERROR_NULLPTR;
    }

    int32_t pointerDisplayX = pointerItem.GetDisplayX();
    int32_t pointerDisplayY = pointerItem.GetDisplayY();
    int32_t pointerDisplayWindowX = pointerItem.GetWindowX();
    int32_t pointerDisplayWindowY = pointerItem.GetWindowY();
    switch (pointerEvent->GetPointerAction()) {
        case MMI::PointerEvent::POINTER_ACTION_BUTTON_DOWN:
        case MMI::PointerEvent::POINTER_ACTION_DOWN:
            moveTempProperty_.pointerId_ = pointerId;
            moveTempProperty_.pointerType_ = pointerType;
            moveTempProperty_.lastDownPointerPosX_ = pointerDisplayX;
            moveTempProperty_.lastDownPointerPosY_ = pointerDisplayY;
            moveTempProperty_.lastMovePointerPosX_ = pointerDisplayX;
            moveTempProperty_.lastMovePointerPosY_ = pointerDisplayY;
            moveTempProperty_.lastDownPointerWindowX_ = pointerItem.GetWindowX();
            moveTempProperty_.lastDownPointerWindowY_ = pointerItem.GetWindowY();
            break;
        case MMI::PointerEvent::POINTER_ACTION_MOVE:
            if ((startPointerId != -1 && startPointerId != pointerId) ||
                (startPointerType != -1 && pointerType != startPointerType)) {
                WLOGFI("block unnecessary pointer event inside the window");
                return WSError::WS_DO_NOTHING;
            }
            moveTempProperty_.lastMovePointerPosX_ = pointerDisplayX;
            moveTempProperty_.lastMovePointerPosY_ = pointerDisplayY;
            break;
        case MMI::PointerEvent::POINTER_ACTION_UP:
        case MMI::PointerEvent::POINTER_ACTION_BUTTON_UP:
        case MMI::PointerEvent::POINTER_ACTION_CANCEL: {
            moveTempProperty_ = {-1, -1, -1, -1, -1, -1, -1, -1};
            break;
        }
        default:
            break;
    }
    return WSError::WS_OK;
}

void MoveDragController::CalcFirstMoveTargetRect(const WSRect& windowRect, bool isFullToFloating)
{
    if (!GetStartMoveFlag() || moveTempProperty_.isEmpty()) {
        return;
    }

    WSRect originalRect = {
        moveTempProperty_.lastDownPointerPosX_ - moveTempProperty_.lastDownPointerWindowX_,
        moveTempProperty_.lastDownPointerPosY_ - moveTempProperty_.lastDownPointerWindowY_,
        windowRect.width_,
        windowRect.height_
    };
    if (isFullToFloating) {
        originalRect.posX_ = windowRect.posX_;
        originalRect.posY_ = windowRect.posY_;
    }
    SetOriginalValue(moveTempProperty_.pointerId_,
                     moveTempProperty_.pointerType_,
                     moveTempProperty_.lastDownPointerPosX_,
                     moveTempProperty_.lastDownPointerPosY_,
                     moveTempProperty_.lastDownPointerWindowX_,
                     moveTempProperty_.lastDownPointerWindowY_,
                     originalRect);

    int32_t offsetX = moveTempProperty_.lastMovePointerPosX_ - moveTempProperty_.lastDownPointerPosX_;
    int32_t offsetY = moveTempProperty_.lastMovePointerPosY_ - moveTempProperty_.lastDownPointerPosY_;
    WSRect targetRect = {
        originalRect.posX_ + offsetX,
        originalRect.posY_ + offsetY,
        originalRect.width_,
        originalRect.height_
    };
    TLOGI(WmsLogTag::WMS_LAYOUT, "first move rect: [%{public}d, %{public}d, %{public}u, %{public}u]", targetRect.posX_,
        targetRect.posY_, targetRect.width_, targetRect.height_);
    moveDragProperty_.targetRect_ = targetRect;
    ProcessSessionRectChange(SizeChangeReason::DRAG_MOVE);
}

bool MoveDragController::CheckDragEventLegal(const std::shared_ptr<MMI::PointerEvent>& pointerEvent,
    const sptr<WindowSessionProperty> property)
{
    if (pointerEvent == nullptr || property == nullptr) {
        WLOGE("ConsumeDragEvent stop because of nullptr");
        return false;
    }
    if (GetStartMoveFlag()) {
        WLOGFD("the window is being moved");
        return false;
    }
    if (!GetStartDragFlag() && pointerEvent->GetPointerAction() != MMI::PointerEvent::POINTER_ACTION_DOWN &&
        pointerEvent->GetPointerAction() != MMI::PointerEvent::POINTER_ACTION_BUTTON_DOWN) {
        return false;
    }
    int32_t pointerId = pointerEvent->GetPointerId();
    int32_t startPointerId = moveDragProperty_.pointerId_;
    if (GetStartDragFlag() && startPointerId != -1 && startPointerId != pointerId) {
        WLOGFI("block unnecessary pointer event inside the window");
        return false;
    }
    return true;
}

void MoveDragController::UpdateHotAreaType(const std::shared_ptr<MMI::PointerEvent>& pointerEvent)
{
    int32_t pointerId = pointerEvent->GetPointerId();
    MMI::PointerEvent::PointerItem pointerItem;
    if (!pointerEvent->GetPointerItem(pointerId, pointerItem)) {
        WLOGFW("invalid pointerEvent");
        return;
    }
    int32_t pointerDisplayX = pointerItem.GetDisplayX();
    int32_t pointerDisplayY = pointerItem.GetDisplayY();
    DisplayId displayId = static_cast<uint64_t>(pointerEvent->GetTargetDisplayId());
    uint32_t windowDragHotAreaType = SceneSession::GetWindowDragHotAreaType(displayId, WINDOW_HOT_AREA_TYPE_UNDEFINED,
        pointerDisplayX, pointerDisplayY);
    if (windowDragHotAreaType_ != windowDragHotAreaType) {
        WLOGFI("the pointerEvent is window drag hot area, old type is: %{public}d, new type is: %{public}d",
            windowDragHotAreaType_, windowDragHotAreaType);
    }
    if (hotAreaDisplayId_ != displayId) {
        TLOGI(WmsLogTag::WMS_LAYOUT, "displayId is changed, old: %{public}" PRIu64 ", new: %{public}" PRIu64,
            moveDragStartDisplayId_, displayId);
        hotAreaDisplayId_ = displayId;
    }
    windowDragHotAreaType_ = windowDragHotAreaType;
}

int32_t MoveDragController::GetOriginalPointerPosX()
{
    return moveDragProperty_.originalPointerPosX_;
}

int32_t MoveDragController::GetOriginalPointerPosY()
{
    return moveDragProperty_.originalPointerPosY_;
}

void MoveDragController::SetWindowDragHotAreaFunc(const NotifyWindowDragHotAreaFunc& func)
{
    windowDragHotAreaFunc_ = func;
}

void MoveDragController::OnLostFocus()
{
    TLOGW(WmsLogTag::WMS_LAYOUT, "window id %{public}d lost focus, should stop MoveDrag isMove: %{public}d,"
        "isDrag: %{public}d", persistentId_, isStartMove_, isStartDrag_);
    moveDragIsInterrupted_ = true;
}

std::set<uint64_t> MoveDragController::GetNewAddedDisplayIdsDuringMoveDrag()
{
    std::set<uint64_t> newAddedDisplayIdSet;
    WSRect windowRect = GetTargetRect(TargetRectCoordinate::GLOBAL);
    std::map<ScreenId, ScreenProperty> screenProperties =
        ScreenSessionManagerClient::GetInstance().GetAllScreensProperties();
    std::lock_guard<std::mutex> lock(displayIdSetDuringMoveDragMutex_);
    for (const auto& [screenId, screenProperty] : screenProperties) {
        if (displayIdSetDuringMoveDrag_.find(screenId) != displayIdSetDuringMoveDrag_.end()) {
            continue;
        }
        WSRect screenRect = {
            screenProperty.GetStartX(),
            screenProperty.GetStartY(),
            screenProperty.GetBounds().rect_.GetWidth(),
            screenProperty.GetBounds().rect_.GetHeight(),
        };
        if (windowRect.IsOverlap(screenRect)) {
            displayIdSetDuringMoveDrag_.insert(screenId);
            newAddedDisplayIdSet.insert(screenId);
        }
    }
    return newAddedDisplayIdSet;
}

void MoveDragController::ResSchedReportData(int32_t type, bool onOffTag)
{
#ifdef RES_SCHED_ENABLE
    std::unordered_map<std::string, std::string> payload;
    // 0 is start, 1 is end
    if (onOffTag) {
        OHOS::ResourceSchedule::ResSchedClient::GetInstance().ReportData(type, 0, payload);
    } else {
        OHOS::ResourceSchedule::ResSchedClient::GetInstance().ReportData(type, 1, payload);
    }
    WLOGFD("ResSchedReportData success type: %{public}d onOffTag: %{public}d", type, onOffTag);
#endif
}
}  // namespace OHOS::Rosen
