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

#include <filesystem>
#include <fstream>
#include <gtest/gtest.h>

#include "ability_context_impl.h"
#include "accessibility_event_info.h"
#include "color_parser.h"
#include "mock_session.h"
#include "mock_session_stub.h"
#include "mock_uicontent.h"
#include "mock_window.h"
#include "mock_uiext_session_permission.h"
#include "parameters.h"
#include "scene_board_judgement.h"
#include "window_accessibility_controller.h"
#include "window_helper.h"
#include "window_session_impl.h"
#include "window_scene_session_impl.h"
#include "wm_common.h"
#include "window_manager_hilog.h"

using namespace testing;
using namespace testing::ext;

namespace OHOS {
namespace Rosen {
namespace {
    std::string g_errLog;
    void MyLogCallback(const LogType type, const LogLevel level, const unsigned int domain, const char *tag,
        const char *msg)
    {
        g_errLog = msg;
    }
class WindowSessionImplTest4 : public testing::Test {
public:
    static void SetUpTestCase();
    static void TearDownTestCase();
    void SetUp() override;
    void TearDown() override;

    std::shared_ptr<AbilityRuntime::AbilityContext> abilityContext_;

private:
    static constexpr uint32_t WAIT_SYNC_IN_NS = 50000;
};

void WindowSessionImplTest4::SetUpTestCase() {}

void WindowSessionImplTest4::TearDownTestCase() {}

void WindowSessionImplTest4::SetUp()
{
    abilityContext_ = std::make_shared<AbilityRuntime::AbilityContextImpl>();
}

void WindowSessionImplTest4::TearDown()
{
    usleep(WAIT_SYNC_IN_NS);
    abilityContext_ = nullptr;
}

namespace {
/**
 * @tc.name: GetRequestWindowStatetest01
 * @tc.desc: GetRequestWindowState
 * @tc.type: FUNC
 */
HWTEST_F(WindowSessionImplTest4, GetRequestWindowState, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "WindowSessionImplTest4: GetRequestWindowStatetest01 start";
    sptr<WindowOption> option = sptr<WindowOption>::MakeSptr();
    option->SetWindowName("GetRequestWindowState");
    sptr<WindowSessionImpl> window = sptr<WindowSessionImpl>::MakeSptr(option);
    auto ret = window->GetRequestWindowState();
    ASSERT_EQ(ret, WindowState::STATE_INITIAL);
    GTEST_LOG_(INFO) << "WindowSessionImplTest4: GetRequestWindowStatetest01 end";
}

/**
 * @tc.name: TransferAccessibilityEvent
 * @tc.desc: TransferAccessibilityEvent
 * @tc.type: FUNC
 */
HWTEST_F(WindowSessionImplTest4, TransferAccessibilityEvent, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "WindowSessionImplTest4: TransferAccessibilityEvent start";
    sptr<WindowOption> option = sptr<WindowOption>::MakeSptr();
    sptr<WindowSessionImpl> window = sptr<WindowSessionImpl>::MakeSptr(option);
    Accessibility::AccessibilityEventInfo info;
    int64_t uiExtensionIdLevel = 0;
    ASSERT_EQ(WMError::WM_OK, window->TransferAccessibilityEvent(info, uiExtensionIdLevel));
    GTEST_LOG_(INFO) << "WindowSessionImplTest4: TransferAccessibilityEvent end";
}

/**
 * @tc.name: SetSingleFrameComposerEnabled01
 * @tc.desc: SetSingleFrameComposerEnabled and check the retCode
 * @tc.type: FUNC
 */
HWTEST_F(WindowSessionImplTest4, SetSingleFrameComposerEnabled01, TestSize.Level1)
{
    sptr<WindowOption> option = sptr<WindowOption>::MakeSptr();
    option->SetWindowName("SetSingleFrameComposerEnabled01");
    sptr<WindowSessionImpl> window = sptr<WindowSessionImpl>::MakeSptr(option);
    WMError retCode = window->SetSingleFrameComposerEnabled(false);
    ASSERT_EQ(retCode, WMError::WM_ERROR_INVALID_WINDOW);
    window->property_->SetPersistentId(1);
    SessionInfo sessionInfo = { "CreateTestBundle", "CreateTestModule", "CreateTestAbility" };
    sptr<SessionMocker> session = new (std::nothrow) SessionMocker(sessionInfo);
    ASSERT_NE(nullptr, session);
    window->hostSession_ = session;
    window->state_ = WindowState::STATE_CREATED;
    retCode = window->SetSingleFrameComposerEnabled(false);
    ASSERT_EQ(retCode, WMError::WM_OK);

    window->surfaceNode_ = nullptr;
    retCode = window->SetSingleFrameComposerEnabled(false);
    ASSERT_EQ(retCode, WMError::WM_ERROR_INVALID_WINDOW);
}

/**
 * @tc.name: SetDecorVisible
 * @tc.desc: SetDecorVisible and check the retCode
 * @tc.type: FUNC
 */
HWTEST_F(WindowSessionImplTest4, SetDecorVisible, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "WindowSessionImplTest4: SetDecorVisibletest01 start";
    sptr<WindowOption> option = sptr<WindowOption>::MakeSptr();
    option->SetWindowName("SetDecorVisible");
    sptr<WindowSessionImpl> window = sptr<WindowSessionImpl>::MakeSptr(option);
    ASSERT_NE(window->property_, nullptr);
    window->property_->SetPersistentId(1);
    SessionInfo sessionInfo = { "CreateTestBundle", "CreateTestModule", "CreateTestAbility" };
    sptr<SessionMocker> session = sptr<SessionMocker>::MakeSptr(sessionInfo);
    ASSERT_NE(nullptr, session);
    window->hostSession_ = session;

    bool isVisible = true;
    WMError res = window->SetDecorVisible(isVisible);
    ASSERT_EQ(res, WMError::WM_ERROR_NULLPTR);

    window->uiContent_ = std::make_unique<Ace::UIContentMocker>();
    res = window->SetDecorVisible(isVisible);
    ASSERT_EQ(res, WMError::WM_OK);
    GTEST_LOG_(INFO) << "WindowSessionImplTest4: SetDecorVisibletest01 end";
}

/**
 * @tc.name: GetDecorVisible
 * @tc.desc: GetDecorVisible and check the retCode
 * @tc.type: FUNC
 */
HWTEST_F(WindowSessionImplTest4, GetDecorVisible, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "WindowSessionImplTest4: GetDecorVisible start";
    sptr<WindowOption> option = sptr<WindowOption>::MakeSptr();
    option->SetWindowName("GetDecorVisible");
    sptr<WindowSessionImpl> window = sptr<WindowSessionImpl>::MakeSptr(option);
    SessionInfo sessionInfo = { "CreateTestBundle", "CreateTestModule", "CreateTestAbility" };
    sptr<SessionMocker> session = sptr<SessionMocker>::MakeSptr(sessionInfo);
    window->hostSession_ = session;
    window->property_->SetPersistentId(INVALID_SESSION_ID);
    bool isVisible = true;
    ASSERT_EQ(WMError::WM_ERROR_INVALID_WINDOW, window->GetDecorVisible(isVisible));
    window->property_->SetPersistentId(1);
    ASSERT_EQ(WMError::WM_ERROR_NULLPTR, window->GetDecorVisible(isVisible));
    auto uiContent = std::make_unique<Ace::UIContentMocker>();
    EXPECT_CALL(*uiContent, GetContainerModalTitleVisible(_)).WillRepeatedly(Return(false));
    window->uiContent_ = std::move(uiContent);
    ASSERT_EQ(WMError::WM_OK, window->GetDecorVisible(isVisible));
    ASSERT_FALSE(isVisible);
    GTEST_LOG_(INFO) << "WindowSessionImplTest4: GetDecorVisible end";
}

/**
 * @tc.name: SetWindowTitleMoveEnabled
 * @tc.desc: SetWindowTitleMoveEnabled and check the retCode
 * @tc.type: FUNC
 */
HWTEST_F(WindowSessionImplTest4, SetWindowTitleMoveEnabled, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "WindowSessionImplTest4: SetWindowTitleMoveEnabledtest01 start";
    sptr<WindowOption> option = sptr<WindowOption>::MakeSptr();
    option->SetWindowName("SetWindowTitleMoveEnabled");
    sptr<WindowSessionImpl> window = sptr<WindowSessionImpl>::MakeSptr(option);
    WMError res = window->SetWindowTitleMoveEnabled(true);
    EXPECT_EQ(res, WMError::WM_ERROR_INVALID_WINDOW);
    window->property_->SetPersistentId(1);
    SessionInfo sessionInfo = { "CreateTestBundle", "CreateTestModule", "CreateTestAbility" };
    sptr<SessionMocker> session = sptr<SessionMocker>::MakeSptr(sessionInfo);
    window->hostSession_ = session;
    window->property_->SetWindowType(WindowType::WINDOW_TYPE_UI_EXTENSION);
    res = window->SetWindowTitleMoveEnabled(true);
    EXPECT_EQ(res, WMError::WM_ERROR_INVALID_CALLING);
    window->property_->SetWindowType(WindowType::WINDOW_TYPE_APP_SUB_WINDOW);
    res = window->SetWindowTitleMoveEnabled(true);
    EXPECT_EQ(res, WMError::WM_ERROR_NULLPTR);
    window->uiContent_ = std::make_unique<Ace::UIContentMocker>();
    res = window->SetWindowTitleMoveEnabled(true);
    EXPECT_EQ(res, WMError::WM_OK);
    res = window->SetWindowTitleMoveEnabled(false);
    EXPECT_EQ(res, WMError::WM_OK);
    GTEST_LOG_(INFO) << "WindowSessionImplTest4: SetWindowTitleMoveEnabledtest01 end";
}

/**
 * @tc.name: SetSubWindowModal
 * @tc.desc: SetSubWindowModal and check the retCode
 * @tc.type: FUNC
 */
HWTEST_F(WindowSessionImplTest4, SetSubWindowModal, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "WindowSessionImplTest4: SetSubWindowModaltest01 start";
    sptr<WindowOption> option = sptr<WindowOption>::MakeSptr();
    option->SetWindowName("SetSubWindowModal");
    sptr<WindowSessionImpl> mainWindow = sptr<WindowSessionImpl>::MakeSptr(option);
    SessionInfo sessionInfo = { "CreateTestBundle", "CreateTestModule", "CreateTestAbility" };
    sptr<SessionMocker> session = sptr<SessionMocker>::MakeSptr(sessionInfo);
    mainWindow->hostSession_ = session;
    ASSERT_NE(nullptr, mainWindow->property_);
    mainWindow->property_->SetPersistentId(1); // 1 is main window id
    mainWindow->state_ = WindowState::STATE_CREATED;
    WMError res = mainWindow->SetSubWindowModal(true); // main window is invalid
    ASSERT_EQ(WMError::WM_ERROR_INVALID_CALLING, res);

    option->SetWindowType(WindowType::WINDOW_TYPE_APP_SUB_WINDOW);
    sptr<WindowSessionImpl> window = sptr<WindowSessionImpl>::MakeSptr(option);
    ASSERT_NE(window, nullptr);
    res = window->SetSubWindowModal(true);            // sub window is valid
    ASSERT_EQ(WMError::WM_ERROR_INVALID_WINDOW, res); // window state is invalid

    window->hostSession_ = session;
    ASSERT_NE(nullptr, window->property_);
    window->property_->SetPersistentId(2); // 2 is sub window id
    window->state_ = WindowState::STATE_CREATED;
    res = window->SetSubWindowModal(true); // sub window is valid
    ASSERT_EQ(WMError::WM_OK, res);
    res = window->SetSubWindowModal(false);
    ASSERT_EQ(WMError::WM_OK, res);
    GTEST_LOG_(INFO) << "WindowSessionImplTest4: SetSubWindowModaltest01 end";
}

/**
 * @tc.name: SetSubWindowModal02
 * @tc.desc: SetSubWindowModal and check the retCode
 * @tc.type: FUNC
 */
HWTEST_F(WindowSessionImplTest4, SetSubWindowModal02, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "WindowSessionImplTest4: SetSubWindowModaltest02 start";
    sptr<WindowOption> option = sptr<WindowOption>::MakeSptr();
    option->SetWindowName("SetSubWindowModal02");
    option->SetWindowType(WindowType::WINDOW_TYPE_APP_SUB_WINDOW);
    sptr<WindowSessionImpl> window = sptr<WindowSessionImpl>::MakeSptr(option);
    ASSERT_NE(nullptr, window->property_);
    window->property_->SetPersistentId(1);
    SessionInfo sessionInfo = { "CreateTestBundle", "CreateTestModule", "CreateTestAbility" };
    sptr<SessionMocker> session = sptr<SessionMocker>::MakeSptr(sessionInfo);
    window->hostSession_ = session;
    window->windowSystemConfig_.windowUIType_ = WindowUIType::PC_WINDOW;
    WMError res = window->SetSubWindowModal(true, ModalityType::WINDOW_MODALITY);
    ASSERT_EQ(res, WMError::WM_OK);
    res = window->SetSubWindowModal(true, ModalityType::APPLICATION_MODALITY);
    ASSERT_EQ(res, WMError::WM_OK);
    GTEST_LOG_(INFO) << "WindowSessionImplTest4: SetSubWindowModaltest02 end";
}

/**
 * @tc.name: SetSubWindowModal03
 * @tc.desc: SetSubWindowModal and check the retcode
 * @tc.type: FUNC
 */
HWTEST_F(WindowSessionImplTest4, SetSubWindowModal03, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "WindowSessionImplTest4: SetSubWindowModaltest03 start";
    sptr<WindowOption> option = sptr<WindowOption>::MakeSptr();
    option->SetWindowName("SetSubWindowModal03");
    sptr<WindowSessionImpl> window = sptr<WindowSessionImpl>::MakeSptr(option);
    WMError res = window->SetSubWindowModal(true, ModalityType::WINDOW_MODALITY);
    ASSERT_EQ(res, WMError::WM_ERROR_INVALID_WINDOW);
    GTEST_LOG_(INFO) << "WindowSessionImplTest4: SetSubWindowModaltest03 end";
}

/**
 * @tc.name: SetSubWindowModal04
 * @tc.desc: SetSubWindowModal SetSubWindowModal and check the retcode
 * @tc.type: FUNC
 */
HWTEST_F(WindowSessionImplTest4, SetSubWindowModal04, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "WindowSessionImplTest4: SetSubWindowModaltest04 start";
    sptr<WindowOption> option = sptr<WindowOption>::MakeSptr();
    option->SetWindowName("SetSubWindowModal04");
    sptr<WindowSessionImpl> window = sptr<WindowSessionImpl>::MakeSptr(option);

    SessionInfo sessionInfo = {"CreateTestBundle", "CreateTestModule", "CreateTestAbility"};
    sptr<SessionMocker> session = sptr<SessionMocker>::MakeSptr(sessionInfo);
    window->hostSession_ = session;
    window->property_->SetPersistentId(1);
    option->SetWindowType(WindowType::WINDOW_TYPE_APP_MAIN_WINDOW);
    WMError res = window->SetSubWindowModal(true, ModalityType::WINDOW_MODALITY);
    ASSERT_EQ(res, WMError::WM_ERROR_INVALID_CALLING);
    GTEST_LOG_(INFO) << "WindowSessionImplTest4: SetSubWindowModaltest04 end";
}

/**
 * @tc.name: SetWindowModal
 * @tc.desc: SetWindowModal and check the retCode
 * @tc.type: FUNC
 */
HWTEST_F(WindowSessionImplTest4, SetWindowModal, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "WindowSessionImplTest4: SetWindowModal start";
    sptr<WindowOption> option = sptr<WindowOption>::MakeSptr();
    option->SetWindowName("SetWindowModal");
    option->SetWindowType(WindowType::WINDOW_TYPE_APP_MAIN_WINDOW);
    sptr<WindowSessionImpl> window = sptr<WindowSessionImpl>::MakeSptr(option);
    window->property_->SetPersistentId(1);
    SessionInfo sessionInfo = { "CreateTestBundle", "CreateTestModule", "CreateTestAbility" };
    sptr<SessionMocker> session = sptr<SessionMocker>::MakeSptr(sessionInfo);
    window->hostSession_ = session;
    window->windowSystemConfig_.windowUIType_ = WindowUIType::PC_WINDOW;
    WMError res = window->SetWindowModal(true);
    ASSERT_EQ(res, WMError::WM_OK);
    res = window->SetWindowModal(false);
    ASSERT_EQ(res, WMError::WM_OK);
    window->windowSystemConfig_.windowUIType_ = WindowUIType::PHONE_WINDOW;
    res = window->SetWindowModal(true);
    ASSERT_EQ(res, WMError::WM_ERROR_DEVICE_NOT_SUPPORT);
    res = window->SetWindowModal(false);
    ASSERT_EQ(res, WMError::WM_ERROR_DEVICE_NOT_SUPPORT);
    GTEST_LOG_(INFO) << "WindowSessionImplTest4: SetWindowModal end";
}

/**
 * @tc.name: IsPcWindow
 * @tc.desc: IsPcWindow
 * @tc.type: FUNC
 */
HWTEST_F(WindowSessionImplTest4, IsPcWindow, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "WindowSessionImplTest4: IsPcWindow start";
    sptr<WindowOption> option = sptr<WindowOption>::MakeSptr();
    option->SetWindowName("IsPcWindow");
    option->SetWindowType(WindowType::WINDOW_TYPE_APP_SUB_WINDOW);
    sptr<WindowSessionImpl> window = sptr<WindowSessionImpl>::MakeSptr(option);
    window->property_->SetPersistentId(1);
    SessionInfo sessionInfo = { "CreateTestBundle", "CreateTestModule", "CreateTestAbility" };
    sptr<SessionMocker> session = sptr<SessionMocker>::MakeSptr(sessionInfo);
    window->hostSession_ = session;
    window->windowSystemConfig_.windowUIType_ = WindowUIType::PC_WINDOW;
    ASSERT_EQ(true, window->IsPcWindow());
    window->windowSystemConfig_.windowUIType_ = WindowUIType::PHONE_WINDOW;
    ASSERT_EQ(false, window->IsPcWindow());
    GTEST_LOG_(INFO) << "WindowSessionImplTest4: IsPcWindow end";
}

/**
 * @tc.name: IsPhonePadOrPcWindow
 * @tc.desc: IsPhonePadOrPcWindow
 * @tc.type: FUNC
 */
HWTEST_F(WindowSessionImplTest4, IsPhonePadOrPcWindow, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "WindowSessionImplTest4: IsPhonePadOrPcWindow start";
    sptr<WindowOption> option = sptr<WindowOption>::MakeSptr();
    option->SetWindowName("IsPhonePadOrPcWindow");
    option->SetWindowType(WindowType::WINDOW_TYPE_APP_SUB_WINDOW);
    sptr<WindowSessionImpl> window = sptr<WindowSessionImpl>::MakeSptr(option);
    window->property_->SetPersistentId(1);
    SessionInfo sessionInfo = { "CreateTestBundle", "CreateTestModule", "CreateTestAbility" };
    sptr<SessionMocker> session = sptr<SessionMocker>::MakeSptr(sessionInfo);
    window->hostSession_ = session;
    // Case 1: Test phone window
    window->windowSystemConfig_.windowUIType_ = WindowUIType::PHONE_WINDOW;
    EXPECT_EQ(true, window->IsPhonePadOrPcWindow());

    // Case 2: Test pad window
    window->windowSystemConfig_.windowUIType_ = WindowUIType::PAD_WINDOW;
    EXPECT_EQ(true, window->IsPhonePadOrPcWindow());

    // Case 3: Test pc window
    window->windowSystemConfig_.windowUIType_ = WindowUIType::PC_WINDOW;
    EXPECT_EQ(true, window->IsPhonePadOrPcWindow());

    // Case 4: Test invalid window
    window->windowSystemConfig_.windowUIType_ = WindowUIType::INVALID_WINDOW;
    EXPECT_EQ(false, window->IsPhonePadOrPcWindow());
    GTEST_LOG_(INFO) << "WindowSessionImplTest4: IsPhonePadOrPcWindow end";
}

/**
 * @tc.name: IsPadAndNotFreeMultiWindowCompatibleMode
 * @tc.desc: IsPadAndNotFreeMultiWindowCompatibleMode
 * @tc.type: FUNC
 */
HWTEST_F(WindowSessionImplTest4, IsPadAndNotFreeMultiWindowCompatibleMode, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "WindowSessionImplTest4: IsPadAndNotFreeMultiWindowCompatibleMode start";
    sptr<WindowOption> option = sptr<WindowOption>::MakeSptr();
    option->SetWindowName("IsPadAndNotFreeMultiWindowCompatibleMode");
    option->SetWindowType(WindowType::WINDOW_TYPE_APP_SUB_WINDOW);
    sptr<WindowSessionImpl> window = sptr<WindowSessionImpl>::MakeSptr(option);
    window->property_->SetPersistentId(1);
    SessionInfo sessionInfo = { "CreateTestBundle", "CreateTestModule", "CreateTestAbility" };
    sptr<SessionMocker> session = sptr<SessionMocker>::MakeSptr(sessionInfo);
    window->hostSession_ = session;
    window->windowSystemConfig_.windowUIType_ = WindowUIType::PAD_WINDOW;
    window->property_->SetPcAppInpadCompatibleMode(true);
    window->windowSystemConfig_.freeMultiWindowEnable_ = false;
    EXPECT_EQ(true, window->IsPadAndNotFreeMultiWindowCompatibleMode());
    window->windowSystemConfig_.windowUIType_ = WindowUIType::PAD_WINDOW;
    window->property_->SetPcAppInpadCompatibleMode(true);
    window->windowSystemConfig_.freeMultiWindowEnable_ = true;
    window->windowSystemConfig_.freeMultiWindowSupport_ = true;
    EXPECT_EQ(false, window->IsPadAndNotFreeMultiWindowCompatibleMode());
    GTEST_LOG_(INFO) << "WindowSessionImplTest4: IsPadAndNotFreeMultiWindowCompatibleMode end";
}

/**
 * @tc.name: IsPcOrPadFreeMultiWindowMode
 * @tc.desc: IsPcOrPadFreeMultiWindowMode
 * @tc.type: FUNC
 */
HWTEST_F(WindowSessionImplTest4, IsPcOrPadFreeMultiWindowMode, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "WindowSessionImplTest4: IsPcOrPadFreeMultiWindowMode start";
    sptr<WindowOption> option = sptr<WindowOption>::MakeSptr();
    option->SetWindowName("IsPcOrPadFreeMultiWindowMode");
    option->SetWindowType(WindowType::WINDOW_TYPE_APP_SUB_WINDOW);
    sptr<WindowSessionImpl> window = sptr<WindowSessionImpl>::MakeSptr(option);
    window->property_->SetPersistentId(1);
    SessionInfo sessionInfo = { "CreateTestBundle", "CreateTestModule", "CreateTestAbility" };
    sptr<SessionMocker> session = sptr<SessionMocker>::MakeSptr(sessionInfo);
    window->hostSession_ = session;
    window->windowSystemConfig_.windowUIType_ = WindowUIType::PC_WINDOW;
    ASSERT_EQ(true, window->IsPcOrPadFreeMultiWindowMode());
    window->windowSystemConfig_.windowUIType_ = WindowUIType::PHONE_WINDOW;
    ASSERT_EQ(false, window->IsPcOrPadFreeMultiWindowMode());
    GTEST_LOG_(INFO) << "WindowSessionImplTest4: IsPcOrPadFreeMultiWindowMode end";
}

/**
 * @tc.name: GetVirtualPixelRatio01
 * @tc.desc: GetVirtualPixelRatio
 * @tc.type: FUNC
 */
HWTEST_F(WindowSessionImplTest4, GetVirtualPixelRatio01, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "WindowSessionImplTest4: GetVirtualPixelRatio01 start";
    sptr<WindowOption> option = sptr<WindowOption>::MakeSptr();
    option->SetWindowName("GetVirtualPixelRatio01");
    sptr<WindowSessionImpl> window = sptr<WindowSessionImpl>::MakeSptr(option);
    window->property_->SetPersistentId(1);
    SessionInfo sessionInfo = { "CreateTestBundle", "CreateTestModule", "CreateTestAbility" };
    sptr<SessionMocker> session = sptr<SessionMocker>::MakeSptr(sessionInfo);
    window->hostSession_ = session;
    float vpr = 0.f;
    window->property_->SetDisplayId(-1);
    auto res = window->GetVirtualPixelRatio(vpr);
    ASSERT_EQ(res, WMError::WM_ERROR_NULLPTR);
    window->property_->SetDisplayId(0);
    res = window->GetVirtualPixelRatio(vpr);
    ASSERT_EQ(res, WMError::WM_OK);
    GTEST_LOG_(INFO) << "WindowSessionImplTest4: GetVirtualPixelRatio01 end";
}

/**
 * @tc.name: GetDecorHeight
 * @tc.desc: GetDecorHeight and check the retCode
 * @tc.type: FUNC
 */
HWTEST_F(WindowSessionImplTest4, GetDecorHeight, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "WindowSessionImplTest4: GetDecorHeighttest01 start";
    sptr<WindowOption> option = sptr<WindowOption>::MakeSptr();
    option->SetWindowName("GetDecorHeight");
    sptr<WindowSessionImpl> window = sptr<WindowSessionImpl>::MakeSptr(option);
    ASSERT_NE(window->property_, nullptr);
    window->property_->SetPersistentId(1);
    SessionInfo sessionInfo = { "CreateTestBundle", "CreateTestModule", "CreateTestAbility" };
    sptr<SessionMocker> session = sptr<SessionMocker>::MakeSptr(sessionInfo);
    window->hostSession_ = session;
    int32_t height = 0;
    WMError res = window->GetDecorHeight(height);
    ASSERT_EQ(res, WMError::WM_ERROR_NULLPTR);
    GTEST_LOG_(INFO) << "WindowSessionImplTest4: GetDecorHeighttest01 end";
}

/**
 * @tc.name: GetDecorButtonStyle
 * @tc.desc: GetDecorButtonStyle and check the retCode
 * @tc.type: FUNC
 */
HWTEST_F(WindowSessionImplTest4, GetDecorButtonStyle, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "WindowSessionImplTest4: GetDecorButtonStyle start";
    sptr<WindowOption> option = sptr<WindowOption>::MakeSptr();
    option->SetWindowName("GetDecorButtonStyle");
    sptr<WindowSessionImpl> window = sptr<WindowSessionImpl>::MakeSptr(option);
    window->property_->SetPersistentId(1);
    SessionInfo sessionInfo = { "CreateTestBundle", "CreateTestModule", "CreateTestAbility" };
    sptr<SessionMocker> session = sptr<SessionMocker>::MakeSptr(sessionInfo);
    window->hostSession_ = session;
    window->state_ = WindowState::STATE_CREATED;

    // check window type
    DecorButtonStyle style;
    window->property_->SetWindowType(WindowType::WINDOW_TYPE_DIALOG);
    WMError res = window->GetDecorButtonStyle(style);
    ASSERT_EQ(res, WMError::WM_ERROR_INVALID_CALLING);

    // check default set
    window->property_->SetWindowType(WindowType::APP_MAIN_WINDOW_BASE);
    res = window->GetDecorButtonStyle(style);
    ASSERT_EQ(style.buttonBackgroundSize, DEFAULT_BUTTON_BACKGROUND_SIZE);
    ASSERT_EQ(style.closeButtonRightMargin, DEFAULT_CLOSE_BUTTON_RIGHT_MARGIN);
    ASSERT_EQ(style.spacingBetweenButtons, DEFAULT_SPACING_BETWEEN_BUTTONS);
    ASSERT_EQ(style.colorMode, DEFAULT_COLOR_MODE);
    ASSERT_EQ(style.buttonIconSize, DEFAULT_BUTTON_ICON_SIZE);
    ASSERT_EQ(style.buttonBackgroundCornerRadius, DEFAULT_BUTTON_BACKGROUND_CORNER_RADIUS);
    ASSERT_EQ(res, WMError::WM_OK);
}

/**
 * @tc.name: SetDecorButtonStyle
 * @tc.desc: SetDecorButtonStyle and check the retCode
 * @tc.type: FUNC
 */
HWTEST_F(WindowSessionImplTest4, SetDecorButtonStyle, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "WindowSessionImplTest4: SetDecorButtonStyle start";
    sptr<WindowOption> option = sptr<WindowOption>::MakeSptr();
    option->SetWindowName("SetDecorButtonStyle");
    sptr<WindowSessionImpl> window = sptr<WindowSessionImpl>::MakeSptr(option);
    ASSERT_NE(window, nullptr);
    // check window type
    window->property_->SetPersistentId(1);
    SessionInfo sessionInfo = { "CreateTestBundle", "CreateTestModule", "CreateTestAbility" };
    sptr<SessionMocker> session = sptr<SessionMocker>::MakeSptr(sessionInfo);
    window->hostSession_ = session;
    window->state_ = WindowState::STATE_CREATED;

    // check window type
    DecorButtonStyle style;
    window->property_->SetWindowType(WindowType::WINDOW_TYPE_DIALOG);
    WMError res = window->SetDecorButtonStyle(style);
    ASSERT_EQ(res, WMError::WM_ERROR_INVALID_CALLING);

    // check get value
    window->property_->SetWindowType(WindowType::APP_MAIN_WINDOW_BASE);
    style.buttonBackgroundSize = -1;
    res = window->SetDecorButtonStyle(style);
    ASSERT_EQ(res, WMError::WM_ERROR_INVALID_PARAM);

    // check uiContent is null
    style.buttonBackgroundSize = 40;
    res = window->SetDecorButtonStyle(style);
    ASSERT_EQ(res, WMError::WM_ERROR_NULLPTR);

    // check same style data
    window->uiContent_ = std::make_unique<Ace::UIContentMocker>();
    res = window->SetDecorButtonStyle(style);
    ASSERT_EQ(res, WMError::WM_OK);
    DecorButtonStyle styleRes;
    res = window->GetDecorButtonStyle(styleRes);
    ASSERT_EQ(styleRes.buttonBackgroundSize, style.buttonBackgroundSize);
}

/**
 * @tc.name: GetTitleButtonArea
 * @tc.desc: GetTitleButtonArea and check the retCode
 * @tc.type: FUNC
 */
HWTEST_F(WindowSessionImplTest4, GetTitleButtonArea, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "WindowSessionImplTest4: GetTitleButtonAreatest01 start";
    sptr<WindowOption> option = sptr<WindowOption>::MakeSptr();
    option->SetWindowName("GetTitleButtonArea");
    sptr<WindowSessionImpl> window = sptr<WindowSessionImpl>::MakeSptr(option);
    ASSERT_NE(window->property_, nullptr);
    window->property_->SetPersistentId(1);
    SessionInfo sessionInfo = { "CreateTestBundle", "CreateTestModule", "CreateTestAbility" };
    sptr<SessionMocker> session = sptr<SessionMocker>::MakeSptr(sessionInfo);
    ASSERT_NE(nullptr, session);
    window->hostSession_ = session;
    TitleButtonRect titleButtonRect;
    WMError res = window->GetTitleButtonArea(titleButtonRect);
    ASSERT_EQ(res, WMError::WM_ERROR_NULLPTR);
    GTEST_LOG_(INFO) << "WindowSessionImplTest4: GetDecorHeighttest01 end";
}

/**
 * @tc.name: GetUIContentRemoteObj
 * @tc.desc: GetUIContentRemoteObj and check the retCode
 * @tc.type: FUNC
 */
HWTEST_F(WindowSessionImplTest4, GetUIContentRemoteObj, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "WindowSessionImplTest4: GetUIContentRemoteObj start";
    sptr<WindowOption> option = sptr<WindowOption>::MakeSptr();
    sptr<WindowSessionImpl> window = sptr<WindowSessionImpl>::MakeSptr(option);
    sptr<IRemoteObject> remoteObj;
    WSError res = window->GetUIContentRemoteObj(remoteObj);
    ASSERT_EQ(res, WSError::WS_ERROR_NULLPTR);
    window->uiContent_ = std::make_unique<Ace::UIContentMocker>();
    res = window->GetUIContentRemoteObj(remoteObj);
    ASSERT_EQ(res, WSError::WS_OK);
    MockUIExtSessionPermission::SetIsSystemCallingFlag(false);
    res = window->GetUIContentRemoteObj(remoteObj);
    ASSERT_EQ(res, WSError::WS_ERROR_NOT_SYSTEM_APP);
    MockUIExtSessionPermission::ClearAllFlag();
    GTEST_LOG_(INFO) << "WindowSessionImplTest4: GetUIContentRemoteObj end";
}

/**
 * @tc.name: RegisterExtensionAvoidAreaChangeListener
 * @tc.desc: RegisterExtensionAvoidAreaChangeListener Test
 * @tc.type: FUNC
 */
HWTEST_F(WindowSessionImplTest4, RegisterExtensionAvoidAreaChangeListener, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "WindowSessionImplTest4: RegisterExtensionAvoidAreaChangeListener start";
    sptr<WindowOption> option = sptr<WindowOption>::MakeSptr();
    option->SetWindowName("GetTitleButtonArea");
    sptr<WindowSessionImpl> window = sptr<WindowSessionImpl>::MakeSptr(option);
    sptr<IAvoidAreaChangedListener> listener = nullptr;
    WMError res = window->RegisterExtensionAvoidAreaChangeListener(listener);
    ASSERT_EQ(res, WMError::WM_ERROR_NULLPTR);

    listener = sptr<IAvoidAreaChangedListener>::MakeSptr();
    vector<sptr<IAvoidAreaChangedListener>> holder;
    window->avoidAreaChangeListeners_[window->property_->GetPersistentId()] = holder;
    res = window->RegisterExtensionAvoidAreaChangeListener(listener);
    ASSERT_EQ(res, WMError::WM_OK);
    holder = window->avoidAreaChangeListeners_[window->property_->GetPersistentId()];
    auto existsListener = std::find(holder.begin(), holder.end(), listener);
    ASSERT_NE(existsListener, holder.end());

    // already registered
    res = window->RegisterExtensionAvoidAreaChangeListener(listener);
    ASSERT_EQ(res, WMError::WM_OK);
    GTEST_LOG_(INFO) << "WindowSessionImplTest4: RegisterExtensionAvoidAreaChangeListener end";
}

/**
 * @tc.name: UnregisterExtensionAvoidAreaChangeListener
 * @tc.desc: UnregisterExtensionAvoidAreaChangeListener Test
 * @tc.type: FUNC
 */
HWTEST_F(WindowSessionImplTest4, UnregisterExtensionAvoidAreaChangeListener, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "WindowSessionImplTest4: UnregisterExtensionAvoidAreaChangeListener start";
    sptr<WindowOption> option = sptr<WindowOption>::MakeSptr();
    option->SetWindowName("GetTitleButtonArea");
    sptr<WindowSessionImpl> window = sptr<WindowSessionImpl>::MakeSptr(option);
    sptr<IAvoidAreaChangedListener> listener = nullptr;
    WMError res = window->UnregisterExtensionAvoidAreaChangeListener(listener);
    ASSERT_EQ(res, WMError::WM_ERROR_NULLPTR);

    listener = sptr<IAvoidAreaChangedListener>::MakeSptr();
    vector<sptr<IAvoidAreaChangedListener>> holder;
    window->avoidAreaChangeListeners_[window->property_->GetPersistentId()] = holder;
    window->RegisterExtensionAvoidAreaChangeListener(listener);

    res = window->UnregisterExtensionAvoidAreaChangeListener(listener);
    ASSERT_EQ(res, WMError::WM_OK);

    holder = window->avoidAreaChangeListeners_[window->property_->GetPersistentId()];
    auto existsListener = std::find(holder.begin(), holder.end(), listener);
    ASSERT_EQ(existsListener, holder.end());
    GTEST_LOG_(INFO) << "WindowSessionImplTest4: UnregisterExtensionAvoidAreaChangeListener end";
}

/**
 * @tc.name: SetPipActionEvent
 * @tc.desc: SetPipActionEvent Test
 * @tc.type: FUNC
 */
HWTEST_F(WindowSessionImplTest4, SetPipActionEvent, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "WindowSessionImplTest4: SetPipActionEvent start";
    sptr<WindowOption> option = sptr<WindowOption>::MakeSptr();
    option->SetWindowName("GetTitleButtonArea");
    sptr<WindowSessionImpl> window = sptr<WindowSessionImpl>::MakeSptr(option);
    ASSERT_EQ(nullptr, window->GetUIContentWithId(10000));
    window->property_->SetPersistentId(1);

    SessionInfo sessionInfo = { "CreateTestBundle", "TestGetUIContentWithId", "CreateTestAbility" };
    sptr<SessionMocker> session = new (std::nothrow) SessionMocker(sessionInfo);
    ASSERT_NE(nullptr, session);
    ASSERT_EQ(WMError::WM_OK, window->Create(nullptr, session));
    window->uiContent_ = std::make_unique<Ace::UIContentMocker>();
    if (SceneBoardJudgement::IsSceneBoardEnabled()) {
        ASSERT_EQ(window->FindWindowById(1), nullptr);
        ASSERT_EQ(nullptr, window->GetUIContentWithId(1));
        ASSERT_EQ(WMError::WM_ERROR_INVALID_WINDOW, window->Destroy());
    }
    GTEST_LOG_(INFO) << "WindowSessionImplTest4: SetPipActionEvent end";
}

/**
 * @tc.name: SetPiPControlEvent
 * @tc.desc: SetPiPControlEvent Test
 * @tc.type: FUNC
 */
HWTEST_F(WindowSessionImplTest4, SetPiPControlEvent, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "WindowSessionImplTest4: SetPiPControlEvent start";
    auto option = sptr<WindowOption>::MakeSptr();
    option->SetWindowName("GetTitleButtonArea");
    auto window = sptr<WindowSessionImpl>::MakeSptr(option);
    auto controlType = WsPiPControlType::VIDEO_PLAY_PAUSE;
    auto status = WsPiPControlStatus::PLAY;
    WSError res = window->SetPiPControlEvent(controlType, status);
    ASSERT_EQ(res, WSError::WS_OK);
    GTEST_LOG_(INFO) << "WindowSessionImplTest4: SetPiPControlEvent end";
}

/**
 * @tc.name: NotifyPiPActiveStatusChange
 * @tc.desc: NotifyPiPActiveStatusChange Test
 * @tc.type: FUNC
 */
HWTEST_F(WindowSessionImplTest4, NotifyPiPActiveStatusChange, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "WindowSessionImplTest4: NotifyPiPActiveStatusChange start";
    auto option = sptr<WindowOption>::MakeSptr();
    option->SetWindowName("NotifyPiPActiveStatusChange");
    auto window = sptr<WindowSessionImpl>::MakeSptr(option);
    WSError res = window->NotifyPiPActiveStatusChange(true);
    ASSERT_EQ(res, WSError::WS_OK);
    GTEST_LOG_(INFO) << "WindowSessionImplTest4: NotifyPiPActiveStatusChange end";
}

/**
 * @tc.name: SetUIContentInner
 * @tc.desc: SetUIContentInner Test
 * @tc.type: FUNC
 */
HWTEST_F(WindowSessionImplTest4, SetUIContentInner, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "WindowSessionImplTest4: SetUIContentInner start";
    sptr<WindowOption> option = sptr<WindowOption>::MakeSptr();
    option->SetWindowName("SetUIContentInner");
    option->SetIsUIExtFirstSubWindow(true);
    sptr<WindowSessionImpl> window = sptr<WindowSessionImpl>::MakeSptr(option);
    window->property_->SetPersistentId(1);
    std::string url = "";
    EXPECT_TRUE(window->IsWindowSessionInvalid());
    WMError res1 = window->SetUIContentInner(
        url, nullptr, nullptr, WindowSetUIContentType::DEFAULT, BackupAndRestoreType::NONE, nullptr);
    ASSERT_EQ(res1, WMError::WM_ERROR_INVALID_WINDOW);
    GTEST_LOG_(INFO) << "WindowSessionImplTest4: SetUIContentInner end";
}

/**
 * @tc.name: TestGetUIContentWithId
 * @tc.desc: Get uicontent with id
 * @tc.type: FUNC
 */
HWTEST_F(WindowSessionImplTest4, TestGetUIContentWithId, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "WindowSessionImplTest4: TestGetUIContentWithId start";
    sptr<WindowOption> option = sptr<WindowOption>::MakeSptr();
    option->SetWindowName("TestGetUIContentWithId");
    sptr<WindowSessionImpl> window = sptr<WindowSessionImpl>::MakeSptr(option);
    ASSERT_EQ(nullptr, window->GetUIContentWithId(10000));
    window->property_->SetPersistentId(1);

    SessionInfo sessionInfo = { "CreateTestBundle", "TestGetUIContentWithId", "CreateTestAbility" };
    sptr<SessionMocker> session = sptr<SessionMocker>::MakeSptr(sessionInfo);
    ASSERT_EQ(WMError::WM_OK, window->Create(nullptr, session));
    window->uiContent_ = std::make_unique<Ace::UIContentMocker>();
    if (SceneBoardJudgement::IsSceneBoardEnabled()) {
        ASSERT_NE(window->FindWindowById(1), nullptr);
        ASSERT_EQ(nullptr, window->GetUIContentWithId(1));
        ASSERT_EQ(WMError::WM_ERROR_INVALID_WINDOW, window->Destroy());
    }
    GTEST_LOG_(INFO) << "WindowSessionImplTest4: TestGetUIContentWithId end";
}

/**
 * @tc.name: GetCallingWindowRect
 * @tc.desc: GetCallingWindowRect Test
 * @tc.type: FUNC
 */
HWTEST_F(WindowSessionImplTest4, GetCallingWindowRect, TestSize.Level1)
{
    sptr<WindowOption> option = sptr<WindowOption>::MakeSptr();
    option->SetWindowName("GetCallingWindowRect");
    sptr<WindowSessionImpl> window = sptr<WindowSessionImpl>::MakeSptr(option);
    Rect rect = { 0, 0, 0, 0 };
    WMError retCode = window->GetCallingWindowRect(1, rect);
    ASSERT_EQ(retCode, WMError::WM_ERROR_INVALID_WINDOW);
    window->property_->SetPersistentId(1);
    SessionInfo sessionInfo = { "CreateTestBundle", "CreateTestModule", "CreateTestAbility" };
    sptr<SessionMocker> session = sptr<SessionMocker>::MakeSptr(sessionInfo);
    window->hostSession_ = session;
    window->state_ = WindowState::STATE_CREATED;
    retCode = window->GetCallingWindowRect(1, rect);
    ASSERT_EQ(retCode, WMError::WM_ERROR_INVALID_PERMISSION);
}

/**
 * @tc.name: GetCallingWindowWindowStatus
 * @tc.desc: GetCallingWindowWindowStatus Test
 * @tc.type: FUNC
 */
HWTEST_F(WindowSessionImplTest4, GetCallingWindowWindowStatus, TestSize.Level1)
{
    sptr<WindowOption> option = sptr<WindowOption>::MakeSptr();
    option->SetWindowName("GetCallingWindowWindowStatus");
    sptr<WindowSessionImpl> window = sptr<WindowSessionImpl>::MakeSptr(option);
    ASSERT_NE(nullptr, window);
    WindowStatus windowStatus = WindowStatus::WINDOW_STATUS_UNDEFINED;
    WMError retCode = window->GetCallingWindowWindowStatus(1, windowStatus);
    ASSERT_EQ(retCode, WMError::WM_ERROR_INVALID_WINDOW);
    window->property_->SetPersistentId(1);
    SessionInfo sessionInfo = { "CreateTestBundle", "CreateTestModule", "CreateTestAbility" };
    sptr<SessionMocker> session = sptr<SessionMocker>::MakeSptr(sessionInfo);
    ASSERT_NE(nullptr, session);
    window->hostSession_ = session;
    window->state_ = WindowState::STATE_CREATED;
    retCode = window->GetCallingWindowWindowStatus(1, windowStatus);
    ASSERT_EQ(retCode, WMError::WM_ERROR_INVALID_PERMISSION);
}

/**
 * @tc.name: GetParentId
 * @tc.desc: GetParentId Test
 * @tc.type: FUNC
 */
HWTEST_F(WindowSessionImplTest4, GetParentId, TestSize.Level1)
{
    sptr<WindowOption> option = sptr<WindowOption>::MakeSptr();
    sptr<WindowSessionImpl> window = sptr<WindowSessionImpl>::MakeSptr(option);
    const int32_t res = window->GetParentId();
    ASSERT_EQ(res, 0);
    ASSERT_EQ(true, window->IsSupportWideGamut());
}

/**
 * @tc.name: PreNotifyKeyEvent
 * @tc.desc: PreNotifyKeyEvent Test
 * @tc.type: FUNC
 */
HWTEST_F(WindowSessionImplTest4, PreNotifyKeyEvent, TestSize.Level1)
{
    sptr<WindowOption> option = sptr<WindowOption>::MakeSptr();
    sptr<WindowSessionImpl> window = sptr<WindowSessionImpl>::MakeSptr(option);
    std::shared_ptr<MMI::PointerEvent> pointerEvent;
    window->ConsumePointerEvent(pointerEvent);

    std::shared_ptr<MMI::KeyEvent> keyEvent = MMI::KeyEvent::Create();
    window->ConsumeKeyEvent(keyEvent);
    ASSERT_EQ(nullptr, window->GetUIContentSharedPtr());
    ASSERT_EQ(false, window->PreNotifyKeyEvent(keyEvent));
    ASSERT_EQ(false, window->NotifyOnKeyPreImeEvent(keyEvent));
    window->uiContent_ = std::make_unique<Ace::UIContentMocker>();
    ASSERT_NE(nullptr, window->GetUIContentSharedPtr());
    ASSERT_EQ(false, window->PreNotifyKeyEvent(keyEvent));
    ASSERT_EQ(false, window->NotifyOnKeyPreImeEvent(keyEvent));
}

/**
 * @tc.name: CheckIfNeedCommitRsTransaction
 * @tc.desc: CheckIfNeedCommitRsTransaction
 * @tc.type: FUNC
 */
HWTEST_F(WindowSessionImplTest4, CheckIfNeedCommitRsTransaction, TestSize.Level1)
{
    sptr<WindowOption> option = sptr<WindowOption>::MakeSptr();
    sptr<WindowSessionImpl> window = sptr<WindowSessionImpl>::MakeSptr(option);

    bool res = false;
    WindowSizeChangeReason wmReason = WindowSizeChangeReason::UNDEFINED;
    for (uint32_t i = static_cast<uint32_t>(WindowSizeChangeReason::UNDEFINED);
         i < static_cast<uint32_t>(WindowSizeChangeReason::END);
         i++) {
        wmReason = static_cast<WindowSizeChangeReason>(i);
        res = window->CheckIfNeedCommitRsTransaction(wmReason);
        if (wmReason == WindowSizeChangeReason::FULL_TO_SPLIT || wmReason == WindowSizeChangeReason::FULL_TO_FLOATING ||
            wmReason == WindowSizeChangeReason::RECOVER || wmReason == WindowSizeChangeReason::MAXIMIZE) {
            ASSERT_EQ(res, false);
        } else {
            ASSERT_EQ(res, true);
        }
    }
    window->Destroy();
}

/**
 * @tc.name: NotifyRotationAnimationEnd
 * @tc.desc: NotifyRotationAnimationEnd Test
 * @tc.type: FUNC
 */
HWTEST_F(WindowSessionImplTest4, NotifyRotationAnimationEnd, TestSize.Level1)
{
    sptr<WindowOption> option = sptr<WindowOption>::MakeSptr();
    sptr<WindowSessionImpl> window = sptr<WindowSessionImpl>::MakeSptr(option);
    window->NotifyRotationAnimationEnd();

    OHOS::Ace::UIContentErrorCode aceRet = OHOS::Ace::UIContentErrorCode::NO_ERRORS;
    window->InitUIContent(
        "", nullptr, nullptr, WindowSetUIContentType::BY_ABC, BackupAndRestoreType::NONE, nullptr, aceRet);
    window->NotifyRotationAnimationEnd();
    ASSERT_NE(nullptr, window->uiContent_);
}

/**
 * @tc.name: SetTitleButtonVisible
 * @tc.desc: SetTitleButtonVisible and GetTitleButtonVisible
 * @tc.type: FUNC
 */
HWTEST_F(WindowSessionImplTest4, SetTitleButtonVisible, TestSize.Level1)
{
    sptr<WindowOption> option = sptr<WindowOption>::MakeSptr();
    sptr<WindowSessionImpl> window = sptr<WindowSessionImpl>::MakeSptr(option);
    bool isMaximizeVisible = true;
    bool isMinimizeVisible = true;
    bool isSplitVisible = true;
    bool isCloseVisible = true;
    auto res = window->SetTitleButtonVisible(isMaximizeVisible, isMinimizeVisible, isSplitVisible, isCloseVisible);

    bool& hideMaximizeButton = isMaximizeVisible;
    bool& hideMinimizeButton = isMinimizeVisible;
    bool& hideSplitButton = isSplitVisible;
    bool& hideCloseButton = isCloseVisible;
    window->windowSystemConfig_.windowUIType_ = WindowUIType::PC_WINDOW;
    window->GetTitleButtonVisible(hideMaximizeButton, hideMinimizeButton, hideSplitButton, hideCloseButton);
    ASSERT_EQ(res, WMError::WM_ERROR_INVALID_WINDOW);
}

/**
 * @tc.name: IsFocused
 * @tc.desc: IsFocused
 * @tc.type: FUNC
 */
HWTEST_F(WindowSessionImplTest4, IsFocused, TestSize.Level1)
{
    sptr<WindowOption> option = sptr<WindowOption>::MakeSptr();
    option->SetWindowName("WindowSessionCreateCheck");
    sptr<WindowSessionImpl> window = sptr<WindowSessionImpl>::MakeSptr(option);
    bool res = window->IsFocused();
    ASSERT_EQ(res, false);

    ASSERT_EQ(WMError::WM_ERROR_INVALID_WINDOW, window->RequestFocus());

    SessionInfo sessionInfo = { "CreateTestBundle", "CreateTestModule", "CreateTestAbility" };
    sptr<SessionMocker> session = sptr<SessionMocker>::MakeSptr(sessionInfo);
    ASSERT_EQ(WMError::WM_OK, window->Create(nullptr, session));
    int32_t persistentId = window->GetPersistentId();
    if (persistentId == INVALID_SESSION_ID) {
        persistentId = 1;
        window->property_->SetPersistentId(persistentId);
    }
    if (window->state_ == WindowState::STATE_DESTROYED) {
        window->state_ = WindowState::STATE_SHOWN;
    }
    window->hostSession_ = session;
    window->RequestFocus();
    ASSERT_FALSE(window->IsWindowSessionInvalid());
    ASSERT_EQ(persistentId, window->GetPersistentId());
    ASSERT_EQ(WMError::WM_OK, window->Destroy());
}

/**
 * @tc.name: NapiSetUIContent
 * @tc.desc: NapiSetUIContent Test
 * @tc.type: FUNC
 */
HWTEST_F(WindowSessionImplTest4, NapiSetUIContent, TestSize.Level1)
{
    sptr<WindowOption> option = sptr<WindowOption>::MakeSptr();
    option->SetWindowName("NapiSetUIContent");
    option->SetIsUIExtFirstSubWindow(true);
    sptr<WindowSessionImpl> window = sptr<WindowSessionImpl>::MakeSptr(option);
    window->property_->SetPersistentId(1);
    std::string url = "";
    AppExecFwk::Ability* ability = nullptr;

    window->SetUIContentByName(url, nullptr, nullptr, nullptr);
    window->SetUIContentByAbc(url, nullptr, nullptr, nullptr);
    WMError res1 = window->NapiSetUIContent(url, (napi_env)nullptr, (napi_value)nullptr,
        BackupAndRestoreType::CONTINUATION, nullptr, ability);
    ASSERT_EQ(res1, WMError::WM_ERROR_INVALID_WINDOW);
}

/**
 * @tc.name: GetAbcContent
 * @tc.desc: GetAbcContent Test
 * @tc.type: FUNC
 */
HWTEST_F(WindowSessionImplTest4, GetAbcContent, TestSize.Level1)
{
    sptr<WindowOption> option = sptr<WindowOption>::MakeSptr();
    option->SetWindowName("GetAbcContent");
    sptr<WindowSessionImpl> window = sptr<WindowSessionImpl>::MakeSptr(option);
    std::string abcPath = "";
    std::shared_ptr<std::vector<uint8_t>> res = window->GetAbcContent(abcPath);
    std::filesystem::path abcFile{ abcPath };
    ASSERT_TRUE(abcFile.empty());
    ASSERT_TRUE(!abcFile.is_absolute());
    ASSERT_TRUE(!std::filesystem::exists(abcFile));
    ASSERT_EQ(res, nullptr);

    abcPath = "/abc";
    res = window->GetAbcContent(abcPath);
    std::filesystem::path abcFile2{ abcPath };
    ASSERT_FALSE(abcFile2.empty());
    ASSERT_FALSE(!abcFile2.is_absolute());
    ASSERT_TRUE(!std::filesystem::exists(abcFile2));
    ASSERT_EQ(res, nullptr);

    abcPath = "abc";
    res = window->GetAbcContent(abcPath);
    std::filesystem::path abcFile3{ abcPath };
    ASSERT_FALSE(abcFile3.empty());
    ASSERT_TRUE(!abcFile3.is_absolute());
    ASSERT_TRUE(!std::filesystem::exists(abcFile3));
    ASSERT_EQ(res, nullptr);

    abcPath = "/log";
    res = window->GetAbcContent(abcPath);
    std::filesystem::path abcFile4{ abcPath };
    ASSERT_FALSE(abcFile4.empty());
    ASSERT_FALSE(!abcFile4.is_absolute());
    if (SceneBoardJudgement::IsSceneBoardEnabled()) {
        ASSERT_FALSE(!std::filesystem::exists(abcFile4));
        ASSERT_NE(res, nullptr);
        std::fstream file(abcFile, std::ios::in | std::ios::binary);
        ASSERT_FALSE(file);
    }
    window->Destroy();
}

/**
 * @tc.name: SetLandscapeMultiWindow
 * @tc.desc: SetLandscapeMultiWindow and check the retCode
 * @tc.type: FUNC
 */
HWTEST_F(WindowSessionImplTest4, SetLandscapeMultiWindow, TestSize.Level1)
{
    sptr<WindowOption> option = sptr<WindowOption>::MakeSptr();
    option->SetWindowName("SetLandscapeMultiWindow");
    sptr<WindowSessionImpl> window = sptr<WindowSessionImpl>::MakeSptr(option);
    WMError retCode = window->SetLandscapeMultiWindow(false);
    ASSERT_EQ(retCode, WMError::WM_ERROR_INVALID_WINDOW);
    window->property_->SetPersistentId(1);
    SessionInfo sessionInfo = { "CreateTestBundle", "CreateTestModule", "CreateTestAbility" };
    sptr<SessionMocker> session = sptr<SessionMocker>::MakeSptr(sessionInfo);
    window->hostSession_ = session;
    window->state_ = WindowState::STATE_CREATED;
    retCode = window->SetLandscapeMultiWindow(false);
    ASSERT_EQ(retCode, WMError::WM_OK);
}

/**
 * @tc.name: GetTouchable
 * @tc.desc: GetTouchable
 * @tc.type: FUNC
 */
HWTEST_F(WindowSessionImplTest4, GetTouchable, TestSize.Level1)
{
    sptr<WindowOption> option = sptr<WindowOption>::MakeSptr();
    option->SetWindowName("GetTouchable");
    sptr<WindowSessionImpl> window = sptr<WindowSessionImpl>::MakeSptr(option);
    window->GetTouchable();
    window->GetBrightness();
    ASSERT_NE(window, nullptr);
}

/**
 * @tc.name: Notify03
 * @tc.desc: NotifyCloseExistPipWindow NotifyAfterResumed NotifyAfterPaused
 * @tc.type: FUNC
 */
HWTEST_F(WindowSessionImplTest4, Notify03, TestSize.Level1)
{
    sptr<WindowOption> option = sptr<WindowOption>::MakeSptr();
    option->SetWindowName("Notify03");
    sptr<WindowSessionImpl> window = sptr<WindowSessionImpl>::MakeSptr(option);

    SessionInfo sessionInfo = { "CreateTestBundle", "CreateTestModule", "CreateTestAbility" };
    sptr<SessionMocker> session = sptr<SessionMocker>::MakeSptr(sessionInfo);
    ASSERT_EQ(WMError::WM_OK, window->Create(nullptr, session));

    window->NotifyAfterResumed();
    window->NotifyAfterPaused();
    WSError res = window->NotifyCloseExistPipWindow();
    ASSERT_EQ(res, WSError::WS_OK);
    AAFwk::WantParams wantParams;
    WSError ret = window->NotifyTransferComponentData(wantParams);
    ASSERT_EQ(ret, WSError::WS_OK);
    ASSERT_EQ(WMError::WM_ERROR_INVALID_WINDOW, window->Destroy());
}

/**
 * @tc.name: Filter
 * @tc.desc: Filter
 * @tc.type: FUNC
 */
HWTEST_F(WindowSessionImplTest4, Filter, TestSize.Level1)
{
    sptr<WindowOption> option = sptr<WindowOption>::MakeSptr();
    option->SetWindowName("Filter");
    sptr<WindowSessionImpl> window = sptr<WindowSessionImpl>::MakeSptr(option);
    std::shared_ptr<MMI::KeyEvent> keyEvent = MMI::KeyEvent::Create();
    window->FilterKeyEvent(keyEvent);
    ASSERT_EQ(window->keyEventFilter_, nullptr);
    window->SetKeyEventFilter([](const MMI::KeyEvent& keyEvent) {
        GTEST_LOG_(INFO) << "WindowSessionImplTest4: SetKeyEventFilter";
        return true;
    });
    ASSERT_NE(window->keyEventFilter_, nullptr);
    window->FilterKeyEvent(keyEvent);
    auto ret = window->ClearKeyEventFilter();
    ASSERT_EQ(ret, WMError::WM_OK);
}

/**
 * @tc.name: UpdateOrientation
 * @tc.desc: UpdateOrientation
 * @tc.type: FUNC
 */
HWTEST_F(WindowSessionImplTest4, UpdateOrientation, TestSize.Level1)
{
    sptr<WindowOption> option = sptr<WindowOption>::MakeSptr();
    option->SetWindowName("UpdateOrientation");
    sptr<WindowSessionImpl> window = sptr<WindowSessionImpl>::MakeSptr(option);
    auto ret = window->UpdateOrientation();
    ASSERT_EQ(WSError::WS_OK, ret);
}

/**
 * @tc.name: SetTitleButtonVisible01
 * @tc.desc: SetTitleButtonVisible
 * @tc.type: FUNC
 */
HWTEST_F(WindowSessionImplTest4, SetTitleButtonVisible01, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "WindowSessionImplTest4: SetTitleButtonVisible01 start";
    sptr<WindowOption> option = sptr<WindowOption>::MakeSptr();
    option->SetWindowName("SetTitleButtonVisible");
    sptr<WindowSessionImpl> window = sptr<WindowSessionImpl>::MakeSptr(option);
    SessionInfo sessionInfo = { "CreateTestBundle", "CreateTestModule", "CreateTestAbility" };
    sptr<SessionMocker> session = sptr<SessionMocker>::MakeSptr(sessionInfo);
    window->hostSession_ = session;
    ASSERT_NE(window->property_, nullptr);
    window->property_->SetPersistentId(1);
    window->property_->SetWindowType(WindowType::WINDOW_TYPE_APP_SUB_WINDOW);
    WMError res = window->SetTitleButtonVisible(false, false, false, true);
    ASSERT_EQ(res, WMError::WM_ERROR_INVALID_CALLING);
    GTEST_LOG_(INFO) << "WindowSessionImplTest4: SetTitleButtonVisible01 end";
}

/**
 * @tc.name: SetTitleButtonVisible02
 * @tc.desc: SetTitleButtonVisible
 * @tc.type: FUNC
 */
HWTEST_F(WindowSessionImplTest4, SetTitleButtonVisible02, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "WindowSessionImplTest4: SetTitleButtonVisible02 start";
    sptr<WindowOption> option = sptr<WindowOption>::MakeSptr();
    option->SetWindowName("SetTitleButtonVisible");
    sptr<WindowSessionImpl> window = sptr<WindowSessionImpl>::MakeSptr(option);
    ASSERT_NE(window->property_, nullptr);
    window->property_->SetWindowType(WindowType::APP_MAIN_WINDOW_BASE);
    WMError res = window->SetTitleButtonVisible(false, false, false, true);
    ASSERT_EQ(res, WMError::WM_ERROR_INVALID_WINDOW);
    GTEST_LOG_(INFO) << "WindowSessionImplTest4: SetTitleButtonVisible02 end";
}

/**
 * @tc.name: SetTitleButtonVisible03
 * @tc.desc: SetTitleButtonVisible
 * @tc.type: FUNC
 */
HWTEST_F(WindowSessionImplTest4, SetTitleButtonVisible03, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "WindowSessionImplTest4: SetTitleButtonVisible03 start";
    sptr option = sptr<WindowOption>::MakeSptr();
    option->SetWindowName("SetTitleButtonVisible");
    sptr window = sptr<WindowSessionImpl>::MakeSptr(option);
    ASSERT_NE(window->property_, nullptr);
    window->property_->SetWindowType(WindowType::APP_MAIN_WINDOW_BASE);
    window->property_->SetWindowMode(WindowMode::WINDOW_MODE_FLOATING);
    window->uiContent_ = std::make_unique<Ace::UIContentMocker>();
    window->windowSystemConfig_.freeMultiWindowSupport_ = true;
    window->windowSystemConfig_.isSystemDecorEnable_ = true;
    window->windowSystemConfig_.windowUIType_ = WindowUIType::PHONE_WINDOW;
    WMError res = window->SetTitleButtonVisible(false, false, false, true);
    ASSERT_EQ(res, WMError::WM_ERROR_INVALID_WINDOW);
    window->windowSystemConfig_.windowUIType_ = WindowUIType::PC_WINDOW;
    res = window->SetTitleButtonVisible(false, false, false, true);
    ASSERT_EQ(res, WMError::WM_ERROR_INVALID_WINDOW);
    GTEST_LOG_(INFO) << "WindowSessionImplTest4: SetTitleButtonVisible03 end";
}

/**
 * @tc.name: GetTitleButtonVisible01
 * @tc.desc: GetTitleButtonVisible
 * @tc.type: FUNC
 */
HWTEST_F(WindowSessionImplTest4, GetTitleButtonVisible01, TestSize.Level1)
{
    sptr<WindowOption> option = sptr<WindowOption>::MakeSptr();
    option->SetWindowName("GetTitleButtonVisible01");
    sptr<WindowSessionImpl> window = sptr<WindowSessionImpl>::MakeSptr(option);
    ASSERT_NE(window->property_, nullptr);
    uint32_t windowModeSupportType = 1 | (1 << 1) | (1 << 2);
    window->property_->SetWindowModeSupportType(windowModeSupportType);
    window->property_->SetWindowMode(WindowMode::WINDOW_MODE_FLOATING);
    // show Maximize, Minimize, Split buttons.
    window->windowTitleVisibleFlags_ = { false, false, false, false };
    bool hideMaximizeButton = false;
    bool hideMinimizeButton = false;
    bool hideSplitButton = false;
    bool hideCloseButton = false;
    window->windowSystemConfig_.windowUIType_ = WindowUIType::PC_WINDOW;
    window->GetTitleButtonVisible(hideMaximizeButton, hideMinimizeButton, hideSplitButton, hideCloseButton);
    ASSERT_EQ(hideMaximizeButton, true);
    ASSERT_EQ(hideMinimizeButton, true);
    ASSERT_EQ(hideSplitButton, true);
    ASSERT_EQ(hideCloseButton, true);
}

/**
 * @tc.name: UpdateRect03
 * @tc.desc: UpdateRect
 * @tc.type: FUNC
 */
HWTEST_F(WindowSessionImplTest4, UpdateRect03, TestSize.Level1)
{
    sptr<WindowOption> option = sptr<WindowOption>::MakeSptr();
    option->SetDisplayId(0);
    option->SetWindowName("WindowSessionCreateCheck");
    sptr<WindowSessionImpl> window = sptr<WindowSessionImpl>::MakeSptr(option);

    WSRect rect;
    rect.posX_ = 0;
    rect.posY_ = 0;
    rect.height_ = 0;
    rect.width_ = 0;

    Rect rectW; // GetRect().IsUninitializedRect is true
    rectW.posX_ = 0;
    rectW.posY_ = 0;
    rectW.height_ = 0; // rectW - rect > 50
    rectW.width_ = 0;  // rectW - rect > 50

    window->property_->SetWindowRect(rectW);
    SizeChangeReason reason = SizeChangeReason::UNDEFINED;
    WSError res = window->UpdateRect(rect, reason);
    ASSERT_EQ(res, WSError::WS_OK);

    rect.height_ = 50;
    rect.width_ = 50;
    rectW.height_ = 50;
    rectW.width_ = 50;
    window->property_->SetWindowRect(rectW);
    res = window->UpdateRect(rect, reason);
    ASSERT_EQ(res, WSError::WS_OK);
}

/**
 * @tc.name: UpdateRect04
 * @tc.desc: UpdateRect
 * @tc.type: FUNC
 */
HWTEST_F(WindowSessionImplTest4, UpdateRect04, TestSize.Level2)
{
    sptr<WindowOption> option = sptr<WindowOption>::MakeSptr();
    option->SetDisplayId(0);
    option->SetWindowName("WindowSessionImplUpdateRect04");
    sptr<WindowSessionImpl> window = sptr<WindowSessionImpl>::MakeSptr(option);
    auto runner = AppExecFwk::EventRunner::Create("UpdateRectForRotation02");
    std::shared_ptr<AppExecFwk::EventHandler> handler = std::make_shared<AppExecFwk::EventHandler>(runner);
    runner->Run();
    window->handler_ = handler;

    WSRect rect;
    rect.posX_ = 0;
    rect.posY_ = 0;
    rect.height_ = 50;
    rect.width_ = 50;
 
    Rect preRect;
    preRect.posX_ = 0;
    preRect.posY_ = 0;
    preRect.height_ = 200;
    preRect.width_ = 200;

    window->property_->SetWindowRect(preRect);
    SizeChangeReason reason = SizeChangeReason::SNAPSHOT_ROTATION;
    WSError res = window->UpdateRect(rect, reason);
    EXPECT_EQ(res, WSError::WS_OK);

    preRect.height_ = 200;
    preRect.width_ = 200;
    window->property_->SetWindowRect(preRect);
    reason = SizeChangeReason::UNDEFINED;
    res = window->UpdateRect(rect, reason);
    EXPECT_EQ(res, WSError::WS_OK);

    window->handler_ = nullptr;
    res = window->UpdateRect(rect, reason);
    EXPECT_EQ(res, WSError::WS_OK);
}

/**
 * @tc.name: GetTitleButtonVisible02
 * @tc.desc: GetTitleButtonVisible
 * @tc.type: FUNC
 */
HWTEST_F(WindowSessionImplTest4, GetTitleButtonVisible02, TestSize.Level1)
{
    g_errLog.clear();
    LOG_SetCallback(MyLogCallback);
    sptr<WindowOption> option = sptr<WindowOption>::MakeSptr();
    option->SetWindowName("GetTitleButtonVisible02");
    sptr<WindowSessionImpl> window = sptr<WindowSessionImpl>::MakeSptr(option);
    // only not support WINDOW_MODE_SUPPORT_SPLIT
    uint32_t windowModeSupportType = 1 | (1 << 1);
    window->property_->SetWindowModeSupportType(windowModeSupportType);
    window->property_->SetWindowMode(WindowMode::WINDOW_MODE_FLOATING);
    // show Maximize, Minimize, Split buttons.
    window->windowTitleVisibleFlags_ = { true, true, true, true };
    bool hideMaximizeButton = true;
    bool hideMinimizeButton = false;
    bool hideSplitButton = false;
    bool hideCloseButton = false;
    window->windowSystemConfig_.windowUIType_ = WindowUIType::PC_WINDOW;
    window->GetTitleButtonVisible(hideMaximizeButton, hideMinimizeButton, hideSplitButton, hideCloseButton);
    EXPECT_TRUE(g_errLog.find("isMaximizeVisible param INVALID") != std::string::npos);
    LOG_SetCallback(nullptr);
}

/**
 * @tc.name: GetTitleButtonVisible03
 * @tc.desc: GetTitleButtonVisible
 * @tc.type: FUNC
 */
HWTEST_F(WindowSessionImplTest4, GetTitleButtonVisible03, TestSize.Level1)
{
    g_errLog.clear();
    LOG_SetCallback(MyLogCallback);
    sptr<WindowOption> option = sptr<WindowOption>::MakeSptr();
    option->SetWindowName("GetTitleButtonVisible03");
    option->SetDisplayId(1);
    sptr<WindowSessionImpl> window = sptr<WindowSessionImpl>::MakeSptr(option);
    // only not support WINDOW_MODE_SUPPORT_SPLIT
    uint32_t windowModeSupportType = 1 | (1 << 1) | (1 << 2);
    window->property_->SetWindowModeSupportType(windowModeSupportType);
    window->property_->SetWindowMode(WindowMode::WINDOW_MODE_FLOATING);
    // show Maximize, Minimize, Split buttons.
    window->windowTitleVisibleFlags_ = { false, false, false, false };
    bool hideMaximizeButton = true;
    bool hideMinimizeButton = true;
    bool hideSplitButton = true;
    bool hideCloseButton = true;
    window->windowSystemConfig_.windowUIType_ = WindowUIType::PHONE_WINDOW;
    window->GetTitleButtonVisible(hideMaximizeButton, hideMinimizeButton, hideSplitButton, hideCloseButton);
    EXPECT_FALSE(g_errLog.find("isMaximizeVisible param INVALID") != std::string::npos);
    LOG_SetCallback(nullptr);
}

/**
 * @tc.name: GetAppForceLandscapeConfig01
 * @tc.desc: GetAppForceLandscapeConfig
 * @tc.type: FUNC
 */
HWTEST_F(WindowSessionImplTest4, GetAppForceLandscapeConfig01, TestSize.Level1)
{
    sptr<WindowOption> option = sptr<WindowOption>::MakeSptr();
    option->SetWindowName("GetAppForceLandscapeConfig01");
    sptr<WindowSessionImpl> window = sptr<WindowSessionImpl>::MakeSptr(option);
    ASSERT_NE(window->property_, nullptr);
    window->property_->SetPersistentId(1);
    SessionInfo sessionInfo = { "CreateTestBundle", "CreateTestModule", "CreateTestAbility" };
    sptr<SessionMocker> session = sptr<SessionMocker>::MakeSptr(sessionInfo);
    window->hostSession_ = session;
    AppForceLandscapeConfig config = {};
    window->GetAppForceLandscapeConfig(config);
    window->hostSession_ = nullptr;
    WMError res = window->GetAppForceLandscapeConfig(config);
    ASSERT_EQ(res, WMError::WM_ERROR_INVALID_WINDOW);
}

/**
 * @tc.name: UpdatePiPControlStatus01
 * @tc.desc: UpdatePiPControlStatus
 * @tc.type: FUNC
 */
HWTEST_F(WindowSessionImplTest4, UpdatePiPControlStatus01, TestSize.Level1)
{
    sptr<WindowOption> option = sptr<WindowOption>::MakeSptr();
    option->SetWindowName("UpdatePiPControlStatus01");
    sptr<WindowSessionImpl> window = sptr<WindowSessionImpl>::MakeSptr(option);
    ASSERT_NE(window->property_, nullptr);
    window->property_->SetPersistentId(1);
    SessionInfo sessionInfo = { "CreateTestBundle", "CreateTestModule", "CreateTestAbility" };
    sptr<SessionMocker> session = sptr<SessionMocker>::MakeSptr(sessionInfo);
    window->hostSession_ = session;
    auto controlType = PiPControlType::VIDEO_PLAY_PAUSE;
    auto status = PiPControlStatus::ENABLED;
    window->UpdatePiPControlStatus(controlType, status);
    window->hostSession_ = nullptr;
    window->UpdatePiPControlStatus(controlType, status);
}

/**
 * @tc.name: SetAutoStartPiP
 * @tc.desc: SetAutoStartPiP
 * @tc.type: FUNC
 */
HWTEST_F(WindowSessionImplTest4, SetAutoStartPiP, TestSize.Level1)
{
    auto option = sptr<WindowOption>::MakeSptr();
    ASSERT_NE(option, nullptr);
    option->SetWindowName("SetAutoStartPiP");
    auto window = sptr<WindowSessionImpl>::MakeSptr(option);
    ASSERT_NE(window, nullptr);
    window->property_->SetPersistentId(1);
    SessionInfo sessionInfo = { "SetAutoStartPiP", "SetAutoStartPiP", "SetAutoStartPiP" };
    auto session = sptr<SessionMocker>::MakeSptr(sessionInfo);
    ASSERT_NE(nullptr, session);
    window->hostSession_ = session;
    bool isAutoStart = true;
    uint32_t priority = 1;
    uint32_t width = 1;
    uint32_t height = 1;
    window->SetAutoStartPiP(isAutoStart, priority, width, height);
    window->hostSession_ = nullptr;
    window->SetAutoStartPiP(isAutoStart, priority, width, height);
}

/**
 * @tc.name: UpdatePiPTemplateInfo
 * @tc.desc: UpdatePiPTemplateInfo
 * @tc.type: FUNC
 */
HWTEST_F(WindowSessionImplTest4, UpdatePiPTemplateInfo, Function | SmallTest | Level2)
{
    auto option = sptr<WindowOption>::MakeSptr();
    ASSERT_NE(option, nullptr);
    std::string testName = "UpdatePiPTemplateInfo";
    option->SetWindowName(testName);
    auto window = sptr<WindowSessionImpl>::MakeSptr(option);
    ASSERT_NE(window, nullptr);
    window->property_->SetPersistentId(1);
    SessionInfo sessionInfo = { testName, testName, testName };
    auto session = sptr<SessionMocker>::MakeSptr(sessionInfo);
    ASSERT_NE(nullptr, session);
    window->hostSession_ = session;
    PiPTemplateInfo templateInfo;
    window->UpdatePiPTemplateInfo(templateInfo);
}

/**
 * @tc.name: NotifyWindowVisibility01
 * @tc.desc: NotifyWindowVisibility
 * @tc.type: FUNC
 */
HWTEST_F(WindowSessionImplTest4, NotifyWindowVisibility01, TestSize.Level1)
{
    sptr<WindowOption> option = sptr<WindowOption>::MakeSptr();
    option->SetWindowName("NotifyWindowVisibility01");
    sptr<WindowSessionImpl> window = sptr<WindowSessionImpl>::MakeSptr(option);
    ASSERT_NE(window->property_, nullptr);
    window->property_->SetPersistentId(1);
    SessionInfo sessionInfo = { "CreateTestBundle", "CreateTestModule", "CreateTestAbility" };
    sptr<SessionMocker> session = sptr<SessionMocker>::MakeSptr(sessionInfo);
    window->hostSession_ = session;
    window->NotifyWindowVisibility(false);
    sptr<IWindowVisibilityChangedListener> listener = sptr<IWindowVisibilityChangedListener>::MakeSptr();
    window->RegisterWindowVisibilityChangeListener(listener);
    window->NotifyWindowVisibility(false);
    window->UnregisterWindowVisibilityChangeListener(listener);
}

/**
 * @tc.name: NotifyExtensionSecureLimitChange01
 * @tc.desc: NotifyExtensionSecureLimitChange
 * @tc.type: FUNC
 */
HWTEST_F(WindowSessionImplTest4, NotifyExtensionSecureLimitChange01, TestSize.Level1)
{
    sptr<WindowOption> option = sptr<WindowOption>::MakeSptr();
    option->SetWindowName("NotifyExtensionSecureLimitChange01");
    sptr<WindowSessionImpl> window = sptr<WindowSessionImpl>::MakeSptr(option);
    ASSERT_NE(window->property_, nullptr);
    window->property_->SetPersistentId(1);
    SessionInfo sessionInfo = { "CreateTestBundle", "CreateTestModule", "CreateTestAbility" };
    sptr<SessionMocker> session = sptr<SessionMocker>::MakeSptr(sessionInfo);
    window->hostSession_ = session;
    sptr<IExtensionSecureLimitChangeListener> listener = sptr<IExtensionSecureLimitChangeListener>::MakeSptr();
    window->RegisterExtensionSecureLimitChangeListener(listener);
    WSError res = window->NotifyExtensionSecureLimitChange(false);
    EXPECT_EQ(res, WSError::WS_OK);
    window->uiContent_ = nullptr;
    res = window->NotifyExtensionSecureLimitChange(false);
    EXPECT_EQ(res, WSError::WS_OK);
    window->uiContent_ = std::make_unique<Ace::UIContentMocker>();
    res = window->NotifyExtensionSecureLimitChange(false);
    EXPECT_EQ(res, WSError::WS_OK);
    window->UnregisterExtensionSecureLimitChangeListener(listener);
}

/**
 * @tc.name: RegisterExtensionSecureLimitChangeListener01
 * @tc.desc: RegisterExtensionSecureLimitChangeListener
 * @tc.type: FUNC
 */
HWTEST_F(WindowSessionImplTest4, RegisterExtensionSecureLimitChangeListener01, TestSize.Level1)
{
    sptr<WindowOption> option = sptr<WindowOption>::MakeSptr();
    option->SetWindowName("RegisterExtensionSecureLimitChangeListener01");
    sptr<WindowSessionImpl> window = sptr<WindowSessionImpl>::MakeSptr(option);
    ASSERT_NE(window->property_, nullptr);
    window->property_->SetPersistentId(1);
    SessionInfo sessionInfo = { "CreateTestBundle", "CreateTestModule", "CreateTestAbility" };
    sptr<SessionMocker> session = sptr<SessionMocker>::MakeSptr(sessionInfo);
    window->hostSession_ = session;

    sptr<IExtensionSecureLimitChangeListener> listener = sptr<IExtensionSecureLimitChangeListener>::MakeSptr();
    WMError res = window->RegisterExtensionSecureLimitChangeListener(listener);
    EXPECT_EQ(res, WMError::WM_OK);
}

/**
 * @tc.name: RegisterExtensionSecureLimitChangeListener02
 * @tc.desc: RegisterExtensionSecureLimitChangeListener
 * @tc.type: FUNC
 */
HWTEST_F(WindowSessionImplTest4, RegisterExtensionSecureLimitChangeListener02, TestSize.Level1)
{
    sptr<WindowOption> option = sptr<WindowOption>::MakeSptr();
    option->SetWindowName("RegisterExtensionSecureLimitChangeListener02");
    sptr<WindowSessionImpl> window = sptr<WindowSessionImpl>::MakeSptr(option);
    ASSERT_NE(window->property_, nullptr);
    window->property_->SetPersistentId(1);
    SessionInfo sessionInfo = { "CreateTestBundle", "CreateTestModule", "CreateTestAbility" };
    sptr<SessionMocker> session = sptr<SessionMocker>::MakeSptr(sessionInfo);
    window->hostSession_ = session;

    sptr<IExtensionSecureLimitChangeListener> listener = nullptr;
    WMError res = window->RegisterExtensionSecureLimitChangeListener(listener);
    EXPECT_EQ(res, WMError::WM_ERROR_NULLPTR);
}

/**
 * @tc.name: UnregisterExtensionSecureLimitChangeListener01
 * @tc.desc: UnregisterExtensionSecureLimitChangeListener
 * @tc.type: FUNC
 */
HWTEST_F(WindowSessionImplTest4, UnregisterExtensionSecureLimitChangeListener01, TestSize.Level1)
{
    sptr<WindowOption> option = sptr<WindowOption>::MakeSptr();
    option->SetWindowName("UnregisterExtensionSecureLimitChangeListener01");
    sptr<WindowSessionImpl> window = sptr<WindowSessionImpl>::MakeSptr(option);
    ASSERT_NE(window->property_, nullptr);
    window->property_->SetPersistentId(1);
    SessionInfo sessionInfo = { "CreateTestBundle", "CreateTestModule", "CreateTestAbility" };
    sptr<SessionMocker> session = sptr<SessionMocker>::MakeSptr(sessionInfo);
    window->hostSession_ = session;

    sptr<IExtensionSecureLimitChangeListener> listener = sptr<IExtensionSecureLimitChangeListener>::MakeSptr();
    WMError res = window->RegisterExtensionSecureLimitChangeListener(listener);
    EXPECT_EQ(res, WMError::WM_OK);
    res = window->UnregisterExtensionSecureLimitChangeListener(listener);
    EXPECT_EQ(res, WMError::WM_OK);
}

/**
 * @tc.name: UnregisterExtensionSecureLimitChangeListener02
 * @tc.desc: UnregisterExtensionSecureLimitChangeListener
 * @tc.type: FUNC
 */
HWTEST_F(WindowSessionImplTest4, UnregisterExtensionSecureLimitChangeListener02, TestSize.Level1)
{
    sptr<WindowOption> option = sptr<WindowOption>::MakeSptr();
    option->SetWindowName("UnregisterExtensionSecureLimitChangeListener02");
    sptr<WindowSessionImpl> window = sptr<WindowSessionImpl>::MakeSptr(option);
    ASSERT_NE(window->property_, nullptr);
    window->property_->SetPersistentId(1);
    SessionInfo sessionInfo = { "CreateTestBundle", "CreateTestModule", "CreateTestAbility" };
    sptr<SessionMocker> session = sptr<SessionMocker>::MakeSptr(sessionInfo);
    window->hostSession_ = session;

    sptr<IExtensionSecureLimitChangeListener> listener = nullptr;
    WMError res = window->UnregisterExtensionSecureLimitChangeListener(listener);
    EXPECT_EQ(res, WMError::WM_ERROR_NULLPTR);
}

/**
 * @tc.name: UpdateVirtualPixelRatio
 * @tc.desc: test UpdateVirtualPixelRatio
 * @tc.type: FUNC
 */
HWTEST_F(WindowSessionImplTest4, UpdateVirtualPixelRatio, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "WindowSessionImplTest4: UpdateVirtualPixelRatio start";
    sptr<WindowOption> option = sptr<WindowOption>::MakeSptr();
    option->SetWindowName("UpdateVirtualPixelRatio");
    sptr<WindowSessionImpl> window = sptr<WindowSessionImpl>::MakeSptr(option);
    window->property_->SetWindowType(WindowType::APP_MAIN_WINDOW_BASE);
    window->property_->SetWindowMode(WindowMode::WINDOW_MODE_FLOATING);

    window->property_->SetDisplayId(-1);
    sptr<Display> display = nullptr;
    window->UpdateVirtualPixelRatio(display);
    ASSERT_EQ(window->virtualPixelRatio_, 1.0f);

    window->property_->SetDisplayId(0);
    display = SingletonContainer::Get<DisplayManager>().GetDisplayById(window->property_->GetDisplayId());
    window->UpdateVirtualPixelRatio(display);
    ASSERT_NE(window->virtualPixelRatio_, 1.0f);
    GTEST_LOG_(INFO) << "WindowSessionImplTest4: UpdateVirtualPixelRatio end";
}

/**
 * @tc.name: NotifyMainWindowClose01
 * @tc.desc: NotifyMainWindowClose
 * @tc.type: FUNC
 */
HWTEST_F(WindowSessionImplTest4, NotifyMainWindowClose01, TestSize.Level1)
{
    sptr<WindowOption> option = sptr<WindowOption>::MakeSptr();
    option->SetWindowName("NotifyMainWindowClose01");
    sptr<WindowSessionImpl> window = sptr<WindowSessionImpl>::MakeSptr(option);
    SessionInfo sessionInfo = { "CreateTestBundle", "CreateTestModule", "CreateTestAbility" };
    sptr<SessionMocker> session = sptr<SessionMocker>::MakeSptr(sessionInfo);
    window->hostSession_ = session;
    window->property_->SetPersistentId(1);
    window->windowSystemConfig_.windowUIType_ = WindowUIType::PC_WINDOW;
    window->property_->SetWindowType(WindowType::APP_MAIN_WINDOW_BASE);

    bool terminateCloseProcess = false;
    WMError res = window->NotifyMainWindowClose(terminateCloseProcess);
    EXPECT_EQ(res, WMError::WM_ERROR_NULLPTR);
    sptr<IMainWindowCloseListener> listener = sptr<IMainWindowCloseListener>::MakeSptr();
    EXPECT_EQ(window->RegisterMainWindowCloseListeners(listener), WMError::WM_OK);
    res = window->NotifyMainWindowClose(terminateCloseProcess);
    EXPECT_EQ(res, WMError::WM_OK);
    EXPECT_EQ(window->UnregisterMainWindowCloseListeners(listener), WMError::WM_OK);
}

/**
 * @tc.name: NotifyWindowWillClose
 * @tc.desc: NotifyWindowWillClose
 * @tc.type: FUNC
 */
HWTEST_F(WindowSessionImplTest4, NotifyWindowWillClose, TestSize.Level1)
{
    sptr<WindowOption> option = sptr<WindowOption>::MakeSptr();
    option->SetWindowName("NotifyWindowWillClose");
    sptr<WindowSessionImpl> window = sptr<WindowSessionImpl>::MakeSptr(option);
    SessionInfo sessionInfo = { "CreateTestBundle", "CreateTestModule", "CreateTestAbility" };
    sptr<SessionMocker> session = sptr<SessionMocker>::MakeSptr(sessionInfo);
    window->hostSession_ = session;
    window->property_->SetPersistentId(1);
    WMError res = window->NotifyWindowWillClose(window);
    EXPECT_EQ(res, WMError::WM_ERROR_NULLPTR);

    sptr<IWindowWillCloseListener> listener = sptr<MockIWindowWillCloseListener>::MakeSptr();
    window->windowSystemConfig_.windowUIType_ = WindowUIType::PC_WINDOW;
    window->property_->SetWindowType(WindowType::APP_MAIN_WINDOW_BASE);
    res = window->RegisterWindowWillCloseListeners(listener);
    EXPECT_EQ(res, WMError::WM_OK);
    res = window->NotifyWindowWillClose(window);
    EXPECT_EQ(res, WMError::WM_OK);
    res = window->UnRegisterWindowWillCloseListeners(listener);
    EXPECT_EQ(res, WMError::WM_OK);
    res = window->NotifyWindowWillClose(window);
    EXPECT_EQ(res, WMError::WM_ERROR_NULLPTR);
}

/**
 * @tc.name: IsPcOrFreeMultiWindowCapabilityEnabled
 * @tc.desc: IsPcOrFreeMultiWindowCapabilityEnabled test
 * @tc.type: FUNC
 */
HWTEST_F(WindowSessionImplTest4, IsPcOrFreeMultiWindowCapabilityEnabled, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "WindowSessionImplTest4: IsPcOrFreeMultiWindowCapabilityEnabled start";
    sptr<WindowOption> option = sptr<WindowOption>::MakeSptr();
    option->SetWindowName("IsPcOrFreeMultiWindowCapabilityEnabled");
    sptr<WindowSessionImpl> window = sptr<WindowSessionImpl>::MakeSptr(option);
    ASSERT_NE(window->property_, nullptr);
    window->property_->SetPersistentId(1);
    window->property_->SetWindowType(WindowType::APP_MAIN_WINDOW_BASE);
    window->property_->SetWindowMode(WindowMode::WINDOW_MODE_FLOATING);
    SessionInfo sessionInfo = { "CreateTestBundle", "CreateTestModule", "CreateTestAbility" };
    sptr<SessionMocker> session = sptr<SessionMocker>::MakeSptr(sessionInfo);
    ASSERT_NE(nullptr, session);
    window->hostSession_ = session;

    window->windowSystemConfig_.windowUIType_ = WindowUIType::PC_WINDOW;
    EXPECT_EQ(true, window->IsPcOrFreeMultiWindowCapabilityEnabled());
    window->windowSystemConfig_.windowUIType_ = WindowUIType::PHONE_WINDOW;
    EXPECT_EQ(false, window->IsPcOrFreeMultiWindowCapabilityEnabled());
    window->windowSystemConfig_.windowUIType_ = WindowUIType::PAD_WINDOW;
    EXPECT_EQ(false, window->IsPcOrFreeMultiWindowCapabilityEnabled());
    window->property_->SetIsPcAppInPad(true);
    EXPECT_EQ(true, window->IsPcOrFreeMultiWindowCapabilityEnabled());
    EXPECT_EQ(WMError::WM_OK, window->Destroy(true));
    GTEST_LOG_(INFO) << "WindowSessionImplTest4: IsPcOrFreeMultiWindowCapabilityEnabled end";
}

/**
 * @tc.name: DestroySubWindow
 * @tc.desc: DestroySubWindow test
 * @tc.type: FUNC
 */
HWTEST_F(WindowSessionImplTest4, DestroySubWindow, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "WindowSessionImplTest4: DestroySubWindow start";
    sptr<WindowOption> option = sptr<WindowOption>::MakeSptr();
    option->SetWindowName("DestroySubWindow");
    option->SetWindowType(WindowType::APP_MAIN_WINDOW_BASE);
    sptr<WindowSessionImpl> window = sptr<WindowSessionImpl>::MakeSptr(option);
    ASSERT_NE(window->property_, nullptr);
    window->property_->SetPersistentId(1);
    SessionInfo sessionInfo = { "CreateTestBundle", "CreateTestModule", "CreateTestAbility" };
    sptr<SessionMocker> session = sptr<SessionMocker>::MakeSptr(sessionInfo);
    window->hostSession_ = session;
    window->windowSystemConfig_.windowUIType_ = WindowUIType::PC_WINDOW;

    sptr<WindowOption> subOption = sptr<WindowOption>::MakeSptr();
    subOption->SetWindowName("DestroySubWindow01");
    subOption->SetWindowType(WindowType::APP_SUB_WINDOW_BASE);
    sptr<WindowSessionImpl> subWindow = sptr<WindowSessionImpl>::MakeSptr(subOption);
    ASSERT_NE(subWindow->property_, nullptr);
    subWindow->property_->SetPersistentId(2);
    SessionInfo subSessionInfo = { "CreateTestBundle", "CreateTestModule", "CreateTestAbility" };
    sptr<SessionMocker> subSession = sptr<SessionMocker>::MakeSptr(subSessionInfo);
    subWindow->hostSession_ = subSession;
    subWindow->windowSystemConfig_.windowUIType_ = WindowUIType::PC_WINDOW;
    std::vector<sptr<WindowSessionImpl>> vec;
    WindowSessionImpl::subWindowSessionMap_.insert(std::pair<int32_t, std::vector<sptr<WindowSessionImpl>>>(1, vec));
    WindowSessionImpl::subWindowSessionMap_[1].push_back(subWindow);
    window->DestroySubWindow();
    EXPECT_EQ(WMError::WM_OK, window->Destroy(true));
}

/**
 * @tc.name: UpdateSubWindowStateAndNotify01
 * @tc.desc: UpdateSubWindowStateAndNotify
 * @tc.type: FUNC
 */
HWTEST_F(WindowSessionImplTest4, UpdateSubWindowStateAndNotify01, TestSize.Level1)
{
    sptr<WindowOption> option = sptr<WindowOption>::MakeSptr();
    option->SetWindowName("UpdateSubWindowStateAndNotify01");
    option->SetWindowType(WindowType::APP_MAIN_WINDOW_BASE);
    sptr<WindowSessionImpl> window = sptr<WindowSessionImpl>::MakeSptr(option);
    ASSERT_NE(window->property_, nullptr);
    window->property_->SetPersistentId(1);
    SessionInfo sessionInfo = { "CreateTestBundle", "CreateTestModule", "CreateTestAbility" };
    sptr<SessionMocker> session = sptr<SessionMocker>::MakeSptr(sessionInfo);
    window->hostSession_ = session;
    window->windowSystemConfig_.windowUIType_ = WindowUIType::PC_WINDOW;

    sptr<WindowOption> subOption = sptr<WindowOption>::MakeSptr();
    subOption->SetWindowName("UpdateSubWindowStateAndNotify011");
    subOption->SetWindowType(WindowType::APP_SUB_WINDOW_BASE);
    sptr<WindowSessionImpl> subWindow = sptr<WindowSessionImpl>::MakeSptr(subOption);
    ASSERT_NE(subWindow->property_, nullptr);
    subWindow->property_->SetPersistentId(2);
    SessionInfo subSessionInfo = { "CreateTestBundle", "CreateTestModule", "CreateTestAbility" };
    sptr<SessionMocker> subSession = sptr<SessionMocker>::MakeSptr(subSessionInfo);
    ASSERT_NE(nullptr, subSession);
    subWindow->hostSession_ = subSession;
    subWindow->windowSystemConfig_.windowUIType_ = WindowUIType::PC_WINDOW;
    std::vector<sptr<WindowSessionImpl>> vec;
    WindowSessionImpl::subWindowSessionMap_.insert(std::pair<int32_t, std::vector<sptr<WindowSessionImpl>>>(1, vec));
    subWindow->UpdateSubWindowStateAndNotify(1, WindowState::STATE_HIDDEN);
    WindowSessionImpl::subWindowSessionMap_[1].push_back(subWindow);
    subWindow->state_ = WindowState::STATE_SHOWN;
    window->UpdateSubWindowStateAndNotify(1, WindowState::STATE_HIDDEN);
    window->state_ = WindowState::STATE_HIDDEN;
    window->UpdateSubWindowStateAndNotify(1, WindowState::STATE_HIDDEN);
    window->state_ = WindowState::STATE_SHOWN;
    window->UpdateSubWindowStateAndNotify(1, WindowState::STATE_SHOWN);
    window->state_ = WindowState::STATE_SHOWN;
    window->UpdateSubWindowStateAndNotify(1, WindowState::STATE_SHOWN);
    EXPECT_EQ(WMError::WM_OK, subWindow->Destroy(true));
    EXPECT_EQ(WMError::WM_OK, window->Destroy(true));
}

/**
 * @tc.name: NotifyRotationAnimationEnd
 * @tc.desc: test NotifyRotationAnimationEnd
 * @tc.type: FUNC
 */
HWTEST_F(WindowSessionImplTest4, NotifyRotationAnimationEnd001, TestSize.Level1)
{
    sptr<WindowOption> option = sptr<WindowOption>::MakeSptr();
    option->SetWindowName("NotifyRotationAnimationEnd001");
    sptr<WindowSessionImpl> window = sptr<WindowSessionImpl>::MakeSptr(option);

    ASSERT_NE(window->handler_, nullptr);
    window->uiContent_ = std::make_unique<Ace::UIContentMocker>();
    ASSERT_NE(window->uiContent_, nullptr);
    window->NotifyRotationAnimationEnd();

    window->uiContent_ = nullptr;
    ASSERT_EQ(window->uiContent_, nullptr);
    window->NotifyRotationAnimationEnd();

    window->handler_ = nullptr;
    ASSERT_EQ(window->handler_, nullptr);
    window->NotifyRotationAnimationEnd();
}

/**
 * @tc.name: SetEnableDragBySystem
 * @tc.desc: test SetEnableDragBySystem
 * @tc.type: FUNC
 */
HWTEST_F(WindowSessionImplTest4, SetEnableDragBySystem, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "WindowSessionImplTest4: GetSubWindow start";
    sptr<WindowOption> option = sptr<WindowOption>::MakeSptr();
    option->SetWindowName("GetSubWindow");
    sptr<WindowSessionImpl> window = sptr<WindowSessionImpl>::MakeSptr(option);
    window->property_->SetDragEnabled(true);
    window->SetEnableDragBySystem(false);
    ASSERT_FALSE(window->property_->GetDragEnabled());
}

/**
 * @tc.name: GetSubWindow
 * @tc.desc: test GetSubWindow
 * @tc.type: FUNC
 */
HWTEST_F(WindowSessionImplTest4, GetSubWindow, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "WindowSessionImplTest4: GetSubWindow start";
    sptr<WindowOption> option = sptr<WindowOption>::MakeSptr();
    option->SetWindowName("GetSubWindow");
    sptr<WindowSessionImpl> window = sptr<WindowSessionImpl>::MakeSptr(option);
    ASSERT_NE(nullptr, window);
    ASSERT_NE(nullptr, window->property_);
    window->property_->SetWindowType(WindowType::WINDOW_TYPE_APP_SUB_WINDOW);
    window->property_->SetParentId(101);                              // this subWindow's parentId is 101
    std::vector<sptr<Window>> subWindows = window->GetSubWindow(101); // 101 is parentId
    ASSERT_EQ(0, subWindows.size());
    WindowSessionImpl::subWindowSessionMap_.insert(
        std::pair<int32_t, std::vector<sptr<WindowSessionImpl>>>(101, { window }));
    subWindows = window->GetSubWindow(101); // 101 is parentId
    ASSERT_EQ(1, subWindows.size());
    WindowSessionImpl::subWindowSessionMap_.erase(101); // 101
    GTEST_LOG_(INFO) << "WindowSessionImplTest4: GetVirtualPixelRatio end";
}

/**
 * @tc.name: ClearListenersById_displayMoveListeners
 * @tc.desc: ClearListenersById_displayMoveListeners
 * @tc.type: FUNC
 */
HWTEST_F(WindowSessionImplTest4, ClearListenersById_displayMoveListeners, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "WindowSessionImplTest4: ClearListenersById_displayMoveListeners start";
    sptr<WindowOption> option_ = sptr<WindowOption>::MakeSptr();
    option_->SetWindowName("ClearListenersById_displayMoveListeners");
    sptr<WindowSessionImpl> window_ = sptr<WindowSessionImpl>::MakeSptr(option_);

    int persistentId = window_->GetPersistentId();
    window_->ClearListenersById(persistentId);

    sptr<IDisplayMoveListener> listener_ = new (std::nothrow) MockIDisplayMoveListener();
    window_->RegisterDisplayMoveListener(listener_);
    ASSERT_NE(window_->displayMoveListeners_.find(persistentId), window_->displayMoveListeners_.end());

    window_->ClearListenersById(persistentId);
    ASSERT_EQ(window_->displayMoveListeners_.find(persistentId), window_->displayMoveListeners_.end());

    GTEST_LOG_(INFO) << "WindowSessionImplTest4: ClearListenersById_displayMoveListeners end";
}

/**
 * @tc.name: ClearListenersById_lifecycleListeners
 * @tc.desc: ClearListenersById_lifecycleListeners
 * @tc.type: FUNC
 */
HWTEST_F(WindowSessionImplTest4, ClearListenersById_lifecycleListeners, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "WindowSessionImplTest4: ClearListenersById_lifecycleListeners start";
    sptr<WindowOption> option_ = sptr<WindowOption>::MakeSptr();
    option_->SetWindowName("ClearListenersById_lifecycleListeners");
    sptr<WindowSessionImpl> window_ = sptr<WindowSessionImpl>::MakeSptr(option_);

    int persistentId = window_->GetPersistentId();
    window_->ClearListenersById(persistentId);

    sptr<IWindowLifeCycle> listener_ = new (std::nothrow) MockWindowLifeCycleListener();
    window_->RegisterLifeCycleListener(listener_);
    ASSERT_NE(window_->lifecycleListeners_.find(persistentId), window_->lifecycleListeners_.end());

    window_->ClearListenersById(persistentId);
    ASSERT_EQ(window_->lifecycleListeners_.find(persistentId), window_->lifecycleListeners_.end());

    GTEST_LOG_(INFO) << "WindowSessionImplTest4: ClearListenersById_lifecycleListeners end";
}

/**
 * @tc.name: ClearListenersById_windowChangeListeners
 * @tc.desc: ClearListenersById_windowChangeListeners
 * @tc.type: FUNC
 */
HWTEST_F(WindowSessionImplTest4, ClearListenersById_windowChangeListeners, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "WindowSessionImplTest4: ClearListenersById_windowChangeListeners start";
    sptr<WindowOption> option_ = sptr<WindowOption>::MakeSptr();
    option_->SetWindowName("ClearListenersById_windowChangeListeners");
    sptr<WindowSessionImpl> window_ = sptr<WindowSessionImpl>::MakeSptr(option_);

    int persistentId = window_->GetPersistentId();
    window_->ClearListenersById(persistentId);

    sptr<IWindowChangeListener> listener_ = new (std::nothrow) MockWindowChangeListener();
    window_->RegisterWindowChangeListener(listener_);
    ASSERT_NE(window_->windowChangeListeners_.find(persistentId), window_->windowChangeListeners_.end());

    window_->ClearListenersById(persistentId);
    ASSERT_EQ(window_->windowChangeListeners_.find(persistentId), window_->windowChangeListeners_.end());

    GTEST_LOG_(INFO) << "WindowSessionImplTest4: ClearListenersById_windowChangeListeners end";
}

/**
 * @tc.name: ClearListenersById_avoidAreaChangeListeners
 * @tc.desc: ClearListenersById_avoidAreaChangeListeners
 * @tc.type: FUNC
 */
HWTEST_F(WindowSessionImplTest4, ClearListenersById_avoidAreaChangeListeners, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "WindowSessionImplTest4: ClearListenersById_avoidAreaChangeListeners start";
    sptr<WindowOption> option_ = sptr<WindowOption>::MakeSptr();
    option_->SetWindowName("ClearListenersById_avoidAreaChangeListeners");
    sptr<WindowSessionImpl> window_ = sptr<WindowSessionImpl>::MakeSptr(option_);

    int persistentId = window_->GetPersistentId();
    window_->ClearListenersById(persistentId);

    sptr<IAvoidAreaChangedListener> listener_ = new (std::nothrow) MockAvoidAreaChangedListener();
    window_->RegisterExtensionAvoidAreaChangeListener(listener_);
    ASSERT_NE(window_->avoidAreaChangeListeners_.find(persistentId), window_->avoidAreaChangeListeners_.end());

    window_->ClearListenersById(persistentId);
    ASSERT_EQ(window_->avoidAreaChangeListeners_.find(persistentId), window_->avoidAreaChangeListeners_.end());

    GTEST_LOG_(INFO) << "WindowSessionImplTest4: ClearListenersById_avoidAreaChangeListeners end";
}

/**
 * @tc.name: ClearListenersById_dialogDeathRecipientListeners
 * @tc.desc: ClearListenersById_dialogDeathRecipientListeners
 * @tc.type: FUNC
 */
HWTEST_F(WindowSessionImplTest4, ClearListenersById_dialogDeathRecipientListeners, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "WindowSessionImplTest4: ClearListenersById_dialogDeathRecipientListeners start";
    sptr<WindowOption> option_ = sptr<WindowOption>::MakeSptr();
    option_->SetWindowName("ClearListenersById_dialogDeathRecipientListeners");
    sptr<WindowSessionImpl> window_ = sptr<WindowSessionImpl>::MakeSptr(option_);

    int persistentId = window_->GetPersistentId();
    window_->ClearListenersById(persistentId);

    sptr<IDialogDeathRecipientListener> listener_ = new (std::nothrow) MockIDialogDeathRecipientListener();
    window_->RegisterDialogDeathRecipientListener(listener_);
    ASSERT_NE(window_->dialogDeathRecipientListeners_.find(persistentId),
              window_->dialogDeathRecipientListeners_.end());

    window_->ClearListenersById(persistentId);
    ASSERT_EQ(window_->dialogDeathRecipientListeners_.find(persistentId),
              window_->dialogDeathRecipientListeners_.end());

    GTEST_LOG_(INFO) << "WindowSessionImplTest4: ClearListenersById_dialogDeathRecipientListeners end";
}

/**
 * @tc.name: ClearListenersById_dialogTargetTouchListener
 * @tc.desc: ClearListenersById_dialogTargetTouchListener
 * @tc.type: FUNC
 */
HWTEST_F(WindowSessionImplTest4, ClearListenersById_dialogTargetTouchListener, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "WindowSessionImplTest4: ClearListenersById_dialogTargetTouchListener start";
    sptr<WindowOption> option_ = sptr<WindowOption>::MakeSptr();
    option_->SetWindowName("ClearListenersById_dialogTargetTouchListener");
    sptr<WindowSessionImpl> window_ = sptr<WindowSessionImpl>::MakeSptr(option_);

    int persistentId = window_->GetPersistentId();
    window_->ClearListenersById(persistentId);

    sptr<IDialogTargetTouchListener> listener_ = new (std::nothrow) MockIDialogTargetTouchListener();
    window_->RegisterDialogTargetTouchListener(listener_);
    ASSERT_NE(window_->dialogTargetTouchListener_.find(persistentId), window_->dialogTargetTouchListener_.end());

    window_->ClearListenersById(persistentId);
    ASSERT_EQ(window_->dialogTargetTouchListener_.find(persistentId), window_->dialogTargetTouchListener_.end());

    GTEST_LOG_(INFO) << "WindowSessionImplTest4: ClearListenersById_dialogTargetTouchListener end";
}

/**
 * @tc.name: ClearListenersById_screenshotListeners
 * @tc.desc: ClearListenersById_screenshotListeners
 * @tc.type: FUNC
 */
HWTEST_F(WindowSessionImplTest4, ClearListenersById_screenshotListeners, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "WindowSessionImplTest4: ClearListenersById_screenshotListeners start";
    sptr<WindowOption> option_ = sptr<WindowOption>::MakeSptr();
    option_->SetWindowName("ClearListenersById_screenshotListeners");
    sptr<WindowSessionImpl> window_ = sptr<WindowSessionImpl>::MakeSptr(option_);

    int persistentId = window_->GetPersistentId();
    window_->ClearListenersById(persistentId);

    sptr<IScreenshotListener> listener_ = new (std::nothrow) MockIScreenshotListener();
    window_->RegisterScreenshotListener(listener_);
    ASSERT_NE(window_->screenshotListeners_.find(persistentId), window_->screenshotListeners_.end());

    window_->ClearListenersById(persistentId);
    ASSERT_EQ(window_->screenshotListeners_.find(persistentId), window_->screenshotListeners_.end());

    GTEST_LOG_(INFO) << "WindowSessionImplTest4: ClearListenersById_screenshotListeners end";
}

/**
 * @tc.name: ClearListenersById_windowStatusChangeListeners
 * @tc.desc: ClearListenersById_windowStatusChangeListeners
 * @tc.type: FUNC
 */
HWTEST_F(WindowSessionImplTest4, ClearListenersById_windowStatusChangeListeners, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "WindowSessionImplTest4: ClearListenersById_windowStatusChangeListeners start";
    sptr<WindowOption> option_ = sptr<WindowOption>::MakeSptr();
    option_->SetWindowName("ClearListenersById_windowStatusChangeListeners");
    sptr<WindowSessionImpl> window_ = sptr<WindowSessionImpl>::MakeSptr(option_);

    int persistentId = window_->GetPersistentId();
    window_->ClearListenersById(persistentId);

    sptr<IWindowStatusChangeListener> listener_ = new (std::nothrow) MockWindowStatusChangeListener();
    window_->RegisterWindowStatusChangeListener(listener_);
    ASSERT_NE(window_->windowStatusChangeListeners_.find(persistentId), window_->windowStatusChangeListeners_.end());

    window_->ClearListenersById(persistentId);
    ASSERT_EQ(window_->windowStatusChangeListeners_.find(persistentId), window_->windowStatusChangeListeners_.end());

    GTEST_LOG_(INFO) << "WindowSessionImplTest4: ClearListenersById_windowStatusChangeListeners end";
}

/**
 * @tc.name: ClearListenersById_windowTitleButtonRectChangeListeners
 * @tc.desc: ClearListenersById_windowTitleButtonRectChangeListeners
 * @tc.type: FUNC
 */
HWTEST_F(WindowSessionImplTest4, ClearListenersById_windowTitleButtonRectChangeListeners, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "WindowSessionImplTest4: ClearListenersById_windowTitleButtonRectChangeListeners start";
    sptr<WindowOption> option_ = sptr<WindowOption>::MakeSptr();
    option_->SetWindowName("ClearListenersById_windowTitleButtonRectChangeListeners");
    sptr<WindowSessionImpl> window_ = sptr<WindowSessionImpl>::MakeSptr(option_);

    int persistentId = window_->GetPersistentId();
    window_->ClearListenersById(persistentId);

    sptr<IWindowTitleButtonRectChangedListener> listener_ =
        new (std::nothrow) MockWindowTitleButtonRectChangedListener();
    window_->windowTitleButtonRectChangeListeners_[persistentId].emplace_back(listener_);
    ASSERT_NE(window_->windowTitleButtonRectChangeListeners_.find(persistentId),
              window_->windowTitleButtonRectChangeListeners_.end());

    window_->ClearListenersById(persistentId);
    ASSERT_EQ(window_->windowTitleButtonRectChangeListeners_.find(persistentId),
              window_->windowTitleButtonRectChangeListeners_.end());

    GTEST_LOG_(INFO) << "WindowSessionImplTest4: ClearListenersById_windowTitleButtonRectChangeListeners end";
}

/**
 * @tc.name: ClearListenersById_windowNoInteractionListeners
 * @tc.desc: ClearListenersById_windowNoInteractionListeners
 * @tc.type: FUNC
 */
HWTEST_F(WindowSessionImplTest4, ClearListenersById_windowNoInteractionListeners, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "WindowSessionImplTest4: ClearListenersById_windowNoInteractionListeners start";
    sptr<WindowOption> option_ = sptr<WindowOption>::MakeSptr();
    option_->SetWindowName("ClearListenersById_windowNoInteractionListeners");
    sptr<WindowSessionImpl> window_ = sptr<WindowSessionImpl>::MakeSptr(option_);

    int persistentId = window_->GetPersistentId();
    window_->ClearListenersById(persistentId);

    sptr<IWindowNoInteractionListener> listener_ = new (std::nothrow) MockWindowNoInteractionListener();
    window_->RegisterWindowNoInteractionListener(listener_);
    ASSERT_NE(window_->windowNoInteractionListeners_.find(persistentId), window_->windowNoInteractionListeners_.end());

    window_->ClearListenersById(persistentId);
    ASSERT_EQ(window_->windowNoInteractionListeners_.find(persistentId), window_->windowNoInteractionListeners_.end());

    GTEST_LOG_(INFO) << "WindowSessionImplTest4: ClearListenersById_windowNoInteractionListeners end";
}

/**
 * @tc.name: ClearListenersById_windowRectChangeListeners
 * @tc.desc: ClearListenersById_windowRectChangeListeners
 * @tc.type: FUNC
 */
HWTEST_F(WindowSessionImplTest4, ClearListenersById_windowRectChangeListeners, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "WindowSessionImplTest4: ClearListenersById_windowRectChangeListeners start";
    sptr<WindowOption> option_ = sptr<WindowOption>::MakeSptr();
    option_->SetWindowName("ClearListenersById_windowRectChangeListeners");
    sptr<WindowSessionImpl> window_ = sptr<WindowSessionImpl>::MakeSptr(option_);

    int persistentId = window_->GetPersistentId();
    window_->ClearListenersById(persistentId);

    sptr<IWindowRectChangeListener> listener_ = new (std::nothrow) MockWindowRectChangeListener();
    window_->RegisterWindowRectChangeListener(listener_);
    ASSERT_NE(window_->windowRectChangeListeners_.find(persistentId), window_->windowRectChangeListeners_.end());

    window_->ClearListenersById(persistentId);
    ASSERT_EQ(window_->windowRectChangeListeners_.find(persistentId), window_->windowRectChangeListeners_.end());

    GTEST_LOG_(INFO) << "WindowSessionImplTest4: ClearListenersById_windowRectChangeListeners end";
}

/**
 * @tc.name: ClearListenersById_subWindowCloseListeners
 * @tc.desc: ClearListenersById_subWindowCloseListeners
 * @tc.type: FUNC
 */
HWTEST_F(WindowSessionImplTest4, ClearListenersById_subWindowCloseListeners, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "WindowSessionImplTest4: ClearListenersById_subWindowCloseListeners start";
    sptr<WindowOption> option_ = sptr<WindowOption>::MakeSptr();
    option_->SetWindowName("ClearListenersById_subWindowCloseListeners");
    sptr<WindowSessionImpl> window_ = sptr<WindowSessionImpl>::MakeSptr(option_);

    int persistentId = window_->GetPersistentId();
    window_->ClearListenersById(persistentId);

    sptr<ISubWindowCloseListener> listener_ = new (std::nothrow) MockISubWindowCloseListener();
    window_->subWindowCloseListeners_[persistentId] = listener_;
    ASSERT_NE(window_->subWindowCloseListeners_.find(persistentId), window_->subWindowCloseListeners_.end());

    window_->ClearListenersById(persistentId);
    ASSERT_EQ(window_->subWindowCloseListeners_.find(persistentId), window_->subWindowCloseListeners_.end());

    GTEST_LOG_(INFO) << "WindowSessionImplTest4: ClearListenersById_subWindowCloseListeners end";
}

/**
 * @tc.name: ClearListenersById_mainWindowCloseListeners
 * @tc.desc: ClearListenersById_mainWindowCloseListeners
 * @tc.type: FUNC
 */
HWTEST_F(WindowSessionImplTest4, ClearListenersById_mainWindowCloseListeners, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "WindowSessionImplTest4: ClearListenersById_mainWindowCloseListeners start";
    sptr<WindowOption> option_ = sptr<WindowOption>::MakeSptr();
    option_->SetWindowName("ClearListenersById_mainWindowCloseListeners");
    sptr<WindowSessionImpl> window_ = sptr<WindowSessionImpl>::MakeSptr(option_);

    int persistentId = window_->GetPersistentId();
    window_->ClearListenersById(persistentId);

    sptr<IMainWindowCloseListener> listener_ = new (std::nothrow) MockIMainWindowCloseListener();
    window_->mainWindowCloseListeners_[persistentId] = listener_;
    ASSERT_NE(window_->mainWindowCloseListeners_.find(persistentId), window_->mainWindowCloseListeners_.end());

    window_->ClearListenersById(persistentId);
    ASSERT_EQ(window_->mainWindowCloseListeners_.find(persistentId), window_->mainWindowCloseListeners_.end());

    GTEST_LOG_(INFO) << "WindowSessionImplTest4: ClearListenersById_mainWindowCloseListeners end";
}

/**
 * @tc.name: ClearListenersById_windowWillCloseListeners
 * @tc.desc: ClearListenersById_windowWillCloseListeners
 * @tc.type: FUNC
 */
HWTEST_F(WindowSessionImplTest4, ClearListenersById_windowWillCloseListeners, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "WindowSessionImplTest4: ClearListenersById_windowWillCloseListeners start";
    sptr<WindowOption> option_ = sptr<WindowOption>::MakeSptr();
    option_->SetWindowName("ClearListenersById_windowWillCloseListeners");
    sptr<WindowSessionImpl> window_ = sptr<WindowSessionImpl>::MakeSptr(option_);
    SessionInfo sessionInfo = { "CreateTestBundle", "CreateTestModule", "CreateTestAbility" };
    sptr<SessionMocker> session = sptr<SessionMocker>::MakeSptr(sessionInfo);
    window_->hostSession_ = session;
    window_->property_->SetPersistentId(1);
    int persistentId = window_->GetPersistentId();
    window_->ClearListenersById(persistentId);

    sptr<IWindowWillCloseListener> listener_ = sptr<MockIWindowWillCloseListener>::MakeSptr();
    window_->windowSystemConfig_.windowUIType_ = WindowUIType::PC_WINDOW;
    ASSERT_EQ(WMError::WM_OK, window_->RegisterWindowWillCloseListeners(listener_));
    ASSERT_NE(window_->windowWillCloseListeners_.find(persistentId), window_->windowWillCloseListeners_.end());

    window_->ClearListenersById(persistentId);
    ASSERT_EQ(window_->windowWillCloseListeners_.find(persistentId), window_->windowWillCloseListeners_.end());

    GTEST_LOG_(INFO) << "WindowSessionImplTest4: ClearListenersById_windowWillCloseListeners end";
}

/**
 * @tc.name: ClearListenersById_occupiedAreaChangeListeners
 * @tc.desc: ClearListenersById_occupiedAreaChangeListeners
 * @tc.type: FUNC
 */
HWTEST_F(WindowSessionImplTest4, ClearListenersById_occupiedAreaChangeListeners, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "WindowSessionImplTest4: ClearListenersById_occupiedAreaChangeListeners start";
    sptr<WindowOption> option_ = sptr<WindowOption>::MakeSptr();
    option_->SetWindowName("ClearListenersById_occupiedAreaChangeListeners");
    sptr<WindowSessionImpl> window_ = sptr<WindowSessionImpl>::MakeSptr(option_);

    int persistentId = window_->GetPersistentId();
    window_->ClearListenersById(persistentId);

    sptr<IOccupiedAreaChangeListener> listener_ = new (std::nothrow) MockIOccupiedAreaChangeListener();
    window_->RegisterOccupiedAreaChangeListener(listener_);
    ASSERT_NE(window_->occupiedAreaChangeListeners_.find(persistentId), window_->occupiedAreaChangeListeners_.end());

    window_->ClearListenersById(persistentId);
    ASSERT_EQ(window_->occupiedAreaChangeListeners_.find(persistentId), window_->occupiedAreaChangeListeners_.end());

    GTEST_LOG_(INFO) << "WindowSessionImplTest4: ClearListenersById_occupiedAreaChangeListeners end";
}

/**
 * @tc.name: ClearListenersById_keyboardDidShowListeners
 * @tc.desc: ClearListenersById_keyboardDidShowListeners
 * @tc.type: FUNC
 */
HWTEST_F(WindowSessionImplTest4, ClearListenersById_keyboardDidShowListeners, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "WindowSessionImplTest4: ClearListenersById_keyboardDidShowListeners start";
    sptr<WindowOption> option_ = sptr<WindowOption>::MakeSptr();
    option_->SetWindowName("ClearListenersById_keyboardDidShowListeners");
    sptr<WindowSessionImpl> window_ = sptr<WindowSessionImpl>::MakeSptr(option_);

    int persistentId = window_->GetPersistentId();
    window_->ClearListenersById(persistentId);

    sptr<IKeyboardDidShowListener> listener_ = new (std::nothrow) MockIKeyboardDidShowListener();
    window_->RegisterKeyboardDidShowListener(listener_);
    ASSERT_NE(window_->keyboardDidShowListeners_.find(persistentId), window_->keyboardDidShowListeners_.end());

    window_->ClearListenersById(persistentId);
    ASSERT_EQ(window_->keyboardDidShowListeners_.find(persistentId), window_->keyboardDidShowListeners_.end());

    GTEST_LOG_(INFO) << "WindowSessionImplTest4: ClearListenersById_keyboardDidShowListeners end";
}

/**
 * @tc.name: ClearListenersById_keyboardDidHideListeners
 * @tc.desc: ClearListenersById_keyboardDidHideListeners
 * @tc.type: FUNC
 */
HWTEST_F(WindowSessionImplTest4, ClearListenersById_keyboardDidHideListeners, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "WindowSessionImplTest4: ClearListenersById_keyboardDidHideListeners start";
    sptr<WindowOption> option_ = sptr<WindowOption>::MakeSptr();
    option_->SetWindowName("ClearListenersById_keyboardDidHideListeners");
    sptr<WindowSessionImpl> window_ = sptr<WindowSessionImpl>::MakeSptr(option_);

    int persistentId = window_->GetPersistentId();
    window_->ClearListenersById(persistentId);

    sptr<IKeyboardDidHideListener> listener_ = new (std::nothrow) MockIKeyboardDidHideListener();
    window_->RegisterKeyboardDidHideListener(listener_);
    ASSERT_NE(window_->keyboardDidHideListeners_.find(persistentId), window_->keyboardDidHideListeners_.end());

    window_->ClearListenersById(persistentId);
    ASSERT_EQ(window_->keyboardDidHideListeners_.find(persistentId), window_->keyboardDidHideListeners_.end());

    GTEST_LOG_(INFO) << "WindowSessionImplTest4: ClearListenersById_keyboardDidHideListeners end";
}

/**
 * @tc.name: ClearListenersById_switchFreeMultiWindowListeners
 * @tc.desc: ClearListenersById_switchFreeMultiWindowListeners
 * @tc.type: FUNC
 */
HWTEST_F(WindowSessionImplTest4, ClearListenersById_switchFreeMultiWindowListeners, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "WindowSessionImplTest4: ClearListenersById_switchFreeMultiWindowListeners start";
    sptr<WindowOption> option_ = sptr<WindowOption>::MakeSptr();
    option_->SetWindowName("ClearListenersById_switchFreeMultiWindowListeners");
    sptr<WindowSessionImpl> window_ = sptr<WindowSessionImpl>::MakeSptr(option_);

    int persistentId = window_->GetPersistentId();
    window_->ClearListenersById(persistentId);

    sptr<ISwitchFreeMultiWindowListener> listener_ = new (std::nothrow) MockISwitchFreeMultiWindowListener();
    window_->RegisterSwitchFreeMultiWindowListener(listener_);
    ASSERT_NE(window_->switchFreeMultiWindowListeners_.find(persistentId),
              window_->switchFreeMultiWindowListeners_.end());

    window_->ClearListenersById(persistentId);
    ASSERT_EQ(window_->switchFreeMultiWindowListeners_.find(persistentId),
              window_->switchFreeMultiWindowListeners_.end());

    WindowAccessibilityController::GetInstance().SetAnchorAndScale(0, 0, 2);
    sleep(1);
    WindowAccessibilityController::GetInstance().SetAnchorOffset(0, 0);
    sleep(1);
    WindowAccessibilityController::GetInstance().OffWindowZoom();
    sleep(1);

    GTEST_LOG_(INFO) << "WindowSessionImplTest4: ClearListenersById_switchFreeMultiWindowListeners end";
}

/**
 * @tc.name: FlushLayoutSize
 * @tc.desc: FlushLayoutSize
 * @tc.type: FUNC
 */
HWTEST_F(WindowSessionImplTest4, FlushLayoutSize, TestSize.Level1)
{
#undef private
    GTEST_LOG_(INFO) << "WindowSessionImplTest4: FlushLayoutSize start";
    sptr<WindowOption> option = sptr<WindowOption>::MakeSptr();
    option->SetWindowName("FlushLayoutSize");
    sptr<WindowSessionImpl> window = sptr<WindowSessionImpl>::MakeSptr(option);
    window->property_->SetWindowType(WindowType::APP_SUB_WINDOW_BASE);
    int32_t width = 1320;
    int32_t height = 2710;
    WSRect rect = { 0, 0, width, height };
    window->FlushLayoutSize(width, height);

    window->property_->SetWindowType(WindowType::APP_MAIN_WINDOW_BASE);
    window->windowSizeChanged_ = true;
    window->FlushLayoutSize(width, height);
    ASSERT_EQ(window->windowSizeChanged_, false);

    window->layoutRect_ = { 0, 0, 2710, 1320 };
    window->FlushLayoutSize(width, height);
    ASSERT_EQ(window->layoutRect_, rect);

    window->enableFrameLayoutFinishCb_ = true;
    window->FlushLayoutSize(width, height);
    ASSERT_EQ(window->enableFrameLayoutFinishCb_, false);

    GTEST_LOG_(INFO) << "WindowSessionImplTest4: FlushLayoutSize end";
}

/**
 * @tc.name: FlushVsync
 * @tc.desc: FlushVsync test
 * @tc.type: FUNC
 */
HWTEST_F(WindowSessionImplTest4, FlushVsync, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "WindowSessionImplTest4: FlushVsync start";
    sptr<WindowOption> option = sptr<WindowOption>::MakeSptr();
    option->SetWindowName("FlushVsync");
    sptr<WindowSessionImpl> window = sptr<WindowSessionImpl>::MakeSptr(option);

    SessionInfo sessionInfo = { "CreateTestBundle", "CreateTestModule", "CreateTestAbility" };
    sptr<SessionMocker> session = sptr<SessionMocker>::MakeSptr(sessionInfo);
    window->hostSession_ = session;

    window->property_->SetWindowType(WindowType::APP_SUB_WINDOW_BASE);
    window->FlushVsync();
    ASSERT_EQ(window->vsyncCount_, 0);

    window->property_->SetWindowType(WindowType::APP_MAIN_WINDOW_BASE);
    window->property_->SetPrelaunch(false);
    window->FlushVsync();
    ASSERT_EQ(window->vsyncCount_, 0);

    window->property_->SetPrelaunch(true);
    window->property_->SetFrameNum(0);
    window->FlushVsync();
    ASSERT_EQ(window->vsyncCount_, 0);

    window->property_->SetWindowType(WindowType::APP_MAIN_WINDOW_BASE);
    window->property_->SetPrelaunch(true);
    int frameNum = 3;
    window->property_->SetFrameNum(frameNum);
    for (int idx = 0; idx < frameNum; ++idx) {
        window->FlushVsync();
    }
    ASSERT_EQ(window->vsyncCount_, 3);
    ASSERT_EQ(window->hasNotifyPrelaunchStartingwindow_, true);
    window->FlushVsync();
    ASSERT_EQ(window->vsyncCount_, 3);

    GTEST_LOG_(INFO) << "WindowSessionImplTest4: FlushVsync end";
}

/**
 * @tc.name: NotifySnapshotUpdate
 * @tc.desc: NotifySnapshotUpdate
 * @tc.type: FUNC
 */
HWTEST_F(WindowSessionImplTest4, NotifySnapshotUpdate, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "WindowSessionImplTest4: NotifySnapshotUpdate start";
    sptr<WindowOption> option = sptr<WindowOption>::MakeSptr();
    option->SetWindowName("NotifySnapshotUpdate");

    sptr<WindowSessionImpl> window = sptr<WindowSessionImpl>::MakeSptr(option);
    WMError ret = window->NotifySnapshotUpdate();
    EXPECT_EQ(ret, WMError::WM_ERROR_INVALID_SESSION);

    SessionInfo sessioninfo = { "CreateTestBundle", "CreateTestModule", "CreateTestAbility" };
    sptr<SessionMocker> session = sptr<SessionMocker>::MakeSptr(sessioninfo);
    window->hostSession_ = session;
    ret = window->NotifySnapshotUpdate();
    EXPECT_EQ(ret, WMError::WM_OK);

    GTEST_LOG_(INFO) << "WindowSessionImplTest4: NotifySnapshotUpdate end";
}

/**
 * @tc.name: RegisterDisplayIdChangeListener01
 * @tc.desc: RegisterDisplayIdChangeListener01
 * @tc.type: FUNC
 */
HWTEST_F(WindowSessionImplTest4, RegisterDisplayIdChangeListener01, TestSize.Level1)
{
    sptr<WindowOption> option = sptr<WindowOption>::MakeSptr();
    option->SetWindowName("RegisterDisplayIdChangeListener01");

    sptr<WindowSessionImpl> window = sptr<WindowSessionImpl>::MakeSptr(option);
    sptr<IDisplayIdChangeListener> listener = nullptr;
    WMError ret = window->RegisterDisplayIdChangeListener(listener);
    ASSERT_EQ(ret, WMError::WM_ERROR_NULLPTR);

    listener = sptr<IDisplayIdChangeListener>::MakeSptr();
    std::vector<sptr<IDisplayIdChangeListener>> holder;
    window->displayIdChangeListeners_[window->property_->GetPersistentId()] = holder;
    ret = window->RegisterDisplayIdChangeListener(listener);
    ASSERT_EQ(ret, WMError::WM_OK);
    holder = window->displayIdChangeListeners_[window->property_->GetPersistentId()];
    auto existsListener = std::find(holder.begin(), holder.end(), listener);
    ASSERT_NE(existsListener, holder.end());

    ret = window->RegisterDisplayIdChangeListener(listener);
    ASSERT_EQ(ret, WMError::WM_OK);
}

/**
 * @tc.name: UnregisterDisplayIdChangeListener01
 * @tc.desc: UnregisterDisplayIdChangeListener01
 * @tc.type: FUNC
 */
HWTEST_F(WindowSessionImplTest4, UnregisterDisplayIdChangeListener01, TestSize.Level1)
{
    sptr<WindowOption> option = sptr<WindowOption>::MakeSptr();
    option->SetWindowName("UnregisterDisplayIdChangeListener01");

    sptr<WindowSessionImpl> window = sptr<WindowSessionImpl>::MakeSptr(option);
    sptr<IDisplayIdChangeListener> listener = nullptr;
    WMError ret = window->UnregisterDisplayIdChangeListener(listener);
    ASSERT_EQ(ret, WMError::WM_ERROR_NULLPTR);

    listener = sptr<IDisplayIdChangeListener>::MakeSptr();
    std::vector<sptr<IDisplayIdChangeListener>> holder;
    window->displayIdChangeListeners_[window->property_->GetPersistentId()] = holder;
    window->UnregisterDisplayIdChangeListener(listener);

    ret = window->UnregisterDisplayIdChangeListener(listener);
    ASSERT_EQ(ret, WMError::WM_OK);

    holder = window->displayIdChangeListeners_[window->property_->GetPersistentId()];
    auto existsListener = std::find(holder.begin(), holder.end(), listener);
    ASSERT_EQ(existsListener, holder.end());
}

/**
 * @tc.name: NotifyDisplayIdChange01
 * @tc.desc: NotifyDisplayIdChange01
 * @tc.type: FUNC
 */
HWTEST_F(WindowSessionImplTest4, NotifyDisplayIdChange01, TestSize.Level1)
{
    sptr<WindowOption> option = sptr<WindowOption>::MakeSptr();
    option->SetWindowName("NotifyDisplayIdChange01");

    sptr<WindowSessionImpl> window = sptr<WindowSessionImpl>::MakeSptr(option);

    SessionInfo sessioninfo = { "CreateTestBundle", "CreateTestModule", "CreateTestAbility" };
    sptr<SessionMocker> session = sptr<SessionMocker>::MakeSptr(sessioninfo);
    ASSERT_NE(session, nullptr);
    ASSERT_EQ(WMError::WM_OK, window->Create(nullptr, session));
    DisplayId displayId = 12;
    auto ret = window->NotifyDisplayIdChange(displayId);
    ASSERT_EQ(WSError::WS_OK, ret);
    ASSERT_EQ(WMError::WM_ERROR_INVALID_WINDOW, window->Destroy());
}

/**
 * @tc.name: RegisterSystemDensityChangeListener01
 * @tc.desc: RegisterSystemDensityChangeListener01
 * @tc.type: FUNC
 */
HWTEST_F(WindowSessionImplTest4, RegisterSystemDensityChangeListener01, TestSize.Level1)
{
    sptr<WindowOption> option = sptr<WindowOption>::MakeSptr();
    option->SetWindowName("RegisterSystemDensityChangeListener01");

    sptr<WindowSessionImpl> window = sptr<WindowSessionImpl>::MakeSptr(option);
    sptr<ISystemDensityChangeListener> listener = nullptr;
    WMError ret = window->RegisterSystemDensityChangeListener(listener);
    ASSERT_EQ(ret, WMError::WM_ERROR_NULLPTR);

    listener = sptr<ISystemDensityChangeListener>::MakeSptr();
    std::vector<sptr<ISystemDensityChangeListener>> holder;
    window->systemDensityChangeListeners_[window->property_->GetPersistentId()] = holder;
    ret = window->RegisterSystemDensityChangeListener(listener);
    ASSERT_EQ(ret, WMError::WM_OK);
    holder = window->systemDensityChangeListeners_[window->property_->GetPersistentId()];
    auto existsListener = std::find(holder.begin(), holder.end(), listener);
    ASSERT_NE(existsListener, holder.end());

    ret = window->RegisterSystemDensityChangeListener(listener);
    ASSERT_EQ(ret, WMError::WM_OK);
    holder = window->systemDensityChangeListeners_[window->property_->GetPersistentId()];
    ASSERT_EQ(holder.size(), 1);
}

/**
 * @tc.name: UnregisterSystemDensityChangeListener01
 * @tc.desc: UnregisterSystemDensityChangeListener01
 * @tc.type: FUNC
 */
HWTEST_F(WindowSessionImplTest4, UnregisterSystemDensityChangeListener01, TestSize.Level1)
{
    sptr<WindowOption> option = sptr<WindowOption>::MakeSptr();
    option->SetWindowName("UnregisterSystemDensityChangeListener01");

    sptr<WindowSessionImpl> window = sptr<WindowSessionImpl>::MakeSptr(option);
    sptr<ISystemDensityChangeListener> listener = nullptr;
    WMError ret = window->UnregisterSystemDensityChangeListener(listener);
    ASSERT_EQ(ret, WMError::WM_ERROR_NULLPTR);

    listener = sptr<ISystemDensityChangeListener>::MakeSptr();
    std::vector<sptr<ISystemDensityChangeListener>> holder;
    window->systemDensityChangeListeners_[window->property_->GetPersistentId()] = holder;
    ret = window->UnregisterSystemDensityChangeListener(listener);
    ASSERT_EQ(ret, WMError::WM_OK);

    holder = window->systemDensityChangeListeners_[window->property_->GetPersistentId()];
    auto existsListener = std::find(holder.begin(), holder.end(), listener);
    ASSERT_EQ(existsListener, holder.end());
}

/**
 * @tc.name: NotifySystemDensityChange01
 * @tc.desc: NotifySystemDensityChange01
 * @tc.type: FUNC
 */
HWTEST_F(WindowSessionImplTest4, NotifySystemDensityChange01, TestSize.Level1)
{
    sptr<WindowOption> option = sptr<WindowOption>::MakeSptr();
    option->SetWindowName("NotifySystemDensityChange01");

    sptr<WindowSessionImpl> window = sptr<WindowSessionImpl>::MakeSptr(option);
    SessionInfo sessioninfo = { "CreateTestBundle", "CreateTestModule", "CreateTestAbility" };
    sptr<SessionMocker> session = sptr<SessionMocker>::MakeSptr(sessioninfo);
    ASSERT_EQ(WMError::WM_OK, window->Create(nullptr, session));

    float density = 1.5f;
    auto ret = window->NotifySystemDensityChange(density);
    ASSERT_EQ(WSError::WS_OK, ret);
    ASSERT_EQ(WMError::WM_ERROR_INVALID_WINDOW, window->Destroy());
}

/**
 * @tc.name: GetIsMidScene
 * @tc.desc: GetIsMidScene
 * @tc.type: FUNC
 */
HWTEST_F(WindowSessionImplTest4, GetIsMidScene, TestSize.Level1)
{
    sptr<WindowOption> option = sptr<WindowOption>::MakeSptr();
    option->SetWindowName("GetIsMidScene");

    sptr<WindowSessionImpl> window = sptr<WindowSessionImpl>::MakeSptr(option);
    SessionInfo sessionInfo = { "CreateTestBundle", "CreateTestModule", "CreateTestAbility" };
    sptr<SessionMocker> session = sptr<SessionMocker>::MakeSptr(sessionInfo);
    ASSERT_EQ(WMError::WM_OK, window->Create(nullptr, session));

    bool isMidScene = false;
    auto ret = window->GetIsMidScene(isMidScene);
    ASSERT_EQ(WMError::WM_ERROR_INVALID_WINDOW, ret);
    ASSERT_EQ(WMError::WM_ERROR_INVALID_WINDOW, window->Destroy());
}

/**
 * @tc.name: GetLayoutTransform
 * @tc.desc: GetLayoutTransform
 * @tc.type: FUNC
 */
HWTEST_F(WindowSessionImplTest4, GetLayoutTransform, TestSize.Level1)
{
    sptr<WindowOption> option = sptr<WindowOption>::MakeSptr();
    option->SetWindowName("GetLayoutTransform");
    sptr<WindowSessionImpl> window = sptr<WindowSessionImpl>::MakeSptr(option);

    Transform transform;
    transform.scaleX_ = 1.0;
    transform.scaleY_ = 1.0;
    window->uiContent_ = std::make_unique<Ace::UIContentMocker>();
    window->NotifyTransformChange(transform);
    Transform layoutTransform = window->GetLayoutTransform();
    ASSERT_EQ(transform.scaleX_, layoutTransform.scaleX_);
    ASSERT_EQ(transform.scaleY_, layoutTransform.scaleY_);
    ASSERT_EQ(WMError::WM_ERROR_INVALID_WINDOW, window->Destroy());
}

/**
 * @tc.name: SetExclusivelyHighlighted
 * @tc.desc: SetExclusivelyHighlighted
 * @tc.type: FUNC
 */
HWTEST_F(WindowSessionImplTest4, SetExclusivelyHighlighted, TestSize.Level1)
{
    sptr<WindowOption> option = sptr<WindowOption>::MakeSptr();
    option->SetWindowName("SetExclusivelyHighlighted");
    sptr<WindowSessionImpl> window = sptr<WindowSessionImpl>::MakeSptr(option);
    SessionInfo sessionInfo = { "CreateTestBundle", "CreateTestModule", "CreateTestAbility" };
    sptr<SessionMocker> session = sptr<SessionMocker>::MakeSptr(sessionInfo);
    ASSERT_EQ(WMError::WM_OK, window->Create(nullptr, session));
    window->hostSession_ = session;
    window->property_->SetPersistentId(INVALID_SESSION_ID);
    ASSERT_EQ(window->SetExclusivelyHighlighted(true), WMError::WM_ERROR_INVALID_WINDOW);
    window->property_->SetPersistentId(1);
    window->property_->SetWindowType(WindowType::APP_MAIN_WINDOW_BASE);
    ASSERT_EQ(window->SetExclusivelyHighlighted(true), WMError::WM_ERROR_INVALID_CALLING);
    window->property_->SetWindowType(WindowType::WINDOW_TYPE_DIALOG);
    ASSERT_EQ(window->SetExclusivelyHighlighted(true), WMError::WM_ERROR_INVALID_CALLING);
    window->property_->SetWindowType(WindowType::APP_SUB_WINDOW_BASE);
    window->property_->AddWindowFlag(WindowFlag::WINDOW_FLAG_IS_APPLICATION_MODAL);
    ASSERT_EQ(window->SetExclusivelyHighlighted(true), WMError::WM_ERROR_INVALID_CALLING);
    window->property_->flags_ = 0;
    ASSERT_EQ(window->SetExclusivelyHighlighted(true), WMError::WM_OK);
    ASSERT_EQ(window->SetExclusivelyHighlighted(false), WMError::WM_OK);
}

/**
 * @tc.name: GetExclusivelyHighlighted
 * @tc.desc: GetExclusivelyHighlighted
 * @tc.type: FUNC
 */
HWTEST_F(WindowSessionImplTest4, GetExclusivelyHighlighted, TestSize.Level1)
{
    sptr<WindowOption> option = sptr<WindowOption>::MakeSptr();
    option->SetWindowName("GetExclusivelyHighlighted");
    sptr<WindowSessionImpl> window = sptr<WindowSessionImpl>::MakeSptr(option);
    ASSERT_EQ(window->GetExclusivelyHighlighted(), true);
}

/**
 * @tc.name: NotifyHighlightChange
 * @tc.desc: NotifyHighlightChange
 * @tc.type: FUNC
 */
HWTEST_F(WindowSessionImplTest4, NotifyHighlightChange, TestSize.Level1)
{
    sptr<WindowOption> option = sptr<WindowOption>::MakeSptr();
    option->SetWindowName("NotifyHighlightChange01");
    sptr<WindowSessionImpl> window = sptr<WindowSessionImpl>::MakeSptr(option);
    SessionInfo sessionInfo = { "CreateTestBundle", "CreateTestModule", "CreateTestAbility" };
    sptr<SessionMocker> session = sptr<SessionMocker>::MakeSptr(sessionInfo);
    window->hostSession_ = session;
    window->property_->SetPersistentId(1);

    bool highlight = false;
    sptr<HighlightNotifyInfo> info = nullptr;
    WSError res = window->NotifyHighlightChange(info, highlight);
    info = sptr<HighlightNotifyInfo>::MakeSptr();
    info->isSyncNotify_ = false;
    res = window->NotifyHighlightChange(info, highlight);
    EXPECT_EQ(res, WSError::WS_OK);
    sptr<IWindowHighlightChangeListener> listener = sptr<IWindowHighlightChangeListener>::MakeSptr();
    window->RegisterWindowHighlightChangeListeners(listener);
    res = window->NotifyHighlightChange(info, highlight);
    EXPECT_EQ(res, WSError::WS_OK);
    window->UnregisterWindowHighlightChangeListeners(listener);
}

/**
 * @tc.name: NotifyHighlightChange01
 * @tc.desc: NotifyHighlightChange
 * @tc.type: FUNC
 */
HWTEST_F(WindowSessionImplTest4, NotifyHighlightChange01, TestSize.Level1)
{
    sptr<WindowOption> option = sptr<WindowOption>::MakeSptr();
    option->SetWindowName("NotifyHighlightChange01");
    sptr<WindowSessionImpl> window = sptr<WindowSessionImpl>::MakeSptr(option);
    window->property_->SetPersistentId(1);
    window->updateHighlightTimeStamp_.store(2);
    bool highlight = false;
    auto info = sptr<HighlightNotifyInfo>::MakeSptr();
    info->isSyncNotify_ = true;
    info->timeStamp_ = 1;
    WSError res = window->NotifyHighlightChange(info, highlight);
    EXPECT_EQ(window->updateHighlightTimeStamp_.load(), 2);
    info->timeStamp_ = 3;
    res = window->NotifyHighlightChange(info, highlight);
    EXPECT_EQ(window->updateHighlightTimeStamp_.load(), 3);
    EXPECT_EQ(res, WSError::WS_OK);
}

/**
 * @tc.name: NotifyHighlightChange02
 * @tc.desc: NotifyHighlightChange
 * @tc.type: FUNC
 */
HWTEST_F(WindowSessionImplTest4, NotifyHighlightChange02, TestSize.Level1)
{
    sptr<WindowOption> option = sptr<WindowOption>::MakeSptr();
    option->SetWindowName("NotifyHighlightChange02");
    sptr<WindowSessionImpl> window = sptr<WindowSessionImpl>::MakeSptr(option);
    sptr<WindowSessionImpl> window1 = sptr<WindowSessionImpl>::MakeSptr(option);
    window->property_->SetPersistentId(1);
    window1->property_->SetPersistentId(2);
    window->updateHighlightTimeStamp_.store(2);
    bool highlight = false;
    auto info = sptr<HighlightNotifyInfo>::MakeSptr(3, std::vector<int32_t>(1, 2), 2, true);
    WSError res = window->NotifyHighlightChange(info, highlight);
    EXPECT_EQ(window->updateHighlightTimeStamp_.load(), 3);
    info->timeStamp_ = 4;
    res = window->NotifyHighlightChange(info, highlight);
    EXPECT_EQ(window->updateHighlightTimeStamp_.load(), 4);
    EXPECT_EQ(res, WSError::WS_OK);
}

/**
 * @tc.name: IsWindowHighlighted
 * @tc.desc: IsWindowHighlighted
 * @tc.type: FUNC
 */
HWTEST_F(WindowSessionImplTest4, IsWindowHighlighted, TestSize.Level1)
{
    sptr<WindowOption> option = sptr<WindowOption>::MakeSptr();
    option->SetWindowName("IsWindowHighlighted");
    sptr<WindowSessionImpl> window = sptr<WindowSessionImpl>::MakeSptr(option);
    SessionInfo sessionInfo = { "CreateTestBundle", "CreateTestModule", "CreateTestAbility" };
    sptr<SessionMocker> session = sptr<SessionMocker>::MakeSptr(sessionInfo);
    ASSERT_EQ(WMError::WM_OK, window->Create(nullptr, session));
    bool isHighlighted = false;
    window->hostSession_ = session;
    window->property_->SetPersistentId(INVALID_SESSION_ID);
    ASSERT_EQ(window->IsWindowHighlighted(isHighlighted), WMError::WM_ERROR_INVALID_WINDOW);
    window->property_->SetPersistentId(1);
    ASSERT_EQ(window->IsWindowHighlighted(isHighlighted), WMError::WM_OK);
}

/**
 * @tc.name: NotifyWindowCrossAxisChange
 * @tc.desc: NotifyWindowCrossAxisChange
 * @tc.type: FUNC
 */
HWTEST_F(WindowSessionImplTest4, NotifyWindowCrossAxisChange, TestSize.Level1)
{
    sptr<WindowOption> option = sptr<WindowOption>::MakeSptr();
    option->SetWindowName("NotifyWindowCrossAxisChange");
    sptr<WindowSessionImpl> window = sptr<WindowSessionImpl>::MakeSptr(option);
    sptr<MockIWindowCrossAxisListener> crossListener = sptr<MockIWindowCrossAxisListener>::MakeSptr();
    WindowSessionImpl::windowCrossAxisListeners_[window->property_->persistentId_].push_back(crossListener);
    EXPECT_CALL(*crossListener, OnCrossAxisChange(CrossAxisState::STATE_CROSS)).Times(1);
    window->NotifyWindowCrossAxisChange(CrossAxisState::STATE_CROSS);
    EXPECT_EQ(window->crossAxisState_.load(), CrossAxisState::STATE_CROSS);
    WindowSessionImpl::windowCrossAxisListeners_[window->property_->persistentId_].clear();
}

/**
 * @tc.name: GetCrossAxisState
 * @tc.desc: GetCrossAxisState
 * @tc.type: FUNC
 */
HWTEST_F(WindowSessionImplTest4, GetCrossAxisState, TestSize.Level1)
{
    sptr<WindowOption> option = sptr<WindowOption>::MakeSptr();
    option->SetWindowName("GetCrossAxisState");
    sptr<WindowSessionImpl> window = sptr<WindowSessionImpl>::MakeSptr(option);
    window->crossAxisState_ =  CrossAxisState::STATE_CROSS;
    EXPECT_EQ(window->GetCrossAxisState(), CrossAxisState::STATE_CROSS);
    window->crossAxisState_ =  CrossAxisState::STATE_INVALID;
    window->hostSession_ = nullptr;
    EXPECT_EQ(window->GetCrossAxisState(), CrossAxisState::STATE_INVALID);
    auto mockHostSession = sptr<SessionStubMocker>::MakeSptr();
    window->hostSession_ = mockHostSession;
    window->property_->persistentId_ = 1234;
    EXPECT_CALL(*mockHostSession, GetCrossAxisState(_))
        .WillOnce(DoAll(SetArgReferee<0>(CrossAxisState::STATE_CROSS), Return(WSError::WS_OK)));
    EXPECT_EQ(window->GetCrossAxisState(), CrossAxisState::STATE_CROSS);
}

/**
 * @tc.name: SendContainerModalEvent
 * @tc.desc: SendContainerModalEvent
 * @tc.type: FUNC
 */
HWTEST_F(WindowSessionImplTest4, SendContainerModalEvent, TestSize.Level1)
{
    sptr<WindowOption> option = sptr<WindowOption>::MakeSptr();
    option->SetWindowName("SendContainerModalEvent");
    sptr<WindowSessionImpl> window = sptr<WindowSessionImpl>::MakeSptr(option);
    auto ret = window->SendContainerModalEvent("scb_back_visibility", "true");
    EXPECT_EQ(ret, WSError::WS_OK);
    ret = window->SendContainerModalEvent("scb_back_visibility", "false");
    EXPECT_EQ(ret, WSError::WS_OK);
    ret = window->SendContainerModalEvent("win_waterfall_visibility", "true");
    EXPECT_EQ(ret, WSError::WS_OK);
    ret = window->SendContainerModalEvent("win_waterfall_visibility", "false");
    EXPECT_EQ(ret, WSError::WS_OK);
}

/**
 * @tc.name: SetSubWindowZLevelToProperty
 * @tc.desc: SetSubWindowZLevelToProperty
 * @tc.type: FUNC
 */
HWTEST_F(WindowSessionImplTest4, SetSubWindowZLevelToProperty, TestSize.Level1)
{
    sptr<WindowOption> option = sptr<WindowOption>::MakeSptr();
    option->SetWindowType(WindowType::WINDOW_TYPE_APP_MAIN_WINDOW);
    option->SetSubWindowZLevel(0);
    sptr<WindowSessionImpl> mainWindowSessionImpl = sptr<WindowSessionImpl>::MakeSptr(option);
    option->SetSubWindowZLevel(1);
    mainWindowSessionImpl->windowOption_ = option;
    mainWindowSessionImpl->SetSubWindowZLevelToProperty();
    int32_t zLevel = mainWindowSessionImpl->property_->zLevel_;
    EXPECT_NE(1, zLevel);

    option->SetWindowType(WindowType::WINDOW_TYPE_APP_SUB_WINDOW);
    sptr<WindowSessionImpl> subWindowSessionImpl = sptr<WindowSessionImpl>::MakeSptr(option);
    option->SetSubWindowZLevel(2);
    subWindowSessionImpl->windowOption_ = option;
    subWindowSessionImpl->SetSubWindowZLevelToProperty();
    zLevel = subWindowSessionImpl->property_->zLevel_;
    EXPECT_EQ(2, zLevel);
}

/**
 * @tc.name: GetSubWindowZLevelByFlags01
 * @tc.desc: GetSubWindowZLevelByFlags
 * @tc.type: FUNC
 */
HWTEST_F(WindowSessionImplTest4, GetSubWindowZLevelByFlags01, Function | SmallTest | Level2)
{
    sptr<WindowOption> option = sptr<WindowOption>::MakeSptr();
    option->SetWindowType(WindowType::WINDOW_TYPE_APP_SUB_WINDOW);
    sptr<WindowSceneSessionImpl> normalSubWindow = sptr<WindowSceneSessionImpl>::MakeSptr(option);
    int32_t ret = normalSubWindow->GetSubWindowZLevelByFlags(normalSubWindow->GetType(),
        normalSubWindow->GetWindowFlags(), normalSubWindow->IsTopmost());
    EXPECT_EQ(ret, NORMAL_SUB_WINDOW_Z_LEVEL);

    option->SetWindowFlags(static_cast<uint32_t>(WindowFlag::WINDOW_FLAG_IS_TEXT_MENU));
    sptr<WindowSceneSessionImpl> textMenuSubWindow = sptr<WindowSceneSessionImpl>::MakeSptr(option);
    ret = textMenuSubWindow->GetSubWindowZLevelByFlags(textMenuSubWindow->GetType(),
        textMenuSubWindow->GetWindowFlags(), textMenuSubWindow->IsTopmost());
    EXPECT_EQ(ret, TEXT_MENU_SUB_WINDOW_Z_LEVEL);

    option->SetWindowFlags(static_cast<uint32_t>(WindowFlag::WINDOW_FLAG_IS_TOAST));
    sptr<WindowSceneSessionImpl> toastSubWindow = sptr<WindowSceneSessionImpl>::MakeSptr(option);
    ret = toastSubWindow->GetSubWindowZLevelByFlags(toastSubWindow->GetType(),
        toastSubWindow->GetWindowFlags(), toastSubWindow->IsTopmost());
    EXPECT_EQ(ret, TOAST_SUB_WINDOW_Z_LEVEL);

    option->SetWindowFlags(static_cast<uint32_t>(WindowFlag::WINDOW_FLAG_IS_MODAL));
    sptr<WindowSceneSessionImpl> modalSubWindow = sptr<WindowSceneSessionImpl>::MakeSptr(option);
    ret = modalSubWindow->GetSubWindowZLevelByFlags(modalSubWindow->GetType(),
        modalSubWindow->GetWindowFlags(), modalSubWindow->IsTopmost());
    EXPECT_EQ(ret, MODALITY_SUB_WINDOW_Z_LEVEL);

    option->AddWindowFlag(WindowFlag::WINDOW_FLAG_IS_APPLICATION_MODAL);
    sptr<WindowSceneSessionImpl> appModalSubWindow = sptr<WindowSceneSessionImpl>::MakeSptr(option);
    ret = appModalSubWindow->GetSubWindowZLevelByFlags(appModalSubWindow->GetType(),
        appModalSubWindow->GetWindowFlags(), appModalSubWindow->IsTopmost());
    EXPECT_EQ(ret, APPLICATION_MODALITY_SUB_WINDOW_Z_LEVEL);
}

/**
 * @tc.name: GetSubWindowZLevelByFlags02
 * @tc.desc: GetSubWindowZLevelByFlags
 * @tc.type: FUNC
 */
HWTEST_F(WindowSessionImplTest4, GetSubWindowZLevelByFlags02, Function | SmallTest | Level2)
{
    sptr<WindowOption> option = sptr<WindowOption>::MakeSptr();
    option->SetWindowType(WindowType::WINDOW_TYPE_APP_SUB_WINDOW);
    option->SetWindowFlags(static_cast<uint32_t>(WindowFlag::WINDOW_FLAG_IS_MODAL));
    option->SetWindowTopmost(true);
    sptr<WindowSceneSessionImpl> topmostModalSubWindow = sptr<WindowSceneSessionImpl>::MakeSptr(option);
    int32_t ret = topmostModalSubWindow->GetSubWindowZLevelByFlags(topmostModalSubWindow->GetType(),
        topmostModalSubWindow->GetWindowFlags(), topmostModalSubWindow->IsTopmost());
    EXPECT_EQ(ret, MODALITY_SUB_WINDOW_Z_LEVEL + TOPMOST_SUB_WINDOW_Z_LEVEL);

    option->AddWindowFlag(WindowFlag::WINDOW_FLAG_IS_APPLICATION_MODAL);
    sptr<WindowSceneSessionImpl> topmostAppModalSubWindow = sptr<WindowSceneSessionImpl>::MakeSptr(option);
    ret = topmostAppModalSubWindow->GetSubWindowZLevelByFlags(topmostAppModalSubWindow->GetType(),
        topmostAppModalSubWindow->GetWindowFlags(), topmostAppModalSubWindow->IsTopmost());
    EXPECT_EQ(ret, APPLICATION_MODALITY_SUB_WINDOW_Z_LEVEL + TOPMOST_SUB_WINDOW_Z_LEVEL);
}

/**
 * @tc.name: GetSubWindowZLevelByFlags03
 * @tc.desc: GetSubWindowZLevelByFlags
 * @tc.type: FUNC
 */
HWTEST_F(WindowSessionImplTest4, GetSubWindowZLevelByFlags03, Function | SmallTest | Level2)
{
    sptr<WindowOption> option = sptr<WindowOption>::MakeSptr();
    option->SetWindowType(WindowType::WINDOW_TYPE_DIALOG);
    sptr<WindowSceneSessionImpl> dialogWindow = sptr<WindowSceneSessionImpl>::MakeSptr(option);
    int32_t ret = dialogWindow->GetSubWindowZLevelByFlags(dialogWindow->GetType(),
        dialogWindow->GetWindowFlags(), dialogWindow->IsTopmost());
    EXPECT_EQ(ret, DIALOG_SUB_WINDOW_Z_LEVEL);
}

/**
 * @tc.name: NotifyAppForceLandscapeConfigUpdated
 * @tc.desc: NotifyAppForceLandscapeConfigUpdated
 * @tc.type: FUNC
 */
HWTEST_F(WindowSessionImplTest4, NotifyAppForceLandscapeConfigUpdated, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "WindowSessionImplTest4: NotifyAppForceLandscapeConfigUpdated start";
    sptr<WindowOption> option = sptr<WindowOption>::MakeSptr();
    option->SetWindowName("NotifyAppForceLandscapeConfigUpdated");

    sptr<WindowSessionImpl> window = sptr<WindowSessionImpl>::MakeSptr(option);
    SessionInfo sessionInfo = { "CreateTestBundle", "CreateTestModule", "CreateTestAbility" };
    sptr<SessionMocker> session = sptr<SessionMocker>::MakeSptr(sessionInfo);
    ASSERT_EQ(WMError::WM_OK, window->Create(nullptr, session));

    WSError res = window->NotifyAppForceLandscapeConfigUpdated();
    EXPECT_EQ(res, WSError::WS_DO_NOTHING);
    EXPECT_EQ(WMError::WM_ERROR_INVALID_WINDOW, window->Destroy());
    GTEST_LOG_(INFO) << "WindowSessionImplTest4: NotifyAppForceLandscapeConfigUpdated end";
}

/**
 * @tc.name: IsStageDefaultDensityEnabled01
 * @tc.desc: Main Window
 * @tc.type: FUNC
 */
HWTEST_F(WindowSessionImplTest4, IsStageDefaultDensityEnabled01, TestSize.Level1)
{
    sptr<WindowOption> mainWindowOption = sptr<WindowOption>::MakeSptr();
    mainWindowOption->SetWindowName("mainWindow");
    sptr<WindowSessionImpl> mainWindowSession = sptr<WindowSessionImpl>::MakeSptr(mainWindowOption);
    mainWindowSession->property_->SetWindowType(WindowType::APP_MAIN_WINDOW_BASE);
    mainWindowSession->context_ = abilityContext_;
    mainWindowSession->defaultDensityEnabledStageConfig_.store(false);
    mainWindowSession->windowSessionMap_.["mainWindow"] =
        std::pair<int32_t, sptr<WindowSessionImpl>>(1, mainWindowSession);

    mainWindowSession->property_->SetWindowType(WindowType::APP_MAIN_WINDOW_BASE);
    mainWindowSession->defaultDensityEnabledStageConfig_.store(true);
    EXPECT_TRUE(mainWindowSession->IsStageDefaultDensityEnabled());

    window->defaultDensityEnabledStageConfig_.store(false);
    EXPECT_FALSE(mainWindowSession->IsStageDefaultDensityEnabled());
}

/**
 * @tc.name: IsStageDefaultDensityEnabled02
 * @tc.desc: Sub Window
 * @tc.type: FUNC
 */
HWTEST_F(WindowSessionImplTest4, IsStageDefaultDensityEnabled02, TestSize.Level1)
{
    sptr<WindowOption> mainWindowOption = sptr<WindowOption>::MakeSptr();
    mainWindowOption->SetWindowName("mainWindow");
    sptr<WindowSessionImpl> mainWindowSession = sptr<WindowSessionImpl>::MakeSptr(mainWindowOption);
    mainWindowSession->property_->SetWindowType(WindowType::APP_MAIN_WINDOW_BASE);
    mainWindowSession->context_ = abilityContext_;
    mainWindowSession->defaultDensityEnabledStageConfig_.store(false);
    mainWindowSession->windowSessionMap_.["mainWindow"] =
        std::pair<int32_t, sptr<WindowSessionImpl>>(1, mainWindowSession);

    sptr<WindowOption> subWindowOption = sptr<WindowOption>::MakeSptr();
    subWindowOption->SetWindowName("subWindow");
    sptr<WindowSessionImpl> subWindowSession = sptr<WindowSessionImpl>::MakeSptr(subWindowOption);
    subWindowSession->property_->SetWindowType(WindowType::WINDOW_TYPE_APP_SUB_WINDOW);
    subWindowSession->context_ = abilityContext_;
    subWindowSession->defaultDensityEnabledStageConfig_.store(true);
    WindowSessionImpl::windowSessionMap_.insert(std::make_pair("subWindow", subWindowSession));
    subWindowSession->windowSessionMap_.["subWindow"] =
        std::pair<int32_t, sptr<WindowSessionImpl>>(2, subWindowSession);

    EXPECT_FALSE(subWindowSession->IsStageDefaultDensityEnabled());

    mainWindowSession->defaultDensityEnabledStageConfig_.store(true);
    EXPECT_TRUE(subWindowSession->IsStageDefaultDensityEnabled());
}
}
} // namespace
} // namespace Rosen
} // namespace OHOS
