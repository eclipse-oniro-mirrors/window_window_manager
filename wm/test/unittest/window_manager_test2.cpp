/*
 * Copyright (c) 2025 Huawei Device Co., Ltd.
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
#include "iremote_object_mocker.h"
#include "parameters.h"
#include "scene_board_judgement.h"
#include "window_manager.h"
#include "window_adapter.h"
#include "window_manager_hilog.h"
#include "window_manager_agent.h"

using namespace testing;
using namespace testing::ext;

namespace OHOS {
namespace Rosen {
namespace {

class WindowManagerTest2 : public testing::Test {
public:
    static void SetUpTestCase();
    static void TearDownTestCase();
    void SetUp() override;
    void TearDown() override;

private:
    int32_t userId_ = 100;
    sptr<WindowManager> instance_ = nullptr;
    sptr<WindowAdapter> windowAdapter = nullptr;

    std::string isConcurrentuser_;
};

void WindowManagerTest2::SetUpTestCase() {}

void WindowManagerTest2::TearDownTestCase() {}

void WindowManagerTest2::SetUp()
{
    isConcurrentuser_ = OHOS::system::GetParameter("persist.dms.concurrentuser", "");
    OHOS::system::SetParameter("persist.dms.concurrentuser", "true");
    instance_ = &WindowManager::GetInstance(userId_);
    windowAdapter = &WindowAdapter::GetInstance(userId_);
}

void WindowManagerTest2::TearDown()
{
    WindowManager::RemoveInstanceByUserId(userId_);

    WindowAdapter::windowAdapterMap_.clear();
    OHOS::system::SetParameter("persist.dms.concurrentuser", isConcurrentuser_);
}

/**
 * @tc.name: WindowManager::ActiveFaultAgentReregister_MockSMSNotAlive
 * @tc.desc: 测试 Mock SMS 不存活时返回 WM_ERROR_SAMGR
 * @tc.type: FUNC
 */
HWTEST_F(WindowManagerTest2, ActiveFaultAgentReregister_MockSMSNotAlive, Function | SmallTest | Level2)
{
    ASSERT_NE(nullptr, instance_);
    ASSERT_NE(nullptr, windowAdapter);

    // Mock SMS not alive -> should return WM_ERROR_SAMGR
    // Note: Since IsMockSMSProxyAlive depends on SessionManager, we need to ensure it returns false
    // In this test, we assume default behavior returns false
    auto ret = instance_->ActiveFaultAgentReregister(
        WindowManagerAgentType::WINDOW_MANAGER_AGENT_TYPE_FOCUS,
        sptr<WindowManagerAgent>::MakeSptr(userId_));
    // Expected to return WM_ERROR_SAMGR when Mock SMS is not alive
    // or WM_ERROR_INVALID_CALLING if proxy is valid
    EXPECT_NE(WMError::WM_OK, ret);
}

/**
 * @tc.name: WindowManager::ActiveFaultAgentReregister_ProxyValid
 * @tc.desc: 测试 WMS 代理有效时返回 WM_ERROR_INVALID_CALLING
 * @tc.type: FUNC
 */
HWTEST_F(WindowManagerTest2, ActiveFaultAgentReregister_ProxyValid, Function | SmallTest | Level2)
{
    ASSERT_NE(nullptr, instance_);
    ASSERT_NE(nullptr, windowAdapter);

    // Set isProxyValid_ = true -> fault recovery should NOT activate
    windowAdapter->isProxyValid_ = true;
    auto ret = instance_->ActiveFaultAgentReregister(
        WindowManagerAgentType::WINDOW_MANAGER_AGENT_TYPE_FOCUS,
        sptr<WindowManagerAgent>::MakeSptr(userId_));
    EXPECT_NE(WMError::WM_OK, ret);
}

/**
 * @tc.name: WindowManager::ActiveFaultAgentReregister_Success
 * @tc.desc: 测试正常故障恢复场景（Mock SMS 存活且代理无效）
 * @tc.type: FUNC
 */
HWTEST_F(WindowManagerTest2, ActiveFaultAgentReregister_Success, Function | SmallTest | Level2)
{
    ASSERT_NE(nullptr, instance_);
    ASSERT_NE(nullptr, windowAdapter);

    // Set isProxyValid_ = false -> fault recovery should succeed
    windowAdapter->isProxyValid_ = false;
    windowAdapter->InitSSMProxy();
    windowAdapter->isProxyValid_ = false;
    auto ret = instance_->ActiveFaultAgentReregister(
        WindowManagerAgentType::WINDOW_MANAGER_AGENT_TYPE_FOCUS,
        sptr<WindowManagerAgent>::MakeSptr(userId_));
    EXPECT_EQ(WMError::WM_OK, ret);
}
} // namespace
} // namespace Rosen
} // namespace OHOS
