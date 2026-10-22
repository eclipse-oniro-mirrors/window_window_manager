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
#include "display_info.h"
#include "ability_context_impl.h"
#include "mock_session.h"
#include "window_session_impl.h"
#include "mock_uicontent.h"
#include "window_scene_session_impl.h"
#include "mock_window_adapter.h"
#include "singleton_mocker.h"
#include "session/host/include/scene_session.h"

using namespace testing;
using namespace testing::ext;

namespace OHOS {
namespace Rosen {
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

class WindowSceneSessionImplSpecialWindowTest : public testing::Test {
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

void WindowSceneSessionImplSpecialWindowTest::SetUpTestCase() {}

void WindowSceneSessionImplSpecialWindowTest::TearDownTestCase() {}

void WindowSceneSessionImplSpecialWindowTest::SetUp()
{
    abilityContext_ = std::make_shared<AbilityRuntime::AbilityContextImpl>();
}

void WindowSceneSessionImplSpecialWindowTest::TearDown()
{
    usleep(WAIT_SYNC_IN_NS);
    abilityContext_ = nullptr;
}

RSSurfaceNode::SharedPtr WindowSceneSessionImplSpecialWindowTest::CreateRSSurfaceNode()
{
    struct RSSurfaceNodeConfig rsSurfaceNodeConfig;
    rsSurfaceNodeConfig.SurfaceNodeName = "startingWindowTestSurfaceNode";
    auto surfaceNode = RSSurfaceNode::Create(rsSurfaceNodeConfig, RSSurfaceNodeType::DEFAULT);
    return surfaceNode;
}

namespace {
/**
 * @tc.name: CreateAndConnectSpecificSession01
 * @tc.desc: CreateAndConnectSpecificSession
 * @tc.type: FUNC
 */
HWTEST_F(WindowSceneSessionImplSpecialWindowTest, CreateAndConnectSpecificSession01, TestSize.Level1)
{
    sptr<WindowOption> option = new (std::nothrow) WindowOption();
    ASSERT_NE(nullptr, option);
    option->SetWindowName("CreateAndConnectSpecificSession01");
    sptr<WindowSceneSessionImpl> windowSceneSession = new (std::nothrow) WindowSceneSessionImpl(option);
    ASSERT_NE(nullptr, windowSceneSession);

    windowSceneSession->SetWindowType(WindowType::APP_MAIN_WINDOW_BASE);
    ASSERT_EQ(WMError::WM_ERROR_NULLPTR, windowSceneSession->CreateAndConnectSpecificSession().errCode);
    windowSceneSession->property_->SetPersistentId(102);
    windowSceneSession->property_->SetParentPersistentId(100);
    windowSceneSession->property_->SetParentId(100);
    SessionInfo sessionInfo = {"CreateTestBundle", "CreateTestModule", "CreateTestAbility"};
    sptr<SessionMocker> session = new (std::nothrow) SessionMocker(sessionInfo);
    ASSERT_NE(nullptr, session);

    ASSERT_EQ(WMError::WM_OK, windowSceneSession->Create(abilityContext_, session));
    ASSERT_EQ(WMError::WM_OK, windowSceneSession->Destroy(true));
}

/**
 * @tc.name: CreateAndConnectSpecificSession02
 * @tc.desc: CreateAndConnectSpecificSession
 * @tc.type: FUNC
 */
HWTEST_F(WindowSceneSessionImplSpecialWindowTest, CreateAndConnectSpecificSession02, TestSize.Level1)
{
    sptr<WindowOption> option = new (std::nothrow) WindowOption();
    ASSERT_NE(nullptr, option);
    option->SetWindowTag(WindowTag::SUB_WINDOW);
    option->SetWindowName("CreateAndConnectSpecificSession02");
    sptr<WindowSceneSessionImpl> windowSceneSession = new (std::nothrow) WindowSceneSessionImpl(option);
    ASSERT_NE(nullptr, windowSceneSession);
    
    SessionInfo sessionInfo = { "CreateTestBundle0", "CreateTestModule0", "CreateTestAbility0" };
    sptr<SessionMocker> session = new (std::nothrow) SessionMocker(sessionInfo);
    ASSERT_NE(nullptr, session);
    ASSERT_EQ(WMError::WM_OK, windowSceneSession->Create(abilityContext_, session));
    windowSceneSession->property_->SetPersistentId(103);
    windowSceneSession->property_->SetParentPersistentId(102);
    windowSceneSession->property_->SetParentId(102);
    windowSceneSession->hostSession_ = session;

    windowSceneSession->property_->type_ = WindowType::APP_SUB_WINDOW_BASE;
    ASSERT_EQ(WMError::WM_ERROR_NULLPTR, windowSceneSession->CreateAndConnectSpecificSession().errCode);
    ASSERT_EQ(WMError::WM_OK, windowSceneSession->Destroy(true));
}

/**
 * @tc.name: CreateAndConnectSpecificSession03
 * @tc.desc: CreateAndConnectSpecificSession
 * @tc.type: FUNC
 */
HWTEST_F(WindowSceneSessionImplSpecialWindowTest, CreateAndConnectSpecificSession03, TestSize.Level1)
{
    sptr<WindowOption> option = new (std::nothrow) WindowOption();
    ASSERT_NE(nullptr, option);
    option->SetWindowTag(WindowTag::SUB_WINDOW);
    option->SetWindowName("CreateAndConnectSpecificSession03");
    option->SetIsUIExtFirstSubWindow(true);
    sptr<WindowSceneSessionImpl> windowSceneSession = new (std::nothrow) WindowSceneSessionImpl(option);
    ASSERT_NE(nullptr, windowSceneSession);
    
    SessionInfo sessionInfo = { "CreateTestBundle0", "CreateTestModule0", "CreateTestAbility0" };
    sptr<SessionMocker> session = new (std::nothrow) SessionMocker(sessionInfo);
    ASSERT_NE(nullptr, session);
    ASSERT_EQ(WMError::WM_OK, windowSceneSession->Create(abilityContext_, session));

    windowSceneSession->property_->SetParentPersistentId(102);
    windowSceneSession->property_->SetParentId(102);
    windowSceneSession->hostSession_ = session;

    windowSceneSession->property_->type_ = WindowType::APP_SUB_WINDOW_BASE;
    ASSERT_EQ(WMError::WM_OK, windowSceneSession->CreateAndConnectSpecificSession().errCode);
    ASSERT_EQ(WMError::WM_OK, windowSceneSession->Destroy(true));
}

/**
 * @tc.name: CreateAndConnectSpecificSession04
 * @tc.desc: CreateAndConnectSpecificSession
 * @tc.type: FUNC
 */
HWTEST_F(WindowSceneSessionImplSpecialWindowTest, CreateAndConnectSpecificSession04, TestSize.Level1)
{
    sptr<WindowOption> option = new (std::nothrow) WindowOption();
    ASSERT_NE(nullptr, option);
    option->SetWindowTag(WindowTag::SUB_WINDOW);
    option->SetWindowName("CreateAndConnectSpecificSession04");
    sptr<WindowSceneSessionImpl> windowSceneSession = new (std::nothrow) WindowSceneSessionImpl(option);
    ASSERT_NE(nullptr, windowSceneSession);
    
    SessionInfo sessionInfo = { "CreateTestBundle4", "CreateTestModule4", "CreateTestAbility4" };
    sptr<SessionMocker> session = new (std::nothrow) SessionMocker(sessionInfo);
    ASSERT_NE(nullptr, session);
    ASSERT_EQ(WMError::WM_OK, windowSceneSession->Create(abilityContext_, session));

    windowSceneSession->property_->SetPersistentId(104);
    windowSceneSession->property_->SetParentPersistentId(103);
    windowSceneSession->property_->SetParentId(103);
    windowSceneSession->property_->type_ = WindowType::APP_MAIN_WINDOW_BASE;
    windowSceneSession->hostSession_ = session;

    windowSceneSession->property_->type_ = WindowType::APP_SUB_WINDOW_BASE;
    ASSERT_EQ(WMError::WM_ERROR_NULLPTR, windowSceneSession->CreateAndConnectSpecificSession().errCode);
    ASSERT_EQ(WMError::WM_OK, windowSceneSession->Destroy(true));
}

/**
 * @tc.name: CreateAndConnectSpecificSession05
 * @tc.desc: CreateAndConnectSpecificSession
 * @tc.type: FUNC
 */
HWTEST_F(WindowSceneSessionImplSpecialWindowTest, CreateAndConnectSpecificSession05, TestSize.Level1)
{
    sptr<WindowOption> option = new (std::nothrow) WindowOption();
    ASSERT_NE(nullptr, option);
    option->SetWindowTag(WindowTag::SUB_WINDOW);
    option->SetWindowName("CreateAndConnectSpecificSession05");
    sptr<WindowSceneSessionImpl> windowSceneSession = new (std::nothrow) WindowSceneSessionImpl(option);
    ASSERT_NE(nullptr, windowSceneSession);
    
    SessionInfo sessionInfo = { "CreateTestBundle5", "CreateTestModule5", "CreateTestAbility5" };
    sptr<SessionMocker> session = new (std::nothrow) SessionMocker(sessionInfo);
    ASSERT_NE(nullptr, session);
    ASSERT_EQ(WMError::WM_OK, windowSceneSession->Create(abilityContext_, session));

    windowSceneSession->property_->SetParentPersistentId(104);
    windowSceneSession->property_->SetParentId(104);
    windowSceneSession->hostSession_ = session;

    windowSceneSession->property_->type_ = WindowType::APP_SUB_WINDOW_BASE;
    ASSERT_NE(WMError::WM_ERROR_INVALID_TYPE, windowSceneSession->CreateAndConnectSpecificSession().errCode);
    ASSERT_NE(WMError::WM_OK, windowSceneSession->Destroy(true));
}

/**
 * @tc.name: CreateAndConnectSpecificSession06
 * @tc.desc: CreateAndConnectSpecificSession
 * @tc.type: FUNC
 */
HWTEST_F(WindowSceneSessionImplSpecialWindowTest, CreateAndConnectSpecificSession06, TestSize.Level1)
{
    sptr<WindowOption> option = new (std::nothrow) WindowOption();
    ASSERT_NE(nullptr, option);
    option->SetWindowTag(WindowTag::SYSTEM_WINDOW);
    option->SetWindowName("CreateAndConnectSpecificSession06");
    sptr<WindowSceneSessionImpl> windowSceneSession = new (std::nothrow) WindowSceneSessionImpl(option);
    ASSERT_NE(nullptr, windowSceneSession);
    
    SessionInfo sessionInfo = { "CreateTestBundle6", "CreateTestModule6", "CreateTestAbility6" };
    sptr<SessionMocker> session = new (std::nothrow) SessionMocker(sessionInfo);
    ASSERT_NE(nullptr, session);
    ASSERT_EQ(WMError::WM_OK, windowSceneSession->Create(abilityContext_, session));
    
    windowSceneSession->property_->SetPersistentId(105);
    windowSceneSession->property_->SetParentPersistentId(102);
    windowSceneSession->property_->SetParentId(102);
    windowSceneSession->hostSession_ = session;

    windowSceneSession->property_->type_ = WindowType::SYSTEM_WINDOW_BASE;
    ASSERT_EQ(WMError::WM_OK, windowSceneSession->CreateAndConnectSpecificSession().errCode);
    ASSERT_EQ(WMError::WM_OK, windowSceneSession->Destroy(true));
}

/**
 * @tc.name: CreateAndConnectSpecificSession07
 * @tc.desc: CreateAndConnectSpecificSession
 * @tc.type: FUNC
 */
HWTEST_F(WindowSceneSessionImplSpecialWindowTest, CreateAndConnectSpecificSession07, TestSize.Level1)
{
    sptr<WindowOption> option = new (std::nothrow) WindowOption();
    ASSERT_NE(nullptr, option);
    option->SetWindowTag(WindowTag::SYSTEM_WINDOW);
    option->SetWindowName("CreateAndConnectSpecificSession07");
    sptr<WindowSceneSessionImpl> windowSceneSession = new (std::nothrow) WindowSceneSessionImpl(option);
    ASSERT_NE(nullptr, windowSceneSession);
    
    SessionInfo sessionInfo = { "CreateTestBundle7", "CreateTestModule7", "CreateTestAbility7" };
    sptr<SessionMocker> session = new (std::nothrow) SessionMocker(sessionInfo);
    ASSERT_NE(nullptr, session);
    ASSERT_EQ(WMError::WM_OK, windowSceneSession->Create(abilityContext_, session));
    
    windowSceneSession->property_->SetPersistentId(106);
    windowSceneSession->property_->SetParentPersistentId(105);
    windowSceneSession->property_->SetParentId(105);
    windowSceneSession->hostSession_ = session;

    windowSceneSession->property_->type_ = WindowType::SYSTEM_SUB_WINDOW_BASE;
    ASSERT_EQ(WMError::WM_ERROR_NULLPTR, windowSceneSession->CreateAndConnectSpecificSession().errCode);
    ASSERT_EQ(WMError::WM_OK, windowSceneSession->Destroy(true));
}

/**
 * @tc.name: CreateAndConnectSpecificSession08
 * @tc.desc: CreateAndConnectSpecificSession
 * @tc.type: FUNC
 */
HWTEST_F(WindowSceneSessionImplSpecialWindowTest, CreateAndConnectSpecificSession08, TestSize.Level1)
{
    constexpr int parentId = 10000;
    constexpr int displayId = 100;
    sptr<WindowOption> option = new (std::nothrow) WindowOption();
    ASSERT_NE(nullptr, option);
    option->SetWindowType(WindowType::WINDOW_TYPE_APP_SUB_WINDOW);
    option->SetWindowName("CreateAndConnectSpecificSession08");
    option->SetParentId(parentId);

    // scene session manager processing
    option->SetIsUIExtFirstSubWindow(true);
    sptr<WindowSceneSessionImpl> windowSceneSession = new (std::nothrow) WindowSceneSessionImpl(option);
    ASSERT_NE(nullptr, windowSceneSession);
    windowSceneSession->Create(abilityContext_, nullptr);
    ASSERT_EQ(windowSceneSession->property_->GetParentPersistentId(), option->GetParentId());
    // scene session manager processing
    option->SetIsUIExtFirstSubWindow(false);
    option->SetIsUIExtAnySubWindow(true);
    option->AddWindowFlag(WindowFlag::WINDOW_FLAG_IS_TOAST);
    windowSceneSession = new (std::nothrow) WindowSceneSessionImpl(option);
    ASSERT_NE(nullptr, windowSceneSession);
    windowSceneSession->Create(abilityContext_, nullptr);
    ASSERT_EQ(windowSceneSession->property_->GetParentPersistentId(), option->GetParentId());

    sptr<WindowOption> mainOption = new WindowOption();
    mainOption->SetWindowType(WindowType::WINDOW_TYPE_APP_MAIN_WINDOW);
    mainOption->SetDisplayId(displayId);
    sptr<WindowSceneSessionImpl> mainWindow = new (std::nothrow) WindowSceneSessionImpl(option);
    ASSERT_NE(nullptr, windowSceneSession);
    windowSceneSession->property_->SetPersistentId(parentId);
    // window processing
    option->SetIsUIExtFirstSubWindow(false);
    option->SetIsUIExtAnySubWindow(true);
    option->RemoveWindowFlag(WindowFlag::WINDOW_FLAG_IS_TOAST);
    windowSceneSession = new (std::nothrow) WindowSceneSessionImpl(option);
    ASSERT_NE(nullptr, windowSceneSession);
    windowSceneSession->windowSessionMap_["mainWindow"] = {parentId, mainWindow};
    windowSceneSession->Create(abilityContext_, nullptr);
    ASSERT_EQ(windowSceneSession->property_->GetDisplayId(), mainWindow->property_->GetDisplayId());
    // window processing
    option->SetIsUIExtAnySubWindow(false);
    windowSceneSession = new (std::nothrow) WindowSceneSessionImpl(option);
    ASSERT_NE(nullptr, windowSceneSession);
    windowSceneSession->windowSessionMap_["mainWindow"] = {parentId, mainWindow};
    windowSceneSession->Create(abilityContext_, nullptr);
    ASSERT_EQ(windowSceneSession->property_->GetDisplayId(), mainWindow->property_->GetDisplayId());
    windowSceneSession->Destroy(true);
}

/**
 * @tc.name: FindParentSessionByParentId01
 * @tc.desc: FindParentSessionByParentId
 * @tc.type: FUNC
 */
HWTEST_F(WindowSceneSessionImplSpecialWindowTest, FindParentSessionByParentId01, TestSize.Level1)
{
    sptr<WindowOption> option = new (std::nothrow) WindowOption();
    option->SetWindowTag(WindowTag::MAIN_WINDOW);
    option->SetWindowName("FindParentSessionByParentId01");
    sptr<WindowSceneSessionImpl> windowSceneSession = new (std::nothrow) WindowSceneSessionImpl(option);
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
    sptr<SessionMocker> session = new (std::nothrow) SessionMocker(sessionInfo);
    ASSERT_NE(nullptr, session);

    ASSERT_EQ(WMError::WM_OK, windowSceneSession->Create(abilityContext_, session));
    windowSceneSession->hostSession_ = session;
    ASSERT_TRUE(nullptr != windowSceneSession->FindParentSessionByParentId(1112));
    windowSceneSession->Destroy(true);
}
}
} // namespace Rosen
} // namespace OHOS
