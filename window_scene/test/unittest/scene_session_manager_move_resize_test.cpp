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

#include <gtest/gtest.h>
#include <unistd.h>
#include "parameters.h"
#include "session_manager/include/scene_session_manager.h"
#include "session/host/include/scene_session.h"
#include "session_info.h"
#include "wm_common.h"
#include "mock/mock_accesstoken_kit.h"
#include "window_manager_hilog.h"

using namespace testing;
using namespace testing::ext;

namespace OHOS {
namespace Rosen {

class SceneSessionManagerMoveResizeTest : public testing::Test {
public:
    static void SetUpTestCase();
    static void TearDownTestCase();
    void SetUp() override;
    void TearDown() override;

    static sptr<SceneSessionManager> ssm_;

private:
    static constexpr uint32_t WAIT_SYNC_IN_US = 200000;
};

sptr<SceneSessionManager> SceneSessionManagerMoveResizeTest::ssm_ = nullptr;

void SceneSessionManagerMoveResizeTest::SetUpTestCase()
{
    ssm_ = &SceneSessionManager::GetInstance();
}

void SceneSessionManagerMoveResizeTest::TearDownTestCase()
{
    ssm_ = nullptr;
}

void SceneSessionManagerMoveResizeTest::SetUp()
{
    ssm_->sceneSessionMap_.clear();
}

void SceneSessionManagerMoveResizeTest::TearDown()
{
    usleep(WAIT_SYNC_IN_US);
    MockAccesstokenKit::ChangeMockStateToInit();
    ssm_->sceneSessionMap_.clear();
}

namespace {
class WindowUITypeGuard {
public:
    explicit WindowUITypeGuard(SceneSessionManager* ssm)
        : ssm_(ssm), oldUIType_(ssm->systemConfig_.windowUIType_) {}
    ~WindowUITypeGuard()
    {
        ssm_->systemConfig_.windowUIType_ = oldUIType_;
    }
private:
    SceneSessionManager* ssm_;
    WindowUIType oldUIType_;
};

constexpr int32_t MOVE_TEST_WINDOW_ID = 301;
constexpr int32_t MOVE_TEST_POS_X = 100;
constexpr int32_t MOVE_TEST_POS_Y = 200;
constexpr int32_t MOVE_TEST_WIDTH = 300;
constexpr int32_t MOVE_TEST_HEIGHT = 400;
constexpr uint32_t WAIT_ASYNC_EXEC_IN_US = 50000;
} // namespace

/**
 * @tc.name: MoveTargetWindowPermissionDenied
 * @tc.desc: Permission denied when caller is neither SA nor has CONTROL_DEVICE permission
 * @tc.type: FUNC
 */
HWTEST_F(SceneSessionManagerMoveResizeTest, MoveTargetWindowPermissionDenied, TestSize.Level1)
{
    ASSERT_NE(nullptr, ssm_);
    MockAccesstokenKit::MockIsSACalling(false);
    MockAccesstokenKit::MockAccessTokenKitRet(-1);
    ASSERT_EQ(ssm_->MoveTargetWindow(MOVE_TEST_WINDOW_ID, MOVE_TEST_POS_X, MOVE_TEST_POS_Y).errCode,
        WSError::WS_ERROR_INVALID_PERMISSION);
}

/**
 * @tc.name: MoveTargetWindowDeviceNotSupport
 * @tc.desc: Device not support when not in PC mode
 * @tc.type: FUNC
 */
HWTEST_F(SceneSessionManagerMoveResizeTest, MoveTargetWindowDeviceNotSupport, TestSize.Level1)
{
    ASSERT_NE(nullptr, ssm_);
    MockAccesstokenKit::MockIsSACalling(true);
    WindowUITypeGuard uiTypeGuard(ssm_.GetRefPtr());
    ssm_->systemConfig_.windowUIType_ = WindowUIType::PHONE_WINDOW;
    ASSERT_EQ(ssm_->MoveTargetWindow(MOVE_TEST_WINDOW_ID, MOVE_TEST_POS_X, MOVE_TEST_POS_Y).errCode,
        WSError::WS_ERROR_DEVICE_NOT_SUPPORT);
}

/**
 * @tc.name: MoveTargetWindowSessionNotFound
 * @tc.desc: Session not found
 * @tc.type: FUNC
 */
HWTEST_F(SceneSessionManagerMoveResizeTest, MoveTargetWindowSessionNotFound, TestSize.Level1)
{
    ASSERT_NE(nullptr, ssm_);
    MockAccesstokenKit::MockIsSACalling(true);
    WindowUITypeGuard uiTypeGuard(ssm_.GetRefPtr());
    ssm_->systemConfig_.windowUIType_ = WindowUIType::PC_WINDOW;
    ssm_->sceneSessionMap_.erase(MOVE_TEST_WINDOW_ID);
    ASSERT_EQ(ssm_->MoveTargetWindow(MOVE_TEST_WINDOW_ID, MOVE_TEST_POS_X, MOVE_TEST_POS_Y).errCode,
        WSError::WS_ERROR_INVALID_PARAM);
}

/**
 * @tc.name: MoveTargetWindowSessionTerminated
 * @tc.desc: Session is terminating (disconnected)
 * @tc.type: FUNC
 */
HWTEST_F(SceneSessionManagerMoveResizeTest, MoveTargetWindowSessionTerminated, TestSize.Level1)
{
    ASSERT_NE(nullptr, ssm_);
    MockAccesstokenKit::MockIsSACalling(true);
    WindowUITypeGuard uiTypeGuard(ssm_.GetRefPtr());
    ssm_->systemConfig_.windowUIType_ = WindowUIType::PC_WINDOW;

    SessionInfo sessionInfo;
    sessionInfo.persistentId_ = MOVE_TEST_WINDOW_ID;
    sptr<SceneSession> sceneSession = sptr<SceneSession>::MakeSptr(sessionInfo, nullptr);
    ASSERT_NE(sceneSession, nullptr);
    sceneSession->SetSessionState(SessionState::STATE_DISCONNECT);
    int32_t persistentId = sceneSession->GetPersistentId();
    ssm_->sceneSessionMap_.insert({ persistentId, sceneSession });
    ASSERT_EQ(ssm_->MoveTargetWindow(persistentId, MOVE_TEST_POS_X, MOVE_TEST_POS_Y).errCode,
        WSError::WS_ERROR_INVALID_PARAM);
    ssm_->sceneSessionMap_.erase(persistentId);
}

/**
 * @tc.name: MoveTargetWindowNotMainWindow
 * @tc.desc: Window is not main window
 * @tc.type: FUNC
 */
HWTEST_F(SceneSessionManagerMoveResizeTest, MoveTargetWindowNotMainWindow, TestSize.Level1)
{
    ASSERT_NE(nullptr, ssm_);
    MockAccesstokenKit::MockIsSACalling(true);
    WindowUITypeGuard uiTypeGuard(ssm_.GetRefPtr());
    ssm_->systemConfig_.windowUIType_ = WindowUIType::PC_WINDOW;

    SessionInfo sessionInfo;
    sessionInfo.persistentId_ = MOVE_TEST_WINDOW_ID;
    sptr<SceneSession> sceneSession = sptr<SceneSession>::MakeSptr(sessionInfo, nullptr);
    ASSERT_NE(sceneSession, nullptr);
    sceneSession->property_->SetWindowType(WindowType::WINDOW_TYPE_APP_SUB_WINDOW);
    sceneSession->SetSessionState(SessionState::STATE_ACTIVE);
    int32_t persistentId = sceneSession->GetPersistentId();
    ssm_->sceneSessionMap_.insert({ persistentId, sceneSession });
    ASSERT_EQ(ssm_->MoveTargetWindow(persistentId, MOVE_TEST_POS_X, MOVE_TEST_POS_Y).errCode,
        WSError::WS_ERROR_INVALID_CALLING);
    ssm_->sceneSessionMap_.erase(persistentId);
}

/**
 * @tc.name: MoveTargetWindowNotFloating
 * @tc.desc: Window mode is not floating (fullscreen by default), returns 1300010 mapped error
 * @tc.type: FUNC
 */
HWTEST_F(SceneSessionManagerMoveResizeTest, MoveTargetWindowNotFloating, TestSize.Level1)
{
    ASSERT_NE(nullptr, ssm_);
    MockAccesstokenKit::MockIsSACalling(true);
    WindowUITypeGuard uiTypeGuard(ssm_.GetRefPtr());
    ssm_->systemConfig_.windowUIType_ = WindowUIType::PC_WINDOW;

    SessionInfo sessionInfo;
    sessionInfo.persistentId_ = MOVE_TEST_WINDOW_ID;
    sptr<SceneSession> sceneSession = sptr<SceneSession>::MakeSptr(sessionInfo, nullptr);
    ASSERT_NE(sceneSession, nullptr);
    sceneSession->property_->SetWindowType(WindowType::APP_MAIN_WINDOW_BASE);
    // default window mode of WindowSessionProperty is WINDOW_MODE_FULLSCREEN
    sceneSession->SetSessionState(SessionState::STATE_ACTIVE);
    int32_t persistentId = sceneSession->GetPersistentId();
    ssm_->sceneSessionMap_.insert({ persistentId, sceneSession });
    // WS_ERROR_OPER_FULLSCREEN_FAILED maps to 1300010 on the js side (non-FLOATING rejection)
    ASSERT_EQ(ssm_->MoveTargetWindow(persistentId, MOVE_TEST_POS_X, MOVE_TEST_POS_Y).errCode,
        WSError::WS_ERROR_OPER_FULLSCREEN_FAILED);
    ssm_->sceneSessionMap_.erase(persistentId);
}

/**
 * @tc.name: MoveTargetWindowSuccess
 * @tc.desc: Move floating main window to target position success, verify request rect updated
 * @tc.type: FUNC
 */
HWTEST_F(SceneSessionManagerMoveResizeTest, MoveTargetWindowSuccess, TestSize.Level1)
{
    ASSERT_NE(nullptr, ssm_);
    MockAccesstokenKit::MockIsSACalling(true);
    WindowUITypeGuard uiTypeGuard(ssm_.GetRefPtr());
    ssm_->systemConfig_.windowUIType_ = WindowUIType::PC_WINDOW;

    SessionInfo sessionInfo;
    sessionInfo.persistentId_ = MOVE_TEST_WINDOW_ID;
    sptr<SceneSession> sceneSession = sptr<SceneSession>::MakeSptr(sessionInfo, nullptr);
    ASSERT_NE(sceneSession, nullptr);
    sceneSession->property_->SetWindowType(WindowType::APP_MAIN_WINDOW_BASE);
    sceneSession->property_->SetWindowMode(WindowMode::WINDOW_MODE_FLOATING);
    sceneSession->SetSessionState(SessionState::STATE_ACTIVE);
    WSRect curRect = { 50, 60, MOVE_TEST_WIDTH, MOVE_TEST_HEIGHT };
    sceneSession->SetSessionRect(curRect);
    int32_t persistentId = sceneSession->GetPersistentId();
    ssm_->sceneSessionMap_.insert({ persistentId, sceneSession });

    ASSERT_EQ(ssm_->MoveTargetWindow(persistentId, MOVE_TEST_POS_X, MOVE_TEST_POS_Y).errCode, WSError::WS_OK);

    // UpdateSessionRect posts to session thread; session created without handler executes inline,
    // wait briefly anyway in case a handler was set elsewhere
    usleep(WAIT_ASYNC_EXEC_IN_US);
    WSRect requestRect = sceneSession->GetSessionRequestRect();
    EXPECT_EQ(requestRect.posX_, MOVE_TEST_POS_X);
    EXPECT_EQ(requestRect.posY_, MOVE_TEST_POS_Y);

    ssm_->sceneSessionMap_.erase(persistentId);
}

/**
 * @tc.name: ResizeWindowPermissionDenied
 * @tc.desc: Permission denied when caller is neither SA nor has CONTROL_DEVICE permission
 * @tc.type: FUNC
 */
HWTEST_F(SceneSessionManagerMoveResizeTest, ResizeWindowPermissionDenied, TestSize.Level1)
{
    ASSERT_NE(nullptr, ssm_);
    MockAccesstokenKit::MockIsSACalling(false);
    MockAccesstokenKit::MockAccessTokenKitRet(-1);
    ASSERT_EQ(ssm_->ResizeWindow(MOVE_TEST_WINDOW_ID, MOVE_TEST_WIDTH, MOVE_TEST_HEIGHT).errCode,
        WSError::WS_ERROR_INVALID_PERMISSION);
}

/**
 * @tc.name: ResizeWindowDeviceNotSupport
 * @tc.desc: Device not support when not in PC mode
 * @tc.type: FUNC
 */
HWTEST_F(SceneSessionManagerMoveResizeTest, ResizeWindowDeviceNotSupport, TestSize.Level1)
{
    ASSERT_NE(nullptr, ssm_);
    MockAccesstokenKit::MockIsSACalling(true);
    WindowUITypeGuard uiTypeGuard(ssm_.GetRefPtr());
    ssm_->systemConfig_.windowUIType_ = WindowUIType::PHONE_WINDOW;
    ASSERT_EQ(ssm_->ResizeWindow(MOVE_TEST_WINDOW_ID, MOVE_TEST_WIDTH, MOVE_TEST_HEIGHT).errCode,
        WSError::WS_ERROR_DEVICE_NOT_SUPPORT);
}

/**
 * @tc.name: ResizeWindowInvalidSize
 * @tc.desc: Invalid size when width or height is not positive
 * @tc.type: FUNC
 */
HWTEST_F(SceneSessionManagerMoveResizeTest, ResizeWindowInvalidSize, TestSize.Level1)
{
    ASSERT_NE(nullptr, ssm_);
    MockAccesstokenKit::MockIsSACalling(true);
    WindowUITypeGuard uiTypeGuard(ssm_.GetRefPtr());
    ssm_->systemConfig_.windowUIType_ = WindowUIType::PC_WINDOW;
    ASSERT_EQ(ssm_->ResizeWindow(MOVE_TEST_WINDOW_ID, 0, MOVE_TEST_HEIGHT).errCode,
        WSError::WS_ERROR_INVALID_PARAM);
    ASSERT_EQ(ssm_->ResizeWindow(MOVE_TEST_WINDOW_ID, MOVE_TEST_WIDTH, -1).errCode,
        WSError::WS_ERROR_INVALID_PARAM);
}

/**
 * @tc.name: ResizeWindowSessionNotFound
 * @tc.desc: Session not found
 * @tc.type: FUNC
 */
HWTEST_F(SceneSessionManagerMoveResizeTest, ResizeWindowSessionNotFound, TestSize.Level1)
{
    ASSERT_NE(nullptr, ssm_);
    MockAccesstokenKit::MockIsSACalling(true);
    WindowUITypeGuard uiTypeGuard(ssm_.GetRefPtr());
    ssm_->systemConfig_.windowUIType_ = WindowUIType::PC_WINDOW;
    ssm_->sceneSessionMap_.erase(MOVE_TEST_WINDOW_ID);
    ASSERT_EQ(ssm_->ResizeWindow(MOVE_TEST_WINDOW_ID, MOVE_TEST_WIDTH, MOVE_TEST_HEIGHT).errCode,
        WSError::WS_ERROR_INVALID_PARAM);
}

/**
 * @tc.name: ResizeWindowNotMainWindow
 * @tc.desc: Window is not main window
 * @tc.type: FUNC
 */
HWTEST_F(SceneSessionManagerMoveResizeTest, ResizeWindowNotMainWindow, TestSize.Level1)
{
    ASSERT_NE(nullptr, ssm_);
    MockAccesstokenKit::MockIsSACalling(true);
    WindowUITypeGuard uiTypeGuard(ssm_.GetRefPtr());
    ssm_->systemConfig_.windowUIType_ = WindowUIType::PC_WINDOW;

    SessionInfo sessionInfo;
    sessionInfo.persistentId_ = MOVE_TEST_WINDOW_ID;
    sptr<SceneSession> sceneSession = sptr<SceneSession>::MakeSptr(sessionInfo, nullptr);
    ASSERT_NE(sceneSession, nullptr);
    sceneSession->property_->SetWindowType(WindowType::WINDOW_TYPE_APP_SUB_WINDOW);
    sceneSession->SetSessionState(SessionState::STATE_ACTIVE);
    int32_t persistentId = sceneSession->GetPersistentId();
    ssm_->sceneSessionMap_.insert({ persistentId, sceneSession });
    ASSERT_EQ(ssm_->ResizeWindow(persistentId, MOVE_TEST_WIDTH, MOVE_TEST_HEIGHT).errCode,
        WSError::WS_ERROR_INVALID_CALLING);
    ssm_->sceneSessionMap_.erase(persistentId);
}

/**
 * @tc.name: ResizeWindowNotFloating
 * @tc.desc: Window mode is not floating (fullscreen by default)
 * @tc.type: FUNC
 */
HWTEST_F(SceneSessionManagerMoveResizeTest, ResizeWindowNotFloating, TestSize.Level1)
{
    ASSERT_NE(nullptr, ssm_);
    MockAccesstokenKit::MockIsSACalling(true);
    WindowUITypeGuard uiTypeGuard(ssm_.GetRefPtr());
    ssm_->systemConfig_.windowUIType_ = WindowUIType::PC_WINDOW;

    SessionInfo sessionInfo;
    sessionInfo.persistentId_ = MOVE_TEST_WINDOW_ID;
    sptr<SceneSession> sceneSession = sptr<SceneSession>::MakeSptr(sessionInfo, nullptr);
    ASSERT_NE(sceneSession, nullptr);
    sceneSession->property_->SetWindowType(WindowType::APP_MAIN_WINDOW_BASE);
    // default window mode of WindowSessionProperty is WINDOW_MODE_FULLSCREEN
    sceneSession->SetSessionState(SessionState::STATE_ACTIVE);
    int32_t persistentId = sceneSession->GetPersistentId();
    ssm_->sceneSessionMap_.insert({ persistentId, sceneSession });
    // WS_ERROR_OPER_FULLSCREEN_FAILED maps to 1300010 on the js side (non-FLOATING rejection)
    ASSERT_EQ(ssm_->ResizeWindow(persistentId, MOVE_TEST_WIDTH, MOVE_TEST_HEIGHT).errCode,
        WSError::WS_ERROR_OPER_FULLSCREEN_FAILED);
    ssm_->sceneSessionMap_.erase(persistentId);
}

/**
 * @tc.name: ResizeWindowSuccess
 * @tc.desc: Resize floating main window success, verify request rect updated with target size
 * @tc.type: FUNC
 */
HWTEST_F(SceneSessionManagerMoveResizeTest, ResizeWindowSuccess, TestSize.Level1)
{
    ASSERT_NE(nullptr, ssm_);
    MockAccesstokenKit::MockIsSACalling(true);
    WindowUITypeGuard uiTypeGuard(ssm_.GetRefPtr());
    ssm_->systemConfig_.windowUIType_ = WindowUIType::PC_WINDOW;

    SessionInfo sessionInfo;
    sessionInfo.persistentId_ = MOVE_TEST_WINDOW_ID;
    sptr<SceneSession> sceneSession = sptr<SceneSession>::MakeSptr(sessionInfo, nullptr);
    ASSERT_NE(sceneSession, nullptr);
    sceneSession->property_->SetWindowType(WindowType::APP_MAIN_WINDOW_BASE);
    sceneSession->property_->SetWindowMode(WindowMode::WINDOW_MODE_FLOATING);
    sceneSession->SetSessionState(SessionState::STATE_ACTIVE);
    WSRect curRect = { MOVE_TEST_POS_X, MOVE_TEST_POS_Y, MOVE_TEST_WIDTH, MOVE_TEST_HEIGHT };
    sceneSession->SetSessionRect(curRect);
    int32_t persistentId = sceneSession->GetPersistentId();
    ssm_->sceneSessionMap_.insert({ persistentId, sceneSession });

    constexpr int32_t RESIZE_TEST_WIDTH = 500;
    constexpr int32_t RESIZE_TEST_HEIGHT = 600;
    ASSERT_EQ(ssm_->ResizeWindow(persistentId, RESIZE_TEST_WIDTH, RESIZE_TEST_HEIGHT).errCode,
        WSError::WS_OK);

    // UpdateSessionRect posts to session thread; session created without handler executes inline,
    // wait briefly anyway in case a handler was set elsewhere
    usleep(WAIT_ASYNC_EXEC_IN_US);
    WSRect requestRect = sceneSession->GetSessionRequestRect();
    EXPECT_EQ(requestRect.width_, RESIZE_TEST_WIDTH);
    EXPECT_EQ(requestRect.height_, RESIZE_TEST_HEIGHT);

    ssm_->sceneSessionMap_.erase(persistentId);
}
} // namespace Rosen
} // namespace OHOS
