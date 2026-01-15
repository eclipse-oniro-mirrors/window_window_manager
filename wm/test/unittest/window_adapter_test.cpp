/*
 * Copyright (c) 2023-2023 Huawei Device Co., Ltd.
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

#include "iremote_window_manager_mocker.h"
#include "mock_RSIWindowAnimationController.h"
#include "remote_animation.h"
#include "scene_board_judgement.h"
#include "session_manager.h"
#include "starting_window.h"
#include "ui_effect_controller_client.h"
#include "ui_effect_controller_interface.h"
#include "window_adapter.h"
#include "window_agent.h"
#include "window_manager_agent.h"
#include "window_property.h"
#include "window_transition_info.h"

using namespace testing;
using namespace testing::ext;

namespace OHOS {
namespace Rosen {
constexpr uint32_t WINDOW_ID = 1000;
class WindowAdapterTest : public testing::Test {
public:
    static void SetUpTestCase();
    static void TearDownTestCase();
    void SetUp() override;
    void TearDown() override;

private:
    int32_t userId_ = 100;
    sptr<WindowAdapter> instance_ = nullptr;
};

void WindowAdapterTest::SetUpTestCase() {}

void WindowAdapterTest::TearDownTestCase() {}

void WindowAdapterTest::SetUp()
{
    instance_ = &WindowAdapter::GetInstance(userId_);
}

void WindowAdapterTest::TearDown()
{
    instance_ = nullptr;
    WindowAdapter::windowAdapterMap_.clear();
}

namespace {
std::string g_errLog;
void MyLogCallback(const LogType type,
                   const LogLevel level,
                   const unsigned int domain,
                   const char* tag,
                   const char* msg)
{
    g_errLog += msg;
}

/**
 * @tc.name: AddWindow
 * @tc.desc: WindowAdapter/AddWindow
 * @tc.type: FUNC
 */
HWTEST_F(WindowAdapterTest, AddWindow, TestSize.Level1)
{
    sptr<WindowProperty> windowProperty = nullptr;
    WindowAdapter windowAdapter;

    windowAdapter.AddWindow(windowProperty);
    auto ret = windowAdapter.InitWMSProxy();
    ASSERT_EQ(true, ret);
}

/**
 * @tc.name: RemoveWindow
 * @tc.desc: WindowAdapter/RemoveWindow
 * @tc.type: FUNC
 */
HWTEST_F(WindowAdapterTest, RemoveWindow, TestSize.Level1)
{
    uint32_t windowId = 0;
    bool isFromInnerkits = false;
    WindowAdapter windowAdapter;

    windowAdapter.RemoveWindow(windowId, isFromInnerkits);
    auto ret = windowAdapter.InitWMSProxy();
    ASSERT_EQ(true, ret);
}

/**
 * @tc.name: RequestFocus
 * @tc.desc: WindowAdapter/RequestFocus
 * @tc.type: FUNC
 */
HWTEST_F(WindowAdapterTest, RequestFocus, TestSize.Level1)
{
    uint32_t windowId = 0;
    WindowAdapter windowAdapter;

    windowAdapter.RequestFocus(windowId);
    auto ret = windowAdapter.InitWMSProxy();
    ASSERT_EQ(true, ret);
}

/**
 * @tc.name: RequestFocusStatusBySA
 * @tc.desc: WindowAdapter/RequestFocusStatusBySA
 * @tc.type: FUNC
 */
HWTEST_F(WindowAdapterTest, RequestFocusStatusBySA, TestSize.Level1)
{
    WindowAdapter windowAdapter;
    int32_t persistentId = 1;
    bool isFocused = true;
    bool byForeground = true;
    FocusChangeReason reason = FocusChangeReason::CLICK;
    windowAdapter.isProxyValid_ = true;
    windowAdapter.windowManagerServiceProxy_ =  nullptr;
 
    auto result = windowAdapter.RequestFocusStatusBySA(
        persistentId, isFocused, byForeground, reason);
    EXPECT_EQ(result, WMError::WM_ERROR_SAMGR);
}

/**
 * @tc.name: GetUIContentRemoteObj
 * @tc.desc: WindowAdapter/GetUIContentRemoteObj
 * @tc.type: FUNC
 */
HWTEST_F(WindowAdapterTest, GetUIContentRemoteObj, TestSize.Level1)
{
    WindowAdapter windowAdapter;
    windowAdapter.isProxyValid_ = true;
    windowAdapter.windowManagerServiceProxy_ = nullptr;
    sptr<IRemoteObject> remoteObj;
    auto ret = windowAdapter.GetUIContentRemoteObj(WINDOW_ID, remoteObj);
    ASSERT_EQ(ret, WMError::WM_ERROR_SAMGR);
}

/**
 * @tc.name: CheckWindowId
 * @tc.desc: WindowAdapter/CheckWindowId
 * @tc.type: FUNC
 */
HWTEST_F(WindowAdapterTest, CheckWindowId, TestSize.Level1)
{
    int32_t windowId = 0;
    int32_t pid = 0;
    WindowAdapter windowAdapter;

    windowAdapter.CheckWindowId(windowId, pid);
    auto ret = windowAdapter.InitWMSProxy();
    ASSERT_EQ(true, ret);
}

/**
 * @tc.name: GetAccessibilityWindowInfo
 * @tc.desc: WindowAdapter/GetAccessibilityWindowInfo
 * @tc.type: FUNC
 */
HWTEST_F(WindowAdapterTest, GetAccessibilityWindowInfo, TestSize.Level1)
{
    WindowAdapter windowAdapter;
    std::vector<sptr<AccessibilityWindowInfo>> infos;
    windowAdapter.isProxyValid_ = true;
    windowAdapter.windowManagerServiceProxy_ =  nullptr;
    ASSERT_EQ(WMError::WM_ERROR_SAMGR, windowAdapter.GetAccessibilityWindowInfo(infos));
}

/**
 * @tc.name: ConvertToRelativeCoordinateExtended
 * @tc.desc: WindowAdapter/ConvertToRelativeCoordinateExtended
 * @tc.type: FUNC
 */
HWTEST_F(WindowAdapterTest, ConvertToRelativeCoordinateExtended, TestSize.Level1)
{
    WindowAdapter windowAdapter;
    Rect rect;
    Rect newRect;
    DisplayId newDisplayId = 0;
    rect = { 100, 3000, 400, 600 };
    auto ret = windowAdapter.ConvertToRelativeCoordinateExtended(rect, newRect, newDisplayId);
    EXPECT_EQ(WMError::WM_DO_NOTHING, ret);
    EXPECT_EQ(newRect.posX_, 100);
    EXPECT_EQ(newRect.posY_, 3000);
}

/**
 * @tc.name: GetGlobalWindowMode
 * @tc.desc: WindowAdapter/GetGlobalWindowMode
 * @tc.type: FUNC
 */
HWTEST_F(WindowAdapterTest, GetGlobalWindowMode, TestSize.Level1)
{
    WindowAdapter windowAdapter;
    GlobalWindowMode globalWinMode = GlobalWindowMode::UNKNOWN;
    windowAdapter.isProxyValid_ = true;
    windowAdapter.windowManagerServiceProxy_ =  nullptr;
    ASSERT_EQ(WMError::WM_ERROR_SAMGR, windowAdapter.GetGlobalWindowMode(0, globalWinMode));
}

/**
 * @tc.name: SkipSnapshotForAppProcess
 * @tc.desc: WindowAdapter/SkipSnapshotForAppProcess
 * @tc.type: FUNC
 */
HWTEST_F(WindowAdapterTest, SkipSnapshotForAppProcess, TestSize.Level1)
{
    int32_t pid = 1000;
    bool skip = true;
    WindowAdapter windowAdapter;
    windowAdapter.SkipSnapshotForAppProcess(pid, skip);
    auto ret = windowAdapter.InitWMSProxy();
    ASSERT_EQ(true, ret);
}

/**
 * @tc.name: SetWindowAnimationController
 * @tc.desc: WindowAdapter/SetWindowAnimationController
 * @tc.type: FUNC
 */
HWTEST_F(WindowAdapterTest, SetWindowAnimationController, TestSize.Level1)
{
    sptr<RSIWindowAnimationController> controller = nullptr;
    WindowAdapter windowAdapter;

    windowAdapter.SetWindowAnimationController(controller);
    auto ret = windowAdapter.InitWMSProxy();
    ASSERT_EQ(true, ret);
}

/**
 * @tc.name: GetAvoidAreaByType
 * @tc.desc: WindowAdapter/GetAvoidAreaByType
 * @tc.type: FUNC
 */
HWTEST_F(WindowAdapterTest, GetAvoidAreaByType, TestSize.Level1)
{
    uint32_t windowId = 0;
    AvoidAreaType type = AvoidAreaType::TYPE_CUTOUT;
    AvoidArea avoidArea;
    WindowAdapter windowAdapter;

    windowAdapter.GetAvoidAreaByType(windowId, type, avoidArea);
    auto ret = windowAdapter.InitWMSProxy();
    ASSERT_EQ(true, ret);
}

/**
 * @tc.name: NotifyServerReadyToMoveOrDrag
 * @tc.desc: WindowAdapter/NotifyServerReadyToMoveOrDrag
 * @tc.type: FUNC
 */
HWTEST_F(WindowAdapterTest, NotifyServerReadyToMoveOrDrag, TestSize.Level1)
{
    uint32_t windowId = 0;
    sptr<WindowProperty> windowProperty = nullptr;
    sptr<MoveDragProperty> moveDragProperty = nullptr;
    WindowAdapter windowAdapter;

    windowAdapter.NotifyServerReadyToMoveOrDrag(windowId, windowProperty, moveDragProperty);
    auto ret = windowAdapter.InitWMSProxy();
    ASSERT_EQ(true, ret);
}

/**
 * @tc.name: ProcessPointDown
 * @tc.desc: WindowAdapter/ProcessPointDown
 * @tc.type: FUNC
 */
HWTEST_F(WindowAdapterTest, ProcessPointDown, TestSize.Level1)
{
    uint32_t windowId = 0;
    bool isPointDown = false;
    WindowAdapter windowAdapter;

    windowAdapter.ProcessPointDown(windowId, isPointDown);
    windowAdapter.ProcessPointUp(windowId);
    auto ret = windowAdapter.InitWMSProxy();
    ASSERT_EQ(true, ret);
}

/**
 * @tc.name: ToggleShownStateForAllAppWindows
 * @tc.desc: WindowAdapter/ToggleShownStateForAllAppWindows
 * @tc.type: FUNC
 */
HWTEST_F(WindowAdapterTest, ToggleShownStateForAllAppWindows, TestSize.Level1)
{
    WindowAdapter windowAdapter;

    windowAdapter.ToggleShownStateForAllAppWindows();
    auto ret = windowAdapter.InitWMSProxy();
    ASSERT_EQ(true, ret);
}

/**
 * @tc.name: GetSnapshotByWindowId
 * @tc.desc: WindowAdapter/GetSnapshotByWindowId
 * @tc.type: FUNC
 */
HWTEST_F(WindowAdapterTest, GetSnapshotByWindowId, TestSize.Level1)
{
    WindowAdapter windowAdapter;
    int32_t persistentId = -1;
    std::shared_ptr<Media::PixelMap> pixelMap = nullptr;
    windowAdapter.GetSnapshotByWindowId(persistentId, pixelMap);
    auto ret = windowAdapter.InitWMSProxy();
    ASSERT_EQ(true, ret);
}

/**
 * @tc.name: RegisterWMSConnectionChangedListener
 * @tc.desc: WindowAdapter/RegisterWMSConnectionChangedListener
 * @tc.type: FUNC
 */
HWTEST_F(WindowAdapterTest, RegisterWMSConnectionChangedListener, TestSize.Level1)
{
    WindowAdapter windowAdapter;
    ASSERT_EQ(WMError::WM_ERROR_NULLPTR, windowAdapter.RegisterWMSConnectionChangedListener(nullptr));
}

/**
 * @tc.name: InitWMSProxy
 * @tc.desc: WindowAdapter/InitWMSProxy
 * @tc.type: FUNC
 */
HWTEST_F(WindowAdapterTest, InitWMSProxy, TestSize.Level1)
{
    WindowAdapter windowAdapter;
    auto ret = windowAdapter.InitWMSProxy();
    ASSERT_EQ(true, ret);
}

/**
 * @tc.name: WindowManagerAndSessionRecover
 * @tc.desc: WindowAdapter/WindowManagerAndSessionRecover
 * @tc.type: FUNC
 */
HWTEST_F(WindowAdapterTest, WindowManagerAndSessionRecover, TestSize.Level1)
{
    WindowAdapter windowAdapter;
    int32_t persistentId = 1;
    int32_t ret = 0;
    auto testFunc = [&ret] {
        ret += 1;
        return WMError::WM_DO_NOTHING;
    };

    auto testFunc2 = [&ret] {
        ret += 1;
        return WMError::WM_OK;
    };

    auto testFunc3 = [] {
        return WMError::WM_OK;
    };
    auto testFunc4 = [] {
        return WMError::WM_DO_NOTHING;
    };
    windowAdapter.RegisterSessionRecoverCallbackFunc(persistentId, testFunc);
    windowAdapter.RegisterUIEffectRecoverCallbackFunc(persistentId, testFunc3);
    windowAdapter.RegisterOutlineRecoverCallbackFunc(testFunc3);
    windowAdapter.WindowManagerAndSessionRecover();
    if (SceneBoardJudgement::IsSceneBoardEnabled()) {
        ASSERT_EQ(ret, 1);
    }
    windowAdapter.RegisterSessionRecoverCallbackFunc(persistentId, testFunc2);
    windowAdapter.RegisterUIEffectRecoverCallbackFunc(persistentId, testFunc4);
    windowAdapter.RegisterOutlineRecoverCallbackFunc(testFunc4);
    windowAdapter.WindowManagerAndSessionRecover();
    if (SceneBoardJudgement::IsSceneBoardEnabled()) {
        ASSERT_EQ(ret, 2);
    }
}

/**
 * @tc.name: GetUnreliableWindowInfo
 * @tc.desc: WindowAdapter/GetUnreliableWindowInfo
 * @tc.type: FUNC
 */
HWTEST_F(WindowAdapterTest, GetUnreliableWindowInfo, TestSize.Level1)
{
    int32_t windowId = 0;
    std::vector<sptr<UnreliableWindowInfo>> infos;
    WindowAdapter windowAdapter;
    windowAdapter.GetUnreliableWindowInfo(windowId, infos);
    auto ret = windowAdapter.InitWMSProxy();
    ASSERT_EQ(true, ret);
}

/**
 * @tc.name: RecoverAndConnectSpecificSession
 * @tc.desc: WindowAdapter/RecoverAndConnectSpecificSession
 * @tc.type: FUNC
 */
HWTEST_F(WindowAdapterTest, RecoverAndConnectSpecificSession, TestSize.Level1)
{
    WindowAdapter windowAdapter;
    auto ret = windowAdapter.InitWMSProxy();
    sptr<ISessionStage> sessionStage;
    sptr<IWindowEventChannel> eventChannel;
    std::shared_ptr<RSSurfaceNode> node;
    sptr<WindowSessionProperty> property;
    sptr<ISession> session;
    SystemSessionConfig systemConfig;
    sptr<IRemoteObject> token;
    windowAdapter.RecoverAndConnectSpecificSession(
        sessionStage, eventChannel, node, property, session, token);
    ASSERT_EQ(ret, true);
}

/**
 * @tc.name: DestroyAndDisconnectSpecificSession
 * @tc.desc: WindowAdapter/DestroyAndDisconnectSpecificSession
 * @tc.type: FUNC
 */
HWTEST_F(WindowAdapterTest, DestroyAndDisconnectSpecificSession, TestSize.Level1)
{
    uint32_t persistentId = 0;
    WindowAdapter windowAdapter;
    windowAdapter.DestroyAndDisconnectSpecificSession(persistentId);
    auto ret = windowAdapter.InitWMSProxy();
    ASSERT_EQ(true, ret);
}

/**
 * @tc.name: DestroyAndDisconnectSpecificSessionWithDetachCallback
 * @tc.desc: WindowAdapter/DestroyAndDisconnectSpecificSessionWithDetachCallback
 * @tc.type: FUNC
 */
HWTEST_F(WindowAdapterTest, DestroyAndDisconnectSpecificSessionWithDetachCallback, TestSize.Level1)
{
    uint32_t persistentId = 0;
    WindowAdapter windowAdapter;
    windowAdapter.DestroyAndDisconnectSpecificSessionWithDetachCallback(persistentId, nullptr);
    auto ret = windowAdapter.InitWMSProxy();
    ASSERT_EQ(true, ret);
}

/**
 * @tc.name: UpdateModalExtensionRect
 * @tc.desc: WindowAdapter/UpdateModalExtensionRect
 * @tc.type: FUNC
 */
HWTEST_F(WindowAdapterTest, UpdateModalExtensionRect, TestSize.Level1)
{
    WindowAdapter windowAdapter;
    Rect rect{ 1, 2, 3, 4 };
    sptr<IRemoteObject> token = sptr<IRemoteObjectMocker>::MakeSptr();
    ASSERT_NE(token, nullptr);
    windowAdapter.UpdateModalExtensionRect(token, rect);
    windowAdapter.UpdateModalExtensionRect(nullptr, rect);
    auto ret = windowAdapter.InitWMSProxy();
    EXPECT_TRUE(ret);
}

/**
 * @tc.name: ProcessModalExtensionPointDown
 * @tc.desc: WindowAdapter/ProcessModalExtensionPointDown
 * @tc.type: FUNC
 */
HWTEST_F(WindowAdapterTest, ProcessModalExtensionPointDown, TestSize.Level1)
{
    WindowAdapter windowAdapter;
    sptr<IRemoteObject> token = sptr<IRemoteObjectMocker>::MakeSptr();
    ASSERT_NE(token, nullptr);
    windowAdapter.ProcessModalExtensionPointDown(token, 0, 0);
    windowAdapter.ProcessModalExtensionPointDown(nullptr, 0, 0);
    auto ret = windowAdapter.InitWMSProxy();
    EXPECT_TRUE(ret);
}

/**
 * @tc.name: AddExtensionWindowStageToSCB
 * @tc.desc: WindowAdapter/AddExtensionWindowStageToSCB
 * @tc.type: FUNC
 */
HWTEST_F(WindowAdapterTest, AddExtensionWindowStageToSCB, TestSize.Level1)
{
    WindowAdapter windowAdapter;
    windowAdapter.AddExtensionWindowStageToSCB(nullptr, nullptr, 0, -1);
    auto ret = windowAdapter.InitWMSProxy();
    EXPECT_TRUE(ret);
}

/**
 * @tc.name: RemoveExtensionWindowStageFromSCB
 * @tc.desc: WindowAdapter/RemoveExtensionWindowStageFromSCB
 * @tc.type: FUNC
 */
HWTEST_F(WindowAdapterTest, RemoveExtensionWindowStageFromSCB, TestSize.Level1)
{
    WindowAdapter windowAdapter;
    windowAdapter.RemoveExtensionWindowStageFromSCB(nullptr, nullptr);
    auto ret = windowAdapter.InitWMSProxy();
    EXPECT_TRUE(ret);
}

/**
 * @tc.name: AddOrRemoveSecureSession
 * @tc.desc: WindowAdapter/AddOrRemoveSecureSession
 * @tc.type: FUNC
 */
HWTEST_F(WindowAdapterTest, AddOrRemoveSecureSession, TestSize.Level1)
{
    int32_t persistentId = 0;
    WindowAdapter windowAdapter;
    windowAdapter.AddOrRemoveSecureSession(persistentId, true);
    auto ret = windowAdapter.InitWMSProxy();
    ASSERT_EQ(true, ret);
}

/**
 * @tc.name: UpdateExtWindowFlags
 * @tc.desc: WindowAdapter/UpdateExtWindowFlags
 * @tc.type: FUNC
 */
HWTEST_F(WindowAdapterTest, UpdateExtWindowFlags, TestSize.Level1)
{
    WindowAdapter windowAdapter;
    sptr<IRemoteObject> token = sptr<IRemoteObjectMocker>::MakeSptr();
    ASSERT_NE(token, nullptr);
    windowAdapter.UpdateExtWindowFlags(token, 7, 7);
    windowAdapter.UpdateExtWindowFlags(nullptr, 7, 7);
    auto ret = windowAdapter.InitWMSProxy();
    ASSERT_EQ(true, ret);
}

/**
 * @tc.name: GetVisibilityWindowInfo
 * @tc.desc: WindowAdapter/GetVisibilityWindowInfo
 * @tc.type: FUNC
 */
HWTEST_F(WindowAdapterTest, GetVisibilityWindowInfo, TestSize.Level1)
{
    WindowAdapter windowAdapter;
    windowAdapter.isProxyValid_ = true;
    windowAdapter.windowManagerServiceProxy_ =  nullptr;
    std::vector<sptr<WindowVisibilityInfo>> infos;
    auto ret = windowAdapter.GetVisibilityWindowInfo(infos);
    windowAdapter.WindowManagerAndSessionRecover();
    ASSERT_EQ(WMError::WM_ERROR_SAMGR, ret);
}

/**
 * @tc.name: RecoverWatermarkImageForApp
 * @tc.desc: WindowAdapter/RecoverWatermarkImageForApp
 * @tc.type: FUNC
 */
HWTEST_F(WindowAdapterTest, RecoverWatermarkImageForApp01, TestSize.Level1)
{
    WindowAdapter windowAdapter;
    auto ret = windowAdapter.RecoverWatermarkImageForApp();
    EXPECT_EQ(ret, WMError::WM_OK);
    windowAdapter.appWatermarkName_ = "bundleName#pid";
    ret = windowAdapter.RecoverWatermarkImageForApp();
    EXPECT_EQ(ret, WMError::WM_ERROR_NULLPTR);
    windowAdapter.InitWMSProxy();
    ASSERT_NE(windowAdapter.windowManagerServiceProxy_, nullptr);
    ret = windowAdapter.RecoverWatermarkImageForApp();
    EXPECT_EQ(ret, WMError::WM_ERROR_DEVICE_NOT_SUPPORT);
}

/**
 * @tc.name: RecoverWindowPropertyChangeFlag
 * @tc.desc: WindowAdapter/RecoverWindowPropertyChangeFlag
 * @tc.type: FUNC
 */
HWTEST_F(WindowAdapterTest, RecoverWindowPropertyChangeFlag01, TestSize.Level1)
{
    WindowAdapter windowAdapter;
    auto ret = windowAdapter.RecoverWindowPropertyChangeFlag();
    EXPECT_EQ(WMError::WM_ERROR_NULLPTR, ret);

    windowAdapter.InitWMSProxy();
    ASSERT_NE(windowAdapter.windowManagerServiceProxy_, nullptr);
    ret = windowAdapter.RecoverWindowPropertyChangeFlag();
    EXPECT_EQ(WMError::WM_ERROR_DEVICE_NOT_SUPPORT, ret);
}

/**
 * @tc.name: ReregisterWindowManagerAgent
 * @tc.desc: WindowAdapter/ReregisterWindowManagerAgent
 * @tc.type: FUNC
 */
HWTEST_F(WindowAdapterTest, ReregisterWindowManagerAgent, TestSize.Level1)
{
    g_errLog.clear();
    LOG_SetCallback(MyLogCallback);
    auto type = WindowManagerAgentType::WINDOW_MANAGER_AGENT_TYPE_FOCUS;
    auto agent = sptr<WindowManagerAgent>::MakeSptr();

    // branch 1
    instance_->windowManagerServiceProxy_ = nullptr;
    instance_->ReregisterWindowManagerAgent();
    EXPECT_TRUE(g_errLog.find("window manager proxy is nullptr") != std::string::npos);

    // branch 2: send ipc failed
    g_errLog.clear();
    ASSERT_EQ(true, instance_->InitSSMProxy());
    instance_->windowManagerAgentMap_[type].insert(agent);
    instance_->ReregisterWindowManagerAgent();
    EXPECT_TRUE(g_errLog.find("register failed due to wms proxy") != std::string::npos);

    // branch 3: mock send ipc success
    g_errLog.clear();
    auto remoteObject = sptr<IRemoteWindowManagerMocker>::MakeSptr();
    EXPECT_CALL(*remoteObject, RegisterWindowManagerAgent(_, _)).Times(1).WillOnce(Return(WMError::WM_OK));

    auto wmsProxy = iface_cast<IWindowManager>(remoteObject);
    instance_->windowManagerServiceProxy_ = wmsProxy;
    instance_->ReregisterWindowManagerAgent();
    EXPECT_FALSE(g_errLog.find("register failed due to wms proxy") != std::string::npos);

    LOG_SetCallback(nullptr);
}

/**
 * @tc.name: UpdateProperty
 * @tc.desc: WindowAdapter/UpdateProperty
 * @tc.type: FUNC
 */
HWTEST_F(WindowAdapterTest, UpdateProperty, TestSize.Level1)
{
    WindowAdapter windowAdapter;
    sptr<WindowProperty> windowProperty = sptr<WindowProperty>::MakeSptr();
    PropertyChangeAction action = PropertyChangeAction::ACTION_UPDATE_RECT;
    windowAdapter.isProxyValid_ = true;
    windowAdapter.windowManagerServiceProxy_ =  nullptr;

    auto ret = windowAdapter.UpdateProperty(windowProperty, action);
    windowAdapter.OnUserSwitch();
    windowAdapter.ClearWindowAdapter();
    ASSERT_EQ(WMError::WM_ERROR_SAMGR, ret);
}

/**
 * @tc.name: SetWindowGravity
 * @tc.desc: WindowAdapter/SetWindowGravity
 * @tc.type: FUNC
 */
HWTEST_F(WindowAdapterTest, SetWindowGravity, TestSize.Level1)
{
    WindowAdapter windowAdapter;
    windowAdapter.isProxyValid_ = true;
    windowAdapter.windowManagerServiceProxy_ =  nullptr;
    WindowGravity gravity = WindowGravity::WINDOW_GRAVITY_FLOAT;
    ASSERT_EQ(WMError::WM_ERROR_SAMGR, windowAdapter.SetWindowGravity(0, gravity, 0));
}

/**
 * @tc.name: NotifyWindowTransition
 * @tc.desc: WindowAdapter/NotifyWindowTransition
 * @tc.type: FUNC
 */
HWTEST_F(WindowAdapterTest, NotifyWindowTransition, TestSize.Level1)
{
    WindowAdapter windowAdapter;
    windowAdapter.isProxyValid_ = true;
    windowAdapter.windowManagerServiceProxy_ =  nullptr;
    sptr<WindowTransitionInfo> from = sptr<WindowTransitionInfo>::MakeSptr();
    sptr<WindowTransitionInfo> to = sptr<WindowTransitionInfo>::MakeSptr();
    ASSERT_EQ(WMError::WM_ERROR_SAMGR, windowAdapter.NotifyWindowTransition(from, to));
}

/**
 * @tc.name: MinimizeWindowsByLauncher
 * @tc.desc: WindowAdapter/MinimizeWindowsByLauncher
 * @tc.type: FUNC
 */
HWTEST_F(WindowAdapterTest, MinimizeWindowsByLauncher, TestSize.Level1)
{
    WindowAdapter windowAdapter;
    auto ret = windowAdapter.InitWMSProxy();
    std::vector<uint32_t> windowIds;
    bool isAnimated = true;
    sptr<RSIWindowAnimationFinishedCallback> finishCallback;
    windowAdapter.MinimizeWindowsByLauncher(windowIds, isAnimated, finishCallback);
    ASSERT_EQ(true, ret);
}

/**
 * @tc.name: UpdateRsTree
 * @tc.desc: WindowAdapter/UpdateRsTree
 * @tc.type: FUNC
 */
HWTEST_F(WindowAdapterTest, UpdateRsTree, TestSize.Level1)
{
    WindowAdapter windowAdapter;
    windowAdapter.isProxyValid_ = true;
    windowAdapter.windowManagerServiceProxy_ =  nullptr;
    ASSERT_EQ(WMError::WM_ERROR_SAMGR, windowAdapter.UpdateRsTree(0, false));
}

/**
 * @tc.name: BindDialogTarget
 * @tc.desc: WindowAdapter/BindDialogTarget
 * @tc.type: FUNC
 */
HWTEST_F(WindowAdapterTest, BindDialogTarget, TestSize.Level1)
{
    WindowAdapter windowAdapter;
    windowAdapter.isProxyValid_ = true;
    windowAdapter.windowManagerServiceProxy_ = nullptr;
    sptr<IRemoteObject> targetToken;
    uint32_t windowId = 0;
    ASSERT_EQ(WMError::WM_ERROR_SAMGR, windowAdapter.BindDialogTarget(windowId, targetToken));
}

/**
 * @tc.name: RaiseToAppTop
 * @tc.desc: WindowAdapter/RaiseToAppTop
 * @tc.type: FUNC
 */
HWTEST_F(WindowAdapterTest, RaiseToAppTop, TestSize.Level1)
{
    WindowAdapter windowAdapter;
    uint32_t windowId = 0;

    windowAdapter.isProxyValid_ = true;
    auto ret = windowAdapter.RaiseToAppTop(windowId);
    ASSERT_EQ(WMError::WM_ERROR_SAMGR, ret);
    windowAdapter.isProxyValid_ = false;

    ret = windowAdapter.RaiseToAppTop(windowId);
    std::shared_ptr<MMI::KeyEvent> event = nullptr;
    windowAdapter.DispatchKeyEvent(windowId, event);
    ASSERT_EQ(WMError::WM_OK, ret);
}

/**
 * @tc.name: GetSnapshot
 * @tc.desc: WindowAdapter/GetSnapshot
 * @tc.type: FUNC
 */
HWTEST_F(WindowAdapterTest, GetSnapshot, TestSize.Level1)
{
    WindowAdapter windowAdapter;
    int32_t windowId = 0;
    auto ret = windowAdapter.GetSnapshot(windowId);
    ASSERT_EQ(nullptr, ret);
}

/**
 * @tc.name: GetWindowAnimationTargets
 * @tc.desc: WindowAdapter/GetWindowAnimationTargets
 * @tc.type: FUNC
 */
HWTEST_F(WindowAdapterTest, GetWindowAnimationTargets, TestSize.Level1)
{
    WindowAdapter windowAdapter;
    std::vector<uint32_t> missionIds;
    std::vector<sptr<RSWindowAnimationTarget>> targets;
    auto ret = windowAdapter.GetWindowAnimationTargets(missionIds, targets);
    if (SceneBoardJudgement::IsSceneBoardEnabled()) {
        ASSERT_EQ(WMError::WM_OK, ret);
    } else {
        ASSERT_EQ(WMError::WM_ERROR_NO_MEM, ret);
    }
}

/**
 * @tc.name: GetMaximizeMode
 * @tc.desc: WindowAdapter/GetMaximizeMode
 * @tc.type: FUNC
 */
HWTEST_F(WindowAdapterTest, GetMaximizeMode, TestSize.Level1)
{
    WindowAdapter windowAdapter;
    auto ret = windowAdapter.InitWMSProxy();
    ASSERT_EQ(true, ret);
    windowAdapter.isProxyValid_ = true;
    ASSERT_EQ(MaximizeMode::MODE_FULL_FILL, windowAdapter.GetMaximizeMode());
}

/**
 * @tc.name: UpdateSessionAvoidAreaListener
 * @tc.desc: WindowAdapter/UpdateSessionAvoidAreaListener
 * @tc.type: FUNC
 */
HWTEST_F(WindowAdapterTest, UpdateSessionAvoidAreaListener, TestSize.Level1)
{
    WindowAdapter windowAdapter;
    windowAdapter.isProxyValid_ = true;
    windowAdapter.windowManagerServiceProxy_ = nullptr;
    int32_t persistentId = 0;
    bool haveListener = true;
    auto ret = windowAdapter.UpdateSessionAvoidAreaListener(persistentId, haveListener);
    ASSERT_EQ(WMError::WM_DO_NOTHING, ret);
}

/**
 * @tc.name: UpdateSessionTouchOutsideListener
 * @tc.desc: WindowAdapter/UpdateSessionTouchOutsideListener
 * @tc.type: FUNC
 */
HWTEST_F(WindowAdapterTest, UpdateSessionTouchOutsideListener, TestSize.Level1)
{
    WindowAdapter windowAdapter;
    windowAdapter.isProxyValid_ = true;
    windowAdapter.windowManagerServiceProxy_ = nullptr;
    int32_t persistentId = 0;
    bool haveListener = true;
    auto ret = windowAdapter.UpdateSessionTouchOutsideListener(persistentId, haveListener);
    ASSERT_EQ(WMError::WM_DO_NOTHING, ret);
}

/**
 * @tc.name: SetSessionGravity
 * @tc.desc: WindowAdapter/SetSessionGravity
 * @tc.type: FUNC
 */
HWTEST_F(WindowAdapterTest, SetSessionGravity, TestSize.Level1)
{
    WindowAdapter windowAdapter;
    windowAdapter.isProxyValid_ = true;
    windowAdapter.windowManagerServiceProxy_ = nullptr;
    int32_t persistentId = 0;
    SessionGravity gravity = SessionGravity::SESSION_GRAVITY_FLOAT;
    uint32_t percent = 0;
    auto ret = windowAdapter.SetSessionGravity(persistentId, gravity, percent);
    ASSERT_EQ(WMError::WM_DO_NOTHING, ret);
}

/**
 * @tc.name: BindDialogSessionTarget
 * @tc.desc: WindowAdapter/BindDialogSessionTarget
 * @tc.type: FUNC
 */
HWTEST_F(WindowAdapterTest, BindDialogSessionTarget, TestSize.Level1)
{
    WindowAdapter windowAdapter;
    windowAdapter.isProxyValid_ = true;
    windowAdapter.windowManagerServiceProxy_ = nullptr;
    uint64_t persistentId = 0;
    sptr<IRemoteObject> targetToken = nullptr;
    auto ret = windowAdapter.BindDialogSessionTarget(persistentId, targetToken);
    ASSERT_EQ(WMError::WM_DO_NOTHING, ret);
}

/**
 * @tc.name: GetHostWindowRect
 * @tc.desc: WindowAdapter/GetHostWindowRect
 * @tc.type: FUNC
 */
HWTEST_F(WindowAdapterTest, GetHostWindowRect, TestSize.Level1)
{
    WindowAdapter windowAdapter;
    windowAdapter.isProxyValid_ = true;
    windowAdapter.windowManagerServiceProxy_ = nullptr;
    int32_t hostWindowId = 0;
    Rect rect = { 0, 0, 0, 0 };
    auto ret = windowAdapter.GetHostWindowRect(hostWindowId, rect);
    ASSERT_EQ(WMError::WM_DO_NOTHING, ret);
}

/**
 * @tc.name: GetHostGlobalScaledRect
 * @tc.desc: WindowAdapter/GetHostGlobalScaledRect
 * @tc.type: FUNC
 */
HWTEST_F(WindowAdapterTest, GetHostGlobalScaledRect, TestSize.Level1)
{
    WindowAdapter windowAdapter;
    windowAdapter.isProxyValid_ = true;
    windowAdapter.windowManagerServiceProxy_ = nullptr;
    int32_t hostWindowId = 0;
    Rect rect = { 0, 0, 0, 0 };
    auto ret = windowAdapter.GetHostGlobalScaledRect(hostWindowId, rect);
    ASSERT_EQ(WMError::WM_DO_NOTHING, ret);
}

/**
 * @tc.name: GetWindowModeType
 * @tc.desc: WindowAdapter/GetWindowModeType
 * @tc.type: FUNC
 */
HWTEST_F(WindowAdapterTest, GetWindowModeType, TestSize.Level1)
{
    WindowModeType windowModeType;
    ASSERT_EQ(true, instance_->InitSSMProxy());
    EXPECT_NE(WMError::WM_OK, instance_->GetWindowModeType(windowModeType));
}

/**
 * @tc.name: GetWindowStyleType
 * @tc.desc: WindowAdapter/GetWindowStyleType
 * @tc.type: FUNC
 */
HWTEST_F(WindowAdapterTest, GetWindowStyleType, TestSize.Level1)
{
    WindowAdapter windowAdapter;
    windowAdapter.isProxyValid_ = true;
    windowAdapter.windowManagerServiceProxy_ = nullptr;
    WindowStyleType windowStyleType = Rosen::WindowStyleType::WINDOW_STYLE_DEFAULT;
    ASSERT_EQ(WMError::WM_ERROR_SAMGR, windowAdapter.GetWindowStyleType(windowStyleType));
}

/**
 * @tc.name: GetWindowIdsByCoordinate
 * @tc.desc: WindowAdapter/GetWindowIdsByCoordinate
 * @tc.type: FUNC
 */
HWTEST_F(WindowAdapterTest, GetWindowIdsByCoordinate, TestSize.Level1)
{
    WindowAdapter windowAdapter;
    windowAdapter.isProxyValid_ = true;
    windowAdapter.windowManagerServiceProxy_ = nullptr;
    std::vector<int32_t> windowIds;
    ASSERT_EQ(WMError::WM_ERROR_SAMGR, windowAdapter.GetWindowIdsByCoordinate(0, 0, 0, 0, windowIds));
}

/**
 * @tc.name: SetProcessWatermark
 * @tc.desc: WindowAdapter/SetProcessWatermark
 * @tc.type: FUNC
 */
HWTEST_F(WindowAdapterTest, SetProcessWatermark, TestSize.Level1)
{
    int32_t pid = 1000;
    const std::string watermarkName = "SetProcessWatermarkName";
    bool isEnabled = true;
    WindowAdapter windowAdapter;
    windowAdapter.SetProcessWatermark(pid, watermarkName, isEnabled);
    auto ret = windowAdapter.InitWMSProxy();
    ASSERT_EQ(true, ret);
}

/**
 * @tc.name: NotifyScreenshotEvent
 * @tc.desc: NotifyScreenshotEvent
 * @tc.type: FUNC
 */
HWTEST_F(WindowAdapterTest, NotifyScreenshotEvent, TestSize.Level1)
{
    ScreenshotEventType type = ScreenshotEventType::SCROLL_SHOT_START;
    WindowAdapter windowAdapter;
    windowAdapter.isProxyValid_ = true;
    windowAdapter.windowManagerServiceProxy_ = nullptr;
    auto err = windowAdapter.NotifyScreenshotEvent(type);
    EXPECT_EQ(err, WMError::WM_ERROR_SAMGR);
    auto ret = windowAdapter.InitWMSProxy();
    EXPECT_EQ(ret, true);
}

/**
 * @tc.name: UpdateScreenLockStatusForApp
 * @tc.desc: WindowAdapter/UpdateScreenLockStatusForApp
 * @tc.type: FUNC
 */
HWTEST_F(WindowAdapterTest, UpdateScreenLockStatusForApp, TestSize.Level1)
{
    WindowAdapter windowAdapter;
    windowAdapter.isProxyValid_ = true;
    windowAdapter.windowManagerServiceProxy_ = nullptr;
    auto err = windowAdapter.UpdateScreenLockStatusForApp("", true);
    ASSERT_EQ(err, WMError::WM_DO_NOTHING);
    auto ret = windowAdapter.InitWMSProxy();
    ASSERT_EQ(ret, true);
}

/**
 * @tc.name: CreateAndConnectSpecificSession
 * @tc.desc: WindowAdapter/CreateAndConnectSpecificSession
 * @tc.type: FUNC
 */
HWTEST_F(WindowAdapterTest, CreateAndConnectSpecificSession, TestSize.Level1)
{
    WindowAdapter windowAdapter;
    auto ret = windowAdapter.InitWMSProxy();
    ASSERT_EQ(ret, true);

    sptr<ISessionStage> sessionStage;
    sptr<IWindowEventChannel> eventChannel;
    std::shared_ptr<RSSurfaceNode> node;
    sptr<WindowSessionProperty> property;
    sptr<ISession> session;
    SystemSessionConfig systemConfig;
    sptr<IRemoteObject> token;
    int32_t id = 101; // 101 is persistentId
    windowAdapter.CreateAndConnectSpecificSession(
        sessionStage, eventChannel, node, property, id, session, systemConfig, token);
    ASSERT_EQ(session, nullptr);
}

/**
 * @tc.name: IsPcWindow
 * @tc.desc: WindowAdapter/IsPcWindow
 * @tc.type: FUNC
 */
HWTEST_F(WindowAdapterTest, IsPcWindow, TestSize.Level1)
{
    WindowAdapter windowAdapter;
    windowAdapter.isProxyValid_ = true;
    windowAdapter.windowManagerServiceProxy_ = nullptr;
    bool isPcWindow = false;
    auto err = windowAdapter.IsPcWindow(isPcWindow);
    ASSERT_EQ(err, WMError::WM_ERROR_SAMGR);
}

/**
 * @tc.name: IsFreeMultiWindowMode
 * @tc.desc: WindowAdapter/IsFreeMultiWindowMode
 * @tc.type: FUNC
 */
HWTEST_F(WindowAdapterTest, IsFreeMultiWindowMode, TestSize.Level1)
{
    WindowAdapter windowAdapter;
    auto ret = windowAdapter.InitWMSProxy();
    ASSERT_EQ(ret, true);

    auto proxy = windowAdapter.GetWindowManagerServiceProxy();
    ASSERT_NE(proxy, nullptr);
    bool proxyIsFreeMultWindow = false;
    auto proxyRet = proxy->IsFreeMultiWindow(proxyIsFreeMultWindow);
    ASSERT_EQ(proxyRet, WMError::WM_OK);

    bool isFreeMultiWindow = false;
    auto err = windowAdapter.IsFreeMultiWindowMode(isFreeMultiWindow);

    ASSERT_EQ(err, WMError::WM_OK);
}

/**
 * @tc.name: IsPcOrPadFreeMultiWindowMode
 * @tc.desc: WindowAdapter/IsPcOrPadFreeMultiWindowMode
 * @tc.type: FUNC
 */
HWTEST_F(WindowAdapterTest, IsPcOrPadFreeMultiWindowMode, TestSize.Level1)
{
    WindowAdapter windowAdapter;
    windowAdapter.isProxyValid_ = true;
    windowAdapter.windowManagerServiceProxy_ = nullptr;
    bool isPcOrPadFreeMultiWindowMode = false;
    auto err = windowAdapter.IsPcOrPadFreeMultiWindowMode(isPcOrPadFreeMultiWindowMode);
    ASSERT_EQ(err, WMError::WM_ERROR_SAMGR);
    auto ret = windowAdapter.InitWMSProxy();
    ASSERT_EQ(ret, true);
}

/**
 * @tc.name: IsWindowRectAutoSave
 * @tc.desc: WindowAdapter/IsWindowRectAutoSave
 * @tc.type: FUNC
 */
HWTEST_F(WindowAdapterTest, IsWindowRectAutoSave, TestSize.Level1)
{
    WindowAdapter windowAdapter;
    std::string key = "com.example.recposentryEntryAbility";
    bool enabled = false;
    int persistentId = 1;
    auto err = windowAdapter.IsWindowRectAutoSave(key, enabled, persistentId);
    ASSERT_EQ(err, WMError::WM_ERROR_INVALID_SESSION);
    windowAdapter.isProxyValid_ = true;
    auto ret = windowAdapter.InitWMSProxy();
    ASSERT_EQ(ret, true);
}

/**
 * @tc.name: SetStartWindowBackgroundColor
 * @tc.desc: WindowAdapter/SetStartWindowBackgroundColor
 * @tc.type: FUNC
 */
HWTEST_F(WindowAdapterTest, SetStartWindowBackgroundColor, TestSize.Level1)
{
    WindowAdapter windowAdapter;
    windowAdapter.isProxyValid_ = true;
    windowAdapter.windowManagerServiceProxy_ = nullptr;
    const std::string& moduleName = "testModuleName";
    const std::string& abilityName = "testAbilityName";
    ASSERT_EQ(WMError::WM_ERROR_SAMGR, windowAdapter.SetStartWindowBackgroundColor(moduleName, abilityName, 0, 0));
}

/**
 * @tc.name: GetDisplayIdByWindowId
 * @tc.desc: WindowAdapter/GetDisplayIdByWindowId
 * @tc.type: FUNC
 */
HWTEST_F(WindowAdapterTest, GetDisplayIdByWindowId, TestSize.Level1)
{
    WindowAdapter windowAdapter;
    windowAdapter.isProxyValid_ = true;
    windowAdapter.windowManagerServiceProxy_ = nullptr;
    const std::vector<uint64_t> windowIds = { 1, 2 };
    std::unordered_map<uint64_t, DisplayId> windowDisplayIdMap;
    auto err = windowAdapter.GetDisplayIdByWindowId(windowIds, windowDisplayIdMap);
    ASSERT_EQ(err, WMError::WM_ERROR_SAMGR);
}

/**
 * @tc.name: SetGlobalDragResizeType
 * @tc.desc: WindowAdapter/SetGlobalDragResizeType
 * @tc.type: FUNC
 */
HWTEST_F(WindowAdapterTest, SetGlobalDragResizeType, TestSize.Level1)
{
    WindowAdapter windowAdapter;
    windowAdapter.isProxyValid_ = true;
    windowAdapter.windowManagerServiceProxy_ = nullptr;
    DragResizeType dragResizeType = DragResizeType::RESIZE_EACH_FRAME;
    auto err = windowAdapter.SetGlobalDragResizeType(dragResizeType);
    ASSERT_EQ(err, WMError::WM_ERROR_SAMGR);
}

/**
 * @tc.name: GetGlobalDragResizeType
 * @tc.desc: WindowAdapter/GetGlobalDragResizeType
 * @tc.type: FUNC
 */
HWTEST_F(WindowAdapterTest, GetGlobalDragResizeType, TestSize.Level1)
{
    WindowAdapter windowAdapter;
    windowAdapter.isProxyValid_ = true;
    windowAdapter.windowManagerServiceProxy_ = nullptr;
    DragResizeType dragResizeType = DragResizeType::RESIZE_TYPE_UNDEFINED;
    auto err = windowAdapter.GetGlobalDragResizeType(dragResizeType);
    ASSERT_EQ(err, WMError::WM_ERROR_SAMGR);
}

/**
 * @tc.name: SetAppDragResizeType
 * @tc.desc: WindowAdapter/SetAppDragResizeType
 * @tc.type: FUNC
 */
HWTEST_F(WindowAdapterTest, SetAppDragResizeType, TestSize.Level1)
{
    WindowAdapter windowAdapter;
    windowAdapter.isProxyValid_ = true;
    windowAdapter.windowManagerServiceProxy_ = nullptr;
    DragResizeType dragResizeType = DragResizeType::RESIZE_EACH_FRAME;
    const std::string bundleName = "test";
    auto err = windowAdapter.SetAppDragResizeType(bundleName, dragResizeType);
    ASSERT_EQ(err, WMError::WM_ERROR_SAMGR);
}

/**
 * @tc.name: GetAppDragResizeType
 * @tc.desc: WindowAdapter/GetAppDragResizeType
 * @tc.type: FUNC
 */
HWTEST_F(WindowAdapterTest, GetAppDragResizeType, TestSize.Level1)
{
    WindowAdapter windowAdapter;
    windowAdapter.isProxyValid_ = true;
    windowAdapter.windowManagerServiceProxy_ = nullptr;
    DragResizeType dragResizeType = DragResizeType::RESIZE_TYPE_UNDEFINED;
    const std::string bundleName = "test";
    auto err = windowAdapter.GetAppDragResizeType(bundleName, dragResizeType);
    ASSERT_EQ(err, WMError::WM_ERROR_SAMGR);
}

/**
 * @tc.name: SetAppKeyFramePolicy
 * @tc.desc: WindowAdapter/SetAppKeyFramePolicy
 * @tc.type: FUNC
 */
HWTEST_F(WindowAdapterTest, SetAppKeyFramePolicy, TestSize.Level1)
{
    WindowAdapter windowAdapter;
    windowAdapter.isProxyValid_ = true;
    windowAdapter.windowManagerServiceProxy_ = nullptr;
    const std::string bundleName = "test";
    KeyFramePolicy keyFramePolicy;
    keyFramePolicy.dragResizeType_ = DragResizeType::RESIZE_KEY_FRAME;
    auto err = windowAdapter.SetAppKeyFramePolicy(bundleName, keyFramePolicy);
    ASSERT_EQ(err, WMError::WM_ERROR_SAMGR);
}

/**
 * @tc.name: SetParentWindow
 * @tc.desc: WindowAdapter/SetParentWindow
 * @tc.type: FUNC
 */
HWTEST_F(WindowAdapterTest, SetParentWindow, TestSize.Level1)
{
    WindowAdapter windowAdapter;
    windowAdapter.isProxyValid_ = true;
    windowAdapter.windowManagerServiceProxy_ = nullptr;
    int32_t subWindowId = 1;
    int32_t newParentWindowId = 2;
    auto err = windowAdapter.SetParentWindow(subWindowId, newParentWindowId);
    ASSERT_EQ(err, WMError::WM_ERROR_SAMGR);
}

/**
 * @tc.name: NotifyWatchGestureConsumeResult
 * @tc.desc: WindowAdapter/NotifyWatchGestureConsumeResult
 * @tc.type: FUNC
 */
HWTEST_F(WindowAdapterTest, NotifyWatchGestureConsumeResult, TestSize.Level1)
{
    WindowAdapter windowAdapter;
    windowAdapter.isProxyValid_ = true;
    windowAdapter.windowManagerServiceProxy_ = nullptr;
    ASSERT_EQ(WMError::WM_ERROR_SAMGR, windowAdapter.NotifyWatchGestureConsumeResult(0, true));
}

/**
 * @tc.name: NotifyWatchFocusActiveChange
 * @tc.desc: WindowAdapter/NotifyWatchFocusActiveChange
 * @tc.type: FUNC
 */
HWTEST_F(WindowAdapterTest, NotifyWatchFocusActiveChange, TestSize.Level1)
{
    WindowAdapter windowAdapter;
    windowAdapter.isProxyValid_ = true;
    windowAdapter.windowManagerServiceProxy_ = nullptr;
    ASSERT_EQ(WMError::WM_ERROR_SAMGR, windowAdapter.NotifyWatchFocusActiveChange(true));
}

/**
 * @tc.name: MinimizeByWindowId
 * @tc.desc: WindowAdapter/MinimizeByWindowId
 * @tc.type: FUNC
 */
HWTEST_F(WindowAdapterTest, MinimizeByWindowId, TestSize.Level1)
{
    WindowAdapter windowAdapter;
    windowAdapter.isProxyValid_ = true;
    windowAdapter.windowManagerServiceProxy_ = nullptr;
    std::vector<int32_t> windowIds;
    auto err = windowAdapter.MinimizeByWindowId(windowIds);
    ASSERT_EQ(WMError::WM_ERROR_SAMGR, err);
    auto ret = windowAdapter.InitWMSProxy();
    ASSERT_EQ(ret, true);
}

/**
 * @tc.name: ListWindowInfo01
 * @tc.desc: WindowAdapter/ListWindowInfo
 * @tc.type: FUNC
 */
HWTEST_F(WindowAdapterTest, ListWindowInfo01, Function | SmallTest | Level2)
{
    WindowAdapter windowAdapter;
    windowAdapter.isProxyValid_ = true;
    windowAdapter.windowManagerServiceProxy_ = nullptr;
    WindowInfoOption windowInfoOption;
    std::vector<sptr<WindowInfo>> infos;
    auto err = windowAdapter.ListWindowInfo(windowInfoOption, infos);
    ASSERT_EQ(WMError::WM_ERROR_SAMGR, err);
    auto ret = windowAdapter.InitWMSProxy();
    ASSERT_EQ(ret, true);
}

/**
 * @tc.name: RegisterWindowPropertyChangeAgent
 * @tc.desc: check RegisterWindowPropertyChangeAgent
 * @tc.type: FUNC
 */
HWTEST_F(WindowAdapterTest, RegisterWindowPropertyChangeAgent, TestSize.Level1)
{
    WMError ret;
    WindowInfoOption windowInfoOption;
    WindowInfoKey windowInfoKey = WindowInfoKey::NONE;
    uint32_t interestInfo = 0;
    auto agent = sptr<WindowManagerAgent>::MakeSptr();

    // branch 1: send ipc failed
    ASSERT_EQ(true, instance_->InitSSMProxy());
    ret = instance_->RegisterWindowPropertyChangeAgent(windowInfoKey, interestInfo, agent);
    EXPECT_NE(WMError::WM_OK, ret);

    // branch 2: mock send ipc success
    instance_->isProxyValid_ = true;
    instance_->windowManagerServiceProxy_ = nullptr;

    // use mock window manager service proxy.
    auto remoteObject = sptr<IRemoteWindowManagerMocker>::MakeSptr();
    EXPECT_CALL(*remoteObject, RegisterWindowPropertyChangeAgent(_, _, _)).Times(1).WillOnce(Return(WMError::WM_OK));
    auto wmsProxy = iface_cast<IWindowManager>(remoteObject);
    instance_->windowManagerServiceProxy_ = wmsProxy;

    ret = instance_->RegisterWindowPropertyChangeAgent(windowInfoKey, interestInfo, agent);
    EXPECT_EQ(WMError::WM_OK, ret);
}

/**
 * @tc.name: UnregisterWindowPropertyChangeAgent01
 * @tc.desc: WindowAdapter/UnregisterWindowPropertyChangeAgent
 * @tc.type: FUNC
 */
HWTEST_F(WindowAdapterTest, UnregisterWindowPropertyChangeAgent01, Function | SmallTest | Level2)
{
    WindowAdapter windowAdapter;
    WindowInfoOption windowInfoOption;
    WindowInfoKey windowInfoKey = WindowInfoKey::NONE;
    uint32_t interestInfo = 0;
    sptr<IWindowManagerAgent> windowManagerAgent;
    auto err = windowAdapter.UnregisterWindowPropertyChangeAgent(windowInfoKey, interestInfo, windowManagerAgent);
    EXPECT_EQ(WMError::WM_ERROR_NULLPTR, err);
    auto ret = windowAdapter.InitWMSProxy();
    EXPECT_EQ(ret, true);
}

/**
 * @tc.name: CreateUIEffectController
 * @tc.desc: WindowAdapter/CreateUIEffectController
 * @tc.type: FUNC
 */
HWTEST_F(WindowAdapterTest, CreateUIEffectController, Function | SmallTest | Level2)
{
    WindowAdapter windowAdapter;
    auto client = sptr<UIEffectControllerClient>::MakeSptr();
    sptr<IUIEffectController> controller = nullptr;
    int32_t controllerId = 0;
    windowAdapter.CreateUIEffectController(client, controller, controllerId);

    auto ret = windowAdapter.InitWMSProxy();
    EXPECT_EQ(ret, true);
    windowAdapter.CreateUIEffectController(client, controller, controllerId);
}

/**
 * @tc.name: AddSessionBlackList01
 * @tc.desc: WindowAdapter/AddSessionBlackList
 * @tc.type: FUNC
 */
HWTEST_F(WindowAdapterTest, AddSessionBlackList01, Function | SmallTest | Level2)
{
    WindowAdapter windowAdapter;
    windowAdapter.isProxyValid_ = true;
    windowAdapter.windowManagerServiceProxy_ = nullptr;
    std::unordered_set<std::string> bundleNames;
    std::unordered_set<std::string> privacyWindowTags;
    sptr<IWindowManagerAgent> windowManagerAgent;
    auto err = windowAdapter.AddSessionBlackList(bundleNames, privacyWindowTags);
    EXPECT_EQ(WMError::WM_ERROR_SAMGR, err);
    auto ret = windowAdapter.InitWMSProxy();
    EXPECT_EQ(ret, true);
}

/**
 * @tc.name: NotifySupportRotationRegistered
 * @tc.desc: WindowAdapter/NotifySupportRotationRegistered
 * @tc.type: FUNC
 */
HWTEST_F(WindowAdapterTest, NotifySupportRotationRegistered, Function | SmallTest | Level2)
{
    WindowAdapter windowAdapter;
    windowAdapter.isProxyValid_ = true;
    windowAdapter.windowManagerServiceProxy_ = nullptr;
    sptr<IWindowManagerAgent> windowManagerAgent;
    auto err = windowAdapter.NotifySupportRotationRegistered();
    EXPECT_EQ(WMError::WM_ERROR_SAMGR, err);
    auto ret = windowAdapter.InitWMSProxy();
    EXPECT_EQ(ret, true);

    err = windowAdapter.NotifySupportRotationRegistered();
    EXPECT_EQ(err, WMError::WM_ERROR_SAMGR);
}

/**
 * @tc.name: RemoveSessionBlackList01
 * @tc.desc: WindowAdapter/RemoveSessionBlackList
 * @tc.type: FUNC
 */
HWTEST_F(WindowAdapterTest, RemoveSessionBlackList01, Function | SmallTest | Level2)
{
    WindowAdapter windowAdapter;
    windowAdapter.isProxyValid_ = true;
    windowAdapter.windowManagerServiceProxy_ = nullptr;
    std::unordered_set<std::string> bundleNames;
    std::unordered_set<std::string> privacyWindowTags;
    sptr<IWindowManagerAgent> windowManagerAgent;
    auto err = windowAdapter.RemoveSessionBlackList(bundleNames, privacyWindowTags);
    EXPECT_EQ(WMError::WM_ERROR_SAMGR, err);
    auto ret = windowAdapter.InitWMSProxy();
    EXPECT_EQ(ret, true);
}

/**
 * @tc.name: GetPiPSettingSwitchStatus
 * @tc.desc: WindowAdapter/GetPiPSettingSwitchStatus
 * @tc.type: FUNC
 */
HWTEST_F(WindowAdapterTest, GetPiPSettingSwitchStatus, TestSize.Level1)
{
    WindowAdapter windowAdapter;
    bool switchStatus = false;
    auto err = windowAdapter.GetPiPSettingSwitchStatus(switchStatus);
    ASSERT_EQ(WMError::WM_OK, err);
    windowAdapter.isProxyValid_ = true;
    windowAdapter.windowManagerServiceProxy_ = nullptr;
    ASSERT_EQ(WMError::WM_DO_NOTHING, windowAdapter.UseImplicitAnimation(0, true));
}

/**
 *@tc.name: GetIsPipEnabled
 *@tc.desc: WindowAdapter/GetIsPipEnabled
 *@tc.type: FUNC
*/
HWTEST_F(WindowAdapterTest, GetIsPipEnabled, TestSize.Level1)
{
    WindowAdapter windowAdapter;
    bool isPipEnabled = false;
    auto err = windowAdapter.GetIsPipEnabled(isPipEnabled);
    ASSERT_EQ(WMError::WM_OK, err);
    windowAdapter.isProxyValid_ = true;
    windowAdapter.windowManagerServiceProxy_ = nullptr;
    ASSERT_EQ(WMError::WM_DO_NOTHING, windowAdapter.UseImplicitAnimation(0, true));
}

/**
 * @tc.name: UseImplicitAnimation
 * @tc.desc: WindowAdapter/UseImplicitAnimation
 * @tc.type: FUNC
 */
HWTEST_F(WindowAdapterTest, UseImplicitAnimation, TestSize.Level1)
{
    WindowAdapter windowAdapter;
    ASSERT_EQ(WMError::WM_ERROR_INVALID_SESSION, windowAdapter.UseImplicitAnimation(0, true));
}

/**
 * @tc.name: WMSDeathRecipient
 * @tc.desc: normal func
 * @tc.type: FUNC
 */
HWTEST_F(WindowAdapterTest, WMSDeathRecipient, TestSize.Level1)
{
    auto wmsDeath_ = sptr<WMSDeathRecipient>::MakeSptr();
    ASSERT_NE(wmsDeath_, nullptr);

    sptr<IRemoteObject> token = nullptr;
    wmsDeath_->OnRemoteDied(wptr(token));

    token = sptr<IRemoteObjectMocker>::MakeSptr();
    wmsDeath_->OnRemoteDied(wptr(token));
}

/**
 * @tc.name: RegisterAndUnregisterOutlineRecoverCallbackFunc
 * @tc.desc: normal function
 * @tc.type: FUNC
 */
HWTEST_F(WindowAdapterTest, RegisterAndUnregisterOutlineRecoverCallbackFunc, TestSize.Level1)
{
    WindowAdapter windowAdapter;
    auto testFunc = [] {
        return WMError::WM_OK;
    };
    windowAdapter.RegisterOutlineRecoverCallbackFunc(testFunc);
    EXPECT_NE(windowAdapter.outlineRecoverCallbackFunc_, nullptr);
    windowAdapter.UnregisterOutlineRecoverCallbackFunc();
    EXPECT_EQ(windowAdapter.outlineRecoverCallbackFunc_, nullptr);
}

/**
 * @tc.name: UpdateOutline
 * @tc.desc: normal function
 * @tc.type: FUNC
 */
HWTEST_F(WindowAdapterTest, UpdateOutline, TestSize.Level1)
{
    WindowAdapter windowAdapter;
    windowAdapter.isProxyValid_ = true;
    windowAdapter.windowManagerServiceProxy_ = nullptr;
    OutlineParams params;
    auto ret = windowAdapter.UpdateOutline(nullptr, params);
    EXPECT_EQ(WMError::WM_ERROR_SAMGR, ret);
}

/**
 * @tc.name: GetInstance
 * @tc.desc: normal function
 * @tc.type: FUNC
 */
HWTEST_F(WindowAdapterTest, GetInstance, TestSize.Level1)
{
    sptr<WindowAdapter> instance = nullptr;
    int32_t userId = -1;
    instance = &WindowAdapter::GetInstance(userId);
    ASSERT_NE(instance, nullptr);

    userId = 101;
    instance = &WindowAdapter::GetInstance(userId);
    ASSERT_NE(instance, nullptr);

    // branch overried
    instance = &WindowAdapter::GetInstance(userId);
    ASSERT_NE(instance, nullptr);
}

/**
 * @tc.name: InitSSMProxy
 * @tc.desc: normal function
 * @tc.type: FUNC
 */
HWTEST_F(WindowAdapterTest, InitSSMProxy, TestSize.Level1)
{
    // branch 1
    instance_->isProxyValid_ = true;
    ASSERT_EQ(true, instance_->InitSSMProxy());

    // branch 2
    instance_->isProxyValid_ = false;
    instance_->recoverInitialized_ = true;
    instance_->InitSSMProxy();

    // branch 3
    ASSERT_NE(nullptr, instance_);
    instance_->recoverInitialized_ = false;
    instance_->InitSSMProxy();
}

/**
 * @tc.name: UnregisterWMSConnectionChangedListener
 * @tc.desc: normal function
 * @tc.type: FUNC
 */
HWTEST_F(WindowAdapterTest, UnregisterWMSConnectionChangedListener, TestSize.Level1)
{
    ASSERT_NE(nullptr, instance_);
    instance_->UnregisterWMSConnectionChangedListener();
}

/**
 * @tc.name: RegisterWindowManagerAgentWhenSCBFault
 * @tc.desc: normal function
 * @tc.type: FUNC
 */
HWTEST_F(WindowAdapterTest, RegisterWindowManagerAgentWhenSCBFault, TestSize.Level1)
{
    WindowManagerAgentType type = WindowManagerAgentType::WINDOW_MANAGER_AGENT_TYPE_FOCUS;
    sptr<IWindowManagerAgent> agent = new WindowManagerAgent();
    ASSERT_NE(nullptr, instance_);

    // This function does not require assertions.
    instance_->RegisterWindowManagerAgentWhenSCBFault(type, agent);
}

/**
 * @tc.name: RegisterWindowManagerAgent
 * @tc.desc: normal function
 * @tc.type: FUNC
 */
HWTEST_F(WindowAdapterTest, RegisterWindowManagerAgent, TestSize.Level1)
{
    WMError ret;
    auto type = WindowManagerAgentType::WINDOW_MANAGER_AGENT_TYPE_FOCUS;
    auto agent = sptr<WindowManagerAgent>::MakeSptr();

    // branch 1: send ipc failed
    ASSERT_EQ(true, instance_->InitSSMProxy());
    ret = instance_->RegisterWindowManagerAgent(type, agent);
    EXPECT_NE(WMError::WM_OK, ret);

    // branch 2: mock proxy and return ok
    instance_->isProxyValid_ = true;
    instance_->windowManagerServiceProxy_ = nullptr;

    auto remoteObject = sptr<IRemoteWindowManagerMocker>::MakeSptr();
    EXPECT_CALL(*remoteObject, RegisterWindowManagerAgent(_, _)).Times(1).WillOnce(Return(WMError::WM_OK));
    auto wmsProxy = iface_cast<IWindowManager>(remoteObject);
    instance_->windowManagerServiceProxy_ = wmsProxy;

    ret = instance_->RegisterWindowManagerAgent(type, agent);
    EXPECT_EQ(WMError::WM_OK, ret);
}

/**
 * @tc.name: UnregisterWindowManagerAgent
 * @tc.desc: check UnregisterWindowManagerAgent
 * @tc.type: FUNC
 */
HWTEST_F(WindowAdapterTest, UnregisterWindowManagerAgent, TestSize.Level1)
{
    g_errLog.clear();
    LOG_SetCallback(MyLogCallback);
    WMError ret;
    auto type = WindowManagerAgentType::WINDOW_MANAGER_AGENT_TYPE_FOCUS;
    auto agent = sptr<WindowManagerAgent>::MakeSptr();
    instance_->isProxyValid_ = true; // skip InitSSMProxy
    instance_->windowManagerServiceProxy_ = nullptr;

    // mock window manager service proxy
    auto remoteObject = sptr<IRemoteWindowManagerMocker>::MakeSptr();
    EXPECT_CALL(*remoteObject, UnregisterWindowManagerAgent(_, _)).Times(3)
        .WillOnce(Return(WMError::WM_ERROR_SAMGR))
        .WillRepeatedly(Return(WMError::WM_OK));
    auto wmProxy = iface_cast<IWindowManager>(remoteObject);
    instance_->windowManagerServiceProxy_ = wmProxy;

    // branch 1: return failed
    g_errLog.clear();
    ret = instance_->UnregisterWindowManagerAgent(type, agent);
    EXPECT_TRUE(g_errLog.find("unregister failed due to proxy") != std::string::npos);
    EXPECT_NE(WMError::WM_OK, ret);

    // branch 2-1: agent not found
    g_errLog.clear();
    instance_->windowManagerAgentMap_.clear();
    ret = instance_->UnregisterWindowManagerAgent(type, agent);
    EXPECT_TRUE(g_errLog.find("agent not found") != std::string::npos);
    EXPECT_EQ(WMError::WM_OK, ret);

    // branch 2-2: erase success
    g_errLog.clear();
    instance_->windowManagerAgentMap_[type].insert(agent);
    ret = instance_->UnregisterWindowManagerAgent(type, agent);
    EXPECT_TRUE(g_errLog.find("erase success") != std::string::npos);
    EXPECT_EQ(WMError::WM_OK, ret);

    LOG_SetCallback(nullptr);
}

/**
 * @tc.name: ReregisterWindowManagerFaultAgent
 * @tc.desc: normal function
 * @tc.type: FUNC
 */
HWTEST_F(WindowAdapterTest, ReregisterWindowManagerFaultAgent, TestSize.Level1)
{
    LOG_SetCallback(MyLogCallback);
    sptr<IWindowManager> wmsProxy = nullptr;

    // branch 1
    g_errLog.clear();
    instance_->windowManagerServiceProxy_ = nullptr;
    instance_->ReregisterWindowManagerFaultAgent(wmsProxy);
    EXPECT_TRUE(g_errLog.find("proxy is null") != std::string::npos);

    // use mock window manager service proxy
    instance_->isProxyValid_ = true;
    instance_->windowManagerServiceProxy_ = nullptr;
    auto remoteObject = sptr<IRemoteWindowManagerMocker>::MakeSptr();
    EXPECT_CALL(*remoteObject, RegisterWindowManagerAgent(_, _)).Times(2)
        .WillOnce(Return(WMError::WM_ERROR_SAMGR))
        .WillRepeatedly(Return(WMError::WM_OK));
    wmsProxy = iface_cast<IWindowManager>(remoteObject);

    // branch 2
    g_errLog.clear();
    instance_->windowManagerAgentFaultMap_.clear();
    instance_->ReregisterWindowManagerFaultAgent(wmsProxy);
    EXPECT_TRUE(g_errLog.find("no fault agent") != std::string::npos);

    // branch 3: enter map, mock send ipc failed
    g_errLog.clear();
    WindowManagerAgentType type = WindowManagerAgentType::WINDOW_MANAGER_AGENT_TYPE_FOCUS;
    auto agent = sptr<WindowManagerAgent>::MakeSptr();
    instance_->windowManagerAgentFaultMap_[type].insert(agent);
    instance_->ReregisterWindowManagerFaultAgent(wmsProxy);
    EXPECT_TRUE(g_errLog.find("Re-register agent failed") != std::string::npos);

    // branch 4: enter map, mock send ipc success
    g_errLog.clear();
    instance_->ReregisterWindowManagerFaultAgent(wmsProxy);
    EXPECT_TRUE(g_errLog.find("end") != std::string::npos);

    LOG_SetCallback(nullptr);
}

/**
 * @tc.name: SetSpecificSystemWindowZIndex
 * @tc.desc: SetSpecificSystemWindowZIndex
 * @tc.type: FUNC
 */
HWTEST_F(WindowAdapterTest, SetSpecificSystemWindowZIndex, TestSize.Level1)
{
    WindowAdapter windowAdapter;
    auto ret = windowAdapter.InitWMSProxy();
    ASSERT_EQ(ret, true);

    auto result = windowAdapter.SetSpecificWindowZIndex(
        WindowType::WINDOW_TYPE_WALLET_SWIPE_CARD, 20);
    EXPECT_NE(WMError::WM_OK, result);
}

/**
 * @tc.name: RecoverSpecificZIndexSetByApp
 * @tc.desc: RecoverSpecificZIndexSetByApp
 * @tc.type: FUNC
 */
HWTEST_F(WindowAdapterTest, RecoverSpecificZIndexSetByApp, TestSize.Level1)
{
    WindowAdapter windowAdapter;
    sptr<WindowAdapter> instance = &WindowAdapter::GetInstance(-1);
    ASSERT_NE(nullptr, instance);
    instance->RecoverSpecificZIndexSetByApp();

    instance->specificZIndexMap_[WindowType::WINDOW_TYPE_WALLET_SWIPE_CARD] = 20;
    instance->RecoverSpecificZIndexSetByApp();
}
} // namespace
} // namespace Rosen
} // namespace OHOS
