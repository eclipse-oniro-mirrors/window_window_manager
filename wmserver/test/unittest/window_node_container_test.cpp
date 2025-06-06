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

#include "display_manager.h"
#include "display_manager_config.h"
#include "window_node_container.h"
#include "future.h"
#include "window_node.h"
#include "wm_common.h"
#include "window_transition_info.h"
#include "starting_window.h"
#include "minimize_app.h"

using namespace testing;
using namespace testing::ext;

namespace OHOS {
namespace Rosen {
namespace {
constexpr HiviewDFX::HiLogLabel LABEL = {LOG_CORE, HILOG_DOMAIN_WINDOW, "WindowNodeContainerTest"};
}

class WindowNodeContainerTest : public testing::Test {
public:
    static void SetUpTestCase();
    static void TearDownTestCase();
    virtual void SetUp() override;
    virtual void TearDown() override;
    static sptr<Display> defaultDisplay_;
    static sptr<WindowNodeContainer> container_;
    static Rect windowRect_;
};

sptr<Display> WindowNodeContainerTest::defaultDisplay_ = nullptr;
sptr<WindowNodeContainer> WindowNodeContainerTest::container_ = nullptr;
Rect WindowNodeContainerTest::windowRect_;

void WindowNodeContainerTest::SetUpTestCase()
{
    defaultDisplay_ = DisplayManager::GetInstance().GetDefaultDisplay();
    ASSERT_TRUE((defaultDisplay_ != nullptr));
    WLOGI("GetDefaultDisplay: id %{public}" PRIu64", w %{public}d, h %{public}d, fps %{public}u",
        defaultDisplay_->GetId(), defaultDisplay_->GetWidth(), defaultDisplay_->GetHeight(),
        defaultDisplay_->GetRefreshRate());

    container_ = new WindowNodeContainer(defaultDisplay_->GetDisplayInfo(), defaultDisplay_->GetScreenId());
    windowRect_ = {0, 0, 100, 200};
}

void WindowNodeContainerTest::TearDownTestCase()
{
    container_ = nullptr;
}

void WindowNodeContainerTest::SetUp()
{
}

void WindowNodeContainerTest::TearDown()
{
}

sptr<WindowProperty> CreateWindowProperty(uint32_t windowId, const std::string& windowName,
    WindowType type, WindowMode mode, const Rect& screenRect)
{
    sptr<WindowProperty> property = new WindowProperty();
    property->SetWindowId(windowId);
    property->SetWindowName(windowName);
    property->SetWindowType(type);
    property->SetWindowMode(mode);
    property->SetWindowRect(screenRect);
    return property;
}

RSSurfaceNode::SharedPtr CreateSFNode(const std::string& nodeName)
{
    struct RSSurfaceNodeConfig rsSurfaceNodeConfig;
    rsSurfaceNodeConfig.SurfaceNodeName = "name1";
    RSSurfaceNodeType rsSurfaceNodeType = RSSurfaceNodeType::DEFAULT;

    return RSSurfaceNode::Create(rsSurfaceNodeConfig, rsSurfaceNodeType);
}

namespace {
/**
 * @tc.name: AddWindowNodeOnWindowTree01
 * @tc.desc: add system sub window to system window
 * @tc.type: FUNC
 */
HWTEST_F(WindowNodeContainerTest, AddWindowNodeOnWindowTree01, TestSize.Level1)
{
    sptr<WindowProperty> parentProperty = CreateWindowProperty(110u, "test1",
        WindowType::WINDOW_TYPE_APP_LAUNCHING, WindowMode::WINDOW_MODE_FLOATING, windowRect_);
    sptr<WindowNode> parentNode = new WindowNode(parentProperty, nullptr, nullptr);
    ASSERT_NE(nullptr, parentNode);
    parentNode->SetWindowProperty(parentProperty);
    sptr<WindowProperty> subProperty = CreateWindowProperty(111u, "test2",
        WindowType::WINDOW_TYPE_SYSTEM_SUB_WINDOW, WindowMode::WINDOW_MODE_FLOATING, windowRect_);
    sptr<WindowNode> subNode = new WindowNode(subProperty, nullptr, nullptr);
    ASSERT_NE(nullptr, subNode);
    subNode->SetWindowProperty(subProperty);
    ASSERT_EQ(WMError::WM_OK, container_->AddWindowNodeOnWindowTree(subNode, parentNode));
}

/**
 * @tc.name: AddWindowNodeOnWindowTree02
 * @tc.desc: add system sub window to system sub window
 * @tc.type: FUNC
 */
HWTEST_F(WindowNodeContainerTest, AddWindowNodeOnWindowTree02, TestSize.Level1)
{
    sptr<WindowProperty> parentProperty = CreateWindowProperty(110u, "test1",
        WindowType::WINDOW_TYPE_SYSTEM_SUB_WINDOW, WindowMode::WINDOW_MODE_FLOATING, windowRect_);
    sptr<WindowNode> parentNode = new WindowNode(parentProperty, nullptr, nullptr);
    parentNode->SetWindowProperty(parentProperty);
    sptr<WindowProperty> subProperty = CreateWindowProperty(111u, "test2",
        WindowType::WINDOW_TYPE_SYSTEM_SUB_WINDOW, WindowMode::WINDOW_MODE_FLOATING, windowRect_);
    sptr<WindowNode> subNode = new WindowNode(subProperty, nullptr, nullptr);
    subNode->SetWindowProperty(subProperty);
    ASSERT_EQ(WMError::WM_ERROR_INVALID_PARAM, container_->AddWindowNodeOnWindowTree(subNode, parentNode));
}

/**
 * @tc.name: AddWindowNodeOnWindowTree03
 * @tc.desc: add system sub window without parent
 * @tc.type: FUNC
 */
HWTEST_F(WindowNodeContainerTest, AddWindowNodeOnWindowTree03, TestSize.Level1)
{
    sptr<WindowProperty> subProperty = CreateWindowProperty(110u, "test1",
        WindowType::WINDOW_TYPE_SYSTEM_SUB_WINDOW, WindowMode::WINDOW_MODE_FLOATING, windowRect_);
    sptr<WindowNode> subNode = new WindowNode(subProperty, nullptr, nullptr);
    subNode->SetWindowProperty(subProperty);
    ASSERT_EQ(WMError::WM_ERROR_NULLPTR, container_->AddWindowNodeOnWindowTree(subNode, nullptr));
}

/**
 * @tc.name: MinimizeAppNodeExceptOptions
 * @tc.desc: minimize app node
 * @tc.type: FUNC
 */
HWTEST_F(WindowNodeContainerTest, MinimizeAppNodeExceptOptions, TestSize.Level1)
{
    std::vector<uint32_t> exceptionalIds;
    std::vector<WindowMode> exceptionalModes;
    ASSERT_EQ(WMError::WM_OK, container_->MinimizeAppNodeExceptOptions(MinimizeReason::OTHER_WINDOW,
        exceptionalIds, exceptionalModes));

    sptr<WindowProperty> property1 = CreateWindowProperty(110u, "test1",
        WindowType::WINDOW_TYPE_APP_MAIN_WINDOW, WindowMode::WINDOW_MODE_FULLSCREEN, windowRect_);
    sptr<WindowNode> node1 = new WindowNode(property1, nullptr, nullptr);
    node1->SetWindowProperty(property1);
    sptr<WindowProperty> property2 = CreateWindowProperty(111u, "test2",
        WindowType::WINDOW_TYPE_APP_MAIN_WINDOW, WindowMode::WINDOW_MODE_FLOATING, windowRect_);
    sptr<WindowNode> node2 = new WindowNode(property2, nullptr, nullptr);
    node2->SetWindowProperty(property2);
    ASSERT_EQ(WMError::WM_OK, container_->AddWindowNodeOnWindowTree(node1, nullptr));
    ASSERT_EQ(WMError::WM_OK, container_->AddWindowNodeOnWindowTree(node2, nullptr));
    ASSERT_EQ(WMError::WM_OK, container_->MinimizeAppNodeExceptOptions(MinimizeReason::OTHER_WINDOW,
        exceptionalIds, exceptionalModes));
}

/**
 * @tc.name: DropShowWhenLockedWindowIfNeeded
 * @tc.desc: drop show when locken window
 * @tc.type: FUNC
 */
HWTEST_F(WindowNodeContainerTest, DropShowWhenLockedWindowIfNeeded, TestSize.Level1)
{
    sptr<WindowProperty> property = CreateWindowProperty(110u, "test1",
        WindowType::WINDOW_TYPE_KEYGUARD, WindowMode::WINDOW_MODE_FULLSCREEN, windowRect_);
    sptr<WindowNode> node = new WindowNode(property, nullptr, nullptr);
    ASSERT_NE(nullptr, node);
    node->SetWindowProperty(property);
    container_->DropShowWhenLockedWindowIfNeeded(node);
}

/**
 * @tc.name: GetModeChangeHotZones
 * @tc.desc: get mode change hot zones
 * @tc.type: FUNC
 */
HWTEST_F(WindowNodeContainerTest, GetModeChangeHotZones, TestSize.Level1)
{
    ModeChangeHotZonesConfig hotZonesConfig { true, 10, 20, 30 };
    ModeChangeHotZones hotZones;
    container_->GetModeChangeHotZones(0, hotZones, hotZonesConfig);
    ASSERT_EQ(hotZones.fullscreen_.height_, 10);
    ASSERT_EQ(hotZones.primary_.width_, 20);
    ASSERT_EQ(hotZones.secondary_.width_, 30);
}

/**
 * @tc.name: UpdateCameraFloatWindowStatus
 * @tc.desc: update camera float window status
 * @tc.type: FUNC
 */
HWTEST_F(WindowNodeContainerTest, UpdateCameraFloatWindowStatus, TestSize.Level1)
{
    sptr<WindowProperty> property = CreateWindowProperty(110u, "test1",
        WindowType::WINDOW_TYPE_FLOAT_CAMERA, WindowMode::WINDOW_MODE_FULLSCREEN, windowRect_);
    sptr<WindowNode> node = new WindowNode(property, nullptr, nullptr);
    ASSERT_NE(nullptr, node);
    node->SetWindowProperty(property);
    container_->UpdateCameraFloatWindowStatus(node, true);
}

/**
 * @tc.name: UpdateWindowNode
 * @tc.desc: preprocess node and update RSTree
 * @tc.type: FUNC
 */
HWTEST_F(WindowNodeContainerTest, UpdateWindowNode, TestSize.Level1)
{
    sptr<WindowProperty> property = CreateWindowProperty(110u, "test1",
        WindowType::SYSTEM_WINDOW_BASE, WindowMode::WINDOW_MODE_FULLSCREEN, windowRect_);
    sptr<WindowNode> node = new WindowNode(property, nullptr, nullptr);
    node->SetWindowProperty(property);
    ASSERT_EQ(WMError::WM_OK, container_->UpdateWindowNode(node, WindowUpdateReason::UPDATE_ALL));
    ASSERT_EQ(WMError::WM_OK, container_->UpdateWindowNode(node, WindowUpdateReason::UPDATE_MODE));
}

/**
 * @tc.name: ShowStartingWindow
 * @tc.desc: show starting window
 * @tc.type: FUNC
 */
HWTEST_F(WindowNodeContainerTest, ShowStartingWindow, TestSize.Level1)
{
    sptr<WindowNodeContainer> container = sptr<WindowNodeContainer>::MakeSptr(defaultDisplay_->GetDisplayInfo(),
        defaultDisplay_->GetScreenId());
    sptr<WindowTransitionInfo> transitionInfo = sptr<WindowTransitionInfo>::MakeSptr();
    sptr<WindowNode> node = StartingWindow::CreateWindowNode(transitionInfo, 101); // 101 is windowId
    node->SetWindowRect({0, 0, 100, 100});
    node->currentVisibility_ = true;
    ASSERT_EQ(WMError::WM_OK, container->ShowStartingWindow(node));
}

/**
 * @tc.name: ShowStartingWindow02
 * @tc.desc: show starting window
 * @tc.type: FUNC
 */
HWTEST_F(WindowNodeContainerTest, ShowStartingWindow02, TestSize.Level1)
{
    sptr<WindowNodeContainer> container = sptr<WindowNodeContainer>::MakeSptr(defaultDisplay_->GetDisplayInfo(),
        defaultDisplay_->GetScreenId());
    sptr<WindowTransitionInfo> transitionInfo = sptr<WindowTransitionInfo>::MakeSptr();
    sptr<WindowNode> node = StartingWindow::CreateWindowNode(transitionInfo, 101); // 101 is windowId
    node->SetWindowRect({0, 0, 100, 100});
    node->currentVisibility_ = false;
    ASSERT_EQ(WMError::WM_OK, container->AddWindowNodeOnWindowTree(node, nullptr));
    ASSERT_EQ(WMError::WM_ERROR_INVALID_OPERATION, container->ShowStartingWindow(node));
}

/**
 * @tc.name: ShowStartingWindow03
 * @tc.desc: show starting window
 * @tc.type: FUNC
 */
HWTEST_F(WindowNodeContainerTest, ShowStartingWindow03, TestSize.Level1)
{
    sptr<WindowNodeContainer> container = sptr<WindowNodeContainer>::MakeSptr(defaultDisplay_->GetDisplayInfo(),
        defaultDisplay_->GetScreenId());
    sptr<WindowTransitionInfo> transitionInfo = sptr<WindowTransitionInfo>::MakeSptr();
    WindowType invalidType = static_cast<WindowType>(0);
    sptr<WindowProperty> invalidProperty = CreateWindowProperty(110u, "test1", invalidType,
        WindowMode::WINDOW_MODE_FULLSCREEN, windowRect_);
    sptr<WindowNode> invalidNode = sptr<WindowNode>::MakeSptr(invalidProperty, nullptr, nullptr);
    invalidNode->SetWindowProperty(invalidProperty);
    ASSERT_EQ(WMError::WM_ERROR_NULLPTR, container->AddWindowNodeOnWindowTree(invalidNode, nullptr));
    ASSERT_EQ(WMError::WM_ERROR_NULLPTR, container->ShowStartingWindow(invalidNode));
}

/**
 * @tc.name: IsForbidDockSliceMove
 * @tc.desc: forbid dock slice move
 * @tc.type: FUNC
 */
HWTEST_F(WindowNodeContainerTest, IsForbidDockSliceMove, TestSize.Level1)
{
    sptr<WindowNodeContainer> container = new WindowNodeContainer(defaultDisplay_->GetDisplayInfo(),
        defaultDisplay_->GetScreenId());
    ASSERT_NE(nullptr, container->displayGroupController_);
    ASSERT_NE(nullptr, container->displayGroupController_->GetWindowPairByDisplayId(defaultDisplay_->GetId()));
    ASSERT_TRUE(!container->IsForbidDockSliceMove(defaultDisplay_->GetId()));
}

/**
 * @tc.name: GetWindowCountByType01
 * @tc.desc: get window count
 * @tc.type: FUNC
 */
HWTEST_F(WindowNodeContainerTest, GetWindowCountByType01, TestSize.Level1)
{
    sptr<WindowNodeContainer> container = sptr<WindowNodeContainer>::MakeSptr(defaultDisplay_->GetDisplayInfo(),
        defaultDisplay_->GetScreenId());
    ASSERT_EQ(0, container->GetWindowCountByType(WindowType::BELOW_APP_SYSTEM_WINDOW_BASE));
    sptr<WindowProperty> property = CreateWindowProperty(110u, "test1", WindowType::BELOW_APP_SYSTEM_WINDOW_BASE,
        WindowMode::WINDOW_MODE_FULLSCREEN, windowRect_);
    ASSERT_NE(property, nullptr);
    sptr<WindowNode> node = sptr<WindowNode>::MakeSptr(property, nullptr, nullptr);
    node->SetWindowProperty(property);
    container->belowAppWindowNode_->children_.insert(container->belowAppWindowNode_->children_.end(), node);
    ASSERT_EQ(0, container->GetWindowCountByType(WindowType::WINDOW_TYPE_KEYGUARD));
    ASSERT_EQ(1, container->GetWindowCountByType(WindowType::BELOW_APP_SYSTEM_WINDOW_BASE));
    node->startingWindowShown_ = true;
    ASSERT_EQ(1, container->GetWindowCountByType(WindowType::BELOW_APP_SYSTEM_WINDOW_BASE));
}

/**
 * @tc.name: GetWindowCountByType02
 * @tc.desc: get window count
 * @tc.type: FUNC
 */
HWTEST_F(WindowNodeContainerTest, GetWindowCountByType02, TestSize.Level1)
{
    sptr<WindowNodeContainer> container = sptr<WindowNodeContainer>::MakeSptr(defaultDisplay_->GetDisplayInfo(),
        defaultDisplay_->GetScreenId());
    ASSERT_EQ(0, container->GetWindowCountByType(WindowType::BELOW_APP_SYSTEM_WINDOW_BASE));
    sptr<WindowProperty> property = CreateWindowProperty(111u, "test2", WindowType::WINDOW_TYPE_APP_MAIN_WINDOW,
        WindowMode::WINDOW_MODE_FULLSCREEN, windowRect_);
    ASSERT_NE(property, nullptr);
    sptr<WindowNode> node = sptr<WindowNode>::MakeSptr(property, nullptr, nullptr);
    node->SetWindowProperty(property);
    container->appWindowNode_->children_.insert(container->appWindowNode_->children_.end(), node);
    ASSERT_EQ(0, container->GetWindowCountByType(WindowType::WINDOW_TYPE_KEYGUARD));
    ASSERT_EQ(1, container->GetWindowCountByType(WindowType::WINDOW_TYPE_APP_MAIN_WINDOW));
    node->startingWindowShown_ = true;
    ASSERT_EQ(1, container->GetWindowCountByType(WindowType::WINDOW_TYPE_APP_MAIN_WINDOW));
}

/**
 * @tc.name: GetWindowCountByType03
 * @tc.desc: get window count
 * @tc.type: FUNC
 */
HWTEST_F(WindowNodeContainerTest, GetWindowCountByType03, TestSize.Level1)
{
    sptr<WindowNodeContainer> container = sptr<WindowNodeContainer>::MakeSptr(defaultDisplay_->GetDisplayInfo(),
        defaultDisplay_->GetScreenId());
    ASSERT_EQ(0, container->GetWindowCountByType(WindowType::BELOW_APP_SYSTEM_WINDOW_BASE));
    sptr<WindowProperty> property = CreateWindowProperty(112u, "test3", WindowType::ABOVE_APP_SYSTEM_WINDOW_BASE,
        WindowMode::WINDOW_MODE_FULLSCREEN, windowRect_);
    sptr<WindowNode> node = new WindowNode(property, nullptr, nullptr);
    node->SetWindowProperty(property);
    container->aboveAppWindowNode_->children_.insert(container->aboveAppWindowNode_->children_.end(), node);
    ASSERT_EQ(0, container->GetWindowCountByType(WindowType::WINDOW_TYPE_KEYGUARD));
    ASSERT_EQ(1, container->GetWindowCountByType(WindowType::ABOVE_APP_SYSTEM_WINDOW_BASE));
    node->startingWindowShown_ = true;
    ASSERT_EQ(1, container->GetWindowCountByType(WindowType::ABOVE_APP_SYSTEM_WINDOW_BASE));
}

/**
 * @tc.name: IsTileRectSatisfiedWithSizeLimits
 * @tc.desc: judge tile rect satisfied with size limits
 * @tc.type: FUNC
 */
HWTEST_F(WindowNodeContainerTest, IsTileRectSatisfiedWithSizeLimits, TestSize.Level1)
{
    sptr<WindowNodeContainer> container = new WindowNodeContainer(defaultDisplay_->GetDisplayInfo(),
        defaultDisplay_->GetScreenId());
    sptr<WindowProperty> property = CreateWindowProperty(111u, "test1", WindowType::ABOVE_APP_SYSTEM_WINDOW_BASE,
        WindowMode::WINDOW_MODE_FULLSCREEN, windowRect_);
    sptr<WindowNode> node = new WindowNode(property, nullptr, nullptr);
    node->SetWindowProperty(property);
    node->SetDisplayId(defaultDisplay_->GetId());
    ASSERT_EQ(defaultDisplay_->GetId(), node->GetDisplayId());
    ASSERT_EQ(WMError::WM_OK, container->IsTileRectSatisfiedWithSizeLimits(node));
    ASSERT_EQ(WMError::WM_OK, container->SwitchLayoutPolicy(WindowLayoutMode::TILE, node->GetDisplayId()));
    ASSERT_EQ(WMError::WM_OK, container->IsTileRectSatisfiedWithSizeLimits(node));
}

/**
 * @tc.name: AddWindowNode01
 * @tc.desc: add main window
 * @tc.type: FUNC
 */
HWTEST_F(WindowNodeContainerTest, AddWindowNode01, TestSize.Level1)
{
    sptr<WindowNodeContainer> container = new WindowNodeContainer(defaultDisplay_->GetDisplayInfo(),
        defaultDisplay_->GetScreenId());
    sptr<WindowProperty> property = CreateWindowProperty(111u, "test1", WindowType::WINDOW_TYPE_APP_MAIN_WINDOW,
        WindowMode::WINDOW_MODE_FULLSCREEN, windowRect_);
    sptr<WindowNode> node = new WindowNode(property, nullptr, nullptr);
    node->SetWindowProperty(property);
    node->GetWindowProperty()->SetPrivacyMode(true);
    sptr<WindowNode> parentNode = nullptr;
    ASSERT_EQ(WMError::WM_OK, container->AddWindowNode(node, parentNode));
    node->startingWindowShown_ = true;
    node->GetWindowProperty()->SetPrivacyMode(false);
    ASSERT_EQ(WMError::WM_OK, container->AddWindowNode(node, parentNode));
}

/**
 * @tc.name: AddWindowNode02
 * @tc.desc: add sub window
 * @tc.type: FUNC
 */
HWTEST_F(WindowNodeContainerTest, AddWindowNode02, TestSize.Level1)
{
    sptr<WindowNodeContainer> container = new WindowNodeContainer(defaultDisplay_->GetDisplayInfo(),
        defaultDisplay_->GetScreenId());
    sptr<WindowProperty> parentProperty = CreateWindowProperty(110u, "test1",
        WindowType::WINDOW_TYPE_APP_MAIN_WINDOW, WindowMode::WINDOW_MODE_FULLSCREEN, windowRect_);
    sptr<WindowNode> parentNode = new WindowNode(parentProperty, nullptr, nullptr);
    parentNode->SetWindowProperty(parentProperty);
    sptr<WindowProperty> subProperty = CreateWindowProperty(111u, "test2",
        WindowType::WINDOW_TYPE_APP_SUB_WINDOW, WindowMode::WINDOW_MODE_FULLSCREEN, windowRect_);
    sptr<WindowNode> subNode = new WindowNode(subProperty, nullptr, nullptr);
    subNode->SetWindowProperty(subProperty);
    sptr<WindowNode> rootNode = nullptr;
    ASSERT_EQ(WMError::WM_OK, container->AddWindowNode(parentNode, rootNode));
    ASSERT_EQ(WMError::WM_OK, container->AddWindowNodeOnWindowTree(subNode, parentNode));
    ASSERT_EQ(WMError::WM_OK, container->AddWindowNode(subNode, parentNode));
}

/**
 * @tc.name: AddWindowNode03
 * @tc.desc: add system window
 * @tc.type: FUNC
 */
HWTEST_F(WindowNodeContainerTest, AddWindowNode03, TestSize.Level1)
{
    sptr<WindowNodeContainer> container = new WindowNodeContainer(defaultDisplay_->GetDisplayInfo(),
        defaultDisplay_->GetScreenId());
    sptr<WindowProperty> property = CreateWindowProperty(111u, "test1", WindowType::ABOVE_APP_SYSTEM_WINDOW_BASE,
        WindowMode::WINDOW_MODE_FULLSCREEN, windowRect_);
    sptr<WindowNode> node = new WindowNode(property, nullptr, nullptr);
    node->SetWindowProperty(property);
    sptr<WindowNode> parentNode = nullptr;
    ASSERT_EQ(WMError::WM_OK, container->AddWindowNode(node, parentNode));
}

/**
 * @tc.name: RemoveWindowNodeFromWindowTree
 * @tc.desc: remove sub window from window tree
 * @tc.type: FUNC
 */
HWTEST_F(WindowNodeContainerTest, RemoveWindowNodeFromWindowTree, TestSize.Level1)
{
    sptr<WindowNodeContainer> container = new WindowNodeContainer(defaultDisplay_->GetDisplayInfo(),
        defaultDisplay_->GetScreenId());
    sptr<WindowProperty> parentProperty = CreateWindowProperty(110u, "test1",
        WindowType::WINDOW_TYPE_APP_MAIN_WINDOW, WindowMode::WINDOW_MODE_FULLSCREEN, windowRect_);
    sptr<WindowNode> parentNode = new WindowNode(parentProperty, nullptr, nullptr);
    parentNode->SetWindowProperty(parentProperty);
    sptr<WindowProperty> subProperty = CreateWindowProperty(111u, "test2",
        WindowType::WINDOW_TYPE_APP_SUB_WINDOW, WindowMode::WINDOW_MODE_FULLSCREEN, windowRect_);
    sptr<WindowNode> subNode = new WindowNode(subProperty, nullptr, nullptr);
    subNode->SetWindowProperty(subProperty);
    sptr<WindowNode> rootNode = nullptr;
    ASSERT_EQ(WMError::WM_OK, container->AddWindowNode(parentNode, rootNode));
    ASSERT_EQ(WMError::WM_OK, container->AddWindowNodeOnWindowTree(subNode, parentNode));
    ASSERT_EQ(WMError::WM_OK, container->AddWindowNode(subNode, parentNode));
    ASSERT_EQ(1, parentNode->children_.size());
    container->RemoveWindowNodeFromWindowTree(subNode);
    ASSERT_EQ(0, parentNode->children_.size());
}

/**
 * @tc.name: RemoveWindowNode01
 * @tc.desc: remove main window
 * @tc.type: FUNC
 */
HWTEST_F(WindowNodeContainerTest, RemoveWindowNode01, TestSize.Level1)
{
    sptr<WindowNodeContainer> container = new WindowNodeContainer(defaultDisplay_->GetDisplayInfo(),
        defaultDisplay_->GetScreenId());
    sptr<WindowNode> node = nullptr;
    ASSERT_EQ(WMError::WM_ERROR_DESTROYED_OBJECT, container->RemoveWindowNode(node));
    sptr<WindowProperty> property = CreateWindowProperty(111u, "test1", WindowType::WINDOW_TYPE_APP_MAIN_WINDOW,
        WindowMode::WINDOW_MODE_FULLSCREEN, windowRect_);
    node = new WindowNode(property, nullptr, nullptr);
    node->SetWindowProperty(property);
    sptr<WindowNode> parentNode = nullptr;
    node->GetWindowProperty()->SetPrivacyMode(true);
    ASSERT_EQ(WMError::WM_OK, container->AddWindowNode(node, parentNode));
    node->GetWindowProperty()->SetPrivacyMode(false);
    ASSERT_EQ(WMError::WM_OK, container->RemoveWindowNode(node));
}

/**
 * @tc.name: RemoveWindowNode02
 * @tc.desc: remove sub window
 * @tc.type: FUNC
 */
HWTEST_F(WindowNodeContainerTest, RemoveWindowNode02, TestSize.Level1)
{
    sptr<WindowNodeContainer> container = new WindowNodeContainer(defaultDisplay_->GetDisplayInfo(),
        defaultDisplay_->GetScreenId());
    sptr<WindowProperty> parentProperty = CreateWindowProperty(110u, "test1",
        WindowType::WINDOW_TYPE_APP_MAIN_WINDOW, WindowMode::WINDOW_MODE_FULLSCREEN, windowRect_);
    sptr<WindowNode> parentNode = new WindowNode(parentProperty, nullptr, nullptr);
    parentNode->SetWindowProperty(parentProperty);
    sptr<WindowProperty> subProperty = CreateWindowProperty(111u, "test2",
        WindowType::WINDOW_TYPE_APP_SUB_WINDOW, WindowMode::WINDOW_MODE_FULLSCREEN, windowRect_);
    sptr<WindowNode> subNode = new WindowNode(subProperty, nullptr, nullptr);
    subNode->SetWindowProperty(subProperty);

    sptr<WindowNode> rootNode = nullptr;
    ASSERT_EQ(WMError::WM_OK, container->AddWindowNode(parentNode, rootNode));
    ASSERT_EQ(WMError::WM_OK, container->AddWindowNodeOnWindowTree(subNode, parentNode));
    ASSERT_EQ(WMError::WM_OK, container->AddWindowNode(subNode, parentNode));
    ASSERT_EQ(WMError::WM_OK, container->RemoveWindowNode(subNode));
}

/**
 * @tc.name: RemoveWindowNode03
 * @tc.desc: remove keyguard window
 * @tc.type: FUNC
 */
HWTEST_F(WindowNodeContainerTest, RemoveWindowNode03, TestSize.Level1)
{
    sptr<WindowNodeContainer> container = new WindowNodeContainer(defaultDisplay_->GetDisplayInfo(),
        defaultDisplay_->GetScreenId());
    sptr<WindowProperty> property = CreateWindowProperty(111u, "test1", WindowType::WINDOW_TYPE_APP_MAIN_WINDOW,
        WindowMode::WINDOW_MODE_FULLSCREEN, windowRect_);
    sptr<WindowNode> node = new WindowNode(property, nullptr, nullptr);
    node->SetWindowProperty(property);
    sptr<WindowNode> parentNode = nullptr;
    ASSERT_EQ(WMError::WM_OK, container->AddWindowNode(node, parentNode));
    ASSERT_EQ(WMError::WM_OK, container->RemoveWindowNode(node));
}

/**
 * @tc.name: RemoveWindowNode04
 * @tc.desc: remove boot animation window
 * @tc.type: FUNC
 */
HWTEST_F(WindowNodeContainerTest, RemoveWindowNode04, TestSize.Level1)
{
    sptr<WindowNodeContainer> container = new WindowNodeContainer(defaultDisplay_->GetDisplayInfo(),
        defaultDisplay_->GetScreenId());
    sptr<WindowProperty> property = CreateWindowProperty(111u, "test1", WindowType::WINDOW_TYPE_APP_MAIN_WINDOW,
        WindowMode::WINDOW_MODE_FULLSCREEN, windowRect_);
    sptr<WindowNode> node = new WindowNode(property, nullptr, nullptr);
    node->SetWindowProperty(property);
    sptr<WindowNode> parentNode = nullptr;
    ASSERT_EQ(WMError::WM_OK, container->AddWindowNode(node, parentNode));
    ASSERT_EQ(WMError::WM_OK, container->RemoveWindowNode(node));
}

/**
 * @tc.name: HandleRemoveWindow01
 * @tc.desc: remove status bar
 * @tc.type: FUNC
 */
HWTEST_F(WindowNodeContainerTest, HandleRemoveWindow01, TestSize.Level1)
{
    sptr<WindowNodeContainer> container = new WindowNodeContainer(defaultDisplay_->GetDisplayInfo(),
        defaultDisplay_->GetScreenId());
    sptr<WindowProperty> property = CreateWindowProperty(111u, "test1", WindowType::WINDOW_TYPE_STATUS_BAR,
        WindowMode::WINDOW_MODE_FULLSCREEN, windowRect_);
    sptr<WindowNode> node = new WindowNode(property, nullptr, nullptr);
    node->SetWindowProperty(property);
    sptr<WindowNode> parentNode = nullptr;
    ASSERT_EQ(WMError::WM_OK, container->AddWindowNode(node, parentNode));
    ASSERT_EQ(WMError::WM_OK, container->HandleRemoveWindow(node));
}

/**
 * @tc.name: HandleRemoveWindow02
 * @tc.desc: remove navigation bar
 * @tc.type: FUNC
 */
HWTEST_F(WindowNodeContainerTest, HandleRemoveWindow02, TestSize.Level1)
{
    sptr<WindowNodeContainer> container = new WindowNodeContainer(defaultDisplay_->GetDisplayInfo(),
        defaultDisplay_->GetScreenId());
    sptr<WindowProperty> property = CreateWindowProperty(111u, "test1", WindowType::WINDOW_TYPE_NAVIGATION_BAR,
        WindowMode::WINDOW_MODE_FULLSCREEN, windowRect_);
    sptr<WindowNode> node = new WindowNode(property, nullptr, nullptr);
    node->SetWindowProperty(property);
    sptr<WindowNode> parentNode = nullptr;
    ASSERT_EQ(WMError::WM_OK, container->AddWindowNode(node, parentNode));
    ASSERT_EQ(WMError::WM_OK, container->HandleRemoveWindow(node));
}

/**
 * @tc.name: FindDividerNode
 * @tc.desc: find divider node
 * @tc.type: FUNC
 */
HWTEST_F(WindowNodeContainerTest, FindDividerNode, TestSize.Level1)
{
    sptr<WindowNodeContainer> container = new WindowNodeContainer(defaultDisplay_->GetDisplayInfo(),
        defaultDisplay_->GetScreenId());
    ASSERT_EQ(nullptr, container->FindDividerNode());
    sptr<WindowProperty> property = CreateWindowProperty(111u, "test1", WindowType::WINDOW_TYPE_APP_MAIN_WINDOW,
        WindowMode::WINDOW_MODE_FULLSCREEN, windowRect_);
    sptr<WindowNode> node = new WindowNode(property, nullptr, nullptr);
    node->SetWindowProperty(property);
    sptr<WindowNode> parentNode = nullptr;
    ASSERT_EQ(WMError::WM_OK, container->AddWindowNode(node, parentNode));
    ASSERT_EQ(1, container->appWindowNode_->children_.size());
    ASSERT_EQ(nullptr, container->FindDividerNode());
    sptr<WindowProperty> dividerProperty = CreateWindowProperty(112u, "test2", WindowType::WINDOW_TYPE_DOCK_SLICE,
        WindowMode::WINDOW_MODE_FULLSCREEN, windowRect_);
    sptr<WindowNode> dividerNode = new WindowNode(dividerProperty, nullptr, nullptr);
    dividerNode->SetWindowProperty(dividerProperty);
    ASSERT_EQ(WMError::WM_OK, container->AddWindowNode(dividerNode, parentNode));
    ASSERT_EQ(2, container->appWindowNode_->children_.size());
    ASSERT_EQ(dividerNode, container->FindDividerNode());
}

/**
 * @tc.name: RaiseZOrderForAppWindow01
 * @tc.desc: raise main window z order
 * @tc.type: FUNC
 */
HWTEST_F(WindowNodeContainerTest, RaiseZOrderForAppWindow01, TestSize.Level1)
{
    sptr<WindowNodeContainer> container = new WindowNodeContainer(defaultDisplay_->GetDisplayInfo(),
        defaultDisplay_->GetScreenId());
    sptr<WindowProperty> property1 = CreateWindowProperty(111u, "test1", WindowType::WINDOW_TYPE_APP_MAIN_WINDOW,
        WindowMode::WINDOW_MODE_FULLSCREEN, windowRect_);
    sptr<WindowNode> node1 = nullptr;
    sptr<WindowNode> parentNode = nullptr;
    ASSERT_EQ(WMError::WM_ERROR_NULLPTR, container->RaiseZOrderForAppWindow(node1, parentNode));
    node1 = new WindowNode(property1, nullptr, nullptr);
    node1->SetWindowProperty(property1);
    ASSERT_EQ(WMError::WM_OK, container->AddWindowNode(node1, parentNode));
    sptr<WindowProperty> property2 = CreateWindowProperty(112u, "test2", WindowType::WINDOW_TYPE_APP_MAIN_WINDOW,
        WindowMode::WINDOW_MODE_FULLSCREEN, windowRect_);
    sptr<WindowNode> node2 = new WindowNode(property2, nullptr, nullptr);
    node2->SetWindowProperty(property2);
    ASSERT_EQ(WMError::WM_OK, container->AddWindowNode(node2, parentNode));
    ASSERT_EQ(WMError::WM_OK, container->RaiseZOrderForAppWindow(node1, parentNode));
}

/**
 * @tc.name: RaiseZOrderForAppWindow02
 * @tc.desc: raise sub window z order
 * @tc.type: FUNC
 */
HWTEST_F(WindowNodeContainerTest, RaiseZOrderForAppWindow02, TestSize.Level1)
{
    sptr<WindowNodeContainer> container = new WindowNodeContainer(defaultDisplay_->GetDisplayInfo(),
        defaultDisplay_->GetScreenId());
    sptr<WindowProperty> parentProperty = CreateWindowProperty(110u, "test1",
        WindowType::WINDOW_TYPE_APP_MAIN_WINDOW, WindowMode::WINDOW_MODE_FULLSCREEN, windowRect_);
    sptr<WindowNode> parentNode = new WindowNode(parentProperty, nullptr, nullptr);
    parentNode->SetWindowProperty(parentProperty);
    sptr<WindowProperty> subProperty = CreateWindowProperty(111u, "test2",
        WindowType::WINDOW_TYPE_APP_SUB_WINDOW, WindowMode::WINDOW_MODE_FULLSCREEN, windowRect_);
    sptr<WindowNode> subNode = new WindowNode(subProperty, nullptr, nullptr);
    subNode->SetWindowProperty(subProperty);
    sptr<WindowNode> rootNode = nullptr;
    ASSERT_EQ(WMError::WM_ERROR_NULLPTR, container->RaiseZOrderForAppWindow(subNode, rootNode));
    ASSERT_EQ(WMError::WM_OK, container->RaiseZOrderForAppWindow(subNode, parentNode));
}

/**
 * @tc.name: RaiseZOrderForAppWindow03
 * @tc.desc: raise dialog z order
 * @tc.type: FUNC
 */
HWTEST_F(WindowNodeContainerTest, RaiseZOrderForAppWindow03, TestSize.Level1)
{
    sptr<WindowNodeContainer> container = new WindowNodeContainer(defaultDisplay_->GetDisplayInfo(),
        defaultDisplay_->GetScreenId());
    sptr<WindowProperty> parentProperty = CreateWindowProperty(110u, "test1",
        WindowType::WINDOW_TYPE_APP_MAIN_WINDOW, WindowMode::WINDOW_MODE_FULLSCREEN, windowRect_);
    sptr<WindowNode> parentNode = new WindowNode(parentProperty, nullptr, nullptr);
    parentNode->SetWindowProperty(parentProperty);
    sptr<WindowProperty> dialogProperty = CreateWindowProperty(111u, "test2",
        WindowType::WINDOW_TYPE_DIALOG, WindowMode::WINDOW_MODE_FULLSCREEN, windowRect_);
    sptr<WindowNode> dialog = new WindowNode(dialogProperty, nullptr, nullptr);
    dialog->SetWindowProperty(dialogProperty);
    sptr<WindowNode> rootNode = nullptr;
    ASSERT_EQ(WMError::WM_ERROR_NULLPTR, container->RaiseZOrderForAppWindow(dialog, rootNode));
    ASSERT_EQ(WMError::WM_OK, container->RaiseZOrderForAppWindow(dialog, parentNode));
}

/**
 * @tc.name: IsDockSliceInExitSplitModeArea
 * @tc.desc: if dock slice in exit split mode area
 * @tc.type: FUNC
 */
HWTEST_F(WindowNodeContainerTest, IsDockSliceInExitSplitModeArea, TestSize.Level1)
{
    sptr<WindowNodeContainer> container = new WindowNodeContainer(defaultDisplay_->GetDisplayInfo(),
        defaultDisplay_->GetScreenId());
    ASSERT_NE(nullptr, container->displayGroupController_);
    ASSERT_NE(nullptr, container->displayGroupController_->GetWindowPairByDisplayId(defaultDisplay_->GetId()));
    ASSERT_TRUE(!container->IsDockSliceInExitSplitModeArea(defaultDisplay_->GetId()));
}

/**
 * @tc.name: ExitSplitMode
 * @tc.desc: exit split mode
 * @tc.type: FUNC
 */
HWTEST_F(WindowNodeContainerTest, ExitSplitMode, TestSize.Level1)
{
    sptr<WindowNodeContainer> container = new WindowNodeContainer(defaultDisplay_->GetDisplayInfo(),
        defaultDisplay_->GetScreenId());
    ASSERT_NE(nullptr, container->displayGroupController_);
    ASSERT_NE(nullptr, container->displayGroupController_->GetWindowPairByDisplayId(defaultDisplay_->GetId()));
    container->ExitSplitMode(defaultDisplay_->GetId());
}

/**
 * @tc.name: MinimizeOldestAppWindow01
 * @tc.desc: minimize main window
 * @tc.type: FUNC
 */
HWTEST_F(WindowNodeContainerTest, MinimizeOldestAppWindow01, TestSize.Level1)
{
    sptr<WindowNodeContainer> container = new WindowNodeContainer(defaultDisplay_->GetDisplayInfo(),
        defaultDisplay_->GetScreenId());
    container->MinimizeOldestAppWindow();
    sptr<WindowProperty> property = CreateWindowProperty(111u, "test1", WindowType::WINDOW_TYPE_APP_MAIN_WINDOW,
        WindowMode::WINDOW_MODE_FULLSCREEN, windowRect_);
    sptr<WindowNode> node = new WindowNode(property, nullptr, nullptr);
    node->SetWindowProperty(property);
    sptr<WindowNode> parentNode = nullptr;
    ASSERT_EQ(WMError::WM_OK, container->AddWindowNode(node, parentNode));
    ASSERT_EQ(1, container->appWindowNode_->children_.size());
    container->MinimizeOldestAppWindow();
    ASSERT_EQ(1, MinimizeApp::needMinimizeAppNodes_.size());
    MinimizeApp::needMinimizeAppNodes_.clear();
}

/**
 * @tc.name: MinimizeOldestAppWindow02
 * @tc.desc: minimize above main window
 * @tc.type: FUNC
 */
HWTEST_F(WindowNodeContainerTest, MinimizeOldestAppWindow02, TestSize.Level1)
{
    sptr<WindowNodeContainer> container = new WindowNodeContainer(defaultDisplay_->GetDisplayInfo(),
        defaultDisplay_->GetScreenId());
    sptr<WindowProperty> property = CreateWindowProperty(111u, "test2",
        WindowType::WINDOW_TYPE_STATUS_BAR, WindowMode::WINDOW_MODE_FULLSCREEN, windowRect_);
    sptr<WindowNode> node = new WindowNode(property, nullptr, nullptr);
    node->SetWindowProperty(property);
    sptr<WindowNode> parentNode = nullptr;
    ASSERT_EQ(WMError::WM_OK, container->AddWindowNode(node, parentNode));
    ASSERT_EQ(1, container->aboveAppWindowNode_->children_.size());
    size_t size = MinimizeApp::needMinimizeAppNodes_.size();
    container->MinimizeOldestAppWindow();
    ASSERT_EQ(size, MinimizeApp::needMinimizeAppNodes_.size());
    MinimizeApp::needMinimizeAppNodes_.clear();
}

/**
 * @tc.name: ToggleShownStateForAllAppWindows01
 * @tc.desc: toggle shown state for status bar
 * @tc.type: FUNC
 */
HWTEST_F(WindowNodeContainerTest, ToggleShownStateForAllAppWindows01, TestSize.Level1)
{
    sptr<WindowNodeContainer> container = new WindowNodeContainer(defaultDisplay_->GetDisplayInfo(),
        defaultDisplay_->GetScreenId());
    sptr<WindowProperty> property = CreateWindowProperty(111u, "test2",
        WindowType::WINDOW_TYPE_STATUS_BAR, WindowMode::WINDOW_MODE_FULLSCREEN, windowRect_);
    sptr<WindowNode> statusBar = new WindowNode(property, nullptr, nullptr);
    statusBar->SetWindowProperty(property);
    sptr<WindowNode> parentNode = nullptr;
    ASSERT_EQ(WMError::WM_OK, container->AddWindowNode(statusBar, parentNode));
    ASSERT_EQ(1, container->aboveAppWindowNode_->children_.size());
    auto restoreFunc = [](uint32_t windowId, WindowMode mode) {
        return false;
    };
    ASSERT_EQ(WMError::WM_OK, container->ToggleShownStateForAllAppWindows(restoreFunc, true));
}

/**
 * @tc.name: ToggleShownStateForAllAppWindows02
 * @tc.desc: toggle shown state for launcher recent
 * @tc.type: FUNC
 */
HWTEST_F(WindowNodeContainerTest, ToggleShownStateForAllAppWindows02, TestSize.Level1)
{
    sptr<WindowNodeContainer> container = new WindowNodeContainer(defaultDisplay_->GetDisplayInfo(),
        defaultDisplay_->GetScreenId());
    sptr<WindowProperty> property = CreateWindowProperty(111u, "test2",
        WindowType::WINDOW_TYPE_LAUNCHER_RECENT, WindowMode::WINDOW_MODE_FULLSCREEN, windowRect_);
    sptr<WindowNode> statusBar = new WindowNode(property, nullptr, nullptr);
    statusBar->SetWindowProperty(property);
    sptr<WindowNode> parentNode = nullptr;
    ASSERT_EQ(WMError::WM_OK, container->AddWindowNode(statusBar, parentNode));
    ASSERT_EQ(1, container->aboveAppWindowNode_->children_.size());
    auto restoreFunc = [](uint32_t windowId, WindowMode mode) {
        return false;
    };
    ASSERT_EQ(WMError::WM_DO_NOTHING, container->ToggleShownStateForAllAppWindows(restoreFunc, true));
}

/**
 * @tc.name: SetWindowMode01
 * @tc.desc: set main window mode
 * @tc.type: FUNC
 */
HWTEST_F(WindowNodeContainerTest, SetWindowMode01, TestSize.Level1)
{
    sptr<WindowNodeContainer> container = new WindowNodeContainer(defaultDisplay_->GetDisplayInfo(),
        defaultDisplay_->GetScreenId());
    sptr<WindowNode> node = nullptr;
    WindowMode dstMode = WindowMode::WINDOW_MODE_FLOATING;
    ASSERT_EQ(WMError::WM_ERROR_NULLPTR, container->SetWindowMode(node, dstMode));
    sptr<WindowProperty> property = CreateWindowProperty(111u, "test1", WindowType::WINDOW_TYPE_APP_MAIN_WINDOW,
        WindowMode::WINDOW_MODE_FULLSCREEN, windowRect_);
    node = new WindowNode(property, nullptr, nullptr);
    node->SetWindowProperty(property);
    sptr<WindowNode> parentNode = nullptr;
    ASSERT_EQ(WMError::WM_OK, container->AddWindowNode(node, parentNode));
    ASSERT_EQ(1, container->appWindowNode_->children_.size());
    dstMode = WindowMode::WINDOW_MODE_SPLIT_PRIMARY;
    container->isScreenLocked_ = false;
    ASSERT_EQ(WMError::WM_OK, container->SetWindowMode(node, dstMode));
}

/**
 * @tc.name: SetWindowMode02
 * @tc.desc: set main window mode with show when locked
 * @tc.type: FUNC
 */
HWTEST_F(WindowNodeContainerTest, SetWindowMode02, TestSize.Level1)
{
    sptr<WindowNodeContainer> container = new WindowNodeContainer(defaultDisplay_->GetDisplayInfo(),
        defaultDisplay_->GetScreenId());
    sptr<WindowProperty> property = CreateWindowProperty(111u, "test1", WindowType::WINDOW_TYPE_APP_MAIN_WINDOW,
        WindowMode::WINDOW_MODE_FULLSCREEN, windowRect_);
    property->SetWindowFlags(static_cast<uint32_t>(WindowFlag::WINDOW_FLAG_SHOW_WHEN_LOCKED));
    sptr<WindowNode> node = new WindowNode(property, nullptr, nullptr);
    node->SetWindowProperty(property);
    sptr<WindowNode> parentNode = nullptr;
    ASSERT_EQ(WMError::WM_OK, container->AddWindowNode(node, parentNode));
    ASSERT_EQ(1, container->appWindowNode_->children_.size());
    WindowMode dstMode = WindowMode::WINDOW_MODE_SPLIT_PRIMARY;
    container->isScreenLocked_ = true;
    ASSERT_EQ(WMError::WM_ERROR_INVALID_PARAM, container->SetWindowMode(node, dstMode));
}

/**
 * @tc.name: RemoveSingleUserWindowNodes
 * @tc.desc: remove single user window node
 * @tc.type: FUNC
 */
HWTEST_F(WindowNodeContainerTest, RemoveSingleUserWindowNodes, TestSize.Level1)
{
    sptr<WindowNodeContainer> container = new WindowNodeContainer(defaultDisplay_->GetDisplayInfo(),
        defaultDisplay_->GetScreenId());
    sptr<WindowProperty> property = CreateWindowProperty(111u, "test1", WindowType::WINDOW_TYPE_APP_MAIN_WINDOW,
        WindowMode::WINDOW_MODE_FULLSCREEN, windowRect_);
    property->SetWindowFlags(static_cast<uint32_t>(WindowFlag::WINDOW_FLAG_SHOW_WHEN_LOCKED));
    sptr<WindowNode> node = new WindowNode(property, nullptr, nullptr);
    node->SetWindowProperty(property);
    sptr<WindowNode> parentNode = nullptr;
    ASSERT_EQ(WMError::WM_OK, container->AddWindowNode(node, parentNode));
    ASSERT_EQ(1, container->appWindowNode_->children_.size());
    int accountId = 0;
    container->RemoveSingleUserWindowNodes(accountId);
}

/**
 * @tc.name: TakeWindowPairSnapshot
 * @tc.desc: take window pair snapshot
 * @tc.type: FUNC
 */
HWTEST_F(WindowNodeContainerTest, TakeWindowPairSnapshot, TestSize.Level1)
{
    sptr<WindowNodeContainer> container = new WindowNodeContainer(defaultDisplay_->GetDisplayInfo(),
        defaultDisplay_->GetScreenId());
    sptr<WindowProperty> property = CreateWindowProperty(111u, "test1", WindowType::WINDOW_TYPE_APP_MAIN_WINDOW,
        WindowMode::WINDOW_MODE_FULLSCREEN, windowRect_);
    property->SetWindowFlags(static_cast<uint32_t>(WindowFlag::WINDOW_FLAG_SHOW_WHEN_LOCKED));
    sptr<WindowNode> node = new WindowNode(property, nullptr, nullptr);
    node->SetWindowProperty(property);
    sptr<WindowNode> parentNode = nullptr;
    ASSERT_EQ(WMError::WM_OK, container->AddWindowNode(node, parentNode));
    ASSERT_EQ(1, container->appWindowNode_->children_.size());
    ASSERT_NE(nullptr, container->displayGroupController_);
    ASSERT_NE(nullptr, container->displayGroupController_->GetWindowPairByDisplayId(defaultDisplay_->GetId()));
    ASSERT_TRUE(!container->TakeWindowPairSnapshot(defaultDisplay_->GetId()));
    container->ClearWindowPairSnapshot(defaultDisplay_->GetId());
}

/**
 * @tc.name: Destroy
 * @tc.desc: clear vector cache completely, swap with empty vector
 * @tc.type: FUNC
 */
HWTEST_F(WindowNodeContainerTest, Destroy, TestSize.Level1)
{
    ASSERT_EQ(0, container_->Destroy().size());
}

/**
 * @tc.name: UpdatePrivateStateAndNotify
 * @tc.desc: update private window count
 * @tc.type: FUNC
 */
HWTEST_F(WindowNodeContainerTest, UpdatePrivateStateAndNotify, TestSize.Level1)
{
    container_->belowAppWindowNode_->children_.clear();
    container_->appWindowNode_->children_.clear();
    container_->aboveAppWindowNode_->children_.clear();
    container_->privateWindowCount_ = 0;
    // private window count : from 0 to 1
    sptr<WindowProperty> property = CreateWindowProperty(111u, "test1", WindowType::WINDOW_TYPE_APP_MAIN_WINDOW,
        WindowMode::WINDOW_MODE_FULLSCREEN, windowRect_);
    sptr<WindowNode> node = new WindowNode(property, nullptr, nullptr);
    node->SetWindowProperty(property);
    node->GetWindowProperty()->SetPrivacyMode(true);
    container_->appWindowNode_->children_.emplace_back(node);
    container_->UpdatePrivateStateAndNotify();
    ASSERT_EQ(1, container_->privateWindowCount_);
    // private window count : from 1 to 0
    container_->appWindowNode_->children_.clear();
    container_->UpdatePrivateStateAndNotify();
    ASSERT_EQ(0, container_->privateWindowCount_);
}

/**
 * @tc.name: MinimizeOldestMainFloatingWindow
 * @tc.desc: check function MinimizeOldestMainFloatingWindow
 * @tc.type: FUNC
 */
HWTEST_F(WindowNodeContainerTest, MinimizeOldestMainFloatingWindow, TestSize.Level1)
{
    sptr<WindowNodeContainer> container = new WindowNodeContainer(defaultDisplay_->GetDisplayInfo(),
        defaultDisplay_->GetScreenId());

    sptr<WindowProperty> property1 = CreateWindowProperty(110u, "test1",
        WindowType::WINDOW_TYPE_APP_MAIN_WINDOW, WindowMode::WINDOW_MODE_FLOATING, windowRect_);
    sptr<WindowNode> window1 = new WindowNode(property1, nullptr, nullptr);
    window1->SetWindowProperty(property1);
    sptr<WindowProperty> property2 = CreateWindowProperty(111u, "test2",
        WindowType::WINDOW_TYPE_APP_MAIN_WINDOW, WindowMode::WINDOW_MODE_FLOATING, windowRect_);
    sptr<WindowNode> window2 = new WindowNode(property2, nullptr, nullptr);
    window2->SetWindowProperty(property2);

    container->appWindowNode_->children_.push_back(window1);
    container->appWindowNode_->children_.push_back(window2);

    window1->parent_ = container->appWindowNode_;
    window2->parent_ = container->appWindowNode_;

    auto oldInfo = MinimizeApp::needMinimizeAppNodes_[MinimizeReason::MAX_APP_COUNT];
    MinimizeApp::ClearNodesWithReason(MinimizeReason::MAX_APP_COUNT);

    container->maxMainFloatingWindowNumber_ = -1;
    container->MinimizeOldestMainFloatingWindow(110u);
    auto needMinimizeNum = MinimizeApp::GetNeedMinimizeAppNodesWithReason(MinimizeReason::MAX_APP_COUNT);
    ASSERT_EQ(needMinimizeNum.size(), 0);
    MinimizeApp::ClearNodesWithReason(MinimizeReason::MAX_APP_COUNT);

    container->maxMainFloatingWindowNumber_ = 3;
    container->MinimizeOldestMainFloatingWindow(110u);
    needMinimizeNum = MinimizeApp::GetNeedMinimizeAppNodesWithReason(MinimizeReason::MAX_APP_COUNT);
    ASSERT_EQ(needMinimizeNum.size(), 0);
    MinimizeApp::ClearNodesWithReason(MinimizeReason::MAX_APP_COUNT);

    container->maxMainFloatingWindowNumber_ = 1;
    container->MinimizeOldestMainFloatingWindow(110u);
    needMinimizeNum = MinimizeApp::GetNeedMinimizeAppNodesWithReason(MinimizeReason::MAX_APP_COUNT);
    ASSERT_EQ(needMinimizeNum.size(), 1);
    MinimizeApp::ClearNodesWithReason(MinimizeReason::MAX_APP_COUNT);

    sptr<WindowProperty> property3 = CreateWindowProperty(112u, "test3",
        WindowType::WINDOW_TYPE_APP_MAIN_WINDOW, WindowMode::WINDOW_MODE_FULLSCREEN, windowRect_);
    sptr<WindowNode> window3 = new WindowNode(property3, nullptr, nullptr);
    window3->SetWindowProperty(property3);
    container->appWindowNode_->children_.push_back(window3);
    window3->parent_ = container->appWindowNode_;

    container->maxMainFloatingWindowNumber_ = 1;
    container->MinimizeOldestMainFloatingWindow(110u);
    needMinimizeNum = MinimizeApp::GetNeedMinimizeAppNodesWithReason(MinimizeReason::MAX_APP_COUNT);
    ASSERT_EQ(needMinimizeNum.size(), 1);
    MinimizeApp::ClearNodesWithReason(MinimizeReason::MAX_APP_COUNT);

    auto headItor = MinimizeApp::needMinimizeAppNodes_[MinimizeReason::MAX_APP_COUNT].end();
    MinimizeApp::needMinimizeAppNodes_[MinimizeReason::MAX_APP_COUNT].insert(headItor, oldInfo.begin(), oldInfo.end());
}

/**
 * @tc.name: GetMainFloatingWindowCount
 * @tc.desc: check GetMainFloatingWindowCount
 * @tc.type: FUNC
 */
HWTEST_F(WindowNodeContainerTest, GetMainFloatingWindowCount, TestSize.Level1)
{
    sptr<WindowNodeContainer> container = new WindowNodeContainer(defaultDisplay_->GetDisplayInfo(),
        defaultDisplay_->GetScreenId());

    sptr<WindowProperty> property1 = CreateWindowProperty(110u, "test1",
        WindowType::WINDOW_TYPE_APP_MAIN_WINDOW, WindowMode::WINDOW_MODE_FLOATING, windowRect_);
    sptr<WindowNode> window1 = new WindowNode(property1, nullptr, nullptr);
    window1->SetWindowProperty(property1);

    sptr<WindowProperty> property2 = CreateWindowProperty(111u, "test2",
        WindowType::WINDOW_TYPE_APP_MAIN_WINDOW, WindowMode::WINDOW_MODE_FLOATING, windowRect_);
    sptr<WindowNode> window2 = new WindowNode(property2, nullptr, nullptr);
    window2->SetWindowProperty(property2);
    window2->startingWindowShown_ = true;

    sptr<WindowProperty> property3 = CreateWindowProperty(112u, "test3",
        WindowType::WINDOW_TYPE_APP_MAIN_WINDOW, WindowMode::WINDOW_MODE_FULLSCREEN, windowRect_);
    sptr<WindowNode> window3 = new WindowNode(property3, nullptr, nullptr);
    window3->SetWindowProperty(property3);

    sptr<WindowProperty> property4 = CreateWindowProperty(112u, "test3",
        WindowType::WINDOW_TYPE_APP_MAIN_WINDOW, WindowMode::WINDOW_MODE_FLOATING, windowRect_);
    sptr<WindowNode> window4 = new WindowNode(property4, nullptr, nullptr);
    window4->SetWindowProperty(property4);

    container->appWindowNode_->children_.push_back(window1);
    container->aboveAppWindowNode_->children_.push_back(window2);
    container->appWindowNode_->children_.push_back(window3);
    container->aboveAppWindowNode_->children_.push_back(window4);

    auto result = container->GetMainFloatingWindowCount();
    ASSERT_EQ(result, 3);
}

/**
 * @tc.name: ResetWindowZOrderPriorityWhenSetMode
 * @tc.desc: check function ResetWindowZOrderPriorityWhenSetMode
 * @tc.type: FUNC
 */
HWTEST_F(WindowNodeContainerTest, ResetWindowZOrderPriorityWhenSetMode, TestSize.Level1)
{
    sptr<WindowNodeContainer> container = new WindowNodeContainer(defaultDisplay_->GetDisplayInfo(),
        defaultDisplay_->GetScreenId());

    sptr<WindowProperty> property1 = CreateWindowProperty(110u, "test1",
        WindowType::WINDOW_TYPE_APP_MAIN_WINDOW, WindowMode::WINDOW_MODE_FLOATING, windowRect_);
    sptr<WindowNode> window1 = new WindowNode(property1, nullptr, nullptr);
    window1->SetWindowProperty(property1);
    window1->parent_ = container->appWindowNode_;

    sptr<WindowProperty> property2 = CreateWindowProperty(111u, "test2",
        WindowType::WINDOW_TYPE_APP_MAIN_WINDOW, WindowMode::WINDOW_MODE_FULLSCREEN, windowRect_);
    sptr<WindowNode> window2 = new WindowNode(property2, nullptr, nullptr);
    window2->SetWindowProperty(property2);
    window2->parent_ = container->appWindowNode_;

    container->appWindowNode_->children_.push_back(window1);
    container->appWindowNode_->children_.push_back(window2);

    container->isFloatWindowAboveFullWindow_ = false;
    container->ResetWindowZOrderPriorityWhenSetMode(window1, WindowMode::WINDOW_MODE_FLOATING,
        WindowMode::WINDOW_MODE_FULLSCREEN);
    ASSERT_EQ(window1->priority_, 0);

    container->isFloatWindowAboveFullWindow_ = true;
    std::map<std::pair<WindowMode, WindowMode>, uint32_t> pairMode = {
        {{WindowMode::WINDOW_MODE_FLOATING, WindowMode::WINDOW_MODE_FULLSCREEN}, 2},
        {{WindowMode::WINDOW_MODE_FULLSCREEN, WindowMode::WINDOW_MODE_FLOATING}, 2},
        {{WindowMode::WINDOW_MODE_UNDEFINED, WindowMode::WINDOW_MODE_FULLSCREEN}, 0},
        {{WindowMode::WINDOW_MODE_FULLSCREEN, WindowMode::WINDOW_MODE_SPLIT_PRIMARY}, 2},
        {{WindowMode::WINDOW_MODE_SPLIT_PRIMARY, WindowMode::WINDOW_MODE_SPLIT_SECONDARY}, 2},
        {{WindowMode::WINDOW_MODE_SPLIT_SECONDARY, WindowMode::WINDOW_MODE_SPLIT_PRIMARY}, 2},
    };
    container->focusedWindow_ = 111u;
    for (auto itor = pairMode.begin(); itor != pairMode.end(); itor++) {
        auto pair = itor->first;
        container->ResetWindowZOrderPriorityWhenSetMode(window1, pair.first, pair.second);
        window1->priority_ = 0;
    }

    window1->zOrder_ = 0;
    window1->surfaceNode_ = CreateSFNode("sfNode1");
    window2->surfaceNode_ = CreateSFNode("sfNode2");
    container->focusedWindow_ = window1->GetWindowId();
    container->ResetWindowZOrderPriorityWhenSetMode(window1, WindowMode::WINDOW_MODE_FLOATING,
        WindowMode::WINDOW_MODE_FULLSCREEN);
    ASSERT_EQ(window1->zOrder_, 0);
}

/**
 * @tc.name: ResetMainFloatingWindowPriorityIfNeeded
 * @tc.desc: check function ResetMainFloatingWindowPriorityIfNeeded
 * @tc.type: FUNC
 */
HWTEST_F(WindowNodeContainerTest, ResetMainFloatingWindowPriorityIfNeeded, TestSize.Level1)
{
    sptr<WindowNodeContainer> container = new WindowNodeContainer(defaultDisplay_->GetDisplayInfo(),
        defaultDisplay_->GetScreenId());

    sptr<WindowProperty> property1 = CreateWindowProperty(110u, "test1",
        WindowType::WINDOW_TYPE_APP_MAIN_WINDOW, WindowMode::WINDOW_MODE_FLOATING, windowRect_);
    sptr<WindowNode> window1 = new WindowNode(property1, nullptr, nullptr);
    window1->SetWindowProperty(property1);
    window1->parent_ = container->appWindowNode_;

    sptr<WindowProperty> property2 = CreateWindowProperty(111u, "test2",
        WindowType::WINDOW_TYPE_APP_MAIN_WINDOW, WindowMode::WINDOW_MODE_FULLSCREEN, windowRect_);
    sptr<WindowNode> window2 = new WindowNode(property2, nullptr, nullptr);
    window2->SetWindowProperty(property2);
    window2->parent_ = container->appWindowNode_;

    container->appWindowNode_->children_.push_back(window1);
    container->appWindowNode_->children_.push_back(window2);

    container->isFloatWindowAboveFullWindow_ = false;
    auto zorderPolicy = container->zorderPolicy_;
    const auto baseZOrderPolicy = zorderPolicy->GetWindowPriority(WindowType::WINDOW_TYPE_APP_MAIN_WINDOW);
    container->ResetMainFloatingWindowPriorityIfNeeded(window1);
    ASSERT_EQ(window1->priority_, 0);

    container->isFloatWindowAboveFullWindow_ = true;
    container->ResetMainFloatingWindowPriorityIfNeeded(window1);
    ASSERT_EQ(window1->priority_, 0);

    container->ResetMainFloatingWindowPriorityIfNeeded(window2);
    ASSERT_EQ(window2->priority_, 0);

    auto displayGroupControl = container->displayGroupController_;
    auto defaultDisplayId = defaultDisplay_->GetId();
    sptr<WindowPair> windowPair = displayGroupControl->GetWindowPairByDisplayId(defaultDisplayId);
    windowPair->status_ = WindowPairStatus::SINGLE_PRIMARY;
    window1->SetDisplayId(defaultDisplayId);
    container->ResetMainFloatingWindowPriorityIfNeeded(window1);
    ASSERT_EQ(window1->priority_, baseZOrderPolicy - 1);
}

/**
 * @tc.name: Destroy
 * @tc.desc: check function ResetAllMainFloatingWindowZOrder
 * @tc.type: FUNC
 */
HWTEST_F(WindowNodeContainerTest, ResetAllMainFloatingWindowZOrder, TestSize.Level1)
{
    sptr<WindowNodeContainer> container = new WindowNodeContainer(defaultDisplay_->GetDisplayInfo(),
        defaultDisplay_->GetScreenId());

    sptr<WindowProperty> property1 = CreateWindowProperty(110u, "test1",
        WindowType::WINDOW_TYPE_APP_MAIN_WINDOW, WindowMode::WINDOW_MODE_FLOATING, windowRect_);
    sptr<WindowNode> window1 = new WindowNode(property1, nullptr, nullptr);
    window1->SetWindowProperty(property1);
    window1->parent_ = container->appWindowNode_;

    sptr<WindowProperty> property2 = CreateWindowProperty(111u, "test2",
        WindowType::WINDOW_TYPE_APP_MAIN_WINDOW, WindowMode::WINDOW_MODE_FULLSCREEN, windowRect_);
    sptr<WindowNode> window2 = new WindowNode(property2, nullptr, nullptr);
    window2->SetWindowProperty(property2);
    window2->parent_ = container->appWindowNode_;

    container->appWindowNode_->children_.push_back(window1);
    container->appWindowNode_->children_.push_back(window2);

    container->isFloatWindowAboveFullWindow_ = false;
    container->ResetAllMainFloatingWindowZOrder(container->appWindowNode_);
    ASSERT_EQ(window1->priority_, 0);

    container->isFloatWindowAboveFullWindow_ = true;
    container->ResetAllMainFloatingWindowZOrder(container->appWindowNode_);
    ASSERT_EQ(window1->priority_, 0);
}
}
}
}