/*
 * Copyright (c) 2021-2022 Huawei Device Co., Ltd.
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

// gtest
#include <gtest/gtest.h>
#include "window_test_utils.h"
#include "wm_common.h"
#include "window_adapter.h"
#include "window_scene_session_impl.h"
#include "ability_context_impl.h"
#include "mock_session.h"
#include "session/host/include/scene_session.h"


using namespace testing;
using namespace testing::ext;

namespace OHOS {
namespace Rosen {
using Utils = WindowTestUtils;
class WindowLayoutTest : public testing::Test {
public:
    static void SetUpTestCase();
    static void TearDownTestCase();
    virtual void SetUp() override;
    virtual void TearDown() override;
    DisplayId displayId_ = 0;
    std::vector<sptr<Window>> activeWindows_;
    static vector<Rect> fullScreenExpecteds_;
    static inline float virtualPixelRatio_ = 0.0;
private:
    static constexpr uint32_t WAIT_SYANC_US = 100000;
    static void InitAvoidArea();
    std::shared_ptr<AbilityRuntime::AbilityContext> abilityContext_;
};

vector<Rect> WindowLayoutTest::fullScreenExpecteds_;

void WindowLayoutTest::SetUpTestCase()
{
    SingletonContainer::Get<WindowAdapter>().MinimizeAllAppWindows(0);
    sleep(2);
    auto display = DisplayManager::GetInstance().GetDisplayById(0);
    if (display == nullptr) {
        return;
    }
    ASSERT_TRUE((display != nullptr));
    Rect displayRect = {0, 0, display->GetWidth(), display->GetHeight()};
    Utils::InitByDisplayRect(displayRect);

    virtualPixelRatio_ = WindowTestUtils::GetVirtualPixelRatio(0);

    // calc expected rects
    Rect expected = { // 0. only statusBar
        0,
        Utils::statusBarRect_.height_,
        Utils::displayRect_.width_,
        Utils::displayRect_.height_ - Utils::statusBarRect_.height_,
    };
    fullScreenExpecteds_.push_back(expected);
    expected = { // 1. both statusBar and naviBar
        0,
        Utils::statusBarRect_.height_,
        Utils::displayRect_.width_,
        Utils::displayRect_.height_ - Utils::statusBarRect_.height_ - Utils::naviBarRect_.height_,
    };
    fullScreenExpecteds_.push_back(expected);
    expected = { // 2. only naviBar
        0,
        0,
        Utils::displayRect_.width_,
        Utils::displayRect_.height_ - Utils::naviBarRect_.height_,
    };
    fullScreenExpecteds_.push_back(expected);
    InitAvoidArea();
    sleep(2);
}

void WindowLayoutTest::InitAvoidArea()
{
    Utils::TestWindowInfo info = {
        .name = "avoidArea",
        .rect = {0, 0, 0, 0},
        .type = WindowType::WINDOW_TYPE_APP_MAIN_WINDOW,
        .mode = WindowMode::WINDOW_MODE_FLOATING,
        .needAvoid = true,
        .parentLimit = false,
        .parentId = INVALID_WINDOW_ID,
    };
    const sptr<Window>& window = Utils::CreateTestWindow(info);
    if (window == nullptr) {
        return;
    }
    window->Show();
    window->SetLayoutFullScreen(true);
    window->GetAvoidAreaByType(AvoidAreaType::TYPE_SYSTEM, WindowTestUtils::systemAvoidArea_);
    window->Hide();
    window->Destroy();
}

void WindowLayoutTest::TearDownTestCase()
{
}

void WindowLayoutTest::SetUp()
{
    activeWindows_.clear();
    abilityContext_ = std::make_shared<AbilityRuntime::AbilityContextImpl>();
}

void WindowLayoutTest::TearDown()
{
    abilityContext_ = nullptr;
}

namespace {
/**
 * @tc.name: LayoutWindow01
 * @tc.desc: One FLOATING APP Window with on custom rect
 * @tc.type: FUNC
 */
HWTEST_F(WindowLayoutTest, LayoutWindow01, Function | MediumTest | Level3)
{
    WindowManager::GetInstance().SetWindowLayoutMode(WindowLayoutMode::TILE);
    WindowManager::GetInstance().SetWindowLayoutMode(WindowLayoutMode::CASCADE);
    WindowManager::GetInstance().SetWindowLayoutMode(WindowLayoutMode::TILE);
    WindowManager::GetInstance().SetWindowLayoutMode(WindowLayoutMode::CASCADE);

    Utils::TestWindowInfo info = {
        .name = "main1",
        .rect = {0, 0, 0, 0},
        .type = WindowType::WINDOW_TYPE_APP_MAIN_WINDOW,
        .mode = WindowMode::WINDOW_MODE_FLOATING,
        .needAvoid = true,
        .parentLimit = false,
        .showWhenLocked = true,
        .parentId = INVALID_WINDOW_ID,
    };
    const sptr<Window>& window = Utils::CreateTestWindow(info);
    if (window == nullptr) {
        return;
    }
    ASSERT_EQ(true, window != nullptr);
    activeWindows_.push_back(window);
    Rect expect = Utils::GetDefaultFloatingRect(window, true);
    ASSERT_EQ(WMError::WM_OK, window->Show());
    ASSERT_TRUE(Utils::RectEqualTo(window, Utils::GetFloatingLimitedRect(expect, virtualPixelRatio_)));
    ASSERT_EQ(WMError::WM_OK, window->Hide());
}

/**
 * @tc.name: LayoutWindow02
 * @tc.desc: One FLOATING APP Window
 * @tc.type: FUNC
 */
HWTEST_F(WindowLayoutTest, LayoutWindow02, Function | MediumTest | Level3)
{
    Rect res = Utils::GetFloatingLimitedRect(Utils::customAppRect_, virtualPixelRatio_);
    Utils::TestWindowInfo info = {
        .name = "main2",
        .rect = res,
        .type = WindowType::WINDOW_TYPE_APP_MAIN_WINDOW,
        .mode = WindowMode::WINDOW_MODE_FLOATING,
        .needAvoid = true,
        .parentLimit = false,
        .showWhenLocked = true,
        .parentId = INVALID_WINDOW_ID,
    };
    const sptr<Window>& window = Utils::CreateTestWindow(info);
    if (window == nullptr) {
        return;
    }
    activeWindows_.push_back(window);

    ASSERT_EQ(WMError::WM_OK, window->Show());
    if (window->IsDecorEnable()) {
        ASSERT_TRUE(Utils::RectEqualTo(window, Utils::GetDecorateRect(res, virtualPixelRatio_)));
    } else {
        ASSERT_TRUE(Utils::RectEqualTo(window, res));
    }
    ASSERT_EQ(WMError::WM_OK, window->Hide());
}

/**
 * @tc.name: LayoutWindow04
 * @tc.desc: One FLOATING APP Window & One StatusBar Window
 * @tc.type: FUNC
 */
HWTEST_F(WindowLayoutTest, LayoutWindow04, Function | MediumTest | Level3)
{
    // app window
    Rect res = Utils::GetFloatingLimitedRect(Utils::customAppRect_, virtualPixelRatio_);
    Utils::TestWindowInfo info = {
        .name = "main4",
        .rect = res,
        .type = WindowType::WINDOW_TYPE_APP_MAIN_WINDOW,
        .mode = WindowMode::WINDOW_MODE_FLOATING,
        .needAvoid = true,
        .parentLimit = false,
        .showWhenLocked = true,
        .parentId = INVALID_WINDOW_ID,
    };
    sptr<Window> appWin = Utils::CreateTestWindow(info);
    if (appWin == nullptr) {
        return;
    }

    activeWindows_.push_back(appWin);

    // statusBar window
    sptr<Window> statBar = Utils::CreateStatusBarWindow();
    activeWindows_.push_back(statBar);

    ASSERT_EQ(WMError::WM_OK, appWin->Show());
    if (appWin->IsDecorEnable()) {
        ASSERT_TRUE(Utils::RectEqualTo(appWin, Utils::GetDecorateRect(res, virtualPixelRatio_)));
    } else {
        ASSERT_TRUE(Utils::RectEqualTo(appWin, res));
    }
    ASSERT_EQ(WMError::WM_OK, statBar->Show());
    if (appWin->IsDecorEnable()) {
        ASSERT_TRUE(Utils::RectEqualTo(appWin, Utils::GetDecorateRect(res, virtualPixelRatio_)));
    } else {
        ASSERT_TRUE(Utils::RectEqualTo(appWin, res));
    }
    ASSERT_EQ(WMError::WM_OK, statBar->Hide());
    if (appWin->IsDecorEnable()) {
        ASSERT_TRUE(Utils::RectEqualTo(appWin, Utils::GetDecorateRect(res, virtualPixelRatio_)));
    } else {
        ASSERT_TRUE(Utils::RectEqualTo(appWin, res));
    }
}

/**
 * @tc.name: LayoutWindow06
 * @tc.desc: StatusBar Window and NaviBar & Sys Window FULLSCRENN,NOT NEEDVOID,PARENTLIMIT
 * @tc.type: FUNC
 */
HWTEST_F(WindowLayoutTest, LayoutWindow06, Function | MediumTest | Level3)
{
    sptr<Window> statBar = Utils::CreateStatusBarWindow();
    if (statBar == nullptr) {
        return;
    }
    activeWindows_.push_back(statBar);
    sptr<Window> naviBar = Utils::CreateNavigationBarWindow();
    activeWindows_.push_back(naviBar);
    Utils::TestWindowInfo info = {
        .name = "main6",
        .rect = Utils::customAppRect_,
        .type = WindowType::WINDOW_TYPE_PANEL,
        .mode = WindowMode::WINDOW_MODE_FULLSCREEN,
        .needAvoid = false,
        .parentLimit = true,
        .showWhenLocked = true,
        .parentId = INVALID_WINDOW_ID,
    };
    sptr<Window> sysWin = Utils::CreateTestWindow(info);
    ASSERT_NE(sysWin, nullptr);
    activeWindows_.push_back(sysWin);
    if (statBar->Show() == WMError::WM_OK) {
        ASSERT_EQ(WMError::WM_OK, statBar->Show());
    } else if (statBar->Show() == WMError::WM_ERROR_INVALID_WINDOW) {
        ASSERT_EQ(WMError::WM_ERROR_INVALID_WINDOW, statBar->Show());
    }
    sysWin->Show();
    if (Utils::RectEqualTo(sysWin, Utils::displayRect_)) {
        ASSERT_TRUE(Utils::RectEqualTo(sysWin, Utils::displayRect_));
    }
    if (Utils::RectEqualTo(sysWin, Utils::displayRect_)) {
        ASSERT_TRUE(Utils::RectEqualTo(sysWin, Utils::displayRect_));
    } else {
        ASSERT_FALSE(Utils::RectEqualTo(sysWin, Utils::displayRect_));
    }
    if (WMError::WM_OK == naviBar->Show()) {
        ASSERT_EQ(WMError::WM_OK, naviBar->Show());
    }
    if (Utils::RectEqualTo(sysWin, Utils::displayRect_)) {
        ASSERT_TRUE(Utils::RectEqualTo(sysWin, Utils::displayRect_));
    }
    if (WMError::WM_OK == statBar->Hide()) {
        ASSERT_EQ(WMError::WM_OK, statBar->Hide());
    }
    if (Utils::RectEqualTo(sysWin, Utils::displayRect_)) {
        ASSERT_TRUE(Utils::RectEqualTo(sysWin, Utils::displayRect_));
    }
}

/**
 * @tc.name: LayoutWindow07
 * @tc.desc: StatusBar Window and NaviBar & One Floating Sys Window
 * @tc.type: FUNC
 */
HWTEST_F(WindowLayoutTest, LayoutWindow07, Function | MediumTest | Level3)
{
    // statusBar window
    sptr<Window> statBar = Utils::CreateStatusBarWindow();
    if (statBar == nullptr) {
        return;
    }
    activeWindows_.push_back(statBar);

    // naviBar window
    sptr<Window> naviBar = Utils::CreateNavigationBarWindow();
    ASSERT_NE(naviBar, nullptr);
    activeWindows_.push_back(naviBar);
    // sys window
    Utils::TestWindowInfo info = {
        .name = "main7",
        .rect = Utils::customAppRect_,
        .type = WindowType::WINDOW_TYPE_PANEL,
        .mode = WindowMode::WINDOW_MODE_FLOATING,
        .needAvoid = false,
        .parentLimit = true,
        .showWhenLocked = true,
        .parentId = INVALID_WINDOW_ID,
    };
    sptr<Window> sysWin = Utils::CreateTestWindow(info);
    ASSERT_NE(sysWin, nullptr);
    activeWindows_.push_back(sysWin);
    if (statBar->Show() == WMError::WM_OK) {
        ASSERT_EQ(WMError::WM_OK, statBar->Show());
    } else if (statBar->Show() == WMError::WM_ERROR_INVALID_WINDOW) {
        ASSERT_EQ(WMError::WM_ERROR_INVALID_WINDOW, statBar->Show());
    }
    sysWin->Show();
    
    ASSERT_TRUE(Utils::RectEqualTo(sysWin, Utils::customAppRect_));

    if (WMError::WM_OK == naviBar->Show()) {
        ASSERT_EQ(WMError::WM_OK, naviBar->Show());
    } else {
        ASSERT_NE(WMError::WM_OK, naviBar->Show());
    }

    ASSERT_TRUE(Utils::RectEqualTo(sysWin, Utils::customAppRect_));
    if (statBar->Hide() == WMError::WM_OK) {
        ASSERT_EQ(WMError::WM_OK, statBar->Hide());
    } else if (statBar->Hide() == WMError::WM_ERROR_INVALID_WINDOW) {
        ASSERT_EQ(WMError::WM_ERROR_INVALID_WINDOW, statBar->Hide());
    }
    ASSERT_TRUE(Utils::RectEqualTo(sysWin, Utils::customAppRect_));
}

/**
 * @tc.name: LayoutWindow08
 * @tc.desc: One FLOATING APP Window with on custom rect
 * @tc.type: FUNC
 */
HWTEST_F(WindowLayoutTest, LayoutWindow08, Function | MediumTest | Level3)
{
    Utils::TestWindowInfo info = {
        .name = "main8",
        .rect = {0, 0, 0, 0},
        .type = WindowType::WINDOW_TYPE_APP_MAIN_WINDOW,
        .mode = WindowMode::WINDOW_MODE_FLOATING,
        .needAvoid = true,
        .parentLimit = false,
        .showWhenLocked = true,
        .parentId = INVALID_WINDOW_ID,
    };
    const sptr<Window>& window = Utils::CreateTestWindow(info);
    if (window == nullptr) {
        return;
    }
    activeWindows_.push_back(window);
    Rect expect = Utils::GetDefaultFloatingRect(window, true);
    ASSERT_EQ(WMError::WM_OK, window->Show());
    usleep(WAIT_SYANC_US);
    ASSERT_TRUE(Utils::RectEqualTo(window, expect));
    ASSERT_EQ(WMError::WM_OK, window->Hide());
    usleep(WAIT_SYANC_US);
}

/**
 * @tc.name: LayoutWindow09
 * @tc.desc: Add a floating and resize(2, 2)
 * @tc.type: FUNC
 */
HWTEST_F(WindowLayoutTest, LayoutWindow09, Function | MediumTest | Level3)
{
    Utils::TestWindowInfo info = {
        .name = "main9",
        .rect = {0, 0, 0, 0},
        .type = WindowType::WINDOW_TYPE_APP_MAIN_WINDOW,
        .mode = WindowMode::WINDOW_MODE_FLOATING,
        .needAvoid = true,
        .parentLimit = false,
        .showWhenLocked = true,
        .parentId = INVALID_WINDOW_ID,
    };
    const sptr<Window>& window = Utils::CreateTestWindow(info);
    if (window == nullptr) {
        return;
    }
    activeWindows_.push_back(window);
    Rect expect = Utils::GetDefaultFloatingRect(window, true);

    ASSERT_EQ(WMError::WM_OK, window->Show());
    usleep(WAIT_SYANC_US);
    ASSERT_TRUE(Utils::RectEqualTo(window, expect));

    ASSERT_EQ(WMError::WM_OK, window->Resize(2u, 2u));        // 2: custom min size
    Rect finalExcept = { expect.posX_, expect.posY_, 2u, 2u}; // 2: custom min size
    finalExcept = Utils::GetFloatingLimitedRect(finalExcept, virtualPixelRatio_);
    ASSERT_TRUE(Utils::RectEqualTo(window, finalExcept));
    ASSERT_EQ(WMError::WM_OK, window->Hide());
}

/**
 * @tc.name: LayoutWindow10
 * @tc.desc: One FLOATING APP Window do max and recovery
 * @tc.type: FUNC
 */
HWTEST_F(WindowLayoutTest, LayoutWindow10, Function | MediumTest | Level3)
{
    Utils::TestWindowInfo info = {
        .name = "main10",
        .rect = {0, 0, 0, 0},
        .type = WindowType::WINDOW_TYPE_APP_MAIN_WINDOW,
        .mode = WindowMode::WINDOW_MODE_FLOATING,
        .needAvoid = true,
        .parentLimit = false,
        .showWhenLocked = true,
        .parentId = INVALID_WINDOW_ID,
    };
    const sptr<Window>& window = Utils::CreateTestWindow(info);
    if (window == nullptr) {
        return;
    }
    activeWindows_.push_back(window);
    Rect expect = Utils::GetDefaultFloatingRect(window, true);
    ASSERT_EQ(WMError::WM_OK, window->Show());
    usleep(WAIT_SYANC_US);
    ASSERT_TRUE(Utils::RectEqualTo(window, expect));
    ASSERT_EQ(WMError::WM_OK, window->Maximize());
    usleep(WAIT_SYANC_US);
    ASSERT_TRUE(Utils::RectEqualTo(window, Utils::displayRect_));
    ASSERT_EQ(WMError::WM_OK, window->Recover());
    usleep(WAIT_SYANC_US);
    ASSERT_TRUE(Utils::RectEqualTo(window, expect));
    ASSERT_EQ(WMError::WM_OK, window->Minimize());
    usleep(WAIT_SYANC_US);
    ASSERT_EQ(WMError::WM_OK, window->Close());
}

/**
 * @tc.name: LayoutTile01
 * @tc.desc: One FLOATING APP Window into tile mode, show 4 new window
 * @tc.type: FUNC
 */
HWTEST_F(WindowLayoutTest, LayoutTile01, Function | MediumTest | Level3)
{
    Utils::TestWindowInfo info = {
        .name = "mainTile1", .rect = {0, 0, 0, 0}, .type = WindowType::WINDOW_TYPE_APP_MAIN_WINDOW,
        .mode = WindowMode::WINDOW_MODE_FLOATING, .needAvoid = true, .parentLimit = false,
        .parentId = INVALID_WINDOW_ID,
    };

    const sptr<Window>& window = Utils::CreateTestWindow(info);
    if (window == nullptr) {
        return;
    }
    activeWindows_.push_back(window);
    Rect expect = Utils::GetDefaultFloatingRect(window, true);
    ASSERT_EQ(WMError::WM_OK, window->Show());
    usleep(WAIT_SYANC_US);
    // init tile window rects and get max tile window num
    Utils::InitTileWindowRects(window, false);
    uint32_t maxTileNum = Utils::GetMaxTileWinNum();
    if (maxTileNum < 1) {
        return;
    }

    usleep(WAIT_SYANC_US);
    ASSERT_TRUE(Utils::RectEqualTo(window, expect));
    WindowManager::GetInstance().SetWindowLayoutMode(WindowLayoutMode::TILE);
    usleep(WAIT_SYANC_US);
    ASSERT_TRUE(Utils::RectEqualTo(window, Utils::singleTileRect_));

    info.name = "test1";
    const sptr<Window>& test1 = Utils::CreateTestWindow(info);
    ASSERT_NE(nullptr, test1);
    activeWindows_.push_back(test1);
    ASSERT_EQ(WMError::WM_OK, test1->Show());
    usleep(WAIT_SYANC_US);
    if (maxTileNum == 1) {
        ASSERT_TRUE(Utils::RectEqualTo(test1, Utils::singleTileRect_));
        WindowManager::GetInstance().SetWindowLayoutMode(WindowLayoutMode::CASCADE);
        return;
    }
    ASSERT_TRUE(Utils::RectEqualTo(window, Utils::doubleTileRects_[0]));
    ASSERT_TRUE(Utils::RectEqualTo(test1, Utils::doubleTileRects_[1]));

    info.name = "test2";
    const sptr<Window>& test2 = Utils::CreateTestWindow(info);
    ASSERT_NE(nullptr, test2);
    activeWindows_.push_back(test2);
    ASSERT_EQ(WMError::WM_OK, test2->Show());
    usleep(WAIT_SYANC_US);
    if (maxTileNum == 2) {
        ASSERT_TRUE(Utils::RectEqualTo(test1, Utils::doubleTileRects_[0]));
        ASSERT_TRUE(Utils::RectEqualTo(test2, Utils::doubleTileRects_[1]));
    } else {
        ASSERT_TRUE(Utils::RectEqualTo(window, Utils::tripleTileRects_[0]));
        ASSERT_TRUE(Utils::RectEqualTo(test1, Utils::tripleTileRects_[1]));
        ASSERT_TRUE(Utils::RectEqualTo(test2, Utils::tripleTileRects_[2])); // 2 is second rect idx
    }
    WindowManager::GetInstance().SetWindowLayoutMode(WindowLayoutMode::CASCADE);
}

/**
 * @tc.name: LayoutTileNegative01
 * @tc.desc: negative test for tile window
 * @tc.type: FUNC
 */
HWTEST_F(WindowLayoutTest, LayoutTileNegative01, Function | MediumTest | Level3)
{
    WindowManager::GetInstance().SetWindowLayoutMode(WindowLayoutMode::CASCADE);
    Utils::TestWindowInfo info = {
        .name = "mainTileNegative1", .rect = {-1, -100, -1, -100}, // -1, -100, -1, -100 is typical negative case nums
        .type = WindowType::WINDOW_TYPE_APP_MAIN_WINDOW, .mode = WindowMode::WINDOW_MODE_FLOATING,
        .needAvoid = true, .parentLimit = false, .parentId = INVALID_WINDOW_ID,
    };
    const sptr<Window>& window = Utils::CreateTestWindow(info);
    if (window == nullptr) {
        return;
    }
    activeWindows_.push_back(window);
    ASSERT_EQ(WMError::WM_OK, window->Show());
    usleep(WAIT_SYANC_US);
    // init tile window rects and get max tile window num
    Utils::InitTileWindowRects(window, false);
    uint32_t maxTileNum = Utils::GetMaxTileWinNum();
    if (maxTileNum < 1) {
        return;
    }

    usleep(WAIT_SYANC_US);
    WindowManager::GetInstance().SetWindowLayoutMode(WindowLayoutMode::TILE);
    usleep(WAIT_SYANC_US);
    ASSERT_TRUE(Utils::RectEqualTo(window, Utils::singleTileRect_));

    info.name = "test1";
    const sptr<Window>& test1 = Utils::CreateTestWindow(info);
    ASSERT_NE(nullptr, test1);
    activeWindows_.push_back(test1);
    ASSERT_EQ(WMError::WM_OK, test1->Show());
    usleep(WAIT_SYANC_US);
    if (maxTileNum == 1) {
        ASSERT_TRUE(Utils::RectEqualTo(test1, Utils::singleTileRect_));
        WindowManager::GetInstance().SetWindowLayoutMode(WindowLayoutMode::CASCADE);
        return;
    }
    ASSERT_TRUE(Utils::RectEqualTo(window, Utils::doubleTileRects_[0]));
    ASSERT_TRUE(Utils::RectEqualTo(test1, Utils::doubleTileRects_[1]));

    info.name = "test2";
    const sptr<Window>& test2 = Utils::CreateTestWindow(info);
    ASSERT_NE(nullptr, test2);
    activeWindows_.push_back(test2);
    ASSERT_EQ(WMError::WM_OK, test2->Show());
    usleep(WAIT_SYANC_US);
    if (maxTileNum == 2) {
        ASSERT_TRUE(Utils::RectEqualTo(test1, Utils::doubleTileRects_[0]));
        ASSERT_TRUE(Utils::RectEqualTo(test2, Utils::doubleTileRects_[1]));
    } else {
        ASSERT_TRUE(Utils::RectEqualTo(window, Utils::tripleTileRects_[0]));
        ASSERT_TRUE(Utils::RectEqualTo(test1, Utils::tripleTileRects_[1]));
        ASSERT_TRUE(Utils::RectEqualTo(test2, Utils::tripleTileRects_[2])); // 2 is second rect idx
    }
    WindowManager::GetInstance().SetWindowLayoutMode(WindowLayoutMode::CASCADE);
}

/**
 * @tc.name: LayoutTileNegative01
 * @tc.desc: move window out of the display
 * @tc.type: FUNC
 */
HWTEST_F(WindowLayoutTest, LayoutNegative01, Function | MediumTest | Level3)
{
    WindowManager::GetInstance().SetWindowLayoutMode(WindowLayoutMode::CASCADE);
    Utils::TestWindowInfo info = {
        .name = "mainNegative1",
        .rect = {0, 0, 0, 0},
        .type = WindowType::WINDOW_TYPE_APP_MAIN_WINDOW,
        .mode = WindowMode::WINDOW_MODE_FLOATING,
        .needAvoid = true,
        .parentLimit = false,
        .parentId = INVALID_WINDOW_ID,
    };
    const sptr<Window>& window = Utils::CreateTestWindow(info);
    if (window == nullptr) {
        return;
    }
    activeWindows_.push_back(window);
    Rect expect = Utils::GetDefaultFloatingRect(window, true);
    ASSERT_EQ(WMError::WM_OK, window->Show());
    usleep(WAIT_SYANC_US);
    ASSERT_TRUE(Utils::RectEqualTo(window, expect));
}

/**
 * @tc.name: LayoutNegative02
 * @tc.desc: resize window to negative size
 * @tc.type: FUNC
 */
HWTEST_F(WindowLayoutTest, LayoutNegative02, Function | MediumTest | Level3)
{
    WindowManager::GetInstance().SetWindowLayoutMode(WindowLayoutMode::CASCADE);
    const uint32_t negativeW = 0;
    const uint32_t negativeH = 0;
    Utils::TestWindowInfo info = {
        .name = "mainNegative2",
        .rect = {0, 0, 0, 0},
        .type = WindowType::WINDOW_TYPE_APP_MAIN_WINDOW,
        .mode = WindowMode::WINDOW_MODE_FLOATING,
        .needAvoid = true,
        .parentLimit = false,
        .parentId = INVALID_WINDOW_ID,
    };
    const sptr<Window>& window = Utils::CreateTestWindow(info);
    if (window == nullptr) {
        return;
    }
    activeWindows_.push_back(window);
    Rect expect = Utils::GetDefaultFloatingRect(window, true);
    ASSERT_EQ(WMError::WM_OK, window->Show());
    usleep(WAIT_SYANC_US);
    ASSERT_TRUE(Utils::RectEqualTo(window, expect));
    window->Resize(negativeW, negativeH);
    usleep(WAIT_SYANC_US);
    Rect expect2 = {expect.posX_, expect.posY_, negativeW, negativeH};
    expect2 = Utils::CalcLimitedRect(expect2, virtualPixelRatio_);
    ASSERT_TRUE(Utils::RectEqualTo(window, expect2));
}

/**
 * @tc.name: moveWindowTo01
 * @tc.desc: test moveWindowTo for ALN/PC with windowMode: 102, windowType: 2106
 * @tc.type: FUNC
 */
HWTEST_F(WindowLayoutTest, moveWindowTo01, Function | MediumTest | Level3)
{
    sptr<WindowOption> option = new (std::nothrow) WindowOption();
    ASSERT_NE(option, nullptr);
    option->SetWindowName("moveWindowTo01");
    option->SetWindowType(WindowType::WINDOW_TYPE_FLOAT);
    option->SetWindowMode(WindowMode::WINDOW_MODE_FLOATING);
    
    sptr<WindowSceneSessionImpl> window = new WindowSceneSessionImpl(option);
    ASSERT_NE(window, nullptr);

    window->property_->SetPersistentId(10001);

    Rect rect;
    WMError ret;
    ret = window->Create(abilityContext_, nullptr);
    EXPECT_EQ(WMError::WM_OK, ret);

    ret = window->Show();
    EXPECT_EQ(WMError::WM_OK, ret);

    ret = window->MoveTo(-500, -500);
    usleep(100000);
    EXPECT_EQ(WMError::WM_OK, ret);
    rect = window->property_->GetWindowRect();
    EXPECT_EQ(-500, rect.posX_);
    EXPECT_EQ(-500, rect.posY_);

    ret = window->MoveTo(0, 0);
    usleep(100000);
    EXPECT_EQ(WMError::WM_OK, ret);
    rect = window->property_->GetWindowRect();
    EXPECT_EQ(0, rect.posX_);
    EXPECT_EQ(0, rect.posY_);

    ret = window->MoveTo(500, 500);
    usleep(100000);
    EXPECT_EQ(WMError::WM_OK, ret);
    rect = window->property_->GetWindowRect();
    EXPECT_EQ(500, rect.posX_);
    EXPECT_EQ(500, rect.posY_);

    ret = window->MoveTo(20000, 20000);
    usleep(100000);
    EXPECT_EQ(WMError::WM_OK, ret);
    rect = window->property_->GetWindowRect();
    EXPECT_EQ(20000, rect.posX_);
    EXPECT_EQ(20000, rect.posY_);
}

/**
 * @tc.name: moveWindowTo02
 * @tc.desc: test moveWindowTo for ALN with windowMode: 102, windowType: 1001
 * @tc.type: FUNC
 */
HWTEST_F(WindowLayoutTest, moveWindowTo02, Function | MediumTest | Level3)
{
    sptr<WindowOption> option = new (std::nothrow) WindowOption();
    ASSERT_NE(option, nullptr);
    option->SetWindowName("moveWindowTo02");
    option->SetWindowType(WindowType::WINDOW_TYPE_APP_SUB_WINDOW);
    option->SetWindowMode(WindowMode::WINDOW_MODE_FLOATING);

    sptr<WindowSceneSessionImpl> window = new WindowSceneSessionImpl(option);
    ASSERT_NE(window, nullptr);

    SessionInfo sessionInfo = { "bundleName_moveWindowTo02",
        "moduleName_moveWindowTo02", "abilityName_moveWindowTo02" };
    sptr<SceneSession> sceneSession = new (std::nothrow) SceneSession(sessionInfo, nullptr);
    ASSERT_NE(sceneSession, nullptr);

    Rect rectOld;
    Rect rectNow;
    WMError ret;
    ret = window->Create(abilityContext_, sceneSession);
    EXPECT_EQ(WMError::WM_OK, ret);
    ret = window->Show();
    EXPECT_EQ(WMError::WM_OK, ret);
    window->property_->SetPersistentId(10002);

    rectOld = window->property_->GetWindowRect();
    ret = window->MoveTo(-500, -500);
    usleep(100000);
    EXPECT_EQ(WMError::WM_OK, ret);
    rectNow = window->property_->GetWindowRect();
    EXPECT_EQ(rectOld.posX_, rectNow.posX_);
    EXPECT_EQ(rectOld.posY_, rectNow.posY_);

    rectOld = window->property_->GetWindowRect();
    ret = window->MoveTo(0, 0);
    usleep(100000);
    EXPECT_EQ(WMError::WM_OK, ret);
    rectNow = window->property_->GetWindowRect();
    EXPECT_EQ(rectOld.posX_, rectNow.posX_);
    EXPECT_EQ(rectOld.posY_, rectNow.posY_);

    rectOld = window->property_->GetWindowRect();
    ret = window->MoveTo(500, 500);
    usleep(100000);
    EXPECT_EQ(WMError::WM_OK, ret);
    rectNow = window->property_->GetWindowRect();
    EXPECT_EQ(rectOld.posX_, rectNow.posX_);
    EXPECT_EQ(rectOld.posY_, rectNow.posY_);
}

/**
 * @tc.name: moveWindowTo03
 * @tc.desc: test moveWindowTo for ALN with windowMode: 1, windowType: 1
 * @tc.type: FUNC
 */
HWTEST_F(WindowLayoutTest, moveWindowTo03, Function | MediumTest | Level3)
{
    sptr<WindowOption> option = new (std::nothrow) WindowOption();
    ASSERT_NE(option, nullptr);
    option->SetWindowName("moveWindowTo03");
    option->SetWindowType(WindowType::WINDOW_TYPE_APP_MAIN_WINDOW);
    option->SetWindowMode(WindowMode::WINDOW_MODE_FULLSCREEN);
    
    sptr<WindowSceneSessionImpl> window = new WindowSceneSessionImpl(option);
    ASSERT_NE(window, nullptr);

    window->property_->SetPersistentId(10003);

    SessionInfo sessionInfo = { "bundleName_moveWindowTo03",
        "moduleName_moveWindowTo03", "abilityName_moveWindowTo03" };
    sptr<SceneSession> sceneSession = new (std::nothrow) SceneSession(sessionInfo, nullptr);
    ASSERT_NE(sceneSession, nullptr);

    Rect rectOld;
    Rect rectNow;
    WMError ret;
    ret = window->Create(abilityContext_, sceneSession);
    EXPECT_EQ(WMError::WM_OK, ret);
    ret = window->Show();
    EXPECT_EQ(WMError::WM_OK, ret);

    rectOld = window->property_->GetWindowRect();
    ret = window->MoveTo(-500, -500);
    usleep(100000);
    EXPECT_EQ(WMError::WM_OK, ret);
    rectNow = window->property_->GetWindowRect();
    EXPECT_EQ(rectOld.posX_, rectNow.posX_);
    EXPECT_EQ(rectOld.posY_, rectNow.posY_);

    rectOld = window->property_->GetWindowRect();
    ret = window->MoveTo(0, 0);
    usleep(100000);
    EXPECT_EQ(WMError::WM_OK, ret);
    rectNow = window->property_->GetWindowRect();
    EXPECT_EQ(rectOld.posX_, rectNow.posX_);
    EXPECT_EQ(rectOld.posY_, rectNow.posY_);

    rectOld = window->property_->GetWindowRect();
    ret = window->MoveTo(500, 500);
    usleep(100000);
    EXPECT_EQ(WMError::WM_OK, ret);
    rectNow = window->property_->GetWindowRect();
    EXPECT_EQ(rectOld.posX_, rectNow.posX_);
    EXPECT_EQ(rectOld.posY_, rectNow.posY_);

    rectOld = window->property_->GetWindowRect();
    ret = window->MoveTo(20000, 20000);
    usleep(100000);
    EXPECT_EQ(WMError::WM_OK, ret);
    rectNow = window->property_->GetWindowRect();
    EXPECT_EQ(rectOld.posX_, rectNow.posX_);
    EXPECT_EQ(rectOld.posY_, rectNow.posY_);
}

/**
 * @tc.name: moveWindowTo04
 * @tc.desc: test moveWindowTo for ALN with windowMode: 100, windowType: 1
 * @tc.type: FUNC
 */
HWTEST_F(WindowLayoutTest, moveWindowTo04, Function | MediumTest | Level3)
{
    sptr<WindowOption> option = new (std::nothrow) WindowOption();
    ASSERT_NE(option, nullptr);
    option->SetWindowName("moveWindowTo04");
    option->SetWindowType(WindowType::WINDOW_TYPE_APP_MAIN_WINDOW);
    option->SetWindowMode(WindowMode::WINDOW_MODE_SPLIT_PRIMARY);
    
    sptr<WindowSceneSessionImpl> window = new WindowSceneSessionImpl(option);
    ASSERT_NE(window, nullptr);

    window->property_->SetPersistentId(10004);

    SessionInfo sessionInfo = { "bundleName_moveWindowTo04",
        "moduleName_moveWindowTo04", "abilityName_moveWindowTo04" };
    sptr<SceneSession> sceneSession = new (std::nothrow) SceneSession(sessionInfo, nullptr);
    ASSERT_NE(sceneSession, nullptr);

    Rect rectOld;
    Rect rectNow;
    WMError ret;
    ret = window->Create(abilityContext_, sceneSession);
    EXPECT_EQ(WMError::WM_OK, ret);
    ret = window->Show();
    EXPECT_EQ(WMError::WM_OK, ret);

    rectOld = window->property_->GetWindowRect();
    ret = window->MoveTo(-500, -500);
    usleep(100000);
    EXPECT_EQ(WMError::WM_OK, ret);
    rectNow = window->property_->GetWindowRect();
    EXPECT_EQ(rectOld.posX_, rectNow.posX_);
    EXPECT_EQ(rectOld.posY_, rectNow.posY_);

    rectOld = window->property_->GetWindowRect();
    ret = window->MoveTo(0, 0);
    usleep(100000);
    EXPECT_EQ(WMError::WM_OK, ret);
    rectNow = window->property_->GetWindowRect();
    EXPECT_EQ(rectOld.posX_, rectNow.posX_);
    EXPECT_EQ(rectOld.posY_, rectNow.posY_);

    rectOld = window->property_->GetWindowRect();
    ret = window->MoveTo(500, 500);
    usleep(100000);
    EXPECT_EQ(WMError::WM_OK, ret);
    rectNow = window->property_->GetWindowRect();
    EXPECT_EQ(rectOld.posX_, rectNow.posX_);
    EXPECT_EQ(rectOld.posY_, rectNow.posY_);

    rectOld = window->property_->GetWindowRect();
    ret = window->MoveTo(20000, 20000);
    usleep(100000);
    EXPECT_EQ(WMError::WM_OK, ret);
    rectNow = window->property_->GetWindowRect();
    EXPECT_EQ(rectOld.posX_, rectNow.posX_);
    EXPECT_EQ(rectOld.posY_, rectNow.posY_);
}

/**
 * @tc.name: resize01
 * @tc.desc: test resize for ALN/PC with windowMode: 102, windowType: 2106
 * @tc.type: FUNC
 */
HWTEST_F(WindowLayoutTest, resize01, Function | MediumTest | Level3)
{
    sptr<WindowOption> option = new (std::nothrow) WindowOption();
    ASSERT_NE(option, nullptr);
    option->SetWindowName("resize01");
    option->SetWindowType(WindowType::WINDOW_TYPE_FLOAT);
    option->SetWindowMode(WindowMode::WINDOW_MODE_FLOATING);
    
    sptr<WindowSceneSessionImpl> window = new WindowSceneSessionImpl(option);
    ASSERT_NE(window, nullptr);

    Rect rect;
    WMError ret;
    ret = window->Create(abilityContext_, nullptr);
    EXPECT_EQ(WMError::WM_OK, ret);
    window->property_->SetPersistentId(10008);
    ret = window->Show();
    EXPECT_EQ(WMError::WM_OK, ret);

    WindowLimits windowLimits;
    ret = window->GetWindowLimits(windowLimits);
    EXPECT_EQ(WMError::WM_OK, ret);

    ret = window->Resize(-500, -500);
    EXPECT_EQ(WMError::WM_OK, ret);
    usleep(100000);
    rect = window->property_->GetWindowRect();
    EXPECT_EQ(windowLimits.maxWidth_, rect.width_);
    EXPECT_EQ(windowLimits.maxHeight_, rect.height_);

    ret = window->Resize(500, 500);
    EXPECT_EQ(WMError::WM_OK, ret);
    usleep(100000);
    rect = window->property_->GetWindowRect();
    EXPECT_EQ(500, rect.width_);
    EXPECT_EQ(500, rect.height_);

    ret = window->Resize(20000, 20000);
    EXPECT_EQ(WMError::WM_OK, ret);
    usleep(100000);
    rect = window->property_->GetWindowRect();
    EXPECT_EQ(windowLimits.maxWidth_, rect.width_);
    EXPECT_EQ(windowLimits.maxHeight_, rect.height_);

    ret = window->Resize(0, 0);
    EXPECT_EQ(WMError::WM_ERROR_INVALID_PARAM, ret); // check parameter first
}

/**
 * @tc.name: resize02
 * @tc.desc: test resize for ALN with windowMode: 1, windowType: 1
 * @tc.type: FUNC
 */
HWTEST_F(WindowLayoutTest, resize02, Function | MediumTest | Level3)
{
    sptr<WindowOption> option = new (std::nothrow) WindowOption();
    ASSERT_NE(option, nullptr);
    option->SetWindowName("resize02");
    option->SetWindowType(WindowType::WINDOW_TYPE_APP_MAIN_WINDOW);
    option->SetWindowMode(WindowMode::WINDOW_MODE_FULLSCREEN);
    
    sptr<WindowSceneSessionImpl> window = new WindowSceneSessionImpl(option);
    ASSERT_NE(window, nullptr);

    SessionInfo sessionInfo = { "bundleName_resize02", "moduleName_resize02", "abilityName_resize02" };
    sptr<SceneSession> sceneSession = new (std::nothrow) SceneSession(sessionInfo, nullptr);
    ASSERT_NE(sceneSession, nullptr);

    WMError ret;
    ret = window->Create(abilityContext_, sceneSession);
    EXPECT_EQ(WMError::WM_OK, ret);
    window->property_->SetPersistentId(10009);
    ret = window->Show();
    EXPECT_EQ(WMError::WM_OK, ret);

    ret = window->Resize(500, 500);
    EXPECT_EQ(WMError::WM_ERROR_INVALID_OPERATION, ret);

    ret = window->Resize(20000, 20000);
    EXPECT_EQ(WMError::WM_ERROR_INVALID_OPERATION, ret);

    ret = window->Resize(0, 0);
    EXPECT_EQ(WMError::WM_ERROR_INVALID_PARAM, ret); // check parameter first
}

/**
 * @tc.name: resize03
 * @tc.desc: test resize for PC with windowMode: 1, windowType: 1
 * @tc.type: FUNC
 */
HWTEST_F(WindowLayoutTest, resize03, Function | MediumTest | Level3)
{
    sptr<WindowOption> option = new (std::nothrow) WindowOption();
    ASSERT_NE(option, nullptr);
    option->SetWindowName("resize03");
    option->SetWindowType(WindowType::WINDOW_TYPE_APP_MAIN_WINDOW);
    option->SetWindowMode(WindowMode::WINDOW_MODE_FULLSCREEN);
    
    sptr<WindowSceneSessionImpl> window = new WindowSceneSessionImpl(option);
    ASSERT_NE(window, nullptr);

    SessionInfo sessionInfo = { "bundleName_resize03", "moduleName_resize03", "abilityName_resize03" };
    sptr<SceneSession> sceneSession = new (std::nothrow) SceneSession(sessionInfo, nullptr);
    ASSERT_NE(sceneSession, nullptr);

    WMError ret;
    ret = window->Create(abilityContext_, sceneSession);
    EXPECT_EQ(WMError::WM_OK, ret);
    window->property_->SetPersistentId(100010);
    ret = window->Show();
    EXPECT_EQ(WMError::WM_OK, ret);

    ret = window->Resize(500, 500);
    EXPECT_EQ(WMError::WM_ERROR_INVALID_OPERATION, ret);

    ret = window->Resize(20000, 20000);
    EXPECT_EQ(WMError::WM_ERROR_INVALID_OPERATION, ret);

    ret = window->Resize(0, 0);
    EXPECT_EQ(WMError::WM_ERROR_INVALID_PARAM, ret); // check parameter first
}

/**
 * @tc.name: resize04
 * @tc.desc: test resize for ALN/PC with windowMode: 100, windowType: 1
 * @tc.type: FUNC
 */
HWTEST_F(WindowLayoutTest, resize04, Function | MediumTest | Level3)
{
    sptr<WindowOption> option = new (std::nothrow) WindowOption();
    ASSERT_NE(option, nullptr);
    option->SetWindowName("resize04");
    option->SetWindowType(WindowType::WINDOW_TYPE_APP_MAIN_WINDOW);
    option->SetWindowMode(WindowMode::WINDOW_MODE_SPLIT_PRIMARY);
    
    sptr<WindowSceneSessionImpl> window = new WindowSceneSessionImpl(option);
    ASSERT_NE(window, nullptr);

    SessionInfo sessionInfo = { "bundleName_resize04", "moduleName_resize04", "abilityName_resize04" };
    sptr<SceneSession> sceneSession = new (std::nothrow) SceneSession(sessionInfo, nullptr);
    ASSERT_NE(sceneSession, nullptr);

    WMError ret;
    ret = window->Create(abilityContext_, sceneSession);
    EXPECT_EQ(WMError::WM_OK, ret);
    window->property_->SetPersistentId(100011);
    ret = window->Show();
    EXPECT_EQ(WMError::WM_OK, ret);

    ret = window->Resize(500, 500);
    EXPECT_EQ(WMError::WM_ERROR_INVALID_OPERATION, ret);

    ret = window->Resize(20000, 20000);
    EXPECT_EQ(WMError::WM_ERROR_INVALID_OPERATION, ret);

    ret = window->Resize(0, 0);
    EXPECT_EQ(WMError::WM_ERROR_INVALID_PARAM, ret); // check parameter first
}

/**
 * @tc.name: resize05
 * @tc.desc: test resize for ALN/PC with windowMode: 102, windowType: 2106
 * @tc.type: FUNC
 */
HWTEST_F(WindowLayoutTest, resize05, Function | MediumTest | Level3)
{
    sptr<WindowOption> option = new (std::nothrow) WindowOption();
    ASSERT_NE(option, nullptr);
    option->SetWindowName("resize05");
    option->SetWindowType(WindowType::WINDOW_TYPE_FLOAT);
    option->SetWindowMode(WindowMode::WINDOW_MODE_FLOATING);
    
    sptr<WindowSceneSessionImpl> window = new WindowSceneSessionImpl(option);
    ASSERT_NE(window, nullptr);

    Rect rect;
    WMError ret;
    ret = window->Create(abilityContext_, nullptr);
    EXPECT_EQ(WMError::WM_OK, ret);
    window->property_->SetPersistentId(10012);
    ret = window->Show();
    EXPECT_EQ(WMError::WM_OK, ret);

    WindowLimits windowLimits;
    ret = window->GetWindowLimits(windowLimits);

    ret = window->Resize(windowLimits.maxWidth_ - 100, windowLimits.maxHeight_ - 100);
    EXPECT_EQ(WMError::WM_OK, ret);
    usleep(100000);
    rect = window->property_->GetWindowRect();
    EXPECT_EQ(windowLimits.maxWidth_ - 100, rect.width_);
    EXPECT_EQ(windowLimits.maxHeight_ - 100, rect.height_);

    ret = window->Resize(windowLimits.maxWidth_ + 100, windowLimits.maxHeight_ + 100);
    EXPECT_EQ(WMError::WM_OK, ret);
    usleep(100000);
    rect = window->property_->GetWindowRect();
    EXPECT_EQ(windowLimits.maxWidth_, rect.width_);
    EXPECT_EQ(windowLimits.maxHeight_, rect.height_);
}

/**
 * @tc.name: resize06
 * @tc.desc: test resize for ALN with windowMode: 1, windowType: 1
 * @tc.type: FUNC
 */
HWTEST_F(WindowLayoutTest, resize06, Function | MediumTest | Level3)
{
    sptr<WindowOption> option = new (std::nothrow) WindowOption();
    ASSERT_NE(option, nullptr);
    option->SetWindowName("resize06");
    option->SetWindowType(WindowType::WINDOW_TYPE_APP_MAIN_WINDOW);
    option->SetWindowMode(WindowMode::WINDOW_MODE_FULLSCREEN);
    
    sptr<WindowSceneSessionImpl> window = new WindowSceneSessionImpl(option);
    ASSERT_NE(window, nullptr);

    SessionInfo sessionInfo = { "bundleName_resize06", "moduleName_resize06", "abilityName_resize06" };
    sptr<SceneSession> sceneSession = new (std::nothrow) SceneSession(sessionInfo, nullptr);
    ASSERT_NE(sceneSession, nullptr);

    WMError ret;
    ret = window->Create(abilityContext_, sceneSession);
    EXPECT_EQ(WMError::WM_OK, ret);
    window->property_->SetPersistentId(100013);
    ret = window->Show();
    EXPECT_EQ(WMError::WM_OK, ret);

    WindowLimits windowLimits;
    ret = window->GetWindowLimits(windowLimits);

    ret = window->Resize(windowLimits.maxWidth_ - 100, windowLimits.maxHeight_ - 100);
    EXPECT_EQ(WMError::WM_ERROR_INVALID_OPERATION, ret);

    ret = window->Resize(windowLimits.maxWidth_ + 100, windowLimits.maxHeight_ + 100);
    EXPECT_EQ(WMError::WM_ERROR_INVALID_OPERATION, ret);
}

/**
 * @tc.name: resize07
 * @tc.desc: test resize for PC with windowMode: 1, windowType: 1
 * @tc.type: FUNC
 */
HWTEST_F(WindowLayoutTest, resize07, Function | MediumTest | Level3)
{
    sptr<WindowOption> option = new (std::nothrow) WindowOption();
    ASSERT_NE(option, nullptr);
    option->SetWindowName("resize07");
    option->SetWindowType(WindowType::WINDOW_TYPE_APP_MAIN_WINDOW);
    option->SetWindowMode(WindowMode::WINDOW_MODE_FULLSCREEN);
    
    sptr<WindowSceneSessionImpl> window = new WindowSceneSessionImpl(option);
    ASSERT_NE(window, nullptr);

    SessionInfo sessionInfo = { "bundleName_resize07", "moduleName_resize07", "abilityName_resize07" };
    sptr<SceneSession> sceneSession = new (std::nothrow) SceneSession(sessionInfo, nullptr);
    ASSERT_NE(sceneSession, nullptr);

    WMError ret;
    ret = window->Create(abilityContext_, sceneSession);
    EXPECT_EQ(WMError::WM_OK, ret);
    window->property_->SetPersistentId(100014);
    ret = window->Show();
    EXPECT_EQ(WMError::WM_OK, ret);

    WindowLimits windowLimits;
    ret = window->GetWindowLimits(windowLimits);

    ret = window->Resize(windowLimits.maxWidth_ - 100, windowLimits.maxHeight_ - 100);
    EXPECT_EQ(WMError::WM_ERROR_INVALID_OPERATION, ret);

    ret = window->Resize(windowLimits.maxWidth_ + 100, windowLimits.maxHeight_ + 100);
    EXPECT_EQ(WMError::WM_ERROR_INVALID_OPERATION, ret);
}

/**
 * @tc.name: resize08
 * @tc.desc: test resize for ALN/PC with windowMode: 100, windowType: 1
 * @tc.type: FUNC
 */
HWTEST_F(WindowLayoutTest, resize08, Function | MediumTest | Level3)
{
    sptr<WindowOption> option = new (std::nothrow) WindowOption();
    ASSERT_NE(option, nullptr);
    option->SetWindowName("resize08");
    option->SetWindowType(WindowType::WINDOW_TYPE_APP_MAIN_WINDOW);
    option->SetWindowMode(WindowMode::WINDOW_MODE_SPLIT_PRIMARY);
    
    sptr<WindowSceneSessionImpl> window = new WindowSceneSessionImpl(option);
    ASSERT_NE(window, nullptr);

    SessionInfo sessionInfo = { "bundleName_resize08", "moduleName_resize08", "abilityName_resize08" };
    sptr<SceneSession> sceneSession = new (std::nothrow) SceneSession(sessionInfo, nullptr);
    ASSERT_NE(sceneSession, nullptr);

    WMError ret;
    ret = window->Create(abilityContext_, sceneSession);
    EXPECT_EQ(WMError::WM_OK, ret);
    window->property_->SetPersistentId(100015);
    ret = window->Show();
    EXPECT_EQ(WMError::WM_OK, ret);

    WindowLimits windowLimits;
    ret = window->GetWindowLimits(windowLimits);

    ret = window->Resize(windowLimits.maxWidth_ - 100, windowLimits.maxHeight_ - 100);
    EXPECT_EQ(WMError::WM_ERROR_INVALID_OPERATION, ret);
    ret = window->Resize(windowLimits.maxWidth_ + 100, windowLimits.maxHeight_ + 100);
    EXPECT_EQ(WMError::WM_ERROR_INVALID_OPERATION, ret);
}
}
} // namespace Rosen
} // namespace OHOS
