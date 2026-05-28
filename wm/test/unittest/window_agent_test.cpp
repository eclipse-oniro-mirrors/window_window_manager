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
#include <transaction/rs_sync_transaction_controller.h>

#include "rs_adapter.h"
#include "window_agent.h"
#include "window_manager_hilog.h"
#include "window_stub.h"

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
class WindowAgentTest : public testing::Test {
public:
    static void SetUpTestCase();
    static void TearDownTestCase();
    void SetUp() override;
    void TearDown() override;
    sptr<WindowAgent> windowAgent_;
};

void WindowAgentTest::SetUpTestCase() {}

void WindowAgentTest::TearDownTestCase()
{
}

void WindowAgentTest::SetUp()
{
    sptr<WindowOption> option = new WindowOption();
    sptr<WindowImpl> window = new WindowImpl(option);
    windowAgent_ = new WindowAgent(window);
}

void WindowAgentTest::TearDown()
{
    windowAgent_ = nullptr;
}

namespace {
/**
 * @tc.name: UpdateWindowRect
 * @tc.desc: UpdateWindowRect
 * @tc.type: FUNC
 */
HWTEST_F(WindowAgentTest, UpdateWindowRect, TestSize.Level1)
{
    Rect rect = Rect{ 0, 0, 0, 0 };
    bool status = true;
    WMError err = windowAgent_->UpdateWindowRect(rect, status, WindowSizeChangeReason::HIDE);
    ASSERT_EQ(err, WMError::WM_OK);

    windowAgent_->window_ = nullptr;
    err = windowAgent_->UpdateWindowRect(rect, status, WindowSizeChangeReason::HIDE);
    ASSERT_EQ(err, WMError::WM_ERROR_NULLPTR);
}

/**
 * @tc.name: UpdateWindowMode
 * @tc.desc: UpdateWindowMode
 * @tc.type: FUNC
 */
HWTEST_F(WindowAgentTest, UpdateWindowMode, TestSize.Level1)
{
    WMError err = windowAgent_->UpdateWindowMode(WindowModeInfo{ WindowMode::WINDOW_MODE_FULLSCREEN });
    ASSERT_EQ(err, WMError::WM_OK);

    windowAgent_->window_ = nullptr;
    err = windowAgent_->UpdateWindowMode(WindowModeInfo{ WindowMode::WINDOW_MODE_FULLSCREEN });
    ASSERT_EQ(err, WMError::WM_ERROR_NULLPTR);
}

/**
 * @tc.name: UpdateWindowModeSupportType
 * @tc.desc: UpdateWindowModeSupportType
 * @tc.type: FUNC
 */
HWTEST_F(WindowAgentTest, UpdateWindowModeSupportType, TestSize.Level1)
{
    WMError err = windowAgent_->UpdateWindowModeSupportType(WindowModeSupport::WINDOW_MODE_SUPPORT_FULLSCREEN);
    ASSERT_EQ(err, WMError::WM_OK);

    windowAgent_->window_ = nullptr;
    err = windowAgent_->UpdateWindowModeSupportType(WindowModeSupport::WINDOW_MODE_SUPPORT_FULLSCREEN);
    ASSERT_EQ(err, WMError::WM_ERROR_NULLPTR);
}

/**
 * @tc.name: UpdateFocusStatus
 * @tc.desc: UpdateFocusStatus
 * @tc.type: FUNC
 */
HWTEST_F(WindowAgentTest, UpdateFocusStatus, TestSize.Level1)
{
    WMError err = windowAgent_->UpdateFocusStatus(true);
    ASSERT_EQ(err, WMError::WM_OK);

    windowAgent_->window_ = nullptr;
    err = windowAgent_->UpdateFocusStatus(true);
    ASSERT_EQ(err, WMError::WM_ERROR_NULLPTR);
}

/**
 * @tc.name: UpdateAvoidArea
 * @tc.desc: UpdateAvoidArea
 * @tc.type: FUNC
 */
HWTEST_F(WindowAgentTest, UpdateAvoidArea, TestSize.Level1)
{
    const sptr<AvoidArea>& avoidArea = new AvoidArea();
    WMError err = windowAgent_->UpdateAvoidArea(avoidArea, AvoidAreaType::TYPE_SYSTEM);
    ASSERT_EQ(err, WMError::WM_OK);

    windowAgent_->window_ = nullptr;
    err = windowAgent_->UpdateAvoidArea(avoidArea, AvoidAreaType::TYPE_SYSTEM);
    ASSERT_EQ(err, WMError::WM_ERROR_NULLPTR);
}

/**
 * @tc.name: UpdateWindowState
 * @tc.desc: UpdateWindowState
 * @tc.type: FUNC
 */
HWTEST_F(WindowAgentTest, UpdateWindowState, TestSize.Level1)
{
    WMError err = windowAgent_->UpdateWindowState(WindowState::STATE_BOTTOM);
    ASSERT_EQ(err, WMError::WM_OK);

    windowAgent_->window_ = nullptr;
    err = windowAgent_->UpdateWindowState(WindowState::STATE_SHOWN);
    ASSERT_EQ(err, WMError::WM_ERROR_NULLPTR);
}

/**
 * @tc.name: UpdateWindowDragInfo
 * @tc.desc: UpdateWindowDragInfo
 * @tc.type: FUNC
 */
HWTEST_F(WindowAgentTest, UpdateWindowDragInfo, TestSize.Level1)
{
    PointInfo point;
    point.x = 1;
    point.y = 2;
    WMError err = windowAgent_->UpdateWindowDragInfo(point, DragEvent::DRAG_EVENT_MOVE);
    ASSERT_EQ(err, WMError::WM_OK);

    windowAgent_->window_ = nullptr;
    err = windowAgent_->UpdateWindowDragInfo(point, DragEvent::DRAG_EVENT_MOVE);
    ASSERT_EQ(err, WMError::WM_ERROR_NULLPTR);
}

/**
 * @tc.name: UpdateDisplayId
 * @tc.desc: UpdateDisplayId
 * @tc.type: FUNC
 */
HWTEST_F(WindowAgentTest, UpdateDisplayId, TestSize.Level1)
{
    WMError err = windowAgent_->UpdateDisplayId(0, 1);
    ASSERT_EQ(err, WMError::WM_OK);

    windowAgent_->window_ = nullptr;
    err = windowAgent_->UpdateDisplayId(0, 1);
    ASSERT_EQ(err, WMError::WM_ERROR_NULLPTR);
}

/**
 * @tc.name: UpdateOccupiedAreaChangeInfo
 * @tc.desc: UpdateOccupiedAreaChangeInfo
 * @tc.type: FUNC
 */
HWTEST_F(WindowAgentTest, UpdateOccupiedAreaChangeInfo, TestSize.Level1)
{
    Rect overlapRect = { 0, 0, 0, 0 };
    sptr<OccupiedAreaChangeInfo> info = new OccupiedAreaChangeInfo(OccupiedAreaType::TYPE_INPUT, overlapRect);
    WMError err = windowAgent_->UpdateOccupiedAreaChangeInfo(info, {});
    ASSERT_EQ(err, WMError::WM_OK);

    windowAgent_->window_ = nullptr;
    err = windowAgent_->UpdateOccupiedAreaChangeInfo(info, {});
    ASSERT_EQ(err, WMError::WM_ERROR_NULLPTR);
}

/**
 * @tc.name: UpdateOccupiedAreaAndRect
 * @tc.desc: UpdateOccupiedAreaAndRect
 * @tc.type: FUNC
 */
HWTEST_F(WindowAgentTest, UpdateOccupiedAreaAndRect, TestSize.Level1)
{
    Rect overlapRect = { 0, 0, 0, 0 };
    sptr<OccupiedAreaChangeInfo> info = new OccupiedAreaChangeInfo(OccupiedAreaType::TYPE_INPUT, overlapRect);
    std::shared_ptr<RSUIContext> rsUIContext;
    if (windowAgent_->window_) {
        rsUIContext = windowAgent_->window_->GetRSUIContext();
    }
    auto rsTransaction = RSSyncTransactionAdapter::GetRSTransaction(rsUIContext);

    WMError err = windowAgent_->UpdateOccupiedAreaAndRect(info, overlapRect, {}, rsTransaction);
    ASSERT_EQ(err, WMError::WM_OK);

    windowAgent_->window_ = nullptr;
    err = windowAgent_->UpdateOccupiedAreaAndRect(info, overlapRect, {}, rsTransaction);
    ASSERT_EQ(err, WMError::WM_ERROR_NULLPTR);
}

/**
 * @tc.name: UpdateActiveStatus
 * @tc.desc: UpdateActiveStatus
 * @tc.type: FUNC
 */
HWTEST_F(WindowAgentTest, UpdateActiveStatus, TestSize.Level1)
{
    WMError err = windowAgent_->UpdateActiveStatus(false);
    ASSERT_EQ(err, WMError::WM_OK);

    windowAgent_->window_ = nullptr;
    err = windowAgent_->UpdateActiveStatus(false);
    ASSERT_EQ(err, WMError::WM_ERROR_NULLPTR);
}

/**
 * @tc.name: GetWindowProperty
 * @tc.desc: GetWindowProperty
 * @tc.type: FUNC
 */
HWTEST_F(WindowAgentTest, GetWindowProperty, TestSize.Level1)
{
    auto windowProperty = windowAgent_->GetWindowProperty();
    ASSERT_NE(windowProperty, nullptr);

    windowAgent_->window_ = nullptr;
    windowProperty = windowAgent_->GetWindowProperty();
    ASSERT_EQ(windowProperty, nullptr);
}

/**
 * @tc.name: RestoreSplitWindowMode
 * @tc.desc: RestoreSplitWindowMode
 * @tc.type: FUNC
 */
HWTEST_F(WindowAgentTest, RestoreSplitWindowMode, TestSize.Level1)
{
    WMError err = windowAgent_->RestoreSplitWindowMode(static_cast<uint32_t>(WindowMode::WINDOW_MODE_SPLIT_PRIMARY));
    ASSERT_EQ(err, WMError::WM_OK);

    windowAgent_->window_ = nullptr;
    err = windowAgent_->RestoreSplitWindowMode(static_cast<uint32_t>(WindowMode::WINDOW_MODE_SPLIT_PRIMARY));
    ASSERT_EQ(err, WMError::WM_ERROR_NULLPTR);
}

/**
 * @tc.name: NotifyTouchOutside
 * @tc.desc: NotifyTouchOutside
 * @tc.type: FUNC
 */
HWTEST_F(WindowAgentTest, NotifyTouchOutside, TestSize.Level1)
{
    WMError err = windowAgent_->NotifyTouchOutside();
    ASSERT_EQ(err, WMError::WM_OK);

    windowAgent_->window_ = nullptr;
    err = windowAgent_->NotifyTouchOutside();
    ASSERT_EQ(err, WMError::WM_ERROR_NULLPTR);
}

/**
 * @tc.name: NotifyScreenshot
 * @tc.desc: NotifyScreenshot
 * @tc.type: FUNC
 */
HWTEST_F(WindowAgentTest, NotifyScreenshot, TestSize.Level1)
{
    WMError err = windowAgent_->NotifyScreenshot();
    ASSERT_EQ(err, WMError::WM_OK);

    windowAgent_->window_ = nullptr;
    err = windowAgent_->NotifyScreenshot();
    ASSERT_EQ(err, WMError::WM_ERROR_NULLPTR);
}

/**
 * @tc.name: NotifyScreenshotAppEvent
 * @tc.desc: NotifyScreenshotAppEvent
 * @tc.type: FUNC
 */
HWTEST_F(WindowAgentTest, NotifyScreenshotAppEvent, TestSize.Level1)
{
    WMError err = windowAgent_->NotifyScreenshotAppEvent(ScreenshotEventType::SCROLL_SHOT_START);
    EXPECT_EQ(err, WMError::WM_OK);

    windowAgent_->window_ = nullptr;
    err = windowAgent_->NotifyScreenshotAppEvent(ScreenshotEventType::SCROLL_SHOT_START);
    EXPECT_EQ(err, WMError::WM_ERROR_NULLPTR);
}

/**
 * @tc.name: DumpInfo
 * @tc.desc: DumpInfo
 * @tc.type: FUNC
 */
HWTEST_F(WindowAgentTest, DumpInfo, TestSize.Level1)
{
    std::vector<std::string> params;
    WMError err = windowAgent_->DumpInfo(params);
    ASSERT_EQ(err, WMError::WM_OK);

    windowAgent_->window_ = nullptr;
    err = windowAgent_->DumpInfo(params);
    ASSERT_EQ(err, WMError::WM_ERROR_NULLPTR);
}

/**
 * @tc.name: UpdateZoomTransform
 * @tc.desc: UpdateZoomTransform
 * @tc.type: FUNC
 */
HWTEST_F(WindowAgentTest, UpdateZoomTransform, TestSize.Level1)
{
    Transform transform;
    WMError err = windowAgent_->UpdateZoomTransform(transform, false);
    ASSERT_EQ(err, WMError::WM_OK);

    windowAgent_->window_ = nullptr;
    err = windowAgent_->UpdateZoomTransform(transform, false);
    ASSERT_EQ(err, WMError::WM_ERROR_NULLPTR);
}

/**
 * @tc.name: NotifyDestroy
 * @tc.desc: NotifyDestroy
 * @tc.type: FUNC
 */
HWTEST_F(WindowAgentTest, NotifyDestroy, TestSize.Level1)
{
    WMError err = windowAgent_->NotifyDestroy();
    ASSERT_EQ(err, WMError::WM_OK);

    windowAgent_->window_ = nullptr;
    err = windowAgent_->NotifyDestroy();
    ASSERT_EQ(err, WMError::WM_ERROR_NULLPTR);
}

/**
 * @tc.name: NotifyForeground
 * @tc.desc: NotifyForeground
 * @tc.type: FUNC
 */
HWTEST_F(WindowAgentTest, NotifyForeground, TestSize.Level1)
{
    WMError err = windowAgent_->NotifyForeground();
    ASSERT_EQ(err, WMError::WM_OK);

    windowAgent_->window_ = nullptr;
    err = windowAgent_->NotifyForeground();
    ASSERT_EQ(err, WMError::WM_ERROR_NULLPTR);
}

/**
 * @tc.name: NotifyBackground
 * @tc.desc: NotifyBackground
 * @tc.type: FUNC
 */
HWTEST_F(WindowAgentTest, NotifyBackground, TestSize.Level1)
{
    WMError err = windowAgent_->NotifyBackground();
    ASSERT_EQ(err, WMError::WM_OK);

    windowAgent_->window_ = nullptr;
    err = windowAgent_->NotifyBackground();
    ASSERT_EQ(err, WMError::WM_ERROR_NULLPTR);
}

/**
 * @tc.name: NotifyWindowClientPointUp
 * @tc.desc: NotifyWindowClientPointUp
 * @tc.type: FUNC
 */
HWTEST_F(WindowAgentTest, NotifyWindowClientPointUp, TestSize.Level1)
{
    auto pointerEvent = MMI::PointerEvent::Create();
    WMError err = windowAgent_->NotifyWindowClientPointUp(pointerEvent);
    ASSERT_EQ(err, WMError::WM_OK);

    windowAgent_->window_ = nullptr;
    err = windowAgent_->NotifyWindowClientPointUp(pointerEvent);
    ASSERT_EQ(err, WMError::WM_ERROR_NULLPTR);
}

/**
 * @tc.name: ConsumeKeyEvent
 * @tc.desc: ConsumeKeyEvent
 * @tc.type: FUNC
 */
HWTEST_F(WindowAgentTest, ConsumeKeyEvent, TestSize.Level1)
{
    g_errLog.clear();
    LOG_SetCallback(MyLogCallback);
    auto keyEvent = MMI::KeyEvent::Create();
    windowAgent_->ConsumeKeyEvent(keyEvent);

    windowAgent_->window_ = nullptr;
    windowAgent_->ConsumeKeyEvent(keyEvent);
    EXPECT_TRUE(g_errLog.find("window_ is nullptr") != std::string::npos);
    LOG_SetCallback(nullptr);
}

/**
 * @tc.name: NotifyForegroundInteractiveStatus
 * @tc.desc: NotifyForegroundInteractiveStatus
 * @tc.type: FUNC
 */
HWTEST_F(WindowAgentTest, NotifyForegroundInteractiveStatus, TestSize.Level1)
{
    g_errLog.clear();
    LOG_SetCallback(MyLogCallback);
    bool interactive = false;
    windowAgent_->NotifyForegroundInteractiveStatus(interactive);

    interactive = true;
    windowAgent_->window_ = nullptr;
    windowAgent_->NotifyForegroundInteractiveStatus(interactive);
    EXPECT_TRUE(g_errLog.find("window_ is nullptr") != std::string::npos);
    LOG_SetCallback(nullptr);
}
}
} // namespace
} // namespace Rosen
} // namespace OHOS