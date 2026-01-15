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

#include "move_drag_controller.h"

#include <algorithm>
#include <cinttypes>
#include <optional>

#include "hitrace_meter.h"
#include "input_manager.h"
#include "parameters.h"
#include "pointer_event.h"
#include "transaction/rs_transaction.h"
#include "ui/rs_surface_node.h"

#ifdef RES_SCHED_ENABLE
#include "res_sched_client.h"
#include "res_type.h"
#endif

#include "display_manager.h"
#include "rs_adapter.h"
#include "scene_persistent_storage.h"
#include "scene_session.h"
#include "screen_session_manager_client.h"
#include "session_helper.h"
#include "session_utils.h"
#include "window_helper.h"
#include "window_manager_hilog.h"
#include "wm_common_inner.h"

#define RETURN_IF_NULL_WITH_FUNC(param, funcName, ...)                                         \
    do {                                                                                       \
        if (!param) {                                                                          \
            TLOGNE(WmsLogTag::WMS_LAYOUT, "%{public}s: %{public}s is null", funcName, #param); \
            return __VA_ARGS__;                                                                \
        }                                                                                      \
    } while (0)

#define RETURN_IF_NULL(param, ...) RETURN_IF_NULL_WITH_FUNC(param, __func__, __VA_ARGS__)

namespace OHOS::Rosen {
namespace {
// The system parameter keys for move resampling configuration.
constexpr const char* MOVE_RESAMPLE_ENABLE_PARAM_KEY = "persist.windowlayout.moveresample.enable";
constexpr const char* MOVE_RESAMPLE_MIN_FPS_PARAM_KEY = "persist.windowlayout.moveresample.minfps";
constexpr const char* MOVE_RESAMPLE_MAX_FPS_PARAM_KEY = "persist.windowlayout.moveresample.maxfps";
}

const std::map<DragType, uint32_t> STYLEID_MAP = {
    {DragType::DRAG_UNDEFINED,        MMI::MOUSE_ICON::DEFAULT},
    {DragType::DRAG_BOTTOM_OR_TOP,    MMI::MOUSE_ICON::NORTH_SOUTH},
    {DragType::DRAG_LEFT_OR_RIGHT,    MMI::MOUSE_ICON::WEST_EAST},
    {DragType::DRAG_LEFT_TOP_CORNER,  MMI::MOUSE_ICON::NORTH_WEST_SOUTH_EAST},
    {DragType::DRAG_RIGHT_TOP_CORNER, MMI::MOUSE_ICON::NORTH_EAST_SOUTH_WEST}
};

const std::map<AreaType, Gravity> GRAVITY_MAP = {
    {AreaType::LEFT,          Gravity::TOP_RIGHT},
    {AreaType::TOP,           Gravity::BOTTOM_LEFT},
    {AreaType::RIGHT,         Gravity::TOP_LEFT},
    {AreaType::BOTTOM,        Gravity::TOP_LEFT},
    {AreaType::LEFT_TOP,      Gravity::BOTTOM_RIGHT},
    {AreaType::RIGHT_TOP,     Gravity::BOTTOM_LEFT},
    {AreaType::RIGHT_BOTTOM,  Gravity::TOP_LEFT},
    {AreaType::LEFT_BOTTOM,   Gravity::TOP_RIGHT}
};

/**
 * @brief Get the display's offset in the legacy global coordinate system
 *        (also known as the unified coordinate system).
 *
 * In this coordinate system, (0, 0) corresponds to the top-left corner of the
 * minimum bounding rectangle that covers all displays. The returned offset
 * represents the display's top-left position within this unified coordinate space.
 *
 * @param displayId The ID of the display whose offset is requested.
 * @return std::optional<std::pair<int32_t, int32_t>>
 *         (x, y) offset of the display in the legacy global coordinate system,
 *         or std::nullopt if the display session is unavailable.
 */
std::optional<std::pair<int32_t, int32_t>> GetLegacyGlobalDisplayOffset(DisplayId displayId)
{
    auto session = ScreenSessionManagerClient::GetInstance().GetScreenSessionById(displayId);
    RETURN_IF_NULL(session, std::nullopt);

    const auto prop = session->GetScreenProperty();
    int32_t x = static_cast<int32_t>(prop.GetStartX());
    int32_t y = static_cast<int32_t>(prop.GetStartY());
    return std::make_pair(x, y);
}

/**
 * @brief Retrieve the PointerItem associated with the current pointerId from the given event.
 *
 * @param event The pointer event from which to extract the PointerItem.
 * @return std::optional<MMI::PointerEvent::PointerItem>
 *         The retrieved PointerItem, or std::nullopt if unavailable.
 */
std::optional<MMI::PointerEvent::PointerItem> GetPointerItem(const std::shared_ptr<MMI::PointerEvent>& event)
{
    RETURN_IF_NULL(event, std::nullopt);
    const int32_t pointerId = event->GetPointerId();
    MMI::PointerEvent::PointerItem item;
    if (!event->GetPointerItem(pointerId, item)) {
        TLOGE(WmsLogTag::WMS_LAYOUT, "Failed to get pointer item, pointerId: %{public}d", pointerId);
        return std::nullopt;
    }
    return item;
}

/**
 * @brief Check whether the pointerEvent's ID and source type match
 *        the starting values stored in property.
 *
 * @param event    The pointer event to check.
 * @param property The property containing the starting pointer info.
 * @return true if they match; false otherwise.
 */
bool IsMatchingStartPointer(const std::shared_ptr<MMI::PointerEvent>& event,
                            int32_t startPointerId,
                            int32_t startPointerType)
{
    RETURN_IF_NULL(event, false);

    if (startPointerType != -1 && startPointerType != event->GetSourceType()) {
        TLOGD(WmsLogTag::WMS_LAYOUT,
              "Pointer type mismatch, startPointerType: %{public}d, eventSourceType: %{public}d",
              startPointerType, event->GetSourceType());
        return false;
    }

    auto item = GetPointerItem(event);
    RETURN_IF_NULL(item, false);
    if (startPointerId != -1 && startPointerId != item->GetOriginPointerId()) {
        TLOGD(WmsLogTag::WMS_LAYOUT,
              "Pointer ID mismatch, startPointerId: %{public}d, eventOriginPointerId: %{public}d",
              startPointerId, item->GetOriginPointerId());
        return false;
    }
    return true;
}

MoveDragController::MoveDragController(wptr<SceneSession> sceneSession) : sceneSession_(sceneSession)
{
    if (auto session = sceneSession.promote()) {
        persistentId_ = session->GetPersistentId();
        winType_ = session->GetWindowType();
    }

    // Move Resampling System Config
    std::tie(enableMoveResample_, resampleMinFps_, resampleMaxFps_) = MoveDragController::GetMoveResampleSystemConfig();
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

void MoveDragController::NotifyWindowInputPidChange(bool isServerPid)
{
    if (pidChangeCallback_) {
        pidChangeCallback_(persistentId_, isServerPid);
        TLOGI(WmsLogTag::WMS_LAYOUT, "id:%{public}d, isServerPid:%{public}d", persistentId_, isServerPid);
    }
}

bool MoveDragController::HasPointDown()
{
    return hasPointDown_;
}

void MoveDragController::SetStartMoveFlag(bool flag)
{
    if (flag && (!hasPointDown_ || isStartDrag_)) {
        TLOGD(WmsLogTag::WMS_LAYOUT, "StartMove, but has not pointed down or is dragging, hasPointDown:%{public}d, "
            "isStartFlag:%{public}d", hasPointDown_, isStartDrag_);
        ClearSpecifyMoveStartDisplay();
        return;
    }
    NotifyWindowInputPidChange(flag);
    isStartMove_ = flag;
    if (!isStartMove_) {
        ClearSpecifyMoveStartDisplay();
    }
    ResSchedReportData(OHOS::ResourceSchedule::ResType::RES_TYPE_MOVE_WINDOW, flag);
    TLOGI(WmsLogTag::WMS_LAYOUT, "isStartMove:%{public}d, id:%{public}d", isStartMove_, persistentId_);
}

void MoveDragController::SetStartDragFlag(bool flag)
{
    isStartDrag_ = flag;
    TLOGI(WmsLogTag::WMS_LAYOUT, "isStartDrag:%{public}d, id:%{public}d", isStartDrag_, persistentId_);
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
    TLOGD(WmsLogTag::WMS_LAYOUT, "isStartMove:%{public}d, id:%{public}d", isStartMove_, persistentId_);
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

void MoveDragController::UpdateTargetRect(SizeChangeReason reason, std::optional<WSRect> rectOpt)
{
    moveDragProperty_.targetRectChangeReason_ = reason;
    if (rectOpt) {
        moveDragProperty_.targetRect_ = *rectOpt;
        TLOGD(WmsLogTag::WMS_LAYOUT, "Update targetRect: %{public}s, reason: %{public}u",
              moveDragProperty_.targetRect_.ToString().c_str(), static_cast<uint32_t>(reason));
    } else {
        TLOGD(WmsLogTag::WMS_LAYOUT, "Only update reason: %{public}u", static_cast<uint32_t>(reason));
    }
}

WSRect MoveDragController::GetTargetRect(TargetRectCoordinate coordinate) const
{
    switch (coordinate) {
        case TargetRectCoordinate::GLOBAL:
            return moveDragProperty_.targetRect_.WithOffset(originalDisplayOffsetX_, originalDisplayOffsetY_);
        case TargetRectCoordinate::RELATED_TO_START_DISPLAY:
            return moveDragProperty_.targetRect_;
        case TargetRectCoordinate::RELATED_TO_END_DISPLAY:
            return GetTargetRectByDisplayId(moveDragEndDisplayId_);
        default:
            return moveDragProperty_.targetRect_;
    }
}

WSRect MoveDragController::GetTargetRectByDisplayId(DisplayId displayId) const
{
    return MapRectFromStartToTarget(moveDragProperty_.targetRect_, displayId);
}

WSRect MoveDragController::MapRectFromStartToTarget(const WSRect& relativeStartRect, DisplayId targetDisplayId) const
{
    auto targetDisplayOffset = GetLegacyGlobalDisplayOffset(targetDisplayId);
    RETURN_IF_NULL(targetDisplayOffset, relativeStartRect);

    auto [targetDisplayOffsetX, targetDisplayOffsetY] = *targetDisplayOffset;
    int32_t offsetX = originalDisplayOffsetX_ - targetDisplayOffsetX;
    int32_t offsetY = originalDisplayOffsetY_ - targetDisplayOffsetY;
    return relativeStartRect.WithOffset(offsetX, offsetY);
}

WSRect MoveDragController::MapRectFromTargetToStart(const WSRect& relativeTargetRect, DisplayId targetDisplayId) const
{
    auto targetDisplayOffset = GetLegacyGlobalDisplayOffset(targetDisplayId);
    RETURN_IF_NULL(targetDisplayOffset, relativeTargetRect);

    auto [targetDisplayOffsetX, targetDisplayOffsetY] = *targetDisplayOffset;
    int32_t offsetX = targetDisplayOffsetX - originalDisplayOffsetX_;
    int32_t offsetY = targetDisplayOffsetY - originalDisplayOffsetY_;
    return relativeTargetRect.WithOffset(offsetX, offsetY);
}

void MoveDragController::UpdateSubWindowGravityWhenFollow(const sptr<MoveDragController>& followedController,
    const std::shared_ptr<RSSurfaceNode>& surfaceNode)
{
    RETURN_IF_NULL(surfaceNode);
    RETURN_IF_NULL(followedController);
    auto type = followedController->GetAreaType();
    if (type == AreaType::UNDEFINED) {
        TLOGI(WmsLogTag::WMS_LAYOUT, "type undefined");
        return;
    }
    Gravity dragGravity = GRAVITY_MAP.at(type);
    if (dragGravity >= Gravity::TOP && dragGravity <= Gravity::BOTTOM_RIGHT) {
        TLOGI(WmsLogTag::WMS_LAYOUT, "begin SetFrameGravity when follow, gravity:%{public}d, type:%{public}d",
            dragGravity, type);
        surfaceNode->SetFrameGravity(dragGravity);
        RSTransactionAdapter::FlushImplicitTransaction(surfaceNode);
    }
}

/** @note @window.drag */
void MoveDragController::InitMoveDragProperty()
{
    auto lastScaleX = moveDragProperty_.scaleX_;
    auto lastScaleY = moveDragProperty_.scaleY_;
    moveDragProperty_.Reset();
    moveDragProperty_.scaleX_ = lastScaleX;
    moveDragProperty_.scaleY_ = lastScaleY;

    moveResampler_.Reset();

    // Sync properties from SceneSession
    SyncPropertiesFromSceneSession();
}

void MoveDragController::InitCrossDisplayProperty(DisplayId displayId, uint64_t initParentNodeId)
{
    DMError error = ScreenManager::GetInstance().RegisterScreenListener(this);
    if (error != DMError::DM_OK) {
        TLOGW(WmsLogTag::WMS_LAYOUT,
              "Failed to register ScreenListener, windowId: %{public}d, error: %{public}d",
              persistentId_, error);
    }
    {
        std::lock_guard<std::mutex> lock(displayIdSetDuringMoveDragMutex_);
        displayIdSetDuringMoveDrag_.insert(displayId);
    }
    initParentNodeId_ = initParentNodeId;
    moveDragStartDisplayId_ = displayId;
    auto offset = GetLegacyGlobalDisplayOffset(moveDragStartDisplayId_);
    RETURN_IF_NULL(offset);
    std::tie(originalDisplayOffsetX_, originalDisplayOffsetY_) = *offset;
    TLOGI(WmsLogTag::WMS_LAYOUT,
          "moveDragStartDisplayId: %{public}" PRIu64 ", originalDisplayOffset: [%{public}d, %{public}d]",
          moveDragStartDisplayId_, originalDisplayOffsetX_, originalDisplayOffsetY_);
}

/** @note @window.drag */
void MoveDragController::ResetCrossMoveDragProperty()
{
    moveDragProperty_.Reset();
    DMError error = ScreenManager::GetInstance().UnregisterScreenListener(this);
    if (error != DMError::DM_OK) {
        TLOGW(WmsLogTag::WMS_LAYOUT,
              "Failed to unregister ScreenListener, windowId: %{public}d, error: %{public}d",
              persistentId_, error);
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
    parentRect_ = {0, 0, 0, 0};
    ClearSpecifyMoveStartDisplay();
}

void MoveDragController::SetOriginalMoveDragPos(int32_t pointerId, int32_t pointerType, int32_t pointerPosX,
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

/** @note @window.drag */
WSRect MoveDragController::GetFullScreenToFloatingRect(const WSRect& originalRect, const WSRect& windowRect)
{
    if (moveTempProperty_.isEmpty()) {
        TLOGI(WmsLogTag::WMS_LAYOUT, "move temporary property is empty");
        return originalRect;
    }
    if (originalRect.width_ == 0) {
        TLOGI(WmsLogTag::WMS_LAYOUT, "original rect width is zero");
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
    TLOGI(WmsLogTag::WMS_LAYOUT, "target rect:%{public}s", targetRect.ToString().c_str());
    return targetRect;
}

bool MoveDragController::IsValidMoveEvent(const std::shared_ptr<MMI::PointerEvent>& pointerEvent)
{
    RETURN_IF_NULL(pointerEvent, false);

    // Reject events if the window is currently being resized.
    if (GetStartDragFlag()) {
        TLOGI(WmsLogTag::WMS_LAYOUT, "Invalid event: window is being resized");
        return false;
    }

    // Ensure the event corresponds to the initial pointer used for moving.
    if (!IsMatchingStartPointer(pointerEvent, moveDragProperty_.pointerId_, moveDragProperty_.pointerType_)) {
        TLOGW(WmsLogTag::WMS_LAYOUT, "Invalid event: pointer ID/type mismatch");
        return false;
    }

    // For a new move operation, ignore invalid mouse events
    if (!GetStartMoveFlag() && IsInvalidMouseEvent(pointerEvent)) {
        TLOGD(WmsLogTag::WMS_LAYOUT, "Invalid event: invalid pointer event, windowId: %{public}d", persistentId_);
        return false;
    }
    return true;
}

void MoveDragController::ProcessMoveRectUpdate(const std::shared_ptr<MMI::PointerEvent>& pointerEvent,
                                               SizeChangeReason reason)
{
    auto originalRect = CallSceneSession(&SceneSession::GetMoveRectForWindowDrag, WSRect::EMPTY_RECT);
    // NOTE: Special handling for input windows; Intrusive modification, should be optimized.
    if (WindowHelper::IsInputWindow(winType_) && CalcMoveInputBarRect(pointerEvent, originalRect, reason)) {
        ModifyWindowCoordinates(pointerEvent);
        OnMoveDragCallback(reason, TargetRectUpdateMode::UPDATED_IMMEDIATELY);
        return;
    }

    if (moveDragProperty_.isEmpty()) {
        InitializeMoveDragPropertyNotValid(pointerEvent, originalRect);
        return;
    }

    auto mode = UpdateTargetRectOnMoveEvent(pointerEvent, reason);
    ModifyWindowCoordinates(pointerEvent);
    OnMoveDragCallback(reason, mode);
}

bool MoveDragController::HandleMoving(const std::shared_ptr<MMI::PointerEvent>& pointerEvent)
{
    if (moveDragIsInterrupted_) {
        MoveDragInterrupted();
        return true;
    }
    uint32_t oldWindowDragHotAreaType = windowDragHotAreaType_;
    moveDragEndDisplayId_ = static_cast<DisplayId>(pointerEvent->GetTargetDisplayId());
    UpdateHotAreaType(pointerEvent);
    ProcessWindowDragHotAreaFunc(oldWindowDragHotAreaType != windowDragHotAreaType_, SizeChangeReason::DRAG_MOVE);
    ProcessMoveRectUpdate(pointerEvent, SizeChangeReason::DRAG_MOVE);
    return true;
}

bool MoveDragController::HandleMoveEnd(const std::shared_ptr<MMI::PointerEvent>& pointerEvent)
{
    if (!hasPointDown_) {
        return true;
    }
    if (moveDragIsInterrupted_) {
        MoveDragInterrupted();
        return true;
    }
    SetStartMoveFlag(false);
    hasPointDown_ = false;
    moveDragEndDisplayId_ = static_cast<DisplayId>(pointerEvent->GetTargetDisplayId());
    UpdateHotAreaType(pointerEvent);
    ProcessWindowDragHotAreaFunc(windowDragHotAreaType_ != WINDOW_HOT_AREA_TYPE_UNDEFINED, SizeChangeReason::DRAG_END);
    ProcessMoveRectUpdate(pointerEvent, SizeChangeReason::DRAG_END);
    // The Pointer up event sent to the ArkUI.
    return false;
}

bool MoveDragController::ConsumeMoveEvent(const std::shared_ptr<MMI::PointerEvent>& pointerEvent)
{
    if (!IsValidMoveEvent(pointerEvent)) {
        return false;
    }

    UpdateMoveTempProperty(pointerEvent);

    int32_t action = pointerEvent->GetPointerAction();
    if (!GetStartMoveFlag()) {
        if (action == MMI::PointerEvent::POINTER_ACTION_DOWN ||
            action == MMI::PointerEvent::POINTER_ACTION_BUTTON_DOWN) {
            TLOGD(WmsLogTag::WMS_LAYOUT, "Move event hasPointDown");
            hasPointDown_ = true;
        } else if (action == MMI::PointerEvent::POINTER_ACTION_UP ||
                   action == MMI::PointerEvent::POINTER_ACTION_BUTTON_UP ||
                   action == MMI::PointerEvent::POINTER_ACTION_CANCEL) {
            TLOGD(WmsLogTag::WMS_LAYOUT, "Reset hasPointDown_ when point up or cancel");
            hasPointDown_ = false;
        }
        TLOGD(WmsLogTag::WMS_LAYOUT, "No need to move action:%{public}d, id:%{public}d", action, persistentId_);
        return false;
    }

    switch (action) {
        case MMI::PointerEvent::POINTER_ACTION_MOVE:
            return HandleMoving(pointerEvent);
        case MMI::PointerEvent::POINTER_ACTION_UP:
        case MMI::PointerEvent::POINTER_ACTION_BUTTON_UP:
        case MMI::PointerEvent::POINTER_ACTION_CANCEL:
        case MMI::PointerEvent::POINTER_ACTION_DOWN:
        case MMI::PointerEvent::POINTER_ACTION_BUTTON_DOWN:
            return HandleMoveEnd(pointerEvent);
        default:
            return false;
    }
}

/** @note @window.drag */
void MoveDragController::ModifyWindowCoordinates(const std::shared_ptr<MMI::PointerEvent>& pointerEvent)
{
    auto pointerItemOpt = GetPointerItem(pointerEvent);
    RETURN_IF_NULL(pointerItemOpt);
    auto& pointerItem = *pointerItemOpt;

    int32_t action = pointerEvent->GetPointerAction();
    if (action == MMI::PointerEvent::POINTER_ACTION_BUTTON_UP || action == MMI::PointerEvent::POINTER_ACTION_MOVE) {
        auto targetRect = GetTargetRectByDisplayId(pointerEvent->GetTargetDisplayId());
        int32_t windowX = pointerItem.GetDisplayX() - targetRect.posX_;
        int32_t windowY = pointerItem.GetDisplayY() - targetRect.posY_;
        TLOGD(WmsLogTag::WMS_EVENT, "move end position: windowX:%{private}d windowY:%{private}d action:%{public}d",
            windowX, windowY, action);
        pointerItem.SetWindowX(windowX);
        pointerItem.SetWindowY(windowY);
        pointerEvent->AddPointerItem(pointerItem);
    }
}

/** @note @window.drag */
void MoveDragController::ProcessWindowDragHotAreaFunc(bool isSendHotAreaMessage, SizeChangeReason reason)
{
    if (isSendHotAreaMessage) {
        TLOGI(WmsLogTag::WMS_LAYOUT, "start, isSendHotAreaMessage:%{public}u, reason:%{public}d",
            isSendHotAreaMessage, reason);
    }
    if (windowDragHotAreaFunc_ && isSendHotAreaMessage) {
        windowDragHotAreaFunc_(hotAreaDisplayId_, windowDragHotAreaType_, reason);
    }
}

/** @note @window.drag */
void MoveDragController::UpdateGravityWhenDrag(const std::shared_ptr<MMI::PointerEvent>& pointerEvent,
                                               const std::shared_ptr<RSSurfaceNode>& surfaceNode)
{
    RETURN_IF_NULL(surfaceNode);
    RETURN_IF_NULL(pointerEvent);
    if (type_ == AreaType::UNDEFINED) {
        TLOGE(WmsLogTag::WMS_LAYOUT, "Undefined area type");
        return;
    }
    auto actionType = pointerEvent->GetPointerAction();
    if (actionType == MMI::PointerEvent::POINTER_ACTION_DOWN ||
        actionType == MMI::PointerEvent::POINTER_ACTION_BUTTON_DOWN) {
        preDragGravity_ = surfaceNode->GetStagingProperties().GetFrameGravity();
        Gravity dragGravity = GetGravity(type_);
        surfaceNode->SetFrameGravity(dragGravity);
        RSTransactionAdapter::FlushImplicitTransaction(surfaceNode);
        TLOGI(WmsLogTag::WMS_LAYOUT,
              "windowId: %{public}d, areaType: %{public}u, preDragGravity: %{public}d, dragGravity: %{public}d",
              persistentId_, static_cast<uint32_t>(type_), static_cast<int32_t>(preDragGravity_.value()),
              static_cast<int32_t>(dragGravity));
    }
}

bool MoveDragController::ShouldBlockCrossDisplay(const std::shared_ptr<MMI::PointerEvent>& pointerEvent) const
{
    const DisplayId targetDisplayId = static_cast<DisplayId>(pointerEvent->GetTargetDisplayId());
    if (targetDisplayId != moveDragStartDisplayId_ && !supportCrossDisplay_) {
        TLOGD(WmsLogTag::WMS_LAYOUT,
              "Cross display is not supported, startDisplayId:%{public}" PRIu64 ", targetDisplayId:%{public}" PRIu64,
              moveDragStartDisplayId_, targetDisplayId);
        return true;
    }
    return false;
}

/** @note @window.drag */
TargetRectUpdateMode MoveDragController::UpdateTargetRectOnDragEvent(
    const std::shared_ptr<MMI::PointerEvent>& pointerEvent, SizeChangeReason reason)
{
    if (ShouldBlockCrossDisplay(pointerEvent)) {
        return TargetRectUpdateMode::NONE;
    }

    const auto [offsetX, offsetY] = ComputeOffsetFromStart(pointerEvent);
    TLOGD(WmsLogTag::WMS_LAYOUT, "offsetX: %{public}d, offsetY: %{public}d", offsetX, offsetY);

    auto targetRect = !MathHelper::NearZero(aspectRatio_) ?
        CalcFixedAspectRatioTargetRect(type_, offsetX, offsetY, aspectRatio_, moveDragProperty_.originalRect_) :
        CalcFreeformTargetRect(type_, offsetX, offsetY, moveDragProperty_.originalRect_);
    UpdateTargetRect(reason, targetRect);
    return TargetRectUpdateMode::UPDATED_IMMEDIATELY;
}

Gravity MoveDragController::GetGravity() const
{
    return GetGravity(dragAreaType_);
}

Gravity MoveDragController::GetGravity(AreaType type) const
{
    auto iter = GRAVITY_MAP.find(type);
    if (iter == GRAVITY_MAP.end()) {
        TLOGE(WmsLogTag::WMS_LAYOUT, "No corresponding gravity found, type %{public}u", static_cast<uint32_t>(type));
        return Gravity::TOP_LEFT;
    }
    return iter->second;
}

bool MoveDragController::RestoreToPreDragGravity(const std::shared_ptr<RSSurfaceNode>& surfaceNode)
{
    RETURN_IF_NULL(surfaceNode, false);
    if (!preDragGravity_.has_value()) {
        TLOGE(WmsLogTag::WMS_LAYOUT, "No preDragGravity to restore");
        return false;
    }
    Gravity currentGravity = surfaceNode->GetStagingProperties().GetFrameGravity();
    surfaceNode->SetFrameGravity(preDragGravity_.value());
    RSTransactionAdapter::FlushImplicitTransaction(surfaceNode);
    TLOGI(WmsLogTag::WMS_LAYOUT,
          "windowId: %{public}d, currentGravity: %{public}d, restore preDragGravity: %{public}d",
          persistentId_, static_cast<int32_t>(currentGravity), static_cast<int32_t>(preDragGravity_.value()));
    preDragGravity_.reset();
    return true;
}

bool MoveDragController::IsValidDragEvent(const std::shared_ptr<MMI::PointerEvent>& pointerEvent)
{
    RETURN_IF_NULL(pointerEvent, false);

    // Reject events if the window is currently being moved.
    if (GetStartMoveFlag()) {
        TLOGD(WmsLogTag::WMS_LAYOUT, "Invalid event: window is being moved");
        return false;
    }

    // For a new drag operation, only DOWN actions are valid.
    const auto action = pointerEvent->GetPointerAction();
    if (!GetStartDragFlag() &&
        action != MMI::PointerEvent::POINTER_ACTION_DOWN &&
        action != MMI::PointerEvent::POINTER_ACTION_BUTTON_DOWN) {
        TLOGD(WmsLogTag::WMS_LAYOUT, "Invalid event: invalid start action");
        return false;
    }

    // For an ongoing drag, ensure pointer ID and type remain consistent.
    if (GetStartDragFlag() &&
        !IsMatchingStartPointer(pointerEvent, moveDragProperty_.pointerId_, moveDragProperty_.pointerType_)) {
        TLOGW(WmsLogTag::WMS_LAYOUT, "Invalid event: pointer ID/type mismatch");
        return false;
    }
    return true;
}

/** @note @window.drag */
bool MoveDragController::ConsumeDragEvent(const std::shared_ptr<MMI::PointerEvent>& pointerEvent)
{
    if (!IsValidDragEvent(pointerEvent)) {
        return false;
    }
    const auto action = pointerEvent->GetPointerAction();
    switch (action) {
        case MMI::PointerEvent::POINTER_ACTION_BUTTON_DOWN:
        case MMI::PointerEvent::POINTER_ACTION_DOWN:
            return HandleDragStart(pointerEvent);
        case MMI::PointerEvent::POINTER_ACTION_MOVE:
            return HandleDragging(pointerEvent);
        case MMI::PointerEvent::POINTER_ACTION_UP:
        case MMI::PointerEvent::POINTER_ACTION_BUTTON_UP:
        case MMI::PointerEvent::POINTER_ACTION_CANCEL:
            return HandleDragEnd(pointerEvent);
        default:
            return false;
    }
}

bool MoveDragController::HandleDragStart(const std::shared_ptr<MMI::PointerEvent>& pointerEvent)
{
    if (!EventDownInit(pointerEvent)) {
        return false;
    }

    ResSchedReportData(OHOS::ResourceSchedule::ResType::RES_TYPE_RESIZE_WINDOW, true);

    moveDragProperty_.originalRect_ = SessionUtils::AdjustRectByAspectRatio(
        moveDragProperty_.originalRect_, limits_, decoration_, aspectRatio_);

    TLOGD(WmsLogTag::WMS_LAYOUT, "Drag start, originalRect: %{public}s",
          moveDragProperty_.originalRect_.ToString().c_str());

    UpdateTargetRectOnDragEvent(pointerEvent, SizeChangeReason::DRAG_START);
    OnMoveDragCallback(SizeChangeReason::DRAG_START, TargetRectUpdateMode::UPDATED_IMMEDIATELY);
    return true;
}

bool MoveDragController::HandleDragging(const std::shared_ptr<MMI::PointerEvent>& pointerEvent)
{
    if (moveDragIsInterrupted_) {
        MoveDragInterrupted();
        return true;
    }

    UpdateTargetRectOnDragEvent(pointerEvent, SizeChangeReason::DRAG);
    OnMoveDragCallback(SizeChangeReason::DRAG, TargetRectUpdateMode::UPDATED_IMMEDIATELY);
    return true;
}

bool MoveDragController::HandleDragEnd(const std::shared_ptr<MMI::PointerEvent>& pointerEvent)
{
    if (!hasPointDown_) {
        return true;
    }

    const auto screenRect = GetScreenRectById(moveDragStartDisplayId_);
    if (moveDragIsInterrupted_ || screenRect == WSRect{ -1, -1, -1, -1 }) {
        MoveDragInterrupted();
        return true;
    }

    SetStartDragFlag(false);
    hasPointDown_ = false;

    // Determine final displayId
    const bool isInOriginalDisplay = GetTargetRect(TargetRectCoordinate::GLOBAL).IsOverlap(screenRect);

    moveDragEndDisplayId_ = isInOriginalDisplay ?
        moveDragStartDisplayId_ : static_cast<DisplayId>(pointerEvent->GetTargetDisplayId());

    ResSchedReportData(OHOS::ResourceSchedule::ResType::RES_TYPE_RESIZE_WINDOW, false);
    NotifyWindowInputPidChange(isStartDrag_);

    UpdateTargetRectOnDragEvent(pointerEvent, SizeChangeReason::DRAG_END);
    OnMoveDragCallback(SizeChangeReason::DRAG_END, TargetRectUpdateMode::UPDATED_IMMEDIATELY);
    return true;
}

void MoveDragController::MoveDragInterrupted(bool resetPosition)
{
    TLOGI(WmsLogTag::WMS_LAYOUT, "Screen anomaly, MoveDrag has been interrupted, id:%{public}d", persistentId_);
    SizeChangeReason reason = SizeChangeReason::DRAG_END;
    hasPointDown_ = false;
    if (GetStartDragFlag()) {
        SetStartDragFlag(false);
        ResSchedReportData(OHOS::ResourceSchedule::ResType::RES_TYPE_RESIZE_WINDOW, false);
        NotifyWindowInputPidChange(isStartDrag_);
    };
    if (GetStartMoveFlag()) {
        SetStartMoveFlag(false);
        ProcessWindowDragHotAreaFunc(windowDragHotAreaType_ != WINDOW_HOT_AREA_TYPE_UNDEFINED, reason);
    };
    if (resetPosition) {
        moveDragEndDisplayId_ = moveDragStartDisplayId_;
        UpdateTargetRect(reason, moveDragProperty_.originalRect_);
    } else {
        UpdateTargetRect(reason);
    }
    OnMoveDragCallback(reason, TargetRectUpdateMode::UPDATED_IMMEDIATELY);
}

void MoveDragController::StopMoving()
{
    TLOGD(WmsLogTag::WMS_LAYOUT_PC, "in");
    SizeChangeReason reason = SizeChangeReason::DRAG_END;
    hasPointDown_ = false;
    if (GetStartMoveFlag()) {
        SetStartMoveFlag(false);
        ProcessWindowDragHotAreaFunc(windowDragHotAreaType_ != WINDOW_HOT_AREA_TYPE_UNDEFINED, reason);
    };
    UpdateTargetRect(reason);
    OnMoveDragCallback(reason, TargetRectUpdateMode::UPDATED_IMMEDIATELY);
}

WSRect MoveDragController::GetScreenRectById(DisplayId displayId)
{
    auto screenSession = ScreenSessionManagerClient::GetInstance().GetScreenSessionById(displayId);
    RETURN_IF_NULL(screenSession, WSRect{ -1, -1, -1, -1 });

    ScreenProperty screenProperty = screenSession->GetScreenProperty();
    WSRect screenRect = {
        screenProperty.GetStartX(),
        screenProperty.GetStartY(),
        screenProperty.GetBounds().rect_.GetWidth(),
        screenProperty.GetBounds().rect_.GetHeight(),
    };
    return screenRect;
}

void MoveDragController::SetMoveAvailableArea(const DMRect& area)
{
    moveAvailableArea_.posX_ = area.posX_;
    moveAvailableArea_.posY_ = area.posY_;
    moveAvailableArea_.width_ = area.width_;
    moveAvailableArea_.height_ = area.height_;
}

void MoveDragController::UpdateMoveAvailableArea(DisplayId targetDisplayId)
{
    sptr<Display> display = DisplayManager::GetInstance().GetDisplayById(targetDisplayId);
    if (display == nullptr) {
        TLOGD(WmsLogTag::WMS_KEYBOARD, "Failed to get display");
        return;
    }
    DMRect availableArea;
    DMError ret = display->GetAvailableArea(availableArea);
    if (ret != DMError::DM_OK) {
        return;
    }
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
    screenSizeProperty_.currentDisplayStartX = currentScreenProperty.GetStartX();
    screenSizeProperty_.currentDisplayStartY = currentScreenProperty.GetStartY();
    screenSizeProperty_.currentDisplayLeft = currentScreenProperty.GetBounds().rect_.left_;
    screenSizeProperty_.currentDisplayTop = currentScreenProperty.GetBounds().rect_.top_;
    screenSizeProperty_.width = currentScreenProperty.GetBounds().rect_.width_;
    screenSizeProperty_.height = currentScreenProperty.GetBounds().rect_.height_;
}

void MoveDragController::SetScale(float scaleX, float scaleY)
{
    if (MathHelper::NearZero(moveDragProperty_.scaleX_) || MathHelper::NearZero(moveDragProperty_.scaleY_)) {
        TLOGE(WmsLogTag::WMS_LAYOUT, "scale ratio is 0");
        moveDragProperty_.scaleX_ = 1.0f;
        moveDragProperty_.scaleY_ = 1.0f;
        return;
    }
    moveDragProperty_.scaleX_ = scaleX;
    moveDragProperty_.scaleY_ = scaleY;
}

void MoveDragController::SetParentRect(const Rect& parentRect)
{
    parentRect_ = parentRect;
    TLOGD(WmsLogTag::WMS_LAYOUT, "parentRect_:%{public}s", parentRect_.ToString().c_str());
}

std::pair<int32_t, int32_t> MoveDragController::ComputeOffsetFromStart(
    const std::shared_ptr<MMI::PointerEvent>& pointerEvent)
{
    auto pointerItemOpt = GetPointerItem(pointerEvent);
    RETURN_IF_NULL(pointerItemOpt, { 0, 0 });

    auto displayOffset = GetLegacyGlobalDisplayOffset(static_cast<DisplayId>(pointerEvent->GetTargetDisplayId()));
    RETURN_IF_NULL(displayOffset, { 0, 0 });

    // The current position of the pointer in the unified coordinate system
    const auto& pointerItem = *pointerItemOpt;
    const auto [curDisplayOffsetX, curDisplayOffsetY] = *displayOffset;
    const int32_t curUnifiedX = curDisplayOffsetX + pointerItem.GetDisplayX();
    const int32_t curUnifiedY = curDisplayOffsetY + pointerItem.GetDisplayY();

    // The unified coordinates of the initial pointer
    const int32_t originUnifiedX = originalDisplayOffsetX_ + moveDragProperty_.originalPointerPosX_;
    const int32_t originUnifiedY = originalDisplayOffsetY_ + moveDragProperty_.originalPointerPosY_;

    // calculate trans in unified coordinates
    int32_t deltaX = curUnifiedX - originUnifiedX;
    int32_t deltaY = curUnifiedY - originUnifiedY;

    // NOTE: Intrusive modification, should be optimized.
    if (isAdaptToProportionalScale_) {
        return std::make_pair(deltaX, deltaY);
    }

    if (MathHelper::NearZero(moveDragProperty_.scaleX_) || MathHelper::NearZero(moveDragProperty_.scaleY_)) {
        TLOGE(WmsLogTag::WMS_LAYOUT, "Scale ratio is near 0, fallback to raw delta.");
        return std::make_pair(deltaX, deltaY);
    }
    return std::make_pair(deltaX / moveDragProperty_.scaleX_, deltaY / moveDragProperty_.scaleY_);
}

void MoveDragController::AdjustTargetPositionByAvailableArea(int32_t& moveDragFinalX, int32_t& moveDragFinalY)
{
    moveDragFinalX = std::max(moveAvailableArea_.posX_, moveDragFinalX);
    moveDragFinalY = std::max(moveAvailableArea_.posY_, moveDragFinalY);

    int32_t rightBoundsLimit = moveAvailableArea_.posX_ + static_cast<int32_t>(moveAvailableArea_.width_) -
                               moveDragProperty_.originalRect_.width_;
    int32_t bottomBoundsLimit = moveAvailableArea_.posY_ + static_cast<int32_t>(moveAvailableArea_.height_) -
                                moveDragProperty_.originalRect_.height_;

    if (moveDragFinalX >= rightBoundsLimit) {
        moveDragFinalX = rightBoundsLimit;
    }
    if (moveDragFinalY >= bottomBoundsLimit) {
        moveDragFinalY = bottomBoundsLimit;
    }
}

MoveDirection MoveDragController::CalcMoveDirection(DisplayId lastDisplayId, DisplayId currentDisplayId)
{
    sptr<ScreenSession> lastScreenSession =
        ScreenSessionManagerClient::GetInstance().GetScreenSessionById(lastDisplayId);
    sptr<ScreenSession> currentScreenSession =
        ScreenSessionManagerClient::GetInstance().GetScreenSessionById(currentDisplayId);
    if (!lastScreenSession || !currentScreenSession) {
        TLOGW(WmsLogTag::WMS_KEYBOARD, "Screen session is null, return default mouse move direction.");
        return MoveDirection::UNKNOWN;
    }

    ScreenProperty lastScreenProperty = lastScreenSession->GetScreenProperty();
    ScreenProperty currentScreenProperty = currentScreenSession->GetScreenProperty();

    uint32_t lastOriginStartX = lastScreenProperty.GetStartX();
    uint32_t lastOriginStartY = lastScreenProperty.GetStartY();
    uint32_t currentOriginStartX = currentScreenProperty.GetStartX();
    uint32_t currentOriginStartY = currentScreenProperty.GetStartY();

    uint32_t lastScreenWidth = lastScreenProperty.GetBounds().rect_.width_;
    uint32_t lastScreenHeight = lastScreenProperty.GetBounds().rect_.height_;
    uint32_t currentScreenWidth = currentScreenProperty.GetBounds().rect_.width_;
    uint32_t currentScreenHeight = currentScreenProperty.GetBounds().rect_.height_;

    if (currentOriginStartX == lastOriginStartX + lastScreenWidth) {
        return MoveDirection::LEFT_TO_RIGHT;
    } else if (currentOriginStartX == lastOriginStartX - currentScreenWidth) {
        return MoveDirection::RIGHT_TO_LEFT;
    } else if (currentOriginStartY == lastOriginStartY + lastScreenHeight) {
        return MoveDirection::UP_TO_BOTTOM;
    } else if (currentOriginStartY == lastOriginStartY - currentScreenHeight) {
        return MoveDirection::BOTTOM_TO_UP;
    }

    return MoveDirection::UNKNOWN;
}

void MoveDragController::SetInputBarCrossAttr(MoveDirection moveDirection, DisplayId targetDisplayId)
{
    if (moveDirection == MoveDirection::LEFT_TO_RIGHT ||
        moveDirection == MoveDirection::RIGHT_TO_LEFT) {
        UpdateMoveAvailableArea(targetDisplayId);
    }
    moveInputBarStartDisplayId_ = targetDisplayId;
    originalDisplayOffsetX_ = static_cast<int32_t>(screenSizeProperty_.currentDisplayStartX);
    originalDisplayOffsetY_ = static_cast<int32_t>(screenSizeProperty_.currentDisplayStartY);
    screenSizeProperty_.Reset();
}

void MoveDragController::InitializeMoveDragPropertyNotValid(const std::shared_ptr<MMI::PointerEvent>& pointerEvent,
                                                            const WSRect& originalRect)
{
    auto pointerItemOpt = GetPointerItem(pointerEvent);
    RETURN_IF_NULL(pointerItemOpt);
    const auto& pointerItem = *pointerItemOpt;

    int32_t pointerDisplayX = pointerItem.GetDisplayX();
    int32_t pointerDisplayY = pointerItem.GetDisplayY();
    moveDragProperty_.pointerId_ = pointerItem.GetOriginPointerId();
    moveDragProperty_.pointerType_ = pointerEvent->GetSourceType();
    moveDragProperty_.originalPointerPosX_ = pointerDisplayX;
    moveDragProperty_.originalPointerPosY_ = pointerDisplayY;
    int32_t pointerWindowX = pointerItem.GetWindowX();
    int32_t pointerWindowY = pointerItem.GetWindowY();
    moveDragProperty_.originalRect_ = originalRect;
    moveDragProperty_.originalRect_.posX_ = pointerDisplayX - pointerWindowX - parentRect_.posX_;
    moveDragProperty_.originalRect_.posY_ = pointerDisplayY - pointerWindowY - parentRect_.posY_;
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

void MoveDragController::HandleLeftToRightCross(DisplayId targetDisplayId,
                                                int32_t pointerDisplayX,
                                                int32_t pointerDisplayY,
                                                int32_t& moveDragFinalX,
                                                int32_t& moveDragFinalY)
{
    if (pointerDisplayX > moveDragProperty_.originalPointerWindowX_) {
        moveDragFinalX = pointerDisplayX - moveDragProperty_.originalPointerWindowX_;
    } else {
        moveDragFinalX = 0;
    }
    moveDragFinalY = pointerDisplayY - moveDragProperty_.originalPointerWindowY_;
    SetInputBarCrossAttr(MoveDirection::LEFT_TO_RIGHT, targetDisplayId);
}

void MoveDragController::HandleRightToLeftCross(DisplayId targetDisplayId,
                                                int32_t pointerDisplayX,
                                                int32_t pointerDisplayY,
                                                int32_t& moveDragFinalX,
                                                int32_t& moveDragFinalY)
{
    int32_t boundaryPos = screenSizeProperty_.width -
        moveDragProperty_.originalRect_.width_ + moveDragProperty_.originalPointerWindowX_;
    if (pointerDisplayX <= boundaryPos) {
        moveDragFinalX = pointerDisplayX - moveDragProperty_.originalPointerWindowX_;
    } else {
        moveDragFinalX = screenSizeProperty_.width - moveDragProperty_.originalRect_.width_;
    }
    moveDragFinalY = pointerDisplayY - moveDragProperty_.originalPointerWindowY_;
    SetInputBarCrossAttr(MoveDirection::RIGHT_TO_LEFT, targetDisplayId);
}

void MoveDragController::HandleUpToBottomCross(DisplayId targetDisplayId,
                                               int32_t pointerDisplayX,
                                               int32_t pointerDisplayY,
                                               int32_t& moveDragFinalX,
                                               int32_t& moveDragFinalY)
{
    UpdateMoveAvailableArea(targetDisplayId);
    int32_t statusBarHeight = moveAvailableArea_.posY_ - screenSizeProperty_.currentDisplayTop;
    if (pointerDisplayY >= statusBarHeight + moveDragProperty_.originalPointerWindowY_) {
        moveDragFinalY = pointerDisplayY - moveDragProperty_.originalPointerWindowY_;
    } else {
        moveDragFinalY = statusBarHeight;
    }
    moveDragFinalX = pointerDisplayX - moveDragProperty_.originalPointerWindowX_;
    SetInputBarCrossAttr(MoveDirection::UP_TO_BOTTOM, targetDisplayId);
}

void MoveDragController::HandleBottomToUpCross(DisplayId targetDisplayId,
                                               int32_t pointerDisplayX,
                                               int32_t pointerDisplayY,
                                               int32_t& moveDragFinalX,
                                               int32_t& moveDragFinalY)
{
    UpdateMoveAvailableArea(targetDisplayId);
    int32_t dockBarHeight = screenSizeProperty_.currentDisplayTop - moveAvailableArea_.posY_ -
                            static_cast<int32_t>(moveAvailableArea_.height_);
    int32_t boundaryPos =
        screenSizeProperty_.height - dockBarHeight - moveDragProperty_.originalPointerWindowY_;
    if (pointerDisplayY <= boundaryPos) {
        moveDragFinalY = pointerDisplayY - moveDragProperty_.originalPointerWindowY_;
    } else {
        moveDragFinalY = screenSizeProperty_.height - dockBarHeight - moveDragProperty_.originalPointerWindowY_;
    }
    moveDragFinalX = pointerDisplayX - moveDragProperty_.originalPointerWindowX_;
    SetInputBarCrossAttr(MoveDirection::BOTTOM_TO_UP, targetDisplayId);
}

void MoveDragController::CalcMoveForSameDisplay(const std::shared_ptr<MMI::PointerEvent>& pointerEvent,
                                                int32_t& moveDragFinalX, int32_t& moveDragFinalY)
{
    auto pointerItemOpt = GetPointerItem(pointerEvent);
    RETURN_IF_NULL(pointerItemOpt);
    const auto& pointerItem = *pointerItemOpt;

    int32_t pointerDisplayX = pointerItem.GetDisplayX();
    int32_t pointerDisplayY = pointerItem.GetDisplayY();
    moveDragFinalX = pointerDisplayX - moveDragProperty_.originalPointerWindowX_;
    moveDragFinalY = pointerDisplayY - moveDragProperty_.originalPointerWindowY_;
    AdjustTargetPositionByAvailableArea(moveDragFinalX, moveDragFinalY);
}

bool MoveDragController::CalcMoveInputBarRect(
    const std::shared_ptr<MMI::PointerEvent>& pointerEvent,  const WSRect& originalRect, SizeChangeReason reason)
{
    if (!CheckAndInitializeMoveDragProperty(pointerEvent, originalRect)) {
        return false;
    }

    auto pointerItemOpt = GetPointerItem(pointerEvent);
    RETURN_IF_NULL(pointerItemOpt, false);
    const auto& pointerItem = *pointerItemOpt;

    DisplayId targetDisplayId = static_cast<DisplayId>(pointerEvent->GetTargetDisplayId());
    int32_t moveDragFinalX = 0;
    int32_t moveDragFinalY = 0;
    int32_t pointerDisplayX = pointerItem.GetDisplayX();
    int32_t pointerDisplayY = pointerItem.GetDisplayY();

    if (targetDisplayId == moveInputBarStartDisplayId_) {
        CalcMoveForSameDisplay(pointerEvent, moveDragFinalX, moveDragFinalY);
    } else {
        MoveDirection moveDirection =
            CalcMoveDirection(moveInputBarStartDisplayId_, pointerEvent->GetTargetDisplayId());
        if (screenSizeProperty_.IsEmpty()) {
            SetCurrentScreenProperty(targetDisplayId);
        }

        switch (moveDirection) {
            case MoveDirection::LEFT_TO_RIGHT:
                HandleLeftToRightCross(
                    targetDisplayId, pointerDisplayX, pointerDisplayY, moveDragFinalX, moveDragFinalY);
                break;
            case MoveDirection::RIGHT_TO_LEFT:
                HandleRightToLeftCross(
                    targetDisplayId, pointerDisplayX, pointerDisplayY, moveDragFinalX, moveDragFinalY);
                break;
            case MoveDirection::UP_TO_BOTTOM:
                HandleUpToBottomCross(
                    targetDisplayId, pointerDisplayX, pointerDisplayY, moveDragFinalX, moveDragFinalY);
                break;
            case MoveDirection::BOTTOM_TO_UP:
                HandleBottomToUpCross(
                    targetDisplayId, pointerDisplayX, pointerDisplayY, moveDragFinalX, moveDragFinalY);
                break;
            default:
                moveDragFinalX = moveDragProperty_.targetRect_.posX_;
                moveDragFinalY = moveDragProperty_.targetRect_.posY_;
                break;
        }
    }
    WSRect targetRect = { moveDragFinalX, moveDragFinalY, originalRect.width_, originalRect.height_ };
    UpdateTargetRect(reason, targetRect);
    TLOGD(WmsLogTag::WMS_KEYBOARD, "move rect: %{public}s", moveDragProperty_.targetRect_.ToString().c_str());
    return true;
}

void MoveDragController::UpdateTargetRectWithOffset(int32_t offsetX, int32_t offsetY, SizeChangeReason reason)
{
    // The position is updated relative to the move start offset, while the size
    // must be refreshed because the window may resize during movement.
    auto curRect = CallSceneSession(&SceneSession::GetMoveRectForWindowDrag, WSRect::EMPTY_RECT);
    WSRect targetRect = {
        moveDragProperty_.originalRect_.posX_ + offsetX,
        moveDragProperty_.originalRect_.posY_ + offsetY,
        curRect.width_,
        curRect.height_
    };
    UpdateTargetRect(reason, targetRect);
}

TargetRectUpdateMode MoveDragController::UpdateTargetRectOnMoveEvent(
    const std::shared_ptr<MMI::PointerEvent>& pointerEvent, SizeChangeReason reason)
{
    if (ShouldBlockCrossDisplay(pointerEvent)) {
        return TargetRectUpdateMode::NONE;
    }

    const auto [offsetX, offsetY] = ComputeOffsetFromStart(pointerEvent);

    // If move resampling is active and the drag operation has not ended yet,
    // do not update targetRect immediately. Instead, push the move event into
    // moveResampler so that the target rect can be updated on the next vsync
    // using resampled data.
    //
    // Note:
    // - When reason is DRAG_END, it indicates that the drag-move operation
    //   has finished. In this case, isStartMove_ will be set to false and
    //   move resampling must be stopped.
    // - No further events should be pushed into moveResampler after DRAG_END.
    // - The final position at DRAG_END should be applied to the window
    //   immediately, without resampling.
    if (moveDragProperty_.isMoveResampleActive_ && reason != SizeChangeReason::DRAG_END) {
        const int64_t eventTimeUs = pointerEvent->GetActionTime();
        moveResampler_.PushEvent(eventTimeUs, offsetX, offsetY);
        return TargetRectUpdateMode::RESAMPLE_SCHEDULED;
    }

    UpdateTargetRectWithOffset(offsetX, offsetY, reason);
    return TargetRectUpdateMode::UPDATED_IMMEDIATELY;
}

std::pair<TargetRectUpdateMode, WSRect> MoveDragController::ResampleTargetRectOnVsync(int64_t vsyncTimeUs)
{
    if (!GetStartMoveFlag()) {
        TLOGW(WmsLogTag::WMS_LAYOUT, "Not in moving state, skip resampled targetRect update.");
        return { TargetRectUpdateMode::NONE, WSRect::EMPTY_RECT };
    }

    // Update move resampling activation based on the effective FPS observed
    // on the first resample vsync during a drag-move.
    UpdateResampleActivationByFps();

    if (!moveDragProperty_.isMoveResampleActive_) {
        TLOGW(WmsLogTag::WMS_LAYOUT, "Move resampling not activated, skip resampled targetRect update.");
        return { TargetRectUpdateMode::NONE, WSRect::EMPTY_RECT };
    }

    // Compute pointer position at this vsync via resampling.
    auto sample = moveResampler_.ResampleAt(vsyncTimeUs);

    // Update internal targetRect_ using the resampled offset.
    UpdateTargetRectWithOffset(sample.posX, sample.posY, moveDragProperty_.targetRectChangeReason_);

    // Resampling occurs only during DRAG_MOVE. The returned rectangle is always
    // expressed in the legacy global (unified) coordinate system.
    auto rect = GetTargetRect(MoveDragController::TargetRectCoordinate::GLOBAL);
    return { TargetRectUpdateMode::UPDATED_IMMEDIATELY, rect };
}

/** @note @window.drag */
bool MoveDragController::EventDownInit(const std::shared_ptr<MMI::PointerEvent>& pointerEvent)
{
    if (IsInvalidMouseEvent(pointerEvent)) {
        TLOGD(WmsLogTag::WMS_LAYOUT, "Invalid mouse event, buttonId: %{public}d", pointerEvent->GetButtonId());
        return false;
    }

    HITRACE_METER_FMT(HITRACE_TAG_WINDOW_MANAGER, "MoveDragController::EventDownInit");

    auto pointerItemOpt = GetPointerItem(pointerEvent);
    RETURN_IF_NULL(pointerItemOpt, false);
    const auto& pointerItem = *pointerItemOpt;

    InitMoveDragProperty();
    hasPointDown_ = true;
    auto originalRect = CallSceneSession(&SceneSession::GetGlobalOrWinRect, WSRect::EMPTY_RECT);
    moveDragProperty_.originalRect_ = originalRect;
    auto display = DisplayManager::GetInstance().GetDisplayById(
        static_cast<uint64_t>(pointerEvent->GetTargetDisplayId()));
    if (display) {
        vpr_ = display->GetVirtualPixelRatio();
    } else {
        vpr_ = 1.5f;  // 1.5f: default virtual pixel ratio
    }
    const auto sourceType = pointerEvent->GetSourceType();
    int outside = (sourceType == MMI::PointerEvent::SOURCE_TYPE_MOUSE) ? HOTZONE_POINTER * vpr_ : HOTZONE_TOUCH * vpr_;
    type_ = SessionHelper::GetAreaType(pointerItem.GetWindowX(), pointerItem.GetWindowY(), sourceType, outside, vpr_,
        moveDragProperty_.originalRect_, limits_);
    dragAreaType_ = SessionHelper::GetAreaTypeForScaleResize(pointerItem.GetWindowX(), pointerItem.GetWindowY(),
        outside, moveDragProperty_.originalRect_);
    TLOGI(WmsLogTag::WMS_LAYOUT, "pointWinX:%{public}d, pointWinY:%{public}d, outside:%{public}d, vpr:%{public}f, "
        "rect:%{public}s, type:%{public}d", pointerItem.GetWindowX(), pointerItem.GetWindowY(), outside, vpr_,
        moveDragProperty_.originalRect_.ToString().c_str(), type_);
    if (type_ == AreaType::UNDEFINED) {
        return false;
    }
    auto property = CallSceneSession(&SceneSession::GetSessionProperty, sptr<WindowSessionProperty>(nullptr));
    if (property) {
        isAdaptToDragScale_ = property->IsAdaptToDragScale();
    }
    moveDragProperty_.pointerId_ = pointerItem.GetOriginPointerId();
    moveDragProperty_.pointerType_ = sourceType;
    moveDragProperty_.originalPointerPosX_ = pointerItem.GetDisplayX();
    moveDragProperty_.originalPointerPosY_ = pointerItem.GetDisplayY();
    if (MathHelper::NearZero(aspectRatio_)) {
        CalcFreeformTranslateLimits(type_);
    }
    moveDragProperty_.originalRect_.posX_ = (originalRect.posX_ - parentRect_.posX_) / moveDragProperty_.scaleX_;
    moveDragProperty_.originalRect_.posY_ = (originalRect.posY_ - parentRect_.posY_) / moveDragProperty_.scaleY_;
    mainMoveAxis_ = AxisType::UNDEFINED;
    SetStartDragFlag(true);
    NotifyWindowInputPidChange(isStartDrag_);
    return true;
}

/** @note @window.drag */
WSRect MoveDragController::CalcFreeformTargetRect(AreaType type, int32_t tranX, int32_t tranY, WSRect originalRect)
{
    WSRect targetRect = originalRect;
    if (!isAdaptToDragScale_) {
        FixTranslateByLimits(tranX, tranY);
    }
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
    if (isAdaptToDragScale_) {
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
    int32_t width = originalRect.width_;
    int32_t height = originalRect.height_;
    if (mainMoveAxis_ == AxisType::UNDEFINED) {
        if (!InitMainAxis(type, tranX, tranY)) {
            return originalRect;
        }
    }

    TLOGD(WmsLogTag::WMS_LAYOUT, "ratio:%{public}f, areaType:%{public}u", aspectRatio, type);
    ConvertXYByAspectRatio(tranX, tranY, aspectRatio);
    FixTranslateByLimits(tranX, tranY);
    switch (type) {
        // tranX and tranY is signed
        case AreaType::LEFT_TOP: {
            return {posX + tranX, posY + tranY, width - tranX, height - tranY};
        }
        case AreaType::RIGHT_TOP: {
            return {posX, posY + tranY, width + tranX, height - tranY};
        }
        case AreaType::RIGHT_BOTTOM: {
            return {posX, posY, width + tranX, height + tranY};
        }
        case AreaType::LEFT_BOTTOM: {
            return {posX + tranX, posY, width - tranX, height + tranY};
        }
        case AreaType::LEFT: {
            return {posX + tranX, posY, width - tranX, height + tranY};
        }
        case AreaType::TOP: {
            return {posX, posY + tranY, width + tranX, height - tranY};
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
    if (MathHelper::NearZero(aspectRatio_)) {
        TLOGE(WmsLogTag::WMS_LAYOUT, "aspectRatio is near 0, id:%{public}d", persistentId_);
        return;
    }

    const int32_t origW = moveDragProperty_.originalRect_.width_;
    const int32_t origH = moveDragProperty_.originalRect_.height_;

    auto adjustedLimits = SessionUtils::AdjustLimitsByAspectRatio(limits_, decoration_, aspectRatio_);
    const int32_t minW = static_cast<int32_t>(adjustedLimits.minWidth_);
    const int32_t maxW = static_cast<int32_t>(adjustedLimits.maxWidth_);
    const int32_t minH = static_cast<int32_t>(adjustedLimits.minHeight_);
    const int32_t maxH = static_cast<int32_t>(adjustedLimits.maxHeight_);

    switch (type) {
        case AreaType::LEFT:
        case AreaType::LEFT_BOTTOM: {
            minTranX_ = origW - maxW;
            maxTranX_ = origW - minW;
            minTranY_ = minH - origH;
            maxTranY_ = maxH - origH;
            break;
        }
        case AreaType::LEFT_TOP: {
            minTranX_ = origW - maxW;
            maxTranX_ = origW - minW;
            minTranY_ = origH - maxH;
            maxTranY_ = origH - minH;
            break;
        }
        case AreaType::RIGHT:
        case AreaType::RIGHT_BOTTOM:
        case AreaType::BOTTOM: {
            minTranX_ = minW - origW;
            maxTranX_ = maxW - origW;
            minTranY_ = minH - origH;
            maxTranY_ = maxH - origH;
            break;
        }
        case AreaType::RIGHT_TOP:
        case AreaType::TOP: {
            minTranX_ = minW - origW;
            maxTranX_ = maxW - origW;
            minTranY_ = origH - maxH;
            maxTranY_ = origH - minH;
            break;
        }
        default: {
            TLOGE(WmsLogTag::WMS_LAYOUT, "Invalid type: %{public}u", static_cast<uint32_t>(type));
            break;
        }
    }
}

void MoveDragController::FixTranslateByLimits(int32_t& tranX, int32_t& tranY)
{
    tranX = std::clamp(tranX, minTranX_, maxTranX_);
    tranY = std::clamp(tranY, minTranY_, maxTranY_);
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

int32_t MoveDragController::ConvertByAreaType(int32_t tran) const
{
    const static std::map<AreaType, int32_t> areaTypeMap = {
        { AreaType::LEFT, NEGATIVE_CORRELATION },
        { AreaType::RIGHT, POSITIVE_CORRELATION },
        { AreaType::TOP, NEGATIVE_CORRELATION },
        { AreaType::BOTTOM, POSITIVE_CORRELATION },
        { AreaType::LEFT_TOP, POSITIVE_CORRELATION },
        { AreaType::RIGHT_TOP, NEGATIVE_CORRELATION },
        { AreaType::LEFT_BOTTOM, NEGATIVE_CORRELATION },
        { AreaType::RIGHT_BOTTOM, POSITIVE_CORRELATION },
    };
    if (areaTypeMap.find(type_) == areaTypeMap.end()) {
        TLOGE(WmsLogTag::WMS_LAYOUT, "not find type:%{public}d", type_);
        return tran;
    }
    return areaTypeMap.at(type_) * tran;
}

void MoveDragController::ConvertXYByAspectRatio(int32_t& tx, int32_t& ty, float aspectRatio)
{
    if (MathHelper::NearZero(aspectRatio)) {
        TLOGE(WmsLogTag::WMS_LAYOUT, "aspectRatio near zero");
        return;
    }
    if (mainMoveAxis_ == AxisType::X_AXIS) {
        ty = tx / aspectRatio;
        ty = ConvertByAreaType(ty);
    } else if (mainMoveAxis_ == AxisType::Y_AXIS) {
        tx = ty * aspectRatio;
        tx = ConvertByAreaType(tx);
    }
}

WSError MoveDragController::UpdateMoveTempProperty(const std::shared_ptr<MMI::PointerEvent>& pointerEvent)
{
    if (IsInvalidMouseEvent(pointerEvent)) {
        TLOGW(WmsLogTag::WMS_LAYOUT, "Invalid mouse event, buttonId: %{public}d", pointerEvent->GetButtonId());
        return WSError::WS_ERROR_NULLPTR;
    }

    auto pointerItemOpt = GetPointerItem(pointerEvent);
    RETURN_IF_NULL(pointerItemOpt, WSError::WS_ERROR_NULLPTR);
    const auto& pointerItem = *pointerItemOpt;

    switch (pointerEvent->GetPointerAction()) {
        case MMI::PointerEvent::POINTER_ACTION_BUTTON_DOWN:
        case MMI::PointerEvent::POINTER_ACTION_DOWN:
            moveTempProperty_.pointerId_ = pointerItem.GetOriginPointerId();
            moveTempProperty_.pointerType_ = pointerEvent->GetSourceType();
            moveTempProperty_.lastDownPointerPosX_ = pointerItem.GetDisplayX();
            moveTempProperty_.lastDownPointerPosY_ = pointerItem.GetDisplayY();
            moveTempProperty_.lastMovePointerPosX_ = pointerItem.GetDisplayX();
            moveTempProperty_.lastMovePointerPosY_ = pointerItem.GetDisplayY();
            moveTempProperty_.lastDownPointerWindowX_ = pointerItem.GetWindowX();
            moveTempProperty_.lastDownPointerWindowY_ = pointerItem.GetWindowY();
            break;
        case MMI::PointerEvent::POINTER_ACTION_MOVE:
            if (!IsMatchingStartPointer(pointerEvent, moveTempProperty_.pointerId_, moveTempProperty_.pointerType_)) {
                TLOGW(WmsLogTag::WMS_LAYOUT, "Ignoring pointer event, pointer ID or type mismatch");
                return WSError::WS_DO_NOTHING;
            }
            moveTempProperty_.lastMovePointerPosX_ = pointerItem.GetDisplayX();
            moveTempProperty_.lastMovePointerPosY_ = pointerItem.GetDisplayY();
            break;
        case MMI::PointerEvent::POINTER_ACTION_UP:
        case MMI::PointerEvent::POINTER_ACTION_BUTTON_UP:
        case MMI::PointerEvent::POINTER_ACTION_CANCEL: {
            moveTempProperty_.Reset();
            break;
        }
        default:
            break;
    }
    return WSError::WS_OK;
}

void MoveDragController::SetSpecifyMoveStartDisplay(DisplayId displayId)
{
    TLOGD(WmsLogTag::WMS_LAYOUT_PC, "called");
    std::lock_guard<std::mutex> lock(specifyMoveStartMutex_);
    specifyMoveStartDisplayId_ = displayId;
    isSpecifyMoveStart_ = true;
}

void MoveDragController::ClearSpecifyMoveStartDisplay()
{
    TLOGD(WmsLogTag::WMS_LAYOUT_PC, "called");
    std::lock_guard<std::mutex> lock(specifyMoveStartMutex_);
    specifyMoveStartDisplayId_ = DISPLAY_ID_INVALID;
    isSpecifyMoveStart_ = false;
}

void MoveDragController::HandleStartMovingWithCoordinate(const MoveCoordinateProperty& property, bool isMovable)
{
    moveTempProperty_.lastDownPointerPosX_ = property.pointerPosX;
    moveTempProperty_.lastDownPointerPosY_ = property.pointerPosY;
    moveTempProperty_.lastMovePointerPosX_ = property.pointerPosX;
    moveTempProperty_.lastMovePointerPosY_ = property.pointerPosY;
    moveTempProperty_.lastDownPointerWindowX_ = property.pointerWindowX;
    moveTempProperty_.lastDownPointerWindowY_ = property.pointerWindowY;

    // Map the winRect from its display to the start display.
    WSRect targetRect = MapRectFromTargetToStart(property.winRect, property.displayId);
    TLOGI(WmsLogTag::WMS_LAYOUT_PC, "displayId:%{public}" PRIu64 " targetRect: %{public}s",
        property.displayId, targetRect.ToString().c_str());
    UpdateTargetRect(SizeChangeReason::DRAG_MOVE, targetRect);
    moveDragEndDisplayId_ = property.displayId;
    if (isMovable) {
        OnMoveDragCallback(SizeChangeReason::DRAG_MOVE, TargetRectUpdateMode::UPDATED_IMMEDIATELY);
    }
}

/** @note @window.drag */
void MoveDragController::CalcFirstMoveTargetRect(const WSRect& windowRect, bool useWindowRect)
{
    if (!GetStartMoveFlag() || moveTempProperty_.isEmpty()) {
        return;
    }

    WSRect originalRect = {
        (windowRect.posX_ - parentRect_.posX_) / moveDragProperty_.scaleX_,
        (windowRect.posY_ - parentRect_.posY_) / moveDragProperty_.scaleY_,
        windowRect.width_,
        windowRect.height_
    };
    if (useWindowRect) {
        originalRect.posX_ = windowRect.posX_;
        originalRect.posY_ = windowRect.posY_;
    }
    SetOriginalMoveDragPos(moveTempProperty_.pointerId_,
                           moveTempProperty_.pointerType_,
                           moveTempProperty_.lastDownPointerPosX_,
                           moveTempProperty_.lastDownPointerPosY_,
                           moveTempProperty_.lastDownPointerWindowX_,
                           moveTempProperty_.lastDownPointerWindowY_,
                           originalRect);

    int32_t offsetX = moveTempProperty_.lastMovePointerPosX_ - moveTempProperty_.lastDownPointerPosX_;
    int32_t offsetY = moveTempProperty_.lastMovePointerPosY_ - moveTempProperty_.lastDownPointerPosY_;
    WSRect targetRect = originalRect.WithOffset(offsetX, offsetY);
    bool isSpecifyMoveStart = false;
    {
        std::lock_guard<std::mutex> lock(specifyMoveStartMutex_);
        isSpecifyMoveStart = isSpecifyMoveStart_;
    }
    if (isSpecifyMoveStart) {
        TLOGI(WmsLogTag::WMS_LAYOUT_PC, "specify start display:%{public}" PRIu64, specifyMoveStartDisplayId_);
        moveDragProperty_.originalRect_.posX_ = moveTempProperty_.lastDownPointerPosX_ -
            moveTempProperty_.lastDownPointerWindowX_ - parentRect_.posX_;
        moveDragProperty_.originalRect_.posY_ = moveTempProperty_.lastDownPointerPosY_ -
            moveTempProperty_.lastDownPointerWindowY_ - parentRect_.posY_;
        targetRect.posX_ = moveDragProperty_.originalRect_.posX_ + offsetX;
        targetRect.posY_ = moveDragProperty_.originalRect_.posY_ + offsetY;
        targetRect = MapRectFromTargetToStart(targetRect, specifyMoveStartDisplayId_);
    }
    UpdateTargetRect(SizeChangeReason::DRAG_MOVE, targetRect);
    isAdaptToProportionalScale_ = useWindowRect;
    bool resampleActivated = ShouldOpenMoveResample(moveTempProperty_.pointerType_);
    moveDragProperty_.isMoveResampleActive_ = resampleActivated;
    TLOGI(WmsLogTag::WMS_LAYOUT, "id: %{public}d, first move rect: %{public}s, resampleActivated: %{public}d",
          persistentId_, targetRect.ToString().c_str(), resampleActivated);

    auto mode = resampleActivated ?
        TargetRectUpdateMode::RESAMPLE_ACTIVATED : TargetRectUpdateMode::UPDATED_IMMEDIATELY;
    OnMoveDragCallback(SizeChangeReason::DRAG_MOVE, mode);
}

/** @note @window.drag */
void MoveDragController::UpdateHotAreaType(const std::shared_ptr<MMI::PointerEvent>& pointerEvent)
{
    auto pointerItemOpt = GetPointerItem(pointerEvent);
    RETURN_IF_NULL(pointerItemOpt);
    const auto& pointerItem = *pointerItemOpt;

    int32_t pointerDisplayX = pointerItem.GetDisplayX();
    int32_t pointerDisplayY = pointerItem.GetDisplayY();
    DisplayId displayId = static_cast<uint64_t>(pointerEvent->GetTargetDisplayId());
    uint32_t windowDragHotAreaType = SceneSession::GetWindowDragHotAreaType(displayId, WINDOW_HOT_AREA_TYPE_UNDEFINED,
        pointerDisplayX, pointerDisplayY);
    if (windowDragHotAreaType_ != windowDragHotAreaType) {
        TLOGI(WmsLogTag::WMS_LAYOUT, "window drag hot area is changed, old type:%{public}d, new type:%{public}d",
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

int32_t MoveDragController::GetPointerType() const
{
    return moveDragProperty_.pointerType_;
}

void MoveDragController::SetWindowDragHotAreaFunc(const NotifyWindowDragHotAreaFunc& func)
{
    windowDragHotAreaFunc_ = func;
}

void MoveDragController::OnLostFocus()
{
    if (isStartMove_ || isStartDrag_) {
        TLOGW(WmsLogTag::WMS_LAYOUT, "window id:%{public}d lost focus, should stop MoveDrag, isMove:%{public}d, "
            "isDrag:%{public}d", persistentId_, isStartMove_, isStartDrag_);
        MoveDragInterrupted(false);
    }
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
    TLOGD(WmsLogTag::WMS_LAYOUT, "ResSchedReportData success type: %{public}d onOffTag: %{public}d", type, onOffTag);
#endif
}

template <typename Ret, typename Func, typename... Args>
Ret MoveDragController::CallSceneSession(Func func, Ret defaultValue, Args&& ...args) const
{
    if (auto session = sceneSession_.promote()) {
        return (session->*func)(std::forward<Args>(args)...);
    } else {
        TLOGE(WmsLogTag::WMS_LAYOUT, "Failed to get sceneSession, windowId: %{public}d", persistentId_);
        return defaultValue;
    }
}

template <typename Func, typename... Args>
void MoveDragController::CallSceneSessionVoid(Func func, Args&& ...args) const
{
    if (auto session = sceneSession_.promote()) {
        (session->*func)(std::forward<Args>(args)...);
        return;
    }
    TLOGE(WmsLogTag::WMS_LAYOUT, "Failed to get sceneSession, windowId: %{public}d", persistentId_);
}

void MoveDragController::OnMoveDragCallback(SizeChangeReason reason, TargetRectUpdateMode mode)
{
    CallSceneSessionVoid(&SceneSession::OnMoveDragCallback, reason, mode);
}

void MoveDragController::SetMoveResampleSystemConfig(
    bool enable, std::optional<uint32_t> minFps, std::optional<uint32_t> maxFps)
{
    system::SetParameter(MOVE_RESAMPLE_ENABLE_PARAM_KEY, enable ? "true" : "false");
    std::string minFpsStr = minFps ? std::to_string(*minFps) : ""; // empty value means unlimited
    std::string maxFpsStr = maxFps ? std::to_string(*maxFps) : "";
    system::SetParameter(MOVE_RESAMPLE_MIN_FPS_PARAM_KEY, minFpsStr);
    system::SetParameter(MOVE_RESAMPLE_MAX_FPS_PARAM_KEY, maxFpsStr);
    TLOGI(WmsLogTag::WMS_LAYOUT, "enable: %{public}d, minFps: %{public}s, maxFps: %{public}s",
          enable, minFpsStr.c_str(), maxFpsStr.c_str());
}

std::tuple<bool, std::optional<uint32_t>, std::optional<uint32_t>> MoveDragController::GetMoveResampleSystemConfig()
{
    bool enable = system::GetBoolParameter(MOVE_RESAMPLE_ENABLE_PARAM_KEY, false);
    std::optional<uint32_t> minFps;
    std::optional<uint32_t> maxFps;
    std::string minFpsStr = system::GetParameter(MOVE_RESAMPLE_MIN_FPS_PARAM_KEY, "");
    std::string maxFpsStr = system::GetParameter(MOVE_RESAMPLE_MAX_FPS_PARAM_KEY, "");

    auto parseOptionalUint32 = [](const std::string& str) -> std::optional<uint32_t> {
        if (str.empty()) {
            return std::nullopt;
        }
        uint32_t value = 0;
        auto [_, ec] = std::from_chars(str.data(), str.data() + str.size(), value);
        if (ec != std::errc{}) {
            return std::nullopt;
        }
        return value;
    };
    minFps = parseOptionalUint32(minFpsStr);
    maxFps = parseOptionalUint32(maxFpsStr);
    TLOGD(WmsLogTag::WMS_LAYOUT,
          "enable: %{public}d, minFps: %{public}s, maxFps: %{public}s",
          enable,
          minFps ? std::to_string(*minFps).c_str() : "unlimited",
          maxFps ? std::to_string(*maxFps).c_str() : "unlimited");
    return { enable, minFps, maxFps };
}

bool MoveDragController::IsInvalidMouseEvent(const std::shared_ptr<MMI::PointerEvent>& pointerEvent) const
{
    RETURN_IF_NULL(pointerEvent, false);
    return pointerEvent->GetSourceType() == MMI::PointerEvent::SOURCE_TYPE_MOUSE &&
           pointerEvent->GetButtonId() != MMI::PointerEvent::MOUSE_BUTTON_LEFT;
}

bool MoveDragController::SyncPropertiesFromSceneSession()
{
    auto session = sceneSession_.promote();
    RETURN_IF_NULL(session, false);

    aspectRatio_ = session->GetAspectRatio();
    limits_ = session->GetWindowLimits();
    decoration_ = session->GetWindowDecoration();
    supportCrossDisplay_ = session->IsCrossDisplayDragSupported();
    TLOGD(WmsLogTag::WMS_LAYOUT,
          "windowId: %{public}d, aspectRatio: %{public}f, "
          "limits: %{public}s, decoration: %{public}s, "
          "supportCrossDisplay: %{public}d",
          persistentId_, aspectRatio_,
          limits_.ToString().c_str(), decoration_.ToString().c_str(),
          supportCrossDisplay_);
    return true;
}

bool MoveDragController::ShouldOpenMoveResample(int32_t pointerType) const
{
    // Global enable switch
    if (!enableMoveResample_) {
        return false;
    }

    // Only touchscreen events support move resampling.
    // For other pointer types (e.g. mouse input), we prefer immediate
    // responsiveness over additional smoothing. Mouse events are typically
    // delivered at high frequency with fine-grained movement, and applying
    // resampling may introduce unnecessary latency and reduce perceived
    // precision.
    if (pointerType != MMI::PointerEvent::SOURCE_TYPE_TOUCHSCREEN) {
        return false;
    }

    // Input windows do not support move resampling. To be optimized later.
    if (WindowHelper::IsInputWindow(winType_)) {
        return false;
    }
    return true;
}

void MoveDragController::UpdateResampleActivationByFps()
{
    // FPS range validation is only applicable when move resampling is active.
    if (!moveDragProperty_.isMoveResampleActive_) {
        return;
    }

    // Perform the FPS range check at most once per drag-move operation.
    if (moveDragProperty_.isResampleFpsRangeChecked_) {
        return;
    }
    moveDragProperty_.isResampleFpsRangeChecked_ = true;

    // Query the effective display FPS during drag-move and disable move
    // resampling if it falls outside the configured range.
    std::optional<uint32_t> curFps =
        CallSceneSession(&SceneSession::GetFpsFromVsync, std::optional<uint32_t>(std::nullopt));
    if (!curFps) {
        moveDragProperty_.isMoveResampleActive_ = false;
        TLOGW(WmsLogTag::WMS_LAYOUT, "Failed to get current FPS, disabling move resampling.");
        return;
    }

    if ((resampleMinFps_ && *curFps < *resampleMinFps_) || (resampleMaxFps_ && *curFps > *resampleMaxFps_)) {
        moveDragProperty_.isMoveResampleActive_ = false;
        TLOGI(WmsLogTag::WMS_LAYOUT,
              "Current FPS %{public}u is outside resample range [%{public}s, %{public}s], "
              "disabling move resampling.",
              *curFps,
              resampleMinFps_ ? std::to_string(*resampleMinFps_).c_str() : "unlimited",
              resampleMaxFps_ ? std::to_string(*resampleMaxFps_).c_str() : "unlimited");
        return;
    }
    TLOGI(WmsLogTag::WMS_LAYOUT,
          "Current FPS %{public}u is within resample range [%{public}s, %{public}s], "
          "keeping move resampling active.",
          *curFps,
          resampleMinFps_ ? std::to_string(*resampleMinFps_).c_str() : "unlimited",
          resampleMaxFps_ ? std::to_string(*resampleMaxFps_).c_str() : "unlimited");
}
} // namespace OHOS::Rosen
