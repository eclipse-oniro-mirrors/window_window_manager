/*
 * Copyright (c) 2024 Huawei Device Co., Ltd.
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 * http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include <gtest/gtest.h>
#include <parameters.h>
#include "ability_context_impl.h"
#include "display_info.h"
#include "mock_session.h"
#include "mock_uicontent.h"
#include "mock_window_adapter.h"
#include "singleton_mocker.h"
#include "window_scene_session_impl.h"
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
using Mocker = SingletonMocker<WindowAdapter, MockWindowAdapter>;
uint32_t MaxWith = 32;

class WindowSceneSessionImplTest2 : public testing::Test {
public:
    static void SetUpTestCase();
    static void TearDownTestCase();
    void SetUp() override;
    void TearDown() override;

    std::shared_ptr<AbilityRuntime::AbilityContext> abilityContext_;
    std::unique_ptr<Mocker> m = std::make_unique<Mocker>();

private:
    RSSurfaceNode::SharedPtr CreateRSSurfaceNode();
    sptr<WindowSceneSessionImpl> InitialWindowState();
    static constexpr uint32_t WAIT_SYNC_IN_NS = 200000;
};

void WindowSceneSessionImplTest2::SetUpTestCase() {}

void WindowSceneSessionImplTest2::TearDownTestCase() {}

void WindowSceneSessionImplTest2::SetUp()
{
    abilityContext_ = std::make_shared<AbilityRuntime::AbilityContextImpl>();
}

void WindowSceneSessionImplTest2::TearDown()
{
    usleep(WAIT_SYNC_IN_NS);
    abilityContext_ = nullptr;
}

RSSurfaceNode::SharedPtr WindowSceneSessionImplTest2::CreateRSSurfaceNode()
{
    struct RSSurfaceNodeConfig rsSurfaceNodeConfig;
    rsSurfaceNodeConfig.SurfaceNodeName = "startingWindowTestSurfaceNode";
    auto surfaceNode = RSSurfaceNode::Create(rsSurfaceNodeConfig, RSSurfaceNodeType::DEFAULT);
    return surfaceNode;
}

sptr<WindowSceneSessionImpl> WindowSceneSessionImplTest2::InitialWindowState()
{
    sptr<WindowOption> option = sptr<WindowOption>::MakeSptr();
    sptr<WindowSceneSessionImpl> windowSceneSession = sptr<WindowSceneSessionImpl>::MakeSptr(option);
    windowSceneSession->property_->SetPersistentId(1);
    SessionInfo sessionInfo = {"CreateTestBundle", "CreatTestModule", "CreateTestAbility"};
    sptr<SessionMocker> session = sptr<SessionMocker>::MakeSptr(sessionInfo);
    windowSceneSession->hostSession_ = session;
    windowSceneSession->state_ = WindowState::STATE_SHOWN;
    return windowSceneSession;
}

namespace {
/**
 * @tc.name: SetBackdropBlur
 * @tc.desc: SetBackdropBlur test
 * @tc.type: FUNC
 */
HWTEST_F(WindowSceneSessionImplTest2, SetBackdropBlur, Function | SmallTest | Level3)
{
    sptr<WindowOption> option = sptr<WindowOption>::MakeSptr();
    sptr<WindowSceneSessionImpl> window = sptr<WindowSceneSessionImpl>::MakeSptr(option);
    window->property_->SetWindowName("SetBackdropBlur");
    window->property_->SetWindowType(WindowType::SYSTEM_SUB_WINDOW_BASE);

    ASSERT_EQ(WMError::WM_ERROR_INVALID_PARAM, window->SetBackdropBlur(-1.0));
    ASSERT_EQ(WMError::WM_OK, window->SetBackdropBlur(1.0));
    ASSERT_EQ(WMError::WM_OK, window->SetBackdropBlur(0.0));
    window->surfaceNode_ = nullptr;
    ASSERT_EQ(WMError::WM_ERROR_NULLPTR, window->SetBackdropBlur(1.0));
}

/**
 * @tc.name: SetShadowColor
 * @tc.desc: SetShadowColor test
 * @tc.type: FUNC
 */
HWTEST_F(WindowSceneSessionImplTest2, SetShadowColor, Function | SmallTest | Level3)
{
    sptr<WindowOption> option = sptr<WindowOption>::MakeSptr();
    sptr<WindowSceneSessionImpl> window = sptr<WindowSceneSessionImpl>::MakeSptr(option);
    window->property_->SetWindowName("SetShadowColor");
    window->property_->SetWindowType(WindowType::SYSTEM_SUB_WINDOW_BASE);

    ASSERT_EQ(WMError::WM_ERROR_INVALID_PARAM, window->SetShadowColor("111ff22ee44"));
    ASSERT_EQ(WMError::WM_OK, window->SetShadowColor("#ff22ee44"));
    ASSERT_EQ(WMError::WM_OK, window->SetShadowColor("#000999"));
    window->surfaceNode_ = nullptr;
    ASSERT_EQ(WMError::WM_ERROR_NULLPTR, window->SetShadowColor("#ff22ee44"));
}

/**
 * @tc.name: SetCornerRadius
 * @tc.desc: SetCornerRadius test
 * @tc.type: FUNC
 */
HWTEST_F(WindowSceneSessionImplTest2, SetCornerRadius, Function | SmallTest | Level3)
{
    sptr<WindowOption> option = sptr<WindowOption>::MakeSptr();
    sptr<WindowSceneSessionImpl> window = sptr<WindowSceneSessionImpl>::MakeSptr(option);
    window->property_->SetWindowName("SetCornerRadius");
    window->property_->SetWindowType(WindowType::SYSTEM_SUB_WINDOW_BASE);

    ASSERT_EQ(WMError::WM_OK, window->SetCornerRadius(1.0));
    window->surfaceNode_ = nullptr;
    ASSERT_EQ(WMError::WM_ERROR_NULLPTR, window->SetCornerRadius(1.0));
}

/**
 * @tc.name: SetShadowRadius
 * @tc.desc: SetShadowRadius test
 * @tc.type: FUNC
 */
HWTEST_F(WindowSceneSessionImplTest2, SetShadowRadius, Function | SmallTest | Level3)
{
    sptr<WindowOption> option = sptr<WindowOption>::MakeSptr();
    sptr<WindowSceneSessionImpl> window = sptr<WindowSceneSessionImpl>::MakeSptr(option);
    window->property_->SetWindowName("SetShadowRadius");
    window->property_->SetWindowType(WindowType::SYSTEM_SUB_WINDOW_BASE);

    ASSERT_EQ(WMError::WM_ERROR_INVALID_PARAM, window->SetShadowRadius(-1.0));
    ASSERT_EQ(WMError::WM_OK, window->SetShadowRadius(0.0));
    ASSERT_EQ(WMError::WM_OK, window->SetShadowRadius(1.0));
    window->surfaceNode_ = nullptr;
    ASSERT_EQ(WMError::WM_ERROR_NULLPTR, window->SetShadowRadius(1.0));
}

/**
 * @tc.name: SetTransform01
 * @tc.desc: set transform
 * @tc.type: FUNC
 * @tc.require:issueI7IJVV
 */
HWTEST_F(WindowSceneSessionImplTest2, SetTransform01, Function | SmallTest | Level3)
{
    std::unique_ptr<Mocker> m = std::make_unique<Mocker>();
    sptr<WindowOption> option = sptr<WindowOption>::MakeSptr();
    option->SetWindowName("SetTransform01");
    sptr<WindowSceneSessionImpl> window = sptr<WindowSceneSessionImpl>::MakeSptr(option);
    window->property_->SetPersistentId(1);
    Transform trans_;
    ASSERT_EQ(WMError::WM_ERROR_INVALID_WINDOW, window->SetTransform(trans_));
}

/**
 * @tc.name: SetTransform01
 * @tc.desc: set transform
 * @tc.type: FUNC
 */
HWTEST_F(WindowSceneSessionImplTest2, SetTransform02, Function | SmallTest | Level3)
{
    std::unique_ptr<Mocker> m = std::make_unique<Mocker>();
    sptr<WindowOption> option = sptr<WindowOption>::MakeSptr();
    option->SetWindowName("SetTransform01");
    sptr<WindowSceneSessionImpl> window = sptr<WindowSceneSessionImpl>::MakeSptr(option);

    SessionInfo sessionInfo = {"CreateTestBundle", "CreateTestModule", "CreateTestAbility"};
    sptr<SessionMocker> session = sptr<SessionMocker>::MakeSptr(sessionInfo);
    window->hostSession_ = session;
    window->property_->SetPersistentId(1);
    window->state_ = WindowState::STATE_CREATED;
    Transform trans_;
    ASSERT_EQ(WMError::WM_OK, window->SetTransform(trans_));
    ASSERT_EQ(trans_, window->GetTransform());
}

/**
 * @tc.name: RegisterAnimationTransitionController01
 * @tc.desc: RegisterAnimationTransitionController
 * @tc.type: FUNC
 */
HWTEST_F(WindowSceneSessionImplTest2, RegisterAnimationTransitionController01, TestSize.Level1)
{
    std::unique_ptr<Mocker> m = std::make_unique<Mocker>();
    sptr<WindowOption> option = sptr<WindowOption>::MakeSptr();
    option->SetWindowName("RegisterAnimationTransitionController01");
    sptr<WindowSceneSessionImpl> window = sptr<WindowSceneSessionImpl>::MakeSptr(option);
    window->property_->SetPersistentId(1);
    ASSERT_EQ(WMError::WM_ERROR_NULLPTR, window->RegisterAnimationTransitionController(nullptr));
}

/**
 * @tc.name: SetNeedDefaultAnimation01
 * @tc.desc: SetNeedDefaultAnimation
 * @tc.type: FUNC
 * @tc.require:issueI7IJVV
 */
HWTEST_F(WindowSceneSessionImplTest2, SetNeedDefaultAnimation01, TestSize.Level1)
{
    std::unique_ptr<Mocker> m = std::make_unique<Mocker>();
    sptr<WindowOption> option = sptr<WindowOption>::MakeSptr();
    option->SetWindowName("SetNeedDefaultAnimation01");
    sptr<WindowSceneSessionImpl> window = sptr<WindowSceneSessionImpl>::MakeSptr(option);
    auto ret = true;
    window->property_->SetPersistentId(1);

    SessionInfo sessionInfo = { "CreateTestBundle", "CreateTestModule", "CreateTestAbility" };
    sptr<SessionMocker> session = sptr<SessionMocker>::MakeSptr(sessionInfo);
    window->hostSession_ = session;
    window->SetNeedDefaultAnimation(false);
    ASSERT_TRUE(ret);
}

/**
 * @tc.desc: UpdateSurfaceNodeAfterCustomAnimation01
 * @tc.desc: UpdateSurfaceNodeAfterCustomAnimation
 * @tc.type: FUNC
 * @tc.require:issueI7IJVV
 */
HWTEST_F(WindowSceneSessionImplTest2, UpdateSurfaceNodeAfterCustomAnimation, TestSize.Level1)
{
    std::unique_ptr<Mocker> m = std::make_unique<Mocker>();
    sptr<WindowOption> option = sptr<WindowOption>::MakeSptr();
    option->SetWindowName("UpdateSurfaceNodeAfterCustomAnimation");
    sptr<WindowSceneSessionImpl> window = sptr<WindowSceneSessionImpl>::MakeSptr(option);
    window->property_->SetPersistentId(1);

    window->UpdateSurfaceNodeAfterCustomAnimation(false);
    ASSERT_EQ(WMError::WM_ERROR_INVALID_WINDOW, window->UpdateSurfaceNodeAfterCustomAnimation(false));
    window->property_->SetPersistentId(1);
    SessionInfo sessionInfo = { "CreateTestBundle", "CreateTestModule", "CreateTestAbility" };
    sptr<SessionMocker> session = sptr<SessionMocker>::MakeSptr(sessionInfo);
    window->hostSession_ = session;
    window->property_->SetWindowType(WindowType::WINDOW_TYPE_APP_SUB_WINDOW);
    ASSERT_EQ(WMError::WM_ERROR_INVALID_OPERATION, window->UpdateSurfaceNodeAfterCustomAnimation(false));
}

/**
 * @tc.name: SetAlpha01
 * @tc.desc: SetAlpha
 * @tc.type: FUNC
 */
HWTEST_F(WindowSceneSessionImplTest2, SetAlpha01, Function | SmallTest | Level2)
{
    sptr<WindowOption> option = sptr<WindowOption>::MakeSptr();
    option->SetWindowName("SetAlpha01");
    option->SetWindowType(WindowType::WINDOW_TYPE_APP_MAIN_WINDOW);
    sptr<WindowSceneSessionImpl> windowSceneSession = sptr<WindowSceneSessionImpl>::MakeSptr(option);
    windowSceneSession->property_->SetPersistentId(11);
    windowSceneSession->property_->SetWindowType(WindowType::WINDOW_TYPE_APP_MAIN_WINDOW);
    ASSERT_EQ(WMError::WM_ERROR_INVALID_WINDOW, windowSceneSession->SetAlpha(1.0));
}

/**
 * @tc.name: SetAlpha02
 * @tc.desc: SetAlpha
 * @tc.type: FUNC
 */
HWTEST_F(WindowSceneSessionImplTest2, SetAlpha02, Function | SmallTest | Level2)
{
    sptr<WindowOption> option = sptr<WindowOption>::MakeSptr();
    option->SetWindowName("SetAlpha02");
    option->SetWindowType(WindowType::WINDOW_TYPE_APP_MAIN_WINDOW);
    sptr<WindowSceneSessionImpl> windowSceneSession = sptr<WindowSceneSessionImpl>::MakeSptr(option);

    SessionInfo sessionInfo = {"CreateTestBundle", "CreateTestMode", "CreateTestAbility"};
    sptr<SessionMocker> session = sptr<SessionMocker>::MakeSptr(sessionInfo);
    windowSceneSession->hostSession_ = session;
    windowSceneSession->property_->SetPersistentId(1);
    windowSceneSession->property_->SetWindowType(WindowType::WINDOW_TYPE_APP_MAIN_WINDOW);
    ASSERT_EQ(WMError::WM_OK, windowSceneSession->SetAlpha(1.0));
}

/**
 * @tc.name: DestroySubWindow01
 * @tc.desc: DestroySubWindow
 * @tc.type: FUNC
 */
HWTEST_F(WindowSceneSessionImplTest2, DestroySubWindow01, TestSize.Level1)
{
    sptr<WindowOption> option = sptr<WindowOption>::MakeSptr();
    option->SetWindowName("DestroySubWindow01");
    option->SetWindowType(WindowType::WINDOW_TYPE_APP_MAIN_WINDOW);
    sptr<WindowSceneSessionImpl> windowSceneSession = sptr<WindowSceneSessionImpl>::MakeSptr(option);
    windowSceneSession->DestroySubWindow();
    ASSERT_EQ(INVALID_SESSION_ID, windowSceneSession->property_->GetParentPersistentId());
}

/**
 * @tc.name: UpdateFloatingWindowSizeBySizeLimits01
 * @tc.desc: UpdateFloatingWindowSizeBySizeLimits
 * @tc.type: FUNC
 */
HWTEST_F(WindowSceneSessionImplTest2, UpdateFloatingWindowSizeBySizeLimits01, TestSize.Level1)
{
    g_errLog.clear();
    LOG_SetCallback(MyLogCallback);
    sptr<WindowOption> option = sptr<WindowOption>::MakeSptr();
    option->SetWindowName("UpdateFloatingWindowSizeBySizeLimits01");
    option->SetWindowType(WindowType::WINDOW_TYPE_FLOAT_CAMERA);
    sptr<WindowSceneSessionImpl> windowSceneSession = sptr<WindowSceneSessionImpl>::MakeSptr(option);
    windowSceneSession->UpdateFloatingWindowSizeBySizeLimits(MaxWith, MaxWith);
    EXPECT_FALSE(g_errLog.find("float camera type window") != std::string::npos);
    LOG_SetCallback(nullptr);
}

/**
 * @tc.name: UpdateAnimationFlagProperty01
 * @tc.desc: UpdateAnimationFlagProperty
 * @tc.type: FUNC
 */
HWTEST_F(WindowSceneSessionImplTest2, UpdateAnimationFlagProperty01, TestSize.Level1)
{
    sptr<WindowOption> option = sptr<WindowOption>::MakeSptr();
    option->SetWindowName("UpdateAnimationFlagProperty01");
    option->SetWindowType(WindowType::SYSTEM_WINDOW_BASE);
    sptr<WindowSceneSessionImpl> windowSceneSession = sptr<WindowSceneSessionImpl>::MakeSptr(option);
    ASSERT_EQ(WMError::WM_ERROR_INVALID_WINDOW, windowSceneSession->UpdateAnimationFlagProperty(false));
}

/**
 * @tc.name: UpdateAnimationFlagProperty02
 * @tc.desc: UpdateAnimationFlagProperty
 * @tc.type: FUNC
 */
HWTEST_F(WindowSceneSessionImplTest2, UpdateAnimationFlagProperty02, TestSize.Level1)
{
    sptr<WindowOption> option = sptr<WindowOption>::MakeSptr();
    option->SetWindowName("UpdateAnimationFlagProperty01");
    option->SetWindowType(WindowType::APP_MAIN_WINDOW_BASE);

    sptr<WindowSceneSessionImpl> windowSceneSession = sptr<WindowSceneSessionImpl>::MakeSptr(option);
    ASSERT_EQ(WMError::WM_OK, windowSceneSession->UpdateAnimationFlagProperty(false));
}

/**
 * @tc.name: UpdateWindowModeImmediately01
 * @tc.desc: UpdateWindowModeImmediately
 * @tc.type: FUNC
 */
HWTEST_F(WindowSceneSessionImplTest2, UpdateWindowModeImmediately01, TestSize.Level1)
{
    sptr<WindowOption> option = sptr<WindowOption>::MakeSptr();
    option->SetWindowName("UpdateWindowModeImmediately01");
    option->SetWindowType(WindowType::SYSTEM_WINDOW_BASE);

    sptr<WindowSceneSessionImpl> windowSceneSession = sptr<WindowSceneSessionImpl>::MakeSptr(option);
    ASSERT_EQ(WMError::WM_OK, windowSceneSession->UpdateWindowModeImmediately(WindowMode::WINDOW_MODE_UNDEFINED));
    windowSceneSession->state_ = WindowState::STATE_CREATED;
    ASSERT_EQ(WMError::WM_OK, windowSceneSession->UpdateWindowModeImmediately(WindowMode::WINDOW_MODE_UNDEFINED));
}

/**
 * @tc.name: UpdateWindowMode01
 * @tc.desc: UpdateWindowMode
 * @tc.type: FUNC
 */
HWTEST_F(WindowSceneSessionImplTest2, UpdateWindowMode01, TestSize.Level1)
{
    sptr<WindowOption> option = sptr<WindowOption>::MakeSptr();
    option->SetWindowName("UpdateWindowMode01");
    option->SetWindowType(WindowType::SYSTEM_WINDOW_BASE);

    sptr<WindowSceneSessionImpl> windowSceneSession = sptr<WindowSceneSessionImpl>::MakeSptr(option);
    ASSERT_EQ(WSError::WS_ERROR_INVALID_WINDOW,
              windowSceneSession->UpdateWindowMode(WindowMode::WINDOW_MODE_FULLSCREEN));
    windowSceneSession->state_ = WindowState::STATE_CREATED;
    ASSERT_EQ(WSError::WS_ERROR_INVALID_WINDOW,
              windowSceneSession->UpdateWindowMode(WindowMode::WINDOW_MODE_FULLSCREEN));

    SessionInfo sessionInfo = { "CreateTestBundle", "CreateTestModule", "CreateTestAbility" };
    sptr<SessionMocker> session = sptr<SessionMocker>::MakeSptr(sessionInfo);
    windowSceneSession->hostSession_ = session;
    ASSERT_EQ(WSError::WS_ERROR_INVALID_WINDOW,
              windowSceneSession->UpdateWindowMode(WindowMode::WINDOW_MODE_FULLSCREEN));
    windowSceneSession->property_->SetPersistentId(1);
    ASSERT_EQ(WSError::WS_ERROR_INVALID_WINDOW_MODE_OR_SIZE,
              windowSceneSession->UpdateWindowMode(WindowMode::WINDOW_MODE_UNDEFINED));
}

/**
 * @tc.name: UpdateWindowMode02
 * @tc.desc: UpdateWindowMode
 * @tc.type: FUNC
 */
HWTEST_F(WindowSceneSessionImplTest2, UpdateWindowMode02, TestSize.Level1)
{
    sptr<WindowOption> option = sptr<WindowOption>::MakeSptr();
    option->SetWindowName("UpdateWindowMode02");
    option->SetWindowType(WindowType::SYSTEM_WINDOW_BASE);

    sptr<WindowSceneSessionImpl> windowSceneSession = sptr<WindowSceneSessionImpl>::MakeSptr(option);
    ASSERT_EQ(WSError::WS_ERROR_INVALID_WINDOW,
              windowSceneSession->UpdateWindowMode(WindowMode::WINDOW_MODE_FULLSCREEN));
    windowSceneSession->state_ = WindowState::STATE_CREATED;
    ASSERT_EQ(WSError::WS_ERROR_INVALID_WINDOW,
              windowSceneSession->UpdateWindowMode(WindowMode::WINDOW_MODE_FULLSCREEN));

    SessionInfo sessionInfo = { "CreateTestBundle", "CreateTestModule", "CreateTestAbility" };
    sptr<SessionMocker> session = sptr<SessionMocker>::MakeSptr(sessionInfo);
    windowSceneSession->hostSession_ = session;
    ASSERT_EQ(WSError::WS_ERROR_INVALID_WINDOW,
              windowSceneSession->UpdateWindowMode(WindowMode::WINDOW_MODE_FULLSCREEN));
    windowSceneSession->property_->SetPersistentId(1);
    ASSERT_EQ(WSError::WS_OK, windowSceneSession->UpdateWindowMode(WindowMode::WINDOW_MODE_FULLSCREEN));
}

/**
 * @tc.name: GetTopNavDestinationName01
 * @tc.desc: test GetTopNavDestinationName whether get the top nav destination name.
 * @tc.type: FUNC
 */
HWTEST_F(WindowSceneSessionImplTest2, GetTopNavDestinationName01, TestSize.Level1)
{
    sptr<WindowOption> option = sptr<WindowOption>::MakeSptr();
    option->SetWindowName("GetTopNavDestinationName01");
    option->SetWindowType(WindowType::SYSTEM_WINDOW_BASE);
    sptr<WindowSceneSessionImpl> windowSceneSession = sptr<WindowSceneSessionImpl>::MakeSptr(option);
    windowSceneSession->uiContent_ = nullptr;
    std::string topNavDestName;
    windowSceneSession->GetTopNavDestinationName(topNavDestName);
    EXPECT_EQ(topNavDestName, "");

    windowSceneSession->uiContent_ = std::make_shared<Ace::UIContentMocker>();
    windowSceneSession->GetTopNavDestinationName(topNavDestName);
    EXPECT_EQ(topNavDestName, "");

    Ace::UIContentMocker* uiContent = reinterpret_cast<Ace::UIContentMocker*>(windowSceneSession->uiContent_.get());
    EXPECT_CALL(*uiContent, GetTopNavDestinationInfo(_, _)).WillOnce(Return("{"));
    windowSceneSession->GetTopNavDestinationName(topNavDestName);
    EXPECT_EQ(topNavDestName, "");

    EXPECT_CALL(*uiContent, GetTopNavDestinationInfo(_, _)).WillOnce(Return("{\"name\":1}"));
    windowSceneSession->GetTopNavDestinationName(topNavDestName);
    EXPECT_EQ(topNavDestName, "");

    EXPECT_CALL(*uiContent, GetTopNavDestinationInfo(_, _)).WillOnce(Return("{\"name\":\"test\"}"));
    windowSceneSession->GetTopNavDestinationName(topNavDestName);
    EXPECT_EQ(topNavDestName, "test");
}

/**
 * @tc.name: UpdateDefaultStatusBarColor01
 * @tc.desc: test UpdateDefaultStatusBarColor
 * @tc.type: FUNC
 */
HWTEST_F(WindowSceneSessionImplTest2, UpdateDefaultStatusBarColor01, TestSize.Level1)
{
    sptr<WindowOption> option = sptr<WindowOption>::MakeSptr();
    sptr<WindowSceneSessionImpl> windowSceneSession = sptr<WindowSceneSessionImpl>::MakeSptr(option);
    windowSceneSession->property_ = sptr<WindowSessionProperty>::MakeSptr();
    windowSceneSession->property_->SetWindowType(WindowType::APP_SUB_WINDOW_BASE);
    windowSceneSession->UpdateDefaultStatusBarColor();
    windowSceneSession->property_->SetWindowType(WindowType::WINDOW_TYPE_APP_MAIN_WINDOW);
    windowSceneSession->UpdateDefaultStatusBarColor();
    windowSceneSession->specifiedColorMode_ = "light";
    windowSceneSession->UpdateDefaultStatusBarColor();
    EXPECT_EQ(windowSceneSession->specifiedColorMode_, "light");
}

/**
 * @tc.name: RemoveWindowFlag01
 * @tc.desc: RemoveWindowFlag
 * @tc.type: FUNC
 */
HWTEST_F(WindowSceneSessionImplTest2, RemoveWindowFlag01, TestSize.Level1)
{
    sptr<WindowOption> option = sptr<WindowOption>::MakeSptr();
    option->SetWindowName("RemoveWindowFlag01");
    option->SetWindowType(WindowType::SYSTEM_WINDOW_BASE);

    sptr<WindowSceneSessionImpl> windowSceneSession = sptr<WindowSceneSessionImpl>::MakeSptr(option);
    ASSERT_EQ(WMError::WM_ERROR_INVALID_WINDOW,
              windowSceneSession->RemoveWindowFlag(WindowFlag::WINDOW_FLAG_NEED_AVOID));
    windowSceneSession->state_ = WindowState::STATE_CREATED;
    ASSERT_EQ(WMError::WM_ERROR_INVALID_WINDOW,
              windowSceneSession->RemoveWindowFlag(WindowFlag::WINDOW_FLAG_NEED_AVOID));

    SessionInfo sessionInfo = { "CreateTestBundle", "CreateTestModule", "CreateTestAbility" };
    sptr<SessionMocker> session = sptr<SessionMocker>::MakeSptr(sessionInfo);
    windowSceneSession->hostSession_ = session;
    ASSERT_EQ(WMError::WM_ERROR_INVALID_WINDOW,
              windowSceneSession->RemoveWindowFlag(WindowFlag::WINDOW_FLAG_NEED_AVOID));
}

/**
 * @tc.name: GetConfigurationFromAbilityInfo01
 * @tc.desc: GetConfigurationFromAbilityInfo
 * @tc.type: FUNC
 */
HWTEST_F(WindowSceneSessionImplTest2, GetConfigurationFromAbilityInfo01, TestSize.Level1)
{
    g_errLog.clear();
    LOG_SetCallback(MyLogCallback);
    sptr<WindowOption> option = sptr<WindowOption>::MakeSptr();
    option->SetWindowName("GetConfigurationFromAbilityInfo01");
    option->SetWindowType(WindowType::SYSTEM_WINDOW_BASE);
    sptr<WindowSceneSessionImpl> windowSceneSession = sptr<WindowSceneSessionImpl>::MakeSptr(option);
    windowSceneSession->GetConfigurationFromAbilityInfo();
    EXPECT_TRUE(g_errLog.find("abilityContext is nullptr") != std::string::npos);
    LOG_SetCallback(nullptr);
}

/**
 * @tc.name: ExtractSupportWindowModeFromMetaData
 * @tc.desc: ExtractSupportWindowModeFromMetaData
 * @tc.type: FUNC
 */
HWTEST_F(WindowSceneSessionImplTest2, ExtractSupportWindowModeFromMetaData, Function | SmallTest | Level2)
{
    sptr<WindowOption> option = sptr<WindowOption>::MakeSptr();
    option->SetWindowName("ExtractSupportWindowModeFromMetaData");
    option->SetWindowType(WindowType::SYSTEM_WINDOW_BASE);
    sptr<WindowSceneSessionImpl> windowSceneSession = sptr<WindowSceneSessionImpl>::MakeSptr(option);

    windowSceneSession->windowSystemConfig_.windowUIType_ = WindowUIType::PC_WINDOW;
    AppExecFwk::AbilityInfo abilityInfo;
    int ret = 0;
    std::vector<AppExecFwk::SupportWindowMode> updateWindowModes =
        windowSceneSession->ExtractSupportWindowModeFromMetaData(
        std::make_shared<OHOS::AppExecFwk::AbilityInfo>(abilityInfo));
    ASSERT_EQ(ret, 0);

    windowSceneSession->windowSystemConfig_.windowUIType_ = WindowUIType::PHONE_WINDOW;
    windowSceneSession->windowSystemConfig_.freeMultiWindowEnable_ = false;
    updateWindowModes =
        windowSceneSession->ExtractSupportWindowModeFromMetaData(
        std::make_shared<OHOS::AppExecFwk::AbilityInfo>(abilityInfo));
    ASSERT_EQ(ret, 0);
}

/**
 * @tc.name: ParseWindowModeFromMetaData
 * @tc.desc: ParseWindowModeFromMetaData
 * @tc.type: FUNC
 */
HWTEST_F(WindowSceneSessionImplTest2, ParseWindowModeFromMetaData, Function | SmallTest | Level2)
{
    sptr<WindowOption> option = sptr<WindowOption>::MakeSptr();
    option->SetWindowName("ParseWindowModeFromMetaData");
    option->SetWindowType(WindowType::SYSTEM_WINDOW_BASE);
    sptr<WindowSceneSessionImpl> windowSceneSession = sptr<WindowSceneSessionImpl>::MakeSptr(option);

    std::vector<AppExecFwk::SupportWindowMode> updateWindowModes =
        {AppExecFwk::SupportWindowMode::FULLSCREEN, AppExecFwk::SupportWindowMode::SPLIT,
        AppExecFwk::SupportWindowMode::FLOATING};
    ASSERT_EQ(updateWindowModes, windowSceneSession->ParseWindowModeFromMetaData("fullscreen,split,floating"));
}

/**
 * @tc.name: SetDefaultProperty01
 * @tc.desc: SetDefaultProperty
 * @tc.type: FUNC
 */
HWTEST_F(WindowSceneSessionImplTest2, SetDefaultProperty01, TestSize.Level1)
{
    sptr<WindowOption> option = sptr<WindowOption>::MakeSptr();
    option->SetWindowName("SetDefaultProperty01");
    option->SetWindowType(WindowType::SYSTEM_WINDOW_BASE);
    option->SetWindowMode(WindowMode::WINDOW_MODE_UNDEFINED);
    sptr<WindowSceneSessionImpl> windowSceneSession = sptr<WindowSceneSessionImpl>::MakeSptr(option);
    windowSceneSession->SetDefaultProperty();
    ASSERT_NE(WindowMode::WINDOW_MODE_FLOATING, windowSceneSession->GetWindowMode());
}

/**
 * @tc.name: UpdateConfiguration01
 * @tc.desc: UpdateConfiguration
 * @tc.type: FUNC
 */
HWTEST_F(WindowSceneSessionImplTest2, UpdateConfiguration01, TestSize.Level1)
{
    sptr<WindowOption> option = sptr<WindowOption>::MakeSptr();
    option->SetWindowName("UpdateConfiguration01");
    option->SetWindowType(WindowType::SYSTEM_WINDOW_BASE);
    sptr<WindowSceneSessionImpl> windowSceneSession = sptr<WindowSceneSessionImpl>::MakeSptr(option);
    std::shared_ptr<AppExecFwk::Configuration> configuration;
    windowSceneSession->UpdateConfiguration(configuration);
    windowSceneSession->uiContent_ = std::make_shared<Ace::UIContentMocker>();
    ASSERT_NE(windowSceneSession->uiContent_, nullptr);
    windowSceneSession->UpdateConfiguration(configuration);
    sptr<WindowOption> option2 = sptr<WindowOption>::MakeSptr();
    option2->SetWindowName("subWindow");
    option2->SetWindowType(WindowType::WINDOW_TYPE_APP_SUB_WINDOW);
    sptr<WindowSceneSessionImpl> subSession = sptr<WindowSceneSessionImpl>::MakeSptr(option2);
    ASSERT_NE(subSession, nullptr);
    subSession->property_->SetPersistentId(8);
    windowSceneSession->subWindowSessionMap_.clear();
    windowSceneSession->subWindowSessionMap_[windowSceneSession->GetPersistentId()].push_back(subSession);
    windowSceneSession->UpdateConfiguration(configuration);
    windowSceneSession->subWindowSessionMap_.clear();
}

/**
 * @tc.name: UpdateConfigurationSync
 * @tc.desc: Test UpdateConfigurationSync can update new configuration of window
 * @tc.type: FUNC
 */
HWTEST_F(WindowSceneSessionImplTest2, UpdateConfigurationSync, TestSize.Level1)
{
    sptr<WindowOption> option = sptr<WindowOption>::MakeSptr();
    option->SetWindowName("UpdateConfigurationSync01");
    option->SetWindowType(WindowType::SYSTEM_WINDOW_BASE);
    sptr<WindowSceneSessionImpl> windowSceneSession = sptr<WindowSceneSessionImpl>::MakeSptr(option);
    std::shared_ptr<AppExecFwk::Configuration> configuration;
    windowSceneSession->UpdateConfigurationSync(configuration);
    windowSceneSession->uiContent_ = std::make_shared<Ace::UIContentMocker>();
    windowSceneSession->UpdateConfigurationSync(configuration);
    sptr<WindowOption> option2 = sptr<WindowOption>::MakeSptr();
    option2->SetWindowName("subWindow");
    option2->SetWindowType(WindowType::WINDOW_TYPE_APP_SUB_WINDOW);
    sptr<WindowSceneSessionImpl> subSession = sptr<WindowSceneSessionImpl>::MakeSptr(option2);
    ASSERT_NE(subSession, nullptr);
    subSession->property_->SetPersistentId(8);
    windowSceneSession->subWindowSessionMap_.clear();
    windowSceneSession->subWindowSessionMap_[windowSceneSession->GetPersistentId()].push_back(subSession);
    windowSceneSession->UpdateConfigurationSync(configuration);
    windowSceneSession->subWindowSessionMap_.clear();
}

/**
 * @tc.name: UpdateConfigurationForSpecified
 * @tc.desc: UpdateConfigurationForSpecified
 * @tc.type: FUNC
 */
HWTEST_F(WindowSceneSessionImplTest2, UpdateConfigurationForSpecified, TestSize.Level1)
{
    sptr<WindowOption> option = sptr<WindowOption>::MakeSptr();
    option->SetWindowName("UpdateConfiguration01");
    option->SetWindowType(WindowType::SYSTEM_WINDOW_BASE);
    sptr<WindowSceneSessionImpl> windowSceneSession = sptr<WindowSceneSessionImpl>::MakeSptr(option);
    std::shared_ptr<AppExecFwk::Configuration> configuration;
    std::shared_ptr<Global::Resource::ResourceManager> resourceManager;
    windowSceneSession->uiContent_ = nullptr;
    windowSceneSession->UpdateConfigurationForSpecified(configuration, resourceManager);
    windowSceneSession->uiContent_ = std::make_shared<Ace::UIContentMocker>();
    windowSceneSession->UpdateConfigurationForSpecified(configuration, resourceManager);
    sptr<WindowOption> option2 = sptr<WindowOption>::MakeSptr();
    option2->SetWindowName("subWindow");
    option2->SetWindowType(WindowType::WINDOW_TYPE_APP_SUB_WINDOW);
    sptr<WindowSceneSessionImpl> subSession = sptr<WindowSceneSessionImpl>::MakeSptr(option2);
    ASSERT_NE(subSession, nullptr);
    subSession->property_->SetPersistentId(8);
    windowSceneSession->subWindowSessionMap_.clear();
    windowSceneSession->subWindowSessionMap_[windowSceneSession->GetPersistentId()].push_back(subSession);
    windowSceneSession->UpdateConfigurationForSpecified(configuration, resourceManager);
    windowSceneSession->subWindowSessionMap_.clear();
    windowSceneSession->UpdateConfigurationForAll(configuration);
    EXPECT_FALSE(g_errLog.find("scene map size: %{public}u") != std::string::npos);
}

/**
 * @tc.name: UpdateConfigurationForAll02
 * @tc.desc: UpdateConfigurationForAll02 Test
 * @tc.type: FUNC
 */
HWTEST_F(WindowSceneSessionImplTest2, UpdateConfigurationForAll02, TestSize.Level1)
{
    sptr<WindowOption> option = sptr<WindowOption>::MakeSptr();
    option->SetWindowName("UpdateConfigurationForAll02");
    option->SetWindowType(WindowType::SYSTEM_WINDOW_BASE);
    sptr<WindowSceneSessionImpl> window = sptr<WindowSceneSessionImpl>::MakeSptr(option);
    auto abilityContext = std::make_shared<AbilityRuntime::AbilityContextImpl>();
    ASSERT_NE(nullptr, abilityContext);
    SessionInfo sessionInfo = { "CreateTestBundle", "CreateTestModule", "CreateTestAbility" };
    sptr<SessionMocker> session = sptr<SessionMocker>::MakeSptr(sessionInfo);
    ASSERT_EQ(WMError::WM_OK, window->Create(abilityContext, session));
    std::shared_ptr<AppExecFwk::Configuration> configuration;
    std::vector<std::shared_ptr<AbilityRuntime::Context>> ignoreWindowContexts;
    ignoreWindowContexts.push_back(abilityContext);
    window->UpdateConfigurationForAll(configuration, ignoreWindowContexts);
}

/**
 * @tc.name: GetTopWindowWithContext01
 * @tc.desc: GetTopWindowWithContext
 * @tc.type: FUNC
 */
HWTEST_F(WindowSceneSessionImplTest2, GetTopWindowWithContext01, TestSize.Level1)
{
    sptr<WindowOption> option = sptr<WindowOption>::MakeSptr();
    option->SetWindowName("GetTopWindowWithContext01");
    option->SetWindowType(WindowType::SYSTEM_WINDOW_BASE);
    std::shared_ptr<AbilityRuntime::Context> context;
    sptr<WindowSceneSessionImpl> windowSceneSession = sptr<WindowSceneSessionImpl>::MakeSptr(option);
    if (windowSceneSession->GetTopWindowWithContext(context) == nullptr) {
        ASSERT_EQ(nullptr, windowSceneSession->GetTopWindowWithContext(context));
    }
    SessionInfo sessionInfo = { "CreateTestBundle", "CreateTestModule", "CreateTestAbility" };
    sptr<SessionMocker> session = sptr<SessionMocker>::MakeSptr(sessionInfo);
    ASSERT_EQ(WMError::WM_OK, windowSceneSession->Create(abilityContext_, session));
    windowSceneSession->GetTopWindowWithContext(context);
    windowSceneSession->Destroy(true);
}

/**
 * @tc.name: GetMainWindowWithContext01
 * @tc.desc: GetMainWindowWithContext
 * @tc.type: FUNC
 */
HWTEST_F(WindowSceneSessionImplTest2, GetMainWindowWithContext01, TestSize.Level1)
{
    sptr<WindowOption> option = sptr<WindowOption>::MakeSptr();
    option->SetWindowName("GetMainWindowWithContext01");
    option->SetWindowType(WindowType::SYSTEM_WINDOW_BASE);
    std::shared_ptr<AbilityRuntime::Context> context;
    sptr<WindowSceneSessionImpl> windowSceneSession = sptr<WindowSceneSessionImpl>::MakeSptr(option);
    if (windowSceneSession->GetMainWindowWithContext(context) == nullptr) {
        ASSERT_EQ(nullptr, windowSceneSession->GetMainWindowWithContext(context));
    }
    SessionInfo sessionInfo = { "CreateTestBundle", "CreateTestModule", "CreateTestAbility" };
    sptr<SessionMocker> session = sptr<SessionMocker>::MakeSptr(sessionInfo);
    ASSERT_EQ(WMError::WM_OK, windowSceneSession->Create(abilityContext_, session));
    windowSceneSession->GetMainWindowWithContext(context);
    windowSceneSession->Destroy(true);
}

/**
 * @tc.name: NotifyMemoryLevel01
 * @tc.desc: NotifyMemoryLevel
 * @tc.type: FUNC
 */
HWTEST_F(WindowSceneSessionImplTest2, NotifyMemoryLevel01, TestSize.Level1)
{
    sptr<WindowOption> option = sptr<WindowOption>::MakeSptr();
    option->SetWindowName("NotifyMemoryLevel01");
    option->SetWindowType(WindowType::SYSTEM_WINDOW_BASE);
    sptr<WindowSceneSessionImpl> windowSceneSession = sptr<WindowSceneSessionImpl>::MakeSptr(option);
    std::shared_ptr<AppExecFwk::Configuration> configuration;
    windowSceneSession->NotifyMemoryLevel(2);
    ASSERT_EQ(WMError::WM_ERROR_NULLPTR, windowSceneSession->NotifyMemoryLevel(2));
    windowSceneSession->uiContent_ = std::make_unique<Ace::UIContentMocker>();
    ASSERT_EQ(WMError::WM_OK, windowSceneSession->NotifyMemoryLevel(2));
}

/**
 * @tc.name: GetSystemSizeLimits01
 * @tc.desc: GetSystemSizeLimits
 * @tc.type: FUNC
 */
HWTEST_F(WindowSceneSessionImplTest2, GetSystemSizeLimits01, TestSize.Level1)
{
    constexpr uint32_t minMainWidth = 10;
    constexpr uint32_t minMainHeight = 20;
    constexpr uint32_t minSubWidth = 30;
    constexpr uint32_t minSubHeight = 40;
    constexpr uint32_t displayWidth = 100;
    constexpr uint32_t displayHeight = 100;
    constexpr float displayVpr = 1;
    sptr<WindowOption> option = sptr<WindowOption>::MakeSptr();
    option->SetWindowName("GetSystemSizeLimits01");
    option->SetWindowType(WindowType::WINDOW_TYPE_APP_MAIN_WINDOW);
    sptr<WindowSceneSessionImpl> windowSceneSession = sptr<WindowSceneSessionImpl>::MakeSptr(option);
    windowSceneSession->windowSystemConfig_.miniWidthOfMainWindow_ = minMainWidth;
    windowSceneSession->windowSystemConfig_.miniHeightOfMainWindow_ = minMainHeight;
    windowSceneSession->windowSystemConfig_.miniWidthOfSubWindow_ = minSubWidth;
    windowSceneSession->windowSystemConfig_.miniHeightOfSubWindow_ = minSubHeight;

    const auto& [limits, _] = windowSceneSession->GetSystemSizeLimits(displayWidth, displayHeight, displayVpr);
    EXPECT_EQ(limits.minWidth_, minMainWidth);
    EXPECT_EQ(limits.minHeight_, minMainHeight);

    windowSceneSession->SetWindowType(WindowType::WINDOW_TYPE_APP_SUB_WINDOW);
    const auto& [limits2, _2] = windowSceneSession->GetSystemSizeLimits(displayWidth, displayHeight, displayVpr);
    EXPECT_EQ(limits2.minWidth_, minMainWidth);
    EXPECT_EQ(limits2.minHeight_, minMainHeight);

    windowSceneSession->SetWindowType(WindowType::SYSTEM_WINDOW_BASE);
    const auto& [limits3, _3] = windowSceneSession->GetSystemSizeLimits(displayWidth, displayHeight, displayVpr);
    EXPECT_EQ(limits3.minWidth_, minMainWidth);
    EXPECT_EQ(limits3.minHeight_, minMainHeight);
}

/**
 * @tc.name: DumpSessionElementInfo
 * @tc.desc: DumpSessionElementInfo 1: params num
 * @tc.type: FUNC
 */
HWTEST_F(WindowSceneSessionImplTest2, DumpSessionElementInfo1, TestSize.Level1)
{
    sptr<WindowOption> option = sptr<WindowOption>::MakeSptr();
    option->SetWindowName("DumpSessionElementInfo1");
    option->SetWindowType(WindowType::SYSTEM_WINDOW_BASE);
    sptr<WindowSceneSessionImpl> window = sptr<WindowSceneSessionImpl>::MakeSptr(option);
    std::vector<std::string> params;
    params.push_back("-h");
    window->DumpSessionElementInfo(params);
    window->uiContent_ = std::make_unique<Ace::UIContentMocker>();
    ASSERT_EQ(WMError::WM_OK, window->NotifyMemoryLevel(2));
    ASSERT_EQ(WMError::WM_ERROR_INVALID_WINDOW, window->Destroy(false));
}

/**
 * @tc.name: DumpSessionElementInfo
 * @tc.desc: DumpSessionElementInfo2
 * @tc.type: FUNC
 */
HWTEST_F(WindowSceneSessionImplTest2, DumpSessionElementInfo2, TestSize.Level1)
{
    sptr<WindowOption> option = sptr<WindowOption>::MakeSptr();
    option->SetWindowName("DumpSessionElementInfo2");
    option->SetWindowType(WindowType::SYSTEM_WINDOW_BASE);
    sptr<WindowSceneSessionImpl> window = sptr<WindowSceneSessionImpl>::MakeSptr(option);
    std::vector<std::string> params;
    params.push_back("-h");
    window->DumpSessionElementInfo(params);
    params.push_back("-s");
    window->uiContent_ = std::make_unique<Ace::UIContentMocker>();
    window->DumpSessionElementInfo(params);
    ASSERT_EQ(WMError::WM_OK, window->NotifyMemoryLevel(2));
    ASSERT_EQ(WMError::WM_ERROR_INVALID_WINDOW, window->Destroy(false));
}

/**
 * @tc.name: DumpSessionElementInfo
 * @tc.desc: DumpSessionElementInfo3
 * @tc.type: FUNC
 */
HWTEST_F(WindowSceneSessionImplTest2, DumpSessionElementInfo3, TestSize.Level1)
{
    sptr<WindowOption> option = sptr<WindowOption>::MakeSptr();
    option->SetWindowName("DumpSessionElementInfo3");
    option->SetWindowType(WindowType::SYSTEM_WINDOW_BASE);
    sptr<WindowSceneSessionImpl> window = sptr<WindowSceneSessionImpl>::MakeSptr(option);
    std::vector<std::string> params;
    params.push_back("-s");
    window->DumpSessionElementInfo(params);
    window->uiContent_ = std::make_unique<Ace::UIContentMocker>();
    ASSERT_EQ(WMError::WM_OK, window->NotifyMemoryLevel(2));
    ASSERT_EQ(WMError::WM_ERROR_INVALID_WINDOW, window->Destroy(false));
}

/**
 * @tc.name: DisableAppWindowDecor02
 * @tc.desc: DisableAppWindowDecor
 * @tc.type: FUNC
 */
HWTEST_F(WindowSceneSessionImplTest2, DisableAppWindowDecor02, TestSize.Level1)
{
    sptr<WindowOption> option = sptr<WindowOption>::MakeSptr();
    option->SetWindowName("DisableAppWindowDecor02");
    option->SetWindowType(WindowType::SYSTEM_WINDOW_BASE);
    sptr<WindowSceneSessionImpl> windowSceneSession = sptr<WindowSceneSessionImpl>::MakeSptr(option);
    windowSceneSession->DisableAppWindowDecor();
    ASSERT_EQ(WMError::WM_ERROR_INVALID_WINDOW, windowSceneSession->DisableAppWindowDecor());
}

/**
 * @tc.name: RaiseAboveTarget01
 * @tc.desc: RaiseAboveTarget
 * @tc.type: FUNC
 */
HWTEST_F(WindowSceneSessionImplTest2, RaiseAboveTarget01, TestSize.Level1)
{
    sptr<WindowOption> option = sptr<WindowOption>::MakeSptr();
    option->SetWindowName("RaiseAboveTarget01");
    sptr<WindowSceneSessionImpl> windowSceneSessionImpl = sptr<WindowSceneSessionImpl>::MakeSptr(option);
    windowSceneSessionImpl->property_->SetPersistentId(6);
    windowSceneSessionImpl->property_->SetParentPersistentId(0);
    SessionInfo sessionInfo = { "CreateTestBundle", "CreateTestModule", "CreateTestAbility" };
    sptr<SessionMocker> session = sptr<SessionMocker>::MakeSptr(sessionInfo);
    windowSceneSessionImpl->hostSession_ = session;
    auto ret = windowSceneSessionImpl->RaiseAboveTarget(1);
    EXPECT_EQ(WMError::WM_ERROR_INVALID_PARENT, ret);

    windowSceneSessionImpl->property_->SetParentPersistentId(1);
    ret = windowSceneSessionImpl->RaiseAboveTarget(1);
    EXPECT_EQ(WMError::WM_ERROR_INVALID_PARAM, ret);

    sptr<WindowSessionImpl> winSession = sptr<WindowSessionImpl>::MakeSptr(option);
    WindowSessionImpl::subWindowSessionMap_.insert(
        std::make_pair<int32_t, std::vector<sptr<WindowSessionImpl>>>(1, { winSession }));
    winSession->property_->SetPersistentId(6);
    windowSceneSessionImpl->property_->SetWindowType(WindowType::WINDOW_TYPE_APP_MAIN_WINDOW);
    ret = windowSceneSessionImpl->RaiseAboveTarget(6);
    EXPECT_EQ(WMError::WM_ERROR_INVALID_CALLING, ret);

    winSession->state_ = WindowState::STATE_CREATED;
    windowSceneSessionImpl->property_->SetWindowType(WindowType::WINDOW_TYPE_APP_SUB_WINDOW);
    ret = windowSceneSessionImpl->RaiseAboveTarget(6);
    EXPECT_EQ(WMError::WM_DO_NOTHING, ret);

    windowSceneSessionImpl->state_ = WindowState::STATE_SHOWN;
    winSession->state_ = WindowState::STATE_SHOWN;
    ret = windowSceneSessionImpl->RaiseAboveTarget(6);
    EXPECT_EQ(WMError::WM_OK, ret);

    windowSceneSessionImpl->property_->SetPersistentId(3);
    ret = windowSceneSessionImpl->RaiseAboveTarget(6);
    EXPECT_EQ(WMError::WM_OK, ret);

    WindowSessionImpl::subWindowSessionMap_.erase(1);
}

/**
 * @tc.name: RaiseMainWindowAboveTarget_DeviceTypeTest
 * @tc.desc: Test raising main window above target on different devices
 * @tc.type: FUNC
 */
HWTEST_F(WindowSceneSessionImplTest2, RaiseMainWindowAboveTarget_DeviceTypeTest, TestSize.Level1)
{
    sptr<WindowOption> option1 = sptr<WindowOption>::MakeSptr();
    option1->SetWindowName("RaiseMainWindowAboveTarget_DeviceTypeTest_SourceWindow");
    option1->SetWindowType(WindowType::WINDOW_TYPE_APP_MAIN_WINDOW);
    sptr<WindowSceneSessionImpl> sourceSceneSession = sptr<WindowSceneSessionImpl>::MakeSptr(option1);

    sourceSceneSession->state_ = WindowState::STATE_SHOWN;
    sourceSceneSession->property_->SetDisplayId(0);
    sourceSceneSession->property_->SetPersistentId(101);
    sourceSceneSession->windowSystemConfig_.windowUIType_ = WindowUIType::PC_WINDOW;

    sptr<WindowOption> option2 = sptr<WindowOption>::MakeSptr();
    option2->SetWindowName("RaiseMainWindowAboveTarget_DeviceTypeTest_TargetWindow");
    option2->SetWindowType(WindowType::WINDOW_TYPE_APP_MAIN_WINDOW);
    sptr<WindowSceneSessionImpl> targetSceneSession = sptr<WindowSceneSessionImpl>::MakeSptr(option2);

    targetSceneSession->state_ = WindowState::STATE_SHOWN;
    targetSceneSession->property_->SetDisplayId(0);
    targetSceneSession->property_->SetPersistentId(102);
    targetSceneSession->windowSystemConfig_.windowUIType_ = WindowUIType::PC_WINDOW;

    WMError ret = sourceSceneSession->RaiseMainWindowAboveTarget(102);
    EXPECT_NE(WMError::WM_ERROR_DEVICE_NOT_SUPPORT, ret);

    sourceSceneSession->windowSystemConfig_.windowUIType_ = WindowUIType::PAD_WINDOW;
    sourceSceneSession->windowSystemConfig_.freeMultiWindowEnable_ = false;
    sourceSceneSession->windowSystemConfig_.freeMultiWindowSupport_ = false;
    ret = sourceSceneSession->RaiseMainWindowAboveTarget(102);
    EXPECT_EQ(WMError::WM_ERROR_DEVICE_NOT_SUPPORT, ret);

    sourceSceneSession->windowSystemConfig_.freeMultiWindowEnable_ = false;
    sourceSceneSession->windowSystemConfig_.freeMultiWindowSupport_ = false;
    ret = sourceSceneSession->RaiseMainWindowAboveTarget(102);
    EXPECT_NE(WMError::WM_ERROR_DEVICE_NOT_SUPPORT, ret);

    sourceSceneSession->windowSystemConfig_.windowUIType_ = WindowUIType::PHONE_WINDOW;
    sourceSceneSession->windowSystemConfig_.freeMultiWindowEnable_ = false;
    sourceSceneSession->windowSystemConfig_.freeMultiWindowSupport_ = false;
    ret = sourceSceneSession->RaiseMainWindowAboveTarget(102);
    EXPECT_EQ(WMError::WM_ERROR_DEVICE_NOT_SUPPORT, ret);
}

/**
 * @tc.name: SetSubWindowZLevel
 * @tc.desc: SetSubWindowZLevel
 * @tc.type: FUNC
 */
HWTEST_F(WindowSceneSessionImplTest2, SetSubWindowZLevel, TestSize.Level1)
{
    sptr<WindowOption> option = sptr<WindowOption>::MakeSptr();
    option->SetWindowName("SetSubWindowZLevel01");
    sptr<WindowSceneSessionImpl> windowSceneSessionImpl = sptr<WindowSceneSessionImpl>::MakeSptr(option);
    SessionInfo sessionInfo = {"CreateTestBundle", "CreateTestModule", "CreateTestAbility"};
    sptr<SessionMocker> session = sptr<SessionMocker>::MakeSptr(sessionInfo);
    windowSceneSessionImpl->state_ = WindowState::STATE_SHOWN;

    windowSceneSessionImpl->property_->SetPersistentId(2);
    auto ret = windowSceneSessionImpl->SetSubWindowZLevel(10001);
    EXPECT_EQ(WMError::WM_ERROR_INVALID_WINDOW, ret);
    windowSceneSessionImpl->hostSession_ = session;

    windowSceneSessionImpl->property_->SetWindowType(WindowType::WINDOW_TYPE_APP_MAIN_WINDOW);
    ret = windowSceneSessionImpl->SetSubWindowZLevel(10001);
    EXPECT_EQ(WMError::WM_ERROR_INVALID_CALLING, ret);
    windowSceneSessionImpl->property_->SetWindowType(WindowType::WINDOW_TYPE_APP_SUB_WINDOW);

    windowSceneSessionImpl->property_->SetParentPersistentId(INVALID_SESSION_ID);
    ret = windowSceneSessionImpl->SetSubWindowZLevel(10001);
    EXPECT_EQ(WMError::WM_ERROR_INVALID_PARENT, ret);
    windowSceneSessionImpl->property_->SetParentPersistentId(3);

    ret = windowSceneSessionImpl->SetSubWindowZLevel(10001);
    EXPECT_EQ(WMError::WM_ERROR_INVALID_PARAM, ret);

    windowSceneSessionImpl->property_->zLevel_ = 1;
    ret = windowSceneSessionImpl->SetSubWindowZLevel(1);
    EXPECT_EQ(WMError::WM_OK, ret);

    ret = windowSceneSessionImpl->SetSubWindowZLevel(2);
    EXPECT_EQ(WMError::WM_OK, ret);
}

/**
 * @tc.name: GetSubWindowZLevel
 * @tc.desc: GetSubWindowZLevel
 * @tc.type: FUNC
 */
HWTEST_F(WindowSceneSessionImplTest2, GetSubWindowZLevel, TestSize.Level1)
{
    sptr<WindowOption> option = sptr<WindowOption>::MakeSptr();
    option->SetWindowName("GetSubWindowZLevel01");
    sptr<WindowSceneSessionImpl> windowSceneSessionImpl = sptr<WindowSceneSessionImpl>::MakeSptr(option);
    SessionInfo sessionInfo = {"CreateTestBundle", "CreateTestModule", "CreateTestAbility"};
    sptr<SessionMocker> session = sptr<SessionMocker>::MakeSptr(sessionInfo);
    windowSceneSessionImpl->state_ = WindowState::STATE_SHOWN;
    windowSceneSessionImpl->hostSession_ = session;
    windowSceneSessionImpl->property_->zLevel_ = 1;
    int32_t zLevel = 0;

    windowSceneSessionImpl->property_->SetPersistentId(INVALID_SESSION_ID);
    auto ret = windowSceneSessionImpl->GetSubWindowZLevel(zLevel);
    EXPECT_EQ(WMError::WM_ERROR_INVALID_WINDOW, ret);
    windowSceneSessionImpl->property_->SetPersistentId(2);

    windowSceneSessionImpl->property_->SetWindowType(WindowType::WINDOW_TYPE_APP_MAIN_WINDOW);
    ret = windowSceneSessionImpl->GetSubWindowZLevel(zLevel);
    EXPECT_EQ(WMError::WM_ERROR_INVALID_CALLING, ret);
    windowSceneSessionImpl->property_->SetWindowType(WindowType::WINDOW_TYPE_FLOAT);
    ret = windowSceneSessionImpl->GetSubWindowZLevel(zLevel);
    EXPECT_EQ(WMError::WM_ERROR_INVALID_CALLING, ret);

    windowSceneSessionImpl->property_->SetWindowType(WindowType::WINDOW_TYPE_APP_SUB_WINDOW);
    ret = windowSceneSessionImpl->GetSubWindowZLevel(zLevel);
    EXPECT_EQ(WMError::WM_OK, ret);
    EXPECT_EQ(1, zLevel);

    windowSceneSessionImpl->property_->zLevel_ = 2;
    windowSceneSessionImpl->property_->SetWindowType(WindowType::WINDOW_TYPE_DIALOG);
    ret = windowSceneSessionImpl->GetSubWindowZLevel(zLevel);
    EXPECT_EQ(WMError::WM_OK, ret);
    EXPECT_EQ(2, zLevel);
}

/**
 * @tc.name: RaiseMainWindowAboveTarget_WindowStateTest01
 * @tc.desc: Test raising main window above target while window state invalid
 * @tc.type: FUNC
 */
HWTEST_F(WindowSceneSessionImplTest2, RaiseMainWindowAboveTarget_WindowStateTest01, TestSize.Level1)
{
    SessionInfo sessionInfo = {"CreateTestBundle", "CreatTestModule", "CreateTestAbility"};
    sptr<SessionMocker> session = sptr<SessionMocker>::MakeSptr(sessionInfo);

    sptr<WindowOption> option1 = sptr<WindowOption>::MakeSptr();
    option1->SetWindowName("RaiseMainWindowAboveTarget_DeviceTypeTest01_SourceWindow");
    option1->SetWindowType(WindowType::WINDOW_TYPE_APP_MAIN_WINDOW);
    sptr<WindowSceneSessionImpl> sourceSceneSession = sptr<WindowSceneSessionImpl>::MakeSptr(option1);

    sourceSceneSession->state_ = WindowState::STATE_SHOWN;
    sourceSceneSession->property_->SetDisplayId(0);
    sourceSceneSession->property_->SetPersistentId(103);
    sourceSceneSession->windowSystemConfig_.windowUIType_ = WindowUIType::PC_WINDOW;
    sourceSceneSession->hostSession_ = session;

    sptr<WindowOption> option2 = sptr<WindowOption>::MakeSptr();
    option2->SetWindowName("RaiseMainWindowAboveTarget_DeviceTypeTest01_TargetWindow");
    option2->SetWindowType(WindowType::WINDOW_TYPE_APP_MAIN_WINDOW);
    sptr<WindowSceneSessionImpl> targetSceneSession = sptr<WindowSceneSessionImpl>::MakeSptr(option2);

    targetSceneSession->state_ = WindowState::STATE_SHOWN;
    targetSceneSession->property_->SetDisplayId(0);
    targetSceneSession->property_->SetPersistentId(104);
    targetSceneSession->windowSystemConfig_.windowUIType_ = WindowUIType::PC_WINDOW;
    targetSceneSession->hostSession_ = session;

    WindowSceneSessionImpl::windowSessionMap_.insert(std::make_pair(sourceSceneSession->GetWindowName(),
        std::pair<uint64_t, sptr<WindowSessionImpl>>(sourceSceneSession->GetWindowId(), sourceSceneSession)));

    WMError ret = sourceSceneSession->RaiseMainWindowAboveTarget(104);
    EXPECT_EQ(WMError::WM_ERROR_INVALID_WINDOW, ret);

    WindowSceneSessionImpl::windowSessionMap_.insert(std::make_pair(targetSceneSession->GetWindowName(),
        std::pair<uint64_t, sptr<WindowSessionImpl>>(targetSceneSession->GetWindowId(), targetSceneSession)));

    ret = sourceSceneSession->RaiseMainWindowAboveTarget(104);
    EXPECT_NE(WMError::WM_ERROR_INVALID_WINDOW, ret);

    WindowSceneSessionImpl::windowSessionMap_.erase(sourceSceneSession->GetWindowName());
    WindowSceneSessionImpl::windowSessionMap_.erase(targetSceneSession->GetWindowName());
}

/**
 * @tc.name: RaiseMainWindowAboveTarget_WindowStateTest02
 * @tc.desc: Test raising main window above target while window state invalid
 * @tc.type: FUNC
 */
HWTEST_F(WindowSceneSessionImplTest2, RaiseMainWindowAboveTarget_WindowStateTest02, TestSize.Level1)
{
    SessionInfo sessionInfo = {"CreateTestBundle", "CreatTestModule", "CreateTestAbility"};
    sptr<SessionMocker> session = sptr<SessionMocker>::MakeSptr(sessionInfo);

    sptr<WindowOption> option1 = sptr<WindowOption>::MakeSptr();
    option1->SetWindowName("RaiseMainWindowAboveTarget_DeviceTypeTest02_SourceWindow");
    option1->SetWindowType(WindowType::WINDOW_TYPE_APP_MAIN_WINDOW);
    sptr<WindowSceneSessionImpl> sourceSceneSession = sptr<WindowSceneSessionImpl>::MakeSptr(option1);

    sourceSceneSession->state_ = WindowState::STATE_SHOWN;
    sourceSceneSession->property_->SetDisplayId(0);
    sourceSceneSession->property_->SetPersistentId(INVALID_WINDOW_ID);
    sourceSceneSession->windowSystemConfig_.windowUIType_ = WindowUIType::PC_WINDOW;
    sourceSceneSession->hostSession_ = session;

    sptr<WindowOption> option2 = sptr<WindowOption>::MakeSptr();
    option2->SetWindowName("RaiseMainWindowAboveTarget_DeviceTypeTest02_TargetWindow");
    option2->SetWindowType(WindowType::WINDOW_TYPE_APP_MAIN_WINDOW);
    sptr<WindowSceneSessionImpl> targetSceneSession = sptr<WindowSceneSessionImpl>::MakeSptr(option2);

    targetSceneSession->state_ = WindowState::STATE_SHOWN;
    targetSceneSession->property_->SetDisplayId(0);
    targetSceneSession->property_->SetPersistentId(106);
    targetSceneSession->windowSystemConfig_.windowUIType_ = WindowUIType::PC_WINDOW;
    targetSceneSession->hostSession_ = session;

    WindowSceneSessionImpl::windowSessionMap_.insert(std::make_pair(sourceSceneSession->GetWindowName(),
        std::pair<uint64_t, sptr<WindowSessionImpl>>(sourceSceneSession->GetWindowId(), sourceSceneSession)));
    WindowSceneSessionImpl::windowSessionMap_.insert(std::make_pair(targetSceneSession->GetWindowName(),
        std::pair<uint64_t, sptr<WindowSessionImpl>>(targetSceneSession->GetWindowId(), targetSceneSession)));

    WMError ret = sourceSceneSession->RaiseMainWindowAboveTarget(106);
    EXPECT_EQ(WMError::WM_ERROR_INVALID_WINDOW, ret);

    WindowSceneSessionImpl::windowSessionMap_.erase(sourceSceneSession->GetWindowName());
    WindowSceneSessionImpl::windowSessionMap_.erase(targetSceneSession->GetWindowName());
}

/**
 * @tc.name: RaiseMainWindowAboveTarget_WindowStateTest03
 * @tc.desc: Test raising main window above target while window state invalid
 * @tc.type: FUNC
 */
HWTEST_F(WindowSceneSessionImplTest2, RaiseMainWindowAboveTarget_WindowStateTest03, TestSize.Level1)
{
    SessionInfo sessionInfo = {"CreateTestBundle", "CreatTestModule", "CreateTestAbility"};
    sptr<SessionMocker> session = sptr<SessionMocker>::MakeSptr(sessionInfo);

    sptr<WindowOption> option1 = sptr<WindowOption>::MakeSptr();
    option1->SetWindowName("RaiseMainWindowAboveTarget_DeviceTypeTest03_SourceWindow");
    option1->SetWindowType(WindowType::WINDOW_TYPE_APP_MAIN_WINDOW);
    sptr<WindowSceneSessionImpl> sourceSceneSession = sptr<WindowSceneSessionImpl>::MakeSptr(option1);

    sourceSceneSession->state_ = WindowState::STATE_SHOWN;
    sourceSceneSession->property_->SetDisplayId(0);
    sourceSceneSession->property_->SetPersistentId(107);
    sourceSceneSession->windowSystemConfig_.windowUIType_ = WindowUIType::PC_WINDOW;
    sourceSceneSession->hostSession_ = session;

    sptr<WindowOption> option2 = sptr<WindowOption>::MakeSptr();
    option2->SetWindowName("RaiseMainWindowAboveTarget_DeviceTypeTest03_TargetWindow");
    option2->SetWindowType(WindowType::WINDOW_TYPE_APP_MAIN_WINDOW);
    sptr<WindowSceneSessionImpl> targetSceneSession = sptr<WindowSceneSessionImpl>::MakeSptr(option2);

    targetSceneSession->state_ = WindowState::STATE_SHOWN;
    targetSceneSession->property_->SetDisplayId(0);
    targetSceneSession->property_->SetPersistentId(108);
    targetSceneSession->windowSystemConfig_.windowUIType_ = WindowUIType::PC_WINDOW;
    targetSceneSession->hostSession_ = session;

    WindowSceneSessionImpl::windowSessionMap_.insert(std::make_pair(sourceSceneSession->GetWindowName(),
        std::pair<uint64_t, sptr<WindowSessionImpl>>(sourceSceneSession->GetWindowId(), sourceSceneSession)));
    WindowSceneSessionImpl::windowSessionMap_.insert(std::make_pair(targetSceneSession->GetWindowName(),
        std::pair<uint64_t, sptr<WindowSessionImpl>>(targetSceneSession->GetWindowId(), nullptr)));

    WMError ret = sourceSceneSession->RaiseMainWindowAboveTarget(108);
    EXPECT_EQ(WMError::WM_ERROR_INVALID_WINDOW, ret);

    WindowSceneSessionImpl::windowSessionMap_["RaiseMainWindowAboveTarget_DeviceTypeTest03_TargetWindow"] =
        std::make_pair(targetSceneSession->GetWindowId(), targetSceneSession);
    ret = sourceSceneSession->RaiseMainWindowAboveTarget(108);
    EXPECT_NE(WMError::WM_ERROR_INVALID_WINDOW, ret);

    targetSceneSession->hostSession_ = nullptr;
    ret = sourceSceneSession->RaiseMainWindowAboveTarget(108);
    EXPECT_EQ(WMError::WM_ERROR_INVALID_WINDOW, ret);

    targetSceneSession->hostSession_ = session;
    targetSceneSession->state_ = WindowState::STATE_DESTROYED;
    ret = sourceSceneSession->RaiseMainWindowAboveTarget(108);
    EXPECT_EQ(WMError::WM_ERROR_INVALID_WINDOW, ret);

    targetSceneSession->state_ = WindowState::STATE_SHOWN;
    sourceSceneSession->state_ = WindowState::STATE_HIDDEN;
    ret = sourceSceneSession->RaiseMainWindowAboveTarget(108);
    EXPECT_EQ(WMError::WM_ERROR_INVALID_WINDOW, ret);

    WindowSceneSessionImpl::windowSessionMap_.erase(sourceSceneSession->GetWindowName());
    WindowSceneSessionImpl::windowSessionMap_.erase(targetSceneSession->GetWindowName());
}

/**
 * @tc.name: RaiseMainWindowAboveTarget_WindowTypeTest
 * @tc.desc: Test raising main window above target while window type invalid
 * @tc.type: FUNC
 */
HWTEST_F(WindowSceneSessionImplTest2, RaiseMainWindowAboveTarget_WindowTypeTest, TestSize.Level1)
{
    SessionInfo sessionInfo = {"CreateTestBundle", "CreatTestModule", "CreateTestAbility"};
    sptr<SessionMocker> session = sptr<SessionMocker>::MakeSptr(sessionInfo);

    sptr<WindowOption> option1 = sptr<WindowOption>::MakeSptr();
    option1->SetWindowName("RaiseMainWindowAboveTarget_WindowTypeTest_SourceWindow");
    option1->SetWindowType(WindowType::WINDOW_TYPE_APP_SUB_WINDOW);
    sptr<WindowSceneSessionImpl> sourceSceneSession = sptr<WindowSceneSessionImpl>::MakeSptr(option1);

    sourceSceneSession->state_ = WindowState::STATE_SHOWN;
    sourceSceneSession->property_->SetDisplayId(0);
    sourceSceneSession->property_->SetPersistentId(109);
    sourceSceneSession->windowSystemConfig_.windowUIType_ = WindowUIType::PC_WINDOW;
    sourceSceneSession->hostSession_ = session;

    sptr<WindowOption> option2 = sptr<WindowOption>::MakeSptr();
    option2->SetWindowName("RaiseMainWindowAboveTarget_WindowTypeTest_TargetWindow");
    option2->SetWindowType(WindowType::WINDOW_TYPE_APP_MAIN_WINDOW);
    sptr<WindowSceneSessionImpl> targetSceneSession = sptr<WindowSceneSessionImpl>::MakeSptr(option2);

    targetSceneSession->state_ = WindowState::STATE_SHOWN;
    targetSceneSession->property_->SetDisplayId(0);
    targetSceneSession->property_->SetPersistentId(110);
    targetSceneSession->windowSystemConfig_.windowUIType_ = WindowUIType::PC_WINDOW;
    targetSceneSession->hostSession_ = session;

    WindowSceneSessionImpl::windowSessionMap_.insert(std::make_pair(sourceSceneSession->GetWindowName(),
        std::pair<uint64_t, sptr<WindowSessionImpl>>(sourceSceneSession->GetWindowId(), sourceSceneSession)));
    WindowSceneSessionImpl::windowSessionMap_.insert(std::make_pair(targetSceneSession->GetWindowName(),
        std::pair<uint64_t, sptr<WindowSessionImpl>>(targetSceneSession->GetWindowId(), targetSceneSession)));

    WMError ret = sourceSceneSession->RaiseMainWindowAboveTarget(110);
    EXPECT_EQ(WMError::WM_ERROR_INVALID_CALLING, ret);

    ret = targetSceneSession->RaiseMainWindowAboveTarget(110);
    EXPECT_NE(WMError::WM_ERROR_INVALID_CALLING, ret);

    WindowSceneSessionImpl::windowSessionMap_.erase(sourceSceneSession->GetWindowName());
    WindowSceneSessionImpl::windowSessionMap_.erase(targetSceneSession->GetWindowName());
}

/**
 * @tc.name: RaiseMainWindowAboveTarget_SpecialHierarchyTest01
 * @tc.desc: Test raising main window above target while window is modal or topmost, etc.
 * @tc.type: FUNC
 */
HWTEST_F(WindowSceneSessionImplTest2, RaiseMainWindowAboveTarget_SpecialHierarchyTest01, TestSize.Level1)
{
    SessionInfo sessionInfo = {"CreateTestBundle", "CreatTestModule", "CreateTestAbility"};
    sptr<SessionMocker> session = sptr<SessionMocker>::MakeSptr(sessionInfo);

    sptr<WindowOption> option1 = sptr<WindowOption>::MakeSptr();
    option1->SetWindowName("RaiseMainWindowAboveTarget_SpecialHierarchyTest01_SourceWindow");
    option1->SetWindowType(WindowType::WINDOW_TYPE_APP_MAIN_WINDOW);
    sptr<WindowSceneSessionImpl> sourceSceneSession = sptr<WindowSceneSessionImpl>::MakeSptr(option1);

    sourceSceneSession->state_ = WindowState::STATE_SHOWN;
    sourceSceneSession->property_->SetDisplayId(0);
    sourceSceneSession->property_->SetPersistentId(111);
    sourceSceneSession->windowSystemConfig_.windowUIType_ = WindowUIType::PC_WINDOW;
    sourceSceneSession->hostSession_ = session;

    sptr<WindowOption> option2 = sptr<WindowOption>::MakeSptr();
    option2->SetWindowName("RaiseMainWindowAboveTarget_SpecialHierarchyTest01_TargetWindow");
    option2->SetWindowType(WindowType::WINDOW_TYPE_APP_MAIN_WINDOW);
    sptr<WindowSceneSessionImpl> targetSceneSession = sptr<WindowSceneSessionImpl>::MakeSptr(option2);

    targetSceneSession->state_ = WindowState::STATE_SHOWN;
    targetSceneSession->property_->SetDisplayId(0);
    targetSceneSession->property_->SetPersistentId(112);
    targetSceneSession->windowSystemConfig_.windowUIType_ = WindowUIType::PC_WINDOW;
    targetSceneSession->hostSession_ = session;
    targetSceneSession->AddWindowFlag(WindowFlag::WINDOW_FLAG_IS_MODAL);

    WindowSceneSessionImpl::windowSessionMap_.insert(std::make_pair(sourceSceneSession->GetWindowName(),
        std::pair<uint64_t, sptr<WindowSessionImpl>>(sourceSceneSession->GetWindowId(), sourceSceneSession)));
    WindowSceneSessionImpl::windowSessionMap_.insert(std::make_pair(targetSceneSession->GetWindowName(),
        std::pair<uint64_t, sptr<WindowSessionImpl>>(targetSceneSession->GetWindowId(), targetSceneSession)));

    WMError ret = sourceSceneSession->RaiseMainWindowAboveTarget(112);
    EXPECT_EQ(WMError::WM_ERROR_INVALID_CALLING, ret);

    targetSceneSession->RemoveWindowFlag(WindowFlag::WINDOW_FLAG_IS_MODAL);
    sourceSceneSession->AddWindowFlag(WindowFlag::WINDOW_FLAG_IS_MODAL);
    ret = sourceSceneSession->RaiseMainWindowAboveTarget(112);
    EXPECT_EQ(WMError::WM_ERROR_INVALID_CALLING, ret);

    sourceSceneSession->RemoveWindowFlag(WindowFlag::WINDOW_FLAG_IS_MODAL);
    ret = sourceSceneSession->RaiseMainWindowAboveTarget(112);
    EXPECT_NE(WMError::WM_ERROR_INVALID_CALLING, ret);

    sptr<WindowOption> subOption = sptr<WindowOption>::MakeSptr();
    subOption->SetWindowName("RaiseMainWindowAboveTarget_SpecialHierarchyTest01_ModalitySubWindow");
    subOption->SetWindowType(WindowType::WINDOW_TYPE_APP_SUB_WINDOW);
    sptr<WindowSceneSessionImpl> modalitySubWindow = sptr<WindowSceneSessionImpl>::MakeSptr(subOption);

    modalitySubWindow->property_->SetParentPersistentId(sourceSceneSession->GetPersistentId());
    modalitySubWindow->state_ = WindowState::STATE_SHOWN;
    modalitySubWindow->property_->AddWindowFlag(WindowFlag::WINDOW_FLAG_IS_MODAL);
    modalitySubWindow->property_->AddWindowFlag(WindowFlag::WINDOW_FLAG_IS_APPLICATION_MODAL);
    sourceSceneSession->subWindowSessionMap_[sourceSceneSession->GetPersistentId()].push_back(modalitySubWindow);

    ret = sourceSceneSession->RaiseMainWindowAboveTarget(112);
    EXPECT_EQ(WMError::WM_ERROR_INVALID_CALLING, ret);

    sourceSceneSession->subWindowSessionMap_[sourceSceneSession->GetPersistentId()].pop_back();
    ret = sourceSceneSession->RaiseMainWindowAboveTarget(112);
    EXPECT_EQ(WMError::WM_OK, ret);

    WindowSceneSessionImpl::windowSessionMap_.erase(sourceSceneSession->GetWindowName());
    WindowSceneSessionImpl::windowSessionMap_.erase(targetSceneSession->GetWindowName());
    sourceSceneSession->subWindowSessionMap_.erase(sourceSceneSession->GetPersistentId());
}

/**
 * @tc.name: RaiseMainWindowAboveTarget_SpecialHierarchyTest02
 * @tc.desc: Test raising main window above target while window is modal or topmost, etc.
 * @tc.type: FUNC
 */
HWTEST_F(WindowSceneSessionImplTest2, RaiseMainWindowAboveTarget_SpecialHierarchyTest02, TestSize.Level1)
{
    SessionInfo sessionInfo = {"CreateTestBundle", "CreatTestModule", "CreateTestAbility"};
    sptr<SessionMocker> session = sptr<SessionMocker>::MakeSptr(sessionInfo);

    sptr<WindowOption> option1 = sptr<WindowOption>::MakeSptr();
    option1->SetWindowName("RaiseMainWindowAboveTarget_SpecialHierarchyTest02_SourceWindow");
    option1->SetWindowType(WindowType::WINDOW_TYPE_APP_MAIN_WINDOW);
    sptr<WindowSceneSessionImpl> sourceSceneSession = sptr<WindowSceneSessionImpl>::MakeSptr(option1);

    sourceSceneSession->state_ = WindowState::STATE_SHOWN;
    sourceSceneSession->property_->SetDisplayId(0);
    sourceSceneSession->property_->SetPersistentId(113);
    sourceSceneSession->windowSystemConfig_.windowUIType_ = WindowUIType::PC_WINDOW;
    sourceSceneSession->hostSession_ = session;
    sourceSceneSession->property_->topmost_ = true;

    sptr<WindowOption> option2 = sptr<WindowOption>::MakeSptr();
    option2->SetWindowName("RaiseMainWindowAboveTarget_SpecialHierarchyTest02_TargetWindow");
    option2->SetWindowType(WindowType::WINDOW_TYPE_APP_MAIN_WINDOW);
    sptr<WindowSceneSessionImpl> targetSceneSession = sptr<WindowSceneSessionImpl>::MakeSptr(option2);

    targetSceneSession->state_ = WindowState::STATE_SHOWN;
    targetSceneSession->property_->SetDisplayId(0);
    targetSceneSession->property_->SetPersistentId(114);
    targetSceneSession->windowSystemConfig_.windowUIType_ = WindowUIType::PC_WINDOW;
    targetSceneSession->hostSession_ = session;

    WindowSceneSessionImpl::windowSessionMap_.insert(std::make_pair(sourceSceneSession->GetWindowName(),
        std::pair<uint64_t, sptr<WindowSessionImpl>>(sourceSceneSession->GetWindowId(), sourceSceneSession)));
    WindowSceneSessionImpl::windowSessionMap_.insert(std::make_pair(targetSceneSession->GetWindowName(),
        std::pair<uint64_t, sptr<WindowSessionImpl>>(targetSceneSession->GetWindowId(), targetSceneSession)));

    WMError ret = sourceSceneSession->RaiseMainWindowAboveTarget(114);
    EXPECT_EQ(WMError::WM_ERROR_INVALID_CALLING, ret);

    sourceSceneSession->property_->topmost_ = false;
    targetSceneSession->property_->topmost_ = true;
    ret = sourceSceneSession->RaiseMainWindowAboveTarget(114);
    EXPECT_EQ(WMError::WM_ERROR_INVALID_CALLING, ret);

    targetSceneSession->property_->topmost_ = false;
    sourceSceneSession->property_->mainWindowTopmost_ = true;
    ret = sourceSceneSession->RaiseMainWindowAboveTarget(114);
    EXPECT_EQ(WMError::WM_ERROR_INVALID_CALLING, ret);

    sourceSceneSession->property_->mainWindowTopmost_ = false;
    targetSceneSession->property_->mainWindowTopmost_ = true;
    ret = sourceSceneSession->RaiseMainWindowAboveTarget(114);
    EXPECT_EQ(WMError::WM_ERROR_INVALID_CALLING, ret);

    targetSceneSession->property_->mainWindowTopmost_ = false;
    ret = sourceSceneSession->RaiseMainWindowAboveTarget(114);
    EXPECT_NE(WMError::WM_ERROR_INVALID_CALLING, ret);

    WindowSceneSessionImpl::windowSessionMap_.erase(sourceSceneSession->GetWindowName());
    WindowSceneSessionImpl::windowSessionMap_.erase(targetSceneSession->GetWindowName());
}

/**
 * @tc.name: FindParentSessionByParentId02
 * @tc.desc: FindParentSessionByParentId
 * @tc.type: FUNC
 */
HWTEST_F(WindowSceneSessionImplTest2, FindParentSessionByParentId02, TestSize.Level1)
{
    sptr<WindowOption> option = sptr<WindowOption>::MakeSptr();
    option->SetWindowTag(WindowTag::MAIN_WINDOW);
    option->SetWindowName("FindParentSessionByParentId02");
    sptr<WindowSceneSessionImpl> parentSceneSession = sptr<WindowSceneSessionImpl>::MakeSptr(option);
    ASSERT_EQ(nullptr, parentSceneSession->FindParentSessionByParentId(-1));
}

/**
 * @tc.name: GetConfigurationFromAbilityInfo02
 * @tc.desc: GetConfigurationFromAbilityInfo
 * @tc.type: FUNC
 */
HWTEST_F(WindowSceneSessionImplTest2, GetConfigurationFromAbilityInfo02, TestSize.Level1)
{
    g_errLog.clear();
    LOG_SetCallback(MyLogCallback);
    sptr<WindowOption> option = sptr<WindowOption>::MakeSptr();
    option->SetWindowName("GetConfigurationFromAbilityInfo02");
    option->SetWindowType(WindowType::SYSTEM_WINDOW_BASE);
    sptr<WindowSceneSessionImpl> windowSceneSession = sptr<WindowSceneSessionImpl>::MakeSptr(option);
    windowSceneSession->GetConfigurationFromAbilityInfo();
    EXPECT_TRUE(g_errLog.find("abilityContext is nullptr") != std::string::npos);
    LOG_SetCallback(nullptr);
}

/**
 * @tc.name: NotifyWindowSessionProperty01
 * @tc.desc: NotifyWindowSessionProperty
 * @tc.type: FUNC
 */
HWTEST_F(WindowSceneSessionImplTest2, NotifyWindowSessionProperty01, TestSize.Level1)
{
    g_errLog.clear();
    LOG_SetCallback(MyLogCallback);
    sptr<WindowOption> option = sptr<WindowOption>::MakeSptr();
    option->SetWindowName("NotifyWindowSessionProperty01");
    option->SetWindowType(WindowType::SYSTEM_WINDOW_BASE);
    sptr<WindowSceneSessionImpl> windowSceneSession = sptr<WindowSceneSessionImpl>::MakeSptr(option);
    windowSceneSession->NotifyWindowSessionProperty();
    EXPECT_TRUE(g_errLog.find("session is invalid") != std::string::npos);
    ASSERT_EQ(WMError::WM_ERROR_INVALID_WINDOW, windowSceneSession->NotifyWindowSessionProperty());
    LOG_SetCallback(nullptr);
}

HWTEST_F(WindowSceneSessionImplTest2, IsTransparent01, TestSize.Level1)
{
    g_errLog.clear();
    LOG_SetCallback(MyLogCallback);
    sptr<WindowOption> option = sptr<WindowOption>::MakeSptr();
    option->SetWindowName("IsTransparent01");
    option->SetWindowType(WindowType::SYSTEM_WINDOW_BASE);
    sptr<WindowSceneSessionImpl> windowSceneSession = sptr<WindowSceneSessionImpl>::MakeSptr(option);
    windowSceneSession->IsTransparent();
    EXPECT_FALSE(g_errLog.find("color: %{public}u, alpha: %{public}u") != std::string::npos);
    LOG_SetCallback(nullptr);
}

/**
 * @tc.name: SetTransparent02
 * @tc.desc: SetTransparent
 * @tc.type: FUNC
 */
HWTEST_F(WindowSceneSessionImplTest2, SetTransparent02, TestSize.Level1)
{
    g_errLog.clear();
    LOG_SetCallback(MyLogCallback);
    sptr<WindowOption> option = sptr<WindowOption>::MakeSptr();
    option->SetWindowName("SetTransparent02");
    option->SetWindowType(WindowType::SYSTEM_WINDOW_BASE);
    sptr<WindowSceneSessionImpl> windowSceneSession = sptr<WindowSceneSessionImpl>::MakeSptr(option);
    bool isTransparent = false;
    windowSceneSession->SetTransparent(isTransparent);
    EXPECT_TRUE(g_errLog.find("session is invalid") != std::string::npos);
    ASSERT_EQ(WMError::WM_ERROR_INVALID_WINDOW, windowSceneSession->SetTransparent(isTransparent));
    LOG_SetCallback(nullptr);
}

/**
 * @tc.name: Snapshot01
 * @tc.desc: Snapshot
 * @tc.type: FUNC
 */
HWTEST_F(WindowSceneSessionImplTest2, Snapshot01, TestSize.Level1)
{
    sptr<WindowOption> option = sptr<WindowOption>::MakeSptr();
    option->SetWindowName("Snapshot01");
    option->SetWindowType(WindowType::SYSTEM_WINDOW_BASE);
    sptr<WindowSceneSessionImpl> windowSceneSession = sptr<WindowSceneSessionImpl>::MakeSptr(option);
    windowSceneSession->Snapshot();
    ASSERT_EQ(nullptr, windowSceneSession->Snapshot());
}

/**
 * @tc.name: SnapshotSync01
 * @tc.desc: Test WMError::WM_ERROR_INVALID_WINDOW
 * @tc.type: FUNC
 */
HWTEST_F(WindowSceneSessionImplTest2, SnapshotSync01, TestSize.Level1)
{
    sptr<WindowOption> option = sptr<WindowOption>::MakeSptr();
    sptr<WindowSceneSessionImpl> windowSceneSession = sptr<WindowSceneSessionImpl>::MakeSptr(option);
    ASSERT_NE(nullptr, windowSceneSession);
    std::shared_ptr<Media::PixelMap> pixelMap = nullptr;
    EXPECT_EQ(WMError::WM_ERROR_INVALID_WINDOW, windowSceneSession->Snapshot(pixelMap));

    windowSceneSession->property_->SetPersistentId(1);
    SessionInfo sessionInfo = {"CreateTestBundle", "CreatTestModule", "CreateTestAbility"};
    sptr<SessionMocker> session = sptr<SessionMocker>::MakeSptr(sessionInfo);
    windowSceneSession->hostSession_ = session;
    windowSceneSession->state_ = WindowState::STATE_CREATED;
    EXPECT_EQ(WMError::WM_ERROR_INVALID_WINDOW, windowSceneSession->Snapshot(pixelMap));
}

/**
 * @tc.name: SnapshotSync02
 * @tc.desc: Test WMError::WM_ERROR_INVALID_OPERATION
 * @tc.type: FUNC
 */
HWTEST_F(WindowSceneSessionImplTest2, SnapshotSync02, TestSize.Level1)
{
    auto windowSceneSession = InitialWindowState();
    ASSERT_NE(nullptr, windowSceneSession);
    windowSceneSession->surfaceNode_ = nullptr;
    std::shared_ptr<Media::PixelMap> pixelMap = nullptr;
    EXPECT_EQ(WMError::WM_ERROR_INVALID_OPERATION, windowSceneSession->Snapshot(pixelMap));
}

/**
 * @tc.name: SnapshotSync03
 * @tc.desc: Test WMError::WM_ERROR_TIMEOUT
 * @tc.type: FUNC
 */
HWTEST_F(WindowSceneSessionImplTest2, SnapshotSync03, TestSize.Level1)
{
    auto windowSceneSession = InitialWindowState();
    ASSERT_NE(nullptr, windowSceneSession);
    std::shared_ptr<Media::PixelMap> pixelMap = nullptr;
    auto surfaceNode_mocker = CreateRSSurfaceNode();
    ASSERT_NE(nullptr, surfaceNode_mocker);
    windowSceneSession->surfaceNode_ = surfaceNode_mocker;
    EXPECT_EQ(WMError::WM_ERROR_TIMEOUT, windowSceneSession->Snapshot(pixelMap));
}

/**
 * @tc.name: SnapshotIgnorePrivacy
 * @tc.desc: SnapshotIgnorePrivacy
 * @tc.type: FUNC
 */
HWTEST_F(WindowSceneSessionImplTest2, SnapshotIgnorePrivacy, TestSize.Level1)
{
    sptr<WindowOption> option = sptr<WindowOption>::MakeSptr();
    sptr<WindowSceneSessionImpl> windowSceneSession = sptr<WindowSceneSessionImpl>::MakeSptr(option);
    std::shared_ptr<Media::PixelMap> pixelMap = nullptr;
    ASSERT_EQ(WMError::WM_ERROR_INVALID_WINDOW, windowSceneSession->SnapshotIgnorePrivacy(pixelMap));

    windowSceneSession->property_->SetPersistentId(1);
    SessionInfo sessionInfo = {"CreateTestBundle", "CreatTestModule", "CreateTestAbility"};
    sptr<SessionMocker> session = sptr<SessionMocker>::MakeSptr(sessionInfo);
    windowSceneSession->hostSession_ = session;
    ASSERT_EQ(WMError::WM_ERROR_NULLPTR, windowSceneSession->SnapshotIgnorePrivacy(pixelMap));

    windowSceneSession->surfaceNode_ = nullptr;
    ASSERT_EQ(WMError::WM_ERROR_INVALID_OPERATION, windowSceneSession->SnapshotIgnorePrivacy(pixelMap));
}

/**
 * @tc.name: BindDialogTarget01
 * @tc.desc: BindDialogTarget
 * @tc.type: FUNC
 */
HWTEST_F(WindowSceneSessionImplTest2, BindDialogTarget01, TestSize.Level1)
{
    sptr<WindowOption> option = sptr<WindowOption>::MakeSptr();
    option->SetWindowName("BindDialogTarget01");
    option->SetWindowType(WindowType::SYSTEM_WINDOW_BASE);
    sptr<WindowSceneSessionImpl> windowSceneSession = sptr<WindowSceneSessionImpl>::MakeSptr(option);
    sptr<IRemoteObject> targetToken;
    EXPECT_EQ(WMError::WM_ERROR_INVALID_WINDOW, windowSceneSession->BindDialogTarget(targetToken));
}

/**
 * @tc.name: BindDialogTarget02
 * @tc.desc: BindDialogTarget
 * @tc.type: FUNC
 */
HWTEST_F(WindowSceneSessionImplTest2, BindDialogTarget02, TestSize.Level1)
{
    sptr<WindowOption> option = sptr<WindowOption>::MakeSptr();
    option->SetWindowName("BindDialogTarget01");
    option->SetWindowType(WindowType::SYSTEM_WINDOW_BASE);
    sptr<WindowSceneSessionImpl> windowSceneSession = sptr<WindowSceneSessionImpl>::MakeSptr(option);

    SessionInfo sessionInfo = { "CreateTestBundle", "CreateTestMode", "CreateTestAbility" };
    sptr<SessionMocker> session = sptr<SessionMocker>::MakeSptr(sessionInfo);
    windowSceneSession->property_->SetPersistentId(1);
    windowSceneSession->hostSession_ = session;
    sptr<IRemoteObject> targetToken;
    WMError ret = windowSceneSession->BindDialogTarget(targetToken);
    ASSERT_EQ(ret, WMError::WM_ERROR_NULLPTR);
}

/**
 * @tc.name: SetDialogBackGestureEnabled01
 * @tc.desc: SetDialogBackGestureEnabled
 * @tc.type: FUNC
 */
HWTEST_F(WindowSceneSessionImplTest2, SetDialogBackGestureEnabled01, TestSize.Level1)
{
    sptr<WindowOption> option = sptr<WindowOption>::MakeSptr();
    option->SetWindowName("SetDialogBackGestureEnabled01");
    option->SetWindowType(WindowType::SYSTEM_WINDOW_BASE);
    sptr<WindowSceneSessionImpl> windowSceneSession = sptr<WindowSceneSessionImpl>::MakeSptr(option);
    WMError ret = windowSceneSession->SetDialogBackGestureEnabled(true);
    ASSERT_EQ(ret, WMError::WM_ERROR_INVALID_CALLING);
}

/**
 * @tc.name: SetDialogBackGestureEnabled02
 * @tc.desc: SetDialogBackGestureEnabled
 * @tc.type: FUNC
 */
HWTEST_F(WindowSceneSessionImplTest2, SetDialogBackGestureEnabled02, TestSize.Level1)
{
    sptr<WindowOption> option = sptr<WindowOption>::MakeSptr();
    option->SetWindowName("SetDialogBackGestureEnabled02");
    option->SetWindowType(WindowType::WINDOW_TYPE_DIALOG);
    sptr<WindowSceneSessionImpl> windowSceneSession = sptr<WindowSceneSessionImpl>::MakeSptr(option);
    WMError ret = windowSceneSession->SetDialogBackGestureEnabled(true);
    ASSERT_EQ(ret, WMError::WM_ERROR_NULLPTR);
}

/**
 * @tc.name: SetDialogBackGestureEnabled03
 * @tc.desc: SetDialogBackGestureEnabled
 * @tc.type: FUNC
 */
HWTEST_F(WindowSceneSessionImplTest2, SetDialogBackGestureEnabled03, TestSize.Level1)
{
    sptr<WindowOption> option = sptr<WindowOption>::MakeSptr();
    option->SetWindowName("SetDialogBackGestureEnabled02");
    option->SetWindowType(WindowType::WINDOW_TYPE_DIALOG);
    sptr<WindowSceneSessionImpl> windowSceneSession = sptr<WindowSceneSessionImpl>::MakeSptr(option);

    SessionInfo sessionInfo = {"CreateTestBundle", "CreateTestModule", "CreateTestAbility"};
    sptr<SessionMocker> session = sptr<SessionMocker>::MakeSptr(sessionInfo);
    windowSceneSession->hostSession_ = session;
    windowSceneSession->property_->SetPersistentId(1);
    WMError ret = windowSceneSession->SetDialogBackGestureEnabled(true);
    ASSERT_EQ(ret, WMError::WM_OK);
}

/**
 * @tc.name: NotifySessionForeground
 * @tc.desc: NotifySessionForeground
 * @tc.type: FUNC
 */
HWTEST_F(WindowSceneSessionImplTest2, NotifySessionForeground, TestSize.Level1)
{
    sptr<WindowOption> option = sptr<WindowOption>::MakeSptr();
    option->SetWindowName("NotifySessionForeground");
    option->SetWindowType(WindowType::SYSTEM_WINDOW_BASE);
    sptr<WindowSceneSessionImpl> windowSceneSession = sptr<WindowSceneSessionImpl>::MakeSptr(option);
    ASSERT_NE(nullptr, windowSceneSession);

    uint32_t reason = 1;
    bool withAnimation = true;
    windowSceneSession->NotifySessionForeground(reason, withAnimation);
    ASSERT_EQ(false, windowSceneSession->GetDefaultDensityEnabled());
}

/**
 * @tc.name: NotifySessionBackground
 * @tc.desc: NotifySessionBackground
 * @tc.type: FUNC
 */
HWTEST_F(WindowSceneSessionImplTest2, NotifySessionBackground, TestSize.Level1)
{
    sptr<WindowOption> option = sptr<WindowOption>::MakeSptr();
    option->SetWindowName("NotifySessionBackground");
    option->SetWindowType(WindowType::SYSTEM_WINDOW_BASE);
    sptr<WindowSceneSessionImpl> windowSceneSession = sptr<WindowSceneSessionImpl>::MakeSptr(option);
    ASSERT_NE(nullptr, windowSceneSession);

    uint32_t reason = 1;
    bool withAnimation = true;
    bool isFromInnerkits = true;
    windowSceneSession->NotifySessionBackground(reason, withAnimation, isFromInnerkits);
    ASSERT_EQ(false, windowSceneSession->GetDefaultDensityEnabled());
}

/**
 * @tc.name: NotifySessionFullScreen
 * @tc.desc: NotifySessionFullScreen
 * @tc.type: FUNC
 */
HWTEST_F(WindowSceneSessionImplTest2, NotifySessionFullScreen, TestSize.Level1)
{
    sptr<WindowOption> option = sptr<WindowOption>::MakeSptr();
    option->SetWindowName("NotifySessionFullScreen");
    option->SetWindowType(WindowType::SYSTEM_WINDOW_BASE);
    sptr<WindowSceneSessionImpl> windowSceneSession = sptr<WindowSceneSessionImpl>::MakeSptr(option);
    ASSERT_NE(nullptr, windowSceneSession);

    bool fullScreen = true;
    windowSceneSession->NotifySessionFullScreen(fullScreen);
}

/**
 * @tc.name: NotifyPrepareClosePiPWindow01
 * @tc.desc: NotifyPrepareClosePiPWindow
 * @tc.type: FUNC
 */
HWTEST_F(WindowSceneSessionImplTest2, NotifyPrepareClosePiPWindow01, TestSize.Level1)
{
    sptr<WindowOption> option = sptr<WindowOption>::MakeSptr();
    option->SetWindowName("NotifyPrepareClosePiPWindow01");
    option->SetWindowType(WindowType::WINDOW_TYPE_PIP);
    sptr<WindowSceneSessionImpl> windowSceneSession = sptr<WindowSceneSessionImpl>::MakeSptr(option);
    SessionInfo sessionInfo = { "CreateTestBundle", "CreateTestModule", "CreateTestAbility" };
    sptr<SessionMocker> session = sptr<SessionMocker>::MakeSptr(sessionInfo);
    windowSceneSession->hostSession_ = session;
    ASSERT_EQ(WMError::WM_OK, windowSceneSession->NotifyPrepareClosePiPWindow());
}

/**
 * @tc.name: SetDefaultDensityEnabled01
 * @tc.desc: normal test
 * @tc.type: FUNC
 */
HWTEST_F(WindowSceneSessionImplTest2, SetDefaultDensityEnabled01, TestSize.Level1)
{
    sptr<WindowOption> option = sptr<WindowOption>::MakeSptr();
    option->SetWindowName("SetDefaultDensityEnabled01");
    option->SetWindowType(WindowType::WINDOW_TYPE_APP_MAIN_WINDOW);
    sptr<WindowSceneSessionImpl> window = sptr<WindowSceneSessionImpl>::MakeSptr(option);

    SessionInfo sessionInfo = { "CreateTestBundle", "CreateTestModule", "CreateTestAbility" };
    sptr<SessionMocker> session = sptr<SessionMocker>::MakeSptr(sessionInfo);
    window->property_->SetPersistentId(1);
    window->hostSession_ = session;
    window->state_ = WindowState::STATE_SHOWN;
    window->property_->SetDisplayId(0);

    WindowSceneSessionImpl::windowSessionMap_.insert(std::make_pair(
        window->GetWindowName(), std::pair<uint64_t, sptr<WindowSessionImpl>>(window->GetWindowId(), window)));
    ASSERT_EQ(WMError::WM_OK, window->SetDefaultDensityEnabled(true));
    ASSERT_EQ(true, window->GetDefaultDensityEnabled());
    WindowSceneSessionImpl::windowSessionMap_.erase(window->GetWindowName());
}

/**
 * @tc.name: SetDefaultDensityEnabled02
 * @tc.desc: window session is invalid
 * @tc.type: FUNC
 */
HWTEST_F(WindowSceneSessionImplTest2, SetDefaultDensityEnabled02, TestSize.Level1)
{
    sptr<WindowOption> option = sptr<WindowOption>::MakeSptr();
    option->SetWindowName("SetDefaultDensityEnabled02");
    option->SetWindowType(WindowType::WINDOW_TYPE_APP_MAIN_WINDOW);
    sptr<WindowSceneSessionImpl> window = sptr<WindowSceneSessionImpl>::MakeSptr(option);
    SessionInfo sessionInfo = { "CreateTestBundle", "CreateTestModule", "CreateTestAbility" };
    sptr<SessionMocker> session = sptr<SessionMocker>::MakeSptr(sessionInfo);
    window->property_->SetPersistentId(1);
    window->state_ = WindowState::STATE_SHOWN;
    ASSERT_EQ(WMError::WM_ERROR_INVALID_WINDOW, window->SetDefaultDensityEnabled(true));
}

/**
 * @tc.name: SetDefaultDensityEnabled03
 * @tc.desc: not app main window
 * @tc.type: FUNC
 */
HWTEST_F(WindowSceneSessionImplTest2, SetDefaultDensityEnabled03, TestSize.Level1)
{
    sptr<WindowOption> option = sptr<WindowOption>::MakeSptr();
    option->SetWindowName("SetDefaultDensityEnabled03");
    option->SetWindowType(WindowType::WINDOW_TYPE_APP_SUB_WINDOW);
    sptr<WindowSceneSessionImpl> window = sptr<WindowSceneSessionImpl>::MakeSptr(option);
    SessionInfo sessionInfo = { "CreateTestBundle", "CreateTestModule", "CreateTestAbility" };
    sptr<SessionMocker> session = sptr<SessionMocker>::MakeSptr(sessionInfo);
    window->property_->SetPersistentId(1);
    window->hostSession_ = session;
    window->state_ = WindowState::STATE_SHOWN;
    ASSERT_EQ(WMError::WM_ERROR_INVALID_CALLING, window->SetDefaultDensityEnabled(true));
}

/**
 * @tc.name: SetDefaultDensityEnabled04
 * @tc.desc: isDefaultDensityEnabled_ not change
 * @tc.type: FUNC
 */
HWTEST_F(WindowSceneSessionImplTest2, SetDefaultDensityEnabled04, TestSize.Level1)
{
    sptr<WindowOption> option = sptr<WindowOption>::MakeSptr();
    option->SetWindowName("SetDefaultDensityEnabled04");
    option->SetWindowType(WindowType::WINDOW_TYPE_APP_MAIN_WINDOW);
    sptr<WindowSceneSessionImpl> window = sptr<WindowSceneSessionImpl>::MakeSptr(option);
    SessionInfo sessionInfo = { "CreateTestBundle", "CreateTestModule", "CreateTestAbility" };
    sptr<SessionMocker> session = sptr<SessionMocker>::MakeSptr(sessionInfo);
    window->property_->SetPersistentId(1);
    window->hostSession_ = session;
    window->state_ = WindowState::STATE_SHOWN;
    ASSERT_EQ(WMError::WM_OK, window->SetDefaultDensityEnabled(true));
    ASSERT_EQ(WMError::WM_OK, window->SetDefaultDensityEnabled(true));
}

/**
 * @tc.name: GetDefaultDensityEnabled01
 * @tc.desc: normal test
 * @tc.type: FUNC
 */
HWTEST_F(WindowSceneSessionImplTest2, GetDefaultDensityEnabled01, TestSize.Level1)
{
    sptr<WindowOption> option = sptr<WindowOption>::MakeSptr();
    option->SetWindowName("GetDefaultDensityEnabled01");
    sptr<WindowSceneSessionImpl> window = sptr<WindowSceneSessionImpl>::MakeSptr(option);
    window->isDefaultDensityEnabled_ = true;
    ASSERT_EQ(true, window->GetDefaultDensityEnabled());
}

/**
 * @tc.name: GetDefaultDensityEnabled02
 * @tc.desc: test default value
 * @tc.type: FUNC
 */
HWTEST_F(WindowSceneSessionImplTest2, GetDefaultDensityEnabled02, TestSize.Level1)
{
    sptr<WindowOption> option = sptr<WindowOption>::MakeSptr();
    option->SetWindowName("GetDefaultDensityEnabled02");
    sptr<WindowSceneSessionImpl> window = sptr<WindowSceneSessionImpl>::MakeSptr(option);
    window->isDefaultDensityEnabled_ = true;
    ASSERT_EQ(true, window->GetDefaultDensityEnabled());
}

/**
 * @tc.name: SetWindowDefaultDensityEnabled01
 * @tc.desc: SetWindowDefaultDensityEnabled
 * @tc.type: FUNC
 */
HWTEST_F(WindowSceneSessionImplTest2, SetWindowDefaultDensityEnabled01, TestSize.Level1)
{
    sptr<WindowOption> option = sptr<WindowOption>::MakeSptr();
    option->SetWindowName("SetWindowDefaultDensityEnabled");
    option->SetWindowType(WindowType::WINDOW_TYPE_APP_MAIN_WINDOW);
    sptr<WindowSceneSessionImpl> window = sptr<WindowSceneSessionImpl>::MakeSptr(option);

    auto res = window->SetWindowDefaultDensityEnabled(true);
    EXPECT_EQ(WMError::WM_ERROR_INVALID_WINDOW, res);

    SessionInfo sessionInfo = { "CreateTestBundle", "CreateTestModule", "CreateTestAbility" };
    sptr<SessionMocker> session = sptr<SessionMocker>::MakeSptr(sessionInfo);
    window->property_->SetPersistentId(1);
    window->hostSession_ = session;
    window->state_ = WindowState::STATE_SHOWN;
    res = window->SetWindowDefaultDensityEnabled(true);
    EXPECT_EQ(WMError::WM_OK, res);
}

/**
 * @tc.name: SetDefaultDensityEnabledValue01
 * @tc.desc: SetDefaultDensityEnabledValue
 * @tc.type: FUNC
 */
HWTEST_F(WindowSceneSessionImplTest2, SetDefaultDensityEnabledValue01, TestSize.Level1)
{
    sptr<WindowOption> option = sptr<WindowOption>::MakeSptr();
    option->SetWindowName("SetDefaultDensityEnabledValue");
    option->SetWindowType(WindowType::WINDOW_TYPE_APP_MAIN_WINDOW);
    sptr<WindowSceneSessionImpl> window = sptr<WindowSceneSessionImpl>::MakeSptr(option);

    window->SetDefaultDensityEnabledValue(true);
    EXPECT_EQ(true, window->GetDefaultDensityEnabled());

    window->SetDefaultDensityEnabledValue(false);
    EXPECT_EQ(false, window->GetDefaultDensityEnabled());
}

/**
 * @tc.name: GetVirtualPixelRatio01
 * @tc.desc: main window isDefaultDensityEnabled_ true
 * @tc.type: FUNC
 */
HWTEST_F(WindowSceneSessionImplTest2, GetVirtualPixelRatio01, TestSize.Level1)
{
    sptr<WindowOption> option = sptr<WindowOption>::MakeSptr();
    option->SetWindowName("GetVirtualPixelRatio01");
    option->SetWindowType(WindowType::WINDOW_TYPE_APP_MAIN_WINDOW);
    sptr<WindowSceneSessionImpl> window = sptr<WindowSceneSessionImpl>::MakeSptr(option);

    SessionInfo sessionInfo = { "CreateTestBundle", "CreateTestModule", "CreateTestAbility" };
    sptr<SessionMocker> session = sptr<SessionMocker>::MakeSptr(sessionInfo);
    window->property_->SetPersistentId(1);
    window->hostSession_ = session;
    window->state_ = WindowState::STATE_SHOWN;
    window->property_->SetDisplayId(0);

    window->isDefaultDensityEnabled_ = true;
    sptr<DisplayInfo> displayInfo = sptr<DisplayInfo>::MakeSptr();
    auto defautDensity = 1.0f;
    auto density = 2.0f;
    displayInfo->SetDefaultVirtualPixelRatio(defautDensity);
    displayInfo->SetVirtualPixelRatio(density);
    ASSERT_EQ(defautDensity, window->GetVirtualPixelRatio(displayInfo));
}

/**
 * @tc.name: GetVirtualPixelRatio02
 * @tc.desc: main window isDefaultDensityEnabled_ false
 * @tc.type: FUNC
 */
HWTEST_F(WindowSceneSessionImplTest2, GetVirtualPixelRatio02, TestSize.Level1)
{
    sptr<WindowOption> option = sptr<WindowOption>::MakeSptr();
    option->SetWindowName("GetVirtualPixelRatio02");
    option->SetWindowType(WindowType::WINDOW_TYPE_APP_MAIN_WINDOW);
    sptr<WindowSceneSessionImpl> window = sptr<WindowSceneSessionImpl>::MakeSptr(option);

    SessionInfo sessionInfo = { "CreateTestBundle", "CreateTestModule", "CreateTestAbility" };
    sptr<SessionMocker> session = sptr<SessionMocker>::MakeSptr(sessionInfo);
    window->property_->SetPersistentId(1);
    window->hostSession_ = session;
    window->state_ = WindowState::STATE_SHOWN;
    window->property_->SetDisplayId(0);

    window->isDefaultDensityEnabled_ = false;
    sptr<DisplayInfo> displayInfo = sptr<DisplayInfo>::MakeSptr();
    auto defautDensity = 1.0f;
    auto density = 2.0f;
    displayInfo->SetDefaultVirtualPixelRatio(defautDensity);
    displayInfo->SetVirtualPixelRatio(density);
    ASSERT_EQ(density, window->GetVirtualPixelRatio(displayInfo));

    window->useUniqueDensity_ = true;
    ASSERT_EQ(window->virtualPixelRatio_, window->GetVirtualPixelRatio(displayInfo));
}

/**
 * @tc.name: GetVirtualPixelRatio03
 * @tc.desc: sub window isDefaultDensityEnabled_ true
 * @tc.type: FUNC
 */
HWTEST_F(WindowSceneSessionImplTest2, GetVirtualPixelRatio03, TestSize.Level1)
{
    sptr<WindowOption> option = sptr<WindowOption>::MakeSptr();
    option->SetWindowName("GetVirtualPixelRatio03");
    option->SetWindowType(WindowType::WINDOW_TYPE_APP_MAIN_WINDOW);
    sptr<WindowSceneSessionImpl> window = sptr<WindowSceneSessionImpl>::MakeSptr(option);

    SessionInfo sessionInfo = { "CreateTestBundle", "CreateTestModule", "CreateTestAbility" };
    sptr<SessionMocker> session = sptr<SessionMocker>::MakeSptr(sessionInfo);
    window->property_->SetPersistentId(1);
    window->hostSession_ = session;
    window->state_ = WindowState::STATE_SHOWN;
    window->property_->SetDisplayId(0);

    window->isDefaultDensityEnabled_ = true;
    sptr<DisplayInfo> displayInfo = sptr<DisplayInfo>::MakeSptr();
    auto defautDensity = 1.0f;
    auto density = 2.0f;
    displayInfo->SetDefaultVirtualPixelRatio(defautDensity);
    displayInfo->SetVirtualPixelRatio(density);

    sptr<WindowOption> subOption = sptr<WindowOption>::MakeSptr();
    subOption->SetWindowName("GetVirtualPixelRatio03SubWindow");
    subOption->SetWindowType(WindowType::WINDOW_TYPE_APP_SUB_WINDOW);
    sptr<WindowSceneSessionImpl> subWindow = sptr<WindowSceneSessionImpl>::MakeSptr(subOption);
    subWindow->property_->SetParentPersistentId(window->GetWindowId());

    displayInfo->SetDefaultVirtualPixelRatio(defautDensity);
    displayInfo->SetVirtualPixelRatio(defautDensity);
    WindowSceneSessionImpl::windowSessionMap_.insert(std::make_pair(
        window->GetWindowName(), std::pair<uint64_t, sptr<WindowSessionImpl>>(window->GetWindowId(), window)));
    ASSERT_EQ(defautDensity, subWindow->GetVirtualPixelRatio(displayInfo));
    WindowSceneSessionImpl::windowSessionMap_.erase(window->GetWindowName());
}

/**
 * @tc.name: GetVirtualPixelRatio04
 * @tc.desc: sub window isDefaultDensityEnabled_ false
 * @tc.type: FUNC
 */
HWTEST_F(WindowSceneSessionImplTest2, GetVirtualPixelRatio04, TestSize.Level1)
{
    sptr<WindowOption> option = sptr<WindowOption>::MakeSptr();
    option->SetWindowName("GetVirtualPixelRatio04");
    option->SetWindowType(WindowType::WINDOW_TYPE_APP_MAIN_WINDOW);
    sptr<WindowSceneSessionImpl> window = sptr<WindowSceneSessionImpl>::MakeSptr(option);

    SessionInfo sessionInfo = { "CreateTestBundle", "CreateTestModule", "CreateTestAbility" };
    sptr<SessionMocker> session = sptr<SessionMocker>::MakeSptr(sessionInfo);
    window->property_->SetPersistentId(1);
    window->hostSession_ = session;
    window->state_ = WindowState::STATE_SHOWN;
    window->property_->SetDisplayId(0);

    window->isDefaultDensityEnabled_ = false;
    sptr<DisplayInfo> displayInfo = sptr<DisplayInfo>::MakeSptr();
    auto defautDensity = 1.0f;
    auto density = 2.0f;
    displayInfo->SetDefaultVirtualPixelRatio(defautDensity);
    displayInfo->SetVirtualPixelRatio(density);

    sptr<WindowOption> subOption = sptr<WindowOption>::MakeSptr();
    subOption->SetWindowName("GetVirtualPixelRatio04SubWindow");
    subOption->SetWindowType(WindowType::WINDOW_TYPE_APP_SUB_WINDOW);
    sptr<WindowSceneSessionImpl> subWindow = sptr<WindowSceneSessionImpl>::MakeSptr(subOption);
    subWindow->property_->SetParentPersistentId(window->GetWindowId());

    WindowSceneSessionImpl::windowSessionMap_.insert(std::make_pair(
        window->GetWindowName(), std::pair<uint64_t, sptr<WindowSessionImpl>>(window->GetWindowId(), window)));
    displayInfo->SetDefaultVirtualPixelRatio(defautDensity);
    displayInfo->SetVirtualPixelRatio(defautDensity);
    ASSERT_EQ(defautDensity, subWindow->GetVirtualPixelRatio(displayInfo));
    WindowSceneSessionImpl::windowSessionMap_.erase(window->GetWindowName());
}

/**
 * @tc.name: GetWindowLimits01
 * @tc.desc: GetWindowLimits
 * @tc.type: FUNC
 */
HWTEST_F(WindowSceneSessionImplTest2, GetWindowLimits01, TestSize.Level1)
{
    sptr<WindowOption> option = sptr<WindowOption>::MakeSptr();
    option->SetWindowName("GetWindowLimits01");
    sptr<WindowSceneSessionImpl> window = sptr<WindowSceneSessionImpl>::MakeSptr(option);
    WindowLimits windowLimits = { 1000, 1000, 1000, 1000, 0.0f, 0.0f };
    window->property_->SetWindowType(WindowType::WINDOW_TYPE_APP_MAIN_WINDOW);
    window->property_->SetWindowLimits(windowLimits);
    window->property_->SetPersistentId(1);
    window->state_ = WindowState::STATE_HIDDEN;
    SessionInfo sessionInfo = { "CreateTestBundle", "CreateTestModule", "CreateTestAbility" };
    sptr<SessionMocker> session = sptr<SessionMocker>::MakeSptr(sessionInfo);
    window->hostSession_ = session;

    WindowLimits windowSizeLimits;
    ASSERT_EQ(WMError::WM_OK, window->GetWindowLimits(windowSizeLimits));
    ASSERT_EQ(windowSizeLimits.maxWidth_, 1000);
    ASSERT_EQ(windowSizeLimits.maxHeight_, 1000);
    ASSERT_EQ(windowSizeLimits.minWidth_, 1000);
    ASSERT_EQ(windowSizeLimits.minHeight_, 1000);
}

/**
 * @tc.name: AdjustKeyboardLayout01
 * @tc.desc: adjust keyboard layout
 * @tc.type: FUNC
 */
HWTEST_F(WindowSceneSessionImplTest2, AdjustKeyboardLayout01, TestSize.Level1)
{
    sptr<WindowOption> option = sptr<WindowOption>::MakeSptr();
    option->SetWindowName("AdjustKeyboardLayout01");
    option->SetDisplayId(0);
    sptr<WindowSceneSessionImpl> window = sptr<WindowSceneSessionImpl>::MakeSptr(option);
    window->property_->SetPersistentId(123);
    window->property_->SetWindowType(WindowType::WINDOW_TYPE_APP_MAIN_WINDOW);
    window->state_ = WindowState::STATE_FROZEN;
    SessionInfo sessionInfo = { "CreateTestBundle", "CreateTestModule", "CreateTestAbility" };
    sptr<SessionMocker> session = sptr<SessionMocker>::MakeSptr(sessionInfo);
    window->hostSession_ = session;

    KeyboardLayoutParams params;
    params.gravity_ = WindowGravity::WINDOW_GRAVITY_FLOAT;
    params.LandscapeKeyboardRect_ = { 1, 2, 3, 4 };
    params.PortraitKeyboardRect_ = { 1, 2, 3, 4 };
    params.LandscapePanelRect_ = { 1, 2, 3, 4 };
    params.PortraitPanelRect_ = { 1, 2, 3, 4 };
    ASSERT_EQ(WMError::WM_OK, window->AdjustKeyboardLayout(params));
}

/**
 * @tc.name: HideNonSecureWindows01
 * @tc.desc: HideNonSecureWindows
 * @tc.type: FUNC
 */
HWTEST_F(WindowSceneSessionImplTest2, HideNonSecureWindows01, TestSize.Level1)
{
    sptr<WindowOption> option = sptr<WindowOption>::MakeSptr();
    option->SetWindowName("HideNonSecureWindows01");
    sptr<WindowSceneSessionImpl> window = sptr<WindowSceneSessionImpl>::MakeSptr(option);
    ASSERT_EQ(WMError::WM_OK, window->HideNonSecureWindows(true));
}

/**
 * @tc.name: SetGrayScale01
 * @tc.desc: SetGrayScale
 * @tc.type: FUNC
 */
HWTEST_F(WindowSceneSessionImplTest2, SetGrayScale01, TestSize.Level1)
{
    sptr<WindowOption> option = sptr<WindowOption>::MakeSptr();
    option->SetWindowMode(WindowMode::WINDOW_MODE_PIP);
    sptr<WindowSceneSessionImpl> window = sptr<WindowSceneSessionImpl>::MakeSptr(option);
    constexpr float grayScale = 0.5f;
    ASSERT_EQ(WMError::WM_ERROR_INVALID_WINDOW, window->SetGrayScale(grayScale));
}

/**
 * @tc.name: SetGrayScale02
 * @tc.desc: SetGrayScale
 * @tc.type: FUNC
 */
HWTEST_F(WindowSceneSessionImplTest2, SetGrayScale02, TestSize.Level1)
{
    sptr<WindowOption> option = sptr<WindowOption>::MakeSptr();
    sptr<WindowSceneSessionImpl> window = sptr<WindowSceneSessionImpl>::MakeSptr(option);

    window->state_ = WindowState::STATE_SHOWN;
    SessionInfo sessionInfo = { "CreateTestBundle", "CreateTestModule", "CreateTestAbility" };
    sptr<SessionMocker> session = sptr<SessionMocker>::MakeSptr(sessionInfo);
    window->property_->SetPersistentId(1);
    window->hostSession_ = session;
    window->uiContent_ = std::make_unique<Ace::UIContentMocker>();

    std::vector<WindowType> types = { WindowType::WINDOW_TYPE_APP_MAIN_WINDOW,
                                      WindowType::WINDOW_TYPE_APP_SUB_WINDOW,
                                      WindowType::SYSTEM_WINDOW_BASE };
    for (WindowType type : types) {
        window->SetWindowType(type);
        float grayScale = -0.001f;
        ASSERT_EQ(WMError::WM_ERROR_INVALID_PARAM, window->SetGrayScale(grayScale));
        grayScale = 1.001f;
        ASSERT_EQ(WMError::WM_ERROR_INVALID_PARAM, window->SetGrayScale(grayScale));
    }
}

/**
 * @tc.name: SetGrayScale03
 * @tc.desc: SetGrayScale
 * @tc.type: FUNC
 */
HWTEST_F(WindowSceneSessionImplTest2, SetGrayScale03, TestSize.Level1)
{
    sptr<WindowOption> option = sptr<WindowOption>::MakeSptr();
    sptr<WindowSceneSessionImpl> window = sptr<WindowSceneSessionImpl>::MakeSptr(option);
    window->state_ = WindowState::STATE_SHOWN;
    SessionInfo sessionInfo = { "CreateTestBundle", "CreateTestModule", "CreateTestAbility" };
    sptr<SessionMocker> session = sptr<SessionMocker>::MakeSptr(sessionInfo);
    window->property_->SetPersistentId(1);
    window->hostSession_ = session;
    ASSERT_EQ(WMError::WM_ERROR_NULLPTR, window->SetGrayScale(0.5));
    window->uiContent_ = std::make_unique<Ace::UIContentMocker>();
    std::vector<WindowType> types = { WindowType::WINDOW_TYPE_APP_MAIN_WINDOW,
                                      WindowType::WINDOW_TYPE_APP_SUB_WINDOW,
                                      WindowType::SYSTEM_WINDOW_BASE };
    for (WindowType type : types) {
        window->SetWindowType(type);
        float grayScale = 0.0f;
        ASSERT_EQ(WMError::WM_OK, window->SetGrayScale(grayScale));
        grayScale = 1.0f;
        ASSERT_EQ(WMError::WM_OK, window->SetGrayScale(grayScale));
        grayScale = 0.5f;
        ASSERT_EQ(WMError::WM_OK, window->SetGrayScale(grayScale));
    }
}

/**
 * @tc.name: SetGrayScale04
 * @tc.desc: Infinite non-circulating decimals
 * @tc.type: FUNC
 */
HWTEST_F(WindowSceneSessionImplTest2, SetGrayScale04, TestSize.Level1)
{
    sptr<WindowOption> option = sptr<WindowOption>::MakeSptr();
    sptr<WindowSceneSessionImpl> window = sptr<WindowSceneSessionImpl>::MakeSptr(option);
    window->state_ = WindowState::STATE_SHOWN;
    SessionInfo sessionInfo = { "CreateTestBundle", "CreateTestModule", "CreateTestAbility" };
    sptr<SessionMocker> session = sptr<SessionMocker>::MakeSptr(sessionInfo);
    window->property_->SetPersistentId(1);
    window->hostSession_ = session;
    ASSERT_EQ(WMError::WM_ERROR_NULLPTR, window->SetGrayScale(0.5));
    window->uiContent_ = std::make_unique<Ace::UIContentMocker>();
    std::vector<WindowType> types = { WindowType::WINDOW_TYPE_APP_MAIN_WINDOW,
                                      WindowType::WINDOW_TYPE_APP_SUB_WINDOW,
                                      WindowType::SYSTEM_WINDOW_BASE };
    for (WindowType type : types) {
        window->SetWindowType(type);
        float grayScale = 1.0f / 3.0f;
        ASSERT_EQ(WMError::WM_OK, window->SetGrayScale(grayScale));
    }
}

/**
 * @tc.name: Maximize02
 * @tc.desc: test errorCode
 * @tc.type: FUNC
 */
HWTEST_F(WindowSceneSessionImplTest2, Maximize02, TestSize.Level1)
{
    sptr<WindowOption> option = sptr<WindowOption>::MakeSptr();
    option->SetWindowName("Maximize02");
    option->SetDisplayId(0);

    sptr<WindowSceneSessionImpl> window = sptr<WindowSceneSessionImpl>::MakeSptr(option);
    MaximizePresentation presentation = MaximizePresentation::ENTER_IMMERSIVE;
    ASSERT_EQ(WMError::WM_ERROR_INVALID_WINDOW, window->Maximize(presentation));

    window->property_->SetWindowName("Maximize02");
    window->property_->SetWindowType(WindowType::WINDOW_TYPE_APP_SUB_WINDOW);
    window->property_->SetPersistentId(2);

    SessionInfo sessionInfo = { "CreateTestBundle", "CreateTestModule", "CreateTestAbility" };
    sptr<SessionMocker> session = sptr<SessionMocker>::MakeSptr(sessionInfo);
    window->hostSession_ = session;
    // not support subWindow call
    ASSERT_EQ(WMError::WM_ERROR_INVALID_CALLING, window->Maximize(presentation));

    // window not support fullscreen
    window->property_->SetWindowType(WindowType::WINDOW_TYPE_APP_MAIN_WINDOW);
    window->property_->SetWindowModeSupportType(WindowModeSupport::WINDOW_MODE_SUPPORT_PIP);
    ASSERT_EQ(WMError::WM_ERROR_INVALID_WINDOW, window->Maximize(presentation));
}

/**
 * @tc.name: Maximize03
 * @tc.desc: test presentation
 * @tc.type: FUNC
 */
HWTEST_F(WindowSceneSessionImplTest2, Maximize03, TestSize.Level1)
{
    sptr<WindowOption> option = sptr<WindowOption>::MakeSptr();
    option->SetWindowName("Maximize03");
    option->SetDisplayId(0);

    sptr<WindowSceneSessionImpl> window = sptr<WindowSceneSessionImpl>::MakeSptr(option);
    window->property_->SetWindowName("Maximize03");
    window->property_->SetWindowType(WindowType::WINDOW_TYPE_APP_MAIN_WINDOW);
    window->property_->SetPersistentId(2);

    SessionInfo sessionInfo = { "CreateTestBundle", "CreateTestModule", "CreateTestAbility" };
    sptr<SessionMocker> session = sptr<SessionMocker>::MakeSptr(sessionInfo);
    window->hostSession_ = session;

    // case1: only set maximize()
    MaximizePresentation presentation = MaximizePresentation::ENTER_IMMERSIVE;
    window->windowSystemConfig_.windowUIType_ = WindowUIType::PHONE_WINDOW;
    auto ret = window->Maximize(presentation);
    ASSERT_EQ(WMError::WM_OK, ret);
    window->windowSystemConfig_.windowUIType_ = WindowUIType::PC_WINDOW;
    ret = window->Maximize(presentation);
    ASSERT_EQ(WMError::WM_OK, ret);
    ASSERT_EQ(window->GetImmersiveModeEnabledState(), true);

    // case2: maximize(EXIT_IMMERSIVE) and the immersive value will be set ad false
    presentation = MaximizePresentation::EXIT_IMMERSIVE;
    window->windowSystemConfig_.windowUIType_ = WindowUIType::PHONE_WINDOW;
    ret = window->Maximize(presentation);
    ASSERT_EQ(WMError::WM_OK, ret);
    window->windowSystemConfig_.windowUIType_ = WindowUIType::PC_WINDOW;
    ret = window->Maximize(presentation);
    ASSERT_EQ(WMError::WM_OK, ret);
    ASSERT_EQ(window->GetImmersiveModeEnabledState(), false);

    // case3: maximize(FOLLOW_APP_IMMERSIVE_SETTING) and the immersive value will be set as client set
    presentation = MaximizePresentation::FOLLOW_APP_IMMERSIVE_SETTING;
    window->windowSystemConfig_.windowUIType_ = WindowUIType::PHONE_WINDOW;
    ret = window->Maximize(presentation);
    ASSERT_EQ(WMError::WM_OK, ret);
    window->windowSystemConfig_.windowUIType_ = WindowUIType::PC_WINDOW;
    ret = window->Maximize(presentation);
    ASSERT_EQ(WMError::WM_OK, ret);
    ASSERT_EQ(window->GetImmersiveModeEnabledState(), false);

    // case4: maximize(ENTER_IMMERSIVE_DISABLE_TITLE_AND_DOCK_HOVER) and the immersive value will be set as true
    presentation = MaximizePresentation::ENTER_IMMERSIVE_DISABLE_TITLE_AND_DOCK_HOVER;
    window->windowSystemConfig_.windowUIType_ = WindowUIType::PHONE_WINDOW;
    ret = window->Maximize(presentation);
    ASSERT_EQ(WMError::WM_OK, ret);
    window->windowSystemConfig_.windowUIType_ = WindowUIType::PC_WINDOW;
    ret = window->Maximize(presentation);
    ASSERT_EQ(WMError::WM_OK, ret);
    ASSERT_EQ(window->GetImmersiveModeEnabledState(), true);
}

/**
 * @tc.name: Test01
 * @tc.desc: Test01
 * @tc.type: FUNC
 */
HWTEST_F(WindowSceneSessionImplTest2, Test01, TestSize.Level1)
{
    WindowLimits windowLimits = { 3, 3, 3, 3, 2.0, 2.0 };
    ASSERT_EQ(windowLimits.IsEmpty(), false);
    Rect rect = { 2, 2, 2, 2 };
    Rect rect1 = { 2, 2, 2, 2 };
    ASSERT_EQ(rect.IsInsideOf(rect1), true);
    TitleButtonRect titleButtonRect = { 3, 3, 3, 3 };
    TitleButtonRect titleButtonRect1 = { 3, 3, 3, 3 };
    ASSERT_EQ(titleButtonRect.IsUninitializedRect(), false);
    ASSERT_EQ(titleButtonRect.IsInsideOf(titleButtonRect1), true);
    AvoidArea avoidArea;
    AvoidArea avoidArea1;
    ASSERT_EQ((avoidArea != avoidArea1), false);
}

/**
 * @tc.name: GetTitleButtonVisible01
 * @tc.desc: GetTitleButtonVisible
 * @tc.type: FUNC
 */
HWTEST_F(WindowSceneSessionImplTest2, GetTitleButtonVisible01, TestSize.Level1)
{
    sptr<WindowOption> option = sptr<WindowOption>::MakeSptr();
    option->SetWindowName("GetTitleButtonVisible01");
    sptr<WindowSceneSessionImpl> window = sptr<WindowSceneSessionImpl>::MakeSptr(option);
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
 * @tc.name: GetTitleButtonVisible02
 * @tc.desc: GetTitleButtonVisible
 * @tc.type: FUNC
 */
HWTEST_F(WindowSceneSessionImplTest2, GetTitleButtonVisible02, TestSize.Level1)
{
    g_errLog.clear();
    LOG_SetCallback(MyLogCallback);
    sptr<WindowOption> option = sptr<WindowOption>::MakeSptr();
    option->SetWindowName("GetTitleButtonVisible02");
    sptr<WindowSceneSessionImpl> window = sptr<WindowSceneSessionImpl>::MakeSptr(option);
    // only not support WINDOW_MODE_SUPPORT_SPLIT
    uint32_t windowModeSupportType = 1 | (1 << 1);
    window->property_->SetWindowModeSupportType(windowModeSupportType);
    window->property_->SetWindowMode(WindowMode::WINDOW_MODE_FLOATING);
    // show Maximize, Minimize, Split buttons.
    window->windowTitleVisibleFlags_ = { true, true, true, true };
    bool hideMaximizeButton = false;
    bool hideMinimizeButton = false;
    bool hideSplitButton = false;
    bool hideCloseButton = false;
    window->windowSystemConfig_.windowUIType_ = WindowUIType::PC_WINDOW;
    window->GetTitleButtonVisible(hideMaximizeButton, hideMinimizeButton, hideSplitButton, hideCloseButton);
    EXPECT_FALSE(g_errLog.find("device not support") != std::string::npos);
    LOG_SetCallback(nullptr);
}

/**
 * @tc.name: GetTitleButtonVisible03
 * @tc.desc: GetTitleButtonVisible
 * @tc.type: FUNC
 */
HWTEST_F(WindowSceneSessionImplTest2, GetTitleButtonVisible03, TestSize.Level1)
{
    g_errLog.clear();
    LOG_SetCallback(MyLogCallback);
    sptr<WindowOption> option = sptr<WindowOption>::MakeSptr();
    option->SetWindowName("GetTitleButtonVisible03");
    sptr<WindowSceneSessionImpl> window = sptr<WindowSceneSessionImpl>::MakeSptr(option);
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
    EXPECT_TRUE(g_errLog.find("device not support") != std::string::npos);
    LOG_SetCallback(nullptr);
}

/**
 * @tc.name: SetTitleButtonVisible01
 * @tc.desc: SetTitleButtonVisible
 * @tc.type: FUNC
 */
HWTEST_F(WindowSceneSessionImplTest2, SetTitleButtonVisible01, TestSize.Level1)
{
    sptr<WindowOption> option = sptr<WindowOption>::MakeSptr();
    option->SetWindowName("SetTitleButtonVisible");
    sptr<WindowSceneSessionImpl> window = sptr<WindowSceneSessionImpl>::MakeSptr(option);
    ASSERT_NE(window, nullptr);
    window->property_->SetWindowType(WindowType::WINDOW_TYPE_APP_SUB_WINDOW);
    window->SetTitleButtonVisible(false, false, false, true);
}

/**
 * @tc.name: SetTitleButtonVisible02
 * @tc.desc: SetTitleButtonVisible
 * @tc.type: FUNC
 */
HWTEST_F(WindowSceneSessionImplTest2, SetTitleButtonVisible02, TestSize.Level1)
{
    sptr<WindowOption> option = sptr<WindowOption>::MakeSptr();
    option->SetWindowName("SetTitleButtonVisible");
    sptr<WindowSceneSessionImpl> window = sptr<WindowSceneSessionImpl>::MakeSptr(option);
    window->property_->SetWindowType(WindowType::APP_MAIN_WINDOW_BASE);
    WMError res = window->SetTitleButtonVisible(false, false, false, true);
    ASSERT_EQ(res, WMError::WM_ERROR_INVALID_WINDOW);
}

/**
 * @tc.name: SetTitleButtonVisible03
 * @tc.desc: SetTitleButtonVisible
 * @tc.type: FUNC
 */
HWTEST_F(WindowSceneSessionImplTest2, SetTitleButtonVisible03, TestSize.Level1)
{
    sptr<WindowOption> option = sptr<WindowOption>::MakeSptr();
    option->SetWindowName("SetTitleButtonVisible");
    sptr<WindowSceneSessionImpl> window = sptr<WindowSceneSessionImpl>::MakeSptr(option);
    SessionInfo sessionInfo = { "CreateTestBundle", "CreateTestModule", "CreateTestAbility" };
    sptr<SessionMocker> session = sptr<SessionMocker>::MakeSptr(sessionInfo);
    window->hostSession_ = session;
    window->property_->SetPersistentId(1);
    window->property_->SetWindowType(WindowType::APP_MAIN_WINDOW_BASE);
    window->property_->SetWindowMode(WindowMode::WINDOW_MODE_FLOATING);
    window->uiContent_ = std::make_unique<Ace::UIContentMocker>();
    window->windowSystemConfig_.freeMultiWindowSupport_ = true;
    window->windowSystemConfig_.isSystemDecorEnable_ = false;
    WMError res = window->SetTitleButtonVisible(false, false, false, true);
    ASSERT_EQ(res, WMError::WM_ERROR_INVALID_WINDOW);
    window->windowSystemConfig_.isSystemDecorEnable_ = true;
    res = window->SetTitleButtonVisible(false, false, false, true);
    ASSERT_EQ(res, WMError::WM_OK);
}

/**
 * @tc.name: IsWindowRectAutoSave
 * @tc.desc: IsWindowRectAutoSave
 * @tc.type: FUNC
 */
HWTEST_F(WindowSceneSessionImplTest2, IsWindowRectAutoSave, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "WindowSceneSessionImplTest2: IsWindowRectAutoSave start";
    sptr<WindowOption> option = sptr<WindowOption>::MakeSptr();
    option->SetWindowName("IsWindowRectAutoSave");
    sptr<WindowSceneSessionImpl> windowSceneSessionImpl = sptr<WindowSceneSessionImpl>::MakeSptr(option);
    bool enabled = false;
    auto ret = windowSceneSessionImpl->IsWindowRectAutoSave(enabled);
    EXPECT_EQ(WMError::WM_ERROR_INVALID_WINDOW, ret);
    enabled = true;
    ret = windowSceneSessionImpl->IsWindowRectAutoSave(enabled);
    EXPECT_EQ(WMError::WM_ERROR_INVALID_WINDOW, ret);
    SessionInfo sessionInfo = { "CreateTestBundle", "CreateTestModule", "CreateTestAbility" };
    sptr<SessionMocker> session = sptr<SessionMocker>::MakeSptr(sessionInfo);
    windowSceneSessionImpl->property_->SetPersistentId(1);
    windowSceneSessionImpl->hostSession_ = session;
    windowSceneSessionImpl->context_ = abilityContext_;
    EXPECT_CALL(m->Mock(), IsWindowRectAutoSave(_, _, _)).WillRepeatedly(Return(WMError::WM_OK));
    enabled = false;
    ret = windowSceneSessionImpl->IsWindowRectAutoSave(enabled);
    EXPECT_EQ(WMError::WM_OK, ret);
    enabled = true;
    ret = windowSceneSessionImpl->IsWindowRectAutoSave(enabled);
    EXPECT_EQ(WMError::WM_OK, ret);
    windowSceneSessionImpl->context_ = nullptr;
    enabled = false;
    ret = windowSceneSessionImpl->IsWindowRectAutoSave(enabled);
    EXPECT_EQ(WMError::WM_ERROR_NULLPTR, ret);
    enabled = true;
    ret = windowSceneSessionImpl->IsWindowRectAutoSave(enabled);
    EXPECT_EQ(WMError::WM_ERROR_NULLPTR, ret);
    windowSceneSessionImpl->property_->SetWindowType(WindowType::APP_SUB_WINDOW_BASE);
    ret = windowSceneSessionImpl->IsWindowRectAutoSave(enabled);
    EXPECT_EQ(WMError::WM_ERROR_INVALID_CALLING, ret);
    GTEST_LOG_(INFO) << "WindowSceneSessionImplTest2: IsWindowRectAutoSave end";
}

/**
 * @tc.name: IsWindowRectAutoSave
 * @tc.desc: IsWindowRectAutoSave
 * @tc.type: FUNC
 */
HWTEST_F(WindowSceneSessionImplTest2, IsWindowRectAutoSave002, TestSize.Level1)
{
    sptr<WindowOption> option = sptr<WindowOption>::MakeSptr();
    option->SetWindowName("IsWindowRectAutoSave002");
    sptr<WindowSceneSessionImpl> windowSceneSessionImpl = sptr<WindowSceneSessionImpl>::MakeSptr(option);
    SessionInfo sessionInfo = { "CreateTestBundle", "CreateTestModule", "CreateTestAbility" };
    sptr<SessionMocker> session = sptr<SessionMocker>::MakeSptr(sessionInfo);
    windowSceneSessionImpl->property_->SetPersistentId(1);
    windowSceneSessionImpl->hostSession_ = session;
    windowSceneSessionImpl->context_ = abilityContext_;
    windowSceneSessionImpl->windowSystemConfig_.windowUIType_ = WindowUIType::PC_WINDOW;
    bool enabled = false;
    auto ret = windowSceneSessionImpl->IsWindowRectAutoSave(enabled);
    EXPECT_EQ(WMError::WM_ERROR_INVALID_CALLING, ret);

    windowSceneSessionImpl->property_->SetWindowType(WindowType::APP_MAIN_WINDOW_END);
    ret = windowSceneSessionImpl->IsWindowRectAutoSave(enabled);
    EXPECT_EQ(WMError::WM_ERROR_INVALID_CALLING, ret);

    option->SetWindowName("IsWindowRectAutoSave002");
    windowSceneSessionImpl->state_ = WindowState::STATE_INITIAL;
    windowSceneSessionImpl->property_->SetPersistentId(1);
    ret = windowSceneSessionImpl->IsWindowRectAutoSave(enabled);
    EXPECT_EQ(WMError::WM_ERROR_INVALID_CALLING, ret);
}

/**
 * @tc.name: SetLayoutFullScreen01
 * @tc.desc: SetLayoutFullScreen test
 * @tc.type: FUNC
 */
HWTEST_F(WindowSceneSessionImplTest2, SetLayoutFullScreen01, TestSize.Level0)
{
    sptr<WindowOption> option = sptr<WindowOption>::MakeSptr();
    sptr<WindowSceneSessionImpl> window = sptr<WindowSceneSessionImpl>::MakeSptr(option);
    window->property_->SetWindowName("SetLayoutFullScreen01");
    window->property_->SetWindowType(WindowType::BELOW_APP_SYSTEM_WINDOW_BASE);
    ASSERT_EQ(WMError::WM_ERROR_INVALID_WINDOW, window->SetLayoutFullScreen(false));
    window->property_->SetWindowType(WindowType::WINDOW_TYPE_APP_MAIN_WINDOW);
    window->property_->SetWindowMode(WindowMode::WINDOW_MODE_UNDEFINED);
    ASSERT_EQ(WMError::WM_ERROR_INVALID_WINDOW, window->SetLayoutFullScreen(false));
    window->property_->SetWindowMode(WindowMode::WINDOW_MODE_FULLSCREEN);
    ASSERT_EQ(WMError::WM_ERROR_INVALID_WINDOW, window->SetLayoutFullScreen(false));
    window->property_->SetPersistentId(1);
    SessionInfo sessionInfo = { "CreateTestBundle", "CreateTestModule", "CreateTestAbility" };
    sptr<SessionMocker> session = sptr<SessionMocker>::MakeSptr(sessionInfo);
    window->hostSession_ = session;
    WMError res = window->SetLayoutFullScreen(false);
    ASSERT_EQ(WMError::WM_OK, res);
    ASSERT_EQ(false, window->IsLayoutFullScreen());
    ASSERT_EQ(WMError::WM_OK, window->SetLayoutFullScreen(true));
    ASSERT_EQ(true, window->IsLayoutFullScreen());
    window->property_->SetWindowType(WindowType::WINDOW_TYPE_APP_MAIN_WINDOW);
    window->property_->SetWindowMode(WindowMode::WINDOW_MODE_FULLSCREEN);
    sptr<CompatibleModeProperty> compatibleModeProperty = sptr<CompatibleModeProperty>::MakeSptr();
    ASSERT_NE(compatibleModeProperty, nullptr);
    compatibleModeProperty->SetDisableFullScreen(false);
    window->property_->SetCompatibleModeProperty(compatibleModeProperty);
    window->SetLayoutFullScreen(true);
    compatibleModeProperty->SetDisableFullScreen(true);
    window->property_->SetCompatibleModeProperty(compatibleModeProperty);
    ASSERT_EQ(WMError::WM_OK, window->SetLayoutFullScreen(true));
    window->property_->SetWindowModeSupportType(0);
    ASSERT_EQ(WMError::WM_ERROR_INVALID_WINDOW, window->SetLayoutFullScreen(false));
}

/**
 * @tc.name: SetFullScreen
 * @tc.desc: SetFullScreen test
 * @tc.type: FUNC
 */
HWTEST_F(WindowSceneSessionImplTest2, SetFullScreen, TestSize.Level0)
{
    sptr<WindowOption> option = sptr<WindowOption>::MakeSptr();
    sptr<WindowSceneSessionImpl> window = sptr<WindowSceneSessionImpl>::MakeSptr(option);
    window->property_->SetWindowName("SetLayoutFullScreen01");
    window->property_->SetWindowType(WindowType::BELOW_APP_SYSTEM_WINDOW_BASE);
    ASSERT_EQ(WMError::WM_ERROR_INVALID_WINDOW, window->SetFullScreen(false));

    window->property_->SetWindowType(WindowType::WINDOW_TYPE_APP_MAIN_WINDOW);
    window->windowSystemConfig_.windowUIType_ = WindowUIType::PC_WINDOW;
    window->property_->SetWindowModeSupportType(0);
    ASSERT_EQ(WMError::WM_ERROR_INVALID_WINDOW, window->SetFullScreen(false));

    window->property_->SetWindowModeSupportType(1);
    window->property_->SetWindowMode(WindowMode::WINDOW_MODE_FULLSCREEN);
    ASSERT_EQ(WMError::WM_ERROR_INVALID_WINDOW, window->SetFullScreen(false));

    window->property_->SetPersistentId(1);
    window->windowSystemConfig_.windowUIType_ = WindowUIType::PHONE_WINDOW;
    ASSERT_EQ(WMError::WM_ERROR_INVALID_WINDOW, window->SetFullScreen(false));
    SessionInfo sessionInfo = { "CreateTestBundle", "CreateTestModule", "CreateTestAbility" };
    sptr<SessionMocker> session = sptr<SessionMocker>::MakeSptr(sessionInfo);
    window->state_ = WindowState::STATE_SHOWN;
    window->hostSession_ = session;
    ASSERT_EQ(WMError::WM_OK, window->SetFullScreen(false));
    ASSERT_EQ(false, window->IsLayoutFullScreen());
    ASSERT_EQ(WMError::WM_OK, window->SetFullScreen(true));
    ASSERT_EQ(true, window->IsLayoutFullScreen());

    window->property_->SetWindowType(WindowType::WINDOW_TYPE_APP_MAIN_WINDOW);
    window->windowSystemConfig_.windowUIType_ = WindowUIType::PC_WINDOW;
    sptr<CompatibleModeProperty> compatibleModeProperty = sptr<CompatibleModeProperty>::MakeSptr();
    ASSERT_NE(compatibleModeProperty, nullptr);
    compatibleModeProperty->SetDisableFullScreen(false);
    window->property_->SetCompatibleModeProperty(compatibleModeProperty);
    window->SetFullScreen(true);
    compatibleModeProperty->SetDisableFullScreen(true);
    window->property_->SetCompatibleModeProperty(compatibleModeProperty);
    ASSERT_EQ(WMError::WM_OK, window->SetFullScreen(true));
}
}
} // namespace
} // namespace Rosen
} // namespace OHOS