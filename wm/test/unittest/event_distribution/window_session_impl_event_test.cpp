/*
 * Copyright (c) 2023 Huawei Device Co., Ltd.
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

#include <filesystem>
#include <fstream>
#include <gtest/gtest.h>
#include <int_wrapper.h>
#include <want_params_wrapper.h>

#include "ability_context_impl.h"
#include "application_context.h"
#include "color_parser.h"
#include "extension/extension_business_info.h"
#include "mock_session.h"
#include "parameters.h"
#include "window_helper.h"
#include "window_session_impl.h"
#include "wm_common.h"
#include "window_manager_hilog.h"

using namespace testing;
using namespace testing::ext;

namespace OHOS {
namespace Rosen {
class WindowSessionImplEventTest : public testing::Test {
public:
    static void SetUpTestCase();
    static void TearDownTestCase();
    void SetUp() override;
    void TearDown() override;

    std::shared_ptr<AbilityRuntime::AbilityContext> abilityContext_;
};

void WindowSessionImplEventTest::SetUpTestCase() {}

void WindowSessionImplEventTest::TearDownTestCase() {}

void WindowSessionImplEventTest::SetUp()
{
    abilityContext_ = std::make_shared<AbilityRuntime::AbilityContextImpl>();
}

void WindowSessionImplEventTest::TearDown()
{
    abilityContext_ = nullptr;
}

namespace {
/**
 * @tc.name: SetTouchable
 * @tc.desc: SetTouchable
 * @tc.type: FUNC
 */
HWTEST_F(WindowSessionImplEventTest, SetTouchable, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "WindowSessionImplTest: SetTouchable start";
    sptr<WindowOption> option = sptr<WindowOption>::MakeSptr();
    option->SetWindowName("SetTouchable");
    sptr<WindowSessionImpl> window = sptr<WindowSessionImpl>::MakeSptr(option);

    SessionInfo sessionInfo = { "CreateTestBundle", "CreateTestModule", "CreateTestAbility" };
    sptr<SessionMocker> session = new (std::nothrow) SessionMocker(sessionInfo);
    ASSERT_NE(nullptr, session);
    ASSERT_EQ(WMError::WM_OK, window->Create(nullptr, session));
    ASSERT_NE(window->property_, nullptr);
    window->hostSession_ = session;
    window->property_->SetPersistentId(1);
    ASSERT_FALSE(window->IsWindowSessionInvalid());
    WMError res = window->SetTouchable(true);
    ASSERT_EQ(res, WMError::WM_OK);
    ASSERT_NE(window->property_, nullptr);
    ASSERT_TRUE(window->property_->touchable_);
    ASSERT_EQ(WMError::WM_OK, window->Destroy());

    // session is null
    window = sptr<WindowSessionImpl>::MakeSptr(option);
    ASSERT_EQ(WMError::WM_OK, window->Create(abilityContext_, nullptr));
    res = window->SetTouchable(true);
    ASSERT_EQ(res, WMError::WM_ERROR_INVALID_WINDOW);
    res = window->SetTouchable(false);
    ASSERT_EQ(res, WMError::WM_ERROR_INVALID_WINDOW);
    ASSERT_EQ(WMError::WM_ERROR_INVALID_WINDOW, window->Destroy());
    GTEST_LOG_(INFO) << "WindowSessionImplTest: SetTouchable end";
}
}
}
}