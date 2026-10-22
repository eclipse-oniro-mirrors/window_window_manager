/*
 * Copyright (c) 2026 Huawei Device Co., Ltd.
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
 
#include "application_context.h"
#include "ability_context_impl.h"
#include "common_test_utils.h"
#include "display_info.h"
#include "mock_session.h"
#include "mock_uicontent.h"
#include "mock_window.h"
#include "mock_window_adapter.h"
#include "pointer_event.h"
#include "rs_adapter.h"
#include "scene_board_judgement.h"
#include "singleton_mocker.h"
#include "wm_common_inner.h"
#include "window_scene_session_impl.h"
#include "window_session_impl.h"
 
using namespace testing;
using namespace testing::ext;
 
namespace OHOS {
namespace Rosen {
namespace {
using WindowAdapterMocker = SingletonMocker<WindowAdapter, MockWindowAdapter>;
}
namespace {
    std::string logMsg;
    void MyLogCallback(const LogType type, const LogLevel level, const unsigned int domain, const char* tag,
        const char* msg)
    {
        logMsg += msg;
    }
}
class WindowSceneSessionImplCompatibleModeTest : public testing::Test {
public:
    static void SetUpTestCase();
    static void TearDownTestCase();
    void SetUp() override;
    void TearDown() override;
private:
    static constexpr uint32_t WAIT_SYNC_IN_NS = 200000;
};
 
void WindowSceneSessionImplCompatibleModeTest::SetUpTestCase()
{
}
 
void WindowSceneSessionImplCompatibleModeTest::TearDownTestCase()
{
}
 
void WindowSceneSessionImplCompatibleModeTest::SetUp()
{
}
 
void WindowSceneSessionImplCompatibleModeTest::TearDown()
{
}
 
namespace {
/**
 * @tc.name: NotifyPageEnable
 * @tc.desc: Test NotifyPageEnable with valid session
 * @tc.type: FUNC
 */
HWTEST_F(WindowSceneSessionImplCompatibleModeTest, NotifyPageEnable, TestSize.Level1)
{
    sptr<WindowOption> option = sptr<WindowOption>::MakeSptr();
    option->SetWindowName("NotifyPageEnable");
    sptr<WindowSceneSessionImpl> window = sptr<WindowSceneSessionImpl>::MakeSptr(option);
    ASSERT_NE(nullptr, window);

    auto ret = window->NotifyPageEnable("enter", "HomePage");
    EXPECT_EQ(ret, WMError::WM_ERROR_INVALID_WINDOW);

    window->property_->SetPersistentId(1);
    SessionInfo sessionInfo = { "CreateTestBundle", "CreateTestModule", "CreateTestAbility" };
    sptr<SessionMocker> session = sptr<SessionMocker>::MakeSptr(sessionInfo);
    window->hostSession_ = session;

    EXPECT_CALL(*session, NotifyPageEnable(_, _))
        .Times(1)
        .WillOnce(Return(WSError::WS_OK));

    ret = window->NotifyPageEnable("enter", "HomePage");
    EXPECT_EQ(ret, WMError::WM_OK);
}
 
/**
 * @tc.name: NotifyPageEnable01
 * @tc.desc: Test NotifyPageEnable with null host session
 * @tc.type: FUNC
 */
HWTEST_F(WindowSceneSessionImplCompatibleModeTest, NotifyPageEnable01, TestSize.Level1)
{
    sptr<WindowOption> option = sptr<WindowOption>::MakeSptr();
    option->SetWindowName("NotifyPageEnable01");
    sptr<WindowSceneSessionImpl> window = sptr<WindowSceneSessionImpl>::MakeSptr(option);
    ASSERT_NE(nullptr, window);

    window->property_->SetPersistentId(1);
    window->hostSession_ = nullptr;

    auto ret = window->NotifyPageEnable("enter", "HomePage");
    EXPECT_EQ(ret, WMError::WM_ERROR_INVALID_WINDOW);
}
 
/**
 * @tc.name: NotifyPageEnable02
 * @tc.desc: Test NotifyPageEnable when host session returns error
 * @tc.type: FUNC
 */
HWTEST_F(WindowSceneSessionImplCompatibleModeTest, NotifyPageEnable02, TestSize.Level1)
{
    sptr<WindowOption> option = sptr<WindowOption>::MakeSptr();
    option->SetWindowName("NotifyPageEnable02");
    sptr<WindowSceneSessionImpl> window = sptr<WindowSceneSessionImpl>::MakeSptr(option);
    ASSERT_NE(nullptr, window);

    window->property_->SetPersistentId(1);
    SessionInfo sessionInfo = { "CreateTestBundle", "CreateTestModule", "CreateTestAbility" };
    sptr<SessionMocker> session = sptr<SessionMocker>::MakeSptr(sessionInfo);
    window->hostSession_ = session;

    EXPECT_CALL(*session, NotifyPageEnable(_, _))
        .Times(1)
        .WillOnce(Return(WSError::WS_ERROR_INVALID_PARAM));

    auto ret = window->NotifyPageEnable("enter", "HomePage");
    EXPECT_EQ(ret, WMError::WM_ERROR_INVALID_PARAM);
}
 
/**
 * @tc.name: NotifyPageEnable03
 * @tc.desc: Test NotifyPageEnable with multiple calls
 * @tc.type: FUNC
 */
HWTEST_F(WindowSceneSessionImplCompatibleModeTest, NotifyPageEnable03, TestSize.Level1)
{
    sptr<WindowOption> option = sptr<WindowOption>::MakeSptr();
    option->SetWindowName("NotifyPageEnable03");
    sptr<WindowSceneSessionImpl> window = sptr<WindowSceneSessionImpl>::MakeSptr(option);
    ASSERT_NE(nullptr, window);

    window->property_->SetPersistentId(1);
    SessionInfo sessionInfo = { "CreateTestBundle", "CreateTestModule", "CreateTestAbility" };
    sptr<SessionMocker> session = sptr<SessionMocker>::MakeSptr(sessionInfo);
    window->hostSession_ = session;

    EXPECT_CALL(*session, NotifyPageEnable(_, _))
        .Times(3)
        .WillRepeatedly(Return(WSError::WS_OK));

    EXPECT_EQ(window->NotifyPageEnable("enter", "HomePage"), WMError::WM_OK);
    EXPECT_EQ(window->NotifyPageEnable("exit", "HomePage"), WMError::WM_OK);
    EXPECT_EQ(window->NotifyPageEnable("enter", "DetailPage"), WMError::WM_OK);
}

/**
 * @tc.name: NotifyPageEnableHookScene
 * @tc.desc: Test NotifyPageEnable stores message only for hookscene action
 * @tc.type: FUNC
 */
HWTEST_F(WindowSceneSessionImplCompatibleModeTest, NotifyPageEnableHookScene, TestSize.Level1)
{
    sptr<WindowOption> option = sptr<WindowOption>::MakeSptr();
    option->SetWindowName("NotifyPageEnableHookScene");
    sptr<WindowSceneSessionImpl> window = sptr<WindowSceneSessionImpl>::MakeSptr(option);
    ASSERT_NE(nullptr, window);

    window->property_->SetPersistentId(1);
    SessionInfo sessionInfo = { "CreateTestBundle", "CreateTestModule", "CreateTestAbility" };
    sptr<SessionMocker> session = sptr<SessionMocker>::MakeSptr(sessionInfo);
    window->hostSession_ = session;

    EXPECT_CALL(*session, NotifyPageEnable(_, _))
        .Times(2)
        .WillRepeatedly(Return(WSError::WS_OK));
 
    EXPECT_TRUE(window->property_->GetHookSceneMessage().empty());

    std::string hookSceneMessage = "hookSceneMessage";
    EXPECT_EQ(window->NotifyPageEnable("hookscene", hookSceneMessage), WMError::WM_OK);
    EXPECT_EQ(window->property_->GetHookSceneMessage(), hookSceneMessage);

    EXPECT_EQ(window->NotifyPageEnable("enter", "HomePage"), WMError::WM_OK);
    EXPECT_EQ(window->property_->GetHookSceneMessage(), hookSceneMessage);
}
} // namespace
} // namespace Rosen
} // namespace OHOS