/*
 * Copyright (c) 2022-2025 Huawei Device Co., Ltd.
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

#include "display_manager.h"
#include "display_manager_adapter.h"
#include "display_manager_proxy.h"
#include "display_manager_agent_default.h"
#include "scene_board_judgement.h"
#include "screen_manager.h"
#include "iconsumer_surface.h"
#include "window_scene.h"
#include "screen_manager_utils.h"

using namespace testing;
using namespace testing::ext;

namespace OHOS::Rosen {

class DisplayManagerAdapterTestTwo : public testing::Test {
public:
    static void SetUpTestCase();
    static void TearDownTestCase();
    void SetUp() override;
    void TearDown() override;
};

void DisplayManagerAdapterTestTwo::SetUpTestCase() {}

void DisplayManagerAdapterTestTwo::TearDownTestCase() {}

void DisplayManagerAdapterTestTwo::SetUp() {}

void DisplayManagerAdapterTestTwo::TearDown() {}

namespace {
std::string g_errLog;
void MyLogCallback(const LogType type, const LogLevel level, const unsigned int domain, const char* tag,
    const char* msg)
{
    g_errLog = msg;
}
/**
 * @tc.name: AddVirtualScreenSurface01
 * @tc.desc: test screenSessionManagerServiceProxy_ is nullptr
 * @tc.type: FUNC
 */
HWTEST_F(DisplayManagerAdapterTestTwo, AddVirtualScreenSurface01, TestSize.Level1)
{
    if (!SceneBoardJudgement::IsSceneBoardEnabled()) {
        return;
    }
    ScreenId screenId = 1;
    sptr<Surface> surface = nullptr;
    DMRect surfaceRegion = {0, 0, 100, 100};

    auto screenSessionManagerServiceProxy =
        SingletonContainer::Get<ScreenManagerAdapter>().screenSessionManagerServiceProxy_;
    SingletonContainer::Get<ScreenManagerAdapter>().screenSessionManagerServiceProxy_ = nullptr;
    DMError err = SingletonContainer::Get<ScreenManagerAdapter>().AddVirtualScreenSurface(screenId, surface, surfaceRegion);
    EXPECT_EQ(err, DMError::DM_ERROR_DEVICE_NOT_SUPPORT);
    SingletonContainer::Get<ScreenManagerAdapter>().screenSessionManagerServiceProxy_ =
        screenSessionManagerServiceProxy;
}

/**
 * @tc.name: AddVirtualScreenSurface02
 * @tc.desc: test AddVirtualScreenSurface with surface nullptr
 * @tc.type: FUNC
 */
HWTEST_F(DisplayManagerAdapterTestTwo, AddVirtualScreenSurface02, TestSize.Level1)
{
    if (!SceneBoardJudgement::IsSceneBoardEnabled()) {
        return;
    }
    ScreenId screenId = 1;
    sptr<Surface> surface = nullptr;
    DMRect surfaceRegion = {0, 0, 100, 100};

    g_errLog.clear();
    LOG_SetCallback(MyLogCallback);
    DMError err = SingletonContainer::Get<ScreenManagerAdapter>().AddVirtualScreenSurface(screenId, surface, surfaceRegion);
    EXPECT_EQ(err, DMError::DM_ERROR_INVALID_PARAM);
    EXPECT_TRUE(g_errLog.find("surface is null") != std::string::npos);
    LOG_SetCallback(nullptr);
}

/**
 * @tc.name: RemoveVirtualScreenSurface01
 * @tc.desc: test screenSessionManagerServiceProxy_ is nullptr
 * @tc.type: FUNC
 */
HWTEST_F(DisplayManagerAdapterTestTwo, RemoveVirtualScreenSurface01, TestSize.Level1)
{
    if (!SceneBoardJudgement::IsSceneBoardEnabled()) {
        return;
    }
    ScreenId screenId = 1;
    sptr<Surface> surface = nullptr;

    auto screenSessionManagerServiceProxy =
        SingletonContainer::Get<ScreenManagerAdapter>().screenSessionManagerServiceProxy_;
    SingletonContainer::Get<ScreenManagerAdapter>().screenSessionManagerServiceProxy_ = nullptr;
    DMError err = SingletonContainer::Get<ScreenManagerAdapter>().RemoveVirtualScreenSurface(screenId, surface);
    EXPECT_EQ(err, DMError::DM_ERROR_DEVICE_NOT_SUPPORT);
    SingletonContainer::Get<ScreenManagerAdapter>().screenSessionManagerServiceProxy_ =
        screenSessionManagerServiceProxy;
}

/**
 * @tc.name: RemoveVirtualScreenSurface02
 * @tc.desc: test RemoveVirtualScreenSurface with surface nullptr
 * @tc.type: FUNC
 */
HWTEST_F(DisplayManagerAdapterTestTwo, RemoveVirtualScreenSurface02, TestSize.Level1)
{
    if (!SceneBoardJudgement::IsSceneBoardEnabled()) {
        return;
    }
    ScreenId screenId = 1;
    sptr<Surface> surface = nullptr;

    g_errLog.clear();
    LOG_SetCallback(MyLogCallback);
    DMError err = SingletonContainer::Get<ScreenManagerAdapter>().RemoveVirtualScreenSurface(screenId, surface);
    EXPECT_EQ(err, DMError::DM_ERROR_INVALID_PARAM);
    EXPECT_TRUE(g_errLog.find("surface is null") != std::string::npos);
    LOG_SetCallback(nullptr);
}

/**
 * @tc.name: AddVirtualScreenSurface03
 * @tc.desc: test AddVirtualScreenSurface with screenSessionManagerServiceProxy_ not nullptr
 * @tc.type: FUNC
 */
HWTEST_F(DisplayManagerAdapterTestTwo, AddVirtualScreenSurface03, TestSize.Level1)
{
    if (!SceneBoardJudgement::IsSceneBoardEnabled()) {
        return;
    }
    ScreenId screenId = 1;
    ScreenManagerUtils utils;
    ASSERT_TRUE(utils.CreateSurface());
    DMRect surfaceRegion = {0, 0, 100, 100};

    g_errLog.clear();
    LOG_SetCallback(MyLogCallback);
    DMError err = SingletonContainer::Get<ScreenManagerAdapter>().AddVirtualScreenSurface(
        screenId, utils.psurface_, surfaceRegion);
    EXPECT_TRUE(g_errLog.find("enter AddVirtualScreenSurface") != std::string::npos);
    LOG_SetCallback(nullptr);
}

/**
 * @tc.name: RemoveVirtualScreenSurface03
 * @tc.desc: test RemoveVirtualScreenSurface with screenSessionManagerServiceProxy_ not nullptr
 * @tc.type: FUNC
 */
HWTEST_F(DisplayManagerAdapterTestTwo, RemoveVirtualScreenSurface03, TestSize.Level1)
{
    if (!SceneBoardJudgement::IsSceneBoardEnabled()) {
        return;
    }
    ScreenId screenId = 1;
    ScreenManagerUtils utils;
    ASSERT_TRUE(utils.CreateSurface());

    g_errLog.clear();
    LOG_SetCallback(MyLogCallback);
    DMError err = SingletonContainer::Get<ScreenManagerAdapter>().RemoveVirtualScreenSurface(
        screenId, utils.psurface_);
    EXPECT_TRUE(g_errLog.find("enter RemoveVirtualScreenSurface") != std::string::npos);
    LOG_SetCallback(nullptr);
}
} // namespace
} // namespace OHOS::Rosen
