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

#include <gtest/gtest.h>
#include "root_scene.h"
#include <event_handler.h>
#include <input_manager.h>
#include <ui_content.h>
#include <viewport_config.h>

#include "app_mgr_client.h"
#include "singleton.h"
#include "singleton_container.h"

#include "vsync_station.h"
#include "window_manager_hilog.h"

using namespace testing;
using namespace testing::ext;

namespace OHOS {
namespace Rosen {
const uint32_t MOCK_LEM_SUB_WIDTH = 340;
const uint32_t MOCK_LEM_SUB_HEIGHT = 340;

class RootSceneTest : public testing::Test {
public:
    static void SetUpTestCase();
    static void TearDownTestCase();
    void SetUp() override;
    void TearDown() override;
private:
    static constexpr uint32_t WAIT_SYNC_IN_NS = 200000;
};

void RootSceneTest::SetUpTestCase()
{
}

void RootSceneTest::TearDownTestCase()
{
}

void RootSceneTest::SetUp()
{
}

void RootSceneTest::TearDown()
{
    usleep(WAIT_SYNC_IN_NS);
}

namespace {
/**
 * @tc.name: LoadContent01
 * @tc.desc: context is nullptr
 * @tc.type: FUNC
 */
HWTEST_F(RootSceneTest, LoadContent01, Function | SmallTest | Level3)
{
    RootScene rootScene;
    rootScene.LoadContent("a", nullptr, nullptr, nullptr);
    ASSERT_EQ(1, rootScene.GetWindowId());
}

/**
 * @tc.name: UpdateViewportConfig01
 * @tc.desc: UpdateViewportConfig Test
 * @tc.type: FUNC
 */
HWTEST_F(RootSceneTest, UpdateViewportConfig01, Function | SmallTest | Level3)
{
    RootScene rootScene;
    Rect rect;

    rootScene.uiContent_ = nullptr;
    rootScene.UpdateViewportConfig(rect, WindowSizeChangeReason::UNDEFINED);

    auto uiContent= Ace::UIContent::Create(nullptr, nullptr);
    rootScene.uiContent_=std::move(uiContent);
    rootScene.UpdateViewportConfig(rect, WindowSizeChangeReason::UNDEFINED);

    rect.width_ = MOCK_LEM_SUB_WIDTH;
    rect.height_ = MOCK_LEM_SUB_HEIGHT;
    rootScene.UpdateViewportConfig(rect, WindowSizeChangeReason::UNDEFINED);
    ASSERT_EQ(1, rootScene.GetWindowId());
}

/**
 * @tc.name: UpdateConfiguration
 * @tc.desc: UpdateConfiguration Test
 * @tc.type: FUNC
 */
HWTEST_F(RootSceneTest, UpdateConfiguration, Function | SmallTest | Level3)
{
    RootScene rootScene;
    std::shared_ptr<AppExecFwk::Configuration> configuration = std::make_shared<AppExecFwk::Configuration>();

    rootScene.uiContent_ = nullptr;
    rootScene.UpdateConfiguration(configuration);

    auto uiContent= Ace::UIContent::Create(nullptr,nullptr);
    rootScene.uiContent_=std::move(uiContent);
    rootScene.UpdateConfiguration(configuration);
    ASSERT_EQ(1, rootScene.GetWindowId());
}

/**
 * @tc.name: UpdateConfigurationForAll
 * @tc.desc: UpdateConfigurationForAll Test
 * @tc.type: FUNC
 */
HWTEST_F(RootSceneTest, UpdateConfigurationForAll, Function | SmallTest | Level3)
{
    RootScene rootScene;
    std::shared_ptr<AppExecFwk::Configuration> configuration = std::make_shared<AppExecFwk::Configuration>();

    auto prevStaticRootScene = RootScene::staticRootScene_;
    rootScene.UpdateConfigurationForAll(configuration);

    sptr<RootScene> staticRootScene;
    RootScene::staticRootScene_ = staticRootScene;
    rootScene.UpdateConfigurationForAll(configuration);

    RootScene::staticRootScene_ = prevStaticRootScene;
    ASSERT_EQ(1, rootScene.GetWindowId());
}

/**
 * @tc.name: RegisterInputEventListener01
 * @tc.desc: RegisterInputEventListener Test
 * @tc.type: FUNC
 */
HWTEST_F(RootSceneTest, RegisterInputEventListener01, Function | SmallTest | Level3)
{
    RootScene rootScene;
    rootScene.RegisterInputEventListener();
    ASSERT_EQ(1, rootScene.GetWindowId());
}

/**
 * @tc.name: RequestVsyncErr
 * @tc.desc: RequestVsync Test Err
 * @tc.type: FUNC
 */
HWTEST_F(RootSceneTest, RequestVsyncErr, Function | SmallTest | Level3)
{
    RootScene rootScene;
    std::shared_ptr<VsyncCallback> vsyncCallback = std::make_shared<VsyncCallback>();
    rootScene.RequestVsync(vsyncCallback);
    ASSERT_EQ(1, rootScene.GetWindowId());
}

/**
 * @tc.name: GetVSyncPeriod
 * @tc.desc: GetVSyncPeriod Test
 * @tc.type: FUNC
 */
HWTEST_F(RootSceneTest, GetVSyncPeriod, Function | SmallTest | Level3)
{
    RootScene rootScene;
    rootScene.GetVSyncPeriod();
    ASSERT_EQ(1, rootScene.GetWindowId());
}

/**
 * @tc.name: FlushFrameRate
 * @tc.desc: FlushFrameRate Test
 * @tc.type: FUNC
 */
HWTEST_F(RootSceneTest, FlushFrameRate, Function | SmallTest | Level3)
{
    RootScene rootScene;
    uint32_t rate = 120;
    int32_t animatorExpectedFrameRate = -1;
    rootScene.FlushFrameRate(rate, animatorExpectedFrameRate);
    ASSERT_EQ(1, rootScene.GetWindowId());
}

/**
 * @tc.name: SetFrameLayoutFinishCallback
 * @tc.desc: SetFrameLayoutFinishCallback Test
 * @tc.type: FUNC
 */
HWTEST_F(RootSceneTest, SetFrameLayoutFinishCallback, Function | SmallTest | Level3)
{
    RootScene rootScene;
    
    rootScene.SetFrameLayoutFinishCallback(nullptr);

    auto uiContent= Ace::UIContent::Create(nullptr, nullptr);
    rootScene.uiContent_=std::move(uiContent);
    rootScene.SetFrameLayoutFinishCallback(nullptr);
    ASSERT_EQ(1, rootScene.GetWindowId());
}

/**
 * @tc.name: SetUiDvsyncSwitch
 * @tc.desc: SetUiDvsyncSwitch Test
 * @tc.type: FUNC
 */
HWTEST_F(RootSceneTest, SetUiDvsyncSwitchSucc, Function | SmallTest | Level3)
{
    RootScene rootScene;
    rootScene.SetUiDvsyncSwitch(true);
    rootScene.SetUiDvsyncSwitch(false);
    ASSERT_EQ(1, rootScene.GetWindowId());
}

/**
 * @tc.name: SetUiDvsyncSwitch
 * @tc.desc: SetUiDvsyncSwitch Test
 * @tc.type: FUNC
 */
HWTEST_F(RootSceneTest, SetUiDvsyncSwitchErr, Function | SmallTest | Level3)
{
    RootScene rootScene;
    rootScene.vsyncStation_ = nullptr;
    rootScene.SetUiDvsyncSwitch(true);
    rootScene.SetUiDvsyncSwitch(false);
    ASSERT_EQ(1, rootScene.GetWindowId());
}
}
} // namespace Rosen
} // namespace OHOS