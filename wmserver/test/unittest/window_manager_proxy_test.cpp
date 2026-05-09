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

#include <gtest/gtest.h>
#include "window_manager_proxy.h"
#include "window_manager_stub_impl.h"
#include "iremote_object_mocker.h"
#include "mock_message_parcel.h"
#include <rs_window_animation_target.h>

using namespace testing;
using namespace testing::ext;
namespace OHOS {
namespace Rosen {
class WindowManagerProxyTest : public testing::Test {
public:
    static void SetUpTestCase();
    static void TearDownTestCase();
    void SetUp() override;
    void TearDown() override;
    sptr<WindowManagerStubImpl> mockWindowManagerStub_;
    sptr<WindowManagerProxy> windowManagerProxy_;
};

void WindowManagerProxyTest::SetUpTestCase()
{
}

void WindowManagerProxyTest::TearDownTestCase()
{
}

void WindowManagerProxyTest::SetUp()
{
    mockWindowManagerStub_ = new WindowManagerStubImpl();
    windowManagerProxy_ = new WindowManagerProxy(mockWindowManagerStub_);
}

void WindowManagerProxyTest::TearDown()
{
}

namespace {
/**
 * @tc.name: RequestFocus
 * @tc.desc: test success
 * @tc.type: FUNC
 */
HWTEST_F(WindowManagerProxyTest, RequestFocus, TestSize.Level1)
{
    uint32_t windowId = 0;
    WMError err = windowManagerProxy_->RequestFocus(windowId);
    ASSERT_EQ(err, WMError::WM_OK);
}

/**
 * @tc.name: SetWindowAnimationController
 * @tc.desc: test failed
 * @tc.type: FUNC
 */
HWTEST_F(WindowManagerProxyTest, SetWindowAnimationController, TestSize.Level1)
{
    sptr<RSIWindowAnimationController> controller = nullptr;
    WMError err = windowManagerProxy_->SetWindowAnimationController(controller);
    ASSERT_EQ(err, WMError::WM_ERROR_IPC_FAILED);
}

/**
 * @tc.name: SetWindowAnimationController01
 * @tc.desc: test success
 * @tc.type: FUNC
 */
HWTEST_F(WindowManagerProxyTest, SetWindowAnimationController01, TestSize.Level1)
{
    sptr<IRemoteObject> iRemoteObjectMocker = new IRemoteObjectMocker();
    sptr<RSIWindowAnimationController> controller = iface_cast<RSIWindowAnimationController>(iRemoteObjectMocker);
    WMError err = windowManagerProxy_->SetWindowAnimationController(controller);
    ASSERT_EQ(err, WMError::WM_OK);
}

/**
 * @tc.name: ToggleShownStateForAllAppWindows
 * @tc.desc: test success
 * @tc.type: FUNC
 */
HWTEST_F(WindowManagerProxyTest, ToggleShownStateForAllAppWindows, TestSize.Level1)
{
    WMError err = windowManagerProxy_->ToggleShownStateForAllAppWindows();
    ASSERT_EQ(err, WMError::WM_OK);
}

/**
 * @tc.name: GetTopWindowId
 * @tc.desc: test success
 * @tc.type: FUNC
 */
HWTEST_F(WindowManagerProxyTest, GetTopWindowId, TestSize.Level1)
{
    uint32_t mainWinId = 0;
    uint32_t topWinId;
    WMError err = windowManagerProxy_->GetTopWindowId(mainWinId, topWinId);
    ASSERT_EQ(err, WMError::WM_OK);
}

/**
 * @tc.name: GetSnapshotByWindowId
 * @tc.desc: test GetSnapshotByWindowId
 * @tc.type: FUNC
 */
HWTEST_F(WindowManagerProxyTest, GetSnapshotByWindowId, TestSize.Level1)
{
    int windowId = INVALID_WINDOW_ID;
    std::shared_ptr<Media::PixelMap> pixelMap = nullptr;
    WMError err = windowManagerProxy_->GetSnapshotByWindowId(windowId, pixelMap);
    ASSERT_EQ(err, WMError::WM_ERROR_NULLPTR);
}

/**
 * @tc.name: NotifyWindowTransition
 * @tc.desc: test success
 * @tc.type: FUNC
 */
HWTEST_F(WindowManagerProxyTest, NotifyWindowTransition, TestSize.Level1)
{
    sptr<WindowTransitionInfo> from = new WindowTransitionInfo();
    sptr<WindowTransitionInfo> to = new WindowTransitionInfo();
    bool isFromClient = false;
    WMError err = windowManagerProxy_->NotifyWindowTransition(from, to, isFromClient);
    ASSERT_EQ(err, WMError::WM_OK);
}

/**
 * @tc.name: GetModeChangeHotZones
 * @tc.desc: test success
 * @tc.type: FUNC
 */
HWTEST_F(WindowManagerProxyTest, GetModeChangeHotZones, TestSize.Level1)
{
    DisplayId displayId = 10;
    ModeChangeHotZones hotZones;
    WMError err = windowManagerProxy_->GetModeChangeHotZones(displayId, hotZones);
    ASSERT_EQ(err, WMError::WM_OK);
}

/**
 * @tc.name: MinimizeWindowsByLauncher
 * @tc.desc: test success
 * @tc.type: FUNC
 */
HWTEST_F(WindowManagerProxyTest, MinimizeWindowsByLauncher, TestSize.Level1)
{
    std::vector<uint32_t> windowIds;
    bool isAnimated = false;
    sptr<RSIWindowAnimationFinishedCallback> finishCallback;
    windowManagerProxy_->MinimizeWindowsByLauncher(windowIds, isAnimated, finishCallback);
    ASSERT_EQ(finishCallback, nullptr);
}

/**
 * @tc.name: MinimizeWindowsByLauncher01
 * @tc.desc: test success
 * @tc.type: FUNC
 */
HWTEST_F(WindowManagerProxyTest, MinimizeWindowsByLauncher01, TestSize.Level1)
{
    std::vector<uint32_t> windowIds;
    windowIds.push_back(0);
    windowIds.push_back(1);
    bool isAnimated = false;
    sptr<RSIWindowAnimationFinishedCallback> finishCallback;
    windowManagerProxy_->MinimizeWindowsByLauncher(windowIds, isAnimated, finishCallback);
    ASSERT_EQ(finishCallback, nullptr);
}

/**
 * @tc.name: UpdateRsTree
 * @tc.desc: test success
 * @tc.type: FUNC
 */
HWTEST_F(WindowManagerProxyTest, UpdateRsTree, TestSize.Level1)
{
    uint32_t windowId = 0;
    bool isAdd = false;
    WMError err = windowManagerProxy_->UpdateRsTree(windowId, isAdd);
    ASSERT_EQ(err, WMError::WM_OK);
}

/**
 * @tc.name: BindDialogTarget
 * @tc.desc: test success
 * @tc.type: FUNC
 */
HWTEST_F(WindowManagerProxyTest, BindDialogTarget, TestSize.Level1)
{
    uint32_t windowId = 0;
    sptr<IRemoteObject> targetToken = nullptr;
    WMError err = windowManagerProxy_->BindDialogTarget(windowId, targetToken);
    ASSERT_EQ(err, WMError::WM_ERROR_IPC_FAILED);
}

/**
 * @tc.name: BindDialogTarget01
 * @tc.desc: test success
 * @tc.type: FUNC
 */
HWTEST_F(WindowManagerProxyTest, BindDialogTarget01, TestSize.Level1)
{
    uint32_t windowId = 0;
    sptr<IRemoteObject> targetToken = nullptr;
    WMError err = windowManagerProxy_->BindDialogTarget(windowId, targetToken);
    ASSERT_EQ(err, WMError::WM_ERROR_IPC_FAILED);
}

/**
 * @tc.name: GetVisibilityWindowInfo01
 * @tc.desc: test success
 * @tc.type: FUNC
 */
HWTEST_F(WindowManagerProxyTest, GetVisibilityWindowInfo01, TestSize.Level1)
{
    std::vector<sptr<WindowVisibilityInfo>> infos;
    WMError err = windowManagerProxy_->GetVisibilityWindowInfo(infos);
    ASSERT_EQ(err, WMError::WM_OK);
}

/**
 * @tc.name: GetUnreliableWindowInfo01
 * @tc.desc: test success
 * @tc.type: FUNC
 */
HWTEST_F(WindowManagerProxyTest, GetUnreliableWindowInfo01, TestSize.Level1)
{
    std::vector<sptr<UnreliableWindowInfo>> infos;
    int32_t windowId = 0;
    WMError err = windowManagerProxy_->GetUnreliableWindowInfo(windowId, infos);
    ASSERT_EQ(err, WMError::WM_OK);
}

/**
 * @tc.name: GetWindowAnimationTargets01
 * @tc.desc: test success
 * @tc.type: FUNC
 */
HWTEST_F(WindowManagerProxyTest, GetWindowAnimationTargets01, TestSize.Level1)
{
    std::vector<uint32_t> missionIds;
    missionIds.push_back(0);
    missionIds.push_back(1);
    std::vector<sptr<RSWindowAnimationTarget>> targets;
    WMError err = windowManagerProxy_->GetWindowAnimationTargets(missionIds, targets);
    ASSERT_EQ(err, WMError::WM_OK);
}

/**
 * @tc.name: RemoveWindow
 * @tc.desc: test success
 * @tc.type: FUNC
 */
HWTEST_F(WindowManagerProxyTest, RemoveWindow, TestSize.Level1)
{
    uint32_t windowId = 0;
    bool isFromInnerkits = true;
    WMError err = windowManagerProxy_->RemoveWindow(windowId, isFromInnerkits);
    EXPECT_NE(err, static_cast<WMError>(1));
}

/**
 * @tc.name: DestroyWindow
 * @tc.desc: test success
 * @tc.type: FUNC
 */
HWTEST_F(WindowManagerProxyTest, DestroyWindow, TestSize.Level1)
{
    uint32_t windowId = 0;
    bool isFromInnerkits = true;
    WMError err = windowManagerProxy_->DestroyWindow(windowId, isFromInnerkits);
    EXPECT_NE(err, static_cast<WMError>(1));
}

/**
 * @tc.name: GetAvoidAreaByType
 * @tc.desc: test success
 * @tc.type: FUNC
 */
HWTEST_F(WindowManagerProxyTest, GetAvoidAreaByType, TestSize.Level1)
{
    uint32_t windowId = 0;
    AvoidAreaType type = AvoidAreaType::TYPE_SYSTEM;
    AvoidArea avoidArea;
    AvoidArea err = windowManagerProxy_->GetAvoidAreaByType(windowId, type);
    EXPECT_EQ(err, avoidArea);
}

/**
 * @tc.name: RegisterWindowManagerAgent
 * @tc.desc: test success
 * @tc.type: FUNC
 */
HWTEST_F(WindowManagerProxyTest, RegisterWindowManagerAgent, TestSize.Level1)
{
    MessageParcel reply;
    sptr<IWindowManagerAgent> windowManagerAgent = new WindowManagerAgent();
    WindowManagerAgentType type = WindowManagerAgentType::WINDOW_MANAGER_AGENT_TYPE_FOCUS;
    WMError err = windowManagerProxy_->RegisterWindowManagerAgent(type, windowManagerAgent);
    EXPECT_EQ(err, static_cast<WMError>(reply.ReadInt32()));
}

/**
 * @tc.name: UnregisterWindowManagerAgent
 * @tc.desc: test success
 * @tc.type: FUNC
 */
HWTEST_F(WindowManagerProxyTest, UnregisterWindowManagerAgent, TestSize.Level1)
{
    MessageParcel reply;
    sptr<IWindowManagerAgent> windowManagerAgent = new WindowManagerAgent();
    WindowManagerAgentType type = WindowManagerAgentType::WINDOW_MANAGER_AGENT_TYPE_FOCUS;
    WMError err = windowManagerProxy_->UnregisterWindowManagerAgent(type, windowManagerAgent);
    EXPECT_EQ(err, static_cast<WMError>(reply.ReadInt32()));
}

/**
 * @tc.name: NotifyServerReadyToMoveOrDrag
 * @tc.desc: test success
 * @tc.type: FUNC
 */
HWTEST_F(WindowManagerProxyTest, NotifyServerReadyToMoveOrDrag, TestSize.Level1)
{
    uint32_t windowId = 0;
    sptr<WindowProperty> windowProperty;
    sptr<MoveDragProperty> moveDragProperty;
    MessageParcel reply;
    sptr<IWindowManagerAgent> windowManagerAgent = new WindowManagerAgent();
    WindowManagerAgentType type = WindowManagerAgentType::WINDOW_MANAGER_AGENT_TYPE_FOCUS;
    windowManagerProxy_->NotifyServerReadyToMoveOrDrag(windowId, windowProperty, moveDragProperty);
    WMError err = windowManagerProxy_->UnregisterWindowManagerAgent(type, windowManagerAgent);
    EXPECT_EQ(err, static_cast<WMError>(reply.ReadInt32()));
}

/**
 * @tc.name: ProcessPointDown
 * @tc.desc: test success
 * @tc.type: FUNC
 */
HWTEST_F(WindowManagerProxyTest, ProcessPointDown, TestSize.Level1)
{
    uint32_t windowId = 0;
    bool isPointDown = true;
    MessageParcel reply;
    sptr<IWindowManagerAgent> windowManagerAgent = new WindowManagerAgent();
    WindowManagerAgentType type = WindowManagerAgentType::WINDOW_MANAGER_AGENT_TYPE_FOCUS;
    windowManagerProxy_->ProcessPointDown(windowId, isPointDown);
    WMError err = windowManagerProxy_->UnregisterWindowManagerAgent(type, windowManagerAgent);
    EXPECT_EQ(err, static_cast<WMError>(reply.ReadInt32()));
}

/**
 * @tc.name: ProcessPointUp
 * @tc.desc: test success
 * @tc.type: FUNC
 */
HWTEST_F(WindowManagerProxyTest, ProcessPointUp, TestSize.Level1)
{
    uint32_t windowId = 0;
    MessageParcel reply;
    sptr<IWindowManagerAgent> windowManagerAgent = new WindowManagerAgent();
    WindowManagerAgentType type = WindowManagerAgentType::WINDOW_MANAGER_AGENT_TYPE_FOCUS;
    windowManagerProxy_->ProcessPointUp(windowId);
    WMError err = windowManagerProxy_->UnregisterWindowManagerAgent(type, windowManagerAgent);
    EXPECT_EQ(err, static_cast<WMError>(reply.ReadInt32()));
}

/**
 * @tc.name: MinimizeAllAppWindows
 * @tc.desc: test success
 * @tc.type: FUNC
 */
HWTEST_F(WindowManagerProxyTest, MinimizeAllAppWindows, TestSize.Level1)
{
    DisplayId displayId = 0;
    WMError err = windowManagerProxy_->MinimizeAllAppWindows(displayId);
    EXPECT_EQ(err, WMError::WM_OK);
}

/**
 * @tc.name: SetWindowLayoutMode
 * @tc.desc: test success
 * @tc.type: FUNC
 */
HWTEST_F(WindowManagerProxyTest, SetWindowLayoutMode, TestSize.Level1)
{
    WindowLayoutMode mode = WindowLayoutMode::BASE;
    MessageParcel reply;
    WMError err = windowManagerProxy_->SetWindowLayoutMode(mode);
    EXPECT_EQ(err, static_cast<WMError>(reply.ReadInt32()));
}

/**
 * @tc.name: NotifyScreenshotEvent
 * @tc.desc: test success
 * @tc.type: FUNC
 */
HWTEST_F(WindowManagerProxyTest, NotifyScreenshotEvent, TestSize.Level1)
{
    ScreenshotEventType type = ScreenshotEventType::SCROLL_SHOT_START;
    MessageParcel reply;
    WMError err = windowManagerProxy_->NotifyScreenshotEvent(type);
    EXPECT_EQ(err, static_cast<WMError>(reply.ReadInt32()));
}

/**
 * @tc.name: UpdateProperty
 * @tc.desc: test success
 * @tc.type: FUNC
 */
HWTEST_F(WindowManagerProxyTest, UpdateProperty, TestSize.Level1)
{
    sptr<WindowProperty> windowProperty = new WindowProperty();
    PropertyChangeAction action = PropertyChangeAction::ACTION_UPDATE_RECT;
    bool isAsyncTask = true;
    MessageParcel reply;
    WMError err = windowManagerProxy_->UpdateProperty(windowProperty, action, isAsyncTask);
    EXPECT_EQ(err, static_cast<WMError>(reply.ReadInt32()));
}

/**
 * @tc.name: SetWindowGravity
 * @tc.desc: test success
 * @tc.type: FUNC
 */
HWTEST_F(WindowManagerProxyTest, SetWindowGravity, TestSize.Level1)
{
    uint32_t windowId = 1;
    WindowGravity gravity = WindowGravity::WINDOW_GRAVITY_FLOAT;
    uint32_t percent = 1;
    MessageParcel reply;
    WMError err = windowManagerProxy_->SetWindowGravity(windowId, gravity, percent);
    EXPECT_EQ(err, static_cast<WMError>(reply.ReadInt32()));
}

/**
 * @tc.name: GetSystemConfig
 * @tc.desc: test success
 * @tc.type: FUNC
 */
HWTEST_F(WindowManagerProxyTest, GetSystemConfig, TestSize.Level1)
{
    SystemConfig systemConfig;
    MessageParcel reply;
    WMError err = windowManagerProxy_->GetSystemConfig(systemConfig);
    EXPECT_EQ(err, static_cast<WMError>(reply.ReadInt32()));
}

/**
 * @tc.name: UpdateAvoidAreaListener
 * @tc.desc: test success
 * @tc.type: FUNC
 */
HWTEST_F(WindowManagerProxyTest, UpdateAvoidAreaListener, TestSize.Level1)
{
    uint32_t windowId = 0;
    bool haveListener = true;
    MessageParcel reply;
    WMError err = windowManagerProxy_->UpdateAvoidAreaListener(windowId, haveListener);
    EXPECT_EQ(err, static_cast<WMError>(reply.ReadInt32()));
}

/**
 * @tc.name: SetAnchorAndScale
 * @tc.desc: test success
 * @tc.type: FUNC
 */
HWTEST_F(WindowManagerProxyTest, SetAnchorAndScale, TestSize.Level1)
{
    int32_t x = 0;
    int32_t y = 1;
    float scale = 1.1;
    MessageParcel reply;
    SystemConfig systemConfig;
    windowManagerProxy_->SetAnchorAndScale(x, y, scale);
    WMError err = windowManagerProxy_->GetSystemConfig(systemConfig);
    EXPECT_EQ(err, static_cast<WMError>(reply.ReadInt32()));
}

/**
 * @tc.name: SetAnchorOffset
 * @tc.desc: test success
 * @tc.type: FUNC
 */
HWTEST_F(WindowManagerProxyTest, SetAnchorOffset, TestSize.Level1)
{
    int32_t deltaX = 0;
    int32_t deltaY = 1;
    SystemConfig systemConfig;
    MessageParcel reply;
    windowManagerProxy_->SetAnchorOffset(deltaX, deltaY);
    WMError err = windowManagerProxy_->GetSystemConfig(systemConfig);
    EXPECT_EQ(err, static_cast<WMError>(reply.ReadInt32()));
}

/**
 * @tc.name: OffWindowZoom
 * @tc.desc: test success
 * @tc.type: FUNC
 */
HWTEST_F(WindowManagerProxyTest, OffWindowZoom, TestSize.Level1)
{
    SystemConfig systemConfig;
    MessageParcel reply;
    windowManagerProxy_->OffWindowZoom();
    WMError err = windowManagerProxy_->GetSystemConfig(systemConfig);
    EXPECT_EQ(err, static_cast<WMError>(reply.ReadInt32()));
}

/**
 * @tc.name: RaiseToAppTop
 * @tc.desc: test success
 * @tc.type: FUNC
 */
HWTEST_F(WindowManagerProxyTest, RaiseToAppTop, TestSize.Level1)
{
    uint32_t windowId = 0;
    WMError err = windowManagerProxy_->RaiseToAppTop(windowId);
    EXPECT_EQ(err, WMError::WM_OK);
}

/**
 * @tc.name: GetSnapshot
 * @tc.desc: test success
 * @tc.type: FUNC
 */
HWTEST_F(WindowManagerProxyTest, GetSnapshot, TestSize.Level1)
{
    uint32_t windowId = 0;
    Media::InitializationOptions opts;
    opts.size.width = 200;
    opts.size.height = 300;
    std::shared_ptr<Media::PixelMap> pixelMap(Media::PixelMap::Create(opts).release());
    auto res = windowManagerProxy_->GetSnapshot(windowId);
    EXPECT_NE(res, pixelMap);
}

/**
 * @tc.name: SetGestureNavigationEnabled
 * @tc.desc: test success
 * @tc.type: FUNC
 */
HWTEST_F(WindowManagerProxyTest, SetGestureNavigationEnabled, TestSize.Level1)
{
    bool enable = true;
    MessageParcel reply;
    WMError err = windowManagerProxy_->SetGestureNavigationEnabled(enable);
    EXPECT_EQ(err, static_cast<WMError>(reply.ReadInt32()));
}

/**
 * @tc.name: DispatchKeyEvent
 * @tc.desc: test success
 * @tc.type: FUNC
 */
HWTEST_F(WindowManagerProxyTest, DispatchKeyEvent, TestSize.Level1)
{
    uint32_t windowId = 0;
    std::shared_ptr<MMI::KeyEvent> event = MMI::KeyEvent::Create();
    SystemConfig systemConfig;
    MessageParcel reply;
    windowManagerProxy_->DispatchKeyEvent(windowId, event);
    WMError err = windowManagerProxy_->GetSystemConfig(systemConfig);
    EXPECT_EQ(err, static_cast<WMError>(reply.ReadInt32()));
}

/**
 * @tc.name: NotifyDumpInfoResult
 * @tc.desc: test success
 * @tc.type: FUNC
 */
HWTEST_F(WindowManagerProxyTest, NotifyDumpInfoResult, TestSize.Level1)
{
    std::vector<std::string> info;
    string windowName = "windowName";
    info.push_back(windowName);
    SystemConfig systemConfig;
    MessageParcel reply;
    windowManagerProxy_->NotifyDumpInfoResult(info);
    WMError err = windowManagerProxy_->GetSystemConfig(systemConfig);
    EXPECT_EQ(err, static_cast<WMError>(reply.ReadInt32()));
}

/**
 * @tc.name: SetMaximizeMode
 * @tc.desc: test success
 * @tc.type: FUNC
 */
HWTEST_F(WindowManagerProxyTest, SetMaximizeMode, TestSize.Level1)
{
    MaximizeMode maximizeMode = MaximizeMode::MODE_FULL_FILL;
    SystemConfig systemConfig;
    MessageParcel reply;
    windowManagerProxy_->SetMaximizeMode(maximizeMode);
    WMError err = windowManagerProxy_->GetSystemConfig(systemConfig);
    EXPECT_EQ(err, static_cast<WMError>(reply.ReadInt32()));
}

/**
 * @tc.name: GetMaximizeMode
 * @tc.desc: test success
 * @tc.type: FUNC
 */
HWTEST_F(WindowManagerProxyTest, GetMaximizeMode, TestSize.Level1)
{
    MessageParcel reply;
    MaximizeMode mode = windowManagerProxy_->GetMaximizeMode();
    EXPECT_NE(mode, static_cast<MaximizeMode>(reply.ReadInt32()));
}

/**
 * @tc.name: GetFocusWindowInfo
 * @tc.desc: test success
 * @tc.type: FUNC
 */
HWTEST_F(WindowManagerProxyTest, GetFocusWindowInfo, TestSize.Level1)
{
    FocusChangeInfo focusInfo;
    SystemConfig systemConfig;
    MessageParcel reply;
    windowManagerProxy_->GetFocusWindowInfo(focusInfo);
    WMError err = windowManagerProxy_->GetSystemConfig(systemConfig);
    EXPECT_EQ(err, static_cast<WMError>(reply.ReadInt32()));
}

/**
 * @tc.name: GetWindowStateSnapshot01
 * @tc.desc: test all error branches
 * @tc.type: FUNC
 */
HWTEST_F(WindowManagerProxyTest, GetWindowStateSnapshot01, TestSize.Level1)
{
    int32_t persistentId = 1;
    std::string winStateSnapshotJsonStr = "{}";

    // Test with null remote
    auto tempProxy = sptr<WindowManagerProxy>::MakeSptr(nullptr);
    auto ret = tempProxy->GetWindowStateSnapshot(persistentId, winStateSnapshotJsonStr);
    EXPECT_EQ(ret, WMError::WM_ERROR_IPC_FAILED);

    sptr<MockIRemoteObject> remoteMocker = sptr<MockIRemoteObject>::MakeSptr();
    auto proxy = sptr<WindowManagerProxy>::MakeSptr(remoteMocker);
    ASSERT_NE(proxy, nullptr);

    // Test WriteInterfaceToken failed
    MockMessageParcel::ClearAllErrorFlag();
    MockMessageParcel::SetWriteInterfaceTokenErrorFlag(true);
    ret = proxy->GetWindowStateSnapshot(persistentId, winStateSnapshotJsonStr);
    EXPECT_EQ(ret, WMError::WM_ERROR_IPC_FAILED);
    MockMessageParcel::SetWriteInterfaceTokenErrorFlag(false);

    // Test WriteInt32 failed
    MockMessageParcel::SetWriteInt32ErrorFlag(true);
    ret = proxy->GetWindowStateSnapshot(persistentId, winStateSnapshotJsonStr);
    EXPECT_EQ(ret, WMError::WM_ERROR_IPC_FAILED);
    MockMessageParcel::SetWriteInt32ErrorFlag(false);

    // Test WriteString failed
    MockMessageParcel::SetWriteStringErrorFlag(true);
    ret = proxy->GetWindowStateSnapshot(persistentId, winStateSnapshotJsonStr);
    EXPECT_EQ(ret, WMError::WM_ERROR_IPC_FAILED);
    MockMessageParcel::SetWriteStringErrorFlag(false);

    // Test SendRequest failed
    remoteMocker->SetRequestResult(ERR_INVALID_DATA);
    ret = proxy->GetWindowStateSnapshot(persistentId, winStateSnapshotJsonStr);
    EXPECT_EQ(ret, WMError::WM_ERROR_IPC_FAILED);

    // Test ReadString failed
    MockMessageParcel::SetReadStringErrorFlag(true);
    remoteMocker->SetRequestResult(ERR_NONE);
    ret = proxy->GetWindowStateSnapshot(persistentId, winStateSnapshotJsonStr);
    EXPECT_EQ(ret, WMError::WM_ERROR_IPC_FAILED);
    MockMessageParcel::SetReadStringErrorFlag(false);

    // Test ReadInt32 failed
    MockMessageParcel::SetReadInt32ErrorFlag(true);
    ret = proxy->GetWindowStateSnapshot(persistentId, winStateSnapshotJsonStr);
    EXPECT_EQ(ret, WMError::WM_ERROR_IPC_FAILED);
    MockMessageParcel::SetReadInt32ErrorFlag(false);

    // Test success
    remoteMocker->SetRequestResult(ERR_NONE);
    ret = proxy->GetWindowStateSnapshot(persistentId, winStateSnapshotJsonStr);
    EXPECT_NE(ret, WMError::WM_ERROR_INVALID_CALLING);
}
}
}
}
