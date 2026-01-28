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
#include <int_wrapper.h>
#include <want_params_wrapper.h>

#include "ability_context_impl.h"
#include "accessibility_event_info.h"
#include "application_context.h"
#include "color_parser.h"
#include "extension/extension_business_info.h"
#include "mock_session.h"
#include "mock_session_stub.h"
#include "mock_uicontent.h"
#include "mock_window.h"
#include "mock_window_session_property.h"
#include "parameters.h"
#include "window_helper.h"
#include "window_session_impl.h"
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

void WindowSessionImplTest::SetUpTestCase() {}

void WindowSessionImplTest::TearDownTestCase() {}

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
HWTEST_F(WindowSessionImplTest, CreateWindowAndDestroy01, TestSize.Level1)
{
    sptr<WindowOption> option = sptr<WindowOption>::MakeSptr();
    option->SetWindowName("CreateWindow01");
    sptr<WindowSessionImpl> window = sptr<WindowSessionImpl>::MakeSptr(option);

    SessionInfo sessionInfo = { "CreateTestBundle", "CreateTestModule", "CreateTestAbility" };
    sptr<SessionMocker> session = new (std::nothrow) SessionMocker(sessionInfo);
    ASSERT_NE(nullptr, session);
    ASSERT_EQ(WMError::WM_OK, window->Create(nullptr, session));
    ASSERT_EQ(WMError::WM_OK, window->Create(abilityContext_, session));
    ASSERT_EQ(WMError::WM_OK, window->Create(abilityContext_, session));
    window->property_->SetPersistentId(1);
    ASSERT_EQ(WMError::WM_ERROR_INVALID_WINDOW, window->Destroy());
    // session is null
    window = sptr<WindowSessionImpl>::MakeSptr(option);
    ASSERT_EQ(WMError::WM_OK, window->Create(abilityContext_, nullptr));
    ASSERT_EQ(WMError::WM_ERROR_INVALID_WINDOW, window->Destroy());

    window = sptr<WindowSessionImpl>::MakeSptr(option);
    ASSERT_EQ(WMError::WM_OK, window->Create(abilityContext_, session));
    ASSERT_EQ(WMError::WM_ERROR_INVALID_WINDOW, window->Destroy(false));
}

/**
 * @tc.name: UpdateDefaultStatusBarColor
 * @tc.desc: UpdateDefaultStatusBarColor Test
 * @tc.type: FUNC
 */
HWTEST_F(WindowSessionImplTest, UpdateDefaultStatusBarColor, TestSize.Level1)
{
    sptr<WindowOption> option = sptr<WindowOption>::MakeSptr();
    sptr<WindowSessionImpl> window = sptr<WindowSessionImpl>::MakeSptr(option);
    window->property_ = sptr<WindowSessionProperty>::MakeSptr();
    window->property_->SetPersistentId(1);
    window->state_ = WindowState::STATE_SHOWN;
    window->uiContent_ = std::make_unique<Ace::UIContentMocker>();
    SystemBarProperty property = SystemBarProperty();
    property.settingFlag_ = SystemBarSettingFlag::COLOR_SETTING;
    window->property_->sysBarPropMap_[WindowType::WINDOW_TYPE_STATUS_BAR] = property;
    uint32_t contentColor = 0;
    EXPECT_EQ(window->UpdateStatusBarColorByColorMode(contentColor), WMError::WM_DO_NOTHING);

    property.settingFlag_ = SystemBarSettingFlag::DEFAULT_SETTING;
    window->property_->sysBarPropMap_[WindowType::WINDOW_TYPE_STATUS_BAR] = property;
    EXPECT_EQ(window->UpdateStatusBarColorByColorMode(contentColor), WMError::WM_ERROR_NULLPTR);

    std::shared_ptr<AbilityRuntime::Context> context = std::make_shared<AbilityRuntime::AbilityContextImpl>();
    context->applicationContext_ = std::make_shared<AbilityRuntime::ApplicationContext>();
    ASSERT_EQ(WMError::WM_OK, window->Create(context, nullptr));
    EXPECT_EQ(window->UpdateStatusBarColorByColorMode(contentColor), WMError::WM_ERROR_NULLPTR);

    context->applicationContext_->contextImpl_ = std::make_shared<AbilityRuntime::ContextImpl>();
    context->applicationContext_->contextImpl_->config_ = std::make_shared<AppExecFwk::Configuration>();
    ASSERT_EQ(WMError::WM_OK, window->Create(context, nullptr));
    EXPECT_EQ(window->UpdateStatusBarColorByColorMode(contentColor), WMError::WM_OK);

    std::string key = AAFwk::GlobalConfigurationKey::SYSTEM_COLORMODE;
    std::string value = AppExecFwk::ConfigurationInner::COLOR_MODE_LIGHT;
    context->applicationContext_->contextImpl_->config_->configParameter_[key] = value;
    EXPECT_EQ(window->UpdateStatusBarColorByColorMode(contentColor), WMError::WM_OK);

    value = AppExecFwk::ConfigurationInner::COLOR_MODE_DARK;
    context->applicationContext_->contextImpl_->config_->configParameter_[key] = value;
    EXPECT_EQ(window->UpdateStatusBarColorByColorMode(contentColor), WMError::WM_OK);

    key = AAFwk::GlobalConfigurationKey::COLORMODE_IS_SET_BY_APP;
    value = AppExecFwk::ConfigurationInner::COLOR_MODE_LIGHT;
    context->applicationContext_->contextImpl_->config_->configParameter_[key] = value;
    EXPECT_EQ(window->UpdateStatusBarColorByColorMode(contentColor), WMError::WM_OK);

    value = AppExecFwk::ConfigurationInner::COLOR_MODE_DARK;
    context->applicationContext_->contextImpl_->config_->configParameter_[key] = value;
    EXPECT_EQ(window->UpdateStatusBarColorByColorMode(contentColor), WMError::WM_OK);

    window->specifiedColorMode_ = "light";
    EXPECT_EQ(window->UpdateStatusBarColorByColorMode(contentColor), WMError::WM_OK);
}

/**
 * @tc.name: CreateWindowAndDestroy02
 * @tc.desc: Create window and destroy window
 * @tc.type: FUNC
 */
HWTEST_F(WindowSessionImplTest, CreateWindowAndDestroy02, TestSize.Level1)
{
    std::string identityToken = "testToken";
    sptr<WindowOption> option = sptr<WindowOption>::MakeSptr();
    option->SetWindowName("CreateWindow01");
    sptr<WindowSessionImpl> window = sptr<WindowSessionImpl>::MakeSptr(option);

    SessionInfo sessionInfo = { "CreateTestBundle", "CreateTestModule", "CreateTestAbility" };
    sptr<SessionMocker> session = new (std::nothrow) SessionMocker(sessionInfo);
    ASSERT_NE(nullptr, session);
    ASSERT_EQ(WMError::WM_OK, window->Create(nullptr, session, identityToken));
    ASSERT_EQ(WMError::WM_OK, window->Create(abilityContext_, session, identityToken));
    ASSERT_EQ(WMError::WM_OK, window->Create(abilityContext_, session, identityToken));
    window->property_->SetPersistentId(1);
    ASSERT_EQ(WMError::WM_ERROR_INVALID_WINDOW, window->Destroy());
    // session is null
    window = sptr<WindowSessionImpl>::MakeSptr(option);
    ASSERT_EQ(WMError::WM_OK, window->Create(abilityContext_, nullptr));
    ASSERT_EQ(WMError::WM_ERROR_INVALID_WINDOW, window->Destroy());

    window = sptr<WindowSessionImpl>::MakeSptr(option);
    ASSERT_EQ(WMError::WM_OK, window->Create(abilityContext_, session, identityToken));
    ASSERT_EQ(WMError::WM_ERROR_INVALID_WINDOW, window->Destroy(false));
}

/**
 * @tc.name: Connect01
 * @tc.desc: Connect session
 * @tc.type: FUNC
 */
HWTEST_F(WindowSessionImplTest, Connect01, TestSize.Level1)
{
    sptr<WindowOption> option = sptr<WindowOption>::MakeSptr();
    option->SetWindowName("Connect01");
    sptr<WindowSessionImpl> window = sptr<WindowSessionImpl>::MakeSptr(option);
    window->property_->SetPersistentId(1);
    // connect with null session
    ASSERT_EQ(WMError::WM_ERROR_NULLPTR, window->Connect());

    SessionInfo sessionInfo = { "CreateTestBundle", "CreateTestModule", "CreateTestAbility" };
    sptr<SessionMocker> session = new (std::nothrow) SessionMocker(sessionInfo);
    ASSERT_NE(nullptr, session);
    window->hostSession_ = session;
    EXPECT_CALL(*(session), Connect(_, _, _, _, _, _, _)).WillOnce(Return(WSError::WS_ERROR_NULLPTR));
    ASSERT_EQ(WMError::WM_ERROR_NULLPTR, window->Connect());
    EXPECT_CALL(*(session), Connect(_, _, _, _, _, _, _)).WillOnce(Return(WSError::WS_OK));
    ASSERT_EQ(WMError::WM_OK, window->Connect());
    ASSERT_EQ(WMError::WM_OK, window->Destroy());
}

/**
 * @tc.name: Connect02
 * @tc.desc: Connect session
 * @tc.type: FUNC
 */
HWTEST_F(WindowSessionImplTest, Connect02, TestSize.Level1)
{
    sptr<WindowOption> option = sptr<WindowOption>::MakeSptr();
    option->SetWindowName("Connect02");
    sptr<WindowSessionImpl> window = sptr<WindowSessionImpl>::MakeSptr(option);
    sptr<WindowSessionPropertyMocker> property = sptr<WindowSessionPropertyMocker>::MakeSptr();
    window->property_ = property;
    window->property_->SetPersistentId(1);
    window->SetContext(abilityContext_);
 
    SessionInfo sessionInfo = { "CreateTestBundle", "CreateTestModule", "CreateTestAbility" };
    sptr<SessionMocker> session = new(std::nothrow) SessionMocker(sessionInfo);
    window->hostSession_ = session;
    EXPECT_CALL(*(property), GetDisplayId()).WillOnce(Return(0)).WillRepeatedly(Return(1001));
    EXPECT_CALL(*(session), Connect(_, _, _, _, _, _, _)).WillOnce(Return(WSError::WS_OK));
    ASSERT_EQ(WMError::WM_OK, window->Connect());
    ASSERT_EQ(WMError::WM_OK, window->Destroy());
}

/**
 * @tc.name: Connect_RegisterWindowScaleCallback
 * @tc.desc: Connect test RegisterWindowScaleCallback
 * @tc.type: FUNC
 */
HWTEST_F(WindowSessionImplTest, Connect_RegisterWindowScaleCallback, TestSize.Level1)
{
    sptr<WindowOption> option = sptr<WindowOption>::MakeSptr();
    option->SetWindowName("Connect_RegisterWindowScaleCallback");
    sptr<WindowSessionImpl> window = sptr<WindowSessionImpl>::MakeSptr(option);
    SessionInfo sessionInfo = { "CreateTestBundle", "CreateTestModule", "CreateTestAbility" };
    sptr<SessionMocker> session = sptr<SessionMocker>::MakeSptr(sessionInfo);
    window->hostSession_ = session;
    window->property_->SetPersistentId(1);
    EXPECT_CALL(*(session), Connect(_, _, _, _, _, _, _)).WillRepeatedly(Return(WSError::WS_OK));
    window->property_->SetWindowType(WindowType::WINDOW_TYPE_UI_EXTENSION);
    EXPECT_EQ(WMError::WM_OK, window->Connect());
    window->property_->SetWindowType(WindowType::WINDOW_TYPE_APP_MAIN_WINDOW);
    sptr<CompatibleModeProperty> compatibleModeProperty = sptr<CompatibleModeProperty>::MakeSptr();
    compatibleModeProperty->SetIsAdaptToSimulationScale(true);
    window->property_->SetCompatibleModeProperty(compatibleModeProperty);
    EXPECT_EQ(WMError::WM_OK, window->Connect());
    EXPECT_EQ(WMError::WM_OK, window->Destroy());
}

/**
 * @tc.name: Show01
 * @tc.desc: Show session
 * @tc.type: FUNC
 */
HWTEST_F(WindowSessionImplTest, Show01, TestSize.Level1)
{
    sptr<WindowOption> option = sptr<WindowOption>::MakeSptr();
    option->SetWindowName("Show01");
    sptr<WindowSessionImpl> window = new (std::nothrow) WindowSessionImpl(option);
    ASSERT_NE(nullptr, window);
    window->property_->SetPersistentId(1);
    // show with null session
    ASSERT_EQ(WMError::WM_ERROR_INVALID_WINDOW, window->Show());

    SessionInfo sessionInfo = { "CreateTestBundle", "CreateTestModule", "CreateTestAbility" };
    sptr<SessionMocker> session = new (std::nothrow) SessionMocker(sessionInfo);
    ASSERT_NE(nullptr, session);
    window->hostSession_ = session;
    EXPECT_CALL(*(session), Foreground(_, _, _)).WillOnce(Return(WSError::WS_OK));
    ASSERT_EQ(WMError::WM_OK, window->Show());
    ASSERT_EQ(WMError::WM_OK, window->Show());
    window->state_ = WindowState::STATE_CREATED;
    EXPECT_CALL(*(session), Foreground(_, _, _)).WillOnce(Return(WSError::WS_ERROR_INVALID_SESSION));
    ASSERT_EQ(WMError::WM_ERROR_INVALID_SESSION, window->Show());
    ASSERT_EQ(WMError::WM_OK, window->Destroy());
}

/**
 * @tc.name: Hide01
 * @tc.desc: Hide session
 * @tc.type: FUNC
 */
HWTEST_F(WindowSessionImplTest, Hide01, TestSize.Level1)
{
    sptr<WindowOption> option = sptr<WindowOption>::MakeSptr();
    option->SetWindowName("Hide01");
    sptr<WindowSessionImpl> window = new (std::nothrow) WindowSessionImpl(option);
    ASSERT_NE(nullptr, window);
    window->property_->SetPersistentId(1);
    // show with null session
    ASSERT_EQ(WMError::WM_ERROR_INVALID_WINDOW, window->Hide());

    SessionInfo sessionInfo = { "CreateTestBundle", "CreateTestModule", "CreateTestAbility" };
    sptr<SessionMocker> session = new (std::nothrow) SessionMocker(sessionInfo);
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
 * @tc.name: SetWindowType01
 * @tc.desc: SetWindowType
 * @tc.type: FUNC
 */
HWTEST_F(WindowSessionImplTest, SetWindowType01, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "WindowSessionImplTest: SetWindowType01 start";
    sptr<WindowOption> option = sptr<WindowOption>::MakeSptr();
    sptr<WindowSessionImpl> window = sptr<WindowSessionImpl>::MakeSptr(option);
    ASSERT_NE(window->property_, nullptr);

    window->property_->SetPersistentId(1);
    option->SetWindowName("SetWindowType01");
    WindowType type = WindowType::WINDOW_TYPE_BOOT_ANIMATION;
    option->SetWindowType(type);
    window = sptr<WindowSessionImpl>::MakeSptr(option);
    ASSERT_NE(window, nullptr);

    WindowType type1 = WindowType::WINDOW_TYPE_POINTER;
    option->SetWindowType(type1);
    window = sptr<WindowSessionImpl>::MakeSptr(option);
    ASSERT_NE(window, nullptr);

    WindowType type2 = WindowType::WINDOW_TYPE_APP_MAIN_WINDOW;
    option->SetWindowType(type2);
    window = sptr<WindowSessionImpl>::MakeSptr(option);
    ASSERT_NE(window, nullptr);

    WindowType type3 = WindowType::APP_MAIN_WINDOW_END;
    option->SetWindowType(type3);
    window = sptr<WindowSessionImpl>::MakeSptr(option);
    ASSERT_NE(window, nullptr);

    GTEST_LOG_(INFO) << "WindowSessionImplTest: SetWindowType01 end";
}

/**
 * @tc.name: ColorSpace
 * @tc.desc: SetColorSpace and GetColorSpace
 * @tc.type: FUNC
 */
HWTEST_F(WindowSessionImplTest, ColorSpace, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "WindowSessionImplTest: ColorSpace start";
    sptr<WindowOption> option = sptr<WindowOption>::MakeSptr();
    option->SetWindowName("ColorSpace");
    sptr<WindowSessionImpl> window = new (std::nothrow) WindowSessionImpl(option);
    ASSERT_NE(window, nullptr);
    ASSERT_NE(window->property_, nullptr);
    window->property_->SetPersistentId(1);

    window->SetColorSpace(ColorSpace::COLOR_SPACE_DEFAULT);
    ColorSpace colorSpace = window->GetColorSpace();
    ASSERT_EQ(colorSpace, ColorSpace::COLOR_SPACE_DEFAULT);

    SessionInfo sessionInfo = { "CreateTestBundle", "CreateTestModule", "CreateTestAbility" };
    sptr<SessionMocker> session = new (std::nothrow) SessionMocker(sessionInfo);
    ASSERT_NE(session, nullptr);
    window->hostSession_ = session;
    window->state_ = WindowState::STATE_CREATED;
    ASSERT_FALSE(window->IsWindowSessionInvalid());
    window->surfaceNode_ = nullptr;
    window->SetColorSpace(ColorSpace::COLOR_SPACE_WIDE_GAMUT);
    ColorSpace colorSpace1 = window->GetColorSpace();
    ASSERT_EQ(colorSpace1, ColorSpace::COLOR_SPACE_DEFAULT);

    struct RSSurfaceNodeConfig config;
    window->surfaceNode_ = RSSurfaceNode::Create(config);
    window->SetColorSpace(ColorSpace::COLOR_SPACE_WIDE_GAMUT);
    ColorSpace colorSpace2 = window->GetColorSpace();
    ASSERT_EQ(colorSpace2, ColorSpace::COLOR_SPACE_WIDE_GAMUT);
    GTEST_LOG_(INFO) << "WindowSessionImplTest: ColorSpace end";
}

/**
 * @tc.name: MakeSubOrDialogWindowDragableAndMoveble02
 * @tc.desc: MakeSubOrDialogWindowDragableAndMoveble
 * @tc.type: FUNC
 */
HWTEST_F(WindowSessionImplTest, MakeSubOrDialogWindowDragableAndMoveble02, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "WindowSessionImplTest: MakeSubOrDialogWindowDragableAndMoveble02 start";
    sptr<WindowOption> option = sptr<WindowOption>::MakeSptr();
    option->SetDialogDecorEnable(true);
    option->SetWindowName("MakeSubOrDialogWindowDragableAndMoveble02");
    sptr<WindowSessionImpl> window = new (std::nothrow) WindowSessionImpl(option);
    ASSERT_NE(nullptr, window);
    window->property_->SetWindowType(WindowType::WINDOW_TYPE_DIALOG);
    window->windowSystemConfig_.windowUIType_ = WindowUIType::PC_WINDOW;
    window->MakeSubOrDialogWindowDragableAndMoveble();
    ASSERT_EQ(true, window->property_->IsDecorEnable());
    GTEST_LOG_(INFO) << "WindowSessionImplTest: MakeSubOrDialogWindowDragableAndMoveble02 end";
}

/**
 * @tc.name: MakeSubOrDialogWindowDragableAndMoveble03
 * @tc.desc: MakeSubOrDialogWindowDragableAndMoveble
 * @tc.type: FUNC
 */
HWTEST_F(WindowSessionImplTest, MakeSubOrDialogWindowDragableAndMoveble03, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "WindowSessionImplTest: MakeSubOrDialogWindowDragableAndMoveble03 start";
    sptr<WindowOption> option = sptr<WindowOption>::MakeSptr();
    option->SetDialogDecorEnable(true);
    option->SetWindowName("MakeSubOrDialogWindowDragableAndMoveble03");
    sptr<WindowSessionImpl> window = new (std::nothrow) WindowSessionImpl(option);
    ASSERT_NE(nullptr, window);
    window->property_->SetWindowType(WindowType::WINDOW_TYPE_DIALOG);
    window->windowSystemConfig_.windowUIType_ = WindowUIType::PHONE_WINDOW;
    window->MakeSubOrDialogWindowDragableAndMoveble();
    ASSERT_EQ(false, window->property_->IsDecorEnable());
    GTEST_LOG_(INFO) << "WindowSessionImplTest: MakeSubOrDialogWindowDragableAndMoveble03 end";
}

/**
 * @tc.name: MakeSubOrDialogWindowDragableAndMoveble
 * @tc.desc: MakeSubOrDialogWindowDragableAndMoveble04
 * @tc.type: FUNC
 */
HWTEST_F(WindowSessionImplTest, MakeSubOrDialogWindowDragableAndMoveble04, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "WindowSessionImplTest: MakeSubOrDialogWindowDragableAndMoveble04 start";
    sptr<WindowOption> option = sptr<WindowOption>::MakeSptr();
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
 * @tc.name: MakeSubOrDialogWindowDragableAndMoveble05
 * @tc.desc: MakeSubOrDialogWindowDragableAndMoveble
 * @tc.type: FUNC
 */
HWTEST_F(WindowSessionImplTest, MakeSubOrDialogWindowDragableAndMoveble05, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "WindowSessionImplTest: MakeSubOrDialogWindowDragableAndMoveble05 start";
    sptr<WindowOption> option = sptr<WindowOption>::MakeSptr();
    option->SetSubWindowDecorEnable(true);
    option->SetWindowName("MakeSubOrDialogWindowDragableAndMoveble05");
    sptr<WindowSessionImpl> window = sptr<WindowSessionImpl>::MakeSptr(option);
    window->property_->SetWindowType(WindowType::WINDOW_TYPE_APP_SUB_WINDOW);
    window->windowSystemConfig_.windowUIType_ = WindowUIType::PC_WINDOW;
    EXPECT_EQ(false, window->property_->IsDecorEnable());
    window->MakeSubOrDialogWindowDragableAndMoveble();
    EXPECT_EQ(true, window->property_->IsDecorEnable());
    window->property_->SetDecorEnable(false);
    sptr<WindowOption> option1 = sptr<WindowOption>::MakeSptr();
    option->SetWindowName("MakeSubOrDialogWindowDragableAndMoveble05_mainWindow");
    sptr<WindowSessionImpl> mainWindow = sptr<WindowSessionImpl>::MakeSptr(option1);
    mainWindow->property_->SetPersistentId(1);
    mainWindow->property_->SetWindowType(WindowType::WINDOW_TYPE_APP_MAIN_WINDOW);
    WindowSessionImpl::windowSessionMap_.clear();
    WindowSessionImpl::windowSessionMap_.insert(std::make_pair(mainWindow->GetWindowName(),
        std::pair<uint64_t, sptr<WindowSessionImpl>>(mainWindow->GetWindowId(), mainWindow)));
    window->MakeSubOrDialogWindowDragableAndMoveble();
    EXPECT_EQ(true, window->property_->IsDecorEnable());
    sptr<CompatibleModeProperty> compatibleModeProperty = sptr<CompatibleModeProperty>::MakeSptr();
    compatibleModeProperty->SetIsAdaptToSubWindow(true);
    mainWindow->context_ = std::make_shared<AbilityRuntime::AbilityContextImpl>();
    window->context_ = mainWindow->context_;
    window->property_->SetDecorEnable(false);
    window->MakeSubOrDialogWindowDragableAndMoveble();
    EXPECT_EQ(false, window->property_->IsDecorEnable());
    GTEST_LOG_(INFO) << "WindowSessionImplTest: MakeSubOrDialogWindowDragableAndMoveble05 end";
}

/**
 * @tc.name: WindowSessionCreateCheck01
 * @tc.desc: WindowSessionCreateCheck01
 * @tc.type: FUNC
 */
HWTEST_F(WindowSessionImplTest, WindowSessionCreateCheck01, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "WindowSessionImplTest: WindowSessionCreateCheck01 start";
    sptr<WindowOption> option = sptr<WindowOption>::MakeSptr();
    option->SetWindowName("WindowSessionCreateCheck");
    sptr<WindowSessionImpl> window = new (std::nothrow) WindowSessionImpl(option);
    ASSERT_NE(nullptr, window);

    sptr<WindowOption> option1 = sptr<WindowOption>::MakeSptr();
    option1->SetWindowName("WindowSessionCreateCheck"); // set the same name
    sptr<WindowSessionImpl> window1 = new (std::nothrow) WindowSessionImpl(option1);
    ASSERT_NE(nullptr, window1);

    WMError res = window1->WindowSessionCreateCheck();
    ASSERT_EQ(res, WMError::WM_OK);
    GTEST_LOG_(INFO) << "WindowSessionImplTest: WindowSessionCreateCheck01 end";
}

/**
 * @tc.name: WindowSessionCreateCheck
 * @tc.desc: WindowSessionCreateCheck03
 * @tc.type: FUNC
 */
HWTEST_F(WindowSessionImplTest, WindowSessionCreateCheck03, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "WindowSessionImplTest: WindowSessionCreateCheck03 start";
    sptr<WindowOption> option = sptr<WindowOption>::MakeSptr();
    std::string name = "WindowSessionCreateCheck03";
    option->SetWindowName(name);
    sptr<WindowSessionImpl> window = new (std::nothrow) WindowSessionImpl(option);
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
HWTEST_F(WindowSessionImplTest, SetActive, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "WindowSessionImplTest: SetActive start";
    sptr<WindowOption> option = sptr<WindowOption>::MakeSptr();
    option->SetWindowName("WindowSessionCreateCheck");
    sptr<WindowSessionImpl> window = new (std::nothrow) WindowSessionImpl(option);
    ASSERT_NE(window, nullptr);

    WSError res1 = window->SetActive(true);
    ASSERT_EQ(res1, WSError::WS_OK);
    res1 = window->SetActive(false);
    ASSERT_EQ(res1, WSError::WS_OK);

    GTEST_LOG_(INFO) << "WindowSessionImplTest: SetActive end";
}

/**
 * @tc.name: UpdateFocus
 * @tc.desc: UpdateFocus
 * @tc.type: FUNC
 */
HWTEST_F(WindowSessionImplTest, UpdateFocus, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "WindowSessionImplTest: UpdateFocus start";
    sptr<WindowOption> option = sptr<WindowOption>::MakeSptr();
    option->SetWindowName("UpdateFocus");
    sptr<WindowSessionImpl> window = new (std::nothrow) WindowSessionImpl(option);
    ASSERT_NE(window, nullptr);
    sptr<FocusNotifyInfo> info = nullptr;
    WSError res = window->UpdateFocus(info, true);
    info = sptr<FocusNotifyInfo>::MakeSptr();
    info->isSyncNotify_ = false;
    res = window->UpdateFocus(info, true);
    EXPECT_EQ(res, WSError::WS_OK);
    res = window->UpdateFocus(info, false);
    EXPECT_EQ(res, WSError::WS_OK);

    GTEST_LOG_(INFO) << "WindowSessionImplTest: UpdateFocus end";
}

/**
 * @tc.name: UpdateFocus01
 * @tc.desc: UpdateFocus
 * @tc.type: FUNC
 */
HWTEST_F(WindowSessionImplTest, UpdateFocus01, TestSize.Level1)
{
    sptr<WindowOption> option = sptr<WindowOption>::MakeSptr();
    option->SetWindowName("UpdateFocus01");
    sptr<WindowSessionImpl> window = new (std::nothrow) WindowSessionImpl(option);
    ASSERT_NE(window, nullptr);
    window->updateFocusTimeStamp_.store(2);
    auto info = sptr<FocusNotifyInfo>::MakeSptr();
    info->isSyncNotify_ = true;
    info->timeStamp_ = 1;
    WSError res = window->UpdateFocus(info, true);
    EXPECT_EQ(window->updateFocusTimeStamp_.load(), 2);
    info->timeStamp_ = 3;
    res = window->UpdateFocus(info, false);
    EXPECT_EQ(window->updateFocusTimeStamp_.load(), 3);
    res = window->UpdateFocus(info, true);
    EXPECT_EQ(res, WSError::WS_OK);
}

/**
 * @tc.name: UpdateFocus02
 * @tc.desc: UpdateFocus
 * @tc.type: FUNC
 */
HWTEST_F(WindowSessionImplTest, UpdateFocus02, TestSize.Level1)
{
    sptr<WindowOption> option = sptr<WindowOption>::MakeSptr();
    option->SetWindowName("UpdateFocus02");
    sptr<WindowSessionImpl> window = new (std::nothrow) WindowSessionImpl(option);
    ASSERT_NE(window, nullptr);
    sptr<WindowSessionImpl> window1 = new (std::nothrow) WindowSessionImpl(option);
    ASSERT_NE(window1, nullptr);
    window->property_->SetPersistentId(1);
    window1->property_->SetPersistentId(2);
    window->updateFocusTimeStamp_.store(2);
    auto info = sptr<FocusNotifyInfo>::MakeSptr(3, window->GetWindowId(), window1->GetWindowId(), true);
    WSError res = window->UpdateFocus(info, true);
    EXPECT_EQ(window->updateFocusTimeStamp_.load(), 3);
    info->timeStamp_ = 4;
    res = window->UpdateFocus(info, false);
    EXPECT_EQ(window->updateFocusTimeStamp_.load(), 4);
    EXPECT_EQ(res, WSError::WS_OK);
}

/**
 * @tc.name: RequestFocusByClient
 * @tc.desc: RequestFocusByClient Test
 * @tc.type: FUNC
 */
HWTEST_F(WindowSessionImplTest, RequestFocusByClient, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "WindowSessionImplTest: RequestFocusByClient start";
    sptr<WindowOption> option = sptr<WindowOption>::MakeSptr();
    option->SetWindowName("WindowRequestFocusByClientCheck");
    sptr<WindowSessionImpl> window = new (std::nothrow) WindowSessionImpl(option);
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
 * @tc.name: CreateWindowAndDestroy01
 * @tc.desc: GetPersistentId
 * @tc.type: FUNC
 */
HWTEST_F(WindowSessionImplTest, GetPersistentId01, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "WindowSessionImplTest: GetPersistentId start";
    sptr<WindowOption> option = sptr<WindowOption>::MakeSptr();
    sptr<WindowSessionImpl> window = sptr<WindowSessionImpl>::MakeSptr(option);
    ASSERT_NE(window->property_, nullptr);

    window->property_->SetPersistentId(1);
    const int32_t res2 = window->GetPersistentId();
    ASSERT_EQ(res2, 1);
    GTEST_LOG_(INFO) << "WindowSessionImplTest: GetPersistentId end";
}

/**
 * @tc.name: GetFloatingWindowParentId
 * @tc.desc: GetFloatingWindowParentId and UpdateTitleButtonVisibility
 * @tc.type: FUNC
 */
HWTEST_F(WindowSessionImplTest, GetFloatingWindowParentId, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "WindowSessionImplTest: GetFloatingWindowParentId start";
    sptr<WindowOption> option = sptr<WindowOption>::MakeSptr();
    option->SetWindowName("Connect");
    sptr<WindowSessionImpl> window = new (std::nothrow) WindowSessionImpl(option);
    ASSERT_NE(nullptr, window);
    window->property_->SetPersistentId(1);
    // connect with null session
    ASSERT_EQ(WMError::WM_ERROR_NULLPTR, window->Connect());

    SessionInfo sessionInfo = { "CreateTestBundle", "CreateTestModule", "CreateTestAbility" };
    sptr<SessionMocker> session = new (std::nothrow) SessionMocker(sessionInfo);
    ASSERT_NE(nullptr, session);
    window->hostSession_ = session;
    EXPECT_CALL(*(session), Connect(_, _, _, _, _, _, _)).WillOnce(Return(WSError::WS_ERROR_NULLPTR));
    ASSERT_EQ(WMError::WM_ERROR_NULLPTR, window->Connect());
    EXPECT_CALL(*(session), Connect(_, _, _, _, _, _, _)).WillOnce(Return(WSError::WS_OK));
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
HWTEST_F(WindowSessionImplTest, UpdateDecorEnable, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "WindowSessionImplTest: UpdateDecorEnable start";
    sptr<WindowOption> option = sptr<WindowOption>::MakeSptr();
    option->SetWindowName("UpdateDecorEnable");
    sptr<WindowSessionImpl> window = sptr<WindowSessionImpl>::MakeSptr(option);

    WindowMode mode = WindowMode::WINDOW_MODE_UNDEFINED;
    window->UpdateDecorEnable(true, mode);
    ASSERT_EQ(window->property_->windowMode_, mode);
    mode = WindowMode::WINDOW_MODE_UNDEFINED;
    window->UpdateDecorEnable(false, mode);
    ASSERT_EQ(window->property_->windowMode_, mode);
    mode = WindowMode::WINDOW_MODE_FULLSCREEN;
    sptr<CompatibleModeProperty> compatibleModeProperty = sptr<CompatibleModeProperty>::MakeSptr();
    ASSERT_NE(compatibleModeProperty, nullptr);
    compatibleModeProperty->SetDisableDecorFullscreen(true);
    window->property_->SetCompatibleModeProperty(compatibleModeProperty);
    window->UpdateDecorEnable(true, mode);
    GTEST_LOG_(INFO) << "WindowSessionImplTest: UpdateDecorEnable end";
}

/**
 * @tc.name: NotifyModeChange
 * @tc.desc: NotifyModeChange
 * @tc.type: FUNC
 */
HWTEST_F(WindowSessionImplTest, NotifyModeChange, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "WindowSessionImplTest: NotifyModeChange start";
    sptr<WindowOption> option = sptr<WindowOption>::MakeSptr();
    option->SetWindowName("NotifyModeChange");
    sptr<WindowSessionImpl> window = sptr<WindowSessionImpl>::MakeSptr(option);
    SessionInfo sessionInfo = { "CreateTestBundle", "CreateTestModule", "CreateTestAbility" };
    sptr<SessionMocker> session = sptr<SessionMocker>::MakeSptr(sessionInfo);
    window->hostSession_ = session;

    WindowMode mode = WindowMode::WINDOW_MODE_UNDEFINED;
    window->NotifyModeChange(mode, true);
    window->NotifyModeChange(mode, false);
    ASSERT_EQ(window->property_->windowMode_, mode);
    ASSERT_EQ(window->property_->isDecorEnable_, false);
    GTEST_LOG_(INFO) << "WindowSessionImplTest: NotifyModeChange end";
}

/**
 * @tc.name: RequestVsyncSucc
 * @tc.desc: RequestVsync Test Succ
 * @tc.type: FUNC
 */
HWTEST_F(WindowSessionImplTest, RequestVsyncSucc, TestSize.Level1)
{
    sptr<WindowOption> option = sptr<WindowOption>::MakeSptr();
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
HWTEST_F(WindowSessionImplTest, RequestVsyncErr, TestSize.Level1)
{
    sptr<WindowOption> option = sptr<WindowOption>::MakeSptr();
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
HWTEST_F(WindowSessionImplTest, ClearVsync, TestSize.Level1)
{
    sptr<WindowOption> option = sptr<WindowOption>::MakeSptr();
    option->SetWindowName("ClearVsync");
    sptr<WindowSessionImpl> window = new (std::nothrow) WindowSessionImpl(option);
    ASSERT_NE(window, nullptr);
    window->ClearVsyncStation();
    ASSERT_NE(window, nullptr);
}

/**
 * @tc.name: SetFocusable
 * @tc.desc: SetFocusable
 * @tc.type: FUNC
 */
HWTEST_F(WindowSessionImplTest, SetFocusable, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "WindowSessionImplTest: SetFocusable start";
    sptr<WindowOption> option = sptr<WindowOption>::MakeSptr();
    option->SetWindowName("SetFocusable");
    sptr<WindowSessionImpl> window = sptr<WindowSessionImpl>::MakeSptr(option);

    SessionInfo sessionInfo = { "CreateTestBundle", "CreateTestModule", "CreateTestAbility" };
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
    window = sptr<WindowSessionImpl>::MakeSptr(option);
    ASSERT_EQ(WMError::WM_OK, window->Create(abilityContext_, nullptr));
    res = window->SetFocusable(true);
    ASSERT_EQ(res, WMError::WM_ERROR_INVALID_WINDOW);
    res = window->SetFocusable(false);
    ASSERT_EQ(res, WMError::WM_ERROR_INVALID_WINDOW);
    ASSERT_EQ(WMError::WM_ERROR_INVALID_WINDOW, window->Destroy());
    GTEST_LOG_(INFO) << "WindowSessionImplTest: SetFocusable end";
}

/**
 * @tc.name: SetBrightness01
 * @tc.desc: SetBrightness
 * @tc.type: FUNC
 */
HWTEST_F(WindowSessionImplTest, SetBrightness01, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "WindowSessionImplTest: SetBrightness01 start";
    sptr<WindowOption> option = sptr<WindowOption>::MakeSptr();
    option->SetWindowName("SetBrightness01");
    sptr<WindowSessionImpl> window = sptr<WindowSessionImpl>::MakeSptr(option);

    SessionInfo sessionInfo = { "CreateTestBundle", "CreateTestModule", "CreateTestAbility" };
    sptr<SessionMocker> session = new (std::nothrow) SessionMocker(sessionInfo);
    EXPECT_NE(nullptr, session);
    EXPECT_EQ(WMError::WM_OK, window->Create(nullptr, session));
    EXPECT_NE(nullptr, window->property_);
    window->property_->SetPersistentId(1);

    float brightness = -0.5f; // brightness < 0
    WMError res = window->SetBrightness(brightness);
    EXPECT_EQ(res, WMError::WM_ERROR_INVALID_PARAM);
    brightness = 2.0f; // brightness > 1
    res = window->SetBrightness(brightness);
    EXPECT_EQ(res, WMError::WM_ERROR_INVALID_PARAM);

    brightness = 0.5f;
    window->hostSession_ = session;
    EXPECT_FALSE(window->IsWindowSessionInvalid());
    res = window->SetBrightness(brightness);
    EXPECT_EQ(res, WMError::WM_OK);
    ASSERT_EQ(WMError::WM_OK, window->Destroy());
    GTEST_LOG_(INFO) << "WindowSessionImplTest: SetBrightness01 end";
}

/**
 * @tc.name: SetBrightness02
 * @tc.desc: SetBrightness
 * @tc.type: FUNC
 */
HWTEST_F(WindowSessionImplTest, SetBrightness02, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "WindowSessionImplTest: SetBrightness02 start";
    sptr<WindowOption> option = sptr<WindowOption>::MakeSptr();
    option->SetWindowName("SetBrightness02");
    sptr<WindowSessionImpl> window = sptr<WindowSessionImpl>::MakeSptr(option);

    SessionInfo sessionInfo = { "CreateTestBundle", "CreateTestModule", "CreateTestAbility" };
    sptr<SessionMocker> session = new (std::nothrow) SessionMocker(sessionInfo);
    EXPECT_NE(nullptr, session);
    EXPECT_EQ(WMError::WM_OK, window->Create(nullptr, session));
    window->hostSession_ = session;
    EXPECT_NE(nullptr, window->property_);
    window->property_->SetPersistentId(1);
    window->property_->SetWindowType(WindowType::APP_MAIN_WINDOW_END);
    float brightness = 0.5f;
    WMError res = window->SetBrightness(brightness);
    EXPECT_EQ(res, WMError::WM_ERROR_INVALID_TYPE);

    window->property_->SetWindowType(WindowType::APP_SUB_WINDOW_BASE);
    res = window->SetBrightness(brightness);
    EXPECT_EQ(res, WMError::WM_OK);

    window->property_->SetWindowType(WindowType::WINDOW_TYPE_WALLET_SWIPE_CARD);
    res = window->SetBrightness(brightness);
    EXPECT_EQ(res, WMError::WM_OK);
    ASSERT_EQ(WMError::WM_OK, window->Destroy());
    GTEST_LOG_(INFO) << "WindowSessionImplTest: SetBrightness02 end";
}

/**
 * @tc.name: GetContentInfo
 * @tc.desc: GetContentInfo
 * @tc.type: FUNC
 */
HWTEST_F(WindowSessionImplTest, GetContentInfo, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "WindowSessionImplTest: GetContentInfo start";
    sptr<WindowOption> option = sptr<WindowOption>::MakeSptr();
    option->SetWindowName("GetContentInfo");
    sptr<WindowSessionImpl> window = sptr<WindowSessionImpl>::MakeSptr(option);

    SessionInfo sessionInfo = { "CreateTestBundle", "CreateTestModule", "CreateTestAbility" };
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
HWTEST_F(WindowSessionImplTest, OnNewWant, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "WindowSessionImplTest: OnNewWant start";
    sptr<WindowOption> option = sptr<WindowOption>::MakeSptr();
    option->SetWindowName("OnNewWant");
    sptr<WindowSessionImpl> window = sptr<WindowSessionImpl>::MakeSptr(option);

    SessionInfo sessionInfo = { "CreateTestBundle", "CreateTestModule", "CreateTestAbility" };
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
HWTEST_F(WindowSessionImplTest, SetAPPWindowLabel, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "WindowSessionImplTest: SetAPPWindowLabel start";
    sptr<WindowOption> option = sptr<WindowOption>::MakeSptr();
    option->SetWindowName("SetAPPWindowLabel");
    sptr<WindowSessionImpl> window = sptr<WindowSessionImpl>::MakeSptr(option);

    SessionInfo sessionInfo = { "CreateTestBundle", "CreateTestModule", "CreateTestAbility" };
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
HWTEST_F(WindowSessionImplTest, RegisterListener01, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "WindowSessionImplTest: RegisterListener01 start";
    sptr<WindowOption> option = sptr<WindowOption>::MakeSptr();
    option->SetWindowName("RegisterListener01");
    sptr<WindowSessionImpl> window = sptr<WindowSessionImpl>::MakeSptr(option);

    SessionInfo sessionInfo = { "CreateTestBundle", "CreateTestModule", "CreateTestAbility" };
    sptr<SessionMocker> session = new (std::nothrow) SessionMocker(sessionInfo);
    ASSERT_NE(nullptr, session);
    ASSERT_EQ(WMError::WM_OK, window->Create(nullptr, session));
    window->hostSession_ = session;
    ASSERT_NE(window->property_, nullptr);
    window->property_->SetPersistentId(1);

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
    sptr<IWindowCrossAxisListener> listener6 = nullptr;
    res = window->RegisterWindowCrossAxisListener(listener6);
    ASSERT_EQ(res, WMError::WM_ERROR_NULLPTR);
    res = window->UnregisterWindowCrossAxisListener(listener6);
    ASSERT_EQ(res, WMError::WM_ERROR_NULLPTR);
    ASSERT_EQ(WMError::WM_OK, window->Destroy());
    GTEST_LOG_(INFO) << "WindowSessionImplTest: RegisterListener01 end";
}

/**
 * @tc.name: RegisterListener02
 * @tc.desc: RegisterListener and UnregisterListener
 * @tc.type: FUNC
 */
HWTEST_F(WindowSessionImplTest, RegisterListener02, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "WindowSessionImplTest: RegisterListener02 start";
    sptr<WindowOption> option = sptr<WindowOption>::MakeSptr();
    option->SetWindowName("RegisterListener02");
    sptr<WindowSessionImpl> window = sptr<WindowSessionImpl>::MakeSptr(option);
    SessionInfo sessionInfo = { "CreateTestBundle", "CreateTestModule", "CreateTestAbility" };
    sptr<SessionMocker> session = new (std::nothrow) SessionMocker(sessionInfo);
    ASSERT_NE(nullptr, session);
    ASSERT_EQ(WMError::WM_OK, window->Create(nullptr, session));
    window->hostSession_ = session;
    ASSERT_NE(window->property_, nullptr);
    window->property_->SetPersistentId(1);

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

    IDisplayIdChangeListenerSptr listener9 = nullptr;
    res = window->RegisterDisplayIdChangeListener(listener9);
    ASSERT_EQ(res, WMError::WM_ERROR_NULLPTR);
    res = window->UnregisterDisplayIdChangeListener(listener9);
    ASSERT_EQ(res, WMError::WM_ERROR_NULLPTR);

    sptr<IWindowTitleButtonRectChangedListener> listener10 = nullptr;
    res = window->RegisterWindowTitleButtonRectChangeListener(listener10);
    ASSERT_EQ(res, WMError::WM_ERROR_NULLPTR);
    res = window->UnregisterWindowTitleButtonRectChangeListener(listener10);
    ASSERT_EQ(res, WMError::WM_ERROR_NULLPTR);
    ASSERT_EQ(WMError::WM_OK, window->Destroy());
    GTEST_LOG_(INFO) << "WindowSessionImplTest: RegisterListener02 end";
}

/**
 * @tc.name: RegisterListener03
 * @tc.desc: RegisterListener and UnregisterListener
 * @tc.type: FUNC
 */
HWTEST_F(WindowSessionImplTest, RegisterListener03, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "WindowSessionImplTest: RegisterListener03 start";
    sptr<WindowOption> option = sptr<WindowOption>::MakeSptr();
    option->SetWindowName("RegisterListener03");
    sptr<WindowSessionImpl> window = sptr<WindowSessionImpl>::MakeSptr(option);

    SessionInfo sessionInfo = { "CreateTestBundle", "CreateTestModule", "CreateTestAbility" };
    sptr<SessionMocker> session = sptr<SessionMocker>::MakeSptr(sessionInfo);
    ASSERT_NE(nullptr, session);
    ASSERT_EQ(WMError::WM_OK, window->Create(nullptr, session));
    window->hostSession_ = session;
    ASSERT_NE(window->property_, nullptr);
    window->property_->SetPersistentId(1);

    sptr<IDisplayMoveListener> listener6 = nullptr;
    WMError res = window->RegisterDisplayMoveListener(listener6);
    ASSERT_EQ(res, WMError::WM_ERROR_NULLPTR);
    res = window->UnregisterDisplayMoveListener(listener6);
    ASSERT_EQ(res, WMError::WM_ERROR_NULLPTR);

    sptr<IWindowRectChangeListener> listener7 = nullptr;
    res = window->RegisterWindowRectChangeListener(listener7);
    ASSERT_EQ(res, WMError::WM_ERROR_NULLPTR);

    sptr<ISubWindowCloseListener> listener10 = nullptr;
    res = window->RegisterSubWindowCloseListeners(listener10);
    ASSERT_EQ(res, WMError::WM_ERROR_NULLPTR);
    res = window->UnregisterSubWindowCloseListeners(listener10);
    ASSERT_EQ(res, WMError::WM_ERROR_NULLPTR);

    sptr<ISwitchFreeMultiWindowListener> listener11 = nullptr;
    res = window->RegisterSwitchFreeMultiWindowListener(listener11);
    ASSERT_EQ(res, WMError::WM_ERROR_NULLPTR);
    res = window->UnregisterSwitchFreeMultiWindowListener(listener11);
    ASSERT_EQ(res, WMError::WM_ERROR_NULLPTR);

    sptr<IMainWindowCloseListener> listener12 = nullptr;
    res = window->RegisterMainWindowCloseListeners(listener12);
    EXPECT_EQ(res, WMError::WM_ERROR_NULLPTR);
    res = window->UnregisterMainWindowCloseListeners(listener12);
    EXPECT_EQ(res, WMError::WM_ERROR_NULLPTR);

    ISystemDensityChangeListenerSptr listener13 = nullptr;
    res = window->RegisterSystemDensityChangeListener(listener13);
    ASSERT_EQ(res, WMError::WM_ERROR_NULLPTR);
    res = window->UnregisterSystemDensityChangeListener(listener13);
    ASSERT_EQ(res, WMError::WM_ERROR_NULLPTR);
    EXPECT_EQ(WMError::WM_OK, window->Destroy());
    GTEST_LOG_(INFO) << "WindowSessionImplTest: RegisterListener03 end";
}

/**
 * @tc.name: RegisterListener04
 * @tc.desc: RegisterListener and UnregisterListener
 * @tc.type: FUNC
 */
HWTEST_F(WindowSessionImplTest, RegisterListener04, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "WindowSessionImplTest: RegisterListener04 start";
    sptr<WindowOption> option = sptr<WindowOption>::MakeSptr();
    option->SetWindowName("RegisterListener04");
    sptr<WindowSessionImpl> window = sptr<WindowSessionImpl>::MakeSptr(option);

    SessionInfo sessionInfo = { "CreateTestBundle", "CreateTestModule", "CreateTestAbility" };
    sptr<SessionMocker> session = sptr<SessionMocker>::MakeSptr(sessionInfo);
    ASSERT_EQ(WMError::WM_OK, window->Create(nullptr, session));
    window->hostSession_ = session;
    window->property_->SetPersistentId(1);

    sptr<IWindowWillCloseListener> listener14 = nullptr;
    WMError res = window->RegisterWindowWillCloseListeners(listener14);
    ASSERT_EQ(res, WMError::WM_ERROR_NULLPTR);
    res = window->UnRegisterWindowWillCloseListeners(listener14);
    ASSERT_EQ(res, WMError::WM_ERROR_NULLPTR);

    EXPECT_EQ(WMError::WM_OK, window->Destroy());
    GTEST_LOG_(INFO) << "WindowSessionImplTest: RegisterListener04 end";
}

/**
 * @tc.name: RegisterListener05
 * @tc.desc: RegisterListener and UnregisterListener
 * @tc.type: FUNC
 */
HWTEST_F(WindowSessionImplTest, RegisterListener05, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "WindowSessionImplTest: RegisterListener05 start";
    sptr<WindowOption> listenerOption = sptr<WindowOption>::MakeSptr();
    ASSERT_NE(nullptr, listenerOption);
    listenerOption->SetWindowName("RegisterListener05");
    sptr<WindowSessionImpl> listenerwindow = sptr<WindowSessionImpl>::MakeSptr(listenerOption);
    SessionInfo sessionInfo = { "CreateTestBundle", "CreateTestModule", "CreateTestAbility" };
    sptr<SessionMocker> session = sptr<SessionMocker>::MakeSptr(sessionInfo);
    EXPECT_EQ(WMError::WM_OK, listenerwindow->Create(nullptr, session));
    listenerwindow->hostSession_ = session;
    ASSERT_NE(nullptr, listenerwindow->property_);
    listenerwindow->property_->SetPersistentId(1);
    sptr<IWindowStageLifeCycle> listener = nullptr;
    WMError res = listenerwindow->RegisterWindowStageLifeCycleListener(listener);
    EXPECT_EQ(res, WMError::WM_ERROR_NULLPTR);
    res = listenerwindow->UnregisterWindowStageLifeCycleListener(listener);
    EXPECT_EQ(res, WMError::WM_ERROR_NULLPTR);
    EXPECT_EQ(WMError::WM_OK, listenerwindow->Destroy());
    GTEST_LOG_(INFO) << "WindowSessionImplTest: RegisterListener05 end";
}

/**
 * @tc.name: NotifyDisplayMove
 * @tc.desc: NotifyDisplayMove
 * @tc.type: FUNC
 */
HWTEST_F(WindowSessionImplTest, NotifyDisplayMove, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "WindowSessionImplTest: NotifyDisplayMove start";
    sptr<WindowOption> option = sptr<WindowOption>::MakeSptr();
    option->SetWindowName("NotifyDisplayMove");
    sptr<WindowSessionImpl> window = sptr<WindowSessionImpl>::MakeSptr(option);

    SessionInfo sessionInfo = { "CreateTestBundle", "CreateTestModule", "CreateTestAbility" };
    sptr<SessionMocker> session = sptr<SessionMocker>::MakeSptr(sessionInfo);
    ASSERT_EQ(WMError::WM_OK, window->Create(nullptr, session));

    DisplayId from = 0;
    DisplayId to = 2;
    window->NotifyDisplayMove(from, to);
    ASSERT_EQ(WMError::WM_ERROR_INVALID_WINDOW, window->Destroy());
    GTEST_LOG_(INFO) << "WindowSessionImplTest: NotifyDisplayMove end";
}

/**
 * @tc.name: NotifyAfterForeground
 * @tc.desc: NotifyAfterForeground
 * @tc.type: FUNC
 */
HWTEST_F(WindowSessionImplTest, NotifyAfterForeground, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "WindowSessionImplTest: NotifyAfterForeground start";
    sptr<WindowOption> option = sptr<WindowOption>::MakeSptr();
    option->SetWindowName("NotifyAfterForeground");
    sptr<WindowSessionImpl> window = sptr<WindowSessionImpl>::MakeSptr(option);

    SessionInfo sessionInfo = { "CreateTestBundle", "CreateTestModule", "CreateTestAbility" };
    sptr<SessionMocker> session = sptr<SessionMocker>::MakeSptr(sessionInfo);
    ASSERT_NE(nullptr, session);
    ASSERT_EQ(WMError::WM_OK, window->Create(nullptr, session));

    window->NotifyAfterForeground(true, true);
    window->NotifyAfterForeground(false, false);
    window->vsyncStation_ = nullptr;
    window->NotifyAfterForeground(false, false);
    ASSERT_EQ(WMError::WM_ERROR_INVALID_WINDOW, window->Destroy());
    GTEST_LOG_(INFO) << "WindowSessionImplTest: NotifyAfterForeground end";
}

/**
 * @tc.name: NotifyAfterBackground
 * @tc.desc: NotifyAfterBackground
 * @tc.type: FUNC
 */
HWTEST_F(WindowSessionImplTest, NotifyAfterBackground, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "WindowSessionImplTest: NotifyAfterBackground start";
    sptr<WindowOption> option = sptr<WindowOption>::MakeSptr();
    option->SetWindowName("NotifyAfterBackground");
    sptr<WindowSessionImpl> window = sptr<WindowSessionImpl>::MakeSptr(option);

    SessionInfo sessionInfo = { "CreateTestBundle", "CreateTestModule", "CreateTestAbility" };
    sptr<SessionMocker> session = sptr<SessionMocker>::MakeSptr(sessionInfo);
    ASSERT_EQ(WMError::WM_OK, window->Create(nullptr, session));

    window->NotifyAfterBackground(true, true);
    window->NotifyAfterBackground(false, false);
    window->vsyncStation_ = nullptr;
    window->NotifyAfterBackground(false, false);
    ASSERT_EQ(WMError::WM_ERROR_INVALID_WINDOW, window->Destroy());
    GTEST_LOG_(INFO) << "WindowSessionImplTest: NotifyAfterBackground end";
}

/**
 * @tc.name: NotifyForegroundInteractiveStatus
 * @tc.desc: NotifyForegroundInteractiveStatus
 * @tc.type: FUNC
 */
HWTEST_F(WindowSessionImplTest, NotifyForegroundInteractiveStatus, TestSize.Level1)
{
    g_errLog.clear();
    LOG_SetCallback(MyLogCallback);
    GTEST_LOG_(INFO) << "WindowSessionImplTest: NotifyForegroundInteractiveStatus start";
    sptr<WindowOption> option = new WindowOption();
    ASSERT_NE(option, nullptr);
    option->SetWindowName("NotifyForegroundInteractiveStatus");
    sptr<WindowSessionImpl> foreWindow = sptr<WindowSessionImpl>::MakeSptr(option);;
    ASSERT_NE(foreWindow, nullptr);
    SessionInfo sessionInfo = { "CreateTestBundle", "CreateTestModule", "CreateTestAbility" };
    sptr<SessionMocker> session = new (std::nothrow) SessionMocker(sessionInfo);
    ASSERT_NE(session, nullptr);
    foreWindow->property_->SetPersistentId(1);
    foreWindow->hostSession_ = session;
    foreWindow->state_ = WindowState::STATE_SHOWN;
    EXPECT_FALSE(foreWindow->IsWindowSessionInvalid());
    foreWindow->isDidForeground_ = false;
    foreWindow->NotifyForegroundInteractiveStatus(true);
    foreWindow->NotifyForegroundInteractiveStatus(false);
    EXPECT_TRUE(g_errLog.find("isDidForeground:") == std::string::npos);
    EXPECT_EQ(WMError::WM_OK, foreWindow->Destroy());
    LOG_SetCallback(nullptr);
    GTEST_LOG_(INFO) << "WindowSessionImplTest: NotifyForegroundInteractiveStatus end";
}

/**
 * @tc.name: NotifyForegroundInteractiveStatus01
 * @tc.desc: NotifyForegroundInteractiveStatus01
 * @tc.type: FUNC
 */
HWTEST_F(WindowSessionImplTest, NotifyForegroundInteractiveStatus01, TestSize.Level1)
{
    g_errLog.clear();
    LOG_SetCallback(MyLogCallback);
    GTEST_LOG_(INFO) << "WindowSessionImplTest: NotifyForegroundInteractiveStatus01 start";
    sptr<WindowOption> option = new WindowOption();
    ASSERT_NE(option, nullptr);
    option->SetWindowName("NotifyForegroundInteractiveStatus01");
    sptr<WindowSessionImpl> foreWindow1 = new (std::nothrow) WindowSessionImpl(option);
    ASSERT_NE(foreWindow1, nullptr);
    SessionInfo sessionInfo = { "CreateTestBundle", "CreateTestModule", "CreateTestAbility" };
    sptr<SessionMocker> session = new (std::nothrow) SessionMocker(sessionInfo);
    ASSERT_NE(session, nullptr);
    foreWindow1->property_->SetPersistentId(1);
    foreWindow1->hostSession_ = session;
    foreWindow1->state_ = WindowState::STATE_SHOWN;
    EXPECT_FALSE(foreWindow1->IsWindowSessionInvalid());
    foreWindow1->isDidForeground_ = true;
    foreWindow1->NotifyForegroundInteractiveStatus(true);
    EXPECT_TRUE(g_errLog.find("isDidForeground:") == std::string::npos);

    ASSERT_NE(nullptr, foreWindow1->property_);
    foreWindow1->property_->SetPersistentId(1);
    foreWindow1->property_->SetUseControlState(true);
    foreWindow1->NotifyForegroundInteractiveStatus(false);
    foreWindow1->NotifyForegroundInteractiveStatus(true);
    EXPECT_TRUE(g_errLog.find("useControlState:") != std::string::npos);
    foreWindow1->property_->SetUseControlState(false);
    foreWindow1->NotifyForegroundInteractiveStatus(false);
    foreWindow1->NotifyForegroundInteractiveStatus(true);
    EXPECT_EQ(WMError::WM_OK, foreWindow1->Destroy());
    LOG_SetCallback(nullptr);
    GTEST_LOG_(INFO) << "WindowSessionImplTest: NotifyForegroundInteractiveStatus01 end";
}

/**
 * @tc.name: NotifyLifecyclePausedStatus
 * @tc.desc: NotifyLifecyclePausedStatus
 * @tc.type: FUNC
 */
HWTEST_F(WindowSessionImplTest, NotifyLifecyclePausedStatus, TestSize.Level1)
{
    g_errLog.clear();
    LOG_SetCallback(MyLogCallback);
    GTEST_LOG_(INFO) << "WindowSessionImplTest: NotifyLifecyclePausedStatus start";
    sptr<WindowOption> option = new WindowOption();
    ASSERT_NE(nullptr, option);
    option->SetWindowName("NotifyLifecyclePausedStatus");
    sptr<WindowSessionImpl> window = new (std::nothrow) WindowSessionImpl(option);
    ASSERT_NE(nullptr, window);
    window->property_->SetPersistentId(1);
    window->state_ = WindowState::STATE_SHOWN;
    window->NotifyLifecyclePausedStatus();
    window->state_ = WindowState::STATE_DESTROYED;
    window->NotifyLifecyclePausedStatus();

    SessionInfo sessionInfo = { "CreateTestBundle", "CreateTestModule", "CreateTestAbility" };
    sptr<SessionMocker> session = new (std::nothrow) SessionMocker(sessionInfo);
    ASSERT_NE(nullptr, session);
    window->hostSession_ = session;
    window->state_ = WindowState::STATE_SHOWN;
    EXPECT_FALSE(window->IsWindowSessionInvalid());
    window->NotifyLifecyclePausedStatus();
    window->state_ = WindowState::STATE_DESTROYED;
    window->NotifyLifecyclePausedStatus();
    EXPECT_TRUE(g_errLog.find("isDidForeground:") == std::string::npos);
    EXPECT_EQ(WMError::WM_ERROR_INVALID_WINDOW, window->Destroy());
    LOG_SetCallback(nullptr);
    GTEST_LOG_(INFO) << "WindowSessionImplTest: NotifyLifecyclePausedStatus end";
}

/**
 * @tc.name: NotifyAfterLifecycleForeground
 * @tc.desc: NotifyAfterLifecycleForeground
 * @tc.type: FUNC
 */
HWTEST_F(WindowSessionImplTest, NotifyAfterLifecycleForeground, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "WindowSessionImplTest: NotifyAfterLifecycleForeground start";
    sptr<WindowOption> foreOption = sptr<WindowOption>::MakeSptr();
    ASSERT_NE(nullptr, foreOption);
    foreOption->SetWindowName("NotifyAfterLifecycleForeground");
    sptr<WindowSessionImpl> foreWindow = sptr<WindowSessionImpl>::MakeSptr(foreOption);
    ASSERT_NE(nullptr, foreWindow);
    SessionInfo sessionInfo = { "CreateTestBundle", "CreateTestModule", "CreateTestAbility" };
    sptr<SessionMocker> session = new (std::nothrow) SessionMocker(sessionInfo);
    ASSERT_NE(nullptr, session);
    EXPECT_EQ(WMError::WM_OK, foreWindow->Create(nullptr, session));
    foreWindow->NotifyAfterLifecycleForeground();
    EXPECT_EQ(WMError::WM_ERROR_INVALID_WINDOW, foreWindow->Destroy());
    GTEST_LOG_(INFO) << "WindowSessionImplTest: NotifyAfterLifecycleForeground end";
}

/**
 * @tc.name: NotifyAfterLifecycleBackground
 * @tc.desc: NotifyAfterLifecycleBackground
 * @tc.type: FUNC
 */
HWTEST_F(WindowSessionImplTest, NotifyAfterLifecycleBackground, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "WindowSessionImplTest: NotifyAfterLifecycleBackground start";
    sptr<WindowOption> backOption = sptr<WindowOption>::MakeSptr();
    ASSERT_NE(nullptr, backOption);
    backOption->SetWindowName("NotifyAfterLifecycleBackground");
    sptr<WindowSessionImpl> window = sptr<WindowSessionImpl>::MakeSptr(backOption);
    SessionInfo sessionInfo = { "CreateTestBundle", "CreateTestModule", "CreateTestAbility" };
    sptr<SessionMocker> session = new (std::nothrow) SessionMocker(sessionInfo);
    ASSERT_NE(nullptr, session);
    EXPECT_EQ(WMError::WM_OK, window->Create(nullptr, session));
    window->NotifyAfterLifecycleBackground();
    EXPECT_EQ(WMError::WM_ERROR_INVALID_WINDOW, window->Destroy());
    GTEST_LOG_(INFO) << "WindowSessionImplTest: NotifyAfterLifecycleBackground end";
}

/**
 * @tc.name: Notify03
 * @tc.desc: NotifyCloseExistPipWindow NotifyAfterLifecycleResumed
 * @tc.type: FUNC
 */
 HWTEST_F(WindowSessionImplTest, NotifyAfterLifecycleResumed, TestSize.Level1)
{
    sptr<WindowOption> notifyOption = sptr<WindowOption>::MakeSptr();
    ASSERT_NE(nullptr, notifyOption);
    notifyOption->SetWindowName("NotifyAfterLifecycleResumed");
    sptr<WindowSessionImpl> notifyWindow = sptr<WindowSessionImpl>::MakeSptr(notifyOption);
    ASSERT_NE(nullptr, notifyWindow);
    SessionInfo sessionInfo = { "CreateTestBundle", "CreateTestModule", "CreateTestAbility" };
    sptr<SessionMocker> session = sptr<SessionMocker>::MakeSptr(sessionInfo);
    EXPECT_EQ(WMError::WM_OK, notifyWindow->Create(nullptr, session));
    notifyWindow->NotifyAfterLifecycleResumed();
    EXPECT_EQ(false, notifyWindow->isInteractiveStateFlag_);
    notifyWindow->state_ = WindowState::STATE_CREATED;
    notifyWindow->isDidForeground_ = true;
    notifyWindow->NotifyAfterLifecycleResumed();
    notifyWindow->state_ = WindowState::STATE_SHOWN;
    notifyWindow->isDidForeground_ = false;
    notifyWindow->NotifyAfterLifecycleResumed();

    notifyWindow->state_ = WindowState::STATE_CREATED;
    notifyWindow->isDidForeground_ = false;
    notifyWindow->NotifyAfterLifecycleResumed();
    notifyWindow->state_ = WindowState::STATE_SHOWN;
    notifyWindow->isDidForeground_ = true;
    notifyWindow->NotifyAfterLifecycleResumed();
    EXPECT_EQ(true, notifyWindow->isInteractiveStateFlag_);
    notifyWindow->NotifyAfterLifecycleResumed();

    ASSERT_NE(nullptr, notifyWindow->property_);
    notifyWindow->property_->SetPersistentId(1);
    notifyWindow->property_->SetUseControlState(true);
    notifyWindow->NotifyAfterLifecycleResumed();
    WSError res = notifyWindow->NotifyCloseExistPipWindow();
    EXPECT_EQ(res, WSError::WS_OK);
    EXPECT_EQ(WMError::WM_ERROR_INVALID_WINDOW, notifyWindow->Destroy());
}

/**
 * @tc.name: Notify03
 * @tc.desc: NotifyCloseExistPipWindow NotifyAfterLifecyclePaused
 * @tc.type: FUNC
 */
 HWTEST_F(WindowSessionImplTest, NotifyAfterLifecyclePaused, TestSize.Level1)
{
    sptr<WindowOption> pauseOption = sptr<WindowOption>::MakeSptr();
    ASSERT_NE(nullptr, pauseOption);
    pauseOption->SetWindowName("NotifyAfterLifecyclePaused");
    sptr<WindowSessionImpl> pauseWindow = sptr<WindowSessionImpl>::MakeSptr(pauseOption);
    ASSERT_NE(nullptr, pauseWindow);
    SessionInfo sessionInfo = { "CreateTestBundle", "CreateTestModule", "CreateTestAbility" };
    sptr<SessionMocker> session = sptr<SessionMocker>::MakeSptr(sessionInfo);
    EXPECT_EQ(WMError::WM_OK, pauseWindow->Create(nullptr, session));
    pauseWindow->NotifyAfterLifecycleResumed();
    pauseWindow->NotifyAfterLifecyclePaused();
    EXPECT_EQ(false, pauseWindow->isInteractiveStateFlag_);
    pauseWindow->NotifyAfterLifecyclePaused();
    WSError res = pauseWindow->NotifyCloseExistPipWindow();
    EXPECT_EQ(res, WSError::WS_OK);
    EXPECT_EQ(WMError::WM_ERROR_INVALID_WINDOW, pauseWindow->Destroy());
}

/**
 * @tc.name: Notify04
 * @tc.desc: NotifyAppUseControlStatus
 * @tc.type: FUNC
 */
 HWTEST_F(WindowSessionImplTest, NotifyAppUseControlStatusWhenSetStateThenReturnOK, TestSize.Level1)
{
    sptr<WindowOption> appOption = sptr<WindowOption>::MakeSptr();
    ASSERT_NE(nullptr, appOption);
    appOption->SetWindowName("NotifyAppUseControlStatus");
    sptr<WindowSessionImpl> window = sptr<WindowSessionImpl>::MakeSptr(appOption);
    ASSERT_NE(nullptr, window);
    SessionInfo sessionInfo = { "CreateTestBundle", "CreateTestModule", "CreateTestAbility" };
    sptr<SessionMocker> session = sptr<SessionMocker>::MakeSptr(sessionInfo);
    EXPECT_EQ(WMError::WM_OK, window->Create(nullptr, session));

    window->property_->SetPersistentId(1);
    window->hostSession_ = session;
    window->state_ = WindowState::STATE_HIDDEN;
    window->NotifyAppUseControlStatus(true);

    window->state_ = WindowState::STATE_SHOWN;
    window->NotifyAppUseControlStatus(true);

    EXPECT_FALSE(window->IsWindowSessionInvalid());
    window->state_ = WindowState::STATE_HIDDEN;
    window->NotifyAppUseControlStatus(true);

    window->state_ = WindowState::STATE_SHOWN;
    window->NotifyAppUseControlStatus(true);
    window->NotifyAppUseControlStatus(false);

    WSError res = window->NotifyCloseExistPipWindow();
    EXPECT_EQ(res, WSError::WS_OK);
    EXPECT_EQ(WMError::WM_OK, window->Destroy());
}

/**
 * @tc.name: MarkProcessed
 * @tc.desc: MarkProcessed
 * @tc.type: FUNC
 */
HWTEST_F(WindowSessionImplTest, MarkProcessed, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "WindowSessionImplTest: MarkProcessed start";
    sptr<WindowOption> option = sptr<WindowOption>::MakeSptr();
    option->SetWindowName("MarkProcessed");
    sptr<WindowSessionImpl> window = sptr<WindowSessionImpl>::MakeSptr(option);

    SessionInfo sessionInfo = { "CreateTestBundle", "CreateTestModule", "CreateTestAbility" };
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
HWTEST_F(WindowSessionImplTest, Notify01, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "WindowSessionImplTest: Notify01 start";
    sptr<WindowOption> option = sptr<WindowOption>::MakeSptr();
    option->SetWindowName("Notify01");
    sptr<WindowSessionImpl> window = sptr<WindowSessionImpl>::MakeSptr(option);

    SessionInfo sessionInfo = { "CreateTestBundle", "CreateTestModule", "CreateTestAbility" };
    sptr<SessionMocker> session = new (std::nothrow) SessionMocker(sessionInfo);
    ASSERT_NE(nullptr, session);
    ASSERT_EQ(WMError::WM_OK, window->Create(nullptr, session));

    window->NotifyTouchDialogTarget();
    window->uiContent_ = nullptr;
    window->NotifyScreenshot();
    window->uiContent_ = std::make_unique<Ace::UIContentMocker>();
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
HWTEST_F(WindowSessionImplTest, NotifyKeyEvent, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "WindowSessionImplTest: NotifyKeyEvent start";
    sptr<WindowOption> option = sptr<WindowOption>::MakeSptr();
    option->SetWindowName("NotifyKeyEvent");
    sptr<WindowSessionImpl> window = sptr<WindowSessionImpl>::MakeSptr(option);

    SessionInfo sessionInfo = { "CreateTestBundle", "CreateTestModule", "CreateTestAbility" };
    sptr<SessionMocker> session = sptr<SessionMocker>::MakeSptr(sessionInfo);
    ASSERT_EQ(WMError::WM_OK, window->Create(nullptr, session));

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
    ASSERT_EQ(WMError::WM_ERROR_INVALID_WINDOW, window->Destroy());
    GTEST_LOG_(INFO) << "WindowSessionImplTest: NotifyKeyEvent end";
}

/**
 * @tc.name: UpdateProperty01
 * @tc.desc: UpdateProperty
 * @tc.type: FUNC
 */
HWTEST_F(WindowSessionImplTest, UpdateProperty01, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "WindowSessionImplTest: UpdateProperty01 start";
    sptr<WindowOption> option = sptr<WindowOption>::MakeSptr();
    option->SetWindowName("UpdateProperty01");
    sptr<WindowSessionImpl> window = sptr<WindowSessionImpl>::MakeSptr(option);
    SessionInfo sessionInfo = { "CreateTestBundle", "CreateTestModule", "CreateTestAbility" };
    sptr<SessionMocker> session = sptr<SessionMocker>::MakeSptr(sessionInfo);
    ASSERT_EQ(WMError::WM_OK, window->Create(nullptr, session));

    ASSERT_NE(nullptr, window->property_);
    window->property_->SetPersistentId(1);
    WMError res = window->UpdateProperty(WSPropertyChangeAction::ACTION_UPDATE_RECT);
    ASSERT_EQ(res, WMError::WM_ERROR_INVALID_WINDOW);
    ASSERT_EQ(WMError::WM_ERROR_INVALID_WINDOW, window->Destroy());
}

/**
 * @tc.name: UpdateProperty02
 * @tc.desc: UpdateProperty
 * @tc.type: FUNC
 */
HWTEST_F(WindowSessionImplTest, UpdateProperty02, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "WindowSessionImplTest: UpdateProperty02 start";
    sptr<WindowOption> option = sptr<WindowOption>::MakeSptr();
    option->SetWindowName("UpdateProperty02");

    // session is null
    sptr<WindowSessionImpl> window = sptr<WindowSessionImpl>::MakeSptr(option);
    ASSERT_EQ(WMError::WM_OK, window->Create(abilityContext_, nullptr));
    WMError res = window->UpdateProperty(WSPropertyChangeAction::ACTION_UPDATE_RECT);
    ASSERT_EQ(res, WMError::WM_ERROR_INVALID_WINDOW);
    ASSERT_EQ(WMError::WM_ERROR_INVALID_WINDOW, window->Destroy());
    GTEST_LOG_(INFO) << "WindowSessionImplTest: UpdateProperty02 end";
}

/**
 * @tc.name: Find
 * @tc.desc: Find
 * @tc.type: FUNC
 */
HWTEST_F(WindowSessionImplTest, Find, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "WindowSessionImplTest: Find start";
    sptr<WindowOption> option = sptr<WindowOption>::MakeSptr();
    option->SetWindowName("Find");
    sptr<WindowSessionImpl> window = sptr<WindowSessionImpl>::MakeSptr(option);

    SessionInfo sessionInfo = { "CreateTestBundle", "CreateTestModule", "CreateTestAbility" };
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
HWTEST_F(WindowSessionImplTest, SetBackgroundColor01, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "WindowSessionImplTest: SetBackgroundColor01 start";
    sptr<WindowOption> option = sptr<WindowOption>::MakeSptr();
    option->SetWindowName("SetBackgroundColor01");
    sptr<WindowSessionImpl> window = sptr<WindowSessionImpl>::MakeSptr(option);

    SessionInfo sessionInfo = { "CreateTestBundle", "CreateTestModule", "CreateTestAbility" };
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
    ASSERT_EQ(WMError::WM_ERROR_INVALID_WINDOW, window->Destroy());
    GTEST_LOG_(INFO) << "WindowSessionImplTest: SetBackgroundColor01 end";
}

/**
 * @tc.name: SetBackgroundColor02
 * @tc.desc: SetBackgroundColor(uint32_t) and GetBackgroundColor
 * @tc.type: FUNC
 */
HWTEST_F(WindowSessionImplTest, SetBackgroundColor02, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "WindowSessionImplTest: SetBackgroundColor02 start";
    sptr<WindowOption> option = sptr<WindowOption>::MakeSptr();
    option->SetWindowName("SetBackgroundColor02");
    sptr<WindowSessionImpl> window = sptr<WindowSessionImpl>::MakeSptr(option);

    SessionInfo sessionInfo = { "CreateTestBundle", "CreateTestModule", "CreateTestAbility" };
    sptr<SessionMocker> session = new (std::nothrow) SessionMocker(sessionInfo);
    ASSERT_NE(nullptr, session);
    ASSERT_EQ(WMError::WM_OK, window->Create(nullptr, session));

    WMError res = window->SetBackgroundColor(0xffffffff);
    ASSERT_EQ(res, WMError::WM_ERROR_INVALID_OPERATION);
    uint32_t ret = window->GetBackgroundColor();
    ASSERT_EQ(ret, 0xffffffff);
    ASSERT_EQ(WMError::WM_ERROR_INVALID_WINDOW, window->Destroy());
    GTEST_LOG_(INFO) << "WindowSessionImplTest: SetBackgroundColor02 end";
}

/**
 * @tc.name: SetAPPWindowIcon
 * @tc.desc: SetAPPWindowIcon
 * @tc.type: FUNC
 */
HWTEST_F(WindowSessionImplTest, SetAPPWindowIcon, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "WindowSessionImplTest: SetAPPWindowIcon start";
    sptr<WindowOption> option = sptr<WindowOption>::MakeSptr();
    option->SetWindowName("SetAPPWindowIcon");
    sptr<WindowSessionImpl> window = sptr<WindowSessionImpl>::MakeSptr(option);

    SessionInfo sessionInfo = { "CreateTestBundle", "CreateTestModule", "CreateTestAbility" };
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
    opts.size.width = 200;  // 200 test width
    opts.size.height = 300; // 300 test height
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
HWTEST_F(WindowSessionImplTest, Notify02, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "WindowSessionImplTest: Notify02 start";
    sptr<WindowOption> option = sptr<WindowOption>::MakeSptr();
    option->SetWindowName("Notify02");
    sptr<WindowSessionImpl> window = sptr<WindowSessionImpl>::MakeSptr(option);

    SessionInfo sessionInfo = { "CreateTestBundle", "CreateTestModule", "CreateTestAbility" };
    sptr<SessionMocker> session = sptr<SessionMocker>::MakeSptr(sessionInfo);
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

    bool enable = false;
    window->NotifySwitchFreeMultiWindow(enable);

    ASSERT_EQ(WMError::WM_ERROR_INVALID_WINDOW, window->Destroy());
    GTEST_LOG_(INFO) << "WindowSessionImplTest: Notify02 end";
}

/**
 * @tc.name: SetAceAbilityHandler
 * @tc.desc: SetAceAbilityHandler
 * @tc.type: FUNC
 */
HWTEST_F(WindowSessionImplTest, SetAceAbilityHandler, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "WindowSessionImplTest: SetAceAbilityHandler start";
    sptr<WindowOption> option = sptr<WindowOption>::MakeSptr();
    option->SetWindowName("SetAceAbilityHandler");
    sptr<WindowSessionImpl> window = sptr<WindowSessionImpl>::MakeSptr(option);

    SessionInfo sessionInfo = { "CreateTestBundle", "CreateTestModule", "CreateTestAbility" };
    sptr<SessionMocker> session = sptr<SessionMocker>::MakeSptr(sessionInfo);
    ASSERT_EQ(WMError::WM_OK, window->Create(nullptr, session));

    sptr<IAceAbilityHandler> handler = sptr<IAceAbilityHandler>();
    ASSERT_EQ(handler, nullptr);
    GTEST_LOG_(INFO) << "WindowSessionImplTest: SetAceAbilityHandler 111";
    window->SetAceAbilityHandler(handler);
    ASSERT_EQ(WMError::WM_ERROR_INVALID_WINDOW, window->Destroy());
    GTEST_LOG_(INFO) << "WindowSessionImplTest: SetAceAbilityHandler end";
}

/**
 * @tc.name: SetRaiseByClickEnabled01
 * @tc.desc: SetRaiseByClickEnabled and check the retCode
 * @tc.type: FUNC
 */
HWTEST_F(WindowSessionImplTest, SetRaiseByClickEnabled01, TestSize.Level1)
{
    sptr<WindowOption> option = sptr<WindowOption>::MakeSptr();
    option->SetWindowName("SetRaiseByClickEnabled01");
    option->SetWindowType(WindowType::WINDOW_TYPE_APP_MAIN_WINDOW);
    sptr<WindowSessionImpl> window = new (std::nothrow) WindowSessionImpl(option);
    ASSERT_NE(nullptr, window);

    WMError retCode = window->SetRaiseByClickEnabled(true);
    ASSERT_EQ(retCode, WMError::WM_ERROR_INVALID_PARENT);

    option->SetWindowName("SetRaiseByClickForFloatWindow");
    option->SetWindowType(WindowType::WINDOW_TYPE_FLOAT);
    sptr<WindowSessionImpl> floatWindow = new (std::nothrow) WindowSessionImpl(option);
    floatWindow->property_->SetParentPersistentId(1);

    retCode = floatWindow->SetRaiseByClickEnabled(true);
    ASSERT_EQ(retCode, WMError::WM_ERROR_INVALID_CALLING);

    option->SetWindowName("SetRaiseByClickForSubWindow");
    option->SetWindowType(WindowType::WINDOW_TYPE_APP_SUB_WINDOW);
    sptr<WindowSessionImpl> subWindow = new (std::nothrow) WindowSessionImpl(option);
    ASSERT_NE(nullptr, subWindow);

    subWindow->property_->SetParentPersistentId(1);
    subWindow->Hide();
    retCode = subWindow->SetRaiseByClickEnabled(true);
    ASSERT_EQ(retCode, WMError::WM_DO_NOTHING);

    subWindow->state_ = WindowState::STATE_SHOWN;
    retCode = subWindow->SetRaiseByClickEnabled(true);
    ASSERT_EQ(retCode, WMError::WM_ERROR_INVALID_WINDOW);

    subWindow->property_->SetParentPersistentId(2);
    subWindow->SetRaiseByClickEnabled(true);
    ASSERT_EQ(subWindow->property_->GetRaiseEnabled(), true);
}

/**
 * @tc.name: SetMainWindowRaiseByClickEnabled
 * @tc.desc: SetMainWindowRaiseByClickEnabled test
 * @tc.type: FUNC
 */
HWTEST_F(WindowSessionImplTest, SetMainWindowRaiseByClickEnabled, TestSize.Level1)
{
    sptr<WindowOption> option = sptr<WindowOption>::MakeSptr();
    option->SetWindowName("SetMainWindowRaiseByClickEnabled");
    option->SetWindowType(WindowType::WINDOW_TYPE_FLOAT);
    sptr<WindowSessionImpl> window = new (std::nothrow) WindowSessionImpl(option);
    ASSERT_NE(nullptr, window);

    WMError retCode = window->SetMainWindowRaiseByClickEnabled(true);
    EXPECT_EQ(retCode, WMError::WM_ERROR_INVALID_CALLING);

    window->property_->SetWindowType(WindowType::WINDOW_TYPE_APP_MAIN_WINDOW);
    window->state_ = WindowState::STATE_CREATED;
    retCode = window->SetMainWindowRaiseByClickEnabled(true);
    EXPECT_EQ(retCode, WMError::WM_DO_NOTHING);

    window->state_ = WindowState::STATE_SHOWN;
    window->property_->SetPersistentId(0);
    retCode = window->SetMainWindowRaiseByClickEnabled(true);
    EXPECT_EQ(retCode, WMError::WM_ERROR_INVALID_WINDOW);

    window->property_->SetPersistentId(1);
    SessionInfo sessionInfo;
    sptr<SessionMocker> session = sptr<SessionMocker>::MakeSptr(sessionInfo);
    window->hostSession_ = session;
    window->SetMainWindowRaiseByClickEnabled(false);
    EXPECT_EQ(window->property_->GetRaiseEnabled(), false);
}

/**
 * @tc.name: HideNonSystemFloatingWindows01
 * @tc.desc: HideNonSystemFloatingWindows and check the retCode
 * @tc.type: FUNC
 */
HWTEST_F(WindowSessionImplTest, HideNonSystemFloatingWindows01, TestSize.Level1)
{
    sptr<WindowOption> option = sptr<WindowOption>::MakeSptr();
    option->SetWindowName("HideNonSystemFloatingWindows01");
    sptr<WindowSessionImpl> window = new (std::nothrow) WindowSessionImpl(option);
    ASSERT_NE(nullptr, window);
    WMError retCode = window->HideNonSystemFloatingWindows(false);
    ASSERT_EQ(retCode, WMError::WM_ERROR_INVALID_WINDOW);
    window->property_->SetPersistentId(1);
    SessionInfo sessionInfo = { "CreateTestBundle", "CreateTestModule", "CreateTestAbility" };
    sptr<SessionMocker> session = new (std::nothrow) SessionMocker(sessionInfo);
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
HWTEST_F(WindowSessionImplTest, UpdateWindowMode, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "WindowSessionImplTest: UpdateWindowModetest01 start";
    sptr<WindowOption> option = sptr<WindowOption>::MakeSptr();
    ASSERT_NE(option, nullptr);
    option->SetWindowName("UpdateWindowMode");
    sptr<WindowSessionImpl> window = new (std::nothrow) WindowSessionImpl(option);
    ASSERT_NE(window, nullptr);
    WindowMode mode = WindowMode{ 0 };
    auto ret = window->UpdateWindowMode(mode);
    ASSERT_EQ(ret, WSError::WS_OK);
    GTEST_LOG_(INFO) << "WindowSessionImplTest: UpdateWindowModetest01 end";
}

/**
 * @tc.name: UpdateDensitytest01
 * @tc.desc: UpdateDensity
 * @tc.type: FUNC
 */
HWTEST_F(WindowSessionImplTest, UpdateDensity, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "WindowSessionImplTest: UpdateDensitytest01 start";
    sptr<WindowOption> option = sptr<WindowOption>::MakeSptr();
    option->SetWindowName("UpdateDensity");
    sptr<WindowSessionImpl> window = new (std::nothrow) WindowSessionImpl(option);
    ASSERT_NE(window, nullptr);
    ASSERT_NE(window->property_, nullptr);

    Rect rect1;
    rect1.posX_ = 1;
    rect1.posY_ = 2;
    rect1.height_ = 3;
    rect1.width_ = 4;
    window->property_->SetWindowRect(rect1);
    auto rect2 = window->GetRect();
    ASSERT_EQ(1, rect2.posX_);
    ASSERT_EQ(2, rect2.posY_);
    ASSERT_EQ(3, rect2.height_);
    ASSERT_EQ(4, rect2.width_);

    window->UpdateDensity();
    GTEST_LOG_(INFO) << "WindowSessionImplTest: UpdateDensitytest01 end";
}

/**
 * @tc.name: UpdateDisplayIdtest01
 * @tc.desc: UpdateDisplayId
 * @tc.type: FUNC
 */
HWTEST_F(WindowSessionImplTest, UpdateDisplayId, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "WindowSessionImplTest: UpdateDisplayIdtest01 start";
    sptr<WindowOption> option = sptr<WindowOption>::MakeSptr();
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
HWTEST_F(WindowSessionImplTest, IsFloatingWindowAppType, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "WindowSessionImplTest: IsFloatingWindowAppTypetest01 start";
    sptr<WindowOption> option = sptr<WindowOption>::MakeSptr();
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
HWTEST_F(WindowSessionImplTest, SetUniqueVirtualPixelRatio, TestSize.Level1)
{
    sptr<WindowOption> option = new (std::nothrow) WindowOption();
    ASSERT_NE(option, nullptr);
    option->SetWindowName("SetUniqueVirtualPixelRatio");
    sptr<WindowSessionImpl> window = new (std::nothrow) WindowSessionImpl(option);
    ASSERT_NE(window, nullptr);
    window->SetUniqueVirtualPixelRatio(true, 3.25f);
    window->SetUniqueVirtualPixelRatio(false, 3.25f);
}

/**
 * @tc.name: UpdateAnimationSpeed
 * @tc.desc: UpdateAnimationSpeed
 * @tc.type: FUNC
 */
HWTEST_F(WindowSessionImplTest, UpdateAnimationSpeed, TestSize.Level1)
{
    sptr<WindowOption> option = new (std::nothrow) WindowOption();
    ASSERT_NE(option, nullptr);
    option->SetWindowName("UpdateAnimationSpeed");
    sptr<WindowSessionImpl> window = new (std::nothrow) WindowSessionImpl(option);
    ASSERT_NE(window, nullptr);
    window->UpdateAnimationSpeed(1.0f);
    window->UpdateAnimationSpeed(2.0f);
    usleep(WAIT_SYNC_IN_NS);
}

/**
 * @tc.name: UpdateAllWindowSpeed
 * @tc.desc: UpdateAllWindowSpeed
 * @tc.type: FUNC
 */
HWTEST_F(WindowSessionImplTest, UpdateAllWindowSpeed, TestSize.Level1)
{
    g_errLog.clear();
    LOG_SetCallback(MyLogCallback);
    sptr<WindowOption> option = new (std::nothrow) WindowOption();
    ASSERT_NE(option, nullptr);
    option->SetWindowName("UpdateAllWindowSpeed");
    sptr<WindowSessionImpl> window = new (std::nothrow) WindowSessionImpl(option);
    ASSERT_NE(window, nullptr);
    window->UpdateAllWindowSpeed(2.0f);
    WindowSessionImpl::windowSessionMap_.clear();
    window->windowSessionMap_.insert(std::make_pair("test1", std::make_pair(1, nullptr)));
    window->UpdateAllWindowSpeed(2.0f);
    WindowSessionImpl::windowSessionMap_.clear();
    window->windowSessionMap_.insert(std::make_pair("test2", std::make_pair(1, window)));
    window->rsUIDirector_ = nullptr;
    window->UpdateAllWindowSpeed(2.0f);
    EXPECT_TRUE(g_errLog.find("Failed to open implicit animation") != std::string::npos);
}

/**
 * @tc.name: EnableDrag
 * @tc.desc: EnableDrag Test
 * @tc.type: FUNC
 */
HWTEST_F(WindowSessionImplTest, EnableDrag, TestSize.Level1)
{
    sptr<WindowOption> option = sptr<WindowOption>::MakeSptr();
    option->SetWindowName("EnableDrag");
    sptr<WindowSessionImpl> window = sptr<WindowSessionImpl>::MakeSptr(option);
    window->property_->SetPersistentId(2101);
    window->property_->SetWindowType(WindowType::WINDOW_TYPE_GLOBAL_SEARCH);
    SessionInfo sessionInfo = { "CreateTestBundle", "CreateTestModule", "CreateTestAbility" };
    sptr<SessionMocker> session = sptr<SessionMocker>::MakeSptr(sessionInfo);
    window->hostSession_ = session;

    window->windowSystemConfig_.windowUIType_ = WindowUIType::INVALID_WINDOW;
    auto result = window->EnableDrag(true);
    ASSERT_EQ(result, WMError::WM_ERROR_DEVICE_NOT_SUPPORT);

    window->windowSystemConfig_.windowUIType_ = WindowUIType::PHONE_WINDOW;
    window->property_->type_ = WindowType::WINDOW_TYPE_APP_MAIN_WINDOW;
    result = window->EnableDrag(true);
    ASSERT_EQ(result, WMError::WM_ERROR_INVALID_CALLING);

    window->windowSystemConfig_.windowUIType_ = WindowUIType::PC_WINDOW;
    window->property_->type_ = WindowType::WINDOW_TYPE_APP_SUB_WINDOW;
    result = window->EnableDrag(true);
    ASSERT_NE(result, WMError::WM_ERROR_DEVICE_NOT_SUPPORT);
}

/**
 * @tc.name: AddSetUIContentTimeoutCheck
 * @tc.desc: AddSetUIContentTimeoutCheck
 * @tc.type: FUNC
 */
HWTEST_F(WindowSessionImplTest, AddSetUIContentTimeoutCheck_test, TestSize.Level1)
{
    sptr<WindowOption> option = new (std::nothrow) WindowOption();
    ASSERT_NE(option, nullptr);
    sptr<WindowSessionImpl> window = new (std::nothrow) WindowSessionImpl(option);
    ASSERT_NE(window, nullptr);
    window->handler_ = nullptr;

    option->SetWindowName("AddSetUIContentTimeoutCheck_test");
    option->SetBundleName("UTtest");
    WindowType type1 = WindowType::APP_MAIN_WINDOW_BASE;
    option->SetWindowType(type1);
    sptr<WindowSessionImpl> window1 = new (std::nothrow) WindowSessionImpl(option);
    ASSERT_NE(window1, nullptr);
    window1->AddSetUIContentTimeoutCheck();

    WindowType type2 = WindowType::WINDOW_TYPE_UI_EXTENSION;
    option->SetWindowType(type2);
    sptr<WindowSessionImpl> window2 = new (std::nothrow) WindowSessionImpl(option);
    ASSERT_NE(window2, nullptr);
    window2->AddSetUIContentTimeoutCheck();
    EXPECT_EQ(WindowType::WINDOW_TYPE_UI_EXTENSION, window2->property_->GetWindowType());
}

/**
 * @tc.name: FindMainWindowWithContext01
 * @tc.desc: FindMainWindowWithContext
 * @tc.type: FUNC
 */
HWTEST_F(WindowSessionImplTest, FindMainWindowWithContext01, TestSize.Level1)
{
    sptr<WindowOption> option = new (std::nothrow) WindowOption();
    option->SetWindowTag(WindowTag::MAIN_WINDOW);
    option->SetWindowName("FindMainWindowWithContext01");
    sptr<WindowSessionImpl> windowSession = new (std::nothrow) WindowSessionImpl(option);
    ASSERT_NE(nullptr, windowSession);

    windowSession->SetWindowType(WindowType::WINDOW_TYPE_DIALOG);
    ASSERT_TRUE(windowSession->FindMainWindowWithContext() == nullptr);
    windowSession->SetWindowType(WindowType::ABOVE_APP_SYSTEM_WINDOW_END);
    ASSERT_TRUE(windowSession->FindMainWindowWithContext() == nullptr);

    windowSession->property_->SetPersistentId(1002);
    windowSession->property_->SetWindowType(WindowType::WINDOW_TYPE_APP_MAIN_WINDOW);
    SessionInfo sessionInfo = { "CreateTestBundle", "CreateTestModule", "CreateTestAbility" };
    sptr<SessionMocker> session = new (std::nothrow) SessionMocker(sessionInfo);
    ASSERT_NE(nullptr, session);
    ASSERT_EQ(WMError::WM_OK, windowSession->Create(abilityContext_, session));
    windowSession->Destroy(true);
}

/**
 * @tc.name: FindExtensionWindowWithContext01
 * @tc.desc: FindExtensionWindowWithContext
 * @tc.type: FUNC
 */
HWTEST_F(WindowSessionImplTest, FindExtensionWindowWithContext01, TestSize.Level1)
{
    sptr<WindowOption> option = sptr<WindowOption>::MakeSptr();
    option->SetWindowName("FindExtensionWindowWithContext01");
    sptr<WindowSessionImpl> windowSession = sptr<WindowSessionImpl>::MakeSptr(option);

    ASSERT_TRUE(windowSession->FindExtensionWindowWithContext() == nullptr);

    windowSession->property_->SetPersistentId(12345);
    windowSession->property_->SetWindowType(WindowType::WINDOW_TYPE_UI_EXTENSION);
    windowSession->context_ = abilityContext_;
    WindowSessionImpl::GetWindowExtensionSessionSet().insert(windowSession);
    ASSERT_TRUE(nullptr != windowSession->FindExtensionWindowWithContext());
    windowSession->Destroy(true);
}

/**
 * @tc.name: SetUIContentComplete
 * @tc.desc: SetUIContentComplete
 * @tc.type: FUNC
 */
HWTEST_F(WindowSessionImplTest, SetUIContentComplete, TestSize.Level1)
{
    sptr<WindowOption> option = new (std::nothrow) WindowOption();
    ASSERT_NE(option, nullptr);
    sptr<WindowSessionImpl> window = new (std::nothrow) WindowSessionImpl(option);
    ASSERT_NE(window, nullptr);
    window->SetUIContentComplete();
    EXPECT_EQ(window->setUIContentCompleted_.load(), true);

    window->SetUIContentComplete();
    EXPECT_EQ(window->setUIContentCompleted_.load(), true);
}

/**
 * @tc.name: NotifySetUIContentComplete
 * @tc.desc: NotifySetUIContentComplete
 * @tc.type: FUNC
 */
HWTEST_F(WindowSessionImplTest, NotifySetUIContentComplete, TestSize.Level1)
{
    sptr<WindowOption> option = new (std::nothrow) WindowOption();
    ASSERT_NE(nullptr, option);
    option->SetWindowName("NotifySetUIContent");
    option->SetWindowType(WindowType::APP_MAIN_WINDOW_BASE);
    sptr<WindowSessionImpl> window = new (std::nothrow) WindowSessionImpl(option);
    ASSERT_NE(nullptr, window);
    window->NotifySetUIContentComplete();
    EXPECT_EQ(window->setUIContentCompleted_.load(), true);

    option->SetWindowType(WindowType::APP_SUB_WINDOW_BASE);
    window = new (std::nothrow) WindowSessionImpl(option);
    ASSERT_NE(nullptr, window);
    window->NotifySetUIContentComplete();
    EXPECT_EQ(window->setUIContentCompleted_.load(), true);

    option->SetWindowType(WindowType::SYSTEM_WINDOW_BASE);
    window = new (std::nothrow) WindowSessionImpl(option);
    ASSERT_NE(nullptr, window);
    window->NotifySetUIContentComplete();
    EXPECT_EQ(window->setUIContentCompleted_.load(), true);
}

/**
 * @tc.name: SetUIExtensionDestroyComplete
 * @tc.desc: SetUIExtensionDestroyComplete
 * @tc.type: FUNC
 */
HWTEST_F(WindowSessionImplTest, SetUIExtensionDestroyComplete, TestSize.Level1)
{
    sptr<WindowOption> option = sptr<WindowOption>::MakeSptr();
    sptr<WindowSessionImpl> window = sptr<WindowSessionImpl>::MakeSptr(option);
    window->SetUIExtensionDestroyComplete();
    EXPECT_EQ(window->setUIExtensionDestroyCompleted_.load(), true);
    window->SetUIExtensionDestroyComplete();
    EXPECT_EQ(window->setUIExtensionDestroyCompleted_.load(), true);
}

/**
 * @tc.name: SetUIExtensionDestroyCompleteInSubWindow
 * @tc.desc: SetUIExtensionDestroyCompleteInSubWindow
 * @tc.type: FUNC
 */
HWTEST_F(WindowSessionImplTest, SetUIExtensionDestroyCompleteInSubWindow, TestSize.Level1)
{
    sptr<WindowOption> subWindowOption = sptr<WindowOption>::MakeSptr();
    subWindowOption->SetWindowName("subWindow");
    sptr<WindowSessionImpl> subWindowSession = sptr<WindowSessionImpl>::MakeSptr(subWindowOption);
    subWindowSession->property_->SetWindowType(WindowType::WINDOW_TYPE_APP_SUB_WINDOW);
    subWindowSession->context_ = abilityContext_;
    subWindowSession->SetUIExtensionDestroyCompleteInSubWindow();

    sptr<WindowOption> option = sptr<WindowOption>::MakeSptr();
    option->SetWindowName("SetUIExtensionDestroyCompleteInSubWindow");
    sptr<WindowSessionImpl> windowSession = sptr<WindowSessionImpl>::MakeSptr(option);
    ASSERT_TRUE(windowSession->FindExtensionWindowWithContext() == nullptr);
    windowSession->property_->SetPersistentId(12345);
    windowSession->property_->SetWindowType(WindowType::WINDOW_TYPE_UI_EXTENSION);
    windowSession->context_ = abilityContext_;
    WindowSessionImpl::GetWindowExtensionSessionSet().insert(windowSession);
    ASSERT_TRUE(nullptr != windowSession->FindExtensionWindowWithContext());
    windowSession->AddSetUIExtensionDestroyTimeoutCheck();
    EXPECT_EQ(windowSession->startUIExtensionDestroyTimer_.load(), true);
    subWindowSession->SetUIExtensionDestroyCompleteInSubWindow();
    EXPECT_EQ(windowSession->startUIExtensionDestroyTimer_.load(), false);
}

/**
 * @tc.name: AddSetUIExtensionDestroyTimeoutCheck
 * @tc.desc: AddSetUIExtensionDestroyTimeoutCheck
 * @tc.type: FUNC
 */
HWTEST_F(WindowSessionImplTest, AddSetUIExtensionDestroyTimeoutCheck, TestSize.Level1)
{
    sptr<WindowOption> option = sptr<WindowOption>::MakeSptr();
    option->SetWindowName("AddSetUIExtensionDestroyTimeoutCheck");
    option->SetBundleName("UTtest");
    option->SetWindowType(WindowType::WINDOW_TYPE_UI_EXTENSION);
    sptr<WindowSessionImpl> window = sptr<WindowSessionImpl>::MakeSptr(option);
    window->AddSetUIExtensionDestroyTimeoutCheck();
    EXPECT_EQ(WindowType::WINDOW_TYPE_UI_EXTENSION, window->property_->GetWindowType());
    EXPECT_EQ(window->startUIExtensionDestroyTimer_.load(), true);
}

/**
 * @tc.name: GetStatusBarHeight
 * @tc.desc: GetStatusBarHeight test
 * @tc.type: FUNC
 */
HWTEST_F(WindowSessionImplTest, GetStatusBarHeight, TestSize.Level1)
{
    sptr<WindowOption> option = sptr<WindowOption>::MakeSptr();
    option->SetWindowName("GetStatusBarHeight");
    sptr<WindowSessionImpl> window = sptr<WindowSessionImpl>::MakeSptr(option);
    ASSERT_NE(window, nullptr);
    ASSERT_EQ(0, window->GetStatusBarHeight());
}

/**
 * @tc.name: GetWindowStatusInner
 * @tc.desc: GetWindowStatusInner test
 * @tc.type: FUNC
 */
HWTEST_F(WindowSessionImplTest, GetWindowStatusInner, TestSize.Level1)
{
    sptr<WindowOption> option = sptr<WindowOption>::MakeSptr();
    option->SetWindowName("GetWindowStatusInner");
    sptr<WindowSessionImpl> window = sptr<WindowSessionImpl>::MakeSptr(option);
    window->property_->SetPersistentId(1);
    SessionInfo sessionInfo = { "CreateTestBundle", "CreateTestModule", "CreateTestAbility" };
    sptr<SessionMocker> session = sptr<SessionMocker>::MakeSptr(sessionInfo);
    window->hostSession_ = session;
    EXPECT_EQ(WindowStatus::WINDOW_STATUS_UNDEFINED, window->GetWindowStatusInner(WindowMode::WINDOW_MODE_PIP));

    window->SetTargetAPIVersion(14);
    window->windowSystemConfig_.windowUIType_ = WindowUIType::PC_WINDOW;
    EXPECT_EQ(WindowStatus::WINDOW_STATUS_FULLSCREEN, window->GetWindowStatusInner(WindowMode::WINDOW_MODE_FULLSCREEN));
    window->SetTargetAPIVersion(12);
    EXPECT_EQ(WindowStatus::WINDOW_STATUS_FULLSCREEN, window->GetWindowStatusInner(WindowMode::WINDOW_MODE_FULLSCREEN));

    EXPECT_EQ(WindowStatus::WINDOW_STATUS_SPLITSCREEN,
    window->GetWindowStatusInner(WindowMode::WINDOW_MODE_SPLIT_PRIMARY));
    EXPECT_EQ(WindowStatus::WINDOW_STATUS_SPLITSCREEN,
    window->GetWindowStatusInner(WindowMode::WINDOW_MODE_SPLIT_SECONDARY));

    window->property_->maximizeMode_ = MaximizeMode::MODE_AVOID_SYSTEM_BAR;
    EXPECT_EQ(WindowStatus::WINDOW_STATUS_MAXIMIZE, window->GetWindowStatusInner(WindowMode::WINDOW_MODE_FLOATING));
    window->property_->maximizeMode_ = MaximizeMode::MODE_FULL_FILL;
    EXPECT_EQ(WindowStatus::WINDOW_STATUS_FLOATING, window->GetWindowStatusInner(WindowMode::WINDOW_MODE_FLOATING));

    window->state_ = WindowState::STATE_HIDDEN;
    EXPECT_EQ(WindowStatus::WINDOW_STATUS_MINIMIZE, window->GetWindowStatusInner(WindowMode::WINDOW_MODE_PIP));
}

/**
 * @tc.name: GetBackgroundColor01
 * @tc.desc: GetBackgroundColor01 test
 * @tc.type: FUNC
 */
HWTEST_F(WindowSessionImplTest, GetBackgroundColor01, TestSize.Level1)
{
    sptr<WindowOption> option = sptr<WindowOption>::MakeSptr();
    option->SetWindowName("GetBackgroundColor01");
    sptr<WindowSessionImpl> window = sptr<WindowSessionImpl>::MakeSptr(option);
    window->uiContent_ = std::make_unique<Ace::UIContentMocker>();
    EXPECT_EQ(0, window->GetBackgroundColor());
}

/**
 * @tc.name: GetBackgroundColor02
 * @tc.desc: GetBackgroundColor02 test
 * @tc.type: FUNC
 */
HWTEST_F(WindowSessionImplTest, GetBackgroundColor02, TestSize.Level1)
{
    sptr<WindowOption> option = sptr<WindowOption>::MakeSptr();
    option->SetWindowName("GetBackgroundColor02");
    sptr<WindowSessionImpl> window = sptr<WindowSessionImpl>::MakeSptr(option);
    sptr<IAceAbilityHandler> handler = sptr<IAceAbilityHandler>();
    window->SetAceAbilityHandler(handler);
    EXPECT_EQ(0xffffffff, window->GetBackgroundColor());
}

/**
 * @tc.name: GetBackgroundColor03
 * @tc.desc: GetBackgroundColor03 test
 * @tc.type: FUNC
 */
HWTEST_F(WindowSessionImplTest, GetBackgroundColor03, TestSize.Level1)
{
    sptr<WindowOption> option = sptr<WindowOption>::MakeSptr();
    option->SetWindowName("GetBackgroundColor03");
    sptr<WindowSessionImpl> window = sptr<WindowSessionImpl>::MakeSptr(option);
    EXPECT_EQ(0xffffffff, window->GetBackgroundColor());
}

/**
 * @tc.name: GetExtensionConfig
 * @tc.desc: GetExtensionConfig test
 * @tc.type: FUNC
 */
HWTEST_F(WindowSessionImplTest, GetExtensionConfig, TestSize.Level1)
{
    sptr<WindowOption> option = sptr<WindowOption>::MakeSptr();
    option->SetWindowName("GetExtensionConfig");
    sptr<WindowSessionImpl> window = sptr<WindowSessionImpl>::MakeSptr(option);
    SessionInfo sessionInfo;
    window->hostSession_ = sptr<SessionMocker>::MakeSptr(sessionInfo);
    window->crossAxisState_ = CrossAxisState::STATE_CROSS;
    AAFwk::WantParams want;
    window->GetExtensionConfig(want);
    EXPECT_EQ(want.GetIntParam(Extension::CROSS_AXIS_FIELD, 0), static_cast<int32_t>(CrossAxisState::STATE_CROSS));
    window->crossAxisState_ = CrossAxisState::STATE_INVALID;
    window->GetExtensionConfig(want);
    EXPECT_EQ(want.GetIntParam(Extension::CROSS_AXIS_FIELD, 0), static_cast<int32_t>(CrossAxisState::STATE_INVALID));
    window->crossAxisState_ = CrossAxisState::STATE_NO_CROSS;
    window->GetExtensionConfig(want);
    EXPECT_EQ(want.GetIntParam(Extension::CROSS_AXIS_FIELD, 0), static_cast<int32_t>(CrossAxisState::STATE_NO_CROSS));
    window->crossAxisState_ = CrossAxisState::STATE_END;
    window->GetExtensionConfig(want);
    EXPECT_EQ(want.GetIntParam(Extension::CROSS_AXIS_FIELD, 0), static_cast<int32_t>(CrossAxisState::STATE_END));

    bool isHostWindowDelayRaiseEnabled = true;
    window->property_->SetWindowDelayRaiseEnabled(isHostWindowDelayRaiseEnabled);
    window->GetExtensionConfig(want);
    EXPECT_EQ(want.GetIntParam(Extension::HOST_WINDOW_DELAY_RAISE_STATE_FIELD, 0),
        static_cast<int32_t>(isHostWindowDelayRaiseEnabled));
    isHostWindowDelayRaiseEnabled = false;
    window->property_->SetWindowDelayRaiseEnabled(isHostWindowDelayRaiseEnabled);
    window->GetExtensionConfig(want);
    EXPECT_EQ(want.GetIntParam(Extension::HOST_WINDOW_DELAY_RAISE_STATE_FIELD, 0),
        static_cast<int32_t>(isHostWindowDelayRaiseEnabled));
}

/**
 * @tc.name: OnExtensionMessage
 * @tc.desc: OnExtensionMessage test
 * @tc.type: FUNC
 */
HWTEST_F(WindowSessionImplTest, OnExtensionMessage, TestSize.Level1)
{
    sptr<WindowOption> option = sptr<WindowOption>::MakeSptr();
    option->SetWindowName("OnExtensionMessage");
    sptr<WindowSessionImpl> window = sptr<WindowSessionImpl>::MakeSptr(option);
    uint32_t code = 9999;
    int32_t persistentId = 1111;
    AAFwk::Want want;
    auto ret = window->OnExtensionMessage(code, persistentId, want);
    EXPECT_EQ(WMError::WM_OK, ret);

    code = static_cast<uint32_t>(Extension::Businesscode::NOTIFY_HOST_WINDOW_TO_RAISE);
    window->hostSession_ = nullptr;
    ASSERT_EQ(nullptr, window->GetHostSession());
    ret = window->OnExtensionMessage(code, persistentId, want);
    EXPECT_EQ(WMError::WM_ERROR_INVALID_WINDOW, ret);
    SessionInfo sessionInfo;
    window->hostSession_ = sptr<SessionMocker>::MakeSptr(sessionInfo);
    window->property_->SetPersistentId(1);
    ASSERT_FALSE(window->GetPersistentId() == INVALID_SESSION_ID);
    window->state_ = WindowState::STATE_CREATED;
    ASSERT_FALSE(window->state_ == WindowState::STATE_DESTROYED);
    window->property_->SetWindowType(WindowType::WINDOW_TYPE_APP_MAIN_WINDOW);
    ASSERT_EQ(true, WindowHelper::IsAppWindow(window->GetType()));
    ret = window->OnExtensionMessage(code, persistentId, want);
    EXPECT_EQ(WMError::WM_OK, ret);

    code = static_cast<uint32_t>(Extension::Businesscode::REGISTER_HOST_WINDOW_RECT_CHANGE_LISTENER);
    EXPECT_EQ(WMError::WM_OK, window->OnExtensionMessage(code, persistentId, want));
    code = static_cast<uint32_t>(Extension::Businesscode::UNREGISTER_HOST_WINDOW_RECT_CHANGE_LISTENER);
    EXPECT_EQ(WMError::WM_OK, window->OnExtensionMessage(code, persistentId, want));
    code = static_cast<uint32_t>(Extension::Businesscode::REGISTER_HOST_RECT_CHANGE_IN_GLOBAL_DISPLAY_LISTENER);
    EXPECT_EQ(WMError::WM_OK, window->OnExtensionMessage(code, persistentId, want));
    code = static_cast<uint32_t>(Extension::Businesscode::UNREGISTER_HOST_RECT_CHANGE_IN_GLOBAL_DISPLAY_LISTENER);
    EXPECT_EQ(WMError::WM_OK, window->OnExtensionMessage(code, persistentId, want));
}

/**
 * @tc.name: OnExtensionMessage_KeyboardListener
 * @tc.desc: OnExtensionMessage_KeyboardListener test
 * @tc.type: FUNC
 */
HWTEST_F(WindowSessionImplTest, OnExtensionMessage_KeyboardListener, TestSize.Level1)
{
    sptr<WindowOption> option = sptr<WindowOption>::MakeSptr();
    option->SetWindowName("OnExtensionMessage");
    sptr<WindowSessionImpl> window = sptr<WindowSessionImpl>::MakeSptr(option);
    uint32_t code = 9999;
    int32_t persistentId = 1111;
    AAFwk::Want want;

    ASSERT_TRUE(window->keyboardDidShowUIExtListeners_.empty());
    code = static_cast<uint32_t>(Extension::Businesscode::REGISTER_KEYBOARD_DID_SHOW_LISTENER);
    EXPECT_EQ(WMError::WM_OK, window->OnExtensionMessage(code, persistentId, want));
    ASSERT_FALSE(window->keyboardDidShowUIExtListeners_.empty());
    EXPECT_EQ(WMError::WM_OK, window->OnExtensionMessage(code, persistentId, want));

    code = static_cast<uint32_t>(Extension::Businesscode::UNREGISTER_KEYBOARD_DID_SHOW_LISTENER);
    EXPECT_EQ(WMError::WM_OK, window->OnExtensionMessage(code, persistentId, want));
    ASSERT_TRUE(window->keyboardDidShowUIExtListeners_.empty());
    EXPECT_EQ(WMError::WM_OK, window->OnExtensionMessage(code, persistentId, want));

    ASSERT_TRUE(window->keyboardDidHideUIExtListeners_.empty());
    code = static_cast<uint32_t>(Extension::Businesscode::REGISTER_KEYBOARD_DID_HIDE_LISTENER);
    EXPECT_EQ(WMError::WM_OK, window->OnExtensionMessage(code, persistentId, want));
    ASSERT_FALSE(window->keyboardDidHideUIExtListeners_.empty());
    EXPECT_EQ(WMError::WM_OK, window->OnExtensionMessage(code, persistentId, want));

    code = static_cast<uint32_t>(Extension::Businesscode::UNREGISTER_KEYBOARD_DID_HIDE_LISTENER);
    EXPECT_EQ(WMError::WM_OK, window->OnExtensionMessage(code, persistentId, want));
    ASSERT_TRUE(window->keyboardDidHideUIExtListeners_.empty());
    EXPECT_EQ(WMError::WM_OK, window->OnExtensionMessage(code, persistentId, want));
}

/**
 * @tc.name: RegisterWaterfallModeChangeListener
 * @tc.desc: RegisterWaterfallModeChangeListener Test
 * @tc.type: FUNC
 */
HWTEST_F(WindowSessionImplTest, RegisterWaterfallModeChangeListener, TestSize.Level1)
{
    sptr<WindowOption> option = sptr<WindowOption>::MakeSptr();
    option->SetWindowName("waterfall");
    sptr<WindowSessionImpl> window = sptr<WindowSessionImpl>::MakeSptr(option);
    sptr<IWaterfallModeChangeListener> listener = sptr<IWaterfallModeChangeListener>::MakeSptr();
    auto ret = window->RegisterWaterfallModeChangeListener(listener);
    ASSERT_EQ(WMError::WM_OK, ret);
    listener = nullptr;
    ret = window->RegisterWaterfallModeChangeListener(listener);
    ASSERT_EQ(WMError::WM_ERROR_NULLPTR, ret);
    auto listeners = window->GetWaterfallModeChangeListeners();
    ASSERT_EQ(listeners.size(), 1);
}

/**
 * @tc.name: UnregisterWaterfallModeChangeListener
 * @tc.desc: UnregisterWaterfallModeChangeListener Test
 * @tc.type: FUNC
 */
HWTEST_F(WindowSessionImplTest, UnregisterWaterfallModeChangeListener, TestSize.Level1)
{
    sptr<WindowOption> option = sptr<WindowOption>::MakeSptr();
    option->SetWindowName("waterfall");
    sptr<WindowSessionImpl> window = sptr<WindowSessionImpl>::MakeSptr(option);
    window->waterfallModeChangeListeners_.clear();
    sptr<IWaterfallModeChangeListener> listener = sptr<IWaterfallModeChangeListener>::MakeSptr();
    auto ret = window->RegisterWaterfallModeChangeListener(listener);
    ASSERT_EQ(WMError::WM_OK, ret);
    auto listeners = window->GetWaterfallModeChangeListeners();
    ASSERT_EQ(listeners.size(), 1);
    ret = window->UnregisterWaterfallModeChangeListener(listener);
    ASSERT_EQ(WMError::WM_OK, ret);
    listeners = window->GetWaterfallModeChangeListeners();
    ASSERT_EQ(listeners.size(), 0);
    listener = nullptr;
    ret = window->UnregisterWaterfallModeChangeListener(listener);
    ASSERT_EQ(WMError::WM_ERROR_NULLPTR, ret);
}

/**
 * @tc.name: GetRouterStackInfo
 * @tc.desc: GetRouterStackInfo
 * @tc.type: FUNC
 */
HWTEST_F(WindowSessionImplTest, GetRouterStackInfo, TestSize.Level1)
{
    sptr<WindowOption> option = sptr<WindowOption>::MakeSptr();
    option->SetWindowName("GetRouterStackInfo");
    sptr<WindowSessionImpl> window = sptr<WindowSessionImpl>::MakeSptr(option);
    
    window->uiContent_ = std::make_unique<Ace::UIContentMocker>();
    std::string testInfo;
    auto res = window->GetRouterStackInfo(testInfo);
    EXPECT_EQ(res, WMError::WM_OK);

    window->uiContent_ = nullptr;
    res = window->GetRouterStackInfo(testInfo);
    EXPECT_EQ(res, WMError::WM_ERROR_NULLPTR);
}

/**
 * @tc.name: IsWaterfallModeEnabled
 * @tc.desc: IsWaterfallModeEnabled
 * @tc.type: FUNC
 */
HWTEST_F(WindowSessionImplTest, IsWaterfallModeEnabled, TestSize.Level1)
{
    sptr<WindowOption> option = sptr<WindowOption>::MakeSptr();
    option->SetWindowName("IsWaterfallModeEnabled");
    sptr<WindowSessionImpl> window = sptr<WindowSessionImpl>::MakeSptr(option);
    window->hostSession_ = nullptr;
    EXPECT_EQ(window->IsWaterfallModeEnabled(), false);

    auto mockHostSession = sptr<SessionStubMocker>::MakeSptr();
    window->hostSession_ = mockHostSession;
    window->property_->persistentId_ = 1234;
    EXPECT_CALL(*mockHostSession, GetWaterfallMode(_)).WillOnce(DoAll(SetArgReferee<0>(true), Return(WSError::WS_OK)));
    EXPECT_EQ(window->IsWaterfallModeEnabled(), true);
}

/**
 * @tc.name: NotifyWaterfallModeChange
 * @tc.desc: NotifyWaterfallModeChange Test
 * @tc.type: FUNC
 */
HWTEST_F(WindowSessionImplTest, NotifyWaterfallModeChange, TestSize.Level1)
{
    sptr<WindowOption> option = sptr<WindowOption>::MakeSptr();
    option->SetWindowName("waterfall");
    sptr<WindowSessionImpl> window = sptr<WindowSessionImpl>::MakeSptr(option);
    window->state_ = WindowState::STATE_SHOWN;
    sptr<IWaterfallModeChangeListener> listener = sptr<IWaterfallModeChangeListener>::MakeSptr();
    auto ret = window->RegisterWaterfallModeChangeListener(listener);
    ASSERT_EQ(WMError::WM_OK, ret);
    window->NotifyWaterfallModeChange(true);
}

/**
 * @tc.name: NotifyAcrossDisplaysChange
 * @tc.desc: NotifyAcrossDisplaysChange Test
 * @tc.type: FUNC
 */
HWTEST_F(WindowSessionImplTest, NotifyAcrossDisplaysChange, TestSize.Level1)
{
    sptr<WindowOption> option = sptr<WindowOption>::MakeSptr();
    option->SetWindowName("waterfall");
    sptr<WindowSessionImpl> window = sptr<WindowSessionImpl>::MakeSptr(option);
    window->property_->SetPersistentId(1);
    window->state_ = WindowState::STATE_SHOWN;
    sptr<IAcrossDisplaysChangeListener> listener = sptr<IAcrossDisplaysChangeListener>::MakeSptr();
    window->acrossDisplaysChangeListeners_[1].push_back(listener);
    window->RegisterAcrossDisplaysChangeListener(listener);
    auto ret = window->NotifyAcrossDisplaysChange(true);
    EXPECT_EQ(WMError::WM_OK, ret);
    ret = window->NotifyAcrossDisplaysChange(true);
    EXPECT_EQ(WMError::WM_DO_NOTHING, ret);
    window->acrossDisplaysChangeListeners_[1].push_back(nullptr);
    ret = window->NotifyAcrossDisplaysChange(false);
    EXPECT_EQ(WMError::WM_OK, ret);
}

/**
 * @tc.name: CreateSubWindowOutlineEnabled
 * @tc.desc: CreateSubWindowOutlineEnabled Test
 * @tc.type: FUNC
 */
HWTEST_F(WindowSessionImplTest, CreateSubWindowOutlineEnabled, TestSize.Level1)
{
    sptr<WindowOption> option1 = sptr<WindowOption>::MakeSptr();
    option1->SetWindowName("CreateSubWindowOutlineEnabled01");
    option1->SetWindowType(WindowType::WINDOW_TYPE_APP_SUB_WINDOW);
    sptr<WindowSessionImpl> window1 = sptr<WindowSessionImpl>::MakeSptr(option1);
    ASSERT_EQ(false, window1->property_->IsSubWindowOutlineEnabled());

    sptr<WindowOption> option2 = sptr<WindowOption>::MakeSptr();
    option2->SetWindowName("CreateSubWindowOutlineEnabled02");
    option2->SetWindowType(WindowType::WINDOW_TYPE_APP_SUB_WINDOW);
    option2->SetSubWindowOutlineEnabled(false);
    sptr<WindowSessionImpl> window2 = sptr<WindowSessionImpl>::MakeSptr(option2);
    ASSERT_EQ(false, window2->property_->IsSubWindowOutlineEnabled());

    sptr<WindowOption> option3 = sptr<WindowOption>::MakeSptr();
    option3->SetWindowName("CreateSubWindowOutlineEnabled03");
    option3->SetWindowType(WindowType::WINDOW_TYPE_APP_SUB_WINDOW);
    option3->SetSubWindowOutlineEnabled(true);
    sptr<WindowSessionImpl> window3 = sptr<WindowSessionImpl>::MakeSptr(option3);
    ASSERT_EQ(true, window3->property_->IsSubWindowOutlineEnabled());
}

/**
 * @tc.name: GetAttachStateSyncResult
 * @tc.desc: GetAttachStateSyncResult Test
 * @tc.type: FUNC
 */
HWTEST_F(WindowSessionImplTest, GetAttachStateSyncResult, TestSize.Level1)
{
    g_errLog.clear();
    LOG_SetCallback(MyLogCallback);
    sptr<WindowOption> option = sptr<WindowOption>::MakeSptr();
    option->SetWindowName("GetAttachStateSyncResult");
    option->SetWindowType(WindowType::WINDOW_TYPE_INPUT_METHOD_FLOAT);
    sptr<WindowSessionImpl> window = sptr<WindowSessionImpl>::MakeSptr(option);

    window->lifecycleCallback_ = sptr<LifecycleFutureCallback>::MakeSptr();
    window->GetAttachStateSyncResult(true, true);
    EXPECT_TRUE(g_errLog.find("get attach state sync result") == std::string::npos);

    window->GetAttachStateSyncResult(true, false);
    EXPECT_TRUE(g_errLog.find("get attach state sync result") == std::string::npos);

    window->lifecycleCallback_ = nullptr;
    window->property_->SetWindowType(WindowType::WINDOW_TYPE_APP_SUB_WINDOW);
    window->GetAttachStateSyncResult(true, true);
    EXPECT_TRUE(g_errLog.find("lifecycleCallback is null") != std::string::npos);
    EXPECT_TRUE(g_errLog.find("get attach state sync result") == std::string::npos);

    window->lifecycleCallback_ = sptr<LifecycleFutureCallback>::MakeSptr();
    window->GetAttachStateSyncResult(false, true);
    EXPECT_TRUE(g_errLog.find("get attach state sync result") == std::string::npos);

    window->GetAttachStateSyncResult(true, true);
    EXPECT_TRUE(g_errLog.find("get attach state sync result") != std::string::npos);

    window->GetAttachStateSyncResult(true, false);
    EXPECT_TRUE(g_errLog.find("get attach state sync result") != std::string::npos);
}
}
} // namespace
} // namespace Rosen
} // namespace OHOS
