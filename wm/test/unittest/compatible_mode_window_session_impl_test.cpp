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

#include "mock_session.h"
#include "mock_session_stub.h"
#include "mock_uicontent.h"
#include "mock_window.h"
#include "parameters.h"
#include "window_helper.h"
#include "window_scene_session_impl.h"
#include "wm_common.h"
#include "window_manager_hilog.h"

using namespace testing;
using namespace testing::ext;

namespace OHOS {
namespace Rosen {
std::string g_errLog;
void MyLogCallback(const LogType type, const LogLevel level, const unsigned int domain, const char *tag,
    const char *msg)
{
    g_errLog = msg;
}

class CompatibleModeWindowSessionImplTest : public testing::Test {
public:
    static void SetUpTestCase();
    static void TearDownTestCase();
    void SetUp() override;
    void TearDown() override;

private:
    static constexpr uint32_t WAIT_SYNC_IN_NS = 500000;
};

void CompatibleModeWindowSessionImplTest::SetUpTestCase()
{
    GTEST_LOG_(INFO) << "setup test case";
}

void CompatibleModeWindowSessionImplTest::TearDownTestCase()
{
    GTEST_LOG_(INFO) << "teardown test case";
}

void CompatibleModeWindowSessionImplTest::SetUp()
{
    GTEST_LOG_(INFO) << "setup";
}

void CompatibleModeWindowSessionImplTest::TearDown()
{
    GTEST_LOG_(INFO) << "teardown";
}

namespace {

/**
 * @tc.name: RegisterUIContentCreateListener
 * @tc.desc: RegisterUIContentCreateListenerWithNullListener
 * @tc.type: FUNC
 */
HWTEST_F(CompatibleModeWindowSessionImplTest, RegisterUIContentCreateListenerWithNullListener,
    TestSize.Level1)
{
    GTEST_LOG_(INFO) << "RegisterUIContentCreateListenerWithNullListener test start";
    sptr<WindowOption> option = sptr<WindowOption>::MakeSptr();
    sptr<WindowSessionImpl> window = sptr<WindowSessionImpl>::MakeSptr(option);
    sptr<IUIContentCreateListener> listener = nullptr;
    auto ret = window->RegisterUIContentCreateListener(listener);
    EXPECT_EQ(ret, WMError::WM_ERROR_NULLPTR);
    GTEST_LOG_(INFO) << "RegisterUIContentCreateListenerWithNullListener test end";
}

/**
 * @tc.name: RegisterUIContentCreateListener
 * @tc.desc: RegisterUIContentCreateListenerWithNotNullListener
 * @tc.type: FUNC
 */
HWTEST_F(CompatibleModeWindowSessionImplTest, RegisterUIContentCreateListenerWithNotNullListener,
    TestSize.Level1)
{
    GTEST_LOG_(INFO) << "RegisterUIContentCreateListenerWithNotNullListener test start";
    sptr<WindowOption> option = sptr<WindowOption>::MakeSptr();
    sptr<WindowSessionImpl> window = sptr<WindowSessionImpl>::MakeSptr(option);
    sptr<IUIContentCreateListener> listener = sptr<IUIContentCreateListener>::MakeSptr();
    auto ret = window->RegisterUIContentCreateListener(listener);
    EXPECT_EQ(ret, WMError::WM_OK);
    GTEST_LOG_(INFO) << "RegisterUIContentCreateListenerWithNotNullListener test end";
}

/**
 * @tc.name: UnregisterUIContentCreateListener
 * @tc.desc: UnregisterUIContentCreateListenerWithNullListener
 * @tc.type: FUNC
 */
HWTEST_F(CompatibleModeWindowSessionImplTest, UnregisterUIContentCreateListenerWithNullListener,
    TestSize.Level1)
{
    GTEST_LOG_(INFO) << "UnregisterUIContentCreateListenerWithNullListener test start";
    sptr<WindowOption> option = sptr<WindowOption>::MakeSptr();
    sptr<WindowSessionImpl> window = sptr<WindowSessionImpl>::MakeSptr(option);
    sptr<IUIContentCreateListener> listener = nullptr;
    auto ret = window->UnregisterUIContentCreateListener(listener);
    EXPECT_EQ(ret, WMError::WM_ERROR_NULLPTR);
    GTEST_LOG_(INFO) << "UnregisterUIContentCreateListenerWithNullListener test end";
}

/**
 * @tc.name: UnregisterUIContentCreateListener
 * @tc.desc: UnregisterUIContentCreateListenerWithNotNullListener
 * @tc.type: FUNC
 */
HWTEST_F(CompatibleModeWindowSessionImplTest, UnregisterUIContentCreateListenerWithNotNullListener,
    TestSize.Level1)
{
    GTEST_LOG_(INFO) << "UnregisterUIContentCreateListenerWithNotNullListener test start";
    sptr<WindowOption> option = sptr<WindowOption>::MakeSptr();
    sptr<WindowSessionImpl> window = sptr<WindowSessionImpl>::MakeSptr(option);
    sptr<IUIContentCreateListener> listener = sptr<IUIContentCreateListener>::MakeSptr();
    auto ret = window->UnregisterUIContentCreateListener(listener);
    EXPECT_EQ(ret, WMError::WM_OK);
    GTEST_LOG_(INFO) << "UnregisterUIContentCreateListenerWithNotNullListener test end";
}

/**
 * @tc.name: UpdateCompatibleStyleMode
 * @tc.desc: UpdateCompatibleStyleModeWithNullSession
 * @tc.type: FUNC
 */
HWTEST_F(CompatibleModeWindowSessionImplTest, UpdateCompatibleStyleModeWithNullSession,
    TestSize.Level1)
{
    GTEST_LOG_(INFO) << "UpdateCompatibleStyleModeWithNullSession test start";
    sptr<WindowOption> option = sptr<WindowOption>::MakeSptr();
    sptr<WindowSessionImpl> window = sptr<WindowSessionImpl>::MakeSptr(option);
    window->uiContent_ = std::make_unique<Ace::UIContentMocker>();
    auto ret = window->UpdateCompatibleStyleMode(CompatibleStyleMode::LANDSCAPE_18_9);
    EXPECT_EQ(ret, WMError::WM_ERROR_NULLPTR);
    GTEST_LOG_(INFO) << "UpdateCompatibleStyleModeWithNullSession test end";
}

/**
 * @tc.name: UpdateCompatibleStyleMode
 * @tc.desc: UpdateCompatibleStyleMode
 * @tc.type: FUNC
 */
HWTEST_F(CompatibleModeWindowSessionImplTest, UpdateCompatibleStyleMode, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "UpdateCompatibleStyleMode test start";
    sptr<WindowOption> option = sptr<WindowOption>::MakeSptr();
    sptr<WindowSessionImpl> window = sptr<WindowSessionImpl>::MakeSptr(option);
    window->uiContent_ = std::make_unique<Ace::UIContentMocker>();
    SessionInfo sessionInfo;
    window->hostSession_ = sptr<SessionMocker>::MakeSptr(sessionInfo);
    auto ret = window->UpdateCompatibleStyleMode(CompatibleStyleMode::LANDSCAPE_18_9);
    EXPECT_EQ(ret, WMError::WM_OK);
    GTEST_LOG_(INFO) << "UpdateCompatibleStyleMode test end";
}

/**
 * @tc.name: SetAppHookWindowInfo
 * @tc.desc: SetAppHookWindowInfo
 * @tc.type: FUNC
 */
HWTEST_F(CompatibleModeWindowSessionImplTest, SetAppHookWindowInfo, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "CompatibleModeWindowSessionImplTest: SetAppHookWindowInfo start";
    sptr<WindowOption> option = sptr<WindowOption>::MakeSptr();
    sptr<WindowSessionImpl> window = sptr<WindowSessionImpl>::MakeSptr(option);

    // Case 1: SetAppHookWindowInfo
    HookWindowInfo hookWindowInfo;
    hookWindowInfo.enableHookWindow = false;
    hookWindowInfo.widthHookRatio = 1.0f;
    hookWindowInfo.notifyWindowChange = false;
    window->SetAppHookWindowInfo(hookWindowInfo);
    const uint32_t defaultSize = 800;
    Rect rect = { 0, 0, defaultSize, defaultSize };
    window->HookWindowSizeByHookWindowInfo(rect);
    EXPECT_EQ(rect.width_, defaultSize);

    // Case 2: SetAppHookWindowInfo with notifyWindowChange
    hookWindowInfo.notifyWindowChange = true;
    window->SetAppHookWindowInfo(hookWindowInfo);
    window->HookWindowSizeByHookWindowInfo(rect);
    EXPECT_EQ(rect.width_, defaultSize);
    GTEST_LOG_(INFO) << "CompatibleModeWindowSessionImplTest: SetAppHookWindowInfo end";
}

/**
 * @tc.name: UpdateAppHookWindowInfo
 * @tc.desc: UpdateAppHookWindowInfo
 * @tc.type: FUNC
 */
HWTEST_F(CompatibleModeWindowSessionImplTest, UpdateAppHookWindowInfo, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "CompatibleModeWindowSessionImplTest: UpdateAppHookWindowInfo start";
    sptr<WindowOption> option = sptr<WindowOption>::MakeSptr();
    option->SetWindowName("UpdateAppHookWindowInfo");
    option->SetWindowType(WindowType::WINDOW_TYPE_APP_MAIN_WINDOW);
    sptr<WindowSceneSessionImpl> window = sptr<WindowSceneSessionImpl>::MakeSptr(option);
    const int32_t windowId = 2025;
    window->property_->SetPersistentId(windowId);
    HookWindowInfo hookWindowInfo;
    hookWindowInfo.enableHookWindow = false;
    hookWindowInfo.widthHookRatio = 1.0f;
    hookWindowInfo.notifyWindowChange = false;

    // Case 1: success
    SessionInfo sessionInfo = { "CreateTestBundle", "CreateTestModule", "CreateTestAbility" };
    sptr<SessionMocker> session = sptr<SessionMocker>::MakeSptr(sessionInfo);
    window->hostSession_ = session;
    EXPECT_CALL(*session, Disconnect(_, _, _)).WillOnce(Return(WSError::WS_OK));
    auto res = window->UpdateAppHookWindowInfo(hookWindowInfo);
    EXPECT_EQ(res, WSError::WS_OK);

    // Case 2: not mainWindow
    window->property_->SetWindowType(WindowType::WINDOW_TYPE_APP_SUB_WINDOW);
    res = window->UpdateAppHookWindowInfo(hookWindowInfo);
    EXPECT_EQ(res, WSError::WS_DO_NOTHING);
    GTEST_LOG_(INFO) << "CompatibleModeWindowSessionImplTest: UpdateAppHookWindowInfo end";
}

/**
 * @tc.name: NotifySplitRatioChanged01
 * @tc.desc: NotifySplitRatioChanged with hookWindow disabled
 * @tc.type: FUNC
 */
HWTEST_F(CompatibleModeWindowSessionImplTest, NotifySplitRatioChanged01, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "CompatibleModeWindowSessionImplTest: NotifySplitRatioChanged01 start";
    sptr<WindowOption> option = sptr<WindowOption>::MakeSptr();
    option->SetWindowName("NotifySplitRatioChanged01");
    option->SetWindowType(WindowType::WINDOW_TYPE_APP_MAIN_WINDOW);
    sptr<WindowSceneSessionImpl> window = sptr<WindowSceneSessionImpl>::MakeSptr(option);
    window->property_->SetPersistentId(1);

    HookWindowInfo hookWindowInfo;
    hookWindowInfo.enableHookWindow = false;
    hookWindowInfo.widthHookRatio = 1.0f;
    window->property_->SetHookWindowInfo(hookWindowInfo);

    auto ret = window->NotifySplitRatioChanged(1.0f / 3.0f);
    EXPECT_EQ(ret, WMError::WM_DO_NOTHING);
    GTEST_LOG_(INFO) << "CompatibleModeWindowSessionImplTest: NotifySplitRatioChanged01 end";
}

/**
 * @tc.name: NotifySplitRatioChanged02
 * @tc.desc: NotifySplitRatioChanged with non-main window type
 * @tc.type: FUNC
 */
HWTEST_F(CompatibleModeWindowSessionImplTest, NotifySplitRatioChanged02, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "CompatibleModeWindowSessionImplTest: NotifySplitRatioChanged02 start";
    sptr<WindowOption> option = sptr<WindowOption>::MakeSptr();
    option->SetWindowName("NotifySplitRatioChanged02");
    option->SetWindowType(WindowType::WINDOW_TYPE_APP_SUB_WINDOW);
    sptr<WindowSceneSessionImpl> window = sptr<WindowSceneSessionImpl>::MakeSptr(option);
    window->property_->SetPersistentId(2);

    HookWindowInfo hookWindowInfo;
    hookWindowInfo.enableHookWindow = true;
    hookWindowInfo.widthHookRatio = 1.0f;
    window->property_->SetHookWindowInfo(hookWindowInfo);

    auto ret = window->NotifySplitRatioChanged(1.0f / 3.0f);
    EXPECT_EQ(ret, WMError::WM_DO_NOTHING);
    GTEST_LOG_(INFO) << "CompatibleModeWindowSessionImplTest: NotifySplitRatioChanged02 end";
}

/**
 * @tc.name: NotifySplitRatioChanged03
 * @tc.desc: NotifySplitRatioChanged with invalid ratio
 * @tc.type: FUNC
 */
HWTEST_F(CompatibleModeWindowSessionImplTest, NotifySplitRatioChanged03, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "CompatibleModeWindowSessionImplTest: NotifySplitRatioChanged03 start";
    sptr<WindowOption> option = sptr<WindowOption>::MakeSptr();
    option->SetWindowName("NotifySplitRatioChanged03");
    option->SetWindowType(WindowType::WINDOW_TYPE_APP_MAIN_WINDOW);
    sptr<WindowSceneSessionImpl> window = sptr<WindowSceneSessionImpl>::MakeSptr(option);
    window->property_->SetPersistentId(3);

    HookWindowInfo hookWindowInfo;
    hookWindowInfo.enableHookWindow = true;
    hookWindowInfo.widthHookRatio = 1.0f;
    window->property_->SetHookWindowInfo(hookWindowInfo);

    auto ret = window->NotifySplitRatioChanged(0.1f);
    EXPECT_EQ(ret, WMError::WM_ERROR_INVALID_PARAM);

    ret = window->NotifySplitRatioChanged(0.25f);
    EXPECT_EQ(ret, WMError::WM_ERROR_INVALID_PARAM);

    ret = window->NotifySplitRatioChanged(0.4f);
    EXPECT_EQ(ret, WMError::WM_ERROR_INVALID_PARAM);

    ret = window->NotifySplitRatioChanged(0.75f);
    EXPECT_EQ(ret, WMError::WM_ERROR_INVALID_PARAM);

    ret = window->NotifySplitRatioChanged(0.9f);
    EXPECT_EQ(ret, WMError::WM_ERROR_INVALID_PARAM);
    GTEST_LOG_(INFO) << "CompatibleModeWindowSessionImplTest: NotifySplitRatioChanged03 end";
}

/**
 * @tc.name: NotifySplitRatioChanged04
 * @tc.desc: NotifySplitRatioChanged with null hostSession
 * @tc.type: FUNC
 */
HWTEST_F(CompatibleModeWindowSessionImplTest, NotifySplitRatioChanged04, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "CompatibleModeWindowSessionImplTest: NotifySplitRatioChanged04 start";
    sptr<WindowOption> option = sptr<WindowOption>::MakeSptr();
    option->SetWindowName("NotifySplitRatioChanged04");
    option->SetWindowType(WindowType::WINDOW_TYPE_APP_MAIN_WINDOW);
    sptr<WindowSceneSessionImpl> window = sptr<WindowSceneSessionImpl>::MakeSptr(option);
    window->property_->SetPersistentId(4);
    window->hostSession_ = nullptr;

    HookWindowInfo hookWindowInfo;
    hookWindowInfo.enableHookWindow = true;
    hookWindowInfo.widthHookRatio = 1.0f;
    window->property_->SetHookWindowInfo(hookWindowInfo);

    auto ret = window->NotifySplitRatioChanged(1.0f / 3.0f);
    EXPECT_EQ(ret, WMError::WM_ERROR_NULLPTR);
    GTEST_LOG_(INFO) << "CompatibleModeWindowSessionImplTest: NotifySplitRatioChanged04 end";
}

/**
 * @tc.name: NotifySplitRatioChanged05
 * @tc.desc: NotifySplitRatioChanged with valid params and SMALL_SPLIT_RATIO
 * @tc.type: FUNC
 */
HWTEST_F(CompatibleModeWindowSessionImplTest, NotifySplitRatioChanged05, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "CompatibleModeWindowSessionImplTest: NotifySplitRatioChanged05 start";
    sptr<WindowOption> option = sptr<WindowOption>::MakeSptr();
    option->SetWindowName("NotifySplitRatioChanged05");
    option->SetWindowType(WindowType::WINDOW_TYPE_APP_MAIN_WINDOW);
    sptr<WindowSceneSessionImpl> window = sptr<WindowSceneSessionImpl>::MakeSptr(option);
    window->property_->SetPersistentId(5);

    HookWindowInfo hookWindowInfo;
    hookWindowInfo.enableHookWindow = true;
    hookWindowInfo.widthHookRatio = 1.0f;
    window->property_->SetHookWindowInfo(hookWindowInfo);

    SessionInfo sessionInfo = { "CreateTestBundle", "CreateTestModule", "CreateTestAbility" };
    sptr<SessionMocker> session = sptr<SessionMocker>::MakeSptr(sessionInfo);
    window->hostSession_ = session;

    EXPECT_CALL(*session, NotifySplitRatioChanged(1.0f / 3.0f)).WillOnce(Return(WMError::WM_OK));
    EXPECT_CALL(*session, Disconnect(_, _, _)).WillOnce(Return(WSError::WS_OK));
    auto ret = window->NotifySplitRatioChanged(1.0f / 3.0f);
    EXPECT_EQ(ret, WMError::WM_OK);

    auto updatedHookInfo = window->property_->GetHookWindowInfo();
    EXPECT_FLOAT_EQ(updatedHookInfo.widthHookRatio, 1.0f / 3.0f);
    GTEST_LOG_(INFO) << "CompatibleModeWindowSessionImplTest: NotifySplitRatioChanged05 end";
}

/**
 * @tc.name: NotifySplitRatioChanged06
 * @tc.desc: NotifySplitRatioChanged with valid params and MIDDLE_SPLIT_RATIO
 * @tc.type: FUNC
 */
HWTEST_F(CompatibleModeWindowSessionImplTest, NotifySplitRatioChanged06, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "CompatibleModeWindowSessionImplTest: NotifySplitRatioChanged06 start";
    sptr<WindowOption> option = sptr<WindowOption>::MakeSptr();
    option->SetWindowName("NotifySplitRatioChanged06");
    option->SetWindowType(WindowType::WINDOW_TYPE_APP_MAIN_WINDOW);
    sptr<WindowSceneSessionImpl> window = sptr<WindowSceneSessionImpl>::MakeSptr(option);
    window->property_->SetPersistentId(6);

    HookWindowInfo hookWindowInfo;
    hookWindowInfo.enableHookWindow = true;
    hookWindowInfo.widthHookRatio = 1.0f;
    window->property_->SetHookWindowInfo(hookWindowInfo);

    SessionInfo sessionInfo = { "CreateTestBundle", "CreateTestModule", "CreateTestAbility" };
    sptr<SessionMocker> session = sptr<SessionMocker>::MakeSptr(sessionInfo);
    window->hostSession_ = session;

    EXPECT_CALL(*session, NotifySplitRatioChanged(1.0f / 2.0f)).WillOnce(Return(WMError::WM_OK));
    EXPECT_CALL(*session, Disconnect(_, _, _)).WillOnce(Return(WSError::WS_OK));
    auto ret = window->NotifySplitRatioChanged(1.0f / 2.0f);
    EXPECT_EQ(ret, WMError::WM_OK);

    auto updatedHookInfo = window->property_->GetHookWindowInfo();
    EXPECT_FLOAT_EQ(updatedHookInfo.widthHookRatio, 1.0f / 2.0f);
    GTEST_LOG_(INFO) << "CompatibleModeWindowSessionImplTest: NotifySplitRatioChanged06 end";
}

/**
 * @tc.name: NotifySplitRatioChanged07
 * @tc.desc: NotifySplitRatioChanged with valid params and LARGE_SPLIT_RATIO
 * @tc.type: FUNC
 */
HWTEST_F(CompatibleModeWindowSessionImplTest, NotifySplitRatioChanged07, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "CompatibleModeWindowSessionImplTest: NotifySplitRatioChanged07 start";
    sptr<WindowOption> option = sptr<WindowOption>::MakeSptr();
    option->SetWindowName("NotifySplitRatioChanged07");
    option->SetWindowType(WindowType::WINDOW_TYPE_APP_MAIN_WINDOW);
    sptr<WindowSceneSessionImpl> window = sptr<WindowSceneSessionImpl>::MakeSptr(option);
    window->property_->SetPersistentId(7);

    HookWindowInfo hookWindowInfo;
    hookWindowInfo.enableHookWindow = true;
    hookWindowInfo.widthHookRatio = 1.0f;
    window->property_->SetHookWindowInfo(hookWindowInfo);

    SessionInfo sessionInfo = { "CreateTestBundle", "CreateTestModule", "CreateTestAbility" };
    sptr<SessionMocker> session = sptr<SessionMocker>::MakeSptr(sessionInfo);
    window->hostSession_ = session;

    EXPECT_CALL(*session, NotifySplitRatioChanged(2.0f / 3.0f)).WillOnce(Return(WMError::WM_OK));
    EXPECT_CALL(*session, Disconnect(_, _, _)).WillOnce(Return(WSError::WS_OK));
    auto ret = window->NotifySplitRatioChanged(2.0f / 3.0f);
    EXPECT_EQ(ret, WMError::WM_OK);

    auto updatedHookInfo = window->property_->GetHookWindowInfo();
    EXPECT_FLOAT_EQ(updatedHookInfo.widthHookRatio, 2.0f / 3.0f);
    GTEST_LOG_(INFO) << "CompatibleModeWindowSessionImplTest: NotifySplitRatioChanged07 end";
}

/**
 * @tc.name: NotifySplitRatioChanged08
 * @tc.desc: NotifySplitRatioChanged when hostSession returns error
 * @tc.type: FUNC
 */
HWTEST_F(CompatibleModeWindowSessionImplTest, NotifySplitRatioChanged08, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "CompatibleModeWindowSessionImplTest: NotifySplitRatioChanged08 start";
    sptr<WindowOption> option = sptr<WindowOption>::MakeSptr();
    option->SetWindowName("NotifySplitRatioChanged08");
    option->SetWindowType(WindowType::WINDOW_TYPE_APP_MAIN_WINDOW);
    sptr<WindowSceneSessionImpl> window = sptr<WindowSceneSessionImpl>::MakeSptr(option);
    window->property_->SetPersistentId(8);

    HookWindowInfo hookWindowInfo;
    hookWindowInfo.enableHookWindow = true;
    hookWindowInfo.widthHookRatio = 1.0f;
    window->property_->SetHookWindowInfo(hookWindowInfo);

    SessionInfo sessionInfo = { "CreateTestBundle", "CreateTestModule", "CreateTestAbility" };
    sptr<SessionMocker> session = sptr<SessionMocker>::MakeSptr(sessionInfo);
    window->hostSession_ = session;

    EXPECT_CALL(*session, NotifySplitRatioChanged(1.0f / 2.0f)).WillOnce(Return(WMError::WM_ERROR_INVALID_OPERATION));
    EXPECT_CALL(*session, Disconnect(_, _, _)).WillOnce(Return(WSError::WS_OK));
    auto ret = window->NotifySplitRatioChanged(1.0f / 2.0f);
    EXPECT_EQ(ret, WMError::WM_ERROR_INVALID_OPERATION);

    auto hookInfo = window->property_->GetHookWindowInfo();
    EXPECT_FLOAT_EQ(hookInfo.widthHookRatio, 1.0f);
    GTEST_LOG_(INFO) << "CompatibleModeWindowSessionImplTest: NotifySplitRatioChanged08 end";
}

/**
 * @tc.name: CompatibleStyleModeValueTest
 * @tc.desc: Verify LANDSCAPE_SCALE_VERTICAL_FULL enum value equals 23
 * @tc.type: FUNC
 */
HWTEST_F(CompatibleModeWindowSessionImplTest, CompatibleStyleModeValueTest, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "CompatibleStyleModeValueTest test start";
    EXPECT_EQ(static_cast<uint32_t>(CompatibleStyleMode::LANDSCAPE_SCALE_VERTICAL_FULL), 23u);
    GTEST_LOG_(INFO) << "CompatibleStyleModeValueTest test end";
}

/**
 * @tc.name: UpdateCompatibleStyleModeWithVerticalFullScreenNullSession
 * @tc.desc: UpdateCompatibleStyleMode with LANDSCAPE_SCALE_VERTICAL_FULL and null session
 * @tc.type: FUNC
 */
HWTEST_F(CompatibleModeWindowSessionImplTest, UpdateCompatibleStyleModeWithVerticalFullScreenNullSession,
    TestSize.Level1)
{
    GTEST_LOG_(INFO) << "UpdateCompatibleStyleModeWithVerticalFullScreenNullSession test start";
    sptr<WindowOption> option = sptr<WindowOption>::MakeSptr();
    sptr<WindowSessionImpl> window = sptr<WindowSessionImpl>::MakeSptr(option);
    window->uiContent_ = std::make_unique<Ace::UIContentMocker>();
    auto ret = window->UpdateCompatibleStyleMode(CompatibleStyleMode::LANDSCAPE_SCALE_VERTICAL_FULL);
    EXPECT_EQ(ret, WMError::WM_ERROR_NULLPTR);
    GTEST_LOG_(INFO) << "UpdateCompatibleStyleModeWithVerticalFullScreenNullSession test end";
}

/**
 * @tc.name: UpdateCompatibleStyleModeVerticalFullScreen
 * @tc.desc: UpdateCompatibleStyleMode with LANDSCAPE_SCALE_VERTICAL_FULL successfully
 * @tc.type: FUNC
 */
HWTEST_F(CompatibleModeWindowSessionImplTest, UpdateCompatibleStyleModeVerticalFullScreen, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "UpdateCompatibleStyleModeVerticalFullScreen test start";
    sptr<WindowOption> option = sptr<WindowOption>::MakeSptr();
    sptr<WindowSessionImpl> window = sptr<WindowSessionImpl>::MakeSptr(option);
    window->uiContent_ = std::make_unique<Ace::UIContentMocker>();
    SessionInfo sessionInfo;
    window->hostSession_ = sptr<SessionMocker>::MakeSptr(sessionInfo);
    auto ret = window->UpdateCompatibleStyleMode(CompatibleStyleMode::LANDSCAPE_SCALE_VERTICAL_FULL);
    EXPECT_EQ(ret, WMError::WM_OK);
    GTEST_LOG_(INFO) << "UpdateCompatibleStyleModeVerticalFullScreen test end";
}

/**
 * @tc.name: SwitchCompatibleModeVerticalFullScreen
 * @tc.desc: SwitchCompatibleMode with LANDSCAPE_SCALE_VERTICAL_FULL on main window
 * @tc.type: FUNC
 */
HWTEST_F(CompatibleModeWindowSessionImplTest, SwitchCompatibleModeVerticalFullScreen, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "SwitchCompatibleModeVerticalFullScreen test start";
    sptr<WindowOption> option = sptr<WindowOption>::MakeSptr();
    option->SetWindowName("SwitchCompatibleModeVerticalFullScreen");
    option->SetWindowType(WindowType::WINDOW_TYPE_APP_MAIN_WINDOW);
    sptr<WindowSceneSessionImpl> window = sptr<WindowSceneSessionImpl>::MakeSptr(option);
    window->property_->SetPersistentId(100);
    SessionInfo sessionInfo;
    sptr<SessionMocker> session = sptr<SessionMocker>::MakeSptr(sessionInfo);
    window->hostSession_ = session;
    EXPECT_CALL(*session, OnSessionEvent(SessionEvent::EVENT_SWITCH_COMPATIBLE_MODE, _))
        .WillOnce(Return(WSError::WS_OK));
    EXPECT_CALL(*session, Disconnect(_, _, _)).WillOnce(Return(WSError::WS_OK));
    auto ret = window->SwitchCompatibleMode(CompatibleStyleMode::LANDSCAPE_SCALE_VERTICAL_FULL);
    EXPECT_EQ(ret, WMError::WM_OK);
    GTEST_LOG_(INFO) << "SwitchCompatibleModeVerticalFullScreen test end";
}

/**
 * @tc.name: SwitchCompatibleModeVerticalFullScreenNotMain
 * @tc.desc: SwitchCompatibleMode with LANDSCAPE_SCALE_VERTICAL_FULL on non-main window
 * @tc.type: FUNC
 */
HWTEST_F(CompatibleModeWindowSessionImplTest, SwitchCompatibleModeVerticalFullScreenNotMain, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "SwitchCompatibleModeVerticalFullScreenNotMain test start";
    sptr<WindowOption> option = sptr<WindowOption>::MakeSptr();
    option->SetWindowName("SwitchCompatibleModeVerticalFullScreenNotMain");
    option->SetWindowType(WindowType::WINDOW_TYPE_APP_SUB_WINDOW);
    sptr<WindowSceneSessionImpl> window = sptr<WindowSceneSessionImpl>::MakeSptr(option);
    window->property_->SetPersistentId(101);
    SessionInfo sessionInfo;
    sptr<SessionMocker> session = sptr<SessionMocker>::MakeSptr(sessionInfo);
    window->hostSession_ = session;
    EXPECT_CALL(*session, Disconnect(_, _, _)).WillOnce(Return(WSError::WS_OK));
    auto ret = window->SwitchCompatibleMode(CompatibleStyleMode::LANDSCAPE_SCALE_VERTICAL_FULL);
    EXPECT_EQ(ret, WMError::WM_ERROR_INVALID_CALLING);
    GTEST_LOG_(INFO) << "SwitchCompatibleModeVerticalFullScreenNotMain test end";
}

/**
 * @tc.name: PageCompatibleModeVerticalFullScreen
 * @tc.desc: SetPageCompatibleMode/GetPageCompatibleMode round-trip with LANDSCAPE_SCALE_VERTICAL_FULL
 * @tc.type: FUNC
 */
HWTEST_F(CompatibleModeWindowSessionImplTest, PageCompatibleModeVerticalFullScreen, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "PageCompatibleModeVerticalFullScreen test start";
    sptr<WindowOption> option = sptr<WindowOption>::MakeSptr();
    sptr<WindowSessionImpl> window = sptr<WindowSessionImpl>::MakeSptr(option);
    window->property_->SetPageCompatibleMode(CompatibleStyleMode::LANDSCAPE_SCALE_VERTICAL_FULL);
    auto mode = window->property_->GetPageCompatibleMode();
    EXPECT_EQ(mode, CompatibleStyleMode::LANDSCAPE_SCALE_VERTICAL_FULL);
    GTEST_LOG_(INFO) << "PageCompatibleModeVerticalFullScreen test end";
}

/**
 * @tc.name: SwitchCompatibleModeVerticalFullScreenInvalidSession
 * @tc.desc: SwitchCompatibleMode with LANDSCAPE_SCALE_VERTICAL_FULL on invalid session
 * @tc.type: FUNC
 */
HWTEST_F(CompatibleModeWindowSessionImplTest, SwitchCompatibleModeVerticalFullScreenInvalidSession,
    TestSize.Level1)
{
    GTEST_LOG_(INFO) << "SwitchCompatibleModeVerticalFullScreenInvalidSession test start";
    sptr<WindowOption> option = sptr<WindowOption>::MakeSptr();
    option->SetWindowName("SwitchCompatibleModeVerticalFullScreenInvalidSession");
    sptr<WindowSceneSessionImpl> window = sptr<WindowSceneSessionImpl>::MakeSptr(option);
    // persistentId remains INVALID_SESSION_ID, state_ is not STATE_SHOWN -> IsWindowSessionInvalid returns true
    auto ret = window->SwitchCompatibleMode(CompatibleStyleMode::LANDSCAPE_SCALE_VERTICAL_FULL);
    EXPECT_EQ(ret, WMError::WM_ERROR_INVALID_WINDOW);
    GTEST_LOG_(INFO) << "SwitchCompatibleModeVerticalFullScreenInvalidSession test end";
}

/**
 * @tc.name: SwitchCompatibleModeVerticalFullScreenNullHostSession
 * @tc.desc: SwitchCompatibleMode with LANDSCAPE_SCALE_VERTICAL_FULL when hostSession is null,
 *           IsWindowSessionInvalid catches it first and returns WM_ERROR_INVALID_WINDOW
 * @tc.type: FUNC
 */
HWTEST_F(CompatibleModeWindowSessionImplTest, SwitchCompatibleModeVerticalFullScreenNullHostSession,
    TestSize.Level1)
{
    GTEST_LOG_(INFO) << "SwitchCompatibleModeVerticalFullScreenNullHostSession test start";
    sptr<WindowOption> option = sptr<WindowOption>::MakeSptr();
    option->SetWindowName("SwitchCompatibleModeVerticalFullScreenNullHostSession");
    option->SetWindowType(WindowType::WINDOW_TYPE_APP_MAIN_WINDOW);
    sptr<WindowSceneSessionImpl> window = sptr<WindowSceneSessionImpl>::MakeSptr(option);
    window->property_->SetPersistentId(102);
    // hostSession_ is nullptr -> IsWindowSessionInvalid() returns true first
    auto ret = window->SwitchCompatibleMode(CompatibleStyleMode::LANDSCAPE_SCALE_VERTICAL_FULL);
    EXPECT_EQ(ret, WMError::WM_ERROR_INVALID_WINDOW);
    GTEST_LOG_(INFO) << "SwitchCompatibleModeVerticalFullScreenNullHostSession test end";
}

/**
 * @tc.name: RecoverForCompatibleModeVerticalFullScreen
 * @tc.desc: RecoverForCompatibleMode on main window with LANDSCAPE_SCALE_VERTICAL_FULL
 * @tc.type: FUNC
 */
HWTEST_F(CompatibleModeWindowSessionImplTest, RecoverForCompatibleModeVerticalFullScreen, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "RecoverForCompatibleModeVerticalFullScreen test start";
    sptr<WindowOption> option = sptr<WindowOption>::MakeSptr();
    option->SetWindowName("RecoverForCompatibleModeVerticalFullScreen");
    option->SetWindowType(WindowType::WINDOW_TYPE_APP_MAIN_WINDOW);
    sptr<WindowSceneSessionImpl> window = sptr<WindowSceneSessionImpl>::MakeSptr(option);
    window->property_->SetPersistentId(103);
    SessionInfo sessionInfo;
    sptr<SessionMocker> session = sptr<SessionMocker>::MakeSptr(sessionInfo);
    window->hostSession_ = session;
    EXPECT_CALL(*session, OnSessionEvent(SessionEvent::EVENT_COMPATIBLE_TO_RECOVER, _))
        .WillOnce(Return(WSError::WS_OK));
    EXPECT_CALL(*session, Disconnect(_, _, _)).WillOnce(Return(WSError::WS_OK));
    auto ret = window->RecoverForCompatibleMode();
    EXPECT_EQ(ret, WMError::WM_OK);
    GTEST_LOG_(INFO) << "RecoverForCompatibleModeVerticalFullScreen test end";
}

/**
 * @tc.name: RecoverForCompatibleModeNotMain
 * @tc.desc: RecoverForCompatibleMode on non-main window
 * @tc.type: FUNC
 */
HWTEST_F(CompatibleModeWindowSessionImplTest, RecoverForCompatibleModeNotMain, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "RecoverForCompatibleModeNotMain test start";
    sptr<WindowOption> option = sptr<WindowOption>::MakeSptr();
    option->SetWindowName("RecoverForCompatibleModeNotMain");
    option->SetWindowType(WindowType::WINDOW_TYPE_APP_SUB_WINDOW);
    sptr<WindowSceneSessionImpl> window = sptr<WindowSceneSessionImpl>::MakeSptr(option);
    window->property_->SetPersistentId(104);
    SessionInfo sessionInfo;
    sptr<SessionMocker> session = sptr<SessionMocker>::MakeSptr(sessionInfo);
    window->hostSession_ = session;
    EXPECT_CALL(*session, Disconnect(_, _, _)).WillOnce(Return(WSError::WS_OK));
    auto ret = window->RecoverForCompatibleMode();
    EXPECT_EQ(ret, WMError::WM_ERROR_INVALID_CALLING);
    GTEST_LOG_(INFO) << "RecoverForCompatibleModeNotMain test end";
}

/**
 * @tc.name: UpdateCompatibleStyleModeVerticalFullNotifyCallback
 * @tc.desc: UpdateCompatibleStyleMode with LANDSCAPE_SCALE_VERTICAL_FULL verifies SetPageCompatibleMode
 * @tc.type: FUNC
 */
HWTEST_F(CompatibleModeWindowSessionImplTest, UpdateCompatibleStyleModeVerticalFullNotifyCallback,
    TestSize.Level1)
{
    GTEST_LOG_(INFO) << "UpdateCompatibleStyleModeVerticalFullNotifyCallback test start";
    sptr<WindowOption> option = sptr<WindowOption>::MakeSptr();
    sptr<WindowSessionImpl> window = sptr<WindowSessionImpl>::MakeSptr(option);
    window->uiContent_ = std::make_unique<Ace::UIContentMocker>();
    SessionInfo sessionInfo;
    window->hostSession_ = sptr<SessionMocker>::MakeSptr(sessionInfo);
    auto ret = window->UpdateCompatibleStyleMode(CompatibleStyleMode::LANDSCAPE_SCALE_VERTICAL_FULL);
    EXPECT_EQ(ret, WMError::WM_OK);
    EXPECT_EQ(window->property_->GetPageCompatibleMode(), CompatibleStyleMode::LANDSCAPE_SCALE_VERTICAL_FULL);
    GTEST_LOG_(INFO) << "UpdateCompatibleStyleModeVerticalFullNotifyCallback test end";
}

/**
 * @tc.name: PageCompatibleModeDefaultIsInvalidValue
 * @tc.desc: Verify default PageCompatibleMode is INVALID_VALUE
 * @tc.type: FUNC
 */
HWTEST_F(CompatibleModeWindowSessionImplTest, PageCompatibleModeDefaultIsInvalidValue, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "PageCompatibleModeDefaultIsInvalidValue test start";
    sptr<WindowOption> option = sptr<WindowOption>::MakeSptr();
    sptr<WindowSessionImpl> window = sptr<WindowSessionImpl>::MakeSptr(option);
    auto mode = window->property_->GetPageCompatibleMode();
    EXPECT_EQ(mode, CompatibleStyleMode::INVALID_VALUE);
    GTEST_LOG_(INFO) << "PageCompatibleModeDefaultIsInvalidValue test end";
}

/**
 * @tc.name: PageCompatibleModeAllEnumValuesRoundTrip
 * @tc.desc: Verify SetPageCompatibleMode/GetPageCompatibleMode round-trip for all enum values
 * @tc.type: FUNC
 */
HWTEST_F(CompatibleModeWindowSessionImplTest, PageCompatibleModeAllEnumValuesRoundTrip, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "PageCompatibleModeAllEnumValuesRoundTrip test start";
    sptr<WindowOption> option = sptr<WindowOption>::MakeSptr();
    sptr<WindowSessionImpl> window = sptr<WindowSessionImpl>::MakeSptr(option);
    std::vector<CompatibleStyleMode> modes = {
        CompatibleStyleMode::LANDSCAPE_DEFAULT,
        CompatibleStyleMode::LANDSCAPE_18_9,
        CompatibleStyleMode::LANDSCAPE_1_1,
        CompatibleStyleMode::LANDSCAPE_2_3,
        CompatibleStyleMode::LANDSCAPE_SPLIT,
        CompatibleStyleMode::LANDSCAPE_3_2,
        CompatibleStyleMode::LANDSCAPE_4_3,
        CompatibleStyleMode::LANDSCAPE_16_9,
        CompatibleStyleMode::LANDSCAPE_SCALE_VERTICAL_FULL,
    };
    for (auto mode : modes) {
        window->property_->SetPageCompatibleMode(mode);
        EXPECT_EQ(window->property_->GetPageCompatibleMode(), mode);
    }
    GTEST_LOG_(INFO) << "PageCompatibleModeAllEnumValuesRoundTrip test end";
}
} // namespace
} // namespace Rosen
} // namespace OHOS
