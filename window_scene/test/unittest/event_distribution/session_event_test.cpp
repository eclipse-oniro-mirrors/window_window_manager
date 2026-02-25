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

#include <gtest/gtest.h>
#include <regex>
#include <pointer_event.h>
#include <ui/rs_surface_node.h>

#include "session/host/include/extension_session.h"
#include "session/host/include/move_drag_controller.h"
#include "session/host/include/scene_session.h"
#include "session_manager/include/scene_session_manager.h"
#include "session/host/include/pc_fold_screen_manager.h"
#include "session/host/include/session.h"
#include "session_info.h"
#include "key_event.h"
#include "wm_common.h"
#include "window_manager_hilog.h"

using namespace testing;
using namespace testing::ext;

namespace OHOS {
namespace Rosen {
class SessionEventTest : public testing::Test {
public:
    static void SetUpTestCase();
    static void TearDownTestCase();
    void SetUp() override;
    void TearDown() override;
    sptr<Session> session_ = nullptr;
};

void SessionEventTest::SetUpTestCase() {}

void SessionEventTest::TearDownTestCase() {}

void SessionEventTest::SetUp()
{
    SessionInfo info;
    info.abilityName_ = "testSession1";
    info.moduleName_ = "testSession2";
    info.bundleName_ = "testSession3";
    session_ = sptr<Session>::MakeSptr(info);
}

void SessionEventTest::TearDown()
{
    session_ = nullptr;
}

namespace {
/**
 * @tc.name: SetTouchable01
 * @tc.desc: IsSessionValid() return false
 * @tc.type: FUNC
 */
HWTEST_F(SessionEventTest, SetTouchable01, TestSize.Level1)
{
    ASSERT_NE(session_, nullptr);
    session_->state_ = SessionState::STATE_DISCONNECT;
    session_->sessionInfo_.isSystem_ = true;
    EXPECT_EQ(WSError::WS_ERROR_INVALID_SESSION, session_->SetTouchable(false));
}

/**
 * @tc.name: SetTouchable02
 * @tc.desc: IsSessionValid() return true
 * @tc.type: FUNC
 */
HWTEST_F(SessionEventTest, SetTouchable02, TestSize.Level1)
{
    ASSERT_NE(session_, nullptr);
    session_->state_ = SessionState::STATE_FOREGROUND;
    session_->sessionInfo_.isSystem_ = false;
    EXPECT_EQ(WSError::WS_OK, session_->SetTouchable(false));
}

/**
 * @tc.name: SetTouchable03
 * @tc.desc: IsSessionValid() and touchable return true
 * @tc.type: FUNC
 */
HWTEST_F(SessionEventTest, SetTouchable03, TestSize.Level1)
{
    ASSERT_NE(session_, nullptr);
    session_->SetSessionState(SessionState::STATE_FOREGROUND);
    session_->sessionInfo_.isSystem_ = false;
    EXPECT_EQ(WSError::WS_OK, session_->SetTouchable(true));
}

/**
 * @tc.name: GetTouchable
 * @tc.desc: GetTouchable
 * @tc.type: FUNC
 */
HWTEST_F(SessionEventTest, GetTouchable, TestSize.Level1)
{
    ASSERT_NE(session_, nullptr);
    session_->SetTouchable(true);
    bool res = session_->GetTouchable();
    EXPECT_EQ(true, res);
}

/**
 * @tc.name: SetForceTouchable
 * @tc.desc: SetForceTouchable
 * @tc.type: FUNC
 */
HWTEST_F(SessionEventTest, SetForceTouchable, TestSize.Level1)
{
    ASSERT_NE(session_, nullptr);
    bool touchable = false;
    session_->SetForceTouchable(touchable);
    EXPECT_EQ(session_->forceTouchable_, touchable);
}

/**
 * @tc.name: SetSystemTouchable
 * @tc.desc: SetSystemTouchable
 * @tc.type: FUNC
 */
HWTEST_F(SessionEventTest, SetSystemTouchable, TestSize.Level1)
{
    ASSERT_NE(session_, nullptr);
    bool touchable = false;
    session_->SetSystemTouchable(touchable);
    EXPECT_EQ(session_->systemTouchable_, touchable);
}

/**
 * @tc.name: GetSystemTouchable
 * @tc.desc: GetSystemTouchable
 * @tc.type: FUNC
 */
HWTEST_F(SessionEventTest, GetSystemTouchable, TestSize.Level1)
{
    ASSERT_NE(session_, nullptr);
    session_->SetSystemTouchable(true);
    bool res = session_->GetSystemTouchable();
    EXPECT_EQ(res, true);
}

/**
 * @tc.name: GetWindowTouchableForMMITest001
 * @tc.desc: GetWindowTouchableForMMI Test
 * @tc.type: FUNC
 */
HWTEST_F(SessionEventTest, GetWindowTouchableForMMITest001, TestSize.Level1)
{
    SessionInfo sessionInfo;
    sessionInfo.isSystem_ = false;
    sessionInfo.bundleName_ = "event_test_bundleName";
    sessionInfo.abilityName_ = "event_test_abilityName";
    sessionInfo.windowType_ = static_cast<uint32_t>(WindowType::WINDOW_TYPE_APP_MAIN_WINDOW);
    sptr<SceneSession> sceneSession = sptr<SceneSession>::MakeSptr(sessionInfo, nullptr);
    auto screenId = 0;
    sceneSession->GetSessionProperty()->SetDisplayId(screenId);
    auto isTouchable = sceneSession->GetWindowTouchableForMMI(screenId);
    EXPECT_EQ(isTouchable, true);
}

/**
 * @tc.name: GetWindowTouchableForMMITest002
 * @tc.desc: GetWindowTouchableForMMI Test
 * @tc.type: FUNC
 */
HWTEST_F(SessionEventTest, GetWindowTouchableForMMITest002, TestSize.Level1)
{
    SessionInfo sessionInfo;
    sessionInfo.isSystem_ = false;
    sessionInfo.bundleName_ = "event_test_bundleName";
    sessionInfo.abilityName_ = "event_test_abilityName";
    sessionInfo.windowType_ = static_cast<uint32_t>(WindowType::WINDOW_TYPE_APP_MAIN_WINDOW);
    sptr<SceneSession> sceneSession = sptr<SceneSession>::MakeSptr(sessionInfo, nullptr);
    auto screenId = 0;
    ScreenSessionConfig config;
    sptr<ScreenSession> screenSession =
        sptr<ScreenSession>::MakeSptr(config, ScreenSessionReason::CREATE_SESSION_FOR_CLIENT);
    ASSERT_NE(screenSession, nullptr);
    sptr<DisplayInfo> displayInfo = new(std::nothrow) DisplayInfo();
    ASSERT_NE(displayInfo, nullptr);
    displayInfo->SetScreenId(screenId);
    displayInfo->SetDisplayId(screenId);
    sceneSession->GetSessionProperty()->SetDisplayId(screenId);

    screenSession->SetTouchEnabledFromJs(false);
    ScreenSessionManagerClient::GetInstance().screenSessionMap_.insert(std::make_pair(screenId, screenSession));
    auto isTouchable = sceneSession->GetWindowTouchableForMMI(screenId);
    EXPECT_EQ(isTouchable, false);
}

/**
 * @tc.name: GetWindowTouchableForMMITest003
 * @tc.desc: GetWindowTouchableForMMI Test
 * @tc.type: FUNC
 */
HWTEST_F(SessionEventTest, GetWindowTouchableForMMITest003, TestSize.Level1)
{
    SessionInfo sessionInfo;
    sessionInfo.isSystem_ = false;
    sessionInfo.bundleName_ = "event_test_bundleName";
    sessionInfo.abilityName_ = "event_test_abilityName";
    sessionInfo.windowType_ = static_cast<uint32_t>(WindowType::WINDOW_TYPE_APP_MAIN_WINDOW);
    sptr<SceneSession> sceneSession = sptr<SceneSession>::MakeSptr(sessionInfo, nullptr);
    auto screenId = 0;
    ScreenSessionConfig config;
    sptr<ScreenSession> screenSession =
        sptr<ScreenSession>::MakeSptr(config, ScreenSessionReason::CREATE_SESSION_FOR_CLIENT);
    ASSERT_NE(screenSession, nullptr);
    sptr<DisplayInfo> displayInfo = new(std::nothrow) DisplayInfo();
    ASSERT_NE(displayInfo, nullptr);
    displayInfo->SetScreenId(screenId);
    displayInfo->SetDisplayId(screenId);
    sceneSession->GetSessionProperty()->SetDisplayId(screenId);

    screenSession->SetTouchEnabledFromJs(true);
    sceneSession->SetSystemTouchable(false);
    ScreenSessionManagerClient::GetInstance().screenSessionMap_.clear();
    ScreenSessionManagerClient::GetInstance().screenSessionMap_.insert(std::make_pair(screenId, screenSession));
    auto isTouchable = sceneSession->GetWindowTouchableForMMI(screenId);
    EXPECT_EQ(isTouchable, false);
}

/**
 * @tc.name: GetWindowTouchableForMMITest004
 * @tc.desc: GetWindowTouchableForMMI Test
 * @tc.type: FUNC
 */
HWTEST_F(SessionEventTest, GetWindowTouchableForMMITest004, TestSize.Level1)
{
    SessionInfo sessionInfo;
    sessionInfo.isSystem_ = false;
    sessionInfo.bundleName_ = "event_test_bundleName";
    sessionInfo.abilityName_ = "event_test_abilityName";
    sessionInfo.windowType_ = static_cast<uint32_t>(WindowType::WINDOW_TYPE_APP_MAIN_WINDOW);
    sptr<SceneSession> sceneSession = sptr<SceneSession>::MakeSptr(sessionInfo, nullptr);
    auto screenId = 0;
    ScreenSessionConfig config;
    sptr<ScreenSession> screenSession =
        sptr<ScreenSession>::MakeSptr(config, ScreenSessionReason::CREATE_SESSION_FOR_CLIENT);
    ASSERT_NE(screenSession, nullptr);
    sptr<DisplayInfo> displayInfo = new(std::nothrow) DisplayInfo();
    ASSERT_NE(displayInfo, nullptr);
    displayInfo->SetScreenId(screenId);
    displayInfo->SetDisplayId(screenId);
    sceneSession->GetSessionProperty()->SetDisplayId(screenId);

    screenSession->SetTouchEnabledFromJs(true);
    sceneSession->SetSystemTouchable(true);
    sceneSession->SetForegroundInteractiveStatus(false);
    ScreenSessionManagerClient::GetInstance().screenSessionMap_.clear();
    ScreenSessionManagerClient::GetInstance().screenSessionMap_.insert(std::make_pair(screenId, screenSession));
    auto isTouchable = sceneSession->GetWindowTouchableForMMI(screenId);
    EXPECT_EQ(isTouchable, false);
}

/**
 * @tc.name: GetWindowTouchableForMMITest005
 * @tc.desc: GetWindowTouchableForMMI Test
 * @tc.type: FUNC
 */
HWTEST_F(SessionEventTest, GetWindowTouchableForMMITest005, TestSize.Level1)
{
    SessionInfo sessionInfo;
    sessionInfo.isSystem_ = false;
    sessionInfo.bundleName_ = "event_test_bundleName";
    sessionInfo.abilityName_ = "event_test_abilityName";
    sessionInfo.windowType_ = static_cast<uint32_t>(WindowType::WINDOW_TYPE_APP_MAIN_WINDOW);
    sptr<SceneSession> sceneSession = sptr<SceneSession>::MakeSptr(sessionInfo, nullptr);
    auto screenId = 0;
    ScreenSessionConfig config;
    sptr<ScreenSession> screenSession =
        sptr<ScreenSession>::MakeSptr(config, ScreenSessionReason::CREATE_SESSION_FOR_CLIENT);
    ASSERT_NE(screenSession, nullptr);
    sptr<DisplayInfo> displayInfo = new(std::nothrow) DisplayInfo();
    ASSERT_NE(displayInfo, nullptr);
    displayInfo->SetScreenId(screenId);
    displayInfo->SetDisplayId(screenId);
    sceneSession->GetSessionProperty()->SetDisplayId(screenId);

    screenSession->SetTouchEnabledFromJs(true);
    sceneSession->SetSystemTouchable(true);
    sceneSession->SetForegroundInteractiveStatus(true);
    ScreenSessionManagerClient::GetInstance().screenSessionMap_.clear();
    ScreenSessionManagerClient::GetInstance().screenSessionMap_.insert(std::make_pair(screenId, screenSession));
    auto isTouchable = sceneSession->GetWindowTouchableForMMI(screenId);
    EXPECT_EQ(isTouchable, true);
}

/**
 * @tc.name: GetWindowTouchableForMMITest006
 * @tc.desc: GetWindowTouchableForMMI Test
 * @tc.type: FUNC
 */
HWTEST_F(SessionEventTest, GetWindowTouchableForMMITest006, TestSize.Level1)
{
    SessionInfo sessionInfo;
    sessionInfo.isSystem_ = false;
    sessionInfo.bundleName_ = "event_test_bundleName";
    sessionInfo.abilityName_ = "event_test_abilityName";
    sessionInfo.windowType_ = static_cast<uint32_t>(WindowType::WINDOW_TYPE_APP_MAIN_WINDOW);
    sptr<SceneSession> sceneSession = sptr<SceneSession>::MakeSptr(sessionInfo, nullptr);
    auto screenId = 0;
    ScreenSessionConfig config;
    sptr<ScreenSession> screenSession =
        sptr<ScreenSession>::MakeSptr(config, ScreenSessionReason::CREATE_SESSION_FOR_CLIENT);
    ASSERT_NE(screenSession, nullptr);
    sptr<DisplayInfo> displayInfo = new(std::nothrow) DisplayInfo();
    ASSERT_NE(displayInfo, nullptr);
    displayInfo->SetScreenId(screenId);
    displayInfo->SetDisplayId(screenId);
    sceneSession->GetSessionProperty()->SetDisplayId(screenId);

    screenSession->SetTouchEnabledFromJs(false);
    sceneSession->SetSystemTouchable(false);
    sceneSession->SetForegroundInteractiveStatus(false);
    ScreenSessionManagerClient::GetInstance().screenSessionMap_.clear();
    ScreenSessionManagerClient::GetInstance().screenSessionMap_.insert(std::make_pair(screenId, screenSession));
    auto isTouchable = sceneSession->GetWindowTouchableForMMI(screenId);
    EXPECT_EQ(isTouchable, false);
}
}
}
}