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
} // namespace
} // namespace Rosen
} // namespace OHOS
