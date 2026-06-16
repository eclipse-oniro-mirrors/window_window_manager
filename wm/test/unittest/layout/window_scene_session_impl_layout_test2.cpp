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

#include "ability_context_impl.h"
#include "mock_session.h"
#include "window_scene_session_impl.h"
#include "window_session_impl.h"
#include "window_manager_hilog.h"
#include "wm_common.h"

using namespace testing;
using namespace testing::ext;

namespace OHOS {
namespace Rosen {

class WindowSceneSessionImplLayoutTest2 : public testing::Test {
public:
    static void SetUpTestCase();
    static void TearDownTestCase();
    void SetUp() override;
    void TearDown() override;

    std::shared_ptr<AbilityRuntime::AbilityContextImpl> abilityContext_;

private:
    static constexpr uint32_t WAIT_SYNC_IN_NS = 200000;
};

void WindowSceneSessionImplLayoutTest2::SetUpTestCase() {}

void WindowSceneSessionImplLayoutTest2::TearDownTestCase() {}

void WindowSceneSessionImplLayoutTest2::SetUp()
{
    abilityContext_ = std::make_shared<AbilityRuntime::AbilityContextImpl>();
}

void WindowSceneSessionImplLayoutTest2::TearDown()
{
    usleep(WAIT_SYNC_IN_NS);
    abilityContext_ = nullptr;
}

/**
 * @tc.name: UpdateSupportWindowModesWhenSwitchFreeMultiWindow01
 * @tc.desc: freeMultiWindow enabled with valid supported modes and hostSession, uses property modes
 * @tc.type: FUNC
 */
HWTEST_F(WindowSceneSessionImplLayoutTest2, UpdateSupportWindowModesWhenSwitchFreeMultiWindow01,
    Function | SmallTest | Level2)
{
    sptr<WindowOption> option = sptr<WindowOption>::MakeSptr();
    option->SetWindowName("UpdateSupportWindowModesWhenSwitchFreeMultiWindow01");
    sptr<WindowSceneSessionImpl> window = sptr<WindowSceneSessionImpl>::MakeSptr(option);

    auto abilityInfo = std::make_shared<AppExecFwk::AbilityInfo>();
    abilityContext_->SetAbilityInfo(abilityInfo);
    window->context_ = abilityContext_;
    window->windowSystemConfig_.freeMultiWindowEnable_ = true;

    std::vector<AppExecFwk::SupportWindowMode> modes = {
        AppExecFwk::SupportWindowMode::FULLSCREEN,
        AppExecFwk::SupportWindowMode::FLOATING,
    };
    window->property_->SetSupportedWindowModes(modes);

    SessionInfo sessionInfo = { "TestBundle", "TestModule", "TestAbility" };
    sptr<SessionMocker> session = sptr<SessionMocker>::MakeSptr(sessionInfo);
    window->hostSession_ = session;

    window->UpdateSupportWindowModesWhenSwitchFreeMultiWindow();
    uint32_t expected = WindowModeSupport::WINDOW_MODE_SUPPORT_FULLSCREEN |
        WindowModeSupport::WINDOW_MODE_SUPPORT_FLOATING;
    EXPECT_EQ(window->property_->GetWindowModeSupportType(), expected);
}

/**
 * @tc.name: UpdateSupportWindowModesWhenSwitchFreeMultiWindow02
 * @tc.desc: freeMultiWindow disabled, falls back to metadata extraction
 * @tc.type: FUNC
 */
HWTEST_F(WindowSceneSessionImplLayoutTest2, UpdateSupportWindowModesWhenSwitchFreeMultiWindow02,
    Function | SmallTest | Level2)
{
    sptr<WindowOption> option = sptr<WindowOption>::MakeSptr();
    option->SetWindowName("UpdateSupportWindowModesWhenSwitchFreeMultiWindow02");
    sptr<WindowSceneSessionImpl> window = sptr<WindowSceneSessionImpl>::MakeSptr(option);

    auto abilityInfo = std::make_shared<AppExecFwk::AbilityInfo>();
    abilityContext_->SetAbilityInfo(abilityInfo);
    window->context_ = abilityContext_;
    window->windowSystemConfig_.freeMultiWindowEnable_ = false;

    std::vector<AppExecFwk::SupportWindowMode> modes = {
        AppExecFwk::SupportWindowMode::FULLSCREEN,
    };
    window->property_->SetSupportedWindowModes(modes);

    window->UpdateSupportWindowModesWhenSwitchFreeMultiWindow();
    EXPECT_EQ(window->property_->GetWindowModeSupportType(), 0u);
}

/**
 * @tc.name: UpdateSupportWindowModesWhenSwitchFreeMultiWindow03
 * @tc.desc: freeMultiWindow enabled but supported modes empty, falls back to metadata
 * @tc.type: FUNC
 */
HWTEST_F(WindowSceneSessionImplLayoutTest2, UpdateSupportWindowModesWhenSwitchFreeMultiWindow03,
    Function | SmallTest | Level2)
{
    sptr<WindowOption> option = sptr<WindowOption>::MakeSptr();
    option->SetWindowName("UpdateSupportWindowModesWhenSwitchFreeMultiWindow03");
    sptr<WindowSceneSessionImpl> window = sptr<WindowSceneSessionImpl>::MakeSptr(option);

    auto abilityInfo = std::make_shared<AppExecFwk::AbilityInfo>();
    abilityContext_->SetAbilityInfo(abilityInfo);
    window->context_ = abilityContext_;
    window->windowSystemConfig_.freeMultiWindowEnable_ = true;
    // modes left empty (default)

    window->UpdateSupportWindowModesWhenSwitchFreeMultiWindow();
    EXPECT_EQ(window->property_->GetWindowModeSupportType(), 0u);
}

/**
 * @tc.name: UpdateSupportWindowModesWhenSwitchFreeMultiWindow04
 * @tc.desc: freeMultiWindow enabled with modes but hostSession is null, no crash
 * @tc.type: FUNC
 */
HWTEST_F(WindowSceneSessionImplLayoutTest2, UpdateSupportWindowModesWhenSwitchFreeMultiWindow04,
    Function | SmallTest | Level2)
{
    sptr<WindowOption> option = sptr<WindowOption>::MakeSptr();
    option->SetWindowName("UpdateSupportWindowModesWhenSwitchFreeMultiWindow04");
    sptr<WindowSceneSessionImpl> window = sptr<WindowSceneSessionImpl>::MakeSptr(option);

    auto abilityInfo = std::make_shared<AppExecFwk::AbilityInfo>();
    abilityContext_->SetAbilityInfo(abilityInfo);
    window->context_ = abilityContext_;
    window->windowSystemConfig_.freeMultiWindowEnable_ = true;
    // hostSession_ is null by default

    std::vector<AppExecFwk::SupportWindowMode> modes = {
        AppExecFwk::SupportWindowMode::FLOATING,
    };
    window->property_->SetSupportedWindowModes(modes);

    window->UpdateSupportWindowModesWhenSwitchFreeMultiWindow();
    EXPECT_EQ(window->property_->GetWindowModeSupportType(),
        static_cast<uint32_t>(WindowModeSupport::WINDOW_MODE_SUPPORT_FLOATING));
}

} // namespace Rosen
} // namespace OHOS
