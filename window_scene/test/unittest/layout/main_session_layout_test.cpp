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
#include <pointer_event.h>
#include <ui/rs_surface_node.h>

#include "session/host/include/main_session.h"
#include "session/host/include/session.h"
#include "session/host/include/sub_session.h"
#include "session/screen/include/screen_session.h"
#include "window_helper.h"
#include "window_manager_hilog.h"
#include "test/mock/mock_session_stage.h"

using namespace testing;
using namespace testing::ext;

namespace OHOS {
namespace Rosen {
class MainSessionLayoutTest : public testing::Test {
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

    // Helper function to create attached WindowAnchorInfo
    WindowAnchorInfo CreateAttachInfo(bool heightLimit = true, bool widthLimit = true) const
    {
        WindowAnchorInfo anchorInfo;
        anchorInfo.isAnchoredByAttach_ = true;
        anchorInfo.isFromAttachOrDetach_ = true;
        anchorInfo.attachOptions.isIntersectedHeightLimit = heightLimit;
        anchorInfo.attachOptions.isIntersectedWidthLimit = widthLimit;
        return anchorInfo;
    }

    // Helper function to create sub session with attach info and mock stage
    struct SubSessionWithMock {
        sptr<SubSession> session;
        sptr<SessionStageMocker> mockStage;
    };
    SubSessionWithMock CreateSubSessionWithMock(const std::string& name)
    {
        SubSessionWithMock result;
        result.session = sptr<SubSession>::MakeSptr(CreateSessionInfo(name), nullptr);
        result.mockStage = sptr<SessionStageMocker>::MakeSptr();
        result.session->sessionStage_ = result.mockStage;
        result.session->SetWindowAnchorInfo(CreateAttachInfo());
        return result;
    }
};

void MainSessionLayoutTest::SetUpTestCase() {}

void MainSessionLayoutTest::TearDownTestCase() {}

void MainSessionLayoutTest::SetUp() {}

void MainSessionLayoutTest::TearDown() {}

RSSurfaceNode::SharedPtr MainSessionLayoutTest::CreateRSSurfaceNode()
{
    struct RSSurfaceNodeConfig rsSurfaceNodeConfig;
    rsSurfaceNodeConfig.SurfaceNodeName = "WindowSessionTestSurfaceNode";
    auto surfaceNode = RSSurfaceNode::Create(rsSurfaceNodeConfig);
    return surfaceNode;
}

namespace {
/**
 * @tc.name: SetSubWindowBoundsDuringCross
 * @tc.desc: Check reason when setSubWindowBoundsDuringCross
 * @tc.type: FUNC
 */
HWTEST_F(MainSessionLayoutTest, SetSubWindowBoundsDuringCross, TestSize.Level1)
{
    SessionInfo info;
    info.bundleName_ = "SetSubWindowBoundsDuringCross";
    info.moduleName_ = "SetSubWindowBoundsDuringCross";
    info.abilityName_ = "SetSubWindowBoundsDuringCross";
    sptr<MainSession> mainSession = sptr<MainSession>::MakeSptr(info, nullptr);
    sptr<SceneSession> subSession = sptr<SceneSession>::MakeSptr(info, nullptr);
    mainSession->subSession_.emplace_back(nullptr);
    mainSession->subSession_.emplace_back(subSession);
    mainSession->GetLayoutController()->SetSessionRect({ 50, 50, 500, 500 });
    subSession->GetLayoutController()->SetSessionRect({ 0, 0, 200, 200 });

    subSession->state_ = SessionState::STATE_FOREGROUND;
    subSession->windowAnchorInfo_.isAnchorEnabled_ = false;
    subSession->Session::UpdateSizeChangeReason(SizeChangeReason::RESIZE);
    mainSession->SetSubWindowBoundsDuringCross(mainSession->GetSessionRect(), true, true);
    EXPECT_EQ(subSession->GetSizeChangeReason(), SizeChangeReason::RESIZE);

    subSession->windowAnchorInfo_.isAnchorEnabled_ = true;
    subSession->Session::UpdateSizeChangeReason(SizeChangeReason::RESIZE);
    mainSession->SetSubWindowBoundsDuringCross(mainSession->GetSessionRect(), true, true);
    EXPECT_EQ(subSession->GetSizeChangeReason(), SizeChangeReason::UNDEFINED);

    subSession->state_ = SessionState::STATE_BACKGROUND;
    subSession->Session::UpdateSizeChangeReason(SizeChangeReason::RESIZE);
    mainSession->SetSubWindowBoundsDuringCross(mainSession->GetSessionRect(), true, true);
    EXPECT_EQ(subSession->GetSizeChangeReason(), SizeChangeReason::RESIZE);
}

/**
 * @tc.name: NotifySubSessionRectChangeByAnchor
 * @tc.desc: Check reason when NotifySubSessionRectChangeByAnchor
 * @tc.type: FUNC
 */
HWTEST_F(MainSessionLayoutTest, NotifySubSessionRectChangeByAnchor, TestSize.Level1)
{
    SessionInfo info;
    info.bundleName_ = "NotifySubSessionRectChangeByAnchor";
    info.moduleName_ = "NotifySubSessionRectChangeByAnchor";
    info.abilityName_ = "NotifySubSessionRectChangeByAnchor";
    sptr<MainSession> mainSession = sptr<MainSession>::MakeSptr(info, nullptr);
    sptr<SceneSession> subSession = sptr<SceneSession>::MakeSptr(info, nullptr);
    mainSession->subSession_.emplace_back(nullptr);
    mainSession->subSession_.emplace_back(subSession);
    mainSession->GetLayoutController()->SetSessionRect({ 50, 50, 500, 500 });
    subSession->GetLayoutController()->SetSessionRect({ 0, 0, 200, 200 });
    const auto& func = [subSession](const WSRect& rect,
                                    SizeChangeReason reason,
                                    DisplayId displayId) {
        subSession->GetLayoutController()->SetSessionRect(rect);
        subSession->Session::UpdateSizeChangeReason(reason);
    };
    subSession->SetSessionRectChangeCallback(func);

    subSession->windowAnchorInfo_.isAnchorEnabled_ = false;
    subSession->Session::UpdateSizeChangeReason(SizeChangeReason::UNDEFINED);
    mainSession->NotifySubSessionRectChangeByAnchor(mainSession->GetSessionRect(), SizeChangeReason::RESIZE);
    EXPECT_EQ(subSession->GetSizeChangeReason(), SizeChangeReason::UNDEFINED);

    subSession->windowAnchorInfo_.isAnchorEnabled_ = true;
    subSession->Session::UpdateSizeChangeReason(SizeChangeReason::UNDEFINED);
    mainSession->NotifySubSessionRectChangeByAnchor(mainSession->GetSessionRect(), SizeChangeReason::RESIZE);
    EXPECT_EQ(subSession->GetSizeChangeReason(), SizeChangeReason::RESIZE);
}

/**
 * @tc.name: HandleSubSessionSurfaceNodeByWindowAnchor
 * @tc.desc: Check reason when HandleSubSessionSurfaceNodeByWindowAnchor
 * @tc.type: FUNC
 */
HWTEST_F(MainSessionLayoutTest, HandleSubSessionSurfaceNodeByWindowAnchor, TestSize.Level1)
{
    SessionInfo info;
    info.bundleName_ = "HandleSubSessionSurfaceNodeByWindowAnchor";
    info.moduleName_ = "HandleSubSessionSurfaceNodeByWindowAnchor";
    info.abilityName_ = "HandleSubSessionSurfaceNodeByWindowAnchor";
    sptr<MainSession> mainSession = sptr<MainSession>::MakeSptr(info, nullptr);
    sptr<SubSession> subSession = sptr<SubSession>::MakeSptr(info, nullptr);
    mainSession->subSession_.emplace_back(nullptr);
    mainSession->subSession_.emplace_back(subSession);
    auto surfaceNode = CreateRSSurfaceNode();
    ASSERT_NE(nullptr, surfaceNode);
    subSession->SetSurfaceNode(surfaceNode);
    subSession->SetFindScenePanelRsNodeByZOrderFunc([this](uint64_t screenId, uint32_t targetZOrder) {
        return CreateRSSurfaceNode();
    });

    subSession->state_ = SessionState::STATE_FOREGROUND;
    subSession->windowAnchorInfo_.isAnchorEnabled_ = false;
    subSession->cloneNodeCountDuringCross_.store(0);
    mainSession->HandleSubSessionSurfaceNodeByWindowAnchor(SizeChangeReason::DRAG, 0);
    EXPECT_EQ(subSession->cloneNodeCountDuringCross_, 0);

    subSession->windowAnchorInfo_.isAnchorEnabled_ = true;
    mainSession->HandleSubSessionSurfaceNodeByWindowAnchor(SizeChangeReason::DRAG, 0);
    EXPECT_EQ(subSession->cloneNodeCountDuringCross_, 1);

    subSession->state_ = SessionState::STATE_BACKGROUND;
    mainSession->HandleSubSessionSurfaceNodeByWindowAnchor(SizeChangeReason::DRAG_END, 0);
    EXPECT_EQ(subSession->cloneNodeCountDuringCross_, 1);
}

/**
 * @tc.name: RequestUpdateAttachedWindowLimits01
 * @tc.desc: Test main window updates own limits and propagates to children
 * @tc.type: FUNC
 */
HWTEST_F(MainSessionLayoutTest, RequestUpdateAttachedWindowLimits01, TestSize.Level1)
{
    SessionInfo info;
    info.bundleName_ = "RequestUpdateAttachedWindowLimits01";
    info.moduleName_ = "RequestUpdateAttachedWindowLimits01";
    info.abilityName_ = "RequestUpdateAttachedWindowLimits01";
    sptr<MainSession> mainSession = sptr<MainSession>::MakeSptr(info, nullptr);

    int32_t sourcePersistentId = 1001;
    WindowLimits attachedLimits = { 2000, 1000, 200, 300, 0.0f, 0.0f, 0.0f, PixelUnit::PX };
    bool isIntersectedHeightLimit = true;
    bool isIntersectedWidthLimit = true;

    // Test with null sessionStage_
    mainSession->sessionStage_ = nullptr;
    WSError ret = mainSession->RequestUpdateAttachedWindowLimits(sourcePersistentId, attachedLimits,
        isIntersectedHeightLimit, isIntersectedWidthLimit);
    EXPECT_EQ(WSError::WS_ERROR_NULLPTR, ret);

    // Test with valid sessionStage_
    sptr<SessionStageMocker> mainSessionStage = sptr<SessionStageMocker>::MakeSptr();
    mainSession->sessionStage_ = mainSessionStage;

    EXPECT_CALL(*mainSessionStage, UpdateAttachedWindowLimits(
        sourcePersistentId, testing::_, isIntersectedHeightLimit, isIntersectedWidthLimit))
        .Times(1).WillOnce(testing::Return(WSError::WS_OK));

    ret = mainSession->RequestUpdateAttachedWindowLimits(sourcePersistentId, attachedLimits,
        isIntersectedHeightLimit, isIntersectedWidthLimit);
    EXPECT_EQ(WSError::WS_OK, ret);
}

/**
 * @tc.name: RequestUpdateAttachedWindowLimits02
 * @tc.desc: Test main window propagates to attached children only
 * @tc.type: FUNC
 */
HWTEST_F(MainSessionLayoutTest, RequestUpdateAttachedWindowLimits02, TestSize.Level1)
{
    SessionInfo info;
    info.bundleName_ = "RequestUpdateAttachedWindowLimits02";
    info.moduleName_ = "RequestUpdateAttachedWindowLimits02";
    info.abilityName_ = "RequestUpdateAttachedWindowLimits02";
    sptr<MainSession> mainSession = sptr<MainSession>::MakeSptr(info, nullptr);

    sptr<SessionStageMocker> mainSessionStage = sptr<SessionStageMocker>::MakeSptr();
    mainSession->sessionStage_ = mainSessionStage;

    sptr<SubSession> subSession1 = sptr<SubSession>::MakeSptr(info, nullptr);
    sptr<SessionStageMocker> subStage1 = sptr<SessionStageMocker>::MakeSptr();
    subSession1->sessionStage_ = subStage1;
    sptr<SubSession> subSession2 = sptr<SubSession>::MakeSptr(info, nullptr);
    sptr<SessionStageMocker> subStage2 = sptr<SessionStageMocker>::MakeSptr();
    subSession2->sessionStage_ = subStage2;

    // Setup: subSession1 is attached, subSession2 is not
    subSession1->windowAnchorInfo_.isAnchoredByAttach_ = true;
    subSession1->windowAnchorInfo_.attachOptions.isIntersectedHeightLimit = true;
    subSession2->windowAnchorInfo_.isAnchoredByAttach_ = false;

    mainSession->subSession_.emplace_back(subSession1);
    mainSession->subSession_.emplace_back(subSession2);

    WindowLimits attachedLimits = { 2000, 1000, 200, 300, 0.0f, 0.0f, 0.0f, PixelUnit::PX };
    int32_t sourcePersistentId = 1001;

    // Main session updates own limits
    EXPECT_CALL(*mainSessionStage, UpdateAttachedWindowLimits(
        sourcePersistentId, testing::_, true, true))
        .Times(1).WillOnce(testing::Return(WSError::WS_OK));

    // Only subSession1 should be notified (has attach enabled)
    EXPECT_CALL(*subStage1, UpdateAttachedWindowLimits(
        sourcePersistentId, testing::_, true, true))
        .Times(1).WillOnce(testing::Return(WSError::WS_OK));

    // subSession2 should NOT be notified (no attach relationship)
    EXPECT_CALL(*subStage2, UpdateAttachedWindowLimits(testing::_, testing::_, testing::_, testing::_))
        .Times(0);

    WSError ret = mainSession->RequestUpdateAttachedWindowLimits(sourcePersistentId, attachedLimits,
        true, true); // use default excludePersistentId=INVALID_SESSION_ID so main updates itself
    EXPECT_EQ(WSError::WS_OK, ret);
}

/**
 * @tc.name: RequestRemoveAttachedWindowLimits01
 * @tc.desc: Test main window removes own limits and propagates to children
 * @tc.type: FUNC
 */
HWTEST_F(MainSessionLayoutTest, RequestRemoveAttachedWindowLimits01, TestSize.Level1)
{
    SessionInfo info;
    info.bundleName_ = "RequestRemoveAttachedWindowLimits01";
    info.moduleName_ = "RequestRemoveAttachedWindowLimits01";
    info.abilityName_ = "RequestRemoveAttachedWindowLimits01";
    sptr<MainSession> mainSession = sptr<MainSession>::MakeSptr(info, nullptr);

    int32_t sourcePersistentId = 2001;

    // Test with null sessionStage_
    mainSession->sessionStage_ = nullptr;
    WSError ret = mainSession->RequestRemoveAttachedWindowLimits(sourcePersistentId);
    EXPECT_EQ(WSError::WS_ERROR_NULLPTR, ret);

    // Test with valid sessionStage_
    sptr<SessionStageMocker> mainSessionStage = sptr<SessionStageMocker>::MakeSptr();
    mainSession->sessionStage_ = mainSessionStage;

    EXPECT_CALL(*mainSessionStage, RemoveAttachedWindowLimits(sourcePersistentId))
        .Times(1).WillOnce(testing::Return(WSError::WS_OK));

    ret = mainSession->RequestRemoveAttachedWindowLimits(sourcePersistentId, INVALID_SESSION_ID);
    EXPECT_EQ(WSError::WS_OK, ret);
}

/**
 * @tc.name: RequestUpdateAttachedWindowLimitsPropagate01
 * @tc.desc: Test MainSession RequestUpdateAttachedWindowLimits propagation to multiple sub sessions
 * @tc.type: FUNC
 */
HWTEST_F(MainSessionLayoutTest, RequestUpdateAttachedWindowLimitsPropagate01, TestSize.Level1)
{
    auto info = CreateSessionInfo("RequestUpdateAttachedWindowLimitsPropagate01");
    sptr<MainSession> mainSession = sptr<MainSession>::MakeSptr(info, nullptr);

    // Create mock session stage for main session
    sptr<SessionStageMocker> mainSessionStage = sptr<SessionStageMocker>::MakeSptr();
    mainSession->sessionStage_ = mainSessionStage;

    // Create sub sessions with mock stages and attach info
    auto sub1 = CreateSubSessionWithMock("Sub1");
    auto sub2 = CreateSubSessionWithMock("Sub2");
    auto sub3 = CreateSubSessionWithMock("Sub3");

    // Add sub sessions to main session
    mainSession->subSession_.emplace_back(sub1.session);
    mainSession->subSession_.emplace_back(sub2.session);
    mainSession->subSession_.emplace_back(sub3.session);
    sub1.session->windowAnchorInfo_.isAnchoredByAttach_ = true;
    sub2.session->windowAnchorInfo_.isAnchoredByAttach_ = true;
    sub3.session->windowAnchorInfo_.isAnchoredByAttach_ = true;
    sub1.session->windowAnchorInfo_.attachOptions.isIntersectedWidthLimit = true;
    sub2.session->windowAnchorInfo_.attachOptions.isIntersectedWidthLimit = true;
    sub3.session->windowAnchorInfo_.attachOptions.isIntersectedWidthLimit = true;

    WindowLimits newLimits = { 2000, 1000, 200, 300, 0.0f, 0.0f, 0.0f, PixelUnit::PX };
    int32_t sourcePersistentId = 100;

    // All sub sessions should be notified via UpdateAttachedWindowLimits
    EXPECT_CALL(*sub1.mockStage, UpdateAttachedWindowLimits(
        sourcePersistentId, testing::_, true, true))
        .Times(1).WillOnce(testing::Return(WSError::WS_OK));
    EXPECT_CALL(*sub2.mockStage, UpdateAttachedWindowLimits(
        sourcePersistentId, testing::_, true, true))
        .Times(1).WillOnce(testing::Return(WSError::WS_OK));
    EXPECT_CALL(*sub3.mockStage, UpdateAttachedWindowLimits(
        sourcePersistentId, testing::_, true, true))
        .Times(1).WillOnce(testing::Return(WSError::WS_OK));

    // Propagate to all sub sessions (exclude self to skip redundant update)
    mainSession->RequestUpdateAttachedWindowLimits(sourcePersistentId, newLimits,
        true, true, mainSession->GetPersistentId());
}

/**
 * @tc.name: RequestUpdateAttachedWindowLimitsPropagate02
 * @tc.desc: Test MainSession RequestUpdateAttachedWindowLimits with exclude persistentId
 * @tc.type: FUNC
 */
HWTEST_F(MainSessionLayoutTest, RequestUpdateAttachedWindowLimitsPropagate02, TestSize.Level1)
{
    auto info = CreateSessionInfo("RequestUpdateAttachedWindowLimitsPropagate02");
    sptr<MainSession> mainSession = sptr<MainSession>::MakeSptr(info, nullptr);

    // Create mock session stage for main session
    sptr<SessionStageMocker> mainSessionStage = sptr<SessionStageMocker>::MakeSptr();
    mainSession->sessionStage_ = mainSessionStage;

    // Create sub sessions with mock stages and attach info
    auto sub1 = CreateSubSessionWithMock("Sub1");
    auto sub2 = CreateSubSessionWithMock("Sub2");
    auto sub3 = CreateSubSessionWithMock("Sub3");

    // Add sub sessions to main session
    mainSession->subSession_.emplace_back(sub1.session);
    mainSession->subSession_.emplace_back(sub2.session);
    mainSession->subSession_.emplace_back(sub3.session);
    sub1.session->windowAnchorInfo_.isAnchoredByAttach_ = true;
    sub2.session->windowAnchorInfo_.isAnchoredByAttach_ = true;
    sub3.session->windowAnchorInfo_.isAnchoredByAttach_ = true;
    sub1.session->windowAnchorInfo_.attachOptions.isIntersectedWidthLimit = true;
    sub2.session->windowAnchorInfo_.attachOptions.isIntersectedWidthLimit = true;
    sub3.session->windowAnchorInfo_.attachOptions.isIntersectedWidthLimit = true;

    WindowLimits newLimits = { 2000, 1000, 200, 300, 0.0f, 0.0f, 0.0f, PixelUnit::PX };
    int32_t sourcePersistentId = 100;

    // Main session should update its own limits
    EXPECT_CALL(*mainSessionStage, UpdateAttachedWindowLimits(
        sourcePersistentId, testing::_, true, true))
        .Times(1).WillOnce(testing::Return(WSError::WS_OK));

    // subSession1 and subSession3 should be notified via UpdateAttachedWindowLimits, but NOT subSession2 (excluded)
    EXPECT_CALL(*sub1.mockStage, UpdateAttachedWindowLimits(
        sourcePersistentId, testing::_, true, true))
        .Times(1).WillOnce(testing::Return(WSError::WS_OK));
    EXPECT_CALL(*sub2.mockStage, UpdateAttachedWindowLimits(
        testing::_, testing::_, testing::_, testing::_))
        .Times(0);
    EXPECT_CALL(*sub3.mockStage, UpdateAttachedWindowLimits(
        sourcePersistentId, testing::_, true, true))
        .Times(1).WillOnce(testing::Return(WSError::WS_OK));

    // Propagate to sub sessions, excluding subSession2
    mainSession->RequestUpdateAttachedWindowLimits(sourcePersistentId, newLimits,
        true, true, sub2.session->GetPersistentId());
}

/**
 * @tc.name: RequestUpdateAttachedWindowLimitsPropagate03
 * @tc.desc: Test MainSession RequestUpdateAttachedWindowLimits with empty sub sessions
 * @tc.type: FUNC
 */
HWTEST_F(MainSessionLayoutTest, RequestUpdateAttachedWindowLimitsPropagate03, TestSize.Level1)
{
    SessionInfo info;
    info.bundleName_ = "RequestUpdateAttachedWindowLimitsPropagate03";
    info.moduleName_ = "RequestUpdateAttachedWindowLimitsPropagate03";
    info.abilityName_ = "RequestUpdateAttachedWindowLimitsPropagate03";
    sptr<MainSession> mainSession = sptr<MainSession>::MakeSptr(info, nullptr);

    // Create mock session stage for main session
    sptr<SessionStageMocker> mainSessionStage = sptr<SessionStageMocker>::MakeSptr();
    mainSession->sessionStage_ = mainSessionStage;

    WindowLimits newLimits = { 2000, 1000, 200, 300, 0.0f, 0.0f, 0.0f, PixelUnit::PX };
    int32_t sourcePersistentId = 100;

    // No sub sessions, and excludePersistentId == GetPersistentId(), so no update to own limits
    // Verify that UpdateAttachedWindowLimits is NOT called on mainSessionStage
    EXPECT_CALL(*mainSessionStage, UpdateAttachedWindowLimits(
        testing::_, testing::_, testing::_, testing::_))
        .Times(0);
    mainSession->RequestUpdateAttachedWindowLimits(sourcePersistentId, newLimits,
        true, true, mainSession->GetPersistentId());
}

/**
 * @tc.name: RequestUpdateAttachedWindowLimitsPropagate04
 * @tc.desc: Test MainSession RequestUpdateAttachedWindowLimits with sub sessions without attach relationship
 * @tc.type: FUNC
 */
HWTEST_F(MainSessionLayoutTest, RequestUpdateAttachedWindowLimitsPropagate04, TestSize.Level1)
{
    SessionInfo info;
    info.bundleName_ = "RequestUpdateAttachedWindowLimitsPropagate04";
    info.moduleName_ = "RequestUpdateAttachedWindowLimitsPropagate04";
    info.abilityName_ = "RequestUpdateAttachedWindowLimitsPropagate04";
    sptr<MainSession> mainSession = sptr<MainSession>::MakeSptr(info, nullptr);

    // Create mock session stage for main session
    sptr<SessionStageMocker> mainSessionStage = sptr<SessionStageMocker>::MakeSptr();
    mainSession->sessionStage_ = mainSessionStage;

    sptr<SubSession> subSession1 = sptr<SubSession>::MakeSptr(info, nullptr);
    sptr<SubSession> subSession2 = sptr<SubSession>::MakeSptr(info, nullptr);

    // Create mock session stages for sub sessions
    sptr<SessionStageMocker> subSessionStage1 = sptr<SessionStageMocker>::MakeSptr();
    sptr<SessionStageMocker> subSessionStage2 = sptr<SessionStageMocker>::MakeSptr();
    subSession1->sessionStage_ = subSessionStage1;
    subSession2->sessionStage_ = subSessionStage2;

    // Set up attach info WITHOUT intersected limits
    WindowAnchorInfo anchorInfo;
    anchorInfo.isAnchoredByAttach_ = true;
    anchorInfo.isFromAttachOrDetach_ = true;
    anchorInfo.attachOptions.isIntersectedHeightLimit = false;
    anchorInfo.attachOptions.isIntersectedWidthLimit = false;
    subSession1->SetWindowAnchorInfo(anchorInfo);
    subSession2->SetWindowAnchorInfo(anchorInfo);

    // Add sub sessions to main session
    mainSession->subSession_.emplace_back(subSession1);
    mainSession->subSession_.emplace_back(subSession2);

    WindowLimits newLimits = { 2000, 1000, 200, 300, 0.0f, 0.0f, 0.0f, PixelUnit::PX };
    int32_t sourcePersistentId = 100;

    // Should not notify sub sessions without intersected limits
    EXPECT_CALL(*subSessionStage1, UpdateAttachedWindowLimits(
        testing::_, testing::_, testing::_, testing::_))
        .Times(0);
    EXPECT_CALL(*subSessionStage2, UpdateAttachedWindowLimits(
        testing::_, testing::_, testing::_, testing::_))
        .Times(0);

    // Main session should NOT update its own limits since excludePersistentId == GetPersistentId()
    mainSession->RequestUpdateAttachedWindowLimits(sourcePersistentId, newLimits,
        true, true, mainSession->GetPersistentId());
}

/**
 * @tc.name: RequestRemoveAttachedWindowLimitsPropagate01
 * @tc.desc: Test MainSession RequestRemoveAttachedWindowLimits with multiple sub sessions
 * @tc.type: FUNC
 */
HWTEST_F(MainSessionLayoutTest, RequestRemoveAttachedWindowLimitsPropagate01, TestSize.Level1)
{
    auto info = CreateSessionInfo("RequestRemoveAttachedWindowLimitsPropagate01");
    sptr<MainSession> mainSession = sptr<MainSession>::MakeSptr(info, nullptr);

    // Create mock session stage for main session
    sptr<SessionStageMocker> mainSessionStage = sptr<SessionStageMocker>::MakeSptr();
    mainSession->sessionStage_ = mainSessionStage;

    // Create sub sessions with mock stages and attach info
    auto sub1 = CreateSubSessionWithMock("Sub1");
    auto sub2 = CreateSubSessionWithMock("Sub2");
    auto sub3 = CreateSubSessionWithMock("Sub3");

    // Add sub sessions to main session
    mainSession->subSession_.emplace_back(sub1.session);
    mainSession->subSession_.emplace_back(sub2.session);
    mainSession->subSession_.emplace_back(sub3.session);
    sub1.session->windowAnchorInfo_.isAnchoredByAttach_ = true;
    sub2.session->windowAnchorInfo_.isAnchoredByAttach_ = true;
    sub3.session->windowAnchorInfo_.isAnchoredByAttach_ = true;
    sub1.session->windowAnchorInfo_.attachOptions.isIntersectedWidthLimit = true;
    sub2.session->windowAnchorInfo_.attachOptions.isIntersectedWidthLimit = true;
    sub3.session->windowAnchorInfo_.attachOptions.isIntersectedWidthLimit = true;

    int32_t sourcePersistentId = 100;

    // All sub sessions should have their limits removed
    EXPECT_CALL(*sub1.mockStage, RemoveAttachedWindowLimits(sourcePersistentId))
        .Times(1).WillOnce(testing::Return(WSError::WS_OK));
    EXPECT_CALL(*sub2.mockStage, RemoveAttachedWindowLimits(sourcePersistentId))
        .Times(1).WillOnce(testing::Return(WSError::WS_OK));
    EXPECT_CALL(*sub3.mockStage, RemoveAttachedWindowLimits(sourcePersistentId))
        .Times(1).WillOnce(testing::Return(WSError::WS_OK));

    // Remove limits from all sub sessions (exclude self)
    mainSession->RequestRemoveAttachedWindowLimits(sourcePersistentId, mainSession->GetPersistentId());
}

/**
 * @tc.name: RequestRemoveAttachedWindowLimitsPropagate02
 * @tc.desc: Test MainSession RequestRemoveAttachedWindowLimits with exclude persistentId
 * @tc.type: FUNC
 */
HWTEST_F(MainSessionLayoutTest, RequestRemoveAttachedWindowLimitsPropagate02, TestSize.Level1)
{
    auto info = CreateSessionInfo("RequestRemoveAttachedWindowLimitsPropagate02");
    sptr<MainSession> mainSession = sptr<MainSession>::MakeSptr(info, nullptr);

    // Create mock session stage for main session
    sptr<SessionStageMocker> mainSessionStage = sptr<SessionStageMocker>::MakeSptr();
    mainSession->sessionStage_ = mainSessionStage;

    // Create sub sessions with mock stages and attach info
    auto sub1 = CreateSubSessionWithMock("Sub1");
    auto sub2 = CreateSubSessionWithMock("Sub2");
    auto sub3 = CreateSubSessionWithMock("Sub3");

    // Add sub sessions to main session
    mainSession->subSession_.emplace_back(sub1.session);
    mainSession->subSession_.emplace_back(sub2.session);
    mainSession->subSession_.emplace_back(sub3.session);
    sub1.session->windowAnchorInfo_.isAnchoredByAttach_ = true;
    sub2.session->windowAnchorInfo_.isAnchoredByAttach_ = true;
    sub3.session->windowAnchorInfo_.isAnchoredByAttach_ = true;
    sub1.session->windowAnchorInfo_.attachOptions.isIntersectedWidthLimit = true;
    sub2.session->windowAnchorInfo_.attachOptions.isIntersectedWidthLimit = true;
    sub3.session->windowAnchorInfo_.attachOptions.isIntersectedWidthLimit = true;

    int32_t sourcePersistentId = 100;

    // Main session should remove its own limits
    EXPECT_CALL(*mainSessionStage, RemoveAttachedWindowLimits(sourcePersistentId))
        .Times(1).WillOnce(testing::Return(WSError::WS_OK));

    // subSession1 and subSession3 should have their limits removed, but NOT subSession2 (excluded)
    EXPECT_CALL(*sub1.mockStage, RemoveAttachedWindowLimits(sourcePersistentId))
        .Times(1).WillOnce(testing::Return(WSError::WS_OK));
    EXPECT_CALL(*sub2.mockStage, RemoveAttachedWindowLimits(testing::_))
        .Times(0);
    EXPECT_CALL(*sub3.mockStage, RemoveAttachedWindowLimits(sourcePersistentId))
        .Times(1).WillOnce(testing::Return(WSError::WS_OK));

    // Remove limits from sub sessions, excluding subSession2
    mainSession->RequestRemoveAttachedWindowLimits(sourcePersistentId, sub2.session->GetPersistentId());
}

/**
 * @tc.name: RequestRemoveAttachedWindowLimitsPropagate03
 * @tc.desc: Test MainSession RequestRemoveAttachedWindowLimits with empty sub sessions
 * @tc.type: FUNC
 */
HWTEST_F(MainSessionLayoutTest, RequestRemoveAttachedWindowLimitsPropagate03, TestSize.Level1)
{
    SessionInfo info;
    info.bundleName_ = "RequestRemoveAttachedWindowLimitsPropagate03";
    info.moduleName_ = "RequestRemoveAttachedWindowLimitsPropagate03";
    info.abilityName_ = "RequestRemoveAttachedWindowLimitsPropagate03";
    sptr<MainSession> mainSession = sptr<MainSession>::MakeSptr(info, nullptr);

    // Create mock session stage for main session
    sptr<SessionStageMocker> mainSessionStage = sptr<SessionStageMocker>::MakeSptr();
    mainSession->sessionStage_ = mainSessionStage;

    int32_t sourcePersistentId = 100;

    // No sub sessions, and excludePersistentId == GetPersistentId(), so no remove from own limits
    // Verify that RemoveAttachedWindowLimits is NOT called on mainSessionStage
    EXPECT_CALL(*mainSessionStage, RemoveAttachedWindowLimits(testing::_))
        .Times(0);
    mainSession->RequestRemoveAttachedWindowLimits(sourcePersistentId, mainSession->GetPersistentId());
}

/**
 * @tc.name: RequestRemoveAttachedWindowLimitsPropagate04
 * @tc.desc: Test MainSession RequestRemoveAttachedWindowLimits with sub sessions without attach relationship
 * @tc.type: FUNC
 */
HWTEST_F(MainSessionLayoutTest, RequestRemoveAttachedWindowLimitsPropagate04, TestSize.Level1)
{
    SessionInfo info;
    info.bundleName_ = "RequestRemoveAttachedWindowLimitsPropagate04";
    info.moduleName_ = "RequestRemoveAttachedWindowLimitsPropagate04";
    info.abilityName_ = "RequestRemoveAttachedWindowLimitsPropagate04";
    sptr<MainSession> mainSession = sptr<MainSession>::MakeSptr(info, nullptr);

    // Create mock session stage for main session
    sptr<SessionStageMocker> mainSessionStage = sptr<SessionStageMocker>::MakeSptr();
    mainSession->sessionStage_ = mainSessionStage;

    sptr<SubSession> subSession1 = sptr<SubSession>::MakeSptr(info, nullptr);
    sptr<SubSession> subSession2 = sptr<SubSession>::MakeSptr(info, nullptr);

    // Create mock session stages for sub sessions
    sptr<SessionStageMocker> subSessionStage1 = sptr<SessionStageMocker>::MakeSptr();
    sptr<SessionStageMocker> subSessionStage2 = sptr<SessionStageMocker>::MakeSptr();
    subSession1->sessionStage_ = subSessionStage1;
    subSession2->sessionStage_ = subSessionStage2;

    // Set up attach info WITHOUT intersected limits (should not be notified)
    WindowAnchorInfo anchorInfo;
    anchorInfo.isAnchoredByAttach_ = true;
    anchorInfo.isFromAttachOrDetach_ = true;
    anchorInfo.attachOptions.isIntersectedHeightLimit = false;
    anchorInfo.attachOptions.isIntersectedWidthLimit = false;
    subSession1->SetWindowAnchorInfo(anchorInfo);
    subSession2->SetWindowAnchorInfo(anchorInfo);

    // Add sub sessions to main session
    mainSession->subSession_.emplace_back(subSession1);
    mainSession->subSession_.emplace_back(subSession2);

    int32_t sourcePersistentId = 100;

    // Should not notify sub sessions without intersected limits
    EXPECT_CALL(*subSessionStage1, RemoveAttachedWindowLimits(testing::_))
        .Times(0);
    EXPECT_CALL(*subSessionStage2, RemoveAttachedWindowLimits(testing::_))
        .Times(0);

    // Main session should NOT remove its own limits since excludePersistentId == GetPersistentId()
    mainSession->RequestRemoveAttachedWindowLimits(sourcePersistentId, mainSession->GetPersistentId());
}

/**
 * @tc.name: RequestUpdateAttachedWindowLimits05
 * @tc.desc: Test MainSession RequestUpdateAttachedWindowLimits with null sessionStage
 * @tc.type: FUNC
 */
HWTEST_F(MainSessionLayoutTest, RequestUpdateAttachedWindowLimits05, TestSize.Level1)
{
    SessionInfo info;
    info.bundleName_ = "RequestUpdateAttachedWindowLimits05";
    info.moduleName_ = "RequestUpdateAttachedWindowLimits05";
    info.abilityName_ = "RequestUpdateAttachedWindowLimits05";
    sptr<MainSession> mainSession = sptr<MainSession>::MakeSptr(info, nullptr);

    // Do NOT set sessionStage_ - leave it as null

    WindowLimits newLimits = { 2000, 1000, 200, 300, 0.0f, 0.0f, 0.0f, PixelUnit::PX };
    int32_t sourcePersistentId = 100;

    // Should return WS_ERROR_NULLPTR since sessionStage_ is null
    WSError res = mainSession->RequestUpdateAttachedWindowLimits(sourcePersistentId, newLimits,
        true, true, mainSession->GetPersistentId());
    EXPECT_EQ(WSError::WS_ERROR_NULLPTR, res);
}

/**
 * @tc.name: RequestUpdateAttachedWindowLimits06
 * @tc.desc: Test MainSession RequestUpdateAttachedWindowLimits with UpdateAttachedWindowLimits failure
 * @tc.type: FUNC
 */
HWTEST_F(MainSessionLayoutTest, RequestUpdateAttachedWindowLimits06, TestSize.Level1)
{
    SessionInfo info;
    info.bundleName_ = "RequestUpdateAttachedWindowLimits06";
    info.moduleName_ = "RequestUpdateAttachedWindowLimits06";
    info.abilityName_ = "RequestUpdateAttachedWindowLimits06";
    sptr<MainSession> mainSession = sptr<MainSession>::MakeSptr(info, nullptr);

    // Create mock session stage for main session
    sptr<SessionStageMocker> mainSessionStage = sptr<SessionStageMocker>::MakeSptr();
    mainSession->sessionStage_ = mainSessionStage;

    WindowLimits newLimits = { 2000, 1000, 200, 300, 0.0f, 0.0f, 0.0f, PixelUnit::PX };
    int32_t sourcePersistentId = 100;

    // Set excludePersistentId to a value different from GetPersistentId() so mainSession updates its own limits
    int32_t excludePersistentId = INVALID_SESSION_ID;

    // Main session's UpdateAttachedWindowLimits should fail
    EXPECT_CALL(*mainSessionStage, UpdateAttachedWindowLimits(
        sourcePersistentId, testing::_, true, true))
        .Times(1).WillOnce(testing::Return(WSError::WS_ERROR_IPC_FAILED));

    // Should return WS_ERROR_IPC_FAILED since UpdateAttachedWindowLimits failed
    WSError res = mainSession->RequestUpdateAttachedWindowLimits(sourcePersistentId, newLimits,
        true, true, excludePersistentId);
    EXPECT_EQ(WSError::WS_ERROR_IPC_FAILED, res);
}

/**
 * @tc.name: RequestRemoveAttachedWindowLimits05
 * @tc.desc: Test MainSession RequestRemoveAttachedWindowLimits with null sessionStage
 * @tc.type: FUNC
 */
HWTEST_F(MainSessionLayoutTest, RequestRemoveAttachedWindowLimits05, TestSize.Level1)
{
    SessionInfo info;
    info.bundleName_ = "RequestRemoveAttachedWindowLimits05";
    info.moduleName_ = "RequestRemoveAttachedWindowLimits05";
    info.abilityName_ = "RequestRemoveAttachedWindowLimits05";
    sptr<MainSession> mainSession = sptr<MainSession>::MakeSptr(info, nullptr);

    // Do NOT set sessionStage_ - leave it as null

    int32_t sourcePersistentId = 100;

    // Should return WS_ERROR_NULLPTR since sessionStage_ is null
    WSError res = mainSession->RequestRemoveAttachedWindowLimits(sourcePersistentId, mainSession->GetPersistentId());
    EXPECT_EQ(WSError::WS_ERROR_NULLPTR, res);
}

/**
 * @tc.name: RequestRemoveAttachedWindowLimits06
 * @tc.desc: Test MainSession RequestRemoveAttachedWindowLimits with RemoveAttachedWindowLimits failure
 * @tc.type: FUNC
 */
HWTEST_F(MainSessionLayoutTest, RequestRemoveAttachedWindowLimits06, TestSize.Level1)
{
    SessionInfo info;
    info.bundleName_ = "RequestRemoveAttachedWindowLimits06";
    info.moduleName_ = "RequestRemoveAttachedWindowLimits06";
    info.abilityName_ = "RequestRemoveAttachedWindowLimits06";
    sptr<MainSession> mainSession = sptr<MainSession>::MakeSptr(info, nullptr);

    // Create mock session stage for main session
    sptr<SessionStageMocker> mainSessionStage = sptr<SessionStageMocker>::MakeSptr();
    mainSession->sessionStage_ = mainSessionStage;

    int32_t sourcePersistentId = 100;

    // Set excludePersistentId to a value different from GetPersistentId() so mainSession removes its own limits
    int32_t excludePersistentId = INVALID_SESSION_ID;

    // Main session's RemoveAttachedWindowLimits should fail
    EXPECT_CALL(*mainSessionStage, RemoveAttachedWindowLimits(sourcePersistentId))
        .Times(1).WillOnce(testing::Return(WSError::WS_ERROR_IPC_FAILED));

    // Should return WS_ERROR_IPC_FAILED since RemoveAttachedWindowLimits failed
    WSError res = mainSession->RequestRemoveAttachedWindowLimits(sourcePersistentId, excludePersistentId);
    EXPECT_EQ(WSError::WS_ERROR_IPC_FAILED, res);
}
} // namespace
} // namespace Rosen
} // namespace OHOS