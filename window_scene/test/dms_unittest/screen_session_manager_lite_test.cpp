/*
 * Copyright (c) 2024 Huawei Device Co., Ltd.
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

#include "screen_session_manager/include/screen_session_manager_lite.h"
#include "window_manager_hilog.h"
#include "display_manager_agent_default.h"
#include "flod_screen_state_internel.h"

using namespace testing;
using namespace testing::ext;

namespace OHOS {
namespace Rosen {
namespace {
constexpr uint32_t SLEEP_TIME_US = 100000;
}

class ScreenSessionManagerLiteTest : public testing::Test {
public:
    static void SetUpTestCase();
    static void TearDownTestCase();
    void SetUp() override;
    void TearDown() override;
};

void ScreenSessionManagerLiteTest::SetUpTestCase()
{
}

void ScreenSessionManagerLiteTest::TearDownTestCase()
{
}

void ScreenSessionManagerLiteTest::SetUp()
{
}

void ScreenSessionManagerLiteTest::TearDown()
{
    usleep(SLEEP_TIME_US);
}

namespace {

/**
 * @tc.name: ConnectToServer
 * @tc.desc: ConnectToServer
 * @tc.type: FUNC
 */
HWTEST_F(ScreenSessionManagerLiteTest, ConnectToServer, TestSize.Level1)
{
    ScreenSessionManagerLite screenSessionManagerLite = ScreenSessionManagerLite();
    screenSessionManagerLite.ConnectToServer();
    ASSERT_NE(screenSessionManagerLite.screenSessionManager_, nullptr);
}

/**
 * @tc.name: RegisterDisplayManagerAgent
 * @tc.desc: RegisterDisplayManagerAgent
 * @tc.type: FUNC
 */
HWTEST_F(ScreenSessionManagerLiteTest, RegisterDisplayManagerAgent, TestSize.Level1)
{
    ScreenSessionManagerLite screenSessionManagerLite = ScreenSessionManagerLite();
    sptr<IDisplayManagerAgent> displayManagerAgent= new DisplayManagerAgentDefault();
    DisplayManagerAgentType type = DisplayManagerAgentType::DISPLAY_POWER_EVENT_LISTENER;
    DMError ret = screenSessionManagerLite.RegisterDisplayManagerAgent(displayManagerAgent, type);
    ASSERT_NE(ret, DMError::DM_ERROR_NULLPTR);
}

/**
 * @tc.name: UnregisterDisplayManagerAgent
 * @tc.desc: UnregisterDisplayManagerAgent
 * @tc.type: FUNC
 */
HWTEST_F(ScreenSessionManagerLiteTest, UnregisterDisplayManagerAgent, TestSize.Level1)
{
    ScreenSessionManagerLite screenSessionManagerLite = ScreenSessionManagerLite();
    sptr<IDisplayManagerAgent> displayManagerAgent= new DisplayManagerAgentDefault();
    DisplayManagerAgentType type = DisplayManagerAgentType::DISPLAY_POWER_EVENT_LISTENER;
    DMError ret = screenSessionManagerLite.UnregisterDisplayManagerAgent(displayManagerAgent, type);
    ASSERT_NE(ret, DMError::DM_ERROR_NULLPTR);
}

/**
 * @tc.name: GetFoldDisplayMode
 * @tc.desc: GetFoldDisplayMode
 * @tc.type: FUNC
 */
HWTEST_F(ScreenSessionManagerLiteTest, GetFoldDisplayMode, TestSize.Level1)
{
    ScreenSessionManagerLite screenSessionManagerLite = ScreenSessionManagerLite();
    bool isFoldable = screenSessionManagerLite.IsFoldable();
    if (!isFoldable) {
        GTEST_SKIP();
    }
    if (FoldScreenStateInternel::IsSuperFoldDisplayDevice())
    {
        FoldDisplayMode ret = screenSessionManagerLite.GetFoldDisplayMode();
        ASSERT_EQ(ret, FoldDisplayMode::UNKNOWN);
    } else {
        FoldDisplayMode ret = screenSessionManagerLite.GetFoldDisplayMode();
        ASSERT_NE(ret, FoldDisplayMode::UNKNOWN);
    }
}

/**
 * @tc.name: GetDefaultDisplayInfo
 * @tc.desc: GetDefaultDisplayInfo
 * @tc.type: FUNC
 */
HWTEST_F(ScreenSessionManagerLiteTest, GetDefaultDisplayInfo, TestSize.Level1)
{
    ScreenSessionManagerLite screenSessionManagerLite = ScreenSessionManagerLite();
    sptr<DisplayInfo> ret = screenSessionManagerLite.GetDefaultDisplayInfo();
    ASSERT_NE(ret, nullptr);
}

/**
 * @tc.name: GetDisplayInfoById
 * @tc.desc: GetDisplayInfoById
 * @tc.type: FUNC
 */
HWTEST_F(ScreenSessionManagerLiteTest, GetDisplayInfoById, TestSize.Level1)
{
    ScreenSessionManagerLite screenSessionManagerLite = ScreenSessionManagerLite();
    DisplayId displayId = 0;
    sptr<DisplayInfo> ret = screenSessionManagerLite.GetDisplayInfoById(displayId);
    ASSERT_NE(ret, nullptr);
}

/**
 * @tc.name: GetCutoutInfo
 * @tc.desc: GetCutoutInfo
 * @tc.type: FUNC
 */
HWTEST_F(ScreenSessionManagerLiteTest, GetCutoutInfo, TestSize.Level1)
{
    ScreenSessionManagerLite screenSessionManagerLite = ScreenSessionManagerLite();
    DisplayId displayId = 0;
    sptr<CutoutInfo> ret = screenSessionManagerLite.GetCutoutInfo(displayId);
    ASSERT_NE(ret, nullptr);
}

/**
 * @tc.name: Clear
 * @tc.desc: Clear
 * @tc.type: FUNC
 */
HWTEST_F(ScreenSessionManagerLiteTest, Clear, TestSize.Level1)
{
    ScreenSessionManagerLite screenSessionManagerLite = ScreenSessionManagerLite();
    screenSessionManagerLite.Clear();
    ASSERT_EQ(screenSessionManagerLite.screenSessionManager_, nullptr);
}

/**
 * @tc.name: GetInstance_ShouldCreateNewInstance_WhenInstanceIsNull
 * @tc.desc: Test if GetInstance creates a new instance when instance is nullptr
 * @tc.type: FUNC
 */
HWTEST_F(ScreenSessionManagerLiteTest, GetInstance_ShouldCreateNewInstance_WhenInstanceIsNull, TestSize.Level1)
{
    ScreenSessionManagerLite& instance = ScreenSessionManagerLite::GetInstance();

    EXPECT_NE(nullptr, &instance);
}

/**
 * @tc.name: GetInstance_ShouldReturnExistingInstance_WhenInstanceIsNotNull
 * @tc.desc: Test if GetInstance returns the existing instance when it's not nullptr
 * @tc.type: FUNC
 */
HWTEST_F(ScreenSessionManagerLiteTest, GetInstance_ShouldReturnExistingInstance_WhenInstanceIsNotNull,
    TestSize.Level1)
{
    ScreenSessionManagerLite& instance1 = ScreenSessionManagerLite::GetInstance();

    ScreenSessionManagerLite& instance2 = ScreenSessionManagerLite::GetInstance();

    EXPECT_EQ(&instance1, &instance2);
}
}
} // namespace Rosen
} // namespace OHOS