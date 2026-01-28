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
#include <transaction/rs_transaction.h>
#include <ui/rs_surface_node.h>
#include "display_test_utils.h"
#include "display.h"
#include "display_manager_proxy.h"
#include "rs_adapter.h"
#include "screen.h"
#include "surface_draw.h"
#include "wm_common.h"
#include "wm_common_inner.h"
#include "window.h"
#include "window_option.h"
#include "window_manager_hilog.h"
#include "display_manager_agent_controller.h"

using namespace testing;
using namespace testing::ext;

namespace OHOS::Rosen {
namespace  {
constexpr HiviewDFX::HiLogLabel LABEL = {LOG_CORE, HILOG_DOMAIN_WINDOW, "DisplayManagerTest"};
const int WAIT_FOR_SYNC_US = 1;  // 1s
}

class DisplayChangeEventListener : public DisplayManager::IDisplayListener {
public:
    virtual void OnCreate(DisplayId displayId) {}

    virtual void OnDestroy(DisplayId displayId) {}

    virtual void OnChange(DisplayId displayId) {}
};

class DisplayManagerTest : public testing::Test {
public:
    static void SetUpTestCase();
    static void TearDownTestCase();
    virtual void SetUp() override;
    virtual void TearDown() override;

    sptr<Window> CreateWindow(std::string name, WindowMode mode, Rect rect, uint32_t color = 0xff000000);
    bool DrawWindowColor(const sptr<Window>& window, uint32_t color, int32_t width, int32_t height);
    static inline DisplayId displayId_;
    static inline int32_t displayWidth_;
    static inline int32_t displayHeight_;
};

void DisplayManagerTest::SetUpTestCase()
{
    displayId_ = DisplayManager::GetInstance().GetDefaultDisplayId();
    sptr<Display> display = DisplayManager::GetInstance().GetDefaultDisplay();
    if (display == nullptr) {
        return;
    }
    displayWidth_ = display->GetWidth();
    displayHeight_ = display->GetHeight();
}

void DisplayManagerTest::TearDownTestCase()
{
}

void DisplayManagerTest::SetUp()
{
}

void DisplayManagerTest::TearDown()
{
}

sptr<Window> DisplayManagerTest::CreateWindow(std::string name,
    WindowMode mode, Rect rect, uint32_t color)
{
    sptr<WindowOption> option = new WindowOption();
    option->SetDisplayId(displayId_);
    option->SetWindowType(WindowType::WINDOW_TYPE_APP_MAIN_WINDOW);
    int32_t width = 0;
    int32_t height = 0;
    if (mode != WindowMode::WINDOW_MODE_FULLSCREEN) {
        option->SetWindowRect(rect);
    } else {
        width = displayWidth_;
        height = displayHeight_;
    }
    option->SetWindowMode(mode);
    option->SetWindowName(name);
    sptr<Window> window = Window::Create(option->GetWindowName(), option);
    if (window == nullptr) {
        return nullptr;
    }
    window->AddWindowFlag(WindowFlag::WINDOW_FLAG_SHOW_WHEN_LOCKED);
    window->Show();
    sleep(WAIT_FOR_SYNC_US); // wait for rect updated
    width = window->GetRect().width_;
    height = window->GetRect().height_;
    DrawWindowColor(window, color, width, height); // 0x66000000 color_black
    RSTransactionAdapter::FlushImplicitTransaction(window->GetRSUIContext());
    return window;
}

bool DisplayManagerTest::DrawWindowColor(const sptr<Window>& window, uint32_t color, int32_t width, int32_t height)
{
    auto surfaceNode = window->GetSurfaceNode();
    if (surfaceNode == nullptr) {
        WLOGFE("Failed to GetSurfaceNode!");
        return false;
    }
    SurfaceDraw::DrawColor(surfaceNode, width, height, color);
    surfaceNode->SetAbilityBGAlpha(255); // 255 is alpha
    return true;
}

namespace {
/**
 * @tc.name: HasPrivateWindow
 * @tc.desc: Check whether there is a private window in the current display
 * @tc.type: FUNC
 * @tc.require issueI5HF6V
 */
HWTEST_F(DisplayManagerTest, HasPrivateWindow, TestSize.Level1)
{
    sptr<Window> window = CreateWindow("test", WindowMode::WINDOW_MODE_FULLSCREEN, Rect {0, 0, 0, 0});
    if (window == nullptr) {
        GTEST_SKIP();
    }
    ASSERT_NE(nullptr, window);
    window->SetPrivacyMode(true);
    sleep(WAIT_FOR_SYNC_US);
    bool hasPrivateWindow = false;
    DisplayId id = DisplayManager::GetInstance().GetDefaultDisplayId();
    DisplayManager::GetInstance().HasPrivateWindow(id, hasPrivateWindow);

    window->SetPrivacyMode(false);
    sleep(WAIT_FOR_SYNC_US);
    DisplayManager::GetInstance().HasPrivateWindow(id, hasPrivateWindow);
    window->Destroy();
    ASSERT_TRUE(!hasPrivateWindow);
}

/**
 * @tc.name: HasPrivateWindowCovered
 * @tc.desc: The private window is covered
 * @tc.type: FUNC
 * @tc.require issueI5HF6V
 */
HWTEST_F(DisplayManagerTest, HasPrivateWindowCovered, TestSize.Level1)
{
    auto displayWidth = DisplayManagerTest::displayWidth_;
    auto displayHeight = DisplayManagerTest::displayHeight_;

    sptr<Window> window1 = CreateWindow("test", WindowMode::WINDOW_MODE_FULLSCREEN, Rect {0, 0, 0, 0});
    if (window1 == nullptr) {
        GTEST_SKIP();
    }
    ASSERT_NE(nullptr, window1);
    // 10:rect.posX_, 120:rect.posY_, 650:rect.width, 500:rect.height
    sptr<Window> window2 = CreateWindow("private", WindowMode::WINDOW_MODE_FLOATING,
                                        Rect{0, 0, 300, 300}, 0xffff0000);
    ASSERT_NE(nullptr, window2);
    window2->SetPrivacyMode(true);
    // The window shadows is too large to cover. so, set a special position for cover window easily.
    sleep(WAIT_FOR_SYNC_US);
    window2->MoveTo(displayWidth * 0.53, displayHeight * 0.66);
    sleep(WAIT_FOR_SYNC_US);

    // 10:rect.posX_, 110:rect.posY_, 650:rect.width, 500:rect.height
    sptr<Window> window3 = CreateWindow("covered", WindowMode::WINDOW_MODE_FLOATING,
                                        Rect{0, 0, displayWidth, displayHeight}, 0xff00ff00);
    ASSERT_NE(nullptr, window3);
    sleep(WAIT_FOR_SYNC_US);
    window3->MoveTo(45, 115);
    sleep(WAIT_FOR_SYNC_US);

    bool hasPrivateWindow = false;
    DisplayId id = DisplayManager::GetInstance().GetDefaultDisplayId();
    DisplayManager::GetInstance().HasPrivateWindow(id, hasPrivateWindow);
    window1->Destroy();
    window2->Destroy();
    window3->Destroy();
    if (!hasPrivateWindow) {
        ASSERT_TRUE(!hasPrivateWindow);
    }
}

/**
 * @tc.name: HasPrivateWindowCovered01
 * @tc.desc: The private window is partially covered
 * @tc.type: FUNC
 * @tc.require issueI5HF6V
 */
HWTEST_F(DisplayManagerTest, HasPrivateWindowCovered01, TestSize.Level1)
{
    sptr<Window> window1 = CreateWindow("test", WindowMode::WINDOW_MODE_FULLSCREEN, Rect{0, 0, 0, 0});

    // 10:rect.posX_, 120:rect.posY_, 650:rect.width, 500:rect.height
    if (window1 == nullptr)
    {
        GTEST_SKIP();
    }
    ASSERT_NE(nullptr, window1);
    sptr<Window> window2 = CreateWindow("private", WindowMode::WINDOW_MODE_FLOATING,
                                        Rect{10, 120, 650, 500}, 0xffff0000);
    ASSERT_NE(nullptr, window2);
    window2->SetPrivacyMode(true);
    // 5:rect.posX_, 110:rect.posY_, 650:rect.width, 500:rect.height
    sptr<Window> window3 = CreateWindow("covered", WindowMode::WINDOW_MODE_FLOATING,
                                        Rect{5, 110, 650, 500}, 0xff00ff00);
    ASSERT_NE(nullptr, window3);

    sleep(WAIT_FOR_SYNC_US);
    bool hasPrivateWindow = false;
    DisplayId id = DisplayManager::GetInstance().GetDefaultDisplayId();
    DisplayManager::GetInstance().HasPrivateWindow(id, hasPrivateWindow);
    window1->Destroy();
    window2->Destroy();
    window3->Destroy();
    if (hasPrivateWindow)
    {
        ASSERT_TRUE(hasPrivateWindow);
    }
}

/**
 * @tc.name: HasPrivateWindowCovered02
 * @tc.desc: The private window is covered
 * @tc.type: FUNC
 * @tc.require issueI5HF6V
 */
HWTEST_F(DisplayManagerTest, HasPrivateWindowCovered02, TestSize.Level1)
{
    auto displayWidth = DisplayManagerTest::displayWidth_;
    auto displayHeight = DisplayManagerTest::displayHeight_;

    sptr<Window> window1 = CreateWindow("test", WindowMode::WINDOW_MODE_FULLSCREEN, Rect {0, 0, 0, 0});
    if (window1 == nullptr) {
        GTEST_SKIP();
    }
    ASSERT_NE(nullptr, window1);
    // 10:rect.posX_, 120:rect.posY_, 650:rect.width, 500:rect.height
    sptr<Window> window2 = CreateWindow("private", WindowMode::WINDOW_MODE_FLOATING,
                                        Rect {0, 0, 300, 300}, 0xffff0000);
    ASSERT_NE(nullptr, window2);
    window2->SetPrivacyMode(true);
    // The window shadows is too large to cover. so, set a special position for cover window easily.
    sleep(WAIT_FOR_SYNC_US);
    window2->MoveTo(displayWidth * 0.53, displayHeight * 0.66);
    sleep(WAIT_FOR_SYNC_US);

    // 5:rect.posX_, 110:rect.posY_, 655:rect.width, 500:rect.height
    sptr<Window> window3 = CreateWindow("covered1", WindowMode::WINDOW_MODE_FLOATING,
                                        Rect { 0, 0, displayWidth, displayHeight / 2}, 0xff00ff00);
    ASSERT_NE(nullptr, window3);
    sleep(WAIT_FOR_SYNC_US);
    window3->MoveTo(45, 115);
    sleep(WAIT_FOR_SYNC_US);

    // 5:rect.posX_, 300:rect.posY_, 655:rect.width, 500:rect.height
    sptr<Window> window4 = CreateWindow("covered2", WindowMode::WINDOW_MODE_FLOATING,
                                        Rect { 0, 0, displayWidth, displayHeight / 2 + 200 }, 0xff00ff00);
    ASSERT_NE(nullptr, window4);
    window4->MoveTo(45, displayHeight / 2 - 95);

    sleep(WAIT_FOR_SYNC_US);
    bool hasPrivateWindow = false;
    DisplayId id = DisplayManager::GetInstance().GetDefaultDisplayId();
    DisplayManager::GetInstance().HasPrivateWindow(id, hasPrivateWindow);
    window1->Destroy();
    window2->Destroy();
    window3->Destroy();
    window4->Destroy();
    if (!hasPrivateWindow) {
        ASSERT_TRUE(!hasPrivateWindow);
    }
}

/**
 * @tc.name: HasPrivateWindowCovered03
 * @tc.desc: The private window is partially covered
 * @tc.type: FUNC
 * @tc.require issueI5HF6V
 */
HWTEST_F(DisplayManagerTest, HasPrivateWindowCovered03, TestSize.Level1)
{
    sptr<Window> window1 = CreateWindow("test", WindowMode::WINDOW_MODE_FULLSCREEN, Rect {0, 0, 0, 0});
   
    if (window1 == nullptr) {
        GTEST_SKIP();
    }
    ASSERT_NE(nullptr, window1);
    // 10:rect.posX_, 120:rect.pos_Y, rect.width_:650, rect.height_:700
    sptr<Window> window2 = CreateWindow("private", WindowMode::WINDOW_MODE_FLOATING,
                                        Rect{10, 120, 650, 700}, 0xffff0000);
    ASSERT_NE(nullptr, window2);
    window2->SetPrivacyMode(true);
    // 5:rect.posX_, 110:rect.pos_Y, rect.width_:655, rect.height_:500
    sptr<Window> window3 = CreateWindow("covered1", WindowMode::WINDOW_MODE_FLOATING,
                                        Rect{5, 110, 655, 500}, 0xff00ff00);
    ASSERT_NE(nullptr, window3);
    // 5:rect.posX_, 700:rect.pos_Y, rect.width_:655, rect.height_:500
    sptr<Window> window4 = CreateWindow("covered2", WindowMode::WINDOW_MODE_FLOATING,
                                        Rect{5, 700, 655, 500}, 0xff00ff00);
    ASSERT_NE(nullptr, window4);

    sleep(WAIT_FOR_SYNC_US);
    bool hasPrivateWindow = false;
    DisplayId id = DisplayManager::GetInstance().GetDefaultDisplayId();
    DisplayManager::GetInstance().HasPrivateWindow(id, hasPrivateWindow);
    window1->Destroy();
    window2->Destroy();
    window3->Destroy();
    window4->Destroy();
    if (hasPrivateWindow) {
        ASSERT_TRUE(hasPrivateWindow);
    }
}

/**
 * @tc.name: HasPrivateWindowSkipSnapShot
 * @tc.desc: set snap shot skip
 * @tc.type: FUNC
 * @tc.require issueI5HF6V
 */
HWTEST_F(DisplayManagerTest, HasPrivateWindowSkipSnapShot, TestSize.Level1)
{
    sptr<Window> window1 = CreateWindow("test", WindowMode::WINDOW_MODE_FULLSCREEN, Rect {0, 0, 0, 0});
    if (window1 == nullptr) {
        GTEST_SKIP();
    }
    ASSERT_NE(nullptr, window1);
    // 10:rect.posX_, 120:rect.posY_, 650:rect.width, 500:rect.height
    sptr<Window> window2 = CreateWindow("private", WindowMode::WINDOW_MODE_FLOATING,
        Rect {10, 120, 650, 500}, 0xffff0000);
    ASSERT_NE(nullptr, window2);
    window2->SetSnapshotSkip(true);
    sleep(WAIT_FOR_SYNC_US);
    bool hasPrivateWindow = false;
    DisplayId id = DisplayManager::GetInstance().GetDefaultDisplayId();
    DisplayManager::GetInstance().HasPrivateWindow(id, hasPrivateWindow);
    window1->Destroy();
    window2->Destroy();
    if (hasPrivateWindow) {
        ASSERT_TRUE(hasPrivateWindow);
    }
}

/**
 * @tc.name: AddSurfaceNodeToDisplay | RemoveSurfaceNodeFromDisplay
 * @tc.desc: add/remove surfaceNode to/from display
 * @tc.type: FUNC
 */
HWTEST_F(DisplayManagerTest, AddAndRemoveSurfaceNode, TestSize.Level1)
{
    RSSurfaceNodeConfig config;
    config.SurfaceNodeName = "TestSurfaceNode";
    auto surfaceNode = RSSurfaceNode::Create(config);
    DisplayId id = DisplayManager::GetInstance().GetDefaultDisplayId();
    ASSERT_EQ(DMError::DM_OK, DisplayManager::GetInstance().AddSurfaceNodeToDisplay(id, surfaceNode));
    sleep(2);
    ASSERT_EQ(DMError::DM_OK, DisplayManager::GetInstance().RemoveSurfaceNodeFromDisplay(id, surfaceNode));
}

/**
 * @tc.name: AddSurfaceNodeToDisplay | RemoveSurfaceNodeFromDisplay
 * @tc.desc: add/remove surfaceNode to/from display
 * @tc.type: FUNC
 */
HWTEST_F(DisplayManagerTest, SetVirtualScreenSecurityExemption, TestSize.Level1)
{
    ScreenId id = 0;
    uint32_t pid = 0;
    std::vector<uint64_t> windowList;
    auto ret = DisplayManager::GetInstance().SetVirtualScreenSecurityExemption(id, pid, windowList);
    ASSERT_NE(DMError::DM_OK, ret); // not virtual screen for id 0
    sleep(2);
}

}
} // namespace OHOS::Rosen
