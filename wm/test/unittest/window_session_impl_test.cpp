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

using namespace testing;
using namespace testing::ext;

namespace OHOS {
namespace Rosen {
class WindowSessionImplTest : public testing::Test {
public:
    static void SetUpTestCase();
    static void TearDownTestCase();
    void SetUp() override;
    void TearDown() override;

    std::shared_ptr<AbilityRuntime::AbilityContext> abilityContext_;
private:
    static constexpr uint32_t WAIT_SYNC_IN_NS = 50000;
};

void WindowSessionImplTest::SetUpTestCase()
{
}

void WindowSessionImplTest::TearDownTestCase()
{
}

void WindowSessionImplTest::SetUp()
{
    abilityContext_ = std::make_shared<AbilityRuntime::AbilityContextImpl>();
}

void WindowSessionImplTest::TearDown()
{
    usleep(WAIT_SYNC_IN_NS);
    abilityContext_ = nullptr;
}

namespace {
/**
 * @tc.name: CreateWindowAndDestroy01
 * @tc.desc: Create window and destroy window
 * @tc.type: FUNC
 */
HWTEST_F(WindowSessionImplTest, CreateWindowAndDestroy01, Function | SmallTest | Level2)
{
    sptr<WindowOption> option = new WindowOption();
    option->SetWindowName("CreateWindow01");
    sptr<WindowSessionImpl> window = new WindowSessionImpl(option);

    SessionInfo sessionInfo = { "CreateTestBundle", "CreateTestModule", "CreateTestAbility" };
    sptr<SessionMocker> session = new(std::nothrow) SessionMocker(sessionInfo);
    ASSERT_NE(nullptr, session);
    ASSERT_EQ(WMError::WM_OK, window->Create(nullptr, session));
    ASSERT_EQ(WMError::WM_OK, window->Create(abilityContext_, session));
    ASSERT_EQ(WMError::WM_OK, window->Create(abilityContext_, session));
    window->property_->SetPersistentId(1);
    ASSERT_EQ(WMError::WM_ERROR_INVALID_WINDOW, window->Destroy());
    // session is null
    window = new WindowSessionImpl(option);
    ASSERT_EQ(WMError::WM_OK, window->Create(abilityContext_, nullptr));
    ASSERT_EQ(WMError::WM_ERROR_INVALID_WINDOW, window->Destroy());

    window = new WindowSessionImpl(option);
    ASSERT_EQ(WMError::WM_OK, window->Create(abilityContext_, session));
    ASSERT_EQ(WMError::WM_ERROR_INVALID_WINDOW, window->Destroy(false));
}

/**
 * @tc.name: CreateWindowAndDestroy02
 * @tc.desc: Create window and destroy window
 * @tc.type: FUNC
 */
HWTEST_F(WindowSessionImplTest, CreateWindowAndDestroy02, Function | SmallTest | Level2)
{
    std::string identityToken = "testToken";
    sptr<WindowOption> option = new WindowOption();
    ASSERT_NE(nullptr, option);
    option->SetWindowName("CreateWindow01");
    sptr<WindowSessionImpl> window = new WindowSessionImpl(option);

    SessionInfo sessionInfo = { "CreateTestBundle", "CreateTestModule", "CreateTestAbility" };
    sptr<SessionMocker> session = new(std::nothrow) SessionMocker(sessionInfo);
    ASSERT_NE(nullptr, session);
    ASSERT_EQ(WMError::WM_OK, window->Create(nullptr, session, identityToken));
    ASSERT_EQ(WMError::WM_OK, window->Create(abilityContext_, session, identityToken));
    ASSERT_EQ(WMError::WM_OK, window->Create(abilityContext_, session, identityToken));
    window->property_->SetPersistentId(1);
    ASSERT_EQ(WMError::WM_ERROR_INVALID_WINDOW, window->Destroy());
    // session is null
    window = new WindowSessionImpl(option);
    ASSERT_EQ(WMError::WM_OK, window->Create(abilityContext_, nullptr));
    ASSERT_EQ(WMError::WM_ERROR_INVALID_WINDOW, window->Destroy());

    window = new WindowSessionImpl(option);
    ASSERT_EQ(WMError::WM_OK, window->Create(abilityContext_, session, identityToken));
    ASSERT_EQ(WMError::WM_ERROR_INVALID_WINDOW, window->Destroy(false));
}

/**
 * @tc.name: Connect01
 * @tc.desc: Connect session
 * @tc.type: FUNC
 */
HWTEST_F(WindowSessionImplTest, Connect01, Function | SmallTest | Level2)
{
    sptr<WindowOption> option = new WindowOption();
    option->SetWindowName("Connect01");
    sptr<WindowSessionImpl> window = new(std::nothrow) WindowSessionImpl(option);
    ASSERT_NE(nullptr, window);
    window->property_->SetPersistentId(1);
    // connect with null session
    ASSERT_EQ(WMError::WM_ERROR_NULLPTR, window->Connect());

    SessionInfo sessionInfo = { "CreateTestBundle", "CreateTestModule", "CreateTestAbility" };
    sptr<SessionMocker> session = new(std::nothrow) SessionMocker(sessionInfo);
    ASSERT_NE(nullptr, session);
    window->hostSession_ = session;
    EXPECT_CALL(*(session), Connect(_, _, _, _, _, _, _)).WillOnce(Return(WSError::WS_ERROR_NULLPTR));
    ASSERT_EQ(WMError::WM_ERROR_NULLPTR, window->Connect());
    EXPECT_CALL(*(session), Connect(_, _, _, _, _, _, _)).WillOnce(Return(WSError::WS_OK));
    ASSERT_EQ(WMError::WM_OK, window->Connect());
    ASSERT_EQ(WMError::WM_OK, window->Destroy());
}

/**
 * @tc.name: Show01
 * @tc.desc: Show session
 * @tc.type: FUNC
 */
HWTEST_F(WindowSessionImplTest, Show01, Function | SmallTest | Level2)
{
    sptr<WindowOption> option = new WindowOption();
    option->SetWindowName("Show01");
    sptr<WindowSessionImpl> window = new(std::nothrow) WindowSessionImpl(option);
    ASSERT_NE(nullptr, window);
    window->property_->SetPersistentId(1);
    // show with null session
    ASSERT_EQ(WMError::WM_ERROR_INVALID_WINDOW, window->Show());

    SessionInfo sessionInfo = { "CreateTestBundle", "CreateTestModule", "CreateTestAbility" };
    sptr<SessionMocker> session = new(std::nothrow) SessionMocker(sessionInfo);
    ASSERT_NE(nullptr, session);
    window->hostSession_ = session;
    EXPECT_CALL(*(session), Foreground(_, _)).WillOnce(Return(WSError::WS_OK));
    ASSERT_EQ(WMError::WM_OK, window->Show());
    ASSERT_EQ(WMError::WM_OK, window->Show());
    window->state_ = WindowState::STATE_CREATED;
    EXPECT_CALL(*(session), Foreground(_, _)).WillOnce(Return(WSError::WS_ERROR_INVALID_SESSION));
    ASSERT_EQ(WMError::WM_ERROR_INVALID_SESSION, window->Show());
    ASSERT_EQ(WMError::WM_OK, window->Destroy());
}

/**
 * @tc.name: Hide01
 * @tc.desc: Hide session
 * @tc.type: FUNC
 */
HWTEST_F(WindowSessionImplTest, Hide01, Function | SmallTest | Level2)
{
    sptr<WindowOption> option = new WindowOption();
    option->SetWindowName("Hide01");
    sptr<WindowSessionImpl> window = new(std::nothrow) WindowSessionImpl(option);
    ASSERT_NE(nullptr, window);
    window->property_->SetPersistentId(1);
    // show with null session
    ASSERT_EQ(WMError::WM_ERROR_INVALID_WINDOW, window->Hide());

    SessionInfo sessionInfo = { "CreateTestBundle", "CreateTestModule", "CreateTestAbility" };
    sptr<SessionMocker> session = new(std::nothrow) SessionMocker(sessionInfo);
    ASSERT_NE(nullptr, session);
    window->hostSession_ = session;
    ASSERT_EQ(WMError::WM_OK, window->Hide());
    ASSERT_EQ(WMError::WM_OK, window->Hide());
    window->state_ = WindowState::STATE_CREATED;
    ASSERT_EQ(WMError::WM_OK, window->Hide());
    window->state_ = WindowState::STATE_SHOWN;
    window->property_->type_ = WindowType::WINDOW_TYPE_FLOAT;
    ASSERT_EQ(WMError::WM_OK, window->Hide());
    window->property_->type_ = WindowType::WINDOW_TYPE_APP_MAIN_WINDOW;
    ASSERT_EQ(WMError::WM_OK, window->Destroy());
}

/**
 * @tc.name: SetResizeByDragEnabled01
 * @tc.desc: SetResizeByDragEnabled and check the retCode
 * @tc.type: FUNC
 */
HWTEST_F(WindowSessionImplTest, SetResizeByDragEnabled01, Function | SmallTest | Level2)
{
    sptr<WindowOption> option = new WindowOption();
    option->SetWindowName("SetResizeByDragEnabled01");
    sptr<WindowSessionImpl> window = new(std::nothrow) WindowSessionImpl(option);
    ASSERT_NE(nullptr, window);
    WMError retCode = window->SetResizeByDragEnabled(true);
    ASSERT_EQ(retCode, WMError::WM_ERROR_INVALID_WINDOW);
    window->property_->SetPersistentId(1);
    SessionInfo sessionInfo = { "CreateTestBundle", "CreateTestModule", "CreateTestAbility" };
    sptr<SessionMocker> session = new(std::nothrow) SessionMocker(sessionInfo);
    ASSERT_NE(nullptr, session);
    window->hostSession_ = session;
    window->state_ = WindowState::STATE_CREATED;
    ASSERT_FALSE(window->IsWindowSessionInvalid());
    retCode = window->SetResizeByDragEnabled(true);
    ASSERT_EQ(retCode, WMError::WM_OK);

    window->property_->type_ = WindowType::APP_SUB_WINDOW_BASE;
    ASSERT_FALSE(WindowHelper::IsMainWindow(window->GetType()));
    retCode = window->SetResizeByDragEnabled(true);
    ASSERT_EQ(retCode, WMError::WM_ERROR_INVALID_TYPE);
}

/**
 * @tc.name: SetWindowType01
 * @tc.desc: SetWindowType
 * @tc.type: FUNC
 */
HWTEST_F(WindowSessionImplTest, SetWindowType01, Function | SmallTest | Level2)
{
    GTEST_LOG_(INFO) << "WindowSessionImplTest: SetWindowType01 start";
    sptr<WindowOption> option = new WindowOption();
    sptr<WindowSessionImpl> window = new WindowSessionImpl(option);
    ASSERT_NE(window->property_, nullptr);

    window->property_->SetPersistentId(1);
    option->SetWindowName("SetWindowType01");
    WindowType type = WindowType::WINDOW_TYPE_BOOT_ANIMATION;
    option->SetWindowType(type);
    window = new WindowSessionImpl(option);
    ASSERT_NE(window, nullptr);

    WindowType type1 = WindowType::WINDOW_TYPE_POINTER;
    option->SetWindowType(type1);
    window = new WindowSessionImpl(option);
    ASSERT_NE(window, nullptr);

    WindowType type2 = WindowType::WINDOW_TYPE_APP_MAIN_WINDOW;
    option->SetWindowType(type2);
    window = new WindowSessionImpl(option);
    ASSERT_NE(window, nullptr);

    WindowType type3 = WindowType::APP_MAIN_WINDOW_END;
    option->SetWindowType(type3);
    window = new WindowSessionImpl(option);
    ASSERT_NE(window, nullptr);

    GTEST_LOG_(INFO) << "WindowSessionImplTest: SetWindowType01 end";
}

/**
 * @tc.name: ColorSpace
 * @tc.desc: SetColorSpace and GetColorSpace
 * @tc.type: FUNC
 */
HWTEST_F(WindowSessionImplTest, ColorSpace, Function | SmallTest | Level2)
{
    GTEST_LOG_(INFO) << "WindowSessionImplTest: ColorSpace start";
    sptr<WindowOption> option = new WindowOption();
    option->SetWindowName("ColorSpace");
    sptr<WindowSessionImpl> window =
        new (std::nothrow) WindowSessionImpl(option);
    ASSERT_NE(nullptr, window);
    window->property_->SetPersistentId(1);

    window->SetColorSpace(ColorSpace::COLOR_SPACE_DEFAULT);
    ColorSpace colorSpace = window->GetColorSpace();
    ASSERT_EQ(colorSpace, ColorSpace::COLOR_SPACE_DEFAULT);

    window->SetColorSpace(ColorSpace::COLOR_SPACE_WIDE_GAMUT);
    ColorSpace colorSpace1 = window->GetColorSpace();
    ASSERT_EQ(colorSpace1, ColorSpace::COLOR_SPACE_WIDE_GAMUT);
    GTEST_LOG_(INFO) << "WindowSessionImplTest: ColorSpace end";
}

/**
 * @tc.name: MakeSubOrDialogWindowDragableAndMoveble01
 * @tc.desc: MakeSubOrDialogWindowDragableAndMoveble
 * @tc.type: FUNC
 */
HWTEST_F(WindowSessionImplTest, MakeSubOrDialogWindowDragableAndMoveble01, Function | SmallTest | Level2)
{
    GTEST_LOG_(INFO) << "WindowSessionImplTest: MakeSubOrDialogWindowDragableAndMoveble01 start";
    sptr<WindowOption> option = new WindowOption();
    ASSERT_NE(nullptr, option);
    option->SetSubWindowDecorEnable(true);
    option->SetWindowName("MakeSubOrDialogWindowDragableAndMoveble01");
    sptr<WindowSessionImpl> window =
        new (std::nothrow) WindowSessionImpl(option);
    ASSERT_NE(nullptr, window);
    window->property_->SetWindowType(WindowType::WINDOW_TYPE_APP_SUB_WINDOW);
    window->windowSystemConfig_.uiType_ = "pc";
    window->MakeSubOrDialogWindowDragableAndMoveble();
    ASSERT_EQ(true, window->property_->IsDecorEnable());
    GTEST_LOG_(INFO) << "WindowSessionImplTest: MakeSubOrDialogWindowDragableAndMoveble01 end";
}

/**
 * @tc.name: MakeSubOrDialogWindowDragableAndMoveble02
 * @tc.desc: MakeSubOrDialogWindowDragableAndMoveble
 * @tc.type: FUNC
 */
HWTEST_F(WindowSessionImplTest, MakeSubOrDialogWindowDragableAndMoveble02, Function | SmallTest | Level2)
{
    GTEST_LOG_(INFO) << "WindowSessionImplTest: MakeSubOrDialogWindowDragableAndMoveble02 start";
    sptr<WindowOption> option = new WindowOption();
    ASSERT_NE(nullptr, option);
    option->SetDialogDecorEnable(true);
    option->SetWindowName("MakeSubOrDialogWindowDragableAndMoveble02");
    sptr<WindowSessionImpl> window =
        new (std::nothrow) WindowSessionImpl(option);
    ASSERT_NE(nullptr, window);
    window->property_->SetWindowType(WindowType::WINDOW_TYPE_DIALOG);
    window->windowSystemConfig_.uiType_ = "pc";
    window->MakeSubOrDialogWindowDragableAndMoveble();
    ASSERT_EQ(true, window->property_->IsDecorEnable());
    GTEST_LOG_(INFO) << "WindowSessionImplTest: MakeSubOrDialogWindowDragableAndMoveble02 end";
}

/**
 * @tc.name: MakeSubOrDialogWindowDragableAndMoveble03
 * @tc.desc: MakeSubOrDialogWindowDragableAndMoveble
 * @tc.type: FUNC
 */
HWTEST_F(WindowSessionImplTest, MakeSubOrDialogWindowDragableAndMoveble03, Function | SmallTest | Level2)
{
    GTEST_LOG_(INFO) << "WindowSessionImplTest: MakeSubOrDialogWindowDragableAndMoveble03 start";
    sptr<WindowOption> option = new WindowOption();
    ASSERT_NE(nullptr, option);
    option->SetDialogDecorEnable(true);
    option->SetWindowName("MakeSubOrDialogWindowDragableAndMoveble03");
    sptr<WindowSessionImpl> window =
        new (std::nothrow) WindowSessionImpl(option);
    ASSERT_NE(nullptr, window);
    window->property_->SetWindowType(WindowType::WINDOW_TYPE_DIALOG);
    window->windowSystemConfig_.uiType_ = "phone";
    window->MakeSubOrDialogWindowDragableAndMoveble();
    ASSERT_EQ(false, window->property_->IsDecorEnable());
    GTEST_LOG_(INFO) << "WindowSessionImplTest: MakeSubOrDialogWindowDragableAndMoveble03 end";
}

/**
 * @tc.name: MakeSubOrDialogWindowDragableAndMoveble
 * @tc.desc: MakeSubOrDialogWindowDragableAndMoveble04
 * @tc.type: FUNC
 */
HWTEST_F(WindowSessionImplTest, MakeSubOrDialogWindowDragableAndMoveble04, Function | SmallTest | Level2)
{
    GTEST_LOG_(INFO) << "WindowSessionImplTest: MakeSubOrDialogWindowDragableAndMoveble04 start";
    sptr<WindowOption> option = new WindowOption();
    ASSERT_NE(nullptr, option);
    option->SetSubWindowDecorEnable(true);
    option->SetWindowName("MakeSubOrDialogWindowDragableAndMoveble04");
    sptr<WindowSessionImpl> window = new (std::nothrow) WindowSessionImpl(option);
    ASSERT_NE(nullptr, window);
    window->property_->SetWindowType(WindowType::WINDOW_TYPE_APP_SUB_WINDOW);
    window->windowSystemConfig_.freeMultiWindowSupport_ = true;
    window->windowSystemConfig_.freeMultiWindowEnable_ = true;
    window->MakeSubOrDialogWindowDragableAndMoveble();
    ASSERT_TRUE(window->property_->IsDecorEnable());
    GTEST_LOG_(INFO) << "WindowSessionImplTest: MakeSubOrDialogWindowDragableAndMoveble04 end";
}

/**
 * @tc.name: WindowSessionCreateCheck01
 * @tc.desc: WindowSessionCreateCheck01
 * @tc.type: FUNC
 */
HWTEST_F(WindowSessionImplTest, WindowSessionCreateCheck01, Function | SmallTest | Level2)
{
    GTEST_LOG_(INFO) << "WindowSessionImplTest: WindowSessionCreateCheck01 start";
    sptr<WindowOption> option = new WindowOption();
    option->SetWindowName("WindowSessionCreateCheck");
    sptr<WindowSessionImpl> window =
        new (std::nothrow) WindowSessionImpl(option);
    ASSERT_NE(nullptr, window);

    sptr<WindowOption> option1 = new WindowOption();
    option1->SetWindowName("WindowSessionCreateCheck"); // set the same name
    sptr<WindowSessionImpl> window1 =
        new (std::nothrow) WindowSessionImpl(option1);

    ASSERT_NE(nullptr, window1);

    WMError res = window1->WindowSessionCreateCheck();
    ASSERT_EQ(res, WMError::WM_OK);

    window1->property_ = nullptr;
    WMError res1 = window1->WindowSessionCreateCheck();
    ASSERT_EQ(res1, WMError::WM_ERROR_NULLPTR);

    GTEST_LOG_(INFO) << "WindowSessionImplTest: WindowSessionCreateCheck01 end";
}

/**
 * @tc.name: WindowSessionCreateCheck02
 * @tc.desc: WindowSessionCreateCheck02
 * @tc.type: FUNC
 */
HWTEST_F(WindowSessionImplTest, WindowSessionCreateCheck02, Function | SmallTest | Level2)
{
    GTEST_LOG_(INFO) << "WindowSessionImplTest: WindowSessionCreateCheck02 start";
    sptr<WindowOption> option = new WindowOption();
    option->SetWindowName("WindowSessionCreateCheck");
    sptr<WindowSessionImpl> window =
        new (std::nothrow) WindowSessionImpl(option);
    ASSERT_NE(window, nullptr);

    window->property_->SetWindowType(WindowType::WINDOW_TYPE_FLOAT_CAMERA);
    WMError res1 = window->WindowSessionCreateCheck();
    ASSERT_EQ(res1, WMError::WM_OK);

    GTEST_LOG_(INFO) << "WindowSessionImplTest: WindowSessionCreateCheck02 end";
}

/**
 * @tc.name: WindowSessionCreateCheck
 * @tc.desc: WindowSessionCreateCheck03
 * @tc.type: FUNC
 */
HWTEST_F(WindowSessionImplTest, WindowSessionCreateCheck03, Function | SmallTest | Level2)
{
    GTEST_LOG_(INFO) << "WindowSessionImplTest: WindowSessionCreateCheck03 start";
    sptr<WindowOption> option = new WindowOption();
    std::string name = "WindowSessionCreateCheck03";
    option->SetWindowName(name);
    sptr<WindowSessionImpl> window =
        new (std::nothrow) WindowSessionImpl(option);
    ASSERT_NE(window, nullptr);
    window->windowSessionMap_[name] = std::pair<int32_t, sptr<WindowSessionImpl>>(1, window);
    WMError res = window->WindowSessionCreateCheck();
    ASSERT_EQ(res, WMError::WM_ERROR_REPEAT_OPERATION);
    GTEST_LOG_(INFO) << "WindowSessionImplTest: WindowSessionCreateCheck03 end";
}

/**
 * @tc.name: SetActive
 * @tc.desc: SetActive
 * @tc.type: FUNC
 */
HWTEST_F(WindowSessionImplTest, SetActive, Function | SmallTest | Level2)
{
    GTEST_LOG_(INFO) << "WindowSessionImplTest: SetActive start";
    sptr<WindowOption> option = new WindowOption();
    option->SetWindowName("WindowSessionCreateCheck");
    sptr<WindowSessionImpl> window =
        new (std::nothrow) WindowSessionImpl(option);
    ASSERT_NE(window, nullptr);

    WSError res1 = window->SetActive(true);
    ASSERT_EQ(res1, WSError::WS_OK);
    res1 = window->SetActive(false);
    ASSERT_EQ(res1, WSError::WS_OK);

    GTEST_LOG_(INFO) << "WindowSessionImplTest: SetActive end";
}

/**
 * @tc.name: UpdateRect01
 * @tc.desc: UpdateRect
 * @tc.type: FUNC
 */
HWTEST_F(WindowSessionImplTest, UpdateRect01, Function | SmallTest | Level2)
{
    GTEST_LOG_(INFO) << "WindowSessionImplTest: UpdateRect01 start";
    sptr<WindowOption> option = new WindowOption();
    option->SetWindowName("WindowSessionCreateCheck");
    sptr<WindowSessionImpl> window =
        new (std::nothrow) WindowSessionImpl(option);
    ASSERT_NE(window, nullptr);

    WSRect rect;
    rect.posX_ = 0;
    rect.posY_ = 0;
    rect.height_ = 50;
    rect.width_ = 50;

    Rect rectW; // GetRect().IsUninitializedRect is false
    rectW.posX_ = 0;
    rectW.posY_ = 0;
    rectW.height_ = 200; // rectW - rect > 50
    rectW.width_ = 200;  // rectW - rect > 50

    window->property_->SetWindowRect(rectW);
    SizeChangeReason reason = SizeChangeReason::UNDEFINED;
    WSError res = window->UpdateRect(rect, reason);
    ASSERT_EQ(res, WSError::WS_OK);

    rectW.height_ = 50;
    window->property_->SetWindowRect(rectW);
    res = window->UpdateRect(rect, reason);
    ASSERT_EQ(res, WSError::WS_OK);

    rectW.height_ = 200;
    rectW.width_ = 50;
    window->property_->SetWindowRect(rectW);
    res = window->UpdateRect(rect, reason);
    ASSERT_EQ(res, WSError::WS_OK);
    GTEST_LOG_(INFO) << "WindowSessionImplTest: UpdateRect01 end";
}

/**
 * @tc.name: UpdateRect02
 * @tc.desc: UpdateRect
 * @tc.type: FUNC
 */
HWTEST_F(WindowSessionImplTest, UpdateRect02, Function | SmallTest | Level2)
{
    GTEST_LOG_(INFO) << "WindowSessionImplTest: UpdateRect02 start";
    sptr<WindowOption> option = new WindowOption();
    option->SetWindowName("WindowSessionCreateCheck");
    sptr<WindowSessionImpl> window =
        new (std::nothrow) WindowSessionImpl(option);
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
    GTEST_LOG_(INFO) << "WindowSessionImplTest: UpdateRect02 end";
}

/**
 * @tc.name: UpdateFocus
 * @tc.desc: UpdateFocus
 * @tc.type: FUNC
 */
HWTEST_F(WindowSessionImplTest, UpdateFocus, Function | SmallTest | Level2)
{
    GTEST_LOG_(INFO) << "WindowSessionImplTest: UpdateFocus start";
    sptr<WindowOption> option = new WindowOption();
    option->SetWindowName("WindowSessionCreateCheck");
    sptr<WindowSessionImpl> window =
        new (std::nothrow) WindowSessionImpl(option);
    ASSERT_NE(window, nullptr);

    WSError res = window->UpdateFocus(true);
    ASSERT_EQ(res, WSError::WS_OK);
    res = window->UpdateFocus(false);
    ASSERT_EQ(res, WSError::WS_OK);

    GTEST_LOG_(INFO) << "WindowSessionImplTest: UpdateFocus end";
}

/**
 * @tc.name: RequestFocusByClient
 * @tc.desc: RequestFocusByClient Test
 * @tc.type: FUNC
 */
HWTEST_F(WindowSessionImplTest, RequestFocusByClient, Function | SmallTest | Level2)
{
    GTEST_LOG_(INFO) << "WindowSessionImplTest: RequestFocusByClient start";
    sptr<WindowOption> option = new WindowOption();
    ASSERT_NE(nullptr, option);
    option->SetWindowName("WindowRequestFocusByClientCheck");
    sptr<WindowSessionImpl> window =
        new (std::nothrow) WindowSessionImpl(option);
    ASSERT_NE(window, nullptr);

    WMError res = window->RequestFocusByClient(true);
    ASSERT_EQ(res, WMError::WM_ERROR_INVALID_WINDOW);
    res = window->RequestFocusByClient(false);
    ASSERT_EQ(res, WMError::WM_ERROR_INVALID_WINDOW);

    window->property_->SetPersistentId(1);
    SessionInfo sessionInfo = { "RequestFocusByClient", "RequestFocusByClient", "RequestFocusByClient" };
    sptr<SessionMocker> session = sptr<SessionMocker>::MakeSptr(sessionInfo);
    ASSERT_NE(session, nullptr);
    window->hostSession_ = session;
    window->state_ = WindowState::STATE_INITIAL;
    res = window->RequestFocusByClient(true);
    ASSERT_EQ(res, WMError::WM_OK);
    res = window->RequestFocusByClient(false);
    ASSERT_EQ(res, WMError::WM_OK);

    GTEST_LOG_(INFO) << "WindowSessionImplTest: RequestFocusByClient end";
}

/**
 * @tc.name: UpdateViewportConfig
 * @tc.desc: UpdateViewportConfig
 * @tc.type: FUNC
 */
HWTEST_F(WindowSessionImplTest, UpdateViewportConfig, Function | SmallTest | Level2)
{
    GTEST_LOG_(INFO) << "WindowSessionImplTest: UpdateViewportConfig start";
    sptr<WindowOption> option = new WindowOption();
    option->SetWindowName("WindowSessionCreateCheck");
    sptr<WindowSessionImpl> window =
        new (std::nothrow) WindowSessionImpl(option);
    ASSERT_NE(window, nullptr);

    Rect rectW; // GetRect().IsUninitializedRect is true
    rectW.posX_ = 0;
    rectW.posY_ = 0;
    rectW.height_ = 0; // rectW - rect > 50
    rectW.width_ = 0;  // rectW - rect > 50

    WindowSizeChangeReason reason = WindowSizeChangeReason::UNDEFINED;
    int32_t res = 0;
    window->UpdateViewportConfig(rectW, reason);
    ASSERT_EQ(res, 0);

    DisplayId displayId = 1;
    window->property_->SetDisplayId(displayId);
    window->UpdateViewportConfig(rectW, reason);
    ASSERT_EQ(res, 0);

    GTEST_LOG_(INFO) << "WindowSessionImplTest: UpdateViewportConfig end";
}

/**
 * @tc.name: CreateWindowAndDestroy01
 * @tc.desc: GetPersistentId
 * @tc.type: FUNC
 */
HWTEST_F(WindowSessionImplTest, GetPersistentId01, Function | SmallTest | Level2)
{
    GTEST_LOG_(INFO) << "WindowSessionImplTest: GetPersistentId start";
    sptr<WindowOption> option = new WindowOption();
    sptr<WindowSessionImpl> window = new WindowSessionImpl(option);
    ASSERT_NE(window->property_, nullptr);

    window->property_->SetPersistentId(1);
    const int32_t res2 = window->GetPersistentId();
    ASSERT_EQ(res2, 1);

    window->property_ = nullptr;
    const int32_t res1 = window->GetPersistentId();
    ASSERT_EQ(res1, INVALID_SESSION_ID);
    GTEST_LOG_(INFO) << "WindowSessionImplTest: GetPersistentId end";
}

/**
 * @tc.name: GetFloatingWindowParentId
 * @tc.desc: GetFloatingWindowParentId and UpdateTitleButtonVisibility
 * @tc.type: FUNC
 */
HWTEST_F(WindowSessionImplTest, GetFloatingWindowParentId, Function | SmallTest | Level2) {
    GTEST_LOG_(INFO) << "WindowSessionImplTest: GetFloatingWindowParentId start";
    sptr<WindowOption> option = new WindowOption();
    option->SetWindowName("Connect");
    sptr<WindowSessionImpl> window =
        new (std::nothrow) WindowSessionImpl(option);
    ASSERT_NE(nullptr, window);
    window->property_->SetPersistentId(1);
    // connect with null session
    ASSERT_EQ(WMError::WM_ERROR_NULLPTR, window->Connect());

    SessionInfo sessionInfo = {"CreateTestBundle", "CreateTestModule",
                               "CreateTestAbility"};
    sptr<SessionMocker> session = new (std::nothrow) SessionMocker(sessionInfo);
    ASSERT_NE(nullptr, session);
    window->hostSession_ = session;
    EXPECT_CALL(*(session), Connect(_, _, _, _, _, _, _))
        .WillOnce(Return(WSError::WS_ERROR_NULLPTR));
    ASSERT_EQ(WMError::WM_ERROR_NULLPTR, window->Connect());
    EXPECT_CALL(*(session), Connect(_, _, _, _, _, _, _))
        .WillOnce(Return(WSError::WS_OK));
    ASSERT_EQ(WMError::WM_OK, window->Connect());

    window->UpdateTitleButtonVisibility();
    int32_t res = window->GetFloatingWindowParentId();
    ASSERT_EQ(res, INVALID_SESSION_ID);
    GTEST_LOG_(INFO) << "WindowSessionImplTest: GetFloatingWindowParentId start";
}

/**
 * @tc.name: UpdateDecorEnable
 * @tc.desc: UpdateDecorEnable
 * @tc.type: FUNC
 */
HWTEST_F(WindowSessionImplTest, UpdateDecorEnable, Function | SmallTest | Level2)
{
    GTEST_LOG_(INFO) << "WindowSessionImplTest: UpdateDecorEnable start";
    sptr<WindowOption> option = new WindowOption();
    option->SetWindowName("UpdateDecorEnable");
    sptr<WindowSessionImpl> window = new (std::nothrow) WindowSessionImpl(option);
    ASSERT_NE(window, nullptr);

    int res = 1;
    window->UpdateDecorEnable(true);
    window->UpdateDecorEnable(false);
    ASSERT_EQ(res, 1);
    GTEST_LOG_(INFO) << "WindowSessionImplTest: UpdateDecorEnable end";
}

/**
 * @tc.name: NotifyModeChange
 * @tc.desc: NotifyModeChange
 * @tc.type: FUNC
 */
HWTEST_F(WindowSessionImplTest, NotifyModeChange, Function | SmallTest | Level2)
{
    GTEST_LOG_(INFO) << "WindowSessionImplTest: NotifyModeChange start";
    sptr<WindowOption> option = new WindowOption();
    option->SetWindowName("NotifyModeChange");
    sptr<WindowSessionImpl> window = new (std::nothrow) WindowSessionImpl(option);
    ASSERT_NE(window, nullptr);

    WindowMode mode = WindowMode::WINDOW_MODE_UNDEFINED;
    int res = 1;
    window->NotifyModeChange(mode, true);
    window->NotifyModeChange(mode, false);
    ASSERT_EQ(res, 1);
    GTEST_LOG_(INFO) << "WindowSessionImplTest: NotifyModeChange end";
}

/**
 * @tc.name: RequestVsyncSucc
 * @tc.desc: RequestVsync Test Succ
 * @tc.type: FUNC
 */
HWTEST_F(WindowSessionImplTest, RequestVsyncSucc, Function | SmallTest | Level2)
{
    sptr<WindowOption> option = new WindowOption();
    option->SetWindowName("RequestVsyncSucc");
    sptr<WindowSessionImpl> window = new (std::nothrow) WindowSessionImpl(option);
    ASSERT_NE(window, nullptr);
    std::shared_ptr<VsyncCallback> vsyncCallback = std::make_shared<VsyncCallback>();
    window->state_ = WindowState::STATE_SHOWN;
    ASSERT_EQ(WindowState::STATE_SHOWN, window->GetWindowState());
    ASSERT_NE(window->vsyncStation_, nullptr);
    window->RequestVsync(vsyncCallback);
    window->vsyncStation_ = nullptr;
    window->RequestVsync(vsyncCallback);
}


/**
 * @tc.name: RequestVsyncErr
 * @tc.desc: RequestVsync Test Err
 * @tc.type: FUNC
 */
HWTEST_F(WindowSessionImplTest, RequestVsyncErr, Function | SmallTest | Level2)
{
    sptr<WindowOption> option = new WindowOption();
    option->SetWindowName("RequestVsyncErr");
    sptr<WindowSessionImpl> window = new (std::nothrow) WindowSessionImpl(option);
    ASSERT_NE(window, nullptr);
    std::shared_ptr<VsyncCallback> vsyncCallback = std::make_shared<VsyncCallback>();
    window->state_ = WindowState::STATE_DESTROYED;
    ASSERT_EQ(WindowState::STATE_DESTROYED, window->GetWindowState());
    window->vsyncStation_ = nullptr;
    window->RequestVsync(vsyncCallback);
}

/**
 * @tc.name: ClearVsync
 * @tc.desc: Clear vsync test
 * @tc.type: FUNC
 */
HWTEST_F(WindowSessionImplTest, ClearVsync, Function | SmallTest | Level2)
{
    sptr<WindowOption> option = new WindowOption();
    ASSERT_NE(option, nullptr);
    option->SetWindowName("ClearVsync");
    sptr<WindowSessionImpl> window = new (std::nothrow) WindowSessionImpl(option);
    ASSERT_NE(window, nullptr);
    window->ClearVsyncStation();
    ASSERT_EQ(window->vsyncStation_, nullptr);
}

/**
 * @tc.name: SetFocusable
 * @tc.desc: SetFocusable
 * @tc.type: FUNC
 */
HWTEST_F(WindowSessionImplTest, SetFocusable, Function | SmallTest | Level2)
{
    GTEST_LOG_(INFO) << "WindowSessionImplTest: SetFocusable start";
    sptr<WindowOption> option = new WindowOption();
    option->SetWindowName("SetFocusable");
    sptr<WindowSessionImpl> window = new WindowSessionImpl(option);

    SessionInfo sessionInfo = {"CreateTestBundle", "CreateTestModule",
                               "CreateTestAbility"};
    sptr<SessionMocker> session = new (std::nothrow) SessionMocker(sessionInfo);
    ASSERT_NE(nullptr, session);
    ASSERT_EQ(WMError::WM_OK, window->Create(nullptr, session));
    window->hostSession_ = session;
    window->property_->SetPersistentId(1);
    ASSERT_FALSE(window->GetPersistentId() == INVALID_SESSION_ID);
    ASSERT_FALSE(window->IsWindowSessionInvalid());
    WMError res = window->SetFocusable(true);
    ASSERT_EQ(res, WMError::WM_OK);
    ASSERT_EQ(WMError::WM_OK, window->Destroy());

    // session is null
    window = new WindowSessionImpl(option);
    ASSERT_EQ(WMError::WM_OK, window->Create(abilityContext_, nullptr));
    res = window->SetFocusable(true);
    ASSERT_EQ(res, WMError::WM_ERROR_INVALID_WINDOW);
    res = window->SetFocusable(false);
    ASSERT_EQ(res, WMError::WM_ERROR_INVALID_WINDOW);
    ASSERT_EQ(WMError::WM_ERROR_INVALID_WINDOW, window->Destroy());
    GTEST_LOG_(INFO) << "WindowSessionImplTest: SetFocusable end";
}

/**
 * @tc.name: SetTouchable
 * @tc.desc: SetTouchable
 * @tc.type: FUNC
 */
HWTEST_F(WindowSessionImplTest, SetTouchable, Function | SmallTest | Level2)
{
    GTEST_LOG_(INFO) << "WindowSessionImplTest: SetTouchable start";
    sptr<WindowOption> option = new WindowOption();
    option->SetWindowName("SetTouchable");
    sptr<WindowSessionImpl> window = new WindowSessionImpl(option);

    SessionInfo sessionInfo = {"CreateTestBundle", "CreateTestModule",
                               "CreateTestAbility"};
    sptr<SessionMocker> session = new (std::nothrow) SessionMocker(sessionInfo);
    ASSERT_NE(nullptr, session);
    ASSERT_EQ(WMError::WM_OK, window->Create(nullptr, session));
    ASSERT_NE(window->property_, nullptr);
    window->hostSession_ = session;
    window->property_->SetPersistentId(1);
    ASSERT_FALSE(window->IsWindowSessionInvalid());
    WMError res = window->SetTouchable(true);
    ASSERT_EQ(res, WMError::WM_OK);
    ASSERT_NE(window->property_, nullptr);
    ASSERT_TRUE(window->property_->touchable_);
    ASSERT_EQ(WMError::WM_OK, window->Destroy());

    // session is null
    window = new WindowSessionImpl(option);
    ASSERT_EQ(WMError::WM_OK, window->Create(abilityContext_, nullptr));
    res = window->SetTouchable(true);
    ASSERT_EQ(res, WMError::WM_ERROR_INVALID_WINDOW);
    res = window->SetTouchable(false);
    ASSERT_EQ(res, WMError::WM_ERROR_INVALID_WINDOW);
    ASSERT_EQ(WMError::WM_ERROR_INVALID_WINDOW, window->Destroy());
    GTEST_LOG_(INFO) << "WindowSessionImplTest: SetTouchable end";
}

/**
 * @tc.name: SetBrightness01
 * @tc.desc: SetBrightness
 * @tc.type: FUNC
 */
HWTEST_F(WindowSessionImplTest, SetBrightness01, Function | SmallTest | Level2)
{
    GTEST_LOG_(INFO) << "WindowSessionImplTest: SetBrightness01 start";
    sptr<WindowOption> option = new WindowOption();
    option->SetWindowName("SetBrightness01");
    sptr<WindowSessionImpl> window = new WindowSessionImpl(option);

    SessionInfo sessionInfo = {"CreateTestBundle", "CreateTestModule",
                               "CreateTestAbility"};
    sptr<SessionMocker> session = new (std::nothrow) SessionMocker(sessionInfo);
    ASSERT_NE(nullptr, session);
    ASSERT_EQ(WMError::WM_OK, window->Create(nullptr, session));
    window->property_->SetPersistentId(1);

    float brightness = -1.0; // brightness < 0
    WMError res = window->SetBrightness(brightness);
    ASSERT_EQ(res, WMError::WM_ERROR_INVALID_WINDOW);

    brightness = 2.0; // brightness > 1
    res = window->SetBrightness(brightness);
    ASSERT_EQ(res, WMError::WM_ERROR_INVALID_PARAM);

    brightness = 0.5;
    res = window->SetBrightness(brightness);
    ASSERT_EQ(res, WMError::WM_ERROR_INVALID_WINDOW);
    ASSERT_EQ(WMError::WM_ERROR_INVALID_WINDOW, window->Destroy());
    GTEST_LOG_(INFO) << "WindowSessionImplTest: SetBrightness01 end";
}

/**
 * @tc.name: SetBrightness02
 * @tc.desc: SetBrightness
 * @tc.type: FUNC
 */
HWTEST_F(WindowSessionImplTest, SetBrightness02, Function | SmallTest | Level2)
{
    GTEST_LOG_(INFO) << "WindowSessionImplTest: SetBrightness02 start";
    sptr<WindowOption> option = new WindowOption();
    option->SetWindowName("SetBrightness02");
    sptr<WindowSessionImpl> window = new WindowSessionImpl(option);

    SessionInfo sessionInfo = {"CreateTestBundle", "CreateTestModule",
                               "CreateTestAbility"};
    sptr<SessionMocker> session = new (std::nothrow) SessionMocker(sessionInfo);
    ASSERT_NE(nullptr, session);
    ASSERT_EQ(WMError::WM_OK, window->Create(nullptr, session));
    window->property_->SetPersistentId(1);
    window->property_->SetWindowType(WindowType::APP_MAIN_WINDOW_END);
    float brightness = 0.5;
    WMError res = window->SetBrightness(brightness);
    ASSERT_EQ(res, WMError::WM_ERROR_INVALID_TYPE);

    window->state_ = WindowState::STATE_SHOWN;
    res = window->SetBrightness(brightness);
    ASSERT_NE(res, WMError::WM_ERROR_NULLPTR);
    ASSERT_EQ(WMError::WM_ERROR_INVALID_WINDOW, window->Destroy());
    GTEST_LOG_(INFO) << "WindowSessionImplTest: SetBrightness02 end";
}

/**
 * @tc.name: SetRequestedOrientation
 * @tc.desc: SetRequestedOrientation
 * @tc.type: FUNC
 */
HWTEST_F(WindowSessionImplTest, SetRequestedOrientation, Function | SmallTest | Level2)
{
    GTEST_LOG_(INFO) << "WindowSessionImplTest: SetRequestedOrientation start";
    sptr<WindowOption> option = new WindowOption();
    option->SetWindowName("SetRequestedOrientation");
    sptr<WindowSessionImpl> window = new WindowSessionImpl(option);

    SessionInfo sessionInfo = {"CreateTestBundle", "CreateTestModule",
                               "CreateTestAbility"};
    sptr<SessionMocker> session = new (std::nothrow) SessionMocker(sessionInfo);
    ASSERT_NE(nullptr, session);
    ASSERT_EQ(WMError::WM_OK, window->Create(nullptr, session));

    window->hostSession_ = session;
    window->property_->SetPersistentId(1);

    Orientation ori = Orientation::VERTICAL;
    window->SetRequestedOrientation(ori);
    Orientation ret = window->GetRequestedOrientation();
    ASSERT_EQ(ret, ori);

    window->SetRequestedOrientation(Orientation::AUTO_ROTATION_UNSPECIFIED);
    Orientation ret1 = window->GetRequestedOrientation();
    ASSERT_EQ(ret1, Orientation::AUTO_ROTATION_UNSPECIFIED);

    window->SetRequestedOrientation(Orientation::USER_ROTATION_PORTRAIT);
    Orientation ret2 = window->GetRequestedOrientation();
    ASSERT_EQ(ret2, Orientation::USER_ROTATION_PORTRAIT);

    window->SetRequestedOrientation(Orientation::USER_ROTATION_LANDSCAPE);
    Orientation ret3 = window->GetRequestedOrientation();
    ASSERT_EQ(ret3, Orientation::USER_ROTATION_LANDSCAPE);

    window->SetRequestedOrientation(Orientation::USER_ROTATION_PORTRAIT_INVERTED);
    Orientation ret4 = window->GetRequestedOrientation();
    ASSERT_EQ(ret4, Orientation::USER_ROTATION_PORTRAIT_INVERTED);

    window->SetRequestedOrientation(Orientation::USER_ROTATION_LANDSCAPE_INVERTED);
    Orientation ret5 = window->GetRequestedOrientation();
    ASSERT_EQ(ret5, Orientation::USER_ROTATION_LANDSCAPE_INVERTED);

    window->SetRequestedOrientation(Orientation::FOLLOW_DESKTOP);
    Orientation ret6 = window->GetRequestedOrientation();
    ASSERT_EQ(ret6, Orientation::FOLLOW_DESKTOP);
    ASSERT_EQ(WMError::WM_OK, window->Destroy());
    GTEST_LOG_(INFO) << "WindowSessionImplTest: SetRequestedOrientation end";
}

/**
 * @tc.name: GetRequestedOrientationtest01
 * @tc.desc: GetRequestedOrientation
 * @tc.type: FUNC
 */
HWTEST_F(WindowSessionImplTest, GetRequestedOrientation, Function | SmallTest | Level2)
{
    GTEST_LOG_(INFO) << "WindowSessionImplTest: GetRequestedOrientationtest01 start";
    sptr<WindowOption> option = new WindowOption();
    ASSERT_NE(option, nullptr);
    option->SetWindowName("GetRequestedOrientation");

    sptr<WindowSessionImpl> window = new (std::nothrow) WindowSessionImpl(option);
    ASSERT_NE(window, nullptr);

    SessionInfo sessionInfo = {"CreateTestBundle", "CreateTestModule",
                               "CreateTestAbility"};
    sptr<SessionMocker> session = new (std::nothrow) SessionMocker(sessionInfo);
    ASSERT_NE(nullptr, session);
    ASSERT_EQ(WMError::WM_OK, window->Create(nullptr, session));

    window->hostSession_ = session;
    window->property_->SetPersistentId(1);

    Orientation ori = Orientation::HORIZONTAL;
    window->SetRequestedOrientation(ori);
    Orientation ret = window->GetRequestedOrientation();
    ASSERT_EQ(ret, ori);

    window->SetRequestedOrientation(Orientation::AUTO_ROTATION_UNSPECIFIED);
    Orientation ret1 = window->GetRequestedOrientation();
    ASSERT_EQ(ret1, Orientation::AUTO_ROTATION_UNSPECIFIED);

    window->SetRequestedOrientation(Orientation::USER_ROTATION_PORTRAIT);
    Orientation ret2 = window->GetRequestedOrientation();
    ASSERT_EQ(ret2, Orientation::USER_ROTATION_PORTRAIT);

    window->SetRequestedOrientation(Orientation::USER_ROTATION_LANDSCAPE);
    Orientation ret3 = window->GetRequestedOrientation();
    ASSERT_EQ(ret3, Orientation::USER_ROTATION_LANDSCAPE);

    window->SetRequestedOrientation(Orientation::USER_ROTATION_PORTRAIT_INVERTED);
    Orientation ret4 = window->GetRequestedOrientation();
    ASSERT_EQ(ret4, Orientation::USER_ROTATION_PORTRAIT_INVERTED);

    window->SetRequestedOrientation(Orientation::USER_ROTATION_LANDSCAPE_INVERTED);
    Orientation ret5 = window->GetRequestedOrientation();
    ASSERT_EQ(ret5, Orientation::USER_ROTATION_LANDSCAPE_INVERTED);

    window->SetRequestedOrientation(Orientation::FOLLOW_DESKTOP);
    Orientation ret6 = window->GetRequestedOrientation();
    ASSERT_EQ(ret6, Orientation::FOLLOW_DESKTOP);

    GTEST_LOG_(INFO) << "WindowSessionImplTest: GetRequestedOrientationtest01 end";
}

/**
 * @tc.name: GetContentInfo
 * @tc.desc: GetContentInfo
 * @tc.type: FUNC
 */
HWTEST_F(WindowSessionImplTest, GetContentInfo, Function | SmallTest | Level2)
{
    GTEST_LOG_(INFO) << "WindowSessionImplTest: GetContentInfo start";
    sptr<WindowOption> option = new WindowOption();
    option->SetWindowName("GetContentInfo");
    sptr<WindowSessionImpl> window = new WindowSessionImpl(option);

    SessionInfo sessionInfo = {"CreateTestBundle", "CreateTestModule",
                               "CreateTestAbility"};
    sptr<SessionMocker> session = new (std::nothrow) SessionMocker(sessionInfo);
    ASSERT_NE(nullptr, session);
    ASSERT_EQ(WMError::WM_OK, window->Create(nullptr, session));

    std::string res = window->GetContentInfo();
    ASSERT_EQ(res, "");
    window->uiContent_ = nullptr;
    res = window->GetContentInfo();
    ASSERT_EQ(res, "");
    ASSERT_EQ(WMError::WM_ERROR_INVALID_WINDOW, window->Destroy());
    GTEST_LOG_(INFO) << "WindowSessionImplTest: GetContentInfo end";
}

/**
 * @tc.name: OnNewWant
 * @tc.desc: OnNewWant
 * @tc.type: FUNC
 */
HWTEST_F(WindowSessionImplTest, OnNewWant, Function | SmallTest | Level2)
{
    GTEST_LOG_(INFO) << "WindowSessionImplTest: OnNewWant start";
    sptr<WindowOption> option = new WindowOption();
    option->SetWindowName("OnNewWant");
    sptr<WindowSessionImpl> window = new WindowSessionImpl(option);

    SessionInfo sessionInfo = {"CreateTestBundle", "CreateTestModule",
                               "CreateTestAbility"};
    sptr<SessionMocker> session = new (std::nothrow) SessionMocker(sessionInfo);
    ASSERT_NE(nullptr, session);
    ASSERT_EQ(WMError::WM_OK, window->Create(nullptr, session));

    AAFwk::Want want;
    window->uiContent_ = nullptr;
    window->OnNewWant(want);
    ASSERT_EQ(window->GetUIContentSharedPtr(), nullptr);
    window->uiContent_ = std::make_unique<Ace::UIContentMocker>();
    window->OnNewWant(want);
    ASSERT_NE(window->GetUIContentSharedPtr(), nullptr);

    ASSERT_EQ(WMError::WM_ERROR_INVALID_WINDOW, window->Destroy());
    GTEST_LOG_(INFO) << "WindowSessionImplTest: OnNewWant end";
}

/**
 * @tc.name: SetAPPWindowLabel
 * @tc.desc: SetAPPWindowLabel
 * @tc.type: FUNC
 */
HWTEST_F(WindowSessionImplTest, SetAPPWindowLabel, Function | SmallTest | Level2)
{
    GTEST_LOG_(INFO) << "WindowSessionImplTest: SetAPPWindowLabel start";
    sptr<WindowOption> option = new WindowOption();
    option->SetWindowName("SetAPPWindowLabel");
    sptr<WindowSessionImpl> window = new WindowSessionImpl(option);

    SessionInfo sessionInfo = {"CreateTestBundle", "CreateTestModule",
                               "CreateTestAbility"};
    sptr<SessionMocker> session = new (std::nothrow) SessionMocker(sessionInfo);
    ASSERT_NE(nullptr, session);
    ASSERT_EQ(WMError::WM_OK, window->Create(nullptr, session));

    std::string label = "label";
    window->uiContent_ = nullptr;
    WMError res = window->SetAPPWindowLabel(label);
    ASSERT_EQ(res, WMError::WM_ERROR_NULLPTR);

    window->uiContent_ = std::make_unique<Ace::UIContentMocker>();
    res = window->SetAPPWindowLabel(label);
    ASSERT_EQ(res, WMError::WM_OK);
    ASSERT_EQ(WMError::WM_ERROR_INVALID_WINDOW, window->Destroy());
    GTEST_LOG_(INFO) << "WindowSessionImplTest: SetAPPWindowLabel end";
}

/**
 * @tc.name: RegisterListener01
 * @tc.desc: RegisterListener and UnregisterListener
 * @tc.type: FUNC
 */
HWTEST_F(WindowSessionImplTest, RegisterListener01, Function | SmallTest | Level2)
{
    GTEST_LOG_(INFO) << "WindowSessionImplTest: RegisterListener01 start";
    sptr<WindowOption> option = new WindowOption();
    option->SetWindowName("RegisterListener01");
    sptr<WindowSessionImpl> window = new WindowSessionImpl(option);

    SessionInfo sessionInfo = {"CreateTestBundle", "CreateTestModule",
                               "CreateTestAbility"};
    sptr<SessionMocker> session = new (std::nothrow) SessionMocker(sessionInfo);
    ASSERT_NE(nullptr, session);
    ASSERT_EQ(WMError::WM_OK, window->Create(nullptr, session));

    sptr<IWindowLifeCycle> listener = nullptr;
    WMError res = window->RegisterLifeCycleListener(listener);
    ASSERT_EQ(res, WMError::WM_ERROR_NULLPTR);
    res = window->UnregisterLifeCycleListener(listener);
    ASSERT_EQ(res, WMError::WM_ERROR_NULLPTR);

    sptr<IOccupiedAreaChangeListener> listener1 = nullptr;
    res = window->RegisterOccupiedAreaChangeListener(listener1);
    ASSERT_EQ(res, WMError::WM_ERROR_NULLPTR);
    res = window->UnregisterOccupiedAreaChangeListener(listener1);
    ASSERT_EQ(res, WMError::WM_ERROR_NULLPTR);

    sptr<IWindowChangeListener> listener2 = nullptr;
    res = window->RegisterWindowChangeListener(listener2);
    ASSERT_EQ(res, WMError::WM_ERROR_NULLPTR);
    res = window->UnregisterWindowChangeListener(listener2);
    ASSERT_EQ(res, WMError::WM_ERROR_NULLPTR);

    sptr<IDialogDeathRecipientListener> listener3 = nullptr;
    window->RegisterDialogDeathRecipientListener(listener3);
    window->UnregisterDialogDeathRecipientListener(listener3);

    sptr<IDialogTargetTouchListener> listener4 = nullptr;
    res = window->RegisterDialogTargetTouchListener(listener4);
    ASSERT_EQ(res, WMError::WM_ERROR_NULLPTR);
    res = window->UnregisterDialogTargetTouchListener(listener4);
    ASSERT_EQ(res, WMError::WM_ERROR_NULLPTR);

    sptr<IWindowStatusChangeListener> listener5 = nullptr;
    res = window->RegisterWindowStatusChangeListener(listener5);
    ASSERT_EQ(res, WMError::WM_ERROR_NULLPTR);
    res = window->UnregisterWindowStatusChangeListener(listener5);
    ASSERT_EQ(res, WMError::WM_ERROR_NULLPTR);

    sptr<IDisplayMoveListener> listener6 = nullptr;
    res = window->RegisterDisplayMoveListener(listener6);
    ASSERT_EQ(res, WMError::WM_ERROR_NULLPTR);
    res = window->UnregisterDisplayMoveListener(listener6);
    ASSERT_EQ(res, WMError::WM_ERROR_NULLPTR);

    sptr<IWindowRectChangeListener> listener7 = nullptr;
    res = window->RegisterWindowRectChangeListener(listener7);
    ASSERT_EQ(res, WMError::WM_ERROR_NULLPTR);
    ASSERT_EQ(WMError::WM_ERROR_INVALID_WINDOW, window->Destroy());
    GTEST_LOG_(INFO) << "WindowSessionImplTest: RegisterListener01 end";
}

/**
 * @tc.name: RegisterListener02
 * @tc.desc: RegisterListener and UnregisterListener
 * @tc.type: FUNC
 */
HWTEST_F(WindowSessionImplTest, RegisterListener02, Function | SmallTest | Level2)
{
    GTEST_LOG_(INFO) << "WindowSessionImplTest: RegisterListener02 start";
    sptr<WindowOption> option = new WindowOption();
    option->SetWindowName("RegisterListener02");
    sptr<WindowSessionImpl> window = new WindowSessionImpl(option);

    SessionInfo sessionInfo = {"CreateTestBundle", "CreateTestModule",
                               "CreateTestAbility"};
    sptr<SessionMocker> session = new (std::nothrow) SessionMocker(sessionInfo);
    ASSERT_NE(nullptr, session);
    ASSERT_EQ(WMError::WM_OK, window->Create(nullptr, session));

    sptr<IScreenshotListener> listener5 = nullptr;
    WMError res = window->RegisterScreenshotListener(listener5);
    ASSERT_EQ(res, WMError::WM_ERROR_NULLPTR);
    res = window->UnregisterScreenshotListener(listener5);
    ASSERT_EQ(res, WMError::WM_ERROR_NULLPTR);

    sptr<IAvoidAreaChangedListener> listener6 = nullptr;
    res = window->RegisterAvoidAreaChangeListener(listener6);
    ASSERT_EQ(res, WMError::WM_ERROR_NULLPTR);
    res = window->UnregisterAvoidAreaChangeListener(listener6);
    ASSERT_EQ(res, WMError::WM_ERROR_NULLPTR);

    sptr<ITouchOutsideListener> listener7 = nullptr;
    res = window->RegisterTouchOutsideListener(listener7);
    ASSERT_EQ(res, WMError::WM_ERROR_NULLPTR);
    res = window->UnregisterTouchOutsideListener(listener7);
    ASSERT_EQ(res, WMError::WM_ERROR_NULLPTR);

    IWindowVisibilityListenerSptr listener8 = nullptr;
    res = window->RegisterWindowVisibilityChangeListener(listener8);
    ASSERT_EQ(res, WMError::WM_ERROR_NULLPTR);
    res = window->UnregisterWindowVisibilityChangeListener(listener8);
    ASSERT_EQ(res, WMError::WM_ERROR_NULLPTR);

    sptr<IWindowTitleButtonRectChangedListener> listener9 = nullptr;
    res = window->RegisterWindowTitleButtonRectChangeListener(listener9);
    ASSERT_EQ(res, WMError::WM_ERROR_NULLPTR);
    res = window->UnregisterWindowTitleButtonRectChangeListener(listener9);
    ASSERT_EQ(res, WMError::WM_ERROR_NULLPTR);

    sptr<ISubWindowCloseListener> listener10 = nullptr;
    res = window->RegisterSubWindowCloseListeners(listener10);
    ASSERT_EQ(res, WMError::WM_ERROR_NULLPTR);
    res = window->UnregisterSubWindowCloseListeners(listener10);
    ASSERT_EQ(res, WMError::WM_ERROR_NULLPTR);
    ASSERT_EQ(WMError::WM_ERROR_INVALID_WINDOW, window->Destroy());
    GTEST_LOG_(INFO) << "WindowSessionImplTest: RegisterListener02 end";
}

/**
 * @tc.name: NotifyDisplayMove
 * @tc.desc: NotifyDisplayMove
 * @tc.type: FUNC
 */
HWTEST_F(WindowSessionImplTest, NotifyDisplayMove, Function | SmallTest | Level2)
{
    GTEST_LOG_(INFO) << "WindowSessionImplTest: NotifyDisplayMove start";
    sptr<WindowOption> option = new WindowOption();
    option->SetWindowName("NotifyDisplayMove");
    sptr<WindowSessionImpl> window = new WindowSessionImpl(option);

    SessionInfo sessionInfo = {"CreateTestBundle", "CreateTestModule",
                               "CreateTestAbility"};
    sptr<SessionMocker> session = new (std::nothrow) SessionMocker(sessionInfo);
    ASSERT_NE(nullptr, session);
    ASSERT_EQ(WMError::WM_OK, window->Create(nullptr, session));

    int res = 0;
    DisplayId from = 0;
    DisplayId to = 2;
    window->NotifyDisplayMove(from, to);
    ASSERT_EQ(res, 0);
    ASSERT_EQ(WMError::WM_ERROR_INVALID_WINDOW, window->Destroy());
    GTEST_LOG_(INFO) << "WindowSessionImplTest: NotifyDisplayMove end";
}

/**
 * @tc.name: NotifyAfterForeground
 * @tc.desc: NotifyAfterForeground
 * @tc.type: FUNC
 */
HWTEST_F(WindowSessionImplTest, NotifyAfterForeground, Function | SmallTest | Level2)
{
    GTEST_LOG_(INFO) << "WindowSessionImplTest: NotifyAfterForeground start";
    sptr<WindowOption> option = new WindowOption();
    option->SetWindowName("NotifyAfterForeground");
    sptr<WindowSessionImpl> window = new WindowSessionImpl(option);

    SessionInfo sessionInfo = {"CreateTestBundle", "CreateTestModule",
                               "CreateTestAbility"};
    sptr<SessionMocker> session = new (std::nothrow) SessionMocker(sessionInfo);
    ASSERT_NE(nullptr, session);
    ASSERT_EQ(WMError::WM_OK, window->Create(nullptr, session));

    int res = 0;
    window->NotifyAfterForeground(true, true);
    window->NotifyAfterForeground(false, false);
    window->vsyncStation_ = nullptr;
    window->NotifyAfterForeground(false, false);
    ASSERT_EQ(res, 0);
    ASSERT_EQ(WMError::WM_ERROR_INVALID_WINDOW, window->Destroy());
    GTEST_LOG_(INFO) << "WindowSessionImplTest: NotifyAfterForeground end";
}

/**
 * @tc.name: NotifyAfterBackground
 * @tc.desc: NotifyAfterBackground
 * @tc.type: FUNC
 */
HWTEST_F(WindowSessionImplTest, NotifyAfterBackground, Function | SmallTest | Level2)
{
    GTEST_LOG_(INFO) << "WindowSessionImplTest: NotifyAfterBackground start";
    sptr<WindowOption> option = new WindowOption();
    option->SetWindowName("NotifyAfterBackground");
    sptr<WindowSessionImpl> window = new WindowSessionImpl(option);

    SessionInfo sessionInfo = {"CreateTestBundle", "CreateTestModule",
                               "CreateTestAbility"};
    sptr<SessionMocker> session = new (std::nothrow) SessionMocker(sessionInfo);
    ASSERT_NE(nullptr, session);
    ASSERT_EQ(WMError::WM_OK, window->Create(nullptr, session));

    int res = 0;
    window->NotifyAfterBackground(true, true);
    window->NotifyAfterBackground(false, false);
    window->vsyncStation_ = nullptr;
    window->NotifyAfterBackground(false, false);
    ASSERT_EQ(res, 0);
    ASSERT_EQ(WMError::WM_ERROR_INVALID_WINDOW, window->Destroy());
    GTEST_LOG_(INFO) << "WindowSessionImplTest: NotifyAfterBackground end";
}

/**
 * @tc.name: NotifyAfterUnfocused
 * @tc.desc: NotifyAfterUnfocused
 * @tc.type: FUNC
 */
HWTEST_F(WindowSessionImplTest, NotifyAfterUnfocused, Function | SmallTest | Level2)
{
    GTEST_LOG_(INFO) << "WindowSessionImplTest: NotifyAfterUnfocused start";
    sptr<WindowOption> option = new WindowOption();
    option->SetWindowName("NotifyAfterUnfocused");
    sptr<WindowSessionImpl> window = new WindowSessionImpl(option);

    SessionInfo sessionInfo = {"CreateTestBundle", "CreateTestModule",
                               "CreateTestAbility"};
    sptr<SessionMocker> session = new (std::nothrow) SessionMocker(sessionInfo);
    ASSERT_NE(nullptr, session);
    ASSERT_EQ(WMError::WM_OK, window->Create(nullptr, session));

    int res = 0;
    window->NotifyAfterUnfocused(true);
    window->NotifyAfterUnfocused(false);
    ASSERT_EQ(res, 0);

    OHOS::Ace::UIContentErrorCode aceRet = OHOS::Ace::UIContentErrorCode::NO_ERRORS;
    window->InitUIContent("NotifyAfterUnfocused", nullptr, nullptr, WindowSetUIContentType::DEFAULT,
        nullptr, aceRet);
    window->NotifyAfterUnfocused(true);
    ASSERT_NE(window->GetUIContentSharedPtr(), nullptr);
    ASSERT_EQ(WMError::WM_ERROR_INVALID_WINDOW, window->Destroy());
    GTEST_LOG_(INFO) << "WindowSessionImplTest: NotifyAfterUnfocused end";
}

/**
 * @tc.name: NotifyForegroundInteractiveStatus
 * @tc.desc: NotifyForegroundInteractiveStatus
 * @tc.type: FUNC
 */
HWTEST_F(WindowSessionImplTest, NotifyForegroundInteractiveStatus, Function | SmallTest | Level2)
{
    GTEST_LOG_(INFO) << "WindowSessionImplTest: NotifyForegroundInteractiveStatus start";
    sptr<WindowOption> option = new WindowOption();
    ASSERT_NE(option, nullptr);
    option->SetWindowName("NotifyForegroundInteractiveStatus");
    sptr<WindowSessionImpl> window =
        new (std::nothrow) WindowSessionImpl(option);
    ASSERT_NE(window, nullptr);

    int res = 0;
    window->NotifyForegroundInteractiveStatus(true);
    window->NotifyForegroundInteractiveStatus(false);
    ASSERT_EQ(res, 0);

    GTEST_LOG_(INFO) << "WindowSessionImplTest: NotifyForegroundInteractiveStatus end";
}

/**
 * @tc.name: NotifyBeforeDestroy
 * @tc.desc: NotifyBeforeDestroy
 * @tc.type: FUNC
 */
HWTEST_F(WindowSessionImplTest, NotifyBeforeDestroy, Function | SmallTest | Level2)
{
    GTEST_LOG_(INFO) << "WindowSessionImplTest: NotifyBeforeDestroy start";
    sptr<WindowOption> option = new WindowOption();
    option->SetWindowName("NotifyBeforeDestroy");
    sptr<WindowSessionImpl> window = new WindowSessionImpl(option);

    SessionInfo sessionInfo = {"CreateTestBundle", "CreateTestModule",
                               "CreateTestAbility"};
    sptr<SessionMocker> session = new (std::nothrow) SessionMocker(sessionInfo);
    ASSERT_NE(nullptr, session);
    ASSERT_EQ(WMError::WM_OK, window->Create(nullptr, session));

    int res = 0;
    std::string windowName = "NotifyBeforeDestroy";
    window->NotifyBeforeDestroy(windowName);
    window->handler_ = nullptr;
    window->NotifyBeforeDestroy(windowName);
    ASSERT_EQ(res, 0);

    // uiContent!=nullptr
    OHOS::Ace::UIContentErrorCode aceRet = OHOS::Ace::UIContentErrorCode::NO_ERRORS;
    window->InitUIContent("NotifyAfterUnfocused", nullptr, nullptr, WindowSetUIContentType::DEFAULT,
        nullptr, aceRet);
    ASSERT_NE(window->uiContent_, nullptr);
    window->NotifyBeforeDestroy(windowName);
    ASSERT_EQ(window->uiContent_, nullptr);

    // notifyNativeFunc_!=nullptr
    NotifyNativeWinDestroyFunc func = [&](std::string name)
    {
        GTEST_LOG_(INFO) << "NotifyNativeWinDestroyFunc";
        ASSERT_EQ(windowName, name);
    };
    window->RegisterWindowDestroyedListener(func);
    window->NotifyBeforeDestroy(windowName);

    ASSERT_EQ(WMError::WM_ERROR_INVALID_WINDOW, window->Destroy());
    GTEST_LOG_(INFO) << "WindowSessionImplTest: NotifyBeforeDestroy end";
}

/**
 * @tc.name: MarkProcessed
 * @tc.desc: MarkProcessed
 * @tc.type: FUNC
 */
HWTEST_F(WindowSessionImplTest, MarkProcessed, Function | SmallTest | Level2)
{
    GTEST_LOG_(INFO) << "WindowSessionImplTest: MarkProcessed start";
    sptr<WindowOption> option = new WindowOption();
    option->SetWindowName("MarkProcessed");
    sptr<WindowSessionImpl> window = new WindowSessionImpl(option);

    SessionInfo sessionInfo = {"CreateTestBundle", "CreateTestModule",
                               "CreateTestAbility"};
    sptr<SessionMocker> session = new (std::nothrow) SessionMocker(sessionInfo);
    ASSERT_NE(nullptr, session);
    ASSERT_EQ(WMError::WM_OK, window->Create(nullptr, session));

    int32_t eventId = 1;
    window->state_ = WindowState::STATE_DESTROYED;
    window->hostSession_ = session;
    ASSERT_EQ(window->GetPersistentId(), INVALID_SESSION_ID);
    ASSERT_EQ(window->state_, WindowState::STATE_DESTROYED);
    WSError res = window->MarkProcessed(eventId);
    ASSERT_EQ(res, WSError::WS_DO_NOTHING);
    window->hostSession_ = nullptr;
    res = window->MarkProcessed(eventId);
    ASSERT_EQ(res, WSError::WS_DO_NOTHING);
    ASSERT_EQ(WMError::WM_ERROR_INVALID_WINDOW, window->Destroy());
    GTEST_LOG_(INFO) << "WindowSessionImplTest: MarkProcessed end";
}

/**
 * @tc.name: Notify01
 * @tc.desc: NotifyDestroy NotifyTouchDialogTarget NotifyScreenshot
 * @tc.type: FUNC
 */
HWTEST_F(WindowSessionImplTest, Notify01, Function | SmallTest | Level2)
{
    GTEST_LOG_(INFO) << "WindowSessionImplTest: Notify01 start";
    sptr<WindowOption> option = new WindowOption();
    option->SetWindowName("Notify01");
    sptr<WindowSessionImpl> window = new WindowSessionImpl(option);

    SessionInfo sessionInfo = {"CreateTestBundle", "CreateTestModule",
                               "CreateTestAbility"};
    sptr<SessionMocker> session = new (std::nothrow) SessionMocker(sessionInfo);
    ASSERT_NE(nullptr, session);
    ASSERT_EQ(WMError::WM_OK, window->Create(nullptr, session));

    window->NotifyTouchDialogTarget();
    window->NotifyScreenshot();
    WSError res = window->NotifyDestroy();
    ASSERT_EQ(res, WSError::WS_OK);
    ASSERT_EQ(WMError::WM_ERROR_INVALID_WINDOW, window->Destroy());
    GTEST_LOG_(INFO) << "WindowSessionImplTest: Notify01 end";
}

/**
 * @tc.name: NotifyKeyEvent
 * @tc.desc: NotifyKeyEvent
 * @tc.type: FUNC
 */
HWTEST_F(WindowSessionImplTest, NotifyKeyEvent, Function | SmallTest | Level2)
{
    GTEST_LOG_(INFO) << "WindowSessionImplTest: NotifyKeyEvent start";
    sptr<WindowOption> option = new WindowOption();
    option->SetWindowName("NotifyKeyEvent");
    sptr<WindowSessionImpl> window = new WindowSessionImpl(option);

    SessionInfo sessionInfo = {"CreateTestBundle", "CreateTestModule",
                               "CreateTestAbility"};
    sptr<SessionMocker> session = new (std::nothrow) SessionMocker(sessionInfo);
    ASSERT_NE(nullptr, session);
    ASSERT_EQ(WMError::WM_OK, window->Create(nullptr, session));

    int res = 0;
    std::shared_ptr<MMI::KeyEvent> keyEvent = MMI::KeyEvent::Create();
    bool isConsumed = false;
    bool notifyInputMethod = false;
    keyEvent->SetKeyCode(MMI::KeyEvent::KEYCODE_VIRTUAL_MULTITASK);
    window->NotifyKeyEvent(keyEvent, isConsumed, notifyInputMethod);

    keyEvent->SetKeyCode(MMI::KeyEvent::KEYCODE_BACK);
    window->NotifyKeyEvent(keyEvent, isConsumed, notifyInputMethod);

    notifyInputMethod = true;
    window->NotifyKeyEvent(keyEvent, isConsumed, notifyInputMethod);

    keyEvent = nullptr;
    window->NotifyKeyEvent(keyEvent, isConsumed, notifyInputMethod);
    ASSERT_EQ(res, 0);
    ASSERT_EQ(WMError::WM_ERROR_INVALID_WINDOW, window->Destroy());
    GTEST_LOG_(INFO) << "WindowSessionImplTest: NotifyKeyEvent end";
}

/**
 * @tc.name: UpdateProperty
 * @tc.desc: UpdateProperty
 * @tc.type: FUNC
 */
HWTEST_F(WindowSessionImplTest, UpdateProperty, Function | SmallTest | Level2)
{
    GTEST_LOG_(INFO) << "WindowSessionImplTest: UpdateProperty start";
    sptr<WindowOption> option = new WindowOption();
    option->SetWindowName("UpdateProperty");
    sptr<WindowSessionImpl> window = new WindowSessionImpl(option);

    SessionInfo sessionInfo = {"CreateTestBundle", "CreateTestModule",
                               "CreateTestAbility"};
    sptr<SessionMocker> session = new (std::nothrow) SessionMocker(sessionInfo);
    ASSERT_NE(nullptr, session);
    ASSERT_EQ(WMError::WM_OK, window->Create(nullptr, session));

    WMError res = window->UpdateProperty(WSPropertyChangeAction::ACTION_UPDATE_RECT);
    ASSERT_EQ(res, WMError::WM_ERROR_INVALID_WINDOW);
    ASSERT_EQ(WMError::WM_ERROR_INVALID_WINDOW, window->Destroy());
    // session is null
    window = new WindowSessionImpl(option);
    ASSERT_EQ(WMError::WM_OK, window->Create(abilityContext_, nullptr));
    res = window->UpdateProperty(WSPropertyChangeAction::ACTION_UPDATE_RECT);
    ASSERT_EQ(res, WMError::WM_ERROR_INVALID_WINDOW);
    ASSERT_EQ(WMError::WM_ERROR_INVALID_WINDOW, window->Destroy());
    GTEST_LOG_(INFO) << "WindowSessionImplTest: UpdateProperty end";
}

/**
 * @tc.name: Find
 * @tc.desc: Find
 * @tc.type: FUNC
 */
HWTEST_F(WindowSessionImplTest, Find, Function | SmallTest | Level2)
{
    GTEST_LOG_(INFO) << "WindowSessionImplTest: Find start";
    sptr<WindowOption> option = new WindowOption();
    option->SetWindowName("Find");
    sptr<WindowSessionImpl> window = new WindowSessionImpl(option);

    SessionInfo sessionInfo = {"CreateTestBundle", "CreateTestModule",
                               "CreateTestAbility"};
    sptr<SessionMocker> session = new (std::nothrow) SessionMocker(sessionInfo);
    ASSERT_NE(nullptr, session);
    ASSERT_EQ(WMError::WM_OK, window->Create(nullptr, session));

    std::string name = "Find";
    sptr<Window> res = window->Find(name);
    ASSERT_EQ(res, nullptr);

    name = "111";
    res = window->Find(name);
    ASSERT_EQ(res, nullptr);
    ASSERT_EQ(WMError::WM_ERROR_INVALID_WINDOW, window->Destroy());
    GTEST_LOG_(INFO) << "WindowSessionImplTest: Find end";
}

/**
 * @tc.name: SetBackgroundColor01
 * @tc.desc: SetBackgroundColor string
 * @tc.type: FUNC
 */
HWTEST_F(WindowSessionImplTest, SetBackgroundColor01, Function | SmallTest | Level2)
{
    GTEST_LOG_(INFO) << "WindowSessionImplTest: SetBackgroundColor01 start";
    sptr<WindowOption> option = new WindowOption();
    option->SetWindowName("SetBackgroundColor01");
    sptr<WindowSessionImpl> window = new WindowSessionImpl(option);

    SessionInfo sessionInfo = {"CreateTestBundle", "CreateTestModule",
                               "CreateTestAbility"};
    sptr<SessionMocker> session = new (std::nothrow) SessionMocker(sessionInfo);
    ASSERT_NE(nullptr, session);
    ASSERT_EQ(WMError::WM_OK, window->Create(abilityContext_, session));

    std::string color = "Blue";
    WMError res = window->SetBackgroundColor(color);
    ASSERT_EQ(res, WMError::WM_ERROR_INVALID_WINDOW);

    color = "111";
    res = window->SetBackgroundColor(color);
    ASSERT_EQ(res, WMError::WM_ERROR_INVALID_WINDOW);
    ASSERT_EQ(WMError::WM_ERROR_INVALID_WINDOW, window->Destroy());
    // session is null
    window = new WindowSessionImpl(option);
    ASSERT_EQ(WMError::WM_OK, window->Create(abilityContext_, nullptr));
    res = window->SetBackgroundColor(color);
    ASSERT_EQ(res, WMError::WM_ERROR_INVALID_WINDOW);

    color = "Blue";
    window->property_->SetPersistentId(1);
    window->state_ = WindowState::STATE_SHOWN;
    window->hostSession_ = session;
    ASSERT_FALSE(window->IsWindowSessionInvalid());
    uint32_t colorValue;
    ASSERT_FALSE(ColorParser::Parse(color, colorValue));
    res = window->SetBackgroundColor(color);
    ASSERT_EQ(res, WMError::WM_ERROR_INVALID_PARAM);

    color = "#FFFFFF00";
    ASSERT_TRUE(ColorParser::Parse(color, colorValue));
    res = window->SetBackgroundColor(color);
    ASSERT_EQ(res, WMError::WM_ERROR_INVALID_OPERATION);
    ASSERT_EQ(WMError::WM_OK, window->Destroy());
    GTEST_LOG_(INFO) << "WindowSessionImplTest: SetBackgroundColor01 end";
}

/**
 * @tc.name: SetBackgroundColor02
 * @tc.desc: SetBackgroundColor(uint32_t) and GetBackgroundColor
 * @tc.type: FUNC
 */
HWTEST_F(WindowSessionImplTest, SetBackgroundColor02, Function | SmallTest | Level2)
{
    GTEST_LOG_(INFO) << "WindowSessionImplTest: SetBackgroundColor02 start";
    sptr<WindowOption> option = new WindowOption();
    option->SetWindowName("SetBackgroundColor02");
    sptr<WindowSessionImpl> window = new WindowSessionImpl(option);

    SessionInfo sessionInfo = {"CreateTestBundle", "CreateTestModule",
                               "CreateTestAbility"};
    sptr<SessionMocker> session = new (std::nothrow) SessionMocker(sessionInfo);
    ASSERT_NE(nullptr, session);
    ASSERT_EQ(WMError::WM_OK, window->Create(nullptr, session));

    WMError res = window->SetBackgroundColor(0xffffffff);
    ASSERT_EQ(res, WMError::WM_ERROR_INVALID_OPERATION);
    uint32_t ret = window->GetBackgroundColor();
    ASSERT_EQ(ret, 0xffffffff);

    ASSERT_EQ(window->aceAbilityHandler_, nullptr);
    sptr<IAceAbilityHandler> handler = new (std::nothrow) MockIAceAbilityHandler();
    window->SetAceAbilityHandler(handler);
    res = window->SetBackgroundColor(0xffffffff);
    ASSERT_NE(window->aceAbilityHandler_, nullptr);
    ASSERT_EQ(res, WMError::WM_OK);

    ASSERT_TRUE(!(0xff0000 & 0xff000000));
    ASSERT_TRUE(WindowHelper::IsMainWindow(window->GetType()));
    window->uiContent_ = std::make_unique<Ace::UIContentMocker>();
    res = window->SetBackgroundColor(0xff0000);
    ASSERT_EQ(res, WMError::WM_OK);
    ASSERT_EQ(WMError::WM_ERROR_INVALID_WINDOW, window->Destroy());
    GTEST_LOG_(INFO) << "WindowSessionImplTest: SetBackgroundColor02 end";
}

/**
 * @tc.name: SetAPPWindowIcon
 * @tc.desc: SetAPPWindowIcon
 * @tc.type: FUNC
 */
HWTEST_F(WindowSessionImplTest, SetAPPWindowIcon, Function | SmallTest | Level2)
{
    GTEST_LOG_(INFO) << "WindowSessionImplTest: SetAPPWindowIcon start";
    sptr<WindowOption> option = new WindowOption();
    option->SetWindowName("SetAPPWindowIcon");
    sptr<WindowSessionImpl> window = new WindowSessionImpl(option);

    SessionInfo sessionInfo = {"CreateTestBundle", "CreateTestModule",
                               "CreateTestAbility"};
    sptr<SessionMocker> session = new (std::nothrow) SessionMocker(sessionInfo);
    ASSERT_NE(nullptr, session);
    ASSERT_EQ(WMError::WM_OK, window->Create(nullptr, session));
    std::shared_ptr<Media::PixelMap> icon1(nullptr);
    WMError res = window->SetAPPWindowIcon(icon1);
    ASSERT_EQ(res, WMError::WM_ERROR_NULLPTR);

    std::shared_ptr<Media::PixelMap> icon2 = std::shared_ptr<Media::PixelMap>();
    res = window->SetAPPWindowIcon(icon2);
    ASSERT_EQ(res, WMError::WM_ERROR_NULLPTR);

    Media::InitializationOptions opts;
    opts.size.width = 200;  // 200： test width
    opts.size.height = 300; // 300： test height
    opts.pixelFormat = Media::PixelFormat::ARGB_8888;
    opts.alphaType = Media::AlphaType::IMAGE_ALPHA_TYPE_OPAQUE;
    std::unique_ptr<Media::PixelMap> pixelMapPtr = Media::PixelMap::Create(opts);
    ASSERT_NE(pixelMapPtr.get(), nullptr);
    window->uiContent_ = std::make_unique<Ace::UIContentMocker>();
    res = window->SetAPPWindowIcon(std::shared_ptr<Media::PixelMap>(pixelMapPtr.release()));
    ASSERT_EQ(res, WMError::WM_OK);
    ASSERT_NE(window->GetUIContentSharedPtr(), nullptr);
    ASSERT_EQ(WMError::WM_ERROR_INVALID_WINDOW, window->Destroy());
    GTEST_LOG_(INFO) << "WindowSessionImplTest: SetAPPWindowIcon end";
}

/**
 * @tc.name: Notify02
 * @tc.desc: NotifyAvoidAreaChange NotifyPointerEvent NotifyTouchOutside NotifyWindowVisibility
 * @tc.type: FUNC
 */
HWTEST_F(WindowSessionImplTest, Notify02, Function | SmallTest | Level2)
{
    GTEST_LOG_(INFO) << "WindowSessionImplTest: Notify02 start";
    sptr<WindowOption> option = new WindowOption();
    option->SetWindowName("Notify02");
    sptr<WindowSessionImpl> window = new WindowSessionImpl(option);

    SessionInfo sessionInfo = {"CreateTestBundle", "CreateTestModule",
                               "CreateTestAbility"};
    sptr<SessionMocker> session = new (std::nothrow) SessionMocker(sessionInfo);
    ASSERT_NE(nullptr, session);
    ASSERT_EQ(WMError::WM_OK, window->Create(nullptr, session));

    sptr<AvoidArea> avoidArea = new AvoidArea();
    avoidArea->topRect_ = { 1, 0, 0, 0 };
    avoidArea->leftRect_ = { 0, 1, 0, 0 };
    avoidArea->rightRect_ = { 0, 0, 1, 0 };
    avoidArea->bottomRect_ = { 0, 0, 0, 1 };
    AvoidAreaType type = AvoidAreaType::TYPE_SYSTEM;
    window->NotifyAvoidAreaChange(avoidArea, type);

    std::shared_ptr<MMI::PointerEvent> pointerEvent = MMI::PointerEvent::Create();
    window->NotifyPointerEvent(pointerEvent);
    WSError res = window->NotifyTouchOutside();
    ASSERT_EQ(res, WSError::WS_OK);

    res = window->NotifyWindowVisibility(true);
    ASSERT_EQ(res, WSError::WS_OK);
    bool terminateCloseProcess = false;
    window->NotifySubWindowClose(terminateCloseProcess);
    ASSERT_EQ(terminateCloseProcess, false);

    ASSERT_EQ(WMError::WM_ERROR_INVALID_WINDOW, window->Destroy());
    GTEST_LOG_(INFO) << "WindowSessionImplTest: Notify02 end";
}

/**
 * @tc.name: SetAceAbilityHandler
 * @tc.desc: SetAceAbilityHandler
 * @tc.type: FUNC
 */
HWTEST_F(WindowSessionImplTest, SetAceAbilityHandler, Function | SmallTest | Level2)
{
    GTEST_LOG_(INFO) << "WindowSessionImplTest: SetAceAbilityHandler start";
    sptr<WindowOption> option = new WindowOption();
    option->SetWindowName("SetAceAbilityHandler");
    sptr<WindowSessionImpl> window = new WindowSessionImpl(option);

    SessionInfo sessionInfo = {"CreateTestBundle", "CreateTestModule",
                               "CreateTestAbility"};
    sptr<SessionMocker> session = new (std::nothrow) SessionMocker(sessionInfo);
    ASSERT_NE(nullptr, session);
    ASSERT_EQ(WMError::WM_OK, window->Create(nullptr, session));

    int res = 0;
    sptr<IAceAbilityHandler> handler = sptr<IAceAbilityHandler>();
    ASSERT_EQ(handler, nullptr);
    GTEST_LOG_(INFO) << "WindowSessionImplTest: SetAceAbilityHandler 111";
    window->SetAceAbilityHandler(handler);
    ASSERT_EQ(res, 0);
    ASSERT_EQ(WMError::WM_ERROR_INVALID_WINDOW, window->Destroy());
    GTEST_LOG_(INFO) << "WindowSessionImplTest: SetAceAbilityHandler end";
}

/**
 * @tc.name: SetRaiseByClickEnabled01
 * @tc.desc: SetRaiseByClickEnabled and check the retCode
 * @tc.type: FUNC
 */
HWTEST_F(WindowSessionImplTest, SetRaiseByClickEnabled01, Function | SmallTest | Level2)
{
    sptr<WindowOption> option = new WindowOption();
    option->SetWindowName("SetRaiseByClickEnabled01");
    sptr<WindowSessionImpl> window = new(std::nothrow) WindowSessionImpl(option);
    ASSERT_NE(nullptr, window);
    WMError retCode = window->SetRaiseByClickEnabled(true);
    ASSERT_EQ(retCode, WMError::WM_ERROR_INVALID_WINDOW);
    window->property_->SetPersistentId(1);
    SessionInfo sessionInfo = { "CreateTestBundle", "CreateTestModule", "CreateTestAbility" };
    sptr<SessionMocker> session = new(std::nothrow) SessionMocker(sessionInfo);
    ASSERT_NE(nullptr, session);
    window->hostSession_ = session;
    window->state_ = WindowState::STATE_CREATED;
    window->SetRaiseByClickEnabled(true);
    ASSERT_NE(nullptr, session);
}

/**
 * @tc.name: HideNonSystemFloatingWindows01
 * @tc.desc: HideNonSystemFloatingWindows and check the retCode
 * @tc.type: FUNC
 */
HWTEST_F(WindowSessionImplTest, HideNonSystemFloatingWindows01, Function | SmallTest | Level2)
{
    sptr<WindowOption> option = new WindowOption();
    option->SetWindowName("HideNonSystemFloatingWindows01");
    sptr<WindowSessionImpl> window = new(std::nothrow) WindowSessionImpl(option);
    ASSERT_NE(nullptr, window);
    WMError retCode = window->HideNonSystemFloatingWindows(false);
    ASSERT_EQ(retCode, WMError::WM_ERROR_INVALID_WINDOW);
    window->property_->SetPersistentId(1);
    SessionInfo sessionInfo = { "CreateTestBundle", "CreateTestModule", "CreateTestAbility" };
    sptr<SessionMocker> session = new(std::nothrow) SessionMocker(sessionInfo);
    ASSERT_NE(nullptr, session);
    window->hostSession_ = session;
    window->state_ = WindowState::STATE_CREATED;
    window->HideNonSystemFloatingWindows(false);
}

/**
 * @tc.name: UpdateWindowModetest01
 * @tc.desc: UpdateWindowMode
 * @tc.type: FUNC
 */
HWTEST_F(WindowSessionImplTest, UpdateWindowMode, Function | SmallTest | Level2)
{
    GTEST_LOG_(INFO) << "WindowSessionImplTest: UpdateWindowModetest01 start";
    sptr<WindowOption> option = new WindowOption();
    ASSERT_NE(option, nullptr);
    option->SetWindowName("UpdateWindowMode");
    sptr<WindowSessionImpl> window = new (std::nothrow) WindowSessionImpl(option);
    ASSERT_NE(window, nullptr);
    WindowMode mode = WindowMode{0};
    auto ret = window->UpdateWindowMode(mode);
    ASSERT_EQ(ret, WSError::WS_OK);
    GTEST_LOG_(INFO) << "WindowSessionImplTest: UpdateWindowModetest01 end";
}

/**
 * @tc.name: UpdateDensitytest01
 * @tc.desc: UpdateDensity
 * @tc.type: FUNC
 */
HWTEST_F(WindowSessionImplTest, UpdateDensity, Function | SmallTest | Level2)
{
    GTEST_LOG_(INFO) << "WindowSessionImplTest: UpdateDensitytest01 start";
    sptr<WindowOption> option = new WindowOption();
    ASSERT_NE(option, nullptr);
    option->SetWindowName("UpdateDensity");
    sptr<WindowSessionImpl> window = new (std::nothrow) WindowSessionImpl(option);
    ASSERT_NE(window, nullptr);
    int ret = 0;
    window->UpdateDensity();
    ASSERT_EQ(ret, 0);
    GTEST_LOG_(INFO) << "WindowSessionImplTest: UpdateDensitytest01 end";
}

/**
 * @tc.name: UpdateDisplayIdtest01
 * @tc.desc: UpdateDisplayId
 * @tc.type: FUNC
 */
HWTEST_F(WindowSessionImplTest, UpdateDisplayId, Function | SmallTest | Level2)
{
    GTEST_LOG_(INFO) << "WindowSessionImplTest: UpdateDisplayIdtest01 start";
    sptr<WindowOption> option = new WindowOption();
    ASSERT_NE(option, nullptr);
    option->SetWindowName("UpdateDisplayId");
    sptr<WindowSessionImpl> window = new (std::nothrow) WindowSessionImpl(option);
    ASSERT_NE(window, nullptr);
    uint64_t newDisplayId = 2;
    auto ret = window->UpdateDisplayId(newDisplayId);
    ASSERT_EQ(ret, WSError::WS_OK);
    uint64_t displayId = window->property_->GetDisplayId();
    ASSERT_EQ(newDisplayId, displayId);
    GTEST_LOG_(INFO) << "WindowSessionImplTest: UpdateDisplayIdtest01 end";
}

/**
 * @tc.name: IsFloatingWindowAppTypetest01
 * @tc.desc: IsFloatingWindowAppType
 * @tc.type: FUNC
 */
HWTEST_F(WindowSessionImplTest, IsFloatingWindowAppType, Function | SmallTest | Level2)
{
    GTEST_LOG_(INFO) << "WindowSessionImplTest: IsFloatingWindowAppTypetest01 start";
    sptr<WindowOption> option = new WindowOption();
    ASSERT_NE(option, nullptr);
    option->SetWindowName("IsFloatingWindowAppType");
    sptr<WindowSessionImpl> window = new (std::nothrow) WindowSessionImpl(option);
    ASSERT_NE(window, nullptr);
    window->IsFloatingWindowAppType();
    ASSERT_NE(window, nullptr);
    GTEST_LOG_(INFO) << "WindowSessionImplTest: IsFloatingWindowAppTypetest01 end";
}

/**
 * @tc.name: SetUniqueVirtualPixelRatio
 * @tc.desc: SetUniqueVirtualPixelRatio
 * @tc.type: FUNC
 */
HWTEST_F(WindowSessionImplTest, SetUniqueVirtualPixelRatio, Function | SmallTest | Level2)
{
    sptr<WindowOption> option = new (std::nothrow) WindowOption();
    ASSERT_NE(option, nullptr);
    option->SetWindowName("SetUniqueVirtualPixelRatio");
    sptr<WindowSessionImpl> window = new (std::nothrow) WindowSessionImpl(option);
    ASSERT_NE(window, nullptr);
    window->SetUniqueVirtualPixelRatio(true, 3.25f);
    window->SetUniqueVirtualPixelRatio(false, 3.25f);
}
}
} // namespace Rosen
} // namespace OHOS
