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

#include "fold_screen_state_internel.h"
#include "interfaces/include/ws_common.h"
#include "ipc_skeleton.h"
#include "mock/mock_accesstoken_kit.h"
#include "session/host/include/scene_session.h"
#include "session_info.h"
#include "session_manager/include/scene_session_manager.h"
#include "wm_common.h"

using namespace testing;
using namespace testing::ext;

namespace OHOS {
namespace Rosen {
namespace {
const int32_t SLEEP_TIME{ 10000 };
} // namespace

class SceneSessionManagerTest14 : public testing::Test {
public:
    static void SetUpTestCase();
    static void TearDownTestCase();
    void SetUp() override;
    void TearDown() override;

    sptr<SceneSession> CreateSceneSession(const std::string& bundleName, WindowType windowType);
    void InsertDefaultPhysicalScreen();
    void ClearScreenSessions();

    static sptr<SceneSessionManager> ssm_;
};

sptr<SceneSessionManager> SceneSessionManagerTest14::ssm_ = nullptr;

void SceneSessionManagerTest14::SetUpTestCase()
{
    ssm_ = &SceneSessionManager::GetInstance();
    ssm_->systemConfig_.windowUIType_ = WindowUIType::PC_WINDOW;
}

void SceneSessionManagerTest14::TearDownTestCase()
{
    ssm_ = nullptr;
}

void SceneSessionManagerTest14::SetUp()
{
    ssm_->sceneSessionMap_.clear();
    ClearScreenSessions();
}

void SceneSessionManagerTest14::TearDown()
{
    ssm_->sceneSessionMap_.clear();
    ClearScreenSessions();
    MockAccesstokenKit::ChangeMockStateToInit();
}

void SceneSessionManagerTest14::InsertDefaultPhysicalScreen()
{
    ScreenSessionConfig config;
    sptr<ScreenSession> screenSession =
        sptr<ScreenSession>::MakeSptr(config, ScreenSessionReason::CREATE_SESSION_FOR_CLIENT);
    ScreenSessionManagerClient::GetInstance().screenSessionMap_.insert(
        std::make_pair(DEFAULT_DISPLAY_ID, screenSession));
}

void SceneSessionManagerTest14::ClearScreenSessions()
{
    ScreenSessionManagerClient::GetInstance().screenSessionMap_.clear();
}

sptr<SceneSession> SceneSessionManagerTest14::CreateSceneSession(const std::string& bundleName,
    WindowType windowType)
{
    SessionInfo sessionInfo;
    sessionInfo.bundleName_ = bundleName;

    sptr<WindowSessionProperty> property = sptr<WindowSessionProperty>::MakeSptr();
    property->SetWindowType(windowType);
    property->SetWindowName(bundleName);

    sptr<SceneSession> sceneSession = sptr<SceneSession>::MakeSptr(sessionInfo, nullptr);
    sceneSession->property_ = property;
    return sceneSession;
}

/**
 * @tc.name: SetWindowPositionNotPcWindow
 * @tc.desc: test function : SetWindowPosition on a non-PC device
 * @tc.type: FUNC
 */
HWTEST_F(SceneSessionManagerTest14, SetWindowPositionNotPcWindow, TestSize.Level1)
{
    // 1. Set up the test environment
    ssm_->systemConfig_.windowUIType_ = WindowUIType::PHONE_WINDOW;
    WindowPositionParams position;
    position.windowId = 1;
    position.insertAfter = 0;
    WindowPositionInfo windowPositionInfo;
    windowPositionInfo.windowPositions.push_back(position);
    // 2. Call the function to be tested
    WSError ret = ssm_->SetWindowPosition(windowPositionInfo);
    // 3. Verify the result
    EXPECT_EQ(ret, WSError::WS_ERROR_DEVICE_NOT_SUPPORT);
    ssm_->systemConfig_.windowUIType_ = WindowUIType::PC_WINDOW;
}

/**
 * @tc.name: SetWindowPositionEmptyList
 * @tc.desc: test function : SetWindowPosition with empty list
 * @tc.type: FUNC
 */
HWTEST_F(SceneSessionManagerTest14, SetWindowPositionEmptyList, TestSize.Level1)
{
    // 1. Set up the test environment
    WindowPositionInfo windowPositionInfo;
    // 2. Call the function to be tested
    WSError ret = ssm_->SetWindowPosition(windowPositionInfo);
    // 3. Verify the result
    EXPECT_EQ(ret, WSError::WS_ERROR_INVALID_PARAM);
}

/**
 * @tc.name: SetWindowPositionInvalidWindowId
 * @tc.desc: test function : SetWindowPosition with invalid windowId
 * @tc.type: FUNC
 */
HWTEST_F(SceneSessionManagerTest14, SetWindowPositionInvalidWindowId, TestSize.Level1)
{
    // 1. Set up the test environment
    WindowPositionParams position;
    position.windowId = 0;
    position.insertAfter = 0;
    WindowPositionInfo windowPositionInfo;
    windowPositionInfo.windowPositions.push_back(position);
    // 2. Call the function to be tested
    WSError ret = ssm_->SetWindowPosition(windowPositionInfo);
    // 3. Verify the result
    EXPECT_EQ(ret, WSError::WS_ERROR_INVALID_PARAM);
}

/**
 * @tc.name: SetWindowPositionInvalidInsertAfter
 * @tc.desc: test function : SetWindowPosition with invalid insertAfter
 * @tc.type: FUNC
 */
HWTEST_F(SceneSessionManagerTest14, SetWindowPositionInvalidInsertAfter, TestSize.Level1)
{
    // 1. Set up the test environment
    WindowPositionParams position;
    position.windowId = 1;
    position.insertAfter = -4; // less than WindowPosition::NOT_TOPMOST
    WindowPositionInfo windowPositionInfo;
    windowPositionInfo.windowPositions.push_back(position);
    // 2. Call the function to be tested
    WSError ret = ssm_->SetWindowPosition(windowPositionInfo);
    // 3. Verify the result
    EXPECT_EQ(ret, WSError::WS_ERROR_INVALID_PARAM);
}

/**
 * @tc.name: SetWindowPositionTopmostNoPermission
 * @tc.desc: test function : SetWindowPosition with topmost level but no permission
 * @tc.type: FUNC
 */
HWTEST_F(SceneSessionManagerTest14, SetWindowPositionTopmostNoPermission, TestSize.Level1)
{
    // 1. Set up the test environment
    MockAccesstokenKit::MockAccessTokenKitRet(-1); // PERMISSION_DENIED
    WindowPositionParams position;
    position.windowId = 1;
    position.insertAfter = static_cast<int32_t>(WindowPosition::TOPMOST);
    WindowPositionInfo windowPositionInfo;
    windowPositionInfo.windowPositions.push_back(position);
    // 2. Call the function to be tested
    WSError ret = ssm_->SetWindowPosition(windowPositionInfo);
    // 3. Verify the result
    EXPECT_EQ(ret, WSError::WS_ERROR_INVALID_PERMISSION);
}

/**
 * @tc.name: SetWindowPositionTopmostWithPermission
 * @tc.desc: test function : SetWindowPosition with topmost level and granted permission
 * @tc.type: FUNC
 */
HWTEST_F(SceneSessionManagerTest14, SetWindowPositionTopmostWithPermission, TestSize.Level1)
{
    // 1. Set up the test environment
    MockAccesstokenKit::MockAccessTokenKitRet(0); // PERMISSION_GRANTED
    WindowPositionParams position;
    position.windowId = 1;
    position.insertAfter = static_cast<int32_t>(WindowPosition::TOPMOST);
    WindowPositionInfo windowPositionInfo;
    windowPositionInfo.windowPositions.push_back(position);
    // 2. Call the function to be tested
    WSError ret = ssm_->SetWindowPosition(windowPositionInfo);
    // 3. Verify the result
    EXPECT_EQ(ret, WSError::WS_ERROR_INVALID_SESSION);
}

/**
 * @tc.name: SetWindowPositionSessionNotFound
 * @tc.desc: test function : SetWindowPosition with non-existent session
 * @tc.type: FUNC
 */
HWTEST_F(SceneSessionManagerTest14, SetWindowPositionSessionNotFound, TestSize.Level1)
{
    // 1. Set up the test environment
    WindowPositionParams position;
    position.windowId = 1;
    position.insertAfter = 0;
    WindowPositionInfo windowPositionInfo;
    windowPositionInfo.windowPositions.push_back(position);
    // 2. Call the function to be tested
    WSError ret = ssm_->SetWindowPosition(windowPositionInfo);
    // 3. Verify the result
    EXPECT_EQ(ret, WSError::WS_ERROR_INVALID_SESSION);
}

/**
 * @tc.name: SetWindowPositionSessionTerminated
 * @tc.desc: test function : SetWindowPosition with terminated session
 * @tc.type: FUNC
 */
HWTEST_F(SceneSessionManagerTest14, SetWindowPositionSessionTerminated, TestSize.Level1)
{
    // 1. Set up the test environment
    sptr<SceneSession> sceneSession = CreateSceneSession("test", WindowType::WINDOW_TYPE_APP_MAIN_WINDOW);
    ASSERT_NE(sceneSession, nullptr);
    sceneSession->SetSessionState(SessionState::STATE_DISCONNECT);
    ssm_->sceneSessionMap_.insert({ 1, sceneSession });
    WindowPositionParams position;
    position.windowId = 1;
    position.insertAfter = 0;
    WindowPositionInfo windowPositionInfo;
    windowPositionInfo.windowPositions.push_back(position);
    // 2. Call the function to be tested
    WSError ret = ssm_->SetWindowPosition(windowPositionInfo);
    // 3. Verify the result
    EXPECT_EQ(ret, WSError::WS_ERROR_INVALID_SESSION);
    ssm_->sceneSessionMap_.clear();
}

/**
 * @tc.name: SetWindowPositionNotMainWindow
 * @tc.desc: test function : SetWindowPosition with non-main window
 * @tc.type: FUNC
 */
HWTEST_F(SceneSessionManagerTest14, SetWindowPositionNotMainWindow, TestSize.Level1)
{
    // 1. Set up the test environment
    sptr<SceneSession> sceneSession = CreateSceneSession("test", WindowType::WINDOW_TYPE_PIP);
    ASSERT_NE(sceneSession, nullptr);
    sceneSession->SetSessionState(SessionState::STATE_FOREGROUND);
    sceneSession->SetCallingPid(IPCSkeleton::GetCallingPid());
    ssm_->sceneSessionMap_.insert({ 1, sceneSession });
    WindowPositionParams position;
    position.windowId = 1;
    position.insertAfter = 0;
    WindowPositionInfo windowPositionInfo;
    windowPositionInfo.windowPositions.push_back(position);
    // 2. Call the function to be tested
    WSError ret = ssm_->SetWindowPosition(windowPositionInfo);
    // 3. Verify the result
    EXPECT_EQ(ret, WSError::WS_ERROR_INVALID_CALLING);
    ssm_->sceneSessionMap_.clear();
}

/**
 * @tc.name: SetWindowPositionDifferentProcess
 * @tc.desc: test function : SetWindowPosition called by a different process
 * @tc.type: FUNC
 */
HWTEST_F(SceneSessionManagerTest14, SetWindowPositionDifferentProcess, TestSize.Level1)
{
    // 1. Set up the test environment
    sptr<SceneSession> sceneSession = CreateSceneSession("test", WindowType::WINDOW_TYPE_APP_MAIN_WINDOW);
    ASSERT_NE(sceneSession, nullptr);
    sceneSession->SetSessionState(SessionState::STATE_FOREGROUND);
    sceneSession->SetCallingPid(IPCSkeleton::GetCallingPid() + 1);
    ssm_->sceneSessionMap_.insert({ 1, sceneSession });
    WindowPositionParams position;
    position.windowId = 1;
    position.insertAfter = 0;
    WindowPositionInfo windowPositionInfo;
    windowPositionInfo.windowPositions.push_back(position);
    // 2. Call the function to be tested
    WSError ret = ssm_->SetWindowPosition(windowPositionInfo);
    // 3. Verify the result
    EXPECT_EQ(ret, WSError::WS_ERROR_INVALID_CALLING);
    ssm_->sceneSessionMap_.clear();
}

/**
 * @tc.name: SetWindowPositionInsertAfterNotFound
 * @tc.desc: test function : SetWindowPosition with non-existent insertAfter target
 * @tc.type: FUNC
 */
HWTEST_F(SceneSessionManagerTest14, SetWindowPositionInsertAfterNotFound, TestSize.Level1)
{
    // 1. Set up the test environment
    sptr<SceneSession> sceneSession = CreateSceneSession("test", WindowType::WINDOW_TYPE_APP_MAIN_WINDOW);
    ASSERT_NE(sceneSession, nullptr);
    sceneSession->SetSessionState(SessionState::STATE_FOREGROUND);
    sceneSession->SetCallingPid(IPCSkeleton::GetCallingPid());
    ssm_->sceneSessionMap_.insert({ 1, sceneSession });
    InsertDefaultPhysicalScreen();
    WindowPositionParams position;
    position.windowId = 1;
    position.insertAfter = 999; // target does not exist
    WindowPositionInfo windowPositionInfo;
    windowPositionInfo.windowPositions.push_back(position);
    // 2. Call the function to be tested
    WSError ret = ssm_->SetWindowPosition(windowPositionInfo);
    // 3. Verify the result
    EXPECT_EQ(ret, WSError::WS_ERROR_INVALID_SESSION);
    ssm_->sceneSessionMap_.clear();
    ClearScreenSessions();
}

/**
 * @tc.name: SetWindowPositionInsertAfterValid
 * @tc.desc: test function : SetWindowPosition with valid insertAfter target
 * @tc.type: FUNC
 */
HWTEST_F(SceneSessionManagerTest14, SetWindowPositionInsertAfterValid, TestSize.Level1)
{
    // 1. Set up the test environment
    sptr<SceneSession> sceneSession = CreateSceneSession("test", WindowType::WINDOW_TYPE_APP_MAIN_WINDOW);
    ASSERT_NE(sceneSession, nullptr);
    sceneSession->SetSessionState(SessionState::STATE_FOREGROUND);
    sceneSession->SetCallingPid(IPCSkeleton::GetCallingPid());
    ssm_->sceneSessionMap_.insert({ 1, sceneSession });
    sptr<SceneSession> targetSession = CreateSceneSession("test", WindowType::WINDOW_TYPE_APP_MAIN_WINDOW);
    ASSERT_NE(targetSession, nullptr);
    targetSession->SetSessionState(SessionState::STATE_FOREGROUND);
    targetSession->SetCallingPid(IPCSkeleton::GetCallingPid());
    ssm_->sceneSessionMap_.insert({ 2, targetSession });
    InsertDefaultPhysicalScreen();
    WindowPositionParams position;
    position.windowId = 1;
    position.insertAfter = 2; // target exists
    WindowPositionInfo windowPositionInfo;
    windowPositionInfo.windowPositions.push_back(position);
    // 2. Call the function to be tested
    WSError ret = ssm_->SetWindowPosition(windowPositionInfo);
    // 3. Verify the result
    EXPECT_EQ(ret, WSError::WS_OK);
    ssm_->sceneSessionMap_.clear();
    ClearScreenSessions();
}

/**
 * @tc.name: SetWindowPositionInsertAfterNotMainWindow
 * @tc.desc: test function : SetWindowPosition with insertAfter target that is not a main window
 * @tc.type: FUNC
 */
HWTEST_F(SceneSessionManagerTest14, SetWindowPositionInsertAfterNotMainWindow, TestSize.Level1)
{
    // 1. Set up the test environment
    sptr<SceneSession> sceneSession = CreateSceneSession("test", WindowType::WINDOW_TYPE_APP_MAIN_WINDOW);
    ASSERT_NE(sceneSession, nullptr);
    sceneSession->SetSessionState(SessionState::STATE_FOREGROUND);
    sceneSession->SetCallingPid(IPCSkeleton::GetCallingPid());
    ssm_->sceneSessionMap_.insert({ 1, sceneSession });
    sptr<SceneSession> targetSession = CreateSceneSession("test", WindowType::WINDOW_TYPE_PIP);
    ASSERT_NE(targetSession, nullptr);
    targetSession->SetSessionState(SessionState::STATE_FOREGROUND);
    targetSession->SetCallingPid(IPCSkeleton::GetCallingPid());
    ssm_->sceneSessionMap_.insert({ 2, targetSession });
    InsertDefaultPhysicalScreen();
    WindowPositionParams position;
    position.windowId = 1;
    position.insertAfter = 2; // target exists but is not a main window
    WindowPositionInfo windowPositionInfo;
    windowPositionInfo.windowPositions.push_back(position);
    // 2. Call the function to be tested
    WSError ret = ssm_->SetWindowPosition(windowPositionInfo);
    // 3. Verify the result
    EXPECT_EQ(ret, WSError::WS_ERROR_INVALID_CALLING);
    ssm_->sceneSessionMap_.clear();
    ClearScreenSessions();
}

/**
 * @tc.name: SetWindowPositionInsertAfterVirtualScreen
 * @tc.desc: test function : SetWindowPosition with insertAfter target on a virtual screen
 * @tc.type: FUNC
 */
HWTEST_F(SceneSessionManagerTest14, SetWindowPositionInsertAfterVirtualScreen, TestSize.Level1)
{
    // 1. Set up the test environment
    constexpr DisplayId VIRTUAL_DISPLAY_ID = 100;
    sptr<SceneSession> sceneSession = CreateSceneSession("test", WindowType::WINDOW_TYPE_APP_MAIN_WINDOW);
    ASSERT_NE(sceneSession, nullptr);
    sceneSession->SetSessionState(SessionState::STATE_FOREGROUND);
    sceneSession->SetCallingPid(IPCSkeleton::GetCallingPid());
    ssm_->sceneSessionMap_.insert({ 1, sceneSession });
    sptr<SceneSession> targetSession = CreateSceneSession("test", WindowType::WINDOW_TYPE_APP_MAIN_WINDOW);
    ASSERT_NE(targetSession, nullptr);
    targetSession->SetSessionState(SessionState::STATE_FOREGROUND);
    targetSession->SetCallingPid(IPCSkeleton::GetCallingPid());
    targetSession->GetSessionProperty()->SetDisplayId(VIRTUAL_DISPLAY_ID);
    ssm_->sceneSessionMap_.insert({ 2, targetSession });
    // The window itself stays on a physical screen, only the insertAfter target is on a virtual one.
    InsertDefaultPhysicalScreen();
    ScreenSessionConfig config;
    sptr<ScreenSession> virtualScreenSession =
        sptr<ScreenSession>::MakeSptr(config, ScreenSessionReason::CREATE_SESSION_FOR_CLIENT);
    virtualScreenSession->SetScreenType(ScreenType::VIRTUAL);
    ScreenSessionManagerClient::GetInstance().screenSessionMap_.insert(
        std::make_pair(VIRTUAL_DISPLAY_ID, virtualScreenSession));
    WindowPositionParams position;
    position.windowId = 1;
    position.insertAfter = 2; // target is on a virtual screen
    WindowPositionInfo windowPositionInfo;
    windowPositionInfo.windowPositions.push_back(position);
    // 2. Call the function to be tested
    WSError ret = ssm_->SetWindowPosition(windowPositionInfo);
    // 3. Verify the result
    EXPECT_EQ(ret, WSError::WS_ERROR_INVALID_SESSION);
    ssm_->sceneSessionMap_.clear();
    ClearScreenSessions();
}

/**
 * @tc.name: SetWindowPositionInsertAfterOuterScreen
 * @tc.desc: test function : SetWindowPosition with insertAfter target on super multi fold outer screen
 * @tc.type: FUNC
 */
HWTEST_F(SceneSessionManagerTest14, SetWindowPositionInsertAfterOuterScreen, TestSize.Level1)
{
    if (!FoldScreenStateInternel::IsSuperFoldMultiDisplayDevice()) {
        GTEST_SKIP() << "Not SPN device, skipping test.";
    }
    // 1. Set up the test environment
    sptr<SceneSession> sceneSession = CreateSceneSession("test", WindowType::WINDOW_TYPE_APP_MAIN_WINDOW);
    ASSERT_NE(sceneSession, nullptr);
    sceneSession->SetSessionState(SessionState::STATE_FOREGROUND);
    sceneSession->SetCallingPid(IPCSkeleton::GetCallingPid());
    ssm_->sceneSessionMap_.insert({ 1, sceneSession });
    sptr<SceneSession> targetSession = CreateSceneSession("test", WindowType::WINDOW_TYPE_APP_MAIN_WINDOW);
    ASSERT_NE(targetSession, nullptr);
    targetSession->SetSessionState(SessionState::STATE_FOREGROUND);
    targetSession->SetCallingPid(IPCSkeleton::GetCallingPid());
    targetSession->GetSessionProperty()->SetDisplayId(Session::SCREEN_ID_MAIN);
    ssm_->sceneSessionMap_.insert({ 2, targetSession });
    // The window itself stays on a normal screen, only the insertAfter target is on the outer screen.
    InsertDefaultPhysicalScreen();
    WindowPositionParams position;
    position.windowId = 1;
    position.insertAfter = 2; // target is on the super multi fold outer screen
    WindowPositionInfo windowPositionInfo;
    windowPositionInfo.windowPositions.push_back(position);
    // 2. Call the function to be tested
    WSError ret = ssm_->SetWindowPosition(windowPositionInfo);
    // 3. Verify the result
    EXPECT_EQ(ret, WSError::WS_ERROR_DEVICE_NOT_SUPPORT);
    ssm_->sceneSessionMap_.clear();
    ClearScreenSessions();
}

/**
 * @tc.name: SetWindowPositionSetMainWindowTopmost
 * @tc.desc: test function : SetWindowPosition sets the main window topmost property
 * @tc.type: FUNC
 */
HWTEST_F(SceneSessionManagerTest14, SetWindowPositionSetMainWindowTopmost, TestSize.Level1)
{
    // 1. Set up the test environment
    MockAccesstokenKit::MockAccessTokenKitRet(0); // PERMISSION_GRANTED
    sptr<SceneSession> sceneSession = CreateSceneSession("test", WindowType::WINDOW_TYPE_APP_MAIN_WINDOW);
    ASSERT_NE(sceneSession, nullptr);
    sceneSession->SetSessionState(SessionState::STATE_FOREGROUND);
    sceneSession->SetCallingPid(IPCSkeleton::GetCallingPid());
    ssm_->sceneSessionMap_.insert({ 1, sceneSession });
    InsertDefaultPhysicalScreen();
    WindowPositionParams position;
    position.windowId = 1;
    position.insertAfter = static_cast<int32_t>(WindowPosition::TOPMOST);
    WindowPositionInfo windowPositionInfo;
    windowPositionInfo.windowPositions.push_back(position);
    // 2. Call the function to be tested
    WSError ret = ssm_->SetWindowPosition(windowPositionInfo);
    // 3. Verify the result
    EXPECT_EQ(ret, WSError::WS_OK);
    EXPECT_TRUE(sceneSession->GetSessionProperty()->IsMainWindowTopmost());
    ssm_->sceneSessionMap_.clear();
    ClearScreenSessions();
}

/**
 * @tc.name: SetWindowPositionRemoveMainWindowTopmost
 * @tc.desc: test function : SetWindowPosition clears the main window topmost property
 * @tc.type: FUNC
 */
HWTEST_F(SceneSessionManagerTest14, SetWindowPositionRemoveMainWindowTopmost, TestSize.Level1)
{
    // 1. Set up the test environment
    MockAccesstokenKit::MockAccessTokenKitRet(0); // PERMISSION_GRANTED
    sptr<SceneSession> sceneSession = CreateSceneSession("test", WindowType::WINDOW_TYPE_APP_MAIN_WINDOW);
    ASSERT_NE(sceneSession, nullptr);
    sceneSession->SetSessionState(SessionState::STATE_FOREGROUND);
    sceneSession->SetCallingPid(IPCSkeleton::GetCallingPid());
    sceneSession->GetSessionProperty()->SetMainWindowTopmost(true);
    ssm_->sceneSessionMap_.insert({ 1, sceneSession });
    InsertDefaultPhysicalScreen();
    WindowPositionParams position;
    position.windowId = 1;
    position.insertAfter = static_cast<int32_t>(WindowPosition::NOT_TOPMOST);
    WindowPositionInfo windowPositionInfo;
    windowPositionInfo.windowPositions.push_back(position);
    // 2. Call the function to be tested
    WSError ret = ssm_->SetWindowPosition(windowPositionInfo);
    // 3. Verify the result
    EXPECT_EQ(ret, WSError::WS_OK);
    EXPECT_FALSE(sceneSession->GetSessionProperty()->IsMainWindowTopmost());
    ssm_->sceneSessionMap_.clear();
    ClearScreenSessions();
}

/**
 * @tc.name: SetWindowPositionSingleAdjustKeepsMainWindowTopmost
 * @tc.desc: test function : a single z-order adjustment does not change the topmost property nor need permission
 * @tc.type: FUNC
 */
HWTEST_F(SceneSessionManagerTest14, SetWindowPositionSingleAdjustKeepsMainWindowTopmost, TestSize.Level1)
{
    // 1. Set up the test environment
    MockAccesstokenKit::MockAccessTokenKitRet(-1); // PERMISSION_DENIED
    sptr<SceneSession> sceneSession = CreateSceneSession("test", WindowType::WINDOW_TYPE_APP_MAIN_WINDOW);
    ASSERT_NE(sceneSession, nullptr);
    sceneSession->SetSessionState(SessionState::STATE_FOREGROUND);
    sceneSession->SetCallingPid(IPCSkeleton::GetCallingPid());
    sceneSession->GetSessionProperty()->SetMainWindowTopmost(true);
    ssm_->sceneSessionMap_.insert({ 1, sceneSession });
    InsertDefaultPhysicalScreen();
    WindowPositionParams position;
    position.windowId = 1;
    position.insertAfter = static_cast<int32_t>(WindowPosition::TOP);
    WindowPositionInfo windowPositionInfo;
    windowPositionInfo.windowPositions.push_back(position);
    // 2. Call the function to be tested
    WSError ret = ssm_->SetWindowPosition(windowPositionInfo);
    // 3. Verify the result
    EXPECT_EQ(ret, WSError::WS_OK);
    EXPECT_TRUE(sceneSession->GetSessionProperty()->IsMainWindowTopmost());
    ssm_->sceneSessionMap_.clear();
    ClearScreenSessions();
}

/**
 * @tc.name: SetWindowPositionNotTopmost
 * @tc.desc: test function : SetWindowPosition with WindowPosition::NOT_TOPMOST sentinel
 * @tc.type: FUNC
 */
HWTEST_F(SceneSessionManagerTest14, SetWindowPositionNotTopmost, TestSize.Level1)
{
    // 1. Set up the test environment
    sptr<SceneSession> sceneSession = CreateSceneSession("test", WindowType::WINDOW_TYPE_APP_MAIN_WINDOW);
    ASSERT_NE(sceneSession, nullptr);
    sceneSession->SetSessionState(SessionState::STATE_FOREGROUND);
    sceneSession->SetCallingPid(IPCSkeleton::GetCallingPid());
    ssm_->sceneSessionMap_.insert({ 1, sceneSession });
    InsertDefaultPhysicalScreen();
    WindowPositionParams position;
    position.windowId = 1;
    position.insertAfter = static_cast<int32_t>(WindowPosition::NOT_TOPMOST);
    WindowPositionInfo windowPositionInfo;
    windowPositionInfo.windowPositions.push_back(position);
    // 2. Call the function to be tested
    WSError ret = ssm_->SetWindowPosition(windowPositionInfo);
    // 3. Verify the result
    EXPECT_EQ(ret, WSError::WS_OK);
    ssm_->sceneSessionMap_.clear();
    ClearScreenSessions();
}

/**
 * @tc.name: SetWindowPositionSuccess
 * @tc.desc: test function : SetWindowPosition success and callback invoked
 * @tc.type: FUNC
 */
HWTEST_F(SceneSessionManagerTest14, SetWindowPositionSuccess, TestSize.Level1)
{
    // 1. Set up the test environment
    sptr<SceneSession> sceneSession = CreateSceneSession("test", WindowType::WINDOW_TYPE_APP_MAIN_WINDOW);
    ASSERT_NE(sceneSession, nullptr);
    sceneSession->SetSessionState(SessionState::STATE_FOREGROUND);
    sceneSession->SetCallingPid(IPCSkeleton::GetCallingPid());
    ssm_->sceneSessionMap_.insert({ 1, sceneSession });
    InsertDefaultPhysicalScreen();
    auto receivedWindowId = std::make_shared<int32_t>(0);
    ssm_->SetWindowPositionListener([receivedWindowId](const WindowPositionInfo& windowPositionInfo) {
        if (!windowPositionInfo.windowPositions.empty()) {
            *receivedWindowId = windowPositionInfo.windowPositions[0].windowId;
        }
    });
    WindowPositionParams position;
    position.windowId = 1;
    position.insertAfter = 0;
    WindowPositionInfo windowPositionInfo;
    windowPositionInfo.windowPositions.push_back(position);
    // 2. Call the function to be tested
    WSError ret = ssm_->SetWindowPosition(windowPositionInfo);
    // 3. Verify the result
    EXPECT_EQ(ret, WSError::WS_OK);
    usleep(SLEEP_TIME);
    EXPECT_EQ(*receivedWindowId, 1);
    ssm_->SetWindowPositionListener(nullptr);
    ssm_->sceneSessionMap_.clear();
    ClearScreenSessions();
}

/**
 * @tc.name: SetWindowPositionSuccessNoListener
 * @tc.desc: test function : SetWindowPosition success without listener
 * @tc.type: FUNC
 */
HWTEST_F(SceneSessionManagerTest14, SetWindowPositionSuccessNoListener, TestSize.Level1)
{
    // 1. Set up the test environment
    ssm_->SetWindowPositionListener(nullptr);
    sptr<SceneSession> sceneSession = CreateSceneSession("test", WindowType::WINDOW_TYPE_APP_MAIN_WINDOW);
    ASSERT_NE(sceneSession, nullptr);
    sceneSession->SetSessionState(SessionState::STATE_FOREGROUND);
    sceneSession->SetCallingPid(IPCSkeleton::GetCallingPid());
    ssm_->sceneSessionMap_.insert({ 1, sceneSession });
    InsertDefaultPhysicalScreen();
    WindowPositionParams position;
    position.windowId = 1;
    position.insertAfter = 0;
    WindowPositionInfo windowPositionInfo;
    windowPositionInfo.windowPositions.push_back(position);
    // 2. Call the function to be tested
    WSError ret = ssm_->SetWindowPosition(windowPositionInfo);
    // 3. Verify the result
    EXPECT_EQ(ret, WSError::WS_OK);
    ssm_->sceneSessionMap_.clear();
    ClearScreenSessions();
}

/**
 * @tc.name: SetWindowPositionNoScreen
 * @tc.desc: test function : SetWindowPosition on window with no screen session registered
 * @tc.type: FUNC
 */
HWTEST_F(SceneSessionManagerTest14, SetWindowPositionNoScreen, TestSize.Level1)
{
    // 1. Set up the test environment
    ClearScreenSessions();
    sptr<SceneSession> sceneSession = CreateSceneSession("test", WindowType::WINDOW_TYPE_APP_MAIN_WINDOW);
    ASSERT_NE(sceneSession, nullptr);
    sceneSession->SetSessionState(SessionState::STATE_FOREGROUND);
    sceneSession->SetCallingPid(IPCSkeleton::GetCallingPid());
    ssm_->sceneSessionMap_.insert({ 1, sceneSession });
    WindowPositionParams position;
    position.windowId = 1;
    position.insertAfter = 0;
    WindowPositionInfo windowPositionInfo;
    windowPositionInfo.windowPositions.push_back(position);
    // 2. Call the function to be tested
    WSError ret = ssm_->SetWindowPosition(windowPositionInfo);
    // 3. Verify the result
    EXPECT_EQ(ret, WSError::WS_ERROR_INVALID_SESSION);
    ssm_->sceneSessionMap_.clear();
    ClearScreenSessions();
}

/**
 * @tc.name: SetWindowPositionVirtualScreen
 * @tc.desc: test function : SetWindowPosition on window on a virtual screen
 * @tc.type: FUNC
 */
HWTEST_F(SceneSessionManagerTest14, SetWindowPositionVirtualScreen, TestSize.Level1)
{
    // 1. Set up the test environment
    ClearScreenSessions();
    sptr<SceneSession> sceneSession = CreateSceneSession("test", WindowType::WINDOW_TYPE_APP_MAIN_WINDOW);
    ASSERT_NE(sceneSession, nullptr);
    sceneSession->SetSessionState(SessionState::STATE_FOREGROUND);
    sceneSession->SetCallingPid(IPCSkeleton::GetCallingPid());
    ssm_->sceneSessionMap_.insert({ 1, sceneSession });
    ScreenSessionConfig config;
    sptr<ScreenSession> screenSession =
        sptr<ScreenSession>::MakeSptr(config, ScreenSessionReason::CREATE_SESSION_FOR_CLIENT);
    screenSession->SetScreenType(ScreenType::VIRTUAL);
    ScreenSessionManagerClient::GetInstance().screenSessionMap_.insert(
        std::make_pair(DEFAULT_DISPLAY_ID, screenSession));
    WindowPositionParams position;
    position.windowId = 1;
    position.insertAfter = 0;
    WindowPositionInfo windowPositionInfo;
    windowPositionInfo.windowPositions.push_back(position);
    // 2. Call the function to be tested
    WSError ret = ssm_->SetWindowPosition(windowPositionInfo);
    // 3. Verify the result
    EXPECT_EQ(ret, WSError::WS_ERROR_INVALID_SESSION);
    ssm_->sceneSessionMap_.clear();
    ClearScreenSessions();
}

/**
 * @tc.name: SetWindowPositionTopmostToNotTopmostNoPermission
 * @tc.desc: test function : SetWindowPosition remove topmost state without permission
 * @tc.type: FUNC
 */
HWTEST_F(SceneSessionManagerTest14, SetWindowPositionTopmostToNotTopmostNoPermission, TestSize.Level1)
{
    // 1. Set up the test environment
    MockAccesstokenKit::MockAccessTokenKitRet(-1); // PERMISSION_DENIED
    sptr<SceneSession> sceneSession = CreateSceneSession("test", WindowType::WINDOW_TYPE_APP_MAIN_WINDOW);
    ASSERT_NE(sceneSession, nullptr);
    sceneSession->SetSessionState(SessionState::STATE_FOREGROUND);
    sceneSession->SetCallingPid(IPCSkeleton::GetCallingPid());
    sceneSession->property_->SetMainWindowTopmost(true);
    ssm_->sceneSessionMap_.insert({ 1, sceneSession });
    InsertDefaultPhysicalScreen();
    WindowPositionParams position;
    position.windowId = 1;
    position.insertAfter = static_cast<int32_t>(WindowPosition::NOT_TOPMOST);
    WindowPositionInfo windowPositionInfo;
    windowPositionInfo.windowPositions.push_back(position);
    // 2. Call the function to be tested
    WSError ret = ssm_->SetWindowPosition(windowPositionInfo);
    // 3. Verify the result
    EXPECT_EQ(ret, WSError::WS_ERROR_INVALID_PERMISSION);
    ssm_->sceneSessionMap_.clear();
    ClearScreenSessions();
}

/**
 * @tc.name: SetWindowPositionTopmostToNotTopmostWithPermission
 * @tc.desc: test function : SetWindowPosition remove topmost state with granted permission
 * @tc.type: FUNC
 */
HWTEST_F(SceneSessionManagerTest14, SetWindowPositionTopmostToNotTopmostWithPermission, TestSize.Level1)
{
    // 1. Set up the test environment
    MockAccesstokenKit::MockAccessTokenKitRet(0); // PERMISSION_GRANTED
    sptr<SceneSession> sceneSession = CreateSceneSession("test", WindowType::WINDOW_TYPE_APP_MAIN_WINDOW);
    ASSERT_NE(sceneSession, nullptr);
    sceneSession->SetSessionState(SessionState::STATE_FOREGROUND);
    sceneSession->SetCallingPid(IPCSkeleton::GetCallingPid());
    sceneSession->property_->SetMainWindowTopmost(true);
    ssm_->sceneSessionMap_.insert({ 1, sceneSession });
    InsertDefaultPhysicalScreen();
    WindowPositionParams position;
    position.windowId = 1;
    position.insertAfter = static_cast<int32_t>(WindowPosition::NOT_TOPMOST);
    WindowPositionInfo windowPositionInfo;
    windowPositionInfo.windowPositions.push_back(position);
    // 2. Call the function to be tested
    WSError ret = ssm_->SetWindowPosition(windowPositionInfo);
    // 3. Verify the result
    EXPECT_EQ(ret, WSError::WS_OK);
    ssm_->sceneSessionMap_.clear();
    ClearScreenSessions();
}

} // namespace Rosen
} // namespace OHOS
