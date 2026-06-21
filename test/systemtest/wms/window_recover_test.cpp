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

// gtest
#include <gmock/gmock.h>
#include <gtest/gtest.h>
#include "ability_context_impl.h"
#include "context_impl.h"
#include "iremote_object_mocker.h"
#include "mock_session.h"
#include "mock_window_adapter.h"
#include "singleton_mocker.h"
#include "window_scene_session_impl.h"
#include "window_session_impl.h"
#include "window_test_utils.h"

using namespace testing;
using namespace testing::ext;

namespace OHOS {
namespace Rosen {
using Mocker = SingletonMocker<WindowAdapter, MockWindowAdapter>;

using Utils = WindowTestUtils;
class WindowRecoverTest : public testing::Test {
public:
    static void SetUpTestCase();
    static void TearDownTestCase();
    void SetUp() override;
    void TearDown() override;

    std::unique_ptr<Mocker> m = std::make_unique<Mocker>();
};

void WindowRecoverTest::SetUpTestCase() {}

void WindowRecoverTest::TearDownTestCase()
{
}

void WindowRecoverTest::SetUp() {}

void WindowRecoverTest::TearDown() {}

namespace {
/**
 * @tc.name: RecoverAndReconnectSceneSession
 * @tc.desc: RecoverAndReconnectSceneSession
 * @tc.type: FUNC
 */
HWTEST_F(WindowRecoverTest, RecoverAndReconnectSceneSession, TestSize.Level1)
{
    sptr<WindowOption> option = new (std::nothrow) WindowOption();
    ASSERT_NE(nullptr, option);
    option->SetWindowName("RecoverAndReconnectSceneSession");
    sptr<WindowSceneSessionImpl> windowSceneSession = new (std::nothrow) WindowSceneSessionImpl(option);
    ASSERT_NE(nullptr, windowSceneSession);
    windowSceneSession->property_->SetPersistentId(1112);
    windowSceneSession->property_->SetParentId(1000);
    windowSceneSession->property_->SetParentPersistentId(1000);
    windowSceneSession->property_->SetWindowType(WindowType::WINDOW_TYPE_APP_MAIN_WINDOW);

    std::shared_ptr<AbilityRuntime::AbilityContextImpl> context =
        std::make_shared<AbilityRuntime::AbilityContextImpl>();
    ASSERT_NE(nullptr, context);
    SessionInfo sessionInfo = { "CreateTestBundle", "CreateTestModule", "CreateTestAbility" };
    sptr<SessionMocker> session = new (std::nothrow) SessionMocker(sessionInfo);
    ASSERT_NE(nullptr, session);
    std::shared_ptr<AppExecFwk::AbilityInfo> abilityInfo = std::make_shared<AppExecFwk::AbilityInfo>();
    context->SetAbilityInfo(abilityInfo);
    sptr<IRemoteObject> sessionToken = new (std::nothrow) IRemoteObjectMocker();
    ASSERT_NE(nullptr, sessionToken);
    context->SetToken(sessionToken);
    std::shared_ptr<AppExecFwk::ApplicationInfo> applicationInfo = std::make_shared<AppExecFwk::ApplicationInfo>();
    ASSERT_NE(nullptr, applicationInfo);
    applicationInfo->apiCompatibleVersion = 12;
    auto stageContent = std::make_shared<AbilityRuntime::ContextImpl>();
    ASSERT_NE(nullptr, stageContent);
    stageContent->SetApplicationInfo(applicationInfo);
    stageContent->InitHapModuleInfo(abilityInfo);
    context->stageContext_ = stageContent;

    struct RSSurfaceNodeConfig config;
    std::shared_ptr<RSSurfaceNode> windowSceneSessionSurfaceNode = RSSurfaceNode::Create(config);
    windowSceneSession->surfaceNode_ = windowSceneSessionSurfaceNode;
    ASSERT_EQ(WMError::WM_OK, windowSceneSession->Create(context, session));

    std::shared_ptr<RSSurfaceNode> surfaceNode = nullptr;
    sptr<WindowSessionProperty> property = nullptr;
    sptr<IRemoteObject> token = nullptr;
    EXPECT_CALL(m->Mock(), RecoverAndReconnectSceneSession(_, _, _, _, _, _))
        .WillOnce(DoAll(SaveArg<2>(&surfaceNode), SaveArg<4>(&property), SaveArg<5>(&token), Return(WMError::WM_OK)));
    windowSceneSession->RecoverAndReconnectSceneSession();
    ASSERT_EQ(surfaceNode, windowSceneSession->surfaceNode_);
    ASSERT_EQ(property, windowSceneSession->property_);
    ASSERT_EQ(token, sessionToken);
    windowSceneSession->Destroy(false, false);
}

/**
 * @tc.name: RecoverAndConnectSpecificSession
 * @tc.desc: RecoverAndConnectSpecificSession
 * @tc.type: FUNC
 */
HWTEST_F(WindowRecoverTest, RecoverAndConnectSpecificSession, TestSize.Level1)
{
    sptr<WindowOption> option = new (std::nothrow) WindowOption();
    ASSERT_NE(nullptr, option);
    option->SetWindowName("RecoverAndConnectSpecificSession");
    sptr<WindowSceneSessionImpl> windowSceneSession = new (std::nothrow) WindowSceneSessionImpl(option);
    ASSERT_NE(nullptr, windowSceneSession);
    windowSceneSession->property_->SetPersistentId(1112);
    windowSceneSession->property_->SetWindowType(WindowType::APP_SUB_WINDOW_BASE);
    windowSceneSession->property_->SetIsUIExtFirstSubWindow(true);

    std::shared_ptr<AbilityRuntime::AbilityContextImpl> context =
        std::make_shared<AbilityRuntime::AbilityContextImpl>();
    ASSERT_NE(nullptr, context);
    SessionInfo sessionInfo = { "CreateTestBundle", "CreateTestModule", "CreateTestAbility" };
    sptr<SessionMocker> session = new (std::nothrow) SessionMocker(sessionInfo);
    ASSERT_NE(nullptr, session);
    sptr<IRemoteObject> sessionToken = new (std::nothrow) IRemoteObjectMocker();
    ASSERT_NE(nullptr, sessionToken);
    context->SetToken(sessionToken);
    struct RSSurfaceNodeConfig config;
    std::shared_ptr<RSSurfaceNode> windowSceneSessionSurfaceNode = RSSurfaceNode::Create(config);
    ASSERT_NE(nullptr, windowSceneSessionSurfaceNode);
    windowSceneSession->surfaceNode_ = windowSceneSessionSurfaceNode;
    ASSERT_EQ(WMError::WM_OK, windowSceneSession->Create(context, session));

    std::shared_ptr<RSSurfaceNode> surfaceNode = nullptr;
    sptr<WindowSessionProperty> property = nullptr;
    sptr<IRemoteObject> token = nullptr;
    EXPECT_CALL(m->Mock(), RecoverAndConnectSpecificSession(_, _, _, _, _, _))
        .WillOnce(DoAll(SaveArg<2>(&surfaceNode), SaveArg<3>(&property), SaveArg<5>(&token)));
    windowSceneSession->RecoverAndConnectSpecificSession();
    ASSERT_EQ(surfaceNode, windowSceneSession->surfaceNode_);
    ASSERT_EQ(property, windowSceneSession->property_);
    ASSERT_EQ(token, sessionToken);
    windowSceneSession->Destroy(false, false);
}
} // namespace
} // namespace Rosen
} // namespace OHOS
