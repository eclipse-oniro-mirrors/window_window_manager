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
#include "window_helper.h"
#include "window_session_impl.h"
#include "wm_common.h"
#include "mock_uicontent.h"
#include "mock_window.h"
#include "parameters.h"
#include "scene_board_judgement.h"

using namespace testing;
using namespace testing::ext;

namespace OHOS {
namespace Rosen {
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

void WindowSessionImplTest4::SetUpTestCase()
{
}

void WindowSessionImplTest4::TearDownTestCase()
{
}

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
HWTEST_F(WindowSessionImplTest4, GetRequestWindowState, Function | SmallTest | Level2)
{
    GTEST_LOG_(INFO) << "WindowSessionImplTest4: GetRequestWindowStatetest01 start";
    sptr<WindowOption> option = new WindowOption();
    ASSERT_NE(option, nullptr);
    option->SetWindowName("GetRequestWindowState");
    sptr<WindowSessionImpl> window = new (std::nothrow) WindowSessionImpl(option);
    ASSERT_NE(window, nullptr);
    auto ret = window->GetRequestWindowState();
    ASSERT_EQ(ret, WindowState::STATE_INITIAL);
    GTEST_LOG_(INFO) << "WindowSessionImplTest4: GetRequestWindowStatetest01 end";
}

/**
 * @tc.name: GetFocusabletest01
 * @tc.desc: GetFocusable
 * @tc.type: FUNC
 */
HWTEST_F(WindowSessionImplTest4, GetFocusable, Function | SmallTest | Level2)
{
    GTEST_LOG_(INFO) << "WindowSessionImplTest4: GetFocusabletest01 start";
    sptr<WindowOption> option = new WindowOption();
    ASSERT_NE(option, nullptr);
    option->SetWindowName("GetFocusable");
    sptr<WindowSessionImpl> window = new (std::nothrow) WindowSessionImpl(option);
    ASSERT_NE(window, nullptr);
    bool ret = window->GetFocusable();
    ASSERT_EQ(ret, true);
    GTEST_LOG_(INFO) << "WindowSessionImplTest4: GetFocusabletest01 end";
}


/**
 * @tc.name: TransferAccessibilityEvent
 * @tc.desc: TransferAccessibilityEvent
 * @tc.type: FUNC
 */
HWTEST_F(WindowSessionImplTest4, TransferAccessibilityEvent, Function | SmallTest | Level2)
{
    GTEST_LOG_(INFO) << "WindowSessionImplTest4: TransferAccessibilityEvent start";
    sptr<WindowOption> option = new WindowOption();
    ASSERT_NE(option, nullptr);
    sptr<WindowSessionImpl> window = new (std::nothrow) WindowSessionImpl(option);
    ASSERT_NE(window, nullptr);
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
HWTEST_F(WindowSessionImplTest4, SetSingleFrameComposerEnabled01, Function | SmallTest | Level2)
{
    sptr<WindowOption> option = new WindowOption();
    option->SetWindowName("SetSingleFrameComposerEnabled01");
    sptr<WindowSessionImpl> window = new(std::nothrow) WindowSessionImpl(option);
    ASSERT_NE(nullptr, window);
    WMError retCode = window->SetSingleFrameComposerEnabled(false);
    ASSERT_EQ(retCode, WMError::WM_ERROR_INVALID_WINDOW);
    window->property_->SetPersistentId(1);
    SessionInfo sessionInfo = { "CreateTestBundle", "CreateTestModule", "CreateTestAbility" };
    sptr<SessionMocker> session = new(std::nothrow) SessionMocker(sessionInfo);
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
 * @tc.name: SetTopmost
 * @tc.desc: SetTopmost
 * @tc.type: FUNC
 */
HWTEST_F(WindowSessionImplTest4, SetTopmost, Function | SmallTest | Level2)
{
    sptr<WindowOption> option = new WindowOption();
    option->SetWindowName("SetTopmost");
    sptr<WindowSessionImpl> window = new (std::nothrow) WindowSessionImpl(option);
    ASSERT_NE(nullptr, window);
    window->windowSystemConfig_.uiType_ = "phone";
    WMError res = window->SetTopmost(true);
    ASSERT_EQ(WMError::WM_ERROR_DEVICE_NOT_SUPPORT, res);
    window->windowSystemConfig_.uiType_ = "pc";
    res = window->SetTopmost(true);
    ASSERT_EQ(WMError::WM_ERROR_INVALID_WINDOW, res);

    window->property_->SetPersistentId(1);
    SessionInfo sessionInfo = {"CreateTestBundle", "CreateTestModule", "CreateTestAbility"};
    sptr<SessionMocker> session = new (std::nothrow) SessionMocker(sessionInfo);
    ASSERT_NE(nullptr, session);
    window->hostSession_ = session;
    window->state_ = WindowState::STATE_CREATED;
    res = window->SetTopmost(true);
    ASSERT_EQ(WMError::WM_OK, res);
}

/**
 * @tc.name: IsTopmost
 * @tc.desc: IsTopmost
 * @tc.type: FUNC
 */
HWTEST_F(WindowSessionImplTest4, IsTopmost, Function | SmallTest | Level2)
{
    sptr<WindowOption> option = new WindowOption();
    option->SetWindowName("IsTopmost");
    sptr<WindowSessionImpl> window = new WindowSessionImpl(option);
    ASSERT_NE(window, nullptr);
    bool res = window->IsTopmost();
    ASSERT_FALSE(res);
}

/**
 * @tc.name: SetMainWindowTopmost
 * @tc.desc: SetMainWindowTopmost
 * @tc.type: FUNC
 */
HWTEST_F(WindowSessionImplTest4, SetMainWindowTopmost, Function | SmallTest | Level2)
{
    sptr<WindowOption> option = sptr<WindowOption>::MakeSptr();
    option->SetWindowName("SetMainWindowTopmost");
    sptr<WindowSessionImpl> window = sptr<WindowSessionImpl>::MakeSptr(option);

    window->property_->SetPersistentId(1);
    SessionInfo sessionInfo = {"CreateTestBundle", "CreateTestModule", "CreateTestAbility"};
    sptr<SessionMocker> session = new (std::nothrow) SessionMocker(sessionInfo);
    ASSERT_NE(nullptr, session);
    window->hostSession_ = session;
    window->state_ = WindowState::STATE_CREATED;
    WMError res = window->SetMainWindowTopmost(true);
    ASSERT_EQ(WMError::WM_OK, res);
}

/**
 * @tc.name: IsMainWindowTopmost
 * @tc.desc: IsMainWindowTopmost
 * @tc.type: FUNC
 */
HWTEST_F(WindowSessionImplTest4, IsMainWindowTopmost, Function | SmallTest | Level2)
{
    sptr<WindowOption> option = new WindowOption();
    ASSERT_NE(option, nullptr);
    option->SetWindowName("IsMainWindowTopmost");
    sptr<WindowSessionImpl> window = new WindowSessionImpl(option);
    ASSERT_NE(window, nullptr);
    bool res = window->IsMainWindowTopmost();
    ASSERT_FALSE(res);
}

/**
 * @tc.name: SetDecorVisible
 * @tc.desc: SetDecorVisible and check the retCode
 * @tc.type: FUNC
 */
HWTEST_F(WindowSessionImplTest4, SetDecorVisible, Function | SmallTest | Level2)
{
    GTEST_LOG_(INFO) << "WindowSessionImplTest4: SetDecorVisibletest01 start";
    sptr<WindowOption> option = new WindowOption();
    ASSERT_NE(option, nullptr);
    option->SetWindowName("SetDecorVisible");
    sptr<WindowSessionImpl> window = new (std::nothrow) WindowSessionImpl(option);
    ASSERT_NE(window, nullptr);
    ASSERT_NE(window->property_, nullptr);
    window->property_->SetPersistentId(1);
    SessionInfo sessionInfo = {"CreateTestBundle", "CreateTestModule", "CreateTestAbility"};
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
 * @tc.name: SetSubWindowModal
 * @tc.desc: SetSubWindowModal and check the retCode
 * @tc.type: FUNC
 */
HWTEST_F(WindowSessionImplTest4, SetSubWindowModal, Function | SmallTest | Level2)
{
    GTEST_LOG_(INFO) << "WindowSessionImplTest4: SetSubWindowModaltest01 start";
    sptr<WindowOption> option = sptr<WindowOption>::MakeSptr();
    ASSERT_NE(option, nullptr);
    option->SetWindowName("SetSubWindowModal");
    option->SetWindowType(WindowType::WINDOW_TYPE_APP_SUB_WINDOW);
    sptr<WindowSessionImpl> window = sptr<WindowSessionImpl>::MakeSptr(option);
    ASSERT_NE(window, nullptr);
    window->property_->SetPersistentId(1);
    SessionInfo sessionInfo = {"CreateTestBundle", "CreateTestModule", "CreateTestAbility"};
    sptr<SessionMocker> session = sptr<SessionMocker>::MakeSptr(sessionInfo);
    ASSERT_NE(nullptr, session);
    window->hostSession_ = session;
    WMError res = window->SetSubWindowModal(true);
    ASSERT_EQ(res, WMError::WM_OK);
    GTEST_LOG_(INFO) << "WindowSessionImplTest4: SetSubWindowModaltest01 end";
}

/**
 * @tc.name: SetSubWindowModal02
 * @tc.desc: SetSubWindowModal and check the retCode
 * @tc.type: FUNC
 */
HWTEST_F(WindowSessionImplTest4, SetSubWindowModal02, Function | SmallTest | Level2)
{
    GTEST_LOG_(INFO) << "WindowSessionImplTest4: SetSubWindowModaltest02 start";
    sptr<WindowOption> option = sptr<WindowOption>::MakeSptr();
    ASSERT_NE(option, nullptr);
    option->SetWindowName("SetSubWindowModal02");
    option->SetWindowType(WindowType::WINDOW_TYPE_APP_SUB_WINDOW);
    sptr<WindowSessionImpl> window = sptr<WindowSessionImpl>::MakeSptr(option);
    ASSERT_NE(window, nullptr);
    window->property_->SetPersistentId(1);
    SessionInfo sessionInfo = {"CreateTestBundle", "CreateTestModule", "CreateTestAbility"};
    sptr<SessionMocker> session = sptr<SessionMocker>::MakeSptr(sessionInfo);
    ASSERT_NE(nullptr, session);
    window->hostSession_ = session;
    window->windowSystemConfig_.uiType_ = "pc";
    WMError res = window->SetSubWindowModal(true, ModalityType::WINDOW_MODALITY);
    ASSERT_EQ(res, WMError::WM_OK);
    res = window->SetSubWindowModal(true, ModalityType::APPLICATION_MODALITY);
    ASSERT_EQ(res, WMError::WM_OK);
    GTEST_LOG_(INFO) << "WindowSessionImplTest4: SetSubWindowModaltest02 end";
}

/**
 * @tc.name: IsPcOrPadFreeMultiWindowMode
 * @tc.desc: IsPcOrPadFreeMultiWindowMode
 * @tc.type: FUNC
 */
HWTEST_F(WindowSessionImplTest4, IsPcOrPadFreeMultiWindowMode, Function | SmallTest | Level2)
{
    GTEST_LOG_(INFO) << "WindowSessionImplTest4: IsPcOrPadFreeMultiWindowMode start";
    sptr<WindowOption> option = sptr<WindowOption>::MakeSptr();
    ASSERT_NE(option, nullptr);
    option->SetWindowName("IsPcOrPadFreeMultiWindowMode");
    option->SetWindowType(WindowType::WINDOW_TYPE_APP_SUB_WINDOW);
    sptr<WindowSessionImpl> window = sptr<WindowSessionImpl>::MakeSptr(option);
    ASSERT_NE(window, nullptr);
    window->property_->SetPersistentId(1);
    SessionInfo sessionInfo = {"CreateTestBundle", "CreateTestModule", "CreateTestAbility"};
    sptr<SessionMocker> session = sptr<SessionMocker>::MakeSptr(sessionInfo);
    ASSERT_NE(nullptr, session);
    window->hostSession_ = session;
    window->windowSystemConfig_.uiType_ = "pc";
    ASSERT_EQ(true, window->IsPcOrPadFreeMultiWindowMode());
    window->windowSystemConfig_.uiType_ = "phone";
    ASSERT_EQ(false, window->IsPcOrPadFreeMultiWindowMode());
    GTEST_LOG_(INFO) << "WindowSessionImplTest4: IsPcOrPadFreeMultiWindowMode end";
}

/**
 * @tc.name: GetDecorHeight
 * @tc.desc: GetDecorHeight and check the retCode
 * @tc.type: FUNC
 */
HWTEST_F(WindowSessionImplTest4, GetDecorHeight, Function | SmallTest | Level2)
{
    GTEST_LOG_(INFO) << "WindowSessionImplTest4: GetDecorHeighttest01 start";
    sptr<WindowOption> option = new WindowOption();
    ASSERT_NE(option, nullptr);
    option->SetWindowName("GetDecorHeight");
    sptr<WindowSessionImpl> window = new (std::nothrow) WindowSessionImpl(option);
    ASSERT_NE(window, nullptr);
    ASSERT_NE(window->property_, nullptr);
    window->property_->SetPersistentId(1);
    SessionInfo sessionInfo = {"CreateTestBundle", "CreateTestModule", "CreateTestAbility"};
    sptr<SessionMocker> session = sptr<SessionMocker>::MakeSptr(sessionInfo);
    ASSERT_NE(nullptr, session);
    window->hostSession_ = session;
    int32_t height = 0;
    WMError res = window->GetDecorHeight(height);
    ASSERT_EQ(res, WMError::WM_ERROR_NULLPTR);
    GTEST_LOG_(INFO) << "WindowSessionImplTest4: GetDecorHeighttest01 end";
}

/**
 * @tc.name: GetTitleButtonArea
 * @tc.desc: GetTitleButtonArea and check the retCode
 * @tc.type: FUNC
 */
HWTEST_F(WindowSessionImplTest4, GetTitleButtonArea, Function | SmallTest | Level2)
{
    GTEST_LOG_(INFO) << "WindowSessionImplTest4: GetTitleButtonAreatest01 start";
    sptr<WindowOption> option = new WindowOption();
    ASSERT_NE(option, nullptr);
    option->SetWindowName("GetTitleButtonArea");
    sptr<WindowSessionImpl> window = new (std::nothrow) WindowSessionImpl(option);
    ASSERT_NE(window, nullptr);
    ASSERT_NE(window->property_, nullptr);
    window->property_->SetPersistentId(1);
    SessionInfo sessionInfo = {"CreateTestBundle", "CreateTestModule", "CreateTestAbility"};
    sptr<SessionMocker> session = sptr<SessionMocker>::MakeSptr(sessionInfo);
    ASSERT_NE(nullptr, session);
    window->hostSession_ = session;
    TitleButtonRect titleButtonRect;
    WMError res = window->GetTitleButtonArea(titleButtonRect);
    ASSERT_EQ(res, WMError::WM_ERROR_NULLPTR);
    GTEST_LOG_(INFO) << "WindowSessionImplTest4: GetDecorHeighttest01 end";
}

/**
 * @tc.name: RegisterExtensionAvoidAreaChangeListener
 * @tc.desc: RegisterExtensionAvoidAreaChangeListener Test
 * @tc.type: FUNC
 */
HWTEST_F(WindowSessionImplTest4, RegisterExtensionAvoidAreaChangeListener, Function | SmallTest | Level2)
{
    GTEST_LOG_(INFO) << "WindowSessionImplTest4: RegisterExtensionAvoidAreaChangeListener start";
    sptr<WindowOption> option = new WindowOption();
    ASSERT_NE(option, nullptr);
    option->SetWindowName("GetTitleButtonArea");
    sptr<WindowSessionImpl> window = new (std::nothrow) WindowSessionImpl(option);
    ASSERT_NE(window, nullptr);
    sptr<IAvoidAreaChangedListener> listener = nullptr;
    WMError res = window->RegisterExtensionAvoidAreaChangeListener(listener);
    ASSERT_EQ(res, WMError::WM_ERROR_NULLPTR);
    GTEST_LOG_(INFO) << "WindowSessionImplTest4: RegisterExtensionAvoidAreaChangeListener end";
}

/**
 * @tc.name: UnregisterExtensionAvoidAreaChangeListener
 * @tc.desc: UnregisterExtensionAvoidAreaChangeListener Test
 * @tc.type: FUNC
 */
HWTEST_F(WindowSessionImplTest4, UnregisterExtensionAvoidAreaChangeListener, Function | SmallTest | Level2)
{
    GTEST_LOG_(INFO) << "WindowSessionImplTest4: UnregisterExtensionAvoidAreaChangeListener start";
    sptr<WindowOption> option = new WindowOption();
    ASSERT_NE(option, nullptr);
    option->SetWindowName("GetTitleButtonArea");
    sptr<WindowSessionImpl> window = new (std::nothrow) WindowSessionImpl(option);
    ASSERT_NE(window, nullptr);
    sptr<IAvoidAreaChangedListener> listener = nullptr;
    WMError res = window->UnregisterExtensionAvoidAreaChangeListener(listener);
    ASSERT_EQ(res, WMError::WM_ERROR_NULLPTR);
    GTEST_LOG_(INFO) << "WindowSessionImplTest4: UnregisterExtensionAvoidAreaChangeListener end";
}

/**
 * @tc.name: SetPipActionEvent
 * @tc.desc: SetPipActionEvent Test
 * @tc.type: FUNC
 */
HWTEST_F(WindowSessionImplTest4, SetPipActionEvent, Function | SmallTest | Level2)
{
    GTEST_LOG_(INFO) << "WindowSessionImplTest4: SetPipActionEvent start";
    sptr<WindowOption> option = new WindowOption();
    ASSERT_NE(option, nullptr);
    option->SetWindowName("GetTitleButtonArea");
    sptr<WindowSessionImpl> window = new (std::nothrow) WindowSessionImpl(option);
    ASSERT_NE(window, nullptr);
    ASSERT_EQ(nullptr, window->GetUIContentWithId(10000));
    window->property_->SetPersistentId(1);

    SessionInfo sessionInfo = { "CreateTestBundle", "TestGetUIContentWithId", "CreateTestAbility" };
    sptr<SessionMocker> session = new(std::nothrow) SessionMocker(sessionInfo);
    ASSERT_NE(nullptr, session);
    ASSERT_EQ(WMError::WM_OK, window->Create(nullptr, session));
    window->uiContent_ = std::make_unique<Ace::UIContentMocker>();
    ASSERT_EQ(window->FindWindowById(1), nullptr);
    ASSERT_EQ(nullptr, window->GetUIContentWithId(1));
    ASSERT_EQ(WMError::WM_ERROR_INVALID_WINDOW, window->Destroy());
    GTEST_LOG_(INFO) << "WindowSessionImplTest4: SetPipActionEvent end";
}

/**
 * @tc.name: SetPiPControlEvent
 * @tc.desc: SetPiPControlEvent Test
 * @tc.type: FUNC
 */
HWTEST_F(WindowSessionImplTest4, SetPiPControlEvent, Function | SmallTest | Level2)
{
    GTEST_LOG_(INFO) << "WindowSessionImplTest4: SetPiPControlEvent start";
    auto option = sptr<WindowOption>::MakeSptr();
    ASSERT_NE(option, nullptr);
    option->SetWindowName("GetTitleButtonArea");
    auto window = sptr<WindowSessionImpl>::MakeSptr(option);
    ASSERT_NE(window, nullptr);
    auto controlType = WsPiPControlType::VIDEO_PLAY_PAUSE;
    auto status = WsPiPControlStatus::PLAY;
    WSError res = window->SetPiPControlEvent(controlType, status);
    ASSERT_EQ(res, WSError::WS_OK);
    GTEST_LOG_(INFO) << "WindowSessionImplTest4: SetPiPControlEvent end";
}

/**
 * @tc.name: SetAutoStartPiP
 * @tc.desc: SetAutoStartPiP
 * @tc.type: FUNC
 */
HWTEST_F(WindowSessionImplTest4, SetAutoStartPiP, Function | SmallTest | Level2)
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
    window->SetAutoStartPiP(isAutoStart, priority);
    window->hostSession_ = nullptr;
    window->SetAutoStartPiP(isAutoStart, priority);
}

/**
 * @tc.name: TestGetUIContentWithId
 * @tc.desc: Get uicontent with id
 * @tc.type: FUNC
 */
HWTEST_F(WindowSessionImplTest4, TestGetUIContentWithId, Function | SmallTest | Level2)
{
    GTEST_LOG_(INFO) << "WindowSessionImplTest4: TestGetUIContentWithId start";
    sptr<WindowOption> option = new WindowOption();
    ASSERT_NE(nullptr, option);
    option->SetWindowName("TestGetUIContentWithId");
    sptr<WindowSessionImpl> window = new WindowSessionImpl(option);
    ASSERT_NE(nullptr, window);
    ASSERT_EQ(nullptr, window->GetUIContentWithId(10000));
    window->property_->SetPersistentId(1);

    SessionInfo sessionInfo = { "CreateTestBundle", "TestGetUIContentWithId", "CreateTestAbility" };
    sptr<SessionMocker> session = new(std::nothrow) SessionMocker(sessionInfo);
    ASSERT_NE(nullptr, session);
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
HWTEST_F(WindowSessionImplTest4, GetCallingWindowRect, Function | SmallTest | Level2)
{
    sptr<WindowOption> option = new WindowOption();
    option->SetWindowName("GetCallingWindowRect");
    sptr<WindowSessionImpl> window = new(std::nothrow) WindowSessionImpl(option);
    ASSERT_NE(nullptr, window);
    Rect rect = {0, 0, 0, 0};
    WMError retCode = window->GetCallingWindowRect(rect);
    ASSERT_EQ(retCode, WMError::WM_ERROR_INVALID_WINDOW);
    window->property_->SetPersistentId(1);
    SessionInfo sessionInfo = { "CreateTestBundle", "CreateTestModule", "CreateTestAbility" };
    sptr<SessionMocker> session = new(std::nothrow) SessionMocker(sessionInfo);
    ASSERT_NE(nullptr, session);
    window->hostSession_ = session;
    window->state_ = WindowState::STATE_CREATED;
    window->GetCallingWindowRect(rect);
}

/**
 * @tc.name: GetCallingWindowWindowStatus
 * @tc.desc: GetCallingWindowWindowStatus Test
 * @tc.type: FUNC
 */
HWTEST_F(WindowSessionImplTest4, GetCallingWindowWindowStatus, Function | SmallTest | Level2)
{
    sptr<WindowOption> option = new WindowOption();
    option->SetWindowName("GetCallingWindowWindowStatus");
    sptr<WindowSessionImpl> window = new(std::nothrow) WindowSessionImpl(option);
    ASSERT_NE(nullptr, window);
    WindowStatus windowStatus = WindowStatus::WINDOW_STATUS_UNDEFINED;
    WMError retCode = window->GetCallingWindowWindowStatus(windowStatus);
    ASSERT_EQ(retCode, WMError::WM_ERROR_INVALID_WINDOW);
    window->property_->SetPersistentId(1);
    SessionInfo sessionInfo = { "CreateTestBundle", "CreateTestModule", "CreateTestAbility" };
    sptr<SessionMocker> session = new(std::nothrow) SessionMocker(sessionInfo);
    ASSERT_NE(nullptr, session);
    window->hostSession_ = session;
    window->state_ = WindowState::STATE_CREATED;
    window->GetCallingWindowWindowStatus(windowStatus);
}

/**
 * @tc.name: GetParentId
 * @tc.desc: GetParentId Test
 * @tc.type: FUNC
 */
HWTEST_F(WindowSessionImplTest4, GetParentId, Function | SmallTest | Level2)
{
    sptr<WindowOption> option = new WindowOption();
    sptr<WindowSessionImpl> window = new WindowSessionImpl(option);
    const int32_t res = window->GetParentId();
    ASSERT_EQ(res, 0);
    ASSERT_EQ(true, window->IsSupportWideGamut());
}

/**
 * @tc.name: PreNotifyKeyEvent
 * @tc.desc: PreNotifyKeyEvent Test
 * @tc.type: FUNC
 */
HWTEST_F(WindowSessionImplTest4, PreNotifyKeyEvent, Function | SmallTest | Level2)
{
    sptr<WindowOption> option = new (std::nothrow) WindowOption();
    ASSERT_NE(nullptr, option);
    sptr<WindowSessionImpl> window = new (std::nothrow) WindowSessionImpl(option);
    ASSERT_NE(nullptr, window);
    std::shared_ptr<MMI::PointerEvent> pointerEvent;
    window->ConsumePointerEvent(pointerEvent);

    std::shared_ptr<MMI::KeyEvent> keyEvent;
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
 * @tc.name: UpdateRectForRotation
 * @tc.desc: UpdateRectForRotation Test
 * @tc.type: FUNC
 */
HWTEST_F(WindowSessionImplTest4, UpdateRectForRotation, Function | SmallTest | Level2)
{
    sptr<WindowOption> option = new WindowOption();
    option->SetWindowName("WindowSessionCreateCheck");
    sptr<WindowSessionImpl> window =
        new (std::nothrow) WindowSessionImpl(option);
    ASSERT_NE(window, nullptr);

    Rect wmRect;
    wmRect.posX_ = 0;
    wmRect.posY_ = 0;
    wmRect.height_ = 50;
    wmRect.width_ = 50;

    WSRect rect;
    wmRect.posX_ = 0;
    wmRect.posY_ = 0;
    wmRect.height_ = 50;
    wmRect.width_ = 50;

    Rect preRect;
    preRect.posX_ = 0;
    preRect.posY_ = 0;
    preRect.height_ = 200;
    preRect.width_ = 200;

    window->property_->SetWindowRect(preRect);
    WindowSizeChangeReason wmReason = WindowSizeChangeReason{0};
    std::shared_ptr<RSTransaction> rsTransaction;
    SceneAnimationConfig config { .rsTransaction_ = rsTransaction };
    window->UpdateRectForRotation(wmRect, preRect, wmReason, config);

    SizeChangeReason reason = SizeChangeReason::UNDEFINED;
    auto res = window->UpdateRect(rect, reason);
    ASSERT_EQ(res, WSError::WS_OK);
}

/**
 * @tc.name: SetTitleButtonVisible
 * @tc.desc: SetTitleButtonVisible and GetTitleButtonVisible
 * @tc.type: FUNC
 */
HWTEST_F(WindowSessionImplTest4, SetTitleButtonVisible, Function | SmallTest | Level2)
{
    sptr<WindowOption> option = new WindowOption();
    sptr<WindowSessionImpl> window = new WindowSessionImpl(option);
    ASSERT_NE(window, nullptr);
    bool isMaximizeVisible = true;
    bool isMinimizeVisible = true;
    bool isSplitVisible = true;
    bool isCloseVisible = true;
    auto res = window->SetTitleButtonVisible(isMaximizeVisible, isMinimizeVisible,
        isSplitVisible, isCloseVisible);

    bool &hideMaximizeButton = isMaximizeVisible;
    bool &hideMinimizeButton = isMinimizeVisible;
    bool &hideSplitButton = isSplitVisible;
    bool &hideCloseButton = isCloseVisible;
    window->windowSystemConfig_.uiType_ = "pc";
    window->GetTitleButtonVisible(hideMaximizeButton, hideMinimizeButton,
        hideSplitButton, hideCloseButton);
    ASSERT_EQ(res, WMError::WM_ERROR_INVALID_WINDOW);
}

/**
 * @tc.name: IsFocused
 * @tc.desc: IsFocused
 * @tc.type: FUNC
 */
HWTEST_F(WindowSessionImplTest4, IsFocused, Function | SmallTest | Level2)
{
    sptr<WindowOption> option = new WindowOption();
    option->SetWindowName("WindowSessionCreateCheck");
    sptr<WindowSessionImpl> window =
        new (std::nothrow) WindowSessionImpl(option);
    ASSERT_NE(window, nullptr);
    bool res = window->IsFocused();
    ASSERT_EQ(res, false);

    ASSERT_EQ(WMError::WM_ERROR_INVALID_WINDOW, window->RequestFocus());

    SessionInfo sessionInfo = {"CreateTestBundle", "CreateTestModule",
                               "CreateTestAbility"};
    sptr<SessionMocker> session = new (std::nothrow) SessionMocker(sessionInfo);
    ASSERT_NE(nullptr, session);
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
 * @tc.name: GetAbcContent
 * @tc.desc: GetAbcContent Test
 * @tc.type: FUNC
 */
HWTEST_F(WindowSessionImplTest4, GetAbcContent, Function | SmallTest | Level2)
{
    sptr<WindowOption> option = new WindowOption();
    ASSERT_NE(option, nullptr);
    option->SetWindowName("GetAbcContent");
    sptr<WindowSessionImpl> window = new (std::nothrow) WindowSessionImpl(option);
    ASSERT_NE(window, nullptr);
    std::string abcPath = "";
    std::shared_ptr<std::vector<uint8_t>> res = window->GetAbcContent(abcPath);
    std::filesystem::path abcFile{abcPath};
    ASSERT_TRUE(abcFile.empty());
    ASSERT_TRUE(!abcFile.is_absolute());
    ASSERT_TRUE(!std::filesystem::exists(abcFile));
    ASSERT_EQ(res, nullptr);

    abcPath = "/abc";
    res = window->GetAbcContent(abcPath);
    std::filesystem::path abcFile2{abcPath};
    ASSERT_FALSE(abcFile2.empty());
    ASSERT_FALSE(!abcFile2.is_absolute());
    ASSERT_TRUE(!std::filesystem::exists(abcFile2));
    ASSERT_EQ(res, nullptr);

    abcPath = "abc";
    res = window->GetAbcContent(abcPath);
    std::filesystem::path abcFile3{abcPath};
    ASSERT_FALSE(abcFile3.empty());
    ASSERT_TRUE(!abcFile3.is_absolute());
    ASSERT_TRUE(!std::filesystem::exists(abcFile3));
    ASSERT_EQ(res, nullptr);

    abcPath = "/log";
    res = window->GetAbcContent(abcPath);
    std::filesystem::path abcFile4{abcPath};
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
HWTEST_F(WindowSessionImplTest4, SetLandscapeMultiWindow, Function | SmallTest | Level2)
{
    sptr<WindowOption> option = new WindowOption();
    option->SetWindowName("SetLandscapeMultiWindow");
    sptr<WindowSessionImpl> window = new(std::nothrow) WindowSessionImpl(option);
    ASSERT_NE(nullptr, window);
    WMError retCode = window->SetLandscapeMultiWindow(false);
    ASSERT_EQ(retCode, WMError::WM_ERROR_INVALID_WINDOW);
    window->property_->SetPersistentId(1);
    SessionInfo sessionInfo = { "CreateTestBundle", "CreateTestModule", "CreateTestAbility" };
    sptr<SessionMocker> session = new(std::nothrow) SessionMocker(sessionInfo);
    ASSERT_NE(nullptr, session);
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
HWTEST_F(WindowSessionImplTest4, GetTouchable, Function | SmallTest | Level2)
{
    sptr<WindowOption> option = new WindowOption();
    ASSERT_NE(option, nullptr);
    option->SetWindowName("GetTouchable");
    sptr<WindowSessionImpl> window = new (std::nothrow) WindowSessionImpl(option);
    ASSERT_NE(window, nullptr);
    window->GetTouchable();
    window->GetBrightness();
    ASSERT_NE(window, nullptr);
}

/**
 * @tc.name: Notify03
 * @tc.desc: NotifyCloseExistPipWindow NotifyAfterResumed NotifyAfterPaused
 * @tc.type: FUNC
 */
HWTEST_F(WindowSessionImplTest4, Notify03, Function | SmallTest | Level2)
{
    sptr<WindowOption> option = new WindowOption();
    option->SetWindowName("Notify03");
    sptr<WindowSessionImpl> window = new WindowSessionImpl(option);

    SessionInfo sessionInfo = {"CreateTestBundle", "CreateTestModule",
                               "CreateTestAbility"};
    sptr<SessionMocker> session = new (std::nothrow) SessionMocker(sessionInfo);
    ASSERT_NE(nullptr, session);
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
HWTEST_F(WindowSessionImplTest4, Filter, Function | SmallTest | Level2)
{
    sptr<WindowOption> option = new WindowOption();
    ASSERT_NE(option, nullptr);
    option->SetWindowName("Filter");
    sptr<WindowSessionImpl> window = new (std::nothrow) WindowSessionImpl(option);
    ASSERT_NE(window, nullptr);
    std::shared_ptr<MMI::KeyEvent> keyEvent = MMI::KeyEvent::Create();
    window->FilterKeyEvent(keyEvent);
    ASSERT_EQ(window->keyEventFilter_, nullptr);
    window->SetKeyEventFilter([](MMI::KeyEvent& keyEvent) {
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
HWTEST_F(WindowSessionImplTest4, UpdateOrientation, Function | SmallTest | Level2)
{
    sptr<WindowOption> option = new WindowOption();
    ASSERT_NE(option, nullptr);
    option->SetWindowName("UpdateOrientation");
    sptr<WindowSessionImpl> window = new (std::nothrow) WindowSessionImpl(option);
    ASSERT_NE(window, nullptr);
    auto ret = window->UpdateOrientation();
    ASSERT_EQ(WSError::WS_OK, ret);
}

/**
 * @tc.name: SetTitleButtonVisible01
 * @tc.desc: SetTitleButtonVisible
 * @tc.type: FUNC
*/
HWTEST_F(WindowSessionImplTest4, SetTitleButtonVisible01, Function | SmallTest | Level2)
{
    GTEST_LOG_(INFO) << "WindowSessionImplTest4: SetTitleButtonVisible01 start";
    sptr<WindowOption> option = new (std::nothrow) WindowOption();
    ASSERT_NE(option, nullptr);
    option->SetWindowName("SetTitleButtonVisible");
    sptr<WindowSessionImpl> window = new (std::nothrow) WindowSessionImpl(option);
    ASSERT_NE(window, nullptr);
    SessionInfo sessionInfo = {"CreateTestBundle", "CreateTestModule", "CreateTestAbility"};
    sptr<SessionMocker> session = sptr<SessionMocker>::MakeSptr(sessionInfo);
    ASSERT_NE(nullptr, session);
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
HWTEST_F(WindowSessionImplTest4, SetTitleButtonVisible02, Function | SmallTest | Level2)
{
    GTEST_LOG_(INFO) << "WindowSessionImplTest4: SetTitleButtonVisible02 start";
    sptr<WindowOption> option = new (std::nothrow) WindowOption();
    ASSERT_NE(option, nullptr);
    option->SetWindowName("SetTitleButtonVisible");
    sptr<WindowSessionImpl> window = new (std::nothrow) WindowSessionImpl(option);
    ASSERT_NE(window, nullptr);
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
HWTEST_F(WindowSessionImplTest4, SetTitleButtonVisible03, Function | SmallTest | Level2)
{
    GTEST_LOG_(INFO) << "WindowSessionImplTest4: SetTitleButtonVisible03 start";
    sptr option = new (std::nothrow) WindowOption();
    ASSERT_NE(option, nullptr);
    option->SetWindowName("SetTitleButtonVisible");
    sptr window = new (std::nothrow) WindowSessionImpl(option);
    ASSERT_NE(window, nullptr);
    ASSERT_NE(window->property_, nullptr);
    window->property_->SetWindowType(WindowType::APP_MAIN_WINDOW_BASE);
    window->property_->SetWindowMode(WindowMode::WINDOW_MODE_FLOATING);
    window->uiContent_ = std::make_unique<Ace::UIContentMocker>();
    window->windowSystemConfig_.freeMultiWindowSupport_ = true;
    window->windowSystemConfig_.isSystemDecorEnable_ = true;
    window->windowSystemConfig_.uiType_ = "phone";
    WMError res = window->SetTitleButtonVisible(false, false, false, true);
    ASSERT_EQ(res, WMError::WM_ERROR_INVALID_WINDOW);
    window->windowSystemConfig_.uiType_ = "pc";
    res = window->SetTitleButtonVisible(false, false, false, true);
    ASSERT_EQ(res, WMError::WM_ERROR_INVALID_WINDOW);
    GTEST_LOG_(INFO) << "WindowSessionImplTest4: SetTitleButtonVisible03 end";
}

/**
 * @tc.name: GetTitleButtonVisible01
 * @tc.desc: GetTitleButtonVisible
 * @tc.type: FUNC
*/
HWTEST_F(WindowSessionImplTest4, GetTitleButtonVisible01, Function | SmallTest | Level2)
{
    sptr<WindowOption> option = new (std::nothrow) WindowOption();
    ASSERT_NE(option, nullptr);
    option->SetWindowName("GetTitleButtonVisible01");
    sptr<WindowSessionImpl> window = new (std::nothrow) WindowSessionImpl(option);
    ASSERT_NE(window, nullptr);
    ASSERT_NE(window->property_, nullptr);
    uint32_t modeSupportInfo = 1 | (1 << 1) | (1 << 2);
    window->property_->SetModeSupportInfo(modeSupportInfo);
    window->property_->SetWindowMode(WindowMode::WINDOW_MODE_FLOATING);
    // show Maximize, Minimize, Split buttons.
    window->windowTitleVisibleFlags_ = { false, false, false, false };
    bool hideMaximizeButton = false;
    bool hideMinimizeButton = false;
    bool hideSplitButton = false;
    bool hideCloseButton = false;
    window->windowSystemConfig_.uiType_ = "pc";
    window->GetTitleButtonVisible(hideMaximizeButton, hideMinimizeButton, hideSplitButton,
        hideCloseButton);
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
HWTEST_F(WindowSessionImplTest4, UpdateRect03, Function | SmallTest | Level2)
{
    sptr<WindowOption> option = new WindowOption();
    option->SetWindowName("WindowSessionCreateCheck");
    sptr<WindowSessionImpl> window = new (std::nothrow) WindowSessionImpl(option);
    ASSERT_NE(window, nullptr);

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
 * @tc.name: GetTitleButtonVisible02
 * @tc.desc: GetTitleButtonVisible
 * @tc.type: FUNC
 */
HWTEST_F(WindowSessionImplTest4, GetTitleButtonVisible02, Function | SmallTest | Level2)
{
    sptr<WindowOption> option = new (std::nothrow) WindowOption();
    ASSERT_NE(option, nullptr);
    option->SetWindowName("GetTitleButtonVisible02");
    sptr<WindowSessionImpl> window = new (std::nothrow) WindowSessionImpl(option);
    ASSERT_NE(window, nullptr);
    ASSERT_NE(window->property_, nullptr);
    // only not support WINDOW_MODE_SUPPORT_SPLIT
    uint32_t modeSupportInfo = 1 | (1 << 1);
    window->property_->SetModeSupportInfo(modeSupportInfo);
    window->property_->SetWindowMode(WindowMode::WINDOW_MODE_FLOATING);
    // show Maximize, Minimize, Split buttons.
    window->windowTitleVisibleFlags_ = { true, true, true, true };
    bool hideMaximizeButton = false;
    bool hideMinimizeButton = false;
    bool hideSplitButton = false;
    bool hideCloseButton = false;
    window->windowSystemConfig_.uiType_ = "pc";
    window->GetTitleButtonVisible(hideMaximizeButton, hideMinimizeButton, hideSplitButton, hideCloseButton);
    ASSERT_EQ(hideMaximizeButton, false);
    ASSERT_EQ(hideMinimizeButton, false);
    ASSERT_EQ(hideSplitButton, false);
    ASSERT_EQ(hideCloseButton, false);
}

/**
 * @tc.name: GetTitleButtonVisible03
 * @tc.desc: GetTitleButtonVisible
 * @tc.type: FUNC
 */
HWTEST_F(WindowSessionImplTest4, GetTitleButtonVisible03, Function | SmallTest | Level2)
{
    sptr<WindowOption> option = new (std::nothrow) WindowOption();
    ASSERT_NE(option, nullptr);
    option->SetWindowName("GetTitleButtonVisible03");
    option->SetDisplayId(1);
    sptr<WindowSessionImpl> window = new (std::nothrow) WindowSessionImpl(option);
    ASSERT_NE(window, nullptr);
    ASSERT_NE(window->property_, nullptr);
    ASSERT_EQ(1, window->GetDisplayId());
    // only not support WINDOW_MODE_SUPPORT_SPLIT
    uint32_t modeSupportInfo = 1 | (1 << 1) | (1 << 2);
    window->property_->SetModeSupportInfo(modeSupportInfo);
    window->property_->SetWindowMode(WindowMode::WINDOW_MODE_FLOATING);
    // show Maximize, Minimize, Split buttons.
    window->windowTitleVisibleFlags_ = { false, false, false, false };
    bool hideMaximizeButton = true;
    bool hideMinimizeButton = true;
    bool hideSplitButton = true;
    bool hideCloseButton = true;
    window->windowSystemConfig_.uiType_ = "phone";
    window->GetTitleButtonVisible(hideMaximizeButton, hideMinimizeButton, hideSplitButton, hideCloseButton);
    ASSERT_EQ(hideMaximizeButton, true);
    ASSERT_EQ(hideMinimizeButton, true);
    ASSERT_EQ(hideSplitButton, true);
    ASSERT_EQ(hideCloseButton, true);
}

/**
 * @tc.name: SetUiDvsyncSwitch
 * @tc.desc: SetUiDvsyncSwitch
 * @tc.type: FUNC
 */
HWTEST_F(WindowSessionImplTest4, SetUiDvsyncSwitch, Function | SmallTest | Level2)
{
    sptr<WindowOption> option = new (std::nothrow) WindowOption();
    ASSERT_NE(option, nullptr);
    option->SetWindowName("SetUiDvsyncSwitch");
    sptr<WindowSessionImpl> window = new (std::nothrow) WindowSessionImpl(option);
    ASSERT_NE(window, nullptr);
    window->SetUiDvsyncSwitch(true);
    window->vsyncStation_ = nullptr;
    window->SetUiDvsyncSwitch(true);
}

/**
 * @tc.name: GetVSyncPeriod
 * @tc.desc: GetVSyncPeriod
 * @tc.type: FUNC
 */
HWTEST_F(WindowSessionImplTest4, GetVSyncPeriod, Function | SmallTest | Level2)
{
    sptr<WindowOption> option = new (std::nothrow) WindowOption();
    ASSERT_NE(option, nullptr);
    option->SetWindowName("GetVSyncPeriod");
    sptr<WindowSessionImpl> window = new (std::nothrow) WindowSessionImpl(option);
    ASSERT_NE(window, nullptr);
    window->GetVSyncPeriod();
    window->vsyncStation_ = nullptr;
    window->GetVSyncPeriod();
}

/**
 * @tc.name: UpdatePiPControlStatus01
 * @tc.desc: UpdatePiPControlStatus
 * @tc.type: FUNC
 */
HWTEST_F(WindowSessionImplTest4, UpdatePiPControlStatus01, Function | SmallTest | Level2)
{
    sptr<WindowOption> option = new (std::nothrow) WindowOption();
    ASSERT_NE(option, nullptr);
    option->SetWindowName("UpdatePiPControlStatus01");
    sptr<WindowSessionImpl> window = new (std::nothrow) WindowSessionImpl(option);
    ASSERT_NE(window, nullptr);
    ASSERT_NE(window->property_, nullptr);
    window->property_->SetPersistentId(1);
    SessionInfo sessionInfo = { "CreateTestBundle", "CreateTestModule", "CreateTestAbility" };
    sptr<SessionMocker> session = new(std::nothrow) SessionMocker(sessionInfo);
    ASSERT_NE(nullptr, session);
    window->hostSession_ = session;
    auto controlType = PiPControlType::VIDEO_PLAY_PAUSE;
    auto status = PiPControlStatus::ENABLED;
    window->UpdatePiPControlStatus(controlType, status);
    window->hostSession_ = nullptr;
    window->UpdatePiPControlStatus(controlType, status);
}

/**
 * @tc.name: NotifyWindowVisibility01
 * @tc.desc: NotifyWindowVisibility
 * @tc.type: FUNC
 */
HWTEST_F(WindowSessionImplTest4, NotifyWindowVisibility01, Function | SmallTest | Level2)
{
    sptr<WindowOption> option = new (std::nothrow) WindowOption();
    ASSERT_NE(option, nullptr);
    option->SetWindowName("NotifyWindowVisibility01");
    sptr<WindowSessionImpl> window = new (std::nothrow) WindowSessionImpl(option);
    ASSERT_NE(window, nullptr);
    ASSERT_NE(window->property_, nullptr);
    window->property_->SetPersistentId(1);
    SessionInfo sessionInfo = { "CreateTestBundle", "CreateTestModule", "CreateTestAbility" };
    sptr<SessionMocker> session = new(std::nothrow) SessionMocker(sessionInfo);
    ASSERT_NE(nullptr, session);
    window->hostSession_ = session;
    window->NotifyWindowVisibility(false);
    sptr<IWindowVisibilityChangedListener> listener = new IWindowVisibilityChangedListener();
    window->RegisterWindowVisibilityChangeListener(listener);
    window->NotifyWindowVisibility(false);
    window->UnregisterWindowVisibilityChangeListener(listener);
}

/**
 * @tc.name: NotifyMainWindowClose01
 * @tc.desc: NotifyMainWindowClose
 * @tc.type: FUNC
 */
HWTEST_F(WindowSessionImplTest4, NotifyMainWindowClose01, Function | SmallTest | Level2)
{
    sptr<WindowOption> option = sptr<WindowOption>::MakeSptr();
    ASSERT_NE(option, nullptr);
    option->SetWindowName("NotifyMainWindowClose01");
    sptr<WindowSessionImpl> window = sptr<WindowSessionImpl>::MakeSptr(option);
    ASSERT_NE(window, nullptr);
    SessionInfo sessionInfo = { "CreateTestBundle", "CreateTestModule", "CreateTestAbility" };
    sptr<SessionMocker> session = sptr<SessionMocker>::MakeSptr(sessionInfo);
    ASSERT_NE(nullptr, session);
    window->hostSession_ = session;
    window->property_->SetPersistentId(1);

    bool terminateCloseProcess = false;
    WMError res = window->NotifyMainWindowClose(terminateCloseProcess);
    EXPECT_EQ(terminateCloseProcess, false);
    EXPECT_EQ(res, WMError::WM_ERROR_NULLPTR);
    sptr<IMainWindowCloseListener> listener = sptr<IMainWindowCloseListener>::MakeSptr();
    window->RegisterMainWindowCloseListeners(listener);
    res = window->NotifyMainWindowClose(terminateCloseProcess);
    EXPECT_EQ(terminateCloseProcess, false);
    EXPECT_EQ(res, WMError::WM_ERROR_NULLPTR);
    window->UnregisterMainWindowCloseListeners(listener);
}

/**
 * @tc.name: UpdateVirtualPixelRatio
 * @tc.desc: test UpdateVirtualPixelRatio
 * @tc.type: FUNC
 */
HWTEST_F(WindowSessionImplTest4, UpdateVirtualPixelRatio, Function | SmallTest | Level2)
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
 * @tc.name: IsPcOrPadCapabilityEnabled
 * @tc.desc: IsPcOrPadCapabilityEnabled test
 * @tc.type: FUNC
 */
HWTEST_F(WindowSessionImplTest4, IsPcOrPadCapabilityEnabled, Function | SmallTest | Level2)
{
    GTEST_LOG_(INFO) << "WindowSessionImplTest4: IsPcOrPadCapabilityEnabled start";
    sptr<WindowOption> option = sptr<WindowOption>::MakeSptr();
    option->SetWindowName("IsPcOrPadCapabilityEnabled");
    sptr<WindowSessionImpl> window = sptr<WindowSessionImpl>::MakeSptr(option);
    ASSERT_NE(window->property_, nullptr);
    window->property_->SetPersistentId(1);
    window->property_->SetWindowType(WindowType::APP_MAIN_WINDOW_BASE);
    window->property_->SetWindowMode(WindowMode::WINDOW_MODE_FLOATING);
    SessionInfo sessionInfo = { "CreateTestBundle", "CreateTestModule", "CreateTestAbility" };
    sptr<SessionMocker> session = sptr<SessionMocker>::MakeSptr(sessionInfo);
    ASSERT_NE(nullptr, session);
    window->hostSession_ = session;

    window->windowSystemConfig_.uiType_ = "pc";
    EXPECT_EQ(true, window->IsPcOrPadCapabilityEnabled());
    window->windowSystemConfig_.uiType_ = "phone";
    EXPECT_EQ(false, window->IsPcOrPadCapabilityEnabled());
    window->windowSystemConfig_.uiType_ = "pad";
    EXPECT_EQ(false, window->IsPcOrPadCapabilityEnabled());
    window->property_->SetIsPcAppInPad(true);
    EXPECT_EQ(true, window->IsPcOrPadCapabilityEnabled());
    EXPECT_EQ(WMError::WM_OK, window->Destroy(true));
    GTEST_LOG_(INFO) << "WindowSessionImplTest4: IsPcOrPadCapabilityEnabled end";
}

/**
 * @tc.name: DestroySubWindow
 * @tc.desc: DestroySubWindow test
 * @tc.type: FUNC
 */
HWTEST_F(WindowSessionImplTest4, DestroySubWindow, Function | SmallTest | Level2)
{
    GTEST_LOG_(INFO) << "WindowSessionImplTest4: DestroySubWindow start";
    sptr<WindowOption> option = sptr<WindowOption>::MakeSptr();
    ASSERT_NE(option, nullptr);
    option->SetWindowName("DestroySubWindow");
    option->SetWindowType(WindowType::APP_MAIN_WINDOW_BASE);
    sptr<WindowSessionImpl> window = sptr<WindowSessionImpl>::MakeSptr(option);
    ASSERT_NE(window->property_, nullptr);
    window->property_->SetPersistentId(1);
    SessionInfo sessionInfo = { "CreateTestBundle", "CreateTestModule", "CreateTestAbility" };
    sptr<SessionMocker> session = sptr<SessionMocker>::MakeSptr(sessionInfo);
    ASSERT_NE(nullptr, session);
    window->hostSession_ = session;
    window->windowSystemConfig_.uiType_ = "pc";

    sptr<WindowOption> subOption = sptr<WindowOption>::MakeSptr();
    ASSERT_NE(subOption, nullptr);
    subOption->SetWindowName("DestroySubWindow01");
    subOption->SetWindowType(WindowType::APP_SUB_WINDOW_BASE);
    sptr<WindowSessionImpl> subWindow = sptr<WindowSessionImpl>::MakeSptr(subOption);
    ASSERT_NE(subWindow, nullptr);
    ASSERT_NE(subWindow->property_, nullptr);
    subWindow->property_->SetPersistentId(2);
    SessionInfo subSessionInfo = { "CreateTestBundle", "CreateTestModule", "CreateTestAbility" };
    sptr<SessionMocker> subSession = sptr<SessionMocker>::MakeSptr(subSessionInfo);
    ASSERT_NE(nullptr, subSession);
    subWindow->hostSession_ = subSession;
    window->windowSystemConfig_.uiType_ = "pc";
    std::vector<sptr<WindowSessionImpl>> vec;
    WindowSessionImpl::subWindowSessionMap_.insert(std::pair<int32_t,
        std::vector<sptr<WindowSessionImpl>>>(1, vec));
    WindowSessionImpl::subWindowSessionMap_[1].push_back(subWindow);
    window->DestroySubWindow();
    EXPECT_EQ(WMError::WM_OK, window->Destroy(true));
}

/**
 * @tc.name: UpdateSubWindowStateAndNotify01
 * @tc.desc: UpdateSubWindowStateAndNotify
 * @tc.type: FUNC
 */
HWTEST_F(WindowSessionImplTest4, UpdateSubWindowStateAndNotify01, Function | SmallTest | Level2)
{
    sptr<WindowOption> option = sptr<WindowOption>::MakeSptr();
    ASSERT_NE(option, nullptr);
    option->SetWindowName("UpdateSubWindowStateAndNotify01");
    option->SetWindowType(WindowType::APP_MAIN_WINDOW_BASE);
    sptr<WindowSessionImpl> window = sptr<WindowSessionImpl>::MakeSptr(option);
    ASSERT_NE(window->property_, nullptr);
    window->property_->SetPersistentId(1);
    SessionInfo sessionInfo = { "CreateTestBundle", "CreateTestModule", "CreateTestAbility" };
    sptr<SessionMocker> session = sptr<SessionMocker>::MakeSptr(sessionInfo);
    ASSERT_NE(nullptr, session);
    window->hostSession_ = session;
    window->windowSystemConfig_.uiType_ = "pc";

    sptr<WindowOption> subOption = sptr<WindowOption>::MakeSptr();
    ASSERT_NE(subOption, nullptr);
    subOption->SetWindowName("UpdateSubWindowStateAndNotify011");
    subOption->SetWindowType(WindowType::APP_SUB_WINDOW_BASE);
    sptr<WindowSessionImpl> subWindow = sptr<WindowSessionImpl>::MakeSptr(subOption);
    ASSERT_NE(subWindow, nullptr);
    ASSERT_NE(subWindow->property_, nullptr);
    subWindow->property_->SetPersistentId(2);
    SessionInfo subSessionInfo = { "CreateTestBundle", "CreateTestModule", "CreateTestAbility" };
    sptr<SessionMocker> subSession = sptr<SessionMocker>::MakeSptr(subSessionInfo);
    ASSERT_NE(nullptr, subSession);
    subWindow->hostSession_ = subSession;
    window->windowSystemConfig_.uiType_ = "pc";
    std::vector<sptr<WindowSessionImpl>> vec;
    WindowSessionImpl::subWindowSessionMap_.insert(std::pair<int32_t,
        std::vector<sptr<WindowSessionImpl>>>(1, vec));
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
 * @tc.name: SetEnableDragBySystem
 * @tc.desc: test SetEnableDragBySystem
 * @tc.type: FUNC
 */
HWTEST_F(WindowSessionImplTest4, SetEnableDragBySystem, Function | SmallTest | Level2)
{
    GTEST_LOG_(INFO) << "WindowSessionImplTest4: GetSubWindow start";
    sptr<WindowOption> option = sptr<WindowOption>::MakeSptr();
    option->SetWindowName("GetSubWindow");
    sptr<WindowSessionImpl> window = sptr<WindowSessionImpl>::MakeSptr(option);
    ASSERT_NE(nullptr, window);
    window->property_->SetDragEnabled(true);
    window->SetEnableDragBySystem(false);
    ASSERT_FALSE(window->property_->GetDragEnabled());
}
}
} // namespace Rosen
} // namespace OHOS
