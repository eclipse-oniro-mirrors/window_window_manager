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
#include "session_manager/include/session_manager_agent_controller.h"
#include "window_manager_agent.h"
#include "zidl/window_manager_agent_interface.h"

using namespace testing;
using namespace testing::ext;

namespace OHOS {
namespace Rosen {
class SessionManagerAgentControllerMultiUserTest : public testing::Test {
public:
    static void SetUpTestCase();
    static void TearDownTestCase();
    void SetUp() override;
    void TearDown() override;
};

void SessionManagerAgentControllerMultiUserTest::SetUpTestCase() {}

void SessionManagerAgentControllerMultiUserTest::TearDownTestCase() {}

void SessionManagerAgentControllerMultiUserTest::SetUp() {}

void SessionManagerAgentControllerMultiUserTest::TearDown() {}

/**
 * @tc.name: RegisterMultiUser01
 * @tc.desc: Same pid, different userId, same type: both agents registered independently
 * @tc.type: FUNC
 */
HWTEST_F(SessionManagerAgentControllerMultiUserTest, RegisterMultiUser01, TestSize.Level1)
{
    int32_t pid = 65535;
    int32_t userId100 = 100;
    int32_t userId200 = 200;
    WindowManagerAgentType type = WindowManagerAgentType::WINDOW_MANAGER_AGENT_TYPE_FOCUS;
    sptr<IWindowManagerAgent> agent100 = sptr<WindowManagerAgent>::MakeSptr(userId100);
    sptr<IWindowManagerAgent> agent200 = sptr<WindowManagerAgent>::MakeSptr(userId200);
    auto& ctrl = SessionManagerAgentController::GetInstance();
    ASSERT_EQ(WMError::WM_OK,
        ctrl.RegisterWindowManagerAgent(agent100, type, pid, userId100));
    ASSERT_EQ(WMError::WM_OK,
        ctrl.RegisterWindowManagerAgent(agent200, type, pid, userId200));
    ASSERT_EQ(WMError::WM_OK,
        ctrl.UnregisterWindowManagerAgent(agent100, type, pid, userId100));
    ASSERT_EQ(WMError::WM_OK,
        ctrl.UnregisterWindowManagerAgent(agent200, type, pid, userId200));
}

/**
 * @tc.name: RegisterReplaceOld01
 * @tc.desc: Same (pid, userId, type) register twice: old agent replaced by new
 * @tc.type: FUNC
 */
HWTEST_F(SessionManagerAgentControllerMultiUserTest, RegisterReplaceOld01, TestSize.Level1)
{
    int32_t pid = 65535;
    int32_t instanceUserId = 0;
    WindowManagerAgentType type = WindowManagerAgentType::WINDOW_MANAGER_AGENT_TYPE_FOCUS;
    sptr<IWindowManagerAgent> agentOld = sptr<WindowManagerAgent>::MakeSptr();
    sptr<IWindowManagerAgent> agentNew = sptr<WindowManagerAgent>::MakeSptr();
    auto& ctrl = SessionManagerAgentController::GetInstance();
    ASSERT_EQ(WMError::WM_OK,
        ctrl.RegisterWindowManagerAgent(agentOld, type, pid, instanceUserId));
    ASSERT_EQ(WMError::WM_OK,
        ctrl.RegisterWindowManagerAgent(agentNew, type, pid, instanceUserId));
    ASSERT_EQ(WMError::WM_OK,
        ctrl.UnregisterWindowManagerAgent(agentNew, type, pid, instanceUserId));
}

/**
 * @tc.name: UnregisterMultiUser01
 * @tc.desc: Unregister one user's agent, verify the other user's agent still works
 * @tc.type: FUNC
 */
HWTEST_F(SessionManagerAgentControllerMultiUserTest, UnregisterMultiUser01, TestSize.Level1)
{
    int32_t pid = 65535;
    int32_t userId100 = 100;
    int32_t userId200 = 200;
    WindowManagerAgentType type = WindowManagerAgentType::WINDOW_MANAGER_AGENT_TYPE_FOCUS;
    sptr<IWindowManagerAgent> agent100 = sptr<WindowManagerAgent>::MakeSptr(userId100);
    sptr<IWindowManagerAgent> agent200 = sptr<WindowManagerAgent>::MakeSptr(userId200);
    auto& ctrl = SessionManagerAgentController::GetInstance();
    ASSERT_EQ(WMError::WM_OK,
        ctrl.RegisterWindowManagerAgent(agent100, type, pid, userId100));
    ASSERT_EQ(WMError::WM_OK,
        ctrl.RegisterWindowManagerAgent(agent200, type, pid, userId200));
    ASSERT_EQ(WMError::WM_OK,
        ctrl.UnregisterWindowManagerAgent(agent100, type, pid, userId100));
    sptr<FocusChangeInfo> focusChangeInfo = sptr<FocusChangeInfo>::MakeSptr();
    ctrl.UpdateFocusChangeInfo(focusChangeInfo, true);
    ASSERT_EQ(WMError::WM_OK,
        ctrl.UnregisterWindowManagerAgent(agent200, type, pid, userId200));
}

} // namespace Rosen
} // namespace OHOS
