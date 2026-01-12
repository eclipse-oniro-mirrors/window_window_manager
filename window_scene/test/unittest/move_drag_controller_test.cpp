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

#include <gtest/gtest.h>
#include <memory>

#include "pointer_event.h"
#include "ui/rs_surface_node.h"

#include "mock_vsync_station.h"
#include "scene_board_judgement.h"
#include "scene_session.h"
#include "screen_manager.h"
#include "screen_session_manager_client/include/screen_session_manager_client.h"
#include "session.h"
#include "session/screen/include/screen_session.h"
#include "window_manager_hilog.h"
#include "window_scene.h"

using namespace testing;
using namespace testing::ext;

namespace OHOS {
namespace Rosen {
class MoveDragControllerTest : public testing::Test {
public:
    static void SetUpTestCase();
    static void TearDownTestCase();
    void SetUp() override;
    void TearDown() override;
    sptr<MoveDragController> moveDragController;
    sptr<SceneSession> session_;
    std::shared_ptr<MockVsyncStation> mockVsyncStation_;
};

void MoveDragControllerTest::SetUpTestCase() {}

void MoveDragControllerTest::TearDownTestCase()
{
}

void MoveDragControllerTest::SetUp()
{
    SessionInfo info;
    info.abilityName_ = "testSession1";
    info.moduleName_ = "testSession2";
    info.bundleName_ = "testSession3";
    session_ = sptr<SceneSession>::MakeSptr(info, nullptr);

    mockVsyncStation_ = std::make_shared<MockVsyncStation>();
    session_->SetVsyncStation(mockVsyncStation_);

    moveDragController = sptr<MoveDragController>::MakeSptr(wptr(session_));
}

void MoveDragControllerTest::TearDown()
{
    session_ = nullptr;
    moveDragController = nullptr;
    mockVsyncStation_ = nullptr;
}

namespace {
/**
 * @tc.name: SetStartMoveFlag
 * @tc.desc: test function : SetStartMoveFlag
 * @tc.type: FUNC
 */
HWTEST_F(MoveDragControllerTest, SetStartMoveFlag, TestSize.Level0)
{
    moveDragController->hasPointDown_ = false;
    moveDragController->SetStartMoveFlag(true);
    ASSERT_FALSE(moveDragController->isStartMove_);
    moveDragController->hasPointDown_ = true;
    moveDragController->SetStartMoveFlag(true);
    ASSERT_TRUE(moveDragController->isStartMove_);
    moveDragController->hasPointDown_ = false;
}

/**
 * @tc.name: GetStartMoveFlag
 * @tc.desc: test function : GetStartMoveFlag
 * @tc.type: FUNC
 */
HWTEST_F(MoveDragControllerTest, GetStartMoveFlag, TestSize.Level0)
{
    auto preIsStartMove = moveDragController->isStartMove_;
    auto preHasPointDown = moveDragController->hasPointDown_;
    moveDragController->hasPointDown_ = false;
    moveDragController->SetStartMoveFlag(true);
    bool res = moveDragController->GetStartMoveFlag();
    EXPECT_EQ(preIsStartMove, res);
    moveDragController->SetStartMoveFlag(false);
    res = moveDragController->GetStartMoveFlag();
    EXPECT_EQ(false, res);
    moveDragController->hasPointDown_ = true;
    moveDragController->SetStartMoveFlag(true);
    res = moveDragController->GetStartMoveFlag();
    EXPECT_EQ(true, res);
    moveDragController->SetStartMoveFlag(false);
    res = moveDragController->GetStartMoveFlag();
    EXPECT_EQ(false, res);
    moveDragController->isStartMove_ = preIsStartMove;
    moveDragController->hasPointDown_ = preHasPointDown;
}

/**
 * @tc.name: GetStartDragFlag
 * @tc.desc: test function : GetStartDragFlag
 * @tc.type: FUNC
 */
HWTEST_F(MoveDragControllerTest, GetStartDragFlag, TestSize.Level1)
{
    bool res = moveDragController->GetStartDragFlag();
    EXPECT_EQ(false, res);
}

/**
 * @tc.name: GetMoveDragStartDisplayId
 * @tc.desc: test function : GetMoveDragStartDisplayId
 * @tc.type: FUNC
 */
HWTEST_F(MoveDragControllerTest, GetMoveDragStartDisplayId, TestSize.Level1)
{
    uint64_t res = moveDragController->GetMoveDragStartDisplayId();
    EXPECT_EQ(-1ULL, res);
}

/**
 * @tc.name: GetMoveDragEndDisplayId
 * @tc.desc: test function : GetMoveDragEndDisplayId
 * @tc.type: FUNC
 */
HWTEST_F(MoveDragControllerTest, GetMoveDragEndDisplayId, TestSize.Level1)
{
    uint64_t res = moveDragController->GetMoveDragEndDisplayId();
    EXPECT_EQ(-1ULL, res);
}

/**
 * @tc.name: GetInitParentNodeId
 * @tc.desc: test function : GetInitParentNodeId
 * @tc.type: FUNC
 */
HWTEST_F(MoveDragControllerTest, GetInitParentNodeId, TestSize.Level1)
{
    uint64_t res = moveDragController->GetInitParentNodeId();
    EXPECT_EQ(-1ULL, res);
}

/**
 * @tc.name: GetDisplayIdsDuringMoveDrag
 * @tc.desc: test function : GetDisplayIdsDuringMoveDrag
 * @tc.type: FUNC
 */
HWTEST_F(MoveDragControllerTest, GetDisplayIdsDuringMoveDrag, TestSize.Level1)
{
    std::set<uint64_t> res = moveDragController->GetDisplayIdsDuringMoveDrag();
    ASSERT_EQ(true, res.empty());
}

/**
 * @tc.name: GetTargetRect
 * @tc.desc: test function : GetTargetRect
 * @tc.type: FUNC
 */
HWTEST_F(MoveDragControllerTest, GetTargetRect, TestSize.Level1)
{
    uint32_t tmp = 0;
    int32_t pos = 0;
    moveDragController->InitMoveDragProperty();
    WSRect res = moveDragController->GetTargetRect(MoveDragController::TargetRectCoordinate::GLOBAL);
    EXPECT_EQ(tmp, res.height_);
    EXPECT_EQ(tmp, res.width_);
    EXPECT_EQ(pos, res.posX_);
    EXPECT_EQ(pos, res.posY_);

    res = moveDragController->GetTargetRect(MoveDragController::TargetRectCoordinate::RELATED_TO_START_DISPLAY);
    EXPECT_EQ(tmp, res.height_);
    EXPECT_EQ(tmp, res.width_);
    EXPECT_EQ(pos, res.posX_);
    EXPECT_EQ(pos, res.posY_);
}

/**
 * @tc.name: InitCrossDisplayProperty
 * @tc.desc: test function : InitCrossDisplayProperty
 * @tc.type: FUNC
 */
HWTEST_F(MoveDragControllerTest, InitCrossDisplayProperty, TestSize.Level0)
{
    moveDragController->InitCrossDisplayProperty(1, 2);
    ASSERT_EQ(1, moveDragController->GetMoveDragStartDisplayId());
    ASSERT_EQ(2, moveDragController->GetInitParentNodeId());
    ASSERT_EQ(true,
              moveDragController->GetDisplayIdsDuringMoveDrag().find(1) !=
                  moveDragController->GetDisplayIdsDuringMoveDrag().end());
    ScreenId screenId = 1;
    ScreenSessionConfig config;
    sptr<ScreenSession> screenSession =
        sptr<ScreenSession>::MakeSptr(config, ScreenSessionReason::CREATE_SESSION_FOR_CLIENT);
    ScreenSessionManagerClient::GetInstance().screenSessionMap_.insert(std::make_pair(screenId, screenSession));
    moveDragController->InitCrossDisplayProperty(1, 2);
    ASSERT_EQ(1, moveDragController->GetMoveDragStartDisplayId());
    ASSERT_EQ(2, moveDragController->GetInitParentNodeId());
    ASSERT_EQ(true,
              moveDragController->GetDisplayIdsDuringMoveDrag().find(1) !=
                  moveDragController->GetDisplayIdsDuringMoveDrag().end());
}

/**
 * @tc.name: SetOriginalMoveDragPos
 * @tc.desc: test function : SetOriginalMoveDragPos
 * @tc.type: FUNC
 */
HWTEST_F(MoveDragControllerTest, SetOriginalMoveDragPos, TestSize.Level0)
{
    std::shared_ptr<MMI::PointerEvent> pointerEvent = MMI::PointerEvent::Create();
    int32_t pointerId = pointerEvent->GetPointerId();
    int32_t pointerType = pointerEvent->GetSourceType();
    int32_t pointerPosX = 10;
    int32_t pointerPosY = 30;
    int32_t pointerWindowX = 10;
    int32_t pointerWindowY = 10;
    WSRect winRect = { 100, 100, 1000, 1000 };
    moveDragController->SetOriginalMoveDragPos(
        pointerId, pointerType, pointerPosX, pointerPosY, pointerWindowX, pointerWindowY, winRect);
    ASSERT_EQ(moveDragController->moveDragProperty_.pointerId_, pointerId);
    ASSERT_EQ(moveDragController->moveDragProperty_.pointerType_, pointerType);
    ASSERT_EQ(moveDragController->moveDragProperty_.originalPointerPosX_, pointerPosX);
    ASSERT_EQ(moveDragController->moveDragProperty_.originalPointerPosY_, pointerPosY);
    ASSERT_EQ(moveDragController->moveDragProperty_.originalPointerWindowX_, pointerWindowX);
    ASSERT_EQ(moveDragController->moveDragProperty_.originalPointerWindowY_, pointerWindowY);
    ASSERT_EQ(moveDragController->moveDragProperty_.originalRect_, winRect);
}

/**
 * @tc.name: UpdateGravityWhenDrag
 * @tc.desc: test function : UpdateGravityWhenDrag
 * @tc.type: FUNC
 */
HWTEST_F(MoveDragControllerTest, UpdateGravityWhenDrag, TestSize.Level0)
{
    struct RSSurfaceNodeConfig config;
    std::shared_ptr<RSSurfaceNode> surfaceNode = RSSurfaceNode::Create(config);
    std::shared_ptr<MMI::PointerEvent> pointerEvent = MMI::PointerEvent::Create();
    if (SceneBoardJudgement::IsSceneBoardEnabled()) {
        ASSERT_NE(nullptr, surfaceNode);
        ASSERT_NE(nullptr, pointerEvent);
        pointerEvent->SetButtonId(MMI::PointerEvent::MOUSE_BUTTON_LEFT);
        pointerEvent->SetSourceType(MMI::PointerEvent::SOURCE_TYPE_MOUSE);
        pointerEvent->SetPointerAction(MMI::PointerEvent::POINTER_ACTION_BUTTON_DOWN);
        moveDragController->UpdateGravityWhenDrag(nullptr, surfaceNode);
        moveDragController->UpdateGravityWhenDrag(pointerEvent, nullptr);
        moveDragController->type_ = AreaType::UNDEFINED;
        moveDragController->UpdateGravityWhenDrag(pointerEvent, surfaceNode);
        moveDragController->type_ = AreaType::RIGHT;
        moveDragController->UpdateGravityWhenDrag(pointerEvent, surfaceNode);
        auto modifier = surfaceNode->GetModifierByType(ModifierNG::RSModifierType::CLIP_TO_FRAME);
        ASSERT_NE(modifier, nullptr);

        pointerEvent->SetPointerAction(MMI::PointerEvent::POINTER_ACTION_BUTTON_DOWN);
        moveDragController->UpdateGravityWhenDrag(pointerEvent, surfaceNode);
        modifier = surfaceNode->GetModifierByType(ModifierNG::RSModifierType::CLIP_TO_FRAME);
        ASSERT_NE(modifier, nullptr);

        pointerEvent->SetPointerAction(MMI::PointerEvent::POINTER_ACTION_DOWN);
        moveDragController->UpdateGravityWhenDrag(pointerEvent, surfaceNode);
        modifier = surfaceNode->GetModifierByType(ModifierNG::RSModifierType::CLIP_TO_FRAME);
        ASSERT_NE(modifier, nullptr);

        pointerEvent->SetPointerAction(MMI::PointerEvent::POINTER_ACTION_UP);
        moveDragController->UpdateGravityWhenDrag(pointerEvent, surfaceNode);
        modifier = surfaceNode->GetModifierByType(ModifierNG::RSModifierType::CLIP_TO_FRAME);
        ASSERT_NE(modifier, nullptr);

        pointerEvent->SetPointerAction(MMI::PointerEvent::POINTER_ACTION_BUTTON_UP);
        moveDragController->UpdateGravityWhenDrag(pointerEvent, surfaceNode);
        modifier = surfaceNode->GetModifierByType(ModifierNG::RSModifierType::CLIP_TO_FRAME);
        ASSERT_NE(modifier, nullptr);

        pointerEvent->SetPointerAction(MMI::PointerEvent::POINTER_ACTION_CANCEL);
        moveDragController->UpdateGravityWhenDrag(pointerEvent, surfaceNode);
        modifier = surfaceNode->GetModifierByType(ModifierNG::RSModifierType::CLIP_TO_FRAME);
        ASSERT_NE(modifier, nullptr);

        pointerEvent->SetPointerAction(MMI::PointerEvent::POINTER_ACTION_UNKNOWN);
        moveDragController->UpdateGravityWhenDrag(pointerEvent, surfaceNode);
        modifier = surfaceNode->GetModifierByType(ModifierNG::RSModifierType::CLIP_TO_FRAME);
        ASSERT_NE(modifier, nullptr);
    }
}

/**
 * @tc.name: CalcMoveInputBarRect
 * @tc.desc: test function : CalcMoveInputBarRect
 * @tc.type: FUNC
 */
HWTEST_F(MoveDragControllerTest, CalcMoveInputBarRect, TestSize.Level1)
{
    moveDragController->InitMoveDragProperty();
    moveDragController->SetMoveAvailableArea({ 0, 75, 3120, 1980 });
    moveDragController->SetMoveInputBarStartDisplayId(1);
    WSRect originalRect = { 10, 20, 336, 146 };

    std::shared_ptr<MMI::PointerEvent> pointerEvent = MMI::PointerEvent::Create();
    pointerEvent->SetTargetDisplayId(1);
    pointerEvent->SetPointerId(1);
    pointerEvent->SetSourceType(MMI::PointerEvent::SOURCE_TYPE_MOUSE);
    MMI::PointerEvent::PointerItem pointerItem;
    pointerItem.SetPointerId(1);
    pointerItem.SetOriginPointerId(1);
    pointerItem.SetDisplayX(100);
    pointerItem.SetDisplayY(200);
    pointerEvent->AddPointerItem(pointerItem);
    int32_t pointerPosX = 10;
    int32_t pointerPosY = 30;
    int32_t pointerWindowX = 10;
    int32_t pointerWindowY = 10;
    moveDragController->SetOriginalMoveDragPos(pointerItem.GetOriginPointerId(),
                                               pointerEvent->GetSourceType(),
                                               pointerPosX,
                                               pointerPosY,
                                               pointerWindowX,
                                               pointerWindowY,
                                               originalRect);
    moveDragController->CalcMoveInputBarRect(pointerEvent, originalRect, SizeChangeReason::DRAG_MOVE);

    ASSERT_EQ(90, moveDragController->moveDragProperty_.targetRect_.posX_);
    ASSERT_EQ(190, moveDragController->moveDragProperty_.targetRect_.posY_);
}

/**
 * @tc.name: AdjustTargetPositionByAvailableArea
 * @tc.desc: test function : AdjustTargetPositionByAvailableArea
 * @tc.type: FUNC
 */
HWTEST_F(MoveDragControllerTest, AdjustTargetPositionByAvailableArea, TestSize.Level1)
{
    DMRect moveAvailableArea = { 0, 75, 3120, 1980 };
    WSRect originalRect = { 10, 20, 336, 146 };
    moveDragController->moveAvailableArea_ = moveAvailableArea;
    moveDragController->moveDragProperty_.originalRect_ = originalRect;

    int32_t x;
    int32_t y;

    x = 50, y = 100;
    moveDragController->AdjustTargetPositionByAvailableArea(x, y);
    EXPECT_EQ(x, 50);
    EXPECT_EQ(y, 100);

    x = -10, y = 100;
    moveDragController->AdjustTargetPositionByAvailableArea(x, y);
    EXPECT_EQ(x, 0);
    EXPECT_EQ(y, 100);

    x = 3200, y = 200;
    moveDragController->AdjustTargetPositionByAvailableArea(x, y);
    EXPECT_EQ(x, 2784);
    EXPECT_EQ(y, 200);

    x = 100, y = 60;
    moveDragController->AdjustTargetPositionByAvailableArea(x, y);
    EXPECT_EQ(x, 100);
    EXPECT_EQ(y, 75);

    x = 100, y = 1980;
    moveDragController->AdjustTargetPositionByAvailableArea(x, y);
    EXPECT_EQ(x, 100);
    EXPECT_EQ(y, 1909);
}

/**
 * @tc.name: EventDownInit
 * @tc.desc: test function : EventDownInit
 * @tc.type: FUNC
 */
HWTEST_F(MoveDragControllerTest, EventDownInit, TestSize.Level1)
{
    moveDragController->InitMoveDragProperty();
    std::shared_ptr<MMI::PointerEvent> pointerEvent = MMI::PointerEvent::Create();
    WSRect originalRect = { 100, 100, 1000, 1000 };
    session_->SetSessionRect(originalRect);
    session_->SetSessionGlobalRect(originalRect);

    pointerEvent = MMI::PointerEvent::Create();
    pointerEvent->SetButtonId(MMI::PointerEvent::MOUSE_BUTTON_RIGHT);
    pointerEvent->SetSourceType(MMI::PointerEvent::SOURCE_TYPE_MOUSE);

    auto res = moveDragController->EventDownInit(pointerEvent);
    EXPECT_EQ(false, res);
    pointerEvent->SetButtonId(MMI::PointerEvent::MOUSE_BUTTON_LEFT);
    pointerEvent->SetTargetDisplayId(0);
    MMI::PointerEvent::PointerItem pointerItem;
    pointerItem.SetPointerId(0);
    pointerItem.SetOriginPointerId(0);
    pointerItem.SetWindowX(5);
    pointerItem.SetWindowY(5);
    pointerEvent->SetPointerId(0);
    pointerEvent->AddPointerItem(pointerItem);
    originalRect = { 10, 10, 10, 10 };
    session_->SetSessionRect(originalRect);
    session_->SetSessionGlobalRect(originalRect);
    res = moveDragController->EventDownInit(pointerEvent);
    EXPECT_EQ(true, res);
}

/**
 * @tc.name: EventDownInit01
 * @tc.desc: test function : EventDownInit
 * @tc.type: FUNC
 */
HWTEST_F(MoveDragControllerTest, EventDownInit01, TestSize.Level1)
{
    moveDragController->InitMoveDragProperty();
    std::shared_ptr<MMI::PointerEvent> pointerEvent = MMI::PointerEvent::Create();
    MMI::PointerEvent::PointerItem pointerItem;
    pointerItem.SetPointerId(1);
    pointerItem.SetOriginPointerId(1);
    pointerItem.SetWindowX(1);
    pointerItem.SetWindowY(1);
    pointerEvent->SetPointerId(1);
    pointerEvent->AddPointerItem(pointerItem);
    pointerEvent->SetButtonId(MMI::PointerEvent::MOUSE_BUTTON_LEFT);
    pointerEvent->SetSourceType(MMI::PointerEvent::SOURCE_TYPE_MOUSE);
    WSRect originalRect = { 100, 100, 1000, 1000 };
    session_->SetSessionRect(originalRect);
    session_->SetSessionGlobalRect(originalRect);
    auto res = moveDragController->EventDownInit(pointerEvent);
    EXPECT_EQ(true, res);
}

/**
 * @tc.name: CalcFreeformTargetRect
 * @tc.desc: test function : CalcFreeformTargetRect
 * @tc.type: FUNC
 */
HWTEST_F(MoveDragControllerTest, CalcFreeformTargetRect, TestSize.Level1)
{
    AreaType type = AreaType::RIGHT;
    WSRect originalRect = { 100, 100, 1000, 1000 };
    int32_t tranX = 10;
    int32_t tranY = 30;
    ASSERT_TRUE((moveDragController != nullptr));
    moveDragController->CalcFreeformTargetRect(type, tranX, tranY, originalRect);
    type = AreaType::LEFT;
    moveDragController->CalcFreeformTargetRect(type, tranX, tranY, originalRect);
    type = AreaType::RIGHT;
    moveDragController->CalcFreeformTargetRect(type, tranX, tranY, originalRect);
    type = AreaType::BOTTOM;
    ASSERT_TRUE((moveDragController != nullptr));
    moveDragController->CalcFreeformTargetRect(type, tranX, tranY, originalRect);
    originalRect = { 100, 100, 1000, 0 };
    ASSERT_TRUE((moveDragController != nullptr));
    moveDragController->CalcFreeformTargetRect(type, tranX, tranY, originalRect);
    originalRect = { 100, 100, 500, 100 };
    moveDragController->limits_ = { 3, 3, 3, 3, 2.0, 2.0 };
    ASSERT_TRUE((moveDragController != nullptr));
    moveDragController->CalcFreeformTargetRect(type, tranX, tranY, originalRect);
    moveDragController->isAdaptToDragScale_ = true;
    originalRect = { 100, 100, 500, 100 };
    moveDragController->limits_ = { 3, 3, 3, 3, 2.0, 2.0 };
    ASSERT_TRUE((moveDragController != nullptr));
    moveDragController->CalcFreeformTargetRect(type, tranX, tranY, originalRect);
    moveDragController->isAdaptToDragScale_ = false;
    type = AreaType::RIGHT;
    originalRect = { 100, 100, 500, 0 };
    ASSERT_TRUE((moveDragController != nullptr));
    moveDragController->CalcFreeformTargetRect(type, tranX, tranY, originalRect);
    originalRect = { 100, 100, 100, 100 };
    ASSERT_TRUE((moveDragController != nullptr));
    moveDragController->CalcFreeformTargetRect(type, tranX, tranY, originalRect);
    type = AreaType::UNDEFINED;
    originalRect = { 100, 100, 500, 100 };
    ASSERT_TRUE((moveDragController != nullptr));
    moveDragController->CalcFreeformTargetRect(type, tranX, tranY, originalRect);
    type = AreaType::RIGHT;
    ASSERT_TRUE((moveDragController != nullptr));
    moveDragController->CalcFreeformTargetRect(type, tranX, tranY, originalRect);
    moveDragController->limits_ = { 3, 3, 3, 3, 0.0001, 0.0001 };
    ASSERT_TRUE((moveDragController != nullptr));
    moveDragController->CalcFreeformTargetRect(type, tranX, tranY, originalRect);
}

/**
 * @tc.name: CalcFixedAspectRatioTargetRect01
 * @tc.desc: test function : CalcFixedAspectRatioTargetRect01
 * @tc.type: FUNC
 */
HWTEST_F(MoveDragControllerTest, CalcFixedAspectRatioTargetRect01, TestSize.Level0)
{
    AreaType type = AreaType::RIGHT;
    float aspectRatio = 0.5;
    WSRect originalRect = { 100, 100, 1000, 1000 };
    int32_t tranX = 0;
    int32_t tranY = 0;
    ASSERT_TRUE((moveDragController != nullptr));
    moveDragController->CalcFixedAspectRatioTargetRect(type, tranX, tranY, aspectRatio, originalRect);
}

/**
 * @tc.name: CalcFixedAspectRatioTargetRect02
 * @tc.desc: test function : CalcFixedAspectRatioTargetRect02
 * @tc.type: FUNC
 */
HWTEST_F(MoveDragControllerTest, CalcFixedAspectRatioTargetRect02, TestSize.Level1)
{
    AreaType type = AreaType::RIGHT;
    float aspectRatio = 0.5;
    WSRect originalRect = { 100, 100, 1000, 1000 };
    int32_t tranX = 20;
    int32_t tranY = 20;
    ASSERT_TRUE((moveDragController != nullptr));
    type = AreaType::UNDEFINED;
    moveDragController->mainMoveAxis_ = MoveDragController::AxisType::UNDEFINED;
    tranX = 0;
    tranY = 0;
    moveDragController->CalcFixedAspectRatioTargetRect(type, tranX, tranY, aspectRatio, originalRect);
    type = AreaType::RIGHT;
    moveDragController->CalcFixedAspectRatioTargetRect(type, tranX, tranY, aspectRatio, originalRect);
    moveDragController->mainMoveAxis_ = MoveDragController::AxisType::X_AXIS;
    moveDragController->CalcFixedAspectRatioTargetRect(type, tranX, tranY, aspectRatio, originalRect);
    type = AreaType::LEFT_TOP;
    moveDragController->CalcFixedAspectRatioTargetRect(type, tranX, tranY, aspectRatio, originalRect);
    type = AreaType::RIGHT_TOP;
    moveDragController->CalcFixedAspectRatioTargetRect(type, tranX, tranY, aspectRatio, originalRect);
    type = AreaType::RIGHT_BOTTOM;
    moveDragController->CalcFixedAspectRatioTargetRect(type, tranX, tranY, aspectRatio, originalRect);
    type = AreaType::LEFT_BOTTOM;
    moveDragController->CalcFixedAspectRatioTargetRect(type, tranX, tranY, aspectRatio, originalRect);
    type = AreaType::LEFT;
    moveDragController->CalcFixedAspectRatioTargetRect(type, tranX, tranY, aspectRatio, originalRect);
    type = AreaType::TOP;
    moveDragController->CalcFixedAspectRatioTargetRect(type, tranX, tranY, aspectRatio, originalRect);
    type = AreaType::RIGHT;
    moveDragController->CalcFixedAspectRatioTargetRect(type, tranX, tranY, aspectRatio, originalRect);
    type = AreaType::BOTTOM;
    moveDragController->CalcFixedAspectRatioTargetRect(type, tranX, tranY, aspectRatio, originalRect);
    type = AreaType::UNDEFINED;
    moveDragController->CalcFixedAspectRatioTargetRect(type, tranX, tranY, aspectRatio, originalRect);
}

/**
 * @tc.name: CalcFreeformTranslateLimits01
 * @tc.desc: test function : CalcFreeformTranslateLimits01
 * @tc.type: FUNC
 */
HWTEST_F(MoveDragControllerTest, CalcFreeformTranslateLimits01, TestSize.Level0)
{
    AreaType type = AreaType::RIGHT;
    ASSERT_TRUE((moveDragController != nullptr));
    moveDragController->CalcFreeformTranslateLimits(type);
    type = AreaType::BOTTOM;
    moveDragController->CalcFreeformTranslateLimits(type);
    type = AreaType::TOP;
    moveDragController->CalcFreeformTranslateLimits(type);
    type = AreaType::LEFT;
    moveDragController->CalcFreeformTranslateLimits(type);
}

/**
 * @tc.name: CalcFixedAspectRatioTranslateLimitsAbnormalBranches
 * @tc.desc: Verify early-return and default branch (invalid AreaType) behaviors
 * @tc.type: FUNC
 */
HWTEST_F(MoveDragControllerTest, CalcFixedAspectRatioTranslateLimitsAbnormalBranches, TestSize.Level1)
{
    moveDragController->limits_ = WindowLimits(300, 150, 100, 50, FLT_MAX, 0.0f);
    moveDragController->decoration_ = {0, 0, 0, 0}; // no decoration
    moveDragController->moveDragProperty_.originalRect_ = {0, 0, 200, 120};
    moveDragController->minTranX_ = INT32_MIN;
    moveDragController->minTranY_ = INT32_MIN;
    moveDragController->maxTranX_ = INT32_MAX;
    moveDragController->maxTranY_ = INT32_MAX;

    // Case 1: aspectRatio_ is near zero -> early return, no modification
    moveDragController->aspectRatio_ = 0.0f;
    moveDragController->CalcFixedAspectRatioTranslateLimits(AreaType::LEFT);
    EXPECT_EQ(moveDragController->minTranX_, INT32_MIN);
    EXPECT_EQ(moveDragController->minTranY_, INT32_MIN);
    EXPECT_EQ(moveDragController->maxTranX_, INT32_MAX);
    EXPECT_EQ(moveDragController->maxTranY_, INT32_MAX);

    // Case 2: invalid AreaType -> no modification
    moveDragController->aspectRatio_ = 2.0f;
    moveDragController->CalcFixedAspectRatioTranslateLimits(AreaType::UNDEFINED);
    EXPECT_EQ(moveDragController->minTranX_, INT32_MIN);
    EXPECT_EQ(moveDragController->minTranY_, INT32_MIN);
    EXPECT_EQ(moveDragController->maxTranX_, INT32_MAX);
    EXPECT_EQ(moveDragController->maxTranY_, INT32_MAX);
}

/**
 * @tc.name: CalcFixedAspectRatioTranslateLimitsNormalBranches
 * @tc.desc: Verify correct translate limits for all handled AreaType when aspectRatio is valid
 * @tc.type: FUNC
 */
HWTEST_F(MoveDragControllerTest, CalcFixedAspectRatioTranslateLimitsNormalBranches, TestSize.Level1)
{
    moveDragController->limits_ = WindowLimits(300, 150, 100, 50, FLT_MAX, 0.0f); // compatible with aspectRatio = 2.0
    moveDragController->decoration_ = {0, 0, 0, 0};  // no decoration
    moveDragController->moveDragProperty_.originalRect_ = {0, 0, 200, 120};
    moveDragController->aspectRatio_ = 2.0f;

    // Case 1: LEFT
    moveDragController->CalcFixedAspectRatioTranslateLimits(AreaType::LEFT);
    EXPECT_EQ(moveDragController->minTranX_, -100);
    EXPECT_EQ(moveDragController->maxTranX_, 100);
    EXPECT_EQ(moveDragController->minTranY_, -70);
    EXPECT_EQ(moveDragController->maxTranY_, 30);

    // Case 2: LEFT_BOTTOM
    moveDragController->CalcFixedAspectRatioTranslateLimits(AreaType::LEFT_BOTTOM);
    EXPECT_EQ(moveDragController->minTranX_, -100);
    EXPECT_EQ(moveDragController->maxTranX_, 100);
    EXPECT_EQ(moveDragController->minTranY_, -70);
    EXPECT_EQ(moveDragController->maxTranY_, 30);

    // Case 3: LEFT_TOP
    moveDragController->CalcFixedAspectRatioTranslateLimits(AreaType::LEFT_TOP);
    EXPECT_EQ(moveDragController->minTranX_, -100);
    EXPECT_EQ(moveDragController->maxTranX_, 100);
    EXPECT_EQ(moveDragController->minTranY_, -30);
    EXPECT_EQ(moveDragController->maxTranY_, 70);

    // Case 4: RIGHT
    moveDragController->CalcFixedAspectRatioTranslateLimits(AreaType::RIGHT);
    EXPECT_EQ(moveDragController->minTranX_, -100);
    EXPECT_EQ(moveDragController->maxTranX_, 100);
    EXPECT_EQ(moveDragController->minTranY_, -70);
    EXPECT_EQ(moveDragController->maxTranY_, 30);

    // Case 5: RIGHT_BOTTOM
    moveDragController->CalcFixedAspectRatioTranslateLimits(AreaType::RIGHT_BOTTOM);
    EXPECT_EQ(moveDragController->minTranX_, -100);
    EXPECT_EQ(moveDragController->maxTranX_, 100);
    EXPECT_EQ(moveDragController->minTranY_, -70);
    EXPECT_EQ(moveDragController->maxTranY_, 30);

    // Case 6: BOTTOM
    moveDragController->CalcFixedAspectRatioTranslateLimits(AreaType::BOTTOM);
    EXPECT_EQ(moveDragController->minTranX_, -100);
    EXPECT_EQ(moveDragController->maxTranX_, 100);
    EXPECT_EQ(moveDragController->minTranY_, -70);
    EXPECT_EQ(moveDragController->maxTranY_, 30);

    // Case 7: RIGHT_TOP
    moveDragController->CalcFixedAspectRatioTranslateLimits(AreaType::RIGHT_TOP);
    EXPECT_EQ(moveDragController->minTranX_, -100);
    EXPECT_EQ(moveDragController->maxTranX_, 100);
    EXPECT_EQ(moveDragController->minTranY_, -30);
    EXPECT_EQ(moveDragController->maxTranY_, 70);

    // Case 8: TOP
    moveDragController->CalcFixedAspectRatioTranslateLimits(AreaType::TOP);
    EXPECT_EQ(moveDragController->minTranX_, -100);
    EXPECT_EQ(moveDragController->maxTranX_, 100);
    EXPECT_EQ(moveDragController->minTranY_, -30);
    EXPECT_EQ(moveDragController->maxTranY_, 70);
}

/**
 * @tc.name: FixTranslateByLimits01
 * @tc.desc: test function : FixTranslateByLimits01
 * @tc.type: FUNC
 */
HWTEST_F(MoveDragControllerTest, FixTranslateByLimits01, TestSize.Level1)
{
    int32_t tranX = 100;
    int32_t tranY = 100;
    moveDragController->maxTranX_ = 50;
    moveDragController->maxTranY_ = 50;
    ASSERT_TRUE((moveDragController != nullptr));
    moveDragController->FixTranslateByLimits(tranX, tranY);
    tranX = 10;
    moveDragController->FixTranslateByLimits(tranX, tranY);
    tranY = 10;
    moveDragController->FixTranslateByLimits(tranX, tranY);
}

/**
 * @tc.name: InitMainAxis01
 * @tc.desc: test function : InitMainAxis01
 * @tc.type: FUNC
 */
HWTEST_F(MoveDragControllerTest, InitMainAxis01, TestSize.Level1)
{
    AreaType type = AreaType::LEFT;
    int32_t tranX = 100;
    int32_t tranY = 100;
    ASSERT_TRUE((moveDragController != nullptr));
    moveDragController->InitMainAxis(type, tranX, tranY);
    type = AreaType::RIGHT;
    moveDragController->InitMainAxis(type, tranX, tranY);
    type = AreaType::TOP;
    moveDragController->InitMainAxis(type, tranX, tranY);
    type = AreaType::BOTTOM;
    moveDragController->InitMainAxis(type, tranX, tranY);
    type = AreaType::UNDEFINED;
    tranX = 0;
    tranY = 0;
    moveDragController->InitMainAxis(type, tranX, tranY);
    tranY = 1;
    moveDragController->InitMainAxis(type, tranX, tranY);
}

/**
 * @tc.name: ConvertXYByAspectRatio01
 * @tc.desc: test function : ConvertXYByAspectRatio01
 * @tc.type: FUNC
 */
HWTEST_F(MoveDragControllerTest, ConvertXYByAspectRatio01, TestSize.Level1)
{
    float aspectRatio = 1.0f;
    int32_t tx = 100;
    int32_t ty = 100;
    moveDragController->mainMoveAxis_ = MoveDragController::AxisType::X_AXIS;
    ASSERT_TRUE((moveDragController != nullptr));
    moveDragController->ConvertXYByAspectRatio(tx, ty, aspectRatio);
    moveDragController->mainMoveAxis_ = MoveDragController::AxisType::Y_AXIS;
    moveDragController->ConvertXYByAspectRatio(tx, ty, aspectRatio);
}

/**
 *@tc.name: GetGravity_TopLeft
 *@tc.desc: test function : GetGravity
 *@tc.type: FUNC
 */
HWTEST_F(MoveDragControllerTest, GetGravity_TopLeft, TestSize.Level1)
{
    moveDragController->dragAreaType_ = AreaType::UNDEFINED;
    Gravity gravity = moveDragController->GetGravity();
    EXPECT_EQ(gravity, Gravity::TOP_LEFT);
}

/**
 *@tc.name: GetGravity_TopRight
 *@tc.desc: test function : GetGravity
 *@tc.type: FUNC
 */
HWTEST_F(MoveDragControllerTest, GetGravity_TopRight, TestSize.Level1)
{
    moveDragController->dragAreaType_ = AreaType::LEFT_BOTTOM;
    Gravity gravity = moveDragController->GetGravity();
    EXPECT_EQ(gravity, Gravity::TOP_RIGHT);
}

/**
 * @tc.name: TestConsumeMoveEventWithEarlyReturns
 * @tc.desc: Verify that ConsumeMoveEvent handles various early-return scenarios
 * @tc.type: FUNC
 */
HWTEST_F(MoveDragControllerTest, TestConsumeMoveEventWithEarlyReturns, TestSize.Level1)
{
    if (!SceneBoardJudgement::IsSceneBoardEnabled()) {
        GTEST_SKIP() << "Skip test when SceneBoard is disabled.";
    }

    constexpr int32_t pointerId = 0;
    MMI::PointerEvent::PointerItem pointerItem;
    pointerItem.SetPointerId(pointerId);
    pointerItem.SetOriginPointerId(pointerId);
    auto pointerEvent = MMI::PointerEvent::Create();
    pointerEvent->SetPointerId(pointerId);
    pointerEvent->AddPointerItem(pointerItem);

    // Case 1: Pointer event is null
    EXPECT_FALSE(moveDragController->ConsumeMoveEvent(nullptr));

    // Case 2: Start drag flag is true
    moveDragController->SetStartDragFlag(true);
    EXPECT_FALSE(moveDragController->ConsumeMoveEvent(pointerEvent));
    moveDragController->SetStartDragFlag(false);

    // Case 3: Failed to get pointer item
    std::shared_ptr<MMI::PointerEvent> noItemEvent = MMI::PointerEvent::Create();
    noItemEvent->SetPointerId(pointerId);
    EXPECT_FALSE(moveDragController->ConsumeMoveEvent(noItemEvent));

    // Case 4: StartPointerId mismatch
    moveDragController->moveDragProperty_.pointerId_ = pointerId + 1;
    EXPECT_FALSE(moveDragController->ConsumeMoveEvent(pointerEvent));
    moveDragController->moveDragProperty_.pointerId_ = pointerId;

    // Case 5: StartPointerType mismatch
    pointerEvent->SetSourceType(MMI::PointerEvent::SOURCE_TYPE_MOUSE);
    moveDragController->moveDragProperty_.pointerType_ = pointerEvent->GetSourceType() + 1;
    EXPECT_FALSE(moveDragController->ConsumeMoveEvent(pointerEvent));
}

/**
 * @tc.name: TestConsumeMoveEventWithNotStartMove
 * @tc.desc: Verify that ConsumeMoveEvent handles not-start-move flows
 * @tc.type: FUNC
 */
HWTEST_F(MoveDragControllerTest, TestConsumeMoveEventWithNotStartMove, TestSize.Level1)
{
    if (!SceneBoardJudgement::IsSceneBoardEnabled()) {
        GTEST_SKIP() << "Skip test when SceneBoard is disabled.";
    }

    constexpr int32_t pointerId = 0;
    MMI::PointerEvent::PointerItem pointerItem;
    pointerItem.SetPointerId(pointerId);
    pointerItem.SetOriginPointerId(pointerId);
    auto pointerEvent = MMI::PointerEvent::Create();
    pointerEvent->SetPointerId(pointerId);
    pointerEvent->AddPointerItem(pointerItem);
    pointerEvent->SetSourceType(MMI::PointerEvent::SOURCE_TYPE_MOUSE);
    moveDragController->SetStartDragFlag(false);
    moveDragController->moveDragProperty_.pointerId_ = pointerId;
    moveDragController->moveDragProperty_.pointerType_ = pointerEvent->GetSourceType();
    moveDragController->isStartMove_ = false;

    // Case 1: Mouse right button & not start move
    pointerEvent->SetButtonId(MMI::PointerEvent::MOUSE_BUTTON_RIGHT);
    EXPECT_FALSE(moveDragController->ConsumeMoveEvent(pointerEvent));

    // Case 2: Mouse left button & not start move
    pointerEvent->SetButtonId(MMI::PointerEvent::MOUSE_BUTTON_LEFT);
    pointerEvent->SetPointerAction(MMI::PointerEvent::POINTER_ACTION_UNKNOWN);
    EXPECT_FALSE(moveDragController->ConsumeMoveEvent(pointerEvent));

    // Case 3: ACTION_DOWN sets hasPointDown_
    moveDragController->hasPointDown_ = false;
    pointerEvent->SetPointerAction(MMI::PointerEvent::POINTER_ACTION_DOWN);
    EXPECT_FALSE(moveDragController->ConsumeMoveEvent(pointerEvent));
    EXPECT_TRUE(moveDragController->hasPointDown_);

    // Case 4: ACTION_BUTTON_DOWN sets hasPointDown_
    moveDragController->hasPointDown_ = false;
    pointerEvent->SetPointerAction(MMI::PointerEvent::POINTER_ACTION_BUTTON_DOWN);
    EXPECT_FALSE(moveDragController->ConsumeMoveEvent(pointerEvent));
    EXPECT_TRUE(moveDragController->hasPointDown_);

    // Case 5: ACTION_UP resets hasPointDown_
    moveDragController->hasPointDown_ = true;
    pointerEvent->SetPointerAction(MMI::PointerEvent::POINTER_ACTION_UP);
    EXPECT_FALSE(moveDragController->ConsumeMoveEvent(pointerEvent));
    EXPECT_FALSE(moveDragController->hasPointDown_);

    // Case 6: ACTION_BUTTON_UP resets hasPointDown_
    moveDragController->hasPointDown_ = true;
    pointerEvent->SetPointerAction(MMI::PointerEvent::POINTER_ACTION_BUTTON_UP);
    EXPECT_FALSE(moveDragController->ConsumeMoveEvent(pointerEvent));
    EXPECT_FALSE(moveDragController->hasPointDown_);

    // Case 7: ACTION_CANCEL resets hasPointDown_
    moveDragController->hasPointDown_ = true;
    pointerEvent->SetPointerAction(MMI::PointerEvent::POINTER_ACTION_CANCEL);
    EXPECT_FALSE(moveDragController->ConsumeMoveEvent(pointerEvent));
    EXPECT_FALSE(moveDragController->hasPointDown_);
}

/**
 * @tc.name: TestConsumeMoveEventWithStartMove
 * @tc.desc: Verify that ConsumeMoveEvent handles start-move flows
 * @tc.type: FUNC
 */
HWTEST_F(MoveDragControllerTest, TestConsumeMoveEventWithStartMove, TestSize.Level1)
{
    if (!SceneBoardJudgement::IsSceneBoardEnabled()) {
        GTEST_SKIP() << "Skip test when SceneBoard is disabled.";
    }

    constexpr int32_t pointerId = 0;
    MMI::PointerEvent::PointerItem pointerItem;
    pointerItem.SetPointerId(pointerId);
    pointerItem.SetOriginPointerId(pointerId);
    auto pointerEvent = MMI::PointerEvent::Create();
    pointerEvent->SetPointerId(pointerId);
    pointerEvent->AddPointerItem(pointerItem);
    pointerEvent->SetSourceType(MMI::PointerEvent::SOURCE_TYPE_MOUSE);
    moveDragController->SetStartDragFlag(false);
    moveDragController->moveDragProperty_.pointerId_ = pointerId;
    moveDragController->moveDragProperty_.pointerType_ = pointerEvent->GetSourceType();

    // Case 1: MOVE normal flow
    moveDragController->isStartMove_ = true;
    moveDragController->moveDragIsInterrupted_ = false;
    pointerEvent->SetPointerAction(MMI::PointerEvent::POINTER_ACTION_MOVE);
    EXPECT_TRUE(moveDragController->ConsumeMoveEvent(pointerEvent));

    // Case 2: MOVE interrupted
    moveDragController->isStartMove_ = true;
    moveDragController->moveDragIsInterrupted_ = true;
    EXPECT_TRUE(moveDragController->ConsumeMoveEvent(pointerEvent));
    EXPECT_FALSE(moveDragController->isStartMove_);

    // Case 3: UP ends drag
    moveDragController->isStartMove_ = true;
    moveDragController->hasPointDown_ = true;
    moveDragController->moveDragIsInterrupted_ = false;
    pointerEvent->SetPointerAction(MMI::PointerEvent::POINTER_ACTION_UP);
    EXPECT_FALSE(moveDragController->ConsumeMoveEvent(pointerEvent));
    EXPECT_FALSE(moveDragController->hasPointDown_);
    EXPECT_FALSE(moveDragController->isStartMove_);

    // Case 4: CANCEL with hasPointDown_ is false
    moveDragController->isStartMove_ = true;
    moveDragController->hasPointDown_ = false;
    pointerEvent->SetPointerAction(MMI::PointerEvent::POINTER_ACTION_CANCEL);
    EXPECT_TRUE(moveDragController->ConsumeMoveEvent(pointerEvent));

    // Case 5: BUTTON_DOWN with moveDragIsInterrupted_ is true
    moveDragController->isStartMove_ = true;
    moveDragController->hasPointDown_ = true;
    moveDragController->moveDragIsInterrupted_ = true;
    pointerEvent->SetPointerAction(MMI::PointerEvent::POINTER_ACTION_BUTTON_DOWN);
    EXPECT_TRUE(moveDragController->ConsumeMoveEvent(pointerEvent));
    EXPECT_FALSE(moveDragController->isStartMove_);

    // Case 6: UNKNOWN action
    moveDragController->isStartMove_ = true;
    pointerEvent->SetPointerAction(MMI::PointerEvent::POINTER_ACTION_UNKNOWN);
    EXPECT_FALSE(moveDragController->ConsumeMoveEvent(pointerEvent));
}

/**
 * @tc.name: ProcessWindowDragHotAreaFunc
 * @tc.desc: test function : ProcessWindowDragHotAreaFunc
 * @tc.type: FUNC
 */
HWTEST_F(MoveDragControllerTest, ProcessWindowDragHotAreaFunc, TestSize.Level1)
{
    bool isSendHotAreaMessage = true;
    bool isDragHotAreaFuncCalled = false;
    SizeChangeReason reason = SizeChangeReason::UNDEFINED;
    auto dragHotAreaFunc = [&isDragHotAreaFuncCalled](DisplayId displayId, int32_t type, SizeChangeReason reason) {
        isDragHotAreaFuncCalled = true;
    };
    moveDragController->windowDragHotAreaFunc_ = dragHotAreaFunc;
    moveDragController->ProcessWindowDragHotAreaFunc(isSendHotAreaMessage, reason);
    EXPECT_TRUE(isDragHotAreaFuncCalled);
}

/**
 * @tc.name: ConsumeDragEvent
 * @tc.desc: test function : ConsumeDragEvent
 * @tc.type: FUNC
 */
HWTEST_F(MoveDragControllerTest, ConsumeDragEvent, TestSize.Level1)
{
    std::shared_ptr<MMI::PointerEvent> pointerEvent = MMI::PointerEvent::Create();
    moveDragController->moveDragProperty_.pointerId_ = pointerEvent->GetPointerId();
    moveDragController->moveDragProperty_.pointerType_ = pointerEvent->GetSourceType();
    if (SceneBoardJudgement::IsSceneBoardEnabled()) {
        ASSERT_NE(nullptr, pointerEvent);
        ASSERT_EQ(false, moveDragController->ConsumeDragEvent(nullptr));
        ASSERT_EQ(false, moveDragController->ConsumeDragEvent(pointerEvent));
        moveDragController->SetStartMoveFlag(true);
        ASSERT_EQ(false, moveDragController->ConsumeDragEvent(pointerEvent));
        moveDragController->SetStartMoveFlag(false);
        pointerEvent->SetPointerAction(static_cast<int32_t>(MMI::PointerEvent::POINTER_ACTION_DOWN));
        ASSERT_EQ(false, moveDragController->ConsumeDragEvent(pointerEvent));
        MMI::PointerEvent::PointerItem pointerItem;
        pointerItem.SetPointerId(0);
        pointerItem.SetOriginPointerId(0);
        pointerItem.SetWindowX(0);
        pointerItem.SetWindowY(0);
        pointerEvent->AddPointerItem(pointerItem);
        pointerEvent->SetPointerId(0);
        pointerEvent->SetPointerAction(static_cast<int32_t>(MMI::PointerEvent::POINTER_ACTION_DOWN));
        ASSERT_EQ(true, moveDragController->ConsumeDragEvent(pointerEvent));
        pointerEvent->SetSourceType(MMI::PointerEvent::SOURCE_TYPE_MOUSE);
        pointerEvent->SetButtonId(MMI::PointerEvent::MOUSE_BUTTON_RIGHT);
        ASSERT_EQ(false, moveDragController->ConsumeDragEvent(pointerEvent));
        pointerEvent->SetPointerAction(static_cast<int32_t>(MMI::PointerEvent::POINTER_ACTION_MOVE));
        ASSERT_EQ(true, moveDragController->ConsumeDragEvent(pointerEvent));
        pointerEvent->SetPointerAction(static_cast<int32_t>(MMI::PointerEvent::POINTER_ACTION_UP));
        ASSERT_EQ(true, moveDragController->ConsumeDragEvent(pointerEvent));
        pointerEvent->SetPointerAction(static_cast<int32_t>(MMI::PointerEvent::POINTER_ACTION_UNKNOWN));
        ASSERT_EQ(false, moveDragController->ConsumeDragEvent(pointerEvent));
        moveDragController->moveDragProperty_.pointerId_ = pointerItem.GetOriginPointerId() + 1;
        ASSERT_EQ(false, moveDragController->ConsumeDragEvent(pointerEvent));
        moveDragController->moveDragProperty_.pointerId_ = pointerItem.GetOriginPointerId();
        moveDragController->moveDragProperty_.pointerType_ = pointerEvent->GetSourceType() + 1;
        ASSERT_EQ(false, moveDragController->ConsumeDragEvent(pointerEvent));
    }
}

/**
 * @tc.name: ConsumeDragEvent2
 * @tc.desc: test function : ConsumeDragEvent2
 * @tc.type: FUNC
 */
HWTEST_F(MoveDragControllerTest, ConsumeDragEvent2, TestSize.Level1)
{
    std::shared_ptr<MMI::PointerEvent> pointerEvent = MMI::PointerEvent::Create();
    if (SceneBoardJudgement::IsSceneBoardEnabled()) {
        ASSERT_NE(nullptr, pointerEvent);
        WSRect originalRect = { 100, 100, 1000, 1000 };
        session_->SetSessionRect(originalRect);
        session_->SetSessionGlobalRect(originalRect);
        ASSERT_EQ(false, moveDragController->ConsumeDragEvent(nullptr));
        ASSERT_EQ(false, moveDragController->ConsumeDragEvent(pointerEvent));
        moveDragController->SetStartMoveFlag(true);
        ASSERT_EQ(false, moveDragController->ConsumeDragEvent(pointerEvent));
        moveDragController->SetStartMoveFlag(false);
        pointerEvent->SetPointerAction(static_cast<int32_t>(MMI::PointerEvent::POINTER_ACTION_DOWN));
        ASSERT_EQ(false, moveDragController->ConsumeDragEvent(pointerEvent));
        MMI::PointerEvent::PointerItem pointerItem;
        pointerItem.SetPointerId(0);
        pointerItem.SetOriginPointerId(0);
        pointerItem.SetWindowX(0);
        pointerItem.SetWindowY(0);
        pointerEvent->AddPointerItem(pointerItem);
        pointerEvent->SetPointerId(0);
        pointerEvent->SetPointerAction(static_cast<int32_t>(MMI::PointerEvent::POINTER_ACTION_DOWN));
        ASSERT_EQ(true, moveDragController->ConsumeDragEvent(pointerEvent));
        pointerEvent->SetSourceType(MMI::PointerEvent::SOURCE_TYPE_MOUSE);
        moveDragController->moveDragProperty_.pointerId_ = pointerItem.GetOriginPointerId();
        moveDragController->moveDragProperty_.pointerType_ = pointerEvent->GetSourceType();
        pointerEvent->SetButtonId(MMI::PointerEvent::MOUSE_BUTTON_RIGHT);
        ASSERT_EQ(false, moveDragController->ConsumeDragEvent(pointerEvent));
        pointerEvent->SetPointerAction(static_cast<int32_t>(MMI::PointerEvent::POINTER_ACTION_MOVE));
        ASSERT_EQ(true, moveDragController->ConsumeDragEvent(pointerEvent));
        moveDragController->moveDragIsInterrupted_ = false;
        ASSERT_EQ(true, moveDragController->ConsumeDragEvent(pointerEvent));
        moveDragController->moveDragIsInterrupted_ = true;
        pointerEvent->SetPointerAction(static_cast<int32_t>(MMI::PointerEvent::POINTER_ACTION_UP));
        ASSERT_EQ(true, moveDragController->ConsumeDragEvent(pointerEvent));
        moveDragController->hasPointDown_ = false;
        ASSERT_EQ(false, moveDragController->ConsumeDragEvent(pointerEvent));
        moveDragController->hasPointDown_ = true;
        moveDragController->moveDragIsInterrupted_ = false;
        ScreenProperty screenProperty0;
        ScreenSessionManagerClient::GetInstance().screenSessionMap_[0] =
            sptr<ScreenSession>::MakeSptr(0, screenProperty0, 0);
        moveDragController->moveDragStartDisplayId_ = 0;
        ASSERT_EQ(false, moveDragController->ConsumeDragEvent(pointerEvent));
    }
}

/**
 * @tc.name: CalcFirstMoveTargetRect
 * @tc.desc: test function : CalcFirstMoveTargetRect
 * @tc.type: FUNC
 */
HWTEST_F(MoveDragControllerTest, CalcFirstMoveTargetRect, TestSize.Level1)
{
    int res = 0;
    WSRect windowRect = { 0, 0, 0, 0 };
    moveDragController->CalcFirstMoveTargetRect(windowRect, false);
    res++;
    moveDragController->moveTempProperty_.pointerId_ = 0;
    moveDragController->CalcFirstMoveTargetRect(windowRect, false);
    EXPECT_EQ(res, 1);
    auto preIsStartMove = moveDragController->GetStartMoveFlag();
    auto preMoveTempProperty = moveDragController->moveTempProperty_;
    moveDragController->isStartMove_ = false;
    moveDragController->moveTempProperty_ = { -1, -1, -1, -1, -1, -1, -1, -1 };
    moveDragController->CalcFirstMoveTargetRect(windowRect, false);
    EXPECT_EQ(res, 1);
    moveDragController->moveTempProperty_ = { 1, 1, 1, 1, 1, 1, 1, 1 };
    moveDragController->CalcFirstMoveTargetRect(windowRect, false);
    EXPECT_EQ(res, 1);
    moveDragController->isStartMove_ = true;
    moveDragController->CalcFirstMoveTargetRect(windowRect, false);
    EXPECT_EQ(res, 1);
    moveDragController->moveTempProperty_ = { -1, -1, -1, -1, -1, -1, -1, -1 };
    moveDragController->CalcFirstMoveTargetRect(windowRect, false);
    EXPECT_EQ(res, 1);
    moveDragController->moveTempProperty_ = { 1, 1, 1, 1, 1, 1, 1, 1 };
    moveDragController->CalcFirstMoveTargetRect(windowRect, false);
    EXPECT_EQ(res, 1);
    moveDragController->CalcFirstMoveTargetRect(windowRect, true);
    EXPECT_EQ(res, 1);

    moveDragController->CalcFirstMoveTargetRect(windowRect, false);
    EXPECT_EQ(0, moveDragController->moveDragProperty_.targetRect_.posX_);
    EXPECT_EQ(0, moveDragController->moveDragProperty_.targetRect_.posY_);

    moveDragController->SetParentRect({10, 10, 10, 10});
    moveDragController->CalcFirstMoveTargetRect(windowRect, false);
    EXPECT_EQ(-10, moveDragController->moveDragProperty_.targetRect_.posX_);
    EXPECT_EQ(-10, moveDragController->moveDragProperty_.targetRect_.posY_);

    moveDragController->isStartMove_ = preIsStartMove;
    moveDragController->moveTempProperty_ = preMoveTempProperty;
}

/**
 * @tc.name: CalcFirstMoveTargetRect001
 * @tc.desc: test function : CalcFirstMoveTargetRect001
 * @tc.type: FUNC
 */
HWTEST_F(MoveDragControllerTest, CalcFirstMoveTargetRect001, TestSize.Level1)
{
    WSRect windowRect = { 1, 2, 3, 4 };
    moveDragController->InitMoveDragProperty();
    moveDragController->SetStartMoveFlag(true);
    moveDragController->CalcFirstMoveTargetRect(windowRect, true);
    WSRect targetRect =
        moveDragController->GetTargetRect(MoveDragController::TargetRectCoordinate::RELATED_TO_START_DISPLAY);
    EXPECT_EQ(targetRect.posX_, 0);
}

/**
 * @tc.name: CalcFirstMoveTargetRect002
 * @tc.desc: test function : CalcFirstMoveTargetRect002
 * @tc.type: FUNC
 */
HWTEST_F(MoveDragControllerTest, CalcFirstMoveTargetRect002, TestSize.Level1)
{
    WSRect windowRect = { 1, 2, 3, 4 };
    moveDragController->InitMoveDragProperty();
    moveDragController->moveTempProperty_ = { 1, 1, 1, 1, 1, 1, 1, 1 };
    moveDragController->isStartMove_ = true;
    moveDragController->specifyMoveStartDisplayId_ = 0;
    moveDragController->isSpecifyMoveStart_ = true;
    moveDragController->CalcFirstMoveTargetRect(windowRect, false);
    WSRect targetRect =
        moveDragController->GetTargetRect(MoveDragController::TargetRectCoordinate::RELATED_TO_START_DISPLAY);
    EXPECT_EQ(targetRect.posX_, 0);
}

/**
 * @tc.name: GetFullScreenToFloatingRect
 * @tc.desc: test function : GetFullScreenToFloatingRect
 * @tc.type: FUNC
 */
HWTEST_F(MoveDragControllerTest, GetFullScreenToFloatingRect, TestSize.Level1)
{
    WSRect originalRect = { 1, 2, 0, 4 };
    WSRect windowRect = { 5, 6, 7, 8 };
    auto preMoveTempProperty = moveDragController->moveTempProperty_;
    moveDragController->moveTempProperty_ = { -1, -1, -1, -1, -1, -1, -1, -1 };
    WSRect rect = moveDragController->GetFullScreenToFloatingRect(originalRect, windowRect);
    // move temporary property is empty
    EXPECT_EQ(originalRect.posX_, rect.posX_);
    moveDragController->moveTempProperty_ = { 1, 1, 1, 1, 1, 1, 1, 1 };
    rect = moveDragController->GetFullScreenToFloatingRect(originalRect, windowRect);
    // original rect witch is zero
    EXPECT_EQ(windowRect.posX_, rect.posX_);
    originalRect = { 1, 2, 3, 4 };
    rect = moveDragController->GetFullScreenToFloatingRect(originalRect, windowRect);
    WSRect targetRect = { 1, 2, 7, 8 };
    EXPECT_EQ(targetRect.posX_, rect.posX_);
    moveDragController->moveTempProperty_ = preMoveTempProperty;
}

/**
 * @tc.name: IsValidDragEvent
 * @tc.desc: test function : IsValidDragEvent
 * @tc.type: FUNC
 */
HWTEST_F(MoveDragControllerTest, IsValidDragEvent, TestSize.Level1)
{
    constexpr int32_t pointerId = 0;
    MMI::PointerEvent::PointerItem pointerItem;
    pointerItem.SetPointerId(pointerId);
    pointerItem.SetOriginPointerId(pointerId);
    auto pointerEvent = MMI::PointerEvent::Create();
    pointerEvent->SetPointerId(pointerId);
    pointerEvent->AddPointerItem(pointerItem);
    auto result = moveDragController->IsValidDragEvent(nullptr);
    EXPECT_EQ(result, false);
    result = moveDragController->IsValidDragEvent(pointerEvent);
    EXPECT_EQ(result, false);
    moveDragController->isStartMove_ = true;
    result = moveDragController->IsValidDragEvent(pointerEvent);
    EXPECT_EQ(result, false);
    moveDragController->isStartMove_ = false;
    pointerEvent->SetPointerAction(static_cast<int32_t>(MMI::PointerEvent::POINTER_ACTION_UP));
    result = moveDragController->IsValidDragEvent(pointerEvent);
    EXPECT_EQ(result, false);
    pointerEvent->SetPointerAction(static_cast<int32_t>(MMI::PointerEvent::POINTER_ACTION_DOWN));
    result = moveDragController->IsValidDragEvent(pointerEvent);
    EXPECT_EQ(result, true);
    pointerEvent->SetPointerAction(static_cast<int32_t>(MMI::PointerEvent::POINTER_ACTION_BUTTON_DOWN));
    result = moveDragController->IsValidDragEvent(pointerEvent);
    EXPECT_EQ(result, true);
    auto preMoveDragProperty = moveDragController->moveDragProperty_;
    moveDragController->moveDragProperty_.pointerId_ = -1;
    result = moveDragController->IsValidDragEvent(pointerEvent);
    EXPECT_EQ(result, true);
    moveDragController->moveDragProperty_ = preMoveDragProperty;
}

/**
 * @tc.name: UpdateMoveTempProperty
 * @tc.desc: test function : UpdateMoveTempProperty
 * @tc.type: FUNC
 */
HWTEST_F(MoveDragControllerTest, UpdateMoveTempProperty, TestSize.Level1)
{
    std::shared_ptr<MMI::PointerEvent> pointerEvent = MMI::PointerEvent::Create();
    ASSERT_NE(pointerEvent, nullptr);
    pointerEvent->SetSourceType(MMI::PointerEvent::SOURCE_TYPE_TOUCHSCREEN);
    pointerEvent->SetButtonId(MMI::PointerEvent::MOUSE_BUTTON_RIGHT);
    auto result = moveDragController->UpdateMoveTempProperty(pointerEvent);
    EXPECT_EQ(result, WSError::WS_ERROR_NULLPTR);
    MMI::PointerEvent::PointerItem pointerItem;
    pointerItem.SetPointerId(0);
    pointerItem.SetOriginPointerId(0);
    pointerEvent->AddPointerItem(pointerItem);
    pointerEvent->SetPointerId(0);
    pointerEvent->SetSourceType(MMI::PointerEvent::SOURCE_TYPE_MOUSE);
    result = moveDragController->UpdateMoveTempProperty(pointerEvent);
    EXPECT_EQ(result, WSError::WS_ERROR_NULLPTR);
    pointerEvent->SetSourceType(MMI::PointerEvent::SOURCE_TYPE_MOUSE);
    pointerEvent->SetButtonId(MMI::PointerEvent::MOUSE_BUTTON_RIGHT);
    result = moveDragController->UpdateMoveTempProperty(pointerEvent);
    EXPECT_EQ(result, WSError::WS_ERROR_NULLPTR);
    pointerEvent->SetButtonId(MMI::PointerEvent::MOUSE_BUTTON_LEFT);
    pointerEvent->SetPointerAction(static_cast<int32_t>(MMI::PointerEvent::POINTER_ACTION_BUTTON_DOWN));
    result = moveDragController->UpdateMoveTempProperty(pointerEvent);
    EXPECT_EQ(result, WSError::WS_OK);
    pointerEvent->SetPointerAction(static_cast<int32_t>(MMI::PointerEvent::POINTER_ACTION_DOWN));
    result = moveDragController->UpdateMoveTempProperty(pointerEvent);
    EXPECT_EQ(result, WSError::WS_OK);
    pointerEvent->SetPointerAction(static_cast<int32_t>(MMI::PointerEvent::POINTER_ACTION_MOVE));
    result = moveDragController->UpdateMoveTempProperty(pointerEvent);
    EXPECT_EQ(result, WSError::WS_OK);
    pointerEvent->SetPointerAction(static_cast<int32_t>(MMI::PointerEvent::POINTER_ACTION_UP));
    result = moveDragController->UpdateMoveTempProperty(pointerEvent);
    EXPECT_EQ(result, WSError::WS_OK);
    pointerEvent->SetPointerAction(static_cast<int32_t>(MMI::PointerEvent::POINTER_ACTION_BUTTON_UP));
    result = moveDragController->UpdateMoveTempProperty(pointerEvent);
    EXPECT_EQ(result, WSError::WS_OK);
    pointerEvent->SetPointerAction(static_cast<int32_t>(MMI::PointerEvent::POINTER_ACTION_CANCEL));
    result = moveDragController->UpdateMoveTempProperty(pointerEvent);
    EXPECT_EQ(result, WSError::WS_OK);
    pointerEvent->SetPointerAction(static_cast<int32_t>(MMI::PointerEvent::POINTER_ACTION_UNKNOWN));
    result = moveDragController->UpdateMoveTempProperty(pointerEvent);
    EXPECT_EQ(result, WSError::WS_OK);
}

/**
 * @tc.name: UpdateHotAreaType
 * @tc.desc: UpdateHotAreaType
 * @tc.type: FUNC
 */
HWTEST_F(MoveDragControllerTest, UpdateHotAreaType, TestSize.Level1)
{
    std::shared_ptr<MMI::PointerEvent> pointerEvent = MMI::PointerEvent::Create();
    ASSERT_NE(pointerEvent, nullptr);
    moveDragController->UpdateHotAreaType(pointerEvent);
    MMI::PointerEvent::PointerItem pointerItem;
    pointerItem.SetPointerId(0);
    pointerItem.SetOriginPointerId(0);
    pointerEvent->AddPointerItem(pointerItem);
    pointerEvent->SetPointerId(0);
    moveDragController->UpdateHotAreaType(pointerEvent);
    auto preWindowDragHotAreaType = moveDragController->windowDragHotAreaType_;
    moveDragController->windowDragHotAreaType_ = WINDOW_HOT_AREA_TYPE_UNDEFINED;
    moveDragController->UpdateHotAreaType(pointerEvent);
    moveDragController->windowDragHotAreaType_ = preWindowDragHotAreaType;
}

/**
 * @tc.name: OnLostFocus
 * @tc.desc: OnLostFocus
 * @tc.type: FUNC
 */
HWTEST_F(MoveDragControllerTest, OnLostFocus, TestSize.Level1)
{
    std::shared_ptr<MMI::PointerEvent> pointerEvent = MMI::PointerEvent::Create();
    ASSERT_NE(pointerEvent, nullptr);
    moveDragController->isStartMove_ = true;
    moveDragController->isStartDrag_ = false;
    moveDragController->OnLostFocus();
    moveDragController->isStartMove_ = false;
    moveDragController->isStartDrag_ = true;
    int windowHotAreaTypeOther = 1;
    moveDragController->windowDragHotAreaType_ = windowHotAreaTypeOther;
    moveDragController->OnLostFocus();
    moveDragController->windowDragHotAreaType_ = WINDOW_HOT_AREA_TYPE_UNDEFINED;
    moveDragController->OnLostFocus();
}

/**
 * @tc.name: NotifyWindowInputPidChange
 * @tc.desc: NotifyWindowInputPidChange
 * @tc.type: FUNC
 */
HWTEST_F(MoveDragControllerTest, NotifyWindowInputPidChange, TestSize.Level1)
{
    bool isServerPid = true;
    auto preCallback = moveDragController->pidChangeCallback_;
    moveDragController->NotifyWindowInputPidChange(isServerPid);
    isServerPid = false;
    moveDragController->NotifyWindowInputPidChange(isServerPid);
    moveDragController->SetNotifyWindowPidChangeCallback(nullptr);
    ASSERT_EQ(moveDragController->pidChangeCallback_, nullptr);
    isServerPid = true;
    moveDragController->NotifyWindowInputPidChange(isServerPid);
    isServerPid = false;
    moveDragController->NotifyWindowInputPidChange(isServerPid);
    moveDragController->SetNotifyWindowPidChangeCallback(preCallback);
}

/**
 * @tc.name: HasPointDown
 * @tc.desc: HasPointDown
 * @tc.type: FUNC
 */
HWTEST_F(MoveDragControllerTest, HasPointDown, TestSize.Level1)
{
    bool preHasPointDown = moveDragController->hasPointDown_;
    moveDragController->hasPointDown_ = true;
    bool res = moveDragController->HasPointDown();
    EXPECT_EQ(res, true);
    moveDragController->hasPointDown_ = false;
    res = moveDragController->HasPointDown();
    EXPECT_EQ(res, false);
    moveDragController->hasPointDown_ = preHasPointDown;
}

/**
 * @tc.name: GetOriginalPointerPosX
 * @tc.desc: GetOriginalPointerPosX
 * @tc.type: FUNC
 */
HWTEST_F(MoveDragControllerTest, GetOriginalPointerPosX, TestSize.Level1)
{
    int32_t posX = moveDragController->moveDragProperty_.originalPointerPosX_;
    int32_t res = moveDragController->GetOriginalPointerPosX();
    EXPECT_EQ(posX, res);
}

/**
 * @tc.name: GetOriginalPointerPosY
 * @tc.desc: GetOriginalPointerPosY
 * @tc.type: FUNC
 */
HWTEST_F(MoveDragControllerTest, GetOriginalPointerPosY, TestSize.Level1)
{
    int32_t posY = moveDragController->moveDragProperty_.originalPointerPosY_;
    int32_t res = moveDragController->GetOriginalPointerPosY();
    EXPECT_EQ(posY, res);
}

/**
 * @tc.name: GetPointerType
 * @tc.desc: GetPointerType
 * @tc.type: FUNC
 */
HWTEST_F(MoveDragControllerTest, GetPointerType, TestSize.Level1)
{
    int32_t testType = MMI::PointerEvent::SOURCE_TYPE_TOUCHSCREEN;
    moveDragController->moveDragProperty_.pointerType_ = testType;
    EXPECT_EQ(testType, moveDragController->GetPointerType());
}

/**
 * @tc.name: GetNewAddedDisplayIdsDuringMoveDrag
 * @tc.desc: test function : GetNewAddedDisplayIdsDuringMoveDrag
 * @tc.type: FUNC
 */
HWTEST_F(MoveDragControllerTest, GetNewAddedDisplayIdsDuringMoveDrag, TestSize.Level1)
{
    std::set<uint64_t> res = moveDragController->GetDisplayIdsDuringMoveDrag();
    EXPECT_EQ(true, res.empty());
}

/**
 * @tc.name: GetNewAddedDisplayIdsDuringMoveDrag02
 * @tc.desc: test function : GetNewAddedDisplayIdsDuringMoveDrag02
 * @tc.type: FUNC
 */
HWTEST_F(MoveDragControllerTest, GetNewAddedDisplayIdsDuringMoveDrag02, TestSize.Level1)
{
    ScreenSessionManagerClient::GetInstance().screenSessionMap_.clear();
    std::set<uint64_t> res = moveDragController->GetNewAddedDisplayIdsDuringMoveDrag();
    EXPECT_EQ(true, res.empty());
    moveDragController->displayIdSetDuringMoveDrag_.insert(0);
    moveDragController->displayIdSetDuringMoveDrag_.insert(1001);
    ScreenProperty screenProperty0;
    ScreenSessionManagerClient::GetInstance().screenSessionMap_[0] =
        sptr<ScreenSession>::MakeSptr(0, screenProperty0, 0);
    res = moveDragController->GetNewAddedDisplayIdsDuringMoveDrag();
    EXPECT_EQ(true, res.empty());
    ScreenSessionManagerClient::GetInstance().screenSessionMap_.clear();
}

/**
 * @tc.name: MoveDragInterrupted
 * @tc.desc: test function : MoveDragInterrupted
 * @tc.type: FUNC
 */
HWTEST_F(MoveDragControllerTest, MoveDragInterrupted, TestSize.Level1)
{
    moveDragController->MoveDragInterrupted();
    EXPECT_EQ(false, moveDragController->GetStartDragFlag());
    EXPECT_EQ(false, moveDragController->GetStartMoveFlag());
    EXPECT_EQ(false, moveDragController->hasPointDown_);
    moveDragController->isStartMove_ = true;
    EXPECT_EQ(false, moveDragController->GetStartDragFlag());
    EXPECT_EQ(true, moveDragController->GetStartMoveFlag());
    EXPECT_EQ(false, moveDragController->hasPointDown_);
    moveDragController->isStartMove_ = false;

    moveDragController->SetStartDragFlag(true);
    moveDragController->MoveDragInterrupted();
    EXPECT_EQ(false, moveDragController->GetStartDragFlag());
}

/**
 * @tc.name: ResetCrossMoveDragProperty
 * @tc.desc: test function : ResetCrossMoveDragProperty
 * @tc.type: FUNC
 */
HWTEST_F(MoveDragControllerTest, ResetCrossMoveDragProperty, TestSize.Level1)
{
    moveDragController->ResetCrossMoveDragProperty();
    EXPECT_EQ(false, moveDragController->hasPointDown_);
}

/**
 * @tc.name: OnConnect
 * @tc.desc: test function : OnConnect
 * @tc.type: FUNC
 */
HWTEST_F(MoveDragControllerTest, OnConnect, TestSize.Level1)
{
    ScreenId screenId = 1001;
    moveDragController->OnConnect(screenId);
    EXPECT_EQ(moveDragController->moveDragIsInterrupted_, true);
}

/**
 * @tc.name: OnDisconnect
 * @tc.desc: test function : OnDisconnect
 * @tc.type: FUNC
 */
HWTEST_F(MoveDragControllerTest, OnDisconnect, TestSize.Level1)
{
    ScreenId screenId = 1001;
    moveDragController->OnDisconnect(screenId);
    EXPECT_EQ(moveDragController->moveDragIsInterrupted_, true);
}

/**
 * @tc.name: OnChange
 * @tc.desc: test function : OnChange
 * @tc.type: FUNC
 */
HWTEST_F(MoveDragControllerTest, OnChange, TestSize.Level1)
{
    ScreenId screenId = 1001;
    moveDragController->OnChange(screenId);
    EXPECT_EQ(moveDragController->moveDragIsInterrupted_, true);
}

/**
 * @tc.name: StopMoving
 * @tc.desc: test function : StopMoving
 * @tc.type: FUNC
 */
HWTEST_F(MoveDragControllerTest, StopMoving, TestSize.Level1)
{
    moveDragController->isStartMove_ = true;
    moveDragController->StopMoving();
    EXPECT_EQ(false, moveDragController->GetStartMoveFlag());
    EXPECT_EQ(false, moveDragController->hasPointDown_);
}

/**
 * @tc.name: HandleStartMovingWithCoordinate
 * @tc.desc: test function : HandleStartMovingWithCoordinate
 * @tc.type: FUNC
 */
HWTEST_F(MoveDragControllerTest, HandleStartMovingWithCoordinate, TestSize.Level1)
{
    WSRect winRect = { 200, 200, 1000, 1000 };
    MoveDragController::MoveCoordinateProperty property = { 100, 50, 300, 500, 0, winRect };
    moveDragController->HandleStartMovingWithCoordinate(property);
    EXPECT_EQ(300, moveDragController->moveTempProperty_.lastDownPointerPosX_);
    EXPECT_EQ(500, moveDragController->moveTempProperty_.lastDownPointerPosY_);
    EXPECT_EQ(300, moveDragController->moveTempProperty_.lastMovePointerPosX_);
    EXPECT_EQ(500, moveDragController->moveTempProperty_.lastMovePointerPosY_);
    EXPECT_EQ(100, moveDragController->moveTempProperty_.lastDownPointerWindowX_);
    EXPECT_EQ(50, moveDragController->moveTempProperty_.lastDownPointerWindowY_);
    EXPECT_EQ(200, moveDragController->moveDragProperty_.targetRect_.posX_);
    EXPECT_EQ(200, moveDragController->moveDragProperty_.targetRect_.posY_);
    EXPECT_EQ(1000, moveDragController->moveDragProperty_.targetRect_.width_);
    EXPECT_EQ(1000, moveDragController->moveDragProperty_.targetRect_.height_);
}

/**
 * @tc.name: SetSpecifyMoveStartDisplay
 * @tc.desc: test function : SetSpecifyMoveStartDisplay
 * @tc.type: FUNC
 */
HWTEST_F(MoveDragControllerTest, SetSpecifyMoveStartDisplay, TestSize.Level1)
{
    moveDragController->specifyMoveStartDisplayId_ = 0;
    moveDragController->isSpecifyMoveStart_ = false;
    moveDragController->SetSpecifyMoveStartDisplay(1);
    EXPECT_EQ(1, moveDragController->specifyMoveStartDisplayId_);
    EXPECT_EQ(true, moveDragController->isSpecifyMoveStart_);
}

/**
 * @tc.name: ClearSpecifyMoveStartDisplay
 * @tc.desc: test function : ClearSpecifyMoveStartDisplay
 * @tc.type: FUNC
 */
HWTEST_F(MoveDragControllerTest, ClearSpecifyMoveStartDisplay, TestSize.Level1)
{
    moveDragController->specifyMoveStartDisplayId_ = 1;
    moveDragController->isSpecifyMoveStart_ = true;
    moveDragController->ClearSpecifyMoveStartDisplay();
    EXPECT_EQ(DISPLAY_ID_INVALID, moveDragController->specifyMoveStartDisplayId_);
    EXPECT_EQ(false, moveDragController->isSpecifyMoveStart_);
}

/**
 * @tc.name: MapRectFromTargetToStart
 * @tc.desc: test function : MapRectFromTargetToStart
 * @tc.type: FUNC
 */
HWTEST_F(MoveDragControllerTest, MapRectFromTargetToStart, TestSize.Level1)
{
    WSRect winRect = { 200, 200, 1000, 1000 };
    ScreenSessionManagerClient::GetInstance().screenSessionMap_.clear();
    WSRect ret = moveDragController->MapRectFromTargetToStart(winRect, 0);
    EXPECT_EQ(ret, winRect);

    ScreenProperty screenProperty0;
    ScreenSessionManagerClient::GetInstance().screenSessionMap_[0] =
        sptr<ScreenSession>::MakeSptr(0, screenProperty0, 0);
    ret = moveDragController->MapRectFromTargetToStart(winRect, 0);
    int32_t currentDisplayOffsetX = static_cast<int32_t>(screenProperty0.GetStartX());
    int32_t currentDisplayOffsetY = static_cast<int32_t>(screenProperty0.GetStartY());
    WSRect testRect = { winRect.posX_ + currentDisplayOffsetX - moveDragController->originalDisplayOffsetX_,
                        winRect.posY_ + currentDisplayOffsetY - moveDragController->originalDisplayOffsetY_,
                        winRect.width_,
                        winRect.height_ };
    EXPECT_EQ(ret, testRect);
}

/**
 * @tc.name: GetTargetRectByDisplayId
 * @tc.desc: test function : GetTargetRectByDisplayId
 * @tc.type: FUNC
 */
HWTEST_F(MoveDragControllerTest, GetTargetRectByDisplayId, TestSize.Level1)
{
    ScreenSessionManagerClient::GetInstance().screenSessionMap_.clear();
    WSRect ret = moveDragController->GetTargetRectByDisplayId(0);
    EXPECT_EQ(ret, moveDragController->moveDragProperty_.targetRect_);
    ScreenProperty screenProperty0;
    ScreenSessionManagerClient::GetInstance().screenSessionMap_[0] =
        sptr<ScreenSession>::MakeSptr(0, screenProperty0, 0);
    ret = moveDragController->GetTargetRectByDisplayId(0);
    int32_t currentDisplayOffsetX = static_cast<int32_t>(screenProperty0.GetStartX());
    int32_t currentDisplayOffsetY = static_cast<int32_t>(screenProperty0.GetStartY());
    WSRect testRect = { moveDragController->moveDragProperty_.targetRect_.posX_ +
                            moveDragController->originalDisplayOffsetX_ - currentDisplayOffsetX,
                        moveDragController->moveDragProperty_.targetRect_.posY_ +
                            moveDragController->originalDisplayOffsetY_ - currentDisplayOffsetY,
                        moveDragController->moveDragProperty_.targetRect_.width_,
                        moveDragController->moveDragProperty_.targetRect_.height_ };
    EXPECT_EQ(ret, testRect);
}

/**
 * @tc.name: UpdateMoveAvailableArea
 * @tc.desc: test function : UpdateMoveAvailableArea
 * @tc.type: FUNC
 */
HWTEST_F(MoveDragControllerTest, UpdateMoveAvailableArea, TestSize.Level1)
{
    moveDragController->UpdateMoveAvailableArea(-1);
    EXPECT_EQ(moveDragController->moveAvailableArea_.posX_, 0);
    SessionOption option = {
        .rsId_ = 0,
        .isExtend_ = false,
        .screenId_ = 0,
    };
    ScreenEvent screenEvent = ScreenEvent::CONNECTED;
    ScreenSessionManagerClient::GetInstance().OnScreenConnectionChanged(option, screenEvent);
    moveDragController->UpdateMoveAvailableArea(0);
    EXPECT_EQ(moveDragController->moveAvailableArea_.posX_, 0);
}

/**
 * @tc.name: GetMoveInputBarStartDisplayId
 * @tc.desc: test function : GetMoveInputBarStartDisplayId
 * @tc.type: FUNC
 */
HWTEST_F(MoveDragControllerTest, GetMoveInputBarStartDisplayId, TestSize.Level1)
{
    auto ret = moveDragController->GetMoveInputBarStartDisplayId();
    EXPECT_EQ(moveDragController->moveInputBarStartDisplayId_, ret);
}

/**
 * @tc.name: SetCurrentScreenProperty
 * @tc.desc: test function : SetCurrentScreenProperty
 * @tc.type: FUNC
 */
HWTEST_F(MoveDragControllerTest, SetCurrentScreenProperty, TestSize.Level1)
{
    SessionOption option = {
        .rsId_ = 0,
        .isExtend_ = false,
        .screenId_ = 0,
    };
    ScreenEvent screenEvent = ScreenEvent::CONNECTED;
    ScreenSessionManagerClient::GetInstance().OnScreenConnectionChanged(option, screenEvent);
    moveDragController->SetCurrentScreenProperty(-1);
    EXPECT_EQ(moveDragController->screenSizeProperty_.currentDisplayStartX, 0);
    moveDragController->SetCurrentScreenProperty(0);
    EXPECT_EQ(moveDragController->screenSizeProperty_.currentDisplayStartX, 0);
}

/**
 * @tc.name: UpdateSubWindowGravityWhenFollow
 * @tc.desc: UpdateSubWindowGravityWhenFollow
 * @tc.type: FUNC
 */
HWTEST_F(MoveDragControllerTest, UpdateSubWindowGravityWhenFollow01, TestSize.Level1)
{
    SessionInfo info;
    sptr<SceneSession> followSession = sptr<SceneSession>::MakeSptr(info, nullptr);

    sptr<MoveDragController> followController =
        sptr<MoveDragController>::MakeSptr(wptr(followSession));
    struct RSSurfaceNodeConfig rsSurfaceNodeConfig;
    std::shared_ptr<RSSurfaceNode> surfaceNode = RSSurfaceNode::Create(rsSurfaceNodeConfig, RSSurfaceNodeType::DEFAULT);

    followController->type_ = AreaType::UNDEFINED;
    moveDragController->UpdateSubWindowGravityWhenFollow(nullptr, nullptr);
    auto modifier = surfaceNode->GetModifierByType(ModifierNG::RSModifierType::CLIP_TO_FRAME);
    ASSERT_NE(modifier, nullptr);

    moveDragController->UpdateSubWindowGravityWhenFollow(nullptr, surfaceNode);
    modifier = surfaceNode->GetModifierByType(ModifierNG::RSModifierType::CLIP_TO_FRAME);
    ASSERT_NE(modifier, nullptr);

    moveDragController->UpdateSubWindowGravityWhenFollow(followController, surfaceNode);
    modifier = surfaceNode->GetModifierByType(ModifierNG::RSModifierType::CLIP_TO_FRAME);
    ASSERT_NE(modifier, nullptr);

    followController->type_ = AreaType::TOP;
    moveDragController->UpdateSubWindowGravityWhenFollow(followController, surfaceNode);
    modifier = surfaceNode->GetModifierByType(ModifierNG::RSModifierType::CLIP_TO_FRAME);
    ASSERT_NE(modifier, nullptr);
}

/**
 * @tc.name: TestUpdateTargetRectOnDragEvent
 * @tc.desc: Verify normal flows of UpdateTargetRectOnDragEvent
 * @tc.type: FUNC
 */
HWTEST_F(MoveDragControllerTest, TestUpdateTargetRectOnDragEvent, TestSize.Level1)
{
    WSRect originalRect = {0, 0, 200, 100};
    moveDragController->moveDragProperty_.originalRect_ = originalRect;
    moveDragController->limits_ = WindowLimits(400, 400, 200, 50, FLT_MAX, 0.0f);
    moveDragController->decoration_ = {0, 0, 0, 0};
    moveDragController->type_ = AreaType::RIGHT;

    // Prepare pointer event + item
    auto pointerEvent = MMI::PointerEvent::Create();
    MMI::PointerEvent::PointerItem pointerItem;
    pointerItem.SetPointerId(1);
    pointerItem.SetOriginPointerId(1);
    pointerItem.SetDisplayX(100);  // Current pointer position
    pointerItem.SetDisplayY(200);
    pointerEvent->SetPointerId(1);
    pointerEvent->AddPointerItem(pointerItem);
    pointerEvent->SetTargetDisplayId(0);

    // Case 1: Cross-display disabled & displayId mismatch → no update
    moveDragController->moveDragProperty_.targetRect_ = WSRect::EMPTY_RECT;
    moveDragController->supportCrossDisplay_ = false;
    moveDragController->moveDragStartDisplayId_ = 1;
    moveDragController->UpdateTargetRectOnDragEvent(pointerEvent, SizeChangeReason::DRAG);
    EXPECT_EQ(moveDragController->moveDragProperty_.targetRect_, WSRect::EMPTY_RECT);

    // Case 2: Cross-display enabled but aspect ratio is 0
    moveDragController->moveDragProperty_.targetRect_ = WSRect::EMPTY_RECT;
    moveDragController->supportCrossDisplay_ = true;
    moveDragController->aspectRatio_ = 0.0f;
    moveDragController->UpdateTargetRectOnDragEvent(pointerEvent, SizeChangeReason::DRAG);
    EXPECT_NE(moveDragController->moveDragProperty_.targetRect_, WSRect::EMPTY_RECT);

    // Case 3: PointerEvent‘s displayId equals moveDragStartDisplayId and aspect ratio is not 0
    moveDragController->moveDragProperty_.targetRect_ = WSRect::EMPTY_RECT;
    pointerEvent->SetTargetDisplayId(0);
    moveDragController->moveDragStartDisplayId_ = 0;
    moveDragController->aspectRatio_ = 1.0f;
    moveDragController->UpdateTargetRectOnDragEvent(pointerEvent, SizeChangeReason::DRAG);
    EXPECT_NE(moveDragController->moveDragProperty_.targetRect_, WSRect::EMPTY_RECT);
}

/**
 * @tc.name: TestGetGravity
 * @tc.desc: Verify GetGravity returns correct value or TOP_LEFT when not found
 * @tc.type: FUNC
 */
HWTEST_F(MoveDragControllerTest, TestGetGravity, TestSize.Level1)
{
    // Case 1: type in GRAVITY_MAP
    auto gravity = moveDragController->GetGravity(AreaType::TOP);
    EXPECT_EQ(gravity, Gravity::BOTTOM_LEFT);

    // Case 2: type not in GRAVITY_MAP
    gravity = moveDragController->GetGravity(static_cast<AreaType>(999));
    EXPECT_EQ(gravity, Gravity::TOP_LEFT);
}

/**
 * @tc.name: TestRestoreToPreDragGravity
 * @tc.desc: Verify RestoreToPreDragGravity handles null, no-value, and valid cases
 * @tc.type: FUNC
 */
HWTEST_F(MoveDragControllerTest, TestRestoreToPreDragGravity, TestSize.Level1)
{
    // Case 1: surfaceNode is null
    auto res = moveDragController->RestoreToPreDragGravity(nullptr);
    EXPECT_FALSE(res);

    struct RSSurfaceNodeConfig config;
    auto surfaceNode = RSSurfaceNode::Create(config);

    // Case 2: preDragGravity is nullopt
    moveDragController->preDragGravity_ = std::nullopt;
    res = moveDragController->RestoreToPreDragGravity(surfaceNode);
    EXPECT_FALSE(res);

    // Case 3: preDragGravity has value
    moveDragController->preDragGravity_ = Gravity::BOTTOM_RIGHT;
    res = moveDragController->RestoreToPreDragGravity(surfaceNode);
    Gravity gravity = surfaceNode->GetStagingProperties().GetFrameGravity();
    EXPECT_TRUE(res);
    EXPECT_EQ(gravity, Gravity::BOTTOM_RIGHT);
    EXPECT_EQ(moveDragController->preDragGravity_, std::nullopt);
}

/**
 * @tc.name: TestMoveDragControllerCtor
 * @tc.desc: Verify MoveDragController constructor initializes members correctly
 */
HWTEST_F(MoveDragControllerTest, TestMoveDragControllerCtor, TestSize.Level0)
{
    // Case 1: sceneSession is not null
    ASSERT_EQ(moveDragController->persistentId_, session_->GetPersistentId());
    ASSERT_EQ(moveDragController->winType_, session_->GetWindowType());

    // Case 2: sceneSession is null
    sptr<MoveDragController> moveDragControllerNull = sptr<MoveDragController>::MakeSptr(wptr<SceneSession>(nullptr));
    ASSERT_EQ(moveDragControllerNull->persistentId_, INVALID_WINDOW_ID);
}

/**
 * @tc.name: TestConvertXYByAspectRatio
 * @tc.desc: Verify aspect-ratio-based coordinate conversion with area type adjustment
 * @tc.type: FUNC
 */
HWTEST_F(MoveDragControllerTest, TestConvertXYByAspectRatio, TestSize.Level1)
{
    int32_t tx, ty;

    // Case 1: aspectRatio near zero → no modification
    {
        tx = 100;
        ty = 50;
        moveDragController->mainMoveAxis_ = MoveDragController::AxisType::X_AXIS;
        moveDragController->type_ = AreaType::RIGHT;
        moveDragController->ConvertXYByAspectRatio(tx, ty, 0.0f);

        EXPECT_EQ(tx, 100);
        EXPECT_EQ(ty, 50);
    }

    // Case 2: mainMoveAxis = X_AXIS, positive correlation
    {
        tx = 120;
        ty = 0;
        moveDragController->mainMoveAxis_ = MoveDragController::AxisType::X_AXIS;
        moveDragController->type_ = AreaType::RIGHT; // POSITIVE_CORRELATION
        moveDragController->ConvertXYByAspectRatio(tx, ty, 2.0f);
        // ty = tx / ratio = 120 / 2 = 60 → positive correlation → 60
        EXPECT_EQ(tx, 120);
        EXPECT_EQ(ty, 60);
    }

    // Case 3: mainMoveAxis = X_AXIS, negative correlation
    {
        tx = 90;
        ty = 0;
        moveDragController->mainMoveAxis_ = MoveDragController::AxisType::X_AXIS;
        moveDragController->type_ = AreaType::LEFT; // NEGATIVE_CORRELATION
        moveDragController->ConvertXYByAspectRatio(tx, ty, 3.0f);
        // temp ty = 90 / 3 = 30 → negative correlation → -30
        EXPECT_EQ(tx, 90);
        EXPECT_EQ(ty, -30);
    }

    // Case 4: mainMoveAxis = Y_AXIS, positive correlation
    {
        tx = 0;
        ty = 50;
        moveDragController->mainMoveAxis_ = MoveDragController::AxisType::Y_AXIS;
        moveDragController->type_ = AreaType::BOTTOM; // POSITIVE_CORRELATION
        moveDragController->ConvertXYByAspectRatio(tx, ty, 1.5f);
        // tx = ty * ratio = 50 * 1.5 = 75 → positive correlation → 75
        EXPECT_EQ(tx, 75);
        EXPECT_EQ(ty, 50);
    }

    // Case 5: mainMoveAxis = Y_AXIS, negative correlation
    {
        tx = 0;
        ty = 40;
        moveDragController->mainMoveAxis_ = MoveDragController::AxisType::Y_AXIS;
        moveDragController->type_ = AreaType::TOP; // NEGATIVE_CORRELATION
        moveDragController->ConvertXYByAspectRatio(tx, ty, 0.5f);
        // temp tx = 40 * 0.5 = 20 → negative correlation → -20
        EXPECT_EQ(tx, -20);
        EXPECT_EQ(ty, 40);
    }
}

/**
 * @tc.name: TestHandleDragEnd
 * @tc.desc: Verify return value and key state changes of HandleDragEnd
 * @tc.type: FUNC
 */
HWTEST_F(MoveDragControllerTest, TestHandleDragEnd, TestSize.Level1)
{
    auto pointerEvent = MMI::PointerEvent::Create();
    pointerEvent->SetTargetDisplayId(1);

    // Prepare a valid screen session (display 0)
    ScreenSessionManagerClient::GetInstance().screenSessionMap_.clear();
    auto screenSession = sptr<ScreenSession>::MakeSptr();
    ScreenProperty prop;
    prop.SetStartX(0);
    prop.SetStartY(0);
    RRect bounds({ 0, 0, 300, 200}, 0.0f, 0.0f);
    prop.SetBounds(bounds);
    screenSession->SetScreenProperty(prop);

    ScreenSessionManagerClient::GetInstance().screenSessionMap_.insert({ 0, screenSession });

    // Case 1: hasPointDown_ = false → return true
    {
        moveDragController->hasPointDown_ = false; // no pointer down
        bool ret = moveDragController->HandleDragEnd(pointerEvent);
        EXPECT_TRUE(ret);
    }

    // Case 2: screenRect invalid → return true
    {
        moveDragController->hasPointDown_ = true;
        moveDragController->moveDragStartDisplayId_ = 5; // not in screenSessionMap → invalid
        moveDragController->moveDragIsInterrupted_ = false;
        bool ret = moveDragController->HandleDragEnd(pointerEvent);
        EXPECT_TRUE(ret);
        EXPECT_FALSE(moveDragController->hasPointDown_);
    }

    // Case 3: normal flow → return true
    {
        moveDragController->hasPointDown_ = true;
        moveDragController->moveDragStartDisplayId_ = 0;
        moveDragController->moveDragIsInterrupted_ = false;
        // Make GetTargetRect() return rect overlapping screen rect
        moveDragController->moveDragProperty_.targetRect_ = { 0, 0, 50, 50 };
        bool ret = moveDragController->HandleDragEnd(pointerEvent);
        EXPECT_TRUE(ret);
        // Because target rect overlaps screen 0, drag end display should remain 0
        EXPECT_EQ(moveDragController->moveDragEndDisplayId_, 0);
    }

    // Case 4: pointer ends on different display → choose pointerEvent->TargetDisplayId()
    {
        moveDragController->hasPointDown_ = true;
        moveDragController->moveDragStartDisplayId_ = 0;
        // Make targetRect NOT overlap screen rect → force choose pointerEvent display
        moveDragController->moveDragProperty_.targetRect_ = { 400, 400, 100, 100 };
        pointerEvent->SetTargetDisplayId(7);
        bool ret = moveDragController->HandleDragEnd(pointerEvent);
        EXPECT_TRUE(ret);
        EXPECT_EQ(moveDragController->moveDragEndDisplayId_, 7);
    }

    ScreenSessionManagerClient::GetInstance().screenSessionMap_.clear();
}

/**
 * @tc.name: TestShouldBlockCrossDisplay
 * @tc.desc: Verify the blocking rule for cross-display dragging
 * @tc.type: FUNC
 */
HWTEST_F(MoveDragControllerTest, TestShouldBlockCrossDisplay, TestSize.Level1)
{
    std::shared_ptr<MMI::PointerEvent> pointerEvent = MMI::PointerEvent::Create();

    // Case 1: targetDisplayId != moveDragStartDisplayId_ AND supportCrossDisplay_ == false → should block
    {
        pointerEvent->SetTargetDisplayId(1);
        moveDragController->moveDragStartDisplayId_ = 0;
        moveDragController->supportCrossDisplay_ = false;

        bool ret = moveDragController->ShouldBlockCrossDisplay(pointerEvent);
        EXPECT_TRUE(ret);
    }

    // Case 2: targetDisplayId != moveDragStartDisplayId_ AND supportCrossDisplay_ == true → should NOT block
    {
        pointerEvent->SetTargetDisplayId(2);
        moveDragController->moveDragStartDisplayId_ = 0;
        moveDragController->supportCrossDisplay_ = true;

        bool ret = moveDragController->ShouldBlockCrossDisplay(pointerEvent);
        EXPECT_FALSE(ret);
    }

    // Case 3: targetDisplayId == moveDragStartDisplayId_ → never block, regardless of supportCrossDisplay_
    {
        pointerEvent->SetTargetDisplayId(5);
        moveDragController->moveDragStartDisplayId_ = 5;
        moveDragController->supportCrossDisplay_ = false; // or true

        bool ret = moveDragController->ShouldBlockCrossDisplay(pointerEvent);
        EXPECT_FALSE(ret);
    }
}

/**
 * @tc.name: TestComputeOffsetFromStart
 * @tc.desc: Verify deltaX/deltaY calculation in unified coordinates
 * @tc.type: FUNC
 */
HWTEST_F(MoveDragControllerTest, TestComputeOffsetFromStart, TestSize.Level1)
{
    // Prepare pointer event + item
    auto pointerEvent = MMI::PointerEvent::Create();
    MMI::PointerEvent::PointerItem pointerItem;
    pointerItem.SetPointerId(1);
    pointerItem.SetOriginPointerId(1);
    pointerItem.SetDisplayX(100);  // Current pointer position
    pointerItem.SetDisplayY(200);
    pointerEvent->SetPointerId(1);
    pointerEvent->AddPointerItem(pointerItem);
    pointerEvent->SetTargetDisplayId(0);

    // Prepare ScreenSessionManagerClient screen offset
    ScreenSessionManagerClient::GetInstance().screenSessionMap_.clear();
    auto screenSession = sptr<ScreenSession>::MakeSptr();
    ScreenProperty prop;
    prop.SetStartX(10);  // We define the screen offset
    prop.SetStartY(20);
    screenSession->SetScreenProperty(prop);
    ScreenSessionManagerClient::GetInstance().screenSessionMap_.insert(std::make_pair(0, screenSession));

    // Setup MoveDragController internal state
    // Assume original pointer position was at (50, 80)
    moveDragController->moveDragProperty_.originalPointerPosX_ = 50;
    moveDragController->moveDragProperty_.originalPointerPosY_ = 80;
    // Assume original display offset at drag start was (0, 0)
    moveDragController->originalDisplayOffsetX_ = 0;
    moveDragController->originalDisplayOffsetY_ = 0;
    // Normal scaling
    moveDragController->moveDragProperty_.scaleX_ = 2.0f;
    moveDragController->moveDragProperty_.scaleY_ = 4.0f;
    // Disable special path
    moveDragController->isAdaptToProportionalScale_ = false;

    // Case 1: Normal path, with scaling
    {
        auto [dx, dy] = moveDragController->ComputeOffsetFromStart(pointerEvent);

        // Unified current coordinates: (10 + 100, 20 + 200) = (110, 220)
        // Unified origin: (0 + 50, 0 + 80) = (50, 80)
        // Raw delta: (60, 140)
        // With scale: (60 / 2.0, 140 / 4.0) = (30, 35)
        EXPECT_EQ(dx, 30);
        EXPECT_EQ(dy, 35);
    }

    // Case 2: scaleX or scaleY is near zero → fallback raw delta
    {
        moveDragController->moveDragProperty_.scaleX_ = 0.0f; // near zero
        moveDragController->moveDragProperty_.scaleY_ = 1.0f;

        auto [dx, dy] = moveDragController->ComputeOffsetFromStart(pointerEvent);

        // Raw delta = (60, 140)
        EXPECT_EQ(dx, 60);
        EXPECT_EQ(dy, 140);
    }

    // Case 3: isAdaptToProportionalScale_ = true → no scaling
    {
        moveDragController->isAdaptToProportionalScale_ = true;

        auto [dx, dy] = moveDragController->ComputeOffsetFromStart(pointerEvent);

        // Raw delta still (60, 140)
        EXPECT_EQ(dx, 60);
        EXPECT_EQ(dy, 140);
    }
}

/**
 * @tc.name: TestShouldOpenMoveResampleAllBranches
 * @tc.desc: Verify all decision branches of ShouldOpenMoveResample
 * @tc.type: FUNC
 */
HWTEST_F(MoveDragControllerTest, TestShouldOpenMoveResampleAllBranches, TestSize.Level1)
{
    // Case 1: Global enable switch disabled → false
    moveDragController->enableMoveResample_ = false;
    EXPECT_FALSE(moveDragController->ShouldOpenMoveResample(MMI::PointerEvent::SOURCE_TYPE_TOUCHSCREEN));
    moveDragController->enableMoveResample_ = true;

    // Case 2: Pointer type is not touchscreen → false
    EXPECT_FALSE(moveDragController->ShouldOpenMoveResample(MMI::PointerEvent::SOURCE_TYPE_MOUSE));

    // Case 3: Input window does not support move resample → false
    moveDragController->winType_ = WindowType::WINDOW_TYPE_INPUT_METHOD_FLOAT;
    EXPECT_FALSE(moveDragController->ShouldOpenMoveResample(MMI::PointerEvent::SOURCE_TYPE_TOUCHSCREEN));

    // Case 7: All conditions satisfied → true
    moveDragController->winType_ = WindowType::APP_WINDOW_BASE;
    EXPECT_TRUE(moveDragController->ShouldOpenMoveResample(MMI::PointerEvent::SOURCE_TYPE_TOUCHSCREEN));
}

/**
 * @tc.name: TestSyncPropertiesFromSceneSession
 * @tc.desc: Verify MoveDragController correctly syncs properties from SceneSession
 * @tc.type: FUNC
 */
HWTEST_F(MoveDragControllerTest, TestSyncPropertiesFromSceneSession, TestSize.Level1)
{
    // Case 1: session is null → return false
    {
        moveDragController->sceneSession_ = nullptr;
        bool ret = moveDragController->SyncPropertiesFromSceneSession();
        EXPECT_FALSE(ret);
    }

    // Case 2: session is valid → properties copied correctly
    {
        moveDragController->sceneSession_ = wptr(session_);
        bool ret = moveDragController->SyncPropertiesFromSceneSession();
        EXPECT_TRUE(ret);
    }
}

/**
 * @tc.name: TestUpdateTargetRectOnMoveEvent
 * @tc.desc: Verify return values of UpdateTargetRectOnMoveEvent without mocking
 * @tc.type: FUNC
 */
HWTEST_F(MoveDragControllerTest, TestUpdateTargetRectOnMoveEvent, TestSize.Level1)
{
    std::shared_ptr<MMI::PointerEvent> pointerEvent = MMI::PointerEvent::Create();
    pointerEvent->SetPointerAction(MMI::PointerEvent::POINTER_ACTION_MOVE);
    pointerEvent->SetSourceType(MMI::PointerEvent::SOURCE_TYPE_TOUCHSCREEN);
    pointerEvent->SetActionTime(123456);

    // Case 1: Cross-display is blocked → UNCHANGED
    {
        moveDragController->moveDragStartDisplayId_ = 0;
        moveDragController->supportCrossDisplay_ = false; // Disable cross-display
        pointerEvent->SetTargetDisplayId(1);              // Different display → block

        auto state = moveDragController->UpdateTargetRectOnMoveEvent(pointerEvent, SizeChangeReason::DRAG_MOVE);
        EXPECT_EQ(state, TargetRectUpdateMode::NONE);
    }

    // Case 2: Move resample activated → RESAMPLE_SCHEDULED
    {
        moveDragController->moveDragStartDisplayId_ = 0;
        moveDragController->supportCrossDisplay_ = true; // Not blocked now
        pointerEvent->SetTargetDisplayId(0);             // Same display
        moveDragController->moveDragProperty_.isMoveResampleActive_ = true;  // Enable resample

        auto state = moveDragController->UpdateTargetRectOnMoveEvent(pointerEvent, SizeChangeReason::DRAG_MOVE);
        EXPECT_EQ(state, TargetRectUpdateMode::RESAMPLE_SCHEDULED);
    }

    // Case 3: Direct update → UPDATED_IMMEDIATELY
    {
        moveDragController->moveDragStartDisplayId_ = 0;
        moveDragController->supportCrossDisplay_ = true; // Allowed
        moveDragController->moveDragProperty_.isMoveResampleActive_ = false; // Disable resample
        pointerEvent->SetTargetDisplayId(0);             // Same display

        auto state = moveDragController->UpdateTargetRectOnMoveEvent(pointerEvent, SizeChangeReason::DRAG_MOVE);
        EXPECT_EQ(state, TargetRectUpdateMode::UPDATED_IMMEDIATELY);

        moveDragController->moveDragProperty_.isMoveResampleActive_ = true; // Enable resample but reason is DRAG_END
        state = moveDragController->UpdateTargetRectOnMoveEvent(pointerEvent, SizeChangeReason::DRAG_END);
        EXPECT_EQ(state, TargetRectUpdateMode::UPDATED_IMMEDIATELY);
    }
}

/**
 * @tc.name: TestResampleTargetRectOnVsyncAllBranches
 * @tc.desc: Verify ResampleTargetRectOnVsync behavior under different move/resample states
 * @tc.type: FUNC
 */
HWTEST_F(MoveDragControllerTest, TestResampleTargetRectOnVsyncAllBranches, TestSize.Level1)
{
    constexpr int64_t vsyncTime = 1000;
    auto& prop = moveDragController->moveDragProperty_;

    // Case 1: Drag not started
    moveDragController->isStartMove_ = false;

    {
        auto [mode, _] = moveDragController->ResampleTargetRectOnVsync(vsyncTime);
        EXPECT_EQ(mode, TargetRectUpdateMode::NONE);
    }

    // Case 2: Drag started but move resample is inactive
    moveDragController->isStartMove_ = true;
    prop.isMoveResampleActive_ = false;
    prop.isResampleFpsRangeChecked_ = true; // Skip FPS logic

    {
        auto [mode, _] = moveDragController->ResampleTargetRectOnVsync(vsyncTime);
        EXPECT_EQ(mode, TargetRectUpdateMode::NONE);
    }

    // Case 3: Drag started and move resample is active
    prop.isMoveResampleActive_ = true;
    prop.isResampleFpsRangeChecked_ = true;

    {
        auto [mode, _] = moveDragController->ResampleTargetRectOnVsync(vsyncTime);
        EXPECT_EQ(mode, TargetRectUpdateMode::UPDATED_IMMEDIATELY);
    }
}

/**
 * @tc.name: TestUpdateResampleActivationByFpsNoOp
 * @tc.desc: Verify no-op branches of UpdateResampleActivationByFps
 * @tc.type: FUNC
 */
HWTEST_F(MoveDragControllerTest, TestUpdateResampleActivationByFpsNoOp, TestSize.Level1)
{
    auto& prop = moveDragController->moveDragProperty_;

    // Case 1: move resample not activated → no-op
    prop.isMoveResampleActive_ = false;
    prop.isResampleFpsRangeChecked_ = false;
    moveDragController->UpdateResampleActivationByFps();
    EXPECT_FALSE(prop.isMoveResampleActive_);
    EXPECT_FALSE(prop.isResampleFpsRangeChecked_);

    // Case 2: FPS range already checked → no-op
    prop.isMoveResampleActive_ = true;
    prop.isResampleFpsRangeChecked_ = true;
    moveDragController->UpdateResampleActivationByFps();
    EXPECT_TRUE(prop.isMoveResampleActive_);
    EXPECT_TRUE(prop.isResampleFpsRangeChecked_);
}

/**
 * @tc.name: TestUpdateResampleActivationByFpsRangeCheck
 * @tc.desc: Verify FPS range validation behavior of UpdateResampleActivationByFps
 * @tc.type: FUNC
 */
HWTEST_F(MoveDragControllerTest, TestUpdateResampleActivationByFpsRangeCheck, TestSize.Level1)
{
    constexpr uint32_t FPS_30  = 30;
    constexpr uint32_t FPS_60  = 60;
    constexpr uint32_t FPS_120 = 120;

    std::optional<uint32_t> fps = FPS_60;
    EXPECT_CALL(*mockVsyncStation_, GetFps())
        .WillRepeatedly(InvokeWithoutArgs([&fps]() { return fps; }));

    auto& prop = moveDragController->moveDragProperty_;

    // Case 1: failed to get FPS → disable
    prop.isMoveResampleActive_ = true;
    prop.isResampleFpsRangeChecked_ = false;
    fps = std::nullopt;
    moveDragController->UpdateResampleActivationByFps();
    EXPECT_FALSE(prop.isMoveResampleActive_);
    EXPECT_TRUE(prop.isResampleFpsRangeChecked_);

    // Case 2: FPS below minimum threshold → disable
    prop.isMoveResampleActive_ = true;
    prop.isResampleFpsRangeChecked_ = false;
    moveDragController->resampleMinFps_ = FPS_60;
    fps = FPS_30;
    moveDragController->UpdateResampleActivationByFps();
    EXPECT_FALSE(prop.isMoveResampleActive_);
    EXPECT_TRUE(prop.isResampleFpsRangeChecked_);
    moveDragController->resampleMinFps_.reset();

    // Case 3: FPS above maximum threshold → disable
    prop.isMoveResampleActive_ = true;
    prop.isResampleFpsRangeChecked_ = false;
    moveDragController->resampleMaxFps_ = FPS_60;
    fps = FPS_120;
    moveDragController->UpdateResampleActivationByFps();
    EXPECT_FALSE(prop.isMoveResampleActive_);
    EXPECT_TRUE(prop.isResampleFpsRangeChecked_);
    moveDragController->resampleMaxFps_.reset();

    // Case 4: FPS within range → keep activated
    prop.isMoveResampleActive_ = true;
    prop.isResampleFpsRangeChecked_ = false;
    fps = FPS_60;
    moveDragController->UpdateResampleActivationByFps();
    EXPECT_TRUE(prop.isMoveResampleActive_);
    EXPECT_TRUE(prop.isResampleFpsRangeChecked_);

    // Case 5: explicit min/max range and FPS inside → keep activated
    prop.isMoveResampleActive_ = true;
    prop.isResampleFpsRangeChecked_ = false;
    moveDragController->resampleMinFps_ = FPS_30;
    moveDragController->resampleMaxFps_ = FPS_120;
    fps = FPS_60;
    moveDragController->UpdateResampleActivationByFps();
    EXPECT_TRUE(prop.isMoveResampleActive_);
    EXPECT_TRUE(prop.isResampleFpsRangeChecked_);
}
} // namespace
} // namespace Rosen
} // namespace OHOS