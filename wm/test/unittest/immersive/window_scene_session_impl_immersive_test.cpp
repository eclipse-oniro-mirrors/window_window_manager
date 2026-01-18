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

#include "ability_context_impl.h"
#include "display_info.h"
#include "mock_session.h"
#include "mock_uicontent.h"
#include "mock_window_adapter.h"
#include "mock_window_scene_session_impl.h"
#include "scene_board_judgement.h"
#include "session/host/include/scene_session.h"
#include "window_scene_session_impl.h"
#include "window_session_impl.h"

using namespace testing;
using namespace testing::ext;

namespace OHOS {
namespace Rosen {

class WindowSceneSessionImplImmersiveTest : public testing::Test {
public:
    static void SetUpTestCase();
    static void TearDownTestCase();
    void SetUp() override;
    void TearDown() override;

    std::shared_ptr<AbilityRuntime::AbilityContext> abilityContext_;

private:
    static constexpr uint32_t WAIT_SYNC_IN_NS = 200000;
    static constexpr uint32_t WAIT_SERVERAL_FRAMES = 36000;
};

void WindowSceneSessionImplImmersiveTest::SetUpTestCase() {}

void WindowSceneSessionImplImmersiveTest::TearDownTestCase() {}

void WindowSceneSessionImplImmersiveTest::SetUp()
{
    abilityContext_ = std::make_shared<AbilityRuntime::AbilityContextImpl>();
}

void WindowSceneSessionImplImmersiveTest::TearDown()
{
    usleep(WAIT_SYNC_IN_NS);
    abilityContext_ = nullptr;
}

namespace {

/**
 * @tc.name: GetAvoidAreaByType
 * @tc.desc: GetAvoidAreaByType test
 * @tc.type: FUNC
 */
HWTEST_F(WindowSceneSessionImplImmersiveTest, GetAvoidAreaByType, TestSize.Level1)
{
    sptr<WindowOption> option = sptr<WindowOption>::MakeSptr();
    option->SetWindowMode(WindowMode::WINDOW_MODE_PIP);
    option->SetWindowName("GetAvoidAreaByType");
    sptr<WindowSceneSessionImpl> window = sptr<WindowSceneSessionImpl>::MakeSptr(option);

    window->property_->SetPersistentId(1);
    SessionInfo sessionInfo = { "CreateTestBundle", "CreateTestModule", "CreateTestAbility" };
    sptr<SessionMocker> session = sptr<SessionMocker>::MakeSptr(sessionInfo);
    AvoidArea avoidarea;
    ASSERT_EQ(WMError::WM_ERROR_INVALID_WINDOW, window->GetAvoidAreaByType(AvoidAreaType::TYPE_CUTOUT, avoidarea));
    window->hostSession_ = session;
    ASSERT_EQ(WMError::WM_OK, window->GetAvoidAreaByType(AvoidAreaType::TYPE_CUTOUT, avoidarea));
    window->property_->SetWindowType(WindowType::WINDOW_TYPE_FLOAT);
    Rect rect;
    ASSERT_EQ(WMError::WM_OK, window->GetAvoidAreaByType(AvoidAreaType::TYPE_CUTOUT, avoidarea, rect, 15));
    ASSERT_EQ(WMError::WM_OK, window->GetAvoidAreaByType(AvoidAreaType::TYPE_CUTOUT, avoidarea, rect, 16));
}
} // namespace
} // namespace Rosen
} // namespace OHOS
