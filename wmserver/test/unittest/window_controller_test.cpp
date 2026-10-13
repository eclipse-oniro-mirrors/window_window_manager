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
#include <map>
#include "display_manager.h"
#include "iremote_object_mocker.h"
#include "mock_rs_iwindow_animation_controller.h"
#include "remote_animation.h"
#include "starting_window.h"
#include "window_controller.h"
#include "scene_board_judgement.h"

using namespace testing;
using namespace testing::ext;

namespace OHOS {
namespace Rosen {
class WindowControllerTest : public testing::Test {
public:
    static void SetUpTestCase();
    static void TearDownTestCase();
    void SetUp() override;
    void TearDown() override;

    static sptr<WindowController> windowController_;
    static sptr<WindowRoot> windowRoot_;
    static sptr<InputWindowMonitor> inputWindowMonitor_;
    static sptr<WindowNode> node_;
    static sptr<WindowTransitionInfo> transitionInfo_;
};

sptr<WindowController> WindowControllerTest::windowController_ = nullptr;
sptr<WindowRoot> WindowControllerTest::windowRoot_ = nullptr;
sptr<InputWindowMonitor> WindowControllerTest::inputWindowMonitor_ = nullptr;
sptr<WindowNode> WindowControllerTest::node_ = nullptr;
sptr<WindowTransitionInfo> WindowControllerTest::transitionInfo_ = nullptr;

void RootCallback(Event event, const sptr<IRemoteObject>& remoteObject)
{
    return;
}

void WindowControllerTest::SetUpTestCase()
{
    windowRoot_ = new WindowRoot(RootCallback);
    windowRoot_->displayIdMap_[0].push_back(0);
    inputWindowMonitor_ = new InputWindowMonitor(windowRoot_);
    windowController_ = new WindowController(windowRoot_, inputWindowMonitor_);
    transitionInfo_ = new WindowTransitionInfo();
    transitionInfo_->SetWindowType(WindowType::WINDOW_TYPE_APP_MAIN_WINDOW);
    node_ = StartingWindow::CreateWindowNode(transitionInfo_, 101); // 101 is windowId
    node_->SetWindowMode(WindowMode::WINDOW_MODE_FULLSCREEN);
}

void WindowControllerTest::TearDownTestCase()
{
}

void WindowControllerTest::SetUp()
{
}

void WindowControllerTest::TearDown()
{
}

namespace {
/**
 * @tc.name: GetSnapshot
 * @tc.desc: test GetSnapshot
 * @tc.type: FUNC
 */
HWTEST_F(WindowControllerTest, GetSnapshot, TestSize.Level1)
{
    int windowId = INVALID_WINDOW_ID;
    ASSERT_EQ(nullptr, windowController_->GetSnapshot(windowId));
}

/**
 * @tc.name: StartingWindow
 * @tc.desc: Window controller starting window
 * @tc.type: FUNC
 */
HWTEST_F(WindowControllerTest, StartingWindow, TestSize.Level1)
{
    windowRoot_->windowNodeMap_.clear();
    windowController_->StartingWindow(nullptr, nullptr, 0, false);
    ASSERT_EQ(0, windowRoot_->windowNodeMap_.size());

    transitionInfo_->SetWindowMode(WindowMode::WINDOW_MODE_UNDEFINED);
    windowController_->StartingWindow(transitionInfo_, nullptr, 0, false);
    ASSERT_EQ(0, windowRoot_->windowNodeMap_.size());

    sptr<IRemoteObject> abilityTokenMocker = new IRemoteObjectMocker();
    transitionInfo_->SetAbilityToken(abilityTokenMocker);
    windowController_->StartingWindow(transitionInfo_, nullptr, 0, false);
    windowController_->StartingWindow(transitionInfo_, nullptr, 0, true);
    ASSERT_EQ(1, windowRoot_->windowNodeMap_.size());

    windowRoot_->windowNodeMap_.clear();
    sptr<IRemoteObject> iRemoteObjectMocker = new IRemoteObjectMocker();
    RemoteAnimation::windowAnimationController_ = iface_cast<RSIWindowAnimationController>(iRemoteObjectMocker);
    windowController_->StartingWindow(transitionInfo_, nullptr, 0, true);
    ASSERT_EQ(1, windowRoot_->windowNodeMap_.size());

    windowRoot_->windowNodeMap_.clear();
    windowRoot_->windowNodeMap_.insert(std::make_pair(node_->GetWindowId(), node_));
    node_->abilityToken_ = abilityTokenMocker;
    node_->stateMachine_.currState_ = WindowNodeState::SHOW_ANIMATION_PLAYING;
    windowController_->StartingWindow(transitionInfo_, nullptr, 0, false);
    ASSERT_EQ(1, windowRoot_->windowNodeMap_.size());

    node_->stateMachine_.currState_ = WindowNodeState::STARTING_CREATED;
    windowController_->StartingWindow(transitionInfo_, nullptr, 0, false);
    transitionInfo_->SetWindowMode(WindowMode::WINDOW_MODE_FULLSCREEN);
    windowController_->StartingWindow(transitionInfo_, nullptr, 0, false);
    transitionInfo_->SetWindowMode(WindowMode::WINDOW_MODE_FLOATING);
    node_->property_->windowModeSupportType_ = WindowModeSupport::WINDOW_MODE_SUPPORT_ALL;
    windowController_->StartingWindow(transitionInfo_, nullptr, 0, false);
    ASSERT_EQ(1, windowRoot_->windowNodeMap_.size());

    // Cancel starting window
    windowController_->CancelStartingWindow(nullptr);
    windowController_->CancelStartingWindow(abilityTokenMocker);

    node_->startingWindowShown_ = true;
    windowController_->CancelStartingWindow(abilityTokenMocker);
    ASSERT_EQ(0, windowRoot_->windowNodeMap_.size());

    windowRoot_->windowNodeMap_.clear();
    RemoteAnimation::windowAnimationController_ = nullptr;
}

/**
 * @tc.name: NotifyWindowTransition
 * @tc.desc: Window controller notify window transtition
 * @tc.type: FUNC
 */
HWTEST_F(WindowControllerTest, NotifyWindowTransition, Function | SmallTest | Level3)
{
    sptr<WindowTransitionInfo> srcInfo = nullptr;
    sptr<WindowTransitionInfo> dstInfo = nullptr;
    ASSERT_EQ(WMError::WM_ERROR_NULLPTR, windowController_->NotifyWindowTransition(srcInfo, dstInfo));

    srcInfo = new WindowTransitionInfo();
    sptr<IRemoteObject> srcAbilityTokenMocker = new IRemoteObjectMocker();
    srcInfo->SetAbilityToken(srcAbilityTokenMocker);
    sptr<WindowNode> srcNode = StartingWindow::CreateWindowNode(srcInfo, 102); // 102 is windowId
    srcNode->property_->windowModeSupportType_ = WindowModeSupport::WINDOW_MODE_SUPPORT_ALL;

    dstInfo = new WindowTransitionInfo();
    sptr<IRemoteObject> dstAbilityTokenMocker = new IRemoteObjectMocker();
    dstInfo->SetAbilityToken(dstAbilityTokenMocker);
    sptr<WindowNode> dstNode = StartingWindow::CreateWindowNode(dstInfo, 103); // 103 is windowId
    dstNode->property_->windowModeSupportType_ = WindowModeSupport::WINDOW_MODE_SUPPORT_ALL;

    windowRoot_->windowNodeMap_.clear();
    windowRoot_->windowNodeMap_.insert(std::make_pair(srcNode->GetWindowId(), srcNode));
    windowRoot_->windowNodeMap_.insert(std::make_pair(dstNode->GetWindowId(), dstNode));

    sptr<DisplayInfo> displayInfo = new DisplayInfo();
    sptr<WindowNodeContainer> container = new WindowNodeContainer(displayInfo, 0);
    windowRoot_->windowNodeContainerMap_.insert(std::make_pair(0, container));

    sptr<MockRSIWindowAnimationController> mock = new MockRSIWindowAnimationController();
    RemoteAnimation::windowAnimationController_ = mock;
    RemoteAnimation::windowRoot_ = windowRoot_;
    RemoteAnimation::animationFirst_ = true;

    srcInfo->SetTransitionReason(TransitionReason::MINIMIZE);
    srcNode->stateMachine_.currState_ = WindowNodeState::HIDDEN;
    ASSERT_EQ(WMError::WM_ERROR_NO_REMOTE_ANIMATION, windowController_->NotifyWindowTransition(srcInfo, dstInfo));

    srcInfo->SetTransitionReason(TransitionReason::MINIMIZE);
    srcNode->stateMachine_.currState_ = WindowNodeState::STARTING_CREATED;
    EXPECT_CALL(*mock, OnMinimizeWindow(_, _)).Times(1);
    ASSERT_EQ(WMError::WM_OK, windowController_->NotifyWindowTransition(srcInfo, dstInfo));

    srcInfo->SetTransitionReason(TransitionReason::CLOSE);
    srcNode->stateMachine_.currState_ = WindowNodeState::STARTING_CREATED;
    EXPECT_CALL(*mock, OnCloseWindow(_, _)).Times(1);
    ASSERT_EQ(WMError::WM_OK, windowController_->NotifyWindowTransition(srcInfo, dstInfo));

    srcInfo->SetTransitionReason(TransitionReason::BACK_TRANSITION);
    srcNode->stateMachine_.currState_ = WindowNodeState::STARTING_CREATED;
    EXPECT_CALL(*mock, OnAppBackTransition(_, _, _)).Times(1);
    ASSERT_EQ(WMError::WM_OK, windowController_->NotifyWindowTransition(srcInfo, dstInfo));

    srcInfo->SetTransitionReason(TransitionReason::ABILITY_TRANSITION);
    dstNode->stateMachine_.currState_ = WindowNodeState::STARTING_CREATED;
    dstNode->property_->SetWindowType(WindowType::WINDOW_TYPE_APP_MAIN_WINDOW);
    dstNode->SetWindowMode(WindowMode::WINDOW_MODE_FLOATING);
    EXPECT_CALL(*mock, OnStartApp(_, _, _)).Times(1);
    ASSERT_EQ(WMError::WM_OK, windowController_->NotifyWindowTransition(srcInfo, dstInfo));

    dstNode->stateMachine_.currState_ = WindowNodeState::STARTING_CREATED;
    dstNode->SetWindowMode(WindowMode::WINDOW_MODE_FULLSCREEN);
    EXPECT_CALL(*mock, OnStartApp(_, _, _)).Times(1);
    ASSERT_EQ(WMError::WM_OK, windowController_->NotifyWindowTransition(srcInfo, dstInfo));

    windowRoot_->windowNodeContainerMap_.clear();
    RemoteAnimation::windowAnimationController_ = nullptr;
}

/**
 * @tc.name: FocusWindow
 * @tc.desc: Window controller focus window
 * @tc.type: FUNC
 */
HWTEST_F(WindowControllerTest, FocusWindow, TestSize.Level1)
{
    sptr<IRemoteObject> abilityToken = nullptr;
    windowController_->GetFocusWindowInfo(abilityToken);

    sptr<DisplayInfo> displayInfo = new DisplayInfo();
    sptr<WindowNodeContainer> container = new WindowNodeContainer(displayInfo, 0);
    windowRoot_->windowNodeContainerMap_.insert(std::make_pair(0, container));

    sptr<WindowNode> windowNode;
    ASSERT_EQ(WMError::WM_ERROR_INVALID_WINDOW, windowController_->GetFocusWindowNode(0, windowNode));

    windowRoot_->windowNodeMap_.clear();
    windowRoot_->windowNodeMap_.insert(std::make_pair(node_->GetWindowId(), node_));
    container->focusedWindow_ = node_->GetWindowId();
    node_->currentVisibility_ = false;
    ASSERT_EQ(WMError::WM_ERROR_INVALID_WINDOW, windowController_->GetFocusWindowNode(0, windowNode));

    node_->currentVisibility_ = true;
    ASSERT_EQ(WMError::WM_OK, windowController_->GetFocusWindowNode(0, windowNode));
    windowRoot_->windowNodeContainerMap_.clear();
}

/**
 * @tc.name: CreateWindow
 * @tc.desc: Window controller create window
 * @tc.type: FUNC
 */
HWTEST_F(WindowControllerTest, CreateWindow, TestSize.Level1)
{
    windowRoot_->windowNodeMap_.clear();
    sptr<IWindow> window;
    sptr<WindowProperty> property = new WindowProperty();
    std::shared_ptr<RSSurfaceNode> surfaceNode = nullptr;

    sptr<WindowProperty> property2 = new WindowProperty();
    property2->SetWindowType(WindowType::WINDOW_TYPE_APP_MAIN_WINDOW);
    sptr<WindowNode> windowNode = new WindowNode(property2);
    windowRoot_->windowNodeMap_.insert(std::make_pair(1,windowNode));
    sptr<WindowProperty> property3 = new WindowProperty();
    property3->SetWindowType(WindowType::BELOW_APP_SYSTEM_WINDOW_BASE);
    sptr<WindowNode> windowNode2 = new WindowNode(property3);
    windowRoot_->windowNodeMap_.insert(std::make_pair(2,windowNode2));

    uint32_t windowId;
    property->SetParentId(INVALID_WINDOW_ID);
    ASSERT_EQ(WMError::WM_ERROR_NULLPTR,
        windowController_->CreateWindow(window, property, surfaceNode, windowId, nullptr, 0, 0));
    struct RSSurfaceNodeConfig surfaceNodeConfig;
    surfaceNodeConfig.SurfaceNodeName = "SurfaceNode";
    surfaceNode = RSSurfaceNode::Create(surfaceNodeConfig, RSSurfaceNodeType::DEFAULT);
    ASSERT_EQ(WMError::WM_OK, windowController_->CreateWindow(window, property, surfaceNode, windowId, nullptr, 0, 0));

    property->SetParentId(1);
    property->SetWindowType(WindowType::WINDOW_TYPE_APP_MAIN_WINDOW);
    ASSERT_EQ(WMError::WM_ERROR_INVALID_PARENT,
        windowController_->CreateWindow(window, property, surfaceNode, windowId, nullptr, 0, 0));

    property->SetWindowType(WindowType::WINDOW_TYPE_APP_SUB_WINDOW);
    ASSERT_EQ(WMError::WM_OK, windowController_->CreateWindow(window, property, surfaceNode, windowId, nullptr, 0, 0));

    property->SetParentId(2);
    property->SetWindowType(WindowType::WINDOW_TYPE_SYSTEM_SUB_WINDOW);
    ASSERT_EQ(WMError::WM_OK, windowController_->CreateWindow(window, property, surfaceNode, windowId, nullptr, 0, 0));
    windowRoot_->windowNodeMap_.clear();

    sptr<IRemoteObject> abilityTokenMocker = new IRemoteObjectMocker();
    node_->abilityToken_ = abilityTokenMocker;

    property->SetParentId(INVALID_WINDOW_ID);
    property->SetWindowType(WindowType::WINDOW_TYPE_APP_MAIN_WINDOW);
    ASSERT_EQ(WMError::WM_OK,
        windowController_->CreateWindow(window, property, surfaceNode, windowId, abilityTokenMocker, 0, 0));

    property->SetWindowType(WindowType::WINDOW_TYPE_APP_SUB_WINDOW);
    ASSERT_EQ(WMError::WM_ERROR_INVALID_PARENT,
        windowController_->CreateWindow(window, property, surfaceNode, windowId, abilityTokenMocker, 0, 0));

    property->SetWindowType(WindowType::WINDOW_TYPE_APP_MAIN_WINDOW);
    node_->startingWindowShown_ = false;
    ASSERT_EQ(WMError::WM_OK,
        windowController_->CreateWindow(window, property, surfaceNode, windowId, abilityTokenMocker, 0, 0));
    windowRoot_->windowNodeMap_.clear();
}

/**
 * @tc.name: NotifyAfterAddWindow
 * @tc.desc: Window controller notify after add window
 * @tc.type: FUNC
 */
HWTEST_F(WindowControllerTest, NotifyAfterAddWindow, TestSize.Level1)
{
    ASSERT_NE(nullptr, windowController_);
    sptr<WindowNode> node0 = new WindowNode();
    windowController_->NotifyAfterAddWindow(node0);
    ASSERT_EQ(0, node0->children_.size());

    sptr<WindowNode> node1 = new WindowNode();
    node1->currentVisibility_ = false;
    sptr<WindowNode> node2= new WindowNode();
    node2->currentVisibility_ = true;

    node0->children_.push_back(node1);
    node0->children_.push_back(node2);
    windowController_->NotifyAfterAddWindow(node0);
    ASSERT_EQ(2, node0->children_.size());
    ASSERT_EQ(nullptr, node0->children_[0]->abilityToken_);
}

/**
 * @tc.name: AddWindowNode
 * @tc.desc: Window controller add window node
 * @tc.type: FUNC
 */
HWTEST_F(WindowControllerTest, AddWindowNode, TestSize.Level1)
{
    sptr<WindowProperty> property = new WindowProperty();
    property->SetWindowId(0);
    ASSERT_EQ(WMError::WM_ERROR_NULLPTR, windowController_->AddWindowNode(property));

    windowRoot_->windowNodeMap_.clear();
    windowRoot_->windowNodeMap_.insert(std::make_pair(node_->GetWindowId(), node_));
    property->SetWindowId(node_->GetWindowId());
    node_->currentVisibility_ = true;
    node_->startingWindowShown_ = false;
    ASSERT_EQ(WMError::WM_ERROR_INVALID_OPERATION, windowController_->AddWindowNode(property));

    node_->currentVisibility_ = false;
    ASSERT_EQ(WMError::WM_ERROR_INVALID_DISPLAY, windowController_->AddWindowNode(property));

    Rect requestRect{0, 0, 100, 100};
    property->SetRequestRect(requestRect);
    ASSERT_EQ(WMError::WM_ERROR_INVALID_DISPLAY, windowController_->AddWindowNode(property));

    node_->startingWindowShown_ = true;
    ASSERT_EQ(WMError::WM_ERROR_INVALID_DISPLAY, windowController_->AddWindowNode(property));

    windowRoot_->windowNodeMap_.clear();
}

/**
 * @tc.name: InputCallingWindow
 * @tc.desc: Window controller input calling window
 * @tc.type: FUNC
 */
HWTEST_F(WindowControllerTest, InputCallingWindow, TestSize.Level1)
{
    windowController_->callingWindowId_ = 0;
    windowRoot_->windowNodeMap_.clear();
    sptr<WindowNode> node = new WindowNode();
    node->property_->callingWindow_ = 0;
    node->property_->displayId_ = DISPLAY_ID_INVALID;
    windowController_->ResizeSoftInputCallingWindowIfNeed(node);
    ASSERT_EQ(0, windowController_->callingWindowId_);

    sptr<DisplayInfo> displayInfo = new DisplayInfo();
    sptr<WindowNodeContainer> container = new WindowNodeContainer(displayInfo, 0);
    windowRoot_->windowNodeContainerMap_.insert(std::make_pair(0, container));
    node->property_->displayId_ = 0;
    windowController_->ResizeSoftInputCallingWindowIfNeed(node);
    ASSERT_EQ(0, windowController_->callingWindowId_);

    windowRoot_->windowNodeMap_.insert(std::make_pair(node_->GetWindowId(), node_));
    container->focusedWindow_ = node_->GetWindowId();
    node_->SetWindowMode(WindowMode::WINDOW_MODE_FLOATING);
    node_->currentVisibility_ = false;
    windowController_->ResizeSoftInputCallingWindowIfNeed(node);
    ASSERT_EQ(0, windowController_->callingWindowId_);

    node_->currentVisibility_ = true;
    node_->SetWindowMode(WindowMode::WINDOW_MODE_FULLSCREEN);
    windowController_->ResizeSoftInputCallingWindowIfNeed(node);

    node_->currentVisibility_ = true;
    node_->SetWindowMode(WindowMode::WINDOW_MODE_FLOATING);
    windowController_->ResizeSoftInputCallingWindowIfNeed(node);
    ASSERT_EQ(0, windowController_->callingWindowId_);

    windowController_->callingWindowId_ = node_->GetWindowId();
    windowController_->callingWindowRestoringRect_ = {0, 0, 0, 0};
    windowController_->RestoreCallingWindowSizeIfNeed();
    ASSERT_EQ(0, windowController_->callingWindowId_);

    windowController_->callingWindowRestoringRect_ = {0, 0, 1, 1};
    windowController_->callingWindowId_ = 0;
    windowController_->RestoreCallingWindowSizeIfNeed();

    windowController_->callingWindowId_ = node_->GetWindowId();
    windowController_->RestoreCallingWindowSizeIfNeed();
    ASSERT_EQ(0, windowController_->callingWindowId_);

    windowController_->callingWindowId_ = node_->GetWindowId();
    node_->SetWindowMode(WindowMode::WINDOW_MODE_FULLSCREEN);
    windowController_->RestoreCallingWindowSizeIfNeed();
    ASSERT_EQ(0, windowController_->callingWindowId_);

    windowRoot_->windowNodeMap_.clear();
    windowRoot_->windowNodeContainerMap_.clear();
}

/**
 * @tc.name: SetDefaultDisplayInfo
 * @tc.desc: Window controller set default display info
 * @tc.type: FUNC
 */
HWTEST_F(WindowControllerTest, SetDefaultDisplayInfo, TestSize.Level1)
{
    const int32_t displayWidth = 100;
    const int32_t displayHeight = 200;
    windowController_->defaultDisplayRect_ = { 0, 0, 0, 0 };

    sptr<DisplayInfo> displayInfo = nullptr;
    windowController_->SetDefaultDisplayInfo(0, displayInfo);
    ASSERT_EQ(0, windowController_->defaultDisplayRect_.width_);
    ASSERT_EQ(0, windowController_->defaultDisplayRect_.height_);

    displayInfo = new DisplayInfo();
    displayInfo->id_ = 1;
    displayInfo->width_ = displayWidth;
    displayInfo->height_ = displayHeight;

    windowController_->SetDefaultDisplayInfo(0, displayInfo);
    ASSERT_EQ(0, windowController_->defaultDisplayRect_.width_);
    ASSERT_EQ(0, windowController_->defaultDisplayRect_.height_);

    displayInfo->id_ = 0;
    windowController_->SetDefaultDisplayInfo(0, displayInfo);
    ASSERT_EQ(displayWidth, windowController_->defaultDisplayRect_.width_);
    ASSERT_EQ(displayHeight, windowController_->defaultDisplayRect_.height_);
}

/**
 * @tc.name: ProcessDisplayCompression
 * @tc.desc: Window controller process display compression
 * @tc.type: FUNC
 */
HWTEST_F(WindowControllerTest, ProcessDisplayCompression, TestSize.Level1)
{
    ASSERT_NE(nullptr, windowController_);
    DisplayId defaultDisplayId = 0;
    sptr<DisplayInfo> displayInfo = new DisplayInfo();
    displayInfo->id_ = 1;
    windowController_->ProcessDisplayCompression(defaultDisplayId, displayInfo);
    ASSERT_EQ(nullptr, windowController_->maskingSurfaceNode_);

    displayInfo->id_ = defaultDisplayId;
    displayInfo->waterfallDisplayCompressionStatus_ = false;
    windowController_->ProcessDisplayCompression(defaultDisplayId, displayInfo);
    ASSERT_EQ(nullptr, windowController_->maskingSurfaceNode_);

    displayInfo->waterfallDisplayCompressionStatus_ = true;
    windowController_->ProcessDisplayCompression(defaultDisplayId, displayInfo);
    ASSERT_NE(nullptr, windowController_->maskingSurfaceNode_);
}

/**
 * @tc.name: StopBootAnimationIfNeed
 * @tc.desc: Window controller stop boot animation if need
 * @tc.type: FUNC
 */
HWTEST_F(WindowControllerTest, StopBootAnimationIfNeed, TestSize.Level1)
{
    ASSERT_NE(nullptr, windowController_);

    sptr<WindowNode> node = nullptr;
    windowController_->isBootAnimationStopped_ = true;
    windowController_->StopBootAnimationIfNeed(node);
    ASSERT_EQ(true, windowController_->isBootAnimationStopped_);

    windowController_->isBootAnimationStopped_ = false;
    windowController_->StopBootAnimationIfNeed(node);
    ASSERT_EQ(false, windowController_->isBootAnimationStopped_);

    node = new WindowNode();
    node->SetDisplayId(DISPLAY_ID_INVALID + 1);
    windowController_->StopBootAnimationIfNeed(node);
    ASSERT_EQ(false, windowController_->isBootAnimationStopped_);

    node->SetDisplayId(DISPLAY_ID_INVALID);
    windowController_->StopBootAnimationIfNeed(node);
    ASSERT_EQ(false, windowController_->isBootAnimationStopped_);
}

/**
 * @tc.name: GetEmbedNodeId
 * @tc.desc: Window controller get embed node id
 * @tc.type: FUNC
 */
HWTEST_F(WindowControllerTest, GetEmbedNodeId, TestSize.Level1)
{
    std::vector<sptr<WindowNode>> windowNodes;
    sptr<WindowNode> node0 = nullptr;
    sptr<WindowNode> node1 = new WindowNode();
    node1->property_->windowId_ = 1;
    sptr<WindowNode> node2 = new WindowNode();
    node2->property_->windowId_ = 2;
    sptr<WindowNode> node3 = new WindowNode();
    node3->property_->windowId_ = 3;

    node1->property_->SetWindowType(WindowType::WINDOW_TYPE_APP_MAIN_WINDOW);
    ASSERT_EQ(0, windowController_->GetEmbedNodeId(windowNodes, node1));

    node1->property_->SetWindowType(WindowType::WINDOW_TYPE_APP_COMPONENT);
    ASSERT_EQ(0, windowController_->GetEmbedNodeId(windowNodes, node1));

    windowNodes.push_back(node0);
    windowNodes.push_back(node2);
    windowNodes.push_back(node1);
    windowNodes.push_back(node2);
    windowNodes.push_back(node3);

    node1->SetWindowRect({50, 50, 50, 50});
    node3->SetWindowRect({0, 0, 200, 200});
    ASSERT_EQ(node3->GetWindowId(), windowController_->GetEmbedNodeId(windowNodes, node1));
}

/**
 * @tc.name: BindDialogTarget
 * @tc.desc: Window controller bind dialog target
 * @tc.type: FUNC
 */
HWTEST_F(WindowControllerTest, BindDialogTarget, TestSize.Level1)
{
    windowRoot_->windowNodeMap_.clear();

    uint32_t id = 0;
    sptr<IRemoteObject> abilityTokenMocker = new IRemoteObjectMocker();
    ASSERT_EQ(WMError::WM_ERROR_NULLPTR, windowController_->BindDialogTarget(id, abilityTokenMocker));

    windowRoot_->windowNodeMap_.insert(std::make_pair(node_->GetWindowId(), node_));
    id = node_->GetWindowId();
    ASSERT_EQ(WMError::WM_OK, windowController_->BindDialogTarget(id, abilityTokenMocker));
    windowRoot_->windowNodeMap_.clear();
}

/**
 * @tc.name: RaiseToAppTop
 * @tc.desc: check app subwindow raise to top
 * @tc.type: FUNC
 */
HWTEST_F(WindowControllerTest, RaiseToAppTop, TestSize.Level1)
{
    windowRoot_->windowNodeMap_.clear();

    sptr<WindowNode> windowNode = new (std::nothrow)WindowNode();
    windowNode->property_->windowId_ = 100;
    windowNode->SetDisplayId(DISPLAY_ID_INVALID);

    uint32_t windowId = windowNode->GetWindowId();
    ASSERT_EQ(WMError::WM_DO_NOTHING, windowController_->RaiseToAppTop(windowId));

    windowRoot_->windowNodeMap_.insert(std::make_pair(windowNode->GetWindowId(), windowNode));
    ASSERT_EQ(WMError::WM_ERROR_INVALID_PARENT, windowController_->RaiseToAppTop(windowId));

    sptr<WindowNode> parentWindow = new (std::nothrow)WindowNode();
    parentWindow->property_->windowId_ = 90;
    parentWindow->SetDisplayId(DISPLAY_ID_INVALID);
    windowRoot_->windowNodeMap_.insert(std::make_pair(parentWindow->GetWindowId(), parentWindow));

    windowNode->parent_ = parentWindow;
    ASSERT_EQ(WMError::WM_DO_NOTHING, windowController_->RaiseToAppTop(windowId));

    windowRoot_->windowNodeMap_.clear();
}

/**
 * @tc.name: GetFocusWindowInfo
 * @tc.desc: Window controller focus window
 * @tc.type: FUNC
 */
HWTEST_F(WindowControllerTest, GetFocusWindowInfo, TestSize.Level1)
{
    sptr<DisplayInfo> displayInfo = new DisplayInfo();
    sptr<WindowNodeContainer> container = new WindowNodeContainer(displayInfo, 0);
    windowRoot_->windowNodeContainerMap_.insert(std::make_pair(0, container));

    FocusChangeInfo focusInfo;
    WMError res = windowController_->GetFocusWindowInfo(focusInfo);
    windowRoot_->windowNodeContainerMap_.clear();
    ASSERT_EQ(res, WMError::WM_ERROR_INVALID_WINDOW);
}

/**
 * @tc.name: CheckParentWindowValid
 * @tc.desc: Window controller CheckParentWindowValid
 * @tc.type: FUNC
 */
HWTEST_F(WindowControllerTest, CreateWindow01, TestSize.Level1)
{
    windowRoot_->windowNodeMap_.clear();
    sptr<IWindow> window;
    sptr<WindowProperty> property = new WindowProperty();
    std::shared_ptr<RSSurfaceNode> surfaceNode = nullptr;

    sptr<WindowProperty> property2 = new WindowProperty();
    property2->SetWindowType(WindowType::SYSTEM_SUB_WINDOW_BASE);
    sptr<WindowNode> windowNode = new WindowNode(property2);
    windowRoot_->windowNodeMap_.insert(std::make_pair(1, windowNode));
    sptr<WindowProperty> property3 = new WindowProperty();
    property3->SetWindowType(WindowType::SYSTEM_SUB_WINDOW_BASE);
    sptr<WindowNode> windowNode2 = new WindowNode(property3);
    windowRoot_->windowNodeMap_.insert(std::make_pair(2, windowNode2));

    uint32_t windowId;
    property->SetParentId(1);
    property->SetWindowType(WindowType::APP_SUB_WINDOW_BASE);
    struct RSSurfaceNodeConfig surfaceNodeConfig;
    surfaceNodeConfig.SurfaceNodeName = "CheckParentWindowValid";
    surfaceNode = RSSurfaceNode::Create(surfaceNodeConfig, RSSurfaceNodeType::DEFAULT);
    ASSERT_EQ(WMError::WM_OK,
        windowController_->CreateWindow(window, property, surfaceNode, windowId, nullptr, 0, 0));

    property2->SetParentId(INVALID_WINDOW_ID);
    ASSERT_EQ(WMError::WM_ERROR_INVALID_PARENT,
        windowController_->CreateWindow(window, property2, surfaceNode, windowId, nullptr, 0, 0));

    property3->SetParentId(1);
    ASSERT_EQ(WMError::WM_ERROR_INVALID_PARENT,
        windowController_->CreateWindow(window, property2, surfaceNode, windowId, nullptr, 0, 0));
}

/**
 * @tc.name: CheckMultiDialogWindows
 * @tc.desc: Window controller CheckParentWindowValid
 * @tc.type: FUNC
 */
HWTEST_F(WindowControllerTest, CreateWindow02, TestSize.Level1)
{
    windowRoot_->windowNodeMap_.clear();
    sptr<IWindow> window;
    sptr<WindowProperty> property = new WindowProperty();
    std::shared_ptr<RSSurfaceNode> surfaceNode = nullptr;

    uint32_t windowId;
    property->SetParentId(INVALID_WINDOW_ID);
    property->SetWindowType(WindowType::WINDOW_TYPE_DIALOG);
    struct RSSurfaceNodeConfig surfaceNodeConfig;
    surfaceNodeConfig.SurfaceNodeName = "CheckMultiDialogWindows";
    surfaceNode = RSSurfaceNode::Create(surfaceNodeConfig, RSSurfaceNodeType::DEFAULT);
    sptr<IRemoteObject> abilityTokenMocker = new IRemoteObjectMocker();
    node_->abilityToken_ = abilityTokenMocker;

    ASSERT_EQ(WMError::WM_OK,
        windowController_->CreateWindow(window, property, surfaceNode, windowId, abilityTokenMocker, 0, 0));
}

/**
 * @tc.name: CheckMultiDialogWindows
 * @tc.desc: Window controller CheckParentWindowValid
 * @tc.type: FUNC
 */
HWTEST_F(WindowControllerTest, CreateWindow03, TestSize.Level1)
{
    windowRoot_->windowNodeMap_.clear();
    sptr<IWindow> window;
    sptr<WindowProperty> property = new WindowProperty();
    std::shared_ptr<RSSurfaceNode> surfaceNode = nullptr;

    uint32_t windowId;
    property->SetParentId(INVALID_WINDOW_ID);
    property->SetWindowType(WindowType::APP_MAIN_WINDOW_BASE);
    struct RSSurfaceNodeConfig surfaceNodeConfig;
    surfaceNodeConfig.SurfaceNodeName = "CheckMultiDialogWindows1";
    surfaceNode = RSSurfaceNode::Create(surfaceNodeConfig, RSSurfaceNodeType::DEFAULT);
    sptr<IRemoteObject> abilityTokenMocker = new IRemoteObjectMocker();
    node_->abilityToken_ = abilityTokenMocker;
    node_->startingWindowShown_ = true;

    ASSERT_EQ(WMError::WM_OK,
        windowController_->CreateWindow(window, property, surfaceNode, windowId, abilityTokenMocker, 0, 0));
}

/**
 * @tc.name: RemoveWindowNode
 * @tc.desc: Window controller RemoveWindowNode
 * @tc.type: FUNC
 */
HWTEST_F(WindowControllerTest, RemoveWindowNode, TestSize.Level1)
{
    windowRoot_->windowNodeMap_.clear();
    sptr<IWindow> window;
    sptr<WindowProperty> property = new WindowProperty();
    std::shared_ptr<RSSurfaceNode> surfaceNode = nullptr;

    uint32_t windowId;
    property->SetParentId(INVALID_WINDOW_ID);
    property->SetWindowType(WindowType::WINDOW_TYPE_DIALOG);
    struct RSSurfaceNodeConfig surfaceNodeConfig;
    surfaceNodeConfig.SurfaceNodeName = "RemoveWindowNode";
    surfaceNode = RSSurfaceNode::Create(surfaceNodeConfig, RSSurfaceNodeType::DEFAULT);
    ASSERT_EQ(WMError::WM_OK, windowController_->CreateWindow(window, property, surfaceNode, windowId, nullptr, 0, 0));

    WMError res = windowController_->RemoveWindowNode(windowId, false);
    ASSERT_EQ(WMError::WM_ERROR_INVALID_DISPLAY, res);
}

/**
 * @tc.name: RemoveWindowNode
 * @tc.desc: Window controller RemoveWindowNode
 * @tc.type: FUNC
 */
HWTEST_F(WindowControllerTest, RemoveWindowNode1, TestSize.Level1)
{
    windowRoot_->windowNodeMap_.clear();
    sptr<IWindow> window;
    sptr<WindowProperty> property = new WindowProperty();
    std::shared_ptr<RSSurfaceNode> surfaceNode = nullptr;

    uint32_t windowId;
    property->SetParentId(INVALID_WINDOW_ID);
    property->SetWindowType(WindowType::WINDOW_TYPE_DIALOG);
    struct RSSurfaceNodeConfig surfaceNodeConfig;
    surfaceNodeConfig.SurfaceNodeName = "RemoveWindowNode1";
    surfaceNode = RSSurfaceNode::Create(surfaceNodeConfig, RSSurfaceNodeType::DEFAULT);
    ASSERT_EQ(WMError::WM_OK, windowController_->CreateWindow(window, property, surfaceNode, windowId, nullptr, 0, 0));

    WMError res = windowController_->RemoveWindowNode(windowId, true);
    ASSERT_EQ(WMError::WM_ERROR_INVALID_DISPLAY, res);
}

/**
 * @tc.name: RemoveWindowNode
 * @tc.desc: Window controller RemoveWindowNode
 * @tc.type: FUNC
 */
HWTEST_F(WindowControllerTest, RemoveWindowNode2, TestSize.Level1)
{
    windowRoot_->windowNodeMap_.clear();
    sptr<IWindow> window;
    sptr<WindowProperty> property = new WindowProperty();
    std::shared_ptr<RSSurfaceNode> surfaceNode = nullptr;

    uint32_t windowId;
    property->SetParentId(INVALID_WINDOW_ID);
    property->SetWindowType(WindowType::WINDOW_TYPE_KEYGUARD);
    struct RSSurfaceNodeConfig surfaceNodeConfig;
    surfaceNodeConfig.SurfaceNodeName = "RemoveWindowNode2";
    surfaceNode = RSSurfaceNode::Create(surfaceNodeConfig, RSSurfaceNodeType::DEFAULT);
    ASSERT_EQ(WMError::WM_OK, windowController_->CreateWindow(window, property, surfaceNode, windowId, nullptr, 0, 0));

    WMError res = windowController_->RemoveWindowNode(windowId, true);
    ASSERT_EQ(WMError::WM_ERROR_INVALID_DISPLAY, res);
}

/**
 * @tc.name: DestroyWindow
 * @tc.desc: Window controller DestroyWindow true
 * @tc.type: FUNC
 */
HWTEST_F(WindowControllerTest, DestroyWindow, TestSize.Level1)
{
    windowRoot_->windowNodeMap_.clear();
    sptr<IWindow> window;
    sptr<WindowProperty> property = new WindowProperty();
    std::shared_ptr<RSSurfaceNode> surfaceNode = nullptr;

    uint32_t windowId;
    property->SetParentId(INVALID_WINDOW_ID);
    property->SetWindowType(WindowType::WINDOW_TYPE_DIALOG);
    struct RSSurfaceNodeConfig surfaceNodeConfig;
    surfaceNodeConfig.SurfaceNodeName = "DestroyWindow";
    surfaceNode = RSSurfaceNode::Create(surfaceNodeConfig, RSSurfaceNodeType::DEFAULT);
    ASSERT_EQ(WMError::WM_OK, windowController_->CreateWindow(window, property, surfaceNode, windowId, nullptr, 0, 0));

    WMError res = windowController_->DestroyWindow(100, true);
    ASSERT_EQ(WMError::WM_ERROR_NULLPTR, res);

    res = windowController_->DestroyWindow(windowId, true);
    ASSERT_EQ(WMError::WM_OK, res);
}

/**
 * @tc.name: DestroyWindow1
 * @tc.desc: Window controller DestroyWindow false
 * @tc.type: FUNC
 */
HWTEST_F(WindowControllerTest, DestroyWindow1, TestSize.Level1)
{
    windowRoot_->windowNodeMap_.clear();
    sptr<IWindow> window;
    sptr<WindowProperty> property = new WindowProperty();
    std::shared_ptr<RSSurfaceNode> surfaceNode = nullptr;

    uint32_t windowId;
    property->SetParentId(INVALID_WINDOW_ID);
    property->SetWindowType(WindowType::WINDOW_TYPE_DIALOG);
    struct RSSurfaceNodeConfig surfaceNodeConfig;
    surfaceNodeConfig.SurfaceNodeName = "DestroyWindow1";
    surfaceNode = RSSurfaceNode::Create(surfaceNodeConfig, RSSurfaceNodeType::DEFAULT);
    ASSERT_EQ(WMError::WM_OK, windowController_->CreateWindow(window, property, surfaceNode, windowId, nullptr, 0, 0));

    WMError res = windowController_->DestroyWindow(100, false);
    ASSERT_EQ(WMError::WM_ERROR_NULLPTR, res);

    res = windowController_->DestroyWindow(windowId, false);
    ASSERT_EQ(WMError::WM_OK, res);
}

/**
 * @tc.name: RequestFocus
 * @tc.desc: Window controller RequestFocus false
 * @tc.type: FUNC
 */
HWTEST_F(WindowControllerTest, RequestFocus, TestSize.Level1)
{
    windowRoot_->windowNodeMap_.clear();
    sptr<IWindow> window;
    sptr<WindowProperty> property = new WindowProperty();
    std::shared_ptr<RSSurfaceNode> surfaceNode = nullptr;
    sptr<WindowNode> windowNode = new WindowNode(property);
    windowRoot_->windowNodeMap_.insert(std::make_pair(0, windowNode));

    uint32_t windowId;
    property->SetParentId(INVALID_WINDOW_ID);
    property->SetWindowType(WindowType::WINDOW_TYPE_DIALOG);
    struct RSSurfaceNodeConfig surfaceNodeConfig;
    surfaceNodeConfig.SurfaceNodeName = "RequestFocus";
    surfaceNode = RSSurfaceNode::Create(surfaceNodeConfig, RSSurfaceNodeType::DEFAULT);
    ASSERT_EQ(WMError::WM_OK, windowController_->CreateWindow(window, property, surfaceNode, windowId, nullptr, 0, 0));

    WMError res = windowController_->RequestFocus(10);
    ASSERT_EQ(WMError::WM_ERROR_NULLPTR, res);

    windowId = windowNode->GetWindowId();
    res = windowController_->RequestFocus(windowId);
    if (!SceneBoardJudgement::IsSceneBoardEnabled()) {
        ASSERT_NE(WMError::WM_ERROR_INVALID_OPERATION, res);
    } else {
        ASSERT_EQ(WMError::WM_ERROR_INVALID_OPERATION, res);
    }
}

/**
 * @tc.name: NotifyDisplayStateChange
 * @tc.desc: Window controller NotifyDisplayStateChange
 * @tc.type: FUNC
 */
HWTEST_F(WindowControllerTest, NotifyDisplayStateChange, TestSize.Level1)
{
    windowRoot_->windowNodeMap_.clear();
    sptr<IWindow> window;
    sptr<WindowProperty> property = new WindowProperty();
    std::shared_ptr<RSSurfaceNode> surfaceNode = nullptr;
    sptr<WindowNode> windowNode = new WindowNode(property);
    windowRoot_->windowNodeMap_.insert(std::make_pair(1, windowNode));

    uint32_t windowId;
    property->SetParentId(INVALID_WINDOW_ID);
    property->SetWindowType(WindowType::WINDOW_TYPE_DIALOG);
    struct RSSurfaceNodeConfig surfaceNodeConfig;
    surfaceNodeConfig.SurfaceNodeName = "NotifyDisplayStateChange";
    surfaceNode = RSSurfaceNode::Create(surfaceNodeConfig, RSSurfaceNodeType::DEFAULT);

    DisplayId defaultDisplayId = 0;
    sptr<DisplayInfo> displayInfo = new DisplayInfo();
    std::map < DisplayId, sptr < DisplayInfo >> displayInfoMap;

    DisplayStateChangeType type = DisplayStateChangeType::BEFORE_SUSPEND;
    windowController_->NotifyDisplayStateChange(defaultDisplayId, displayInfo, displayInfoMap, type);
    type = DisplayStateChangeType::BEFORE_UNLOCK;
    windowController_->NotifyDisplayStateChange(defaultDisplayId, displayInfo, displayInfoMap, type);
    type = DisplayStateChangeType::CREATE;
    windowController_->NotifyDisplayStateChange(defaultDisplayId, displayInfo, displayInfoMap, type);
    type = DisplayStateChangeType::DESTROY;
    windowController_->NotifyDisplayStateChange(defaultDisplayId, displayInfo, displayInfoMap, type);
    type = DisplayStateChangeType::VIRTUAL_PIXEL_RATIO_CHANGE;
    windowController_->NotifyDisplayStateChange(defaultDisplayId, displayInfo, displayInfoMap, type);
    type = DisplayStateChangeType::UNKNOWN;
    windowController_->NotifyDisplayStateChange(defaultDisplayId, displayInfo, displayInfoMap, type);

    ASSERT_EQ(WMError::WM_OK,
        windowController_->CreateWindow(window, property, surfaceNode, windowId, nullptr, 0, 0));
}

/**
 * @tc.name: NotifyDisplayStateChange
 * @tc.desc: Window controller NotifyDisplayStateChange
 * @tc.type: FUNC
 */
HWTEST_F(WindowControllerTest, NotifyDisplayStateChange1, TestSize.Level1)
{
    windowRoot_->windowNodeMap_.clear();
    sptr<IWindow> window;
    sptr<WindowProperty> property = new WindowProperty();
    std::shared_ptr<RSSurfaceNode> surfaceNode = nullptr;
    sptr<WindowNode> windowNode = new WindowNode(property);
    windowRoot_->windowNodeMap_.insert(std::make_pair(1, windowNode));

    uint32_t windowId;
    property->SetParentId(INVALID_WINDOW_ID);
    property->SetWindowType(WindowType::WINDOW_TYPE_DIALOG);
    struct RSSurfaceNodeConfig surfaceNodeConfig;
    surfaceNodeConfig.SurfaceNodeName = "NotifyDisplayStateChange1";
    surfaceNode = RSSurfaceNode::Create(surfaceNodeConfig, RSSurfaceNodeType::DEFAULT);

    DisplayId defaultDisplayId = 0;
    sptr<DisplayInfo> displayInfo = new DisplayInfo();
    std::map < DisplayId, sptr < DisplayInfo >> displayInfoMap;

    DisplayStateChangeType type = DisplayStateChangeType::VIRTUAL_PIXEL_RATIO_CHANGE;
    windowController_->NotifyDisplayStateChange(defaultDisplayId, displayInfo, displayInfoMap, type);

    ASSERT_EQ(WMError::WM_OK,
        windowController_->CreateWindow(window, property, surfaceNode, windowId, nullptr, 0, 0));
}

/**
 * @tc.name: ProcessDisplayChange
 * @tc.desc: Window controller ProcessDisplayChange
 * @tc.type: FUNC
 */
HWTEST_F(WindowControllerTest, ProcessDisplayChange, TestSize.Level1)
{
    windowRoot_->windowNodeMap_.clear();
    sptr<IWindow> window;
    sptr<WindowProperty> property = new WindowProperty();
    std::shared_ptr<RSSurfaceNode> surfaceNode = nullptr;
    sptr<WindowNode> windowNode = new WindowNode(property);
    windowRoot_->windowNodeMap_.insert(std::make_pair(1, windowNode));

    uint32_t windowId;
    property->SetParentId(INVALID_WINDOW_ID);
    property->SetWindowType(WindowType::WINDOW_TYPE_DIALOG);
    struct RSSurfaceNodeConfig surfaceNodeConfig;
    surfaceNodeConfig.SurfaceNodeName = "ProcessDisplayChange";
    surfaceNode = RSSurfaceNode::Create(surfaceNodeConfig, RSSurfaceNodeType::DEFAULT);

    DisplayId defaultDisplayId = 0;
    sptr<DisplayInfo> displayInfo = new DisplayInfo();
    std::map<DisplayId, sptr<DisplayInfo>> displayInfoMap;
    DisplayStateChangeType type = DisplayStateChangeType::VIRTUAL_PIXEL_RATIO_CHANGE;
    windowController_->NotifyDisplayStateChange(defaultDisplayId, displayInfo, displayInfoMap, type);

    displayInfo->SetDisplayId(defaultDisplayId);
    windowController_->NotifyDisplayStateChange(defaultDisplayId, displayInfo, displayInfoMap, type);

    sptr<DisplayInfo> displayInfo1 = nullptr;
    windowController_->NotifyDisplayStateChange(defaultDisplayId, displayInfo1, displayInfoMap, type);

    ASSERT_EQ(WMError::WM_OK,
        windowController_->CreateWindow(window, property, surfaceNode, windowId, nullptr, 0, 0));
}

/**
 * @tc.name: ChangeMouseStyle
 * @tc.desc: Window controller ChangeMouseStyle width > height
 * @tc.type: FUNC
 */
HWTEST_F(WindowControllerTest, ChangeMouseStyle1, TestSize.Level1)
{
    windowRoot_->windowNodeMap_.clear();
    sptr<IWindow> window;
    sptr<WindowProperty> property = new WindowProperty();
    std::shared_ptr<RSSurfaceNode> surfaceNode = nullptr;
    property->SetParentId(INVALID_WINDOW_ID);
    property->SetWindowType(WindowType::WINDOW_TYPE_DOCK_SLICE);
    sptr<WindowNode> windowNode = new WindowNode(property);
    windowNode->SetWindowRect({50, 50, 100, 50});
    windowRoot_->windowNodeMap_.insert(std::make_pair(1, windowNode));

    uint32_t windowId = windowNode->GetWindowId();
    struct RSSurfaceNodeConfig surfaceNodeConfig;
    surfaceNodeConfig.SurfaceNodeName = "ChangeMouseStyle1";
    surfaceNode = RSSurfaceNode::Create(surfaceNodeConfig, RSSurfaceNodeType::DEFAULT);
    ASSERT_EQ(WMError::WM_OK,
        windowController_->CreateWindow(window, property, surfaceNode, windowId, nullptr, 0, 0));

    sptr<MoveDragProperty> moveDragProperty;
    WMError res = windowController_->ChangeMouseStyle(windowId, moveDragProperty);
    if (SceneBoardJudgement::IsSceneBoardEnabled()) {
        ASSERT_EQ(WMError::WM_ERROR_INVALID_OPERATION, res);
    } else {
        ASSERT_EQ(WMError::WM_OK, res);
    }
}

/**
 * @tc.name: ChangeMouseStyle
 * @tc.desc: Window controller ChangeMouseStyle width < height
 * @tc.type: FUNC
 */
HWTEST_F(WindowControllerTest, ChangeMouseStyle2, TestSize.Level1)
{
    windowRoot_->windowNodeMap_.clear();
    sptr<IWindow> window;
    sptr<WindowProperty> property = new WindowProperty();
    std::shared_ptr<RSSurfaceNode> surfaceNode = nullptr;
    property->SetParentId(INVALID_WINDOW_ID);
    property->SetWindowType(WindowType::WINDOW_TYPE_DOCK_SLICE);
    sptr<WindowNode> windowNode = new WindowNode(property);
    windowNode->SetWindowRect({50, 50, 20, 50});
    windowRoot_->windowNodeMap_.insert(std::make_pair(1, windowNode));

    uint32_t windowId = windowNode->GetWindowId();
    struct RSSurfaceNodeConfig surfaceNodeConfig;
    surfaceNodeConfig.SurfaceNodeName = "ChangeMouseStyle2";
    surfaceNode = RSSurfaceNode::Create(surfaceNodeConfig, RSSurfaceNodeType::DEFAULT);
    ASSERT_EQ(WMError::WM_OK,
        windowController_->CreateWindow(window, property, surfaceNode, windowId, nullptr, 0, 0));

    sptr<MoveDragProperty> moveDragProperty;
    WMError res = windowController_->ChangeMouseStyle(windowId, moveDragProperty);
    if (SceneBoardJudgement::IsSceneBoardEnabled()) {
        ASSERT_EQ(WMError::WM_ERROR_INVALID_OPERATION, res);
    } else {
        ASSERT_EQ(WMError::WM_OK, res);
    }
}

/**
 * @tc.name: ChangeMouseStyle
 * @tc.desc: Window controller ChangeMouseStyle
 * @tc.type: FUNC
 */
HWTEST_F(WindowControllerTest, ChangeMouseStyle3, TestSize.Level1)
{
    windowRoot_->windowNodeMap_.clear();
    sptr<IWindow> window;
    sptr<WindowProperty> property = new WindowProperty();
    std::shared_ptr<RSSurfaceNode> surfaceNode = nullptr;
    sptr<WindowNode> windowNode = new WindowNode(property);
    property->SetParentId(INVALID_WINDOW_ID);
    property->SetWindowType(WindowType::APP_WINDOW_BASE);
    windowNode->SetWindowRect({50, 50, 50, 50});
    windowRoot_->windowNodeMap_.insert(std::make_pair(1, windowNode));

    uint32_t windowId = windowNode->GetWindowId();
    struct RSSurfaceNodeConfig surfaceNodeConfig;
    surfaceNodeConfig.SurfaceNodeName = "ChangeMouseStyle3";
    surfaceNode = RSSurfaceNode::Create(surfaceNodeConfig, RSSurfaceNodeType::DEFAULT);
    ASSERT_EQ(WMError::WM_OK,
        windowController_->CreateWindow(window, property, surfaceNode, windowId, nullptr, 0, 0));

    sptr<MoveDragProperty> moveDragProperty = new MoveDragProperty();
    moveDragProperty->dragType_ = DragType::DRAG_UNDEFINED;
    WMError res = windowController_->ChangeMouseStyle(windowId, moveDragProperty);
    if (!SceneBoardJudgement::IsSceneBoardEnabled()) {
        ASSERT_EQ(WMError::WM_ERROR_INVALID_OPERATION, res);
    } else {
        ASSERT_EQ(WMError::WM_ERROR_INVALID_OPERATION, res);
    }
}

/**
 * @tc.name: NotifyServerReadyToMoveOrDrag
 * @tc.desc: Window controller NotifyServerReadyToMoveOrDrag1
 * @tc.type: FUNC
 */
HWTEST_F(WindowControllerTest, NotifyServerReadyToMoveOrDrag1, TestSize.Level1)
{
    windowRoot_->windowNodeMap_.clear();
    sptr<IWindow> window;
    sptr<WindowProperty> property = new WindowProperty();
    property->SetMaximizeMode(MaximizeMode::MODE_AVOID_SYSTEM_BAR);
    std::shared_ptr<RSSurfaceNode> surfaceNode = nullptr;
    sptr<WindowNode> windowNode = new WindowNode(property);
    property->SetParentId(INVALID_WINDOW_ID);
    property->SetWindowType(WindowType::APP_WINDOW_BASE);
    windowRoot_->windowNodeMap_.insert(std::make_pair(1, windowNode));

    uint32_t windowId = windowNode->GetWindowId();
    struct RSSurfaceNodeConfig surfaceNodeConfig;
    surfaceNodeConfig.SurfaceNodeName = "NotifyServerReadyToMoveOrDrag1";
    surfaceNode = RSSurfaceNode::Create(surfaceNodeConfig, RSSurfaceNodeType::DEFAULT);
    ASSERT_EQ(WMError::WM_OK,
        windowController_->CreateWindow(window, property, surfaceNode, windowId, nullptr, 0, 0));

    sptr<MoveDragProperty> moveDragProperty = new MoveDragProperty();
    WMError res = windowController_->NotifyServerReadyToMoveOrDrag(10, moveDragProperty);
    ASSERT_EQ(WMError::WM_ERROR_NULLPTR, res);

    ASSERT_EQ(windowNode->currentVisibility_, false);
    res = windowController_->NotifyServerReadyToMoveOrDrag(windowId, moveDragProperty);
    ASSERT_EQ(WMError::WM_ERROR_INVALID_OPERATION, res);

    windowNode->currentVisibility_ = true;
    res = windowController_->NotifyServerReadyToMoveOrDrag(windowId, moveDragProperty);
    ASSERT_EQ(WMError::WM_ERROR_INVALID_OPERATION, res);
}

/**
 * @tc.name: NotifyServerReadyToMoveOrDrag
 * @tc.desc: Window controller NotifyServerReadyToMoveOrDrag2
 * @tc.type: FUNC
 */
HWTEST_F(WindowControllerTest, NotifyServerReadyToMoveOrDrag2, TestSize.Level1)
{
    windowRoot_->windowNodeMap_.clear();
    sptr<IWindow> window;
    sptr<WindowProperty> property = new WindowProperty();
    std::shared_ptr<RSSurfaceNode> surfaceNode = nullptr;
    sptr<WindowNode> windowNode = new WindowNode(property);
    property->SetParentId(INVALID_WINDOW_ID);
    property->SetWindowType(WindowType::APP_WINDOW_BASE);
    windowNode->currentVisibility_ = true;
    windowRoot_->windowNodeMap_.insert(std::make_pair(1, windowNode));

    uint32_t windowId = windowNode->GetWindowId();
    struct RSSurfaceNodeConfig surfaceNodeConfig;
    surfaceNodeConfig.SurfaceNodeName = "NotifyServerReadyToMoveOrDrag2";
    surfaceNode = RSSurfaceNode::Create(surfaceNodeConfig, RSSurfaceNodeType::DEFAULT);
    ASSERT_EQ(WMError::WM_OK,
        windowController_->CreateWindow(window, property, surfaceNode, windowId, nullptr, 0, 0));

    sptr<MoveDragProperty> moveDragProperty = new MoveDragProperty();
    moveDragProperty->startMoveFlag_ = false;
    moveDragProperty->startDragFlag_ = false;
    WMError res = windowController_->NotifyServerReadyToMoveOrDrag(windowId, moveDragProperty);
    ASSERT_EQ(WMError::WM_ERROR_INVALID_OPERATION, res);

    moveDragProperty->startMoveFlag_ = true;
    res = windowController_->NotifyServerReadyToMoveOrDrag(windowId, moveDragProperty);
    ASSERT_EQ(WMError::WM_ERROR_INVALID_OPERATION, res);

    moveDragProperty->startMoveFlag_ = false;
    moveDragProperty->startDragFlag_ = true;
    res = windowController_->NotifyServerReadyToMoveOrDrag(windowId, moveDragProperty);
    ASSERT_EQ(WMError::WM_ERROR_INVALID_OPERATION, res);

    moveDragProperty->startMoveFlag_ = true;
    res = windowController_->NotifyServerReadyToMoveOrDrag(windowId, moveDragProperty);
    ASSERT_EQ(WMError::WM_ERROR_INVALID_OPERATION, res);
}

/**
 * @tc.name: NotifyServerReadyToMoveOrDrag
 * @tc.desc: Window controller NotifyServerReadyToMoveOrDrag WindowType = WINDOW_TYPE_DOCK_SLICE
 * @tc.type: FUNC
 */
HWTEST_F(WindowControllerTest, NotifyServerReadyToMoveOrDrag3, TestSize.Level1)
{
    windowRoot_->windowNodeMap_.clear();
    sptr<IWindow> window;
    sptr<WindowProperty> property = new WindowProperty();
    std::shared_ptr<RSSurfaceNode> surfaceNode = nullptr;
    sptr<WindowNode> windowNode = new WindowNode(property);
    property->SetParentId(INVALID_WINDOW_ID);
    property->SetWindowType(WindowType::WINDOW_TYPE_DOCK_SLICE);
    windowNode->currentVisibility_ = true;
    windowRoot_->windowNodeMap_.insert(std::make_pair(1, windowNode));

    uint32_t windowId = windowNode->GetWindowId();
    struct RSSurfaceNodeConfig surfaceNodeConfig;
    surfaceNodeConfig.SurfaceNodeName = "NotifyServerReadyToMoveOrDrag3";
    surfaceNode = RSSurfaceNode::Create(surfaceNodeConfig, RSSurfaceNodeType::DEFAULT);
    ASSERT_EQ(WMError::WM_OK,
        windowController_->CreateWindow(window, property, surfaceNode, windowId, nullptr, 0, 0));

    sptr<MoveDragProperty> moveDragProperty = new MoveDragProperty();
    moveDragProperty->startMoveFlag_ = false;
    moveDragProperty->startDragFlag_ = false;
    WMError res = windowController_->NotifyServerReadyToMoveOrDrag(windowId, moveDragProperty);
    ASSERT_EQ(WMError::WM_ERROR_INVALID_OPERATION, res);

    moveDragProperty->startMoveFlag_ = true;
    res = windowController_->NotifyServerReadyToMoveOrDrag(windowId, moveDragProperty);
    ASSERT_EQ(WMError::WM_ERROR_INVALID_OPERATION, res);

    moveDragProperty->startMoveFlag_ = false;
    moveDragProperty->startDragFlag_ = true;
    res = windowController_->NotifyServerReadyToMoveOrDrag(windowId, moveDragProperty);
    ASSERT_EQ(WMError::WM_ERROR_INVALID_OPERATION, res);

    moveDragProperty->startMoveFlag_ = true;
    res = windowController_->NotifyServerReadyToMoveOrDrag(windowId, moveDragProperty);
    ASSERT_EQ(WMError::WM_ERROR_INVALID_OPERATION, res);
}

/**
 * @tc.name: ProcessPointDown
 * @tc.desc: Window controller ProcessPointDown
 * @tc.type: FUNC
 */
HWTEST_F(WindowControllerTest, ProcessPointDown1, TestSize.Level1)
{
    windowRoot_->windowNodeMap_.clear();
    sptr<IWindow> window;
    sptr<WindowProperty> property = new WindowProperty();
    std::shared_ptr<RSSurfaceNode> surfaceNode = nullptr;
    sptr<WindowNode> windowNode = new WindowNode(property);
    property->SetParentId(INVALID_WINDOW_ID);
    property->SetWindowType(WindowType::APP_WINDOW_BASE);
    windowRoot_->windowNodeMap_.insert(std::make_pair(1, windowNode));

    uint32_t windowId = windowNode->GetWindowId();
    struct RSSurfaceNodeConfig surfaceNodeConfig;
    surfaceNodeConfig.SurfaceNodeName = "ProcessPointDown1";
    surfaceNode = RSSurfaceNode::Create(surfaceNodeConfig, RSSurfaceNodeType::DEFAULT);
    ASSERT_EQ(WMError::WM_OK,
        windowController_->CreateWindow(window, property, surfaceNode, windowId, nullptr, 0, 0));

    bool isPointDown = true;
    WMError res = windowController_->ProcessPointDown(10, isPointDown);
    ASSERT_EQ(WMError::WM_ERROR_NULLPTR, res);

    ASSERT_EQ(windowNode->currentVisibility_, false);
    res = windowController_->ProcessPointDown(windowId, isPointDown);
    ASSERT_EQ(WMError::WM_ERROR_INVALID_OPERATION, res);

    windowNode->currentVisibility_ = true;
    res = windowController_->ProcessPointDown(windowId, isPointDown);
    ASSERT_EQ(WMError::WM_ERROR_INVALID_OPERATION, res);
}

/**
 * @tc.name: ProcessPointDown
 * @tc.desc: Window controller ProcessPointDown
 * @tc.type: FUNC
 */
HWTEST_F(WindowControllerTest, ProcessPointDown2, TestSize.Level1)
{
    windowRoot_->windowNodeMap_.clear();
    sptr<IWindow> window;
    sptr<WindowProperty> property = new WindowProperty();
    std::shared_ptr<RSSurfaceNode> surfaceNode = nullptr;
    sptr<WindowNode> windowNode = new WindowNode(property);
    property->SetParentId(INVALID_WINDOW_ID);
    property->SetWindowType(WindowType::WINDOW_TYPE_DOCK_SLICE);
    windowRoot_->windowNodeMap_.insert(std::make_pair(1, windowNode));

    uint32_t windowId = windowNode->GetWindowId();
    struct RSSurfaceNodeConfig surfaceNodeConfig;
    surfaceNodeConfig.SurfaceNodeName = "ProcessPointDown2";
    surfaceNode = RSSurfaceNode::Create(surfaceNodeConfig, RSSurfaceNodeType::DEFAULT);
    ASSERT_EQ(WMError::WM_OK,
        windowController_->CreateWindow(window, property, surfaceNode, windowId, nullptr, 0, 0));

    bool isPointDown = true;
    windowNode->currentVisibility_ = true;
    WMError res = windowController_->ProcessPointDown(windowId, isPointDown);
    ASSERT_EQ(WMError::WM_ERROR_INVALID_OPERATION, res);

    isPointDown = false;
    res = windowController_->ProcessPointDown(windowId, isPointDown);
    ASSERT_EQ(WMError::WM_ERROR_INVALID_OPERATION, res);
}

/**
 * @tc.name: ProcessPointUp
 * @tc.desc: Window controller ProcessPointUp WindowType = WINDOW_TYPE_DOCK_SLICE
 * @tc.type: FUNC
 */
HWTEST_F(WindowControllerTest, ProcessPointUp, TestSize.Level1)
{
    windowRoot_->windowNodeMap_.clear();
    sptr<IWindow> window;
    sptr<WindowProperty> property = new WindowProperty();
    std::shared_ptr<RSSurfaceNode> surfaceNode = nullptr;
    sptr<WindowNode> windowNode = new WindowNode(property);
    property->SetParentId(INVALID_WINDOW_ID);
    property->SetWindowType(WindowType::WINDOW_TYPE_DOCK_SLICE);
    windowRoot_->windowNodeMap_.insert(std::make_pair(1, windowNode));

    uint32_t windowId = windowNode->GetWindowId();
    struct RSSurfaceNodeConfig surfaceNodeConfig;
    surfaceNodeConfig.SurfaceNodeName = "ProcessPointUp";
    surfaceNode = RSSurfaceNode::Create(surfaceNodeConfig, RSSurfaceNodeType::DEFAULT);
    ASSERT_EQ(WMError::WM_OK,
        windowController_->CreateWindow(window, property, surfaceNode, windowId, nullptr, 0, 0));

    WMError res = windowController_->ProcessPointUp(10);
    ASSERT_EQ(WMError::WM_ERROR_NULLPTR, res);

    res = windowController_->ProcessPointUp(windowId);
    ASSERT_EQ(WMError::WM_ERROR_INVALID_DISPLAY, res);
}

/**
 * @tc.name: ProcessPointUp2
 * @tc.desc: Window controller ProcessPointUp2 WindowType = WINDOW_TYPE_APP_MAIN_WINDOW
 * @tc.type: FUNC
 */
HWTEST_F(WindowControllerTest, ProcessPointUp2, TestSize.Level1)
{
    windowRoot_->windowNodeMap_.clear();
    sptr<IWindow> window;
    sptr<WindowProperty> property = new WindowProperty();
    std::shared_ptr<RSSurfaceNode> surfaceNode = nullptr;
    sptr<WindowNode> windowNode = new WindowNode(property);
    property->SetParentId(INVALID_WINDOW_ID);
    property->SetWindowType(WindowType::WINDOW_TYPE_APP_MAIN_WINDOW);
    windowRoot_->windowNodeMap_.insert(std::make_pair(1, windowNode));

    uint32_t windowId = windowNode->GetWindowId();
    struct RSSurfaceNodeConfig surfaceNodeConfig;
    surfaceNodeConfig.SurfaceNodeName = "ProcessPointUp2";
    surfaceNode = RSSurfaceNode::Create(surfaceNodeConfig, RSSurfaceNodeType::DEFAULT);
    ASSERT_EQ(WMError::WM_OK,
        windowController_->CreateWindow(window, property, surfaceNode, windowId, nullptr, 0, 0));

    WMError res = windowController_->ProcessPointUp(windowId);
    ASSERT_EQ(WMError::WM_ERROR_INVALID_DISPLAY, res);
}

/**
 * @tc.name: ProcessPointUp3
 * @tc.desc: Window controller ProcessPointUp3 WindowType = APP_WINDOW_BASE
 * @tc.type: FUNC
 */
HWTEST_F(WindowControllerTest, ProcessPointUp3, TestSize.Level1)
{
    windowRoot_->windowNodeMap_.clear();
    sptr<IWindow> window;
    sptr<WindowProperty> property = new WindowProperty();
    std::shared_ptr<RSSurfaceNode> surfaceNode = nullptr;
    sptr<WindowNode> windowNode = new WindowNode(property);
    property->SetParentId(INVALID_WINDOW_ID);
    property->SetWindowType(WindowType::APP_WINDOW_BASE);
    windowRoot_->windowNodeMap_.insert(std::make_pair(1, windowNode));

    uint32_t windowId = windowNode->GetWindowId();
    struct RSSurfaceNodeConfig surfaceNodeConfig;
    surfaceNodeConfig.SurfaceNodeName = "ProcessPointUp3";
    surfaceNode = RSSurfaceNode::Create(surfaceNodeConfig, RSSurfaceNodeType::DEFAULT);
    ASSERT_EQ(WMError::WM_OK,
        windowController_->CreateWindow(window, property, surfaceNode, windowId, nullptr, 0, 0));

    WMError res = windowController_->ProcessPointUp(windowId);
    ASSERT_EQ(WMError::WM_ERROR_INVALID_DISPLAY, res);
}

/**
 * @tc.name: InterceptInputEventToServer
 * @tc.desc: Window controller InterceptInputEventToServer
 * @tc.type: FUNC
 */
HWTEST_F(WindowControllerTest, InterceptInputEventToServer, TestSize.Level1)
{
    windowRoot_->windowNodeMap_.clear();
    sptr<IWindow> window;
    sptr<WindowProperty> property = new WindowProperty();
    std::shared_ptr<RSSurfaceNode> surfaceNode = nullptr;
    sptr<WindowNode> windowNode = new WindowNode(property);
    property->SetParentId(INVALID_WINDOW_ID);
    property->SetWindowType(WindowType::APP_WINDOW_BASE);
    windowRoot_->windowNodeMap_.insert(std::make_pair(1, windowNode));

    uint32_t windowId = windowNode->GetWindowId();
    struct RSSurfaceNodeConfig surfaceNodeConfig;
    surfaceNodeConfig.SurfaceNodeName = "InterceptInputEventToServer";
    surfaceNode = RSSurfaceNode::Create(surfaceNodeConfig, RSSurfaceNodeType::DEFAULT);
    ASSERT_EQ(WMError::WM_OK,
        windowController_->CreateWindow(window, property, surfaceNode, windowId, nullptr, 0, 0));

    WMError res = windowController_->InterceptInputEventToServer(10);
    ASSERT_EQ(WMError::WM_ERROR_NULLPTR, res);

    res = windowController_->InterceptInputEventToServer(windowId);
    ASSERT_EQ(WMError::WM_OK, res);
}

/**
 * @tc.name: RecoverInputEventToClient
 * @tc.desc: Window controller RecoverInputEventToClient
 * @tc.type: FUNC
 */
HWTEST_F(WindowControllerTest, RecoverInputEventToClient, TestSize.Level1)
{
    windowRoot_->windowNodeMap_.clear();
    sptr<IWindow> window;
    sptr<WindowProperty> property = new WindowProperty();
    std::shared_ptr<RSSurfaceNode> surfaceNode = nullptr;
    sptr<WindowNode> windowNode = new WindowNode(property);
    property->SetParentId(INVALID_WINDOW_ID);
    property->SetWindowType(WindowType::APP_WINDOW_BASE);
    windowNode->SetInputEventCallingPid(2048);
    windowNode->SetCallingPid(2048);
    windowRoot_->windowNodeMap_.insert(std::make_pair(1, windowNode));

    uint32_t windowId = windowNode->GetWindowId();
    struct RSSurfaceNodeConfig surfaceNodeConfig;
    surfaceNodeConfig.SurfaceNodeName = "RecoverInputEventToClient";
    surfaceNode = RSSurfaceNode::Create(surfaceNodeConfig, RSSurfaceNodeType::DEFAULT);
    ASSERT_EQ(WMError::WM_OK,
        windowController_->CreateWindow(window, property, surfaceNode, windowId, nullptr, 0, 0));

    WMError res = windowController_->RecoverInputEventToClient(10);
    ASSERT_EQ(WMError::WM_ERROR_NULLPTR, res);

    res = windowController_->RecoverInputEventToClient(windowId);
    ASSERT_EQ(WMError::WM_OK, res);
}

/**
 * @tc.name: RecoverInputEventToClient2
 * @tc.desc: Window controller RecoverInputEventToClient2
 * @tc.type: FUNC
 */
HWTEST_F(WindowControllerTest, RecoverInputEventToClient2, TestSize.Level1)
{
    windowRoot_->windowNodeMap_.clear();
    sptr<IWindow> window;
    sptr<WindowProperty> property = new WindowProperty();
    std::shared_ptr<RSSurfaceNode> surfaceNode = nullptr;
    sptr<WindowNode> windowNode = new WindowNode(property);
    property->SetParentId(INVALID_WINDOW_ID);
    property->SetWindowType(WindowType::APP_WINDOW_BASE);
    windowNode->SetInputEventCallingPid(2048);
    windowNode->SetCallingPid(1024);
    windowRoot_->windowNodeMap_.insert(std::make_pair(1, windowNode));

    uint32_t windowId = windowNode->GetWindowId();
    struct RSSurfaceNodeConfig surfaceNodeConfig;
    surfaceNodeConfig.SurfaceNodeName = "RecoverInputEventToClient2";
    surfaceNode = RSSurfaceNode::Create(surfaceNodeConfig, RSSurfaceNodeType::DEFAULT);
    ASSERT_EQ(WMError::WM_OK,
        windowController_->CreateWindow(window, property, surfaceNode, windowId, nullptr, 0, 0));

    WMError res = windowController_->RecoverInputEventToClient(windowId);
    ASSERT_EQ(WMError::WM_OK, res);
}

/**
 * @tc.name: RecoverDefaultMouseStyle
 * @tc.desc: Window controller RecoverDefaultMouseStyle
 * @tc.type: FUNC
 */
HWTEST_F(WindowControllerTest, RecoverDefaultMouseStyle, TestSize.Level1)
{
    windowRoot_->windowNodeMap_.clear();
    sptr<IWindow> window;
    sptr<WindowProperty> property = new WindowProperty();
    std::shared_ptr<RSSurfaceNode> surfaceNode = nullptr;
    sptr<WindowNode> windowNode = new WindowNode(property);
    property->SetParentId(INVALID_WINDOW_ID);
    property->SetWindowType(WindowType::APP_WINDOW_BASE);
    windowNode->SetInputEventCallingPid(2048);
    windowNode->SetCallingPid(1024);
    windowRoot_->windowNodeMap_.insert(std::make_pair(1, windowNode));

    uint32_t windowId = windowNode->GetWindowId();
    windowController_->RecoverDefaultMouseStyle(windowId);
    struct RSSurfaceNodeConfig surfaceNodeConfig;
    surfaceNodeConfig.SurfaceNodeName = "RecoverDefaultMouseStyle";
    surfaceNode = RSSurfaceNode::Create(surfaceNodeConfig, RSSurfaceNodeType::DEFAULT);
    ASSERT_EQ(WMError::WM_OK,
        windowController_->CreateWindow(window, property, surfaceNode, windowId, nullptr, 0, 0));
}

/**
 * @tc.name: DispatchKeyEvent
 * @tc.desc: Window controller DispatchKeyEvent
 * @tc.type: FUNC
 */
HWTEST_F(WindowControllerTest, DispatchKeyEvent, TestSize.Level1)
{
    windowRoot_->windowNodeMap_.clear();
    sptr<IWindow> window;
    sptr<WindowProperty> property = new WindowProperty();
    std::shared_ptr<RSSurfaceNode> surfaceNode = nullptr;
    sptr<WindowNode> windowNode = new WindowNode(property);
    property->SetParentId(INVALID_WINDOW_ID);
    property->SetWindowType(WindowType::APP_WINDOW_BASE);
    windowNode->SetInputEventCallingPid(2048);
    windowNode->SetCallingPid(2048);
    windowRoot_->windowNodeMap_.insert(std::make_pair(1, windowNode));

    uint32_t windowId = windowNode->GetWindowId();
    struct RSSurfaceNodeConfig surfaceNodeConfig;
    surfaceNodeConfig.SurfaceNodeName = "DispatchKeyEvent";
    surfaceNode = RSSurfaceNode::Create(surfaceNodeConfig, RSSurfaceNodeType::DEFAULT);
    std::shared_ptr<MMI::KeyEvent> event = nullptr;
    windowController_->DispatchKeyEvent(10, event);
    windowController_->DispatchKeyEvent(windowId, event);
    ASSERT_EQ(WMError::WM_OK,
        windowController_->CreateWindow(window, property, surfaceNode, windowId, nullptr, 0, 0));
}

/**
 * @tc.name: DispatchKeyEvent
 * @tc.desc: Window controller DispatchKeyEvent WindowType = WINDOW_TYPE_APP_COMPONENT
 * @tc.type: FUNC
 */
HWTEST_F(WindowControllerTest, DispatchKeyEvent2, TestSize.Level1)
{
    windowRoot_->windowNodeMap_.clear();
    sptr<IWindow> window;
    sptr<WindowProperty> property = new WindowProperty();
    std::shared_ptr<RSSurfaceNode> surfaceNode = nullptr;
    sptr<WindowNode> windowNode = new WindowNode(property);
    property->SetParentId(INVALID_WINDOW_ID);
    property->SetWindowType(WindowType::WINDOW_TYPE_APP_COMPONENT);
    windowNode->SetInputEventCallingPid(2048);
    windowNode->SetCallingPid(2048);
    windowRoot_->windowNodeMap_.insert(std::make_pair(1, windowNode));

    uint32_t windowId = windowNode->GetWindowId();
    struct RSSurfaceNodeConfig surfaceNodeConfig;
    surfaceNodeConfig.SurfaceNodeName = "DispatchKeyEvent2";
    surfaceNode = RSSurfaceNode::Create(surfaceNodeConfig, RSSurfaceNodeType::DEFAULT);

    std::shared_ptr<MMI::KeyEvent> event = nullptr;
    windowController_->DispatchKeyEvent(windowId, event);
    ASSERT_EQ(WMError::WM_ERROR_INVALID_PARENT,
        windowController_->CreateWindow(window, property, surfaceNode, windowId, nullptr, 0, 0));
}

/**
 * @tc.name: NotifyWindowClientPointUp
 * @tc.desc: Window controller NotifyWindowClientPointUp
 * @tc.type: FUNC
 */
HWTEST_F(WindowControllerTest, NotifyWindowClientPointUp, TestSize.Level1)
{
    windowRoot_->windowNodeMap_.clear();
    sptr<IWindow> window;
    sptr<WindowProperty> property = new WindowProperty();
    std::shared_ptr<RSSurfaceNode> surfaceNode = nullptr;
    sptr<WindowNode> windowNode = new WindowNode(property);
    property->SetParentId(INVALID_WINDOW_ID);
    property->SetWindowType(WindowType::WINDOW_TYPE_DOCK_SLICE);
    windowRoot_->windowNodeMap_.insert(std::make_pair(1, windowNode));

    uint32_t windowId = windowNode->GetWindowId();
    struct RSSurfaceNodeConfig surfaceNodeConfig;
    surfaceNodeConfig.SurfaceNodeName = "ProcessPointUp";
    surfaceNode = RSSurfaceNode::Create(surfaceNodeConfig, RSSurfaceNodeType::DEFAULT);
    ASSERT_EQ(WMError::WM_OK,
        windowController_->CreateWindow(window, property, surfaceNode, windowId, nullptr, 0, 0));

    std::shared_ptr<MMI::PointerEvent> pointerEvent = nullptr;
    WMError res = windowController_->NotifyWindowClientPointUp(10, pointerEvent);
    ASSERT_EQ(WMError::WM_ERROR_NULLPTR, res);

    res = windowController_->NotifyWindowClientPointUp(windowId, pointerEvent);
    ASSERT_EQ(WMError::WM_OK, res);
}

/**
 * @tc.name: MinimizeAllAppWindows
 * @tc.desc: Window controller MinimizeAllAppWindows
 * @tc.type: FUNC
 */
HWTEST_F(WindowControllerTest, MinimizeAllAppWindows, TestSize.Level1)
{
    windowRoot_->windowNodeMap_.clear();
    sptr<IWindow> window;
    sptr<WindowProperty> property = new WindowProperty();
    std::shared_ptr<RSSurfaceNode> surfaceNode = nullptr;
    sptr<WindowNode> windowNode = new WindowNode(property);
    property->SetParentId(INVALID_WINDOW_ID);
    property->SetWindowType(WindowType::APP_WINDOW_BASE);
    windowRoot_->windowNodeMap_.insert(std::make_pair(1, windowNode));

    uint32_t windowId = windowNode->GetWindowId();
    windowController_->MinimizeAllAppWindows(0);
    struct RSSurfaceNodeConfig surfaceNodeConfig;
    surfaceNodeConfig.SurfaceNodeName = "MinimizeAllAppWindows";
    surfaceNode = RSSurfaceNode::Create(surfaceNodeConfig, RSSurfaceNodeType::DEFAULT);
    ASSERT_EQ(WMError::WM_OK,
        windowController_->CreateWindow(window, property, surfaceNode, windowId, nullptr, 0, 0));
}

/**
 * @tc.name: ToggleShownStateForAllAppWindows
 * @tc.desc: Window controller ToggleShownStateForAllAppWindows
 * @tc.type: FUNC
 */
HWTEST_F(WindowControllerTest, ToggleShownStateForAllAppWindows, TestSize.Level1)
{
    windowRoot_->windowNodeMap_.clear();
    sptr<IWindow> window;
    sptr<WindowProperty> property = new WindowProperty();
    std::shared_ptr<RSSurfaceNode> surfaceNode = nullptr;
    sptr<WindowNode> windowNode = new WindowNode(property);
    property->SetParentId(INVALID_WINDOW_ID);
    property->SetWindowType(WindowType::APP_WINDOW_BASE);
    windowRoot_->windowNodeMap_.insert(std::make_pair(1, windowNode));

    uint32_t windowId = windowNode->GetWindowId();
    struct RSSurfaceNodeConfig surfaceNodeConfig;
    surfaceNodeConfig.SurfaceNodeName = "ToggleShownStateForAllAppWindows";
    surfaceNode = RSSurfaceNode::Create(surfaceNodeConfig, RSSurfaceNodeType::DEFAULT);
    ASSERT_EQ(WMError::WM_OK,
        windowController_->CreateWindow(window, property, surfaceNode, windowId, nullptr, 0, 0));

    WMError res = windowController_->ToggleShownStateForAllAppWindows();
    ASSERT_EQ(WMError::WM_OK, res);
}

/**
 * @tc.name: NotifyScreenshotEvent
 * @tc.desc: Window controller NotifyScreenshotEvent
 * @tc.type: FUNC
 */
HWTEST_F(WindowControllerTest, NotifyScreenshotEvent, TestSize.Level1)
{
    windowRoot_->windowNodeMap_.clear();
    sptr<IWindow> window;
    sptr<WindowProperty> property = new WindowProperty();
    std::shared_ptr<RSSurfaceNode> surfaceNode = nullptr;
    sptr<WindowNode> windowNode = new WindowNode(property);
    property->SetParentId(INVALID_WINDOW_ID);
    property->SetWindowType(WindowType::APP_WINDOW_BASE);
    windowRoot_->windowNodeMap_.insert(std::make_pair(1, windowNode));

    uint32_t windowId = windowNode->GetWindowId();
    struct RSSurfaceNodeConfig surfaceNodeConfig;
    surfaceNodeConfig.SurfaceNodeName = "NotifyScreenshotEvent";
    surfaceNode = RSSurfaceNode::Create(surfaceNodeConfig, RSSurfaceNodeType::DEFAULT);
    EXPECT_EQ(WMError::WM_OK,
        windowController_->CreateWindow(window, property, surfaceNode, windowId, nullptr, 0, 0));
        
    ScreenshotEventType type = ScreenshotEventType::SCROLL_SHOT_START;
    WMError res = windowController_->NotifyScreenshotEvent(type);
    EXPECT_EQ(WMError::WM_OK, res);
}

/**
 * @tc.name: GetUnreliableWindowInfo
 * @tc.desc: Window controller window is unreliable window
 * @tc.type: FUNC
 */
HWTEST_F(WindowControllerTest, GetUnreliableWindowInfo1, TestSize.Level1)
{
    windowRoot_->windowNodeMap_.clear();
    sptr<WindowProperty> property = new WindowProperty();
    ASSERT_NE(nullptr, property);
    property->SetWindowType(WindowType::WINDOW_TYPE_TOAST);
    sptr<WindowNode> windowNode = new WindowNode(property);
    ASSERT_NE(nullptr, windowNode);
    windowNode->currentVisibility_ = true;
    windowRoot_->windowNodeMap_.insert(std::make_pair(windowNode->GetWindowId(), windowNode));
    std::vector<sptr<UnreliableWindowInfo>> infos;
    ASSERT_EQ(WMError::WM_OK, windowController_->GetUnreliableWindowInfo(0, infos));
    EXPECT_EQ(1, infos.size());

    sptr<WindowProperty> property2 = new WindowProperty();
    ASSERT_NE(nullptr, property2);
    property2->SetWindowType(WindowType::WINDOW_TYPE_INPUT_METHOD_FLOAT);
    sptr<WindowNode> windowNode2 = new WindowNode(property2);
    ASSERT_NE(nullptr, windowNode2);
    windowNode2->currentVisibility_ = true;
    windowRoot_->windowNodeMap_.insert(std::make_pair(windowNode2->GetWindowId(), windowNode2));
    ASSERT_EQ(WMError::WM_OK, windowController_->GetUnreliableWindowInfo(0, infos));
    EXPECT_EQ(2, infos.size());

    sptr<WindowProperty> property3 = new WindowProperty();
    ASSERT_NE(nullptr, property3);
    property3->SetParentId(windowNode->GetWindowId());
    property3->SetWindowType(WindowType::WINDOW_TYPE_APP_SUB_WINDOW);
    sptr<WindowNode> windowNode3 = new WindowNode(property3);
    ASSERT_NE(nullptr, windowNode3);
    windowNode3->currentVisibility_ = true;
    windowRoot_->windowNodeMap_.insert(std::make_pair(windowNode3->GetWindowId(), windowNode3));
    ASSERT_EQ(WMError::WM_OK, windowController_->GetUnreliableWindowInfo(0, infos));
    EXPECT_EQ(3, infos.size());
}

/**
 * @tc.name: GetUnreliableWindowInfo
 * @tc.desc: Window controller windowId is equal to the parameter
 * @tc.type: FUNC
 */
HWTEST_F(WindowControllerTest, GetUnreliableWindowInfo2, TestSize.Level1)
{
    windowRoot_->windowNodeMap_.clear();
    sptr<WindowProperty> property = new WindowProperty();
    ASSERT_NE(nullptr, property);
    sptr<WindowNode> windowNode = new WindowNode(property);
    ASSERT_NE(nullptr, windowNode);
    windowRoot_->windowNodeMap_.insert(std::make_pair(1, windowNode));
    std::vector<sptr<UnreliableWindowInfo>> infos;
    ASSERT_EQ(WMError::WM_OK, windowController_->GetUnreliableWindowInfo(1, infos));
    ASSERT_EQ(false, infos.empty());
}

/**
 * @tc.name: GetUnreliableWindowInfo
 * @tc.desc: Window controller window type is not correct, window is invisible
 * @tc.type: FUNC
 */
HWTEST_F(WindowControllerTest, GetUnreliableWindowInfo3, TestSize.Level1)
{
    windowRoot_->windowNodeMap_.clear();
    sptr<WindowProperty> property = new WindowProperty();
    ASSERT_NE(nullptr, property);
    property->SetWindowType(WindowType::WINDOW_TYPE_DIALOG);
    sptr<WindowNode> windowNode = new WindowNode(property);
    ASSERT_NE(nullptr, windowNode);
    windowNode->currentVisibility_ = true;
    windowRoot_->windowNodeMap_.insert(std::make_pair(1, windowNode));
    std::vector<sptr<UnreliableWindowInfo>> infos;
    ASSERT_EQ(WMError::WM_OK, windowController_->GetUnreliableWindowInfo(0, infos));
    ASSERT_EQ(true, infos.empty());

    windowRoot_->windowNodeMap_.clear();
    windowNode->currentVisibility_ = false;
    windowRoot_->windowNodeMap_.insert(std::make_pair(1, windowNode));
    ASSERT_EQ(WMError::WM_OK, windowController_->GetUnreliableWindowInfo(0, infos));
    ASSERT_EQ(true, infos.empty());
}

/**
 * @tc.name: UpdateProperty
 * @tc.desc: Window controller UpdateProperty property is nullptr
 * @tc.type: FUNC
 */
HWTEST_F(WindowControllerTest, UpdateProperty1, TestSize.Level1)
{
    windowRoot_->windowNodeMap_.clear();
    sptr<WindowProperty> property = nullptr;
    PropertyChangeAction action = PropertyChangeAction::ACTION_UPDATE_RECT;
    WMError res = windowController_->UpdateProperty(property, action);
    ASSERT_EQ(WMError::WM_ERROR_NULLPTR, res);
}

/**
 * @tc.name: UpdateProperty
 * @tc.desc: Window controller UpdateProperty windowRoot_ is nullptr
 * @tc.type: FUNC
 */
HWTEST_F(WindowControllerTest, UpdateProperty2, TestSize.Level1)
{
    windowRoot_->windowNodeMap_.clear();
    sptr<IWindow> window;
    sptr<WindowProperty> property = new WindowProperty();
    std::shared_ptr<RSSurfaceNode> surfaceNode = nullptr;
    sptr<WindowNode> windowNode = new WindowNode(property);
    property->SetParentId(INVALID_WINDOW_ID);
    property->SetWindowType(WindowType::APP_WINDOW_BASE);

    uint32_t windowId = windowNode->GetWindowId();
    ASSERT_EQ(nullptr, windowRoot_->GetWindowNode(windowId));
    ASSERT_NE(nullptr, property);

    PropertyChangeAction action = PropertyChangeAction::ACTION_UPDATE_RECT;
    WMError res = windowController_->UpdateProperty(property, action);
    ASSERT_EQ(WMError::WM_ERROR_NULLPTR, res);
}

/**
 * @tc.name: UpdateProperty
 * @tc.desc: Window controller UpdateProperty ACTION_UPDATE_RECT
 * @tc.type: FUNC
 */
HWTEST_F(WindowControllerTest, UpdateProperty3, TestSize.Level1)
{
    windowRoot_->windowNodeMap_.clear();
    sptr<IWindow> window;
    sptr<WindowProperty> property = new WindowProperty();
    std::shared_ptr<RSSurfaceNode> surfaceNode = nullptr;
    sptr<WindowNode> windowNode = new WindowNode(property);
    property->SetParentId(INVALID_WINDOW_ID);
    windowRoot_->windowNodeMap_.insert(std::make_pair(0, windowNode));

    uint32_t windowId = windowNode->GetWindowId();
    struct RSSurfaceNodeConfig surfaceNodeConfig;
    surfaceNodeConfig.SurfaceNodeName = "UpdateProperty3";
    surfaceNode = RSSurfaceNode::Create(surfaceNodeConfig, RSSurfaceNodeType::DEFAULT);
    ASSERT_EQ(WMError::WM_OK,
        windowController_->CreateWindow(window, property, surfaceNode, windowId, nullptr, 0, 0));
    ASSERT_NE(nullptr, windowRoot_->GetWindowNode(windowId));
    ASSERT_NE(nullptr, property);

    PropertyChangeAction action = PropertyChangeAction::ACTION_UPDATE_RECT;
    WMError res = windowController_->UpdateProperty(property, action);
    ASSERT_EQ(WMError::WM_ERROR_INVALID_OPERATION, res);

    windowNode->SetWindowRect({50, 50, 50, 50});
    windowNode->SetWindowMode(WindowMode::WINDOW_MODE_FLOATING);
    res = windowController_->UpdateProperty(property, action);
    ASSERT_EQ(WMError::WM_ERROR_INVALID_DISPLAY, res);

    property->SetWindowType(WindowType::WINDOW_TYPE_STATUS_BAR);
    property->SetWindowSizeChangeReason(WindowSizeChangeReason::UNDEFINED);
    res = windowController_->UpdateProperty(property, action);
    ASSERT_EQ(WMError::WM_ERROR_INVALID_DISPLAY, res);

    property->SetWindowSizeChangeReason(WindowSizeChangeReason::RESIZE);
    res = windowController_->UpdateProperty(property, action);
    ASSERT_EQ(WMError::WM_ERROR_INVALID_DISPLAY, res);

    property->SetWindowSizeChangeReason(WindowSizeChangeReason::MOVE);
    res = windowController_->UpdateProperty(property, action);
    ASSERT_EQ(WMError::WM_ERROR_INVALID_DISPLAY, res);

    property->SetWindowSizeChangeReason(WindowSizeChangeReason::DRAG_MOVE);
    res = windowController_->UpdateProperty(property, action);
    ASSERT_EQ(WMError::WM_ERROR_INVALID_DISPLAY, res);
}

/**
 * @tc.name: UpdateProperty
 * @tc.desc: Window controller UpdateProperty ACTION_UPDATE_MODE
 * @tc.type: FUNC
 */
HWTEST_F(WindowControllerTest, UpdateProperty4, TestSize.Level1)
{
    windowRoot_->windowNodeMap_.clear();
    sptr<IWindow> window;
    sptr<WindowProperty> property = new WindowProperty();
    std::shared_ptr<RSSurfaceNode> surfaceNode = nullptr;
    sptr<WindowNode> windowNode = new WindowNode(property);
    property->SetParentId(INVALID_WINDOW_ID);
    property->SetWindowType(WindowType::APP_WINDOW_BASE);
    windowRoot_->windowNodeMap_.insert(std::make_pair(0, windowNode));

    uint32_t windowId = windowNode->GetWindowId();
    struct RSSurfaceNodeConfig surfaceNodeConfig;
    surfaceNodeConfig.SurfaceNodeName = "UpdateProperty4";
    surfaceNode = RSSurfaceNode::Create(surfaceNodeConfig, RSSurfaceNodeType::DEFAULT);
    ASSERT_EQ(WMError::WM_OK,
        windowController_->CreateWindow(window, property, surfaceNode, windowId, nullptr, 0, 0));

    ASSERT_NE(nullptr, windowRoot_->GetWindowNode(windowId));
    ASSERT_NE(nullptr, property);
    PropertyChangeAction action = PropertyChangeAction::ACTION_UPDATE_MODE;
    WMError res = windowController_->UpdateProperty(property, action);
    ASSERT_EQ(WMError::WM_ERROR_INVALID_DISPLAY, res);
}

/**
 * @tc.name: UpdateProperty
 * @tc.desc: Window controller UpdateProperty ACTION_UPDATE_FLAGS
 * @tc.type: FUNC
 */
HWTEST_F(WindowControllerTest, UpdateProperty5, TestSize.Level1)
{
    windowRoot_->windowNodeMap_.clear();
    sptr<IWindow> window;
    sptr<WindowProperty> property = new WindowProperty();
    std::shared_ptr<RSSurfaceNode> surfaceNode = nullptr;
    sptr<WindowNode> windowNode = new WindowNode(property);
    property->SetParentId(INVALID_WINDOW_ID);
    property->SetWindowType(WindowType::APP_WINDOW_BASE);
    windowRoot_->windowNodeMap_.insert(std::make_pair(0, windowNode));

    uint32_t windowId = windowNode->GetWindowId();
    struct RSSurfaceNodeConfig surfaceNodeConfig;
    surfaceNodeConfig.SurfaceNodeName = "UpdateProperty5";
    surfaceNode = RSSurfaceNode::Create(surfaceNodeConfig, RSSurfaceNodeType::DEFAULT);
    ASSERT_EQ(WMError::WM_OK,
        windowController_->CreateWindow(window, property, surfaceNode, windowId, nullptr, 0, 0));

    ASSERT_NE(nullptr, windowRoot_->GetWindowNode(windowId));
    ASSERT_NE(nullptr, property);
    PropertyChangeAction action = PropertyChangeAction::ACTION_UPDATE_FLAGS;
    WMError res = windowController_->UpdateProperty(property, action);
    ASSERT_EQ(WMError::WM_ERROR_INVALID_DISPLAY, res);
}

/**
 * @tc.name: UpdateProperty
 * @tc.desc: Window controller UpdateProperty ACTION_UPDATE_OTHER_PROPS
 * @tc.type: FUNC
 */
HWTEST_F(WindowControllerTest, UpdateProperty6, TestSize.Level1)
{
    windowRoot_->windowNodeMap_.clear();
    sptr<IWindow> window;
    sptr<WindowProperty> property = new WindowProperty();
    std::shared_ptr<RSSurfaceNode> surfaceNode = nullptr;
    sptr<WindowNode> windowNode = new WindowNode(property);
    property->SetParentId(INVALID_WINDOW_ID);
    property->SetWindowType(WindowType::APP_WINDOW_BASE);
    windowRoot_->windowNodeMap_.insert(std::make_pair(0, windowNode));

    uint32_t windowId = windowNode->GetWindowId();
    struct RSSurfaceNodeConfig surfaceNodeConfig;
    surfaceNodeConfig.SurfaceNodeName = "UpdateProperty6";
    surfaceNode = RSSurfaceNode::Create(surfaceNodeConfig, RSSurfaceNodeType::DEFAULT);
    ASSERT_EQ(WMError::WM_OK,
        windowController_->CreateWindow(window, property, surfaceNode, windowId, nullptr, 0, 0));

    ASSERT_NE(nullptr, windowRoot_->GetWindowNode(windowId));
    ASSERT_NE(nullptr, property);
    PropertyChangeAction action = PropertyChangeAction::ACTION_UPDATE_OTHER_PROPS;
    WMError res = windowController_->UpdateProperty(property, action);
    ASSERT_EQ(WMError::WM_OK, res);
}

/**
 * @tc.name: UpdateProperty
 * @tc.desc: Window controller UpdateProperty ACTION_UPDATE_FOCUSABLE
 * @tc.type: FUNC
 */
HWTEST_F(WindowControllerTest, UpdateProperty7, TestSize.Level1)
{
    windowRoot_->windowNodeMap_.clear();
    sptr<IWindow> window;
    sptr<WindowProperty> property = new WindowProperty();
    std::shared_ptr<RSSurfaceNode> surfaceNode = nullptr;
    sptr<WindowNode> windowNode = new WindowNode(property);
    property->SetParentId(INVALID_WINDOW_ID);
    property->SetWindowType(WindowType::APP_WINDOW_BASE);
    windowRoot_->windowNodeMap_.insert(std::make_pair(0, windowNode));

    uint32_t windowId = windowNode->GetWindowId();
    struct RSSurfaceNodeConfig surfaceNodeConfig;
    surfaceNodeConfig.SurfaceNodeName = "UpdateProperty7";
    surfaceNode = RSSurfaceNode::Create(surfaceNodeConfig, RSSurfaceNodeType::DEFAULT);
    ASSERT_EQ(WMError::WM_OK,
        windowController_->CreateWindow(window, property, surfaceNode, windowId, nullptr, 0, 0));

    ASSERT_NE(nullptr, windowRoot_->GetWindowNode(windowId));
    ASSERT_NE(nullptr, property);
    PropertyChangeAction action = PropertyChangeAction::ACTION_UPDATE_FOCUSABLE;
    WMError res = windowController_->UpdateProperty(property, action);
    ASSERT_EQ(WMError::WM_OK, res);
}

/**
 * @tc.name: UpdateProperty
 * @tc.desc: Window controller UpdateProperty ACTION_UPDATE_TOUCHABLE
 * @tc.type: FUNC
 */
HWTEST_F(WindowControllerTest, UpdateProperty8, TestSize.Level1)
{
    windowRoot_->windowNodeMap_.clear();
    sptr<IWindow> window;
    sptr<WindowProperty> property = new WindowProperty();
    std::shared_ptr<RSSurfaceNode> surfaceNode = nullptr;
    sptr<WindowNode> windowNode = new WindowNode(property);
    property->SetParentId(INVALID_WINDOW_ID);
    property->SetWindowType(WindowType::APP_WINDOW_BASE);
    windowRoot_->windowNodeMap_.insert(std::make_pair(0, windowNode));

    uint32_t windowId = windowNode->GetWindowId();
    struct RSSurfaceNodeConfig surfaceNodeConfig;
    surfaceNodeConfig.SurfaceNodeName = "UpdateProperty8";
    surfaceNode = RSSurfaceNode::Create(surfaceNodeConfig, RSSurfaceNodeType::DEFAULT);
    ASSERT_EQ(WMError::WM_OK,
        windowController_->CreateWindow(window, property, surfaceNode, windowId, nullptr, 0, 0));

    ASSERT_NE(nullptr, windowRoot_->GetWindowNode(windowId));
    ASSERT_NE(nullptr, property);
    PropertyChangeAction action = PropertyChangeAction::ACTION_UPDATE_TOUCHABLE;
    WMError res = windowController_->UpdateProperty(property, action);
    ASSERT_EQ(WMError::WM_OK, res);
}

/**
 * @tc.name: UpdateProperty
 * @tc.desc: Window controller UpdateProperty ACTION_UPDATE_CALLING_WINDOW
 * @tc.type: FUNC
 */
HWTEST_F(WindowControllerTest, UpdateProperty9, TestSize.Level1)
{
    windowRoot_->windowNodeMap_.clear();
    sptr<IWindow> window;
    sptr<WindowProperty> property = new WindowProperty();
    std::shared_ptr<RSSurfaceNode> surfaceNode = nullptr;
    sptr<WindowNode> windowNode = new WindowNode(property);
    property->SetParentId(INVALID_WINDOW_ID);
    property->SetWindowType(WindowType::APP_WINDOW_BASE);
    windowRoot_->windowNodeMap_.insert(std::make_pair(0, windowNode));

    uint32_t windowId = windowNode->GetWindowId();
    struct RSSurfaceNodeConfig surfaceNodeConfig;
    surfaceNodeConfig.SurfaceNodeName = "UpdateProperty9";
    surfaceNode = RSSurfaceNode::Create(surfaceNodeConfig, RSSurfaceNodeType::DEFAULT);
    ASSERT_EQ(WMError::WM_OK,
        windowController_->CreateWindow(window, property, surfaceNode, windowId, nullptr, 0, 0));

    ASSERT_NE(nullptr, windowRoot_->GetWindowNode(windowId));
    ASSERT_NE(nullptr, property);
    PropertyChangeAction action = PropertyChangeAction::ACTION_UPDATE_CALLING_WINDOW;
    WMError res = windowController_->UpdateProperty(property, action);
    ASSERT_EQ(WMError::WM_OK, res);
}

/**
 * @tc.name: UpdateProperty
 * @tc.desc: Window controller UpdateProperty ACTION_UPDATE_ORIENTATION
 * @tc.type: FUNC
 */
HWTEST_F(WindowControllerTest, UpdateProperty10, TestSize.Level1)
{
    windowRoot_->windowNodeMap_.clear();
    sptr<IWindow> window;
    sptr<WindowProperty> property = new WindowProperty();
    std::shared_ptr<RSSurfaceNode> surfaceNode = nullptr;
    sptr<WindowNode> windowNode = new WindowNode(property);
    property->SetParentId(INVALID_WINDOW_ID);
    property->SetWindowType(WindowType::APP_WINDOW_BASE);
    windowRoot_->windowNodeMap_.insert(std::make_pair(0, windowNode));

    uint32_t windowId = windowNode->GetWindowId();
    struct RSSurfaceNodeConfig surfaceNodeConfig;
    surfaceNodeConfig.SurfaceNodeName = "UpdateProperty10";
    surfaceNode = RSSurfaceNode::Create(surfaceNodeConfig, RSSurfaceNodeType::DEFAULT);
    ASSERT_EQ(WMError::WM_OK,
        windowController_->CreateWindow(window, property, surfaceNode, windowId, nullptr, 0, 0));

    ASSERT_NE(nullptr, windowRoot_->GetWindowNode(windowId));
    ASSERT_NE(nullptr, property);
    PropertyChangeAction action = PropertyChangeAction::ACTION_UPDATE_ORIENTATION;
    WMError res = windowController_->UpdateProperty(property, action);
    ASSERT_EQ(WMError::WM_OK, res);
}

/**
 * @tc.name: UpdateProperty
 * @tc.desc: Window controller UpdateProperty ACTION_UPDATE_TURN_SCREEN_ON
 * @tc.type: FUNC
 */
HWTEST_F(WindowControllerTest, UpdateProperty11, TestSize.Level1)
{
    windowRoot_->windowNodeMap_.clear();
    sptr<IWindow> window;
    sptr<WindowProperty> property = new WindowProperty();
    std::shared_ptr<RSSurfaceNode> surfaceNode = nullptr;
    sptr<WindowNode> windowNode = new WindowNode(property);
    property->SetParentId(INVALID_WINDOW_ID);
    property->SetWindowType(WindowType::APP_WINDOW_BASE);
    windowRoot_->windowNodeMap_.insert(std::make_pair(0, windowNode));

    uint32_t windowId = windowNode->GetWindowId();
    struct RSSurfaceNodeConfig surfaceNodeConfig;
    surfaceNodeConfig.SurfaceNodeName = "UpdateProperty11";
    surfaceNode = RSSurfaceNode::Create(surfaceNodeConfig, RSSurfaceNodeType::DEFAULT);
    ASSERT_EQ(WMError::WM_OK,
        windowController_->CreateWindow(window, property, surfaceNode, windowId, nullptr, 0, 0));

    ASSERT_NE(nullptr, windowRoot_->GetWindowNode(windowId));
    ASSERT_NE(nullptr, property);
    PropertyChangeAction action = PropertyChangeAction::ACTION_UPDATE_TURN_SCREEN_ON;
    WMError res = windowController_->UpdateProperty(property, action);
    ASSERT_EQ(WMError::WM_OK, res);
}

/**
 * @tc.name: UpdateProperty
 * @tc.desc: Window controller UpdateProperty ACTION_UPDATE_KEEP_SCREEN_ON
 * @tc.type: FUNC
 */
HWTEST_F(WindowControllerTest, UpdateProperty12, TestSize.Level1)
{
    windowRoot_->windowNodeMap_.clear();
    sptr<IWindow> window;
    sptr<WindowProperty> property = new WindowProperty();
    std::shared_ptr<RSSurfaceNode> surfaceNode = nullptr;
    sptr<WindowNode> windowNode = new WindowNode(property);
    property->SetParentId(INVALID_WINDOW_ID);
    property->SetWindowType(WindowType::APP_WINDOW_BASE);
    windowRoot_->windowNodeMap_.insert(std::make_pair(0, windowNode));

    uint32_t windowId = windowNode->GetWindowId();
    struct RSSurfaceNodeConfig surfaceNodeConfig;
    surfaceNodeConfig.SurfaceNodeName = "UpdateProperty12";
    surfaceNode = RSSurfaceNode::Create(surfaceNodeConfig, RSSurfaceNodeType::DEFAULT);
    ASSERT_EQ(WMError::WM_OK,
        windowController_->CreateWindow(window, property, surfaceNode, windowId, nullptr, 0, 0));

    ASSERT_NE(nullptr, windowRoot_->GetWindowNode(windowId));
    ASSERT_NE(nullptr, property);
    PropertyChangeAction action = PropertyChangeAction::ACTION_UPDATE_KEEP_SCREEN_ON;
    WMError res = windowController_->UpdateProperty(property, action);
    ASSERT_EQ(WMError::WM_OK, res);
}

/**
 * @tc.name: UpdateProperty
 * @tc.desc: Window controller UpdateProperty ACTION_UPDATE_SET_BRIGHTNESS
 * @tc.type: FUNC
 */
HWTEST_F(WindowControllerTest, UpdateProperty13, TestSize.Level1)
{
    windowRoot_->windowNodeMap_.clear();
    sptr<IWindow> window;
    sptr<WindowProperty> property = new WindowProperty();
    std::shared_ptr<RSSurfaceNode> surfaceNode = nullptr;
    sptr<WindowNode> windowNode = new WindowNode(property);
    property->SetParentId(INVALID_WINDOW_ID);
    property->SetWindowType(WindowType::APP_WINDOW_BASE);
    windowRoot_->windowNodeMap_.insert(std::make_pair(0, windowNode));

    uint32_t windowId = windowNode->GetWindowId();
    struct RSSurfaceNodeConfig surfaceNodeConfig;
    surfaceNodeConfig.SurfaceNodeName = "UpdateProperty13";
    surfaceNode = RSSurfaceNode::Create(surfaceNodeConfig, RSSurfaceNodeType::DEFAULT);
    ASSERT_EQ(WMError::WM_OK,
        windowController_->CreateWindow(window, property, surfaceNode, windowId, nullptr, 0, 0));

    ASSERT_NE(nullptr, windowRoot_->GetWindowNode(windowId));
    ASSERT_NE(nullptr, property);
    PropertyChangeAction action = PropertyChangeAction::ACTION_UPDATE_SET_BRIGHTNESS;
    WMError res = windowController_->UpdateProperty(property, action);
    ASSERT_EQ(WMError::WM_OK, res);
}

/**
 * @tc.name: UpdateProperty
 * @tc.desc: Window controller UpdateProperty ACTION_UPDATE_MODE_SUPPORT_INFO
 * @tc.type: FUNC
 */
HWTEST_F(WindowControllerTest, UpdateProperty14, TestSize.Level1)
{
    windowRoot_->windowNodeMap_.clear();
    sptr<IWindow> window;
    sptr<WindowProperty> property = new WindowProperty();
    std::shared_ptr<RSSurfaceNode> surfaceNode = nullptr;
    sptr<WindowNode> windowNode = new WindowNode(property);
    property->SetParentId(INVALID_WINDOW_ID);
    property->SetWindowType(WindowType::APP_WINDOW_BASE);
    windowRoot_->windowNodeMap_.insert(std::make_pair(0, windowNode));

    uint32_t windowId = windowNode->GetWindowId();
    struct RSSurfaceNodeConfig surfaceNodeConfig;
    surfaceNodeConfig.SurfaceNodeName = "UpdateProperty14";
    surfaceNode = RSSurfaceNode::Create(surfaceNodeConfig, RSSurfaceNodeType::DEFAULT);
    ASSERT_EQ(WMError::WM_OK,
        windowController_->CreateWindow(window, property, surfaceNode, windowId, nullptr, 0, 0));

    ASSERT_NE(nullptr, windowRoot_->GetWindowNode(windowId));
    ASSERT_NE(nullptr, property);
    PropertyChangeAction action = PropertyChangeAction::ACTION_UPDATE_MODE_SUPPORT_INFO;
    WMError res = windowController_->UpdateProperty(property, action);
    ASSERT_EQ(WMError::WM_OK, res);
}

/**
 * @tc.name: UpdateProperty
 * @tc.desc: Window controller UpdateProperty ACTION_UPDATE_TOUCH_HOT_AREA
 * @tc.type: FUNC
 */
HWTEST_F(WindowControllerTest, UpdateProperty15, TestSize.Level1)
{
    windowRoot_->windowNodeMap_.clear();
    sptr<IWindow> window;
    sptr<WindowProperty> property = new WindowProperty();
    std::shared_ptr<RSSurfaceNode> surfaceNode = nullptr;
    sptr<WindowNode> windowNode = new WindowNode(property);
    property->SetParentId(INVALID_WINDOW_ID);
    property->SetWindowType(WindowType::APP_WINDOW_BASE);
    windowRoot_->windowNodeMap_.insert(std::make_pair(0, windowNode));

    uint32_t windowId = windowNode->GetWindowId();
    struct RSSurfaceNodeConfig surfaceNodeConfig;
    surfaceNodeConfig.SurfaceNodeName = "UpdateProperty15";
    surfaceNode = RSSurfaceNode::Create(surfaceNodeConfig, RSSurfaceNodeType::DEFAULT);
    ASSERT_EQ(WMError::WM_OK,
        windowController_->CreateWindow(window, property, surfaceNode, windowId, nullptr, 0, 0));

    ASSERT_NE(nullptr, windowRoot_->GetWindowNode(windowId));
    ASSERT_NE(nullptr, property);
    PropertyChangeAction action = PropertyChangeAction::ACTION_UPDATE_TOUCH_HOT_AREA;
    WMError res = windowController_->UpdateProperty(property, action);
    ASSERT_EQ(WMError::WM_OK, res);
}

/**
 * @tc.name: UpdateProperty
 * @tc.desc: Window controller UpdateProperty ACTION_UPDATE_ANIMATION_FLAG
 * @tc.type: FUNC
 */
HWTEST_F(WindowControllerTest, UpdateProperty16, TestSize.Level1)
{
    windowRoot_->windowNodeMap_.clear();
    sptr<IWindow> window;
    sptr<WindowProperty> property = new WindowProperty();
    std::shared_ptr<RSSurfaceNode> surfaceNode = nullptr;
    sptr<WindowNode> windowNode = new WindowNode(property);
    property->SetParentId(INVALID_WINDOW_ID);
    property->SetWindowType(WindowType::APP_WINDOW_BASE);
    windowRoot_->windowNodeMap_.insert(std::make_pair(0, windowNode));

    uint32_t windowId = windowNode->GetWindowId();
    struct RSSurfaceNodeConfig surfaceNodeConfig;
    surfaceNodeConfig.SurfaceNodeName = "UpdateProperty16";
    surfaceNode = RSSurfaceNode::Create(surfaceNodeConfig, RSSurfaceNodeType::DEFAULT);
    ASSERT_EQ(WMError::WM_OK,
        windowController_->CreateWindow(window, property, surfaceNode, windowId, nullptr, 0, 0));

    ASSERT_NE(nullptr, windowRoot_->GetWindowNode(windowId));
    ASSERT_NE(nullptr, property);
    PropertyChangeAction action = PropertyChangeAction::ACTION_UPDATE_ANIMATION_FLAG;
    WMError res = windowController_->UpdateProperty(property, action);
    ASSERT_EQ(WMError::WM_OK, res);
}

/**
 * @tc.name: UpdateProperty
 * @tc.desc: Window controller UpdateProperty ACTION_UPDATE_TRANSFORM_PROPERTY
 * @tc.type: FUNC
 */
HWTEST_F(WindowControllerTest, UpdateProperty17, TestSize.Level1)
{
    windowRoot_->windowNodeMap_.clear();
    sptr<IWindow> window;
    sptr<WindowProperty> property = new WindowProperty();
    std::shared_ptr<RSSurfaceNode> surfaceNode = nullptr;
    sptr<WindowNode> windowNode = new WindowNode(property);
    property->SetParentId(INVALID_WINDOW_ID);
    property->SetWindowType(WindowType::APP_WINDOW_BASE);
    windowRoot_->windowNodeMap_.insert(std::make_pair(0, windowNode));

    uint32_t windowId = windowNode->GetWindowId();
    struct RSSurfaceNodeConfig surfaceNodeConfig;
    surfaceNodeConfig.SurfaceNodeName = "UpdateProperty17";
    surfaceNode = RSSurfaceNode::Create(surfaceNodeConfig, RSSurfaceNodeType::DEFAULT);
    ASSERT_EQ(WMError::WM_OK,
        windowController_->CreateWindow(window, property, surfaceNode, windowId, nullptr, 0, 0));

    ASSERT_NE(nullptr, windowRoot_->GetWindowNode(windowId));
    ASSERT_NE(nullptr, property);
    PropertyChangeAction action = PropertyChangeAction::ACTION_UPDATE_TRANSFORM_PROPERTY;
    WMError res = windowController_->UpdateProperty(property, action);
    ASSERT_EQ(WMError::WM_ERROR_INVALID_DISPLAY, res);
}

/**
 * @tc.name: UpdateProperty
 * @tc.desc: Window controller UpdateProperty ACTION_UPDATE_ASPECT_RATIO
 * @tc.type: FUNC
 */
HWTEST_F(WindowControllerTest, UpdateProperty19, TestSize.Level1)
{
    windowRoot_->windowNodeMap_.clear();
    sptr<IWindow> window;
    sptr<WindowProperty> property = new WindowProperty();
    std::shared_ptr<RSSurfaceNode> surfaceNode = nullptr;
    sptr<WindowNode> windowNode = new WindowNode(property);
    property->SetParentId(INVALID_WINDOW_ID);
    property->SetWindowType(WindowType::APP_WINDOW_BASE);
    windowRoot_->windowNodeMap_.insert(std::make_pair(0, windowNode));

    uint32_t windowId = windowNode->GetWindowId();
    struct RSSurfaceNodeConfig surfaceNodeConfig;
    surfaceNodeConfig.SurfaceNodeName = "UpdateProperty19";
    surfaceNode = RSSurfaceNode::Create(surfaceNodeConfig, RSSurfaceNodeType::DEFAULT);
    ASSERT_EQ(WMError::WM_OK,
        windowController_->CreateWindow(window, property, surfaceNode, windowId, nullptr, 0, 0));

    ASSERT_NE(nullptr, windowRoot_->GetWindowNode(windowId));
    ASSERT_NE(nullptr, property);
    PropertyChangeAction action = PropertyChangeAction::ACTION_UPDATE_ASPECT_RATIO;
    WMError res = windowController_->UpdateProperty(property, action);
    ASSERT_EQ(WMError::WM_OK, res);
}

/**
 * @tc.name: UpdateProperty
 * @tc.desc: Window controller UpdateProperty ACTION_UPDATE_MAXIMIZE_STATE ResizeRect
 * @tc.type: FUNC
 */
HWTEST_F(WindowControllerTest, UpdateProperty20, TestSize.Level1)
{
    windowRoot_->windowNodeMap_.clear();
    sptr<IWindow> window;
    sptr<WindowProperty> property = new WindowProperty();
    std::shared_ptr<RSSurfaceNode> surfaceNode = nullptr;
    sptr<WindowNode> windowNode = new WindowNode(property);
    property->SetParentId(INVALID_WINDOW_ID);
    property->SetWindowType(WindowType::APP_WINDOW_BASE);
    windowRoot_->windowNodeMap_.insert(std::make_pair(0, windowNode));

    uint32_t windowId = windowNode->GetWindowId();
    struct RSSurfaceNodeConfig surfaceNodeConfig;
    surfaceNodeConfig.SurfaceNodeName = "UpdateProperty20";
    surfaceNode = RSSurfaceNode::Create(surfaceNodeConfig, RSSurfaceNodeType::DEFAULT);
    ASSERT_EQ(WMError::WM_OK,
        windowController_->CreateWindow(window, property, surfaceNode, windowId, nullptr, 0, 0));
    ASSERT_NE(nullptr, windowRoot_->GetWindowNode(windowId));
    ASSERT_NE(nullptr, property);
    PropertyChangeAction action = PropertyChangeAction::ACTION_UPDATE_MAXIMIZE_STATE;
    WMError res = windowController_->UpdateProperty(property, action);
    ASSERT_EQ(WMError::WM_ERROR_INVALID_OPERATION, res);

    windowNode->SetWindowMode(WindowMode::WINDOW_MODE_FLOATING);
    res = windowController_->UpdateProperty(property, action);
    ASSERT_EQ(WMError::WM_ERROR_INVALID_DISPLAY, res);

    property->SetWindowSizeChangeReason(WindowSizeChangeReason::MOVE);
    property->SetWindowType(WindowType::WINDOW_TYPE_DOCK_SLICE);
    res = windowController_->UpdateProperty(property, action);
    ASSERT_EQ(WMError::WM_ERROR_INVALID_DISPLAY, res);

    property->SetWindowType(WindowType::WINDOW_TYPE_INPUT_METHOD_FLOAT);
    res = windowController_->UpdateProperty(property, action);
    ASSERT_EQ(WMError::WM_ERROR_INVALID_DISPLAY, res);

    property->SetWindowSizeChangeReason(WindowSizeChangeReason::RESIZE);
    res = windowController_->UpdateProperty(property, action);
    ASSERT_EQ(WMError::WM_ERROR_INVALID_DISPLAY, res);

    property->SetWindowSizeChangeReason(WindowSizeChangeReason::DRAG);
    res = windowController_->UpdateProperty(property, action);
    ASSERT_EQ(WMError::WM_ERROR_INVALID_DISPLAY, res);

    property->SetWindowSizeChangeReason(WindowSizeChangeReason::MAXIMIZE);
    res = windowController_->UpdateProperty(property, action);
    ASSERT_EQ(WMError::WM_ERROR_INVALID_DISPLAY, res);
}

/**
 * @tc.name: MinimizeWindowsByLauncher
 * @tc.desc: Window controller MinimizeWindowsByLauncher
 * @tc.type: FUNC
 */
HWTEST_F(WindowControllerTest, MinimizeWindowsByLauncher, TestSize.Level1)
{
    windowRoot_->windowNodeMap_.clear();
    sptr<IWindow> window;
    sptr<WindowProperty> property = new WindowProperty();
    std::shared_ptr<RSSurfaceNode> surfaceNode = nullptr;
    sptr<WindowNode> windowNode = new WindowNode(property);
    property->SetParentId(INVALID_WINDOW_ID);
    property->SetWindowType(WindowType::APP_WINDOW_BASE);
    windowRoot_->windowNodeMap_.insert(std::make_pair(0, windowNode));

    uint32_t windowId = windowNode->GetWindowId();
    struct RSSurfaceNodeConfig surfaceNodeConfig;
    surfaceNodeConfig.SurfaceNodeName = "MinimizeWindowsByLauncher";
    surfaceNode = RSSurfaceNode::Create(surfaceNodeConfig, RSSurfaceNodeType::DEFAULT);

    std::vector<uint32_t> windowIds;
    windowIds.push_back(windowId);
    bool isAnimated = true;
    sptr<RSIWindowAnimationFinishedCallback> finishCallback;
    windowController_->MinimizeWindowsByLauncher(windowIds, isAnimated, finishCallback);
    isAnimated = false;
    windowController_->MinimizeWindowsByLauncher(windowIds, isAnimated, finishCallback);
    ASSERT_EQ(WMError::WM_OK,
        windowController_->CreateWindow(window, property, surfaceNode, windowId, nullptr, 0, 0));
}

/**
 * @tc.name: OnScreenshot
 * @tc.desc: Window controller OnScreenshot
 * @tc.type: FUNC
 */
HWTEST_F(WindowControllerTest, OnScreenshot, TestSize.Level1)
{
    windowRoot_->windowNodeMap_.clear();
    sptr<IWindow> window;
    sptr<WindowProperty> property = new WindowProperty();
    std::shared_ptr<RSSurfaceNode> surfaceNode = nullptr;
    sptr<WindowNode> windowNode = new WindowNode(property);
    property->SetParentId(INVALID_WINDOW_ID);
    property->SetWindowType(WindowType::APP_WINDOW_BASE);
    windowRoot_->windowNodeMap_.insert(std::make_pair(0, windowNode));

    uint32_t windowId = windowNode->GetWindowId();
    struct RSSurfaceNodeConfig surfaceNodeConfig;
    surfaceNodeConfig.SurfaceNodeName = "OnScreenshot";
    surfaceNode = RSSurfaceNode::Create(surfaceNodeConfig, RSSurfaceNodeType::DEFAULT);

    DisplayId displayId = static_cast<DisplayId>(windowId);
    windowController_->OnScreenshot(10);
    windowController_->OnScreenshot(displayId);
    ASSERT_EQ(WMError::WM_OK,
        windowController_->CreateWindow(window, property, surfaceNode, windowId, nullptr, 0, 0));
}
}
}
}
