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
#include <ui_content.h>
#include <viewport_config.h>

#include "app_mgr_client.h"
#include "singleton.h"
#include "singleton_container.h"

#include "root_scene.h"
#include "screen_scene.h"
#include "vsync_station.h"
#include "window_manager_hilog.h"

using namespace testing;
using namespace testing::ext;

namespace OHOS {
namespace Rosen {
class ScreenSceneTest : public testing::Test {
public:
    static void SetUpTestCase();
    static void TearDownTestCase();
    void SetUp() override;
    void TearDown() override;
};

void ScreenSceneTest::SetUpTestCase() {}

void ScreenSceneTest::TearDownTestCase() {}

void ScreenSceneTest::SetUp() {}

void ScreenSceneTest::TearDown() {}

namespace {
/**
 * @tc.name: LoadContent01
 * @tc.desc: context is nullptr
 * @tc.type: FUNC
 */
HWTEST_F(ScreenSceneTest, LoadContent01, TestSize.Level1)
{
    ScreenScene screenScene("UNKNOWN");
    screenScene.LoadContent("a", nullptr, nullptr, nullptr);
    ASSERT_EQ(1, screenScene.GetWindowId());
}

/**
 * @tc.name: Destroy01
 * @tc.desc: context is nullptr
 * @tc.type: FUNC
 */
HWTEST_F(ScreenSceneTest, Destroy01, TestSize.Level1)
{
    ScreenScene screenScene("UNKNOWN");
    screenScene.LoadContent("a", nullptr, nullptr, nullptr);
    screenScene.Destroy();
    ASSERT_EQ(1, screenScene.GetWindowId());
}

/**
 * @tc.name: UpdateViewportConfig01
 * @tc.desc: UpdateViewportConfig Test
 * @tc.type: FUNC
 */
HWTEST_F(ScreenSceneTest, UpdateViewportConfig01, TestSize.Level1)
{
    ScreenScene screenScene("UNKNOWN");
    Rect rect;
    screenScene.UpdateViewportConfig(rect, WindowSizeChangeReason::UNDEFINED);
    ASSERT_EQ(1, screenScene.GetWindowId());
}

/**
 * @tc.name: UpdateConfiguration
 * @tc.desc: UpdateConfiguration Test
 * @tc.type: FUNC
 */
HWTEST_F(ScreenSceneTest, UpdateConfiguration, TestSize.Level1)
{
    ScreenScene screenScene("UNKNOWN");
    std::shared_ptr<AppExecFwk::Configuration> configuration = std::make_shared<AppExecFwk::Configuration>();
    screenScene.UpdateConfiguration(configuration);
    ASSERT_EQ(1, screenScene.GetWindowId());
}

/**
 * @tc.name: RequestVsync
 * @tc.desc: RequestVsync Test
 * @tc.type: FUNC
 */
HWTEST_F(ScreenSceneTest, RequestVsync, TestSize.Level1)
{
    ScreenScene screenScene("UNKNOWN");
    std::shared_ptr<VsyncCallback> vsyncCallback = std::make_shared<VsyncCallback>();
    screenScene.RequestVsync(vsyncCallback);
    ASSERT_EQ(1, screenScene.GetWindowId());
}

/**
 * @tc.name: GetVSyncPeriod
 * @tc.desc: GetVSyncPeriod Test
 * @tc.type: FUNC
 */
HWTEST_F(ScreenSceneTest, GetVSyncPeriod, TestSize.Level1)
{
    ScreenScene screenScene("UNKNOWN");
    screenScene.GetVSyncPeriod();
    ASSERT_EQ(1, screenScene.GetWindowId());
}

/**
 * @tc.name: FlushFrameRate
 * @tc.desc: FlushFrameRate Test
 * @tc.type: FUNC
 */
HWTEST_F(ScreenSceneTest, FlushFrameRate, TestSize.Level1)
{
    ScreenScene screenScene("UNKNOWN");
    uint32_t rate = 120;
    int32_t animatorExpectedFrameRate = -1;
    screenScene.FlushFrameRate(rate, animatorExpectedFrameRate);
    ASSERT_EQ(1, screenScene.GetWindowId());
}

/**
 * @tc.name: OnBundleUpdated
 * @tc.desc: OnBundleUpdated Test
 * @tc.type: FUNC
 */
HWTEST_F(ScreenSceneTest, OnBundleUpdated, TestSize.Level1)
{
    ScreenScene screenScene("UNKNOWN");
    std::string bundleName = "test";
    screenScene.OnBundleUpdated(bundleName);
    ASSERT_EQ(1, screenScene.GetWindowId());
}

/**
 * @tc.name: GetDisplayId & SetDisplayId
 * @tc.desc: GetDisplayId & SetDisplayId Test01
 * @tc.type: FUNC
 */
HWTEST_F(ScreenSceneTest, DisplayIdTest01, TestSize.Level1)
{
    ScreenScene screenScene("UNKNOWN");
    DisplayId setDisplayId = 0;
    screenScene.SetDisplayId(setDisplayId);
    screenScene.GetDisplayId();
    ASSERT_EQ(1, screenScene.GetWindowId());
}

/**
 * @tc.name: GetDisplayId & SetDisplayId
 * @tc.desc: GetDisplayId & SetDisplayId Test02
 * @tc.type: FUNC
 */
HWTEST_F(ScreenSceneTest, DisplayIdTest02, TestSize.Level1)
{
    ScreenScene screenScene("UNKNOWN");
    DisplayId setDisplayId = 100;
    screenScene.SetDisplayId(setDisplayId);
    screenScene.GetDisplayId();
    ASSERT_EQ(1, screenScene.GetWindowId());
}

/**
 * @tc.name: SetDisplayOrientation
 * @tc.desc: SetDisplayOrientation Test01
 * @tc.type: FUNC
 */
HWTEST_F(ScreenSceneTest, SetDisplayOrientation01, TestSize.Level1)
{
    ScreenScene screenScene("UNKNOWN");
    int32_t orientation = 0;
    screenScene.SetDisplayOrientation(orientation);
    ASSERT_EQ(1, screenScene.GetWindowId());
}

/**
 * @tc.name: SetDisplayOrientation
 * @tc.desc: SetDisplayOrientation Test02
 * @tc.type: FUNC
 */
HWTEST_F(ScreenSceneTest, SetDisplayOrientation02, TestSize.Level1)
{
    ScreenScene screenScene("UNKNOWN");
    int32_t orientation = 100;
    screenScene.SetDisplayOrientation(orientation);
    ASSERT_EQ(1, screenScene.GetWindowId());
}

/**
 * @tc.name: SetFrameLayoutFinishCallback
 * @tc.desc: SetFrameLayoutFinishCallback Test
 * @tc.type: FUNC
 */
HWTEST_F(ScreenSceneTest, SetFrameLayoutFinishCallback, TestSize.Level1)
{
    ScreenScene screenScene("UNKNOWN");
    screenScene.SetFrameLayoutFinishCallback([]() {});
    ASSERT_EQ(1, screenScene.GetWindowId());
}

/**
 * @tc.name: SetDisplayDensity
 * @tc.desc: SetDisplayDensity Test
 * @tc.type: FUNC
 */
HWTEST_F(ScreenSceneTest, SetDisplayDensity, TestSize.Level1)
{
    ScreenScene screenScene("UNKNOWN");
    float density = 2.0f;
    screenScene.SetDisplayDensity(density);
    ASSERT_EQ(1, screenScene.GetWindowId());
}

/**
 * @tc.name: IsSystemWindow
 * @tc.desc: IsSystemWindow Test
 * @tc.type: FUNC
 */
HWTEST_F(ScreenSceneTest, IsSystemWindow, TestSize.Level1)
{
    ScreenScene screenScene("UNKNOWN");
    bool res = screenScene.IsSystemWindow();
    ASSERT_EQ(true, res);
}

/**
 * @tc.name: IsAppWindow
 * @tc.desc: IsAppWindow Test
 * @tc.type: FUNC
 */
HWTEST_F(ScreenSceneTest, IsAppWindow, TestSize.Level1)
{
    ScreenScene screenScene("UNKNOWN");
    bool res = screenScene.IsAppWindow();
    ASSERT_EQ(false, res);
}

/**
 * @tc.name: RegisterInputEventListener
 * @tc.desc: RegisterInputEventListener Test
 * @tc.type: FUNC
 */
HWTEST_F(ScreenSceneTest, RegisterInputEventListener, TestSize.Level1)
{
    sptr<ScreenScene> screenScene = sptr<ScreenScene>::MakeSptr("UNKNOWN");
    auto prevStaticRootScene = RootScene::staticRootScene_;

    RootScene::staticRootScene_ = nullptr;
    screenScene->RegisterInputEventListener();

    sptr<RootScene> staticRootScene = sptr<RootScene>::MakeSptr();
    RootScene::staticRootScene_ = staticRootScene;
    screenScene->RegisterInputEventListener();

    RootScene::staticRootScene_ = prevStaticRootScene;
    ASSERT_EQ(1, screenScene->GetWindowId());
}
} // namespace
} // namespace Rosen
} // namespace OHOS