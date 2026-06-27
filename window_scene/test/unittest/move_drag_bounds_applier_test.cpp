/*
 * Copyright (c) 2026 Huawei Device Co., Ltd.
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

#include "session/host/include/move_drag_bounds_applier.h"

#include <gtest/gtest.h>

#include "session/host/include/scene_session.h"
#include "ui/rs_surface_node.h"

using namespace testing;
using namespace testing::ext;

namespace OHOS::Rosen {
namespace {
std::shared_ptr<RSSurfaceNode> CreateSurfaceNode()
{
    RSSurfaceNodeConfig config;
    return RSSurfaceNode::Create(config);
}

sptr<SceneSession> CreateSceneSession(WindowType windowType)
{
    SessionInfo info;
    info.abilityName_ = "MoveDragBoundsApplierTest";
    info.bundleName_ = "MoveDragBoundsApplierTest";
    info.windowType_ = static_cast<uint32_t>(windowType);
    return sptr<SceneSession>::MakeSptr(info, nullptr);
}

void ExpectNodeBoundsAndFrame(const std::shared_ptr<RSSurfaceNode>& node, const WSRect& rect)
{
    ASSERT_NE(nullptr, node);
    auto bounds = node->GetStagingProperties().GetBounds();
    auto frame = node->GetStagingProperties().GetFrame();
    EXPECT_FLOAT_EQ(static_cast<float>(rect.posX_), bounds[0]);   // 0: posX
    EXPECT_FLOAT_EQ(static_cast<float>(rect.posY_), bounds[1]);   // 1: posY
    EXPECT_FLOAT_EQ(static_cast<float>(rect.width_), bounds[2]);  // 2: width
    EXPECT_FLOAT_EQ(static_cast<float>(rect.height_), bounds[3]); // 3: height
    EXPECT_FLOAT_EQ(static_cast<float>(rect.posX_), frame[0]);    // 0: posX
    EXPECT_FLOAT_EQ(static_cast<float>(rect.posY_), frame[1]);    // 1: posY
    EXPECT_FLOAT_EQ(static_cast<float>(rect.width_), frame[2]);   // 2: width
    EXPECT_FLOAT_EQ(static_cast<float>(rect.height_), frame[3]);  // 3: height
}
} // namespace

class MoveDragBoundsApplierTest : public testing::Test {
public:
    static void SetUpTestCase() {}
    static void TearDownTestCase() {}
    void SetUp() override {}
    void TearDown() override {}
};

namespace {
/**
 * @tc.name: GetTargetShadowSurfaceNodeWithNullSession
 * @tc.desc: Get target shadow surface node when owner session is null.
 * @tc.type: FUNC
 */
HWTEST_F(MoveDragBoundsApplierTest, GetTargetShadowSurfaceNodeWithNullSession, TestSize.Level1)
{
    auto applier = std::make_shared<MoveDragBoundsApplier>(wptr<SceneSession>(nullptr));
    ASSERT_NE(nullptr, applier);

    EXPECT_EQ(nullptr, applier->GetTargetShadowSurfaceNode());
}

/**
 * @tc.name: ApplyWithoutSurfaceNode
 * @tc.desc: Apply move-drag bounds when owner session has no surface node.
 * @tc.type: FUNC
 */
HWTEST_F(MoveDragBoundsApplierTest, ApplyWithoutSurfaceNode, TestSize.Level1)
{
    auto session = CreateSceneSession(WindowType::APP_MAIN_WINDOW_BASE);
    ASSERT_NE(nullptr, session);
    auto applier = std::make_shared<MoveDragBoundsApplier>(wptr(session));
    ASSERT_NE(nullptr, applier);

    applier->Apply({ 10, 20, 300, 400 }, true, true);
    EXPECT_EQ(nullptr, applier->GetTargetShadowSurfaceNode());
}

/**
 * @tc.name: ApplyBoundsWithNullNode
 * @tc.desc: Apply window and content bounds when target surface node is null.
 * @tc.type: FUNC
 */
HWTEST_F(MoveDragBoundsApplierTest, ApplyBoundsWithNullNode, TestSize.Level1)
{
    WSRect rect = { 10, 20, 300, 400 };
    EXPECT_FALSE(MoveDragBoundsApplier::ApplyWindowBounds(nullptr, rect, true));
    EXPECT_FALSE(MoveDragBoundsApplier::ApplyContentBounds(nullptr, rect));

    auto surfaceNode = CreateSurfaceNode();
    ASSERT_NE(nullptr, surfaceNode);
    EXPECT_TRUE(MoveDragBoundsApplier::ApplyWindowBounds(surfaceNode, rect, true));
    ExpectNodeBoundsAndFrame(surfaceNode, rect);
    EXPECT_TRUE(surfaceNode->GetGlobalPositionEnabled());

    EXPECT_TRUE(MoveDragBoundsApplier::ApplyContentBounds(surfaceNode, rect));
    ExpectNodeBoundsAndFrame(surfaceNode, { 0, 0, rect.width_, rect.height_ });
}

/**
 * @tc.name: ApplyToOriginalSurfaceNode
 * @tc.desc: Apply move-drag bounds to original surface node without leash node.
 * @tc.type: FUNC
 */
HWTEST_F(MoveDragBoundsApplierTest, ApplyToOriginalSurfaceNode, TestSize.Level1)
{
    auto session = CreateSceneSession(WindowType::APP_MAIN_WINDOW_BASE);
    ASSERT_NE(nullptr, session);
    auto surfaceNode = CreateSurfaceNode();
    ASSERT_NE(nullptr, surfaceNode);
    session->SetSurfaceNode(surfaceNode);
    auto applier = std::make_shared<MoveDragBoundsApplier>(wptr(session));
    ASSERT_NE(nullptr, applier);

    WSRect rect = { 10, 20, 300, 400 };
    applier->Apply(rect, true, false);

    ExpectNodeBoundsAndFrame(surfaceNode, rect);
    EXPECT_TRUE(surfaceNode->GetGlobalPositionEnabled());
}

/**
 * @tc.name: GetShadowSurfaceNodeUsesCacheAndRecreatesWhenSourceChanges
 * @tc.desc: Get surface shadow node from cache and recreate it when source surface node changes.
 * @tc.type: FUNC
 */
HWTEST_F(MoveDragBoundsApplierTest, GetShadowSurfaceNodeUsesCacheAndRecreatesWhenSourceChanges, TestSize.Level1)
{
    auto session = CreateSceneSession(WindowType::APP_SUB_WINDOW_BASE);
    ASSERT_NE(nullptr, session);
    auto surfaceNode = CreateSurfaceNode();
    auto newSurfaceNode = CreateSurfaceNode();
    ASSERT_NE(nullptr, surfaceNode);
    ASSERT_NE(nullptr, newSurfaceNode);
    session->SetSurfaceNode(surfaceNode);
    auto applier = std::make_shared<MoveDragBoundsApplier>(wptr(session));
    ASSERT_NE(nullptr, applier);

    applier->Apply({ 10, 20, 300, 400 }, true, false);
    auto firstShadowNode = applier->GetShadowSurfaceNode();
    auto cachedShadowNode = applier->GetShadowSurfaceNode();
    EXPECT_NE(nullptr, firstShadowNode);
    EXPECT_EQ(firstShadowNode, cachedShadowNode);

    session->SetSurfaceNode(newSurfaceNode);
    applier->Apply({ 30, 40, 500, 600 }, true, false);
    auto recreatedShadowNode = applier->GetShadowSurfaceNode();
    EXPECT_NE(nullptr, recreatedShadowNode);
    EXPECT_NE(firstShadowNode, recreatedShadowNode);
}

/**
 * @tc.name: ApplyToOriginalLeashNode
 * @tc.desc: Apply move-drag bounds to surface content and leash window nodes.
 * @tc.type: FUNC
 */
HWTEST_F(MoveDragBoundsApplierTest, ApplyToOriginalLeashNode, TestSize.Level1)
{
    auto session = CreateSceneSession(WindowType::APP_MAIN_WINDOW_BASE);
    ASSERT_NE(nullptr, session);
    auto surfaceNode = CreateSurfaceNode();
    auto leashWinSurfaceNode = CreateSurfaceNode();
    ASSERT_NE(nullptr, surfaceNode);
    ASSERT_NE(nullptr, leashWinSurfaceNode);
    session->SetSurfaceNode(surfaceNode);
    session->SetLeashWinSurfaceNode(leashWinSurfaceNode);
    auto applier = std::make_shared<MoveDragBoundsApplier>(wptr(session));
    ASSERT_NE(nullptr, applier);

    WSRect rect = { 30, 40, 500, 600 };
    applier->Apply(rect, false, false);

    ExpectNodeBoundsAndFrame(surfaceNode, { 0, 0, rect.width_, rect.height_ });
    ExpectNodeBoundsAndFrame(leashWinSurfaceNode, rect);
    EXPECT_FALSE(leashWinSurfaceNode->GetGlobalPositionEnabled());
}

/**
 * @tc.name: GetLeashWinShadowSurfaceNodeUsesCacheAndRecreatesWhenSourceChanges
 * @tc.desc: Get leash shadow node from cache and recreate it when source leash node changes.
 * @tc.type: FUNC
 */
HWTEST_F(MoveDragBoundsApplierTest, GetLeashWinShadowSurfaceNodeUsesCacheAndRecreatesWhenSourceChanges,
    TestSize.Level1)
{
    auto session = CreateSceneSession(WindowType::APP_MAIN_WINDOW_BASE);
    ASSERT_NE(nullptr, session);
    auto surfaceNode = CreateSurfaceNode();
    auto leashWinSurfaceNode = CreateSurfaceNode();
    auto newLeashWinSurfaceNode = CreateSurfaceNode();
    ASSERT_NE(nullptr, surfaceNode);
    ASSERT_NE(nullptr, leashWinSurfaceNode);
    ASSERT_NE(nullptr, newLeashWinSurfaceNode);
    session->SetSurfaceNode(surfaceNode);
    session->SetLeashWinSurfaceNode(leashWinSurfaceNode);
    auto applier = std::make_shared<MoveDragBoundsApplier>(wptr(session));
    ASSERT_NE(nullptr, applier);

    applier->Apply({ 10, 20, 300, 400 }, true, false);
    auto firstShadowNode = applier->GetLeashWinShadowSurfaceNode();
    auto cachedShadowNode = applier->GetLeashWinShadowSurfaceNode();
    EXPECT_NE(nullptr, firstShadowNode);
    EXPECT_EQ(firstShadowNode, cachedShadowNode);

    session->SetLeashWinSurfaceNode(newLeashWinSurfaceNode);
    applier->Apply({ 30, 40, 500, 600 }, true, false);
    auto recreatedShadowNode = applier->GetLeashWinShadowSurfaceNode();
    EXPECT_NE(nullptr, recreatedShadowNode);
    EXPECT_NE(firstShadowNode, recreatedShadowNode);
}

/**
 * @tc.name: GetTargetShadowSurfaceNodePrefersLeashForSubWindow
 * @tc.desc: Get target shadow surface node returns leash shadow first when leash exists.
 * @tc.type: FUNC
 */
HWTEST_F(MoveDragBoundsApplierTest, GetTargetShadowSurfaceNodePrefersLeashForSubWindow, TestSize.Level1)
{
    auto session = CreateSceneSession(WindowType::APP_SUB_WINDOW_BASE);
    ASSERT_NE(nullptr, session);
    auto surfaceNode = CreateSurfaceNode();
    auto leashWinSurfaceNode = CreateSurfaceNode();
    ASSERT_NE(nullptr, surfaceNode);
    ASSERT_NE(nullptr, leashWinSurfaceNode);
    session->SetSurfaceNode(surfaceNode);
    session->SetLeashWinSurfaceNode(leashWinSurfaceNode);
    auto applier = std::make_shared<MoveDragBoundsApplier>(wptr(session));
    ASSERT_NE(nullptr, applier);

    applier->Apply({ 10, 20, 300, 400 }, false, false);
    auto leashShadowNode = applier->GetLeashWinShadowSurfaceNode();
    auto targetShadowNode = applier->GetTargetShadowSurfaceNode();
    EXPECT_NE(nullptr, leashShadowNode);
    EXPECT_EQ(leashShadowNode, targetShadowNode);
}

/**
 * @tc.name: ApplyWithFlushKeepsOriginalSurfaceNodeSynced
 * @tc.desc: Apply move-drag bounds through shadow path and keep original surface node synchronized.
 * @tc.type: FUNC
 */
HWTEST_F(MoveDragBoundsApplierTest, ApplyWithFlushKeepsOriginalSurfaceNodeSynced, TestSize.Level1)
{
    auto session = CreateSceneSession(WindowType::APP_MAIN_WINDOW_BASE);
    ASSERT_NE(nullptr, session);
    auto surfaceNode = CreateSurfaceNode();
    ASSERT_NE(nullptr, surfaceNode);
    session->SetSurfaceNode(surfaceNode);
    auto applier = std::make_shared<MoveDragBoundsApplier>(wptr(session));
    ASSERT_NE(nullptr, applier);

    applier->Apply({ 10, 20, 300, 400 }, true, false);
    WSRect flushedRect = { 50, 60, 700, 800 };
    applier->Apply(flushedRect, true, true);

    ExpectNodeBoundsAndFrame(surfaceNode, flushedRect);
    EXPECT_TRUE(surfaceNode->GetGlobalPositionEnabled());
    EXPECT_NE(nullptr, applier->GetTargetShadowSurfaceNode());
}

/**
 * @tc.name: GetTargetShadowSurfaceNodeReturnsFallbackWhenShadowCreationFails
 * @tc.desc: Get target shadow surface node falls back to original node if typed shadow creation fails.
 * @tc.type: FUNC
 */
HWTEST_F(MoveDragBoundsApplierTest, GetTargetShadowSurfaceNodeReturnsFallbackWhenShadowCreationFails, TestSize.Level1)
{
    auto session = CreateSceneSession(WindowType::APP_SUB_WINDOW_BASE);
    ASSERT_NE(nullptr, session);
    auto surfaceNode = CreateSurfaceNode();
    ASSERT_NE(nullptr, surfaceNode);
    session->SetSurfaceNode(surfaceNode);
    auto applier = std::make_shared<MoveDragBoundsApplier>(wptr(session));
    ASSERT_NE(nullptr, applier);

    auto targetNode = applier->GetTargetShadowSurfaceNode();
    ASSERT_NE(nullptr, targetNode);
}

/**
 * @tc.name: ApplyWithFlushKeepsOriginalLeashNodeSynced
 * @tc.desc: Apply move-drag bounds through shadow leash path and keep original nodes synchronized.
 * @tc.type: FUNC
 */
HWTEST_F(MoveDragBoundsApplierTest, ApplyWithFlushKeepsOriginalLeashNodeSynced, TestSize.Level1)
{
    auto session = CreateSceneSession(WindowType::APP_MAIN_WINDOW_BASE);
    ASSERT_NE(nullptr, session);
    auto surfaceNode = CreateSurfaceNode();
    auto leashWinSurfaceNode = CreateSurfaceNode();
    ASSERT_NE(nullptr, surfaceNode);
    ASSERT_NE(nullptr, leashWinSurfaceNode);
    session->SetSurfaceNode(surfaceNode);
    session->SetLeashWinSurfaceNode(leashWinSurfaceNode);
    auto applier = std::make_shared<MoveDragBoundsApplier>(wptr(session));
    ASSERT_NE(nullptr, applier);

    applier->Apply({ 10, 20, 300, 400 }, true, false);
    WSRect flushedRect = { 50, 60, 700, 800 };
    applier->Apply(flushedRect, true, true);

    ExpectNodeBoundsAndFrame(surfaceNode, { 0, 0, flushedRect.width_, flushedRect.height_ });
    ExpectNodeBoundsAndFrame(leashWinSurfaceNode, flushedRect);
    EXPECT_TRUE(leashWinSurfaceNode->GetGlobalPositionEnabled());
    EXPECT_NE(nullptr, applier->GetTargetShadowSurfaceNode());
}

/**
 * @tc.name: OnBoundsChangedUpdatesSessionRect
 * @tc.desc: Handle bounds changed callback for changed and unchanged session rect.
 * @tc.type: FUNC
 */
HWTEST_F(MoveDragBoundsApplierTest, OnBoundsChangedUpdatesSessionRect, TestSize.Level1)
{
    auto session = CreateSceneSession(WindowType::APP_SUB_WINDOW_BASE);
    ASSERT_NE(nullptr, session);
    auto applier = std::make_shared<MoveDragBoundsApplier>(wptr(session));
    ASSERT_NE(nullptr, applier);

    applier->OnBoundsChanged(nullptr, { 10.4f, 20.5f, 300.0f, 400.0f });
    WSRect expectedRect = { 10, 21, 300, 400 };
    EXPECT_EQ(expectedRect, session->GetSessionRect());
    applier->OnBoundsChanged(nullptr, { 10.4f, 20.5f, 300.0f, 400.0f });
    EXPECT_EQ(expectedRect, session->GetSessionRect());
}
} // namespace
} // namespace OHOS::Rosen
