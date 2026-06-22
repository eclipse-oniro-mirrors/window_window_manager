/*
 * Copyright (c) 2026 Huawei Device Co., Ltd.
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
#include "mock_session.h"
#include "mock_uicontent.h"
#include "mock_window.h"
#include "parameters.h"
#include "window_helper.h"
#include "window_session_impl.h"
#include "wm_common.h"
#include "window_manager_hilog.h"

using namespace testing;
using namespace testing::ext;

namespace OHOS {
namespace Rosen {
class WindowSessionImplCompatibleModeTest : public testing::Test {
public:
    static void SetUpTestCase();
    static void TearDownTestCase();
    void SetUp() override;
    void TearDown() override;

    std::shared_ptr<AbilityRuntime::AbilityContext> abilityContext_;

private:
    static constexpr uint32_t WAIT_SYNC_IN_NS = 50000;
};

void WindowSessionImplCompatibleModeTest::SetUpTestCase() {}

void WindowSessionImplCompatibleModeTest::TearDownTestCase() {}

void WindowSessionImplCompatibleModeTest::SetUp()
{
    abilityContext_ = std::make_shared<AbilityRuntime::AbilityContextImpl>();
}

void WindowSessionImplCompatibleModeTest::TearDown()
{
    usleep(WAIT_SYNC_IN_NS);
    abilityContext_ = nullptr;
}

namespace {
sptr<WindowSessionImpl> GetTestWindowImpl(const std::string& name)
{
    sptr<WindowOption> option = new (std::nothrow) WindowOption();
    if (option == nullptr) {
        return nullptr;
    }
    option->SetWindowName(name);
    sptr<WindowSessionImpl> window = new (std::nothrow) WindowSessionImpl(option);
    if (window == nullptr) {
        return nullptr;
    }
    SessionInfo sessionInfo = { name, name, name };
    sptr<SessionMocker> session = new (std::nothrow) SessionMocker(sessionInfo);
    if (session == nullptr) {
        return nullptr;
    }
    window->hostSession_ = session;
    return window;
}

/**
 * @tc.name: SetAppHookWindowInfoInShownState
 * @tc.desc: Test SetAppHookWindowInfo when window is in STATE_SHOWN
 * @tc.type: FUNC
 */
HWTEST_F(WindowSessionImplCompatibleModeTest, SetAppHookWindowInfoInShownState, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "WindowSessionImplCompatibleModeTest: SetAppHookWindowInfoInShownState start";
    sptr<WindowOption> option = new (std::nothrow) WindowOption();
    ASSERT_NE(option, nullptr);
    option->SetWindowName("SetAppHookWindowInfoInShownState");
    sptr<WindowSessionImpl> window = new (std::nothrow) WindowSessionImpl(option);
    ASSERT_NE(window, nullptr);

    SessionInfo sessionInfo = { "testBundle", "testModule", "testAbility" };
    sptr<SessionMocker> session = new (std::nothrow) SessionMocker(sessionInfo);
    ASSERT_NE(session, nullptr);
    window->hostSession_ = session;
    window->property_->SetPersistentId(1001);

    window->state_ = WindowState::STATE_SHOWN;
    window->property_->SetWindowRect({ 0, 0, 800, 600 });

    HookWindowInfo hookWindowInfo;
    hookWindowInfo.enableHookWindow = true;
    hookWindowInfo.widthHookRatio = 0.5f;
    hookWindowInfo.notifyWindowChange = true;

    window->SetAppHookWindowInfo(hookWindowInfo);

    EXPECT_EQ(window->state_, WindowState::STATE_SHOWN);
    EXPECT_FALSE(window->notifySizeChangeInCompatibleMode_.load());
    EXPECT_FALSE(window->notifyRectChangeInCompatibleMode_.load());
    GTEST_LOG_(INFO) << "WindowSessionImplCompatibleModeTest: SetAppHookWindowInfoInShownState end";
}

/**
 * @tc.name: SetAppHookWindowInfoInHiddenState
 * @tc.desc: Test SetAppHookWindowInfo when window is not in STATE_SHOWN
 * @tc.type: FUNC
 */
HWTEST_F(WindowSessionImplCompatibleModeTest, SetAppHookWindowInfoInHiddenState, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "WindowSessionImplCompatibleModeTest: SetAppHookWindowInfoInHiddenState start";
    sptr<WindowOption> option = new (std::nothrow) WindowOption();
    ASSERT_NE(option, nullptr);
    option->SetWindowName("SetAppHookWindowInfoInHiddenState");
    sptr<WindowSessionImpl> window = new (std::nothrow) WindowSessionImpl(option);
    ASSERT_NE(window, nullptr);

    SessionInfo sessionInfo = { "testBundle", "testModule", "testAbility" };
    sptr<SessionMocker> session = new (std::nothrow) SessionMocker(sessionInfo);
    ASSERT_NE(session, nullptr);
    window->hostSession_ = session;
    window->property_->SetPersistentId(1002);

    window->state_ = WindowState::STATE_CREATED;
    window->property_->SetWindowRect({ 0, 0, 800, 600 });

    HookWindowInfo hookWindowInfo;
    hookWindowInfo.enableHookWindow = true;
    hookWindowInfo.widthHookRatio = 0.5f;
    hookWindowInfo.notifyWindowChange = true;

    window->SetAppHookWindowInfo(hookWindowInfo);

    EXPECT_EQ(window->state_, WindowState::STATE_CREATED);
    EXPECT_TRUE(window->notifySizeChangeInCompatibleMode_.load());
    EXPECT_TRUE(window->notifyRectChangeInCompatibleMode_.load());
    GTEST_LOG_(INFO) << "WindowSessionImplCompatibleModeTest: SetAppHookWindowInfoInHiddenState end";
}

/**
 * @tc.name: SetAppHookWindowInfoWithoutNotify
 * @tc.desc: Test SetAppHookWindowInfo when notifyWindowChange is false
 * @tc.type: FUNC
 */
HWTEST_F(WindowSessionImplCompatibleModeTest, SetAppHookWindowInfoWithoutNotify, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "WindowSessionImplCompatibleModeTest: SetAppHookWindowInfoWithoutNotify start";
    sptr<WindowOption> option = new (std::nothrow) WindowOption();
    ASSERT_NE(option, nullptr);
    option->SetWindowName("SetAppHookWindowInfoWithoutNotify");
    sptr<WindowSessionImpl> window = new (std::nothrow) WindowSessionImpl(option);
    ASSERT_NE(window, nullptr);

    SessionInfo sessionInfo = { "testBundle", "testModule", "testAbility" };
    sptr<SessionMocker> session = new (std::nothrow) SessionMocker(sessionInfo);
    ASSERT_NE(session, nullptr);
    window->hostSession_ = session;
    window->property_->SetPersistentId(1003);

    window->state_ = WindowState::STATE_CREATED;
    window->property_->SetWindowRect({ 0, 0, 800, 600 });

    HookWindowInfo hookWindowInfo;
    hookWindowInfo.enableHookWindow = true;
    hookWindowInfo.widthHookRatio = 0.5f;
    hookWindowInfo.notifyWindowChange = false;

    window->SetAppHookWindowInfo(hookWindowInfo);

    EXPECT_EQ(window->state_, WindowState::STATE_CREATED);
    EXPECT_FALSE(window->notifySizeChangeInCompatibleMode_.load());
    EXPECT_FALSE(window->notifyRectChangeInCompatibleMode_.load());
    GTEST_LOG_(INFO) << "WindowSessionImplCompatibleModeTest: SetAppHookWindowInfoWithoutNotify end";
}

/**
 * @tc.name: UpdateGlobalDisplayRectFromServerWithPendingNotification
 * @tc.desc: Test UpdateGlobalDisplayRectFromServer when notifyRectChangeInCompatibleMode_ is true
 * @tc.type: FUNC
 */
HWTEST_F(WindowSessionImplCompatibleModeTest, UpdateGlobalDisplayRectFromServerWithPendingNotification,
    TestSize.Level1)
{
    GTEST_LOG_(INFO) << "WindowSessionImplCompatibleModeTest: "
        "UpdateGlobalDisplayRectFromServerWithPendingNotification start";
    sptr<WindowOption> option = new (std::nothrow) WindowOption();
    ASSERT_NE(option, nullptr);
    option->SetWindowName("UpdateGlobalDisplayRectFromServerWithPendingNotification");
    sptr<WindowSessionImpl> window = new (std::nothrow) WindowSessionImpl(option);
    ASSERT_NE(window, nullptr);

    window->property_->SetPersistentId(1004);
    window->property_->SetGlobalDisplayRect({ 10, 20, 200, 100 });
    window->globalDisplayRectSizeChangeReason_ = SizeChangeReason::RESIZE;
    window->notifyRectChangeInCompatibleMode_.store(true);

    WSRect rect = { 10, 20, 200, 100 };
    auto ret = window->UpdateGlobalDisplayRectFromServer(rect, SizeChangeReason::RESIZE);

    EXPECT_EQ(ret, WSError::WS_DO_NOTHING);
    EXPECT_FALSE(window->notifyRectChangeInCompatibleMode_.load());
    GTEST_LOG_(INFO) << "WindowSessionImplCompatibleModeTest: "
        "UpdateGlobalDisplayRectFromServerWithPendingNotification end";
}

/**
 * @tc.name: UpdateGlobalDisplayRectFromServerWithoutPendingNotification
 * @tc.desc: Test UpdateGlobalDisplayRectFromServer when notifyRectChangeInCompatibleMode_ is false
 * @tc.type: FUNC
 */
HWTEST_F(WindowSessionImplCompatibleModeTest, UpdateGlobalDisplayRectFromServerWithoutPendingNotification,
 TestSize.Level1)
{
    GTEST_LOG_(INFO) << "WindowSessionImplCompatibleModeTest: "
        "UpdateGlobalDisplayRectFromServerWithoutPendingNotification start";
    sptr<WindowOption> option = new (std::nothrow) WindowOption();
    ASSERT_NE(option, nullptr);
    option->SetWindowName("UpdateGlobalDisplayRectFromServerWithoutPendingNotification");
    sptr<WindowSessionImpl> window = new (std::nothrow) WindowSessionImpl(option);
    ASSERT_NE(window, nullptr);

    window->property_->SetPersistentId(1005);
    window->property_->SetGlobalDisplayRect({ 10, 20, 200, 100 });
    window->globalDisplayRectSizeChangeReason_ = SizeChangeReason::RESIZE;
    window->notifyRectChangeInCompatibleMode_.store(false);

    WSRect rect = { 10, 20, 200, 100 };
    auto ret = window->UpdateGlobalDisplayRectFromServer(rect, SizeChangeReason::RESIZE);

    EXPECT_EQ(ret, WSError::WS_DO_NOTHING);
    EXPECT_FALSE(window->notifyRectChangeInCompatibleMode_.load());
    GTEST_LOG_(INFO) << "WindowSessionImplCompatibleModeTest: "
        "UpdateGlobalDisplayRectFromServerWithoutPendingNotification end";
}

/**
 * @tc.name: UpdateGlobalDisplayRectFromServerWithRectChange
 * @tc.desc: Test UpdateGlobalDisplayRectFromServer when rect changes
 * @tc.type: FUNC
 */
HWTEST_F(WindowSessionImplCompatibleModeTest, UpdateGlobalDisplayRectFromServerWithRectChange, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "WindowSessionImplCompatibleModeTest: UpdateGlobalDisplayRectFromServerWithRectChange start";
    sptr<WindowOption> option = new (std::nothrow) WindowOption();
    ASSERT_NE(option, nullptr);
    option->SetWindowName("UpdateGlobalDisplayRectFromServerWithRectChange");
    sptr<WindowSessionImpl> window = new (std::nothrow) WindowSessionImpl(option);
    ASSERT_NE(window, nullptr);

    window->property_->SetPersistentId(1006);
    window->property_->SetGlobalDisplayRect({ 10, 20, 200, 100 });
    window->globalDisplayRectSizeChangeReason_ = SizeChangeReason::RESIZE;
    window->notifyRectChangeInCompatibleMode_.store(true);

    WSRect rect = { 30, 40, 200, 100 };
    auto ret = window->UpdateGlobalDisplayRectFromServer(rect, SizeChangeReason::RESIZE);

    EXPECT_EQ(ret, WSError::WS_OK);
    Rect expectedRect = { 30, 40, 200, 100 };
    EXPECT_EQ(window->GetGlobalDisplayRect(), expectedRect);
    EXPECT_EQ(window->globalDisplayRectSizeChangeReason_, SizeChangeReason::RESIZE);
    GTEST_LOG_(INFO) << "WindowSessionImplCompatibleModeTest: UpdateGlobalDisplayRectFromServerWithRectChange end";
}

/**
 * @tc.name: UpdateGlobalDisplayRectFromServerWithReasonChange
 * @tc.desc: Test UpdateGlobalDisplayRectFromServer when reason changes
 * @tc.type: FUNC
 */
HWTEST_F(WindowSessionImplCompatibleModeTest, UpdateGlobalDisplayRectFromServerWithReasonChange, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "WindowSessionImplCompatibleModeTest: UpdateGlobalDisplayRectFromServerWithReasonChange start";
    sptr<WindowOption> option = new (std::nothrow) WindowOption();
    ASSERT_NE(option, nullptr);
    option->SetWindowName("UpdateGlobalDisplayRectFromServerWithReasonChange");
    sptr<WindowSessionImpl> window = new (std::nothrow) WindowSessionImpl(option);
    ASSERT_NE(window, nullptr);

    window->property_->SetPersistentId(1007);
    window->property_->SetGlobalDisplayRect({ 10, 20, 200, 100 });
    window->globalDisplayRectSizeChangeReason_ = SizeChangeReason::RESIZE;
    window->notifyRectChangeInCompatibleMode_.store(true);

    WSRect rect = { 10, 20, 200, 100 };
    auto ret = window->UpdateGlobalDisplayRectFromServer(rect, SizeChangeReason::MOVE);

    EXPECT_EQ(ret, WSError::WS_OK);
    Rect expectedRect = { 10, 20, 200, 100 };
    EXPECT_EQ(window->GetGlobalDisplayRect(), expectedRect);
    EXPECT_EQ(window->globalDisplayRectSizeChangeReason_, SizeChangeReason::MOVE);
    GTEST_LOG_(INFO) << "WindowSessionImplCompatibleModeTest: UpdateGlobalDisplayRectFromServerWithReasonChange end";
}

/**
 * @tc.name: UpdateRectForOtherReasonTaskWithPendingNotification
 * @tc.desc: Test UpdateRectForOtherReasonTask when notifySizeChangeInCompatibleMode_ is true
 * @tc.type: FUNC
 */
HWTEST_F(WindowSessionImplCompatibleModeTest, UpdateRectForOtherReasonTaskWithPendingNotification, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "WindowSessionImplCompatibleModeTest: "
        "UpdateRectForOtherReasonTaskWithPendingNotification start";
    sptr<WindowOption> option = new (std::nothrow) WindowOption();
    ASSERT_NE(option, nullptr);
    option->SetWindowName("UpdateRectForOtherReasonTaskWithPendingNotification");
    sptr<WindowSessionImpl> window = new (std::nothrow) WindowSessionImpl(option);
    ASSERT_NE(window, nullptr);

    SessionInfo sessionInfo = { "testBundle", "testModule", "testAbility" };
    sptr<SessionMocker> session = new (std::nothrow) SessionMocker(sessionInfo);
    ASSERT_NE(session, nullptr);
    window->hostSession_ = session;
    window->property_->SetPersistentId(1008);

    Rect wmRect = { 0, 0, 800, 600 };
    Rect preRect = { 0, 0, 800, 600 };
    WindowSizeChangeReason wmReason = WindowSizeChangeReason::UNDEFINED;
    std::shared_ptr<RSTransaction> rsTransaction = nullptr;

    window->lastSizeChangeReason_ = WindowSizeChangeReason::UNDEFINED;
    window->postTaskDone_ = true;
    window->notifySizeChangeInCompatibleMode_.store(true);

    window->UpdateRectForOtherReasonTask(wmRect, preRect, wmReason, rsTransaction);

    EXPECT_FALSE(window->notifySizeChangeInCompatibleMode_.load());
    EXPECT_TRUE(window->postTaskDone_);
    GTEST_LOG_(INFO) << "WindowSessionImplCompatibleModeTest: "
        "UpdateRectForOtherReasonTaskWithPendingNotification end";
}

/**
 * @tc.name: UpdateRectForOtherReasonTaskWithoutPendingNotification
 * @tc.desc: Test UpdateRectForOtherReasonTask when notifySizeChangeInCompatibleMode_ is false
 * @tc.type: FUNC
 */
HWTEST_F(WindowSessionImplCompatibleModeTest, UpdateRectForOtherReasonTaskWithoutPendingNotification, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "WindowSessionImplCompatibleModeTest: "
        "UpdateRectForOtherReasonTaskWithoutPendingNotification start";
    sptr<WindowOption> option = new (std::nothrow) WindowOption();
    ASSERT_NE(option, nullptr);
    option->SetWindowName("UpdateRectForOtherReasonTaskWithoutPendingNotification");
    sptr<WindowSessionImpl> window = new (std::nothrow) WindowSessionImpl(option);
    ASSERT_NE(window, nullptr);

    SessionInfo sessionInfo = { "testBundle", "testModule", "testAbility" };
    sptr<SessionMocker> session = new (std::nothrow) SessionMocker(sessionInfo);
    ASSERT_NE(session, nullptr);
    window->hostSession_ = session;
    window->property_->SetPersistentId(1009);

    Rect wmRect = { 0, 0, 800, 600 };
    Rect preRect = { 0, 0, 800, 600 };
    WindowSizeChangeReason wmReason = WindowSizeChangeReason::UNDEFINED;
    std::shared_ptr<RSTransaction> rsTransaction = nullptr;

    window->lastSizeChangeReason_ = WindowSizeChangeReason::UNDEFINED;
    window->postTaskDone_ = true;
    window->notifySizeChangeInCompatibleMode_.store(false);

    window->UpdateRectForOtherReasonTask(wmRect, preRect, wmReason, rsTransaction);

    EXPECT_FALSE(window->notifySizeChangeInCompatibleMode_.load());
    EXPECT_TRUE(window->postTaskDone_);
    GTEST_LOG_(INFO) << "WindowSessionImplCompatibleModeTest: "
        "UpdateRectForOtherReasonTaskWithoutPendingNotification end";
}

/**
 * @tc.name: SetAppHookWindowInfoAndUpdateRectSequence
 * @tc.desc: Test sequence: SetAppHookWindowInfo in hidden state, then UpdateRectForOtherReasonTask
 * @tc.type: FUNC
 */
HWTEST_F(WindowSessionImplCompatibleModeTest, SetAppHookWindowInfoAndUpdateRectSequence, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "WindowSessionImplCompatibleModeTest: SetAppHookWindowInfoAndUpdateRectSequence start";
    sptr<WindowOption> option = new (std::nothrow) WindowOption();
    ASSERT_NE(option, nullptr);
    option->SetWindowName("SetAppHookWindowInfoAndUpdateRectSequence");
    sptr<WindowSessionImpl> window = new (std::nothrow) WindowSessionImpl(option);
    ASSERT_NE(window, nullptr);

    SessionInfo sessionInfo = { "testBundle", "testModule", "testAbility" };
    sptr<SessionMocker> session = new (std::nothrow) SessionMocker(sessionInfo);
    ASSERT_NE(session, nullptr);
    window->hostSession_ = session;
    window->property_->SetPersistentId(1010);

    window->state_ = WindowState::STATE_CREATED;
    window->property_->SetWindowRect({ 0, 0, 800, 600 });

    HookWindowInfo hookWindowInfo;
    hookWindowInfo.enableHookWindow = true;
    hookWindowInfo.widthHookRatio = 0.5f;
    hookWindowInfo.notifyWindowChange = true;

    window->SetAppHookWindowInfo(hookWindowInfo);

    EXPECT_TRUE(window->notifySizeChangeInCompatibleMode_.load());
    EXPECT_TRUE(window->notifyRectChangeInCompatibleMode_.load());

    Rect wmRect = { 0, 0, 800, 600 };
    Rect preRect = { 0, 0, 800, 600 };
    WindowSizeChangeReason wmReason = WindowSizeChangeReason::UNDEFINED;
    std::shared_ptr<RSTransaction> rsTransaction = nullptr;

    window->lastSizeChangeReason_ = WindowSizeChangeReason::UNDEFINED;
    window->postTaskDone_ = true;

    window->UpdateRectForOtherReasonTask(wmRect, preRect, wmReason, rsTransaction);

    EXPECT_FALSE(window->notifySizeChangeInCompatibleMode_.load());
    EXPECT_TRUE(window->notifyRectChangeInCompatibleMode_.load());
    GTEST_LOG_(INFO) << "WindowSessionImplCompatibleModeTest: SetAppHookWindowInfoAndUpdateRectSequence end";
}

/**
 * @tc.name: SetAppHookWindowInfoAndUpdateGlobalDisplayRectSequence
 * @tc.desc: Test sequence: SetAppHookWindowInfo in hidden state, then UpdateGlobalDisplayRectFromServer
 * @tc.type: FUNC
 */
HWTEST_F(WindowSessionImplCompatibleModeTest, SetAppHookWindowInfoAndUpdateGlobalDisplayRectSequence, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "WindowSessionImplCompatibleModeTest: "
        "SetAppHookWindowInfoAndUpdateGlobalDisplayRectSequence start";
    sptr<WindowOption> option = new (std::nothrow) WindowOption();
    ASSERT_NE(option, nullptr);
    option->SetWindowName("SetAppHookWindowInfoAndUpdateGlobalDisplayRectSequence");
    sptr<WindowSessionImpl> window = new (std::nothrow) WindowSessionImpl(option);
    ASSERT_NE(window, nullptr);

    window->property_->SetPersistentId(1011);
    window->state_ = WindowState::STATE_CREATED;
    window->property_->SetWindowRect({ 0, 0, 800, 600 });
    window->property_->SetGlobalDisplayRect({ 10, 20, 200, 100 });
    window->globalDisplayRectSizeChangeReason_ = SizeChangeReason::RESIZE;

    HookWindowInfo hookWindowInfo;
    hookWindowInfo.enableHookWindow = true;
    hookWindowInfo.widthHookRatio = 0.5f;
    hookWindowInfo.notifyWindowChange = true;

    window->SetAppHookWindowInfo(hookWindowInfo);

    EXPECT_TRUE(window->notifySizeChangeInCompatibleMode_.load());
    EXPECT_TRUE(window->notifyRectChangeInCompatibleMode_.load());

    WSRect rect = { 10, 20, 200, 100 };
    auto ret = window->UpdateGlobalDisplayRectFromServer(rect, SizeChangeReason::RESIZE);

    EXPECT_EQ(ret, WSError::WS_DO_NOTHING);
    EXPECT_TRUE(window->notifySizeChangeInCompatibleMode_.load());
    EXPECT_FALSE(window->notifyRectChangeInCompatibleMode_.load());
    GTEST_LOG_(INFO) << "WindowSessionImplCompatibleModeTest: "
        "SetAppHookWindowInfoAndUpdateGlobalDisplayRectSequence end";
}
}
}
}