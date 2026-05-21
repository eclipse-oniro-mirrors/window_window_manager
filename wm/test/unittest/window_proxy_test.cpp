/*
 * Copyright (c) 2022 Huawei Device Co., Ltd.
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
#include "window_proxy.h"

#include "window_agent.h"
#include "window_manager_hilog.h"

using namespace testing;
using namespace testing::ext;
namespace OHOS {
namespace Rosen {
namespace {
    std::string g_errLog;
    void MyLogCallback(const LogType type, const LogLevel level, const unsigned int domain, const char *tag,
        const char *msg)
    {
        g_errLog = msg;
    }
class WindowProxyTest : public testing::Test {
public:
    static void SetUpTestCase();
    static void TearDownTestCase();
    void SetUp() override;
    void TearDown() override;
    sptr<WindowAgent> mockWindowAgent_;
    sptr<WindowProxy> windowProxy_;
};

void WindowProxyTest::SetUpTestCase() {}

void WindowProxyTest::TearDownTestCase() {}

void WindowProxyTest::SetUp()
{
    sptr<WindowOption> option = new WindowOption();
    sptr<WindowImpl> window = new WindowImpl(option);
    mockWindowAgent_ = new WindowAgent(window);
    windowProxy_ = new WindowProxy(mockWindowAgent_);
}

void WindowProxyTest::TearDown() {}

namespace {
/**
 * @tc.name: UpdateWindowRect01
 * @tc.desc: normal function
 * @tc.type: FUNC
 */
HWTEST_F(WindowProxyTest, UpdateWindowRect01, TestSize.Level1)
{
    WMError err = windowProxy_->UpdateWindowRect(Rect{ 0, 0, 0, 0 }, false, WindowSizeChangeReason::HIDE);
    ASSERT_EQ(err, WMError::WM_OK);
}

/**
 * @tc.name: UpdateWindowMode01
 * @tc.desc: normal function
 * @tc.type: FUNC
 */
HWTEST_F(WindowProxyTest, UpdateWindowMode01, TestSize.Level1)
{
    WMError err = windowProxy_->UpdateWindowMode(WindowModeInfo{ WindowMode::WINDOW_MODE_FLOATING });
    ASSERT_EQ(err, WMError::WM_OK);
}

/**
 * @tc.name: UpdateWindowModeSupportType01
 * @tc.desc: normal function
 * @tc.type: FUNC
 */
HWTEST_F(WindowProxyTest, UpdateWindowModeSupportType01, TestSize.Level1)
{
    WMError err = windowProxy_->UpdateWindowModeSupportType(WindowModeSupport::WINDOW_MODE_SUPPORT_SPLIT_PRIMARY);
    ASSERT_EQ(err, WMError::WM_OK);
}

/**
 * @tc.name: UpdateFocusStatus01
 * @tc.desc: normal function
 * @tc.type: FUNC
 */
HWTEST_F(WindowProxyTest, UpdateFocusStatus01, TestSize.Level1)
{
    WMError err = windowProxy_->UpdateFocusStatus(false);
    ASSERT_EQ(err, WMError::WM_OK);
}

/**
 * @tc.name: UpdateAvoidArea01
 * @tc.desc: normal function
 * @tc.type: FUNC
 */
HWTEST_F(WindowProxyTest, UpdateAvoidArea01, TestSize.Level1)
{
    const sptr<AvoidArea>& avoidArea = new AvoidArea();
    WMError err = windowProxy_->UpdateAvoidArea(avoidArea, AvoidAreaType::TYPE_SYSTEM);
    ASSERT_EQ(err, WMError::WM_OK);
}

/**
 * @tc.name: UpdateWindowState01
 * @tc.desc: normal function
 * @tc.type: FUNC
 */
HWTEST_F(WindowProxyTest, UpdateWindowState01, TestSize.Level1)
{
    WMError err = windowProxy_->UpdateWindowState(WindowState::STATE_BOTTOM);
    ASSERT_EQ(err, WMError::WM_OK);
}

/**
 * @tc.name: UpdateWindowDragInfo01
 * @tc.desc: normal function
 * @tc.type: FUNC
 */
HWTEST_F(WindowProxyTest, UpdateWindowDragInfo01, TestSize.Level1)
{
    PointInfo point;
    point.x = 1;
    point.y = 2;
    WMError err = windowProxy_->UpdateWindowDragInfo(point, DragEvent::DRAG_EVENT_MOVE);
    ASSERT_EQ(err, WMError::WM_OK);
}

/**
 * @tc.name: UpdateDisplayId01
 * @tc.desc: normal function
 * @tc.type: FUNC
 */
HWTEST_F(WindowProxyTest, UpdateDisplayId01, TestSize.Level1)
{
    WMError err = windowProxy_->UpdateDisplayId(0, 1);
    ASSERT_EQ(err, WMError::WM_OK);
}

/**
 * @tc.name: UpdateOccupiedAreaChangeInfo01
 * @tc.desc: normal function
 * @tc.type: FUNC
 */
HWTEST_F(WindowProxyTest, UpdateOccupiedAreaChangeInfo01, TestSize.Level1)
{
    Rect overlapRect = { 0, 0, 0, 0 };
    sptr<OccupiedAreaChangeInfo> info = new OccupiedAreaChangeInfo(OccupiedAreaType::TYPE_INPUT, overlapRect);
    WMError err = windowProxy_->UpdateOccupiedAreaChangeInfo(info, {});
    ASSERT_EQ(err, WMError::WM_OK);
}

/**
 * @tc.name: UpdateActiveStatus01
 * @tc.desc: normal function
 * @tc.type: FUNC
 */
HWTEST_F(WindowProxyTest, UpdateActiveStatus01, TestSize.Level1)
{
    WMError err = windowProxy_->UpdateActiveStatus(false);
    ASSERT_EQ(err, WMError::WM_OK);
}

/**
 * @tc.name: NotifyTouchOutside01
 * @tc.desc: normal function
 * @tc.type: FUNC
 */
HWTEST_F(WindowProxyTest, NotifyTouchOutside01, TestSize.Level1)
{
    WMError err = windowProxy_->NotifyTouchOutside();
    ASSERT_EQ(err, WMError::WM_OK);
}

/**
 * @tc.name: DumpInfo01
 * @tc.desc: normal function
 * @tc.type: FUNC
 */
HWTEST_F(WindowProxyTest, DumpInfo01, TestSize.Level1)
{
    std::vector<std::string> params;
    WMError err = windowProxy_->DumpInfo(params);
    ASSERT_EQ(err, WMError::WM_OK);
}

/**
 * @tc.name: NotifyDestroy01
 * @tc.desc: normal function
 * @tc.type: FUNC
 */
HWTEST_F(WindowProxyTest, NotifyDestroy01, TestSize.Level1)
{
    WMError err = windowProxy_->NotifyDestroy();
    ASSERT_EQ(err, WMError::WM_OK);
}

/**
 * @tc.name: NotifyForeground01
 * @tc.desc: normal function
 * @tc.type: FUNC
 */
HWTEST_F(WindowProxyTest, NotifyForeground01, TestSize.Level1)
{
    WMError err = windowProxy_->NotifyForeground();
    ASSERT_EQ(err, WMError::WM_OK);
}

/**
 * @tc.name: NotifyBackground01
 * @tc.desc: normal function
 * @tc.type: FUNC
 */
HWTEST_F(WindowProxyTest, NotifyBackground01, TestSize.Level1)
{
    WMError err = windowProxy_->NotifyBackground();
    ASSERT_EQ(err, WMError::WM_OK);
}

/**
 * @tc.name: NotifyWindowClientPointUp01
 * @tc.desc: param is nullptr
 * @tc.type: FUNC
 */
HWTEST_F(WindowProxyTest, NotifyWindowClientPointUp01, TestSize.Level1)
{
    WMError err = windowProxy_->NotifyWindowClientPointUp(nullptr);
    ASSERT_EQ(err, WMError::WM_ERROR_NULLPTR);
}

/**
 * @tc.name: NotifyWindowClientPointUp02
 * @tc.desc: normal function
 * @tc.type: FUNC
 */
HWTEST_F(WindowProxyTest, NotifyWindowClientPointUp02, TestSize.Level1)
{
    auto pointerEvent = MMI::PointerEvent::Create();
    WMError err = windowProxy_->NotifyWindowClientPointUp(pointerEvent);
    ASSERT_EQ(err, WMError::WM_OK);
}

/**
 * @tc.name: UpdateZoomTransform01
 * @tc.desc: normal function
 * @tc.type: FUNC
 */
HWTEST_F(WindowProxyTest, UpdateZoomTransform01, TestSize.Level1)
{
    Transform transform;
    WMError err = windowProxy_->UpdateZoomTransform(transform, false);
    ASSERT_EQ(err, WMError::WM_OK);
}

/**
 * @tc.name: RestoreSplitWindowMode01
 * @tc.desc: normal function
 * @tc.type: FUNC
 */
HWTEST_F(WindowProxyTest, RestoreSplitWindowMode01, TestSize.Level1)
{
    WMError err = windowProxy_->RestoreSplitWindowMode(200);
    ASSERT_EQ(err, WMError::WM_OK);
}

/**
 * @tc.name: NotifyScreenshot
 * @tc.desc: normal function
 * @tc.type: FUNC
 */
HWTEST_F(WindowProxyTest, NotifyScreenshot, TestSize.Level1)
{
    WMError err = windowProxy_->NotifyScreenshot();
    ASSERT_EQ(err, WMError::WM_OK);
}

/**
 * @tc.name: NotifyScreenshotAppEvent
 * @tc.desc: normal function
 * @tc.type: FUNC
 */
HWTEST_F(WindowProxyTest, NotifyScreenshotAppEvent, TestSize.Level1)
{
    WMError err = windowProxy_->NotifyScreenshotAppEvent(ScreenshotEventType::SCROLL_SHOT_START);
    EXPECT_EQ(err, WMError::WM_OK);
}

/**
 * @tc.name: NotifyForegroundInteractiveStatus
 * @tc.desc: normal function
 * @tc.type: FUNC
 */
HWTEST_F(WindowProxyTest, NotifyForegroundInteractiveStatus, TestSize.Level1)
{
    g_errLog.clear();
    LOG_SetCallback(MyLogCallback);
    ASSERT_NE(windowProxy_, nullptr);
    bool interactive = false;
    windowProxy_->NotifyForegroundInteractiveStatus(interactive);
    EXPECT_FALSE(g_errLog.find("remote is null") != std::string::npos);
    LOG_SetCallback(nullptr);
}
}
} // namespace
} // namespace Rosen
} // namespace OHOS
