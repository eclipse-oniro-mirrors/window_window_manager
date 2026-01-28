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

#include <gtest/gtest.h>
#include <regex>
#include <pointer_event.h>
#include <ui/rs_surface_node.h>

#include "mock/mock_session_stage.h"
#include "mock/mock_window_event_channel.h"
#include "mock/mock_pattern_detach_callback.h"
#include "session/host/include/extension_session.h"
#include "session/host/include/move_drag_controller.h"
#include "session/host/include/scene_session.h"
#include "session_manager/include/scene_session_manager.h"
#include "session/host/include/session.h"
#include "session_info.h"
#include "key_event.h"
#include "wm_common.h"
#include "window_manager_hilog.h"

using namespace testing;
using namespace testing::ext;

namespace OHOS {
namespace Rosen {
namespace {
const std::string UNDEFINED = "undefined";
}

class SessionLayoutTest : public testing::Test {
public:
    static void SetUpTestCase();
    static void TearDownTestCase();
    void SetUp() override;
    void TearDown() override;
    int32_t GetTaskCount();
    sptr<SceneSessionManager> ssm_;

private:
    RSSurfaceNode::SharedPtr CreateRSSurfaceNode();
    sptr<Session> session_ = nullptr;
    static constexpr uint32_t WAIT_SYNC_IN_NS = 500000;

    class TLifecycleListener : public ILifecycleListener {
    public:
        virtual ~TLifecycleListener() {}
        void OnActivation() override {}
        void OnConnect() override {}
        void OnForeground() override {}
        void OnBackground() override {}
        void OnDisconnect() override {}
        void OnExtensionDied() override {}
        void OnExtensionTimeout(int32_t errorCode) override {}
        void OnAccessibilityEvent(const Accessibility::AccessibilityEventInfo& info,
                                  int64_t uiExtensionIdLevel) override
        {
        }
        void OnDrawingCompleted() override {}
        void OnAppRemoveStartingWindow() override {}
    };
    std::shared_ptr<TLifecycleListener> lifecycleListener_ = std::make_shared<TLifecycleListener>();

    sptr<SessionStageMocker> mockSessionStage_ = nullptr;
    sptr<WindowEventChannelMocker> mockEventChannel_ = nullptr;
};

void SessionLayoutTest::SetUpTestCase() {}

void SessionLayoutTest::TearDownTestCase() {}

void SessionLayoutTest::SetUp()
{
    SessionInfo info;
    info.abilityName_ = "testSession1";
    info.moduleName_ = "testSession2";
    info.bundleName_ = "testSession3";
    session_ = sptr<Session>::MakeSptr(info);
    session_->surfaceNode_ = CreateRSSurfaceNode();
    ssm_ = sptr<SceneSessionManager>::MakeSptr();
    session_->SetEventHandler(ssm_->taskScheduler_->GetEventHandler(), ssm_->eventHandler_);
    auto isScreenLockedCallback = [this]() { return ssm_->IsScreenLocked(); };
    session_->RegisterIsScreenLockedCallback(isScreenLockedCallback);
    mockSessionStage_ = sptr<SessionStageMocker>::MakeSptr();
    mockEventChannel_ = sptr<WindowEventChannelMocker>::MakeSptr(mockSessionStage_);
}

void SessionLayoutTest::TearDown()
{
    session_ = nullptr;
    usleep(WAIT_SYNC_IN_NS);
}

RSSurfaceNode::SharedPtr SessionLayoutTest::CreateRSSurfaceNode()
{
    struct RSSurfaceNodeConfig rsSurfaceNodeConfig;
    rsSurfaceNodeConfig.SurfaceNodeName = "WindowSessionTestSurfaceNode";
    auto surfaceNode = RSSurfaceNode::Create(rsSurfaceNodeConfig);
    if (surfaceNode == nullptr) {
        GTEST_LOG_(INFO) << "SessionLayoutTest::CreateRSSurfaceNode surfaceNode is nullptr";
    }
    return surfaceNode;
}

int32_t SessionLayoutTest::GetTaskCount()
{
    std::string dumpInfo = session_->handler_->GetEventRunner()->GetEventQueue()->DumpCurrentQueueSize();
    std::regex pattern("\\d+");
    std::smatch matches;
    int32_t taskNum = 0;
    while (std::regex_search(dumpInfo, matches, pattern)) {
        taskNum += std::stoi(matches.str());
        dumpInfo = matches.suffix();
    }
    return taskNum;
}

namespace {
/**
 * @tc.name: UpdateRect01
 * @tc.desc: update rect
 * @tc.type: FUNC
 * @tc.require: #I6JLSI
 */
HWTEST_F(SessionLayoutTest, UpdateRect01, TestSize.Level1)
{
    bool preBackgroundUpdateRectNotifyEnabled = Session::IsBackgroundUpdateRectNotifyEnabled();
    Session::SetBackgroundUpdateRectNotifyEnabled(true);
    sptr<SessionStageMocker> mockSessionStage = sptr<SessionStageMocker>::MakeSptr();
    session_->sessionStage_ = mockSessionStage;
    EXPECT_CALL(*(mockSessionStage), UpdateRect(_, _, _, _)).Times(AtLeast(1)).WillOnce(Return(WSError::WS_OK));

    WSRect rect = { 0, 0, 0, 0 };
    ASSERT_EQ(WSError::WS_ERROR_INVALID_SESSION,
              session_->UpdateRect(rect, SizeChangeReason::UNDEFINED, "SessionLayoutTest"));
    sptr<WindowEventChannelMocker> mockEventChannel = sptr<WindowEventChannelMocker>::MakeSptr(mockSessionStage);
    SystemSessionConfig sessionConfig;
    sptr<WindowSessionProperty> property = sptr<WindowSessionProperty>::MakeSptr();
    ASSERT_EQ(WSError::WS_OK, session_->Connect(mockSessionStage, mockEventChannel, nullptr, sessionConfig, property));

    rect = { 0, 0, 100, 100 };
    ASSERT_EQ(WSError::WS_ERROR_INVALID_SESSION,
              session_->UpdateRect(rect, SizeChangeReason::UNDEFINED, "SessionLayoutTest"));
    ASSERT_EQ(rect, session_->GetSessionRect());

    rect = { 0, 0, 200, 200 };
    session_->UpdateSessionState(SessionState::STATE_ACTIVE);
    ASSERT_EQ(WSError::WS_OK, session_->UpdateRect(rect, SizeChangeReason::UNDEFINED, "SessionLayoutTest"));
    ASSERT_EQ(rect, session_->GetSessionRect());

    rect = { 0, 0, 300, 300 };
    session_->sessionStage_ = nullptr;
    ASSERT_EQ(WSError::WS_OK, session_->UpdateRect(rect, SizeChangeReason::UNDEFINED, "SessionLayoutTest"));
    ASSERT_EQ(rect, session_->GetSessionRect());
    Session::SetBackgroundUpdateRectNotifyEnabled(preBackgroundUpdateRectNotifyEnabled);
}

/**
 * @tc.name: UpdateRect_TestForeground
 * @tc.desc: update rect
 * @tc.type: FUNC
 * @tc.require: #I6JLSI
 */
HWTEST_F(SessionLayoutTest, UpdateRect_TestForeground, TestSize.Level1)
{
    bool preBackgroundUpdateRectNotifyEnabled = Session::IsBackgroundUpdateRectNotifyEnabled();
    Session::SetBackgroundUpdateRectNotifyEnabled(false);
    sptr<SessionStageMocker> mockSessionStage = sptr<SessionStageMocker>::MakeSptr();
    session_->sessionStage_ = mockSessionStage;

    WSRect rect = { 0, 0, 100, 100 };
    session_->UpdateSessionState(SessionState::STATE_ACTIVE);
    ASSERT_EQ(WSError::WS_OK, session_->UpdateRect(rect, SizeChangeReason::UNDEFINED, "SessionLayoutTest"));
    session_->UpdateSessionState(SessionState::STATE_BACKGROUND);
    ASSERT_EQ(WSError::WS_DO_NOTHING, session_->UpdateRect(rect, SizeChangeReason::UNDEFINED, "SessionLayoutTest"));
    Session::SetBackgroundUpdateRectNotifyEnabled(preBackgroundUpdateRectNotifyEnabled);
}

/**
 * @tc.name: UpdateSessionRect01
 * @tc.desc: UpdateSessionRect
 * @tc.type: FUNC
 */
HWTEST_F(SessionLayoutTest, UpdateSessionRect01, TestSize.Level1)
{
    SessionInfo info;
    info.abilityName_ = "testSession1";
    info.bundleName_ = "testSession3";
    sptr<SceneSession> sceneSession = sptr<SceneSession>::MakeSptr(info, nullptr);
    WSRect rect = { 0, 0, 320, 240 }; // width: 320, height: 240
    auto result = sceneSession->UpdateSessionRect(rect, SizeChangeReason::RESIZE);
    ASSERT_EQ(result, WSError::WS_OK);

    result = sceneSession->UpdateSessionRect(rect, SizeChangeReason::RESIZE);
    ASSERT_EQ(result, WSError::WS_OK);
}

/**
 * @tc.name: SetSingleHandTransform
 * @tc.desc: SetSingleHandTransform
 * @tc.type: FUNC
 */
HWTEST_F(SessionLayoutTest, SetSingleHandTransform, TestSize.Level1)
{
    SessionInfo info;
    info.abilityName_ = "SetSingleHandTransform";
    info.bundleName_ = "SetSingleHandTransform";
    sptr<Session> session = sptr<Session>::MakeSptr(info);
    SingleHandTransform transform;
    session->SetSingleHandTransform(transform);
    ASSERT_EQ(transform, session->GetSingleHandTransform());
}

/**
 * @tc.name: IsDraggingReason
 * @tc.desc: IsDraggingReason
 * @tc.type: FUNC
 */
HWTEST_F(SessionLayoutTest, IsDraggingReason, TestSize.Level1)
{
    SessionInfo info;
    info.abilityName_ = "IsDraggingReason";
    info.bundleName_ = "IsDraggingReason";
    sptr<Session> session = sptr<Session>::MakeSptr(info);
    ASSERT_EQ(false, session->IsDraggingReason(SizeChangeReason::UNDEFINED));
    ASSERT_EQ(true, session->IsDraggingReason(SizeChangeReason::DRAG));
    ASSERT_EQ(true, session->IsDraggingReason(SizeChangeReason::DRAG_START));
    ASSERT_EQ(true, session->IsDraggingReason(SizeChangeReason::DRAG_MOVE));
}

/**
 * @tc.name: SetDragStart
 * @tc.desc: SetDragStart
 * @tc.type: FUNC
 */
HWTEST_F(SessionLayoutTest, SetDragStart, TestSize.Level1)
{
    SessionInfo info;
    info.abilityName_ = "SetDragStart";
    info.bundleName_ = "SetDragStart";
    sptr<Session> session = sptr<Session>::MakeSptr(info);
    session->SetDragStart(true);
    ASSERT_EQ(true, session->IsDragStart());
    session->SetDragStart(false);
    ASSERT_EQ(false, session->IsDragStart());
}

/**
 * @tc.name: UpdateWindowModeSupportType01
 * @tc.desc: UpdateWindowModeSupportType
 * @tc.type: FUNC
 */
HWTEST_F(SessionLayoutTest, UpdateWindowModeSupportType01, TestSize.Level1)
{
    SessionInfo info;
    info.abilityName_ = "UpdateWindowModeSupportType01";
    info.bundleName_ = "UpdateWindowModeSupportType01";
    sptr<Session> session = sptr<Session>::MakeSptr(info);

    EXPECT_EQ(session->UpdateWindowModeSupportType(nullptr), false);

    std::shared_ptr<AppExecFwk::AbilityInfo> abilityInfo = std::make_shared<AppExecFwk::AbilityInfo>();
    EXPECT_EQ(session->UpdateWindowModeSupportType(abilityInfo), false);
}

/**
 * @tc.name: NotifyWindowStatusDidChangeIfNeedWhenUpdateRect
 * @tc.desc: NotifyWindowStatusDidChangeIfNeedWhenUpdateRect
 * @tc.type: FUNC
 */
HWTEST_F(SessionLayoutTest, NotifyWindowStatusDidChangeIfNeedWhenUpdateRect, TestSize.Level1)
{
    ASSERT_NE(session_, nullptr);

    sptr<SessionStageMocker> mockSessionStage = sptr<SessionStageMocker>::MakeSptr();
    EXPECT_CALL(*mockSessionStage, NotifyLayoutFinishAfterWindowModeChange(_)).Times(0);
    session_->NotifyWindowStatusDidChangeIfNeedWhenUpdateRect(SizeChangeReason::MAXIMIZE);

    session_->sessionStage_ = mockSessionStage;
    EXPECT_CALL(*mockSessionStage, NotifyLayoutFinishAfterWindowModeChange(_)).Times(1);

    session_->NotifyWindowStatusDidChangeIfNeedWhenUpdateRect(SizeChangeReason::MAXIMIZE);
    EXPECT_CALL(*mockSessionStage, NotifyLayoutFinishAfterWindowModeChange(_)).Times(1);

    session_->NotifyWindowStatusDidChangeIfNeedWhenUpdateRect(SizeChangeReason::MAXIMIZE_IN_IMPLICT);
}

/**
 * @tc.name: SetHasRequestedVsyncFunc
 * @tc.desc: SetHasRequestedVsyncFunc
 * @tc.type: FUNC
 */
HWTEST_F(SessionLayoutTest, SetHasRequestedVsyncFunc, TestSize.Level1)
{
    SessionInfo info;
    info.abilityName_ = "SetHasRequestedVsyncFunc";
    info.bundleName_ = "SetHasRequestedVsyncFunc";
    sptr<Session> session = sptr<Session>::MakeSptr(info);
    session->SetHasRequestedVsyncFunc(nullptr);
    ASSERT_EQ(nullptr, session->hasRequestedVsyncFunc_);
    session->SetHasRequestedVsyncFunc([](bool& hasRequestedVsync) {
        hasRequestedVsync = true;
        return WSError::WS_OK;
    });
    ASSERT_NE(nullptr, session->hasRequestedVsyncFunc_);
}

/**
 * @tc.name: SetRequestNextVsyncWhenModeChangeFunc
 * @tc.desc: SetRequestNextVsyncWhenModeChangeFunc
 * @tc.type: FUNC
 */
HWTEST_F(SessionLayoutTest, SetRequestNextVsyncWhenModeChangeFunc, TestSize.Level1)
{
    SessionInfo info;
    info.abilityName_ = "SetRequestNextVsyncWhenModeChangeFunc";
    info.bundleName_ = "SetRequestNextVsyncWhenModeChangeFunc";
    sptr<Session> session = sptr<Session>::MakeSptr(info);
    session->SetRequestNextVsyncWhenModeChangeFunc(nullptr);
    ASSERT_EQ(nullptr, session->requestNextVsyncWhenModeChangeFunc_);
    session->SetRequestNextVsyncWhenModeChangeFunc([](const std::shared_ptr<VsyncCallback>& vsyncCallback) {
        return WSError::WS_OK;
    });
    ASSERT_NE(nullptr, session->requestNextVsyncWhenModeChangeFunc_);
}

/**
 * @tc.name: RequestNextVsyncWhenModeChange
 * @tc.desc: RequestNextVsyncWhenModeChange
 * @tc.type: FUNC
 */
HWTEST_F(SessionLayoutTest, RequestNextVsyncWhenModeChange, TestSize.Level1)
{
    SessionInfo info;
    info.abilityName_ = "RequestNextVsyncWhenModeChange";
    info.bundleName_ = "RequestNextVsyncWhenModeChange";
    sptr<Session> session = sptr<Session>::MakeSptr(info);
    auto ret = session->RequestNextVsyncWhenModeChange();
    EXPECT_EQ(WSError::WS_ERROR_NULLPTR, ret);
    session->SetHasRequestedVsyncFunc([](bool& hasRequestedVsync) {
        hasRequestedVsync = true;
        return WSError::WS_OK;
    });
    ret = session->RequestNextVsyncWhenModeChange();
    EXPECT_EQ(WSError::WS_OK, ret);
}

/**
 * @tc.name: InitVsyncCallbackForModeChangeAndRequestNextVsync
 * @tc.desc: InitVsyncCallbackForModeChangeAndRequestNextVsync
 * @tc.type: FUNC
 */
HWTEST_F(SessionLayoutTest, InitVsyncCallbackForModeChangeAndRequestNextVsync, TestSize.Level1)
{
    SessionInfo info;
    info.abilityName_ = "InitVsyncCallbackForModeChangeAndRequestNextVsync";
    info.bundleName_ = "InitVsyncCallbackForModeChangeAndRequestNextVsync";
    sptr<Session> session = sptr<Session>::MakeSptr(info);
    session->isWindowModeDirty_.store(true);
    session->timesToWaitForVsync_.store(2);
    session->requestNextVsyncWhenModeChangeFunc_ = nullptr;
    session->InitVsyncCallbackForModeChangeAndRequestNextVsync();
    usleep(WAIT_SYNC_IN_NS);
    EXPECT_EQ(session->timesToWaitForVsync_.load(), 2);

    session->SetRequestNextVsyncWhenModeChangeFunc([](const std::shared_ptr<VsyncCallback>& vsyncCallback) {
        vsyncCallback->onCallback(1, 1);
        return WSError::WS_OK;
    });
    session->InitVsyncCallbackForModeChangeAndRequestNextVsync();
    usleep(WAIT_SYNC_IN_NS);
    EXPECT_NE(session->timesToWaitForVsync_.load(), 2);
}

/**
 * @tc.name: OnVsyncReceivedAfterModeChanged
 * @tc.desc: OnVsyncReceivedAfterModeChanged
 * @tc.type: FUNC
 */
HWTEST_F(SessionLayoutTest, OnVsyncReceivedAfterModeChanged, TestSize.Level1)
{
    SessionInfo info;
    info.abilityName_ = "OnVsyncReceivedAfterModeChanged";
    info.bundleName_ = "OnVsyncReceivedAfterModeChanged";
    sptr<Session> session = sptr<Session>::MakeSptr(info);
    session->isWindowModeDirty_.store(false);
    session->timesToWaitForVsync_.store(2);
    session->OnVsyncReceivedAfterModeChanged();
    usleep(WAIT_SYNC_IN_NS);
    EXPECT_EQ(session->timesToWaitForVsync_.load(), 2);

    session->SetRequestNextVsyncWhenModeChangeFunc([](const std::shared_ptr<VsyncCallback>& vsyncCallback) {
        vsyncCallback->onCallback(1, 1);
        return WSError::WS_OK;
    });
    session->isWindowModeDirty_.store(true);
    session->OnVsyncReceivedAfterModeChanged();
    usleep(WAIT_SYNC_IN_NS);
    EXPECT_NE(session->timesToWaitForVsync_.load(), 2);

    session->isWindowModeDirty_.store(true);
    session->timesToWaitForVsync_.store(-1);
    session->OnVsyncReceivedAfterModeChanged();
    usleep(WAIT_SYNC_IN_NS);
    EXPECT_EQ(session->timesToWaitForVsync_.load(), 0);

    session->isWindowModeDirty_.store(true);
    session->timesToWaitForVsync_.store(1);
    session->sessionStage_ = nullptr;
    session->OnVsyncReceivedAfterModeChanged();
    usleep(WAIT_SYNC_IN_NS);
    EXPECT_EQ(session->isWindowModeDirty_.load(), false);

    session->isWindowModeDirty_.store(true);
    session->timesToWaitForVsync_.store(1);
    session->sessionStage_ = sptr<SessionStageMocker>::MakeSptr();
    session->OnVsyncReceivedAfterModeChanged();
    usleep(WAIT_SYNC_IN_NS);
    EXPECT_EQ(session->isWindowModeDirty_.load(), false);
}

/**
 * @tc.name: SetGetRsCmdBlockingCountFunc
 * @tc.desc: SetGetRsCmdBlockingCountFunc
 * @tc.type: FUNC
 */
HWTEST_F(SessionLayoutTest, SetGetRsCmdBlockingCountFunc, TestSize.Level1)
{
    SessionInfo info;
    info.abilityName_ = "SetGetRsCmdBlockingCountFunc";
    info.bundleName_ = "SetGetRsCmdBlockingCountFunc";
    sptr<Session> session = sptr<Session>::MakeSptr(info);
    session->SetGetRsCmdBlockingCountFunc(nullptr);
    ASSERT_EQ(nullptr, session->getRsCmdBlockingCountFunc_);
    session->SetGetRsCmdBlockingCountFunc([]() {
        return 0;
    });
    ASSERT_NE(nullptr, session->getRsCmdBlockingCountFunc_);
}

/**
 * @tc.name: NotifyAppHookWindowInfoUpdated
 * @tc.desc: NotifyAppHookWindowInfoUpdated
 * @tc.type: FUNC
 */
HWTEST_F(SessionLayoutTest, NotifyAppHookWindowInfoUpdated, TestSize.Level1)
{
    SessionInfo info;
    info.abilityName_ = "NotifyAppHookWindowInfoUpdated";
    info.bundleName_ = "NotifyAppHookWindowInfoUpdated";
    sptr<Session> session = sptr<Session>::MakeSptr(info);

    session->sessionStage_ = nullptr;
    WSError errCode = session->NotifyAppHookWindowInfoUpdated();
    EXPECT_EQ(errCode, WSError::WS_ERROR_NULLPTR);

    session->sessionStage_ = sptr<SessionStageMocker>::MakeSptr();
    errCode = session->NotifyAppHookWindowInfoUpdated();
    EXPECT_EQ(errCode, WSError::WS_OK);
}
} // namespace
} // namespace Rosen
} // namespace OHOS
