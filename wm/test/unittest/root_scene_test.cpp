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

#include <event_handler.h>
#include <gtest/gtest.h>
#include <input_manager.h>
#include <ui_content.h>
#include "mock_uicontent.h"
#include <viewport_config.h>
#include "root_scene.h"
#include "screen_scene.h"

#include "app_mgr_client.h"
#include "mock_uicontent.h"
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

void RootSceneTest::SetUpTestCase() {}

void RootSceneTest::TearDownTestCase() {}

void RootSceneTest::SetUp() {}

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
HWTEST_F(RootSceneTest, LoadContent01, TestSize.Level1)
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
HWTEST_F(RootSceneTest, UpdateViewportConfig01, TestSize.Level1)
{
    RootScene rootScene;
    Rect rect;

    rootScene.uiContent_ = nullptr;
    rootScene.UpdateViewportConfig(rect, WindowSizeChangeReason::UNDEFINED);

    rootScene.uiContent_ = std::make_unique<Ace::UIContentMocker>();
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
HWTEST_F(RootSceneTest, UpdateConfiguration, TestSize.Level1)
{
    RootScene rootScene;
    sptr<RootScene> staticRootScene = sptr<RootScene>::MakeSptr();
    wptr<Window> weakWindow(staticRootScene);
    rootScene.AddRootScene(DEFAULT_DISPLAY_ID, weakWindow);
    rootScene.AddRootScene(100, nullptr);
    std::shared_ptr<AppExecFwk::Configuration> configuration = std::make_shared<AppExecFwk::Configuration>();
    rootScene.uiContent_ = nullptr;
    rootScene.UpdateConfiguration(configuration);

    rootScene.uiContent_ = std::make_unique<Ace::UIContentMocker>();
    rootScene.UpdateConfiguration(configuration);
    ASSERT_EQ(1, rootScene.GetWindowId());
}

/**
 * @tc.name: UpdateConfigurationForSpecified
 * @tc.desc: UpdateConfigurationForSpecified Test
 * @tc.type: FUNC
 */
HWTEST_F(RootSceneTest, UpdateConfigurationForSpecified, TestSize.Level1)
{
    RootScene rootScene;
    std::shared_ptr<AppExecFwk::Configuration> configuration;
    std::shared_ptr<Global::Resource::ResourceManager> resourceManager;

    rootScene.uiContent_ = std::make_unique<Ace::UIContentMocker>();
    rootScene.UpdateConfigurationForSpecified(configuration, resourceManager);
    ASSERT_EQ(1, rootScene.GetWindowId());
}

/**
 * @tc.name: UpdateConfigurationForAll
 * @tc.desc: UpdateConfigurationForAll Test
 * @tc.type: FUNC
 */
HWTEST_F(RootSceneTest, UpdateConfigurationForAll, TestSize.Level1)
{
    RootScene rootScene;
    std::shared_ptr<AppExecFwk::Configuration> configuration = std::make_shared<AppExecFwk::Configuration>();

    auto prevStaticRootScene = RootScene::staticRootScene_;
    rootScene.UpdateConfigurationForAll(configuration);

    sptr<RootScene> staticRootScene = sptr<RootScene>::MakeSptr();
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
HWTEST_F(RootSceneTest, RegisterInputEventListener01, TestSize.Level1)
{
    sptr<RootScene> rootScene = sptr<RootScene>::MakeSptr();
    auto prevStaticRootScene = RootScene::staticRootScene_;

    RootScene::staticRootScene_ = nullptr;
    rootScene->RegisterInputEventListener();

    sptr<RootScene> staticRootScene = sptr<RootScene>::MakeSptr();
    RootScene::staticRootScene_ = staticRootScene;
    rootScene->RegisterInputEventListener();

    RootScene::staticRootScene_ = prevStaticRootScene;
    ASSERT_EQ(1, rootScene->GetWindowId());
}

/**
 * @tc.name: RequestVsyncErr
 * @tc.desc: RequestVsync Test Err
 * @tc.type: FUNC
 */
HWTEST_F(RootSceneTest, RequestVsyncErr, TestSize.Level1)
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
HWTEST_F(RootSceneTest, GetVSyncPeriod, TestSize.Level1)
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
HWTEST_F(RootSceneTest, FlushFrameRate, TestSize.Level1)
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
HWTEST_F(RootSceneTest, SetFrameLayoutFinishCallback, TestSize.Level1)
{
    RootScene rootScene;
    rootScene.SetFrameLayoutFinishCallback(nullptr);
    ASSERT_EQ(rootScene.frameLayoutFinishCb_, nullptr);

    rootScene.uiContent_ = std::make_unique<Ace::UIContentMocker>();
    rootScene.SetFrameLayoutFinishCallback(nullptr);
    ASSERT_EQ(rootScene.frameLayoutFinishCb_, nullptr);
    ASSERT_EQ(1, rootScene.GetWindowId());
}

/**
 * @tc.name: SetUiDvsyncSwitch
 * @tc.desc: SetUiDvsyncSwitch Test
 * @tc.type: FUNC
 */
HWTEST_F(RootSceneTest, SetUiDvsyncSwitchSucc, TestSize.Level1)
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
HWTEST_F(RootSceneTest, SetUiDvsyncSwitchErr, TestSize.Level1)
{
    RootScene rootScene;
    rootScene.SetUiDvsyncSwitch(true);
    rootScene.SetUiDvsyncSwitch(false);
    ASSERT_EQ(1, rootScene.GetWindowId());
}

/**
 * @tc.name: SetTouchEvent
 * @tc.desc: SetTouchEvent Test
 * @tc.type: FUNC
 */
HWTEST_F(RootSceneTest, SetTouchEventSucc, TestSize.Level1)
{
    RootScene rootScene;
    rootScene.SetTouchEvent(0);
    rootScene.SetTouchEvent(1);
    ASSERT_EQ(1, rootScene.GetWindowId());
}

/**
 * @tc.name: SetTouchEvent
 * @tc.desc: SetTouchEvent Test
 * @tc.type: FUNC
 */
HWTEST_F(RootSceneTest, SetTouchEventErr, TestSize.Level1)
{
    RootScene rootScene;
    rootScene.SetTouchEvent(0);
    rootScene.SetTouchEvent(1);
    ASSERT_EQ(1, rootScene.GetWindowId());
}

/**
 * @tc.name: IsLastFrameLayoutFinished
 * @tc.desc: IsLastFrameLayoutFinished Test
 * @tc.type: FUNC
 */
HWTEST_F(RootSceneTest, IsLastFrameLayoutFinished, TestSize.Level1)
{
    RootScene rootScene;
    auto ret = rootScene.IsLastFrameLayoutFinished();
    ASSERT_EQ(ret, true);
    ASSERT_EQ(1, rootScene.GetWindowId());
}

/**
 * @tc.name: OnFlushUIParams
 * @tc.desc: OnFlushUIParams Test
 * @tc.type: FUNC
 */
HWTEST_F(RootSceneTest, OnFlushUIParams, TestSize.Level1)
{
    RootScene rootScene;
    rootScene.OnFlushUIParams();
    ASSERT_EQ(1, rootScene.GetWindowId());
}

/**
 * @tc.name: OnBundleUpdated
 * @tc.desc: OnBundleUpdated Test
 * @tc.type: FUNC
 */
HWTEST_F(RootSceneTest, OnBundleUpdated, TestSize.Level1)
{
    RootScene rootScene;
    std::string bundleName = "test";
    rootScene.OnBundleUpdated(bundleName);
    ASSERT_EQ(1, rootScene.GetWindowId());
}

/**
 * @tc.name: SetDisplayOrientation
 * @tc.desc: SetDisplayOrientation Test01
 * @tc.type: FUNC
 */
HWTEST_F(RootSceneTest, SetDisplayOrientationTest01, TestSize.Level1)
{
    RootScene rootScene;
    int32_t orientation = 0;
    rootScene.SetDisplayOrientation(orientation);
    ASSERT_EQ(1, rootScene.GetWindowId());
}

/**
 * @tc.name: SetDisplayOrientation
 * @tc.desc: SetDisplayOrientation Test02
 * @tc.type: FUNC
 */
HWTEST_F(RootSceneTest, SetDisplayOrientationTest02, TestSize.Level1)
{
    RootScene rootScene;
    int32_t orientation = 100;
    rootScene.SetDisplayOrientation(orientation);
    ASSERT_EQ(1, rootScene.GetWindowId());
}

/**
 * @tc.name: RegisterAvoidAreaChangeListener
 * @tc.desc: RegisterAvoidAreaChangeListener Test
 * @tc.type: FUNC
 */
HWTEST_F(RootSceneTest, RegisterAvoidAreaChangeListener, TestSize.Level1)
{
    RootScene rootScene;
    rootScene.updateRootSceneAvoidAreaCallback_ = [] {};
    sptr<IAvoidAreaChangedListener> listener = sptr<IAvoidAreaChangedListener>::MakeSptr();
    auto ret = rootScene.RegisterAvoidAreaChangeListener(listener);
    ASSERT_EQ(WMError::WM_OK, ret);
    listener = nullptr;
    ret = rootScene.RegisterAvoidAreaChangeListener(listener);
    ASSERT_EQ(WMError::WM_ERROR_NULLPTR, ret);
}

/**
 * @tc.name: UnregisterAvoidAreaChangeListener
 * @tc.desc: UnregisterAvoidAreaChangeListener Test
 * @tc.type: FUNC
 */
HWTEST_F(RootSceneTest, UnregisterAvoidAreaChangeListener, TestSize.Level1)
{
    RootScene rootScene;
    sptr<IAvoidAreaChangedListener> listener = sptr<IAvoidAreaChangedListener>::MakeSptr();
    auto ret = rootScene.UnregisterAvoidAreaChangeListener(listener);
    ASSERT_EQ(WMError::WM_OK, ret);
    listener = nullptr;
    ret = rootScene.UnregisterAvoidAreaChangeListener(listener);
    ASSERT_EQ(WMError::WM_ERROR_NULLPTR, ret);
}

/**
 * @tc.name: NotifyAvoidAreaChangeForRoot
 * @tc.desc: NotifyAvoidAreaChangeForRoot Test
 * @tc.type: FUNC
 */
HWTEST_F(RootSceneTest, NotifyAvoidAreaChangeForRoot, TestSize.Level1)
{
    RootScene rootScene;
    sptr<IAvoidAreaChangedListener> listener = sptr<IAvoidAreaChangedListener>::MakeSptr();
    ASSERT_NE(nullptr, listener);
    rootScene.avoidAreaChangeListeners_.insert(listener);
    AvoidAreaType type = AvoidAreaType::TYPE_SYSTEM_GESTURE;
    AvoidArea avoidArea;
    rootScene.NotifyAvoidAreaChangeForRoot(new AvoidArea(avoidArea), type);
}

/**
 * @tc.name: GetAvoidAreaByType
 * @tc.desc: GetAvoidAreaByType Test err
 * @tc.type: FUNC
 */
HWTEST_F(RootSceneTest, GetAvoidAreaByType, TestSize.Level1)
{
    RootScene rootScene;
    AvoidAreaType type = AvoidAreaType::TYPE_SYSTEM_GESTURE;
    AvoidArea avoidArea;
    rootScene.getSessionAvoidAreaByTypeCallback_ = nullptr;

    auto ret = rootScene.GetAvoidAreaByType(type, avoidArea);
    ASSERT_EQ(WMError::WM_ERROR_NULLPTR, ret);
}

/**
 * @tc.name: GetAvoidAreaByTypeTest
 * @tc.desc: GetAvoidAreaByType Test err
 * @tc.type: FUNC
 */
HWTEST_F(RootSceneTest, GetAvoidAreaByTypeTest, TestSize.Level1)
{
    RootScene rootScene;
    AvoidAreaType type = AvoidAreaType::TYPE_SYSTEM_GESTURE;
    AvoidArea avoidArea;
    AvoidArea testAvoidArea;
    testAvoidArea.topRect_ = {1, 1, 1, 1};
    GetSessionAvoidAreaByTypeCallback func = [testAvoidArea](AvoidAreaType type, bool ignoreVisibility)->AvoidArea {
        return testAvoidArea;
    };
    rootScene.getSessionAvoidAreaByTypeCallback_ = func;

    auto ret = rootScene.GetAvoidAreaByType(type, avoidArea, Rect::EMPTY_RECT, 15);
    ASSERT_EQ(WMError::WM_DO_NOTHING, ret);
}

/**
 * @tc.name: GetAvoidAreaByTypeTest001
 * @tc.desc: GetAvoidAreaByType Test err
 * @tc.type: FUNC
 */
HWTEST_F(RootSceneTest, GetAvoidAreaByTypeTest001, TestSize.Level1)
{
    RootScene rootScene;
    AvoidAreaType type = AvoidAreaType::TYPE_SYSTEM_GESTURE;
    AvoidArea avoidArea;
    AvoidArea testAvoidArea;
    testAvoidArea.topRect_ = {1, 1, 1, 1};
    GetSessionAvoidAreaByTypeCallback func = [testAvoidArea](AvoidAreaType type, bool ignoreVisibility)->AvoidArea {
        return testAvoidArea;
    };
    rootScene.getSessionAvoidAreaByTypeCallback_ = func;

    constexpr int32_t API_VERSION_18 = 18;
    auto ret = rootScene.GetAvoidAreaByType(type, avoidArea, Rect::EMPTY_RECT, API_VERSION_18);
    ASSERT_EQ(WMError::WM_OK, ret);
    ASSERT_EQ(avoidArea.topRect_, testAvoidArea.topRect_);
}

/**
 * @tc.name: GetAvoidAreaByTypeTestIgnoringVisibility_SUCCESS
 * @tc.desc: GetAvoidAreaByTypeTestIgnoringVisibility_SUCCESS Test
 * @tc.type: FUNC
 */
HWTEST_F(RootSceneTest, GetAvoidAreaByTypeTestIgnoringVisibility_SUCCESS, TestSize.Level1)
{
    RootScene rootScene;
    AvoidAreaType type = AvoidAreaType::TYPE_SYSTEM_GESTURE;
    AvoidArea avoidArea;
    AvoidArea testAvoidArea;
    testAvoidArea.topRect_ = {1, 1, 1, 1};
    GetSessionAvoidAreaByTypeCallback func = [testAvoidArea](AvoidAreaType type, bool ignoreVisibility)->AvoidArea {
        return testAvoidArea;
    };
    rootScene.getSessionAvoidAreaByTypeCallback_ = func;

    rootScene.GetAvoidAreaByTypeIgnoringVisibility(type, avoidArea, Rect::EMPTY_RECT);
    EXPECT_TRUE(avoidArea == testAvoidArea);
}

/**
 * @tc.name: GetAvoidAreaByTypeTestIgnoringVisibility_ERR
 * @tc.desc: GetAvoidAreaByTypeTestIgnoringVisibility_ERR Test
 * @tc.type: FUNC
 */
HWTEST_F(RootSceneTest, GetAvoidAreaByTypeTestIgnoringVisibility_ERR, TestSize.Level1)
{
    RootScene rootScene;
    AvoidArea avoidArea;
    rootScene.getSessionAvoidAreaByTypeCallback_ = nullptr;

    auto ret = rootScene.GetAvoidAreaByTypeIgnoringVisibility(
        AvoidAreaType::TYPE_SYSTEM_GESTURE, avoidArea, Rect::EMPTY_RECT);
    EXPECT_EQ(WMError::WM_ERROR_NULLPTR, ret);
    ret = rootScene.GetAvoidAreaByTypeIgnoringVisibility(
        AvoidAreaType::TYPE_KEYBOARD, avoidArea, Rect::EMPTY_RECT);
    EXPECT_EQ(WMError::WM_ERROR_ILLEGAL_PARAM, ret);
}

/**
 * @tc.name: UpdateConfigurationSync
 * @tc.desc: UpdateConfigurationSync Test
 * @tc.type: FUNC
 */
HWTEST_F(RootSceneTest, UpdateConfigurationSync, TestSize.Level1)
{
    RootScene rootScene;
    sptr<RootScene> staticRootScene = sptr<RootScene>::MakeSptr();
    wptr<Window> weakWindow(staticRootScene);
    rootScene.AddRootScene(DEFAULT_DISPLAY_ID, weakWindow);
    rootScene.AddRootScene(100, nullptr);
    std::shared_ptr<AppExecFwk::Configuration> configuration = std::make_shared<AppExecFwk::Configuration>();

    rootScene.uiContent_ = nullptr;
    rootScene.UpdateConfigurationSync(configuration);
    rootScene.uiContent_ = std::make_unique<Ace::UIContentMocker>();
    rootScene.UpdateConfigurationSync(configuration);
    ASSERT_EQ(1, rootScene.GetWindowId());
}

/**
 * @tc.name: UpdateConfigurationSyncForAll
 * @tc.desc: UpdateConfigurationSyncForAll Test
 * @tc.type: FUNC
 */
HWTEST_F(RootSceneTest, UpdateConfigurationSyncForAll, TestSize.Level1)
{
    RootScene rootScene;
    std::shared_ptr<AppExecFwk::Configuration> configuration = std::make_shared<AppExecFwk::Configuration>();

    auto prevStaticRootScene = RootScene::staticRootScene_;
    rootScene.UpdateConfigurationSyncForAll(configuration);

    sptr<RootScene> staticRootScene;
    RootScene::staticRootScene_ = staticRootScene;
    rootScene.UpdateConfigurationSyncForAll(configuration);

    RootScene::staticRootScene_ = prevStaticRootScene;
    ASSERT_EQ(1, rootScene.GetWindowId());
}

/**
 * @tc.name: IsSystemWindow
 * @tc.desc: IsSystemWindow Test
 * @tc.type: FUNC
 */
HWTEST_F(RootSceneTest, IsSystemWindow, TestSize.Level1)
{
    RootScene rootScene;
    bool res = rootScene.IsSystemWindow();
    ASSERT_EQ(true, res);
}

/**
 * @tc.name: IsAppWindow
 * @tc.desc: IsAppWindow Test
 * @tc.type: FUNC
 */
HWTEST_F(RootSceneTest, IsAppWindow, TestSize.Level1)
{
    RootScene rootScene;
    bool res = rootScene.IsAppWindow();
    ASSERT_EQ(false, res);
}

/**
 * @tc.name: RegisterOccupiedAreaChangeListener
 * @tc.desc: RegisterOccupiedAreaChangeListener Test
 * @tc.type: FUNC
 */
HWTEST_F(RootSceneTest, RegisterOccupiedAreaChangeListener, TestSize.Level1)
{
    RootScene rootScene;
    sptr<IOccupiedAreaChangeListener> listener = sptr<IOccupiedAreaChangeListener>::MakeSptr();
    auto ret = rootScene.RegisterOccupiedAreaChangeListener(listener);
    ASSERT_EQ(WMError::WM_OK, ret);
    listener = nullptr;
    ret = rootScene.RegisterOccupiedAreaChangeListener(listener);
    ASSERT_EQ(WMError::WM_ERROR_NULLPTR, ret);
}

/**
 * @tc.name: UnregisterOccupiedAreaChangeListener
 * @tc.desc: UnregisterOccupiedAreaChangeListener Test
 * @tc.type: FUNC
 */
HWTEST_F(RootSceneTest, UnregisterOccupiedAreaChangeListener, TestSize.Level1)
{
    RootScene rootScene;
    sptr<IOccupiedAreaChangeListener> listener = sptr<IOccupiedAreaChangeListener>::MakeSptr();
    auto ret = rootScene.UnregisterOccupiedAreaChangeListener(listener);
    ASSERT_EQ(WMError::WM_OK, ret);
    listener = nullptr;
    ret = rootScene.UnregisterOccupiedAreaChangeListener(listener);
    ASSERT_EQ(WMError::WM_ERROR_NULLPTR, ret);
}

/**
 * @tc.name: GetRSNodeByStringIDTest
 * @tc.desc: For GetRSNodeByStringID Test
 * @tc.type: FUNC
 */
HWTEST_F(RootSceneTest, GetRSNodeByStringIDTest, TestSize.Level1)
{
    string stringId = "GetRSNodeByStringIDTest";
    sptr<RootScene> rootScene = sptr<RootScene>::MakeSptr();
    rootScene->uiContent_ = std::make_unique<Ace::UIContentMocker>();
    rootScene->SetTopWindowBoundaryByID(stringId);
    rootScene->OnBundleUpdated(stringId);

    auto res = rootScene->GetRSNodeByStringID(stringId);
    ASSERT_EQ(res, nullptr);
}

/**
 * @tc.name: GetRSNodeByStringIDTest001
 * @tc.desc: For GetRSNodeByStringID Test
 * @tc.type: FUNC
 */
HWTEST_F(RootSceneTest, GetRSNodeByStringIDTest001, TestSize.Level1)
{
    string stringId = "GetRSNodeByStringIDTest";
    sptr<RootScene> rootScene = sptr<RootScene>::MakeSptr();
    rootScene->uiContent_ = nullptr;
    rootScene->SetTopWindowBoundaryByID(stringId);
    rootScene->OnBundleUpdated(stringId);

    auto res = rootScene->GetRSNodeByStringID(stringId);
    ASSERT_EQ(res, nullptr);
}

/**
 * @tc.name: GetStatusBarHeight
 * @tc.desc: GetStatusBarHeight test
 * @tc.type: FUNC
 */
HWTEST_F(RootSceneTest, GetStatusBarHeight, TestSize.Level1)
{
    RootScene rootScene;
    auto height = rootScene.GetStatusBarHeight();
    EXPECT_EQ(0, height);
    rootScene.getStatusBarHeightCallback_ = []() -> uint32_t { return 100; };
    height = rootScene.GetStatusBarHeight();
    EXPECT_EQ(100, height);
}

/**
 * @tc.name: AddRootScene
 * @tc.desc: For AddRootScene Test
 * @tc.type: FUNC
 */
HWTEST_F(RootSceneTest, AddRootScene, TestSize.Level1)
{
    sptr<RootScene> rootScene = sptr<RootScene>::MakeSptr();
    sptr<ScreenScene> screenScene = sptr<ScreenScene>::MakeSptr("AddRootScene");
    rootScene->uiContent_ = std::make_unique<Ace::UIContentMocker>();
    wptr<Window> weakWindow(rootScene);
    wptr<Window> weakWindow1(screenScene);
    rootScene->AddRootScene(1, weakWindow);
    rootScene->AddRootScene(12, weakWindow1);

    auto res = rootScene->GetUIContentByDisplayId(1);
    ASSERT_EQ(res.second, true);
    res = rootScene->GetUIContentByDisplayId(12);
}

/**
 * @tc.name: RemoveRootScene
 * @tc.desc: For RemoveRootScene Test
 * @tc.type: FUNC
 */
HWTEST_F(RootSceneTest, RemoveRootScene, TestSize.Level1)
{
    sptr<RootScene> rootScene = sptr<RootScene>::MakeSptr();
    rootScene->uiContent_ = std::make_unique<Ace::UIContentMocker>();
    wptr<Window> weakWindow(rootScene);
    rootScene->AddRootScene(0, weakWindow);

    auto res = rootScene->GetUIContentByDisplayId(0);
    ASSERT_EQ(res.second, true);
    rootScene->RemoveRootScene(0);
    res = rootScene->GetUIContentByDisplayId(0);
    ASSERT_EQ(res.second, false);
}

} // namespace
} // namespace Rosen
} // namespace OHOS