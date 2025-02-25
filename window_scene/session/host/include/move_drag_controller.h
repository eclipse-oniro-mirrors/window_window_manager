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

#ifndef OHOS_ROSEN_WINDOW_SCENE_MOVE_DRAG_CONTROLLER_H
#define OHOS_ROSEN_WINDOW_SCENE_MOVE_DRAG_CONTROLLER_H

#include <refbase.h>
#include <struct_multimodal.h>

#include "common/include/window_session_property.h"
#include "property/rs_properties_def.h"
#include "window.h"
#include "ws_common_inner.h"

namespace OHOS::MMI {
class PointerEvent;
} // namespace MMI

namespace OHOS::Rosen {

using MoveDragCallback = std::function<void(const SizeChangeReason&)>;

using NotifyWindowDragHotAreaFunc = std::function<void(uint32_t type, const SizeChangeReason& reason)>;

using NotifyWindowPidChangeCallback = std::function<void(int32_t windowId, bool startMoving)>;

const uint32_t WINDOW_HOT_AREA_TYPE_UNDEFINED = 0;

class MoveDragController : public RefBase {
public:
    MoveDragController(int32_t persistentId);
    ~MoveDragController() = default;

    void RegisterMoveDragCallback(const MoveDragCallback& callBack);
    void SetStartMoveFlag(bool flag);
    bool GetStartMoveFlag() const;
    bool GetStartDragFlag() const;
    bool HasPointDown();
    void SetNotifyWindowPidChangeCallback(const NotifyWindowPidChangeCallback& callback);
    WSRect GetTargetRect() const;
    void InitMoveDragProperty();
    void SetOriginalMoveDragPos(int32_t pointerId, int32_t pointerType, int32_t pointerPosX,
        int32_t pointerPosY, int32_t pointerWindowX, int32_t pointerWindowY,
        const WSRect& winRect);
    void SetAspectRatio(float ratio);
    bool ConsumeMoveEvent(const std::shared_ptr<MMI::PointerEvent>& pointerEvent, const WSRect& originalRect);
    bool ConsumeDragEvent(const std::shared_ptr<MMI::PointerEvent>& pointerEvent, const WSRect& originalRect,
        const sptr<WindowSessionProperty> property, const SystemSessionConfig& sysConfig);
    void CalcFirstMoveTargetRect(const WSRect& windowRect, bool isFullToFloating);
    WSRect GetFullScreenToFloatingRect(const WSRect& originalRect, const WSRect& windowRect);
    int32_t GetOriginalPointerPosX();
    int32_t GetOriginalPointerPosY();
    void SetWindowDragHotAreaFunc(const NotifyWindowDragHotAreaFunc& func);
    void UpdateGravityWhenDrag(const std::shared_ptr<MMI::PointerEvent>& pointerEvent,
        const std::shared_ptr<RSSurfaceNode>& surfaceNode);
    void OnLostFocus();
    bool GetMoveInputBarFlag();
    DisplayId GetMoveInputBarStartDisplayId();
    void SetMoveInputBarStartDisplayId(DisplayId displayId);
    void SetMoveAvailableArea(const DMRect& area);
    void SetMoveInputBarFlag(bool moveInputBarFlag);

    /*
     * PC Window Layout
     */
    void HandleStartMovingWithCoordinate(int32_t offsetX, int32_t offsetY,
        int32_t pointerPosX, int32_t pointerPosY, const WSRect& winRect);
    void StopMoving();

private:
    struct MoveDragProperty {
        int32_t pointerId_ = -1;
        int32_t pointerType_ = -1;
        int32_t originalPointerPosX_ = -1;
        int32_t originalPointerPosY_ = -1;
        // the x coordinate of the pointer related to the active window
        int32_t originalPointerWindowX_ = -1;
        // the y coordinate of the pointer related to the active window
        int32_t originalPointerWindowY_ = -1;
        WSRect originalRect_ = { 0, 0, 0, 0 };
        WSRect targetRect_ = { 0, 0, 0, 0 };

        bool isEmpty() const
        {
            return (pointerId_ == -1 && originalPointerPosX_ == -1 && originalPointerPosY_ == -1);
        }
    };

    struct MoveTempProperty {
        int32_t pointerId_ = -1;
        int32_t pointerType_ = -1;
        int32_t lastDownPointerPosX_ = -1;
        int32_t lastDownPointerPosY_ = -1;
        int32_t lastDownPointerWindowX_ = -1;
        int32_t lastDownPointerWindowY_ = -1;
        int32_t lastMovePointerPosX_ = -1;
        int32_t lastMovePointerPosY_ = -1;

        bool isEmpty() const
        {
            return (pointerId_ == -1 && lastDownPointerPosX_ == -1 && lastDownPointerPosY_ == -1);
        }
    };

    enum AxisType { UNDEFINED, X_AXIS, Y_AXIS };
    constexpr static float NEAR_ZERO = 0.001f;

    bool CalcMoveTargetRect(const std::shared_ptr<MMI::PointerEvent>& pointerEvent, const WSRect& originalRect);
    bool CalcMoveInputBarRect(const std::shared_ptr<MMI::PointerEvent>& pointerEvent, const WSRect& originalRect);
    void AdjustTargetPositionByAvailableArea(int32_t& moveDragFinalX, int32_t& moveDragFinalY);
    void InitializeMoveDragPropertyNotValid(const std::shared_ptr<MMI::PointerEvent>& pointerEvent,
                                            const WSRect& originalRect);
    bool CheckAndInitializeMoveDragProperty(const std::shared_ptr<MMI::PointerEvent>& pointerEvent,
                                            const WSRect& originalRect);
    void CalcMoveForSameDisplay(const std::shared_ptr<MMI::PointerEvent>& pointerEvent,
                                int32_t& moveDragFinalX, int32_t& moveDragFinalY);
    bool EventDownInit(const std::shared_ptr<MMI::PointerEvent>& pointerEvent, const WSRect& originalRect,
        const sptr<WindowSessionProperty> property, const SystemSessionConfig& sysConfig);
    AreaType GetAreaType(int32_t pointWinX, int32_t pointWinY, int32_t sourceType, const WSRect& rect);
    WSRect CalcFreeformTargetRect(AreaType type, int32_t tranX, int32_t tranY, WSRect originalRect);
    WSRect CalcFixedAspectRatioTargetRect(AreaType type, int32_t tranX, int32_t tranY, float aspectRatio,
        WSRect originalRect);
    void CalcFreeformTranslateLimits(AreaType type);
    void CalcFixedAspectRatioTranslateLimits(AreaType type, AxisType axis);
    void FixTranslateByLimits(int32_t& tranX, int32_t& tranY);
    bool InitMainAxis(AreaType type, int32_t tranX, int32_t tranY);
    void ConvertXYByAspectRatio(int32_t& tx, int32_t& ty, float aspectRatio);
    void ProcessSessionRectChange(const SizeChangeReason& reason);
    void InitDecorValue(const sptr<WindowSessionProperty> property, const SystemSessionConfig& sysConfig);

    float GetVirtualPixelRatio() const;
    void UpdateDragType(int32_t startPointPosX, int32_t startPointPosY);
    bool IsPointInDragHotZone(int32_t startPointPosX, int32_t startPointPosY,
        int32_t sourceType, const WSRect& winRect);
    void CalculateStartRectExceptHotZone(float vpr, const WSRect& winRect);
    WSError UpdateMoveTempProperty(const std::shared_ptr<MMI::PointerEvent>& pointerEvent);
    bool CheckDragEventLegal(const std::shared_ptr<MMI::PointerEvent>& pointerEvent,
        const sptr<WindowSessionProperty> property);
    void ResSchedReportData(int32_t type, bool onOffTag);
    void NotifyWindowInputPidChange(bool isServerPid);

    bool isStartMove_ = false;
    bool isStartDrag_ = false;
    bool isDecorEnable_ = true;
    bool hasPointDown_ = false;
    float aspectRatio_ = 0.0f;
    float vpr_ = 1.0f;
    int32_t minTranX_ = INT32_MIN;
    int32_t minTranY_ = INT32_MIN;
    int32_t maxTranX_ = INT32_MAX;
    int32_t maxTranY_ = INT32_MAX;
    AreaType type_ = AreaType::UNDEFINED;
    AxisType mainMoveAxis_ = AxisType::UNDEFINED;
    WindowLimits limits_;
    MoveDragProperty moveDragProperty_;
    MoveDragCallback moveDragCallback_;
    int32_t persistentId_;

    enum class DragType : uint32_t {
        DRAG_UNDEFINED,
        DRAG_LEFT_OR_RIGHT,
        DRAG_BOTTOM_OR_TOP,
        DRAG_LEFT_TOP_CORNER,
        DRAG_RIGHT_TOP_CORNER,
    };
    const std::map<DragType, uint32_t> STYLEID_MAP = {
        {DragType::DRAG_UNDEFINED,        MMI::MOUSE_ICON::DEFAULT},
        {DragType::DRAG_BOTTOM_OR_TOP,    MMI::MOUSE_ICON::NORTH_SOUTH},
        {DragType::DRAG_LEFT_OR_RIGHT,    MMI::MOUSE_ICON::WEST_EAST},
        {DragType::DRAG_LEFT_TOP_CORNER,  MMI::MOUSE_ICON::NORTH_WEST_SOUTH_EAST},
        {DragType::DRAG_RIGHT_TOP_CORNER, MMI::MOUSE_ICON::NORTH_EAST_SOUTH_WEST}
    };
    Rect rectExceptFrame_ { 0, 0, 0, 0 };
    Rect rectExceptCorner_ { 0, 0, 0, 0 };
    uint32_t mouseStyleID_ = 0;
    DragType dragType_ = DragType::DRAG_UNDEFINED;
    MoveTempProperty moveTempProperty_;
    DMRect moveAvailableArea_ = {0, 0, 0, 0};
    DisplayId moveInputBarStartDisplayId_ = DISPLAY_ID_INVALID;
    bool moveInputBarFlag_ = false;

    void UpdateHotAreaType(const std::shared_ptr<MMI::PointerEvent>& pointerEvent);
    void ProcessWindowDragHotAreaFunc(bool flag, const SizeChangeReason& reason);
    uint32_t windowDragHotAreaType_ = WINDOW_HOT_AREA_TYPE_UNDEFINED;
    NotifyWindowDragHotAreaFunc windowDragHotAreaFunc_;
    NotifyWindowPidChangeCallback pidChangeCallback_;

    const std::map<AreaType, Gravity> GRAVITY_MAP = {
        {AreaType::LEFT,          Gravity::RIGHT},
        {AreaType::TOP,           Gravity::BOTTOM},
        {AreaType::RIGHT,         Gravity::LEFT},
        {AreaType::BOTTOM,        Gravity::TOP},
        {AreaType::LEFT_TOP,      Gravity::BOTTOM_RIGHT},
        {AreaType::RIGHT_TOP,     Gravity::BOTTOM_LEFT},
        {AreaType::RIGHT_BOTTOM,  Gravity::TOP_LEFT},
        {AreaType::LEFT_BOTTOM,   Gravity::TOP_RIGHT}
    };
};
} // namespace OHOS::Rosen
#endif // OHOS_ROSEN_WINDOW_SCENE_MOVE_DRAG_CONTROLLER_H
