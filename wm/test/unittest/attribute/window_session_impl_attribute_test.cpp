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

#include "mock_session.h"
#include "vsync_station.h"
#include "window_session_impl.h"
#include "wm_common.h"

using namespace testing;
using namespace testing::ext;

namespace OHOS {
namespace Rosen {
class WindowSessionImplAttributeTest : public testing::Test {
public:
    static void SetUpTestCase();
    static void TearDownTestCase();
    void SetUp() override;
    void TearDown() override;

    sptr<WindowSessionImpl> window_;
};

void WindowSessionImplAttributeTest::SetUpTestCase() {}

void WindowSessionImplAttributeTest::TearDownTestCase() {}

void WindowSessionImplAttributeTest::SetUp() {}

void WindowSessionImplAttributeTest::TearDown()
{
    if (window_ != nullptr) {
        window_->Destroy();
    }
}

namespace {
sptr<WindowSessionImpl> GetTestWindowImpl(const std::string& name)
{
    sptr<WindowOption> option = sptr<WindowOption>::MakeSptr();
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
    window->property_->SetPersistentId(1);
    window->vsyncStation_ = std::make_shared<VsyncStation>(0);
    return window;
}
} // namespace

/**
 * @tc.name: SetUiDvsyncSwitchFromApiNotForeground
 * @tc.desc: API enable is ignored when window is not foreground.
 * @tc.type: FUNC
 */
HWTEST_F(WindowSessionImplAttributeTest, SetUiDvsyncSwitchFromApiNotForeground, TestSize.Level1)
{
    window_ = GetTestWindowImpl("SetUiDvsyncSwitchFromApiNotForeground");
    ASSERT_NE(window_, nullptr);
    window_->state_ = WindowState::STATE_HIDDEN;
    window_->isFocused_ = true;
    window_->SetUiDvsyncSwitch(true, FromWhom::API);
    ASSERT_FALSE(window_->isUiDvsyncSwitchApiEnabled_.load());
}

/**
 * @tc.name: SetUiDvsyncSwitchFromApiNotFocused
 * @tc.desc: API enable is ignored when window is foreground but not focused.
 * @tc.type: FUNC
 */
HWTEST_F(WindowSessionImplAttributeTest, SetUiDvsyncSwitchFromApiNotFocused, TestSize.Level1)
{
    window_ = GetTestWindowImpl("SetUiDvsyncSwitchFromApiNotFocused");
    ASSERT_NE(window_, nullptr);
    window_->state_ = WindowState::STATE_SHOWN;
    window_->isFocused_ = false;
    window_->SetUiDvsyncSwitch(true, FromWhom::API);
    ASSERT_FALSE(window_->isUiDvsyncSwitchApiEnabled_.load());
}

/**
 * @tc.name: SetUiDvsyncSwitchFromApiEnableAndDisable
 * @tc.desc: API enable only takes effect when foreground and focused, and disable clears it.
 * @tc.type: FUNC
 */
HWTEST_F(WindowSessionImplAttributeTest, SetUiDvsyncSwitchFromApiEnableAndDisable, TestSize.Level1)
{
    window_ = GetTestWindowImpl("SetUiDvsyncSwitchFromApiEnableAndDisable");
    ASSERT_NE(window_, nullptr);
    window_->state_ = WindowState::STATE_SHOWN;
    window_->isFocused_ = true;
    window_->SetUiDvsyncSwitch(true, FromWhom::API);
    ASSERT_TRUE(window_->isUiDvsyncSwitchApiEnabled_.load());
    window_->SetUiDvsyncSwitch(false, FromWhom::API);
    ASSERT_FALSE(window_->isUiDvsyncSwitchApiEnabled_.load());
}

/**
 * @tc.name: SetUiDvsyncSwitchFromInner
 * @tc.desc: INNER path does not affect the API enable state.
 * @tc.type: FUNC
 */
HWTEST_F(WindowSessionImplAttributeTest, SetUiDvsyncSwitchFromInner, TestSize.Level1)
{
    window_ = GetTestWindowImpl("SetUiDvsyncSwitchFromInner");
    ASSERT_NE(window_, nullptr);
    window_->state_ = WindowState::STATE_HIDDEN;
    window_->isFocused_ = false;
    window_->SetUiDvsyncSwitch(true, FromWhom::INNER);
    window_->SetUiDvsyncSwitch(false, FromWhom::INNER);
    ASSERT_FALSE(window_->isUiDvsyncSwitchApiEnabled_.load());
}

/**
 * @tc.name: DisableUiDvsyncSwitchApiIfNeeded
 * @tc.desc: Disabling by reason clears the enable state and is idempotent.
 * @tc.type: FUNC
 */
HWTEST_F(WindowSessionImplAttributeTest, DisableUiDvsyncSwitchApiIfNeeded, TestSize.Level1)
{
    window_ = GetTestWindowImpl("DisableUiDvsyncSwitchApiIfNeeded");
    ASSERT_NE(window_, nullptr);
    window_->isUiDvsyncSwitchApiEnabled_.store(true);
    window_->DisableUiDvsyncSwitchApiIfNeeded(DvsyncDisableReason::UNFOCUS_DISABLE);
    ASSERT_FALSE(window_->isUiDvsyncSwitchApiEnabled_.load());
    window_->DisableUiDvsyncSwitchApiIfNeeded(DvsyncDisableReason::BACKGROUND_DISABLE);
    ASSERT_FALSE(window_->isUiDvsyncSwitchApiEnabled_.load());
}
} // namespace Rosen
} // namespace OHOS
