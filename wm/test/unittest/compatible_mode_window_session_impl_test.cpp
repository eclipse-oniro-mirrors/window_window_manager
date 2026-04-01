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

#include "mock_session.h"
#include "mock_session_stub.h"
#include "mock_uicontent.h"
#include "mock_window.h"
#include "parameters.h"
#include "window_helper.h"
#include "window_scene_session_impl.h"
#include "wm_common.h"
#include "window_manager_hilog.h"

using namespace testing;
using namespace testing::ext;

namespace OHOS {
namespace Rosen {
std::string g_errLog;
void MyLogCallback(const LogType type, const LogLevel level, const unsigned int domain, const char *tag,
    const char *msg)
{
    g_errLog = msg;
}

class CompatibleModeWindowSessionImplTest : public testing::Test {
public:
    static void SetUpTestCase();
    static void TearDownTestCase();
    void SetUp() override;
    void TearDown() override;

private:
    static constexpr uint32_t WAIT_SYNC_IN_NS = 500000;
};

void CompatibleModeWindowSessionImplTest::SetUpTestCase()
{
    GTEST_LOG_(INFO) << "setup test case";
}

void CompatibleModeWindowSessionImplTest::TearDownTestCase()
{
    GTEST_LOG_(INFO) << "teardown test case";
}

void CompatibleModeWindowSessionImplTest::SetUp()
{
    GTEST_LOG_(INFO) << "setup";
}

void CompatibleModeWindowSessionImplTest::TearDown()
{
    GTEST_LOG_(INFO) << "teardown";
}

namespace {

/**
 * @tc.name: RegisterUIContentCreateListener
 * @tc.desc: RegisterUIContentCreateListenerWithNullListener
 * @tc.type: FUNC
 */
HWTEST_F(CompatibleModeWindowSessionImplTest, RegisterUIContentCreateListenerWithNullListener,
    TestSize.Level1)
{
    GTEST_LOG_(INFO) << "RegisterUIContentCreateListenerWithNullListener test start";
    sptr<WindowOption> option = sptr<WindowOption>::MakeSptr();
    sptr<WindowSessionImpl> window = sptr<WindowSessionImpl>::MakeSptr(option);
    sptr<IUIContentCreateListener> listener = nullptr;
    auto ret = window->RegisterUIContentCreateListener(listener);
    EXPECT_EQ(ret, WMError::WM_ERROR_NULLPTR);
    GTEST_LOG_(INFO) << "RegisterUIContentCreateListenerWithNullListener test end";
}

/**
 * @tc.name: RegisterUIContentCreateListener
 * @tc.desc: RegisterUIContentCreateListenerWithNotNullListener
 * @tc.type: FUNC
 */
HWTEST_F(CompatibleModeWindowSessionImplTest, RegisterUIContentCreateListenerWithNotNullListener,
    TestSize.Level1)
{
    GTEST_LOG_(INFO) << "RegisterUIContentCreateListenerWithNotNullListener test start";
    sptr<WindowOption> option = sptr<WindowOption>::MakeSptr();
    sptr<WindowSessionImpl> window = sptr<WindowSessionImpl>::MakeSptr(option);
    sptr<IUIContentCreateListener> listener = sptr<IUIContentCreateListener>::MakeSptr();
    auto ret = window->RegisterUIContentCreateListener(listener);
    EXPECT_EQ(ret, WMError::WM_OK);
    GTEST_LOG_(INFO) << "RegisterUIContentCreateListenerWithNotNullListener test end";
}

/**
 * @tc.name: UnregisterUIContentCreateListener
 * @tc.desc: UnregisterUIContentCreateListenerWithNullListener
 * @tc.type: FUNC
 */
HWTEST_F(CompatibleModeWindowSessionImplTest, UnregisterUIContentCreateListenerWithNullListener,
    TestSize.Level1)
{
    GTEST_LOG_(INFO) << "UnregisterUIContentCreateListenerWithNullListener test start";
    sptr<WindowOption> option = sptr<WindowOption>::MakeSptr();
    sptr<WindowSessionImpl> window = sptr<WindowSessionImpl>::MakeSptr(option);
    sptr<IUIContentCreateListener> listener = nullptr;
    auto ret = window->UnregisterUIContentCreateListener(listener);
    EXPECT_EQ(ret, WMError::WM_ERROR_NULLPTR);
    GTEST_LOG_(INFO) << "UnregisterUIContentCreateListenerWithNullListener test end";
}

/**
 * @tc.name: UnregisterUIContentCreateListener
 * @tc.desc: UnregisterUIContentCreateListenerWithNotNullListener
 * @tc.type: FUNC
 */
HWTEST_F(CompatibleModeWindowSessionImplTest, UnregisterUIContentCreateListenerWithNotNullListener,
    TestSize.Level1)
{
    GTEST_LOG_(INFO) << "UnregisterUIContentCreateListenerWithNotNullListener test start";
    sptr<WindowOption> option = sptr<WindowOption>::MakeSptr();
    sptr<WindowSessionImpl> window = sptr<WindowSessionImpl>::MakeSptr(option);
    sptr<IUIContentCreateListener> listener = sptr<IUIContentCreateListener>::MakeSptr();
    auto ret = window->UnregisterUIContentCreateListener(listener);
    EXPECT_EQ(ret, WMError::WM_OK);
    GTEST_LOG_(INFO) << "UnregisterUIContentCreateListenerWithNotNullListener test end";
}

/**
 * @tc.name: UpdateCompatibleStyleMode
 * @tc.desc: UpdateCompatibleStyleModeWithNullSession
 * @tc.type: FUNC
 */
HWTEST_F(CompatibleModeWindowSessionImplTest, UpdateCompatibleStyleModeWithNullSession,
    TestSize.Level1)
{
    GTEST_LOG_(INFO) << "UpdateCompatibleStyleModeWithNullSession test start";
    sptr<WindowOption> option = sptr<WindowOption>::MakeSptr();
    sptr<WindowSessionImpl> window = sptr<WindowSessionImpl>::MakeSptr(option);
    window->uiContent_ = std::make_unique<Ace::UIContentMocker>();
    auto ret = window->UpdateCompatibleStyleMode(CompatibleStyleMode::LANDSCAPE_18_9);
    EXPECT_EQ(ret, WMError::WM_ERROR_NULLPTR);
    GTEST_LOG_(INFO) << "UpdateCompatibleStyleModeWithNullSession test end";
}

/**
 * @tc.name: UpdateCompatibleStyleMode
 * @tc.desc: UpdateCompatibleStyleMode
 * @tc.type: FUNC
 */
HWTEST_F(CompatibleModeWindowSessionImplTest, UpdateCompatibleStyleMode, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "UpdateCompatibleStyleMode test start";
    sptr<WindowOption> option = sptr<WindowOption>::MakeSptr();
    sptr<WindowSessionImpl> window = sptr<WindowSessionImpl>::MakeSptr(option);
    window->uiContent_ = std::make_unique<Ace::UIContentMocker>();
    SessionInfo sessionInfo;
    window->hostSession_ = sptr<SessionMocker>::MakeSptr(sessionInfo);
    auto ret = window->UpdateCompatibleStyleMode(CompatibleStyleMode::LANDSCAPE_18_9);
    EXPECT_EQ(ret, WMError::WM_OK);
    GTEST_LOG_(INFO) << "UpdateCompatibleStyleMode test end";
}

/**
 * @tc.name: SetAppHookWindowInfo
 * @tc.desc: SetAppHookWindowInfo
 * @tc.type: FUNC
 */
HWTEST_F(CompatibleModeWindowSessionImplTest, SetAppHookWindowInfo, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "CompatibleModeWindowSessionImplTest: SetAppHookWindowInfo start";
    sptr<WindowOption> option = sptr<WindowOption>::MakeSptr();
    sptr<WindowSessionImpl> window = sptr<WindowSessionImpl>::MakeSptr(option);

    // Case 1: SetAppHookWindowInfo
    HookWindowInfo hookWindowInfo;
    hookWindowInfo.enableHookWindow = false;
    hookWindowInfo.widthHookRatio = 1.0f;
    hookWindowInfo.notifyWindowChange = false;
    window->SetAppHookWindowInfo(hookWindowInfo);
    const uint32_t defaultSize = 800;
    Rect rect = { 0, 0, defaultSize, defaultSize };
    window->HookWindowSizeByHookWindowInfo(rect);
    EXPECT_EQ(rect.width_, defaultSize);

    // Case 2: SetAppHookWindowInfo with notifyWindowChange
    hookWindowInfo.notifyWindowChange = true;
    window->SetAppHookWindowInfo(hookWindowInfo);
    window->HookWindowSizeByHookWindowInfo(rect);
    EXPECT_EQ(rect.width_, defaultSize);
    GTEST_LOG_(INFO) << "CompatibleModeWindowSessionImplTest: SetAppHookWindowInfo end";
}

/**
 * @tc.name: UpdateAppHookWindowInfo
 * @tc.desc: UpdateAppHookWindowInfo
 * @tc.type: FUNC
 */
HWTEST_F(CompatibleModeWindowSessionImplTest, UpdateAppHookWindowInfo, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "CompatibleModeWindowSessionImplTest: UpdateAppHookWindowInfo start";
    sptr<WindowOption> option = sptr<WindowOption>::MakeSptr();
    option->SetWindowName("UpdateAppHookWindowInfo");
    option->SetWindowType(WindowType::WINDOW_TYPE_APP_MAIN_WINDOW);
    sptr<WindowSceneSessionImpl> window = sptr<WindowSceneSessionImpl>::MakeSptr(option);
    const int32_t windowId = 2025;
    window->property_->SetPersistentId(windowId);
    HookWindowInfo hookWindowInfo;
    hookWindowInfo.enableHookWindow = false;
    hookWindowInfo.widthHookRatio = 1.0f;
    hookWindowInfo.notifyWindowChange = false;

    // Case 1: success
    SessionInfo sessionInfo = { "CreateTestBundle", "CreateTestModule", "CreateTestAbility" };
    sptr<SessionMocker> session = sptr<SessionMocker>::MakeSptr(sessionInfo);
    window->hostSession_ = session;
    auto res = window->UpdateAppHookWindowInfo(hookWindowInfo);
    EXPECT_EQ(res, WSError::WS_OK);

    // Case 2: not mainWindow
    window->property_->SetWindowType(WindowType::WINDOW_TYPE_APP_SUB_WINDOW);
    res = window->UpdateAppHookWindowInfo(hookWindowInfo);
    EXPECT_EQ(res, WSError::WS_DO_NOTHING);
    GTEST_LOG_(INFO) << "CompatibleModeWindowSessionImplTest: UpdateAppHookWindowInfo end";
}
} // namespace
} // namespace Rosen
} // namespace OHOS
