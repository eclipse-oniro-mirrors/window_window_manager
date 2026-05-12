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

#include "mock_session.h"
#include "mock_window_adapter.h"
#include "singleton_mocker.h"
#include "window_scene_session_impl.h"

using namespace testing;
using namespace testing::ext;

namespace OHOS {
namespace Rosen {
using Mocker = SingletonMocker<WindowAdapter, MockWindowAdapter>;

class WindowSceneSessionImplRotationTest : public testing::Test {
public:
    static void SetUpTestCase();
    static void TearDownTestCase();
    void SetUp() override;
    void TearDown() override;

    std::unique_ptr<Mocker> mocker_ = std::make_unique<Mocker>();
};

void WindowSceneSessionImplRotationTest::SetUpTestCase() {}

void WindowSceneSessionImplRotationTest::TearDownTestCase() {}

void WindowSceneSessionImplRotationTest::SetUp() {}

void WindowSceneSessionImplRotationTest::TearDown() {}

namespace {
/**
 * @tc.name: CheckAndModifyWindowRect
 * @tc.desc: CheckAndModifyWindowRect
 * @tc.type: FUNC
 */
HWTEST_F(WindowSceneSessionImplRotationTest, CheckAndModifyWindowRect, Function | SmallTest | Level2)
{
    sptr<WindowOption> option = sptr<WindowOption>::MakeSptr();
    option->SetWindowName("CheckAndModifyWindowRect");
    sptr<WindowSceneSessionImpl> windowSceneSessionImpl = sptr<WindowSceneSessionImpl>::MakeSptr(option);
    SessionInfo sessionInfo = { "CreateTestBundle", "CreateTestModule", "CreateTestAbility" };
    sptr<SessionMocker> session = sptr<SessionMocker>::MakeSptr(sessionInfo);
    uint32_t width = 0;
    uint32_t height = 0;
    auto ret = windowSceneSessionImpl->CheckAndModifyWindowRect(width, height);
    EXPECT_EQ(WMError::WM_ERROR_INVALID_PARAM, ret);
    width = 100;
    height = 100;
    ret = windowSceneSessionImpl->CheckAndModifyWindowRect(width, height);
    EXPECT_EQ(WMError::WM_ERROR_INVALID_WINDOW, ret);

    windowSceneSessionImpl->property_->SetPersistentId(1);
    windowSceneSessionImpl->hostSession_ = session;
    windowSceneSessionImpl->state_ = WindowState::STATE_SHOWN;
    windowSceneSessionImpl->property_->SetWindowType(WindowType::WINDOW_TYPE_PIP);
    ret = windowSceneSessionImpl->CheckAndModifyWindowRect(width, height);
    EXPECT_EQ(WMError::WM_ERROR_INVALID_OPERATION, ret);
    windowSceneSessionImpl->property_->SetWindowType(WindowType::APP_SUB_WINDOW_BASE);
    windowSceneSessionImpl->property_->SetWindowMode(WindowMode::WINDOW_MODE_SPLIT_PRIMARY);
    ret = windowSceneSessionImpl->CheckAndModifyWindowRect(width, height);
    EXPECT_EQ(WMError::WM_ERROR_INVALID_OPERATION, ret);
    windowSceneSessionImpl->property_->SetWindowMode(WindowMode::WINDOW_MODE_FLOATING);
    ret = windowSceneSessionImpl->CheckAndModifyWindowRect(width, height);
    EXPECT_EQ(WMError::WM_OK, ret);
    windowSceneSessionImpl->property_->SetWindowMode(WindowMode::WINDOW_MODE_FULLSCREEN);
    windowSceneSessionImpl->property_->SetIsPcAppInPad(true);
    windowSceneSessionImpl->property_->SetWindowModeSupportType(WindowModeSupport::WINDOW_MODE_SUPPORT_FULLSCREEN);
    windowSceneSessionImpl->property_->SetDragEnabled(false);
    ret = windowSceneSessionImpl->CheckAndModifyWindowRect(width, height);
    EXPECT_EQ(WMError::WM_OK, ret);
    windowSceneSessionImpl->property_->SetDragEnabled(true);
    WindowLimits windowLimits = { 5000, 5000, 50, 50, 0.0f, 0.0f };
    windowSceneSessionImpl->property_->SetWindowLimits(windowLimits);
    ret = windowSceneSessionImpl->CheckAndModifyWindowRect(width, height);
    EXPECT_EQ(WMError::WM_OK, ret);
    WindowLimits windowLimits1 = { 800, 800, 50, 50, 0.0f, 0.0f };
    windowSceneSessionImpl->property_->SetWindowLimits(windowLimits1);
    ret = windowSceneSessionImpl->CheckAndModifyWindowRect(width, height);
    if (!windowSceneSessionImpl->IsFreeMultiWindowMode()) {
        EXPECT_EQ(WMError::WM_OK, ret);
    } else {
        EXPECT_EQ(WMError::WM_ERROR_INVALID_OPERATION, ret);
    }
}

/**
 * @tc.name: SetRotationLocked_InvalidWindow
 * @tc.desc: Test SetRotationLocked with invalid window state
 * @tc.type: FUNC
 */
HWTEST_F(WindowSceneSessionImplRotationTest, SetRotationLocked_InvalidWindow, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "WindowSceneSessionImplRotationTest: SetRotationLocked_InvalidWindow start";
    sptr<WindowOption> option = sptr<WindowOption>::MakeSptr();
    option->SetWindowName("SetRotationLocked_InvalidWindow");
    sptr<WindowSceneSessionImpl> window = sptr<WindowSceneSessionImpl>::MakeSptr(option);
    window->state_ = WindowState::STATE_DESTROYED;
    auto ret = window->SetRotationLocked(true);
    EXPECT_EQ(ret, WMError::WM_ERROR_INVALID_WINDOW);
    GTEST_LOG_(INFO) << "WindowSceneSessionImplRotationTest: SetRotationLocked_InvalidWindow end";
}

/**
 * @tc.name: SetRotationLocked_DeviceNotSupport
 * @tc.desc: Test SetRotationLocked with device not supporting rotation lock
 * @tc.type: FUNC
 */
HWTEST_F(WindowSceneSessionImplRotationTest, SetRotationLocked_DeviceNotSupport, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "WindowSceneSessionImplRotationTest: SetRotationLocked_DeviceNotSupport start";
    sptr<WindowOption> option = sptr<WindowOption>::MakeSptr();
    option->SetWindowName("SetRotationLocked_DeviceNotSupport");
    option->SetWindowType(WindowType::WINDOW_TYPE_STATUS_BAR);
    sptr<WindowSceneSessionImpl> window = sptr<WindowSceneSessionImpl>::MakeSptr(option);
    SessionInfo sessionInfo = { "TestBundle", "TestModule", "TestAbility" };
    sptr<SessionMocker> session = sptr<SessionMocker>::MakeSptr(sessionInfo);
    window->hostSession_ = session;
    window->property_->SetPersistentId(1);
    window->state_ = WindowState::STATE_CREATED;
    window->windowSystemConfig_.windowUIType_ = WindowUIType::INVALID_WINDOW;
    auto ret = window->SetRotationLocked(true);
    EXPECT_EQ(ret, WMError::WM_ERROR_DEVICE_NOT_SUPPORT);
    GTEST_LOG_(INFO) << "WindowSceneSessionImplRotationTest: SetRotationLocked_DeviceNotSupport end";
}

/**
 * @tc.name: SetRotationLocked_InvalidWindowType
 * @tc.desc: Test SetRotationLocked with non-system window type
 * @tc.type: FUNC
 */
HWTEST_F(WindowSceneSessionImplRotationTest, SetRotationLocked_InvalidWindowType, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "WindowSceneSessionImplRotationTest: SetRotationLocked_InvalidWindowType start";
    sptr<WindowOption> option = sptr<WindowOption>::MakeSptr();
    option->SetWindowName("SetRotationLocked_InvalidWindowType");
    option->SetWindowType(WindowType::WINDOW_TYPE_APP_MAIN_WINDOW);
    sptr<WindowSceneSessionImpl> window = sptr<WindowSceneSessionImpl>::MakeSptr(option);
    SessionInfo sessionInfo = { "TestBundle", "TestModule", "TestAbility" };
    sptr<SessionMocker> session = sptr<SessionMocker>::MakeSptr(sessionInfo);
    window->hostSession_ = session;
    window->property_->SetPersistentId(1);
    window->state_ = WindowState::STATE_CREATED;
    window->windowSystemConfig_.windowUIType_ = WindowUIType::PHONE_WINDOW;
    auto ret = window->SetRotationLocked(true);
    EXPECT_EQ(ret, WMError::WM_ERROR_INVALID_WINDOW_TYPE);
    GTEST_LOG_(INFO) << "WindowSceneSessionImplRotationTest: SetRotationLocked_InvalidWindowType end";
}

/**
 * @tc.name: SetRotationLocked_Success
 * @tc.desc: Test SetRotationLocked with valid configuration
 * @tc.type: FUNC
 */
HWTEST_F(WindowSceneSessionImplRotationTest, SetRotationLocked_Success, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "WindowSceneSessionImplRotationTest: SetRotationLocked_Success start";
    sptr<WindowOption> option = sptr<WindowOption>::MakeSptr();
    option->SetWindowName("SetRotationLocked_Success");
    option->SetWindowType(WindowType::WINDOW_TYPE_STATUS_BAR);
    sptr<WindowSceneSessionImpl> window = sptr<WindowSceneSessionImpl>::MakeSptr(option);
    SessionInfo sessionInfo = { "TestBundle", "TestModule", "TestAbility" };
    sptr<SessionMocker> session = sptr<SessionMocker>::MakeSptr(sessionInfo);
    window->hostSession_ = session;
    window->property_->SetPersistentId(1);
    window->state_ = WindowState::STATE_CREATED;
    window->windowSystemConfig_.windowUIType_ = WindowUIType::PHONE_WINDOW;
    auto ret = window->SetRotationLocked(true);
    EXPECT_EQ(ret, WMError::WM_OK);
    ret = window->SetRotationLocked(false);
    EXPECT_EQ(ret, WMError::WM_OK);
    GTEST_LOG_(INFO) << "WindowSceneSessionImplRotationTest: SetRotationLocked_Success end";
}

/**
 * @tc.name: GetRotationLocked_InvalidWindow
 * @tc.desc: Test GetRotationLocked with invalid window state
 * @tc.type: FUNC
 */
HWTEST_F(WindowSceneSessionImplRotationTest, GetRotationLocked_InvalidWindow, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "WindowSceneSessionImplRotationTest: GetRotationLocked_InvalidWindow start";
    sptr<WindowOption> option = sptr<WindowOption>::MakeSptr();
    option->SetWindowName("GetRotationLocked_InvalidWindow");
    sptr<WindowSceneSessionImpl> window = sptr<WindowSceneSessionImpl>::MakeSptr(option);
    window->state_ = WindowState::STATE_DESTROYED;
    bool locked = false;
    auto ret = window->GetRotationLocked(locked);
    EXPECT_EQ(ret, WMError::WM_ERROR_INVALID_WINDOW);
    GTEST_LOG_(INFO) << "WindowSceneSessionImplRotationTest: GetRotationLocked_InvalidWindow end";
}

/**
 * @tc.name: GetRotationLocked_DeviceNotSupport
 * @tc.desc: Test GetRotationLocked with device not supporting rotation lock
 * @tc.type: FUNC
 */
HWTEST_F(WindowSceneSessionImplRotationTest, GetRotationLocked_DeviceNotSupport, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "WindowSceneSessionImplRotationTest: GetRotationLocked_DeviceNotSupport start";
    sptr<WindowOption> option = sptr<WindowOption>::MakeSptr();
    option->SetWindowName("GetRotationLocked_DeviceNotSupport");
    option->SetWindowType(WindowType::WINDOW_TYPE_STATUS_BAR);
    sptr<WindowSceneSessionImpl> window = sptr<WindowSceneSessionImpl>::MakeSptr(option);
    SessionInfo sessionInfo = { "TestBundle", "TestModule", "TestAbility" };
    sptr<SessionMocker> session = sptr<SessionMocker>::MakeSptr(sessionInfo);
    window->hostSession_ = session;
    window->property_->SetPersistentId(1);
    window->state_ = WindowState::STATE_CREATED;
    window->windowSystemConfig_.windowUIType_ = WindowUIType::INVALID_WINDOW;
    bool locked = false;
    auto ret = window->GetRotationLocked(locked);
    EXPECT_EQ(ret, WMError::WM_ERROR_DEVICE_NOT_SUPPORT);
    GTEST_LOG_(INFO) << "WindowSceneSessionImplRotationTest: GetRotationLocked_DeviceNotSupport end";
}

/**
 * @tc.name: GetRotationLocked_InvalidWindowType
 * @tc.desc: Test GetRotationLocked with non-system window type
 * @tc.type: FUNC
 */
HWTEST_F(WindowSceneSessionImplRotationTest, GetRotationLocked_InvalidWindowType, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "WindowSceneSessionImplRotationTest: GetRotationLocked_InvalidWindowType start";
    sptr<WindowOption> option = sptr<WindowOption>::MakeSptr();
    option->SetWindowName("GetRotationLocked_InvalidWindowType");
    option->SetWindowType(WindowType::WINDOW_TYPE_APP_MAIN_WINDOW);
    sptr<WindowSceneSessionImpl> window = sptr<WindowSceneSessionImpl>::MakeSptr(option);
    SessionInfo sessionInfo = { "TestBundle", "TestModule", "TestAbility" };
    sptr<SessionMocker> session = sptr<SessionMocker>::MakeSptr(sessionInfo);
    window->hostSession_ = session;
    window->property_->SetPersistentId(1);
    window->state_ = WindowState::STATE_CREATED;
    window->windowSystemConfig_.windowUIType_ = WindowUIType::PHONE_WINDOW;
    bool locked = false;
    auto ret = window->GetRotationLocked(locked);
    EXPECT_EQ(ret, WMError::WM_ERROR_INVALID_WINDOW_TYPE);
    GTEST_LOG_(INFO) << "WindowSceneSessionImplRotationTest: GetRotationLocked_InvalidWindowType end";
}

/**
 * @tc.name: GetRotationLocked_Success
 * @tc.desc: Test GetRotationLocked with valid configuration
 * @tc.type: FUNC
 */
HWTEST_F(WindowSceneSessionImplRotationTest, GetRotationLocked_Success, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "WindowSceneSessionImplRotationTest: GetRotationLocked_Success start";
    sptr<WindowOption> option = sptr<WindowOption>::MakeSptr();
    option->SetWindowName("GetRotationLocked_Success");
    option->SetWindowType(WindowType::WINDOW_TYPE_STATUS_BAR);
    sptr<WindowSceneSessionImpl> window = sptr<WindowSceneSessionImpl>::MakeSptr(option);
    SessionInfo sessionInfo = { "TestBundle", "TestModule", "TestAbility" };
    sptr<SessionMocker> session = sptr<SessionMocker>::MakeSptr(sessionInfo);
    window->hostSession_ = session;
    window->property_->SetPersistentId(1);
    window->state_ = WindowState::STATE_CREATED;
    window->windowSystemConfig_.windowUIType_ = WindowUIType::PHONE_WINDOW;
    window->property_->SetRotationLocked(true);
    bool locked = false;
    auto ret = window->GetRotationLocked(locked);
    EXPECT_EQ(ret, WMError::WM_OK);
    EXPECT_EQ(locked, true);
    window->property_->SetRotationLocked(false);
    ret = window->GetRotationLocked(locked);
    EXPECT_EQ(ret, WMError::WM_OK);
    EXPECT_EQ(locked, false);
    GTEST_LOG_(INFO) << "WindowSceneSessionImplRotationTest: GetRotationLocked_Success end";
}
} // namespace
} // namespace Rosen
} // namespace OHOS
