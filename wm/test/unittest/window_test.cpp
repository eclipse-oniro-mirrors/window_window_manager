/*
 * Copyright (c) 2021-2022 Huawei Device Co., Ltd.
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
#include "ability_context_impl.h"
#include "accessibility_event_info.h"
#include "key_event.h"
#include "mock_window_adapter.h"
#include "scene_board_judgement.h"
#include "singleton_mocker.h"
#include "window.h"
#include "window_manager_hilog.h"
#include "window_session_impl.h"
#include "wm_common.h"

using namespace testing;
using namespace testing::ext;

namespace OHOS {
namespace {
    std::string g_logMsg;
    void MyLogCallback(const LogType type, const LogLevel level, const unsigned int domain, const char *tag,
        const char *msg)
    {
        g_logMsg = msg;
    }
}
namespace Rosen {
using Mocker = SingletonMocker<WindowAdapter, MockWindowAdapter>;
class WindowTest : public testing::Test {
public:
    static void SetUpTestCase();
    static void TearDownTestCase();
    virtual void SetUp() override;
    virtual void TearDown() override;
    static inline std::shared_ptr<AbilityRuntime::AbilityContext> abilityContext_;
};
void WindowTest::SetUpTestCase()
{
    abilityContext_ = std::make_shared<AbilityRuntime::AbilityContextImpl>();
}

void WindowTest::TearDownTestCase() {}

void WindowTest::SetUp() {}

void WindowTest::TearDown() {}

namespace {
/**
 * @tc.name: Create01
 * @tc.desc: Create window with no WindowName，no option and no context
 * @tc.type: FUNC
 */
HWTEST_F(WindowTest, Create01, TestSize.Level1)
{
    sptr<WindowOption> option = nullptr;
    ASSERT_EQ(nullptr, Window::Create("", option));
}

/**
 * @tc.name: Create02
 * @tc.desc: Create window with WindowName，no option and no context
 * @tc.type: FUNC
 */
HWTEST_F(WindowTest, Create02, TestSize.Level1)
{
    // no option: Window::Create with defult option
    // default option : default WindowType is WindowType::WINDOW_TYPE_APP_MAIN_WINDOW
    //                  default onlySupportSceneBoard_ is false
    sptr<WindowOption> option = nullptr;
    auto window = Window::Create("WindowTest02", option);
    // Create app main window need context and isession
    ASSERT_NE(nullptr, window);
}

/**
 * @tc.name: Create03
 * @tc.desc: Create window with WindowName, option and no context
 * @tc.type: FUNC
 */
HWTEST_F(WindowTest, Create03, TestSize.Level1)
{
    sptr<WindowOption> option = sptr<WindowOption>::MakeSptr();
    option->SetWindowType(WindowType::WINDOW_TYPE_UI_EXTENSION);
    // WindowType::WINDOW_TYPE_UI_EXTENSION is neither appWindow nor systemWindow
    auto window = Window::Create("WindowTest03", option);
    ASSERT_EQ(nullptr, window);
}

/**
 * @tc.name: Create04
 * @tc.desc: Create window with WindowName and no abilityToken
 * @tc.type: FUNC
 */
HWTEST_F(WindowTest, Create04, TestSize.Level1)
{
    sptr<WindowOption> option = sptr<WindowOption>::MakeSptr();
    // Create app TRANSPARENT_VIEW window but only support sceneBoard
    // Create app TRANSPARENT_VIEW window no need context and isession
    option->SetWindowType(WindowType::WINDOW_TYPE_TRANSPARENT_VIEW);
    option->SetOnlySupportSceneBoard(true);
    auto window = Window::Create("WindowTest04", option);
    if (SceneBoardJudgement::IsSceneBoardEnabled()) {
        ASSERT_NE(nullptr, window);
        ASSERT_EQ(WMError::WM_OK, window->Destroy());
    } else {
        ASSERT_EQ(nullptr, window);
    }
}

/**
 * @tc.name: Create05
 * @tc.desc: Create window with WindowName option and context
 * @tc.type: FUNC
 */
HWTEST_F(WindowTest, Create05, TestSize.Level1)
{
    std::unique_ptr<Mocker> m = std::make_unique<Mocker>();
    sptr<WindowOption> option = sptr<WindowOption>::MakeSptr();
    ASSERT_NE(nullptr, Window::Create("WindowTest05", option, abilityContext_));
}

/**
 * @tc.name: GetAndVerifyWindowTypeForArkUI
 * @tc.desc: get and verify WindowType
 * @tc.type: FUNC
 */
HWTEST_F(WindowTest, GetAndVerifyWindowTypeForArkUI, TestSize.Level1)
{
    WindowType windowType;
    auto ret = Window::GetAndVerifyWindowTypeForArkUI(100, "GetAndVerifyWindowTypeForArkUITest",
        WindowType::WINDOW_TYPE_SCENE_BOARD, windowType);
    EXPECT_EQ(WMError::WM_OK, ret);
    EXPECT_EQ(windowType == WindowType::WINDOW_TYPE_SYSTEM_FLOAT, true);

    ret = Window::GetAndVerifyWindowTypeForArkUI(100, "GetAndVerifyWindowTypeForArkUITest",
        WindowType::WINDOW_TYPE_DESKTOP, windowType);
    EXPECT_EQ(WMError::WM_OK, ret);
    EXPECT_EQ(windowType == WindowType::WINDOW_TYPE_SYSTEM_FLOAT, true);

    ret = Window::GetAndVerifyWindowTypeForArkUI(100, "GetAndVerifyWindowTypeForArkUITest",
        WindowType::WINDOW_TYPE_UI_EXTENSION, windowType);
    EXPECT_EQ(WMError::WM_OK, ret);
    EXPECT_EQ(windowType == WindowType::WINDOW_TYPE_APP_SUB_WINDOW, true);

    ret = Window::GetAndVerifyWindowTypeForArkUI(100, "GetAndVerifyWindowTypeForArkUITest",
        WindowType::WINDOW_TYPE_SYSTEM_SUB_WINDOW, windowType);
    if (SceneBoardJudgement::IsSceneBoardEnabled()) {
        EXPECT_EQ(WMError::WM_ERROR_INVALID_TYPE, ret);
    } else {
        EXPECT_EQ(WMError::WM_OK, ret);
        EXPECT_EQ(windowType == WindowType::WINDOW_TYPE_SYSTEM_SUB_WINDOW, true);
    }
    
    ret = Window::GetAndVerifyWindowTypeForArkUI(100, "GetAndVerifyWindowTypeForArkUITest",
        WindowType::WINDOW_TYPE_FLOAT, windowType);
    EXPECT_EQ(WMError::WM_OK, ret);
    EXPECT_EQ(windowType == WindowType::WINDOW_TYPE_SYSTEM_SUB_WINDOW, true);

    ret = Window::GetAndVerifyWindowTypeForArkUI(100, "GetAndVerifyWindowTypeForArkUITest",
        WindowType::WINDOW_TYPE_APP_MAIN_WINDOW, windowType);
    if (SceneBoardJudgement::IsSceneBoardEnabled()) {
        EXPECT_EQ(WMError::WM_ERROR_INVALID_WINDOW, ret);
    } else {
        EXPECT_EQ(WMError::WM_OK, ret);
        EXPECT_EQ(windowType == WindowType::WINDOW_TYPE_APP_SUB_WINDOW, true);
    }
}

/**
 * @tc.name: CreatePiP
 * @tc.desc: Create PiP window with option
 * @tc.type: FUNC
 */
HWTEST_F(WindowTest, CreatePiP, TestSize.Level1)
{
    sptr<WindowOption> option = nullptr;
    PiPTemplateInfo pipTemplateInfo;
    ASSERT_EQ(nullptr, Window::CreatePiP(option, pipTemplateInfo, abilityContext_));
    option = sptr<WindowOption>::MakeSptr();
    ASSERT_EQ(nullptr, Window::CreatePiP(option, pipTemplateInfo, abilityContext_));
    option->SetWindowName("pip_window");
    ASSERT_EQ(nullptr, Window::CreatePiP(option, pipTemplateInfo, abilityContext_));
    option->SetWindowType(WindowType::WINDOW_TYPE_PIP);
    option->SetWindowMode(WindowMode::WINDOW_MODE_PIP);
    Rect rect = { 0, 0, 10, 10 };
    option->SetWindowRect(rect);
    WMError errCode;
    if (SceneBoardJudgement::IsSceneBoardEnabled()) {
        sptr<Window> window = Window::CreatePiP(option, pipTemplateInfo, abilityContext_, errCode);
        if (errCode == WMError::WM_OK) {
            ASSERT_NE(nullptr, window);
        } else {
            ASSERT_EQ(nullptr, window);
        }
    } else {
        ASSERT_EQ(nullptr, Window::CreatePiP(option, pipTemplateInfo, abilityContext_, errCode));
    }
}

/**
 * @tc.name: CreateFb
 * @tc.desc: Create FloatingBall window with option
 * @tc.type: FUNC
 */
HWTEST_F(WindowTest, CreateFb, TestSize.Level1)
{
    sptr<WindowOption> option = nullptr;
    FloatingBallTemplateBaseInfo fbTemplateBaseInfo;
    std::shared_ptr<Media::PixelMap> icon = nullptr;
    WMError errCode = WMError::WM_OK;
    ASSERT_EQ(nullptr, Window::CreateFb(option, fbTemplateBaseInfo, icon, abilityContext_, errCode));
    option = sptr<WindowOption>::MakeSptr();
    ASSERT_EQ(nullptr, Window::CreateFb(option, fbTemplateBaseInfo, icon, abilityContext_, errCode));
    option->SetWindowName("fb_window");
    ASSERT_EQ(nullptr, Window::CreateFb(option, fbTemplateBaseInfo, icon, abilityContext_, errCode));
    option->SetWindowType(WindowType::WINDOW_TYPE_FB);
    option->SetWindowMode(WindowMode::WINDOW_MODE_FB);
    Rect rect = {0, 0, 10, 10};
    option->SetWindowRect(rect);
    if (SceneBoardJudgement::IsSceneBoardEnabled()) {
        sptr<Window> window = Window::CreateFb(option, fbTemplateBaseInfo, icon, abilityContext_, errCode);
        if (errCode == WMError::WM_OK) {
            ASSERT_NE(nullptr, window);
        } else {
            ASSERT_EQ(nullptr, window);
        }
    } else {
        ASSERT_EQ(nullptr, Window::CreateFb(option, fbTemplateBaseInfo, icon, abilityContext_, errCode));
    }
}

/**
 * @tc.name: Find01
 * @tc.desc: Find with no name
 * @tc.type: FUNC
 */
HWTEST_F(WindowTest, Find01, TestSize.Level1)
{
    ASSERT_EQ(nullptr, Window::Find(""));
}

/**
 * @tc.name: Find02
 * @tc.desc: Find with name
 * @tc.type: FUNC
 */
HWTEST_F(WindowTest, Find02, TestSize.Level1)
{
    std::unique_ptr<Mocker> m = std::make_unique<Mocker>();
    sptr<WindowOption> option = sptr<WindowOption>::MakeSptr();

    auto window = Window::Create("WindowTest03", option);
    if (window != nullptr) {
        ASSERT_NE(nullptr, window);
    }
    if (Window::Find("WindowTest03") != nullptr) {
        ASSERT_NE(nullptr, Window::Find("WindowTest03"));
    }

    if (window != nullptr) {
        ASSERT_EQ(WMError::WM_OK, window->Destroy());
    }
}

/**
 * @tc.name: GetSurfaceNode
 * @tc.desc: get node
 * @tc.type: FUNC
 */
HWTEST_F(WindowTest, GetSurfaceNode, TestSize.Level1)
{
    sptr<Window> window = sptr<Window>::MakeSptr();
    ASSERT_EQ(nullptr, window->GetSurfaceNode());
    ASSERT_EQ(WMError::WM_OK, window->Destroy());
}

/**
 * @tc.name: GetContext
 * @tc.desc: get context
 * @tc.type: FUNC
 */
HWTEST_F(WindowTest, GetContext, TestSize.Level1)
{
    sptr<Window> window = sptr<Window>::MakeSptr();
    ASSERT_EQ(nullptr, window->GetContext());
    ASSERT_EQ(WMError::WM_OK, window->Destroy());
}

/**
 * @tc.name: GetTopWindowWithId
 * @tc.desc: get top window with id
 * @tc.type: FUNC
 */
HWTEST_F(WindowTest, GetTopWindowWithId, TestSize.Level1)
{
    sptr<Window> window = sptr<Window>::MakeSptr();
    std::unique_ptr<Mocker> m = std::make_unique<Mocker>();
    ASSERT_NE(nullptr, m);
    EXPECT_CALL(m->Mock(), GetTopWindowId(_, _)).Times(1).WillOnce(Return(WMError::WM_DO_NOTHING));
    uint32_t mainWinId = 0;
    ASSERT_EQ(nullptr, window->GetTopWindowWithId(mainWinId));

    sptr<WindowOption> option = sptr<WindowOption>::MakeSptr();
    sptr<WindowSessionImpl> windowSession = sptr<WindowSessionImpl>::MakeSptr(option);
    string winName = "test";
    int32_t winId = 1;
    WindowSessionImpl::windowSessionMap_.insert(
        std::make_pair(winName, pair<int32_t, sptr<WindowSessionImpl>>(winId, windowSession)));
    EXPECT_CALL(m->Mock(), GetTopWindowId(_, _))
        .Times(1)
        .WillOnce(DoAll(SetArgReferee<1>(winId), Return(WMError::WM_OK)));
    ASSERT_NE(nullptr, window->GetTopWindowWithId(mainWinId));

    int32_t tempWinId = 3;
    EXPECT_CALL(m->Mock(), GetTopWindowId(_, _))
        .Times(1)
        .WillOnce(DoAll(SetArgReferee<1>(tempWinId), Return(WMError::WM_OK)));
    ASSERT_EQ(nullptr, window->GetTopWindowWithId(mainWinId));
    ASSERT_EQ(WMError::WM_OK, window->Destroy());

    WindowSessionImpl::windowSessionMap_.erase(winName);
}

/**
 * @tc.name: GetRect
 * @tc.desc: get rect
 * @tc.type: FUNC
 */
HWTEST_F(WindowTest, GetRect, TestSize.Level1)
{
    sptr<Window> window = sptr<Window>::MakeSptr();
    ASSERT_EQ(Rect(), window->GetRect());
    ASSERT_EQ(WMError::WM_OK, window->Destroy());
}

/**
 * @tc.name: GetRequestRect
 * @tc.desc: get rect
 * @tc.type: FUNC
 */
HWTEST_F(WindowTest, GetRequestRect, TestSize.Level1)
{
    sptr<Window> window = sptr<Window>::MakeSptr();
    ASSERT_EQ(Rect(), window->GetRequestRect());
    ASSERT_EQ(WMError::WM_OK, window->Destroy());
}

/**
 * @tc.name: GetType
 * @tc.desc: get type
 * @tc.type: FUNC
 */
HWTEST_F(WindowTest, GetType, TestSize.Level1)
{
    sptr<Window> window = sptr<Window>::MakeSptr();
    ASSERT_EQ(WindowType::WINDOW_TYPE_APP_MAIN_WINDOW, window->GetType());
    ASSERT_EQ(WMError::WM_OK, window->Destroy());
}

/**
 * @tc.name: GetWindowMode
 * @tc.desc: get mode
 * @tc.type: FUNC
 */
HWTEST_F(WindowTest, GetWindowMode, TestSize.Level1)
{
    sptr<Window> window = sptr<Window>::MakeSptr();
    ASSERT_EQ(WindowMode::WINDOW_MODE_UNDEFINED, window->GetWindowMode());
    ASSERT_EQ(WMError::WM_OK, window->Destroy());

    sptr<Window> window2 = sptr<Window>::MakeSptr();
    ASSERT_NE(nullptr, window2);
    ASSERT_EQ(WindowMode::WINDOW_MODE_UNDEFINED, window2->GetWindowMode());
}

/**
 * @tc.name: GetAlpha
 * @tc.desc: get alpha
 * @tc.type: FUNC
 */
HWTEST_F(WindowTest, GetAlpha, TestSize.Level1)
{
    sptr<Window> window = sptr<Window>::MakeSptr();
    ASSERT_EQ(0.0f, window->GetAlpha());
    ASSERT_EQ(WMError::WM_OK, window->Destroy());
}

/**
 * @tc.name: GetFocusable
 * @tc.desc: get focusable
 * @tc.type: FUNC
 */
HWTEST_F(WindowTest, GetFocusable, TestSize.Level1)
{
    sptr<Window> window = sptr<Window>::MakeSptr();
    ASSERT_EQ(false, window->GetFocusable());
    ASSERT_EQ(WMError::WM_OK, window->Destroy());
}

/**
 * @tc.name: SetFocusable
 * @tc.desc: set Focusable
 * @tc.type: FUNC
 */
HWTEST_F(WindowTest, SetFocusable, TestSize.Level1)
{
    sptr<Window> window = sptr<Window>::MakeSptr();
    ASSERT_EQ(WMError::WM_OK, window->SetFocusable(true));
    ASSERT_EQ(WMError::WM_OK, window->Destroy());
}

/**
 * @tc.name: GetTouchable
 * @tc.desc: get Touchable
 * @tc.type: FUNC
 */
HWTEST_F(WindowTest, GetTouchable, TestSize.Level1)
{
    sptr<Window> window = sptr<Window>::MakeSptr();
    ASSERT_EQ(false, window->GetTouchable());
    ASSERT_EQ(WMError::WM_OK, window->Destroy());
}

/**
 * @tc.name: SetTouchable
 * @tc.desc: set Touchable
 * @tc.type: FUNC
 */
HWTEST_F(WindowTest, SetTouchable, TestSize.Level1)
{
    sptr<Window> window = sptr<Window>::MakeSptr();
    ASSERT_EQ(WMError::WM_OK, window->SetTouchable(true));
    ASSERT_EQ(WMError::WM_OK, window->Destroy());
}

/**
 * @tc.name: GetSystemBarPropertyByType
 * @tc.desc: get SystemBarPropertyByType
 * @tc.type: FUNC
 */
HWTEST_F(WindowTest, GetSystemBarPropertyByType, TestSize.Level1)
{
    sptr<Window> window = sptr<Window>::MakeSptr();
    ASSERT_EQ(SystemBarProperty(), window->GetSystemBarPropertyByType(WindowType::WINDOW_TYPE_APP_MAIN_WINDOW));
    ASSERT_EQ(WMError::WM_OK, window->Destroy());
}

/**
 * @tc.name: SetSystemBarProperty
 * @tc.desc: set SystemBarProperty
 * @tc.type: FUNC
 */
HWTEST_F(WindowTest, SetSystemBarProperty, TestSize.Level1)
{
    sptr<Window> window = sptr<Window>::MakeSptr();
    SystemBarProperty prop;
    auto ret = window->SetSystemBarProperty(WindowType::WINDOW_TYPE_APP_MAIN_WINDOW, prop);
    ASSERT_EQ(WMError::WM_OK, ret);
    ASSERT_EQ(WMError::WM_OK, window->Destroy());
}

/**
 * @tc.name: IsFullScreen
 * @tc.desc: get FullScreen
 * @tc.type: FUNC
 */
HWTEST_F(WindowTest, IsFullScreen, TestSize.Level1)
{
    sptr<Window> window = sptr<Window>::MakeSptr();
    ASSERT_EQ(false, window->IsFullScreen());
    ASSERT_EQ(WMError::WM_OK, window->Destroy());
}

/**
 * @tc.name: IsLayoutFullScreen
 * @tc.desc: get
 * @tc.type: FUNC
 */
HWTEST_F(WindowTest, IsLayoutFullScreen, TestSize.Level1)
{
    sptr<Window> window = sptr<Window>::MakeSptr();
    ASSERT_EQ(false, window->IsLayoutFullScreen());
    ASSERT_EQ(WMError::WM_OK, window->Destroy());
}

/**
 * @tc.name: SetAlpha
 * @tc.desc: set
 * @tc.type: FUNC
 */
HWTEST_F(WindowTest, SetAlpha, TestSize.Level1)
{
    sptr<Window> window = sptr<Window>::MakeSptr();
    ASSERT_EQ(WMError::WM_OK, window->SetAlpha(0.0f));
    ASSERT_EQ(WMError::WM_OK, window->Destroy());
}

/**
 * @tc.name: SetTransform
 * @tc.desc: set
 * @tc.type: FUNC
 */
HWTEST_F(WindowTest, SetTransform, TestSize.Level1)
{
    sptr<Window> window = sptr<Window>::MakeSptr();
    Transform trans;
    ASSERT_EQ(WMError::WM_OK, window->SetTransform(trans));
    ASSERT_EQ(WMError::WM_OK, window->Destroy());
}

/**
 * @tc.name: GetTransform
 * @tc.desc: get
 * @tc.type: FUNC
 */
HWTEST_F(WindowTest, GetTransform, TestSize.Level1)
{
    sptr<Window> window = sptr<Window>::MakeSptr();
    Transform trans;
    ASSERT_EQ(trans, window->GetTransform());
    ASSERT_EQ(WMError::WM_OK, window->Destroy());
}

/**
 * @tc.name: GetAvoidAreaByType
 * @tc.desc: get
 * @tc.type: FUNC
 */
HWTEST_F(WindowTest, GetAvoidAreaByType, TestSize.Level1)
{
    sptr<Window> window = sptr<Window>::MakeSptr();
    AvoidArea avoidArea;
    auto ret = window->GetAvoidAreaByType(AvoidAreaType::TYPE_CUTOUT, avoidArea);
    ASSERT_EQ(WMError::WM_OK, ret);
    ASSERT_EQ(WMError::WM_OK, window->Destroy());
}

/**
 * @tc.name: SetImmersiveModeEnabledState
 * @tc.desc: get
 * @tc.type: FUNC
 */
HWTEST_F(WindowTest, SetImmersiveModeEnabledState, TestSize.Level1)
{
    sptr<Window> window = sptr<Window>::MakeSptr();
    auto ret = window->SetImmersiveModeEnabledState(true);
    ASSERT_EQ(WMError::WM_OK, ret);
    ASSERT_EQ(WMError::WM_OK, window->Destroy());
}

/**
 * @tc.name: SetLayoutFullScreen
 * @tc.desc: get
 * @tc.type: FUNC
 */
HWTEST_F(WindowTest, SetLayoutFullScreen, TestSize.Level1)
{
    sptr<Window> window = sptr<Window>::MakeSptr();
    auto ret = window->SetLayoutFullScreen(true);
    ASSERT_EQ(WMError::WM_OK, ret);
    ASSERT_EQ(WMError::WM_OK, window->Destroy());
}

/**
 * @tc.name: SetIgnoreSafeArea
 * @tc.desc: SetIgnoreSafeArea
 * @tc.type: FUNC
 */
HWTEST_F(WindowTest, SetIgnoreSafeArea, TestSize.Level1)
{
    sptr<Window> window = sptr<Window>::MakeSptr();
    auto ret = window->SetIgnoreSafeArea(true);
    EXPECT_EQ(WMError::WM_OK, ret);
    ret = window->SetIgnoreSafeArea(false);
    EXPECT_EQ(WMError::WM_OK, ret);
    EXPECT_EQ(WMError::WM_OK, window->Destroy());
}

/**
 * @tc.name: SetTitleAndDockHoverShown
 * @tc.desc: get
 * @tc.type: FUNC
 */
HWTEST_F(WindowTest, SetTitleAndDockHoverShown, TestSize.Level1)
{
    sptr<Window> window = sptr<Window>::MakeSptr();
    auto ret = window->SetTitleAndDockHoverShown(true, true);
    EXPECT_EQ(WMError::WM_ERROR_DEVICE_NOT_SUPPORT, ret);
    EXPECT_EQ(WMError::WM_OK, window->Destroy());
}

/**
 * @tc.name: SetFullScreen
 * @tc.desc: get
 * @tc.type: FUNC
 */
HWTEST_F(WindowTest, SetFullScreen, TestSize.Level1)
{
    sptr<Window> window = sptr<Window>::MakeSptr();
    auto ret = window->SetFullScreen(true);
    ASSERT_EQ(WMError::WM_OK, ret);
    ASSERT_EQ(WMError::WM_OK, window->Destroy());
}

/**
 * @tc.name: Destroy
 * @tc.desc: get
 * @tc.type: FUNC
 */
HWTEST_F(WindowTest, Destroy, TestSize.Level1)
{
    sptr<Window> window = sptr<Window>::MakeSptr();
    auto ret = window->Destroy();
    ASSERT_EQ(WMError::WM_OK, ret);
    ASSERT_EQ(WMError::WM_OK, window->Destroy());
}

/**
 * @tc.name: Show
 * @tc.desc: get
 * @tc.type: FUNC
 */
HWTEST_F(WindowTest, Show, TestSize.Level1)
{
    sptr<Window> window = sptr<Window>::MakeSptr();
    auto ret = window->Show();
    ASSERT_EQ(WMError::WM_OK, ret);
    ASSERT_EQ(WMError::WM_OK, window->Destroy());
}

/**
 * @tc.name: Hide
 * @tc.desc: get
 * @tc.type: FUNC
 */
HWTEST_F(WindowTest, Hide, TestSize.Level1)
{
    sptr<Window> window = sptr<Window>::MakeSptr();
    auto ret = window->Hide();
    ASSERT_EQ(WMError::WM_OK, ret);
    ASSERT_EQ(WMError::WM_OK, window->Destroy());
}

/**
 * @tc.name: MoveTo
 * @tc.desc: get
 * @tc.type: FUNC
 */
HWTEST_F(WindowTest, MoveTo, TestSize.Level1)
{
    sptr<Window> window = sptr<Window>::MakeSptr();
    auto ret = window->MoveTo(0, 0);
    ASSERT_EQ(WMError::WM_OK, ret);
    ASSERT_EQ(WMError::WM_OK, window->Destroy());
}

/**
 * @tc.name: Resize
 * @tc.desc: get
 * @tc.type: FUNC
 */
HWTEST_F(WindowTest, Resize, TestSize.Level1)
{
    sptr<Window> window = sptr<Window>::MakeSptr();
    auto ret = window->Resize(0, 0);
    ASSERT_EQ(WMError::WM_OK, ret);
    ASSERT_EQ(WMError::WM_OK, window->Destroy());
}

/**
 * @tc.name: SetKeepScreenOn01
 * @tc.desc: SetKeepScreenOn true
 * @tc.type: FUNC
 */
HWTEST_F(WindowTest, SetKeepScreenOn01, TestSize.Level1)
{
    sptr<Window> window = sptr<Window>::MakeSptr();
    auto ret = window->SetKeepScreenOn(true);
    ASSERT_EQ(WMError::WM_OK, ret);
    ASSERT_EQ(WMError::WM_OK, window->Destroy());
}

/**
 * @tc.name: SetKeepScreenOn02
 * @tc.desc: SetKeepScreenOn false
 * @tc.type: FUNC
 */
HWTEST_F(WindowTest, SetKeepScreenOn02, TestSize.Level1)
{
    sptr<Window> window = sptr<Window>::MakeSptr();
    auto ret = window->SetKeepScreenOn(false);
    ASSERT_EQ(WMError::WM_OK, ret);
    ASSERT_EQ(WMError::WM_OK, window->Destroy());
}

/**
 * @tc.name: IsKeepScreenOn
 * @tc.desc: IsKeepScreenOn
 * @tc.type: FUNC
 */
HWTEST_F(WindowTest, IsKeepScreenOn, TestSize.Level1)
{
    sptr<Window> window = sptr<Window>::MakeSptr();
    auto ret = window->IsKeepScreenOn();
    ASSERT_EQ(false, ret);
    ASSERT_EQ(WMError::WM_OK, window->Destroy());
}

/**
 * @tc.name: SetTurnScreenOn
 * @tc.desc: get
 * @tc.type: FUNC
 */
HWTEST_F(WindowTest, SetTurnScreenOn, TestSize.Level1)
{
    sptr<Window> window = sptr<Window>::MakeSptr();
    auto ret = window->SetTurnScreenOn(true);
    ASSERT_EQ(WMError::WM_OK, ret);
    ASSERT_EQ(WMError::WM_OK, window->Destroy());
}

/**
 * @tc.name: IsTurnScreenOn
 * @tc.desc: get
 * @tc.type: FUNC
 */
HWTEST_F(WindowTest, IsTurnScreenOn, TestSize.Level1)
{
    sptr<Window> window = sptr<Window>::MakeSptr();
    auto ret = window->IsTurnScreenOn();
    ASSERT_EQ(false, ret);
    ASSERT_EQ(WMError::WM_OK, window->Destroy());
}

/**
 * @tc.name: SetBackgroundColor
 * @tc.desc: get
 * @tc.type: FUNC
 */
HWTEST_F(WindowTest, SetBackgroundColor, TestSize.Level1)
{
    sptr<Window> window = sptr<Window>::MakeSptr();
    auto ret = window->SetBackgroundColor("0x00000000");
    ASSERT_EQ(WMError::WM_OK, ret);
    ASSERT_EQ(WMError::WM_OK, window->Destroy());
}

/**
 * @tc.name: SetTransparent
 * @tc.desc: get
 * @tc.type: FUNC
 */
HWTEST_F(WindowTest, SetTransparent, TestSize.Level1)
{
    sptr<Window> window = sptr<Window>::MakeSptr();
    auto ret = window->SetTransparent(true);
    ASSERT_EQ(WMError::WM_OK, ret);
    ASSERT_EQ(WMError::WM_OK, window->Destroy());
}

/**
 * @tc.name: IsTransparent
 * @tc.desc: get
 * @tc.type: FUNC
 */
HWTEST_F(WindowTest, IsTransparent, TestSize.Level1)
{
    sptr<Window> window = sptr<Window>::MakeSptr();
    auto ret = window->IsTransparent();
    ASSERT_EQ(false, ret);
    ASSERT_EQ(WMError::WM_OK, window->Destroy());
}

/**
 * @tc.name: SetBrightness
 * @tc.desc: get
 * @tc.type: FUNC
 */
HWTEST_F(WindowTest, SetBrightness, TestSize.Level1)
{
    sptr<Window> window = sptr<Window>::MakeSptr();
    auto ret = window->SetBrightness(0.0f);
    ASSERT_EQ(WMError::WM_OK, ret);
    ASSERT_EQ(WMError::WM_OK, window->Destroy());
}

/**
 * @tc.name: GetBrightness
 * @tc.desc: get
 * @tc.type: FUNC
 */
HWTEST_F(WindowTest, GetBrightness, TestSize.Level1)
{
    sptr<Window> window = sptr<Window>::MakeSptr();
    auto ret = window->GetBrightness();
    ASSERT_EQ(0.0f, ret);
    ASSERT_EQ(WMError::WM_OK, window->Destroy());
}

/**
 * @tc.name: SetPrivacyMode
 * @tc.desc: get
 * @tc.type: FUNC
 */
HWTEST_F(WindowTest, SetPrivacyMode, TestSize.Level1)
{
    sptr<Window> window = sptr<Window>::MakeSptr();
    auto ret = window->SetPrivacyMode(0.0f);
    ASSERT_EQ(WMError::WM_OK, ret);
    ASSERT_EQ(WMError::WM_OK, window->Destroy());
}

/**
 * @tc.name: IsPrivacyMode
 * @tc.desc: get
 * @tc.type: FUNC
 */
HWTEST_F(WindowTest, IsPrivacyMode, TestSize.Level1)
{
    sptr<Window> window = sptr<Window>::MakeSptr();
    auto ret = window->IsPrivacyMode();
    ASSERT_EQ(false, ret);
    ASSERT_EQ(WMError::WM_OK, window->Destroy());
}

/**
 * @tc.name: SetSystemPrivacyMode
 * @tc.desc: get
 * @tc.type: FUNC
 */
HWTEST_F(WindowTest, SetSystemPrivacyMode, TestSize.Level1)
{
    g_logMsg.clear();
    LOG_SetCallback(MyLogCallback);
    sptr<Window> window = sptr<Window>::MakeSptr();
    window->SetSystemPrivacyMode(true);
    EXPECT_FALSE(g_logMsg.find("SetSystemPrivacyMode") != std::string::npos);
    LOG_SetCallback(nullptr);
    ASSERT_EQ(WMError::WM_OK, window->Destroy());
}

/**
 * @tc.name: BindDialogTarget
 * @tc.desc: get
 * @tc.type: FUNC
 */
HWTEST_F(WindowTest, BindDialogTarget, TestSize.Level1)
{
    sptr<Window> window = sptr<Window>::MakeSptr();
    sptr<IRemoteObject> targetToken;
    auto ret = window->BindDialogTarget(targetToken);
    ASSERT_EQ(WMError::WM_OK, ret);
    ASSERT_EQ(WMError::WM_OK, window->Destroy());
}

/**
 * @tc.name: RaiseToAppTop
 * @tc.desc: get
 * @tc.type: FUNC
 */
HWTEST_F(WindowTest, RaiseToAppTop, TestSize.Level1)
{
    sptr<Window> window = sptr<Window>::MakeSptr();
    auto ret = window->RaiseToAppTop();
    ASSERT_EQ(WMError::WM_OK, ret);
    ASSERT_EQ(WMError::WM_OK, window->Destroy());
}

/**
 * @tc.name: SetSnapshotSkip
 * @tc.desc: get
 * @tc.type: FUNC
 */
HWTEST_F(WindowTest, SetSnapshotSkip, TestSize.Level1)
{
    sptr<Window> window = sptr<Window>::MakeSptr();
    auto ret = window->SetSnapshotSkip(true);
    ASSERT_EQ(WMError::WM_OK, ret);
    ASSERT_EQ(WMError::WM_OK, window->Destroy());
}

/**
 * @tc.name: SetCornerRadius
 * @tc.desc: get
 * @tc.type: FUNC
 */
HWTEST_F(WindowTest, SetCornerRadius, TestSize.Level1)
{
    sptr<Window> window = sptr<Window>::MakeSptr();
    auto ret = window->SetCornerRadius(1.0f);
    ASSERT_EQ(WMError::WM_OK, ret);
    ASSERT_EQ(WMError::WM_OK, window->Destroy());
}

/**
 * @tc.name: SetShadowRadius
 * @tc.desc: get
 * @tc.type: FUNC
 */
HWTEST_F(WindowTest, SetShadowRadius, TestSize.Level1)
{
    sptr<Window> window = sptr<Window>::MakeSptr();
    auto ret = window->SetShadowRadius(1.0f);
    ASSERT_EQ(WMError::WM_OK, ret);
    ASSERT_EQ(WMError::WM_OK, window->Destroy());
}

/**
 * @tc.name: SetShadowColor
 * @tc.desc: get
 * @tc.type: FUNC
 */
HWTEST_F(WindowTest, SetShadowColor, TestSize.Level1)
{
    sptr<Window> window = sptr<Window>::MakeSptr();
    auto ret = window->SetShadowColor("0x00000000");
    ASSERT_EQ(WMError::WM_OK, ret);
    ASSERT_EQ(WMError::WM_OK, window->Destroy());
}

/**
 * @tc.name: SetShadowOffsetX
 * @tc.desc: get
 * @tc.type: FUNC
 */
HWTEST_F(WindowTest, SetShadowOffsetX, TestSize.Level1)
{
    sptr<Window> window = sptr<Window>::MakeSptr();
    auto ret = window->SetShadowOffsetX(0.0f);
    ASSERT_EQ(WMError::WM_OK, ret);
    ASSERT_EQ(WMError::WM_OK, window->Destroy());
}

/**
 * @tc.name: SetShadowOffsetY
 * @tc.desc: get
 * @tc.type: FUNC
 */
HWTEST_F(WindowTest, SetShadowOffsetY, TestSize.Level1)
{
    sptr<Window> window = sptr<Window>::MakeSptr();
    auto ret = window->SetShadowOffsetY(0.0f);
    ASSERT_EQ(WMError::WM_OK, ret);
    ASSERT_EQ(WMError::WM_OK, window->Destroy());
}

/**
 * @tc.name: SetBlur
 * @tc.desc: get
 * @tc.type: FUNC
 */
HWTEST_F(WindowTest, SetBlur, TestSize.Level1)
{
    sptr<Window> window = sptr<Window>::MakeSptr();
    auto ret = window->SetBlur(0.0f);
    ASSERT_EQ(WMError::WM_OK, ret);
    ASSERT_EQ(WMError::WM_OK, window->Destroy());
}

/**
 * @tc.name: SetBackdropBlur
 * @tc.desc: get
 * @tc.type: FUNC
 */
HWTEST_F(WindowTest, SetBackdropBlur, TestSize.Level1)
{
    sptr<Window> window = sptr<Window>::MakeSptr();
    auto ret = window->SetBackdropBlur(0.0f);
    ASSERT_EQ(WMError::WM_OK, ret);
    ASSERT_EQ(WMError::WM_OK, window->Destroy());
}

/**
 * @tc.name: SetBackdropBlurStyle
 * @tc.desc: get
 * @tc.type: FUNC
 */
HWTEST_F(WindowTest, SetBackdropBlurStyle, TestSize.Level1)
{
    sptr<Window> window = sptr<Window>::MakeSptr();
    auto ret = window->SetBackdropBlurStyle(WindowBlurStyle::WINDOW_BLUR_OFF);
    ASSERT_EQ(WMError::WM_OK, ret);
    ASSERT_EQ(WMError::WM_OK, window->Destroy());
}

/**
 * @tc.name: RequestFocus
 * @tc.desc: get
 * @tc.type: FUNC
 */
HWTEST_F(WindowTest, RequestFocus, TestSize.Level1)
{
    sptr<Window> window = sptr<Window>::MakeSptr();
    auto ret = window->RequestFocus();
    ASSERT_EQ(WMError::WM_OK, ret);
    ASSERT_EQ(WMError::WM_OK, window->Destroy());
}

/**
 * @tc.name: IsFocused
 * @tc.desc: get
 * @tc.type: FUNC
 */
HWTEST_F(WindowTest, IsFocused, TestSize.Level1)
{
    sptr<Window> window = sptr<Window>::MakeSptr();
    auto ret = window->IsFocused();
    ASSERT_EQ(false, ret);
    ASSERT_EQ(WMError::WM_OK, window->Destroy());
}

/**
 * @tc.name: UpdateSurfaceNodeAfterCustomAnimation
 * @tc.desc: get
 * @tc.type: FUNC
 */
HWTEST_F(WindowTest, UpdateSurfaceNodeAfterCustomAnimation, TestSize.Level1)
{
    sptr<Window> window = sptr<Window>::MakeSptr();
    auto ret = window->UpdateSurfaceNodeAfterCustomAnimation(false);
    ASSERT_EQ(WMError::WM_OK, ret);
    ASSERT_EQ(WMError::WM_OK, window->Destroy());
}

/**
 * @tc.name: SetInputEventConsumer
 * @tc.desc: get
 * @tc.type: FUNC
 */
HWTEST_F(WindowTest, SetInputEventConsumer, TestSize.Level1)
{
    g_logMsg.clear();
    LOG_SetCallback(MyLogCallback);
    sptr<Window> window = sptr<Window>::MakeSptr();
    std::shared_ptr<IInputEventConsumer> inputEventConsumer;
    window->SetInputEventConsumer(inputEventConsumer);
    EXPECT_FALSE(g_logMsg.find("in") != std::string::npos);
    LOG_SetCallback(nullptr);
    ASSERT_EQ(WMError::WM_OK, window->Destroy());
}

/**
 * @tc.name: ConsumeKeyEvent
 * @tc.desc: get
 * @tc.type: FUNC
 */
HWTEST_F(WindowTest, ConsumeKeyEvent, TestSize.Level1)
{
    g_logMsg.clear();
    LOG_SetCallback(MyLogCallback);
    sptr<Window> window = sptr<Window>::MakeSptr();
    std::shared_ptr<MMI::KeyEvent> inputEvent = nullptr;
    window->ConsumeKeyEvent(inputEvent);
    EXPECT_FALSE(g_logMsg.find("There is no key event consumer") != std::string::npos);
    LOG_SetCallback(nullptr);
    ASSERT_EQ(WMError::WM_OK, window->Destroy());
}

/**
 * @tc.name: PreNotifyKeyEvent
 * @tc.desc: get
 * @tc.type: FUNC
 */
HWTEST_F(WindowTest, PreNotifyKeyEvent, TestSize.Level1)
{
    sptr<Window> window = sptr<Window>::MakeSptr();
    std::shared_ptr<MMI::KeyEvent> inputEvent = nullptr;
    auto ret = window->PreNotifyKeyEvent(inputEvent);
    EXPECT_EQ(ret, false);
    ASSERT_EQ(WMError::WM_OK, window->Destroy());
}

/**
 * @tc.name: ConsumePointerEvent
 * @tc.desc: get
 * @tc.type: FUNC
 */
HWTEST_F(WindowTest, ConsumePointerEvent, TestSize.Level1)
{
    g_logMsg.clear();
    LOG_SetCallback(MyLogCallback);
    sptr<Window> window = sptr<Window>::MakeSptr();
    std::shared_ptr<MMI::PointerEvent> inputEvent = nullptr;
    window->ConsumePointerEvent(inputEvent);
    EXPECT_FALSE(g_logMsg.find("PointerEvent is nullptr") != std::string::npos);
    LOG_SetCallback(nullptr);
    ASSERT_EQ(WMError::WM_OK, window->Destroy());
}

/**
 * @tc.name: RequestVsync
 * @tc.desc: get
 * @tc.type: FUNC
 */
HWTEST_F(WindowTest, RequestVsync, TestSize.Level1)
{
    g_logMsg.clear();
    LOG_SetCallback(MyLogCallback);
    sptr<Window> window = sptr<Window>::MakeSptr();
    std::shared_ptr<VsyncCallback> vsyncCallback = nullptr;
    window->RequestVsync(vsyncCallback);
    // no return
    EXPECT_FALSE(g_logMsg.find("vsyncStation is null") != std::string::npos);
    LOG_SetCallback(nullptr);
    ASSERT_EQ(WMError::WM_OK, window->Destroy());
}

/**
 * @tc.name: UpdateConfiguration
 * @tc.desc: get
 * @tc.type: FUNC
 */
HWTEST_F(WindowTest, UpdateConfiguration, TestSize.Level1)
{
    g_logMsg.clear();
    LOG_SetCallback(MyLogCallback);
    sptr<Window> window = sptr<Window>::MakeSptr();
    std::shared_ptr<AppExecFwk::Configuration> conf = nullptr;
    window->UpdateConfiguration(conf);
    // no return
    EXPECT_FALSE(g_logMsg.find("uiContent null") != std::string::npos);
    LOG_SetCallback(nullptr);
    ASSERT_EQ(WMError::WM_OK, window->Destroy());
}

/**
 * @tc.name: RegisterLifeCycleListener
 * @tc.desc: get
 * @tc.type: FUNC
 */
HWTEST_F(WindowTest, RegisterLifeCycleListener, TestSize.Level1)
{
    sptr<Window> window = sptr<Window>::MakeSptr();
    sptr<IWindowLifeCycle> listener = nullptr;
    auto ret = window->RegisterLifeCycleListener(listener);
    ASSERT_EQ(WMError::WM_OK, ret);
    ASSERT_EQ(WMError::WM_OK, window->Destroy());
}

/**
 * @tc.name: UnregisterLifeCycleListener
 * @tc.desc: get
 * @tc.type: FUNC
 */
HWTEST_F(WindowTest, UnregisterLifeCycleListener, TestSize.Level1)
{
    sptr<Window> window = sptr<Window>::MakeSptr();
    sptr<IWindowLifeCycle> listener = nullptr;
    auto ret = window->UnregisterLifeCycleListener(listener);
    ASSERT_EQ(WMError::WM_OK, ret);
    ASSERT_EQ(WMError::WM_OK, window->Destroy());
}

/**
 * @tc.name: RegisterWindowChangeListener
 * @tc.desc: get
 * @tc.type: FUNC
 */
HWTEST_F(WindowTest, RegisterWindowChangeListener, TestSize.Level1)
{
    sptr<Window> window = sptr<Window>::MakeSptr();
    sptr<IWindowChangeListener> listener = nullptr;
    auto ret = window->RegisterWindowChangeListener(listener);
    ASSERT_EQ(WMError::WM_OK, ret);
    ASSERT_EQ(WMError::WM_OK, window->Destroy());
}

/**
 * @tc.name: UnregisterWindowChangeListener
 * @tc.desc: get
 * @tc.type: FUNC
 */
HWTEST_F(WindowTest, UnregisterWindowChangeListener, TestSize.Level1)
{
    sptr<Window> window = sptr<Window>::MakeSptr();
    sptr<IWindowChangeListener> listener = nullptr;
    auto ret = window->UnregisterWindowChangeListener(listener);
    ASSERT_EQ(WMError::WM_OK, ret);
    ASSERT_EQ(WMError::WM_OK, window->Destroy());
}

/**
 * @tc.name: RegisterAvoidAreaChangeListener
 * @tc.desc: get
 * @tc.type: FUNC
 */
HWTEST_F(WindowTest, RegisterAvoidAreaChangeListener, TestSize.Level1)
{
    sptr<Window> window = sptr<Window>::MakeSptr();
    sptr<IAvoidAreaChangedListener> listener = nullptr;
    auto ret = window->RegisterAvoidAreaChangeListener(listener);
    ASSERT_EQ(WMError::WM_OK, ret);
    ASSERT_EQ(WMError::WM_OK, window->Destroy());
}

/**
 * @tc.name: UnregisterAvoidAreaChangeListener
 * @tc.desc: get
 * @tc.type: FUNC
 */
HWTEST_F(WindowTest, UnregisterAvoidAreaChangeListener, TestSize.Level1)
{
    sptr<Window> window = sptr<Window>::MakeSptr();
    sptr<IAvoidAreaChangedListener> listener = nullptr;
    auto ret = window->UnregisterAvoidAreaChangeListener(listener);
    ASSERT_EQ(WMError::WM_OK, ret);
    ASSERT_EQ(WMError::WM_OK, window->Destroy());
}

/**
 * @tc.name: RegisterDragListener
 * @tc.desc: get
 * @tc.type: FUNC
 */
HWTEST_F(WindowTest, RegisterDragListener, TestSize.Level1)
{
    sptr<Window> window = sptr<Window>::MakeSptr();
    sptr<IWindowDragListener> listener = nullptr;
    auto ret = window->RegisterDragListener(listener);
    ASSERT_EQ(WMError::WM_OK, ret);
    ASSERT_EQ(WMError::WM_OK, window->Destroy());
}

/**
 * @tc.name: UnregisterDragListener
 * @tc.desc: get
 * @tc.type: FUNC
 */
HWTEST_F(WindowTest, UnregisterDragListener, TestSize.Level1)
{
    sptr<Window> window = sptr<Window>::MakeSptr();
    sptr<IWindowDragListener> listener = nullptr;
    auto ret = window->UnregisterDragListener(listener);
    ASSERT_EQ(WMError::WM_OK, ret);
    ASSERT_EQ(WMError::WM_OK, window->Destroy());
}

/**
 * @tc.name: RegisterDisplayMoveListener
 * @tc.desc: get
 * @tc.type: FUNC
 */
HWTEST_F(WindowTest, RegisterDisplayMoveListener, TestSize.Level1)
{
    sptr<Window> window = sptr<Window>::MakeSptr();
    sptr<IDisplayMoveListener> listener = nullptr;
    auto ret = window->RegisterDisplayMoveListener(listener);
    ASSERT_EQ(WMError::WM_OK, ret);
    ASSERT_EQ(WMError::WM_OK, window->Destroy());
}

/**
 * @tc.name: UnregisterDisplayMoveListener
 * @tc.desc: get
 * @tc.type: FUNC
 */
HWTEST_F(WindowTest, UnregisterDisplayMoveListener, TestSize.Level1)
{
    sptr<Window> window = sptr<Window>::MakeSptr();
    sptr<IDisplayMoveListener> listener = nullptr;
    auto ret = window->UnregisterDisplayMoveListener(listener);
    ASSERT_EQ(WMError::WM_OK, ret);
    ASSERT_EQ(WMError::WM_OK, window->Destroy());
}

/**
 * @tc.name: RegisterWindowDestroyedListener
 * @tc.desc: get
 * @tc.type: FUNC
 */
HWTEST_F(WindowTest, RegisterWindowDestroyedListener, TestSize.Level1)
{
    g_logMsg.clear();
    LOG_SetCallback(MyLogCallback);
    sptr<Window> window = sptr<Window>::MakeSptr();
    NotifyNativeWinDestroyFunc func = nullptr;
    window->RegisterWindowDestroyedListener(func);
    // no return
    EXPECT_FALSE(g_logMsg.find("Start register") != std::string::npos);
    LOG_SetCallback(nullptr);
    ASSERT_EQ(WMError::WM_OK, window->Destroy());
}

/**
 * @tc.name: RegisterOccupiedAreaChangeListener
 * @tc.desc: get
 * @tc.type: FUNC
 */
HWTEST_F(WindowTest, RegisterOccupiedAreaChangeListener, TestSize.Level1)
{
    sptr<Window> window = sptr<Window>::MakeSptr();
    sptr<IOccupiedAreaChangeListener> listener = nullptr;
    auto ret = window->RegisterOccupiedAreaChangeListener(listener);
    ASSERT_EQ(WMError::WM_OK, ret);
    ASSERT_EQ(WMError::WM_OK, window->Destroy());
}

/**
 * @tc.name: UnregisterOccupiedAreaChangeListener
 * @tc.desc: get
 * @tc.type: FUNC
 */
HWTEST_F(WindowTest, UnregisterOccupiedAreaChangeListener, TestSize.Level1)
{
    sptr<Window> window = sptr<Window>::MakeSptr();
    sptr<IOccupiedAreaChangeListener> listener = nullptr;
    auto ret = window->UnregisterOccupiedAreaChangeListener(listener);
    ASSERT_EQ(WMError::WM_OK, ret);
    ASSERT_EQ(WMError::WM_OK, window->Destroy());
}

/**
 * @tc.name: RegisterTouchOutsideListener
 * @tc.desc: get
 * @tc.type: FUNC
 */
HWTEST_F(WindowTest, RegisterTouchOutsideListener, TestSize.Level1)
{
    sptr<Window> window = sptr<Window>::MakeSptr();
    sptr<ITouchOutsideListener> listener = nullptr;
    auto ret = window->RegisterTouchOutsideListener(listener);
    ASSERT_EQ(WMError::WM_OK, ret);
    ASSERT_EQ(WMError::WM_OK, window->Destroy());
}

/**
 * @tc.name: UnregisterTouchOutsideListener
 * @tc.desc: get
 * @tc.type: FUNC
 */
HWTEST_F(WindowTest, UnregisterTouchOutsideListener, TestSize.Level1)
{
    sptr<Window> window = sptr<Window>::MakeSptr();
    sptr<ITouchOutsideListener> listener = nullptr;
    auto ret = window->UnregisterTouchOutsideListener(listener);
    ASSERT_EQ(WMError::WM_OK, ret);
    ASSERT_EQ(WMError::WM_OK, window->Destroy());
}

/**
 * @tc.name: RegisterAnimationTransitionController
 * @tc.desc: get
 * @tc.type: FUNC
 */
HWTEST_F(WindowTest, RegisterAnimationTransitionController, TestSize.Level1)
{
    sptr<Window> window = sptr<Window>::MakeSptr();
    sptr<IAnimationTransitionController> listener = nullptr;
    auto ret = window->RegisterAnimationTransitionController(listener);
    ASSERT_EQ(WMError::WM_OK, ret);
    ASSERT_EQ(WMError::WM_OK, window->Destroy());
}

/**
 * @tc.name: RegisterScreenshotListener
 * @tc.desc: get
 * @tc.type: FUNC
 */
HWTEST_F(WindowTest, RegisterScreenshotListener, TestSize.Level1)
{
    sptr<Window> window = sptr<Window>::MakeSptr();
    sptr<IScreenshotListener> listener = nullptr;
    auto ret = window->RegisterScreenshotListener(listener);
    ASSERT_EQ(WMError::WM_OK, ret);
    ASSERT_EQ(WMError::WM_OK, window->Destroy());
}

/**
 * @tc.name: UnregisterScreenshotListener
 * @tc.desc: get
 * @tc.type: FUNC
 */
HWTEST_F(WindowTest, UnregisterScreenshotListener, TestSize.Level1)
{
    sptr<Window> window = sptr<Window>::MakeSptr();
    sptr<IScreenshotListener> listener = nullptr;
    auto ret = window->UnregisterScreenshotListener(listener);
    ASSERT_EQ(WMError::WM_OK, ret);
    ASSERT_EQ(WMError::WM_OK, window->Destroy());
}

/**
 * @tc.name: RegisterScreenshotAppEventListener
 * @tc.desc: RegisterScreenshotAppEventListener
 * @tc.type: FUNC
 */
HWTEST_F(WindowTest, RegisterScreenshotAppEventListener, TestSize.Level1)
{
    sptr<Window> window = sptr<Window>::MakeSptr();
    sptr<IScreenshotAppEventListener> listener = nullptr;
    auto ret = window->RegisterScreenshotAppEventListener(listener);
    ASSERT_EQ(WMError::WM_ERROR_DEVICE_NOT_SUPPORT, ret);
    ASSERT_EQ(WMError::WM_OK, window->Destroy());
}

/**
 * @tc.name: UnregisterScreenshotAppEventListener
 * @tc.desc: UnregisterScreenshotAppEventListener
 * @tc.type: FUNC
 */
HWTEST_F(WindowTest, UnregisterScreenshotAppEventListener, TestSize.Level1)
{
    sptr<Window> window = sptr<Window>::MakeSptr();
    sptr<IScreenshotAppEventListener> listener = nullptr;
    auto ret = window->UnregisterScreenshotAppEventListener(listener);
    ASSERT_EQ(WMError::WM_ERROR_DEVICE_NOT_SUPPORT, ret);
    ASSERT_EQ(WMError::WM_OK, window->Destroy());
}

/**
 * @tc.name: RegisterDialogTargetTouchListener
 * @tc.desc: get
 * @tc.type: FUNC
 */
HWTEST_F(WindowTest, RegisterDialogTargetTouchListener, TestSize.Level1)
{
    sptr<Window> window = sptr<Window>::MakeSptr();
    sptr<IDialogTargetTouchListener> listener = nullptr;
    auto ret = window->RegisterDialogTargetTouchListener(listener);
    ASSERT_EQ(WMError::WM_OK, ret);
    ASSERT_EQ(WMError::WM_OK, window->Destroy());

    sptr<Window> window2 = sptr<Window>::MakeSptr();
    sptr<IDialogTargetTouchListener> listener2;
    auto ret2 = window2->RegisterDialogTargetTouchListener(listener2);
    ASSERT_EQ(WMError::WM_OK, ret2);
    ASSERT_EQ(WMError::WM_OK, window2->Destroy());
}

/**
 * @tc.name: UnregisterDialogTargetTouchListener
 * @tc.desc: get
 * @tc.type: FUNC
 */
HWTEST_F(WindowTest, UnregisterDialogTargetTouchListener, TestSize.Level1)
{
    sptr<Window> window = sptr<Window>::MakeSptr();
    sptr<IDialogTargetTouchListener> listener = nullptr;
    auto ret = window->UnregisterDialogTargetTouchListener(listener);
    ASSERT_EQ(WMError::WM_OK, ret);
    ASSERT_EQ(WMError::WM_OK, window->Destroy());

    sptr<Window> window2 = sptr<Window>::MakeSptr();
    ASSERT_EQ(WMError::WM_OK, window2->UnregisterDialogTargetTouchListener(listener));
    ASSERT_EQ(WMError::WM_OK, window2->Destroy());
}

/**
 * @tc.name: RegisterDialogDeathRecipientListener
 * @tc.desc: get
 * @tc.type: FUNC
 */
HWTEST_F(WindowTest, RegisterDialogDeathRecipientListener, TestSize.Level1)
{
    g_logMsg.clear();
    LOG_SetCallback(MyLogCallback);
    sptr<Window> window = sptr<Window>::MakeSptr();
    sptr<IDialogDeathRecipientListener> listener = nullptr;
    window->RegisterDialogDeathRecipientListener(listener);
    EXPECT_FALSE(g_logMsg.find("listener is null") != std::string::npos);
    LOG_SetCallback(nullptr);
    ASSERT_EQ(WMError::WM_OK, window->Destroy());
}

/**
 * @tc.name: UnregisterDialogDeathRecipientListener
 * @tc.desc: get
 * @tc.type: FUNC
 */
HWTEST_F(WindowTest, UnregisterDialogDeathRecipientListener, TestSize.Level1)
{
    g_logMsg.clear();
    LOG_SetCallback(MyLogCallback);
    sptr<Window> window = sptr<Window>::MakeSptr();
    sptr<IDialogDeathRecipientListener> listener = nullptr;
    window->UnregisterDialogDeathRecipientListener(listener);
    EXPECT_FALSE(g_logMsg.find("Start unregister") != std::string::npos);
    LOG_SetCallback(nullptr);
    ASSERT_EQ(WMError::WM_OK, window->Destroy());
}

/**
 * @tc.name: NotifyTouchDialogTarget
 * @tc.desc: get
 * @tc.type: FUNC
 */
HWTEST_F(WindowTest, NotifyTouchDialogTarget, TestSize.Level1)
{
    sptr<Window> window = sptr<Window>::MakeSptr();
    sptr<IDialogTargetTouchListener> listener = nullptr;
    window->NotifyTouchDialogTarget();
    ASSERT_EQ(WMError::WM_OK, window->Destroy());
}

/**
 * @tc.name: SetAceAbilityHandler
 * @tc.desc: get
 * @tc.type: FUNC
 */
HWTEST_F(WindowTest, SetAceAbilityHandler, TestSize.Level1)
{
    g_logMsg.clear();
    LOG_SetCallback(MyLogCallback);
    sptr<Window> window = sptr<Window>::MakeSptr();
    sptr<IAceAbilityHandler> handler = nullptr;
    window->SetAceAbilityHandler(handler);
    EXPECT_FALSE(g_logMsg.find("ace ability handler is nullptr") != std::string::npos);
    LOG_SetCallback(nullptr);
    ASSERT_EQ(WMError::WM_OK, window->Destroy());
}

/**
 * @tc.name: NapiSetUIContent
 * @tc.desc: get
 * @tc.type: FUNC
 */
HWTEST_F(WindowTest, NapiSetUIContent, TestSize.Level1)
{
    sptr<Window> window = sptr<Window>::MakeSptr();
    napi_env env = nullptr;
    napi_value storage = nullptr;
    auto ret = window->NapiSetUIContent("info", env, storage);
    ASSERT_EQ(WMError::WM_OK, ret);
    ASSERT_EQ(WMError::WM_OK, window->Destroy());
}

/**
 * @tc.name: NapiSetUIContentByName
 * @tc.desc: get
 * @tc.type: FUNC
 */
HWTEST_F(WindowTest, NapiSetUIContentByName, TestSize.Level1)
{
    auto window = sptr<Window>::MakeSptr();
    napi_env env = nullptr;
    napi_value storage = nullptr;
    auto ret = window->NapiSetUIContentByName("info", env, storage);
    ASSERT_EQ(WMError::WM_OK, ret);
    ASSERT_EQ(WMError::WM_OK, window->Destroy());
}

/**
 * @tc.name: SetUIContentByAbc
 * @tc.desc: get
 * @tc.type: FUNC
 */
HWTEST_F(WindowTest, SetUIContentByAbc, TestSize.Level1)
{
    sptr<Window> window = sptr<Window>::MakeSptr();
    napi_env env = nullptr;
    napi_value storage = nullptr;
    auto ret = window->SetUIContentByAbc("/system/etc/window/resources/test.abc", env, storage);
    ASSERT_EQ(WMError::WM_OK, ret);
    ASSERT_EQ(WMError::WM_OK, window->Destroy());
}

/**
 * @tc.name: GetContentInfo
 * @tc.desc: get
 * @tc.type: FUNC
 */
HWTEST_F(WindowTest, GetContentInfo, TestSize.Level1)
{
    sptr<Window> window = sptr<Window>::MakeSptr();
    auto ret = window->GetContentInfo();
    ASSERT_EQ(std::string(), ret);
    ASSERT_EQ(WMError::WM_OK, window->Destroy());
}

/**
 * @tc.name: GetUIContent
 * @tc.desc: get
 * @tc.type: FUNC
 */
HWTEST_F(WindowTest, GetUIContent, TestSize.Level1)
{
    sptr<Window> window = sptr<Window>::MakeSptr();
    auto ret = window->GetUIContent();
    ASSERT_EQ(nullptr, ret);
    ASSERT_EQ(WMError::WM_OK, window->Destroy());
}

/**
 * @tc.name: OnNewWant
 * @tc.desc: get
 * @tc.type: FUNC
 */
HWTEST_F(WindowTest, OnNewWant, TestSize.Level1)
{
    g_logMsg.clear();
    LOG_SetCallback(MyLogCallback);
    sptr<Window> window = sptr<Window>::MakeSptr();
    AAFwk::Want want;
    window->OnNewWant(want);
    EXPECT_FALSE(g_logMsg.find("window") != std::string::npos);
    LOG_SetCallback(nullptr);
    ASSERT_EQ(WMError::WM_OK, window->Destroy());
}

/**
 * @tc.name: SetRequestedOrientation
 * @tc.desc: get
 * @tc.type: FUNC
 */
HWTEST_F(WindowTest, SetRequestedOrientation, TestSize.Level1)
{
    g_logMsg.clear();
    LOG_SetCallback(MyLogCallback);
    sptr<Window> window = sptr<Window>::MakeSptr();
    Orientation ori = Orientation::UNSPECIFIED;
    window->SetRequestedOrientation(ori);
    EXPECT_FALSE(g_logMsg.find("window is invalid") != std::string::npos);
    LOG_SetCallback(nullptr);
    ASSERT_EQ(WMError::WM_OK, window->Destroy());
}

/**
 * @tc.name: GetRequestedOrientation
 * @tc.desc: get
 * @tc.type: FUNC
 */
HWTEST_F(WindowTest, GetRequestedOrientation, TestSize.Level1)
{
    sptr<Window> window = sptr<Window>::MakeSptr();
    auto ret = window->GetRequestedOrientation();
    ASSERT_EQ(Orientation::UNSPECIFIED, ret);
    ASSERT_EQ(WMError::WM_OK, window->Destroy());
}

/**
 * @tc.name: SetRequestWindowModeSupportType
 * @tc.desc: get
 * @tc.type: FUNC
 */
HWTEST_F(WindowTest, SetRequestWindowModeSupportType, TestSize.Level1)
{
    sptr<Window> window = sptr<Window>::MakeSptr();
    uint32_t windowModeSupportType = 0;
    window->SetRequestWindowModeSupportType(windowModeSupportType);
    ASSERT_EQ(static_cast<uint32_t>(Orientation::UNSPECIFIED), windowModeSupportType);
    ASSERT_EQ(WMError::WM_OK, window->Destroy());
}

/**
 * @tc.name: GetRequestWindowModeSupportType
 * @tc.desc: get
 * @tc.type: FUNC
 */
HWTEST_F(WindowTest, GetRequestWindowModeSupportType, TestSize.Level1)
{
    sptr<Window> window = sptr<Window>::MakeSptr();
    uint32_t ret = window->GetRequestWindowModeSupportType();
    ASSERT_EQ(true, ret == 0);
    ASSERT_EQ(WMError::WM_OK, window->Destroy());
}

/**
 * @tc.name: SetTouchHotAreas
 * @tc.desc: get
 * @tc.type: FUNC
 */
HWTEST_F(WindowTest, SetTouchHotAreas, TestSize.Level1)
{
    sptr<Window> window = sptr<Window>::MakeSptr();
    std::vector<Rect> rects;
    auto ret = window->SetTouchHotAreas(rects);
    ASSERT_EQ(WMError::WM_OK, ret);
    ASSERT_EQ(WMError::WM_OK, window->Destroy());
}

/**
 * @tc.name: GetRequestedTouchHotAreas
 * @tc.desc: get
 * @tc.type: FUNC
 */
HWTEST_F(WindowTest, GetRequestedTouchHotAreas, TestSize.Level1)
{
    sptr<Window> window = sptr<Window>::MakeSptr();
    std::vector<Rect> rects;
    window->GetRequestedTouchHotAreas(rects);
    ASSERT_EQ(WMError::WM_OK, window->Destroy());
}

/**
 * @tc.name: IsMainHandlerAvailable
 * @tc.desc: get
 * @tc.type: FUNC
 */
HWTEST_F(WindowTest, IsMainHandlerAvailable, TestSize.Level1)
{
    sptr<Window> window = sptr<Window>::MakeSptr();
    sptr<WindowOption> option = sptr<WindowOption>::MakeSptr();
    option->SetMainHandlerAvailable(false);
    auto ret = window->IsMainHandlerAvailable();
    ASSERT_EQ(false, ret);
    ASSERT_EQ(WMError::WM_OK, window->Destroy());
}

/**
 * @tc.name: SetAPPWindowLabel
 * @tc.desc: get
 * @tc.type: FUNC
 */
HWTEST_F(WindowTest, SetAPPWindowLabel, TestSize.Level1)
{
    sptr<Window> window = sptr<Window>::MakeSptr();
    auto ret = window->SetAPPWindowLabel("");
    ASSERT_EQ(WMError::WM_OK, ret);
    ASSERT_EQ(WMError::WM_OK, window->Destroy());

    sptr<Window> window2 = sptr<Window>::MakeSptr();
    ASSERT_EQ(WMError::WM_OK, window2->SetAPPWindowLabel("000111"));
    ASSERT_EQ(WMError::WM_OK, window2->Destroy());
}

/**
 * @tc.name: IsDecorEnable
 * @tc.desc: get
 * @tc.type: FUNC
 */
HWTEST_F(WindowTest, IsDecorEnable, TestSize.Level1)
{
    sptr<Window> window = sptr<Window>::MakeSptr();
    auto ret = window->IsDecorEnable();
    ASSERT_EQ(false, ret);
    ASSERT_EQ(WMError::WM_OK, window->Destroy());
}

/**
 * @tc.name: Maximize
 * @tc.desc: get
 * @tc.type: FUNC
 */
HWTEST_F(WindowTest, Maximize, TestSize.Level1)
{
    sptr<Window> window = sptr<Window>::MakeSptr();
    auto ret = window->Maximize();
    ASSERT_EQ(WMError::WM_OK, ret);
    ASSERT_EQ(WMError::WM_OK, window->Destroy());
}

/**
 * @tc.name: MaximizeFloating
 * @tc.desc: get
 * @tc.type: FUNC
 */
HWTEST_F(WindowTest, MaximizeFloating, TestSize.Level1)
{
    sptr<Window> window = sptr<Window>::MakeSptr();
    auto ret = window->MaximizeFloating();
    ASSERT_EQ(WMError::WM_OK, ret);
    ASSERT_EQ(WMError::WM_OK, window->Destroy());
}

/**
 * @tc.name: Minimize
 * @tc.desc: get
 * @tc.type: FUNC
 */
HWTEST_F(WindowTest, Minimize, TestSize.Level1)
{
    sptr<Window> window = sptr<Window>::MakeSptr();
    auto ret = window->Minimize();
    ASSERT_EQ(WMError::WM_ERROR_DEVICE_NOT_SUPPORT, ret);
    ASSERT_EQ(WMError::WM_OK, window->Destroy());
}

/**
 * @tc.name: Recover
 * @tc.desc: get
 * @tc.type: FUNC
 */
HWTEST_F(WindowTest, Recover, TestSize.Level1)
{
    sptr<Window> window = sptr<Window>::MakeSptr();
    auto ret = window->Recover();

    if (SceneBoardJudgement::IsSceneBoardEnabled()) {
        ASSERT_NE(WMError::WM_ERROR_DEVICE_NOT_SUPPORT, ret);
    } else {
        ASSERT_EQ(WMError::WM_OK, ret);
    }
    ASSERT_EQ(WMError::WM_OK, window->Destroy());
}

/**
 * @tc.name: Close
 * @tc.desc: Close
 * @tc.type: FUNC
 */
HWTEST_F(WindowTest, Close, TestSize.Level1)
{
    sptr<Window> window = sptr<Window>::MakeSptr();
    auto ret = window->Close();
    ASSERT_EQ(true, ret == WMError::WM_OK);
    ASSERT_EQ(WMError::WM_OK, window->Destroy());
}

/**
 * @tc.name: CloseDirectly
 * @tc.desc: CloseDirectly
 * @tc.type: FUNC
 */
HWTEST_F(WindowTest, CloseDirectly, TestSize.Level1)
{
    sptr<Window> window = sptr<Window>::MakeSptr();
    auto ret = window->CloseDirectly();
    ASSERT_EQ(ret, WMError::WM_ERROR_DEVICE_NOT_SUPPORT);
    ASSERT_EQ(WMError::WM_OK, window->Destroy());
}

/**
 * @tc.name: StartMove
 * @tc.desc: get
 * @tc.type: FUNC
 */
HWTEST_F(WindowTest, StartMove, TestSize.Level1)
{
    g_logMsg.clear();
    LOG_SetCallback(MyLogCallback);
    sptr<Window> window = sptr<Window>::MakeSptr();
    window->StartMove();
    EXPECT_FALSE(g_logMsg.find("current window can not be moved") != std::string::npos);
    LOG_SetCallback(nullptr);
    ASSERT_EQ(WMError::WM_OK, window->Destroy());
}

/**
 * @tc.name: SetGlobalMaximizeMode
 * @tc.desc: get
 * @tc.type: FUNC
 */
HWTEST_F(WindowTest, SetGlobalMaximizeMode, TestSize.Level1)
{
    sptr<Window> window = sptr<Window>::MakeSptr();
    auto ret = window->SetGlobalMaximizeMode(MaximizeMode::MODE_AVOID_SYSTEM_BAR);
    ASSERT_EQ(WMError::WM_OK, ret);
    ASSERT_EQ(WMError::WM_OK, window->Destroy());
}

/**
 * @tc.name: GetGlobalMaximizeMode
 * @tc.desc: get
 * @tc.type: FUNC
 */
HWTEST_F(WindowTest, GetGlobalMaximizeMode, TestSize.Level1)
{
    sptr<Window> window = sptr<Window>::MakeSptr();

    auto ret = window->GetGlobalMaximizeMode();
    ASSERT_EQ(MaximizeMode::MODE_FULL_FILL, ret);
    ASSERT_EQ(WMError::WM_OK, window->Destroy());
}

/**
 * @tc.name: IsSupportWideGamut
 * @tc.desc: get
 * @tc.type: FUNC
 */
HWTEST_F(WindowTest, IsSupportWideGamut, TestSize.Level1)
{
    sptr<Window> window = sptr<Window>::MakeSptr();
    auto ret = window->IsSupportWideGamut();
    ASSERT_EQ(false, ret);
    ASSERT_EQ(WMError::WM_OK, window->Destroy());
}

/**
 * @tc.name: SetColorSpace
 * @tc.desc: get
 * @tc.type: FUNC
 */
HWTEST_F(WindowTest, SetColorSpace, TestSize.Level1)
{
    g_logMsg.clear();
    LOG_SetCallback(MyLogCallback);
    sptr<Window> window = sptr<Window>::MakeSptr();
    window->SetColorSpace(ColorSpace::COLOR_SPACE_DEFAULT);
    EXPECT_FALSE(g_logMsg.find("surface node is nullptr") != std::string::npos);
    LOG_SetCallback(nullptr);
    ASSERT_EQ(WMError::WM_OK, window->Destroy());
}

/**
 * @tc.name: GetColorSpace
 * @tc.desc: get
 * @tc.type: FUNC
 */
HWTEST_F(WindowTest, GetColorSpace, TestSize.Level1)
{
    sptr<Window> window = sptr<Window>::MakeSptr();
    auto ret = window->GetColorSpace();
    ASSERT_EQ(ColorSpace::COLOR_SPACE_DEFAULT, ret);
    ASSERT_EQ(WMError::WM_OK, window->Destroy());
}

/**
 * @tc.name: DumpInfo
 * @tc.desc: get
 * @tc.type: FUNC
 */
HWTEST_F(WindowTest, DumpInfo, TestSize.Level1)
{
    g_logMsg.clear();
    LOG_SetCallback(MyLogCallback);
    sptr<Window> window = sptr<Window>::MakeSptr();
    std::vector<std::string> params;
    std::vector<std::string> info;
    window->DumpInfo(params, info);
    EXPECT_FALSE(g_logMsg.find("Dump ArkUI help Info") != std::string::npos);
    LOG_SetCallback(nullptr);
    ASSERT_EQ(WMError::WM_OK, window->Destroy());
}

/**
 * @tc.name: Snapshot
 * @tc.desc: get
 * @tc.type: FUNC
 */
HWTEST_F(WindowTest, Snapshot, TestSize.Level1)
{
    sptr<Window> window = sptr<Window>::MakeSptr();
    auto pixmap = window->Snapshot();
    ASSERT_EQ(pixmap, nullptr);
    ASSERT_EQ(WMError::WM_OK, window->Destroy());
}

/**
 * @tc.name: NotifyMemoryLevel
 * @tc.desc: get
 * @tc.type: FUNC
 */
HWTEST_F(WindowTest, NotifyMemoryLevel, TestSize.Level1)
{
    sptr<Window> window = sptr<Window>::MakeSptr();
    auto ret = window->NotifyMemoryLevel(0);
    ASSERT_EQ(WMError::WM_OK, ret);
    ASSERT_EQ(WMError::WM_OK, window->Destroy());

    sptr<Window> window2 = sptr<Window>::MakeSptr();
    ASSERT_EQ(WMError::WM_OK, window2->NotifyMemoryLevel(22));
    ASSERT_EQ(WMError::WM_OK, window2->Destroy());
}

/**
 * @tc.name: IsAllowHaveSystemSubWindow
 * @tc.desc: get
 * @tc.type: FUNC
 */
HWTEST_F(WindowTest, IsAllowHaveSystemSubWindow, TestSize.Level1)
{
    sptr<Window> window = sptr<Window>::MakeSptr();
    window->SetWindowType(WindowType::WINDOW_TYPE_APP_MAIN_WINDOW);
    auto ret = window->IsAllowHaveSystemSubWindow();
    ASSERT_EQ(false, ret);
    ASSERT_EQ(WMError::WM_OK, window->Destroy());
}

/**
 * @tc.name: SetAspectRatio
 * @tc.desc: get
 * @tc.type: FUNC
 */
HWTEST_F(WindowTest, SetAspectRatio, TestSize.Level1)
{
    sptr<Window> window = sptr<Window>::MakeSptr();
    auto ret = window->SetAspectRatio(0.0f);
    ASSERT_EQ(WMError::WM_OK, ret);
    ASSERT_EQ(WMError::WM_OK, window->Destroy());

    sptr<Window> window2 = sptr<Window>::MakeSptr();
    ASSERT_EQ(WMError::WM_OK, window2->SetAspectRatio(0.1f));
    ASSERT_EQ(WMError::WM_OK, window2->Destroy());
}

/**
 * @tc.name: ResetAspectRatio
 * @tc.desc: get
 * @tc.type: FUNC
 */
HWTEST_F(WindowTest, ResetAspectRatio, TestSize.Level1)
{
    sptr<Window> window = sptr<Window>::MakeSptr();
    auto ret = window->ResetAspectRatio();
    ASSERT_EQ(WMError::WM_OK, ret);
    ASSERT_EQ(WMError::WM_OK, window->Destroy());
}

/**
 * @tc.name: GetKeyboardAnimationConfig
 * @tc.desc: get
 * @tc.type: FUNC
 */
HWTEST_F(WindowTest, GetKeyboardAnimationConfig, TestSize.Level1)
{
    sptr<Window> window = sptr<Window>::MakeSptr();
    KeyboardAnimationCurve curve;
    auto ret = window->GetKeyboardAnimationConfig();
    ASSERT_EQ(true, ret.curveIn.duration_ == curve.duration_);
    ASSERT_EQ(WMError::WM_OK, window->Destroy());
}

/**
 * @tc.name: SetNeedDefaultAnimation
 * @tc.desc: get
 * @tc.type: FUNC
 */
HWTEST_F(WindowTest, SetNeedDefaultAnimation, TestSize.Level1)
{
    sptr<Window> window = sptr<Window>::MakeSptr();
    window->SetNeedDefaultAnimation(true);
    ASSERT_EQ(WMError::WM_OK, window->Destroy());
}

/**
 * @tc.name: TransferAbilityResult
 * @tc.desc: get
 * @tc.type: FUNC
 */
HWTEST_F(WindowTest, TransferAbilityResult, TestSize.Level1)
{
    sptr<Window> window = sptr<Window>::MakeSptr();
    AAFwk::Want want;
    auto ret = window->TransferAbilityResult(0, want);
    ASSERT_EQ(WMError::WM_OK, ret);
    ASSERT_EQ(WMError::WM_OK, window->Destroy());
}

/**
 * @tc.name: TransferExtensionData
 * @tc.desc: get
 * @tc.type: FUNC
 */
HWTEST_F(WindowTest, TransferExtensionData, TestSize.Level1)
{
    sptr<Window> window = sptr<Window>::MakeSptr();
    AAFwk::WantParams wantParams;
    auto ret = window->TransferExtensionData(wantParams);
    ASSERT_EQ(WMError::WM_OK, ret);
    ASSERT_EQ(WMError::WM_OK, window->Destroy());
}

/**
 * @tc.name: RegisterTransferComponentDataListener
 * @tc.desc: get
 * @tc.type: FUNC
 */
HWTEST_F(WindowTest, RegisterTransferComponentDataListener, TestSize.Level1)
{
    g_logMsg.clear();
    LOG_SetCallback(MyLogCallback);
    sptr<Window> window = sptr<Window>::MakeSptr();
    NotifyTransferComponentDataFunc func;
    window->RegisterTransferComponentDataListener(func);
    EXPECT_FALSE(g_logMsg.find("Window invalid.") != std::string::npos);
    LOG_SetCallback(nullptr);
    ASSERT_EQ(WMError::WM_OK, window->Destroy());
}

/**
 * @tc.name: WindowChangeListener
 * @tc.desc: WindowChangeListener01 fun
 * @tc.type: FUNC
 */
HWTEST_F(WindowTest, WindowChangeListener01, TestSize.Level1)
{
    sptr<Window> window = sptr<Window>::MakeSptr();
    sptr<IWindowChangeListener> listener = sptr<IWindowChangeListener>::MakeSptr();
    auto ret = window->RegisterWindowChangeListener(listener);
    EXPECT_EQ(WMError::WM_OK, ret);
    listener->OnModeChange(WindowMode::WINDOW_MODE_UNDEFINED, false);
    auto ret1 = window->UnregisterWindowChangeListener(listener);
    EXPECT_EQ(WMError::WM_OK, ret1);
    ASSERT_EQ(WMError::WM_OK, window->Destroy());
}

/**
 * @tc.name: IOccupiedAreaChangeListener
 * @tc.desc: IOccupiedAreaChangeListener fun
 * @tc.type: FUNC
 */
HWTEST_F(WindowTest, IOccupiedAreaChangeListener, TestSize.Level1)
{
    sptr<Window> window = sptr<Window>::MakeSptr();

    sptr<IOccupiedAreaChangeListener> listener = sptr<IOccupiedAreaChangeListener>::MakeSptr();
    Rect rect = { 0, 0, 0, 0 };
    auto ret = window->RegisterOccupiedAreaChangeListener(listener);
    EXPECT_EQ(WMError::WM_OK, ret);
    sptr<OccupiedAreaChangeInfo> info = sptr<OccupiedAreaChangeInfo>::MakeSptr(OccupiedAreaType::TYPE_INPUT, rect, 80);
    listener->OnSizeChange(info, nullptr);
    auto ret1 = window->UnregisterOccupiedAreaChangeListener(listener);
    EXPECT_EQ(WMError::WM_OK, ret1);
    ASSERT_EQ(WMError::WM_OK, window->Destroy());
}

/**
 * @tc.name: WindowChangeListener
 * @tc.desc: WindowChangeListener02 fun
 * @tc.type: FUNC
 */
HWTEST_F(WindowTest, WindowChangeListener02, TestSize.Level1)
{
    sptr<Window> window = sptr<Window>::MakeSptr();
    sptr<IWindowChangeListener> listener = sptr<IWindowChangeListener>::MakeSptr();
    auto ret = window->RegisterWindowChangeListener(listener);
    EXPECT_EQ(WMError::WM_OK, ret);
    Rect rect = { 0, 0, 0, 0 };
    std::shared_ptr<RSTransaction> rstransaction;
    listener->OnSizeChange(rect, WindowSizeChangeReason::UNDEFINED, rstransaction);
    auto ret1 = window->UnregisterWindowChangeListener(listener);
    EXPECT_EQ(WMError::WM_OK, ret1);
    ASSERT_EQ(WMError::WM_OK, window->Destroy());
}

/**
 * @tc.name: IAnimationTransitionController
 * @tc.desc: IAnimationTransitionController fun
 * @tc.type: FUNC
 */
HWTEST_F(WindowTest, IAnimationTransitionController, TestSize.Level1)
{
    sptr<Window> window = sptr<Window>::MakeSptr();
    sptr<IAnimationTransitionController> listener = sptr<IAnimationTransitionController>::MakeSptr();
    auto ret = window->RegisterAnimationTransitionController(listener);
    EXPECT_EQ(WMError::WM_OK, ret);
    listener->AnimationForShown();
    listener->AnimationForHidden();
    ASSERT_EQ(WMError::WM_OK, window->Destroy());
}

/**
 * @tc.name: IInputEventConsumer
 * @tc.desc: IInputEventConsumer fun
 * @tc.type: FUNC
 */
HWTEST_F(WindowTest, IInputEventConsumer, TestSize.Level1)
{
    sptr<Window> window = sptr<Window>::MakeSptr();
    std::shared_ptr<IInputEventConsumer> listener = std::make_shared<IInputEventConsumer>();
    std::shared_ptr<MMI::KeyEvent> keyEvent = nullptr;
    std::shared_ptr<MMI::PointerEvent> pointerEvent = nullptr;
    std::shared_ptr<MMI::AxisEvent> axisEvent = nullptr;
    listener->OnInputEvent(keyEvent);
    listener->OnInputEvent(pointerEvent);
    listener->OnInputEvent(axisEvent);
    ASSERT_EQ(WMError::WM_OK, window->Destroy());
}

/**
 * @tc.name: IAceAbilityHandler
 * @tc.desc: IAceAbilityHandler fun
 * @tc.type: FUNC
 */
HWTEST_F(WindowTest, IAceAbilityHandler, TestSize.Level1)
{
    g_logMsg.clear();
    LOG_SetCallback(MyLogCallback);
    sptr<Window> window = sptr<Window>::MakeSptr();
    sptr<IAceAbilityHandler> listener = sptr<IAceAbilityHandler>::MakeSptr();
    uint32_t color = 66;
    listener->SetBackgroundColor(color);
    EXPECT_FALSE(g_logMsg.find("invalid color string") != std::string::npos);
    LOG_SetCallback(nullptr);
    auto ret = listener->GetBackgroundColor();
    EXPECT_EQ(ret, 0xffffffff);
    ASSERT_EQ(WMError::WM_OK, window->Destroy());
}

/**
 * @tc.name: Marshalling
 * @tc.desc: keyboardAnimationCurve marshalling
 * @tc.type: FUNC
 */
HWTEST_F(WindowTest, keyboardAnimationCurveMarshalling, TestSize.Level1)
{
    MessageParcel data;
    KeyboardAnimationCurve curveConfig;
    auto ret = data.WriteParcelable(&curveConfig);
    Parcel parcel;
    curveConfig.Unmarshalling(parcel);
    ASSERT_EQ(true, ret);
}

/**
 * @tc.name: GetVSyncPeriod
 * @tc.desc: window GetVSyncPeriod
 * @tc.type: FUNC
 */
HWTEST_F(WindowTest, GetVSyncPeriod, TestSize.Level1)
{
    sptr<WindowOption> winOption = sptr<WindowOption>::MakeSptr();
    winOption->SetWindowType(OHOS::Rosen::WindowType::WINDOW_TYPE_INPUT_METHOD_FLOAT);

    sptr<WindowOption> option = sptr<WindowOption>::MakeSptr();
    sptr<Window> window = Window::Create("win", option);
    if (window != nullptr) {
        ASSERT_NE(nullptr, window);
        int64_t period = window->GetVSyncPeriod();
        ASSERT_LE(-1, period);
    }
    sptr<Window> window2 = sptr<Window>::MakeSptr();
    ASSERT_NE(nullptr, window2);
    int64_t period_ = window2->GetVSyncPeriod();
    ASSERT_LE(-1, period_);
}

/**
 * @tc.name: performBack
 * @tc.desc: window performBack
 * @tc.type: FUNC
 */
HWTEST_F(WindowTest, performBack, TestSize.Level1)
{
    sptr<WindowOption> winOption = sptr<WindowOption>::MakeSptr();
    winOption->SetWindowType(OHOS::Rosen::WindowType::WINDOW_TYPE_INPUT_METHOD_FLOAT);

    sptr<WindowOption> option = sptr<WindowOption>::MakeSptr();
    sptr<Window> window = Window::Create("performBack", option);
    if (window != nullptr) {
        ASSERT_NE(nullptr, window);
        window->PerformBack();
    }
    sptr<Window> window2 = sptr<Window>::MakeSptr();
    ASSERT_NE(nullptr, window2);
    window2->PerformBack();
}

/**
 * @tc.name: SetResizeByDragEnabled
 * @tc.desc: set dragEnabled flag
 * @tc.type: FUNC
 */
HWTEST_F(WindowTest, SetResizeByDragEnabled, TestSize.Level1)
{
    sptr<Window> window = sptr<Window>::MakeSptr();
    ASSERT_EQ(WMError::WM_ERROR_DEVICE_NOT_SUPPORT, window->SetResizeByDragEnabled(true));
    ASSERT_EQ(WMError::WM_OK, window->Destroy());
}

/**
 * @tc.name: SetMainWindowRaiseByClickEnabled
 * @tc.desc: set raiseEnabled flag
 * @tc.type: FUNC
 */
HWTEST_F(WindowTest, SetMainWindowRaiseByClickEnabled, TestSize.Level1)
{
    sptr<Window> window = sptr<Window>::MakeSptr();
    EXPECT_EQ(WMError::WM_ERROR_DEVICE_NOT_SUPPORT, window->SetMainWindowRaiseByClickEnabled(true));
    EXPECT_EQ(WMError::WM_OK, window->Destroy());
}

/**
 * @tc.name: SetRaiseByClickEnabled
 * @tc.desc: set raiseEnabled flag
 * @tc.type: FUNC
 */
HWTEST_F(WindowTest, SetRaiseByClickEnabled, TestSize.Level1)
{
    sptr<Window> window = sptr<Window>::MakeSptr();
    ASSERT_EQ(WMError::WM_ERROR_DEVICE_NOT_SUPPORT, window->SetRaiseByClickEnabled(true));
    ASSERT_EQ(WMError::WM_OK, window->Destroy());
}

/**
 * @tc.name: RaiseAboveTarget
 * @tc.desc: RaiseAboveTarget flag
 * @tc.type: FUNC
 */
HWTEST_F(WindowTest, RaiseAboveTarget, TestSize.Level1)
{
    sptr<Window> window = sptr<Window>::MakeSptr();
    ASSERT_EQ(WMError::WM_ERROR_DEVICE_NOT_SUPPORT, window->RaiseAboveTarget(2));
    ASSERT_EQ(WMError::WM_OK, window->Destroy());
}

/**
 * @tc.name: HideNonSystemFloatingWindows
 * @tc.desc: set shouldHide flag
 * @tc.type: FUNC
 */
HWTEST_F(WindowTest, HideNonSystemFloatingWindows, TestSize.Level1)
{
    sptr<Window> window = sptr<Window>::MakeSptr();
    ASSERT_EQ(WMError::WM_ERROR_DEVICE_NOT_SUPPORT, window->HideNonSystemFloatingWindows(false));
    ASSERT_EQ(WMError::WM_OK, window->Destroy());
}

/**
 * @tc.name: GetWindowLimits
 * @tc.desc: window GetWindowLimits
 * @tc.type: FUNC
 */
HWTEST_F(WindowTest, GetWindowLimits, TestSize.Level1)
{
    sptr<Window> window = sptr<Window>::MakeSptr();
    WindowLimits windowLimits;
    auto ret = window->GetWindowLimits(windowLimits);
    ASSERT_EQ(WMError::WM_ERROR_DEVICE_NOT_SUPPORT, ret);
    ASSERT_EQ(WMError::WM_OK, window->Destroy());
}

/**
 * @tc.name: SetWindowLimits
 * @tc.desc: window SetWindowLimits
 * @tc.type: FUNC
 */
HWTEST_F(WindowTest, SetWindowLimits, TestSize.Level1)
{
    sptr<Window> window = sptr<Window>::MakeSptr();
    WindowLimits windowLimits;
    auto ret = window->SetWindowLimits(windowLimits, false);
    ASSERT_EQ(WMError::WM_ERROR_DEVICE_NOT_SUPPORT, ret);
    ret = window->SetWindowLimits(windowLimits, true);
    ASSERT_EQ(WMError::WM_ERROR_DEVICE_NOT_SUPPORT, ret);
    ASSERT_EQ(WMError::WM_OK, window->Destroy());
}

/**
 * @tc.name: RegisterWindowVisibilityChangeListener
 * @tc.desc: Register window visibility change listener
 * @tc.type: FUNC
 */
HWTEST_F(WindowTest, RegisterWindowVisibilityChangeListener, TestSize.Level1)
{
    sptr<Window> window = sptr<Window>::MakeSptr();
    ASSERT_EQ(WMError::WM_ERROR_DEVICE_NOT_SUPPORT, window->RegisterWindowVisibilityChangeListener(nullptr));
    ASSERT_EQ(WMError::WM_OK, window->Destroy());
}

/**
 * @tc.name: UnregisterWindowVisibilityChangeListener
 * @tc.desc: Unregister window visibility change listener
 * @tc.type: FUNC
 */
HWTEST_F(WindowTest, UnregisterWindowVisibilityChangeListener, TestSize.Level1)
{
    sptr<Window> window = sptr<Window>::MakeSptr();
    ASSERT_EQ(WMError::WM_ERROR_DEVICE_NOT_SUPPORT, window->UnregisterWindowVisibilityChangeListener(nullptr));
    ASSERT_EQ(WMError::WM_OK, window->Destroy());
}

/**
 * @tc.name: TransferAccessibilityEvent
 * @tc.desc: get
 * @tc.type: FUNC
 */
HWTEST_F(WindowTest, TransferAccessibilityEvent, TestSize.Level1)
{
    sptr<Window> window = sptr<Window>::MakeSptr();
    Accessibility::AccessibilityEventInfo info;
    int64_t uiExtensionIdLevel = 0;
    ASSERT_EQ(WMError::WM_OK, window->TransferAccessibilityEvent(info, uiExtensionIdLevel));
}

/**
 * @tc.name: FlushFrameRate
 * @tc.desc: FlushFrameRate Test
 * @tc.type: FUNC
 */
HWTEST_F(WindowTest, FlushFrameRate, TestSize.Level1)
{
    sptr<Window> window = sptr<Window>::MakeSptr();
    uint32_t rate = 120;
    uint32_t rateType = 0;
    int32_t animatorExpectedFrameRate = -1;
    window->FlushFrameRate(rate, animatorExpectedFrameRate, rateType);
    ASSERT_EQ(WMError::WM_OK, window->Destroy());
}

/**
 * @tc.name: SetSingleFrameComposerEnabled
 * @tc.desc: set single frame composer enable flag
 * @tc.type: FUNC
 */
HWTEST_F(WindowTest, SetSingleFrameComposerEnabled, TestSize.Level1)
{
    sptr<Window> window = sptr<Window>::MakeSptr();
    ASSERT_EQ(WMError::WM_ERROR_DEVICE_NOT_SUPPORT, window->SetSingleFrameComposerEnabled(false));
    ASSERT_EQ(WMError::WM_OK, window->Destroy());
}

/**
 * @tc.name: Maximize01
 * @tc.desc: maximize interface Test
 * @tc.type: FUNC
 */
HWTEST_F(WindowTest, Maximize01, TestSize.Level1)
{
    sptr<Window> window = sptr<Window>::MakeSptr();
    MaximizePresentation presentation = MaximizePresentation::ENTER_IMMERSIVE;
    ASSERT_EQ(WMError::WM_ERROR_DEVICE_NOT_SUPPORT, window->Maximize(presentation));
}

/**
 * @tc.name: RegisterWindowRectChangeListener
 * @tc.desc: get
 * @tc.type: FUNC
 */
HWTEST_F(WindowTest, RegisterWindowRectChangeListener, TestSize.Level1)
{
    sptr<Window> window = sptr<Window>::MakeSptr();
    sptr<IWindowRectChangeListener> listener = nullptr;
    auto ret = window->RegisterWindowRectChangeListener(listener);
    ASSERT_EQ(WMError::WM_ERROR_DEVICE_NOT_SUPPORT, ret);
    ASSERT_EQ(WMError::WM_OK, window->Destroy());
}

/**
 * @tc.name: UnregisterWindowRectChangeListener
 * @tc.desc: get
 * @tc.type: FUNC
 */
HWTEST_F(WindowTest, UnregisterWindowRectChangeListener, TestSize.Level1)
{
    sptr<Window> window = sptr<Window>::MakeSptr();
    sptr<IWindowRectChangeListener> listener = nullptr;
    auto ret = window->UnregisterWindowRectChangeListener(listener);
    ASSERT_EQ(WMError::WM_ERROR_DEVICE_NOT_SUPPORT, ret);
    ASSERT_EQ(WMError::WM_OK, window->Destroy());
}

/**
 * @tc.name: RegisterKeyboardPanelInfoChangeListener
 * @tc.desc: get
 * @tc.type: FUNC
 */
HWTEST_F(WindowTest, RegisterKeyboardPanelInfoChangeListener, TestSize.Level1)
{
    sptr<Window> window = sptr<Window>::MakeSptr();
    sptr<IKeyboardPanelInfoChangeListener> listener = nullptr;
    auto ret = window->RegisterKeyboardPanelInfoChangeListener(listener);
    ASSERT_EQ(WMError::WM_OK, ret);
    ASSERT_EQ(WMError::WM_OK, window->Destroy());
}

/**
 * @tc.name: UnregisterKeyboardPanelInfoChangeListener
 * @tc.desc: get
 * @tc.type: FUNC
 */
HWTEST_F(WindowTest, UnregisterKeyboardPanelInfoChangeListener, TestSize.Level1)
{
    sptr<Window> window = sptr<Window>::MakeSptr();
    sptr<IKeyboardPanelInfoChangeListener> listener = nullptr;
    auto ret = window->UnregisterKeyboardPanelInfoChangeListener(listener);
    ASSERT_EQ(WMError::WM_OK, ret);
    ASSERT_EQ(WMError::WM_OK, window->Destroy());
}

/**
 * @tc.name: GetTopWindowWithContext
 * @tc.desc: get
 * @tc.type: FUNC
 */
HWTEST_F(WindowTest, GetTopWindowWithContext, TestSize.Level1)
{
    sptr<Window> window = sptr<Window>::MakeSptr();
    ASSERT_EQ(nullptr, window->GetTopWindowWithContext(nullptr));

    sptr<WindowOption> option = sptr<WindowOption>::MakeSptr();
    sptr<WindowSessionImpl> winSession = sptr<WindowSessionImpl>::MakeSptr(option);
    winSession->property_->SetWindowType(WindowType::WINDOW_TYPE_APP_MAIN_WINDOW);
    winSession->property_->SetPersistentId(1);
    string winName = "test";
    int32_t winId = 1;
    WindowSessionImpl::windowSessionMap_.insert(
        make_pair(winName, std::pair<int32_t, sptr<WindowSessionImpl>>(winId, winSession)));
    std::unique_ptr<Mocker> m = std::make_unique<Mocker>();
    EXPECT_CALL(m->Mock(), GetTopWindowId(_, _))
        .Times(1)
        .WillOnce(DoAll(SetArgReferee<1>(winId), Return(WMError::WM_OK)));
    ASSERT_NE(nullptr, window->GetTopWindowWithContext(nullptr));

    EXPECT_CALL(m->Mock(), GetTopWindowId(_, _))
        .Times(2)
        .WillOnce(DoAll(SetArgReferee<1>(winId), Return(WMError::WM_DO_NOTHING)));
    ASSERT_EQ(nullptr, window->GetTopWindowWithContext(nullptr));

    winSession->property_->SetWindowType(WindowType::WINDOW_TYPE_APP_SUB_WINDOW);
    ASSERT_EQ(nullptr, window->GetTopWindowWithContext(nullptr));

    winSession->property_->SetWindowType(WindowType::WINDOW_TYPE_APP_MAIN_WINDOW);
    int32_t tempWinId = 4;
    EXPECT_CALL(m->Mock(), GetTopWindowId(_, _))
        .Times(1)
        .WillOnce(DoAll(SetArgReferee<1>(tempWinId), Return(WMError::WM_OK)));
    ASSERT_EQ(nullptr, window->GetTopWindowWithContext(nullptr));

    WindowSessionImpl::windowSessionMap_.erase(winName);
}

/**
 * @tc.name: GetMainWindowWithContext|GetWindowWithId
 *                      |GetSubWindow|UpdateConfigurationForAll
 * @tc.desc: get
 * @tc.type: FUNC
 */
HWTEST_F(WindowTest, GetMainWindowWithContext, TestSize.Level1)
{
    sptr<Window> window = sptr<Window>::MakeSptr();
    uint32_t windId = 0;
    uint32_t parentId = 1;
    std::shared_ptr<AppExecFwk::Configuration> configuration = nullptr;

    std::shared_ptr<AbilityRuntime::Context> context = nullptr;
    auto ret = window->GetMainWindowWithContext(context);
    window->GetWindowWithId(windId);
    window->GetSubWindow(parentId);
    window->UpdateConfigurationForAll(configuration);
    ASSERT_EQ(nullptr, ret);
}

/**
 * @tc.name: SetTopmost|GetWIsTopmostindowWithId
 * @tc.desc: get
 * @tc.type: FUNC
 */
HWTEST_F(WindowTest, SetTopmost, TestSize.Level1)
{
    sptr<Window> window = sptr<Window>::MakeSptr();
    auto ret = window->SetTopmost(false);
    ASSERT_EQ(WMError::WM_OK, ret);
    ASSERT_EQ(false, window->IsTopmost());
    ASSERT_EQ(WMError::WM_OK, window->Destroy());
}

/**
 * @tc.name: SetUIContentByName
 * @tc.desc: get
 * @tc.type: FUNC
 */
HWTEST_F(WindowTest, SetUIContentByName, TestSize.Level1)
{
    sptr<Window> window = sptr<Window>::MakeSptr();
    napi_env env = nullptr;
    napi_value storage = nullptr;
    auto ret = window->SetUIContentByName("/system/etc/window/resources/test.abc", env, storage);
    ASSERT_EQ(WMError::WM_OK, ret);
    ASSERT_EQ(WMError::WM_OK, window->Destroy());
}

/**
 * @tc.name: TriggerBindModalUIExtension
 * @tc.desc: get
 * @tc.type: FUNC
 */
HWTEST_F(WindowTest, TriggerBindModalUIExtension, TestSize.Level1)
{
    sptr<WindowOption> winOption = sptr<WindowOption>::MakeSptr();
    winOption->SetWindowType(OHOS::Rosen::WindowType::WINDOW_TYPE_INPUT_METHOD_FLOAT);
    sptr<WindowOption> option = sptr<WindowOption>::MakeSptr();
    sptr<Window> window = Window::Create("TriggerBindModalUIExtension", option);
    if (window != nullptr) {
        ASSERT_NE(nullptr, window);
        window->TriggerBindModalUIExtension();
    }
    sptr<Window> window2 = sptr<Window>::MakeSptr();
    ASSERT_NE(nullptr, window2);
    window2->PerformBack();
}

/**
 * @tc.name: RegisterTransferComponentDataForResultListener
 * @tc.desc: get
 * @tc.type: FUNC
 */
HWTEST_F(WindowTest, RegisterTransferComponentDataForResultListener, TestSize.Level1)
{
    g_logMsg.clear();
    LOG_SetCallback(MyLogCallback);
    sptr<Window> window = sptr<Window>::MakeSptr();
    NotifyTransferComponentDataForResultFunc func;
    window->RegisterTransferComponentDataForResultListener(func);
    EXPECT_FALSE(g_logMsg.find("session invalid.") != std::string::npos);
    LOG_SetCallback(nullptr);
    ASSERT_EQ(WMError::WM_OK, window->Destroy());
}

/**
 * @tc.name: SetTextFieldAvoidInfo|KeepKeyboardOnFocus
 * @tc.desc: get
 * @tc.type: FUNC
 */
HWTEST_F(WindowTest, SetTextFieldAvoidInfo, TestSize.Level1)
{
    sptr<Window> window = sptr<Window>::MakeSptr();
    auto ret = window->SetTextFieldAvoidInfo(50.0, 100.0);
    ASSERT_EQ(WMError::WM_OK, ret);
    auto retur = window->KeepKeyboardOnFocus(false);
    ASSERT_EQ(WmErrorCode::WM_ERROR_DEVICE_NOT_SUPPORT, retur);
    ASSERT_EQ(WMError::WM_OK, window->Destroy());
}

/**
 * @tc.name: Test01
 * @tc.desc: Test01
 * @tc.type: FUNC
 */
HWTEST_F(WindowTest, Test01, TestSize.Level1)
{
    sptr<Window> window = sptr<Window>::MakeSptr();
    SystemBarProperty prop;
    ASSERT_EQ(WMError::WM_OK, window->SetSpecificBarProperty(WindowType::WINDOW_TYPE_APP_MAIN_WINDOW, prop));
    bool isVisble = false;
    ASSERT_EQ(WMError::WM_ERROR_DEVICE_NOT_SUPPORT, window->SetDecorVisible(true));
    ASSERT_EQ(WMError::WM_ERROR_DEVICE_NOT_SUPPORT, window->GetDecorVisible(isVisble));
    ASSERT_EQ(WMError::WM_ERROR_DEVICE_NOT_SUPPORT, window->SetWindowTitleMoveEnabled(true));
    ASSERT_EQ(WMError::WM_ERROR_DEVICE_NOT_SUPPORT, window->SetTitleButtonVisible(true, true, true, true));
    auto var = 5;
    ASSERT_EQ(WMError::WM_ERROR_DEVICE_NOT_SUPPORT, window->SetDecorHeight(var));
    ASSERT_EQ(WMError::WM_ERROR_DEVICE_NOT_SUPPORT, window->GetDecorHeight(var));
    ASSERT_EQ(WMError::WM_ERROR_DEVICE_NOT_SUPPORT, window->ClearKeyEventFilter());
    IWindowVisibilityChangedListener windowVisibilityChangedListener;
    windowVisibilityChangedListener.OnWindowVisibilityChangedCallback(false);
    ASSERT_EQ(WMError::WM_OK, window->Destroy());
}

/**
 * @tc.name: Test02
 * @tc.desc: Test02
 * @tc.type: FUNC
 */
HWTEST_F(WindowTest, Test02, TestSize.Level1)
{
    sptr<Window> window = sptr<Window>::MakeSptr();
    IWindowLifeCycle windowLifeCycle;
    windowLifeCycle.AfterResumed();
    windowLifeCycle.AfterPaused();
    windowLifeCycle.AfterDestroyed();
    IWindowStatusChangeListener windowStatusChangeListener;
    windowStatusChangeListener.OnWindowStatusChange(WindowStatus::WINDOW_STATUS_UNDEFINED);
    ASSERT_EQ(WMError::WM_ERROR_DEVICE_NOT_SUPPORT, window->SetDefaultDensityEnabled(true));
    ASSERT_EQ(false, window->GetDefaultDensityEnabled());
    Rect rect = { 0, 0, 0, 0 };
    window->UpdatePiPRect(rect, WindowSizeChangeReason::UNDEFINED);
    IWindowRectChangeListener windowRectChangeListener;
    windowRectChangeListener.OnRectChange(rect, WindowSizeChangeReason::UNDEFINED);
    ASSERT_EQ(WMError::WM_OK, window->Destroy());
}

/**
 * @tc.name: Test03
 * @tc.desc: Test03
 * @tc.type: FUNC
 */
HWTEST_F(WindowTest, Test03, TestSize.Level1)
{
    sptr<Window> window = sptr<Window>::MakeSptr();
    KeyEventFilterFunc keyEventFilterFunc;
    ASSERT_EQ(WMError::WM_ERROR_DEVICE_NOT_SUPPORT, window->SetKeyEventFilter(keyEventFilterFunc));
    IWindowNoInteractionListener windowNoInteractionListener;
    windowNoInteractionListener.OnWindowNoInteractionCallback();
    windowNoInteractionListener.SetTimeout(100);
    ASSERT_EQ(0, windowNoInteractionListener.GetTimeout());
    TitleButtonRect titleButtonRect = { 3, 3, 3, 3 };
    ASSERT_EQ(WMError::WM_ERROR_DEVICE_NOT_SUPPORT, window->GetTitleButtonArea(titleButtonRect));
    IWindowTitleButtonRectChangedListener windowTitleButtonRectChangedListener;
    windowTitleButtonRectChangedListener.OnWindowTitleButtonRectChanged(titleButtonRect);
    ASSERT_EQ(WMError::WM_OK, window->Destroy());
}

/**
 * @tc.name: Test04
 * @tc.desc: Test04
 * @tc.type: FUNC
 */
HWTEST_F(WindowTest, Test04, TestSize.Level1)
{
    sptr<Window> window = sptr<Window>::MakeSptr();
    ASSERT_EQ(nullptr, window->GetUIContentWithId(0));
    window->TriggerBindModalUIExtension();
    ASSERT_EQ(WMError::WM_ERROR_DEVICE_NOT_SUPPORT, window->SetGrayScale(0));
    ASSERT_EQ(WMError::WM_OK, window->Destroy());
}

/**
 * @tc.name: Test05
 * @tc.desc: Test05
 * @tc.type: FUNC
 */
HWTEST_F(WindowTest, Test05, TestSize.Level1)
{
    sptr<Window> window = sptr<Window>::MakeSptr();
    auto mainWinId = 0;
    auto window1 = window->GetTopWindowWithId(mainWinId);
    ASSERT_EQ(nullptr, window1);
    ASSERT_EQ(WMError::WM_OK, window->Destroy());
}

/**
 * @tc.name: SetTitleButtonVisible
 * @tc.desc: SetTitleButtonVisible
 * @tc.type: FUNC
 */
HWTEST_F(WindowTest, SetTitleButtonVisible, TestSize.Level1)
{
    sptr<Window> window = sptr<Window>::MakeSptr();
    WMError res = window->SetTitleButtonVisible(true, true, true, true);
    ASSERT_EQ(res, WMError::WM_ERROR_DEVICE_NOT_SUPPORT);
    res = window->SetTitleButtonVisible(false, true, true, true);
    ASSERT_EQ(res, WMError::WM_ERROR_DEVICE_NOT_SUPPORT);
    res = window->SetTitleButtonVisible(true, false, true, true);
    ASSERT_EQ(res, WMError::WM_ERROR_DEVICE_NOT_SUPPORT);
    res = window->SetTitleButtonVisible(true, true, false, true);
    ASSERT_EQ(res, WMError::WM_ERROR_DEVICE_NOT_SUPPORT);
    res = window->SetTitleButtonVisible(false, false, true, true);
    ASSERT_EQ(res, WMError::WM_ERROR_DEVICE_NOT_SUPPORT);
    res = window->SetTitleButtonVisible(false, true, false, true);
    ASSERT_EQ(res, WMError::WM_ERROR_DEVICE_NOT_SUPPORT);
    res = window->SetTitleButtonVisible(true, false, false, true);
    ASSERT_EQ(res, WMError::WM_ERROR_DEVICE_NOT_SUPPORT);
    res = window->SetTitleButtonVisible(false, false, false, true);
    ASSERT_EQ(res, WMError::WM_ERROR_DEVICE_NOT_SUPPORT);
}

/**
 * @tc.name: SetWindowTitle
 * @tc.desc: SetWindowTitle
 * @tc.type: FUNC
 */
HWTEST_F(WindowTest, SetWindowTitle, TestSize.Level1)
{
    sptr<Window> window = sptr<Window>::MakeSptr();
    std::string title = "SetWindowTitle";
    auto ret = window->SetWindowTitle(title);
    ASSERT_EQ(WMError::WM_ERROR_DEVICE_NOT_SUPPORT, ret);
    ASSERT_EQ(WMError::WM_OK, window->Destroy());
}

/**
 * @tc.name: SetSubWindowModal
 * @tc.desc: SetSubWindowModal
 * @tc.type: FUNC
 */
HWTEST_F(WindowTest, SetSubWindowModal, TestSize.Level1)
{
    sptr<Window> window = sptr<Window>::MakeSptr();
    auto ret = window->SetSubWindowModal(true);
    ASSERT_EQ(WMError::WM_ERROR_DEVICE_NOT_SUPPORT, ret);
    ASSERT_EQ(WMError::WM_OK, window->Destroy());
}

/**
 * @tc.name: GetWindowStatus
 * @tc.desc: GetWindowStatus
 * @tc.type: FUNC
 */
HWTEST_F(WindowTest, GetWindowStatus, TestSize.Level1)
{
    sptr<Window> window = sptr<Window>::MakeSptr();
    WindowStatus windowStatus;
    auto ret = window->GetWindowStatus(windowStatus);
    ASSERT_EQ(WMError::WM_ERROR_DEVICE_NOT_SUPPORT, ret);
    ASSERT_EQ(WMError::WM_OK, window->Destroy());
}

/**
 * @tc.name: GetCompatibleModeInPc
 * @tc.desc: GetCompatibleModeInPc
 * @tc.type: FUNC
 */
HWTEST_F(WindowTest, GetCompatibleModeInPc, TestSize.Level1)
{
    sptr<Window> window = sptr<Window>::MakeSptr();
    ASSERT_NE(window, nullptr);
    auto ret = window->GetCompatibleModeInPc();
    EXPECT_EQ(false, ret);
    EXPECT_EQ(WMError::WM_OK, window->Destroy());
}

/**
 * @tc.name: IsAdaptToCompatibleImmersive
 * @tc.desc: IsAdaptToCompatibleImmersive
 * @tc.type: FUNC
 */
HWTEST_F(WindowTest, IsAdaptToCompatibleImmersive, TestSize.Level1)
{
    sptr<Window> window = sptr<Window>::MakeSptr();
    ASSERT_NE(window, nullptr);
    auto ret = window->IsAdaptToCompatibleImmersive();
    EXPECT_EQ(false, ret);
    EXPECT_EQ(WMError::WM_OK, window->Destroy());
}

/**
 * @tc.name: IsPcOrFreeMultiWindowCapabilityEnabled
 * @tc.desc: IsPcOrFreeMultiWindowCapabilityEnabled
 * @tc.type: FUNC
 */
HWTEST_F(WindowTest, IsPcOrFreeMultiWindowCapabilityEnabled, TestSize.Level1)
{
    sptr<Window> window = sptr<Window>::MakeSptr();
    ASSERT_NE(window, nullptr);
    auto ret = window->IsPcOrFreeMultiWindowCapabilityEnabled();
    EXPECT_EQ(false, ret);
    EXPECT_EQ(WMError::WM_OK, window->Destroy());
}

/**
 * @tc.name: IsSceneBoardEnabled
 * @tc.desc: IsSceneBoardEnabled
 * @tc.type: FUNC
 */
HWTEST_F(WindowTest, IsSceneBoardEnabled, TestSize.Level1)
{
    sptr<Window> window = sptr<Window>::MakeSptr();
    auto ret = window->IsSceneBoardEnabled();
    EXPECT_EQ(false, ret);
    EXPECT_EQ(WMError::WM_OK, window->Destroy());
}

/**
 * @tc.name: RegisterMainWindowCloseListeners
 * @tc.desc: RegisterMainWindowCloseListeners
 * @tc.type: FUNC
 */
HWTEST_F(WindowTest, RegisterMainWindowCloseListeners, TestSize.Level1)
{
    sptr<Window> window = sptr<Window>::MakeSptr();
    ASSERT_NE(window, nullptr);
    sptr<IMainWindowCloseListener> listener = sptr<IMainWindowCloseListener>::MakeSptr();
    auto ret = window->RegisterMainWindowCloseListeners(listener);
    EXPECT_EQ(WMError::WM_ERROR_DEVICE_NOT_SUPPORT, ret);
    EXPECT_EQ(WMError::WM_OK, window->Destroy());
}

/**
 * @tc.name: UnregisterMainWindowCloseListeners
 * @tc.desc: UnregisterMainWindowCloseListeners
 * @tc.type: FUNC
 */
HWTEST_F(WindowTest, UnregisterMainWindowCloseListeners, TestSize.Level1)
{
    sptr<Window> window = sptr<Window>::MakeSptr();
    ASSERT_NE(window, nullptr);
    sptr<IMainWindowCloseListener> listener = sptr<IMainWindowCloseListener>::MakeSptr();
    auto ret = window->UnregisterMainWindowCloseListeners(listener);
    EXPECT_EQ(WMError::WM_ERROR_DEVICE_NOT_SUPPORT, ret);
    EXPECT_EQ(WMError::WM_OK, window->Destroy());
}

/**
 * @tc.name: RegisterWindowWillCloseListeners
 * @tc.desc: RegisterWindowWillCloseListeners
 * @tc.type: FUNC
 */
HWTEST_F(WindowTest, RegisterWindowWillCloseListeners, TestSize.Level1)
{
    sptr<Window> window = sptr<Window>::MakeSptr();
    sptr<IWindowWillCloseListener> listener = sptr<IWindowWillCloseListener>::MakeSptr();
    auto ret = window->RegisterWindowWillCloseListeners(listener);
    EXPECT_EQ(WMError::WM_ERROR_DEVICE_NOT_SUPPORT, ret);
    EXPECT_EQ(WMError::WM_OK, window->Destroy());
}

/**
 * @tc.name: UnRegisterWindowWillCloseListeners
 * @tc.desc: UnRegisterWindowWillCloseListeners
 * @tc.type: FUNC
 */
HWTEST_F(WindowTest, UnRegisterWindowWillCloseListeners, TestSize.Level1)
{
    sptr<Window> window = sptr<Window>::MakeSptr();
    sptr<IWindowWillCloseListener> listener = sptr<IWindowWillCloseListener>::MakeSptr();
    auto ret = window->UnRegisterWindowWillCloseListeners(listener);
    EXPECT_EQ(WMError::WM_ERROR_DEVICE_NOT_SUPPORT, ret);
    EXPECT_EQ(WMError::WM_OK, window->Destroy());
}

/**
 * @tc.name: Marshalling
 * @tc.desc: Marshalling
 * @tc.type: FUNC
 */
HWTEST_F(WindowTest, Marshalling, TestSize.Level1)
{
    OccupiedAreaType type = OccupiedAreaType::TYPE_INPUT;
    Rect rect = { 0, 0, 0, 0 };
    auto safeHeight = 0;
    auto textFieldPositionY = 0.0;
    auto textFieldHeight = 0.0;
    sptr<OccupiedAreaChangeInfo> info =
        sptr<OccupiedAreaChangeInfo>::MakeSptr(type, rect, safeHeight, textFieldPositionY, textFieldHeight);
    ASSERT_NE(info, nullptr);
    Parcel parcel;
    auto ret = info->Marshalling(parcel);
    EXPECT_EQ(true, ret);
}

/**
 * @tc.name: SetDecorButtonStyle
 * @tc.desc: SetDecorButtonStyle
 * @tc.type: FUNC
 */
HWTEST_F(WindowTest, SetDecorButtonStyle, TestSize.Level1)
{
    sptr<Window> window = sptr<Window>::MakeSptr();
    DecorButtonStyle style;
    WMError res = window->SetDecorButtonStyle(style);
    ASSERT_EQ(WMError::WM_ERROR_DEVICE_NOT_SUPPORT, res);
}

/**
 * @tc.name: GetDecorButtonStyle
 * @tc.desc: GetDecorButtonStyle
 * @tc.type: FUNC
 */
HWTEST_F(WindowTest, GetDecorButtonStyle, TestSize.Level1)
{
    sptr<Window> window = sptr<Window>::MakeSptr();
    DecorButtonStyle style;
    WMError res = window->GetDecorButtonStyle(style);
    ASSERT_EQ(WMError::WM_ERROR_DEVICE_NOT_SUPPORT, res);
}

/**
 * @tc.name: GetIsMidScene
 * @tc.desc: GetIsMidScene
 * @tc.type: FUNC
 */
HWTEST_F(WindowTest, GetIsMidScene, TestSize.Level1)
{
    sptr<Window> window = sptr<Window>::MakeSptr();
    bool isMidScene = false;
    WMError res = window->GetIsMidScene(isMidScene);
    EXPECT_EQ(WMError::WM_OK, res);
    EXPECT_EQ(WMError::WM_OK, window->Destroy());
}

/**
 * @tc.name: GetLayoutTransform
 * @tc.desc: get
 * @tc.type: FUNC
 */
HWTEST_F(WindowTest, GetLayoutTransform, TestSize.Level1)
{
    sptr<Window> window = sptr<Window>::MakeSptr();
    Transform trans;
    ASSERT_EQ(trans, window->GetLayoutTransform());
    ASSERT_EQ(WMError::WM_OK, window->Destroy());
}

/**
 * @tc.name: SetParentWindow
 * @tc.desc: SetParentWindow
 * @tc.type: FUNC
 */
HWTEST_F(WindowTest, SetParentWindow, TestSize.Level1)
{
    sptr<Window> window = sptr<Window>::MakeSptr();
    int32_t newParentWindowId = 1;
    ASSERT_EQ(WMError::WM_ERROR_DEVICE_NOT_SUPPORT, window->SetParentWindow(newParentWindowId));
    ASSERT_EQ(WMError::WM_OK, window->Destroy());
}

/**
 * @tc.name: GetParentWindow
 * @tc.desc: GetParentWindow
 * @tc.type: FUNC
 */
HWTEST_F(WindowTest, GetParentWindow, TestSize.Level1)
{
    sptr<Window> window = sptr<Window>::MakeSptr();
    sptr<Window> parentWindow = nullptr;
    ASSERT_EQ(WMError::WM_ERROR_DEVICE_NOT_SUPPORT, window->GetParentWindow(parentWindow));
    ASSERT_EQ(parentWindow, nullptr);
    ASSERT_EQ(WMError::WM_OK, window->Destroy());
}

/**
 * @tc.name: SetFollowParentMultiScreenPolicy
 * @tc.desc: SetFollowParentMultiScreenPolicy
 * @tc.type: FUNC
 */
HWTEST_F(WindowTest, SetFollowParentMultiScreenPolicy, Function | SmallTest | Level2)
{
    sptr<Window> window = sptr<Window>::MakeSptr();
    EXPECT_EQ(WMError::WM_ERROR_DEVICE_NOT_SUPPORT, window->SetFollowParentMultiScreenPolicy(true));
    EXPECT_EQ(WMError::WM_ERROR_DEVICE_NOT_SUPPORT, window->SetFollowParentMultiScreenPolicy(false));
    EXPECT_EQ(WMError::WM_OK, window->Destroy());
}

/**
 * @tc.name: IsPcOrPadFreeMultiWindowMode
 * @tc.desc: IsPcOrPadFreeMultiWindowMode
 * @tc.type: FUNC
 */
HWTEST_F(WindowTest, IsPcOrPadFreeMultiWindowMode, TestSize.Level1)
{
    sptr<Window> window = sptr<Window>::MakeSptr();
    auto ret = window->IsPcOrPadFreeMultiWindowMode();
    EXPECT_EQ(ret, false);
    EXPECT_EQ(WMError::WM_OK, window->Destroy());
}

/**
 * @tc.name: GetFreeMultiWindowModeEnabledState
 * @tc.desc: GetFreeMultiWindowModeEnabledState
 * @tc.type: FUNC
 */
HWTEST_F(WindowTest, GetFreeMultiWindowModeEnabledState, TestSize.Level1)
{
    sptr<Window> window = sptr<Window>::MakeSptr();
    auto ret = window->GetFreeMultiWindowModeEnabledState();
    EXPECT_EQ(ret, false);
    EXPECT_EQ(WMError::WM_OK, window->Destroy());
}
} // namespace
} // namespace Rosen
} // namespace OHOS