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

#include <gtest/gtest.h>
#include "session/host/include/sub_session.h"

#include "common/include/session_permission.h"
#include "key_event.h"
#include "mock/mock_session_stage.h"
#include "session/host/include/session.h"
#include "session/host/include/main_session.h"
#include "session/host/include/system_session.h"
#include <ui/rs_surface_node.h>
#include "window_event_channel_base.h"
#include "window_helper.h"
#include "window_manager_hilog.h"
#include "window_property.h"
#include "window_session_property.h"

using namespace testing;
using namespace testing::ext;

namespace OHOS {
namespace Rosen {
class SessionStubLifecycleTest : public testing::Test {
public:
    static void SetUpTestCase();
    static void TearDownTestCase();
    void SetUp() override;
    void TearDown() override;
    SessionInfo info;
    sptr <SubSession::SpecificSessionCallback> specificCallback = nullptr;
private:
    RSSurfaceNode::SharedPtr CreateRSSurfaceNode();
    sptr <SubSession> subSession_;
    SystemSessionConfig systemConfig_;
};

void SessionStubLifecycleTest::SetUpTestCase()
{
}

void SessionStubLifecycleTest::TearDownTestCase()
{
}

void SessionStubLifecycleTest::SetUp()
{
    SessionInfo info;
    info.abilityName_ = "testMainSession1";
    info.moduleName_ = "testMainSession2";
    info.bundleName_ = "testMainSession3";
    subSession_ = new SubSession(info, specificCallback);
    EXPECT_NE(nullptr, subSession_);
}

void SessionStubLifecycleTest::TearDown()
{
    subSession_ = nullptr;
}

RSSurfaceNode::SharedPtr SessionStubLifecycleTest::CreateRSSurfaceNode()
{
    struct RSSurfaceNodeConfig rsSurfaceNodeConfig;
    rsSurfaceNodeConfig.SurfaceNodeName = "WindowSessionTestSurfaceNode";
    auto surfaceNode = RSSurfaceNode::Create(rsSurfaceNodeConfig);
    return surfaceNode;
}

namespace {

/**
 * @tc.name: Reconnect01
 * @tc.desc: check func Reconnect
 * @tc.type: FUNC
 */
HWTEST_F(SessionStubLifecycleTest, Reconnect01, Function | SmallTest | Level1)
{
    auto surfaceNode = CreateRSSurfaceNode();
    sptr<WindowSessionProperty> property = new (std::nothrow) WindowSessionProperty();
    ASSERT_NE(nullptr, property);
    sptr<SessionStageMocker> mockSessionStage = new (std::nothrow) SessionStageMocker();
    EXPECT_NE(nullptr, mockSessionStage);
    sptr<TestWindowEventChannel> testWindowEventChannel = new (std::nothrow) TestWindowEventChannel();
    EXPECT_NE(nullptr, testWindowEventChannel);

    auto result = subSession_->Reconnect(nullptr, nullptr, nullptr, property);
    ASSERT_EQ(result, WSError::WS_ERROR_NULLPTR);

    result = subSession_->Reconnect(nullptr, testWindowEventChannel, surfaceNode, property);
    ASSERT_EQ(result, WSError::WS_ERROR_NULLPTR);

    result = subSession_->Reconnect(mockSessionStage, nullptr, surfaceNode, property);
    ASSERT_EQ(result, WSError::WS_ERROR_NULLPTR);

    result = subSession_->Reconnect(mockSessionStage, testWindowEventChannel, surfaceNode, nullptr);
    ASSERT_EQ(result, WSError::WS_ERROR_NULLPTR);

    result = subSession_->Reconnect(mockSessionStage, testWindowEventChannel, surfaceNode, property);
    ASSERT_EQ(result, WSError::WS_ERROR_INVALID_PARAM);

    property->SetWindowState(WindowState::STATE_INITIAL);
    result = subSession_->Reconnect(mockSessionStage, testWindowEventChannel, surfaceNode, property);
    ASSERT_EQ(result, WSError::WS_ERROR_INVALID_PARAM);

    property->SetWindowState(WindowState::STATE_CREATED);
    result = subSession_->Reconnect(mockSessionStage, testWindowEventChannel, surfaceNode, property);
    ASSERT_EQ(result, WSError::WS_OK);

    property->SetWindowState(WindowState::STATE_SHOWN);
    result = subSession_->Reconnect(mockSessionStage, testWindowEventChannel, surfaceNode, property);
    ASSERT_EQ(result, WSError::WS_OK);

    property->SetWindowState(WindowState::STATE_HIDDEN);
    result = subSession_->Reconnect(mockSessionStage, testWindowEventChannel, surfaceNode, property);
    ASSERT_EQ(result, WSError::WS_OK);

    property->SetWindowState(WindowState::STATE_FROZEN);
    result = subSession_->Reconnect(mockSessionStage, testWindowEventChannel, surfaceNode, property);
    ASSERT_EQ(result, WSError::WS_ERROR_INVALID_PARAM);
}

/**
 * @tc.name: Hide01
 * @tc.desc: check func Hide
 * @tc.type: FUNC
 */
HWTEST_F(SessionStubLifecycleTest, Hide01, Function | SmallTest | Level1)
{
    subSession_->Hide();
    subSession_->GetMissionId();
    
    subSession_->isActive_ = true;
    ASSERT_EQ(WSError::WS_OK, subSession_->Hide());
    subSession_->isActive_ = false;

    subSession_->sessionInfo_.isSystem_ = true;
    ASSERT_EQ(WSError::WS_OK, subSession_->Hide());
    subSession_->sessionInfo_.isSystem_ = false;

    sptr<ISessionStage> tempStage_ = subSession_->sessionStage_;
    subSession_->sessionStage_ = nullptr;
    ASSERT_EQ(WSError::WS_OK, subSession_->Hide());
    subSession_->sessionStage_ = tempStage_;
    
    WSRect rect;
    subSession_->UpdatePointerArea(rect);
    subSession_->RectCheck(50, 100);
    ASSERT_EQ(WSError::WS_OK, subSession_->ProcessPointDownSession(50, 100));
}

/**
 * @tc.name: Hide02
 * @tc.desc: check func Hide
 * @tc.type: FUNC
 */
HWTEST_F(SessionStubLifecycleTest, Hide02, Function | SmallTest | Level1)
{
    sptr<WindowSessionProperty> property = new (std::nothrow) WindowSessionProperty();
    ASSERT_NE(nullptr, property);
    property->SetWindowType(WindowType::APP_MAIN_WINDOW_BASE);
    ASSERT_TRUE(subSession_ != nullptr);
    subSession_->SetSessionProperty(property);
    auto result = subSession_->Hide();
    ASSERT_EQ(result, WSError::WS_OK);
}

/**
 * @tc.name: Hide03
 * @tc.desc: check func Hide
 * @tc.type: FUNC
 */
HWTEST_F(SessionStubLifecycleTest, Hide03, Function | SmallTest | Level1)
{
    sptr<WindowSessionProperty> property = nullptr;
    ASSERT_TRUE(subSession_ != nullptr);
    subSession_->SetSessionProperty(property);
    auto result = subSession_->Hide();
    ASSERT_EQ(result, WSError::WS_OK);
}

/**
 * @tc.name: Hide04
 * @tc.desc: check func Hide
 * @tc.type: FUNC
 */
HWTEST_F(SessionStubLifecycleTest, Hide04, Function | SmallTest | Level1)
{
    sptr<WindowSessionProperty> property = new (std::nothrow) WindowSessionProperty();
    ASSERT_NE(nullptr, property);
    sptr<WindowProperty> winPropSrc = new (std::nothrow) WindowProperty();
    ASSERT_NE(nullptr, winPropSrc);
    uint32_t animationFlag = 1;
    winPropSrc->SetAnimationFlag(animationFlag);
    uint32_t res = winPropSrc->GetAnimationFlag();
    ASSERT_NE(res, static_cast<uint32_t>(WindowAnimation::CUSTOM));
    ASSERT_TRUE(subSession_ != nullptr);
    auto result = subSession_->Hide();
    ASSERT_EQ(result, WSError::WS_OK);

    animationFlag = 3;
    winPropSrc->SetAnimationFlag(animationFlag);
    res = winPropSrc->GetAnimationFlag();
    ASSERT_EQ(res, static_cast<uint32_t>(WindowAnimation::CUSTOM));
    ASSERT_TRUE(subSession_ != nullptr);
    result = subSession_->Hide();
    ASSERT_EQ(result, WSError::WS_OK);
}

/**
 * @tc.name: Show01
 * @tc.desc: check func Show
 * @tc.type: FUNC
 */
HWTEST_F(SessionStubLifecycleTest, Show01, Function | SmallTest | Level1)
{
    sptr<WindowSessionProperty> property = nullptr;
    ASSERT_EQ(nullptr, property);

    ASSERT_TRUE(subSession_ != nullptr);
    ASSERT_EQ(WSError::WS_OK, subSession_->Show(property));
}

/**
 * @tc.name: Show02
 * @tc.desc: check func Show
 * @tc.type: FUNC
 */
HWTEST_F(SessionStubLifecycleTest, Show02, Function | SmallTest | Level1)
{
    sptr<WindowSessionProperty> property = new (std::nothrow) WindowSessionProperty();
    ASSERT_NE(nullptr, property);

    sptr<WindowProperty> winPropSrc = new (std::nothrow) WindowProperty();
    ASSERT_NE(nullptr, winPropSrc);
    uint32_t animationFlag = 1;
    winPropSrc->SetAnimationFlag(animationFlag);
    uint32_t res = winPropSrc->GetAnimationFlag();
    ASSERT_NE(res, static_cast<uint32_t>(WindowAnimation::CUSTOM));

    ASSERT_TRUE(subSession_ != nullptr);
    auto result = subSession_->Show(property);
    ASSERT_EQ(result, WSError::WS_OK);
}

/**
 * @tc.name: Show03
 * @tc.desc: check func Show
 * @tc.type: FUNC
 */
HWTEST_F(SessionStubLifecycleTest, Show03, Function | SmallTest | Level1)
{
    sptr<WindowSessionProperty> property = new (std::nothrow) WindowSessionProperty();
    ASSERT_NE(nullptr, property);

    sptr<WindowProperty> winPropSrc = new (std::nothrow) WindowProperty();
    ASSERT_NE(nullptr, winPropSrc);
    uint32_t animationFlag = 3;
    winPropSrc->SetAnimationFlag(animationFlag);
    uint32_t res = winPropSrc->GetAnimationFlag();
    ASSERT_EQ(res, static_cast<uint32_t>(WindowAnimation::CUSTOM));

    ASSERT_TRUE(subSession_ != nullptr);
    auto result = subSession_->Show(property);
    ASSERT_EQ(result, WSError::WS_OK);
}

/**
 * @tc.name: Show04
 * @tc.desc: check func Show
 * @tc.type: FUNC
 */
HWTEST_F(SessionStubLifecycleTest, Show04, Function | SmallTest | Level1)
{
    sptr<WindowSessionProperty> property = new (std::nothrow) WindowSessionProperty();
    ASSERT_NE(property, nullptr);
    property->SetAnimationFlag(static_cast<uint32_t>(WindowAnimation::DEFAULT));
    ASSERT_EQ(subSession_->Show(property), WSError::WS_OK);

    property->SetAnimationFlag(static_cast<uint32_t>(WindowAnimation::CUSTOM));
    ASSERT_EQ(subSession_->Show(property), WSError::WS_OK);

    subSession_->SetSessionProperty(property);
    ASSERT_EQ(subSession_->Show(property), WSError::WS_OK);
}

/**
 * @tc.name: ProcessPointDownSession01
 * @tc.desc: check func ProcessPointDownSession
 * @tc.type: FUNC
 */
HWTEST_F(SessionStubLifecycleTest, ProcessPointDownSession01, Function | SmallTest | Level1)
{
    subSession_->Hide();
    subSession_->SetParentSession(subSession_);
    ASSERT_TRUE(subSession_->GetParentSession() != nullptr);

    WSRect rect;
    subSession_->UpdatePointerArea(rect);
    subSession_->RectCheck(50, 100);
    ASSERT_EQ(subSession_->ProcessPointDownSession(50, 100), WSError::WS_OK);
}

/**
 * @tc.name: ProcessPointDownSession02
 * @tc.desc: check func ProcessPointDownSession
 * @tc.type: FUNC
 */
HWTEST_F(SessionStubLifecycleTest, ProcessPointDownSession02, Function | SmallTest | Level1)
{
    subSession_->Hide();
    WSRect rect;
    subSession_->UpdatePointerArea(rect);
    subSession_->RectCheck(50, 100);

    auto property = subSession_->GetSessionProperty();
    ASSERT_NE(property, nullptr);
    property->SetRaiseEnabled(false);
    ASSERT_FALSE(subSession_->GetSessionProperty()->GetRaiseEnabled());
    ASSERT_EQ(subSession_->ProcessPointDownSession(50, 100), WSError::WS_OK);
}
}
}
}