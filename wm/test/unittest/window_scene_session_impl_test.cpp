/*
 * Copyright (c) 2023 Huawei Device Co., Ltd.
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
#include "scene_board_judgement.h"
#include "session/host/include/scene_session.h"
#include "singleton_mocker.h"
#include "window_scene_session_impl.h"
#include "window_session_impl.h"

using namespace testing;
using namespace testing::ext;

namespace OHOS {
namespace Rosen {
namespace {
    std::string g_logMsg;
    void LogCallback(const LogType type, const LogLevel level, const unsigned int domain, const char* tag,
        const char* msg)
    {
        g_logMsg += msg;
    }
}
using Mocker = SingletonMocker<WindowAdapter, MockWindowAdapter>;

class MockWindowChangeListener : public IWindowChangeListener {
public:
    MOCK_METHOD3(OnSizeChange,
                 void(Rect rect, WindowSizeChangeReason reason, const std::shared_ptr<RSTransaction>& rsTransaction));
};

class MockWindowLifeCycleListener : public IWindowLifeCycle {
public:
    MOCK_METHOD0(AfterForeground, void(void));
    MOCK_METHOD0(AfterBackground, void(void));
    MOCK_METHOD0(AfterFocused, void(void));
    MOCK_METHOD0(AfterUnfocused, void(void));
    MOCK_METHOD1(ForegroundFailed, void(int32_t));
    MOCK_METHOD0(AfterActive, void(void));
    MOCK_METHOD0(AfterInactive, void(void));
    MOCK_METHOD0(AfterResumed, void(void));
    MOCK_METHOD0(AfterPaused, void(void));
};

class WindowSceneSessionImplTest : public testing::Test {
public:
    static void SetUpTestCase();
    static void TearDownTestCase();
    void SetUp() override;
    void TearDown() override;

    std::shared_ptr<AbilityRuntime::AbilityContext> abilityContext_;
    std::unique_ptr<Mocker> m = std::make_unique<Mocker>();

private:
    RSSurfaceNode::SharedPtr CreateRSSurfaceNode();
    static constexpr uint32_t WAIT_SYNC_IN_NS = 200000;
};

void WindowSceneSessionImplTest::SetUpTestCase() {}

void WindowSceneSessionImplTest::TearDownTestCase() {}

void WindowSceneSessionImplTest::SetUp()
{
    abilityContext_ = std::make_shared<AbilityRuntime::AbilityContextImpl>();
}

void WindowSceneSessionImplTest::TearDown()
{
    usleep(WAIT_SYNC_IN_NS);
    abilityContext_ = nullptr;
}

RSSurfaceNode::SharedPtr WindowSceneSessionImplTest::CreateRSSurfaceNode()
{
    struct RSSurfaceNodeConfig rsSurfaceNodeConfig;
    rsSurfaceNodeConfig.SurfaceNodeName = "startingWindowTestSurfaceNode";
    auto surfaceNode = RSSurfaceNode::Create(rsSurfaceNodeConfig, RSSurfaceNodeType::DEFAULT);
    return surfaceNode;
}

static sptr<WindowSceneSessionImpl> CreateWindow(std::string windowName, WindowType type, int32_t id)
{
    sptr<WindowOption> option = sptr<WindowOption>::MakeSptr();
    option->SetWindowName(windowName);
    option->SetWindowType(type);
    auto window = sptr<WindowSceneSessionImpl>::MakeSptr(option);
    window->property_->SetPersistentId(id);
    return window;
}

namespace {
/**
 * @tc.name: CreateWindowAndDestroy01
 * @tc.desc: Create window and destroy window
 * @tc.type: FUNC
 */
HWTEST_F(WindowSceneSessionImplTest, CreateWindowAndDestroy01, TestSize.Level0)
{
    sptr<WindowOption> option = sptr<WindowOption>::MakeSptr();
    option->SetWindowName("CreateWindowAndDestroy01");
    sptr<WindowSceneSessionImpl> window = sptr<WindowSceneSessionImpl>::MakeSptr(option);
    SessionInfo sessionInfo = { "CreateTestBundle", "CreateTestModule", "CreateTestAbility" };
    sptr<SessionMocker> session = sptr<SessionMocker>::MakeSptr(sessionInfo);

    ASSERT_EQ(WMError::WM_OK, window->Create(abilityContext_, session));
    ASSERT_EQ(WMError::WM_ERROR_INVALID_WINDOW, window->Destroy(false));
    ASSERT_EQ(WMError::WM_ERROR_REPEAT_OPERATION, window->Create(abilityContext_, session));
    window->property_->SetPersistentId(1);
    ASSERT_EQ(WMError::WM_OK, window->Destroy(false));
    ASSERT_EQ(WMError::WM_ERROR_NULLPTR, window->Create(abilityContext_, session));
    ASSERT_EQ(WMError::WM_ERROR_INVALID_WINDOW, window->Destroy(true));
}

/**
 * @tc.name: CreateWindowAndDestroy02
 * @tc.desc: Create window and destroy window
 * @tc.type: FUNC
 */
HWTEST_F(WindowSceneSessionImplTest, CreateWindowAndDestroy02, TestSize.Level0)
{
    sptr<WindowOption> option = sptr<WindowOption>::MakeSptr();
    option->SetWindowName("CreateWindowAndDestroy02");
    sptr<WindowSceneSessionImpl> window = sptr<WindowSceneSessionImpl>::MakeSptr(option);
    SessionInfo sessionInfo = { "CreateTestBundle", "CreateTestModule", "CreateTestAbility" };
    sptr<SessionMocker> session = sptr<SessionMocker>::MakeSptr(sessionInfo);
    std::string identityToken = "testToken";
    window->Create(abilityContext_, session);
    ASSERT_EQ(WMError::WM_ERROR_REPEAT_OPERATION, window->Create(abilityContext_, session, identityToken));
    window->property_->SetPersistentId(1);
    window->Destroy(false);
    ASSERT_EQ(WMError::WM_ERROR_NULLPTR, window->Create(abilityContext_, session, identityToken));
    ASSERT_EQ(WMError::WM_ERROR_INVALID_WINDOW, window->Destroy(false));
}

/**
 * @tc.name: UpdateAnimationSpeedIfEnabled
 * @tc.desc: UpdateAnimationSpeedIfEnabled
 * @tc.type: FUNC
 */
HWTEST_F(WindowSceneSessionImplTest, UpdateAnimationSpeedIfEnabled, TestSize.Level0)
{
    g_logMsg.clear();
    LOG_SetCallback(LogCallback);
    sptr<WindowOption> option = sptr<WindowOption>::MakeSptr();
    option->SetWindowName("UpdateAnimationSpeedIfEnabled");
    sptr<WindowSceneSessionImpl> window = sptr<WindowSceneSessionImpl>::MakeSptr(option);

    window->UpdateAnimationSpeedIfEnabled();
    EXPECT_TRUE(g_logMsg.find("isEnableAnimationSpeed_ is true") == std::string::npos);
}

/**
 * @tc.name: SetPcAppInpadSpecificSystemBarInvisible
 * @tc.desc: SetPcAppInpadSpecificSystemBarInvisible
 * @tc.type: FUNC
 */
HWTEST_F(WindowSceneSessionImplTest, SetPcAppInpadSpecificSystemBarInvisible, TestSize.Level0)
{
    sptr<WindowOption> option = sptr<WindowOption>::MakeSptr();
    option->SetWindowName("SetPcAppInpadSpecificSystemBarInvisible");
    sptr<WindowSceneSessionImpl> window = sptr<WindowSceneSessionImpl>::MakeSptr(option);
    SessionInfo sessionInfo = { "CreateTestBundle", "CreateTestModule", "CreateTestAbility" };
    sptr<SessionMocker> session = sptr<SessionMocker>::MakeSptr(sessionInfo);
    EXPECT_EQ(WMError::WM_ERROR_INVALID_CALLING, window->SetPcAppInpadSpecificSystemBarInvisible());
    window->windowSystemConfig_.windowUIType_ = WindowUIType::PAD_WINDOW;
    window->property_->SetIsPcAppInPad(true);
    window->property_->SetWindowType(WindowType::APP_MAIN_WINDOW_BASE);
    window->windowSystemConfig_.freeMultiWindowEnable_ = false;
    window->property_->SetPcAppInpadSpecificSystemBarInvisible(true);
    window->property_->SetPcAppInpadCompatibleMode(true);
    EXPECT_EQ(WMError::WM_OK, window->SetPcAppInpadSpecificSystemBarInvisible());
    ASSERT_EQ(WMError::WM_ERROR_INVALID_WINDOW, window->Destroy(false));
}

/**
 * @tc.name: SetPcAppInpadOrientationLandscape
 * @tc.desc: SetPcAppInpadOrientationLandscape
 * @tc.type: FUNC
 */
HWTEST_F(WindowSceneSessionImplTest, SetPcAppInpadOrientationLandscape, TestSize.Level0)
{
    sptr<WindowOption> option = sptr<WindowOption>::MakeSptr();
    option->SetWindowName("SetPcAppInpadOrientationLandscape");
    sptr<WindowSceneSessionImpl> window = sptr<WindowSceneSessionImpl>::MakeSptr(option);
    SessionInfo sessionInfo = { "CreateTestBundle", "CreateTestModule", "CreateTestAbility" };
    sptr<SessionMocker> session = sptr<SessionMocker>::MakeSptr(sessionInfo);
    EXPECT_EQ(WMError::WM_ERROR_INVALID_CALLING, window->SetPcAppInpadOrientationLandscape());
    window->windowSystemConfig_.windowUIType_ = WindowUIType::PAD_WINDOW;
    window->property_->SetIsPcAppInPad(true);
    window->property_->SetWindowType(WindowType::APP_MAIN_WINDOW_BASE);
    window->windowSystemConfig_.freeMultiWindowEnable_ = false;
    window->property_->SetPcAppInpadOrientationLandscape(true);
    window->property_->SetPcAppInpadCompatibleMode(true);
    EXPECT_EQ(WMError::WM_OK, window->SetPcAppInpadOrientationLandscape());
    ASSERT_EQ(WMError::WM_ERROR_INVALID_WINDOW, window->Destroy(false));
}

/**
 * @tc.name: CreateAndConnectSpecificSession01
 * @tc.desc: CreateAndConnectSpecificSession
 * @tc.type: FUNC
 */
HWTEST_F(WindowSceneSessionImplTest, CreateAndConnectSpecificSession01, TestSize.Level0)
{
    if (!SceneBoardJudgement::IsSceneBoardEnabled()) {
        GTEST_SKIP() << "SceneBoard is not enabled, skipping test.";
    }
    sptr<WindowOption> option = sptr<WindowOption>::MakeSptr();
    option->SetWindowName("CreateAndConnectSpecificSession01");
    sptr<WindowSceneSessionImpl> windowSceneSession = sptr<WindowSceneSessionImpl>::MakeSptr(option);

    windowSceneSession->SetWindowType(WindowType::APP_MAIN_WINDOW_BASE);
    ASSERT_EQ(WMError::WM_ERROR_NULLPTR, windowSceneSession->CreateAndConnectSpecificSession());
    windowSceneSession->property_->SetPersistentId(102);
    windowSceneSession->property_->SetParentPersistentId(100);
    windowSceneSession->property_->SetParentId(100);
    SessionInfo sessionInfo = { "CreateTestBundle", "CreateTestModule", "CreateTestAbility" };
    sptr<SessionMocker> session = sptr<SessionMocker>::MakeSptr(sessionInfo);

    ASSERT_EQ(WMError::WM_OK, windowSceneSession->Create(abilityContext_, session));
    ASSERT_EQ(WMError::WM_OK, windowSceneSession->Destroy(true));
}

/**
 * @tc.name: CreateAndConnectSpecificSession02
 * @tc.desc: CreateAndConnectSpecificSession
 * @tc.type: FUNC
 */
HWTEST_F(WindowSceneSessionImplTest, CreateAndConnectSpecificSession02, TestSize.Level1)
{
    if (!SceneBoardJudgement::IsSceneBoardEnabled()) {
        GTEST_SKIP() << "SceneBoard is not enabled, skipping test.";
    }
    sptr<WindowOption> option = sptr<WindowOption>::MakeSptr();
    option->SetWindowTag(WindowTag::SUB_WINDOW);
    option->SetWindowName("CreateAndConnectSpecificSession02");
    sptr<WindowSceneSessionImpl> windowSceneSession = sptr<WindowSceneSessionImpl>::MakeSptr(option);

    SessionInfo sessionInfo = { "CreateTestBundle0", "CreateTestModule0", "CreateTestAbility0" };
    sptr<SessionMocker> session = sptr<SessionMocker>::MakeSptr(sessionInfo);
    ASSERT_EQ(WMError::WM_OK, windowSceneSession->Create(abilityContext_, session));
    windowSceneSession->property_->SetPersistentId(103);
    windowSceneSession->property_->SetParentPersistentId(102);
    windowSceneSession->property_->SetParentId(102);
    windowSceneSession->hostSession_ = session;

    windowSceneSession->property_->type_ = WindowType::APP_SUB_WINDOW_BASE;
    ASSERT_EQ(WMError::WM_ERROR_NULLPTR, windowSceneSession->CreateAndConnectSpecificSession());
    ASSERT_EQ(WMError::WM_OK, windowSceneSession->Destroy(true));
}

/**
 * @tc.name: CreateAndConnectSpecificSession03
 * @tc.desc: CreateAndConnectSpecificSession
 * @tc.type: FUNC
 */
HWTEST_F(WindowSceneSessionImplTest, CreateAndConnectSpecificSession03, TestSize.Level1)
{
    if (!SceneBoardJudgement::IsSceneBoardEnabled()) {
        GTEST_SKIP() << "SceneBoard is not enabled, skipping test.";
    }
    sptr<WindowOption> option = sptr<WindowOption>::MakeSptr();
    option->SetWindowTag(WindowTag::SUB_WINDOW);
    option->SetWindowName("CreateAndConnectSpecificSession03");
    option->SetIsUIExtFirstSubWindow(true);
    sptr<WindowSceneSessionImpl> windowSceneSession = sptr<WindowSceneSessionImpl>::MakeSptr(option);

    SessionInfo sessionInfo = { "CreateTestBundle0", "CreateTestModule0", "CreateTestAbility0" };
    sptr<SessionMocker> session = sptr<SessionMocker>::MakeSptr(sessionInfo);
    ASSERT_EQ(WMError::WM_OK, windowSceneSession->Create(abilityContext_, session));

    windowSceneSession->property_->SetParentPersistentId(102);
    windowSceneSession->property_->SetParentId(102);
    windowSceneSession->hostSession_ = session;

    windowSceneSession->property_->type_ = WindowType::APP_SUB_WINDOW_BASE;
    ASSERT_EQ(WMError::WM_OK, windowSceneSession->CreateAndConnectSpecificSession());
    ASSERT_EQ(WMError::WM_OK, windowSceneSession->Destroy(true));
}

/**
 * @tc.name: CreateAndConnectSpecificSession04
 * @tc.desc: CreateAndConnectSpecificSession
 * @tc.type: FUNC
 */
HWTEST_F(WindowSceneSessionImplTest, CreateAndConnectSpecificSession04, TestSize.Level1)
{
    if (!SceneBoardJudgement::IsSceneBoardEnabled()) {
        GTEST_SKIP() << "SceneBoard is not enabled, skipping test.";
    }
    sptr<WindowOption> option = sptr<WindowOption>::MakeSptr();
    option->SetWindowTag(WindowTag::SUB_WINDOW);
    option->SetWindowName("CreateAndConnectSpecificSession04");
    sptr<WindowSceneSessionImpl> windowSceneSession = sptr<WindowSceneSessionImpl>::MakeSptr(option);

    SessionInfo sessionInfo = { "CreateTestBundle4", "CreateTestModule4", "CreateTestAbility4" };
    sptr<SessionMocker> session = sptr<SessionMocker>::MakeSptr(sessionInfo);
    ASSERT_EQ(WMError::WM_OK, windowSceneSession->Create(abilityContext_, session));

    windowSceneSession->property_->SetPersistentId(104);
    windowSceneSession->property_->SetParentPersistentId(103);
    windowSceneSession->property_->SetParentId(103);
    windowSceneSession->property_->type_ = WindowType::APP_MAIN_WINDOW_BASE;
    windowSceneSession->hostSession_ = session;

    windowSceneSession->property_->type_ = WindowType::APP_SUB_WINDOW_BASE;
    ASSERT_EQ(WMError::WM_ERROR_NULLPTR, windowSceneSession->CreateAndConnectSpecificSession());
    ASSERT_EQ(WMError::WM_OK, windowSceneSession->Destroy(true));
}

/**
 * @tc.name: CreateAndConnectSpecificSession05
 * @tc.desc: CreateAndConnectSpecificSession
 * @tc.type: FUNC
 */
HWTEST_F(WindowSceneSessionImplTest, CreateAndConnectSpecificSession05, TestSize.Level1)
{
    if (!SceneBoardJudgement::IsSceneBoardEnabled()) {
        GTEST_SKIP() << "SceneBoard is not enabled, skipping test.";
    }
    sptr<WindowOption> option = sptr<WindowOption>::MakeSptr();
    option->SetWindowTag(WindowTag::SUB_WINDOW);
    option->SetWindowName("CreateAndConnectSpecificSession05");
    sptr<WindowSceneSessionImpl> windowSceneSession = sptr<WindowSceneSessionImpl>::MakeSptr(option);

    SessionInfo sessionInfo = { "CreateTestBundle5", "CreateTestModule5", "CreateTestAbility5" };
    sptr<SessionMocker> session = sptr<SessionMocker>::MakeSptr(sessionInfo);
    ASSERT_EQ(WMError::WM_OK, windowSceneSession->Create(abilityContext_, session));

    windowSceneSession->property_->SetParentPersistentId(104);
    windowSceneSession->property_->SetParentId(104);
    windowSceneSession->hostSession_ = session;

    windowSceneSession->property_->type_ = WindowType::APP_SUB_WINDOW_BASE;
    ASSERT_NE(WMError::WM_ERROR_INVALID_TYPE, windowSceneSession->CreateAndConnectSpecificSession());
    ASSERT_NE(WMError::WM_OK, windowSceneSession->Destroy(true));
}

/**
 * @tc.name: CreateAndConnectSpecificSession06
 * @tc.desc: CreateAndConnectSpecificSession
 * @tc.type: FUNC
 */
HWTEST_F(WindowSceneSessionImplTest, CreateAndConnectSpecificSession06, TestSize.Level1)
{
    if (!SceneBoardJudgement::IsSceneBoardEnabled()) {
        GTEST_SKIP() << "SceneBoard is not enabled, skipping test.";
    }
    sptr<WindowOption> option = sptr<WindowOption>::MakeSptr();
    option->SetWindowTag(WindowTag::SYSTEM_WINDOW);
    option->SetWindowName("CreateAndConnectSpecificSession06");
    sptr<WindowSceneSessionImpl> windowSceneSession = sptr<WindowSceneSessionImpl>::MakeSptr(option);

    SessionInfo sessionInfo = { "CreateTestBundle6", "CreateTestModule6", "CreateTestAbility6" };
    sptr<SessionMocker> session = sptr<SessionMocker>::MakeSptr(sessionInfo);
    ASSERT_EQ(WMError::WM_OK, windowSceneSession->Create(abilityContext_, session));

    windowSceneSession->property_->SetPersistentId(105);
    windowSceneSession->property_->SetParentPersistentId(102);
    windowSceneSession->property_->SetParentId(102);
    windowSceneSession->hostSession_ = session;

    windowSceneSession->property_->type_ = WindowType::SYSTEM_WINDOW_BASE;
    ASSERT_EQ(WMError::WM_OK, windowSceneSession->CreateAndConnectSpecificSession());
    ASSERT_EQ(WMError::WM_OK, windowSceneSession->Destroy(true));
}

/**
 * @tc.name: CreateAndConnectSpecificSession07
 * @tc.desc: CreateAndConnectSpecificSession
 * @tc.type: FUNC
 */
HWTEST_F(WindowSceneSessionImplTest, CreateAndConnectSpecificSession07, TestSize.Level1)
{
    if (!SceneBoardJudgement::IsSceneBoardEnabled()) {
        GTEST_SKIP() << "SceneBoard is not enabled, skipping test.";
    }
    sptr<WindowOption> option = sptr<WindowOption>::MakeSptr();
    option->SetWindowTag(WindowTag::SYSTEM_WINDOW);
    option->SetWindowName("CreateAndConnectSpecificSession07");
    sptr<WindowSceneSessionImpl> windowSceneSession = sptr<WindowSceneSessionImpl>::MakeSptr(option);

    SessionInfo sessionInfo = { "CreateTestBundle7", "CreateTestModule7", "CreateTestAbility7" };
    sptr<SessionMocker> session = sptr<SessionMocker>::MakeSptr(sessionInfo);
    ASSERT_EQ(WMError::WM_OK, windowSceneSession->Create(abilityContext_, session));

    windowSceneSession->property_->SetPersistentId(106);
    windowSceneSession->property_->SetParentPersistentId(105);
    windowSceneSession->property_->SetParentId(105);
    windowSceneSession->hostSession_ = session;

    windowSceneSession->property_->type_ = WindowType::SYSTEM_SUB_WINDOW_BASE;
    ASSERT_EQ(WMError::WM_ERROR_NULLPTR, windowSceneSession->CreateAndConnectSpecificSession());
    ASSERT_EQ(WMError::WM_OK, windowSceneSession->Destroy(true));
}

/**
 * @tc.name: CreateAndConnectSpecificSession08
 * @tc.desc: CreateAndConnectSpecificSession
 * @tc.type: FUNC
 */
HWTEST_F(WindowSceneSessionImplTest, CreateAndConnectSpecificSession08, TestSize.Level0)
{
    constexpr int parentId = 10000;
    sptr<WindowOption> option = sptr<WindowOption>::MakeSptr();
    option->SetWindowType(WindowType::WINDOW_TYPE_APP_SUB_WINDOW);
    option->SetWindowName("CreateAndConnectSpecificSession08");
    option->SetParentId(parentId);

    // scene session manager processing
    option->SetIsUIExtFirstSubWindow(true);
    sptr<WindowSceneSessionImpl> windowSceneSession = sptr<WindowSceneSessionImpl>::MakeSptr(option);
    windowSceneSession->Create(abilityContext_, nullptr);
    ASSERT_EQ(windowSceneSession->property_->GetParentPersistentId(), option->GetParentId());
    // scene session manager processing
    option->SetIsUIExtFirstSubWindow(false);
    option->SetIsUIExtAnySubWindow(true);
    option->AddWindowFlag(WindowFlag::WINDOW_FLAG_IS_TOAST);
    windowSceneSession = sptr<WindowSceneSessionImpl>::MakeSptr(option);
    ASSERT_NE(nullptr, windowSceneSession);
    windowSceneSession->Create(abilityContext_, nullptr);
    ASSERT_EQ(windowSceneSession->property_->GetParentPersistentId(), option->GetParentId());

    sptr<WindowSceneSessionImpl> mainWindow = sptr<WindowSceneSessionImpl>::MakeSptr(option);
    windowSceneSession->property_->SetPersistentId(parentId);
    // window processing
    option->SetIsUIExtFirstSubWindow(false);
    option->SetIsUIExtAnySubWindow(true);
    option->RemoveWindowFlag(WindowFlag::WINDOW_FLAG_IS_TOAST);
    windowSceneSession = sptr<WindowSceneSessionImpl>::MakeSptr(option);
    windowSceneSession->windowSessionMap_["mainWindow"] = { parentId, mainWindow };
    windowSceneSession->Create(abilityContext_, nullptr);
    ASSERT_EQ(windowSceneSession->property_->GetDisplayId(), mainWindow->property_->GetDisplayId());
    // window processing
    option->SetIsUIExtAnySubWindow(false);
    windowSceneSession = sptr<WindowSceneSessionImpl>::MakeSptr(option);
    windowSceneSession->windowSessionMap_["mainWindow"] = { parentId, mainWindow };
    windowSceneSession->Create(abilityContext_, nullptr);
    ASSERT_EQ(windowSceneSession->property_->GetDisplayId(), mainWindow->property_->GetDisplayId());
}

/**
 * @tc.name: CreateAndConnectSpecificSession09
 * @tc.desc: CreateAndConnectSpecificSession
 * @tc.type: FUNC
 */
HWTEST_F(WindowSceneSessionImplTest, CreateAndConnectSpecificSession09, TestSize.Level0)
{
    if (!SceneBoardJudgement::IsSceneBoardEnabled()) {
        GTEST_SKIP() << "SceneBoard is not enabled, skipping test.";
    }
    sptr<WindowOption> option = sptr<WindowOption>::MakeSptr();
    option->SetWindowName("CreateAndConnectSpecificSession09");
    sptr<WindowSceneSessionImpl> windowSceneSession = sptr<WindowSceneSessionImpl>::MakeSptr(option);

    windowSceneSession->SetWindowType(WindowType::WINDOW_TYPE_WALLET_SWIPE_CARD);
    ASSERT_EQ(WMError::WM_ERROR_NULLPTR, windowSceneSession->CreateAndConnectSpecificSession());
    windowSceneSession->property_->SetPersistentId(102);
    windowSceneSession->property_->SetParentPersistentId(100);
    windowSceneSession->property_->SetParentId(100);
    SessionInfo sessionInfo = { "CreateTestBundle", "CreateTestModule", "CreateTestAbility" };
    sptr<SessionMocker> session = sptr<SessionMocker>::MakeSptr(sessionInfo);

    ASSERT_EQ(WMError::WM_OK, windowSceneSession->Create(abilityContext_, session));
    ASSERT_EQ(WMError::WM_OK, windowSceneSession->Destroy(true));
}

/**
 * @tc.name: CreateAndConnectSpecificSession10
 * @tc.desc: CreateAndConnectSpecificSession
 * @tc.type: FUNC
 */
HWTEST_F(WindowSceneSessionImplTest, CreateAndConnectSpecificSession10, TestSize.Level1)
{
    constexpr int parentId = 1000;
    constexpr int displayId = 100;
    sptr<WindowOption> parentOption = sptr<WindowOption>::MakeSptr();
    parentOption->SetWindowTag(WindowTag::MAIN_WINDOW);
    parentOption->SetWindowName("MainWindow");
    parentOption->SetWindowType(WindowType::WINDOW_TYPE_APP_MAIN_WINDOW);
    parentOption->SetDisplayId(displayId);
    sptr<WindowSceneSessionImpl> parentWindow = sptr<WindowSceneSessionImpl>::MakeSptr(parentOption);
    ASSERT_NE(nullptr, parentWindow);

    parentWindow->property_->SetPersistentId(parentId);
    SessionInfo sessionInfo = {"TextMenuTestBundle", "TextMenuTestModule", "TextMenuTestAbility"};
    sptr<SessionMocker> session = sptr<SessionMocker>::MakeSptr(sessionInfo);
    WMError error = parentWindow->Create(abilityContext_, session);
    ASSERT_EQ(error, WMError::WM_OK);

    sptr<WindowOption> option = sptr<WindowOption>::MakeSptr();
    ASSERT_NE(nullptr, option);
    option->SetWindowType(WindowType::WINDOW_TYPE_APP_SUB_WINDOW);
    option->AddWindowFlag(WindowFlag::WINDOW_FLAG_IS_TEXT_MENU);
    option->SetWindowName("TextMenu");
    option->SetParentId(parentId);
    sptr<WindowSceneSessionImpl> textMenuWindow = sptr<WindowSceneSessionImpl>::MakeSptr(option);
    ASSERT_NE(nullptr, textMenuWindow);
    error = textMenuWindow->Create(abilityContext_, nullptr);
    ASSERT_EQ(error, WMError::WM_OK);
    ASSERT_EQ(WMError::WM_OK, textMenuWindow->Destroy(true));
    ASSERT_EQ(WMError::WM_OK, parentWindow->Destroy(true));
}

/**
 * @tc.name: CreateAndConnectSpecificSession11
 * @tc.desc: CreateAndConnectSpecificSession
 * @tc.type: FUNC
 */
HWTEST_F(WindowSceneSessionImplTest, CreateAndConnectSpecificSession11, TestSize.Level0)
{
    if (!SceneBoardJudgement::IsSceneBoardEnabled()) {
        GTEST_SKIP() << "SceneBoard is not enabled, skipping test.";
    }
    sptr<WindowOption> option = sptr<WindowOption>::MakeSptr();
    option->SetWindowName("CreateAndConnectSpecificSession10");
    sptr<WindowSceneSessionImpl> windowSceneSession = sptr<WindowSceneSessionImpl>::MakeSptr(option);

    windowSceneSession->SetWindowType(WindowType::WINDOW_TYPE_FLOAT_NAVIGATION);
    ASSERT_EQ(WMError::WM_ERROR_NULLPTR, windowSceneSession->CreateAndConnectSpecificSession());
    windowSceneSession->property_->SetPersistentId(102);
    windowSceneSession->property_->SetParentPersistentId(100);
    windowSceneSession->property_->SetParentId(100);
    SessionInfo sessionInfo = { "CreateTestBundle", "CreateTestModule", "CreateTestAbility" };
    sptr<SessionMocker> session = sptr<SessionMocker>::MakeSptr(sessionInfo);

    ASSERT_EQ(WMError::WM_OK, windowSceneSession->Create(abilityContext_, session));
    ASSERT_EQ(WMError::WM_OK, windowSceneSession->Destroy(true));
}

/**
 * @tc.name: CreateAndConnectSpecificSession12
 * @tc.desc: CreateAndConnectSpecificSession
 * @tc.type: FUNC
 */
HWTEST_F(WindowSceneSessionImplTest, CreateAndConnectSpecificSession12, TestSize.Level1)
{
    if (!SceneBoardJudgement::IsSceneBoardEnabled()) {
        GTEST_SKIP() << "SceneBoard is not enabled, skipping test.";
    }
    sptr<WindowOption> option = sptr<WindowOption>::MakeSptr();
    option->SetWindowTag(WindowTag::SYSTEM_WINDOW);
    option->SetWindowName("CreateAndConnectSpecificSession12");
    option->SetDisplayId(999);
    sptr<WindowSceneSessionImpl> windowSceneSession = sptr<WindowSceneSessionImpl>::MakeSptr(option);

    SessionInfo sessionInfo = { "CreateTestBundle6", "CreateTestModule612", "CreateTestAbility12" };
    sptr<SessionMocker> session = sptr<SessionMocker>::MakeSptr(sessionInfo);
    ASSERT_EQ(WMError::WM_OK, windowSceneSession->Create(abilityContext_, session));

    windowSceneSession->property_->SetPersistentId(105);
    windowSceneSession->property_->SetParentPersistentId(102);
    windowSceneSession->property_->SetParentId(102);
    windowSceneSession->hostSession_ = session;

    windowSceneSession->property_->type_ = WindowType::SYSTEM_WINDOW_BASE;
    ASSERT_EQ(WMError::WM_OK, windowSceneSession->CreateAndConnectSpecificSession());
    ASSERT_EQ(999, windowSceneSession->property_->GetDisplayId());
    ASSERT_EQ(WMError::WM_OK, windowSceneSession->Destroy(true));
}

/**
 * @tc.name: CreateAndConnectSpecificSession13
 * @tc.desc: CreateAndConnectSpecificSession
 * @tc.type: FUNC
 */
HWTEST_F(WindowSceneSessionImplTest, CreateAndConnectSpecificSession13, TestSize.Level1)
{
    if (!SceneBoardJudgement::IsSceneBoardEnabled()) {
        GTEST_SKIP() << "SceneBoard is not enabled, skipping test.";
    }
    sptr<WindowOption> option = sptr<WindowOption>::MakeSptr();
    option->SetWindowName("CreateAndConnectSpecificSession13");
    sptr<WindowSceneSessionImpl> windowSceneSession = sptr<WindowSceneSessionImpl>::MakeSptr(option);

    windowSceneSession->SetWindowType(WindowType::WINDOW_TYPE_MUTISCREEN_COLLABORATION);
    ASSERT_EQ(WMError::WM_ERROR_NULLPTR, windowSceneSession->CreateAndConnectSpecificSession());
    windowSceneSession->property_->SetPersistentId(102);
    windowSceneSession->property_->SetParentPersistentId(100);
    windowSceneSession->property_->SetParentId(100);
    SessionInfo sessionInfo = { "CreateTestBundle", "CreateTestModule", "CreateTestAbility" };
    sptr<SessionMocker> session = sptr<SessionMocker>::MakeSptr(sessionInfo);

    ASSERT_EQ(WMError::WM_OK, windowSceneSession->Create(abilityContext_, session));
    ASSERT_EQ(WMError::WM_OK, windowSceneSession->Destroy(true));
}

/**
 * @tc.name: CreateAndConnectSpecificSession14
 * @tc.desc: CreateAndConnectSpecificSession
 * @tc.type: FUNC
 */
HWTEST_F(WindowSceneSessionImplTest, CreateAndConnectSpecificSession14, TestSize.Level0)
{
    if (!SceneBoardJudgement::IsSceneBoardEnabled()) {
        GTEST_SKIP() << "SceneBoard is not enabled, skipping test.";
    }
    sptr<WindowOption> option = sptr<WindowOption>::MakeSptr();
    option->SetWindowName("CreateAndConnectSpecificSession14");
    sptr<WindowSceneSessionImpl> windowSceneSession = sptr<WindowSceneSessionImpl>::MakeSptr(option);

    windowSceneSession->SetWindowType(WindowType::WINDOW_TYPE_MAGNIFICATION);
    EXPECT_EQ(WMError::WM_ERROR_NULLPTR, windowSceneSession->CreateAndConnectSpecificSession());
    windowSceneSession->property_->SetPersistentId(102);
    windowSceneSession->property_->SetParentPersistentId(100);
    windowSceneSession->property_->SetParentId(100);
    windowSceneSession->property_->SetWindowMode(WindowMode::WINDOW_MODE_FLOATING);
    SessionInfo sessionInfo = { "CreateTestBundle", "CreateTestModule", "CreateTestAbility" };
    sptr<SessionMocker> session = sptr<SessionMocker>::MakeSptr(sessionInfo);

    EXPECT_EQ(WMError::WM_OK, windowSceneSession->Create(abilityContext_, session));
    EXPECT_EQ(WMError::WM_OK, windowSceneSession->Show());
    EXPECT_EQ(WMError::WM_OK, windowSceneSession->Destroy(true));
}

/**
 * @tc.name: CreateAndConnectSpecificSession15
 * @tc.desc: CreateAndConnectSpecificSession
 * @tc.type: FUNC
 */
HWTEST_F(WindowSceneSessionImplTest, CreateAndConnectSpecificSession15, TestSize.Level0)
{
    if (!SceneBoardJudgement::IsSceneBoardEnabled()) {
        GTEST_SKIP() << "SceneBoard is not enabled, skipping test.";
    }
    sptr<WindowOption> option = sptr<WindowOption>::MakeSptr();
    option->SetWindowName("CreateAndConnectSpecificSession15");
    sptr<WindowSceneSessionImpl> windowSceneSession = sptr<WindowSceneSessionImpl>::MakeSptr(option);

    windowSceneSession->SetWindowType(WindowType::WINDOW_TYPE_MAGNIFICATION_MENU);
    EXPECT_EQ(WMError::WM_ERROR_NULLPTR, windowSceneSession->CreateAndConnectSpecificSession());
    windowSceneSession->property_->SetPersistentId(102);
    windowSceneSession->property_->SetParentPersistentId(100);
    windowSceneSession->property_->SetParentId(100);
    windowSceneSession->property_->SetWindowMode(WindowMode::WINDOW_MODE_FLOATING);
    SessionInfo sessionInfo = { "CreateTestBundle", "CreateTestModule", "CreateTestAbility" };
    sptr<SessionMocker> session = sptr<SessionMocker>::MakeSptr(sessionInfo);

    EXPECT_EQ(WMError::WM_OK, windowSceneSession->Create(abilityContext_, session));
    EXPECT_EQ(WMError::WM_OK, windowSceneSession->Show());
    EXPECT_EQ(WMError::WM_OK, windowSceneSession->Destroy(true));
}

/**
 * @tc.name: CreateAndConnectSpecificSession16
 * @tc.desc: CreateAndConnectSpecificSession
 * @tc.type: FUNC
 */
HWTEST_F(WindowSceneSessionImplTest, CreateAndConnectSpecificSession16, TestSize.Level0)
{
    if (!SceneBoardJudgement::IsSceneBoardEnabled()) {
        GTEST_SKIP() << "SceneBoard is not enabled, skipping test.";
    }
    sptr<WindowOption> option = sptr<WindowOption>::MakeSptr();
    option->SetWindowName("CreateAndConnectSpecificSession16");
    sptr<WindowSceneSessionImpl> windowSceneSession = sptr<WindowSceneSessionImpl>::MakeSptr(option);

    windowSceneSession->SetWindowType(WindowType::WINDOW_TYPE_SELECTION);
    EXPECT_EQ(WMError::WM_ERROR_NULLPTR, windowSceneSession->CreateAndConnectSpecificSession());
    windowSceneSession->property_->SetPersistentId(102); // 102 is persistentId
    windowSceneSession->property_->SetParentPersistentId(100); // 100 is parentPersistentId
    windowSceneSession->property_->SetParentId(100); // 100 is parentId
    windowSceneSession->property_->SetWindowMode(WindowMode::WINDOW_MODE_FLOATING);
    SessionInfo sessionInfo = { "CreateTestBundle", "CreateTestModule", "CreateTestAbility" };
    sptr<SessionMocker> session = sptr<SessionMocker>::MakeSptr(sessionInfo);

    EXPECT_EQ(WMError::WM_OK, windowSceneSession->Create(abilityContext_, session));
    EXPECT_EQ(WMError::WM_OK, windowSceneSession->Show());
    EXPECT_EQ(WMError::WM_OK, windowSceneSession->Destroy(true));
}

/**
 * @tc.name: CreateAndConnectSpecificSession17
 * @tc.desc: CreateAndConnectSpecificSession
 * @tc.type: FUNC
 */
HWTEST_F(WindowSceneSessionImplTest, CreateAndConnectSpecificSession17, TestSize.Level0)
{
    if (!SceneBoardJudgement::IsSceneBoardEnabled()) {
        GTEST_SKIP() << "SceneBoard is not enabled, skipping test.";
    }
    sptr<WindowOption> option = sptr<WindowOption>::MakeSptr();
    option->SetWindowName("CreateAndConnectSpecificSession17");
    sptr<WindowSceneSessionImpl> windowSceneSession = sptr<WindowSceneSessionImpl>::MakeSptr(option);

    windowSceneSession->SetWindowType(WindowType::WINDOW_TYPE_INPUT_METHOD_FLOAT);
    EXPECT_EQ(WMError::WM_ERROR_NULLPTR, windowSceneSession->CreateAndConnectSpecificSession());
    windowSceneSession->property_->SetPersistentId(102); // 102 is persistentId
    windowSceneSession->property_->SetParentPersistentId(100); // 100 is parentPersistentId
    windowSceneSession->property_->SetParentId(100); // 100 is parentId
    windowSceneSession->property_->SetWindowMode(WindowMode::WINDOW_MODE_FLOATING);
    SessionInfo sessionInfo = { "CreateTestBundle", "CreateTestModule", "CreateTestAbility" };
    sptr<SessionMocker> session = sptr<SessionMocker>::MakeSptr(sessionInfo);

    EXPECT_EQ(WMError::WM_OK, windowSceneSession->Create(abilityContext_, session));
    EXPECT_EQ(WMError::WM_OK, windowSceneSession->Show());
    EXPECT_EQ(WMError::WM_OK, windowSceneSession->Destroy(true));
    windowSceneSession->property_->SetWindowType(WindowType::WINDOW_TYPE_APP_MAIN_WINDOW);
    windowSceneSession->RegisterListenerForKeyboard();
    windowSceneSession->property_->SetWindowType(WindowType::WINDOW_TYPE_INPUT_METHOD_FLOAT);
    windowSceneSession->RegisterListenerForKeyboard();
}

/**
 * @tc.name: IsValidSystemWindowType01
 * @tc.desc: IsValidSystemWindowType
 * @tc.type: FUNC
 */
HWTEST_F(WindowSceneSessionImplTest, IsValidSystemWindowType01, TestSize.Level1)
{
    sptr<WindowOption> option = sptr<WindowOption>::MakeSptr();
    option->SetWindowName("IsValidSystemWindowType01");
    sptr<WindowSceneSessionImpl> windowSceneSession = sptr<WindowSceneSessionImpl>::MakeSptr(option);
    ASSERT_NE(nullptr, windowSceneSession);
    ASSERT_FALSE(!windowSceneSession->IsValidSystemWindowType(WindowType::WINDOW_TYPE_SYSTEM_ALARM_WINDOW));
    ASSERT_FALSE(!windowSceneSession->IsValidSystemWindowType(WindowType::WINDOW_TYPE_INPUT_METHOD_FLOAT));
    ASSERT_FALSE(!windowSceneSession->IsValidSystemWindowType(WindowType::WINDOW_TYPE_FLOAT_CAMERA));
    ASSERT_FALSE(!windowSceneSession->IsValidSystemWindowType(WindowType::WINDOW_TYPE_DIALOG));
    ASSERT_FALSE(!windowSceneSession->IsValidSystemWindowType(WindowType::WINDOW_TYPE_FLOAT));
    ASSERT_FALSE(!windowSceneSession->IsValidSystemWindowType(WindowType::WINDOW_TYPE_SCREENSHOT));
    ASSERT_FALSE(!windowSceneSession->IsValidSystemWindowType(WindowType::WINDOW_TYPE_GLOBAL_SEARCH));
    ASSERT_FALSE(!windowSceneSession->IsValidSystemWindowType(WindowType::WINDOW_TYPE_VOICE_INTERACTION));
    ASSERT_FALSE(!windowSceneSession->IsValidSystemWindowType(WindowType::WINDOW_TYPE_POINTER));
    ASSERT_FALSE(!windowSceneSession->IsValidSystemWindowType(WindowType::WINDOW_TYPE_TOAST));
    ASSERT_FALSE(!windowSceneSession->IsValidSystemWindowType(WindowType::WINDOW_TYPE_DOCK_SLICE));
    ASSERT_TRUE(!windowSceneSession->IsValidSystemWindowType(WindowType::WINDOW_TYPE_APP_LAUNCHING));
}

/**
 * @tc.name: InvalidWindow
 * @tc.desc: InvalidWindow test
 * @tc.type: FUNC
 */
HWTEST_F(WindowSceneSessionImplTest, InvalidWindow, TestSize.Level1)
{
    sptr<WindowOption> option = sptr<WindowOption>::MakeSptr();
    option->SetWindowName("InvalidWindow");
    sptr<WindowSceneSessionImpl> window = sptr<WindowSceneSessionImpl>::MakeSptr(option);
    ASSERT_EQ(WMError::WM_ERROR_INVALID_WINDOW, window->MoveTo(0, 0));
    ASSERT_EQ(WMError::WM_ERROR_INVALID_OPERATION, window->Resize(0, 0));
    ASSERT_EQ(WMError::WM_ERROR_INVALID_WINDOW, window->SetBackgroundColor(std::string("???")));
    ASSERT_EQ(WMError::WM_ERROR_INVALID_WINDOW, window->SetTransparent(false));
    ASSERT_EQ(WMError::WM_ERROR_INVALID_WINDOW, window->Show(2, false));
    ASSERT_EQ(WMError::WM_ERROR_INVALID_OPERATION, window->Resize(2, 2));
    ASSERT_EQ(WMError::WM_ERROR_INVALID_WINDOW, window->Minimize());
    ASSERT_EQ(WMError::WM_ERROR_INVALID_WINDOW, window->Maximize());
    ASSERT_EQ(WMError::WM_ERROR_INVALID_WINDOW, window->MaximizeFloating());
    ASSERT_EQ(WMError::WM_ERROR_INVALID_WINDOW, window->Recover());
    ASSERT_EQ(WMError::WM_ERROR_INVALID_WINDOW, window->MaximizeFloating());
    ASSERT_EQ(WMError::WM_ERROR_INVALID_WINDOW, window->SetGlobalMaximizeMode(MaximizeMode::MODE_AVOID_SYSTEM_BAR));
}

/**
 * @tc.name: FindParentSessionByParentId01
 * @tc.desc: FindParentSessionByParentId
 * @tc.type: FUNC
 */
HWTEST_F(WindowSceneSessionImplTest, FindParentSessionByParentId01, TestSize.Level1)
{
    sptr<WindowOption> option = sptr<WindowOption>::MakeSptr();
    option->SetWindowTag(WindowTag::MAIN_WINDOW);
    option->SetWindowName("FindParentSessionByParentId01");
    sptr<WindowSceneSessionImpl> windowSceneSession = sptr<WindowSceneSessionImpl>::MakeSptr(option);
    ASSERT_NE(nullptr, windowSceneSession);

    windowSceneSession->SetWindowType(WindowType::WINDOW_TYPE_DIALOG);
    ASSERT_TRUE(windowSceneSession->FindMainWindowWithContext() == nullptr);
    windowSceneSession->SetWindowType(WindowType::ABOVE_APP_SYSTEM_WINDOW_END);
    ASSERT_TRUE(windowSceneSession->FindMainWindowWithContext() == nullptr);

    windowSceneSession->property_->SetPersistentId(1112);
    windowSceneSession->property_->SetParentId(1000);
    windowSceneSession->property_->SetParentPersistentId(1000);
    windowSceneSession->property_->SetWindowType(WindowType::WINDOW_TYPE_APP_MAIN_WINDOW);
    SessionInfo sessionInfo = { "CreateTestBundle", "CreateTestModule", "CreateTestAbility" };
    sptr<SessionMocker> session = sptr<SessionMocker>::MakeSptr(sessionInfo);
    ASSERT_NE(nullptr, session);

    ASSERT_EQ(WMError::WM_OK, windowSceneSession->Create(abilityContext_, session));
    windowSceneSession->hostSession_ = session;
    ASSERT_TRUE(nullptr != windowSceneSession->FindParentSessionByParentId(1112));
    windowSceneSession->Destroy(true);
}

/**
 * @tc.name: DisableAppWindowDecor01
 * @tc.desc: DisableAppWindowDecor
 * @tc.type: FUNC
 */
HWTEST_F(WindowSceneSessionImplTest, DisableAppWindowDecor01, TestSize.Level1)
{
    sptr<WindowOption> option = sptr<WindowOption>::MakeSptr();
    option->SetWindowName("DisableAppWindowDecor01");
    sptr<WindowSceneSessionImpl> windowSession = sptr<WindowSceneSessionImpl>::MakeSptr(option);
    ASSERT_NE(nullptr, windowSession);

    SessionInfo sessionInfo = { "CreateTestBundle", "CreateTestModule", "CreateTestAbility" };
    sptr<SessionMocker> session = sptr<SessionMocker>::MakeSptr(sessionInfo);

    std::shared_ptr<AbilityRuntime::Context> context;
    ASSERT_EQ(WMError::WM_OK, windowSession->Create(context, session));
    windowSession->property_->SetPersistentId(1);

    windowSession->UpdateDecorEnable(false);
    windowSession->windowSystemConfig_.isSystemDecorEnable_ = false;

    windowSession->property_->SetWindowType(WindowType::WINDOW_TYPE_FLOAT);
    windowSession->DisableAppWindowDecor();
    ASSERT_FALSE(windowSession->IsDecorEnable());
    windowSession->SetWindowType(WindowType::WINDOW_TYPE_APP_MAIN_WINDOW);
    windowSession->DisableAppWindowDecor();
    windowSession->Destroy(true);
}

/**
 * @tc.name: DisableAppWindowDecor02
 * @tc.desc: DisableAppWindowDecor
 * @tc.type: FUNC
 */
HWTEST_F(WindowSceneSessionImplTest, DisableAppWindowDecor02, TestSize.Level1)
{
    sptr<WindowOption> option = sptr<WindowOption>::MakeSptr();
    option->SetWindowName("DisableAppWindowDecor02");
    sptr<WindowSceneSessionImpl> windowSession = sptr<WindowSceneSessionImpl>::MakeSptr(option);

    SessionInfo sessionInfo = { "CreateTestBundle", "CreateTestModule", "CreateTestAbility" };
    sptr<SessionMocker> session = sptr<SessionMocker>::MakeSptr(sessionInfo);

    std::shared_ptr<AbilityRuntime::Context> context;
    ASSERT_EQ(WMError::WM_OK, windowSession->Create(context, session));
    windowSession->property_->SetPersistentId(0);

    windowSession->UpdateDecorEnable(false);
    windowSession->windowSystemConfig_.isSystemDecorEnable_ = true;

    windowSession->property_->SetWindowType(WindowType::WINDOW_TYPE_FLOAT);
    EXPECT_EQ(WMError::WM_ERROR_INVALID_WINDOW, windowSession->DisableAppWindowDecor());
    windowSession->Destroy(true);
}

/**
 * @tc.name: DisableAppWindowDecor03
 * @tc.desc: DisableAppWindowDecor
 * @tc.type: FUNC
 */
HWTEST_F(WindowSceneSessionImplTest, DisableAppWindowDecor03, TestSize.Level1)
{
    sptr<WindowOption> option = sptr<WindowOption>::MakeSptr();
    option->SetWindowName("DisableAppWindowDecor03");
    sptr<WindowSceneSessionImpl> windowSession = sptr<WindowSceneSessionImpl>::MakeSptr(option);

    SessionInfo sessionInfo = { "CreateTestBundle", "CreateTestModule", "CreateTestAbility" };
    sptr<SessionMocker> session = sptr<SessionMocker>::MakeSptr(sessionInfo);

    std::shared_ptr<AbilityRuntime::Context> context;
    ASSERT_EQ(WMError::WM_OK, windowSession->Create(context, session));
    windowSession->property_->SetPersistentId(1);

    windowSession->UpdateDecorEnable(false);
    windowSession->windowSystemConfig_.isSystemDecorEnable_ = true;

    windowSession->property_->SetWindowType(WindowType::WINDOW_TYPE_APP_SUB_WINDOW);
    EXPECT_EQ(WMError::WM_ERROR_INVALID_OPERATION, windowSession->DisableAppWindowDecor());
    windowSession->Destroy(true);
}

/**
 * @tc.name: RaiseToAppTop01
 * @tc.desc: RaiseToAppTop
 * @tc.type: FUNC
 */
HWTEST_F(WindowSceneSessionImplTest, RaiseToAppTop01, TestSize.Level1)
{
    sptr<WindowOption> option = sptr<WindowOption>::MakeSptr();
    option->SetWindowName("RaiseToAppTop01");
    sptr<WindowSceneSessionImpl> windowSceneSession = sptr<WindowSceneSessionImpl>::MakeSptr(option);
    ASSERT_NE(nullptr, windowSceneSession);

    windowSceneSession->property_->SetPersistentId(6);
    windowSceneSession->property_->SetParentPersistentId(6);
    windowSceneSession->property_->SetWindowType(WindowType::WINDOW_TYPE_APP_MAIN_WINDOW);
    ASSERT_EQ(WMError::WM_ERROR_INVALID_WINDOW, windowSceneSession->RaiseToAppTop());

    SessionInfo sessionInfo = { "CreateTestBundle", "CreateTestModule", "CreateTestAbility" };
    sptr<SessionMocker> session = sptr<SessionMocker>::MakeSptr(sessionInfo);
    windowSceneSession->hostSession_ = session;
    ASSERT_EQ(WMError::WM_ERROR_INVALID_CALLING, windowSceneSession->RaiseToAppTop());

    windowSceneSession->property_->SetWindowType(WindowType::WINDOW_TYPE_APP_SUB_WINDOW);
    windowSceneSession->state_ = WindowState::STATE_HIDDEN;
    ASSERT_EQ(WMError::WM_DO_NOTHING, windowSceneSession->RaiseToAppTop());

    windowSceneSession->state_ = WindowState::STATE_SHOWN;
    ASSERT_EQ(WMError::WM_OK, windowSceneSession->RaiseToAppTop());
}

/**
 * @tc.name: GetGlobalScaledRect
 * @tc.desc: GetGlobalScaledRect
 * @tc.type: FUNC
 */
HWTEST_F(WindowSceneSessionImplTest, GetGlobalScaledRect, TestSize.Level1)
{
    sptr<WindowOption> option = sptr<WindowOption>::MakeSptr();
    option->SetWindowName("GetGlobalScaledRect");
    sptr<WindowSceneSessionImpl> windowSceneSession = sptr<WindowSceneSessionImpl>::MakeSptr(option);
    Rect globalScaledRect;
    windowSceneSession->property_->SetPersistentId(6);
    ASSERT_EQ(WMError::WM_ERROR_INVALID_WINDOW, windowSceneSession->GetGlobalScaledRect(globalScaledRect));

    SessionInfo sessionInfo = { "CreateTestBundle", "CreateTestModule", "CreateTestAbility" };
    sptr<SessionMocker> session = sptr<SessionMocker>::MakeSptr(sessionInfo);
    windowSceneSession->hostSession_ = session;
    EXPECT_CALL(*session, GetGlobalScaledRect(_)).Times(1).WillOnce(Return(WMError::WM_OK));
    ASSERT_EQ(WMError::WM_OK, windowSceneSession->GetGlobalScaledRect(globalScaledRect));
}

/**
 * @tc.name: Minimize01
 * @tc.desc: Minimize
 * @tc.type: FUNC
 */
HWTEST_F(WindowSceneSessionImplTest, Minimize01, TestSize.Level1)
{
    sptr<WindowOption> option = sptr<WindowOption>::MakeSptr();
    option->SetWindowName("Minimize01");
    sptr<WindowSceneSessionImpl> windowSceneSession = sptr<WindowSceneSessionImpl>::MakeSptr(option);

    windowSceneSession->property_->SetPersistentId(1);
    windowSceneSession->property_->SetWindowType(WindowType::WINDOW_TYPE_APP_MAIN_WINDOW);
    SessionInfo sessionInfo = { "CreateTestBundle", "CreateTestModule", "CreateTestAbility" };
    sptr<SessionMocker> session = sptr<SessionMocker>::MakeSptr(sessionInfo);

    windowSceneSession->hostSession_ = session;
    ASSERT_EQ(WMError::WM_OK, windowSceneSession->Minimize());
}

/**
 * @tc.name: Minimize02
 * @tc.desc: Minimize
 * @tc.type: FUNC
 */
HWTEST_F(WindowSceneSessionImplTest, Minimize02, TestSize.Level1)
{
    sptr<WindowOption> option = sptr<WindowOption>::MakeSptr();
    option->SetWindowName("Minimize02");
    sptr<WindowSceneSessionImpl> windowSceneSession = sptr<WindowSceneSessionImpl>::MakeSptr(option);
    windowSceneSession->property_->SetPersistentId(0);
    ASSERT_EQ(WMError::WM_ERROR_INVALID_WINDOW, windowSceneSession->Minimize());
}

/**
 * @tc.name: Minimize03
 * @tc.desc: Minimize
 * @tc.type: FUNC
 */
HWTEST_F(WindowSceneSessionImplTest, Minimize03, TestSize.Level1)
{
    sptr<WindowOption> option = sptr<WindowOption>::MakeSptr();
    option->SetWindowName("Minimize03");
    sptr<WindowSceneSessionImpl> windowSceneSession = sptr<WindowSceneSessionImpl>::MakeSptr(option);

    windowSceneSession->property_->SetPersistentId(1);
    windowSceneSession->property_->SetWindowType(WindowType::WINDOW_TYPE_APP_MAIN_WINDOW);
    SessionInfo sessionInfo = { "CreateTestBundle", "CreateTestModule", "CreateTestAbility" };
    sptr<SessionMocker> session = sptr<SessionMocker>::MakeSptr(sessionInfo);

    ASSERT_EQ(WMError::WM_ERROR_INVALID_WINDOW, windowSceneSession->Minimize());
}

/**
 * @tc.name: Minimize04
 * @tc.desc: Minimize
 * @tc.type: FUNC
 */
HWTEST_F(WindowSceneSessionImplTest, Minimize04, TestSize.Level1)
{
    sptr<WindowOption> option = sptr<WindowOption>::MakeSptr();
    option->SetWindowName("Minimize04");
    sptr<WindowSceneSessionImpl> windowSceneSession = sptr<WindowSceneSessionImpl>::MakeSptr(option);

    windowSceneSession->property_->SetPersistentId(1);
    windowSceneSession->property_->SetWindowType(WindowType::WINDOW_TYPE_APP_MAIN_WINDOW);
    SessionInfo sessionInfo = { "CreateTestBundle", "CreateTestModule", "CreateTestAbility" };
    sptr<SessionMocker> session = sptr<SessionMocker>::MakeSptr(sessionInfo);
    windowSceneSession->hostSession_ = session;
    windowSceneSession->property_->SetWindowType(WindowType::WINDOW_TYPE_APP_SUB_WINDOW);
    ASSERT_EQ(WMError::WM_DO_NOTHING, windowSceneSession->Minimize());
}

/**
 * @tc.name: StartMove01
 * @tc.desc: StartMove
 * @tc.type: FUNC
 */
HWTEST_F(WindowSceneSessionImplTest, StartMove01, TestSize.Level1)
{
    sptr<WindowOption> option = sptr<WindowOption>::MakeSptr();
    option->SetWindowName("StartMove01");
    sptr<WindowSceneSessionImpl> windowSceneSession = sptr<WindowSceneSessionImpl>::MakeSptr(option);
    windowSceneSession->property_->SetPersistentId(1);
    // show with null session

    windowSceneSession->property_->SetWindowType(WindowType::WINDOW_TYPE_APP_MAIN_WINDOW);
    SessionInfo sessionInfo = { "CreateTestBundle", "CreateTestModule", "CreateTestAbility" };
    sptr<SessionMocker> session = sptr<SessionMocker>::MakeSptr(sessionInfo);
    windowSceneSession->hostSession_ = session;
    windowSceneSession->StartMove();
    ASSERT_NE(nullptr, session);
}

/**
 * @tc.name: StartMoveWindow_IsDeviceSupportOrNot
 * @tc.desc: StartMoveWindow Test, is device support or not
 * @tc.type: FUNC
 */
HWTEST_F(WindowSceneSessionImplTest, StartMoveWindow_IsDeviceSupportOrNot, TestSize.Level1)
{
    sptr<WindowOption> option = sptr<WindowOption>::MakeSptr();
    option->SetWindowName("StartMoveWindow_IsDeviceSupportOrNot");
    sptr<WindowSceneSessionImpl> window = sptr<WindowSceneSessionImpl>::MakeSptr(option);
    window->property_->SetPersistentId(1);
    window->property_->SetWindowType(WindowType::WINDOW_TYPE_GLOBAL_SEARCH);
    SessionInfo sessionInfo = { "CreateTestBundle", "CreateTestModule", "CreateTestAbility" };
    sptr<SessionMocker> session = sptr<SessionMocker>::MakeSptr(sessionInfo);
    window->hostSession_ = session;

    window->windowSystemConfig_.windowUIType_ = WindowUIType::PHONE_WINDOW;
    window->property_->type_ = WindowType::WINDOW_TYPE_APP_MAIN_WINDOW;
    ASSERT_EQ(window->StartMoveWindow(), WmErrorCode::WM_ERROR_INVALID_CALLING);

    window->windowSystemConfig_.windowUIType_ = WindowUIType::PC_WINDOW;
    ASSERT_NE(window->StartMoveWindow(), WmErrorCode::WM_ERROR_DEVICE_NOT_SUPPORT);

    window->windowSystemConfig_.windowUIType_ = WindowUIType::INVALID_WINDOW;
    ASSERT_EQ(window->StartMoveWindow(), WmErrorCode::WM_ERROR_INVALID_CALLING);

    window->windowSystemConfig_.windowUIType_ = WindowUIType::PAD_WINDOW;
    window->windowSystemConfig_.freeMultiWindowEnable_ = false;
    window->windowSystemConfig_.freeMultiWindowSupport_ = false;
    window->property_->SetIsPcAppInPad(false);
    ASSERT_EQ(window->StartMoveWindow(), WmErrorCode::WM_ERROR_INVALID_CALLING);
}

/**
 * @tc.name: StartMoveWindow_02
 * @tc.desc: StartMoveWindow Test
 * @tc.type: FUNC
 */
HWTEST_F(WindowSceneSessionImplTest, StartMoveWindow_02, TestSize.Level1)
{
    sptr<WindowOption> option = sptr<WindowOption>::MakeSptr();
    option->SetWindowName("StartMoveWindow_02");
    sptr<WindowSceneSessionImpl> window = sptr<WindowSceneSessionImpl>::MakeSptr(option);
    window->property_->SetPersistentId(1);
    window->property_->SetWindowType(WindowType::WINDOW_TYPE_UI_EXTENSION);
    ASSERT_EQ(window->StartMoveWindow(), WmErrorCode::WM_ERROR_INVALID_CALLING);

    window->property_->SetWindowType(WindowType::APP_MAIN_WINDOW_BASE);
    window->windowSystemConfig_.windowUIType_ = WindowUIType::PHONE_WINDOW;
    ASSERT_EQ(window->StartMoveWindow(), WmErrorCode::WM_ERROR_INVALID_CALLING);

    window->windowSystemConfig_.windowUIType_ = WindowUIType::PC_WINDOW;
    ASSERT_EQ(window->StartMoveWindow(), WmErrorCode::WM_ERROR_SYSTEM_ABNORMALLY);

    SessionInfo info = { "CreateTestBundle", "CreateTestModule", "CreateTestAbility" };
    sptr<SceneSession> sceneSession = sptr<SceneSession>::MakeSptr(info, nullptr);
    window->hostSession_ = sceneSession;
    ASSERT_EQ(window->StartMoveWindow(), WmErrorCode::WM_ERROR_STATE_ABNORMALLY);

    sceneSession->moveDragController_ = sptr<MoveDragController>::MakeSptr(wptr(sceneSession));
    ASSERT_EQ(window->StartMoveWindow(), WmErrorCode::WM_OK);
    sceneSession->moveDragController_->hasPointDown_ = true;
    sceneSession->moveDragController_->SetStartMoveFlag(true);
    ASSERT_EQ(window->StartMoveWindow(), WmErrorCode::WM_ERROR_REPEAT_OPERATION);
}

/**
 * @tc.name: StartMoveWindowWithCoordinate_03
 * @tc.desc: StartMoveWindowWithCoordinate Test
 * @tc.type: FUNC
 */
HWTEST_F(WindowSceneSessionImplTest, StartMoveWindowWithCoordinate_03, TestSize.Level1)
{
    sptr<WindowOption> option = sptr<WindowOption>::MakeSptr();
    sptr<WindowSceneSessionImpl> window = sptr<WindowSceneSessionImpl>::MakeSptr(option);
    window->property_->SetPersistentId(1);
    Rect windowRect = { 200, 200, 1000, 1000};
    window->property_->SetWindowRect(windowRect);
    window->property_->SetWindowType(WindowType::WINDOW_TYPE_UI_EXTENSION);
    ASSERT_EQ(window->StartMoveWindowWithCoordinate(-1, 50), WmErrorCode::WM_ERROR_INVALID_CALLING);

    window->property_->SetWindowType(WindowType::APP_MAIN_WINDOW_BASE);
    window->windowSystemConfig_.windowUIType_ = WindowUIType::PHONE_WINDOW;
    ASSERT_EQ(window->StartMoveWindowWithCoordinate(-1, 50), WmErrorCode::WM_ERROR_DEVICE_NOT_SUPPORT);

    window->windowSystemConfig_.windowUIType_ = WindowUIType::PC_WINDOW;
    ASSERT_EQ(window->StartMoveWindowWithCoordinate(-1, 50), WmErrorCode::WM_ERROR_STATE_ABNORMALLY);

    SessionInfo info = { "CreateTestBundle", "CreateTestModule", "CreateTestAbility" };
    sptr<SceneSession> sceneSession = sptr<SceneSession>::MakeSptr(info, nullptr);
    window->hostSession_ = sceneSession;
    ASSERT_EQ(window->StartMoveWindowWithCoordinate(-1, 50), WmErrorCode::WM_ERROR_INVALID_PARAM);

    sceneSession->moveDragController_ = sptr<MoveDragController>::MakeSptr(wptr(sceneSession));
    ASSERT_EQ(window->StartMoveWindowWithCoordinate(500, 500), WmErrorCode::WM_OK);
    sceneSession->moveDragController_->hasPointDown_ = true;
    sceneSession->moveDragController_->SetStartMoveFlag(true);
    ASSERT_EQ(window->StartMoveWindowWithCoordinate(500, 500), WmErrorCode::WM_ERROR_REPEAT_OPERATION);
}

/**
 * @tc.name: StopMoveWindow
 * @tc.desc: StopMoveWindow Test
 * @tc.type: FUNC
 */
HWTEST_F(WindowSceneSessionImplTest, StopMoveWindow, TestSize.Level1)
{
    sptr<WindowOption> option = sptr<WindowOption>::MakeSptr();
    option->SetWindowName("StopMoveWindow");
    sptr<WindowSceneSessionImpl> window = sptr<WindowSceneSessionImpl>::MakeSptr(option);
    window->property_->SetPersistentId(1);
    window->property_->SetWindowType(WindowType::WINDOW_TYPE_UI_EXTENSION);
    ASSERT_EQ(window->StopMoveWindow(), WmErrorCode::WM_ERROR_INVALID_CALLING);

    window->property_->SetWindowType(WindowType::APP_MAIN_WINDOW_BASE);
    window->windowSystemConfig_.windowUIType_ = WindowUIType::PHONE_WINDOW;
    ASSERT_EQ(window->StopMoveWindow(), WmErrorCode::WM_ERROR_DEVICE_NOT_SUPPORT);

    window->windowSystemConfig_.windowUIType_ = WindowUIType::PC_WINDOW;
    ASSERT_EQ(window->StopMoveWindow(), WmErrorCode::WM_ERROR_STATE_ABNORMALLY);

    SessionInfo info = { "CreateTestBundle", "CreateTestModule", "CreateTestAbility" };
    sptr<SceneSession> sceneSession = sptr<SceneSession>::MakeSptr(info, nullptr);
    window->hostSession_ = sceneSession;
    ASSERT_EQ(window->StopMoveWindow(), WmErrorCode::WM_ERROR_STATE_ABNORMALLY);

    sceneSession->moveDragController_ = sptr<MoveDragController>::MakeSptr(wptr(sceneSession));
    ASSERT_EQ(window->StopMoveWindow(), WmErrorCode::WM_OK);
}
/**
 * @tc.name: Close01
 * @tc.desc: Close
 * @tc.type: FUNC
 */
HWTEST_F(WindowSceneSessionImplTest, Close01, TestSize.Level1)
{
    sptr<WindowOption> option = sptr<WindowOption>::MakeSptr();
    option->SetWindowName("Close01");
    sptr<WindowSceneSessionImpl> windowSceneSession = sptr<WindowSceneSessionImpl>::MakeSptr(option);
    windowSceneSession->property_->SetPersistentId(1);
    windowSceneSession->property_->SetWindowType(WindowType::WINDOW_TYPE_APP_MAIN_WINDOW);
    SessionInfo sessionInfo = { "CreateTestBundle", "CreateTestModule", "CreateTestAbility" };
    sptr<SessionMocker> session = sptr<SessionMocker>::MakeSptr(sessionInfo);
    windowSceneSession->hostSession_ = session;

    sptr<IWindowWillCloseListener> listener = sptr<IWindowWillCloseListener>::MakeSptr();
    windowSceneSession->windowSystemConfig_.windowUIType_ = WindowUIType::PC_WINDOW;
    auto res = windowSceneSession->RegisterWindowWillCloseListeners(listener);
    ASSERT_EQ(WMError::WM_OK, res);
    ASSERT_EQ(WMError::WM_OK, windowSceneSession->Close());
    res = windowSceneSession->UnRegisterWindowWillCloseListeners(listener);
    ASSERT_EQ(WMError::WM_OK, res);
    ASSERT_EQ(WMError::WM_OK, windowSceneSession->Close());
}

/**
 * @tc.name: CloseDirectly
 * @tc.desc: CloseDirectly
 * @tc.type: FUNC
 */
HWTEST_F(WindowSceneSessionImplTest, CloseDirectly, TestSize.Level1)
{
    sptr<WindowOption> option = sptr<WindowOption>::MakeSptr();
    option->SetWindowName("CloseDirectly");
    sptr<WindowSceneSessionImpl> window = sptr<WindowSceneSessionImpl>::MakeSptr(option);
    auto res = window->CloseDirectly();
    ASSERT_EQ(WMError::WM_ERROR_INVALID_WINDOW, res);

    window->property_->SetPersistentId(1);
    window->property_->SetWindowType(WindowType::WINDOW_TYPE_APP_MAIN_WINDOW);
    SessionInfo sessionInfo = { "CreateTestBundle", "CreateTestModule", "CreateTestAbility" };
    sptr<SessionMocker> session = sptr<SessionMocker>::MakeSptr(sessionInfo);
    window->hostSession_ = session;
    res = window->CloseDirectly();
    ASSERT_EQ(WMError::WM_OK, res);

    window->property_->SetWindowType(WindowType::WINDOW_TYPE_APP_SUB_WINDOW);
    res = window->CloseDirectly();
    ASSERT_EQ(WMError::WM_OK, res);
}

/**
 * @tc.name: SetActive01
 * @tc.desc: SetActive
 * @tc.type: FUNC
 */
HWTEST_F(WindowSceneSessionImplTest, SetActive01, TestSize.Level1)
{
    sptr<WindowOption> option = sptr<WindowOption>::MakeSptr();
    option->SetWindowName("SetActive01");
    sptr<WindowSceneSessionImpl> windowSceneSession = sptr<WindowSceneSessionImpl>::MakeSptr(option);

    windowSceneSession->property_->SetPersistentId(1);
    windowSceneSession->property_->SetWindowType(WindowType::WINDOW_TYPE_APP_SUB_WINDOW);
    SessionInfo sessionInfo = { "CreateTestBundle", "CreateTestModule", "CreateTestAbility" };
    sptr<SessionMocker> session = sptr<SessionMocker>::MakeSptr(sessionInfo);

    windowSceneSession->hostSession_ = session;
    ASSERT_EQ(WSError::WS_OK, windowSceneSession->SetActive(false));
    ASSERT_EQ(WSError::WS_OK, windowSceneSession->SetActive(true));
}

/**
 * @tc.name: Recover01
 * @tc.desc: Recover
 * @tc.type: FUNC
 */
HWTEST_F(WindowSceneSessionImplTest, Recover01, TestSize.Level1)
{
    sptr<WindowOption> option = sptr<WindowOption>::MakeSptr();
    option->SetWindowName("Recover01");
    sptr<WindowSceneSessionImpl> windowSceneSession = sptr<WindowSceneSessionImpl>::MakeSptr(option);

    windowSceneSession->property_->SetPersistentId(1);
    windowSceneSession->property_->SetWindowType(WindowType::WINDOW_TYPE_APP_MAIN_WINDOW);
    SessionInfo sessionInfo = { "CreateTestBundle", "CreateTestModule", "CreateTestAbility" };
    sptr<SessionMocker> session = sptr<SessionMocker>::MakeSptr(sessionInfo);

    windowSceneSession->hostSession_ = session;
    ASSERT_EQ(WMError::WM_OK, windowSceneSession->Recover());
}

/**
 * @tc.name: Recover02
 * @tc.desc: Recover02
 * @tc.type: FUNC
 */
HWTEST_F(WindowSceneSessionImplTest, Recover02, Function | SmallTest | Level2)
{
    sptr<WindowOption> subWindowOption = sptr<WindowOption>::MakeSptr();
    subWindowOption->SetWindowName("Recover02");
    subWindowOption->SetWindowType(WindowType::APP_SUB_WINDOW_BASE);
    subWindowOption->SetSubWindowMaximizeSupported(true);
    sptr<WindowSceneSessionImpl> window = sptr<WindowSceneSessionImpl>::MakeSptr(subWindowOption);
    window->property_->SetPersistentId(1);
    window->property_->SetWindowType(WindowType::APP_SUB_WINDOW_BASE);
    window->property_->SetWindowMode(WindowMode::WINDOW_MODE_FLOATING);
    window->windowSystemConfig_.windowUIType_ = WindowUIType::PC_WINDOW;
    SessionInfo sessionInfo = { "CreateTestBundle", "CreateTestModule", "CreateTestAbility" };
    sptr<SessionMocker> session = sptr<SessionMocker>::MakeSptr(sessionInfo);

    window->hostSession_ = session;
    ASSERT_EQ(WMError::WM_ERROR_REPEAT_OPERATION, window->Recover());
    window->property_->SetWindowMode(WindowMode::WINDOW_MODE_FULLSCREEN);
    ASSERT_EQ(WMError::WM_OK, window->Recover());
}

/**
 * @tc.name: Maximize01
 * @tc.desc: Maximize
 * @tc.type: FUNC
 */
HWTEST_F(WindowSceneSessionImplTest, Maximize01, TestSize.Level1)
{
    sptr<WindowOption> option = sptr<WindowOption>::MakeSptr();
    option->SetWindowName("Maximize01");
    option->SetWindowType(WindowType::WINDOW_TYPE_APP_SUB_WINDOW);
    sptr<WindowSceneSessionImpl> windowSceneSession = sptr<WindowSceneSessionImpl>::MakeSptr(option);
    ASSERT_EQ(WMError::WM_ERROR_INVALID_WINDOW, windowSceneSession->Maximize());

    windowSceneSession->property_->SetPersistentId(1);
    SessionInfo sessionInfo = { "CreateTestBundle", "CreateTestModule", "CreateTestAbility" };
    sptr<SessionMocker> session = sptr<SessionMocker>::MakeSptr(sessionInfo);

    windowSceneSession->hostSession_ = session;
    ASSERT_EQ(WMError::WM_OK, windowSceneSession->Maximize());
}

/**
 * @tc.name: MaximizeForCompatibleMode
 * @tc.desc: MaximizeForCompatibleMode
 * @tc.type: FUNC
 */
HWTEST_F(WindowSceneSessionImplTest, MaximizeForCompatibleMode, Function | SmallTest | Level2)
{
    sptr<WindowOption> option = sptr<WindowOption>::MakeSptr();
    option->SetWindowName("MaximizeForCompatibleMode");
    option->SetWindowType(WindowType::WINDOW_TYPE_APP_MAIN_WINDOW);
    sptr<WindowSceneSessionImpl> windowSceneSession = sptr<WindowSceneSessionImpl>::MakeSptr(option);
    ASSERT_EQ(WMError::WM_ERROR_INVALID_WINDOW, windowSceneSession->MaximizeForCompatibleMode());

    windowSceneSession->property_->SetPersistentId(1);
    SessionInfo sessionInfo = { "CreateTestBundle", "CreateTestModule", "CreateTestAbility" };
    sptr<SessionMocker> session = sptr<SessionMocker>::MakeSptr(sessionInfo);

    windowSceneSession->hostSession_ = session;
    ASSERT_EQ(WMError::WM_OK, windowSceneSession->MaximizeForCompatibleMode());
}

/**
 * @tc.name: RecoverForCompatibleMode
 * @tc.desc: RecoverForCompatibleMode
 * @tc.type: FUNC
 */
HWTEST_F(WindowSceneSessionImplTest, RecoverForCompatibleMode, Function | SmallTest | Level2)
{
    sptr<WindowOption> option = sptr<WindowOption>::MakeSptr();
    option->SetWindowName("RecoverForCompatibleMode");
    option->SetWindowType(WindowType::WINDOW_TYPE_APP_MAIN_WINDOW);
    sptr<WindowSceneSessionImpl> windowSceneSession = sptr<WindowSceneSessionImpl>::MakeSptr(option);
    ASSERT_EQ(WMError::WM_ERROR_INVALID_WINDOW, windowSceneSession->RecoverForCompatibleMode());

    windowSceneSession->property_->SetPersistentId(1);
    SessionInfo sessionInfo = { "CreateTestBundle", "CreateTestModule", "CreateTestAbility" };
    sptr<SessionMocker> session = sptr<SessionMocker>::MakeSptr(sessionInfo);

    windowSceneSession->hostSession_ = session;
    ASSERT_EQ(WMError::WM_OK, windowSceneSession->RecoverForCompatibleMode());
}

/**
 * @tc.name: Hide01
 * @tc.desc: Hide session
 * @tc.type: FUNC
 */
HWTEST_F(WindowSceneSessionImplTest, Hide01, TestSize.Level0)
{
    sptr<WindowOption> option = sptr<WindowOption>::MakeSptr();
    option->SetWindowName("Hide01");
    sptr<WindowSceneSessionImpl> window = sptr<WindowSceneSessionImpl>::MakeSptr(option);

    window->property_->SetPersistentId(1);
    // show with null session
    ASSERT_EQ(WMError::WM_ERROR_INVALID_WINDOW, window->Hide(2, false, false));

    SessionInfo sessionInfo = { "CreateTestBundle", "CreateTestModule", "CreateTestAbility" };
    sptr<SessionMocker> session = sptr<SessionMocker>::MakeSptr(sessionInfo);

    window->hostSession_ = session;
    ASSERT_EQ(WMError::WM_OK, window->Hide(2, false, false));
    ASSERT_EQ(WMError::WM_OK, window->Hide(2, false, false));

    window->state_ = WindowState::STATE_CREATED;
    ASSERT_EQ(WMError::WM_OK, window->Hide(2, false, false));
    window->state_ = WindowState::STATE_SHOWN;
    window->property_->type_ = WindowType::APP_SUB_WINDOW_BASE;
    ASSERT_EQ(WMError::WM_OK, window->Hide(2, false, false));

    window->property_->type_ = WindowType::APP_SUB_WINDOW_BASE;
    ASSERT_EQ(WMError::WM_OK, window->Destroy(false));
}

/**
 * @tc.name: Hide02
 * @tc.desc: Hide session
 * @tc.type: FUNC
 */
HWTEST_F(WindowSceneSessionImplTest, Hide02, TestSize.Level0)
{
    sptr<WindowOption> option = sptr<WindowOption>::MakeSptr();
    option->SetWindowName("Hide02");
    sptr<WindowSceneSessionImpl> window = sptr<WindowSceneSessionImpl>::MakeSptr(option);

    window->property_->SetPersistentId(1);
    window->property_->SetWindowType(WindowType::WINDOW_TYPE_APP_SUB_WINDOW);
    // show with null session
    EXPECT_EQ(WMError::WM_ERROR_INVALID_WINDOW, window->Hide(2, false, false));

    SessionInfo sessionInfo = { "CreateTestBundle", "CreateTestModule", "CreateTestAbility" };
    sptr<SessionMocker> session = sptr<SessionMocker>::MakeSptr(sessionInfo);

    window->hostSession_ = session;
    EXPECT_EQ(WMError::WM_OK, window->Hide(0, false, false, true));
    EXPECT_EQ(WMError::WM_OK, window->Destroy(false));
}

/**
 * @tc.name: Hide03
 * @tc.desc: Hide session
 * @tc.type: FUNC
 */
HWTEST_F(WindowSceneSessionImplTest, Hide03, TestSize.Level0)
{
    g_logMsg.clear();
    LOG_SetCallback(LogCallback);
    sptr<WindowOption> option = sptr<WindowOption>::MakeSptr();
    option->SetWindowName("Hide03");
    sptr<WindowSceneSessionImpl> window = sptr<WindowSceneSessionImpl>::MakeSptr(option);

    window->property_->SetPersistentId(1);
    window->property_->SetWindowType(WindowType::WINDOW_TYPE_APP_SUB_WINDOW);
    SessionInfo sessionInfo = { "CreateTestBundle", "CreateTestModule", "CreateTestAbility" };
    sptr<SessionMocker> session = sptr<SessionMocker>::MakeSptr(sessionInfo);
    window->hostSession_ = session;

    window->NotifyWindowAttachStateChange(false);
    EXPECT_TRUE(g_logMsg.find("notifyAttachState id") == std::string::npos);

    EXPECT_EQ(WMError::WM_OK, window->Hide(0, false, false, false));
    EXPECT_TRUE(g_logMsg.find("init lifecycleCallback") == std::string::npos);
    EXPECT_EQ(WMError::WM_OK, window->Show(0, false, true, false));

    window->property_->SetWindowType(WindowType::WINDOW_TYPE_INPUT_METHOD_FLOAT);
    EXPECT_EQ(WMError::WM_OK, window->Hide(0, false, false, true));
    EXPECT_TRUE(g_logMsg.find("init lifecycleCallback") == std::string::npos);
    EXPECT_EQ(WMError::WM_OK, window->Show(0, false, true, true));

    window->property_->SetWindowType(WindowType::WINDOW_TYPE_KEYBOARD_PANEL);
    EXPECT_EQ(WMError::WM_OK, window->Hide(0, false, false, true));
    EXPECT_TRUE(g_logMsg.find("init lifecycleCallback") == std::string::npos);
    EXPECT_EQ(WMError::WM_OK, window->Show(0, false, true, true));

    window->property_->SetWindowType(WindowType::WINDOW_TYPE_APP_SUB_WINDOW);
    window->lifecycleCallback_ = sptr<LifecycleFutureCallback>::MakeSptr();
    EXPECT_EQ(WMError::WM_OK, window->Hide(0, false, false, true));
    EXPECT_TRUE(g_logMsg.find("init lifecycleCallback") == std::string::npos);
    EXPECT_EQ(WMError::WM_OK, window->Show(0, false, true, true));

    window->NotifyWindowAttachStateChange(false);
    EXPECT_TRUE(g_logMsg.find("notifyAttachState id") != std::string::npos);
    EXPECT_EQ(WMError::WM_OK, window->Hide(0, false, false, true));
    EXPECT_TRUE(g_logMsg.find("Window hide") != std::string::npos);
    EXPECT_TRUE(g_logMsg.find("get attach state sync result") != std::string::npos);

    window->lifecycleCallback_ = nullptr;
    EXPECT_EQ(WMError::WM_OK, window->Show(0, false, true, true));
    EXPECT_EQ(WMError::WM_OK, window->Hide(0, false, false, true));
    EXPECT_TRUE(g_logMsg.find("init lifecycleCallback") != std::string::npos);
    EXPECT_EQ(WMError::WM_OK, window->Destroy(false));
}

/**
 * @tc.name: Hide04
 * @tc.desc: Hide session
 * @tc.type: FUNC
 */
HWTEST_F(WindowSceneSessionImplTest, Hide04, TestSize.Level0)
{
    g_logMsg.clear();
    LOG_SetCallback(LogCallback);
    sptr option = sptr::MakeSptr();
    option->SetWindowName("Hide04");
    sptr window = sptr::MakeSptr(option);

    window->property_->SetPersistentId(1);
    window->property_->SetWindowType(WindowType::WINDOW_TYPE_APP_SUB_WINDOW);
    SessionInfo sessionInfo = { "CreateTestBundle", "CreateTestModule", "CreateTestAbility"};
    sptr session = sptr::MakeSptr(sessionInfo);
    window->hostSession_ = session;

    EXPECT_EQ(WMError::WM_OK, window->Hide(0, false, false, false));
    EXPECT_TRUE(g_logMsg.find("init lifecycleCallback") == std::string::npos);
    EXPECT_EQ(WMError::WM_OK, window->Show(0, false, true, false));

    EXPECT_EQ(WMError::WM_OK, window->Hide(0, false, false, true));
    EXPECT_FALSE(g_logMsg.find("lifecycleCallback is null") == std::string::npos);
    EXPECT_FALSE(g_logMsg.find("window attach state timeout, persistentId") == std::string::npos);
    EXPECT_FALSE(g_logMsg.find("get attach state sync result, id") == std::string::npos);
    EXPECT_EQ(WMError::WM_OK, window->Show(0, false, true, true));

    EXPECT_EQ(WMError::WM_OK, window->Destroy(false));
}

/**
 * @tc.name: Show01
 * @tc.desc: Show session
 * @tc.type: FUNC
 */
HWTEST_F(WindowSceneSessionImplTest, Show01, TestSize.Level0)
{
    sptr<WindowOption> option = sptr<WindowOption>::MakeSptr();
    option->SetWindowName("Show01");
    option->SetDisplayId(0);
    sptr<WindowSceneSessionImpl> window = sptr<WindowSceneSessionImpl>::MakeSptr(option);
    window->property_->SetPersistentId(1);

    SessionInfo sessionInfo = { "CreateTestBundle", "CreateTestModule", "CreateTestAbility" };
    sptr<SessionMocker> session = sptr<SessionMocker>::MakeSptr(sessionInfo);

    window->hostSession_ = session;
    ASSERT_EQ(WMError::WM_OK, window->Show(2, false));

    window->state_ = WindowState::STATE_CREATED;
    ASSERT_EQ(WMError::WM_OK, window->Show(2, false));
    ASSERT_EQ(WMError::WM_OK, window->Destroy(false));
}

/**
 * @tc.name: Show02
 * @tc.desc: Show session
 * @tc.type: FUNC
 */
HWTEST_F(WindowSceneSessionImplTest, Show02, TestSize.Level0)
{
    sptr<WindowOption> option = sptr<WindowOption>::MakeSptr();
    option->SetWindowName("Show02");
    option->SetDisplayId(0);
    sptr<WindowSceneSessionImpl> window = sptr<WindowSceneSessionImpl>::MakeSptr(option);
    window->property_->SetPersistentId(1);
    window->property_->SetWindowType(WindowType::WINDOW_TYPE_APP_SUB_WINDOW);

    SessionInfo sessionInfo = { "CreateTestBundle", "CreateTestModule", "CreateTestAbility" };
    sptr<SessionMocker> session = sptr<SessionMocker>::MakeSptr(sessionInfo);

    window->hostSession_ = session;
    ASSERT_EQ(WMError::WM_OK, window->Show(0, false, true, true));
    ASSERT_EQ(WMError::WM_OK, window->Destroy(false));
}

/**
 * @tc.name: Show03
 * @tc.desc: Show session
 * @tc.type: FUNC
 */
HWTEST_F(WindowSceneSessionImplTest, Show03, TestSize.Level0)
{
    g_logMsg.clear();
    LOG_SetCallback(LogCallback);
    sptr<WindowOption> option = sptr<WindowOption>::MakeSptr();
    option->SetWindowName("Show03");
    sptr<WindowSceneSessionImpl> window = sptr<WindowSceneSessionImpl>::MakeSptr(option);

    window->property_->SetPersistentId(1);
    window->property_->SetWindowType(WindowType::WINDOW_TYPE_APP_SUB_WINDOW);
    SessionInfo sessionInfo = { "CreateTestBundle", "CreateTestModule", "CreateTestAbility" };
    sptr<SessionMocker> session = sptr<SessionMocker>::MakeSptr(sessionInfo);
    window->hostSession_ = session;

    window->NotifyWindowAttachStateChange(true);
    EXPECT_TRUE(g_logMsg.find("notifyAttachState id") == std::string::npos);

    EXPECT_EQ(WMError::WM_OK, window->Show(0, false, true, false));
    EXPECT_TRUE(g_logMsg.find("init lifecycleCallback") == std::string::npos);
    EXPECT_EQ(WMError::WM_OK, window->Hide(0, false, false, false));

    window->property_->SetWindowType(WindowType::WINDOW_TYPE_INPUT_METHOD_FLOAT);
    EXPECT_EQ(WMError::WM_OK, window->Show(0, false, true, true));
    EXPECT_TRUE(g_logMsg.find("init lifecycleCallback") == std::string::npos);
    EXPECT_EQ(WMError::WM_OK, window->Hide(0, false, false, true));

    window->property_->SetWindowType(WindowType::WINDOW_TYPE_KEYBOARD_PANEL);
    EXPECT_EQ(WMError::WM_OK, window->Show(0, false, true, true));
    EXPECT_TRUE(g_logMsg.find("init lifecycleCallback") == std::string::npos);
    EXPECT_EQ(WMError::WM_OK, window->Hide(0, false, false, true));

    window->property_->SetWindowType(WindowType::WINDOW_TYPE_APP_SUB_WINDOW);
    window->lifecycleCallback_ = sptr<LifecycleFutureCallback>::MakeSptr();
    EXPECT_EQ(WMError::WM_OK, window->Show(0, false, true, true));
    EXPECT_TRUE(g_logMsg.find("init lifecycleCallback") == std::string::npos);
    EXPECT_EQ(WMError::WM_OK, window->Hide(0, false, false, true));

    window->NotifyWindowAttachStateChange(true);
    EXPECT_TRUE(g_logMsg.find("notifyAttachState id") != std::string::npos);
    EXPECT_EQ(WMError::WM_OK, window->Show(0, false, true, true));
    EXPECT_TRUE(g_logMsg.find("Window show") != std::string::npos);
    EXPECT_TRUE(g_logMsg.find("get attach state sync result") != std::string::npos);

    window->lifecycleCallback_ = nullptr;
    EXPECT_EQ(WMError::WM_OK, window->Hide(0, false, false, true));
    EXPECT_EQ(WMError::WM_OK, window->Show(0, false, true, true));
    EXPECT_TRUE(g_logMsg.find("init lifecycleCallback") != std::string::npos);
    EXPECT_EQ(WMError::WM_OK, window->Destroy(false));
}

/**
 * @tc.name: GetDisplayInfo
 * @tc.desc: get DisplayInfo
 * @tc.type: FUNC
 */
HWTEST_F(WindowSceneSessionImplTest, GetDisplayInfo, TestSize.Level0)
{
    g_logMsg.clear();
    LOG_SetCallback(LogCallback);
    sptr<WindowOption> option = sptr<WindowOption>::MakeSptr();
    option->SetWindowName("GetDisplayInfo");
    option->SetDisplayId(10);
    sptr<WindowSceneSessionImpl> window = sptr<WindowSceneSessionImpl>::MakeSptr(option);
    window->property_->SetPersistentId(1);

    SessionInfo sessionInfo = { "CreateTestBundle", "CreateTestModule", "CreateTestAbility" };
    sptr<SessionMocker> session = sptr<SessionMocker>::MakeSptr(sessionInfo);
    window->hostSession_ = session;

    window->property_->SetWindowType(WindowType::SYSTEM_WINDOW_BASE);
    EXPECT_TRUE(g_logMsg.find("use default display id") == std::string::npos);
    EXPECT_EQ(true, window->GetDisplayInfo() == nullptr);
    
    window->property_->SetWindowType(WindowType::WINDOW_TYPE_APP_SUB_WINDOW);
    EXPECT_EQ(true, window->GetDisplayInfo() != nullptr);
    EXPECT_TRUE(g_logMsg.find("use default display id") != std::string::npos);
    EXPECT_EQ(WSError::WS_OK, window->UpdateDisplayId(0));
    EXPECT_EQ(true, window->GetDisplayInfo() != nullptr);

    EXPECT_EQ(WMError::WM_OK, window->Destroy(false));
}

/**
 * @tc.name: NotifyDrawingCompleted
 * @tc.desc: NotifyDrawingCompleted session
 * @tc.type: FUNC
 */
HWTEST_F(WindowSceneSessionImplTest, NotifyDrawingCompleted, TestSize.Level1)
{
    sptr<WindowOption> option = sptr<WindowOption>::MakeSptr();
    ASSERT_NE(nullptr, option);
    option->SetWindowName("NotifyDrawingCompleted");
    option->SetDisplayId(0);
    sptr<WindowSceneSessionImpl> window = sptr<WindowSceneSessionImpl>::MakeSptr(option);
    ASSERT_NE(nullptr, window->property_);
    window->property_->SetPersistentId(1);

    SessionInfo sessionInfo = { "CreateTestBundle", "CreateTestModule", "CreateTestAbility" };
    sptr<SessionMocker> session = sptr<SessionMocker>::MakeSptr(sessionInfo);

    window->hostSession_ = session;
    auto ret = window->NotifyDrawingCompleted();
    ASSERT_EQ(ret, WMError::WM_ERROR_INVALID_PERMISSION);
}

/**
 * @tc.name: NotifyRemoveStartingWindow
 * @tc.desc: NotifyRemoveStartingWindow session
 * @tc.type: FUNC
 */
HWTEST_F(WindowSceneSessionImplTest, NotifyRemoveStartingWindow, TestSize.Level1)
{
    sptr<WindowOption> option = sptr<WindowOption>::MakeSptr();
    option->SetWindowName("NotifyRemoveStartingWindow");
    option->SetDisplayId(0);
    sptr<WindowSceneSessionImpl> window = sptr<WindowSceneSessionImpl>::MakeSptr(option);
    auto ret = window->NotifyRemoveStartingWindow();
    ASSERT_EQ(ret, WMError::WM_ERROR_INVALID_WINDOW);
    window->property_->SetPersistentId(1);
    window->windowSystemConfig_.windowUIType_ = WindowUIType::PC_WINDOW;

    SessionInfo sessionInfo = { "CreateTestBundle", "CreateTestModule", "CreateTestAbility" };
    sptr<SessionMocker> session = sptr<SessionMocker>::MakeSptr(sessionInfo);
    window->hostSession_ = session;
    window->property_->SetWindowType(WindowType::WINDOW_TYPE_APP_SUB_WINDOW);
    ret = window->NotifyRemoveStartingWindow();
    ASSERT_EQ(ret, WMError::WM_ERROR_INVALID_WINDOW);
    window->property_->SetWindowType(WindowType::WINDOW_TYPE_APP_MAIN_WINDOW);
    EXPECT_CALL(*(session), RemoveStartingWindow()).WillOnce(Return(WSError::WS_OK));
    ret = window->NotifyRemoveStartingWindow();
    ASSERT_EQ(ret, WMError::WM_OK);
}

/**
 * @tc.name: SetTransparent
 * @tc.desc: SetTransparent test
 * @tc.type: FUNC
 */
HWTEST_F(WindowSceneSessionImplTest, SetTransparent, TestSize.Level1)
{
    sptr<WindowOption> option = sptr<WindowOption>::MakeSptr();
    option->SetWindowName("SetTransparent");
    sptr<WindowSceneSessionImpl> window = sptr<WindowSceneSessionImpl>::MakeSptr(option);
    ASSERT_EQ(WMError::WM_ERROR_INVALID_WINDOW, window->SetTransparent(true));
    window->property_->SetPersistentId(1);
    window->property_->SetWindowType(WindowType::APP_SUB_WINDOW_BASE);
    SessionInfo sessionInfo = { "CreateTestBundle", "CreateTestModule", "CreateTestAbility" };
    sptr<SessionMocker> session = sptr<SessionMocker>::MakeSptr(sessionInfo);
    ASSERT_NE(nullptr, session);
    window->hostSession_ = session;
    window->uiContent_ = std::make_unique<Ace::UIContentMocker>();
    window->SetBackgroundColor(333);

    auto ret = window->SetTransparent(true);
    ASSERT_EQ(ret, WMError::WM_OK);
}

/**
 * @tc.name: GetTopwindowWithId
 * @tc.desc: GetTopwindowWithId test
 * @tc.type: FUNC
 */
HWTEST_F(WindowSceneSessionImplTest, GetTopwindowWithId, TestSize.Level1)
{
    sptr<WindowOption> option = sptr<WindowOption>::MakeSptr();
    option->SetWindowName("GetTopwindowWithId");
    sptr<WindowSceneSessionImpl> window = sptr<WindowSceneSessionImpl>::MakeSptr(option);
    sptr<WindowSessionImpl> session = sptr<WindowSessionImpl>::MakeSptr(option);
    uint32_t windowId = 1;
    string winName = "test";
    WindowSessionImpl::windowSessionMap_.insert(
        std::make_pair(winName, std::pair<int32_t, sptr<WindowSessionImpl>>(windowId, session)));
    EXPECT_CALL(m->Mock(), GetTopWindowId(_, _)).Times(1).WillOnce(Return(WMError::WM_DO_NOTHING));
    uint32_t mainWinId = 1;
    ASSERT_EQ(nullptr, window->GetTopWindowWithId(mainWinId));

    EXPECT_CALL(m->Mock(), GetTopWindowId(_, _))
        .Times(1)
        .WillOnce(DoAll(SetArgReferee<1>(windowId), Return(WMError::WM_OK)));
    ASSERT_NE(nullptr, window->GetTopWindowWithId(mainWinId));

    int32_t tempWinId = 3;
    EXPECT_CALL(m->Mock(), GetTopWindowId(_, _))
        .Times(1)
        .WillOnce(DoAll(SetArgReferee<1>(tempWinId), Return(WMError::WM_OK)));
    ASSERT_EQ(nullptr, window->GetTopWindowWithId(mainWinId));

    WindowSessionImpl::windowSessionMap_.erase(winName);
}

/**
 * @tc.name: GetAvoidAreaByType
 * @tc.desc: GetAvoidAreaByType test
 * @tc.type: FUNC
 */
HWTEST_F(WindowSceneSessionImplTest, GetAvoidAreaByType, TestSize.Level1)
{
    sptr<WindowOption> option = sptr<WindowOption>::MakeSptr();
    option->SetWindowMode(WindowMode::WINDOW_MODE_PIP);
    option->SetWindowName("GetAvoidAreaByType");
    sptr<WindowSceneSessionImpl> window = sptr<WindowSceneSessionImpl>::MakeSptr(option);

    window->property_->SetPersistentId(1);
    SessionInfo sessionInfo = { "CreateTestBundle", "CreateTestModule", "CreateTestAbility" };
    sptr<SessionMocker> session = sptr<SessionMocker>::MakeSptr(sessionInfo);
    AvoidArea avoidarea;
    ASSERT_EQ(WMError::WM_ERROR_INVALID_WINDOW, window->GetAvoidAreaByType(AvoidAreaType::TYPE_CUTOUT, avoidarea));
    window->hostSession_ = session;
    ASSERT_EQ(WMError::WM_OK, window->GetAvoidAreaByType(AvoidAreaType::TYPE_CUTOUT, avoidarea));
    window->property_->SetWindowType(WindowType::WINDOW_TYPE_FLOAT);
    Rect rect;
    ASSERT_EQ(WMError::WM_OK, window->GetAvoidAreaByType(AvoidAreaType::TYPE_CUTOUT, avoidarea, rect, 15));
    ASSERT_EQ(WMError::WM_OK, window->GetAvoidAreaByType(AvoidAreaType::TYPE_CUTOUT, avoidarea, rect, 16));
}

/**
 * @tc.name: GetAvoidAreaByTypeIgnoringVisibility
 * @tc.desc: GetAvoidAreaByTypeIgnoringVisibility test
 * @tc.type: FUNC
 */
HWTEST_F(WindowSceneSessionImplTest, GetAvoidAreaByTypeIgnoringVisibility, TestSize.Level1)
{
    sptr<WindowOption> option = sptr<WindowOption>::MakeSptr();
    option->SetWindowMode(WindowMode::WINDOW_MODE_PIP);
    option->SetWindowName("GetAvoidAreaByType");
    sptr<WindowSceneSessionImpl> window = sptr<WindowSceneSessionImpl>::MakeSptr(option);

    window->property_->SetPersistentId(1);
    SessionInfo sessionInfo = { "CreateTestBundle", "CreateTestModule", "CreateTestAbility" };
    sptr<SessionMocker> session = sptr<SessionMocker>::MakeSptr(sessionInfo);
    AvoidArea avoidarea;
    EXPECT_EQ(WMError::WM_ERROR_INVALID_WINDOW,
        window->GetAvoidAreaByTypeIgnoringVisibility(AvoidAreaType::TYPE_CUTOUT, avoidarea));
    window->hostSession_ = session;
    EXPECT_EQ(WMError::WM_OK, window->GetAvoidAreaByTypeIgnoringVisibility(AvoidAreaType::TYPE_CUTOUT, avoidarea));
}

/**
 * @tc.name: Immersive
 * @tc.desc: Immersive01 test
 * @tc.type: FUNC
 */
HWTEST_F(WindowSceneSessionImplTest, Immersive, TestSize.Level1)
{
    sptr<WindowOption> option = sptr<WindowOption>::MakeSptr();
    option->SetWindowMode(WindowMode::WINDOW_MODE_PIP);
    option->SetWindowName("Immersive");
    sptr<WindowSceneSessionImpl> window = sptr<WindowSceneSessionImpl>::MakeSptr(option);

    ASSERT_EQ(WMError::WM_ERROR_INVALID_WINDOW, window->SetLayoutFullScreen(false));
    ASSERT_EQ(WMError::WM_ERROR_INVALID_WINDOW, window->SetFullScreen(false));
    SessionInfo sessionInfo = { "CreateTestBundle", "CreateTestModule", "CreateTestAbility" };
    sptr<SessionMocker> session = sptr<SessionMocker>::MakeSptr(sessionInfo);
    window->hostSession_ = session;

    ASSERT_EQ(WMError::WM_ERROR_INVALID_WINDOW, window->SetFullScreen(false));
    ASSERT_EQ(false, window->IsLayoutFullScreen());
    ASSERT_EQ(false, window->IsFullScreen());
}

/**
 * @tc.name: SystemBarProperty
 * @tc.desc: SystemBarProperty01 test
 * @tc.type: FUNC
 */
HWTEST_F(WindowSceneSessionImplTest, SystemBarProperty, TestSize.Level1)
{
    sptr<WindowOption> option = sptr<WindowOption>::MakeSptr();
    option->SetWindowName("SystemBarProperty");
    option->SetWindowMode(WindowMode::WINDOW_MODE_PIP);
    sptr<WindowSceneSessionImpl> window = sptr<WindowSceneSessionImpl>::MakeSptr(option);

    SystemBarProperty property = SystemBarProperty();
    ASSERT_EQ(WMError::WM_ERROR_INVALID_WINDOW,
              window->SetSystemBarProperty(WindowType::WINDOW_TYPE_STATUS_BAR, property));
}

/**
 * @tc.name: SystemBarProperty02
 * @tc.desc: SystemBarProperty02 test
 * @tc.type: FUNC
 */
HWTEST_F(WindowSceneSessionImplTest, SystemBarProperty02, TestSize.Level1)
{
    sptr<WindowOption> option = sptr<WindowOption>::MakeSptr();
    option->SetWindowName("SystemBarProperty02");
    sptr<WindowSceneSessionImpl> window = sptr<WindowSceneSessionImpl>::MakeSptr(option);

    window->state_ = WindowState::STATE_SHOWN;
    SessionInfo sessionInfo = { "CreateTestBundle", "CreateTestModule", "CreateTestAbility" };
    sptr<SessionMocker> session = sptr<SessionMocker>::MakeSptr(sessionInfo);
    window->property_->SetPersistentId(1);
    window->hostSession_ = session;

    SystemBarProperty property;
    ASSERT_EQ(WMError::WM_OK, window->SetSystemBarProperty(WindowType::WINDOW_TYPE_STATUS_BAR, property));
    ASSERT_FALSE(window->property_->GetSystemBarProperty()[WindowType::WINDOW_TYPE_STATUS_BAR].enableAnimation_);

    property.enableAnimation_ = false;
    ASSERT_EQ(WMError::WM_OK, window->SetSystemBarProperty(WindowType::WINDOW_TYPE_STATUS_BAR, property));
    ASSERT_FALSE(window->property_->GetSystemBarProperty()[WindowType::WINDOW_TYPE_STATUS_BAR].enableAnimation_);
}

/**
 * @tc.name: SystemBarProperty03
 * @tc.desc: SystemBarProperty03 test
 * @tc.type: FUNC
 */
HWTEST_F(WindowSceneSessionImplTest, SystemBarProperty03, TestSize.Level1)
{
    sptr<WindowOption> option = sptr<WindowOption>::MakeSptr();
    option->SetWindowName("SystemBarProperty03");
    sptr<WindowSceneSessionImpl> window = sptr<WindowSceneSessionImpl>::MakeSptr(option);

    window->state_ = WindowState::STATE_SHOWN;
    SessionInfo sessionInfo = { "CreateTestBundle", "CreateTestModule", "CreateTestAbility" };
    sptr<SessionMocker> session = sptr<SessionMocker>::MakeSptr(sessionInfo);
    window->property_->SetPersistentId(1);
    window->hostSession_ = session;

    SystemBarProperty property;
    property.enableAnimation_ = true;
    ASSERT_EQ(WMError::WM_OK, window->SetSystemBarProperty(WindowType::WINDOW_TYPE_STATUS_BAR, property));
    ASSERT_TRUE(window->property_->GetSystemBarProperty()[WindowType::WINDOW_TYPE_STATUS_BAR].enableAnimation_);
}

/**
 * @tc.name: SystemBarProperty04
 * @tc.desc: SystemBarProperty04 test
 * @tc.type: FUNC
 */
HWTEST_F(WindowSceneSessionImplTest, SystemBarProperty04, TestSize.Level1)
{
    sptr<WindowOption> option = sptr<WindowOption>::MakeSptr();
    option->SetWindowName("SystemBarProperty04");
    option->SetWindowMode(WindowMode::WINDOW_MODE_PIP);
    sptr<WindowSceneSessionImpl> window = sptr<WindowSceneSessionImpl>::MakeSptr(option);

    SystemBarProperty property;
    ASSERT_EQ(WMError::WM_ERROR_INVALID_WINDOW,
              window->SetSpecificBarProperty(WindowType::WINDOW_TYPE_STATUS_BAR, property));
}

/**
 * @tc.name: SystemBarProperty05
 * @tc.desc: SystemBarProperty05 test
 * @tc.type: FUNC
 */
HWTEST_F(WindowSceneSessionImplTest, SystemBarProperty05, TestSize.Level1)
{
    sptr<WindowOption> option = sptr<WindowOption>::MakeSptr();
    option->SetWindowName("SystemBarProperty05");
    sptr<WindowSceneSessionImpl> window = sptr<WindowSceneSessionImpl>::MakeSptr(option);

    window->state_ = WindowState::STATE_SHOWN;
    SessionInfo sessionInfo = { "CreateTestBundle", "CreateTestModule", "CreateTestAbility" };
    sptr<SessionMocker> session = sptr<SessionMocker>::MakeSptr(sessionInfo);
    window->property_->SetPersistentId(1);
    window->hostSession_ = session;

    SystemBarProperty property;
    ASSERT_EQ(WMError::WM_OK, window->SetSpecificBarProperty(WindowType::WINDOW_TYPE_STATUS_BAR, property));
    ASSERT_FALSE(window->property_->GetSystemBarProperty()[WindowType::WINDOW_TYPE_STATUS_BAR].enableAnimation_);

    property.enableAnimation_ = false;
    ASSERT_EQ(WMError::WM_OK, window->SetSpecificBarProperty(WindowType::WINDOW_TYPE_STATUS_BAR, property));
    ASSERT_FALSE(window->property_->GetSystemBarProperty()[WindowType::WINDOW_TYPE_STATUS_BAR].enableAnimation_);
}

/**
 * @tc.name: SystemBarProperty06
 * @tc.desc: SystemBarProperty06 test
 * @tc.type: FUNC
 */
HWTEST_F(WindowSceneSessionImplTest, SystemBarProperty06, TestSize.Level1)
{
    sptr<WindowOption> option = sptr<WindowOption>::MakeSptr();
    option->SetWindowName("SystemBarProperty06");
    sptr<WindowSceneSessionImpl> window = sptr<WindowSceneSessionImpl>::MakeSptr(option);

    window->state_ = WindowState::STATE_SHOWN;
    SessionInfo sessionInfo = { "CreateTestBundle", "CreateTestModule", "CreateTestAbility" };
    sptr<SessionMocker> session = sptr<SessionMocker>::MakeSptr(sessionInfo);
    window->property_->SetPersistentId(1);
    window->hostSession_ = session;

    SystemBarProperty property;
    property.enableAnimation_ = true;
    ASSERT_EQ(WMError::WM_OK, window->SetSpecificBarProperty(WindowType::WINDOW_TYPE_STATUS_BAR, property));
    ASSERT_TRUE(window->property_->GetSystemBarProperty()[WindowType::WINDOW_TYPE_STATUS_BAR].enableAnimation_);
}

/**
 * @tc.name: SystemBarProperty07
 * @tc.desc: SystemBarProperty07 test
 * @tc.type: FUNC
 */
HWTEST_F(WindowSceneSessionImplTest, SystemBarProperty07, TestSize.Level1)
{
    sptr<WindowOption> option = sptr<WindowOption>::MakeSptr();
    option->SetWindowName("SystemBarProperty07");
    sptr<WindowSceneSessionImpl> window = sptr<WindowSceneSessionImpl>::MakeSptr(option);

    window->state_ = WindowState::STATE_SHOWN;
    SessionInfo sessionInfo = { "CreateTestBundle", "CreateTestModule", "CreateTestAbility" };
    sptr<SessionMocker> session = sptr<SessionMocker>::MakeSptr(sessionInfo);
    window->property_->SetPersistentId(1);
    ASSERT_EQ(WMError::WM_OK, window->Create(abilityContext_, session));
    window->hostSession_ = session;

    SystemBarProperty property;
    ASSERT_EQ(WMError::WM_OK, window->SetSystemBarProperty(WindowType::WINDOW_TYPE_STATUS_BAR, property));
    ASSERT_EQ(SystemBarSettingFlag::DEFAULT_SETTING,
              window->GetSystemBarPropertyByType(WindowType::WINDOW_TYPE_STATUS_BAR).settingFlag_);

    property.enable_ = false;
    property.settingFlag_ = SystemBarSettingFlag::ENABLE_SETTING;
    ASSERT_EQ(WMError::WM_OK, window->SetSystemBarProperty(WindowType::WINDOW_TYPE_STATUS_BAR, property));
    ASSERT_EQ(SystemBarSettingFlag::ENABLE_SETTING,
              window->GetSystemBarPropertyByType(WindowType::WINDOW_TYPE_STATUS_BAR).settingFlag_);

    property.backgroundColor_ = 0xB3000000;
    property.settingFlag_ = SystemBarSettingFlag::COLOR_SETTING;
    ASSERT_EQ(WMError::WM_OK, window->SetSystemBarProperty(WindowType::WINDOW_TYPE_STATUS_BAR, property));
    ASSERT_EQ(SystemBarSettingFlag::COLOR_SETTING,
              window->GetSystemBarPropertyByType(WindowType::WINDOW_TYPE_STATUS_BAR).settingFlag_);

    property.enable_ = true;
    property.backgroundColor_ = 0x4C000000;
    property.settingFlag_ = SystemBarSettingFlag::ALL_SETTING;
    ASSERT_EQ(WMError::WM_OK, window->SetSystemBarProperty(WindowType::WINDOW_TYPE_STATUS_BAR, property));
    ASSERT_EQ(WMError::WM_OK, window->Destroy(true));
}

/**
 * @tc.name: SpecificBarProperty
 * @tc.desc: SpecificBarProperty01 test
 * @tc.type: FUNC
 */
HWTEST_F(WindowSceneSessionImplTest, SpecificBarProperty, TestSize.Level1)
{
    sptr<WindowOption> option = sptr<WindowOption>::MakeSptr();
    option->SetWindowName("SpecificBarProperty");
    option->SetWindowMode(WindowMode::WINDOW_MODE_PIP);
    sptr<WindowSceneSessionImpl> window = sptr<WindowSceneSessionImpl>::MakeSptr(option);

    SystemBarProperty property = SystemBarProperty();
    ASSERT_EQ(WMError::WM_ERROR_INVALID_WINDOW,
              window->SetSpecificBarProperty(WindowType::WINDOW_TYPE_STATUS_BAR, property));
}

/**
 * @tc.name: NotifySpecificWindowSessionProperty
 * @tc.desc: NotifySpecificWindowSessionProperty01 test
 * @tc.type: FUNC
 */
HWTEST_F(WindowSceneSessionImplTest, NotifySpecificWindowSessionProperty, TestSize.Level1)
{
    sptr<WindowOption> option = sptr<WindowOption>::MakeSptr();
    option->SetWindowMode(WindowMode::WINDOW_MODE_PIP);
    option->SetWindowName("NotifySpecificWindowSessionProperty");
    sptr<WindowSceneSessionImpl> window = sptr<WindowSceneSessionImpl>::MakeSptr(option);

    SystemBarProperty property = SystemBarProperty();
    ASSERT_EQ(WMError::WM_ERROR_INVALID_WINDOW,
              window->NotifySpecificWindowSessionProperty(WindowType::WINDOW_TYPE_STATUS_BAR, property));
    window->property_->SetPersistentId(190);
    SessionInfo sessionInfo = { "CreateTestBundle", "CreateTestModule", "CreateTestAbility" };
    sptr<SessionMocker> session = sptr<SessionMocker>::MakeSptr(sessionInfo);
    ASSERT_NE(nullptr, session);
    window->hostSession_ = session;
    window->state_ = WindowState::STATE_HIDDEN;
    ASSERT_EQ(WMError::WM_OK,
              window->NotifySpecificWindowSessionProperty(WindowType::WINDOW_TYPE_NAVIGATION_BAR, property));
    window->state_ = WindowState::STATE_SHOWN;
    ASSERT_EQ(WMError::WM_OK,
              window->NotifySpecificWindowSessionProperty(WindowType::WINDOW_TYPE_NAVIGATION_INDICATOR, property));
}

/**
 * @tc.name: LimitCameraFloatWindowMininumSize
 * @tc.desc: LimitCameraFloatWindowMininumSize01 test
 * @tc.type: FUNC
 */
HWTEST_F(WindowSceneSessionImplTest, LimitCameraFloatWindowMininumSize, TestSize.Level1)
{
    sptr<WindowOption> option = sptr<WindowOption>::MakeSptr();
    ASSERT_NE(option, nullptr);
    option->SetWindowMode(WindowMode::WINDOW_MODE_PIP);
    option->SetWindowName("LimitCameraFloatWindowMininumSize");
    sptr<WindowSceneSessionImpl> window = sptr<WindowSceneSessionImpl>::MakeSptr(option);
    ASSERT_NE(window, nullptr);
    uint32_t width = 33;
    uint32_t height = 31;
    float vpr = 0.0f;
    window->LimitCameraFloatWindowMininumSize(width, height, vpr);
}

/**
 * @tc.name: NotifyWindowNeedAvoid
 * @tc.desc: NotifyWindowNeedAvoid test
 * @tc.type: FUNC
 */
HWTEST_F(WindowSceneSessionImplTest, NotifyWindowNeedAvoid, TestSize.Level1)
{
    sptr<WindowOption> option = sptr<WindowOption>::MakeSptr();
    option->SetWindowName("NotifyWindowNeedAvoid");
    sptr<WindowSceneSessionImpl> window = sptr<WindowSceneSessionImpl>::MakeSptr(option);
    ASSERT_EQ(WMError::WM_ERROR_INVALID_WINDOW, window->NotifyWindowNeedAvoid(false));

    window->state_ = WindowState::STATE_SHOWN;
    SessionInfo sessionInfo = { "CreateTestBundle", "CreateTestModule", "CreateTestAbility" };
    sptr<SessionMocker> session = sptr<SessionMocker>::MakeSptr(sessionInfo);
    window->property_->SetPersistentId(190);
    window->hostSession_ = session;
    ASSERT_EQ(WMError::WM_OK, window->NotifyWindowNeedAvoid(false));
}

/**
 * @tc.name: SetLayoutFullScreenByApiVersion
 * @tc.desc: SetLayoutFullScreenByApiVersion test
 * @tc.type: FUNC
 */
HWTEST_F(WindowSceneSessionImplTest, SetLayoutFullScreenByApiVersion, TestSize.Level0)
{
    sptr<WindowOption> option = sptr<WindowOption>::MakeSptr();
    option->SetWindowName("SetLayoutFullScreenByApiVersion");
    sptr<WindowSceneSessionImpl> window = sptr<WindowSceneSessionImpl>::MakeSptr(option);
    ASSERT_EQ(WMError::WM_ERROR_INVALID_WINDOW, window->SetLayoutFullScreenByApiVersion(false));
    window->state_ = WindowState::STATE_SHOWN;
    window->property_->SetPersistentId(190);
    SessionInfo sessionInfo = { "CreateTestBundle", "CreateTestModule", "CreateTestAbility" };
    sptr<SessionMocker> session = sptr<SessionMocker>::MakeSptr(sessionInfo);
    window->hostSession_ = session;
    ASSERT_EQ(WMError::WM_OK, window->SetLayoutFullScreenByApiVersion(false));
    ASSERT_EQ(false, window->property_->IsLayoutFullScreen());
    ASSERT_EQ(WMError::WM_OK, window->SetLayoutFullScreenByApiVersion(true));
    ASSERT_EQ(false, window->maximizeLayoutFullScreen_);
}

/**
 * @tc.name: SetIgnoreSafeArea
 * @tc.desc: SetIgnoreSafeArea test
 * @tc.type: FUNC
 */
HWTEST_F(WindowSceneSessionImplTest, SetIgnoreSafeArea, TestSize.Level0)
{
    sptr<WindowOption> option = sptr<WindowOption>::MakeSptr();
    option->SetWindowName("SetIgnoreSafeArea");
    sptr<WindowSceneSessionImpl> window = sptr<WindowSceneSessionImpl>::MakeSptr(option);
    EXPECT_EQ(WMError::WM_ERROR_INVALID_WINDOW, window->SetIgnoreSafeArea(false));
    SessionInfo sessionInfo = { "CreateTestBundle", "CreateTestModule", "CreateTestAbility" };
    sptr<SessionMocker> session = sptr<SessionMocker>::MakeSptr(sessionInfo);
    window->hostSession_ = session;
    window->state_ = WindowState::STATE_SHOWN;
    window->property_->SetPersistentId(1);
    EXPECT_EQ(WMError::WM_OK, window->SetIgnoreSafeArea(false));
    EXPECT_FALSE(window->isIgnoreSafeArea_);
    EXPECT_EQ(WMError::WM_OK, window->SetIgnoreSafeArea(true));
    EXPECT_TRUE(window->isIgnoreSafeArea_);
}

/**
 * @tc.name: SetGlobalMaximizeMode
 * @tc.desc: SetGlobalMaximizeMode test
 * @tc.type: FUNC
 */
HWTEST_F(WindowSceneSessionImplTest, SetGlobalMaximizeMode, TestSize.Level1)
{
    sptr<WindowOption> option = sptr<WindowOption>::MakeSptr();
    option->SetWindowName("SetGlobalMaximizeMode");
    sptr<WindowSceneSessionImpl> window = sptr<WindowSceneSessionImpl>::MakeSptr(option);
    ASSERT_EQ(WMError::WM_ERROR_INVALID_WINDOW, window->SetGlobalMaximizeMode(MaximizeMode::MODE_RECOVER));

    window->state_ = WindowState::STATE_SHOWN;
    SessionInfo sessionInfo = { "CreateTestBundle", "CreateTestModule", "CreateTestAbility" };
    sptr<SessionMocker> session = sptr<SessionMocker>::MakeSptr(sessionInfo);
    window->property_->SetPersistentId(190);
    window->hostSession_ = session;
    window->property_->SetWindowType(WindowType::APP_SUB_WINDOW_BASE);
    ASSERT_EQ(WMError::WM_ERROR_INVALID_PARAM, window->SetGlobalMaximizeMode(MaximizeMode::MODE_RECOVER));

    window->property_->SetWindowType(WindowType::APP_MAIN_WINDOW_BASE);
    ASSERT_EQ(WMError::WM_OK, window->SetGlobalMaximizeMode(MaximizeMode::MODE_RECOVER));
}

/**
 * @tc.name: CheckParmAndPermission
 * @tc.desc: CheckParmAndPermission test
 * @tc.type: FUNC
 */
HWTEST_F(WindowSceneSessionImplTest, CheckParmAndPermission, TestSize.Level1)
{
    sptr<WindowOption> option = sptr<WindowOption>::MakeSptr();
    sptr<WindowSceneSessionImpl> window = sptr<WindowSceneSessionImpl>::MakeSptr(option);
    window->property_->SetWindowName("CheckParmAndPermission");
    window->property_->SetWindowType(WindowType::SYSTEM_SUB_WINDOW_BASE);

    auto surfaceNode = window->GetSurfaceNode();
    if (surfaceNode == nullptr) {
        ASSERT_EQ(WMError::WM_ERROR_NULLPTR, window->CheckParmAndPermission());
    } else {
        ASSERT_EQ(WMError::WM_OK, window->CheckParmAndPermission());
        window->property_->SetWindowType(WindowType::WINDOW_TYPE_APP_SUB_WINDOW);
        ASSERT_EQ(WMError::WM_OK, window->CheckParmAndPermission());
    }
}

/**
 * @tc.name: SetTurnScreenOn
 * @tc.desc: SetTurnScreenOn test
 * @tc.type: FUNC
 */
HWTEST_F(WindowSceneSessionImplTest, SetTurnScreenOn, TestSize.Level1)
{
    sptr<WindowOption> option = sptr<WindowOption>::MakeSptr();
    sptr<WindowSceneSessionImpl> window = sptr<WindowSceneSessionImpl>::MakeSptr(option);
    window->property_->SetWindowName("SetTurnScreenOn");
    window->property_->SetWindowType(WindowType::SYSTEM_SUB_WINDOW_BASE);
    ASSERT_EQ(WMError::WM_ERROR_INVALID_WINDOW, window->SetTurnScreenOn(false));

    window->property_->SetPersistentId(1);
    SessionInfo sessionInfo = { "CreateTestBundle", "CreateTestModule", "CreateTestAbility" };
    sptr<SessionMocker> session = sptr<SessionMocker>::MakeSptr(sessionInfo);
    window->hostSession_ = session;
    ASSERT_EQ(WMError::WM_OK, window->SetTurnScreenOn(false));
}

/**
 * @tc.name: SetKeepScreenOn01
 * @tc.desc: Window is Invalid
 * @tc.type: FUNC
 */
HWTEST_F(WindowSceneSessionImplTest, SetKeepScreenOn01, TestSize.Level1)
{
    sptr<WindowOption> option = sptr<WindowOption>::MakeSptr();
    sptr<WindowSceneSessionImpl> window = sptr<WindowSceneSessionImpl>::MakeSptr(option);

    window->property_->SetWindowName("SetKeepScreenOn");
    window->property_->SetWindowType(WindowType::SYSTEM_SUB_WINDOW_BASE);
    ASSERT_EQ(WMError::WM_ERROR_INVALID_WINDOW, window->SetKeepScreenOn(false));
    ASSERT_EQ(WMError::WM_ERROR_INVALID_WINDOW, window->SetKeepScreenOn(true));
}

/**
 * @tc.name: SetKeepScreenOn02
 * @tc.desc: Window is Valid
 * @tc.type: FUNC
 */
HWTEST_F(WindowSceneSessionImplTest, SetKeepScreenOn02, TestSize.Level1)
{
    sptr<WindowOption> option = sptr<WindowOption>::MakeSptr();
    sptr<WindowSceneSessionImpl> window = sptr<WindowSceneSessionImpl>::MakeSptr(option);

    window->property_->SetWindowName("SetKeepScreenOn");
    window->property_->SetWindowType(WindowType::SYSTEM_SUB_WINDOW_BASE);
    window->property_->SetPersistentId(1);
    SessionInfo sessionInfo = { "CreateTestBundle", "CreateTestModule", "CreateTestAbility" };
    sptr<SessionMocker> session = sptr<SessionMocker>::MakeSptr(sessionInfo);
    window->hostSession_ = session;
    ASSERT_EQ(WMError::WM_OK, window->SetKeepScreenOn(true));
    ASSERT_TRUE(window->IsKeepScreenOn());
    ASSERT_EQ(WMError::WM_OK, window->SetKeepScreenOn(false));
    ASSERT_FALSE(window->IsKeepScreenOn());
}

/**
 * @tc.name: SetViewKeepScreenOn01
 * @tc.desc: Window is Invalid
 * @tc.type: FUNC
 */
HWTEST_F(WindowSceneSessionImplTest, SetViewKeepScreenOn01, TestSize.Level1)
{
    sptr<WindowOption> option = sptr<WindowOption>::MakeSptr();
    sptr<WindowSceneSessionImpl> window = sptr<WindowSceneSessionImpl>::MakeSptr(option);

    window->property_->SetWindowName("SetViewKeepScreenOn");
    window->property_->SetWindowType(WindowType::SYSTEM_SUB_WINDOW_BASE);
    ASSERT_EQ(WMError::WM_ERROR_INVALID_WINDOW, window->SetViewKeepScreenOn(false));
    ASSERT_EQ(WMError::WM_ERROR_INVALID_WINDOW, window->SetViewKeepScreenOn(true));
}

/**
 * @tc.name: SetViewKeepScreenOn02
 * @tc.desc: Window is Valid
 * @tc.type: FUNC
 */
HWTEST_F(WindowSceneSessionImplTest, SetViewKeepScreenOn02, TestSize.Level1)
{
    sptr<WindowOption> option = sptr<WindowOption>::MakeSptr();
    sptr<WindowSceneSessionImpl> window = sptr<WindowSceneSessionImpl>::MakeSptr(option);

    window->property_->SetWindowName("SetViewKeepScreenOn");
    window->property_->SetWindowType(WindowType::SYSTEM_SUB_WINDOW_BASE);
    window->property_->SetPersistentId(1);
    SessionInfo sessionInfo = { "CreateTestBundle", "CreateTestModule", "CreateTestAbility" };
    sptr<SessionMocker> session = sptr<SessionMocker>::MakeSptr(sessionInfo);
    window->hostSession_ = session;
    ASSERT_EQ(WMError::WM_OK, window->SetViewKeepScreenOn(true));
    ASSERT_TRUE(window->IsViewKeepScreenOn());
    ASSERT_EQ(WMError::WM_OK, window->SetViewKeepScreenOn(false));
    ASSERT_FALSE(window->IsViewKeepScreenOn());
}

/**
 * @tc.name: SetWindowShadowEnabled01
 * @tc.desc: Window is Invalid
 * @tc.type: FUNC
 */
HWTEST_F(WindowSceneSessionImplTest, SetWindowShadowEnabled01, TestSize.Level1)
{
    sptr<WindowOption> option = sptr<WindowOption>::MakeSptr();
    option->SetBundleName("SetWindowShadowEnabled");
    option->SetWindowName("SetWindowShadowEnabled");
    sptr<WindowSceneSessionImpl> window = sptr<WindowSceneSessionImpl>::MakeSptr(option);
    ASSERT_NE(nullptr, window);

    window->property_->SetWindowType(WindowType::APP_SUB_WINDOW_BASE);
    window->property_->SetPersistentId(1);
    window->hostSession_ = nullptr;
    window->windowSystemConfig_.windowUIType_ = WindowUIType::PHONE_WINDOW;
    EXPECT_EQ(WMError::WM_ERROR_DEVICE_NOT_SUPPORT, window->SetWindowShadowEnabled(false));
    EXPECT_EQ(WMError::WM_ERROR_DEVICE_NOT_SUPPORT, window->SetWindowShadowEnabled(true));

    window->windowSystemConfig_.windowUIType_ = WindowUIType::PC_WINDOW;
    EXPECT_EQ(WMError::WM_ERROR_INVALID_WINDOW, window->SetWindowShadowEnabled(false));
    EXPECT_EQ(WMError::WM_ERROR_INVALID_WINDOW, window->SetWindowShadowEnabled(true));

    window->windowSystemConfig_.windowUIType_ = WindowUIType::PAD_WINDOW;
    EXPECT_EQ(WMError::WM_ERROR_INVALID_WINDOW, window->SetWindowShadowEnabled(false));
    EXPECT_EQ(WMError::WM_ERROR_INVALID_WINDOW, window->SetWindowShadowEnabled(true));

    SessionInfo sessionInfo = { "CreateTestBundle", "CreateTestModule", "CreateTestAbility" };
    sptr<SessionMocker> session = sptr<SessionMocker>::MakeSptr(sessionInfo);
    ASSERT_NE(nullptr, session);
    window->hostSession_ = session;
    EXPECT_EQ(WMError::WM_OK, window->SetWindowShadowEnabled(true));
    EXPECT_EQ(true, window->GetWindowShadowEnabled());
    EXPECT_EQ(WMError::WM_OK, window->SetWindowShadowEnabled(false));
    EXPECT_EQ(false, window->GetWindowShadowEnabled());

    window->windowSystemConfig_.windowUIType_ = WindowUIType::PAD_WINDOW;
    window->property_->SetPcAppInpadCompatibleMode(true);
    EXPECT_EQ(WMError::WM_OK, window->SetWindowShadowEnabled(true));
}

/**
 * @tc.name: SetPrivacyMode01
 * @tc.desc: SetPrivacyMode as true
 * @tc.type: FUNC
 */
HWTEST_F(WindowSceneSessionImplTest, SetPrivacyMode01, TestSize.Level1)
{
    sptr<WindowOption> option = sptr<WindowOption>::MakeSptr();
    sptr<WindowSceneSessionImpl> window = sptr<WindowSceneSessionImpl>::MakeSptr(option);

    window->property_->SetWindowName("SetPrivacyMode");
    window->property_->SetWindowType(WindowType::SYSTEM_SUB_WINDOW_BASE);
    window->property_->SetPersistentId(1);
    SessionInfo sessionInfo = { "CreateTestBundle", "CreateTestModule", "CreateTestAbility" };
    sptr<SessionMocker> session = sptr<SessionMocker>::MakeSptr(sessionInfo);
    window->hostSession_ = session;
    ASSERT_EQ(WMError::WM_OK, window->SetPrivacyMode(true));
    ASSERT_EQ(true, window->IsPrivacyMode());
}

/**
 * @tc.name: SetPrivacyMode02
 * @tc.desc: SetPrivacyMode as false
 * @tc.type: FUNC
 */
HWTEST_F(WindowSceneSessionImplTest, SetPrivacyMode02, TestSize.Level1)
{
    sptr<WindowOption> option = sptr<WindowOption>::MakeSptr();
    sptr<WindowSceneSessionImpl> window = sptr<WindowSceneSessionImpl>::MakeSptr(option);

    window->property_->SetWindowName("SetPrivacyMode");
    window->property_->SetWindowType(WindowType::SYSTEM_SUB_WINDOW_BASE);
    window->property_->SetPersistentId(1);
    SessionInfo sessionInfo = { "CreateTestBundle", "CreateTestModule", "CreateTestAbility" };
    sptr<SessionMocker> session = sptr<SessionMocker>::MakeSptr(sessionInfo);
    window->hostSession_ = session;
    ASSERT_EQ(WMError::WM_OK, window->SetPrivacyMode(false));
    ASSERT_EQ(false, window->IsPrivacyMode());
}

/**
 * @tc.name: SetPrivacyMode03
 * @tc.desc: Window is invalid
 * @tc.type: FUNC
 */
HWTEST_F(WindowSceneSessionImplTest, SetPrivacyMode03, TestSize.Level1)
{
    sptr<WindowOption> option = sptr<WindowOption>::MakeSptr();
    sptr<WindowSceneSessionImpl> window = sptr<WindowSceneSessionImpl>::MakeSptr(option);

    window->property_->SetWindowName("SetPrivacyMode");
    window->property_->SetWindowType(WindowType::SYSTEM_SUB_WINDOW_BASE);
    ASSERT_EQ(WMError::WM_ERROR_INVALID_WINDOW, window->SetPrivacyMode(false));
}

/**
 * @tc.name: IsPrivacyMode
 * @tc.desc: Set window privacy mode as true and false
 * @tc.type: FUNC
 */
HWTEST_F(WindowSceneSessionImplTest, IsPrivacyModec, TestSize.Level1)
{
    sptr<WindowOption> option = sptr<WindowOption>::MakeSptr();
    sptr<WindowSceneSessionImpl> window = sptr<WindowSceneSessionImpl>::MakeSptr(option);

    window->property_->SetWindowName("IsPrivacyModec");
    window->property_->SetWindowType(WindowType::SYSTEM_SUB_WINDOW_BASE);
    window->property_->SetPersistentId(1);
    SessionInfo sessionInfo = { "CreateTestBundle", "CreateTestModule", "CreateTestAbility" };
    sptr<SessionMocker> session = sptr<SessionMocker>::MakeSptr(sessionInfo);
    window->hostSession_ = session;
    window->SetPrivacyMode(true);
    ASSERT_EQ(true, window->IsPrivacyMode());
    window->SetPrivacyMode(false);
    ASSERT_EQ(false, window->IsPrivacyMode());
}

/**
 * @tc.name: SetSystemPrivacyMode
 * @tc.desc: Set Ststemwindow privacy mode as true and false
 * @tc.type: FUNC
 */
HWTEST_F(WindowSceneSessionImplTest, SetSystemPrivacyMode, TestSize.Level1)
{
    sptr<WindowOption> option = sptr<WindowOption>::MakeSptr();
    sptr<WindowSceneSessionImpl> window = sptr<WindowSceneSessionImpl>::MakeSptr(option);

    window->property_->SetWindowName("SetSystemPrivacyMode");
    window->property_->SetWindowType(WindowType::SYSTEM_SUB_WINDOW_BASE);
    SessionInfo sessionInfo = { "CreateTestBundle", "CreateTestModule", "CreateTestAbility" };
    sptr<SessionMocker> session = sptr<SessionMocker>::MakeSptr(sessionInfo);
    window->hostSession_ = session;
    window->SetSystemPrivacyMode(true);
    ASSERT_EQ(true, window->property_->GetSystemPrivacyMode());
    window->SetSystemPrivacyMode(false);
    ASSERT_EQ(false, window->property_->GetSystemPrivacyMode());
}

/**
 * @tc.name: SetSnapshotSkip
 * @tc.desc: SetSnapshotSkip test
 * @tc.type: FUNC
 */
HWTEST_F(WindowSceneSessionImplTest, SetSnapshotSkip, TestSize.Level1)
{
    sptr<WindowOption> option = sptr<WindowOption>::MakeSptr();
    sptr<WindowSceneSessionImpl> window = sptr<WindowSceneSessionImpl>::MakeSptr(option);
    window->property_->SetWindowName("SetSnapshotSkip");
    window->property_->SetWindowType(WindowType::ABOVE_APP_SYSTEM_WINDOW_BASE);
    window->property_->SetPersistentId(1);
    auto surfaceNode_mocker = CreateRSSurfaceNode();
    ASSERT_NE(nullptr, surfaceNode_mocker);

    window->surfaceNode_ = surfaceNode_mocker;
    auto surfaceNode = window->GetSurfaceNode();
    window->property_->SetSnapshotSkip(true);
    ASSERT_EQ(WMError::WM_ERROR_INVALID_WINDOW, window->SetSnapshotSkip(false));
    ASSERT_EQ(true, window->property_->GetSnapshotSkip());
}

/**
 * @tc.name: SetImmersiveModeEnabledState
 * @tc.desc: SetImmersiveModeEnabledState test
 * @tc.type: FUNC
 */
HWTEST_F(WindowSceneSessionImplTest, SetImmersiveModeEnabledState, TestSize.Level0)
{
    sptr<WindowOption> option = sptr<WindowOption>::MakeSptr();
    sptr<WindowSceneSessionImpl> window = sptr<WindowSceneSessionImpl>::MakeSptr(option);
    ASSERT_EQ(WMError::WM_ERROR_INVALID_WINDOW, window->SetImmersiveModeEnabledState(false));

    window->property_->SetPersistentId(1);
    SessionInfo sessionInfo = { "CreateTestBundle", "CreateTestModule", "CreateTestAbility" };
    sptr<SessionMocker> session = sptr<SessionMocker>::MakeSptr(sessionInfo);
    window->hostSession_ = session;
    window->property_->SetWindowName("SetImmersiveModeEnabledState");
    window->property_->SetWindowType(WindowType::WINDOW_TYPE_PIP);
    window->state_ = WindowState::STATE_CREATED;
    ASSERT_EQ(WMError::WM_ERROR_INVALID_WINDOW, window->SetImmersiveModeEnabledState(false));

    window->property_->SetWindowType(WindowType::WINDOW_TYPE_APP_MAIN_WINDOW);
    ASSERT_EQ(WMError::WM_OK, window->SetImmersiveModeEnabledState(true));
    ASSERT_EQ(true, window->GetImmersiveModeEnabledState());
    ASSERT_EQ(WMError::WM_OK, window->SetImmersiveModeEnabledState(false));
    ASSERT_EQ(false, window->GetImmersiveModeEnabledState());

    window->property_->SetWindowMode(WindowMode::WINDOW_MODE_FULLSCREEN);
    ASSERT_EQ(WMError::WM_OK, window->SetLayoutFullScreen(true));
    ASSERT_EQ(true, window->IsLayoutFullScreen());
    ASSERT_EQ(WMError::WM_OK, window->SetLayoutFullScreen(false));
    ASSERT_EQ(false, window->IsLayoutFullScreen());
    ASSERT_EQ(WMError::WM_OK, window->SetImmersiveModeEnabledState(true));
    ASSERT_EQ(true, window->IsLayoutFullScreen());
    ASSERT_EQ(WMError::WM_OK, window->SetImmersiveModeEnabledState(false));
    ASSERT_EQ(false, window->IsLayoutFullScreen());

    window->property_->SetWindowMode(WindowMode::WINDOW_MODE_FLOATING);
    ASSERT_EQ(WMError::WM_OK, window->SetImmersiveModeEnabledState(true));
    ASSERT_EQ(true, window->IsLayoutFullScreen());
    ASSERT_EQ(true, window->GetImmersiveModeEnabledState());
    ASSERT_EQ(WMError::WM_OK, window->MaximizeFloating());
    ASSERT_EQ(true, window->IsLayoutFullScreen());
}

/**
 * @tc.name: IsImmersiveLayout01
 * @tc.desc: IsImmersiveLayout test
 * @tc.type: FUNC
 */
HWTEST_F(WindowSceneSessionImplTest, IsImmersiveLayout01, TestSize.Level0)
{
    sptr<WindowOption> option = sptr<WindowOption>::MakeSptr();
    sptr<WindowSceneSessionImpl> window = sptr<WindowSceneSessionImpl>::MakeSptr(option);
    window->property_->SetPersistentId(1);
    SessionInfo sessionInfo = { "CreateTestBundle", "CreateTestModule", "CreateTestAbility" };
    sptr<SessionMocker> session = sptr<SessionMocker>::MakeSptr(sessionInfo);
    window->hostSession_ = session;
    window->isIgnoreSafeArea_ = true;
    window->state_ = WindowState::STATE_CREATED;

    bool isImmersiveLayout = false;
    EXPECT_EQ(WMError::WM_OK, window->IsImmersiveLayout(isImmersiveLayout));

    window->isIgnoreSafeArea_ = false;
    EXPECT_EQ(WMError::WM_OK, window->IsImmersiveLayout(isImmersiveLayout));

    window->state_ = WindowState::STATE_DESTROYED;
    EXPECT_EQ(WMError::WM_ERROR_INVALID_WINDOW, window->IsImmersiveLayout(isImmersiveLayout));
}

/**
 * @tc.name: SetLayoutFullScreen01
 * @tc.desc: SetLayoutFullScreen test
 * @tc.type: FUNC
 */
HWTEST_F(WindowSceneSessionImplTest, SetLayoutFullScreen01, TestSize.Level0)
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
    window->property_->SetWindowModeSupportType(0);
    ASSERT_EQ(WMError::WM_ERROR_INVALID_WINDOW, window->SetLayoutFullScreen(false));
}

/**
 * @tc.name: SetTitleAndDockHoverShown
 * @tc.desc: SetTitleAndDockHoverShown test
 * @tc.type: FUNC
 */
HWTEST_F(WindowSceneSessionImplTest, SetTitleAndDockHoverShown, TestSize.Level1)
{
    sptr<WindowOption> option = sptr<WindowOption>::MakeSptr();
    sptr<WindowSceneSessionImpl> window = sptr<WindowSceneSessionImpl>::MakeSptr(option);
    EXPECT_EQ(WMError::WM_ERROR_INVALID_WINDOW, window->SetTitleAndDockHoverShown(true, true));

    window->property_->SetPersistentId(1);
    window->property_->SetWindowType(WindowType::SYSTEM_SUB_WINDOW_BASE);
    SessionInfo sessionInfo = { "CreateTestBundle", "CreateTestModule", "CreateTestAbility" };
    sptr<SessionMocker> session = sptr<SessionMocker>::MakeSptr(sessionInfo);
    window->hostSession_ = session;
    EXPECT_EQ(WMError::WM_ERROR_INVALID_CALLING, window->SetTitleAndDockHoverShown(true, true));

    window->property_->SetWindowType(WindowType::APP_MAIN_WINDOW_BASE);
    window->windowSystemConfig_.windowUIType_ = WindowUIType::PHONE_WINDOW;
    EXPECT_EQ(WMError::WM_ERROR_DEVICE_NOT_SUPPORT, window->SetTitleAndDockHoverShown(true, true));

    window->windowSystemConfig_.windowUIType_ = WindowUIType::PC_WINDOW;
    window->property_->SetWindowType(WindowType::APP_SUB_WINDOW_BASE);
    EXPECT_EQ(WMError::WM_ERROR_INVALID_CALLING, window->SetTitleAndDockHoverShown(true, true));
    window->property_->SetWindowType(WindowType::APP_MAIN_WINDOW_BASE);
    EXPECT_EQ(WMError::WM_OK, window->SetTitleAndDockHoverShown(true, true));

    window->windowSystemConfig_.windowUIType_ = WindowUIType::PAD_WINDOW;
    window->property_->SetPcAppInpadCompatibleMode(true);
    window->property_->SetWindowType(WindowType::APP_MAIN_WINDOW_BASE);
    window->windowSystemConfig_.freeMultiWindowEnable_ = false;
    EXPECT_EQ(WMError::WM_OK, window->SetTitleAndDockHoverShown(true, true));
}

/**
 * @tc.name: NotifyCompatibleModePropertyChange
 * @tc.desc: NotifyCompatibleModePropertyChange
 * @tc.type: FUNC
 */
HWTEST_F(WindowSceneSessionImplTest, NotifyCompatibleModePropertyChange, TestSize.Level1)
{
    sptr<WindowOption> option = sptr<WindowOption>::MakeSptr();
    ASSERT_NE(nullptr, option);
    option->SetWindowName("HandleDownForCompatibleMode");
    sptr<WindowSceneSessionImpl> window = sptr<WindowSceneSessionImpl>::MakeSptr(option);
    ASSERT_NE(nullptr, window);

    window->hostSession_ = nullptr;
    window->property_->persistentId_ = INVALID_SESSION_ID;
    window->state_ = WindowState::STATE_DESTROYED;

    sptr<CompatibleModeProperty> compatibleModeProperty = sptr<CompatibleModeProperty>::MakeSptr();
    auto ret = window->NotifyCompatibleModePropertyChange(compatibleModeProperty);
    ASSERT_EQ(ret, WSError::WS_ERROR_INVALID_WINDOW);

    SessionInfo sessionInfo = {"CreateTestBundle", "CreateTestModule", "CreateTestAbility"};
    sptr<SessionMocker> session = sptr<SessionMocker>::MakeSptr(sessionInfo);
    window->hostSession_ = session;
    window->property_->persistentId_ = ROTATE_ANIMATION_DURATION;
    window->state_ = WindowState::STATE_CREATED;
    ret = window->NotifyCompatibleModePropertyChange(compatibleModeProperty);
    ASSERT_EQ(ret, WSError::WS_OK);
}

/**
 * @tc.name: SetLayoutFullScreen03
 * @tc.desc: SetLayoutFullScreen test
 * @tc.type: FUNC
 */
HWTEST_F(WindowSceneSessionImplTest, SetLayoutFullScreen03, TestSize.Level1)
{
    sptr<WindowOption> option = sptr<WindowOption>::MakeSptr();
    sptr<WindowSceneSessionImpl> window = sptr<WindowSceneSessionImpl>::MakeSptr(option);
    window->property_->SetWindowName("SetLayoutFullScreen03");
    window->property_->SetPersistentId(1);
    SessionInfo sessionInfo = { "CreateTestBundle", "CreateTestModule", "CreateTestAbility" };
    sptr<SessionMocker> session = sptr<SessionMocker>::MakeSptr(sessionInfo);
    window->hostSession_ = session;
    WMError res = window->SetLayoutFullScreen(false);
    window->property_->SetWindowType(WindowType::BELOW_APP_SYSTEM_WINDOW_BASE);
    ASSERT_EQ(WMError::WM_OK, res);
    ASSERT_EQ(false, window->IsLayoutFullScreen());
    ASSERT_EQ(WMError::WM_OK, window->SetLayoutFullScreen(true));
    ASSERT_EQ(true, window->IsLayoutFullScreen());
    window->property_->SetWindowModeSupportType(0);
    sptr<CompatibleModeProperty> compatibleModeProperty = sptr<CompatibleModeProperty>::MakeSptr();
    compatibleModeProperty->SetIsAdaptToImmersive(true);
    window->property_->SetCompatibleModeProperty(compatibleModeProperty);
    sptr<WindowSessionImpl> mainWindow = CreateWindow("mainWindow", WindowType::WINDOW_TYPE_APP_MAIN_WINDOW, 100);
    mainWindow->SetFreeMultiWindowMode(true);
    ASSERT_EQ(WMError::WM_OK, mainWindow->SetLayoutFullScreen(true));
}

/**
 * @tc.name: SetFullScreen
 * @tc.desc: SetFullScreen test
 * @tc.type: FUNC
 */
HWTEST_F(WindowSceneSessionImplTest, SetFullScreen, TestSize.Level0)
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
}

/**
 * @tc.name: SetGestureBackEnabled
 * @tc.desc: SetGestureBackEnabled test
 * @tc.type: FUNC
 */
HWTEST_F(WindowSceneSessionImplTest, SetGestureBackEnabled, TestSize.Level1)
{
    sptr<WindowOption> option = sptr<WindowOption>::MakeSptr();
    sptr<WindowSceneSessionImpl> window = sptr<WindowSceneSessionImpl>::MakeSptr(option);
    window->property_->SetPersistentId(1);
    SessionInfo sessionInfo = { "CreateTestBundle", "CreateTestModule", "CreateTestAbility" };
    sptr<SessionMocker> session = sptr<SessionMocker>::MakeSptr(sessionInfo);
    window->hostSession_ = session;
    window->property_->SetWindowName("SetGestureBackEnabled");
    window->windowSystemConfig_.windowUIType_ = WindowUIType::PC_WINDOW;
    ASSERT_EQ(WMError::WM_ERROR_DEVICE_NOT_SUPPORT, window->SetGestureBackEnabled(false));
    window->property_->compatibleModeProperty_ = sptr<CompatibleModeProperty>::MakeSptr();
    window->property_->compatibleModeProperty_->SetIsAdaptToCompatibleDevice(true);
    EXPECT_EQ(window->SetGestureBackEnabled(false), WMError::WM_OK);
    bool enabled = false;
    EXPECT_EQ(window->GetGestureBackEnabled(enabled), WMError::WM_OK);
    window->property_->SetWindowType(WindowType::WINDOW_TYPE_PIP);
    window->state_ = WindowState::STATE_CREATED;
    window->windowSystemConfig_.windowUIType_ = WindowUIType::PHONE_WINDOW;
    ASSERT_EQ(WMError::WM_ERROR_INVALID_PARAM, window->SetGestureBackEnabled(false));
    window->property_->SetWindowType(WindowType::WINDOW_TYPE_APP_MAIN_WINDOW);
    window->property_->SetWindowMode(WindowMode::WINDOW_MODE_FULLSCREEN);
    ASSERT_EQ(WMError::WM_OK, window->SetGestureBackEnabled(false));
    bool enable;
    ASSERT_EQ(WMError::WM_OK, window->GetGestureBackEnabled(enable));
    ASSERT_EQ(false, enable);
    ASSERT_EQ(WMError::WM_OK, window->SetGestureBackEnabled(true));
    ASSERT_EQ(WMError::WM_OK, window->GetGestureBackEnabled(enable));
    ASSERT_EQ(true, enable);
}

/**
 * @tc.name: PcAppInPadNormalClose
 * @tc.desc: PcAppInPadNormalClose test
 * @tc.type: FUNC
 */
HWTEST_F(WindowSceneSessionImplTest, PcAppInPadNormalClose, TestSize.Level1)
{
    sptr<WindowOption> option = sptr<WindowOption>::MakeSptr();
    option->SetWindowName("PcAppInPadNormalClose");
    sptr<WindowSceneSessionImpl> window = sptr<WindowSceneSessionImpl>::MakeSptr(option);
    SessionInfo sessionInfo = { "CreateTestBundle", "CreateTestModule", "CreateTestAbility" };
    sptr<SessionMocker> session = sptr<SessionMocker>::MakeSptr(sessionInfo);
    ASSERT_EQ(WSError::WS_ERROR_INVALID_WINDOW, window->PcAppInPadNormalClose());

    window->hostSession_ = session;
    window->property_->SetPersistentId(1);
    ASSERT_EQ(WSError::WS_ERROR_INVALID_WINDOW, window->PcAppInPadNormalClose());

    window->property_->SetIsPcAppInPad(true);
    ASSERT_EQ(WSError::WS_OK, window->PcAppInPadNormalClose());
}

/**
 * @tc.name: NotifyIsFullScreenInForceSplitMode
 * @tc.desc: NotifyIsFullScreenInForceSplitMode test
 * @tc.type: FUNC
 */
HWTEST_F(WindowSceneSessionImplTest, NotifyIsFullScreenInForceSplitMode, TestSize.Level3)
{
    sptr<WindowOption> option = sptr<WindowOption>::MakeSptr();
    option->SetWindowName("NotifyIsFullScreenInForceSplitMode");
    sptr<WindowSceneSessionImpl> window = sptr<WindowSceneSessionImpl>::MakeSptr(option);
    window->NotifyIsFullScreenInForceSplitMode(false);
    window->NotifyIsFullScreenInForceSplitMode(true);
    SessionInfo sessionInfo = { "CreateTestBundle", "CreateTestModule", "CreateTestAbility" };
    sptr<SessionMocker> session = sptr<SessionMocker>::MakeSptr(sessionInfo);
    window->hostSession_ = session;
    window->NotifyIsFullScreenInForceSplitMode(false);
    ASSERT_FALSE(window->isFullScreenInForceSplit_.load());
}

/**
 * @tc.name: SetPropertySessionInfo01
 * @tc.desc: SetPropertySessionInfo test
 * @tc.type: FUNC
 */
HWTEST_F(WindowSceneSessionImplTest, SetPropertySessionInfo01, TestSize.Level1)
{
    const std::string bundleName = "setPropertSessionInfoTest";
    sptr<WindowOption> option = sptr<WindowOption>::MakeSptr();
    sptr<WindowSceneSessionImpl> windowSceneSession = sptr<WindowSceneSessionImpl>::MakeSptr(option);
    windowSceneSession->property_->sessionInfo_.bundleName_ = bundleName;

    windowSceneSession->context_ = nullptr;
    windowSceneSession->CreateAndConnectSpecificSession();
    ASSERT_EQ(windowSceneSession->property_->sessionInfo_.bundleName_, bundleName);

    windowSceneSession->context_ = abilityContext_;
    windowSceneSession->property_->sessionInfo_.bundleName_ = bundleName;
    windowSceneSession->CreateAndConnectSpecificSession();
    ASSERT_EQ(windowSceneSession->property_->sessionInfo_.bundleName_, abilityContext_->GetBundleName());
}

/**
 * @tc.name: SetWindowDelayRaiseEnabled
 * @tc.desc: SetWindowDelayRaiseEnabled test
 * @tc.type: FUNC
 */
HWTEST_F(WindowSceneSessionImplTest, SetWindowDelayRaiseEnabled, TestSize.Level1)
{
    sptr<WindowOption> option = sptr<WindowOption>::MakeSptr();
    sptr<WindowSceneSessionImpl> window = sptr<WindowSceneSessionImpl>::MakeSptr(option);
    window->property_->SetPersistentId(1);
    SessionInfo sessionInfo = {"CreateTestBundle", "CreateTestModule", "CreateTestAbility"};
    sptr<SessionMocker> session = sptr<SessionMocker>::MakeSptr(sessionInfo);
    window->hostSession_ = session;
    window->property_->SetWindowName("SetWindowDelayRaiseEnabled");
    window->windowSystemConfig_.windowUIType_ = WindowUIType::PHONE_WINDOW;
    ASSERT_EQ(WMError::WM_ERROR_DEVICE_NOT_SUPPORT, window->SetWindowDelayRaiseEnabled(false));
    window->windowSystemConfig_.windowUIType_ = WindowUIType::PAD_WINDOW;
    window->windowSystemConfig_.freeMultiWindowEnable_ = true;
    window->windowSystemConfig_.freeMultiWindowSupport_ = true;
    window->uiContent_ = std::make_unique<Ace::UIContentMocker>();
    ASSERT_EQ(WMError::WM_OK, window->SetWindowDelayRaiseEnabled(false));
    window->windowSystemConfig_.freeMultiWindowEnable_ = false;
    window->windowSystemConfig_.freeMultiWindowSupport_ = false;
    ASSERT_EQ(WMError::WM_ERROR_DEVICE_NOT_SUPPORT, window->SetWindowDelayRaiseEnabled(false));
    window->windowSystemConfig_.windowUIType_ = WindowUIType::PC_WINDOW;
    window->property_->SetWindowType(WindowType::WINDOW_TYPE_TOAST);
    ASSERT_EQ(WMError::WM_ERROR_INVALID_TYPE, window->SetWindowDelayRaiseEnabled(false));
    window->property_->SetWindowType(WindowType::WINDOW_TYPE_APP_MAIN_WINDOW);
    ASSERT_EQ(WMError::WM_OK, window->SetWindowDelayRaiseEnabled(false));
    ASSERT_EQ(false, window->IsWindowDelayRaiseEnabled());
    ASSERT_EQ(WMError::WM_OK, window->SetWindowDelayRaiseEnabled(true));
    ASSERT_EQ(true, window->IsWindowDelayRaiseEnabled());
}

/**
 * @tc.name: SetFollowParentMultiScreenPolicy
 * @tc.desc: SetFollowParentMultiScreenPolicy test
 * @tc.type: FUNC
 */
HWTEST_F(WindowSceneSessionImplTest, SetFollowParentMultiScreenPolicy, Function | SmallTest | Level2)
{
    sptr<WindowOption> option = sptr<WindowOption>::MakeSptr();
    sptr<WindowSceneSessionImpl> window = sptr<WindowSceneSessionImpl>::MakeSptr(option);
    SessionInfo sessionInfo = {"CreateTestBundle", "CreateTestModule", "CreateTestAbility"};
    sptr<SessionMocker> session = sptr<SessionMocker>::MakeSptr(sessionInfo);
    window->hostSession_ = session;
    window->property_->SetWindowName("SetFollowParentMultiScreenPolicy");
    window->property_->SetPersistentId(0);

    // Case 1: Invalid window
    window->property_->SetPersistentId(1);
    window->property_->SetWindowType(WindowType::WINDOW_TYPE_APP_SUB_WINDOW);
    window->windowSystemConfig_.windowUIType_ = WindowUIType::INVALID_WINDOW;
    EXPECT_EQ(WMError::WM_ERROR_DEVICE_NOT_SUPPORT, window->SetFollowParentMultiScreenPolicy(true));
    EXPECT_EQ(WMError::WM_ERROR_DEVICE_NOT_SUPPORT, window->SetFollowParentMultiScreenPolicy(false));

    // Case 2: Phone window
    window->windowSystemConfig_.windowUIType_ = WindowUIType::PHONE_WINDOW;
    EXPECT_EQ(WMError::WM_OK, window->SetFollowParentMultiScreenPolicy(true));
    EXPECT_EQ(WMError::WM_OK, window->SetFollowParentMultiScreenPolicy(false));

    // Case 3: Pad mainwindow with freeMultiWindow mode disable
    window->windowSystemConfig_.windowUIType_ = WindowUIType::PAD_WINDOW;
    window->property_->SetWindowType(WindowType::WINDOW_TYPE_APP_MAIN_WINDOW);
    window->windowSystemConfig_.freeMultiWindowEnable_ = false;
    window->windowSystemConfig_.freeMultiWindowSupport_ = false;
    EXPECT_EQ(WMError::WM_ERROR_INVALID_CALLING, window->SetFollowParentMultiScreenPolicy(true));
    EXPECT_EQ(WMError::WM_ERROR_INVALID_CALLING, window->SetFollowParentMultiScreenPolicy(false));

    // Case 4: Pad mainwindow with freeMultiWindow mode enable
    window->windowSystemConfig_.freeMultiWindowEnable_ = true;
    window->windowSystemConfig_.freeMultiWindowSupport_ = true;
    EXPECT_EQ(WMError::WM_ERROR_INVALID_CALLING, window->SetFollowParentMultiScreenPolicy(true));
    EXPECT_EQ(WMError::WM_ERROR_INVALID_CALLING, window->SetFollowParentMultiScreenPolicy(false));

    // Case 5: Pad subwindow with freeMultiWindow mode enable
    window->property_->SetWindowType(WindowType::WINDOW_TYPE_APP_SUB_WINDOW);
    window->windowSystemConfig_.freeMultiWindowEnable_ = false;
    window->windowSystemConfig_.freeMultiWindowSupport_ = false;
    EXPECT_EQ(WMError::WM_OK, window->SetFollowParentMultiScreenPolicy(true));
    EXPECT_EQ(WMError::WM_OK, window->SetFollowParentMultiScreenPolicy(false));
    
    // Case 6: PC subwindow
    window->windowSystemConfig_.windowUIType_ = WindowUIType::PC_WINDOW;
    EXPECT_EQ(WMError::WM_OK, window->SetFollowParentMultiScreenPolicy(true));
    EXPECT_EQ(WMError::WM_OK, window->SetFollowParentMultiScreenPolicy(false));

    // Case 7: Pc app in pad
    window->windowSystemConfig_.windowUIType_ = WindowUIType::PAD_WINDOW;
    window->property_->SetPcAppInpadCompatibleMode(true);
    window->windowSystemConfig_.freeMultiWindowEnable_ = false;
    EXPECT_EQ(WMError::WM_OK, window->SetFollowParentMultiScreenPolicy(false));
}

/**
 * @tc.name: CloseSpecificScene
 * @tc.desc: CloseSpecificScene test
 * @tc.type: FUNC
 */
HWTEST_F(WindowSceneSessionImplTest, CloseSpecificScene, TestSize.Level1)
{
    sptr<WindowOption> option = sptr<WindowOption>::MakeSptr();
    option->SetWindowName("Close01");
    sptr<WindowSceneSessionImpl> windowSceneSession = sptr<WindowSceneSessionImpl>::MakeSptr(option);
    windowSceneSession->property_->SetPersistentId(1);
    windowSceneSession->property_->SetWindowType(WindowType::WINDOW_TYPE_APP_SUB_WINDOW);
    windowSceneSession->property_->SetDecorEnable(false);
    EXPECT_EQ(WSError::WS_ERROR_INVALID_OPERATION, windowSceneSession->CloseSpecificScene());
    windowSceneSession->property_->SetDecorEnable(true);
    SessionInfo sessionInfo = { "CreateTestBundle", "CreateTestModule", "CreateTestAbility" };
    sptr<SessionMocker> session = sptr<SessionMocker>::MakeSptr(sessionInfo);
    windowSceneSession->hostSession_ = session;
    sptr<IWindowWillCloseListener> listener = sptr<IWindowWillCloseListener>::MakeSptr();
    windowSceneSession->windowSystemConfig_.windowUIType_ = WindowUIType::PC_WINDOW;
    auto res = windowSceneSession->RegisterWindowWillCloseListeners(listener);
    EXPECT_EQ(WMError::WM_OK, res);
    EXPECT_EQ(WSError::WS_OK, windowSceneSession->CloseSpecificScene());
    res = windowSceneSession->UnRegisterWindowWillCloseListeners(listener);
    EXPECT_EQ(WMError::WM_OK, res);
    EXPECT_EQ(WSError::WS_OK, windowSceneSession->CloseSpecificScene());
}

/**
 * @tc.name: SetSubWindowSource
 * @tc.desc: SetSubWindowSource test 1
 * @tc.type: FUNC
 */
HWTEST_F(WindowSceneSessionImplTest, SetSubWindowSource01, TestSize.Level1)
{
    sptr<WindowOption> option = sptr<WindowOption>::MakeSptr();
    sptr<WindowSceneSessionImpl> window = sptr<WindowSceneSessionImpl>::MakeSptr(option);
    window->SetWindowType(WindowType::APP_MAIN_WINDOW_BASE);
    window->property_->SetPersistentId(1);
    SessionInfo sessionInfo = {"CreateTestBundle", "CreateTestModule", "CreateTestAbility"};
    sptr<SessionMocker> session = sptr<SessionMocker>::MakeSptr(sessionInfo);
    window->hostSession_ = session;
    window->property_->SetWindowName("SetSubWindowSource");
    auto res = window->SetSubWindowSource(SubWindowSource::SUB_WINDOW_SOURCE_ARKUI);
    EXPECT_EQ(WMError::WM_ERROR_INVALID_OPERATION, res);
    window->SetWindowType(WindowType::APP_SUB_WINDOW_BASE);
    res = window->SetSubWindowSource(SubWindowSource::SUB_WINDOW_SOURCE_ARKUI);
    EXPECT_EQ(WMError::WM_OK, res);
}

/**
 * @tc.name: SetSubWindowSource
 * @tc.desc: SetSubWindowSource test 2
 * @tc.type: FUNC
 */
HWTEST_F(WindowSceneSessionImplTest, SetSubWindowSource02, TestSize.Level1)
{
    sptr<WindowOption> option = sptr<WindowOption>::MakeSptr();
    sptr<WindowSceneSessionImpl> window = sptr<WindowSceneSessionImpl>::MakeSptr(option);
    window->SetWindowType(WindowType::APP_MAIN_WINDOW_BASE);
    window->property_->SetPersistentId(0);
    window->hostSession_ = nullptr;
    auto res = window->SetSubWindowSource(SubWindowSource::SUB_WINDOW_SOURCE_ARKUI);
    EXPECT_EQ(res, WMError::WM_ERROR_INVALID_WINDOW);
}

/**
 * @tc.name: RestoreMainWindow
 * @tc.desc: SetSubWindowSource test
 * @tc.type: FUNC
 */
HWTEST_F(WindowSceneSessionImplTest, RestoreMainWindow, TestSize.Level1)
{
    sptr<WindowOption> option = sptr<WindowOption>::MakeSptr();
    option->SetWindowName("RestoreMainWindow");
    sptr<WindowSceneSessionImpl> window = sptr<WindowSceneSessionImpl>::MakeSptr(option);
    window->property_->SetPersistentId(1);
    window->property_->SetParentPersistentId(-1);
    window->SetWindowType(WindowType::WINDOW_TYPE_MEDIA);
    SessionInfo sessionInfo = { "CreateTestBundle", "CreateTestModule", "CreateTestAbility" };
    sptr<SessionMocker> session = sptr<SessionMocker>::MakeSptr(sessionInfo);
    EXPECT_EQ(WMError::WM_OK, window->Create(abilityContext_, session));
    window->state_ = WindowState::STATE_CREATED;
    window->hostSession_ = session;
    {
        std::unique_lock<std::shared_mutex> lock(window->windowSessionMutex_);
        WindowSessionImpl::windowSessionMap_.insert(std::make_pair(window->property_->GetWindowName(),
            std::pair<uint64_t, sptr<WindowSessionImpl>>(window->property_->GetPersistentId(), window)));
    }
    
    std::shared_ptr<AAFwk::WantParams> wantParams = std::shared_ptr<AAFwk::WantParams>();
    WMError res = window->RestoreMainWindow(wantParams);
    EXPECT_EQ(res, WMError::WM_ERROR_INVALID_CALLING);
    window->SetWindowType(WindowType::WINDOW_TYPE_FLOAT);

    res = window->RestoreMainWindow(wantParams);
    EXPECT_EQ(res, WMError::WM_ERROR_INVALID_CALLING);

    sptr<WindowOption> parentOption = sptr<WindowOption>::MakeSptr();
    option->SetWindowName("RestoreParentMainWindow");
    sptr<WindowSceneSessionImpl> parentWindow = sptr<WindowSceneSessionImpl>::MakeSptr(parentOption);
    parentWindow->property_->SetPersistentId(100);
    window->property_->SetParentPersistentId(100);
    {
        std::unique_lock<std::shared_mutex> lock(window->windowSessionMutex_);
        WindowSessionImpl::windowSessionMap_.insert(std::make_pair(parentWindow->property_->GetWindowName(),
            std::pair<uint64_t, sptr<WindowSessionImpl>>(parentWindow->property_->GetPersistentId(), parentWindow)));
    }

    res = window->RestoreMainWindow(wantParams);
    EXPECT_EQ(res, WMError::WM_ERROR_INVALID_CALLING);
    window->state_ = WindowState::STATE_SHOWN;

    window->hostSession_ = nullptr;
    res = window->RestoreMainWindow(wantParams);
    EXPECT_EQ(res, WMError::WM_ERROR_INVALID_WINDOW);
    window->hostSession_ = session;

    res = window->RestoreMainWindow(wantParams);
    EXPECT_EQ(res, WMError::WM_OK);
}

/**
 * @tc.name: GetAndVerifyWindowTypeForArkUI01
 * @tc.desc: GetAndVerifyWindowTypeForArkUI01 test
 * @tc.type: FUNC
 */
HWTEST_F(WindowSceneSessionImplTest, GetAndVerifyWindowTypeForArkUI01, TestSize.Level1)
{
    sptr<WindowOption> option = sptr<WindowOption>::MakeSptr();
    option->SetWindowName("GetAndVerifyWindowTypeForArkUI");
    sptr<WindowSceneSessionImpl> windowSceneSession = sptr<WindowSceneSessionImpl>::MakeSptr(option);

    windowSceneSession->SetWindowType(WindowType::WINDOW_TYPE_SCENE_BOARD);
    windowSceneSession->property_->SetPersistentId(100);
    windowSceneSession->property_->SetParentPersistentId(99);
    windowSceneSession->property_->SetParentId(99);
    windowSceneSession->property_->SetWindowMode(WindowMode::WINDOW_MODE_FLOATING);
    SessionInfo sessionInfo = { "CreateTestBundle", "CreateTestModule", "CreateTestAbility" };
    sptr<SessionMocker> session = sptr<SessionMocker>::MakeSptr(sessionInfo);
    EXPECT_EQ(WMError::WM_OK, windowSceneSession->Create(abilityContext_, session));

    auto parentWindow = WindowSceneSessionImpl::GetWindowWithId(100);
    EXPECT_EQ(parentWindow != nullptr, true);

    std::string windowName ="GetAndVerifyWindowTypeForArkUIWindowName";
    WindowType windowType;
    WindowType parentWindowType = WindowType::WINDOW_TYPE_SCENE_BOARD;
    windowSceneSession->SetWindowType(WindowType::WINDOW_TYPE_SCENE_BOARD);
    auto ret = WindowSceneSessionImpl::GetAndVerifyWindowTypeForArkUI(100, "GetAndVerifyWindowTypeForArkUI",
        parentWindowType, windowType);
    EXPECT_EQ(WMError::WM_ERROR_REPEAT_OPERATION, ret);

    windowSceneSession->SetWindowType(WindowType::WINDOW_TYPE_SCENE_BOARD);
    ret = WindowSceneSessionImpl::GetAndVerifyWindowTypeForArkUI(100, windowName, parentWindowType, windowType);
    EXPECT_EQ(WMError::WM_OK, ret);
    EXPECT_EQ(windowType == WindowType::WINDOW_TYPE_SYSTEM_FLOAT, true);

    parentWindowType = WindowType::WINDOW_TYPE_DESKTOP;
    windowSceneSession->SetWindowType(WindowType::WINDOW_TYPE_DESKTOP);
    ret = WindowSceneSessionImpl::GetAndVerifyWindowTypeForArkUI(100, windowName, parentWindowType, windowType);
    EXPECT_EQ(WMError::WM_OK, ret);
    EXPECT_EQ(windowType == WindowType::WINDOW_TYPE_SYSTEM_FLOAT, true);

    parentWindowType = WindowType::WINDOW_TYPE_UI_EXTENSION;
    windowSceneSession->SetWindowType(WindowType::WINDOW_TYPE_UI_EXTENSION);
    ret = WindowSceneSessionImpl::GetAndVerifyWindowTypeForArkUI(100, windowName, parentWindowType, windowType);
    EXPECT_EQ(WMError::WM_OK, ret);
    EXPECT_EQ(windowType == WindowType::WINDOW_TYPE_APP_SUB_WINDOW, true);

    EXPECT_EQ(WMError::WM_OK, windowSceneSession->Destroy(true));
}

/**
 * @tc.name: GetAndVerifyWindowTypeForArkUI02
 * @tc.desc: GetAndVerifyWindowTypeForArkUI02 test
 * @tc.type: FUNC
 */
HWTEST_F(WindowSceneSessionImplTest, GetAndVerifyWindowTypeForArkUI02, TestSize.Level1)
{
    sptr<WindowOption> option = sptr<WindowOption>::MakeSptr();
    option->SetWindowName("GetAndVerifyWindowTypeForArkUI");
    sptr<WindowSceneSessionImpl> windowSceneSession = sptr<WindowSceneSessionImpl>::MakeSptr(option);

    windowSceneSession->SetWindowType(WindowType::WINDOW_TYPE_SCENE_BOARD);
    windowSceneSession->property_->SetPersistentId(101);
    windowSceneSession->property_->SetParentPersistentId(100);
    windowSceneSession->property_->SetParentId(100);
    windowSceneSession->property_->SetWindowMode(WindowMode::WINDOW_MODE_FLOATING);
    SessionInfo sessionInfo = { "CreateTestBundle", "CreateTestModule", "CreateTestAbility" };
    sptr<SessionMocker> session = sptr<SessionMocker>::MakeSptr(sessionInfo);
    EXPECT_EQ(WMError::WM_OK, windowSceneSession->Create(abilityContext_, session));

    auto parentWindow = WindowSceneSessionImpl::GetWindowWithId(101);
    EXPECT_EQ(parentWindow != nullptr, true);

    std::string windowName ="GetAndVerifyWindowTypeForArkUIWindowName";
    WindowType windowType;
    WindowType parentWindowType = WindowType::WINDOW_TYPE_SYSTEM_SUB_WINDOW;
    windowSceneSession->SetWindowType(WindowType::WINDOW_TYPE_SYSTEM_SUB_WINDOW);
    auto ret = WindowSceneSessionImpl::GetAndVerifyWindowTypeForArkUI(101, windowName, parentWindowType, windowType);
    EXPECT_EQ(WMError::WM_ERROR_INVALID_TYPE, ret);

    parentWindowType = WindowType::WINDOW_TYPE_FLOAT;
    windowSceneSession->SetWindowType(WindowType::WINDOW_TYPE_FLOAT);
    ret = WindowSceneSessionImpl::GetAndVerifyWindowTypeForArkUI(101, windowName, parentWindowType, windowType);
    EXPECT_EQ(WMError::WM_OK, ret);
    EXPECT_EQ(windowType == WindowType::WINDOW_TYPE_SYSTEM_SUB_WINDOW, true);

    parentWindowType = WindowType::WINDOW_TYPE_APP_SUB_WINDOW;
    windowSceneSession->SetWindowType(WindowType::WINDOW_TYPE_APP_SUB_WINDOW);
    ret = WindowSceneSessionImpl::GetAndVerifyWindowTypeForArkUI(102, windowName, parentWindowType, windowType);
    EXPECT_EQ(WMError::WM_ERROR_INVALID_WINDOW, ret);

    parentWindowType = WindowType::WINDOW_TYPE_APP_MAIN_WINDOW;
    windowSceneSession->SetWindowType(WindowType::WINDOW_TYPE_APP_MAIN_WINDOW);
    ret = WindowSceneSessionImpl::GetAndVerifyWindowTypeForArkUI(101, windowName, parentWindowType, windowType);
    EXPECT_EQ(WMError::WM_OK, ret);
    EXPECT_EQ(windowType == WindowType::WINDOW_TYPE_APP_SUB_WINDOW, true);

    EXPECT_EQ(WMError::WM_OK, windowSceneSession->Destroy(true));
}

/**
 * @tc.name: GetAndVerifyWindowTypeForArkUI03
 * @tc.desc: GetAndVerifyWindowTypeForArkUI03 test
 * @tc.type: FUNC
 */
HWTEST_F(WindowSceneSessionImplTest, GetAndVerifyWindowTypeForArkUI03, TestSize.Level1)
{
    sptr<WindowOption> option = sptr<WindowOption>::MakeSptr();
    option->SetWindowName("GetAndVerifyWindowTypeForArkUI");
    sptr<WindowSceneSessionImpl> windowSceneSession = sptr<WindowSceneSessionImpl>::MakeSptr(option);

    windowSceneSession->SetWindowType(WindowType::WINDOW_TYPE_SCENE_BOARD);
    windowSceneSession->property_->SetPersistentId(101);
    windowSceneSession->property_->SetParentPersistentId(100);
    windowSceneSession->property_->SetParentId(100);
    windowSceneSession->property_->SetWindowMode(WindowMode::WINDOW_MODE_FLOATING);
    SessionInfo sessionInfo = { "CreateTestBundle", "CreateTestModule", "CreateTestAbility" };
    sptr<SessionMocker> session = sptr<SessionMocker>::MakeSptr(sessionInfo);
    EXPECT_EQ(WMError::WM_OK, windowSceneSession->Create(abilityContext_, session));

    auto parentWindow = WindowSceneSessionImpl::GetWindowWithId(101);
    EXPECT_EQ(parentWindow != nullptr, true);

    std::string windowName ="GetAndVerifyWindowTypeForArkUIWindowName";
    WindowType windowType;
    WindowType parentWindowType = WindowType::WINDOW_TYPE_APP_SUB_WINDOW;

    windowSceneSession->windowSystemConfig_.windowUIType_ = WindowUIType::PHONE_WINDOW;
    windowSceneSession->SetWindowType(WindowType::WINDOW_TYPE_APP_SUB_WINDOW);
    windowSceneSession->property_->SetSubWindowLevel(1);
    auto ret = WindowSceneSessionImpl::GetAndVerifyWindowTypeForArkUI(101, windowName, parentWindowType, windowType);
    EXPECT_EQ(WMError::WM_ERROR_DEVICE_NOT_SUPPORT, ret);

    ret = WindowSceneSessionImpl::GetAndVerifyWindowTypeForArkUI(101, windowName,
        WindowType::WINDOW_TYPE_APP_MAIN_WINDOW, windowType);
    EXPECT_EQ(WMError::WM_ERROR_INVALID_WINDOW, ret);

    windowSceneSession->windowSystemConfig_.windowUIType_ = WindowUIType::PAD_WINDOW;
    ret = WindowSceneSessionImpl::GetAndVerifyWindowTypeForArkUI(101, windowName, parentWindowType, windowType);
    EXPECT_EQ(WMError::WM_ERROR_DEVICE_NOT_SUPPORT, ret);

    windowSceneSession->property_->SetIsUIExtFirstSubWindow(true);
    ret = WindowSceneSessionImpl::GetAndVerifyWindowTypeForArkUI(101, windowName, parentWindowType, windowType);
    EXPECT_EQ(WMError::WM_OK, ret);

    windowSceneSession->property_->SetIsUIExtFirstSubWindow(false);
    windowSceneSession->windowSystemConfig_.windowUIType_ = WindowUIType::PC_WINDOW;
    ret = WindowSceneSessionImpl::GetAndVerifyWindowTypeForArkUI(101, windowName, parentWindowType, windowType);
    EXPECT_EQ(WMError::WM_OK, ret);
    EXPECT_EQ(windowType, WindowType::WINDOW_TYPE_APP_SUB_WINDOW);

    EXPECT_EQ(WMError::WM_OK, windowSceneSession->Destroy(true));
}

/**
 * @tc.name: VerifySubWindowLevel
 * @tc.desc: VerifySubWindowLevel test
 * @tc.type: FUNC
 */
HWTEST_F(WindowSceneSessionImplTest, VerifySubWindowLevel, TestSize.Level1)
{
    sptr<WindowSessionImpl> windowSceneSession = nullptr;
    auto ret = WindowSceneSessionImpl::VerifySubWindowLevel(false, windowSceneSession);
    EXPECT_EQ(WMError::WM_ERROR_NULLPTR, ret);

    sptr<WindowOption> option = sptr<WindowOption>::MakeSptr();
    option->SetWindowName("VerifySubWindowLevel");
    windowSceneSession = sptr<WindowSessionImpl>::MakeSptr(option);
    ret = WindowSceneSessionImpl::VerifySubWindowLevel(true, windowSceneSession);
    EXPECT_EQ(WMError::WM_OK, ret);

    windowSceneSession->property_->SetIsUIExtFirstSubWindow(true);
    ret = WindowSceneSessionImpl::VerifySubWindowLevel(false, windowSceneSession);
    EXPECT_EQ(WMError::WM_OK, ret);

    windowSceneSession->property_->SetIsUIExtFirstSubWindow(false);
    windowSceneSession->property_->SetSubWindowLevel(1);
    windowSceneSession->windowSystemConfig_.windowUIType_ = WindowUIType::PAD_WINDOW;
    ret = WindowSceneSessionImpl::VerifySubWindowLevel(false, windowSceneSession);
    EXPECT_EQ(WMError::WM_ERROR_DEVICE_NOT_SUPPORT, ret);

    windowSceneSession->windowSystemConfig_.windowUIType_ = WindowUIType::PC_WINDOW;
    ret = WindowSceneSessionImpl::VerifySubWindowLevel(false, windowSceneSession);
    EXPECT_EQ(WMError::WM_OK, ret);
}
} // namespace
} // namespace Rosen
} // namespace OHOS
