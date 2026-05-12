/*
 * Copyright (c) 2022-2022 Huawei Device Co., Ltd.
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

#include <fcntl.h>
#include <gtest/gtest.h>
#include "common_test_utils.h"
#include "iremote_object_mocker.h"
#include "mock_IWindow.h"
#include "mock_RSIWindowAnimationController.h"
#include "window_manager_service.h"

#include <thread>

#include <ability_manager_client.h>
#include <cinttypes>
#include <chrono>
#include <hisysevent.h>
#include <hitrace_meter.h>
#include <ipc_skeleton.h>
#include <parameters.h>
#include <rs_iwindow_animation_controller.h>
#include <system_ability_definition.h>
#include <sstream>
#include "xcollie/watchdog.h"

#include "color_parser.h"
#include "display_manager.h"
#include "display_manager_service_inner.h"
#include "dm_common.h"
#include "drag_controller.h"
#include "minimize_app.h"
#include "permission.h"
#include "remote_animation.h"
#include "singleton_container.h"
#include "ui/rs_ui_director.h"
#include "window_helper.h"
#include "window_inner_manager.h"
#include "window_manager_agent_controller.h"
#include "window_manager_hilog.h"
#include "wm_common.h"
#include "wm_math.h"
#include "scene_board_judgement.h"


using namespace testing;
using namespace testing::ext;
namespace OHOS {
namespace Rosen {
class WindowManagerServiceTest : public testing::Test {
public:
    static void SetUpTestCase();
    static void TearDownTestCase();
    void SetUp() override;
    void TearDown() override;

    void SetAceessTokenPermission(const std::string processName);
    sptr<WindowManagerService> wms = new WindowManagerService();
    sptr<WindowManagerServiceHandler> wmsHandler_ = new WindowManagerServiceHandler();
    sptr<IDisplayChangeListener> listener = new DisplayChangeListener();
    sptr<IWindowInfoQueriedListener> windowInfoQueriedListener = new WindowInfoQueriedListener();
};

void WindowManagerServiceTest::SetUpTestCase()
{
}

void WindowManagerServiceTest::TearDownTestCase()
{
}

void WindowManagerServiceTest::SetUp()
{
    CommonTestUtils::SetAceessTokenPermission("WindowManagerServiceTest");
}

void WindowManagerServiceTest::TearDown()
{
}

namespace {
/**
 * @tc.name: OnAddSystemAbility
 * @tc.desc: OnAddSystemAbility test
 * @tc.type: FUNC
 */
HWTEST_F(WindowManagerServiceTest, OnAddSystemAbility01, TestSize.Level1)
{
    std::string str = "OnAddSystemAbility";
    wms->OnAddSystemAbility(0, str);
    ASSERT_EQ(nullptr, wms->windowCommonEvent_->subscriber_);
}

/**
 * @tc.name: WindowVisibilityChangeCallback
 * @tc.desc: WindowVisibilityChangeCallback test
 * @tc.type: FUNC
 */
HWTEST_F(WindowManagerServiceTest, WindowVisibilityChangeCallback01, TestSize.Level1)
{
    std::shared_ptr<RSOcclusionData> occlusionData = nullptr;
    wms->WindowVisibilityChangeCallback(occlusionData);
    ASSERT_EQ(nullptr, occlusionData);
    int time = 10000;
    std::chrono::milliseconds dura(time);
    std::this_thread::sleep_for(dura);
}

/**
 * @tc.name: InitWithAbilityManagerServiceAdded
 * @tc.desc: Init with ability manager service added.
 * @tc.type: FUNC
 */
HWTEST_F(WindowManagerServiceTest, InitWithAbilityManagerServiceAdded01, TestSize.Level1)
{
    wms->InitWithAbilityManagerServiceAdded();
    wms->wmsHandler_ = new WindowManagerServiceHandler;
    wms->InitWithAbilityManagerServiceAdded();
    ASSERT_NE(nullptr, wms->wmsHandler_);
}

/**
 * @tc.name: Dump
 * @tc.desc: Dump info
 * @tc.type: FUNC
 */
HWTEST_F(WindowManagerServiceTest, Dump01, TestSize.Level1)
{
    wms->windowDumper_ = nullptr;
    std::vector<std::u16string> args;
    const std::string dumpFile = "data/window_dump_test.txt";
    int fd = open(dumpFile.c_str(), O_RDWR | O_CREAT | O_TRUNC, 0666);
    ASSERT_NE(fd, -1);
    if (!SceneBoardJudgement::IsSceneBoardEnabled()) {
        ASSERT_EQ(static_cast<int>(WMError::WM_OK), wms->Dump(-1, args));
    } else {
        ASSERT_NE(static_cast<int>(WMError::WM_ERROR_INVALID_PARAM), wms->Dump(-1, args));
    }
    ASSERT_EQ(static_cast<int>(WMError::WM_OK), wms->Dump(fd, args));
    close(fd);
    unlink(dumpFile.c_str());
}

/**
 * @tc.name: NotifyWindowTransition
 * @tc.desc: NotifyWindowTransition test
 * @tc.type: FUNC
 */
HWTEST_F(WindowManagerServiceTest, NotifyWindowTransition01, TestSize.Level1)
{
    sptr<WindowTransitionInfo> fromInfo = nullptr;
    sptr<WindowTransitionInfo> toInfo = nullptr;
    ASSERT_EQ(WMError::WM_OK, wms->NotifyWindowTransition(fromInfo, toInfo, false));
    if (!SceneBoardJudgement::IsSceneBoardEnabled()) {
        ASSERT_EQ(WMError::WM_OK, wms->NotifyWindowTransition(fromInfo, toInfo, true));
    } else {
        ASSERT_NE(WMError::WM_ERROR_NO_REMOTE_ANIMATION, wms->NotifyWindowTransition(fromInfo, toInfo, true));
    }
}

/**
 * @tc.name: StartingWindow
 * @tc.desc: StartingWindow test
 * @tc.type: FUNC
 */
HWTEST_F(WindowManagerServiceTest, StartingWindow01, TestSize.Level1)
{
    wms->startingOpen_ = false;
    wms->StartingWindow(nullptr, nullptr, false, 0);
    ASSERT_EQ(false, wms->startingOpen_);
    wms->CancelStartingWindow(nullptr);
    wms->startingOpen_ = true;
    wms->StartingWindow(nullptr, nullptr, false, 0);
    ASSERT_EQ(true, wms->startingOpen_);
    wms->CancelStartingWindow(nullptr);
}

/**
 * @tc.name: MoveMissionsToForeground
 * @tc.desc: MoveMissionsToForont test
 * @tc.type: FUNC
 */
HWTEST_F(WindowManagerServiceTest, MoveMissionsToForeground01, TestSize.Level1)
{
    WMError rs = wms->MoveMissionsToForeground({}, -1);
    ASSERT_EQ(WMError::WM_OK, rs);
}

/**
 * @tc.name: MoveMissionsToBackground
 * @tc.desc: MoveMissionsToBackground test
 * @tc.type: FUNC
 */
HWTEST_F(WindowManagerServiceTest, MoveMissionsToBackground01, TestSize.Level1)
{
    std::vector<int32_t> moveRs;
    WMError rs = wms->MoveMissionsToBackground({}, moveRs);
    ASSERT_EQ(WMError::WM_OK, rs);
}

/**
 * @tc.name: CreateWindow
 * @tc.desc: CreateWindow test
 * @tc.type: FUNC
 */
HWTEST_F(WindowManagerServiceTest, CreateWindow01, TestSize.Level1)
{
    sptr<IWindow> window = nullptr;
    uint32_t id = 2;
    std::shared_ptr<RSSurfaceNode> RS_node = nullptr;
    sptr<WindowProperty> property = new WindowProperty();
    property->SetWindowType(WindowType::WINDOW_TYPE_WALLPAPER);
    ASSERT_EQ(WMError::WM_ERROR_NULLPTR, wms->CreateWindow(window, property, RS_node, id, nullptr));
    wms->DestroyWindow(id, true);
}

/**
 * @tc.name: AddWindow
 * @tc.desc: AddWindow test
 * @tc.type: FUNC
 */
HWTEST_F(WindowManagerServiceTest, AddWindow01, TestSize.Level1)
{
    sptr<WindowProperty> property = nullptr;
    ASSERT_EQ(WMError::WM_ERROR_NULLPTR, wms->AddWindow(property));
}

/**
 * @tc.name: RegisterWindowManagerAgent
 * @tc.desc: RegisterWindowManagerAgent test
 * @tc.type: FUNC
 */
HWTEST_F(WindowManagerServiceTest, RegisterWindowManagerAgent01, TestSize.Level1)
{
    sptr<IWindowManagerAgent> windowManagerAgent = nullptr;
    WindowManagerAgentType type = WindowManagerAgentType::WINDOW_MANAGER_AGENT_TYPE_CAMERA_FLOAT;
    ASSERT_EQ(WMError::WM_ERROR_NULLPTR, wms->RegisterWindowManagerAgent(type, windowManagerAgent));
    ASSERT_EQ(WMError::WM_ERROR_NULLPTR, wms->UnregisterWindowManagerAgent(type, windowManagerAgent));
}

/**
 * @tc.name: SetWindowAnimationController
 * @tc.desc: SetWindowAnimationController test
 * @tc.type: FUNC
 */
HWTEST_F(WindowManagerServiceTest, SetWindowAnimationController01, TestSize.Level1)
{
    sptr<RSIWindowAnimationController> controller = nullptr;
    ASSERT_EQ(WMError::WM_ERROR_NULLPTR, wms->SetWindowAnimationController(controller));
    controller = new RSIWindowAnimationControllerMocker;
    ASSERT_EQ(WMError::WM_OK, wms->SetWindowAnimationController(controller));
}

/**
 * @tc.name: OnWindowEvent
 * @tc.desc: OnWindowEvent test
 * @tc.type: FUNC
 */
HWTEST_F(WindowManagerServiceTest, OnWindowEvent01, TestSize.Level1)
{
    sptr<IRemoteObject> remoteObject = new IRemoteObjectMocker;
    wms->OnWindowEvent(static_cast<Event>(1), remoteObject);
    wms->OnWindowEvent(Event::REMOTE_DIED, remoteObject);
    ASSERT_EQ(INVALID_WINDOW_ID, wms->windowRoot_->GetWindowIdByObject(remoteObject));
}

/**
 * @tc.name: UpdateProperty
 * @tc.desc: UpdateProperty test
 * @tc.type: FUNC
 */
HWTEST_F(WindowManagerServiceTest, UpdateProperty01, TestSize.Level1)
{
    sptr<WindowProperty> windowProperty = nullptr;
    ASSERT_EQ(WMError::WM_ERROR_NULLPTR, wms->UpdateProperty(windowProperty,
        PropertyChangeAction::ACTION_UPDATE_ANIMATION_FLAG, true));

    windowProperty = sptr<WindowProperty>::MakeSptr();
    EXPECT_EQ(WMError::WM_ERROR_INVALID_PERMISSION, wms->UpdateProperty(windowProperty,
        PropertyChangeAction::ACTION_UPDATE_TRANSFORM_PROPERTY, true));
    EXPECT_EQ(WMError::WM_ERROR_INVALID_PERMISSION, wms->UpdateProperty(windowProperty,
        PropertyChangeAction::ACTION_UPDATE_SNAPSHOT_SKIP, true));
}

/**
 * @tc.name: GetModeChangeHotZones
 * @tc.desc: GetModeChangeHotZones test
 * @tc.type: FUNC
 */
HWTEST_F(WindowManagerServiceTest, GetModeChangeHotZones01, TestSize.Level1)
{
    ModeChangeHotZonesConfig config = {false, 0, 0, 0};
    DisplayId displayId = 0;
    ModeChangeHotZones hotZone;
    wms->hotZonesConfig_ = config;
    ASSERT_EQ(WMError::WM_DO_NOTHING, wms->GetModeChangeHotZones(displayId, hotZone));
    config.isModeChangeHotZoneConfigured_ = true;
    wms->hotZonesConfig_ = config;
    ASSERT_EQ(WMError::WM_ERROR_NULLPTR, wms->GetModeChangeHotZones(displayId, hotZone));
}

/**
 * @tc.name: UpdateAvoidAreaListener
 * @tc.desc: UpdateAvoidAreaListener test
 * @tc.type: FUNC
 */
HWTEST_F(WindowManagerServiceTest, UpdateAvoidAreaListener01, TestSize.Level1)
{
    sptr<WindowProperty> property = new WindowProperty();
    sptr<WindowNode> node = new WindowNode(property);
    wms->windowRoot_->windowNodeMap_.insert(std::make_pair(0, node));
    if (!SceneBoardJudgement::IsSceneBoardEnabled()) {
        ASSERT_EQ(WMError::WM_OK, wms->UpdateAvoidAreaListener(0, true));
    } else {
        ASSERT_NE(WMError::WM_DO_NOTHING, wms->UpdateAvoidAreaListener(0, true));
    }
}

/**
 * @tc.name: BindDialogTarget
 * @tc.desc: BindDialogTarget test
 * @tc.type: FUNC
 */
HWTEST_F(WindowManagerServiceTest, BindDialogTarget01, TestSize.Level1)
{
    sptr<IRemoteObject> targetToken = new IRemoteObjectMocker();
    uint32_t id = 0;
    if (!SceneBoardJudgement::IsSceneBoardEnabled()) {
        ASSERT_EQ(WMError::WM_OK, wms->BindDialogTarget(id, targetToken));
    } else {
        ASSERT_NE(WMError::WM_ERROR_NULLPTR, wms->BindDialogTarget(id, targetToken));
    }
}

/**
 * @tc.name: DispatchKeyEvent01
 * @tc.desc: Dispatch KeyEvent for app window bellow the component window.
 * @tc.require: issueI6RMUY
 * @tc.type: FUNC
 */
HWTEST_F(WindowManagerServiceTest, DispatchKeyEvent01, TestSize.Level1)
{
    wms = new WindowManagerService();
    sptr<WindowNode> compNode = new WindowNode();
    compNode->property_->type_ = WindowType::WINDOW_TYPE_APP_COMPONENT;
    compNode->SetWindowToken(new IWindowMocker);
    compNode->property_->windowId_ = 1U;
    sptr<WindowNode> appNode = new WindowNode();
    appNode->property_->type_ = WindowType::WINDOW_TYPE_APP_MAIN_WINDOW;
    IWindowMocker* token = new IWindowMocker;
    appNode->SetWindowToken(token);
    appNode->property_->windowId_ = 2U;
    auto event = MMI::KeyEvent::Create();
    wms->windowController_->windowRoot_->windowNodeMap_[appNode->GetWindowId()] = appNode;
    wms->windowController_->windowRoot_->windowNodeMap_[compNode->GetWindowId()] = compNode;
    sptr<WindowNodeContainer> container = new WindowNodeContainer(new DisplayInfo, 0);
    wms->windowController_->windowRoot_->windowNodeContainerMap_[compNode->GetDisplayId()] = container;
    wms->windowController_->windowRoot_->displayIdMap_[compNode->GetDisplayId()] = { compNode->GetDisplayId() };
    container->appWindowNode_->children_.push_back(appNode);
    container->appWindowNode_->children_.push_back(compNode);
    std::vector<sptr<WindowNode>> windowNodes;
    container->TraverseContainer(windowNodes);
    ASSERT_TRUE(windowNodes[0] == compNode);
    ASSERT_TRUE(windowNodes[1] == appNode);
    EXPECT_CALL(*token, ConsumeKeyEvent(_));
    wms->DispatchKeyEvent(compNode->GetWindowId(), event);
    testing::Mock::AllowLeak(token);
}

/**
 * @tc.name: DispatchKeyEvent02
 * @tc.desc: Dispatch KeyEvent for app window bellow the app window.
 * @tc.require: issueI6RMUY
 * @tc.type: FUNC
 */
HWTEST_F(WindowManagerServiceTest, DispatchKeyEvent02, TestSize.Level1)
{
    wms = new WindowManagerService();
    sptr<WindowNode> appNode1 = new WindowNode();
    appNode1->property_->type_ = WindowType::WINDOW_TYPE_APP_MAIN_WINDOW;
    appNode1->SetWindowToken(new IWindowMocker);
    appNode1->property_->windowId_ = 1U;
    sptr<WindowNode> appNode2 = new WindowNode();
    appNode2->property_->type_ = WindowType::WINDOW_TYPE_APP_MAIN_WINDOW;
    appNode2->SetWindowToken(new IWindowMocker);
    appNode2->property_->windowId_ = 2U;
    auto event = MMI::KeyEvent::Create();
    wms->windowController_->windowRoot_->windowNodeMap_[appNode2->GetWindowId()] = appNode2;
    wms->windowController_->windowRoot_->windowNodeMap_[appNode1->GetWindowId()] = appNode1;
    sptr<WindowNodeContainer> container = new WindowNodeContainer(new DisplayInfo, 0);
    wms->windowController_->windowRoot_->windowNodeContainerMap_[appNode1->GetDisplayId()] = container;
    wms->windowController_->windowRoot_->displayIdMap_[appNode1->GetDisplayId()] = { appNode1->GetDisplayId() };
    container->appWindowNode_->children_.push_back(appNode2);
    container->appWindowNode_->children_.push_back(appNode1);
    std::vector<sptr<WindowNode>> windowNodes;
    container->TraverseContainer(windowNodes);
    ASSERT_TRUE(windowNodes[0] == appNode1);
    ASSERT_TRUE(windowNodes[1] == appNode2);
    wms->DispatchKeyEvent(appNode1->GetWindowId(), event);
}

/**
 * @tc.name: DispatchKeyEvent03
 * @tc.desc: Dispatch KeyEvent for app window bellow two component window.
 * @tc.require: issueI6RMUY
 * @tc.type: FUNC
 */
HWTEST_F(WindowManagerServiceTest, DispatchKeyEvent03, TestSize.Level1)
{
    wms = new WindowManagerService();
    sptr<WindowNode> compNode1 = new WindowNode();
    compNode1->property_->type_ = WindowType::WINDOW_TYPE_APP_COMPONENT;
    compNode1->SetWindowToken(new IWindowMocker);
    compNode1->property_->windowId_ = 1U;
    sptr<WindowNode> compNode2 = new WindowNode();
    compNode2->property_->type_ = WindowType::WINDOW_TYPE_APP_COMPONENT;
    compNode2->SetWindowToken(new IWindowMocker);
    compNode2->property_->windowId_ = 2U;
    sptr<WindowNode> appNode = new WindowNode();
    appNode->property_->type_ = WindowType::WINDOW_TYPE_APP_MAIN_WINDOW;
    IWindowMocker* token = new IWindowMocker;
    appNode->SetWindowToken(token);
    appNode->property_->windowId_ = 3U;
    auto event = MMI::KeyEvent::Create();
    wms->windowController_->windowRoot_->windowNodeMap_[appNode->GetWindowId()] = appNode;
    wms->windowController_->windowRoot_->windowNodeMap_[compNode1->GetWindowId()] = compNode1;
    wms->windowController_->windowRoot_->windowNodeMap_[compNode2->GetWindowId()] = compNode2;
    sptr<WindowNodeContainer> container = new WindowNodeContainer(new DisplayInfo, 0);
    wms->windowController_->windowRoot_->windowNodeContainerMap_[compNode1->GetDisplayId()] = container;
    wms->windowController_->windowRoot_->displayIdMap_[compNode1->GetDisplayId()] = { compNode1->GetDisplayId() };
    container->appWindowNode_->children_.push_back(appNode);
    container->appWindowNode_->children_.push_back(compNode1);
    container->appWindowNode_->children_.push_back(compNode2);
    std::vector<sptr<WindowNode>> windowNodes;
    container->TraverseContainer(windowNodes);
    ASSERT_TRUE(windowNodes[0] == compNode2);
    ASSERT_TRUE(windowNodes[1] == compNode1);
    ASSERT_TRUE(windowNodes[2] == appNode);
    EXPECT_CALL(*token, ConsumeKeyEvent(_));
    wms->DispatchKeyEvent(compNode2->GetWindowId(), event);
    testing::Mock::AllowLeak(token);
}

/**
 * @tc.name: SetWindowGravity
 * @tc.desc: SetWindowGravity test
 * @tc.type: FUNC
 */
HWTEST_F(WindowManagerServiceTest, SetWindowGravity01, TestSize.Level1)
{
    uint32_t id = 0;
    if (!SceneBoardJudgement::IsSceneBoardEnabled()) {
        ASSERT_EQ(WMError::WM_OK, wms->SetWindowGravity(id, WindowGravity::WINDOW_GRAVITY_BOTTOM, 0));
    } else {
        ASSERT_NE(WMError::WM_ERROR_NULLPTR, wms->SetWindowGravity(id, WindowGravity::WINDOW_GRAVITY_BOTTOM, 0));
    }
}

/**
 * @tc.name: GetWindowAnimationTargets01
 * @tc.desc: get window animation targets
 * @tc.type: FUNC
 */
HWTEST_F(WindowManagerServiceTest, GetWindowAnimationTargets01, TestSize.Level1)
{
    wms = new WindowManagerService();
    sptr<WindowNode> compNode = new WindowNode();
    compNode->property_->type_ = WindowType::WINDOW_TYPE_APP_COMPONENT;
    compNode->SetWindowToken(new IWindowMocker);
    compNode->property_->windowId_ = 1U;
    compNode->abilityInfo_.missionId_ = 1;
    sptr<WindowNode> appNode = new WindowNode();
    appNode->property_->type_ = WindowType::WINDOW_TYPE_APP_MAIN_WINDOW;
    IWindowMocker* token = new IWindowMocker;
    appNode->SetWindowToken(token);
    appNode->property_->windowId_ = 2U;
    appNode->abilityInfo_.missionId_ = 2;
    wms->windowController_->windowRoot_->windowNodeMap_[appNode->GetWindowId()] = appNode;
    wms->windowController_->windowRoot_->windowNodeMap_[compNode->GetWindowId()] = compNode;
    sptr<WindowNodeContainer> container = new WindowNodeContainer(new DisplayInfo, 0);
    wms->windowController_->windowRoot_->windowNodeContainerMap_[compNode->GetDisplayId()] = container;
    wms->windowController_->windowRoot_->displayIdMap_[compNode->GetDisplayId()] = { compNode->GetDisplayId() };
    container->appWindowNode_->children_.push_back(appNode);
    container->appWindowNode_->children_.push_back(compNode);
    RemoteAnimation::SetWindowControllerAndRoot(wms->windowController_, wms->windowController_->windowRoot_);
    std::vector<uint32_t> missionIds;
    missionIds.push_back(1);
    missionIds.push_back(2);
    std::vector<sptr<RSWindowAnimationTarget>> targets;
    ASSERT_EQ(WMError::WM_OK, wms->GetWindowAnimationTargets(missionIds, targets));
    ASSERT_EQ(0, targets.size());
}

/**
 * @tc.name: OnAccountSwitched
 * @tc.desc: OnAccountSwitched test
 * @tc.type: FUNC
 */
HWTEST_F(WindowManagerServiceTest, OnAccountSwitched, TestSize.Level1)
{
    int accountId = 0;
    ASSERT_TRUE(wms != nullptr);
    wms->OnAccountSwitched(accountId);
    int time = 10000;
    std::chrono::milliseconds dura(time);
    std::this_thread::sleep_for(dura);
}

/**
 * @tc.name: InitWithRanderServiceAdded
 * @tc.desc: InitWithRanderServiceAdded test
 * @tc.type: FUNC
 */
HWTEST_F(WindowManagerServiceTest, InitWithRanderServiceAdded, TestSize.Level1)
{
    ASSERT_TRUE(wms != nullptr);
    wms->InitWithRanderServiceAdded();
}

/**
 * @tc.name: NotifyWindowTransition02
 * @tc.desc: NotifyWindowTransition02 test
 * @tc.type: FUNC
 */
HWTEST_F(WindowManagerServiceTest, NotifyWindowTransition02, TestSize.Level1)
{
    sptr<AAFwk::AbilityTransitionInfo> from = new AAFwk::AbilityTransitionInfo();
    sptr<AAFwk::AbilityTransitionInfo> to = new AAFwk::AbilityTransitionInfo();
    bool animaEnabled = false;
    ASSERT_TRUE(wmsHandler_ != nullptr);
    wmsHandler_->NotifyWindowTransition(from, to, animaEnabled);
}

/**
 * @tc.name: NotifyAnimationAbilityDied
 * @tc.desc: NotifyAnimationAbilityDied test
 * @tc.type: FUNC
 */
HWTEST_F(WindowManagerServiceTest, NotifyAnimationAbilityDied, TestSize.Level1)
{
    sptr<AAFwk::AbilityTransitionInfo> info = new AAFwk::AbilityTransitionInfo();
    ASSERT_TRUE(wmsHandler_ != nullptr);
    wmsHandler_->NotifyAnimationAbilityDied(info);
}

/**
 * @tc.name: StartingWindow02
 * @tc.desc: StartingWindow02 test
 * @tc.type: FUNC
 */
HWTEST_F(WindowManagerServiceTest, StartingWindow02, TestSize.Level1)
{
    sptr<AAFwk::AbilityTransitionInfo> info = new AAFwk::AbilityTransitionInfo();
    std::shared_ptr<Media::PixelMap> pixelMap = std::make_shared<Media::PixelMap>();
    ASSERT_TRUE(wmsHandler_ != nullptr);
    wmsHandler_->StartingWindow(info, pixelMap);
}

/**
 * @tc.name: StartingWindow03
 * @tc.desc: StartingWindow03 test
 * @tc.type: FUNC
 */
HWTEST_F(WindowManagerServiceTest, StartingWindow03, TestSize.Level1)
{
    sptr<AAFwk::AbilityTransitionInfo> info = new AAFwk::AbilityTransitionInfo();
    std::shared_ptr<Media::PixelMap> pixelMap = std::make_shared<Media::PixelMap>();
    uint32_t bgColor = 1;
    ASSERT_TRUE(wmsHandler_ != nullptr);
    wmsHandler_->StartingWindow(info, pixelMap, bgColor);
}

/**
 * @tc.name: CancelStartingWindow01
 * @tc.desc: CancelStartingWindow test
 * @tc.type: FUNC
 */
HWTEST_F(WindowManagerServiceTest, CancelStartingWindow01, TestSize.Level1)
{
    sptr<IRemoteObject> abilityToken = nullptr;
    wms->startingOpen_ = false;
    ASSERT_TRUE(wms != nullptr);
    wmsHandler_->CancelStartingWindow(abilityToken);
}

/**
 * @tc.name: MoveMissionsToForeground02
 * @tc.desc: MoveMissionsToForeground test
 * @tc.type: FUNC
 */
HWTEST_F(WindowManagerServiceTest, MoveMissionsToForeground02, TestSize.Level1)
{
    const std::vector<int32_t> missionIds;
    int32_t topMissionId = 1;
    wms->windowGroupMgr_ = nullptr;
    ASSERT_TRUE(wms != nullptr);
    wmsHandler_->MoveMissionsToForeground(missionIds, topMissionId);
}

/**
 * @tc.name: Init
 * @tc.desc: Init test
 * @tc.type: FUNC
 */
HWTEST_F(WindowManagerServiceTest, Init, TestSize.Level1)
{
    wms->windowRoot_ = new WindowRoot(nullptr);
    wms->Init();
    ASSERT_NE(wms->windowRoot_, nullptr);
}

/**
 * @tc.name: Dump02
 * @tc.desc: Dump02 test
 * @tc.type: FUNC
 */
HWTEST_F(WindowManagerServiceTest, Dump02, TestSize.Level1)
{
    int fd = 2;
    std::vector<std::u16string> args;
    wms->Dump(fd, args);
    wms->windowDumper_ = nullptr;
    ASSERT_TRUE(wms != nullptr);
    wms->Dump(fd, args);
}

/**
 * @tc.name: ConfigStartingWindowAnimation
 * @tc.desc: ConfigStartingWindowAnimation test
 * @tc.type: FUNC
 */
HWTEST_F(WindowManagerServiceTest, ConfigStartingWindowAnimation, TestSize.Level1)
{
    WindowManagerConfig::ConfigItem animeConfig;
    ASSERT_TRUE(wms != nullptr);
    wms->ConfigStartingWindowAnimation(animeConfig);
}

/**
 * @tc.name: RequestFocus
 * @tc.desc: RequestFocus test
 * @tc.type: FUNC
 */
HWTEST_F(WindowManagerServiceTest, RequestFocus, TestSize.Level1)
{
    uint32_t windowId = 1;
    WMError res = wms->RequestFocus(windowId);
    EXPECT_EQ(res, WMError::WM_ERROR_INVALID_OPERATION);
}

/**
 * @tc.name: RequestFocus01
 * @tc.desc: RequestFocus with valid permission test
 * @tc.type: FUNC
 */
HWTEST_F(WindowManagerServiceTest, RequestFocus01, TestSize.Level1)
{
    uint32_t windowId = 1;
    wms->accessTokenIdMaps_.insert(windowId, IPCSkeleton::GetCallingTokenID());
    WMError res = wms->RequestFocus(windowId);
    if (!SceneBoardJudgement::IsSceneBoardEnabled()) {
        EXPECT_NE(res, WMError::WM_OK);
    } else {
        EXPECT_EQ(res, WMError::WM_OK);
    }
    wms->accessTokenIdMaps_.clear();
}

/**
 * @tc.name: GetAvoidAreaByType
 * @tc.desc: GetAvoidAreaByType test
 * @tc.type: FUNC
 */
HWTEST_F(WindowManagerServiceTest, GetAvoidAreaByType, TestSize.Level1)
{
    uint32_t windowId = 1;
    AvoidAreaType avoidAreaType = AvoidAreaType::TYPE_SYSTEM;
    ASSERT_TRUE(wms != nullptr);
    wms->GetAvoidAreaByType(windowId, avoidAreaType);
}

/**
 * @tc.name: NotifyDisplayStateChange
 * @tc.desc: NotifyDisplayStateChange test
 * @tc.type: FUNC
 */
HWTEST_F(WindowManagerServiceTest, NotifyDisplayStateChange, TestSize.Level1)
{
    DisplayId defaultDisplayId = DisplayGroupInfo::GetInstance().GetDefaultDisplayId();
    auto display = DisplayManager::GetInstance().GetDefaultDisplay();
    sptr<DisplayInfo> displayInfo = display->GetDisplayInfo();
    std::map<DisplayId, sptr<DisplayInfo>> displayInfoMap;
    DisplayStateChangeType type = DisplayStateChangeType::BEFORE_SUSPEND;
    ASSERT_TRUE(wms != nullptr);
    wms->NotifyDisplayStateChange(defaultDisplayId, displayInfo, displayInfoMap, type);
}

/**
 * @tc.name: OnDisplayStateChange
 * @tc.desc: OnDisplayStateChange test
 * @tc.type: FUNC
 */
HWTEST_F(WindowManagerServiceTest, OnDisplayStateChange, TestSize.Level1)
{
    DisplayId defaultDisplayId = DisplayGroupInfo::GetInstance().GetDefaultDisplayId();
    auto display = DisplayManager::GetInstance().GetDefaultDisplay();
    sptr<DisplayInfo> displayInfo = display->GetDisplayInfo();
    std::map<DisplayId, sptr<DisplayInfo>> displayInfoMap;
    DisplayStateChangeType type = DisplayStateChangeType::BEFORE_SUSPEND;
    ASSERT_TRUE(listener != nullptr);
    listener->OnDisplayStateChange(defaultDisplayId, displayInfo, displayInfoMap, type);
}

/**
 * @tc.name: NotifyServerReadyToMoveOrDrag
 * @tc.desc: NotifyServerReadyToMoveOrDrag test
 * @tc.type: FUNC
 */
HWTEST_F(WindowManagerServiceTest, NotifyServerReadyToMoveOrDrag, TestSize.Level1)
{
    uint32_t windowId = 1;
    sptr<WindowProperty> windowProperty;
    sptr<MoveDragProperty> moveDragProperty;
    ASSERT_TRUE(wms != nullptr);
    wms->NotifyServerReadyToMoveOrDrag(windowId, windowProperty, moveDragProperty);
}

/**
 * @tc.name: ProcessPointDown
 * @tc.desc: ProcessPointDown test
 * @tc.type: FUNC
 */
HWTEST_F(WindowManagerServiceTest, ProcessPointDown, TestSize.Level1)
{
    uint32_t windowId = 1;
    bool isPointDown = false;
    ASSERT_TRUE(wms != nullptr);
    wms->ProcessPointDown(windowId, isPointDown);
}

/**
 * @tc.name: MinimizeAllAppWindows
 * @tc.desc: MinimizeAllAppWindows test
 * @tc.type: FUNC
 */
HWTEST_F(WindowManagerServiceTest, MinimizeAllAppWindows, TestSize.Level1)
{
    DisplayId displayId = 1;
    WMError res = wms->MinimizeAllAppWindows(displayId);
    ASSERT_EQ(WMError::WM_OK, res);
}

/**
 * @tc.name: ToggleShownStateForAllAppWindows
 * @tc.desc: ToggleShownStateForAllAppWindows test
 * @tc.type: FUNC
 */
HWTEST_F(WindowManagerServiceTest, ToggleShownStateForAllAppWindows, TestSize.Level1)
{
    WMError res = wms->ToggleShownStateForAllAppWindows();
    ASSERT_EQ(WMError::WM_OK, res);
}

/**
 * @tc.name: GetTopWindowId
 * @tc.desc: GetTopWindowId test
 * @tc.type: FUNC
 */
HWTEST_F(WindowManagerServiceTest, GetTopWindowId, TestSize.Level1)
{
    uint32_t mainWinId = 1;
    uint32_t topWinId = 1;
    WMError res = wms->GetTopWindowId(mainWinId, topWinId);
    if (!SceneBoardJudgement::IsSceneBoardEnabled()) {
        ASSERT_EQ(WMError::WM_OK, res);
    } else {
        ASSERT_NE(WMError::WM_ERROR_INVALID_WINDOW, res);
    }
}

/**
 * @tc.name: SetWindowLayoutMode
 * @tc.desc: SetWindowLayoutMode test
 * @tc.type: FUNC
 */
HWTEST_F(WindowManagerServiceTest, SetWindowLayoutMode, TestSize.Level1)
{
    WindowLayoutMode mode = WindowLayoutMode::BASE;
    WMError res = wms->SetWindowLayoutMode(mode);
    ASSERT_EQ(WMError::WM_OK, res);
}

/**
 * @tc.name: NotifyScreenshotEvent
 * @tc.desc: NotifyScreenshotEvent test
 * @tc.type: FUNC
 */
HWTEST_F(WindowManagerServiceTest, NotifyScreenshotEvent, TestSize.Level1)
{
    ScreenshotEventType type = ScreenshotEventType::SCROLL_SHOT_START;
    WMError res = wms->NotifyScreenshotEvent(type);
    EXPECT_EQ(WMError::WM_OK, res);
}

/**
 * @tc.name: GetAccessibilityWindowInfo
 * @tc.desc: GetAccessibilityWindowInfo test
 * @tc.type: FUNC
 */
HWTEST_F(WindowManagerServiceTest, GetAccessibilityWindowInfo, TestSize.Level1)
{
    std::vector<sptr<AccessibilityWindowInfo>> infos;
    WMError res = wms->GetAccessibilityWindowInfo(infos);
    ASSERT_EQ(WMError::WM_OK, res);
}

/**
 * @tc.name: GetUnreliableWindowInfo
 * @tc.desc: GetUnreliableWindowInfo test
 * @tc.type: FUNC
 */
HWTEST_F(WindowManagerServiceTest, GetUnreliableWindowInfo, TestSize.Level1)
{
    std::vector<sptr<UnreliableWindowInfo>> infos;
    int32_t windowId = 0;
    WMError res = wms->GetUnreliableWindowInfo(windowId, infos);
    ASSERT_EQ(WMError::WM_OK, res);
}

/**
 * @tc.name: GetVisibilityWindowInfo
 * @tc.desc: GetVisibilityWindowInfo test
 * @tc.type: FUNC
 */
HWTEST_F(WindowManagerServiceTest, GetVisibilityWindowInfo, TestSize.Level1)
{
    std::vector<sptr<WindowVisibilityInfo>> infos;
    WMError res = wms->GetVisibilityWindowInfo(infos);
    ASSERT_EQ(WMError::WM_OK, res);
}

/**
 * @tc.name: RaiseToAppTop
 * @tc.desc: RaiseToAppTop test
 * @tc.type: FUNC
 */
HWTEST_F(WindowManagerServiceTest, RaiseToAppTop, TestSize.Level1)
{
    uint32_t windowId = 1;
    WMError res = wms->RaiseToAppTop(windowId);
    EXPECT_EQ(res, WMError::WM_ERROR_INVALID_OPERATION);
}

/**
 * @tc.name: RaiseToAppTop01
 * @tc.desc: RaiseToAppTop with valid permission test
 * @tc.type: FUNC
 */
HWTEST_F(WindowManagerServiceTest, RaiseToAppTop01, TestSize.Level1)
{
    uint32_t windowId = 1;
    wms->accessTokenIdMaps_.insert(windowId, IPCSkeleton::GetCallingTokenID());
    WMError res = wms->RaiseToAppTop(windowId);
    if (!SceneBoardJudgement::IsSceneBoardEnabled()) {
        EXPECT_EQ(WMError::WM_OK, res);
    } else {
        EXPECT_NE(WMError::WM_DO_NOTHING, res);
    }
    wms->accessTokenIdMaps_.clear();
}

/**
 * @tc.name: GetSnapshot
 * @tc.desc: GetSnapshot test
 * @tc.type: FUNC
 */
HWTEST_F(WindowManagerServiceTest, GetSnapshot, TestSize.Level1)
{
    uint32_t windowId = 1;
    ASSERT_EQ(nullptr, wms->GetSnapshot(windowId));
}

/**
 * @tc.name: MinimizeWindowsByLauncher
 * @tc.desc: MinimizeWindowsByLauncher test
 * @tc.type: FUNC
 */
HWTEST_F(WindowManagerServiceTest, MinimizeWindowsByLauncher, TestSize.Level1)
{
    std::vector<uint32_t> windowIds;
    bool isAnimated = false;
    sptr<RSIWindowAnimationFinishedCallback> finishCallback;
    ASSERT_TRUE(wms != nullptr);
    wms->MinimizeWindowsByLauncher(windowIds, isAnimated, finishCallback);
}

/**
 * @tc.name: SetAnchorAndScale
 * @tc.desc: SetAnchorAndScale test
 * @tc.type: FUNC
 */
HWTEST_F(WindowManagerServiceTest, SetAnchorAndScale, TestSize.Level1)
{
    int32_t x = 1;
    int32_t y = 2;
    float scale = 0.1;
    ASSERT_TRUE(wms != nullptr);
    wms->SetAnchorAndScale(x, y, scale);
}

/**
 * @tc.name: SetAnchorOffset
 * @tc.desc: SetAnchorOffset test
 * @tc.type: FUNC
 */
HWTEST_F(WindowManagerServiceTest, SetAnchorOffset, TestSize.Level1)
{
    int32_t deltaX = 1;
    int32_t deltaY = 2;
    ASSERT_TRUE(wms != nullptr);
    wms->SetAnchorOffset(deltaX, deltaY);
}

/**
 * @tc.name: OffWindowZoom
 * @tc.desc: OffWindowZoom test
 * @tc.type: FUNC
 */
HWTEST_F(WindowManagerServiceTest, OffWindowZoom, TestSize.Level1)
{
    ASSERT_TRUE(wms != nullptr);
    wms->OffWindowZoom();
}

/**
 * @tc.name: UpdateRsTree
 * @tc.desc: UpdateRsTree test
 * @tc.type: FUNC
 */
HWTEST_F(WindowManagerServiceTest, UpdateRsTree, TestSize.Level1)
{
    uint32_t windowId = 1;
    bool isAdd = false;
    ASSERT_TRUE(wms != nullptr);
    wms->UpdateRsTree(windowId, isAdd);
}

/**
 * @tc.name: OnScreenshot
 * @tc.desc: OnScreenshot test
 * @tc.type: FUNC
 */
HWTEST_F(WindowManagerServiceTest, OnScreenshot, TestSize.Level1)
{
    DisplayId displayId = 1;
    ASSERT_TRUE(wms != nullptr);
    wms->OnScreenshot(displayId);
}

/**
 * @tc.name: HasPrivateWindow01
 * @tc.desc: HasPrivateWindow01 test
 * @tc.type: FUNC
 */
HWTEST_F(WindowManagerServiceTest, HasPrivateWindow01, TestSize.Level1)
{
    DisplayId displayId = 1;
    bool hasPrivateWindow = false;
    ASSERT_TRUE(wms != nullptr);
    wms->HasPrivateWindow(displayId, hasPrivateWindow);
}

/**
 * @tc.name: SetGestureNavigationEnabled
 * @tc.desc: SetGestureNavigationEnabled test
 * @tc.type: FUNC
 */
HWTEST_F(WindowManagerServiceTest, SetGestureNavigationEnabled, TestSize.Level1)
{
    bool enable = false;
    ASSERT_TRUE(wms != nullptr);
    WMError res = wms->SetGestureNavigationEnabled(enable);
    ASSERT_EQ(WMError::WM_OK, res);
}

/**
 * @tc.name: HasPrivateWindow02
 * @tc.desc: HasPrivateWindow02 test
 * @tc.type: FUNC
 */
HWTEST_F(WindowManagerServiceTest, HasPrivateWindow02, TestSize.Level1)
{
    DisplayId displayId = 1;
    bool hasPrivateWindow = false;
    ASSERT_TRUE(windowInfoQueriedListener != nullptr);
    windowInfoQueriedListener->HasPrivateWindow(displayId, hasPrivateWindow);
}

/**
 * @tc.name: SetMaximizeMode
 * @tc.desc: SetMaximizeMode test
 * @tc.type: FUNC
 */
HWTEST_F(WindowManagerServiceTest, SetMaximizeMode, TestSize.Level1)
{
    MaximizeMode maximizeMode = MaximizeMode::MODE_AVOID_SYSTEM_BAR;
    ASSERT_TRUE(wms != nullptr);
    wms->SetMaximizeMode(maximizeMode);
}

/**
 * @tc.name: GetMaximizeMode
 * @tc.desc: GetMaximizeMode test
 * @tc.type: FUNC
 */
HWTEST_F(WindowManagerServiceTest, GetMaximizeMode, TestSize.Level1)
{
    ASSERT_TRUE(wms != nullptr);
    ASSERT_EQ(MaximizeMode::MODE_RECOVER, wms->GetMaximizeMode());
}

/**
 * @tc.name: GetFocusWindowInfo
 * @tc.desc: GetFocusWindowInfo test
 * @tc.type: FUNC
 */
HWTEST_F(WindowManagerServiceTest, GetFocusWindowInfo, TestSize.Level1)
{
    FocusChangeInfo focusInfo;
    ASSERT_TRUE(wms != nullptr);
    wms->GetFocusWindowInfo(focusInfo);
}

/**
 * @tc.name: PostAsyncTask
 * @tc.desc: PostAsyncTask test
 * @tc.type: FUNC
 */
HWTEST_F(WindowManagerServiceTest, PostAsyncTask, TestSize.Level1)
{
    Task task;
    std::shared_ptr<AppExecFwk::EventRunner> runner = AppExecFwk::EventRunner::Create("Test");
    wms->handler_ = std::make_shared<AppExecFwk::EventHandler>(runner);
    ASSERT_TRUE(wms != nullptr);
    wms->PostAsyncTask(task);
    wms->PostVoidSyncTask(task);
}

/**
 * @tc.name: OnAddSystemAbility02
 * @tc.desc: OnAddSystemAbility02 test
 * @tc.type: FUNC
 */
HWTEST_F(WindowManagerServiceTest, OnAddSystemAbility02, TestSize.Level1)
{
    std::string str = "OnAddSystemAbility02";
    wms->OnAddSystemAbility(10, str);
    ASSERT_EQ(nullptr, wms->windowCommonEvent_->subscriber_);
    wms->OnAddSystemAbility(180, str);
    ASSERT_EQ(nullptr, wms->windowCommonEvent_->subscriber_);
    wms->OnAddSystemAbility(3299, str);
    ASSERT_NE(nullptr, wms->windowCommonEvent_->subscriber_);
}

/**
 * @tc.name: GetFocusWindow
 * @tc.desc: GetFocusWindow test
 * @tc.type: FUNC
 */
HWTEST_F(WindowManagerServiceTest, GetFocusWindow, TestSize.Level1)
{
    sptr<IRemoteObject> abilityToken = new IRemoteObjectMocker();
    ASSERT_TRUE(wmsHandler_ != nullptr);
    wmsHandler_->GetFocusWindow(abilityToken);
}

/**
 * @tc.name: MoveMissionsToBackground02
 * @tc.desc: MoveMissionsToBackground02 test
 * @tc.type: FUNC
 */
HWTEST_F(WindowManagerServiceTest, MoveMissionsToBackground02, TestSize.Level1)
{
    std::vector<int32_t> moveRs;
    int32_t rs = wmsHandler_->MoveMissionsToBackground({}, moveRs);
    ASSERT_EQ(0, rs);
}

/**
 * @tc.name: ConfigAppWindowCornerRadius
 * @tc.desc: ConfigAppWindowCornerRadius test
 * @tc.type: FUNC
 */
HWTEST_F(WindowManagerServiceTest, ConfigAppWindowCornerRadius, TestSize.Level1)
{
    const auto& config = WindowManagerConfig::GetConfig();
    WindowManagerConfig::ConfigItem item = config["decor"];
    float out = 1.0;
    ASSERT_TRUE(wms != nullptr);
    bool res = wms->ConfigAppWindowCornerRadius(item, out);
    ASSERT_EQ(res, false);
}

/**
 * @tc.name: GetFocusWindowInfo01
 * @tc.desc: GetFocusWindowInfo01 test
 * @tc.type: FUNC
 */
HWTEST_F(WindowManagerServiceTest, GetFocusWindowInfo01, TestSize.Level1)
{
    sptr<IRemoteObject> abilityToken = new IRemoteObjectMocker();
    ASSERT_TRUE(wms != nullptr);
    wms->GetFocusWindowInfo(abilityToken);
}

/**
 * @tc.name: OnStop
 * @tc.desc: OnStop test
 * @tc.type: FUNC
 */
HWTEST_F(WindowManagerServiceTest, OnStop, TestSize.Level1)
{
    ASSERT_TRUE(wms != nullptr);
    wms->OnStop();
}

/**
 * @tc.name: CheckSystemWindowPermission
 * @tc.desc: CheckSystemWindowPermission test
 * @tc.type: FUNC
 */
HWTEST_F(WindowManagerServiceTest, CheckSystemWindowPermission, TestSize.Level1)
{
    sptr<WindowProperty> property = new WindowProperty();
    property->SetWindowType(WindowType::WINDOW_TYPE_INPUT_METHOD_FLOAT);
    ASSERT_TRUE(wms != nullptr);
    bool res = wms->CheckSystemWindowPermission(property);
    ASSERT_EQ(res, true);
    property->SetWindowType(WindowType::WINDOW_TYPE_DIALOG);
    res = wms->CheckSystemWindowPermission(property);
    ASSERT_EQ(res, true);
    property->SetWindowType(WindowType::WINDOW_TYPE_FLOAT);
    res = wms->CheckSystemWindowPermission(property);
    ASSERT_EQ(res, true);
}

/**
 * @tc.name: CreateWindow02
 * @tc.desc: CreateWindow02 test
 * @tc.type: FUNC
 */
HWTEST_F(WindowManagerServiceTest, CreateWindow02, TestSize.Level1)
{
    sptr<IRemoteObject> iRemoteObjectMocker = new IRemoteObjectMocker();
    sptr<IWindow> iWindow = iface_cast<IWindow>(iRemoteObjectMocker);
    uint32_t id = 2;
    RSSurfaceNodeConfig config;
    config.SurfaceNodeName = "webTestSurfaceName";
    auto surfaceNode = RSSurfaceNode::Create(config, false);
    sptr<WindowProperty> property = new WindowProperty();
    property->SetWindowType(WindowType::WINDOW_TYPE_INPUT_METHOD_FLOAT);
    ASSERT_EQ(WMError::WM_OK, wms->CreateWindow(iWindow, property, surfaceNode, id, nullptr));
    wms->DestroyWindow(id, true);
    property->SetWindowType(WindowType::APP_WINDOW_BASE);
    ASSERT_EQ(WMError::WM_OK, wms->CreateWindow(iWindow, property, surfaceNode, id, nullptr));
    wms->DestroyWindow(id, true);
}

/**
 * @tc.name: AddWindow02
 * @tc.desc: AddWindow02 test
 * @tc.type: FUNC
 */
HWTEST_F(WindowManagerServiceTest, AddWindow02, TestSize.Level1)
{
    sptr<WindowProperty> property = new WindowProperty();
    property->SetWindowType(WindowType::WINDOW_TYPE_WALLPAPER);
    ASSERT_EQ(WMError::WM_OK, wms->AddWindow(property));
    property->SetWindowType(WindowType::WINDOW_TYPE_INPUT_METHOD_FLOAT);
}

/**
 * @tc.name: RemoveWindow
 * @tc.desc: RemoveWindow test
 * @tc.type: FUNC
 */
HWTEST_F(WindowManagerServiceTest, RemoveWindow, TestSize.Level1)
{
    sptr<WindowNode> appNode = new WindowNode();
    IWindowMocker* token = new IWindowMocker;
    appNode->SetWindowToken(token);
    bool isFromInnerkits = false;
    wms->windowController_->windowRoot_->windowNodeMap_[appNode->GetWindowId()] = appNode;
    WMError res = wms->RemoveWindow(appNode->GetWindowId(), isFromInnerkits);
    ASSERT_EQ(res, WMError::WM_ERROR_INVALID_OPERATION);
    res = wms->RemoveWindow(appNode->GetWindowId(), isFromInnerkits);
    ASSERT_EQ(res, WMError::WM_ERROR_INVALID_OPERATION);
}

/**
 * @tc.name: DestroyWindow
 * @tc.desc: DestroyWindow test
 * @tc.type: FUNC
 */
HWTEST_F(WindowManagerServiceTest, DestroyWindow, TestSize.Level1)
{
    sptr<IRemoteObject> iRemoteObjectMocker = new IRemoteObjectMocker();
    sptr<IWindow> iWindow = iface_cast<IWindow>(iRemoteObjectMocker);
    sptr<WindowNode> appNode = new WindowNode();
    RSSurfaceNodeConfig config;
    config.SurfaceNodeName = "webTestSurfaceName";
    auto surfaceNode = RSSurfaceNode::Create(config, false);
    sptr<WindowProperty> property = new WindowProperty();
    property->SetWindowType(WindowType::WINDOW_TYPE_INPUT_METHOD_FLOAT);
    uint32_t id = appNode->GetWindowId();
    ASSERT_EQ(WMError::WM_OK, wms->CreateWindow(iWindow, property, surfaceNode, id, nullptr));
    IWindowMocker* token = new IWindowMocker;
    appNode->SetWindowToken(token);
    bool isFromInnerkits = true;
    wms->windowController_->windowRoot_->windowNodeMap_[appNode->GetWindowId()] = appNode;
    WMError res = wms->RemoveWindow(appNode->GetWindowId(), isFromInnerkits);
    ASSERT_EQ(res, WMError::WM_OK);
    res = wms->DestroyWindow(appNode->GetWindowId(), true);
    ASSERT_EQ(res, WMError::WM_OK);
}

/**
 * @tc.name: OnScreenshot01
 * @tc.desc: OnScreenshot01 test
 * @tc.type: FUNC
 */
HWTEST_F(WindowManagerServiceTest, OnScreenshot01, TestSize.Level1)
{
    DisplayId displayId = 1;
    ASSERT_TRUE(listener != nullptr);
    listener->OnScreenshot(displayId);
}

/**
 * @tc.name: NotifyServerReadyToMoveOrDrag02
 * @tc.desc: NotifyServerReadyToMoveOrDrag02 test
 * @tc.type: FUNC
 */
HWTEST_F(WindowManagerServiceTest, NotifyServerReadyToMoveOrDrag02, TestSize.Level1)
{
    sptr<WindowNode> appNode = new WindowNode();
    IWindowMocker* token = new IWindowMocker;
    appNode->SetWindowToken(token);
    wms->windowController_->windowRoot_->windowNodeMap_[appNode->GetWindowId()] = appNode;
    sptr<WindowProperty> property = new WindowProperty();
    sptr<MoveDragProperty> moveDragProperty = new MoveDragProperty();
    ASSERT_TRUE(wms != nullptr);
    wms->NotifyServerReadyToMoveOrDrag(appNode->GetWindowId(), property, moveDragProperty);
}

/**
 * @tc.name: ProcessPointUp
 * @tc.desc: ProcessPointUp test
 * @tc.type: FUNC
 */
HWTEST_F(WindowManagerServiceTest, ProcessPointUp, TestSize.Level1)
{
    uint32_t windowId = 1;
    ASSERT_TRUE(wms != nullptr);
    wms->ProcessPointUp(windowId);
}
}
}
}
