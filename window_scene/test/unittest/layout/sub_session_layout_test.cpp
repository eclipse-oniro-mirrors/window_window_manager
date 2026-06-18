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

#include <gtest/gtest.h>
#include <gmock/gmock.h>
#include <ui/rs_surface_node.h>

#include "session/host/include/session.h"
#include "session/host/include/main_session.h"
#include "session/host/include/sub_session.h"
#include "session/screen/include/screen_session.h"
#include "test/mock/mock_session_stage.h"
#include "window_helper.h"
#include "window_manager_hilog.h"
#include "window_property.h"
#include "window_session_property.h"

using namespace testing;
using namespace testing::ext;

namespace OHOS {
namespace Rosen {
class SubSessionLayoutTest : public testing::Test {
public:
    static void SetUpTestCase();
    static void TearDownTestCase();
    void SetUp() override;
    void TearDown() override;

private:
    RSSurfaceNode::SharedPtr CreateRSSurfaceNode();

protected:
    // Helper function to create SessionInfo with test name
    SessionInfo CreateSessionInfo(const std::string& name) const
    {
        SessionInfo info;
        info.bundleName_ = name;
        info.moduleName_ = name;
        info.abilityName_ = name;
        return info;
    }
};

void SubSessionLayoutTest::SetUpTestCase() {}

void SubSessionLayoutTest::TearDownTestCase() {}

void SubSessionLayoutTest::SetUp() {}

void SubSessionLayoutTest::TearDown() {}

RSSurfaceNode::SharedPtr SubSessionLayoutTest::CreateRSSurfaceNode()
{
    struct RSSurfaceNodeConfig rsSurfaceNodeConfig;
    rsSurfaceNodeConfig.SurfaceNodeName = "WindowSessionTestSurfaceNode";
    auto surfaceNode = RSSurfaceNode::Create(rsSurfaceNodeConfig);
    return surfaceNode;
}

namespace {
/**
 * @tc.name: HandleCrossSurfaceNodeByWindowAnchor
 * @tc.desc: Check cloneNodeDuringCross when handleCrossSurfaceNodeByWindowAnchor
 * @tc.type: FUNC
 */
HWTEST_F(SubSessionLayoutTest, HandleCrossSurfaceNodeByWindowAnchor, TestSize.Level1)
{
    SessionInfo info;
    info.abilityName_ = "HandleCrossSurfaceNodeByWindowAnchor";
    info.bundleName_ = "HandleCrossSurfaceNodeByWindowAnchor";
    sptr<SubSession> sceneSession = sptr<SubSession>::MakeSptr(info, nullptr);
    sceneSession->GetSessionProperty()->SetWindowType(WindowType::WINDOW_TYPE_APP_SUB_WINDOW);
    sceneSession->HandleCrossSurfaceNodeByWindowAnchor(SizeChangeReason::UNDEFINED,
        std::numeric_limits<uint32_t>::max());
    sceneSession->HandleCrossSurfaceNodeByWindowAnchor(SizeChangeReason::UNDEFINED, 0);
    sceneSession->SetFindScenePanelRsNodeByZOrderFunc([this](uint64_t screenId, uint32_t targetZOrder) {
        return CreateRSSurfaceNode();
    });
    struct RSSurfaceNodeConfig rsSurfaceNodeConfig;
    rsSurfaceNodeConfig.SurfaceNodeName = info.abilityName_;
    RSSurfaceNodeType rsSurfaceNodeType = RSSurfaceNodeType::DEFAULT;
    std::shared_ptr<RSSurfaceNode> surfaceNode = RSSurfaceNode::Create(rsSurfaceNodeConfig, rsSurfaceNodeType);
    ASSERT_NE(surfaceNode, nullptr);
    sceneSession->SetSurfaceNode(surfaceNode);

    sceneSession->cloneNodeCountDuringCross_.store(0);
    sceneSession->HandleCrossSurfaceNodeByWindowAnchor(SizeChangeReason::DRAG, 0);
    EXPECT_EQ(1, sceneSession->cloneNodeCountDuringCross_.load());
    sceneSession->HandleCrossSurfaceNodeByWindowAnchor(SizeChangeReason::DRAG_MOVE, 0);
    EXPECT_EQ(2, sceneSession->cloneNodeCountDuringCross_.load());
    sceneSession->HandleCrossSurfaceNodeByWindowAnchor(SizeChangeReason::DRAG_END, 0);
    EXPECT_EQ(1, sceneSession->cloneNodeCountDuringCross_.load());
    sceneSession->HandleCrossSurfaceNodeByWindowAnchor(SizeChangeReason::UNDEFINED, 0);
    EXPECT_EQ(1, sceneSession->cloneNodeCountDuringCross_.load());
}

/**
 * @tc.name: RequestUpdateAttachedWindowLimits01
 * @tc.desc: Test sub window updates own limits only
 * @tc.type: FUNC
 */
HWTEST_F(SubSessionLayoutTest, RequestUpdateAttachedWindowLimits01, TestSize.Level1)
{
    auto info = CreateSessionInfo("RequestUpdateAttachedWindowLimits01");
    sptr<SubSession> subSession = sptr<SubSession>::MakeSptr(info, nullptr);

    int32_t sourcePersistentId = 1001;
    WindowLimits attachedLimits = { 2000, 1000, 200, 300, 0.0f, 0.0f, 0.0f, PixelUnit::PX };
    bool isIntersectedHeightLimit = true;
    bool isIntersectedWidthLimit = true;

    // Test with null sessionStage_
    subSession->sessionStage_ = nullptr;
    WSError ret = subSession->RequestUpdateAttachedWindowLimits(sourcePersistentId, attachedLimits,
        isIntersectedHeightLimit, isIntersectedWidthLimit, INVALID_SESSION_ID);
    EXPECT_EQ(WSError::WS_ERROR_NULLPTR, ret);

    // Test with valid sessionStage_
    sptr<SessionStageMocker> sessionStageMock = sptr<SessionStageMocker>::MakeSptr();
    subSession->sessionStage_ = sessionStageMock;

    EXPECT_CALL(*sessionStageMock, UpdateAttachedWindowLimits(
        sourcePersistentId, testing::_, isIntersectedHeightLimit, isIntersectedWidthLimit))
        .Times(1).WillOnce(testing::Return(WSError::WS_OK));

    ret = subSession->RequestUpdateAttachedWindowLimits(sourcePersistentId, attachedLimits,
        isIntersectedHeightLimit, isIntersectedWidthLimit, INVALID_SESSION_ID);
    EXPECT_EQ(WSError::WS_OK, ret);
}

/**
 * @tc.name: RequestUpdateAttachedWindowLimits02
 * @tc.desc: Test sub window with VP unit limits
 * @tc.type: FUNC
 */
HWTEST_F(SubSessionLayoutTest, RequestUpdateAttachedWindowLimits02, TestSize.Level1)
{
    auto info = CreateSessionInfo("RequestUpdateAttachedWindowLimits02");
    sptr<SubSession> subSession = sptr<SubSession>::MakeSptr(info, nullptr);

    int32_t sourcePersistentId = 1002;
    WindowLimits attachedLimits = { 1000, 500, 50, 100, 0.0f, 0.0f, 0.0f, PixelUnit::VP };
    bool isIntersectedHeightLimit = false;
    bool isIntersectedWidthLimit = true;

    // Test with null sessionStage_
    subSession->sessionStage_ = nullptr;
    WSError ret = subSession->RequestUpdateAttachedWindowLimits(sourcePersistentId, attachedLimits,
        isIntersectedHeightLimit, isIntersectedWidthLimit, INVALID_SESSION_ID);
    EXPECT_EQ(WSError::WS_ERROR_NULLPTR, ret);

    // Test with valid sessionStage_
    sptr<SessionStageMocker> sessionStageMock = sptr<SessionStageMocker>::MakeSptr();
    subSession->sessionStage_ = sessionStageMock;

    EXPECT_CALL(*sessionStageMock, UpdateAttachedWindowLimits(
        sourcePersistentId, testing::_, isIntersectedHeightLimit, isIntersectedWidthLimit))
        .Times(1).WillOnce(testing::Return(WSError::WS_OK));

    ret = subSession->RequestUpdateAttachedWindowLimits(sourcePersistentId, attachedLimits,
        isIntersectedHeightLimit, isIntersectedWidthLimit, INVALID_SESSION_ID);
    EXPECT_EQ(WSError::WS_OK, ret);
}

/**
 * @tc.name: RequestRemoveAttachedWindowLimits01
 * @tc.desc: Test sub window removes own limits only
 * @tc.type: FUNC
 */
HWTEST_F(SubSessionLayoutTest, RequestRemoveAttachedWindowLimits01, TestSize.Level1)
{
    auto info = CreateSessionInfo("RequestRemoveAttachedWindowLimits01");
    sptr<SubSession> subSession = sptr<SubSession>::MakeSptr(info, nullptr);

    int32_t sourcePersistentId = 2001;

    // Test with null sessionStage_
    subSession->sessionStage_ = nullptr;
    WSError ret = subSession->RequestRemoveAttachedWindowLimits(sourcePersistentId, INVALID_SESSION_ID);
    EXPECT_EQ(WSError::WS_ERROR_NULLPTR, ret);

    // Test with valid sessionStage_
    sptr<SessionStageMocker> sessionStageMock = sptr<SessionStageMocker>::MakeSptr();
    subSession->sessionStage_ = sessionStageMock;

    EXPECT_CALL(*sessionStageMock, RemoveAttachedWindowLimits(sourcePersistentId))
        .Times(1).WillOnce(testing::Return(WSError::WS_OK));

    ret = subSession->RequestRemoveAttachedWindowLimits(sourcePersistentId, INVALID_SESSION_ID);
    EXPECT_EQ(WSError::WS_OK, ret);
}

/**
 * @tc.name: RequestUpdateAttachedWindowLimits03
 * @tc.desc: Test sub window RequestUpdateAttachedWindowLimits with failure
 * @tc.type: FUNC
 */
HWTEST_F(SubSessionLayoutTest, RequestUpdateAttachedWindowLimits03, TestSize.Level1)
{
    auto info = CreateSessionInfo("RequestUpdateAttachedWindowLimits03");
    sptr<SubSession> subSession = sptr<SubSession>::MakeSptr(info, nullptr);

    int32_t sourcePersistentId = 1003;
    WindowLimits attachedLimits = { 1600, 800, 100, 200, 0.0f, 0.0f, 0.0f, PixelUnit::PX };
    bool isIntersectedHeightLimit = true;
    bool isIntersectedWidthLimit = false;

    // Test with valid sessionStage_ but UpdateAttachedWindowLimits fails
    sptr<SessionStageMocker> sessionStageMock = sptr<SessionStageMocker>::MakeSptr();
    subSession->sessionStage_ = sessionStageMock;

    EXPECT_CALL(*sessionStageMock, UpdateAttachedWindowLimits(
        sourcePersistentId, testing::_, isIntersectedHeightLimit, isIntersectedWidthLimit))
        .Times(1).WillOnce(testing::Return(WSError::WS_ERROR_IPC_FAILED));

    WSError ret = subSession->RequestUpdateAttachedWindowLimits(sourcePersistentId, attachedLimits,
        isIntersectedHeightLimit, isIntersectedWidthLimit, INVALID_SESSION_ID);
    EXPECT_EQ(WSError::WS_ERROR_IPC_FAILED, ret);
}

/**
 * @tc.name: RequestRemoveAttachedWindowLimits02
 * @tc.desc: Test sub window RequestRemoveAttachedWindowLimits with failure
 * @tc.type: FUNC
 */
HWTEST_F(SubSessionLayoutTest, RequestRemoveAttachedWindowLimits02, TestSize.Level1)
{
    auto info = CreateSessionInfo("RequestRemoveAttachedWindowLimits02");
    sptr<SubSession> subSession = sptr<SubSession>::MakeSptr(info, nullptr);

    int32_t sourcePersistentId = 2002;

    // Test with valid sessionStage_ but RemoveAttachedWindowLimits fails
    sptr<SessionStageMocker> sessionStageMock = sptr<SessionStageMocker>::MakeSptr();
    subSession->sessionStage_ = sessionStageMock;

    EXPECT_CALL(*sessionStageMock, RemoveAttachedWindowLimits(sourcePersistentId))
        .Times(1).WillOnce(testing::Return(WSError::WS_ERROR_IPC_FAILED));

    WSError ret = subSession->RequestRemoveAttachedWindowLimits(sourcePersistentId, INVALID_SESSION_ID);
    EXPECT_EQ(WSError::WS_ERROR_IPC_FAILED, ret);
}

/**
 * @tc.name: RequestRemoveAttachedWindowLimits03
 * @tc.desc: Test sub window RequestRemoveAttachedWindowLimits when sourcePersistentId == winId (detaching)
 * @tc.type: FUNC
 */
HWTEST_F(SubSessionLayoutTest, RequestRemoveAttachedWindowLimits03, TestSize.Level1)
{
    auto info = CreateSessionInfo("RequestRemoveAttachedWindowLimits03");
    sptr<SubSession> subSession = sptr<SubSession>::MakeSptr(info, nullptr);

    sptr<SessionStageMocker> sessionStageMock = sptr<SessionStageMocker>::MakeSptr();
    subSession->sessionStage_ = sessionStageMock;

    // Pre-populate attached limits lists to verify they get cleared
    auto property = subSession->GetSessionProperty();
    property->SetAttachedWindowLimits(100, { 2000, 1000, 200, 300, 0.0f, 0.0f, 0.0f, PixelUnit::PX });
    property->SetAttachedWindowLimits(200, { 1800, 900, 150, 250, 0.0f, 0.0f, 0.0f, PixelUnit::PX });
    AttachLimitOptions options = { true, true };
    property->SetAttachedLimitOptions(100, options);
    property->SetAttachedLimitOptions(200, options);

    EXPECT_FALSE(property->GetAttachedWindowLimitsList().empty());
    EXPECT_FALSE(property->GetAttachedLimitOptionsList().empty());

    // Use subSession's own persistentId as sourcePersistentId (detaching scenario)
    int32_t winId = subSession->GetPersistentId();

    EXPECT_CALL(*sessionStageMock, RemoveAttachedWindowLimits(winId))
        .Times(1).WillOnce(testing::Return(WSError::WS_OK));

    WSError ret = subSession->RequestRemoveAttachedWindowLimits(winId, INVALID_SESSION_ID);
    EXPECT_EQ(WSError::WS_OK, ret);

    // Verify that all attached limits lists were cleared
    EXPECT_TRUE(property->GetAttachedWindowLimitsList().empty());
    EXPECT_TRUE(property->GetAttachedLimitOptionsList().empty());
}
} // namespace
} // namespace Rosen
} // namespace OHOS