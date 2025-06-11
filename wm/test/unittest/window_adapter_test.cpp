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
#include "mock_RSIWindowAnimationController.h"

#include "remote_animation.h"
#include "scene_board_judgement.h"
#include "session_manager.h"
#include "starting_window.h"
#include "window_adapter.h"
#include "window_agent.h"
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
};

void WindowAdapterTest::SetUpTestCase() {}

void WindowAdapterTest::TearDownTestCase() {}

void WindowAdapterTest::SetUp() {}

void WindowAdapterTest::TearDown() {}

namespace {
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
 
    auto result = windowAdapter.RequestFocusStatusBySA(
        persistentId, isFocused, byForeground, reason);
    EXPECT_EQ(result, WMError::WM_ERROR_DEVICE_NOT_SUPPORT);
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
        ret = 1;
        return WMError::WM_DO_NOTHING;
    };

    auto testFunc2 = [&ret] {
        ret = 2;
        return WMError::WM_OK;
    };
    windowAdapter.RegisterSessionRecoverCallbackFunc(persistentId, testFunc);
    windowAdapter.WindowManagerAndSessionRecover();
    if (SceneBoardJudgement::IsSceneBoardEnabled()) {
        ASSERT_EQ(ret, 1);
    }
    windowAdapter.RegisterSessionRecoverCallbackFunc(persistentId, testFunc2);
    windowAdapter.WindowManagerAndSessionRecover();
    if (SceneBoardJudgement::IsSceneBoardEnabled()) {
        ASSERT_EQ(ret, 2);
    } else {
        ASSERT_EQ(ret, 0);
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
    windowAdapter.AddExtensionWindowStageToSCB(nullptr, nullptr, 0);
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
    std::vector<sptr<WindowVisibilityInfo>> infos;
    auto ret = windowAdapter.GetVisibilityWindowInfo(infos);
    windowAdapter.WindowManagerAndSessionRecover();
    ASSERT_EQ(WMError::WM_OK, ret);
}

/**
 * @tc.name: ReregisterWindowManagerAgent
 * @tc.desc: WindowAdapter/ReregisterWindowManagerAgent
 * @tc.type: FUNC
 */
HWTEST_F(WindowAdapterTest, ReregisterWindowManagerAgent, TestSize.Level1)
{
    WindowAdapter windowAdapter;
    auto displayId = 0;
    ModeChangeHotZones hotZones;
    auto ret = windowAdapter.GetModeChangeHotZones(displayId, hotZones);
    windowAdapter.ReregisterWindowManagerAgent();
    ASSERT_EQ(WMError::WM_OK, ret);
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
    auto ret = windowAdapter.UpdateProperty(windowProperty, action);
    windowAdapter.OnUserSwitch();
    windowAdapter.ClearWindowAdapter();
    ASSERT_EQ(WMError::WM_OK, ret);
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
    windowAdapter.windowManagerServiceProxy_ = nullptr;
    uint32_t windowId = 0;
    WindowGravity gravity = WindowGravity::WINDOW_GRAVITY_FLOAT;
    uint32_t percent = 0;
    auto ret = windowAdapter.SetWindowGravity(windowId, gravity, percent);
    ASSERT_EQ(WMError::WM_ERROR_SAMGR, ret);
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
    windowAdapter.windowManagerServiceProxy_ = nullptr;
    sptr<WindowTransitionInfo> from = nullptr;
    sptr<WindowTransitionInfo> to = nullptr;
    auto ret = windowAdapter.NotifyWindowTransition(from, to);
    ASSERT_EQ(WMError::WM_ERROR_SAMGR, ret);
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
    windowAdapter.isProxyValid_ = true;
    windowAdapter.windowManagerServiceProxy_ = nullptr;
    auto ret = windowAdapter.GetMaximizeMode();
    ASSERT_EQ(MaximizeMode::MODE_FULL_FILL, ret);
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
    int32_t persistentId = 0;
    SessionGravity gravity = SessionGravity::SESSION_GRAVITY_FLOAT;
    uint32_t percent = 0;
    auto ret = windowAdapter.SetSessionGravity(persistentId, gravity, percent);
    ASSERT_EQ(WMError::WM_OK, ret);
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
    int32_t hostWindowId = 0;
    Rect rect = { 0, 0, 0, 0 };
    auto ret = windowAdapter.GetHostWindowRect(hostWindowId, rect);
    ASSERT_EQ(WMError::WM_OK, ret);
}

/**
 * @tc.name: GetHostGlobalScaledRect
 * @tc.desc: WindowAdapter/GetHostGlobalScaledRect
 * @tc.type: FUNC
 */
HWTEST_F(WindowAdapterTest, GetHostGlobalScaledRect, TestSize.Level1)
{
    WindowAdapter windowAdapter;
    int32_t hostWindowId = 0;
    Rect rect = { 0, 0, 0, 0 };
    auto ret = windowAdapter.GetHostGlobalScaledRect(hostWindowId, rect);
    ASSERT_EQ(WMError::WM_OK, ret);
}

/**
 * @tc.name: GetWindowStyleType
 * @tc.desc: WindowAdapter/GetWindowStyleType
 * @tc.type: FUNC
 */
HWTEST_F(WindowAdapterTest, GetWindowStyleType, TestSize.Level1)
{
    WindowAdapter windowAdapter;
    WindowStyleType windowStyleType = Rosen::WindowStyleType::WINDOW_STYLE_DEFAULT;
    windowAdapter.GetWindowStyleType(windowStyleType);
    ASSERT_EQ(Rosen::WindowStyleType::WINDOW_STYLE_DEFAULT, windowStyleType);
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
 * @tc.name: UpdateScreenLockStatusForApp
 * @tc.desc: WindowAdapter/UpdateScreenLockStatusForApp
 * @tc.type: FUNC
 */
HWTEST_F(WindowAdapterTest, UpdateScreenLockStatusForApp, TestSize.Level1)
{
    WindowAdapter windowAdapter;
    auto err = windowAdapter.UpdateScreenLockStatusForApp("", true);
    ASSERT_EQ(err, WMError::WM_OK);
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
    bool isPcWindow = false;
    auto err = windowAdapter.IsPcWindow(isPcWindow);
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
    bool isPcOrPadFreeMultiWindowMode = false;
    auto err = windowAdapter.IsPcOrPadFreeMultiWindowMode(isPcOrPadFreeMultiWindowMode);
    ASSERT_EQ(err, WMError::WM_OK);
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
    ASSERT_EQ(err, WMError::WM_OK);
    auto ret = windowAdapter.InitWMSProxy();
    ASSERT_EQ(ret, true);
}

/**
 * @tc.name: GetDisplayIdByWindowId
 * @tc.desc: WindowAdapter/GetDisplayIdByWindowId
 * @tc.type: FUNC
 */
HWTEST_F(WindowAdapterTest, GetDisplayIdByWindowId, TestSize.Level1)
{
    WindowAdapter windowAdapter;
    const std::vector<uint64_t> windowIds = { 1, 2 };
    std::unordered_map<uint64_t, DisplayId> windowDisplayIdMap;
    auto err = windowAdapter.GetDisplayIdByWindowId(windowIds, windowDisplayIdMap);
    ASSERT_EQ(err, WMError::WM_OK);
}

/**
 * @tc.name: SetGlobalDragResizeType
 * @tc.desc: WindowAdapter/SetGlobalDragResizeType
 * @tc.type: FUNC
 */
HWTEST_F(WindowAdapterTest, SetGlobalDragResizeType, TestSize.Level1)
{
    WindowAdapter windowAdapter;
    DragResizeType dragResizeType = DragResizeType::RESIZE_EACH_FRAME;
    auto err = windowAdapter.SetGlobalDragResizeType(dragResizeType);
    ASSERT_EQ(err, WMError::WM_OK);
}

/**
 * @tc.name: GetGlobalDragResizeType
 * @tc.desc: WindowAdapter/GetGlobalDragResizeType
 * @tc.type: FUNC
 */
HWTEST_F(WindowAdapterTest, GetGlobalDragResizeType, TestSize.Level1)
{
    WindowAdapter windowAdapter;
    DragResizeType dragResizeType = DragResizeType::RESIZE_TYPE_UNDEFINED;
    auto err = windowAdapter.GetGlobalDragResizeType(dragResizeType);
    ASSERT_EQ(err, WMError::WM_OK);
}

/**
 * @tc.name: SetAppDragResizeType
 * @tc.desc: WindowAdapter/SetAppDragResizeType
 * @tc.type: FUNC
 */
HWTEST_F(WindowAdapterTest, SetAppDragResizeType, TestSize.Level1)
{
    WindowAdapter windowAdapter;
    DragResizeType dragResizeType = DragResizeType::RESIZE_EACH_FRAME;
    const std::string bundleName = "test";
    auto err = windowAdapter.SetAppDragResizeType(bundleName, dragResizeType);
    ASSERT_EQ(err, WMError::WM_OK);
}

/**
 * @tc.name: GetAppDragResizeType
 * @tc.desc: WindowAdapter/GetAppDragResizeType
 * @tc.type: FUNC
 */
HWTEST_F(WindowAdapterTest, GetAppDragResizeType, TestSize.Level1)
{
    WindowAdapter windowAdapter;
    DragResizeType dragResizeType = DragResizeType::RESIZE_TYPE_UNDEFINED;
    const std::string bundleName = "test";
    auto err = windowAdapter.GetAppDragResizeType(bundleName, dragResizeType);
    ASSERT_EQ(err, WMError::WM_OK);
}

/**
 * @tc.name: SetAppKeyFramePolicy
 * @tc.desc: WindowAdapter/SetAppKeyFramePolicy
 * @tc.type: FUNC
 */
HWTEST_F(WindowAdapterTest, SetAppKeyFramePolicy, TestSize.Level1)
{
    WindowAdapter windowAdapter;
    const std::string bundleName = "test";
    KeyFramePolicy keyFramePolicy;
    keyFramePolicy.dragResizeType_ = DragResizeType::RESIZE_KEY_FRAME;
    auto err = windowAdapter.SetAppKeyFramePolicy(bundleName, keyFramePolicy);
    ASSERT_EQ(err, WMError::WM_OK);
}

/**
 * @tc.name: SetParentWindow
 * @tc.desc: WindowAdapter/SetParentWindow
 * @tc.type: FUNC
 */
HWTEST_F(WindowAdapterTest, SetParentWindow, TestSize.Level1)
{
    WindowAdapter windowAdapter;
    int32_t subWindowId = 1;
    int32_t newParentWindowId = 2;
    auto err = windowAdapter.SetParentWindow(subWindowId, newParentWindowId);
    ASSERT_EQ(err, WMError::WM_OK);
}

/**
 * @tc.name: GetHostWindowCompatiblityInfo
 * @tc.desc: WindowAdapter/GetHostWindowCompatiblityInfo
 * @tc.type: FUNC
 */
HWTEST_F(WindowAdapterTest, GetHostWindowCompatiblityInfo, TestSize.Level1)
{
    WindowAdapter windowAdapter;
    sptr<CompatibleModeProperty> property = sptr<CompatibleModeProperty>::MakeSptr();
    sptr<IRemoteObject> token = sptr<IRemoteObjectMocker>::MakeSptr();
    auto err = windowAdapter.GetHostWindowCompatiblityInfo(token, property);
    ASSERT_EQ(err, WMError::WM_OK);
}

/**
 * @tc.name: MinimizeByWindowId
 * @tc.desc: WindowAdapter/MinimizeByWindowId
 * @tc.type: FUNC
 */
HWTEST_F(WindowAdapterTest, MinimizeByWindowId, TestSize.Level1)
{
    WindowAdapter windowAdapter;
    std::vector<int32_t> windowIds;
    auto err = windowAdapter.MinimizeByWindowId(windowIds);
    ASSERT_EQ(WMError::WM_OK, err);
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
    WindowInfoOption windowInfoOption;
    std::vector<sptr<WindowInfo>> infos;
    auto err = windowAdapter.ListWindowInfo(windowInfoOption, infos);
    ASSERT_EQ(WMError::WM_ERROR_DEVICE_NOT_SUPPORT, err);
    auto ret = windowAdapter.InitWMSProxy();
    ASSERT_EQ(ret, true);
}

/**
 * @tc.name: RegisterWindowPropertyChangeAgent01
 * @tc.desc: WindowAdapter/RegisterWindowPropertyChangeAgent
 * @tc.type: FUNC
 */
HWTEST_F(WindowAdapterTest, RegisterWindowPropertyChangeAgent01, Function | SmallTest | Level2)
{
    WindowAdapter windowAdapter;
    WindowInfoOption windowInfoOption;
    WindowInfoKey windowInfoKey = WindowInfoKey::NONE;
    uint32_t interestInfo = 0;
    sptr<IWindowManagerAgent> windowManagerAgent;
    auto err = windowAdapter.RegisterWindowPropertyChangeAgent(windowInfoKey, interestInfo, windowManagerAgent);
    ASSERT_EQ(WMError::WM_ERROR_DEVICE_NOT_SUPPORT, err);
    auto ret = windowAdapter.InitWMSProxy();
    ASSERT_EQ(ret, true);
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
    ASSERT_EQ(WMError::WM_ERROR_DEVICE_NOT_SUPPORT, err);
    auto ret = windowAdapter.InitWMSProxy();
    ASSERT_EQ(ret, true);
}
} // namespace
} // namespace Rosen
} // namespace OHOS